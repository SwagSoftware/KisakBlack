#include "flame_system.h"
#include "flame_class_chunk.h"
#include "flame_class_fire.h"
#include "flame_class_smoke.h"
#include "flame_class_stream.h"
#include "flame_class_drips.h"
#include <universal/dvar.h>
#include <universal/q_shared.h>
#include <bgame/bg_weapons.h>
#include <bgame/bg_weapons_def.h>
#include <qcommon/common.h>
#include <cgame_mp/cg_local_mp.h>
#include "flame_sound.h"
#include <gfx_d3d/r_drawsurf.h>
#include <gfx_d3d/r_sprite.h>
#include <vehicle/nitrous_vehicle.h>
#include <clientscript/scr_const.h>
#include <cgame_mp/cg_vehicles_mp.h>
#include <cgame/cg_weapons.h>
#include <demo/demo_playback.h>
#include <cgame/cg_event.h>
#include <EffectsCore/fx_update.h>
#include <universal/com_math_anglevectors.h>
#include <cgame/cg_drawtools.h>
#include <cgame/cg_world.h>
#include <game_mp/g_utils_mp.h>

flameVarDef_t flameVars[128] =
{
  {
    "flameVar_streamChunkGravityStart",
    "stream/Chunk/GravityStart",
    200.0,
    -10000.0,
    10000.0,
    "Stream Gravity at Start"
  },
  {
    "flameVar_streamChunkGravityEnd",
    "stream/Chunk/GravityEnd",
    -240.0,
    -10000.0,
    10000.0,
    "Stream Gravity at End"
  },
  {
    "flameVar_streamChunkMaxSize",
    "stream/Chunk/MaxSize",
    100.0,
    1.0,
    500.0,
    "Maximum size any stream chunk can be"
  },
  {
    "flameVar_streamChunkStartSize",
    "stream/Chunk/StartSize",
    3.0,
    1.0,
    10.0,
    "Starting size of each stream chunk"
  },
  {
    "flameVar_streamChunkEndSize",
    "stream/Chunk/EndSize",
    60.0,
    1.0,
    500.0,
    "Goal ending size of each stream chunk"
  },
  {
    "flameVar_streamChunkStartSizeRand",
    "stream/Chunk/StartSizeRand",
    0.30000001,
    0.0,
    1.0,
    "Maximum percentage of start size to reduce each chunk by randomly"
  },
  {
    "flameVar_streamChunkEndSizeRand",
    "stream/Chunk/EndSizeRand",
    0.69999999,
    0.0,
    1.0,
    "Maximum percentage of end size to reduce each chunk by randomly"
  },
  {
    "flameVar_streamChunkDistScalar",
    "stream/Chunk/DistScalar",
    1.0,
    0.1,
    10.0,
    "Controls the chunk spacing.. lower values cause more overlapping, with decreased performance"
  },
  {
    "flameVar_streamChunkDistSwayScale",
    "stream/Chunk/DistSwyScale",
    0.2,
    0.02,
    1.0,
    "Sets the maximum distance scaling based on angular motion"
  },
  {
    "flameVar_streamChunkDistSwayVelMax",
    "stream/Chunk/DistSwyVelMax",
    90.0,
    1.0,
    1000.0,
    "Sets the maximum sway rate, at which the full dist scaling is applied"
  },
  {
    "flameVar_streamChunkSpeed",
    "stream/Chunk/Speed",
    800.0,
    10.0,
    2000.0,
    "Speed of chunks when they leave the nozzle"
  },
  {
    "flameVar_streamChunkDecel",
    "stream/Chunk/Decel",
    0.40000001,
    0.0,
    20.0,
    "Fraction of current speed to reduce per second"
  },
  {
    "flameVar_streamChunkVelocityAddScale",
    "stream/Chunk/VelAddScale",
    0.75,
    0.0,
    1.0,
    "Fraction of movement speed to add to the firing velocity"
  },
  {
    "flameVar_streamChunkDuration",
    "stream/Chunk/Duration",
    2.4000001,
    0.02,
    10.0,
    "Lifetime of each chunk in seconds"
  },
  {
    "flameVar_streamChunkDurationScaleMaxVel",
    "stream/Chunk/DurMaxVel",
    150.0,
    0.0,
    500.0,
    "Velocity at which to apply full duration scaling"
  },
  {
    "flameVar_streamChunkDurationVelScalar",
    "stream/Chunk/DurVelScalar",
    0.40000001,
    0.0,
    1.0,
    "Maximum movement duration scalar"
  },
  {
    "flameVar_streamChunkSizeSpeedScale",
    "stream/Chunk/SizeSpeedScale",
    0.0,
    -5.0,
    5.0,
    "Controls the rate at which size increases based on current speed"
  },
  {
    "flameVar_streamChunkSizeAgeScale",
    "stream/Chunk/SizeAgeScale",
    -2.0,
    -5.0,
    5.0,
    "Controls the rate at which size increases based on current age"
  },
  {
    "flameVar_streamChunkSpawnFireIntervalStart",
    "stream/Chunk/SpwnFireStart",
    0.1,
    0.0099999998,
    5.0,
    "Interval between spawning flames from chunks at start of chunk life"
  },
  {
    "flameVar_streamChunkSpawnFireIntervalEnd",
    "stream/Chunk/SpwnFireEnd",
    0.80000001,
    0.0099999998,
    5.0,
    "Interval between spawning flames from chunks at end of chunk life"
  },
  {
    "flameVar_streamChunkSpawnFireMinLifeFrac",
    "stream/Chunk/SpwnFireMinLife",
    0.0,
    0.0,
    1.0,
    "Don't spawn fire before this life frac"
  },
  {
    "flameVar_streamChunkSpawnFireMaxLifeFrac",
    "stream/Chunk/SpwnFireMaxLife",
    0.89999998,
    0.1,
    1.0,
    "Don't spawn fire after this life frac"
  },
  {
    "flameVar_streamChunkFireMinLifeFrac",
    "stream/Chunk/FireMinLife",
    0.80000001,
    0.0,
    1.0,
    "Make sure all flames spawned by chunks are at least this old when spawned"
  },
  {
    "flameVar_streamChunkFireMinLifeFracStart",
    "stream/Chunk/FireMinLifeStart",
    0.0,
    0.0,
    1.0,
    "Starting point for adjusting min flame life"
  },
  {
    "flameVar_streamChunkFireMinLifeFracEnd",
    "stream/Chunk/FireMinLifeEnd",
    0.2,
    0.0,
    1.0,
    "End point for adjusting min flame life"
  },
  {
    "flameVar_streamChunkDripsMinLifeFrac",
    "stream/Chunk/DripsMinLife",
    0.0,
    0.0,
    1.0,
    "Make sure all flames spawned by chunks are at least this old when spawned"
  },
  {
    "flameVar_streamChunkDripsMinLifeFracStart",
    "stream/Chunk/DripsMinLifeStart",
    0.0,
    0.0,
    1.0,
    "Starting point for adjusting min drips life"
  },
  {
    "flameVar_streamChunkDripsMinLifeFracEnd",
    "stream/Chunk/DripsMinLifeEnd",
    1.0,
    0.0,
    1.0,
    "End point for adjusting min drips life"
  },
  {
    "flameVar_streamChunkRotationRange",
    "stream/Chunk/RotationRange",
    180.0,
    0.0,
    180.0,
    "Maximum rotation (roll) of chunks"
  },
  {
    "flameVar_streamSizeRandSinWave",
    "stream/Chunk/SizeRandSinWave",
    4.1322398,
    0.1,
    50.0,
    "Stream thickness fluctuation frequency (SINE)"
  },
  {
    "flameVar_streamSizeRandCosWave",
    "stream/Chunk/SizeRandCosWave",
    7.23244,
    0.1,
    50.0,
    "Stream thickness fluctuation frequency (COSINE)"
  },
  {
    "flameVar_streamDripsChunkInterval",
    "stream/Chunk/DripsInterval",
    0.025,
    0.0099999998,
    1.0,
    "Interval between spawning a chunk that will (in it's lifetime) randomly spawn a Drips particle"
  },
  {
    "flameVar_streamDripsChunkMinFrac",
    "stream/Chunk/DripsMinFrac",
    0.0,
    0.0,
    1.0,
    "Minimum life fraction to spawn a Drips particle at"
  },
  {
    "flameVar_streamDripsChunkRandFrac",
    "stream/Chunk/DripsRandFrac",
    0.80000001,
    0.0,
    1.0,
    "Random fraction of life frac to add to the minimum fraction when deciding on Drips spawn time"
  },
  {
    "flameVar_streamSmokeChunkInterval",
    "stream/Chunk/SmokeInterval",
    0.025,
    0.0099999998,
    1.0,
    "Interval between spawning a chunk that will (in it's lifetime) randomly spawn a smoke particle"
  },
  {
    "flameVar_streamSmokeChunkMinFrac",
    "stream/Chunk/SmokeMinFrac",
    0.0,
    0.0,
    1.0,
    "Minimum life fraction to spawn a smoke particle at"
  },
  {
    "flameVar_streamSmokeChunkRandFrac",
    "stream/Chunk/SmokeRandFrac",
    0.80000001,
    0.0,
    1.0,
    "Random fraction of life frac to add to the minimum fraction when deciding on smoke spawn time"
  },
  {
    "flameVar_streamChunkCullDistSizeFrac",
    "stream/Chunk/CullDistSizeFrac",
    0.2,
    0.0,
    3.0,
    "Cull chunks within this percentage of our size"
  },
  {
    "flameVar_streamChunkCullMinLife",
    "stream/Chunk/CullMinLife",
    0.2,
    0.0,
    1.0,
    "Don't cull chunks younger than this age"
  },
  {
    "flameVar_streamChunkCullMaxLife",
    "stream/Chunk/CullMaxLife",
    0.80000001,
    0.0,
    1.0,
    "Don't cull chunks older than this age"
  },
  {
    "flameVar_streamFuelSizeStart",
    "stream/Fuel/SizeStart",
    1.0,
    0.1,
    50.0,
    "Stream fuel size at start"
  },
  {
    "flameVar_streamFuelSizeEnd",
    "stream/Fuel/SizeEnd",
    3.0,
    0.1,
    50.0,
    "Stream fuel size at end"
  },
  {
    "flameVar_streamFuelLength",
    "stream/Fuel/Length",
    96.0,
    0.0099999998,
    512.0,
    "Stream fuel length"
  },
  {
    "flameVar_streamFuelNumSegments",
    "stream/Fuel/NumSegments",
    16.0,
    1.0,
    128.0,
    "Number of stream fuel segments"
  },
  {
    "flameVar_streamFuelAnimLoopTime",
    "stream/Fuel/AnimLoopTime",
    1.0,
    0.1,
    5.0,
    "Animation cycle time for animated fuel sprites"
  },
  {
    "flameVar_streamFlameSizeStart",
    "stream/Flame/SizeStart",
    1.0,
    0.1,
    50.0,
    "Stream flame size at start"
  },
  {
    "flameVar_streamFlameSizeEnd",
    "stream/Flame/SizeEnd",
    3.0,
    0.1,
    50.0,
    "Stream flame size at end"
  },
  {
    "flameVar_streamFlameLength",
    "stream/Flame/Length",
    96.0,
    0.0099999998,
    512.0,
    "Stream flame length"
  },
  {
    "flameVar_streamFlameNumSegments",
    "stream/Flame/NumSegments",
    16.0,
    1.0,
    128.0,
    "Number of stream flame segments"
  },
  {
    "flameVar_streamFlameAnimLoopTime",
    "stream/Flame/AnimLoopTime",
    1.0,
    0.1,
    5.0,
    "Animation cycle time for animated flame sprites"
  },
  {
    "flameVar_streamPrimaryLightRadius",
    "stream/Light/PrimaryRadius",
    375.0,
    0.0,
    1000.0,
    "Primary flamethrower light source radius"
  },
  {
    "flameVar_streamPrimaryLightRadiusFlutter",
    "stream/Light/PrimaryRadiusFlutter",
    120.0,
    0.0,
    1000.0,
    "Primary flamethrower light source radius modulation"
  },
  {
    "flameVar_streamPrimaryLightR",
    "stream/Light/PrimaryR",
    0.69999999,
    0.0,
    1.0,
    "Primary flamethrower light source color R"
  },
  {
    "flameVar_streamPrimaryLightG",
    "stream/Light/PrimaryG",
    0.40000001,
    0.0,
    1.0,
    "Primary flamethrower light source color G"
  },
  {
    "flameVar_streamPrimaryLightB",
    "stream/Light/PrimaryB",
    0.0,
    0.0,
    1000.0,
    "Primary flamethrower light source color B"
  },
  {
    "flameVar_streamPrimaryLightFlutterR",
    "stream/Light/PrimaryFlutterR",
    0.1,
    0.0,
    1.0,
    "Primary flamethrower light source color modulation R"
  },
  {
    "flameVar_streamPrimaryLightFlutterG",
    "stream/Light/PrimaryFlutterG",
    0.1,
    0.0,
    1.0,
    "Primary flamethrower light source color modulation G"
  },
  {
    "flameVar_streamPrimaryLightFlutterB",
    "stream/Light/PrimaryFlutterB",
    0.1,
    0.0,
    1.0,
    "Primary flamethrower light source color modulation B"
  },
  {
    "flameVar_fireLife",
    "stream/fire/Life",
    0.80000001,
    0.1,
    3.0,
    "Life duration of flames"
  },
  {
    "flameVar_fireLifeRand",
    "stream/fire/LifeRand",
    0.60000002,
    0.0,
    3.0,
    "Random additive applied to life duration"
  },
  {
    "flameVar_fireSpeedScale",
    "stream/fire/SpeedScale",
    0.75,
    0.0,
    1.0,
    "Fraction of chunk speed that is applied to starting velocity of flames"
  },
  {
    "flameVar_fireSpeedScaleRand",
    "stream/fire/SpeedScaleRand",
    0.5,
    0.0,
    1.0,
    "Randomize speed scale by this fraction"
  },
  {
    "flameVar_fireVelocityAddZ",
    "stream/fire/VelocityAddZ",
    60.0,
    -200.0,
    200.0,
    "Upwards velocity applied to flame velocity"
  },
  {
    "flameVar_fireVelocityAddZRand",
    "stream/fire/VelocityAddZRand",
    -20.0,
    -200.0,
    200.0,
    "Random additive applied to upwards velocity"
  },
  {
    "flameVar_fireVelocityAddSideways",
    "stream/fire/VelocityAddSideways",
    0.30000001,
    0.0,
    1.0,
    "Random sideways velocity as fraction of forward velocity"
  },
  {
    "flameVar_fireGravity",
    "stream/fire/Gravity",
    -200.0,
    -500.0,
    0.0,
    "Fire gravity at start of life"
  },
  {
    "flameVar_fireGravityEnd",
    "stream/fire/GravityEnd",
    -300.0,
    -500.0,
    0.0,
    "Fire gravity at end of life"
  },
  {
    "flameVar_fireMaxRotVel",
    "stream/fire/MaxRotVel",
    60.0,
    0.0,
    200.0,
    "Maximum rotational velocity of flame"
  },
  {
    "flameVar_fireFriction",
    "stream/fire/Friction",
    4.0,
    0.0,
    10.0,
    "Fraction of starting velocity to deccelerate per second"
  },
  {
    "flameVar_fireEndSizeAdd",
    "stream/fire/EndSizeAdd",
    15.0,
    0.0,
    100.0,
    "Add this to the size of the chunk, to give the end size of the flame"
  },
  {
    "flameVar_fireStartSizeScale",
    "stream/fire/StartSizeScale",
    1.0,
    0.1,
    20.0,
    "Multiply this by the size of the chunk, to give the start size of the flame"
  },
  {
    "flameVar_fireEndSizeScale",
    "stream/fire/EndSizeScale",
    1.0,
    0.0,
    20.0,
    "Multiply this by the size of the chunk, to give the end size of the flame"
  },
  {
    "flameVar_dripsLife",
    "stream/drips/Life",
    5.0,
    0.1,
    10.0,
    "Life duration of flames"
  },
  {
    "flameVar_dripsLifeRand",
    "stream/drips/LifeRand",
    0.60000002,
    0.0,
    3.0,
    "Random additive applied to life duration"
  },
  {
    "flameVar_dripsSpeedScale",
    "stream/drips/SpeedScale",
    0.75,
    0.0,
    1.0,
    "Fraction of chunk speed that is applied to starting velocity of flames"
  },
  {
    "flameVar_dripsSpeedScaleRand",
    "stream/drips/SpeedScaleRand",
    0.5,
    0.0,
    1.0,
    "Randomize speed scale by this fraction"
  },
  {
    "flameVar_dripsVelocityAddZ",
    "stream/drips/VelocityAddZ",
    60.0,
    -200.0,
    200.0,
    "Upwards velocity applied to flame velocity"
  },
  {
    "flameVar_dripsVelocityAddZRand",
    "stream/drips/VelocityAddZRand",
    -20.0,
    -200.0,
    200.0,
    "Random additive applied to upwards velocity"
  },
  {
    "flameVar_dripsVelocityAddSideways",
    "stream/drips/VelocityAddSideways",
    0.30000001,
    0.0,
    1.0,
    "Random sideways velocity as fraction of forward velocity"
  },
  {
    "flameVar_dripsGravity",
    "stream/drips/Gravity",
    200.0,
    -1000.0,
    1000.0,
    "Fire gravity at start of life"
  },
  {
    "flameVar_dripsGravityEnd",
    "stream/drips/GravityEnd",
    200.0,
    -1000.0,
    1000.0,
    "Fire gravity at end of life"
  },
  {
    "flameVar_dripsMaxRotVel",
    "stream/drips/MaxRotVel",
    60.0,
    0.0,
    200.0,
    "Maximum rotational velocity of flame"
  },
  {
    "flameVar_dripsFriction",
    "stream/drips/Friction",
    4.0,
    0.0,
    10.0,
    "Fraction of starting velocity to deccelerate per second"
  },
  {
    "flameVar_dripsEndSizeAdd",
    "stream/drips/EndSizeAdd",
    0.0,
    0.0,
    100.0,
    "Add this to the size of the chunk, to give the end size of the flame"
  },
  {
    "flameVar_dripsStartSizeScale",
    "stream/drips/StartSizeScale",
    0.1,
    0.1,
    20.0,
    "Multiply this by the size of the chunk, to give the start size of the flame"
  },
  {
    "flameVar_dripsEndSizeScale",
    "stream/drips/EndSizeScale",
    0.0,
    0.0,
    20.0,
    "Multiply this by the size of the chunk, to give the end size of the flame"
  },
  {
    "flameVar_smokeLife",
    "stream/smoke/Life",
    2.5,
    0.1,
    5.0,
    "Life duration of smoke"
  },
  {
    "flameVar_smokeLifeRand",
    "stream/smoke/LifeRand",
    1.0,
    0.1,
    5.0,
    "Random time to add to life duration of smoke"
  },
  {
    "flameVar_smokeSpeedScale",
    "stream/smoke/SpeedScale",
    0.34999999,
    0.0,
    1.0,
    "Fraction of chunk velocity to apply to starting velocity of smoke"
  },
  {
    "flameVar_smokeVelocityAddZ",
    "stream/smoke/VelocityAddZ",
    10.0,
    0.0,
    100.0,
    "Upwards velocity applied to starting velocity of smoke"
  },
  {
    "flameVar_smokeGravity",
    "stream/smoke/Gravity",
    -80.0,
    -500.0,
    0.0,
    "Starting gravity of smoke"
  },
  {
    "flameVar_smokeGravityEnd",
    "stream/smoke/GravityEnd",
    -100.0,
    -500.0,
    0.0,
    "Ending gravity of smoke"
  },
  {
    "flameVar_smokeMaxRotation",
    "stream/smoke/MaxRotation",
    20.0,
    0.0,
    180.0,
    "Maximum rotation (ROLL) of smoke"
  },
  {
    "flameVar_smokeMaxRotVel",
    "stream/smoke/MaxRotVel",
    20.0,
    0.0,
    200.0,
    "Maximum rotational velocity of smoke"
  },
  {
    "flameVar_smokeFriction",
    "stream/smoke/Friction",
    4.0,
    0.0,
    10.0,
    "Fraction of initial velocity to deccelerate per second"
  },
  {
    "flameVar_smokeEndSizeAdd",
    "stream/smoke/EndSizeAdd",
    80.0,
    0.0,
    200.0,
    "Add this to the size of the chunk, to give the end size of the smoke"
  },
  {
    "flameVar_smokeStartSizeAdd",
    "stream/smoke/StartSizeAdd",
    20.0,
    0.0,
    200.0,
    "Add this to the size of the chunk, to give the start size of the smoke"
  },
  {
    "flameVar_smokeOriginSizeOfsZScale",
    "stream/smoke/OrgSizeOfsZScale",
    0.69999999,
    0.0,
    3.0,
    "Fraction of size to add to the Z origin of the smoke"
  },
  {
    "flameVar_smokeOriginOfsZ",
    "stream/smoke/OriginOfsZ",
    4.0,
    -30.0,
    100.0,
    "Absoluate value to add to the starting Z origin"
  },
  {
    "flameVar_smokeFadein",
    "stream/smoke/Fadein",
    0.5,
    0.0,
    1.0,
    "Fraction of life to spend fading the smoke in"
  },
  {
    "flameVar_smokeFadeout",
    "stream/smoke/Fadeout",
    0.5,
    0.0,
    1.0,
    "Fraction of life to spend fading the smoke out"
  },
  {
    "flameVar_smokeMaxAlpha",
    "stream/smoke/MaxAlpha",
    0.2,
    0.0,
    1.0,
    "Maximum alpha for smoke"
  },
  {
    "flameVar_smokeBrightness",
    "stream/smoke/Brightness",
    0.5,
    0.0,
    1.0,
    "Brightness value of smoke. 1.0 = untouched"
  },
  {
    "flameVar_smokeOriginOffset",
    "stream/smoke/OriginOffset",
    72.0,
    0.0,
    300.0,
    "Offset multiplier to shift smoke create location"
  },
  {
    "flameVar_collisionSpeedScale",
    "collisionSpeedScale",
    0.60000002,
    0.0,
    1.0,
    "Reduce speed by this scale when colliding with solid objects"
  },
  {
    "flameVar_collisionVolumeScale",
    "collisionVolumeScale",
    0.5,
    0.0,
    1.0,
    "Value used to scale the volume/size of the flame chunks used for character collision"
  },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL },
  { NULL, NULL, 0.0, 0.0, 0.0, NULL }
};

cspField_t flameTableFields[119] =
{
  { "flameVar_streamChunkGravityStart", 0, 7 },
  { "flameVar_streamChunkGravityEnd", 4, 7 },
  { "flameVar_streamChunkMaxSize", 8, 7 },
  { "flameVar_streamChunkStartSize", 12, 7 },
  { "flameVar_streamChunkEndSize", 16, 7 },
  { "flameVar_streamChunkStartSizeRand", 20, 7 },
  { "flameVar_streamChunkEndSizeRand", 24, 7 },
  { "flameVar_streamChunkDistScalar", 28, 7 },
  { "flameVar_streamChunkDistSwayScale", 32, 7 },
  { "flameVar_streamChunkDistSwayVelMax", 36, 7 },
  { "flameVar_streamChunkSpeed", 40, 7 },
  { "flameVar_streamChunkDecel", 44, 7 },
  { "flameVar_streamChunkVelocityAddScale", 48, 7 },
  { "flameVar_streamChunkDuration", 52, 7 },
  { "flameVar_streamChunkDurationScaleMaxVel", 56, 7 },
  { "flameVar_streamChunkDurationVelScalar", 60, 7 },
  { "flameVar_streamChunkSizeSpeedScale", 64, 7 },
  { "flameVar_streamChunkSizeAgeScale", 68, 7 },
  { "flameVar_streamChunkSpawnFireIntervalStart", 72, 7 },
  { "flameVar_streamChunkSpawnFireIntervalEnd", 76, 7 },
  { "flameVar_streamChunkSpawnFireMinLifeFrac", 80, 7 },
  { "flameVar_streamChunkSpawnFireMaxLifeFrac", 84, 7 },
  { "flameVar_streamChunkFireMinLifeFrac", 88, 7 },
  { "flameVar_streamChunkFireMinLifeFracStart", 92, 7 },
  { "flameVar_streamChunkFireMinLifeFracEnd", 96, 7 },
  { "flameVar_streamChunkDripsMinLifeFrac", 100, 7 },
  { "flameVar_streamChunkDripsMinLifeFracStart", 104, 7 },
  { "flameVar_streamChunkDripsMinLifeFracEnd", 108, 7 },
  { "flameVar_streamChunkRotationRange", 112, 7 },
  { "flameVar_streamSizeRandSinWave", 116, 7 },
  { "flameVar_streamSizeRandCosWave", 120, 7 },
  { "flameVar_streamDripsChunkInterval", 124, 7 },
  { "flameVar_streamDripsChunkMinFrac", 128, 7 },
  { "flameVar_streamDripsChunkRandFrac", 132, 7 },
  { "flameVar_streamSmokeChunkInterval", 136, 7 },
  { "flameVar_streamSmokeChunkMinFrac", 140, 7 },
  { "flameVar_streamSmokeChunkRandFrac", 144, 7 },
  { "flameVar_streamChunkCullDistSizeFrac", 148, 7 },
  { "flameVar_streamChunkCullMinLife", 152, 7 },
  { "flameVar_streamChunkCullMaxLife", 156, 7 },
  { "flameVar_streamFuelSizeStart", 160, 7 },
  { "flameVar_streamFuelSizeEnd", 164, 7 },
  { "flameVar_streamFuelLength", 168, 7 },
  { "flameVar_streamFuelNumSegments", 172, 7 },
  { "flameVar_streamFuelAnimLoopTime", 176, 7 },
  { "flameVar_streamFlameSizeStart", 180, 7 },
  { "flameVar_streamFlameSizeEnd", 184, 7 },
  { "flameVar_streamFlameLength", 188, 7 },
  { "flameVar_streamFlameNumSegments", 192, 7 },
  { "flameVar_streamFlameAnimLoopTime", 196, 7 },
  { "flameVar_streamPrimaryLightRadius", 200, 7 },
  { "flameVar_streamPrimaryLightRadiusFlutter", 204, 7 },
  { "flameVar_streamPrimaryLightR", 208, 7 },
  { "flameVar_streamPrimaryLightG", 212, 7 },
  { "flameVar_streamPrimaryLightB", 216, 7 },
  { "flameVar_streamPrimaryLightFlutterR", 220, 7 },
  { "flameVar_streamPrimaryLightFlutterG", 224, 7 },
  { "flameVar_streamPrimaryLightFlutterB", 228, 7 },
  { "flameVar_fireLife", 232, 7 },
  { "flameVar_fireLifeRand", 236, 7 },
  { "flameVar_fireSpeedScale", 240, 7 },
  { "flameVar_fireSpeedScaleRand", 244, 7 },
  { "flameVar_fireVelocityAddZ", 248, 7 },
  { "flameVar_fireVelocityAddZRand", 252, 7 },
  { "flameVar_fireVelocityAddSideways", 256, 7 },
  { "flameVar_fireGravity", 260, 7 },
  { "flameVar_fireGravityEnd", 264, 7 },
  { "flameVar_fireMaxRotVel", 268, 7 },
  { "flameVar_fireFriction", 272, 7 },
  { "flameVar_fireEndSizeAdd", 276, 7 },
  { "flameVar_fireStartSizeScale", 280, 7 },
  { "flameVar_fireEndSizeScale", 284, 7 },
  { "flameVar_dripsLife", 288, 7 },
  { "flameVar_dripsLifeRand", 292, 7 },
  { "flameVar_dripsSpeedScale", 296, 7 },
  { "flameVar_dripsSpeedScaleRand", 300, 7 },
  { "flameVar_dripsVelocityAddZ", 304, 7 },
  { "flameVar_dripsVelocityAddZRand", 308, 7 },
  { "flameVar_dripsVelocityAddSideways", 312, 7 },
  { "flameVar_dripsGravity", 316, 7 },
  { "flameVar_dripsGravityEnd", 320, 7 },
  { "flameVar_dripsMaxRotVel", 324, 7 },
  { "flameVar_dripsFriction", 328, 7 },
  { "flameVar_dripsEndSizeAdd", 332, 7 },
  { "flameVar_dripsStartSizeScale", 336, 7 },
  { "flameVar_dripsEndSizeScale", 340, 7 },
  { "flameVar_smokeLife", 344, 7 },
  { "flameVar_smokeLifeRand", 348, 7 },
  { "flameVar_smokeSpeedScale", 352, 7 },
  { "flameVar_smokeVelocityAddZ", 356, 7 },
  { "flameVar_smokeGravity", 360, 7 },
  { "flameVar_smokeGravityEnd", 364, 7 },
  { "flameVar_smokeMaxRotation", 368, 7 },
  { "flameVar_smokeMaxRotVel", 372, 7 },
  { "flameVar_smokeFriction", 376, 7 },
  { "flameVar_smokeEndSizeAdd", 380, 7 },
  { "flameVar_smokeStartSizeAdd", 384, 7 },
  { "flameVar_smokeOriginSizeOfsZScale", 388, 7 },
  { "flameVar_smokeOriginOfsZ", 392, 7 },
  { "flameVar_smokeFadein", 396, 7 },
  { "flameVar_smokeFadeout", 400, 7 },
  { "flameVar_smokeMaxAlpha", 404, 7 },
  { "flameVar_smokeBrightness", 408, 7 },
  { "flameVar_smokeOriginOffset", 412, 7 },
  { "flameVar_collisionSpeedScale", 416, 7 },
  { "flameVar_collisionVolumeScale", 420, 7 },
  { "name", 424, 0 },
  { "fire", 428, 11 },
  { "smoke", 432, 11 },
  { "heat", 436, 11 },
  { "drips", 440, 11 },
  { "streamFuel", 444, 11 },
  { "streamFuel2", 448, 11 },
  { "streamFlame", 452, 11 },
  { "streamFlame2", 456, 11 },
  { "flameOffLoopSound", 460, 0 },
  { "flameIgniteSound", 464, 0 },
  { "flameOnLoopSound", 468, 0 },
  { "flameCooldownSound", 472, 0 }
};

unsigned int flame_freeze_id;
unsigned int flame_spawn_id;

unsigned __int8 sv_flameSourceLookup[1024];
unsigned __int8 flameSourceLookup[1024];
flameSource_t flameSources[64];
flameSource_t sv_flameSources[64];

flameRender_s flameVarList[16];
int numFlameVars;

const int bg_iNumFlameTableFields = 119;

phys_static_array<flameGeneric_s *, 1000> sv_flames;
phys_static_array<flameGeneric_s *, 1000> cl_flames;

int numFlameTablesLoaded;
const char *loadedFlameTables[8];

const dvar_s *flameDvars[128];

const dvar_t *flame_test;
const dvar_t *flame_use_dvars;
const dvar_t *flame_render;
const dvar_t *flame_team_damage;
const dvar_t *flame_debug_render;
const dvar_t *flame_config_valid;
const dvar_t *default_flameVars_initialHitDamage;
const dvar_t *default_flameVars_timedDamageDuration;
const dvar_t *default_flameVars_timedDamageInterval;
const dvar_t *flameVar_editingFlameTable;
const dvar_t *flameVar_lastFlameTable;
const dvar_t *flame_kick_offset;
const dvar_t *flame_kick_speed;
const dvar_t *flame_kick_recover_speed;

cmd_function_s Flame_CMD_PrintDVarsToConsol_VAR;
cmd_function_s Flame_CMD_Test_Toggle_VAR;
cmd_function_s Flame_CMD_Use_Dvars_Toggle_VAR;

void __cdecl Flame_Init_FlameVars()
{
    int i; // [esp+14h] [ebp-4h]

    memset(flameVarList, 0, sizeof(flameVarList));
    numFlameVars = 0;
    for ( i = 0; i < bg_iNumFlameTableFields; ++i )
    {
        if (flameVars[i].dvarName)
            flameDvars[i] = _Dvar_RegisterFloat(
                flameVars[i].dvarName,
                flameVars[i].defaultVal,
                flameVars[i].minVal,
                flameVars[i].maxVal,
                0,
                flameVars[i].description);
    }
}

void __cdecl Flame_Reset_FlameVars()
{
    memset(flameVarList, 0, sizeof(flameVarList));
    numFlameVars = 0;
}

void __cdecl Flame_Init_Classes()
{
    flame_freeze_id = 0;
    flame_spawn_id = 0;
    Flame_Class_Chunk_Init();
    Flame_Class_Fire_Init();
    Flame_Class_Smoke_Init();
    Flame_Class_Stream_Init();
    Flame_Class_Drips_Init();
}

void __cdecl Flame_Init_Sources()
{
    int i; // [esp+0h] [ebp-4h]

    Flame_Init_DVars();
    memset(flameSources, 0, sizeof(flameSources));
    memset(flameSourceLookup, 0, 0x3FEu);
    memset(sv_flameSources, 0, sizeof(sv_flameSources));
    memset(sv_flameSourceLookup, 0, 0x3FEu);
    for ( i = 0; i < 64; ++i )
        sv_flameSources[i].is_server_alloc = 1;
}

void __cdecl Flame_Init_DVars()
{
    if ( !flame_test )
    {
        flame_test = _Dvar_RegisterBool("flame_test", 0, 0x4080u, "Turn on/off Flamethrower testing");
        flame_use_dvars = _Dvar_RegisterBool(
                                                "flame_use_dvars",
                                                0,
                                                0x4080u,
                                                "Turn on/off use of dvars for flamethrower behavior");
        flame_render = _Dvar_RegisterBool("flame_render", 1, 0x4080u, "Turn on/off Flamethrower rendering");
        flame_team_damage = _Dvar_RegisterBool(
                                                    "flame_team_damage",
                                                    0,
                                                    0x4080u,
                                                    "Turn on/off Flamethrower damage to friendlies");
        flame_debug_render = _Dvar_RegisterInt(
                                                     "flame_debug_render",
                                                     0,
                                                     0,
                                                     2,
                                                     0x4080u,
                                                     "Turn on/off Flamethrower debug rendering");
        flame_config_valid = _Dvar_RegisterBool("flame_config_valid", 0, 0x4000u, "Internal use only");
        default_flameVars_initialHitDamage = _Dvar_RegisterFloat(
                                                                                     "defaultHitDamage",
                                                                                     5.0,
                                                                                     1.0,
                                                                                     200.0,
                                                                                     0,
                                                                                     "default initial damage delivered by getting hit by a flame ball");
        default_flameVars_timedDamageDuration = _Dvar_RegisterFloat(
                                                                                            "defaultDamageDuration",
                                                                                            300.0,
                                                                                            0.0,
                                                                                            10000.0,
                                                                                            0,
                                                                                            "default duration in milliseconds that damage should continue to get delivered");
        default_flameVars_timedDamageInterval = _Dvar_RegisterFloat(
                                                                                            "defaultDamageInterval",
                                                                                            300.0,
                                                                                            0.0,
                                                                                            10000.0,
                                                                                            0,
                                                                                            "default interval in milliseconds that damage should continue to get delivered");
        flameVar_editingFlameTable = _Dvar_RegisterString(
                                                                     "flameVar_editingFlameTable",
                                                                     "none",
                                                                     0,
                                                                     "This is the most recent flametable that has been put up for editing, mostly included to help artists");
        flameVar_lastFlameTable = _Dvar_RegisterString(
                                                                "flameVar_lastFlameTable",
                                                                "none",
                                                                0,
                                                                "This is the previous flametable that was put up for editing, mostly included to help artists");
        flame_kick_offset = _Dvar_RegisterVec3(
                                                    "flame_kick_offset",
                                                    (-2.0),
                                                    (0.0),
                                                    (-0.5),
                                                    -3.4028235e38,
                                                    3.4028235e38,
                                                    0x80u,
                                                    "Origin offset for flamethrower view model while firing.");
        flame_kick_speed = _Dvar_RegisterFloat(
                                                 "flame_kick_speed",
                                                 4.0,
                                                 0.0,
                                                 3.4028235e38,
                                                 0x80u,
                                                 "Speed at which the flamethrower view model transitions to flame_kick_offset.");
        flame_kick_recover_speed = _Dvar_RegisterFloat(
                                                                 "flame_kick_recover_speed",
                                                                 4.0,
                                                                 0.0,
                                                                 3.4028235e38,
                                                                 0x80u,
                                                                 "Speed at which the flamethrower view model transitions back from flame_kick_offset.");
    }
}

char __cdecl Flame_TableInDevGui(const char *tableName)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < numFlameTablesLoaded; ++i )
    {
        if ( !I_strcmp(tableName, loadedFlameTables[i]) )
            return 1;
    }
    return 0;
}

void __cdecl Flame_InitDevGui()
{
    char *v0; // eax
    char *v1; // eax
    unsigned int j; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    int ia; // [esp+4h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    numFlameTablesLoaded = 0;
    for (j = 0; j < BG_GetNumWeapons(); ++j)
    {
        weapDef = BG_GetWeaponDef(j);
        if (weapDef && weapDef->flameTableFirstPersonPtr && !Flame_TableInDevGui(weapDef->flameTableFirstPerson))
        {
            if (numFlameTablesLoaded >= 8
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_system.cpp",
                    333,
                    0,
                    "%s\n\t(numFlameTablesLoaded) = %i",
                    "(numFlameTablesLoaded < 8)",
                    numFlameTablesLoaded))
            {
                __debugbreak();
            }
            loadedFlameTables[numFlameTablesLoaded] = weapDef->flameTableFirstPerson;
            if (!numFlameTablesLoaded)
            {
                Dvar_SetString((dvar_s*)flameVar_editingFlameTable, loadedFlameTables[0]);
                Flame_DVarsToFlameVars(weapDef->flameTableFirstPersonPtr);
            }
            ++numFlameTablesLoaded;
        }
        if (weapDef && weapDef->flameTableThirdPersonPtr && !Flame_TableInDevGui(weapDef->flameTableThirdPerson))
        {
            if (numFlameTablesLoaded >= 8
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_system.cpp",
                    347,
                    0,
                    "%s\n\t(numFlameTablesLoaded) = %i",
                    "(numFlameTablesLoaded < 8)",
                    numFlameTablesLoaded))
            {
                __debugbreak();
            }
            loadedFlameTables[numFlameTablesLoaded] = weapDef->flameTableThirdPerson;
            if (!numFlameTablesLoaded)
            {
                Dvar_SetString((dvar_s *)flameVar_editingFlameTable, loadedFlameTables[0]);
                Flame_DVarsToFlameVars(weapDef->flameTableThirdPersonPtr);
            }
            ++numFlameTablesLoaded;
        }
    }
    Cmd_AddCommandInternal("flame_printdvars", Flame_CMD_PrintDVarsToConsol, &Flame_CMD_PrintDVarsToConsol_VAR);
    Cmd_AddCommandInternal("flame_test_toggle", Flame_CMD_Test_Toggle, &Flame_CMD_Test_Toggle_VAR);
    Cmd_AddCommandInternal("flame_use_dvars_toggle", Flame_CMD_Use_Dvars_Toggle, &Flame_CMD_Use_Dvars_Toggle_VAR);
    Cbuf_InsertText(0, "devgui_cmd \"FT/Toggle Flamethrower:1\" flame_test_toggle");
    Cbuf_InsertText(0, "devgui_cmd \"FT/Toggle Dvar Use:2\" flame_use_dvars_toggle");
    for (i = 0; i < numFlameTablesLoaded; ++i)
    {
        v0 = va(
            "devgui_cmd \"FT/Flametables:3/%s:%i\" \"set flameVar_editingFlameTable %s\"",
            loadedFlameTables[i],
            i + 1,
            loadedFlameTables[i]);
        Cbuf_InsertText(0, v0);
    }
    for (ia = 0; ia < 128; ++ia)
    {
        if (flameVars[ia].name)
        {
            v1 = va("devgui_dvar \"FT/Edit Dvars:4/%s:%i\" %s", flameVars[ia].name, ia + 1, flameVars[ia].dvarName);
            Cbuf_InsertText(0, v1);
        }
    }
    Cbuf_InsertText(0, "devgui_cmd \"FT/Print DVars:5\" flame_printdvars");
}

void __cdecl Flame_Init()
{
    Flame_Init_Classes();
    Flame_Init_Sources();
}

void __cdecl Flame_Reset()
{
    Flame_Init_Classes();
    Flame_Init_Sources();
    Flame_Reset_FlameVars();
}

void __cdecl Flame_CMD_PrintDVarsToConsol()
{
    int i; // [esp+8h] [ebp-4h]

    Com_Printf(0, "\n------------------------------------------\nDumping Flamethrower Dvars...\n\n[BEGIN]\n");
    Com_Printf(
        0,
        "These are the float values for the flametable %s. BE CAREFUL NOT TO CLOBBER THE MATERIAL AND SOUND NAMES IN THE WEAP"
        "ONSETTINGS.GDT!\n"
        "\n",
        flameVar_editingFlameTable->current.string);
    for ( i = 0; i < bg_iNumFlameTableFields; ++i )
    {
        if ( flameVars[i].dvarName )
            Com_Printf(0, "        \"%s\" \"%f\"\n", flameVars[i].dvarName, flameDvars[i]->current.value);
    }
    Com_Printf(0, "[END]\n\nDone.\n------------------------------------------\n");
}

void __cdecl Flame_CMD_Test_Toggle()
{
    if ( flame_test->current.enabled )
        Cbuf_InsertText(0, "flame_test 0");
    else
        Cbuf_InsertText(0, "flame_test 1");
}

void __cdecl Flame_CMD_Use_Dvars_Toggle()
{
    if ( flame_use_dvars->current.enabled )
        Cbuf_InsertText(0, "flame_use_dvars 0");
    else
        Cbuf_InsertText(0, "flame_use_dvars 1");
}

void __cdecl Flame_DVarsToFlameVars(flameTable *fTable)
{
    const cspField_t *field; // [esp+4h] [ebp-10h]
    const cspField_t *fielda; // [esp+4h] [ebp-10h]
    float flt; // [esp+8h] [ebp-Ch]
    flameTable *fltptr; // [esp+Ch] [ebp-8h]
    flameTable *fltptra; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]
    int ia; // [esp+10h] [ebp-4h]

    if ( !fTable
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_system.cpp", 467, 0, "%s", "fTable") )
    {
        __debugbreak();
    }
    if ( !fTable->name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_system.cpp", 468, 0, "%s", "fTable->name") )
    {
        __debugbreak();
    }
    if ( I_strcmp(flameVar_editingFlameTable->current.string, flameVar_lastFlameTable->current.string) )
    {
        i = 0;
        fltptr = fTable;
        field = flameTableFields;
        while ( i < bg_iNumFlameTableFields )
        {
            if ( field->iFieldType == 7 )
                Dvar_SetFloatByName(field->szName, fltptr->flameVar_streamChunkGravityStart);
            ++field;
            ++i;
            fltptr = (flameTable *)((char *)fltptr + 4);
        }
        Dvar_SetString((dvar_s *)flameVar_lastFlameTable, flameVar_editingFlameTable->current.string);
    }
    ia = 0;
    fielda = flameTableFields;
    fltptra = fTable;
    while ( ia < bg_iNumFlameTableFields )
    {
        if ( fielda->iFieldType == 7 )
        {
            flt = Dvar_GetFloat(fielda->szName);
            fltptra->flameVar_streamChunkGravityStart = flt;
        }
        ++fielda;
        ++ia;
        fltptra = (flameTable *)((char *)fltptra + 4);
    }
}

flameTable *__cdecl Flame_FindFlameTable(const char *tableName)
{
    unsigned int i; // [esp+0h] [ebp-Ch]
    unsigned int numWeapons; // [esp+4h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    numWeapons = BG_GetNumWeapons();
    for ( i = 0; i < numWeapons; ++i )
    {
        weapDef = BG_GetWeaponDef(i);
        if ( weapDef && weapDef->flameTableFirstPersonPtr && !I_strcmp(weapDef->flameTableFirstPerson, tableName) )
            return weapDef->flameTableFirstPersonPtr;
        if ( weapDef && weapDef->flameTableThirdPersonPtr && !I_strcmp(weapDef->flameTableThirdPerson, tableName) )
            return weapDef->flameTableThirdPersonPtr;
    }
    return 0;
}

float __cdecl Flame_SwayRand(float x, float y, int time)
{
    float sx = fmodf((float)time * x * 3.1415927f * 2.0f, 6.2831855f);
    float sy = fmodf((float)time * y * 3.1415927f * 2.0f, 6.2831855f);

    return sinf(sx) * cosf(sy);
}

flameSource_t *__cdecl SV_Flame_Source_Get(int entityNum)
{
    if ( sv_flameSourceLookup[entityNum] )
        return &sv_flameSources[sv_flameSourceLookup[entityNum] - 1];
    else
        return 0;
}

flameSource_t *__cdecl Flame_Source_Get(int entityNum)
{
    if ( flameSourceLookup[entityNum] )
        return &flameSources[flameSourceLookup[entityNum] - 1];
    else
        return 0;
}

flameRender_s *__cdecl Flame_Get_FlameRender(const char *name)
{
    char v1; // cl
    flameRender_s *v3; // [esp+8h] [ebp-38h]
    const char *v4; // [esp+Ch] [ebp-34h]
    bool is_found; // [esp+37h] [ebp-9h]
    flameRender_s *trav; // [esp+38h] [ebp-8h]
    int i; // [esp+3Ch] [ebp-4h]

    if ( strlen(name) >= 0x7F
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_system.cpp",
                    579,
                    0,
                    "%s\n\t%s",
                    "strlen( name ) < (MAX_FLAMEVAR_NAME-1)",
                    "Flame config name is too long") )
    {
        __debugbreak();
    }
    is_found = 0;
    trav = flameVarList;
    for ( i = 0; i < numFlameVars; ++i )
    {
        if ( !strcmp(name, trav->name) )
        {
            is_found = 1;
            break;
        }
        ++trav;
    }
    if ( !is_found )
    {
        if ( numFlameVars >= 15
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_system.cpp",
                        594,
                        0,
                        "%s",
                        "numFlameVars < MAX_FLAMEVARLIST-1") )
        {
            __debugbreak();
        }
        trav = &flameVarList[numFlameVars++];
        v4 = name;
        v3 = trav;
        do
        {
            v1 = *v4;
            v3->name[0] = *v4++;
            v3 = (flameRender_s *)((char *)v3 + 1);
        }
        while ( v1 );
    }
    return trav;
}

void __cdecl Flame_Item_Init(flameGeneric_s *item, unsigned int itemSize)
{
    flameList_t globalList; // [esp+0h] [ebp-10h]
    flameList_t localList; // [esp+8h] [ebp-8h]

    globalList = item->listGlobal;
    localList = item->listLocal;
    memset(item, 0, itemSize);
    item->listGlobal = globalList;
    item->listLocal = localList;
}

void __cdecl Flame_List_Init(flameGeneric_s *list, int itemSize, int listSize)
{
    int i; // [esp+4h] [ebp-8h]

    for ( i = 0; i < listSize; ++i )
    {
        list->listGlobal.prev = (flameGeneric_s *)((char *)list - itemSize);
        if ( !i )
            list->listGlobal.prev = 0;
        list->listGlobal.next = (flameGeneric_s *)((char *)list + itemSize);
        if ( i >= listSize - 1 )
            list->listGlobal.next = 0;
        list = (flameGeneric_s *)((char *)list + itemSize);
    }
}

void __cdecl Flame_List_Move_Global(flameGeneric_s *item, flameGeneric_s **fromList, flameGeneric_s **toList)
{
    if ( *fromList )
    {
        if ( item == *fromList )
            *fromList = item->listGlobal.next;
        if ( item->listGlobal.prev )
            item->listGlobal.prev->listGlobal.next = item->listGlobal.next;
        if ( item->listGlobal.next )
            item->listGlobal.next->listGlobal.prev = item->listGlobal.prev;
        item->listGlobal.next = 0;
        item->listGlobal.prev = 0;
        if ( *toList )
        {
            item->listGlobal.next = *toList;
            (*toList)->listGlobal.prev = item;
        }
        *toList = item;
    }
}

void __cdecl Flame_List_Add_Local(flameGeneric_s *item, flameGeneric_s **localList)
{
    item->listLocal.prev = 0;
    item->listLocal.next = *localList;
    if ( *localList )
        (*localList)->listLocal.prev = item;
    *localList = item;
}

void __cdecl Flame_List_Sub_Local(flameGeneric_s *item, flameGeneric_s **localList)
{
    if ( *localList == item )
        *localList = item->listLocal.next;
    if ( item->listLocal.prev )
        item->listLocal.prev->listLocal.next = item->listLocal.next;
    if ( item->listLocal.next )
        item->listLocal.next->listLocal.prev = item->listLocal.prev;
    item->listLocal.prev = 0;
    item->listLocal.next = 0;
}

void __cdecl CG_Flame_Age_All_Objects(int time)
{
    PROF_SCOPED("CG_Flame_Age_All_Objects");

    cl_flames.m_alloc_count = 0;
    Flame_Class_Chunk_Age(0, time);
    Flame_Class_Fire_Age(0, time);
    Flame_Class_Smoke_Age(0, time);
    Flame_Class_Stream_Age(0);
    Flame_Class_Drips_Age(0, time);
}

void __cdecl SV_Flame_Age_All_Objects(int time)
{
    PROF_SCOPED("SV_Flame_Age_All_Objects");

    sv_flames.m_alloc_count = 0;
    Flame_Class_Chunk_Age(1, time);
    Flame_Class_Fire_Age(1, time);
    Flame_Class_Smoke_Age(1, time);
    Flame_Class_Stream_Age(1);
    Flame_Class_Drips_Age(1, time);
}

flameSource_t *__cdecl Flame_Source_Alloc(int entityNum)
{
    flameSource_t *source; // [esp+0h] [ebp-8h]
    unsigned __int8 i; // [esp+7h] [ebp-1h]

    i = 0;
    source = flameSources;
    while ( i < 0x40u )
    {
        if ( !source->inUse )
        {
            source->entityNum = entityNum;
            source->inUse = 1;
            flameSourceLookup[entityNum] = i + 1;
            Flame_Class_Source_Sound_Init(source);
            return source;
        }
        ++i;
        ++source;
    }
    return 0;
}

flameSource_t *__cdecl SV_Flame_Source_Alloc(int entityNum)
{
    flameSource_t *v2; // [esp+0h] [ebp-8h]
    unsigned __int8 i; // [esp+7h] [ebp-1h]

    i = 0;
    v2 = sv_flameSources;
    while ( i < 0x40u )
    {
        if ( !v2->inUse )
        {
            v2->entityNum = entityNum;
            v2->inUse = 1;
            sv_flameSourceLookup[entityNum] = i + 1;
            return v2;
        }
        ++i;
        ++v2;
    }
    return 0;
}

void __cdecl Flame_Render_Sprite(
                cg_s *clientGlobals,
                Material *material,
                flameGeneric_s *flameGeneric,
                float alpha,
                float frame,
                unsigned __int8 col)
{
    float angles[3]; // [esp+Ch] [ebp-10h] BYREF
    unsigned __int8 rgbaColor[4]; // [esp+18h] [ebp-4h] BYREF

    rgbaColor[0] = col;
    rgbaColor[1] = col;
    rgbaColor[2] = col;
    rgbaColor[3] = (int)(alpha * 255.0);
    AxisToAngles(clientGlobals->refdef.viewaxis, angles);
    angles[2] = flameGeneric->phys.rotation;
    if ( frame < 0.0 )
        frame = (float)(flameGeneric->age.lastUpdateTime - flameGeneric->age.startTime)
                    / (float)(flameGeneric->age.endTime - flameGeneric->age.startTime);
    R_BeginCodeMeshVerts();
    R_GenerateQuadStampCodeMeshVerts(
        material,
        angles,
        flameGeneric->phys.origin,
        flameGeneric->size.current,
        rgbaColor,
        frame);
    R_EndCodeMeshVerts();
}

void __cdecl Flame_Render_Sprites(
    cg_s *clientGlobals,
    Material *material,
    flameGeneric_s **flameGenericList,
    int numItems)
{
    renderQuad_t quads[512];
    float viewAxis[3][3];
    int quadCount;
    int totalCount;
    flameGeneric_s **cur;

    AxisCopy(clientGlobals->refdef.viewaxis, viewAxis);

    cur = &flameGenericList[numItems - 1];

    R_BeginCodeMeshVerts();

    totalCount = 0;

    do
    {
        quadCount = 0;

        while (totalCount < numItems && quadCount < 512)
        {
            flameGeneric_s *f = *cur;
            renderQuad_t *q = &quads[quadCount];

            q->pos[0] = f->phys.origin[0];
            q->pos[1] = f->phys.origin[1];
            q->pos[2] = f->phys.origin[2];

            q->rotation = f->phys.rotation;   // +0x2C
            q->radius = f->size.current;     // +0x34

            q->lifeFrac =
                (float)(f->age.lastUpdateTime - f->age.startTime) /
                (float)(f->age.endTime - f->age.startTime);

            --cur;
            ++quadCount;
            ++totalCount;
        }

        R_GenerateQuadStampCodeMeshVertsArray(
            material,
            quads,
            quadCount,
            viewAxis
        );

    } while (totalCount < numItems);

    R_EndCodeMeshVerts();
}


int __cdecl Flame_GetLocalClientFlameSource(int localClientNum, int EntNum)
{
    return EntNum;
}

bool __cdecl Flame_GetLocalClientSourceRange(const char *__formal)
{
    return 1;
}

void __cdecl Flame_Source_Update(flameSource_t *source, flameWeaponConfig_t *weaponConfig)
{
    source->bIsFiring = weaponConfig->bIsFiring;
}

void __cdecl SV_Flame_Update_Source_Internal(
                flameSource_t *source,
                int entityNum,
                flameWeaponConfig_t *weaponConfig,
                int time)
{
    if ( weaponConfig->bIsFiring != source->bIsFiring )
        source->currentStream = 0;
    if ( weaponConfig->bIsFiring || weaponConfig->bFireWhileIdle )
        Flame_Class_Stream_Fire_Chunks(source, weaponConfig, time, 0);
    Flame_Source_Update(source, weaponConfig);
}

void __cdecl CG_Flame_Update_Source_Internal(
                flameSource_t *source,
                int entityNum,
                flameWeaponConfig_t *weaponConfig,
                int time,
                flameRender_s *flameRend)
{
    if ( weaponConfig->bIsFiring != source->bIsFiring )
        source->currentStream = 0;
    if ( weaponConfig->bIsFiring || weaponConfig->bFireWhileIdle )
        Flame_Class_Stream_Fire_Chunks(source, weaponConfig, time, flameRend);
    Flame_Source_Update(source, weaponConfig);
}

void __cdecl SV_Flame_Update_Source(int entityNum, flameWeaponConfig_t *weaponConfig, int time)
{
    flameSource_t *source; // [esp+0h] [ebp-4h]

    source = SV_Flame_Source_Get(entityNum);
    if ( !source )
    {
        source = SV_Flame_Source_Alloc(entityNum);
        if ( !source
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_system.cpp",
                        1003,
                        0,
                        "%s\n\t%s",
                        "source",
                        "No free flameSources") )
        {
            __debugbreak();
        }
    }
    SV_Flame_Update_Source_Internal(source, entityNum, weaponConfig, time);
}

void __cdecl CG_Flame_Update_Source(int localClientNum)
{
    int LocalClientFlameSource; // eax
    int v2; // eax
    int v3; // eax
    int v4; // [esp+8h] [ebp-20Ch]
    int time; // [esp+Ch] [ebp-208h]
    float dir[3]; // [esp+34h] [ebp-1E0h] BYREF
    float start[3]; // [esp+40h] [ebp-1D4h] BYREF
    flameSource_t *source; // [esp+88h] [ebp-18Ch]
    const clientInfo_t *ci; // [esp+8Ch] [ebp-188h]
    const centity_s *player; // [esp+90h] [ebp-184h]
    int j; // [esp+94h] [ebp-180h]
    int weapon; // [esp+98h] [ebp-17Ch]
    const WeaponDef *gunnerWeapDef; // [esp+9Ch] [ebp-178h]
    int gunnerIndex; // [esp+A0h] [ebp-174h]
    const vehicle_info_t *info; // [esp+A4h] [ebp-170h]
    unsigned __int8 thirdBoneIndex; // [esp+AAh] [ebp-16Ah] BYREF
    bool isFireEvent; // [esp+ABh] [ebp-169h]
    flameWeaponConfig_t flameWeaponConfig; // [esp+ACh] [ebp-168h] BYREF
    int thirdDobjHandle; // [esp+F4h] [ebp-120h]
    bool isFirstPerson; // [esp+FBh] [ebp-119h]
    flameRender_s *flameRend; // [esp+FCh] [ebp-118h]
    unsigned __int8 firstBoneIndex; // [esp+103h] [ebp-111h] BYREF
    orientation_t thirdOrient; // [esp+104h] [ebp-110h] BYREF
    int firstDobjHandle; // [esp+134h] [ebp-E0h]
    orientation_t firstOrient; // [esp+138h] [ebp-DCh] BYREF
    bool isFiring; // [esp+16Bh] [ebp-A9h]
    unsigned int bits; // [esp+16Ch] [ebp-A8h]
    int indexHigh; // [esp+170h] [ebp-A4h]
    unsigned __int16 boneName; // [esp+174h] [ebp-A0h]
    const centity_s *centForFlags; // [esp+178h] [ebp-9Ch]
    cg_s *cgameGlob; // [esp+17Ch] [ebp-98h]
    centity_s *cent; // [esp+180h] [ebp-94h]
    unsigned int centInNextSnapshot[32]; // [esp+184h] [ebp-90h] BYREF
    int usingVehicleTagFlash; // [esp+204h] [ebp-10h]
    int i; // [esp+208h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+20Ch] [ebp-8h]
    int clientNum; // [esp+210h] [ebp-4h]

    boneName = scr_const.tag_flash;
    weapDef = 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->nextSnap && (!zombietron->current.enabled || localClientNum <= 0) )
    {
        memcpy(centInNextSnapshot, cgameGlob->centInNextSnapshot, sizeof(centInNextSnapshot));
        clientNum = cgameGlob->clientNum;
        centInNextSnapshot[clientNum >> 5] |= 0x80000000 >> (clientNum & 0x1F);
        clientNum = cgameGlob->predictedPlayerState.clientNum;
        centInNextSnapshot[clientNum >> 5] |= 0x80000000 >> (clientNum & 0x1F);
        for ( indexHigh = 0; indexHigh < 32; ++indexHigh )
        {
            bits = centInNextSnapshot[indexHigh];
            i = 32 * indexHigh;
            while ( bits )
            {
                if ( (bits & 0x80000000) != 0 )
                {
                    cent = CG_GetEntity(localClientNum, i);
                    centForFlags = cent;
                    usingVehicleTagFlash = 0;
                    if ( ((*((unsigned int *)cent + 201) >> 1) & 1) != 0 )
                    {
                        weapDef = BG_GetWeaponDef(cent->nextState.weapon);
                        if ( cent->nextState.eType == 14 )
                        {
                            info = CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
                            for ( gunnerIndex = 0; gunnerIndex < 4; ++gunnerIndex )
                            {
                                weapon = info->gunnerWeaponIndex[gunnerIndex];
                                gunnerWeapDef = BG_GetWeaponDef(weapon);
                                if ( gunnerWeapDef->weapType == WEAPTYPE_GAS )
                                {
                                    boneName = scr_const.tag_flash_gunner[gunnerIndex];
                                    usingVehicleTagFlash = 1;
                                    weapDef = gunnerWeapDef;
                                    for ( j = 0; j < com_maxclients->current.integer; ++j )
                                    {
                                        player = CG_GetEntity(localClientNum, j);
                                        if ( player->nextState.eType == 1 )
                                        {
                                            ci = &cgameGlob->bgs.clientinfo[player->nextState.clientNum];
                                            if ( ci->attachedVehEntNum == i && ci->attachedVehSeat == gunnerIndex + 1 )
                                            {
                                                centForFlags = player;
                                                goto LABEL_25;
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                        }
LABEL_25:
                        if ( centForFlags && weapDef && weapDef->weapType == WEAPTYPE_GAS )
                        {
                            thirdDobjHandle = cent->nextState.number;
                            firstDobjHandle = CG_WeaponDObjHandle(localClientNum);
                            isFirstPerson = cgameGlob->nextSnap->ps.clientNum == i
                                                     && ((cgameGlob->nextSnap->ps.otherFlags & 2) == 0
                                                        || cgameGlob->renderingThirdPerson != TP_FOR_MODEL);
                            if ( Demo_IsThirdPersonCamera() || Demo_IsMovieCamera() )
                                isFirstPerson = 0;
                            if ( !weapDef->bUseAltTagFlash || isFirstPerson )
                            {
                                if ( !usingVehicleTagFlash )
                                    boneName = scr_const.tag_flash;
                            }
                            else
                            {
                                boneName = scr_const.tag_flash_alt;
                            }
                            firstBoneIndex = -2;
                            thirdBoneIndex = -2;
                            isFireEvent = cent->nextState.eType == 50;
                            if ( cent->nextState.eType == 6 && (centForFlags->nextState.lerp.eFlags & 0x40) != 0 )
                                isFireEvent = 1;
                            if ( (isFirstPerson
                                 || isFireEvent
                                 || CG_GetBoneIndex(localClientNum, thirdDobjHandle, boneName, &thirdBoneIndex))
                                && (!isFirstPerson || CG_GetBoneIndex(localClientNum, firstDobjHandle, boneName, &firstBoneIndex)) )
                            {
                                isFiring = flame_test->current.enabled
                                                || (centForFlags->nextState.lerp.eFlags & 0x40) != 0
                                                && (cent->nextState.lerp.eFlags & 0x300) == 0
                                                && cgameGlob->predictedPlayerState.waterlevel < 3
                                                || isFireEvent;
                                if ( isFiring )
                                    *((unsigned int *)cent + 201) |= 4u;
                                if ( isFiring && isFirstPerson && cent->lastMuzzleFlash < cgameGlob->time - weapDef->iRechamberBoltTime )
                                {
                                    *((unsigned int *)cent + 201) |= 4u;
                                    cent->lastMuzzleFlash = cgameGlob->time;
                                }
                                if ( isFiring )
                                {
                                    if ( isFirstPerson )
                                    {
                                        FX_GetBoneOrientation(localClientNum, firstDobjHandle, firstBoneIndex, &firstOrient);
                                    }
                                    else if ( isFireEvent )
                                    {
                                        thirdOrient.origin[0] = cent->pose.origin[0];
                                        thirdOrient.origin[1] = cent->pose.origin[1];
                                        thirdOrient.origin[2] = cent->pose.origin[2];
                                        AnglesToAxis(cent->pose.angles, thirdOrient.axis);
                                    }
                                    else
                                    {
                                        FX_GetBoneOrientation(localClientNum, thirdDobjHandle, thirdBoneIndex, &thirdOrient);
                                    }
                                }
                                if ( isFirstPerson )
                                {
                                    flameWeaponConfig.fTable = weapDef->flameTableFirstPersonPtr;
                                    flameRend = Flame_Get_FlameRender(weapDef->flameTableFirstPersonPtr->name);
                                }
                                else
                                {
                                    flameWeaponConfig.fTable = weapDef->flameTableThirdPersonPtr;
                                    flameRend = Flame_Get_FlameRender(weapDef->flameTableThirdPersonPtr->name);
                                }
                                if ( isFiring )
                                {
                                    if ( isFirstPerson )
                                    {
                                        flameWeaponConfig.origin[0] = firstOrient.origin[0];
                                        flameWeaponConfig.origin[1] = firstOrient.origin[1];
                                        flameWeaponConfig.origin[2] = firstOrient.origin[2];
                                        AxisToAngles(firstOrient.axis, flameWeaponConfig.angle);
                                    }
                                    else
                                    {
                                        flameWeaponConfig.origin[0] = thirdOrient.origin[0];
                                        flameWeaponConfig.origin[1] = thirdOrient.origin[1];
                                        flameWeaponConfig.origin[2] = thirdOrient.origin[2];
                                        AxisToAngles(thirdOrient.axis, flameWeaponConfig.angle);
                                    }
                                }
                                else
                                {
                                    flameWeaponConfig.origin[0] = cent->nextState.lerp.pos.trBase[0];
                                    flameWeaponConfig.origin[1] = cent->nextState.lerp.pos.trBase[1];
                                    flameWeaponConfig.origin[2] = cent->nextState.lerp.pos.trBase[2];
                                    flameWeaponConfig.angle[0] = cent->nextState.lerp.apos.trBase[0];
                                    flameWeaponConfig.angle[1] = cent->nextState.lerp.apos.trBase[1];
                                    flameWeaponConfig.angle[2] = cent->nextState.lerp.apos.trBase[2];
                                }
                                flameWeaponConfig.bFireWhileIdle = 0;
                                flameWeaponConfig.bIsFiring = isFiring;
                                flameWeaponConfig.burnRate = 1.0f;
                                flameWeaponConfig.strength = 1.0f;
                                flameWeaponConfig.thickness = 1.0f;
                                flameWeaponConfig.entityOrigin[0] = cent->nextState.lerp.pos.trBase[0];
                                flameWeaponConfig.entityOrigin[1] = cent->nextState.lerp.pos.trBase[1];
                                flameWeaponConfig.entityOrigin[2] = cent->nextState.lerp.pos.trBase[2];
                                flameWeaponConfig.damage = weapDef->damage;
                                LocalClientFlameSource = Flame_GetLocalClientFlameSource(localClientNum, i);
                                source = Flame_Source_Get(LocalClientFlameSource);
                                if ( !source )
                                {
                                    CG_GetLocalClientGlobals(localClientNum);
                                    v2 = Flame_GetLocalClientFlameSource(localClientNum, i);
                                    source = Flame_Source_Alloc(v2);
                                    if ( !source
                                        && !Assert_MyHandler(
                                                    "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_system.cpp",
                                                    1271,
                                                    0,
                                                    "%s\n\t%s",
                                                    "source",
                                                    "No free flameSources") )
                                    {
                                        __debugbreak();
                                    }
                                }
                                if ( source )
                                {
                                    v3 = Flame_GetLocalClientFlameSource(localClientNum, i);
                                    source->entityNum = v3;
                                    source->firstDobjHandle = firstDobjHandle;
                                    source->firstBoneIndex = firstBoneIndex;
                                    source->thirdDobjHandle = thirdDobjHandle;
                                    source->thirdBoneIndex = thirdBoneIndex;
                                    source->isKillcamFlame = cgameGlob->inKillCam;
                                    source->firingThroughGeo = 0;
                                    if ( isFiring && !isFirstPerson )
                                    {
                                        trace_t trace; // [esp+4Ch] [ebp-1C8h] BYREF
                                        AngleVectors(flameWeaponConfig.angle, dir, 0, 0);
                                        start[0] = (float)(-23.0 * dir[0]) + flameWeaponConfig.origin[0];
                                        start[1] = (float)(-23.0 * dir[1]) + flameWeaponConfig.origin[1];
                                        start[2] = (float)(-23.0 * dir[2]) + flameWeaponConfig.origin[2];
                                        if ( flame_debug_render->current.integer > 0 )
                                            CG_DebugLine(start, flameWeaponConfig.origin, colorRed, 0, 300);
                                        CG_LocationalTrace(
                                            &trace,
                                            start,
                                            flameWeaponConfig.origin,
                                            source->entityNum,
                                            0x280E833,
                                            0,
                                            0);
                                        if ( trace.fraction < 1.0 )
                                        {
                                            flameWeaponConfig.origin[0] = start[0];
                                            flameWeaponConfig.origin[1] = start[1];
                                            flameWeaponConfig.origin[2] = start[2];
                                            source->firingThroughGeo = 1;
                                        }
                                    }
                                    time = CG_GetLocalClientGlobals(localClientNum)->time;
                                    CG_Flame_Update_Source_Internal(source, i, &flameWeaponConfig, time, flameRend);
                                    v4 = CG_GetLocalClientGlobals(localClientNum)->time;
                                    Flame_Class_Source_Sound(
                                        localClientNum,
                                        isFiring,
                                        isFirstPerson,
                                        v4,
                                        flameWeaponConfig.fTable,
                                        source);
                                }
                            }
                        }
                    }
                }
                ++i;
                bits *= 2;
            }
        }
    }
}

void __cdecl Flame_Generate_Verts(int localClientNum)
{
    char *Name; // [esp+8h] [ebp-Ch]
    int i; // [esp+10h] [ebp-4h]

    PROF_SCOPED("Flame_Generate_Verts");
    ZoneTextF("(cl=%d)", localClientNum);
    if ( flame_render->current.enabled )
    {
        Flame_Class_Smoke_Render_All(localClientNum);
        Flame_Class_Fire_Render_All(localClientNum);
        Flame_Class_Drips_Render_All(localClientNum);
        Flame_Class_Chunk_Render_All(localClientNum);
        Flame_Class_Stream_Render_All(localClientNum);
        for ( i = 0; i < numFlameVars; ++i )
        {
            Flame_Class_Fire_Render_Local_List(localClientNum, (flameFire_t *)flameVarList[i].fireList);
            Flame_Class_Drips_Render_Local_List(localClientNum, (flameDrips_t *)flameVarList[i].dripsList);
        }
    }
}

void __cdecl CG_Flame_Update_ViewModel(int localClientNum, centity_s *cent)
{
    int k; // [esp+18h] [ebp-14h]
    float oldOffset; // [esp+1Ch] [ebp-10h]
    int j; // [esp+20h] [ebp-Ch]
    int i; // [esp+24h] [ebp-8h]
    cg_s *cgameGlob; // [esp+28h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( (cent->nextState.lerp.eFlags & 0x40) != 0 )
    {
        for ( i = 0; i < 3; ++i )
        {
            cgameGlob->flamethrowerKickOffset[i] = (float)((float)((float)(flame_kick_offset->current.vector[i]
                                                                                                                                     * flame_kick_speed->current.value)
                                                                                                                     * (float)cgameGlob->frametime)
                                                                                                     / 1000.0)
                                                                                     + cgameGlob->flamethrowerKickOffset[i];
            if (fabs(cgameGlob->flamethrowerKickOffset[i]) > fabs(flame_kick_offset->current.vector[i]))
                cgameGlob->flamethrowerKickOffset[i] = flame_kick_offset->current.vector[i];
        }
    }
    else
    {
        for ( j = 0; j < 3; ++j )
        {
            oldOffset = cgameGlob->flamethrowerKickOffset[j];
            cgameGlob->flamethrowerKickOffset[j] = oldOffset
                                                                                     - (float)((float)((float)(flame_kick_offset->current.vector[j]
                                                                                                                                     * flame_kick_recover_speed->current.value)
                                                                                                                     * (float)cgameGlob->frametime)
                                                                                                     / 1000.0);
            if ( fabs(cgameGlob->flamethrowerKickOffset[j]) > fabs(oldOffset) )
                cgameGlob->flamethrowerKickOffset[j] = 0.0f;
        }
    }
    for ( k = 0; k < 3; ++k )
    {
        cgameGlob->viewModelAxis[3][0] = (float)(cgameGlob->flamethrowerKickOffset[k] * cgameGlob->viewModelAxis[k][0])
                                                                     + cgameGlob->viewModelAxis[3][0];
        cgameGlob->viewModelAxis[3][1] = (float)(cgameGlob->flamethrowerKickOffset[k] * cgameGlob->viewModelAxis[k][1])
                                                                     + cgameGlob->viewModelAxis[3][1];
        cgameGlob->viewModelAxis[3][2] = (float)(cgameGlob->flamethrowerKickOffset[k] * cgameGlob->viewModelAxis[k][2])
                                                                     + cgameGlob->viewModelAxis[3][2];
    }
}

int flame_randomseed = 12345; // cool
double __cdecl Flame_Random(bool is_server)
{
    if ( is_server )
        return G_random();
    flame_randomseed = 214013 * flame_randomseed + 2531011;
    return (double)((flame_randomseed >> 16) & 0x7FFF) / 32768.0;
}

double __cdecl Flame_CRandom(bool is_server)
{
    return Flame_Random(is_server) * 2.0 - 1.0;
}

double __cdecl Flame_CalcTimeScale(int startTime, int endTime)
{
    if ( endTime - startTime <= 0 )
        return 0.0;
    else
        return 1.0 / (double)(endTime - startTime);
}

double __cdecl Flame_CalcInvStartSpeed(float invInitialSpeed, float speedScale)
{
    float v3; // [esp+0h] [ebp-8h]
    float speedScalea; // [esp+14h] [ebp+Ch]

    if ( invInitialSpeed == 0.0 )
        v3 = 0.0f;
    else
        v3 = 1.0 / invInitialSpeed;
    speedScalea = speedScale * v3;
    if ( speedScalea == 0.0 )
        return 0.0;
    else
        return 1.0 / speedScalea;
}

