#include "snd_occlusion.h"
#include <universal/profile.h>
#include <qcommon/cm_trace.h>
#include <cgame/cg_world.h>
#include "snd.h"
#include "snd_dvar.h"
#include "snd_utils.h"
#include <win32/win_common.h>
#include <qcommon/threads.h>
#include <win32/win_net.h>

double __cdecl SND_LosOcclusionTrace(bool fancy, int *cache, const float *listener, const float *playback)
{
    float v5; // [esp+18h] [ebp-DCh]
    float v6; // [esp+1Ch] [ebp-D8h]
    float v7; // [esp+38h] [ebp-BCh]
    float v8; // [esp+3Ch] [ebp-B8h]
    unsigned int i; // [esp+5Ch] [ebp-98h]
    float side[3]; // [esp+60h] [ebp-94h] BYREF
    float occlusion_factor; // [esp+6Ch] [ebp-88h]
    int playback_rays; // [esp+70h] [ebp-84h]
    float up_listen_ray_step; // [esp+74h] [ebp-80h]
    trace_t result; // [esp+78h] [ebp-7Ch] BYREF
    int listen_rays; // [esp+B4h] [ebp-40h]
    unsigned int valid[3]; // [esp+B8h] [ebp-3Ch] BYREF
    float up_playback_ray_step; // [esp+C4h] [ebp-30h]
    float up[3]; // [esp+C8h] [ebp-2Ch] BYREF
    float side_playback_ray_step; // [esp+D4h] [ebp-20h]
    float to[3]; // [esp+D8h] [ebp-1Ch] BYREF
    unsigned int hits[3]; // [esp+E4h] [ebp-10h] BYREF
    float side_listen_ray_step; // [esp+F0h] [ebp-4h]

    occlusion_factor = 1.0f;
    memset(valid, 0, sizeof(valid));
    memset(hits, 0, sizeof(hits));
    listen_rays = 2;
    playback_rays = 2;
    if ( fancy )
    {
        listen_rays = 2;
        playback_rays = 4;
    }
    up_listen_ray_step = 20.0 / (float)listen_rays;
    up_playback_ray_step = 40.0 / (float)playback_rays;
    side_listen_ray_step = 30.0 / (float)listen_rays;
    side_playback_ray_step = 50.0 / (float)playback_rays;

    PROF_SCOPED("Occlusion Trace");

    if ( CG_SightTracePoint(cache, listener, playback, 49, &result) )
    {
        up[0] = 0.0f;
        up[1] = 0.0f;
        up[2] = 1.0f;
        to[0] = *listener - *playback;
        to[1] = listener[1] - playback[1];
        to[2] = listener[2] - playback[2];
        if ( Vec3Normalize(to) >= SND_EPSILON )
        {
            side[0] = to[1];
            side[1] = -to[0];
            side[2] = 0.0f;
            Snd_LosOcclusionMultiTrace(
                cache,
                listener,
                playback,
                up,
                up,
                up_listen_ray_step,
                up_playback_ray_step,
                listen_rays,
                playback_rays,
                valid,
                hits);
            Snd_LosOcclusionMultiTrace(
                cache,
                listener,
                playback,
                side,
                side,
                side_listen_ray_step,
                side_playback_ray_step,
                listen_rays,
                playback_rays,
                &valid[1],
                &hits[1]);
            Snd_LosOcclusionMultiTrace(
                cache,
                listener,
                playback,
                side,
                side,
                //COERCE_FLOAT(LODWORD(side_listen_ray_step) ^ _mask__NegFloat_),
                -side_listen_ray_step,
                //COERCE_FLOAT(LODWORD(side_playback_ray_step) ^ _mask__NegFloat_),
                -side_playback_ray_step,
                listen_rays,
                playback_rays,
                &valid[2],
                &hits[2]);
            for ( i = 0; i < 3; ++i )
            {
                if ( valid[i] )
                {
                    v8 = (double)hits[i] / (double)valid[i];
                    if ( (float)(0.2 - v8) < 0.0 )
                        v6 = (double)hits[i] / (double)valid[i];
                    else
                        v6 = 0.2f;
                    v7 = v6;
                }
                else
                {
                    v7 = 1.0f;
                }
                if ( (float)(v7 - occlusion_factor) < 0.0 )
                    v5 = v7;
                else
                    v5 = occlusion_factor;
                occlusion_factor = v5;
            }
            return occlusion_factor;
        }
        else
        {
            return 0.0;
        }
    }
    else
    {
        return 0.0;
    }
}

void __cdecl Snd_LosOcclusionMultiTrace(
                int *global_cache,
                const float *listener,
                const float *playback,
                const float *dlistener,
                const float *dplayback,
                float listen_step,
                float playback_step,
                unsigned int listen_rays,
                unsigned int playback_rays,
                unsigned int *valid,
                unsigned int *hits)
{
    float scale; // [esp+0h] [ebp-78h]
    float scalea; // [esp+0h] [ebp-78h]
    unsigned int j; // [esp+18h] [ebp-60h]
    unsigned int i; // [esp+1Ch] [ebp-5Ch]
    trace_t result; // [esp+20h] [ebp-58h] BYREF
    int cache; // [esp+5Ch] [ebp-1Ch] BYREF
    float dst[3]; // [esp+60h] [ebp-18h] BYREF
    float dp[3]; // [esp+6Ch] [ebp-Ch] BYREF

    cache = 0;
    for ( i = 0; i < listen_rays; ++i )
    {
        scale = (double)(i + 1) * listen_step;
        Vec3ScaleAdd(listener, dlistener, scale, dst);
        for ( j = 0; j < playback_rays; ++j )
        {
            if ( !cache )
                cache = *global_cache;
            scalea = (double)(j + 1) * playback_step;
            Vec3ScaleAdd(playback, dplayback, scalea, dp);
            if ( CG_SightTracePoint(&cache, dst, dp, 49, &result) && !result.startsolid )
                ++*hits;
            if ( !result.startsolid )
                ++*valid;
            if ( cache )
                *global_cache = cache;
        }
    }
}

void __cdecl Vec3ScaleAdd(const float *base, const float *dir, float scale, float *dst)
{
    *dst = *dir;
    dst[1] = dir[1];
    dst[2] = dir[2];
    *dst = scale * *dst;
    dst[1] = scale * dst[1];
    dst[2] = scale * dst[2];
    *dst = *base + *dst;
    dst[1] = base[1] + dst[1];
    dst[2] = base[2] + dst[2];
}

// local variable allocation has failed, the output may be wrong!
//void    SND_TraceProximity(
//                float *front,
//                float *origin,
//                float *resultDistance,
//                float *resultReflection)
//{
//    double v5; // xmm0_8
//    double v6; // xmm0_8
//    long double v7; // [esp-1Ch] [ebp-BCh]
//    long double v8; // [esp-1Ch] [ebp-BCh]
//    long double v9; // [esp-1Ch] [ebp-BCh]
//    long double v10; // [esp-1Ch] [ebp-BCh]
//    long double v11; // [esp-1Ch] [ebp-BCh]
//    long double v12; // [esp-14h] [ebp-B4h]
//    float x; // [esp+0h] [ebp-A0h] BYREF
//    float v14; // [esp+4h] [ebp-9Ch]
//    int v15; // [esp+8h] [ebp-98h]
//    __int128 end; // [esp+Ch] [ebp-94h] OVERLAPPED
//    int t; // [esp+1Ch] [ebp-84h] BYREF
//    unsigned int v18[2]; // [esp+20h] [ebp-80h] BYREF
//    int cache; // [esp+28h] [ebp-78h]
//    trace_t result; // [esp+2Ch] [ebp-74h]
//    unsigned int c; // [esp+64h] [ebp-3Ch]
//    float MAX_DISTANCE_SCALE; // [esp+68h] [ebp-38h]
//    float MAX_DISTANCE; // [esp+6Ch] [ebp-34h]
//    int reflect; // [esp+70h] [ebp-30h]
//    float distance; // [esp+74h] [ebp-2Ch]
//    float quadT; // [esp+78h] [ebp-28h]
//    int i; // [esp+7Ch] [ebp-24h]
//    float v28; // [esp+80h] [ebp-20h]
//    float v29; // [esp+84h] [ebp-1Ch]
//    int v30; // [esp+88h] [ebp-18h]
//    float v31; // [esp+8Ch] [ebp-14h]
//    int v32; // [esp+94h] [ebp-Ch]
//    void *v33; // [esp+98h] [ebp-8h]
//    void *retaddr; // [esp+A0h] [ebp+0h]
//
//    v32 = a1;
//    v33 = retaddr;
//    //PIXBeginNamedEvent(-1, "proximity");
//    v31 = *front;
//    v30 = *((unsigned int *)front + 1);
//    __libm_sse2_atan2(v7, v12);
//    v29 = v31;
//    __libm_sse2_cos(v8);
//    v28 = v31;
//    *(float *)&i = v31 - front[1];
//    if ( fabs(i) >= SND_EPSILON
//        && !Assert_MyHandler(
//                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_occlusion.cpp",
//                    209,
//                    0,
//                    "%s",
//                    "fabs(cosf(forwardT)-front[1])< SND_EPSILON") )
//    {
//        __debugbreak();
//    }
//    __libm_sse2_sin(v9);
//    quadT = v29;
//    distance = v29 - *front;
//    if ( fabs(distance) >= SND_EPSILON
//        && !Assert_MyHandler(
//                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_occlusion.cpp",
//                    210,
//                    0,
//                    "%s",
//                    "fabs(sinf(forwardT)-front[0])< SND_EPSILON") )
//    {
//        __debugbreak();
//    }
//    for ( reflect = 0; reflect < 4; ++reflect )
//    {
//        MAX_DISTANCE = (float)((float)((float)reflect * 1.5707964) + v29) - 1.5707964;
//        MAX_DISTANCE_SCALE = 0.0f;
//        c = 0;
//        *(float *)&result.hitPartition = 1000.0f;
//        *(float *)&result.staticModel = 1000.0 / 16.0;
//        for ( *(unsigned int *)&result.walkable = 0; *(unsigned int *)&result.walkable < 0x10u; ++*(unsigned int *)&result.walkable )
//        {
//            v18[0] = 0;
//            v18[1] = 0;
//            cache = 0;
//            result.normal.vec.u[0] = 0;
//            t = 0;
//            *((_QWORD *)&end + 1) = *(unsigned int *)&result.walkable;
//            *((float *)&end + 1) = (double)*(unsigned int *)&result.walkable / 16.0 * 1.5707964 + MAX_DISTANCE;
//            x = *origin;
//            v14 = origin[1];
//            v15 = *((unsigned int *)origin + 2);
//            *(float *)&end = origin[3];
//            v5 = *((float *)&end + 1);
//            __libm_sse2_cos(v10);
//            *(float *)&v5 = v5;
//            v14 = (float)(*(float *)&v5 * *(float *)&result.hitPartition) + v14;
//            v6 = *((float *)&end + 1);
//            __libm_sse2_sin(v11);
//            *(float *)&v6 = v6;
//            x = (float)(*(float *)&v6 * *(float *)&result.hitPartition) + x;
//            if ( CG_SightTracePoint(&t, origin, &x, 49, (trace_t *)v18) )
//            {
//                MAX_DISTANCE_SCALE = (float)((float)(1.0
//                                                                                     - (float)((float)(1.0 - result.normal.vec.v[1])
//                                                                                                     * (float)(1.0 - result.normal.vec.v[1])))
//                                                                     * *(float *)&result.staticModel)
//                                                     + MAX_DISTANCE_SCALE;
//                *(float *)&c = SND_SurfaceTypeToReflectance((unsigned __int8)((int)(0x3F00000
//                                                                                                                                                    & result.normal.vec.u[2]) >> 20))
//                                         + *(float *)&c;
//            }
//            else
//            {
//                MAX_DISTANCE_SCALE = MAX_DISTANCE_SCALE + *(float *)&result.staticModel;
//                *(float *)&c = *(float *)&c + 0.5;
//            }
//        }
//        *(float *)&c = *(float *)&c / 16.0;
//        if ( (c & 0x7F800000) == 0x7F800000
//            && !Assert_MyHandler(
//                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_occlusion.cpp",
//                        250,
//                        0,
//                        "%s",
//                        "!IS_NAN(reflect)") )
//        {
//            __debugbreak();
//        }
//        if ( (LODWORD(MAX_DISTANCE_SCALE) & 0x7F800000) == 0x7F800000
//            && !Assert_MyHandler(
//                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_occlusion.cpp",
//                        251,
//                        0,
//                        "%s",
//                        "!IS_NAN(distance)") )
//        {
//            __debugbreak();
//        }
//        resultDistance[reflect] = MAX_DISTANCE_SCALE;
//        resultReflection[reflect] = *(float *)&c;
//    }
//    //if ( g_DXDeviceThread == GetCurrentThreadId() )
//        //D3DPERF_EndEvent();
//}

// (aislop cleanup)
void SND_TraceProximity(
    float *front,
    float *origin,
    float *resultDistance,
    float *resultReflection)
{
    float forwardT, quadT, MAX_DISTANCE, MAX_DISTANCE_SCALE, distance;
    float x, y, z;
    int reflect, i;
    trace_t result;

    //PIXBeginNamedEvent(-1, "proximity");

    // Engine angle convention: atan2(X, Y), so sin(t)=X=front[0], cos(t)=Y=front[1].
    forwardT = atan2(front[0], front[1]);
    if (fabsf(cosf(forwardT) - front[1]) >= SND_EPSILON)
        __debugbreak();
    if (fabsf(sinf(forwardT) - front[0]) >= SND_EPSILON)
        __debugbreak();

    // Loop over reflection directions
    for (reflect = 0; reflect < 4; ++reflect)
    {
        MAX_DISTANCE = reflect * (float)M_PI_HALF + forwardT - (float)M_PI_HALF;
        MAX_DISTANCE_SCALE = 0.0f;
        distance = 0.0f;

        for (i = 0; i < 16; ++i)
        {
            // Compute point to trace
            float theta = (float)i / 16.0f * (float)M_PI_HALF + MAX_DISTANCE;
            x = origin[0] + cosf(theta) * 1000.0f;
            y = origin[1] + sinf(theta) * 1000.0f;
            z = origin[2];

            int hitNum = 0;
            if (CG_SightTracePoint(&hitNum, origin, &x, 49, &result))
            {
                MAX_DISTANCE_SCALE += (1.0f - (1.0f - result.normal.vec.v[1]) * (1.0f - result.normal.vec.v[1])) * (1000.0f / 16.0f);
                distance += SND_SurfaceTypeToReflectance((result.normal.vec.u[2] & 0x3F00000) >> 20);
            }
            else
            {
                MAX_DISTANCE_SCALE += (1000.0f / 16.0f);
                distance += 0.5f;
            }
        }

        distance /= 16.0f;
        resultDistance[reflect] = MAX_DISTANCE_SCALE;
        resultReflection[reflect] = distance;
    }

    //if (g_DXDeviceThread == GetCurrentThreadId())
    //    D3DPERF_EndEvent();
}


float __cdecl SND_SurfaceTypeToReflectance(unsigned int surface)
{
    if (surface <= 0x1000000)
    {
        if (surface == 0x1000000)
            return 0.60000002;
        if (surface > 0x800000)
        {
            if (surface > 0xC00000)
            {
                if (surface == 0xD00000)
                    return 1.0;
                if (surface != 0xE00000 && surface != 0xF00000)
                    return 0.5;
                return 0.60000002;
            }
            if (surface == 0xC00000 || surface == 0x900000)
                return 1.0;
            if (surface != 0xA00000)
            {
                if (surface != 0xB00000)
                    return 0.5;
                return 0.60000002;
            }
        }
        else if (surface != 0x800000)
        {
            if (surface > 0x400000)
            {
                if (surface == 0x500000)
                    return 1.0;
                if (surface == 0x600000)
                    return 0.60000002;
                if (surface != 0x700000)
                    return 0.5;
            }
            else if (surface != 0x400000)
            {
                if (surface != 0x100000)
                {
                    if (surface != 0x200000)
                    {
                        if (surface != 0x300000)
                            return 0.5;
                        return 0.2;
                    }
                    return 1.0;
                }
                return 0.60000002;
            }
            return 0.2;
        }
        return 0.40000001;
    }
    if (surface > 0x1800000)
    {
        if (surface > 0x1C00000)
        {
            if (surface == 0x1D00000)
                return 0.2;
            if (surface != 0x1E00000)
                return 0.5;
        }
        else
        {
            if (surface == 0x1C00000)
                return 1.0;
            if (surface != 0x1900000)
            {
                if (surface != 0x1A00000 && surface != 0x1B00000)
                    return 0.5;
                return 0.2;
            }
        }
        return 0.40000001;
    }
    if (surface == 0x1800000)
        return 1.0;
    if (surface > 0x1400000)
    {
        if (surface != 0x1500000 && surface != 0x1600000 && surface != 0x1700000)
            return 0.5;
        return 1.0;
    }
    if (surface == 0x1400000 || surface == 0x1100000)
        return 1.0;
    if (surface == 0x1200000)
        return 0.60000002;
    if (surface != 0x1300000)
        return 0.5;
    return 0.2;
}

void __cdecl SND_UpdateProximity()
{
    int j; // [esp+28h] [ebp-Ch]
    float scale; // [esp+2Ch] [ebp-8h]
    int i; // [esp+30h] [ebp-4h]

    *(_QWORD *)&g_snd.proximityFront[1] = *(_QWORD *)&g_snd.listeners[0].orient.axis[0][1];
    g_snd.proximityOrigin[0] = g_snd.listeners[0].orient.origin[0];
    g_snd.proximityOrigin[1] = g_snd.listeners[0].orient.origin[1];
    *(_QWORD *)&g_snd.proximityOrigin[2] = *(_QWORD *)&g_snd.listeners[0].orient.origin[2];
    if ( snd_proximity_enable->current.enabled )
    {
        for ( i = 0; i < 4; ++i )
        {
            float x = I_fmap(
                snd_proximity_min_dist->current.value,
                snd_proximity_max_dist->current.value,
                snd_proximity_max_late_cut->current.value,
                0.0,
                g_snd.proximityDistance[i]);
            scale = SND_dBToLinear(x);
            g_snd.proximityLateModifier[i] = (float)((float)(1.0 - snd_proximity_filter->current.value)
                                                                                         * g_snd.proximityLateModifier[i])
                                                                         + (float)(scale * snd_proximity_filter->current.value);
        }
    }
    else
    {
        for ( j = 0; j < 4; ++j )
            g_snd.proximityLateModifier[j] = 1.0f;
    }
}

void __cdecl SND_LosOcclusionCmd()
{
    unsigned int i; // [esp+8h] [ebp-8h]
    int savedregs; // [esp+10h] [ebp+0h] BYREF

    Sys_EnterCriticalSection(CRITSEC_SOUND_OCCLUSION);
    if ( g_snd.occlusionRunning )
    {
        for ( i = 0; i < 0x4A; ++i )
        {
            if ( g_snd.occlusionTraces[i].id != -1 )
                g_snd.occlusionTraces[i].occlusion = SND_LosOcclusionTrace(
                                                                                             1,
                                                                                             &g_snd.occlusionTraces[i].cache,
                                                                                             g_snd.occlusionTraces[i].listener,
                                                                                             g_snd.occlusionTraces[i].playback);
        }
        SND_TraceProximity(
            g_snd.proximityFront,
            g_snd.proximityOrigin,
            g_snd.proximityDistance,
            g_snd.proximityReflectance);
        g_snd.lastTraceSequence = g_snd.traceSequence;
        Sys_LeaveCriticalSection(CRITSEC_SOUND_OCCLUSION);
    }
    else
    {
        Sys_LeaveCriticalSection(CRITSEC_SOUND_OCCLUSION);
    }
}

void __cdecl SND_LosOcclusionUpdate()
{
    LONG Target; // [esp+4h] [ebp-34h] BYREF
    float *playback; // [esp+8h] [ebp-30h]
    float *position; // [esp+Ch] [ebp-2Ch]
    snd_occlusion_trace_t *v4; // [esp+10h] [ebp-28h]
    snd_listener *v5; // [esp+14h] [ebp-24h]
    int li; // [esp+28h] [ebp-10h]
    snd_voice_t *voice; // [esp+2Ch] [ebp-Ch]
    unsigned int i; // [esp+30h] [ebp-8h]
    unsigned int updated; // [esp+34h] [ebp-4h]

    if (g_snd.lastTraceSequence == g_snd.traceSequence && Sys_TryEnterCriticalSection(CRITSEC_SOUND_OCCLUSION))
    {
        SND_UpdateProximity();
        updated = 0;
        for (i = 0; i < 0x4A; ++i)
        {
            voice = &g_snd.voice[i];
            if (g_snd.voiceAliasHash[(int)(472 * i) / 472] && (voice->alias->flags & 2) >> 1)
            {
                if (voice->firstPlaybackId == g_snd.occlusionTraces[i].id)
                    SND_FaderSetGoal(&voice->losOcclusion, g_snd.occlusionTraces[i].occlusion);
                else
                    g_snd.occlusionTraces[i].cache = 0;
                li = voice->closestListenerIndex;
                g_snd.occlusionTraces[i].id = voice->firstPlaybackId;
                v4 = &g_snd.occlusionTraces[i];
                v5 = &g_snd.listeners[li];
                v4->listener[0] = v5->orient.origin[0];
                v4->listener[1] = v5->orient.origin[1];
                v4->listener[2] = v5->orient.origin[2];
                playback = g_snd.occlusionTraces[i].playback;
                position = voice->position;
                *playback = voice->position[0];
                playback[1] = position[1];
                playback[2] = position[2];
                g_snd.occlusionTraces[i].occlusion = 0.0f;
                ++updated;
            }
            else
            {
                g_snd.occlusionTraces[i].id = -1;
                g_snd.occlusionTraces[i].cache = 0;
            }
        }
        if (updated)
        {
            ++g_snd.traceSequence;
            Target = 0;
            InterlockedExchange(&Target, 0);
            Sys_SetOcclusion();
        }
        g_snd.framesSinceLastOcclusionUpdate = 0;
        Sys_LeaveCriticalSection(CRITSEC_SOUND_OCCLUSION);
    }
    else
    {
        ++g_snd.framesSinceLastOcclusionUpdate;
    }
}

void __cdecl SND_LosOcclusionThreadMain()
{
    while ( Sys_QueryWin32QuitEvent() )
    {
        while ( g_snd.occlusionRunning )
        {
            Sys_WaitForOcclusion();
            if ( !g_snd.occlusionRunning )
                return;
            Sys_EnterCriticalSection(CRITSEC_SOUND_OCCLUSION);
            Sys_LeaveCriticalSection(CRITSEC_SOUND_OCCLUSION);
            if ( snd_losOcclusion->current.enabled )
            {
                PROF_SCOPED("SND_LosOcclusionCmd");
                SND_LosOcclusionCmd();
            }
        }
        NET_Sleep(0xAu);
    }
}

void __cdecl SND_LosOcclusionInit()
{
    LONG Target; // [esp+0h] [ebp-8h] BYREF
    unsigned int i; // [esp+4h] [ebp-4h]

    memset((unsigned __int8 *)g_snd.occlusionTraces, 0, sizeof(g_snd.occlusionTraces));
    for (i = 0; i < 0x4A; ++i)
        g_snd.occlusionTraces[i].id = -1;
    g_snd.occlusionRunning = 1;
    Target = 0;
    InterlockedExchange(&Target, 0);
    Sys_SpawnOcclusion((void(__cdecl *)(unsigned int))SND_LosOcclusionThreadMain);
}

void __cdecl SND_LosOcclusionFini()
{
    LONG Target; // [esp+0h] [ebp-4h] BYREF

    g_snd.occlusionRunning = 0;
    Target = 0;
    InterlockedExchange(&Target, 0);
    Sys_EnterCriticalSection(CRITSEC_SOUND_OCCLUSION);
    Sys_SetOcclusion();
    Sys_LeaveCriticalSection(CRITSEC_SOUND_OCCLUSION);
    g_snd.traceSequence = 0;
    g_snd.lastTraceSequence = 0;
}

