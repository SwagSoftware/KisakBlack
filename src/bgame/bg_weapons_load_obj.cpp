#include "bg_weapons_load_obj.h"
#include "bg_weapons.h"
#include "bg_animation.h"
#include <universal/com_loadutils.h>
#include <qcommon/common.h>
#include <universal/q_parse.h>
#include <universal/com_memory.h>
#include <universal/com_files.h>
#include "bg_weapons_attachment.h"
#include "bg_weapons_util.h"
#include <clientscript/cscr_stringlist.h>
#include <universal/surfaceflags.h>
#include <game/g_weapon_load_obj.h>
#include "bg_weapons_def.h"
#include <database/db_registry.h>

const char *szWeapOverlayReticleNames[2] =
{ "none", "crosshair" };

const char *penetrateTypeNames[4] =
{ "none", "small", "medium", "large" };

const char *impactTypeNames_0[16] =
{
  "none",
  "bullet_small",
  "bullet_large",
  "bullet_ap",
  "bullet_xtreme",
  "shotgun",
  "grenade_bounce",
  "grenade_explode",
  "rifle_grenade",
  "rocket_explode",
  "rocket_explode_xtreme",
  "projectile_dud",
  "mortar_shell",
  "tank_shell",
  "bolt",
  "blade"
};

const char *szWeapStanceNames[3] =
{ "stand", "duck", "prone" };

const char *szProjectileExplosionNames[10] =
{
  "grenade",
  "rocket",
  "flashbang",
  "none",
  "dud",
  "smoke",
  "heavy explosive",
  "fire",
  "napalmblob",
  "bolt"
};

const char *offhandClassNames[5] =
{ "None", "Frag Grenade", "Smoke Grenade", "Flash Grenade", "Gear" };

const char *offhandSlotNames[5] =
{ "None", "Lethal grenade", "Tactical grenade", "Equipment", "Specific use" };

const char *activeReticleNames[3] =
{ "None", "Pip-On-A-Stick", "Bouncing diamond" };

const char *guidedMissileNames[7] =
{
  "None",
  "Sidewinder",
  "Hellfire",
  "Javelin",
  "Ballistic",
  "WireGuided",
  "TVGuided"
};

const char *stickinessNames[6] =
{
  "Don't stick",
  "Stick to all",
  "Stick to all, except ai and clients",
  "Stick to ground",
  "Stick to ground, maintain yaw",
  "Stick to flesh"
};

const char *rotateTypeNames[3] =
{
  "Rotate both axis, grenade style",
  "Rotate one axis, blade style",
  "Rotate like a cylinder"
};

const char *overlayInterfaceNames[3] =
{ "None", "Javelin", "Turret Scope" };

const char *szWeapFireTypeNames[7] =
{
  "Full Auto",
  "Single Shot",
  "2-Round Burst",
  "3-Round Burst",
  "4-Round Burst",
  "Stacked Fire",
  "Minigun"
};

const char *szWeapClipTypeNames[6] =
{ "bottom", "top", "left", "dp28", "ptrs", "lmg" };

const char *ammoCounterClipNames[7] =
{
  "None",
  "Magazine",
  "ShortMagazine",
  "Shotgun",
  "Rocket",
  "Beltfed",
  "AltWeapon"
};

const char *weapIconRatioNames[3] =
{ "1:1", "2:1", "4:1" };

WeaponFullDef bg_defaultWeaponFullDefs;
SurfaceTypeSoundList surfaceTypeSoundLists[16];

char *s_weaponGripAnimSubstrings[64];

char *g_playerAnimTypeNames[32];
unsigned int g_playerAnimTypeNamesCount;

int g_playerAnimTypeIndex_revivee;
int g_playerAnimTypeIndex_sniper;
int g_playerAnimTypeIndex_sniper_rearclip;
int g_playerAnimTypeIndex_briefcase;

int surfaceTypeSoundListCount;
int s_numWeaponGripAnimSubstrings;

unsigned int bg_nextParseFlameTableIndex;

flameTable *bg_flameTables[8];

cspField_t weaponDefFields[748] =
{
  { "displayName", 12, 0 },
  { "AIOverlayDescription", 228, 0 },
  { "modeName", 240, 0 },
  { "playerAnimType", 252, 24 },
  { "gunModel", 2284, 10 },
  { "gunModel2", 2288, 10 },
  { "gunModel3", 2292, 10 },
  { "gunModel4", 2296, 10 },
  { "gunModel5", 2300, 10 },
  { "gunModel6", 2304, 10 },
  { "gunModel7", 2308, 10 },
  { "gunModel8", 2312, 10 },
  { "gunModel9", 2316, 10 },
  { "gunModel10", 2320, 10 },
  { "gunModel11", 2324, 10 },
  { "gunModel12", 2328, 10 },
  { "gunModel13", 2332, 10 },
  { "gunModel14", 2336, 10 },
  { "gunModel15", 2340, 10 },
  { "gunModel16", 2344, 10 },
  { "handModel", 236, 10 },
  { "hideTags", 2612, 40 },
  { "notetrackSoundMap", 2676, 42 },
  { "idleAnim", 2352, 0 },
  { "idleAnimLeft", 2588, 0 },
  { "emptyIdleAnim", 2356, 0 },
  { "emptyIdleAnim", 2592, 0 },
  { "fireAnim", 2360, 0 },
  { "fireAnimLeft", 2580, 0 },
  { "holdFireAnim", 2364, 0 },
  { "lastShotAnim", 2368, 0 },
  { "lastShotAnimLeft", 2584, 0 },
  { "detonateAnim", 2504, 0 },
  { "rechamberAnim", 2372, 0 },
  { "meleeAnim", 2376, 0 },
  { "meleeChargeAnim", 2380, 0 },
  { "reloadAnim", 2384, 0 },
  { "reloadAnimRight", 2388, 0 },
  { "reloadAnimLeft", 2600, 0 },
  { "reloadEmptyAnim", 2392, 0 },
  { "reloadEmptyAnimLeft", 2596, 0 },
  { "reloadStartAnim", 2396, 0 },
  { "reloadEndAnim", 2400, 0 },
  { "reloadQuickAnim", 2404, 0 },
  { "reloadQuickEmptyAnim", 2408, 0 },
  { "raiseAnim", 2412, 0 },
  { "dropAnim", 2420, 0 },
  { "firstRaiseAnim", 2416, 0 },
  { "altRaiseAnim", 2424, 0 },
  { "altDropAnim", 2428, 0 },
  { "quickRaiseAnim", 2432, 0 },
  { "quickDropAnim", 2436, 0 },
  { "emptyRaiseAnim", 2440, 0 },
  { "emptyDropAnim", 2444, 0 },
  { "sprintInAnim", 2448, 0 },
  { "sprintLoopAnim", 2452, 0 },
  { "sprintOutAnim", 2456, 0 },
  { "sprintInEmptyAnim", 2460, 0 },
  { "sprintLoopEmptyAnim", 2464, 0 },
  { "sprintOutEmptyAnim", 2468, 0 },
  { "lowReadyInAnim", 2472, 0 },
  { "lowReadyLoopAnim", 2476, 0 },
  { "lowReadyOutAnim", 2480, 0 },
  { "contFireInAnim", 2484, 0 },
  { "contFireLoopAnim", 2488, 0 },
  { "contFireOutAnim", 2492, 0 },
  { "deployAnim", 2496, 0 },
  { "breakdownAnim", 2500, 0 },
  { "nightVisionWearAnim", 2508, 0 },
  { "nightVisionRemoveAnim", 2512, 0 },
  { "adsFireAnim", 2516, 0 },
  { "adsLastShotAnim", 2520, 0 },
  { "adsRechamberAnim", 2524, 0 },
  { "adsUpAnim", 2604, 0 },
  { "adsDownAnim", 2608, 0 },
  { "deployAnim", 2496, 0 },
  { "breakdownAnim", 2500, 0 },
  { "dtp_in", 2528, 0 },
  { "dtp_loop", 2532, 0 },
  { "dtp_out", 2536, 0 },
  { "dtp_empty_in", 2540, 0 },
  { "dtp_empty_loop", 2544, 0 },
  { "dtp_empty_out", 2548, 0 },
  { "slide_in", 2552, 0 },
  { "mantleAnim", 2556, 0 },
  { "sprintCameraAnim", 2560, 0 },
  { "dtpInCameraAnim", 2564, 0 },
  { "dtpLoopCameraAnim", 2568, 0 },
  { "dtpOutCameraAnim", 2572, 0 },
  { "mantleCameraAnim", 2576, 0 },
  { "script", 2176, 0 },
  { "weaponType", 256, 15 },
  { "weaponClass", 260, 16 },
  { "penetrateType", 264, 18 },
  { "impactType", 268, 19 },
  { "inventoryType", 272, 31 },
  { "fireType", 276, 32 },
  { "clipType", 280, 33 },
  { "offhandClass", 332, 22 },
  { "offhandSlot", 336, 23 },
  { "viewFlashEffect", 344, 9 },
  { "worldFlashEffect", 348, 9 },
  { "pickupSound", 352, 0 },
  { "pickupSoundPlayer", 356, 0 },
  { "ammoPickupSound", 360, 0 },
  { "ammoPickupSoundPlayer", 364, 0 },
  { "projectileSound", 368, 0 },
  { "pullbackSound", 372, 0 },
  { "pullbackSoundPlayer", 376, 0 },
  { "fireSound", 380, 0 },
  { "crackSound", 428, 0 },
  { "whizbySound", 432, 0 },
  { "fireSoundPlayer", 384, 0 },
  { "loopFireSound", 388, 0 },
  { "loopFireSoundPlayer", 392, 0 },
  { "loopFireEndSound", 396, 0 },
  { "loopFireEndSoundPlayer", 400, 0 },
  { "stopFireSound", 404, 0 },
  { "stopFireSoundPlayer", 408, 0 },
  { "lastShotSound", 412, 0 },
  { "lastShotSoundPlayer", 416, 0 },
  { "emptyFireSound", 420, 0 },
  { "emptyFireSoundPlayer", 424, 0 },
  { "meleeSwipeSound", 436, 0 },
  { "meleeSwipeSoundPlayer", 440, 0 },
  { "meleeHitSound", 444, 0 },
  { "meleeMissSound", 448, 0 },
  { "rechamberSound", 452, 0 },
  { "rechamberSoundPlayer", 456, 0 },
  { "reloadSound", 460, 0 },
  { "reloadSoundPlayer", 464, 0 },
  { "reloadEmptySound", 468, 0 },
  { "reloadEmptySoundPlayer", 472, 0 },
  { "reloadStartSound", 476, 0 },
  { "reloadStartSoundPlayer", 480, 0 },
  { "reloadEndSound", 484, 0 },
  { "reloadEndSoundPlayer", 488, 0 },
  { "rotateLoopSound", 492, 0 },
  { "rotateLoopSoundPlayer", 496, 0 },
  { "deploySound", 500, 0 },
  { "deploySoundPlayer", 504, 0 },
  { "finishDeploySound", 508, 0 },
  { "finishDeploySoundPlayer", 512, 0 },
  { "breakdownSound", 516, 0 },
  { "breakdownSoundPlayer", 520, 0 },
  { "finishBreakdownSound", 524, 0 },
  { "finishBreakdownSoundPlayer", 528, 0 },
  { "detonateSound", 532, 0 },
  { "detonateSoundPlayer", 536, 0 },
  { "nightVisionWearSound", 540, 0 },
  { "nightVisionWearSoundPlayer", 544, 0 },
  { "nightVisionRemoveSound", 548, 0 },
  { "nightVisionRemoveSoundPlayer", 552, 0 },
  { "raiseSound", 564, 0 },
  { "raiseSoundPlayer", 568, 0 },
  { "firstRaiseSound", 572, 0 },
  { "firstRaiseSoundPlayer", 576, 0 },
  { "altSwitchSound", 556, 0 },
  { "altSwitchSoundPlayer", 560, 0 },
  { "putawaySound", 580, 0 },
  { "putawaySoundPlayer", 584, 0 },
  { "overheatSound", 588, 0 },
  { "overheatSoundPlayer", 592, 0 },
  { "adsZoomSound", 596, 0 },
  { "bounceSound", 600, 27 },
  { "standMountedWeapdef", 604, 0 },
  { "crouchMountedWeapdef", 608, 0 },
  { "proneMountedWeapdef", 612, 0 },
  { "viewShellEjectEffect", 628, 9 },
  { "worldShellEjectEffect", 632, 9 },
  { "viewLastShotEjectEffect", 636, 9 },
  { "worldLastShotEjectEffect", 640, 9 },
  { "reticleCenter", 644, 11 },
  { "reticleSide", 648, 11 },
  { "reticleCenterSize", 652, 4 },
  { "reticleSideSize", 656, 4 },
  { "reticleMinOfs", 660, 4 },
  { "activeReticleType", 664, 25 },
  { "standMoveF", 668, 7 },
  { "standMoveR", 672, 7 },
  { "standMoveU", 676, 7 },
  { "standRotP", 680, 7 },
  { "standRotY", 684, 7 },
  { "standRotR", 688, 7 },
  { "duckedOfsF", 692, 7 },
  { "duckedOfsR", 696, 7 },
  { "duckedOfsU", 700, 7 },
  { "duckedMoveF", 704, 7 },
  { "duckedMoveR", 708, 7 },
  { "duckedMoveU", 712, 7 },
  { "duckedSprintOfsF", 716, 7 },
  { "duckedSprintOfsR", 720, 7 },
  { "duckedSprintOfsU", 724, 7 },
  { "duckedSprintRotP", 728, 7 },
  { "duckedSprintRotY", 732, 7 },
  { "duckedSprintRotR", 736, 7 },
  { "duckedSprintBobH", 740, 7 },
  { "duckedSprintBobV", 744, 7 },
  { "duckedSprintScale", 748, 7 },
  { "sprintOfsF", 752, 7 },
  { "sprintOfsR", 756, 7 },
  { "sprintOfsU", 760, 7 },
  { "sprintRotP", 764, 7 },
  { "sprintRotY", 768, 7 },
  { "sprintRotR", 772, 7 },
  { "sprintBobH", 776, 7 },
  { "sprintBobV", 780, 7 },
  { "sprintScale", 784, 7 },
  { "lowReadyOfsF", 788, 7 },
  { "lowReadyOfsR", 792, 7 },
  { "lowReadyOfsU", 796, 7 },
  { "lowReadyRotP", 800, 7 },
  { "lowReadyRotY", 804, 7 },
  { "lowReadyRotR", 808, 7 },
  { "dtpOfsF", 812, 7 },
  { "dtpOfsR", 816, 7 },
  { "dtpOfsU", 820, 7 },
  { "dtpRotP", 824, 7 },
  { "dtpRotY", 828, 7 },
  { "dtpRotR", 832, 7 },
  { "dtpBobH", 836, 7 },
  { "dtpBobV", 840, 7 },
  { "dtpScale", 844, 7 },
  { "mantleOfsF", 848, 7 },
  { "mantleOfsR", 852, 7 },
  { "mantleOfsU", 856, 7 },
  { "mantleRotP", 860, 7 },
  { "mantleRotY", 864, 7 },
  { "mantleRotR", 868, 7 },
  { "slideOfsF", 872, 7 },
  { "slideOfsR", 876, 7 },
  { "slideOfsU", 880, 7 },
  { "slideRotP", 884, 7 },
  { "slideRotY", 888, 7 },
  { "slideRotR", 892, 7 },
  { "duckedRotP", 896, 7 },
  { "duckedRotY", 900, 7 },
  { "duckedRotR", 904, 7 },
  { "proneOfsF", 908, 7 },
  { "proneOfsR", 912, 7 },
  { "proneOfsU", 916, 7 },
  { "proneMoveF", 920, 7 },
  { "proneMoveR", 924, 7 },
  { "proneMoveU", 928, 7 },
  { "proneRotP", 932, 7 },
  { "proneRotY", 936, 7 },
  { "proneRotR", 940, 7 },
  { "strafeMoveF", 944, 7 },
  { "strafeMoveR", 948, 7 },
  { "strafeMoveU", 952, 7 },
  { "strafeRotP", 956, 7 },
  { "strafeRotY", 960, 7 },
  { "strafeRotR", 964, 7 },
  { "posMoveRate", 968, 7 },
  { "posProneMoveRate", 972, 7 },
  { "standMoveMinSpeed", 976, 7 },
  { "duckedMoveMinSpeed", 980, 7 },
  { "proneMoveMinSpeed", 984, 7 },
  { "posRotRate", 988, 7 },
  { "posProneRotRate", 992, 7 },
  { "standRotMinSpeed", 996, 7 },
  { "duckedRotMinSpeed", 1000, 7 },
  { "proneRotMinSpeed", 1004, 7 },
  { "worldModel", 2756, 10 },
  { "worldModel2", 2760, 10 },
  { "worldModel3", 2764, 10 },
  { "worldModel4", 2768, 10 },
  { "worldModel5", 2772, 10 },
  { "worldModel6", 2776, 10 },
  { "worldModel7", 2780, 10 },
  { "worldModel8", 2784, 10 },
  { "worldModel9", 2788, 10 },
  { "worldModel10", 2792, 10 },
  { "worldModel11", 2796, 10 },
  { "worldModel12", 2800, 10 },
  { "worldModel13", 2804, 10 },
  { "worldModel14", 2808, 10 },
  { "worldModel15", 2812, 10 },
  { "worldModel16", 2816, 10 },
  { "worldClipModel", 1012, 10 },
  { "rocketModel", 1016, 10 },
  { "mountedModel", 1020, 10 },
  { "AdditionalMeleeModel", 1024, 10 },
  { "hudIcon", 1028, 11 },
  { "hudIconRatio", 1032, 35 },
  { "indicatorIcon", 1036, 11 },
  { "indicatorIconRatio", 1040, 39 },
  { "ammoCounterIcon", 1044, 11 },
  { "ammoCounterIconRatio", 1048, 36 },
  { "ammoCounterClip", 1052, 34 },
  { "startAmmo", 1056, 4 },
  { "ammoName", 64, 0 },
  { "clipName", 72, 0 },
  { "maxAmmo", 1064, 4 },
  { "clipSize", 32, 4 },
  { "shotCount", 1068, 4 },
  { "sharedAmmoCapName", 1072, 0 },
  { "sharedAmmoCap", 1080, 4 },
  { "unlimitedAmmo", 1084, 5 },
  { "ammoCountClipRelative", 1085, 5 },
  { "sharedAmmo", 1584, 5 },
  { "jamFireTime", 292, 8 },
  { "overheatWeapon", 308, 4 },
  { "overheatRate", 312, 7 },
  { "cooldownRate", 316, 7 },
  { "overheatEndVal", 320, 7 },
  { "coolWhileFiring", 324, 4 },
  { "fuelTankWeapon", 325, 4 },
  { "tankLifeTime", 328, 8 },
  { "damage", 1088, 4 },
  { "damageDuration", 1092, 7 },
  { "damageInterval", 1096, 7 },
  { "playerDamage", 1100, 4 },
  { "meleeDamage", 1104, 4 },
  { "minDamage", 2180, 4 },
  { "minPlayerDamage", 2184, 4 },
  { "maxDamageRange", 2188, 7 },
  { "minDamageRange", 2192, 7 },
  { "destabilizationRateTime", 2196, 7 },
  { "destabilizationCurvatureMax", 2200, 7 },
  { "destabilizeDistance", 2204, 4 },
  { "fireDelay", 1116, 8 },
  { "meleeDelay", 1120, 8 },
  { "meleeChargeDelay", 1124, 8 },
  { "spinUpTime", 1132, 8 },
  { "spinDownTime", 1136, 8 },
  { "spinRate", 1140, 7 },
  { "spinLoopSound", 1144, 0 },
  { "spinLoopSoundPlayer", 1148, 0 },
  { "startSpinSound", 1152, 0 },
  { "startSpinSoundPlayer", 1156, 0 },
  { "stopSpinSound", 1160, 0 },
  { "stopSpinSoundPlayer", 1164, 0 },
  { "fireTime", 1168, 8 },
  { "lastFireTime", 1172, 8 },
  { "rechamberTime", 1176, 8 },
  { "rechamberBoltTime", 1180, 8 },
  { "holdFireTime", 1184, 8 },
  { "detonateTime", 1188, 8 },
  { "detonateDelay", 1128, 8 },
  { "meleeTime", 1192, 8 },
  { "meleeChargeTime", 1196, 8 },
  { "reloadTime", 36, 8 },
  { "reloadShowRocketTime", 1208, 8 },
  { "reloadEmptyTime", 40, 8 },
  { "reloadAddTime", 1216, 8 },
  { "reloadEmptyAddTime", 1220, 8 },
  { "reloadQuickAddTime", 1224, 8 },
  { "reloadQuickEmptyAddTime", 1228, 8 },
  { "reloadStartTime", 1232, 8 },
  { "reloadStartAddTime", 1236, 8 },
  { "reloadEndTime", 1240, 8 },
  { "reloadQuickTime", 44, 8 },
  { "reloadQuickEmptyTime", 48, 8 },
  { "dropTime", 1244, 8 },
  { "raiseTime", 1248, 8 },
  { "altDropTime", 1252, 8 },
  { "altRaiseTime", 60, 8 },
  { "quickDropTime", 1256, 8 },
  { "quickRaiseTime", 1260, 8 },
  { "firstRaiseTime", 1264, 8 },
  { "emptyRaiseTime", 1268, 8 },
  { "emptyDropTime", 1272, 8 },
  { "sprintInTime", 1276, 8 },
  { "sprintLoopTime", 1280, 8 },
  { "sprintOutTime", 1284, 8 },
  { "lowReadyInTime", 1288, 8 },
  { "lowReadyLoopTime", 1292, 8 },
  { "lowReadyOutTime", 1296, 8 },
  { "contFireInTime", 1300, 8 },
  { "contFireLoopTime", 1304, 8 },
  { "contFireOutTime", 1308, 8 },
  { "dtpInTime", 1312, 8 },
  { "dtpLoopTime", 1316, 8 },
  { "dtpOutTime", 1320, 8 },
  { "slideInTime", 1324, 8 },
  { "deployTime", 1328, 8 },
  { "breakdownTime", 1332, 8 },
  { "tracerFrequency", 296, 4 },
  { "tracerWidth", 300, 7 },
  { "tracerLength", 304, 7 },
  { "nightVisionWearTime", 1336, 8 },
  { "nightVisionWearTimeFadeOutEnd", 1340, 8 },
  { "nightVisionWearTimePowerUp", 1344, 8 },
  { "nightVisionRemoveTime", 1348, 8 },
  { "nightVisionRemoveTimePowerDown", 1352, 8 },
  { "nightVisionRemoveTimeFadeInStart", 1356, 8 },
  { "fuseTime", 1360, 8 },
  { "aifuseTime", 1364, 8 },
  { "lockOnRadius", 1368, 4 },
  { "lockOnSpeed", 1372, 4 },
  { "requireLockonToFire", 1376, 5 },
  { "noAdsWhenMagEmpty", 1377, 5 },
  { "avoidDropCleanup", 1378, 5 },
  { "stackFire", 1380, 4 },
  { "stackFireSpread", 1384, 7 },
  { "stackFireAccuracyDecay", 1388, 7 },
  { "stackSound", 1392, 0 },
  { "autoAimRange", 1396, 7 },
  { "aimAssistRange", 1400, 7 },
  { "aimAssistRangeAds", 80, 7 },
  { "mountableWeapon", 1404, 5 },
  { "aimPadding", 1408, 7 },
  { "enemyCrosshairRange", 1412, 7 },
  { "crosshairColorChange", 1416, 5 },
  { "moveSpeedScale", 1420, 7 },
  { "adsMoveSpeedScale", 1424, 7 },
  { "sprintDurationScale", 1428, 7 },
  { "idleCrouchFactor", 1524, 7 },
  { "idleProneFactor", 1528, 7 },
  { "gunMaxPitch", 1532, 7 },
  { "gunMaxYaw", 1536, 7 },
  { "swayMaxAngle", 1540, 7 },
  { "swayLerpSpeed", 1544, 7 },
  { "swayPitchScale", 1548, 7 },
  { "swayYawScale", 1552, 7 },
  { "swayHorizScale", 1556, 7 },
  { "swayVertScale", 1560, 7 },
  { "swayShellShockScale", 1564, 7 },
  { "adsSwayMaxAngle", 1568, 7 },
  { "adsSwayLerpSpeed", 1572, 7 },
  { "adsSwayPitchScale", 1576, 7 },
  { "adsSwayYawScale", 1580, 7 },
  { "adsSwayHorizScale", 84, 7 },
  { "adsSwayVertScale", 88, 7 },
  { "meleeChargeRange", 1836, 7 },
  { "rifleBullet", 1585, 5 },
  { "armorPiercing", 1586, 5 },
  { "boltAction", 1587, 5 },
  { "useAltTagFlash", 1588, 5 },
  { "useAntiLagRewind", 1589, 5 },
  { "isCarriedKillstreakWeapon", 1590, 5 },
  { "aimDownSight", 1591, 5 },
  { "rechamberWhileAds", 1592, 5 },
  { "reloadWhileAds", 1593, 5 },
  { "adsViewErrorMin", 1596, 7 },
  { "adsViewErrorMax", 1600, 7 },
  { "clipOnly", 1605, 5 },
  { "canUseInVehicle", 1606, 5 },
  { "noDropsOrRaises", 1607, 5 },
  { "cookOffHold", 1604, 5 },
  { "adsFire", 1608, 5 },
  { "cancelAutoHolsterWhenEmpty", 1609, 5 },
  { "suppressAmmoReserveDisplay", 1610, 5 },
  { "laserSightDuringNightvision", 1611, 5 },
  { "bayonet", 1613, 5 },
  { "dualWield", 1614, 5 },
  { "hideThirdPerson", 1612, 5 },
  { "explodeOnGround", 1615, 5 },
  { "throwBack", 1616, 5 },
  { "retrievable", 1617, 5 },
  { "dieOnRespawn", 1618, 5 },
  { "noThirdPersonDropsOrRaises", 1619, 5 },
  { "continuousFire", 1620, 5 },
  { "fullMetalJacket", 134, 5 },
  { "hollowPoint", 135, 5 },
  { "useAsMelee", 1840, 5 },
  { "rapidFire", 136, 5 },
  { "noPing", 1621, 5 },
  { "forceBounce", 1622, 5 },
  { "useDroppedModelAsStowed", 1623, 5 },
  { "noQuickDropWhenEmpty", 1624, 5 },
  { "keepCrosshairWhenADS", 1625, 5 },
  { "useOnlyAltWeaoponHideTagsInAltMode", 1626, 5 },
  { "killIcon", 1628, 11 },
  { "killIconRatio", 1632, 37 },
  { "flipKillIcon", 1636, 5 },
  { "dpadIcon", 148, 11 },
  { "dpadIconRatio", 152, 38 },
  { "noPartialReload", 1637, 5 },
  { "segmentedReload", 1638, 5 },
  { "noADSAutoReload", 1639, 5 },
  { "reloadAmmoAdd", 1640, 4 },
  { "reloadStartAdd", 1644, 4 },
  { "altWeapon", 20, 0 },
  { "DualWieldWeapon", 1652, 0 },
  { "grenadeWeapon", 1648, 0 },
  { "dropAmmoMin", 1660, 4 },
  { "dropAmmoMax", 1664, 4 },
  { "dropClipAmmoMin", 1668, 4 },
  { "dropClipAmmoMax", 1672, 4 },
  { "blocksProne", 1676, 5 },
  { "silenced", 132, 5 },
  { "dualMag", 133, 5 },
  { "isRollingGrenade", 1680, 6 },
  { "showIndicator", 1677, 5 },
  { "explosionRadius", 1684, 4 },
  { "explosionRadiusMin", 1688, 4 },
  { "indicatorRadius", 1692, 4 },
  { "explosionInnerDamage", 1696, 4 },
  { "explosionOuterDamage", 1700, 4 },
  { "damageConeAngle", 1704, 7 },
  { "projectileSpeed", 1708, 4 },
  { "projectileSpeedRelativeUp", 1716, 4 },
  { "projectileSpeedUp", 1712, 4 },
  { "projectileSpeedForward", 1720, 4 },
  { "projectileActivateDist", 1724, 4 },
  { "projectileLifetime", 1728, 7 },
  { "timeToAccelerate", 1732, 7 },
  { "projectileCurvature", 1736, 7 },
  { "projectileModel", 1740, 10 },
  { "projExplosionType", 1744, 21 },
  { "projExplosionEffect", 1748, 9 },
  { "projExplosionEffectForceNormalUp", 1752, 5 },
  { "projExplosionEffect2", 1756, 9 },
  { "projExplosionEffect2ForceNormalUp", 1760, 5 },
  { "projExplosionEffect3", 1764, 9 },
  { "projExplosionEffect3ForceNormalUp", 1768, 5 },
  { "projExplosionEffect4", 1772, 9 },
  { "projExplosionEffect4ForceNormalUp", 1776, 5 },
  { "projExplosionEffect5", 1780, 9 },
  { "projExplosionEffect5ForceNormalUp", 1784, 5 },
  { "projExplosionSound", 1792, 0 },
  { "projDudEffect", 1788, 9 },
  { "projDudSound", 1796, 0 },
  { "projImpactExplode", 1808, 5 },
  { "bulletImpactExplode", 1809, 5 },
  { "mortarShellSound", 1800, 0 },
  { "tankShellSound", 1804, 0 },
  { "stickiness", 1812, 28 },
  { "rotateType", 1816, 29 },
  { "hasDetonator", 1821, 5 },
  { "plantable", 1820, 5 },
  { "timedDetonation", 1822, 5 },
  { "noCrumpleMissile", 1823, 5 },
  { "rotate", 1824, 5 },
  { "keepRolling", 1825, 5 },
  { "holdButtonToThrow", 1826, 5 },
  { "offhandHoldIsCancelable", 1827, 5 },
  { "freezeMovementWhenFiring", 1828, 5 },
  { "lowAmmoWarningThreshold", 1832, 7 },
  { "explosionTag", 1112, 41 },
  { "isCameraSensor", 1841, 5 },
  { "isAcousticSensor", 1842, 5 },
  { "parallelDefaultBounce", 2820, 7 },
  { "parallelAsphaltBounce", 2908, 7 },
  { "parallelBarkBounce", 2824, 7 },
  { "parallelBrickBounce", 2828, 7 },
  { "parallelCarpetBounce", 2832, 7 },
  { "parallelCeramicBounce", 2912, 7 },
  { "parallelClothBounce", 2836, 7 },
  { "parallelConcreteBounce", 2840, 7 },
  { "parallelCushionBounce", 2924, 7 },
  { "parallelDirtBounce", 2844, 7 },
  { "parallelFleshBounce", 2848, 7 },
  { "parallelFoliageBounce", 2852, 7 },
  { "parallelFruitBounce", 2928, 7 },
  { "parallelGlassBounce", 2856, 7 },
  { "parallelGrassBounce", 2860, 7 },
  { "parallelGravelBounce", 2864, 7 },
  { "parallelIceBounce", 2868, 7 },
  { "parallelMetalBounce", 2872, 7 },
  { "parallelMudBounce", 2876, 7 },
  { "parallelPaintedMetalBounce", 2932, 7 },
  { "parallelPaperBounce", 2880, 7 },
  { "parallelPlasterBounce", 2884, 7 },
  { "parallelPlasticBounce", 2916, 7 },
  { "parallelRockBounce", 2888, 7 },
  { "parallelRubberBounce", 2920, 7 },
  { "parallelSandBounce", 2892, 7 },
  { "parallelSnowBounce", 2896, 7 },
  { "parallelWaterBounce", 2900, 7 },
  { "parallelWoodBounce", 2904, 7 },
  { "perpendicularDefaultBounce", 2944, 7 },
  { "perpendicularAsphaltBounce", 3032, 7 },
  { "perpendicularBarkBounce", 2948, 7 },
  { "perpendicularBrickBounce", 2952, 7 },
  { "perpendicularCarpetBounce", 2956, 7 },
  { "perpendicularCeramicBounce", 3036, 7 },
  { "perpendicularClothBounce", 2960, 7 },
  { "perpendicularConcreteBounce", 2964, 7 },
  { "perpendicularCushionBounce", 3048, 7 },
  { "perpendicularDirtBounce", 2968, 7 },
  { "perpendicularFleshBounce", 2972, 7 },
  { "perpendicularFoliageBounce", 2976, 7 },
  { "perpendicularFruitBounce", 3052, 7 },
  { "perpendicularGlassBounce", 2980, 7 },
  { "perpendicularGrassBounce", 2984, 7 },
  { "perpendicularGravelBounce", 2988, 7 },
  { "perpendicularIceBounce", 2992, 7 },
  { "perpendicularMetalBounce", 2996, 7 },
  { "perpendicularMudBounce", 3000, 7 },
  { "perpendicularPaintedMetalBounce", 3056, 7 },
  { "perpendicularPaperBounce", 3004, 7 },
  { "perpendicularPlasterBounce", 3008, 7 },
  { "perpendicularPlasticBounce", 3040, 7 },
  { "perpendicularRockBounce", 3012, 7 },
  { "perpendicularRubberBounce", 3044, 7 },
  { "perpendicularSandBounce", 3016, 7 },
  { "perpendicularSnowBounce", 3020, 7 },
  { "perpendicularWaterBounce", 3024, 7 },
  { "perpendicularWoodBounce", 3028, 7 },
  { "projTrailEffect", 1852, 9 },
  { "projectileRed", 1856, 7 },
  { "projectileGreen", 1860, 7 },
  { "projectileBlue", 1864, 7 },
  { "guidedMissileType", 1868, 26 },
  { "maxSteeringAccel", 1872, 7 },
  { "projIgnitionDelay", 1876, 4 },
  { "projIgnitionEffect", 1880, 9 },
  { "projIgnitionSound", 1884, 0 },
  { "tagFx_preparationEffect", 2268, 9 },
  { "tagFlash_preparationEffect", 2272, 9 },
  { "adsTransInTime", 52, 8 },
  { "adsTransOutTime", 56, 8 },
  { "adsIdleAmount", 1508, 7 },
  { "adsIdleSpeed", 1516, 7 },
  { "adsZoomFov1", 100, 7 },
  { "adsZoomFov2", 104, 7 },
  { "adsZoomFov3", 108, 7 },
  { "adsZoomInFrac", 112, 7 },
  { "adsZoomOutFrac", 116, 7 },
  { "adsOverlayShader", 140, 12 },
  { "adsOverlayShaderLowRes", 144, 12 },
  { "adsOverlayReticle", 1432, 17 },
  { "adsOverlayInterface", 1436, 30 },
  { "adsOverlayWidth", 1440, 7 },
  { "adsOverlayHeight", 1444, 7 },
  { "adsOverlayAlphaScale", 120, 7 },
  { "adsBobFactor", 1448, 7 },
  { "adsViewBobMult", 1452, 7 },
  { "adsAimPitch", 1888, 7 },
  { "adsCrosshairInFrac", 1892, 7 },
  { "adsCrosshairOutFrac", 1896, 7 },
  { "adsReloadTransTime", 2080, 8 },
  { "adsGunKickReducedKickBullets", 1900, 4 },
  { "adsGunKickReducedKickPercent", 1904, 7 },
  { "adsGunKickPitchMin", 1908, 7 },
  { "adsGunKickPitchMax", 1912, 7 },
  { "adsGunKickYawMin", 1916, 7 },
  { "adsGunKickYawMax", 1920, 7 },
  { "adsGunKickAccel", 1924, 7 },
  { "adsGunKickSpeedMax", 1928, 7 },
  { "adsGunKickSpeedDecay", 1932, 7 },
  { "adsGunKickStaticDecay", 1936, 7 },
  { "adsViewKickPitchMin", 1940, 7 },
  { "adsViewKickPitchMax", 1944, 7 },
  { "adsViewKickYawMin", 1948, 7 },
  { "adsViewKickYawMax", 1952, 7 },
  { "adsViewKickCenterSpeed", 92, 7 },
  { "adsSpread", 1964, 7 },
  { "guidedMissileType", 1868, 26 },
  { "hipSpreadStandMin", 1456, 7 },
  { "hipSpreadDuckedMin", 1460, 7 },
  { "hipSpreadProneMin", 1464, 7 },
  { "hipSpreadMax", 1468, 7 },
  { "hipSpreadDuckedMax", 1472, 7 },
  { "hipSpreadProneMax", 1476, 7 },
  { "hipSpreadDecayRate", 1480, 7 },
  { "hipSpreadFireAdd", 1484, 7 },
  { "hipSpreadTurnAdd", 1488, 7 },
  { "hipSpreadMoveAdd", 1492, 7 },
  { "hipSpreadDuckedDecay", 1496, 7 },
  { "hipSpreadProneDecay", 1500, 7 },
  { "hipReticleSidePos", 1504, 7 },
  { "hipIdleAmount", 1512, 7 },
  { "hipIdleSpeed", 1520, 7 },
  { "hipGunKickReducedKickBullets", 1968, 4 },
  { "hipGunKickReducedKickPercent", 1972, 7 },
  { "hipGunKickPitchMin", 1976, 7 },
  { "hipGunKickPitchMax", 1980, 7 },
  { "hipGunKickYawMin", 1984, 7 },
  { "hipGunKickYawMax", 1988, 7 },
  { "hipGunKickAccel", 1992, 7 },
  { "hipGunKickSpeedMax", 1996, 7 },
  { "hipGunKickSpeedDecay", 2000, 7 },
  { "hipGunKickStaticDecay", 2004, 7 },
  { "hipViewKickPitchMin", 2008, 7 },
  { "hipViewKickPitchMax", 2012, 7 },
  { "hipViewKickYawMin", 2016, 7 },
  { "hipViewKickYawMax", 2020, 7 },
  { "hipViewKickCenterSpeed", 96, 7 },
  { "leftArc", 2084, 7 },
  { "rightArc", 2088, 7 },
  { "topArc", 2092, 7 },
  { "bottomArc", 2096, 7 },
  { "accuracy", 2100, 7 },
  { "aiSpread", 2104, 7 },
  { "playerSpread", 2108, 7 },
  { "maxVertTurnSpeed", 2120, 7 },
  { "maxHorTurnSpeed", 2124, 7 },
  { "minVertTurnSpeed", 2112, 7 },
  { "minHorTurnSpeed", 2116, 7 },
  { "pitchConvergenceTime", 2128, 7 },
  { "yawConvergenceTime", 2132, 7 },
  { "suppressionTime", 2136, 7 },
  { "maxRange", 2140, 7 },
  { "animHorRotateInc", 2144, 7 },
  { "playerPositionDist", 2148, 7 },
  { "stance", 340, 20 },
  { "useHintString", 2152, 0 },
  { "dropHintString", 2156, 0 },
  { "horizViewJitter", 2168, 7 },
  { "vertViewJitter", 2172, 7 },
  { "fightDist", 2032, 7 },
  { "maxDist", 2036, 7 },
  { "aiVsAiAccuracyGraph", 2040, 0 },
  { "aiVsPlayerAccuracyGraph", 2044, 0 },
  { "locNone", 3068, 7 },
  { "locHelmet", 3072, 7 },
  { "locHead", 3076, 7 },
  { "locNeck", 3080, 7 },
  { "locTorsoUpper", 3084, 7 },
  { "locTorsoLower", 3088, 7 },
  { "locRightArmUpper", 3092, 7 },
  { "locRightArmLower", 3100, 7 },
  { "locRightHand", 3108, 7 },
  { "locLeftArmUpper", 3096, 7 },
  { "locLeftArmLower", 3104, 7 },
  { "locLeftHand", 3112, 7 },
  { "locRightLegUpper", 3116, 7 },
  { "locRightLegLower", 3124, 7 },
  { "locRightFoot", 3132, 7 },
  { "locLeftLegUpper", 3120, 7 },
  { "locLeftLegLower", 3128, 7 },
  { "locLeftFoot", 3136, 7 },
  { "locGun", 3140, 7 },
  { "fireRumble", 2212, 0 },
  { "meleeImpactRumble", 2216, 0 },
  { "reloadRumble", 2220, 0 },
  { "adsDofStart", 2224, 7 },
  { "adsDofEnd", 2228, 7 },
  { "scanSpeed", 2240, 7 },
  { "scanAccel", 2244, 7 },
  { "scanPauseTime", 2248, 8 },
  { "flameTableFirstPerson", 2252, 0 },
  { "flameTableThirdPerson", 2256, 0 },
  { "ikLeftHandOffsetF", 156, 7 },
  { "ikLeftHandOffsetR", 160, 7 },
  { "ikLeftHandOffsetU", 164, 7 },
  { "ikLeftHandRotationP", 168, 7 },
  { "ikLeftHandRotationY", 172, 7 },
  { "ikLeftHandRotationR", 176, 7 },
  { "ikLeftHandProneOffsetF", 180, 7 },
  { "ikLeftHandProneOffsetR", 184, 7 },
  { "ikLeftHandProneOffsetU", 188, 7 },
  { "ikLeftHandProneRotationP", 192, 7 },
  { "ikLeftHandProneRotationY", 196, 7 },
  { "ikLeftHandProneRotationR", 200, 7 },
  { "ikLeftHandUiViewerOffsetF", 204, 7 },
  { "ikLeftHandUiViewerOffsetR", 208, 7 },
  { "ikLeftHandUiViewerOffsetU", 212, 7 },
  { "ikLeftHandUiViewerRotationP", 216, 7 },
  { "ikLeftHandUiViewerRotationY", 220, 7 },
  { "ikLeftHandUiViewerRotationR", 224, 7 },
  { "parentWeaponName", 288, 0 },
  { "doGibbing", 2276, 5 },
  { "maxGibDistance", 2280, 7 }
};



const char *szWeapTypeNames[8] =
{
  "bullet",
  "grenade",
  "projectile",
  "binoculars",
  "gas",
  "bomb",
  "mine",
  "melee"
};

const char *szWeapClassNames[13] =
{
  "rifle",
  "mg",
  "smg",
  "spread",
  "pistol",
  "grenade",
  "rocketlauncher",
  "turret",
  "non-player",
  "gas",
  "item",
  "melee",
  "Killstreak Alt Stored Weapon"
};

const char *szWeapInventoryTypeNames[5] =
{ "primary", "offhand", "item", "altmode", "melee" };





char *__cdecl BG_GetPlayerAnimTypeName(int index)
{
    return g_playerAnimTypeNames[index];
}

int __cdecl BG_GetPlayerAnimTypeIndex(int typeEnum)
{
    int result; // eax

    switch ( typeEnum )
    {
        case 0:
            result = g_playerAnimTypeIndex_revivee;
            break;
        case 1:
            result = g_playerAnimTypeIndex_sniper;
            break;
        case 2:
            result = g_playerAnimTypeIndex_sniper_rearclip;
            break;
        case 3:
            result = g_playerAnimTypeIndex_briefcase;
            break;
        default:
            result = -1;
            break;
    }
    return result;
}

const char *__cdecl BG_GetWeaponTypeName(weapType_t type)
{
    if ( (unsigned int)type >= WEAPTYPE_NUM
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_load_obj.cpp",
                    1465,
                    0,
                    "type doesn't index ARRAY_COUNT( szWeapTypeNames )\n\t%i not in [0, %i)",
                    type,
                    8) )
    {
        __debugbreak();
    }
    return szWeapTypeNames[type];
}

const char *__cdecl BG_GetWeaponClassName(weapClass_t type)
{
    if ( (unsigned int)type >= WEAPCLASS_NUM
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_load_obj.cpp",
                    1473,
                    0,
                    "type doesn't index ARRAY_COUNT( szWeapClassNames )\n\t%i not in [0, %i)",
                    type,
                    13) )
    {
        __debugbreak();
    }
    return szWeapClassNames[type];
}

const char *__cdecl BG_GetWeaponInventoryTypeName(weapInventoryType_t type)
{
    if ( (unsigned int)type >= WEAPINVENTORYCOUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_load_obj.cpp",
                    1481,
                    0,
                    "type doesn't index ARRAY_COUNT( szWeapInventoryTypeNames )\n\t%i not in [0, %i)",
                    type,
                    5) )
    {
        __debugbreak();
    }
    return szWeapInventoryTypeNames[type];
}

void __cdecl BG_LoadWeaponStrings()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < g_playerAnimTypeNamesCount; ++i )
        BG_InitWeaponString(i, g_playerAnimTypeNames[i]);
}

void __cdecl BG_LoadPlayerAnimTypes()
{
    char v0; // [esp+53h] [ebp-29h]
    char *v1; // [esp+58h] [ebp-24h]
    const char *v2; // [esp+5Ch] [ebp-20h]
    char *buf; // [esp+70h] [ebp-Ch]
    const char *text_p; // [esp+74h] [ebp-8h] BYREF
    const char *token; // [esp+78h] [ebp-4h]

    g_playerAnimTypeNamesCount = 0;
    g_playerAnimTypeIndex_revivee = -1;
    buf = Com_LoadRawTextFile("mp/playeranimtypes.txt");
    if ( !buf )
        Com_Error(ERR_DROP, "Couldn't load file '%s'", "mp/playeranimtypes.txt");
    text_p = buf;
    Com_BeginParseSession("BG_AnimParseAnimScript");
    while ( 1 )
    {
        token = (const char *)Com_Parse(&text_p);
        if ( !token || !*token )
            break;
        if ( g_playerAnimTypeNamesCount >= 0x20 )
            Com_Error(ERR_DROP, "Player anim type array size exceeded");
        g_playerAnimTypeNames[g_playerAnimTypeNamesCount] = (char *)Hunk_Alloc(
                                                                                                                                    strlen(token) + 1,
                                                                                                                                    "BG_LoadPlayerAnimTypes",
                                                                                                                                    10);
        v2 = token;
        v1 = g_playerAnimTypeNames[g_playerAnimTypeNamesCount];
        do
        {
            v0 = *v2;
            *v1++ = *v2++;
        }
        while ( v0 );
        if ( !strcmp(token, "revivee") )
        {
            g_playerAnimTypeIndex_revivee = g_playerAnimTypeNamesCount;
        }
        else if ( !strcmp(token, "sniper") )
        {
            g_playerAnimTypeIndex_sniper = g_playerAnimTypeNamesCount;
        }
        else if ( !strcmp(token, "briefcase") )
        {
            g_playerAnimTypeIndex_briefcase = g_playerAnimTypeNamesCount;
        }
        else if ( !strcmp(token, "rearclipsniper") )
        {
            g_playerAnimTypeIndex_sniper_rearclip = g_playerAnimTypeNamesCount;
        }
        ++g_playerAnimTypeNamesCount;
    }
    Com_EndParseSession();
    Com_UnloadRawTextFile(buf);
}

void __cdecl BG_ClearWeaponDefInternal()
{
    surfaceTypeSoundListCount = 0;
}

void __cdecl SetConfigString(char **ppszConfigString, const char *pszKeyValue)
{
    char v2; // [esp+3h] [ebp-21h]
    char *v3; // [esp+8h] [ebp-1Ch]
    const char *v4; // [esp+Ch] [ebp-18h]
    char *buf; // [esp+20h] [ebp-4h]

    if ( *pszKeyValue )
    {
        buf = (char *)Hunk_AllocLowAlign(strlen(pszKeyValue) + 1, 1, "SetConfigString", 10);
        v4 = pszKeyValue;
        v3 = buf;
        do
        {
            v2 = *v4;
            *v3++ = *v4++;
        }
        while ( v2 );
        *ppszConfigString = buf;
    }
    else
    {
        *ppszConfigString = (char *)"";
    }
}

flameTable *__cdecl BG_LoadFlameTableInternal(const char *folder, char *name)
{
    char buffer[10244]; // [esp+14h] [ebp-2858h] BYREF
    int f; // [esp+2818h] [ebp-54h] BYREF
    int len; // [esp+281Ch] [ebp-50h]
    signed int v6; // [esp+2820h] [ebp-4Ch]
    char dest[64]; // [esp+2824h] [ebp-48h] BYREF
    flameTable *fTable; // [esp+2868h] [ebp-4h]

    len = strlen("FLAMETABLEFILE");
    fTable = (flameTable *)Hunk_AllocLow(0x1DCu, "BG_LoadFlameTableInternal", 10);
    InitFlameTable(fTable);
    Com_sprintf(dest, 0x40u, "weapons/%s/%s", folder, name);
    v6 = FS_FOpenFileByMode(dest, &f, FS_READ);
    if ( v6 >= 0 )
    {
        FS_Read((unsigned __int8 *)buffer, len, f);
        buffer[len] = 0;
        if ( !strncmp(buffer, "FLAMETABLEFILE", len) )
        {
            if ( (unsigned int)(v6 - len) < 0x2800 )
            {
                memset(buffer, 0, 0x2800u);
                FS_Read((unsigned __int8 *)buffer, v6 - len, f);
                buffer[v6 - len] = 0;
                FS_FCloseFile(f);
                if ( Info_Validate(buffer) )
                {
                    SetConfigString((char **)&fTable->name, name);
                    if ( ParseConfigStringToStruct(
                                 (unsigned __int8 *)fTable,
                                 flameTableFields,
                                 119,
                                 buffer,
                                 0,
                                 0,
                                 SetConfigString2) )
                    {
                        if ( bg_nextParseFlameTableIndex >= 8
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_load_obj.cpp",
                                        2123,
                                        0,
                                        "bg_nextParseFlameTableIndex doesn't index ARRAY_COUNT( bg_flameTables )\n\t%i not in [0, %i)",
                                        bg_nextParseFlameTableIndex,
                                        8) )
                        {
                            __debugbreak();
                        }
                        bg_flameTables[bg_nextParseFlameTableIndex++] = fTable;
                        return fTable;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    Com_PrintWarning(17, "WARNING: \"%s\" is not a valid flametable file\n", dest);
                    return 0;
                }
            }
            else
            {
                Com_PrintWarning(
                    17,
                    "WARNING: \"%s\" Is too long of a flametable file to parse (fileLength = %d identifierLength = %d)\n",
                    dest,
                    v6,
                    len);
                FS_FCloseFile(f);
                return 0;
            }
        }
        else
        {
            Com_PrintWarning(17, "WARNING: \"%s\" does not appear to be a flametable file\n", dest);
            FS_FCloseFile(f);
            return 0;
        }
    }
    else
    {
        Com_PrintWarning(17, "WARNING: Could not load flametable file '%s'\n", dest);
        return 0;
    }
}

void __cdecl SetConfigString2(unsigned __int8 *pMember, const char *pszKeyValue)
{
    SetConfigString((char **)pMember, pszKeyValue);
}

void __cdecl InitFlameTable(flameTable *fTable)
{
    const cspField_t *pField; // [esp+4h] [ebp-8h]
    int iField; // [esp+8h] [ebp-4h]

    iField = 0;
    pField = flameTableFields;
    while ( iField < 119 )
    {
        if ( !pField->iFieldType )
            *(unsigned int *)((char *)&fTable->flameVar_streamChunkGravityStart + pField->iOffset) = (unsigned int)"";
        ++iField;
        ++pField;
    }
}

flameTable *__cdecl BG_GetFlameTable(const char *folder, char *name)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < bg_nextParseFlameTableIndex; ++i )
    {
        if ( !I_stricmp(bg_flameTables[i]->name, name) )
            return bg_flameTables[i];
    }
    return BG_LoadFlameTableInternal(folder, name);
}

char __cdecl BG_LoadWeaponFile(char *szFileName, char *szBuffer, int iBufferSize)
{
    int hFile; // [esp+14h] [ebp-Ch] BYREF
    int iIdentifierLen; // [esp+18h] [ebp-8h]
    int iFileLength; // [esp+1Ch] [ebp-4h]

    iIdentifierLen = strlen("WEAPONFILE");
    iFileLength = FS_FOpenFileByMode(szFileName, &hFile, FS_READ);
    if ( iFileLength >= 0 )
    {
        FS_Read((unsigned __int8 *)szBuffer, iIdentifierLen, hFile);
        szBuffer[iIdentifierLen] = 0;
        if ( !strncmp(szBuffer, "WEAPONFILE", iIdentifierLen) )
        {
            if ( iFileLength - iIdentifierLen < iBufferSize )
            {
                memset(szBuffer, 0, iBufferSize);
                FS_Read((unsigned __int8 *)szBuffer, iFileLength - iIdentifierLen, hFile);
                szBuffer[iFileLength - iIdentifierLen] = 0;
                FS_FCloseFile(hFile);
                if ( Info_Validate(szBuffer) )
                {
                    return 1;
                }
                else
                {
                    Com_PrintWarning(17, "WARNING: \"%s\" is not a valid weapon file\n", szFileName);
                    return 0;
                }
            }
            else
            {
                Com_PrintWarning(
                    17,
                    "WARNING: \"%s\" Is too long of a weapon file to parse (fileLength = %d identifierLength = %d)\n",
                    szFileName,
                    iFileLength,
                    iIdentifierLen);
                FS_FCloseFile(hFile);
                return 0;
            }
        }
        else
        {
            Com_PrintWarning(17, "WARNING: \"%s\" does not appear to be a weapon file\n", szFileName);
            FS_FCloseFile(hFile);
            return 0;
        }
    }
    else
    {
        Com_PrintWarning(17, "WARNING: Could not load weapon file '%s'\n", szFileName);
        return 0;
    }
}

void __cdecl BG_LoadWeaponMergeSupport()
{
    char v0; // [esp+3h] [ebp-2Dh]
    char *v1; // [esp+8h] [ebp-28h]
    const char *v2; // [esp+Ch] [ebp-24h]
    char *buf; // [esp+24h] [ebp-Ch]
    const char *text_p; // [esp+28h] [ebp-8h] BYREF
    const char *token; // [esp+2Ch] [ebp-4h]

    s_numWeaponGripAnimSubstrings = 0;
    if ( !useFastFile->current.enabled )
    {
        BG_LoadWeaponAttachmentTable();
        buf = Com_LoadRawTextFile("weapon_gripanims.csv");
        if ( !buf )
            Com_Error(ERR_DROP, "Couldn't load file '%s'", "weapon_gripanims.csv");
        text_p = buf;
        Com_BeginParseSession("BG_LoadWeaponMergeSupport");
        while ( 1 )
        {
            token = (const char *)Com_Parse(&text_p);
            if ( !token || !*token )
                break;
            if ( (unsigned int)s_numWeaponGripAnimSubstrings >= 0x40 )
                Com_Error(ERR_DROP, "Weapon grip anim substrings table size exceeded");
            s_weaponGripAnimSubstrings[s_numWeaponGripAnimSubstrings] = (char *)Hunk_Alloc(
                                                                                                                                                        strlen(token) + 1,
                                                                                                                                                        "BG_LoadWeaponMergeSupport",
                                                                                                                                                        10);
            v2 = token;
            v1 = s_weaponGripAnimSubstrings[s_numWeaponGripAnimSubstrings];
            do
            {
                v0 = *v2;
                *v1++ = *v2++;
            }
            while ( v0 );
            ++s_numWeaponGripAnimSubstrings;
        }
        Com_EndParseSession();
        Com_UnloadRawTextFile(buf);
    }
}

char __cdecl BG_SplitWeaponDefNames(
                const char *name,
                char **sources,
                WeaponComponentList *componentAll,
                char *outputName)
{
    int slot; // [esp+0h] [ebp-27Ch]
    WeaponComponentList componentBase; // [esp+4h] [ebp-278h] BYREF
    WeaponComponentList componentBaseAttachment; // [esp+13Ch] [ebp-140h] BYREF
    int count; // [esp+278h] [ebp-4h]

    BG_WeaponNameToComponentList(name, componentAll);
    BG_WeaponComponentListIdentify(componentAll);
    BG_WeaponComponentListSort(name, componentAll, outputName);
    count = BG_WeaponComponentListCountAttachments(componentAll);
    if ( count <= 5 )
    {
        if ( count >= 2 )
        {
            for ( slot = 1; slot < count + 1; ++slot )
            {
                BG_WeaponComponentListNthAttachment(componentAll, slot, &componentBaseAttachment);
                BG_WeaponComponentListToName(&componentBaseAttachment, sources[slot], 64);
            }
            BG_WeaponComponentListRemoveAllAttachments(componentAll, &componentBase);
            BG_WeaponComponentListToName(&componentBase, *sources, 64);
            return 1;
        }
        else
        {
            Com_PrintError(1, "Can not create merged weapon %s.    Too few attachments, must have at least two.\n", name);
            return 0;
        }
    }
    else
    {
        Com_PrintError(1, "Can not create merged weapon %s.    Too many attachments, max is %i\n", name, 5);
        return 0;
    }
}

int __cdecl BG_MergeWeaponDefClipName(char **value, char *mergedValue, int size)
{
    char v4; // [esp+3h] [ebp-529h]
    WeaponComponent *v5; // [esp+8h] [ebp-524h]
    WeaponComponent *v6; // [esp+Ch] [ebp-520h]
    char v7; // [esp+13h] [ebp-519h]
    WeaponComponent *v8; // [esp+18h] [ebp-514h]
    WeaponComponent *v9; // [esp+1Ch] [ebp-510h]
    char v10; // [esp+23h] [ebp-509h]
    WeaponComponent *v11; // [esp+28h] [ebp-504h]
    WeaponComponent *v12; // [esp+2Ch] [ebp-500h]
    int m; // [esp+30h] [ebp-4FCh]
    WeaponComponentList merged; // [esp+34h] [ebp-4F8h] BYREF
    int comp[3]; // [esp+170h] [ebp-3BCh] BYREF
    WeaponComponentList componentList[3]; // [esp+17Ch] [ebp-3B0h] BYREF

    memset(&merged, 0, sizeof(merged));
    for ( m = 0; m < 3; ++m )
        BG_WeaponNameToComponentList(value[m], &componentList[m]);
    memset(comp, 0, sizeof(comp));
    while ( comp[0] < componentList[0].numComponents )
    {
        if ( comp[1] < componentList[1].numComponents
            && I_stricmp(componentList[1].components[comp[1]].name, componentList[0].components[comp[0]].name) )
        {
            v12 = &componentList[1].components[comp[1]];
            v11 = &merged.components[merged.numComponents];
            do
            {
                v10 = v12->name[0];
                v11->name[0] = v12->name[0];
                v12 = (WeaponComponent *)((char *)v12 + 1);
                v11 = (WeaponComponent *)((char *)v11 + 1);
            }
            while ( v10 );
            ++merged.numComponents;
        }
        ++comp[1];
        if ( comp[2] < componentList[2].numComponents
            && I_stricmp(componentList[2].components[comp[2]].name, componentList[0].components[comp[0]].name) )
        {
            v9 = &componentList[2].components[comp[2]];
            v8 = &merged.components[merged.numComponents];
            do
            {
                v7 = v9->name[0];
                v8->name[0] = v9->name[0];
                v9 = (WeaponComponent *)((char *)v9 + 1);
                v8 = (WeaponComponent *)((char *)v8 + 1);
            }
            while ( v7 );
            ++merged.numComponents;
        }
        ++comp[2];
        v6 = &componentList[0].components[comp[0]];
        v5 = &merged.components[merged.numComponents];
        do
        {
            v4 = v6->name[0];
            v5->name[0] = v6->name[0];
            v6 = (WeaponComponent *)((char *)v6 + 1);
            v5 = (WeaponComponent *)((char *)v5 + 1);
        }
        while ( v4 );
        ++merged.numComponents;
        ++comp[0];
    }
    BG_WeaponComponentListToName(&merged, mergedValue, size);
    return 1;
}

int __cdecl BG_MergeWeaponDefFloatValue(char **value, char *mergedValue)
{
    float valueRight; // [esp+8h] [ebp-8h]
    float valueLeft; // [esp+Ch] [ebp-4h]

    valueLeft = atof(value[1]);
    valueRight = atof(value[2]);
    Com_sprintf(mergedValue, 4u, "%.2f", (float)((float)(valueLeft + valueRight) / 2.0));
    return 1;
}

int __cdecl BG_MergeWeaponDefHidetags(char **value, char *mergedValue, int size)
{
    int v4; // [esp+10h] [ebp-10E0h]
    int i; // [esp+14h] [ebp-10DCh]
    _BYTE v6[196]; // [esp+18h] [ebp-10D8h] BYREF
    char *s1; // [esp+DCh] [ebp-1014h]
    char s0[4100]; // [esp+E0h] [ebp-1010h] BYREF
    int j; // [esp+10E8h] [ebp-8h]
    char *data_p; // [esp+10ECh] [ebp-4h] BYREF

    v4 = 0;
    for ( i = 0; i < 3; ++i )
    {
        data_p = value[i];
        for ( s1 = (char *)Com_Parse((const char **)&data_p); s1 && *s1; s1 = (char *)Com_Parse((const char **)&data_p) )
        {
            if ( strlen(s1) >= 0x40
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_load_obj.cpp",
                            2474,
                            0,
                            "%s",
                            "strlen( token ) < MAX_TAG_LEN") )
            {
                __debugbreak();
            }
            for ( j = 0; j < v4; ++j )
            {
                if ( !I_stricmp(&s0[64 * j], s1) )
                {
                    v6[3 * j + i] = 1;
                    break;
                }
            }
            if ( j == v4 )
            {
                strncpy(&s0[64 * v4], s1, 0x40u);
                v6[3 * v4] = 0;
                v6[3 * v4 + 1] = 0;
                v6[3 * v4 + 2] = 0;
                v6[3 * v4++ + i] = 1;
            }
        }
    }
    *mergedValue = 0;
    for ( j = 0; j < v4; ++j )
    {
        if ( !v6[3 * j] || v6[3 * j + 1] && v6[3 * j + 2] )
        {
            if ( !v6[3 * j + 1]
                && !v6[3 * j + 2]
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_load_obj.cpp",
                            2505,
                            0,
                            "%s",
                            "tagsPresent[tag][MERGE_LEFT] || tagsPresent[tag][MERGE_RIGHT]") )
            {
                __debugbreak();
            }
            I_strncat(mergedValue, size, &s0[64 * j]);
            I_strncat(mergedValue, size, "\n");
        }
    }
    return 1;
}

bool __cdecl BG_IsAnimationField(const char *fieldName)
{
    const char *v2; // [esp+Ch] [ebp-8h]

    v2 = &fieldName[strlen(fieldName) + 1];
    return v2 - (fieldName + 1) >= 4 && I_stricmp(v2 - 5, "anim") == 0;
}

bool __cdecl BG_IsADSField(const char *fieldName)
{
    return I_strnicmp(fieldName, "ads", 3) == 0;
}

char __cdecl BG_IsGripAnimationName(const char *name)
{
    int idx; // [esp+0h] [ebp-4h]

    for ( idx = 0; idx < s_numWeaponGripAnimSubstrings; ++idx )
    {
        if ( I_stristr(name, s_weaponGripAnimSubstrings[idx]) )
            return 1;
    }
    return 0;
}

int __cdecl BG_MergeWeaponDefAnimations(const char *fieldName, char **value, char *mergedValue, unsigned int size)
{
    char rightGrip; // [esp+1h] [ebp-3h]
    char leftGrip; // [esp+2h] [ebp-2h]
    bool isADS; // [esp+3h] [ebp-1h]

    leftGrip = BG_IsGripAnimationName(value[1]);
    rightGrip = BG_IsGripAnimationName(value[2]);
    isADS = BG_IsADSField(fieldName);
    if ( leftGrip && rightGrip )
    {
        Com_PrintError(
            1,
            "Can't merge anim field '%s', both '%s' and '%s' are bottom attachment point animations",
            fieldName,
            value[1],
            value[2]);
        return 0;
    }
    if ( !leftGrip && !rightGrip )
    {
        Com_PrintError(
            1,
            "Can't merge anim field '%s', neither '%s' nor '%s' are bottom attachment point animations",
            fieldName,
            value[1],
            value[2]);
        return 0;
    }
    if ( !leftGrip
        && !rightGrip
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_load_obj.cpp",
                    2564,
                    0,
                    "%s",
                    "leftGrip || rightGrip") )
    {
        __debugbreak();
    }
    if ( !isADS )
    {
        if ( !leftGrip )
            goto LABEL_15;
LABEL_16:
        strncpy(mergedValue, value[1], size);
        return 1;
    }
    if ( !leftGrip )
        goto LABEL_16;
LABEL_15:
    strncpy(mergedValue, value[2], size);
    return 1;
}

int __cdecl BG_MergeWeaponDefSpecialCases(const char *fieldName, char **value, char *mergedValue, int size)
{
    if ( I_stricmp(fieldName, "displayName") )
    {
        if ( I_stricmp(fieldName, "clipName") )
        {
            if ( I_stricmp(fieldName, "hideTags") )
            {
                if ( I_stricmp(fieldName, "adsViewKickCenterSpeed")
                    && I_stricmp(fieldName, "hipViewKickCenterSpeed")
                    && I_stricmp(fieldName, "hipViewKickYawMax") )
                {
                    if ( BG_IsAnimationField(fieldName) )
                        return BG_MergeWeaponDefAnimations(fieldName, value, mergedValue, size);
                    else
                        return 0;
                }
                else
                {
                    return BG_MergeWeaponDefFloatValue(value, mergedValue);
                }
            }
            else
            {
                return BG_MergeWeaponDefHidetags(value, mergedValue, size);
            }
        }
        else
        {
            return BG_MergeWeaponDefClipName(value, mergedValue, size);
        }
    }
    else
    {
        _snprintf(mergedValue, size, "%s_DUAL_ATTACH", *value);
        return 1;
    }
}

char __cdecl BG_LoadWeaponVariantDefFile(WeaponFullDef *weapFullDef, const char *folder, char *name)
{
    char *sourceName; // [esp+10h] [ebp-2847Ch] BYREF
    char *v5; // [esp+14h] [ebp-28478h]
    _BYTE *v6; // [esp+18h] [ebp-28474h]
    char *pszBuffer; // [esp+1Ch] [ebp-28470h] BYREF
    char *v8; // [esp+20h] [ebp-2846Ch]
    char *v9; // [esp+24h] [ebp-28468h]
    int v10; // [esp+28h] [ebp-28464h]
    char *sources[6]; // [esp+2Ch] [ebp-28460h] BYREF
    WeaponComponentList componentAll; // [esp+44h] [ebp-28448h] BYREF
    char szBuffer[16384]; // [esp+17Ch] [ebp-28310h] BYREF
    char v14; // [esp+417Ch] [ebp-24310h] BYREF
    _BYTE v15[64]; // [esp+1817Ch] [ebp-10310h] BYREF
    char v16[66116]; // [esp+181BCh] [ebp-102D0h] BYREF
    char **ppszConfigString; // [esp+28400h] [ebp-8Ch]
    char outputName[64]; // [esp+28404h] [ebp-88h] BYREF
    char dest[64]; // [esp+28444h] [ebp-48h] BYREF
    int i; // [esp+28488h] [ebp-4h]

    ppszConfigString = (char **)weapFullDef;
    for ( i = 0; i < 4; ++i )
    {
        v16[0x4000 * i + 320] = 0;
        v16[64 * i + 65856] = 0;
    }
    outputName[0] = 0;
    Com_sprintf(dest, 0x40u, "weapons/%s/%s", folder, name);
    if ( (int)FS_FOpenFileByMode(dest, 0, FS_READ) <= 0 )
    {
        for ( i = 0; i < 6; ++i )
            sources[i] = (char*)&v15[64 * i];
        if ( !BG_SplitWeaponDefNames(name, sources, &componentAll, outputName) )
            return 0;
        if ( componentAll.numComponents - 1 > 6
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_load_obj.cpp",
                        2663,
                        0,
                        "%s",
                        "componentAll.numComponents - 1 <= MERGE_BASE_FILE_COUNT") )
        {
            __debugbreak();
        }
        for ( i = 0; i < componentAll.numComponents - 1; ++i )
        {
            Com_sprintf(dest, 0x40u, "weapons/%s/%s", folder, &v15[64 * i]);
            if ( !BG_LoadWeaponFile(dest, &szBuffer[0x4000 * i], 0x4000) )
            {
                Com_PrintError(
                    1,
                    "Can not create merged weapon %s.    Could not load base or attachment file %s",
                    outputName,
                    dest);
                return 0;
            }
        }
        SetConfigString(ppszConfigString, outputName);
        sourceName = (char*)v15;
        pszBuffer = szBuffer;
        v5 = v16;
        v8 = &v14;
        v10 = BG_WeaponComponentListCountAttachments(&componentAll);
        for ( i = 0; i < v10 - 1; ++i )
        {
            v6 = &v15[64 * i + 128];
            v9 = &szBuffer[0x4000 * i + 0x8000];
            if ( !ParseConfigStringToStructMerged(
                            (unsigned __int8 *)weapFullDef,
                            weaponDefFields,
                            748,
                            name,
                            (const char **)&pszBuffer,
                            (const char **)&sourceName,
                            &v16[0x4000 * i + 320],
                            43,
                            (int (__cdecl *)(unsigned __int8 *, const char *, const int, const int))BG_ParseWeaponDefSpecificFieldType,
                            SetConfigString2,
                            BG_MergeWeaponDefSpecialCases) )
                return 0;
            componentAll.numComponents = i + 3;
            BG_WeaponComponentListToName(&componentAll, &v16[64 * i + 65856], 64);
            strcat(&v16[64 * i + 65856], "_mp");
            v5 = &v16[64 * i + 65856];
            v8 = &v16[0x4000 * i + 320];
        }
    }
    else
    {
        if ( !BG_LoadWeaponFile(dest, szBuffer, 0x4000) )
            return 0;
        SetConfigString(ppszConfigString, name);
        if ( !ParseConfigStringToStruct(
                        (unsigned __int8 *)weapFullDef,
                        weaponDefFields,
                        748,
                        szBuffer,
                        43,
                        (int (__cdecl *)(unsigned __int8 *, const char *, const int, const int))BG_ParseWeaponDefSpecificFieldType,
                        SetConfigString2) )
            return 0;
    }
    return 1;
}

int __cdecl BG_ParseWeaponDefSpecificFieldType(WeaponFullDef *pStruct, const char *pValue, int iFieldType)
{
    unsigned __int16 StringOfSize; // ax
    unsigned __int16 v4; // ax
    unsigned __int16 LowercaseString; // ax
    unsigned __int16 v6; // ax
    int result; // eax
    char v8; // [esp+3h] [ebp-A9h]
    char *v9; // [esp+8h] [ebp-A4h]
    const char *v10; // [esp+Ch] [ebp-A0h]
    int v11; // [esp+10h] [ebp-9Ch]
    const char *pos; // [esp+48h] [ebp-64h] BYREF
    int numHideTags; // [esp+4Ch] [ebp-60h]
    WeaponFullDef *weapFullDef; // [esp+50h] [ebp-5Ch]
    int numNoteTrackMappings; // [esp+54h] [ebp-58h]
    WeaponVariantDef *weapVariantDef; // [esp+58h] [ebp-54h]
    char keyName[64]; // [esp+5Ch] [ebp-50h] BYREF
    int arrayIndex; // [esp+A0h] [ebp-Ch]
    const char *token; // [esp+A4h] [ebp-8h]
    WeaponDef *weapDef; // [esp+A8h] [ebp-4h]

    if ( !pStruct
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_load_obj.cpp", 1643, 0, "%s", "pStruct") )
    {
        __debugbreak();
    }
    if ( !pValue
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_load_obj.cpp", 1644, 0, "%s", "pValue") )
    {
        __debugbreak();
    }
    weapFullDef = pStruct;
    weapVariantDef = &pStruct->weapVariantDef;
    weapDef = &pStruct->weapDef;
    switch ( iFieldType )
    {
        case 15:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, szWeapTypeNames, 8);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown weapon type \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->weapType = (weapType_t)arrayIndex;
            goto LABEL_104;
        case 16:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, szWeapClassNames, 13);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown weapon class \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->weapClass = (weapClass_t)arrayIndex;
            goto LABEL_104;
        case 17:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, szWeapOverlayReticleNames, 2);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown weapon overlay reticle \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->overlayReticle = (weapOverlayReticle_t)arrayIndex;
            goto LABEL_104;
        case 18:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, penetrateTypeNames, 4);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown penetration type \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->penetrateType = (PenetrateType)arrayIndex;
            goto LABEL_104;
        case 19:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, impactTypeNames_0, 16);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown impact type \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->impactType = (ImpactType)arrayIndex;
            goto LABEL_104;
        case 20:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, szWeapStanceNames, 3);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown weapon stance \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->stance = (weapStance_t)arrayIndex;
            goto LABEL_104;
        case 21:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, szProjectileExplosionNames, 10);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown projectile explosion \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->projExplosion = (weapProjExposion_t)arrayIndex;
            goto LABEL_104;
        case 22:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, offhandClassNames, 5);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown offhand class \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->offhandClass = (OffhandClass)arrayIndex;
            goto LABEL_104;
        case 23:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, offhandSlotNames, 5);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown offhand slot \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->offhandSlot = (OffhandSlot)arrayIndex;
            goto LABEL_104;
        case 24:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, (const char **)g_playerAnimTypeNames, g_playerAnimTypeNamesCount);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown player anim type \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->playerAnimType = arrayIndex;
            goto LABEL_104;
        case 25:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, activeReticleNames, 3);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown active reticle type \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->activeReticleType = (activeReticleType_t)arrayIndex;
            goto LABEL_104;
        case 26:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, guidedMissileNames, 7);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown guided missile type \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->guidedMissileType = (guidedMissileType_t)arrayIndex;
            goto LABEL_104;
        case 27:
            weapDef->bounceSound = (const char **)BG_RegisterSurfaceTypeSounds(pValue);
            goto LABEL_104;
        case 28:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, stickinessNames, 6);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown weapon stickiness type \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->stickiness = (WeapStickinessType)arrayIndex;
            goto LABEL_104;
        case 29:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, rotateTypeNames, 3);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown weapon rotate type \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->rotateType = (WeapRotateType)arrayIndex;
            goto LABEL_104;
        case 30:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, overlayInterfaceNames, 3);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown weapon overlay interface \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->overlayInterface = (WeapOverlayInteface_t)arrayIndex;
            goto LABEL_104;
        case 31:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, szWeapInventoryTypeNames, 5);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown weapon inventory type \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->inventoryType = (weapInventoryType_t)arrayIndex;
            goto LABEL_104;
        case 32:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, szWeapFireTypeNames, 7);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown weapon fire type \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->fireType = (weapFireType_t)arrayIndex;
            goto LABEL_104;
        case 33:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, szWeapClipTypeNames, 6);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown weapon clip type \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->clipType = (weapClipType_t)arrayIndex;
            goto LABEL_104;
        case 34:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, ammoCounterClipNames, 7);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown weapon ammo counter clip \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->ammoCounterClip = (ammoCounterClipType_t)arrayIndex;
            goto LABEL_104;
        case 35:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, weapIconRatioNames, 3);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown weapon hud icon ratio \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->hudIconRatio = (weaponIconRatioType_t)arrayIndex;
            goto LABEL_104;
        case 36:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, weapIconRatioNames, 3);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown weapon ammo counter icon ratio \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->ammoCounterIconRatio = (weaponIconRatioType_t)arrayIndex;
            goto LABEL_104;
        case 37:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, weapIconRatioNames, 3);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown weapon kill icon ratio \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->killIconRatio = (weaponIconRatioType_t)arrayIndex;
            goto LABEL_104;
        case 38:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, weapIconRatioNames, 3);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown weapon dpad icon ratio \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapVariantDef->dpadIconRatio = (weaponIconRatioType_t)arrayIndex;
            goto LABEL_104;
        case 39:
            arrayIndex = Weapon_GetStringArrayIndex(pValue, weapIconRatioNames, 3);
            if ( arrayIndex < 0 )
                Com_Error(ERR_DROP, "Unknown weapon indicator icon ratio \"%s\" in '%s\"", pValue, weapVariantDef->szInternalName);
            weapDef->indicatorIconRatio = (weaponIconRatioType_t)arrayIndex;
            goto LABEL_104;
        case 40:
            numHideTags = 0;
            pos = pValue;
            while ( 1 )
            {
                token = (const char *)Com_Parse(&pos);
                if ( !pos )
                    break;
                if ( numHideTags >= 32 )
                    Com_Error(ERR_DROP, "maximum hide tags (%s) exceeded: %i > %i", token, numHideTags, 32);
                StringOfSize = SL_GetStringOfSize(SCRIPTINSTANCE_SERVER, (char *)token, 0, strlen(token) + 1, 10);
                weapVariantDef->hideTags[numHideTags] = StringOfSize;
                weapVariantDef->hideTags[numHideTags] = SL_ConvertToLowercase(
                                                                                                    SCRIPTINSTANCE_SERVER,
                                                                                                    weapVariantDef->hideTags[numHideTags],
                                                                                                    0,
                                                                                                    10);
                ++numHideTags;
            }
            goto LABEL_104;
        case 41:
            pos = pValue;
            token = (const char *)Com_Parse(&pos);
            if ( pos )
            {
                v4 = SL_GetStringOfSize(SCRIPTINSTANCE_SERVER, (char *)token, 0, strlen(token) + 1, 10);
                weapDef->explosionTag = v4;
                weapDef->explosionTag = SL_ConvertToLowercase(SCRIPTINSTANCE_SERVER, weapDef->explosionTag, 0, 10);
            }
            goto LABEL_104;
        case 42:
            numNoteTrackMappings = 0;
            pos = pValue;
            keyName[0] = 0;
            while ( 1 )
            {
                token = (const char *)Com_Parse(&pos);
                if ( !pos )
                    break;
                if ( numNoteTrackMappings >= 20 )
                    Com_Error(ERR_DROP, "Max notetrack-to-sound mappings (%i) exceeded with entry \"%s\".", 20, token);
                if ( keyName[0] )
                {
                    LowercaseString = SL_GetLowercaseString(keyName, 0, SCRIPTINSTANCE_SERVER);
                    weapFullDef->notetrackSoundMapKeys[numNoteTrackMappings] = LowercaseString;
                    v6 = SL_GetLowercaseString(token, 0, SCRIPTINSTANCE_SERVER);
                    weapFullDef->notetrackSoundMapValues[numNoteTrackMappings++] = v6;
                    keyName[0] = 0;
                }
                else
                {
                    v11 = strlen(token);
                    if ( v11 >= 63 )
                        Com_Error(ERR_DROP, "Noetrack-to-sopund: keyname \"%s\" is too long (length %i/%i).", token, v11, 63);
                    v10 = token;
                    v9 = keyName;
                    do
                    {
                        v8 = *v10;
                        *v9++ = *v10++;
                    }
                    while ( v8 );
                }
            }
            if ( keyName[0] )
                Com_PrintWarning(
                    0,
                    "Notetrack-to-Sound: Weapon '%s' has bad entry; notetrack '%s' doesn't have a corresponding sound.\n",
                    weapVariantDef->szInternalName,
                    keyName);
LABEL_104:
            result = 1;
            break;
        default:
            Com_Error(ERR_DROP, "Bad field type %i in %s", iFieldType, weapVariantDef->szInternalName);
            result = 0;
            break;
    }
    return result;
}

int __cdecl Weapon_GetStringArrayIndex(const char *value, const char **stringArray, int arraySize)
{
    int arrayIndex; // [esp+0h] [ebp-4h]

    if ( !value
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_load_obj.cpp", 1561, 0, "%s", "value") )
    {
        __debugbreak();
    }
    if ( !stringArray
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_load_obj.cpp",
                    1562,
                    0,
                    "%s",
                    "stringArray") )
    {
        __debugbreak();
    }
    for ( arrayIndex = 0; arrayIndex < arraySize; ++arrayIndex )
    {
        if ( !I_stricmp(value, stringArray[arrayIndex]) )
            return arrayIndex;
    }
    return -1;
}

char **__cdecl BG_RegisterSurfaceTypeSounds(const char *surfaceSoundBase)
{
    const char *v2; // eax
    unsigned __int8 *v3; // eax
    char v4; // [esp+3h] [ebp-149h]
    char *v5; // [esp+8h] [ebp-144h]
    const char *v6; // [esp+Ch] [ebp-140h]
    char v7; // [esp+23h] [ebp-129h]
    char *v8; // [esp+28h] [ebp-124h]
    char *v9; // [esp+2Ch] [ebp-120h]
    char **result; // [esp+40h] [ebp-10Ch]
    char aliasName[256]; // [esp+44h] [ebp-108h] BYREF
    int i; // [esp+148h] [ebp-4h]

    if (!surfaceSoundBase
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_load_obj.cpp",
            1584,
            0,
            "%s",
            "surfaceSoundBase"))
    {
        __debugbreak();
    }
    if (!*surfaceSoundBase)
        return 0;
    for (i = 0; i < surfaceTypeSoundListCount; ++i)
    {
        if (!I_strcmp(surfaceTypeSoundLists[i].surfaceSoundBase, surfaceSoundBase))
            return surfaceTypeSoundLists[i].soundAliasList;
    }
    if (surfaceTypeSoundListCount == 16)
        Com_Error(ERR_DROP, "Exceeded MAX_SURFACE_TYPE_SOUND_LISTS (%d)", 16);
    result = (char **)Hunk_AllocLow(0x7Cu, "BG_RegisterSurfaceTypeSounds", 17);
    for (i = 0; i < 31; ++i)
    {
        v2 = Com_SurfaceTypeToName(i);
        Com_sprintf(aliasName, 0x100u, "%s_%s", surfaceSoundBase, v2);
        v3 = Hunk_AllocLow(&aliasName[strlen(aliasName) + 1] - &aliasName[1] + 1, "BG_RegisterSurfaceTypeSounds", 17);
        result[i] = (char *)v3;
        v9 = aliasName;
        v8 = result[i];
        do
        {
            v7 = *v9;
            *v8++ = *v9++;
        } while (v7);
    }
    surfaceTypeSoundLists[surfaceTypeSoundListCount].surfaceSoundBase = (char *)Hunk_AllocLow(
        strlen(surfaceSoundBase) + 1,
        "BG_RegisterSurfaceTypeSounds",
        17);
    v6 = surfaceSoundBase;
    v5 = surfaceTypeSoundLists[surfaceTypeSoundListCount].surfaceSoundBase;
    do
    {
        v4 = *v6;
        *v5++ = *v6++;
    } while (v4);
    surfaceTypeSoundLists[surfaceTypeSoundListCount++].soundAliasList = result;
    return result;
}

WeaponFullDef *__cdecl BG_LoadWeaponVariantDefInternal(const char *folder, char *name)
{
    WeaponFullDef *weapFullDef; // [esp+8h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+10h] [ebp-4h]

    weapFullDef = (WeaponFullDef *)Hunk_AllocLow(0xC48u, "BG_LoadWeaponVariantDefInternal", 10);
    InitWeaponDef(weapFullDef);
    weapDef = &weapFullDef->weapDef;
    if (!BG_LoadWeaponVariantDefFile(weapFullDef, folder, name))
        return 0;
    weapFullDef->weapDef.flameTableFirstPersonPtr = 0;
    weapFullDef->weapDef.flameTableThirdPersonPtr = 0;
    bg_nextParseFlameTableIndex = 0;
    if (*weapFullDef->weapDef.flameTableFirstPerson)
        weapFullDef->weapDef.flameTableFirstPersonPtr = BG_GetFlameTable(
            folder,
            (char *)weapFullDef->weapDef.flameTableFirstPerson);
    if (*weapFullDef->weapDef.flameTableThirdPerson)
        weapFullDef->weapDef.flameTableThirdPersonPtr = BG_GetFlameTable(
            folder,
            (char *)weapFullDef->weapDef.flameTableThirdPerson);
    if (I_stricmp(name, "defaultweapon_mp"))
    {
        if (!weapFullDef->weapDef.viewLastShotEjectEffect)
            weapFullDef->weapDef.viewLastShotEjectEffect = weapFullDef->weapDef.viewShellEjectEffect;
        if (!weapFullDef->weapDef.worldLastShotEjectEffect)
            weapFullDef->weapDef.worldLastShotEjectEffect = weapFullDef->weapDef.worldShellEjectEffect;
        if (!weapFullDef->weapDef.raiseSound)
            weapFullDef->weapDef.raiseSound = "wpn_default_raise";
        if (!weapFullDef->weapDef.putawaySound)
            weapFullDef->weapDef.putawaySound = "wpn_default_putaway";
        if (!weapFullDef->weapDef.pickupSound)
            weapFullDef->weapDef.pickupSound = "wpn_default_pickup";
        if (!weapFullDef->weapDef.ammoPickupSound)
            weapFullDef->weapDef.ammoPickupSound = "wpn_default_ammo_pickup";
        if (!weapFullDef->weapDef.emptyFireSound)
            weapFullDef->weapDef.emptyFireSound = "wpn_default_no_ammo";
    }
    BG_SetupTransitionTimes(&weapFullDef->weapVariantDef);
    BG_CheckWeaponDamageRanges(weapDef);
    if (weapFullDef->weapDef.enemyCrosshairRange > 15000.0)
        Com_Error(ERR_DROP, "Enemy crosshair ranges should be less than %f ", 15000.0);
    if (weapFullDef->weapDef.weapType == WEAPTYPE_PROJECTILE)
        BG_CheckProjectileValues(weapFullDef);
    if (!G_ParseWeaponAccurayGraphs(weapDef))
        return 0;
    if (weapFullDef->weapDef.sharedAmmo)
    {
        I_strlwr((char *)weapFullDef->weapVariantDef.szAmmoName);
        I_strlwr((char *)weapFullDef->weapVariantDef.szClipName);
    }
    else
    {
        weapFullDef->weapVariantDef.szAmmoName = "";
        weapFullDef->weapVariantDef.szClipName = "";
    }
    //BG_EvalVehicleName();
    return weapFullDef;
}

void __cdecl BG_SetupTransitionTimes(WeaponVariantDef *weapVariantDef)
{
    if ( weapVariantDef->iAdsTransInTime <= 0 )
        weapVariantDef->fOOPosAnimLength[0] = 1.0 / 300.0;
    else
        weapVariantDef->fOOPosAnimLength[0] = 1.0 / (float)weapVariantDef->iAdsTransInTime;
    if ( weapVariantDef->iAdsTransOutTime <= 0 )
        weapVariantDef->fOOPosAnimLength[1] = 1.0 / 500.0;
    else
        weapVariantDef->fOOPosAnimLength[1] = 1.0 / (float)weapVariantDef->iAdsTransOutTime;
}

void __cdecl BG_CheckWeaponDamageRanges(WeaponDef *weapDef)
{
    if ( weapDef->fMaxDamageRange <= 0.0 )
        weapDef->fMaxDamageRange = 999999.0f;
    if ( weapDef->fMinDamageRange <= 0.0 )
        weapDef->fMinDamageRange = 999999.12f;
}

void __cdecl BG_CheckProjectileValues(WeaponFullDef *weaponFullDef)
{
    if ( weaponFullDef->weapDef.weapType != WEAPTYPE_PROJECTILE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_load_obj.cpp",
                    1941,
                    0,
                    "%s",
                    "weaponDef->weapType == WEAPTYPE_PROJECTILE") )
    {
        __debugbreak();
    }
    if ( (float)weaponFullDef->weapDef.iProjectileSpeed <= 0.0 )
        Com_Error(
            ERR_DROP,
            "Projectile speed for WeapType %s must be greater than 0.0",
            weaponFullDef->weapVariantDef.szDisplayName);
    if ( weaponFullDef->weapDef.destabilizationCurvatureMax >= 1000000000.0
        || weaponFullDef->weapDef.destabilizationCurvatureMax < 0.0 )
    {
        Com_Error(
            ERR_DROP,
            "Destabilization angle for for WeapType %s must be between 0 and 45 degrees",
            weaponFullDef->weapVariantDef.szDisplayName);
    }
    if ( weaponFullDef->weapDef.destabilizationRateTime < 0.0 )
        Com_Error(
            ERR_DROP,
            "Destabilization rate time for for WeapType %s must be non-negative",
            weaponFullDef->weapVariantDef.szDisplayName);
}

void __cdecl InitWeaponDef(WeaponFullDef *weapFullDef)
{
    const cspField_t *pField; // [esp+8h] [ebp-Ch]
    int iField; // [esp+Ch] [ebp-8h]

    weapFullDef->weapVariantDef.weapDef = &weapFullDef->weapDef;
    weapFullDef->weapVariantDef.szInternalName = "";
    weapFullDef->weapVariantDef.szXAnims = weapFullDef->szXAnims;
    weapFullDef->weapVariantDef.hideTags = weapFullDef->hideTags;
    weapFullDef->weapDef.gunXModel = weapFullDef->gunXModel;
    weapFullDef->weapDef.notetrackSoundMapKeys = weapFullDef->notetrackSoundMapKeys;
    weapFullDef->weapDef.notetrackSoundMapValues = weapFullDef->notetrackSoundMapValues;
    weapFullDef->weapDef.worldModel = weapFullDef->worldModel;
    weapFullDef->weapDef.parallelBounce = weapFullDef->parallelBounce;
    weapFullDef->weapDef.perpendicularBounce = weapFullDef->perpendicularBounce;
    weapFullDef->weapDef.locationDamageMultipliers = weapFullDef->locationDamageMultipliers;
    iField = 0;
    pField = weaponDefFields;
    while ( iField < 748 )
    {
        if ( !pField->iFieldType )
            *(const char **)((char *)&weapFullDef->weapVariantDef.szInternalName + pField->iOffset) = "";
        ++iField;
        ++pField;
    }
}

char __cdecl BG_WeaponUpdateField(const char *weaponName, char *keyValue)
{
    WeaponFullDef weapFullDef; // [esp+8h] [ebp-C58h] BYREF
    WeaponVariantDef *weapVariantDef; // [esp+C58h] [ebp-8h]
    int w; // [esp+C5Ch] [ebp-4h]

    w = BG_FindWeaponIndexForName(weaponName);
    if ( w )
    {
        weapVariantDef = (WeaponVariantDef *)BG_GetWeaponVariantDef(w);
        memcpy(&weapFullDef, weapVariantDef, 0xE4u);
        memcpy(&weapFullDef.weapDef, weapVariantDef->weapDef, sizeof(weapFullDef.weapDef));
        if ( !ParseConfigStringToStruct(
                        (unsigned __int8 *)&weapFullDef,
                        weaponDefFields,
                        748,
                        keyValue,
                        43,
                        (int (__cdecl *)(unsigned __int8 *, const char *, const int, const int))BG_ParseWeaponDefSpecificFieldType,
                        SetConfigString2) )
            return 0;
        memcpy(weapVariantDef, &weapFullDef, sizeof(WeaponVariantDef));
        memcpy(weapVariantDef->weapDef, &weapFullDef.weapDef, sizeof(WeaponDef));
        BG_SetupTransitionTimes(weapVariantDef);
        BG_CheckWeaponDamageRanges(weapVariantDef->weapDef);
    }
    return 1;
}

bool __cdecl BG_FlameTableUpdateField(const char *flameTableName, char *keyValue)
{
    flameTable *ft; // [esp+0h] [ebp-4h]

    ft = Flame_FindFlameTable(flameTableName);
    return !ft
            || ParseConfigStringToStruct((unsigned __int8 *)ft, flameTableFields, 119, keyValue, 0, 0, SetConfigString2);
}

WeaponVariantDef *__cdecl BG_LoadDefaultWeaponVariantDef()
{
    if ( useFastFile->current.enabled )
        return (WeaponVariantDef *)((int (__cdecl *)(WeaponVariantDef *(__cdecl *)()))BG_LoadDefaultWeaponVariantDef_FastFile)(BG_LoadDefaultWeaponVariantDef_FastFile);
    else
        return (WeaponVariantDef *)((int (__cdecl *)(WeaponFullDef *(__cdecl *)()))BG_LoadDefaultWeaponVariantDef_LoadObj)(BG_LoadDefaultWeaponVariantDef_LoadObj);
}

WeaponFullDef *__cdecl BG_LoadDefaultWeaponVariantDef_LoadObj()
{
    InitWeaponDef(&bg_defaultWeaponFullDefs);
    bg_defaultWeaponFullDefs.weapVariantDef.weapDef = &bg_defaultWeaponFullDefs.weapDef;
    bg_defaultWeaponFullDefs.weapVariantDef.weapDef->playerAnimType = Weapon_GetStringArrayIndex(
                                                                                                                                            "default",
                                                                                                                                            (const char **)g_playerAnimTypeNames,
                                                                                                                                            g_playerAnimTypeNamesCount);
    if ( bg_defaultWeaponFullDefs.weapVariantDef.weapDef->playerAnimType < 0 )
        bg_defaultWeaponFullDefs.weapVariantDef.weapDef->playerAnimType = 0;
    bg_defaultWeaponFullDefs.weapVariantDef.szInternalName = "none";
    bg_defaultWeaponFullDefs.weapDef.accuracyGraphName[0] = "noweapon.accu";
    bg_defaultWeaponFullDefs.weapDef.accuracyGraphName[1] = "noweapon.accu";
    bg_defaultWeaponFullDefs.weapDef.sprintDurationScale = 1.75f;
    G_ParseWeaponAccurayGraphs(&bg_defaultWeaponFullDefs.weapDef);
    return &bg_defaultWeaponFullDefs;
}

WeaponVariantDef *__cdecl BG_LoadDefaultWeaponVariantDef_FastFile()
{
    return DB_FindXAssetHeader(ASSET_TYPE_WEAPON, (char*)"none", 1, -1).weapon;
}

