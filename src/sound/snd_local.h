#pragma once
#include "snd.h"

enum SND_ENVEFFECTPRIO : __int32
{
    SND_ENVEFFECTPRIO_NONE       = 0x0,
    SND_ENVEFFECTPRIO_LEVEL      = 0x1,
    SND_ENVEFFECTPRIO_SHELLSHOCK = 0x2,
    SND_ENVEFFECTPRIO_COUNT      = 0x3,
};

void __cdecl SNDL_AliasName(char *name, unsigned int id);
int __cdecl SNDL_Play(
                unsigned int aliasHash,
                int fadeTimeMs,
                float attenuation,
                SndEntHandle entHandle,
                float *position,
                const float *direction,
                bool notify,
                snd_playback *playback);
void __cdecl SNDL_StopSoundAliasOnEnt(SndEntHandle sndEnt, unsigned int name);
void __cdecl StopSoundAliasesOnEnt(SndEntHandle sndEnt, unsigned int name);
bool __cdecl SND_IsAliasStopOnDeath(const snd_alias_t *alias);
void __cdecl SNDL_StopSoundsOnEnt(SndEntHandle sndEnt);
void __cdecl SNDL_NotifyCinematicStart(float volume);
void __cdecl SNDL_NotifyCinematicEnd();
void __cdecl SNDL_DisconnectListener(int localClientNum);
void __cdecl SNDL_SetListener(
                int localClientNum,
                int clientNum,
                team_t team,
                const float *origin,
                const float (*inAxis)[3]);
void __cdecl SNDL_StopSounds(snd_stop_sound_flags which);
void __cdecl SNDL_FadeOut();
void __cdecl SNDL_FadeIn();
void __cdecl SNDL_SetEnvironmentEffects(
                int priority,
                unsigned int roomstring,
                float drylevel,
                float wetlevel,
                int fademsec);
void __cdecl SNDL_DeactivateEnvironmentEffects(int priority, int fademsec);
void __cdecl SNDL_SetPlaybackAttenuation(int playbackId, float attenuation);
void __cdecl SNDL_SetPlaybackAttenuationRate(int playbackId, float rate);
void __cdecl SNDL_SetPlaybackPitch(int playbackId, float pitch);
void __cdecl SNDL_SetPlaybackPitchRate(int playbackId, float rate);
void __cdecl SNDL_StopPlayback(int playbackId);
void __cdecl SNDL_SetSnapshot(snd_snapshot_type type, unsigned int id, float length, float amount);
void __cdecl SNDL_SetGameState(bool isMature, bool isPaused, float timescale, unsigned int cg_time, unsigned int seed);
void __cdecl SNDL_SetScriptTimescale(float value);
void __cdecl SNDL_PlayLoopAt(unsigned int id, float *origin);
void __cdecl SNDL_StopLoopAt(unsigned int id, const float *origin);
void __cdecl SNDL_PlayLineAt(unsigned int id, float *origin0, float *origin1);
void __cdecl SNDL_StopLineAt(unsigned int id, const float *origin0, const float *origin1);
int __cdecl SNDL_GetPlaybackTime(int playbackId);
void __cdecl SNDL_GameReset();
bool __cdecl SND_IsAliasMusic(const snd_alias_t *alias);
void __cdecl SNDL_SetContext(unsigned int type, unsigned int value);
void __cdecl SNDL_UpdateLoopingSounds();
