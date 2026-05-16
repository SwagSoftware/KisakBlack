#include "cg_world.h"
#include <qcommon/dobj_management.h>
#include <cgame_mp/cg_local_mp.h>
#include "cg_colltree.h"
#include <universal/com_math_anglevectors.h>
#include <cgame_mp/cg_ents_mp.h>
#include <bgame/bg_slidemove.h>
#include <qcommon/cm_load.h>
#include <qcommon/cm_tracebox.h>
#include <cgame_mp/cg_pose_mp.h>
#include <glass/glass_client.h>
#include <bgame/bg_misc.h>
#include <qcommon/cm_world.h>
#include <game_mp/g_combat_mp.h>
#include <physics/rope.h>
#include <xanim/xmodel.h>
#include <xanim/dobj_utils.h>
#include <clientscript/cscr_stringlist.h>

float actorLocationalMinsBig[3] = { -54.0, -54.0, -32.0 };
float actorLocationalMaxsBig[3] = { 54.0, 54.0, 72.0 };
float actorLocationalMins[3]    = { -36.0, -36.0, -10.0 };
float actorLocationalMaxs[3]    = { 36.0, 36.0, 72.0 };

int cgCollWorldLocalClientNum;

int __cdecl CG_GetEntityBModelContents(const centity_s *cent)
{
    int eType; // [esp+0h] [ebp-8h]

    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 42, 0, "%s", "cent") )
        __debugbreak();
    if ( !cent->nextState.solid
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 45, 0, "%s", "es->solid") )
    {
        __debugbreak();
    }
    if (cent->nextState.solid == 0xFFFFFF)
        return CM_ContentsOfModel(cent->nextState.index.brushmodel);
    eType = cent->nextState.eType;
    switch ( eType )
    {
        case 1:
            return 0x2000000;
        case 17:
            return 0x8000;
        case 19:
            return 0x4000000;
    }
    return 1;
}
void CG_GetEntityBModelBounds(
    const centity_s *cent,
    float *mins,
    float *maxs,
    float *absMins,
    float *absMaxs)
{
    const entityState_s *es;

    iassert(cent);
    iassert(mins);
    iassert(maxs);

    es = &cent->nextState;
    iassert(es->solid);

    if (es->solid == 0xFFFFFF)
    {
        CM_ModelBounds(es->index.brushmodel, mins, maxs);
    }
    else
    {
        int x = (uint8_t)(es->solid);
        int zd = (uint8_t)(es->solid >> 8) - 1;
        int zu = (uint8_t)(es->solid >> 16) - 32;

        mins[0] = 1.0f - x;
        mins[1] = 1.0f - x;
        mins[2] = 1.0f - zd;

        maxs[0] = x - 1.0f;
        maxs[1] = x - 1.0f;
        maxs[2] = zu - 1.0f;
    }

    // aislop to cleanup stack
    if (!absMins || !absMaxs)
        return;

    phys_vec3 localMin, localMax;
    phys_vec3 center, halfExtents;
    phys_mat44 axis;
    float axis3x3[3][3];

    /* Convert to phys vectors */
    Phys_Vec3ToNitrousVec(mins, &localMin);
    Phys_Vec3ToNitrousVec(maxs, &localMax);

    /* Half-extents */
    halfExtents.x = (localMax.x - localMin.x) * 0.5f;
    halfExtents.y = (localMax.y - localMin.y) * 0.5f;
    halfExtents.z = (localMax.z - localMin.z) * 0.5f;

    /* Center */
    center.x = (localMax.x + localMin.x) * 0.5f;
    center.y = (localMax.y + localMin.y) * 0.5f;
    center.z = (localMax.z + localMin.z) * 0.5f;

    /* Orientation */
    AnglesToAxis(cent->pose.angles, axis3x3);
    Phys_AxisToNitrousMat(axis3x3, &axis);

    /* Translation */
    Phys_Vec3ToNitrousVec(cent->pose.origin, &axis.w);

    /* Compute world AABB */
    phys_calc_world_aabb(
        &center,
        &halfExtents,
        &axis,
        &localMin,
        &localMax);

    Phys_NitrousVecToVec3(&localMin, absMins);
    Phys_NitrousVecToVec3(&localMax, absMaxs);
}



void __cdecl CG_GetEntityDobjBounds(
                const centity_s *cent,
                const DObj *dobj,
                float *mins,
                float *maxs,
                float *absMins,
                float *absMaxs)
{
    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 145, 0, "%s", "cent") )
        __debugbreak();
    if ( !dobj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 146, 0, "%s", "dobj") )
        __debugbreak();
    if ( !mins && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 147, 0, "%s", "mins") )
        __debugbreak();
    if ( !maxs && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 148, 0, "%s", "maxs") )
        __debugbreak();
    if ( !absMins && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 149, 0, "%s", "absMins") )
        __debugbreak();
    if ( !absMaxs && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 150, 0, "%s", "absMaxs") )
        __debugbreak();
    DObjGetBounds(dobj, mins, maxs);
    *absMins = cent->pose.origin[0] + *mins;
    absMins[1] = cent->pose.origin[1] + mins[1];
    absMins[2] = cent->pose.origin[2] + mins[2];
    *absMaxs = cent->pose.origin[0] + *maxs;
    absMaxs[1] = cent->pose.origin[1] + maxs[1];
    absMaxs[2] = cent->pose.origin[2] + maxs[2];
}

unsigned int __cdecl CG_GetEntityDObjContents(const centity_s *cent, const DObj *dobj)
{
    int contents; // [esp+4h] [ebp-4h]

    contents = DObjGetContents(dobj);
    switch (cent->nextState.eType)
    {
    case 1:
    case 2:
        return contents | 0x2000000;
    case 0xE:
    case 0x10:
        return contents | 0x800000;
    case 0x11:
    case 0x13:
        return contents | 0x8000;
    }
    DObjHasCollmap(dobj);
    return contents;
}

bool __cdecl CG_EntityNeedsLinked(int localClientNum, unsigned int entIndex)
{
    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp",
                    188,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    if ( entIndex >= 0x600
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp",
                    189,
                    0,
                    "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                    entIndex,
                    1536) )
    {
        __debugbreak();
    }
    return CG_GetEntity(localClientNum, entIndex)->nextState.solid || Com_GetClientDObj(entIndex, localClientNum) != 0;
}

void __cdecl CG_UnlinkEntity(int localClientNum, unsigned int entIndex)
{
    CG_UnlinkEntityColl(localClientNum, entIndex);
}

void __cdecl CG_CalcWorldBounds(centity_s *cent, DObj *dobj)
{
    float v2; // [esp+0h] [ebp-B0h]
    float v3; // [esp+4h] [ebp-ACh]
    float v4; // [esp+8h] [ebp-A8h]
    BOOL v5; // [esp+Ch] [ebp-A4h]
    int k; // [esp+50h] [ebp-60h]
    float xform[3][3]; // [esp+54h] [ebp-5Ch] BYREF
    float mins[3]; // [esp+78h] [ebp-38h] BYREF
    int hasCollMap; // [esp+84h] [ebp-2Ch]
    int character; // [esp+88h] [ebp-28h]
    float maxs[3]; // [esp+8Ch] [ebp-24h] BYREF
    float xformed_v[3]; // [esp+98h] [ebp-18h] BYREF
    float v[3]; // [esp+A4h] [ebp-Ch] BYREF

    hasCollMap = DObjHasCollmap(dobj);
    character = 0;
    character = cent->nextState.eType == 1 || cent->nextState.eType == 2;
    v5 = cent->nextState.surfType == 7 && !hasCollMap;
    character |= v5 || cent->nextState.eType == 17 || cent->nextState.eType == 19;
    if (cent->pose.isRagdoll)
    {
        cent->pose.absmin[0] = cent->pose.origin[0] + actorLocationalMinsBig[0];
        cent->pose.absmin[1] = cent->pose.origin[1] + actorLocationalMinsBig[1];
        cent->pose.absmin[2] = cent->pose.origin[2] + actorLocationalMinsBig[2];
        cent->pose.absmax[0] = cent->pose.origin[0] + actorLocationalMaxsBig[0];
        cent->pose.absmax[1] = cent->pose.origin[1] + actorLocationalMaxsBig[1];
        cent->pose.absmax[2] = cent->pose.origin[2] + actorLocationalMaxsBig[2];
    }
    else if (character)
    {
        cent->pose.absmin[0] = cent->pose.origin[0] + actorLocationalMins[0];
        cent->pose.absmin[1] = cent->pose.origin[1] + actorLocationalMins[1];
        cent->pose.absmin[2] = cent->pose.origin[2] + actorLocationalMins[2];
        cent->pose.absmax[0] = cent->pose.origin[0] + actorLocationalMaxs[0];
        cent->pose.absmax[1] = cent->pose.origin[1] + actorLocationalMaxs[1];
        cent->pose.absmax[2] = cent->pose.origin[2] + actorLocationalMaxs[2];
    }
    else
    {
        AnglesToAxis(cent->pose.angles, xform);
        DObjCalcBounds(dobj, mins, maxs);
        cent->pose.absmin[0] = FLT_MAX;
        cent->pose.absmin[1] = FLT_MAX;
        cent->pose.absmin[2] = FLT_MAX;
        cent->pose.absmax[0] = -FLT_MAX;
        cent->pose.absmax[1] = -FLT_MAX;
        cent->pose.absmax[2] = -FLT_MAX;
        for (k = 0; k < 8; ++k)
        {
            if ((k & 1) != 0)
                v4 = mins[0];
            else
                v4 = maxs[0];
            v[0] = v4;
            if ((k & 2) != 0)
                v3 = mins[1];
            else
                v3 = maxs[1];
            v[1] = v3;
            if ((k & 4) != 0)
                v2 = mins[2];
            else
                v2 = maxs[2];
            v[2] = v2;
            MatrixTransformVector(v, xform, xformed_v);
            Vec3Min(cent->pose.absmin, xformed_v, cent->pose.absmin);
            Vec3Max(cent->pose.absmax, xformed_v, cent->pose.absmax);
        }
        cent->pose.absmin[0] = cent->pose.absmin[0] + cent->pose.origin[0];
        cent->pose.absmin[1] = cent->pose.absmin[1] + cent->pose.origin[1];
        cent->pose.absmin[2] = cent->pose.absmin[2] + cent->pose.origin[2];
        cent->pose.absmax[0] = cent->pose.absmax[0] + cent->pose.origin[0];
        cent->pose.absmax[1] = cent->pose.absmax[1] + cent->pose.origin[1];
        cent->pose.absmax[2] = cent->pose.absmax[2] + cent->pose.origin[2];
    }
}

void __cdecl CG_LinkEntity(int localClientNum, unsigned int entIndex)
{
    DObj *dobj; // [esp+8h] [ebp-24h]
    float mins[3]; // [esp+Ch] [ebp-20h] BYREF
    centity_s *cent; // [esp+18h] [ebp-14h]
    const entityState_s *p_nextState; // [esp+1Ch] [ebp-10h]
    float maxs[3]; // [esp+20h] [ebp-Ch] BYREF
    int savedregs; // [esp+2Ch] [ebp+0h] BYREF

    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp",
                    270,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    if ( entIndex >= 0x600
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp",
                    271,
                    0,
                    "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                    entIndex,
                    1536) )
    {
        __debugbreak();
    }
    cent = CG_GetEntity(localClientNum, entIndex);
    p_nextState = &cent->nextState;
    dobj = Com_GetClientDObj(entIndex, localClientNum);
    if ( dobj )
    {
        CG_CalcWorldBounds(cent, dobj);
        CG_LinkEntityColl(localClientNum, entIndex, cent->pose.absmin, cent->pose.absmax);
    }
    else if ( p_nextState->solid )
    {
        CG_GetEntityBModelBounds(cent, mins, maxs, cent->pose.absmin, cent->pose.absmax);
        CG_LinkEntityColl(localClientNum, entIndex, cent->pose.absmin, cent->pose.absmax);
    }
    else
    {
        CG_UnlinkEntity(localClientNum, entIndex);
    }
}

void __cdecl CG_PointTraceToEntity(const pointtrace_t *clip, unsigned int entIndex, trace_t *results)
{
    const char *v3; // eax
    const char *v4; // eax
    DObj *v5; // [esp+4h] [ebp-1E8h]
    bool v6; // [esp+Ch] [ebp-1E0h]
    DObj *ClientDObj; // [esp+10h] [ebp-1DCh]
    unsigned __int16 number; // [esp+1Eh] [ebp-1CEh]
    char *v9; // [esp+B8h] [ebp-134h]
    char *Name; // [esp+C4h] [ebp-128h]
    PhysGeomList *geoms; // [esp+118h] [ebp-D4h]
    float angles[3]; // [esp+124h] [ebp-C8h] BYREF
    unsigned int cmodel; // [esp+130h] [ebp-BCh]
    float oldFraction; // [esp+134h] [ebp-B8h]
    DObj *dobj; // [esp+13Ch] [ebp-B0h]
    float localStart[3]; // [esp+144h] [ebp-A8h] BYREF
    cg_s *cgameGlob; // [esp+150h] [ebp-9Ch]
    DObjTrace_s objTrace; // [esp+154h] [ebp-98h] BYREF
    float localEnd[3]; // [esp+174h] [ebp-78h] BYREF
    bool character; // [esp+183h] [ebp-69h]
    int hasCollMap; // [esp+184h] [ebp-68h]
    const IgnoreEntParams *ignoreEntParams; // [esp+188h] [ebp-64h]
    int contents; // [esp+18Ch] [ebp-60h]
    DObj *dobjEA; // [esp+190h] [ebp-5Ch]
    float mins[3]; // [esp+198h] [ebp-54h] BYREF
    float absMaxs[3]; // [esp+1A4h] [ebp-48h] BYREF
    int localClientNum; // [esp+1B0h] [ebp-3Ch]
    const centity_s *centEA; // [esp+1B4h] [ebp-38h]
    const centity_s *cent; // [esp+1B8h] [ebp-34h]
    const entityState_s *p_nextState; // [esp+1BCh] [ebp-30h]
    float maxs[3]; // [esp+1C0h] [ebp-2Ch] BYREF
    int partBits[5]; // [esp+1CCh] [ebp-20h] BYREF
    float absMins[3]; // [esp+1E0h] [ebp-Ch] BYREF

    PROF_SCOPED("CG_PointTraceToEntity");

    if ( !clip && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 573, 0, "%s", "clip") )
        __debugbreak();
    if ( entIndex >= 0x600
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp",
                    574,
                    0,
                    "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                    entIndex,
                    1536) )
    {
        __debugbreak();
    }
    if ( !results && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 575, 0, "%s", "results") )
        __debugbreak();
    contents = 0;
    localClientNum = cgCollWorldLocalClientNum;
    centEA = CG_GetEntity(cgCollWorldLocalClientNum, entIndex);
    cent = centEA;
    if ( clip->ignoreEntParams )
    {
        ignoreEntParams = clip->ignoreEntParams;
        if ( ignoreEntParams->baseEntity != 1023 )
        {
            if ( ignoreEntParams->ignoreSelf && entIndex == ignoreEntParams->baseEntity )
            {
                return;
            }
            if ( ignoreEntParams->ignoreParent && entIndex == ignoreEntParams->parentEntity )
            {
                return;
            }
        }
    }
    p_nextState = &cent->nextState;
    if ( clip->ignoreEntParams
        && entIndex == clip->ignoreEntParams->baseEntity
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp",
                    602,
                    0,
                    "%s",
                    "!clip->ignoreEntParams || entIndex != clip->ignoreEntParams->baseEntity") )
    {
        __debugbreak();
    }
    if ( clip->bLocational )
        ClientDObj = Com_GetClientDObj(entIndex, localClientNum);
    else
        ClientDObj = 0;
    dobjEA = ClientDObj;
    if ( p_nextState->solid || dobjEA )
    {
        if ( !dobjEA )
        {
            PROF_SCOPED("non_dobj");

            if ( !p_nextState->solid
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 691, 0, "%s", "es->solid") )
            {
                __debugbreak();
            }
            if (p_nextState->solid == 0xFFFFFF && (p_nextState->lerp.eFlags & 1) != 0)
            {
                return;
            }
            contents = CG_GetEntityBModelContents(cent);
            if ( (clip->contentmask & contents) == 0 )
            {
                return;
            }
            if ( !intersect_extents_aabb(&clip->extents, cent->pose.absmin, cent->pose.absmax, results->fraction) )
            {
                return;
            }
            if (p_nextState->solid == 0xFFFFFF)
            {
                cmodel = p_nextState->index.brushmodel;
                angles[0] = cent->pose.angles[0];
                angles[1] = cent->pose.angles[1];
                angles[2] = cent->pose.angles[2];
            }
            else
            {
                if ( cent->nextState.eType == 6 || cent->nextState.eType == 14 )
                    v5 = Com_GetClientDObj(entIndex, localClientNum);
                else
                    v5 = 0;
                if ( v5 )
                {
                    if ( !DObjHasCollmap(v5) )
                    {
                        return;
                    }
                    geoms = DObjGetCollmap(v5);
                    if ( !geoms )
                    {
                        return;
                    }
                    cmodel = CM_TempBrushModel(geoms);
                    angles[0] = cent->pose.angles[0];
                    angles[1] = cent->pose.angles[1];
                    angles[2] = cent->pose.angles[2];
                }
                else
                {
                    absMins[0] = cent->pose.absmin[0];
                    absMins[1] = cent->pose.absmin[1];
                    absMins[2] = cent->pose.absmin[2];
                    absMaxs[0] = cent->pose.absmax[0];
                    absMaxs[1] = cent->pose.absmax[1];
                    absMaxs[2] = cent->pose.absmax[2];
                    CG_GetEntityBModelBounds(cent, mins, maxs, absMins, absMaxs);
                    cmodel = CM_TempBoxModel(mins, maxs, contents);
                    memset(angles, 0, sizeof(angles));
                }
            }
            oldFraction = results->fraction;
            CM_TransformedBoxTrace(
                results,
                clip->extents.start.vec.v,
                clip->extents.end.vec.v,
                vec3_origin,
                vec3_origin,
                cmodel,
                clip->contentmask,
                cent->pose.origin,
                angles);
            if ( results->fraction >= oldFraction )
            {
                return;
            }
            results->modelIndex = 0;
            results->partName = 0;
            results->partGroup = 0;
            results->boneIndex = 254;
            number = p_nextState->number;
            if ( !results
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\physics\\../qcommon/cm_public.h",
                            175,
                            0,
                            "%s",
                            "trace") )
            {
                __debugbreak();
            }
            results->hitType = TRACE_HITTYPE_ENTITY;
            results->hitId = number;
            results->cflags = contents;
            return;
        }
        dobj = dobjEA;

        PROF_SCOPED("dobj");

        hasCollMap = DObjHasCollmap(dobjEA);
        character = cent->nextState.eType == 19
                         || cent->nextState.eType == 17
                         || cent->nextState.eType == 2
                         || cent->nextState.eType == 1;
        v6 = cent->nextState.surfType == 7 && !hasCollMap;
        character |= v6;
        if ( cent->pose.isRagdoll )
            character = 1;
        contents = DObjGetContents(dobj);
        absMins[0] = cent->pose.absmin[0];
        absMins[1] = cent->pose.absmin[1];
        absMins[2] = cent->pose.absmin[2];
        absMaxs[0] = cent->pose.absmax[0];
        absMaxs[1] = cent->pose.absmax[1];
        absMaxs[2] = cent->pose.absmax[2];
        if ( character )
            contents = 0x8000;
        if ( (clip->contentmask & contents) == 0 )
        {
            return;
        }
        if ( !intersect_extents_aabb(&clip->extents, absMins, absMaxs, results->fraction) )
        {
            return;
        }
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        *(_QWORD *)localStart = *(_QWORD *)clip->extents.start.vec.v;
        localStart[2] = clip->extents.start.vec.v[2];
        *(_QWORD *)localEnd = *(_QWORD *)clip->extents.end.vec.v;
        localEnd[2] = clip->extents.end.vec.v[2];
        objTrace.fraction = results->fraction;
        if ( character )
        {
            PROF_SCOPED("DObjTraceline");
            v3 = DObjGetName(dobj);
            //Name = va("char: %s", v3);
            ZoneTextF("char: %s", v3);

            DObjTracelinePartBits(dobj, partBits);
            CG_DObjCalcPose(&centEA->pose, dobj, partBits);
            DObjTraceline(dobj, localStart, localEnd, clip->priorityMap, &objTrace);
        }
        else
        {
            PROF_SCOPED("DObjGeomTraceline");

            v4 = DObjGetName(dobj);
            //v9 = va("non_char: %s", v4);
            ZoneTextF("non_char: %s", v4);

            DObjGeomTracelinePartBits(dobj, clip->contentmask, partBits);
            CG_DObjCalcPose(&centEA->pose, dobj, partBits);
            DObjGeomTraceline(dobj, localStart, localEnd, clip->contentmask, &objTrace);
        }
        if (objTrace.fraction >= results->fraction)
        {
            return;
        }
        results->fraction = objTrace.fraction;
        results->sflags = objTrace.sflags;
        results->modelIndex = objTrace.modelIndex;
        results->partName = objTrace.partName;
        results->partGroup = objTrace.partGroup;
        results->boneIndex = objTrace.localBoneIndex;
        *(_QWORD*)results->normal.vec.v = *(_QWORD*)objTrace.normal;
        results->normal.vec.u[2] = LODWORD(objTrace.normal[2]);
        results->walkable = results->normal.vec.v[2] >= 0.69999999;
        results->hitType = TRACE_HITTYPE_ENTITY;
        results->hitId = p_nextState->number;
        results->cflags = contents;
    }
}

void __cdecl CG_PointTraceToEntities(const pointtrace_t *clip, trace_t *results)
{
    _QWORD start[2]; // [esp+18h] [ebp-20h] BYREF
    _QWORD end[2]; // [esp+28h] [ebp-10h] BYREF

    PROF_SCOPED("cg_pointtracetoentities");

    if ( !clip && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 839, 0, "%s", "clip") )
        __debugbreak();
    if ( !results && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 840, 0, "%s", "results") )
        __debugbreak();
    if ( results->fraction <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp",
                    841,
                    0,
                    "%s\n\t(results->fraction) = %g",
                    "(results->fraction > 0.0f)",
                    results->fraction) )
    {
        __debugbreak();
    }
    if ( results->fraction > 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp",
                    842,
                    0,
                    "%s\n\t(results->fraction) = %g",
                    "(results->fraction <= 1.0f)",
                    results->fraction) )
    {
        __debugbreak();
    }
    start[0] = *(_QWORD *)clip->extents.start.vec.v;
    LODWORD(start[1]) = clip->extents.start.vec.u[2];
    end[0] = *(_QWORD *)clip->extents.end.vec.v;
    LODWORD(end[1]) = clip->extents.end.vec.u[2];
    HIDWORD(start[1]) = 0;
    HIDWORD(end[1]) = LODWORD(results->fraction);
    CG_PointTraceToEntities_r(clip, 1u, (const float *)start, (const float *)end, results);
}

void __cdecl CG_PointTraceToEntities_r(
                const pointtrace_t *clip,
                unsigned __int16 sectorIndex,
                const float *p1,
                const float *p2,
                trace_t *results)
{
    CgEntCollWorld *v5; // eax
    float v6; // [esp+10h] [ebp-54h]
    unsigned int v7; // [esp+14h] [ebp-50h]
    const CgEntCollSector *sector; // [esp+20h] [ebp-44h]
    const CgEntCollNode *node; // [esp+24h] [ebp-40h]
    unsigned __int16 listIndex; // [esp+2Ch] [ebp-38h]
    float t1; // [esp+30h] [ebp-34h]
    float frac; // [esp+34h] [ebp-30h]
    int localClientNum; // [esp+38h] [ebp-2Ch]
    int ignoreEntParamsBaseEntity; // [esp+3Ch] [ebp-28h]
    float t2; // [esp+40h] [ebp-24h]
    float p[4]; // [esp+44h] [ebp-20h] BYREF
    float mid[4]; // [esp+54h] [ebp-10h] BYREF

    if ( !clip && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 765, 0, "%s", "clip") )
        __debugbreak();
    if ( !p1 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 766, 0, "%s", "p1") )
        __debugbreak();
    if ( !p2 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 767, 0, "%s", "p2") )
        __debugbreak();
    if ( !results && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 768, 0, "%s", "results") )
        __debugbreak();
    if ( results->fraction > 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp",
                    769,
                    0,
                    "%s\n\t(results->fraction) = %g",
                    "(results->fraction <= 1.0f)",
                    results->fraction) )
    {
        __debugbreak();
    }
    localClientNum = cgCollWorldLocalClientNum;
    p[0] = *p1;
    p[1] = p1[1];
    p[2] = p1[2];
    p[3] = p1[3];
    ignoreEntParamsBaseEntity = clip->ignoreEntParams->baseEntity;
    while ( sectorIndex )
    {
        if ( localClientNum
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                        99,
                        0,
                        "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        localClientNum,
                        1) )
        {
            __debugbreak();
        }
        if ( sectorIndex >= 0x400u
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                        101,
                        0,
                        "%s\n\t(sectorIndex) = %i",
                        "(sectorIndex < 1024)",
                        sectorIndex) )
        {
            __debugbreak();
        }
        v5 = &cgEntCollWorld[localClientNum];
        sector = &v5->sectors[sectorIndex];
        for ( listIndex = v5->sectors[sectorIndex].entListHead; listIndex; listIndex = node->nextEntInSector )
        {
            v7 = listIndex - 1;
            if ( localClientNum
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                            108,
                            0,
                            "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                            localClientNum,
                            1) )
            {
                __debugbreak();
            }
            if ( v7 >= 0x600
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                            109,
                            0,
                            "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                            v7,
                            1536) )
            {
                __debugbreak();
            }
            node = &cgEntCollNodes[localClientNum][v7];
            if ( listIndex - 1 != ignoreEntParamsBaseEntity  )
            {
                if ( intersect_extents_aabb(&clip->extents, node->linkMins, node->linkMaxs, results->fraction) )
                {
                    CG_PointTraceToEntity(clip, listIndex - 1, results);
                    if ( results->fraction == 0.0 )
                        return;
                }
            }
        }
        t1 = p[sector->tree.axis] - sector->tree.dist;
        t2 = p2[sector->tree.axis] - sector->tree.dist;
        if ( (float)(t1 * t2) < 0.0 )
        {
            if ( p[3] >= results->fraction )
                return;
            frac = t1 / (float)(t1 - t2);
            if ( frac < 0.0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 812, 0, "%s", "frac >= 0.0f") )
            {
                __debugbreak();
            }
            if ( frac > 1.0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 813, 0, "%s", "frac <= 1.0f") )
            {
                __debugbreak();
            }
            mid[0] = (float)((float)(*p2 - p[0]) * frac) + p[0];
            mid[1] = (float)((float)(p2[1] - p[1]) * frac) + p[1];
            mid[2] = (float)((float)(p2[2] - p[2]) * frac) + p[2];
            mid[3] = (float)((float)(p2[3] - p[3]) * frac) + p[3];
            CG_PointTraceToEntities_r(clip, sector->tree.child[t2 >= 0.0], p, mid, results);
            if ( results->fraction == 0.0 )
                return;
            sectorIndex = sector->tree.child[t2 < 0.0];
            p[0] = mid[0];
            p[1] = mid[1];
            p[2] = mid[2];
            p[3] = mid[3];
        }
        else
        {
            if ( (float)(t2 - t1) < 0.0 )
                v6 = p2[sector->tree.axis] - sector->tree.dist;
            else
                v6 = p[sector->tree.axis] - sector->tree.dist;
            sectorIndex = sector->tree.child[v6 < 0.0];
        }
    }
}

void __cdecl CG_TraceCapsule(
                trace_t *results,
                const float *start,
                const float *mins,
                const float *maxs,
                const float *end,
                int passEntityNum,
                int contentMask,
                col_context_t *context)
{
    float delta[3]; // [esp+B8h] [ebp-Ch]

    PROF_SCOPED("CG_TraceCapsule");

    if ( !results && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 863, 0, "%s", "results") )
        __debugbreak();
    if ( !mins && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 864, 0, "%s", "mins") )
        __debugbreak();
    if ( !maxs && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 864, 0, "%s", "maxs") )
        __debugbreak();
    if ( *maxs < *mins
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 864, 0, "%s", "maxs[0] >= mins[0]") )
    {
        __debugbreak();
    }
    if ( maxs[1] < mins[1]
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 864, 0, "%s", "maxs[1] >= mins[1]") )
    {
        __debugbreak();
    }
    if ( maxs[2] < mins[2]
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 864, 0, "%s", "maxs[2] >= mins[2]") )
    {
        __debugbreak();
    }
    nanassertvec3(mins);
    nanassertvec3(maxs);
    nanassertvec3(start);
    nanassertvec3(end);

    CM_BoxTrace(results, start, end, mins, maxs, contentMask, context);
    if ( (LODWORD(results->fraction) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp",
                    870,
                    0,
                    "%s",
                    "!IS_NAN(results->fraction)") )
    {
        __debugbreak();
    }
    if ( results->fraction == 1.0 )
    {
        if ( !results
            && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\physics\\../qcommon/cm_public.h", 175, 0, "%s", "trace") )
        {
            __debugbreak();
        }
        results->hitType = TRACE_HITTYPE_NONE;
        results->hitId = 0;
    }
    else
    {
        if ( !results
            && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\physics\\../qcommon/cm_public.h", 175, 0, "%s", "trace") )
        {
            __debugbreak();
        }
        results->hitType = TRACE_HITTYPE_ENTITY;
        results->hitId = 1022;
    }
    if ( results->fraction == 0.0 )
    {
        return;
    }
    else
    {
        moveclip_t clip; // [esp+4Ch] [ebp-78h] BYREF

        ////TraceExtents::TraceExtents(&clip.extents);
        clip.contentmask = contentMask;
        clip.passEntityNum = passEntityNum;
        clip.passOwnerNum = -1;
        clip.outerSize[0] = *maxs - *mins;
        clip.outerSize[1] = maxs[1] - mins[1];
        clip.outerSize[2] = maxs[2] - mins[2];
        clip.maxs[0] = 0.5 * clip.outerSize[0];
        clip.maxs[1] = 0.5 * clip.outerSize[1];
        clip.maxs[2] = 0.5 * clip.outerSize[2];
        clip.mins[0] = -1.0 * (float)(0.5 * clip.outerSize[0]);
        clip.mins[1] = -1.0 * (float)(0.5 * clip.outerSize[1]);
        clip.mins[2] = -1.0 * (float)(0.5 * clip.outerSize[2]);
        clip.outerSize[0] = (float)(0.5 * clip.outerSize[0]) + 1.0;
        clip.outerSize[1] = (float)(0.5 * clip.outerSize[1]) + 1.0;
        clip.outerSize[2] = (float)(0.5 * clip.outerSize[2]) + 1.0;
        delta[0] = *maxs + *mins;
        delta[1] = maxs[1] + mins[1];
        delta[2] = maxs[2] + mins[2];
        delta[0] = 0.5 * delta[0];
        delta[1] = 0.5 * delta[1];
        delta[2] = 0.5 * delta[2];
        clip.extents.start.vec.v[0] = *start + delta[0];
        clip.extents.start.vec.v[1] = start[1] + delta[1];
        clip.extents.start.vec.v[2] = start[2] + delta[2];
        clip.extents.end.vec.v[0] = *end + delta[0];
        clip.extents.end.vec.v[1] = end[1] + delta[1];
        clip.extents.end.vec.v[2] = end[2] + delta[2];
        CM_CalcTraceExtents(&clip.extents);
        CG_ClipMoveToEntities(&clip, results);
        GlassCl_ClipMoveTrace(&clip, results);
    }
}

void __cdecl CG_ClipMoveToEntities(const moveclip_t *clip, trace_t *results)
{
    _QWORD start[2]; // [esp+1Ch] [ebp-20h] BYREF
    _QWORD end[2]; // [esp+2Ch] [ebp-10h] BYREF

    PROF_SCOPED("cg_clipmovetoentities");

    if ( !clip && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 543, 0, "%s", "clip") )
        __debugbreak();
    if ( !results && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 544, 0, "%s", "results") )
        __debugbreak();
    if ( results->fraction <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp",
                    545,
                    0,
                    "%s\n\t(results->fraction) = %g",
                    "(results->fraction > 0.0f)",
                    results->fraction) )
    {
        __debugbreak();
    }
    if ( results->fraction > 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp",
                    546,
                    0,
                    "%s\n\t(results->fraction) = %g",
                    "(results->fraction <= 1.0f)",
                    results->fraction) )
    {
        __debugbreak();
    }
    start[0] = *(_QWORD *)clip->extents.start.vec.v;
    LODWORD(start[1]) = clip->extents.start.vec.u[2];
    end[0] = *(_QWORD *)clip->extents.end.vec.v;
    LODWORD(end[1]) = clip->extents.end.vec.u[2];
    HIDWORD(start[1]) = 0;
    HIDWORD(end[1]) = LODWORD(results->fraction);
    CG_ClipMoveToEntities_r(clip, 1u, (const float *)start, (const float *)end, results);
}

void __cdecl CG_ClipMoveToEntities_r(
                const moveclip_t *clip,
                unsigned __int16 sectorIndex,
                const float *p1,
                const float *p2,
                trace_t *results)
{
    CgEntCollWorld *v5; // eax
    float v6; // [esp+8h] [ebp-94h]
    float v7; // [esp+Ch] [ebp-90h]
    float v8; // [esp+14h] [ebp-88h]
    float v9; // [esp+18h] [ebp-84h]
    float v10; // [esp+1Ch] [ebp-80h]
    unsigned int v11; // [esp+20h] [ebp-7Ch]
    const CgEntCollNode *node; // [esp+44h] [ebp-58h]
    const CgEntCollSector *sector; // [esp+48h] [ebp-54h]
    bool side; // [esp+4Ch] [ebp-50h]
    float diff; // [esp+50h] [ebp-4Ch]
    unsigned __int16 listIndex; // [esp+58h] [ebp-44h]
    float t1; // [esp+5Ch] [ebp-40h]
    float frac; // [esp+60h] [ebp-3Ch]
    int localClientNum; // [esp+64h] [ebp-38h]
    float offset; // [esp+68h] [ebp-34h]
    float t2; // [esp+6Ch] [ebp-30h]
    float frac2; // [esp+70h] [ebp-2Ch]
    float absDiff; // [esp+74h] [ebp-28h]
    float p[4]; // [esp+7Ch] [ebp-20h] BYREF
    float mid[4]; // [esp+8Ch] [ebp-10h] BYREF

    if ( !clip && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 443, 0, "%s", "clip") )
        __debugbreak();

    nanassertvec3(p1);
    nanassertvec3(p2);

    if ( !results && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 446, 0, "%s", "results") )
        __debugbreak();
    if ( results->fraction > 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp",
                    447,
                    0,
                    "%s\n\t(results->fraction) = %g",
                    "(results->fraction <= 1.0f)",
                    results->fraction) )
    {
        __debugbreak();
    }
    localClientNum = cgCollWorldLocalClientNum;
    p[0] = *p1;
    p[1] = p1[1];
    p[2] = p1[2];
    p[3] = p1[3];
    while ( sectorIndex )
    {
        if ( localClientNum
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                        99,
                        0,
                        "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        localClientNum,
                        1) )
        {
            __debugbreak();
        }
        if ( sectorIndex >= 0x400u
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                        101,
                        0,
                        "%s\n\t(sectorIndex) = %i",
                        "(sectorIndex < 1024)",
                        sectorIndex) )
        {
            __debugbreak();
        }
        v5 = &cgEntCollWorld[localClientNum];
        sector = &v5->sectors[sectorIndex];
        for ( listIndex = v5->sectors[sectorIndex].entListHead; listIndex; listIndex = node->nextEntInSector )
        {
            v11 = listIndex - 1;
            if ( localClientNum
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                            108,
                            0,
                            "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                            localClientNum,
                            1) )
            {
                __debugbreak();
            }
            if ( v11 >= 0x600
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                            109,
                            0,
                            "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                            v11,
                            1536) )
            {
                __debugbreak();
            }
            node = &cgEntCollNodes[localClientNum][v11];
            if ( listIndex - 1 != clip->passEntityNum )
            {
                CG_ClipMoveToEntity(clip, listIndex - 1, results);
                if ( results->allsolid || results->fraction == 0.0 )
                    return;
            }
        }
        t1 = p[sector->tree.axis] - sector->tree.dist;
        t2 = p2[sector->tree.axis] - sector->tree.dist;
        offset = clip->outerSize[sector->tree.axis];
        if ( (float)(t2 - t1) < 0.0 )
            v10 = p2[sector->tree.axis] - sector->tree.dist;
        else
            v10 = p[sector->tree.axis] - sector->tree.dist;
        if ( v10 < offset )
        {
            if ( (float)(t1 - t2) < 0.0 )
                v9 = p2[sector->tree.axis] - sector->tree.dist;
            else
                v9 = p[sector->tree.axis] - sector->tree.dist;
            //if ( COERCE_FLOAT(LODWORD(offset) ^ _mask__NegFloat_) < v9 )
            if ( -offset < v9 )
            {
                if ( p[3] >= results->fraction )
                    return;
                diff = t2 - t1;
                if ( (float)(t2 - t1) == 0.0 )
                {
                    frac = 1.0f;
                    frac2 = 0.0f;
                    side = 0;
                }
                else
                {
                    absDiff = fabs(diff);
                    if ( diff < 0.0 )
                        v8 = p[sector->tree.axis] - sector->tree.dist;
                    else
                        v8 = -t1;
                    frac = (float)(v8 + offset) * (float)(1.0 / absDiff);
                    frac2 = (float)(v8 - offset) * (float)(1.0 / absDiff);
                    side = diff >= 0.0;
                }
                if ( frac < 0.0
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 512, 0, "%s", "frac >= 0.0f") )
                {
                    __debugbreak();
                }
                v7 = (float)(1.0 - frac) < 0.0 ? 1.0f : frac;
                mid[0] = (float)((float)(*p2 - p[0]) * v7) + p[0];
                mid[1] = (float)((float)(p2[1] - p[1]) * v7) + p[1];
                mid[2] = (float)((float)(p2[2] - p[2]) * v7) + p[2];
                mid[3] = (float)((float)(p2[3] - p[3]) * v7) + p[3];
                CG_ClipMoveToEntities_r(clip, sector->tree.child[side], p, mid, results);
                if ( results->fraction == 0.0 )
                    return;
                if ( frac2 > 1.0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp",
                                524,
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
                p[0] = (float)((float)(*p2 - p[0]) * v6) + p[0];
                p[1] = (float)((float)(p2[1] - p[1]) * v6) + p[1];
                p[2] = (float)((float)(p2[2] - p[2]) * v6) + p[2];
                p[3] = (float)((float)(p2[3] - p[3]) * v6) + p[3];
                sectorIndex = sector->tree.child[1 - side];
            }
            else
            {
                sectorIndex = sector->tree.child[1];
            }
        }
        else
        {
            sectorIndex = sector->tree.child[0];
        }
    }
}

void __cdecl CG_ClipMoveToEntity(const moveclip_t *clip, int entIndex, trace_t *results)
{
    DObj *ClientDObj; // [esp+4h] [ebp-98h]
    unsigned __int16 v4; // [esp+Ah] [ebp-92h]
    unsigned __int16 number; // [esp+26h] [ebp-76h]
    PhysGeomList *geoms; // [esp+40h] [ebp-5Ch]
    int contents; // [esp+44h] [ebp-58h]
    float mins[3]; // [esp+4Ch] [ebp-50h] BYREF
    float absMaxs[3]; // [esp+58h] [ebp-44h] BYREF
    const centity_s *cent; // [esp+64h] [ebp-38h]
    int localClientNum; // [esp+68h] [ebp-34h]
    const entityState_s *p_nextState; // [esp+6Ch] [ebp-30h]
    float angles[3]; // [esp+70h] [ebp-2Ch] BYREF
    float maxs[3]; // [esp+7Ch] [ebp-20h] BYREF
    unsigned int cmodel; // [esp+88h] [ebp-14h]
    float oldFraction; // [esp+8Ch] [ebp-10h]
    float absMins[3]; // [esp+90h] [ebp-Ch] BYREF
    int savedregs; // [esp+9Ch] [ebp+0h] BYREF

    if ( !clip && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 317, 0, "%s", "clip") )
        __debugbreak();
    if ( (unsigned int)entIndex >= 0x600
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp",
                    318,
                    0,
                    "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                    entIndex,
                    1536) )
    {
        __debugbreak();
    }
    if ( !results && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 319, 0, "%s", "results") )
        __debugbreak();
    if ( entIndex < 1024 )
    {
        localClientNum = cgCollWorldLocalClientNum;
        cent = CG_GetEntity(cgCollWorldLocalClientNum, entIndex);
        p_nextState = &cent->nextState;
        if ( cent->nextState.eType != 4 && (p_nextState->lerp.eFlags & 1) == 0 )
        {
            if ( entIndex == clip->passEntityNum
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp",
                            336,
                            0,
                            "%s",
                            "entIndex != clip->passEntityNum") )
            {
                __debugbreak();
            }
            if ( BG_ClipMoveToDobj(p_nextState) )
                ClientDObj = Com_GetClientDObj(entIndex, localClientNum);
            else
                ClientDObj = 0;
            if ( ClientDObj )
            {
                if ( DObjHasCollmap(ClientDObj) )
                {
                    if ( DObjHasContents(ClientDObj, clip->contentmask) )
                    {
                        absMins[0] = cent->pose.absmin[0];
                        absMins[1] = cent->pose.absmin[1];
                        absMins[2] = cent->pose.absmin[2];
                        absMaxs[0] = cent->pose.absmax[0];
                        absMaxs[1] = cent->pose.absmax[1];
                        absMaxs[2] = cent->pose.absmax[2];
                        absMins[0] = absMins[0] + clip->mins[0];
                        absMins[1] = absMins[1] + clip->mins[1];
                        absMins[2] = absMins[2] + clip->mins[2];
                        absMaxs[0] = absMaxs[0] + clip->maxs[0];
                        absMaxs[1] = absMaxs[1] + clip->maxs[1];
                        absMaxs[2] = absMaxs[2] + clip->maxs[2];
                        if ( intersect_extents_aabb(&clip->extents, absMins, absMaxs, results->fraction) )
                        {
                            geoms = DObjGetCollmap(ClientDObj);
                            if ( geoms )
                            {
                                cmodel = CM_TempBrushModel(geoms);
                                oldFraction = results->fraction;
                                CM_TransformedBoxTrace(
                                    results,
                                    clip->extents.start.vec.v,
                                    clip->extents.end.vec.v,
                                    clip->mins,
                                    clip->maxs,
                                    cmodel,
                                    clip->contentmask,
                                    cent->pose.origin,
                                    cent->pose.angles);
                                if ( results->fraction < oldFraction )
                                {
                                    results->modelIndex = 0;
                                    results->partName = 0;
                                    results->partGroup = 0;
                                    results->boneIndex = 254;
                                    number = p_nextState->number;
                                    if ( !results
                                        && !Assert_MyHandler(
                                                    "c:\\projects_pc\\cod\\codsrc\\src\\physics\\../qcommon/cm_public.h",
                                                    175,
                                                    0,
                                                    "%s",
                                                    "trace") )
                                    {
                                        __debugbreak();
                                    }
                                    results->hitType = TRACE_HITTYPE_ENTITY;
                                    results->hitId = number;
                                }
                            }
                        }
                    }
                }
            }
            else if (p_nextState->solid && (p_nextState->solid != 0xFFFFFF || (p_nextState->lerp.eFlags & 1) == 0))
            {
                contents = CG_GetEntityBModelContents(cent);
                if ( (clip->contentmask & contents) != 0 )
                {
                    absMins[0] = cent->pose.absmin[0];
                    absMins[1] = cent->pose.absmin[1];
                    absMins[2] = cent->pose.absmin[2];
                    absMaxs[0] = cent->pose.absmax[0];
                    absMaxs[1] = cent->pose.absmax[1];
                    absMaxs[2] = cent->pose.absmax[2];
                    absMins[0] = absMins[0] + clip->mins[0];
                    absMins[1] = absMins[1] + clip->mins[1];
                    absMins[2] = absMins[2] + clip->mins[2];
                    absMaxs[0] = absMaxs[0] + clip->maxs[0];
                    absMaxs[1] = absMaxs[1] + clip->maxs[1];
                    absMaxs[2] = absMaxs[2] + clip->maxs[2];
                    if ( intersect_extents_aabb(&clip->extents, absMins, absMaxs, results->fraction) )
                    {
                        if (p_nextState->solid == 0xFFFFFF)
                        {
                            cmodel = p_nextState->index.brushmodel;
                            angles[0] = cent->pose.angles[0];
                            angles[1] = cent->pose.angles[1];
                            angles[2] = cent->pose.angles[2];
                        }
                        else
                        {
                            absMins[0] = cent->pose.absmin[0];
                            absMins[1] = cent->pose.absmin[1];
                            absMins[2] = cent->pose.absmin[2];
                            absMaxs[0] = cent->pose.absmax[0];
                            absMaxs[1] = cent->pose.absmax[1];
                            absMaxs[2] = cent->pose.absmax[2];
                            CG_GetEntityBModelBounds(cent, mins, maxs, absMins, absMaxs);
                            cmodel = CM_TempBoxModel(mins, maxs, contents);
                            memset(angles, 0, sizeof(angles));
                        }
                        oldFraction = results->fraction;
                        CM_TransformedBoxTrace(
                            results,
                            clip->extents.start.vec.v,
                            clip->extents.end.vec.v,
                            clip->mins,
                            clip->maxs,
                            cmodel,
                            clip->contentmask,
                            cent->pose.origin,
                            angles);
                        if ( results->fraction < oldFraction )
                        {
                            results->modelIndex = 0;
                            results->partName = 0;
                            results->partGroup = 0;
                            results->boneIndex = 254;
                            v4 = p_nextState->number;
                            if ( !results
                                && !Assert_MyHandler(
                                            "c:\\projects_pc\\cod\\codsrc\\src\\physics\\../qcommon/cm_public.h",
                                            175,
                                            0,
                                            "%s",
                                            "trace") )
                            {
                                __debugbreak();
                            }
                            results->hitType = TRACE_HITTYPE_ENTITY;
                            results->hitId = v4;
                        }
                    }
                }
            }
        }
    }
}

void __cdecl CG_LocationalTrace(
                trace_t *results,
                const float *start,
                const float *end,
                int passEntityNum,
                int contentMask,
                bool checkRopes,
                col_context_t *context)
{
    col_context_t *p_tempContext; // [esp+0h] [ebp-30h]

    ////col_context_t::col_context_t(&tempContext, contentMask);
    col_context_t tempContext(contentMask);

    if ( context )
        p_tempContext = context;
    else
        p_tempContext = &tempContext;
    CG_TracePoint(results, start, end, passEntityNum, contentMask, 1, 1, checkRopes, p_tempContext);
}

void __cdecl CG_TracePoint(
                trace_t *results,
                const float *start,
                const float *end,
                int passEntityNum,
                int contentMask,
                bool locational,
                bool staticModels,
                bool checkRopes,
                col_context_t *context)
{
    IgnoreEntParams *p_ignoreEntParams; // [esp+8h] [ebp-A8h]
    float _end[3]; // [esp+4Ch] [ebp-64h] BYREF
    IgnoreEntParams ignoreEntParams; // [esp+A0h] [ebp-10h] BYREF

    PROF_SCOPED("CG_TracePoint");

    nanassertvec3(start);
    nanassertvec3(end);

    CM_BoxTrace(results, start, end, vec3_origin, vec3_origin, contentMask, context);
    if ( (LODWORD(results->fraction) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp",
                    928,
                    0,
                    "%s",
                    "!IS_NAN(results->fraction)") )
    {
        __debugbreak();
    }
    if ( results->fraction == 1.0 )
    {
        if ( !results
            && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\physics\\../qcommon/cm_public.h", 175, 0, "%s", "trace") )
        {
            __debugbreak();
        }
        results->hitType = TRACE_HITTYPE_NONE;
        results->hitId = 0;
    }
    else
    {
        if ( !results
            && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\physics\\../qcommon/cm_public.h", 175, 0, "%s", "trace") )
        {
            __debugbreak();
        }
        results->hitType = TRACE_HITTYPE_ENTITY;
        results->hitId = 1022;
    }
    if ( results->fraction == 0.0 )
    {
        return;
    }
    else
    {
        pointtrace_t clip; // [esp+58h] [ebp-58h] BYREF

        ////TraceExtents::TraceExtents(&clip.extents);
        if ( !staticModels )
            goto LABEL_34;
        CM_PointTraceStaticModels(results, start, end, contentMask);
        if ( (LODWORD(results->fraction) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp",
                        944,
                        0,
                        "%s",
                        "!IS_NAN(results->fraction)") )
        {
            __debugbreak();
        }
        if ( results->fraction == 0.0 )
        {
            return;
        }
        else
        {
LABEL_34:
            ignoreEntParams.baseEntity = passEntityNum;
            ignoreEntParams.parentEntity = -1;
            ignoreEntParams.ignoreSelf = 1;
            ignoreEntParams.ignoreParent = 0;
            ignoreEntParams.ignoreChildren = 0;
            ignoreEntParams.ignoreSiblings = 0;
            *(_QWORD *)clip.extents.start.vec.v = *(_QWORD *)start;
            clip.extents.start.vec.v[2] = start[2];
            *(_QWORD *)clip.extents.end.vec.v = *(_QWORD *)end;
            clip.extents.end.vec.v[2] = end[2];
            CM_CalcTraceExtents(&clip.extents);
            if ( context->ignoreEntParams )
                p_ignoreEntParams = context->ignoreEntParams;
            else
                p_ignoreEntParams = &ignoreEntParams;
            clip.ignoreEntParams = p_ignoreEntParams;
            clip.contentmask = contentMask;
            clip.bLocational = locational;
            clip.priorityMap = bulletPriorityMap;
            CG_PointTraceToEntities(&clip, results);
            if ( checkRopes )
            {
                Vec3Lerp(start, end, results->fraction, _end);
                Rope_Trace(start, _end);
                GlassCl_TracePoint(&clip, results);
            }
        }
    }
}

void __cdecl CG_AreaEntities_r(unsigned __int16 sectorIndex, CEntityAreaParms *areaParms)
{
    CgEntCollWorld *v2; // ecx
    unsigned int v3; // [esp+0h] [ebp-20h]
    const CgEntCollNode *node; // [esp+Ch] [ebp-14h]
    const CgEntCollSector *sector; // [esp+10h] [ebp-10h]
    unsigned __int16 listIndex; // [esp+14h] [ebp-Ch]
    int localClientNum; // [esp+18h] [ebp-8h]
    unsigned __int16 nextSectorIndex; // [esp+1Ch] [ebp-4h]

    if ( !areaParms
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 1042, 0, "%s", "areaParms") )
    {
        __debugbreak();
    }
    localClientNum = cgCollWorldLocalClientNum;
    while ( sectorIndex )
    {
        if ( localClientNum
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                        99,
                        0,
                        "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        localClientNum,
                        1) )
        {
            __debugbreak();
        }
        if ( sectorIndex >= 0x400u
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                        101,
                        0,
                        "%s\n\t(sectorIndex) = %i",
                        "(sectorIndex < 1024)",
                        sectorIndex) )
        {
            __debugbreak();
        }
        v2 = &cgEntCollWorld[localClientNum];
        sector = &v2->sectors[sectorIndex];
        for ( listIndex = v2->sectors[sectorIndex].entListHead; listIndex; listIndex = node->nextEntInSector )
        {
            v3 = listIndex - 1;
            if ( localClientNum
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                            108,
                            0,
                            "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                            localClientNum,
                            1) )
            {
                __debugbreak();
            }
            if ( v3 >= 0x600
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                            109,
                            0,
                            "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                            v3,
                            1536) )
            {
                __debugbreak();
            }
            node = &cgEntCollNodes[localClientNum][v3];
            if ( CG_EntityInArea(listIndex - 1, areaParms->mins, areaParms->maxs, areaParms->contentMask) )
            {
                if ( areaParms->count == areaParms->maxCount )
                    return;
                areaParms->list[areaParms->count++] = listIndex - 1;
            }
        }
        if ( areaParms->maxs[sector->tree.axis] <= sector->tree.dist )
        {
            if ( areaParms->mins[sector->tree.axis] >= sector->tree.dist )
                return;
            sectorIndex = sector->tree.child[1];
        }
        else if ( sector->tree.dist <= areaParms->mins[sector->tree.axis] )
        {
            sectorIndex = sector->tree.child[0];
        }
        else
        {
            nextSectorIndex = sector->tree.child[1];
            CG_AreaEntities_r(sector->tree.child[0], areaParms);
            sectorIndex = nextSectorIndex;
        }
    }
}

char __cdecl CG_EntityInArea(unsigned int entIndex, const float *mins, const float *maxs, int contentMask)
{
    const DObj *dobj; // [esp+24h] [ebp-Ch]
    const centity_s *cent; // [esp+28h] [ebp-8h]
    int localClientNum; // [esp+2Ch] [ebp-4h]

    if ( entIndex >= 0x600
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp",
                    1007,
                    0,
                    "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                    entIndex,
                    1536) )
    {
        __debugbreak();
    }
    localClientNum = cgCollWorldLocalClientNum;
    cent = CG_GetEntity(cgCollWorldLocalClientNum, entIndex);
    dobj = Com_GetClientDObj(entIndex, localClientNum);
    if ( dobj )
    {
        if ( (contentMask & CG_GetEntityDObjContents(cent, dobj)) == 0 )
            return 0;
        if ( cent->pose.absmax[0] >= *mins
            && cent->pose.absmax[1] >= mins[1]
            && cent->pose.absmax[2] >= mins[2]
            && *maxs >= cent->pose.absmin[0]
            && maxs[1] >= cent->pose.absmin[1]
            && maxs[2] >= cent->pose.absmin[2] )
        {
            return 1;
        }
    }
    else if (cent->nextState.solid == 0xFFFFFF
                 && cent->nextState.eType != 14
                 && cent->nextState.eType != 16
                 && cent->pose.absmax[0] >= *mins
                 && cent->pose.absmax[1] >= mins[1]
                 && cent->pose.absmax[2] >= mins[2]
                 && *maxs >= cent->pose.absmin[0]
                 && maxs[1] >= cent->pose.absmin[1]
                 && maxs[2] >= cent->pose.absmin[2] )
    {
        return 1;
    }
    return 0;
}

int __cdecl CG_AreaEntities(const float *mins, const float *maxs, int *entityList, int maxEntities, int contentMask)
{
    int count; // [esp+20h] [ebp-20h]
    CEntityAreaParms areaParms; // [esp+24h] [ebp-1Ch] BYREF

    PROF_SCOPED("CG_AreaEntities");

    if ( !mins && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 1097, 0, "%s", "mins") )
        __debugbreak();
    if ( !maxs && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 1097, 0, "%s", "maxs") )
        __debugbreak();
    if ( *maxs < *mins
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 1097, 0, "%s", "maxs[0] >= mins[0]") )
    {
        __debugbreak();
    }
    if ( maxs[1] < mins[1]
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 1097, 0, "%s", "maxs[1] >= mins[1]") )
    {
        __debugbreak();
    }
    if ( maxs[2] < mins[2]
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 1097, 0, "%s", "maxs[2] >= mins[2]") )
    {
        __debugbreak();
    }

    nanassertvec3(mins);
    nanassertvec3(maxs);

    if ( maxEntities <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp",
                    1098,
                    0,
                    "%s\n\t(maxEntities) = %i",
                    "(maxEntities > 0)",
                    maxEntities) )
    {
        __debugbreak();
    }
    if ( !entityList
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 1099, 0, "%s", "entityList") )
    {
        __debugbreak();
    }
    areaParms.mins = mins;
    areaParms.maxs = maxs;
    areaParms.contentMask = contentMask;
    areaParms.list = entityList;
    areaParms.maxCount = maxEntities;
    areaParms.count = 0;
    CG_AreaEntities_r(1u, &areaParms);
    count = areaParms.count;
    return count;
}

bool __cdecl CG_SightTracePointInternal(int *hitNum, const float *start, const float *end, int mask, trace_t *trace)
{
    traceWork_t tw; // [esp+48h] [ebp-E8h] BYREF

    //traceWork_t::traceWork_t(&tw);

    nanassertvec3(start);
    nanassertvec3(end);

    if ( !cm.numNodes
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 1126, 0, "%s", "cm.numNodes") )
    {
        __debugbreak();
    }
    trace->fraction = 1.0f;
    trace->startsolid = 0;
    trace->allsolid = 0;
    tw.contents = mask;
    memset(&tw.size, 0, 12);
    *(_QWORD *)tw.extents.start.vec.v = *(_QWORD *)start;
    tw.extents.start.vec.v[2] = start[2];
    *(_QWORD *)tw.extents.end.vec.v = *(_QWORD *)end;
    tw.extents.end.vec.v[2] = end[2];
    tw.midpoint.vec.v[0] = (float)(0.5 * tw.extents.start.vec.v[0]) + (float)(0.5 * tw.extents.end.vec.v[0]);
    tw.midpoint.vec.v[1] = (float)(0.5 * tw.extents.start.vec.v[1]) + (float)(0.5 * tw.extents.end.vec.v[1]);
    tw.midpoint.vec.v[2] = (float)(0.5 * tw.extents.start.vec.v[2]) + (float)(0.5 * tw.extents.end.vec.v[2]);
    tw.delta.vec.v[0] = tw.extents.end.vec.v[0] - tw.extents.start.vec.v[0];
    tw.delta.vec.v[1] = tw.extents.end.vec.v[1] - tw.extents.start.vec.v[1];
    tw.delta.vec.v[2] = tw.extents.end.vec.v[2] - tw.extents.start.vec.v[2];
    tw.halfDelta.vec.v[0] = 0.5 * (float)(tw.extents.end.vec.v[0] - tw.extents.start.vec.v[0]);
    tw.halfDelta.vec.v[1] = 0.5 * (float)(tw.extents.end.vec.v[1] - tw.extents.start.vec.v[1]);
    tw.halfDelta.vec.v[2] = 0.5 * (float)(tw.extents.end.vec.v[2] - tw.extents.start.vec.v[2]);
    memset(&tw.radiusOffset, 0, 12);
    tw.halfDeltaAbs.vec.v[0] = I_fabs(tw.halfDelta.vec.v[0]);
    tw.halfDeltaAbs.vec.v[1] = I_fabs(tw.halfDelta.vec.v[1]);
    tw.halfDeltaAbs.vec.v[2] = I_fabs(tw.halfDelta.vec.v[2]);
    CM_CalcTraceExtents(&tw.extents);
    tw.deltaLenSq = (float)((float)(tw.delta.vec.v[0] * tw.delta.vec.v[0]) + (float)(tw.delta.vec.v[1] * tw.delta.vec.v[1]))
                                + (float)(tw.delta.vec.v[2] * tw.delta.vec.v[2]);
    tw.deltaLen = sqrtf(tw.deltaLenSq);
    Vec3Min(tw.extents.start.vec.v, tw.extents.end.vec.v, tw.bounds[0].vec.v);
    Vec3Max(tw.extents.start.vec.v, tw.extents.end.vec.v, tw.bounds[1].vec.v);
    tw.axialCullOnly = 0;
    tw.isPoint = 1;
    memset(&tw.radius, 0, 12);
    CM_GetTraceThreadInfo(&tw.threadInfo);
    if ( !*hitNum )
        *hitNum = CM_SightTraceThroughTree(&tw, 0, tw.extents.start.vec.v, tw.extents.end.vec.v, trace);
    return *hitNum != 0;
}

bool __cdecl CG_SightTracePoint(int *hitNum, const float *start, const float *end, int mask, trace_t *trace)
{
    int j; // [esp+10h] [ebp-194h]
    float v7; // [esp+14h] [ebp-190h]
    float dir; // [esp+18h] [ebp-18Ch]
    float dir_4; // [esp+1Ch] [ebp-188h]
    float dir_8; // [esp+20h] [ebp-184h]
    float v11; // [esp+24h] [ebp-180h]
    float v12; // [esp+24h] [ebp-180h]
    float p1[3]; // [esp+28h] [ebp-17Ch] BYREF
    float p0[3]; // [esp+34h] [ebp-170h] BYREF
    float t0; // [esp+40h] [ebp-164h] BYREF
    float t1; // [esp+44h] [ebp-160h] BYREF
    float mx[3]; // [esp+48h] [ebp-15Ch] BYREF
    float mn[3]; // [esp+54h] [ebp-150h] BYREF
    int i; // [esp+60h] [ebp-144h]
    float intervals[32][2]; // [esp+64h] [ebp-140h] BYREF
    int nintervals; // [esp+164h] [ebp-40h]
    const cbrush_t *brush; // [esp+168h] [ebp-3Ch]
    TraceExtents extents; // [esp+16Ch] [ebp-38h] BYREF
    int oldHitNum; // [esp+1A0h] [ebp-4h]

    ////TraceExtents::TraceExtents(&extents);
    *(_QWORD *)extents.start.vec.v = *(_QWORD *)start;
    extents.start.vec.v[2] = start[2];
    *(_QWORD *)extents.end.vec.v = *(_QWORD *)end;
    extents.end.vec.v[2] = end[2];
    CM_CalcTraceExtents(&extents);
    oldHitNum = *hitNum;
    *hitNum = 0;
    if ( oldHitNum > 0 && --oldHitNum < cm.numBrushes )
    {
        brush = &cm.brushes[oldHitNum];
        if ( CM_SightTracePointThroughBrush(&extents, brush) )
            *hitNum = oldHitNum + 1;
    }
    if ( !*hitNum )
    {
        nintervals = 0;
        for ( i = 0; i < num_occlusion_overrides; ++i )
        {
            mn[0] = occlusion_overrides[i][0][0];
            mn[1] = occlusion_overrides[i][0][1];
            mn[2] = occlusion_overrides[i][0][2];
            *(_QWORD *)mx = *(_QWORD *)&occlusion_overrides[i][1][0];
            mx[2] = occlusion_overrides[i][1][2];
            t0 = 0.0f;
            t1 = 1.0f;
            if ( intersect_extents_aabb(&extents, mn, mx, &t0, &t1) )
            {
                intervals[nintervals][0] = t0;
                intervals[nintervals++][1] = t1;
                if ( nintervals == 32 )
                    break;
            }
        }
        if ( nintervals <= 0 )
            return CG_SightTracePointInternal(hitNum, start, end, mask, trace);
        qsort(intervals, nintervals, 8u, (int (__cdecl *)(const void *, const void *))cmpr);
        v11 = 0.0f;
        dir = *end - *start;
        dir_4 = end[1] - start[1];
        dir_8 = end[2] - start[2];
        for ( j = 0; j < nintervals; ++j )
        {
            v7 = v11;
            v12 = intervals[j][0];
            if ( v12 > v7 )
            {
                p0[0] = (float)(v7 * dir) + *start;
                p0[1] = (float)(v7 * dir_4) + start[1];
                p0[2] = (float)(v7 * dir_8) + start[2];
                p1[0] = (float)(v12 * dir) + *start;
                p1[1] = (float)(v12 * dir_4) + start[1];
                p1[2] = (float)(v12 * dir_8) + start[2];
                if ( CG_SightTracePointInternal(hitNum, p0, p1, mask, trace) )
                {
                    trace->fraction = (float)((float)(v12 - v7) * trace->fraction) + v7;
                    return 1;
                }
            }
            v11 = intervals[j][1];
        }
        if ( v11 < 1.0 )
        {
            p0[0] = (float)(v11 * dir) + *start;
            p0[1] = (float)(v11 * dir_4) + start[1];
            p0[2] = (float)(v11 * dir_8) + start[2];
            p1[0] = (float)(1.0 * dir) + *start;
            p1[1] = (float)(1.0 * dir_4) + start[1];
            p1[2] = (float)(1.0 * dir_8) + start[2];
            if ( CG_SightTracePointInternal(hitNum, p0, p1, mask, trace) )
            {
                trace->fraction = (float)((float)(1.0 - v11) * trace->fraction) + v11;
                return 1;
            }
        }
    }
    return *hitNum != 0;
}

int __cdecl cmpr(float *e0, float *e1)
{
    if ( *e0 <= *e1 )
        return -1;
    else
        return 1;
}

int __cdecl CG_GetEntityBoneInfo(int entID, int boneIndex, float *bonePos, float (*boneAxis)[3], char **boneName)
{
    DObj *dobj; // [esp+8h] [ebp-1Ch]
    int boneBase; // [esp+Ch] [ebp-18h]
    XModel *model; // [esp+10h] [ebp-14h]
    int localClientNum; // [esp+14h] [ebp-10h]
    const centity_s *cent; // [esp+18h] [ebp-Ch]
    int modelIndex; // [esp+1Ch] [ebp-8h]
    unsigned __int16 *boneNames; // [esp+20h] [ebp-4h]

    localClientNum = cgCollWorldLocalClientNum;
    dobj = Com_GetClientDObj(entID, cgCollWorldLocalClientNum);
    cent = CG_GetEntity(localClientNum, entID);
    if ( !dobj )
        return 0;
    modelIndex = 0;
    for ( boneBase = 0; ; boneBase += model->numBones )
    {
        if ( boneIndex < boneBase || modelIndex >= DObjGetNumModels(dobj) )
            return 0;
        model = DObjGetModel(dobj, modelIndex);
        if ( boneIndex - boneBase < model->numBones )
            break;
        ++modelIndex;
    }
    CG_DObjGetWorldBoneMatrix(&cent->pose, dobj, boneIndex, boneAxis, bonePos);
    boneNames = XModelBoneNames(model);
    if ( !boneNames
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_world.cpp", 1286, 0, "%s", "boneNames") )
    {
        __debugbreak();
    }
    *boneName = SL_ConvertToString(boneNames[boneIndex - boneBase], SCRIPTINSTANCE_SERVER);
    return 1;
}