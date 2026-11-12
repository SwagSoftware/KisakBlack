#include "phys_effects.h"
#include <sound/snd_bank.h>
#include <cgame_mp/cg_local_mp.h>
#include <client/splitscreen.h>
#include <bgame/bg_slidemove.h>

const char *surface_type_names[8] = { "hard", "soft", "metal", "glass", "water", "wood", "foliage", "flesh" };

Phys_SurfaceType remap_table[31] =
{
  EFFECT_SURF_TYPE_SOFT,
  EFFECT_SURF_TYPE_HARD,
  EFFECT_SURF_TYPE_HARD,
  EFFECT_SURF_TYPE_SOFT,
  EFFECT_SURF_TYPE_SOFT,
  EFFECT_SURF_TYPE_HARD,
  EFFECT_SURF_TYPE_HARD,
  EFFECT_SURF_TYPE_FLESH,
  EFFECT_SURF_TYPE_FOLIAGE,
  EFFECT_SURF_TYPE_GLASS,
  EFFECT_SURF_TYPE_SOFT,
  EFFECT_SURF_TYPE_HARD,
  EFFECT_SURF_TYPE_HARD,
  EFFECT_SURF_TYPE_METAL,
  EFFECT_SURF_TYPE_SOFT,
  EFFECT_SURF_TYPE_SOFT,
  EFFECT_SURF_TYPE_SOFT,
  EFFECT_SURF_TYPE_HARD,
  EFFECT_SURF_TYPE_SOFT,
  EFFECT_SURF_TYPE_SOFT,
  EFFECT_SURF_TYPE_WATER,
  EFFECT_SURF_TYPE_WOOD,
  EFFECT_SURF_TYPE_HARD,
  EFFECT_SURF_TYPE_GLASS,
  EFFECT_SURF_TYPE_HARD,
  EFFECT_SURF_TYPE_SOFT,
  EFFECT_SURF_TYPE_SOFT,
  EFFECT_SURF_TYPE_SOFT,
  EFFECT_SURF_TYPE_METAL,
  EFFECT_SURF_TYPE_SOFT,
  EFFECT_SURF_TYPE_SOFT
};

unsigned int sound_aliases[36];

const dvar_t *phys_impact_intensity_limit;
const dvar_t *phys_impact_silence_window;
const dvar_t *phys_impact_distance_cutoff;
const dvar_t *phys_impact_render;
const dvar_t *phys_impact_fx;
const dvar_t *phys_reeval_frequency;
const dvar_t *phys_impact_max_pfx_per_frame;
const dvar_t *phys_impact_max_sfx_per_frame;

void __cdecl Phys_InitSoundAliases()
{
    unsigned int index; // [esp+0h] [ebp-94h]
    Phys_SurfaceType type2; // [esp+4h] [ebp-90h]
    Phys_SurfaceType type1; // [esp+8h] [ebp-8Ch]
    char name[132]; // [esp+Ch] [ebp-88h] BYREF

    for ( type1 = EFFECT_SURF_TYPE_HARD; type1 < EFFECT_SURF_TYPE_COUNT; ++type1 )
    {
        for ( type2 = type1; type2 < EFFECT_SURF_TYPE_COUNT; ++type2 )
        {
            index = Phys_GetSoundAliasIndex(type1, type2);
            sprintf(name, "phy_impact_%s_%s", surface_type_names[type1], surface_type_names[type2]);
            sound_aliases[index] = SND_FindAliasId(name);
            if ( !sound_aliases[index] )
            {
                sprintf(name, "phy_impact_%s_%s", surface_type_names[type2], surface_type_names[type1]);
                sound_aliases[index] = SND_FindAliasId(name);
            }
        }
    }
}

unsigned int __cdecl Phys_GetSoundAliasIndex(Phys_SurfaceType type1, Phys_SurfaceType type2)
{
    unsigned int v3; // [esp+0h] [ebp-8h]

    if ( (unsigned int)type1 >= EFFECT_SURF_TYPE_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_effects.cpp",
                    160,
                    0,
                    "%s",
                    "type1 >= 0 && type1 < EFFECT_SURF_TYPE_COUNT") )
    {
        __debugbreak();
    }
    if ( (unsigned int)type2 >= EFFECT_SURF_TYPE_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_effects.cpp",
                    161,
                    0,
                    "%s",
                    "type2 >= 0 && type2 < EFFECT_SURF_TYPE_COUNT") )
    {
        __debugbreak();
    }
    if ( type1 < type2 )
        v3 = type1 + type2 * (type2 + 1) / 2;
    else
        v3 = type2 + type1 * (type1 + 1) / 2;
    if ( v3 >= 0x24
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_effects.cpp",
                    163,
                    0,
                    "%s",
                    "index >= 0 && index < NUM_EFFECT_SURF_TYPE_PAIRS") )
    {
        __debugbreak();
    }
    return v3;
}

void __cdecl Phys_EffectsInit()
{
    phys_impact_intensity_limit = _Dvar_RegisterFloat(
                                                                    "phys_impact_intensity_limit",
                                                                    50.0,
                                                                    0.0,
                                                                    3.4028235e38,
                                                                    0,
                                                                    "Lower limit for the intensity of the impact sounds that the engine triggers.");
    phys_impact_silence_window = _Dvar_RegisterFloat(
                                                                 "phys_impact_silence_window",
                                                                 0.30000001,
                                                                 0.0,
                                                                 10.0,
                                                                 0,
                                                                 "The length of the silence window. When a body triggers an impact sound it will not trig"
                                                                 "ger new sounds for the duration of this silence window.");
    phys_impact_distance_cutoff = _Dvar_RegisterFloat(
                                                                    "phys_impact_distance_cutoff",
                                                                    800.0,
                                                                    0.0,
                                                                    3.4028235e38,
                                                                    0x1000u,
                                                                    "Disable effects that are further than distance_cutoff value. Uses the distance /in inc"
                                                                    "hes/ to the closes active player.");
    phys_impact_render = _Dvar_RegisterBool("phys_impact_render", 0, 0, "Render a star at the impact origin.");
    phys_impact_fx = _Dvar_RegisterBool("phys_impact_fx", 1, 0x1000u, "Enable/Disable the physics impact effects.");
    phys_reeval_frequency = _Dvar_RegisterFloat(
                                                        "phys_reeval_frequency",
                                                        2.0,
                                                        0.0,
                                                        3.4028235e38,
                                                        0,
                                                        "Controls how often the engine reevalutes the piority of the physics object. If the object ha"
                                                        "s low priority /its too far or smth/ we dont play effect.");
    phys_impact_max_pfx_per_frame = _Dvar_RegisterInt(
                                                                        "phys_impact_max_pfx_per_frame",
                                                                        10,
                                                                        0,
                                                                        50,
                                                                        0,
                                                                        "Maximum number of particle effects spawned on physics object impact. The value is per frame.");
    phys_impact_max_sfx_per_frame = _Dvar_RegisterInt(
                                                                        "phys_impact_max_sfx_per_frame",
                                                                        10,
                                                                        0,
                                                                        50,
                                                                        0,
                                                                        "Maximum number of sound effects spawned on physics objects impact. The value is per frame.");
}

void __cdecl Phys_ReEvalPriority(PhysObjUserData *userData)
{
    float player_pos[3]; // [esp+1Ch] [ebp-28h] BYREF
    float body_pos[3]; // [esp+28h] [ebp-1Ch] BYREF
    cg_s *cgameGlob; // [esp+34h] [ebp-10h]
    float dist2; // [esp+38h] [ebp-Ch]
    int localClientNum; // [esp+3Ch] [ebp-8h]
    unsigned int play_effect; // [esp+40h] [ebp-4h]

    if ( userData->m_time_since_last_reeval > phys_reeval_frequency->current.value )
    {
        userData->m_time_since_last_reeval = 0.0f;
        play_effect = 0;
        for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
        {
            if ( CL_LocalClient_IsActive(localClientNum) )
            {
                cgameGlob = CG_GetLocalClientGlobals(localClientNum);
                player_pos[0] = cgameGlob->refdef.vieworg[0];
                player_pos[1] = cgameGlob->refdef.vieworg[1];
                player_pos[2] = cgameGlob->refdef.vieworg[2];
                Phys_NitrousVecToVec3(userData->body->m_mat.w, body_pos);
                dist2 = Vec3DistanceSq(body_pos, player_pos);
                if ( (float)(phys_impact_distance_cutoff->current.value * phys_impact_distance_cutoff->current.value) > dist2 )
                {
                    play_effect = 1;
                    break;
                }
            }
        }
        if ( play_effect )
            userData->m_flags |= 1u;
        else
            userData->m_flags &= ~1u;
    }
}

void __cdecl Phys_EffectsProcess()
{
    ; // KISAKTODO: this function actually exists but needs IDA rework
}

Phys_SurfaceType __cdecl Phys_RemmapSurfaceType(int stype)
{
    return remap_table[stype];
}

unsigned int __cdecl Phys_GetSoundAliasForImpact(Phys_SurfaceType type1, Phys_SurfaceType type2)
{
    return sound_aliases[Phys_GetSoundAliasIndex(type1, type2)];
}

bool __cdecl Phys_CanPlayEffect(PhysObjUserData *userData)
{
    return userData->m_time_since_last_event > phys_impact_silence_window->current.value && (userData->m_flags & 1) != 0;
}

