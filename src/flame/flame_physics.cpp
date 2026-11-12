#include "flame_physics.h"
#include "flame_system.h"
#include "flame_class_stream.h"
#include "tl/base/include/tl_system.h"
#include <DynEntity/DynEntity_coll.h>
#include <server_mp/sv_main_mp.h>
#include <qcommon/cm_tracebox.h>
#include <server/sv_world.h>
#include <game_mp/g_main_mp.h>
#include <clientscript/scr_const.h>
#include <qcommon/cm_load.h>
#include <DynEntity/DynEntity_server.h>
#include <glass/glass_server.h>
#include <game_mp/g_combat_mp.h>
#include <DynEntity/DynEntity_client.h>
#include <DynEntity/DynEntity_load_obj.h>
#include <qcommon/dobj_management.h>
#include <cgame_mp/cg_local_mp.h>
#include <cgame/cg_world.h>
#include <physics/destructible.h>
#include <glass/glass_client.h>
#include <game_mp/g_trigger_mp.h>
#include <bgame/bg_fire.h>
#include <game/g_weapon.h>
#include <game/g_debug.h>
#include <gfx_d3d/r_dpvs.h>
#include <qcommon/cm_world.h>
#include <algorithm>
#include <client/splitscreen.h>
#include <cgame/cg_drawtools.h>

cdl_proftimer sv_flame_proftimer;
cdl_proftimer cl_flame_proftimer;

colgeom_visitor_inlined_t<500> cl_flame_chunks_proximity_cache;
colgeom_visitor_inlined_t<500> sv_flame_chunks_proximity_cache;

float FLAME_OVERCLIP = 0.001f;

float mins[3] = { -8.0, -8.0, -8.0 };
float maxs[3] = { 8.0, 8.0, 8.0 };
float size[3] = { 8.0, 8.0, 8.0 };



void __cdecl Flame_ClipVelocity(const float *in, const float *normal, float *out)
{
    float v3; // xmm2_4
    float v4; // [esp+0h] [ebp-Ch]

    v3 = (float)((float)(*in * *normal) + (float)(in[1] * normal[1])) + (float)(in[2] * normal[2]);
    //LODWORD(v4) = COERCE_UNSIGNED_INT(v3 - (float)(FLAME_OVERCLIP * fabs(v3))) ^ _mask__NegFloat_;
    v4 = -(v3 - (float)(FLAME_OVERCLIP * fabs(v3)));
    *out = (float)(v4 * *normal) + *in;
    out[1] = (float)(v4 * normal[1]) + in[1];
    out[2] = (float)(v4 * normal[2]) + in[2];
}

void __cdecl trace_sphere(trace_t *trace, const float *start, const float *end, float radius, col_context_t *context)
{
    const CollisionAabbTree *tree; // [esp+20h] [ebp-70h]
    const float *v8; // [esp+28h] [ebp-68h]
    const CollisionAabbTree *v9; // [esp+2Ch] [ebp-64h]
    const float *halfSize; // [esp+30h] [ebp-60h]
    const CollisionAabbTree *v11; // [esp+34h] [ebp-5Ch]
    int i; // [esp+40h] [ebp-50h]
    float dir[3]; // [esp+44h] [ebp-4Ch] BYREF
    float rvec[3]; // [esp+50h] [ebp-40h]
    const col_prim_t *prim; // [esp+5Ch] [ebp-34h]
    float mx[3]; // [esp+60h] [ebp-30h]
    float bounds[2][3]; // [esp+6Ch] [ebp-24h] BYREF
    float mn[3]; // [esp+84h] [ebp-Ch]

    rvec[0] = radius;
    rvec[1] = radius;
    rvec[2] = radius;
    Vec3Min(start, end, bounds[0]);
    Vec3Max(start, end, bounds[1]);
    bounds[0][0] = bounds[0][0] - radius;
    bounds[0][1] = bounds[0][1] - radius;
    bounds[0][2] = bounds[0][2] - radius;
    bounds[1][0] = bounds[1][0] + radius;
    bounds[1][1] = bounds[1][1] + radius;
    bounds[1][2] = bounds[1][2] + radius;
    dir[0] = *end - *start;
    dir[1] = end[1] - start[1];
    dir[2] = end[2] - start[2];
    trace->fraction = 1.0f;
    prim = context->prims;
    for ( i = 0; i < context->nprims; ++i )
    {
        if ( prim->type )
        {
            tree = prim->tree;
            if ( tree->halfSize[0] >= bounds[0][0]
                && prim->tree->halfSize[1] >= bounds[0][1]
                && prim->tree->halfSize[2] >= bounds[0][2]
                && bounds[1][0] >= tree->origin[0]
                && bounds[1][1] >= tree->origin[1]
                && bounds[1][2] >= tree->origin[2] )
            {
                trace_sphere_through_brush(
                    start,
                    dir,
                    &trace->fraction,
                    radius,
                    prim->brush,
                    trace->normal.vec.v,
                    &trace->sflags,
                    NULL);
            }
        }
        else
        {
            halfSize = prim->tree->halfSize;
            v11 = prim->tree;
            mn[0] = v11->origin[0] - *halfSize;
            mn[1] = v11->origin[1] - halfSize[1];
            mn[2] = v11->origin[2] - halfSize[2];
            v8 = prim->tree->halfSize;
            v9 = prim->tree;
            mx[0] = v9->origin[0] + *v8;
            mx[1] = v9->origin[1] + v8[1];
            mx[2] = v9->origin[2] + v8[2];
            if ( mx[0] >= bounds[0][0]
                && mx[1] >= bounds[0][1]
                && mx[2] >= bounds[0][2]
                && bounds[1][0] >= mn[0]
                && bounds[1][1] >= mn[1]
                && bounds[1][2] >= mn[2] )
            {
                trace_sphere_through_partition(
                    start,
                    dir,
                    &trace->fraction,
                    radius,
                    prim->tree,
                    trace->normal.vec.v,
                    &trace->sflags);
            }
        }
        ++prim;
    }
    if ( trace->fraction == 1.0 )
    {
        if ( !trace
            && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\physics\\../qcommon/cm_public.h", 175, 0, "%s", "trace") )
        {
            __debugbreak();
        }
        trace->hitType = TRACE_HITTYPE_NONE;
        trace->hitId = 0;
    }
    else
    {
        if ( !trace
            && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\physics\\../qcommon/cm_public.h", 175, 0, "%s", "trace") )
        {
            __debugbreak();
        }
        trace->hitType = TRACE_HITTYPE_ENTITY;
        trace->hitId = 1022;
    }
    if ( trace->fraction > 0.0 )
    {
        if ( trace->fraction > 1.0 )
            trace->fraction = 1.0f;
    }
    else
    {
        trace->fraction = 0.0f;
    }
}

void __cdecl Flame_Phys_Collision(
                bool is_server,
                trace_t *trace,
                flameGeneric_s *gen,
                col_context_t *context,
                unsigned __int16 (*dynEnts)[2][100],
                const int *dynEntsCount,
                int *ents,
                int entsCount,
                int *close_characters,
                int *close_characters_count,
                int *close_triggers,
                int *close_triggers_count)
{
    float endPos[3]; // [esp+20h] [ebp-18h] BYREF
    float startPos[3]; // [esp+2Ch] [ebp-Ch] BYREF

    startPos[0] = gen->phys.origin[0];
    startPos[1] = gen->phys.origin[1];
    startPos[2] = gen->phys.origin[2];
    endPos[0] = gen->phys.newPos[0];
    endPos[1] = gen->phys.newPos[1];
    endPos[2] = gen->phys.newPos[2];
    if ( ((LODWORD(startPos[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(startPos[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(startPos[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_physics.cpp",
                    392,
                    0,
                    "%s",
                    "!IS_NAN((startPos)[0]) && !IS_NAN((startPos)[1]) && !IS_NAN((startPos)[2])") )
    {
        __debugbreak();
    }
    if ( is_server )
    {
        sv_flame_proftimer.stamp = tlPcGetTick();
        ++sv_flame_proftimer.calls;
        Flame_Server_Trace(
            trace,
            gen,
            startPos,
            mins,
            maxs,
            endPos,
            context,
            dynEnts,
            dynEntsCount,
            ents,
            entsCount,
            close_characters,
            close_characters_count,
            close_triggers,
            close_triggers_count);
        //sv_flame_proftimer.value += tlPcGetTick() - sv_flame_proftimer.stamp;
        sv_flame_proftimer.value += tlPcGetTick() - sv_flame_proftimer.stamp;
    }
    else
    {
        cl_flame_proftimer.stamp = tlPcGetTick();
        ++cl_flame_proftimer.calls;
        Flame_Client_Trace(
            trace,
            gen,
            startPos,
            mins,
            maxs,
            endPos,
            context,
            dynEnts,
            dynEntsCount,
            ents,
            entsCount,
            close_characters,
            close_characters_count);
        //cl_flame_proftimer.value += tlPcGetTick() - cl_flame_proftimer.stamp;
        cl_flame_proftimer.value += tlPcGetTick() - cl_flame_proftimer.stamp;
    }
}

float radius_2 = 1.0f;
void __cdecl Flame_Server_Trace(
                trace_t *trace,
                flameGeneric_s *gen,
                const float *startPos,
                const float *mins,
                const float *maxs,
                float *endPos,
                col_context_t *context,
                unsigned __int16 (*dynEnts)[2][100],
                const int *dynEntsCount,
                int *ents,
                int entsCount,
                int *close_characters,
                int *close_characters_count,
                int *close_triggers,
                int *close_triggers_count)
{
    float hitDir[3]; // [esp+68h] [ebp-138h] BYREF
    float hitPos[3]; // [esp+74h] [ebp-12Ch] BYREF
    const DynEntityColl *coll; // [esp+80h] [ebp-120h]
    DynEntityCollType colType; // [esp+84h] [ebp-11Ch]
    unsigned __int16 id; // [esp+88h] [ebp-118h]
    int j; // [esp+8Ch] [ebp-114h]
    int num; // [esp+90h] [ebp-110h]
    int drawType; // [esp+94h] [ebp-10Ch]
    trace_t old_trace; // [esp+98h] [ebp-108h] BYREF
    float center[3]; // [esp+D0h] [ebp-D0h] BYREF
    float size; // [esp+DCh] [ebp-C4h]
    float dist2; // [esp+E0h] [ebp-C0h]
    bool character_collision; // [esp+E7h] [ebp-B9h]
    gentity_s *ent; // [esp+E8h] [ebp-B8h]
    int i; // [esp+ECh] [ebp-B4h]
    pointtrace_t clip; // [esp+F0h] [ebp-B0h] BYREF
    IgnoreEntParams ignoreEntParams; // [esp+134h] [ebp-6Ch] BYREF
    gentity_s *owner; // [esp+140h] [ebp-60h]
    trace_t trace2; // [esp+144h] [ebp-5Ch] BYREF
    float bounds[2][3]; // [esp+180h] [ebp-20h] BYREF
    unsigned int entnum; // [esp+198h] [ebp-8h]
    svEntity_s *check; // [esp+19Ch] [ebp-4h]

    //TraceExtents::TraceExtents(&clip.extents);
    trace_sphere(trace, startPos, endPos, radius_2, context);
    if (trace->fraction > 0.0)
    {
        Vec3Min(startPos, endPos, bounds[0]);
        Vec3Max(startPos, endPos, bounds[1]);
        clip.contentmask = 0x2C0EC33;
        *(_QWORD *)clip.extents.start.vec.v = *(_QWORD *)startPos;
        clip.extents.start.vec.v[2] = startPos[2];
        *(_QWORD *)clip.extents.end.vec.v = *(_QWORD *)endPos;
        clip.extents.end.vec.v[2] = endPos[2];
        CM_CalcTraceExtents(&clip.extents);
        clip.bLocational = 1;
        clip.priorityMap = bulletPriorityMap;
        SV_SetupIgnoreEntParams(&ignoreEntParams, gen->stream->entityNum);
        owner = &g_entities[gen->stream->entityNum];
        ignoreEntParams.ignoreChildren = 0;
        clip.ignoreEntParams = &ignoreEntParams;
        for (i = 0; i < entsCount; ++i)
        {
            clip.bLocational = 1;
            entnum = ents[i];
            character_collision = 0;
            ent = (gentity_s *)((char *)sv.gentities + entnum * sv.gentitySize);
            if (ent)
                character_collision = ent->client != 0;
            if (character_collision)
            {
                if (*close_characters_count < 16)
                {
                    dist2 = point_aabb_dist2(endPos, ent->r.absmin, ent->r.absmax);
                    size = gen->size.current * gen->stream->flameVars->flameVar_collisionVolumeScale;
                    if ((float)(size * size) > dist2 && gen->stream->entityNum != entnum)
                    {
                        center[0] = (float)(0.5 * ent->r.absmin[0]) + (float)(0.5 * ent->r.absmax[0]);
                        center[1] = (float)(0.5 * ent->r.absmin[1]) + (float)(0.5 * ent->r.absmax[1]);
                        center[2] = (float)(0.5 * ent->r.absmin[2]) + (float)(0.5 * ent->r.absmax[2]);
                        if (!collide_segment(startPos, center, context))
                            close_characters[(*close_characters_count)++] = entnum;
                    }
                }
            }
            else
            {
                check = &sv.svEntities[entnum];
                if (check->linkmax[0] >= bounds[0][0]
                    && sv.svEntities[entnum].linkmax[1] >= bounds[0][1]
                    && sv.svEntities[entnum].linkmax[2] >= bounds[0][2]
                    && bounds[1][0] >= sv.svEntities[entnum].linkmin[0]
                    && bounds[1][1] >= sv.svEntities[entnum].linkmin[1]
                    && bounds[1][2] >= sv.svEntities[entnum].linkmin[2])
                {
                    if (ent->classname == scr_const.trigger_damage)
                    {
                        memcpy(&old_trace, trace, sizeof(old_trace));
                        SV_TracePointToEntity(&clip, check, trace);
                        if (old_trace.fraction > trace->fraction)
                        {
                            if (*close_triggers_count < 16)
                                close_triggers[(*close_triggers_count)++] = entnum;
                            memcpy(trace, &old_trace, sizeof(trace_t));
                        }
                    }
                    else
                    {
                        SV_TracePointToEntity(&clip, check, trace);
                        if (trace->fraction <= 0.0)
                            return;
                    }
                }
            }
        }
        for (drawType = 0; drawType < 2; ++drawType)
        {
            num = dynEntsCount[drawType];
            for (j = 0; j < num; ++j)
            {
                id = (*dynEnts)[drawType][j];
                colType = (DynEntityCollType)(drawType + 2);
                coll = &cm.dynEntCollList[drawType + 2][id];
                if (bounds[0][2] > coll->linkMaxs[2])
                    break;
                if (coll->linkMaxs[0] >= bounds[0][0]
                    && coll->linkMaxs[1] >= bounds[0][1]
                    && coll->linkMaxs[2] >= bounds[0][2]
                    && bounds[1][0] >= coll->linkMins[0]
                    && bounds[1][1] >= coll->linkMins[1]
                    && bounds[1][2] >= coll->linkMins[2])
                {
                    DynEntSv_PointTrace(id, (DynEntityDrawType)drawType, &clip, trace);
                    if (trace->fraction <= 0.0)
                        return;
                }
            }
        }
        memcpy(&trace2, trace, sizeof(trace2));
        GlassSv_PointTrace(&clip, &trace2);
        if (trace->fraction > trace2.fraction && trace2.hitType == TRACE_HITTYPE_GLASS)
        {
            Vec3Lerp(clip.extents.start.vec.v, clip.extents.end.vec.v, trace2.fraction, hitPos);
            hitDir[0] = clip.extents.end.vec.v[0] - clip.extents.start.vec.v[0];
            hitDir[1] = clip.extents.end.vec.v[1] - clip.extents.start.vec.v[1];
            hitDir[2] = clip.extents.end.vec.v[2] - clip.extents.start.vec.v[2];
            Vec3Normalize(hitDir);
            GlassSv_Damage(trace2.hitId, 10000, 17, hitPos, hitDir);
        }
    }
}

double __cdecl point_aabb_dist2(float *a, const float *mn, float *mx)
{
    float a_proj; // [esp+Ch] [ebp-Ch]
    float a_proj_4; // [esp+10h] [ebp-8h]
    float a_proj_8; // [esp+14h] [ebp-4h]

    a_proj = *a;
    a_proj_4 = a[1];
    a_proj_8 = a[2];
    if ( *a > *mx )
        a_proj = *mx;
    if ( a_proj_4 > mx[1] )
        a_proj_4 = mx[1];
    if ( a_proj_8 > mx[2] )
        a_proj_8 = mx[2];
    if ( *mn > a_proj )
        a_proj = *mn;
    if ( mn[1] > a_proj_4 )
        a_proj_4 = mn[1];
    if ( mn[2] > a_proj_8 )
        a_proj_8 = mn[2];
    return (float)(a[2] - a_proj_8) * (float)(a[2] - a_proj_8)
             + (float)(a[1] - a_proj_4) * (float)(a[1] - a_proj_4)
             + (float)(*a - a_proj) * (float)(*a - a_proj);
}

float radius_3 = 1.0f;
void __cdecl Flame_Client_Trace(
                trace_t *trace,
                flameGeneric_s *gen,
                const float *startPos,
                const float *mins,
                const float *maxs,
                float *endPos,
                col_context_t *context,
                unsigned __int16 (*dynEnts)[2][100],
                const int *dynEntsCount,
                int *ents,
                int entsCount,
                int *close_characters,
                int *close_characters_count)
{
    float absMin[3]; // [esp+50h] [ebp-108h] BYREF
    float absMax[3]; // [esp+5Ch] [ebp-FCh] BYREF
    float actorMins[3]; // [esp+68h] [ebp-F0h]
    float size; // [esp+74h] [ebp-E4h]
    float dist2; // [esp+78h] [ebp-E0h]
    float actorMaxs[3]; // [esp+7Ch] [ebp-DCh]
    int contents; // [esp+88h] [ebp-D0h]
    DObj *dobj; // [esp+8Ch] [ebp-CCh]
    bool character_collision; // [esp+93h] [ebp-C5h]
    centity_s *ent; // [esp+94h] [ebp-C4h]
    int entnum; // [esp+98h] [ebp-C0h]
    int j; // [esp+9Ch] [ebp-BCh]
    DynEntityPose *dynEntPose; // [esp+A0h] [ebp-B8h]
    const DynEntityDef *dynEntDef; // [esp+A4h] [ebp-B4h]
    const DynEntityColl *coll; // [esp+A8h] [ebp-B0h]
    DynEntityCollType colType; // [esp+ACh] [ebp-ACh]
    unsigned __int16 id; // [esp+B0h] [ebp-A8h]
    int i; // [esp+B4h] [ebp-A4h]
    int num; // [esp+B8h] [ebp-A0h]
    int drawType; // [esp+BCh] [ebp-9Ch]
    pointtrace_t clip; // [esp+C0h] [ebp-98h] BYREF
    trace_t trace2; // [esp+104h] [ebp-54h] BYREF
    float bounds[2][3]; // [esp+140h] [ebp-18h] BYREF

    ////TraceExtents::TraceExtents(&clip.extents);
    trace_sphere(trace, startPos, endPos, radius_3, context);
    if ( trace->fraction > 0.0 )
    {
        Vec3Min(startPos, endPos, bounds[0]);
        Vec3Max(startPos, endPos, bounds[1]);
        clip.contentmask = 0x2C0EC33;
        *(_QWORD *)clip.extents.start.vec.v = *(_QWORD *)startPos;
        clip.extents.start.vec.v[2] = startPos[2];
        *(_QWORD *)clip.extents.end.vec.v = *(_QWORD *)endPos;
        clip.extents.end.vec.v[2] = endPos[2];
        CM_CalcTraceExtents(&clip.extents);
        clip.bLocational = 1;
        clip.priorityMap = bulletPriorityMap;
        clip.ignoreEntParams = 0;
        for ( drawType = 0; drawType < 2; ++drawType )
        {
            num = dynEntsCount[drawType];
            for ( i = 0; i < num; ++i )
            {
                id = (*dynEnts)[drawType][i];
                colType = (DynEntityCollType)drawType;
                coll = &cm.dynEntCollList[drawType][id];
                if ( bounds[0][2] > coll->linkMaxs[2] )
                    break;
                if ( coll->linkMaxs[0] >= bounds[0][0]
                    && coll->linkMaxs[1] >= bounds[0][1]
                    && coll->linkMaxs[2] >= bounds[0][2]
                    && bounds[1][0] >= coll->linkMins[0]
                    && bounds[1][1] >= coll->linkMins[1]
                    && bounds[1][2] >= coll->linkMins[2] )
                {
                    dynEntDef = DynEnt_GetEntityDef(id, (DynEntityDrawType)drawType);
                    dynEntPose = DynEnt_GetClientPose(id, (DynEntityDrawType)drawType);
                    if ( drawType )
                    {
                        if ( drawType != 1
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_physics.cpp",
                                        307,
                                        0,
                                        "%s\n\t(drawType) = %i",
                                        "(drawType == DYNENT_DRAW_BRUSH)",
                                        drawType) )
                        {
                            __debugbreak();
                        }
                        DynEnt_PointTraceToBrush(dynEntDef, &dynEntPose->pose, &clip, trace);
                    }
                    else
                    {
                        DynEnt_PointTraceToModel(dynEntDef, &dynEntPose->pose, &clip, trace);
                    }
                    if ( trace->fraction <= 0.0 )
                        return;
                }
            }
        }
        for ( j = 0; j < entsCount; ++j )
        {
            entnum = ents[j];
            if ( (gen->stream->entityNum <= 32 || gen->stream->entityNum != entnum + 32) && gen->stream->entityNum != entnum )
            {
                character_collision = 0;
                ent = CG_GetEntity(0, entnum);
                dobj = Com_GetClientDObj(entnum, 0);
                if ( dobj )
                {
                    contents = CG_GetEntityDObjContents(ent, dobj);
                    character_collision = (contents & 0x2008000) != 0;
                    if ((contents & 0x2008000) != 0)
                    {
                        actorMins[0] = -15.0f;
                        actorMins[1] = -15.0f;
                        actorMins[2] = 0.0f;
                        actorMaxs[0] = 15.0f;
                        actorMaxs[1] = 15.0f;
                        actorMaxs[2] = 48.0f;
                        absMin[0] = ent->pose.origin[0] + -15.0;
                        absMin[1] = ent->pose.origin[1] + -15.0;
                        absMin[2] = ent->pose.origin[2] + 0.0;
                        absMax[0] = ent->pose.origin[0] + 15.0;
                        absMax[1] = ent->pose.origin[1] + 15.0;
                        absMax[2] = ent->pose.origin[2] + 48.0;
                        dist2 = point_aabb_dist2(endPos, absMin, absMax);
                        size = gen->size.current * gen->stream->flameVars->flameVar_collisionVolumeScale;
                        if ( (float)(size * size) > dist2 && gen->stream->entityNum != entnum && *close_characters_count < 16 )
                            close_characters[(*close_characters_count)++] = entnum;
                    }
                    else if ( ent->destructible )
                    {
                        if ( CG_DestructibleIsClientOnly(ent->destructible) )
                        {
                            CG_PointTraceToEntity(&clip, entnum, trace);
                            if ( trace->fraction <= 0.0 )
                                return;
                        }
                    }
                }
            }
        }
        memcpy(&trace2, trace, sizeof(trace2));
        GlassCl_TracePoint(&clip, &trace2);
    }
}

void __cdecl Flame_Phys_Collision_Response(bool is_server, trace_t *trace, flameGeneric_s *gen)
{
    float v3; // [esp+0h] [ebp-70h]
    float fraction; // [esp+4h] [ebp-6Ch]
    float flameVar_collisionSpeedScale; // [esp+Ch] [ebp-64h]
    float len; // [esp+30h] [ebp-40h]
    float endPos; // [esp+34h] [ebp-3Ch]
    float endPos_4; // [esp+38h] [ebp-38h]
    float endPos_8; // [esp+3Ch] [ebp-34h]
    float dir[3]; // [esp+40h] [ebp-30h] BYREF
    float startPos[3]; // [esp+4Ch] [ebp-24h]
    float newVel[3]; // [esp+58h] [ebp-18h] BYREF
    float hitPos[3]; // [esp+64h] [ebp-Ch]

    startPos[0] = gen->phys.origin[0];
    startPos[1] = gen->phys.origin[1];
    startPos[2] = gen->phys.origin[2];
    endPos = gen->phys.newPos[0];
    endPos_4 = gen->phys.newPos[1];
    endPos_8 = gen->phys.newPos[2];
    if ( ((LODWORD(startPos[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(startPos[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(startPos[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_physics.cpp",
                    417,
                    0,
                    "%s",
                    "!IS_NAN((startPos)[0]) && !IS_NAN((startPos)[1]) && !IS_NAN((startPos)[2])") )
    {
        __debugbreak();
    }
    dir[0] = endPos - startPos[0];
    dir[1] = endPos_4 - startPos[1];
    dir[2] = endPos_8 - startPos[2];
    if ( trace->fraction == 1.0 )
    {
        gen->phys.origin[0] = endPos;
        gen->phys.origin[1] = endPos_4;
        gen->phys.origin[2] = endPos_8;
        return;
    }
    if ( trace->fraction == 0.0 )
    {
        *(_QWORD *)trace->normal.vec.v = *(_QWORD *)dir;
        trace->normal.vec.v[2] = dir[2];
        len = Vec3Length(dir);
        if ( len <= 0.000099999997 )
        {
            gen->phys.origin[0] = startPos[0];
            gen->phys.origin[1] = startPos[1];
            gen->phys.origin[2] = startPos[2];
            return;
        }
        trace->normal.vec.v[0] = (float)(-1.0 / len) * trace->normal.vec.v[0];
        trace->normal.vec.v[1] = (float)(-1.0 / len) * trace->normal.vec.v[1];
        trace->normal.vec.v[2] = (float)(-1.0 / len) * trace->normal.vec.v[2];
    }
    if ( ((*((unsigned int *)gen + 23) >> 4) & 0xFFFFFFFu) > flame_freeze_id )
        flame_freeze_id = (*((unsigned int *)gen + 23) >> 4) & 0xFFFFFFF;
    if ( Flame_Random(is_server) < 0.69999999 )
        *((unsigned int *)gen + 23) |= 8u;
    Flame_ClipVelocity(gen->phys.velocity, trace->normal.vec.v, newVel);
    gen->phys.velocity[0] = newVel[0];
    gen->phys.velocity[1] = newVel[1];
    gen->phys.velocity[2] = newVel[2];
    flameVar_collisionSpeedScale = gen->stream->flameVars->flameVar_collisionSpeedScale;
    gen->phys.velocity[0] = flameVar_collisionSpeedScale * gen->phys.velocity[0];
    gen->phys.velocity[1] = flameVar_collisionSpeedScale * gen->phys.velocity[1];
    gen->phys.velocity[2] = flameVar_collisionSpeedScale * gen->phys.velocity[2];
    fraction = trace->fraction;
    hitPos[0] = (float)(fraction * dir[0]) + startPos[0];
    hitPos[1] = (float)(fraction * dir[1]) + startPos[1];
    hitPos[2] = (float)(fraction * dir[2]) + startPos[2];
    v3 = FLAME_OVERCLIP;
    gen->phys.origin[0] = (float)(FLAME_OVERCLIP * trace->normal.vec.v[0]) + hitPos[0];
    gen->phys.origin[1] = (float)(v3 * trace->normal.vec.v[1]) + hitPos[1];
    gen->phys.origin[2] = (float)(v3 * trace->normal.vec.v[2]) + hitPos[2];
}

void __cdecl Flame_Impact_Process(bool is_server, flameGeneric_s *gen, trace_t *trace, int *models, int models_count)
{
    int Time; // eax
    int WeaponIndexForName; // eax
    bool v7; // [esp+20h] [ebp-94h]
    float damageInterval; // [esp+24h] [ebp-90h]
    float damageDuration; // [esp+28h] [ebp-8Ch]
    int damage; // [esp+2Ch] [ebp-88h]
    bool v11; // [esp+30h] [ebp-84h]
    gentity_s *v12; // [esp+34h] [ebp-80h]
    gentity_s *v13; // [esp+38h] [ebp-7Ch]
    float v14[3]; // [esp+50h] [ebp-64h] BYREF
    centity_s *cent; // [esp+5Ch] [ebp-58h]
    bool isWall; // [esp+63h] [ebp-51h]
    DynEntityDrawType v17; // [esp+64h] [ebp-50h] BYREF
    unsigned __int16 EntityHitId; // [esp+68h] [ebp-4Ch]
    bool targetIsAlive; // [esp+6Eh] [ebp-46h]
    bool targetWasAlive; // [esp+6Fh] [ebp-45h]
    float v21[3]; // [esp+70h] [ebp-44h] BYREF
    int dflags; // [esp+7Ch] [ebp-38h]
    hitLocation_t hitLoc; // [esp+80h] [ebp-34h]
    float dir[3]; // [esp+84h] [ebp-30h] BYREF
    float flameDamageDuration; // [esp+90h] [ebp-24h]
    DynEntityDrawType drawType; // [esp+94h] [ebp-20h] BYREF
    int flameDamage; // [esp+98h] [ebp-1Ch]
    float flameDamageInterval; // [esp+9Ch] [ebp-18h]
    unsigned __int16 DynEntHitId; // [esp+A0h] [ebp-14h]
    bool hitFlesh; // [esp+A7h] [ebp-Dh]
    gentity_s *attacker; // [esp+A8h] [ebp-Ch]
    gentity_s *hitEnt; // [esp+ACh] [ebp-8h]
    unsigned __int16 hitEntId; // [esp+B0h] [ebp-4h]

    if ( is_server )
    {
        hitEntId = Trace_GetEntityHitId(trace);
        if ( hitEntId == 1022 )
            v13 = 0;
        else
            v13 = (gentity_s *)((char *)sv.gentities + sv.gentitySize * hitEntId);
        hitEnt = v13;
        if ( hitEntId == 1022 )
            v12 = 0;
        else
            v12 = (gentity_s *)((char *)sv.gentities + gen->stream->entityNum * sv.gentitySize);
        attacker = v12;
        if ( hitEnt )
        {
            v11 = hitEnt->s.eType == 1 || hitEnt->s.eType == 2 && !trace->sflags;
            hitFlesh = v11;
            if ( v11 )
                trace->sflags = 0x700000;
            if ( attacker->s.number != hitEntId )
            {
                if ( (float)gen->stream->damage > 0.0 )
                    damage = gen->stream->damage;
                else
                    damage = (int)default_flameVars_initialHitDamage->current.value;
                flameDamage = damage;
                if ( gen->stream->damageDuration > 0.0 )
                    damageDuration = gen->stream->damageDuration;
                else
                    damageDuration = default_flameVars_timedDamageDuration->current.value;
                flameDamageDuration = damageDuration;
                if ( gen->stream->damageInterval > 0.0 )
                    damageInterval = gen->stream->damageInterval;
                else
                    damageInterval = default_flameVars_timedDamageInterval->current.value;
                flameDamageInterval = damageInterval;
                Flame_trigger_damage(hitEnt, attacker, flameDamage, gen->phys.origin, 17, gen->phys.velocity);
                DynEntHitId = Trace_GetDynEntHitId(trace, &drawType);
                if ( DynEntHitId != 0xFFFF )
                {
                    dir[0] = 0.0f;
                    dir[1] = 1.0f;
                    dir[2] = 0.0f;
                    DynEntSv_Damage(DynEntHitId, drawType, gen->phys.origin, dir, flameDamage, 0);
                }
                if ( hitEnt )
                {
                    v21[0] = 0.0f;
                    v21[1] = 1.0f;
                    v21[2] = 0.0f;
                    dflags = 0;
                    targetWasAlive = hitEnt->health > 0;
                    hitLoc = (hitLocation_t)trace->partGroup;
                    if ( G_GetTime() >= hitEnt->flame_timed_damage[0].end_timestamp
                        || !hitEnt->flame_timed_damage[0].end_timestamp )
                    {
                        hitEnt->flame_timed_damage[0].attacker = attacker;
                        hitEnt->flame_timed_damage[0].damage = 0;
                        hitEnt->flame_timed_damage[0].damageDuration = flameDamageDuration;
                        hitEnt->flame_timed_damage[0].damageInterval = flameDamageInterval;
                        hitEnt->flame_timed_damage[0].start_timestamp = G_GetTime();
                        hitEnt->flame_timed_damage[0].end_timestamp = (int)flameDamageInterval + G_GetTime();
                        if ( attacker->r.inuse )
                        {
                            Time = G_GetTime();
                            G_Damage(
                                hitEnt,
                                attacker,
                                attacker,
                                v21,
                                gen->phys.origin,
                                flameDamage,
                                dflags,
                                17,
                                0xFFFFFFFF,
                                hitLoc,
                                trace->modelIndex,
                                trace->partName,
                                level.time - Time);
                        }
                    }
                    v7 = hitEnt->r.inuse && hitEnt->health > 0;
                    targetIsAlive = v7;
                }
            }
        }
    }
    else
    {
        if ( trace->fraction > 0.0 )
        {
            //isWall = COERCE_FLOAT(trace->normal.vec.u[2] & _mask__AbsFloat_) < 0.30000001;
            isWall = fabsf(trace->normal.vec.u[2]) < 0.30000001;
            if ((trace->sflags & 0x3F00000) != 0x1400000)
            {
                if ( isWall )
                {
                    CG_SetFireToWall(0, gen->phys.origin, models, models_count, TFS_FLAMETHROWER);
                }
                else
                {
                    WeaponIndexForName = G_GetWeaponIndexForName((char*)"m2_flamethrower_mp");
                    CG_SetFireToTerrain(
                        gen->phys.origin,
                        1.0,
                        gen->stream->entityNum,
                        WeaponIndexForName,
                        1.0,
                        models,
                        models_count,
                        TFS_FLAMETHROWER);
                }
            }
        }
        EntityHitId = Trace_GetDynEntHitId(trace, &v17);
        if ( EntityHitId != 0xFFFF )
            DynEntCl_FlameDamage(0, EntityHitId, (DynEntityCollType)v17, gen->phys.origin, gen->phys.velocity, 5);
        EntityHitId = Trace_GetEntityHitId(trace);
        if ( EntityHitId != 1023 && EntityHitId != 1022 )
        {
            cent = CG_GetEntity(0, EntityHitId);
            if ( cent->destructible )
            {
                v14[0] = 0.0f;
                v14[1] = 1.0f;
                v14[2] = 0.0f;
                CG_DestructibleDamage(cent, 0, v14, gen->phys.origin, gen->stream->damage, 17, 0, 0, 1);
            }
        }
    }
}

// Comparator based on stream pointer (can be customized)
struct Flame_SortByStream
{
    bool operator()(flameGeneric_s *a, flameGeneric_s *b) const
    {
        return a->stream < b->stream;
    }
};

void __cdecl Flame_Phys_Update_Items(bool is_server)
{
    flameGeneric_s **v1; // eax
    flameGeneric_s **v2; // eax
    phys_static_array<flameGeneric_s *,1000> *v3; // [esp+0h] [ebp-50h]
    flameGeneric_s **v4; // [esp+10h] [ebp-40h]
    flameGeneric_s **_First; // [esp+14h] [ebp-3Ch]
    int i; // [esp+34h] [ebp-1Ch]
    flameStream_s *prevStream; // [esp+38h] [ebp-18h]
    int startIndex; // [esp+44h] [ebp-Ch]
    int nflames; // [esp+48h] [ebp-8h]
    int count; // [esp+4Ch] [ebp-4h]

    if ( is_server )
        v3 = &sv_flames;
    else
        v3 = &cl_flames;
    nflames = v3->get_count();
    if ( nflames )
    {
        if (is_server)
        {
            //PIXBeginNamedEvent(-1, "sv_flame_physics");
        }
        else
        {
            //PIXBeginNamedEvent(-1, "cl_flame_physics");
        }
        //v4 = &phys_static_array<flameGeneric_s *,1000>::operator[](v3, 0)[nflames];
        v4 = v3->get_list_head() + nflames;

        //_First = phys_static_array<flameGeneric_s *,1000>::operator[](v3, 0);
        //std::_Sort<flameGeneric_s * *,int,Flame_SortByStream>(_First, v4, v4 - _First, 0);
        std::sort(v3->get_list_head(), v4, Flame_SortByStream{});
        //prevStream = (*phys_static_array<flameGeneric_s *,1000>::operator[](v3, 0))->stream;
        prevStream = (flameStream_s*)((*v3)[0]->stream);
        startIndex = 0;
        count = 0;
        for ( i = 0; i < nflames; ++i )
        {
            //if ( (*phys_static_array<flameGeneric_s *,1000>::operator[](v3, i))->stream == prevStream )
            if ( (*v3)[i]->stream == prevStream )
            {
                ++count;
            }
            else
            {
                if ( count <= 0
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_physics.cpp", 684, 0, "%s", "count > 0") )
                {
                    __debugbreak();
                }
                //v1 = phys_static_array<flameGeneric_s *,1000>::operator[](v3, startIndex);
                v1 = &(*v3)[startIndex];
                Flame_Phys_Update_Items_PerStream(is_server, count, v1);
                startIndex += count;
                count = 0;
                if (i >= nflames - 1)
                {
                    prevStream = 0;
                }
                else
                {
                    //prevStream = (*phys_static_array<flameGeneric_s *, 1000>::operator[](v3, i + 1))->stream;
                    prevStream = (*v3)[i + 1]->stream;
                }
            }
        }
        if ( count > 0 )
        {
            //v2 = phys_static_array<flameGeneric_s *,1000>::operator[](v3, startIndex);
            v2 = &(*v3)[startIndex];
            Flame_Phys_Update_Items_PerStream(is_server, count, v2);
        }
        v3->remove_all();
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
    }
}

void __cdecl Flame_Phys_Update_Items_PerStream(bool is_server, int nitems, flameGeneric_s **items)
{
    colgeom_visitor_inlined_t<500> *v3; // [esp+1Ch] [ebp-9ACh]
    float v4; // [esp+20h] [ebp-9A8h]
    float v5; // [esp+24h] [ebp-9A4h]
    unsigned __int16 v6; // [esp+28h] [ebp-9A0h]
    unsigned __int16 v7; // [esp+2Ah] [ebp-99Eh]
    float v8; // [esp+3Ch] [ebp-98Ch]
    int ti; // [esp+50h] [ebp-978h]
    int ci; // [esp+54h] [ebp-974h]
    float genClr[4]; // [esp+58h] [ebp-970h] BYREF
    float *point; // [esp+68h] [ebp-960h]
    int k; // [esp+6Ch] [ebp-95Ch]
    int num; // [esp+70h] [ebp-958h]
    int drawType; // [esp+74h] [ebp-954h]
    float clr[4]; // [esp+78h] [ebp-950h] BYREF
    float origin[3]; // [esp+88h] [ebp-940h] BYREF
    cg_s *cgameGlob; // [esp+94h] [ebp-934h]
    float dist; // [esp+98h] [ebp-930h]
    int j; // [esp+9Ch] [ebp-92Ch]
    flameGeneric_s *gen; // [esp+A0h] [ebp-928h]
    int i; // [esp+A4h] [ebp-924h]
    col_context_t context; // [esp+A8h] [ebp-920h] BYREF
    int close_characters_count; // [esp+D0h] [ebp-8F8h] BYREF
    int models_count; // [esp+D4h] [ebp-8F4h] BYREF
    float nearestClientDist; // [esp+D8h] [ebp-8F0h]
    int numLocalClients; // [esp+DCh] [ebp-8ECh]
    int ents[128]; // [esp+E0h] [ebp-8E8h] BYREF
    int dynEntsCount[2]; // [esp+318h] [ebp-6B0h] BYREF
    float mx[3]; // [esp+320h] [ebp-6A8h] BYREF
    int close_triggers_count; // [esp+32Ch] [ebp-69Ch] BYREF
    int close_characters[16]; // [esp+330h] [ebp-698h] BYREF
    float lod; // [esp+374h] [ebp-654h]
    float expand_vec[3]; // [esp+378h] [ebp-650h] BYREF
    int max_models; // [esp+384h] [ebp-644h]
    int skip; // [esp+388h] [ebp-640h]
    colgeom_visitor_inlined_t<500> *proximity_cache; // [esp+3C4h] [ebp-604h]
    float mn[3]; // [esp+3C8h] [ebp-600h] BYREF
    int entsCount; // [esp+3D4h] [ebp-5F4h]
    int models[257]; // [esp+3D8h] [ebp-5F0h] BYREF
    int skipcounter; // [esp+7DCh] [ebp-1ECh]
    int close_triggers[16]; // [esp+7E0h] [ebp-1E8h] BYREF
    float center[3]; // [esp+824h] [ebp-1A4h] BYREF
    unsigned __int16 dynEnts[2][100]; // [esp+830h] [ebp-198h] BYREF

    mn[0] = FLT_MAX;
    mn[1] = FLT_MAX;
    mn[2] = FLT_MAX;
    mx[0] = -FLT_MAX;
    mx[1] = -FLT_MAX;
    mx[2] = -FLT_MAX;
    if ( nitems <= 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_physics.cpp", 711, 0, "%s", "nitems > 0") )
    {
        __debugbreak();
    }
    if ( (*items)->stream )
    {
        for ( i = 0; i < nitems; ++i )
        {
            gen = items[i];
            Vec3Min(gen->phys.origin, mn, mn);
            Vec3Min(gen->phys.newPos, mn, mn);
            Vec3Max(gen->phys.origin, mx, mx);
            Vec3Max(gen->phys.newPos, mx, mx);
        }

        mn[0] = mn[0] - size[0];
        mn[1] = mn[1] - size[1];
        mn[2] = mn[2] - size[2];

        mx[0] = mx[0] + size[0];
        mx[1] = mx[1] + size[1];
        mx[2] = mx[2] + size[2];

        Vec3Lerp(mn, mx, 0.5, center);
        nearestClientDist = FLT_MAX;
        numLocalClients = CL_LocalClient_GetActiveCount();
        for ( j = 0; j < numLocalClients; ++j )
        {
            cgameGlob = CG_GetLocalClientGlobals(j);
            dist = Vec3Distance(cgameGlob->refdef.vieworg, center);
            if ( (float)(dist - nearestClientDist) < 0.0 )
                v5 = dist;
            else
                v5 = nearestClientDist;
            nearestClientDist = v5;
        }
        if ( (float)((float)(nearestClientDist / 1000.0) - 1.0) < 0.0 )
            v8 = nearestClientDist / 1000.0;
        else
            v8 = 1.0f;
        if ( (float)(0.0 - (float)(nearestClientDist / 1000.0)) < 0.0 )
            v4 = v8;
        else
            v4 = 0.0f;
        lod = v4;
        if ( flame_debug_render && flame_debug_render->current.integer > 1 )
        {
            memset(origin, 0, sizeof(origin));
            clr[0] = 1.0f;
            clr[1] = 1.0f;
            clr[2] = 1.0f;
            clr[3] = 1.0f;
            CG_DebugBox(origin, mn, mx, 0.0, clr, 1, 3);
        }
        if ( is_server )
            v3 = &sv_flame_chunks_proximity_cache;
        else
            v3 = &cl_flame_chunks_proximity_cache;
        proximity_cache = v3;
        //colgeom_visitor_inlined_t<500>::reset((colgeom_visitor_inlined_t<200> *)v3);
        v3->reset();
        expand_vec[0] = 1.0f;
        expand_vec[1] = 1.0f;
        expand_vec[2] = 1.0f;
        proximity_cache->update(mn, mx, 0x2C0EC33, expand_vec);
        ////col_context_t::col_context_t(&context);
        context.prims = proximity_cache->prims;
        context.nprims = proximity_cache->nprims;
        for ( drawType = 0; drawType < 2; ++drawType )
        {
            if ( is_server )
                num = DynEntSv_AreaEntities((DynEntityDrawType)drawType, mn, mx, -1, 0x64u, dynEnts[drawType]);
            else
                num = DynEntCl_AreaEntities((DynEntityDrawType)drawType, mn, mx, -1, 0x64u, dynEnts[drawType]);
            dynEntsCount[drawType] = num;
        }
        if ( is_server )
            entsCount = CM_AreaEntities(mn, mx, ents, 128, 0x2C0EC33);
        else
            entsCount = CG_AreaEntities(mn, mx, ents, 128, 0x2C0EC33);
        close_characters_count = 0;
        close_triggers_count = 0;
        max_models = 256;
        models_count = 0;
        R_GetStaticModels(mn, mx, models, &models_count, 256);
        skip = 1;
        skipcounter = 1;

        trace_t trace; // [esp+38Ch] [ebp-63Ch] BYREF
        trace_t trace2; // [esp+2E0h] [ebp-6E8h] BYREF
        for ( k = 0; k < nitems; ++k )
        {
            point = items[k]->phys.origin;
            if ( flame_debug_render && flame_debug_render->current.integer > 1 )
            {
                if ( is_server )
                {
                    genClr[0] = 0.0f;
                    genClr[1] = 0.0f;
                    genClr[2] = 1.0f;
                }
                else
                {
                    genClr[0] = 1.0f;
                    genClr[1] = 0.0f;
                    genClr[2] = 0.0f;
                }
                genClr[3] = 1.0f;
                G_DebugStar(point, genClr, 3);
            }
            --skipcounter;
            if ( !k || !skipcounter )
            {
                Flame_Phys_Collision(
                    is_server,
                    &trace,
                    (flameGeneric_s *)point,
                    &context,
                    (unsigned __int16 (*)[2][100])dynEnts,
                    dynEntsCount,
                    ents,
                    entsCount,
                    close_characters,
                    &close_characters_count,
                    close_triggers,
                    &close_triggers_count);
                memcpy(&trace2, &trace, sizeof(trace2));
                skipcounter = skip;
            }
            Flame_Phys_Collision_Response(is_server, &trace, (flameGeneric_s *)point);
            if ( trace.fraction < 1.0 && trace.fraction > 0.0 )
                Flame_Impact_Process(is_server, (flameGeneric_s *)point, &trace, models, models_count);
            for ( ci = 0; ci < close_characters_count; ++ci )
            {
                v7 = close_characters[ci];
                if ( !&trace2
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\physics\\../qcommon/cm_public.h",
                                175,
                                0,
                                "%s",
                                "trace") )
                {
                    __debugbreak();
                }
                trace2.hitType = TRACE_HITTYPE_ENTITY;
                trace2.hitId = v7;
                trace2.fraction = 0.0f;
                trace2.cflags = 0x8000;
                trace2.modelIndex = 0;
                trace2.partName = 0;
                trace2.partGroup = 0;
                Flame_Impact_Process(is_server, (flameGeneric_s *)point, &trace2, models, models_count);
            }
            for ( ti = 0; ti < close_triggers_count; ++ti )
            {
                v6 = close_triggers[ti];
                if ( !&trace2
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\physics\\../qcommon/cm_public.h",
                                175,
                                0,
                                "%s",
                                "trace") )
                {
                    __debugbreak();
                }
                trace2.hitType = TRACE_HITTYPE_ENTITY;
                trace2.hitId = v6;
                trace2.fraction = 0.0f;
                trace2.cflags = 0x8000;
                trace2.modelIndex = 0;
                trace2.partName = 0;
                trace2.partGroup = 0;
                Flame_Impact_Process(is_server, (flameGeneric_s *)point, &trace2, models, models_count);
            }
        }
    }
}

void __cdecl Flame_Phys_Update_Item_Stream(flameGeneric_s *gen, int time)
{
    float v2; // [esp+0h] [ebp-1Ch]
    float frametime; // [esp+10h] [ebp-Ch]
    float sizeAdd; // [esp+14h] [ebp-8h]
    float lifeFrac; // [esp+18h] [ebp-4h]

    if ( time > gen->age.lastUpdateTime )
    {
        frametime = (float)(time - gen->age.lastUpdateTime) * 0.001;
        gen->age.lastUpdateTime = time;
        v2 = (float)(gen->age.lastUpdateTime - gen->age.startTime);
        lifeFrac = Flame_CalcTimeScale(gen->age.startTime, gen->age.endTime) * v2;
        if ( lifeFrac < 1.0 )
        {
            if ( (LODWORD(gen->size.rate) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_physics.cpp",
                            896,
                            0,
                            "%s",
                            "!IS_NAN(gen->size.rate)") )
            {
                __debugbreak();
            }
            sizeAdd = gen->size.rate * frametime;
            gen->phys.rotation = (float)(gen->phys.rotVel * frametime) + gen->phys.rotation;
            gen->size.current = gen->size.current + sizeAdd;
            if ( (LODWORD(gen->size.current) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_physics.cpp",
                            904,
                            0,
                            "%s",
                            "!IS_NAN(gen->size.current)") )
            {
                __debugbreak();
            }
            gen->phys.newPos[0] = (float)(frametime * gen->phys.velocity[0]) + gen->phys.origin[0];
            gen->phys.newPos[1] = (float)(frametime * gen->phys.velocity[1]) + gen->phys.origin[1];
            gen->phys.newPos[2] = (float)(frametime * gen->phys.velocity[2]) + gen->phys.origin[2];
        }
    }
}

void __cdecl Flame_Phys_Update_Item_Chunk(
                flameGeneric_s *gen,
                int time,
                phys_static_array<flameGeneric_s *,1000> *flames)
{
    float v3; // [esp+0h] [ebp-5Ch]
    float v4; // [esp+4h] [ebp-58h]
    float v5; // [esp+8h] [ebp-54h]
    float v6; // [esp+Ch] [ebp-50h]
    flameGeneric_s **v7; // [esp+14h] [ebp-48h]
    flameTable *vars; // [esp+34h] [ebp-28h]
    float ageScale; // [esp+38h] [ebp-24h]
    float frametime; // [esp+3Ch] [ebp-20h]
    float sizeAdd; // [esp+40h] [ebp-1Ch]
    float sizeAdda; // [esp+40h] [ebp-1Ch]
    float gravityStart; // [esp+44h] [ebp-18h]
    float lifeFrac; // [esp+48h] [ebp-14h]
    float gravityEnd; // [esp+50h] [ebp-Ch]
    float friction; // [esp+54h] [ebp-8h]
    float speedScale; // [esp+58h] [ebp-4h]

    if ( time > gen->age.lastUpdateTime )
    {
        vars = gen->stream->flameVars;
        gravityStart = vars->flameVar_streamChunkGravityStart;
        gravityEnd = vars->flameVar_streamChunkGravityEnd;
        friction = vars->flameVar_streamChunkDecel;
        speedScale = vars->flameVar_streamChunkSizeSpeedScale;
        ageScale = vars->flameVar_streamChunkSizeAgeScale;
        frametime = (float)(time - gen->age.lastUpdateTime) * 0.001;
        gen->age.lastUpdateTime = time;
        v6 = (float)(gen->age.lastUpdateTime - gen->age.startTime);
        lifeFrac = Flame_CalcTimeScale(gen->age.startTime, gen->age.endTime) * v6;
        if ( lifeFrac < 1.0 )
        {
            if ( (LODWORD(gen->size.rate) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_physics.cpp",
                            939,
                            0,
                            "%s",
                            "!IS_NAN(gen->size.rate)") )
            {
                __debugbreak();
            }
            sizeAdd = (float)((float)(1.0 - ageScale) + (float)((float)(2.0 * ageScale) * lifeFrac))
                            * (float)(gen->size.rate * frametime);
            v5 = 1.0 - speedScale;
            v4 = 2.0 * speedScale;
            sizeAdda = ((1.0 - Vec3Length(gen->phys.velocity) * gen->phys.invStartSpeed) * v4 + v5) * sizeAdd;
            gen->phys.rotation = (float)(gen->phys.rotVel * frametime) + gen->phys.rotation;
            gen->size.current = gen->size.current + sizeAdda;
            if ( (LODWORD(gen->size.current) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_physics.cpp",
                            951,
                            0,
                            "%s",
                            "!IS_NAN(gen->size.current)") )
            {
                __debugbreak();
            }
            if ( (float)((float)(1.0 - (float)(frametime * friction)) - 0.0) < 0.0 )
                v3 = 0.0f;
            else
                v3 = 1.0 - (float)(frametime * friction);
            gen->phys.velocity[0] = v3 * gen->phys.velocity[0];
            gen->phys.velocity[1] = v3 * gen->phys.velocity[1];
            gen->phys.velocity[2] = v3 * gen->phys.velocity[2];
            gen->phys.gravity = (float)((float)(gravityEnd - gravityStart) * lifeFrac) + gravityStart;
            gen->phys.velocity[2] = gen->phys.velocity[2] - (float)(frametime * gen->phys.gravity);
            gen->phys.newPos[0] = (float)(frametime * gen->phys.velocity[0]) + gen->phys.origin[0];
            gen->phys.newPos[1] = (float)(frametime * gen->phys.velocity[1]) + gen->phys.origin[1];
            gen->phys.newPos[2] = (float)(frametime * gen->phys.velocity[2]) + gen->phys.origin[2];
            if ( flames->get_count() != 1000 )
            {
                v7 = flames->add(0, "phys array add overflow.");
                *v7 = gen;
            }
        }
    }
}

void __cdecl Flame_Phys_Update_Item_Fire(flameGeneric_s *gen, int time)
{
    float v2; // [esp+0h] [ebp-40h]
    float v3; // [esp+4h] [ebp-3Ch]
    flameTable *vars; // [esp+20h] [ebp-20h]
    float frametime; // [esp+24h] [ebp-1Ch]
    float sizeAdd; // [esp+28h] [ebp-18h]
    float gravityStart; // [esp+2Ch] [ebp-14h]
    float lifeFrac; // [esp+30h] [ebp-10h]
    float gravityEnd; // [esp+38h] [ebp-8h]
    float friction; // [esp+3Ch] [ebp-4h]

    if ( time > gen->age.lastUpdateTime )
    {
        vars = gen->stream->flameVars;
        gravityStart = vars->flameVar_fireGravity;
        gravityEnd = vars->flameVar_fireGravityEnd;
        friction = vars->flameVar_fireFriction;
        frametime = (float)(time - gen->age.lastUpdateTime) * 0.001;
        gen->age.lastUpdateTime = time;
        v3 = (float)(gen->age.lastUpdateTime - gen->age.startTime);
        lifeFrac = Flame_CalcTimeScale(gen->age.startTime, gen->age.endTime) * v3;
        if ( lifeFrac < 1.0 )
        {
            if ( (LODWORD(gen->size.rate) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_physics.cpp",
                            999,
                            0,
                            "%s",
                            "!IS_NAN(gen->size.rate)") )
            {
                __debugbreak();
            }
            sizeAdd = gen->size.rate * frametime;
            gen->phys.rotation = (float)(gen->phys.rotVel * frametime) + gen->phys.rotation;
            gen->size.current = gen->size.current + sizeAdd;
            if ( (LODWORD(gen->size.current) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_physics.cpp",
                            1007,
                            0,
                            "%s",
                            "!IS_NAN(gen->size.current)") )
            {
                __debugbreak();
            }
            if ( (float)((float)(1.0 - (float)(frametime * friction)) - 0.0) < 0.0 )
                v2 = 0.0f;
            else
                v2 = 1.0 - (float)(frametime * friction);
            gen->phys.velocity[0] = v2 * gen->phys.velocity[0];
            gen->phys.velocity[1] = v2 * gen->phys.velocity[1];
            gen->phys.velocity[2] = v2 * gen->phys.velocity[2];
            gen->phys.gravity = (float)((float)(gravityEnd - gravityStart) * lifeFrac) + gravityStart;
            gen->phys.velocity[2] = gen->phys.velocity[2] - (float)(frametime * gen->phys.gravity);
            gen->phys.newPos[0] = (float)(frametime * gen->phys.velocity[0]) + gen->phys.origin[0];
            gen->phys.newPos[1] = (float)(frametime * gen->phys.velocity[1]) + gen->phys.origin[1];
            gen->phys.newPos[2] = (float)(frametime * gen->phys.velocity[2]) + gen->phys.origin[2];
            gen->phys.origin[0] = gen->phys.newPos[0];
            gen->phys.origin[1] = gen->phys.newPos[1];
            gen->phys.origin[2] = gen->phys.newPos[2];
        }
    }
}

void __cdecl Flame_Phys_Update_Item_Smoke(flameGeneric_s *gen, int time)
{
    float v2; // [esp+0h] [ebp-40h]
    float v3; // [esp+4h] [ebp-3Ch]
    flameTable *vars; // [esp+20h] [ebp-20h]
    float frametime; // [esp+24h] [ebp-1Ch]
    float sizeAdd; // [esp+28h] [ebp-18h]
    float gravityStart; // [esp+2Ch] [ebp-14h]
    float lifeFrac; // [esp+30h] [ebp-10h]
    float gravityEnd; // [esp+38h] [ebp-8h]
    float friction; // [esp+3Ch] [ebp-4h]

    if ( time > gen->age.lastUpdateTime )
    {
        vars = gen->stream->flameVars;
        gravityStart = vars->flameVar_smokeGravity;
        gravityEnd = vars->flameVar_smokeGravityEnd;
        friction = vars->flameVar_smokeFriction;
        frametime = (float)(time - gen->age.lastUpdateTime) * 0.001;
        gen->age.lastUpdateTime = time;
        v3 = (float)(gen->age.lastUpdateTime - gen->age.startTime);
        lifeFrac = Flame_CalcTimeScale(gen->age.startTime, gen->age.endTime) * v3;
        if ( lifeFrac < 1.0 )
        {
            if ( (LODWORD(gen->size.rate) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_physics.cpp",
                            1051,
                            0,
                            "%s",
                            "!IS_NAN(gen->size.rate)") )
            {
                __debugbreak();
            }
            sizeAdd = gen->size.rate * frametime;
            gen->phys.rotation = (float)(gen->phys.rotVel * frametime) + gen->phys.rotation;
            gen->size.current = gen->size.current + sizeAdd;
            if ( (LODWORD(gen->size.current) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_physics.cpp",
                            1059,
                            0,
                            "%s",
                            "!IS_NAN(gen->size.current)") )
            {
                __debugbreak();
            }
            if ( (float)((float)(1.0 - (float)(frametime * friction)) - 0.0) < 0.0 )
                v2 = 0.0f;
            else
                v2 = 1.0 - (float)(frametime * friction);
            gen->phys.velocity[0] = v2 * gen->phys.velocity[0];
            gen->phys.velocity[1] = v2 * gen->phys.velocity[1];
            gen->phys.velocity[2] = v2 * gen->phys.velocity[2];
            gen->phys.gravity = (float)((float)(gravityEnd - gravityStart) * lifeFrac) + gravityStart;
            gen->phys.velocity[2] = gen->phys.velocity[2] - (float)(frametime * gen->phys.gravity);
            gen->phys.newPos[0] = (float)(frametime * gen->phys.velocity[0]) + gen->phys.origin[0];
            gen->phys.newPos[1] = (float)(frametime * gen->phys.velocity[1]) + gen->phys.origin[1];
            gen->phys.newPos[2] = (float)(frametime * gen->phys.velocity[2]) + gen->phys.origin[2];
            gen->phys.origin[0] = gen->phys.newPos[0];
            gen->phys.origin[1] = gen->phys.newPos[1];
            gen->phys.origin[2] = gen->phys.newPos[2];
        }
    }
}

void __cdecl Flame_Phys_Update_Item_Drip(
                flameGeneric_s *gen,
                int time,
                phys_static_array<flameGeneric_s *,1000> *flames)
{
    float v3; // [esp+0h] [ebp-48h]
    float v4; // [esp+4h] [ebp-44h]
    flameGeneric_s **v5; // [esp+Ch] [ebp-3Ch]
    flameTable *vars; // [esp+28h] [ebp-20h]
    float frametime; // [esp+2Ch] [ebp-1Ch]
    float sizeAdd; // [esp+30h] [ebp-18h]
    float gravityStart; // [esp+34h] [ebp-14h]
    float lifeFrac; // [esp+38h] [ebp-10h]
    float gravityEnd; // [esp+40h] [ebp-8h]
    float friction; // [esp+44h] [ebp-4h]

    if ( time > gen->age.lastUpdateTime )
    {
        vars = gen->stream->flameVars;
        gravityStart = vars->flameVar_dripsGravity;
        gravityEnd = vars->flameVar_dripsGravityEnd;
        friction = vars->flameVar_dripsFriction;
        frametime = (float)(time - gen->age.lastUpdateTime) * 0.001;
        gen->age.lastUpdateTime = time;
        v4 = (float)(gen->age.lastUpdateTime - gen->age.startTime);
        lifeFrac = Flame_CalcTimeScale(gen->age.startTime, gen->age.endTime) * v4;
        if ( lifeFrac < 1.0 )
        {
            if ( (LODWORD(gen->size.rate) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_physics.cpp",
                            1103,
                            0,
                            "%s",
                            "!IS_NAN(gen->size.rate)") )
            {
                __debugbreak();
            }
            sizeAdd = gen->size.rate * frametime;
            gen->phys.rotation = (float)(gen->phys.rotVel * frametime) + gen->phys.rotation;
            gen->size.current = gen->size.current + sizeAdd;
            if ( (LODWORD(gen->size.current) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_physics.cpp",
                            1111,
                            0,
                            "%s",
                            "!IS_NAN(gen->size.current)") )
            {
                __debugbreak();
            }
            if ( (float)((float)(1.0 - (float)(frametime * friction)) - 0.0) < 0.0 )
                v3 = 0.0f;
            else
                v3 = 1.0 - (float)(frametime * friction);
            gen->phys.velocity[0] = v3 * gen->phys.velocity[0];
            gen->phys.velocity[1] = v3 * gen->phys.velocity[1];
            gen->phys.velocity[2] = v3 * gen->phys.velocity[2];
            gen->phys.gravity = (float)((float)(gravityEnd - gravityStart) * lifeFrac) + gravityStart;
            gen->phys.velocity[2] = gen->phys.velocity[2] - (float)(frametime * gen->phys.gravity);
            gen->phys.newPos[0] = (float)(frametime * gen->phys.velocity[0]) + gen->phys.origin[0];
            gen->phys.newPos[1] = (float)(frametime * gen->phys.velocity[1]) + gen->phys.origin[1];
            gen->phys.newPos[2] = (float)(frametime * gen->phys.velocity[2]) + gen->phys.origin[2];
            if ( flames->get_count() != 1000 )
            {
                v5 = flames->add(0, "phys array add overflow.");
                *v5 = gen;
            }
        }
    }
}
