#include "snd_debug.h"
#include <qcommon/cmd.h>
#include "snd_dvar.h"
#include <universal/assertive.h>
#include "snd.h"

#include <cstring>
#include <client/cl_debugdata.h>
#include "snd_db.h"
#include <cgame/cg_sound.h>
#include "snd_public_async.h"
#include <qcommon/common.h>
#include <universal/com_math_anglevectors.h>
#include "snd_globals.h"
#include "snd_utils.h"

cmd_function_s SND_PlayLocal_f_VAR;

void __cdecl SND_DebugInit()
{
    Cmd_AddCommandInternal("snd_playLocal", SND_PlayLocal_f, &SND_PlayLocal_f_VAR);
}

void __cdecl SND_DebugFini()
{
    Cmd_RemoveCommand("snd_playLocal");
}

void __cdecl SND_DebugDrawWorldSounds(int debugDrawStyle)
{
    const char *v2; // eax
    const char *v3; // eax
    int closestId; // [esp+10h] [ebp-180Ch] BYREF
    int dst[1536]; // [esp+14h] [ebp-1808h] BYREF
    int idx; // [esp+1814h] [ebp-8h]
    float closestIdDotProd; // [esp+1818h] [ebp-4h] BYREF

    if (debugDrawStyle)
    {
        closestId = -1;
        closestIdDotProd = -2.0f;
        memset(dst, 0, sizeof(dst));
        for (idx = 0; idx < 74; ++idx)
        {
            if (g_snd.voiceAliasHash[idx])
            {
                if (!g_snd.voice[idx].alias
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_debug.cpp",
                        199,
                        0,
                        "%s",
                        "g_snd.voice[idx].alias"))
                {
                    __debugbreak();
                }
                if ((g_snd.voice[idx].alias->flags & 2) >> 1)
                {
                    if (!snd_solo_alias_substring->current.integer
                        || !*(_BYTE *)snd_solo_alias_substring->current.integer
                        || (v2 = strstr(g_snd.voice[idx].alias->name, snd_solo_alias_substring->current.string),
                            v2))
                    {
                        if (!snd_mute_alias_substring->current.integer
                            || !*(_BYTE *)snd_mute_alias_substring->current.integer
                            || (v3 = strstr(g_snd.voice[idx].alias->name, snd_mute_alias_substring->current.string),
                                !v3))
                        {
                            DebugDrawWorldSound3D(idx, debugDrawStyle, dst, &closestId, &closestIdDotProd);
                        }
                    }
                }
            }
        }
        if (closestId != -1 && closestIdDotProd >= 0.93000001 && debugDrawStyle == 1)
            DebugDrawWorldSound3D(closestId, 3, dst, 0, 0);
    }
}

void __cdecl DebugDrawWorldSound3D(
                unsigned int idx,
                int debugDrawStyle,
                int *offsets,
                int *closestId,
                float *closestIdDotProd)
{
    float sndDir[3]; // [esp+60h] [ebp-148h] BYREF
    int listenerId; // [esp+6Ch] [ebp-13Ch]
    float dot; // [esp+70h] [ebp-138h]
    snd_voice_t *voice; // [esp+74h] [ebp-134h]
    char buffer[260]; // [esp+78h] [ebp-130h] BYREF
    float dist; // [esp+180h] [ebp-28h]
    int time; // [esp+184h] [ebp-24h]
    const float *starColor; // [esp+188h] [ebp-20h]
    float org[3]; // [esp+18Ch] [ebp-1Ch] BYREF
    int entNum; // [esp+198h] [ebp-10h]
    float fontsize; // [esp+19Ch] [ebp-Ch]
    float origZ; // [esp+1A0h] [ebp-8h]
    const char *text; // [esp+1A4h] [ebp-4h]

    if (idx >= 0x4A
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_debug.cpp",
            74,
            0,
            "%s\n\t(idx) = %i",
            "(( idx >= 0 ) && ( idx < (64 + 10) ))",
            idx))
    {
        __debugbreak();
    }
    voice = &g_snd.voice[idx];
    if (!voice->alias
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_debug.cpp", 77, 0, "%s", "voice->alias"))
    {
        __debugbreak();
    }
    entNum = voice->sndEnt.handle & 0xFFF;
    if (entNum > 1022)
        entNum = 1022;
    org[0] = voice->position[0];
    org[1] = voice->position[1];
    org[2] = voice->position[2];
    dist = voice->baseDistance;
    fontsize = FontSizeForDistance(dist);
    if (g_snd.pausetime)
        time = g_snd.pausetime - voice->startTime;
    else
        time = g_snd.time - voice->startTime;
    if (debugDrawStyle != 3)
    {
        listenerId = g_snd.voice[idx].closestListenerIndex;
        if (!closestId
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_debug.cpp", 102, 0, "%s", "closestId"))
        {
            __debugbreak();
        }
        if (!closestIdDotProd
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_debug.cpp", 103, 0, "%s", "closestIdDotProd"))
        {
            __debugbreak();
        }
        sndDir[0] = org[0] - g_snd.listeners[listenerId].orient.origin[0];
        sndDir[1] = org[1] - g_snd.listeners[listenerId].orient.origin[1];
        sndDir[2] = org[2] - g_snd.listeners[listenerId].orient.origin[2];
        Vec3Normalize(sndDir);
        dot = (float)((float)(sndDir[0] * g_snd.listeners[listenerId].orient.axis[0][0])
            + (float)(sndDir[1] * g_snd.listeners[listenerId].orient.axis[0][1]))
            + (float)(sndDir[2] * g_snd.listeners[listenerId].orient.axis[0][2]);
        if (dot > *closestIdDotProd)
        {
            *closestId = idx;
            *closestIdDotProd = dot;
        }
    }
    origZ = org[2];
    if (entNum == 1022)
        starColor = colorOrange;
    else
        starColor = colorMagenta;
    switch (debugDrawStyle)
    {
    case 1:
        goto LABEL_29;
    case 2:
        CL_AddDebugString(org, colorWhiteFaded, fontsize, (char *)voice->alias->name, 1);
    LABEL_29:
        CL_AddDebugStarWithText(org, starColor, colorWhiteFaded, 0, fontsize, 1);
        return;
    case 3:
        fontsize = fontsize * 0.85000002;
        text = va("Details: %s %d", voice->alias->name, entNum);
        if (offsets[entNum])
            CL_AddDebugStarWithText(org, starColor, colorWhiteFaded, 0, fontsize, 1);
        else
            CL_AddDebugStarWithText(org, starColor, colorWhiteFaded, (char *)text, fontsize, 1);
        if (!voice->alias->soundFile
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_debug.cpp",
                144,
                0,
                "%s",
                "voice->alias->soundFile"))
        {
            __debugbreak();
        }
        org[2] = org[2] - (float)(fontsize * 12.0);
        strcpy(buffer, "File: ");
        SND_AliasGetFileName(
            voice->alias,
            &buffer[&buffer[strlen(buffer) + 1] - &buffer[1]],
            256 - (&buffer[strlen(buffer) + 1] - &buffer[1]));
        CL_AddDebugString(org, colorWhiteFaded, fontsize, buffer, 1);
        org[2] = org[2] - (float)(fontsize * 12.0);
        if (entNum == 1022)
            text = va("Owner: World");
        else
            text = va("Owner: entity #%i", voice->sndEnt.handle & 0xFFF);
        CL_AddDebugString(org, colorWhiteFaded, fontsize, (char *)text, 1);
        org[2] = org[2] - (float)(fontsize * 12.0);
        text = va("Distance: %.0f / %.0f", dist, (float)voice->alias->distMax);
        CL_AddDebugString(org, colorWhiteFaded, fontsize, (char *)text, 1);
        org[2] = org[2] - (float)(fontsize * 12.0);
        text = va("Time: %.3f", (float)((float)time / 1000.0));
        CL_AddDebugString(org, colorWhiteFaded, fontsize, (char *)text, 1);
        break;
    }
}

double __cdecl FontSizeForDistance(float distance)
{
    if ( distance >= 10.0 )
        return distance / 2000.0 * 3.5;
    else
        return 0.050000001;
}

void __cdecl SND_PlayLocal_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    char *v4; // eax
    SndEntHandle entHandle; // [esp+4Ch] [ebp-2Ch]
    float soundDir[3]; // [esp+54h] [ebp-24h] BYREF
    float dist; // [esp+60h] [ebp-18h]
    float yaw; // [esp+64h] [ebp-14h]
    float pitch; // [esp+68h] [ebp-10h]
    float soundPos[3]; // [esp+6Ch] [ebp-Ch] BYREF

    dist = 100.0f;
    yaw = 0.0f;
    pitch = 0.0f;
    switch ( Cmd_Argc() )
    {
        case 2:
            goto LABEL_6;
        case 3:
            goto $LN3_216;
        case 4:
            goto $LN4_213;
        case 5:
            v0 = Cmd_Argv(4);
            pitch = atof(v0);
$LN4_213:
            v1 = Cmd_Argv(3);
            yaw = atof(v1);
$LN3_216:
            v2 = Cmd_Argv(2);
            dist = atof(v2);
LABEL_6:
            soundDir[0] = 1.0f;
            soundDir[1] = 0.0f;
            soundDir[2] = 0.0f;
            RelativeToListener(g_snd.listeners, yaw, pitch, dist, soundPos);
            entHandle.field = SND_EntHandle(0, 4094, 0, 0, 1, TEAM_FREE).field;
            v4 = (char *)Cmd_Argv(1);
            SND_Play(v4, 0, 1.0, entHandle, soundPos, soundDir, 0);
            break;
        default:
            v3 = Cmd_Argv(0);
            Com_Printf(0, "USAGE: %s <sndalias> [<dist> <yaw> <pitch>]\n", v3);
            break;
    }
}

void __cdecl RelativeToListener(const snd_listener *listener, float yaw, float pitch, float dist, float *result)
{
    float inputAngles[3]; // [esp+0h] [ebp-24h] BYREF
    float sndDir[3]; // [esp+Ch] [ebp-18h] BYREF
    float clientAngles[3]; // [esp+18h] [ebp-Ch] BYREF

    inputAngles[0] = -pitch;
    inputAngles[2] = 0.0f;
    AxisToAngles(listener->orient.axis, clientAngles);
    //inputAngles[1] = COERCE_FLOAT(LODWORD(yaw) ^ _mask__NegFloat_) + clientAngles[1];
    inputAngles[1] = -yaw + clientAngles[1];
    AngleVectors(inputAngles, sndDir, 0, 0);
    *result = dist * sndDir[0];
    result[1] = dist * sndDir[1];
    result[2] = dist * sndDir[2];
    *result = listener->orient.origin[0] + *result;
    result[1] = listener->orient.origin[1] + result[1];
    result[2] = listener->orient.origin[2] + result[2];
}

int __cdecl SND_GetSoundOverlay(snd_overlay_info *info, int start, int count)
{
    int v3; // ecx
    snd_voice_t *voice; // [esp+10h] [ebp-Ch]
    int i; // [esp+18h] [ebp-4h]

    if (count + start > 74
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_debug.cpp",
            278,
            0,
            "%s",
            "start+count <= SND_MAX_VOICES"))
    {
        __debugbreak();
    }
    memset(info, 0, 240 * count);
    for (i = 0; i < count; ++i)
    {
        voice = &g_snd.voice[start + i];
        info[i].channel = i;
        info[i].fGlobalPriority = -1.0f;
        info[i].pszSampleName[0] = 0;
        if (g_snd.voiceAliasHash[i + start] && SND_GroupGetAttenuation(voice->group) >= SND_EPSILON)
        {
            if (!voice->alias
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_debug.cpp", 301, 0, "%s", "voice->alias"))
            {
                __debugbreak();
            }
            SND_AliasGetFileName(voice->alias, info[i].pszSampleName, 128);
            Com_sprintf(info[i].name, 0x40u, "%s", voice->alias->name);
            info[i].channel |= (voice->alias->flags & 0xC000) >> 14 << 30;
            info[i].fPitch = SND_GetPitch(voice);
            info[i].fDry = SND_LinearToDbSpl(voice->dryLevel);
            info[i].fWet = SND_LinearToDbSpl(voice->wetLevel);
            info[i].fDistance = SND_GetDistance(voice);
            info[i].iEntity = voice->sndEnt.handle & 0xFFF;
            info[i].fLosOcclusion = SND_GetLpfLevel(voice);
            info[i].fGlobalPriority = voice->globalPriority;
            info[i].group = voice->group;
            info[i].pan = voice->alias->pan;
            v3 = i;
            LODWORD(info[v3].played) = voice->played;
            HIDWORD(info[v3].played) = HIDWORD(voice->played);
        }
    }
    return count;
}

int __cdecl SND_GetSoundOverlay(snd_overlay_type type, snd_overlay_info *info, int maxcount)
{
    if ( !info && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_debug.cpp", 328, 0, "%s", "info") )
        __debugbreak();
    if ( maxcount <= 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_debug.cpp", 329, 0, "%s", "maxcount > 0") )
    {
        __debugbreak();
    }
    if ( type == SND_OVERLAY_3D )
    {
        if ( maxcount > 64 )
            return SND_GetSoundOverlay(info, 10, 64);
        else
            return SND_GetSoundOverlay(info, 10, maxcount);
    }
    else if ( type == SND_OVERLAY_STREAM )
    {
        if ( maxcount > 10 )
            return SND_GetSoundOverlay(info, 0, 10);
        else
            return SND_GetSoundOverlay(info, 0, maxcount);
    }
    else
    {
        return 0;
    }
}

