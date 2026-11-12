#pragma once
#include "phys_collision.h"
#include "phys_main.h"

void __cdecl Phys_InitSoundAliases();
unsigned int __cdecl Phys_GetSoundAliasIndex(Phys_SurfaceType type1, Phys_SurfaceType type2);
void __cdecl Phys_EffectsInit();
void __cdecl Phys_ReEvalPriority(PhysObjUserData *userData);
void __cdecl Phys_EffectsProcess();
Phys_SurfaceType __cdecl Phys_RemmapSurfaceType(int stype);
unsigned int __cdecl Phys_GetSoundAliasForImpact(Phys_SurfaceType type1, Phys_SurfaceType type2);
bool __cdecl Phys_CanPlayEffect(PhysObjUserData *userData);

extern const dvar_t *phys_impact_intensity_limit;
extern const dvar_t *phys_impact_silence_window;
extern const dvar_t *phys_impact_distance_cutoff;
extern const dvar_t *phys_impact_render;
extern const dvar_t *phys_impact_fx;
extern const dvar_t *phys_reeval_frequency;
extern const dvar_t *phys_impact_max_pfx_per_frame;
extern const dvar_t *phys_impact_max_sfx_per_frame;