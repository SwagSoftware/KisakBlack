#include "snd_local.h"
#include "snd_log.h"

#include <cstring>
#include "snd_bank.h"
#include "snd_public_async.h"
#include <win32/win_shared.h>
#include <win32/win_voice.h>
#include "snd_utils.h"
#include <qcommon/common.h>
#include <cgame/cg_sound.h>
#include <win32/win_common.h>
#include "snd_globals.h"
#include "snd_driver_xaudio2.h"

void __cdecl SNDL_AliasName(char *name, unsigned int id)
{
    SND_LogRegisterString(name, id);
}

int __cdecl SNDL_Play(
                unsigned int aliasHash,
                int fadeTimeMs,
                float attenuation,
                SndEntHandle entHandle,
                float *position,
                const float *direction,
                bool notify,
                snd_playback *playback)
{
    snd_alias_list_t *list; // [esp+2Ch] [ebp-28h]
    float temp[3]; // [esp+30h] [ebp-24h] BYREF
    snd_occlusion_start_cache ocache; // [esp+3Ch] [ebp-18h] BYREF
    int pId; // [esp+50h] [ebp-4h]

    if ( !aliasHash )
        return -1;
    pId = -1;
    memset(&ocache, 0, sizeof(ocache));
    if ( !position )
    {
        memset(temp, 0, sizeof(temp));
        position = temp;
    }
    list = SND_FindAliasFromId(aliasHash);
    if ( list )
    {
        nanassertvec3(position);
        
        pId = SND_PlaySoundAlias(list, attenuation, entHandle, position, 0, fadeTimeMs, direction, playback, &ocache);
        if ( notify )
            SND_AddLengthNotify(pId, (const char*)entHandle.field.entIndex, SND_LENGTH_NOTIFY_SCRIPT);
    }
    else
    {
        SND_LogMissingAliasId(aliasHash);
    }
    if ( pId != -1 )
    {
        if ( notify )
            SND_AddLengthNotify(pId, (const char *)entHandle.field.entIndex, SND_LENGTH_NOTIFY_SCRIPT);
        if ( playback
            && playback->id != pId
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp",
                        85,
                        0,
                        "%s",
                        "playback->id == pId") )
        {
            __debugbreak();
        }
    }
    if ( pId == -1 )
    {
        if ( playback )
            SND_FreePlaybackNotify(playback);
    }
    return pId;
}

void __cdecl SNDL_StopSoundAliasOnEnt(SndEntHandle sndEnt, unsigned int name)
{
    StopSoundAliasesOnEnt(sndEnt, name);
}

void __cdecl StopSoundAliasesOnEnt(SndEntHandle sndEnt, unsigned int name)
{
    for (int i = 0; i < SND_MAX_VOICES; ++i)
    {
        snd_voice_t *voice = &g_snd.voice[i];
        if (g_snd.voiceAliasHash[i]
            && voice->sndEnt.handle == sndEnt.handle
            && ((voice->alias->flags & 1) != 0 || name || SND_IsAliasStopOnDeath(voice->alias))
            && (!name || name == g_snd.voiceAliasHash[i]))
        {
            SND_StopVoice(i);
        }
    }
}

bool __cdecl SND_IsAliasStopOnDeath(const snd_alias_t *alias)
{
    iassert(alias);
    return (alias->flags & 0x200) >> 9 != 0;
}

void __cdecl SNDL_StopSoundsOnEnt(SndEntHandle sndEnt)
{
    StopSoundAliasesOnEnt(sndEnt, 0);
}

void __cdecl SNDL_NotifyCinematicStart(float volume)
{
    if (!g_snd.inCinematic && volume > SND_EPSILON)
    {
        g_snd.inCinematic = 1;
        g_snd.cinematicUpdate = 1;
        g_snd.cinematicTimestamp = Sys_Milliseconds();
    }
}

void __cdecl SNDL_NotifyCinematicEnd()
{
    if (g_snd.inCinematic)
    {
        g_snd.inCinematic = 0;
        g_snd.cinematicUpdate = 1;
        g_snd.cinematicTimestamp = Sys_Milliseconds();
    }
}

void __cdecl SNDL_DisconnectListener(int localClientNum)
{
    bcassert(localClientNum, 1/*MAX_LOCAL_CLIENTS*/);
    memset((unsigned __int8 *)&g_snd.listeners[localClientNum], 0, sizeof(g_snd.listeners[localClientNum]));
    AxisClear(g_snd.listeners[localClientNum].orient.axis);
    Vec3Clear(g_snd.listeners[localClientNum].orient.origin);

    bool kill = true;
    for (int i = 0; !i; i = 1)
    {
        kill &= !g_snd.listeners[0].active;
    }
    if (kill)
    {
        for (int voiceIndex = 0; voiceIndex < SND_MAX_VOICES; ++voiceIndex)
        {
            if (g_snd.voiceAliasHash[voiceIndex] && (g_snd.voice[voiceIndex].alias->flags & 2) >> 1)
                SND_StopVoice(voiceIndex);
        }
    }
}

void __cdecl SNDL_SetListener(
                int localClientNum,
                int clientNum,
                team_t team,
                const float *origin,
                const float (*inAxis)[3])
{
    snd_listener *v5; // [esp+10h] [ebp-88h]
    float b; // [esp+68h] [ebp-30h]
    float axis[3][3]; // [esp+6Ch] [ebp-2Ch] BYREF
    float a; // [esp+90h] [ebp-8h]
    bool independant; // [esp+97h] [ebp-1h]

    iassert(origin);
    iassert(inAxis);
    bcassert(localClientNum, 1/*MAX_LOCAL_CLIENTS*/);
    nanassertvec3(origin);
    nanassertvec3(inAxis[0]);
    nanassertvec3(inAxis[1]);
    nanassertvec3(inAxis[2]);

    AxisCopy(inAxis, axis);
    axis[0][2] = 0.0f;
    axis[1][2] = 0.0f;
    axis[2][0] = 0.0f;
    //*(_QWORD *)&axis[2][1] = __PAIR64__(LODWORD(1.0f), 0);
    axis[2][1] = 0.0f;
    axis[2][2] = 1.0f;
    a = Vec3Normalize(axis[0]);
    b = Vec3Normalize(axis[1]);

    iassert(!IS_NAN(a));
    iassert(!IS_NAN(b));

    independant = (float)((float)((float)(axis[0][0] * axis[1][0]) + (float)(axis[0][1] * axis[1][1]))
                                            + (float)(axis[0][2] * axis[1][2])) < 0.99998474;
    if ( a > SND_EPSILON && b > SND_EPSILON && independant )
    {
        AxisCopy(axis, g_snd.listeners[localClientNum].orient.axis);
        v5 = &g_snd.listeners[localClientNum];
        v5->orient.origin[0] = *origin;
        v5->orient.origin[1] = origin[1];
        v5->orient.origin[2] = origin[2];
        g_snd.listeners[localClientNum].clientNum = clientNum;
        g_snd.listeners[localClientNum].active = 1;
    }

    iassert(Vec3LengthSq(g_snd.listeners[localClientNum].orient.axis[0]) > 1.0f - SND_EPSILON);
    iassert(Vec3LengthSq(g_snd.listeners[localClientNum].orient.axis[1]) > 1.0f - SND_EPSILON);
    iassert(Vec3LengthSq(g_snd.listeners[localClientNum].orient.axis[2]) > 1.0f - SND_EPSILON);

    g_snd.listeners[localClientNum].clientNum = clientNum;
    g_snd.listeners[localClientNum].active = 1;
    g_snd.listeners[localClientNum].team = team;
}

void __cdecl SNDL_StopSounds(snd_stop_sound_flags which)
{
    if ( which != SND_EVERY_SINGLE_ONE_DONT_ASK_ANY_QUESTIONS && which )
    {
        if ( which == SND_STOP_STREAMED )
        {
            for ( int voiceIndex = 10; voiceIndex < SND_MAX_VOICES; ++voiceIndex )
            {
                if ( g_snd.voiceAliasHash[voiceIndex] )
                    SND_StopVoice(voiceIndex);
            }
        }
    }
    else
    {
        for (int  i = 0; i < SND_MAX_VOICES; ++i )
            SND_StopVoice(i);
    }
}

void __cdecl SNDL_FadeOut()
{
    SND_FaderSetRate(&g_snd.volume, 0.5);
    SND_FaderSetGoal(&g_snd.volume, 0.0);
    Voice_StopClientSamples();
}

void __cdecl SNDL_FadeIn()
{
    SND_FaderSetRate(&g_snd.volume, 4.0);
    SND_FaderSetGoal(&g_snd.volume, 1.0);
}

void __cdecl SNDL_SetEnvironmentEffects(
                int priority,
                unsigned int roomstring,
                float drylevel,
                float wetlevel,
                int fademsec)
{
    snd_enveffect *effect; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if (SND_ActiveListenerCount() == 1)
    {
        iassert(priority > SND_ENVEFFECTPRIO_NONE && priority < SND_ENVEFFECTPRIO_COUNT);
        iassert(roomstring);
        iassert(fademsec >= 0);
        iassert(drylevel >= 0 && drylevel <= 1);
        iassert(wetlevel >= 0 && wetlevel <= 1);

        effect = &g_snd.envEffects[priority];
        effect->active = 1;
        if (fademsec < 1)
            fademsec = 1;
        effect->drygoal = drylevel;
        effect->drylevel = g_snd.effect->drylevel;
        effect->dryrate = (float)(drylevel - g_snd.effect->drylevel) / (float)fademsec;
        effect->wetgoal = wetlevel;
        effect->wetlevel = g_snd.effect->wetlevel;
        effect->wetrate = (float)(wetlevel - g_snd.effect->wetlevel) / (float)fademsec;
        effect->reverbId = roomstring;
        if (effect != g_snd.effect)
        {
            for (i = priority + 1; i < SND_ENVEFFECTPRIO_COUNT; ++i)
            {
                if (g_snd.envEffects[i].active)
                    return;
            }
            g_snd.effect = &g_snd.envEffects[priority];
        }
    }
}

void __cdecl SNDL_DeactivateEnvironmentEffects(int priority, int fademsec)
{
    snd_enveffect *effect; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if (SND_ActiveListenerCount() == 1)
    {
        iassert(priority > SND_ENVEFFECTPRIO_NONE && priority < SND_ENVEFFECTPRIO_COUNT);
        iassert(fademsec >= 0);

        effect = &g_snd.envEffects[priority];
        effect->active = 0;
        if (effect == g_snd.effect)
        {
            for (i = priority - 1; i >= 0 && !g_snd.envEffects[i].active; --i)
                ;

            iassert(i >= SND_ENVEFFECTPRIO_NONE);
 
            if (fademsec < 1)
                fademsec = 1;

            g_snd.effect = &g_snd.envEffects[i];
            g_snd.envEffects[i].drylevel = effect->drylevel;
            g_snd.effect->dryrate = (float)(g_snd.effect->drygoal - effect->drylevel) / (float)fademsec;
            g_snd.effect->wetlevel = effect->wetlevel;
            g_snd.effect->wetrate = (float)(g_snd.effect->wetgoal - effect->wetlevel) / (float)fademsec;
        }
    }
}

void __cdecl SNDL_SetPlaybackAttenuation(int playbackId, float attenuation)
{
    float g; // [esp+4h] [ebp-1Ch]
    float v3; // [esp+18h] [ebp-8h]
    snd_voice_t *voice; // [esp+1Ch] [ebp-4h]

    voice = SND_GetPlaybackVoice(playbackId);
    if ( voice )
    {
        if ( (float)(attenuation - 1.0) < 0.0 )
            v3 = attenuation;
        else
            v3 = 1.0f;
        if ( (float)(0.0 - attenuation) < 0.0 )
            g = v3;
        else
            g = 0.0f;
        SND_FaderSetGoal(&voice->script_fade, g);
    }
}

void __cdecl SNDL_SetPlaybackAttenuationRate(int playbackId, float rate)
{
    snd_voice_t *voice = SND_GetPlaybackVoice(playbackId);
    if ( voice )
        SND_FaderSetRate(&voice->script_fade, rate);
}

void __cdecl SNDL_SetPlaybackPitch(int playbackId, float pitch)
{
    float g; // [esp+4h] [ebp-1Ch]
    float v3; // [esp+18h] [ebp-8h]
    snd_voice_t *voice; // [esp+1Ch] [ebp-4h]

    voice = SND_GetPlaybackVoice(playbackId);
    if ( voice )
    {
        if ( (float)(pitch - 2.0) < 0.0 )
            v3 = pitch;
        else
            v3 = 2.0f;
        if ( (float)(0.1 - pitch) < 0.0 )
            g = v3;
        else
            g = 0.1f;
        SND_FaderSetGoal(&voice->script_pitch, g);
    }
}

void __cdecl SNDL_SetPlaybackPitchRate(int playbackId, float rate)
{
    snd_voice_t *voice = SND_GetPlaybackVoice(playbackId);
    if ( voice )
        SND_FaderSetRate(&voice->script_pitch, rate);
}

void __cdecl SNDL_StopPlayback(int playbackId)
{
    snd_voice_t *voice = SND_GetPlaybackVoice(playbackId);
    if ( voice )
        SND_StopVoice(voice - g_snd.voice);
}

void __cdecl SNDL_SetSnapshot(snd_snapshot_type type, unsigned int id, float length, float amount)
{
    bcassert(type, SND_SNAPSHOT_COUNT);

    if (SND_GetSnapshotById(id))
        g_snd.snapshotCategories[type].snapshot = id;
    else
        g_snd.snapshotCategories[type].snapshot = g_snd.defaultHash;

    g_snd.snapshotCategories[type].length = length;
    g_snd.snapshotCategories[type].amount = amount;
}

void __cdecl SNDL_SetGameState(bool isMature, bool isPaused, float timescale, unsigned int cg_time, unsigned int seed)
{
    unsigned int value; // [esp-4h] [ebp-4h]

    g_snd.gameState.mature = isMature;
    g_snd.gameState.gamePaused = isPaused;
    g_snd.gameState.timescale = timescale;
    g_snd.gameState.cgTime = cg_time;

    if (seed)
        g_snd.gameState.seed = seed;

    if (isMature)
        value = SND_HashName("explicit");
    else
        value = SND_HashName("safe");

    SNDL_SetContext(SND_HashName("mature"), value);
}

void __cdecl SNDL_SetScriptTimescale(float value)
{
    g_snd.scriptTimescale = value;
}

void __cdecl SNDL_PlayLoopAt(unsigned int id, float *origin)
{
    bool gotIt = false;
    for ( int i = 0; i < 0x100; ++i )
    {
        if ( !g_snd.loopEmitters[i].id )
        {
            g_snd.loopEmitters[i].id = id;
            g_snd.loopEmitters[i].alias = 0;
            Vec3Copy(origin, g_snd.loopEmitters[i].origin);
            gotIt = true;
            break;
        }
    }
    if ( !gotIt )
        Com_PrintError(9, "ERROR: Sound ran out of loop emitters\n");
}

void __cdecl SNDL_StopLoopAt(unsigned int id, const float *origin)
{
    for ( int i = 0; i < 0x100; ++i )
    {
        if ( g_snd.loopEmitters[i].id == id && Vec3DistanceSq(origin, g_snd.loopEmitters[i].origin) < SND_EPSILON )
        {
            iassert(g_snd.loopEmitters[i].alias);
            SndEntHandle sndEnt = SND_EntHandle(0, i + 3710, 0, 0, 1, TEAM_FREE);
            SND_ContinueLoopingSound(g_snd.loopEmitters[i].id, 1.0, sndEnt, 0, -500, 0);
            g_snd.loopEmitters[i].id = 0;
            g_snd.loopEmitters[i].alias = 0;
            Vec3Clear(g_snd.loopEmitters[i].origin);
        }
    }
}

void __cdecl SNDL_PlayLineAt(unsigned int id, float *origin0, float *origin1)
{
    unsigned int i; // [esp+8h] [ebp-Ch]
    snd_alias_list_t *list; // [esp+Ch] [ebp-8h]
    bool gotIt; // [esp+13h] [ebp-1h]

    list = SND_FindAliasFromId(id);
    if (list && list->head)
    {
        gotIt = false;
        for (i = 0; i < 0x80; ++i)
        {
            if (!g_snd.lineEmitters[i].id)
            {
                g_snd.lineEmitters[i].id = id;
                Vec3Copy(origin0, g_snd.lineEmitters[i].origin[0]);
                Vec3Copy(origin1, g_snd.lineEmitters[i].origin[1]);
                gotIt = true;
                break;
            }
        }
        if (!gotIt)
            Com_PrintError(9, "ERROR: Sound ran out of line emitters\n");
    }
}

void __cdecl SNDL_StopLineAt(unsigned int id, const float *origin0, const float *origin1)
{
    for (int i = 0; i < 0x80; ++i)
    {
        if (g_snd.lineEmitters[i].id == id
            && Vec3DistanceSq(origin0, g_snd.lineEmitters[i].origin[0]) < SND_EPSILON
            && Vec3DistanceSq(origin1, g_snd.lineEmitters[i].origin[1]) < SND_EPSILON)
        {
            SndEntHandle sndEnt = SND_EntHandle(0, i + 3966, 0, 0, 1, TEAM_FREE);
            SND_ContinueLoopingSound(g_snd.lineEmitters[i].id, 1.0, sndEnt, 0, -500, 0);
            g_snd.lineEmitters[i].id = 0;
            g_snd.lineEmitters[i].alias = 0;
            Vec3Clear(g_snd.lineEmitters[i].origin[0]);
            Vec3Clear(g_snd.lineEmitters[i].origin[1]);
        }
    }
}

int __cdecl SNDL_GetPlaybackTime(int playbackId)
{
    snd_voice_t *voice; // [esp+0h] [ebp-4h]

    voice = SND_GetPlaybackVoice(playbackId);
    if ( !voice )
        return 0;
    if ( g_snd.pausetime )
        return g_snd.pausetime - voice->startTime;
    return g_snd.time - voice->startTime;
}

void __cdecl SNDL_GameReset()
{
    const snd_alias_t *alias; // esi
    snd_voice_t *voice; // [esp+1Ch] [ebp-Ch]
    unsigned int i; // [esp+20h] [ebp-8h]

    if (SND_Active())
    {
        SNDL_FadeIn();
        SND_ResetEntState();
        memset((unsigned __int8 *)g_snd.loopEmitters, 0, sizeof(g_snd.loopEmitters));
        memset((unsigned __int8 *)g_snd.lineEmitters, 0, sizeof(g_snd.lineEmitters));
        memset((unsigned __int8 *)g_snd.currentContexts, 0, sizeof(g_snd.currentContexts));
        for (i = 0; i < SND_MAX_VOICES; ++i)
        {
            if (g_snd.voiceAliasHash[i])
            {
                voice = &g_snd.voice[i];
                alias = voice->alias;
                if (alias->id == SND_HashName("mus_mp_frontend"))
                {
                    if ((voice->alias->flags & 1) != 0)
                    {
                        if (SND_IsAliasMusic(voice->alias))
                        {
                            SND_FaderSetRateTime(&voice->fade, 4.0);
                            SND_FaderSetGoal(&voice->fade, 0.0);
                        }
                    }
                }
                else
                {
                    SND_StopVoice(i);
                }
            }
        }
        SNDL_Update();
        Sys_EnterCriticalSection(CRITSEC_SOUND_OCCLUSION);
        Sys_LeaveCriticalSection(CRITSEC_SOUND_OCCLUSION);
        g_snd.traceSequence = 0;
        g_snd.lastTraceSequence = 0;
        SNDL_SetSnapshot(SND_SNAPSHOT_AMBIENT, g_snd.defaultHash, 0.0, 1.0);
        SNDL_SetSnapshot(SND_SNAPSHOT_SCRIPT, g_snd.defaultHash, 0.0, 1.0);
        SNDL_SetSnapshot(SND_SNAPSHOT_SHELLSHOCK, g_snd.defaultHash, 0.0, 1.0);
        SNDL_SetSnapshot(SND_SNAPSHOT_ADS, g_snd.defaultHash, 0.0, 1.0);
        SNDL_SetSnapshot(SND_SNAPSHOT_BREATH, g_snd.defaultHash, 0.0, 1.0);
        g_snd.scriptTimescale = 1.0f;
        memset((unsigned __int8 *)g_snd.envEffects, 0, sizeof(g_snd.envEffects));
        g_snd.effect = g_snd.envEffects;
        g_snd.envEffects[0].reverbId = g_snd.defaultHash;
        g_snd.envEffects[0].drylevel = 1.0f;
        g_snd.effect->drygoal = 1.0f;
        g_snd.effect->dryrate = 0.0f;
        g_snd.effect->wetlevel = 0.0f;
        g_snd.effect->wetgoal = 0.0f;
        g_snd.effect->wetrate = 0.0f;
        g_snd.effect->active = 1;
    }
}

bool __cdecl SND_IsAliasMusic(const snd_alias_t *alias)
{
    iassert(alias);
    return (alias->flags & 0x100) >> 8 != 0;
}

void __cdecl SNDL_SetContext(unsigned int type, unsigned int value)
{
    signed int index; // [esp+0h] [ebp-4h]

    index = SND_FindContextIndex(type);
    if ( index >= 0 )
        g_snd.currentContexts[index] = value;
}

void __cdecl SNDL_UpdateLoopingSounds()
{
    snd_voice_t *voice; // [esp+4h] [ebp-8h]
    unsigned int i; // [esp+8h] [ebp-4h]

    if (!g_snd.paused)
    {
        for (i = 0; i < SND_MAX_VOICES; ++i)
        {
            voice = &g_snd.voice[i];
            if (g_snd.voiceAliasHash[i]
                && (voice->alias->flags & 1) != 0
                && voice->fade.goal != 0.0
                && voice->looptime != g_snd.looptime
                && (voice->sndEnt.handle & 0xFFF) != 0xFFE)
            {
                SND_StopVoice(i);
            }
        }
        ++g_snd.looptime;
    }
}

