#include "cg_main_mp.h"
#include <universal/q_shared.h>
#include <universal/com_memory.h>
#include <universal/assertive.h>
#include <universal/dvar.h>
#include "cg_view_mp.h"
#include <DynEntity/DynEntity_client.h>
#include <cgame/offhandweapons.h>
#include <cgame/cg_compass.h>
#include <cgame/cg_spikeacoustic.h>
#include <cgame/cg_ammocounter.h>
#include <cgame/cg_visionsets.h>
#include "cg_scoreboard_mp.h"
#include <cgame/cg_hudelem.h>
#include <cgame/cg_weapon_options.h>
#include <turret/turret_placement.h>
#include <bgame/bg_misc.h>
#include <game_mp/g_main_mp.h>
#include "cg_local_mp.h"
#include <ragdoll/ragdoll.h>
#include <ik/ik.h>
#include <universal/com_math_anglevectors.h>
#include <client/cl_console.h>
#include <sound/snd_bank.h>
#include <universal/surfaceflags.h>
#include <cgame/cg_sound.h>
#include <sound/snd_public_async.h>
#include <EffectsCore/fx_system.h>
#include <EffectsCore/fx_update.h>
#include <client_mp/cl_cgame_mp.h>
#include <stringed/stringed_hooks.h>
#include <clientscript/scr_const.h>
#include <clientscript/cscr_stringlist.h>
#include <game/g_load_utils.h>
#include <cgame/cg_scr_main.h>
#include "cg_vehicles_mp.h"
#include <qcommon/dobj_management.h>
#include "cg_ents_mp.h"
#include <clientscript/cscr_memorytree.h>
#include <qcommon/threads.h>
#include <bgame/bg_slidemove.h>
#include <cgame/cg_draw_names.h>
#include <cgame/cg_main.h>
#include <client_mp/cl_input_mp.h>
#include <EffectsCore/fx_load_obj.h>
#include "cg_servercmds_mp.h"
#include <gfx_d3d/r_rendercmds.h>
#include <gfx_d3d/r_dvars.h>
#include <client_mp/cl_scrn_mp.h>
#include "cg_newDraw_mp.h"
#include "cg_consolecmds_mp.h"
#include <bgame/bg_mantle.h>
#include <bgame/bg_dog_animations_mp.h>
#include <bgame/bg_vehicle_anim.h>
#include <clientscript/cscr_vm.h>
#include <cgame/cg_info.h>
#include <gfx_d3d/r_stream.h>
#include <qcommon/com_profilemapload.h>
#include <cgame/cg_bolt.h>
#include <cgame/cg_localents.h>
#include <cgame/cg_drawtools.h>
#include <gfx_d3d/r_water_sim.h>
#include <gfx_d3d/r_bsp_load_obj.h>
#include <aim_assist/aim_target.h>
#include <client/splitscreen.h>
#include <bgame/bg_fire.h>
#include <gfx_d3d/r_model.h>
#include <cgame/cg_effects_load_obj.h>
#include <ui/ui_shared_obj.h>
#include "cg_draw_mp.h"
#include <ragdoll/ragdoll_update.h>
#include <cgame/cg_spawn.h>
#include <gfx_d3d/r_shader_constant_set.h>
#include "cg_animtree_mp.h"
#include <gfx_d3d/r_cinematic.h>
#include "cg_snapshot_mp.h"
#include <glass/glass_client.h>
#include <physics/rope.h>
#include "cg_predict_mp.h"
#include <cgame/cg_local.h>
#include <win32/win_main.h>

const char *cg_thirdPersonModeNames[4] =
{ "Free", "Fixed", "Locked", NULL };

const char *cg_drawMaterialNames[6] =
{
  "Off",
  "CONTENTS_SOLID",
  "NONSOLID",
  "MASK_PLAYERSOLID",
  "MASK_CLIENTEFFECTS",
  NULL
};

const char *snd_drawInfoStrings[5] =
{ "None", "3D", "Stream", "2D", NULL };

const char *snd_drawSortStrings[6] =
{ "priority", "channel", "alias", "dry level", "entity", NULL };

const char *cg_drawBudgetNames[6] =
{ "Off", "Critical Only", "All >=64", "32 to 63", "<32", NULL };

const char *cg_drawFpsNames[5] =
{ "Off", "Simple", "SimpleRanges", "Verbose", NULL };

const char *debugOverlayNames[4] =
{ "Off", "ViewmodelInfo", "FontTest", NULL };

float (*cg_entityOriginArray[1])[3];
unsigned __int8 *cg_ikBuf[1];

centity_s *cg_entitiesArray[1];
fake_centity_s *cg_fakeEntitiesArray;

BattleChatterParams cg_BattleChatters[8];

cgMedia_t cgMedia;
bgsAnim_s cg_bgsAnim;

int cg_usedTriggerCount;
int cg_usedTriggers[300];
bool g_mapLoaded;
bool g_ambientStarted;

bool cg_fakeEntitiesInuseArray[512];
int cg_fakeEntitiesInuseCount[1];
int cg_fakeEntitiesInuseCountFromMap;
int cg_fakeEntitiesInuseCountFromLoadScript;

const dvar_s *cg_loadScripts;
const dvar_s *cg_usingClientScripts;
const dvar_s *cg_drawGun;
const dvar_s *cg_cursorHints;
const dvar_s *cg_retrieveHintTime;
const dvar_s *cg_retrieveHintTimeStuck;
const dvar_s *cg_weaponHintsCoD1Style;
const dvar_s *cg_hintFadeTime;
const dvar_s *cg_seatHintFadeTime;
const dvar_s *cg_fov;
const dvar_s *cg_fov_default;
const dvar_s *cg_fov_default_thirdperson;
const dvar_s *cg_fovScale;
const dvar_s *cg_fovMin;
const dvar_s *cg_fovExtraCam;
const dvar_s *cg_fovCompMax;
const dvar_s *cg_adsZoomToggleStyle;
const dvar_s *cg_viewVehicleInfluenceGunner;
const dvar_s *cg_viewVehicleInfluenceGunnerFiring;
const dvar_s *cg_draw2D;
const dvar_s *cg_drawErrorMessages;
const dvar_s *cg_drawHealth;
const dvar_s *cg_drawBreathHint;
const dvar_s *cg_drawMantleHint;
const dvar_s *cg_wadefps;
const dvar_s *cg_drawFPS;
const dvar_s *cg_drawFPSScale;
const dvar_s *cg_drawBudgets;
const dvar_s *cg_drawDynSModelBudget;
const dvar_s *cg_development;
const dvar_s *cg_drawAnimAttachTags;
const dvar_s *cg_drawFPSOnly;
const dvar_s *cg_profile_physics;
const dvar_s *cg_drawFPSLabels;
const dvar_s *cg_debugInfoCornerOffset;
const dvar_s *cg_drawVersion;
const dvar_s *cg_drawVersionX;
const dvar_s *cg_drawVersionY;
const dvar_s *cg_readTitleStorageLocally;
const dvar_s *snd_drawInfo;
const dvar_s *snd_drawSort;
const dvar_s *cg_drawScriptUsage;
const dvar_s *cg_drawMaterial;
const dvar_s *cg_drawModelAxis;
const dvar_s *cg_drawSnapshot;
const dvar_s *cg_drawSnapshotTime;
const dvar_s *cg_drawCrosshair;
const dvar_s *cg_drawCrosshair3D;
const dvar_s *cg_drawHoldBreathHint;
const dvar_s *cg_drawTurretCrosshair;
const dvar_s *cg_drawCrosshairNames;
const dvar_s *cg_drawCrosshairNamesPosX;
const dvar_s *cg_drawCrosshairNamesPosY;
const dvar_s *cg_drawShellshock;
const dvar_s *cg_hudStanceFlash;
const dvar_s *cg_hudStanceHintPrints;
const dvar_s *cg_hudDamageIconWidth;
const dvar_s *cg_hudDamageIconHeight;
const dvar_s *cg_hudDamageIconOffset;
const dvar_s *cg_hudDamageIconTime;
const dvar_s *cg_hudDamageDirectionalIconTime;
const dvar_s *cg_hudDamageIconInScope;
const dvar_s *cg_hudGrenadeIconMaxRangeFrag;
const dvar_s *cg_hudGrenadeIconMaxRangeFlash;
const dvar_s *cg_hudGrenadeIconMaxHeight;
const dvar_s *cg_hudGrenadeIconInScope;
const dvar_s *cg_hudGrenadeIconOffset;
const dvar_s *cg_hudGrenadeIconHeight;
const dvar_s *cg_hudGrenadeIconWidth;
const dvar_s *cg_hudGrenadeIconEnabledFlash;
const dvar_s *cg_hudGrenadePointerHeight;
const dvar_s *cg_hudGrenadePointerWidth;
const dvar_s *cg_hudGrenadePointerPivot;
const dvar_s *cg_hudGrenadePointerPulseFreq;
const dvar_s *cg_hudGrenadePointerPulseMax;
const dvar_s *cg_hudGrenadePointerPulseMin;
const dvar_s *cg_hudChatPosition;
const dvar_s *cg_hudSayPosition;
const dvar_s *cg_hudChatIntermissionPosition;
const dvar_s *cg_hudVotePosition;
const dvar_s *cg_debugDrawSafeAreas;
const dvar_s *cg_drawLagometer;
const dvar_s *drawEntityCount;
const dvar_s *drawEntityCountPos;
const dvar_s *drawEntityCountSize;
const dvar_s *drawServerBandwidth;
const dvar_s *drawServerBandwidthPos;
const dvar_s *drawServerBandwidthSize;
const dvar_s *drawKillcamData;
const dvar_s *drawKillcamDataPos;
const dvar_s *drawKillcamDataSize;
const dvar_s *cg_hudProneY;
const dvar_s *cg_mapLocationSelectionCursorSpeed;
const dvar_s *cg_mapLocationSelectionRotationSpeed;
const dvar_s *cg_hudGrenadeIndicatorFadeUp;
const dvar_s *cg_hudGrenadeIndicatorTargetColor;
const dvar_s *cg_hudGrenadeIndicatorStartColor;
const dvar_s *cg_weaponCycleDelay;
const dvar_s *cg_crosshairAlpha;
const dvar_s *cg_crosshairAlphaMin;
const dvar_s *cg_crosshairDynamic;
const dvar_s *cg_crosshairEnemyColor;
const dvar_s *cg_brass;
const dvar_s *cg_gun_fovcomp_x;
const dvar_s *cg_gun_fovcomp_y;
const dvar_s *cg_gun_fovcomp_z;
const dvar_s *cg_gun_x;
const dvar_s *cg_gun_y;
const dvar_s *cg_gun_z;
const dvar_s *cg_gun_move_f;
const dvar_s *cg_gun_move_r;
const dvar_s *cg_gun_move_u;
const dvar_s *cg_gun_ofs_f;
const dvar_s *cg_gun_ofs_r;
const dvar_s *cg_gun_ofs_u;
const dvar_s *cg_gun_move_rate;
const dvar_s *cg_gun_move_minspeed;
const dvar_s *cg_centertime;
const dvar_s *cg_debugPosition;
const dvar_s *cg_debugEvents;
const dvar_s *cg_errorDecay;
const dvar_s *cg_nopredict;
const dvar_s *cg_showmiss;
const dvar_s *cg_footsteps;
const dvar_s *cg_footprints;
const dvar_s *cg_footprintsDistortWater;
const dvar_s *cg_footprintsDebug;
const dvar_s *cg_waterTrailRippleFrequency;
const dvar_s *cg_waterTrailRippleVariance;
const dvar_s *cg_treadmarks;
const dvar_s *cg_firstPersonTracerChance;
const dvar_s *cg_laserForceOn;
const dvar_s *cg_laserRange;
const dvar_s *cg_laserRangePlayer;
const dvar_s *cg_laserRadius;
const dvar_s *cg_laserLight;
const dvar_s *cg_laserLightBodyTweak;
const dvar_s *cg_laserLightRadius;
const dvar_s *cg_laserLightBeginOffset;
const dvar_s *cg_laserEndOffset;
const dvar_s *cg_laserLightEndOffset;
const dvar_s *cg_laserFlarePct;
const dvar_s *cg_marks_ents_player_only;
const dvar_s *cg_tracerChance;
const dvar_s *cg_tracerWidth;
const dvar_s *cg_tracerSpeed;
const dvar_s *cg_tracerLength;
const dvar_s *cg_tracerNoDrawTime;
const dvar_s *cg_tracerScale;
const dvar_s *cg_tracerScaleMinDist;
const dvar_s *cg_tracerScaleDistRange;
const dvar_s *cg_tracerScrewDist;
const dvar_s *cg_tracerScrewRadius;
const dvar_s *cg_bulletWidth;
const dvar_s *cg_bulletLength;
const dvar_s *cg_thirdPersonRange;
const dvar_s *cg_thirdPersonAngle;
const dvar_s *cg_thirdPersonFocusDist;
const dvar_s *cg_thirdPerson;
const dvar_s *cg_thirdPersonMode;
const dvar_s *cg_chatTime;
const dvar_s *cg_chatHeight;
const dvar_s *cg_predictItems;
const dvar_s *cg_spectateThirdPerson;
const dvar_s *cg_teamChatsOnly;
const dvar_s *cg_use_colored_smoke;
const dvar_s *cg_fakefireWizbyChance;
const dvar_s *cg_paused;
const dvar_s *cg_drawpaused;
const dvar_s *cg_synchronousClients;
const dvar_s *cg_debug_overlay_viewport;
const dvar_s *cg_fs_debug;
const dvar_s *cg_debugFace;
const dvar_s *cg_dumpAnims;
const dvar_s *cg_developer;
const dvar_s *cg_minicon;
const dvar_s *cg_subtitles;
const dvar_s *cg_subtitleMinTime;
const dvar_s *cg_subtitleWidthStandard;
const dvar_s *cg_subtitleWidthWidescreen;
const dvar_s *cg_gameMessageWidth;
const dvar_s *cg_gameBoldMessageWidth;
const dvar_s *cg_descriptiveText;
const dvar_s *cg_youInKillCamSize;
const dvar_s *cg_scriptIconSize;
const dvar_s *cg_connectionIconSize;
const dvar_s *cg_voiceIconSize;
const dvar_s *cg_constantSizeHeadIcons;
const dvar_s *cg_headIconMinScreenRadius;
const dvar_s *cg_overheadNamesMaxDist;
const dvar_s *cg_overheadNamesNearDist;
const dvar_s *cg_overheadNamesFarDist;
const dvar_s *cg_overheadNamesFarScale;
const dvar_s *cg_overheadNamesSize;
const dvar_s *cg_overheadIconSize;
const dvar_s *cg_overheadRankSize;
const dvar_s *cg_overheadNamesGlow;
const dvar_s *cg_overheadNamesFont;
const dvar_s *cg_drawFriendlyNames;
const dvar_s *cg_enemyNameFadeIn;
const dvar_s *cg_friendlyNameFadeIn;
const dvar_s *cg_enemyNameFadeOut;
const dvar_s *cg_friendlyNameFadeOut;
const dvar_s *cg_drawThroughWalls;
const dvar_s *cg_playerHighlightTargetSize;
const dvar_s *cg_playerHighlightEnemyColor;
const dvar_s *cg_playerHighlightBrightness;
const dvar_s *cg_playerHighlightMinFade;
const dvar_s *cg_playerHighlightBlinkTime;
const dvar_s *cg_corpseHighlightFadeTime;
const dvar_s *cg_cameraSpikeHighlightBrightness;
const dvar_s *cg_cameraSpikeEnemyColor;
const dvar_s *cg_adsZScaleMax;
const dvar_s *cg_infraredHighlightScale;
const dvar_s *cg_infraredHighlightOffset;
const dvar_s *cg_allow_mature;
const dvar_s *cg_mature;
const dvar_s *cg_blood;
const dvar_s *cg_invalidCmdHintDuration;
const dvar_s *cg_invalidCmdHintBlinkInterval;
const dvar_s *cg_viewZSmoothingMin;
const dvar_s *cg_viewZSmoothingMax;
const dvar_s *cg_viewZSmoothingTime;
const dvar_s *overrideNVGModelWithKnife;
const dvar_s *cg_visionSetLerpMaxIncreasePerFrame;
const dvar_s *cg_visionSetLerpMaxDecreasePerFrame;
const dvar_s *cg_flareVisionSetFadeDuration;
const dvar_s *cg_turretBipodOffset;
const dvar_s *cg_AllPlayerNamesVisible;
const dvar_s *cg_ScoresColor_MyTeam;
const dvar_s *cg_ScoresColor_EnemyTeam;
const dvar_s *cg_ScoresColor_Spectator;
const dvar_s *cg_ScoresColor_Free;
const dvar_s *cg_ScoresColor_Allies;
const dvar_s *cg_ScoresColor_Axis;
const dvar_s *cg_TeamName_Allies;
const dvar_s *cg_TeamName_Axis;
const dvar_s *cg_TeamColor_Allies;
const dvar_s *cg_TeamColor_Axis;
const dvar_s *cg_TeamColor_MyTeam;
const dvar_s *cg_TeamColor_EnemyTeam;
const dvar_s *cg_TeamColor_MyTeamAlt;
const dvar_s *cg_TeamColor_EnemyTeamAlt;
const dvar_s *cg_TeamColor_Squad;
const dvar_s *cg_TeamColor_Spectator;
const dvar_s *cg_TeamColor_Free;
const dvar_s *cg_proneFeetCollisionHull;

const dvar_s *g_compassShowEnemies;
const dvar_s *cg_drawWVisDebug;
const dvar_s *debugOverlay;
const dvar_s *cg_motionblur_duration;
const dvar_s *cg_motionblur_fadeout;
const dvar_s *cg_timedDamageDuration;
const dvar_s *cg_MinDownedPulseRate;
const dvar_s *cg_MaxDownedPulseRate;
const dvar_s *cg_playerFrustumHalfHeight;
const dvar_s *cg_overheadNamesTagUpdateInterval;
const dvar_s *cg_canSeeFriendlyFrustumUpdateInterval;
const dvar_s *cg_canSeeFriendlyFrustumExpand;
const dvar_s *cg_canSeeFriendlyFrustumMinDistance;
const dvar_s *cg_watersheeting;
const dvar_s *cg_debug_triggers;
const dvar_s *cg_cameraWaterClip;
const dvar_s *cg_cameraVehicleExitTweenTime;
const dvar_s *cg_vehicle_piece_damagesfx_threshold;
const dvar_s *cg_debugLocHit;
const dvar_s *cg_debugLocHitTime;

cg_s *cgArray;
cgs_t *cgsArray;

int __cdecl CG_GetClientNumForLocalClient(int localClientNum)
{
    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    625,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    return cgArray[localClientNum].clientNum;
}

bool __cdecl CG_IsRagdollTrajectory(const trajectory_t *trajectory)
{
    if ( !trajectory
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 632, 0, "%s", "trajectory") )
    {
        __debugbreak();
    }
    if ( !ragdoll_enable
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 633, 0, "%s", "ragdoll_enable") )
    {
        __debugbreak();
    }
    return ragdoll_enable->current.enabled && trajectory->trType >= 0xCu && trajectory->trType <= 0xEu;
}

void __cdecl CG_SetupSplitscreenDvars()
{
    Dvar_SetFloat((dvar_s *)cg_hudGrenadeIconHeight, 25.0);
    Dvar_SetFloat((dvar_s *)cg_hudGrenadeIconWidth, 25.0);
    Dvar_SetFloat((dvar_s *)cg_hudGrenadeIconOffset, 50.0);
    Dvar_SetFloat((dvar_s *)cg_hudGrenadePointerHeight, 12.0);
    Dvar_SetFloat((dvar_s *)cg_hudGrenadePointerWidth, 25.0);
    Dvar_SetVec2((dvar_s *)cg_hudGrenadePointerPivot, (12.0), (27.0));
    Dvar_SetFloat((dvar_s *)cg_fovScale, 1.0);
}

void __cdecl CG_RegisterDvars()
{
    bool v0; // al

    v0 = G_ExitAfterToolComplete();
    cg_loadScripts = _Dvar_RegisterBool("g_loadScripts", !v0, 0, "Disable scripts from loading");
    cg_usingClientScripts = _Dvar_RegisterBool("cg_usingClientScripts", 1, 0x80u, "True, if client scripts are enabled.");
    cg_drawGun = _Dvar_RegisterBool("cg_drawGun", 1, 0x80u, "Draw the view model");
    cg_cursorHints = _Dvar_RegisterInt(
                                         "cg_cursorHints",
                                         4,
                                         0,
                                         4,
                                         1u,
                                         "Draw cursor hints where:\n"
                                         " 0: no hints\n"
                                         "\t1:\tsin size pulse\n"
                                         "\t2:\tone way size pulse\n"
                                         "\t3:\talpha pulse\n"
                                         "\t4:\tstatic image");
    cg_retrieveHintTime = _Dvar_RegisterInt(
                                                    "cg_retrieveHintTime",
                                                    0,
                                                    0,
                                                    0x7FFFFFFF,
                                                    0x80u,
                                                    "Time in milliseconds between the landing of a retrievable object and the start of the pulse sh"
                                                    "ader to hint that the object is retrievable");
    cg_retrieveHintTimeStuck = _Dvar_RegisterInt(
                                                             "cg_retrieveHintTimeStuck",
                                                             2000,
                                                             0,
                                                             0x7FFFFFFF,
                                                             0x80u,
                                                             "Time in milliseconds between the retrievable object being stuck in an entity and the star"
                                                             "t of the pulse shader to hint that the object is retrievable");
    cg_weaponHintsCoD1Style = _Dvar_RegisterBool(
                                                            "cg_weaponHintsCoD1Style",
                                                            1,
                                                            0x1000u,
                                                            "Draw weapon hints in CoD1 style: with the weapon name, and with the icon below");
    cg_hintFadeTime = _Dvar_RegisterInt(
                                            "cg_hintFadeTime",
                                            100,
                                            0,
                                            0x7FFFFFFF,
                                            1u,
                                            "Time in milliseconds for the cursor hint to fade");
    cg_seatHintFadeTime = _Dvar_RegisterInt(
                                                    "cg_seatHintFadeTime",
                                                    2000,
                                                    0,
                                                    0x7FFFFFFF,
                                                    1u,
                                                    "Time in milliseconds for the seat hint to fade");
    cg_fov = _Dvar_RegisterFloat("cg_fov", 65.0, 1.0, 160.0, 0x80u, "The field of view angle in degrees");
    cg_fov_default = _Dvar_RegisterFloat(
                                         "cg_fov_default",
                                         65.0,
                                         65.0,
                                         80.0,
                                         1u,
                                         "User default field of view angle in degrees");
    cg_fov_default_thirdperson = _Dvar_RegisterFloat(
                                                                 "cg_fov_default_thirdperson",
                                                                 40.0,
                                                                 1.0,
                                                                 160.0,
                                                                 1u,
                                                                 "User default 3rd person field of view angle in degrees");
    cg_fovScale = _Dvar_RegisterFloat("cg_fovScale", 1.0, 0.2, 2.0, 0x80u, "Scale applied to the field of view");
    cg_fovMin = _Dvar_RegisterFloat("cg_fovMin", 10.0, 1.0, 160.0, 0x80u, "The minimum possible field of view");
    cg_fovExtraCam = _Dvar_RegisterFloat(
                                         "cg_fovExtraCam",
                                         30.0,
                                         1.0,
                                         160.0,
                                         0x80u,
                                         "The field of view angle in degrees for the extra cam");
    cg_fovCompMax = _Dvar_RegisterFloat(
                                        "cg_fovCompMax",
                                        85.0,
                                        1.0,
                                        160.0,
                                        0,
                                        "The maximum field of view to compensate for gun placement");
    cg_adsZoomToggleStyle = _Dvar_RegisterInt(
                                                        "cg_adsZoomToggleStyle",
                                                        1,
                                                        0,
                                                        1,
                                                        1u,
                                                        "Style of zoom toggle - 0=oscillate, 1=rotate");
    cg_viewVehicleInfluenceGunner = _Dvar_RegisterVec3(
                                                                        "cg_viewVehicleInfluenceGunner",
                                                                        (1.0),
                                                                        (1.0),
                                                                        (1.0),
                                                                        0.0,
                                                                        1.0,
                                                                        0x1080u,
                                                                        "The influence on the view from being a vehicle gunner");
    cg_viewVehicleInfluenceGunnerFiring = _Dvar_RegisterVec3(
                                                                                    "cg_viewVehicleInfluenceGunnerFiring",
                                                                                    (0.0),
                                                                                    (0.0),
                                                                                    (0.0),
                                                                                    0.0,
                                                                                    1.0,
                                                                                    0x1080u,
                                                                                    "The influence on the view from being a vehicle gunner while firing");
    cg_draw2D = _Dvar_RegisterBool("cg_draw2D", 1, 0x80u, "Draw 2D screen elements");
    cg_drawErrorMessages = _Dvar_RegisterBool("cg_drawErrorMessages", 1, 0, "Draw error/warning text");
    cg_drawHealth = _Dvar_RegisterBool("cg_drawHealth", 0, 0x80u, "Draw health bar");
    cg_drawBreathHint = _Dvar_RegisterBool("cg_drawBreathHint", 1, 1u, "Draw a 'hold breath to steady' hint");
    cg_drawMantleHint = _Dvar_RegisterBool("cg_drawMantleHint", 1, 1u, "Draw a 'press key to mantle' hint");
    cg_wadefps = _Dvar_RegisterBool(
                                 "cl_wadefps",
                                 0,
                                 0,
                                 "Toggles the display of the conspicuous FPS meter in non-development builds only.");
    cg_drawFPS = _Dvar_RegisterEnum("cg_drawFPS", cg_drawFpsNames, 1, 1u, "Draw frames per second");
    cg_drawFPSScale = _Dvar_RegisterFloat("cg_drawFPSScale", 0.0, 0.0, 100.0, 1u, "Draw FPS size scale");
    cg_drawBudgets = _Dvar_RegisterEnum("cg_drawBudgets", cg_drawBudgetNames, 1, 0, "Draw asset type budgets");
    cg_drawDynSModelBudget = _Dvar_RegisterBool("cg_drawDynSModelBudget", 1, 0, "Draw dynamic static model budget");
    cg_development = _Dvar_RegisterBool(
                                         "cg_development",
                                         1,
                                         0,
                                         "Indicates if we are in DEVELOPMENT (non-release ship builds)");
    cg_drawAnimAttachTags = _Dvar_RegisterBool("cg_drawAnimAttachTags", 0, 0x80u, "Display anim attach tags debug data");
    cg_drawFPSOnly = _Dvar_RegisterBool("cg_drawFPSOnly", 0, 1u, "Draw only the FPS stats in the upper right");
    cg_profile_physics = _Dvar_RegisterBool("profile_physics", 0, 1u, "Draw physics & collision profiltimers");
    cg_drawFPSLabels = _Dvar_RegisterBool("cg_drawFPSLabels", 1, 1u, "Draw FPS Info Labels");
    cg_debugInfoCornerOffset = _Dvar_RegisterVec2(
                                                             "cg_debugInfoCornerOffset",
                                                             (0.0),
                                                             (0.0),
                                                             -200.0,
                                                             640.0,
                                                             1u,
                                                             "Offset from top-right corner, for cg_drawFPS, etc");
    cg_drawVersion = _Dvar_RegisterBool("cg_drawVersion", 1, 0, "Draw the game version");
    cg_drawVersionX = _Dvar_RegisterFloat("cg_drawVersionX", 50.0, 0.0, 512.0, 0, "X offset for the version string");
    cg_drawVersionY = _Dvar_RegisterFloat("cg_drawVersionY", 18.0, 0.0, 512.0, 0, "Y offset for the version string");
    cg_readTitleStorageLocally = _Dvar_RegisterBool(
                                                                 "cg_readTitleStorageLocally",
                                                                 0,
                                                                 0,
                                                                 "Read title storage locally, instead of from the Xbox Live server");
    snd_drawInfo = _Dvar_RegisterEnum("snd_drawInfo", snd_drawInfoStrings, 0, 0, "Draw debugging information for sounds");
    snd_drawSort = _Dvar_RegisterEnum("snd_drawSort", snd_drawSortStrings, 0, 0, "Sort debugging information for sounds");
    cg_drawScriptUsage = _Dvar_RegisterBool("cg_drawScriptUsage", 0, 0, "Draw debugging information for scripts");
    cg_drawMaterial = _Dvar_RegisterEnum(
                                            "cg_drawMaterial",
                                            cg_drawMaterialNames,
                                            0,
                                            0x80u,
                                            "Draw debugging information for materials");
    cg_drawModelAxis = _Dvar_RegisterInt(
                                             "cg_drawModelAxis",
                                             -1,
                                             -1,
                                             256,
                                             0x80u,
                                             "Draw debugging axis for a bone of the model under the crosshair");
    cg_drawSnapshot = _Dvar_RegisterBool("cg_drawSnapshot", 0, 1u, "Draw debugging information for snapshots");
    cg_drawSnapshotTime = _Dvar_RegisterBool("cg_drawSnapshotTime", 1, 1u, "Draw length of snapshot buffer");
    cg_drawCrosshair = _Dvar_RegisterBool("cg_drawCrosshair", 1, 0x81u, "Turn on weapon crosshair");
    cg_drawCrosshair3D = _Dvar_RegisterBool("cg_drawCrosshair3D", 1, 0, "Turn on weapon crosshair in 3D mode.");
    cg_drawHoldBreathHint = _Dvar_RegisterBool(
                                                        "cg_drawHoldBreathHint",
                                                        1,
                                                        1u,
                                                        "Turn on hold breath hint string for the sniper rifles");
    cg_drawTurretCrosshair = _Dvar_RegisterBool("cg_drawTurretCrosshair", 1, 1u, "Draw a cross hair when using a turret");
    cg_drawCrosshairNames = _Dvar_RegisterBool(
                                                        "cg_drawCrosshairNames",
                                                        1,
                                                        0x81u,
                                                        "Draw the name of an enemy under the crosshair");
    cg_drawCrosshairNamesPosX = _Dvar_RegisterInt(
                                                                "cg_drawCrosshairNamesPosX",
                                                                300,
                                                                0,
                                                                640,
                                                                0,
                                                                "Virtual screen space position of the crosshair name");
    cg_drawCrosshairNamesPosY = _Dvar_RegisterInt(
                                                                "cg_drawCrosshairNamesPosY",
                                                                180,
                                                                0,
                                                                480,
                                                                0,
                                                                "Virtual screen space position of the crosshair name");
    cg_drawShellshock = _Dvar_RegisterBool("cg_drawShellshock", 1, 0x80u, "Draw shellshock & flashbang screen effects.");
    cg_hudStanceFlash = _Dvar_RegisterColor(
                                                "cg_hudStanceFlash",
                                                1.0,
                                                1.0,
                                                1.0,
                                                1.0,
                                                0,
                                                "The background color of the flash when the stance changes");
    cg_hudStanceHintPrints = _Dvar_RegisterBool(
                                                         "cg_hudStanceHintPrints",
                                                         0,
                                                         1u,
                                                         "Draw helpful text to say how to change stances");
    cg_hudDamageIconWidth = _Dvar_RegisterFloat(
                                                        "cg_hudDamageIconWidth",
                                                        128.0,
                                                        0.0,
                                                        512.0,
                                                        1u,
                                                        "The width of the damage icon");
    cg_hudDamageIconHeight = _Dvar_RegisterFloat(
                                                         "cg_hudDamageIconHeight",
                                                         64.0,
                                                         0.0,
                                                         512.0,
                                                         1u,
                                                         "The height of the damage icon");
    cg_hudDamageIconOffset = _Dvar_RegisterFloat(
                                                         "cg_hudDamageIconOffset",
                                                         128.0,
                                                         0.0,
                                                         512.0,
                                                         1u,
                                                         "The offset from the center of the damage icon");
    cg_hudDamageIconTime = _Dvar_RegisterInt(
                                                     "cg_hudDamageIconTime",
                                                     2000,
                                                     0,
                                                     0x7FFFFFFF,
                                                     1u,
                                                     "The amount of time for the damage icon to stay on screen after damage is taken");
    cg_hudDamageDirectionalIconTime = _Dvar_RegisterInt(
                                                                            "cg_hudDamageDirectionalIconTime",
                                                                            1000,
                                                                            1,
                                                                            0x7FFFFFFF,
                                                                            1u,
                                                                            "The amount of time for the damage icon to stay on screen after damage is taken");
    cg_hudDamageIconInScope = _Dvar_RegisterBool(
                                                            "cg_hudDamageIconInScope",
                                                            0,
                                                            1u,
                                                            "Draw damage icons when aiming down the sight of a scoped weapon");
    cg_hudGrenadeIconMaxRangeFrag = _Dvar_RegisterFloat(
                                                                        "cg_hudGrenadeIconMaxRangeFrag",
                                                                        250.0,
                                                                        0.0,
                                                                        1000.0,
                                                                        0x1080u,
                                                                        "The minimum distance that a grenade has to be from a player in order to be shown on "
                                                                        "the grenade indicator");
    cg_hudGrenadeIconMaxRangeFlash = _Dvar_RegisterFloat(
                                                                         "cg_hudGrenadeIconMaxRangeFlash",
                                                                         500.0,
                                                                         0.0,
                                                                         2000.0,
                                                                         0x1080u,
                                                                         "The minimum distance that a flashbang has to be from a player in order to be shown "
                                                                         "on the grenade indicator");
    cg_hudGrenadeIconMaxHeight = _Dvar_RegisterFloat(
                                                                 "cg_hudGrenadeIconMaxHeight",
                                                                 104.0,
                                                                 0.0,
                                                                 1000.0,
                                                                 1u,
                                                                 "The minimum height difference between a player and a grenade for the grenade to be show"
                                                                 "n on the grenade indicator");
    cg_hudGrenadeIconInScope = _Dvar_RegisterBool(
                                                             "cg_hudGrenadeIconInScope",
                                                             0,
                                                             1u,
                                                             "Show the grenade indicator when aiming down the sight of a scoped weapon");
    cg_hudGrenadeIconOffset = _Dvar_RegisterFloat(
                                                            "cg_hudGrenadeIconOffset",
                                                            50.0,
                                                            0.0,
                                                            512.0,
                                                            1u,
                                                            "The offset from the center of the screen for a grenade icon");
    cg_hudGrenadeIconHeight = _Dvar_RegisterFloat(
                                                            "cg_hudGrenadeIconHeight",
                                                            25.0,
                                                            0.0,
                                                            512.0,
                                                            1u,
                                                            "The height of the grenade indicator icon");
    cg_hudGrenadeIconWidth = _Dvar_RegisterFloat(
                                                         "cg_hudGrenadeIconWidth",
                                                         25.0,
                                                         0.0,
                                                         512.0,
                                                         1u,
                                                         "The width of the grenade indicator icon");
    cg_hudGrenadeIconEnabledFlash = _Dvar_RegisterBool(
                                                                        "cg_hudGrenadeIconEnabledFlash",
                                                                        0,
                                                                        1u,
                                                                        "Show the grenade indicator for flash grenades");
    cg_hudGrenadePointerHeight = _Dvar_RegisterFloat(
                                                                 "cg_hudGrenadePointerHeight",
                                                                 12.0,
                                                                 0.0,
                                                                 512.0,
                                                                 1u,
                                                                 "The height of the grenade indicator pointer");
    cg_hudGrenadePointerWidth = _Dvar_RegisterFloat(
                                                                "cg_hudGrenadePointerWidth",
                                                                25.0,
                                                                0.0,
                                                                512.0,
                                                                1u,
                                                                "The width of the grenade indicator pointer");
    cg_hudGrenadePointerPivot = _Dvar_RegisterVec2(
                                                                "cg_hudGrenadePointerPivot",
                                                                (12.0),
                                                                (27.0),
                                                                0.0,
                                                                512.0,
                                                                1u,
                                                                "The pivot point of th grenade indicator pointer");
    cg_hudGrenadePointerPulseFreq = _Dvar_RegisterFloat(
                                                                        "cg_hudGrenadePointerPulseFreq",
                                                                        1.7,
                                                                        0.1,
                                                                        50.0,
                                                                        0,
                                                                        "The number of times per second that the grenade indicator flashes in Hertz");
    cg_hudGrenadePointerPulseMax = _Dvar_RegisterFloat(
                                                                     "cg_hudGrenadePointerPulseMax",
                                                                     1.85,
                                                                     0.0,
                                                                     3.0,
                                                                     0,
                                                                     "The maximum alpha of the grenade indicator pulse. Values higher than 1 will cause the"
                                                                     " indicator to remain at full brightness for longer");
    cg_hudGrenadePointerPulseMin = _Dvar_RegisterFloat(
                                                                     "cg_hudGrenadePointerPulseMin",
                                                                     0.30000001,
                                                                     -3.0,
                                                                     1.0,
                                                                     0,
                                                                     "The minimum alpha of the grenade indicator pulse. Values lower than 0 will cause the "
                                                                     "indicator to remain at full transparency for longer");
    cg_hudChatPosition = _Dvar_RegisterVec2(
                                                 "cg_hudChatPosition",
                                                 (5.0),
                                                 (204.0),
                                                 0.0,
                                                 640.0,
                                                 1u,
                                                 "Position of the HUD chat box");
    cg_hudSayPosition = _Dvar_RegisterVec2(
                                                "cg_hudSayPosition",
                                                (5.0),
                                                (180.0),
                                                0.0,
                                                640.0,
                                                1u,
                                                "Position of the HUD say box");
    cg_hudChatIntermissionPosition = _Dvar_RegisterVec2(
                                                                         "cg_hudChatIntermissionPosition",
                                                                         (5.0),
                                                                         (90.0),
                                                                         0.0,
                                                                         640.0,
                                                                         1u,
                                                                         "Position of the HUD chat box during intermission");
    cg_hudVotePosition = _Dvar_RegisterVec2(
                                                 "cg_hudVotePosition",
                                                 (5.0),
                                                 (220.0),
                                                 0.0,
                                                 640.0,
                                                 1u,
                                                 "Position of the HUD vote box");
    cg_debugDrawSafeAreas = _Dvar_RegisterBool(
                                                        "cg_debugDrawSafeAreas",
                                                        0,
                                                        0,
                                                        "Show the safe area outlines for the safe areas on the UI");
    cg_drawLagometer = _Dvar_RegisterBool("drawLagometer", 0, 1u, "Enable the 'lagometer'");
    drawEntityCount = _Dvar_RegisterBool("drawEntityCount", 0, 0, "Enable entity count drawing");
    drawEntityCountPos = _Dvar_RegisterVec2(
                                                 "drawEntityCountPos",
                                                 (-55.0),
                                                 (-180.0),
                                                 -3.4028235e38,
                                                 3.4028235e38,
                                                 0,
                                                 "Where to draw the entity count graph");
    drawEntityCountSize = _Dvar_RegisterInt(
                                                    "drawEntityCountSize",
                                                    32,
                                                    0,
                                                    0x7FFFFFFF,
                                                    0,
                                                    "How big to draw the entity count graph");
    drawServerBandwidth = _Dvar_RegisterBool("drawServerBandwidth", 0, 0, "Enable drawing server bandwidth");
    drawServerBandwidthPos = _Dvar_RegisterVec2(
                                                         "drawServerBandwidthPos",
                                                         (-55.0),
                                                         (-280.0),
                                                         -3.4028235e38,
                                                         3.4028235e38,
                                                         0,
                                                         "Where to draw the server bandwidth graph");
    drawServerBandwidthSize = _Dvar_RegisterInt(
                                                            "drawEntityCountSize",
                                                            32,
                                                            0,
                                                            0x7FFFFFFF,
                                                            0,
                                                            "How big to draw the entity count graph");
    drawKillcamData = _Dvar_RegisterBool("drawKillcamData", 0, 0, "Enable drawing server killcam data");
    drawKillcamDataPos = _Dvar_RegisterVec2(
                                                 "drawKillcamDataPos",
                                                 (-55.0),
                                                 (-230.0),
                                                 -3.4028235e38,
                                                 3.4028235e38,
                                                 0,
                                                 "Where to draw the server killcam graph");
    drawKillcamDataSize = _Dvar_RegisterInt(
                                                    "drawKillcamDataSize",
                                                    32,
                                                    0,
                                                    0x7FFFFFFF,
                                                    0,
                                                    "How big to draw the killcam data graph");
    cg_hudProneY = _Dvar_RegisterFloat(
                                     "cg_hudProneY",
                                     -160.0,
                                     -10000.0,
                                     10000.0,
                                     1u,
                                     "Virtual screen y coordinate of the prone blocked message");
    cg_mapLocationSelectionCursorSpeed = _Dvar_RegisterFloat(
                                                                                 "cg_mapLocationSelectionCursorSpeed",
                                                                                 0.60000002,
                                                                                 0.001,
                                                                                 1.0,
                                                                                 1u,
                                                                                 "Speed of the cursor when selecting a location on the map");
    cg_mapLocationSelectionRotationSpeed = _Dvar_RegisterInt(
                                                                                     "cg_mapLocationSelectionRotationSpeed",
                                                                                     3,
                                                                                     1,
                                                                                     10,
                                                                                     1u,
                                                                                     "Rotation speed of the cursor when selecting a location on the map");
    cg_hudGrenadeIndicatorFadeUp = _Dvar_RegisterBool(
                                                                     "cg_hudGrenadeIndicatorFadeUp",
                                                                     0,
                                                                     0,
                                                                     "Draw grenade indicator with distance fade(COD3 style)");
    cg_hudGrenadeIndicatorTargetColor = _Dvar_RegisterVec4(
                                                                                "cg_hudGrenadeIndicatorTargetColor",
                                                                                (1.0),
                                                                                (1.0),
                                                                                (1.0),
                                                                                (1.0),
                                                                                0.0,
                                                                                1.0,
                                                                                0,
                                                                                "");
    cg_hudGrenadeIndicatorStartColor = _Dvar_RegisterVec4(
                                                                             "cg_hudGrenadeIndicatorStartColor",
                                                                             (1.0),
                                                                             (1.0),
                                                                             (1.0),
                                                                             (1.0),
                                                                             0.0,
                                                                             1.0,
                                                                             0,
                                                                             "");
    cg_weaponCycleDelay = _Dvar_RegisterInt(
                                                    "cg_weaponCycleDelay",
                                                    0,
                                                    0,
                                                    0x7FFFFFFF,
                                                    1u,
                                                    "The delay after cycling to a new weapon to prevent holding down the cycle weapon button from cycling too fast");
    cg_crosshairAlpha = _Dvar_RegisterFloat("cg_crosshairAlpha", 1.0, 0.0, 1.0, 0x81u, "The alpha value of the crosshair");
    cg_crosshairAlphaMin = _Dvar_RegisterFloat(
                                                     "cg_crosshairAlphaMin",
                                                     0.5,
                                                     0.0,
                                                     1.0,
                                                     0x81u,
                                                     "The minimum alpha value of the crosshair when it fades in");
    cg_crosshairDynamic = _Dvar_RegisterBool("cg_crosshairDynamic", 0, 0x81u, "Crosshair is Dynamic");
    cg_crosshairEnemyColor = _Dvar_RegisterBool(
                                                         "cg_crosshairEnemyColor",
                                                         1,
                                                         0x81u,
                                                         "The crosshair color when over an enemy");
    cg_brass = _Dvar_RegisterBool("cg_brass", 1, 1u, "Weapons eject brass");
    cg_gun_fovcomp_x = _Dvar_RegisterFloat(
                                             "cg_gun_fovcomp_x",
                                             -2.0,
                                             -3.4028235e38,
                                             3.4028235e38,
                                             0,
                                             "x position FOV offset compensation of the viewmodel");
    cg_gun_fovcomp_y = _Dvar_RegisterFloat(
                                             "cg_gun_fovcomp_y",
                                             0.0,
                                             -3.4028235e38,
                                             3.4028235e38,
                                             0,
                                             "y position FOV offset compensation of the viewmodel");
    cg_gun_fovcomp_z = _Dvar_RegisterFloat(
                                             "cg_gun_fovcomp_z",
                                             0.0,
                                             -3.4028235e38,
                                             3.4028235e38,
                                             0,
                                             "z position FOV offset compensation of the viewmodel");
    cg_gun_x = _Dvar_RegisterFloat("cg_gun_x", 0.0, -3.4028235e38, 3.4028235e38, 0x80u, "x position of the viewmodel");
    cg_gun_y = _Dvar_RegisterFloat("cg_gun_y", 0.0, -3.4028235e38, 3.4028235e38, 0x80u, "y position of the viewmodel");
    cg_gun_z = _Dvar_RegisterFloat("cg_gun_z", 0.0, -3.4028235e38, 3.4028235e38, 0x80u, "z position of the viewmodel");
    cg_gun_move_f = _Dvar_RegisterFloat(
                                        "cg_gun_move_f",
                                        0.0,
                                        -3.4028235e38,
                                        3.4028235e38,
                                        0x80u,
                                        "Weapon movement forward due to player movement");
    cg_gun_move_r = _Dvar_RegisterFloat(
                                        "cg_gun_move_r",
                                        0.0,
                                        -3.4028235e38,
                                        3.4028235e38,
                                        0x80u,
                                        "Weapon movement right due to player movement");
    cg_gun_move_u = _Dvar_RegisterFloat(
                                        "cg_gun_move_u",
                                        0.0,
                                        -3.4028235e38,
                                        3.4028235e38,
                                        0x80u,
                                        "Weapon movement up due to player movement");
    cg_gun_ofs_f = _Dvar_RegisterFloat(
                                     "cg_gun_ofs_f",
                                     0.0,
                                     -3.4028235e38,
                                     3.4028235e38,
                                     0x80u,
                                     "Forward weapon offset when prone/ducked");
    cg_gun_ofs_r = _Dvar_RegisterFloat(
                                     "cg_gun_ofs_r",
                                     0.0,
                                     -3.4028235e38,
                                     3.4028235e38,
                                     0x80u,
                                     "Right weapon offset when prone/ducked");
    cg_gun_ofs_u = _Dvar_RegisterFloat(
                                     "cg_gun_ofs_u",
                                     0.0,
                                     -3.4028235e38,
                                     3.4028235e38,
                                     0x80u,
                                     "Up weapon offset when prone/ducked");
    cg_gun_move_rate = _Dvar_RegisterFloat(
                                             "cg_gun_move_rate",
                                             0.0,
                                             -3.4028235e38,
                                             3.4028235e38,
                                             0x80u,
                                             "The base weapon movement rate");
    cg_gun_move_minspeed = _Dvar_RegisterFloat(
                                                     "cg_gun_move_minspeed",
                                                     0.0,
                                                     -3.4028235e38,
                                                     3.4028235e38,
                                                     0x80u,
                                                     "The minimum weapon movement rate");
    cg_centertime = _Dvar_RegisterFloat(
                                        "cg_centertime",
                                        5.0,
                                        0.0,
                                        3.4028235e38,
                                        0x80u,
                                        "The time for a center printed message to fade");
    cg_debugPosition = _Dvar_RegisterBool("cg_debugposition", 0, 0x80u, "Output position debugging information");
    cg_debugEvents = _Dvar_RegisterBool("cg_debugevents", 0, 0x80u, "Output event debug information");
    cg_errorDecay = _Dvar_RegisterFloat("cg_errordecay", 100.0, 0.0, 3.4028235e38, 0, "Decay for predicted error");
    cg_nopredict = _Dvar_RegisterBool("cg_nopredict", 0, 0, "Don't do client side prediction");
    cg_showmiss = _Dvar_RegisterInt("cg_showmiss", 0, 0, 2, 0, "Show prediction errors");
    cg_footsteps = _Dvar_RegisterBool("cg_footsteps", 1, 0x80u, "Play footstep sounds");
    cg_footprints = _Dvar_RegisterBool("cg_footprints", 1, 0x80u, "Draw footprint decals and effects");
    cg_footprintsDistortWater = _Dvar_RegisterInt(
                                                                "cg_footprintsDistortWater",
                                                                0,
                                                                0,
                                                                1,
                                                                0x80u,
                                                                "Distort water on footprint (0 means no distortion)");
    cg_footprintsDebug = _Dvar_RegisterInt(
                                                 "cg_footprintsDebug",
                                                 0,
                                                 0,
                                                 1,
                                                 0x80u,
                                                 "Debug footprint drawing code (0 means no debugging)");
    cg_waterTrailRippleFrequency = _Dvar_RegisterInt(
                                                                     "cg_waterTrailRippleFrequency",
                                                                     400,
                                                                     0,
                                                                     0x7FFFFFFF,
                                                                     0,
                                                                     "How often (in ms) will play the waist ripple fx for actors in water");
    cg_waterTrailRippleVariance = _Dvar_RegisterInt(
                                                                    "cg_waterTrailRippleVariance",
                                                                    200,
                                                                    0,
                                                                    0x7FFFFFFF,
                                                                    0,
                                                                    "How late (in ms) the waist ripple fx can be played");
    cg_treadmarks = _Dvar_RegisterBool("cg_treadmarks", 0, 0x80u, "Draw treadmark decals and effects");
    cg_firstPersonTracerChance = _Dvar_RegisterFloat(
                                                                 "cg_firstPersonTracerChance",
                                                                 0.0,
                                                                 0.0,
                                                                 1.0,
                                                                 0x80u,
                                                                 "The probability that a bullet is a tracer round for your bullets");
    cg_laserForceOn = _Dvar_RegisterBool(
                                            "cg_laserForceOn",
                                            0,
                                            0x80u,
                                            "Force laser sights on in all possible places (for debug purposes).");
    cg_laserRange = _Dvar_RegisterFloat(
                                        "cg_laserRange",
                                        1500.0,
                                        1.0,
                                        3.4028235e38,
                                        0x80u,
                                        "The maximum range of a laser beam");
    cg_laserRangePlayer = _Dvar_RegisterFloat(
                                                    "cg_laserRangePlayer",
                                                    1500.0,
                                                    1.0,
                                                    3.4028235e38,
                                                    0x80u,
                                                    "The maximum range of the player's laser beam");
    cg_laserRadius = _Dvar_RegisterFloat(
                                         "cg_laserRadius",
                                         0.80000001,
                                         0.001,
                                         3.4028235e38,
                                         0x80u,
                                         "The size (radius) of a laser beam");
    cg_laserLight = _Dvar_RegisterBool(
                                        "cg_laserLight",
                                        1,
                                        0,
                                        "Whether to draw the light emitted from a laser (not the laser itself)");
    cg_laserLightBodyTweak = _Dvar_RegisterFloat(
                                                         "cg_laserLightBodyTweak",
                                                         15.0,
                                                         -3.4028235e38,
                                                         3.4028235e38,
                                                         0x80u,
                                                         "Amount to add to length of beam for light when laser hits a body (for hitboxes).");
    cg_laserLightRadius = _Dvar_RegisterFloat(
                                                    "cg_laserLightRadius",
                                                    3.0,
                                                    0.001,
                                                    3.4028235e38,
                                                    0x80u,
                                                    "The radius of the light at the far end of a laser beam");
    cg_laserLightBeginOffset = _Dvar_RegisterFloat(
                                                             "cg_laserLightBeginOffset",
                                                             13.0,
                                                             -3.4028235e38,
                                                             3.4028235e38,
                                                             0x80u,
                                                             "How far from the true beginning of the beam the light at the beginning is.");
    cg_laserLightEndOffset = _Dvar_RegisterFloat(
                                                         "cg_laserLightEndOffset",
                                                         -3.0,
                                                         -3.4028235e38,
                                                         3.4028235e38,
                                                         0x80u,
                                                         "How far from the true end of the beam the light at the end is.");
    cg_laserEndOffset = _Dvar_RegisterFloat(
                                                "cg_laserEndOffset",
                                                0.5,
                                                -3.4028235e38,
                                                3.4028235e38,
                                                0x80u,
                                                "How far from the point of collision the end of the beam is.");
    cg_laserFlarePct = _Dvar_RegisterFloat(
                                             "cg_laserFlarePct",
                                             0.2,
                                             0.0,
                                             3.4028235e38,
                                             0x80u,
                                             "Percentage laser widens over distance from viewer.");
    cg_marks_ents_player_only = _Dvar_RegisterBool(
                                                                "cg_marks_ents_player_only",
                                                                0,
                                                                1u,
                                                                "Marks on entities from players' bullets only.");
    cg_tracerChance = _Dvar_RegisterFloat(
                                            "cg_tracerchance",
                                            0.2,
                                            0.0,
                                            1.0,
                                            0x80u,
                                            "The probability that a bullet is a tracer round");
    cg_tracerWidth = _Dvar_RegisterFloat("cg_tracerwidth", 3.0, 0.0, 3.4028235e38, 0x80u, "The width of the tracer round");
    cg_tracerSpeed = _Dvar_RegisterFloat(
                                         "cg_tracerSpeed",
                                         7500.0,
                                         0.0,
                                         3.4028235e38,
                                         0x80u,
                                         "The speed of a tracer round in units per second");
    cg_tracerLength = _Dvar_RegisterFloat(
                                            "cg_tracerlength",
                                            100.0,
                                            0.0,
                                            3.4028235e38,
                                            0x80u,
                                            "The length of a tracer round");
    cg_tracerNoDrawTime = _Dvar_RegisterInt(
                                                    "cg_tracerNoDrawTime",
                                                    0,
                                                    0,
                                                    1000,
                                                    0x80u,
                                                    "Delay in milliseconds before a tracer will start rendering");
    cg_tracerScale = _Dvar_RegisterFloat(
                                         "cg_tracerScale",
                                         1.0,
                                         1.0,
                                         3.4028235e38,
                                         0x80u,
                                         "Scale the tracer at a distance, so it's still visible");
    cg_tracerScaleMinDist = _Dvar_RegisterFloat(
                                                        "cg_tracerScaleMinDist",
                                                        5000.0,
                                                        0.0,
                                                        3.4028235e38,
                                                        0x80u,
                                                        "The minimum distance to scale a tracer");
    cg_tracerScaleDistRange = _Dvar_RegisterFloat(
                                                            "cg_tracerScaleDistRange",
                                                            25000.0,
                                                            0.0,
                                                            3.4028235e38,
                                                            0x80u,
                                                            "The range at which a tracer is scaled to its maximum amount");
    cg_tracerScrewDist = _Dvar_RegisterFloat(
                                                 "cg_tracerScrewDist",
                                                 100.0,
                                                 0.0,
                                                 3.4028235e38,
                                                 0x80u,
                                                 "The length a tracer goes as it completes a full corkscrew revolution");
    cg_tracerScrewRadius = _Dvar_RegisterFloat(
                                                     "cg_tracerScrewRadius",
                                                     0.15000001,
                                                     0.0,
                                                     3.4028235e38,
                                                     0x80u,
                                                     "The radius of a tracer's corkscrew motion");
    cg_bulletWidth = _Dvar_RegisterFloat(
                                         "cg_bulletwidth",
                                         2.0,
                                         0.0,
                                         3.4028235e38,
                                         0x80u,
                                         "The width of the non-tracer round");
    cg_bulletLength = _Dvar_RegisterFloat(
                                            "cg_bulletlength",
                                            80.0,
                                            0.0,
                                            3.4028235e38,
                                            0x80u,
                                            "The length of a non-tracer round");
    cg_thirdPersonRange = _Dvar_RegisterFloat(
                                                    "cg_thirdPersonRange",
                                                    120.0,
                                                    0.0,
                                                    1024.0,
                                                    1u,
                                                    "The range of the camera from the player in third person view");
    cg_thirdPersonAngle = _Dvar_RegisterFloat(
                                                    "cg_thirdPersonAngle",
                                                    0.0,
                                                    -180.0,
                                                    360.0,
                                                    1u,
                                                    "The angle of the camera from the player in third person view");
    cg_thirdPersonFocusDist = _Dvar_RegisterFloat(
                                                            "cg_thirdPersonFocusDist",
                                                            512.0,
                                                            0.0,
                                                            1024.0,
                                                            0x80u,
                                                            "The distance infront of the player to aim the 3rd person camera at");
    cg_thirdPerson = _Dvar_RegisterInt("cg_thirdPerson", 0, 0, 2, 0x80u, "Use third person view");
    cg_thirdPersonMode = _Dvar_RegisterEnum(
                                                 "cg_thirdPersonMode",
                                                 cg_thirdPersonModeNames,
                                                 1,
                                                 1u,
                                                 "How the camera behaves in third person");
    cg_chatTime = _Dvar_RegisterInt(
                                    "cg_chatTime",
                                    12000,
                                    0,
                                    60000,
                                    1u,
                                    "The amount of time that a chat message is visible");
    cg_chatHeight = _Dvar_RegisterInt("cg_chatHeight", 5, 0, 8, 1u, "The font height of a chat message");
    cg_predictItems = _Dvar_RegisterBool("cg_predictItems", 1, 3u, "Turn on client side prediction for item pickup");
    cg_spectateThirdPerson = _Dvar_RegisterBool(
                                                         "cg_spectateThirdPerson",
                                                         0,
                                                         0,
                                                         "Default player to thirdperson in spectate");
    cg_teamChatsOnly = _Dvar_RegisterBool("cg_teamChatsOnly", 0, 1u, "Allow chatting only on the same team");
    cg_use_colored_smoke = _Dvar_RegisterBool("cg_use_colored_smoke", 0, 0x80u, "Allow the use of colored smoke grenades");
    cg_fakefireWizbyChance = _Dvar_RegisterFloat(
                                                         "cg_fakefireWizbyChance",
                                                         0.2,
                                                         0.0,
                                                         1.0,
                                                         0x80u,
                                                         "The probability that a fake fire shot plays a wizby to local players round");
    cg_paused = _Dvar_RegisterInt("cl_paused", 0, 0, 1, 0x40u, "Pause the game");
    cg_drawpaused = _Dvar_RegisterBool("cg_drawpaused", 1, 0, "Draw paused screen");
    cg_synchronousClients = _Dvar_RegisterBool(
                                                        "g_synchronousClients",
                                                        0,
                                                        0x100u,
                                                        "Client is synchronized to the server - allows smooth demos");
    cg_debug_overlay_viewport = _Dvar_RegisterBool(
                                                                "cg_debug_overlay_viewport",
                                                                0,
                                                                0x80u,
                                                                "Remove the sniper overlay so you can check that the scissor window is correct.");
    cg_fs_debug = _Dvar_RegisterInt("fs_debug", 0, 0, 2, 0, "Output debugging information for the file system");
    cg_debugFace = _Dvar_RegisterBool("cg_debugFace", 0, 0x80u, "Turn on debug information for face");
    cg_dumpAnims = _Dvar_RegisterInt("cg_dumpAnims", -1, -1, 1535, 0x80u, "Output animation info for the given entity id");
#ifdef _DEBUG // LWSS ADD
    cg_developer = _Dvar_RegisterInt("developer", 1, 0, 2, 0, "Turn on Development systems");
#else
    cg_developer = _Dvar_RegisterInt("developer", 0, 0, 2, 0, "Turn on Development systems");
#endif
    cg_minicon = _Dvar_RegisterBool("con_minicon", 0, 1u, "Display the mini console on screen");
    cg_subtitles = _Dvar_RegisterBool("cg_subtitles", 1, 1u, "Show subtitles");
    cg_subtitleMinTime = _Dvar_RegisterFloat(
                                                 "cg_subtitleMinTime",
                                                 3.0,
                                                 0.0,
                                                 3.4028235e38,
                                                 1u,
                                                 "The minimum time that the subtitles are displayed on screen in seconds");
    cg_subtitleWidthStandard = _Dvar_RegisterInt(
                                                             "cg_subtitleWidthStandard",
                                                             520,
                                                             130,
                                                             1664,
                                                             1u,
                                                             "The width of the subtitles in non wide-screen");
    cg_subtitleWidthWidescreen = _Dvar_RegisterInt(
                                                                 "cg_subtitleWidthWidescreen",
                                                                 520,
                                                                 130,
                                                                 1664,
                                                                 1u,
                                                                 "The width of the subtitles in wide-screen ");
    cg_gameMessageWidth = _Dvar_RegisterInt(
                                                    "cg_gameMessageWidth",
                                                    455,
                                                    130,
                                                    1664,
                                                    1u,
                                                    "The maximum character width of the game messages");
    cg_gameBoldMessageWidth = _Dvar_RegisterInt(
                                                            "cg_gameBoldMessageWidth",
                                                            390,
                                                            130,
                                                            1664,
                                                            1u,
                                                            "The maximum character width of the bold game messages");
    cg_descriptiveText = _Dvar_RegisterBool("cg_descriptiveText", 1, 1u, "Draw descriptive spectator messages");
    cg_youInKillCamSize = _Dvar_RegisterFloat(
                                                    "cg_youInKillCamSize",
                                                    11.0,
                                                    0.0,
                                                    100.0,
                                                    1u,
                                                    "Size of the 'you' Icon in the kill cam");
    cg_scriptIconSize = _Dvar_RegisterFloat("cg_scriptIconSize", 5.0, 0.0, 100.0, 1u, "Size of Icons defined by script");
    cg_connectionIconSize = _Dvar_RegisterFloat(
                                                        "cg_connectionIconSize",
                                                        5.0,
                                                        0.0,
                                                        100.0,
                                                        1u,
                                                        "Size of the connection icon");
    cg_voiceIconSize = _Dvar_RegisterFloat("cg_voiceIconSize", 0.0, 0.0, 100.0, 1u, "Size of the 'voice' icon");
    cg_constantSizeHeadIcons = _Dvar_RegisterBool(
                                                             "cg_constantSizeHeadIcons",
                                                             0,
                                                             0x80u,
                                                             "Head icons are the same size regardless of distance from the player");
    cg_headIconMinScreenRadius = _Dvar_RegisterFloat(
                                                                 "cg_headIconMinScreenRadius",
                                                                 0.02,
                                                                 0.0,
                                                                 1.0,
                                                                 1u,
                                                                 "The minumum radius of a head icon on the screen");
    cg_overheadNamesMaxDist = _Dvar_RegisterFloat(
                                                            "cg_overheadNamesMaxDist",
                                                            10000.0,
                                                            0.0,
                                                            3.4028235e38,
                                                            0x80u,
                                                            "The maximum distance for showing friendly player names");
    cg_overheadNamesNearDist = _Dvar_RegisterFloat(
                                                             "cg_overheadNamesNearDist",
                                                             64.0,
                                                             0.0,
                                                             3.4028235e38,
                                                             0x80u,
                                                             "The near distance at which names are full size");
    cg_overheadNamesFarDist = _Dvar_RegisterFloat(
                                                            "cg_overheadNamesFarDist",
                                                            512.0,
                                                            0.0,
                                                            3.4028235e38,
                                                            0x80u,
                                                            "The far distance at which name sizes are scaled by cg_overheadNamesFarScale");
    cg_overheadNamesFarScale = _Dvar_RegisterFloat(
                                                             "cg_overheadNamesFarScale",
                                                             0.69999999,
                                                             0.0,
                                                             3.4028235e38,
                                                             0x80u,
                                                             "The amount to scale overhead name sizes at cg_overheadNamesFarDist");
    cg_overheadNamesSize = _Dvar_RegisterFloat(
                                                     "cg_overheadNamesSize",
                                                     0.5,
                                                     0.0,
                                                     100.0,
                                                     1u,
                                                     "The maximum size to show overhead names");
    cg_overheadIconSize = _Dvar_RegisterFloat(
                                                    "cg_overheadIconSize",
                                                    0.69999999,
                                                    0.0,
                                                    100.0,
                                                    1u,
                                                    "The maximum size to show overhead icons like 'rank'");
    cg_overheadRankSize = _Dvar_RegisterFloat(
                                                    "cg_overheadRankSize",
                                                    0.5,
                                                    0.0,
                                                    3.4028235e38,
                                                    1u,
                                                    "The size to show rank text");
    cg_overheadNamesGlow = _Dvar_RegisterColor(
                                                     "cg_overheadNamesGlow",
                                                     0.0,
                                                     0.0,
                                                     0.0,
                                                     1.0,
                                                     0x80u,
                                                     "Glow color for overhead names");
    cg_overheadNamesFont = _Dvar_RegisterInt(
                                                     "cg_overheadNamesFont",
                                                     2,
                                                     0,
                                                     6,
                                                     1u,
                                                     "Font for overhead names ( see menudefinition.h )");
    cg_drawFriendlyNames = _Dvar_RegisterBool("cg_drawFriendlyNames", 1, 0x80u, "Whether to show friendly names in game");
    cg_enemyNameFadeIn = _Dvar_RegisterInt(
                                                 "cg_enemyNameFadeIn",
                                                 250,
                                                 0,
                                                 0x7FFFFFFF,
                                                 0x80u,
                                                 "Time in milliseconds to fade in enemy names");
    cg_friendlyNameFadeIn = _Dvar_RegisterInt(
                                                        "cg_friendlyNameFadeIn",
                                                        0,
                                                        0,
                                                        0x7FFFFFFF,
                                                        0x80u,
                                                        "Time in milliseconds to fade in friendly names");
    cg_enemyNameFadeOut = _Dvar_RegisterInt(
                                                    "cg_enemyNameFadeOut",
                                                    250,
                                                    0,
                                                    0x7FFFFFFF,
                                                    0x80u,
                                                    "Time in milliseconds to fade out enemy names");
    cg_friendlyNameFadeOut = _Dvar_RegisterInt(
                                                         "cg_friendlyNameFadeOut",
                                                         1500,
                                                         0,
                                                         0x7FFFFFFF,
                                                         0x80u,
                                                         "Time in milliseconds to fade out friendly names");
    cg_drawThroughWalls = _Dvar_RegisterBool(
                                                    "cg_drawThroughWalls",
                                                    0,
                                                    0x80u,
                                                    "Whether to draw friendly names through walls or not");
    cg_playerHighlightTargetSize = _Dvar_RegisterFloat(
                                                                     "cg_playerHighlightTargetSize",
                                                                     750.0,
                                                                     0.0,
                                                                     3.4028235e38,
                                                                     0x80u,
                                                                     "Size of player target highlights.");
    cg_playerHighlightEnemyColor = _Dvar_RegisterColor(
                                                                     "cg_playerHighlightEnemyColor",
                                                                     1.0,
                                                                     0.1,
                                                                     0.1,
                                                                     1.0,
                                                                     0x80u,
                                                                     "Color of enemy player highlights.");
    cg_playerHighlightBrightness = _Dvar_RegisterFloat(
                                                                     "cg_playerHighlightBrightness",
                                                                     3.0,
                                                                     0.0,
                                                                     3.4028235e38,
                                                                     0x80u,
                                                                     "Brightness of highlights.");
    cg_playerHighlightMinFade = _Dvar_RegisterFloat(
                                                                "cg_playerHighlightMinFade",
                                                                0.40000001,
                                                                0.0,
                                                                1.0,
                                                                0x80u,
                                                                "The minimum fade for player highlight blinking.");
    cg_playerHighlightBlinkTime = _Dvar_RegisterInt(
                                                                    "cg_playerHighlightBlinkTime",
                                                                    1250,
                                                                    0,
                                                                    0x7FFFFFFF,
                                                                    0x80u,
                                                                    "The speed (in ms) at which the player highlights blink.");
    cg_corpseHighlightFadeTime = _Dvar_RegisterFloat(
                                                                 "cg_corpseHighlightFadeTime",
                                                                 2.0,
                                                                 0.0,
                                                                 3.4028235e38,
                                                                 0x80u,
                                                                 "Time (in seconds) that corpse highlights fade out");
    cg_cameraSpikeHighlightBrightness = _Dvar_RegisterFloat(
                                                                                "cg_cameraSpikeHighlightBrightness",
                                                                                5.0,
                                                                                0.0,
                                                                                3.4028235e38,
                                                                                0x80u,
                                                                                "Brightness of player highlights in the camera spike view");
    cg_cameraSpikeEnemyColor = _Dvar_RegisterColor(
                                                             "cg_cameraSpikeEnemyColor",
                                                             1.0,
                                                             0.0,
                                                             0.0,
                                                             1.0,
                                                             0x80u,
                                                             "Color of enemies in the camera spike view");
    cg_adsZScaleMax = _Dvar_RegisterFloat(
                                            "cg_adsZScaleMax",
                                            1.25,
                                            1.01,
                                            1.99,
                                            0x81u,
                                            "The scale factor for shrinky dinks");
    cg_infraredHighlightScale = _Dvar_RegisterFloat(
                                                                "cg_infraredHighlightScale",
                                                                200.0,
                                                                1.0,
                                                                1000.0,
                                                                0,
                                                                "Scale of the player highlight when using infrared scope");
    cg_infraredHighlightOffset = _Dvar_RegisterFloat(
                                                                 "cg_infraredHighlightOffset",
                                                                 0.75,
                                                                 0.0,
                                                                 1.0,
                                                                 0,
                                                                 "Offset to the player highlight when using infrared scope");
    cg_allow_mature = _Dvar_RegisterBool("cg_allow_mature", g_allowMature, 0x40u, "Controls Mature Content selectability");
    if ( g_allowMature )
        cg_mature = _Dvar_RegisterBool("cg_mature", 1, 1u, "Show Mature Content");
    else
        cg_mature = _Dvar_RegisterBool("cg_mature", 0, 0x40u, "Show Mature Content");
    cg_blood = _Dvar_RegisterBool("cg_blood", 1, 1u, "Show Blood");
    cg_invalidCmdHintDuration = _Dvar_RegisterInt(
                                                                "cg_invalidCmdHintDuration",
                                                                1800,
                                                                0,
                                                                0x7FFFFFFF,
                                                                1u,
                                                                "Duration of an invalid command hint");
    cg_invalidCmdHintBlinkInterval = _Dvar_RegisterInt(
                                                                         "cg_invalidCmdHintBlinkInterval",
                                                                         600,
                                                                         1,
                                                                         0x7FFFFFFF,
                                                                         1u,
                                                                         "Blink rate of an invalid command hint");
    cg_viewZSmoothingMin = _Dvar_RegisterFloat(
                                                     "cg_viewZSmoothingMin",
                                                     1.0,
                                                     0.0,
                                                     3.4028235e38,
                                                     1u,
                                                     "Threshhold for the minimum smoothing distance it must move to smooth");
    cg_viewZSmoothingMax = _Dvar_RegisterFloat(
                                                     "cg_viewZSmoothingMax",
                                                     16.0,
                                                     0.0,
                                                     3.4028235e38,
                                                     1u,
                                                     "Threshhold for the maximum smoothing distance we'll do");
    cg_viewZSmoothingTime = _Dvar_RegisterFloat(
                                                        "cg_viewZSmoothingTime",
                                                        0.1,
                                                        0.0,
                                                        3.4028235e38,
                                                        1u,
                                                        "Amount of time to spread the smoothing over");
    overrideNVGModelWithKnife = _Dvar_RegisterBool(
                                                                "overrideNVGModelWithKnife",
                                                                0,
                                                                0x1000u,
                                                                "When true, nightvision animations will attach the weapDef's knife model instead of the n"
                                                                "ight vision goggles.");
    cg_visionSetLerpMaxIncreasePerFrame = _Dvar_RegisterFloat(
                                                                                    "cg_visionSetLerpMaxIncreasePerFrame",
                                                                                    0.02,
                                                                                    0.0,
                                                                                    1.0,
                                                                                    0x81u,
                                                                                    "Maximum jump of customlerp between 2 frames, used for smoothing for flare visionset");
    cg_visionSetLerpMaxDecreasePerFrame = _Dvar_RegisterFloat(
                                                                                    "cg_visionSetLerpMaxDecreasePerFrame",
                                                                                    0.0099999998,
                                                                                    0.0,
                                                                                    1.0,
                                                                                    0x81u,
                                                                                    "Maximum jump of customlerp between 2 frames, used for smoothing for flare visionset");
    cg_flareVisionSetFadeDuration = _Dvar_RegisterInt(
                                                                        "cg_flareVisionSetFadeDuration",
                                                                        2000,
                                                                        0,
                                                                        0x7FFFFFFF,
                                                                        0x81u,
                                                                        "Duration of fade back to normal vision set when you look away from the flare");
    cg_turretBipodOffset = _Dvar_RegisterFloat(
                                                     "cg_turretBipodOffset",
                                                     17.0,
                                                     -50.0,
                                                     50.0,
                                                     0x80u,
                                                     "Offset bipod mount position on gun by this distance");
    cg_AllPlayerNamesVisible = _Dvar_RegisterBool(
                                                             "cg_allPlayerNamesVisible",
                                                             0,
                                                             0x80u,
                                                             "When true all names are visible within visibility range.");
    cg_ScoresColor_MyTeam = _Dvar_RegisterColor(
                                                        "g_ScoresColor_MyTeam",
                                                        0.25,
                                                        0.72000003,
                                                        0.25,
                                                        1.0,
                                                        0x100u,
                                                        "Player team color on scoreboard");
    cg_ScoresColor_EnemyTeam = _Dvar_RegisterColor(
                                                             "g_ScoresColor_EnemyTeam",
                                                             0.69,
                                                             0.07,
                                                             0.050000001,
                                                             1.0,
                                                             0x100u,
                                                             "Enemy team color on scoreboard");
    cg_ScoresColor_Spectator = _Dvar_RegisterColor(
                                                             "g_ScoresColor_Spectator",
                                                             0.25,
                                                             0.25,
                                                             0.25,
                                                             1.0,
                                                             0x100u,
                                                             "Spectator team color on scoreboard");
    cg_ScoresColor_Free = _Dvar_RegisterColor(
                                                    "g_ScoresColor_Free",
                                                    0.75999999,
                                                    0.77999997,
                                                    0.1,
                                                    1.0,
                                                    0x100u,
                                                    "Free Team color on scoreboard");
    cg_ScoresColor_Allies = _Dvar_RegisterColor(
                                                        "g_ScoresColor_Allies",
                                                        0.090000004,
                                                        0.46000001,
                                                        0.07,
                                                        1.0,
                                                        0x100u,
                                                        "Allies team color on scoreboard");
    cg_ScoresColor_Axis = _Dvar_RegisterColor(
                                                    "g_ScoresColor_Axis",
                                                    0.69,
                                                    0.07,
                                                    0.050000001,
                                                    1.0,
                                                    0x100u,
                                                    "Axis team color on scoreboard");
    cg_TeamName_Allies = _Dvar_RegisterString("g_TeamName_Allies", "GAME_ALLIES", 0x100u, "Allied team name");
    cg_TeamName_Axis = _Dvar_RegisterString("g_TeamName_Axis", "GAME_AXIS", 0x100u, "Axis team name");
    cg_TeamColor_Allies = _Dvar_RegisterColor(
                                                    "g_TeamColor_Allies",
                                                    0.60000002,
                                                    0.63999999,
                                                    0.69,
                                                    1.0,
                                                    0x100u,
                                                    "Allies team color");
    cg_TeamColor_Axis = _Dvar_RegisterColor(
                                                "g_TeamColor_Axis",
                                                0.64999998,
                                                0.56999999,
                                                0.41,
                                                1.0,
                                                0x100u,
                                                "Axis team color");
    cg_TeamColor_MyTeam = _Dvar_RegisterColor(
                                                    "g_TeamColor_MyTeam",
                                                    0.40000001,
                                                    0.69999999,
                                                    0.40000001,
                                                    1.0,
                                                    0x100u,
                                                    "Player team color");
    cg_TeamColor_EnemyTeam = _Dvar_RegisterColor(
                                                         "g_TeamColor_EnemyTeam",
                                                         1.0,
                                                         0.315,
                                                         0.34999999,
                                                         1.0,
                                                         0x100u,
                                                         "Enemy team color");
    cg_TeamColor_MyTeamAlt = _Dvar_RegisterColor(
                                                         "g_TeamColor_MyTeamAlt",
                                                         0.34999999,
                                                         1.0,
                                                         1.0,
                                                         1.0,
                                                         0x100u,
                                                         "Player team color");
    cg_TeamColor_EnemyTeamAlt = _Dvar_RegisterColor(
                                                                "g_TeamColor_EnemyTeamAlt",
                                                                1.0,
                                                                0.5,
                                                                0.0,
                                                                1.0,
                                                                0x100u,
                                                                "Enemy team color");
    cg_TeamColor_Squad = _Dvar_RegisterColor("g_TeamColor_Squad", 0.25, 0.25, 0.75, 1.0, 0x100u, "Squad color");
    cg_TeamColor_Spectator = _Dvar_RegisterColor(
                                                         "g_TeamColor_Spectator",
                                                         0.25,
                                                         0.25,
                                                         0.25,
                                                         1.0,
                                                         0x100u,
                                                         "Spectator team color");
    cg_TeamColor_Free = _Dvar_RegisterColor("g_TeamColor_Free", 0.75, 0.25, 0.25, 1.0, 0x100u, "Free Team color");
    cg_proneFeetCollisionHull = _Dvar_RegisterBool(
                                                                "cg_proneFeetCollisionHull",
                                                                1,
                                                                0x80u,
                                                                "Enables the use of the extra physics collision hulls on the feet while prone.");
    CG_ViewRegisterDvars();
    DynEntCl_RegisterDvars();
    CG_OffhandRegisterDvars();
    CG_CompassRegisterDvars();
    CG_SpikeAcousticRegisterDvars();
    CG_AmmoCounterRegisterDvars();
    CG_RegisterVisionSetsDvars();
    CG_RegisterScoreboardDvars();
    CG_HudElemRegisterDvars();
    GC_InitWeaponOptionsDvars();
    Turret_PlaceTurret_RegisterDvars();
    g_compassShowEnemies = _Dvar_RegisterBool(
                                                     "g_compassShowEnemies",
                                                     0,
                                                     0x80u,
                                                     "Whether enemies are visible on the compass at all times");
    BG_RegisterDvars();
    cg_drawWVisDebug = _Dvar_RegisterBool("cg_drawWVisDebug", 0, 0, "Display weapon visibility debug info");
    debugOverlay = _Dvar_RegisterEnum(
                                     "debugOverlay",
                                     debugOverlayNames,
                                     0,
                                     0,
                                     "Toggles the display of various debug info.");
    cg_motionblur_duration = _Dvar_RegisterInt(
                                                         "cg_motionblur_duration",
                                                         2500,
                                                         1,
                                                         10000,
                                                         0x81u,
                                                         "Sets radial motion blur duration");
    cg_motionblur_fadeout = _Dvar_RegisterInt(
                                                        "cg_motionblur_fadeout",
                                                        500,
                                                        1,
                                                        5000,
                                                        0x81u,
                                                        "Sets fade time for radial motion blur");
    cg_timedDamageDuration = _Dvar_RegisterInt(
                                                         "cg_timedDamageDuration",
                                                         500,
                                                         1,
                                                         5000,
                                                         0x81u,
                                                         "Sets the time to display a damage friendly indicator");
    cg_MinDownedPulseRate = _Dvar_RegisterFloat(
                                                        "cg_MinDownedPulseRate",
                                                        1.0,
                                                        1.0,
                                                        10.0,
                                                        0x81u,
                                                        "The amount of alpha to fade per second, at minimum for downed allies");
    cg_MaxDownedPulseRate = _Dvar_RegisterFloat(
                                                        "cg_MaxDownedPulseRate",
                                                        4.0,
                                                        1.0,
                                                        10.0,
                                                        0x81u,
                                                        "The amount of alpha to fade per second, at most for downed allies");
    cg_playerFrustumHalfHeight = _Dvar_RegisterFloat(
                                                                 "cg_playerFrustumHalfHeight",
                                                                 36.0,
                                                                 0.0,
                                                                 128.0,
                                                                 0x80u,
                                                                 "The radius used to calculate frustum target center for a player. Used for fast \"is on screen\" tests");
    cg_overheadNamesTagUpdateInterval = _Dvar_RegisterInt(
                                                                                "cg_overheadNamesTagUpdateInterval",
                                                                                1,
                                                                                1,
                                                                                5000,
                                                                                0x80u,
                                                                                "How often the friendly visibility head tag is updated for the on screen frustum check");
    cg_canSeeFriendlyFrustumUpdateInterval = _Dvar_RegisterInt(
                                                                                         "cg_canSeeFriendlyFrustumUpdateInterval",
                                                                                         250,
                                                                                         1,
                                                                                         5000,
                                                                                         0x80u,
                                                                                         "How often the head tag is updated for the overhead names");
    cg_canSeeFriendlyFrustumExpand = _Dvar_RegisterFloat(
                                                                         "cg_canSeeFriendlyFrustumExpand",
                                                                         -30.0,
                                                                         -1000.0,
                                                                         1000.0,
                                                                         0x80u,
                                                                         "The frustum expansion to determine if a friendly is on screen.    Positive is inwards.");
    cg_canSeeFriendlyFrustumMinDistance = _Dvar_RegisterFloat(
                                                                                    "cg_canSeeFriendlyFrustumMinDistance",
                                                                                    100.0,
                                                                                    0.0,
                                                                                    10000.0,
                                                                                    0x80u,
                                                                                    "If target is inside this distance frustum culling is not applied.");
    cg_watersheeting = _Dvar_RegisterBool(
                                             "cg_watersheeting",
                                             1,
                                             0x80u,
                                             "Enables/disables the watersheeting fullscreen effect");
    cg_debug_triggers = _Dvar_RegisterBool(
                                                "cg_debug_triggers",
                                                0,
                                                0x4000u,
                                                "Debug client side Triggers, prints out all the client triggers the first time they are hit.");
    cg_cameraWaterClip = _Dvar_RegisterFloat(
                                                 "cg_cameraWaterClip",
                                                 4.5,
                                                 -100.0,
                                                 100.0,
                                                 0x1080u,
                                                 "Min distance between camera and water surface. To prevent camera seeing water edge-on. Set to -1 to disable");
    cg_cameraVehicleExitTweenTime = _Dvar_RegisterFloat(
                                                                        "cg_cameraVehicleExitTweenTime",
                                                                        0.40000001,
                                                                        0.0,
                                                                        5.0,
                                                                        0x1000u,
                                                                        "Time(secs) to tween from gunner/vehicle camera to normal player camera");
    cg_vehicle_piece_damagesfx_threshold = _Dvar_RegisterFloat(
                                                                                     "vehicle_piece_damagesfx_threshold",
                                                                                     10.0,
                                                                                     0.0,
                                                                                     3.4028235e38,
                                                                                     0x80u,
                                                                                     "Minimum amount of damage for which a destructible piece damageSound SFX will be played.");
    cg_debugLocHit = _Dvar_RegisterInt(
                                         "g_debugLocHit",
                                         0,
                                         0,
                                         2,
                                         0x80u,
                                         "Display locational damage info for an entity when the entity is hit");
    cg_debugLocHitTime = _Dvar_RegisterInt(
                                                 "g_debugLocHitTime",
                                                 500,
                                                 0,
                                                 0x7FFFFFFF,
                                                 0x80u,
                                                 "Time duration of g_debugLocHit lines");
    CG_SetupSplitscreenDvars();
}

int __cdecl CG_AllocateClientMemory_SizeRequired(int maxLocalClients)
{
    int localClientNum; // [esp+0h] [ebp-8h]
    int size; // [esp+4h] [ebp-4h]

    size = 907268 * maxLocalClients;
    for ( localClientNum = 0; localClientNum < maxLocalClients; ++localClientNum )
        size += 1062080;
    return size;
}

void __cdecl CG_AllocateClientMemory(HunkUser *hunk, int maxLocalClients)
{
    int localClientNum; // [esp+0h] [ebp-4h]

    cgArray = (cg_s *)Hunk_UserAlloc(hunk, 466048 * maxLocalClients, 128, "cgArray");
    cgsArray = (cgs_t *)Hunk_UserAlloc(hunk, 12712 * maxLocalClients, 8, "cgsArray");
    cg_fakeEntitiesArray = (fake_centity_s *)Hunk_UserAlloc(hunk, 415744 * maxLocalClients, 4, "cg_fakeEntitiesArray");
    cg_viewModelArray = (ViewModelInfo *)Hunk_UserAlloc(hunk, 52 * maxLocalClients, 4, "cg_viewModelArray");
    for ( localClientNum = 0; localClientNum < maxLocalClients; ++localClientNum )
    {
        cg_weaponsArray[localClientNum] = (weaponInfo_s *)Hunk_UserAlloc(hunk, 73728, 4, "cg_weaponsArray");
        cg_entitiesArray[localClientNum] = (centity_s *)Hunk_UserAlloc(hunk, 827392, 4, "cg_entitiesArray");
        cg_entityOriginArray[localClientNum] = (float (*)[3])Hunk_UserAlloc(hunk, 18432, 4, "cg_entityOriginArray");
        cg_destructibles[localClientNum] = (Destructible *)Hunk_UserAlloc(hunk, 24768, 4, "cg_destructibles");
        cg_ikBuf[localClientNum] = (unsigned __int8 *)Hunk_UserAlloc(hunk, 117760, 16, "ikStatesArray");
        memset(cg_ikBuf[localClientNum], 0, 0x1CC00u);
        IK_AllocateLocalClientMemory(cg_ikBuf[localClientNum], localClientNum);
    }
}

void __cdecl CG_FreeClientMemory(HunkUser *hunk, int maxLocalClients)
{
    int localClientNum; // [esp+0h] [ebp-4h]

    for ( localClientNum = maxLocalClients - 1; localClientNum >= 0; --localClientNum )
    {
        Hunk_UserFree(hunk, cg_ikBuf[localClientNum]);
        IK_AllocateLocalClientMemory(0, localClientNum);
        Hunk_UserFree(hunk, cg_destructibles[localClientNum]);
        Hunk_UserFree(hunk, cg_entityOriginArray[localClientNum]);
        Hunk_UserFree(hunk, cg_entitiesArray[localClientNum]);
        Hunk_UserFree(hunk, cg_weaponsArray[localClientNum]);
    }
    if ( hunk )
    {
        if ( (!cg_viewModelArray || !cg_fakeEntitiesArray || !cgsArray || !cgArray)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                        1226,
                        0,
                        "%s",
                        "cg_viewModelArray && cg_fakeEntitiesArray && cgsArray && cgArray") )
        {
            __debugbreak();
        }
        Hunk_UserFree(hunk, cg_viewModelArray);
        Hunk_UserFree(hunk, cg_fakeEntitiesArray);
        Hunk_UserFree(hunk, cgsArray);
        Hunk_UserFree(hunk, cgArray);
    }
    cgArray = 0;
    cgsArray = 0;
    cg_weaponsArray[0] = 0;
    cg_viewModelArray = 0;
    cg_entitiesArray[0] = 0;
    cg_entityOriginArray[0] = 0;
    cg_destructibles[0] = 0;
}

void __cdecl CG_GetDObjOrientation(int localClientNum, int dobjHandle, float (*axis)[3], float *origin)
{
    const cg_s *cgameGlob; // [esp+10h] [ebp-8h]
    const centity_s *cent; // [esp+14h] [ebp-4h]

    if ( (unsigned int)dobjHandle > 0x600
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    1254,
                    0,
                    "%s\n\t(dobjHandle) = %i",
                    "(dobjHandle >= 0 && dobjHandle < ((((1<<10) + 512)) + 1))",
                    dobjHandle) )
    {
        __debugbreak();
    }
    if ( !axis && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 1255, 0, "%s", "axis") )
        __debugbreak();
    if ( !origin
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 1256, 0, "%s", "origin") )
    {
        __debugbreak();
    }
    if ( dobjHandle >= 1536 )
    {
        if ( dobjHandle - 1536 >= 2048
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                        1265,
                        0,
                        "%s\n\t(dobjHandle) = %i",
                        "(dobjHandle >= (((1<<10) + 512)) && dobjHandle - (((1<<10) + 512)) < ( 1 << 11 ))",
                        dobjHandle) )
        {
            __debugbreak();
        }
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        AxisCopy(cgameGlob->viewModelAxis, axis);
        *origin = cgameGlob->viewModelAxis[3][0];
        origin[1] = cgameGlob->viewModelAxis[3][1];
        origin[2] = cgameGlob->viewModelAxis[3][2];
    }
    else
    {
        cent = CG_GetEntity(localClientNum, dobjHandle);
        AnglesToAxis(cent->pose.angles, axis);
        *origin = cent->pose.origin[0];
        origin[1] = cent->pose.origin[1];
        origin[2] = cent->pose.origin[2];
    }
}

const playerState_s *__cdecl CG_GetPredictedPlayerState(int localClientNum)
{
    return &CG_GetLocalClientGlobals(localClientNum)->predictedPlayerState;
}

void __cdecl CG_GameMessage(int localClientNum, const char *msg)
{
    CL_ConsolePrint(localClientNum, 2, msg, 0, cg_gameMessageWidth->current.integer, 0);
}

void __cdecl CG_BoldGameMessage(int localClientNum, const char *msg, int duration)
{
    CL_ConsolePrint(localClientNum, 3, msg, duration, cg_gameBoldMessageWidth->current.integer, 0);
}

bool __cdecl CG_IsVehicleMayhemGameType()
{
    const char *String; // eax
    const char *v1; // eax
    bool v3; // [esp+0h] [ebp-4h]

    String = Dvar_GetString("g_gametype");
    v3 = 1;
    if ( I_strcmp(String, "vdm") )
    {
        v1 = Dvar_GetString("g_gametype");
        if ( I_strcmp(v1, "vtdm") )
            return 0;
    }
    return v3;
}

void __cdecl CG_RegisterSounds()
{
    int type; // [esp+0h] [ebp-8Ch]
    char name[132]; // [esp+4h] [ebp-88h] BYREF

    CG_RegisterSurfaceTypeSounds("wpn_grenade_explode", cgMedia.grenadeExplodeSound);
    CG_RegisterSurfaceTypeSounds("wpn_rifle_grenade", cgMedia.rifleGrenadeSound);
    CG_RegisterSurfaceTypeSounds("wpn_rocket_explode", cgMedia.rocketExplodeSound);
    CG_RegisterSurfaceTypeSounds("wpn_rocket_explode_xtreme", cgMedia.rocketExplodeXtremeSound);
    CG_RegisterSurfaceTypeSounds("wpn_tank_shell", cgMedia.tankShellExplodeSound);
    CG_RegisterSurfaceTypeSounds("wpn_mortar_shell", cgMedia.mortarShellExplodeSound);
    CG_RegisterSurfaceTypeSounds("prj_bullet_impact_small", cgMedia.bulletHitSmallSound);
    CG_RegisterSurfaceTypeSounds("prj_bullet_impact_large", cgMedia.bulletHitLargeSound);
    CG_RegisterSurfaceTypeSounds("prj_bullet_impact_ap", cgMedia.bulletHitAPSound);
    CG_RegisterSurfaceTypeSounds("prj_bullet_impact_xtreme", cgMedia.bulletHitXTremeSound);
    CG_RegisterSurfaceTypeSounds("prj_bulletspray_small", cgMedia.shotgunHitSound);
    CG_RegisterSurfaceTypeSounds("prj_bolt_impact", cgMedia.boltHitSound);
    CG_RegisterSurfaceTypeSounds("prj_blade_impact", cgMedia.bladeHitSound);
    CG_RegisterSurfaceTypeSounds("prj_bullet_impact_small_exit", cgMedia.bulletExitSmallSound);
    CG_RegisterSurfaceTypeSounds("prj_bullet_impact_large_exit", cgMedia.bulletExitLargeSound);
    CG_RegisterSurfaceTypeSounds("prj_bullet_impact_ap_exit", cgMedia.bulletExitAPSound);
    CG_RegisterSurfaceTypeSounds("prj_bullet_impact_xtreme_exit", cgMedia.bulletExitXTremeSound);
    CG_RegisterSurfaceTypeSounds("prj_bulletspray_impact_small_exit", cgMedia.shotgunExitSound);
    CG_RegisterSurfaceTypeSounds("prj_bolt_impact_exit", cgMedia.boltExitSound);
    CG_RegisterImpactTypeSounds("prj_impact_veh_armor", cgMedia.weaponImpactsTankArmorSound);
    CG_RegisterImpactTypeSounds("prj_impact_veh_locomotion", cgMedia.weaponImpactsTankTreadSound);
    cgMedia.mantleSound = SND_FindAliasId("chr_launch_exert_npc");
    cgMedia.mantleSoundPlayer = SND_FindAliasId("chr_launch_exert_plr");
    cgMedia.dtpLaunchSoundPlayer = SND_FindAliasId("fly_dtp_launch_plr");
    cgMedia.dtpLaunchSound = SND_FindAliasId("fly_dtp_launch_npc");
    cgMedia.dtpCollideSoundPlayer = SND_FindAliasId("fly_dtp_collide_plr");
    cgMedia.dtpCollideSound = SND_FindAliasId("fly_dtp_collide_npc");
    for ( type = 0; type < 9; ++type )
    {
        sprintf(name, "fly_dtp_land_plr_%s", Dtp_SurfaceTypeNames[type]);
        cgMedia.dtpLandSoundPlayer[type] = SND_FindAliasId(name);
        sprintf(name, "fly_dtp_land_npc_%s", Dtp_SurfaceTypeNames[type]);
        cgMedia.dtpLandSound[type] = SND_FindAliasId(name);
        sprintf(cgMedia.dtpSlideLoopSoundPlayer[type], "fly_dtp_slide_loop_plr_%s", Dtp_SurfaceTypeNames[type]);
        sprintf(cgMedia.dtpSlideLoopSound[type], "fly_dtp_slide_loop_npc_%s", Dtp_SurfaceTypeNames[type]);
        sprintf(name, "fly_dtp_slide_stop_plr_%s", Dtp_SurfaceTypeNames[type]);
        cgMedia.dtpSlideStopSoundPlayer[type] = SND_FindAliasId(name);
        sprintf(name, "fly_dtp_slide_stop_npc_%s", Dtp_SurfaceTypeNames[type]);
        cgMedia.dtpSlideStopSound[type] = SND_FindAliasId(name);
        sprintf(name, "fly_pslide_loop_plr_%s", Dtp_SurfaceTypeNames[type]);
        cgMedia.playerSlidingStart_1p[type] = SND_FindAliasId(name);
        sprintf(name, "fly_pslide_loop_npc_%s", Dtp_SurfaceTypeNames[type]);
        cgMedia.playerSlidingStart_3p[type] = SND_FindAliasId(name);
        sprintf(name, "fly_pslide_stop_plr_%s", Dtp_SurfaceTypeNames[type]);
        cgMedia.playerSlidingStop_1p[type] = SND_FindAliasId(name);
        sprintf(name, "fly_pslide_stop_npc_%s", Dtp_SurfaceTypeNames[type]);
        cgMedia.playerSlidingStop_3p[type] = SND_FindAliasId(name);
    }
    cgMedia.playerSprintGasp = SND_FindAliasId("fly_sprint_gasp");
    cgMedia.underwaterWhizby = SND_FindAliasId("prj_whizbyuw");
    cgMedia.bulletWhizby = SND_FindAliasId("prj_whizby");
    cgMedia.bulletCrack = SND_FindAliasId("prj_crack");
    cgMedia.deathGurgle = SND_FindAliasId("fly_death_gurgle");
    cgMedia.meleeHit = SND_FindAliasId("wpn_melee_hit");
    cgMedia.meleeHitOther = SND_FindAliasId("wpn_melee_hit_other");
    cgMedia.meleeKnifeHit = SND_FindAliasId("wpn_melee_knife_hit_body");
    cgMedia.meleeKnifeHitOther = SND_FindAliasId("wpn_melee_knife_hit_other");
    cgMedia.meleeDogHit = SND_FindAliasId("chr_melee_dog_hit");
    cgMedia.meleeDogHitOther = SND_FindAliasId("chr_melee_dog_hit_other");
    cgMedia.nightVisionOn = SND_FindAliasId("fly_nightvision_on");
    cgMedia.nightVisionOff = SND_FindAliasId("fly_nightvision_off");
    cgMedia.playerHeartBeatSound = SND_FindAliasId("wpn_sniper_heartbeat");
    cgMedia.playerBreathInSound = SND_FindAliasId("wpn_sniper_breathin");
    cgMedia.playerBreathOutSound = SND_FindAliasId("wpn_sniper_breathout");
    cgMedia.playerBreathGaspSound = SND_FindAliasId("wpn_sniper_breathgasp");
    cgMedia.playerSwapOffhand = SND_FindAliasId("wpn_offhand_select");
}

void __cdecl CG_RegisterSurfaceTypeSounds(const char *pszType, unsigned int *sound)
{
    const char *v2; // eax
    int i; // [esp+0h] [ebp-10Ch]
    int ia; // [esp+0h] [ebp-10Ch]
    char szAliasName[260]; // [esp+4h] [ebp-108h] BYREF

    if ( !pszType
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 1340, 0, "%s", "pszType") )
    {
        __debugbreak();
    }
    if ( *pszType )
    {
        for ( ia = 0; ia < 31; ++ia )
        {
            v2 = Com_SurfaceTypeToName(ia);
            Com_sprintf(szAliasName, 0x100u, "%s_%s", pszType, v2);
            sound[ia] = SND_FindAliasId(szAliasName);
        }
    }
    else
    {
        Com_PrintError(9, "ERROR: no alias prefix defined\n");
        for ( i = 0; i < 31; ++i )
            sound[i] = 0;
    }
}

void __cdecl CG_RegisterImpactTypeSounds(const char *pszType, unsigned int *sound)
{
    const char *ImpactTypeName; // eax
    int i; // [esp+0h] [ebp-10Ch]
    int ia; // [esp+0h] [ebp-10Ch]
    char szAliasName[260]; // [esp+4h] [ebp-108h] BYREF

    if ( !pszType
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 1369, 0, "%s", "pszType") )
    {
        __debugbreak();
    }
    if ( *pszType )
    {
        for ( ia = 0; ia < 16; ++ia )
        {
            ImpactTypeName = BG_GetImpactTypeName(ia);
            Com_sprintf(szAliasName, 0x100u, "%s_%s", pszType, ImpactTypeName);
            sound[ia] = SND_FindAliasId(szAliasName);
        }
    }
    else
    {
        Com_PrintError(9, "ERROR: no alias prefix defined for %s\n", "CG_RegisterImpactTypeSounds");
        for ( i = 0; i < 16; ++i )
            sound[i] = 0;
    }
}

void __cdecl CG_PlayBattleChatter(
                int localClientNum,
                int entitynum,
                float *origin,
                unsigned int firstSoundAlias,
                int secondSoundAlias)
{
    float *SndOrigin; // [esp+8h] [ebp-Ch]
    int i; // [esp+Ch] [ebp-8h]
    int SndID; // [esp+10h] [ebp-4h]

    SndID = CG_PlaySoundWithHandle(localClientNum, entitynum, origin, 0, 0, 1.0, firstSoundAlias);
    if (secondSoundAlias)
    {
        for (i = 0; i < 8; ++i)
        {
            if (!cg_BattleChatters[i].WhichSoundIsPlaying)
            {
                cg_BattleChatters[i].CurrentPlayingSound = SndID;
                cg_BattleChatters[i].EntityNum = entitynum;
                cg_BattleChatters[i].WhichSoundIsPlaying = 1;
                cg_BattleChatters[i].LocalClientNum = localClientNum;
                cg_BattleChatters[i].SecondAlias = secondSoundAlias;
                SndOrigin = cg_BattleChatters[i].SndOrigin;
                *SndOrigin = *origin;
                SndOrigin[1] = origin[1];
                SndOrigin[2] = origin[2];
                return;
            }
        }
    }
}

void __cdecl CG_CheckBattleChatter()
{
    int i; // [esp+8h] [ebp-4h]

    for (i = 0; i < 8; ++i)
    {
        if (cg_BattleChatters[i].WhichSoundIsPlaying)
        {
            if (cg_BattleChatters[i].WhichSoundIsPlaying != 1 || SND_IsPlaying(cg_BattleChatters[i].CurrentPlayingSound))
            {
                if (!SND_IsPlaying(cg_BattleChatters[i].CurrentPlayingSound))
                    cg_BattleChatters[i].WhichSoundIsPlaying = 0;
            }
            else
            {
                cg_BattleChatters[i].CurrentPlayingSound = CG_PlaySoundWithHandle(
                    cg_BattleChatters[i].LocalClientNum,
                    cg_BattleChatters[i].EntityNum,
                    cg_BattleChatters[i].SndOrigin,
                    0,
                    0,
                    1.0,
                    cg_BattleChatters[i].SecondAlias);
                cg_BattleChatters[i].WhichSoundIsPlaying = 2;
            }
        }
    }
}

void __cdecl CG_RestartSmokeGrenades(int localClientNum)
{
    int j; // [esp+18h] [ebp-44h]
    int eventIndex; // [esp+1Ch] [ebp-40h]
    const cg_s *cgameGlob; // [esp+20h] [ebp-3Ch]
    const snapshot_s *nextSnap; // [esp+24h] [ebp-38h]
    const WeaponDef *weaponDef; // [esp+28h] [ebp-34h]
    const entityState_s *v6; // [esp+2Ch] [ebp-30h]
    const cgs_t *cgs; // [esp+30h] [ebp-2Ch]
    int i; // [esp+34h] [ebp-28h]
    int ia; // [esp+34h] [ebp-28h]
    float axis[3][3]; // [esp+38h] [ebp-24h] BYREF

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    Com_DPrintf(14, "Playing smoke grenades at time %i\n", cgameGlob->time);
    for ( i = 0; i < 27; ++i )
    {
        if ( !cgs->grenadeFx[i] )
            return;
        FX_KillEffectDef(localClientNum, cgs->grenadeFx[i]);
    }
    FX_RewindTo(localClientNum, cgameGlob->time);
    nextSnap = cgameGlob->nextSnap;
    if ( nextSnap )
    {
        for ( ia = 0; ia < nextSnap->numEntities; ++ia )
        {
            v6 = &nextSnap->entities[ia];
            weaponDef = BG_GetWeaponDef(nextSnap->entities[ia].weapon);
            if ( (nextSnap->entities[ia].lerp.eFlags & 0x4000) != 0
                && nextSnap->entities[ia].time2 >= cgameGlob->time
                && nextSnap->entities[ia].lerp.u.actor.actorNum <= cgameGlob->time
                && !nextSnap->entities[ia].eType )
            {
                eventIndex = ((unsigned __int8)nextSnap->entities[ia].eventSequence - 1) & 3;
                if ( v6->events[eventIndex] != 66
                    && v6->events[eventIndex] != 67
                    && (v6->events[eventIndex] < 0x39u || v6->events[eventIndex] > 0x3Eu)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                                1910,
                                0,
                                "es->events[eventIndex] not in [EV_GRENADE_EXPLODE, EV_CUSTOM_EXPLODE_NOMARKS]\n\t%i not in [%i, %i]",
                                v6->events[eventIndex],
                                57,
                                62) )
                {
                    __debugbreak();
                }
                ByteToDir(v6->eventParms[eventIndex], axis[0]);
                Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                Com_DPrintf(
                    14,
                    "Restarting smoke grenade at time %i at ( %f, %f, %f )\n",
                    nextSnap->entities[ia].lerp.u.actor.actorNum,
                    nextSnap->entities[ia].lerp.pos.trBase[0],
                    nextSnap->entities[ia].lerp.pos.trBase[1],
                    nextSnap->entities[ia].lerp.pos.trBase[2]);
                for ( j = 0; j < 27; ++j )
                {
                    if ( weaponDef->projExplosionEffect == cgs->grenadeFx[j] )
                        FX_PlayOrientedEffect(
                            localClientNum,
                            cgs->grenadeFx[j],
                            nextSnap->entities[ia].lerp.u.actor.actorNum,
                            nextSnap->entities[ia].lerp.pos.trBase,
                            axis);
                }
            }
        }
    }
}

void __cdecl CG_InitVote(int localClientNum)
{
    char *ConfigString; // eax
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    const char *v5; // eax
    cgs_t *cgs; // [esp+0h] [ebp-Ch]
    int time; // [esp+4h] [ebp-8h] BYREF
    int serverId; // [esp+8h] [ebp-4h] BYREF

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    cgs->voteTime = 0;
    ConfigString = CL_GetConfigString(0xFu);
    if ( sscanf(ConfigString, "%d %d", &time, &serverId) == 2 && serverId == cls.serverId )
        cgs->voteTime = time;
    v2 = CL_GetConfigString(0x11u);
    cgs->voteYes = atoi(v2);
    v3 = CL_GetConfigString(0x12u);
    cgs->voteNo = atoi(v3);
    v4 = CL_GetConfigString(0x10u);
    v5 = SEH_LocalizeTextMessage(v4, "vote string", LOCMSG_SAFE);
    I_strncpyz(cgs->voteString, v5, 256);
}

unsigned __int16 __cdecl CG_GetWeaponAttachBone(clientInfo_t *ci, weapType_t weapType, weapInventoryType_t invType)
{
    if ( weapType == WEAPTYPE_GRENADE )
    {
        if ( invType != WEAPINVENTORY_ITEM )
            return scr_const.tag_inhand;
    }
    else if ( ci->leftHandGun )
    {
        return SL_FindString(bg_weaponleftbone->current.string, SCRIPTINSTANCE_SERVER);
    }
    return SL_FindString(bg_weaponrightbone->current.string, SCRIPTINSTANCE_SERVER);
}

void __cdecl CGScr_LoadGameTypeScript(scriptInstance_t inst, const char *gametype, ScriptFunctions *functions)
{
    char filename[68]; // [esp+4h] [ebp-48h] BYREF

    if ( inst )
        Com_sprintf(filename, 64, "%sgametypes/%s", "clientscripts/mp/", gametype);
    else
        Com_sprintf(filename, 64, "%sgametypes/%s", "maps/mp/", gametype);
    CGScr_LoadScriptAndLabel(inst, filename, "main", functions);
}

char __cdecl CGScr_LoadScriptAndLabel(
                scriptInstance_t inst,
                const char *filename,
                const char *label,
                ScriptFunctions *functions)
{
    int func; // [esp+4h] [ebp-4h]

    if ( !cg_loadScripts || !cg_loadScripts->current.enabled )
        return 0;
    if ( functions->count < functions->maxSize )
    {
        if ( Scr_LoadScript(inst, (char*)filename) )
        {
            func = Scr_GetFunctionHandle(inst, filename, label);
            functions->address[functions->count++] = func;
            if ( func )
            {
                return 1;
            }
            else
            {
                Com_Printf(15, "Could not find label '%s' in script '%s'\n", label, filename);
                return 0;
            }
        }
        else
        {
            functions->address[functions->count++] = 0;
            Com_Printf(15, "Could not find script '%s'\n", filename);
            return 0;
        }
    }
    else
    {
        Com_PrintError(15, "CODE ERROR: GScr_LoadScriptAndLabel: functions->maxSize exceeded\n");
        return 0;
    }
}

void __cdecl CGScr_LoadScripts(const char *mapname, const char *gametype, ScriptFunctions *functions)
{
    Scr_BeginLoadScripts(SCRIPTINSTANCE_SERVER, 0);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_SERVER, "codescripts/delete", "main", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_SERVER, "codescripts/struct", "initstructs", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_SERVER, "codescripts/struct", "createstruct", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_SERVER, "codescripts/struct", "findstruct", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_SERVER, "maps/mp/_destructible", "CodeCallback_DestructibleEvent", functions);
    CGScr_LoadScriptAndLabel(
        SCRIPTINSTANCE_SERVER,
        "maps/mp/gametypes/_spawning",
        "CodeCallback_UpdateSpawnPoints",
        functions);
    CGScr_LoadGameTypeScript(SCRIPTINSTANCE_SERVER, gametype, functions);
    CGScr_LoadLevelScript(SCRIPTINSTANCE_SERVER, mapname, functions);
    G_ResetEntityParsePoint();
    Scr_PostCompileScripts(SCRIPTINSTANCE_SERVER);
    Scr_EndLoadScripts(SCRIPTINSTANCE_SERVER);
    Scr_PrecacheAnimTrees(SCRIPTINSTANCE_SERVER, (void *(__cdecl *)(int))Hunk_AllocXAnimCreate, 0, 1);
}

void __cdecl CGScr_LoadLevelScript(scriptInstance_t inst, const char *mapname, ScriptFunctions *functions)
{
    char filename[68]; // [esp+4h] [ebp-48h] BYREF

    if ( inst )
        Com_sprintf(filename, 64, "%s%s", "clientscripts/mp/", mapname);
    else
        Com_sprintf(filename, 64, "%s%s", "maps/mp/", mapname);
    CGScr_LoadScriptAndLabel(inst, filename, "main", functions);
}

void __cdecl CGScr_LoadClientScripts(const char *mapname, ScriptFunctions *functions)
{
    Scr_BeginLoadScripts(SCRIPTINSTANCE_CLIENT, 0);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "codescripts/delete", "main", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "codescripts/struct", "initstructs", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "codescripts/struct", "createstruct", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "codescripts/struct", "findstruct", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "statechange", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "maprestart", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "localclientconnect", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "localclientdisconnect", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "entityspawned", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "playerspawned", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "client_flag_callback", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "client_flagasval_callback", functions);
    CGScr_LoadScriptAndLabel(
        SCRIPTINSTANCE_CLIENT,
        "clientscripts/mp/_destructible",
        "CodeCallback_DestructibleEvent",
        functions);
    CGScr_LoadScriptAndLabel(
        SCRIPTINSTANCE_CLIENT,
        "clientscripts/mp/_callbacks",
        "CodeCallback_CreatingCorpse",
        functions);
    CGScr_LoadScriptAndLabel(
        SCRIPTINSTANCE_CLIENT,
        "clientscripts/mp/_callbacks",
        "CodeCallback_PlayerFootstep",
        functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "CodeCallback_PlayerJump", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "CodeCallback_PlayerLand", functions);
    CGScr_LoadScriptAndLabel(
        SCRIPTINSTANCE_CLIENT,
        "clientscripts/mp/_callbacks",
        "CodeCallback_PlayerFoliage",
        functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_dogs", "playDogstep", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "level_notify", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_dogs", "soundNotify", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "airsupport", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "demo_jump", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "demo_player_switch", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "entityshutdown_callback", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "glass_smash", functions);
    CGScr_LoadLevelScript(SCRIPTINSTANCE_CLIENT, mapname, functions);
    Scr_PostCompileScripts(SCRIPTINSTANCE_CLIENT);
    CScr_PostLoadScripts();
    Scr_PrecacheAnimTrees(SCRIPTINSTANCE_CLIENT, (void *(__cdecl *)(int))Hunk_AllocXAnimCreate, 0, 1);
}

void __cdecl CGScr_LoadClientScriptsAndAnims()
{
    int address[128]; // [esp+0h] [ebp-210h] BYREF
    ScriptFunctions functions; // [esp+200h] [ebp-10h] BYREF
    const char *mapname; // [esp+20Ch] [ebp-4h]

    functions.maxSize = 128;
    functions.count = 0;
    functions.address = address;
    mapname = Dvar_GetString("mapname");
    CGScr_LoadClientScripts(mapname, &functions);
    functions.maxSize = functions.count;
    functions.count = 0;
    CScr_SetClientScripts(&functions);
    if ( functions.maxSize != functions.count )
        Com_Error(ERR_DROP, "Script function count mismatch");
    Scr_EndLoadScripts(SCRIPTINSTANCE_CLIENT);
    Scr_EndLoadAnimTrees(SCRIPTINSTANCE_CLIENT);
}

void __cdecl CG_InitScreenDimensions(int localClientNum)
{
    cgs_t *LocalClientStaticGlobals; // eax

    LocalClientStaticGlobals = CG_GetLocalClientStaticGlobals(localClientNum);
    LocalClientStaticGlobals->viewX = 0;
    LocalClientStaticGlobals->viewY = 0;
    CL_GetScreenDimensions(
        &LocalClientStaticGlobals->viewWidth,
        &LocalClientStaticGlobals->viewHeight,
        &LocalClientStaticGlobals->viewAspect);
}

int __cdecl CG_GetClientNum(int localClientNum)
{
    return CG_GetLocalClientGlobals(localClientNum)->clientNum;
}

bool __cdecl CG_IsMature()
{
    if ( !cg_mature
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    2534,
                    0,
                    "%s\n\t(cg_mature) = %p",
                    "(cg_mature != 0)",
                    0) )
    {
        __debugbreak();
    }
    return cg_mature->current.enabled;
}

bool __cdecl CG_GetEntityOriginAngles(int localClientNum, int entityNum, float *origin, float *angles)
{
    centity_s *cent; // [esp+10h] [ebp-4h]

    cent = CG_GetEntity(localClientNum, entityNum);
    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 2544, 0, "%s", "cent") )
        __debugbreak();
    if ( ((*((unsigned int *)cent + 201) >> 1) & 1) == 0 )
        return 0;
    *origin = cent->pose.origin[0];
    origin[1] = cent->pose.origin[1];
    origin[2] = cent->pose.origin[2];
    *angles = cent->pose.angles[0];
    angles[1] = cent->pose.angles[1];
    angles[2] = cent->pose.angles[2];
    return 1;
}

unsigned __int16 __cdecl CG_GetVehicleTypeString(int clientNum, int entityNum)
{
    const vehicle_info_t *info; // [esp+8h] [ebp-14h]
    unsigned __int16 string; // [esp+Ch] [ebp-10h]
    centity_s *cent; // [esp+10h] [ebp-Ch]
    int localClientNum; // [esp+14h] [ebp-8h]
    int i; // [esp+18h] [ebp-4h]

    localClientNum = -1;
    for ( i = 0; i < 1; ++i )
    {
        if ( CG_GetLocalClientGlobals(i)->clientNum == clientNum )
        {
            localClientNum = i;
            break;
        }
    }
    if ( localClientNum < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    2573,
                    0,
                    "%s",
                    "localClientNum >= 0") )
    {
        __debugbreak();
    }
    cent = CG_GetEntity(localClientNum, entityNum);
    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 2576, 0, "%s", "cent") )
        __debugbreak();
    if ( ((*((unsigned int *)cent + 201) >> 1) & 1) == 0 )
        return 0;
    info = CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
    if ( !info )
        return 0;
    string = SL_FindString(info->animSet, SCRIPTINSTANCE_SERVER);
    if ( !string )
        return SL_GetString_(SCRIPTINSTANCE_SERVER, info->animSet, 0, 22);
    return string;
}

int __cdecl CachedTag_GetCachedTagPos(
                const centity_s *ent,
                cached_client_tag_t *cachedTag,
                unsigned int tagName,
                float *pos,
                int updateInterval,
                bool forceUpdate)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 2667, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->clientTagCache )
        return CachedTag_NoCache_GetTagPos(ent, tagName, pos);
    CachedTag_UpdateTagInternal(ent, cachedTag, tagName, updateInterval, forceUpdate);
    CachedTag_CalcTagPos(ent, cachedTag, pos);
    return cachedTag->time;
}

int __cdecl CachedTag_UpdateTagInternal(
                const centity_s *ent,
                cached_client_tag_t *cachedTag,
                unsigned int tagName,
                int updateInterval,
                bool forceUpdate)
{
    char *v6; // eax
    float pos[3]; // [esp+8h] [ebp-14h] BYREF
    DObj *obj; // [esp+14h] [ebp-8h]
    cg_s *cgameGlob; // [esp+18h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 2598, 0, "%s", "ent") )
        __debugbreak();
    cgameGlob = CG_GetLocalClientGlobals(ent->pose.localClientNum);
    if ( !forceUpdate && cgameGlob->time < updateInterval + cachedTag->time )
        return 0;
    obj = Com_GetClientDObj(ent->nextState.number, ent->pose.localClientNum);
    if ( obj )
    {
        if ( CG_DObjGetWorldTagPos(&ent->pose, obj, tagName, pos) )
        {
            cachedTag->lastLocalTagOrigin[0] = pos[0] - ent->pose.origin[0];
            cachedTag->lastLocalTagOrigin[1] = pos[1] - ent->pose.origin[1];
            cachedTag->lastLocalTagOrigin[2] = pos[2] - ent->pose.origin[2];
            cachedTag->time = cgameGlob->time;
            return 1;
        }
        else
        {
            v6 = SL_ConvertToString(tagName, SCRIPTINSTANCE_SERVER);
            Com_Error(ERR_DROP, "CachedTag_UpdateTagInternal: Cannot find tag [%s] on entity\n", v6);
            return 0;
        }
    }
    else
    {
        Com_Error(ERR_DROP, "CachedTag_UpdateTagInternal: Cannot find dobj on entity\n");
        return 0;
    }
}

void __cdecl CachedTag_CalcTagPos(const centity_s *ent, cached_client_tag_t *cachedTag, float *pos)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 2636, 0, "%s", "ent") )
        __debugbreak();
    *pos = ent->pose.origin[0] + cachedTag->lastLocalTagOrigin[0];
    pos[1] = ent->pose.origin[1] + cachedTag->lastLocalTagOrigin[1];
    pos[2] = ent->pose.origin[2] + cachedTag->lastLocalTagOrigin[2];
}

int __cdecl CachedTag_NoCache_GetTagPos(const centity_s *ent, unsigned int tagName, float *pos)
{
    char *v4; // eax
    DObj *dobj; // [esp+4h] [ebp-8h]

    dobj = Com_GetClientDObj(ent->nextState.number, ent->pose.localClientNum);
    if ( !dobj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 2646, 0, "%s", "dobj") )
        __debugbreak();
    if ( dobj )
    {
        if ( CG_DObjGetWorldTagPos(&ent->pose, dobj, tagName, pos) )
        {
            return CG_GetLocalClientGlobals(ent->pose.localClientNum)->time;
        }
        else
        {
            v4 = SL_ConvertToString(tagName, SCRIPTINSTANCE_SERVER);
            Com_Error(ERR_DROP, "CachedTag_NoCache_GetTagPos: Cannot find tag [%s] on entity\n", v4);
            return 0;
        }
    }
    else
    {
        *pos = ent->pose.origin[0];
        pos[1] = ent->pose.origin[1];
        pos[2] = ent->pose.origin[2];
        return 0;
    }
}

int __cdecl CachedTag_GetTagPos(
                const centity_s *ent,
                unsigned int tagName,
                float *pos,
                int updateInterval,
                bool forceUpdate)
{
    if ( scr_const.aim_vis_bone != scr_const.j_head
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    2686,
                    0,
                    "%s",
                    "scr_const.aim_vis_bone == scr_const.j_head") )
    {
        __debugbreak();
    }
    if ( ent->nextState.eType != 1 || !ent->clientTagCache )
        return CachedTag_NoCache_GetTagPos(ent, tagName, pos);
    if ( scr_const.aim_vis_bone == tagName )
        return CachedTag_GetCachedTagPos(ent, &ent->clientTagCache->aim_head_tag, tagName, pos, updateInterval, forceUpdate);
    if ( scr_const.aim_highest_bone == tagName )
        return CachedTag_GetCachedTagPos(
                         ent,
                         &ent->clientTagCache->aim_highest_tag,
                         tagName,
                         pos,
                         updateInterval,
                         forceUpdate);
    Com_Error(ERR_DROP, "CachedTag_GetTagPos: Called for a tag that it is not set up for\n");
    return CachedTag_GetCachedTagPos(ent, 0, tagName, pos, updateInterval, forceUpdate);
}

void __cdecl CG_InitClientEntityCaches(int localClientNum)
{
    AimTargetCache *aimTargetInfo; // edx
    centity_s *cent; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    for ( i = 0; i < com_maxclients->current.integer; ++i )
    {
        cent = CG_GetEntity(localClientNum, i);
        if ( !cent->clientTagCache )
        {
            cent->clientTagCache = (ClientTagCache *)MT_Alloc(96, 22, SCRIPTINSTANCE_SERVER);
            memset(cent->clientTagCache, 0, sizeof(ClientTagCache));
        }
        if ( !cent->aimTargetInfo )
        {
            cent->aimTargetInfo = (AimTargetCache *)MT_Alloc(8, 22, SCRIPTINSTANCE_SERVER);
            aimTargetInfo = cent->aimTargetInfo;
            aimTargetInfo->lastUpdateTime = 0;
            aimTargetInfo->targetHeight = 0.0;
        }
    }
}

void __cdecl CG_FreeClientEntityCaches(int localClientNum)
{
    centity_s *cent; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    for ( i = 0; i < com_maxclients->current.integer; ++i )
    {
        cent = CG_GetEntity(localClientNum, i);
        if ( cent->clientTagCache )
        {
            MT_Free((unsigned char*)cent->clientTagCache, 96, SCRIPTINSTANCE_SERVER);
            cent->clientTagCache = 0;
        }
        if ( cent->aimTargetInfo )
        {
            MT_Free((unsigned char *)cent->aimTargetInfo, 8, SCRIPTINSTANCE_SERVER);
            cent->aimTargetInfo = 0;
        }
    }
}

void __cdecl CG_Init(int localClientNum, int serverMessageNum, int serverCommandSequence, int clientNum)
{
    unsigned __int16 t; // [esp+1Ch] [ebp-64h]
    cg_s *cgameGlob; // [esp+20h] [ebp-60h]
    cgs_t *cgs; // [esp+28h] [ebp-58h]
    bool loaded_client_scripts; // [esp+33h] [ebp-4Dh]
    const char *s; // [esp+34h] [ebp-4Ch]
    char mapname[64]; // [esp+38h] [ebp-48h] BYREF
    bool loaded_server_scripts; // [esp+7Fh] [ebp-1h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    2851,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    CL_GetLocalClientConnection(localClientNum);
    destroy_client_gjkcc_info(localClientNum);

    memset(cgs, 0, sizeof(cgs_t));
    memset(cgameGlob, 0, sizeof(cg_s));
    memset(&cgDC[localClientNum], 0, sizeof(UiContext));
    memset(cg_entitiesArray[localClientNum], 0, 1024 * sizeof(centity_s)/*0xCA000u*/);
    memset(cg_weaponsArray[localClientNum], 0, 2048 * sizeof(weaponInfo_s) /*0x12000u*/);
    memset(&cg_viewModelArray[localClientNum], 0, sizeof(ViewModelInfo));
    memset(&cg_BattleChatters[0].WhichSoundIsPlaying, 0, sizeof(cg_BattleChatters));

    cgDC[localClientNum].contextIndex = localClientNum;

    CG_InitDestructibles(localClientNum);
    CG_ClearCompassPingData();
    CG_ClearOverheadFade();
    CG_ClearPlayerDetails();
    CG_InitDof(&cgameGlob->refdef.dof);
    CG_SetupSplitscreenDvars();
    Flame_Init();
    Ragdoll_Init();
    Phys_Init();
    IK_InitSystem();
    if ( !CG_HasClientSystemBeenInitialzed() )
    {
        SND_GameReset();
        CG_SndGameReset();
    }
    cgameGlob->localClientNum = localClientNum;
    cgameGlob->viewModelPose.eType = 21;
    cgameGlob->viewModelPose.localClientNum = localClientNum;

    iassert(cgameGlob->viewModelPose.localClientNum == localClientNum);

    CL_SetStance(localClientNum, CL_STANCE_STAND);
    CL_SetADS(localClientNum, 0);

    cgameGlob->objectiveText[0] = 0;
    cgameGlob->bgs.animData = &cg_bgsAnim;
    cgameGlob->bgs.animData->animScriptData.soundAlias = SND_FindAlias;
    cgameGlob->bgs.animData->animScriptData.playSoundAlias = CG_PlayAnimScriptSoundAlias;
    cgameGlob->bgs.GetXModel = FX_RegisterModel;
    cgameGlob->bgs.CreateDObj = CG_CreateDObj;
    cgameGlob->bgs.AttachWeapon = CG_AttachWeapon;
    cgameGlob->bgs.GetDObj = CG_GetDObj;
    cgameGlob->bgs.SafeDObjFree = Com_SafeClientDObjFree;
    cgameGlob->bgs.AllocXAnim = Hunk_AllocXAnimClient;
    cgameGlob->bgs.anim_user = 0;
    cgameGlob->bgs.Rand = CG_rand;
    cgameGlob->bgs.Random = CG_random;
    cgameGlob->clientNum = clientNum;
    cgameGlob->drawHud = 1;

    Dvar_SetBoolByName("r_grassEnable", 0);

    cgameGlob->cameraLinkedEntitiesCount = 0;
    cgameGlob->groundTiltEntNum = -1;
    cgameGlob->lastPlayerStateOverride = -1;
    cgameGlob->extraCamEntity = 1023;
    cgameGlob->lastHealthLerpDelay = 1;

    cgs->processedSnapshotNum = serverMessageNum;
    cgs->serverCommandSequence = serverCommandSequence;
    cgs->localServer = com_sv_running->current.color[0];

    CG_ParseServerInfo(localClientNum);
    CG_ParseCodInfo(localClientNum);
    R_BeginRemoteScreenUpdate();

    if ( !r_reflectionProbeGenerate->current.enabled )
        UI_LoadIngameMenus(localClientNum);

    UI_ClearLocalUIVisibilityBits(localClientNum);
    SCR_UpdateLoadScreen();
    cgMedia.whiteMaterial = Material_RegisterHandle("white", 7);
    cgMedia.smallDevFont = CL_RegisterFont("fonts/smallDevFont", 1);
    cgMedia.bigDevFont = CL_RegisterFont("fonts/bigDevFont", 1);
    Material_RegisterHandle("net_disconnect", 7);
    cgMedia.hudDpadLeftHighlight = Material_RegisterHandle("nightvision_overlay_goggles", 7);
    cgMedia.ammoCounterBullet = Material_RegisterHandle("hud_icon_nvg", 7);
    cgMedia.ammoCounterBeltBullet = Material_RegisterHandle("hud_dpad_arrow", 7);
    cgMedia.ammoCounterRifleBullet = Material_RegisterHandle("hud_dpad_eqip_count_backing", 7);
    cgMedia.ammoCounterRocket = Material_RegisterHandle("hud_dpad_outer_frame_highlight_side", 7);
    cgMedia.ammoCounterShotgunShell = Material_RegisterHandle("ammo_counter_bullet", 7);
    cgMedia.ammoCounterSingle = Material_RegisterHandle("ammo_counter_beltbullet", 7);
    cgMedia.lifeCounterAlive = Material_RegisterHandle("ammo_counter_riflebullet", 7);
    cgMedia.lifeCounterDead = Material_RegisterHandle("ammo_counter_rocket", 7);
    cgMedia.textDecodeCharacters = Material_RegisterHandle("ammo_counter_shotgunshell", 7);
    cgMedia.textDecodeCharactersGlow = Material_RegisterHandle("ammo_counter_single", 7);
    cgMedia.lifeCounterAlive = Material_RegisterHandle("life_counter_alive", 7);
    cgMedia.lifeCounterDead = Material_RegisterHandle("life_counter_dead", 7);
    cgMedia.textDecodeCharacters = Material_RegisterHandle("decode_characters", 7);
    cgMedia.textDecodeCharactersGlow = Material_RegisterHandle("decode_characters_glow", 7);

    Material_RegisterHandle("code_warning_soundcpu", 7);
    Material_RegisterHandle("code_warning_snapshotents", 7);
    Material_RegisterHandle("code_warning_maxeffects", 7);
    Material_RegisterHandle("code_warning_models", 7);
    Material_RegisterHandle("code_warning_file", 7);
    Material_RegisterHandle("code_warning_fps", 7);
    Material_RegisterHandle("code_warning_serverfps", 7);
    Material_RegisterHandle("code_warning_collision", 7);
    Material_RegisterHandle("killicondied", 7);
    Material_RegisterHandle("killiconcrush", 7);
    Material_RegisterHandle("killiconfalling", 7);
    Material_RegisterHandle("killiconsuicide", 7);
    Material_RegisterHandle("killiconheadshot", 7);
    Material_RegisterHandle("killiconmelee", 7);

    if ( cg_fs_debug->current.integer == 2 )
        Dvar_SetInt((dvar_s*)cg_fs_debug, 0);

    CG_AntiBurnInHUD_RegisterDvars();
    CG_InitConsoleCommands();
    CG_InitViewDimensions(localClientNum);
    s = CL_GetConfigString(2);
    if ( strcmp(s, "cod") )
        Com_Error(ERR_DROP, "Client/Server game mismatch: %s/%s", "cod", s);
    SCR_UpdateLoadScreen();
    if ( !com_sv_running
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 3023, 0, "%s", "com_sv_running") )
    {
        __debugbreak();
    }
    if ( !com_sv_running->current.enabled )
        Mantle_CreateAnims(Hunk_AllocXAnimClient);
    if ( !com_sv_running->current.enabled )
        Dog_CreateAnims(Hunk_AllocXAnimClient);
    VehAnim_Init();

    iassert(bgs == 0);
    
    bgs = &cgameGlob->bgs;

    if ( !bg_lastParsedWeaponIndex )
    {
        Com_SetWeaponInfoMemory(2);
        BG_ClearWeaponDef();
    }
    loaded_server_scripts = 0;
    loaded_client_scripts = 0;
    if ( !Scr_IsSystemInitied(SCRIPTINSTANCE_CLIENT) )
    {
        memset(&cg_bgsAnim, 0, sizeof(cg_bgsAnim));
        if ( !com_sv_running->current.enabled )
        {
            CGScr_LoadScriptsAndAnims();
            loaded_server_scripts = 1;
        }
        Scr_InitSystem(SCRIPTINSTANCE_CLIENT, 1);
        Scr_SetLoading(1, SCRIPTINSTANCE_CLIENT);
        Scr_AllocGameVariable(SCRIPTINSTANCE_CLIENT);
        CGScr_LoadClientScriptsAndAnims();
        Scr_SetLoading(0, SCRIPTINSTANCE_CLIENT);
        loaded_client_scripts = 1;
    }
    if ( !g_mapLoaded && !useFastFile->current.enabled )
    {
        CG_LoadingString(localClientNum, "sound aliases");
        //BLOPS_NULLSUB((jpeg_decompress_struct *)cgs->mapname);
    }
    CG_SetupWeaponDef();
    if ( !com_sv_running->current.enabled && !CG_HasClientSystemBeenInitialzed() )
    {
        bg_numVehicleInfos = 0;
        CG_Veh_Init();
    }
    CG_Veh_RegisterMaterials();
    if ( I_strnicmp(cgs->mapname, "maps/", 5)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    3090,
                    0,
                    "%s",
                    "!I_strnicmp( cgs->mapname, \"maps/\", 5 )") )
    {
        __debugbreak();
    }
    Com_StripExtension(&cgs->mapname[5], mapname);
    CG_LoadAnimTrees(localClientNum, cgameGlob, mapname, loaded_client_scripts);
    if ( loaded_server_scripts )
        Scr_FreeScripts(SCRIPTINSTANCE_SERVER, 1u);
    if ( !cgs->localServer )
        GScr_LoadConsts();
    CG_LoadingString(localClientNum, "collision map");
    CL_CM_LoadMap(cgs->mapname);
    Menu_Setup(&cgDC[localClientNum]);
    CG_LoadingString(localClientNum, "graphics");
    CG_ParsePlayerInfos();
    if ( !g_mapLoaded )
    {
        CG_LoadingString(localClientNum, cgs->mapname);
        LoadWorld(cgs->mapname);
        g_mapLoaded = 1;
    }
    CG_LoadingString(localClientNum, "game media");
    if ( !CG_HasClientSystemBeenInitialzed() )
    {
        R_Stream_ResetHintEntities();
        R_PerMap_Init();
        R_Stream_ResetHintEntities();
    }
    ProfLoad_Begin("Init effects system");
    FX_InitSystem(localClientNum);
    FX_RegisterDefaultEffect();
    ProfLoad_End();
    SCR_UpdateLoadScreen();
    CG_RegisterGraphics(localClientNum, mapname);
    CG_LoadingString(localClientNum, "clients");
    if ( !cls.vidConfig.isToolMode )
        GC_InitWeaponOptions();
    CG_SetupGameInformation(localClientNum);
    CG_LoadHudMenu(localClientNum);
    CG_SetGridTable();
    //BLOPS_NULLSUB((jpeg_decompress_struct *)localClientNum);
    CG_InitEntities(localClientNum);
    CG_InitLocalEntities(localClientNum);
    DynEntCl_InitEntities(localClientNum);
    CG_InitVisionSets(localClientNum);
    CG_InitExposure(localClientNum, mapname);
    CG_InitBolt(localClientNum);
    cgameGlob->isLoading = 0;
    CG_SetConfigValues(localClientNum);
    CG_LoadingString(localClientNum, "");
    CG_NorthDirectionChanged(localClientNum);
    CL_FinishLoadingModels();
    if ( !g_mapLoaded )
        SND_StopSounds(SND_STOP_ALL);
    CG_ParseFog(localClientNum);
    R_WaterSimulationRestart();
    R_InitPrimaryLights(cgameGlob->refdef.primaryLights);
    R_ClearShadowedPrimaryLightHistory(localClientNum);
    CL_SetADS(localClientNum, 0);
    AimTarget_Init(localClientNum);
    AimAssist_Init(localClientNum);
    CG_InitClientEntityCaches(localClientNum);
    CG_InitVote(localClientNum);
    Flame_Init_DVars();
    Flame_Init_FlameVars();
    Flame_InitDevGui();
    if ( cg_loadScripts && cg_loadScripts->current.enabled )
    {
        if ( CL_LocalClient_IsFirstActive(localClientNum) )
        {
            CScr_LoadStructs();
            CScr_LoadLevel();
            cg_fakeEntitiesInuseCountFromLoadScript = cg_fakeEntitiesInuseCount[localClientNum];
            if ( cg_fakeEntitiesInuseCountFromLoadScript > 412 )
                Com_Error(
                    ERR_DROP,
                    "To many local clientside entities used for the map: %i and script: %i.    Need to reserve %i for dynamic gameplay usage.\n",
                    cg_fakeEntitiesInuseCountFromLoadScript - cg_fakeEntitiesInuseCountFromMap,
                    cg_fakeEntitiesInuseCountFromMap,
                    100);
            Com_PrintWarning(
                16,
                "Fake Ents:    Using %i fake ents from the map and %i from script at level load.\n",
                cg_fakeEntitiesInuseCountFromLoadScript - cg_fakeEntitiesInuseCountFromMap,
                cg_fakeEntitiesInuseCountFromMap);
        }
        Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
        t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_data.localclientconnect, 1u);
        Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
    }
    BG_InitFire();

    iassert(bgs == &cgameGlob->bgs);
    
    bgs = NULL;
    R_EndRemoteScreenUpdate(0);
    CG_InitScoreboard();
}

clientConnection_t *__cdecl CL_GetLocalClientConnection(int localClientNum)
{
    if ( !clientConnections
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../client/client.h",
                    200,
                    0,
                    "%s",
                    "clientConnections") )
    {
        __debugbreak();
    }
    if ( localClientNum
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../client/client.h",
                    205,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum == 0)",
                    localClientNum) )
    {
        __debugbreak();
    }
    return clientConnections;
}

void __cdecl CG_RegisterGraphics(int localClientNum, const char *mapname)
{
    const FxEffectDef *v2; // eax
    const FxEffectDef *v3; // eax
    shellshock_parms_t *ShellshockParms; // eax
    const char *shellshock; // [esp+0h] [ebp-94h]
    const char *effectname; // [esp+4h] [ebp-90h]
    const char *modelName; // [esp+8h] [ebp-8Ch]
    const char *EffectNames[27]; // [esp+Ch] [ebp-88h]
    cgs_t *cgs; // [esp+80h] [ebp-14h]
    int i; // [esp+84h] [ebp-10h]
    const char *fireEffectNames[3]; // [esp+88h] [ebp-Ch]

    SCR_UpdateLoadScreen();
    CG_LoadingString(localClientNum, " - textures");
    cgMedia.lagometerMaterial = Material_RegisterHandle("lagometer", 7);
    cgMedia.connectionMaterial = Material_RegisterHandle("headicondisconnected", 7);
    cgMedia.youInKillCamMaterial = Material_RegisterHandle("headiconyouinkillcam", 7);
    Material_RegisterHandle("killiconmelee", 7);
    Material_RegisterHandle("killiconsuicide", 7);
    Material_RegisterHandle("killiconfalling", 7);
    Material_RegisterHandle("killiconcrush", 7);
    Material_RegisterHandle("killicondied", 7);
    cgMedia.redTracerMaterial = Material_RegisterHandle("gfx_red_tracer", 6);
    cgMedia.greenTracerMaterial = Material_RegisterHandle("gfx_green_tracer", 6);
    cgMedia.bulletMaterial = Material_RegisterHandle("gfx_bullet", 6);
    cgMedia.laserMaterial = Material_RegisterHandle("gfx_laser", 6);
    cgMedia.laserLightMaterial = Material_RegisterHandle("gfx_laser_light", 6);
    cgMedia.ropeMaterial = Material_RegisterHandle("rope", 6);
    cgMedia.hintMaterials[4] = Material_RegisterHandle("hint_health", 7);
    cgMedia.hintMaterials[5] = Material_RegisterHandle("hint_friendly", 7);
    cgMedia.stanceMaterials[0] = Material_RegisterHandle("stance_stand", 7);
    cgMedia.stanceMaterials[1] = Material_RegisterHandle("stance_crouch", 7);
    cgMedia.stanceMaterials[2] = Material_RegisterHandle("stance_prone", 7);
    cgMedia.stanceMaterials[4] = Material_RegisterHandle("stance_swim", 7);
    cgMedia.stanceMaterials[3] = Material_RegisterHandle("stance_flash", 7);
    cgMedia.objectiveMaterials[0] = Material_RegisterHandle("objective", 7);
    cgMedia.friendMaterials[0] = Material_RegisterHandle("compassping_friendly_mp", 7);
    cgMedia.friendMaterials[1] = Material_RegisterHandle("objective_friendly_chat", 7);
    cgMedia.friendMaterials[2] = Material_RegisterHandle("compass_waypoint_second_chance", 7);
    cgMedia.damageMaterial = Material_RegisterHandle("hit_direction", 7);
    cgMedia.mantleHint = Material_RegisterHandle("hint_mantle", 7);
    cgMedia.compassping_player = Material_RegisterHandle("compassping_player", 7);
    cgMedia.compassping_friendlyfiring = Material_RegisterHandle("compassping_friendlyfiring_mp", 7);
    cgMedia.compassping_friendlyyelling = Material_RegisterHandle("compassping_friendlyyelling_mp", 7);
    cgMedia.compassping_friendlyfakefire = Material_RegisterHandle("compassping_decoyfiring", 7);
    cgMedia.compassping_enemy = Material_RegisterHandle("compassping_enemy", 7);
    cgMedia.compassping_enemydirectional = Material_RegisterHandle("compassping_enemydirectional", 7);
    cgMedia.compassping_enemysatellite = Material_RegisterHandle("compassping_enemysatellite", 7);
    cgMedia.compassping_enemyfiring = Material_RegisterHandle("compassping_enemyfiring", 7);
    cgMedia.compassping_enemyyelling = Material_RegisterHandle("compassping_enemyyelling", 7);
    cgMedia.compassping_grenade = Material_RegisterHandle("compassping_grenade", 7);
    cgMedia.compassping_explosion = Material_RegisterHandle("compassping_explosion", 7);
    cgMedia.compassping_firstplace = Material_RegisterHandle("compassping_firstplace", 7);
    cgMedia.compass_radarline = Material_RegisterHandle("compass_radarline", 7);
    cgMedia.compass_acoustic_ping = Material_RegisterHandle("compass_acoustic_ping", 7);
    cgMedia.watch_face = Material_RegisterHandle("watch_face", 7);
    cgMedia.watch_hour = Material_RegisterHandle("watch_hour", 7);
    cgMedia.watch_minute = Material_RegisterHandle("watch_minute", 7);
    cgMedia.watch_second = Material_RegisterHandle("watch_second", 7);
    cgMedia.acoustic_ping = Material_RegisterHandle("acoustic_ping", 7);
    cgMedia.acoustic_wedge = Material_RegisterHandle("acoustic_wedge", 7);
    cgMedia.acoustic_grid = Material_RegisterHandle("acoustic_grid", 7);
    cgMedia.compass_scrambler_large = Material_RegisterHandle("compass_scrambler_large", 7);
    cgMedia.compass_mortar_selector = Material_RegisterHandle("waypoint_recon_artillery_strike", 7);
    cgMedia.compass_artillery_enemy = Material_RegisterHandle("compass_objpoint_flak_busy", 7);
    cgMedia.compass_artillery_friendly = Material_RegisterHandle("compass_objpoint_flak_friendly", 7);
    cgMedia.compass_mortar_enemy = Material_RegisterHandle("compass_objpoint_mortar_busy", 7);
    cgMedia.compass_mortar_friendly = Material_RegisterHandle("compass_objpoint_mortar_friendly", 7);
    cgMedia.compass_dogs_enemy = Material_RegisterHandle("compassping_dog", 7);
    cgMedia.compass_incoming_artillery = Material_RegisterHandle("waypoint_recon_artillery_strike", 7);
    cgMedia.compass_sentry_friendly = Material_RegisterHandle("compass_turret_green", 7);
    cgMedia.compass_sentry_friendly_firing = Material_RegisterHandle("compass_turret_green_fire", 7);
    cgMedia.compass_sentry_enemy = Material_RegisterHandle("compass_turret_red", 7);
    cgMedia.compass_sentry_enemy_firing = Material_RegisterHandle("compass_turret_red_fire", 7);
    cgMedia.compass_tow_turret_friendly = Material_RegisterHandle("compass_sam_turret_green", 7);
    cgMedia.compass_tow_turret_friendly_firing = Material_RegisterHandle("compass_sam_turret_green_fire", 7);
    cgMedia.compass_tow_turret_enemy = Material_RegisterHandle("compass_sam_turret_red", 7);
    cgMedia.compass_tow_turret_enemy_firing = Material_RegisterHandle("compass_sam_turret_red_fire", 7);
    cgMedia.compass_guided_missile = Material_RegisterHandle("compassping_player_missle", 7);
    cgMedia.grenadeIconFrag = Material_RegisterHandle("hud_grenadeicon", 7);
    cgMedia.grenadeIconFlash = Material_RegisterHandle("hud_flashbangicon", 7);
    cgMedia.grenadeIconThrowBack = Material_RegisterHandle("hud_grenadethrowback", 7);
    cgMedia.grenadePointer = Material_RegisterHandle("hud_grenadepointer", 7);
    cgMedia.offscreenObjectivePointer = Material_RegisterHandle("hud_offscreenobjectivepointer", 7);
    cgMedia.demoTimelineFaded = Material_RegisterHandle("demo_timeline_faded", 7);
    cgMedia.demoTimelineSolid = Material_RegisterHandle("demo_timeline_solid", 7);
    cgMedia.demoTimelineCursor = Material_RegisterHandle("demo_timeline_arrow", 7);
    cgMedia.demoTimelineBookmark = Material_RegisterHandle("demo_timeline_bookmark", 7);
    cgMedia.demoStatePaused = Material_RegisterHandle("demo_pause", 7);
    cgMedia.demoStatePlay = Material_RegisterHandle("demo_play", 7);
    cgMedia.demoStateStop = Material_RegisterHandle("demo_stop", 7);
    cgMedia.demoStateJump = Material_RegisterHandle("demo_step", 7);
    cgMedia.demoStateForwardFast = Material_RegisterHandle("demo_forward_fast", 7);
    cgMedia.demoStateForwardSlow = Material_RegisterHandle("demo_forward_slow", 7);
    cgMedia.theaterUpArrow = Material_RegisterHandle("theater_up_arrow", 7);
    cgMedia.theaterDownArrow = Material_RegisterHandle("theater_down_arrow", 7);
    cgMedia.theaterLeftArrow = Material_RegisterHandle("theater_left_arrow", 7);
    cgMedia.theaterRightArrow = Material_RegisterHandle("theater_right_arrow", 7);
    cgMedia.teamStatusBar = Material_RegisterHandle("hudcolorbar", 7);
    CG_LoadingString(localClientNum, " - models");
    cgMedia.afkLightbulb = Material_RegisterHandle("headicontalkballoon", 7);
    CG_RegisterScoreboardGraphics();
    CG_LoadingString(localClientNum, " - items");
    CG_RegisterItems(localClientNum);
    CG_LoadingString(localClientNum, " - inline models");
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    CG_LoadingString(localClientNum, " - server models");
    for (i = 1; i < 512; ++i)
    {
        modelName = CL_GetConfigString(i + 1568);
        if (*modelName)
        {
            SCR_UpdateLoadScreen();
            cgs->gameModels[i] = R_RegisterModel((char*)modelName);
        }
    }
    for (i = 1; i < 196; ++i)
    {
        effectname = CL_GetConfigString(i + 2080);
        if (*effectname)
        {
            cgs->fxs[i] = FX_Register(effectname);
            if (!cgs->fxs[i]
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    1658,
                    0,
                    "%s",
                    "cgs->fxs[i]"))
            {
                __debugbreak();
            }
        }
    }
    EffectNames[0] = "weapon/grenade/fx_smoke_grenade_11sec_mp";
    EffectNames[1] = "misc/fx_flare_sky_white_10sec_mp";
    EffectNames[2] = "weapon/napalm/fx_napalm_ground_fire_lg_mp";
    EffectNames[3] = "weapon/napalm/fx_napalm_ground_fire_sm_mp";
    EffectNames[4] = "weapon/grenade/fx_exp_incendiary_mp_125r";
    EffectNames[5] = "weapon/grenade/fx_exp_incendiary_mp_75r";
    EffectNames[6] = "weapon/grenade/fx_exp_incendiary_mp_50r";
    EffectNames[7] = "weapon/grenade/fx_exp_incendiary_mp_25r";
    EffectNames[8] = "weapon/grenade/fx_exp_incendiary_mp_center";
    EffectNames[9] = "weapon/rocket/fx_trail_bazooka_geotrail";
    EffectNames[10] = "explosions/fx_default_explosion";
    EffectNames[11] = "env/fire/fx_fire_player_torso_mp";
    EffectNames[12] = "env/fire/fx_fire_player_sm_mp";
    EffectNames[13] = "env/fire/fx_fire_player_md_mp";
    EffectNames[14] = "env/fire/fx_fire_player_sm_smk_2sec";
    EffectNames[15] = "trail/fx_trail_blood_streak_mp";
    EffectNames[16] = "system_elements/fx_blood_drops_decal_emit";
    EffectNames[17] = "impacts/fx_flesh_hit_knife_mp";
    EffectNames[18] = "weapon/grenade/fx_smoke_grenade_mp_125r";
    EffectNames[19] = "weapon/grenade/fx_smoke_grenade_mp_75r";
    EffectNames[20] = "weapon/grenade/fx_smoke_grenade_mp_50r";
    EffectNames[21] = "weapon/grenade/fx_smoke_grenade_mp_25r";
    EffectNames[22] = "weapon/grenade/fx_gas_poison_mp_125r";
    EffectNames[23] = "weapon/grenade/fx_gas_poison_mp_75r";
    EffectNames[24] = "weapon/grenade/fx_gas_poison_mp_50r";
    EffectNames[25] = "weapon/grenade/fx_gas_poison_mp_25r";
    EffectNames[26] = "weapon/grenade/fx_gas_poison_mp";
    for (i = 0; i < 27; ++i)
    {
        v2 = FX_Register(EffectNames[i]);
        cgs->grenadeFx[i] = v2;
        if (!cgs->grenadeFx[i]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                1710,
                0,
                "%s",
                "cgs->grenadeFx[i]"))
        {
            __debugbreak();
        }
    }
    fireEffectNames[0] = "env/fire/fx_fire_player_torso_mp";
    fireEffectNames[1] = "env/fire/fx_fire_player_sm_mp";
    fireEffectNames[2] = "env/fire/fx_fire_player_md_mp";
    for (i = 0; i < 3; ++i)
    {
        v3 = FX_Register(fireEffectNames[i]);
        cgs->playerFireFx[i] = v3;
        if (!cgs->playerFireFx[i]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                1727,
                0,
                "%s",
                "cgs->playerFireFx[i]"))
        {
            __debugbreak();
        }
    }
    for (i = 1; i < 16; ++i)
    {
        shellshock = CL_GetConfigString(i + 2532);
        if (*shellshock)
        {
            if (!BG_LoadShellShockDvars(shellshock))
                Com_Error(ERR_DROP, "couldn't register shellshock '%s' -- see console", shellshock);
            ShellshockParms = BG_GetShellshockParms(i);
            BG_SetShellShockParmsFromDvars(ShellshockParms);
        }
    }
    if (!BG_LoadShellShockDvars("hold_breath_mp"))
        Com_Error(ERR_DROP, "Couldn't find shock file [hold_breath_mp.shock]\n");
    BG_SetShellShockParmsFromDvars(&cgs->holdBreathParams);
    cgMedia.fx = CG_RegisterImpactEffects(mapname);
    if (!cgMedia.fx)
        Com_Error(ERR_DROP, "Error reading CSV files in the fx directory to identify impact effects");
    cgMedia.fxNoBloodFleshHit = FX_Register("impacts/fx_flesh_hit_noblood");
    cgMedia.fxKnifeBlood = FX_Register("impacts/fx_flesh_hit_knife_mp");
    cgMedia.fxKnifeNoBlood = FX_Register("impacts/fx_flesh_hit_knife_noblood");
    cgMedia.fxDogBlood = FX_Register("impacts/fx_deathfx_dogbite");
    cgMedia.fxDogNoBlood = FX_Register("impacts/fx_flesh_hit_knife_noblood");
    cgMedia.fxNonFatalHero = FX_Register("impacts/fx_flesh_hit_body_nonfatal_hero");
    cgMedia.fxBodyArmorSmall = FX_Register("impacts/fx_small_metalhit_bodyarmor");
    cgMedia.fxBodyArmorLarge = FX_Register("impacts/fx_xlarge_metalhit_bodyarmor");
    cgMedia.fxDtpArmSlide1 = FX_Register("bio/player/fx_player_arm_dust_slide");
    cgMedia.fxDtpArmSlide2 = FX_Register("bio/player/fx_player_arm_dust_slide_rk");
    cgMedia.fxPlayerSliding = FX_Register("system_elements/fx_snow_sm_em");
    cgMedia.fxPuff = FX_Register("bio/player/fx_player_dust_inair");
    cgMedia.heliDustEffect = FX_Register("vehicle/treadfx/fx_heli_dust_default");
    cgMedia.heliWaterEffect = FX_Register("vehicle/treadfx/fx_heli_water_spray");
    cgMedia.helicopterLightSmoke = FX_Register("trail/fx_trail_heli_white_smoke");
    cgMedia.helicopterHeavySmoke = FX_Register("trail/fx_trail_heli_black_smoke");
    cgMedia.helicopterOnFire = FX_Register("trail/fx_trail_fire_smoke");
    cgMedia.jetAfterburner = FX_Register("vehicle/exhaust/fx_exhaust_jet_afterburner");
    cgMedia.physicsWaterEffects[0] = FX_Register("impacts/fx_water_hit_sm");
    cgMedia.physicsWaterEffects[1] = FX_Register("impacts/fx_water_hit_md");
    cgMedia.physicsWaterEffects[2] = FX_Register("impacts/fx_water_hit_lg");
    cgMedia.physicsWaterEffects[3] = FX_Register("impacts/fx_water_object_ripple");
    cgMedia.physicsWaterEffects[4] = FX_Register("bio/player/fx_water_hit_player_bubbles");
    cgMedia.physicsWaterEffects[5] = FX_Register("bio/player/fx_player_water_waist_ripple");
    cgMedia.physicsWaterEffects[6] = FX_Register("bio/player/fx_player_water_knee_ripple");
    cgMedia.physicsWaterEffects[7] = FX_Register("bio/player/fx_player_water_splash_impact");
    cgMedia.infraredHeartbeat = FX_Register("weapon/ir_scope/fx_ir_scope_heartbeat");
    CG_LoadingString(localClientNum, " - game media done");
}

void __cdecl CG_LoadHudMenu(int localClientNum)
{
    MenuList *Menus; // eax
    MenuList *v2; // eax
    const char *String; // eax
    const char *v4; // eax
    MenuList *v5; // eax
    cgs_t *cgs; // [esp+0h] [ebp-10h]
    menuDef_t *menu; // [esp+4h] [ebp-Ch]
    MenuList *menuList; // [esp+8h] [ebp-8h]
    MenuList *menuLista; // [esp+8h] [ebp-8h]
    MenuList *menuListb; // [esp+8h] [ebp-8h]
    const rectDef_s *rect; // [esp+Ch] [ebp-4h]

    Menus = UI_LoadMenus("ui_mp/hud_hardcore.txt", 7);
    UI_AddMenuList(localClientNum, &cgDC[localClientNum], Menus, 0);
    menuList = UI_LoadMenus("ui_mp/hud_spectator.txt", 7);
    UI_AddMenuList(localClientNum, &cgDC[localClientNum], menuList, 0);
    if ( !IsHardcoreMode(localClientNum) )
    {
        v2 = UI_LoadMenus("ui_mp/hud.txt", 7);
        UI_AddMenuList(localClientNum, &cgDC[localClientNum], v2, 0);
        String = Dvar_GetString("g_gametype");
        v4 = va("ui_mp/hud_%s.txt", String);
        menuLista = UI_LoadMenus(v4, 7);
        UI_AddMenuList(localClientNum, &cgDC[localClientNum], menuLista, 0);
    }
    v5 = UI_LoadMenus("ui_mp/hud_popups.txt", 7);
    UI_AddMenuList(localClientNum, &cgDC[localClientNum], v5, 0);
    menuListb = UI_LoadMenus("ui_mp/hud_demo.txt", 7);
    UI_AddMenuList(localClientNum, &cgDC[localClientNum], menuListb, 0);
    if ( CL_LocalClient_GetActiveCount() == 1 )
        menu = Menus_FindByName(&cgDC[localClientNum], "Compass");
    else
        menu = Menus_FindByName(&cgDC[localClientNum], "Compass_mp");
    if ( menu )
    {
        rect = Window_GetRect(&menu->window);
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        cgs->compassWidth = rect->w;
        cgs->compassHeight = rect->h;
        cgs->compassY = rect->y;
    }
}

const rectDef_s *__cdecl Window_GetRect(const windowDef_t *w)
{
    if ( !w && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils_api.h", 37, 0, "%s", "w") )
        __debugbreak();
    return &w->rect;
}

unsigned __int16 __cdecl CG_AttachWeapon(DObjModel_s *dobjModels, unsigned __int16 numModels, clientInfo_t *ci)
{
    const WeaponDef *weapDefDW; // [esp+4h] [ebp-10h]
    int oldLeftHand; // [esp+8h] [ebp-Ch]
    unsigned __int8 weaponModel; // [esp+Fh] [ebp-5h]
    const WeaponDef *weapDef; // [esp+10h] [ebp-4h]
    const WeaponDef *weapDefa; // [esp+10h] [ebp-4h]
    const WeaponDef *weapDefb; // [esp+10h] [ebp-4h]

    if ( ci->iDObjWeapon )
    {
        weapDef = BG_GetWeaponDef(ci->iDObjWeapon);
        weaponModel = ci->weaponModel;
        if ( weapDef->worldModel[weaponModel] && !weapDef->bHideThirdPerson )
        {
            if ( numModels >= 0x20u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                            2081,
                            0,
                            "%s",
                            "numModels < DOBJ_MAX_SUBMODELS") )
            {
                __debugbreak();
            }
            dobjModels[numModels].model = weapDef->worldModel[weaponModel];
            dobjModels[numModels].boneName = CG_GetWeaponAttachBone(ci, weapDef->weapType, weapDef->inventoryType);
            dobjModels[numModels++].ignoreCollision = 0;
        }
        if ( weapDef->bDualWield )
        {
            weapDefDW = BG_GetWeaponDef(weapDef->dualWieldWeaponIndex);
            if ( weapDefDW->worldModel[weaponModel] )
            {
                if ( !ci->usingKnife )
                {
                    if ( numModels >= 0x20u
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                                    2099,
                                    0,
                                    "%s",
                                    "numModels < DOBJ_MAX_SUBMODELS") )
                    {
                        __debugbreak();
                    }
                    dobjModels[numModels].model = weapDefDW->worldModel[weaponModel];
                    oldLeftHand = ci->leftHandGun;
                    ci->leftHandGun = 1;
                    dobjModels[numModels].boneName = CG_GetWeaponAttachBone(ci, weapDef->weapType, weapDef->inventoryType);
                    ci->leftHandGun = oldLeftHand;
                    dobjModels[numModels++].ignoreCollision = 0;
                }
            }
        }
        if ( weapDef->additionalMeleeModel )
        {
            if ( numModels >= 0x20u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                            2115,
                            0,
                            "%s",
                            "numModels < DOBJ_MAX_SUBMODELS") )
            {
                __debugbreak();
            }
            dobjModels[numModels].model = weapDef->additionalMeleeModel;
            dobjModels[numModels].boneName = scr_const.tag_weapon_left;
            dobjModels[numModels++].ignoreCollision = 0;
        }
        else if ( ci->usingKnife && ci->iDObjMeleeWeapon )
        {
            weapDefa = BG_GetWeaponDef(ci->iDObjMeleeWeapon);
            if ( weapDefa->worldModel )
            {
                if ( numModels >= 0x20u
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                                2128,
                                0,
                                "%s",
                                "numModels < DOBJ_MAX_SUBMODELS") )
                {
                    __debugbreak();
                }
                dobjModels[numModels].model = weapDefa->worldModel[ci->meleeWeaponModel];
                dobjModels[numModels].boneName = scr_const.tag_weapon_left;
                dobjModels[numModels++].ignoreCollision = 0;
            }
        }
        else if ( ci->usingGrenade )
        {
            if ( ci->iDObjOffhandWeapon )
            {
                weapDefb = BG_GetWeaponDef(ci->iDObjOffhandWeapon);
                if ( weapDefb->worldModel )
                {
                    if ( numModels >= 0x20u
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                                    2142,
                                    0,
                                    "%s",
                                    "numModels < DOBJ_MAX_SUBMODELS") )
                    {
                        __debugbreak();
                    }
                    dobjModels[numModels].model = weapDefb->worldModel[ci->offhandWeaponModel];
                    dobjModels[numModels].boneName = scr_const.tag_inhand;
                    dobjModels[numModels++].ignoreCollision = 0;
                }
            }
        }
    }
    return numModels;
}

void __cdecl CG_CreateDObj(
                DObjModel_s *dobjModels,
                unsigned __int16 numModels,
                XAnimTree_s *tree,
                int handle,
                int localClientNum,
                clientInfo_t *ci)
{
    float *v6; // eax
    centity_s *ent; // [esp+Ch] [ebp-4h]

    Com_ClientDObjCreate(dobjModels, numModels, tree, handle, localClientNum);
    ent = CG_GetEntity(localClientNum, handle);
    if ( ent && ent->pose.isRagdoll )
    {
        if ( ent->pose.killcamRagdollHandle > 0 )
            Ragdoll_RebindBody(ent->pose.killcamRagdollHandle);
        if ( ent->pose.ragdollHandle > 0 )
            Ragdoll_RebindBody(ent->pose.ragdollHandle);
    }
    v6 = cg_entityOriginArray[localClientNum][ci->clientNum];
    v6[0] = 131072.0f;
    v6[1] = 131072.0f;
    v6[2] = 131072.0f;
}

DObj *__cdecl CG_GetDObj(unsigned int handle, int localClientNum)
{
    return Com_GetClientDObj(handle, localClientNum);
}

void __cdecl CG_InitEntities(int localClientNum)
{
    cg_s *LocalClientGlobals; // eax
    int entityIndex; // [esp+8h] [ebp-Ch]
    centity_s *cent; // [esp+10h] [ebp-4h]

    for ( entityIndex = 0; entityIndex < 1024; ++entityIndex )
    {
        cent = CG_GetEntity(localClientNum, entityIndex);
        if ( !cent
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 2471, 0, "%s", "cent") )
        {
            __debugbreak();
        }
        cent->pose.localClientNum = localClientNum;
    }
    memset(&cg_fakeEntitiesArray[512 * localClientNum], 0, 0x65800u);
    CG_InitFakeEntities(localClientNum, 1);
    LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals->predictedPlayerEntity.pose.localClientNum = localClientNum;
    R_InitShaderConstantSet(&LocalClientGlobals->predictedPlayerEntity.pose.constantSet);
}

void __cdecl CG_InitViewDimensions(int localClientNum)
{
    cgs_t *cgs; // [esp+8h] [ebp-4h]

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    cgs->viewX = 0;
    cgs->viewX = 0;
    CL_GetScreenDimensions(&cgs->viewWidth, &cgs->viewHeight, &cgs->viewAspect);
    if ( cgs->viewWidth <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    2509,
                    1,
                    "%s\n\t(cgs->viewWidth) = %i",
                    "(cgs->viewWidth > 0)",
                    cgs->viewWidth) )
    {
        __debugbreak();
    }
    if ( cgs->viewHeight <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    2510,
                    1,
                    "%s\n\t(cgs->viewHeight) = %i",
                    "(cgs->viewHeight > 0)",
                    cgs->viewHeight) )
    {
        __debugbreak();
    }
    if ( cgs->viewAspect <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    2511,
                    1,
                    "%s\n\t(cgs->viewAspect) = %g",
                    "(cgs->viewAspect > 0)",
                    cgs->viewAspect) )
    {
        __debugbreak();
    }
}

void __cdecl CG_InitDof(GfxDepthOfField *dof)
{
    dof->nearStart = 0.0f;
    dof->nearEnd = 0.0f;
    dof->farStart = 5000.0f;
    dof->farEnd = 5000.0f;
    dof->nearBlur = 6.0f;
    dof->farBlur = 0.0f;
}

int CGScr_LoadScriptsAndAnims()
{
    int address[128]; // [esp+0h] [ebp-218h] BYREF
    const char *gametype; // [esp+204h] [ebp-14h]
    ScriptFunctions functions; // [esp+208h] [ebp-10h] BYREF
    const char *mapname; // [esp+214h] [ebp-4h]

    functions.maxSize = 128;
    functions.count = 0;
    functions.address = address;
    mapname = Dvar_GetString("mapname");
    gametype = Dvar_GetString("g_gametype");
    CGScr_LoadScripts(mapname, gametype, &functions);
    BG_LoadAnim(mapname);
    BG_PostLoadAnim(mapname);
    return functions.count;
}

void __cdecl CG_LoadAnimTrees(int localClientNum, cg_s *cgameGlob, const char *mapname, bool loading_scripts)
{
    if ( loading_scripts && com_sv_running->current.enabled )
        CGScr_LoadAnimTrees();
    if ( !cgameGlob->bgs.animData->generic_human.tree.anims && loading_scripts && com_sv_running->current.enabled )
    {
        BG_LoadAnim(mapname);
        BG_PostLoadAnim(mapname);
    }
    if ( !cgameGlob->bgs.animData->generic_human.tree.anims
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    2816,
                    0,
                    "%s",
                    "cgameGlob->bgs.animData->generic_human.tree.anims") )
    {
        __debugbreak();
    }
    CG_LoadAnimTreeInstances(localClientNum);
}

void __cdecl CG_LoadAnimTreeInstances(int localClientNum)
{
    XAnim_s *generic_human; // [esp+0h] [ebp-14h]
    cg_s *cgameGlob; // [esp+4h] [ebp-10h]
    cgs_t *cgs; // [esp+8h] [ebp-Ch]
    XAnim_s *anims; // [esp+10h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    generic_human = cgameGlob->bgs.animData->generic_human.tree.anims;

    for ( int i = 0; i < com_maxclients->current.integer; ++i )
        cgameGlob->bgs.clientinfo[i].pXAnimTree = XAnimCreateTree(generic_human, Hunk_AllocXAnimClient);

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    for (int i = 0; i < 4; ++i)
    {
        cgs->corpseinfo[i].pXAnimTree = XAnimCreateTree(generic_human, Hunk_AllocXAnimClient);
        //*(unsigned int *)&cgs->corpseinfo[1480 * ia + 1332] = (unsigned int)XAnimCreateTree(generic_human, Hunk_AllocXAnimClient);
    }

    anims = Dog_GetAnims();

    iassert(anims);

    for ( int i = 0; i < 16; ++i )
        cgameGlob->bgs.actorinfo[i].pXAnimTree = XAnimCreateTree(anims, Hunk_AllocXAnimClient);

    for ( int i = 0; i < 8; ++i )
    {
        cgs->actorCorpseInfo[i].pXAnimTree = XAnimCreateTree(anims, Hunk_AllocXAnimClient);
        cgs->actorCorpseInfo[i].entityNum = -1;
        //cgs->actorCorpseInfo[ic + 1].animInfo.legs.yawing = (int)XAnimCreateTree(anims, Hunk_AllocXAnimClient);
        //cgs->actorCorpseInfo[ic].animInfo.legs.animation = (animation_s *)-1;
    }
}

void __cdecl CG_SetupGameInformation(int localClientNum)
{
    CG_GetLocalClientGlobals(localClientNum)->matchUIVisibilityFlags = cls.gameState.matchUIVisibilityFlags;
}

void __cdecl CG_Shutdown(int localClientNum)
{
    colgeom_visitor_inlined_t<200> *v1; // [esp+0h] [ebp-28h]
    int i; // [esp+14h] [ebp-14h]
    cg_s *cgameGlob; // [esp+18h] [ebp-10h]
    centity_s *cent; // [esp+1Ch] [ebp-Ch]
    int entnum; // [esp+24h] [ebp-4h]

    g_ropesWithEntsAnchorsCount = 0;
    g_ropeCount = 0;

    for (i = 0; i < 1; ++i)
    {
        //p_proximity_data = &cg_pmove[i].proximity_data;
        //colgeom_visitor_inlined_t<500>::reset(p_proximity_data);
        cg_pmove[i].proximity_data.reset();
    }

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    destroy_client_gjkcc_info(localClientNum);
    R_TrackStatistics(0);
    SND_FadeIn();
    for ( entnum = 0; entnum < 1024; ++entnum )
    {
        cent = CG_GetEntity(localClientNum, entnum);
        if ( cent->pose.ragdollHandle > 0 )
        {
            Ragdoll_Remove(cent->pose.ragdollHandle);
            cent->pose.ragdollHandle = 0;
        }
        if ( cent->pose.physObjId )
        {
            if ( cent->pose.physObjId != -1 )
            {
                Phys_ObjDestroy(0, cent->pose.physObjId);
                cent->pose.physObjId = 0;
            }
        }
    }
    g_ambientStarted = 0;
    g_mapLoaded = 0;
    Mantle_ShutdownAnims();
    Dog_ShutdownAnims();
    if ( !useFastFile->current.enabled )
        Menus_FreeAllMemory(&cgDC[localClientNum]);
    CG_FreeWeapons(localClientNum);
    CG_FreeClientEntityCaches(localClientNum);
    CG_FreeClientDObjInfo(localClientNum);
    CG_FreeEntityDObjInfo(localClientNum);
    CG_GetLocalClientStaticGlobals(localClientNum);
    CG_ShutdownEntities(localClientNum);
    CG_ShutdownEntity(localClientNum, &cgameGlob->predictedPlayerEntity, 1);
    FX_KillAllEffects(localClientNum);
    FX_ShutdownSystem(localClientNum);
    DynEntCl_Shutdown(localClientNum);
    CG_ShutdownFakeEntities(localClientNum);
    GlassCl_Reset(localClientNum);
    num_heli_height_lock_patches = 0;
    CG_FreeAnimTreeInstances(localClientNum);
    cgameGlob->nextSnap = 0;
    memset(cgameGlob, 0, sizeof(cg_s));
    if ( cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    3408,
                    0,
                    "%s",
                    "!cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
}

void __cdecl CG_FreeAnimTreeInstances(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-Ch]
    cgs_t *cgs; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    for (i = 0; i < com_maxclients->current.integer; ++i)
    {
        if (cgameGlob->bgs.clientinfo[i].pXAnimTree)
        {
            XAnimFreeTree(cgameGlob->bgs.clientinfo[i].pXAnimTree, 0, SCRIPTINSTANCE_SERVER);
            cgameGlob->bgs.clientinfo[i].pXAnimTree = 0;
        }
    }
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    for (i = 0; i < 4; ++i)
    {
        if (cgs->corpseinfo[i].pXAnimTree)
        {
            XAnimFreeTree(cgs->corpseinfo[i].pXAnimTree, 0, SCRIPTINSTANCE_SERVER);
            cgs->corpseinfo[i].pXAnimTree = 0;
        }
    }
    for (i = 0; i < 16; ++i)
    {
        if (cgameGlob->bgs.actorinfo[i].pXAnimTree)
        {
            XAnimFreeTree(cgameGlob->bgs.actorinfo[i].pXAnimTree, 0, SCRIPTINSTANCE_SERVER);
            cgameGlob->bgs.actorinfo[i].pXAnimTree = 0;
        }
    }
    for (i = 0; i < 8; ++i)
    {
        if (cgs->actorCorpseInfo[i].pXAnimTree)
        {
            XAnimFreeTree(cgs->actorCorpseInfo[i].pXAnimTree, 0, SCRIPTINSTANCE_SERVER);
            cgs->actorCorpseInfo[i].pXAnimTree = 0;
        }
    }
}

void __cdecl CG_ShutdownOnceForAllClients()
{
    R_Cinematic_StopPlayback();
    Com_ShutdownDynamicMemorySystems();
    SND_GameReset();
    CG_SndGameReset();
    Ragdoll_Shutdown();
    Com_FreeWeaponInfoMemory(2);
    Scr_ShutdownSystem(SCRIPTINSTANCE_CLIENT, 1u, 0);
    CScr_FreeScripts();
    Scr_FreeScripts(SCRIPTINSTANCE_CLIENT, 1u);
    Scr_ShutdownGameStrings(SCRIPTINSTANCE_CLIENT);
    Scr_FreeEntityList(SCRIPTINSTANCE_CLIENT);
    if ( !com_sv_running->current.enabled )
        Scr_ShutdownGameStrings(SCRIPTINSTANCE_SERVER);
    BG_ShutdownFire();
    CG_ClearCompassPingData();
    CG_ShutdownConsoleCommands();
    CG_ResetClientInitializationState();
}

void __cdecl CG_ProcessTriggerDebug(centity_s *ent, trigger_info_t *trigger_info)
{
    const char *v2; // [esp+24h] [ebp-14h]
    centity_s *other; // [esp+30h] [ebp-8h]
    int i; // [esp+34h] [ebp-4h]

    if ( cg_usedTriggerCount < 300 )
    {
        for ( i = 0; i < cg_usedTriggerCount; ++i )
        {
            if ( cg_usedTriggers[i] == ent->nextState.number )
                return;
        }
        if ( cg_debug_triggers->current.enabled )
        {
            if ( cg_usedTriggerCount >= 300 )
            {
                Com_Printf(5, "CTrigger: Max Triggers Process\n");
                return;
            }
            if ( trigger_info )
                v2 = "";
            else
                v2 = "Immediate Notify Maxed!! ";
            Com_Printf(
                5,
                "CTrigger: %d (%g %g %g) %sIdx %d/%d\n",
                ent->nextState.number,
                ent->currentState.pos.trBase[0],
                ent->currentState.pos.trBase[1],
                ent->currentState.pos.trBase[2],
                v2,
                cg_level.entTriggerIndex[ent->nextState.number],
                cg_level.triggerIndex);
            if ( trigger_info )
            {
                other = CG_GetEntity(0, trigger_info->otherEntnum);
                if ( ent->nextState.lerp.useCount != trigger_info->useCount
                    || other->nextState.lerp.useCount != trigger_info->otherUseCount )
                {
                    Com_Printf(
                        5,
                        "CTrigger notify not being sent because useCount. Trig Info %d, Trig %d, Trig Info Other %d, Other %d\n",
                        trigger_info->useCount,
                        ent->nextState.lerp.useCount,
                        trigger_info->otherUseCount,
                        other->nextState.lerp.useCount);
                }
                if ( cg_level.entTriggerIndex[ent->nextState.number] == cg_level.triggerIndex )
                    Com_Printf(5, "CTrigger already triggered this frame loop\n");
            }
        }
        cg_usedTriggers[cg_usedTriggerCount++] = ent->nextState.number;
    }
}

int __cdecl CG_NotifyTriggers()
{
    trigger_info_t *v0; // ecx
    trigger_info_t *trigger_info; // [esp+14h] [ebp-18h]
    unsigned int *other; // [esp+18h] [ebp-14h]
    centity_s *ent; // [esp+1Ch] [ebp-10h]
    int bMoreTriggered; // [esp+20h] [ebp-Ch]
    int entnum; // [esp+24h] [ebp-8h]
    int i; // [esp+28h] [ebp-4h]

    bMoreTriggered = 0;
    ++cg_level.triggerIndex;
    for ( i = 0; i < cg_level.currentTriggerListSize; ++i )
    {
        trigger_info = &cg_level.currentTriggerList[i];
        entnum = trigger_info->entnum;
        ent = CG_GetEntity(0, entnum);
        CG_ProcessTriggerDebug(ent, trigger_info);
        if ( ent->nextState.lerp.useCount == trigger_info->useCount )
        {
            other = (unsigned int *)CG_GetEntity(0, trigger_info->otherEntnum);
            if ( other[151] == trigger_info->otherUseCount
                && ((*((unsigned int *)ent + 201) >> 1) & 1) != 0
                && ((other[201] >> 1) & 1) != 0 )
            {
                if ( cg_level.entTriggerIndex[entnum] == cg_level.triggerIndex )
                {
                    bMoreTriggered = 1;
                    continue;
                }
                cg_level.entTriggerIndex[entnum] = cg_level.triggerIndex;
                CScr_AddEntity(ent, 0);
                CScr_NotifyNum(0, other[122], 0, cscr_const.trigger, 1u);
            }
        }
        --cg_level.currentTriggerListSize;
        --i;
        v0 = &cg_level.currentTriggerList[cg_level.currentTriggerListSize];
        *(unsigned int *)&trigger_info->entnum = *(unsigned int *)&v0->entnum;
        trigger_info->useCount = v0->useCount;
        trigger_info->otherUseCount = v0->otherUseCount;
    }
    return bMoreTriggered;
}

void __cdecl CG_Trigger(centity_s *self, centity_s *other)
{
    trigger_info_t *trigger_info; // [esp+4h] [ebp-4h]

    if ( ((*((unsigned int *)other + 201) >> 1) & 1) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    3630,
                    0,
                    "%s",
                    "other->nextValid") )
    {
        __debugbreak();
    }
    if ( Scr_IsSystemActive(1u, SCRIPTINSTANCE_CLIENT) )
    {
        if ( cg_level.pendingTriggerListSize == 256 )
        {
            CG_ProcessTriggerDebug(self, 0);
            CScr_AddEntity(other, 0);
            CScr_NotifyNum(0, self->nextState.number, 0, cscr_const.trigger, 1u);
        }
        else
        {
            trigger_info = &cg_level.pendingTriggerList[cg_level.pendingTriggerListSize++];
            trigger_info->entnum = self->nextState.number;
            trigger_info->otherEntnum = other->nextState.number;
            trigger_info->useCount = self->nextState.lerp.useCount;
            trigger_info->otherUseCount = other->nextState.lerp.useCount;
        }
    }
}

void __cdecl CG_multi_trigger(centity_s *ent)
{
    if ( ((*((unsigned int *)ent + 201) >> 17) & 1) != 0 )
        CG_FreeEntityDelay(ent);
}

void __cdecl CG_Touch_Multi(centity_s *self, centity_s *other)
{
    if ( ((*((unsigned int *)other + 201) >> 1) & 1) != 0 )
    {
        CG_Trigger(self, other);
        CG_multi_trigger(other);
    }
}

void *__cdecl Hunk_AllocXAnimClient(unsigned int size)
{
    return Hunk_Alloc(size, "Hunk_AllocXAnimClient", 13);
}

