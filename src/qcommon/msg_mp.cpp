#include "msg_mp.h"
#include <server/sv_game.h>
#include <cstring>
#include <zlib/zlib.h>
#include "net_chan_mp.h"
#include "common.h"
#include "huffman.h"
#include "threads.h"
#include <bgame/bg_local.h>
#include "msg.h"
#include "sv_msg_write.h"
#include <bgame/bg_misc.h>
#include <demo/demo_profile.h>
#include <client_mp/cl_parse_mp.h>
#include <win32/win_shared.h>
#include <client/client.h>

const int msg_hData[256] =
{
  274054,
  68777,
  40460,
  40266,
  48059,
  39006,
  48630,
  27692,
  17712,
  15439,
  12386,
  10758,
  9420,
  9979,
  9346,
  15256,
  13184,
  14319,
  7750,
  7221,
  6095,
  5666,
  12606,
  7263,
  7322,
  5807,
  11628,
  6199,
  7826,
  6349,
  7698,
  9656,
  28968,
  5164,
  13629,
  6058,
  4745,
  4519,
  5199,
  4807,
  5323,
  3433,
  3455,
  3563,
  6979,
  5229,
  5002,
  4423,
  14108,
  13631,
  11908,
  11801,
  10261,
  7635,
  7215,
  7218,
  9353,
  6161,
  5689,
  4649,
  5026,
  5866,
  8002,
  10534,
  15381,
  8874,
  11798,
  7199,
  12814,
  6103,
  4982,
  5972,
  6779,
  4929,
  5333,
  3503,
  4345,
  6098,
  14117,
  16440,
  6446,
  3062,
  4695,
  3085,
  4198,
  4013,
  3878,
  3414,
  5514,
  4092,
  3261,
  4740,
  4544,
  3127,
  3385,
  7688,
  11126,
  6417,
  5297,
  4529,
  6333,
  4210,
  7056,
  4658,
  6190,
  3512,
  2843,
  3479,
  9369,
  5203,
  4980,
  5881,
  7509,
  4292,
  6097,
  5492,
  4648,
  2996,
  4988,
  4163,
  6534,
  4001,
  4342,
  4488,
  6039,
  4827,
  7112,
  8654,
  26712,
  8688,
  9677,
  9368,
  7209,
  3399,
  4473,
  4677,
  11087,
  4094,
  3404,
  4176,
  6733,
  3702,
  11420,
  4867,
  5968,
  3475,
  3722,
  3560,
  4571,
  2720,
  3189,
  3099,
  4595,
  4044,
  4402,
  3889,
  4989,
  3186,
  3153,
  5387,
  8020,
  3322,
  3775,
  2886,
  4191,
  2879,
  3110,
  2576,
  3693,
  2436,
  4935,
  3017,
  3538,
  5688,
  3444,
  3410,
  9170,
  4708,
  3425,
  3273,
  3684,
  4564,
  6957,
  4817,
  5224,
  3285,
  3143,
  4227,
  5630,
  6053,
  5851,
  6507,
  13692,
  8270,
  8260,
  5583,
  7568,
  4082,
  3984,
  4574,
  6440,
  3533,
  2992,
  2708,
  5190,
  3889,
  3799,
  4582,
  6020,
  3464,
  4431,
  3495,
  2906,
  2243,
  3856,
  3321,
  8759,
  3928,
  2905,
  3875,
  4382,
  3885,
  5869,
  6235,
  10685,
  4433,
  4639,
  4305,
  4683,
  2849,
  3379,
  4683,
  5477,
  4127,
  3853,
  3515,
  4913,
  3601,
  5237,
  6617,
  9019,
  4857,
  4112,
  5180,
  5998,
  4925,
  4986,
  6365,
  7930,
  5948,
  8085,
  7732,
  8643,
  8901,
  9653,
  32647
};


const NetField entityStateFields[69] =
{
  { "eType", 190, 2, -58, 0u, "MSG_FIELD_ETYPE", "0" },
  { "time2", 120, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "lerp.u.anonymous.data[0]", 84, 4, 32, 0u, "32", "0" },
  { "eventSequence", 210, 2, -59, 0u, "MSG_FIELD_EVENTSEQUENCE", "0" },
  { "lerp.eFlags", 4, 4, -98, 0u, "MSG_FIELD_EFLAGS", "0" },
  { "lerp.pos.trBase[1]", 28, 4, -65, 0u, "MSG_FIELD_ES_ORIGINY", "0" },
  { "lerp.pos.trBase[0]", 24, 4, -66, 0u, "MSG_FIELD_ES_ORIGINX", "0" },
  { "weapon", 202, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "lerp.pos.trBase[2]", 32, 4, -64, 0u, "MSG_FIELD_ES_ORIGINZ", "0" },
  { "lerp.eFlags2", 8, 4, 32, 0u, "32", "0" },
  { "lerp.apos.trBase[1]", 64, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.apos.trBase[0]", 60, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "events[1]", 177, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "events[2]", 178, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "events[0]", 176, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "events[3]", 179, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "un3", 152, 4, 32, 0u, "32", "0" },
  { "weaponModel", 216, 1, 4, 0u, "MAX_WEAPONMODEL_BITS", "0" },
  { "renderOptions", 132, 4, 32, 0u, "MAX_RENDER_OPTIONS_BITS", "0" },
  { "eventParms[0]", 180, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "surfType", 212, 1, 8, 0u, "8", "0" },
  { "targetname", 206, 2, 16, 0u, "16", "0" },
  { "index", 194, 2, 10, 0u, "SUBMODEL_BITS", "0" },
  { "solid", 128, 4, 24, 0u, "24", "0" },
  { "enemyModel", 200, 2, 10, 0u, "SUBMODEL_BITS", "0" },
  { "groundEntityNum", 192, 2, -96, 0u, "MSG_FIELD_GROUNDENTITY", "0" },
  { "un1", 217, 1, 8, 0u, "8", "0" },
  { "clientNum", 213, 1, 6, 0u, "CLIENT_BITS+1", "0" },
  { "lerp.pos.trDelta[0]", 36, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trDelta[1]", 40, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trDelta[2]", 44, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "eventParms[1]", 182, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "eventParms[2]", 184, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.pos.trTime", 16, 4, -68, 0u, "MSG_FIELD_POS_TRTIME", "0" },
  { "lerp.pos.trType", 12, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "eventParm", 188, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.apos.trType", 48, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "lerp.apos.trBase[2]", 68, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.apos.trTime", 52, 4, -70, 0u, "MSG_FIELD_ANGLE_TRTIME", "0" },
  { "lerp.apos.trDelta[0]", 72, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "lerp.apos.trDelta[2]", 80, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "eventParms[3]", 186, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.pos.trDuration", 20, 4, -67, 0u, "MSG_FIELD_POS_TRDURATION", "0" },
  { "lerp.apos.trDelta[1]", 76, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "attackerEntityNum", 198, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "un2.anim.fWaistPitch", 148, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "un2.anim.fTorsoPitch", 144, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "lerp.apos.trDuration", 56, 4, -69, 0u, "MSG_FIELD_ANGLE_TRDURATION", "0" },
  { "un2.anim.torsoAnim", 140, 4, 11, 0u, "ANIM_BITS", "0" },
  { "un2.anim.legsAnim", 136, 4, 11, 0u, "ANIM_BITS", "0" },
  { "loopSoundId", 124, 4, 32, 0u, "SOUNDALIAS_BITS", "0" },
  { "loopSoundFade", 208, 2, -16, 0u, "FADETIME_BITS", "0" },
  { "otherEntityNum", 196, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "lerp.useCount", 116, 4, 6, 0u, "USE_COUNT_BITS", "0" },
  { "lerp.u.anonymous.data[1]", 88, 4, 32, 0u, "32", "0" },
  { "lerp.u.anonymous.data[2]", 92, 4, 32, 0u, "32", "0" },
  { "lerp.u.anonymous.data[3]", 96, 4, 32, 0u, "32", "0" },
  { "lerp.u.anonymous.data[4]", 100, 4, 32, 0u, "32", "0" },
  { "lerp.u.anonymous.data[5]", 104, 4, 32, 0u, "32", "0" },
  { "clientLinkInfo", 218, 4, 26, 0u, "CLIENTLINKINFO_BITS", "0" },
  { "faction", 215, 1, 7, 0u, "2 + CLIENT_BITS", "0" },
  { "lerp.u.anonymous.data[6]", 108, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[7]", 112, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "iHeadIcon", 214, 1, 4, 1u, "HEAD_ICON_BITS", "NEVER_CHANGES" },
  { "partBits[0]", 156, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[1]", 160, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[2]", 164, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[3]", 168, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[4]", 172, 4, 32, 1u, "32", "NEVER_CHANGES" }
};


const int numArchivedEntityFields = 8;
const NetField archivedEntityFields[8] =
{
  { "absmin[1]", 12, 4, 0, 0u, NULL, NULL },
  { "absmax[1]", 24, 4, 0, 0u, NULL, NULL },
  { "absmin[0]", 8, 4, 0, 0u, NULL, NULL },
  { "absmax[0]", 20, 4, 0, 0u, NULL, NULL },
  { "absmax[2]", 28, 4, 0, 0u, NULL, NULL },
  { "absmin[2]", 16, 4, 0, 0u, NULL, NULL },
  { "svFlags", 0, 4, 32, 0u, NULL, NULL },
  { "clientMask[0]", 4, 4, 32, 0u, NULL, NULL }
};

const int numPlayerEntityStateFields = 74;
const NetField playerEntityStateFields[74] =
{
  { "eType", 190, 2, -58, 1u, "MSG_FIELD_ETYPE", "NEVER_CHANGES" },
  {
    "lerp.pos.trBase[0]",
    24,
    4,
    -66,
    2u,
    "MSG_FIELD_ES_ORIGINX",
    "ALWAYS_CHANGES"
  },
  {
    "lerp.pos.trBase[1]",
    28,
    4,
    -65,
    2u,
    "MSG_FIELD_ES_ORIGINY",
    "ALWAYS_CHANGES"
  },
  { "lerp.apos.trBase[1]", 64, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "eventSequence", 210, 2, -59, 0u, "MSG_FIELD_EVENTSEQUENCE", "0" },
  { "lerp.u.player.movementDir", 88, 4, -60, 0u, "MSG_FIELD_MOVEMENTDIR", "0" },
  { "lerp.apos.trBase[0]", 60, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.pos.trBase[2]", 32, 4, -64, 0u, "MSG_FIELD_ES_ORIGINZ", "0" },
  { "un2.anim.legsAnim", 136, 4, 11, 0u, "ANIM_BITS", "0" },
  { "lerp.u.player.moveType", 92, 4, 5, 0u, "ANIM_MOVETYPE_BITS", "0" },
  { "lerp.eFlags", 4, 4, -98, 0u, "MSG_FIELD_EFLAGS", "0" },
  { "events[0]", 176, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "events[3]", 179, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "events[2]", 178, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "events[1]", 177, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventParms[1]", 182, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "eventParms[3]", 186, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "eventParms[0]", 180, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "eventParms[2]", 184, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "solid", 128, 4, 24, 0u, "24", "0" },
  { "un2.anim.torsoAnim", 140, 4, 11, 0u, "ANIM_BITS", "0" },
  { "weapon", 202, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "lerp.u.player.primaryWeapon", 96, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "groundEntityNum", 192, 2, -96, 0u, "MSG_FIELD_GROUNDENTITY", "0" },
  { "partBits[2]", 164, 4, 32, 0u, "32", "0" },
  { "renderOptions", 132, 4, 32, 0u, "MAX_RENDER_OPTIONS_BITS", "0" },
  { "partBits[3]", 168, 4, 32, 0u, "32", "0" },
  { "lerp.u.player.stowedWeapon", 98, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  {
    "lerp.u.player.stowedWeaponCamo",
    109,
    1,
    6,
    0u,
    "RENDER_OPTIONS_CAMO_BITS",
    "0"
  },
  { "partBits[4]", 172, 4, 32, 0u, "32", "0" },
  { "un2.anim.fTorsoPitch", 144, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "un2.anim.fWaistPitch", 148, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "lerp.u.player.offhandWeapon", 100, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "lerp.apos.trBase[2]", 68, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "clientLinkInfo", 218, 4, 26, 0u, "CLIENTLINKINFO_BITS", "0" },
  { "lerp.eFlags2", 8, 4, 32, 0u, "32", "0" },
  { "lerp.u.player.meleeWeapon", 102, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "eventParm", 188, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.pos.trType", 12, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "lerp.apos.trType", 48, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "clientNum", 213, 1, 6, 0u, "CLIENT_BITS+1", "0" },
  { "otherEntityNum", 196, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "weaponModel", 216, 1, 4, 0u, "MAX_WEAPONMODEL_BITS", "0" },
  {
    "lerp.u.player.meleeWeaponModel",
    105,
    1,
    4,
    0u,
    "MAX_WEAPONMODEL_BITS",
    "0"
  },
  { "enemyModel", 200, 2, 4, 0u, "MAX_WEAPONMODEL_BITS", "0" },
  { "iHeadIcon", 214, 1, 4, 0u, "HEAD_ICON_BITS", "0" },
  { "lastStandPrevWeapon", 204, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "faction", 215, 1, 2, 0u, "2", "0" },
  { "lerp.u.player.leanf", 84, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "lerp.u.player.vehicleType", 106, 1, 3, 0u, "VEHICLE_TYPES_BITS", "0" },
  { "lerp.u.player.vehicleAnimBoneIndex", 107, 1, 8, 0u, "MAX_BONE_BITS", "0" },
  { "lerp.u.player.vehicleSeat", 108, 1, 4, 0u, "MAX_VEHICLE_SEAT_BITS", "0" },
  { "targetname", 206, 2, 16, 0u, "16", "0" },
  { "lerp.useCount", 116, 4, 6, 0u, "USE_COUNT_BITS", "0" },
  { "partBits[0]", 156, 4, 32, 0u, "32", "0" },
  { "partBits[1]", 160, 4, 32, 0u, "32", "0" },
  {
    "lerp.pos.trDelta[1]",
    40,
    4,
    -63,
    1u,
    "MSG_FIELD_ES_ORIGIN_DELTA",
    "NEVER_CHANGES"
  },
  {
    "lerp.pos.trDelta[0]",
    36,
    4,
    -63,
    1u,
    "MSG_FIELD_ES_ORIGIN_DELTA",
    "NEVER_CHANGES"
  },
  {
    "lerp.pos.trDuration",
    20,
    4,
    -67,
    1u,
    "MSG_FIELD_POS_TRDURATION",
    "NEVER_CHANGES"
  },
  { "lerp.pos.trTime", 16, 4, -68, 1u, "MSG_FIELD_POS_TRTIME", "NEVER_CHANGES" },
  {
    "lerp.pos.trDelta[2]",
    44,
    4,
    -63,
    1u,
    "MSG_FIELD_ES_ORIGIN_DELTA",
    "NEVER_CHANGES"
  },
  { "surfType", 212, 1, 8, 1u, "8", "NEVER_CHANGES" },
  { "un1", 217, 1, 8, 1u, "8", "NEVER_CHANGES" },
  { "index", 194, 2, 10, 1u, "SUBMODEL_BITS", "NEVER_CHANGES" },
  {
    "lerp.apos.trDelta[0]",
    72,
    4,
    -61,
    1u,
    "MSG_FIELD_ES_ANGLE_DELTA",
    "NEVER_CHANGES"
  },
  {
    "lerp.apos.trDelta[1]",
    76,
    4,
    -61,
    1u,
    "MSG_FIELD_ES_ANGLE_DELTA",
    "NEVER_CHANGES"
  },
  {
    "lerp.apos.trDelta[2]",
    80,
    4,
    -61,
    1u,
    "MSG_FIELD_ES_ANGLE_DELTA",
    "NEVER_CHANGES"
  },
  { "time2", 120, 4, -97, 1u, "MSG_FIELD_TIME", "NEVER_CHANGES" },
  { "loopSoundId", 124, 4, 32, 1u, "SOUNDALIAS_BITS", "NEVER_CHANGES" },
  { "loopSoundFade", 208, 2, -16, 1u, "FADETIME_BITS", "NEVER_CHANGES" },
  { "attackerEntityNum", 198, 2, 10, 1u, "GENTITYNUM_BITS", "NEVER_CHANGES" },
  {
    "lerp.apos.trTime",
    52,
    4,
    -70,
    1u,
    "MSG_FIELD_ANGLE_TRTIME",
    "NEVER_CHANGES"
  },
  {
    "lerp.apos.trDuration",
    56,
    4,
    -69,
    1u,
    "MSG_FIELD_ANGLE_TRDURATION",
    "NEVER_CHANGES"
  },
  { "un3", 152, 4, 32, 1u, "32", "NEVER_CHANGES" }
};

const int numCorpseEntityStateFields = 68;
const NetField corpseEntityStateFields[68] =
{
  { "eType", 190, 2, -58, 1u, "MSG_FIELD_ETYPE", "NEVER_CHANGES" },
  { "lerp.pos.trBase[0]", 24, 4, -66, 0u, "MSG_FIELD_ES_ORIGINX", "0" },
  { "lerp.pos.trBase[1]", 28, 4, -65, 0u, "MSG_FIELD_ES_ORIGINY", "0" },
  { "lerp.pos.trType", 12, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "lerp.eFlags", 4, 4, -98, 0u, "MSG_FIELD_EFLAGS", "0" },
  { "lerp.apos.trType", 48, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "lerp.pos.trBase[2]", 32, 4, -64, 0u, "MSG_FIELD_ES_ORIGINZ", "0" },
  { "lerp.apos.trBase[1]", 64, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.apos.trBase[0]", 60, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.apos.trBase[2]", 68, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.useCount", 116, 4, 6, 0u, "USE_COUNT_BITS", "0" },
  { "clientNum", 213, 1, 6, 0u, "CLIENT_BITS+1", "0" },
  { "renderOptions", 132, 4, 32, 0u, "MAX_RENDER_OPTIONS_BITS", "0" },
  { "un2.anim.legsAnim", 136, 4, 11, 0u, "ANIM_BITS", "0" },
  { "un2.anim.torsoAnim", 140, 4, 11, 0u, "ANIM_BITS", "0" },
  { "lerp.pos.trTime", 16, 4, -68, 0u, "MSG_FIELD_POS_TRTIME", "0" },
  { "lerp.pos.trDelta[0]", 36, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trDelta[1]", 40, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trDelta[2]", 44, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "groundEntityNum", 192, 2, -96, 0u, "MSG_FIELD_GROUNDENTITY", "0" },
  { "clientLinkInfo", 218, 4, 26, 0u, "CLIENTLINKINFO_BITS", "0" },
  { "lerp.eFlags2", 8, 4, 32, 0u, "32", "0" },
  { "lerp.u.player.stowedWeapon", 98, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "partBits[0]", 156, 4, 32, 0u, "32", "0" },
  { "partBits[1]", 160, 4, 32, 0u, "32", "0" },
  { "partBits[2]", 164, 4, 32, 0u, "32", "0" },
  { "partBits[3]", 168, 4, 32, 0u, "32", "0" },
  { "partBits[4]", 172, 4, 32, 0u, "32", "0" },
  { "targetname", 206, 2, 16, 1u, "16", "NEVER_CHANGES" },
  { "enemyModel", 200, 2, 10, 1u, "SUBMODEL_BITS", "NEVER_CHANGES" },
  {
    "lerp.u.player.movementDir",
    88,
    4,
    -60,
    1u,
    "MSG_FIELD_MOVEMENTDIR",
    "NEVER_CHANGES"
  },
  {
    "lerp.u.player.moveType",
    92,
    4,
    5,
    1u,
    "ANIM_MOVETYPE_BITS",
    "NEVER_CHANGES"
  },
  {
    "eventSequence",
    210,
    2,
    -59,
    1u,
    "MSG_FIELD_EVENTSEQUENCE",
    "NEVER_CHANGES"
  },
  { "events[0]", 176, 1, -94, 1u, "MSG_FIELD_EVENT", "NEVER_CHANGES" },
  { "events[1]", 177, 1, -94, 1u, "MSG_FIELD_EVENT", "NEVER_CHANGES" },
  { "events[2]", 178, 1, -94, 1u, "MSG_FIELD_EVENT", "NEVER_CHANGES" },
  { "events[3]", 179, 1, -94, 1u, "MSG_FIELD_EVENT", "NEVER_CHANGES" },
  { "un2.anim.fTorsoPitch", 144, 4, 0, 1u, "MSG_FIELD_FLOAT", "NEVER_CHANGES" },
  { "eventParms[1]", 182, 2, -93, 1u, "MSG_FIELD_EVENTPARAM", "NEVER_CHANGES" },
  { "eventParms[0]", 180, 2, -93, 1u, "MSG_FIELD_EVENTPARAM", "NEVER_CHANGES" },
  { "eventParms[2]", 184, 2, -93, 1u, "MSG_FIELD_EVENTPARAM", "NEVER_CHANGES" },
  { "weapon", 202, 2, 11, 1u, "MAX_WEAPONS_BITS", "NEVER_CHANGES" },
  { "weaponModel", 216, 1, 4, 1u, "MAX_WEAPONMODEL_BITS", "NEVER_CHANGES" },
  { "eventParms[3]", 186, 2, -93, 1u, "MSG_FIELD_EVENTPARAM", "NEVER_CHANGES" },
  { "solid", 128, 4, 24, 1u, "24", "NEVER_CHANGES" },
  {
    "lerp.pos.trDuration",
    20,
    4,
    -67,
    1u,
    "MSG_FIELD_POS_TRDURATION",
    "NEVER_CHANGES"
  },
  { "un2.anim.fWaistPitch", 148, 4, 0, 1u, "MSG_FIELD_FLOAT", "NEVER_CHANGES" },
  { "eventParm", 188, 2, -93, 1u, "MSG_FIELD_EVENTPARAM", "NEVER_CHANGES" },
  { "iHeadIcon", 214, 1, 4, 1u, "HEAD_ICON_BITS", "NEVER_CHANGES" },
  { "faction", 215, 1, 7, 1u, "2 + CLIENT_BITS", "NEVER_CHANGES" },
  { "surfType", 212, 1, 8, 1u, "8", "NEVER_CHANGES" },
  { "un1", 217, 1, 8, 1u, "8", "NEVER_CHANGES" },
  { "otherEntityNum", 196, 2, 10, 1u, "GENTITYNUM_BITS", "NEVER_CHANGES" },
  { "index", 194, 2, 10, 1u, "SUBMODEL_BITS", "NEVER_CHANGES" },
  {
    "lerp.apos.trDelta[0]",
    72,
    4,
    -61,
    1u,
    "MSG_FIELD_ES_ANGLE_DELTA",
    "NEVER_CHANGES"
  },
  {
    "lerp.apos.trDelta[1]",
    76,
    4,
    -61,
    1u,
    "MSG_FIELD_ES_ANGLE_DELTA",
    "NEVER_CHANGES"
  },
  {
    "lerp.apos.trDelta[2]",
    80,
    4,
    -61,
    1u,
    "MSG_FIELD_ES_ANGLE_DELTA",
    "NEVER_CHANGES"
  },
  { "time2", 120, 4, -97, 1u, "MSG_FIELD_TIME", "NEVER_CHANGES" },
  { "loopSoundId", 124, 4, 32, 1u, "SOUNDALIAS_BITS", "NEVER_CHANGES" },
  { "loopSoundFade", 208, 2, -16, 1u, "FADETIME_BITS", "NEVER_CHANGES" },
  { "attackerEntityNum", 198, 2, 10, 1u, "GENTITYNUM_BITS", "NEVER_CHANGES" },
  {
    "lerp.apos.trTime",
    52,
    4,
    -70,
    1u,
    "MSG_FIELD_ANGLE_TRTIME",
    "NEVER_CHANGES"
  },
  { "lerp.u.player.leanf", 84, 4, 0, 1u, "MSG_FIELD_FLOAT", "NEVER_CHANGES" },
  {
    "lerp.apos.trDuration",
    56,
    4,
    -69,
    1u,
    "MSG_FIELD_ANGLE_TRDURATION",
    "NEVER_CHANGES"
  },
  { "un3", 152, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[5]", 104, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[6]", 108, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[7]", 112, 4, 32, 1u, "32", "NEVER_CHANGES" }
};


const int numItemEntityStateFields = 69;
const NetField itemEntityStateFields[69] =
{
  { "eType", 190, 2, -58, 0u, "MSG_FIELD_ETYPE", "0" },
  { "lerp.pos.trType", 12, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "lerp.pos.trTime", 16, 4, -68, 0u, "MSG_FIELD_POS_TRTIME", "0" },
  { "lerp.pos.trDelta[2]", 44, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trBase[2]", 32, 4, -64, 0u, "MSG_FIELD_ES_ORIGINZ", "0" },
  { "lerp.pos.trDelta[0]", 36, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trDelta[1]", 40, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trBase[1]", 28, 4, -65, 0u, "MSG_FIELD_ES_ORIGINY", "0" },
  { "lerp.pos.trBase[0]", 24, 4, -66, 0u, "MSG_FIELD_ES_ORIGINX", "0" },
  { "groundEntityNum", 192, 2, -96, 0u, "MSG_FIELD_GROUNDENTITY", "0" },
  { "lerp.apos.trBase[2]", 68, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "clientNum", 213, 1, 6, 0u, "CLIENT_BITS+1", "0" },
  { "lerp.apos.trBase[0]", 60, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.apos.trBase[1]", 64, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.apos.trType", 48, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "lerp.apos.trTime", 52, 4, -70, 0u, "MSG_FIELD_ANGLE_TRTIME", "0" },
  { "lerp.apos.trDelta[0]", 72, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "lerp.apos.trDelta[1]", 76, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "lerp.apos.trDelta[2]", 80, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "un3", 152, 4, 32, 0u, "32", "0" },
  { "lerp.pos.trDuration", 20, 4, -67, 0u, "MSG_FIELD_POS_TRDURATION", "0" },
  { "solid", 128, 4, 24, 0u, "24", "0" },
  { "eventParm", 188, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.eFlags", 4, 4, -98, 0u, "MSG_FIELD_EFLAGS", "0" },
  { "lerp.eFlags2", 8, 4, 32, 0u, "32", "0" },
  { "eventSequence", 210, 2, -59, 0u, "MSG_FIELD_EVENTSEQUENCE", "0" },
  { "events[0]", 176, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventParms[0]", 180, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "weapon", 202, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "weaponModel", 216, 1, 4, 0u, "MAX_WEAPONMODEL_BITS", "0" },
  { "renderOptions", 132, 4, 32, 0u, "MAX_RENDER_OPTIONS_BITS", "0" },
  { "surfType", 212, 1, 8, 0u, "8", "0" },
  { "otherEntityNum", 196, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "lerp.u.anonymous.data[0]", 84, 4, 32, 0u, "32", "0" },
  { "lerp.u.anonymous.data[1]", 88, 4, 32, 0u, "32", "0" },
  { "lerp.u.anonymous.data[2]", 92, 4, 32, 0u, "32", "0" },
  { "eventParms[1]", 182, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "eventParms[2]", 184, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "eventParms[3]", 186, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "events[1]", 177, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "events[2]", 178, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "events[3]", 179, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "attackerEntityNum", 198, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "un1", 217, 1, 8, 0u, "8", "0" },
  { "time2", 120, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "loopSoundId", 124, 4, 32, 0u, "SOUNDALIAS_BITS", "0" },
  { "loopSoundFade", 208, 2, -16, 0u, "FADETIME_BITS", "0" },
  { "targetname", 206, 2, 16, 0u, "16", "0" },
  { "lerp.useCount", 116, 4, 6, 0u, "USE_COUNT_BITS", "0" },
  { "clientLinkInfo", 218, 4, 26, 0u, "CLIENTLINKINFO_BITS", "0" },
  { "partBits[0]", 156, 4, 32, 0u, "32", "0" },
  { "partBits[1]", 160, 4, 32, 0u, "32", "0" },
  { "partBits[2]", 164, 4, 32, 0u, "32", "0" },
  { "partBits[3]", 168, 4, 32, 0u, "32", "0" },
  { "partBits[4]", 172, 4, 32, 0u, "32", "0" },
  { "index", 194, 2, 10, 1u, "SUBMODEL_BITS", "NEVER_CHANGES" },
  { "un2.anim.legsAnim", 136, 4, 11, 1u, "ANIM_BITS", "NEVER_CHANGES" },
  { "un2.anim.torsoAnim", 140, 4, 11, 1u, "ANIM_BITS", "NEVER_CHANGES" },
  { "un2.anim.fTorsoPitch", 144, 4, 0, 1u, "MSG_FIELD_FLOAT", "NEVER_CHANGES" },
  { "un2.anim.fWaistPitch", 148, 4, 0, 1u, "MSG_FIELD_FLOAT", "NEVER_CHANGES" },
  { "iHeadIcon", 214, 1, 4, 1u, "HEAD_ICON_BITS", "NEVER_CHANGES" },
  { "faction", 215, 1, 7, 1u, "2 + CLIENT_BITS", "NEVER_CHANGES" },
  {
    "lerp.apos.trDuration",
    56,
    4,
    -69,
    1u,
    "MSG_FIELD_ANGLE_TRDURATION",
    "NEVER_CHANGES"
  },
  { "lerp.u.anonymous.data[3]", 96, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[4]", 100, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[5]", 104, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[6]", 108, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[7]", 112, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "enemyModel", 200, 2, 10, 1u, "SUBMODEL_BITS", "NEVER_CHANGES" }
};


const int numMissileEntityStateFields = 69;
const NetField missileEntityStateFields[69] =
{
  { "eType", 190, 2, -58, 0u, "MSG_FIELD_ETYPE", "0" },
  { "lerp.pos.trTime", 16, 4, -68, 0u, "MSG_FIELD_POS_TRTIME", "0" },
  { "lerp.pos.trDelta[1]", 40, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trDelta[0]", 36, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trBase[1]", 28, 4, -65, 0u, "MSG_FIELD_ES_ORIGINY", "0" },
  { "lerp.pos.trBase[0]", 24, 4, -66, 0u, "MSG_FIELD_ES_ORIGINX", "0" },
  { "lerp.pos.trDelta[2]", 44, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.apos.trDelta[0]", 72, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "lerp.apos.trBase[1]", 64, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.apos.trBase[2]", 68, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.apos.trBase[0]", 60, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.pos.trBase[2]", 32, 4, -64, 0u, "MSG_FIELD_ES_ORIGINZ", "0" },
  { "eventSequence", 210, 2, -59, 0u, "MSG_FIELD_EVENTSEQUENCE", "0" },
  { "groundEntityNum", 192, 2, -96, 0u, "MSG_FIELD_GROUNDENTITY", "0" },
  { "lerp.pos.trType", 12, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "surfType", 212, 1, 8, 0u, "8", "0" },
  { "lerp.apos.trTime", 52, 4, -70, 0u, "MSG_FIELD_ANGLE_TRTIME", "0" },
  { "eventParms[0]", 180, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "events[0]", 176, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "lerp.apos.trDelta[2]", 80, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "events[1]", 177, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventParms[1]", 182, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "events[2]", 178, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventParms[2]", 184, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "eventParms[3]", 186, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "events[3]", 179, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "lerp.eFlags", 4, 4, -98, 0u, "MSG_FIELD_EFLAGS", "0" },
  { "lerp.u.missile.launchTime", 84, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "lerp.u.missile.forcedDud", 96, 4, 1, 0u, "1", "0" },
  { "otherEntityNum", 196, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "lerp.apos.trType", 48, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "lerp.eFlags2", 8, 4, 32, 0u, "32", "0" },
  { "lerp.u.missile.fuseTime", 92, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "weapon", 202, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "index", 194, 2, 10, 0u, "SUBMODEL_BITS", "0" },
  { "clientNum", 213, 1, 6, 0u, "CLIENT_BITS+1", "0" },
  { "un3", 152, 4, 1, 0u, "1", "0" },
  { "weaponModel", 216, 1, 4, 0u, "MAX_WEAPONMODEL_BITS", "0" },
  { "renderOptions", 132, 4, 32, 0u, "MAX_RENDER_OPTIONS_BITS", "0" },
  { "eventParm", 188, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "un1", 217, 1, 8, 0u, "8", "0" },
  { "lerp.apos.trDelta[1]", 76, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "attackerEntityNum", 198, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "lerp.pos.trDuration", 20, 4, -67, 0u, "MSG_FIELD_POS_TRDURATION", "0" },
  { "loopSoundId", 124, 4, 32, 0u, "SOUNDALIAS_BITS", "0" },
  { "loopSoundFade", 208, 2, -16, 0u, "FADETIME_BITS", "0" },
  { "solid", 128, 4, 24, 0u, "24", "0" },
  { "time2", 120, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "targetname", 206, 2, 16, 0u, "16", "0" },
  { "lerp.u.missile.parentClientNum", 88, 4, 6, 0u, "CLIENT_BITS+1", "0" },
  { "lerp.useCount", 116, 4, 6, 0u, "USE_COUNT_BITS", "0" },
  { "faction", 215, 1, 7, 0u, "2 + CLIENT_BITS", "0" },
  { "clientLinkInfo", 218, 4, 26, 0u, "CLIENTLINKINFO_BITS", "0" },
  { "un2.anim.legsAnim", 136, 4, 11, 1u, "ANIM_BITS", "NEVER_CHANGES" },
  { "un2.anim.torsoAnim", 140, 4, 11, 1u, "ANIM_BITS", "NEVER_CHANGES" },
  { "un2.anim.fTorsoPitch", 144, 4, 0, 1u, "MSG_FIELD_FLOAT", "NEVER_CHANGES" },
  { "un2.anim.fWaistPitch", 148, 4, 0, 1u, "MSG_FIELD_FLOAT", "NEVER_CHANGES" },
  { "iHeadIcon", 214, 1, 4, 1u, "HEAD_ICON_BITS", "NEVER_CHANGES" },
  {
    "lerp.apos.trDuration",
    56,
    4,
    -69,
    1u,
    "MSG_FIELD_ANGLE_TRDURATION",
    "NEVER_CHANGES"
  },
  { "lerp.u.anonymous.data[4]", 100, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[5]", 104, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[6]", 108, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[7]", 112, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[0]", 156, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[1]", 160, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[2]", 164, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[3]", 168, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[4]", 172, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "enemyModel", 200, 2, 10, 1u, "SUBMODEL_BITS", "NEVER_CHANGES" }
};

const NetField scriptMoverStateFields[72] =
{
  { "eType", 190, 2, -58, 0u, "MSG_FIELD_ETYPE", "0" },
  { "loopSoundFade", 208, 2, -16, 0u, "FADETIME_BITS", "0" },
  { "lerp.apos.trTime", 52, 4, -70, 0u, "MSG_FIELD_ANGLE_TRTIME", "0" },
  { "lerp.apos.trType", 48, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "clientLinkInfo", 218, 4, 26, 0u, "CLIENTLINKINFO_BITS", "0" },
  { "lerp.pos.trType", 12, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "lerp.pos.trBase[0]", 24, 4, -66, 0u, "MSG_FIELD_ES_ORIGINX", "0" },
  { "lerp.pos.trBase[1]", 28, 4, -65, 0u, "MSG_FIELD_ES_ORIGINY", "0" },
  { "loopSoundId", 124, 4, 32, 0u, "SOUNDALIAS_BITS", "0" },
  { "lerp.apos.trBase[1]", 64, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "eventSequence", 210, 2, -59, 0u, "MSG_FIELD_EVENTSEQUENCE", "0" },
  { "eventParms[0]", 180, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "events[0]", 176, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "lerp.pos.trTime", 16, 4, -68, 0u, "MSG_FIELD_POS_TRTIME", "0" },
  { "lerp.eFlags", 4, 4, -98, 0u, "MSG_FIELD_EFLAGS", "0" },
  { "lerp.apos.trDelta[0]", 72, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "lerp.pos.trDelta[2]", 44, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trDelta[0]", 36, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trDelta[1]", 40, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.apos.trDelta[1]", 76, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "lerp.apos.trDelta[2]", 80, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "lerp.apos.trBase[2]", 68, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.apos.trBase[0]", 60, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.pos.trBase[2]", 32, 4, -64, 0u, "MSG_FIELD_ES_ORIGINZ", "0" },
  { "lerp.pos.trDuration", 20, 4, -67, 0u, "MSG_FIELD_POS_TRDURATION", "0" },
  { "partBits[2]", 164, 4, 32, 0u, "32", "0" },
  { "lerp.apos.trDuration", 56, 4, -69, 0u, "MSG_FIELD_ANGLE_TRDURATION", "0" },
  { "partBits[0]", 156, 4, 32, 0u, "32", "0" },
  { "partBits[1]", 160, 4, 32, 0u, "32", "0" },
  { "index", 194, 2, 10, 0u, "SUBMODEL_BITS", "0" },
  { "events[1]", 177, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventParms[1]", 182, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "eventParms[2]", 184, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "events[2]", 178, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventParms[3]", 186, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "events[3]", 179, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "solid", 128, 4, 24, 0u, "24", "0" },
  { "lerp.eFlags2", 8, 4, 32, 0u, "32", "0" },
  { "groundEntityNum", 192, 2, -96, 0u, "MSG_FIELD_GROUNDENTITY", "0" },
  { "clientNum", 213, 1, 6, 0u, "CLIENT_BITS+1", "0" },
  { "eventParm", 188, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "weapon", 202, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "surfType", 212, 1, 8, 0u, "8", "0" },
  { "otherEntityNum", 196, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "targetname", 206, 2, 16, 0u, "16", "0" },
  { "weaponModel", 216, 1, 4, 0u, "MAX_WEAPONMODEL_BITS", "0" },
  { "renderOptions", 132, 4, 32, 0u, "MAX_RENDER_OPTIONS_BITS", "0" },
  { "time2", 120, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "un1", 217, 1, 8, 0u, "8", "0" },
  { "attackerEntityNum", 198, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "un2.anim.fWaistPitch", 148, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "un2.anim.fTorsoPitch", 144, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "faction", 215, 1, 7, 0u, "2 + CLIENT_BITS", "0" },
  { "un2.anim.torsoAnim", 140, 4, 11, 0u, "ANIM_BITS", "0" },
  { "un2.anim.legsAnim", 136, 4, 11, 0u, "ANIM_BITS", "0" },
  { "lerp.u.scriptMover.exploderIndex", 104, 2, 12, 0u, "12", "0" },
  { "lerp.useCount", 116, 4, 6, 0u, "USE_COUNT_BITS", "0" },
  { "enemyModel", 200, 2, 10, 0u, "SUBMODEL_BITS", "0" },
  { "lerp.u.scriptMover.attachModelIndex[0]", 92, 2, 9, 0u, "MODEL_BITS", "0" },
  { "lerp.u.scriptMover.attachModelIndex[1]", 94, 2, 9, 0u, "MODEL_BITS", "0" },
  { "lerp.u.scriptMover.attachModelIndex[2]", 96, 2, 9, 0u, "MODEL_BITS", "0" },
  { "lerp.u.scriptMover.attachModelIndex[3]", 98, 2, 9, 0u, "MODEL_BITS", "0" },
  { "lerp.u.scriptMover.animScriptedAnim", 100, 2, 11, 0u, "ANIM_BITS", "0" },
  {
    "lerp.u.scriptMover.attachedEntNum",
    102,
    2,
    10,
    0u,
    "GENTITYNUM_BITS",
    "0"
  },
  { "lerp.u.scriptMover.attachTagIndex[0]", 84, 1, 5, 0u, "TAG_BITS", "0" },
  { "lerp.u.scriptMover.attachTagIndex[1]", 85, 1, 5, 0u, "TAG_BITS", "0" },
  { "lerp.u.scriptMover.attachTagIndex[2]", 86, 1, 5, 0u, "TAG_BITS", "0" },
  { "lerp.u.scriptMover.attachTagIndex[3]", 87, 1, 5, 0u, "TAG_BITS", "0" },
  { "lerp.u.scriptMover.attachedTagIndex", 88, 4, 5, 0u, "TAG_BITS", "0" },
  { "iHeadIcon", 214, 1, 4, 1u, "HEAD_ICON_BITS", "NEVER_CHANGES" },
  { "partBits[3]", 168, 4, 32, 0u, "32", "0" },
  { "partBits[4]", 172, 4, 32, 0u, "32", "0" }
};


const int numSoundBlendEntityStateFields = 68;
const NetField soundBlendEntityStateFields[68] =
{
  { "eType", 190, 2, -58, 1u, "MSG_FIELD_ETYPE", "NEVER_CHANGES" },
  { "lerp.pos.trTime", 16, 4, -68, 0u, "MSG_FIELD_POS_TRTIME", "0" },
  { "lerp.pos.trBase[0]", 24, 4, -66, 0u, "MSG_FIELD_ES_ORIGINX", "0" },
  { "lerp.pos.trBase[1]", 28, 4, -65, 0u, "MSG_FIELD_ES_ORIGINY", "0" },
  { "lerp.pos.trDelta[0]", 36, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trDelta[1]", 40, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.apos.trBase[1]", 64, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.pos.trBase[2]", 32, 4, -64, 0u, "MSG_FIELD_ES_ORIGINZ", "0" },
  { "lerp.pos.trDelta[2]", 44, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.apos.trBase[0]", 60, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "eventSequence", 210, 2, -59, 0u, "MSG_FIELD_EVENTSEQUENCE", "0" },
  { "un2.anim.legsAnim", 136, 4, 11, 0u, "ANIM_BITS", "0" },
  { "surfType", 212, 1, 8, 0u, "8", "0" },
  { "otherEntityNum", 196, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "un1", 217, 1, 8, 0u, "8", "0" },
  { "lerp.eFlags", 4, 4, -98, 0u, "MSG_FIELD_EFLAGS", "0" },
  { "lerp.eFlags2", 8, 4, 32, 0u, "32", "0" },
  { "groundEntityNum", 192, 2, -96, 0u, "MSG_FIELD_GROUNDENTITY", "0" },
  { "clientNum", 213, 1, 6, 0u, "CLIENT_BITS+1", "0" },
  { "events[0]", 176, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "events[1]", 177, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "events[2]", 178, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "weapon", 202, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "weaponModel", 216, 1, 4, 0u, "MAX_WEAPONMODEL_BITS", "0" },
  { "renderOptions", 132, 4, 32, 0u, "MAX_RENDER_OPTIONS_BITS", "0" },
  { "eventParms[1]", 182, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "eventParms[0]", 180, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "eventParms[2]", 184, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "index", 194, 2, 10, 0u, "SUBMODEL_BITS", "0" },
  { "lerp.pos.trType", 12, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "lerp.apos.trType", 48, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "events[3]", 179, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "lerp.apos.trBase[2]", 68, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.apos.trTime", 52, 4, -70, 0u, "MSG_FIELD_ANGLE_TRTIME", "0" },
  { "lerp.apos.trDelta[0]", 72, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "lerp.apos.trDelta[2]", 80, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "un2.anim.torsoAnim", 140, 4, 11, 0u, "ANIM_BITS", "0" },
  { "eventParms[3]", 186, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "solid", 128, 4, 24, 0u, "24", "0" },
  { "lerp.pos.trDuration", 20, 4, -67, 0u, "MSG_FIELD_POS_TRDURATION", "0" },
  { "lerp.apos.trDelta[1]", 76, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "un3", 152, 4, 32, 0u, "32", "0" },
  { "time2", 120, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "loopSoundId", 124, 4, 32, 0u, "SOUNDALIAS_BITS", "0" },
  { "loopSoundFade", 208, 2, -16, 0u, "FADETIME_BITS", "0" },
  { "attackerEntityNum", 198, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "un2.anim.fWaistPitch", 148, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "un2.anim.fTorsoPitch", 144, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "eventParm", 188, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.apos.trDuration", 56, 4, -69, 0u, "MSG_FIELD_ANGLE_TRDURATION", "0" },
  { "targetname", 206, 2, 16, 0u, "16", "0" },
  { "lerp.useCount", 116, 4, 6, 0u, "USE_COUNT_BITS", "0" },
  { "clientLinkInfo", 218, 4, 26, 0u, "CLIENTLINKINFO_BITS", "0" },
  { "lerp.u.anonymous.data[1]", 88, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[2]", 92, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[3]", 96, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[4]", 100, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[5]", 104, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[6]", 108, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[7]", 112, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[0]", 156, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[1]", 160, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[2]", 164, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[3]", 168, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[4]", 172, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "enemyModel", 200, 2, 10, 1u, "SUBMODEL_BITS", "NEVER_CHANGES" },
  { "iHeadIcon", 214, 1, 4, 1u, "HEAD_ICON_BITS", "NEVER_CHANGES" },
  { "faction", 215, 1, 7, 1u, "2 + CLIENT_BITS", "NEVER_CHANGES" }
};

const NetField fxStateFields[68] =
{
  { "eType", 190, 2, -58, 0u, "MSG_FIELD_ETYPE", "0" },
  { "time2", 120, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "lerp.apos.trBase[0]", 60, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.pos.trBase[0]", 24, 4, -66, 0u, "MSG_FIELD_ES_ORIGINX", "0" },
  { "lerp.pos.trBase[1]", 28, 4, -65, 0u, "MSG_FIELD_ES_ORIGINY", "0" },
  { "un1", 217, 1, 8, 0u, "8", "0" },
  { "lerp.pos.trBase[2]", 32, 4, -64, 0u, "MSG_FIELD_ES_ORIGINZ", "0" },
  { "lerp.apos.trBase[2]", 68, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.apos.trBase[1]", 64, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "targetname", 206, 2, 16, 0u, "16", "0" },
  { "lerp.pos.trTime", 16, 4, -68, 0u, "MSG_FIELD_POS_TRTIME", "0" },
  { "lerp.pos.trType", 12, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "lerp.pos.trDelta[2]", 44, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trDelta[0]", 36, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trDelta[1]", 40, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "clientNum", 213, 1, 6, 0u, "CLIENT_BITS+1", "0" },
  { "lerp.apos.trType", 48, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "lerp.apos.trTime", 52, 4, -70, 0u, "MSG_FIELD_ANGLE_TRTIME", "0" },
  { "lerp.apos.trDelta[0]", 72, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "lerp.apos.trDelta[1]", 76, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "lerp.apos.trDelta[2]", 80, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "index", 194, 2, 10, 0u, "SUBMODEL_BITS", "0" },
  { "groundEntityNum", 192, 2, -96, 0u, "MSG_FIELD_GROUNDENTITY", "0" },
  { "lerp.pos.trDuration", 20, 4, -67, 0u, "MSG_FIELD_POS_TRDURATION", "0" },
  { "solid", 128, 4, 24, 0u, "24", "0" },
  { "eventParm", 188, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.eFlags", 4, 4, -98, 0u, "MSG_FIELD_EFLAGS", "0" },
  { "lerp.eFlags2", 8, 4, 32, 0u, "32", "0" },
  { "eventSequence", 210, 2, -59, 0u, "MSG_FIELD_EVENTSEQUENCE", "0" },
  { "events[0]", 176, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventParms[0]", 180, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "weapon", 202, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "weaponModel", 216, 1, 4, 0u, "MAX_WEAPONMODEL_BITS", "0" },
  { "renderOptions", 132, 4, 32, 0u, "MAX_RENDER_OPTIONS_BITS", "0" },
  { "surfType", 212, 1, 8, 0u, "8", "0" },
  { "otherEntityNum", 196, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "lerp.u.anonymous.data[0]", 84, 4, 32, 0u, "32", "0" },
  { "lerp.u.anonymous.data[1]", 88, 4, 32, 0u, "32", "0" },
  { "lerp.u.anonymous.data[2]", 92, 4, 32, 0u, "32", "0" },
  { "eventParms[1]", 182, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "eventParms[2]", 184, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "eventParms[3]", 186, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "events[1]", 177, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "events[2]", 178, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "events[3]", 179, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "attackerEntityNum", 198, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "loopSoundId", 124, 4, 32, 0u, "SOUNDALIAS_BITS", "0" },
  { "loopSoundFade", 208, 2, -16, 0u, "FADETIME_BITS", "0" },
  { "lerp.useCount", 116, 4, 6, 0u, "USE_COUNT_BITS", "0" },
  { "clientLinkInfo", 218, 4, 26, 0u, "CLIENTLINKINFO_BITS", "0" },
  { "un2.anim.legsAnim", 136, 4, 11, 1u, "ANIM_BITS", "NEVER_CHANGES" },
  { "un2.anim.torsoAnim", 140, 4, 11, 1u, "ANIM_BITS", "NEVER_CHANGES" },
  { "un2.anim.fTorsoPitch", 144, 4, 0, 1u, "MSG_FIELD_FLOAT", "NEVER_CHANGES" },
  { "un2.anim.fWaistPitch", 148, 4, 0, 1u, "MSG_FIELD_FLOAT", "NEVER_CHANGES" },
  { "iHeadIcon", 214, 1, 4, 1u, "HEAD_ICON_BITS", "NEVER_CHANGES" },
  { "faction", 215, 1, 7, 1u, "2 + CLIENT_BITS", "NEVER_CHANGES" },
  {
    "lerp.apos.trDuration",
    56,
    4,
    -69,
    1u,
    "MSG_FIELD_ANGLE_TRDURATION",
    "NEVER_CHANGES"
  },
  { "lerp.u.anonymous.data[3]", 96, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[4]", 100, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[5]", 104, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[6]", 108, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[7]", 112, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[0]", 156, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[1]", 160, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[2]", 164, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[3]", 168, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[4]", 172, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "enemyModel", 200, 2, 10, 1u, "SUBMODEL_BITS", "NEVER_CHANGES" }
};



const int numLoopFxEntityStateFields = 69;
const NetField loopFxEntityStateFields[69] =
{
  { "eType", 190, 2, -58, 1u, "MSG_FIELD_ETYPE", "NEVER_CHANGES" },
  { "lerp.pos.trTime", 16, 4, -68, 0u, "MSG_FIELD_POS_TRTIME", "0" },
  { "lerp.pos.trBase[0]", 24, 4, -66, 0u, "MSG_FIELD_ES_ORIGINX", "0" },
  { "lerp.pos.trBase[1]", 28, 4, -65, 0u, "MSG_FIELD_ES_ORIGINY", "0" },
  { "lerp.pos.trDelta[0]", 36, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trDelta[1]", 40, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.apos.trBase[1]", 64, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.pos.trBase[2]", 32, 4, -64, 0u, "MSG_FIELD_ES_ORIGINZ", "0" },
  { "lerp.pos.trDelta[2]", 44, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.apos.trBase[0]", 60, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.u.loopFx.cullDist", 84, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "lerp.u.loopFx.period", 88, 4, 32, 0u, "32", "0" },
  { "eventSequence", 210, 2, -59, 0u, "MSG_FIELD_EVENTSEQUENCE", "0" },
  { "un2.anim.legsAnim", 136, 4, 11, 0u, "ANIM_BITS", "0" },
  { "surfType", 212, 1, 8, 0u, "8", "0" },
  { "otherEntityNum", 196, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "un1", 217, 1, 8, 0u, "8", "0" },
  { "lerp.eFlags", 4, 4, -98, 0u, "MSG_FIELD_EFLAGS", "0" },
  { "lerp.eFlags2", 8, 4, 32, 0u, "32", "0" },
  { "groundEntityNum", 192, 2, -96, 0u, "MSG_FIELD_GROUNDENTITY", "0" },
  { "clientNum", 213, 1, 6, 0u, "CLIENT_BITS+1", "0" },
  { "events[0]", 176, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "events[1]", 177, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "events[2]", 178, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "weapon", 202, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "weaponModel", 216, 1, 4, 0u, "MAX_WEAPONMODEL_BITS", "0" },
  { "renderOptions", 132, 4, 32, 0u, "MAX_RENDER_OPTIONS_BITS", "0" },
  { "eventParms[1]", 182, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "eventParms[0]", 180, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "eventParms[2]", 184, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "index", 194, 2, 10, 0u, "SUBMODEL_BITS", "0" },
  { "lerp.pos.trType", 12, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "lerp.apos.trType", 48, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "events[3]", 179, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "lerp.apos.trBase[2]", 68, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.apos.trTime", 52, 4, -70, 0u, "MSG_FIELD_ANGLE_TRTIME", "0" },
  { "lerp.apos.trDelta[0]", 72, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "lerp.apos.trDelta[2]", 80, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "un2.anim.torsoAnim", 140, 4, 11, 0u, "ANIM_BITS", "0" },
  { "eventParms[3]", 186, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "solid", 128, 4, 24, 0u, "24", "0" },
  { "lerp.pos.trDuration", 20, 4, -67, 0u, "MSG_FIELD_POS_TRDURATION", "0" },
  { "lerp.apos.trDelta[1]", 76, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "un3", 152, 4, 32, 0u, "32", "0" },
  { "time2", 120, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "loopSoundId", 124, 4, 32, 0u, "SOUNDALIAS_BITS", "0" },
  { "loopSoundFade", 208, 2, -16, 0u, "FADETIME_BITS", "0" },
  { "attackerEntityNum", 198, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "un2.anim.fWaistPitch", 148, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "un2.anim.fTorsoPitch", 144, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "eventParm", 188, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.apos.trDuration", 56, 4, -69, 0u, "MSG_FIELD_ANGLE_TRDURATION", "0" },
  { "targetname", 206, 2, 16, 0u, "16", "0" },
  { "lerp.useCount", 116, 4, 6, 0u, "USE_COUNT_BITS", "0" },
  { "clientLinkInfo", 218, 4, 26, 0u, "CLIENTLINKINFO_BITS", "0" },
  { "lerp.u.anonymous.data[2]", 92, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[3]", 96, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[4]", 100, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[5]", 104, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[6]", 108, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[7]", 112, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[0]", 156, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[1]", 160, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[2]", 164, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[3]", 168, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[4]", 172, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "enemyModel", 200, 2, 10, 1u, "SUBMODEL_BITS", "NEVER_CHANGES" },
  { "iHeadIcon", 214, 1, 4, 1u, "HEAD_ICON_BITS", "NEVER_CHANGES" },
  { "faction", 215, 1, 7, 1u, "2 + CLIENT_BITS", "NEVER_CHANGES" }
};


const int numTurretEntityStateFields = 69;
const NetField turretEntityStateFields[69] =
{
  { "eType", 190, 2, -58, 0u, "MSG_FIELD_ETYPE", "0" },
  {
    "lerp.u.turret.gunAngles[0]",
    84,
    4,
    -100,
    0u,
    "MSG_FIELD_ANGLE2SHORT",
    "0"
  },
  {
    "lerp.u.turret.gunAngles[1]",
    88,
    4,
    -100,
    0u,
    "MSG_FIELD_ANGLE2SHORT",
    "0"
  },
  {
    "lerp.u.turret.gunAngles[2]",
    92,
    4,
    -100,
    0u,
    "MSG_FIELD_ANGLE2SHORT",
    "0"
  },
  { "lerp.u.turret.ownerNum", 96, 4, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "lerp.u.turret.heatVal", 100, 4, -81, 0u, "MSG_FIELD_0TO1_P2", "0" },
  { "lerp.u.turret.overheating", 104, 4, 1, 0u, "1", "0" },
  { "lerp.u.turret.flags", 112, 4, 8, 0u, "MAX_TURRET_NETWORK_BITS", "0" },
  { "lerp.eFlags", 4, 4, -98, 0u, "MSG_FIELD_EFLAGS", "0" },
  { "lerp.eFlags2", 8, 4, 32, 0u, "32", "0" },
  { "lerp.u.turret.pivotOffset", 108, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "lerp.pos.trBase[0]", 24, 4, -66, 0u, "MSG_FIELD_ES_ORIGINX", "0" },
  { "lerp.pos.trBase[1]", 28, 4, -65, 0u, "MSG_FIELD_ES_ORIGINY", "0" },
  { "lerp.pos.trBase[2]", 32, 4, -64, 0u, "MSG_FIELD_ES_ORIGINZ", "0" },
  { "events[0]", 176, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventSequence", 210, 2, -59, 0u, "MSG_FIELD_EVENTSEQUENCE", "0" },
  { "weapon", 202, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "weaponModel", 216, 1, 4, 0u, "MAX_WEAPONMODEL_BITS", "0" },
  { "renderOptions", 132, 4, 32, 0u, "MAX_RENDER_OPTIONS_BITS", "0" },
  { "eventParms[0]", 180, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "surfType", 212, 1, 8, 0u, "8", "0" },
  { "targetname", 206, 2, 16, 0u, "16", "0" },
  { "time2", 120, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "index", 194, 2, 10, 0u, "SUBMODEL_BITS", "0" },
  { "solid", 128, 4, 24, 0u, "24", "0" },
  { "un3", 152, 4, 32, 0u, "32", "0" },
  { "groundEntityNum", 192, 2, -96, 0u, "MSG_FIELD_GROUNDENTITY", "0" },
  { "un1", 217, 1, 8, 0u, "8", "0" },
  { "lerp.apos.trBase[1]", 64, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.apos.trBase[0]", 60, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "clientNum", 213, 1, 6, 0u, "CLIENT_BITS+1", "0" },
  { "lerp.pos.trDelta[0]", 36, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trDelta[1]", 40, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trDelta[2]", 44, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "events[1]", 177, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "events[2]", 178, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventParms[1]", 182, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "eventParms[2]", 184, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.pos.trTime", 16, 4, -68, 0u, "MSG_FIELD_POS_TRTIME", "0" },
  { "lerp.pos.trType", 12, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "eventParm", 188, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.apos.trType", 48, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "events[3]", 179, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "lerp.apos.trBase[2]", 68, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.apos.trTime", 52, 4, -70, 0u, "MSG_FIELD_ANGLE_TRTIME", "0" },
  { "lerp.apos.trDelta[0]", 72, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "lerp.apos.trDelta[2]", 80, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "eventParms[3]", 186, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.pos.trDuration", 20, 4, -67, 0u, "MSG_FIELD_POS_TRDURATION", "0" },
  { "lerp.apos.trDelta[1]", 76, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "attackerEntityNum", 198, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "un2.anim.fWaistPitch", 148, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "un2.anim.fTorsoPitch", 144, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "lerp.apos.trDuration", 56, 4, -69, 0u, "MSG_FIELD_ANGLE_TRDURATION", "0" },
  { "un2.anim.torsoAnim", 140, 4, 11, 0u, "ANIM_BITS", "0" },
  { "un2.anim.legsAnim", 136, 4, 11, 0u, "ANIM_BITS", "0" },
  { "loopSoundId", 124, 4, 32, 0u, "SOUNDALIAS_BITS", "0" },
  { "loopSoundFade", 208, 2, -16, 0u, "FADETIME_BITS", "0" },
  { "otherEntityNum", 196, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "lerp.useCount", 116, 4, 6, 0u, "USE_COUNT_BITS", "0" },
  { "partBits[0]", 156, 4, 32, 0u, "32", "0" },
  { "partBits[1]", 160, 4, 32, 0u, "32", "0" },
  { "partBits[2]", 164, 4, 32, 0u, "32", "0" },
  { "partBits[3]", 168, 4, 32, 0u, "32", "0" },
  { "partBits[4]", 172, 4, 32, 0u, "32", "0" },
  { "clientLinkInfo", 218, 4, 26, 0u, "CLIENTLINKINFO_BITS", "0" },
  { "enemyModel", 200, 2, 10, 1u, "SUBMODEL_BITS", "NEVER_CHANGES" },
  { "iHeadIcon", 214, 1, 4, 1u, "HEAD_ICON_BITS", "NEVER_CHANGES" },
  { "faction", 215, 1, 7, 1u, "2 + CLIENT_BITS", "NEVER_CHANGES" }
};

const NetField helicopterEntityStateFields[71] =
{
  { "eType", 190, 2, -58, 1u, "MSG_FIELD_ETYPE", "NEVER_CHANGES" },
  {
    "faction.teamAndOwnerIndex",
    215,
    1,
    7,
    2u,
    "2 + CLIENT_BITS",
    "ALWAYS_CHANGES"
  },
  { "lerp.pos.trTime", 16, 4, -68, 0u, "MSG_FIELD_POS_TRTIME", "0" },
  { "lerp.apos.trBase[0]", 60, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.apos.trBase[2]", 68, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  {
    "lerp.u.vehicle.gunnerAngles[1].pitch",
    96,
    2,
    -75,
    0u,
    "MSG_FIELD_PITCH_P1",
    "0"
  },
  { "lerp.u.vehicle.gunPitch", 110, 2, -75, 0u, "MSG_FIELD_PITCH_P1", "0" },
  { "lerp.u.vehicle.gunYaw", 112, 2, -74, 0u, "MSG_FIELD_YAW_P1", "0" },
  { "lerp.apos.trBase[1]", 64, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.u.vehicle.steerYaw", 84, 4, -77, 0u, "MSG_FIELD_NEG1TO1_P2", "0" },
  { "lerp.pos.trBase[1]", 28, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "lerp.pos.trBase[0]", 24, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "eventSequence", 210, 2, -59, 0u, "MSG_FIELD_EVENTSEQUENCE", "0" },
  { "lerp.u.vehicle.throttle", 108, 2, -16, 0u, "-16", "0" },
  { "lerp.pos.trBase[2]", 32, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "time2", 120, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "lerp.u.vehicle.bodyRoll", 88, 4, -100, 0u, "MSG_FIELD_ANGLE2SHORT", "0" },
  {
    "lerp.u.vehicle.gunnerAngles[1].yaw",
    98,
    2,
    -74,
    0u,
    "MSG_FIELD_YAW_P1",
    "0"
  },
  { "events[3]", 179, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "events[1]", 177, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventParms[0]", 180, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "events[0]", 176, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventParms[1]", 182, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "events[2]", 178, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventParms[2]", 184, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "eventParms[3]", 186, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.pos.trDuration", 20, 4, -67, 0u, "MSG_FIELD_POS_TRDURATION", "0" },
  { "lerp.eFlags", 4, 4, -98, 0u, "MSG_FIELD_EFLAGS", "0" },
  { "lerp.pos.trType", 12, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "lerp.apos.trType", 48, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "weapon", 202, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "index", 194, 2, 10, 0u, "SUBMODEL_BITS", "0" },
  { "solid", 128, 4, 24, 0u, "24", "0" },
  { "un3", 152, 4, 10, 0u, "SUBMODEL_BITS", "0" },
  { "lerp.useCount", 116, 4, 6, 0u, "USE_COUNT_BITS", "0" },
  { "un1", 217, 1, 8, 0u, "8", "0" },
  {
    "un2.vehicleState.vehicleInfoIndex",
    136,
    1,
    5,
    0u,
    "MAX_VEHICLE_FILES_BITS",
    "0"
  },
  {
    "lerp.u.vehicle.gunnerAngles[0].pitch",
    92,
    2,
    -75,
    0u,
    "MSG_FIELD_PITCH_P1",
    "0"
  },
  {
    "lerp.u.vehicle.gunnerAngles[0].yaw",
    94,
    2,
    -74,
    0u,
    "MSG_FIELD_YAW_P1",
    "0"
  },
  { "otherEntityNum", 196, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "groundEntityNum", 192, 2, -96, 0u, "MSG_FIELD_GROUNDENTITY", "0" },
  { "clientNum", 213, 1, 6, 0u, "CLIENT_BITS+1", "0" },
  { "weaponModel", 216, 1, 4, 0u, "MAX_WEAPONMODEL_BITS", "0" },
  { "renderOptions", 132, 4, 32, 0u, "MAX_RENDER_OPTIONS_BITS", "0" },
  { "lerp.eFlags2", 8, 4, 32, 0u, "32", "0" },
  { "lerp.apos.trTime", 52, 4, -70, 0u, "MSG_FIELD_ANGLE_TRTIME", "0" },
  { "un2.anim.torsoAnim", 140, 4, 11, 0u, "ANIM_BITS", "0" },
  { "loopSoundId", 124, 4, 32, 0u, "SOUNDALIAS_BITS", "0" },
  { "loopSoundFade", 208, 2, -16, 0u, "FADETIME_BITS", "0" },
  { "attackerEntityNum", 198, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "iHeadIcon", 214, 1, 4, 0u, "HEAD_ICON_BITS", "0" },
  { "eventParm", 188, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.apos.trDuration", 56, 4, -69, 0u, "MSG_FIELD_ANGLE_TRDURATION", "0" },
  { "targetname", 206, 2, 16, 0u, "16", "0" },
  {
    "lerp.u.vehicle.gunnerAngles[2].pitch",
    100,
    2,
    -75,
    0u,
    "MSG_FIELD_PITCH_P1",
    "0"
  },
  {
    "lerp.u.vehicle.gunnerAngles[3].pitch",
    104,
    2,
    -75,
    0u,
    "MSG_FIELD_PITCH_P1",
    "0"
  },
  {
    "lerp.u.vehicle.gunnerAngles[2].yaw",
    102,
    2,
    -74,
    0u,
    "MSG_FIELD_YAW_P1",
    "0"
  },
  {
    "lerp.u.vehicle.gunnerAngles[3].yaw",
    106,
    2,
    -74,
    0u,
    "MSG_FIELD_YAW_P1",
    "0"
  },
  { "lerp.pos.trDelta[1]", 40, 4, -72, 0u, "MSG_FIELD_VEHICLE_TVEL", "0" },
  { "lerp.pos.trDelta[0]", 36, 4, -72, 0u, "MSG_FIELD_VEHICLE_TVEL", "0" },
  { "lerp.pos.trDelta[2]", 44, 4, -72, 0u, "MSG_FIELD_VEHICLE_TVEL", "0" },
  { "enemyModel", 200, 2, 10, 0u, "SUBMODEL_BITS", "0" },
  { "clientLinkInfo", 218, 4, 26, 0u, "CLIENTLINKINFO_BITS", "0" },
  {
    "lerp.apos.trDelta[1]",
    76,
    4,
    -73,
    1u,
    "MSG_FIELD_VEHICLE_AVEL",
    "NEVER_CHANGES"
  },
  {
    "lerp.apos.trDelta[0]",
    72,
    4,
    -73,
    1u,
    "MSG_FIELD_VEHICLE_AVEL",
    "NEVER_CHANGES"
  },
  {
    "lerp.apos.trDelta[2]",
    80,
    4,
    -73,
    1u,
    "MSG_FIELD_VEHICLE_AVEL",
    "NEVER_CHANGES"
  },
  { "partBits[0]", 156, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[1]", 160, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[2]", 164, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[3]", 168, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[4]", 172, 4, 32, 1u, "32", "NEVER_CHANGES" }
};

const NetField planeStateFields[66] =
{
  { "eType", 190, 2, -58, 1u, "MSG_FIELD_ETYPE", "NEVER_CHANGES" },
  { "lerp.pos.trBase[0]", 24, 4, 0, 2u, "MSG_FIELD_FLOAT", "ALWAYS_CHANGES" },
  { "lerp.pos.trBase[1]", 28, 4, 0, 2u, "MSG_FIELD_FLOAT", "ALWAYS_CHANGES" },
  { "lerp.pos.trBase[2]", 32, 4, 0, 2u, "MSG_FIELD_FLOAT", "ALWAYS_CHANGES" },
  { "index", 194, 2, 10, 2u, "SUBMODEL_BITS", "ALWAYS_CHANGES" },
  {
    "lerp.pos.trDelta[0]",
    36,
    4,
    -63,
    2u,
    "MSG_FIELD_ES_ORIGIN_DELTA",
    "ALWAYS_CHANGES"
  },
  {
    "lerp.pos.trDelta[1]",
    40,
    4,
    -63,
    2u,
    "MSG_FIELD_ES_ORIGIN_DELTA",
    "ALWAYS_CHANGES"
  },
  {
    "lerp.pos.trTime",
    16,
    4,
    -68,
    2u,
    "MSG_FIELD_POS_TRTIME",
    "ALWAYS_CHANGES"
  },
  { "lerp.pos.trType", 12, 1, 8, 2u, "MSG_TRAJECTORY_BITS", "ALWAYS_CHANGES" },
  {
    "lerp.pos.trDuration",
    20,
    4,
    -67,
    2u,
    "MSG_FIELD_POS_TRDURATION",
    "ALWAYS_CHANGES"
  },
  {
    "faction.teamAndOwnerIndex",
    215,
    1,
    7,
    2u,
    "2 + CLIENT_BITS",
    "ALWAYS_CHANGES"
  },
  {
    "lerp.apos.trBase[1]",
    64,
    4,
    -62,
    2u,
    "MSG_FIELD_ES_ANGLE",
    "ALWAYS_CHANGES"
  },
  { "lerp.pos.trDelta[2]", 44, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "events[0]", 176, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventSequence", 210, 2, -59, 0u, "MSG_FIELD_EVENTSEQUENCE", "0" },
  { "eventParms[0]", 180, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "events[1]", 177, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventParms[1]", 182, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "loopSoundId", 124, 4, 32, 0u, "SOUNDALIAS_BITS", "0" },
  { "loopSoundFade", 208, 2, -16, 0u, "FADETIME_BITS", "0" },
  { "lerp.apos.trType", 48, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "eventParm", 188, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "targetname", 206, 2, 16, 0u, "16", "0" },
  { "weapon", 202, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "weaponModel", 216, 1, 4, 0u, "MAX_WEAPONMODEL_BITS", "0" },
  { "renderOptions", 132, 4, 32, 0u, "MAX_RENDER_OPTIONS_BITS", "0" },
  { "surfType", 212, 1, 8, 0u, "8", "0" },
  { "lerp.u.anonymous.data[0]", 84, 4, 32, 0u, "32", "0" },
  { "time2", 120, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "solid", 128, 4, 24, 0u, "24", "0" },
  { "un3", 152, 4, 32, 0u, "32", "0" },
  { "groundEntityNum", 192, 2, -96, 0u, "MSG_FIELD_GROUNDENTITY", "0" },
  { "un1", 217, 1, 8, 0u, "8", "0" },
  { "lerp.apos.trBase[0]", 60, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "clientNum", 213, 1, 6, 0u, "CLIENT_BITS+1", "0" },
  { "events[2]", 178, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventParms[2]", 184, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "events[3]", 179, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "lerp.apos.trBase[2]", 68, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.apos.trTime", 52, 4, -70, 0u, "MSG_FIELD_ANGLE_TRTIME", "0" },
  { "lerp.apos.trDelta[0]", 72, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "lerp.apos.trDelta[2]", 80, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "eventParms[3]", 186, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.apos.trDelta[1]", 76, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "attackerEntityNum", 198, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "iHeadIcon", 214, 1, 4, 0u, "HEAD_ICON_BITS", "0" },
  { "lerp.apos.trDuration", 56, 4, -69, 0u, "MSG_FIELD_ANGLE_TRDURATION", "0" },
  { "lerp.eFlags", 4, 4, -98, 0u, "MSG_FIELD_EFLAGS", "0" },
  { "lerp.eFlags2", 8, 4, 32, 0u, "32", "0" },
  { "otherEntityNum", 196, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "lerp.useCount", 116, 4, 6, 0u, "USE_COUNT_BITS", "0" },
  { "lerp.u.anonymous.data[1]", 88, 4, 32, 0u, "32", "0" },
  { "lerp.u.anonymous.data[2]", 92, 4, 32, 0u, "32", "0" },
  { "clientLinkInfo", 218, 4, 26, 0u, "CLIENTLINKINFO_BITS", "0" },
  {
    "un2.vehicleState.vehicleInfoIndex",
    136,
    1,
    5,
    0u,
    "MAX_VEHICLE_FILES_BITS",
    "0"
  },
  { "lerp.u.anonymous.data[3]", 96, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[4]", 100, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[5]", 104, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[6]", 108, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[7]", 112, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[0]", 156, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[1]", 160, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[2]", 164, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[3]", 168, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[4]", 172, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "enemyModel", 200, 2, 10, 1u, "SUBMODEL_BITS", "NEVER_CHANGES" }
};



const int numVehicleEntityStateFields = 71;
const NetField vehicleEntityStateFields[71] =
{
  { "eType", 190, 2, -58, 1u, "MSG_FIELD_ETYPE", "NEVER_CHANGES" },
  { "lerp.pos.trTime", 16, 4, -68, 0u, "MSG_FIELD_POS_TRTIME", "0" },
  { "lerp.apos.trBase[0]", 60, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.apos.trBase[2]", 68, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  {
    "lerp.u.vehicle.gunnerAngles[1].pitch",
    96,
    2,
    -75,
    0u,
    "MSG_FIELD_PITCH_P1",
    "0"
  },
  { "lerp.u.vehicle.gunPitch", 110, 2, -75, 0u, "MSG_FIELD_PITCH_P1", "0" },
  { "lerp.u.vehicle.gunYaw", 112, 2, -74, 0u, "MSG_FIELD_YAW_P1", "0" },
  { "lerp.apos.trBase[1]", 64, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.u.vehicle.steerYaw", 84, 4, -77, 0u, "MSG_FIELD_NEG1TO1_P2", "0" },
  { "lerp.pos.trBase[1]", 28, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "lerp.pos.trBase[0]", 24, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "eventSequence", 210, 2, -59, 0u, "MSG_FIELD_EVENTSEQUENCE", "0" },
  { "lerp.u.vehicle.throttle", 108, 2, -16, 0u, "-16", "0" },
  { "lerp.pos.trBase[2]", 32, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "time2", 120, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "lerp.u.vehicle.bodyRoll", 88, 4, -100, 0u, "MSG_FIELD_ANGLE2SHORT", "0" },
  {
    "lerp.u.vehicle.gunnerAngles[1].yaw",
    98,
    2,
    -74,
    0u,
    "MSG_FIELD_YAW_P1",
    "0"
  },
  { "events[3]", 179, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "events[1]", 177, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventParms[0]", 180, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "events[0]", 176, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventParms[1]", 182, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "events[2]", 178, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventParms[2]", 184, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "eventParms[3]", 186, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.pos.trDuration", 20, 4, -67, 0u, "MSG_FIELD_POS_TRDURATION", "0" },
  { "lerp.eFlags", 4, 4, -98, 0u, "MSG_FIELD_EFLAGS", "0" },
  { "lerp.eFlags2", 8, 4, 32, 0u, "32", "0" },
  { "lerp.pos.trType", 12, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "lerp.apos.trType", 48, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "weapon", 202, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "index", 194, 2, 10, 0u, "SUBMODEL_BITS", "0" },
  { "solid", 128, 4, 24, 0u, "24", "0" },
  { "un3", 152, 4, 10, 0u, "SUBMODEL_BITS", "0" },
  { "lerp.useCount", 116, 4, 6, 0u, "USE_COUNT_BITS", "0" },
  { "un1", 217, 1, 8, 0u, "8", "0" },
  {
    "un2.vehicleState.vehicleInfoIndex",
    136,
    1,
    5,
    0u,
    "MAX_VEHICLE_FILES_BITS",
    "0"
  },
  {
    "lerp.u.vehicle.gunnerAngles[0].pitch",
    92,
    2,
    -75,
    0u,
    "MSG_FIELD_PITCH_P1",
    "0"
  },
  {
    "lerp.u.vehicle.gunnerAngles[0].yaw",
    94,
    2,
    -74,
    0u,
    "MSG_FIELD_YAW_P1",
    "0"
  },
  { "otherEntityNum", 196, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "groundEntityNum", 192, 2, -96, 0u, "MSG_FIELD_GROUNDENTITY", "0" },
  { "clientNum", 213, 1, 6, 0u, "CLIENT_BITS+1", "0" },
  { "weaponModel", 216, 1, 4, 0u, "MAX_WEAPONMODEL_BITS", "0" },
  { "renderOptions", 132, 4, 32, 0u, "MAX_RENDER_OPTIONS_BITS", "0" },
  { "lerp.apos.trTime", 52, 4, -70, 0u, "MSG_FIELD_ANGLE_TRTIME", "0" },
  { "un2.anim.torsoAnim", 140, 4, 11, 0u, "ANIM_BITS", "0" },
  { "loopSoundId", 124, 4, 32, 0u, "SOUNDALIAS_BITS", "0" },
  { "loopSoundFade", 208, 2, -16, 0u, "FADETIME_BITS", "0" },
  { "attackerEntityNum", 198, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "iHeadIcon", 214, 1, 4, 0u, "HEAD_ICON_BITS", "0" },
  { "eventParm", 188, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.apos.trDuration", 56, 4, -69, 0u, "MSG_FIELD_ANGLE_TRDURATION", "0" },
  { "targetname", 206, 2, 16, 0u, "16", "0" },
  {
    "lerp.u.vehicle.gunnerAngles[2].pitch",
    100,
    2,
    -75,
    0u,
    "MSG_FIELD_PITCH_P1",
    "0"
  },
  {
    "lerp.u.vehicle.gunnerAngles[3].pitch",
    104,
    2,
    -75,
    0u,
    "MSG_FIELD_PITCH_P1",
    "0"
  },
  {
    "lerp.u.vehicle.gunnerAngles[2].yaw",
    102,
    2,
    -74,
    0u,
    "MSG_FIELD_YAW_P1",
    "0"
  },
  {
    "lerp.u.vehicle.gunnerAngles[3].yaw",
    106,
    2,
    -74,
    0u,
    "MSG_FIELD_YAW_P1",
    "0"
  },
  { "faction.teamAndOwnerIndex", 215, 1, 7, 0u, "2 + CLIENT_BITS", "0" },
  { "lerp.pos.trDelta[1]", 40, 4, -72, 0u, "MSG_FIELD_VEHICLE_TVEL", "0" },
  { "lerp.pos.trDelta[0]", 36, 4, -72, 0u, "MSG_FIELD_VEHICLE_TVEL", "0" },
  { "lerp.pos.trDelta[2]", 44, 4, -72, 0u, "MSG_FIELD_VEHICLE_TVEL", "0" },
  { "lerp.apos.trDelta[1]", 76, 4, -73, 0u, "MSG_FIELD_VEHICLE_AVEL", "0" },
  { "lerp.apos.trDelta[0]", 72, 4, -73, 0u, "MSG_FIELD_VEHICLE_AVEL", "0" },
  { "lerp.apos.trDelta[2]", 80, 4, -73, 0u, "MSG_FIELD_VEHICLE_AVEL", "0" },
  { "enemyModel", 200, 2, 10, 0u, "SUBMODEL_BITS", "0" },
  { "clientLinkInfo", 218, 4, 26, 0u, "CLIENTLINKINFO_BITS", "0" },
  { "partBits[0]", 156, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[1]", 160, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[2]", 164, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[3]", 168, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[4]", 172, 4, 32, 1u, "32", "NEVER_CHANGES" }
};


const int numActorStateFields = 69;
const NetField actorStateFields[69] =
{
  { "eType", 190, 2, -58, 0u, "MSG_FIELD_ETYPE", "0" },
  {
    "lerp.pos.trBase[1]",
    28,
    4,
    -65,
    2u,
    "MSG_FIELD_ES_ORIGINY",
    "ALWAYS_CHANGES"
  },
  {
    "lerp.pos.trBase[0]",
    24,
    4,
    -66,
    2u,
    "MSG_FIELD_ES_ORIGINX",
    "ALWAYS_CHANGES"
  },
  {
    "lerp.apos.trBase[1]",
    64,
    4,
    -62,
    2u,
    "MSG_FIELD_ES_ANGLE",
    "ALWAYS_CHANGES"
  },
  { "lerp.pos.trBase[2]", 32, 4, -64, 0u, "MSG_FIELD_ES_ORIGINZ", "0" },
  { "un2.animState.fLeanAmount", 140, 4, -77, 0u, "MSG_FIELD_NEG1TO1_P2", "0" },
  { "un2.animState.state", 136, 4, 7, 0u, "ACTOR_ANIM_STATE_BITS", "0" },
  { "groundEntityNum", 192, 2, -96, 0u, "MSG_FIELD_GROUNDENTITY", "0" },
  { "lerp.eFlags", 4, 4, -98, 0u, "MSG_FIELD_EFLAGS", "0" },
  { "lerp.eFlags2", 8, 4, 32, 0u, "32", "0" },
  { "events[0]", 176, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventSequence", 210, 2, -59, 0u, "MSG_FIELD_EVENTSEQUENCE", "0" },
  { "weapon", 202, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "weaponModel", 216, 1, 4, 0u, "MAX_WEAPONMODEL_BITS", "0" },
  { "renderOptions", 132, 4, 32, 0u, "MAX_RENDER_OPTIONS_BITS", "0" },
  { "eventParms[0]", 180, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "surfType", 212, 1, 8, 0u, "8", "0" },
  { "un2.animState.fAimUpDown", 144, 4, -77, 0u, "MSG_FIELD_NEG1TO1_P2", "0" },
  {
    "un2.animState.fAimLeftRight",
    148,
    4,
    -77,
    0u,
    "MSG_FIELD_NEG1TO1_P2",
    "0"
  },
  { "targetname", 206, 2, 16, 0u, "16", "0" },
  { "lerp.u.actor.actorNum", 84, 4, 4, 0u, "ACTOR_BITS", "0" },
  { "time2", 120, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "index", 194, 2, 10, 0u, "SUBMODEL_BITS", "0" },
  { "solid", 128, 4, 24, 0u, "24", "0" },
  { "enemyModel", 200, 2, 10, 0u, "SUBMODEL_BITS", "0" },
  { "un1", 217, 1, 8, 0u, "8", "0" },
  {
    "lerp.u.actor.proneInfo.fBodyPitch",
    100,
    2,
    -75,
    0u,
    "MSG_FIELD_PITCH_P1",
    "0"
  },
  { "lerp.u.actor.enemy", 96, 4, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "lerp.apos.trBase[0]", 60, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "clientNum", 213, 1, 6, 0u, "CLIENT_BITS+1", "0" },
  { "lerp.pos.trDelta[0]", 36, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trDelta[1]", 40, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trDelta[2]", 44, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "events[1]", 177, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "events[2]", 178, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventParms[1]", 182, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "eventParms[2]", 184, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.pos.trTime", 16, 4, -68, 0u, "MSG_FIELD_POS_TRTIME", "0" },
  { "lerp.pos.trType", 12, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "eventParm", 188, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.apos.trType", 48, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "events[3]", 179, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "lerp.apos.trBase[2]", 68, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.apos.trTime", 52, 4, -70, 0u, "MSG_FIELD_ANGLE_TRTIME", "0" },
  { "lerp.apos.trDelta[0]", 72, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "lerp.apos.trDelta[2]", 80, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "eventParms[3]", 186, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.pos.trDuration", 20, 4, -67, 0u, "MSG_FIELD_POS_TRDURATION", "0" },
  { "lerp.apos.trDelta[1]", 76, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "attackerEntityNum", 198, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "lerp.apos.trDuration", 56, 4, -69, 0u, "MSG_FIELD_ANGLE_TRDURATION", "0" },
  { "loopSoundId", 124, 4, 32, 0u, "SOUNDALIAS_BITS", "0" },
  { "loopSoundFade", 208, 2, -16, 0u, "FADETIME_BITS", "0" },
  { "otherEntityNum", 196, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "lerp.u.actor.species", 88, 4, 1, 0u, "AI_SPECIES_BITS", "0" },
  { "lerp.u.actor.team", 92, 4, 2, 0u, "2", "0" },
  { "lerp.useCount", 116, 4, 6, 0u, "USE_COUNT_BITS", "0" },
  { "lerp.u.anonymous.data[5]", 104, 4, 32, 0u, "32", "0" },
  { "clientLinkInfo", 218, 4, 26, 0u, "CLIENTLINKINFO_BITS", "0" },
  { "lerp.u.anonymous.data[6]", 108, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[7]", 112, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "iHeadIcon", 214, 1, 4, 1u, "HEAD_ICON_BITS", "NEVER_CHANGES" },
  { "faction", 215, 1, 7, 1u, "2 + CLIENT_BITS", "NEVER_CHANGES" },
  { "partBits[0]", 156, 4, 32, 0u, "32", "0" },
  { "partBits[1]", 160, 4, 32, 0u, "32", "0" },
  { "partBits[2]", 164, 4, 32, 0u, "32", "0" },
  { "partBits[3]", 168, 4, 32, 0u, "32", "0" },
  { "partBits[4]", 172, 4, 32, 0u, "32", "0" },
  { "un3", 152, 4, 10, 1u, "SUBMODEL_BITS", "NEVER_CHANGES" }
};


const int numEventEntityStateFields = 69;
const NetField eventEntityStateFields[69] =
{
  { "eType", 190, 2, -58, 0u, "MSG_FIELD_ETYPE", "0" },
  { "lerp.pos.trBase[0]", 24, 4, -66, 0u, "MSG_FIELD_ES_ORIGINX", "0" },
  { "lerp.pos.trBase[1]", 28, 4, -65, 0u, "MSG_FIELD_ES_ORIGINY", "0" },
  { "lerp.pos.trBase[2]", 32, 4, -64, 0u, "MSG_FIELD_ES_ORIGINZ", "0" },
  { "eventParm", 188, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "surfType", 212, 1, 8, 0u, "8", "0" },
  { "otherEntityNum", 196, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "un1", 217, 1, 8, 0u, "8", "0" },
  { "lerp.u.anonymous.data[0]", 84, 4, 32, 0u, "32", "0" },
  { "lerp.u.anonymous.data[1]", 88, 4, 32, 0u, "32", "0" },
  { "attackerEntityNum", 198, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "lerp.apos.trBase[0]", 60, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "clientNum", 213, 1, 6, 0u, "CLIENT_BITS+1", "0" },
  { "weapon", 202, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "weaponModel", 216, 1, 4, 0u, "MAX_WEAPONMODEL_BITS", "0" },
  { "renderOptions", 132, 4, 32, 0u, "MAX_RENDER_OPTIONS_BITS", "0" },
  { "lerp.u.anonymous.data[2]", 92, 4, 32, 0u, "32", "0" },
  { "index", 194, 2, 10, 0u, "SUBMODEL_BITS", "0" },
  { "solid", 128, 4, 24, 0u, "24", "0" },
  { "lerp.apos.trBase[1]", 64, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "lerp.apos.trBase[2]", 68, 4, -62, 0u, "MSG_FIELD_ES_ANGLE", "0" },
  { "groundEntityNum", 192, 2, -96, 0u, "MSG_FIELD_GROUNDENTITY", "0" },
  { "lerp.u.anonymous.data[4]", 100, 4, 32, 0u, "32", "0" },
  { "lerp.u.anonymous.data[5]", 104, 4, 32, 0u, "32", "0" },
  { "events[0]", 176, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventParms[0]", 180, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.pos.trType", 12, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "lerp.apos.trType", 48, 1, 8, 0u, "MSG_TRAJECTORY_BITS", "0" },
  { "lerp.apos.trTime", 52, 4, -70, 0u, "MSG_FIELD_ANGLE_TRTIME", "0" },
  { "lerp.apos.trDelta[0]", 72, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "lerp.apos.trDelta[2]", 80, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "lerp.pos.trDelta[0]", 36, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trDelta[1]", 40, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "lerp.pos.trDelta[2]", 44, 4, -63, 0u, "MSG_FIELD_ES_ORIGIN_DELTA", "0" },
  { "eventSequence", 210, 2, -59, 0u, "MSG_FIELD_EVENTSEQUENCE", "0" },
  { "lerp.pos.trTime", 16, 4, -68, 0u, "MSG_FIELD_POS_TRTIME", "0" },
  { "events[1]", 177, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "events[2]", 178, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventParms[1]", 182, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "eventParms[2]", 184, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "events[3]", 179, 1, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "eventParms[3]", 186, 2, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "lerp.eFlags", 4, 4, -98, 0u, "MSG_FIELD_EFLAGS", "0" },
  { "lerp.eFlags2", 8, 4, 32, 0u, "32", "0" },
  { "lerp.pos.trDuration", 20, 4, -67, 0u, "MSG_FIELD_POS_TRDURATION", "0" },
  { "lerp.apos.trDelta[1]", 76, 4, -61, 0u, "MSG_FIELD_ES_ANGLE_DELTA", "0" },
  { "time2", 120, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "loopSoundId", 124, 4, 32, 0u, "SOUNDALIAS_BITS", "0" },
  { "loopSoundFade", 208, 2, -16, 0u, "FADETIME_BITS", "0" },
  { "un3", 152, 4, 32, 0u, "32", "0" },
  { "targetname", 206, 2, 16, 0u, "16", "0" },
  { "lerp.useCount", 116, 4, 6, 0u, "USE_COUNT_BITS", "0" },
  { "clientLinkInfo", 218, 4, 26, 0u, "CLIENTLINKINFO_BITS", "0" },
  { "enemyModel", 200, 2, 10, 1u, "SUBMODEL_BITS", "NEVER_CHANGES" },
  { "un2.anim.torsoAnim", 140, 4, 11, 1u, "ANIM_BITS", "NEVER_CHANGES" },
  { "un2.anim.legsAnim", 136, 4, 11, 1u, "ANIM_BITS", "NEVER_CHANGES" },
  { "un2.anim.fWaistPitch", 148, 4, 0, 1u, "MSG_FIELD_FLOAT", "NEVER_CHANGES" },
  { "un2.anim.fTorsoPitch", 144, 4, 0, 1u, "MSG_FIELD_FLOAT", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[3]", 96, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[6]", 108, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "lerp.u.anonymous.data[7]", 112, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "iHeadIcon", 214, 1, 4, 1u, "HEAD_ICON_BITS", "NEVER_CHANGES" },
  { "faction", 215, 1, 7, 1u, "2 + CLIENT_BITS", "NEVER_CHANGES" },
  {
    "lerp.apos.trDuration",
    56,
    4,
    -69,
    1u,
    "MSG_FIELD_ANGLE_TRDURATION",
    "NEVER_CHANGES"
  },
  { "partBits[0]", 156, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[1]", 160, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[2]", 164, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[3]", 168, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "partBits[4]", 172, 4, 32, 1u, "32", "NEVER_CHANGES" }
};

const int numClientStateFields = 50;
const NetField clientStateFields[50] =
{
  { "score.status_icon", 164, 4, 3, 0u, "STATUS_ICON_BITS", "0" },
  { "score.place", 168, 4, 6, 0u, "CLIENT_BITS + 1", "0" },
  { "score.score", 172, 4, -16, 0u, "SCORE_BITS", "0" },
  { "score.deaths", 184, 4, 10, 0u, "ASSIST_BITS", "0" },
  { "score.scoreboardColumns[1]", 192, 4, 10, 0u, "ASSIST_BITS", "0" },
  { "score.kills", 176, 4, 10, 0u, "ASSIST_BITS", "0" },
  { "score.scoreboardColumns[0]", 188, 4, 10, 0u, "ASSIST_BITS", "0" },
  { "modelindex", 12, 4, 9, 0u, "MODEL_BITS", "0" },
  { "attachModelIndex[0]", 16, 4, 9, 0u, "MODEL_BITS", "0" },
  { "attachTagIndex[1]", 44, 4, 5, 0u, "TAG_BITS", "0" },
  { "attachModelIndex[1]", 20, 4, 9, 0u, "MODEL_BITS", "0" },
  {
    "clientUIVisibilityFlags",
    204,
    4,
    4,
    0u,
    "CLIENT_HUD_VISIBILITY_BITS",
    "0"
  },
  { "score.assists", 180, 4, 10, 0u, "ASSIST_BITS", "0" },
  { "rank", 100, 4, 8, 0u, "RANK_BITS", "0" },
  { "team", 4, 4, 2, 0u, "2", "0" },
  { "name[0]", 64, 4, 32, 0u, "32", "0" },
  { "attachedVehEntNum", 144, 4, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "xuid32[1]", 124, 4, 32, 0u, "32", "0" },
  { "name[4]", 68, 4, 32, 0u, "32", "0" },
  { "perks[0]", 128, 4, 32, 0u, "PERK0_BITS", "0" },
  { "perks[1]", 132, 4, 32, 0u, "PERK1_BITS", "0" },
  { "clanAbbrev[0]", 136, 4, 32, 0u, "32", "0" },
  { "name[8]", 72, 4, 32, 0u, "32", "0" },
  { "name[12]", 76, 4, 32, 0u, "32", "0" },
  { "score.scoreboardColumns[2]", 196, 4, 10, 0u, "ASSIST_BITS", "0" },
  { "score.scoreboardColumns[3]", 200, 4, 10, 0u, "ASSIST_BITS", "0" },
  { "xuid32[0]", 120, 4, 32, 0u, "32", "0" },
  { "prestige", 104, 4, 8, 0u, "PRESTIGE_BITS", "0" },
  { "ffaTeam", 8, 4, 2, 0u, "2", "0" },
  { "needsRevive", 152, 4, 1, 0u, "1", "0" },
  { "maxSprintTimeMultiplier", 96, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "vehAnimState", 156, 4, 2, 0u, "MAX_BITS_VEHICLEANIMSTATE", "0" },
  { "name[16]", 80, 4, 32, 0u, "32", "0" },
  { "name[20]", 84, 4, 32, 0u, "32", "0" },
  { "name[24]", 88, 4, 32, 0u, "32", "0" },
  { "name[28]", 92, 4, 32, 0u, "32", "0" },
  { "clanAbbrev[4]", 140, 4, 32, 0u, "32", "0" },
  { "attachedVehSeat", 148, 4, 4, 0u, "VEHSLOT_BITS", "0" },
  { "attachTagIndex[5]", 60, 4, 5, 0u, "TAG_BITS", "0" },
  { "attachTagIndex[0]", 40, 4, 5, 0u, "TAG_BITS", "0" },
  { "attachTagIndex[2]", 48, 4, 5, 0u, "TAG_BITS", "0" },
  { "attachTagIndex[3]", 52, 4, 5, 0u, "TAG_BITS", "0" },
  { "attachTagIndex[4]", 56, 4, 5, 0u, "TAG_BITS", "0" },
  { "attachModelIndex[2]", 24, 4, 9, 0u, "MODEL_BITS", "0" },
  { "attachModelIndex[3]", 28, 4, 9, 0u, "MODEL_BITS", "0" },
  { "attachModelIndex[4]", 32, 4, 9, 0u, "MODEL_BITS", "0" },
  { "attachModelIndex[5]", 36, 4, 9, 0u, "MODEL_BITS", "0" },
  { "lastDamageTime", 108, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "lastStandStartTime", 112, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "score.ping", 160, 4, 10, 0u, "PING_BITS", "0" }
};


const int numPlayerStateFields = 179;
const NetField playerStateFields[179] =
{
  { "commandTime", 0, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "viewangles[1]", 388, 4, -87, 0u, "MSG_FIELD_VIEWANGLES", "0" },
  { "viewangles[0]", 384, 4, -87, 0u, "MSG_FIELD_VIEWANGLES", "0" },
  { "viewangles[2]", 392, 4, -87, 0u, "MSG_FIELD_VIEWANGLES", "0" },
  {
    "origin[0]",
    36,
    4,
    -88,
    3u,
    "MSG_FIELD_FLOAT_RARELYZERO_NONINT",
    "CLIENT_PREDICTED"
  },
  {
    "origin[1]",
    40,
    4,
    -88,
    3u,
    "MSG_FIELD_FLOAT_RARELYZERO_NONINT",
    "CLIENT_PREDICTED"
  },
  { "bobCycle", 8, 4, 8, 3u, "8", "CLIENT_PREDICTED" },
  {
    "velocity[1]",
    52,
    4,
    -88,
    3u,
    "MSG_FIELD_FLOAT_RARELYZERO_NONINT",
    "CLIENT_PREDICTED"
  },
  {
    "velocity[0]",
    48,
    4,
    -88,
    3u,
    "MSG_FIELD_FLOAT_RARELYZERO_NONINT",
    "CLIENT_PREDICTED"
  },
  { "vehicleType", 1096, 4, 3, 0u, "VEHICLE_TYPES_BITS", "0" },
  { "movementDir", 220, 4, -8, 3u, "-8", "CLIENT_PREDICTED" },
  {
    "predictableEventSequence",
    232,
    2,
    -59,
    0u,
    "MSG_FIELD_EVENTSEQUENCE",
    "0"
  },
  {
    "predictableEventSequenceOld",
    234,
    2,
    -59,
    0u,
    "MSG_FIELD_EVENTSEQUENCE",
    "0"
  },
  { "legsAnim", 188, 2, 11, 0u, "ANIM_BITS", "0" },
  {
    "origin[2]",
    44,
    4,
    -88,
    3u,
    "MSG_FIELD_FLOAT_RARELYZERO_NONINT",
    "CLIENT_PREDICTED"
  },
  { "weaponTime", 60, 4, -16, 0u, "-16", "0" },
  { "weaponTimeLeft", 68, 4, -16, 0u, "-16", "0" },
  {
    "weaponIdleTime",
    76,
    4,
    -71,
    3u,
    "MSG_FIELD_WEAPON_IDLE_TIME",
    "CLIENT_PREDICTED"
  },
  { "aimSpreadScale", 1308, 4, -106, 0u, "MSG_FIELD_AIM_SPREAD_SCALE", "0" },
  { "torsoTimer", 184, 4, 16, 0u, "16", "0" },
  { "pm_flags", 12, 4, 25, 0u, "PMF_BIT_COUNT", "0" },
  { "weapAnim", 1300, 4, 11, 0u, "ANIM_BITS", "0" },
  { "weapAnimLeft", 1304, 4, 11, 0u, "ANIM_BITS", "0" },
  { "weaponstate", 344, 4, 7, 0u, "WEAPON_STATE_BITS", "0" },
  { "weaponstateLeft", 348, 4, 7, 0u, "WEAPON_STATE_BITS", "0" },
  {
    "velocity[2]",
    56,
    4,
    -88,
    3u,
    "MSG_FIELD_FLOAT_RARELYZERO_NONINT",
    "CLIENT_PREDICTED"
  },
  { "viewAngleClampBase[1]", 420, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "viewAngleClampBase[0]", 416, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "predictableEventParms[0]", 252, 4, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "predictableEventParms[1]", 256, 4, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "predictableEvents[3]", 248, 4, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "predictableEvents[2]", 244, 4, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "predictableEvents[1]", 240, 4, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "predictableEvents[0]", 236, 4, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "predictableEventParms[2]", 260, 4, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "predictableEventParms[3]", 264, 4, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  {
    "unpredictableEventSequence",
    268,
    2,
    -59,
    0u,
    "MSG_FIELD_EVENTSEQUENCE",
    "0"
  },
  {
    "unpredictableEventSequenceOld",
    270,
    2,
    -59,
    0u,
    "MSG_FIELD_EVENTSEQUENCE",
    "0"
  },
  { "unpredictableEventParms[0]", 288, 4, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "unpredictableEventParms[1]", 292, 4, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "unpredictableEvents[3]", 284, 4, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "unpredictableEvents[2]", 280, 4, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "unpredictableEvents[1]", 276, 4, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "unpredictableEvents[0]", 272, 4, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "unpredictableEventParms[2]", 296, 4, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "unpredictableEventParms[3]", 300, 4, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "torsoAnim", 190, 2, 11, 0u, "ANIM_BITS", "0" },
  {
    "holdBreathScale",
    1192,
    4,
    -88,
    0u,
    "MSG_FIELD_FLOAT_RARELYZERO_NONINT",
    "0"
  },
  { "eFlags", 224, 4, -98, 0u, "MSG_FIELD_EFLAGS", "0" },
  { "viewHeightCurrent", 400, 4, -107, 0u, "MSG_FIELD_VIEW_HEIGHT", "0" },
  { "fWeaponPosFrac", 360, 4, -80, 0u, "MSG_FIELD_0TO1_P3", "0" },
  { "moveType", 176, 4, 5, 0u, "ANIM_MOVETYPE_BITS", "0" },
  { "legsTimer", 180, 4, 16, 0u, "16", "0" },
  { "viewHeightTarget", 396, 4, -8, 0u, "-8", "0" },
  { "sprintState.lastSprintStart", 1160, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "sprintState.lastSprintEnd", 1164, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "lastDtpEnd", 1180, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "weapon", 324, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "stowedWeapon", 332, 4, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "stowedWeaponCamo", 336, 1, 6, 0u, "RENDER_OPTIONS_CAMO_BITS", "0" },
  { "lastWeaponAltModeSwitch", 328, 4, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "meleeWeapon", 340, 4, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "renderOptions", 320, 4, 32, 0u, "MAX_RENDER_OPTIONS_BITS", "0" },
  { "weaponDelay", 64, 4, -16, 0u, "-16", "0" },
  { "sprintState.sprintStartMaxLength", 1168, 4, 14, 0u, "14", "0" },
  { "sprintState.sprintDuration", 1172, 4, 14, 0u, "14", "0" },
  { "sprintState.sprintCooldown", 1176, 4, 14, 0u, "14", "0" },
  { "weapFlags", 16, 4, 24, 0u, "PWF_BIT_COUNT", "0" },
  { "weapLockFlags", 1248, 4, 6, 0u, "WEAPONLOCKFLAG_BITS", "0" },
  { "groundEntityNum", 144, 4, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "damageTimer", 200, 4, 10, 0u, "10", "0" },
  { "delta_angles[1]", 136, 4, -100, 0u, "MSG_FIELD_ANGLE2SHORT", "0" },
  { "offHandIndex", 308, 4, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "pm_time", 24, 4, -16, 0u, "-16", "0" },
  { "otherFlags", 20, 4, 6, 0u, "POF_BIT_COUNT", "0" },
  { "moveSpeedScaleMultiplier", 1200, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "perks[0]", 1256, 4, 32, 0u, "PERK0_BITS", "0" },
  { "perks[1]", 1260, 4, 32, 0u, "PERK1_BITS", "0" },
  { "visionSetLerpRatio", 1384, 4, -80, 0u, "MSG_FIELD_0TO1_P3", "0" },
  { "poisoned", 1388, 4, 1, 2u, "1", "ALWAYS_CHANGES" },
  { "killCamEntity", 2940, 4, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "killCamTargetEntity", 2944, 4, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "throwBackGrenadeOwner", 84, 4, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "actionSlotType[2]", 1272, 4, 2, 0u, "MAX_BITS_ACTIONSLOTTYPE", "0" },
  { "delta_angles[0]", 132, 4, -100, 0u, "MSG_FIELD_ANGLE2SHORT", "0" },
  { "speed", 128, 4, 16, 0u, "16", "0" },
  { "viewlocked_entNum", 1088, 2, 16, 0u, "16", "0" },
  { "gravity", 120, 4, 16, 0u, "16", "0" },
  { "actionSlotType[0]", 1264, 4, 2, 0u, "MAX_BITS_ACTIONSLOTTYPE", "0" },
  { "dofNearBlur", 1340, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "dofFarBlur", 1344, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "eFlags2", 228, 4, 32, 0u, "32", "0" },
  { "clientNum", 304, 1, 8, 0u, "8", "0" },
  { "corpseIndex", 216, 4, 3, 0u, "CLIENT_CORPSE_INDEX_BITS", "0" },
  { "damageEvent", 436, 4, 8, 0u, "8", "0" },
  { "viewHeightLerpTarget", 408, 4, -8, 0u, "-8", "0" },
  { "damageYaw", 440, 4, 8, 0u, "8", "0" },
  { "viewmodelIndex", 380, 4, 9, 0u, "MODEL_BITS", "0" },
  { "damageDuration", 204, 4, 16, 0u, "16", "0" },
  { "damagePitch", 444, 4, 8, 0u, "8", "0" },
  { "dmgDirection", 208, 4, 3, 0u, "3", "0" },
  { "dmgType", 212, 4, 4, 0u, "4", "0" },
  { "weaponShotCount", 352, 4, 3, 0u, "PLAYER_WEAPONSHOTCOUNT_BITS", "0" },
  { "weaponShotCountLeft", 356, 4, 3, 0u, "PLAYER_WEAPONSHOTCOUNT_BITS", "0" },
  { "stackFireCount", 432, 4, 3, 0u, "PLAYER_WEAPONSHOTCOUNT_BITS", "0" },
  { "viewHeightLerpDown", 412, 4, 1, 2u, "1", "ALWAYS_CHANGES" },
  { "cursorHint", 1120, 4, 11, 0u, "CURSOR_HINT_BITS", "0" },
  { "cursorHintString", 1124, 4, -8, 0u, "-8", "0" },
  { "cursorHintEntIndex", 1128, 4, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "viewHeightLerpTime", 404, 4, 32, 0u, "32", "0" },
  { "offhandSecondary", 312, 4, 1, 2u, "1", "ALWAYS_CHANGES" },
  { "offhandPrimary", 316, 4, 1, 2u, "1", "ALWAYS_CHANGES" },
  { "spyplaneTypeEnabled", 1136, 4, 2, 0u, "2", "0" },
  { "satelliteTypeEnabled", 1140, 4, 2, 0u, "2", "0" },
  { "pm_type", 4, 4, 8, 0u, "8", "0" },
  { "fTorsoPitch", 1184, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "holdBreathTimer", 1196, 4, 16, 0u, "16", "0" },
  { "actionSlotParam[2]", 1288, 4, 11, 0u, "MAX_BITS_ACTIONSLOTPARAM", "0" },
  { "jumpTime", 164, 4, 32, 0u, "32", "0" },
  { "slideTime", 172, 4, 32, 0u, "32", "0" },
  { "mantleState.flags", 1216, 4, 12, 0u, "MANTLE_FLAG_BITS", "0" },
  { "fWaistPitch", 1188, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "grenadeTimeLeft", 80, 4, -16, 0u, "-16", "0" },
  { "proneDirection", 1072, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "mantleState.timer", 1208, 4, 32, 0u, "32", "0" },
  { "damageCount", 448, 4, 7, 0u, "7", "0" },
  { "shellshockTime", 1316, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "shellshockDuration", 1320, 4, 16, 2u, "16", "ALWAYS_CHANGES" },
  {
    "sprintState.sprintButtonUpRequired",
    1152,
    4,
    1,
    2u,
    "1",
    "ALWAYS_CHANGES"
  },
  { "shellshockIndex", 1312, 4, 4, 0u, "4", "0" },
  { "proneTorsoPitch", 1080, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "sprintState.sprintDelay", 1156, 4, 1, 2u, "1", "ALWAYS_CHANGES" },
  { "actionSlotParam[3]", 1292, 4, 11, 0u, "MAX_BITS_ACTIONSLOTPARAM", "0" },
  { "actionSlotType[3]", 1276, 4, 2, 0u, "MAX_BITS_ACTIONSLOTTYPE", "0" },
  { "proneDirectionPitch", 1076, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "jumpOriginZ", 168, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "mantleState.yaw", 1204, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "mantleState.transIndex", 1212, 4, 4, 0u, "MANTLE_TRANS_BITS", "0" },
  { "throwBackGrenadeTimeLeft", 88, 4, -16, 0u, "-16", "0" },
  { "foliageSoundTime", 116, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "vLadderVec[0]", 152, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "viewlocked", 1084, 4, 2, 0u, "MAX_BITS_PLAYERVIEWLOCK", "0" },
  { "deltaTime", 2936, 4, 32, 0u, "32", "0" },
  { "linkAngles[1]", 1112, 4, -100, 0u, "MSG_FIELD_ANGLE2SHORT", "0" },
  { "viewAngleClampRange[1]", 428, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "viewAngleClampRange[0]", 424, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "vLadderVec[1]", 156, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  {
    "locationSelectionInfo",
    1148,
    4,
    8,
    0u,
    "LOC_SEL_MTL_BITS + LOC_SEL_RADIUS_BITS",
    "0"
  },
  { "locationSelectionType", 1144, 4, 3, 0u, "3", "0" },
  { "meleeChargeTime", 1244, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "meleeChargeYaw", 1236, 4, -100, 0u, "MSG_FIELD_ANGLE2SHORT", "0" },
  { "meleeChargeDist", 1240, 4, 8, 0u, "8", "0" },
  { "iCompassPlayerInfo", 1132, 4, 32, 0u, "32", "0" },
  { "actionSlotType[1]", 1268, 4, 2, 0u, "MAX_BITS_ACTIONSLOTTYPE", "0" },
  { "vLadderVec[2]", 160, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "weaponRestrictKickTime", 92, 4, -16, 0u, "-16", "0" },
  { "delta_angles[2]", 140, 4, -100, 0u, "MSG_FIELD_ANGLE2SHORT", "0" },
  { "spreadOverride", 368, 4, 6, 0u, "PLAYER_SPREAD_OVERRIDE_BITS", "0" },
  {
    "spreadOverrideState",
    372,
    4,
    2,
    0u,
    "PLAYER_SPREAD_OVERRIDE_STATE_BITS",
    "0"
  },
  { "actionSlotParam[0]", 1280, 4, 11, 0u, "MAX_BITS_ACTIONSLOTPARAM", "0" },
  { "actionSlotParam[1]", 1284, 4, 11, 0u, "MAX_BITS_ACTIONSLOTPARAM", "0" },
  { "dofNearStart", 1324, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "dofNearEnd", 1328, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "dofFarStart", 1332, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "dofFarEnd", 1336, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "dofViewmodelStart", 1348, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "dofViewmodelEnd", 1352, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "scriptedAnim", 1228, 4, 32, 0u, "32", "0" },
  { "adsZoomSelect", 1368, 4, 32, 0u, "32", "0" },
  { "adsZoomLatchState", 1376, 1, 1, 0u, "1", "0" },
  { "leanf", 124, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "vehiclePos", 1092, 4, 4, 0u, "MAX_VEHICLE_SEAT_BITS", "0" },
  {
    "artilleryInboundIconLocation",
    1380,
    4,
    19,
    0u,
    "ARTILLERY_LOCATION_MESSAGE_BITS",
    "0"
  },
  { "weaponSpinLerp", 376, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "lastStandPrevWeapon", 326, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "adsDelayTime", 364, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "vehicleAnimBoneIndex", 1100, 4, 8, 1u, "MAX_BONE_BITS", "NEVER_CHANGES" },
  {
    "vehicleAnimStage",
    1220,
    4,
    4,
    1u,
    "VEHICLE_ANIM_STAGE_BITS",
    "NEVER_CHANGES"
  },
  {
    "vehicleEntryPoint",
    1224,
    4,
    3,
    1u,
    "VEHICLE_ENTRY_POINT_BITS",
    "NEVER_CHANGES"
  }
};


const int numObjectiveFields = 9;
const NetField objectiveFields[9] =
{
  { "origin[0]", 4, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "origin[1]", 8, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "origin[2]", 12, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "size[0]", 16, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "size[1]", 20, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "icon", 32, 4, 12, 0u, "12", "0" },
  { "color", 40, 4, -85, 0u, "MSG_FIELD_RGBA", "0" },
  { "entNum", 24, 4, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "teamNum", 28, 4, 4, 0u, "4", "0" }
};

const int numMatchStateFields = 15;
const NetField matchStateFields[15] =
{
  { "unarchivedState.alliesScore", 16, 4, -16, 0u, "SCORE_BITS", "0" },
  { "unarchivedState.axisScore", 20, 4, -16, 0u, "SCORE_BITS", "0" },
  {
    "archivedState.matchUIVisibilityFlags",
    4,
    4,
    13,
    0u,
    "MATCH_HUD_VISIBILITY_BITS",
    "0"
  },
  {
    "unarchivedState.matchUIVisibilityFlags",
    28,
    4,
    13,
    0u,
    "MATCH_HUD_VISIBILITY_BITS",
    "0"
  },
  { "unarchivedState.scoreLimit", 24, 4, -16, 0u, "SCORE_BITS", "0" },
  { "unarchivedState.talkFlags", 60, 4, 5, 0u, "TALKFLAG_BITS", "0" },
  {
    "unarchivedState.scoreboardColumnTypes[0]",
    32,
    4,
    5,
    0u,
    "COLUMN_TYPE_BITS",
    "0"
  },
  {
    "unarchivedState.scoreboardColumnTypes[1]",
    36,
    4,
    5,
    0u,
    "COLUMN_TYPE_BITS",
    "0"
  },
  {
    "unarchivedState.scoreboardColumnTypes[2]",
    40,
    4,
    5,
    0u,
    "COLUMN_TYPE_BITS",
    "0"
  },
  {
    "unarchivedState.scoreboardColumnTypes[3]",
    44,
    4,
    5,
    0u,
    "COLUMN_TYPE_BITS",
    "0"
  },
  { "unarchivedState.mapCenter[0]", 48, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "unarchivedState.mapCenter[1]", 52, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "unarchivedState.mapCenter[2]", 56, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "archivedState.bombTimer[0]", 8, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "archivedState.bombTimer[1]", 12, 4, -97, 0u, "MSG_FIELD_TIME", "0" }
};


unsigned int kbitmask[33] =
{
  0u,
  1u,
  3u,
  7u,
  15u,
  31u,
  63u,
  127u,
  255u,
  511u,
  1023u,
  2047u,
  4095u,
  8191u,
  16383u,
  32767u,
  65535u,
  131071u,
  262143u,
  524287u,
  1048575u,
  2097151u,
  4194303u,
  8388607u,
  16777215u,
  33554431u,
  67108863u,
  134217727u,
  268435455u,
  536870911u,
  1073741823u,
  2147483647u,
  4294967295u
};



int msgInit;
huffman_t msgHuff;
unsigned int huffBytesSeen[256];
netFieldOrderInfo_t orderInfo;

float (*__cdecl MSG_GetMapCenter())[3]
{
    // LWSS: This is a very important if() lol. 
    // Otherwise remote clients will not have the right starting base for any positions
    if (IsDedicatedServer())
    {
        return SV_GetMapCenter();
    }
    else
    {
        return CL_GetMapCenter();
    }
}

int __cdecl GetMinBitCountForNum(unsigned int num)
{
    int v2; // eax

    if (!_BitScanReverse((unsigned long *)&v2, num))
    {
        //v2 = `CountLeadingZeros'::`2': : notFound;
        v2 = 63;
    }

    return 32 - (v2 ^ 0x1F);
}

void __cdecl MSG_Init(msg_t *buf, unsigned __int8 *data, int length)
{
    if ( !msgInit )
        MSG_InitHuffman();

    memset(buf, 0, sizeof(msg_t));

    buf->data = data;
    buf->maxsize = length;
}

void __cdecl MSG_InitReadOnly(msg_t *buf, unsigned __int8 *data, int length)
{
    if ( !data && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 94, 0, "%s", "data") )
        __debugbreak();
    if ( !msgInit )
        MSG_InitHuffman();
    buf->readOnly = 1;
    buf->data = data;
    buf->maxsize = length;
    buf->cursize = length;
    buf->splitData = 0;
    buf->splitSize = 0;
}

void __cdecl MSG_InitReadOnlySplit(msg_t *buf, unsigned __int8 *data, int length, unsigned __int8 *data2, int length2)
{
    if ( !data && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 111, 0, "%s", "data") )
        __debugbreak();
    if ( !data2 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 112, 0, "%s", "data2") )
        __debugbreak();
    if ( !msgInit )
        MSG_InitHuffman();
    buf->readOnly = 1;
    buf->data = data;
    buf->maxsize = length2 + length;
    buf->cursize = length;
    buf->splitData = data2;
    buf->splitSize = length2;
}

void __cdecl MSG_BeginReading(msg_t *msg)
{
    msg->overflowed = 0;
    msg->readcount = 0;
    msg->bit = 0;
}

void __cdecl MSG_Discard(msg_t *msg)
{
    msg->overflowed = 1;
    msg->cursize = msg->readcount;
    msg->splitSize = 0;
}

int __cdecl MSG_GetUsedBitCount(const msg_t *msg)
{
    return 8 * (msg->splitSize + msg->cursize) - ((8 - msg->bit) & 7);
}

int __cdecl MSG_GetNumBitsRead(const msg_t *msg)
{
    return 8 * msg->readcount - ((8 - msg->bit) & 7);
}

void __cdecl MSG_SetBookmark(const msg_t *msg, msg_bookmark_t *bookmark)
{
    int msgBit; // [esp+0h] [ebp-4h]

    if ( msg->readOnly
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 184, 0, "%s", "!msg->readOnly") )
    {
        __debugbreak();
    }
    msgBit = msg->bit;
    bookmark->overflowed = msg->overflowed;
    bookmark->cursize = msg->cursize;
    bookmark->bit = msgBit;
    bookmark->lastEntityRef = msg->lastEntityRef;
    bookmark->bitByte = msg->data[msgBit >> 3];
    bookmark->readcount = msg->readcount;
}

void __cdecl MSG_GotoBookmark(msg_t *msg, const msg_bookmark_t *bookmark)
{
    int msgBit; // [esp+0h] [ebp-4h]

    if ( msg->readOnly
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 197, 0, "%s", "!msg->readOnly") )
    {
        __debugbreak();
    }
    msgBit = bookmark->bit;
    msg->overflowed = bookmark->overflowed;
    msg->cursize = bookmark->cursize;
    msg->bit = msgBit;
    msg->lastEntityRef = bookmark->lastEntityRef;
    msg->data[msgBit >> 3] = bookmark->bitByte;
    msg->readcount = bookmark->readcount;
}

void __cdecl MSG_WriteBits(msg_t *msg, int value, unsigned int bits)
{
    int bit; // [esp+4h] [ebp-4h]

    if ( bits > 0x20
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 277, 0, "%s", "(unsigned)bits <= 32") )
    {
        __debugbreak();
    }
    if ( msg->readOnly
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 278, 0, "%s", "!msg->readOnly") )
    {
        __debugbreak();
    }
    if ( msg->maxsize - msg->cursize >= 4 )
    {
        while ( bits )
        {
            --bits;
            bit = msg->bit & 7;
            if ( !bit )
            {
                msg->bit = 8 * msg->cursize;
                msg->data[msg->cursize++] = 0;
            }
            if ( (value & 1) != 0 )
                msg->data[msg->bit >> 3] |= 1 << bit;
            ++msg->bit;
            value >>= 1;
        }
    }
    else
    {
        msg->overflowed = 1;
    }
}

void __cdecl MSG_WriteBit0(msg_t *msg)
{
    if ( msg->readOnly )
    {
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 309, 0, "%s", "!msg->readOnly") )
            __debugbreak();
    }
    if ( (msg->bit & 7) == 0 )
    {
        if ( msg->cursize >= msg->maxsize )
        {
            msg->overflowed = 1;
            return;
        }
        msg->bit = 8 * msg->cursize;
        msg->data[msg->cursize++] = 0;
    }
    ++msg->bit;
}

void __cdecl MSG_WriteBit1(msg_t *msg)
{
    int bit; // [esp+4h] [ebp-4h]

    if ( msg->readOnly )
    {
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 332, 0, "%s", "!msg->readOnly") )
            __debugbreak();
    }
    bit = msg->bit & 7;
    if ( !bit )
    {
        if ( msg->cursize >= msg->maxsize )
        {
            msg->overflowed = 1;
            return;
        }
        msg->bit = 8 * msg->cursize;
        msg->data[msg->cursize++] = 0;
    }
    msg->data[msg->bit++ >> 3] |= 1 << bit;
}

int __cdecl MSG_ReadBits(msg_t *msg, unsigned int bits)
{
    unsigned int bit; // [esp+0h] [ebp-Ch]
    int value; // [esp+8h] [ebp-4h]

    iassert((unsigned)bits <= 32);

    value = 0;

    for (int i = 0; i < bits; ++i)
    {
        bit = msg->bit & 7;
        if (!bit)
        {
            if (msg->readcount >= msg->splitSize + msg->cursize)
            {
                msg->overflowed = 1;
                return -1;
            }
            msg->bit = 8 * msg->readcount++;
        }
        value |= ((MSG_GetByte(msg, msg->bit / 8) >> bit) & 1) << i;
        ++msg->bit;
    }

    return value;
}

int __cdecl MSG_GetByte(msg_t *msg, int where)
{
    if ( where < msg->cursize )
        return msg->data[where];

    iassert(msg->splitData);

    return msg->splitData[where - msg->cursize];
}

int __cdecl MSG_ReadBit(msg_t *msg)
{
    int Byte; // eax
    int bit; // [esp+0h] [ebp-8h]

    bit = msg->bit & 7;
    if ( !bit )
    {
        if ( msg->readcount >= msg->splitSize + msg->cursize )
        {
            msg->overflowed = 1;
            return -1;
        }
        msg->bit = 8 * msg->readcount++;
    }
    Byte = MSG_GetByte(msg, msg->bit >> 3);
    ++msg->bit;
    return (Byte >> bit) & 1;
}

unsigned int __cdecl MSG_CompressWithZLib(
                unsigned __int8 *from,
                unsigned int inSizeBytes,
                unsigned __int8 *to,
                unsigned int outSizeBytes)
{
    int v5; // [esp+0h] [ebp-3Ch]
    int err; // [esp+4h] [ebp-38h]
    int erra; // [esp+4h] [ebp-38h]
    z_stream_s stream; // [esp+8h] [ebp-34h] BYREF

    stream.next_in = from;
    stream.avail_in = inSizeBytes;
    stream.next_out = to;
    stream.avail_out = outSizeBytes;
    memset(&stream.zalloc, 0, 16);
    err = deflateInit2_(&stream, 9, 8, -13, 1, 0, "1.2.3", 52);
    if ( !err )
    {
        erra = deflate(&stream, 4u);
        if ( erra == 1 )
        {
            err = deflateEnd(&stream);
        }
        else
        {
            deflateEnd(&stream);
            if ( erra )
                v5 = erra;
            else
                v5 = -5;
            err = v5;
        }
    }
    if ( err )
        return 0;
    else
        return stream.total_out;
}

int __cdecl MSG_WriteBitsCompress(
                bool trainHuffman,
                unsigned __int8 *from,
                int fromSizeBytes,
                unsigned __int8 *to,
                int toSizeBytes)
{
    int bit; // [esp+0h] [ebp-Ch] BYREF
    int compressedSize; // [esp+4h] [ebp-8h]

    if ( msg_zlibCompress->current.enabled
        && (*to = COMPRESSION_TYPE_ZLIB, (compressedSize = MSG_CompressWithZLib(from, fromSizeBytes, to + 1, toSizeBytes - 1)) != 0)
        && compressedSize < fromSizeBytes + 1 )
    {
        if ( msg_zlibCompressOutput->current.enabled )
            Com_Printf(15, "zlib compress %d->%d\n", fromSizeBytes, compressedSize);
        return compressedSize + 1;
    }
    else
    {
        if ( trainHuffman )
        {
            for ( int i = 0; i < fromSizeBytes; ++i )
                ++huffBytesSeen[from[i]];
        }

        *to = COMPRESSION_TYPE_HUFFMAN;
        bit = 0;

        for ( int i = 0; i < fromSizeBytes; ++i )
            Huff_offsetTransmit(&msgHuff.compressDecompress, from[i], to + 1, &bit);
        compressedSize = ((bit + 7) >> 3) + 1;
        if ( compressedSize > fromSizeBytes + 1 )
        {
            *to = COMPRESSION_TYPE_NONE;
            memcpy(to + 1, from, fromSizeBytes);
            return fromSizeBytes + 1;
        }
        return compressedSize;
    }
}

void __cdecl showHuffmanData()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 0x100; ++i )
        Com_Printf(15, "\t%i,\t\t// %i\n", huffBytesSeen[i] >> 4, i);
}

unsigned int __cdecl MSG_DecompressWithZLib(
                unsigned __int8 *from,
                unsigned int inSizeBytes,
                unsigned __int8 *to,
                unsigned int outSizeBytes)
{
    int v5; // [esp+0h] [ebp-3Ch]
    int err; // [esp+4h] [ebp-38h]
    int erra; // [esp+4h] [ebp-38h]
    z_stream_s stream; // [esp+8h] [ebp-34h] BYREF

    stream.next_in = from;
    stream.avail_in = inSizeBytes;
    stream.next_out = to;
    stream.avail_out = outSizeBytes;
    memset(&stream.zalloc, 0, 16);
    err = inflateInit2_(&stream, -13, "1.2.3", 52);
    if ( !err )
    {
        erra = inflate(&stream, 4);
        if ( erra == 1 )
        {
            err = inflateEnd(&stream);
        }
        else
        {
            inflateEnd(&stream);
            if ( erra )
                v5 = erra;
            else
                v5 = -5;
            err = v5;
        }
    }
    if ( err )
        return 0;
    else
        return stream.total_out;
}

unsigned int __cdecl MSG_ReadBitsCompress(
                unsigned __int8 *from,
                int fromSizeBytes,
                unsigned __int8 *to,
                unsigned int toSizeBytes)
{
    int bit; // [esp+4h] [ebp-18h] BYREF
    unsigned __int8 *data; // [esp+8h] [ebp-14h]
    int bits; // [esp+Ch] [ebp-10h]
    compressionType_t compressionType; // [esp+13h] [ebp-9h]
    int i; // [esp+14h] [ebp-8h]
    int get; // [esp+18h] [ebp-4h] BYREF

    compressionType = (compressionType_t)*from;
    from++;
    fromSizeBytes--;

    if (compressionType == COMPRESSION_TYPE_NONE)
    {
        memcpy(to, from, fromSizeBytes);
        return fromSizeBytes;
    }

    if ( compressionType == COMPRESSION_TYPE_HUFFMAN )
    {
        bits = 8 * fromSizeBytes;
        i = 0;
        data = to;
        bit = 0;
        while ( bit < bits )
        {
            Huff_offsetReceive(msgHuff.compressDecompress.tree, &get, from, &bit);
            *data++ = get;
        }
        return data - to;
    }
    else if ( compressionType == COMPRESSION_TYPE_ZLIB )
    {
        toSizeBytes = MSG_DecompressWithZLib(from, fromSizeBytes, to, toSizeBytes);
        if ( msg_zlibCompressOutput->current.enabled )
            Com_Printf(15, "zlib decompress %d\n", toSizeBytes);
        return toSizeBytes;
    }
    else
    {
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 578, 0, va("Unknown compression protocol %d", compressionType)) )
            __debugbreak();
        return 0;
    }
}

void __cdecl MSG_Embed(msg_t *dest, msg_t *src)
{
    unsigned int padBits; // [esp+0h] [ebp-4h]

    if ( dest->maxsize - dest->cursize >= src->cursize + 1 )
    {
        padBits = (5 - dest->bit) & 7;
        MSG_WriteBits(dest, padBits, 3u);
        MSG_WriteBits(dest, 0, padBits);
        memcpy(&dest->data[dest->cursize], src->data, src->cursize);
        dest->bit = src->bit + 8 * dest->cursize;
        dest->cursize += src->cursize;
    }
    else
    {
        dest->overflowed = 1;
    }
}

void __cdecl MSG_UnEmbed(msg_t *msg)
{
    unsigned int padBits; // [esp+0h] [ebp-4h]

    padBits = MSG_ReadBits(msg, 3u);
    MSG_ReadBits(msg, padBits);
}

void __cdecl MSG_WriteByte(msg_t *msg, unsigned __int8 c)
{
    if ( msg->readOnly
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 620, 0, "%s", "!msg->readOnly") )
    {
        __debugbreak();
    }
    if ( msg->cursize >= msg->maxsize )
        msg->overflowed = 1;
    else
        msg->data[msg->cursize++] = c;
}

void __cdecl MSG_WriteData(msg_t *buf, unsigned __int8 *data, unsigned int length)
{
    int newsize; // [esp+0h] [ebp-4h]

    if ( buf->readOnly
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 636, 0, "%s", "!buf->readOnly") )
    {
        __debugbreak();
    }
    newsize = length + buf->cursize;
    if ( newsize > buf->maxsize )
    {
        buf->overflowed = 1;
    }
    else
    {
        memcpy(&buf->data[buf->cursize], data, length);
        buf->cursize = newsize;
    }
}

void __cdecl MSG_WriteShort(msg_t *msg, unsigned short c)
{
    int newsize; // [esp+0h] [ebp-4h]

    if ( msg->readOnly
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 653, 0, "%s", "!msg->readOnly") )
    {
        __debugbreak();
    }
    newsize = msg->cursize + 2;
    if ( newsize > msg->maxsize )
    {
        msg->overflowed = 1;
    }
    else
    {
        *(_WORD *)&msg->data[msg->cursize] = c;
        msg->cursize = newsize;
    }
}

void __cdecl MSG_WriteLong(msg_t *msg, int c)
{
    int newsize; // [esp+0h] [ebp-4h]

    if ( msg->readOnly
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 671, 0, "%s", "!msg->readOnly") )
    {
        __debugbreak();
    }
    newsize = msg->cursize + 4;
    if ( newsize > msg->maxsize )
    {
        msg->overflowed = 1;
    }
    else
    {
        *(unsigned int *)&msg->data[msg->cursize] = c;
        msg->cursize = newsize;
    }
}

void __cdecl MSG_WriteFloat(msg_t *sb, float f)
{
    union
    {
        float f;
        int l;
    } dat;

    dat.f = f;
    iassert(dat.l != -1);
    MSG_WriteLong(sb, dat.l);
}

void __cdecl MSG_WriteInt64(msg_t *msg, unsigned __int64 c)
{
    int newsize; // [esp+4h] [ebp-4h]

    if ( msg->readOnly
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 703, 0, "%s", "!msg->readOnly") )
    {
        __debugbreak();
    }
    newsize = msg->cursize + 8;
    if ( newsize > msg->maxsize )
    {
        msg->overflowed = 1;
    }
    else
    {
        *(_QWORD *)&msg->data[msg->cursize] = LittleLong64(c);
        msg->cursize = newsize;
    }
}

void __cdecl MSG_WriteString(msg_t *sb, const char *s)
{
    unsigned __int8 v2; // al
    int l; // [esp+10h] [ebp-40Ch]
    char string[1024]; // [esp+14h] [ebp-408h] BYREF
    int i; // [esp+418h] [ebp-4h]

    if ( !s && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 722, 0, "%s", "s") )
        __debugbreak();
    if ( sb->readOnly
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 723, 0, "%s", "!sb->readOnly") )
    {
        __debugbreak();
    }
    l = strlen(s);
    if ( l < 1024 )
    {
        for ( i = 0; i < l; ++i )
        {
            v2 = I_CleanChar(s[i]);
            string[i] = v2;
        }
        string[i] = 0;
        MSG_WriteData(sb, (unsigned __int8 *)string, l + 1);
    }
    else
    {
        Com_Printf(16, "MSG_WriteString: MAX_STRING_CHARS");
        MSG_WriteData(sb, (unsigned __int8 *)"", 1u);
    }
}

void __cdecl MSG_WriteBigString(msg_t *sb, char *s)
{
    unsigned __int8 v2; // al
    int v3; // [esp+10h] [ebp-400Ch]
    char dest[16384]; // [esp+14h] [ebp-4008h] BYREF
    int i; // [esp+4018h] [ebp-4h]

    if ( !s && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 747, 0, "%s", "s") )
        __debugbreak();
    if ( sb->readOnly
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 748, 0, "%s", "!sb->readOnly") )
    {
        __debugbreak();
    }
    v3 = strlen(s);
    if ( v3 < 0x4000 )
    {
        I_strncpyz(dest, s, 0x4000);
        for ( i = 0; i < v3; ++i )
        {
            v2 = I_CleanChar(dest[i]);
            dest[i] = v2;
        }
        MSG_WriteData(sb, (unsigned __int8 *)dest, v3 + 1);
    }
    else
    {
        Com_Printf(16, "MSG_WriteString: BIG_INFO_STRING");
        MSG_WriteData(sb, (unsigned __int8 *)"", 1u);
    }
}

void __cdecl MSG_WriteAngle(msg_t *sb, unsigned int bits, float f)
{
    if ( sb->readOnly
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 769, 0, "%s", "!sb->readOnly") )
    {
        __debugbreak();
    }
    MSG_WriteBits(sb, ((1 << bits) - 1) & (int)(float)((float)((float)(1 << bits) * f) / 360.0), bits);
}

void __cdecl MSG_WriteAngle16(msg_t *sb, float f)
{
    if ( sb->readOnly
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 776, 0, "%s", "!sb->readOnly") )
    {
        __debugbreak();
    }
    MSG_WriteShort(sb, (int)(float)(f * 182.04445));
}

void __cdecl MSG_WriteRangedFloatBits(msg_t *sb, float f, float begin, float end, unsigned int bits)
{
    int v5; // [esp+0h] [ebp-1Ch]
    int v6; // [esp+Ch] [ebp-10h]
    float fa; // [esp+28h] [ebp+Ch]

    fa = (float)((float)((1 << bits) - 1) / (float)(end - begin)) * (float)(f - begin);
    if ( (int)(float)(fa + 0.5) < (int)(float)((1 << bits) - 1) )
        v6 = (int)(float)(fa + 0.5);
    else
        v6 = (int)(float)((1 << bits) - 1);
    if ( v6 > 0 )
        v5 = v6;
    else
        v5 = 0;
    MSG_WriteBits(sb, v5, bits);
}

int __cdecl MSG_ReadByte(msg_t *msg)
{
    int c; // [esp+0h] [ebp-4h]

    if ( msg->readcount >= msg->splitSize + msg->cursize )
    {
        msg->overflowed = 1;
        return -1;
    }
    else
    {
        c = MSG_GetByte(msg, msg->readcount);

        iassert(c == (unsigned char)c);
        //if ( c != (unsigned __int8)c
        //    && !Assert_MyHandler(
        //                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
        //                811,
        //                0,
        //                "c == static_cast< byte >( c )\n\t%i, %i",
        //                c,
        //                (unsigned __int8)c) )
        //{
        //    __debugbreak();
        //}
        ++msg->readcount;
        return c;
    }
}

int __cdecl MSG_ReadShort(msg_t *msg)
{
    int where; // [esp+0h] [ebp-14h]
    int i; // [esp+4h] [ebp-10h]
    int c; // [esp+8h] [ebp-Ch]
    int newcount; // [esp+Ch] [ebp-8h]
    //__int16 read[2]; // [esp+10h] [ebp-4h]
    short read;

    newcount = msg->readcount + 2;
    if ( newcount > msg->splitSize + msg->cursize )
    {
        msg->overflowed = 1;
        return -1;
    }
    else
    {
        where = msg->readcount;

        for (i = 0; i < 2; ++i)
        {
            //*((_BYTE *)read + i) = MSG_GetByte(msg, where++);
            *(((_BYTE *)&read) + i) = MSG_GetByte(msg, where++);
        }

        //c = read[0];

        msg->readcount = newcount;
        return read;
    }
}

int __cdecl MSG_ReadLong(msg_t *msg)
{
    int where; // [esp+0h] [ebp-10h]
    int i; // [esp+4h] [ebp-Ch]
    int c; // [esp+8h] [ebp-8h]
    int newcount; // [esp+Ch] [ebp-4h]

    newcount = msg->readcount + 4;
    if ( newcount > msg->splitSize + msg->cursize )
    {
        msg->overflowed = 1;
        return -1;
    }
    else
    {
        where = msg->readcount;
        for ( i = 0; i < 4; ++i )
            *((_BYTE *)&c + i) = MSG_GetByte(msg, where++);
        msg->readcount = newcount;
        return c;
    }
}

unsigned __int64 __cdecl MSG_ReadInt64(msg_t *msg)
{
    int where; // [esp+0h] [ebp-18h]
    int i; // [esp+4h] [ebp-14h]
    unsigned __int64 c; // [esp+8h] [ebp-10h]
    int newcount; // [esp+14h] [ebp-4h]

    newcount = msg->readcount + 8;
    if ( newcount > msg->splitSize + msg->cursize )
    {
        msg->overflowed = 1;
        return 0;
    }
    else
    {
        where = msg->readcount;
        for ( i = 0; i < 8; ++i )
            *((_BYTE *)&c + i) = MSG_GetByte(msg, where++);
        c = LittleLong64(c);
        msg->readcount = newcount;
        return c;
    }
}

float  __cdecl MSG_ReadFloat(msg_t *msg)
{
    //float result; // fp1
    //float f; // [sp+50h] [-20h]
    //
    ////f = COERCE_FLOAT(MSG_ReadBits(msg, 32));
    //*(int *)&f = MSG_ReadBits(msg, 32);
    //
    //if (f == NAN)
    //{
    //    msg->overflowed = 1;
    //    result = -1.0;
    //}
    //else
    //{
    //    result = f;
    //}
    //
    //return result;

    union
    {
        float f;
        int l;
    } dat;

    dat.l = MSG_ReadLong(msg);
    if (dat.l != -1)
    {
        return dat.f;
    }

    msg->overflowed = 1;
    return -1.0f;
}

char *__cdecl MSG_ReadString(msg_t *msg, char *string, unsigned int maxChars)
{
    int c; // [esp+0h] [ebp-8h]
    unsigned int l; // [esp+4h] [ebp-4h]

    if ( !string && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 894, 0, "%s", "string") )
        __debugbreak();
    for ( l = 0; ; ++l )
    {
        c = MSG_ReadByte(msg);
        if ( c == -1 )
            c = 0;
        if ( l < maxChars )
            string[l] = I_CleanChar(c);
        if ( !c )
            break;
    }
    string[maxChars - 1] = 0;
    return string;
}

char string[16384];
char *__cdecl MSG_ReadBigString(msg_t *msg)
{
    int c; // [esp+0h] [ebp-8h]
    unsigned int l; // [esp+4h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 921, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    for ( l = 0; ; ++l )
    {
        c = MSG_ReadByte(msg);
        if ( c == 37 )
        {
            c = 46;
        }
        else if ( c == -1 )
        {
            c = 0;
        }
        if ( l < 0x4000 )
            string[l] = I_CleanChar(c);
        if ( !c )
            break;
    }
    string[16383] = 0;
    return string;
}

char *__cdecl MSG_ReadStringLine(msg_t *msg, char *string, unsigned int maxChars)
{
    int c; // [esp+0h] [ebp-8h]
    unsigned int l; // [esp+4h] [ebp-4h]

    for ( l = 0; ; ++l )
    {
        c = MSG_ReadByte(msg);
        if ( c == 37 )
        {
            c = 46;
        }
        else if ( c == 10 || c == -1 )
        {
            c = 0;
        }
        if ( l < maxChars )
            string[l] = I_CleanChar(c);
        if ( !c )
            break;
    }
    string[maxChars - 1] = 0;
    return string;
}

double __cdecl MSG_ReadAngle(msg_t *msg, unsigned int bits)
{
    return (double)MSG_ReadBits(msg, bits) * (360.0 / (double)(1 << bits));
}

double __cdecl MSG_ReadAngle16(msg_t *msg)
{
    return (double)MSG_ReadShort(msg) * 0.0054931641;
}

double __cdecl MSG_ReadRangedFloatBits(msg_t *sb, float begin, float end, unsigned int bits)
{
    float fa; // [esp+10h] [ebp-8h]
    float f; // [esp+10h] [ebp-8h]

    fa = (double)(unsigned int)MSG_ReadBits(sb, bits) * ((float)(end - begin) / (float)((1 << bits) - 1));
    f = fa + begin;
    if ( f < begin
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 999, 0, "%s", "f >= begin") )
    {
        __debugbreak();
    }
    if ( end < f && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 1000, 0, "%s", "f <= end") )
        __debugbreak();
    return f;
}

void __cdecl MSG_ReadData(msg_t *msg, unsigned __int8 *data, int len)
{
    int newcount; // [esp+0h] [ebp-8h]
    signed int cursize; // [esp+4h] [ebp-4h]

    newcount = len + msg->readcount;
    if ( newcount > msg->cursize )
    {
        if ( newcount > msg->splitSize + msg->cursize )
        {
            msg->overflowed = 1;
            memset(data, 0xFFu, len);
        }
        else
        {
            cursize = msg->cursize - msg->readcount;
            if ( cursize > 0 )
            {
                memcpy(data, &msg->data[msg->readcount], cursize);
                len -= cursize;
                data += cursize;
            }
            if ( len > 0 )
                memcpy(data, &msg->splitData[msg->readcount - msg->cursize], len);
            msg->readcount = newcount;
        }
    }
    else
    {
        memcpy(data, &msg->data[msg->readcount], len);
        msg->readcount = newcount;
    }
}

void __cdecl MSG_WriteDeltaKey(msg_t *msg, int key, int oldV, int newV, unsigned int bits)
{
    if ( msg->readOnly
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 1059, 0, "%s", "!msg->readOnly") )
    {
        __debugbreak();
    }
    if ( oldV == newV )
    {
        MSG_WriteBit0(msg);
    }
    else
    {
        MSG_WriteBit1(msg);
        MSG_WriteBits(msg, key ^ newV, bits);
    }
}

unsigned int __cdecl MSG_ReadDeltaKey(msg_t *msg, int key, int oldV, unsigned int bits)
{
    if ( MSG_ReadBit(msg) )
        return kbitmask[bits] & key ^ MSG_ReadBits(msg, bits);
    else
        return oldV;
}

void __cdecl MSG_WriteKey(msg_t *msg, int key, int newV, unsigned int bits)
{
    if ( msg->readOnly
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 1082, 0, "%s", "!msg->readOnly") )
    {
        __debugbreak();
    }
    MSG_WriteBits(msg, key ^ newV, bits);
}

unsigned int __cdecl MSG_ReadKey(msg_t *msg, int key, unsigned int bits)
{
    return kbitmask[bits] & key ^ MSG_ReadBits(msg, bits);
}

void __cdecl MSG_WriteDeltaKeyByte(msg_t *msg, char key, char oldV, char newV)
{
    if ( msg->readOnly
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 1095, 0, "%s", "!msg->readOnly") )
    {
        __debugbreak();
    }
    if ( oldV == newV )
    {
        MSG_WriteBit0(msg);
    }
    else
    {
        MSG_WriteBit1(msg);
        MSG_WriteByte(msg, key ^ newV);
    }
}

int __cdecl MSG_ReadDeltaKeyByte(msg_t *msg, unsigned __int8 key, int oldV)
{
    if ( MSG_ReadBit(msg) )
        return key ^ (unsigned __int8)MSG_ReadByte(msg);
    else
        return oldV;
}

void __cdecl MSG_WriteDeltaKeyShort(msg_t *msg, __int16 key, __int16 oldV, __int16 newV)
{
    if ( msg->readOnly
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 1117, 0, "%s", "!msg->readOnly") )
    {
        __debugbreak();
    }
    if ( oldV == newV )
    {
        MSG_WriteBit0(msg);
    }
    else
    {
        MSG_WriteBit1(msg);
        MSG_WriteShort(msg, key ^ newV);
    }
}

int __cdecl MSG_ReadDeltaKeyShort(msg_t *msg, __int16 key, int oldV)
{
    if ( MSG_ReadBit(msg) )
        return key ^ (unsigned __int16)MSG_ReadShort(msg);
    else
        return oldV;
}

void __cdecl MSG_SetDefaultUserCmd(playerState_s *ps, usercmd_s *cmd)
{
    int i; // [esp+4h] [ebp-4h]

    memset(cmd, 0, sizeof(usercmd_s));
    cmd->weapon = ps->weapon;
    cmd->offHandIndex = ps->offHandIndex;
    for ( i = 0; i < 2; ++i )
        cmd->angles[i] = (unsigned __int16)(int)(float)((float)(ps->viewangles[i] - ps->delta_angles[i]) * 182.04445);
    if ( (ps->otherFlags & 4) != 0 )
    {
        if ( (ps->eFlags & 8) != 0 )
        {
            cmd->button_bits.setBit(8);
        }
        else if ( (ps->eFlags & 4) != 0 )
        {
            cmd->button_bits.setBit(9);
        }
        if ( ps->leanf <= 0.0 )
        {
            if ( ps->leanf < 0.0 )
               cmd->button_bits.setBit(6);
        }
        else
        {
           cmd->button_bits.setBit(7);
        }
        if ( ps->fWeaponPosFrac != 0.0 )
           cmd->button_bits.setBit(11);
        if ( (ps->pm_flags & 0x8000) != 0 )
           cmd->button_bits.setBit(1);
    }
}

void __cdecl MSG_WriteDeltaUsercmdKey(
                msg_t *msg,
                int key,
                const usercmd_s *from,
                const usercmd_s *to,
                int forceSendAngles)
{
    bool v5; // esi
    bool v6; // al
    bool v7; // al
    bool v8; // [esp+4h] [ebp-2Ch]
    bool v9; // [esp+8h] [ebp-28h]
    int i; // [esp+18h] [ebp-18h]
    bool noButtonChange; // [esp+1Eh] [ebp-12h]
    bitarray<51> ignoreMask(0, -1); // [esp+20h] [ebp-10h] BYREF
    unsigned int delta; // [esp+28h] [ebp-8h]
    bool buttonsAllEqual; // [esp+2Eh] [ebp-2h]
    bool noAngleChange; // [esp+2Fh] [ebp-1h]
    int keyb; // [esp+3Ch] [ebp+Ch]
    int keya; // [esp+3Ch] [ebp+Ch]

    if ( msg->readOnly
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 1255, 0, "%s", "!msg->readOnly") )
    {
        __debugbreak();
    }
    delta = to->serverTime - from->serverTime;
    if ( delta >= 0x100 )
    {
        MSG_WriteBit0(msg);
        MSG_WriteLong(msg, to->serverTime);
    }
    else
    {
        MSG_WriteBit1(msg);
        MSG_WriteByte(msg, delta);
    }
    v9 = from->forwardmove == to->forwardmove && from->rightmove == to->rightmove;
    v8 = forceSendAngles && from->yawmove == to->yawmove && from->pitchmove == to->pitchmove;
    noAngleChange = v8;
    //bitarray<51>::bitarray<51>(&ignoreMask, 0, -1);
    //buttonsAllEqual = bitarray<51>::areAllBitsEqual(&from->button_bits, &to->button_bits, &ignoreMask);
    buttonsAllEqual = from->button_bits.areAllBitsEqual(&to->button_bits, &ignoreMask);
    if ( buttonsAllEqual
        && from->weapon == to->weapon
        && from->offHandIndex == to->offHandIndex
        && from->angles[2] == to->angles[2]
        && from->meleeChargeYaw == to->meleeChargeYaw
        && from->meleeChargeDist == to->meleeChargeDist )
    {
        v5 = from->button_bits.testBit(0);
        noButtonChange = v5 == to->button_bits.testBit(0);
        if ( from->angles[0] == to->angles[0] && from->angles[1] == to->angles[1] && noButtonChange && v9 && noAngleChange )
        {
            MSG_WriteKey(msg, key, 0, 1u);
        }
        else
        {
            MSG_WriteKey(msg, key, 1, 1u);
            MSG_WriteKey(msg, key, 0, 1u);
            keyb = to->serverTime ^ key;
            v6 = to->button_bits.testBit(0);
            MSG_WriteKey(msg, keyb, v6, 1u);
            MSG_WriteDeltaKeyShort(msg, keyb, from->angles[0], to->angles[0]);
            MSG_WriteDeltaKeyShort(msg, keyb, from->angles[1], to->angles[1]);
            MSG_WriteDeltaKeyByte(msg, keyb, from->forwardmove, to->forwardmove);
            MSG_WriteDeltaKeyByte(msg, keyb, from->rightmove, to->rightmove);
            MSG_WriteDeltaKeyByte(msg, keyb, from->pitchmove, to->pitchmove);
            MSG_WriteDeltaKeyByte(msg, keyb, from->yawmove, to->yawmove);
        }
    }
    else
    {
        MSG_WriteKey(msg, key, 1, 1u);
        MSG_WriteKey(msg, key, 1, 1u);
        v7 = to->button_bits.testBit(0);
        MSG_WriteKey(msg, key, v7, 1u);
        MSG_WriteDeltaKeyShort(msg, key, from->angles[0], to->angles[0]);
        MSG_WriteDeltaKeyShort(msg, key, from->angles[1], to->angles[1]);
        MSG_WriteDeltaKeyByte(msg, key, from->forwardmove, to->forwardmove);
        MSG_WriteDeltaKeyByte(msg, key, from->rightmove, to->rightmove);
        MSG_WriteDeltaKeyByte(msg, key, from->pitchmove, to->pitchmove);
        MSG_WriteDeltaKeyByte(msg, key, from->yawmove, to->yawmove);
        keya = to->serverTime ^ key;
        MSG_WriteDeltaKeyShort(msg, keya, from->angles[2], to->angles[2]);
        for ( i = 0; i < 2; ++i )
            MSG_WriteDeltaKey(msg, keya, from->button_bits.array[i], to->button_bits.array[i], 0x20u);
        MSG_WriteDeltaKey(msg, keya, from->weapon, to->weapon, 0xBu);
        MSG_WriteDeltaKey(msg, keya, from->offHandIndex, to->offHandIndex, 0xBu);
        MSG_WriteDeltaKey(msg, keya, from->lastWeaponAltModeSwitch, to->lastWeaponAltModeSwitch, 0xBu);
        if ( to->button_bits.testBit(16) )
        {
            MSG_WriteDeltaKeyByte(msg, keya, from->selectedLocation[0], to->selectedLocation[0]);
            MSG_WriteDeltaKeyByte(msg, keya, from->selectedLocation[1], to->selectedLocation[1]);
            MSG_WriteDeltaKeyByte(msg, keya, from->selectedYaw, to->selectedYaw);
        }
        if ( to->button_bits.testBit(2) )
        {
            MSG_WriteDeltaKeyShort(
                msg,
                keya,
                (int)(float)(from->meleeChargeYaw * 182.04445),
                (int)(float)(to->meleeChargeYaw * 182.04445));
            MSG_WriteDeltaKey(msg, keya, from->meleeChargeDist, to->meleeChargeDist, 8u);
        }
    }
}

void __cdecl MSG_ReadDeltaUsercmdKey(msg_t *msg, int key, const usercmd_s *from, usercmd_s *to)
{
    int i; // [esp+8h] [ebp-Ch]
    unsigned int set_attack; // [esp+10h] [ebp-4h]
    int keya; // [esp+20h] [ebp+Ch]
    int keyb; // [esp+20h] [ebp+Ch]

    memcpy(to, from, sizeof(usercmd_s));
    if ( MSG_ReadBit(msg) )
        to->serverTime = from->serverTime + MSG_ReadByte(msg);
    else
        to->serverTime = MSG_ReadLong(msg);
    if ( MSG_ReadKey(msg, key, 1u) )
    {
        to->button_bits.resetBit(0);
        if ( MSG_ReadKey(msg, key, 1u) )
        {
            set_attack = MSG_ReadKey(msg, key, 1u);
            to->angles[0] = (unsigned __int16)MSG_ReadDeltaKeyShort(msg, key, from->angles[0]);
            to->angles[1] = (unsigned __int16)MSG_ReadDeltaKeyShort(msg, key, from->angles[1]);
            to->forwardmove = MSG_ReadDeltaKeyByte(msg, key, from->forwardmove);
            to->rightmove = MSG_ReadDeltaKeyByte(msg, key, from->rightmove);
            to->pitchmove = MSG_ReadDeltaKeyByte(msg, key, from->pitchmove);
            to->yawmove = MSG_ReadDeltaKeyByte(msg, key, from->yawmove);
            keyb = to->serverTime ^ key;
            to->angles[2] = (unsigned __int16)MSG_ReadDeltaKeyShort(msg, keyb, from->angles[2]);
            for ( i = 0; i < 2; ++i )
                to->button_bits.array[i] = MSG_ReadDeltaKey(msg, keyb, from->button_bits.array[i], 0x20u);
            if ( set_attack )
               to->button_bits.setBit(0);
            to->weapon = MSG_ReadDeltaKey(msg, keyb, from->weapon, 0xBu);
            to->offHandIndex = MSG_ReadDeltaKey(msg, keyb, from->offHandIndex, 0xBu);
            to->lastWeaponAltModeSwitch = MSG_ReadDeltaKey(msg, keyb, from->lastWeaponAltModeSwitch, 0xBu);
            if ( to->button_bits.testBit(16) )
            {
                to->selectedLocation[0] = MSG_ReadDeltaKeyByte(msg, keyb, from->selectedLocation[0]);
                to->selectedLocation[1] = MSG_ReadDeltaKeyByte(msg, keyb, from->selectedLocation[1]);
                to->selectedYaw = MSG_ReadDeltaKeyByte(msg, keyb, from->selectedYaw);
            }
            if ( to->button_bits.testBit(2) )
            {
                to->meleeChargeYaw = (float)MSG_ReadDeltaKeyShort(
                                                                            msg,
                                                                            keyb,
                                                                            (unsigned __int16)(int)(float)(from->meleeChargeYaw * 182.04445))
                                                     * 0.0054931641;
                to->meleeChargeDist = MSG_ReadDeltaKey(msg, keyb, from->meleeChargeDist, 8u);
            }
        }
        else
        {
            keya = to->serverTime ^ key;
            if ( MSG_ReadKey(msg, keya, 1u) )
               to->button_bits.setBit(0);
            to->angles[0] = (unsigned __int16)MSG_ReadDeltaKeyShort(msg, keya, from->angles[0]);
            to->angles[1] = (unsigned __int16)MSG_ReadDeltaKeyShort(msg, keya, from->angles[1]);
            to->forwardmove = MSG_ReadDeltaKeyByte(msg, keya, from->forwardmove);
            to->rightmove = MSG_ReadDeltaKeyByte(msg, keya, from->rightmove);
            to->pitchmove = MSG_ReadDeltaKeyByte(msg, keya, from->pitchmove);
            to->yawmove = MSG_ReadDeltaKeyByte(msg, keya, from->yawmove);
        }
    }
}

void __cdecl MSG_ClearLastReferencedEntity(msg_t *msg)
{
    msg->lastEntityRef = -1;
}

double __cdecl MSG_ReadDemoRoundedFloat(
                msg_t *msg,
                int bits,
                float oldValue,
                unsigned int diffBits,
                unsigned int fullBits)
{
    int fullBias; // [esp+4h] [ebp-20h]
    int roundedCenter; // [esp+Ch] [ebp-18h]
    int index; // [esp+10h] [ebp-14h]

    fullBias = 1 << (fullBits - 1);
    if ( MSG_ReadBit(msg) )
    {
        if ( bits == -103 || bits == -102 || bits == -101 )
        {
            if ( bits == -103 )
            {
                index = 0;
            }
            else if ( bits == -102 )
            {
                index = 1;
            }
            else
            {
                index = 2;
            }
            roundedCenter = (int)(float)((*MSG_GetMapCenter())[index] + 0.5);
            return (float)(roundedCenter
                                     + (((int)oldValue + fullBias - roundedCenter) ^ MSG_ReadBits(msg, fullBits))
                                     - fullBias);
        }
        else
        {
            return (float)(((fullBias + (int)oldValue) ^ MSG_ReadBits(msg, fullBits)) - fullBias);
        }
    }
    else
    {
        return (float)((float)(MSG_ReadBits(msg, diffBits) - (1 << (diffBits - 1))) + oldValue);
    }
}

double __cdecl MSG_ReadRangedFloat(
                msg_t *msg,
                float oldValue,
                float begin,
                float end,
                int precision,
                bool oftenZeroOrOne)
{
    float v7; // [esp+18h] [ebp-3Ch]
    float v8; // [esp+2Ch] [ebp-28h]
    float v9; // [esp+30h] [ebp-24h]
    unsigned int rangeSize; // [esp+38h] [ebp-1Ch]
    int rangeBits; // [esp+40h] [ebp-14h]
    float oldVal; // [esp+44h] [ebp-10h]
    float range; // [esp+4Ch] [ebp-8h]
    int f; // [esp+50h] [ebp-4h]

    if ( end < begin
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                    1748,
                    0,
                    "%s\n\t%s",
                    "end >= begin",
                    "MSG_ReadRangedFloat, end of range is less then the begining.") )
    {
        __debugbreak();
    }
    if ( precision <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                    1749,
                    0,
                    "%s\n\t%s",
                    "precision > 0",
                    "MSG_ReadRangedFloat, precision must be positive.") )
    {
        __debugbreak();
    }
    range = (float)(end - begin) * (float)precision;
    if ( range <= 4294967300.0 )
    {
        if ( oftenZeroOrOne && MSG_ReadBit(msg) )
        {
            if ( MSG_ReadBit(msg) )
                v8 = 1.0f;
            else
                v8 = 0.0f;
            *(float *)&f = v8;
        }
        else
        {
            rangeSize = (__int64)range;
            rangeBits = 0;
            if ( rangeSize )
                rangeBits = GetMinBitCountForNum(rangeSize);
            if ( (float)(oldValue - end) < 0.0 )
                v9 = oldValue;
            else
                v9 = end;
            if ( (float)(begin - oldValue) < 0.0 )
                v7 = v9;
            else
                v7 = begin;
            oldVal = (float)((float)(v7 - begin) * (float)precision) + 0.5;
            if ( oldVal > range )
                oldVal = (float)(end - begin) * (float)precision;
            *(float *)&f = (double)(MSG_ReadBits(msg, rangeBits) ^ (unsigned int)(__int64)oldVal) / (double)precision + begin;
        }
    }
    else
    {
        Com_PrintWarning(
            15,
            "The numerical space defined by range/precision combination is too large. No compression performed.");
        f = LODWORD(oldValue) ^ MSG_ReadLong(msg);
    }
    return *(float *)&f;
}

int __cdecl MSG_ReadEntityIndex(msg_t *msg, unsigned int indexBits)
{
    if ( MSG_ReadBit(msg) )
    {
        if ( msg_printEntityNums->current.enabled )
            Com_Printf(16, "Entity num: 1 bit (inc)\n");
        ++msg->lastEntityRef;
    }
    else if ( indexBits != 10 || MSG_ReadBit(msg) )
    {
        if ( msg_printEntityNums->current.enabled )
            Com_Printf(16, "Entity num: %i bits (full)\n", indexBits + 2);
        msg->lastEntityRef = MSG_ReadBits(msg, indexBits);
    }
    else
    {
        if ( msg_printEntityNums->current.enabled )
            Com_Printf(16, "Entity num: %i bits (delta)\n", 6);
        msg->lastEntityRef += MSG_ReadBits(msg, 4u);
    }
    if ( msg_printEntityNums->current.enabled )
        Com_Printf(16, "Read entity num %i\n", msg->lastEntityRef);
    if ( msg->lastEntityRef < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                    1828,
                    0,
                    "%s\n\t(msg->lastEntityRef) = %i",
                    "(msg->lastEntityRef >= 0)",
                    msg->lastEntityRef) )
    {
        __debugbreak();
    }
    return msg->lastEntityRef;
}

void __cdecl MSG_ReadDeltaField(
                msg_t *msg,
                int time,
                char *from,
                char *to,
                const NetField *field,
                int print,
                bool noXor)
{
    int Bit; // eax
    int Byte; // eax
    int Long; // eax
    int v10; // eax
    int v11; // eax
    int v12; // eax
    int v13; // eax
    int v14; // eax
    double RangedFloat; // st7
    double v16; // st7
    double v17; // st7
    double v18; // st7
    double v19; // st7
    int EFlags; // eax
    int DeltaTime; // eax
    unsigned int MinBitCountForNum; // eax
    int Bits; // esi
    int v24; // eax
    double OriginFloat; // st7
    double OriginZFloat; // st7
    double v27; // st7
    double v28; // st7
    double DemoRoundedFloat; // st7
    double v30; // st7
    double Angle; // st7
    double DeltaAngle; // st7
    double Angle16; // st7
    int v34; // eax
    int v35; // [esp+18h] [ebp-D4h]
    int v36; // [esp+1Ch] [ebp-D0h]
    int v37; // [esp+20h] [ebp-CCh]
    int v38; // [esp+24h] [ebp-C8h]
    int v39; // [esp+28h] [ebp-C4h]
    int v40; // [esp+2Ch] [ebp-C0h]
    int v41; // [esp+30h] [ebp-BCh]
    int v42; // [esp+34h] [ebp-B8h]
    int v43; // [esp+38h] [ebp-B4h]
    int size; // [esp+3Ch] [ebp-B0h]
    float v45; // [esp+40h] [ebp-ACh]
    float v46; // [esp+44h] [ebp-A8h]
    float v47; // [esp+48h] [ebp-A4h]
    int v48; // [esp+5Ch] [ebp-90h]
    int v49; // [esp+68h] [ebp-84h]
    int v50; // [esp+70h] [ebp-7Ch]
    int ValueNoXor; // [esp+78h] [ebp-74h]
    int v52; // [esp+80h] [ebp-6Ch]
    int v53; // [esp+88h] [ebp-64h]
    int DeltaEventParamField; // [esp+90h] [ebp-5Ch]
    int DeltaEventField; // [esp+98h] [ebp-54h]
    int DeltaGroundEntity; // [esp+A0h] [ebp-4Ch]
    float v57; // [esp+A8h] [ebp-44h]
    float v58; // [esp+B0h] [ebp-3Ch]
    float v59; // [esp+B8h] [ebp-34h]
    const hudelem_color_t *fromColor; // [esp+C8h] [ebp-24h]
    hudelem_color_t *toColora; // [esp+CCh] [ebp-20h]
    hudelem_color_t *toColor; // [esp+CCh] [ebp-20h]
    int delta; // [esp+D0h] [ebp-1Ch]
    int zeroVal; // [esp+D8h] [ebp-14h] BYREF
    int trunc; // [esp+DCh] [ebp-10h]
    int *toF; // [esp+E0h] [ebp-Ch]
    const int *fromF; // [esp+E4h] [ebp-8h]
    int value; // [esp+E8h] [ebp-4h]

    zeroVal = 0;
    if ( noXor )
        fromF = &zeroVal;
    else
        fromF = (const int *)&from[field->offset];
    toF = (int *)&to[field->offset];
    if ( msg->overflowed
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 1895, 0, "%s", "!msg->overflowed") )
    {
        __debugbreak();
    }
    if ( msg->overflowed
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 1922, 0, "%s", "!msg->overflowed") )
    {
        __debugbreak();
    }
    switch ( field->bits )
    {
        case 0:
            if ( MSG_ReadBit(msg) )
            {
                if ( MSG_ReadBit(msg) )
                {
                    Long = MSG_ReadLong(msg);
                    *toF = Long;
                    *toF ^= *fromF;
                    if ( print )
                        Com_Printf(16, "%s:%f ", field->name, *(float *)toF);
                }
                else
                {
                    trunc = MSG_ReadBits(msg, 5u);
                    Byte = MSG_ReadByte(msg);
                    trunc += 32 * Byte;
                    trunc ^= (int)*(float *)fromF + 4096;
                    trunc -= 4096;
                    *(float *)toF = (float)trunc;
                    if ( print )
                        Com_Printf(16, "%s:%i ", field->name, trunc);
                }
            }
            else
            {
                Bit = MSG_ReadBit(msg);
                *toF = Bit << 31;
                if ( *(float *)toF != 0.0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                                1929,
                                0,
                                "%s",
                                "*reinterpret_cast< float * >( toF ) == 0.0f") )
                {
                    __debugbreak();
                }
            }
            return;
        case 0xFFFFFFC1:
        case 0xFFFFFFC3:
LABEL_25:
            v10 = MSG_ReadLong(msg);
            *toF = v10;
            return;
        case 0xFFFFFFA7:
            if ( MSG_ReadBit(msg) )
            {
                v12 = MSG_ReadLong(msg);
                *toF = v12;
                *toF ^= *fromF;
                if ( print )
                    Com_Printf(16, "%s:%f ", field->name, *(float *)toF);
            }
            else
            {
                trunc = MSG_ReadBits(msg, 5u);
                v11 = MSG_ReadByte(msg);
                trunc += 32 * v11;
                trunc ^= (int)*(float *)fromF + 4096;
                trunc -= 4096;
                *(float *)toF = (float)trunc;
                if ( print )
                    Com_Printf(16, "%s:%i ", field->name, trunc);
            }
            break;
        case 0xFFFFFFA8:
            v13 = MSG_ReadLong(msg);
            *toF = v13;
            *toF ^= *fromF;
            if ( print )
                Com_Printf(16, "%s:%f ", field->name, *(float *)toF);
            break;
        case 0xFFFFFF9D:
            if ( MSG_ReadBit(msg) )
            {
                if ( MSG_ReadBit(msg) )
                {
                    value = MSG_ReadLong(msg);
                    value ^= *fromF;
                    *toF = value;
                    if ( print )
                        Com_Printf(16, "%s:%f ", field->name, *(float *)toF);
                }
                else
                {
                    trunc = MSG_ReadBits(msg, 4u);
                    v14 = MSG_ReadByte(msg);
                    trunc += 16 * v14;
                    trunc ^= (int)*(float *)fromF + 2048;
                    trunc -= 2048;
                    *(float *)toF = (float)trunc;
                    if ( print )
                        Com_Printf(16, "%s:%i ", field->name, trunc);
                }
            }
            else
            {
                *toF = 0;
                if ( *(float *)toF != 0.0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                                2008,
                                0,
                                "%s",
                                "*reinterpret_cast< float * >( toF ) == 0.0f") )
                {
                    __debugbreak();
                }
            }
            if ( (unsigned int)(__int64)(*(float *)toF + 2048.0) >= 0x1000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                            2040,
                            0,
                            "*(float *)toF + HUDELEM_COORD_BIAS doesn't index 1 << HUDELEM_COORD_BITS\n\t%i not in [0, %i)",
                            (int)(float)(*(float *)toF + 2048.0),
                            4096) )
            {
                __debugbreak();
            }
            break;
        case 0xFFFFFFAF:
            RangedFloat = MSG_ReadRangedFloat(msg, *(float *)fromF, 0.0, 1.0, 100, 0);
            *(float *)toF = RangedFloat;
            return;
        case 0xFFFFFFB0:
            v16 = MSG_ReadRangedFloat(msg, *(float *)fromF, 0.0, 1.0, 1000, 0);
            *(float *)toF = v16;
            return;
        case 0xFFFFFFB3:
            v17 = MSG_ReadRangedFloat(msg, *(float *)fromF, -1.0, 1.0, 100, 0);
            *(float *)toF = v17;
            return;
        case 0xFFFFFFB4:
            v59 = MSG_ReadRangedFloat(msg, (float)*(__int16 *)fromF * 0.000030517578, -1.0, 1.0, 10, 0);
            v47 = floor(v59 * 32768.0 + 0.5);
            *(_WORD *)toF = (int)v47;
            return;
        case 0xFFFFFFB5:
            v58 = MSG_ReadRangedFloat(msg, (float)*(__int16 *)fromF * 0.0054931641, -90.0, 90.0, 10, 0);
            v46 = floor(v58 * 182.04445 + 0.5);
            *(_WORD *)toF = (int)v46;
            return;
        case 0xFFFFFFB6:
            v57 = MSG_ReadRangedFloat(msg, (float)*(__int16 *)fromF * 0.0054931641, -180.0, 180.0, 10, 0);
            v45 = floor(v57 * 182.04445 + 0.5);
            *(_WORD *)toF = (int)v45;
            return;
        case 0xFFFFFFB7:
            v18 = MSG_ReadRangedFloat(msg, *(float *)fromF, -12.0, 12.0, 10, 0);
            *(float *)toF = v18;
            return;
        case 0xFFFFFFB8:
            v19 = MSG_ReadRangedFloat(msg, *(float *)fromF, -6000.0, 6000.0, 10, 0);
            *(float *)toF = v19;
            return;
        case 0xFFFFFF9E:
            EFlags = MSG_ReadEFlags(msg, *fromF);
            *toF = EFlags;
            return;
        case 0xFFFFFF9F:
        case 0xFFFFFFB9:
        case 0xFFFFFFBA:
        case 0xFFFFFFBC:
            DeltaTime = MSG_ReadDeltaTime(msg, time);
            *toF = DeltaTime;
            return;
        case 0xFFFFFFBB:
        case 0xFFFFFFBD:
            goto LABEL_25;
        case 0xFFFFFFA0:
            DeltaGroundEntity = MSG_ReadDeltaGroundEntity(msg);
            size = field->size;
            switch ( size )
            {
                case 1:
                    *(_BYTE *)toF = DeltaGroundEntity;
                    break;
                case 2:
                    *(_WORD *)toF = DeltaGroundEntity;
                    break;
                case 4:
                    *toF = DeltaGroundEntity;
                    break;
            }
            break;
        case 0xFFFFFFA2:
            DeltaEventField = MSG_ReadDeltaEventField(msg);
            v43 = field->size;
            switch ( v43 )
            {
                case 1:
                    *(_BYTE *)toF = DeltaEventField;
                    break;
                case 2:
                    *(_WORD *)toF = DeltaEventField;
                    break;
                case 4:
                    *toF = DeltaEventField;
                    break;
            }
            break;
        case 0xFFFFFFA3:
            DeltaEventParamField = MSG_ReadDeltaEventParamField(msg);
            v42 = field->size;
            switch ( v42 )
            {
                case 1:
                    *(_BYTE *)toF = DeltaEventParamField;
                    break;
                case 2:
                    *(_WORD *)toF = DeltaEventParamField;
                    break;
                case 4:
                    *toF = DeltaEventParamField;
                    break;
            }
            break;
        case 0xFFFFFFC5:
            if ( MSG_ReadBit(msg) == 1 )
            {
                MinBitCountForNum = GetMinBitCountForNum(0x10u);
                Bits = MSG_ReadBits(msg, MinBitCountForNum);
                v53 = MSG_GetField(fromF, field->size) + Bits + 1;
                v41 = field->size;
                switch ( v41 )
                {
                    case 1:
                        *(_BYTE *)toF = v53;
                        break;
                    case 2:
                        *(_WORD *)toF = v53;
                        break;
                    case 4:
                        *toF = v53;
                        break;
                }
            }
            else
            {
                v52 = MSG_ReadValue(msg, fromF, toF, 8, field->size);
                v40 = field->size;
                switch ( v40 )
                {
                    case 1:
                        *(_BYTE *)toF = v52;
                        break;
                    case 2:
                        *(_WORD *)toF = v52;
                        break;
                    case 4:
                        *toF = v52;
                        break;
                }
            }
            break;
        case 0xFFFFFFA1:
            v24 = MSG_ReadBits(msg, 7u);
            *toF = 100 * v24;
            break;
        case 0xFFFFFFC6:
            ValueNoXor = MSG_ReadValueNoXor(msg, 8);
            v39 = field->size;
            switch ( v39 )
            {
                case 1:
                    *(_BYTE *)toF = ValueNoXor;
                    break;
                case 2:
                    *(_WORD *)toF = ValueNoXor;
                    break;
                case 4:
                    *toF = ValueNoXor;
                    break;
            }
            break;
        case 0xFFFFFFA4:
        case 0xFFFFFFA5:
        case 0xFFFFFFBE:
        case 0xFFFFFFBF:
            OriginFloat = MSG_ReadOriginFloat(field->bits, msg, *(float *)fromF);
            *(float *)toF = OriginFloat;
            if ( print )
                Com_Printf(16, "%s:%f ", field->name, *(float *)toF);
            break;
        case 0xFFFFFFA6:
        case 0xFFFFFFC0:
            OriginZFloat = MSG_ReadOriginZFloat(msg, *(float *)fromF);
            *(float *)toF = OriginZFloat;
            if ( print )
                Com_Printf(16, "%s:%f ", field->name, *(float *)toF);
            break;
        case 0xFFFFFF96:
            v27 = MSG_ReadRangedFloat(msg, *(float *)fromF, 0.0, 255.0, 100, 0);
            *(float *)toF = v27;
            break;
        case 0xFFFFFF95:
            v28 = MSG_ReadRangedFloat(msg, *(float *)fromF, 0.0, 60.0, 100, 0);
            *(float *)toF = v28;
            break;
        case 0xFFFFFF99:
        case 0xFFFFFF9A:
        case 0xFFFFFF9B:
            DemoRoundedFloat = MSG_ReadDemoRoundedFloat(msg, field->bits, *(float *)fromF, 5u, 0x10u);
            *(float *)toF = DemoRoundedFloat;
            if ( print )
                Com_Printf(16, "%s:%f ", field->name, *(float *)toF);
            break;
        case 0xFFFFFF98:
            v30 = MSG_ReadDemoRoundedFloat(msg, field->bits, *(float *)fromF, 7u, 0xCu);
            *(float *)toF = v30;
            if ( print )
                Com_Printf(16, "%s:%f ", field->name, *(float *)toF);
            break;
        case 0xFFFFFF97:
            Angle = MSG_ReadAngle(msg, 0xAu);
            *(float *)toF = Angle;
            break;
        case 0xFFFFFF9C:
        case 0xFFFFFFC2:
            DeltaAngle = MSG_ReadDeltaAngle(msg, *(float *)fromF);
            *(float *)toF = DeltaAngle;
            break;
        case 0xFFFFFFA9:
            Angle16 = MSG_ReadAngle16(msg);
            *(float *)toF = Angle16;
            break;
        case 0xFFFFFFC4:
            if ( MSG_ReadBit(msg) == 1 )
            {
                delta = MSG_ReadBits(msg, 5u);
                v50 = MSG_GetField(fromF, field->size) + delta - 16;
                v38 = field->size;
                switch ( v38 )
                {
                    case 1:
                        *(_BYTE *)toF = v50;
                        break;
                    case 2:
                        *(_WORD *)toF = v50;
                        break;
                    case 4:
                        *toF = v50;
                        break;
                }
            }
            else
            {
                v49 = MSG_ReadValue(msg, fromF, toF, -8, field->size);
                v37 = field->size;
                switch ( v37 )
                {
                    case 1:
                        *(_BYTE *)toF = v49;
                        break;
                    case 2:
                        *(_WORD *)toF = v49;
                        break;
                    case 4:
                        *toF = v49;
                        break;
                }
            }
            break;
        case 0xFFFFFFAA:
            v34 = MSG_ReadBits(msg, 6u);
            *(float *)toF = (float)((float)((float)v34 * 1.0) / 10.0) + 1.0;
            break;
        case 0xFFFFFFAB:
            if ( MSG_ReadBit(msg) )
            {
                fromColor = (const hudelem_color_t *)fromF;
                toColora = (hudelem_color_t *)toF;
                *toF = *fromF;
                toColora->a = fromColor->a != 0 ? 0 : -1;
            }
            else
            {
                toColor = (hudelem_color_t *)toF;
                if ( !MSG_ReadBit(msg) )
                {
                    toColor->r = MSG_ReadByte(msg);
                    toColor->g = MSG_ReadByte(msg);
                    toColor->b = MSG_ReadByte(msg);
                }
                toColor->a = 8 * MSG_ReadBits(msg, 5u);
                toColor->a |= ((int)toColor->a >> 5) & 7;
            }
            break;
        default:
            if ( MSG_ReadBit(msg) )
            {
                v48 = MSG_ReadValue(msg, fromF, toF, field->bits, field->size);
                v35 = field->size;
                switch ( v35 )
                {
                    case 1:
                        *(_BYTE *)toF = v48;
                        break;
                    case 2:
                        *(_WORD *)toF = v48;
                        break;
                    case 4:
                        *toF = v48;
                        break;
                }
            }
            else
            {
                v36 = field->size;
                switch ( v36 )
                {
                    case 1:
                        *(_BYTE *)toF = 0;
                        break;
                    case 2:
                        *(_WORD *)toF = 0;
                        break;
                    case 4:
                        *toF = 0;
                        break;
                }
            }
            break;
    }
}

int __cdecl MSG_ReadDeltaTime(msg_t *msg, int timeBase)
{
    if ( MSG_ReadBit(msg) )
        return MSG_ReadLong(msg);
    else
        return timeBase - MSG_ReadBits(msg, 8u);
}

int __cdecl MSG_ReadDeltaGroundEntity(msg_t *msg)
{
    int j; // [esp+4h] [ebp-10h]
    int value; // [esp+10h] [ebp-4h]

    if ( MSG_ReadBit(msg) == 1 )
        return 1022;
    if ( MSG_ReadBit(msg) == 1 )
        return 0;
    value = MSG_ReadBits(msg, 2u);
    for ( j = 2; j < 10; j += 8 )
        value |= MSG_ReadByte(msg) << j;
    return value;
}

int __cdecl MSG_ReadDeltaEventField(msg_t *msg)
{
    return MSG_ReadByte(msg);
}

int __cdecl MSG_ReadDeltaEventParamField(msg_t *msg)
{
    return MSG_ReadBits(msg, 0xBu);
}

int __cdecl MSG_ReadEFlags(msg_t *msg, int oldFlags)
{
    unsigned int bitChanged; // [esp+0h] [ebp-8h]

    if ( msg->overflowed
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 1571, 0, "%s", "!msg->overflowed") )
    {
        __debugbreak();
    }
    if ( MSG_ReadBit(msg) == 1 )
        return MSG_ReadBits(msg, 0x18u);
    bitChanged = MSG_ReadBits(msg, 5u);
    if ( bitChanged >= 0x18
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                    1582,
                    0,
                    "%s\n\t(bitChanged) = %i",
                    "(bitChanged >= 0 && bitChanged < 24)",
                    bitChanged) )
    {
        __debugbreak();
    }
    return oldFlags ^ (1 << bitChanged);
}

double __cdecl MSG_ReadOriginFloat(int bits, msg_t *msg, float oldValue)
{
    int roundedCenter; // [esp+24h] [ebp-14h]
    int index; // [esp+28h] [ebp-10h]

    if ( MSG_ReadBit(msg) )
    {
        if ( bits == -92 || bits == -66 )
        {
            index = 0;
        }
        else
        {
            if ( bits != -91
                && bits != -65
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                            1615,
                            0,
                            "%s",
                            "(bits == MSG_FIELD_ORIGINY) || (bits == MSG_FIELD_ES_ORIGINY)") )
            {
                __debugbreak();
            }
            index = 1;
        }
        roundedCenter = (int)((*MSG_GetMapCenter())[index] + 9.313225746154785e-10);
        return (float)(roundedCenter
                                 + (((int)(oldValue + 9.313225746154785e-10) + 0x8000 - roundedCenter) ^ MSG_ReadBits(msg, 0x10u))
                                 - 0x8000);
    }
    else
    {
        return (float)((float)(MSG_ReadBits(msg, 7u) - 64) + oldValue);
    }
}

double __cdecl MSG_ReadOriginZFloat(msg_t *msg, float oldValue)
{
    int roundedCenter; // [esp+24h] [ebp-10h]

    if ( MSG_ReadBit(msg) )
    {
        roundedCenter = (int)((*MSG_GetMapCenter())[2] + 9.313225746154785e-10);
        return (float)(roundedCenter
                                 + (((int)(oldValue + 9.313225746154785e-10) + 0x10000 - roundedCenter) ^ MSG_ReadBits(msg, 0x11u))
                                 - 0x10000);
    }
    else
    {
        return (float)((float)(MSG_ReadBits(msg, 7u) - 64) + oldValue);
    }
}

int __cdecl MSG_ReadDeltaEntityStruct(
                msg_t *msg,
                int time,
                char *from,
                char *to,
                unsigned int number,
                int totalFields,
                int skippedFieldBits)
{
    bool IsPlaying; // al
    const char *v9; // eax
    const char *v10; // eax
    int v11; // eax
    char *EntityTypeName; // eax
    int v13; // [esp+20h] [ebp-6Ch]
    bool v14; // [esp+24h] [ebp-68h]
    int bits; // [esp+28h] [ebp-64h]
    char v16; // [esp+46h] [ebp-46h]
    NetField *v18; // [esp+50h] [ebp-3Ch]
    const NetField *field; // [esp+58h] [ebp-34h]
    int lastChanged; // [esp+64h] [ebp-28h]
    int nextChanged; // [esp+68h] [ebp-24h]
    const NetFieldList *stateFieldList; // [esp+6Ch] [ebp-20h]
    unsigned int lc; // [esp+74h] [ebp-18h]
    const NetField *stateFields; // [esp+78h] [ebp-14h]
    int etype; // [esp+80h] [ebp-Ch]
    bool justUnlinked; // [esp+87h] [ebp-5h]
    unsigned int i; // [esp+88h] [ebp-4h]
    unsigned int ia; // [esp+88h] [ebp-4h]
    unsigned int ib; // [esp+88h] [ebp-4h]

    if ( number >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                    2290,
                    0,
                    "%s",
                    "number < (1u << GENTITYNUM_BITS)") )
    {
        __debugbreak();
    }
    if ( MSG_ReadBit(msg) == 1 )
    {
        if ( cl_shownet && (cl_shownet->current.integer >= 2 || cl_shownet->current.integer == -1) )
            Com_Printf(16, "%3i: #%-3i remove\n", msg->readcount, number);
        return 1;
    }
    else if ( MSG_ReadBit(msg) )
    {
        justUnlinked = MSG_ReadBit(msg) != 0;
        lc = MSG_ReadLastChangedField(msg, totalFields);
        if ( cl_shownet && (cl_shownet->current.integer >= 2 || cl_shownet->current.integer == -1) )
            Com_Printf(16, "%3i: #%-3i ", msg->readcount, *(unsigned int *)to);
        *(unsigned int *)to = number;
        if ( strcmp(entityStateFields[0].name, "eType")
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                        2348,
                        0,
                        "%s",
                        "strcmp( entityStateFields[0].name, \"eType\" ) == 0") )
        {
            __debugbreak();
        }
        lastChanged = -1;
        nextChanged = MSG_ReadNumFieldsSkipped(msg, skippedFieldBits, lc + 1) - 1;
        if ( nextChanged <= 0 )
        {
            MSG_ReadDeltaField(msg, time, from, to, entityStateFields, 0, 0);
            lastChanged = 0;
            if ( lc )
                nextChanged = MSG_ReadNumFieldsSkipped(msg, skippedFieldBits, lc);
        }
        else
        {
            MSG_CopyFieldOver(entityStateFields, from, to, 0);
        }
        etype = MSG_GetField((const int *)(to + 190), 2);
        IsPlaying = Demo_IsPlaying();
        stateFieldList = MSG_GetStateFieldListForEntityType(etype, IsPlaying);
        stateFields = stateFieldList->array;
        if ( lc <= stateFieldList->count )
        {
            if ( lastChanged == lc )
            {
                lastChanged = lc;
            }
            else
            {
                do
                {
                    for ( i = lastChanged + 1; i < nextChanged; ++i )
                        MSG_CopyFieldOver(stateFields, from, to, i);
                    field = &stateFields[nextChanged];
                    v14 = 0;
                    if ( justUnlinked )
                    {
                        bits = field->bits;
                        if ( bits >= -66 && bits <= -61 )
                            v14 = 1;
                    }
                    MSG_ReadDeltaField(msg, time, from, to, field, 0, v14);
                    lastChanged = nextChanged;
                    if ( nextChanged == lc )
                        break;
                    nextChanged += MSG_ReadNumFieldsSkipped(msg, skippedFieldBits, lc - nextChanged);
                    if ( (int)lc < nextChanged )
                    {
                        v9 = va("nextChanged is %i, lc is %i", nextChanged, lc);
                        if ( !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                                        2440,
                                        0,
                                        "%s\n\t%s",
                                        "static_cast<int>( lc ) >= nextChanged",
                                        v9) )
                            __debugbreak();
                    }
                    if ( nextChanged <= lastChanged )
                    {
                        v10 = va("nextChanged is %i, lastChanged is %i", nextChanged, lastChanged);
                        if ( !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                                        2441,
                                        0,
                                        "%s\n\t%s",
                                        "nextChanged > lastChanged",
                                        v10) )
                            __debugbreak();
                    }
                }
                while ( nextChanged <= (int)lc );
            }
            for ( ia = 0; ia < lc; ++ia )
            {
                if ( stateFields[ia].changeHints == 2 )
                {
                    v18 = (NetField *)&stateFields[ia];
                    if ( justUnlinked && ((v13 = v18->bits, v13 < -66) || v13 > -61 ? (v16 = 0) : (v16 = 1), v16) )
                        MSG_ReadDeltaField(msg, time, from, to, v18, 0, 1);
                    else
                        MSG_ReadDeltaField(msg, time, from, to, v18, 0, 0);
                }
            }
            if ( lastChanged != lc
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                            2491,
                            0,
                            "%s",
                            "lastChanged == static_cast<int>( lc )") )
            {
                __debugbreak();
            }
            if ( lastChanged < 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                            2492,
                            0,
                            "%s",
                            "lastChanged >= 0") )
            {
                __debugbreak();
            }
            for ( ib = lastChanged + 1; ib < stateFieldList->count; ++ib )
                MSG_CopyFieldOver(stateFields, from, to, ib);
            if ( msg_dumpEnts->current.enabled )
            {
                v11 = MSG_GetField((const int *)(to + 190), 2);
                EntityTypeName = BG_GetEntityTypeName(v11);
                Com_Printf(14, "%3i: changed ent, eType %s\n", number, EntityTypeName);
            }
            if ( strcmp(stateFields->name, "eType")
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                            2503,
                            0,
                            "%s",
                            "strcmp( stateFields[0].name, \"eType\" ) == 0") )
            {
                __debugbreak();
            }
            return 0;
        }
        else
        {
            Com_Printf(14, "Last changed field was %i, but there are only %i fields\n", lc, stateFieldList->count);
            msg->overflowed = 1;
            return 0;
        }
    }
    else
    {
        memcpy(to, from, 0xE0u);
        return 0;
    }
}

int __cdecl MSG_ReadDeltaArchivedEntity(
                msg_t *msg,
                int time,
                const archivedEntity_s *from,
                archivedEntity_s *to,
                unsigned int number)
{
    if ( MSG_ReadDeltaEntity(msg, time, (entityState_s*)&from->s, &to->s, number))
        return 1;
    else
        return MSG_ReadAppendedDeltaStruct(
                         msg,
                         time,
                         (char *)&from->r,
                         (char *)&to->r,
                         number,
                         numArchivedEntityFields,
                         10,
                         archivedEntityFields,
                         1);
}

int __cdecl MSG_ReadAppendedDeltaStruct(
                msg_t *msg,
                int time,
                char *from,
                char *to,
                unsigned int number,
                int numFields,
                char indexBits,
                const NetField *stateFields,
                int skippedFieldBits)
{
    if ( number >= 1 << indexBits
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                    2260,
                    0,
                    "%s",
                    "number < (1u << indexBits)") )
    {
        __debugbreak();
    }
    MSG_ReadDeltaFields(msg, time, from, to, numFields, stateFields, skippedFieldBits);
    return 0;
}

void __cdecl MSG_ReadDeltaPlayerstate(
                int localClientNum,
                msg_t *msg,
                int time,
                const playerState_s *from,
                playerState_s *to,
                bool predictedFieldsIgnoreXor)
{
    bool IsPlaying; // al
    bool v7; // al
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    int v11; // eax
    const char *v12; // eax
    int v13; // eax
    const char *v14; // eax
    int v15; // eax
    const char *v16; // eax
    int v17; // eax
    const char *v18; // eax
    int v19; // eax
    int v20; // eax
    int v21; // eax
    const char *v22; // eax
    int v23; // eax
    int v24; // eax
    int v25; // eax
    const char *v26; // eax
    bool v27; // al
    bool v28; // al
    objectiveState_t v29; // eax
    int v30; // eax
    const char *v31; // eax
    int v32; // eax
    const char *v33; // eax
    bool v34; // [esp+16h] [ebp-270Ah]
    bool v35; // [esp+17h] [ebp-2709h]
    int i; // [esp+34h] [ebp-26ECh]
    clientActive_t *LocalClientGlobals; // [esp+38h] [ebp-26E8h]
    int lastChanged; // [esp+3Ch] [ebp-26E4h]
    int nextChanged; // [esp+40h] [ebp-26E0h]
    unsigned int numFields; // [esp+44h] [ebp-26DCh]
    int print; // [esp+48h] [ebp-26D8h]
    NetField *field; // [esp+4Ch] [ebp-26D4h]
    NetField *fielda; // [esp+4Ch] [ebp-26D4h]
    signed int count; // [esp+50h] [ebp-26D0h]
    int lc; // [esp+54h] [ebp-26CCh]
    NetField *array; // [esp+58h] [ebp-26C8h]
    float *v47; // [esp+5Ch] [ebp-26C4h]
    unsigned __int8 dst[9896]; // [esp+60h] [ebp-26C0h] BYREF
    int Bits; // [esp+2708h] [ebp-18h]
    float *v50; // [esp+270Ch] [ebp-14h]
    BOOL v51; // [esp+2710h] [ebp-10h]
    int v52; // [esp+2714h] [ebp-Ch]
    int fieldNum; // [esp+2718h] [ebp-8h]
    NetField *stateFields; // [esp+271Ch] [ebp-4h]

    if (!from)
    {
        from = (const playerState_s *)dst;
        memset(dst, 0, sizeof(playerState_s));
    }

    memcpy(to, from, sizeof(playerState_s));

    if (cl_shownet && (cl_shownet->current.integer >= 2 || cl_shownet->current.integer == -2))
    {
        print = 1;
        Com_Printf(16, "%3i: playerstate ", msg->readcount);
    }
    else
    {
        print = 0;
    }

    v51 = MSG_ReadBit(msg) > 0;
    IsPlaying = Demo_IsPlaying();
    stateFields = (NetField *)MSG_GetNetFieldList(NET_FIELD_TYPE_PLAYERSTATE, IsPlaying)->array;
    v7 = Demo_IsPlaying();
    count = MSG_GetNetFieldList(NET_FIELD_TYPE_PLAYERSTATE, v7)->count;
    lc = MSG_ReadLastChangedField(msg, count + 1);
    if (lc < 0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp", 2671, 0, "%s", "lc >= 0"))
        __debugbreak();
    if (lc <= count + 1)
    {
        if (Demo_IsPlaying())
        {
            v8 = va("PlayerState - Lastchangedfield: %d\n", lc);
            Demo_Printf(512, v8);
        }
        fieldNum = 0;
        lastChanged = -1;
        if (lc > 0)
        {
            --lc;
            while (lastChanged < lc)
            {
                if (msg->overflowed
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                        2692,
                        0,
                        "%s",
                        "!msg->overflowed"))
                {
                    __debugbreak();
                }
                nextChanged = lastChanged + MSG_ReadNumFieldsSkipped(msg, 4, lc - lastChanged);
                Bits = 0;
                if (Demo_IsPlaying())
                    Bits = MSG_GetNumBitsRead(msg);
                for (fieldNum = lastChanged + 1; fieldNum < nextChanged; ++fieldNum)
                {
                    if (stateFields[fieldNum].changeHints != 2)
                        MSG_CopyFieldOver(stateFields, (char *)from, (char *)to, fieldNum);
                }
                field = &stateFields[nextChanged];
                if (nextChanged > lc)
                {
                    v9 = va("nextChanged == %i, lc == %i", nextChanged, lc);
                    if (!Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                        2713,
                        0,
                        "%s\n\t%s",
                        "nextChanged <= lc",
                        v9))
                        __debugbreak();
                }
                if (nextChanged <= lastChanged)
                {
                    v10 = va("nextChanged == %i, lastChanged == %i", nextChanged, lastChanged);
                    if (!Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                        2714,
                        0,
                        "%s\n\t%s",
                        "nextChanged > lastChanged",
                        v10))
                        __debugbreak();
                }
                if (msg->overflowed
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                        2716,
                        0,
                        "%s",
                        "!msg->overflowed"))
                {
                    __debugbreak();
                }
                v35 = predictedFieldsIgnoreXor && v51 && field->changeHints == 3;
                MSG_ReadDeltaField(msg, time, (char *)from, (char *)to, field, print, v35);
                if (msg->overflowed
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                        2718,
                        0,
                        "%s",
                        "!msg->overflowed"))
                {
                    __debugbreak();
                }
                lastChanged = nextChanged;
                if (Demo_IsPlaying())
                {
                    v50 = (float *)((char *)&from->commandTime + field->offset);
                    v47 = (float *)((char *)&to->commandTime + field->offset);
                    if (Demo_GetAnalyzePrintDataType(field->bits))
                    {
                        v13 = MSG_GetNumBitsRead(msg);
                        v14 = va(
                            "PlayerState - FieldName: %s From: %d To: %d BitsUsed: %d\n",
                            field->name,
                            *(_DWORD *)v50,
                            *(_DWORD *)v47,
                            v13 - Bits);
                        Demo_Printf(512, v14);
                    }
                    else
                    {
                        v11 = MSG_GetNumBitsRead(msg);
                        v12 = va(
                            "PlayerState - FieldName: %s From: %.3f To: %.3f BitsUsed: %d\n",
                            field->name,
                            *v50,
                            *v47,
                            v11 - Bits);
                        Demo_Printf(512, v12);
                    }
                }
            }
            if (lastChanged != lc
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                    2733,
                    0,
                    "%s",
                    "lastChanged == lc"))
            {
                __debugbreak();
            }
        }
        for (fieldNum = lastChanged + 1; fieldNum < count; ++fieldNum)
            MSG_CopyFieldOver(stateFields, (char *)from, (char *)to, fieldNum);
        for (fieldNum = 0; fieldNum < lc; ++fieldNum)
        {
            fielda = &stateFields[fieldNum];
            if (fielda->changeHints == 2)
            {
                v34 = predictedFieldsIgnoreXor && v51 && fielda->changeHints == 3;
                MSG_ReadDeltaField(msg, time, (char*)from, (char*)to, fielda, print, v34);
            }
        }
        if (!v51)
        {
            LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
            if (!CL_GetPredictedOriginForServerTime(
                LocalClientGlobals,
                to->commandTime,
                to->origin,
                to->velocity,
                to->viewangles,
                &to->bobCycle,
                &to->movementDir))
            {
                Com_PrintError(14, "Unable to find the origin we sent, delta is not going to work");
                to->origin[0] = from->origin[0];
                to->origin[1] = from->origin[1];
                to->origin[2] = from->origin[2];
                to->velocity[0] = from->velocity[0];
                to->velocity[1] = from->velocity[1];
                to->velocity[2] = from->velocity[2];
                to->bobCycle = from->bobCycle;
                to->movementDir = from->movementDir;
                to->viewangles[0] = from->viewangles[0];
                to->viewangles[1] = from->viewangles[1];
                to->viewangles[2] = from->viewangles[2];
            }
        }
        Bits = 0;
        if (Demo_IsPlaying())
            Bits = MSG_GetNumBitsRead(msg);
        if (MSG_ReadBit(msg))
        {
            if (cl_shownet && cl_shownet->current.integer == 4)
                Com_Printf(16, "%s ", "PS_STATS");
            Bits = MSG_ReadBits(msg, 5);
            if ((Bits & 1) != 0)
                to->stats[0] = MSG_ReadShort(msg);
            if ((Bits & 2) != 0)
                to->stats[1] = MSG_ReadShort(msg);
            if ((Bits & 4) != 0)
                to->stats[2] = MSG_ReadShort(msg);
            if ((Bits & 8) != 0)
                to->stats[3] = MSG_ReadBits(msg, 5);
            if ((Bits & 0x10) != 0)
                to->stats[4] = MSG_ReadByte(msg);
        }
        if (Demo_IsPlaying())
        {
            v15 = MSG_GetNumBitsRead(msg);
            v16 = va("PlayerState - FieldName: PS_STATS BitsUsed: %d\n", v15 - Bits);
            Demo_Printf(512, v16);
            Bits = MSG_GetNumBitsRead(msg);
        }
        if (MSG_ReadBit(msg))
        {
            for (i = 0; i < 15; ++i)
            {
                if (MSG_ReadBit(msg))
                {
                    if (cl_shownet && cl_shownet->current.integer == 4)
                        Com_Printf(16, "%s ", "PS_WEAPON");
                    if (MSG_ReadBit(msg))
                        to->heldWeapons[i].weapon = MSG_ReadBits(msg, 11);
                    if (MSG_ReadBit(msg))
                        to->heldWeapons[i].model = MSG_ReadBits(msg, 4);
                    if (MSG_ReadBit(msg))
                        to->heldWeapons[i].options.i = MSG_ReadBits(msg, 32);
                    if (MSG_ReadBit(msg))
                        to->heldWeapons[i].fuelTankTime = MSG_ReadBits(msg, 32);
                    to->heldWeapons[i].overHeating = MSG_ReadBit(msg) != 0;
                    to->heldWeapons[i].needsRechamber = MSG_ReadBit(msg) != 0;
                    to->heldWeapons[i].heldBefore = MSG_ReadBit(msg) != 0;
                    to->heldWeapons[i].quickReload = MSG_ReadBit(msg) != 0;
                }
            }
        }
        if (Demo_IsPlaying())
        {
            v17 = MSG_GetNumBitsRead(msg);
            v18 = va("PlayerState - FieldName: PS_WEAPON BitsUsed: %d\n", v17 - Bits);
            Demo_Printf(512, v18);
            Bits = MSG_GetNumBitsRead(msg);
        }
        while (MSG_ReadBit(msg))
        {
            v52 = MSG_ReadBits(msg, 4);
            if (cl_shownet && cl_shownet->current.integer == 4)
                Com_Printf(16, "%s ", "PS_AMMO");
            v19 = MSG_ReadBits(msg, 11);
            to->ammoNotInClip[v52].ammoIndex = v19;
            v20 = MSG_ReadBits(msg, 10);
            to->ammoNotInClip[v52].count = v20;
        }
        if (Demo_IsPlaying())
        {
            v21 = MSG_GetNumBitsRead(msg);
            v22 = va("PlayerState - FieldName: PS_AMMO BitsUsed: %d\n", v21 - Bits);
            Demo_Printf(512, v22);
            Bits = MSG_GetNumBitsRead(msg);
        }
        while (MSG_ReadBit(msg))
        {
            v52 = MSG_ReadBits(msg, 4);
            if (cl_shownet && cl_shownet->current.integer == 4)
                Com_Printf(16, "%s ", "PS_AMMOCLIP");
            v23 = MSG_ReadBits(msg, 11);
            to->ammoInClip[v52].clipIndex = v23;
            v24 = MSG_ReadBits(msg, 10);
            to->ammoInClip[v52].count = v24;
        }
        if (Demo_IsPlaying())
        {
            v25 = MSG_GetNumBitsRead(msg);
            v26 = va("PlayerState - FieldName: PS_AMMOCLIP BitsUsed: %d\n", v25 - Bits);
            Demo_Printf(512, v26);
            Bits = MSG_GetNumBitsRead(msg);
        }
        if (MSG_ReadBit(msg))
        {
            v27 = Demo_IsPlaying();
            array = (NetField *)MSG_GetNetFieldList(NET_FIELD_TYPE_OBJECTIVE, v27)->array;
            v28 = Demo_IsPlaying();
            numFields = MSG_GetNetFieldList(NET_FIELD_TYPE_OBJECTIVE, v28)->count;
            for (fieldNum = 0; fieldNum < 32; ++fieldNum)
            {
                v29 = (objectiveState_t)MSG_ReadBits(msg, 3);
                to->objective[fieldNum].state = v29;
                MSG_ReadDeltaFields(msg, time, (char *)&from->objective[fieldNum], (char *)&to->objective[fieldNum], numFields, array, 1);
            }
        }
        if (Demo_IsPlaying())
        {
            v30 = MSG_GetNumBitsRead(msg);
            v31 = va("PlayerState - FieldName: PS_OBJECTIVE BitsUsed: %d\n", v30 - Bits);
            Demo_Printf(512, v31);
            Bits = MSG_GetNumBitsRead(msg);
        }
        if (MSG_ReadBit(msg))
        {
            MSG_ReadDeltaHudElems(msg, time, from->hud.archival, to->hud.archival, 31);
            MSG_ReadDeltaHudElems(msg, time, from->hud.current, to->hud.current, 31);
        }
        if (Demo_IsPlaying())
        {
            v32 = MSG_GetNumBitsRead(msg);
            v33 = va("PlayerState - FieldName: PS_HUDELEMS BitsUsed: %d\n", v32 - Bits);
            Demo_Printf(512, v33);
        }
    }
    else
    {
        MSG_Discard(msg);
    }
}

void __cdecl MSG_ReadDeltaHudElems(msg_t *msg, int time, const hudelem_s *from, hudelem_s *to, int count)
{
    bool IsPlaying; // al
    bool v6; // al
    const char *v7; // eax
    const char *v8; // eax
    int alignY; // [esp+0h] [ebp-28h]
    int alignYa; // [esp+0h] [ebp-28h]
    int nextChanged; // [esp+8h] [ebp-20h]
    int nextChangeda; // [esp+8h] [ebp-20h]
    int lastChanged; // [esp+Ch] [ebp-1Ch]
    int field; // [esp+10h] [ebp-18h]
    signed int numFields; // [esp+14h] [ebp-14h]
    unsigned int lc; // [esp+18h] [ebp-10h]
    signed int lca; // [esp+18h] [ebp-10h]
    const NetField *fields; // [esp+1Ch] [ebp-Ch]
    int i; // [esp+20h] [ebp-8h]
    int inuse; // [esp+24h] [ebp-4h]

    if ( count != 31
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                    2555,
                    0,
                    "%s",
                    "count == MAX_HUDELEMS_ARCHIVAL || count == MAX_HUDELEMS_CURRENT") )
    {
        __debugbreak();
    }
    IsPlaying = Demo_IsPlaying();
    fields = MSG_GetNetFieldList(NET_FIELD_TYPE_HUDELEMS, IsPlaying)->array;
    v6 = Demo_IsPlaying();
    numFields = MSG_GetNetFieldList(NET_FIELD_TYPE_HUDELEMS, v6)->count;
    inuse = MSG_ReadBits(msg, 5u);
    for ( i = 0; i < inuse; ++i )
    {
        lc = MSG_ReadBits(msg, 6u);
        lastChanged = -1;
        nextChanged = -1;
        if ( lc )
        {
            lca = lc - 1;
            while ( lastChanged < lca )
            {
                nextChanged = lastChanged + MSG_ReadNumFieldsSkipped(msg, 3, lca - lastChanged);
                if ( lca < nextChanged )
                {
                    v7 = va("lc is %i, nextChanged is %i", lca, nextChanged);
                    if ( !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                                    2579,
                                    0,
                                    "%s\n\t%s",
                                    "static_cast<int>( lc ) >= nextChanged",
                                    v7) )
                        __debugbreak();
                }
                if ( nextChanged <= lastChanged )
                {
                    v8 = va("nextChanged is %i, lastChanged is %i", nextChanged, lastChanged);
                    if ( !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                                    2580,
                                    0,
                                    "%s\n\t%s",
                                    "nextChanged > lastChanged",
                                    v8) )
                        __debugbreak();
                }
                for ( field = lastChanged + 1; field < nextChanged; ++field )
                    MSG_CopyFieldOver(fields, (char *)&from[i], (char *)&to[i], field);
                MSG_ReadDeltaField(msg, time, (char *)&from[i], (char *)&to[i], &fields[nextChanged], 0, 0);
                lastChanged = nextChanged;
            }
        }
        for ( nextChangeda = nextChanged + 1; nextChangeda < numFields; ++nextChangeda )
            MSG_CopyFieldOver(fields, (char *)&from[i], (char *)&to[i], nextChangeda);
        if ( (from[i].alignOrg & 0xF0) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                        2594,
                        0,
                        "%s\n\t(from[i].alignOrg) = %i",
                        "(!(from[i].alignOrg & ~15))",
                        from[i].alignOrg) )
        {
            __debugbreak();
        }
        if ( (to[i].alignOrg & 0xF0) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                        2595,
                        0,
                        "%s\n\t(from[i].alignOrg) = %i",
                        "(!(to[i].alignOrg & ~15))",
                        from[i].alignOrg) )
        {
            __debugbreak();
        }
        if ( (((int)from[i].alignOrg >> 2) & 3) == 3
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                        2598,
                        0,
                        "%s\n\t(from[i].alignOrg) = %i",
                        "(alignX == 0 || alignX == 1 || alignX == 2)",
                        from[i].alignOrg) )
        {
            __debugbreak();
        }
        alignY = from[i].alignOrg & 3;
        if ( (from[i].alignOrg & 3) != 0
            && alignY != 1
            && alignY != 2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                        2601,
                        0,
                        "%s\n\t(from[i].alignOrg) = %i",
                        "(alignY == 0 || alignY == 1 || alignY == 2)",
                        from[i].alignOrg) )
        {
            __debugbreak();
        }
        if ( (((int)to[i].alignOrg >> 2) & 3) == 3
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                        2604,
                        0,
                        "%s\n\t(to[i].alignOrg) = %i",
                        "(alignX == 0 || alignX == 1 || alignX == 2)",
                        to[i].alignOrg) )
        {
            __debugbreak();
        }
        alignYa = to[i].alignOrg & 3;
        if ( (to[i].alignOrg & 3) != 0
            && alignYa != 1
            && alignYa != 2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                        2607,
                        0,
                        "%s\n\t(to[i].alignOrg) = %i",
                        "(alignY == 0 || alignY == 1 || alignY == 2)",
                        to[i].alignOrg) )
        {
            __debugbreak();
        }
    }
    while ( inuse < count && to[inuse].type )
    {
        memset(&to[inuse], 0, sizeof(hudelem_s));
        if ( to[inuse].type
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg_mp.cpp",
                        2614,
                        0,
                        "%s",
                        "to[inuse].type == HE_TYPE_FREE") )
        {
            __debugbreak();
        }
        ++inuse;
    }
}

void __cdecl MSG_InitHuffman()
{
    msgInit = 1;
    MSG_initHuffmanInternal();
}

void MSG_initHuffmanInternal()
{
    unsigned int time2; // [esp+0h] [ebp-8h]
    unsigned int time; // [esp+4h] [ebp-4h]

    Huff_Init(&msgHuff);
    time = Sys_Milliseconds();
    Huff_BuildFromData(&msgHuff.compressDecompress, msg_hData);
    time2 = Sys_Milliseconds();
    Com_Printf(16, "Huffman Took %d Milliseconds\n", time2 - time);
}

void __cdecl MSG_DumpNetFieldChanges_f()
{
    int arraySize[6]; // [esp+0h] [ebp-58h]
    const int *array; // [esp+18h] [ebp-40h]
    int iSize; // [esp+1Ch] [ebp-3Ch]
    const char *arrayNames[6]; // [esp+20h] [ebp-38h]
    int i; // [esp+38h] [ebp-20h]
    unsigned int iArrayNum; // [esp+3Ch] [ebp-1Ch]
    const int *changeArray[6]; // [esp+40h] [ebp-18h]

    changeArray[0] = (const int *)&orderInfo;
    changeArray[1] = orderInfo.arcEntState;
    changeArray[2] = orderInfo.clientState;
    changeArray[3] = orderInfo.playerState;
    changeArray[4] = orderInfo.objective;
    changeArray[5] = orderInfo.hudElem;
    arraySize[0] = 79;
    arraySize[1] = 10;
    arraySize[2] = 52;
    arraySize[3] = 179;
    arraySize[4] = 9;
    arraySize[5] = 43;
    arrayNames[0] = "Entity State";
    arrayNames[1] = "Archived Entity State";
    arrayNames[2] = "Client State";
    arrayNames[3] = "Player State";
    arrayNames[4] = "Objective";
    arrayNames[5] = "HUD Elem";
    Com_Printf(0, "========================================\n");
    Com_Printf(0, "NetField changes. format: field# : #changes\n");
    for ( iArrayNum = 0; iArrayNum < 6; ++iArrayNum )
    {
        Com_Printf(0, "========================================\n");
        Com_Printf(0, "        %s\n", arrayNames[iArrayNum]);
        Com_Printf(0, "--------------------\n");
        array = changeArray[iArrayNum];
        iSize = arraySize[iArrayNum];
        for ( i = 0; i < iSize; ++i )
        {
            if ( array[i] )
                Com_Printf(0, "%3i :%8i\n", i, array[i]);
        }
    }
    Com_Printf(0, "========================================\n");
    Com_Printf(0, "========================================\n");
}
