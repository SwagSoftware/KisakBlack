#include "snd_public_async.h"
#include <qcommon/common.h>
#include "snd_dvar.h"
#include "snd_public_async_q.h"
#include "snd_utils.h"
#include "snd_bank.h"
#include <cgame_mp/cg_consolecmds_mp.h>
#include <cgame/cg_sound.h>
#include "snd_globals.h"
#include <win32/win_common.h>
#include <universal/com_workercmds.h>

#include <Windows.h>
#include "snd_local.h"
#include "snd_driver_xaudio2.h"

volatile unsigned int updatesound_workerLimit = 1;
jqModule updatesound_workerModule =
{
    .Name = "updatesound_worker",
    .Type = JQ_WORKER_GENERIC,
    .Code = (int(__cdecl *)(jqBatch *))updatesound_workerCallback,
};
jqWorkerCmd updatesound_workerWorkerCmd = { &updatesound_workerModule, 4u, 0, 0, &updatesound_workerLimit, NULL, 0u };

volatile unsigned int entryCount;

void __cdecl SND_PlayInternal(
                unsigned int id,
                int fadeTimeMs,
                float attenuation,
                SndEntHandle entHandle,
                const float *position,
                const float *direction,
                bool notify,
                snd_playback *playback)
{
    snd_command *cmd; // [esp+44h] [ebp-4h]

    iassert(entHandle.field.entIndex < MAX_LOCAL_CENTITIES || entHandle.field.entIndex == SND_ENT_NONE || entHandle.field.entIndex == SND_ENT_NO_STOP);

    if (!(entHandle.field.entIndex < MAX_LOCAL_CENTITIES || entHandle.field.entIndex == SND_ENT_NONE || entHandle.field.entIndex == SND_ENT_NO_STOP))
    {
        Com_Error(ERR_DROP, "Invalid sound handle");
    }
    if ( SND_Active() && id )
    {
        iassert(fadeTimeMs < 32767);
        iassert(fadeTimeMs > -32767);
        iassert(!IS_NAN(attenuation));
        iassert(attenuation >= 0.0f);
        iassert(attenuation <= 1.0f);

        if (position)
        {
            nanassertvec3(position);
        }
        if (direction)
        {
            nanassertvec3(direction);
        }

#ifdef _DEBUG
        if ( snd_assert_on_enqueue
            && snd_assert_on_enqueue->current.integer
            && *(_BYTE *)snd_assert_on_enqueue->current.integer
            && SND_HashName(snd_assert_on_enqueue->current.string) == id
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                        104,
                        0,
                        "%s",
                        "SND_HashName(snd_assert_on_enqueue->current.string) != id") )
        {
            __debugbreak();
        }
#endif
        if (playback)
        {
            iassert(playback->id != SND_PLAYBACKID_NOTPLAYED);
        }

        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_PLAY;
            cmd->context.play.alias = id;
            cmd->context.play.fadeTimeMs = fadeTimeMs;
            cmd->context.play.attenuation = attenuation;
            cmd->context.play.entHandle = entHandle;
            cmd->context.play.notify = notify;
            cmd->context.play.playback = playback;
            if ( position )
            {
                nanassertvec3(position);

                Vec3Copy(position, cmd->context.play.position);
            }
            else
            {
                Vec3Clear(cmd->context.play.position);
            }
            if ( direction )
            {
                nanassertvec3(direction);

                Vec3Copy(direction, cmd->context.play.direction);
            }
            else
            {
                Vec3Clear(cmd->context.play.direction);
            }
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_Play(
                unsigned int id,
                int fadeTimeMs,
                float attenuation,
                SndEntHandle entHandle,
                const float *position,
                const float *direction,
                bool notify)
{
    SND_PlayInternal(id, fadeTimeMs, attenuation, entHandle, position, direction, notify, 0);
}

void __cdecl SND_Play(
                char *alias,
                int fadeTimeMs,
                float attenuation,
                SndEntHandle entHandle,
                const float *position,
                const float *direction,
                bool notify)
{
    unsigned int AliasId; // eax

    if ( SND_Active() )
    {
        AliasId = SND_FindAliasId(alias);
        SND_PlayInternal(AliasId, fadeTimeMs, attenuation, entHandle, position, direction, notify, 0);
    }
}

int __cdecl SND_Playback(
                unsigned int alias,
                int fadeTimeMs,
                float attenuation,
                SndEntHandle entHandle,
                const float *position,
                const float *direction,
                bool notify)
{
    if ( !SND_Active() )
        return SND_PLAYBACKID_NOTPLAYED;
    snd_playback *playback = SND_AllocatePlayback();
    SND_PlayInternal(alias, fadeTimeMs, attenuation, entHandle, position, direction, notify, playback);
    if ( playback )
        return playback->id;
    else
        return SND_PLAYBACKID_NOTPLAYED;
}

void __cdecl SND_StopSoundAliasOnEnt(SndEntHandle ent, unsigned int alias_name)
{
    snd_command *cmd; // [esp+0h] [ebp-4h]

    if ( SND_Active() )
    {
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_STOP_ALIAS;
            cmd->context.stop_alias.ent = ent;
            cmd->context.stop_alias.alias_name = alias_name;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_StopSoundsOnEnt(SndEntHandle ent)
{
    if ( SND_Active() )
    {
        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_STOP_ENT;
            cmd->context.stop_alias.ent = ent;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_NotifyCinematicStart(float volume)
{
    if ( SND_Active() )
    {
        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_CINEMATIC_START;
            cmd->context.notify_cinematic_start.volume = volume;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_NotifyCinematicEnd()
{
    if ( SND_Active() )
    {
        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_CINEMATIC_END;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_DisconnectListener(unsigned int listener)
{
    if ( SND_Active() )
    {
        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_DISCONNECT_LISTENER;
            cmd->context.disconnect_listener.listener = listener;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_SetListener(
                unsigned int listener,
                int clientNum,
                team_t team,
                const float *origin,
                const float (*axis)[3])
{
    if ( SND_Active() )
    {
        iassert(Vec3Length(axis[0]) > 1.0f - (1.52879e-5f) && Vec3Length(axis[0]) < 1.0f + (1.52879e-5f));

        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_SET_LISTENER;

            cmd->context.set_listener.listener = listener;
            cmd->context.set_listener.clientNum = clientNum;
            cmd->context.set_listener.team = team;
            cmd->context.set_listener.origin[0] = origin[0];
            cmd->context.set_listener.origin[1] = origin[1];
            cmd->context.set_listener.origin[2] = origin[2];
            cmd->context.set_listener.axis[0][0] = axis[0][0];
            cmd->context.set_listener.axis[0][1] = axis[0][1];
            cmd->context.set_listener.axis[0][2] = axis[0][2];
            cmd->context.set_listener.axis[1][0] = axis[1][0];
            cmd->context.set_listener.axis[1][1] = axis[1][1];
            cmd->context.set_listener.axis[1][2] = axis[1][2];
            cmd->context.set_listener.axis[2][0] = axis[2][0];
            cmd->context.set_listener.axis[2][1] = axis[2][1];
            cmd->context.set_listener.axis[2][2] = axis[2][2];

            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_StopSounds(snd_stop_sound_flags flags)
{
    if ( SND_Active() )
    {
        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_STOP_SOUNDS;
            cmd->context.play.alias = flags;
            SND_CommandPush(cmd);
        }
        SND_UpdateWait();
    }
}

void __cdecl SND_FadeIn()
{
    if ( SND_Active() )
    {
        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_FADE_IN;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_FadeOut()
{
    if ( SND_Active() )
    {
        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_FADE_OUT;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_SetEnvironmentEffects(
                unsigned int priority,
                const char *preset,
                float drylevel,
                float wetlevel,
                int fademsec)
{
    if ( SND_Active() )
    {
        int id = SND_HashName(preset);
        const snd_radverb *radverb = SND_GetRadverb(id);
        if ( !radverb || radverb->id == g_snd.defaultHash && id != g_snd.defaultHash )
            Com_PrintError(9, "Missing radverb %s\n", preset);
        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_SET_ENVIRONMENT_EFFECTS;
            cmd->context.set_environment_effects.priority = priority;
            cmd->context.set_environment_effects.id = id;
            cmd->context.set_environment_effects.drylevel = drylevel;
            cmd->context.set_environment_effects.wetlevel = wetlevel;
            cmd->context.set_environment_effects.fademsec = fademsec;

            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_DeactivateEnvironmentEffects(unsigned int priority, int fademsec)
{
    if ( SND_Active() )
    {
        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_DEACTIVATE_ENVIRONMENT_EFFECTS;
            cmd->context.deactivate_environment_effects.priority = priority;
            cmd->context.deactivate_environment_effects.fademsec = fademsec;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_SetPlaybackAttenuation(unsigned int id, float attenuation)
{
    if ( SND_Active() )
    {
        iassert(!IS_NAN(attenuation));
        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_SET_PLAYBACK_ATTENUATION;
            cmd->context.set_playback_attenuation.id = id;
            cmd->context.set_playback_attenuation.attenuation = attenuation;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_SetPlaybackAttenuationRate(unsigned int id, float rate)
{
    if ( SND_Active() )
    {
        iassert(!IS_NAN(rate));

        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_SET_PLAYBACK_ATTENUATION_RATE;
            cmd->context.set_playback_attenuation_rate.id = id;
            cmd->context.set_playback_attenuation.attenuation = rate;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_SetPlaybackPitch(unsigned int playbackId, float pitch)
{
    if ( SND_Active() )
    {
        iassert(!IS_NAN(pitch));

        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_SET_PLAYBACK_PITCH;
            cmd->context.set_playback_pitch.id = playbackId;
            cmd->context.set_playback_pitch.pitch = pitch;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_SetPlaybackPitchRate(unsigned int playbackId, float rate)
{
    if ( SND_Active() )
    {
        iassert(!IS_NAN(rate));
        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_SET_PLAYBACK_PITCH_RATE;
            cmd->context.set_playback_pitch_rate.id = playbackId;
            cmd->context.set_playback_pitch_rate.rate = rate;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_StopPlayback(unsigned int playbackId)
{
    if ( SND_Active() )
    {
        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_STOP_PLAYBACK;
            cmd->context.stop_playback.id = playbackId;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_SetSnapshot(snd_snapshot_type type, const char *snapshotName, float length, float amount)
{
    if ( SND_Active() )
    {
        if ( !snapshotName || !*snapshotName )
            snapshotName = "default";

        iassert(!IS_NAN(length));

        uint id = SND_HashName(snapshotName);
        if ( !SND_GetSnapshotById(id) )
        {
            Com_PrintError(9, "Could not find group snapshot %s\n", snapshotName);
            id = g_snd.defaultHash;
        }
        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_SNAPSHOT;
            cmd->context.snapshot.type = type;
            cmd->context.snapshot.id = id;
            cmd->context.snapshot.length = length;
            cmd->context.snapshot.amount = amount;

            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_SetEntState(SndEntHandle handle)
{
    snd_command *cmd; // [esp+4h] [ebp-4h]

    if ( SND_Active()
        && (handle.field.isStationary) == 0
        && (handle.field.entIndex) != 0xFFF
        && CL_LocalClient_IsCUIFlagSet(((unsigned int)handle.handle >> 19) & 3, 32)
        && CG_SoundEntInUse(handle)
        && (((unsigned int)handle.handle >> 12) & 0x7F) == (CG_SoundGetUseCount(handle) & 0x7F) )
    {
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_SET_ENT_STATE;
            cmd->context.set_ent_state.handle = handle;
            CG_GetSoundEntityOrientation(handle, cmd->context.set_ent_state.origin, cmd->context.set_ent_state.orientation, cmd->context.set_ent_state.velocity);
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_PlayLoopAt(unsigned int id, const float *origin)
{
    if ( SND_Active() )
    {
        nanassertvec3(origin);

        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_PLAY_LOOP_AT;
            cmd->context.loop_at.id = id;
            cmd->context.loop_at.origin[0] = origin[0];
            cmd->context.loop_at.origin[1] = origin[1];
            cmd->context.loop_at.origin[2] = origin[2];
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_StopLoopAt(unsigned int id, const float *origin)
{
    if ( SND_Active() )
    {
        nanassertvec3(origin);

        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_STOP_LOOP_AT;
            cmd->context.loop_at.id = id;
            cmd->context.loop_at.origin[0] = origin[0];
            cmd->context.loop_at.origin[1] = origin[1];
            cmd->context.loop_at.origin[2] = origin[2];
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_PlayLineAt(unsigned int id, const float *origin0, const float *origin1)
{
    snd_command *cmd; // [esp+20h] [ebp-4h]

    if ( SND_Active() )
    {
        nanassertvec3(origin0);
        nanassertvec3(origin1);

        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_PLAY_LINE_AT;
            cmd->context.line_at.id = id;
            cmd->context.line_at.origin[0][0] = origin0[0];
            cmd->context.line_at.origin[0][1] = origin0[1];
            cmd->context.line_at.origin[0][2] = origin0[2];
            cmd->context.line_at.origin[1][0] = origin1[0];
            cmd->context.line_at.origin[1][1] = origin1[1];
            cmd->context.line_at.origin[1][2] = origin1[2];
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_StopLineAt(unsigned int id, const float *origin0, const float *origin1)
{
    snd_command *cmd; // [esp+20h] [ebp-4h]

    if ( SND_Active() )
    {
        nanassertvec3(origin0);
        nanassertvec3(origin1);

        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_STOP_LINE_AT;
            cmd->context.line_at.id = id;
            cmd->context.line_at.origin[0][0] = origin0[0];
            cmd->context.line_at.origin[0][1] = origin0[1];
            cmd->context.line_at.origin[0][2] = origin0[2];
            cmd->context.line_at.origin[1][0] = origin1[0];
            cmd->context.line_at.origin[1][1] = origin1[1];
            cmd->context.line_at.origin[1][2] = origin1[2];
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_SetContext(const char *type, const char *value)
{
    unsigned int v2; // eax
    unsigned int v3; // eax
    unsigned int v4; // [esp-4h] [ebp-8h]
    snd_command *cmd; // [esp+0h] [ebp-4h]

    if ( SND_Active() )
    {
        if ( (SND_FindContextIndex(SND_HashName(type)) & 0x80000000) == 0 )
        {
            if ( SND_HashName(value)
                && (v4 = SND_HashName(value), v3 = SND_HashName(type), SND_FindContextValueIndex(v3, v4) < 0) )
            {
                Com_PrintError(9, "Unknown sound context %s\n", value);
            }
            else
            {
                cmd = SND_GetNewCommand();
                if ( cmd )
                {
                    cmd->type = SND_COMMAND_SET_CONTEXT;
                    cmd->context.set_context.type = SND_HashName(type);
                    cmd->context.set_context.value = SND_HashName(value);
                    SND_CommandPush(cmd);
                }
            }
        }
        else
        {
            Com_PrintError(9, "Unknown sound context %s\n", type);
        }
    }
}

void __cdecl SND_SetScriptTimescale(float value)
{
    if ( SND_Active() )
    {
        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_SCRIPT_TIMESCALE;
            cmd->context.script_timescale.value = value;
            SND_CommandPush(cmd);
        }
    }
}

snd_ent_state *__cdecl SND_FindEntState(SndEntHandle handle, bool createNew)
{
    iassert(handle.field.entIndex != SND_ENT_NONE);
    iassert(!handle.field.isStationary);
    bcassert(handle.field.entIndex, ARRAY_COUNT(g_snd.entStateIndex));

    if (handle.field.isStationary || handle.field.entIndex == SND_ENT_NONE || handle.field.entIndex == SND_ENT_NO_STOP)
    {
        return NULL;
    }

    bcassert(handle.field.entIndex, MAX_LOCAL_CENTITIES);

    if (handle.field.entIndex >= MAX_LOCAL_CENTITIES) // lwss: changed to >= instead of >
    {
        Com_Error(ERR_DROP, "Invalid sound handle");
    }

    snd_ent_state *state = g_snd.entStateIndex[handle.field.entIndex];

    for (int i = 0; state && handle.handle != state->handle.handle && i < 0x64; ++i)
    {
        iassert(i < 50);
        iassert(state != state->next);
        iassert(handle.field.entIndex == state->handle.field.entIndex);
        iassert(state->lastUsed);

        state = state->next;
    }

    if (createNew && !state)
    {
        for (int j = 0; j < 0x128; ++j)
        {
            if (!g_snd.entState[j].lastUsed)
            {
                state = &g_snd.entState[j];
                iassert(state != g_snd.entStateIndex[handle.field.entIndex]);
                state->lastUsed = g_snd.frame;
                state->handle = handle;
                state->next = g_snd.entStateIndex[handle.field.entIndex];
                g_snd.entStateIndex[handle.field.entIndex] = state;
                break;
            }
        }
        if (!state)
        {
            Com_Printf(9, "Out of ent state cache entries at time %d\n", g_snd.frame);
            for (int k = 0; k < 0x128; ++k)
                Com_Printf(
                    9,
                    "%d - %x %d %d\n",
                    k,
                    g_snd.entState[k].handle.handle,
                    g_snd.entState[k].lastUsed,
                    g_snd.entState[k].next - g_snd.entState);
        }
        iassert(state);
    }
    return state;
}

void __cdecl SND_UpdateEntState(
                SndEntHandle handle,
                const float *origin,
                const float *velocity,
                const float (*orientation)[3])
{
    iassert(!handle.field.isStationary);
    nanassertvec3(origin);
    nanassertvec3(velocity);
    nanassertvec3(orientation[0]);
    nanassertvec3(orientation[1]);
    nanassertvec3(orientation[2]);

    snd_ent_state *state = SND_FindEntState(handle, 1);

    if ( state )
    {
        iassert(state->handle.handle == handle.handle);
  
        Vec3Copy(origin, state->origin);
        Vec3Copy(velocity, state->velocity);
        AxisCopy(orientation, state->orientation);
    }
}

char __cdecl SND_GetEntState(SndEntHandle handle, float *origin, float *velocity, float (*orientation)[3])
{
    snd_ent_state *state = SND_FindEntState(handle, 0);
    if ( state )
    {
        Vec3Copy(state->origin, origin);
        Vec3Copy(state->velocity, velocity);
        AxisCopy(state->orientation, orientation);
        return 1;
    }
    else
    {
        SND_EntStateRequest(handle);
        return 0;
    }
}

void __cdecl SNDL_SetEntState(
                SndEntHandle handle,
                const float *origin,
                const float *velocity,
                const float (*orientation)[3])
{
    SND_UpdateEntState(handle, origin, velocity, orientation);
}

void __cdecl SND_EntStateFrame()
{
    PROF_SCOPED("SND_EntStateFrame");

    for (int i = 0; i < MAX_LOCAL_CENTITIES; ++i)
    {
        snd_ent_state *state = g_snd.entStateIndex[i];
        for (int j = 0; state && j < 0x64; ++j)
        {
            iassert(j < 50);
            iassert(i == state->handle.field.entIndex);
            iassert(state != state->next);
            iassert(state->lastUsed);
        
            state = state->next;
        }
    }

    for (int k = 0; k < SND_MAX_VOICES; ++k)
    {
        if (g_snd.voiceAliasHash[k])
        {
            if ((((unsigned int)g_snd.voice[k].sndEnt.handle >> 21) & 1) == 0
                && (g_snd.voice[k].sndEnt.handle & 0xFFF) != 0xFFF)
            {
                snd_ent_state *state = SND_FindEntState(g_snd.voice[k].sndEnt, 0);
                if (state)
                {
                    if (state->lastUsed != g_snd.frame)
                        SND_EntStateRequest(g_snd.voice[k].sndEnt);
                    state->lastUsed = g_snd.frame;
                }
            }
        }
    }
    for (int m = 0; m < 0x128; ++m)
    {
        snd_ent_state *state = &g_snd.entState[m];
        if (state->lastUsed && g_snd.frame - state->lastUsed > 2)
        {
            iassert(g_snd.frame >= state->lastUsed);
            bcassert(state->handle.field.entIndex, ARRAY_COUNT(g_snd.entStateIndex));
            snd_ent_state **list = &g_snd.entStateIndex[state->handle.handle & 0xFFF];
            bool removed = 0;
            for (int n = 0; *list && n < 0x64; ++n)
            {
                if (*list == state)
                {
                    *list = (*list)->next;
                    removed = 1;
                    break;
                }
                list = &(*list)->next;
            }
            iassert(removed);
            state->lastUsed = 0;
            state->next = 0;
        }
    }
}

void __cdecl SND_ResetEntState()
{
    for (int i = 0; i < 0x128; ++i)
        g_snd.entState[i].lastUsed = 0;
    for (int j = 0; j < MAX_LOCAL_CENTITIES; ++j)
        g_snd.entStateIndex[j] = 0;
}

void __cdecl SND_EntStateRequest(SndEntHandle handle)
{
    snd_notify *cmd = SND_GetNewNotify();
    if ( cmd )
    {
        cmd->type = SND_NOTIFY_ENT_UPDATE;
        cmd->context.ent_update.handle = handle;
        SND_NotifyPush(cmd);
    }
}

void __cdecl SND_SubtitleNotify(const char *subtitle, unsigned int lengthMs)
{
    snd_notify *cmd = SND_GetNewNotify();
    if ( cmd )
    {
        cmd->type = SND_NOTIFY_SUBTITLE;
        cmd->context.subtitle.subtitle = subtitle;
        cmd->context.subtitle.lengthMs = lengthMs;
        SND_NotifyPush(cmd);
    }
}

void __cdecl SND_LengthNotify(unsigned int ent, unsigned int lengthMs)
{
    snd_notify *cmd = SND_GetNewNotify();
    if ( cmd )
    {
        cmd->type = SND_NOTIFY_LENGTH;
        cmd->context.length.ent = ent;
        cmd->context.length.lengthMs = lengthMs;
        SND_NotifyPush(cmd);
    }
}

void __cdecl SND_FreePlaybackNotify(snd_playback *playback)
{
    iassert(playback);

    snd_notify *cmd = SND_GetNewNotify();
    if ( cmd )
    {
        cmd->type = SND_NOTIFY_PLAYBACK_FREE;
        cmd->context.playback_free.playback = playback;
        SND_NotifyPush(cmd);
    }
}

void __cdecl SND_ResetPlaybacks()
{
    for ( int i = 0; i < SND_PLAYBACK_COUNT; ++i )
        g_snd.playbacks[i].id = SND_PLAYBACKID_NOTPLAYED;
}

snd_playback *__cdecl SND_AllocatePlayback()
{
    snd_playback *playback = NULL;
    Sys_EnterCriticalSection(CRITSECT_SOUND_PLAYBACK_ALLOC);
    for (uint i = 0; i < SND_PLAYBACK_COUNT; ++i)
    {
        if (g_snd.playbacks[i].id == SND_PLAYBACKID_NOTPLAYED)
        {
            g_snd.playbacks[i].id = SND_AcquirePlaybackId();
            g_snd.playbacks[i].attenuation = 1.0f;
            g_snd.playbacks[i].lengthMs = 0;
            g_snd.playbacks[i].playedMs = 0;
            playback = &g_snd.playbacks[i];
            ++g_snd.playbacksInUse;
            break;
        }
    }
    if (!playback)
        Com_PrintError(9, "ran out of playbacks\n");
    Sys_LeaveCriticalSection(CRITSECT_SOUND_PLAYBACK_ALLOC);
    return playback;
}

void __cdecl SND_FreePlayback(snd_playback *playback)
{
    if (playback)
    {
        bcassert(playback - g_snd.playbacks, SND_PLAYBACK_COUNT);
        playback->id = SND_PLAYBACKID_NOTPLAYED;
        g_snd.playbacksInUse--;
    }
}

bool __cdecl SND_IsPlaying(int playbackId)
{
    return SND_FindPlayback(playbackId) != 0;
}

snd_playback *__cdecl SND_FindPlayback(int playbackId)
{
    unsigned int i; // [esp+0h] [ebp-8h]
    snd_playback *playback; // [esp+4h] [ebp-4h]

    playback = 0;
    Sys_EnterCriticalSection(CRITSECT_SOUND_PLAYBACK_ALLOC);
    for ( i = 0; i < SND_PLAYBACK_COUNT; ++i )
    {
        if ( g_snd.playbacks[i].id == playbackId )
        {
            playback = &g_snd.playbacks[i];
            break;
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_SOUND_PLAYBACK_ALLOC);
    return playback;
}

int __cdecl SND_GetPlaybackTime(int playbackId)
{
    snd_playback *playback; // [esp+4h] [ebp-4h]

    playback = SND_FindPlayback(playbackId);
    if ( playback )
        return playback->playedMs;
    else
        return 0;
}

bool __cdecl SND_GetKnownLength(int playbackId, int *msec)
{
    snd_playback *playback = SND_FindPlayback(playbackId);

    if ( playback )
    {
        *msec = playback->lengthMs;
        if ( playback->lengthMs )
            return true;
    }
    return false;
}

void SND_Update()
{
    unsigned int unused; // [esp+0h] [ebp-4h] BYREF

    //unused = (unsigned int)this;
    if ( SND_Active() )
    {
        Sys_AssistAndWaitWorkerCmdInternal(&updatesound_workerWorkerCmd);
        Sys_AddWorkerCmdInternal(&updatesound_workerWorkerCmd, (unsigned __int8 *)&unused, 0);
    }
}

void __cdecl SND_UpdateWait()
{
    if ( SND_Active() )
    {
        SND_Update();
        Sys_AssistAndWaitWorkerCmdInternal(&updatesound_workerWorkerCmd);
    }
}

int __cdecl updatesound_workerCallback(jqBatch *batch)
{
    PROF_SCOPED("updatesound_worker");
    jqLockData(batch);
    jqUnlockData(batch);
    SND_Frame();
    return 0;
}

void SND_Frame()
{
    iassert(entryCount == 0);
    _InterlockedExchangeAdd(&entryCount, 1u);
    iassert(entryCount == 1);
    
    {
        PROF_SCOPED("SND_Frame");

        SND_EntStateFrame();
        SND_CommandPump();
        SNDL_Update();
    }
    
    iassert(entryCount == 1);
    
    _InterlockedExchangeAdd(&entryCount, 0xFFFFFFFF);

    iassert(entryCount == 0);
}

void __cdecl SND_GameReset()
{
    if ( SND_Active() )
    {
        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_GAME_RESET;
            SND_CommandPush(cmd);
        }
        SND_UpdateWait();
    }
}

void __cdecl SND_BeginFrame(bool isMature, bool isPaused, float timescale, unsigned int cgTime, unsigned int seed)
{
    PROF_SCOPED("SND_BeginFrame"); // LWSS ADD

    if ( SND_Active() )
    {
        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_SET_GAME_STATE;
            cmd->context.set_game_state.is_mature = isMature;
            cmd->context.set_game_state.is_paused = isPaused;
            cmd->context.set_game_state.timescale = timescale;
            cmd->context.set_game_state.cg_time = cgTime;
            cmd->context.set_game_state.seed = seed;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_EndFrame()
{
    if ( SND_Active() )
    {
        SND_NotifyPump();
        snd_command *cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_UPDATE_LOOPS;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_AsyncInit()
{
    SND_InitCommands();
    SNDL_GameReset();
}

