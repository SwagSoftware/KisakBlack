#include "g_spawnsystem_mp.h"

#include <algorithm>
#include <universal/com_math_anglevectors.h>
#include "g_main_mp.h"
#include <server/sv_world.h>
#include <cgame/cg_drawtools.h>
#include <client/cl_debugdata.h>
#include <game/g_debug.h>

float g_legacySpawnPointRaiseHeight = 35.0f;
float missilePitchAdjust = 15.0f;
float heliPitchAdjust = 15.0f;
float speedScale = 2.0f;
float sizeScale = 1.0f;
float font_height = 1.5f;


const dvar_t *spawnsystem_debug;
const dvar_t *spawnsystem_debug_best_points;
const dvar_t *spawnsystem_debug_points;
const dvar_t *spawnsystem_debug_point_weights;
const dvar_t *spawnsystem_debug_influencers;
const dvar_t *spawnsystem_debug_influencer_types;
const dvar_t *spawnsystem_debug_player;
const dvar_t *spawnsystem_debug_team;
const dvar_t *spawnsystem_debug_sideswitched;
const dvar_t *spawnsystem_debug_visibility;
const dvar_t *spawnsystem_debug_visibility_time;
const dvar_t *spawnsystem_debug_archive;
const dvar_t *spawnsystem_debug_showclients;
const dvar_t *spawnsystem_debug_liveedit;
const dvar_t *spawnsystem_weapon_influencer_sight_check;
const dvar_t *spawnsystem_weapon_influencer_push_through;
const dvar_t *spawnsystem_weapon_influencer_min_length;
const dvar_t *spawnsystem_weapon_influencer_update_interval;
const dvar_t *spawnsystem_friend_influencer_stacking;
const dvar_t *spawnsystem_sight_check_max_distance;
const dvar_t *spawnsystem_badspawn_damage_delay;
const dvar_t *spawnsystem_badspawn_aggression_delay;
const dvar_t *spawnsystem_badspawn_force_record;

SpawnPoint g_spawnPoints[200];
int g_spawnPointCount;
SortedPointArray g_sortedTeamSpawnPoints[3];
float g_spawnRandomVariation;
int g_sortedSpawnInfluencerCount;
int g_lastInfluencerSortTime;
bool g_spawnPointsArchived;
eSpawnSystemDebugMode g_spawnSystemDebugMode;
SpawnInfluencer g_spawnInfluencers[328];
int g_lastPointComputeTime[3];
SpawnInfluencer *g_sortedSpawnInfluencers[328];
SpawnInfluencerPreset g_spawnInfluencerPresets[328];
int g_lastPointPlayer[3];
int g_sortAxis;
int g_debugHighlightedSpawnPoint;


struct compare_spawnpoint_sort
{
    bool operator()(const SpawnPoint &a, const SpawnPoint &b) const
    {
        return a.orderNum < b.orderNum;
    }
};

void __cdecl SpawnSystem_SortPoints()
{
    //std::sort(&g_spawnPoints[0], &g_spawnPoints[g_spawnPointCount], NULL);
    //std::_Sort<SpawnPoint *,int,compare_spawnpoint_sort>(
    //    g_spawnPoints,
    //    &g_spawnPoints[g_spawnPointCount],
    //    52 * g_spawnPointCount / 52,
    //    0);

    std::sort(
        g_spawnPoints,
        g_spawnPoints + g_spawnPointCount,
        compare_spawnpoint_sort()
    );
}

struct compare_spawnpoint_score_sort
{
    int team;

    compare_spawnpoint_score_sort(int t) : team(t) {}

    bool operator()(int a, int b) const
    {
        return g_spawnPoints[a].weight[team] > g_spawnPoints[b].weight[team];
    }
};

void __cdecl SpawnSystem_SortPointsForTeamByScore(int team)
{
    //int *_Last; // [esp+E8h] [ebp-Ch]
    //
    //_Last = &g_sortedTeamSpawnPoints[team].spawnPointsByIndex[g_sortedTeamSpawnPoints[team].count];
    ////std::_Sort<int *, int, compare_spawnpoint_score_sort>(
    ////    g_sortedTeamSpawnPoints[team].spawnPointsByIndex,
    ////    _Last,
    ////    ((char *)_Last - (char *)&g_sortedTeamSpawnPoints[team]) >> 2,
    ////    (compare_spawnpoint_score_sort)team);
    //
    //std::sort(g_sortedTeamSpawnPoints[team].spawnPointsByIndex, 
    //    &g_sortedTeamSpawnPoints[team].spawnPointsByIndex[g_sortedTeamSpawnPoints[team].count],
    //    team);

    int *first = g_sortedTeamSpawnPoints[team].spawnPointsByIndex;
    int *last = first + g_sortedTeamSpawnPoints[team].count;

    std::sort(
        first,
        last,
        compare_spawnpoint_score_sort(team)
    );
}

void __cdecl SpawnSystem_Init()
{
    int j; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    SpawnSystem_RegisterDvars();
    g_spawnRandomVariation = 0.0f;
    g_spawnPointCount = 0;
    g_sortedSpawnInfluencerCount = 0;
    g_lastInfluencerSortTime = 0;
    g_spawnPointsArchived = 0;
    g_spawnSystemDebugMode = SS_DEBUG_OFF;
    memset(g_spawnPoints, 0, sizeof(g_spawnPoints));
    memset(g_spawnInfluencers, 0, sizeof(g_spawnInfluencers));
    memset(g_sortedSpawnInfluencers, 0, sizeof(g_sortedSpawnInfluencers));
    g_lastPointComputeTime[0] = 0;
    g_lastPointComputeTime[1] = 0;
    g_lastPointComputeTime[2] = 0;
    memset(g_spawnInfluencerPresets, 0, sizeof(g_spawnInfluencerPresets));
    for ( i = 0; i < 328; ++i )
        g_spawnInfluencerPresets[i].type = INFLUENCER_TYPE_UNUSED;
    for ( j = 0; j < 3; ++j )
        g_lastPointPlayer[j] = 1023;
}

const dvar_s *SpawnSystem_RegisterDvars()
{
    const dvar_s *result; // eax

    spawnsystem_debug = _Dvar_RegisterBool("spawnsystem_debug", 0, 0x80u, "Debug and rendering for the spawn system");
    spawnsystem_debug_best_points = _Dvar_RegisterBool(
                                                                        "spawnsystem_debug_best_points",
                                                                        1,
                                                                        0x80u,
                                                                        "Render best spawn point information");
    spawnsystem_debug_points = _Dvar_RegisterInt(
                                                             "spawnsystem_debug_points",
                                                             1,
                                                             0,
                                                             2,
                                                             0x80u,
                                                             "Render spawn point boxes. 2 = no depth test");
    spawnsystem_debug_point_weights = _Dvar_RegisterBool(
                                                                            "spawnsystem_debug_point_weights",
                                                                            1,
                                                                            0x80u,
                                                                            "Render spawn point information");
    spawnsystem_debug_influencers = _Dvar_RegisterInt(
                                                                        "spawnsystem_debug_influencers",
                                                                        1,
                                                                        0,
                                                                        2,
                                                                        0x80u,
                                                                        "Use colors to differentiate influencer types");
    spawnsystem_debug_influencer_types = _Dvar_RegisterBool(
                                                                                 "spawnsystem_debug_influencer_types",
                                                                                 0,
                                                                                 0x80u,
                                                                                 "Render spawn influencer information");
    spawnsystem_debug_player = _Dvar_RegisterString(
                                                             "spawnsystem_debug_player",
                                                             (char *)"",
                                                             0x80u,
                                                             "Name of player to show debug info for");
    spawnsystem_debug_team = _Dvar_RegisterString(
                                                         "spawnsystem_debug_team",
                                                         (char *)"",
                                                         0x80u,
                                                         "Team to show debug info for");
    spawnsystem_debug_sideswitched = _Dvar_RegisterBool(
                                                                         "spawnsystem_debug_sideswitched",
                                                                         0,
                                                                         0x80u,
                                                                         "Switch to \"sideswitched\" mode");
    spawnsystem_debug_visibility = _Dvar_RegisterBool(
                                                                     "spawnsystem_debug_visibility",
                                                                     1,
                                                                     0x80u,
                                                                     "Render spawn point visibility checking");
    spawnsystem_debug_visibility_time = _Dvar_RegisterInt(
                                                                                "spawnsystem_debug_visibility_time",
                                                                                1,
                                                                                0,
                                                                                100000,
                                                                                0x80u,
                                                                                "Length of time to render the visibility check");
    spawnsystem_debug_archive = _Dvar_RegisterInt(
                                                                "spawnsystem_debug_archive",
                                                                0,
                                                                0,
                                                                5,
                                                                0x80u,
                                                                "Sets the spawn system archive mode");
    spawnsystem_debug_showclients = _Dvar_RegisterBool(
                                                                        "spawnsystem_debug_showclients",
                                                                        1,
                                                                        0x80u,
                                                                        "Render archived clients");
    spawnsystem_debug_liveedit = _Dvar_RegisterBool("spawnsystem_debug_liveedit", 0, 0x80u, "Do it, live edit now!");
    spawnsystem_weapon_influencer_sight_check = _Dvar_RegisterBool(
                                                                                                "spawnsystem_weapon_influencer_sight_check",
                                                                                                1,
                                                                                                0x80u,
                                                                                                "Do a line of sight check on the weapon influencer");
    spawnsystem_weapon_influencer_push_through = _Dvar_RegisterFloat(
                                                                                                 "spawnsystem_weapon_influencer_push_through",
                                                                                                 50.0,
                                                                                                 -1024.0,
                                                                                                 1024.0,
                                                                                                 0x80u,
                                                                                                 "Extend the line of sight distance this much after check");
    spawnsystem_weapon_influencer_min_length = _Dvar_RegisterFloat(
                                                                                             "spawnsystem_weapon_influencer_min_length",
                                                                                             0.1,
                                                                                             0.0099999998,
                                                                                             1.0,
                                                                                             0x81u,
                                                                                             "Minimum lenght the line of sight check will drop the distance to");
    spawnsystem_weapon_influencer_update_interval = _Dvar_RegisterInt(
                                                                                                        "spawnsystem_weapon_influencer_update_interval",
                                                                                                        151,
                                                                                                        0,
                                                                                                        5000,
                                                                                                        0x81u,
                                                                                                        "How often to update the sight check in milliseconds");
    spawnsystem_friend_influencer_stacking = _Dvar_RegisterBool(
                                                                                         "spawnsystem_friend_influencer_stacking",
                                                                                         0,
                                                                                         0x80u,
                                                                                         "Combine all friendly player influencers");
    spawnsystem_sight_check_max_distance = _Dvar_RegisterFloat(
                                                                                     "spawnsystem_sight_check_max_distance",
                                                                                     1800.0,
                                                                                     1500.0,
                                                                                     3500.0,
                                                                                     0,
                                                                                     "Max distance for the line of sight check");
    spawnsystem_badspawn_damage_delay = _Dvar_RegisterInt(
                                                                                "spawnsystem_badspawn_damage_delay",
                                                                                3000,
                                                                                0,
                                                                                1000000,
                                                                                0,
                                                                                "Report damage that occurs to a player within this MS of spawning in");
    spawnsystem_badspawn_aggression_delay = _Dvar_RegisterInt(
                                                                                        "spawnsystem_badspawn_aggression_delay",
                                                                                        2000,
                                                                                        0,
                                                                                        1000000,
                                                                                        0,
                                                                                        "Report aggression to others that a player deals within this MS of spawning in");
    result = _Dvar_RegisterBool(
                         "spawnsystem_badspawn_force_record",
                         0,
                         0x80u,
                         "Immediately after each client spawn, submit a corresponding badspawn report");
    spawnsystem_badspawn_force_record = result;
    return result;
}

int __cdecl SpawnSystem_FindPointByEnt(int entNum)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < g_spawnPointCount; ++i )
    {
        if ( g_spawnPoints[i].entNum == entNum )
            return i;
    }
    return -1;
}

void __cdecl SpawnSystem_SetRandomVariation(float variation)
{
    g_spawnRandomVariation = variation;
}

void __cdecl SpawnSystem_ClearPoints()
{
    g_spawnPointCount = 0;
    memset(g_spawnPoints, 0, sizeof(g_spawnPoints));
    g_spawnPointsArchived = 0;
}

char __cdecl SpawnSystem_AddPoint(unsigned int team, gentity_s *ent)
{
    float *weight; // ecx
    float *baseWeight; // eax
    SpawnPoint *v5; // [esp+0h] [ebp-Ch]
    int index; // [esp+8h] [ebp-4h]

    if ( team >= 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    883,
                    0,
                    "team doesn't index MAX_SPAWNPOINT_TEAM\n\t%i not in [0, %i)",
                    team,
                    3) )
    {
        __debugbreak();
    }
    index = SpawnSystem_FindPointByEnt(ent->s.number);
    if ( index == -1 )
    {
        if ( g_spawnPointCount + 1 >= 200 )
            return 0;
        v5 = &g_spawnPoints[g_spawnPointCount];
        v5->origin[0] = ent->r.currentOrigin[0];
        v5->origin[1] = ent->r.currentOrigin[1];
        v5->origin[2] = ent->r.currentOrigin[2];
        g_spawnPoints[g_spawnPointCount].yaw = ent->r.currentAngles[1];
        g_spawnPoints[g_spawnPointCount].origin[2] = g_spawnPoints[g_spawnPointCount].origin[2]
                                                                                             + g_legacySpawnPointRaiseHeight;
        g_spawnPoints[g_spawnPointCount].entNum = ent->s.number;
        weight = g_spawnPoints[g_spawnPointCount].weight;
        *weight = 0.0;
        weight[1] = 0.0;
        weight[2] = 0.0;
        baseWeight = g_spawnPoints[g_spawnPointCount].baseWeight;
        *baseWeight = 0.0;
        baseWeight[1] = 0.0;
        baseWeight[2] = 0.0;
        index = g_spawnPointCount;
        g_spawnPoints[g_spawnPointCount++].teamMask = 0;
    }
    g_spawnPoints[index].teamMask |= 1 << team;
    return 1;
}

void __cdecl SpawnSystem_ClearPointsBaseWeight(int teammask)
{
    int i; // [esp+4h] [ebp-4h]

    if ( !teammask )
        teammask = 7;
    for ( i = 0; i < g_spawnPointCount; ++i )
        SpawnSystem_SetPointBaseWeight(&g_spawnPoints[i], teammask, 0.0);
}

void __cdecl SpawnSystem_SetPointBaseWeight(SpawnPoint *point, char teammask, float weight)
{
    if ( !point
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp", 917, 0, "%s", "point") )
    {
        __debugbreak();
    }
    if ( (teammask & 1) != 0 )
        point->baseWeight[0] = weight;
    if ( (teammask & 2) != 0 )
        point->baseWeight[1] = weight;
    if ( (teammask & 4) != 0 )
        point->baseWeight[2] = weight;
}

void __cdecl SpawnSystem_SetPointsBaseWeight(int teammask, float *objective_position, float angle, float score)
{
    double v4; // xmm0_8
    double v5; // xmm0_8
    long double v6; // [esp+8h] [ebp-58h]
    long double v7; // [esp+8h] [ebp-58h]
    float axis[3][3]; // [esp+14h] [ebp-4Ch] BYREF
    float forward[3]; // [esp+38h] [ebp-28h] BYREF
    float angle_cos; // [esp+44h] [ebp-1Ch]
    float direction[3]; // [esp+48h] [ebp-18h] BYREF
    int i; // [esp+54h] [ebp-Ch]
    float dot; // [esp+58h] [ebp-8h]
    SpawnPoint *point; // [esp+5Ch] [ebp-4h]

    if ( !teammask )
        teammask = 7;
    point = 0;
    //__libm_sse2_cos(v6);
    //angle_cos = angle * 0.017453292;
    angle_cos = cos(angle * 0.017453292);
    dot = 0.0f;
    for ( i = 0; i < g_spawnPointCount; ++i )
    {
        point = &g_spawnPoints[i];
        if ( point->entNum < 0 )
        {
            YawToAxis(point->yaw, axis);
            *(_QWORD *)forward = *(_QWORD *)&axis[0][0];
            forward[2] = axis[0][2];
        }
        else
        {
            AngleVectors(g_entities[point->entNum].r.currentAngles, forward, 0, 0);
        }
        direction[0] = *objective_position - point->origin[0];
        direction[1] = objective_position[1] - point->origin[1];
        direction[2] = objective_position[2] - point->origin[2];
        Vec3Normalize(direction);
        dot = (float)((float)(forward[0] * direction[0]) + (float)(forward[1] * direction[1]))
                + (float)(forward[2] * direction[2]);
        if ( dot < angle_cos )
        {
            iassert((acosf(dot) * 180.f / M_PI) > angle);
            //v5 = dot;
            //__libm_sse2_acos(v7);
            //*(float *)&v5 = v5;
            //if ( (float)((float)(*(float *)&v5 * 180.0) / 3.1415927) <= angle
            //    && !Assert_MyHandler(
            //                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
            //                994,
            //                0,
            //                "%s",
            //                "(acosf( dot ) * 180.f / M_PI) > angle") )
            //{
            //    __debugbreak();
            //}
            SpawnSystem_SetPointBaseWeight(&g_spawnPoints[i], teammask, 0.0);
        }
        else
        {
            iassert((acosf(dot) * 180.f / M_PI) <= angle);
            //v4 = dot;
            //__libm_sse2_acos(v7);
            //*(float *)&v4 = v4;
            //if ( angle < (float)((float)(*(float *)&v4 * 180.0) / 3.1415927)
            //    && !Assert_MyHandler(
            //                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
            //                989,
            //                0,
            //                "%s",
            //                "(acosf( dot ) * 180.f / M_PI) <= angle") )
            //{
            //    __debugbreak();
            //}
            SpawnSystem_SetPointBaseWeight(&g_spawnPoints[i], teammask, score);
        }
    }
}

SpawnInfluencerPreset *__cdecl SpawnSystem_SetPresetData(
                int influencerIndex,
                eInfluencerType type,
                eInfluencerShape shape,
                float length,
                float radius,
                float score,
                eInfluencerScoreCurve score_curve)
{
    SpawnInfluencerPreset *preset; // [esp+0h] [ebp-4h]

    preset = &g_spawnInfluencerPresets[influencerIndex];
    preset->index = influencerIndex;
    preset->type = type;
    preset->shape = shape;
    preset->originalLength = length;
    preset->radius = radius;
    preset->score = score;
    preset->score_curve = score_curve;
    return preset;
}

void __cdecl SpawnSystem_InitInfluencer(
                SpawnInfluencer *influencer,
                eInfluencerType type,
                eInfluencerShape shape,
                const float *origin,
                const float *forward,
                const float *up,
                float radius,
                float axis_length,
                float score,
                eInfluencerScoreCurve score_curve,
                int teamMask,
                unsigned int entNum,
                int timeout)
{
    float right[3]; // [esp+3Ch] [ebp-30h] BYREF
    float axis[3][3]; // [esp+48h] [ebp-24h] BYREF

    if ( entNum >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1370,
                    0,
                    "entNum doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
                    entNum,
                    1024) )
    {
        __debugbreak();
    }
    if ( (unsigned int)type >= MAX_INFLUENCER_TYPE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1371,
                    0,
                    "type doesn't index MAX_INFLUENCER_TYPE\n\t%i not in [0, %i)",
                    type,
                    8) )
    {
        __debugbreak();
    }
    if ( radius <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1372,
                    0,
                    "%s",
                    "radius > 0.0f") )
    {
        __debugbreak();
    }
    influencer->preset = SpawnSystem_SetPresetData(
                                                 influencer - g_spawnInfluencers,
                                                 type,
                                                 shape,
                                                 axis_length,
                                                 radius,
                                                 score,
                                                 score_curve);
    influencer->used = 1;
    influencer->active = 1;
    influencer->teamMask = teamMask;
    influencer->origin[0] = *origin;
    influencer->origin[1] = origin[1];
    influencer->origin[2] = origin[2];
    influencer->forward[0] = *forward;
    influencer->forward[1] = forward[1];
    influencer->forward[2] = forward[2];
    influencer->up[0] = *up;
    influencer->up[1] = up[1];
    influencer->up[2] = up[2];
    influencer->midPoint[0] = *origin;
    influencer->midPoint[1] = origin[1];
    influencer->midPoint[2] = origin[2];
    Vec3Cross(up, forward, right);
    *(_QWORD *)&axis[0][0] = *(_QWORD *)forward;
    axis[0][2] = forward[2];
    *(_QWORD *)&axis[1][0] = *(_QWORD *)right;
    axis[1][2] = right[2];
    axis[2][0] = *up;
    axis[2][1] = up[1];
    axis[2][2] = up[2];
    AxisToQuat(axis, influencer->quat);
    Vec4Normalize(influencer->quat);
    influencer->worldQuat[0] = influencer->quat[0];
    influencer->worldQuat[1] = influencer->quat[1];
    influencer->worldQuat[2] = influencer->quat[2];
    influencer->worldQuat[3] = influencer->quat[3];
    influencer->boundingRadius = radius * radius;
    influencer->length = axis_length;
    if ( entNum == 1023 || entNum == 1022 )
    {
        influencer->entity.setEnt(0);
    }
    else
    {
        if ( g_entities[entNum].s.eType != 1
            && g_entities[entNum].s.eType != 17
            && g_entities[entNum].s.eType != 14
            && g_entities[entNum].s.eType != 6
            && g_entities[entNum].s.eType != 4
            && g_entities[entNum].s.eType != 12
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                        1405,
                        0,
                        "%s",
                        "SPAWNSYSTEM_LINKED_ETYPES( g_entities[entNum].s.eType )") )
        {
            __debugbreak();
        }
        influencer->entity.setEnt(&g_entities[entNum]);
    }
    if ( !influencer->teamMask )
        influencer->teamMask = 7;
    if ( timeout )
        influencer->expireTime = timeout + level.time;
    else
        influencer->expireTime = 0;
}

int __cdecl SpawnSystem_AddSphereInfluencer(
                eInfluencerType type,
                float *origin,
                float radius,
                float score,
                eInfluencerScoreCurve score_curve,
                int teamMask,
                unsigned int entNum,
                int timeout)
{
    SpawnInfluencer *influencer; // [esp+20h] [ebp-20h]
    float forward[3]; // [esp+24h] [ebp-1Ch] BYREF
    int index; // [esp+30h] [ebp-10h]
    float up[3]; // [esp+34h] [ebp-Ch] BYREF

    index = SpawnSystem_FindFreeInfluencer();
    if ( index == -1 )
        Com_Error(ERR_DROP, "No more free spawn influencers");
    influencer = &g_spawnInfluencers[index];
    up[0] = 0.0f;
    up[1] = 0.0f;
    up[2] = 1.0f;
    forward[0] = 1.0f;
    forward[1] = 0.0f;
    forward[2] = 0.0f;
    SpawnSystem_InitInfluencer(
        influencer,
        type,
        INFLUENCER_SHAPE_SPHERE,
        origin,
        forward,
        up,
        radius,
        0.0,
        score,
        score_curve,
        teamMask,
        entNum,
        timeout);
    influencer->boundingRadius = radius;
    SpawnSystem_ComputeInfluencerBounds(influencer);
    if ( g_spawnSystemDebugMode == SS_DEBUG_OFF )
        SpawnSystem_InsertSortedInfluencer(influencer);
    return index;
}

int __cdecl SpawnSystem_FindFreeInfluencer()
{
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 328; ++i )
    {
        if ( !g_spawnInfluencers[i].used )
            return i;
    }
    return -1;
}

void __cdecl SpawnSystem_ComputeInfluencerBounds(SpawnInfluencer *influencer)
{
    if ( !influencer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1035,
                    0,
                    "%s",
                    "influencer") )
    {
        __debugbreak();
    }
    influencer->bounds[0] = influencer->origin[g_sortAxis] - influencer->boundingRadius;
    influencer->bounds[1] = influencer->origin[g_sortAxis] + influencer->boundingRadius;
}

void __cdecl SpawnSystem_InsertSortedInfluencer(SpawnInfluencer *influencer)
{
    int low; // [esp+0h] [ebp-4h]

    iassert(influencer);

    for ( low = 0; low < g_sortedSpawnInfluencerCount; ++low )
    {
        iassert(g_sortedSpawnInfluencers[low]);
        iassert(influencer != g_sortedSpawnInfluencers[low]);

        if ( influencer->bounds[0] < g_sortedSpawnInfluencers[low]->bounds[0] )
        {
            memmove(&g_sortedSpawnInfluencers[low + 1],
                &g_sortedSpawnInfluencers[low],
                sizeof(SpawnInfluencer *) * (g_sortedSpawnInfluencerCount - low));
            g_sortedSpawnInfluencers[low] = influencer;
            ++g_sortedSpawnInfluencerCount;
            return;
        }
    }
    g_sortedSpawnInfluencers[g_sortedSpawnInfluencerCount++] = influencer;
}

int __cdecl SpawnSystem_AddCylinderInfluencer(
                eInfluencerType type,
                float *origin,
                float *forward,
                float *up,
                float radius,
                float axis_length,
                float score,
                eInfluencerScoreCurve score_curve,
                int teamMask,
                unsigned int entNum,
                int timeout)
{
    SpawnInfluencer *influencer; // [esp+24h] [ebp-Ch]
    int index; // [esp+2Ch] [ebp-4h]

    index = SpawnSystem_FindFreeInfluencer();
    if ( index == -1 )
        Com_Error(ERR_DROP, "No more free spawn influencers");
    influencer = &g_spawnInfluencers[index];
    if ( entNum >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1477,
                    0,
                    "entNum doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
                    entNum,
                    1024) )
    {
        __debugbreak();
    }
    if ( (unsigned int)type >= MAX_INFLUENCER_TYPE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1478,
                    0,
                    "type doesn't index MAX_INFLUENCER_TYPE\n\t%i not in [0, %i)",
                    type,
                    8) )
    {
        __debugbreak();
    }
    if ( radius <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1479,
                    0,
                    "%s",
                    "radius > 0.0f") )
    {
        __debugbreak();
    }
    SpawnSystem_InitInfluencer(
        influencer,
        type,
        INFLUENCER_SHAPE_CYLINDER,
        origin,
        forward,
        up,
        radius,
        axis_length,
        score,
        score_curve,
        teamMask,
        entNum,
        timeout);
    influencer->boundingRadius = radius + (float)(0.5 * axis_length);
    SpawnSystem_ComputeInfluencerBounds(influencer);
    SpawnSystem_UpdateCylinderInfluencer(influencer);
    if ( g_spawnSystemDebugMode == SS_DEBUG_OFF )
        SpawnSystem_InsertSortedInfluencer(influencer);
    return index;
}

void __cdecl SpawnSystem_UpdateCylinderInfluencer(SpawnInfluencer *influencer)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    if ( !influencer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1269,
                    0,
                    "%s",
                    "influencer") )
    {
        __debugbreak();
    }
    if ( influencer->preset->shape != INFLUENCER_SHAPE_CYLINDER
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1270,
                    0,
                    "%s",
                    "influencer->preset->shape == INFLUENCER_SHAPE_CYLINDER") )
    {
        __debugbreak();
    }
    if ( !influencer->entity.isDefined() )
        goto LABEL_13;
    ent = influencer->entity.ent();
    if ( !ent->client || ent->client->ps.pm_type != 2 && ent->client->ps.pm_type != 3 )
    {
        SpawnSystem_CalculateCylinderAxis(influencer);
        SpawnSystem_CalculateCylinderLength(influencer);
LABEL_13:
        SpawnSystem_CalculateCylinderAxisMidpoint(influencer);
    }
}

void __cdecl SpawnSystem_CalculateCylinderLength(SpawnInfluencer *influencer)
{
    float originalLength; // [esp+4h] [ebp-88h]
    float end[3]; // [esp+Ch] [ebp-80h] BYREF
    float fraction; // [esp+18h] [ebp-74h]
    gentity_s *player; // [esp+58h] [ebp-34h]
    float speedIPS; // [esp+5Ch] [ebp-30h]
    gentity_s *ent; // [esp+60h] [ebp-2Ch]
    col_context_t context; // [esp+64h] [ebp-28h] BYREF

    if ( !influencer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1120,
                    0,
                    "%s",
                    "influencer") )
    {
        __debugbreak();
    }
    if ( influencer->preset->shape != INFLUENCER_SHAPE_CYLINDER
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1121,
                    0,
                    "%s",
                    "influencer->preset->shape == INFLUENCER_SHAPE_CYLINDER") )
    {
        __debugbreak();
    }
    //col_context_t::col_context_t(&context);
    if ( influencer->active && g_spawnSystemDebugMode == SS_DEBUG_OFF )
    {
        if ( influencer->preset->type == INFLUENCER_TYPE_VEHICLE && influencer->entity.isDefined() )
        {
            ent = influencer->entity.ent();
            if ( !ent->scr_vehicle
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                            1135,
                            0,
                            "%s",
                            "ent->scr_vehicle") )
            {
                __debugbreak();
            }
            influencer->length = influencer->preset->radius * 0.5;
            speedIPS = ent->scr_vehicle->speed;
            if ( speedIPS < -0.001 )
                influencer->length = influencer->length * -1.0;
            influencer->length = (float)(influencer->preset->originalLength * speedIPS) + influencer->length;
        }
        else if ( influencer->preset->type == INFLUENCER_TYPE_WEAPON
                     && spawnsystem_weapon_influencer_sight_check->current.enabled
                     && level.time > spawnsystem_weapon_influencer_update_interval->current.integer + influencer->lastUpdateTime )
        {
            player = influencer->entity.ent();
            if ( !player->client
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                            1155,
                            0,
                            "%s",
                            "player->client") )
            {
                __debugbreak();
            }
            trace_t trace; // [esp+1Ch] [ebp-70h] BYREF
            originalLength = influencer->preset->originalLength;
            end[0] = (float)(originalLength * influencer->up[0]) + influencer->origin[0];
            end[1] = (float)(originalLength * influencer->up[1]) + influencer->origin[1];
            end[2] = (float)(originalLength * influencer->up[2]) + influencer->origin[2];
            G_TraceCapsule(
                &trace,
                influencer->origin,
                vec3_origin,
                vec3_origin,
                end,
                player->s.number,
                0x806833,
                &context);
            fraction = trace.fraction;
            if ( spawnsystem_weapon_influencer_min_length->current.value > trace.fraction )
                fraction = spawnsystem_weapon_influencer_min_length->current.value;
            if ( trace.fraction >= 0.001 )
            {
                influencer->length = (float)(influencer->preset->originalLength * fraction)
                                                     + spawnsystem_weapon_influencer_push_through->current.value;
                if ( influencer->length > influencer->preset->originalLength )
                    influencer->length = influencer->preset->originalLength;
            }
            else
            {
                influencer->length = 1.0f;
            }
            influencer->lastUpdateTime = level.time;
        }
    }
}

void __cdecl SpawnSystem_CalculateCylinderAxis(SpawnInfluencer *influencer)
{
    gclient_s *client; // ecx
    int eType; // [esp+4h] [ebp-ACh]
    float v3; // [esp+8h] [ebp-A8h]
    float v4; // [esp+Ch] [ebp-A4h]
    float v5; // [esp+3Ch] [ebp-74h]
    float v6; // [esp+40h] [ebp-70h]
    float v7; // [esp+48h] [ebp-68h]
    float v8; // [esp+4Ch] [ebp-64h]
    float tempAxis[3][3]; // [esp+5Ch] [ebp-54h] BYREF
    float angles[3]; // [esp+80h] [ebp-30h] BYREF
    gentity_s *ent; // [esp+8Ch] [ebp-24h]
    float ent_quat[4]; // [esp+90h] [ebp-20h] BYREF
    float current_quat[4]; // [esp+A0h] [ebp-10h] BYREF

    if ( !influencer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1195,
                    0,
                    "%s",
                    "influencer") )
    {
        __debugbreak();
    }
    if ( influencer->preset->shape != INFLUENCER_SHAPE_CYLINDER
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1196,
                    0,
                    "%s",
                    "influencer->preset->shape == INFLUENCER_SHAPE_CYLINDER") )
    {
        __debugbreak();
    }
    if ( influencer->entity.isDefined() )
    {
        ent = influencer->entity.ent();
        if ( ent->client && influencer->preset->type == INFLUENCER_TYPE_WEAPON )
        {
            client = ent->client;
            angles[0] = client->ps.viewangles[0];
            angles[1] = client->ps.viewangles[1];
            angles[2] = client->ps.viewangles[2];
        }
        else
        {
            angles[0] = ent->r.currentAngles[0];
            angles[1] = ent->r.currentAngles[1];
            angles[2] = ent->r.currentAngles[2];
        }
        if ( ent->s.eType == 12 )
        {
            v7 = AngleNormalize180(angles[0]) + heliPitchAdjust;
            if ( (float)(v7 - 90.0) < 0.0 )
                v8 = v7;
            else
                v8 = 90.0f;
            if ( (float)(-90.0 - v7) < 0.0 )
                v4 = v8;
            else
                v4 = -90.0f;
            angles[0] = v4;
        }
        else if ( ent->s.eType == 4 )
        {
            v5 = AngleNormalize180(angles[0]) + missilePitchAdjust;
            if ( (float)(v5 - 90.0) < 0.0 )
                v6 = v5;
            else
                v6 = 90.0f;
            if ( (float)(-90.0 - v5) < 0.0 )
                v3 = v6;
            else
                v3 = -90.0f;
            angles[0] = v3;
        }
        AnglesToQuat(angles, ent_quat);
        Vec4Normalize(ent_quat);
        QuatMultiply(influencer->quat, ent_quat, current_quat);
        Vec4Normalize(current_quat);
        influencer->worldQuat[0] = current_quat[0];
        influencer->worldQuat[1] = current_quat[1];
        influencer->worldQuat[2] = current_quat[2];
        influencer->worldQuat[3] = current_quat[3];
        UnitQuatToAxis(current_quat, tempAxis);
        eType = ent->s.eType;
        if ( eType == 4 || eType == 12 )
        {
            influencer->forward[0] = tempAxis[2][0];
            influencer->forward[1] = tempAxis[2][1];
            influencer->forward[2] = tempAxis[2][2];
            *(_QWORD *)influencer->up = *(_QWORD *)&tempAxis[0][0];
            influencer->up[2] = tempAxis[0][2];
        }
        else
        {
            *(_QWORD *)influencer->forward = *(_QWORD *)&tempAxis[0][0];
            influencer->forward[2] = tempAxis[0][2];
            influencer->up[0] = tempAxis[2][0];
            influencer->up[1] = tempAxis[2][1];
            influencer->up[2] = tempAxis[2][2];
        }
    }
}

void __cdecl QuatMultiply(const float *in1, const float *in2, float *out)
{
    if ( in1 == out
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 963, 0, "%s", "in1 != out") )
    {
        __debugbreak();
    }
    if ( in2 == out
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 964, 0, "%s", "in2 != out") )
    {
        __debugbreak();
    }
    *out = (float)((float)((float)(*in1 * in2[3]) + (float)(in1[3] * *in2)) + (float)(in1[2] * in2[1]))
             - (float)(in1[1] * in2[2]);
    out[1] = (float)((float)((float)(in1[1] * in2[3]) - (float)(in1[2] * *in2)) + (float)(in1[3] * in2[1]))
                 + (float)(*in1 * in2[2]);
    out[2] = (float)((float)((float)(in1[2] * in2[3]) + (float)(in1[1] * *in2)) - (float)(*in1 * in2[1]))
                 + (float)(in1[3] * in2[2]);
    out[3] = (float)((float)((float)(in1[3] * in2[3]) - (float)(*in1 * *in2)) - (float)(in1[1] * in2[1]))
                 - (float)(in1[2] * in2[2]);
}

void __cdecl SpawnSystem_CalculateCylinderAxisMidpoint(SpawnInfluencer *influencer)
{
    float half_cylinder_height; // [esp+Ch] [ebp-4h]

    if ( !influencer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1255,
                    0,
                    "%s",
                    "influencer") )
    {
        __debugbreak();
    }
    if ( influencer->preset->shape != INFLUENCER_SHAPE_CYLINDER
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1256,
                    0,
                    "%s",
                    "influencer->preset->shape == INFLUENCER_SHAPE_CYLINDER") )
    {
        __debugbreak();
    }
    half_cylinder_height = 0.5 * influencer->length;
    influencer->midPoint[0] = (float)(half_cylinder_height * influencer->up[0]) + influencer->origin[0];
    influencer->midPoint[1] = (float)(half_cylinder_height * influencer->up[1]) + influencer->origin[1];
    influencer->midPoint[2] = (float)(half_cylinder_height * influencer->up[2]) + influencer->origin[2];
}

char __cdecl SpawnSystem_RemoveInfluencer(unsigned int influencer_index)
{
    if ( influencer_index >= 0x148
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1505,
                    0,
                    "influencer_index doesn't index MAX_INFLUENCERS\n\t%i not in [0, %i)",
                    influencer_index,
                    328) )
    {
        __debugbreak();
    }
    if ( !g_spawnInfluencers[influencer_index].used )
        return 0;
    g_spawnInfluencers[influencer_index].used = 0;
    SpawnSystem_RemoveSortedInfluencer(&g_spawnInfluencers[influencer_index]);
    return 1;
}

void __cdecl SpawnSystem_RemoveSortedInfluencer(SpawnInfluencer *influencer)
{
    signed int low; // [esp+0h] [ebp-4h]

    if ( g_spawnSystemDebugMode == SS_DEBUG_OFF )
    {
        if ( !influencer
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                        1096,
                        0,
                        "%s",
                        "influencer") )
        {
            __debugbreak();
        }
        for ( low = 0; low < g_sortedSpawnInfluencerCount; ++low )
        {
            if ( !g_sortedSpawnInfluencers[low]
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                            1102,
                            0,
                            "%s",
                            "g_sortedSpawnInfluencers[low]") )
            {
                __debugbreak();
            }
            if ( g_sortedSpawnInfluencers[low] == influencer )
            {
                SpawnSystem_RemoveSortedInfluencerByIndex(low);
                return;
            }
        }
        if ( influencer
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                        1111,
                        0,
                        "%s\n\t%s",
                        "!influencer",
                        "Trying to remove an spawn influencer from the sorted array when it has never been added.") )
        {
            __debugbreak();
        }
    }
}

void __cdecl SpawnSystem_RemoveSortedInfluencerByIndex(unsigned int influencer_index)
{
    if ( g_spawnSystemDebugMode == SS_DEBUG_OFF )
    {
        bcassert(influencer_index, g_sortedSpawnInfluencerCount);
        
        if (influencer_index + 1 != g_sortedSpawnInfluencerCount)
        {
            memmove(&g_sortedSpawnInfluencers[influencer_index],
                &g_sortedSpawnInfluencers[influencer_index + 1],
                sizeof(SpawnInfluencer *) * (g_sortedSpawnInfluencerCount - influencer_index));
        }
            
        g_sortedSpawnInfluencers[--g_sortedSpawnInfluencerCount] = 0;
    }
}

char __cdecl SpawnSystem_EnableInfluencer(unsigned int influencer_index, bool enabled)
{
    if (influencer_index >= 0x148
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
            1523,
            0,
            "influencer_index doesn't index MAX_INFLUENCERS\n\t%i not in [0, %i)",
            influencer_index,
            328))
    {
        __debugbreak();
    }
    if (!g_spawnInfluencers[influencer_index].used)
        return 0;
    g_spawnInfluencers[influencer_index].active = enabled;
    return 1;
}

char __cdecl SpawnSystem_SetInfluencerTeamMask(unsigned int influencer_index, int team_mask)
{
    if (influencer_index >= 0x148
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
            1539,
            0,
            "influencer_index doesn't index MAX_INFLUENCERS\n\t%i not in [0, %i)",
            influencer_index,
            328))
    {
        __debugbreak();
    }
    if (!g_spawnInfluencers[influencer_index].used)
        return 0;
    g_spawnInfluencers[influencer_index].teamMask = team_mask;
    return 1;
}

void __cdecl SpawnSystem_ClearEntityInfluencers(gentity_s *ent)
{
    int i; // [esp+0h] [ebp-4h]

    if (!ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp", 1555, 0, "%s", "ent"))
    {
        __debugbreak();
    }
    if (ent->s.eType == 1
        || ent->s.eType == 17
        || ent->s.eType == 14
        || ent->s.eType == 6
        || ent->s.eType == 4
        || ent->s.eType == 12)
    {
        for (i = 0; i < 328; ++i)
        {
            if (g_spawnInfluencers[i].used
                && g_spawnInfluencers[i].entity.isDefined()
                && g_spawnInfluencers[i].entity.ent() == ent)
            {
                if (g_spawnInfluencers[i].expireTime <= level.time)
                {
                    g_spawnInfluencers[i].used = 0;
                    SpawnSystem_RemoveSortedInfluencer(&g_spawnInfluencers[i]);
                }
                else
                {
                    g_spawnInfluencers[i].entity.setEnt(0);
                }
            }
        }
    }
}

void __cdecl SpawnSystem_ClearTimedOutInfluencers()
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 328; ++i)
    {
        if (g_spawnInfluencers[i].used && g_spawnInfluencers[i].expireTime && g_spawnInfluencers[i].expireTime < level.time)
        {
            g_spawnInfluencers[i].used = 0;
            SpawnSystem_RemoveSortedInfluencer(&g_spawnInfluencers[i]);
        }
    }
}

SpawnInfluencer *__cdecl SpawnSystem_GetInfluencerList()
{
    return g_spawnInfluencers;
}

void __cdecl SpawnSystem_Update()
{
    if ( g_spawnSystemDebugMode == SS_DEBUG_OFF )
    {
        SpawnSystem_ClearTimedOutInfluencers();
        SpawnSystem_UpdateInfluencerOriginAngles();
        SpawnSystem_UpdateInfluencerBounds();
        SpawnSystem_SortInfluencers();
    }
    SpawnSystem_DebugRender();
}

struct compare_spawninfluencer_sort
{
    bool operator()(const SpawnInfluencer *a, const SpawnInfluencer *b) const
    {
        return a->expireTime < b->expireTime;
    }
};

void SpawnSystem_SortInfluencers()
{
    //if (g_lastInfluencerSortTime < level.time)
    //{
    //    g_lastInfluencerSortTime = level.time;
    //    //std::_Sort<SpawnInfluencer **, int, compare_spawninfluencer_sort>(
    //    //    g_sortedSpawnInfluencers,
    //    //    &g_sortedSpawnInfluencers[g_sortedSpawnInfluencerCount],
    //    //    (4 * g_sortedSpawnInfluencerCount) >> 2,
    //    //    0);
    //
    //    std::sort(&g_sortedSpawnInfluencers[0], &g_sortedSpawnInfluencers[g_sortedSpawnInfluencerCount], 0);
    //}

    if (g_lastInfluencerSortTime < level.time)
    {
        g_lastInfluencerSortTime = level.time;

        std::sort(
            g_sortedSpawnInfluencers,
            g_sortedSpawnInfluencers + g_sortedSpawnInfluencerCount,
            compare_spawninfluencer_sort()
        );
    }
}

void SpawnSystem_UpdateInfluencerBounds()
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < g_sortedSpawnInfluencerCount; ++i )
    {
        if ( !g_sortedSpawnInfluencers[i]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                        1625,
                        0,
                        "%s",
                        "g_sortedSpawnInfluencers[i]") )
        {
            __debugbreak();
        }
        if ( !g_sortedSpawnInfluencers[i]->used
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                        1626,
                        0,
                        "%s",
                        "g_sortedSpawnInfluencers[i]->used") )
        {
            __debugbreak();
        }
        if ( g_sortedSpawnInfluencers[i]->active && g_sortedSpawnInfluencers[i]->entity.isDefined() )
        {
            SpawnSystem_ComputeInfluencerBounds(g_sortedSpawnInfluencers[i]);
            if ( g_sortedSpawnInfluencers[i]->preset->shape == INFLUENCER_SHAPE_CYLINDER )
                SpawnSystem_UpdateCylinderInfluencer(g_sortedSpawnInfluencers[i]);
        }
    }
}

void SpawnSystem_UpdateInfluencerOriginAngles()
{
    float *origin; // [esp+0h] [ebp-10h]
    gentity_s *ent; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    for ( i = 0; i < g_sortedSpawnInfluencerCount; ++i )
    {
        if ( !g_sortedSpawnInfluencers[i]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                        1653,
                        0,
                        "%s",
                        "g_sortedSpawnInfluencers[i]") )
        {
            __debugbreak();
        }
        if ( !g_sortedSpawnInfluencers[i]->used
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                        1654,
                        0,
                        "%s",
                        "g_sortedSpawnInfluencers[i]->used") )
        {
            __debugbreak();
        }
        if ( g_sortedSpawnInfluencers[i]->active )
        {
            if ( g_sortedSpawnInfluencers[i]->entity.isDefined() )
            {
                ent = g_sortedSpawnInfluencers[i]->entity.ent();
                if ( !ent->client || ent->client->ps.pm_type != 2 && ent->client->ps.pm_type != 3 )
                {
                    origin = g_sortedSpawnInfluencers[i]->origin;
                    *origin = ent->r.currentOrigin[0];
                    origin[1] = ent->r.currentOrigin[1];
                    origin[2] = ent->r.currentOrigin[2];
                    if ( g_sortedSpawnInfluencers[i]->preset->type == INFLUENCER_TYPE_WEAPON )
                    {
                        if ( !ent->client
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                                        1675,
                                        0,
                                        "%s",
                                        "ent->client") )
                        {
                            __debugbreak();
                        }
                        g_sortedSpawnInfluencers[i]->origin[2] = g_sortedSpawnInfluencers[i]->origin[2]
                                                                                                     + ent->client->ps.viewHeightCurrent;
                    }
                    if ( g_sortedSpawnInfluencers[i]->preset->shape == INFLUENCER_SHAPE_CYLINDER )
                        SpawnSystem_UpdateCylinderInfluencer(g_sortedSpawnInfluencers[i]);
                }
            }
        }
    }
}

double __cdecl SpawnSystem_InfluenceWeight(
                eInfluencerScoreCurve curve_type,
                float influencer_radius_sq,
                float distance_sq,
                float score)
{
    double result; // st7
    float dist_over_radius; // [esp+8h] [ebp-4h]

    if ( influencer_radius_sq <= 0.001
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1782,
                    0,
                    "%s",
                    "influencer_radius_sq > EQUAL_EPSILON") )
    {
        __debugbreak();
    }
    dist_over_radius = sqrtf(distance_sq * (float)(1.0 / influencer_radius_sq));
    switch ( curve_type )
    {
        case INFLUENCER_SC_LINEAR:
            result = (1.0 - dist_over_radius) * score;
            break;
        case INFLUENCER_SC_STEEP:
            result = (1.0 - dist_over_radius) * score * (1.0 - dist_over_radius);
            break;
        case INFLUENCER_SC_INVERSE_LINEAR:
            result = score * dist_over_radius;
            break;
        case INFLUENCER_SC_NEGATIVE_TO_POSITIVE:
            result = (2.0 * dist_over_radius - 1.0) * score;
            break;
        default:
            result = score;
            break;
    }
    return result;
}

double __cdecl SpawnSystem_ComputeSphereInfluence(SpawnInfluencer *influencer, SpawnPoint *point)
{
    float influencer_radius_sq; // [esp+18h] [ebp-8h]
    float distanceSq; // [esp+1Ch] [ebp-4h]

    if ( !influencer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1831,
                    0,
                    "%s",
                    "influencer") )
    {
        __debugbreak();
    }
    if ( !point
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp", 1832, 0, "%s", "point") )
    {
        __debugbreak();
    }
    distanceSq = Vec3DistanceSq(influencer->origin, point->origin);
    influencer_radius_sq = influencer->preset->radius * influencer->preset->radius;
    if ( influencer_radius_sq <= distanceSq )
        return 0.0;
    else
        return SpawnSystem_InfluenceWeight(
                         influencer->preset->score_curve,
                         influencer_radius_sq,
                         distanceSq,
                         influencer->preset->score);
}

double __cdecl SpawnSystem_ComputeCylinderInfluence(SpawnInfluencer *influencer, SpawnPoint *point)
{
    float cylinder_radius_squared; // [esp+30h] [ebp-2Ch]
    float rsquared; // [esp+34h] [ebp-28h]
    float amp; // [esp+3Ch] [ebp-20h]
    float amp_4; // [esp+40h] [ebp-1Ch]
    float amp_8; // [esp+44h] [ebp-18h]
    float udir[3]; // [esp+48h] [ebp-14h] BYREF
    float distanceSq; // [esp+54h] [ebp-8h]
    float midpoint_axial_distance; // [esp+58h] [ebp-4h]

    if ( !influencer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1853,
                    0,
                    "%s",
                    "influencer") )
    {
        __debugbreak();
    }
    if ( !point
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp", 1854, 0, "%s", "point") )
    {
        __debugbreak();
    }
    distanceSq = Vec3DistanceSq(influencer->origin, point->origin);
    amp = point->origin[0] - influencer->midPoint[0];
    amp_4 = point->origin[1] - influencer->midPoint[1];
    amp_8 = point->origin[2] - influencer->midPoint[2];
    Vec3NormalizeTo(influencer->up, udir);
    midpoint_axial_distance = fabs((float)((float)(amp * udir[0]) + (float)(amp_4 * udir[1])) + (float)(amp_8 * udir[2]));
    if ( (float)(0.5 * influencer->length) <= midpoint_axial_distance )
        return 0.0;
    rsquared = (float)((float)((float)(amp * amp) + (float)(amp_4 * amp_4)) + (float)(amp_8 * amp_8))
                     - (float)(midpoint_axial_distance * midpoint_axial_distance);
    cylinder_radius_squared = influencer->preset->radius * influencer->preset->radius;
    if ( cylinder_radius_squared <= rsquared )
        return 0.0;
    else
        return SpawnSystem_InfluenceWeight(
                         influencer->preset->score_curve,
                         cylinder_radius_squared,
                         rsquared,
                         influencer->preset->score);
}

void __cdecl SpawnSystem_ComputePointWeight(
                gentity_s *player,
                unsigned int point_team,
                unsigned int influencer_team,
                SpawnPoint *point,
                bool bIsArchiveCall)
{
    eInfluencerType type; // [esp+4h] [ebp-50h]
    eInfluencerShape shape; // [esp+8h] [ebp-4Ch]
    int influencer_type; // [esp+14h] [ebp-40h]
    int index; // [esp+18h] [ebp-3Ch]
    int sortedCount; // [esp+1Ch] [ebp-38h]
    SpawnInfluencer *influencer; // [esp+20h] [ebp-34h]
    float friendly_player_influencer_score; // [esp+24h] [ebp-30h]
    float influencer_scores[8]; // [esp+28h] [ebp-2Ch] BYREF
    int axis; // [esp+48h] [ebp-Ch]
    int teamMask; // [esp+4Ch] [ebp-8h]
    float influencer_score; // [esp+50h] [ebp-4h]

    if ( point_team >= 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1891,
                    0,
                    "point_team doesn't index MAX_SPAWNPOINT_TEAM\n\t%i not in [0, %i)",
                    point_team,
                    3) )
    {
        __debugbreak();
    }
    if ( influencer_team >= 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    1892,
                    0,
                    "influencer_team doesn't index MAX_SPAWNPOINT_TEAM\n\t%i not in [0, %i)",
                    influencer_team,
                    3) )
    {
        __debugbreak();
    }
    if ( !point
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp", 1893, 0, "%s", "point") )
    {
        __debugbreak();
    }
    axis = g_sortAxis;
    teamMask = 1 << influencer_team;
    memset(influencer_scores, 0, sizeof(influencer_scores));
    influencer_score = 0.0f;
    friendly_player_influencer_score = 0.0f;
    sortedCount = g_sortedSpawnInfluencerCount;
    for ( index = 0; index < sortedCount; ++index )
    {
        if ( !g_sortedSpawnInfluencers[index]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                        1908,
                        0,
                        "%s",
                        "g_sortedSpawnInfluencers[index]") )
        {
            __debugbreak();
        }
        influencer = g_sortedSpawnInfluencers[index];
        if ( influencer->active
            && (influencer->teamMask & teamMask) != 0
            && influencer->disableTime < level.time
            && point->origin[axis] <= influencer->bounds[1] )
        {
            if ( influencer->bounds[0] > point->origin[axis] )
                break;
            if ( !player
                || influencer->preset->type != INFLUENCER_TYPE_PLAYER
                || !influencer->entity.isDefined()
                || influencer->entity.ent() != player )
            {
                influencer_score = 0.0f;
                if ( influencer->preset->type >= (unsigned int)MAX_INFLUENCER_TYPE
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                                1961,
                                0,
                                "influencer->preset->type doesn't index MAX_INFLUENCER_TYPE\n\t%i not in [0, %i)",
                                influencer->preset->type,
                                8) )
                {
                    __debugbreak();
                }
                shape = influencer->preset->shape;
                if ( shape )
                {
                    if ( shape == INFLUENCER_SHAPE_CYLINDER )
                    {
                        influencer_score = SpawnSystem_ComputeCylinderInfluence(influencer, point);
                    }
                    else if ( shape != INFLUENCER_SHAPE_BOX
                                 && !Assert_MyHandler(
                                             "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                                             1974,
                                             0,
                                             "%s",
                                             "point != point") )
                    {
                        __debugbreak();
                    }
                }
                else
                {
                    influencer_score = SpawnSystem_ComputeSphereInfluence(influencer, point);
                }
                type = influencer->preset->type;
                if ( type == INFLUENCER_TYPE_PLAYER )
                {
                    if ( spawnsystem_friend_influencer_stacking->current.enabled || influencer_score <= 0.0 )
                    {
LABEL_48:
                        influencer_scores[influencer->preset->type] = influencer_scores[influencer->preset->type] + influencer_score;
                        continue;
                    }
                    if ( influencer_score > friendly_player_influencer_score )
                        friendly_player_influencer_score = influencer_score;
                }
                else
                {
                    if ( type != INFLUENCER_TYPE_GAME_MODE )
                        goto LABEL_48;
                    if ( influencer_score != 0.0
                        && fabs(influencer_score) > fabs(influencer_scores[6]) )
                    {
                        influencer_scores[6] = influencer_score;
                    }
                }
            }
        }
    }
    if ( !bIsArchiveCall )
    {
        point->weight[point_team] = point->baseWeight[point_team];
        for ( influencer_type = 0; influencer_type < 8; ++influencer_type )
            point->weight[point_team] = point->weight[point_team] + influencer_scores[influencer_type];
        point->weight[point_team] = point->weight[point_team] + friendly_player_influencer_score;
    }
}

void __cdecl SpawnSystem_ComputePointWeights(gentity_s *player, unsigned int point_team, unsigned int influencer_team)
{
    int index; // [esp+0h] [ebp-10h]
    SpawnPoint *spList; // [esp+4h] [ebp-Ch]
    int spCount; // [esp+8h] [ebp-8h]

    if (point_team >= 3
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
            2055,
            0,
            "point_team doesn't index MAX_SPAWNPOINT_TEAM\n\t%i not in [0, %i)",
            point_team,
            3))
    {
        __debugbreak();
    }
    if (influencer_team >= 3
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
            2056,
            0,
            "influencer_team doesn't index MAX_SPAWNPOINT_TEAM\n\t%i not in [0, %i)",
            influencer_team,
            3))
    {
        __debugbreak();
    }
    g_sortedTeamSpawnPoints[point_team].count = 0;
    spCount = SpawnSystem_GetSpawnPointCount();
    spList = SpawnSystem_GetSpawnPoints();
    for (index = 0; index < spCount; ++index)
    {
        if (((1 << point_team) & spList[index].teamMask) != 0)
        {
            if (g_spawnSystemDebugMode != SS_DEBUG_ARCHIVE_VIEW)
                SpawnSystem_ComputePointWeight(player, point_team, influencer_team, &spList[index], 0);
            g_sortedTeamSpawnPoints[point_team].spawnPointsByIndex[g_sortedTeamSpawnPoints[point_team].count++] = index;
        }
    }
}

SpawnPoint *__cdecl SpawnSystem_GetSpawnPoints()
{
    return g_spawnPoints;
}

int __cdecl SpawnSystem_GetSpawnPointCount()
{
    return g_spawnPointCount;
}

void __cdecl SpawnSystem_SortPointsByScore(unsigned int team)
{
    if ( team >= 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
                    2092,
                    0,
                    "team doesn't index MAX_SPAWNPOINT_TEAM\n\t%i not in [0, %i)",
                    team,
                    3) )
    {
        __debugbreak();
    }
    SpawnSystem_SortPointsForTeamByScore(team);
}

int __cdecl SpawnSystem_UpdateSpawnPointsForTeam(unsigned int point_team, unsigned int influencer_team)
{
    if (point_team >= 3
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
            2106,
            0,
            "point_team doesn't index MAX_SPAWNPOINT_TEAM\n\t%i not in [0, %i)",
            point_team,
            3))
    {
        __debugbreak();
    }
    if (influencer_team >= 3
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
            2107,
            0,
            "influencer_team doesn't index MAX_SPAWNPOINT_TEAM\n\t%i not in [0, %i)",
            influencer_team,
            3))
    {
        __debugbreak();
    }
    if (g_lastPointPlayer[point_team] != 1023)
    {
        g_sortedTeamSpawnPoints[point_team].sortTime = 0;
        g_lastPointComputeTime[point_team] = 0;
    }
    SpawnSystem_ComputePointWeights(0, point_team, influencer_team);
    SpawnSystem_SortPointsByScore(point_team);
    g_lastPointPlayer[point_team] = 1023;
    return g_sortedTeamSpawnPoints[point_team].count;
}

int __cdecl SpawnSystem_UpdateSpawnPointsForPlayer(
                gentity_s *ent,
                unsigned int point_team,
                unsigned int influencer_team)
{
    if (point_team >= 3
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
            2130,
            0,
            "point_team doesn't index MAX_SPAWNPOINT_TEAM\n\t%i not in [0, %i)",
            point_team,
            3))
    {
        __debugbreak();
    }
    if (influencer_team >= 3
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
            2131,
            0,
            "influencer_team doesn't index MAX_SPAWNPOINT_TEAM\n\t%i not in [0, %i)",
            influencer_team,
            3))
    {
        __debugbreak();
    }
    if (!ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp", 2132, 0, "%s", "ent"))
    {
        __debugbreak();
    }
    if (!ent->client
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
            2133,
            0,
            "%s",
            "ent->client"))
    {
        __debugbreak();
    }
    if (ent->s.number != g_lastPointPlayer[point_team])
    {
        g_sortedTeamSpawnPoints[point_team].sortTime = 0;
        g_lastPointComputeTime[point_team] = 0;
    }
    SpawnSystem_ComputePointWeights(ent, point_team, influencer_team);
    SpawnSystem_SortPointsByScore(point_team);
    g_lastPointPlayer[point_team] = ent->s.number;
    return g_sortedTeamSpawnPoints[point_team].count;
}

int __cdecl SpawnSystem_GetSortedPointEntNum(unsigned int team, unsigned int index)
{
    if (team >= 3
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
            2203,
            0,
            "team doesn't index MAX_SPAWNPOINT_TEAM\n\t%i not in [0, %i)",
            team,
            3))
    {
        __debugbreak();
    }
    if (index >= g_sortedTeamSpawnPoints[team].count
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
            2204,
            0,
            "index doesn't index g_sortedTeamSpawnPoints[team].count\n\t%i not in [0, %i)",
            index,
            g_sortedTeamSpawnPoints[team].count))
    {
        __debugbreak();
    }
    if (g_sortedTeamSpawnPoints[team].spawnPointsByIndex[index] >= (unsigned int)g_spawnPointCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
            2205,
            0,
            "g_sortedTeamSpawnPoints[team].spawnPointsByIndex[index] doesn't index g_spawnPointCount\n\t%i not in [0, %i)",
            g_sortedTeamSpawnPoints[team].spawnPointsByIndex[index],
            g_spawnPointCount))
    {
        __debugbreak();
    }
    if (g_sortedTeamSpawnPoints[team].spawnPointsByIndex[index] >= (unsigned int)g_spawnPointCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawnsystem_mp.cpp",
            2206,
            0,
            "g_sortedTeamSpawnPoints[team].spawnPointsByIndex[index] doesn't index g_spawnPointCount\n\t%i not in [0, %i)",
            g_sortedTeamSpawnPoints[team].spawnPointsByIndex[index],
            g_spawnPointCount))
    {
        __debugbreak();
    }
    return g_spawnPoints[g_sortedTeamSpawnPoints[team].spawnPointsByIndex[index]].entNum;
}

char __cdecl SpawnSystem_IsSpawnPointVisible(float *origin, float *angles, int other_team, gentity_s *ignore_entity)
{
    if ( SpawnSystem_IsSpawnPointVisible(origin, angles, other_team, ignore_entity, 0) )
        return 1;
    SpawnSystem_IsSpawnPointVisible(origin, angles, other_team, ignore_entity, 1);
    return 0;
}

char __cdecl SpawnSystem_IsSpawnPointVisible(
                float *origin,
                float *angles,
                int other_team,
                gentity_s *ignore_entity,
                bool render_mode_success)
{
    float v6; // [esp+10h] [ebp-8Ch]
    float v7; // [esp+14h] [ebp-88h]
    float v8; // [esp+20h] [ebp-7Ch]
    float v9; // [esp+24h] [ebp-78h]
    int index; // [esp+28h] [ebp-74h]
    float diff_4; // [esp+30h] [ebp-6Ch]
    int hitnum; // [esp+3Ch] [ebp-60h] BYREF
    float point_dir[3]; // [esp+68h] [ebp-34h] BYREF
    float sight_point[3]; // [esp+74h] [ebp-28h] BYREF
    float player_sight_origin[3]; // [esp+80h] [ebp-1Ch] BYREF
    int iClipMask; // [esp+8Ch] [ebp-10h]
    int iIgnoreEntNum; // [esp+90h] [ebp-Ch]
    gentity_s *player; // [esp+94h] [ebp-8h]
    float squared_visibility_distance; // [esp+98h] [ebp-4h]

    player = 0;
    iIgnoreEntNum = 1023;
    iClipMask = 8415265;
    squared_visibility_distance = spawnsystem_sight_check_max_distance->current.value
                                                            * spawnsystem_sight_check_max_distance->current.value;
    AngleVectors(angles, point_dir, 0, 0);
    v8 = origin[1];
    v9 = origin[2] + 60.0;
    sight_point[0] = *origin;
    sight_point[1] = v8;
    sight_point[2] = v9;
    //col_context_t::col_context_t(&context, iClipMask);
    col_context_t context(iClipMask); // [esp+40h] [ebp-5Ch] BYREF
    //col_context_t::init_locational(&context, iIgnoreEntNum);
    context.init_locational(iIgnoreEntNum);
    hitnum = -1;
    for ( index = 0; index < com_maxclients->current.integer; ++index )
    {
        player = &g_entities[index];
        if ( player->client )
        {
            if ( ignore_entity != player
                && player->client->sess.connected == CON_CONNECTED
                && player->client->sess.sessionState == SESS_STATE_PLAYING
                && player->client->sess.cs.team == other_team )
            {
                diff_4 = player->client->ps.origin[1] - origin[1];
                if ( (float)((float)((float)((float)(player->client->ps.origin[0] - *origin)
                                                                     * (float)(player->client->ps.origin[0] - *origin))
                                                     + (float)(diff_4 * diff_4))
                                     + (float)(0.0 * 0.0)) <= squared_visibility_distance )
                {
                    v6 = player->client->ps.origin[1];
                    v7 = player->client->ps.origin[2] + 60.0;
                    player_sight_origin[0] = player->client->ps.origin[0];
                    player_sight_origin[1] = v6;
                    player_sight_origin[2] = v7;
                    if ( !render_mode_success && SV_SightTracePoint(&hitnum, player_sight_origin, sight_point, &context) )
                    {
                        if ( spawnsystem_debug->current.enabled && spawnsystem_debug_visibility->current.enabled )
                        {
                            SpawnSystem_DebugRenderVisibilityBox(origin, angles, colorRed);
                            SpawnSystem_DebugRenderVisibilityCheck(
                                origin,
                                angles,
                                player->client->ps.origin,
                                player->r.currentAngles,
                                colorRed,
                                60.0,
                                ignore_entity,
                                1);
                        }
                        return 1;
                    }
                    if ( render_mode_success
                        && spawnsystem_debug->current.enabled
                        && spawnsystem_debug_visibility->current.enabled )
                    {
                        SpawnSystem_DebugRenderVisibilityCheck(
                            origin,
                            angles,
                            player->client->ps.origin,
                            player->r.currentAngles,
                            colorGreen,
                            60.0,
                            ignore_entity,
                            0);
                    }
                }
            }
        }
    }
    if ( render_mode_success && spawnsystem_debug->current.enabled && spawnsystem_debug_visibility->current.enabled )
        SpawnSystem_DebugRenderVisibilityBox(origin, angles, colorGreen);
    return 0;
}

void __cdecl SpawnSystem_DebugRenderVisibilityBox(float *point1, float *angles1, const float *color)
{
    float mins[3]; // [esp+10h] [ebp-1Ch] BYREF
    float maxs[3]; // [esp+1Ch] [ebp-10h] BYREF
    int time; // [esp+28h] [ebp-4h]

    mins[0] = -16.0f;
    mins[1] = -16.0f;
    mins[2] = 0.0f;
    maxs[0] = 16.0f;
    maxs[1] = 16.0f;
    maxs[2] = 72.0f;
    time = 500;
    if ( spawnsystem_debug_visibility_time )
        time = spawnsystem_debug_visibility_time->current.integer;
    CG_DebugBox(point1, mins, maxs, angles1[1], color, 1, time);
}

void __cdecl SpawnSystem_DebugRenderVisibilityCheck(
                float *point1,
                float *angles1,
                float *point2,
                float *angles2,
                const float *color,
                float height_offset,
                gentity_s *ignore_entity,
                bool collision_test)
{
    col_context_t context; // [esp+4Ch] [ebp-44h] BYREF
    float end[3]; // [esp+74h] [ebp-1Ch] BYREF
    float start[3]; // [esp+80h] [ebp-10h] BYREF
    int time; // [esp+8Ch] [ebp-4h]

    //col_context_t::col_context_t(&context);
    time = 500;
    if ( spawnsystem_debug_visibility_time )
        time = spawnsystem_debug_visibility_time->current.integer;
    start[0] = *point1;
    start[1] = point1[1];
    start[2] = point1[2];
    start[2] = start[2] + height_offset;
    end[0] = *point2;
    end[1] = point2[1];
    end[2] = point2[2];
    end[2] = end[2] + height_offset;
    if ( collision_test )
    {
        trace_t trace; // [esp+10h] [ebp-80h] BYREF
        if ( ignore_entity )
            G_TraceCapsule(&trace, start, vec3_origin, vec3_origin, end, ignore_entity->s.number, 0x806833, &context);
        else
            G_TraceCapsule(&trace, start, vec3_origin, vec3_origin, end, 1023, 0x806833, &context);
        Vec3Lerp(start, end, trace.fraction, end);
    }
    CG_DebugLine(start, end, color, 1, time);
}

void __cdecl SpawnSystem_DebugSideSwitch(bool enabled)
{
    Dvar_SetBool((dvar_s *)spawnsystem_debug_sideswitched, enabled);
}

void SpawnSystem_DebugRender()
{
    float diff; // [esp+Ch] [ebp-2Ch]
    float newSizeScale; // [esp+10h] [ebp-28h]
    SpawnInfluencer *influencerList; // [esp+14h] [ebp-24h]
    int j; // [esp+18h] [ebp-20h]
    gentity_s *ent; // [esp+1Ch] [ebp-1Ch]
    int i; // [esp+20h] [ebp-18h]
    int influencer_team; // [esp+24h] [ebp-14h]
    int point_team; // [esp+2Ch] [ebp-Ch]
    gentity_s *localPlayer; // [esp+30h] [ebp-8h]
    gentity_s *player; // [esp+34h] [ebp-4h]

    if ( spawnsystem_debug->current.enabled )
    {
        point_team = 7;
        influencer_team = 7;
        g_debugHighlightedSpawnPoint = -1;
        localPlayer = g_entities;
        if ( !g_entities[0].client || g_entities[0].client->sess.connected != CON_CONNECTED )
            localPlayer = 0;
        player = 0;
        if ( *(_BYTE *)spawnsystem_debug_team->current.integer )
        {
            if ( !_stricmp(spawnsystem_debug_team->current.string, "allies") )
            {
                if ( spawnsystem_debug_sideswitched->current.enabled )
                    point_team = 2;
                else
                    point_team = 4;
                influencer_team = 4;
            }
            else if ( !_stricmp(spawnsystem_debug_team->current.string, "axis") )
            {
                if ( spawnsystem_debug_sideswitched->current.enabled )
                    point_team = 4;
                else
                    point_team = 2;
                influencer_team = 2;
            }
            else if ( !_stricmp(spawnsystem_debug_team->current.string, "free") )
            {
                point_team = 1;
                influencer_team = 1;
            }
        }
        else
        {
            for ( i = 0; i < com_maxclients->current.integer; ++i )
            {
                ent = &g_entities[i];
                if ( ent->client->sess.connected == CON_CONNECTED
                    && (!*(_BYTE *)spawnsystem_debug_player->current.integer
                     || !_stricmp(ent->client->sess.cs.name, spawnsystem_debug_player->current.string)) )
                {
                    point_team = 1 << ent->client->sess.cs.team;
                    influencer_team = point_team;
                    if ( spawnsystem_debug_sideswitched->current.enabled )
                    {
                        if ( ent->client->sess.cs.team == TEAM_AXIS )
                            point_team = 4;
                        else
                            point_team = 2;
                    }
                    player = &g_entities[i];
                    break;
                }
            }
        }
        if ( g_spawnSystemDebugMode == SS_DEBUG_OFF || g_spawnSystemDebugMode > SS_DEBUG_ARCHIVE_VIEW )
        {
            if ( (point_team & 1) != 0 )
                SpawnSystem_DebugUpdateSpawnPoints(player, 0, 0);
            if ( (point_team & 2) != 0 )
                SpawnSystem_DebugUpdateSpawnPoints(player, 1u, spawnsystem_debug_sideswitched->current.enabled + 1);
            if ( (point_team & 4) != 0 )
                SpawnSystem_DebugUpdateSpawnPoints(player, 2u, 2 - spawnsystem_debug_sideswitched->current.enabled);
            SpawnSystem_SetSpawnPointsOrder(0);
        }
        if ( spawnsystem_debug_influencers->current.integer > 0 )
        {
            influencerList = SpawnSystem_GetInfluencerList();
            if ( localPlayer )
            {
                newSizeScale = 1.0 - CMD_GetAnalogButtonValue(&localPlayer->client->sess.cmd, 0x13u);
                diff = newSizeScale - sizeScale;
                if ( fabs(newSizeScale - sizeScale) > 0.001 )
                {
                    if ( fabs((float)(diff * speedScale) * (float)((float)level.frametime * 0.001)) <= fabs(diff) )
                        sizeScale = sizeScale + (float)((float)(diff * speedScale) * (float)((float)level.frametime * 0.001));
                    else
                        sizeScale = newSizeScale;
                }
                else
                {
                    sizeScale = newSizeScale;
                }
            }
            for ( j = 0; j < 328; ++j )
            {
                if ( influencerList[j].used
                    && influencerList[j].active
                    && (influencer_team & influencerList[j].teamMask) != 0
                    && influencerList[j].disableTime < level.time
                    && (!player
                     || !influencerList[j].entity.isDefined()
                     || influencerList[j].entity.ent() != player) )
                {
                    if ( influencerList[j].preset->shape )
                    {
                        //if ( influencerList[j].preset->shape == INFLUENCER_SHAPE_CYLINDER )
                            //BLOPS_NULLSUB();
                    }
                    else
                    {
                        //BLOPS_NULLSUB();
                    }
                }
            }
        }
        if ( spawnsystem_debug_points->current.integer )
            SpawnSystem_DebugRenderTeamPoints(point_team);
        if ( spawnsystem_debug_point_weights->current.enabled )
            SpawnSystem_DebugRenderTeamPointWeights(point_team);
#if 0
        if ( spawnsystem_debug_best_points->current.enabled )
        {
            if ( (point_team & 1) != 0 )
                //BLOPS_NULLSUB();
            if ( (point_team & 2) != 0 )
                //BLOPS_NULLSUB();
            if ( (point_team & 4) != 0 )
                //BLOPS_NULLSUB();
        }
        if ( spawnsystem_debug_showclients->current.enabled )
            //BLOPS_NULLSUB();
#endif
    }
}

void __cdecl SpawnSystem_DebugRenderTeamPointWeights(int teamMask)
{
    char *v1; // eax
    char *v2; // eax
    char *v3; // eax
    float *v4; // [esp+14h] [ebp-34h]
    float *v5; // [esp+18h] [ebp-30h]
    float *color; // [esp+1Ch] [ebp-2Ch]
    int index; // [esp+20h] [ebp-28h]
    float origin[3]; // [esp+24h] [ebp-24h] BYREF
    float baseScore; // [esp+30h] [ebp-18h]
    float start_height; // [esp+34h] [ebp-14h]
    int spCount; // [esp+38h] [ebp-10h]
    float height_inc; // [esp+3Ch] [ebp-Ch]
    SpawnPoint *SpawnPoints; // [esp+40h] [ebp-8h]
    float score; // [esp+44h] [ebp-4h]

    start_height = 45.0f;
    height_inc = 12.0f;
    spCount = SpawnSystem_GetSpawnPointCount();
    SpawnPoints = SpawnSystem_GetSpawnPoints();
    for ( index = 0; index < spCount; ++index )
    {
        if ( (teamMask & SpawnPoints->teamMask) != 0 )
        {
            origin[0] = SpawnPoints->origin[0];
            origin[1] = SpawnPoints->origin[1];
            origin[2] = SpawnPoints->origin[2];
            origin[2] = origin[2] + start_height;
            if ( (teamMask & 1) != 0 )
            {
                score = SpawnPoints->weight[0];
                baseScore = SpawnPoints->baseWeight[0];
                if ( score >= 0.0 )
                    color = (float *)colorGreen;
                else
                    color = (float *)colorRed;
                v1 = va("%.0f\\%.0f", score, baseScore);
                CL_AddDebugString(origin, color, font_height, v1, 1);
                origin[2] = origin[2] + height_inc;
            }
            if ( (teamMask & 2) != 0 )
            {
                score = SpawnPoints->weight[1];
                baseScore = SpawnPoints->baseWeight[1];
                if ( score >= 0.0 )
                    v5 = (float *)colorGreen;
                else
                    v5 = (float *)colorRed;
                v2 = va("%.0f\\%.0f", score, baseScore);
                CL_AddDebugString(origin, v5, font_height, v2, 1);
                origin[2] = origin[2] + height_inc;
            }
            if ( (teamMask & 4) != 0 )
            {
                score = SpawnPoints->weight[2];
                baseScore = SpawnPoints->baseWeight[2];
                if ( score >= 0.0 )
                    v4 = (float *)colorGreen;
                else
                    v4 = (float *)colorRed;
                v3 = va("%.0f\\%.0f", score, baseScore);
                CL_AddDebugString(origin, v4, font_height, v3, 1);
                origin[2] = origin[2] + height_inc;
            }
        }
        ++SpawnPoints;
    }
}

void __cdecl SpawnSystem_DebugRenderTeamPoints(int teamMask)
{
    float yaw; // [esp+4h] [ebp-1Ch]
    float angles[3]; // [esp+8h] [ebp-18h] BYREF
    int index; // [esp+14h] [ebp-Ch]
    int spCount; // [esp+18h] [ebp-8h]
    SpawnPoint *SpawnPoints; // [esp+1Ch] [ebp-4h]

    spCount = SpawnSystem_GetSpawnPointCount();
    SpawnPoints = SpawnSystem_GetSpawnPoints();
    index = 0;
    while ( index < spCount )
    {
        if ( (teamMask & SpawnPoints->teamMask) != 0 )
        {
            if ( index == g_debugHighlightedSpawnPoint )
                SpawnSystem_DebugRenderSpawnPointBox(SpawnPoints, colorWhite, spawnsystem_debug_points->current.integer < 2);
            else
                SpawnSystem_DebugRenderSpawnPointBox(SpawnPoints, colorYellow, spawnsystem_debug_points->current.integer < 2);
            if ( g_spawnSystemDebugMode == SS_DEBUG_OFF
                && g_entities[0].client
                && g_entities[0].client->sess.connected == CON_CONNECTED )
            {
                yaw = SpawnPoints->yaw;
                angles[0] = 0.0f;
                angles[1] = yaw;
                angles[2] = 0.0f;
                SpawnSystem_IsSpawnPointVisible(SpawnPoints->origin, angles, g_entities[0].client->sess.cs.team, 0);
            }
        }
        ++index;
        ++SpawnPoints;
    }
}

void __cdecl SpawnSystem_DebugRenderSpawnPointBox(SpawnPoint *sp, const float *color, int bDepthTest)
{
    float origin[3]; // [esp+10h] [ebp-58h] BYREF
    float mins[3]; // [esp+1Ch] [ebp-4Ch] BYREF
    float end[3]; // [esp+28h] [ebp-40h] BYREF
    float start[3]; // [esp+34h] [ebp-34h] BYREF
    float forward[3]; // [esp+40h] [ebp-28h] BYREF
    float angles[3]; // [esp+4Ch] [ebp-1Ch] BYREF
    float maxs[3]; // [esp+58h] [ebp-10h] BYREF
    float height_offset; // [esp+64h] [ebp-4h]

    mins[0] = -16.0f;
    mins[1] = -16.0f;
    mins[2] = 0.0f;
    maxs[0] = 16.0f;
    maxs[1] = 16.0f;
    maxs[2] = 72.0f;
    height_offset = 45.0f;
    origin[0] = sp->origin[0];
    origin[1] = sp->origin[1];
    origin[2] = sp->origin[2];
    origin[2] = origin[2] - g_legacySpawnPointRaiseHeight;
    memset(angles, 0, sizeof(angles));
    angles[1] = sp->yaw;
    CG_DebugBox(origin, mins, maxs, angles[1], color, bDepthTest, 0);
    AngleVectors(angles, forward, 0, 0);
    start[0] = origin[0];
    start[1] = origin[1];
    start[2] = origin[2] + height_offset;
    end[0] = (float)(48.0 * forward[0]) + origin[0];
    end[1] = (float)(48.0 * forward[1]) + origin[1];
    end[2] = (float)(48.0 * forward[2]) + (float)(origin[2] + height_offset);
    G_DebugLine(start, end, color, 1);
}

void __cdecl SpawnSystem_SetSpawnPointsOrder(int team)
{
    int index; // [esp+4h] [ebp-8h]
    SpawnPoint *SpawnPoints; // [esp+8h] [ebp-4h]

    SpawnPoints = SpawnSystem_GetSpawnPoints();
    for (index = 0; index < g_sortedTeamSpawnPoints[team].count; ++index)
        SpawnPoints[g_sortedTeamSpawnPoints[team].spawnPointsByIndex[index]].orderNum = index;
}

void __cdecl SpawnSystem_DebugUpdateSpawnPoints(
                gentity_s *player,
                unsigned int point_team,
                unsigned int influencer_team)
{
    if ( player )
        SpawnSystem_UpdateSpawnPointsForPlayer(player, point_team, influencer_team);
    else
        SpawnSystem_UpdateSpawnPointsForTeam(point_team, influencer_team);
}

