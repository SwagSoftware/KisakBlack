#include "snd_db.h"
#include "snd.h"

#include <cstring>

const char *CATEGORY_IDS[5] =
{ "sfx", "music", "voice", "ui", NULL };

const char *LIMIT_TYPES[5] =
{ "none", "oldest", "reject", "priority", NULL };

const char *LIMIT_IDS[5] =
{ "none", "oldest", "reject", "softest", NULL };

const char *RANDOMIZE_TYPES[4] =
{ "volume", "pitch", "variant", NULL };

const char *SPATIALIZED_TYPES[4] =
{ "2d", "3d", "2.5d", NULL };

const char *LOAD_TYPES[5] =
{ "unknown", "loaded", "streamed", "primed", NULL };

const char *MATURE_TYPES[4] =
{ "both", "yes", "no", NULL };

const char *LOOP_TYPES[3] =
{ "nonlooping", "looping", NULL };

const char *NO_YES[3] =
{ "no", "yes", NULL };

const char *RESTRICT_IDS[3] =
{ "unrestricted", "restricted", NULL };

const char *PAUSE_IDS[3] =
{ "nopause", "pause", NULL };

const char *BUS_IDS[4] =
{ "world", "game", "voice", NULL };

const char *MOVE_TYPES[9] =
{
  "none",
  "left_player",
  "center_player",
  "right_player",
  "random_player",
  "left_shot",
  "center_shot",
  "right_shot",
  NULL
};



const char *CURVES[33];
const char *PANS[33];
const char *SNAPSHOT_GROUPS[65];
const char *GROUPS[65];

const snd_csv_entry_t SND_CURVE_FIELDS[18] =
{
  { "name", 0u, true, SND_CSV_STRING, 0.0, 0.0, 32u, NULL },
  { "name", 32u, true, SND_CSV_HASH, 0.0, 0.0, 0u, NULL },
  { "x0", 36u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "y0", 40u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "x1", 44u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "y1", 48u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "x2", 52u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "y2", 56u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "x3", 60u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "y3", 64u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "x4", 68u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "y4", 72u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "x5", 76u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "y5", 80u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "x6", 84u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "y6", 88u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "x7", 92u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "y7", 96u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL }
};

const snd_csv_entry_t SND_PAN_FIELDS[8] =
{
  { "name", 0u, true, SND_CSV_STRING, 0.0, 0.0, 32u, NULL },
  { "name", 64u, true, SND_CSV_HASH, 0.0, 0.0, 0u, NULL },
  { "front", 36u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "back", 40u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "center", 44u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "lfe", 48u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "left", 52u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "right", 56u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL }
};

const snd_csv_entry_t SND_GROUP_FIELDS[6] =
{
  { "name", 0u, true, SND_CSV_STRING, 0.0, 0.0, 32u, NULL },
  { "name", 64u, true, SND_CSV_HASH, 0.0, 0.0, 0u, NULL },
  { "attenuationSp", 76u, true, SND_CSV_DBSPL, 0.0, 100.0, 0u, NULL },
  { "attenuationMp", 78u, true, SND_CSV_DBSPL, 0.0, 100.0, 0u, NULL },
  { "category", 72u, true, SND_CSV_ENUM, 0.0, 0.0, 0u, CATEGORY_IDS },
  { "parent", 32u, false, SND_CSV_STRING, 0.0, 0.0, 32u, NULL }
};


const snd_csv_entry_t SND_ALIAS_FIELDS[53] =
{
  { "vol_min", 44u, true, SND_CSV_DBSPL, 0.0, 100.0, 0u, NULL },
  { "vol_max", 46u, true, SND_CSV_DBSPL, 0.0, 100.0, 0u, NULL },
  { "pitch_min", 50u, true, SND_CSV_CENTS, -2400.0, 1200.0, 0u, NULL },
  { "pitch_max", 52u, true, SND_CSV_CENTS, -2400.0, 1200.0, 0u, NULL },
  { "dist_min", 56u, true, SND_CSV_USHORT, 0.0, 65535.0, 0u, NULL },
  { "dist_max", 58u, true, SND_CSV_USHORT, 0.0, 65535.0, 0u, NULL },
  { "dist_reverb_max", 60u, true, SND_CSV_USHORT, 0.0, 65535.0, 0u, NULL },
  { "probability", 70u, true, SND_CSV_NORM_BYTE, 0.0, 1.0, 0u, NULL },
  { "envelop_min", 62u, true, SND_CSV_USHORT, 0.0, 0.0, 0u, NULL },
  { "envelop_max", 64u, true, SND_CSV_USHORT, 0.0, 0.0, 0u, NULL },
  { "envelop_percentage", 66u, true, SND_CSV_DBSPL, 0.0, 100.0, 0u, NULL },
  { "center_send", 42u, true, SND_CSV_DBSPL, 0.0, 100.0, 0u, NULL },
  { "reverb_send", 40u, true, SND_CSV_DBSPL, 0.0, 100.0, 0u, NULL },
  { "start_delay", 38u, true, SND_CSV_USHORT, 0.0, 65535.0, 0u, NULL },
  { "min_priority_threshold", 68u, true, SND_CSV_NORM_BYTE, 0.0, 1.0, 0u, NULL },
  { "max_priority_threshold", 69u, true, SND_CSV_NORM_BYTE, 0.0, 1.0, 0u, NULL },
  { "team_vol_mod", 48u, true, SND_CSV_DBSPL, 0.0, 100.0, 0u, NULL },
  { "team_pitch_mod", 54u, true, SND_CSV_CENTS, -2400.0, 1200.0, 0u, NULL },
  { "occlusion_level", 71u, true, SND_CSV_NORM_BYTE, 0.0, 1.0, 0u, NULL },
  { "occlusion_wet_dry", 72u, true, SND_CSV_NORM_BYTE, 0.0, 1.0, 0u, NULL },
  { "move_time", 36u, true, SND_CSV_USHORT, 0.0, 65535.0, 0u, NULL },
  { "duck", 24u, true, SND_CSV_HASH, 0.0, 0.0, 0u, NULL },
  { "min_priority", 73u, true, SND_CSV_BYTE, 0.0, 128.0, 0u, NULL },
  { "max_priority", 74u, true, SND_CSV_BYTE, 0.0, 128.0, 0u, NULL },
  { "limit_count", 80u, true, SND_CSV_BYTE, 0.0, 128.0, 0u, NULL },
  { "entity_limit_count", 81u, true, SND_CSV_BYTE, 0.0, 128.0, 0u, NULL },
  {
    "volume_falloff_curve",
    76u,
    true,
    SND_CSV_ENUM_BYTE,
    0.0,
    0.0,
    0u,
    CURVES
  },
  {
    "reverb_falloff_curve",
    77u,
    true,
    SND_CSV_ENUM_BYTE,
    0.0,
    0.0,
    0u,
    CURVES
  },
  {
    "volume_min_falloff_curve",
    78u,
    true,
    SND_CSV_ENUM_BYTE,
    0.0,
    0.0,
    0u,
    CURVES
  },
  {
    "reverb_min_falloff_curve",
    79u,
    true,
    SND_CSV_ENUM_BYTE,
    0.0,
    0.0,
    0u,
    CURVES
  },
  { "pan", 75u, true, SND_CSV_ENUM_BYTE, 0.0, 0.0, 0u, PANS },
  { "snapshot", 82u, true, SND_CSV_ENUM_BYTE, 0.0, 0.0, 0u, SNAPSHOT_GROUPS },
  { "context_type", 28u, true, SND_CSV_HASH, 0.0, 0.0, 0u, NULL },
  { "context_value", 32u, true, SND_CSV_HASH, 0.0, 0.0, 0u, NULL },
  { "limit_type", 20u, true, SND_CSV_ENUM_BITS, 2.0, 25.0, 0u, LIMIT_TYPES },
  {
    "entity_limit_type",
    20u,
    true,
    SND_CSV_ENUM_BITS,
    2.0,
    27.0,
    0u,
    LIMIT_TYPES
  },
  {
    "randomize_type",
    20u,
    true,
    SND_CSV_FLAG_BITS,
    3.0,
    29.0,
    0u,
    RANDOMIZE_TYPES
  },
  { "move_type", 20u, true, SND_CSV_ENUM_BITS, 3.0, 22.0, 0u, MOVE_TYPES },
  { "type", 20u, true, SND_CSV_ENUM_BITS, 2.0, 14.0, 0u, LOAD_TYPES },
  { "group", 20u, true, SND_CSV_ENUM_BITS, 6.0, 16.0, 0u, GROUPS },
  { "real_delay", 20u, true, SND_CSV_ENUM_BITS, 1.0, 2.0, 0u, NO_YES },
  { "distance_lpf", 20u, true, SND_CSV_ENUM_BITS, 1.0, 3.0, 0u, NO_YES },
  { "doppler", 20u, true, SND_CSV_ENUM_BITS, 1.0, 4.0, 0u, NO_YES },
  { "is_big", 20u, true, SND_CSV_ENUM_BITS, 1.0, 5.0, 0u, NO_YES },
  { "loop", 20u, true, SND_CSV_ENUM_BITS, 1.0, 0.0, 0u, LOOP_TYPES },
  {
    "spatialized",
    20u,
    true,
    SND_CSV_ENUM_BITS,
    1.0,
    1.0,
    0u,
    SPATIALIZED_TYPES
  },
  { "futz", 20u, true, SND_CSV_ENUM_BITS, 1.0, 6.0, 0u, NO_YES },
  { "music", 20u, true, SND_CSV_ENUM_BITS, 1.0, 8.0, 0u, NO_YES },
  { "timescale", 20u, true, SND_CSV_ENUM_BITS, 1.0, 10.0, 0u, NO_YES },
  { "pause", 20u, true, SND_CSV_ENUM_BITS, 1.0, 7.0, 0u, NO_YES },
  { "stop_on_death", 20u, true, SND_CSV_ENUM_BITS, 1.0, 9.0, 0u, NO_YES },
  { "bus", 20u, true, SND_CSV_ENUM_BITS, 2.0, 12.0, 0u, BUS_IDS },
  { "voice_limit", 20u, true, SND_CSV_ENUM_BITS, 1.0, 11.0, 0u, NO_YES }
};

const snd_csv_entry_t SND_SNAPSHOT_GROUP_FIELDS[1] =
{ { "name", 0u, true, SND_CSV_STRING, 0.0, 0.0, 32u, NULL } };

const snd_csv_entry_t SND_CONTEXT_FIELDS[9] =
{
  { "type", 0u, true, SND_CSV_HASH, 0.0, 0.0, 0u, NULL },
  { "value0", 8u, true, SND_CSV_HASH, 0.0, 0.0, 0u, NULL },
  { "value1", 12u, true, SND_CSV_HASH, 0.0, 0.0, 0u, NULL },
  { "value2", 16u, true, SND_CSV_HASH, 0.0, 0.0, 0u, NULL },
  { "value3", 20u, true, SND_CSV_HASH, 0.0, 0.0, 0u, NULL },
  { "value4", 24u, true, SND_CSV_HASH, 0.0, 0.0, 0u, NULL },
  { "value5", 28u, true, SND_CSV_HASH, 0.0, 0.0, 0u, NULL },
  { "value6", 32u, true, SND_CSV_HASH, 0.0, 0.0, 0u, NULL },
  { "value7", 36u, true, SND_CSV_HASH, 0.0, 0.0, 0u, NULL }
};

snd_csv_entry_t SND_SNAPSHOT_FIELDS[73] =
{
  { "name", 0u, true, SND_CSV_STRING, 0.0, 0.0, 32u, NULL },
  { "name", 32u, true, SND_CSV_HASH, 0.0, 0.0, 32u, NULL },
  { "occlusion", 36u, true, SND_CSV_STRING, 0.0, 0.0, 32u, NULL },
  { "occlusion", 68u, true, SND_CSV_HASH, 0.0, 0.0, 32u, NULL },
  { "fadeIn", 72u, true, SND_CSV_FLOAT, 0.0, 30.0, 0u, NULL },
  { "fadeInCurve", 84u, true, SND_CSV_HASH, 0.0, 0.0, 0u, NULL },
  { "fadeOut", 76u, true, SND_CSV_FLOAT, 0.0, 30.0, 0u, NULL },
  { "fadeOutCurve", 88u, true, SND_CSV_HASH, 0.0, 0.0, 0u, NULL },
  { "distance", 80u, true, SND_CSV_FLOAT, 0.0, 65535.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL },
  { NULL, 0u, false, SND_CSV_STRING, 0.0, 0.0, 0u, NULL }
};

const snd_csv_entry_t SND_RADVERB_FIELDS[17] =
{
  { "name", 0u, true, SND_CSV_STRING, 0.0, 0.0, 32u, NULL },
  { "name", 32u, true, SND_CSV_HASH, 0.0, 0.0, 0u, NULL },
  { "smoothing", 36u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "earlyTime", 40u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "lateTime", 44u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "earlyGain", 48u, true, SND_CSV_FLOAT, 0.0, 4.0, 0u, NULL },
  { "lateGain", 52u, true, SND_CSV_FLOAT, 0.0, 4.0, 0u, NULL },
  { "earlyLpf", 60u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "lateLpf", 64u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "inputLpf", 68u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "dampLpf", 72u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "wallReflect", 76u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "dryGain", 80u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "earlySize", 84u, true, SND_CSV_FLOAT, 0.1, 16.0, 0u, NULL },
  { "lateSize", 88u, true, SND_CSV_FLOAT, 0.1, 16.0, 0u, NULL },
  { "diffusion", 92u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "returnGain", 56u, true, SND_CSV_FLOAT, 0.0, 4.0, 0u, NULL }
};

const snd_csv_entry_t SND_MASTER_FIELDS[37] =
{
  { "name", 0u, true, SND_CSV_STRING, 0.0, 0.0, 32u, NULL },
  { "name", 32u, true, SND_CSV_HASH, 0.0, 0.0, 0u, NULL },
  { "notchE", 36u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "notchG", 40u, true, SND_CSV_FLOAT, -60.0, 12.0, 0u, NULL },
  { "notchF", 44u, true, SND_CSV_FLOAT, 20.0, 20000.0, 0u, NULL },
  { "notchQ", 48u, true, SND_CSV_FLOAT, 0.1, 16.0, 0u, NULL },
  { "lowE", 52u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "lowG", 56u, true, SND_CSV_FLOAT, -60.0, 12.0, 0u, NULL },
  { "lowF", 60u, true, SND_CSV_FLOAT, 20.0, 20000.0, 0u, NULL },
  { "lowQ", 64u, true, SND_CSV_FLOAT, 0.1, 16.0, 0u, NULL },
  { "peak1E", 68u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "peak1G", 72u, true, SND_CSV_FLOAT, -60.0, 12.0, 0u, NULL },
  { "peak1F", 76u, true, SND_CSV_FLOAT, 20.0, 20000.0, 0u, NULL },
  { "peak1Q", 80u, true, SND_CSV_FLOAT, 0.1, 16.0, 0u, NULL },
  { "peak2E", 84u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "peak2G", 88u, true, SND_CSV_FLOAT, -60.0, 12.0, 0u, NULL },
  { "peak2F", 92u, true, SND_CSV_FLOAT, 20.0, 20000.0, 0u, NULL },
  { "peak2Q", 96u, true, SND_CSV_FLOAT, 0.1, 16.0, 0u, NULL },
  { "hiE", 100u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "hiG", 104u, true, SND_CSV_FLOAT, -60.0, 12.0, 0u, NULL },
  { "hiF", 108u, true, SND_CSV_FLOAT, 20.0, 20000.0, 0u, NULL },
  { "hiQ", 112u, true, SND_CSV_FLOAT, 0.1, 16.0, 0u, NULL },
  { "eqG", 116u, true, SND_CSV_FLOAT, 0.0, 16.0, 0u, NULL },
  { "compE", 120u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "compPG", 124u, true, SND_CSV_FLOAT, 0.0, 16.0, 0u, NULL },
  { "compMG", 128u, true, SND_CSV_FLOAT, 0.0, 16.0, 0u, NULL },
  { "compT", 132u, true, SND_CSV_FLOAT, 0.029999999, 1.0, 0u, NULL },
  { "compR", 136u, true, SND_CSV_FLOAT, 0.1, 100.0, 0u, NULL },
  { "compTA", 140u, true, SND_CSV_FLOAT, 0.0, 4.0, 0u, NULL },
  { "compTR", 144u, true, SND_CSV_FLOAT, 0.1, 4.0, 0u, NULL },
  { "limitE", 148u, true, SND_CSV_FLOAT, 0.0, 1.0, 0u, NULL },
  { "limitPG", 152u, true, SND_CSV_FLOAT, 0.0, 16.0, 0u, NULL },
  { "limitMG", 156u, true, SND_CSV_FLOAT, 0.0, 16.0, 0u, NULL },
  { "limitT", 160u, true, SND_CSV_FLOAT, 0.029999999, 1.0, 0u, NULL },
  { "limitR", 164u, true, SND_CSV_FLOAT, 0.1, 100.0, 0u, NULL },
  { "limitTA", 168u, true, SND_CSV_FLOAT, 0.0, 4.0, 0u, NULL },
  { "limitTR", 172u, true, SND_CSV_FLOAT, 0.1, 4.0, 0u, NULL }
};

const snd_csv_entry_t *SND_TABLE_METADATA[9] =
{
  SND_ALIAS_FIELDS,
  SND_GROUP_FIELDS,
  SND_CURVE_FIELDS,
  SND_PAN_FIELDS,
  SND_SNAPSHOT_GROUP_FIELDS,
  SND_SNAPSHOT_FIELDS,
  SND_CONTEXT_FIELDS,
  SND_RADVERB_FIELDS,
  SND_MASTER_FIELDS
};

void __cdecl SND_ParseInit(
                unsigned int groupCount,
                const snd_group *groups,
                unsigned int curveCount,
                const snd_curve *curves,
                unsigned int panCount,
                const snd_pan *pans,
                unsigned int snapshotGroupCount,
                const snd_snapshot_group *snapshotGroups)
{
    unsigned int n; // [esp+0h] [ebp-14h]
    unsigned int i; // [esp+4h] [ebp-10h]
    unsigned int m; // [esp+8h] [ebp-Ch]
    unsigned int k; // [esp+Ch] [ebp-8h]
    unsigned int j; // [esp+10h] [ebp-4h]

    if ( groupCount >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_db.cpp",
                    645,
                    0,
                    "%s",
                    "groupCount < SND_MAX_GROUPS") )
    {
        __debugbreak();
    }
    if ( curveCount >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_db.cpp",
                    646,
                    0,
                    "%s",
                    "curveCount < SND_MAX_CURVES") )
    {
        __debugbreak();
    }
    memset(GROUPS, 0, sizeof(GROUPS));
    for ( j = 0; j < groupCount; ++j )
        GROUPS[j] = groups[j].name;
    memset(PANS, 0, sizeof(PANS));
    for ( k = 0; k < panCount; ++k )
        PANS[k] = pans[k].name;
    memset(CURVES, 0, sizeof(CURVES));
    for ( m = 0; m < curveCount; ++m )
        CURVES[m] = curves[m].name;
    memset(&SND_SNAPSHOT_FIELDS[9], 0, 0x800u);
    for ( i = 0; i < snapshotGroupCount; ++i )
    {
        if ( &snapshotGroups[i] && snapshotGroups[i].name[0] )
        {
            SND_SNAPSHOT_FIELDS[i + 9].name = snapshotGroups[i].name;
            SND_SNAPSHOT_FIELDS[i + 9].offset = 4 * i + 92;
            SND_SNAPSHOT_FIELDS[i + 9].required = 1;
            SND_SNAPSHOT_FIELDS[i + 9].type = SND_CSV_FLOAT;
            SND_SNAPSHOT_FIELDS[i + 9].minimum = 0.0f;
            SND_SNAPSHOT_FIELDS[i + 9].maximum = 1.0f;
            SND_SNAPSHOT_FIELDS[i + 9].length = 0;
            SND_SNAPSHOT_FIELDS[i + 9].enum_ids = 0;
        }
        else
        {
            SND_SNAPSHOT_FIELDS[i + 9].name = 0;
        }
    }
    memset(SNAPSHOT_GROUPS, 0, sizeof(SNAPSHOT_GROUPS));
    for ( n = 0; n < snapshotGroupCount; ++n )
        SNAPSHOT_GROUPS[n] = snapshotGroups[n].name;
}

void __cdecl SND_AliasGetFileName(const snd_alias_t *alias, char *filename, int size)
{
    if ( !alias && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_db.cpp", 735, 0, "%s", "alias") )
        __debugbreak();
    if ( !alias->soundFile
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_db.cpp", 736, 0, "%s", "alias->soundFile") )
    {
        __debugbreak();
    }
    if ( alias->soundFile->type == 1 )
        I_strncpyz(filename, alias->soundFile->u.loadSnd->name, size);
    else
        Com_sprintf(filename, size, "%s", alias->soundFile->u.loadSnd->name);
}

