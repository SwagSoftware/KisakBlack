#include "snd_public_async_q.h"

#include <cstring>
#include <win32/win_common.h>
#include <qcommon/common.h>
#include <win32/win_shared.h>
#include "snd_log.h"
#include "snd_local.h"
#include "snd_public_async.h"
#include <cgame/cg_sound.h>

void __cdecl SND_InitCommands()
{
    memset(g_snd.commands, 0, sizeof(g_snd.commands));
    memset(g_snd.command_q, 0, sizeof(g_snd.command_q));
    g_snd.command_q_head = 0;
    g_snd.command_q_tail = 0;
    g_snd.command_init = 1;
    g_snd.command_process_flag = 1;
}

snd_command *__cdecl SND_GetNewCommand()
{
    snd_command *cmd; // [esp+0h] [ebp-4h]

    if (!g_snd.command_init
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            84,
            0,
            "%s",
            "g_snd.command_init"))
    {
        __debugbreak();
    }
    cmd = 0;
    Sys_EnterCriticalSection(CRITSECT_SOUND_COMMAND_ALLOC);
    if (g_snd.command_q_tail >= 0x400
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            90,
            0,
            "%s",
            "g_snd.command_q_tail < SND_COMMAND_Q_COUNT"))
    {
        __debugbreak();
    }
    if (g_snd.command_q_head >= 0x400
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            91,
            0,
            "%s",
            "g_snd.command_q_head < SND_COMMAND_Q_COUNT"))
    {
        __debugbreak();
    }
    if (g_snd.commands[g_snd.command_free].type)
    {
        Com_PrintError(
            9,
            "No command space in sound queue. SND_Update is not being called enough or someone is playing way to many sounds at once\n");
    }
    else
    {
        cmd = &g_snd.commands[g_snd.command_free];
        g_snd.command_free = SND_NextCommandIndex(g_snd.command_free);
    }
    Sys_LeaveCriticalSection(CRITSECT_SOUND_COMMAND_ALLOC);
    return cmd;
}

int __cdecl SND_NextCommandIndex(__int16 i)
{
    return (i + 1) & 0x3FF;
}

unsigned int __cdecl SND_CommandPush(snd_command *cmd)
{
    unsigned int id; // [esp+4h] [ebp-4h]

    if ( !g_snd.command_init
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
                    111,
                    0,
                    "%s",
                    "g_snd.command_init") )
    {
        __debugbreak();
    }
    Sys_EnterCriticalSection(CRITSECT_SOUND_COMMAND_PUSH);
    if ( g_snd.command_q_tail >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
                    124,
                    0,
                    "%s",
                    "g_snd.command_q_tail <SND_COMMAND_Q_COUNT") )
    {
        __debugbreak();
    }
    if ( g_snd.command_q_head >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
                    125,
                    0,
                    "%s",
                    "g_snd.command_q_head <SND_COMMAND_Q_COUNT") )
    {
        __debugbreak();
    }
    //if ( (cmd < g_snd.commands || cmd >= (snd_command *)&g_snd.command_free)// KISAKTODOmake 
    //    && !Assert_MyHandler(
    //                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
    //                126,
    //                0,
    //                "%s",
    //                "cmd >= g_snd.commands && cmd < g_snd.commands+SND_COMMAND_Q_COUNT") )
    //{
    //    __debugbreak();
    //}
    if ( cmd->type == SND_COMMAND_NOP
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
                    127,
                    0,
                    "%s",
                    "cmd->type != SND_COMMAND_NOP") )
    {
        __debugbreak();
    }
    id = g_snd.command_id++;
    if ( SND_NextCommandIndex(g_snd.command_q_head) == g_snd.command_q_tail )
    {
        Com_PrintError(9, "Failed to push sound command: queue is full\n");
        memset(cmd, 0, sizeof(snd_command));
        id = 0;
    }
    else
    {
        cmd->id = id;
        if ( g_snd.command_q[g_snd.command_q_head]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
                        134,
                        0,
                        "%s",
                        "g_snd.command_q[g_snd.command_q_head] == 0") )
        {
            __debugbreak();
        }
        cmd->timestamp = Sys_Milliseconds();
        g_snd.command_q[g_snd.command_q_head] = cmd;
        g_snd.command_q_head = SND_NextCommandIndex(g_snd.command_q_head);
        if ( g_snd.command_q_head < g_snd.command_q_tail )
            SND_LogCommandQHWM(g_snd.command_q_head + 1024 - g_snd.command_q_tail);
        else
            SND_LogCommandQHWM(g_snd.command_q_head - g_snd.command_q_tail);
    }
    Sys_LeaveCriticalSection(CRITSECT_SOUND_COMMAND_PUSH);
    return id;
}

const snd_command *__cdecl SND_CommandPeek()
{
    if ( !g_snd.command_init
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
                    166,
                    0,
                    "%s",
                    "g_snd.command_init") )
    {
        __debugbreak();
    }
    if ( g_snd.command_q_tail >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
                    169,
                    0,
                    "%s",
                    "g_snd.command_q_tail <SND_COMMAND_Q_COUNT") )
    {
        __debugbreak();
    }
    if ( g_snd.command_q_head >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
                    170,
                    0,
                    "%s",
                    "g_snd.command_q_head <SND_COMMAND_Q_COUNT") )
    {
        __debugbreak();
    }
    if ( g_snd.command_q_tail == g_snd.command_q_head )
        return 0;
    else
        return g_snd.command_q[g_snd.command_q_tail];
}

void __cdecl SND_CommandPop()
{
    if (!g_snd.command_init
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            178,
            0,
            "%s",
            "g_snd.command_init"))
    {
        __debugbreak();
    }
    if (g_snd.command_q_tail >= 0x400
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            180,
            0,
            "%s",
            "g_snd.command_q_tail < SND_COMMAND_Q_COUNT"))
    {
        __debugbreak();
    }
    if (g_snd.command_q_head >= 0x400
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            181,
            0,
            "%s",
            "g_snd.command_q_head < SND_COMMAND_Q_COUNT"))
    {
        __debugbreak();
    }
    if (g_snd.command_q_tail == g_snd.command_q_head
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            182,
            0,
            "%s",
            "g_snd.command_q_tail != g_snd.command_q_head"))
    {
        __debugbreak();
    }
    if (g_snd.command_q[g_snd.command_q_tail]->type == SND_COMMAND_NOP
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            183,
            0,
            "%s",
            "g_snd.command_q[g_snd.command_q_tail]->type != SND_COMMAND_NOP"))
    {
        __debugbreak();
    }
    g_snd.command_q[g_snd.command_q_tail]->type = SND_COMMAND_NOP;
    g_snd.command_q[g_snd.command_q_tail] = 0;
    g_snd.command_q_tail = SND_NextCommandIndex(g_snd.command_q_tail);
}

unsigned int __cdecl SND_CommandPump()
{
    unsigned int count; // [esp+8h] [ebp-8h]
    const snd_command *cmd; // [esp+Ch] [ebp-4h]

    PROF_SCOPED("SND_CommandPump");

    if (!g_snd.command_init
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            456,
            0,
            "%s",
            "g_snd.command_init"))
    {
        __debugbreak();
    }
    count = 0;
    for (cmd = SND_CommandPeek(); cmd; cmd = SND_CommandPeek())
    {
        ++count;
        if (SND_GetProcessCommandFlag())
        {
            SND_CommandProcess(cmd);
            g_snd.command_id_commit = cmd->id;
        }
        SND_CommandPop();
    }

    return count;
}

void __cdecl SND_CommandProcess(const snd_command *cmd)
{
    if ( !g_snd.command_init
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
                    195,
                    0,
                    "%s",
                    "g_snd.command_init") )
    {
        __debugbreak();
    }
    switch ( cmd->type )
    {
        case SND_COMMAND_ALIAS_NAME:
            //PIXBeginNamedEvent(-1, "SNDL_AliasName");
            SNDL_AliasName((char*)cmd->context.alias_name.name, cmd->context.alias_name.id);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_65;
            break;
        case SND_COMMAND_PLAY:
            //PIXBeginNamedEvent(-1, "SNDL_Play");
            SNDL_Play(
                cmd->context.play.alias,
                cmd->context.play.fadeTimeMs,
                cmd->context.play.attenuation,
                cmd->context.play.entHandle,
                (float*)cmd->context.play.position,
                cmd->context.play.direction,
                cmd->context.play.notify != 0,
                cmd->context.play.playback);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_65;
            break;
        case SND_COMMAND_STOP_ALIAS:
            //PIXBeginNamedEvent(-1, "SNDL_StopSoundAliasOnEnt");
            SNDL_StopSoundAliasOnEnt(cmd->context.stop_alias.ent, cmd->context.stop_alias.alias_name);
            //if ( GetCurrentThreadId() == g_DXDeviceThread )
                goto LABEL_65;
            break;
        case SND_COMMAND_STOP_ENT:
            //PIXBeginNamedEvent(-1, "SNDL_StopSoundsOnEnt");
            SNDL_StopSoundsOnEnt(cmd->context.stop_alias.ent);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_65;
            break;
        case SND_COMMAND_CINEMATIC_START:
            //PIXBeginNamedEvent(-1, "SNDL_NotifyCinematicStart");
            SNDL_NotifyCinematicStart(cmd->context.notify_cinematic_start.volume);
            //if ( GetCurrentThreadId() == g_DXDeviceThread )
                goto LABEL_65;
            break;
        case SND_COMMAND_CINEMATIC_END:
            //PIXBeginNamedEvent(-1, "SNDL_NotifyCinematicEnd");
            SNDL_NotifyCinematicEnd();
            //if ( GetCurrentThreadId() == g_DXDeviceThread )
                goto LABEL_65;
            break;
        case SND_COMMAND_DISCONNECT_LISTENER:
            //PIXBeginNamedEvent(-1, "SNDL_DisconnectListener");
            SNDL_DisconnectListener(cmd->context.stop_alias.ent.handle);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_65;
            break;
        case SND_COMMAND_SET_LISTENER:
            //PIXBeginNamedEvent(-1, "SNDL_SetListener");
            SNDL_SetListener(
                cmd->context.stop_alias.ent.handle,
                cmd->context.play.fadeTimeMs,
                cmd->context.set_listener.team,
                cmd->context.set_listener.origin,
                (const float (*)[3])&cmd->context.play.position[1]);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_65;
            break;
        case SND_COMMAND_STOP_SOUNDS:
            //PIXBeginNamedEvent(-1, "SNDL_StopSounds");
            SNDL_StopSounds(cmd->context.stop_sounds.flags);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_65;
            break;
        case SND_COMMAND_FADE_IN:
            //PIXBeginNamedEvent(-1, "SNDL_FadeIn");
            SNDL_FadeIn();
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_65;
            break;
        case SND_COMMAND_FADE_OUT:
            //PIXBeginNamedEvent(-1, "SNDL_FadeOut");
            SNDL_FadeOut();
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_65;
            break;
        case SND_COMMAND_SET_ENVIRONMENT_EFFECTS:
            //PIXBeginNamedEvent(-1, "SNDL_SetEnvironmentEffects");
            SNDL_SetEnvironmentEffects(
                cmd->context.stop_alias.ent.handle,
                cmd->context.stop_alias.alias_name,
                cmd->context.play.attenuation,
                cmd->context.set_listener.origin[0],
                cmd->context.play.notify);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_65;
            break;
        case SND_COMMAND_DEACTIVATE_ENVIRONMENT_EFFECTS:
            //PIXBeginNamedEvent(-1, "SNDL_DeactivateEnvironmentEffects");
            SNDL_DeactivateEnvironmentEffects(cmd->context.stop_alias.ent.handle, cmd->context.play.fadeTimeMs);
            //if ( GetCurrentThreadId() == g_DXDeviceThread )
                goto LABEL_65;
            break;
        case SND_COMMAND_SET_PLAYBACK_ATTENUATION:
            //PIXBeginNamedEvent(-1, "SNDL_SetPlaybackAttenuation");
            SNDL_SetPlaybackAttenuation(cmd->context.stop_alias.ent.handle, cmd->context.set_playback_attenuation.attenuation);
            //if ( GetCurrentThreadId() == g_DXDeviceThread )
                goto LABEL_65;
            break;
        case SND_COMMAND_SET_PLAYBACK_ATTENUATION_RATE:
            //PIXBeginNamedEvent(-1, "SNDL_SetPlaybackAttenuationRate");
            SNDL_SetPlaybackAttenuationRate(
                cmd->context.stop_alias.ent.handle,
                cmd->context.set_playback_attenuation.attenuation);
            //if ( GetCurrentThreadId() == g_DXDeviceThread )
                goto LABEL_65;
            break;
        case SND_COMMAND_SET_PLAYBACK_PITCH:
            //PIXBeginNamedEvent(-1, "SNDL_SetPlaybackPitch");
            SNDL_SetPlaybackPitch(cmd->context.stop_alias.ent.handle, cmd->context.set_playback_attenuation.attenuation);
            //if ( GetCurrentThreadId() == g_DXDeviceThread )
                goto LABEL_65;
            break;
        case SND_COMMAND_SET_PLAYBACK_PITCH_RATE:
            //PIXBeginNamedEvent(-1, "SNDL_SetPlaybackPitchRate");
            SNDL_SetPlaybackPitchRate(cmd->context.stop_alias.ent.handle, cmd->context.set_playback_attenuation.attenuation);
            //if ( GetCurrentThreadId() == g_DXDeviceThread )
                goto LABEL_65;
            break;
        case SND_COMMAND_STOP_PLAYBACK:
            //PIXBeginNamedEvent(-1, "SNDL_StopPlayback");
            SNDL_StopPlayback(cmd->context.stop_alias.ent.handle);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_65;
            break;
        case SND_COMMAND_SNAPSHOT:
            //PIXBeginNamedEvent(-1, "SNDL_SetSnapshot");
            SNDL_SetSnapshot(
                cmd->context.snapshot.type,
                cmd->context.stop_alias.alias_name,
                cmd->context.play.attenuation,
                cmd->context.set_listener.origin[0]);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_65;
            break;
        case SND_COMMAND_SET_ENT_STATE:
            //PIXBeginNamedEvent(-1, "SNDL_SetEntState");
            SNDL_SetEntState(
                cmd->context.stop_alias.ent,
                &cmd->context.set_playback_attenuation.attenuation,
                &cmd->context.set_listener.origin[1],
                (const float (*)[3])&cmd->context.play.position[2]);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_65;
            break;
        case SND_COMMAND_SET_GAME_STATE:
            //PIXBeginNamedEvent(-1, "SNDL_SetGameState");
            SNDL_SetGameState(
                cmd->context.alias_name.name[1],
                cmd->context.alias_name.name[0],
                cmd->context.set_playback_attenuation.attenuation,
                cmd->context.set_game_state.cg_time,
                cmd->context.set_game_state.seed);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_65;
            break;
        case SND_COMMAND_PLAY_LOOP_AT:
            //PIXBeginNamedEvent(-1, "SNDL_PlayLoopAt");
            SNDL_PlayLoopAt(cmd->context.play.alias, (float*)&cmd->context.set_playback_attenuation.attenuation);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_65;
            break;
        case SND_COMMAND_STOP_LOOP_AT:
            //PIXBeginNamedEvent(-1, "SNDL_StopLoopAt");
            SNDL_StopLoopAt(cmd->context.play.alias, &cmd->context.set_playback_attenuation.attenuation);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_65;
            break;
        case SND_COMMAND_PLAY_LINE_AT:
            //PIXBeginNamedEvent(-1, "SNDL_PlayLineAt");
            SNDL_PlayLineAt(
                cmd->context.play.alias,
                (float*)&cmd->context.set_playback_attenuation.attenuation,
                (float*)&cmd->context.set_listener.origin[1]);
            //if ( GetCurrentThreadId() == g_DXDeviceThread )
                goto LABEL_65;
            break;
        case SND_COMMAND_STOP_LINE_AT:
            //PIXBeginNamedEvent(-1, "SNDL_StopLineAt");
            SNDL_StopLineAt(
                cmd->context.play.alias,
                &cmd->context.set_playback_attenuation.attenuation,
                &cmd->context.set_listener.origin[1]);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_65;
            break;
        case SND_COMMAND_GAME_RESET:
            //PIXBeginNamedEvent(-1, "SNDL_GameReset");
            SNDL_GameReset();
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_65;
            break;
        case SND_COMMAND_SET_CONTEXT:
            //PIXBeginNamedEvent(-1, "SNDL_SetContext");
            SNDL_SetContext(cmd->context.play.alias, cmd->context.stop_alias.alias_name);
            //if ( GetCurrentThreadId() == g_DXDeviceThread )
                goto LABEL_65;
            break;
        case SND_COMMAND_SCRIPT_TIMESCALE:
            //PIXBeginNamedEvent(-1, "SNDL_SetContext");
            SNDL_SetScriptTimescale(cmd->context.notify_cinematic_start.volume);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_65;
            break;
        case SND_COMMAND_UPDATE_LOOPS:
            //PIXBeginNamedEvent(-1, "SNDL_UpdateLoopingSounds");
            SNDL_UpdateLoopingSounds();
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
LABEL_65:
                //D3DPERF_EndEvent();
            break;
        default:
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp", 200, 0, "%s", "0") )
                __debugbreak();
            break;
    }
}

snd_notify *__cdecl SND_GetNewNotify()
{
    snd_notify *cmd; // [esp+0h] [ebp-4h]

    if (!g_snd.command_init
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            491,
            0,
            "%s",
            "g_snd.command_init"))
    {
        __debugbreak();
    }
    cmd = 0;
    Sys_EnterCriticalSection(CRITSECT_SOUND_NOTIFY_ALLOC);
    if (g_snd.notify_q_tail >= 0x200
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            497,
            0,
            "%s",
            "g_snd.notify_q_tail < SND_NOTIFY_Q_COUNT"))
    {
        __debugbreak();
    }
    if (g_snd.notify_q_head >= 0x200
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            498,
            0,
            "%s",
            "g_snd.notify_q_head < SND_NOTIFY_Q_COUNT"))
    {
        __debugbreak();
    }
    if (g_snd.notifies[g_snd.notify_free].type)
    {
        Com_PrintError(9, "No notify space in sound queue.\n");
    }
    else
    {
        cmd = &g_snd.notifies[g_snd.notify_free];
        g_snd.notify_free = SND_NextNotifyIndex(g_snd.notify_free);
    }
    Sys_LeaveCriticalSection(CRITSECT_SOUND_NOTIFY_ALLOC);
    return cmd;
}

int __cdecl SND_NextNotifyIndex(__int16 i)
{
    return (i + 1) & 0x1FF;
}

void __cdecl SND_NotifyPush(snd_notify *cmd)
{
    //PIXBeginNamedEvent(-1, "SND_NotifyPush");
    if (!g_snd.command_init
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            577,
            0,
            "%s",
            "g_snd.command_init"))
    {
        __debugbreak();
    }
    Sys_EnterCriticalSection(CRITSECT_SOUND_NOTIFY_PUSH);
    if (g_snd.notify_q_tail >= 0x200
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            581,
            0,
            "%s",
            "g_snd.notify_q_tail <SND_NOTIFY_Q_COUNT"))
    {
        __debugbreak();
    }
    if (g_snd.notify_q_head >= 0x200
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            582,
            0,
            "%s",
            "g_snd.notify_q_head <SND_NOTIFY_Q_COUNT"))
    {
        __debugbreak();
    }
    //if ((cmd < g_snd.notifies || cmd >= (snd_notify *)&g_snd.notify_free) // KISAKTODO
    //    && !Assert_MyHandler(
    //        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
    //        583,
    //        0,
    //        "%s",
    //        "cmd >= g_snd.notifies && cmd < g_snd.notifies+SND_NOTIFY_Q_COUNT"))
    //{
    //    __debugbreak();
    //}
    if (SND_NextNotifyIndex(g_snd.notify_q_head) == g_snd.notify_q_tail
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            584,
            0,
            "%s",
            "SND_NextNotifyIndex(g_snd.notify_q_head) != g_snd.notify_q_tail"))
    {
        __debugbreak();
    }
    if (cmd->type == SND_NOTIFY_NOP
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            585,
            0,
            "%s",
            "cmd->type != SND_NOTIFY_NOP"))
    {
        __debugbreak();
    }
    if (SND_NextNotifyIndex(g_snd.notify_q_head) != g_snd.notify_q_tail)
    {
        if (g_snd.notify_q[g_snd.notify_q_head]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
                590,
                0,
                "%s",
                "g_snd.notify_q[g_snd.notify_q_head] == 0"))
        {
            __debugbreak();
        }
        g_snd.notify_q[g_snd.notify_q_head] = cmd;
        g_snd.notify_q_head = SND_NextNotifyIndex(g_snd.notify_q_head);
        if (g_snd.notify_q_head < g_snd.notify_q_tail)
            SND_LogNotifyQHWM(g_snd.notify_q_head + 512 - g_snd.notify_q_tail);
        else
            SND_LogNotifyQHWM(g_snd.notify_q_head - g_snd.notify_q_tail);
    }
    Sys_LeaveCriticalSection(CRITSECT_SOUND_NOTIFY_PUSH);
    //if (g_DXDeviceThread == GetCurrentThreadId())
    //    D3DPERF_EndEvent();
}

const snd_notify *__cdecl SND_NotifyPeek()
{
    if ( !g_snd.command_init
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
                    616,
                    0,
                    "%s",
                    "g_snd.command_init") )
    {
        __debugbreak();
    }
    if ( g_snd.notify_q_tail >= 0x200
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
                    619,
                    0,
                    "%s",
                    "g_snd.notify_q_tail <SND_NOTIFY_Q_COUNT") )
    {
        __debugbreak();
    }
    if ( g_snd.notify_q_head >= 0x200
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
                    620,
                    0,
                    "%s",
                    "g_snd.notify_q_head <SND_NOTIFY_Q_COUNT") )
    {
        __debugbreak();
    }
    if ( g_snd.notify_q_tail == g_snd.notify_q_head )
        return 0;
    else
        return g_snd.notify_q[g_snd.notify_q_tail];
}

void __cdecl SND_NotifyPop()
{
    if (g_snd.notify_q_tail >= 0x200
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            630,
            0,
            "%s",
            "g_snd.notify_q_tail < SND_NOTIFY_Q_COUNT"))
    {
        __debugbreak();
    }
    if (g_snd.notify_q_head >= 0x200
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            631,
            0,
            "%s",
            "g_snd.notify_q_head < SND_NOTIFY_Q_COUNT"))
    {
        __debugbreak();
    }
    if (g_snd.notify_q_tail == g_snd.notify_q_head
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            632,
            0,
            "%s",
            "g_snd.notify_q_tail != g_snd.notify_q_head"))
    {
        __debugbreak();
    }
    if (g_snd.notify_q[g_snd.notify_q_tail]->type == SND_NOTIFY_NOP
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
            633,
            0,
            "%s",
            "g_snd.notify_q[g_snd.notify_q_tail]->type != SND_NOTIFY_NOP"))
    {
        __debugbreak();
    }
    g_snd.notify_q[g_snd.notify_q_tail]->type = SND_NOTIFY_NOP;
    g_snd.notify_q[g_snd.notify_q_tail] = 0;
    g_snd.notify_q_tail = SND_NextNotifyIndex(g_snd.notify_q_tail);
}

void __cdecl SND_NotifyPump()
{
    const snd_notify *cmd; // [esp+8h] [ebp-4h]

    //PIXBeginNamedEvent(-1, "SND_NotifyPump");
    if ( !g_snd.command_init
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
                    646,
                    0,
                    "%s",
                    "g_snd.command_init") )
    {
        __debugbreak();
    }
    for ( cmd = SND_NotifyPeek(); cmd; cmd = SND_NotifyPeek() )
    {
        SND_NotifyProcess(cmd);
        SND_NotifyPop();
    }
    //if ( g_DXDeviceThread == GetCurrentThreadId() )
        //D3DPERF_EndEvent();
}

void __cdecl SND_NotifyProcess(const snd_notify *cmd)
{
    snd_playback *playback; // [esp+2Ch] [ebp-4h]

    switch ( cmd->type )
    {
        case SND_NOTIFY_ENT_UPDATE:
            //PIXBeginNamedEvent(-1, "SND_SetEntState");
            SND_SetEntState(cmd->context.ent_update.handle);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_8;
            break;
        case SND_NOTIFY_SUBTITLE:
            //PIXBeginNamedEvent(-1, "CG_SubtitleSndLengthNotify");
            CG_SubtitleSndLengthNotify(cmd->context.ent_update.handle.handle, cmd->context.subtitle.subtitle);
            //if ( GetCurrentThreadId() == g_DXDeviceThread )
                goto LABEL_8;
            break;
        case SND_NOTIFY_LENGTH:
            //PIXBeginNamedEvent(-1, "CG_ScriptSndLengthNotify");
            //BLOPS_NULLSUB();
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_8;
            break;
        case SND_NOTIFY_PLAYBACK_UPDATE:
            //PIXBeginNamedEvent(-1, "update playback");
            if ( !cmd->context.ent_update.handle.handle
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
                            552,
                            0,
                            "%s",
                            "cmd->context.playback_update.playback") )
            {
                __debugbreak();
            }
            if ( *(unsigned int *)cmd->context.ent_update.handle.handle == -1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp",
                            553,
                            0,
                            "%s",
                            "cmd->context.playback_update.playback->id != SND_PLAYBACKID_NOTPLAYED") )
            {
                __debugbreak();
            }
            playback = cmd->context.playback_free.playback;
            playback->attenuation = cmd->context.playback_update.attenuation;
            *(_QWORD *)&playback->lengthMs = *(_QWORD *)&cmd->context.playback_update.lengthMs;
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                goto LABEL_8;
            break;
        case SND_NOTIFY_PLAYBACK_FREE:
            //PIXBeginNamedEvent(-1, "SND_FreePlayback");
            SND_FreePlayback(cmd->context.playback_free.playback);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
LABEL_8:
                //D3DPERF_EndEvent();
            break;
        default:
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async_q.cpp", 525, 0, "%s", "0") )
                __debugbreak();
            break;
    }
}

bool __cdecl SND_GetProcessCommandFlag()
{
    return g_snd.command_process_flag;
}

