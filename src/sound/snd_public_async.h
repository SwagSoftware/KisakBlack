#pragma once

#include "snd.h"
#include <tl/jobqueue/jobqueue_all.h>

void __cdecl SND_PlayInternal(
                unsigned int id,
                int fadeTimeMs,
                float attenuation,
                SndEntHandle entHandle,
                const float *position,
                const float *direction,
                bool notify,
                snd_playback *playback);
void __cdecl SND_Play(
                unsigned int id,
                int fadeTimeMs,
                float attenuation,
                SndEntHandle entHandle,
                const float *position,
                const float *direction,
                bool notify);
void __cdecl SND_Play(
                char *alias,
                int fadeTimeMs,
                float attenuation,
                SndEntHandle entHandle,
                const float *position,
                const float *direction,
                bool notify);
inline void SND_Play(
    const char *alias,
    int fadeTimeMs,
    float attenuation,
    SndEntHandle entHandle,
    const float *position,
    const float *direction,
    bool notify)
{
    SND_Play((char *)alias, fadeTimeMs, attenuation, entHandle, position, direction, notify);
}
int __cdecl SND_Playback(
                unsigned int alias,
                int fadeTimeMs,
                float attenuation,
                SndEntHandle entHandle,
                const float *position,
                const float *direction,
                bool notify);
void __cdecl SND_StopSoundAliasOnEnt(SndEntHandle ent, unsigned int alias_name);
void __cdecl SND_StopSoundsOnEnt(SndEntHandle ent);
void __cdecl SND_NotifyCinematicStart(float volume);
void __cdecl SND_NotifyCinematicEnd();
void __cdecl SND_DisconnectListener(unsigned int listener);
void __cdecl SND_SetListener(
                unsigned int listener,
                int clientNum,
                team_t team,
                const float *origin,
                const float (*axis)[3]);
void __cdecl SND_StopSounds(snd_stop_sound_flags flags);
void __cdecl SND_FadeIn();
void __cdecl SND_FadeOut();
void __cdecl SND_SetEnvironmentEffects(
                unsigned int priority,
                const char *preset,
                float drylevel,
                float wetlevel,
                int fademsec);
void __cdecl SND_DeactivateEnvironmentEffects(unsigned int priority, int fademsec);
void __cdecl SND_SetPlaybackAttenuation(unsigned int id, float attenuation);
void __cdecl SND_SetPlaybackAttenuationRate(unsigned int id, float rate);
void __cdecl SND_SetPlaybackPitch(unsigned int playbackId, float pitch);
void __cdecl SND_SetPlaybackPitchRate(unsigned int playbackId, float rate);
void __cdecl SND_StopPlayback(unsigned int playbackId);
void __cdecl SND_SetSnapshot(snd_snapshot_type type, const char *snapshotName, float length, float amount);
void __cdecl SND_SetEntState(SndEntHandle handle);
void __cdecl SND_PlayLoopAt(unsigned int id, const float *origin);
void __cdecl SND_StopLoopAt(unsigned int id, const float *origin);
void __cdecl SND_PlayLineAt(unsigned int id, const float *origin0, const float *origin1);
void __cdecl SND_StopLineAt(unsigned int id, const float *origin0, const float *origin1);
void __cdecl SND_SetContext(const char *type, const char *value);
void __cdecl SND_SetScriptTimescale(float value);
snd_ent_state *__cdecl SND_FindEntState(SndEntHandle handle, bool createNew);
void __cdecl SND_UpdateEntState(
                SndEntHandle handle,
                const float *origin,
                const float *velocity,
                const float (*orientation)[3]);
char __cdecl SND_GetEntState(SndEntHandle handle, float *origin, float *velocity, float (*orientation)[3]);
void __cdecl SNDL_SetEntState(
                SndEntHandle handle,
                const float *origin,
                const float *velocity,
                const float (*orientation)[3]);
void __cdecl SND_EntStateFrame();
void __cdecl SND_ResetEntState();
void __cdecl SND_EntStateRequest(SndEntHandle handle);
void __cdecl SND_SubtitleNotify(const char *subtitle, unsigned int lengthMs);
void __cdecl SND_LengthNotify(unsigned int ent, unsigned int lengthMs);
void __cdecl SND_FreePlaybackNotify(snd_playback *playback);
void __cdecl SND_ResetPlaybacks();
snd_playback *__cdecl SND_AllocatePlayback();
void __cdecl SND_FreePlayback(snd_playback *playback);
bool __cdecl SND_IsPlaying(int playbackId);
snd_playback *__cdecl SND_FindPlayback(int playbackId);
int __cdecl SND_GetPlaybackTime(int playbackId);
bool __cdecl SND_GetKnownLength(int playbackId, int *msec);
void SND_Update();
void __cdecl SND_UpdateWait();
int __cdecl updatesound_workerCallback(jqBatch *batch);
void SND_Frame();
void __cdecl SND_GameReset();
void __cdecl SND_BeginFrame(bool isMature, bool isPaused, float timescale, unsigned int cgTime, unsigned int seed);
void __cdecl SND_EndFrame();
void __cdecl SND_AsyncInit();
