#include "snd.h"
#include <win32/win_main.h>
#include <universal/assertive.h>
#include "snd_public_async.h"
#include <universal/com_math.h>
#include <cfloat>
#include <cstring>

#include <Windows.h>
#include <win32/win_shared.h>
#include "snd_utils.h"
#include "snd_dvar.h"
#include "snd_occlusion.h"
#include <cgame/cg_compass.h>
#include <qcommon/common.h>
#include "snd_globals.h"
#include "snd_bank.h"
#include "snd_log.h"
#include "snd_driver_xaudio2.h"
#include <cgame/cg_sound.h>
#include "snd_debug.h"
#include <qcommon/threads.h>
#include "snd_db.h"
#include "snd_stream.h"
#include "snd_local.h"
#include <database/db_registry.h>

snd_local_t g_snd;

unsigned int occlusionTotal;
unsigned int occlusionOCacheHits;
unsigned int occlusionLevelHits;
unsigned int cache_misses;
unsigned int cache_total;

bool __cdecl SND_ShouldInit()
{
    if ( PC_StartWithNoSounds() )
        return 0;

    return !IsDedicatedServer() && IsFastFileLoad();
}

void __cdecl SND_SetPosition(unsigned int index, float *org)
{
    iassert((index >= 0 && index < (64 + 10)));
    Vec3Copy(org, g_snd.voice[index].position);
}

unsigned int __cdecl SND_ActiveListenerCount()
{
    unsigned int i; // [esp+0h] [ebp-8h]
    unsigned int count; // [esp+4h] [ebp-4h]

    count = 0;
    for ( i = 0; i < 1; i++ )
    {
        if ( g_snd.listeners[0].active )
            ++count;
    }
    return count;
}

int __cdecl SND_GetListenerIndexNearestToOrigin(const float *origin)
{
    float dist[1];
    int i;

    iassert(origin);

    for (i = 0; i < 1; ++i)
    {
        if (g_snd.listeners[i].active)
        {
            dist[i] = Vec3DistanceSq(origin, g_snd.listeners[i].orient.origin);
        }
        else
        {
            dist[i] = FLT_MAX;
        }
    }

    i = 1;
    int nearest = 0;
    while (i < 1)
    {
        if (g_snd.listeners[i].active && dist[nearest] > dist[i])
        {
            nearest = i;
        }
        ++i;
    }

    return nearest;
}

void __cdecl SND_GetNearestListenerPosition(const float *position, float *listener)
{
    snd_listener *v3; // edx
    int idx; // [esp+4h] [ebp-4h]

    idx = SND_GetListenerIndexNearestToOrigin(position);
    listener[0] = g_snd.listeners[idx].orient.origin[0];
    v3 = &g_snd.listeners[idx];
    listener[1] = v3->orient.origin[1];
    listener[2] = v3->orient.origin[2];
}

int __cdecl SND_SetPlaybackIdNotPlayed(unsigned int index)
{
    iassert(index >= 0 && index < SND_MAX_VOICES);
    g_snd.voice[index].playbackId = SND_PLAYBACKID_NOTPLAYED;
    return SND_PLAYBACKID_NOTPLAYED;
}

int __cdecl SND_AcquirePlaybackId()
{
    return _InterlockedIncrement(&g_snd.playbackIdCounter);
}

char __cdecl SND_AddLengthNotify(int playbackId, const char *lengthNotifyData, snd_length_type id)
{
    snd_voice_t *voice; // [esp+0h] [ebp-8h]
    int lengthNotifyIndex; // [esp+4h] [ebp-4h]

    if ( (unsigned int)id >= SND_LENGTH_NOTIFY_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    253,
                    0,
                    "id doesn't index SND_LENGTH_NOTIFY_COUNT\n\t%i not in [0, %i)",
                    id,
                    3) )
    {
        __debugbreak();
    }
    if ( playbackId == -1 )
    {
        DoLengthNotify(0, lengthNotifyData, id);
        return 1;
    }
    else
    {
        voice = SND_GetPlaybackVoice(playbackId);
        if ( voice )
        {
            if ( voice->totalMsec > 0 || (voice->alias->flags & 1) != 0 )
            {
                DoLengthNotify(voice->totalMsec, lengthNotifyData, id);
            }
            else
            {
                for ( lengthNotifyIndex = 0; lengthNotifyIndex < voice->lengthNotifyInfo.count; ++lengthNotifyIndex )
                {
                    if ( voice->lengthNotifyInfo.id[lengthNotifyIndex] == id
                        && voice->lengthNotifyInfo.data[lengthNotifyIndex] == lengthNotifyData )
                    {
                        return 1;
                    }
                }
                if ( (unsigned int)lengthNotifyIndex >= 4
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                                276,
                                0,
                                "lengthNotifyIndex doesn't index SND_LENGTHNOTIFY_COUNT\n\t%i not in [0, %i)",
                                lengthNotifyIndex,
                                4) )
                {
                    __debugbreak();
                }
                ++voice->lengthNotifyInfo.count;
                voice->lengthNotifyInfo.id[lengthNotifyIndex] = id;
                voice->lengthNotifyInfo.data[lengthNotifyIndex] = (void *)lengthNotifyData;
            }
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

void __cdecl DoLengthNotify(unsigned int msec, const char *lengthNotifyData, snd_length_type id)
{
    const char *v3; // eax

    if ( id == SND_LENGTH_NOTIFY_SCRIPT )
    {
        SND_LengthNotify((unsigned int)lengthNotifyData, msec);
    }
    else if ( id == SND_LENGTH_NOTIFY_SUBTITLE )
    {
        SND_SubtitleNotify(lengthNotifyData, msec);
    }
    else
    {
        v3 = va("Unknown snd length notify id: %i\n", id);
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 239, 0, v3) )
            __debugbreak();
    }
}

void __cdecl SND_StartLengthNotify(unsigned int index, unsigned int totalMsec)
{
    snd_voice_t *voice; // [esp+0h] [ebp-Ch]
    int lengthNotifyIndex; // [esp+4h] [ebp-8h]
    snd_length_type id; // [esp+8h] [ebp-4h]

    if ( index >= SND_MAX_VOICES
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    296,
                    0,
                    "%s",
                    "index >= 0 && index < SND_MAX_VOICES") )
    {
        __debugbreak();
    }
    voice = &g_snd.voice[index];
    for ( lengthNotifyIndex = 0; lengthNotifyIndex < voice->lengthNotifyInfo.count; ++lengthNotifyIndex )
    {
        id = voice->lengthNotifyInfo.id[lengthNotifyIndex];
        if ( (unsigned int)id >= SND_LENGTH_NOTIFY_COUNT
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                        304,
                        0,
                        "id doesn't index SND_LENGTH_NOTIFY_COUNT\n\t%i not in [0, %i)",
                        id,
                        3) )
        {
            __debugbreak();
        }
        DoLengthNotify(totalMsec, (const char *)voice->lengthNotifyInfo.data[lengthNotifyIndex], id);
    }
    voice->lengthNotifyInfo.count = 0;
}

void __cdecl SND_ResetVoiceInfo(int index)
{
    snd_voice_t *voice; // [esp+0h] [ebp-4h]

    voice = &g_snd.voice[index];
    if ( voice->playback )
        SND_FreePlaybackNotify(voice->playback);
    memset((unsigned __int8 *)voice, 0xFFu, sizeof(snd_voice_t));
    g_snd.voiceAliasHash[index] = 0;
    voice->soundFileInfo.loadingState = SFLS_UNLOADED;
    voice->alias = 0;
    voice->playback = 0;
}

unsigned int keys[148] =
{
  4294967295u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u
};

unsigned int ages[148];

double __cdecl SND_GetSeed(unsigned int key, unsigned int global_age)
{
    unsigned int n; // [esp+0h] [ebp-2Ch]
    unsigned int oldest_age; // [esp+4h] [ebp-28h]
    unsigned int oldest; // [esp+8h] [ebp-24h]
    unsigned int j; // [esp+Ch] [ebp-20h]
    unsigned int free_zone_len; // [esp+10h] [ebp-1Ch]
    unsigned int m; // [esp+14h] [ebp-18h]
    unsigned int max_free_zone_start; // [esp+18h] [ebp-14h]
    unsigned int max_free_zone_len; // [esp+1Ch] [ebp-10h]
    unsigned int k; // [esp+20h] [ebp-Ch]
    unsigned int i; // [esp+24h] [ebp-8h]
    int index; // [esp+28h] [ebp-4h]

    if ( key == -1 )
        key = 0;
    for ( i = 0; i < 0x94; ++i )
    {
        if ( global_age - ages[i] > 0x7530 && keys[i] != -1 || !ages[i] )
            keys[i] = -1;
    }
    index = -1;
    for ( k = 0; k < 0x94; ++k )
    {
        if ( keys[k] == key )
        {
            index = k;
            break;
        }
    }
    if ( index == -1 )
    {
        max_free_zone_start = 0;
        max_free_zone_len = 0;
        for ( m = 0; m < 0x94; m += free_zone_len + 1 )
        {
            free_zone_len = 0;
            for ( j = m; j < 0x94 && keys[j] == -1; ++j )
                ++free_zone_len;
            if ( free_zone_len > max_free_zone_len )
            {
                max_free_zone_len = free_zone_len;
                max_free_zone_start = m;
            }
        }
        if ( max_free_zone_len )
            index = max_free_zone_start + (max_free_zone_len >> 1);
    }
    if ( index == -1 )
    {
        oldest = 0;
        oldest_age = ages[0];
        for ( n = 1; n < 0x94; ++n )
        {
            if ( ages[n] > oldest_age )
            {
                oldest = n;
                oldest_age = ages[n];
            }
        }
        index = oldest;
    }
    if ( (unsigned int)index >= 0x94
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 408, 0, "%s", "index<ENTRY_COUNT") )
    {
        __debugbreak();
    }
    if ( index < 0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 409, 0, "%s", "index>=0") )
        __debugbreak();
    keys[index] = key;
    ages[index] = global_age;
    return (double)index / 147.0;
}

void __cdecl SND_SetVoiceStartInfo(unsigned int index, SndStartAliasInfo *SndStartAliasInfo)
{
    sndLengthNotifyInfo *p_lengthNotifyInfo; // ecx
    int ListenerIndexNearestToOrigin; // eax
    double DryLevel; // st7
    double WetLevel; // st7
    bool v6; // [esp+8h] [ebp-74h]
    snd_occlusion_start_cache *ocache; // [esp+38h] [ebp-44h]
    float *v8; // [esp+3Ch] [ebp-40h]
    float *position; // [esp+48h] [ebp-34h]
    float *fluxVelocity; // [esp+50h] [ebp-2Ch]
    float *direction; // [esp+60h] [ebp-1Ch]
    int cache; // [esp+6Ch] [ebp-10h] BYREF
    snd_voice_t *voice; // [esp+70h] [ebp-Ch]
    const snd_alias_t *alias; // [esp+74h] [ebp-8h]
    float occlusionGoal; // [esp+78h] [ebp-4h]

    alias = SndStartAliasInfo->alias;
    if ( index >= SND_MAX_VOICES
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    708,
                    0,
                    "%s",
                    "index >= 0 && index < SND_MAX_VOICES") )
    {
        __debugbreak();
    }
    if ( !SndStartAliasInfo->alias
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 709, 0, "%s", "SndStartAliasInfo->alias") )
    {
        __debugbreak();
    }
    voice = &g_snd.voice[index];
    if ( g_snd.voiceAliasHash[index]
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 713, 0, "%s", "SND_IsVoiceFree(index)") )
    {
        __debugbreak();
    }
    g_snd.voiceAliasHash[index] = SndStartAliasInfo->alias->id;
    p_lengthNotifyInfo = &voice->lengthNotifyInfo;
    voice->lengthNotifyInfo.id[0] = SND_LENGTH_NOTIFY_NONE;
    p_lengthNotifyInfo->id[1] = SND_LENGTH_NOTIFY_NONE;
    p_lengthNotifyInfo->id[2] = SND_LENGTH_NOTIFY_NONE;
    p_lengthNotifyInfo->id[3] = SND_LENGTH_NOTIFY_NONE;
    p_lengthNotifyInfo->data[0] = 0;
    p_lengthNotifyInfo->data[1] = 0;
    p_lengthNotifyInfo->data[2] = 0;
    p_lengthNotifyInfo->data[3] = 0;
    p_lengthNotifyInfo->count = 0;
    voice->sndEnt.handle = SndStartAliasInfo->sndEnt.handle;
    voice->group = (SndStartAliasInfo->alias->flags & 0x3F0000) >> 16;
    voice->volModStart = SndStartAliasInfo->volModStart;
    voice->alias = SndStartAliasInfo->alias;
    voice->startDelay = SndStartAliasInfo->startDelay;
    voice->looptime = g_snd.looptime;
    v6 = g_snd.paused && SND_IsAliasPausable(SndStartAliasInfo->alias);
    voice->paused = v6;
    voice->startTime = g_snd.time + voice->startDelay;
    voice->totalMsec = 0;
    voice->positionUpdated = 0;
    voice->playbackId = SndStartAliasInfo->playbackId;
    voice->firstPlaybackId = SndStartAliasInfo->playbackId;
    voice->playback = SndStartAliasInfo->playback;
    direction = voice->direction;
    voice->direction[0] = SndStartAliasInfo->dir[0];
    direction[1] = SndStartAliasInfo->dir[1];
    direction[2] = SndStartAliasInfo->dir[2];
    if ( ((LODWORD(voice->direction[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(voice->direction[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(voice->direction[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    732,
                    0,
                    "%s",
                    "!IS_NAN((voice->direction)[0]) && !IS_NAN((voice->direction)[1]) && !IS_NAN((voice->direction)[2])") )
    {
        __debugbreak();
    }
    fluxVelocity = voice->fluxVelocity;
    voice->fluxVelocity[0] = 0.0f;
    fluxVelocity[1] = 0.0f;
    fluxVelocity[2] = 0.0f;
    voice->losHitCache = 0;
    position = voice->position;
    voice->position[0] = SndStartAliasInfo->org[0];
    position[1] = SndStartAliasInfo->org[1];
    position[2] = SndStartAliasInfo->org[2];
    voice->voiceStartTime = Sys_Milliseconds();
    voice->pitchShift = 0;
    if ( fabs((float)((float)alias->pitchMin / 32767.0) - 1.0) > 0.0000152879
        || fabs((float)((float)alias->pitchMax / 32767.0) - 1.0) > 0.0000152879
        || (alias->flags & 0x10) >> 4 )
    {
        voice->pitchShift = 1;
    }
    voice->played = 0;
    voice->futzBlend = 0.0f;
    SND_SetVoiceStartSeeds(alias, voice);
    SND_SetVoiceStartFades(SndStartAliasInfo->fadetime, voice);
    SND_UpdateVoicePosition(voice, SndStartAliasInfo->org);
    Snd_SpeakerMapZero(&voice->pan);
    Snd_SpeakerMapZero(&voice->panGoal);
    SND_UpdateVoice(voice, 0.0);
    occlusionGoal = 0.0f;
    ++occlusionTotal;
    if ( snd_losOcclusion->current.enabled )
    {
        if ( (float)((float)voice->alias->occlusionLevel / 255.0) >= 0.0000152879 )
        {
            if ( (voice->alias->flags & 2) >> 1 && SndStartAliasInfo->ocache && SndStartAliasInfo->ocache->valid )
            {
                occlusionGoal = SndStartAliasInfo->ocache->value;
                ++occlusionOCacheHits;
            }
            else if ( voice->baseDistance > snd_occlusion_cull_distance->current.value && (voice->alias->flags & 8) >> 3 )
            {
                occlusionGoal = (float)((float)voice->alias->occlusionLevel / 255.0) * 0.5;
            }
            else if ( (voice->alias->flags & 2) >> 1 )
            {
                ++cache_total;
                if ( !CG_IsShowingZombieMap() )
                {
                    ++cache_misses;
                    cache = 0;
                    occlusionGoal = SND_LosOcclusionTrace(
                                                        0,
                                                        &cache,
                                                        g_snd.listeners[voice->closestListenerIndex].orient.origin,
                                                        voice->position);
                }
                if ( SndStartAliasInfo->ocache )
                {
                    SndStartAliasInfo->ocache->valid = 1;
                    SndStartAliasInfo->ocache->value = occlusionGoal;
                    ocache = SndStartAliasInfo->ocache;
                    v8 = voice->position;
                    ocache->position[0] = voice->position[0];
                    ocache->position[1] = v8[1];
                    ocache->position[2] = v8[2];
                }
            }
        }
        else
        {
            ++occlusionLevelHits;
        }
    }
    SND_FaderSetGoal(&voice->losOcclusion, occlusionGoal);
    voice->losOcclusion.value = voice->losOcclusion.goal;
    SND_FaderSetRate(&voice->losOcclusion, snd_occlusion_rate->current.value);
    if ( (LODWORD(voice->losOcclusion.goal) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    829,
                    0,
                    "%s",
                    "!IS_NAN(SND_FaderGetGoal(&voice->losOcclusion))") )
    {
        __debugbreak();
    }
    if ( (LODWORD(voice->losOcclusion.value) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    830,
                    0,
                    "%s",
                    "!IS_NAN(SND_FaderGetValue(&voice->losOcclusion))") )
    {
        __debugbreak();
    }
    if ( (voice->alias->flags & 2) >> 1 )
    {
        ListenerIndexNearestToOrigin = SND_GetListenerIndexNearestToOrigin(voice->position);
        SND_SetVoiceStartFlux(voice, g_snd.listeners[ListenerIndexNearestToOrigin].orient.origin);
    }
    DryLevel = SND_GetDryLevel(voice);
    voice->dryLevel = DryLevel;
    WetLevel = SND_GetWetLevel(voice);
    voice->wetLevel = WetLevel;
    voice->doppler.value = voice->doppler.goal;
}

void __cdecl SND_FaderSetRate(snd_fader_t *fader, float r)
{
    if ( r < 0.0 && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.h", 33, 0, "%s", "r >= 0.0f") )
        __debugbreak();
    if ( (LODWORD(r) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.h", 34, 0, "%s", "!IS_NAN(r)") )
    {
        __debugbreak();
    }
    fader->rate = r;
}

void __cdecl SND_FaderSetGoal(snd_fader_t *fader, float g)
{
    iassert(!IS_NAN(g));

    fader->goal = g;

    //if (COERCE_FLOAT(COERCE_UNSIGNED_INT(fader->rate - 0.0) & _mask__AbsFloat_ ^ _mask__NegFloat_) < 0.0)
    //if (-(fabs(fader->rate)) < 0.0) // this seems retarded, fabs doesnt return < 0?

    if (fader->rate == 0.0f)
        fader->value = g;
}

bool __cdecl SND_IsAliasPausable(const snd_alias_t *alias)
{
    if ( !alias && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_alias_db.h", 470, 0, "%s", "alias") )
        __debugbreak();
    return (alias->flags & 0x80) >> 7 != 0;
}

void __cdecl SND_SetVoiceStartFlux(snd_voice_t *voice, float *player)
{
    snd_flux_type_t move_type; // [esp+60h] [ebp-34h]
    int separation; // [esp+64h] [ebp-30h]
    float moveBasis[2][3]; // [esp+68h] [ebp-2Ch] BYREF
    const snd_alias_t *alias; // [esp+80h] [ebp-14h]
    float speed; // [esp+84h] [ebp-10h]
    float Z[3]; // [esp+88h] [ebp-Ch] BYREF

    Z[0] = 0.0f;
    Z[1] = 0.0f;
    Z[2] = 1.0f;
    alias = voice->alias;
    if ( !player && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 427, 0, "%s", "player") )
        __debugbreak();
    if ( !voice && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 428, 0, "%s", "voice") )
        __debugbreak();
    move_type = (snd_flux_type_t)((voice->alias->flags & 0x1C00000) >> 22);
    if ( move_type )
    {
        if ( move_type == SND_FLUX_TYPE_RANDOM_PLAYER )
            move_type = (snd_flux_type_t)(rand() % 3 + 1);
        if ( !((alias->flags & 2) >> 1) )
        {
            Com_PrintError(9, "Alias %s has move_type but is not 3d\n", alias->name);
            return;
        }
        if ( (float)((float)((float)(voice->direction[0] * voice->direction[0])
                                             + (float)(voice->direction[1] * voice->direction[1]))
                             + (float)(voice->direction[2] * voice->direction[2])) <= 0.0000152879
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                        448,
                        0,
                        "%s",
                        "Vec3LengthSq(voice->direction) > SND_EPSILON") )
        {
            __debugbreak();
        }
        if ( move_type == SND_FLUX_TYPE_LEFT_OF_SHOT
            || move_type == SND_FLUX_TYPE_CENTER_OF_SHOT
            || move_type == SND_FLUX_TYPE_RIGHT_OF_SHOT )
        {
            if ( ((LODWORD(voice->direction[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(voice->direction[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(voice->direction[2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                            454,
                            0,
                            "%s",
                            "!IS_NAN((voice->direction)[0]) && !IS_NAN((voice->direction)[1]) && !IS_NAN((voice->direction)[2])") )
            {
                __debugbreak();
            }
            if ( (float)((float)((float)(voice->direction[0] * voice->direction[0])
                                                 + (float)(voice->direction[1] * voice->direction[1]))
                                 + (float)(voice->direction[2] * voice->direction[2])) < 0.0000152879 )
            {
                Com_PrintError(9, "Alias %s has move_type of shot but has no direction\n", alias->name);
                return;
            }
            *(_QWORD *)&moveBasis[0][0] = *(_QWORD *)voice->direction;
            moveBasis[0][2] = voice->direction[2];
        }
        else
        {
            moveBasis[0][0] = *player - voice->position[0];
            moveBasis[0][1] = player[1] - voice->position[1];
            moveBasis[0][2] = player[2] - voice->position[2];
        }
        Vec3Normalize(moveBasis[0]);
        if ( move_type == SND_FLUX_TYPE_LEFT_OF_SHOT || move_type == SND_FLUX_TYPE_LEFT_OF_PLAYER )
            Z[2] = -1.0f;
        separation = snd_flux_separation->current.integer;
        if ( move_type == SND_FLUX_TYPE_CENTER_OF_SHOT || move_type == SND_FLUX_TYPE_CENTER_OF_PLAYER )
            separation = 0;
        Vec3Cross(moveBasis[0], Z, moveBasis[1]);
        Vec3Lerp(moveBasis[0], moveBasis[1], *(float *)&separation, voice->fluxVelocity);
        Vec3Normalize(voice->fluxVelocity);
        if ( (float)((float)alias->fluxTime / 1000.0) <= 0.0000152879 )
            speed = 0.0f;
        else
            speed = (float)alias->distReverbMax / (float)((float)alias->fluxTime / 1000.0);
        voice->fluxVelocity[0] = speed * voice->fluxVelocity[0];
        voice->fluxVelocity[1] = speed * voice->fluxVelocity[1];
        voice->fluxVelocity[2] = speed * voice->fluxVelocity[2];
        if ( ((LODWORD(voice->fluxVelocity[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(voice->fluxVelocity[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(voice->fluxVelocity[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                        497,
                        0,
                        "%s",
                        "!IS_NAN((voice->fluxVelocity)[0]) && !IS_NAN((voice->fluxVelocity)[1]) && !IS_NAN((voice->fluxVelocity)[2])") )
        {
            __debugbreak();
        }
    }
}

void __cdecl SND_SetVoiceStartSeeds(const snd_alias_t *alias, snd_voice_t *voice)
{
    bool randomSeed; // [esp+17h] [ebp-9h]
    unsigned int baseHash; // [esp+1Ch] [ebp-4h]

    baseHash = ((unsigned __int16)alias->id << 16) + (voice->sndEnt.handle & 0xFFF);
    randomSeed = snd_entityRandomize->current.enabled;
    if ( (((alias->flags & 0xE0000000) >> 29) & 1) != 0 && randomSeed )
        voice->volModSeed = SND_GetSeed(baseHash | 0x80000000, g_snd.time);
    else
        voice->volModSeed = Com_Random();
    if ( (((alias->flags & 0xE0000000) >> 29) & 2) != 0 && randomSeed )
        voice->pitchModSeed = SND_GetSeed(baseHash & 0x7FFFFFFF, g_snd.time);
    else
        voice->pitchModSeed = Com_Random();
    if ( voice->volModSeed < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 527, 0, "%s", "voice->volModSeed >= 0.0f") )
    {
        __debugbreak();
    }
    if ( voice->volModSeed > 1.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 528, 0, "%s", "voice->volModSeed <= 1.0f") )
    {
        __debugbreak();
    }
    if ( (LODWORD(voice->volModSeed) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    529,
                    0,
                    "%s",
                    "!IS_NAN(voice->volModSeed)") )
    {
        __debugbreak();
    }
    if ( voice->pitchModSeed < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    530,
                    0,
                    "%s",
                    "voice->pitchModSeed >= 0.0f") )
    {
        __debugbreak();
    }
    if ( voice->pitchModSeed > 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    531,
                    0,
                    "%s",
                    "voice->pitchModSeed <= 1.0f") )
    {
        __debugbreak();
    }
    if ( (LODWORD(voice->pitchModSeed) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    532,
                    0,
                    "%s",
                    "!IS_NAN(voice->pitchModSeed)") )
    {
        __debugbreak();
    }
}

void __cdecl SND_SetVoiceStartFades(float fadetime, snd_voice_t *voice)
{
    if ( fadetime <= 0.0000152879 )
    {
        SND_FaderSetValue(&voice->fade, 1.0);
        SND_FaderSetRate(&voice->fade, 5.0);
        SND_FaderSetGoal(&voice->fade, 1.0);
    }
    else
    {
        SND_FaderSetValue(&voice->fade, 0.0);
        SND_FaderSetRateTime(&voice->fade, fadetime / 1000.0);
        SND_FaderSetGoal(&voice->fade, 1.0);
        SND_FaderUpdate(&voice->fade, 0.0);
    }
    SND_FaderSetValue(&voice->script_fade, 1.0);
    SND_FaderSetGoal(&voice->script_fade, 1.0);
    SND_FaderSetRateTime(&voice->script_fade, 0.1);
    SND_FaderSetValue(&voice->script_pitch, 1.0);
    SND_FaderSetGoal(&voice->script_pitch, 1.0);
    SND_FaderSetRateTime(&voice->script_pitch, 0.25);
    SND_FaderSetValue(&voice->doppler, 0.0);
    SND_FaderSetGoal(&voice->doppler, 0.0);
    SND_FaderSetRate(&voice->doppler, snd_dopplerRate->current.value);
    SND_FaderSetRate(&voice->losOcclusion, snd_occlusion_rate->current.value);
    SND_FaderSetGoal(&voice->losOcclusion, 0.0);
    voice->losOcclusion.value = voice->losOcclusion.goal;
}

void __cdecl SND_FaderSetRateTime(snd_fader_t *fader, float time)
{
    float v2; // [esp+0h] [ebp-4h]

    if ( time < 0.0
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.h", 41, 0, "%s", "time >= 0.0f") )
    {
        __debugbreak();
    }
    if ( (float)(time - 0.0000099999997) < 0.0 )
        v2 = 0.0f;
    else
        v2 = 1.0 / time;
    fader->rate = v2;
}

void __cdecl SND_FaderSetValue(snd_fader_t *fader, float v)
{
    if ( (LODWORD(v) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.h", 55, 0, "%s", "!IS_NAN(v)") )
    {
        __debugbreak();
    }
    fader->goal = v;
    fader->value = v;
}

void __cdecl SND_FaderUpdate(snd_fader_t *fader, float dt)
{
    float v2; // [esp+0h] [ebp-2Ch]
    float v3; // [esp+4h] [ebp-28h]
    float v4; // [esp+8h] [ebp-24h]
    float rate; // [esp+1Ch] [ebp-10h]
    float value; // [esp+24h] [ebp-8h]
    float goal; // [esp+28h] [ebp-4h]

    if ( (LODWORD(dt) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.h", 88, 0, "%s", "!IS_NAN(dt)") )
    {
        __debugbreak();
    }
    rate = fader->rate;
    value = fader->value;
    goal = fader->goal;
    if ( (float)(goal - fader->value) < 0.0 )
        v4 = -1.0f;
    else
        v4 = 1.0f;
    if ( (float)(fabs(goal - value) - (float)(rate * dt)) < 0.0 )
        v3 = fabs(goal - value);
    else
        v3 = rate * dt;
    if ( (float)(0.0 - rate) < 0.0 )
        v2 = (float)(v4 * v3) + value;
    else
        v2 = fader->goal;
    if ( (LODWORD(v2) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.h", 96, 0, "%s", "!IS_NAN(value)") )
    {
        __debugbreak();
    }
    fader->value = v2;
}

void __cdecl SND_UpdateVoicePosition(snd_voice_t *voice, const float *startPosition)
{
    float *v2; // [esp+0h] [ebp-118h]
    float v3; // [esp+4Ch] [ebp-CCh]
    float v4; // [esp+5Ch] [ebp-BCh]
    float v5; // [esp+6Ch] [ebp-ACh]
    float origin[3]; // [esp+F4h] [ebp-24h] BYREF
    //float v7; // [esp+F8h] [ebp-20h]
    //float v8; // [esp+FCh] [ebp-1Ch]
    float offset[3]; // [esp+100h] [ebp-18h]
    float entityOrigin[3]; // [esp+10Ch] [ebp-Ch] BYREF

    if ( startPosition )
    {
        nanassertvec3(startPosition);
        
        if ( !((voice->alias->flags & 2) >> 1) )
        {
            voice->entity_update = SND_ENTITY_UPDATE_NEVER;
            voice->positionUpdated = 1;
            voice->position[0] = 0.0f;
            voice->position[1] = 0.0f;
            voice->position[2] = 0.0f;
            voice->offset[0] = 0.0f;
            voice->offset[1] = 0.0f;
            voice->offset[2] = 0.0f;
            voice->velocity[0] = 0.0f;
            voice->velocity[1] = 0.0f;
            voice->velocity[2] = 0.0f;
            AxisClear(voice->orientation);
            return;
        }
        if ((voice->sndEnt.handle & 0xFFF) == 0xFFF
            || (((unsigned int)voice->sndEnt.handle >> 21) & 1) != 0
            || (voice->alias->flags & 0x1C00000) >> 22)
        {
            voice->entity_update = SND_ENTITY_UPDATE_NEVER;
            voice->positionUpdated = 1;
            voice->position[0] = *startPosition;
            voice->position[1] = startPosition[1];
            voice->position[2] = startPosition[2];
            voice->offset[0] = 0.0f;
            voice->offset[1] = 0.0f;
            voice->offset[2] = 0.0f;
            voice->velocity[0] = 0.0f;
            voice->velocity[1] = 0.0f;
            voice->velocity[2] = 0.0f;
            AxisClear(voice->orientation);
            return;
        }
        voice->entity_update = SND_ENTITY_UPDATE_ALWAYS;
        voice->position[0] = *startPosition;
        voice->position[1] = startPosition[1];
        voice->position[2] = startPosition[2];
        voice->offset[0] = 0.0f;
        voice->offset[1] = 0.0f;
        voice->offset[2] = 0.0f;
        AxisClear(voice->orientation);
        voice->velocity[0] = 0.0f;
        voice->velocity[1] = 0.0f;
        voice->velocity[2] = 0.0f;
    }
    if ( voice->entity_update != SND_ENTITY_UPDATE_NEVER )
    {
        if ( !((voice->alias->flags & 2) >> 1)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                        614,
                        0,
                        "%s",
                        "SND_IsAliasSpatial( voice->alias )") )
        {
            __debugbreak();
        }
        if ( (voice->sndEnt.handle & 0xFFF) == 0xFFF
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                        615,
                        0,
                        "%s",
                        "voice->sndEnt.field.entIndex != SND_ENT_NONE") )
        {
            __debugbreak();
        }
        if ( (((unsigned int)voice->sndEnt.handle >> 21) & 1) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                        616,
                        0,
                        "%s",
                        "!voice->sndEnt.field.isStationary") )
        {
            __debugbreak();
        }
        if ( voice->entity_update
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                        617,
                        0,
                        "%s",
                        "voice->entity_update == SND_ENTITY_UPDATE_ALWAYS") )
        {
            __debugbreak();
        }
        if ( voice->positionUpdated )
        {
            if ( !SND_GetEntState(voice->sndEnt, origin, voice->velocity, voice->orientation) )
                return;
            if ( ((LODWORD(origin[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(origin[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(origin[2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                            650,
                            0,
                            "%s",
                            "!IS_NAN((entityOrigin)[0]) && !IS_NAN((entityOrigin)[1]) && !IS_NAN((entityOrigin)[2])") )
            {
                __debugbreak();
            }
            if ( ((LODWORD(voice->orientation[0][0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(voice->orientation[0][1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(voice->orientation[0][2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                            651,
                            0,
                            "%s",
                            "!IS_NAN((voice->orientation[0])[0]) && !IS_NAN((voice->orientation[0])[1]) && !IS_NAN((voice->orientation[0])[2])") )
            {
                __debugbreak();
            }
            if ( ((LODWORD(voice->orientation[1][0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(voice->orientation[1][1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(voice->orientation[1][2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                            652,
                            0,
                            "%s",
                            "!IS_NAN((voice->orientation[1])[0]) && !IS_NAN((voice->orientation[1])[1]) && !IS_NAN((voice->orientation[1])[2])") )
            {
                __debugbreak();
            }
            if ( ((LODWORD(voice->orientation[2][0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(voice->orientation[2][1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(voice->orientation[2][2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                            653,
                            0,
                            "%s",
                            "!IS_NAN((voice->orientation[2])[0]) && !IS_NAN((voice->orientation[2])[1]) && !IS_NAN((voice->orientation[2])[2])") )
            {
                __debugbreak();
            }
            if ( ((LODWORD(voice->velocity[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(voice->velocity[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(voice->velocity[2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                            654,
                            0,
                            "%s",
                            "!IS_NAN((voice->velocity)[0]) && !IS_NAN((voice->velocity)[1]) && !IS_NAN((voice->velocity)[2])") )
            {
                __debugbreak();
            }
            voice->position[0] = origin[0];
            voice->position[1] = origin[1];
            voice->position[2] = origin[2];
            v5 = voice->offset[0];
            voice->position[0] = (float)(v5 * voice->orientation[0][0]) + voice->position[0];
            voice->position[1] = (float)(v5 * voice->orientation[0][1]) + voice->position[1];
            voice->position[2] = (float)(v5 * voice->orientation[0][2]) + voice->position[2];
            v4 = voice->offset[1];
            voice->position[0] = (float)(v4 * voice->orientation[1][0]) + voice->position[0];
            voice->position[1] = (float)(v4 * voice->orientation[1][1]) + voice->position[1];
            voice->position[2] = (float)(v4 * voice->orientation[1][2]) + voice->position[2];
            v3 = voice->offset[2];
            voice->position[0] = (float)(v3 * voice->orientation[2][0]) + voice->position[0];
            voice->position[1] = (float)(v3 * voice->orientation[2][1]) + voice->position[1];
            voice->position[2] = (float)(v3 * voice->orientation[2][2]) + voice->position[2];
        }
        else
        {
            if ( !SND_GetEntState(voice->sndEnt, entityOrigin, voice->velocity, voice->orientation) )
                return;
            offset[0] = voice->position[0] - entityOrigin[0];
            offset[1] = voice->position[1] - entityOrigin[1];
            offset[2] = voice->position[2] - entityOrigin[2];
            voice->offset[0] = (float)((float)(offset[0] * voice->orientation[0][0])
                                                             + (float)(offset[1] * voice->orientation[0][1]))
                                             + (float)(offset[2] * voice->orientation[0][2]);
            voice->offset[1] = (float)((float)(offset[0] * voice->orientation[1][0])
                                                             + (float)(offset[1] * voice->orientation[1][1]))
                                             + (float)(offset[2] * voice->orientation[1][2]);
            voice->offset[2] = (float)((float)(offset[0] * voice->orientation[2][0])
                                                             + (float)(offset[1] * voice->orientation[2][1]))
                                             + (float)(offset[2] * voice->orientation[2][2]);
            voice->positionUpdated = 1;
        }
        if ( ((LODWORD(voice->position[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(voice->position[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(voice->position[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                        663,
                        0,
                        "%s",
                        "!IS_NAN((voice->position)[0]) && !IS_NAN((voice->position)[1]) && !IS_NAN((voice->position)[2])") )
        {
            __debugbreak();
        }
        if ( ((LODWORD(voice->orientation[0][0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(voice->orientation[0][1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(voice->orientation[0][2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                        664,
                        0,
                        "%s",
                        "!IS_NAN((voice->orientation[0])[0]) && !IS_NAN((voice->orientation[0])[1]) && !IS_NAN((voice->orientation[0])[2])") )
        {
            __debugbreak();
        }
        if ( ((LODWORD(voice->orientation[1][0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(voice->orientation[1][1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(voice->orientation[1][2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                        665,
                        0,
                        "%s",
                        "!IS_NAN((voice->orientation[1])[0]) && !IS_NAN((voice->orientation[1])[1]) && !IS_NAN((voice->orientation[1])[2])") )
        {
            __debugbreak();
        }
        if ( ((LODWORD(voice->orientation[2][0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(voice->orientation[2][1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(voice->orientation[2][2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                        666,
                        0,
                        "%s",
                        "!IS_NAN((voice->orientation[2])[0]) && !IS_NAN((voice->orientation[2])[1]) && !IS_NAN((voice->orientation[2])[2])") )
        {
            __debugbreak();
        }
        if ( ((LODWORD(voice->velocity[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(voice->velocity[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(voice->velocity[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                        667,
                        0,
                        "%s",
                        "!IS_NAN((voice->velocity)[0]) && !IS_NAN((voice->velocity)[1]) && !IS_NAN((voice->velocity)[2])") )
        {
            __debugbreak();
        }
        v2 = g_snd.voicePositionCache[voice - g_snd.voice];
        *v2 = voice->position[0];
        v2[1] = voice->position[1];
        v2[2] = voice->position[2];
    }
}

void __cdecl SND_SetSoundFileVoiceInfo(
                unsigned int voiceIndex,
                int srcChannelCount,
                int baserate,
                unsigned int total_msec,
                int start_msec,
                SndFileLoadingState loadingState)
{
    snd_voice_t *voice; // [esp+0h] [ebp-4h]

    if ( voiceIndex >= SND_MAX_VOICES
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    852,
                    0,
                    "%s",
                    "voiceIndex >= 0 && voiceIndex < SND_MAX_VOICES") )
    {
        __debugbreak();
    }
    voice = &g_snd.voice[voiceIndex];
    if ( voice->soundFileInfo.loadingState == SFLS_LOADING && loadingState == SFLS_LOADED )
    {
        Sys_Milliseconds();
        //BLOPS_NULLSUB();
    }
    voice->soundFileInfo.loadingState = loadingState;
    voice->soundFileInfo.srcChannelCount = srcChannelCount;
    voice->soundFileInfo.baserate = baserate;
    voice->soundFileInfo.endtime = total_msec + g_snd.time - start_msec;
    voice->soundFileInfo.totalMsec = total_msec;
    SND_StartLengthNotify(voiceIndex, total_msec);
}

unsigned int __cdecl SND_FindFreeVoice(SndStartAliasInfo *startAliasInfo)
{
    unsigned int i; // [esp+4h] [ebp-24h]
    float thatPriority; // [esp+8h] [ebp-20h] BYREF
    int thatVoice; // [esp+Ch] [ebp-1Ch] BYREF
    unsigned int start; // [esp+10h] [ebp-18h]
    float thisVolume; // [esp+14h] [ebp-14h]
    const snd_alias_t *alias; // [esp+18h] [ebp-10h]
    int replacedVoice; // [esp+1Ch] [ebp-Ch]
    unsigned int count; // [esp+20h] [ebp-8h]
    float thisPriority; // [esp+24h] [ebp-4h]

    alias = startAliasInfo->alias;
    thisVolume = Snd_GetGlobalPriorityVolume(startAliasInfo->alias, startAliasInfo->org);
    thisPriority = Snd_GetGlobalPriority(startAliasInfo->alias, thisVolume);
    if ( (alias->flags & 0xC000) >> 14 == 1 )
    {
        start = 10;
        count = 64;
    }
    else
    {
        start = 0;
        count = 10;
    }
    for ( i = start; i < count + start; ++i )
    {
        if ( !g_snd.voiceAliasHash[i] )
            return i;
    }
    replacedVoice = -1;
    thatPriority = 0.0f;
    thatVoice = -1;
    Snd_GetLowestPriority(&thatPriority, &thatVoice, start, count);
    if ( thisPriority > (float)(thatPriority + snd_playing_priority_boost->current.value) )
        replacedVoice = thatVoice;
    if ( replacedVoice >= 0 )
    {
        SND_StopVoice(replacedVoice);
        if ( g_snd.voiceAliasHash[replacedVoice] )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                            1029,
                            0,
                            "%s",
                            "SND_IsVoiceFree(replacedVoice)") )
                __debugbreak();
        }
    }
    return replacedVoice;
}

double __cdecl Snd_GetGlobalPriority(const snd_alias_t *alias, float volume)
{
    float t; // [esp+50h] [ebp-8h]
    float p; // [esp+54h] [ebp-4h]

    if ( !alias && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 874, 0, "%s", "alias") )
        __debugbreak();
    iassert(!IS_NAN(alias->minPriority));
    iassert(!IS_NAN(alias->maxPriority));

    // (KISAKTODO)
   //if ( (COERCE_UNSIGNED_INT((float)alias->minPriorityThreshold / 255.0) & 0x7F800000) == 0x7F800000
   //    && !Assert_MyHandler(
   //                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
   //                877,
   //                0,
   //                "%s",
   //                "!IS_NAN(SND_AliasGetMinPriorityThreshold(alias))") )
   //{
   //    __debugbreak();
   //}
   //if ( (COERCE_UNSIGNED_INT((float)alias->maxPriorityThreshold / 255.0) & 0x7F800000) == 0x7F800000
   //    && !Assert_MyHandler(
   //                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
   //                878,
   //                0,
   //                "%s",
   //                "!IS_NAN(SND_AliasGetMaxPriorityThreshold(alias))") )
   //{
   //    __debugbreak();
   //}
   //if ( (float)((float)alias->minPriorityThreshold / 255.0) < 0.0
   //    && !Assert_MyHandler(
   //                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
   //                883,
   //                0,
   //                "%s\n\t(SND_AliasGetMinPriorityThreshold(alias)) = %g",
   //                "(SND_AliasGetMinPriorityThreshold(alias) >= 0.0f)",
   //                (float)((float)alias->minPriorityThreshold / 255.0)) )
   //{
   //    __debugbreak();
   //}
   //if ( (float)((float)alias->minPriorityThreshold / 255.0) > 1.0
   //    && !Assert_MyHandler(
   //                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
   //                884,
   //                0,
   //                "%s\n\t(SND_AliasGetMinPriorityThreshold(alias)) = %g",
   //                "(SND_AliasGetMinPriorityThreshold(alias) <= 1.0f)",
   //                (float)((float)alias->minPriorityThreshold / 255.0)) )
   //{
   //    __debugbreak();
   //}
   //if ( (float)((float)alias->maxPriorityThreshold / 255.0) < 0.0
   //    && !Assert_MyHandler(
   //                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
   //                885,
   //                0,
   //                "%s\n\t(SND_AliasGetMaxPriorityThreshold(alias)) = %g",
   //                "(SND_AliasGetMaxPriorityThreshold(alias) >= 0.0f)",
   //                (float)((float)alias->maxPriorityThreshold / 255.0)) )
   //{
   //    __debugbreak();
   //}
   //if ( (float)((float)alias->maxPriorityThreshold / 255.0) > 1.0
   //    && !Assert_MyHandler(
   //                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
   //                886,
   //                0,
   //                "%s\n\t(SND_AliasGetMaxPriorityThreshold(alias)) = %g",
   //                "(SND_AliasGetMaxPriorityThreshold(alias) <= 1.0f)",
   //                (float)((float)alias->maxPriorityThreshold / 255.0)) )
   //{
   //    __debugbreak();
   //}
    if ( volume < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    887,
                    0,
                    "%s\n\t(volume) = %g",
                    "(volume >= 0.0f)",
                    volume) )
    {
        __debugbreak();
    }
    if ( volume > 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    888,
                    0,
                    "%s\n\t(volume) = %g",
                    "(volume <= 1.0f)",
                    volume) )
    {
        __debugbreak();
    }
    if ( (float)((float)alias->minPriorityThreshold / 255.0) >= volume )
        return (double)alias->minPriority;
    if ( volume >= (float)((float)alias->maxPriorityThreshold / 255.0) )
        return (double)alias->maxPriority;
    if ( (float)((float)alias->maxPriorityThreshold / 255.0) < (float)((float)alias->minPriorityThreshold / 255.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    900,
                    0,
                    "%s",
                    "SND_AliasGetMaxPriorityThreshold(alias) >= SND_AliasGetMinPriorityThreshold(alias)") )
    {
        __debugbreak();
    }
    t = 0.5f;
    if ( (float)((float)alias->maxPriorityThreshold / 255.0) != (float)((float)alias->minPriorityThreshold / 255.0) )
        t = (float)(volume - (float)((float)alias->minPriorityThreshold / 255.0))
            / (float)((float)((float)alias->maxPriorityThreshold / 255.0) - (float)((float)alias->minPriorityThreshold / 255.0));
    if ( t < -0.00001
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    908,
                    0,
                    "%s\n\t(t) = %g",
                    "(t >= -1e-5)",
                    t) )
    {
        __debugbreak();
    }
    if ( t > 1.00001
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    909,
                    0,
                    "%s\n\t(t) = %g",
                    "(t <= 1.0f+1e-5)",
                    t) )
    {
        __debugbreak();
    }
    p = (float)(t * (float)alias->maxPriority) + (float)((float)(1.0 - t) * (float)alias->minPriority);
    if ( p < -0.00001
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    913,
                    0,
                    "%s\n\t(p) = %g",
                    "(p >= -1e-5)",
                    p) )
    {
        __debugbreak();
    }
    if ( p > 200.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 914, 0, "%s\n\t(p) = %g", "(p <= 200)", p) )
    {
        __debugbreak();
    }
    if ( (float)alias->maxPriority <= (float)alias->minPriority )
    {
        if ( (float)alias->minPriority < p - 0.00001
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                        923,
                        0,
                        "%s\n\t(p) = %g",
                        "(p-1e-5 <= SND_AliasGetMinPriority(alias))",
                        p) )
        {
            __debugbreak();
        }
        if ( p + 0.00001 < (float)alias->maxPriority
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                        924,
                        0,
                        "%s\n\t(p) = %g",
                        "(p+1e-5 >= SND_AliasGetMaxPriority(alias))",
                        p) )
        {
            __debugbreak();
        }
    }
    else
    {
        if ( p + 0.00001 < (float)alias->minPriority
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                        918,
                        0,
                        "%s\n\t(p) = %g",
                        "(p+1e-5 >= SND_AliasGetMinPriority(alias))",
                        p) )
        {
            __debugbreak();
        }
        if ( (float)alias->maxPriority < p - 0.00001
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                        919,
                        0,
                        "%s\n\t(p) = %g",
                        "(p-1e-5 <= SND_AliasGetMaxPriority(alias))",
                        p) )
        {
            __debugbreak();
        }
    }
    return p;
}

double __cdecl Snd_GetGlobalPriorityVolume(const snd_alias_t *alias, const float *org)
{
    int ListenerIndexNearestToOrigin; // eax
    float normDistance; // [esp+10h] [ebp-8h]
    float distance; // [esp+14h] [ebp-4h]

    if ( !((alias->flags & 2) >> 1) )
        return 0.0;
    ListenerIndexNearestToOrigin = SND_GetListenerIndexNearestToOrigin(org);
    distance = Vec3Distance(org, g_snd.listeners[ListenerIndexNearestToOrigin].orient.origin);
    normDistance = distance / (float)alias->distReverbMax;
    if ( normDistance > 1.0 )
        normDistance = 1.0f;
    if ( normDistance < 0.0 )
        normDistance = 0.0f;
    return 1.0 - normDistance;
}

void __cdecl Snd_GetLowestPriority(float *priority, int *channel, unsigned int start, unsigned int count)
{
    int i; // [esp+8h] [ebp-Ch]
    float p; // [esp+Ch] [ebp-8h]
    unsigned int c; // [esp+10h] [ebp-4h]

    *priority = 1.0e10;
    *channel = -1;
    for ( c = 0; c < count; ++c )
    {
        i = start + c;
        if ( !g_snd.voiceAliasHash[start + c]
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 961, 0, "%s", "!SND_IsVoiceFree(i)") )
        {
            __debugbreak();
        }
        p = g_snd.voice[i].globalPriority;
        if ( p < -0.00001
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                        965,
                        0,
                        "%s\n\t(p) = %g",
                        "(p >= -1e-5)",
                        p) )
        {
            __debugbreak();
        }
        if ( p > 200.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                        966,
                        0,
                        "%s\n\t(p) = %g",
                        "(p <= 200)",
                        p) )
        {
            __debugbreak();
        }
        if ( *priority > p )
        {
            *channel = i;
            *priority = p;
        }
    }
    if ( *channel < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 974, 0, "%s", "*channel >= 0") )
    {
        __debugbreak();
    }
}

unsigned int __cdecl SND_ContinueLoopingSound(
                unsigned int aliasId,
                float volumeScale,
                SndEntHandle sndEnt,
                float *org,
                int fadeTime,
                snd_playback *playback)
{
    signed int i; // [esp+40h] [ebp-4h]

    PROF_SCOPED("SND_ContinueLoopingSound");

    for (i = 0; i < 74; ++i)
    {
        if (g_snd.voiceAliasHash[i]
            && g_snd.voice[i].sndEnt.handle == sndEnt.handle
            && (g_snd.voice[i].alias->flags & 1) != 0
            && g_snd.voice[i].alias->id == aliasId)
        {
            SND_ContinueLoopingSound_Internal(i, volumeScale, fadeTime, org, playback);
            if (org)
                SND_SetPosition(i, org);
            return g_snd.voice[i].playbackId;
        }
    }

    return -1;
}

void __cdecl SND_ContinueLoopingSound_Internal(
                unsigned int voiceIndex,
                float volumeScale,
                int fadeTime,
                float *org,
                snd_playback *playback)
{
    snd_voice_t *voice; // [esp+2Ch] [ebp-4h]

    if ( voiceIndex >= SND_MAX_VOICES
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    1072,
                    0,
                    "voiceIndex doesn't index SND_MAX_VOICES\n\t%i not in [0, %i)",
                    voiceIndex,
                    74) )
    {
        __debugbreak();
    }
    voice = &g_snd.voice[voiceIndex];
    if ( fadeTime < 0 && voice->fade.goal != 0.0 )
    {
        //SND_FaderSetRateTime(&voice->fade, COERCE_FLOAT(COERCE_UNSIGNED_INT((float)fadeTime) ^ _mask__NegFloat_) / 1000.0);
        SND_FaderSetRateTime(&voice->fade, -fadeTime / 1000.0);
        SND_FaderSetGoal(&voice->fade, 0.0);
    }
    if ( fadeTime > 0 && voice->fade.goal != 1.0 )
    {
        SND_FaderSetRateTime(&voice->fade, (float)fadeTime / 1000.0);
        SND_FaderSetGoal(&voice->fade, 1.0);
    }
    voice->volModStart = volumeScale;
    voice->looptime = g_snd.looptime;
    if ( playback )
    {
        if ( voice->playback )
            SND_FreePlayback(voice->playback);
        voice->playback = playback;
        voice->playbackId = playback->id;
    }
    if ( org )
        SND_SetPosition(voiceIndex, org);
}

unsigned int __cdecl SND_GetCurrentContext(unsigned int type)
{
    signed int i; // [esp+0h] [ebp-4h]

    i = SND_FindContextIndex(type);
    if ( i < 0 )
        return 0;
    else
        return g_snd.currentContexts[i];
}

snd_alias_t *__cdecl SND_PickSoundAliasFromList(snd_alias_list_t *aliasList, int objectId)
{
    unsigned int v3; // eax
    unsigned int v4; // eax
    unsigned int count; // [esp+0h] [ebp-124h]
    unsigned int j; // [esp+4h] [ebp-120h]
    int seed; // [esp+8h] [ebp-11Ch] BYREF
    snd_alias_t *alias; // [esp+Ch] [ebp-118h]
    unsigned int i; // [esp+10h] [ebp-114h]
    snd_alias_t *selectionArray[64]; // [esp+14h] [ebp-110h]
    unsigned int actualCount; // [esp+118h] [ebp-Ch]
    unsigned int totalCount; // [esp+11Ch] [ebp-8h]
    int randomIndex; // [esp+120h] [ebp-4h]

    if ( !aliasList )
        return 0;
    if ( !aliasList->count )
        return 0;
    if ( aliasList->count >= 0x40u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    1180,
                    0,
                    "%s",
                    "aliasList->count < MAX_VARIANTS") )
    {
        __debugbreak();
    }
    if ( aliasList->count >= 0x40u )
        count = 64;
    else
        count = aliasList->count;
    totalCount = count;
    actualCount = 0;
    for ( i = 0; i < totalCount; ++i )
    {
        alias = &aliasList->head[i];
        if ( alias->contextType )
        {
            if ( !alias->contextValue || alias->contextValue == SND_GetCurrentContext(alias->contextType) )
                selectionArray[actualCount++] = alias;
        }
        else
        {
            selectionArray[actualCount++] = alias;
        }
    }
    if ( !actualCount )
        return 0;
    if ( actualCount == 1 )
    {
        if ( !selectionArray[0]
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 1210, 0, "%s", "selectionArray[0]") )
        {
            __debugbreak();
        }
        return selectionArray[0];
    }
    else
    {
        randomIndex = -1;
        if ( (((selectionArray[0]->flags & 0xE0000000) >> 29) & 4) != 0 )
        {
            seed = (int)aliasList + objectId;
            v3 = RandWithSeed(&seed);
            randomIndex = v3 % actualCount;
        }
        else if ( g_snd.gameState.seed )
        {
            v4 = RandWithSeed((int *)&g_snd.gameState.seed);
            randomIndex = v4 % actualCount;
        }
        else
        {
            for ( j = 0; j < 0x64 && actualCount > 2 && randomIndex == aliasList->sequence || randomIndex == -1; ++j )
                randomIndex = rand() % actualCount;
        }
        aliasList->sequence = randomIndex;
        return selectionArray[randomIndex];
    }
}

snd_alias_t *__cdecl SND_PickSoundAlias(const char *name, int objectid)
{
    snd_alias_list_t *Alias; // eax

    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 1245, 0, "%s", "name") )
        __debugbreak();
    Alias = SND_FindAlias(name);
    return SND_PickSoundAliasFromList(Alias, objectid);
}

void __cdecl SND_AssertValidData(const snd_alias_t *alias)
{
    if ( !alias && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 1417, 0, "%s", "alias") )
        __debugbreak();
    if ( (alias->flags & 2) >> 1 )
    {
        if ( alias->distMin > (int)alias->distMax
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                        1421,
                        0,
                        "%s",
                        "alias->distMin <= alias->distMax") )
        {
            __debugbreak();
        }
        if ( alias->distMax > (int)alias->distReverbMax
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                        1422,
                        0,
                        "%s",
                        "alias->distMax <= alias->distReverbMax") )
        {
            __debugbreak();
        }
    }
}

unsigned int __cdecl SND_PlaySoundAlias(
                const snd_alias_list_t *aliasList,
                float volumeScale,
                SndEntHandle sndEnt,
                float *org,
                int timeshift,
                int fadeTime,
                const float *direction,
                snd_playback *playback,
                snd_occlusion_start_cache *ocache)
{
    unsigned int v10; // eax
    float volume; // [esp+1Ch] [ebp-F0h]
    float v12; // [esp+5Ch] [ebp-B0h]
    snd_listener *v13; // [esp+74h] [ebp-98h]
    int playbackId; // [esp+98h] [ebp-74h]
    float v15; // [esp+9Ch] [ebp-70h]
    unsigned int loopId; // [esp+A0h] [ebp-6Ch]
    float distance; // [esp+A4h] [ebp-68h]
    float distMax; // [esp+ACh] [ebp-60h]
    const snd_alias_list_t *secondaryList; // [esp+B0h] [ebp-5Ch]
    SndStartAliasInfo startAliasInfo; // [esp+B4h] [ebp-58h] BYREF
    int voiceIndex; // [esp+F0h] [ebp-1Ch]
    float priority; // [esp+F4h] [ebp-18h]
    int group; // [esp+F8h] [ebp-14h]
    const snd_alias_t *alias; // [esp+FCh] [ebp-10h]
    float playerPosition[3]; // [esp+100h] [ebp-Ch] BYREF

    if ( !org && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 1441, 0, "%s", "org") )
        __debugbreak();

    nanassertvec3(org);
    
    if ( (((unsigned int)sndEnt.handle >> 19) & 3) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    1443,
                    0,
                    "%s",
                    "sndEnt.field.localClientNum < MAX_LOCAL_CLIENTS") )
    {
        __debugbreak();
    }
    if ( !aliasList )
        return -1;
    SND_LogPlayedAliasId(aliasList->id);
    if ( snd_assert_on_play
        && snd_assert_on_play->current.integer
        && *(_BYTE *)snd_assert_on_play->current.integer
        && !_stricmp(snd_assert_on_play->current.string, aliasList->name)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    1454,
                    0,
                    "%s",
                    "stricmp(snd_assert_on_play->current.string, aliasList->name)") )
    {
        __debugbreak();
    }
    if ( aliasList->head->secondaryname )
    {
        v10 = SND_HashName(aliasList->head->secondaryname);
        secondaryList = SND_FindAliasFromId(v10);
        if ( secondaryList )
        {
            if ( SND_CheckValidSecondary(aliasList->head, secondaryList->head) )
                SND_PlaySoundAlias(secondaryList, volumeScale, sndEnt, org, timeshift, fadeTime, direction, 0, ocache);
        }
    }
    alias = SND_PickSoundAliasFromList((snd_alias_list_t*)aliasList, *(_WORD *)&sndEnt.field & 0xFFF);
    if ( !alias )
        return -1;
    SND_AssertValidData(alias);
    if ( !alias->soundFile )
    {
        if ( alias->subtitle )
            SND_SubtitleNotify(alias->subtitle, 0x1388u);
        return -1;
    }
    if ( alias->soundFile->type == 1 && !alias->soundFile->u.loadSnd )
        return -1;
    if ( (alias->flags & 0xC000) >> 14 != alias->soundFile->type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    1493,
                    0,
                    "%s",
                    "SND_AliasType(alias) == alias->soundFile->type") )
    {
        __debugbreak();
    }
    if ( (float)((float)alias->volMin / 65535.0) < 0.0000152879 && (float)((float)alias->volMax / 65535.0) < 0.0000152879 )
        return -1;
    group = (alias->flags & 0x3F0000) >> 16;
    if ( snd_skip_muted_sounds->current.enabled
        && (float)((float)SND_GetGroupByIndex(group)->attenuationMp / 65535.0) < 0.0000152879 )
    {
        return -1;
    }
    if ( (alias->flags & 2) >> 1 )
    {
        if ( !SND_ActiveListenerCount() )
            return -1;
        v13 = &g_snd.listeners[SND_GetListenerIndexNearestToOrigin(org)];
        playerPosition[0] = v13->orient.origin[0];
        playerPosition[1] = v13->orient.origin[1];
        playerPosition[2] = v13->orient.origin[2];
        distMax = (float)alias->distReverbMax;
        distance = Vec3DistanceSq(org, playerPosition);
        if ( distance > (float)(distMax * distMax) )
            return -1;
    }
    if ( (alias->flags & 1) != 0 )
    {
        loopId = SND_ContinueLoopingSound(alias->id, volumeScale, sndEnt, org, fadeTime, playback);
        if ( loopId != -1 )
            return loopId;
    }
    if ( fadeTime < 0 )
        return -1;
    if ( alias->dryCurve == -1 )
    {
        Com_PrintError(9, "Error: '%s' had bad volume curve\n", alias->name);
        return -1;
    }
    if ( (float)((float)alias->probability / 255.0) > 0.0 )
    {
        v12 = (float)alias->probability / 255.0;
        if ( Com_Random() > v12 )
            return -1;
    }
    volume = Snd_GetGlobalPriorityVolume(alias, org);
    priority = Snd_GetGlobalPriority(alias, volume);
    if ( !SND_Limit(
                    alias->name,
                    alias->id,
                    (snd_limit_type_t)((alias->flags & 0x6000000) >> 25),
                    alias->limitCount,
                    sndEnt,
                    0,
                    priority) )
        return -1;
    if ( !SND_Limit(
                    alias->name,
                    alias->id,
                    (snd_limit_type_t)((alias->flags & 0x18000000) >> 27),
                    alias->entityLimitCount,
                    sndEnt,
                    1,
                    priority) )
        return -1;
    if ( !SND_LimitVoice(alias, sndEnt) )
        return -1;
    memset((unsigned __int8 *)&startAliasInfo, 0xFFu, sizeof(startAliasInfo));
    if ( (float)((float)alias->volMin / 65535.0) > 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    1578,
                    0,
                    "%s",
                    "SND_AliasGetVolMin(alias) <= 1.0f") )
    {
        __debugbreak();
    }
    if ( (float)((float)alias->volMin / 65535.0) < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    1579,
                    0,
                    "%s",
                    "SND_AliasGetVolMin(alias) >= 0.0f") )
    {
        __debugbreak();
    }
    if ( (float)((float)alias->volMax / 65535.0) > 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    1580,
                    0,
                    "%s",
                    "SND_AliasGetVolMax(alias) <= 1.0f") )
    {
        __debugbreak();
    }
    if ( (float)((float)alias->volMax / 65535.0) < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                    1581,
                    0,
                    "%s",
                    "SND_AliasGetVolMax(alias) >= 0.0f") )
    {
        __debugbreak();
    }
    startAliasInfo.volModStart = volumeScale;
    startAliasInfo.alias = alias;
    startAliasInfo.sndEnt = sndEnt;
    startAliasInfo.org[0] = *org;
    startAliasInfo.org[1] = org[1];
    startAliasInfo.org[2] = org[2];
    if ( direction )
    {
        startAliasInfo.dir[0] = *direction;
        startAliasInfo.dir[1] = direction[1];
        startAliasInfo.dir[2] = direction[2];
    }
    else
    {
        memset(startAliasInfo.dir, 0, sizeof(startAliasInfo.dir));
    }
    startAliasInfo.timeshift = timeshift;
    startAliasInfo.startDelay = alias->startDelay;
    startAliasInfo.fadetime = (float)fadeTime;
    startAliasInfo.ocache = ocache;
    startAliasInfo.playback = playback;
    if ( playback )
        startAliasInfo.playbackId = playback->id;
    else
        startAliasInfo.playbackId = SND_AcquirePlaybackId();
    if ( snd_realDelay->current.value != 0.0 )
    {
        if ( (alias->flags & 4) >> 2 )
        {
            v15 = Vec3Distance(playerPosition, org);
            startAliasInfo.startDelay += (int)(float)((float)(1000.0 * v15) / (float)(13397.244 / snd_realDelay->current.value));
            if ( startAliasInfo.startDelay < 8 )
                startAliasInfo.startDelay = 0;
        }
    }
    voiceIndex = SND_FindFreeVoice(&startAliasInfo);
    if ( voiceIndex < 0 )
        return -1;
    playbackId = SD_StartAlias(&startAliasInfo, voiceIndex);
    if ( playbackId == -1 )
    {
        SND_StopVoice(voiceIndex);
        return -1;
    }
    if ( alias->subtitle )
        SND_AddLengthNotify(playbackId, alias->subtitle, SND_LENGTH_NOTIFY_SUBTITLE);
    return playbackId;
}

char __cdecl SND_CheckValidSecondary(const snd_alias_t *alias, const snd_alias_t *secondaryAlias)
{
    const snd_alias_t *tertiaryAlias; // [esp+30h] [ebp-8h]
    int secondaryAliasRecursionCounter; // [esp+34h] [ebp-4h]

    secondaryAliasRecursionCounter = 0;
    if ( (alias->flags & 1) != 0 || (secondaryAlias->flags & 1) == 0 )
    {
        if ( (alias->flags & 1) == 0 || (secondaryAlias->flags & 1) != 0 )
        {
            for ( tertiaryAlias = secondaryAlias;
                        tertiaryAlias && tertiaryAlias->secondaryname && secondaryAliasRecursionCounter < 10;
                        tertiaryAlias = SND_PickSoundAlias(tertiaryAlias->secondaryname, 0) )
            {
                if ( !I_stricmp(alias->name, tertiaryAlias->secondaryname) )
                {
                    Com_PrintError(
                        9,
                        "Error: Infinite recursion in secondary aliases sequenced together.\nAlias sequence start: '%s'->'%s'\n",
                        alias->name,
                        alias->secondaryname);
                    return 0;
                }
                ++secondaryAliasRecursionCounter;
            }
            return 1;
        }
        else
        {
            Com_PrintError(
                9,
                "ERROR: a looping alias cannot have a non-looping secondary alias.\nERROR: Alias sequence: '%s'->'%s'\n",
                alias->name,
                alias->secondaryname);
            return 0;
        }
    }
    else
    {
        Com_PrintError(
            9,
            "ERROR: a non-looping alias cannot have a looping secondary alias.\nERROR: Alias sequence: '%s'->'%s'\n",
            alias->name,
            alias->secondaryname);
        return 0;
    }
}

char __cdecl SND_Limit(
                const char *name,
                unsigned int aliasHash,
                snd_limit_type_t limitType,
                int limitCount,
                SndEntHandle ent,
                bool useEnt,
                float priority)
{
    int oldest; // [esp+0h] [ebp-Ch] BYREF
    int leastImportant; // [esp+4h] [ebp-8h] BYREF
    int count; // [esp+8h] [ebp-4h] BYREF

    if ( limitType == SND_LIMIT_NONE )
        return 1;
    count = 0;
    oldest = -1;
    leastImportant = -1;
    SND_GetPlayingInfo(aliasHash, &count, &oldest, &leastImportant, ent, useEnt);
    if ( count >= limitCount )
    {
        if ( limitType == SND_LIMIT_REJECT )
            return 0;
        if ( limitType == SND_LIMIT_OLDEST && oldest >= 0 )
            SND_StopVoice(oldest);
        if ( limitType == SND_LIMIT_PRIORITY )
        {
            if (leastImportant < 0
                || priority <= (float)(g_snd.voice[leastImportant].globalPriority + snd_playing_priority_boost->current.value))
            {
                return 0;
            }
            SND_StopVoice(leastImportant);
        }
    }
    return 1;
}

void __cdecl SND_GetPlayingInfo(
                unsigned int aliasHash,
                int *pcount,
                int *poldest,
                int *least,
                SndEntHandle ent,
                bool useEnt)
{
    unsigned int i; // [esp+0h] [ebp-14h]
    float leastPriority; // [esp+4h] [ebp-10h]
    int oldTime; // [esp+8h] [ebp-Ch]
    int oldest; // [esp+Ch] [ebp-8h]
    int count; // [esp+10h] [ebp-4h]

    count = 0;
    oldest = -1;
    oldTime = 0x7FFFFFFF;
    leastPriority = 1000000.0f;
    if (!aliasHash
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
            1302,
            0,
            "%s",
            "aliasHash != SND_INVALID_HASH"))
    {
        __debugbreak();
    }
    for (i = 0; i < SND_MAX_VOICES; ++i)
    {
        if (g_snd.voiceAliasHash[i] == aliasHash && (!useEnt || g_snd.voice[i].sndEnt.handle == ent.handle))
        {
            ++count;
            if (g_snd.voice[i].startTime < oldTime)
            {
                oldTime = g_snd.voice[i].startTime;
                oldest = i;
            }
            if (leastPriority > g_snd.voice[i].globalPriority)
            {
                leastPriority = g_snd.voice[i].globalPriority;
                *least = i;
            }
        }
    }
    if (pcount)
        *pcount = count;
    if (poldest)
        *poldest = oldest;
}

char __cdecl SND_LimitVoice(const snd_alias_t *alias, SndEntHandle ent)
{
    unsigned int j; // [esp+0h] [ebp-8h]
    unsigned int i; // [esp+4h] [ebp-4h]

    if (SND_IsAliasVoice(alias))
    {
        for (i = 0; i < SND_MAX_VOICES; ++i)
        {
            if (g_snd.voiceAliasHash[i] && g_snd.voice[i].sndEnt.handle == ent.handle)
            {
                if (SND_IsAliasVoice(g_snd.voice[i].alias))
                    SND_StopVoice(i);
            }
        }
    }
    if ((alias->flags & 0x3F0000) >> 16 == g_snd.announcerGroup)
    {
        for (j = 0; j < SND_MAX_VOICES; ++j)
        {
            if (g_snd.voiceAliasHash[j] && (g_snd.voice[j].alias->flags & 0x3F0000) >> 16 == g_snd.announcerGroup)
                return 0;
        }
    }
    return 1;
}

bool __cdecl SND_IsAliasVoice(const snd_alias_t *alias)
{
    if ( !alias && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_alias_db.h", 429, 0, "%s", "alias") )
        __debugbreak();
    return (alias->flags & 0x800) >> 11 != 0;
}

int __cdecl SND_GetCurrentReverb()
{
    if ( snd_reverb_override->current.integer && *(_BYTE *)snd_reverb_override->current.integer )
        return SND_HashName(snd_reverb_override->current.string);
    if ( g_snd.effect && g_snd.effect->reverbId )
        return g_snd.effect->reverbId;
    return g_snd.defaultHash;
}

void __cdecl SND_UpdateDebugAlias()
{
    SndEntHandle entHandle; // [esp+20h] [ebp-14h]
    float soundDir[3]; // [esp+24h] [ebp-10h] BYREF
    unsigned int i; // [esp+30h] [ebp-4h]

    if ( snd_stop_alias && snd_stop_alias->current.integer && *(_BYTE *)snd_stop_alias->current.integer )
    {
        for ( i = 0; i < SND_MAX_VOICES; ++i )
        {
            if ( g_snd.voiceAliasHash[i] )
            {
                if (strstr(g_snd.voice[i].alias->name, snd_stop_alias->current.string))
                    SND_StopVoice(i);
            }
        }
        Dvar_SetString((dvar_s *)snd_stop_alias, "");
    }
    if ( snd_start_alias && snd_start_alias->current.integer && *(_BYTE *)snd_start_alias->current.integer )
    {
        soundDir[0] = 1.0f;
        soundDir[1] = 0.0f;
        soundDir[2] = 0.0f;
        entHandle.field = SND_EntHandle(0, 4094, 0, 0, 1, TEAM_FREE).field;
        SND_Play((char *)snd_start_alias->current.integer, 0, 1.0, entHandle, g_snd.listeners[0].orient.origin, soundDir, 0);
        Dvar_SetString((dvar_s *)snd_start_alias, "");
    }
}

void __cdecl SNDL_Update()
{
    float fdt; // [esp+4Ch] [ebp-8h]
    int frametime; // [esp+50h] [ebp-4h]

    if (!SND_Active())
    {
        return;
    }

    PROF_SCOPED("SNDL_Update");

    {
        PROF_SCOPED("time update");
        frametime = Sys_Milliseconds() - g_snd.time;
        g_snd.time = Sys_Milliseconds();
        fdt = (float)frametime / 1000.0;
        ++g_snd.frame;
        SND_UpdateDebugAlias();

        g_snd.timescale = (float)((float)((float)((float)(1.0 - g_snd.scriptTimescale) * 1.0)
            + (float)(g_snd.gameState.timescale * g_snd.scriptTimescale))
            * snd_timescale_filter->current.value)
            + (float)((float)(1.0 - snd_timescale_filter->current.value) * g_snd.timescale);
    }

    {
        PROF_SCOPED("SND_UpdateStaticSounds");
        SND_UpdateStaticSounds();
    }
    
    SD_PreUpdate();
    SND_UpdatePause();
    {
        PROF_SCOPED("faders");
        SND_UpdateMasterVolumes(fdt);
        SND_UpdateRoomEffects(frametime);
    }
    {
        PROF_SCOPED("mixing");
        SND_UpdateSnapshot(fdt);
    }
    {
        PROF_SCOPED("systems");
        SND_LosOcclusionUpdate();
    }
    {
        PROF_SCOPED("update voices");
        SND_UpdateVoices(frametime);
    }
    {
        PROF_SCOPED("Driver Post Update");
        //BLOPS_NULLSUB();
    }
    {
        PROF_SCOPED("debug");
        SND_DebugDrawWorldSounds(snd_draw3D->current.integer);
    }
}

void SND_UpdatePause()
{
    int paused;
    
    paused = g_snd.gameState.gamePaused || g_snd.forcePause;

    if (paused != g_snd.paused)
    {
        if (paused)
        {
            SND_PauseSounds();
            g_snd.pausetime = g_snd.time;
        }
        else
        {
            SND_UnpauseSounds();
            g_snd.pausetime = 0;
        }
    }
    g_snd.paused = paused;
}

void SND_PauseSounds()
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 74; ++i )
    {
        if ( g_snd.voiceAliasHash[i] )
        {
            if ( SND_IsAliasPausable(g_snd.voice[i].alias))
            {
                SD_PauseVoice(i);
                g_snd.voice[i].paused = true;
            }
        }
    }
}

void SND_UnpauseSounds()
{
    int i; // [esp+0h] [ebp-8h]
    int timeshift; // [esp+4h] [ebp-4h]

    timeshift = g_snd.time - g_snd.pausetime;
    for (i = 0; i < 74; ++i)
    {
        if (g_snd.voiceAliasHash[i])
        {
            if (g_snd.voice[i].paused)
            {
                g_snd.voice[i].startTime += timeshift;
                g_snd.voice[i].soundFileInfo.endtime += timeshift;
                g_snd.voice[i].paused = 0;
                SD_UnpauseVoice(i);
            }
        }
    }
}

void __cdecl SND_UpdateMasterVolumes(float dt)
{
    unsigned int i; // [esp+30h] [ebp-4h]

    SND_FaderUpdate(&g_snd.volume, dt);
    if (g_snd.volume.value < 0.0000152879 && g_snd.volume.goal < 0.0000152879)
    {
        for (i = 0; i < SND_MAX_VOICES; ++i)
        {
            if (g_snd.voiceAliasHash[i]
                && (SND_GroupCategory((g_snd.voice[i].alias->flags & 0x3F0000) >> 16) != SND_CATEGORY_UI
                    || (g_snd.voice[i].alias->flags & 1) != 0))
            {
                SND_StopVoice(i);
            }
        }
    }
}

void __cdecl SND_UpdateVoices(int frametime)
{
    float dt; // [esp+4h] [ebp-38h]
    float v3; // [esp+20h] [ebp-1Ch]
    float v4; // [esp+24h] [ebp-18h]
    unsigned int voiceIndex; // [esp+28h] [ebp-14h]
    unsigned int j; // [esp+2Ch] [ebp-10h]
    unsigned int i; // [esp+30h] [ebp-Ch]
    int count; // [esp+38h] [ebp-4h]

    v3 = (float)frametime / 1000.0;
    if ((float)(v3 - 0.25) < 0.0)
        v4 = (float)frametime / 1000.0;
    else
        v4 = 0.25f;
    if ((float)(0.0 - v3) < 0.0)
        dt = v4;
    else
        dt = 0.0f;
    count = 0;

    {
        PROF_SCOPED("SND_UpdateVoicePosition");
        for (i = 0; i < SND_MAX_VOICES; ++i)
        {
            if (g_snd.voiceAliasHash[i])
                SND_UpdateVoicePosition(&g_snd.voice[i], 0);
        }
    }
    {
        PROF_SCOPED("SND_UpdateVoice");
        for (j = 0; j < SND_MAX_VOICES; ++j)
        {
            if (g_snd.voiceAliasHash[j])
                SND_UpdateVoice(&g_snd.voice[j], dt);
        }
    }
    {
        PROF_SCOPED("SD_UpdateVoice");
        for (voiceIndex = 0; voiceIndex < SND_MAX_VOICES; ++voiceIndex)
        {
            if (g_snd.voiceAliasHash[voiceIndex])
            {
                SD_UpdateVoice(voiceIndex);
                ++count;
            }
        }
    }
}

void __cdecl SND_UpdateRoomEffects(int frametime)
{
    iassert(g_snd.effect);

    if (g_snd.effect->dryrate != 0.0 || g_snd.effect->wetrate != 0.0)
    {
        if (g_snd.effect->dryrate != 0.0)
        {
            g_snd.effect->drylevel = (float)((float)frametime * g_snd.effect->dryrate) + g_snd.effect->drylevel;
            if (g_snd.effect->dryrate >= 0.0)
            {
                if (g_snd.effect->drylevel >= g_snd.effect->drygoal)
                {
                    g_snd.effect->drylevel = g_snd.effect->drygoal;
                    g_snd.effect->dryrate = 0.0f;
                }
            }
            else if (g_snd.effect->drygoal >= g_snd.effect->drylevel)
            {
                g_snd.effect->drylevel = g_snd.effect->drygoal;
                g_snd.effect->dryrate = 0.0f;
            }
        }
        if (g_snd.effect->wetrate != 0.0)
        {
            g_snd.effect->wetlevel = (float)((float)frametime * g_snd.effect->wetrate) + g_snd.effect->wetlevel;
            if (g_snd.effect->wetrate >= 0.0)
            {
                if (g_snd.effect->wetlevel >= g_snd.effect->wetgoal)
                {
                    g_snd.effect->wetlevel = g_snd.effect->wetgoal;
                    g_snd.effect->wetrate = 0.0f;
                }
            }
            else if (g_snd.effect->wetgoal >= g_snd.effect->wetlevel)
            {
                g_snd.effect->wetlevel = g_snd.effect->wetgoal;
                g_snd.effect->wetrate = 0.0f;
            }
        }
    }
}

void SND_UpdateStaticSounds()
{
    const snd_alias_list_t *AliasFromId; // eax
    const snd_alias_list_t *v2; // eax
    float *v3; // [esp+38h] [ebp-84h]
    float *v4; // [esp+3Ch] [ebp-80h]
    SndEntHandle v5; // [esp+40h] [ebp-7Ch]
    SndEntHandle sndEnt; // [esp+44h] [ebp-78h]
    snd_occlusion_start_cache ocache; // [esp+48h] [ebp-74h] BYREF
    char v8; // [esp+5Fh] [ebp-5Dh]
    float v9[3]; // [esp+60h] [ebp-5Ch] BYREF
    unsigned int j; // [esp+6Ch] [ebp-50h]
    float tmpOrigin[3]; // [esp+70h] [ebp-4Ch] BYREF
    float tmpDistance; // [esp+7Ch] [ebp-40h]
    unsigned int l; // [esp+80h] [ebp-3Ch]
    float origin[3]; // [esp+84h] [ebp-38h] BYREF
    snd_occlusion_start_cache cache; // [esp+90h] [ebp-2Ch] BYREF
    bool firstUpdate; // [esp+A7h] [ebp-15h]
    float distance; // [esp+A8h] [ebp-14h]
    float direction[3]; // [esp+ACh] [ebp-10h] BYREF
    int i; // [esp+B8h] [ebp-4h]

    if (SND_ActiveListenerCount())
    {
        for (i = 0; (unsigned int)i < 0x80; ++i)
        {
            if (g_snd.lineEmitters[i].id)
            {
                firstUpdate = 0;
                if (!g_snd.lineEmitters[i].alias)
                {
                    AliasFromId = SND_FindAliasFromId(g_snd.lineEmitters[i].id);
                    g_snd.lineEmitters[i].alias = AliasFromId;
                    firstUpdate = 1;
                }
                if (g_snd.lineEmitters[i].alias)
                {
                    distance = FLT_MAX;
                    for (l = 0; l < SND_ActiveListenerCount(); ++l)
                    {
                        SND_GetNearestPointOnSegment(
                            g_snd.listeners[l].orient.origin,
                            g_snd.lineEmitters[i].origin[0],
                            g_snd.lineEmitters[i].origin[1],
                            tmpOrigin);
                        tmpDistance = Vec3Distance(tmpOrigin, g_snd.listeners[l].orient.origin);
                        if (distance > tmpDistance)
                        {
                            distance = tmpDistance;
                            origin[0] = tmpOrigin[0];
                            origin[1] = tmpOrigin[1];
                            origin[2] = tmpOrigin[2];
                        }
                    }
                    memset(direction, 0, sizeof(direction));
                    memset(&cache, 0, sizeof(cache));
                    sndEnt.field = SND_EntHandle(0, i + 3966, 0, 0, 1, TEAM_FREE).field;
                    SND_PlaySoundAlias(
                        g_snd.lineEmitters[i].alias,
                        1.0,
                        sndEnt,
                        origin,
                        0,
                        firstUpdate ? 0x1F4 : 0,
                        direction,
                        0,
                        &cache);
                }
                else
                {
                    SND_LogMissingAliasId(g_snd.lineEmitters[i].id);
                    g_snd.lineEmitters[i].id = 0;
                    v4 = g_snd.lineEmitters[i].origin[0];
                    *v4 = 0.0f;
                    v4[1] = 0.0f;
                    v4[2] = 0.0f;
                    v3 = g_snd.lineEmitters[i].origin[1];
                    *v3 = 0.0f;
                    v3[1] = 0.0f;
                    v3[2] = 0.0f;
                }
            }
        }
        for (j = 0; j < 0x100; ++j)
        {
            if (g_snd.loopEmitters[j].id)
            {
                v8 = 0;
                if (!g_snd.loopEmitters[j].alias)
                {
                    v2 = SND_FindAliasFromId(g_snd.loopEmitters[j].id);
                    g_snd.loopEmitters[j].alias = v2;
                    v8 = 1;
                }
                if (g_snd.loopEmitters[j].alias)
                {
                    memset(v9, 0, sizeof(v9));
                    memset(&ocache, 0, sizeof(ocache));
                    v5.field = SND_EntHandle(0, j + 3710, 0, 0, 1, TEAM_FREE).field;
                    SND_PlaySoundAlias(
                        g_snd.loopEmitters[j].alias,
                        1.0,
                        v5,
                        g_snd.loopEmitters[j].origin,
                        0,
                        v8 != 0 ? 0x1F4 : 0,
                        v9,
                        0,
                        &ocache);
                }
                else
                {
                    SND_LogMissingAliasId(g_snd.loopEmitters[j].id);
                    g_snd.loopEmitters[j].id = 0;
                }
            }
        }
    }
}

void __cdecl SND_StopVoice(int voiceIndex)
{
    if (g_snd.voiceAliasHash[voiceIndex])
    {
        if (g_snd.voice[voiceIndex].alias
            && snd_assert_on_stop
            && snd_assert_on_stop->current.integer
            && *(_BYTE *)snd_assert_on_stop->current.integer
            && !_stricmp(snd_assert_on_stop->current.string, g_snd.voice[voiceIndex].alias->name)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                2082,
                0,
                "%s",
                "stricmp(snd_assert_on_stop->current.string, SND_AliasGetName(g_snd.voice[voiceIndex].alias))"))
        {
            __debugbreak();
        }
        SD_StopVoice(voiceIndex);
        SND_ResetVoiceInfo(voiceIndex);
        if (g_snd.voiceAliasHash[voiceIndex])
        {
            if (!Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                2090,
                0,
                "%s",
                "SND_IsVoiceFree(voiceIndex)"))
                __debugbreak();
        }
    }
}

int __cdecl SND_Active()
{
    return g_snd.init;
}

void __cdecl SND_Init()
{
    iassert(!g_snd.init);
    iassert(Sys_IsMainThread());

    memset(&g_snd, 0, sizeof(g_snd));

    SND_InitDvar();
    g_snd.defaultHash = SND_HashName("default");
    if (SND_ShouldInit())
    {
        SND_InitLog();
        //_initp_misc_winxfltr(v1);
        SND_ResetPlaybacks();
        g_snd.global_constants = DB_FindXAssetHeader(ASSET_TYPE_SNDDRIVER_GLOBALS, (char*)"singleton", 1, -1).sndDriverGlobals;
        SND_ParseInit(
            g_snd.global_constants->groupCount,
            g_snd.global_constants->groups,
            g_snd.global_constants->curveCount,
            g_snd.global_constants->curves,
            g_snd.global_constants->panCount,
            g_snd.global_constants->pans,
            g_snd.global_constants->snapshotGroupCount,
            g_snd.global_constants->snapshotGroups);
        g_snd.defaultCurve = SND_GetCurveById(g_snd.defaultHash);

        iassert(g_snd.global_constants);

        g_snd.maximumPriority = 0.0f;
        SND_FaderSetGoal(&g_snd.minimumPriority, 0.0);
        SND_FaderSetRate(&g_snd.minimumPriority, 20.0);
        g_snd.minimumPriority.value = g_snd.minimumPriority.goal;
        g_snd.gameState.timescale = 1.0f;
        g_snd.effect = g_snd.envEffects;
        g_snd.envEffects[0].reverbId = g_snd.defaultHash;
        g_snd.envEffects[0].drylevel = 1.0f;
        g_snd.effect->drygoal = 1.0f;
        g_snd.effect->dryrate = 0.0f;
        g_snd.effect->wetlevel = 0.0f;
        g_snd.effect->wetgoal = 0.0f;
        g_snd.effect->wetrate = 0.0f;
        g_snd.effect->active = true;
        g_snd.forcePause = 0;
        g_snd.timescale = 1.0f;
        g_snd.scriptTimescale = 0.0f;
        Snd_StreamInit();
        memset(g_snd.voiceAliasHash, 0, sizeof(g_snd.voiceAliasHash));

        SND_InitGroups();
        SND_InitSnapshot();
        SND_LosOcclusionInit();
        SND_DebugInit();

        g_snd.playbackIdCounter = 1;
        SND_FaderSetGoal(&g_snd.volume, 1.0);
        SND_FaderSetValue(&g_snd.volume, 1.0);
        SND_FaderSetRate(&g_snd.volume, 1.0);

        g_snd.time = Sys_Milliseconds();
        g_snd.looptime = 0;
        g_snd.traceSequence = 0;
        g_snd.lastTraceSequence = 0;
        memset(g_snd.meters, 0, sizeof(g_snd.meters));
        g_snd.init = SD_Init();
        SND_AsyncInit();
        Sys_SetSndInitializedEvent();
    }
}

void SND_InitGroups()
{
    iassert(g_snd.global_constants);

    for (int i = 0; i < SND_GetGroupCount(); i++)
    {
        if (!I_stricmp("voice", SND_GetGroupByIndex(i)->name))
        {
            g_snd.voiceGroup = i;
        }
        if (!I_stricmp("voice_announcer", SND_GetGroupByIndex(i)->name))
        {
            g_snd.announcerGroup = i;
        }
    }
}

void __cdecl SND_Shutdown()
{
    if ( g_snd.init )
    {
        SND_DebugFini();
        iassert(Sys_IsMainThread());
        SND_ShutdownVoices();
        SND_UpdateWait();
        SD_Shutdown();
        SND_LosOcclusionFini();
        memset(&g_snd, 0, sizeof(g_snd));
        Sleep(1000);
        Snd_StreamFini();
    }
}

void __cdecl SND_ShutdownVoices()
{
    iassert(Sys_IsMainThread());

    for (int i = 0; i < SND_MAX_VOICES; ++i)
    {
        SND_StopVoice(i);
    }
}

void __cdecl SND_InitSnapshot()
{
    unsigned int i; // [esp+Ch] [ebp-14h]
    snd_snapshot_category *v2; // [esp+10h] [ebp-10h]
    unsigned int j; // [esp+14h] [ebp-Ch]
    unsigned int c; // [esp+1Ch] [ebp-4h]

    for (c = 0; c < 0xB; ++c)
    {
        g_snd.snapshotCategories[c].length = 0.0f;
        g_snd.snapshotCategories[c].snapshot = g_snd.defaultHash;
    }
    SND_UpdateSnapshot(0.0);
    for (j = 0; j < 0xB; ++j)
    {
        v2 = &g_snd.snapshotCategories[j];
        for (i = 0; i < g_snd.global_constants->groupCount; ++i)
        {
            v2->attenuation[i].value = v2->attenuation[i].goal;
            v2->occlusion[i].value = v2->occlusion[i].goal;
        }
    }
}

void __cdecl SND_UpdateSnapshot(float dt)
{
    unsigned int v2; // eax
    int v3; // edx
    float v4; // [esp+Ch] [ebp-390h]
    float value; // [esp+10h] [ebp-38Ch]
    float v6; // [esp+14h] [ebp-388h]
    float v7; // [esp+18h] [ebp-384h]
    float v8; // [esp+1Ch] [ebp-380h]
    snd_snapshot_category *v9; // [esp+124h] [ebp-278h]
    unsigned int kk; // [esp+128h] [ebp-274h]
    unsigned int jj; // [esp+12Ch] [ebp-270h]
    unsigned int ii; // [esp+130h] [ebp-26Ch]
    const snd_snapshot *v13; // [esp+134h] [ebp-268h]
    snd_snapshot_category *v14; // [esp+138h] [ebp-264h]
    const snd_snapshot *OcclusionSnapshot; // [esp+13Ch] [ebp-260h]
    unsigned int n; // [esp+140h] [ebp-25Ch]
    unsigned int m; // [esp+144h] [ebp-258h]
    unsigned int k; // [esp+148h] [ebp-254h]
    float listner[3]; // [esp+14Ch] [ebp-250h] BYREF
    float distance; // [esp+158h] [ebp-244h]
    float dlerp; // [esp+15Ch] [ebp-240h]
    float timePlayed; // [esp+160h] [ebp-23Ch]
    float lerp; // [esp+164h] [ebp-238h]
    const snd_curve *fadeOutCurve; // [esp+168h] [ebp-234h]
    const snd_curve *fadeInCurve; // [esp+16Ch] [ebp-230h]
    float timeEnd; // [esp+170h] [ebp-22Ch]
    const snd_snapshot *SnapshotById; // [esp+174h] [ebp-228h]
    const snd_snapshot *oss; // [esp+178h] [ebp-224h]
    bool isLooping; // [esp+17Fh] [ebp-21Dh]
    snd_voice_t *voice; // [esp+180h] [ebp-21Ch]
    unsigned int j; // [esp+184h] [ebp-218h]
    unsigned int i; // [esp+188h] [ebp-214h]
    snd_snapshot_category *category; // [esp+18Ch] [ebp-210h]
    unsigned int c; // [esp+190h] [ebp-20Ch]
    snd_snapshot_category *aliasCategory; // [esp+194h] [ebp-208h]
    const snd_curve *defaultCurve; // [esp+198h] [ebp-204h]
    float voiceAttenuations[64]; // [esp+19Ch] [ebp-200h]
    float voiceOcclusions[64]; // [esp+29Ch] [ebp-100h]

    defaultCurve = g_snd.defaultCurve;
    if (snd_debug_snapshot && snd_debug_snapshot->current.integer && *(_BYTE *)snd_debug_snapshot->current.integer)
    {
        v2 = SND_HashName(snd_debug_snapshot->current.string);
        SNDL_SetSnapshot(SND_SNAPSHOT_DEBUG, v2, 0.0, 1.0);
    }
    for (c = 0; c < 0xB; ++c)
    {
        category = &g_snd.snapshotCategories[c];
        if (category->length > 0.0)
            category->length = category->length - dt;
        if (category->length < 0.0)
            category->snapshot = 0;
        if (!category->snapshot)
        {
            category->snapshot = g_snd.defaultHash;
            category->length = 0.0f;
        }
    }
    for (i = 0; i < g_snd.global_constants->snapshotGroupCount; ++i)
    {
        voiceAttenuations[i] = 1.0f;
        voiceOcclusions[i] = 1.0f;
    }
    for (j = 0; j < SND_MAX_VOICES; ++j)
    {
        voice = &g_snd.voice[j];
        if (g_snd.voiceAliasHash[j])
        {
            if (voice->soundFileInfo.loadingState == SFLS_LOADED)
            {
                if (voice->alias->duck)
                {
                    SnapshotById = SND_GetSnapshotById(voice->alias->duck);
                    oss = SND_GetOcclusionSnapshot(SnapshotById);
                    isLooping = (g_snd.voice[j].alias->flags & 1) != 0;
                    if (SnapshotById)
                    {
                        if (oss)
                        {
                            fadeInCurve = SND_GetCurveById(SnapshotById->fadeInCurve);
                            fadeOutCurve = SND_GetCurveById(SnapshotById->fadeOutCurve);
                            timeEnd = (float)(voice->totalMsec - voice->alias->startDelay) / 1000.0;
                            v3 = g_snd.pausetime ? g_snd.pausetime - voice->startTime : g_snd.time - voice->startTime;
                            timePlayed = (float)(v3 - voice->alias->startDelay) / 1000.0;
                            if (timePlayed >= 0.0 && (isLooping || (float)(SnapshotById->fadeIn + SnapshotById->fadeOut) <= timeEnd))
                            {
                                lerp = 1.0f;
                                if (SnapshotById->fadeIn <= timePlayed)
                                {
                                    if (!isLooping
                                        && timePlayed > (float)(timeEnd - SnapshotById->fadeOut)
                                        && SnapshotById->fadeOut > 0.0)
                                    {
                                        lerp = (float)(timeEnd - timePlayed) / SnapshotById->fadeOut;
                                        lerp = Snd_CurveEvalOverRange(fadeOutCurve, 1.0 - lerp, 0.0, 1.0);
                                    }
                                }
                                else
                                {
                                    lerp = timePlayed / SnapshotById->fadeIn;
                                    lerp = Snd_CurveEvalOverRange(fadeInCurve, lerp, 0.0, 1.0);
                                }
                                if ((LODWORD(lerp) & 0x7F800000) == 0x7F800000
                                    && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                                        2418,
                                        0,
                                        "%s",
                                        "!IS_NAN(lerp)"))
                                {
                                    __debugbreak();
                                }
                                if (SnapshotById->distance >= 1.0 && (voice->alias->flags & 2) >> 1)
                                {
                                    SND_GetNearestListenerPosition(voice->position, listner);
                                    distance = Vec3Distance(voice->position, listner);
                                    dlerp = Snd_CurveEvalOverRange(defaultCurve, distance, 0.0, SnapshotById->distance);
                                    if ((float)(dlerp - lerp) < 0.0)
                                        v8 = dlerp;
                                    else
                                        v8 = lerp;
                                    lerp = v8;
                                }
                                for (k = 0; k < g_snd.global_constants->snapshotGroupCount; ++k)
                                {
                                    if ((float)((float)((float)((float)(1.0 - lerp) * 1.0) + (float)(SnapshotById->attenuation[k] * lerp))
                                        - voiceAttenuations[k]) < 0.0)
                                        v7 = (float)((float)(1.0 - lerp) * 1.0) + (float)(SnapshotById->attenuation[k] * lerp);
                                    else
                                        v7 = voiceAttenuations[k];
                                    voiceAttenuations[k] = v7;
                                    if ((float)((float)((float)((float)(1.0 - lerp) * 1.0) + (float)(oss->attenuation[k] * lerp))
                                        - voiceOcclusions[k]) < 0.0)
                                        v6 = (float)((float)(1.0 - lerp) * 1.0) + (float)(oss->attenuation[k] * lerp);
                                    else
                                        v6 = voiceOcclusions[k];
                                    voiceOcclusions[k] = v6;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    aliasCategory = g_snd.snapshotCategories;
    for (m = 0; m < g_snd.global_constants->snapshotGroupCount; ++m)
    {
        SND_FaderSetGoal(&aliasCategory->attenuation[m], voiceAttenuations[m]);
        SND_FaderSetRate(&aliasCategory->attenuation[m], 30.0);
        SND_FaderUpdate(&aliasCategory->attenuation[m], dt);
        SND_FaderSetGoal(&aliasCategory->occlusion[m], voiceOcclusions[m]);
        SND_FaderSetRate(&aliasCategory->occlusion[m], 30.0);
        SND_FaderUpdate(&aliasCategory->occlusion[m], dt);
    }
    for (n = 0; n < 0xB; ++n)
    {
        if (n)
        {
            v14 = &g_snd.snapshotCategories[n];
            v13 = SND_GetSnapshotById(v14->snapshot);
            OcclusionSnapshot = SND_GetOcclusionSnapshot(v13);
            for (ii = 0; ii < g_snd.global_constants->snapshotGroupCount; ++ii)
            {
                SND_FaderSetRateTime(&v14->attenuation[ii], v13->fadeIn);
                SND_FaderSetGoal(&v14->attenuation[ii], 1.0 - (float)((float)(1.0 - v13->attenuation[ii]) * v14->amount));
                SND_FaderUpdate(&v14->attenuation[ii], dt);
                SND_FaderSetRateTime(&v14->occlusion[ii], OcclusionSnapshot->fadeIn);
                SND_FaderSetGoal(
                    &v14->occlusion[ii],
                    1.0 - (float)((float)(1.0 - OcclusionSnapshot->attenuation[ii]) * v14->amount));
                SND_FaderUpdate(&v14->occlusion[ii], dt);
            }
        }
    }
    for (jj = 0; jj < g_snd.global_constants->snapshotGroupCount; ++jj)
    {
        g_snd.snapshotAttenuation[jj] = 1.0f;
        g_snd.snapshotOcclusion[jj] = 1.0f;
        for (kk = 0; kk < 0xB; ++kk)
        {
            if ((float)(g_snd.snapshotCategories[kk].attenuation[jj].value - g_snd.snapshotAttenuation[jj]) < 0.0)
                value = g_snd.snapshotCategories[kk].attenuation[jj].value;
            else
                value = g_snd.snapshotAttenuation[jj];
            g_snd.snapshotAttenuation[jj] = value;
            v9 = &g_snd.snapshotCategories[kk];
            if ((float)(v9->occlusion[jj].value - g_snd.snapshotOcclusion[jj]) < 0.0)
                v4 = v9->occlusion[jj].value;
            else
                v4 = g_snd.snapshotOcclusion[jj];
            g_snd.snapshotOcclusion[jj] = v4;
        }
    }
}

double __cdecl SND_GetPitch(snd_voice_t *voice)
{
    float v3; // [esp+4h] [ebp-34h]
    float pitch; // [esp+30h] [ebp-8h]
    const snd_alias_t *alias; // [esp+34h] [ebp-4h]

    iassert(voice);
    alias = voice->alias;
    iassert(alias);

    if (!snd_enable_pitch->current.enabled)
        return 1.0;

    pitch = (float)((float)(1.0 - voice->pitchModSeed) * (float)((float)alias->pitchMin / 32767.0)) + (float)((float)((float)alias->pitchMax / 32767.0) * voice->pitchModSeed);
    iassert(!IS_NAN(pitch));

    if (SND_IsAliasTimescale(voice->alias))
    {
        pitch *= (((g_snd.timescale - 1.0) * snd_pitch_timescale->current.value) + 1.0);
    }

    iassert(!IS_NAN(pitch));
    pitch *= voice->script_pitch.value;
    iassert(!IS_NAN(pitch));

    if ((alias->flags & 2) >> 1 && SND_IsOnSameTeam(voice->closestListenerIndex, voice->sndEnt))
    {
        pitch *= (voice->alias->teamPitchMod / 32767.0);
    }

    iassert(!IS_NAN(pitch));
    if ((voice->alias->flags & 0x10) >> 4)
    {
        pitch += voice->doppler.value;
    }
    iassert(!IS_NAN(pitch));

    pitch *= snd_global_pitch->current.value;

    if ((fabs(pitch - 1.0)) > 0.0000152879)
        voice->pitchShift = 1;
    if ((fabs(pitch - 1.0) ) < 0.0000152879)
        pitch = 1.0f;
    if ((float)(pitch - 1.9) < 0.0)
        v3 = pitch;
    else
        v3 = 1.9f;
    if ((float)(0.0099999998 - pitch) < 0.0)
        return v3;
    else
        return 0.0099999998f;
}

bool __cdecl SND_IsAliasTimescale(const snd_alias_t *alias)
{
    if ( !alias && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_alias_db.h", 423, 0, "%s", "alias") )
        __debugbreak();
    return (alias->flags & 0x400) >> 10 != 0;
}

bool __cdecl SND_IsOnSameTeam(unsigned int listenerIndex, SndEntHandle entNum)
{
    if ((*(_WORD *)&entNum.field & 0xFFF) == 0xFFF)
        return 0;
    if ((((unsigned int)entNum.handle >> 23) & 3) != 0)
        return (((unsigned int)entNum.handle >> 23) & 3) == g_snd.listeners[listenerIndex].team;
    return 0;
}

double __cdecl SND_GetBaseLevel(const snd_voice_t *voice)
{
    const char *v2; // eax
    const char *v4; // eax
    float v6; // [esp+8h] [ebp-70h]
    unsigned int i; // [esp+64h] [ebp-14h]
    float attenuation; // [esp+68h] [ebp-10h]
    float attenuationa; // [esp+68h] [ebp-10h]
    float attenuationb; // [esp+68h] [ebp-10h]
    float attenuationc; // [esp+68h] [ebp-10h]
    float attenuationd; // [esp+68h] [ebp-10h]
    float attenuatione; // [esp+68h] [ebp-10h]
    bool looping; // [esp+6Eh] [ebp-Ah]
    unsigned int group; // [esp+70h] [ebp-8h]
    const snd_alias_t *alias; // [esp+74h] [ebp-4h]

    alias = voice->alias;
    if (!alias && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2560, 0, "%s", "alias"))
        __debugbreak();
    if (snd_solo_alias_substring->current.integer)
    {
        if (*(_BYTE *)snd_solo_alias_substring->current.integer)
        {
            v2 = strstr(alias->name, snd_solo_alias_substring->current.string);
            if (!v2)
                return 0.0;
        }
    }
    if (snd_mute_alias_substring->current.integer)
    {
        if (*(_BYTE *)snd_mute_alias_substring->current.integer)
        {
            v4 = strstr(alias->name, snd_mute_alias_substring->current.string);
            if (v4)
                return 0.0;
        }
    }
    attenuation = (float)((float)((float)(1.0 - voice->volModSeed) * (float)((float)alias->volMin / 65535.0))
        + (float)((float)((float)alias->volMax / 65535.0) * voice->volModSeed))
        * 1.0;
    if ((LODWORD(attenuation) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2581, 0, "%s", "!IS_NAN(attenuation)"))
    {
        __debugbreak();
    }
    attenuationa = attenuation * voice->volModStart;
    if ((LODWORD(attenuationa) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2584, 0, "%s", "!IS_NAN(attenuation)"))
    {
        __debugbreak();
    }
    group = voice->group;
    for (i = 0; group != -1 && i < 0x64; ++i)
    {
        attenuationa = (float)((float)SND_GetGroupByIndex(group)->attenuationMp / 65535.0) * attenuationa;
        if ((LODWORD(attenuationa) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2591, 0, "%s", "!IS_NAN(attenuation)"))
        {
            __debugbreak();
        }
        group = SND_GetGroupByIndex(group)->parentIndex;
    }
    attenuationb = attenuationa * g_snd.snapshotAttenuation[alias->snapshotGroup];
    if ((LODWORD(attenuationb) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2596, 0, "%s", "!IS_NAN(attenuation)"))
    {
        __debugbreak();
    }
    attenuationc = (float)(voice->fade.value * voice->fade.value) * attenuationb;
    if ((LODWORD(attenuationc) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2599, 0, "%s", "!IS_NAN(attenuation)"))
    {
        __debugbreak();
    }
    looping = (voice->alias->flags & 1) != 0;
    if (SND_GroupCategory((voice->alias->flags & 0x3F0000) >> 16) != SND_CATEGORY_UI || looping)
        attenuationc = (float)(g_snd.volume.value * g_snd.volume.value) * attenuationc;
    if ((LODWORD(attenuationc) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2608, 0, "%s", "!IS_NAN(attenuation)"))
    {
        __debugbreak();
    }
    attenuationd = attenuationc * voice->script_fade.value;
    if ((LODWORD(attenuationd) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2611, 0, "%s", "!IS_NAN(attenuation)"))
    {
        __debugbreak();
    }
    if (SND_GroupCategory((voice->alias->flags & 0x3F0000) >> 16) == SND_CATEGORY_UI
        && (alias->flags & 2) >> 1
        && SND_IsOnSameTeam(voice->closestListenerIndex, voice->sndEnt))
    {
        attenuationd = (float)((float)voice->alias->teamVolMod / 65535.0) * attenuationd;
    }
    if ((LODWORD(attenuationd) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2619, 0, "%s", "!IS_NAN(attenuation)"))
    {
        __debugbreak();
    }
    switch (SND_GroupCategory(voice->group))
    {
    case SND_CATEGORY_SFX:
    case SND_CATEGORY_UI:
        attenuationd = (float)(snd_menu_sfx->current.value * snd_menu_sfx->current.value) * attenuationd;
        break;
    case SND_CATEGORY_MUSIC:
        attenuationd = (float)(snd_menu_music->current.value * snd_menu_music->current.value) * attenuationd;
        break;
    case SND_CATEGORY_VOICE:
        attenuationd = (float)(snd_menu_voice->current.value * snd_menu_voice->current.value) * attenuationd;
        break;
    default:
        break;
    }
    attenuatione = (float)(snd_menu_master->current.value * snd_menu_master->current.value) * attenuationd;
    if ((LODWORD(attenuatione) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2636, 0, "%s", "!IS_NAN(attenuation)"))
    {
        __debugbreak();
    }
    if (attenuatione < 0.0000152879)
        attenuatione = 0.0f;
    if ((LODWORD(attenuatione) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2644, 0, "%s", "!IS_NAN(attenuation)"))
    {
        __debugbreak();
    }
    if ((float)(attenuatione - 1.0) < 0.0)
        v6 = attenuatione;
    else
        v6 = 1.0f;
    if ((float)(0.0 - attenuatione) < 0.0)
        return v6;
    else
        return 0.0f;
}

double __cdecl SND_GetDryLevel(const snd_voice_t *voice)
{
    float v4; // [esp+4h] [ebp-38h]
    float v5; // [esp+8h] [ebp-34h]
    float v6; // [esp+24h] [ebp-18h]
    float attenuationd; // [esp+30h] [ebp-Ch]
    float attenuation; // [esp+30h] [ebp-Ch]
    float attenuationa; // [esp+30h] [ebp-Ch]
    float attenuationb; // [esp+30h] [ebp-Ch]
    float attenuationc; // [esp+30h] [ebp-Ch]
    const snd_alias_t *alias; // [esp+34h] [ebp-8h]
    float change; // [esp+38h] [ebp-4h]

    alias = voice->alias;
    if (!alias && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2653, 0, "%s", "alias"))
        __debugbreak();
    attenuationd = SND_GetBaseLevel(voice);
    attenuation = attenuationd * voice->distanceAttenuation;
    if ((LODWORD(attenuation) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2658, 0, "%s", "!IS_NAN(attenuation)"))
    {
        __debugbreak();
    }
    change = (float)(1.0
        - (float)((float)((float)alias->occlusionLevel / 255.0) * (float)((float)alias->occlusionWetDry / 255.0)))
        + 0.25;
    if ((float)(change - 1.0) < 0.0)
        v6 = (float)(1.0
            - (float)((float)((float)alias->occlusionLevel / 255.0) * (float)((float)alias->occlusionWetDry / 255.0)))
        + 0.25;
    else
        v6 = 1.0f;
    if ((float)(0.0 - change) < 0.0)
        v4 = v6;
    else
        v4 = 0.0f;
    attenuationa = (float)((float)((float)(1.0 - (float)(1.0 - voice->losOcclusion.value)) * 1.0)
        + (float)(v4 * (float)(1.0 - voice->losOcclusion.value)))
        * attenuation;
    if ((LODWORD(attenuationa) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2663, 0, "%s", "!IS_NAN(attenuation)"))
    {
        __debugbreak();
    }
    attenuationb = attenuationa * g_snd.effect->drylevel;
    if ((LODWORD(attenuationb) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2666, 0, "%s", "!IS_NAN(attenuation)"))
    {
        __debugbreak();
    }
    attenuationc = attenuationb * snd_dry_scale->current.value;
    if ((LODWORD(attenuationc) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2669, 0, "%s", "!IS_NAN(attenuation)"))
    {
        __debugbreak();
    }
    if (attenuationc < 0.0000152879)
        attenuationc = 0.0f;
    if ((LODWORD(attenuationc) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2676, 0, "%s", "!IS_NAN(attenuation)"))
    {
        __debugbreak();
    }
    if ((float)(attenuationc - 1.0) < 0.0)
        v5 = attenuationc;
    else
        v5 = 1.0f;
    if ((float)(0.0 - attenuationc) < 0.0)
        return v5;
    else
        return 0.0f;
}

double __cdecl SND_GetWetLevel(const snd_voice_t *voice)
{
    float v2; // xmm0_4
    float v5; // [esp+4h] [ebp-40h]
    float v6; // [esp+8h] [ebp-3Ch]
    float v7; // [esp+Ch] [ebp-38h]
    float baseDistance; // [esp+14h] [ebp-30h]
    float value; // [esp+18h] [ebp-2Ch]
    float v10; // [esp+2Ch] [ebp-18h]
    float attenuationd; // [esp+38h] [ebp-Ch]
    float attenuation; // [esp+38h] [ebp-Ch]
    float attenuationa; // [esp+38h] [ebp-Ch]
    float attenuationb; // [esp+38h] [ebp-Ch]
    float attenuationc; // [esp+38h] [ebp-Ch]
    float dm; // [esp+3Ch] [ebp-8h]
    float change; // [esp+40h] [ebp-4h]

    if (!voice->alias
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2684, 0, "%s", "voice->alias"))
    {
        __debugbreak();
    }
    attenuationd = SND_GetBaseLevel(voice);
    attenuation = attenuationd * voice->reverbAttenuation;
    if ((LODWORD(attenuation) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2689, 0, "%s", "!IS_NAN(attenuation)"))
    {
        __debugbreak();
    }
    change = (float)((float)((float)((float)voice->alias->occlusionLevel / 255.0)
        * (float)((float)voice->alias->occlusionWetDry / 255.0))
        + 1.0)
        - 0.25;
    if ((float)(change - 1.0) < 0.0)
        v10 = (float)((float)((float)((float)voice->alias->occlusionLevel / 255.0)
            * (float)((float)voice->alias->occlusionWetDry / 255.0))
            + 1.0)
        - 0.25;
    else
        v10 = 1.0f;
    if ((float)(0.0 - change) < 0.0)
        v6 = v10;
    else
        v6 = 0.0f;
    attenuationa = (float)((float)((float)(1.0 - voice->losOcclusion.value) * 1.0)
        + (float)(v6 * voice->losOcclusion.value))
        * attenuation;
    if ((LODWORD(attenuationa) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2695, 0, "%s", "!IS_NAN(attenuation)"))
    {
        __debugbreak();
    }
    attenuationb = attenuationa * g_snd.effect->wetlevel;
    if ((LODWORD(attenuationb) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2698, 0, "%s", "!IS_NAN(attenuation)"))
    {
        __debugbreak();
    }
    attenuationc = attenuationb * snd_wet_scale->current.value;
    if ((LODWORD(attenuationc) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2702, 0, "%s", "!IS_NAN(attenuation)"))
    {
        __debugbreak();
    }
    dm = snd_reverb_proximity_distance->current.value;
    if ((voice->alias->flags & 2) >> 1 && dm > 0.0000152879)
    {
        baseDistance = voice->baseDistance;
        if ((float)(baseDistance - dm) < 0.0)
            value = voice->baseDistance;
        else
            value = snd_reverb_proximity_distance->current.value;
        if ((float)(0.0 - baseDistance) < 0.0)
            v5 = value;
        else
            v5 = 0.0f;
        //v2 = __libm_sse2_sin((float)((float)(v5 / dm) * 1.5707964));
        v2 = sin((float)((float)(v5 / dm) * 1.5707964));
        attenuationc = v2 * attenuationc;
    }
    if ((LODWORD(attenuationc) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2711, 0, "%s", "!IS_NAN(attenuation)"))
    {
        __debugbreak();
    }
    if ((float)(attenuationc - 1.0) < 0.0)
        v7 = attenuationc;
    else
        v7 = 1.0f;
    if ((float)(0.0 - attenuationc) < 0.0)
        return v7;
    else
        return 0.0f;
}

double __cdecl SND_GetDistance(const snd_voice_t *channel)
{
    return channel->baseDistance;
}

double __cdecl SND_GetLpfLevel(const snd_voice_t *voice)
{
    float x; // [esp+10h] [ebp-60h]
    float xa; // [esp+10h] [ebp-60h]
    float v5; // [esp+18h] [ebp-58h]
    float v6; // [esp+44h] [ebp-2Ch]
    int ISBIG_DISTANCE; // [esp+4Ch] [ebp-24h]
    float distanceOcclusion; // [esp+50h] [ebp-20h]
    int MIN_DISTANCE; // [esp+54h] [ebp-1Ch]
    int MAX_DISTANCE; // [esp+58h] [ebp-18h]
    int DISTANCE_ATTENUATION; // [esp+5Ch] [ebp-14h]
    float occlusion; // [esp+60h] [ebp-10h]
    float OCCLUSION_ATTENUATION; // [esp+64h] [ebp-Ch]
    float snapshotOcclusion; // [esp+6Ch] [ebp-4h]

    if (!voice && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2725, 0, "%s", "voice"))
        __debugbreak();
    OCCLUSION_ATTENUATION = snd_occlusion_attenuation->current.value;
    v6 = g_snd.snapshotOcclusion[voice->alias->snapshotGroup];
    snapshotOcclusion = (float)((float)(1.0 - v6) * OCCLUSION_ATTENUATION) + (float)(0.0 * v6);
    if (!((voice->alias->flags & 2) >> 1) || (voice->alias->flags & 0x40) >> 6)
        return snapshotOcclusion;
    occlusion = (float)((float)(1.0 - voice->losOcclusion.value) * 0.0)
        + (float)((float)((float)((float)voice->alias->occlusionLevel / 255.0) * OCCLUSION_ATTENUATION)
            * voice->losOcclusion.value);
    if ((voice->alias->flags & 8) >> 3)
    {
        MAX_DISTANCE = snd_dlpf_max_distance->current.integer;
        MIN_DISTANCE = snd_dlpf_min_distance->current.integer;
        DISTANCE_ATTENUATION = snd_dlpf_attenuation->current.integer;
        x = SND_GetDistance(voice);
        distanceOcclusion = I_fmap(
            *(float *)&MIN_DISTANCE,
            *(float *)&MAX_DISTANCE,
            0.0,
            *(float *)&DISTANCE_ATTENUATION,
            x);
        if ((float)(distanceOcclusion - occlusion) < 0.0)
            v5 = distanceOcclusion;
        else
            v5 = occlusion;
        occlusion = v5;
    }
    if ((voice->alias->flags & 0x20) >> 5 && voice->losOcclusion.value < 1.0)
    {
        ISBIG_DISTANCE = snd_isbig_distance->current.integer;
        xa = SND_GetDistance(voice);
        occlusion = I_fmap(0.0, *(float *)&ISBIG_DISTANCE, 0.0, occlusion, xa);
    }
    if ((float)(snapshotOcclusion - occlusion) < 0.0)
        return (float)((float)((float)(1.0 - v6) * OCCLUSION_ATTENUATION) + (float)(0.0 * v6));
    else
        return occlusion;
}

double __cdecl I_fmap(float minx, float maxx, float miny, float maxy, float x)
{
    float v6; // [esp+0h] [ebp-14h]
    float v7; // [esp+10h] [ebp-4h]

    if ( maxx == minx
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 539, 0, "%s", "maxx != minx") )
    {
        __debugbreak();
    }
    if ( (float)((float)(x - minx) - (float)(maxx - minx)) < 0.0 )
        v7 = x - minx;
    else
        v7 = maxx - minx;
    if ( (float)(0.0 - (float)(x - minx)) < 0.0 )
        v6 = v7;
    else
        v6 = 0.0f;
    return maxy * (float)(v6 / (float)(maxx - minx)) + (1.0 - (float)(v6 / (float)(maxx - minx))) * miny;
}

unsigned int __cdecl SND_GetVoiceLength(unsigned int voiceIndex)
{
    if (voiceIndex >= SND_MAX_VOICES
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
            2777,
            0,
            "voiceIndex doesn't index SND_MAX_VOICES\n\t%i not in [0, %i)",
            voiceIndex,
            74))
    {
        __debugbreak();
    }
    return g_snd.voice[voiceIndex].soundFileInfo.totalMsec;
}

void __cdecl SND_FixupStereoPan(snd_speaker_map *pan)
{
    float volume; // [esp+0h] [ebp-4h]
    float volumea; // [esp+0h] [ebp-4h]
    float volumeb; // [esp+0h] [ebp-4h]
    float volumec; // [esp+0h] [ebp-4h]
    float volumed; // [esp+0h] [ebp-4h]
    float volumee; // [esp+0h] [ebp-4h]

    if (pan->input_channel_count != 2
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
            2783,
            0,
            "%s",
            "pan->input_channel_count == 2"))
    {
        __debugbreak();
    }
    if (pan->output_channel_count >= 2)
    {
        volume = Snd_SpeakerMapGetVolume(pan, 0, 0);
        Snd_SpeakerMapSetVolume(pan, 0, 0, volume);
        volumea = Snd_SpeakerMapGetVolume(pan, 0, 1);
        Snd_SpeakerMapSetVolume(pan, 1, 1, volumea);
        Snd_SpeakerMapSetVolume(pan, 1, 0, 0.0);
        Snd_SpeakerMapSetVolume(pan, 0, 1, 0.0);
    }
    if (pan->output_channel_count >= 6)
    {
        volumeb = Snd_SpeakerMapGetVolume(pan, 0, 4);
        Snd_SpeakerMapSetVolume(pan, 1, 4, volumeb);
        volumec = Snd_SpeakerMapGetVolume(pan, 0, 5);
        Snd_SpeakerMapSetVolume(pan, 0, 5, volumec);
        Snd_SpeakerMapSetVolume(pan, 0, 4, 0.0);
        Snd_SpeakerMapSetVolume(pan, 1, 5, 0.0);
    }
    if (pan->output_channel_count == 8)
    {
        volumed = Snd_SpeakerMapGetVolume(pan, 0, 7);
        Snd_SpeakerMapSetVolume(pan, 1, 7, volumed);
        volumee = Snd_SpeakerMapGetVolume(pan, 0, 6);
        Snd_SpeakerMapSetVolume(pan, 0, 6, volumee);
        Snd_SpeakerMapSetVolume(pan, 1, 6, 0.0);
        Snd_SpeakerMapSetVolume(pan, 0, 7, 0.0);
    }
}

void __cdecl SND_UpdatePanFilter(float dt, snd_voice_t *voice)
{
    float v3; // [esp+4h] [ebp-1Ch]
    float v4; // [esp+8h] [ebp-18h]
    float v5; // [esp+Ch] [ebp-14h]
    unsigned int i; // [esp+18h] [ebp-8h]
    unsigned int count; // [esp+1Ch] [ebp-4h]

    voice->pan.input_channel_count = voice->panGoal.input_channel_count;
    voice->pan.output_channel_count = voice->panGoal.output_channel_count;
    count = voice->pan.output_channel_count * voice->pan.input_channel_count;
    for (i = 0; i < count; ++i)
    {
        if ((LODWORD(voice->panGoal.volumes[i]) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                2893,
                0,
                "%s",
                "!IS_NAN(voice->panGoal.volumes[i])"))
        {
            __debugbreak();
        }
        if ((LODWORD(voice->pan.volumes[i]) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                2894,
                0,
                "%s",
                "!IS_NAN(voice->pan.volumes[i])"))
        {
            __debugbreak();
        }
        v4 = snd_pan_filter->current.value * dt;
        if ((float)(v4 - 1.0) < 0.0)
            v5 = snd_pan_filter->current.value * dt;
        else
            v5 = 1.0f;
        if ((float)(0.0 - v4) < 0.0)
            v3 = v5;
        else
            v3 = 0.0f;
        voice->pan.volumes[i] = (float)((float)(1.0 - v3) * voice->panGoal.volumes[i]) + (float)(voice->pan.volumes[i] * v3);
        if ((LODWORD(voice->pan.volumes[i]) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                2899,
                0,
                "%s",
                "!IS_NAN(voice->pan.volumes[i])"))
        {
            __debugbreak();
        }
    }
}

void __cdecl SND_UpdateVoice(snd_voice_t *voice, float dt)
{
    unsigned int v2; // eax
    unsigned int v3; // eax
    const snd_curve *CurveByIndex; // eax
    const snd_curve *v5; // eax
    const snd_pan *PanByIndex; // eax
    int v7; // eax
    const snd_speaker_config *SpeakerConfig; // eax
    unsigned int MixChannelCount; // eax
    double Volume; // st7
    double v11; // st7
    unsigned int v12; // eax
    unsigned int v13; // eax
    float *v14; // [esp-4h] [ebp-374h]
    snd_listener *minx; // [esp+0h] [ebp-370h]
    const snd_curve *t; // [esp+4h] [ebp-36Ch]
    const snd_curve *ta; // [esp+4h] [ebp-36Ch]
    const snd_pan *t_4; // [esp+8h] [ebp-368h]
    const snd_pan *t_4a; // [esp+8h] [ebp-368h]
    const snd_pan *t_4b; // [esp+8h] [ebp-368h]
    const snd_pan *t_4c; // [esp+8h] [ebp-368h]
    const snd_pan *t_4d; // [esp+8h] [ebp-368h]
    float g; // [esp+Ch] [ebp-364h]
    float ga; // [esp+Ch] [ebp-364h]
    float gb; // [esp+Ch] [ebp-364h]
    float gc; // [esp+Ch] [ebp-364h]
    float gd; // [esp+Ch] [ebp-364h]
    float g_4; // [esp+10h] [ebp-360h]
    float g_4a; // [esp+10h] [ebp-360h]
    float g_4b; // [esp+10h] [ebp-360h]
    float g_4c; // [esp+10h] [ebp-360h]
    float v32; // [esp+14h] [ebp-35Ch]
    float v33; // [esp+18h] [ebp-358h]
    float value; // [esp+1Ch] [ebp-354h]
    bool v35; // [esp+20h] [ebp-350h]
    bool v36; // [esp+24h] [ebp-34Ch]
    int v37; // [esp+28h] [ebp-348h]
    float v38; // [esp+44h] [ebp-32Ch]
    float v39; // [esp+50h] [ebp-320h]
    float v40; // [esp+7Ch] [ebp-2F4h]
    float v41; // [esp+80h] [ebp-2F0h]
    float v42; // [esp+ACh] [ebp-2C4h]
    unsigned int wetMinCurve; // [esp+B0h] [ebp-2C0h]
    unsigned int wetCurve; // [esp+B4h] [ebp-2BCh]
    float v45; // [esp+B8h] [ebp-2B8h]
    float distReverbMax; // [esp+BCh] [ebp-2B4h]
    unsigned int dryMinCurve; // [esp+C0h] [ebp-2B0h]
    unsigned int dryCurve; // [esp+C4h] [ebp-2ACh]
    float distMin; // [esp+C8h] [ebp-2A8h]
    float distMax; // [esp+CCh] [ebp-2A4h]
    snd_listener *v51; // [esp+1ACh] [ebp-1C4h]
    float baseR; // [esp+268h] [ebp-108h]
    float baseL; // [esp+26Ch] [ebp-104h]
    int j; // [esp+270h] [ebp-100h]
    float v55; // [esp+274h] [ebp-FCh] BYREF
    float v56; // [esp+278h] [ebp-F8h]
    float v57; // [esp+27Ch] [ebp-F4h] BYREF
    snd_speaker_map map; // [esp+280h] [ebp-F0h] BYREF
    float atten; // [esp+2CCh] [ebp-A4h]
    int out; // [esp+2D0h] [ebp-A0h]
    float a; // [esp+2D4h] [ebp-9Ch] BYREF
    float ndist; // [esp+2D8h] [ebp-98h]
    float neard; // [esp+2DCh] [ebp-94h]
    float fard; // [esp+2E0h] [ebp-90h]
    float b; // [esp+2E4h] [ebp-8Ch] BYREF
    snd_speaker_map futzPan; // [esp+2E8h] [ebp-88h] BYREF
    bool is3d; // [esp+337h] [ebp-39h]
    unsigned int inputChannelCount; // [esp+338h] [ebp-38h]
    bool isSpatialized; // [esp+33Dh] [ebp-33h]
    bool isDefault; // [esp+33Eh] [ebp-32h]
    bool is2dFalloff; // [esp+33Fh] [ebp-31h]
    float toListner[3]; // [esp+340h] [ebp-30h] BYREF
    float base; // [esp+34Ch] [ebp-24h]
    int i; // [esp+350h] [ebp-20h]
    float blendL; // [esp+354h] [ebp-1Ch] BYREF
    float blendR; // [esp+358h] [ebp-18h] BYREF
    unsigned int src_channel_count; // [esp+35Ch] [ebp-14h]
    float GlobalPriorityVolume; // [esp+360h] [ebp-10h]
    int li; // [esp+364h] [ebp-Ch]
    float thisVolume; // [esp+368h] [ebp-8h]
    const snd_alias_t *alias; // [esp+36Ch] [ebp-4h]

    alias = voice->alias;
    if ( !alias && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2907, 0, "%s", "alias") )
        __debugbreak();
    if ( dt <= 0.0
        || voice->startDelay > 0
        || voice->soundFileInfo.loadingState == SFLS_LOADING
        || (SND_FaderUpdate(&voice->fade, dt), voice->fade.value >= 0.0000152879)
        || voice->fade.goal >= 0.0000152879 )
    {
        if ( !voice->totalMsec && (voice->alias->flags & 1) == 0 )
        {
            voice->totalMsec = SND_GetVoiceLength(voice - g_snd.voice);
            if ( voice->playback )
                voice->playback->lengthMs = voice->totalMsec;
        }
        if ( voice->playback )
        {
            if ( voice->playback->id != voice->playbackId
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                            2933,
                            0,
                            "%s",
                            "voice->playback->id == voice->playbackId") )
            {
                __debugbreak();
            }
            voice->playback->playedMs = SNDL_GetPlaybackTime(voice->playbackId);
        }
        if ( voice->startDelay < 0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 2937, 0, "%s", "voice->startDelay >= 0") )
        {
            __debugbreak();
        }
        if ( voice->startDelay && !g_snd.pausetime )
        {
            if ( voice->startDelay - (int)(float)(dt * 1000.0) > 0 )
                v37 = voice->startDelay - (int)(float)(dt * 1000.0);
            else
                v37 = 0;
            voice->startDelay = v37;
        }
        SND_FaderUpdate(&voice->script_fade, dt);
        SND_FaderUpdate(&voice->script_pitch, dt);
        voice->baseDistance = -1.0f;
        voice->cylinderAttenuation = 1.0f;
        if ( (alias->flags & 2) >> 1 )
        {
            if ( voice->entity_update != SND_ENTITY_UPDATE_NEVER )
            {
                if ( ((LODWORD(voice->position[0]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(voice->position[1]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(voice->position[2]) & 0x7F800000) == 0x7F800000)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                                3017,
                                0,
                                "%s",
                                "!IS_NAN((voice->position)[0]) && !IS_NAN((voice->position)[1]) && !IS_NAN((voice->position)[2])") )
                {
                    __debugbreak();
                }
                if ( ((LODWORD(voice->velocity[0]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(voice->velocity[1]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(voice->velocity[2]) & 0x7F800000) == 0x7F800000)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                                3018,
                                0,
                                "%s",
                                "!IS_NAN((voice->velocity)[0]) && !IS_NAN((voice->velocity)[1]) && !IS_NAN((voice->velocity)[2])") )
                {
                    __debugbreak();
                }
                if ( ((LODWORD(g_snd.listeners[voice->closestListenerIndex].orient.origin[0]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(g_snd.listeners[voice->closestListenerIndex].orient.origin[1]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(g_snd.listeners[voice->closestListenerIndex].orient.origin[2]) & 0x7F800000) == 0x7F800000)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                                3019,
                                0,
                                "%s",
                                "!IS_NAN((g_snd.listeners[voice->closestListenerIndex].orient.origin)[0]) && !IS_NAN((g_snd.listeners[voi"
                                "ce->closestListenerIndex].orient.origin)[1]) && !IS_NAN((g_snd.listeners[voice->closestListenerIndex].or"
                                "ient.origin)[2])") )
                {
                    __debugbreak();
                }
                v51 = &g_snd.listeners[voice->closestListenerIndex];
                toListner[0] = v51->orient.origin[0] - voice->position[0];
                toListner[1] = v51->orient.origin[1] - voice->position[1];
                toListner[2] = v51->orient.origin[2] - voice->position[2];
                if ( (float)((float)((float)(toListner[0] * toListner[0]) + (float)(toListner[1] * toListner[1]))
                                     + (float)(toListner[2] * toListner[2])) > 0.0000152879 )
                {
                    if ( ((LODWORD(toListner[0]) & 0x7F800000) == 0x7F800000
                         || (LODWORD(toListner[1]) & 0x7F800000) == 0x7F800000
                         || (LODWORD(toListner[2]) & 0x7F800000) == 0x7F800000)
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                                    3024,
                                    0,
                                    "%s",
                                    "!IS_NAN((toListner)[0]) && !IS_NAN((toListner)[1]) && !IS_NAN((toListner)[2])") )
                    {
                        __debugbreak();
                    }
                    Vec3Normalize(toListner);
                    if ( (float)((float)((float)(voice->fluxVelocity[0] * voice->fluxVelocity[0])
                                                         + (float)(voice->fluxVelocity[1] * voice->fluxVelocity[1]))
                                         + (float)(voice->fluxVelocity[2] * voice->fluxVelocity[2])) <= 0.0000152879 )
                        SND_FaderSetGoal(
                            &voice->doppler,
                            (float)((float)((float)((float)(voice->velocity[0] * toListner[0])
                                                                        + (float)(voice->velocity[1] * toListner[1]))
                                                        + (float)(voice->velocity[2] * toListner[2]))
                                        / 13397.244)
                        * snd_dopplerScale->current.value);
                    else
                        SND_FaderSetGoal(
                            &voice->doppler,
                            (float)((float)((float)((float)(voice->fluxVelocity[0] * toListner[0])
                                                                        + (float)(voice->fluxVelocity[1] * toListner[1]))
                                                        + (float)(voice->fluxVelocity[2] * toListner[2]))
                                        / 13397.244)
                        * snd_dopplerScale->current.value);
                }
                if ( (LODWORD(voice->doppler.goal) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                                3035,
                                0,
                                "%s",
                                "!IS_NAN(SND_FaderGetGoal(&voice->doppler))") )
                {
                    __debugbreak();
                }
                if ( (LODWORD(voice->doppler.value) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                                3036,
                                0,
                                "%s",
                                "!IS_NAN(SND_FaderGetValue(&voice->doppler))") )
                {
                    __debugbreak();
                }
                SND_FaderUpdate(&voice->doppler, dt);
                if ( (LODWORD(voice->doppler.goal) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                                3038,
                                0,
                                "%s",
                                "!IS_NAN(SND_FaderGetGoal(&voice->doppler))") )
                {
                    __debugbreak();
                }
                if ( (LODWORD(voice->doppler.value) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                                3039,
                                0,
                                "%s",
                                "!IS_NAN(SND_FaderGetValue(&voice->doppler))") )
                {
                    __debugbreak();
                }
            }
            if ( !voice->paused )
            {
                voice->position[0] = (float)(dt * voice->fluxVelocity[0]) + voice->position[0];
                voice->position[1] = (float)(dt * voice->fluxVelocity[1]) + voice->position[1];
                voice->position[2] = (float)(dt * voice->fluxVelocity[2]) + voice->position[2];
            }
            if ( (float)((float)((float)(voice->direction[0] * voice->direction[0])
                                                 + (float)(voice->direction[1] * voice->direction[1]))
                                 + (float)(voice->direction[2] * voice->direction[2])) < 0.0000152879 )
            {
                voice->direction[0] = voice->orientation[0][0];
                voice->direction[1] = voice->orientation[0][1];
                voice->direction[2] = voice->orientation[0][2];
            }
            if ( ((LODWORD(voice->direction[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(voice->direction[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(voice->direction[2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                            3052,
                            0,
                            "%s",
                            "!IS_NAN((voice->direction)[0]) && !IS_NAN((voice->direction)[1]) && !IS_NAN((voice->direction)[2])") )
            {
                __debugbreak();
            }
            li = SND_GetListenerIndexNearestToOrigin(voice->position);
            voice->closestListenerIndex = li;

            nanassertvec3(voice->position);
            nanassertvec3(g_snd.listeners[li].orient.origin);
            
            voice->baseDistance = Vec3Distance(g_snd.listeners[li].orient.origin, voice->position);
            distMax = (float)alias->distMax;
            distMin = (float)alias->distMin;
            dryCurve = alias->dryCurve;
            dryMinCurve = alias->dryMinCurve;
            g_4 = SND_GetDistance(voice);
            t = SND_GetCurveByIndex(dryCurve);
            CurveByIndex = SND_GetCurveByIndex(dryMinCurve);
            voice->distanceAttenuation = Snd_DistanceCurveEval(CurveByIndex, t, distMin, distMax, g_4);
            distReverbMax = (float)alias->distReverbMax;
            v45 = (float)alias->distMin;
            wetCurve = alias->wetCurve;
            wetMinCurve = alias->wetMinCurve;
            v42 = (float)alias->reverbSend / 65535.0;
            g_4a = SND_GetDistance(voice);
            ta = SND_GetCurveByIndex(wetCurve);
            v5 = SND_GetCurveByIndex(wetMinCurve);
            voice->reverbAttenuation = Snd_DistanceCurveEval(v5, ta, v45, distReverbMax, g_4a) * v42;
            SND_FaderSetRate(&voice->losOcclusion, snd_occlusion_rate->current.value);
            SND_FaderUpdate(&voice->losOcclusion, dt);
            thisVolume = Snd_GetGlobalPriorityVolume(voice->alias, voice->position);
            voice->globalPriority = Snd_GetGlobalPriority(voice->alias, thisVolume);
            if ( voice->soundFileInfo.loadingState == SFLS_LOADED )
            {
                inputChannelCount = voice->soundFileInfo.srcChannelCount;
                if ( inputChannelCount != 1
                    && inputChannelCount != 2
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                                3100,
                                0,
                                "%s\n\t(src_channel_count) = %i",
                                "(src_channel_count == 1 || src_channel_count == 2)",
                                inputChannelCount) )
                {
                    __debugbreak();
                }
                PanByIndex = SND_GetPanByIndex(voice->alias->pan);
                v7 = I_stricmp(PanByIndex->name, "default");
                isDefault = v7 == 0;
                isSpatialized = (alias->flags & 2) >> 1 != 0;
                v36 = v7 && isSpatialized;
                is2dFalloff = v36;
                v35 = isSpatialized && !is2dFalloff;
                is3d = v35;
                Snd_SpeakerMapZero(&voice->panGoal);
                if ( v35 )
                {
                    voice->panGoal.input_channel_count = 1;
                    voice->panGoal.output_channel_count = Snd_GetMixChannelCount(snd_speakerConfiguration->current.unsignedInt);
                    v41 = (float)voice->alias->centerSend / 65535.0;
                    gb = SND_GetOmni(voice);
                    minx = &g_snd.listeners[li];
                    v14 = g_snd.listeners[li].orient.axis[0];
                    SpeakerConfig = Snd_GetSpeakerConfig(snd_speakerConfiguration->current.unsignedInt);
                    Snd_Pan3d(SpeakerConfig, voice->position, v14, minx->orient.origin, v41, 0.0, gb, &voice->panGoal);
                    voice->panGoal.input_channel_count = inputChannelCount;
                    if ( (alias->flags & 0x40) >> 6 && snd_dsp_futz->current.enabled )
                    {
                        v40 = (float)alias->centerSend / 65535.0;
                        t_4b = SND_GetPanByIndex(voice->alias->pan);
                        MixChannelCount = Snd_GetMixChannelCount(snd_speakerConfiguration->current.unsignedInt);
                        SND_PanToSpeakermap(1u, MixChannelCount, t_4b, v40, &futzPan);
                        neard = snd_futz_min->current.value;
                        fard = (float)alias->distMin;
                        if ( neard >= fard )
                        {
                            Com_PrintError(9, "Alias %s had invalid dist min %f. needs to be > than %f\n", alias->name, fard, neard);
                            fard = neard + 1.0;
                        }
                        g_4b = SND_GetDistance(voice);
                        ndist = I_fmap(neard, fard, 0.0, 1.0, g_4b);
                        if ( (float)(snd_futz_force->current.value - ndist) < 0.0 )
                            value = ndist;
                        else
                            value = snd_futz_force->current.value;
                        ndist = value;
                        if ( (LODWORD(value) & 0x7F800000) == 0x7F800000
                            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp", 3146, 0, "%s", "!IS_NAN(ndist)") )
                        {
                            __debugbreak();
                        }
                        SND_EqualPowerFadeCoefs(ndist, &a, &b);
                        if ( (LODWORD(a) & 0x7F800000) == 0x7F800000
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                                        3149,
                                        0,
                                        "%s",
                                        "!IS_NAN(blendL)") )
                        {
                            __debugbreak();
                        }
                        if ( (LODWORD(b) & 0x7F800000) == 0x7F800000
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                                        3150,
                                        0,
                                        "%s",
                                        "!IS_NAN(blendR)") )
                        {
                            __debugbreak();
                        }
                        for ( out = 0; out < voice->panGoal.output_channel_count; ++out )
                        {
                            Volume = Snd_SpeakerMapGetVolume(&voice->panGoal, 0, out);
                            v33 = Volume * b;
                            v11 = Snd_SpeakerMapGetVolume(&futzPan, 0, out);
                            atten = v11 * a + v33;
                            if ( (float)(atten - 1.0) < 0.0 )
                                v39 = atten;
                            else
                                v39 = 1.0f;
                            if ( (float)(0.0 - atten) < 0.0 )
                                v32 = v39;
                            else
                                v32 = 0.0f;
                            atten = v32;
                            Snd_SpeakerMapSetVolume(&voice->panGoal, 0, out, v32);
                        }
                        if ( ndist > 0.0 )
                            voice->distanceAttenuation = 1.0f;
                        voice->reverbAttenuation = voice->reverbAttenuation * a;
                        voice->futzBlend = b;
                    }
                    else if ( (alias->flags & 0x40) >> 6 && inputChannelCount == 2 )
                    {
                        gc = (float)alias->centerSend / 65535.0;
                        t_4c = SND_GetPanByIndex(voice->alias->pan);
                        v12 = Snd_GetMixChannelCount(snd_speakerConfiguration->current.unsignedInt);
                        SND_PanToSpeakermap(1u, v12, t_4c, gc, &map);
                        v38 = (float)alias->distMin;
                        g_4c = SND_GetDistance(voice);
                        v56 = I_fmap(snd_futz_min->current.value, v38, 0.0, 1.0, g_4c);
                        SND_EqualPowerFadeCoefs(v56, &v55, &v57);
                        for ( j = 0; j < voice->panGoal.output_channel_count; ++j )
                        {
                            baseL = Snd_SpeakerMapGetVolume(&voice->panGoal, 0, j);
                            baseR = Snd_SpeakerMapGetVolume(&map, 0, j);
                            Snd_SpeakerMapSetVolume(&voice->panGoal, 0, j, baseL * v55);
                            Snd_SpeakerMapSetVolume(&voice->panGoal, 1, j, baseR * v57);
                        }
                        voice->distanceAttenuation = 1.0f;
                        voice->reverbAttenuation = voice->reverbAttenuation * v55;
                    }
                    else if ( inputChannelCount == 2 && snd_stereo_3d->current.enabled )
                    {
                        SND_FixupStereoPan(&voice->panGoal);
                    }
                }
                else
                {
                    if ( isSpatialized
                        && !is2dFalloff
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                                    3196,
                                    0,
                                    "%s",
                                    "!isSpatialized || is2dFalloff") )
                    {
                        __debugbreak();
                    }
                    gd = (float)alias->centerSend / 65535.0;
                    t_4d = SND_GetPanByIndex(voice->alias->pan);
                    v13 = Snd_GetMixChannelCount(snd_speakerConfiguration->current.unsignedInt);
                    SND_PanToSpeakermap(inputChannelCount, v13, t_4d, gd, &voice->panGoal);
                }
                voice->dryLevel = SND_GetDryLevel(voice);
                voice->wetLevel = SND_GetWetLevel(voice);
                SND_UpdatePanFilter(dt, voice);
            }
        }
        else
        {
            voice->distanceAttenuation = 1.0f;
            voice->reverbAttenuation = (float)alias->reverbSend / 65535.0;
            voice->baseDistance = 0.0f;
            SND_FaderSetGoal(&voice->losOcclusion, 0.0);
            voice->losOcclusion.value = voice->losOcclusion.goal;
            voice->closestListenerIndex = 0;
            if ( voice->soundFileInfo.loadingState == SFLS_LOADED )
            {
                src_channel_count = voice->soundFileInfo.srcChannelCount;
                if ( src_channel_count != 1
                    && src_channel_count != 2
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd.cpp",
                                2963,
                                0,
                                "%s\n\t(src_channel_count) = %i",
                                "(src_channel_count == 1 || src_channel_count == 2)",
                                src_channel_count) )
                {
                    __debugbreak();
                }
                if ( (alias->flags & 0x40) >> 6 && src_channel_count == 2 )
                {
                    g = (float)voice->alias->centerSend / 65535.0;
                    t_4 = SND_GetPanByIndex(voice->alias->pan);
                    v2 = Snd_GetMixChannelCount(snd_speakerConfiguration->current.unsignedInt);
                    SND_PanToSpeakermap(1u, v2, t_4, g, &voice->panGoal);
                    voice->panGoal.input_channel_count = 2;
                    SND_EqualPowerFadeCoefs(snd_futz->current.value, &blendL, &blendR);
                    for ( i = 0; i < voice->panGoal.output_channel_count; ++i )
                    {
                        base = Snd_SpeakerMapGetVolume(&voice->panGoal, 0, i);
                        Snd_SpeakerMapSetVolume(&voice->panGoal, 0, i, base * blendL);
                        Snd_SpeakerMapSetVolume(&voice->panGoal, 1, i, base * blendR);
                    }
                }
                else
                {
                    if ( (alias->flags & 0x40) >> 6 )
                        Com_PrintError(9, "ERROR: alias %s has mono asset but is marked as futz\n", alias->name);
                    ga = (float)voice->alias->centerSend / 65535.0;
                    t_4a = SND_GetPanByIndex(voice->alias->pan);
                    v3 = Snd_GetMixChannelCount(snd_speakerConfiguration->current.unsignedInt);
                    SND_PanToSpeakermap(src_channel_count, v3, t_4a, ga, &voice->panGoal);
                }
            }
            voice->dryLevel = SND_GetDryLevel(voice);
            voice->wetLevel = SND_GetWetLevel(voice);
            GlobalPriorityVolume = Snd_GetGlobalPriorityVolume(voice->alias, voice->position);
            voice->globalPriority = Snd_GetGlobalPriority(voice->alias, GlobalPriorityVolume);
            SND_UpdatePanFilter(dt, voice);
        }
    }
    else
    {
        SND_StopVoice(voice - g_snd.voice);
    }
}

double __cdecl Snd_DistanceCurveEval(
                const snd_curve *minCurve,
                const snd_curve *curve,
                float minDist,
                float maxDist,
                float dist)
{
    if ( minDist <= dist )
        return Snd_CurveEvalOverRange(curve, dist, minDist, maxDist);
    else
        return Snd_CurveEvalOverRange(minCurve, dist, 0.0, minDist);
}

double __cdecl SND_GetOmni(const snd_voice_t *voice)
{
    float envelopMin; // [esp+8h] [ebp-1Ch]
    float farPercentage; // [esp+10h] [ebp-14h]
    float dist; // [esp+14h] [ebp-10h]
    float dista; // [esp+14h] [ebp-10h]
    float nearPercentage; // [esp+18h] [ebp-Ch]
    const snd_alias_t *alias; // [esp+1Ch] [ebp-8h]
    float globalOmni; // [esp+20h] [ebp-4h]

    alias = voice->alias;
    globalOmni = snd_omnidirectionalPercentage->current.value;
    iassert(alias);
    if ( globalOmni >= fabs((float)alias->envelopPercentage / 65535.0) )
        return globalOmni;
    if ( (float)((float)alias->envelopPercentage / 65535.0) >= 0.0 )
    {
        farPercentage = globalOmni;
        nearPercentage = (float)alias->envelopPercentage / 65535.0;
    }
    else
    {
        //LODWORD(farPercentage) = COERCE_UNSIGNED_INT((float)alias->envelopPercentage / 65535.0) ^ _mask__NegFloat_;
        farPercentage = -((float)alias->envelopPercentage / 65535.0);
        nearPercentage = globalOmni;
    }
    envelopMin = (float)alias->envelopMin;
    dist = SND_GetDistance(voice) - envelopMin;
    if ( dist <= 0.0 )
        return nearPercentage;
    if ( (float)alias->envelopMax == (float)alias->envelopMin )
        return farPercentage;
    dista = dist / (float)((float)alias->envelopMax - (float)alias->envelopMin);
    if ( dista < 1.0 )
        return farPercentage * dista + (1.0 - dista) * nearPercentage;
    else
        return farPercentage;
}

snd_voice_t *__cdecl SND_GetPlaybackVoice(int playbackId)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    if (playbackId == -1)
        return 0;
    for (i = 0; i < 74; ++i)
    {
        if (g_snd.voice[i].playbackId == playbackId)
            return &g_snd.voice[i];
    }
    return 0;
}

bool __cdecl SND_IsStream(unsigned int index)
{
    return index <= 9;
}

