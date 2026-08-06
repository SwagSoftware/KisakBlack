#include "snd_dvar.h"
#include "snd_utils.h"
#include <universal/assertive.h>

const dvar_t *snd_draw3D;
const dvar_t *snd_slaveFadeInTime;
const dvar_t *snd_slaveFadeOutTime;
const dvar_t *snd_levelFadeTime;
const dvar_t *snd_realDelay;
const dvar_t *snd_omnidirectionalPercentage;
const dvar_t *snd_dopplerScale;
const dvar_t *snd_dopplerRate;
const dvar_t *snd_flux_separation;
const dvar_t *snd_playing_priority_boost;
const dvar_t *snd_dlpf_max_distance;
const dvar_t *snd_dlpf_min_distance;
const dvar_t *snd_dlpf_attenuation;
const dvar_t *snd_losOcclusion;
const dvar_t *snd_occlusion_attenuation;
const dvar_t *snd_occlusion_rate;
const dvar_t *snd_occlusion_cull_distance;
const dvar_t *snd_water_occlusion_attenuation;
const dvar_t *snd_water_occlusion_distance;
const dvar_t *snd_water_occlusion_minimum;
const dvar_t *snd_water_pitch_distance;
const dvar_t *snd_water_pitch_amount;
const dvar_t *snd_water_omni_distance;
const dvar_t *snd_water_omni_amount;
const dvar_t *snd_skip_muted_sounds;
const dvar_t *snd_pitch_timescale;
const dvar_t *snd_menu_voice;
const dvar_t *snd_menu_music;
const dvar_t *snd_menu_sfx;
const dvar_t *snd_menu_master;
const dvar_t *snd_menu_cinematic;
const dvar_t *snd_menu_speaker_setup;
const dvar_t *snd_menu_listen_level;
const dvar_t *snd_menu_left_channel;
const dvar_t *snd_menu_right_channel;
const dvar_t *snd_menu_center_channel;
const dvar_t *snd_menu_left_surround;
const dvar_t *snd_menu_right_surround;
const dvar_t *snd_menu_lfe;
const dvar_t *snd_global_pitch;
const dvar_t *snd_enable_pitch;
const dvar_t *snd_futz;
const dvar_t *snd_futz_min;
const dvar_t *snd_stereo_3d;
const dvar_t *snd_autosim_window;
const dvar_t *snd_isbig_distance;
const dvar_t *snd_hdr;
const dvar_t *snd_hdr_minimum_level;
const dvar_t *snd_hdr_rate;
const dvar_t *snd_hdr_channel_threshold;
const dvar_t *snd_hdr_minimum_priority;
const dvar_t *snd_hdr_maximum_priority;
const dvar_t *snd_priority_window;
const dvar_t *snd_priority_rate;
const dvar_t *snd_dsp_drc;
const dvar_t *snd_dsp_drc_threshold;
const dvar_t *snd_dsp_drc_ratio;
const dvar_t *snd_always_log;
const dvar_t *snd_timescale_filter;
const dvar_t *snd_dry_scale;
const dvar_t *snd_wet_scale;
const dvar_t *snd_test_verb;
const dvar_t *snd_reverb_proximity_distance;
const dvar_t *snd_radverb_matrix;
const dvar_t *snd_reverb_override;
const dvar_t *snd_master_override;
const dvar_t *snd_assert_on_play;
const dvar_t *snd_assert_on_stop;
const dvar_t *snd_assert_on_enqueue;
const dvar_t *snd_debugReplace;
const dvar_t *snd_debugAlias;
const dvar_t *snd_entityRandomize;
const dvar_t *snd_touchStreamFilesOnLoad;
const dvar_t *snd_debug_snapshot;
const dvar_t *snd_boat_current_rpm;
const dvar_t *snd_boat_rpm_scalar;
const dvar_t *snd_boat_using_lerp_rpm;
const dvar_t *snd_boat_lerp_rpm;
const dvar_t *snd_boat_engine_off;
const dvar_t *snd_throttle_time_held_check;
const dvar_t *snd_throttle_reduce_vol;
const dvar_t *snd_boat_rpm_idle_fout_start;
const dvar_t *snd_boat_rpm_idle_end;
const dvar_t *snd_boat_rpm_low_start;
const dvar_t *snd_boat_rpm_low_fin_end;
const dvar_t *snd_boat_rpm_low_fout_start;
const dvar_t *snd_boat_rpm_low_end;
const dvar_t *snd_boat_rpm_med_start;
const dvar_t *snd_boat_rpm_med_fin_end;
const dvar_t *snd_boat_rpm_med_fout_start;
const dvar_t *snd_boat_rpm_med_end;
const dvar_t *snd_boat_rpm_high_start;
const dvar_t *snd_boat_rpm_high_fin_end;
const dvar_t *snd_boat_pitch_idle_min;
const dvar_t *snd_boat_pitch_idle_max;
const dvar_t *snd_boat_pitch_low_min;
const dvar_t *snd_boat_pitch_low_max;
const dvar_t *snd_boat_pitch_med_min;
const dvar_t *snd_boat_pitch_med_max;
const dvar_t *snd_boat_pitch_high_min;
const dvar_t *snd_boat_pitch_high_max;
const dvar_t *snd_boat_water_slow_min;
const dvar_t *snd_boat_water_idle_max;
const dvar_t *snd_boat_water_fast_min;
const dvar_t *snd_boat_water_slow_max;
const dvar_t *snd_boat_water_pitch_max;
const dvar_t *snd_solo_alias_substring;
const dvar_t *snd_mute_alias_substring;
const dvar_t *snd_minigun_loop_start;
const dvar_t *snd_minigun_pitch_start;
const dvar_t *snd_minigun_pitch_end;
const dvar_t *snd_start_alias;
const dvar_t *snd_stop_alias;
const dvar_t *snd_proximity_enable;
const dvar_t *snd_proximity_min_dist;
const dvar_t *snd_proximity_max_dist;
const dvar_t *snd_proximity_max_late_cut;
const dvar_t *snd_proximity_filter;
const dvar_t *snd_auto_random;
const dvar_t *snd_trace_master;
const dvar_t *snd_trace_reverb;
const dvar_t *snd_trace_voice;
const dvar_t *snd_futz_blend;
const dvar_t *snd_futz_bpf_f;
const dvar_t *snd_futz_bpf_q;
const dvar_t *snd_futz_ls_g;
const dvar_t *snd_futz_ls_f;
const dvar_t *snd_futz_ls_q;
const dvar_t *snd_futz_distortion;
const dvar_t *snd_futz_preg;
const dvar_t *snd_futz_postg;
const dvar_t *snd_futz_th;
const dvar_t *snd_futz_tg;
const dvar_t *snd_futz_clip_pre;
const dvar_t *snd_futz_clip_post;
const dvar_t *snd_futz_force;
const dvar_t *snd_dsp_futz;
const dvar_t *snd_pan_filter;
const dvar_t *snd_ps3_vol_occlusion_attenuation_dry;
const dvar_t *snd_ps3_vol_occlusion_attenuation_wet;
const dvar_t *snd_speakerConfiguration;

const char *snd_speakerConfigurationStrings[32];

const char *snd_draw3DNames[5] =
{ "Off", "Targets", "Names", "Verbose", NULL };

void __cdecl SND_InitDvar()
{
    unsigned int i; // [esp+14h] [ebp-4h]

    snd_draw3D = _Dvar_RegisterEnum("snd_draw3D", snd_draw3DNames, 0, 0x80u, "Draw the position and info of world sounds");
    snd_slaveFadeInTime = _Dvar_RegisterFloat(
                                                    "snd_slaveFadeInTime",
                                                    0.15000001,
                                                    0.0,
                                                    5.0,
                                                    0x80u,
                                                    "the time in seconds for slave fade in");
    snd_slaveFadeOutTime = _Dvar_RegisterFloat(
                                                     "snd_slaveFadeOutTime",
                                                     0.25,
                                                     0.0,
                                                     5.0,
                                                     0x80u,
                                                     "the time in seconds for slave fade out");
    snd_levelFadeTime = _Dvar_RegisterInt(
                                                "snd_levelFadeTime",
                                                250,
                                                0,
                                                5000,
                                                0x80u,
                                                "The amount of time in milliseconds for all audio to fade in at the start of a level");
    snd_realDelay = _Dvar_RegisterFloat("snd_realDelay", 0.5, 0.0, 10.0, 0x80u, "turn on 'realistic' sound delay");
    snd_omnidirectionalPercentage = _Dvar_RegisterFloat(
                                                                        "snd_omnidirectionalPercentage",
                                                                        0.0,
                                                                        0.0,
                                                                        1.0,
                                                                        0x80u,
                                                                        "Percentage of sound volume that is 'directionless'");
    snd_dopplerScale = _Dvar_RegisterFloat("snd_dopplerScale", 10.0, -200.0, 100.0, 0x80u, "global doppler ratio");
    snd_dopplerRate = _Dvar_RegisterFloat("snd_dopplerRate", 2.75, 0.0099999998, 20.0, 0x80u, "global doppler lerp rate");
    snd_flux_separation = _Dvar_RegisterFloat(
                                                    "snd_flux_separation",
                                                    0.30000001,
                                                    0.0,
                                                    1.0,
                                                    0x80u,
                                                    "separation of L/R flux elements, 0 none, 1 180 deg, not linear in angle");
    snd_playing_priority_boost = _Dvar_RegisterFloat(
                                                                 "snd_playing_priority_boost",
                                                                 3.0,
                                                                 0.0,
                                                                 100.0,
                                                                 0x80u,
                                                                 "priority penalty for new sounds");
    snd_dlpf_max_distance = _Dvar_RegisterFloat(
                                                        "snd_dlpf_max_distance",
                                                        3000.0,
                                                        100.0,
                                                        10000.0,
                                                        0x80u,
                                                        "max distance for distance lpf");
    snd_dlpf_min_distance = _Dvar_RegisterFloat(
                                                        "snd_dlpf_min_distance",
                                                        800.0,
                                                        100.0,
                                                        10000.0,
                                                        0x80u,
                                                        "min distance for distance lpf");
    snd_dlpf_attenuation = _Dvar_RegisterFloat(
                                                     "snd_dlpf_attenuation",
                                                     -30.0,
                                                     -60.0,
                                                     0.0,
                                                     0x80u,
                                                     "attenuation for distance lpf");
    snd_losOcclusion = _Dvar_RegisterBool("snd_losOcclusion", 1, 0x4001u, "occlusion based off of LOS");
    snd_occlusion_attenuation = _Dvar_RegisterFloat(
                                                                "snd_occlusion_attenuation",
                                                                -35.0,
                                                                -80.0,
                                                                0.0,
                                                                0x80u,
                                                                "maximum occlusion attenuation");
    snd_occlusion_rate = _Dvar_RegisterFloat("snd_occlusion_rate", 6.0, 0.0099999998, 100.0, 0x80u, "snd occlusion rate");
    snd_occlusion_cull_distance = _Dvar_RegisterFloat(
                                                                    "snd_occlusion_cull_distance",
                                                                    2000.0,
                                                                    0.0,
                                                                    10000.0,
                                                                    0x80u,
                                                                    "distance at which we don't do occlusion at start");
    snd_water_occlusion_attenuation = _Dvar_RegisterFloat(
                                                                            "snd_water_occlusion_attenuation",
                                                                            -30.0,
                                                                            -80.0,
                                                                            0.0,
                                                                            0x80u,
                                                                            "in water attenuation");
    snd_water_occlusion_distance = _Dvar_RegisterFloat(
                                                                     "snd_water_occlusion_distance",
                                                                     60.0,
                                                                     1.0,
                                                                     10000.0,
                                                                     0x80u,
                                                                     "max distance from surface for water occlusion");
    snd_water_occlusion_minimum = _Dvar_RegisterFloat(
                                                                    "snd_water_occlusion_minimum",
                                                                    -8.0,
                                                                    -1000.0,
                                                                    1000.0,
                                                                    0x80u,
                                                                    "min distance from surface for water occlusion");
    snd_water_pitch_distance = _Dvar_RegisterFloat(
                                                             "snd_water_pitch_distance",
                                                             30.0,
                                                             0.0,
                                                             1000.0,
                                                             0x80u,
                                                             "max distance for underwater pitch changes");
    snd_water_pitch_amount = _Dvar_RegisterFloat(
                                                         "snd_water_pitch_amount",
                                                         0.89999998,
                                                         0.0,
                                                         2.0,
                                                         0x80u,
                                                         "pitch amount at max distance underwater");
    snd_water_omni_distance = _Dvar_RegisterFloat(
                                                            "snd_water_omni_distance",
                                                            60.0,
                                                            0.0,
                                                            1000.0,
                                                            0x80u,
                                                            "max distance for undewater omni");
    snd_water_omni_amount = _Dvar_RegisterFloat(
                                                        "snd_water_omni_amount",
                                                        0.5,
                                                        0.0,
                                                        1.0,
                                                        0x80u,
                                                        "omni amount at max distance underwater");
    snd_skip_muted_sounds = _Dvar_RegisterBool("snd_skip_muted_sounds", 0, 0x80u, "skip muted sounds");
    snd_pitch_timescale = _Dvar_RegisterFloat("snd_pitch_timescale", 0.25, 0.0, 2.0, 0x80u, "pitch scalar by timescale");
    snd_menu_voice = _Dvar_RegisterFloat("snd_menu_voice", 1.0, 0.0, 1.0, 1u, "options menu voice");
    snd_menu_music = _Dvar_RegisterFloat("snd_menu_music", 1.0, 0.0, 1.0, 1u, "options menu music");
    snd_menu_sfx = _Dvar_RegisterFloat("snd_menu_sfx", 1.0, 0.0, 1.0, 1u, "options menu sfx");
    snd_menu_master = _Dvar_RegisterFloat("snd_menu_master", 1.0, 0.0, 1.0, 1u, "options menu master");
    snd_menu_cinematic = _Dvar_RegisterFloat("snd_menu_cinematic", 1.0, 0.0, 1.0, 1u, "Scales the volume of Bink videos.");
    snd_menu_speaker_setup = _Dvar_RegisterInt("snd_menu_speaker_setup", 0, 0, 2, 1u, "options menu speaker setup");
    snd_menu_listen_level = _Dvar_RegisterInt("snd_menu_listen_level", 0, 0, 1, 1u, "options menu listening level");
    snd_menu_left_channel = _Dvar_RegisterFloat("snd_menu_left_channel", 1.0, 0.0, 1.0, 1u, "options menu left channel");
    snd_menu_right_channel = _Dvar_RegisterFloat(
                                                         "snd_menu_right_channel",
                                                         1.0,
                                                         0.0,
                                                         1.0,
                                                         1u,
                                                         "options menu right channel");
    snd_menu_center_channel = _Dvar_RegisterFloat(
                                                            "snd_menu_center_channel",
                                                            1.0,
                                                            0.0,
                                                            1.0,
                                                            1u,
                                                            "options menu center channel");
    snd_menu_left_surround = _Dvar_RegisterFloat(
                                                         "snd_menu_left_surround",
                                                         1.0,
                                                         0.0,
                                                         1.0,
                                                         1u,
                                                         "options menu left surround");
    snd_menu_right_surround = _Dvar_RegisterFloat(
                                                            "snd_menu_right_surround",
                                                            1.0,
                                                            0.0,
                                                            1.0,
                                                            1u,
                                                            "options menu right surround");
    snd_menu_lfe = _Dvar_RegisterFloat("snd_menu_lfe", 1.0, 0.0, 1.0, 1u, "options menu lfe");
    snd_global_pitch = _Dvar_RegisterFloat("snd_global_pitch", 1.0, 0.0, 2.0, 0x80u, "global pitch");
    snd_enable_pitch = _Dvar_RegisterBool("snd_enable_pitch", 1, 0x80u, "use pitch variation");
    snd_futz = _Dvar_RegisterFloat("snd_futz", 0.0, 0.0, 1.0, 0x80u, "futz");
    snd_futz_min = _Dvar_RegisterFloat("snd_futz_min", 900.0, 0.0, 1000.0, 0x80u, "futz min");
    snd_stereo_3d = _Dvar_RegisterBool("snd_stereo_3d", 1, 0x80u, "pan 3d stereo assets to alternate speakers");
    snd_autosim_window = _Dvar_RegisterBool("snd_autosim_window", 1, 0x80u, "autosim window masking");
    snd_isbig_distance = _Dvar_RegisterFloat(
                                                 "snd_isbig_distance",
                                                 1000.0,
                                                 0.0,
                                                 10000.0,
                                                 0x80u,
                                                 "distance where isbig doesn't make a difference in occlusion");
    snd_hdr = _Dvar_RegisterBool("snd_hdr", 0, 0x80u, "HDR audio");
    snd_hdr_minimum_level = _Dvar_RegisterFloat("snd_hdr_minimum_level", 0.0, 0.0, 1.0, 0x80u, "hdr minimum attenuation");
    snd_hdr_rate = _Dvar_RegisterFloat("snd_hdr_rate", 5.0, 0.5, 20.0, 0x80u, "hdr lerp rate");
    snd_hdr_channel_threshold = _Dvar_RegisterFloat(
                                                                "snd_hdr_channel_threshold",
                                                                0.5,
                                                                0.0,
                                                                0.89999998,
                                                                0x80u,
                                                                "percent of channels active when hdr kicks in");
    snd_hdr_minimum_priority = _Dvar_RegisterFloat(
                                                             "snd_hdr_minimum_priority",
                                                             60.0,
                                                             0.0,
                                                             100.0,
                                                             0x80u,
                                                             "sounds less than this are silent");
    snd_hdr_maximum_priority = _Dvar_RegisterFloat(
                                                             "snd_hdr_maximum_priority",
                                                             80.0,
                                                             0.0,
                                                             100.0,
                                                             0x80u,
                                                             "sounds more than this are at normal volume");
    snd_priority_window = _Dvar_RegisterFloat(
                                                    "snd_priority_window",
                                                    35.0,
                                                    0.0,
                                                    100.0,
                                                    0x80u,
                                                    "width of priority window for sounds to play");
    snd_priority_rate = _Dvar_RegisterFloat(
                                                "snd_priority_rate",
                                                40.0,
                                                0.0,
                                                100.0,
                                                0x80u,
                                                "interpolation rate for minimum priority");
    snd_dsp_drc = _Dvar_RegisterBool("snd_dsp_drc", 0, 0x80u, "enable DRC");
    snd_dsp_drc_threshold = _Dvar_RegisterFloat(
                                                        "snd_dsp_drc_threshold",
                                                        -3.0,
                                                        -60.0,
                                                        0.0,
                                                        0x80u,
                                                        "threshold for master limiter");
    snd_dsp_drc_ratio = _Dvar_RegisterFloat("snd_dsp_drc_ratio", 20.0, 1.0, 500.0, 0x80u, "ratio for master limiter");
    snd_always_log = _Dvar_RegisterBool("snd_always_log", 0, 0x80u, "enable missing sound spam");
    snd_timescale_filter = _Dvar_RegisterFloat(
                                                     "snd_timescale_filter",
                                                     0.07,
                                                     0.0,
                                                     1.0,
                                                     0x80u,
                                                     "filter coef for timescale fade");
    snd_dry_scale = _Dvar_RegisterFloat("snd_dry_scale", 1.0, 0.0, 1.0, 0x80u, "scalar value for dry signal path");
    snd_wet_scale = _Dvar_RegisterFloat("snd_wet_scale", 1.0, 0.0, 1.0, 0x80u, "scalar value for wet signal path");
    snd_test_verb = _Dvar_RegisterFloat("snd_test_verb", 1.0, 0.0, 1.0, 0x80u, "test verb wetdry");
    snd_reverb_proximity_distance = _Dvar_RegisterFloat(
                                                                        "snd_reverb_proximity_distance",
                                                                        0.0,
                                                                        0.0,
                                                                        20000.0,
                                                                        0x80u,
                                                                        "max dist for reverb proximity effect");
    snd_radverb_matrix = _Dvar_RegisterInt("snd_radverb_matrix", 0, 0, 2, 0x80u, "radverb delay matrix selection");
    snd_reverb_override = _Dvar_RegisterString(
                                                    "snd_reverb_override",
                                                    (char *)"",
                                                    0x80u,
                                                    "overide current verb");
    snd_master_override = _Dvar_RegisterString(
                                                    "snd_master_override",
                                                    (char *)"",
                                                    0x80u,
                                                    "overide current master");
    snd_assert_on_play = _Dvar_RegisterString(
                                                 "snd_assert_on_play",
                                                 (char *)"",
                                                 0x80u,
                                                 "assert if played alias");
    snd_assert_on_stop = _Dvar_RegisterString(
                                                 "snd_assert_on_stop",
                                                 (char *)"",
                                                 0x80u,
                                                 "assert if stopped alias");
    snd_assert_on_enqueue = _Dvar_RegisterString(
                                                        "snd_assert_on_enqueue",
                                                        (char *)"",
                                                        0x80u,
                                                        "assert if played alias is put into q");
    snd_debugReplace = _Dvar_RegisterBool(
                                             "snd_debugReplace",
                                             0,
                                             0x80u,
                                             "Print out information about when we stop a playing sound to play another");
    snd_debugAlias = _Dvar_RegisterString(
                                         "snd_debugAlias",
                                         (char *)"",
                                         0x80u,
                                         "Print out tracking information about a particular alias");
    snd_entityRandomize = _Dvar_RegisterBool("snd_entityRandomize", 1, 0x80u, "entity randomization");
    snd_touchStreamFilesOnLoad = _Dvar_RegisterBool(
                                                                 "snd_touchStreamFilesOnLoad",
                                                                 0,
                                                                 0x80u,
                                                                 "Check whether stream sound files exist while loading");
    snd_debug_snapshot = _Dvar_RegisterString(
                                                 "snd_debug_snapshot",
                                                 (char *)"",
                                                 0x80u,
                                                 "enable debug snapshot");
    snd_boat_current_rpm = _Dvar_RegisterFloat(
                                                     "snd_boat_current_rpm",
                                                     2.5,
                                                     0.0,
                                                     10.0,
                                                     0x80u,
                                                     "scalar value for boat engine");
    snd_boat_rpm_scalar = _Dvar_RegisterFloat("snd_boat_rpm_scalar", 1.25, 0.0, 10.0, 0x80u, "scalar value for boat rpm");
    snd_boat_using_lerp_rpm = _Dvar_RegisterBool("snd_boat_using_lerp_rpm", 0, 0x80u, "scalar value for boat rpm");
    snd_boat_lerp_rpm = _Dvar_RegisterFloat("snd_boat_lerp_rpm", 0.80000001, 0.0, 4.0, 0x80u, "scalar value for boat rpm");
    snd_boat_engine_off = _Dvar_RegisterFloat(
                                                    "snd_boat_engine_off",
                                                    0.0015,
                                                    0.0,
                                                    1.0,
                                                    0x80u,
                                                    "value for boat engine low/idle");
    snd_throttle_time_held_check = _Dvar_RegisterFloat(
                                                                     "snd_throttle_time_held_down",
                                                                     4000.0,
                                                                     0.0,
                                                                     10000.0,
                                                                     0x80u,
                                                                     "how long (ms) throttle is held before engine vol begins to decrease");
    snd_throttle_reduce_vol = _Dvar_RegisterFloat(
                                                            "snd_throttle_reduce_vol",
                                                            0.40000001,
                                                            0.0,
                                                            1.0,
                                                            0x80u,
                                                            "ammount to reduce engine vol");
    snd_boat_rpm_idle_fout_start = _Dvar_RegisterFloat(
                                                                     "snd_boat_rpm_idle_fout_start",
                                                                     0.17,
                                                                     0.0,
                                                                     2.0,
                                                                     0x80u,
                                                                     "rpm point where engine_idle begins to fade out");
    snd_boat_rpm_idle_end = _Dvar_RegisterFloat(
                                                        "snd_boat_rpm_idle_end",
                                                        0.46000001,
                                                        0.0,
                                                        2.0,
                                                        0x80u,
                                                        "rpm point where engine_idle ends");
    snd_boat_rpm_low_start = _Dvar_RegisterFloat(
                                                         "snd_boat_rpm_low_start",
                                                         0.05f,
                                                         0.0,
                                                         2.0,
                                                         0x80u,
                                                         "rpm point where engine_low start");
    snd_boat_rpm_low_fin_end = _Dvar_RegisterFloat(
                                                             "snd_boat_rpm_low_fin_end",
                                                             0.41999999,
                                                             0.0,
                                                             2.0,
                                                             0x80u,
                                                             "rpm point where engine_low fade in ends");
    snd_boat_rpm_low_fout_start = _Dvar_RegisterFloat(
                                                                    "snd_boat_rpm_low_fout_start",
                                                                    0.70999998,
                                                                    0.0,
                                                                    2.0,
                                                                    0x80u,
                                                                    "rpm point where engine_low fade out begins");
    snd_boat_rpm_low_end = _Dvar_RegisterFloat(
                                                     "snd_boat_rpm_low_end",
                                                     0.94999999,
                                                     0.0,
                                                     2.0,
                                                     0x80u,
                                                     "rpm point where engine_low ends");
    snd_boat_rpm_med_start = _Dvar_RegisterFloat(
                                                         "snd_boat_rpm_med_start",
                                                         0.36000001,
                                                         0.0,
                                                         2.0,
                                                         0x80u,
                                                         "rpm point where engine_medium start");
    snd_boat_rpm_med_fin_end = _Dvar_RegisterFloat(
                                                             "snd_boat_rpm_med_fin_end",
                                                             0.66000003,
                                                             0.0,
                                                             2.0,
                                                             0x80u,
                                                             "rpm point where engine_med fade in ends");
    snd_boat_rpm_med_fout_start = _Dvar_RegisterFloat(
                                                                    "snd_boat_rpm_med_fout_start",
                                                                    0.94999999,
                                                                    0.0,
                                                                    2.0,
                                                                    0x80u,
                                                                    "rpm point where engine_med fade out begins");
    snd_boat_rpm_med_end = _Dvar_RegisterFloat(
                                                     "snd_boat_rpm_med_end",
                                                     1.22,
                                                     0.0,
                                                     2.0,
                                                     0x80u,
                                                     "rpm point where engine_medium ends");
    snd_boat_rpm_high_start = _Dvar_RegisterFloat(
                                                            "snd_boat_rpm_high_start",
                                                            0.60000002,
                                                            0.0,
                                                            2.0,
                                                            0x80u,
                                                            "rpm point where engine_high start");
    snd_boat_rpm_high_fin_end = _Dvar_RegisterFloat(
                                                                "snd_boat_rpm_high_fin_end",
                                                                1.1900001,
                                                                0.0,
                                                                2.0,
                                                                0x80u,
                                                                "rpm point where engine_high fade in begins");
    snd_boat_pitch_idle_min = _Dvar_RegisterFloat(
                                                            "snd_boat_pitch_idle_min",
                                                            0.88999999,
                                                            0.0,
                                                            2.0,
                                                            0x80u,
                                                            "boat pitch low min");
    snd_boat_pitch_idle_max = _Dvar_RegisterFloat("snd_boat_pitch_idle_max", 1.13, 0.0, 2.0, 0x80u, "boat pitch low min");
    snd_boat_pitch_low_min = _Dvar_RegisterFloat(
                                                         "snd_boat_pitch_low_min",
                                                         0.94999999,
                                                         0.0,
                                                         2.0,
                                                         0x80u,
                                                         "boat pitch low min");
    snd_boat_pitch_low_max = _Dvar_RegisterFloat(
                                                         "snd_boat_pitch_low_max",
                                                         1.1900001,
                                                         0.0,
                                                         2.0,
                                                         0x80u,
                                                         "boat pitch low min");
    snd_boat_pitch_med_min = _Dvar_RegisterFloat(
                                                         "snd_boat_pitch_med_min",
                                                         0.83999997,
                                                         0.0,
                                                         2.0,
                                                         0x80u,
                                                         "boat pitch med min");
    snd_boat_pitch_med_max = _Dvar_RegisterFloat("snd_boat_pitch_med_max", 1.08, 0.0, 2.0, 0x80u, "boat pitch med max");
    snd_boat_pitch_high_min = _Dvar_RegisterFloat("snd_boat_pitch_high_min", 0.87, 0.0, 2.0, 0x80u, "boat pitch high min");
    snd_boat_pitch_high_max = _Dvar_RegisterFloat("snd_boat_pitch_high_max", 1.15, 0.0, 2.0, 0x80u, "boat pitch high max");
    snd_boat_water_slow_min = _Dvar_RegisterFloat("snd_boat_water_slow_min", 1.0, 1.0, 50.0, 0x80u, "boat pitch high max");
    snd_boat_water_idle_max = _Dvar_RegisterFloat(
                                                            "snd_boat_water_idle_max",
                                                            12.05,
                                                            1.0,
                                                            50.0,
                                                            0x80u,
                                                            "boat pitch high max");
    snd_boat_water_fast_min = _Dvar_RegisterFloat(
                                                            "snd_boat_water_fast_min",
                                                            19.42,
                                                            1.0,
                                                            50.0,
                                                            0x80u,
                                                            "boat pitch high max");
    snd_boat_water_slow_max = _Dvar_RegisterFloat(
                                                            "snd_boat_water_slow_max",
                                                            25.209999,
                                                            1.0,
                                                            50.0,
                                                            0x80u,
                                                            "boat pitch high max");
    snd_boat_water_pitch_max = _Dvar_RegisterFloat(
                                                             "snd_boat_water_pitch_max",
                                                             1.3099999,
                                                             0.5,
                                                             2.0,
                                                             0x80u,
                                                             "boat pitch high max");
    snd_solo_alias_substring = _Dvar_RegisterString(
                                                             "snd_solo_alias_substring",
                                                             (char *)"",
                                                             0x80u,
                                                             "mute alias with matching substring");
    snd_mute_alias_substring = _Dvar_RegisterString(
                                                             "snd_mute_alias_substring",
                                                             (char *)"",
                                                             0x80u,
                                                             "mute alias with not matching substring");
    snd_minigun_loop_start = _Dvar_RegisterFloat(
                                                         "snd_minigun_loop_start",
                                                         0.12,
                                                         0.0,
                                                         1.0,
                                                         0x80u,
                                                         "spin time elapse until loop sound starts playing");
    snd_minigun_pitch_start = _Dvar_RegisterFloat(
                                                            "snd_minigun_pitch_start",
                                                            0.69,
                                                            0.0,
                                                            2.0,
                                                            0x80u,
                                                            "pitch where the loop starts");
    snd_minigun_pitch_end = _Dvar_RegisterFloat(
                                                        "snd_minigun_pitch_end",
                                                        1.0,
                                                        0.0,
                                                        2.0,
                                                        0x80u,
                                                        "pitch where the loop ends");
    snd_start_alias = _Dvar_RegisterString("snd_start_alias", (char *)"", 0x80u, "start sound");
    snd_stop_alias = _Dvar_RegisterString("snd_stop_alias", (char *)"", 0x80u, "stop sound");
    snd_proximity_enable = _Dvar_RegisterBool("snd_proximity_enable", 1, 0x80u, "enable reverb prox");
    snd_proximity_min_dist = _Dvar_RegisterFloat("snd_proximity_min_dist", 20.0, 0.0, 10000.0, 0x80u, "");
    snd_proximity_max_dist = _Dvar_RegisterFloat("snd_proximity_max_dist", 400.0, 0.0, 10000.0, 0x80u, "");
    snd_proximity_max_late_cut = _Dvar_RegisterFloat(
                                                                 "snd_proximity_max_late_cut",
                                                                 -20.0,
                                                                 -60.0,
                                                                 0.0,
                                                                 0x80u,
                                                                 "");
    snd_proximity_filter = _Dvar_RegisterFloat("snd_proximity_filter", 0.1, 0.0, 1.0, 0x80u, "");
    snd_auto_random = _Dvar_RegisterFloat(
                                            "snd_auto_random",
                                            0.0,
                                            0.0,
                                            32.0,
                                            0x80u,
                                            "amt to randomize 3p apparent fire rates");
    snd_trace_master = _Dvar_RegisterBool("snd_trace_master", 0, 0x80u, "trace master dsp");
    snd_trace_reverb = _Dvar_RegisterBool("snd_trace_reverb", 0, 0x80u, "trace reverb dsp");
    snd_trace_voice = _Dvar_RegisterBool("snd_trace_voice", 0, 0x80u, "trace voice dsp");
    snd_futz_blend = _Dvar_RegisterFloat("snd_futz_blend", 0.0, 0.0, 1.0, 0x80u, "");
    snd_futz_bpf_f = _Dvar_RegisterFloat("snd_futz_bpf_f", 1454.236, 20.0, 10000.0, 0x80u, "");
    snd_futz_bpf_q = _Dvar_RegisterFloat("snd_futz_bpf_q", 1.781, 0.2, 16.0, 0x80u, "");
    snd_futz_ls_g = _Dvar_RegisterFloat("snd_futz_ls_g", -10.44, -60.0, 12.0, 0x80u, "");
    snd_futz_ls_f = _Dvar_RegisterFloat("snd_futz_ls_f", 483.24969, 20.0, 10000.0, 0x80u, "");
    snd_futz_ls_q = _Dvar_RegisterFloat("snd_futz_ls_q", 10.0, 0.2, 16.0, 0x80u, "");
    snd_futz_distortion = _Dvar_RegisterFloat("snd_futz_distortion", 0.30000001, 0.0, 1.0, 0x80u, "");
    snd_futz_preg = _Dvar_RegisterFloat("snd_futz_preg", 0.247529, 0.0, 12.0, 0x80u, "");
    snd_futz_postg = _Dvar_RegisterFloat("snd_futz_postg", 0.14267001, 0.0, 24.0, 0x80u, "");
    snd_futz_th = _Dvar_RegisterFloat("snd_futz_th", 0.01227265, 0.0099999998, 1.0, 0x80u, "");
    snd_futz_tg = _Dvar_RegisterFloat("snd_futz_tg", 0.28999999, 0.0, 4.0, 0x80u, "");
    snd_futz_clip_pre = _Dvar_RegisterFloat("snd_futz_clip_pre", 0.99636263, 0.0, 4.0, 0x80u, "");
    snd_futz_clip_post = _Dvar_RegisterFloat("snd_futz_clip_post", 1.781976, 0.0, 4.0, 0x80u, "");
    snd_futz_force = _Dvar_RegisterFloat("snd_futz_force", 0.0, 0.0, 1.0, 0x1000u, "");
    snd_dsp_futz = _Dvar_RegisterBool("snd_dsp_futz", 1, 0x1000u, "enable dsp futz");
    snd_pan_filter = _Dvar_RegisterFloat("snd_pan_filter", 1.0, 1.0, 100.0, 0x80u, "");
    snd_ps3_vol_occlusion_attenuation_dry = _Dvar_RegisterFloat(
                                                                                        "snd_ps3_vol_occlusion_attenuation_dry",
                                                                                        -20.0,
                                                                                        -80.0,
                                                                                        0.0,
                                                                                        0x80u,
                                                                                        "maximum dry volume occlusion attenuation for PS3");
    snd_ps3_vol_occlusion_attenuation_wet = _Dvar_RegisterFloat(
                                                                                        "snd_ps3_vol_occlusion_attenuation_wet",
                                                                                        -10.0,
                                                                                        -80.0,
                                                                                        0.0,
                                                                                        0x80u,
                                                                                        "maximum wet volume occlusion attenuation for PS3");
    if ( SND_GetSpeakerConfigCount() >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dvar.cpp",
                    351,
                    0,
                    "%s",
                    "ARRAY_COUNT(snd_speakerConfigurationStrings) > SND_GetSpeakerConfigCount()") )
    {
        __debugbreak();
    }
    for ( i = 0; i < SND_GetSpeakerConfigCount(); ++i )
        snd_speakerConfigurationStrings[i] = Snd_GetSpeakerConfig(i)->name;
    snd_speakerConfigurationStrings[SND_GetSpeakerConfigCount()] = 0;
    snd_speakerConfiguration = _Dvar_RegisterEnum(
                                                             "snd_speakerConfiguration",
                                                             snd_speakerConfigurationStrings,
                                                             0,
                                                             1u,
                                                             "Speaker configuration");
}

void __cdecl Snd_SetConfigStringsBySpeakerCount(unsigned int speakerCount)
{
    unsigned int i; // [esp+0h] [ebp-8h]
    int index; // [esp+4h] [ebp-4h]

    if ( !snd_speakerConfiguration
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dvar.cpp",
                    367,
                    0,
                    "%s",
                    "snd_speakerConfiguration") )
    {
        __debugbreak();
    }
    if ( SND_GetSpeakerConfigCount() >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dvar.cpp",
                    368,
                    0,
                    "%s",
                    "ARRAY_COUNT(snd_speakerConfigurationStrings) > SND_GetSpeakerConfigCount()") )
    {
        __debugbreak();
    }
    index = 0;
    for ( i = 0; i < SND_GetSpeakerConfigCount(); ++i )
    {
        if ( Snd_GetSpeakerConfig(i)->speakerCount <= speakerCount )
            snd_speakerConfigurationStrings[index++] = Snd_GetSpeakerConfig(i)->name;
    }
    snd_speakerConfigurationStrings[index] = 0;
    if ( snd_speakerConfiguration && snd_speakerConfiguration->current.integer >= index )
        Dvar_SetInt((dvar_s *)snd_speakerConfiguration, index - 1);
    Dvar_UpdateEnumDomain((dvar_s *)snd_speakerConfiguration, snd_speakerConfigurationStrings);
}

