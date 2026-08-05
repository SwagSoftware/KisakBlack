#include "cg_compassfriendlies_mp.h"

#include <cstring>
#include <universal/assertive.h>
#include <cgame/cg_compass.h>
#include "cg_vehicles_mp.h"
#include <bgame/bg_perks.h>
#include <bgame/bg_misc.h>
#include "cg_draw_mp.h"
#include <client/cl_voice.h>
#include "cg_main_mp.h"
#include <cgame/cg_drawtools.h>
#include <ui/ui_atoms.h>
#include <qcommon/com_gamemodes.h>
#include "cg_scoreboard_mp.h"
#include <client_mp/cl_cgame_mp.h>
#include <gfx_d3d/r_rendercmds.h>
#include <client/splitscreen.h>

CompassActor s_compassActors[1][32];
CompassArtillery s_compassArtillery[1][6];
CompassTurrets s_compassTurrets[1][32];
CompassVehicle s_compassVehicles[1][16];
CompassDogs s_compassDogs[1][8];
CompassHelicopter s_compassHelicopter[1][8];
CompassFakeFire s_compassFakeFire[1][32][8];
CompassGuidedMissile s_compassGuidedMissiles[1][32];


void __cdecl CG_ClearCompassPingData()
{
    memset(s_compassActors, 0, sizeof(s_compassActors));
    memset(s_compassFakeFire, 0, sizeof(s_compassFakeFire));
    memset(s_compassVehicles, 0, sizeof(s_compassVehicles));
    memset(s_compassDogs, 0, sizeof(s_compassDogs));
    memset(s_compassTurrets, 0, sizeof(s_compassTurrets));
    memset(s_compassArtillery, 0, sizeof(s_compassArtillery));
    memset(s_compassHelicopter, 0, sizeof(s_compassHelicopter));
    memset(s_compassGuidedMissiles, 0, sizeof(s_compassGuidedMissiles));
}

void __cdecl RadarPingEnemyPlayer(CompassActor *actor, int time, unsigned int PerkToIgnore)
{
    if ( !actor
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    299,
                    0,
                    "%s",
                    "actor") )
    {
        __debugbreak();
    }
    if ( actor->lastUpdate > time )
        actor->lastUpdate = 0;
    if ( actor->lastUpdate >= time - 1500 && (PerkToIgnore == 52 || !BG_HasPerk(actor->perks, PerkToIgnore)) )
    {
        *(double *)actor->lastEnemyPos = *(double *)actor->lastPos;
        actor->beginRadarFadeTime = time;
    }
}

void __cdecl RadarPingEnemyTurrets(CompassTurrets *turret, int time)
{
    if ( !turret
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    315,
                    0,
                    "%s",
                    "turret") )
    {
        __debugbreak();
    }
    if ( turret->lastUpdate > time )
        turret->lastUpdate = 0;
    if ( turret->lastUpdate >= time - 1500 )
        turret->beginFadeTime = time;
}

void __cdecl SatellitePingEnemyPlayer(CompassActor *actor, int time, unsigned int PerkToIgnore)
{
    if ( !actor
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    329,
                    0,
                    "%s",
                    "actor") )
    {
        __debugbreak();
    }
    if ( actor->lastUpdate > time )
        actor->lastUpdate = 0;
    if ( actor->lastUpdate >= time - 1500 && (PerkToIgnore == 52 || !BG_HasPerk(actor->perks, PerkToIgnore)) )
    {
        *(double *)actor->lastEnemyPos = *(double *)actor->lastPos;
        if ( time - actor->beginStaticImageFadeTime > compassStaticImageUpdateTime->current.integer )
        {
            actor->beginStaticImageFadeTime = time;
            *(double *)actor->satelliteEnemyPos = *(double *)actor->lastPos;
            actor->beginSatelliteFadeTime = time;
        }
    }
}

void __cdecl CG_CompassUpdateVehicleInfo(int localClientNum, int entityIndex)
{
    CompassVehicle *Vehicle; // eax
    const vehicle_info_t *info; // [esp+10h] [ebp-18h]
    team_t team; // [esp+1Ch] [ebp-Ch]
    cg_s *cgameGlob; // [esp+20h] [ebp-8h]
    centity_s *cent; // [esp+24h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    644,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    cent = CG_GetEntity(localClientNum, entityIndex);
    if ( cent->nextState.eType != 14
        && cent->nextState.eType != 13
        && (cent->nextState.eType != 6 || ((*((unsigned int *)cent + 201) >> 18) & 1) == 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    646,
                    0,
                    "%s",
                    "cent->nextState.eType == ET_VEHICLE || cent->nextState.eType == ET_PLANE || ( cent->nextState.eType == ET_SCRI"
                    "PTMOVER && cent->bIsPlaneOnRadar == true)") )
    {
        __debugbreak();
    }
    info = CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
    if ( (cent->nextState.lerp.eFlags & 0x20) == 0 )
    {
        team = (team_t)(cent->nextState.faction.iHeadIconTeam & 3);
        if ( (info->addToCompassEnemy || team == cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].team)
            && (info->addToCompass
             || (cgameGlob->nextSnap->ps.eFlags & 0x4000) != 0
             && cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].infoValid
             && cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].attachedVehEntNum == cent->nextState.number) )
        {
            Vehicle = GetVehicle(localClientNum, entityIndex);
            Vehicle->lastUpdate = cgameGlob->time;
            *(double *)Vehicle->lastPos = *(double *)cent->pose.origin;
            Vehicle->lastYaw = cent->pose.angles[1];
            Vehicle->team = team;
            Vehicle->ownerIndex = (int)cent->nextState.faction.iHeadIconTeam >> 2;
        }
    }
}

CompassVehicle *__cdecl GetVehicle(int localClientNum, int entityNum)
{
    CompassVehicle *v3; // edx
    int veh; // [esp+4h] [ebp-8h]
    int veha; // [esp+4h] [ebp-8h]
    int vehb; // [esp+4h] [ebp-8h]
    int oldest; // [esp+8h] [ebp-4h]

    for ( veh = 0; veh < 16; ++veh )
    {
        if ( s_compassVehicles[localClientNum][veh].entityNum == entityNum )
            return &s_compassVehicles[localClientNum][veh];
    }
    for ( veha = 0; veha < 16 && s_compassVehicles[localClientNum][veha].lastUpdate; ++veha )
        ;
    if ( veha >= 16 )
    {
        oldest = 0;
        for ( vehb = 1; vehb < 16; ++vehb )
        {
            if ( s_compassVehicles[localClientNum][vehb].lastUpdate < s_compassVehicles[localClientNum][oldest].lastUpdate )
                oldest = vehb;
        }
        veha = oldest;
    }
    v3 = &s_compassVehicles[localClientNum][veha];
    v3->entityNum = 0;
    v3->lastUpdate = 0;
    v3->lastPos[0] = 0.0;
    v3->lastPos[1] = 0.0;
    v3->lastYaw = 0.0;
    v3->team = TEAM_FREE;
    v3->ownerIndex = 0;
    v3->entityNum = entityNum;
    return v3;
}

void __cdecl CG_CompassUpdateHelicopterInfo(int localClientNum, int entityIndex)
{
    CompassHelicopter *Helicopter; // eax
    cg_s *cgameGlob; // [esp+10h] [ebp-Ch]
    centity_s *cent; // [esp+14h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    678,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    cent = CG_GetEntity(localClientNum, entityIndex);
    if ( cent->nextState.eType != 12
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    680,
                    0,
                    "%s",
                    "cent->nextState.eType == ET_HELICOPTER") )
    {
        __debugbreak();
    }
    Helicopter = GetHelicopter(localClientNum, entityIndex);
    Helicopter->lastUpdate = cgameGlob->time;
    *(double *)Helicopter->lastPos = *(double *)cent->pose.origin;
    Helicopter->lastYaw = cent->pose.angles[1];
    Helicopter->team = (team_t)(cent->nextState.faction.iHeadIconTeam & 3);
    Helicopter->ownerIndex = (int)cent->nextState.faction.iHeadIconTeam >> 2;
}

CompassHelicopter *__cdecl GetHelicopter(int localClientNum, int entityNum)
{
    CompassHelicopter *v3; // edx
    int copter; // [esp+4h] [ebp-8h]
    int coptera; // [esp+4h] [ebp-8h]
    int copterb; // [esp+4h] [ebp-8h]
    int oldest; // [esp+8h] [ebp-4h]

    for ( copter = 0; copter < 8; ++copter )
    {
        if ( s_compassHelicopter[localClientNum][copter].entityNum == entityNum )
            return &s_compassHelicopter[localClientNum][copter];
    }
    for ( coptera = 0; coptera < 8 && s_compassHelicopter[localClientNum][coptera].lastUpdate; ++coptera )
        ;
    if ( coptera >= 8 )
    {
        oldest = 0;
        for ( copterb = 1; copterb < 8; ++copterb )
        {
            if ( s_compassHelicopter[localClientNum][copterb].lastUpdate < s_compassHelicopter[localClientNum][oldest].lastUpdate )
                oldest = copterb;
        }
        coptera = oldest;
    }
    v3 = &s_compassHelicopter[localClientNum][coptera];
    v3->entityNum = 0;
    v3->lastUpdate = 0;
    v3->lastPos[0] = 0.0;
    v3->lastPos[1] = 0.0;
    v3->lastYaw = 0.0;
    v3->team = TEAM_FREE;
    v3->ownerIndex = 0;
    v3->entityNum = entityNum;
    return v3;
}

void __cdecl CG_CompassUpdateGuidedMissileInfo(int localClientNum, int entityIndex)
{
    CompassGuidedMissile *GuidedMissile; // eax
    cg_s *cgameGlob; // [esp+10h] [ebp-Ch]
    centity_s *cent; // [esp+14h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    714,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    cent = CG_GetEntity(localClientNum, entityIndex);
    if ( cent->nextState.eType != 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    716,
                    0,
                    "%s",
                    "cent->nextState.eType == ET_MISSILE") )
    {
        __debugbreak();
    }
    GuidedMissile = GetGuidedMissile(localClientNum, entityIndex);
    GuidedMissile->lastUpdate = cgameGlob->time;
    *(double *)GuidedMissile->lastPos = *(double *)cent->pose.origin;
    GuidedMissile->lastYaw = cent->pose.angles[1];
    GuidedMissile->team = (team_t)(cent->nextState.faction.iHeadIconTeam & 3);
    GuidedMissile->ownerIndex = (int)cent->nextState.faction.iHeadIconTeam >> 2;
}

CompassGuidedMissile *__cdecl GetGuidedMissile(int localClientNum, int entityNum)
{
    CompassGuidedMissile *v3; // edx
    int missile; // [esp+4h] [ebp-8h]
    int missilea; // [esp+4h] [ebp-8h]
    int missileb; // [esp+4h] [ebp-8h]
    int oldest; // [esp+8h] [ebp-4h]

    for ( missile = 0; missile < 32; ++missile )
    {
        if ( s_compassGuidedMissiles[localClientNum][missile].entityNum == entityNum )
            return &s_compassGuidedMissiles[localClientNum][missile];
    }
    for ( missilea = 0; missilea < 32 && s_compassGuidedMissiles[localClientNum][missilea].lastUpdate; ++missilea )
        ;
    if ( missilea >= 32 )
    {
        oldest = 0;
        for ( missileb = 1; missileb < 32; ++missileb )
        {
            if ( s_compassGuidedMissiles[localClientNum][missileb].lastUpdate < s_compassGuidedMissiles[localClientNum][oldest].lastUpdate )
                oldest = missileb;
        }
        missilea = oldest;
    }
    v3 = &s_compassGuidedMissiles[localClientNum][missilea];
    v3->entityNum = 0;
    v3->lastUpdate = 0;
    v3->lastPos[0] = 0.0;
    v3->lastPos[1] = 0.0;
    v3->lastYaw = 0.0;
    v3->team = TEAM_FREE;
    v3->ownerIndex = 0;
    v3->entityNum = entityNum;
    return v3;
}

void __cdecl CG_CompassUpdateVehicleOccupantInfo(int localClientNum, int entityIndex)
{
    bool v2; // [esp+4h] [ebp-40h]
    clientInfo_t *localClientInfo; // [esp+20h] [ebp-24h]
    const vehicle_info_t *info; // [esp+24h] [ebp-20h]
    cg_s *cgameGlob; // [esp+28h] [ebp-1Ch]
    unsigned int playerTeam; // [esp+2Ch] [ebp-18h]
    centity_s *cent; // [esp+30h] [ebp-14h]
    CompassActor *actor; // [esp+38h] [ebp-Ch]
    int i; // [esp+3Ch] [ebp-8h]
    int clientNum; // [esp+40h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    739,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    cent = CG_GetEntity(localClientNum, entityIndex);
    if ( cent->nextState.eType != 14
        && cent->nextState.eType != 12
        && cent->nextState.eType != 13
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    741,
                    0,
                    "%s",
                    "cent->nextState.eType == ET_VEHICLE || cent->nextState.eType == ET_HELICOPTER || cent->nextState.eType == ET_PLANE") )
    {
        __debugbreak();
    }
    if ( !cent
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    743,
                    0,
                    "%s",
                    "cent") )
    {
        __debugbreak();
    }
    if ( cent->nextState.eType != 16 && cent->nextState.eType != 12 )
    {
        if ( cent->nextState.eType != 14
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                        746,
                        0,
                        "%s",
                        "cent->nextState.eType == ET_VEHICLE") )
        {
            __debugbreak();
        }
        info = CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
        if ( !info
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                        749,
                        0,
                        "%s",
                        "info") )
        {
            __debugbreak();
        }
        if ( !info->remoteControl )
        {
            clientNum = -1;
            for ( i = 0; i < com_maxclients->current.integer; ++i )
            {
                if ( cgameGlob->bgs.clientinfo[i].infoValid
                    && cgameGlob->bgs.clientinfo[i].attachedVehEntNum == cent->nextState.number
                    && !cgameGlob->bgs.clientinfo[i].attachedVehSeat )
                {
                    clientNum = i;
                    break;
                }
            }
            if ( clientNum != -1 && i < com_maxclients->current.integer )
            {
                localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum];
                if ( cgameGlob->nextSnap->ps.clientNum != clientNum )
                {
                    playerTeam = cgameGlob->bgs.clientinfo[clientNum].team;
                    if ( playerTeam != 3 )
                    {
                        if ( playerTeam > 2
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                                        781,
                                        0,
                                        "%s",
                                        "playerTeam == TEAM_ALLIES || playerTeam == TEAM_AXIS || playerTeam == TEAM_FREE") )
                        {
                            __debugbreak();
                        }
                        actor = &s_compassActors[localClientNum][clientNum];
                        actor->lastUpdate = cgameGlob->time;
                        v2 = localClientInfo->team == TEAM_FREE || playerTeam != localClientInfo->team;
                        actor->enemy = v2;
                        actor->perks[0] = cgameGlob->bgs.clientinfo[clientNum].perks[0];
                        actor->perks[1] = cgameGlob->bgs.clientinfo[clientNum].perks[1];
                        actor->inVehicle = 1;
                        if ( actor->enemy )
                        {
                            ActorUpdatePos(localClientNum, actor, cent->pose.origin, clientNum);
                            actor->lastEnemyPos[0] = cent->pose.origin[0];
                            actor->lastEnemyPos[1] = cent->pose.origin[1];
                        }
                    }
                }
            }
        }
    }
}

void __cdecl ActorUpdatePos(int localClientNum, CompassActor *actor, const float *newPos, int actorClientIndex)
{
    cg_s *cgameGlob; // [esp+18h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !actor
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    406,
                    0,
                    "%s",
                    "actor") )
    {
        __debugbreak();
    }
    if ( actorClientIndex >= 32
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    407,
                    0,
                    "%s",
                    "actorClientIndex < MAX_CLIENTS") )
    {
        __debugbreak();
    }
    if ( actor->enemy && !cgameGlob->inKillCam )
    {
        if ( cgameGlob->predictedPlayerState.spyplaneTypeEnabled
            && actor->lastUpdate > cgameGlob->time - 1500
            && DoesMovementCrossRadar(cgameGlob, cgameGlob->radarProgress, actor->lastPos, newPos) )
        {
            RadarPingEnemyPlayer(actor, cgameGlob->time, 25);
        }
        if (cgameGlob->predictedPlayerState.satelliteTypeEnabled && actor->lastUpdate > cgameGlob->time - 1500)
            SatellitePingEnemyPlayer(
                actor,
                cgameGlob->time,
                perk_blackbirdShowsGpsJammer->current.enabled ? 52 : 25);
//                cgameGlob->predictedPlayerState.satelliteTypeEnabled == 2);
        if ( CanLocalPlayerHearActorFootsteps(localClientNum, newPos, actorClientIndex) )
            RadarPingEnemyPlayer(actor, cgameGlob->time, 38);
        if ( DoesMovementCrossLocalRadar(cgameGlob->localRadarPos, cgameGlob->localRadarProgress, actor->lastPos, newPos) )
            RadarPingEnemyPlayer(actor, cgameGlob->time, 33);
    }
    *(double *)actor->lastPos = *(double *)newPos;
}

bool __cdecl DoesMovementCrossRadar(cg_s *cgameGlob, float radarProgress, const float *p1, const float *p2)
{
    float radarLine[3]; // [esp+10h] [ebp-14h] BYREF
    float v1; // [esp+20h] [ebp-4h]

    if ( !cgameGlob
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    257,
                    0,
                    "%s",
                    "cgameGlob") )
    {
        __debugbreak();
    }
    GetRadarLine(cgameGlob, radarProgress, radarLine);
    v1 = (float)((float)(radarLine[0] * *p1) + (float)(radarLine[1] * p1[1])) - radarLine[2];
    return v1 < 0.0 != (float)((float)((float)(radarLine[0] * *p2) + (float)(radarLine[1] * p2[1])) - radarLine[2]) < 0.0;
}

void __cdecl GetRadarLine(cg_s *cgameGlob, float radarProgress, float *line)
{
    float margin; // [esp+10h] [ebp-4h]

    if ( !cgameGlob
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    233,
                    0,
                    "%s",
                    "cgameGlob") )
    {
        __debugbreak();
    }
    margin = GetRadarLineMargin(cgameGlob);
    line[0] = cgameGlob->compassNorth[1];
    line[1] = -cgameGlob->compassNorth[0];
    //*((unsigned int *)line + 1) = LODWORD(cgameGlob->compassNorth[0]) ^ _mask__NegFloat_;

    line[2] = (float)((float)((float)(cgameGlob->compassMapUpperLeft[0] * *line)
                                                    + (float)(cgameGlob->compassMapUpperLeft[1] * line[1]))
                                    + (float)((float)((float)(2.0 * margin) + cgameGlob->compassMapWorldSize[0]) * radarProgress))
                    - margin;
}

double __cdecl GetRadarLineMargin(cg_s *cgameGlob)
{
    float v2; // [esp+0h] [ebp-10h]

    if ( !cgameGlob
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    203,
                    0,
                    "%s",
                    "cgameGlob") )
    {
        __debugbreak();
    }
    if ( cgameGlob->compassMapWorldSize[0] == 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    204,
                    0,
                    "%s",
                    "cgameGlob->compassMapWorldSize[0]") )
    {
        __debugbreak();
    }
    if ( (float)((float)(compassMaxRange->current.value * SQRT2_0)
                         + (float)(compassRadarLineThickness->current.value * compassMaxRange->current.value)) <= (float)(cg_hudMapRadarLineThickness->current.value * cgameGlob->compassMapWorldSize[0]) )
        v2 = cg_hudMapRadarLineThickness->current.value * cgameGlob->compassMapWorldSize[0];
    else
        v2 = (float)(compassMaxRange->current.value * SQRT2_0)
             + (float)(compassRadarLineThickness->current.value * compassMaxRange->current.value);
    return (float)(0.5 * v2);
}

bool __cdecl DoesMovementCrossLocalRadar(
                const float *radarPos,
                float localRadarProgress,
                const float *p1,
                const float *p2)
{
    float v5; // [esp+10h] [ebp-18h]
    float v6; // [esp+18h] [ebp-10h]
    float radius; // [esp+24h] [ebp-4h]

    radius = GetLocalRadarRadius(localRadarProgress);
    v6 = p1[1] - radarPos[1];
    v5 = p2[1] - radarPos[1];
    return (float)(radius * radius) > (float)((float)((float)(*p1 - *radarPos) * (float)(*p1 - *radarPos))
                                                                                    + (float)(v6 * v6)) != (float)(radius * radius) > (float)((float)((float)(*p2 - *radarPos) * (float)(*p2 - *radarPos))
                                                                                                                                                                                                    + (float)(v5 * v5));
}

double __cdecl GetLocalRadarRadius(float radarProgress)
{
    return compassLocalRadarRadius->current.value * radarProgress;
}

bool __cdecl CanLocalPlayerHearActorFootsteps(int localClientNum, const float *actorPos, int actorClientIndex)
{
    float delta_4; // [esp+3Ch] [ebp-44h]
    cg_s *cgameGlob; // [esp+44h] [ebp-3Ch]
    const centity_s *cent; // [esp+48h] [ebp-38h]
    const snapshot_s *nextSnap; // [esp+4Ch] [ebp-34h]
    const snapshot_s *snap; // [esp+58h] [ebp-28h]
    float currentPos[3]; // [esp+5Ch] [ebp-24h] BYREF
    const playerState_s *ps; // [esp+68h] [ebp-18h]
    float nextPos[3]; // [esp+6Ch] [ebp-14h] BYREF
    float deltaTime; // [esp+78h] [ebp-8h]
    float xyspeedSq; // [esp+7Ch] [ebp-4h]

    if ( !compassEnemyFootstepEnabled->current.enabled )
        return 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    if ( cgameGlob->predictedPlayerState.pm_type )
        return 0;
    if ( (cgameGlob->bgs.clientinfo[actorClientIndex].perks[1] & 0x40) != 0 )
        return 0;
    delta_4 = actorPos[1] - ps->origin[1];
    if ( (float)((float)((float)(*actorPos - ps->origin[0]) * (float)(*actorPos - ps->origin[0]))
                         + (float)(delta_4 * delta_4)) > (float)(compassEnemyFootstepMaxRange->current.value
                                                                                                     * compassEnemyFootstepMaxRange->current.value) )
        return 0;
    if ( fabs(actorPos[2] - ps->origin[2]) > compassEnemyFootstepMaxZ->current.value )
        return 0;
    snap = cgameGlob->snap;
    nextSnap = cgameGlob->nextSnap;
    deltaTime = (float)(nextSnap->serverTime - snap->serverTime) * 0.001;
    if ( deltaTime <= 0.0 )
        return 0;
    cent = CG_GetEntity(localClientNum, actorClientIndex);
    BG_EvaluateTrajectory(&cent->currentState.pos, snap->serverTime, currentPos);
    BG_EvaluateTrajectory(&cent->nextState.lerp.pos, nextSnap->serverTime, nextPos);
    xyspeedSq = (float)((float)((float)(1.0 / deltaTime) * (float)(nextPos[0] - currentPos[0]))
                                        * (float)((float)(1.0 / deltaTime) * (float)(nextPos[0] - currentPos[0])))
                        + (float)((float)((float)(1.0 / deltaTime) * (float)(nextPos[1] - currentPos[1]))
                                        * (float)((float)(1.0 / deltaTime) * (float)(nextPos[1] - currentPos[1])));
    return xyspeedSq >= (float)(compassEnemyFootstepMinSpeed->current.value * compassEnemyFootstepMinSpeed->current.value);
}

void __cdecl CG_CompassUpdateDogInfo(int localClientNum, int entityIndex)
{
    CompassDogs *dog; // [esp+10h] [ebp-Ch]
    cg_s *cgameGlob; // [esp+14h] [ebp-8h]
    centity_s *cent; // [esp+18h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    804,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    cent = CG_GetEntity(localClientNum, entityIndex);
    if ( cent->nextState.eType != 17
        && cent->nextState.eType != 19
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    806,
                    0,
                    "%s",
                    "cent->nextState.eType == ET_ACTOR || cent->nextState.eType == ET_ACTOR_CORPSE") )
    {
        __debugbreak();
    }
    dog = GetDogs(localClientNum, entityIndex, cgameGlob->time);
    dog->lastUpdate = cgameGlob->time;
    *(double *)dog->lastPos = *(double *)cent->pose.origin;
    dog->lastYaw = cent->pose.angles[1];
    dog->team = (team_t)(cent->nextState.lerp.u.actor.team & 3);
    dog->ownerIndex = cent->nextState.lerp.u.actor.team >> 2;
}

int lastUpdateTime;
CompassDogs *__cdecl GetDogs(int localClientNum, int entityNum, int time)
{
    CompassDogs *v4; // ecx
    int dog; // [esp+4h] [ebp-8h]
    int doga; // [esp+4h] [ebp-8h]
    int dogb; // [esp+4h] [ebp-8h]
    int oldest; // [esp+8h] [ebp-4h]

    if ( time - lastUpdateTime > 1000 )
        memset(s_compassDogs, 0, sizeof(s_compassDogs));
    lastUpdateTime = time;
    for ( dog = 0; dog < 8; ++dog )
    {
        if ( s_compassDogs[localClientNum][dog].entityNum == entityNum )
            return &s_compassDogs[localClientNum][dog];
    }
    for ( doga = 0; doga < 8 && s_compassDogs[localClientNum][doga].lastUpdate; ++doga )
        ;
    if ( doga >= 8 )
    {
        oldest = 0;
        for ( dogb = 1; dogb < 8; ++dogb )
        {
            if ( s_compassDogs[localClientNum][dogb].lastUpdate < s_compassDogs[localClientNum][oldest].lastUpdate )
                oldest = dogb;
        }
        doga = oldest;
    }
    v4 = &s_compassDogs[localClientNum][doga];
    v4->entityNum = 0;
    v4->lastUpdate = 0;
    v4->lastPos[0] = 0.0;
    v4->lastPos[1] = 0.0;
    v4->lastYaw = 0.0;
    v4->team = TEAM_FREE;
    v4->ownerIndex = 0;
    *(unsigned int *)&v4->enemy = 0;
    v4->entityNum = entityNum;
    return v4;
}

void __cdecl CG_CompassUpdateTurretInfo(int localClientNum, int entityIndex)
{
    CompassTurrets *turret; // [esp+14h] [ebp-Ch]
    cg_s *cgameGlob; // [esp+18h] [ebp-8h]
    centity_s *cent; // [esp+1Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    823,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    cent = CG_GetEntity(localClientNum, entityIndex);
    if ( cent->nextState.eType != 11
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    826,
                    0,
                    "%s",
                    "cent->nextState.eType == ET_MG42") )
    {
        __debugbreak();
    }
    turret = GetCompassTurrets(localClientNum, entityIndex, cgameGlob->time);
    turret->lastUpdate = cgameGlob->time;
    *(double *)turret->lastPos = *(double *)cent->pose.origin;
    if ( (cent->nextState.lerp.u.turret.flags & 8) != 0 )
        turret->lastYaw = cent->pose.angles[1];
    else
        turret->lastYaw = AngleNormalize360(cent->pose.angles[1] + cent->nextState.lerp.u.turret.gunAngles[1]);
    turret->team = (team_t)(cent->nextState.faction.iHeadIconTeam & 3);
    turret->ownerIndex = (int)cent->nextState.faction.iHeadIconTeam >> 2;
    turret->firing = (cent->nextState.lerp.u.turret.flags & 4) != 0;
}

int lastUpdateTime_0;
CompassTurrets *__cdecl GetCompassTurrets(int localClientNum, int entityNum, int time)
{
    CompassTurrets *v4; // ecx
    int turret; // [esp+4h] [ebp-8h]
    int turreta; // [esp+4h] [ebp-8h]
    int turretb; // [esp+4h] [ebp-8h]
    int oldest; // [esp+8h] [ebp-4h]

    if ( time - lastUpdateTime_0 > 1000 )
        memset(s_compassDogs, 0, sizeof(s_compassDogs));
    lastUpdateTime_0 = time;
    for ( turret = 0; turret < 32; ++turret )
    {
        if ( s_compassTurrets[localClientNum][turret].entityNum == entityNum )
            return &s_compassTurrets[localClientNum][turret];
    }
    for ( turreta = 0; turreta < 32 && s_compassTurrets[localClientNum][turreta].lastUpdate; ++turreta )
        ;
    if ( turreta >= 32 )
    {
        oldest = 0;
        for ( turretb = 1; turretb < 32; ++turretb )
        {
            if ( s_compassTurrets[localClientNum][turretb].lastUpdate < s_compassTurrets[localClientNum][oldest].lastUpdate )
                oldest = turretb;
        }
        turreta = oldest;
    }
    v4 = &s_compassTurrets[localClientNum][turreta];
    v4->entityNum = 0;
    v4->lastUpdate = 0;
    v4->beginFadeTime = 0;
    v4->lastPos[0] = 0.0;
    v4->lastPos[1] = 0.0;
    v4->lastYaw = 0.0;
    v4->team = TEAM_FREE;
    v4->ownerIndex = 0;
    *(unsigned int *)&v4->enemy = 0;
    v4->entityNum = entityNum;
    return v4;
}

void __cdecl CG_CompassRadarPingEnemyTurrets(
                int localClientNum,
                float oldRadarProgress,
                float newRadarProgress,
                float oldLocalRadarProgress,
                float newLocalRadarProgress)
{
    bool v5; // [esp+Ch] [ebp-40h]
    CompassTurrets *turret; // [esp+1Ch] [ebp-30h]
    clientInfo_t *localClientInfo; // [esp+20h] [ebp-2Ch]
    float localRadarRadius2; // [esp+24h] [ebp-28h]
    cg_s *cgameGlob; // [esp+28h] [ebp-24h]
    float localRadarRadius1; // [esp+2Ch] [ebp-20h]
    int turretIndex; // [esp+30h] [ebp-1Ch]
    float radarLine1[3]; // [esp+34h] [ebp-18h] BYREF
    float radarLine2[3]; // [esp+40h] [ebp-Ch] BYREF

    if ( oldRadarProgress <= newRadarProgress )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                        859,
                        0,
                        "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        cgameGlob->nextSnap->ps.clientNum,
                        32) )
        {
            __debugbreak();
        }
        localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum];
        GetRadarLine(cgameGlob, oldRadarProgress, radarLine1);
        GetRadarLine(cgameGlob, newRadarProgress, radarLine2);
        localRadarRadius1 = GetLocalRadarRadius(oldLocalRadarProgress);
        localRadarRadius2 = GetLocalRadarRadius(newLocalRadarProgress);
        turret = s_compassTurrets[localClientNum];
        for ( turretIndex = 0; turretIndex < 32; ++turretIndex )
        {
            CG_GetEntity(localClientNum, turretIndex);
            v5 = turret->team == TEAM_FREE || turret->team != localClientInfo->team;
            turret->enemy = v5;
            if ( turret->enemy
                && (DoLinesSurroundPoint(cgameGlob, radarLine1, radarLine2, turret->lastPos)
                 || DoRadiiSurroundPoint(cgameGlob->localRadarPos, localRadarRadius1, localRadarRadius2, turret->lastPos)) )
            {
                RadarPingEnemyTurrets(turret, cgameGlob->time);
            }
            ++turret;
        }
    }
}

bool __cdecl DoLinesSurroundPoint(cg_s *cgameGlob, float *radarLine1, float *radarLine2, float *pos)
{
    if ( !cgameGlob
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    281,
                    0,
                    "%s",
                    "cgameGlob") )
    {
        __debugbreak();
    }
    return (float)((float)((float)(*radarLine1 * *pos) + (float)(radarLine1[1] * pos[1])) - radarLine1[2]) < 0.0 != (float)((float)((float)(*radarLine2 * *pos) + (float)(radarLine2[1] * pos[1])) - radarLine2[2]) < 0.0;
}

bool __cdecl DoRadiiSurroundPoint(const float *radarPos, float radius1, float radius2, float *pos)
{
    float v6; // [esp+8h] [ebp-8h]
    float distanceSq; // [esp+Ch] [ebp-4h]

    v6 = pos[1] - radarPos[1];
    distanceSq = (float)((float)(*pos - *radarPos) * (float)(*pos - *radarPos)) + (float)(v6 * v6);
    return distanceSq > (float)(radius1 * radius1) && (float)(radius2 * radius2) > distanceSq;
}

void __cdecl CG_CompassRadarPingEnemyPlayers(
                int localClientNum,
                float oldRadarProgress,
                float newRadarProgress,
                float oldLocalRadarProgress,
                float newLocalRadarProgress)
{
    bool v5; // [esp+10h] [ebp-44h]
    centity_s *cent; // [esp+1Ch] [ebp-38h]
    clientInfo_t *localClientInfo; // [esp+20h] [ebp-34h]
    float localRadarRadius2; // [esp+24h] [ebp-30h]
    cg_s *cgameGlob; // [esp+28h] [ebp-2Ch]
    int actorIndex; // [esp+30h] [ebp-24h]
    float localRadarRadius1; // [esp+34h] [ebp-20h]
    float radarLine1[3]; // [esp+38h] [ebp-1Ch] BYREF
    CompassActor *actor; // [esp+44h] [ebp-10h]
    float radarLine2[3]; // [esp+48h] [ebp-Ch] BYREF

    if ( oldRadarProgress <= newRadarProgress )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                        903,
                        0,
                        "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        cgameGlob->nextSnap->ps.clientNum,
                        32) )
        {
            __debugbreak();
        }
        localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum];
        GetRadarLine(cgameGlob, oldRadarProgress, radarLine1);
        GetRadarLine(cgameGlob, newRadarProgress, radarLine2);
        localRadarRadius1 = GetLocalRadarRadius(oldLocalRadarProgress);
        localRadarRadius2 = GetLocalRadarRadius(newLocalRadarProgress);
        if ( localClientNum
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                        914,
                        0,
                        "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        localClientNum,
                        1) )
        {
            __debugbreak();
        }
        actor = s_compassActors[localClientNum];
        for ( actorIndex = 0; actorIndex < 32; ++actorIndex )
        {
            if ( actorIndex < com_maxclients->current.integer && actorIndex != cgameGlob->nextSnap->ps.clientNum )
            {
                if ( (unsigned int)actorIndex >= 0x20
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                                925,
                                0,
                                "actorIndex doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                                actorIndex,
                                32) )
                {
                    __debugbreak();
                }
                if ( cgameGlob->bgs.clientinfo[actorIndex].infoValid )
                {
                    if ( (unsigned int)actorIndex >= 0x20
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                                    929,
                                    0,
                                    "actorIndex doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                                    actorIndex,
                                    32) )
                    {
                        __debugbreak();
                    }
                    v5 = localClientInfo->team == TEAM_FREE || cgameGlob->bgs.clientinfo[actorIndex].team != localClientInfo->team;
                    actor->enemy = v5;
                    *(_QWORD *)actor->perks = *(_QWORD *)cgameGlob->bgs.clientinfo[actorIndex].perks;
                    if ( actor->enemy )
                    {
                        cent = CG_GetEntity(localClientNum, actorIndex);
                        if ( ((*((unsigned int *)cent + 201) >> 1) & 1) == 0
                            || cent->nextState.eType == 1 && (cent->nextState.lerp.eFlags & 0x40000) == 0 )
                        {
                            if ( DoLinesSurroundPoint(cgameGlob, radarLine1, radarLine2, actor->lastPos) )
                                RadarPingEnemyPlayer(actor, cgameGlob->time, 25);
                            if ( DoRadiiSurroundPoint(cgameGlob->localRadarPos, localRadarRadius1, localRadarRadius2, actor->lastPos) )
                                RadarPingEnemyPlayer(actor, cgameGlob->time, 33);
                        }
                    }
                }
            }
            ++actor;
        }
    }
}

void __cdecl CG_CompassIncreaseRadarTime(int localClientNum)
{
    unsigned int satelliteTypeEnabled; // ecx
    float v2; // xmm0_4
    float v3; // [esp+14h] [ebp-24h]
    float v4; // [esp+18h] [ebp-20h]
    float oldRadarProgress; // [esp+2Ch] [ebp-Ch]
    cg_s *cgameGlob; // [esp+30h] [ebp-8h]
    float oldLocalRadarProgress; // [esp+34h] [ebp-4h]

    oldRadarProgress = 0.0f;
    oldLocalRadarProgress = 0.0f;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    967,
                    0,
                    "%s",
                    "cgameGlob") )
    {
        __debugbreak();
    }
    if ( cgameGlob->predictedPlayerState.spyplaneTypeEnabled || cgameGlob->predictedPlayerState.satelliteTypeEnabled )
    {
        oldRadarProgress = cgameGlob->radarProgress;
        if ( cgameGlob->predictedPlayerState.satelliteTypeEnabled )
            satelliteTypeEnabled = cgameGlob->predictedPlayerState.satelliteTypeEnabled;
        else
            satelliteTypeEnabled = cgameGlob->predictedPlayerState.spyplaneTypeEnabled;
        cgameGlob->radarType = satelliteTypeEnabled;
        if ( cgameGlob->radarType == 2 )
            v2 = (float)((float)cgameGlob->frametime / (float)(compassRadarUpdateFastTime->current.value * 1000.0))
                 + cgameGlob->radarProgress;
        else
            v2 = (float)((float)cgameGlob->frametime / (float)(compassRadarUpdateTime->current.value * 1000.0))
                 + cgameGlob->radarProgress;
        cgameGlob->radarProgress = v2;
        v4 = floor(cgameGlob->radarProgress);
        cgameGlob->radarProgress = cgameGlob->radarProgress - v4;
    }
    else
    {
        cgameGlob->radarProgress = 0.0f;
    }
    if ( cgameGlob->hasLocalRadar )
    {
        oldLocalRadarProgress = cgameGlob->localRadarProgress;
        cgameGlob->localRadarProgress = (float)((float)cgameGlob->frametime
                                                                                    / (float)(compassLocalRadarUpdateTime->current.value * 1000.0))
                                                                    + oldLocalRadarProgress;
        v3 = floor(cgameGlob->localRadarProgress);
        cgameGlob->localRadarProgress = cgameGlob->localRadarProgress - v3;
        CG_CompassRadarPingEnemyPlayers(
            localClientNum,
            oldRadarProgress,
            cgameGlob->radarProgress,
            oldLocalRadarProgress,
            cgameGlob->localRadarProgress);
    }
    else
    {
        cgameGlob->localRadarProgress = 0.0f;
    }
    if ( cgameGlob->predictedPlayerState.spyplaneTypeEnabled )
    {
        CG_CompassRadarPingEnemyPlayers(
            localClientNum,
            oldRadarProgress,
            cgameGlob->radarProgress,
            oldLocalRadarProgress,
            cgameGlob->localRadarProgress);
        CG_CompassRadarPingEnemyTurrets(
            localClientNum,
            oldRadarProgress,
            cgameGlob->radarProgress,
            oldLocalRadarProgress,
            cgameGlob->localRadarProgress);
    }
    if ( cgameGlob->hasLocalRadar )
    {
        CG_CompassRadarPingEnemyPlayers(
            localClientNum,
            oldRadarProgress,
            cgameGlob->radarProgress,
            oldLocalRadarProgress,
            cgameGlob->localRadarProgress);
        CG_CompassRadarPingEnemyTurrets(
            localClientNum,
            oldRadarProgress,
            cgameGlob->radarProgress,
            oldLocalRadarProgress,
            cgameGlob->localRadarProgress);
    }
}

void __cdecl CG_AddTurretWeaponPingInfo(int localClientNum, const centity_s *cent, const float *origin, int msec)
{
    bool v4; // [esp+0h] [ebp-10h]
    CompassTurrets *turret; // [esp+4h] [ebp-Ch]
    clientInfo_t *localClientInfo; // [esp+8h] [ebp-8h]
    cg_s *cgameGlob; // [esp+Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cent
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    1017,
                    0,
                    "%s",
                    "cent") )
    {
        __debugbreak();
    }
    if ( cent->nextState.eType != 11
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    1018,
                    0,
                    "%s",
                    "cent->nextState.eType == ET_MG42") )
    {
        __debugbreak();
    }
    localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->clientNum];
    turret = GetCompassTurrets(localClientNum, cent->nextState.number, cgameGlob->time);
    v4 = turret->team == TEAM_FREE || turret->team != localClientInfo->team;
    turret->enemy = v4;
    if ( !turret->enemy
        || !IsHardcoreMode(localClientNum)
        || cgameGlob->predictedPlayerState.satelliteTypeEnabled
        || cgameGlob->predictedPlayerState.spyplaneTypeEnabled )
    {
        turret->beginFadeTime = msec + cgameGlob->time;
    }
}

void __cdecl CG_CompassAddWeaponPingInfo(int localClientNum, const centity_s *cent, const float *origin, int msec)
{
    bool v4; // [esp+4h] [ebp-28h]
    bool v5; // [esp+8h] [ebp-24h]
    int playerIndex; // [esp+10h] [ebp-1Ch]
    clientInfo_t *localClientInfo; // [esp+14h] [ebp-18h]
    cg_s *cgameGlob; // [esp+18h] [ebp-14h]
    unsigned int playerTeam; // [esp+1Ch] [ebp-10h]
    CompassActor *actor; // [esp+28h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cent
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    1043,
                    0,
                    "%s",
                    "cent") )
    {
        __debugbreak();
    }
    if ( cent->nextState.eType != 2 )
    {
        if ( cent->nextState.eType != 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                        1046,
                        0,
                        "%s",
                        "cent->nextState.eType == ET_PLAYER") )
        {
            __debugbreak();
        }
        if ( cent->nextState.number >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                        1047,
                        0,
                        "cent->nextState.number doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        cent->nextState.number,
                        32) )
        {
            __debugbreak();
        }
        if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                        1048,
                        0,
                        "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        cgameGlob->nextSnap->ps.clientNum,
                        32) )
        {
            __debugbreak();
        }
        localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum];
        if ( cgameGlob->nextSnap->ps.clientNum != cent->nextState.clientNum )
        {
            playerIndex = cent->nextState.number;
            playerTeam = cgameGlob->bgs.clientinfo[cent->nextState.clientNum].team;
            if ( playerTeam != 3 )
            {
                if ( playerTeam > 2
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                                1060,
                                0,
                                "%s",
                                "playerTeam == TEAM_ALLIES || playerTeam == TEAM_AXIS || playerTeam == TEAM_FREE") )
                {
                    __debugbreak();
                }
                v5 = localClientInfo->team == TEAM_FREE || playerTeam != localClientInfo->team;
                if ( !v5
                    || !IsHardcoreMode(localClientNum)
                    || cgameGlob->predictedPlayerState.satelliteTypeEnabled
                    || cgameGlob->predictedPlayerState.spyplaneTypeEnabled )
                {
                    actor = &s_compassActors[localClientNum][playerIndex];
                    actor->beginFadeTime = msec + cgameGlob->time;
                    v4 = localClientInfo->team == TEAM_FREE || playerTeam != localClientInfo->team;
                    actor->enemy = v4;
                    actor->perks[0] = cgameGlob->bgs.clientinfo[playerIndex].perks[0];
                    actor->perks[1] = cgameGlob->bgs.clientinfo[playerIndex].perks[1];
                    if ( actor->enemy )
                    {
                        ActorUpdatePos(localClientNum, actor, origin, cent->nextState.clientNum);
                        actor->lastEnemyPos[0] = *origin;
                        actor->lastEnemyPos[1] = origin[1];
                    }
                }
            }
        }
    }
}

void __cdecl CG_CompassAddVehicleWeaponPingInfo(
                int localClientNum,
                const centity_s *cent,
                const float *origin,
                int msec)
{
    bool v4; // [esp+4h] [ebp-34h]
    bool v5; // [esp+8h] [ebp-30h]
    clientInfo_t *localClientInfo; // [esp+18h] [ebp-20h]
    cg_s *cgameGlob; // [esp+1Ch] [ebp-1Ch]
    unsigned int playerTeam; // [esp+20h] [ebp-18h]
    CompassActor *actor; // [esp+2Ch] [ebp-Ch]
    int i; // [esp+30h] [ebp-8h]
    int clientNum; // [esp+34h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cent
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    1092,
                    0,
                    "%s",
                    "cent") )
    {
        __debugbreak();
    }
    if ( cent->nextState.eType != 16 )
    {
        if ( cent->nextState.eType != 14
            && cent->nextState.eType != 12
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                        1095,
                        0,
                        "%s",
                        "cent->nextState.eType == ET_VEHICLE || cent->nextState.eType == ET_HELICOPTER") )
        {
            __debugbreak();
        }
        clientNum = -1;
        for ( i = 0; i < com_maxclients->current.integer; ++i )
        {
            if ( cgameGlob->bgs.clientinfo[i].infoValid
                && cgameGlob->bgs.clientinfo[i].attachedVehEntNum == cent->nextState.number
                && !cgameGlob->bgs.clientinfo[i].attachedVehSeat )
            {
                clientNum = i;
                break;
            }
        }
        if ( i < com_maxclients->current.integer && clientNum != -1 )
        {
            localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum];
            if ( cgameGlob->nextSnap->ps.clientNum != clientNum )
            {
                playerTeam = cgameGlob->bgs.clientinfo[clientNum].team;
                if ( playerTeam != 3 )
                {
                    if ( playerTeam > 2
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                                    1124,
                                    0,
                                    "%s",
                                    "playerTeam == TEAM_ALLIES || playerTeam == TEAM_AXIS || playerTeam == TEAM_FREE") )
                    {
                        __debugbreak();
                    }
                    v5 = localClientInfo->team == TEAM_FREE || playerTeam != localClientInfo->team;
                    if ( !v5
                        || !IsHardcoreMode(localClientNum)
                        || cgameGlob->predictedPlayerState.spyplaneTypeEnabled
                        || cgameGlob->predictedPlayerState.satelliteTypeEnabled )
                    {
                        actor = &s_compassActors[localClientNum][clientNum];
                        actor->beginFadeTime = msec + cgameGlob->time;
                        v4 = localClientInfo->team == TEAM_FREE || playerTeam != localClientInfo->team;
                        actor->enemy = v4;
                        actor->perks[0] = cgameGlob->bgs.clientinfo[clientNum].perks[0];
                        actor->perks[1] = cgameGlob->bgs.clientinfo[clientNum].perks[1];
                        if ( actor->enemy )
                        {
                            ActorUpdatePos(localClientNum, actor, origin, clientNum);
                            actor->lastEnemyPos[0] = *origin;
                            actor->lastEnemyPos[1] = origin[1];
                        }
                    }
                }
            }
        }
    }
}

void __cdecl CG_CompassAddFakeFirePingInfo(
                int localClientNum,
                unsigned int ownerNum,
                int entNum,
                const float *origin,
                const float *dir,
                int msec)
{
    bool v6; // [esp+0h] [ebp-2Ch]
    clientInfo_t *localClientInfo; // [esp+8h] [ebp-24h]
    cg_s *cgameGlob; // [esp+Ch] [ebp-20h]
    unsigned int playerTeam; // [esp+10h] [ebp-1Ch]
    float angles[3]; // [esp+1Ch] [ebp-10h] BYREF
    CompassFakeFire *actor; // [esp+28h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( ownerNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    1179,
                    0,
                    "ownerNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    ownerNum,
                    32) )
    {
        __debugbreak();
    }
    if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    1180,
                    0,
                    "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cgameGlob->nextSnap->ps.clientNum,
                    32) )
    {
        __debugbreak();
    }
    localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum];
    playerTeam = cgameGlob->bgs.clientinfo[ownerNum].team;
    if ( playerTeam != 3 )
    {
        if ( playerTeam > 2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                        1190,
                        0,
                        "%s",
                        "playerTeam == TEAM_ALLIES || playerTeam == TEAM_AXIS || playerTeam == TEAM_FREE") )
        {
            __debugbreak();
        }
        v6 = playerTeam != localClientInfo->team
            || localClientInfo->team == TEAM_FREE && ownerNum != cgameGlob->nextSnap->ps.clientNum;
        if ( !v6
            || !IsHardcoreMode(localClientNum)
            || cgameGlob->predictedPlayerState.spyplaneTypeEnabled
            || cgameGlob->predictedPlayerState.satelliteTypeEnabled )
        {
            actor = CG_FindFakeFireActor(localClientNum, ownerNum, entNum);
            actor->entityNum = entNum;
            actor->beginFadeTime = msec + cgameGlob->time;
            actor->enemy = v6;
            actor->pos[0] = *origin;
            actor->pos[1] = origin[1];
            vectoangles(dir, angles);
            actor->yaw = angles[1];
            actor->satellite = cgameGlob->predictedPlayerState.satelliteTypeEnabled != 0;
        }
    }
}

CompassFakeFire *__cdecl CG_FindFakeFireActor(int localClientNum, int playerIndex, int entNum)
{
    int index; // [esp+0h] [ebp-10h]
    CompassFakeFire *fakeFire; // [esp+4h] [ebp-Ch]
    int oldestIndex; // [esp+8h] [ebp-8h]
    int oldestTime; // [esp+Ch] [ebp-4h]

    oldestIndex = 0;
    oldestTime = 0x7FFFFFFF;
    fakeFire = s_compassFakeFire[localClientNum][playerIndex];
    for ( index = 0; index < 8; ++index )
    {
        if ( fakeFire->entityNum == entNum )
            return fakeFire;
        if ( fakeFire->beginFadeTime < oldestTime )
        {
            oldestIndex = index;
            oldestTime = fakeFire->beginFadeTime;
        }
        ++fakeFire;
    }
    return &s_compassFakeFire[localClientNum][playerIndex][oldestIndex];
}

void __cdecl CG_AddEnemyScrambler(int localClientNum, centity_s *scramblerEnt)
{
    CG_GetLocalClientGlobals(localClientNum)->closestEnemyScrambler = scramblerEnt;
}

void __cdecl CG_ClearNearestEnemyScrambler(int localClientNum)
{
    CG_GetLocalClientGlobals(localClientNum)->closestEnemyScrambler = 0;
}

void __cdecl CG_AddFriendlyScrambler(int localClientNum, float x, float y, int handle)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-10h]
    int oldest_scrambler_time; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    int oldest_scrambler; // [esp+Ch] [ebp-4h]

    oldest_scrambler = 0;
    oldest_scrambler_time = 0x7FFFFFFF;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    1237,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    for ( i = 0; i < 16; ++i )
    {
        if ( oldest_scrambler_time > cgameGlob->compassScrambler[i].startTime )
        {
            oldest_scrambler_time = cgameGlob->compassScrambler[i].startTime;
            oldest_scrambler = i;
        }
    }
    cgameGlob->compassScrambler[oldest_scrambler].startTime = cgameGlob->time;
    cgameGlob->compassScrambler[oldest_scrambler].handle = handle;
    cgameGlob->compassScrambler[oldest_scrambler].pos[0] = x;
    cgameGlob->compassScrambler[oldest_scrambler].pos[1] = y;
}

void __cdecl CG_RemoveFriendlyScrambler(int localClientNum, int handle)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]
    int scamblerIndex; // [esp+4h] [ebp-4h]

    scamblerIndex = 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    1260,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    while ( scamblerIndex < 16 )
    {
        if ( handle == cgameGlob->compassScrambler[scamblerIndex].handle )
        {
            cgameGlob->compassScrambler[scamblerIndex].startTime = 0;
            cgameGlob->compassScrambler[scamblerIndex].handle = 0;
            return;
        }
        ++scamblerIndex;
    }
}

void __cdecl CG_RemoveAllFriendlyScramblers(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]
    int scamblerIndex; // [esp+4h] [ebp-4h]

    scamblerIndex = 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    while ( scamblerIndex < 16 )
    {
        cgameGlob->compassScrambler[scamblerIndex].startTime = 0;
        cgameGlob->compassScrambler[scamblerIndex++].handle = 0;
    }
}

void __cdecl CG_CompassUpdateActors(int localClientNum)
{
    int v1; // eax
    CompassActor *v2; // ecx
    cg_s *v3; // edx
    int v4; // eax
    CompassActor *v5; // ecx
    cg_s *v6; // edx
    bool v7; // [esp+4h] [ebp-58h]
    bool v8; // [esp+8h] [ebp-54h]
    bool v9; // [esp+Ch] [ebp-50h]
    clientInfo_t *localClientInfo; // [esp+1Ch] [ebp-40h]
    float newPos[2]; // [esp+20h] [ebp-3Ch] BYREF
    float clientPos[3]; // [esp+28h] [ebp-34h]
    int team; // [esp+34h] [ebp-28h]
    cg_s *cgameGlob; // [esp+38h] [ebp-24h]
    const centity_s *cent; // [esp+3Ch] [ebp-20h]
    int playerTeam; // [esp+40h] [ebp-1Ch]
    float posDelta[2]; // [esp+44h] [ebp-18h]
    int index; // [esp+4Ch] [ebp-10h]
    int clientIndex; // [esp+50h] [ebp-Ch]
    CompassActor *actor; // [esp+54h] [ebp-8h]
    int num; // [esp+58h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    1325,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    1327,
                    0,
                    "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cgameGlob->nextSnap->ps.clientNum,
                    32) )
    {
        __debugbreak();
    }
    localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum];
    if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    1330,
                    0,
                    "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cgameGlob->nextSnap->ps.clientNum,
                    32) )
    {
        __debugbreak();
    }
    if ( cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].infoValid )
    {
        if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                        1333,
                        0,
                        "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        cgameGlob->nextSnap->ps.clientNum,
                        32) )
        {
            __debugbreak();
        }
        team = cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].team;
        if ( team != 3 )
        {
            clientIndex = localClientNum;
            for ( num = 0; num < cgameGlob->nextSnap->numEntities; ++num )
            {
                index = cgameGlob->nextSnap->entities[num].number;
                if ( index < 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                                1346,
                                0,
                                "%s\n\t(index) = %i",
                                "(index >= 0)",
                                index) )
                {
                    __debugbreak();
                }
                if ( index < com_maxclients->current.integer )
                {
                    if ( (unsigned int)index >= 0x20
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                                    1351,
                                    0,
                                    "index doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                                    index,
                                    32) )
                    {
                        __debugbreak();
                    }
                    if ( cgameGlob->bgs.clientinfo[index].infoValid )
                    {
                        if ( (unsigned int)index >= 0x20
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                                        1355,
                                        0,
                                        "index doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                                        index,
                                        32) )
                        {
                            __debugbreak();
                        }
                        playerTeam = cgameGlob->bgs.clientinfo[index].team;
                        if ( clientIndex
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                                        1358,
                                        0,
                                        "clientIndex doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                                        clientIndex,
                                        1) )
                        {
                            __debugbreak();
                        }
                        if ( (unsigned int)index >= 0x20
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                                        1359,
                                        0,
                                        "index doesn't index MAX_COMPASS_ACTORS\n\t%i not in [0, %i)",
                                        index,
                                        32) )
                        {
                            __debugbreak();
                        }
                        actor = &s_compassActors[clientIndex][index];
                        v9 = localClientInfo->team == TEAM_FREE || playerTeam != localClientInfo->team;
                        actor->enemy = v9;
                        v1 = index;
                        v2 = actor;
                        v3 = cgameGlob;
                        actor->perks[0] = cgameGlob->bgs.clientinfo[index].perks[0];
                        v2->perks[1] = v3->bgs.clientinfo[v1].perks[1];
                        actor->needsRevive = cgameGlob->bgs.clientinfo[index].needsRevive;
                        cent = CG_GetEntity(localClientNum, index);
                        if ( cent->nextState.eType == 1 && (cent->nextState.lerp.eFlags & 0x40000) == 0 )
                        {
                            if ( cent->nextState.clientNum != index
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                                            1371,
                                            0,
                                            "%s",
                                            "cent->nextState.clientNum == index") )
                            {
                                __debugbreak();
                            }
                            actor = &s_compassActors[clientIndex][index];
                            ActorUpdatePos(localClientNum, actor, cent->pose.origin, index);
                            actor->lastUpdate = cgameGlob->time;
                            actor->lastYaw = cent->pose.angles[1];
                            v8 = (cent->nextState.lerp.eFlags & 0x4000) != 0 && (cent->nextState.lerp.eFlags2 & 0x10000000) == 0;
                            actor->inVehicle = v8;
                            if ( CL_IsPlayerTalking(localClientNum, index) )
                                actor->beginVoiceFadeTime = cgameGlob->time + 50;
                            if ( (cent->nextState.lerp.eFlags & 0x400000) != 0 && actor->pingTime <= cgameGlob->time )
                                actor->pingTime = cgameGlob->time + 3000;
                        }
                    }
                }
            }
            if ( cgameGlob->nextSnap->ps.iCompassPlayerInfo )
            {
                index = cgameGlob->nextSnap->ps.iCompassPlayerInfo & 0x3F;
                actor = &s_compassActors[clientIndex][index];
                if ( (unsigned int)index >= 0x20
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                                1404,
                                0,
                                "index doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                                index,
                                32) )
                {
                    __debugbreak();
                }
                playerTeam = cgameGlob->bgs.clientinfo[index].team;
                actor = &s_compassActors[clientIndex][index];
                v7 = localClientInfo->team == TEAM_FREE || playerTeam != localClientInfo->team;
                actor->enemy = v7;
                v4 = index;
                v5 = actor;
                v6 = cgameGlob;
                actor->perks[0] = cgameGlob->bgs.clientinfo[index].perks[0];
                v5->perks[1] = v6->bgs.clientinfo[v4].perks[1];
                posDelta[0] = (float)(4 * ((cgameGlob->nextSnap->ps.iCompassPlayerInfo & 0x7FC0) >> 6) - 1020);
                posDelta[1] = (float)(4 * ((cgameGlob->nextSnap->ps.iCompassPlayerInfo & 0xFF8000) >> 15) - 1020);
                clientPos[0] = cgameGlob->refdef.vieworg[0];
                clientPos[1] = cgameGlob->refdef.vieworg[1];
                clientPos[2] = cgameGlob->refdef.vieworg[2];
                newPos[0] = posDelta[0] + clientPos[0];
                newPos[1] = posDelta[1] + clientPos[1];
                ActorUpdatePos(localClientNum, actor, newPos, index);
                actor->lastYaw = (float)((int)(cgameGlob->nextSnap->ps.iCompassPlayerInfo & 0xFF000000) >> 24) * 1.40625;
                actor->lastUpdate = cgameGlob->time;
                actor->inVehicle = 0;
                if ( ((unsigned int)0x800000 & cgameGlob->nextSnap->ps.eFlags) != 0 && actor->pingTime <= cgameGlob->time )
                    actor->pingTime = cgameGlob->time + 3000;
            }
        }
    }
}

void __cdecl CG_CompassDrawFriendlies(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                float *color)
{
    double v5; // st7
    float v6; // [esp+24h] [ebp-C4h]
    float v7; // [esp+28h] [ebp-C0h]
    float v8; // [esp+2Ch] [ebp-BCh]
    int icon; // [esp+44h] [ebp-A4h]
    float yawVector[2]; // [esp+48h] [ebp-A0h] BYREF
    bool clipped; // [esp+53h] [ebp-95h]
    clientInfo_t *localClientInfo; // [esp+54h] [ebp-94h]
    float xy[2]; // [esp+58h] [ebp-90h] BYREF
    float baseColorModdedByComapassFadeOut[4]; // [esp+60h] [ebp-88h] BYREF
    int team; // [esp+70h] [ebp-78h]
    int timeDiff; // [esp+74h] [ebp-74h]
    const ScreenPlacement *scrPlace; // [esp+78h] [ebp-70h]
    cg_s *cgameGlob; // [esp+7Ch] [ebp-6Ch]
    float dist; // [esp+80h] [ebp-68h]
    float compassFadeOutAlpha; // [esp+84h] [ebp-64h]
    float yawTo; // [esp+88h] [ebp-60h]
    float posDelta[2]; // [esp+8Ch] [ebp-5Ch] BYREF
    Material *firingMaterial; // [esp+94h] [ebp-54h]
    float voiceFade; // [esp+98h] [ebp-50h]
    Material *voiceMaterial; // [esp+9Ch] [ebp-4Ch]
    float centerY; // [esp+A0h] [ebp-48h]
    int clientIndex; // [esp+A4h] [ebp-44h]
    CompassActor *actor; // [esp+A8h] [ebp-40h]
    int num; // [esp+ACh] [ebp-3Ch]
    rectDef_s scaledRect; // [esp+B0h] [ebp-38h] BYREF
    float centerX; // [esp+C8h] [ebp-20h]
    float h; // [esp+CCh] [ebp-1Ch] BYREF
    float fadedColor[4]; // [esp+D0h] [ebp-18h] BYREF
    float firingFade; // [esp+E0h] [ebp-8h]
    float w; // [esp+E4h] [ebp-4h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    1461,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    1463,
                    0,
                    "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cgameGlob->nextSnap->ps.clientNum,
                    32) )
    {
        __debugbreak();
    }
    localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum];
    CG_CompassUpYawVector(cgameGlob, yawVector);
    compassFadeOutAlpha = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    v5 = CG_FadeCompassIcons(localClientNum, 0);
    compassFadeOutAlpha = v5 * compassFadeOutAlpha;
    if ( compassFadeOutAlpha != 0.0 )
    {
        if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                        1473,
                        0,
                        "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        cgameGlob->nextSnap->ps.clientNum,
                        32) )
        {
            __debugbreak();
        }
        if ( cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].infoValid )
        {
            if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                            1476,
                            0,
                            "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                            cgameGlob->nextSnap->ps.clientNum,
                            32) )
            {
                __debugbreak();
            }
            team = cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].team;
            if ( team != 3 && team )
            {
                CG_CompassCalcDimensions(
                    compassType,
                    cgameGlob,
                    parentRect,
                    rect,
                    &scaledRect.x,
                    &scaledRect.y,
                    &scaledRect.w,
                    &scaledRect.h);
                centerX = (float)(scaledRect.w * 0.5) + scaledRect.x;
                centerY = (float)(scaledRect.h * 0.5) + scaledRect.y;
                fadedColor[0] = *color;
                fadedColor[1] = color[1];
                fadedColor[2] = color[2];
                fadedColor[3] = color[3];
                scrPlace = &scrPlaceView[localClientNum];
                clientIndex = localClientNum;
                if ( color[3] > compassFadeOutAlpha )
                    color[3] = compassFadeOutAlpha;
                if ( fadedColor[3] > compassFadeOutAlpha )
                    fadedColor[3] = compassFadeOutAlpha;
                actor = s_compassActors[clientIndex];
                num = 0;
                while ( num < 32 )
                {
                    if ( actor->lastUpdate > cgameGlob->time )
                        actor->lastUpdate = 0;
                    if ( actor->lastUpdate >= cgameGlob->time - 1500
                        && !actor->enemy
                        && cgameGlob->nextSnap->ps.clientNum != num
                        && !actor->inVehicle )
                    {
                        if ( fabs(actor->lastPos[0]) > 1.0
                            || fabs(actor->lastPos[1]) > 1.0 )
                        {
                            posDelta[0] = actor->lastPos[0] - cgameGlob->refdef.vieworg[0];
                            posDelta[1] = actor->lastPos[1] - cgameGlob->refdef.vieworg[1];
                            dist = Vec2Length(posDelta);
                            fadedColor[3] = dist;
                            if ( dist <= compassObjectiveMaxRange->current.value )
                            {
                                if ( compassMaxRange->current.value > fadedColor[3] )
                                    fadedColor[3] = compassMaxRange->current.value;
                            }
                            else
                            {
                                fadedColor[3] = compassObjectiveMaxRange->current.value;
                            }
                            fadedColor[3] = fadedColor[3] - compassMaxRange->current.value;
                            fadedColor[3] = fadedColor[3]
                                                        / (float)(compassObjectiveMaxRange->current.value - compassMaxRange->current.value);
                            fadedColor[3] = (float)((float)(compassObjectiveMinAlpha->current.value - 1.0) * fadedColor[3]) + 1.0;
                        }
                        else
                        {
                            fadedColor[3] = (float)((float)(1.0 - compassObjectiveMinAlpha->current.value) * 0.5)
                                                        + compassObjectiveMinAlpha->current.value;
                        }
                        clipped = CG_WorldPosToCompass(
                                                compassType,
                                                cgameGlob,
                                                &scaledRect,
                                                yawVector,
                                                cgameGlob->refdef.vieworg,
                                                actor->lastPos,
                                                0,
                                                xy);
                        if ( !clipped || compassClampIcons->current.enabled )
                        {
                            CalcCompassFriendlySize(compassType, &w, &h);
                            xy[0] = (float)(centerX - (float)(w * 0.5)) + xy[0];
                            xy[1] = (float)(centerY - (float)(h * 0.5)) + xy[1];
                            if ( compassType || !compassRotation->current.enabled )
                                yawTo = AngleNormalize360(cgameGlob->compassNorthYaw - actor->lastYaw);
                            else
                                yawTo = AngleNormalize360(cgameGlob->refdefViewAngles[1] - actor->lastYaw);
                            icon = 0;
                            if ( actor->pingTime > cgameGlob->time )
                            {
                                timeDiff = (actor->pingTime - cgameGlob->time) % 500;
                                icon = timeDiff >= 250;
                            }
                            firingMaterial = 0;
                            firingFade = 0.0f;
                            if ( actor->beginFadeTime
                                && (float)((float)actor->beginFadeTime + (float)(compassSoundPingFadeTime->current.value * 1000.0)) >= (float)cgameGlob->time )
                            {
                                if ( actor->beginFadeTime < cgameGlob->time )
                                    firingFade = 1.0
                                                         - (float)((float)(cgameGlob->time - actor->beginFadeTime)
                                                                         / (float)(compassSoundPingFadeTime->current.value * 1000.0));
                                else
                                    firingFade = 1.0f;
                                firingMaterial = cgMedia.compassping_friendlyfiring;
                            }
                            voiceMaterial = 0;
                            voiceFade = 0.0f;
                            if ( actor->beginVoiceFadeTime
                                && (float)((float)actor->beginVoiceFadeTime + (float)(compassSoundPingFadeTime->current.value * 1000.0)) >= (float)cgameGlob->time )
                            {
                                if ( actor->beginVoiceFadeTime < cgameGlob->time )
                                    voiceFade = 1.0
                                                        - (float)((float)(cgameGlob->time - actor->beginVoiceFadeTime)
                                                                        / (float)(compassSoundPingFadeTime->current.value * 1000.0));
                                else
                                    voiceFade = 1.0f;
                                voiceMaterial = cgMedia.compassping_friendlyyelling;
                            }
                            baseColorModdedByComapassFadeOut[0] = *color;
                            baseColorModdedByComapassFadeOut[1] = color[1];
                            baseColorModdedByComapassFadeOut[2] = color[2];
                            baseColorModdedByComapassFadeOut[3] = color[3];
                            if ( (float)(compassFadeOutAlpha - baseColorModdedByComapassFadeOut[3]) < 0.0 )
                                v8 = compassFadeOutAlpha;
                            else
                                v8 = baseColorModdedByComapassFadeOut[3];
                            baseColorModdedByComapassFadeOut[3] = v8;
                            if ( (float)(compassFadeOutAlpha - firingFade) < 0.0 )
                                v7 = compassFadeOutAlpha;
                            else
                                v7 = firingFade;
                            firingFade = v7;
                            if ( (float)(compassFadeOutAlpha - voiceFade) < 0.0 )
                                v6 = compassFadeOutAlpha;
                            else
                                v6 = voiceFade;
                            voiceFade = v6;
                            if ( actor->needsRevive )
                                icon = 2;
                            if ( icon == 1 )
                            {
                                UI_DrawHandlePic(
                                    scrPlace,
                                    xy[0],
                                    xy[1],
                                    w,
                                    h,
                                    rect->horzAlign,
                                    rect->vertAlign,
                                    fadedColor,
                                    cgMedia.friendMaterials[1]);
                            }
                            else if ( icon == 2 )
                            {
                                CG_DrawRotatedPic(
                                    scrPlace,
                                    xy[0],
                                    xy[1],
                                    w,
                                    h,
                                    rect->horzAlign,
                                    rect->vertAlign,
                                    cgameGlob->compassNorthYaw,
                                    baseColorModdedByComapassFadeOut,
                                    cgMedia.friendMaterials[2]);
                            }
                            else
                            {
                                if ( (!firingMaterial || firingFade != 1.0) && (!voiceMaterial || voiceFade != 1.0) )
                                    CG_DrawRotatedPic(
                                        scrPlace,
                                        xy[0],
                                        xy[1],
                                        w,
                                        h,
                                        rect->horzAlign,
                                        rect->vertAlign,
                                        yawTo,
                                        baseColorModdedByComapassFadeOut,
                                        cgMedia.friendMaterials[icon]);
                                if ( voiceMaterial )
                                {
                                    fadedColor[3] = voiceFade;
                                    CG_DrawRotatedPic(
                                        scrPlace,
                                        xy[0],
                                        xy[1],
                                        w,
                                        h,
                                        rect->horzAlign,
                                        rect->vertAlign,
                                        yawTo,
                                        fadedColor,
                                        voiceMaterial);
                                }
                                if ( firingMaterial )
                                {
                                    fadedColor[3] = firingFade;
                                    CG_DrawRotatedPic(
                                        scrPlace,
                                        xy[0],
                                        xy[1],
                                        w,
                                        h,
                                        rect->horzAlign,
                                        rect->vertAlign,
                                        yawTo,
                                        fadedColor,
                                        firingMaterial);
                                }
                            }
                        }
                    }
                    ++num;
                    ++actor;
                }
            }
        }
    }
}

int compass_jitter = 1000;
void __cdecl CG_AddArtilleryPing(int localClientNum, float *position)
{
    cg_s *cgameGlob; // [esp+10h] [ebp-10h]
    int oldest_shell_time; // [esp+14h] [ebp-Ch]
    int oldest_shell; // [esp+18h] [ebp-8h]
    int i; // [esp+1Ch] [ebp-4h]

    oldest_shell = 0;
    oldest_shell_time = 0x7FFFFFFF;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    1688,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    for ( i = 0; i < 6; ++i )
    {
        if ( cgameGlob->time - s_compassArtillery[localClientNum][i].fireTime > 5000 )
        {
            oldest_shell = i;
            break;
        }
        if ( oldest_shell_time > s_compassArtillery[localClientNum][i].fireTime )
        {
            oldest_shell_time = s_compassArtillery[localClientNum][i].fireTime;
            oldest_shell = i;
        }
    }
    s_compassArtillery[localClientNum][oldest_shell].fireTime = cgameGlob->time;
    s_compassArtillery[localClientNum][oldest_shell].pos[0] = *position;
    s_compassArtillery[localClientNum][oldest_shell].pos[1] = position[1];
    s_compassArtillery[localClientNum][oldest_shell].realPos[0] = *position;
    s_compassArtillery[localClientNum][oldest_shell].realPos[1] = position[1];
    s_compassArtillery[localClientNum][oldest_shell].pos[0] = (float)irand(-compass_jitter, compass_jitter)
                                                                                                                    + s_compassArtillery[localClientNum][oldest_shell].pos[0];
    s_compassArtillery[localClientNum][oldest_shell].pos[1] = (float)irand(-compass_jitter, compass_jitter)
                                                                                                                    + s_compassArtillery[localClientNum][oldest_shell].pos[1];
}

void __cdecl CG_CompassDrawArtilleryIcon(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                float *color)
{
    double ScramblerEnemyAlpha; // st7
    Material *icon; // [esp+8h] [ebp-3Ch]
    float icon_color[4]; // [esp+Ch] [ebp-38h] BYREF
    int isMortar; // [esp+1Ch] [ebp-28h]
    cg_s *cgameGlob; // [esp+20h] [ebp-24h]
    float max_alpha; // [esp+24h] [ebp-20h]
    int i; // [esp+28h] [ebp-1Ch]
    float position[3]; // [esp+2Ch] [ebp-18h] BYREF
    const playerState_s *ps; // [esp+38h] [ebp-Ch]
    float alpha; // [esp+3Ch] [ebp-8h]
    int pingTime; // [esp+40h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    1723,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    i = 0;
    ps = &cgameGlob->predictedPlayerState;
    if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    1728,
                    0,
                    "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cgameGlob->nextSnap->ps.clientNum,
                    32) )
    {
        __debugbreak();
    }
    if ( (ps->artilleryInboundIconLocation & 0x30000) != 0 )
    {
        isMortar = ps->artilleryInboundIconLocation & 0x40000;
        if ( isMortar )
        {
            icon = cgMedia.compass_mortar_enemy;
            pingTime = 3500;
        }
        else
        {
            icon = cgMedia.compass_artillery_enemy;
            pingTime = 5000;
        }
        if ( (ps->artilleryInboundIconLocation & 0x10000) != 0 )
        {
            if ( isMortar )
                icon = cgMedia.compass_mortar_friendly;
            else
                icon = cgMedia.compass_artillery_friendly;
        }
        icon_color[0] = *color;
        icon_color[1] = color[1];
        icon_color[2] = color[2];
        icon_color[3] = color[3];
        max_alpha = color[3];
        alpha = 0.0f;
        for ( i = 0; i < 6; ++i )
        {
            if ( s_compassArtillery[localClientNum][i].fireTime
                && cgameGlob->time - s_compassArtillery[localClientNum][i].fireTime <= pingTime )
            {
                alpha = (float)(1.0
                                            - (float)((float)(cgameGlob->time - s_compassArtillery[localClientNum][i].fireTime)
                                                            / (float)pingTime))
                            * max_alpha;
                ScramblerEnemyAlpha = CG_GetScramblerEnemyAlpha(localClientNum);
                alpha = (1.0 - ScramblerEnemyAlpha) * alpha;
                if ( cgameGlob->globalScramblerActive )
                    alpha = 0.0f;
                icon_color[3] = alpha;
                if ( isMortar )
                {
                    *(_QWORD *)position = *(_QWORD *)s_compassArtillery[localClientNum][i].realPos;
                }
                else
                {
                    position[0] = s_compassArtillery[localClientNum][i].pos[0];
                    position[1] = s_compassArtillery[localClientNum][i].pos[1];
                }
                CG_CompassDrawIconAtLocation(localClientNum, compassType, parentRect, rect, icon_color, position, icon);
            }
        }
    }
}

void __cdecl CG_CompassDrawIconAtLocation(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                float *color,
                float *vPos,
                Material *material)
{
    float yawVector[2]; // [esp+24h] [ebp-40h] BYREF
    float xy[2]; // [esp+2Ch] [ebp-38h] BYREF
    const ScreenPlacement *scrPlace; // [esp+34h] [ebp-30h]
    cg_s *cgameGlob; // [esp+38h] [ebp-2Ch]
    float centerY; // [esp+3Ch] [ebp-28h]
    rectDef_s scaledRect; // [esp+40h] [ebp-24h] BYREF
    float centerX; // [esp+58h] [ebp-Ch]
    float h; // [esp+5Ch] [ebp-8h] BYREF
    float w; // [esp+60h] [ebp-4h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    CG_CompassUpYawVector(cgameGlob, yawVector);
    CG_CompassCalcDimensions(
        compassType,
        cgameGlob,
        parentRect,
        rect,
        &scaledRect.x,
        &scaledRect.y,
        &scaledRect.w,
        &scaledRect.h);
    centerX = (float)(scaledRect.w * 0.5) + scaledRect.x;
    centerY = (float)(scaledRect.h * 0.5) + scaledRect.y;
    scrPlace = &scrPlaceView[localClientNum];
    CG_WorldPosToCompass(compassType, cgameGlob, &scaledRect, yawVector, cgameGlob->refdef.vieworg, vPos, 0, xy);
    CalcCompassFriendlySize(compassType, &w, &h);
    CG_DrawRotatedPic(
        scrPlace,
        (float)(centerX - (float)(w * 0.5)) + xy[0],
        (float)(centerY - (float)(h * 0.5)) + xy[1],
        w,
        h,
        rect->horzAlign,
        rect->vertAlign,
        0.0,
        color,
        material);
}

void __cdecl CG_CompassDrawIncomingArtilleryIcon(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                float *color)
{
    float yawVector[2]; // [esp+10h] [ebp-40h] BYREF
    Material *icon; // [esp+18h] [ebp-38h]
    bool clipped; // [esp+1Fh] [ebp-31h]
    float xy[2]; // [esp+20h] [ebp-30h] BYREF
    cg_s *cgameGlob; // [esp+28h] [ebp-28h]
    float artilleryPos[2]; // [esp+2Ch] [ebp-24h] BYREF
    rectDef_s scaledRect; // [esp+34h] [ebp-1Ch] BYREF
    const playerState_s *ps; // [esp+4Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    1793,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    if ( (cgameGlob->predictedPlayerState.perks[1] & 0x80) != 0 )
    {
        ps = &cgameGlob->predictedPlayerState;
        if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                        1800,
                        0,
                        "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        cgameGlob->nextSnap->ps.clientNum,
                        32) )
        {
            __debugbreak();
        }
        if ( (ps->artilleryInboundIconLocation & 0x70000) != 0 )
        {
            GetPositionFromPackedBits(localClientNum, 8, 255, ps->artilleryInboundIconLocation, artilleryPos);
            CG_CompassUpYawVector(cgameGlob, yawVector);
            CG_CompassCalcDimensions(
                compassType,
                cgameGlob,
                parentRect,
                rect,
                &scaledRect.x,
                &scaledRect.y,
                &scaledRect.w,
                &scaledRect.h);
            clipped = CG_WorldPosToCompass(
                                    compassType,
                                    cgameGlob,
                                    &scaledRect,
                                    yawVector,
                                    cgameGlob->refdef.vieworg,
                                    artilleryPos,
                                    0,
                                    xy);
            icon = cgMedia.compass_incoming_artillery;
            if ( (ps->artilleryInboundIconLocation & 0x10000) != 0 )
                *color = 0.0f;
            else
                color[1] = 0.0f;
            color[2] = 0.0f;
            if ( !clipped )
                CG_CompassDrawIconAtLocation(localClientNum, compassType, parentRect, rect, color, artilleryPos, icon);
        }
    }
}

void __cdecl GetPositionFromPackedBits(int localClientNum, char bitsPerAxis, int bitMask, int bits, float *vPos)
{
    float loc2d; // [esp+8h] [ebp-10h]
    float loc2d_4; // [esp+Ch] [ebp-Ch]
    cg_s *cgameGlob; // [esp+14h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    loc2d = (float)((float)(bitMask & bits) / (float)bitMask) * cgameGlob->compassMapWorldSize[0];
    loc2d_4 = (float)((float)(bitMask & (bits >> bitsPerAxis)) / (float)bitMask) * cgameGlob->compassMapWorldSize[1];
    *vPos = (float)((float)(loc2d * cgameGlob->compassNorth[1]) + cgameGlob->compassMapUpperLeft[0])
                - (float)(loc2d_4 * cgameGlob->compassNorth[0]);
    vPos[1] = (float)(cgameGlob->compassMapUpperLeft[1] - (float)(loc2d * cgameGlob->compassNorth[0]))
                    - (float)(loc2d_4 * cgameGlob->compassNorth[1]);
}

void __cdecl CG_CompassDrawEnemies(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                float *color)
{
    double v5; // st7
    bool v6; // [esp+24h] [ebp-C8h]
    bool v7; // [esp+28h] [ebp-C4h]
    bool v8; // [esp+2Ch] [ebp-C0h]
    bool v9; // [esp+30h] [ebp-BCh]
    Material *compassFiringMaterial; // [esp+4Ch] [ebp-A0h]
    float yawTo; // [esp+50h] [ebp-9Ch]
    Material *compassPingMaterial; // [esp+54h] [ebp-98h]
    bool withinRadarPingTime; // [esp+5Ah] [ebp-92h]
    float fadeTimeAmount; // [esp+5Ch] [ebp-90h]
    bool hasShowRadarPerk; // [esp+62h] [ebp-8Ah]
    const centity_s *cent; // [esp+64h] [ebp-88h]
    float StaticImageTimeAmount; // [esp+68h] [ebp-84h]
    const clientInfo_t *thisClientInfo; // [esp+6Ch] [ebp-80h]
    float radarTimeAmount; // [esp+70h] [ebp-7Ch]
    float satelliteTimeAmount; // [esp+74h] [ebp-78h]
    float yawVector[2]; // [esp+7Ch] [ebp-70h] BYREF
    bool clipped; // [esp+87h] [ebp-65h]
    clientInfo_t *localClientInfo; // [esp+88h] [ebp-64h]
    float xy[2]; // [esp+8Ch] [ebp-60h] BYREF
    int team; // [esp+94h] [ebp-58h]
    const ScreenPlacement *scrPlace; // [esp+98h] [ebp-54h]
    cg_s *cgameGlob; // [esp+9Ch] [ebp-50h]
    float compassFadeOutAlpha; // [esp+A0h] [ebp-4Ch]
    float centerY; // [esp+A4h] [ebp-48h]
    int clientIndex; // [esp+A8h] [ebp-44h]
    CompassActor *actor; // [esp+ACh] [ebp-40h]
    int num; // [esp+B0h] [ebp-3Ch]
    rectDef_s scaledRect; // [esp+B4h] [ebp-38h] BYREF
    float centerX; // [esp+CCh] [ebp-20h]
    float h; // [esp+D0h] [ebp-1Ch] BYREF
    float fadedColor[4]; // [esp+D4h] [ebp-18h] BYREF
    bool alwaysShowEnemies; // [esp+E7h] [ebp-5h]
    float w; // [esp+E8h] [ebp-4h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    1872,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    1874,
                    0,
                    "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cgameGlob->nextSnap->ps.clientNum,
                    32) )
    {
        __debugbreak();
    }
    localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum];
    CG_CompassUpYawVector(cgameGlob, yawVector);
    compassFadeOutAlpha = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    v5 = CG_FadeCompassIcons(localClientNum, 0);
    compassFadeOutAlpha = v5 * compassFadeOutAlpha;
    if ( compassFadeOutAlpha != 0.0 )
    {
        if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                        1885,
                        0,
                        "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        cgameGlob->nextSnap->ps.clientNum,
                        32) )
        {
            __debugbreak();
        }
        if ( cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].infoValid )
        {
            if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                            1888,
                            0,
                            "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                            cgameGlob->nextSnap->ps.clientNum,
                            32) )
            {
                __debugbreak();
            }
            team = cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].team;
            if ( team != 3 )
            {
                CG_CompassCalcDimensions(
                    compassType,
                    cgameGlob,
                    parentRect,
                    rect,
                    &scaledRect.x,
                    &scaledRect.y,
                    &scaledRect.w,
                    &scaledRect.h);
                centerX = (float)(scaledRect.w * 0.5) + scaledRect.x;
                centerY = (float)(scaledRect.h * 0.5) + scaledRect.y;
                fadedColor[0] = *color;
                fadedColor[1] = color[1];
                fadedColor[2] = color[2];
                fadedColor[3] = color[3];
                scrPlace = &scrPlaceView[localClientNum];
                clientIndex = localClientNum;
                alwaysShowEnemies = Dvar_GetBool("g_compassShowEnemies");
                if ( (cgameGlob->predictedPlayerState.otherFlags & 2) != 0 && compassSpectatorsSeeEnemies->current.enabled )
                    alwaysShowEnemies = 1;
                actor = s_compassActors[clientIndex];
                num = 0;
                while ( num < 32 )
                {
                    fadeTimeAmount = 0.0f;
                    radarTimeAmount = 0.0f;
                    satelliteTimeAmount = 0.0f;
                    StaticImageTimeAmount = 0.0f;
                    if ( actor->enemy )
                    {
                        thisClientInfo = &cgameGlob->bgs.clientinfo[num];
                        if ( thisClientInfo->infoValid )
                        {
                            cent = CG_GetEntity(localClientNum, num);
                            if ( cent->pose.player.waterHeight <= (float)(cent->pose.origin[2] + 60.0)
                                || thisClientInfo->attachedVehEntNum != 1023 )
                            {
                                hasShowRadarPerk = (thisClientInfo->perks[1] & 0x8000) != 0;
                                if ( !alwaysShowEnemies && (thisClientInfo->perks[1] & 0x8000) == 0 )
                                    goto LABEL_106;
                                if ( actor->lastUpdate > cgameGlob->time )
                                    actor->lastUpdate = 0;
                                if ( actor->lastUpdate >= cgameGlob->time - 1500 )
                                {
LABEL_106:
                                    v9 = (float)((float)actor->beginFadeTime + (float)(compassSoundPingFadeTime->current.value * 1000.0)) > (float)cgameGlob->time
                                        && actor->beginFadeTime;
                                    if ( alwaysShowEnemies || hasShowRadarPerk )
                                    {
                                        withinRadarPingTime = 1;
                                    }
                                    else
                                    {
                                        v8 = (float)((float)actor->beginRadarFadeTime
                                                             + (float)(compassRadarPingFadeTime->current.value * 1000.0)) > (float)cgameGlob->time
                                            && actor->beginRadarFadeTime;
                                        withinRadarPingTime = v8;
                                    }
                                    v7 = (float)((float)actor->beginSatelliteFadeTime
                                                         + (float)(compassSatellitePingFadeTime->current.value * 1000.0)) > (float)cgameGlob->time
                                        && actor->beginSatelliteFadeTime;
                                    v6 = (float)((float)actor->beginStaticImageFadeTime
                                                         + (float)(compassSatelliteStaticImageFadeTime->current.value * 1000.0)) > (float)cgameGlob->time
                                        && actor->beginStaticImageFadeTime;
                                    if ( v9 || withinRadarPingTime || v7 || v6 )
                                    {
                                        if ( alwaysShowEnemies || hasShowRadarPerk )
                                            clipped = CG_WorldPosToCompass(
                                                                    compassType,
                                                                    cgameGlob,
                                                                    &scaledRect,
                                                                    yawVector,
                                                                    cgameGlob->refdef.vieworg,
                                                                    actor->lastPos,
                                                                    0,
                                                                    xy);
                                        else
                                            clipped = CG_WorldPosToCompass(
                                                                    compassType,
                                                                    cgameGlob,
                                                                    &scaledRect,
                                                                    yawVector,
                                                                    cgameGlob->refdef.vieworg,
                                                                    actor->lastEnemyPos,
                                                                    0,
                                                                    xy);
                                        if ( !clipped || compassClampIcons->current.enabled )
                                        {
                                            CalcCompassFriendlySize(compassType, &w, &h);
                                            xy[0] = (float)(centerX - (float)(w * 0.5)) + xy[0];
                                            xy[1] = (float)(centerY - (float)(h * 0.5)) + xy[1];
                                            if ( actor->beginRadarFadeTime >= cgameGlob->time || alwaysShowEnemies || hasShowRadarPerk )
                                            {
                                                radarTimeAmount = 1.0f;
                                            }
                                            else if ( actor->beginRadarFadeTime )
                                            {
                                                radarTimeAmount = 1.0
                                                                                - (float)((float)(cgameGlob->time - actor->beginRadarFadeTime)
                                                                                                / (float)(compassRadarPingFadeTime->current.value * 1000.0));
                                            }
                                            if ( actor->beginSatelliteFadeTime < cgameGlob->time )
                                            {
                                                if ( actor->beginSatelliteFadeTime )
                                                    satelliteTimeAmount = 1.0
                                                                                            - (float)((float)(cgameGlob->time - actor->beginSatelliteFadeTime)
                                                                                                            / (float)(compassSatellitePingFadeTime->current.value * 1000.0));
                                            }
                                            else
                                            {
                                                satelliteTimeAmount = 1.0f;
                                            }
                                            if ( actor->beginStaticImageFadeTime < cgameGlob->time )
                                            {
                                                if ( actor->beginStaticImageFadeTime )
                                                    StaticImageTimeAmount = 1.0
                                                                                                - (float)((float)(cgameGlob->time - actor->beginStaticImageFadeTime)
                                                                                                                / (float)(compassSatelliteStaticImageFadeTime->current.value
                                                                                                                                * 1000.0));
                                            }
                                            else
                                            {
                                                StaticImageTimeAmount = 1.0f;
                                            }
                                            if ( actor->beginFadeTime < cgameGlob->time )
                                            {
                                                if ( actor->beginFadeTime )
                                                    fadeTimeAmount = 1.0
                                                                                 - (float)((float)(cgameGlob->time - actor->beginFadeTime)
                                                                                                 / (float)(compassSoundPingFadeTime->current.value * 1000.0));
                                            }
                                            else
                                            {
                                                fadeTimeAmount = 1.0f;
                                            }
                                            actor->currentFadeTimeAmount = 0.0f;
                                            if ( radarTimeAmount >= 0.0 )
                                            {
                                                actor->currentFadeTimeAmount = actor->currentFadeTimeAmount + radarTimeAmount;
                                                if ( thisClientInfo->attachedVehEntNum == 1023
                                                    || (cent->nextState.lerp.eFlags2 & 0x10000000) != 0 )
                                                {
                                                    fadedColor[3] = radarTimeAmount * compassFadeOutAlpha;
                                                    compassPingMaterial = GetCompassPingMaterialForClient(localClientNum, thisClientInfo);
                                                    UI_DrawHandlePic(
                                                        scrPlace,
                                                        xy[0],
                                                        xy[1],
                                                        w,
                                                        h,
                                                        rect->horzAlign,
                                                        rect->vertAlign,
                                                        fadedColor,
                                                        compassPingMaterial);
                                                }
                                            }
                                            if ( satelliteTimeAmount >= 0.0 )
                                            {
                                                actor->currentFadeTimeAmount = actor->currentFadeTimeAmount + satelliteTimeAmount;
                                                if ( thisClientInfo->attachedVehEntNum == 1023
                                                    || (cent->nextState.lerp.eFlags2 & 0x10000000) != 0 )
                                                {
                                                    fadedColor[3] = satelliteTimeAmount * compassFadeOutAlpha;
                                                    if ( compassType || !compassRotation->current.enabled )
                                                        yawTo = AngleNormalize360(cgameGlob->compassNorthYaw - actor->lastYaw);
                                                    else
                                                        yawTo = AngleNormalize360(cgameGlob->refdefViewAngles[1] - actor->lastYaw);
                                                    CG_DrawRotatedPic(
                                                        scrPlace,
                                                        xy[0],
                                                        xy[1],
                                                        w,
                                                        h,
                                                        rect->horzAlign,
                                                        rect->vertAlign,
                                                        yawTo,
                                                        fadedColor,
                                                        cgMedia.compassping_enemydirectional);
                                                }
                                            }
                                            if ( fadeTimeAmount >= 0.0 )
                                            {
                                                actor->currentFadeTimeAmount = actor->currentFadeTimeAmount + fadeTimeAmount;
                                                if ( thisClientInfo->attachedVehEntNum == 1023 )
                                                {
                                                    fadedColor[3] = fadeTimeAmount * compassFadeOutAlpha;
                                                    compassFiringMaterial = GetCompassFiringMaterialForClient(localClientNum, thisClientInfo);
                                                    UI_DrawHandlePic(
                                                        scrPlace,
                                                        xy[0],
                                                        xy[1],
                                                        w,
                                                        h,
                                                        rect->horzAlign,
                                                        rect->vertAlign,
                                                        fadedColor,
                                                        compassFiringMaterial);
                                                }
                                            }
                                        }
                                        CG_WorldPosToCompass(
                                            compassType,
                                            cgameGlob,
                                            &scaledRect,
                                            yawVector,
                                            cgameGlob->refdef.vieworg,
                                            actor->satelliteEnemyPos,
                                            0,
                                            xy);
                                        if ( !clipped && compassSatelliteStaticImageFadeTime->current.value > 0.0 )
                                        {
                                            CalcCompassFriendlySize(compassType, &w, &h);
                                            xy[0] = (float)(centerX - (float)(w * 0.5)) + xy[0];
                                            xy[1] = (float)(centerY - (float)(h * 0.5)) + xy[1];
                                            if ( actor->beginStaticImageFadeTime < cgameGlob->time )
                                            {
                                                if ( actor->beginStaticImageFadeTime )
                                                    StaticImageTimeAmount = 1.0
                                                                                                - (float)((float)(cgameGlob->time - actor->beginStaticImageFadeTime)
                                                                                                                / (float)(compassSatelliteStaticImageFadeTime->current.value
                                                                                                                                * 1000.0));
                                            }
                                            else
                                            {
                                                StaticImageTimeAmount = 1.0f;
                                            }
                                            if ( StaticImageTimeAmount >= 0.0 && thisClientInfo->attachedVehEntNum == 1023 )
                                            {
                                                fadedColor[3] = StaticImageTimeAmount;
                                                UI_DrawHandlePic(
                                                    scrPlace,
                                                    xy[0],
                                                    xy[1],
                                                    w,
                                                    h,
                                                    rect->horzAlign,
                                                    rect->vertAlign,
                                                    fadedColor,
                                                    cgMedia.compassping_enemysatellite);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    ++num;
                    ++actor;
                }
            }
        }
    }
}

Material *__cdecl GetCompassPingMaterialForClient(int localClientNum, const clientInfo_t *clientInfo)
{
    if ( xblive_wagermatch->current.enabled
        && clientInfo
        && clientInfo->score.score > 0
        && CG_GetPlaceWithTiesForScore(localClientNum, clientInfo->score.score) == 1 )
    {
        return cgMedia.compassping_firstplace;
    }
    else
    {
        return cgMedia.compassping_enemy;
    }
}

Material *__cdecl GetCompassFiringMaterialForClient(int localClientNum, const clientInfo_t *clientInfo)
{
    if ( xblive_wagermatch->current.enabled
        && clientInfo
        && clientInfo->score.score > 0
        && CG_GetPlaceWithTiesForScore(localClientNum, clientInfo->score.score) == 1 )
    {
        return cgMedia.compassping_firstplace;
    }
    else
    {
        return cgMedia.compassping_enemyfiring;
    }
}

void __cdecl CG_CompassDrawFakeFire(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                float *color)
{
    double v5; // st7
    float yawTo; // [esp+2Ch] [ebp-80h]
    float fadeTimeAmount; // [esp+34h] [ebp-78h]
    float yawVector[2]; // [esp+38h] [ebp-74h] BYREF
    bool clipped; // [esp+43h] [ebp-69h]
    clientInfo_t *localClientInfo; // [esp+44h] [ebp-68h]
    float xy[2]; // [esp+48h] [ebp-64h] BYREF
    int team; // [esp+50h] [ebp-5Ch]
    const ScreenPlacement *scrPlace; // [esp+54h] [ebp-58h]
    cg_s *cgameGlob; // [esp+58h] [ebp-54h]
    float compassFadeOutAlpha; // [esp+5Ch] [ebp-50h]
    float centerY; // [esp+60h] [ebp-4Ch]
    int clientIndex; // [esp+64h] [ebp-48h]
    CompassFakeFire *actor; // [esp+68h] [ebp-44h]
    int entNum; // [esp+6Ch] [ebp-40h]
    rectDef_s scaledRect; // [esp+70h] [ebp-3Ch] BYREF
    float centerX; // [esp+88h] [ebp-24h]
    float h; // [esp+8Ch] [ebp-20h] BYREF
    float fadedColor[4]; // [esp+90h] [ebp-1Ch] BYREF
    bool alwaysShowEnemies; // [esp+A3h] [ebp-9h]
    int actorNum; // [esp+A4h] [ebp-8h]
    float w; // [esp+A8h] [ebp-4h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    2088,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    2090,
                    0,
                    "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cgameGlob->nextSnap->ps.clientNum,
                    32) )
    {
        __debugbreak();
    }
    localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum];
    CG_CompassUpYawVector(cgameGlob, yawVector);
    compassFadeOutAlpha = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    v5 = CG_FadeCompassIcons(localClientNum, 0);
    compassFadeOutAlpha = v5 * compassFadeOutAlpha;
    if ( compassFadeOutAlpha != 0.0 )
    {
        if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                        2101,
                        0,
                        "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        cgameGlob->nextSnap->ps.clientNum,
                        32) )
        {
            __debugbreak();
        }
        if ( cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].infoValid )
        {
            if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                            2104,
                            0,
                            "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                            cgameGlob->nextSnap->ps.clientNum,
                            32) )
            {
                __debugbreak();
            }
            team = cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].team;
            if ( team != 3 )
            {
                CG_CompassCalcDimensions(
                    compassType,
                    cgameGlob,
                    parentRect,
                    rect,
                    &scaledRect.x,
                    &scaledRect.y,
                    &scaledRect.w,
                    &scaledRect.h);
                centerX = (float)(scaledRect.w * 0.5) + scaledRect.x;
                centerY = (float)(scaledRect.h * 0.5) + scaledRect.y;
                fadedColor[0] = *color;
                fadedColor[1] = color[1];
                fadedColor[2] = color[2];
                fadedColor[3] = color[3];
                scrPlace = &scrPlaceView[localClientNum];
                clientIndex = localClientNum;
                alwaysShowEnemies = Dvar_GetBool("g_compassShowEnemies");
                actorNum = 0;
                while ( actorNum < 32 )
                {
                    actor = s_compassFakeFire[clientIndex][actorNum];
                    entNum = 0;
                    while ( entNum < 8 )
                    {
                        fadeTimeAmount = 0.0f;
                        if ( (float)((float)actor->beginFadeTime + (float)(compassSoundPingFadeTime->current.value * 1000.0)) > (float)cgameGlob->time
                            && actor->beginFadeTime )
                        {
                            clipped = CG_WorldPosToCompass(
                                                    compassType,
                                                    cgameGlob,
                                                    &scaledRect,
                                                    yawVector,
                                                    cgameGlob->refdef.vieworg,
                                                    actor->pos,
                                                    0,
                                                    xy);
                            if ( !clipped || compassClampIcons->current.enabled )
                            {
                                CalcCompassFriendlySize(compassType, &w, &h);
                                xy[0] = (float)(centerX - (float)(w * 0.5)) + xy[0];
                                xy[1] = (float)(centerY - (float)(h * 0.5)) + xy[1];
                                if ( actor->beginFadeTime < cgameGlob->time )
                                {
                                    if ( actor->beginFadeTime )
                                        fadeTimeAmount = 1.0
                                                                     - (float)((float)(cgameGlob->time - actor->beginFadeTime)
                                                                                     / (float)(compassSoundPingFadeTime->current.value * 1000.0));
                                }
                                else
                                {
                                    fadeTimeAmount = 1.0f;
                                }
                                if ( fadeTimeAmount >= 0.0 )
                                {
                                    fadedColor[3] = fadeTimeAmount * compassFadeOutAlpha;
                                    if ( actor->enemy && actor->satellite )
                                    {
                                        if ( compassType || !compassRotation->current.enabled )
                                            yawTo = AngleNormalize360(cgameGlob->compassNorthYaw - actor->yaw);
                                        else
                                            yawTo = AngleNormalize360(cgameGlob->refdefViewAngles[1] - actor->yaw);
                                        CG_DrawRotatedPic(
                                            scrPlace,
                                            xy[0],
                                            xy[1],
                                            w,
                                            h,
                                            rect->horzAlign,
                                            rect->vertAlign,
                                            yawTo,
                                            fadedColor,
                                            cgMedia.compassping_enemydirectional);
                                    }
                                    else if ( actor->enemy )
                                    {
                                        UI_DrawHandlePic(
                                            scrPlace,
                                            xy[0],
                                            xy[1],
                                            w,
                                            h,
                                            rect->horzAlign,
                                            rect->vertAlign,
                                            fadedColor,
                                            cgMedia.compassping_enemyfiring);
                                    }
                                    else
                                    {
                                        UI_DrawHandlePic(
                                            scrPlace,
                                            xy[0],
                                            xy[1],
                                            w,
                                            h,
                                            rect->horzAlign,
                                            rect->vertAlign,
                                            fadedColor,
                                            cgMedia.compassping_friendlyfakefire);
                                    }
                                }
                            }
                        }
                        ++entNum;
                        ++actor;
                    }
                    ++actorNum;
                    ++actor;
                }
            }
        }
    }
}

void __cdecl CG_CompassDrawRadarEffects(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                float *color)
{
    double RadarLineMargin; // st7
    float radarProgress; // [esp+38h] [ebp-C4h]
    float v7; // [esp+48h] [ebp-B4h]
    float yawVector[2]; // [esp+50h] [ebp-ACh] BYREF
    float radiusPixels; // [esp+58h] [ebp-A4h]
    float out[2]; // [esp+5Ch] [ebp-A0h] BYREF
    rectDef_s scissorRect; // [esp+64h] [ebp-98h] BYREF
    float radius; // [esp+7Ch] [ebp-80h]
    rectDef_s compassRect; // [esp+80h] [ebp-7Ch] BYREF
    float pixPerInch; // [esp+98h] [ebp-64h]
    float fadedColor[4]; // [esp+9Ch] [ebp-60h] BYREF
    float center[2]; // [esp+ACh] [ebp-50h]
    float texBottom; // [esp+B4h] [ebp-48h]
    float texTop; // [esp+B8h] [ebp-44h]
    float texRight; // [esp+BCh] [ebp-40h]
    float radarXAmount; // [esp+C0h] [ebp-3Ch]
    float texLeft; // [esp+C4h] [ebp-38h]
    float xy[2]; // [esp+C8h] [ebp-34h] BYREF
    float radarLine[3]; // [esp+D0h] [ebp-2Ch] BYREF
    float rotation; // [esp+DCh] [ebp-20h]
    float radarTextureCenterX; // [esp+E0h] [ebp-1Ch]
    float playerPosRelativeToRadarLine; // [esp+E4h] [ebp-18h]
    const ScreenPlacement *scrPlace; // [esp+E8h] [ebp-14h]
    cg_s *cgameGlob; // [esp+ECh] [ebp-10h]
    float radarLineThickness; // [esp+F0h] [ebp-Ch]
    float h; // [esp+F4h] [ebp-8h] BYREF
    float w; // [esp+F8h] [ebp-4h] BYREF

    if ( !rect
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    2179,
                    0,
                    "%s",
                    "rect") )
    {
        __debugbreak();
    }
    if ( !parentRect
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    2180,
                    0,
                    "%s",
                    "parentRect") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    2183,
                    0,
                    "%s",
                    "cgameGlob") )
    {
        __debugbreak();
    }
    color[3] = CG_FadeCompassIcons(localClientNum, 0) * color[3];
    if ( color[3] != 0.0 )
    {
        if ( !cgameGlob->predictedPlayerState.spyplaneTypeEnabled )
            goto LABEL_31;
        scrPlace = &scrPlaceView[localClientNum];
        CG_CompassCalcDimensions(compassType, cgameGlob, parentRect, rect, xy, &xy[1], &w, &h);
        if ( w <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                        2201,
                        1,
                        "%s",
                        "w > 0.0f") )
        {
            __debugbreak();
        }
        if ( h <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                        2202,
                        1,
                        "%s",
                        "h > 0.0f") )
        {
            __debugbreak();
        }
        if ( compassType == COMPASS_TYPE_PARTIAL )
        {
            GetRadarLine(cgameGlob, cgameGlob->radarProgress, radarLine);
            radarLineThickness = compassRadarLineThickness->current.value;
            playerPosRelativeToRadarLine = (float)((float)(radarLine[0] * cgameGlob->refdef.vieworg[0])
                                                                                     + (float)(radarLine[1] * cgameGlob->refdef.vieworg[1]))
                                                                     - radarLine[2];
            playerPosRelativeToRadarLine = playerPosRelativeToRadarLine / compassMaxRange->current.value;
            radarTextureCenterX = (float)(playerPosRelativeToRadarLine / radarLineThickness) + 0.5;
            if ( compassRotation->current.enabled )
                rotation = cgameGlob->compassNorthYaw - cgameGlob->refdefViewAngles[1];
            else
                rotation = 0.0f;
            CL_DrawStretchPicRotatedST(
                scrPlace,
                xy[0],
                xy[1],
                w,
                h,
                rect->horzAlign,
                rect->vertAlign,
                radarTextureCenterX,
                0.0,
                0.5 / radarLineThickness,
                1.0,
                1.0,
                rotation,
                color,
                cgMedia.compass_radarline);
LABEL_31:
            if ( cgameGlob->hasLocalRadar )
            {
                scrPlace = &scrPlaceView[localClientNum];
                CG_CompassCalcDimensions(
                    compassType,
                    cgameGlob,
                    parentRect,
                    rect,
                    &compassRect.x,
                    &compassRect.y,
                    &compassRect.w,
                    &compassRect.h);
                center[0] = (float)(compassRect.w * 0.5) + compassRect.x;
                center[1] = (float)(compassRect.h * 0.5) + compassRect.y;
                CG_CompassUpYawVector(cgameGlob, yawVector);
                CG_WorldPosToCompass(
                    compassType,
                    cgameGlob,
                    &compassRect,
                    yawVector,
                    cgameGlob->refdef.vieworg,
                    cgameGlob->localRadarPos,
                    out,
                    0);
                radius = GetLocalRadarRadius(cgameGlob->localRadarProgress);
                if ( compassType )
                    pixPerInch = compassRect.h / cgameGlob->compassMapWorldSize[1];
                else
                    pixPerInch = compassRect.h / compassMaxRange->current.value;
                radiusPixels = radius * pixPerInch;
                scissorRect = compassRect;
                ScrPlace_ApplyRect(
                    &scrPlaceView[localClientNum],
                    &scissorRect.x,
                    &scissorRect.y,
                    &scissorRect.w,
                    &scissorRect.h,
                    rect->horzAlign,
                    rect->vertAlign);
                R_AddCmdSetScissorValues(1, (int)scissorRect.x, (int)scissorRect.y, (int)scissorRect.w, (int)scissorRect.h);
                fadedColor[0] = *color;
                fadedColor[1] = color[1];
                fadedColor[2] = color[2];
                fadedColor[3] = color[3];
                fadedColor[3] = 1.0 - (float)((float)(cgameGlob->localRadarProgress - 0.5) * 2.0);
                if ( fadedColor[3] < 0.0 )
                    fadedColor[3] = 0.0f;
                CL_DrawStretchPic(
                    scrPlace,
                    (float)(out[0] + center[0]) - radiusPixels,
                    (float)(out[1] + center[1]) - radiusPixels,
                    radiusPixels * 2.0,
                    radiusPixels * 2.0,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    fadedColor,
                    cgMedia.compass_acoustic_ping);
                R_AddCmdSetScissorValues(0, 0, 0, 0, 0);
            }
            return;
        }
        if ( compassType != COMPASS_TYPE_FULL
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                        2229,
                        0,
                        "%s",
                        "compassType == COMPASS_TYPE_FULL") )
        {
            __debugbreak();
        }
        radarProgress = cgameGlob->radarProgress;
        RadarLineMargin = GetRadarLineMargin(cgameGlob);
        v7 = RadarLineMargin / cgameGlob->compassMapWorldSize[0];
        radarXAmount = (float)((float)((float)(2.0 * v7) + 1.0) * radarProgress) - v7;
        radarLineThickness = cg_hudMapRadarLineThickness->current.value;
        if ( radarLineThickness > 0.0 )
        {
            //texLeft = (float)(COERCE_FLOAT(LODWORD(radarXAmount) ^ _mask__NegFloat_) / radarLineThickness) + 0.5;
            texLeft = (float)((-(radarXAmount)) / radarLineThickness) + 0.5;
            texRight = (float)((float)(1.0 - radarXAmount) / radarLineThickness) + 0.5;
            texBottom = 0.0f;
            if ( h == 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                            2240,
                            0,
                            "%s",
                            "h") )
            {
                __debugbreak();
            }
            texTop = 1.0 / (float)((float)(radarLineThickness * w) / h);
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                xy[0],
                xy[1],
                w,
                h,
                rect->horzAlign,
                rect->vertAlign,
                texLeft,
                texBottom,
                texRight,
                texTop,
                color,
                cgMedia.compass_radarline);
            goto LABEL_31;
        }
    }
}

void __cdecl CG_CompassDrawFlicker(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                float *color,
                Material *material)
{
    unsigned __int16 CodeConst_GenericParam0; // ax
    float c0[4]; // [esp+30h] [ebp-28h] BYREF
    const cg_s *cgameGlob; // [esp+40h] [ebp-18h]
    float scrambleAmount; // [esp+44h] [ebp-14h]
    float x; // [esp+48h] [ebp-10h] BYREF
    float y; // [esp+4Ch] [ebp-Ch] BYREF
    float h; // [esp+50h] [ebp-8h] BYREF
    float w; // [esp+54h] [ebp-4h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    CG_CompassCalcDimensions(compassType, cgameGlob, parentRect, rect, &x, &y, &w, &h);
    scrambleAmount = CG_GetScramblerEnemyAlpha(localClientNum);
    if ( scrambleAmount > 0.0 )
    {
        c0[0] = scrambleAmount;
        memset(&c0[1], 0, 12);
        CodeConst_GenericParam0 = GetCodeConst_GenericParam0();
        CL_SetCustomConstant(CodeConst_GenericParam0, c0);
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            x,
            y,
            w,
            h,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            color,
            material);
    }
}

const float Red[3] = { 0.89399999, 0.50099999, 0.537 };
const float Green[3] = { 0.509, 0.77200001, 0.537 };



void __cdecl CG_CompassDrawVehicles(
                int localClientNum,
                CompassType compassType,
                int eType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                const float *color)
{
    double ScramblerEnemyAlpha; // st7
    CompassActor *v7; // [esp+3Ch] [ebp-A0h]
    const CompassActor *actor; // [esp+40h] [ebp-9Ch]
    unsigned __int16 i; // [esp+48h] [ebp-94h]
    float yawVector[2]; // [esp+4Ch] [ebp-90h] BYREF
    bool clipped; // [esp+57h] [ebp-85h]
    clientInfo_t *localClientInfo; // [esp+58h] [ebp-84h]
    const vehicle_info_t *info; // [esp+5Ch] [ebp-80h]
    float xy[2]; // [esp+60h] [ebp-7Ch] BYREF
    CompassVehicle *veh; // [esp+68h] [ebp-74h]
    cg_s *cgameGlob; // [esp+6Ch] [ebp-70h]
    team_t myTeam; // [esp+70h] [ebp-6Ch]
    Material *material; // [esp+74h] [ebp-68h]
    centity_s *cent; // [esp+78h] [ebp-64h]
    float compassFadeOutAlpha; // [esp+7Ch] [ebp-60h]
    bool enemy; // [esp+82h] [ebp-5Ah]
    bool compassForcePlayerIcon; // [esp+83h] [ebp-59h]
    float yawTo; // [esp+84h] [ebp-58h]
    float centerY; // [esp+88h] [ebp-54h]
    float altColor[4]; // [esp+8Ch] [ebp-50h] BYREF
    bool friendly; // [esp+9Fh] [ebp-3Dh]
    int num; // [esp+A0h] [ebp-3Ch]
    rectDef_s scaledRect; // [esp+A4h] [ebp-38h] BYREF
    float centerX; // [esp+BCh] [ebp-20h]
    float h; // [esp+C0h] [ebp-1Ch]
    float fadedColor[4]; // [esp+C4h] [ebp-18h] BYREF
    bool isSelf; // [esp+D7h] [ebp-5h]
    float w; // [esp+D8h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    2336,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    if ( cgameGlob->clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    2338,
                    0,
                    "cgameGlob->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cgameGlob->clientNum,
                    32) )
    {
        __debugbreak();
    }
    localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->clientNum];
    compassFadeOutAlpha = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    compassFadeOutAlpha = CG_FadeCompassIcons(localClientNum, 0) * compassFadeOutAlpha;
    if ( compassFadeOutAlpha != 0.0 )
    {
        CG_CompassCalcDimensions(
            compassType,
            cgameGlob,
            parentRect,
            rect,
            &scaledRect.x,
            &scaledRect.y,
            &scaledRect.w,
            &scaledRect.h);
        centerX = (float)(scaledRect.w * 0.5) + scaledRect.x;
        centerY = (float)(scaledRect.h * 0.5) + scaledRect.y;
        fadedColor[0] = *color;
        fadedColor[1] = color[1];
        fadedColor[2] = color[2];
        fadedColor[3] = color[3];
        CG_CompassUpYawVector(cgameGlob, yawVector);
        compassForcePlayerIcon = Dvar_GetBool("compassForcePlayerIcon");
        veh = s_compassVehicles[localClientNum];
        num = 0;
        while ( 1 )
        {
            if ( num >= 16 )
                return;
            fadedColor[0] = *color;
            fadedColor[1] = color[1];
            fadedColor[2] = color[2];
            fadedColor[3] = color[3];
            cent = CG_GetEntity(localClientNum, veh->entityNum);
            if ( (cent->nextState.eType == eType || ((*((unsigned int *)cent + 201) >> 18) & 1) != 0)
                && (eType == 13 || ((*((unsigned int *)cent + 201) >> 18) & 1) != 0 || CG_VehGetHealthPercentageEntity(cent) != 0.0) )
            {
                if ( veh->lastUpdate > cgameGlob->time )
                    veh->lastUpdate = 0;
                if ( veh->lastUpdate >= cgameGlob->time - 1500 )
                {
                    clipped = CG_WorldPosToCompass(
                                            compassType,
                                            cgameGlob,
                                            &scaledRect,
                                            yawVector,
                                            cgameGlob->refdef.vieworg,
                                            veh->lastPos,
                                            0,
                                            xy);
                    if ( eType == 13 || ((*((unsigned int *)cent + 201) >> 18) & 1) != 0 || CG_IsVehicleMayhemGameType() )
                        clipped = 0;
                    if ( eType == 12 )
                    {
                        w = (float)(CL_LocalClient_GetActiveCount() <= 1 ? 40 : 80) * compassSize->current.value;
                        h = (float)(CL_LocalClient_GetActiveCount() <= 1 ? 40 : 80) * compassSize->current.value;
                    }
                    else
                    {
                        w = (float)(CL_LocalClient_GetActiveCount() <= 1 ? 20 : 40) * compassSize->current.value;
                        h = (float)(CL_LocalClient_GetActiveCount() <= 1 ? 20 : 40) * compassSize->current.value;
                    }
                    xy[0] = (float)(centerX - (float)(w * 0.5)) + xy[0];
                    xy[1] = (float)(centerY - (float)(h * 0.5)) + xy[1];
                    if ( compassType || !compassRotation->current.enabled )
                        yawTo = AngleNormalize360(cgameGlob->compassNorthYaw - veh->lastYaw);
                    else
                        yawTo = AngleNormalize360(cgameGlob->refdefViewAngles[1] - veh->lastYaw);
                    if ( fadedColor[3] > compassFadeOutAlpha )
                        fadedColor[3] = compassFadeOutAlpha;
                    myTeam = localClientInfo->team;
                    friendly = 0;
                    enemy = 0;
                    isSelf = 0;
                    if ( eType == 13 || ((*((unsigned int *)cent + 201) >> 18) & 1) != 0 )
                    {
                        if ( !cent->compassMaterial || !*cent->compassMaterial->info.name )
                            return;
                        material = cent->compassMaterial;
                        if ( veh->team == myTeam && (veh->team || veh->ownerIndex == localClientInfo->clientNum) )
                            friendly = 1;
                        else
                            enemy = 1;
LABEL_72:
                        if ( !clipped || enemy )
                        {
                            ScramblerEnemyAlpha = CG_GetScramblerEnemyAlpha(localClientNum);
                            fadedColor[3] = (1.0 - ScramblerEnemyAlpha) * fadedColor[3];
                            if ( cgameGlob->globalScramblerActive )
                                fadedColor[3] = 0.0f;
                            if ( eType == 13 && ((*((unsigned int *)cent + 201) >> 18) & 1) != 0 || !isSelf )
                            {
                                if ( friendly )
                                {
                                    if ( team_indicator->current.integer == 3 )
                                    {
                                        Dvar_GetUnpackedColor(cg_TeamColor_MyTeamAlt, altColor);
                                        fadedColor[0] = altColor[0];
                                        fadedColor[1] = altColor[1];
                                        fadedColor[2] = altColor[2];
                                    }
                                    else
                                    {
                                        fadedColor[0] = Green[0];
                                        fadedColor[1] = Green[1];
                                        fadedColor[2] = Green[2];
                                    }
                                    CG_DrawRotatedPic(
                                        &scrPlaceView[localClientNum],
                                        xy[0],
                                        xy[1],
                                        w,
                                        h,
                                        rect->horzAlign,
                                        rect->vertAlign,
                                        yawTo,
                                        fadedColor,
                                        material);
                                }
                                else if ( enemy )
                                {
                                    if ( team_indicator->current.integer == 3 )
                                    {
                                        Dvar_GetUnpackedColor(cg_TeamColor_EnemyTeamAlt, altColor);
                                        fadedColor[0] = altColor[0];
                                        fadedColor[1] = altColor[1];
                                        fadedColor[2] = altColor[2];
                                    }
                                    else
                                    {
                                        fadedColor[0] = Red[0];
                                        fadedColor[1] = Red[1];
                                        fadedColor[2] = Red[2];
                                    }
                                    fadedColor[3] = fadedColor[3] * compassFadeOutAlpha;
                                    CG_DrawRotatedPic(
                                        &scrPlaceView[localClientNum],
                                        xy[0],
                                        xy[1],
                                        w,
                                        h,
                                        rect->horzAlign,
                                        rect->vertAlign,
                                        yawTo,
                                        fadedColor,
                                        material);
                                }
                                else if ( !IsHardcoreMode(localClientNum) )
                                {
                                    fadedColor[0] = 1.0f;
                                    fadedColor[1] = 1.0f;
                                    fadedColor[2] = 1.0f;
                                    CG_DrawRotatedPic(
                                        &scrPlaceView[localClientNum],
                                        xy[0],
                                        xy[1],
                                        w,
                                        h,
                                        rect->horzAlign,
                                        rect->vertAlign,
                                        yawTo,
                                        fadedColor,
                                        material);
                                }
                            }
                            else
                            {
                                fadedColor[0] = 1.0f;
                                fadedColor[1] = 1.0f;
                                fadedColor[2] = 0.0f;
                                CG_DrawRotatedPic(
                                    &scrPlaceView[localClientNum],
                                    xy[0],
                                    xy[1],
                                    w,
                                    h,
                                    rect->horzAlign,
                                    rect->vertAlign,
                                    yawTo,
                                    fadedColor,
                                    material);
                            }
                        }
                        goto LABEL_9;
                    }
                    info = CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
                    material = info->compassIconMaterial;
                    for ( i = 0; i < com_maxclients->current.integer; ++i )
                    {
                        if ( cgameGlob->bgs.clientinfo[i].infoValid
                            && cgameGlob->bgs.clientinfo[i].attachedVehEntNum == veh->entityNum )
                        {
                            if ( cgameGlob->clientNum == cgameGlob->bgs.clientinfo[i].clientNum )
                            {
                                if ( compassForcePlayerIcon )
                                    material = cgMedia.compassping_player;
                                isSelf = 1;
                                break;
                            }
                            if ( cgameGlob->bgs.clientinfo[i].team == myTeam && cgameGlob->bgs.clientinfo[i].team )
                            {
                                if ( compassForcePlayerIcon )
                                    material = cgMedia.compassping_player;
                                friendly = 1;
                                break;
                            }
                            if ( (cgameGlob->predictedPlayerState.perks[1] & 0x80) == 0 || clipped )
                            {
                                if ( enemy )
                                {
                                    v7 = &s_compassActors[localClientNum][cgameGlob->bgs.clientinfo[i].clientNum];
                                    if ( !v7
                                        && !Assert_MyHandler(
                                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                                                    2475,
                                                    0,
                                                    "%s",
                                                    "actor") )
                                    {
                                        __debugbreak();
                                    }
                                    fadedColor[3] = fadedColor[3] + v7->currentFadeTimeAmount;
                                }
                                else
                                {
                                    enemy = 1;
                                    actor = &s_compassActors[localClientNum][cgameGlob->bgs.clientinfo[i].clientNum];
                                    if ( !actor
                                        && !Assert_MyHandler(
                                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                                                    2469,
                                                    0,
                                                    "%s",
                                                    "actor") )
                                    {
                                        __debugbreak();
                                    }
                                    fadedColor[3] = actor->currentFadeTimeAmount;
                                }
                            }
                            else
                            {
                                enemy = 1;
                            }
                        }
                    }
                    if ( !enemy || !info->remoteControl )
                        goto LABEL_72;
                }
            }
LABEL_9:
            ++num;
            ++veh;
        }
    }
}

void __cdecl CG_CompassDrawDogs(
                int localClientNum,
                CompassType compassType,
                int eType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Material *dogMaterial,
                const float *color)
{
    float yawVector[2]; // [esp+3Ch] [ebp-7Ch] BYREF
    CompassDogs *dog; // [esp+44h] [ebp-74h]
    bool clipped; // [esp+4Bh] [ebp-6Dh]
    float xy[2]; // [esp+4Ch] [ebp-6Ch] BYREF
    cg_s *cgameGlob; // [esp+54h] [ebp-64h]
    team_t myTeam; // [esp+58h] [ebp-60h]
    centity_s *cent; // [esp+5Ch] [ebp-5Ch]
    float compassFadeOutAlpha; // [esp+60h] [ebp-58h]
    float yawTo; // [esp+64h] [ebp-54h]
    float centerY; // [esp+68h] [ebp-50h]
    float altColor[4]; // [esp+6Ch] [ebp-4Ch] BYREF
    bool friendly; // [esp+7Fh] [ebp-39h]
    int num; // [esp+80h] [ebp-38h]
    rectDef_s scaledRect; // [esp+84h] [ebp-34h] BYREF
    float centerX; // [esp+9Ch] [ebp-1Ch]
    float h; // [esp+A0h] [ebp-18h]
    float fadedColor[4]; // [esp+A4h] [ebp-14h] BYREF
    float w; // [esp+B4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    2559,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    if ( (cgameGlob->predictedPlayerState.perks[1] & 0x80) != 0 )
    {
        compassFadeOutAlpha = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
        compassFadeOutAlpha = CG_FadeCompassIcons(localClientNum, 0) * compassFadeOutAlpha;
        if ( compassFadeOutAlpha != 0.0 )
        {
            CG_CompassCalcDimensions(
                compassType,
                cgameGlob,
                parentRect,
                rect,
                &scaledRect.x,
                &scaledRect.y,
                &scaledRect.w,
                &scaledRect.h);
            centerX = (float)(scaledRect.w * 0.5) + scaledRect.x;
            centerY = (float)(scaledRect.h * 0.5) + scaledRect.y;
            fadedColor[0] = *color;
            fadedColor[1] = color[1];
            fadedColor[2] = color[2];
            fadedColor[3] = color[3];
            CG_CompassUpYawVector(cgameGlob, yawVector);
            dog = s_compassDogs[localClientNum];
            myTeam = cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team;
            friendly = myTeam == dog->team && myTeam;
            num = 0;
            while ( num < 8 )
            {
                fadedColor[0] = *color;
                fadedColor[1] = color[1];
                fadedColor[2] = color[2];
                fadedColor[3] = color[3];
                cent = CG_GetEntity(localClientNum, dog->entityNum);
                if ( cent->nextState.eType == eType )
                {
                    if ( dog->lastUpdate > cgameGlob->time )
                        dog->lastUpdate = 0;
                    if ( dog->lastUpdate >= cgameGlob->time - 1500 )
                    {
                        clipped = CG_WorldPosToCompass(
                                                compassType,
                                                cgameGlob,
                                                &scaledRect,
                                                yawVector,
                                                cgameGlob->refdef.vieworg,
                                                dog->lastPos,
                                                0,
                                                xy);
                        if ( !clipped || compassClampIcons->current.enabled )
                        {
                            w = (float)(CL_LocalClient_GetActiveCount() <= 1 ? 15 : 30) * compassSize->current.value;
                            h = (float)(CL_LocalClient_GetActiveCount() <= 1 ? 15 : 30) * compassSize->current.value;
                            xy[0] = (float)(centerX - (float)(w * 0.5)) + xy[0];
                            xy[1] = (float)(centerY - (float)(h * 0.5)) + xy[1];
                            if ( compassType || !compassRotation->current.enabled )
                                yawTo = AngleNormalize360(cgameGlob->compassNorthYaw - dog->lastYaw);
                            else
                                yawTo = AngleNormalize360(cgameGlob->refdefViewAngles[1] - dog->lastYaw);
                            fadedColor[3] = compassFadeOutAlpha;
                            if ( friendly )
                            {
                                if ( team_indicator->current.integer == 3 )
                                {
                                    Dvar_GetUnpackedColor(cg_TeamColor_MyTeamAlt, altColor);
                                    fadedColor[0] = altColor[0];
                                    fadedColor[1] = altColor[1];
                                    fadedColor[2] = altColor[2];
                                }
                                else
                                {
                                    fadedColor[0] = Green[0];
                                    fadedColor[1] = Green[1];
                                    fadedColor[2] = Green[2];
                                }
                            }
                            else if ( team_indicator->current.integer == 3 )
                            {
                                Dvar_GetUnpackedColor(cg_TeamColor_EnemyTeamAlt, altColor);
                                fadedColor[0] = altColor[0];
                                fadedColor[1] = altColor[1];
                                fadedColor[2] = altColor[2];
                            }
                            else
                            {
                                fadedColor[0] = Red[0];
                                fadedColor[1] = Red[1];
                                fadedColor[2] = Red[2];
                            }
                            if ( !clipped )
                                CG_DrawRotatedPic(
                                    &scrPlaceView[localClientNum],
                                    xy[0],
                                    xy[1],
                                    w,
                                    h,
                                    rect->horzAlign,
                                    rect->vertAlign,
                                    yawTo,
                                    fadedColor,
                                    dogMaterial);
                        }
                    }
                }
                ++num;
                ++dog;
            }
        }
    }
}

void __cdecl CG_CompassDrawTurrets(
                int localClientNum,
                CompassType compassType,
                int eType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                const float *color)
{
    double ScramblerEnemyAlpha; // st7
    float yawVector[2]; // [esp+2Ch] [ebp-7Ch] BYREF
    CompassTurrets *turret; // [esp+34h] [ebp-74h]
    bool clipped; // [esp+3Bh] [ebp-6Dh]
    clientInfo_t *localClientInfo; // [esp+3Ch] [ebp-6Ch]
    float xy[2]; // [esp+40h] [ebp-68h] BYREF
    cg_s *cgameGlob; // [esp+48h] [ebp-60h]
    team_t myTeam; // [esp+4Ch] [ebp-5Ch]
    Material *material; // [esp+50h] [ebp-58h]
    centity_s *cent; // [esp+54h] [ebp-54h]
    float compassFadeOutAlpha; // [esp+58h] [ebp-50h]
    bool enemy; // [esp+5Eh] [ebp-4Ah]
    bool compassForcePlayerIcon; // [esp+5Fh] [ebp-49h]
    float yawTo; // [esp+60h] [ebp-48h]
    float centerY; // [esp+64h] [ebp-44h]
    bool friendly; // [esp+6Bh] [ebp-3Dh]
    int num; // [esp+6Ch] [ebp-3Ch]
    rectDef_s scaledRect; // [esp+70h] [ebp-38h] BYREF
    float centerX; // [esp+88h] [ebp-20h]
    float h; // [esp+8Ch] [ebp-1Ch]
    float fadedColor[4]; // [esp+90h] [ebp-18h] BYREF
    bool isOwnTurret; // [esp+A3h] [ebp-5h]
    float w; // [esp+A4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    2709,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    if ( cgameGlob->clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    2711,
                    0,
                    "cgameGlob->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cgameGlob->clientNum,
                    32) )
    {
        __debugbreak();
    }
    localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->clientNum];
    compassFadeOutAlpha = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    compassFadeOutAlpha = CG_FadeCompassIcons(localClientNum, 0) * compassFadeOutAlpha;
    if ( compassFadeOutAlpha != 0.0 )
    {
        CG_CompassCalcDimensions(
            compassType,
            cgameGlob,
            parentRect,
            rect,
            &scaledRect.x,
            &scaledRect.y,
            &scaledRect.w,
            &scaledRect.h);
        centerX = (float)(scaledRect.w * 0.5) + scaledRect.x;
        centerY = (float)(scaledRect.h * 0.5) + scaledRect.y;
        fadedColor[0] = *color;
        fadedColor[1] = color[1];
        fadedColor[2] = color[2];
        fadedColor[3] = color[3];
        CG_CompassUpYawVector(cgameGlob, yawVector);
        compassForcePlayerIcon = Dvar_GetBool("compassForcePlayerIcon");
        turret = s_compassTurrets[localClientNum];
        num = 0;
        while ( 1 )
        {
            if ( num >= 32 )
                return;
            fadedColor[0] = *color;
            fadedColor[1] = color[1];
            fadedColor[2] = color[2];
            fadedColor[3] = color[3];
            cent = CG_GetEntity(localClientNum, turret->entityNum);
            if ( cent->nextState.eType == eType )
            {
                if ( turret->lastUpdate > cgameGlob->time )
                    turret->lastUpdate = 0;
                if ( turret->lastUpdate >= cgameGlob->time - 1500 )
                {
                    clipped = CG_WorldPosToCompass(
                                            compassType,
                                            cgameGlob,
                                            &scaledRect,
                                            yawVector,
                                            cgameGlob->refdef.vieworg,
                                            turret->lastPos,
                                            0,
                                            xy);
                    w = (float)(CL_LocalClient_GetActiveCount() <= 1 ? 20 : 40) * compassSize->current.value;
                    h = (float)(CL_LocalClient_GetActiveCount() <= 1 ? 20 : 40) * compassSize->current.value;
                    xy[0] = (float)(centerX - (float)(w * 0.5)) + xy[0];
                    xy[1] = (float)(centerY - (float)(h * 0.5)) + xy[1];
                    if ( compassType || !compassRotation->current.enabled )
                        yawTo = AngleNormalize360(cgameGlob->compassNorthYaw - turret->lastYaw);
                    else
                        yawTo = AngleNormalize360(cgameGlob->refdefViewAngles[1] - turret->lastYaw);
                    if ( fadedColor[3] > compassFadeOutAlpha )
                        fadedColor[3] = compassFadeOutAlpha;
                    myTeam = localClientInfo->team;
                    friendly = 0;
                    enemy = 0;
                    isOwnTurret = 0;
                    if ( myTeam == turret->team && turret->team )
                        friendly = 1;
                    else
                        enemy = 1;
                    if ( turret->ownerIndex == cgameGlob->clientNum )
                    {
                        isOwnTurret = 1;
                        friendly = 1;
                    }
                    material = CG_GetTurretCompassMaterial(cent, friendly, turret->firing);
                    if ( (!clipped || isOwnTurret) && (cent->nextState.lerp.u.turret.flags & 8) == 0 )
                    {
                        ScramblerEnemyAlpha = CG_GetScramblerEnemyAlpha(localClientNum);
                        fadedColor[3] = (1.0 - ScramblerEnemyAlpha) * fadedColor[3];
                        if ( cgameGlob->globalScramblerActive )
                            fadedColor[3] = 0.0f;
                        if ( friendly )
                            goto LABEL_33;
                        if ( enemy )
                        {
                            if ( !cgameGlob->predictedPlayerState.satelliteTypeEnabled )
                                fadedColor[3] = compassFadeOutAlpha
                                                            - (float)((float)(cgameGlob->time - turret->beginFadeTime)
                                                                            / (float)(compassSoundPingFadeTime->current.value * 1000.0));
LABEL_33:
                            CG_DrawRotatedPic(
                                &scrPlaceView[localClientNum],
                                xy[0],
                                xy[1],
                                w,
                                h,
                                rect->horzAlign,
                                rect->vertAlign,
                                yawTo,
                                fadedColor,
                                material);
                            goto LABEL_9;
                        }
                        if ( !IsHardcoreMode(localClientNum) )
                        {
                            fadedColor[0] = 1.0f;
                            fadedColor[1] = 1.0f;
                            fadedColor[2] = 1.0f;
                            CG_DrawRotatedPic(
                                &scrPlaceView[localClientNum],
                                xy[0],
                                xy[1],
                                w,
                                h,
                                rect->horzAlign,
                                rect->vertAlign,
                                yawTo,
                                fadedColor,
                                material);
                        }
                    }
                }
            }
LABEL_9:
            ++num;
            ++turret;
        }
    }
}

Material *__cdecl CG_GetTurretCompassMaterial(centity_s *cent, bool friendly, bool isFiring)
{
    if ( (cent->nextState.lerp.u.turret.flags & 1) != 0 )
    {
        if ( friendly )
        {
            if ( isFiring )
                return cgMedia.compass_sentry_friendly_firing;
            else
                return cgMedia.compass_sentry_friendly;
        }
        else if ( isFiring )
        {
            return cgMedia.compass_sentry_enemy_firing;
        }
        else
        {
            return cgMedia.compass_sentry_enemy;
        }
    }
    else if ( (cent->nextState.lerp.u.turret.flags & 2) != 0 )
    {
        if ( friendly )
        {
            if ( isFiring )
                return cgMedia.compass_tow_turret_friendly_firing;
            else
                return cgMedia.compass_tow_turret_friendly;
        }
        else if ( isFiring )
        {
            return cgMedia.compass_tow_turret_enemy_firing;
        }
        else
        {
            return cgMedia.compass_tow_turret_enemy;
        }
    }
    else
    {
        return 0;
    }
}

void __cdecl CG_CompassDrawHelicopter(
                int localClientNum,
                CompassType compassType,
                int eType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                const float *color)
{
    double ScramblerEnemyAlpha; // st7
    float yawVector[2]; // [esp+2Ch] [ebp-8Ch] BYREF
    clientInfo_t *localClientInfo; // [esp+34h] [ebp-84h]
    const vehicle_info_t *info; // [esp+38h] [ebp-80h]
    float xy[2]; // [esp+3Ch] [ebp-7Ch] BYREF
    cg_s *cgameGlob; // [esp+44h] [ebp-74h]
    team_t myTeam; // [esp+48h] [ebp-70h]
    Material *material; // [esp+4Ch] [ebp-6Ch]
    centity_s *cent; // [esp+50h] [ebp-68h]
    float compassFadeOutAlpha; // [esp+54h] [ebp-64h]
    bool enemy; // [esp+5Bh] [ebp-5Dh]
    float yawTo; // [esp+5Ch] [ebp-5Ch]
    CompassHelicopter *copter; // [esp+60h] [ebp-58h]
    float centerY; // [esp+64h] [ebp-54h]
    float altColor[4]; // [esp+68h] [ebp-50h] BYREF
    bool friendly; // [esp+7Bh] [ebp-3Dh]
    int num; // [esp+7Ch] [ebp-3Ch]
    rectDef_s scaledRect; // [esp+80h] [ebp-38h] BYREF
    float centerX; // [esp+98h] [ebp-20h]
    float h; // [esp+9Ch] [ebp-1Ch]
    float fadedColor[4]; // [esp+A0h] [ebp-18h] BYREF
    bool isSelf; // [esp+B3h] [ebp-5h]
    float w; // [esp+B4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    2842,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    if ( cgameGlob->clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    2844,
                    0,
                    "cgameGlob->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cgameGlob->clientNum,
                    32) )
    {
        __debugbreak();
    }
    localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->clientNum];
    compassFadeOutAlpha = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    compassFadeOutAlpha = CG_FadeCompassIcons(localClientNum, 0) * compassFadeOutAlpha;
    if ( compassFadeOutAlpha != 0.0 )
    {
        CG_CompassCalcDimensions(
            compassType,
            cgameGlob,
            parentRect,
            rect,
            &scaledRect.x,
            &scaledRect.y,
            &scaledRect.w,
            &scaledRect.h);
        centerX = (float)(scaledRect.w * 0.5) + scaledRect.x;
        centerY = (float)(scaledRect.h * 0.5) + scaledRect.y;
        fadedColor[0] = *color;
        fadedColor[1] = color[1];
        fadedColor[2] = color[2];
        fadedColor[3] = color[3];
        CG_CompassUpYawVector(cgameGlob, yawVector);
        copter = s_compassHelicopter[localClientNum];
        num = 0;
        while ( num < 8 )
        {
            fadedColor[0] = *color;
            fadedColor[1] = color[1];
            fadedColor[2] = color[2];
            fadedColor[3] = color[3];
            cent = CG_GetEntity(localClientNum, copter->entityNum);
            if ( cent->nextState.eType == eType && CG_VehGetHealthPercentageEntity(cent) != 0.0 )
            {
                if ( copter->lastUpdate > cgameGlob->time )
                    copter->lastUpdate = 0;
                if ( copter->lastUpdate >= cgameGlob->time - 1500 )
                {
                    CG_WorldPosToCompass(
                        compassType,
                        cgameGlob,
                        &scaledRect,
                        yawVector,
                        cgameGlob->refdef.vieworg,
                        copter->lastPos,
                        0,
                        xy);
                    w = (float)(CL_LocalClient_GetActiveCount() <= 1 ? 40 : 80) * compassSize->current.value;
                    h = (float)(CL_LocalClient_GetActiveCount() <= 1 ? 40 : 80) * compassSize->current.value;
                    xy[0] = (float)(centerX - (float)(w * 0.5)) + xy[0];
                    xy[1] = (float)(centerY - (float)(h * 0.5)) + xy[1];
                    if ( compassType || !compassRotation->current.enabled )
                        yawTo = AngleNormalize360(cgameGlob->compassNorthYaw - copter->lastYaw);
                    else
                        yawTo = AngleNormalize360(cgameGlob->refdefViewAngles[1] - copter->lastYaw);
                    if ( fadedColor[3] > compassFadeOutAlpha )
                        fadedColor[3] = compassFadeOutAlpha;
                    ScramblerEnemyAlpha = CG_GetScramblerEnemyAlpha(localClientNum);
                    fadedColor[3] = (1.0 - ScramblerEnemyAlpha) * fadedColor[3];
                    if ( cgameGlob->globalScramblerActive )
                        fadedColor[3] = 0.0f;
                    myTeam = localClientInfo->team;
                    friendly = 0;
                    enemy = 0;
                    isSelf = 0;
                    info = CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
                    material = info->compassIconMaterial;
                    if ( localClientInfo->team )
                        enemy = localClientInfo->team != copter->team;
                    else
                        enemy = cgameGlob->clientNum != copter->ownerIndex;
                    friendly = !enemy;
                    if ( isSelf )
                    {
                        fadedColor[0] = 1.0f;
                        fadedColor[1] = 1.0f;
                        fadedColor[2] = 0.0f;
                        CG_DrawRotatedPic(
                            &scrPlaceView[localClientNum],
                            xy[0],
                            xy[1],
                            w,
                            h,
                            rect->horzAlign,
                            rect->vertAlign,
                            yawTo,
                            fadedColor,
                            material);
                    }
                    else if ( friendly )
                    {
                        if ( team_indicator->current.integer == 3 )
                        {
                            Dvar_GetUnpackedColor(cg_TeamColor_MyTeamAlt, altColor);
                            fadedColor[0] = altColor[0];
                            fadedColor[1] = altColor[1];
                            fadedColor[2] = altColor[2];
                        }
                        else
                        {
                            fadedColor[0] = Green[0];
                            fadedColor[1] = Green[1];
                            fadedColor[2] = Green[2];
                        }
                        CG_DrawRotatedPic(
                            &scrPlaceView[localClientNum],
                            xy[0],
                            xy[1],
                            w,
                            h,
                            rect->horzAlign,
                            rect->vertAlign,
                            yawTo,
                            fadedColor,
                            material);
                    }
                    else if ( enemy )
                    {
                        if ( team_indicator->current.integer == 3 )
                        {
                            Dvar_GetUnpackedColor(cg_TeamColor_EnemyTeamAlt, altColor);
                            fadedColor[0] = altColor[0];
                            fadedColor[1] = altColor[1];
                            fadedColor[2] = altColor[2];
                        }
                        else
                        {
                            fadedColor[0] = Red[0];
                            fadedColor[1] = Red[1];
                            fadedColor[2] = Red[2];
                        }
                        fadedColor[3] = fadedColor[3] * compassFadeOutAlpha;
                        CG_DrawRotatedPic(
                            &scrPlaceView[localClientNum],
                            xy[0],
                            xy[1],
                            w,
                            h,
                            rect->horzAlign,
                            rect->vertAlign,
                            yawTo,
                            fadedColor,
                            material);
                    }
                    else if ( !IsHardcoreMode(localClientNum) )
                    {
                        fadedColor[0] = 1.0f;
                        fadedColor[1] = 1.0f;
                        fadedColor[2] = 1.0f;
                        CG_DrawRotatedPic(
                            &scrPlaceView[localClientNum],
                            xy[0],
                            xy[1],
                            w,
                            h,
                            rect->horzAlign,
                            rect->vertAlign,
                            yawTo,
                            fadedColor,
                            material);
                    }
                }
            }
            ++num;
            ++copter;
        }
    }
}

void __cdecl CG_CompassDrawGuidedMissile(
                int localClientNum,
                CompassType compassType,
                int eType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                const float *color)
{
    double ScramblerEnemyAlpha; // st7
    float yawVector[2]; // [esp+2Ch] [ebp-88h] BYREF
    clientInfo_t *localClientInfo; // [esp+34h] [ebp-80h]
    float xy[2]; // [esp+38h] [ebp-7Ch] BYREF
    cg_s *cgameGlob; // [esp+40h] [ebp-74h]
    team_t myTeam; // [esp+44h] [ebp-70h]
    Material *material; // [esp+48h] [ebp-6Ch]
    centity_s *cent; // [esp+4Ch] [ebp-68h]
    float compassFadeOutAlpha; // [esp+50h] [ebp-64h]
    bool enemy; // [esp+57h] [ebp-5Dh]
    float yawTo; // [esp+58h] [ebp-5Ch]
    CompassGuidedMissile *missile; // [esp+5Ch] [ebp-58h]
    float centerY; // [esp+60h] [ebp-54h]
    float altColor[4]; // [esp+64h] [ebp-50h] BYREF
    bool friendly; // [esp+77h] [ebp-3Dh]
    int num; // [esp+78h] [ebp-3Ch]
    rectDef_s scaledRect; // [esp+7Ch] [ebp-38h] BYREF
    float centerX; // [esp+94h] [ebp-20h]
    float h; // [esp+98h] [ebp-1Ch]
    float fadedColor[4]; // [esp+9Ch] [ebp-18h] BYREF
    bool isSelf; // [esp+AFh] [ebp-5h]
    float w; // [esp+B0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    2977,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    if ( cgameGlob->clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    2979,
                    0,
                    "cgameGlob->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cgameGlob->clientNum,
                    32) )
    {
        __debugbreak();
    }
    localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->clientNum];
    compassFadeOutAlpha = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    compassFadeOutAlpha = CG_FadeCompassIcons(localClientNum, 0) * compassFadeOutAlpha;
    if ( compassFadeOutAlpha != 0.0 )
    {
        CG_CompassCalcDimensions(
            compassType,
            cgameGlob,
            parentRect,
            rect,
            &scaledRect.x,
            &scaledRect.y,
            &scaledRect.w,
            &scaledRect.h);
        centerX = (float)(scaledRect.w * 0.5) + scaledRect.x;
        centerY = (float)(scaledRect.h * 0.5) + scaledRect.y;
        fadedColor[0] = *color;
        fadedColor[1] = color[1];
        fadedColor[2] = color[2];
        fadedColor[3] = color[3];
        CG_CompassUpYawVector(cgameGlob, yawVector);
        missile = s_compassGuidedMissiles[localClientNum];
        num = 0;
        while ( num < 32 )
        {
            fadedColor[0] = *color;
            fadedColor[1] = color[1];
            fadedColor[2] = color[2];
            fadedColor[3] = color[3];
            cent = CG_GetEntity(localClientNum, missile->entityNum);
            if ( cent->nextState.eType == eType )
            {
                if ( missile->lastUpdate > cgameGlob->time )
                    missile->lastUpdate = 0;
                if ( missile->lastUpdate >= cgameGlob->time - 1500 )
                {
                    CG_WorldPosToCompass(
                        compassType,
                        cgameGlob,
                        &scaledRect,
                        yawVector,
                        cgameGlob->refdef.vieworg,
                        missile->lastPos,
                        0,
                        xy);
                    w = (float)(CL_LocalClient_GetActiveCount() <= 1 ? 15 : 30) * compassSize->current.value;
                    h = (float)(CL_LocalClient_GetActiveCount() <= 1 ? 15 : 30) * compassSize->current.value;
                    xy[0] = (float)(centerX - (float)(w * 0.5)) + xy[0];
                    xy[1] = (float)(centerY - (float)(h * 0.5)) + xy[1];
                    if ( compassType || !compassRotation->current.enabled )
                        yawTo = AngleNormalize360(cgameGlob->compassNorthYaw - missile->lastYaw);
                    else
                        yawTo = AngleNormalize360(cgameGlob->refdefViewAngles[1] - missile->lastYaw);
                    if ( fadedColor[3] > compassFadeOutAlpha )
                        fadedColor[3] = compassFadeOutAlpha;
                    ScramblerEnemyAlpha = CG_GetScramblerEnemyAlpha(localClientNum);
                    fadedColor[3] = (1.0 - ScramblerEnemyAlpha) * fadedColor[3];
                    if ( cgameGlob->globalScramblerActive )
                        fadedColor[3] = 0.0f;
                    myTeam = localClientInfo->team;
                    friendly = 0;
                    enemy = 0;
                    isSelf = 0;
                    material = cgMedia.compass_guided_missile;
                    if ( localClientInfo->team )
                        enemy = localClientInfo->team != missile->team;
                    else
                        enemy = cgameGlob->clientNum != missile->ownerIndex;
                    friendly = !enemy;
                    if ( enemy )
                    {
                        if ( team_indicator->current.integer == 3 )
                        {
                            Dvar_GetUnpackedColor(cg_TeamColor_EnemyTeamAlt, altColor);
                            fadedColor[0] = altColor[0];
                            fadedColor[1] = altColor[1];
                            fadedColor[2] = altColor[2];
                        }
                        else
                        {
                            fadedColor[0] = Red[0];
                            fadedColor[1] = Red[1];
                            fadedColor[2] = Red[2];
                        }
                        fadedColor[3] = fadedColor[3] * compassFadeOutAlpha;
                        CG_DrawRotatedPic(
                            &scrPlaceView[localClientNum],
                            xy[0],
                            xy[1],
                            w,
                            h,
                            rect->horzAlign,
                            rect->vertAlign,
                            yawTo,
                            fadedColor,
                            material);
                    }
                    else
                    {
                        if ( team_indicator->current.integer == 3 )
                        {
                            Dvar_GetUnpackedColor(cg_TeamColor_MyTeamAlt, altColor);
                            fadedColor[0] = altColor[0];
                            fadedColor[1] = altColor[1];
                            fadedColor[2] = altColor[2];
                        }
                        else
                        {
                            fadedColor[0] = Green[0];
                            fadedColor[1] = Green[1];
                            fadedColor[2] = Green[2];
                        }
                        CG_DrawRotatedPic(
                            &scrPlaceView[localClientNum],
                            xy[0],
                            xy[1],
                            w,
                            h,
                            rect->horzAlign,
                            rect->vertAlign,
                            yawTo,
                            fadedColor,
                            material);
                    }
                }
            }
            ++num;
            ++missile;
        }
    }
}

void __cdecl CG_CompassDrawScramblerFriendly(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                const float *color)
{
    double ScramblerEnemyAlpha; // st7
    float scissorWidth; // [esp+24h] [ebp-B4h]
    float scissorHeight; // [esp+28h] [ebp-B0h]
    rectDef_s compassRect; // [esp+30h] [ebp-A8h] BYREF
    float pixPerInch; // [esp+48h] [ebp-90h]
    CompassScrambler *scrambler; // [esp+4Ch] [ebp-8Ch]
    float yawVector[2]; // [esp+50h] [ebp-88h] BYREF
    clientInfo_t *localClientInfo; // [esp+58h] [ebp-80h]
    float xy[2]; // [esp+5Ch] [ebp-7Ch] BYREF
    cg_s *cgameGlob; // [esp+64h] [ebp-74h]
    team_t myTeam; // [esp+68h] [ebp-70h]
    rectDef_s scissorRect; // [esp+6Ch] [ebp-6Ch] BYREF
    Material *material; // [esp+84h] [ebp-54h]
    float compassFadeOutAlpha; // [esp+88h] [ebp-50h]
    float centerY; // [esp+8Ch] [ebp-4Ch]
    float altColor[4]; // [esp+90h] [ebp-48h] BYREF
    int num; // [esp+A0h] [ebp-38h]
    rectDef_s scaledRect; // [esp+A4h] [ebp-34h] BYREF
    float centerX; // [esp+BCh] [ebp-1Ch]
    float h; // [esp+C0h] [ebp-18h]
    float fadedColor[4]; // [esp+C4h] [ebp-14h] BYREF
    float w; // [esp+D4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    3108,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    if ( cgameGlob->clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_compassfriendlies_mp.cpp",
                    3110,
                    0,
                    "cgameGlob->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cgameGlob->clientNum,
                    32) )
    {
        __debugbreak();
    }
    localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->clientNum];
    compassFadeOutAlpha = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    compassFadeOutAlpha = CG_FadeCompassIcons(localClientNum, 0) * compassFadeOutAlpha;
    if ( compassFadeOutAlpha != 0.0 )
    {
        CG_CompassCalcDimensions(
            compassType,
            cgameGlob,
            parentRect,
            rect,
            &scaledRect.x,
            &scaledRect.y,
            &scaledRect.w,
            &scaledRect.h);
        centerX = (float)(scaledRect.w * 0.5) + scaledRect.x;
        centerY = (float)(scaledRect.h * 0.5) + scaledRect.y;
        fadedColor[0] = *color;
        fadedColor[1] = color[1];
        fadedColor[2] = color[2];
        fadedColor[3] = color[3];
        CG_CompassUpYawVector(cgameGlob, yawVector);
        material = cgMedia.compass_scrambler_large;
        scrambler = cgameGlob->compassScrambler;
        num = 0;
        while ( num < 16 )
        {
            if ( scrambler->handle )
            {
                fadedColor[0] = *color;
                fadedColor[1] = color[1];
                fadedColor[2] = color[2];
                fadedColor[3] = color[3];
                CG_WorldPosToCompass(
                    compassType,
                    cgameGlob,
                    &scaledRect,
                    yawVector,
                    cgameGlob->refdef.vieworg,
                    scrambler->pos,
                    xy,
                    0);
                CG_CompassCalcDimensions(
                    compassType,
                    cgameGlob,
                    parentRect,
                    rect,
                    &compassRect.x,
                    &compassRect.y,
                    &compassRect.w,
                    &compassRect.h);
                if ( compassType )
                    pixPerInch = compassRect.h / cgameGlob->compassMapWorldSize[1];
                else
                    pixPerInch = compassRect.h / compassMaxRange->current.value;
                w = (float)(1000.0 * pixPerInch) * 2.0;
                h = w;
                xy[0] = (float)(centerX - (float)(w * 0.5)) + xy[0];
                xy[1] = (float)(centerY - (float)(w * 0.5)) + xy[1];
                if ( fadedColor[3] > compassFadeOutAlpha )
                    fadedColor[3] = compassFadeOutAlpha;
                ScramblerEnemyAlpha = CG_GetScramblerEnemyAlpha(localClientNum);
                fadedColor[3] = (1.0 - ScramblerEnemyAlpha) * fadedColor[3];
                myTeam = localClientInfo->team;
                if ( team_indicator->current.integer == 3 )
                {
                    Dvar_GetUnpackedColor(cg_TeamColor_MyTeamAlt, altColor);
                    fadedColor[0] = altColor[0];
                    fadedColor[1] = altColor[1];
                    fadedColor[2] = altColor[2];
                }
                else
                {
                    fadedColor[0] = Green[0];
                    fadedColor[1] = Green[1];
                    fadedColor[2] = Green[2];
                }
                if ( compassType == COMPASS_TYPE_PARTIAL )
                {
                    scissorRect = *rect;
                    ScrPlace_ApplyRect(
                        &scrPlaceView[localClientNum],
                        &scissorRect.x,
                        &scissorRect.y,
                        &scissorRect.w,
                        &scissorRect.h,
                        scissorRect.horzAlign,
                        scissorRect.vertAlign);
                    scissorWidth = scissorRect.w;
                    scissorHeight = scissorRect.h;
                    if ( CL_LocalClient_GetActiveCount() > 1 )
                    {
                        scissorWidth = scissorWidth * 1.5;
                        scissorHeight = scissorHeight * 1.5;
                    }
                    R_AddCmdSetScissorValues(1, (int)scissorRect.x, (int)scissorRect.y, (int)scissorWidth, (int)scissorHeight);
                }
                CG_DrawRotatedPic(
                    &scrPlaceView[localClientNum],
                    xy[0],
                    xy[1],
                    w,
                    h,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.0,
                    fadedColor,
                    material);
                if ( compassType == COMPASS_TYPE_PARTIAL )
                    R_AddCmdSetScissorValues(0, 0, 0, 0, 0);
            }
            ++num;
            ++scrambler;
        }
    }
}

void __cdecl CG_ResetCompassData(int localClientNum, int time)
{
    int kk; // [esp+0h] [ebp-44h]
    int jj; // [esp+4h] [ebp-40h]
    int ii; // [esp+8h] [ebp-3Ch]
    int n; // [esp+Ch] [ebp-38h]
    int m; // [esp+10h] [ebp-34h]
    int k; // [esp+14h] [ebp-30h]
    int entNum; // [esp+18h] [ebp-2Ch]
    CompassFakeFire *fakefire; // [esp+1Ch] [ebp-28h]
    int j; // [esp+20h] [ebp-24h]
    int i; // [esp+24h] [ebp-20h]
    CompassDogs *dog; // [esp+28h] [ebp-1Ch]
    CompassTurrets *turret; // [esp+2Ch] [ebp-18h]
    CompassArtillery *artillery; // [esp+30h] [ebp-14h]
    CompassHelicopter *helicopter; // [esp+34h] [ebp-10h]
    CompassVehicle *vehicle; // [esp+38h] [ebp-Ch]
    CompassGuidedMissile *missile; // [esp+3Ch] [ebp-8h]
    CompassActor *actor; // [esp+40h] [ebp-4h]

    actor = s_compassActors[localClientNum];
    for ( i = 0; i < 32; ++i )
    {
        if ( actor->lastUpdate > time )
            actor->lastUpdate = 0;
        if ( actor->beginFadeTime > time )
            actor->beginFadeTime = 0;
        if ( actor->beginRadarFadeTime > time )
            actor->beginRadarFadeTime = 0;
        if ( actor->beginSatelliteFadeTime > time )
            actor->beginSatelliteFadeTime = 0;
        if ( actor->beginStaticImageFadeTime > time )
            actor->beginStaticImageFadeTime = 0;
        ++actor;
    }
    for ( j = 0; j < 32; ++j )
    {
        fakefire = s_compassFakeFire[localClientNum][j];
        for ( entNum = 0; entNum < 8; ++entNum )
        {
            if ( fakefire->beginFadeTime > time )
                fakefire->beginFadeTime = 0;
            ++fakefire;
        }
    }
    vehicle = s_compassVehicles[localClientNum];
    for ( k = 0; k < 16; ++k )
    {
        if ( vehicle->lastUpdate > time )
            vehicle->lastUpdate = 0;
        ++vehicle;
    }
    dog = s_compassDogs[localClientNum];
    for ( m = 0; m < 8; ++m )
    {
        if ( dog->lastUpdate > time )
            dog->lastUpdate = 0;
        ++dog;
    }
    artillery = s_compassArtillery[localClientNum];
    for ( n = 0; n < 6; ++n )
    {
        if ( artillery->fireTime > time )
            artillery->fireTime = 0;
        ++artillery;
    }
    helicopter = s_compassHelicopter[localClientNum];
    for ( ii = 0; ii < 8; ++ii )
    {
        if ( helicopter->lastUpdate > time )
            helicopter->lastUpdate = 0;
        ++helicopter;
    }
    turret = s_compassTurrets[localClientNum];
    for ( jj = 0; jj < 32; ++jj )
    {
        if ( turret->lastUpdate > time )
            turret->lastUpdate = 0;
        ++turret;
    }
    missile = s_compassGuidedMissiles[localClientNum];
    for ( kk = 0; kk < 32; ++kk )
    {
        if ( missile->lastUpdate > time )
            missile->lastUpdate = 0;
        ++missile;
    }
}

