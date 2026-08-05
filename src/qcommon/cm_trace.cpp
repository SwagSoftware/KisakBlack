#include "cm_trace.h"
#include "cm_load.h"
#include "threads.h"
#include "cm_tracebox.h"
#include "common.h"
#include "cm_mesh.h"
#include "cm_test.h"
#include <universal/com_math_anglevectors.h>
#include <gfx_d3d/r_water_sim.h>
#include <server/sv_world.h>
#include <bgame/bg_misc.h>
#include <server_mp/sv_main_mp.h>
#include <game/g_scr_helicopter.h>

col_context_t::col_context_t()
{
    this->mask = 0;
    this->prims = 0;
    this->nprims = 0;
    this->ignoreEntParams = 0;
    this->passEntityNum1 = 1023;
    this->passEntityNum0 = 1023;
    this->passEntityNum1 = 1023;
    this->locational = 0;
    this->staticmodels = 0;
    this->priorityMap = 0;
    this->collide_entity_func = 0;
}

col_context_t::col_context_t(int _mask)
{
    this->mask = _mask;
    this->prims = 0;
    this->nprims = 0;
    this->ignoreEntParams = 0;
    this->passEntityNum1 = 1023;
    this->passEntityNum0 = 1023;
    this->locational = 0;
    this->staticmodels = 0;
    this->priorityMap = 0;
    this->collide_entity_func = 0;
}

void __thiscall col_context_t::init_locational(int ent0)
{
    this->passEntityNum0 = ent0;
    this->locational = 1;
    this->staticmodels = 1;
}

void __thiscall col_context_t::init_locational(int ent0, int ent1)
{
    this->passEntityNum0 = ent0;
    this->passEntityNum1 = ent1;
    this->locational = 1;
    this->staticmodels = 1;
}

unsigned __int16 __cdecl Trace_GetEntityHitId(const trace_t *trace)
{
    if ( !trace && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 63, 0, "%s", "trace") )
        __debugbreak();
    switch ( trace->hitType )
    {
        case TRACE_HITTYPE_DYNENT_MODEL:
        case TRACE_HITTYPE_DYNENT_BRUSH:
            return 1022;
        case TRACE_HITTYPE_GLASS:
            return 1022;
        case TRACE_HITTYPE_ENTITY:
            return trace->hitId;
        default:
            return 1023;
    }
}

unsigned __int16 __cdecl Trace_GetDynEntHitId(const trace_t *trace, DynEntityDrawType *drawType)
{
    unsigned __int16 dynEntId; // [esp+0h] [ebp-4h]

    if ( !trace && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 81, 0, "%s", "trace") )
        __debugbreak();
    if ( !drawType
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 82, 0, "%s", "drawType") )
    {
        __debugbreak();
    }
    dynEntId = -1;
    if ( trace->hitType == TRACE_HITTYPE_DYNENT_MODEL )
    {
        *drawType = DYNENT_DRAW_MODEL;
        dynEntId = trace->hitId;
        if ( dynEntId >= (unsigned int)cm.dynEntCount[0]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        90,
                        0,
                        "dynEntId doesn't index cm.dynEntCount[DynEntGetClientCollType( DYNENT_DRAW_MODEL )]\n\t%i not in [0, %i)",
                        dynEntId,
                        cm.dynEntCount[0]) )
        {
            __debugbreak();
        }
    }
    if ( trace->hitType == TRACE_HITTYPE_DYNENT_BRUSH )
    {
        *drawType = DYNENT_DRAW_BRUSH;
        dynEntId = trace->hitId;
        if ( dynEntId >= (unsigned int)cm.dynEntCount[1]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        97,
                        0,
                        "dynEntId doesn't index cm.dynEntCount[DynEntGetClientCollType( DYNENT_DRAW_BRUSH )]\n\t%i not in [0, %i)",
                        dynEntId,
                        cm.dynEntCount[1]) )
        {
            __debugbreak();
        }
    }
    return dynEntId;
}

int __cdecl Trace_GetGlassHitId(const trace_t *trace)
{
    int id; // [esp+0h] [ebp-4h]

    if ( !trace && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 106, 0, "%s", "trace") )
        __debugbreak();
    id = -1;
    if ( trace->hitType == TRACE_HITTYPE_GLASS )
        return trace->hitId;
    return id;
}

void __cdecl CM_GetTraceThreadInfo(TraceThreadInfo *threadInfo)
{
    TraceThreadInfo *value; // [esp+0h] [ebp-4h]

    if ( !threadInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 124, 0, "%s", "threadInfo") )
    {
        __debugbreak();
    }
    value = (TraceThreadInfo *)Sys_GetValue(3);
    if ( !value && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 127, 0, "%s", "value") )
        __debugbreak();
    ++value->checkcount.global;
    *threadInfo = *value;
    if ( !threadInfo->checkcount.partitions
        && cm.partitionCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                    132,
                    0,
                    "%s",
                    "threadInfo->checkcount.partitions || cm.partitionCount == 0") )
    {
        __debugbreak();
    }
}

clipMap_t *__cdecl CM_GetClipMap()
{
    return &cm;
}

unsigned int __cdecl CM_TempBoxModel(const float *mins, const float *maxs, int contents)
{
    float *v4; // [esp+0h] [ebp-18h]
    cbrush_t *v5; // [esp+4h] [ebp-14h]
    float *v6; // [esp+8h] [ebp-10h]
    cmodel_t *v7; // [esp+Ch] [ebp-Ch]
    cbrush_t *box_brush; // [esp+10h] [ebp-8h] BYREF
    cmodel_t *box_model; // [esp+14h] [ebp-4h] BYREF

    CM_GetBox(&box_brush, &box_model, 0);
    v7 = box_model;
    box_model->mins[0] = *mins;
    v7->mins[1] = mins[1];
    v7->mins[2] = mins[2];
    v6 = box_model->maxs;
    box_model->maxs[0] = *maxs;
    v6[1] = maxs[1];
    v6[2] = maxs[2];
    v5 = box_brush;
    box_brush->mins[0] = *mins;
    v5->mins[1] = mins[1];
    v5->mins[2] = mins[2];
    v4 = box_brush->maxs;
    box_brush->maxs[0] = *maxs;
    v4[1] = maxs[1];
    v4[2] = maxs[2];
    box_brush->contents = contents;
    return 0x3FFF;
}

void __cdecl CM_GetBox(cbrush_t **box_brush, cmodel_t **box_model, PhysGeomList ***geoms)
{
    TraceThreadInfo *value; // [esp+0h] [ebp-4h]

    value = (TraceThreadInfo *)Sys_GetValue(3);
    if ( !value && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 152, 0, "%s", "value") )
        __debugbreak();
    if ( value->box_model->leaf.brushContents != -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                    154,
                    0,
                    "%s\n\t(value->box_model->leaf.brushContents) = %i",
                    "(value->box_model->leaf.brushContents == ~0)",
                    value->box_model->leaf.brushContents) )
    {
        __debugbreak();
    }
    if ( value->box_model->leaf.terrainContents
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                    155,
                    0,
                    "%s\n\t(value->box_model->leaf.terrainContents) = %i",
                    "(value->box_model->leaf.terrainContents == 0)",
                    value->box_model->leaf.terrainContents) )
    {
        __debugbreak();
    }
    *box_brush = value->box_brush;
    *box_model = value->box_model;
    if ( geoms )
        *geoms = value->geoms;
}

unsigned int __cdecl CM_TempBrushModel(PhysGeomList *geoms)
{
    cbrush_t *box_brush; // [esp+0h] [ebp-Ch] BYREF
    cmodel_t *box_model; // [esp+4h] [ebp-8h] BYREF
    PhysGeomList **geoms_model; // [esp+8h] [ebp-4h] BYREF

    CM_GetBox(&box_brush, &box_model, &geoms_model);
    *geoms_model = geoms;
    return 0x4000;
}

bool __cdecl CM_ClipHandleIsValid(unsigned int handle)
{
    if ( handle < cm.numSubModels )
        return 1;
    if ( handle == 0x3FFF )
        return 1;
    return handle == 0x4000;
}

cmodel_t *__cdecl CM_ClipHandleToModel(unsigned int handle)
{
    const char *v2; // eax
    cbrush_t *box_brush; // [esp+0h] [ebp-8h] BYREF
    cmodel_t *box_model; // [esp+4h] [ebp-4h] BYREF

    if ( handle < cm.numSubModels )
        return &cm.cmodels[handle];
    if ( handle != 0x3FFF && handle != 0x4000 )
    {
        v2 = va("handle: %d, cm.numSubModels: %d", handle, cm.numSubModels);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        223,
                        0,
                        "%s\n\t%s",
                        "( handle == CAPSULE_MODEL_HANDLE ) || ( handle == TEMP_BRUSH_MODEL_HANDLE )",
                        v2) )
            __debugbreak();
    }
    CM_GetBox(&box_brush, &box_model, 0);
    return box_model;
}

int __cdecl CM_ContentsOfModel(unsigned int handle)
{
    cmodel_t *v1; // edx

    v1 = CM_ClipHandleToModel(handle);
    return v1->leaf.terrainContents | v1->leaf.brushContents;
}

void __cdecl CM_Trace(
                trace_t *results,
                const float *start,
                const float *end,
                const float *mins,
                const float *maxs,
                unsigned int model,
                int brushmask,
                col_context_t *context)
{
    cmodel_t *cmod; // [esp+5Ch] [ebp-124h]
    traceWork_t tw; // [esp+60h] [ebp-120h] BYREF
    float offset[3]; // [esp+144h] [ebp-3Ch]
    float oldFrac; // [esp+150h] [ebp-30h]
    float start_[4]; // [esp+154h] [ebp-2Ch] BYREF
    int oldSurfaceFlags; // [esp+164h] [ebp-1Ch]
    int i; // [esp+168h] [ebp-18h]
    float end_[4]; // [esp+170h] [ebp-10h] BYREF

    PROF_SCOPED("CM_Trace");

    ////traceWork_t::traceWork_t(&tw);

    iassert(cm.numNodes);
    iassert(mins);
    iassert(maxs);
    nanassertvec3(end);

    cmod = CM_ClipHandleToModel(model);
    tw.contents = brushmask;
    for ( i = 0; i < 3; ++i )
    {
        iassert(maxs[i] >= mins[i]);
        offset[i] = (float)(mins[i] + maxs[i]) * 0.5;
        tw.size.vec.v[i] = maxs[i] - offset[i];
        tw.extents.start.vec.v[i] = start[i] + offset[i];
        tw.extents.end.vec.v[i] = end[i] + offset[i];
        tw.midpoint.vec.v[i] = (float)(tw.extents.start.vec.v[i] + tw.extents.end.vec.v[i]) * 0.5;
        tw.delta.vec.v[i] = tw.extents.end.vec.v[i] - tw.extents.start.vec.v[i];
        tw.halfDelta.vec.v[i] = 0.5 * tw.delta.vec.v[i];
        tw.halfDeltaAbs.vec.v[i] = I_fabs(tw.halfDelta.vec.v[i]);
    }
    CM_CalcTraceExtents(&tw.extents);
    tw.deltaLenSq = Vec3LengthSq(tw.delta.vec.v);
    tw.deltaLen = sqrtf(tw.deltaLenSq);

    if ( tw.size.vec.v[0] <= tw.size.vec.v[2] )
        tw.radius = tw.size.vec.v[0];
    else
        tw.radius = tw.size.vec.v[2];

    tw.boundingRadius = Vec3Length(tw.size.vec.v);
    tw.offsetZ = tw.size.vec.v[2] - tw.radius;

    for ( i = 0; i < 2; ++i )
    {
        if ( tw.extents.end.vec.v[i] <= tw.extents.start.vec.v[i] )
        {
            tw.bounds[0].vec.v[i] = tw.extents.end.vec.v[i] - tw.radius;
            tw.bounds[1].vec.v[i] = tw.extents.start.vec.v[i] + tw.radius;
        }
        else
        {
            tw.bounds[0].vec.v[i] = tw.extents.start.vec.v[i] - tw.radius;
            tw.bounds[1].vec.v[i] = tw.extents.end.vec.v[i] + tw.radius;
        }
    }
    iassert(tw.offsetZ >= 0);
    if ( tw.extents.end.vec.v[2] <= tw.extents.start.vec.v[2] )
    {
        tw.bounds[0].vec.v[2] = (float)(tw.extents.end.vec.v[2] - tw.offsetZ) - tw.radius;
        tw.bounds[1].vec.v[2] = (float)(tw.extents.start.vec.v[2] + tw.offsetZ) + tw.radius;
    }
    else
    {
        tw.bounds[0].vec.v[2] = (float)(tw.extents.start.vec.v[2] - tw.offsetZ) - tw.radius;
        tw.bounds[1].vec.v[2] = (float)(tw.extents.end.vec.v[2] + tw.offsetZ) + tw.radius;
    }
    CM_SetAxialCullOnly(&tw);
    CM_GetTraceThreadInfo(&tw.threadInfo);
    oldSurfaceFlags = results->sflags;
    oldFrac = results->fraction;
    results->sflags = -1;
    if ( *start == *end && start[1] == end[1] && start[2] == end[2] )
    {
        tw.isPoint = 0;
        tw.doublesided = doublesided_raycasts->current.enabled;
        if ( model )
        {
            if ( model == 0x3FFF )
            {
                if ( (tw.contents & tw.threadInfo.box_brush->contents) != 0 )
                    CM_TestCapsuleInCapsule(&tw, results);
                iassert(!IS_NAN(results->fraction));
            }
            else if ( model == 0x4000 )
            {
                if ( !results->allsolid )
                    CM_TestInTempBrush(&tw, results);
            }
            else
            {
                if ( !results->allsolid )
                    CM_TestInLeaf(&tw, &cmod->leaf, results);
                iassert(!IS_NAN(results->fraction));
            }
        }
        else
        {
            CM_PositionTest(&tw, results, context);
        }
    }
    else
    {
        if ( tw.size.vec.v[0] < 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 1612, 0, "%s", "tw.size[0] >= 0") )
        {
            __debugbreak();
        }
        if ( tw.size.vec.v[1] < 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 1613, 0, "%s", "tw.size[1] >= 0") )
        {
            __debugbreak();
        }
        if ( tw.size.vec.v[2] < 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 1614, 0, "%s", "tw.size[2] >= 0") )
        {
            __debugbreak();
        }
        tw.isPoint = (float)((float)(tw.size.vec.v[0] + tw.size.vec.v[1]) + tw.size.vec.v[2]) == 0.0;
        tw.doublesided = doublesided_raycasts->current.enabled;
        if ( tw.offsetZ < 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 1623, 0, "%s", "tw.offsetZ >= 0") )
        {
            __debugbreak();
        }
        tw.radiusOffset.vec.v[0] = tw.radius;
        tw.radiusOffset.vec.v[1] = tw.radius;
        tw.radiusOffset.vec.v[2] = tw.radius + tw.offsetZ;
        if ( model )
        {
            if ( model == 0x3FFF )
            {
                if ( (tw.contents & tw.threadInfo.box_brush->contents) != 0 )
                    CM_TraceCapsuleThroughCapsule(&tw, results);
            }
            else if ( model == 0x4000 )
            {
                CM_TraceThroughTempBrush(&tw, results);
            }
            else
            {
                CM_TraceThroughLeaf(&tw, &cmod->leaf, results);
            }
        }
        else
        {
            start_[0] = tw.extents.start.vec.v[0];
            start_[1] = tw.extents.start.vec.v[1];
            start_[2] = tw.extents.start.vec.v[2];
            start_[3] = 0.0f;
            end_[0] = tw.extents.end.vec.v[0];
            end_[1] = tw.extents.end.vec.v[1];
            end_[2] = tw.extents.end.vec.v[2];
            end_[3] = results->fraction;

            if ( context->nprims )
                CM_TraceThroughPrimitives(&tw, (const float *)start_, (const float *)end_, results, context);
            else
                CM_TraceThroughTree(&tw, 0, (const float *)start_, (const float *)end_, results);
        }
    }
    if ( !results->walkable && !results->startsolid )
        results->walkable = results->normal.vec.v[2] >= 0.69999999;
    if ( results->fraction < oldFrac )
    {
        if ( results->sflags == -1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        1674,
                        0,
                        "%s",
                        "results->sflags != SURF_INVALID") )
        {
            __debugbreak();
        }
    }
    else
    {
        if ( results->sflags != -1
            && results->fraction != oldFrac
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        1669,
                        0,
                        "%s",
                        "results->sflags == SURF_INVALID || results->fraction == oldFrac") )
        {
            __debugbreak();
        }
        results->sflags = oldSurfaceFlags;
    }
}

void __cdecl CM_TestInLeaf(traceWork_t *tw, cLeaf_s *leaf, trace_t *trace)
{
    if ( ((tw->contents & leaf->brushContents) == 0 || !CM_TestInLeafBrushNode(tw, leaf, trace))
        && (tw->contents & leaf->terrainContents) != 0 )
    {
        CM_MeshTestInLeaf(tw, leaf, trace);
    }
}

bool __cdecl CM_TestInLeafBrushNode(traceWork_t *tw, cLeaf_s *leaf, trace_t *trace)
{
    int i; // [esp+4h] [ebp-4h]

    if ( !leaf->leafBrushNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                    382,
                    0,
                    "%s",
                    "leaf->leafBrushNode") )
    {
        __debugbreak();
    }
    for ( i = 0; i < 3; ++i )
    {
        if ( leaf->mins[i] >= tw->bounds[1].vec.v[i] )
            return 0;
        if ( tw->bounds[0].vec.v[i] >= leaf->maxs[i] )
            return 0;
    }
    CM_TestInLeafBrushNode_r(tw, &cm.leafbrushNodes[leaf->leafBrushNode], trace);
    return trace->allsolid;
}

void __cdecl CM_TestInLeafBrushNode_r(const traceWork_t *tw, cLeafBrushNode_s *node, trace_t *trace)
{
    int k; // [esp+Ch] [ebp-14h]
    cbrush_t *b; // [esp+10h] [ebp-10h]
    unsigned __int16 *brushes; // [esp+18h] [ebp-8h]

    if ( !node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 326, 0, "%s", "PPU(node)") )
        __debugbreak();
    while ( (tw->contents & node->contents) != 0 )
    {
        if ( node->leafBrushCount )
        {
            if ( node->leafBrushCount > 0 )
            {
                brushes = node->data.leaf.brushes;
                for ( k = 0; k < node->leafBrushCount; ++k )
                {
                    b = &cm.brushes[brushes[k]];
                    if ( (tw->contents & b->contents) != 0 )
                    {
                        CM_TestBoxInBrush(tw, b, trace);
                        if ( trace->allsolid )
                            break;
                    }
                }
                return;
            }
            CM_TestInLeafBrushNode_r(tw, node + 1, trace);
            if ( trace->allsolid )
                return;
        }
        if ( tw->bounds[0].vec.v[node->axis] <= node->data.children.dist )
        {
            if ( tw->bounds[1].vec.v[node->axis] >= node->data.children.dist )
            {
                CM_TestInLeafBrushNode_r(tw, &node[node->data.children.childOffset[0]], trace);
                if ( trace->allsolid )
                    return;
            }
            node += node->data.children.childOffset[1];
        }
        else
        {
            node += node->data.children.childOffset[0];
        }
    }
}

void __cdecl CM_TestBoxInBrush(const traceWork_t *tw, const cbrush_t *brush, trace_t *trace)
{
    float d1; // [esp+78h] [ebp-18h]
    cbrushside_t *side; // [esp+7Ch] [ebp-14h]
    const cplane_s *plane; // [esp+80h] [ebp-10h]
    float dist; // [esp+84h] [ebp-Ch]
    signed int i; // [esp+8Ch] [ebp-4h]

    if ( ((tw->extents.start.vec.u[0] & 0x7F800000) == 0x7F800000
         || (tw->extents.start.vec.u[1] & 0x7F800000) == 0x7F800000
         || (tw->extents.start.vec.u[2] & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                    255,
                    0,
                    "%s",
                    "!IS_NAN((tw->extents.start)[0]) && !IS_NAN((tw->extents.start)[1]) && !IS_NAN((tw->extents.start)[2])") )
    {
        __debugbreak();
    }
    if ( ((tw->extents.end.vec.u[0] & 0x7F800000) == 0x7F800000
         || (tw->extents.end.vec.u[1] & 0x7F800000) == 0x7F800000
         || (tw->extents.end.vec.u[2] & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                    256,
                    0,
                    "%s",
                    "!IS_NAN((tw->extents.end)[0]) && !IS_NAN((tw->extents.end)[1]) && !IS_NAN((tw->extents.end)[2])") )
    {
        __debugbreak();
    }
    if ( tw->bounds[0].vec.v[0] < brush->maxs[0]
        && tw->bounds[0].vec.v[1] < brush->maxs[1]
        && tw->bounds[0].vec.v[2] < brush->maxs[2]
        && brush->mins[0] < tw->bounds[1].vec.v[0]
        && brush->mins[1] < tw->bounds[1].vec.v[1]
        && brush->mins[2] < tw->bounds[1].vec.v[2] )
    {
        side = brush->sides;
        i = brush->numsides;
        if ( i < 0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 277, 0, "%s", "i >= 0") )
            __debugbreak();
        while ( i )
        {
            plane = side->plane;
            if ( (LODWORD(side->plane->dist) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                            284,
                            0,
                            "%s",
                            "!IS_NAN(plane->dist)") )
            {
                __debugbreak();
            }
            if ( (LODWORD(tw->radius) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                            285,
                            0,
                            "%s",
                            "!IS_NAN(tw->radius)") )
            {
                __debugbreak();
            }
            if ( ((LODWORD(plane->normal[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(plane->normal[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(plane->normal[2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                            288,
                            0,
                            "%s",
                            "!IS_NAN((plane->normal)[0]) && !IS_NAN((plane->normal)[1]) && !IS_NAN((plane->normal)[2])") )
            {
                __debugbreak();
            }
            if ( (LODWORD(tw->offsetZ) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                            289,
                            0,
                            "%s",
                            "!IS_NAN(tw->offsetZ)") )
            {
                __debugbreak();
            }
            dist = (float)(plane->dist + tw->radius)
                     + fabs(plane->normal[2] * tw->offsetZ);
            if ( (LODWORD(dist) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 292, 0, "%s", "!IS_NAN(dist)") )
            {
                __debugbreak();
            }
            d1 = (float)((float)((float)(tw->extents.start.vec.v[0] * plane->normal[0])
                                                 + (float)(tw->extents.start.vec.v[1] * plane->normal[1]))
                                 + (float)(tw->extents.start.vec.v[2] * plane->normal[2]))
                 - dist;
            if ( (LODWORD(d1) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 296, 0, "%s", "!IS_NAN(d1)") )
            {
                __debugbreak();
            }
            if ( d1 > 0.0 )
                return;
            --i;
            ++side;
        }
        trace->startsolid = 1;
        trace->allsolid = 1;
        trace->fraction = 0.0f;
        trace->cflags = brush->contents;
        trace->sflags = 0;
    }
}

void __cdecl CM_TestCapsuleInCapsule(const traceWork_t *tw, trace_t *trace)
{
    float v2; // [esp+4h] [ebp-94h]
    float r; // [esp+1Ch] [ebp-7Ch]
    __int64 top; // [esp+20h] [ebp-78h]
    float top_8; // [esp+28h] [ebp-70h]
    float offs; // [esp+2Ch] [ebp-6Ch]
    float p1; // [esp+34h] [ebp-64h]
    float p1_4; // [esp+38h] [ebp-60h]
    float p1_8; // [esp+3Ch] [ebp-5Ch]
    float symetricSize[2][3]; // [esp+40h] [ebp-58h]
    float radius; // [esp+58h] [ebp-40h]
    float fTotalHalfHeight; // [esp+5Ch] [ebp-3Ch]
    float offset[3]; // [esp+60h] [ebp-38h]
    float p2[3]; // [esp+6Ch] [ebp-2Ch]
    float tmp[3]; // [esp+78h] [ebp-20h]
    int i; // [esp+84h] [ebp-14h]
    float bottom[3]; // [esp+88h] [ebp-10h]
    float fHeightDiff; // [esp+94h] [ebp-4h]

    top = *(_QWORD *)tw->extents.start.vec.v;
    top_8 = tw->extents.start.vec.v[2] + tw->offsetZ;
    LODWORD(bottom[0]) = top;
    bottom[1] = tw->extents.start.vec.v[1];
    bottom[2] = tw->extents.start.vec.v[2] - tw->offsetZ;
    for ( i = 0; i < 3; ++i )
    {
        offset[i] = (float)(tw->threadInfo.box_model->mins[i] + tw->threadInfo.box_model->maxs[i]) * 0.5;
        symetricSize[0][i] = tw->threadInfo.box_model->mins[i] - offset[i];
        symetricSize[1][i] = tw->threadInfo.box_model->maxs[i] - offset[i];
    }
    if ( symetricSize[1][0] <= symetricSize[1][2] )
        v2 = symetricSize[1][0];
    else
        v2 = symetricSize[1][2];
    radius = v2;
    offs = symetricSize[1][2] - v2;
    r = (float)(tw->radius + v2) * (float)(tw->radius + v2);
    p1 = offset[0];
    p1_4 = offset[1];
    p1_8 = offset[2] + (float)(symetricSize[1][2] - v2);
    tmp[0] = offset[0] - *(float *)&top;
    tmp[1] = offset[1] - *((float *)&top + 1);
    tmp[2] = p1_8 - top_8;
    if ( r > (float)((float)((float)((float)(offset[0] - *(float *)&top) * (float)(offset[0] - *(float *)&top))
                                                 + (float)((float)(offset[1] - *((float *)&top + 1)) * (float)(offset[1] - *((float *)&top + 1))))
                                 + (float)((float)(p1_8 - top_8) * (float)(p1_8 - top_8))) )
        goto LABEL_8;
    tmp[0] = offset[0] - bottom[0];
    tmp[1] = offset[1] - bottom[1];
    tmp[2] = p1_8 - bottom[2];
    if ( r <= (float)((float)((float)((float)(offset[0] - bottom[0]) * (float)(offset[0] - bottom[0]))
                                                    + (float)((float)(offset[1] - bottom[1]) * (float)(offset[1] - bottom[1])))
                                    + (float)((float)(p1_8 - bottom[2]) * (float)(p1_8 - bottom[2]))) )
    {
        p2[0] = offset[0];
        p2[1] = offset[1];
        p2[2] = offset[2] - offs;
        tmp[0] = offset[0] - *(float *)&top;
        tmp[1] = offset[1] - *((float *)&top + 1);
        tmp[2] = (float)(offset[2] - offs) - top_8;
        if ( r > (float)((float)((float)((float)(offset[0] - *(float *)&top) * (float)(offset[0] - *(float *)&top))
                                                     + (float)((float)(offset[1] - *((float *)&top + 1))
                                                                     * (float)(offset[1] - *((float *)&top + 1))))
                                     + (float)(tmp[2] * tmp[2])) )
            goto LABEL_18;
        tmp[0] = p2[0] - bottom[0];
        tmp[1] = p2[1] - bottom[1];
        tmp[2] = p2[2] - bottom[2];
        if ( r > (float)((float)((float)((float)(p2[0] - bottom[0]) * (float)(p2[0] - bottom[0]))
                                                     + (float)((float)(p2[1] - bottom[1]) * (float)(p2[1] - bottom[1])))
                                     + (float)((float)(p2[2] - bottom[2]) * (float)(p2[2] - bottom[2]))) )
        {
LABEL_8:
            trace->startsolid = 1;
            trace->allsolid = 1;
            trace->fraction = 0.0f;
            trace->sflags = 0;
            return;
        }
        fHeightDiff = tw->extents.start.vec.v[2] - offset[2];
        fTotalHalfHeight = (float)(offs + tw->size.vec.v[2]) - tw->radius;
        if ( fTotalHalfHeight < 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        487,
                        0,
                        "%s",
                        "fTotalHalfHeight >= 0") )
        {
            __debugbreak();
        }
        if ( fTotalHalfHeight >= fabs(fHeightDiff) )
        {
            tmp[0] = *(float *)&top - p1;
            tmp[1] = *((float *)&top + 1) - p1_4;
            tmp[2] = 0.0 - 0.0;
            if ( r > (float)((float)((float)((float)(*(float *)&top - p1) * (float)(*(float *)&top - p1))
                                                         + (float)((float)(*((float *)&top + 1) - p1_4) * (float)(*((float *)&top + 1) - p1_4)))
                                         + (float)((float)(0.0 - 0.0) * (float)(0.0 - 0.0))) )
            {
LABEL_18:
                trace->startsolid = 1;
                trace->allsolid = 1;
                trace->fraction = 0.0f;
                trace->sflags = 0;
            }
        }
    }
    else
    {
        trace->startsolid = 1;
        trace->allsolid = 1;
        trace->fraction = 0.0f;
        trace->sflags = 0;
    }
}

void __cdecl CM_PositionTest(traceWork_t *tw, trace_t *trace, col_context_t *context)
{
    leafList_s ll; // [esp+18h] [ebp-844h] BYREF
    unsigned __int16 leafs[1026]; // [esp+44h] [ebp-818h] BYREF
    int j; // [esp+84Ch] [ebp-10h]
    const cbrush_t *brush; // [esp+850h] [ebp-Ch]
    int i; // [esp+854h] [ebp-8h]
    const col_prim_t *prim; // [esp+858h] [ebp-4h]

    if ( context->nprims )
    {
        prim = context->prims;
        i = 0;
        while ( i < context->nprims && !trace->allsolid )
        {
            if ( prim->type )
            {
                if ( prim->type != 1
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                                524,
                                0,
                                "%s",
                                "prim->type == COL_BRUSH") )
                {
                    __debugbreak();
                }
                brush = prim->brush;
                if ( (tw->contents & brush->contents) != 0 )
                    CM_TestBoxInBrush(tw, brush, trace);
            }
            else
            {
                CM_MeshTest(tw, prim->tree, trace);
            }
            ++i;
            ++prim;
        }
    }
    else if ( !trace->allsolid )
    {
        ll.bounds[0][0] = tw->extents.start.vec.v[0] - tw->size.vec.v[0];
        ll.bounds[0][1] = tw->extents.start.vec.v[1] - tw->size.vec.v[1];
        ll.bounds[0][2] = tw->extents.start.vec.v[2] - tw->size.vec.v[2];
        ll.bounds[1][0] = tw->extents.start.vec.v[0] + tw->size.vec.v[0];
        ll.bounds[1][1] = tw->extents.start.vec.v[1] + tw->size.vec.v[1];
        ll.bounds[1][2] = tw->extents.start.vec.v[2] + tw->size.vec.v[2];
        for ( j = 0; j < 3; ++j )
        {
            ll.bounds[0][j] = ll.bounds[0][j] - 1.0;
            ll.bounds[1][j] = ll.bounds[1][j] + 1.0;
        }
        ll.count = 0;
        ll.maxcount = 1024;
        ll.list = leafs;
        ll.lastLeaf = 0;
        ll.overflowed = 0;
        CM_BoxLeafnums_r(&ll, 0);
        if ( ll.count )
        {
            for ( j = 0; j < ll.count && !trace->allsolid; ++j )
                CM_TestInLeaf(tw, &cm.leafs[leafs[j]], trace);
        }
    }
}

void __cdecl CM_TraceThroughLeaf(const traceWork_t *tw, cLeaf_s *leaf, trace_t *trace)
{
    const CollisionAabbTree *aabbTree; // [esp+8h] [ebp-8h]
    int k; // [esp+Ch] [ebp-4h]

    if ( trace->fraction != 0.0
        && ((tw->contents & leaf->brushContents) == 0 || !CM_TraceThroughLeafBrushNode(tw, leaf, trace))
        && (tw->contents & leaf->terrainContents) != 0 )
    {
        for ( k = 0; k < leaf->collAabbCount; ++k )
        {
            aabbTree = &cm.aabbTrees[k + leaf->firstCollAabbIndex];
            _mm_prefetch((const char *)aabbTree, 1);
            if ( trace->fraction == 0.0 )
                break;
            CM_TraceThroughAabbTree(tw, aabbTree, trace);
        }
    }
}

bool __cdecl CM_TraceThroughLeafBrushNode(const traceWork_t *tw, cLeaf_s *leaf, trace_t *trace)
{
    float absmin[3]; // [esp+24h] [ebp-3Ch] BYREF
    _QWORD start[2]; // [esp+34h] [ebp-2Ch] BYREF
    _QWORD end[2]; // [esp+44h] [ebp-1Ch] BYREF
    float absmax[3]; // [esp+54h] [ebp-Ch] BYREF

    if ( !leaf->leafBrushNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                    915,
                    0,
                    "%s",
                    "leaf->leafBrushNode") )
    {
        __debugbreak();
    }
    absmin[0] = leaf->mins[0] - tw->size.vec.v[0];
    absmin[1] = leaf->mins[1] - tw->size.vec.v[1];
    absmin[2] = leaf->mins[2] - tw->size.vec.v[2];
    absmax[0] = leaf->maxs[0] + tw->size.vec.v[0];
    absmax[1] = leaf->maxs[1] + tw->size.vec.v[1];
    absmax[2] = leaf->maxs[2] + tw->size.vec.v[2];
    if ( !intersect_extents_aabb(&tw->extents, absmin, absmax, trace->fraction) )
        return 0;
    start[0] = *(_QWORD *)tw->extents.start.vec.v;
    LODWORD(start[1]) = tw->extents.start.vec.u[2];
    end[0] = *(_QWORD *)tw->extents.end.vec.v;
    LODWORD(end[1]) = tw->extents.end.vec.u[2];
    HIDWORD(start[1]) = 0;
    HIDWORD(end[1]) = LODWORD(trace->fraction);
    CM_TraceThroughLeafBrushNode_r(
        tw,
        &cm.leafbrushNodes[leaf->leafBrushNode],
        (const float *)start,
        (const float *)end,
        trace);
    return trace->fraction == 0.0;
}

void __cdecl CM_TraceThroughLeafBrushNode_r(
                const traceWork_t *tw,
                cLeafBrushNode_s *node,
                const float *p1_,
                const float *p2,
                trace_t *trace)
{
    float v5; // [esp+10h] [ebp-84h]
    float v6; // [esp+14h] [ebp-80h]
    float v7; // [esp+1Ch] [ebp-78h]
    float v8; // [esp+20h] [ebp-74h]
    float v9; // [esp+24h] [ebp-70h]
    bool side; // [esp+34h] [ebp-60h]
    float diff; // [esp+38h] [ebp-5Ch]
    float t1; // [esp+40h] [ebp-54h]
    float frac; // [esp+44h] [ebp-50h]
    int k; // [esp+48h] [ebp-4Ch]
    float p1[4]; // [esp+50h] [ebp-44h] BYREF
    float offset; // [esp+60h] [ebp-34h]
    float tmax; // [esp+64h] [ebp-30h]
    float t2; // [esp+68h] [ebp-2Ch]
    float frac2; // [esp+6Ch] [ebp-28h]
    unsigned __int16 *brushes; // [esp+70h] [ebp-24h]
    float absDiff; // [esp+74h] [ebp-20h]
    float invDist; // [esp+78h] [ebp-1Ch]
    float tmin; // [esp+7Ch] [ebp-18h]
    int brushnum; // [esp+80h] [ebp-14h]
    float mid[4]; // [esp+84h] [ebp-10h] BYREF

    if ( !node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 811, 0, "%s", "PPU(node)") )
        __debugbreak();
    p1[0] = *p1_;
    p1[1] = p1_[1];
    p1[2] = p1_[2];
    p1[3] = p1_[3];
    while ( (tw->contents & node->contents) != 0 )
    {
        if ( node->leafBrushCount )
        {
            if ( node->leafBrushCount > 0 )
            {
                brushes = node->data.leaf.brushes;
                for ( k = 0; k < node->leafBrushCount; ++k )
                {
                    brushnum = brushes[k];
                    if ( (tw->contents & cm.brushes[brushnum].contents) != 0 )
                        CM_TraceThroughBrush(tw, &cm.brushes[brushnum], trace);
                }
                return;
            }
            CM_TraceThroughLeafBrushNode_r(tw, node + 1, p1, p2, trace);
        }
        t1 = p1[node->axis] - node->data.children.dist;
        t2 = p2[node->axis] - node->data.children.dist;
        offset = (float)(tw->size.vec.v[node->axis] + 0.125) - node->data.children.range;
        if ( (float)(t1 - t2) < 0.0 )
            v9 = t2;
        else
            v9 = t1;
        tmax = v9;
        if ( (float)(t2 - t1) < 0.0 )
            v8 = t2;
        else
            v8 = t1;
        tmin = v8;
        if ( v8 < offset )
        {
            //if ( COERCE_FLOAT(LODWORD(offset) ^ _mask__NegFloat_) < tmax )
            if ( -offset < tmax )
            {
                if ( p1[3] >= trace->fraction )
                    return;
                diff = t2 - t1;
                absDiff = fabs(t2 - t1);
                if ( absDiff <= 0.00000047683716 )
                {
                    side = 0;
                    frac = 1.0f;
                    frac2 = 0.0f;
                }
                else
                {
                    if ( diff < 0.0 )
                        v7 = t1;
                    else
                        v7 = -t1;
                    invDist = 1.0 / absDiff;
                    frac2 = (float)(v7 - offset) * (float)(1.0 / absDiff);
                    frac = (float)(v7 + offset) * (float)(1.0 / absDiff);
                    side = diff >= 0.0;
                }
                if ( frac < 0.0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                                885,
                                0,
                                "frac >= 0.0f\n\t%g, %g",
                                frac,
                                0.0) )
                {
                    __debugbreak();
                }
                if ( (float)(1.0 - frac) < 0.0 )
                    v6 = 1.0f;
                else
                    v6 = frac;
                mid[0] = (float)((float)(*p2 - p1[0]) * v6) + p1[0];
                mid[1] = (float)((float)(p2[1] - p1[1]) * v6) + p1[1];
                mid[2] = (float)((float)(p2[2] - p1[2]) * v6) + p1[2];
                mid[3] = (float)((float)(p2[3] - p1[3]) * v6) + p1[3];
                CM_TraceThroughLeafBrushNode_r(tw, &node[node->data.children.childOffset[side]], p1, mid, trace);
                if ( frac2 > 1.0000005
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                                895,
                                0,
                                "frac2 <= 1.0f + TRACE_EPSILON\n\t%g, %g",
                                frac2,
                                1.000000476837158) )
                {
                    __debugbreak();
                }
                if ( (float)(frac2 - 0.0) < 0.0 )
                    v5 = 0.0f;
                else
                    v5 = frac2;
                frac2 = v5;
                p1[0] = (float)((float)(*p2 - p1[0]) * v5) + p1[0];
                p1[1] = (float)((float)(p2[1] - p1[1]) * v5) + p1[1];
                p1[2] = (float)((float)(p2[2] - p1[2]) * v5) + p1[2];
                p1[3] = (float)((float)(p2[3] - p1[3]) * v5) + p1[3];
                node += node->data.children.childOffset[1 - side];
            }
            else
            {
                node += node->data.children.childOffset[1];
            }
        }
        else
        {
            //if ( COERCE_FLOAT(LODWORD(offset) ^ _mask__NegFloat_) >= tmax )
            if ( -offset >= tmax )
                return;
            node += node->data.children.childOffset[0];
        }
    }
}

void __cdecl CM_TraceThroughBrush(const traceWork_t *tw, const cbrush_t *brush, trace_t *trace)
{
    float v3; // [esp+8h] [ebp-128h]
    float v4; // [esp+Ch] [ebp-124h]
    float v5; // [esp+10h] [ebp-120h]
    float d1; // [esp+C0h] [ebp-70h]
    float d1a; // [esp+C0h] [ebp-70h]
    cbrushside_t *side; // [esp+C4h] [ebp-6Ch]
    int j; // [esp+C8h] [ebp-68h]
    const cplane_s *plane; // [esp+CCh] [ebp-64h]
    float enterFrac; // [esp+D0h] [ebp-60h]
    float delta; // [esp+D4h] [ebp-5Ch]
    float deltaa; // [esp+D4h] [ebp-5Ch]
    bool allsolid; // [esp+DBh] [ebp-55h]
    float frac; // [esp+DCh] [ebp-54h]
    float fraca; // [esp+DCh] [ebp-54h]
    float dist; // [esp+E0h] [ebp-50h]
    cbrushside_t *leadside; // [esp+E4h] [ebp-4Ch]
    cbrushside_t axialSide; // [esp+E8h] [ebp-48h] BYREF
    cplane_s axialPlane; // [esp+F4h] [ebp-3Ch] BYREF
    float sign; // [esp+10Ch] [ebp-24h]
    float d2; // [esp+110h] [ebp-20h]
    const float *bounds; // [esp+114h] [ebp-1Ch]
    float f; // [esp+118h] [ebp-18h]
    int index; // [esp+11Ch] [ebp-14h]
    float leaveFrac; // [esp+120h] [ebp-10h]
    float offsetDotNormal; // [esp+124h] [ebp-Ch]
    int i; // [esp+128h] [ebp-8h]
    const cplane_s *leadPlane; // [esp+12Ch] [ebp-4h]

    if ( ((tw->extents.start.vec.u[0] & 0x7F800000) == 0x7F800000
         || (tw->extents.start.vec.u[1] & 0x7F800000) == 0x7F800000
         || (tw->extents.start.vec.u[2] & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                    594,
                    0,
                    "%s",
                    "!IS_NAN((tw->extents.start)[0]) && !IS_NAN((tw->extents.start)[1]) && !IS_NAN((tw->extents.start)[2])") )
    {
        __debugbreak();
    }
    if ( ((tw->extents.end.vec.u[0] & 0x7F800000) == 0x7F800000
         || (tw->extents.end.vec.u[1] & 0x7F800000) == 0x7F800000
         || (tw->extents.end.vec.u[2] & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                    595,
                    0,
                    "%s",
                    "!IS_NAN((tw->extents.end)[0]) && !IS_NAN((tw->extents.end)[1]) && !IS_NAN((tw->extents.end)[2])") )
    {
        __debugbreak();
    }
    enterFrac = 0.0f;
    leaveFrac = trace->fraction;
    allsolid = 1;
    leadside = 0;
    sign = -1.0f;
    bounds = (const float *)brush;
    index = 0;
    while ( 2 )
    {
        nanassertvec3(bounds);
        nanassertvec3(tw->radiusOffset.vec.v);

        for ( j = 0; j < 3; ++j )
        {
            d1 = (float)((float)(tw->extents.start.vec.v[j] - bounds[j]) * sign) - tw->radiusOffset.vec.v[j];
            d2 = (float)((float)(tw->extents.end.vec.v[j] - bounds[j]) * sign) - tw->radiusOffset.vec.v[j];
            if ( (LODWORD(d1) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 629, 0, "%s", "!IS_NAN(d1)") )
            {
                __debugbreak();
            }
            if ( (LODWORD(d2) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 630, 0, "%s", "!IS_NAN(d2)") )
            {
                __debugbreak();
            }
            if ( d1 <= 0.0 )
            {
                if ( d2 > 0.0 )
                {
                    fraca = (float)(d1 * tw->extents.invDelta.vec.v[j]) * sign;
                    if ( enterFrac >= fraca )
                        return;
                    allsolid = 0;
                    if ( (float)(fraca - leaveFrac) < 0.0 )
                        v4 = (float)(d1 * tw->extents.invDelta.vec.v[j]) * sign;
                    else
                        v4 = leaveFrac;
                    leaveFrac = v4;
                }
            }
            else
            {
                if ( (float)(0.125 - d1) < 0.0 )
                    v5 = 0.125f;
                else
                    v5 = d1;
                if ( d2 >= v5 )
                    return;
                frac = (float)((float)(d1 - 0.125) * tw->extents.invDelta.vec.v[j]) * sign;
                if ( frac >= leaveFrac )
                    return;
                if ( d2 > 0.0 )
                    allsolid = 0;
                if ( frac <= enterFrac )
                {
                    if ( leadside )
                        continue;
                }
                else
                {
                    enterFrac = (float)((float)(d1 - 0.125) * tw->extents.invDelta.vec.v[j]) * sign;
                }
                axialSide.cflags = brush->axial_cflags[index][j];
                axialSide.sflags = brush->axial_sflags[index][j];
                memset(&axialPlane, 0, 12);
                axialPlane.normal[j] = sign;
                axialSide.plane = &axialPlane;
                leadside = &axialSide;
            }
        }
        if ( !index )
        {
            sign = 1.0f;
            bounds = brush->maxs;
            index = 1;
            continue;
        }
        break;
    }
    if ( brush->numsides )
    {
        side = brush->sides;
        i = brush->numsides;
        while ( i )
        {
            plane = side->plane;
            if ( (LODWORD(side->plane->dist) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                            688,
                            0,
                            "%s",
                            "!IS_NAN(plane->dist)") )
            {
                __debugbreak();
            }
            if ( (LODWORD(tw->radius) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                            689,
                            0,
                            "%s",
                            "!IS_NAN(tw->radius)") )
            {
                __debugbreak();
            }
            if ( ((LODWORD(plane->normal[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(plane->normal[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(plane->normal[2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                            692,
                            0,
                            "%s",
                            "!IS_NAN((plane->normal)[0]) && !IS_NAN((plane->normal)[1]) && !IS_NAN((plane->normal)[2])") )
            {
                __debugbreak();
            }
            if ( (LODWORD(tw->offsetZ) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                            693,
                            0,
                            "%s",
                            "!IS_NAN(tw->offsetZ)") )
            {
                __debugbreak();
            }
            offsetDotNormal = fabs(plane->normal[2] * tw->offsetZ);
            dist = (float)(plane->dist + tw->radius) + offsetDotNormal;
            if ( (LODWORD(dist) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 696, 0, "%s", "!IS_NAN(dist)") )
            {
                __debugbreak();
            }
            d1a = (float)((float)((float)(tw->extents.start.vec.v[0] * plane->normal[0])
                                                    + (float)(tw->extents.start.vec.v[1] * plane->normal[1]))
                                    + (float)(tw->extents.start.vec.v[2] * plane->normal[2]))
                    - dist;
            d2 = (float)((float)((float)(tw->extents.end.vec.v[0] * plane->normal[0])
                                                 + (float)(tw->extents.end.vec.v[1] * plane->normal[1]))
                                 + (float)(tw->extents.end.vec.v[2] * plane->normal[2]))
                 - dist;
            if ( (LODWORD(d1a) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 701, 0, "%s", "!IS_NAN(d1)") )
            {
                __debugbreak();
            }
            if ( (LODWORD(d2) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 702, 0, "%s", "!IS_NAN(d2)") )
            {
                __debugbreak();
            }
            if ( d1a <= 0.0 )
            {
                if ( d2 > 0.0 )
                {
                    deltaa = d1a - d2;
                    if ( (float)(d1a - d2) >= 0.0
                        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 732, 0, "%s", "delta < 0") )
                    {
                        __debugbreak();
                    }
                    if ( d1a > (float)(leaveFrac * deltaa) )
                    {
                        leaveFrac = d1a / deltaa;
                        if ( enterFrac >= (float)(d1a / deltaa) )
                            return;
                    }
                    allsolid = 0;
                }
            }
            else
            {
                if ( (float)(0.125 - d1a) < 0.0 )
                    v3 = 0.125f;
                else
                    v3 = d1a;
                if ( d2 >= v3 )
                    return;
                if ( d2 > 0.0 )
                    allsolid = 0;
                delta = d1a - d2;

                iassert(!IS_NAN(delta));
                //if ( (COERCE_UNSIGNED_INT(d1a - d2) & 0x7F800000) == 0x7F800000
                //    && !Assert_MyHandler(
                //                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                //                713,
                //                0,
                //                "%s",
                //                "!IS_NAN(delta)") )
                //{
                //    __debugbreak();
                //}
                if ( delta <= 0.0
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 715, 0, "%s", "delta > 0") )
                {
                    __debugbreak();
                }
                f = d1a - 0.125;
                if ( (float)(d1a - 0.125) <= (float)(enterFrac * delta) )
                {
                    if ( leadside )
                        goto LABEL_54;
                }
                else
                {
                    enterFrac = f / delta;
                    if ( (float)(f / delta) >= leaveFrac )
                        return;
                }
                leadside = side;
            }
LABEL_54:
            --i;
            ++side;
        }
    }
    trace->cflags = brush->contents;
    if ( leadside )
    {
        trace->fraction = enterFrac;
        if ( (trace->fraction < 0.0 || trace->fraction > 1.0)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        765,
                        1,
                        "%s\n\t(trace->fraction) = %g",
                        "(trace->fraction >= 0 && trace->fraction <= 1.0f)",
                        trace->fraction) )
        {
            __debugbreak();
        }
        leadPlane = leadside->plane;
        trace->normal.vec.v[0] = leadPlane->normal[0];
        trace->normal.vec.v[1] = leadPlane->normal[1];
        trace->normal.vec.v[2] = leadPlane->normal[2];
        trace->sflags = leadside->sflags;
        trace->walkable = 0;
    }
    else
    {
        trace->startsolid = 1;
        if ( allsolid )
        {
            trace->allsolid = 1;
            trace->fraction = 0.0f;
        }
        trace->sflags = brush->axial_sflags[1][2];
        trace->cflags = brush->axial_cflags[1][2];
    }
}

void __cdecl CM_TestInTempBrush(const traceWork_t *tw, trace_t *trace)
{
    const cbrush_t **info; // [esp+4h] [ebp-14h]
    unsigned int i; // [esp+Ch] [ebp-Ch]
    PhysGeomList *geoms; // [esp+14h] [ebp-4h]

    geoms = *tw->threadInfo.geoms;
    for ( i = 0; i < geoms->count; ++i )
    {
        info = (const cbrush_t **)&geoms->geoms[i];
        if ( *info )
        {
            if ( (tw->contents & (*info)->contents) != 0 )
            {
                CM_TestBoxInBrush(tw, *info, trace);
                if ( trace->allsolid )
                    break;
            }
        }
    }
}

void __cdecl CM_TraceThroughTempBrush(const traceWork_t *tw, trace_t *trace)
{
    const cbrush_t **info; // [esp+4h] [ebp-14h]
    unsigned int i; // [esp+Ch] [ebp-Ch]
    PhysGeomList *geoms; // [esp+14h] [ebp-4h]

    if ( trace->fraction != 0.0 )
    {
        geoms = *tw->threadInfo.geoms;
        for ( i = 0; i < geoms->count; ++i )
        {
            info = (const cbrush_t **)&geoms->geoms[i];
            if ( *info )
            {
                if ( (tw->contents & (*info)->contents) != 0 )
                {
                    CM_TraceThroughBrush(tw, *info, trace);
                    if ( trace->fraction == 0.0 )
                        break;
                }
            }
        }
    }
}

void __cdecl CM_TraceCapsuleThroughCapsule(const traceWork_t *tw, trace_t *trace)
{
    float v2; // [esp+10h] [ebp-B4h]
    float endtop[3]; // [esp+44h] [ebp-80h] BYREF
    float starttop[3]; // [esp+50h] [ebp-74h] BYREF
    float halfwidth; // [esp+5Ch] [ebp-68h]
    float top[3]; // [esp+60h] [ebp-64h] BYREF
    float offs; // [esp+6Ch] [ebp-58h]
    float halfheight; // [esp+70h] [ebp-54h]
    float symetricSize[2][3]; // [esp+74h] [ebp-50h]
    float radius; // [esp+8Ch] [ebp-38h]
    float offset[3]; // [esp+90h] [ebp-34h] BYREF
    float endbottom[3]; // [esp+9Ch] [ebp-28h] BYREF
    float startbottom[3]; // [esp+A8h] [ebp-1Ch] BYREF
    int i; // [esp+B4h] [ebp-10h]
    float bottom[3]; // [esp+B8h] [ebp-Ch] BYREF

    if ( tw->bounds[0].vec.v[0] <= (float)(tw->threadInfo.box_model->maxs[0] + 1.0)
        && tw->bounds[0].vec.v[1] <= (float)(tw->threadInfo.box_model->maxs[1] + 1.0)
        && tw->bounds[0].vec.v[2] <= (float)(tw->threadInfo.box_model->maxs[2] + 1.0)
        && (float)(tw->threadInfo.box_model->mins[0] - 1.0) <= tw->bounds[1].vec.v[0]
        && (float)(tw->threadInfo.box_model->mins[1] - 1.0) <= tw->bounds[1].vec.v[1]
        && (float)(tw->threadInfo.box_model->mins[2] - 1.0) <= tw->bounds[1].vec.v[2] )
    {
        *(_QWORD *)starttop = *(_QWORD *)tw->extents.start.vec.v;
        starttop[2] = tw->extents.start.vec.v[2] + tw->offsetZ;
        *(_QWORD *)startbottom = *(_QWORD *)tw->extents.start.vec.v;
        startbottom[2] = tw->extents.start.vec.v[2] - tw->offsetZ;
        *(_QWORD *)endtop = *(_QWORD *)tw->extents.end.vec.v;
        endtop[2] = tw->extents.end.vec.v[2] + tw->offsetZ;
        *(_QWORD *)endbottom = *(_QWORD *)tw->extents.end.vec.v;
        endbottom[2] = tw->extents.end.vec.v[2] - tw->offsetZ;
        for ( i = 0; i < 3; ++i )
        {
            offset[i] = (float)(tw->threadInfo.box_model->mins[i] + tw->threadInfo.box_model->maxs[i]) * 0.5;
            symetricSize[0][i] = tw->threadInfo.box_model->mins[i] - offset[i];
            symetricSize[1][i] = tw->threadInfo.box_model->maxs[i] - offset[i];
        }
        halfwidth = symetricSize[1][0];
        halfheight = symetricSize[1][2];
        if ( symetricSize[1][0] <= symetricSize[1][2] )
            v2 = halfwidth;
        else
            v2 = halfheight;
        radius = v2;
        offs = halfheight - v2;
        top[0] = offset[0];
        top[1] = offset[1];
        top[2] = offset[2] + (float)(halfheight - v2);
        bottom[0] = offset[0];
        bottom[1] = offset[1];
        bottom[2] = offset[2] - (float)(halfheight - v2);
        if ( startbottom[2] <= top[2] )
        {
            if ( bottom[2] > starttop[2]
                && (!CM_TraceSphereThroughSphere(tw, starttop, endtop, bottom, radius, trace) || tw->delta.vec.v[2] <= 0.0) )
            {
                return;
            }
        }
        else if ( !CM_TraceSphereThroughSphere(tw, startbottom, endbottom, top, radius, trace) || tw->delta.vec.v[2] >= 0.0 )
        {
            return;
        }
        if ( CM_TraceCylinderThroughCylinder(tw, offset, offs, radius, trace) )
        {
            if ( endbottom[2] <= top[2] )
            {
                if ( bottom[2] > endtop[2] && starttop[2] >= bottom[2] )
                    CM_TraceSphereThroughSphere(tw, starttop, endtop, bottom, radius, trace);
            }
            else if ( top[2] >= startbottom[2] )
            {
                CM_TraceSphereThroughSphere(tw, startbottom, endbottom, top, radius, trace);
            }
        }
    }
}

int __cdecl CM_TraceSphereThroughSphere(
                const traceWork_t *tw,
                const float *vStart,
                const float *vEnd,
                const float *vStationary,
                float radius,
                trace_t *trace)
{
    float v7; // [esp+8h] [ebp-50h]
    float fDiscriminant; // [esp+24h] [ebp-34h]
    float fEntry; // [esp+28h] [ebp-30h]
    float fRadiusSqrd; // [esp+2Ch] [ebp-2Ch]
    float fC; // [esp+30h] [ebp-28h]
    float fB; // [esp+34h] [ebp-24h]
    float fA; // [esp+38h] [ebp-20h]
    float vNormal[3]; // [esp+3Ch] [ebp-1Ch] BYREF
    float vDelta[3]; // [esp+48h] [ebp-10h] BYREF
    float fDeltaLen; // [esp+54h] [ebp-4h]

    vDelta[0] = *vStart - *vStationary;
    vDelta[1] = vStart[1] - vStationary[1];
    vDelta[2] = vStart[2] - vStationary[2];
    fRadiusSqrd = (float)(radius + tw->radius) * (float)(radius + tw->radius);
    fC = (float)((float)((float)(vDelta[0] * vDelta[0]) + (float)(vDelta[1] * vDelta[1])) + (float)(vDelta[2] * vDelta[2]))
         - fRadiusSqrd;
    if ( fC > 0.0 )
    {
        fB = (float)((float)(tw->delta.vec.v[0] * vDelta[0]) + (float)(tw->delta.vec.v[1] * vDelta[1]))
             + (float)(tw->delta.vec.v[2] * vDelta[2]);
        if ( fB < 0.0 )
        {
            fA = tw->deltaLenSq;
            if ( fA <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                            1082,
                            0,
                            "%s\n\t(tw->deltaLenSq) = %g",
                            "(fA > 0.0f)",
                            tw->deltaLenSq) )
            {
                __debugbreak();
            }
            fDiscriminant = (float)(fB * fB) - (float)(fA * fC);
            if ( fDiscriminant >= 0.0 )
            {
                fDeltaLen = Vec3NormalizeTo(vDelta, vNormal);
                //fEntry = (float)((float)(COERCE_FLOAT(LODWORD(fB) ^ _mask__NegFloat_) - sqrtf(fDiscriminant)) / fA) + (float)((float)(fDeltaLen * 0.125) / fB);
                fEntry = (float)((float)(-fB - sqrtf(fDiscriminant)) / fA) + (float)((float)(fDeltaLen * 0.125) / fB);
                if ( trace->fraction <= fEntry )
                {
                    return 1;
                }
                else
                {
                    if ( (float)(fEntry - 0.0) < 0.0 )
                        v7 = 0.0f;
                    else
                        v7 = fEntry;
                    trace->fraction = v7;
                    if ( (trace->fraction < 0.0 || trace->fraction > 1.0)
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                                    1095,
                                    1,
                                    "%s\n\t(trace->fraction) = %g",
                                    "(trace->fraction >= 0 && trace->fraction <= 1.0f)",
                                    trace->fraction) )
                    {
                        __debugbreak();
                    }
                    *(_QWORD *)trace->normal.vec.v = *(_QWORD *)vNormal;
                    trace->normal.vec.v[2] = vNormal[2];
                    trace->cflags = tw->threadInfo.box_brush->contents;
                    trace->walkable = 0;
                    trace->sflags = 0;
                    return 0;
                }
            }
            else
            {
                return 1;
            }
        }
        else
        {
            return 1;
        }
    }
    else
    {
        trace->fraction = 0.0f;
        trace->startsolid = 1;
        trace->walkable = 0;
        Vec3NormalizeTo(vDelta, trace->normal.vec.v);
        trace->cflags = tw->threadInfo.box_brush->contents;
        trace->sflags = 0;
        vDelta[0] = *vEnd - *vStationary;
        vDelta[1] = vEnd[1] - vStationary[1];
        vDelta[2] = vEnd[2] - vStationary[2];
        if ( fRadiusSqrd >= (float)((float)((float)(vDelta[0] * vDelta[0]) + (float)(vDelta[1] * vDelta[1]))
                                                            + (float)(vDelta[2] * vDelta[2])) )
            trace->allsolid = 1;
        return 0;
    }
}

int __cdecl CM_TraceCylinderThroughCylinder(
                const traceWork_t *tw,
                const float *vStationary,
                float fStationaryHalfHeight,
                float radius,
                trace_t *trace)
{
    float v6; // [esp+8h] [ebp-7Ch]
    float fDiscriminant; // [esp+44h] [ebp-40h]
    float fEntry; // [esp+48h] [ebp-3Ch]
    float fHitHeight; // [esp+50h] [ebp-34h]
    float fEpsilon; // [esp+54h] [ebp-30h]
    float fTotalHeight; // [esp+58h] [ebp-2Ch]
    float fTotalHeighta; // [esp+58h] [ebp-2Ch]
    float fC; // [esp+5Ch] [ebp-28h]
    float fB; // [esp+60h] [ebp-24h]
    float fA; // [esp+64h] [ebp-20h]
    float vNormal[3]; // [esp+68h] [ebp-1Ch] BYREF
    float vDelta[3]; // [esp+74h] [ebp-10h] BYREF
    float fDeltaLen; // [esp+80h] [ebp-4h]

    vDelta[0] = tw->extents.start.vec.v[0] - *vStationary;
    vDelta[1] = tw->extents.start.vec.v[1] - vStationary[1];
    vDelta[2] = tw->extents.start.vec.v[2] - vStationary[2];
    fC = (float)((float)(vDelta[0] * vDelta[0]) + (float)(vDelta[1] * vDelta[1]))
         - (float)((float)(radius + tw->radius) * (float)(radius + tw->radius));
    if ( fC > 0.0 )
    {
        fB = (float)(tw->delta.vec.v[0] * vDelta[0]) + (float)(tw->delta.vec.v[1] * vDelta[1]);
        if ( fB < 0.0 )
        {
            fA = (float)(tw->delta.vec.v[0] * tw->delta.vec.v[0]) + (float)(tw->delta.vec.v[1] * tw->delta.vec.v[1]);
            if ( fA <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                            1153,
                            0,
                            "%s\n\t(fA) = %g",
                            "(fA > 0.0f)",
                            fA) )
            {
                __debugbreak();
            }
            fDiscriminant = (float)(fB * fB) - (float)(fA * fC);
            if ( fDiscriminant >= 0.0 )
            {
                vDelta[2] = 0.0f;
                fDeltaLen = Vec3NormalizeTo(vDelta, vNormal);
                fEpsilon = (float)(fDeltaLen * 0.125) / fB;
                //fEntry = (float)((float)(COERCE_FLOAT(LODWORD(fB) ^ _mask__NegFloat_) - sqrtf(fDiscriminant)) / fA) + fEpsilon;
                fEntry = (float)((float)(-fB - sqrtf(fDiscriminant)) / fA) + fEpsilon;
                if ( trace->fraction <= fEntry )
                {
                    return 1;
                }
                else
                {
                    fTotalHeighta = (float)(tw->size.vec.v[2] - tw->radius) + fStationaryHalfHeight;
                    fHitHeight = (float)((float)((float)(fEntry - fEpsilon) * tw->delta.vec.v[2]) + tw->extents.start.vec.v[2])
                                         - vStationary[2];
                    if ( fTotalHeighta < 0.0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                                    1168,
                                    0,
                                    "%s",
                                    "fTotalHeight >= 0") )
                    {
                        __debugbreak();
                    }
                    if ( fabs(fHitHeight) <= fTotalHeighta )
                    {
                        if ( (float)(fEntry - 0.0) < 0.0 )
                            v6 = 0.0f;
                        else
                            v6 = fEntry;
                        trace->fraction = v6;
                        if ( (trace->fraction < 0.0 || trace->fraction > 1.0)
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                                        1173,
                                        1,
                                        "%s\n\t(trace->fraction) = %g",
                                        "(trace->fraction >= 0 && trace->fraction <= 1.0f)",
                                        trace->fraction) )
                        {
                            __debugbreak();
                        }
                        *(_QWORD *)trace->normal.vec.v = *(_QWORD *)vNormal;
                        trace->normal.vec.v[2] = vNormal[2];
                        trace->cflags = tw->threadInfo.box_brush->contents;
                        trace->sflags = 0;
                        trace->walkable = 0;
                        return 0;
                    }
                    else
                    {
                        return 1;
                    }
                }
            }
            else
            {
                return 1;
            }
        }
        else
        {
            return 1;
        }
    }
    else
    {
        fTotalHeight = (float)(tw->size.vec.v[2] - tw->radius) + fStationaryHalfHeight;
        if ( fTotalHeight < 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        1128,
                        0,
                        "%s",
                        "fTotalHeight >= 0") )
        {
            __debugbreak();
        }
        if ( fabs(vDelta[2]) <= fTotalHeight )
        {
            trace->fraction = 0.0f;
            trace->startsolid = 1;
            trace->walkable = 0;
            vDelta[2] = 0.0f;
            Vec3NormalizeTo(vDelta, trace->normal.vec.v);
            trace->cflags = tw->threadInfo.box_brush->contents;
            trace->sflags = 0;
            vDelta[0] = tw->extents.end.vec.v[0] - *vStationary;
            vDelta[1] = tw->extents.end.vec.v[1] - vStationary[1];
            vDelta[2] = tw->extents.end.vec.v[2] - vStationary[2];
            if ( fTotalHeight < 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                            1141,
                            0,
                            "%s",
                            "fTotalHeight >= 0") )
            {
                __debugbreak();
            }
            if ( fTotalHeight >= fabs(vDelta[2]) )
                trace->allsolid = 1;
            return 0;
        }
        else
        {
            return 1;
        }
    }
}

void __cdecl CM_TraceThroughTree(const traceWork_t *tw, int num, const float *p1_, const float *p2, trace_t *trace)
{
    float v5; // [esp+8h] [ebp-70h]
    float v6; // [esp+Ch] [ebp-6Ch]
    float v7; // [esp+14h] [ebp-64h]
    float v8; // [esp+18h] [ebp-60h]
    float v9; // [esp+1Ch] [ebp-5Ch]
    cNode_t *node; // [esp+24h] [ebp-54h]
    bool side; // [esp+28h] [ebp-50h]
    float diff; // [esp+2Ch] [ebp-4Ch]
    cplane_s *plane; // [esp+30h] [ebp-48h]
    float t1; // [esp+38h] [ebp-40h]
    float frac; // [esp+3Ch] [ebp-3Ch]
    float p1[4]; // [esp+44h] [ebp-34h] BYREF
    float offset; // [esp+54h] [ebp-24h]
    float t2; // [esp+58h] [ebp-20h]
    float frac2; // [esp+5Ch] [ebp-1Ch]
    float absDiff; // [esp+60h] [ebp-18h]
    float invDist; // [esp+64h] [ebp-14h]
    float mid[4]; // [esp+68h] [ebp-10h] BYREF

    p1[0] = *p1_;
    p1[1] = p1_[1];
    p1[2] = p1_[2];
    p1[3] = p1_[3];
    while ( num >= 0 )
    {
        node = &cm.nodes[num];
        plane = node->plane;
        if ( node->plane->type >= 3u )
        {
            t1 = (float)((float)((float)(plane->normal[0] * p1[0]) + (float)(plane->normal[1] * p1[1]))
                                 + (float)(plane->normal[2] * p1[2]))
                 - plane->dist;
            t2 = (float)((float)((float)(plane->normal[0] * *p2) + (float)(plane->normal[1] * p2[1]))
                                 + (float)(plane->normal[2] * p2[2]))
                 - plane->dist;
            if ( tw->isPoint )
                offset = 0.125f;
            else
                offset = tw->boundingRadius + 0.125;
        }
        else
        {
            t1 = p1[plane->type] - plane->dist;
            t2 = p2[plane->type] - plane->dist;
            offset = tw->size.vec.v[plane->type] + 0.125;
        }
        if ( (float)(t2 - t1) < 0.0 )
            v9 = t2;
        else
            v9 = t1;
        if ( v9 < offset )
        {
            if ( (float)(t1 - t2) < 0.0 )
                v8 = t2;
            else
                v8 = t1;
            //if ( COERCE_FLOAT(LODWORD(offset) ^ _mask__NegFloat_) < v8 )
            if ( -offset < v8 )
            {
                if ( p1[3] >= trace->fraction )
                    return;
                diff = t2 - t1;
                absDiff = fabs(t2 - t1);
                if ( absDiff <= 0.00000047683716 )
                {
                    side = 0;
                    frac = 1.0f;
                    frac2 = 0.0f;
                }
                else
                {
                    if ( diff < 0.0 )
                        v7 = t1;
                    else
                        v7 = -t1;
                    invDist = 1.0 / absDiff;
                    frac2 = (float)(v7 - offset) * (float)(1.0 / absDiff);
                    frac = (float)(v7 + offset) * (float)(1.0 / absDiff);
                    side = diff >= 0.0;
                }
                if ( frac < 0.0
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 1378, 0, "%s", "frac >= 0") )
                {
                    __debugbreak();
                }
                if ( (float)(1.0 - frac) < 0.0 )
                    v6 = 1.0f;
                else
                    v6 = frac;
                mid[0] = (float)((float)(*p2 - p1[0]) * v6) + p1[0];
                mid[1] = (float)((float)(p2[1] - p1[1]) * v6) + p1[1];
                mid[2] = (float)((float)(p2[2] - p1[2]) * v6) + p1[2];
                mid[3] = (float)((float)(p2[3] - p1[3]) * v6) + p1[3];
                CM_TraceThroughTree(tw, node->children[side], p1, mid, trace);
                if ( frac2 > 1.0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                                1389,
                                0,
                                "%s\n\t(frac) = %g",
                                "(frac2 <= 1.0f)",
                                v6) )
                {
                    __debugbreak();
                }
                if ( (float)(frac2 - 0.0) < 0.0 )
                    v5 = 0.0f;
                else
                    v5 = frac2;
                frac2 = v5;
                p1[0] = (float)((float)(*p2 - p1[0]) * v5) + p1[0];
                p1[1] = (float)((float)(p2[1] - p1[1]) * v5) + p1[1];
                p1[2] = (float)((float)(p2[2] - p1[2]) * v5) + p1[2];
                p1[3] = (float)((float)(p2[3] - p1[3]) * v5) + p1[3];
                num = node->children[!side];
            }
            else
            {
                num = node->children[1];
            }
        }
        else
        {
            num = node->children[0];
        }
    }
    CM_TraceThroughLeaf(tw, &cm.leafs[-1 - num], trace);
}

void __cdecl CM_TraceThroughPrimitives(
                const traceWork_t *tw,
                const float *p1_,
                const float *p2,
                trace_t *trace,
                col_context_t *context)
{
    bool v5; // [esp+3h] [ebp-E1h]
    float v6; // [esp+4h] [ebp-E0h] BYREF
    float v7; // [esp+8h] [ebp-DCh]
    float v8; // [esp+Ch] [ebp-D8h]
    int v9; // [esp+10h] [ebp-D4h]
    float v10; // [esp+14h] [ebp-D0h] BYREF
    float v11; // [esp+18h] [ebp-CCh]
    float v12; // [esp+1Ch] [ebp-C8h]
    int v13; // [esp+20h] [ebp-C4h]
    __int64 v14; // [esp+24h] [ebp-C0h]
    int v15; // [esp+2Ch] [ebp-B8h]
    float v16; // [esp+30h] [ebp-B4h]
    float v17; // [esp+34h] [ebp-B0h]
    float v18; // [esp+38h] [ebp-ACh]
    float v19; // [esp+3Ch] [ebp-A8h]
    float v20; // [esp+40h] [ebp-A4h]
    int v21; // [esp+44h] [ebp-A0h]
    float v22; // [esp+48h] [ebp-9Ch]
    float v23; // [esp+4Ch] [ebp-98h]
    float v24; // [esp+50h] [ebp-94h]
    __int64 v25; // [esp+54h] [ebp-90h]
    int v26; // [esp+5Ch] [ebp-88h]
    float v27; // [esp+60h] [ebp-84h]
    float v28; // [esp+64h] [ebp-80h]
    float v29; // [esp+68h] [ebp-7Ch]
    float v30; // [esp+6Ch] [ebp-78h]
    int v31; // [esp+70h] [ebp-74h]
    float v32; // [esp+74h] [ebp-70h]
    float v33; // [esp+78h] [ebp-6Ch]
    int v34; // [esp+7Ch] [ebp-68h]
    float v35; // [esp+80h] [ebp-64h]
    float v36; // [esp+84h] [ebp-60h]
    int v37; // [esp+88h] [ebp-5Ch]
    float v38; // [esp+8Ch] [ebp-58h]
    hybrid_vector *p_size; // [esp+90h] [ebp-54h]
    hybrid_vector *p_midpoint; // [esp+94h] [ebp-50h]
    float v41; // [esp+98h] [ebp-4Ch]
    float v42; // [esp+9Ch] [ebp-48h]
    float v43; // [esp+A0h] [ebp-44h]
    float v44; // [esp+A4h] [ebp-40h]
    float v45; // [esp+A8h] [ebp-3Ch]
    float v46; // [esp+ACh] [ebp-38h]
    float *v47; // [esp+B4h] [ebp-30h]
    float *halfSize; // [esp+B8h] [ebp-2Ch]
    float *v49; // [esp+BCh] [ebp-28h]
    const cbrush_t *brush; // [esp+C8h] [ebp-1Ch]
    float oldFraction; // [esp+CCh] [ebp-18h]
    dmaterial_t *materialInfo; // [esp+D0h] [ebp-14h]
    const CollisionAabbTree *tree; // [esp+D4h] [ebp-10h]
    int i; // [esp+DCh] [ebp-8h]
    const col_prim_t *prim; // [esp+E0h] [ebp-4h]

    if ( trace->fraction != 0.0 )
    {
        prim = context->prims;
        i = 0;
        while ( i < context->nprims && !trace->allsolid )
        {
            if ( prim->type )
            {
                if ( prim->type != 1
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                                1446,
                                0,
                                "%s",
                                "prim->type == COL_BRUSH") )
                {
                    __debugbreak();
                }
                brush = prim->brush;
                if ( (tw->contents & brush->contents) != 0 )
                    CM_TraceThroughBrush(tw, brush, trace);
            }
            else
            {
                tree = prim->tree;
                materialInfo = &cm.materials[tree->materialIndex];
                if ( (materialInfo->contentFlags & tw->contents) != 0 )
                {
                    halfSize = (float*)tree->halfSize;
                    v49 = &v10;
                    v10 = tree->halfSize[0];
                    v11 = tree->halfSize[1];
                    v12 = tree->halfSize[2];
                    v13 = 0;
                    v47 = &v6;
                    v6 = tree->origin[0];
                    v7 = tree->origin[1];
                    v8 = tree->origin[2];
                    v9 = 0;
                    p_midpoint = (hybrid_vector*)&tw->midpoint;
                    v41 = tw->midpoint.vec.v[0] - v6;
                    v42 = tw->midpoint.vec.v[1] - v7;
                    v43 = tw->midpoint.vec.v[2] - v8;
                    p_size = (hybrid_vector *)&tw->size;
                    v44 = v10 + tw->size.vec.v[0];
                    v45 = v11 + tw->size.vec.v[1];
                    v46 = v12 + tw->size.vec.v[2];
                    v38 = v41;
                    v37 = fabs(v41);
                    v36 = tw->halfDeltaAbs.vec.v[0];
                    if ( fabs(v41) <= (float)(v44 + v36) )
                    {
                        v35 = v42;
                        v34 = fabs(v42);
                        v33 = tw->halfDeltaAbs.vec.v[1];
                        if ( fabs(v42) <= (float)(v45 + v33) )
                        {
                            v32 = v43;
                            v31 = fabs(v43);
                            v30 = tw->halfDeltaAbs.vec.v[2];
                            if ( fabs(v43) <= (float)(v46 + v30) )
                            {
                                if ( tw->axialCullOnly )
                                {
                                    v5 = 0;
                                }
                                else
                                {
                                    v29 = tw->halfDelta.vec.v[1];
                                    v28 = tw->halfDelta.vec.v[2];
                                    v27 = (float)(v29 * v43) - (float)(v28 * v42);
                                    v26 = fabs(v27);
                                    v25 = *(_QWORD *)&tw->halfDeltaAbs.vec.unitVec[1].packed;
                                    if ( fabs(v27) <= (float)((float)(v45 * *((float *)&v25 + 1))
                                                                                                                                                            + (float)(v46 * *(float *)&v25)) )
                                    {
                                        v24 = tw->halfDelta.vec.v[2];
                                        v23 = tw->halfDelta.vec.v[0];
                                        v22 = (float)(v24 * v41) - (float)(v23 * v43);
                                        v21 = fabs(v22);
                                        v20 = tw->halfDeltaAbs.vec.v[0];
                                        v19 = tw->halfDeltaAbs.vec.v[2];
                                        if ( fabs(v22) <= (float)((float)(v46 * v20) + (float)(v44 * v19)) )
                                        {
                                            v18 = tw->halfDelta.vec.v[0];
                                            v17 = tw->halfDelta.vec.v[1];
                                            v16 = (float)(v18 * v42) - (float)(v17 * v41);
                                            v15 = fabs(v16);
                                            v14 = *(_QWORD *)tw->halfDeltaAbs.vec.v;
                                            v5 = fabs(v16) > (float)((float)(v44 * *((float *)&v14 + 1))
                                                                                                                                                                 + (float)(v45 * *(float *)&v14));
                                        }
                                        else
                                        {
                                            v5 = 1;
                                        }
                                    }
                                    else
                                    {
                                        v5 = 1;
                                    }
                                }
                            }
                            else
                            {
                                v5 = 1;
                            }
                        }
                        else
                        {
                            v5 = 1;
                        }
                    }
                    else
                    {
                        v5 = 1;
                    }
                    if ( !v5 )
                    {
                        oldFraction = trace->fraction;
                        CM_TraceThroughPartition(tw, tree->u.firstChildIndex, trace);
                        if ( oldFraction > trace->fraction )
                        {
                            trace->sflags = materialInfo->surfaceFlags;
                            trace->cflags = materialInfo->contentFlags;
                        }
                    }
                }
            }
            if ( trace->fraction == 0.0 )
                break;
            ++i;
            ++prim;
        }
    }
}

void __cdecl CM_SetAxialCullOnly(traceWork_t *tw)
{
    tw->axialCullOnly = (float)((float)(16.0 * (float)((float)(tw->size.vec.v[0] * tw->size.vec.v[1]) * tw->size.vec.v[2]))
                                                        * tw->deltaLen) > (float)((float)((float)(tw->bounds[1].vec.v[0] - tw->bounds[0].vec.v[0])
                                                                                                                        * (float)(tw->bounds[1].vec.v[1] - tw->bounds[0].vec.v[1]))
                                                                                                        * (float)(tw->bounds[1].vec.v[2] - tw->bounds[0].vec.v[2]));
}

void __cdecl CM_BoxTrace(
                trace_t *results,
                const float *start,
                const float *end,
                const float *mins,
                const float *maxs,
                int brushmask,
                col_context_t *context)
{
    memset(results, 0, sizeof(trace_t));
    results->fraction = 1.0f;
    results->boneIndex = 254;
    CM_Trace(results, start, end, mins, maxs, 0, brushmask, context);
}

void __cdecl CM_TransformedBoxTraceRotated(
                trace_t *results,
                const float *start,
                const float *end,
                const float *mins,
                const float *maxs,
                unsigned int model,
                int brushmask,
                const float *origin,
                const float (*matrix)[3])
{
    col_context_t context; // [esp+48h] [ebp-94h] BYREF
    float transpose[3][3]; // [esp+70h] [ebp-6Ch] BYREF
    float halfheight; // [esp+94h] [ebp-48h]
    float symetricSize[2][3]; // [esp+98h] [ebp-44h] BYREF
    float offset[3]; // [esp+B0h] [ebp-2Ch]
    float end_l[3]; // [esp+BCh] [ebp-20h] BYREF
    float start_l[3]; // [esp+C8h] [ebp-14h] BYREF
    int i; // [esp+D4h] [ebp-8h]
    float oldFraction; // [esp+D8h] [ebp-4h]

    ////col_context_t::col_context_t(&context);
    if ( !mins && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 1810, 0, "%s", "mins") )
        __debugbreak();
    if ( !maxs && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 1811, 0, "%s", "maxs") )
        __debugbreak();
    for ( i = 0; i < 3; ++i )
    {
        offset[i] = (float)(mins[i] + maxs[i]) * 0.5;
        symetricSize[0][i] = mins[i] - offset[i];
        symetricSize[1][i] = maxs[i] - offset[i];
        start_l[i] = start[i] + offset[i];
        end_l[i] = end[i] + offset[i];
    }
    start_l[0] = start_l[0] - *origin;
    start_l[1] = start_l[1] - origin[1];
    start_l[2] = start_l[2] - origin[2];
    end_l[0] = end_l[0] - *origin;
    end_l[1] = end_l[1] - origin[1];
    end_l[2] = end_l[2] - origin[2];
    halfheight = symetricSize[1][2];
    RotatePoint(start_l, matrix);
    RotatePoint(end_l, matrix);
    oldFraction = results->fraction;
    CM_Trace(results, start_l, end_l, symetricSize[0], symetricSize[1], model, brushmask, &context);
    if ( oldFraction > results->fraction )
    {
        TransposeMatrix(matrix, transpose);
        RotatePoint(results->normal.vec.v, transpose);
    }
}

void __cdecl TransposeMatrix(const float (*matrix)[3], float (*transpose)[3])
{
    int j; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    for ( i = 0; i < 3; ++i )
    {
        for ( j = 0; j < 3; ++j )
            (*transpose)[3 * i + j] = (*matrix)[3 * j + i];
    }
}

void __cdecl CM_TransformedBoxTrace(
                trace_t *results,
                const float *start,
                const float *end,
                const float *mins,
                const float *maxs,
                unsigned int model,
                int brushmask,
                const float *origin,
                const float *angles)
{
    float matrix[3][3]; // [esp+4h] [ebp-98h] BYREF
    float halfwidth; // [esp+28h] [ebp-74h]
    col_context_t context; // [esp+2Ch] [ebp-70h] BYREF
    float halfheight; // [esp+54h] [ebp-48h]
    float symetricSize[2][3]; // [esp+58h] [ebp-44h] BYREF
    float offset[3]; // [esp+70h] [ebp-2Ch]
    float end_l[3]; // [esp+7Ch] [ebp-20h] BYREF
    float start_l[3]; // [esp+88h] [ebp-14h] BYREF
    int i; // [esp+94h] [ebp-8h]
    float oldFraction; // [esp+98h] [ebp-4h]

    //col_context_t::col_context_t(&context);
    if ( !mins && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 1869, 0, "%s", "mins") )
        __debugbreak();
    if ( !maxs && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 1870, 0, "%s", "maxs") )
        __debugbreak();
    if ( *angles == 0.0 && angles[1] == 0.0 && angles[2] == 0.0 )
    {
        for ( i = 0; i < 3; ++i )
        {
            offset[i] = (float)(mins[i] + maxs[i]) * 0.5;
            symetricSize[0][i] = mins[i] - offset[i];
            symetricSize[1][i] = maxs[i] - offset[i];
            start_l[i] = start[i] + offset[i];
            end_l[i] = end[i] + offset[i];
        }
        start_l[0] = start_l[0] - *origin;
        start_l[1] = start_l[1] - origin[1];
        start_l[2] = start_l[2] - origin[2];
        end_l[0] = end_l[0] - *origin;
        end_l[1] = end_l[1] - origin[1];
        end_l[2] = end_l[2] - origin[2];
        halfwidth = symetricSize[1][0];
        halfheight = symetricSize[1][2];
        oldFraction = results->fraction;
        CM_Trace(results, start_l, end_l, symetricSize[0], symetricSize[1], model, brushmask, &context);
    }
    else
    {
        AnglesToAxis(angles, matrix);
        CM_TransformedBoxTraceRotated(results, start, end, mins, maxs, model, brushmask, origin, matrix);
    }
}

void __cdecl CM_TransformedBoxTraceExternal(
                trace_t *results,
                const float *start,
                const float *end,
                const float *mins,
                const float *maxs,
                unsigned int model,
                int brushmask,
                const float *origin,
                const float *angles)
{
    memset(results, 0, sizeof(trace_t));
    results->fraction = 1.0f;
    CM_TransformedBoxTrace(results, start, end, mins, maxs, model, brushmask, origin, angles);
}

int __cdecl CM_SightTraceThroughBrush(const traceWork_t *tw, const cbrush_t *brush, int brushNum, trace_t *trace)
{
    float v5; // [esp+0h] [ebp-ECh]
    float v6; // [esp+4h] [ebp-E8h]
    float d1; // [esp+B0h] [ebp-3Ch]
    float d1a; // [esp+B0h] [ebp-3Ch]
    cbrushside_t *side; // [esp+B4h] [ebp-38h]
    int j; // [esp+B8h] [ebp-34h]
    const cplane_s *plane; // [esp+BCh] [ebp-30h]
    float enterFrac; // [esp+C0h] [ebp-2Ch]
    float delta; // [esp+C4h] [ebp-28h]
    float deltaa; // [esp+C4h] [ebp-28h]
    float frac; // [esp+C8h] [ebp-24h]
    float fraca; // [esp+C8h] [ebp-24h]
    float dist; // [esp+CCh] [ebp-20h]
    float sign; // [esp+D0h] [ebp-1Ch]
    float d2; // [esp+D4h] [ebp-18h]
    float d2a; // [esp+D4h] [ebp-18h]
    const cbrush_t *bounds; // [esp+D8h] [ebp-14h]
    int index; // [esp+DCh] [ebp-10h]
    float leaveFrac; // [esp+E0h] [ebp-Ch]
    signed int i; // [esp+E8h] [ebp-4h]

    side = 0;
    if ( ((tw->extents.start.vec.u[0] & 0x7F800000) == 0x7F800000
         || (tw->extents.start.vec.u[1] & 0x7F800000) == 0x7F800000
         || (tw->extents.start.vec.u[2] & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                    1939,
                    0,
                    "%s",
                    "!IS_NAN((tw->extents.start)[0]) && !IS_NAN((tw->extents.start)[1]) && !IS_NAN((tw->extents.start)[2])") )
    {
        __debugbreak();
    }
    if ( ((tw->extents.end.vec.u[0] & 0x7F800000) == 0x7F800000
         || (tw->extents.end.vec.u[1] & 0x7F800000) == 0x7F800000
         || (tw->extents.end.vec.u[2] & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                    1940,
                    0,
                    "%s",
                    "!IS_NAN((tw->extents.end)[0]) && !IS_NAN((tw->extents.end)[1]) && !IS_NAN((tw->extents.end)[2])") )
    {
        __debugbreak();
    }
    enterFrac = 0.0f;
    leaveFrac = 1.0f;
    sign = -1.0f;
    bounds = brush;
    for ( index = 0; ; index = 1 )
    {
        if ( ((LODWORD(bounds->mins[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(bounds->mins[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(bounds->mins[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        1963,
                        0,
                        "%s",
                        "!IS_NAN((bounds)[0]) && !IS_NAN((bounds)[1]) && !IS_NAN((bounds)[2])") )
        {
            __debugbreak();
        }
        if ( ((tw->radiusOffset.vec.u[0] & 0x7F800000) == 0x7F800000
             || (tw->radiusOffset.vec.u[1] & 0x7F800000) == 0x7F800000
             || (tw->radiusOffset.vec.u[2] & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        1964,
                        0,
                        "%s",
                        "!IS_NAN((tw->radiusOffset)[0]) && !IS_NAN((tw->radiusOffset)[1]) && !IS_NAN((tw->radiusOffset)[2])") )
        {
            __debugbreak();
        }
        for ( j = 0; j < 3; ++j )
        {
            d1 = (float)((float)(tw->extents.start.vec.v[j] - bounds->mins[j]) * sign) - tw->radiusOffset.vec.v[j];
            d2 = (float)((float)(tw->extents.end.vec.v[j] - bounds->mins[j]) * sign) - tw->radiusOffset.vec.v[j];
            if ( (LODWORD(d1) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 1970, 0, "%s", "!IS_NAN(d1)") )
            {
                __debugbreak();
            }
            if ( (LODWORD(d2) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 1971, 0, "%s", "!IS_NAN(d2)") )
            {
                __debugbreak();
            }
            if ( d1 <= 0.0 )
            {
                if ( d2 > 0.0 )
                {
                    fraca = (float)(d1 * tw->extents.invDelta.vec.v[j]) * sign;
                    if ( enterFrac >= fraca )
                        return 0;
                    if ( (float)(fraca - leaveFrac) < 0.0 )
                        v5 = (float)(d1 * tw->extents.invDelta.vec.v[j]) * sign;
                    else
                        v5 = leaveFrac;
                    leaveFrac = v5;
                }
            }
            else
            {
                if ( d2 > 0.0 )
                    return 0;
                frac = (float)(d1 * tw->extents.invDelta.vec.v[j]) * sign;
                if ( frac >= leaveFrac )
                    return 0;
                if ( (float)(enterFrac - frac) < 0.0 )
                    v6 = (float)(d1 * tw->extents.invDelta.vec.v[j]) * sign;
                else
                    v6 = enterFrac;
                enterFrac = v6;
            }
        }
        if ( index )
            break;
        sign = 1.0f;
        bounds = (const cbrush_t *)brush->maxs;
    }
    i = brush->numsides;
    if ( i )
        side = brush->sides;
    if ( i < 0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2002, 0, "%s", "i >= 0") )
        __debugbreak();
    while ( i )
    {
        plane = side->plane;
        if ( (LODWORD(side->plane->dist) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        2007,
                        0,
                        "%s",
                        "!IS_NAN(plane->dist)") )
        {
            __debugbreak();
        }
        if ( (LODWORD(tw->radius) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        2008,
                        0,
                        "%s",
                        "!IS_NAN(tw->radius)") )
        {
            __debugbreak();
        }
        if ( ((LODWORD(plane->normal[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(plane->normal[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(plane->normal[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        2011,
                        0,
                        "%s",
                        "!IS_NAN((plane->normal)[0]) && !IS_NAN((plane->normal)[1]) && !IS_NAN((plane->normal)[2])") )
        {
            __debugbreak();
        }
        if ( (LODWORD(tw->offsetZ) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        2012,
                        0,
                        "%s",
                        "!IS_NAN(tw->offsetZ)") )
        {
            __debugbreak();
        }
        dist = (float)(plane->dist + tw->radius)
                 + fabs(plane->normal[2] * tw->offsetZ);
        if ( (LODWORD(dist) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2015, 0, "%s", "!IS_NAN(dist)") )
        {
            __debugbreak();
        }
        d1a = (float)((float)((float)(tw->extents.start.vec.v[0] * plane->normal[0])
                                                + (float)(tw->extents.start.vec.v[1] * plane->normal[1]))
                                + (float)(tw->extents.start.vec.v[2] * plane->normal[2]))
                - dist;
        d2a = (float)((float)((float)(tw->extents.end.vec.v[0] * plane->normal[0])
                                                + (float)(tw->extents.end.vec.v[1] * plane->normal[1]))
                                + (float)(tw->extents.end.vec.v[2] * plane->normal[2]))
                - dist;
        if ( (LODWORD(d1a) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2020, 0, "%s", "!IS_NAN(d1)") )
        {
            __debugbreak();
        }
        if ( (LODWORD(d2a) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2021, 0, "%s", "!IS_NAN(d2)") )
        {
            __debugbreak();
        }
        if ( d1a <= 0.0 )
        {
            if ( d2a > 0.0 )
            {
                deltaa = d1a - d2a;
                if ( (float)(d1a - d2a) >= 0.0
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2040, 0, "%s", "delta < 0") )
                {
                    __debugbreak();
                }
                if ( d1a > (float)(leaveFrac * deltaa) )
                {
                    leaveFrac = d1a / deltaa;
                    if ( enterFrac >= (float)(d1a / deltaa) )
                        return 0;
                }
            }
        }
        else
        {
            delta = d1a - d2a;
            iassert(!IS_NAN(delta));
            //if ( (COERCE_UNSIGNED_INT(d1a - d2a) & 0x7F800000) == 0x7F800000
            //    && !Assert_MyHandler(
            //                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
            //                2026,
            //                0,
            //                "%s",
            //                "!IS_NAN(delta)") )
            //{
            //    __debugbreak();
            //}
            if ( d2a > 0.0 )
                return 0;
            if ( delta <= 0.0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2029, 0, "%s", "delta > 0") )
            {
                __debugbreak();
            }
            if ( d1a > (float)(enterFrac * delta) )
            {
                enterFrac = d1a / delta;
                if ( (float)(d1a / delta) >= leaveFrac )
                    return 0;
            }
        }
        --i;
        ++side;
    }
    trace->cflags = brush->contents;
    trace->fraction = enterFrac;
    trace->sflags = 0;
    return brushNum + 1;
}

int __cdecl CM_SightTracePointThroughBrush(const TraceExtents *extents, const cbrush_t *brush)
{
    float v3; // [esp+0h] [ebp-CCh]
    float v4; // [esp+4h] [ebp-C8h]
    float d1; // [esp+90h] [ebp-3Ch]
    float d1a; // [esp+90h] [ebp-3Ch]
    cbrushside_t *side; // [esp+94h] [ebp-38h]
    int j; // [esp+98h] [ebp-34h]
    const cplane_s *plane; // [esp+9Ch] [ebp-30h]
    float enterFrac; // [esp+A0h] [ebp-2Ch]
    float delta; // [esp+A4h] [ebp-28h]
    float deltaa; // [esp+A4h] [ebp-28h]
    float frac; // [esp+A8h] [ebp-24h]
    float fraca; // [esp+A8h] [ebp-24h]
    float dist; // [esp+B0h] [ebp-1Ch]
    float sign; // [esp+B4h] [ebp-18h]
    float d2; // [esp+B8h] [ebp-14h]
    float d2a; // [esp+B8h] [ebp-14h]
    const cbrush_t *bounds; // [esp+BCh] [ebp-10h]
    int index; // [esp+C0h] [ebp-Ch]
    float leaveFrac; // [esp+C4h] [ebp-8h]
    signed int i; // [esp+C8h] [ebp-4h]

    side = 0;
    if ( ((extents->start.vec.u[0] & 0x7F800000) == 0x7F800000
         || (extents->start.vec.u[1] & 0x7F800000) == 0x7F800000
         || (extents->start.vec.u[2] & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                    2074,
                    0,
                    "%s",
                    "!IS_NAN((extents->start)[0]) && !IS_NAN((extents->start)[1]) && !IS_NAN((extents->start)[2])") )
    {
        __debugbreak();
    }
    if ( ((extents->end.vec.u[0] & 0x7F800000) == 0x7F800000
         || (extents->end.vec.u[1] & 0x7F800000) == 0x7F800000
         || (extents->end.vec.u[2] & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                    2075,
                    0,
                    "%s",
                    "!IS_NAN((extents->end)[0]) && !IS_NAN((extents->end)[1]) && !IS_NAN((extents->end)[2])") )
    {
        __debugbreak();
    }
    enterFrac = 0.0f;
    leaveFrac = 1.0f;
    sign = -1.0f;
    bounds = brush;
    for ( index = 0; ; index = 1 )
    {
        if ( ((LODWORD(bounds->mins[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(bounds->mins[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(bounds->mins[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        2089,
                        0,
                        "%s",
                        "!IS_NAN((bounds)[0]) && !IS_NAN((bounds)[1]) && !IS_NAN((bounds)[2])") )
        {
            __debugbreak();
        }
        for ( j = 0; j < 3; ++j )
        {
            d1 = (float)(extents->start.vec.v[j] - bounds->mins[j]) * sign;
            d2 = (float)(extents->end.vec.v[j] - bounds->mins[j]) * sign;
            if ( (LODWORD(d1) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2095, 0, "%s", "!IS_NAN(d1)") )
            {
                __debugbreak();
            }
            if ( (LODWORD(d2) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2096, 0, "%s", "!IS_NAN(d2)") )
            {
                __debugbreak();
            }
            if ( d1 <= 0.0 )
            {
                if ( d2 > 0.0 )
                {
                    fraca = (float)(d1 * extents->invDelta.vec.v[j]) * sign;
                    if ( enterFrac >= fraca )
                        return 0;
                    if ( (float)(fraca - leaveFrac) < 0.0 )
                        v3 = (float)(d1 * extents->invDelta.vec.v[j]) * sign;
                    else
                        v3 = leaveFrac;
                    leaveFrac = v3;
                }
            }
            else
            {
                if ( d2 > 0.0 )
                    return 0;
                frac = (float)(d1 * extents->invDelta.vec.v[j]) * sign;
                if ( frac >= leaveFrac )
                    return 0;
                if ( (float)(enterFrac - frac) < 0.0 )
                    v4 = (float)(d1 * extents->invDelta.vec.v[j]) * sign;
                else
                    v4 = enterFrac;
                enterFrac = v4;
            }
        }
        if ( index )
            break;
        sign = 1.0f;
        bounds = (const cbrush_t *)brush->maxs;
    }
    i = brush->numsides;
    if ( i )
        side = brush->sides;
    if ( i < 0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2128, 0, "%s", "i >= 0") )
        __debugbreak();
    while ( i )
    {
        plane = side->plane;
        if ( (LODWORD(side->plane->dist) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        2133,
                        0,
                        "%s",
                        "!IS_NAN(plane->dist)") )
        {
            __debugbreak();
        }
        if ( ((LODWORD(plane->normal[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(plane->normal[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(plane->normal[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        2136,
                        0,
                        "%s",
                        "!IS_NAN((plane->normal)[0]) && !IS_NAN((plane->normal)[1]) && !IS_NAN((plane->normal)[2])") )
        {
            __debugbreak();
        }
        dist = plane->dist;
        if ( (LODWORD(dist) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2138, 0, "%s", "!IS_NAN(dist)") )
        {
            __debugbreak();
        }
        d1a = (float)((float)((float)(extents->start.vec.v[0] * plane->normal[0])
                                                + (float)(extents->start.vec.v[1] * plane->normal[1]))
                                + (float)(extents->start.vec.v[2] * plane->normal[2]))
                - dist;
        d2a = (float)((float)((float)(extents->end.vec.v[0] * plane->normal[0])
                                                + (float)(extents->end.vec.v[1] * plane->normal[1]))
                                + (float)(extents->end.vec.v[2] * plane->normal[2]))
                - dist;
        if ( (LODWORD(d1a) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2143, 0, "%s", "!IS_NAN(d1)") )
        {
            __debugbreak();
        }
        if ( (LODWORD(d2a) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2144, 0, "%s", "!IS_NAN(d2)") )
        {
            __debugbreak();
        }
        if ( d1a <= 0.0 )
        {
            if ( d2a > 0.0 )
            {
                deltaa = d1a - d2a;
                if ( (float)(d1a - d2a) >= 0.0
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2163, 0, "%s", "delta < 0") )
                {
                    __debugbreak();
                }
                if ( d1a > (float)(leaveFrac * deltaa) )
                {
                    leaveFrac = d1a / deltaa;
                    if ( enterFrac >= (float)(d1a / deltaa) )
                        return 0;
                }
            }
        }
        else
        {
            delta = d1a - d2a;
            iassert(!IS_NAN(delta));
            //if ( (COERCE_UNSIGNED_INT(d1a - d2a) & 0x7F800000) == 0x7F800000
            //    && !Assert_MyHandler(
            //                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
            //                2149,
            //                0,
            //                "%s",
            //                "!IS_NAN(delta)") )
            //{
            //    __debugbreak();
            //}
            if ( d2a > 0.0 )
                return 0;
            if ( delta <= 0.0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2152, 0, "%s", "delta > 0") )
            {
                __debugbreak();
            }
            if ( d1a > (float)(enterFrac * delta) )
            {
                enterFrac = d1a / delta;
                if ( (float)(d1a / delta) >= leaveFrac )
                    return 0;
            }
        }
        --i;
        ++side;
    }
    return 1;
}

int __cdecl CM_SightTraceThroughTree(const traceWork_t *tw, int num, const float *p1_, const float *p2, trace_t *trace)
{
    float v6; // [esp+8h] [ebp-7Ch]
    float v7; // [esp+Ch] [ebp-78h]
    float v8; // [esp+14h] [ebp-70h]
    float v9; // [esp+18h] [ebp-6Ch]
    float v10; // [esp+1Ch] [ebp-68h]
    const cNode_t *node; // [esp+34h] [ebp-50h]
    bool side; // [esp+38h] [ebp-4Ch]
    float diff; // [esp+3Ch] [ebp-48h]
    const cplane_s *plane; // [esp+40h] [ebp-44h]
    float t1; // [esp+48h] [ebp-3Ch]
    float frac; // [esp+4Ch] [ebp-38h]
    float p1[3]; // [esp+54h] [ebp-30h] BYREF
    float offset; // [esp+60h] [ebp-24h]
    float t2; // [esp+64h] [ebp-20h]
    float frac2; // [esp+68h] [ebp-1Ch]
    float absDiff; // [esp+6Ch] [ebp-18h]
    float invDist; // [esp+70h] [ebp-14h]
    int hitNum; // [esp+74h] [ebp-10h]
    float mid[3]; // [esp+78h] [ebp-Ch] BYREF

    p1[0] = *p1_;
    p1[1] = p1_[1];
    p1[2] = p1_[2];
    while ( 1 )
    {
        while ( 1 )
        {
            while ( 1 )
            {
                if ( num < 0 )
                    return CM_SightTraceThroughLeaf(tw, &cm.leafs[-1 - num], trace);
                node = &cm.nodes[num];
                plane = node->plane;
                if ( node->plane->type >= 3u )
                {
                    t1 = (float)((float)((float)(plane->normal[0] * p1[0]) + (float)(plane->normal[1] * p1[1]))
                                         + (float)(plane->normal[2] * p1[2]))
                         - plane->dist;
                    t2 = (float)((float)((float)(plane->normal[0] * *p2) + (float)(plane->normal[1] * p2[1]))
                                         + (float)(plane->normal[2] * p2[2]))
                         - plane->dist;
                    offset = tw->isPoint ? 0.125f : tw->boundingRadius + 0.125;
                }
                else
                {
                    t1 = p1[plane->type] - plane->dist;
                    t2 = p2[plane->type] - plane->dist;
                    offset = tw->size.vec.v[plane->type] + 0.125;
                }
                v10 = (float)(t2 - t1) < 0.0 ? t2 : t1;
                if ( v10 < offset )
                    break;
                num = node->children[0];
            }
            v9 = (float)(t1 - t2) < 0.0 ? t2 : t1;
            //if ( COERCE_FLOAT(LODWORD(offset) ^ _mask__NegFloat_) < v9 )
            if ( -offset < v9 )
                break;
            num = node->children[1];
        }
        diff = t2 - t1;
        absDiff = fabs(t2 - t1);
        if ( absDiff <= 0.00000047683716 )
        {
            side = 0;
            frac = 1.0f;
            frac2 = 0.0f;
        }
        else
        {
            if ( diff < 0.0 )
                v8 = t1;
            else
                v8 = -t1;
            invDist = 1.0 / absDiff;
            frac2 = (float)(v8 - offset) * (float)(1.0 / absDiff);
            frac = (float)(v8 + offset) * (float)(1.0 / absDiff);
            side = diff >= 0.0;
        }
        if ( frac < 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2655, 0, "%s", "frac >= 0") )
        {
            __debugbreak();
        }
        v7 = (float)(1.0 - frac) < 0.0 ? 1.0f : frac;
        mid[0] = (float)((float)(*p2 - p1[0]) * v7) + p1[0];
        mid[1] = (float)((float)(p2[1] - p1[1]) * v7) + p1[1];
        mid[2] = (float)((float)(p2[2] - p1[2]) * v7) + p1[2];
        hitNum = CM_SightTraceThroughTree(tw, node->children[side], p1, mid, trace);
        if ( hitNum )
            break;
        if ( frac2 > 1.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        2667,
                        0,
                        "%s\n\t(frac2) = %g",
                        "(frac2 <= 1.0f)",
                        frac2) )
        {
            __debugbreak();
        }
        if ( (float)(frac2 - 0.0) < 0.0 )
            v6 = 0.0f;
        else
            v6 = frac2;
        frac2 = v6;
        p1[0] = (float)((float)(*p2 - p1[0]) * v6) + p1[0];
        p1[1] = (float)((float)(p2[1] - p1[1]) * v6) + p1[1];
        p1[2] = (float)((float)(p2[2] - p1[2]) * v6) + p1[2];
        num = node->children[!side];
    }
    return hitNum;
}

int __cdecl CM_SightTraceThroughLeaf(const traceWork_t *tw, const cLeaf_s *leaf, trace_t *trace)
{
    const char *v4; // [esp+0h] [ebp-18h]
    int hitnum; // [esp+10h] [ebp-8h]
    int k; // [esp+14h] [ebp-4h]

    if ( (tw->contents & leaf->brushContents) != 0 )
    {
        hitnum = CM_SightTraceThroughLeafBrushNode(tw, leaf, trace);
        if ( hitnum )
            return hitnum;
    }
    if ( trace->fraction != 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                    2337,
                    0,
                    "%s",
                    "trace->fraction == 1.0f") )
    {
        __debugbreak();
    }
    if ( (tw->contents & leaf->terrainContents) != 0 )
    {
        for ( k = 0; k < leaf->collAabbCount; ++k )
        {
            v4 = (const char *)&cm.aabbTrees[k + leaf->firstCollAabbIndex];
            _mm_prefetch(v4, 1);
            CM_SightTraceThroughAabbTree(tw, (const CollisionAabbTree *)v4, trace);
            if ( trace->fraction != 1.0 )
                return leaf->firstCollAabbIndex + k + cm.numBrushes + 1;
        }
    }
    return 0;
}

int __cdecl CM_SightTraceThroughLeafBrushNode(const traceWork_t *tw, const cLeaf_s *leaf, trace_t *trace)
{
    float absmin[3]; // [esp+1Ch] [ebp-20h] BYREF
    float absmax[3]; // [esp+30h] [ebp-Ch] BYREF

    if ( !leaf->leafBrushNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                    2312,
                    0,
                    "%s",
                    "leaf->leafBrushNode") )
    {
        __debugbreak();
    }
    absmin[0] = leaf->mins[0] - tw->size.vec.v[0];
    absmin[1] = leaf->mins[1] - tw->size.vec.v[1];
    absmin[2] = leaf->mins[2] - tw->size.vec.v[2];
    absmax[0] = leaf->maxs[0] + tw->size.vec.v[0];
    absmax[1] = leaf->maxs[1] + tw->size.vec.v[1];
    absmax[2] = leaf->maxs[2] + tw->size.vec.v[2];
    if ( intersect_extents_aabb(&tw->extents, absmin, absmax, 1.0) )
        return CM_SightTraceThroughLeafBrushNode_r(
                         tw,
                         &cm.leafbrushNodes[leaf->leafBrushNode],
                         tw->extents.start.vec.v,
                         tw->extents.end.vec.v,
                         trace);
    else
        return 0;
}

int __cdecl CM_SightTraceThroughLeafBrushNode_r(
                const traceWork_t *tw,
                cLeafBrushNode_s *remoteNode,
                const float *p1_,
                const float *p2,
                trace_t *trace)
{
    float v6; // [esp+8h] [ebp-A0h]
    float v7; // [esp+Ch] [ebp-9Ch]
    float v8; // [esp+14h] [ebp-94h]
    float v9; // [esp+18h] [ebp-90h]
    float v10; // [esp+1Ch] [ebp-8Ch]
    unsigned __int16 *brushes; // [esp+48h] [ebp-60h]
    bool side; // [esp+54h] [ebp-54h]
    float diff; // [esp+58h] [ebp-50h]
    float t1; // [esp+60h] [ebp-48h]
    float frac; // [esp+64h] [ebp-44h]
    int k; // [esp+68h] [ebp-40h]
    float p1[3]; // [esp+6Ch] [ebp-3Ch] BYREF
    float offset; // [esp+78h] [ebp-30h]
    float tmax; // [esp+7Ch] [ebp-2Ch]
    float t2; // [esp+80h] [ebp-28h]
    float frac2; // [esp+84h] [ebp-24h]
    float absDiff; // [esp+88h] [ebp-20h]
    float invDist; // [esp+8Ch] [ebp-1Ch]
    int hitNum; // [esp+90h] [ebp-18h]
    float tmin; // [esp+94h] [ebp-14h]
    int brushnum; // [esp+98h] [ebp-10h]
    float mid[3]; // [esp+9Ch] [ebp-Ch] BYREF

    if ( !remoteNode
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2204, 0, "%s", "PPU(remoteNode)") )
    {
        __debugbreak();
    }
    p1[0] = *p1_;
    p1[1] = p1_[1];
    p1[2] = p1_[2];
    while ( 1 )
    {
        if ( (tw->contents & remoteNode->contents) == 0 )
            return 0;
        if ( remoteNode->leafBrushCount )
            break;
LABEL_19:
        t1 = p1[remoteNode->axis] - remoteNode->data.children.dist;
        t2 = p2[remoteNode->axis] - remoteNode->data.children.dist;
        offset = (float)(tw->size.vec.v[remoteNode->axis] + 0.125) - remoteNode->data.children.range;
        if ( (float)(t1 - t2) < 0.0 )
            v10 = t2;
        else
            v10 = t1;
        tmax = v10;
        if ( (float)(t2 - t1) < 0.0 )
            v9 = t2;
        else
            v9 = t1;
        tmin = v9;
        if ( v9 < offset )
        {
            //if ( COERCE_FLOAT(LODWORD(offset) ^ _mask__NegFloat_) < tmax )
            if ( -offset < tmax )
            {
                diff = t2 - t1;
                absDiff = fabs(t2 - t1);
                if ( absDiff <= 0.00000047683716 )
                {
                    side = 0;
                    frac = 1.0f;
                    frac2 = 0.0f;
                }
                else
                {
                    if ( diff < 0.0 )
                        v8 = t1;
                    else
                        v8 = -t1;
                    invDist = 1.0 / absDiff;
                    frac2 = (float)(v8 - offset) * (float)(1.0 / absDiff);
                    frac = (float)(v8 + offset) * (float)(1.0 / absDiff);
                    side = diff >= 0.0;
                }
                if ( frac < 0.0
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2283, 0, "%s", "frac >= 0") )
                {
                    __debugbreak();
                }
                if ( (float)(1.0 - frac) < 0.0 )
                    v7 = 1.0f;
                else
                    v7 = frac;
                mid[0] = (float)((float)(*p2 - p1[0]) * v7) + p1[0];
                mid[1] = (float)((float)(p2[1] - p1[1]) * v7) + p1[1];
                mid[2] = (float)((float)(p2[2] - p1[2]) * v7) + p1[2];
                hitNum = CM_SightTraceThroughLeafBrushNode_r(
                                     tw,
                                     &remoteNode[remoteNode->data.children.childOffset[side]],
                                     p1,
                                     mid,
                                     trace);
                if ( hitNum )
                    return hitNum;
                if ( frac2 > 1.0001
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                                2294,
                                0,
                                "%s\n\t(frac2) = %g",
                                "(frac2 <= 1.0001f)",
                                frac2) )
                {
                    __debugbreak();
                }
                if ( (float)(frac2 - 0.0) < 0.0 )
                    v6 = 0.0f;
                else
                    v6 = frac2;
                frac2 = v6;
                p1[0] = (float)((float)(*p2 - p1[0]) * v6) + p1[0];
                p1[1] = (float)((float)(p2[1] - p1[1]) * v6) + p1[1];
                p1[2] = (float)((float)(p2[2] - p1[2]) * v6) + p1[2];
                remoteNode += remoteNode->data.children.childOffset[1 - side];
            }
            else
            {
                remoteNode += remoteNode->data.children.childOffset[1];
            }
        }
        else
        {
            if ( -offset >= tmax )
                return 0;
            remoteNode += remoteNode->data.children.childOffset[0];
        }
    }
    if ( remoteNode->leafBrushCount <= 0 )
    {
        hitNum = CM_SightTraceThroughLeafBrushNode_r(tw, remoteNode + 1, p1, p2, trace);
        if ( hitNum )
            return hitNum;
        goto LABEL_19;
    }
    brushes = remoteNode->data.leaf.brushes;
    for ( k = 0; k < remoteNode->leafBrushCount; ++k )
    {
        brushnum = brushes[k];
        if ( (tw->contents & cm.brushes[brushnum].contents) != 0 )
        {
            hitNum = CM_SightTraceThroughBrush(tw, &cm.brushes[brushnum], brushnum, trace);
            if ( hitNum )
                return hitNum;
        }
    }
    return 0;
}

int __cdecl CM_BoxSightTrace(
                int oldHitNum,
                const float *start,
                const float *end,
                const float *mins,
                const float *maxs,
                unsigned int model,
                int brushmask)
{
    float v7; // xmm0_4
    unsigned int v9; // [esp+8h] [ebp-184h]
    cmodel_t *cmod; // [esp+48h] [ebp-144h]
    traceWork_t tw; // [esp+4Ch] [ebp-140h] BYREF
    float offset[3]; // [esp+130h] [ebp-5Ch]
    trace_t trace; // [esp+13Ch] [ebp-50h] BYREF
    int i; // [esp+184h] [ebp-8h]
    int hitNum; // [esp+188h] [ebp-4h]
    int oldHitNuma; // [esp+194h] [ebp+8h]

    //traceWork_t::traceWork_t(&tw);
    if ( !cm.numNodes
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2687, 0, "%s", "cm.numNodes") )
    {
        __debugbreak();
    }
    if ( !mins && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2688, 0, "%s", "mins") )
        __debugbreak();
    if ( !maxs && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2689, 0, "%s", "maxs") )
        __debugbreak();
    cmod = CM_ClipHandleToModel(model);
    trace.fraction = 1.0f;
    trace.startsolid = 0;
    trace.allsolid = 0;
    tw.contents = brushmask;
    for ( i = 0; i < 3; ++i )
    {
        if ( maxs[i] < mins[i]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        2706,
                        0,
                        "%s",
                        "maxs[i] >= mins[i]") )
        {
            __debugbreak();
        }
        offset[i] = (float)(mins[i] + maxs[i]) * 0.5;
        tw.size.vec.v[i] = maxs[i] - offset[i];
        tw.extents.start.vec.v[i] = start[i] + offset[i];
        tw.extents.end.vec.v[i] = end[i] + offset[i];
        tw.midpoint.vec.v[i] = (float)(tw.extents.start.vec.v[i] + tw.extents.end.vec.v[i]) * 0.5;
        tw.delta.vec.v[i] = tw.extents.end.vec.v[i] - tw.extents.start.vec.v[i];
        tw.halfDelta.vec.v[i] = 0.5 * tw.delta.vec.v[i];
        tw.halfDeltaAbs.vec.v[i] = I_fabs(tw.halfDelta.vec.v[i]);
    }
    CM_CalcTraceExtents(&tw.extents);
    tw.deltaLenSq = (float)((float)(tw.delta.vec.v[0] * tw.delta.vec.v[0]) + (float)(tw.delta.vec.v[1] * tw.delta.vec.v[1]))
                                + (float)(tw.delta.vec.v[2] * tw.delta.vec.v[2]);
    tw.deltaLen = sqrtf(tw.deltaLenSq);
    if ( tw.size.vec.v[0] <= tw.size.vec.v[2] )
        v9 = tw.size.vec.u[0];
    else
        v9 = tw.size.vec.u[2];
    LODWORD(tw.radius) = v9;
    tw.boundingRadius = Vec3Length(tw.size.vec.v);
    tw.offsetZ = tw.size.vec.v[2] - tw.radius;
    for ( i = 0; i < 2; ++i )
    {
        if ( tw.extents.end.vec.v[i] <= tw.extents.start.vec.v[i] )
        {
            tw.bounds[0].vec.v[i] = tw.extents.end.vec.v[i] - tw.radius;
            v7 = tw.extents.start.vec.v[i] + tw.radius;
        }
        else
        {
            tw.bounds[0].vec.v[i] = tw.extents.start.vec.v[i] - tw.radius;
            v7 = tw.extents.end.vec.v[i] + tw.radius;
        }
        tw.bounds[1].vec.v[i] = v7;
    }
    if ( tw.offsetZ < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2740, 0, "%s", "tw.offsetZ >= 0") )
    {
        __debugbreak();
    }
    if ( tw.extents.end.vec.v[2] <= tw.extents.start.vec.v[2] )
    {
        tw.bounds[0].vec.v[2] = (float)(tw.extents.end.vec.v[2] - tw.offsetZ) - tw.radius;
        tw.bounds[1].vec.v[2] = (float)(tw.extents.start.vec.v[2] + tw.offsetZ) + tw.radius;
    }
    else
    {
        tw.bounds[0].vec.v[2] = (float)(tw.extents.start.vec.v[2] - tw.offsetZ) - tw.radius;
        tw.bounds[1].vec.v[2] = (float)(tw.extents.end.vec.v[2] + tw.offsetZ) + tw.radius;
    }
    CM_SetAxialCullOnly(&tw);
    if ( tw.size.vec.v[0] < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2756, 0, "%s", "tw.size[0] >= 0") )
    {
        __debugbreak();
    }
    if ( tw.size.vec.v[1] < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2757, 0, "%s", "tw.size[1] >= 0") )
    {
        __debugbreak();
    }
    if ( tw.size.vec.v[2] < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2758, 0, "%s", "tw.size[2] >= 0") )
    {
        __debugbreak();
    }
    tw.isPoint = (float)((float)(tw.size.vec.v[0] + tw.size.vec.v[1]) + tw.size.vec.v[2]) == 0.0;
    tw.doublesided = 0;
    if ( tw.offsetZ < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2763, 0, "%s", "tw.offsetZ >= 0") )
    {
        __debugbreak();
    }
    tw.radiusOffset.vec.u[0] = LODWORD(tw.radius);
    tw.radiusOffset.vec.u[1] = LODWORD(tw.radius);
    tw.radiusOffset.vec.v[2] = tw.radius + tw.offsetZ;
    CM_GetTraceThreadInfo(&tw.threadInfo);
    if ( model )
    {
        if ( model == 0x3FFF )
        {
            if ( (tw.contents & tw.threadInfo.box_brush->contents) != 0 )
                return CM_SightTraceCapsuleThroughCapsule(&tw, &trace);
            else
                return 0;
        }
        else if ( model == 0x4000 )
        {
            return CM_SightTraceThroughTempBrush(&tw, &trace);
        }
        else
        {
            return CM_SightTraceThroughLeaf(&tw, &cmod->leaf, &trace);
        }
    }
    else
    {
        hitNum = 0;
        if ( oldHitNum > 0 )
        {
            oldHitNuma = oldHitNum - 1;
            if ( oldHitNuma < cm.numBrushes )
                hitNum = CM_SightTraceThroughBrush(&tw, &cm.brushes[oldHitNuma], oldHitNuma, &trace);
        }
        if ( !hitNum )
            return CM_SightTraceThroughTree(&tw, 0, tw.extents.start.vec.v, tw.extents.end.vec.v, &trace);
    }
    return hitNum;
}

int __cdecl CM_SightTraceThroughTempBrush(const traceWork_t *tw, trace_t *trace)
{
    const cbrush_t **info; // [esp+Ch] [ebp-14h]
    unsigned int i; // [esp+14h] [ebp-Ch]
    PhysGeomList *geoms; // [esp+1Ch] [ebp-4h]

    geoms = *tw->threadInfo.geoms;
    for ( i = 0; i < geoms->count; ++i )
    {
        info = (const cbrush_t **)&geoms->geoms[i];
        if ( *info && (tw->contents & (*info)->contents) != 0 && CM_SightTraceThroughBrush(tw, *info, 1, trace) )
            return -1;
    }
    return 0;
}

int __cdecl CM_SightTraceCapsuleThroughCapsule(const traceWork_t *tw, trace_t *trace)
{
    float v3; // [esp+10h] [ebp-B4h]
    float endtop[3]; // [esp+44h] [ebp-80h] BYREF
    float starttop[3]; // [esp+50h] [ebp-74h] BYREF
    float halfwidth; // [esp+5Ch] [ebp-68h]
    float top[3]; // [esp+60h] [ebp-64h] BYREF
    float offs; // [esp+6Ch] [ebp-58h]
    float halfheight; // [esp+70h] [ebp-54h]
    float symetricSize[2][3]; // [esp+74h] [ebp-50h]
    float radius; // [esp+8Ch] [ebp-38h]
    float offset[3]; // [esp+90h] [ebp-34h] BYREF
    float endbottom[3]; // [esp+9Ch] [ebp-28h] BYREF
    float startbottom[3]; // [esp+A8h] [ebp-1Ch] BYREF
    int i; // [esp+B4h] [ebp-10h]
    float bottom[3]; // [esp+B8h] [ebp-Ch] BYREF

    if ( tw->bounds[0].vec.v[0] > (float)(tw->threadInfo.box_model->maxs[0] + 1.0)
        || tw->bounds[0].vec.v[1] > (float)(tw->threadInfo.box_model->maxs[1] + 1.0)
        || tw->bounds[0].vec.v[2] > (float)(tw->threadInfo.box_model->maxs[2] + 1.0)
        || (float)(tw->threadInfo.box_model->mins[0] - 1.0) > tw->bounds[1].vec.v[0]
        || (float)(tw->threadInfo.box_model->mins[1] - 1.0) > tw->bounds[1].vec.v[1]
        || (float)(tw->threadInfo.box_model->mins[2] - 1.0) > tw->bounds[1].vec.v[2] )
    {
        return 0;
    }
    *(_QWORD *)starttop = *(_QWORD *)tw->extents.start.vec.v;
    starttop[2] = tw->extents.start.vec.v[2] + tw->offsetZ;
    *(_QWORD *)startbottom = *(_QWORD *)tw->extents.start.vec.v;
    startbottom[2] = tw->extents.start.vec.v[2] - tw->offsetZ;
    *(_QWORD *)endtop = *(_QWORD *)tw->extents.end.vec.v;
    endtop[2] = tw->extents.end.vec.v[2] + tw->offsetZ;
    *(_QWORD *)endbottom = *(_QWORD *)tw->extents.end.vec.v;
    endbottom[2] = tw->extents.end.vec.v[2] - tw->offsetZ;
    for ( i = 0; i < 3; ++i )
    {
        offset[i] = (float)(tw->threadInfo.box_model->mins[i] + tw->threadInfo.box_model->maxs[i]) * 0.5;
        symetricSize[0][i] = tw->threadInfo.box_model->mins[i] - offset[i];
        symetricSize[1][i] = tw->threadInfo.box_model->maxs[i] - offset[i];
    }
    halfwidth = symetricSize[1][0];
    halfheight = symetricSize[1][2];
    if ( symetricSize[1][0] <= symetricSize[1][2] )
        v3 = halfwidth;
    else
        v3 = halfheight;
    radius = v3;
    offs = halfheight - v3;
    top[0] = offset[0];
    top[1] = offset[1];
    top[2] = offset[2] + (float)(halfheight - v3);
    bottom[0] = offset[0];
    bottom[1] = offset[1];
    bottom[2] = offset[2] - (float)(halfheight - v3);
    if ( startbottom[2] <= top[2] )
    {
        if ( bottom[2] > starttop[2] )
        {
            if ( !CM_SightTraceSphereThroughSphere(tw, starttop, endtop, bottom, radius, trace) )
                return -1;
            if ( tw->delta.vec.v[2] <= 0.0 )
                return 0;
        }
    }
    else
    {
        if ( !CM_SightTraceSphereThroughSphere(tw, startbottom, endbottom, top, radius, trace) )
            return -1;
        if ( tw->delta.vec.v[2] >= 0.0 )
            return 0;
    }
    if ( !CM_SightTraceCylinderThroughCylinder(tw, offset, offs, radius, trace) )
        return -1;
    if ( endbottom[2] <= top[2] )
    {
        if ( bottom[2] > endtop[2]
            && starttop[2] >= bottom[2]
            && !CM_SightTraceSphereThroughSphere(tw, starttop, endtop, bottom, radius, trace) )
        {
            return -1;
        }
    }
    else if ( top[2] >= startbottom[2]
                 && !CM_SightTraceSphereThroughSphere(tw, startbottom, endbottom, top, radius, trace) )
    {
        return -1;
    }
    return 0;
}

bool __cdecl CM_SightTraceSphereThroughSphere(
                const traceWork_t *tw,
                const float *vStart,
                const float *vEnd,
                const float *vStationary,
                float radius,
                trace_t *trace)
{
    float fDiscriminant; // [esp+10h] [ebp-34h]
    float fC; // [esp+1Ch] [ebp-28h]
    float fB; // [esp+20h] [ebp-24h]
    float fA; // [esp+24h] [ebp-20h]
    float vNormal[3]; // [esp+28h] [ebp-1Ch] BYREF
    float vDelta[3]; // [esp+34h] [ebp-10h] BYREF
    float fDeltaLen; // [esp+40h] [ebp-4h]

    vDelta[0] = *vStart - *vStationary;
    vDelta[1] = vStart[1] - vStationary[1];
    vDelta[2] = vStart[2] - vStationary[2];
    fC = (float)((float)((float)(vDelta[0] * vDelta[0]) + (float)(vDelta[1] * vDelta[1])) + (float)(vDelta[2] * vDelta[2]))
         - (float)((float)(radius + tw->radius) * (float)(radius + tw->radius));
    if ( fC <= 0.0 )
        return 0;
    fB = (float)((float)(tw->delta.vec.v[0] * vDelta[0]) + (float)(tw->delta.vec.v[1] * vDelta[1]))
         + (float)(tw->delta.vec.v[2] * vDelta[2]);
    if ( fB >= 0.0 )
        return 1;
    fA = tw->deltaLenSq;
    fDiscriminant = (float)(fB * fB) - (float)(fA * fC);
    if ( fDiscriminant < 0.0 )
        return 1;
    fDeltaLen = Vec3NormalizeTo(vDelta, vNormal);
    //return (float)((float)((float)(COERCE_FLOAT(LODWORD(fB) ^ _mask__NegFloat_) - sqrtf(fDiscriminant)) / fA) + (float)((float)(fB * 0.125) / fDeltaLen)) >= trace->fraction;
    return (float)((float)((float)(-fB - sqrtf(fDiscriminant)) / fA) + (float)((float)(fB * 0.125) / fDeltaLen)) >= trace->fraction;
}

bool __cdecl CM_SightTraceCylinderThroughCylinder(
                const traceWork_t *tw,
                const float *vStationary,
                float fStationaryHalfHeight,
                float radius,
                trace_t *trace)
{
    float fDiscriminant; // [esp+2Ch] [ebp-40h]
    float fEntry; // [esp+30h] [ebp-3Ch]
    float fHitHeight; // [esp+38h] [ebp-34h]
    float fEpsilon; // [esp+3Ch] [ebp-30h]
    float fTotalHeight; // [esp+40h] [ebp-2Ch]
    float fTotalHeighta; // [esp+40h] [ebp-2Ch]
    float fC; // [esp+44h] [ebp-28h]
    float fB; // [esp+48h] [ebp-24h]
    float fA; // [esp+4Ch] [ebp-20h]
    float vNormal[3]; // [esp+50h] [ebp-1Ch] BYREF
    float vDelta[3]; // [esp+5Ch] [ebp-10h] BYREF
    float fDeltaLen; // [esp+68h] [ebp-4h]

    vDelta[0] = tw->extents.start.vec.v[0] - *vStationary;
    vDelta[1] = tw->extents.start.vec.v[1] - vStationary[1];
    vDelta[2] = tw->extents.start.vec.v[2] - vStationary[2];
    fC = (float)((float)(vDelta[0] * vDelta[0]) + (float)(vDelta[1] * vDelta[1]))
         - (float)((float)(radius + tw->radius) * (float)(radius + tw->radius));
    if ( fC > 0.0 )
    {
        fB = (float)(tw->delta.vec.v[0] * vDelta[0]) + (float)(tw->delta.vec.v[1] * vDelta[1]);
        if ( fB < 0.0 )
        {
            fA = (float)(tw->delta.vec.v[0] * tw->delta.vec.v[0]) + (float)(tw->delta.vec.v[1] * tw->delta.vec.v[1]);
            fDiscriminant = (float)(fB * fB) - (float)(fA * fC);
            if ( fDiscriminant >= 0.0 )
            {
                vDelta[2] = 0.0f;
                fDeltaLen = Vec3NormalizeTo(vDelta, vNormal);
                fEpsilon = (float)(fB * 0.125) / fDeltaLen;
                //fEntry = (float)((float)(COERCE_FLOAT(LODWORD(fB) ^ _mask__NegFloat_) - sqrtf(fDiscriminant)) / fA) + fEpsilon;
                fEntry = (float)((float)(-fB - sqrtf(fDiscriminant)) / fA) + fEpsilon;
                if ( fEntry < trace->fraction )
                {
                    fTotalHeighta = (float)(tw->size.vec.v[2] - tw->radius) + fStationaryHalfHeight;
                    fHitHeight = (float)((float)((float)(fEntry - fEpsilon) * tw->delta.vec.v[2]) + tw->extents.start.vec.v[2])
                                         - vStationary[2];
                    if ( fTotalHeighta < 0.0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                                    2455,
                                    0,
                                    "%s",
                                    "fTotalHeight >= 0") )
                    {
                        __debugbreak();
                    }
                    return fabs(fHitHeight) > fTotalHeighta;
                }
                else
                {
                    return 1;
                }
            }
            else
            {
                return 1;
            }
        }
        else
        {
            return 1;
        }
    }
    else
    {
        fTotalHeight = (float)(tw->size.vec.v[2] - tw->radius) + fStationaryHalfHeight;
        if ( fTotalHeight < 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        2431,
                        0,
                        "%s",
                        "fTotalHeight >= 0") )
        {
            __debugbreak();
        }
        return fabs(vDelta[2]) > fTotalHeight;
    }
}

int __cdecl CM_SightTracePoint(int oldHitNum, const float *start, const float *end, col_context_t *context)
{
    dmaterial_t *materialInfo; // [esp+4Ch] [ebp-154h]
    const CollisionAabbTree *tree; // [esp+50h] [ebp-150h]
    int j; // [esp+54h] [ebp-14Ch]
    const col_prim_t *prim; // [esp+58h] [ebp-148h]
    traceWork_t tw; // [esp+68h] [ebp-138h] BYREF
    trace_t trace; // [esp+14Ch] [ebp-54h] BYREF
    int i; // [esp+188h] [ebp-18h]
    float mn[3]; // [esp+18Ch] [ebp-14h]
    int hitNum; // [esp+198h] [ebp-8h]
    int brushmask; // [esp+19Ch] [ebp-4h]
    int oldHitNuma; // [esp+1A8h] [ebp+8h]

    //traceWork_t::traceWork_t(&tw);
    hitNum = 0;
    brushmask = context->mask;
    *(_QWORD *)tw.extents.start.vec.v = *(_QWORD *)start;
    tw.extents.start.vec.v[2] = start[2];
    *(_QWORD *)tw.extents.end.vec.v = *(_QWORD *)end;
    tw.extents.end.vec.v[2] = end[2];
    CM_CalcTraceExtents(&tw.extents);
    if ( oldHitNum > 0 )
    {
        oldHitNuma = oldHitNum - 1;
        if ( oldHitNuma < cm.numBrushes )
        {
            if ( CM_SightTracePointThroughBrush(&tw.extents, &cm.brushes[oldHitNuma]) )
                return oldHitNuma + 1;
        }
    }
    trace.fraction = 1.0f;
    trace.startsolid = 0;
    trace.allsolid = 0;
    for ( i = 0; i < 3; ++i )
    {
        tw.size.vec.u[i] = 0;
        tw.midpoint.vec.v[i] = (float)(tw.extents.start.vec.v[i] + tw.extents.end.vec.v[i]) * 0.5;
        tw.delta.vec.v[i] = tw.extents.end.vec.v[i] - tw.extents.start.vec.v[i];
        tw.halfDelta.vec.v[i] = 0.5 * tw.delta.vec.v[i];
        tw.halfDeltaAbs.vec.v[i] = I_fabs(tw.halfDelta.vec.v[i]);
    }
    tw.contents = brushmask;
    tw.deltaLenSq = (float)((float)(tw.delta.vec.v[0] * tw.delta.vec.v[0]) + (float)(tw.delta.vec.v[1] * tw.delta.vec.v[1]))
                                + (float)(tw.delta.vec.v[2] * tw.delta.vec.v[2]);
    tw.deltaLen = sqrtf(tw.deltaLenSq);
    memset(&tw.radius, 0, 12);
    for ( i = 0; i < 3; ++i )
    {
        if ( tw.extents.end.vec.v[i] <= tw.extents.start.vec.v[i] )
        {
            tw.bounds[0].vec.v[i] = tw.extents.end.vec.v[i];
            tw.bounds[1].vec.v[i] = tw.extents.start.vec.v[i];
        }
        else
        {
            tw.bounds[0].vec.v[i] = tw.extents.start.vec.v[i];
            tw.bounds[1].vec.v[i] = tw.extents.end.vec.v[i];
        }
    }
    tw.axialCullOnly = 0;
    tw.isPoint = 1;
    tw.doublesided = 0;
    memset(&tw.radiusOffset, 0, 12);
    CM_GetTraceThreadInfo(&tw.threadInfo);
    if ( !context->nprims )
        return CM_SightTraceThroughTree(&tw, 0, tw.extents.start.vec.v, tw.extents.end.vec.v, &trace);
    prim = context->prims;
    _mm_prefetch((const char *)prim, 1);
    _mm_prefetch((const char *)prim->tree, 1);
    for ( j = 0; j < context->nprims; ++j )
    {
        _mm_prefetch((const char *)context->prims[j + 1].tree, 1);
        if ( prim->type )
        {
            if ( (brushmask & *(unsigned int *)&prim->tree->materialIndex) != 0 )
            {
                hitNum = CM_SightTraceThroughBrush(&tw, prim->brush, ((char *)prim->tree - (char *)cm.brushes) / 96, &trace);
                if ( hitNum )
                    return hitNum;
            }
        }
        else
        {
            tree = prim->tree;
            materialInfo = &cm.materials[tree->materialIndex];
            mn[0] = tree->origin[0] - tree->halfSize[0];
            mn[1] = tree->origin[1] - tree->halfSize[1];
            mn[2] = tree->origin[2] - tree->halfSize[2];
            if ( (brushmask & materialInfo->contentFlags) != 0
                && (float)(tree->origin[0] + tree->halfSize[0]) >= tw.bounds[0].vec.v[0]
                && (float)(tree->origin[1] + tree->halfSize[1]) >= tw.bounds[0].vec.v[1]
                && (float)(tree->origin[2] + tree->halfSize[2]) >= tw.bounds[0].vec.v[2]
                && tw.bounds[1].vec.v[0] >= mn[0]
                && tw.bounds[1].vec.v[1] >= mn[1]
                && tw.bounds[1].vec.v[2] >= mn[2] )
            {
                CM_SightTraceThroughAabbTree(&tw, tree, &trace);
                if ( trace.fraction < 1.0 )
                    return cm.numBrushes + 1;
            }
        }
        ++prim;
    }
    return hitNum;
}

int __cdecl CM_TransformedBoxSightTrace(
                int hitNum,
                const float *start,
                const float *end,
                const float *mins,
                const float *maxs,
                unsigned int model,
                int brushmask,
                const float *origin,
                const float *angles)
{
    bool v10; // [esp+4h] [ebp-9Ch]
    float matrix[3][3]; // [esp+30h] [ebp-70h] BYREF
    float halfwidth; // [esp+54h] [ebp-4Ch]
    float halfheight; // [esp+58h] [ebp-48h]
    float symetricSize[2][3]; // [esp+5Ch] [ebp-44h] BYREF
    float offset[3]; // [esp+74h] [ebp-2Ch]
    float end_l[3]; // [esp+80h] [ebp-20h] BYREF
    float start_l[3]; // [esp+8Ch] [ebp-14h] BYREF
    int rotated; // [esp+98h] [ebp-8h]
    int i; // [esp+9Ch] [ebp-4h]

    if ( !mins && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2945, 0, "%s", "mins") )
        __debugbreak();
    if ( !maxs && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2946, 0, "%s", "maxs") )
        __debugbreak();
    for ( i = 0; i < 3; ++i )
    {
        offset[i] = (float)(mins[i] + maxs[i]) * 0.5;
        symetricSize[0][i] = mins[i] - offset[i];
        symetricSize[1][i] = maxs[i] - offset[i];
        start_l[i] = start[i] + offset[i];
        end_l[i] = end[i] + offset[i];
    }
    start_l[0] = start_l[0] - *origin;
    start_l[1] = start_l[1] - origin[1];
    start_l[2] = start_l[2] - origin[2];
    end_l[0] = end_l[0] - *origin;
    end_l[1] = end_l[1] - origin[1];
    end_l[2] = end_l[2] - origin[2];
    v10 = *angles != 0.0 || angles[1] != 0.0 || angles[2] != 0.0;
    rotated = v10;
    halfwidth = symetricSize[1][0];
    halfheight = symetricSize[1][2];
    if ( v10 )
    {
        AnglesToAxis(angles, matrix);
        RotatePoint(start_l, matrix);
        RotatePoint(end_l, matrix);
    }
    return CM_BoxSightTrace(hitNum, start_l, end_l, symetricSize[0], symetricSize[1], model, brushmask);
}

int __cdecl CM_TracePointDown(
                const float *start,
                const float *end,
                int contentmask,
                int surf_type,
                float *endpos,
                float *fraction,
                float *normal)
{
    float v8; // [esp+0h] [ebp-180h]
    traceWork_t tw; // [esp+40h] [ebp-140h] BYREF
    _QWORD start_[2]; // [esp+120h] [ebp-60h] BYREF
    int i; // [esp+130h] [ebp-50h]
    trace_t results; // [esp+134h] [ebp-4Ch] BYREF
    _QWORD end_[2]; // [esp+170h] [ebp-10h] BYREF

    //traceWork_t::traceWork_t(&tw);
    if ( !start && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2990, 0, "%s", "start") )
        __debugbreak();
    if ( !end && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2991, 0, "%s", "end") )
        __debugbreak();
    nanassertvec3(start);
    nanassertvec3(end);

    memset(&results, 0, sizeof(results));
    results.fraction = 1.0f;
    if ( !cm.numNodes
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 2998, 0, "%s", "cm.numNodes") )
    {
        __debugbreak();
    }
    tw.contents = contentmask;
    for ( i = 0; i < 3; ++i )
    {
        tw.size.vec.u[i] = 0;
        tw.extents.start.vec.v[i] = start[i];
        tw.extents.end.vec.v[i] = end[i];
        tw.midpoint.vec.v[i] = (float)(tw.extents.start.vec.v[i] + tw.extents.end.vec.v[i]) * 0.5;
        tw.delta.vec.v[i] = tw.extents.end.vec.v[i] - tw.extents.start.vec.v[i];
        tw.halfDelta.vec.v[i] = 0.5 * tw.delta.vec.v[i];
        // Original was a bitwise sign-strip on the float bits (== fabsf). Earlier
        // port wrote `fabs(<.u view>)`, which takes fabs of the unsigned int —
        // a no-op for the bit pattern, so a negative halfDelta stays negative
        // in halfDeltaAbs. Downstream SAT culling at line ~2018 then *shrinks*
        // the swept AABB instead of expanding it for downward traces, leaving
        // gravity-pulled traces missing some triangles on entity meshes (e.g.
        // standing on a static-model car → vertical wiggle).
        tw.halfDeltaAbs.vec.v[i] = I_fabs(tw.halfDelta.vec.v[i]);
    }
    CM_CalcTraceExtents(&tw.extents);
    tw.deltaLenSq = (float)((float)(tw.delta.vec.v[0] * tw.delta.vec.v[0]) + (float)(tw.delta.vec.v[1] * tw.delta.vec.v[1]))
                                + (float)(tw.delta.vec.v[2] * tw.delta.vec.v[2]);
    tw.deltaLen = sqrtf(tw.deltaLenSq);
    memset(&tw.radius, 0, 12);
    *(_QWORD *)tw.bounds[0].vec.v = *(_QWORD *)tw.extents.start.vec.v;
    tw.bounds[0].vec.u[2] = tw.extents.start.vec.u[2];
    *(_QWORD *)tw.bounds[1].vec.v = *(_QWORD *)tw.extents.start.vec.v;
    tw.bounds[1].vec.u[2] = tw.extents.start.vec.u[2];
    if ( tw.extents.end.vec.v[2] <= tw.extents.start.vec.v[2] )
    {
        tw.bounds[0].vec.v[2] = tw.extents.end.vec.v[2];
        tw.bounds[1].vec.v[2] = tw.extents.start.vec.v[2];
    }
    else
    {
        tw.bounds[0].vec.v[2] = tw.extents.start.vec.v[2];
        tw.bounds[1].vec.v[2] = tw.extents.end.vec.v[2];
    }
    CM_SetAxialCullOnly(&tw);
    CM_GetTraceThreadInfo(&tw.threadInfo);
    tw.isPoint = 1;
    tw.doublesided = 0;
    memset(&tw.radiusOffset, 0, 12);
    start_[0] = *(_QWORD *)tw.extents.start.vec.v;
    start_[1] = __PAIR64__(0, tw.extents.start.vec.u[2]);
    end_[0] = *(_QWORD *)tw.extents.end.vec.v;
    end_[1] = __PAIR64__(LODWORD(results.fraction), tw.extents.end.vec.u[2]);
    CM_TraceThroughTree(&tw, 0, (const float *)start_, (const float *)end_, &results);
    if ( fraction )
        *fraction = results.fraction;
    if ( normal )
    {
        *(_QWORD *)normal = *(_QWORD *)results.normal.vec.v;
        normal[2] = results.normal.vec.v[2];
    }
    if ( results.fraction == 1.0 || (surf_type & results.sflags) == 0 )
    {
        if ( results.startsolid )
        {
            *endpos = *start;
            endpos[1] = start[1];
            endpos[2] = start[2];
            return results.sflags;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        v8 = results.fraction;
        *endpos = (float)(results.fraction * tw.delta.vec.v[0]) + *start;
        endpos[1] = (float)(v8 * tw.delta.vec.v[1]) + start[1];
        endpos[2] = (float)(v8 * tw.delta.vec.v[2]) + start[2];
        return results.sflags;
    }
}

double __cdecl CM_GetWaterHeight(const float *pos, float z_up, float z_down)
{
    float p[3]; // [esp+14h] [ebp-BCh] BYREF
    trace_t trace; // [esp+60h] [ebp-70h] BYREF
    svEntity_s *check; // [esp+9Ch] [ebp-34h]
    float start[3]; // [esp+A0h] [ebp-30h] BYREF
    float end[3]; // [esp+ACh] [ebp-24h] BYREF
    float waterHeight; // [esp+B8h] [ebp-18h]
    float hitPos[3]; // [esp+BCh] [ebp-14h] BYREF
    int entnum; // [esp+CCh] [ebp-4h]

    waterHeight = R_WaterSimulationGetHeight(pos);
    start[0] = *pos;
    start[1] = pos[1];
    start[2] = pos[2] + z_up;
    end[0] = *pos;
    end[1] = pos[1];
    end[2] = pos[2] + z_down;
    entnum = waterbrush_entity->current.integer;
    if ( entnum > 0 )
    {
        pointtrace_t clip; // [esp+20h] [ebp-B0h] BYREF

        ////TraceExtents::TraceExtents(&clip.extents);
        memset(&trace, 0, sizeof(trace));
        trace.fraction = 1.0f;
        check = &sv.svEntities[entnum];
        clip.contentmask = -1;
        *(_QWORD *)clip.extents.start.vec.v = *(_QWORD *)start;
        clip.extents.start.vec.v[2] = start[2];
        *(_QWORD *)clip.extents.end.vec.v = *(_QWORD *)end;
        clip.extents.end.vec.v[2] = end[2];
        CM_CalcTraceExtents(&clip.extents);
        clip.bLocational = 0;
        clip.ignoreEntParams = 0;
        SV_TracePointToEntity(&clip, check, &trace);
        if ( trace.fraction < 1.0 )
        {
            Vec3Lerp(start, end, trace.fraction, p);
            if ( p[2] > waterHeight )
                waterHeight = p[2];
        }
    }
    if (CM_TracePointDown(start, end, 32, 0x3F00000, hitPos, 0, 0))
    {
        if ( (float)(hitPos[2] - waterHeight) < 0.0 )
            return waterHeight;
        else
            return hitPos[2];
    }
    return waterHeight;
}

void __cdecl calc_closest(const float *v0, const float *v1, const float *v2, const float *p, float *res)
{
    float v5; // [esp+14h] [ebp-90h]
    float pv0[3]; // [esp+1Ch] [ebp-88h] BYREF
    float v02perp[3]; // [esp+28h] [ebp-7Ch] BYREF
    float y21; // [esp+34h] [ebp-70h]
    float v21[3]; // [esp+38h] [ebp-6Ch] BYREF
    float x21; // [esp+44h] [ebp-60h]
    float v10[3]; // [esp+48h] [ebp-5Ch] BYREF
    float x10; // [esp+54h] [ebp-50h]
    float d_sqr02; // [esp+58h] [ebp-4Ch]
    float d_sqr21; // [esp+5Ch] [ebp-48h]
    float y10; // [esp+60h] [ebp-44h]
    float d_sqr10; // [esp+64h] [ebp-40h]
    float v02[3]; // [esp+68h] [ebp-3Ch] BYREF
    float pv1[3]; // [esp+74h] [ebp-30h] BYREF
    float x02; // [esp+80h] [ebp-24h]
    float v02v10; // [esp+84h] [ebp-20h]
    float v21perp[3]; // [esp+88h] [ebp-1Ch] BYREF
    float y02; // [esp+94h] [ebp-10h]
    float v10perp[3]; // [esp+98h] [ebp-Ch]

    v10[0] = *v1 - *v0;
    v10[1] = v1[1] - v0[1];
    v10[2] = v1[2] - v0[2];
    pv0[0] = *p - *v0;
    pv0[1] = p[1] - v0[1];
    pv0[2] = p[2] - v0[2];
    x10 = (float)((float)(pv0[0] * v10[0]) + (float)(pv0[1] * v10[1])) + (float)(pv0[2] * v10[2]);
    v02[0] = *v0 - *v2;
    v02[1] = v0[1] - v2[1];
    v02[2] = v0[2] - v2[2];
    x02 = (float)((float)(pv0[0] * v02[0]) + (float)(pv0[1] * v02[1])) + (float)(pv0[2] * v02[2]);
    if ( x02 < 0.0 || x10 > 0.0 )
    {
        d_sqr02 = (float)((float)(v02[0] * v02[0]) + (float)(v02[1] * v02[1])) + (float)(v02[2] * v02[2]);
        x02 = x02 + d_sqr02;
        d_sqr10 = (float)((float)(v10[0] * v10[0]) + (float)(v10[1] * v10[1])) + (float)(v10[2] * v10[2]);
        v21[0] = *v2 - *v1;
        v21[1] = v2[1] - v1[1];
        v21[2] = v2[2] - v1[2];
        pv1[0] = *p - *v1;
        pv1[1] = p[1] - v1[1];
        pv1[2] = p[2] - v1[2];
        x21 = (float)((float)(pv1[0] * v21[0]) + (float)(pv1[1] * v21[1])) + (float)(pv1[2] * v21[2]);
        if ( x10 < d_sqr10 || x21 > 0.0 )
        {
            d_sqr21 = (float)((float)(v21[0] * v21[0]) + (float)(v21[1] * v21[1])) + (float)(v21[2] * v21[2]);
            if ( x21 < d_sqr21 || x02 > 0.0 )
            {
                v02v10 = (float)((float)(v02[0] * v10[0]) + (float)(v02[1] * v10[1])) + (float)(v02[2] * v10[2]);
                v10perp[0] = (float)(d_sqr10 * v02[0]) + (float)(-v02v10 * v10[0]);
                v10perp[1] = (float)(d_sqr10 * v02[1]) + (float)(-v02v10 * v10[1]);
                v10perp[2] = (float)(d_sqr10 * v02[2]) + (float)(-v02v10 * v10[2]);
                y10 = (float)((float)(pv0[0] * v10perp[0]) + (float)(pv0[1] * v10perp[1])) + (float)(pv0[2] * v10perp[2]);
                if ( y10 < 0.0 || x10 < 0.0 || d_sqr10 < x10 )
                {
                    Vec3ScaleMad(
                        d_sqr21,
                        v10,
                        -((float)((float)(v10[0] * v21[0]) + (float)(v10[1] * v21[1])) + (float)(v10[2] * v21[2])),
                        v21,
                        v21perp);
                    y21 = phys_dot((const phys_vec3 *)pv1, (const phys_vec3 *)v21perp);
                    if ( y21 < 0.0 || x21 < 0.0 || d_sqr21 < x21 )
                    {
                        //Vec3ScaleMad(COERCE_FLOAT(LODWORD(d_sqr02) ^ _mask__NegFloat_), v10, v02v10, v02, v02perp);
                        Vec3ScaleMad(-d_sqr02, v10, v02v10, v02, v02perp);
                        y02 = phys_dot((const phys_vec3 *)pv0, (const phys_vec3 *)v02perp);
                        if ( y02 < 0.0 || x02 < 0.0 || d_sqr02 < x02 )
                            Vec3Copy(p, res);
                        else
                            Vec3Mad(v2, x02 / d_sqr02, v02, res);
                    }
                    else
                    {
                        Vec3Mad(v1, x21 / d_sqr21, v21, res);
                    }
                }
                else
                {
                    v5 = x10 / d_sqr10;
                    *res = (float)((float)(x10 / d_sqr10) * v10[0]) + *v0;
                    res[1] = (float)(v5 * v10[1]) + v0[1];
                    res[2] = (float)(v5 * v10[2]) + v0[2];
                }
            }
            else
            {
                *res = *v2;
                res[1] = v2[1];
                res[2] = v2[2];
            }
        }
        else
        {
            *res = *v1;
            res[1] = v1[1];
            res[2] = v1[2];
        }
    }
    else
    {
        *res = *v0;
        res[1] = v0[1];
        res[2] = v0[2];
    }
}

bool __cdecl is_inside(const float *a, const float *b, const float *n, const float *p)
{
    int v5; // [esp+8h] [ebp-58h]
    int v6; // [esp+10h] [ebp-50h]
    int v7; // [esp+18h] [ebp-48h]
    int v8; // [esp+20h] [ebp-40h]
    int v9; // [esp+24h] [ebp-3Ch]
    int v10; // [esp+28h] [ebp-38h]
    int i2; // [esp+50h] [ebp-10h]
    float alfa; // [esp+54h] [ebp-Ch]
    float alfaa; // [esp+54h] [ebp-Ch]
    int i1; // [esp+58h] [ebp-8h]
    float denom; // [esp+5Ch] [ebp-4h]

    if ( fabs(n[0]) <= fabs(n[1]) )
    {
        i1 = 0;
        if ( fabs(n[1]) <= fabs(n[2]) )
            v9 = 1;
        else
            v9 = 2;
        i2 = v9;
    }
    else
    {
        i1 = 1;
        if ( fabs(n[0]) <= fabs(n[2]) )
            v10 = 0;
        else
            v10 = 2;
        i2 = v10;
    }
    denom = (float)(b[i1] * a[i2]) - (float)(b[i2] * a[i1]);
    alfa = (float)(p[i1] * a[i2]) - (float)(p[i2] * a[i1]);
    if ( alfa >= 0.0 )
        v8 = alfa > 0.0;
    else
        v8 = -1;
    if ( denom >= 0.0 )
        v7 = denom > 0.0;
    else
        v7 = -1;
    if ( v8 != v7 )
        return 0;
    alfaa = (float)(p[i2] * b[i1]) - (float)(p[i1] * b[i2]);
    if ( alfaa >= 0.0 )
        v6 = alfaa > 0.0;
    else
        v6 = -1;
    if ( denom >= 0.0 )
        v5 = denom > 0.0;
    else
        v5 = -1;
    return v6 == v5;
}

bool __cdecl point_in_triangle(const float *p, const float *v0, const float *v1, const float *v2, const float *n)
{
    float v2_v0[3]; // [esp+4h] [ebp-4Ch] BYREF
    float p_v2[3]; // [esp+10h] [ebp-40h] BYREF
    float v1_v2[3]; // [esp+1Ch] [ebp-34h] BYREF
    float v0_v2[3]; // [esp+28h] [ebp-28h] BYREF
    float v1_v0[3]; // [esp+34h] [ebp-1Ch] BYREF
    float p_v0[3]; // [esp+40h] [ebp-10h] BYREF

    v1_v0[0] = *v1 - *v0;
    v1_v0[1] = v1[1] - v0[1];
    v1_v0[2] = v1[2] - v0[2];
    v2_v0[0] = *v2 - *v0;
    v2_v0[1] = v2[1] - v0[1];
    v2_v0[2] = v2[2] - v0[2];
    p_v0[0] = *p - *v0;
    p_v0[1] = p[1] - v0[1];
    p_v0[2] = p[2] - v0[2];
    v1_v2[0] = *v1 - *v2;
    v1_v2[1] = v1[1] - v2[1];
    v1_v2[2] = v1[2] - v2[2];
    v0_v2[0] = *v0 - *v2;
    v0_v2[1] = v0[1] - v2[1];
    v0_v2[2] = v0[2] - v2[2];
    p_v2[0] = *p - *v2;
    p_v2[1] = p[1] - v2[1];
    p_v2[2] = p[2] - v2[2];
    return is_inside(v1_v0, v2_v0, n, p_v0) && is_inside(v1_v2, v0_v2, n, p_v2);
}

char __cdecl trace_sphere_through_triangle(
                const float *c0,
                const float *dir,
                float *t,
                float r,
                const float *v0,
                const float *v1,
                const float *v2,
                const float *n)
{
    float v9; // [esp+0h] [ebp-A0h]
    float v10; // [esp+4h] [ebp-9Ch]
    float v11; // [esp+10h] [ebp-90h]
    float v12; // [esp+14h] [ebp-8Ch]
    float p0; // [esp+24h] [ebp-7Ch]
    float p0_4; // [esp+28h] [ebp-78h]
    float p0_8; // [esp+2Ch] [ebp-74h]
    float _d0; // [esp+34h] [ebp-6Ch]
    float _t; // [esp+38h] [ebp-68h]
    float d; // [esp+44h] [ebp-5Ch]
    float da; // [esp+44h] [ebp-5Ch]
    float rdir; // [esp+48h] [ebp-58h]
    float rdir_4; // [esp+4Ch] [ebp-54h]
    float rdir_8; // [esp+50h] [ebp-50h]
    float root1; // [esp+54h] [ebp-4Ch]
    float offs; // [esp+58h] [ebp-48h]
    float d0; // [esp+5Ch] [ebp-44h]
    float b; // [esp+60h] [ebp-40h]
    float ba; // [esp+60h] [ebp-40h]
    float proj[3]; // [esp+68h] [ebp-38h] BYREF
    float ndir2; // [esp+74h] [ebp-2Ch]
    float c1[3]; // [esp+78h] [ebp-28h]
    float closest[3]; // [esp+84h] [ebp-1Ch] BYREF
    float root2; // [esp+90h] [ebp-10h]
    float w[3]; // [esp+94h] [ebp-Ch]

    v11 = *t;
    c1[0] = (float)(*t * *dir) + *c0;
    c1[1] = (float)(v11 * dir[1]) + c0[1];
    c1[2] = (float)(v11 * dir[2]) + c0[2];
    //LODWORD(offs) = COERCE_UNSIGNED_INT((float)((float)(*n * *v0) + (float)(n[1] * v0[1])) + (float)(n[2] * v0[2])) ^ _mask__NegFloat_;
    offs = -((float)((float)(*n * *v0) + (float)(n[1] * v0[1])) + (float)(n[2] * v0[2]));
    d0 = (float)((float)((float)(*n * *c0) + (float)(n[1] * c0[1])) + (float)(n[2] * c0[2])) + offs;
    //if ( COERCE_FLOAT(LODWORD(r) ^ _mask__NegFloat_) >= d0 )
    if ( -(r) >= d0 )
        return 0;
    if ( (float)((float)((float)((float)(*n * c1[0]) + (float)(n[1] * c1[1])) + (float)(n[2] * c1[2])) + offs) >= r )
        return 0;
    if ( d0 <= 0.0 )
    {
        proj[0] = (float)(d0 * *n) + *c0;
        proj[1] = (float)(d0 * n[1]) + c0[1];
        proj[2] = (float)(d0 * n[2]) + c0[2];
    }
    else
    {
        if ( d0 <= r )
        {
            proj[0] = (float)(-(d0) * *n) + *c0;
            proj[1] = (float)(-(d0) * n[1]) + c0[1];
            proj[2] = (float)(-(d0) * n[2]) + c0[2];
            _t = 0.0f;
        }
        else
        {
            p0 =   (float)(-(r) * *n) + *c0;
            p0_4 = (float)(-(r) * n[1]) + c0[1];
            p0_8 = (float)(-(r) * n[2]) + c0[2];
            _d0 = (float)((float)((float)(*n * p0) + (float)(n[1] * p0_4)) + (float)(n[2] * p0_8)) + offs;
            _t = _d0
                 / (float)(_d0
                                 - (float)((float)((float)((float)(*n * (float)(p0 + *dir)) + (float)(n[1] * (float)(p0_4 + dir[1])))
                                                                 + (float)(n[2] * (float)(p0_8 + dir[2])))
                                                 + offs));
            proj[0] = (float)(_t * *dir) + p0;
            proj[1] = (float)(_t * dir[1]) + p0_4;
            proj[2] = (float)(_t * dir[2]) + p0_8;
        }
        if ( point_in_triangle(proj, v0, v1, v2, n) )
        {
            *t = _t;
            return 1;
        }
    }
    //LODWORD(v10) = *(unsigned int *)t ^ _mask__NegFloat_;
    v10 = -*t;
    rdir = v10 * *dir;
    rdir_4 = v10 * dir[1];
    rdir_8 = v10 * dir[2];
    ndir2 = (float)((float)(rdir * rdir) + (float)(rdir_4 * rdir_4)) + (float)(rdir_8 * rdir_8);
    calc_closest(v0, v1, v2, proj, closest);
    w[0] = closest[0] - *c0;
    w[1] = closest[1] - c0[1];
    w[2] = closest[2] - c0[2];
    b = (float)((float)(rdir * w[0]) + (float)(rdir_4 * w[1])) + (float)(rdir_8 * w[2]);
    d = (float)(b * b)
        + (float)(ndir2
                        * (float)((float)(r * r)
                                        - (float)((float)((float)(w[0] * w[0]) + (float)(w[1] * w[1])) + (float)(w[2] * w[2]))));
    if ( d <= 0.0 )
        return 0;
    ba = b * -1.0;
    da = sqrtf(d);
    root1 = ba - da;
    root2 = ba + da;
    if ( (float)(ba + da) < 0.0 || ndir2 < root1 || (float)((float)(ndir2 * ndir2) + 0.000099999997) <= root1 )
        return 0;
    if ( *t >= 0.0 )
    {
        c1[0] = *c0;
        c1[1] = c0[1];
        c1[2] = c0[2];
    }
    else
    {
        v9 = *t;
        c1[0] = (float)(*t * rdir) + *c0;
        c1[1] = (float)(v9 * rdir_4) + c0[1];
        c1[2] = (float)(v9 * rdir_8) + c0[2];
    }
    //v12 = COERCE_FLOAT(LODWORD(root1) ^ _mask__NegFloat_) / ndir2;
    v12 = -root1 / ndir2;
    if (v12 >= 0.0)
    {
        *t = 0.0f;
    }
    else
    {
        //*t = COERCE_FLOAT(LODWORD(v12) ^ _mask__NegFloat_) * *t;
        *t = -v12 * *t;
    }
    return 1;
}

void __cdecl trace_sphere_through_partition(
                const float *c,
                const float *dir,
                float *t,
                float r,
                const CollisionAabbTree *tree,
                float *hitn,
                int *sflags)
{
    float *v7; // [esp+20h] [ebp-84h]
    float *v8; // [esp+24h] [ebp-80h]
    float *v9; // [esp+28h] [ebp-7Ch]
    int triIndex; // [esp+38h] [ebp-6Ch]
    float v0[3]; // [esp+3Ch] [ebp-68h] BYREF
    float v0_v2[3]; // [esp+48h] [ebp-5Ch] BYREF
    float ndir[3]; // [esp+54h] [ebp-50h] BYREF
    float un[3]; // [esp+60h] [ebp-44h] BYREF
    float v2[3]; // [esp+6Ch] [ebp-38h] BYREF
    float v1[3]; // [esp+78h] [ebp-2Ch] BYREF
    bool hitb; // [esp+87h] [ebp-1Dh]
    float n[3]; // [esp+88h] [ebp-1Ch] BYREF
    const CollisionPartition *partition; // [esp+94h] [ebp-10h]
    float v0_v1[3]; // [esp+98h] [ebp-Ch] BYREF

    hitb = 0;
    Vec3NormalizeTo(dir, ndir);
    partition = &cm.partitions[tree->u.firstChildIndex];
    for ( triIndex = partition->firstTri; triIndex < partition->firstTri + partition->triCount; ++triIndex )
    {
        v9 = cm.verts[cm.triIndices[3 * triIndex]];
        v0[0] = *v9;
        v0[1] = v9[1];
        v0[2] = v9[2];
        v8 = cm.verts[cm.triIndices[3 * triIndex + 1]];
        v1[0] = *v8;
        v1[1] = v8[1];
        v1[2] = v8[2];
        v7 = cm.verts[cm.triIndices[3 * triIndex + 2]];
        v2[0] = *v7;
        v2[1] = v7[1];
        v2[2] = v7[2];
        v0_v1[0] = v0[0] - v1[0];
        v0_v1[1] = v0[1] - v1[1];
        v0_v1[2] = v0[2] - v1[2];
        v0_v2[0] = v0[0] - v2[0];
        v0_v2[1] = v0[1] - v2[1];
        v0_v2[2] = v0[2] - v2[2];
        Vec3Cross(v0_v2, v0_v1, n);
        Vec3NormalizeTo(n, un);
        if ( (float)((float)((float)(ndir[0] * un[0]) + (float)(ndir[1] * un[1])) + (float)(ndir[2] * un[2])) < 0.0 )
        {
            if ( trace_sphere_through_triangle(c, dir, t, r, v0, v1, v2, un) )
            {
                *hitn = un[0];
                hitn[1] = un[1];
                hitn[2] = un[2];
                hitb = 1;
            }
        }
    }
    if ( hitb )
        *sflags = cm.materials[tree->materialIndex].surfaceFlags;
}

void __cdecl trace_sphere_through_brush(
                const float *c0,
                const float *dir,
                float *t,
                float r,
                const cbrush_t *brush,
                float *hitn,
                int *sflags,
                clipMap_t *pClipMap)
{
    float v7; // [esp+4h] [ebp-F4h]
    float v8; // [esp+8h] [ebp-F0h]
    float v9; // [esp+Ch] [ebp-ECh]
    float v10; // [esp+10h] [ebp-E8h]
    float v11; // [esp+14h] [ebp-E4h]
    float v12; // [esp+18h] [ebp-E0h]
    float *normal; // [esp+1Ch] [ebp-DCh]
    float d1; // [esp+6Ch] [ebp-8Ch]
    float d1a; // [esp+6Ch] [ebp-8Ch]
    cbrushside_t *side; // [esp+70h] [ebp-88h]
    int j; // [esp+74h] [ebp-84h]
    cplane_s *plane; // [esp+78h] [ebp-80h]
    float enterFrac; // [esp+7Ch] [ebp-7Ch]
    float delta; // [esp+80h] [ebp-78h]
    float deltaa; // [esp+80h] [ebp-78h]
    float frac; // [esp+84h] [ebp-74h]
    float fraca; // [esp+84h] [ebp-74h]
    int allsolid; // [esp+88h] [ebp-70h]
    float dist; // [esp+8Ch] [ebp-6Ch]
    cbrushside_t *leadside; // [esp+90h] [ebp-68h]
    cbrushside_t axialSide; // [esp+94h] [ebp-64h] BYREF
    cplane_s axialPlane; // [esp+A0h] [ebp-58h] BYREF
    float sign; // [esp+B8h] [ebp-40h]
    float d2; // [esp+BCh] [ebp-3Ch]
    const float *bounds; // [esp+C0h] [ebp-38h]
    float c1[3]; // [esp+C4h] [ebp-34h]
    float f; // [esp+D0h] [ebp-28h]
    int index; // [esp+D4h] [ebp-24h]
    float inv_neg_dir[3]; // [esp+D8h] [ebp-20h]
    float leaveFrac; // [esp+E4h] [ebp-14h]
    int i; // [esp+E8h] [ebp-10h]
    float neg_dir[3]; // [esp+ECh] [ebp-Ch]

    nanassertvec3(c0);
    nanassertvec3(dir);

    c1[0] = *c0 + *dir;
    c1[1] = c0[1] + dir[1];
    c1[2] = c0[2] + dir[2];
    //LODWORD(neg_dir[0]) = *(unsigned int *)dir ^ _mask__NegFloat_;
    //LODWORD(neg_dir[1]) = *((unsigned int *)dir + 1) ^ _mask__NegFloat_;
    //LODWORD(neg_dir[2]) = *((unsigned int *)dir + 2) ^ _mask__NegFloat_;
    neg_dir[0] = -dir[0];
    neg_dir[1] = -dir[1];
    neg_dir[2] = -dir[2];
    if ( neg_dir[0] == 0.0 )
        v12 = 0.0f;
    else
        v12 = 1.0 / neg_dir[0];
    inv_neg_dir[0] = v12;
    if ( neg_dir[1] == 0.0 )
        v11 = 0.0f;
    else
        v11 = 1.0 / neg_dir[1];
    inv_neg_dir[1] = v11;
    if ( neg_dir[2] == 0.0 )
        v10 = 0.0f;
    else
        v10 = 1.0 / neg_dir[2];
    inv_neg_dir[2] = v10;
    enterFrac = 0.0f;
    leaveFrac = *t;
    allsolid = 1;
    leadside = 0;
    sign = -1.0f;
    bounds = (const float *)brush;
    index = 0;
    while ( 2 )
    {
        nanassertvec3(bounds);

        for ( j = 0; j < 3; ++j )
        {
            d1 = (float)((float)(c0[j] - bounds[j]) * sign) - r;
            d2 = (float)((float)(c1[j] - bounds[j]) * sign) - r;
            if ( (LODWORD(d1) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 3412, 0, "%s", "!IS_NAN(d1)") )
            {
                __debugbreak();
            }
            if ( (LODWORD(d2) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 3413, 0, "%s", "!IS_NAN(d2)") )
            {
                __debugbreak();
            }
            if ( d1 <= 0.0 )
            {
                if ( d2 > 0.0 )
                {
                    fraca = (float)(d1 * inv_neg_dir[j]) * sign;
                    if ( enterFrac >= fraca )
                        return;
                    allsolid = 0;
                    if ( (float)(fraca - leaveFrac) < 0.0 )
                        v8 = (float)(d1 * inv_neg_dir[j]) * sign;
                    else
                        v8 = leaveFrac;
                    leaveFrac = v8;
                }
            }
            else
            {
                if ( (float)(0.125 - d1) < 0.0 )
                    v9 = 0.125f;
                else
                    v9 = d1;
                if ( d2 >= v9 )
                    return;
                frac = (float)((float)(d1 - 0.125) * inv_neg_dir[j]) * sign;
                if ( frac >= leaveFrac )
                    return;
                if ( d2 > 0.0 )
                    allsolid = 0;
                if ( frac <= enterFrac )
                {
                    if ( leadside )
                        continue;
                }
                else
                {
                    enterFrac = (float)((float)(d1 - 0.125) * inv_neg_dir[j]) * sign;
                }
                axialSide.cflags = brush->axial_cflags[index][j];
                axialSide.sflags = brush->axial_sflags[index][j];
                memset(&axialPlane, 0, 12);
                axialPlane.normal[j] = sign;
                axialSide.plane = &axialPlane;
                leadside = &axialSide;
            }
        }
        if ( !index )
        {
            sign = 1.0f;
            bounds = brush->maxs;
            index = 1;
            continue;
        }
        break;
    }
    if ( brush->numsides )
        side = brush->sides;
    else
        side = 0;
    i = brush->numsides;
    if ( i < 0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 3461, 0, "%s", "i >= 0") )
        __debugbreak();
    while ( 2 )
    {
        if ( i )
        {
            plane = side->plane;
            if ( (LODWORD(side->plane->dist) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                            3466,
                            0,
                            "%s",
                            "!IS_NAN(plane->dist)") )
            {
                __debugbreak();
            }
            if ( ((LODWORD(plane->normal[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(plane->normal[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(plane->normal[2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                            3469,
                            0,
                            "%s",
                            "!IS_NAN((plane->normal)[0]) && !IS_NAN((plane->normal)[1]) && !IS_NAN((plane->normal)[2])") )
            {
                __debugbreak();
            }
            dist = plane->dist + r;
            if ( (LODWORD(dist) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 3471, 0, "%s", "!IS_NAN(dist)") )
            {
                __debugbreak();
            }
            d1a = (float)((float)((float)(*c0 * plane->normal[0]) + (float)(c0[1] * plane->normal[1]))
                                    + (float)(c0[2] * plane->normal[2]))
                    - dist;
            d2 = (float)((float)((float)(c1[0] * plane->normal[0]) + (float)(c1[1] * plane->normal[1]))
                                 + (float)(c1[2] * plane->normal[2]))
                 - dist;
            if ( (LODWORD(d1a) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 3476, 0, "%s", "!IS_NAN(d1)") )
            {
                __debugbreak();
            }
            if ( (LODWORD(d2) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 3477, 0, "%s", "!IS_NAN(d2)") )
            {
                __debugbreak();
            }
            if ( d1a <= 0.0 )
            {
                if ( d2 > 0.0 )
                {
                    deltaa = d1a - d2;
                    if ( (float)(d1a - d2) >= 0.0
                        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 3507, 0, "%s", "delta < 0") )
                    {
                        __debugbreak();
                    }
                    if ( d1a > (float)(leaveFrac * deltaa) )
                    {
                        leaveFrac = d1a / deltaa;
                        if ( enterFrac >= (float)(d1a / deltaa) )
                            return;
                    }
                    allsolid = 0;
                }
            }
            else
            {
                if ( (float)(0.125 - d1a) < 0.0 )
                    v7 = 0.125f;
                else
                    v7 = d1a;
                if ( d2 >= v7 )
                    return;
                if ( d2 > 0.0 )
                    allsolid = 0;
                delta = d1a - d2;
                iassert(!IS_NAN(delta));
                //if ( (COERCE_UNSIGNED_INT(d1a - d2) & 0x7F800000) == 0x7F800000
                //    && !Assert_MyHandler(
                //                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                //                3488,
                //                0,
                //                "%s",
                //                "!IS_NAN(delta)") )
                //{
                //    __debugbreak();
                //}
                if ( delta <= 0.0
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 3490, 0, "%s", "delta > 0") )
                {
                    __debugbreak();
                }
                f = d1a - 0.125;
                if ( (float)(d1a - 0.125) <= (float)(enterFrac * delta) )
                {
                    if ( !leadside )
                        goto LABEL_99;
                }
                else
                {
                    enterFrac = f / delta;
                    if ( (float)(f / delta) >= leaveFrac )
                        return;
LABEL_99:
                    leadside = side;
                }
            }
            --i;
            ++side;
            continue;
        }
        break;
    }
    if ( leadside )
    {
        *t = enterFrac;
        *sflags = leadside->sflags;
        normal = leadside->plane->normal;
        *hitn = leadside->plane->normal[0];
        hitn[1] = normal[1];
        hitn[2] = normal[2];
    }
    else
    {
        if ( allsolid )
            *t = 0.0f;
        *sflags = 0;
    }
}

void __cdecl trace_sphere_through_brush(
                const float *c0,
                const float *dir,
                float *t,
                float r,
                const cbrush_t *brush,
                float *hitn,
                int *sflags)
{
    trace_sphere_through_brush(c0, dir, t, r, brush, hitn, sflags, NULL);
}

void __cdecl trace_point_through_brush(
                const float *c0,
                const float *dir,
                float *t,
                const cbrush_t *brush,
                float *hitn,
                int *sflags)
{
    if ( (*t < 0.0 || *t > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                    3556,
                    0,
                    "%s",
                    "t >= 0.0f && t <= 1.0f") )
    {
        __debugbreak();
    }
    trace_sphere_through_brush(c0, dir, t, 0.0, brush, hitn, sflags, NULL);
    if ( (*t < 0.0 || *t > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                    3577,
                    0,
                    "%s",
                    "t >= 0.0f && t <= 1.0f") )
    {
        __debugbreak();
    }
}

char __cdecl trace_point_through_triangle(
                const float *p0,
                const float *u0,
                const float *v0,
                const float *v1,
                const float *v2,
                float cur_t,
                float *t)
{
    float b_; // [esp+0h] [ebp-58h]
    float det; // [esp+4h] [ebp-54h]
    float c2[3]; // [esp+8h] [ebp-50h] BYREF
    float c1[3]; // [esp+14h] [ebp-44h] BYREF
    float rs[3]; // [esp+20h] [ebp-38h] BYREF
    float cr_rs_c2[3]; // [esp+2Ch] [ebp-2Ch] BYREF
    float a_; // [esp+38h] [ebp-20h]
    float c0[3]; // [esp+3Ch] [ebp-1Ch] BYREF
    float cr_c0_c1[3]; // [esp+48h] [ebp-10h] BYREF
    float t_; // [esp+54h] [ebp-4h]

    if ( (cur_t < 0.0 || cur_t > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                    3582,
                    0,
                    "%s",
                    "cur_t >= 0.0f && cur_t <= 1.0f") )
    {
        __debugbreak();
    }
    c0[0] = *v1 - *v0;
    c0[1] = v1[1] - v0[1];
    c0[2] = v1[2] - v0[2];
    c1[0] = *v2 - *v0;
    c1[1] = v2[1] - v0[1];
    c1[2] = v2[2] - v0[2];
    //LODWORD(c2[0]) = *(unsigned int *)u0 ^ _mask__NegFloat_;
    //LODWORD(c2[1]) = *((unsigned int *)u0 + 1) ^ _mask__NegFloat_;
    //LODWORD(c2[2]) = *((unsigned int *)u0 + 2) ^ _mask__NegFloat_;
    c2[0] = -u0[0];
    c2[1] = -u0[1];
    c2[2] = -u0[2];

    Vec3Cross(c0, c1, cr_c0_c1);
    det = (float)((float)(cr_c0_c1[0] * c2[0]) + (float)(cr_c0_c1[1] * c2[1])) + (float)(cr_c0_c1[2] * c2[2]);
    if ( fabs(det) < 0.000099999997 )
        return 0;
    rs[0] = *p0 - *v0;
    rs[1] = p0[1] - v0[1];
    rs[2] = p0[2] - v0[2];
    t_ = (float)((float)((float)(cr_c0_c1[0] * rs[0]) + (float)(cr_c0_c1[1] * rs[1])) + (float)(cr_c0_c1[2] * rs[2]))
         / det;
    if ( t_ < 0.0 || t_ > cur_t )
        return 0;
    Vec3Cross(rs, c2, cr_rs_c2);
    //a_ = COERCE_FLOAT(COERCE_UNSIGNED_INT((float)((float)(cr_rs_c2[0] * c1[0]) + (float)(cr_rs_c2[1] * c1[1])) + (float)(cr_rs_c2[2] * c1[2])) ^ _mask__NegFloat_) / det;
    a_ = -(((float)((float)(cr_rs_c2[0] * c1[0]) + (float)(cr_rs_c2[1] * c1[1])) + (float)(cr_rs_c2[2] * c1[2]))) / det;
    if ( a_ < 0.0 )
        return 0;
    b_ = (float)((float)((float)(cr_rs_c2[0] * c0[0]) + (float)(cr_rs_c2[1] * c0[1])) + (float)(cr_rs_c2[2] * c0[2]))
         / det;
    if ( b_ < 0.0 || (float)(a_ + b_) > 1.0 )
        return 0;
    *t = t_;
    if ( (t_ < 0.0 || t_ > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                    3612,
                    0,
                    "%s",
                    "t_ >= 0.0f && t_ <= 1.0f") )
    {
        __debugbreak();
    }
    return 1;
}

void __cdecl trace_sphere_through_partition(
                const float *p,
                const float *dir,
                float *t,
                float r,
                const CollisionPartition *partition,
                float *hitn,
                int *hint,
                clipMap_t *pClipMap)
{
    float *v8; // [esp+20h] [ebp-68h]
    float *v9; // [esp+24h] [ebp-64h]
    float *v10; // [esp+28h] [ebp-60h]
    unsigned __int16 *indicies; // [esp+2Ch] [ebp-5Ch]
    int triIndex; // [esp+30h] [ebp-58h]
    float v0[3]; // [esp+34h] [ebp-54h] BYREF
    float v0_v2[3]; // [esp+40h] [ebp-48h] BYREF
    float un[3]; // [esp+4Ch] [ebp-3Ch] BYREF
    float v2[3]; // [esp+58h] [ebp-30h] BYREF
    float v1[3]; // [esp+64h] [ebp-24h] BYREF
    float n[3]; // [esp+70h] [ebp-18h] BYREF
    float v0_v1[3]; // [esp+7Ch] [ebp-Ch] BYREF

    for ( triIndex = partition->firstTri; triIndex < partition->firstTri + partition->triCount; ++triIndex )
    {
        indicies = &pClipMap->triIndices[3 * triIndex];
        v10 = pClipMap->verts[*indicies];
        v0[0] = *v10;
        v0[1] = v10[1];
        v0[2] = v10[2];
        v9 = pClipMap->verts[indicies[1]];
        v1[0] = *v9;
        v1[1] = v9[1];
        v1[2] = v9[2];
        v8 = pClipMap->verts[indicies[2]];
        v2[0] = *v8;
        v2[1] = v8[1];
        v2[2] = v8[2];
        v0_v1[0] = v0[0] - v1[0];
        v0_v1[1] = v0[1] - v1[1];
        v0_v1[2] = v0[2] - v1[2];
        v0_v2[0] = v0[0] - v2[0];
        v0_v2[1] = v0[1] - v2[1];
        v0_v2[2] = v0[2] - v2[2];
        Vec3Cross(v0_v2, v0_v1, n);
        Vec3NormalizeTo(n, un);
        if ( (float)((float)((float)(*dir * n[0]) + (float)(dir[1] * n[1])) + (float)(dir[2] * n[2])) < 0.0 )
        {
            if ( (*t < 0.0 || *t > 1.0)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                            3641,
                            0,
                            "%s",
                            "t >= 0.0f && t <= 1.0f") )
            {
                __debugbreak();
            }
            if ( trace_sphere_through_triangle(p, dir, t, r, v0, v1, v2, un) )
            {
                *hitn = un[0];
                hitn[1] = un[1];
                hitn[2] = un[2];
                *hint = triIndex + pClipMap->numBrushes;
            }
        }
    }
}

void __cdecl trace_point_through_partition(
                const float *p,
                const float *dir,
                float *t,
                const CollisionPartition *partition,
                float *hitn,
                int *hint,
                clipMap_t *pClipMap)
{
    float *v7; // [esp+14h] [ebp-5Ch]
    float *v8; // [esp+18h] [ebp-58h]
    float *v9; // [esp+1Ch] [ebp-54h]
    unsigned __int16 *indicies; // [esp+20h] [ebp-50h]
    int triIndex; // [esp+24h] [ebp-4Ch]
    float v0[3]; // [esp+28h] [ebp-48h] BYREF
    float v0_v2[3]; // [esp+34h] [ebp-3Ch] BYREF
    float v2[3]; // [esp+40h] [ebp-30h] BYREF
    float v1[3]; // [esp+4Ch] [ebp-24h] BYREF
    float n[3]; // [esp+58h] [ebp-18h] BYREF
    float v0_v1[3]; // [esp+64h] [ebp-Ch] BYREF

    for ( triIndex = partition->firstTri; triIndex < partition->firstTri + partition->triCount; ++triIndex )
    {
        indicies = &pClipMap->triIndices[3 * triIndex];
        v9 = pClipMap->verts[*indicies];
        v0[0] = *v9;
        v0[1] = v9[1];
        v0[2] = v9[2];
        v8 = pClipMap->verts[indicies[1]];
        v1[0] = *v8;
        v1[1] = v8[1];
        v1[2] = v8[2];
        v7 = pClipMap->verts[indicies[2]];
        v2[0] = *v7;
        v2[1] = v7[1];
        v2[2] = v7[2];
        v0_v1[0] = v0[0] - v1[0];
        v0_v1[1] = v0[1] - v1[1];
        v0_v1[2] = v0[2] - v1[2];
        v0_v2[0] = v0[0] - v2[0];
        v0_v2[1] = v0[1] - v2[1];
        v0_v2[2] = v0[2] - v2[2];
        Vec3Cross(v0_v2, v0_v1, n);
        if ( (float)((float)((float)(*dir * n[0]) + (float)(dir[1] * n[1])) + (float)(dir[2] * n[2])) < 0.0 )
        {
            if ( (*t < 0.0 || *t > 1.0)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                            3668,
                            0,
                            "%s",
                            "t >= 0.0f && t <= 1.0f") )
            {
                __debugbreak();
            }
            if ( trace_point_through_triangle(p, dir, v0, v1, v2, *t, t) )
            {
                Vec3NormalizeTo(n, hitn);
                *hint = triIndex + pClipMap->numBrushes;
            }
        }
    }
}

void __cdecl trace_point_vs_env(
                trace_t *trace,
                const float *start,
                const float *end,
                col_context_t *context,
                int *index_hint,
                clipMap_t *pClipMap)
{
    float *v8; // [esp+30h] [ebp-F4h]
    float *v9; // [esp+34h] [ebp-F0h]
    float *v10; // [esp+38h] [ebp-ECh]
    float fraction; // [esp+60h] [ebp-C4h]
    const cbrush_t *v12; // [esp+68h] [ebp-BCh]
    float old_fraction; // [esp+70h] [ebp-B4h]
    const CollisionAabbTree *tree; // [esp+78h] [ebp-ACh]
    int i; // [esp+80h] [ebp-A4h]
    float v0_v2[3]; // [esp+84h] [ebp-A0h] BYREF
    float n[3]; // [esp+90h] [ebp-94h] BYREF
    float v0_v1[4]; // [esp+9Ch] [ebp-88h] BYREF
    unsigned __int16 *indicies; // [esp+ACh] [ebp-78h]
    int tid; // [esp+B0h] [ebp-74h]
    const cbrush_t *brush; // [esp+B8h] [ebp-6Ch]
    float v0[3]; // [esp+BCh] [ebp-68h] BYREF
    float dir[3]; // [esp+C8h] [ebp-5Ch] BYREF
    const col_prim_t *prim; // [esp+D4h] [ebp-50h]
    float mx[3]; // [esp+D8h] [ebp-4Ch]
    float bounds[2][3]; // [esp+E4h] [ebp-40h] BYREF
    float v2[3]; // [esp+FCh] [ebp-28h] BYREF
    bool bHintFound; // [esp+10Bh] [ebp-19h]
    float v1[3]; // [esp+10Ch] [ebp-18h] BYREF
    float mn[3]; // [esp+118h] [ebp-Ch]

    dir[0] = *end - *start;
    dir[1] = end[1] - start[1];
    dir[2] = end[2] - start[2];

    nanassertvec3(start);
    nanassertvec3(end);
    nanassertvec3(dir);

    memset(trace, 0, sizeof(trace_t));
    trace->fraction = 1.0f;
    trace->boneIndex = 254;
    bHintFound = 0;
    if ( *index_hint > 0 )
    {
        if ( *index_hint >= pClipMap->numBrushes )
        {
            tid = *index_hint - pClipMap->numBrushes;
            indicies = &pClipMap->triIndices[3 * tid];
            v10 = pClipMap->verts[*indicies];
            v0[0] = *v10;
            v0[1] = v10[1];
            v0[2] = v10[2];
            v9 = pClipMap->verts[indicies[1]];
            v1[0] = *v9;
            v1[1] = v9[1];
            v1[2] = v9[2];
            v8 = pClipMap->verts[indicies[2]];
            v2[0] = *v8;
            v2[1] = v8[1];
            v2[2] = v8[2];
            if ( trace_point_through_triangle(start, dir, v0, v1, v2, 1.0, &trace->fraction) )
            {
                v0_v1[0] = v0[0] - v1[0];
                v0_v1[1] = v0[1] - v1[1];
                v0_v1[2] = v0[2] - v1[2];
                v0_v2[0] = v0[0] - v2[0];
                v0_v2[1] = v0[1] - v2[1];
                v0_v2[2] = v0[2] - v2[2];
                Vec3Cross(v0_v2, v0_v1, n);
                Vec3NormalizeTo(n, trace->normal.vec.v);
                if ( !trace
                    && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_public.h", 175, 0, "%s", "trace") )
                {
                    __debugbreak();
                }
                trace->hitType = TRACE_HITTYPE_ENTITY;
                trace->hitId = 1022;
                bHintFound = 1;
            }
        }
        else
        {
            brush = &pClipMap->brushes[*index_hint];
            trace_point_through_brush(start, dir, &trace->fraction, brush, trace->normal.vec.v, &trace->sflags);
            if ( trace->fraction < 1.0 )
            {
                if ( !trace
                    && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_public.h", 175, 0, "%s", "trace") )
                {
                    __debugbreak();
                }
                trace->hitType = TRACE_HITTYPE_ENTITY;
                trace->hitId = 1022;
                bHintFound = 1;
            }
        }
    }
    if ( !bHintFound )
        *index_hint = -1;
    Vec3Min(start, end, bounds[0]);
    Vec3Max(start, end, bounds[1]);
    prim = context->prims;
    for ( i = 0; i < context->nprims; ++i )
    {
        if ( prim->type )
        {
            v12 = prim->brush;
            if ( v12->maxs[0] >= bounds[0][0]
                && v12->maxs[1] >= bounds[0][1]
                && v12->maxs[2] >= bounds[0][2]
                && bounds[1][0] >= v12->mins[0]
                && bounds[1][1] >= v12->mins[1]
                && bounds[1][2] >= v12->mins[2] )
            {
                fraction = trace->fraction;
                trace_point_through_brush(start, dir, &trace->fraction, v12, trace->normal.vec.v, &trace->sflags);
                if ( fraction > trace->fraction )
                    *index_hint = ((char *)prim->tree - (char *)pClipMap->brushes) / 96;
            }
        }
        else
        {
            tree = prim->tree;
            mn[0] = tree->origin[0] - tree->halfSize[0];
            mn[1] = tree->origin[1] - tree->halfSize[1];
            mn[2] = tree->origin[2] - tree->halfSize[2];
            mx[0] = tree->origin[0] + tree->halfSize[0];
            mx[1] = tree->origin[1] + tree->halfSize[1];
            mx[2] = tree->origin[2] + tree->halfSize[2];
            if ( mx[0] >= bounds[0][0]
                && mx[1] >= bounds[0][1]
                && mx[2] >= bounds[0][2]
                && bounds[1][0] >= mn[0]
                && bounds[1][1] >= mn[1]
                && bounds[1][2] >= mn[2] )
            {
                old_fraction = trace->fraction;
                trace_point_through_partition(
                    start,
                    dir,
                    &trace->fraction,
                    &pClipMap->partitions[tree->u.firstChildIndex],
                    trace->normal.vec.v,
                    index_hint,
                    pClipMap);
                if ( old_fraction > trace->fraction )
                    trace->sflags = pClipMap->materials[tree->materialIndex].surfaceFlags;
            }
        }
        ++prim;
    }
    if ( trace->fraction >= 1.0 )
    {
        if ( !trace && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_public.h", 175, 0, "%s", "trace") )
            __debugbreak();
        trace->hitType = TRACE_HITTYPE_NONE;
        trace->hitId = 0;
    }
    else
    {
        if ( !trace && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_public.h", 175, 0, "%s", "trace") )
            __debugbreak();
        trace->hitType = TRACE_HITTYPE_ENTITY;
        trace->hitId = 1022;
    }
}

void __cdecl trace_point_vs_env(
                trace_t *trace,
                const float *start,
                const float *end,
                col_context_t *context,
                int *index_hint)
{
    trace_point_vs_env(trace, start, end, context, index_hint, &cm);
}

void __cdecl trace_point_vs_env(
                trace_t *trace,
                const float *start,
                const float *end,
                col_context_t *context,
                int *index_hint,
                unsigned __int8 *pClipMap)
{
    trace_point_vs_env(trace, start, end, context, index_hint, (clipMap_t *)pClipMap);
}

void __cdecl trace_sphere_vs_env(
                trace_t *trace,
                const float *start,
                const float *end,
                float radius,
                col_context_t *context,
                int *index_hint,
                clipMap_t *pClipMap)
{
    float *v9; // [esp+44h] [ebp-100h]
    float *v10; // [esp+48h] [ebp-FCh]
    float *v11; // [esp+4Ch] [ebp-F8h]
    float fraction; // [esp+74h] [ebp-D0h]
    const cbrush_t *brush; // [esp+7Ch] [ebp-C8h]
    float old_fraction; // [esp+84h] [ebp-C0h]
    const CollisionAabbTree *tree; // [esp+8Ch] [ebp-B8h]
    int i; // [esp+94h] [ebp-B0h]
    unsigned __int16 *indicies; // [esp+9Ch] [ebp-A8h]
    float v0[3]; // [esp+ACh] [ebp-98h] BYREF
    float dir[3]; // [esp+B8h] [ebp-8Ch] BYREF
    float v0_v2[3]; // [esp+C4h] [ebp-80h] BYREF
    const col_prim_t *prim; // [esp+D0h] [ebp-74h]
    float mx[3]; // [esp+D4h] [ebp-70h]
    float bounds[2][3]; // [esp+E0h] [ebp-64h] BYREF
    float v2[3]; // [esp+F8h] [ebp-4Ch] BYREF
    bool bHintFound; // [esp+107h] [ebp-3Dh]
    float boxSize[3]; // [esp+108h] [ebp-3Ch]
    float v1[3]; // [esp+114h] [ebp-30h] BYREF
    float mn[3]; // [esp+120h] [ebp-24h]
    float n[3]; // [esp+12Ch] [ebp-18h] BYREF
    float v0_v1[3]; // [esp+138h] [ebp-Ch] BYREF

    dir[0] = *end - *start;
    dir[1] = end[1] - start[1];
    dir[2] = end[2] - start[2];

    nanassertvec3(start);
    nanassertvec3(end);
    nanassertvec3(dir);

    trace->fraction = 1.0f;
    bHintFound = 0;
    if ( *index_hint > 0 )
    {
        if ( *index_hint >= pClipMap->numBrushes )
        {
            indicies = &pClipMap->triIndices[3 * (*index_hint - pClipMap->numBrushes)];
            v11 = pClipMap->verts[*indicies];
            v0[0] = *v11;
            v0[1] = v11[1];
            v0[2] = v11[2];
            v10 = pClipMap->verts[indicies[1]];
            v1[0] = *v10;
            v1[1] = v10[1];
            v1[2] = v10[2];
            v9 = pClipMap->verts[indicies[2]];
            v2[0] = *v9;
            v2[1] = v9[1];
            v2[2] = v9[2];
            v0_v1[0] = v0[0] - v1[0];
            v0_v1[1] = v0[1] - v1[1];
            v0_v1[2] = v0[2] - v1[2];
            v0_v2[0] = v0[0] - v2[0];
            v0_v2[1] = v0[1] - v2[1];
            v0_v2[2] = v0[2] - v2[2];
            Vec3Cross(v0_v2, v0_v1, n);
            Vec3Normalize(n);
            if ( trace_sphere_through_triangle(start, dir, &trace->fraction, radius, v0, v1, v2, n) )
            {
                *(_QWORD *)trace->normal.vec.v = *(_QWORD *)n;
                trace->normal.vec.v[2] = n[2];
                if ( !trace
                    && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_public.h", 175, 0, "%s", "trace") )
                {
                    __debugbreak();
                }
                trace->hitType = TRACE_HITTYPE_ENTITY;
                trace->hitId = 1022;
                bHintFound = 1;
            }
        }
        else
        {
            trace_sphere_through_brush(
                start,
                dir,
                &trace->fraction,
                radius,
                &pClipMap->brushes[*index_hint],
                trace->normal.vec.v,
                &trace->sflags,
                NULL);
            if ( trace->fraction < 1.0 )
            {
                if ( !trace
                    && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_public.h", 175, 0, "%s", "trace") )
                {
                    __debugbreak();
                }
                trace->hitType = TRACE_HITTYPE_ENTITY;
                trace->hitId = 1022;
                bHintFound = 1;
            }
        }
    }
    if ( !bHintFound )
        *index_hint = -1;
    boxSize[0] = radius;
    boxSize[1] = radius;
    boxSize[2] = radius;
    Vec3Min(start, end, bounds[0]);
    bounds[0][0] = bounds[0][0] - boxSize[0];
    bounds[0][1] = bounds[0][1] - boxSize[1];
    bounds[0][2] = bounds[0][2] - boxSize[2];
    Vec3Max(start, end, bounds[1]);
    bounds[1][0] = bounds[1][0] + boxSize[0];
    bounds[1][1] = bounds[1][1] + boxSize[1];
    bounds[1][2] = bounds[1][2] + boxSize[2];
    prim = context->prims;
    for ( i = 0; i < context->nprims; ++i )
    {
        if ( prim->type )
        {
            brush = prim->brush;
            if ( brush->maxs[0] >= bounds[0][0]
                && brush->maxs[1] >= bounds[0][1]
                && brush->maxs[2] >= bounds[0][2]
                && bounds[1][0] >= brush->mins[0]
                && bounds[1][1] >= brush->mins[1]
                && bounds[1][2] >= brush->mins[2] )
            {
                fraction = trace->fraction;
                trace_sphere_through_brush(start, dir, &trace->fraction, radius, brush, trace->normal.vec.v, &trace->sflags, NULL);
                if ( fraction > trace->fraction )
                    *index_hint = ((char *)prim->tree - (char *)pClipMap->brushes) / 96;
            }
        }
        else
        {
            tree = prim->tree;
            mn[0] = tree->origin[0] - tree->halfSize[0];
            mn[1] = tree->origin[1] - tree->halfSize[1];
            mn[2] = tree->origin[2] - tree->halfSize[2];
            mx[0] = tree->origin[0] + tree->halfSize[0];
            mx[1] = tree->origin[1] + tree->halfSize[1];
            mx[2] = tree->origin[2] + tree->halfSize[2];
            if ( mx[0] >= bounds[0][0]
                && mx[1] >= bounds[0][1]
                && mx[2] >= bounds[0][2]
                && bounds[1][0] >= mn[0]
                && bounds[1][1] >= mn[1]
                && bounds[1][2] >= mn[2] )
            {
                old_fraction = trace->fraction;
                trace_sphere_through_partition(
                    start,
                    dir,
                    &trace->fraction,
                    radius,
                    &pClipMap->partitions[tree->u.firstChildIndex],
                    trace->normal.vec.v,
                    index_hint,
                    pClipMap);
                if ( old_fraction > trace->fraction )
                    trace->sflags = pClipMap->materials[tree->materialIndex].surfaceFlags;
            }
        }
        ++prim;
    }
    if ( trace->fraction >= 1.0 )
    {
        if ( !trace && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_public.h", 175, 0, "%s", "trace") )
            __debugbreak();
        trace->hitType = TRACE_HITTYPE_NONE;
        trace->hitId = 0;
    }
    else
    {
        if ( !trace && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_public.h", 175, 0, "%s", "trace") )
            __debugbreak();
        trace->hitType = TRACE_HITTYPE_ENTITY;
        trace->hitId = 1022;
    }
}

void __cdecl trace_sphere_vs_env(
                trace_t *trace,
                const float *start,
                const float *end,
                float radius,
                col_context_t *context,
                int *index_hint,
                unsigned __int8 *pClipMap)
{
    trace_sphere_vs_env(trace, start, end, radius, context, index_hint, (clipMap_t *)pClipMap);
}

int __cdecl collide_segment_brush(const float *p0, const float *p1, const cbrush_t *brush)
{
    float v4; // [esp+4h] [ebp-80h]
    float v5; // [esp+8h] [ebp-7Ch]
    float v6; // [esp+Ch] [ebp-78h]
    float diff; // [esp+44h] [ebp-40h]
    float d1; // [esp+4Ch] [ebp-38h]
    float d1a; // [esp+4Ch] [ebp-38h]
    cbrushside_t *side; // [esp+50h] [ebp-34h]
    int j; // [esp+54h] [ebp-30h]
    const cplane_s *plane; // [esp+58h] [ebp-2Ch]
    float enterFrac; // [esp+5Ch] [ebp-28h]
    float delta; // [esp+60h] [ebp-24h]
    float deltaa; // [esp+60h] [ebp-24h]
    float frac; // [esp+64h] [ebp-20h]
    float fraca; // [esp+64h] [ebp-20h]
    float dist; // [esp+68h] [ebp-1Ch]
    float sign; // [esp+6Ch] [ebp-18h]
    float d2; // [esp+70h] [ebp-14h]
    float d2a; // [esp+70h] [ebp-14h]
    const cbrush_t *bounds; // [esp+74h] [ebp-10h]
    int index; // [esp+78h] [ebp-Ch]
    float leaveFrac; // [esp+7Ch] [ebp-8h]
    signed int i; // [esp+80h] [ebp-4h]

    enterFrac = 0.0f;
    leaveFrac = 1.0f;
    sign = -1.0f;
    bounds = brush;
    for ( index = 0; ; index = 1 )
    {
        if ( ((LODWORD(bounds->mins[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(bounds->mins[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(bounds->mins[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        4117,
                        0,
                        "%s",
                        "!IS_NAN((bounds)[0]) && !IS_NAN((bounds)[1]) && !IS_NAN((bounds)[2])") )
        {
            __debugbreak();
        }
        for ( j = 0; j < 3; ++j )
        {
            d1 = (float)(p0[j] - bounds->mins[j]) * sign;
            d2 = (float)(p1[j] - bounds->mins[j]) * sign;
            if ( (LODWORD(d1) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 4122, 0, "%s", "!IS_NAN(d1)") )
            {
                __debugbreak();
            }
            if ( (LODWORD(d2) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 4123, 0, "%s", "!IS_NAN(d2)") )
            {
                __debugbreak();
            }
            diff = p0[j] - p1[j];
            if ( diff == 0.0 )
                v6 = 0.0f;
            else
                v6 = 1.0 / diff;
            if ( d1 <= 0.0 )
            {
                if ( d2 > 0.0 )
                {
                    fraca = (float)(d1 * v6) * sign;
                    if ( enterFrac >= fraca )
                        return 0;
                    if ( (float)(fraca - leaveFrac) < 0.0 )
                        v4 = (float)(d1 * v6) * sign;
                    else
                        v4 = leaveFrac;
                    leaveFrac = v4;
                }
            }
            else
            {
                if ( d2 > 0.0 )
                    return 0;
                frac = (float)(d1 * v6) * sign;
                if ( frac >= leaveFrac )
                    return 0;
                if ( (float)(enterFrac - frac) < 0.0 )
                    v5 = (float)(d1 * v6) * sign;
                else
                    v5 = enterFrac;
                enterFrac = v5;
            }
        }
        if ( index )
            break;
        sign = 1.0f;
        bounds = (const cbrush_t *)brush->maxs;
    }
    i = brush->numsides;
    side = brush->sides;
    if ( i < 0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 4156, 0, "%s", "i >= 0") )
        __debugbreak();
    while ( i )
    {
        plane = side->plane;
        if ( (LODWORD(side->plane->dist) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        4161,
                        0,
                        "%s",
                        "!IS_NAN(plane->dist)") )
        {
            __debugbreak();
        }
        if ( ((LODWORD(plane->normal[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(plane->normal[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(plane->normal[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
                        4164,
                        0,
                        "%s",
                        "!IS_NAN((plane->normal)[0]) && !IS_NAN((plane->normal)[1]) && !IS_NAN((plane->normal)[2])") )
        {
            __debugbreak();
        }
        dist = plane->dist;
        if ( (LODWORD(dist) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 4166, 0, "%s", "!IS_NAN(dist)") )
        {
            __debugbreak();
        }
        d1a = (float)((float)((float)(*p0 * plane->normal[0]) + (float)(p0[1] * plane->normal[1]))
                                + (float)(p0[2] * plane->normal[2]))
                - dist;
        d2a = (float)((float)((float)(*p1 * plane->normal[0]) + (float)(p1[1] * plane->normal[1]))
                                + (float)(p1[2] * plane->normal[2]))
                - dist;
        if ( (LODWORD(d1a) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 4171, 0, "%s", "!IS_NAN(d1)") )
        {
            __debugbreak();
        }
        if ( (LODWORD(d2a) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 4172, 0, "%s", "!IS_NAN(d2)") )
        {
            __debugbreak();
        }
        if ( d1a <= 0.0 )
        {
            if ( d2a > 0.0 )
            {
                deltaa = d1a - d2a;
                if ( (float)(d1a - d2a) >= 0.0
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 4191, 0, "%s", "delta < 0") )
                {
                    __debugbreak();
                }
                if ( d1a > (float)(leaveFrac * deltaa) )
                {
                    leaveFrac = d1a / deltaa;
                    if ( enterFrac >= (float)(d1a / deltaa) )
                        return 0;
                }
            }
        }
        else
        {
            delta = d1a - d2a;
            iassert(!IS_NAN(delta));
            //if ( (COERCE_UNSIGNED_INT(d1a - d2a) & 0x7F800000) == 0x7F800000
            //    && !Assert_MyHandler(
            //                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp",
            //                4177,
            //                0,
            //                "%s",
            //                "!IS_NAN(delta)") )
            //{
            //    __debugbreak();
            //}
            if ( d2a > 0.0 )
                return 0;
            if ( delta <= 0.0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 4180, 0, "%s", "delta > 0") )
            {
                __debugbreak();
            }
            if ( d1a > (float)(enterFrac * delta) )
            {
                enterFrac = d1a / delta;
                if ( (float)(d1a / delta) >= leaveFrac )
                    return 0;
            }
        }
        --i;
        ++side;
    }
    return 1;
}

bool __cdecl collide_segment_triangle(
                const float *p0,
                const float *u0,
                const float *v0,
                const float *v1,
                const float *v2)
{
    float b_; // [esp+0h] [ebp-58h]
    float det; // [esp+4h] [ebp-54h]
    float c2[3]; // [esp+8h] [ebp-50h] BYREF
    float c1[3]; // [esp+14h] [ebp-44h] BYREF
    float rs[3]; // [esp+20h] [ebp-38h] BYREF
    float cr_rs_c2[3]; // [esp+2Ch] [ebp-2Ch] BYREF
    float a_; // [esp+38h] [ebp-20h]
    float c0[3]; // [esp+3Ch] [ebp-1Ch] BYREF
    float cr_c0_c1[3]; // [esp+48h] [ebp-10h] BYREF
    float t_; // [esp+54h] [ebp-4h]

    c0[0] = *v1 - *v0;
    c0[1] = v1[1] - v0[1];
    c0[2] = v1[2] - v0[2];
    c1[0] = *v2 - *v0;
    c1[1] = v2[1] - v0[1];
    c1[2] = v2[2] - v0[2];
    //LODWORD(c2[0]) = *(unsigned int *)u0 ^ _mask__NegFloat_;
    //LODWORD(c2[1]) = *((unsigned int *)u0 + 1) ^ _mask__NegFloat_;
    //LODWORD(c2[2]) = *((unsigned int *)u0 + 2) ^ _mask__NegFloat_;
    c2[0] = -u0[0];
    c2[1] = -u0[1];
    c2[2] = -u0[2];
    Vec3Cross(c0, c1, cr_c0_c1);
    det = (float)((float)(cr_c0_c1[0] * c2[0]) + (float)(cr_c0_c1[1] * c2[1])) + (float)(cr_c0_c1[2] * c2[2]);
    if ( fabs(det) < 0.000099999997 )
        return 0;
    rs[0] = *p0 - *v0;
    rs[1] = p0[1] - v0[1];
    rs[2] = p0[2] - v0[2];
    t_ = (float)((float)((float)(cr_c0_c1[0] * rs[0]) + (float)(cr_c0_c1[1] * rs[1])) + (float)(cr_c0_c1[2] * rs[2]))
         / det;
    if ( t_ < 0.0 || t_ > 1.0 )
        return 0;
    Vec3Cross(rs, c2, cr_rs_c2);
    //a_ = COERCE_FLOAT(COERCE_UNSIGNED_INT((float)((float)(cr_rs_c2[0] * c1[0]) + (float)(cr_rs_c2[1] * c1[1])) + (float)(cr_rs_c2[2] * c1[2])) ^ _mask__NegFloat_) / det;
    a_ = -(((float)((float)(cr_rs_c2[0] * c1[0]) + (float)(cr_rs_c2[1] * c1[1])) + (float)(cr_rs_c2[2] * c1[2]))) / det;
    if ( a_ < 0.0 )
        return 0;
    b_ = (float)((float)((float)(cr_rs_c2[0] * c0[0]) + (float)(cr_rs_c2[1] * c0[1])) + (float)(cr_rs_c2[2] * c0[2]))
         / det;
    return b_ >= 0.0 && (float)(a_ + b_) <= 1.0;
}

int __cdecl collide_segment(const float *p0, const float *p1, col_context_t *context)
{
    float *v5; // [esp+10h] [ebp-B4h]
    float *v6; // [esp+14h] [ebp-B0h]
    float *v7; // [esp+18h] [ebp-ACh]
    const float *v8; // [esp+20h] [ebp-A4h]
    const CollisionAabbTree *v9; // [esp+24h] [ebp-A0h]
    const float *halfSize; // [esp+28h] [ebp-9Ch]
    const CollisionAabbTree *tree; // [esp+2Ch] [ebp-98h]
    int ti; // [esp+30h] [ebp-94h]
    const CollisionPartition *partition; // [esp+34h] [ebp-90h]
    int i; // [esp+38h] [ebp-8Ch]
    float v0[3]; // [esp+3Ch] [ebp-88h] BYREF
    float ud[3]; // [esp+48h] [ebp-7Ch] BYREF
    float v0_v2[3]; // [esp+54h] [ebp-70h] BYREF
    const col_prim_t *prim; // [esp+60h] [ebp-64h]
    float mx[3]; // [esp+64h] [ebp-60h]
    float v2[3]; // [esp+70h] [ebp-54h] BYREF
    float bounds[2][3]; // [esp+7Ch] [ebp-48h] BYREF
    float v1[3]; // [esp+94h] [ebp-30h] BYREF
    float n[3]; // [esp+A0h] [ebp-24h] BYREF
    float mn[3]; // [esp+ACh] [ebp-18h]
    float v0_v1[3]; // [esp+B8h] [ebp-Ch] BYREF

    prim = context->prims;
    ud[0] = *p1 - *p0;
    ud[1] = p1[1] - p0[1];
    ud[2] = p1[2] - p0[2];
    Vec3Min(p0, p1, bounds[0]);
    Vec3Max(p0, p1, bounds[1]);
    for ( i = 0; i < context->nprims; ++i )
    {
        if ( prim->type )
        {
            if ( collide_segment_brush(p0, p1, prim->brush) )
                return 1;
        }
        else
        {
            halfSize = prim->tree->halfSize;
            tree = prim->tree;
            mn[0] = tree->origin[0] - *halfSize;
            mn[1] = tree->origin[1] - halfSize[1];
            mn[2] = tree->origin[2] - halfSize[2];
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
                partition = &cm.partitions[prim->tree->u.firstChildIndex];
                for ( ti = partition->firstTri; ti < partition->firstTri + partition->triCount; ++ti )
                {
                    v7 = cm.verts[cm.triIndices[3 * ti]];
                    v0[0] = *v7;
                    v0[1] = v7[1];
                    v0[2] = v7[2];
                    v6 = cm.verts[cm.triIndices[3 * ti + 1]];
                    v1[0] = *v6;
                    v1[1] = v6[1];
                    v1[2] = v6[2];
                    v5 = cm.verts[cm.triIndices[3 * ti + 2]];
                    v2[0] = *v5;
                    v2[1] = v5[1];
                    v2[2] = v5[2];
                    v0_v1[0] = v0[0] - v1[0];
                    v0_v1[1] = v0[1] - v1[1];
                    v0_v1[2] = v0[2] - v1[2];
                    v0_v2[0] = v0[0] - v2[0];
                    v0_v2[1] = v0[1] - v2[1];
                    v0_v2[2] = v0[2] - v2[2];
                    Vec3Cross(v0_v2, v0_v1, n);
                    Vec3Normalize(n);
                    if ( collide_segment_triangle(p0, ud, v0, v1, v2) )
                        return 1;
                }
            }
        }
        ++prim;
    }
    return 0;
}

char __cdecl CM_TestPointInBrushModel(const float *point, unsigned int brushmodel, const float *trans)
{
    float *normal; // ecx
    unsigned int i; // [esp+Ch] [ebp-28h]
    const cbrush_t *brush; // [esp+18h] [ebp-1Ch]
    float p; // [esp+28h] [ebp-Ch]
    float p_4; // [esp+2Ch] [ebp-8h]
    float p_8; // [esp+30h] [ebp-4h]

    brush = &cm.brushes[*cm.leafbrushNodes[CM_ClipHandleToModel(brushmodel)->leaf.leafBrushNode].data.leaf.brushes];
    p = *point - *trans;
    p_4 = point[1] - trans[1];
    p_8 = point[2] - trans[2];
    if ( p >= brush->maxs[0] || p_4 >= brush->maxs[1] || p_8 >= brush->maxs[2] )
        return 0;
    if ( brush->mins[0] >= p || brush->mins[1] >= p_4 || brush->mins[2] >= p_8 )
        return 0;
    for ( i = 0; i < brush->numsides; ++i )
    {
        normal = brush->sides[i].plane->normal;
        if ( (float)((float)((float)(p * *normal) + (float)(p_4 * normal[1])) + (float)(p_8 * normal[2])) > normal[3] )
            return 0;
    }
    return 1;
}

char __cdecl CM_GetWaterForce(const float *pt, float *dir, float *force)
{
    float *v4; // [esp+0h] [ebp-Ch]
    int i; // [esp+8h] [ebp-4h]

    if ( !pt && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 4316, 0, "%s", "pt") )
        __debugbreak();
    if ( !dir && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 4317, 0, "%s", "dir") )
        __debugbreak();
    if ( !force && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 4318, 0, "%s", "force") )
        __debugbreak();
    for ( i = 0; i < num_undertow_volumes; ++i )
    {
        if ( CM_TestPointInBrushModel(pt, undertow_volumes[i], undertow_origins[i]) )
        {
            //v4 = undertow_forces[i];
            //*dir = *v4;
            //dir[1] = v4[1];
            //dir[2] = v4[2];

            dir[0] = undertow_forces[i][0];
            dir[1] = undertow_forces[i][1];
            dir[2] = undertow_forces[i][2];
            *force = undertow_forces[i][3];

            return 1;
        }
    }
    return 0;
}

char __cdecl CM_GetHeliHeight(const float *pt, float checkdist, float *result)
{
    unsigned int brushmodel; // [esp+8h] [ebp-68h]
    trace_t results; // [esp+Ch] [ebp-64h] BYREF
    int i; // [esp+48h] [ebp-28h]
    float helipt[3]; // [esp+4Ch] [ebp-24h] BYREF
    float start[3]; // [esp+58h] [ebp-18h] BYREF
    float end[3]; // [esp+64h] [ebp-Ch] BYREF

    if (!result && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_trace.cpp", 4340, 0, "%s", "result"))
        __debugbreak();
    start[0] = *pt;
    start[1] = pt[1];
    start[2] = pt[2];
    end[0] = *pt;
    end[1] = pt[1];
    end[2] = pt[2];
    start[2] = start[2] + checkdist;
    end[2] = end[2] - checkdist;
    for (i = 0; i < num_heli_height_lock_patches; ++i)
    {
        if (heli_height_lock_patches[i].enabled)
        {
            brushmodel = heli_height_lock_patches[i].brushmodel;
            memset(&results, 0, sizeof(results));
            results.fraction = 1.0f;
            results.boneIndex = 254;
            CM_TransformedBoxTrace(
                &results,
                start,
                end,
                vec3_origin,
                vec3_origin,
                brushmodel,
                -1,
                heli_height_lock_patches[i].origin,
                vec3_origin);
            if (results.fraction != 1.0)
            {
                Vec3Lerp(start, end, results.fraction, helipt);
                *result = helipt[2];
                return 1;
            }
        }
    }
    return 0;
}
