#include "cg_sound.h"
#include <client/splitscreen.h>
#include <cgame_mp/cg_local_mp.h>
#include <cgame_mp/cg_main_mp.h>
#include <gfx_d3d/r_scene.h>
#include <sound/snd_public_async.h>
#include <sound/snd_bank.h>
#include <sound/snd_utils.h>
#include <client_mp/cl_cgame_mp.h>
#include <cgame_mp/cg_animscripted_mp.h>
#include <bgame/bg_misc.h>
#include <universal/com_math_anglevectors.h>
#include <bgame/bg_weapons.h>
#include <sound/snd_occlusion.h>

const char *g_snd_fire_names[8] =
{
  "amb_fire_manager_0",
  "amb_fire_manager_1",
  "amb_fire_manager_2",
  "amb_fire_manager_3",
  "amb_fire_manager_4",
  "amb_fire_manager_5",
  "amb_fire_manager_6",
  "amb_fire_manager_7"
};

CgSoundPlaybackTrackingInfo cg_SoundPlaybackTracking[256];

snd_fire_manager g_snd_fires[1][8];

bool __cdecl CG_ShouldPlaySoundOnLocalClient(
                int localClientNum,
                int entitynum,
                const float *origin,
                unsigned int aliasname)
{
    cg_s *cgameGlob; // [esp+10h] [ebp-10h]
    int trackingIndex; // [esp+14h] [ebp-Ch]
    int clientIndex; // [esp+18h] [ebp-8h]
    CgSoundPlaybackTrackingInfo *trackingInfo; // [esp+1Ch] [ebp-4h]
    CgSoundPlaybackTrackingInfo *trackingInfoa; // [esp+1Ch] [ebp-4h]

    if ( CL_LocalClient_GetActiveCount() > 1 )
    {
        if ( CL_LocalClient_GetActiveCount() == 1 )
            return 1;
        if ( !aliasname )
            return 0;
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        if ( !cgameGlob->inKillCam )
        {
            for ( clientIndex = 0; clientIndex < 1; ++clientIndex )
            {
                if ( CL_LocalClient_IsActive(clientIndex) && CG_GetClientNumForLocalClient(clientIndex) == entitynum )
                    return clientIndex == localClientNum;
            }
        }
        for ( trackingIndex = 0; trackingIndex < 256; ++trackingIndex )
        {
            trackingInfo = &cg_SoundPlaybackTracking[trackingIndex];
            if ( !trackingInfo->aliasname )
                break;
            if ( entitynum == trackingInfo->entitynum
                && cgameGlob->inKillCam == trackingInfo->wasInKillcam
                && Vec3DistanceSq(origin, trackingInfo->origin) < 3.0
                && aliasname == trackingInfo->aliasname )
            {
                return 0;
            }
        }
        if ( trackingIndex >= 256 )
        {
            Com_Printf(9, "ERROR: not enough sound tracking entries for splitscreen\n");
            return 0;
        }
        trackingInfoa = &cg_SoundPlaybackTracking[trackingIndex];
        trackingInfoa->entitynum = entitynum;
        trackingInfoa->aliasname = aliasname;
        trackingInfoa->wasInKillcam = cgameGlob->inKillCam;
        trackingInfoa->origin[0] = *origin;
        trackingInfoa->origin[1] = origin[1];
        trackingInfoa->origin[2] = origin[2];
    }
    return 1;
}

SndEntHandle __cdecl CG_SndEntHandle(int localClientNum, int entIndex)
{
    bool isInKillCam; // [esp+17h] [ebp-Dh]
    team_t team; // [esp+18h] [ebp-Ch]
    const cg_s *cgameGlob; // [esp+1Ch] [ebp-8h]
    centity_s *cent; // [esp+20h] [ebp-4h]

    isInKillCam = 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->inKillCam || (cgameGlob->predictedPlayerState.otherFlags & 2) != 0 )
        isInKillCam = 1;
    cent = CG_GetEntity(localClientNum, entIndex);
    team = TEAM_FREE;
    if ( cent->nextState.eType == 1 )
        team = cgameGlob->bgs.clientinfo[cent->nextState.number].team;
    if ( entIndex == 1022 || cent->nextState.eType >= 21 )
        return SND_EntHandle(localClientNum, entIndex, cent->currentState.useCount, isInKillCam, 1, team);
    return SND_EntHandle(localClientNum, entIndex, cent->currentState.useCount, isInKillCam, 0, team);
}

SndEntHandle __cdecl SND_EntHandle(
                char localClientNum,
                __int16 entIndex,
                char useCount,
                bool isInKillCam,
                bool isStationary,
                team_t team)
{
    return (SndEntHandle)(((team & 3) << 23)
                                            | (isStationary << 21) & 0xFE7FFFFF
                                            | (isInKillCam << 22) & 0xFE7FFFFF
                                            | ((useCount & 0x7F) << 12) & 0xFE1FFFFF
                                            | entIndex & 0xFFF
                                            | ((localClientNum & 3) << 19) & 0xFE1FFFFF);
}

void __cdecl CG_PlaySound(
                int localClientNum,
                int entitynum,
                float *origin,
                int fadeMs,
                bool doNotify,
                float attenuation,
                unsigned int id)
{
    SndEntHandle entHandle; // [esp+2Ch] [ebp-24h]
    float midpoint_4; // [esp+34h] [ebp-1Ch]
    float midpoint_8; // [esp+38h] [ebp-18h]
    float *bmodel; // [esp+3Ch] [ebp-14h]
    const centity_s *cent; // [esp+40h] [ebp-10h]
    float tmpOrigin[3]; // [esp+44h] [ebp-Ch] BYREF

    if ( !origin )
    {
        cent = CG_GetEntity(localClientNum, entitynum);
        origin = tmpOrigin;
        if (cent->nextState.solid == 0xFFFFFF)
        {
            bmodel = (float *)R_GetBrushModel(cent->nextState.index.brushmodel);
            midpoint_4 = (float)(bmodel[8] + bmodel[11]) * 0.5;
            midpoint_8 = (float)(bmodel[9] + bmodel[12]) * 0.5;
            tmpOrigin[0] = cent->pose.origin[0] + (float)((float)(bmodel[7] + bmodel[10]) * 0.5);
            tmpOrigin[1] = cent->pose.origin[1] + midpoint_4;
            tmpOrigin[2] = cent->pose.origin[2] + midpoint_8;
        }
        else
        {
            tmpOrigin[0] = cent->pose.origin[0];
            tmpOrigin[1] = cent->pose.origin[1];
            tmpOrigin[2] = cent->pose.origin[2];
        }
    }
    if ( id )
    {
        if ( CG_ShouldPlaySoundOnLocalClient(localClientNum, entitynum, origin, id) )
        {
            entHandle.field = CG_SndEntHandle(localClientNum, entitynum).field;
            SND_Play(id, fadeMs, attenuation, entHandle, origin, 0, doNotify);
        }
    }
}

void __cdecl CG_PlaySound(
                int localClientNum,
                int entitynum,
                float *origin,
                int fadeMs,
                bool doNotify,
                float attenuation,
                char *name)
{
    unsigned int AliasId; // eax

    AliasId = SND_FindAliasId(name);
    CG_PlaySound(localClientNum, entitynum, origin, fadeMs, doNotify, attenuation, AliasId);
}

void __cdecl CG_PlaySound(int localClientNum, float *origin, int fadeMs, bool doNotify, float attenuation, char *name)
{
    CG_PlaySound(localClientNum, 1022, origin, fadeMs, doNotify, attenuation, name);
}

int __cdecl CG_PlaySoundWithHandle(
                int localClientNum,
                int entitynum,
                float *origin,
                int fadeMs,
                bool doNotify,
                float volume,
                unsigned int id)
{
    SndEntHandle entHandle; // [esp+2Ch] [ebp-24h]
    float midpoint_4; // [esp+34h] [ebp-1Ch]
    float midpoint_8; // [esp+38h] [ebp-18h]
    float *bmodel; // [esp+3Ch] [ebp-14h]
    const centity_s *cent; // [esp+40h] [ebp-10h]
    float tmpOrigin[3]; // [esp+44h] [ebp-Ch] BYREF

    if ( !origin )
    {
        cent = CG_GetEntity(localClientNum, entitynum);
        origin = tmpOrigin;
        if (cent->nextState.solid == 0xFFFFFF)
        {
            bmodel = (float *)R_GetBrushModel(cent->nextState.index.brushmodel);
            midpoint_4 = (float)(bmodel[8] + bmodel[11]) * 0.5;
            midpoint_8 = (float)(bmodel[9] + bmodel[12]) * 0.5;
            tmpOrigin[0] = cent->pose.origin[0] + (float)((float)(bmodel[7] + bmodel[10]) * 0.5);
            tmpOrigin[1] = cent->pose.origin[1] + midpoint_4;
            tmpOrigin[2] = cent->pose.origin[2] + midpoint_8;
        }
        else
        {
            tmpOrigin[0] = cent->pose.origin[0];
            tmpOrigin[1] = cent->pose.origin[1];
            tmpOrigin[2] = cent->pose.origin[2];
        }
    }
    if ( !id || !CG_ShouldPlaySoundOnLocalClient(localClientNum, entitynum, origin, id) )
        return -1;
    entHandle.field = CG_SndEntHandle(localClientNum, entitynum).field;
    return SND_Playback(id, fadeMs, volume, entHandle, origin, 0, doNotify);
}

int __cdecl CG_PlaySoundWithHandle(
                int localClientNum,
                int entitynum,
                float *origin,
                int fadeMs,
                bool doNotify,
                float attenuation,
                char *name)
{
    unsigned int AliasId; // eax

    AliasId = SND_FindAliasId(name);
    return CG_PlaySoundWithHandle(localClientNum, entitynum, origin, fadeMs, doNotify, attenuation, AliasId);
}

void __cdecl CG_PumpEntityLoopSound(int localClientNum, const centity_s *cent)
{
    CG_PlaySound(
        localClientNum,
        cent->nextState.number,
        0,
        cent->nextState.loopSoundFade,
        0,
        1.0,
        cent->nextState.loopSoundId);
}

void __cdecl CG_PlayAnimScriptSoundAlias(int clientIndex, snd_alias_list_t *aliasList)
{
    int ClientNumForLocalClient; // eax
    unsigned int v3; // [esp+4h] [ebp-8h]
    int localClientNum; // [esp+8h] [ebp-4h]

    for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
    {
        if ( CL_LocalClient_IsActive(localClientNum) )
        {
            if ( CG_GetClientNumForLocalClient(localClientNum) == clientIndex )
            {
                v3 = SND_HashAlias(aliasList);
                ClientNumForLocalClient = CG_GetClientNumForLocalClient(localClientNum);
                CG_PlaySound(localClientNum, ClientNumForLocalClient, 0, 0, 0, 1.0, v3);
            }
        }
    }
}

void __cdecl CG_SubtitlePrint(int msec, const char *subtitle)
{
    int integer; // [esp+4h] [ebp-20h]
    int v3; // [esp+8h] [ebp-1Ch]

    if ( !subtitle
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_sound.cpp", 309, 0, "%s", "subtitle") )
    {
        __debugbreak();
    }
    if ( !cg_subtitleWidthStandard
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_sound.cpp",
                    310,
                    0,
                    "%s",
                    "cg_subtitleWidthStandard") )
    {
        __debugbreak();
    }
    if ( !cg_subtitleWidthWidescreen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_sound.cpp",
                    311,
                    0,
                    "%s",
                    "cg_subtitleWidthWidescreen") )
    {
        __debugbreak();
    }
    if ( !cg_subtitleMinTime
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_sound.cpp", 312, 0, "%s", "cg_subtitleMinTime") )
    {
        __debugbreak();
    }
    if ( msec && subtitle )
    {
        if ( CG_GetLocalClientStaticGlobals(0)->viewAspect <= 1.3333334 )
            integer = cg_subtitleWidthStandard->current.integer;
        else
            integer = cg_subtitleWidthWidescreen->current.integer;
        v3 = (int)((float)(1000.0 * cg_subtitleMinTime->current.value) + 9.313225746154785e-10);
        if ( v3 < msec )
            CL_SubtitlePrint(0, subtitle, msec, integer);
        else
            CL_SubtitlePrint(0, subtitle, v3, integer);
    }
}

void __cdecl CG_SubtitleSndLengthNotify(int msec, const char *lengthNotifyData)
{
    CG_SubtitlePrint(msec, lengthNotifyData);
}

int __cdecl CG_SoundGetUseCount(SndEntHandle sndEnt)
{
    return CG_GetEntity(((unsigned int)sndEnt.handle >> 19) & 3, *(_WORD *)&sndEnt.field & 0xFFF)->currentState.useCount;
}

bool __cdecl CG_SoundEntInUse(SndEntHandle sndEnt)
{
    if ( (*(_WORD *)&sndEnt.field & 0xFFFu) <= 0x400 )
        return 1;
    return (cg_fakeEntitiesArray[512 * (((unsigned int)sndEnt.handle >> 19) & 3)
                                                         - 1024
                                                         + (*(_WORD *)&sndEnt.field & 0xFFF)].flags
                & 4) == 0
            && cg_fakeEntitiesInuseArray[512 * (((unsigned int)sndEnt.handle >> 19) & 3)
                                                                 + &cg_fakeEntitiesArray[512 * (((unsigned int)sndEnt.handle >> 19) & 3)
                                                                                                             - 1024
                                                                                                             + (*(_WORD *)&sndEnt.field & 0xFFF)]
                                                                 - &cg_fakeEntitiesArray[512 * (((unsigned int)sndEnt.handle >> 19) & 3)]];
}

void __cdecl CG_GetSoundEntityOrientation(
                SndEntHandle sndEnt,
                float *origin_out,
                float (*axis_out)[3],
                float *velocity_out)
{
    const centity_s *parentEnt; // [esp+5Ch] [ebp-24h]
    centity_s *cent; // [esp+60h] [ebp-20h]
    float next[3]; // [esp+64h] [ebp-1Ch] BYREF
    float current[3]; // [esp+70h] [ebp-10h] BYREF
    const cg_s *cgameGlob; // [esp+7Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(((unsigned int)sndEnt.handle >> 19) & 3);
    if ( velocity_out && cgameGlob->snap && cgameGlob->nextSnap )
    {
        cent = CG_GetEntity(((unsigned int)sndEnt.handle >> 19) & 3, *(_WORD *)&sndEnt.field & 0xFFF);
        if ( CG_EntityLinked(((unsigned int)sndEnt.handle >> 19) & 3, cent) )
        {
            parentEnt = CG_GetEntity(((unsigned int)sndEnt.handle >> 19) & 3, cent->linkInfo->linkEnt);
            BG_EvaluateTrajectory(&parentEnt->currentState.pos, cgameGlob->snap->serverTime, current);
            BG_EvaluateTrajectory(&parentEnt->nextState.lerp.pos, cgameGlob->nextSnap->serverTime, next);
        }
        else
        {
            BG_EvaluateTrajectory(&cent->currentState.pos, cgameGlob->snap->serverTime, current);
            BG_EvaluateTrajectory(&cent->nextState.lerp.pos, cgameGlob->nextSnap->serverTime, next);
        }
        *velocity_out = next[0] - current[0];
        velocity_out[1] = next[1] - current[1];
        velocity_out[2] = next[2] - current[2];
    }
    else if ( velocity_out )
    {
        *velocity_out = 0.0f;
        velocity_out[1] = 0.0f;
        velocity_out[2] = 0.0f;
    }
    if (velocity_out)
    {
        nanassertvec3(velocity_out);
    }
    
    CG_CopySoundEntityOrientation(0, *(_WORD *)&sndEnt.field & 0xFFF, origin_out, axis_out);
}

void __cdecl CG_CopySoundEntityOrientation(int localClientNum, int entIndex, float *origin_out, float (*axis_out)[3])
{
    const centity_s *cent; // [esp+3Ch] [ebp-4h]

    if ( !origin_out
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_sound.cpp", 354, 0, "%s", "origin_out") )
    {
        __debugbreak();
    }
    if ( !axis_out
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_sound.cpp", 355, 0, "%s", "axis_out") )
    {
        __debugbreak();
    }
    cent = CG_GetEntity(localClientNum, entIndex);
    *origin_out = cent->pose.origin[0];
    origin_out[1] = cent->pose.origin[1];
    origin_out[2] = cent->pose.origin[2];
    AnglesToAxis(cent->pose.angles, axis_out);

    nanassertvec3(origin_out);
    nanassertvec3(axis_out[0]);
    nanassertvec3(axis_out[1]);
    nanassertvec3(axis_out[2]);
}

void __cdecl CG_StopSoundAlias(int localClientNum, int entityNum, unsigned int aliasName)
{
    SndEntHandle ent; // [esp+0h] [ebp-4h]

    if ( aliasName )
    {
        ent.field = CG_SndEntHandle(localClientNum, entityNum).field;
        SND_StopSoundAliasOnEnt(ent, aliasName);
    }
}

void __cdecl CG_FireManagerUpdate(unsigned int listener, unsigned int index, const float *location, float level)
{
    snd_fire_manager *fire; // [esp+4h] [ebp-4h]

    if ( listener
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_sound.cpp",
                    463,
                    0,
                    "listener doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    listener,
                    1) )
    {
        __debugbreak();
    }
    if ( index >= 8
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_sound.cpp",
                    464,
                    0,
                    "index doesn't index SND_FIRE_COUNT\n\t%i not in [0, %i)",
                    index,
                    8) )
    {
        __debugbreak();
    }
    fire = &g_snd_fires[listener][index];
    fire->active = 1;
    fire->location[0] = *location;
    fire->location[1] = location[1];
    fire->location[2] = location[2];
    fire->level = level;
}

void __cdecl CG_SndFireReset()
{
    snd_fire_manager *fire; // [esp+4h] [ebp-Ch]
    unsigned int f; // [esp+8h] [ebp-8h]
    int localClientNum; // [esp+Ch] [ebp-4h]

    for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
    {
        for ( f = 0; f < 8; ++f )
        {
            fire = &g_snd_fires[localClientNum][f];
            fire->active = 0;
            fire->location[0] = 0.0f;
            fire->location[1] = 0.0f;
            fire->location[2] = 0.0f;
            fire->level = 0.0f;
            fire->id = -1;
        }
    }
}

void __cdecl CG_SndUpdateFire()
{
    unsigned int AliasId; // eax
    snd_fire_manager *fire; // [esp+Ch] [ebp-10h]
    unsigned int f; // [esp+10h] [ebp-Ch]
    cg_s *cgameGlob; // [esp+14h] [ebp-8h]
    int localClientNum; // [esp+18h] [ebp-4h]

    for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        for ( f = 0; f < 8; ++f )
        {
            fire = &g_snd_fires[localClientNum][f];
            if ( fire->active )
            {
                if ( fire->level >= SND_EPSILON )
                {
                    AliasId = SND_FindAliasId((char *)g_snd_fire_names[f % 8]);
                    CG_PlaySound(localClientNum, cgameGlob->clientNum, fire->location, 0, 0, fire->level, AliasId);
                }
            }
        }
    }
}

void __cdecl CG_FlameSourceUpdate(
                int localClientNum,
                int entityNum,
                float *sourcePosition,
                float *flamePosition,
                snd_flame_source_flags flags,
                char *flameOffLoopSound,
                char *flameIgniteSound,
                char *flameOnLoopSound,
                char *flameCooldownSound)
{
    unsigned int AliasId; // eax
    unsigned int v10; // eax
    unsigned int v11; // eax
    unsigned int v12; // eax
    unsigned int v13; // eax

    if ( (flags & 1) != 0 )
    {
        AliasId = SND_FindAliasId(flameCooldownSound);
        CG_StopSoundAlias(localClientNum, entityNum, AliasId);
        v10 = SND_FindAliasId(flameIgniteSound);
        CG_PlaySound(localClientNum, entityNum, sourcePosition, 0, 0, 1.0, v10);
    }
    if ( (flags & 2) != 0 )
    {
        v11 = SND_FindAliasId(flameIgniteSound);
        CG_StopSoundAlias(localClientNum, entityNum, v11);
        v12 = SND_FindAliasId(flameCooldownSound);
        CG_PlaySound(localClientNum, entityNum, sourcePosition, 0, 0, 1.0, v12);
    }
    if ( (flags & 4) != 0 )
        v13 = SND_FindAliasId(flameOnLoopSound);
    else
        v13 = SND_FindAliasId(flameOffLoopSound);
    CG_PlaySound(localClientNum, entityNum, flamePosition, 350, 0, 1.0, v13);
}

void __cdecl CG_SoundWhizby(
                int localClientNum,
                const WeaponDef *weapDef,
                const float *shotPosition,
                const float *shotDirection,
                const float *position)
{
    unsigned int v5; // [esp+14h] [ebp-5Ch]
    unsigned int id; // [esp+18h] [ebp-58h]
    unsigned int AliasId; // [esp+20h] [ebp-50h]
    SndEntHandle v8; // [esp+34h] [ebp-3Ch]
    SndEntHandle entHandle; // [esp+38h] [ebp-38h]
    bool isUnderwater; // [esp+47h] [ebp-29h]
    cg_s *cgameGlob; // [esp+48h] [ebp-28h]
    float startPosition[3]; // [esp+4Ch] [ebp-24h] BYREF
    float endPosition[3]; // [esp+58h] [ebp-18h] BYREF
    float toShot[3]; // [esp+64h] [ebp-Ch] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    toShot[0] = *shotPosition - cgameGlob->refdef.vieworg[0];
    toShot[1] = shotPosition[1] - cgameGlob->refdef.vieworg[1];
    toShot[2] = shotPosition[2] - cgameGlob->refdef.vieworg[2];
    Vec3Normalize(toShot);
    if ( (float)((float)((float)(toShot[0] * *shotDirection) + (float)(toShot[1] * shotDirection[1]))
                         + (float)(toShot[2] * shotDirection[2])) < 0.0
        && CG_SoundWhizbyPath(shotPosition, position, cgameGlob->refdef.vieworg, 250.0, startPosition, endPosition) )
    {
        if ( weapDef->whizbySound && *weapDef->whizbySound )
            AliasId = SND_FindAliasId((char *)weapDef->whizbySound);
        else
            AliasId = cgMedia.bulletWhizby;
        isUnderwater = CM_GetWaterHeight(cgameGlob->refdef.vieworg, 200.0, -200.0) > cgameGlob->refdef.vieworg[2];
        entHandle.field = CG_SndEntHandle(localClientNum, 1022).field;
        if ( isUnderwater )
            id = cgMedia.underwaterWhizby;
        else
            id = AliasId;
        SND_Play(id, 0, 1.0, entHandle, startPosition, shotDirection, 0);
        if ( weapDef->crackSound && *weapDef->crackSound )
            v5 = SND_FindAliasId((char *)weapDef->crackSound);
        else
            v5 = 0;
        if ( v5 )
        {
            v8.field = CG_SndEntHandle(localClientNum, 1022).field;
            SND_Play(v5, 0, 1.0, v8, startPosition, shotDirection, 0);
        }
    }
}

char __cdecl CG_SoundWhizbyPath(const float *p1, const float *p2, const float *p3, float r, float *start, float *end)
{
    float v7; // [esp+8h] [ebp-54h]
    float fraction; // [esp+Ch] [ebp-50h]
    float v9; // [esp+10h] [ebp-4Ch]
    float v10; // [esp+14h] [ebp-48h]
    float t0; // [esp+2Ch] [ebp-30h]
    float d; // [esp+34h] [ebp-28h]
    float da; // [esp+34h] [ebp-28h]
    float t1; // [esp+38h] [ebp-24h]
    float b; // [esp+48h] [ebp-14h]
    float a; // [esp+58h] [ebp-4h]

    a = Vec3DistanceSq(p2, p1);
    b = (float)((float)((float)((float)(*p2 - *p1) * (float)(*p1 - *p3))
                                        + (float)((float)(p2[1] - p1[1]) * (float)(p1[1] - p3[1])))
                        + (float)((float)(p2[2] - p1[2]) * (float)(p1[2] - p3[2])))
        * 2.0;
    d = (float)(b * b)
        - (float)((float)(4.0 * a)
                        * (float)((float)((float)((float)((float)((float)(*p3 * *p3) + (float)(p3[1] * p3[1]))
                                                                                        + (float)(p3[2] * p3[2]))
                                                                        + (float)((float)((float)(*p1 * *p1) + (float)(p1[1] * p1[1]))
                                                                                        + (float)(p1[2] * p1[2])))
                                                        - (float)((float)((float)((float)(*p3 * *p1) + (float)(p3[1] * p1[1]))
                                                                                        + (float)(p3[2] * p1[2]))
                                                                        * 2.0))
                                        - (float)(r * r)));
    if ( d <= 0.0 )
        return 0;
    da = sqrtf(d);
    t0 = (float)(-(b) + da) / (float)(2.0 * a);
    t1 = (float)(-(b) - da) / (float)(2.0 * a);
    if ( (float)(t0 - 1.0) < 0.0 )
        v10 = (float)(-(b) + da) / (float)(2.0 * a);
    else
        v10 = 1.0f;
    if ( (float)(0.0 - t0) < 0.0 )
        fraction = v10;
    else
        fraction = 0.0f;
    if ( (float)(t1 - 1.0) < 0.0 )
        v9 = (float)(-(b) - da) / (float)(2.0 * a);
    else
        v9 = 1.0f;
    if ( (float)(0.0 - t1) < 0.0 )
        v7 = v9;
    else
        v7 = 0.0f;
    if ( fraction == v7 )
        return 0;
    if ( v7 <= fraction )
    {
        Vec3Lerp(p1, p2, v7, start);
        Vec3Lerp(p1, p2, fraction, end);
    }
    else
    {
        Vec3Lerp(p1, p2, fraction, start);
        Vec3Lerp(p1, p2, v7, end);
    }
    return 1;
}

void __cdecl CG_SndUpdate()
{
    PROF_SCOPED("CG_SndUpdate"); // LWSS ADD

    CG_SndUpdateAutoSim();
    CG_SndUpdateFire();
}

void __cdecl CG_SndGameReset()
{
    CG_SndAutoSimReset();
    CG_SndFireReset();
}

