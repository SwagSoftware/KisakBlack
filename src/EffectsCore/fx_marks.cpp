#include "fx_marks.h"
#include <cstddef>
#include <universal/assertive.h>
#include <universal/q_shared.h>
#include <universal/com_workercmds.h>
#include "fx_draw.h"
#include "fx_random.h"
#include "fx_dvars.h"
#include <client/cl_debugdata.h>
#include <gfx_d3d/r_material_load_obj.h>
#include "fx_system.h"
#include <cgame/cg_compass.h>
#include <qcommon/dobj_management.h>

#include <algorithm>
#include <client/splitscreen.h>
#include <universal/com_math_anglevectors.h>
#include <xanim/dobj_utils.h>
#include <gfx_d3d/r_drawsurf.h>
#include <xanim/xmodel_utils.h>
#include "fx_update_util.h"
#include <gfx_d3d/r_scene.h>

volatile unsigned int fx_add_markLimit = 1;
jqModule fx_add_markModule =
{
    .Name = "jx_add_mark",
    .Type = JQ_WORKER_GENERIC,
    .Code = fx_add_markCallback
};
jqWorkerCmd fx_add_markWorkerCmd = { &fx_add_markModule, 44u, 0, 0, &fx_add_markLimit, NULL, 0u };

FxMarkPoint g_fxMarkPoints[3060];
FxMarksSystem *fx_marksSystemPool;
int fx_maxLocalClients;

volatile unsigned int g_fxMarkPointsCurrentBuffer;
volatile unsigned int g_markThread[1];

// *WARNING* One or more selections were skipped as they could not be interpreted as c data
bool __cdecl FX_MarkIsAlphaFadedOut(const FxMark *mark)
{
    return mark->alphaFade.ageLimitMsec > 0 && mark->ageMsec > mark->alphaFade.ageLimitMsec;
}

unsigned __int8 __cdecl FX_MarkGetFadedAlpha(const FxMark *mark)
{
    int v1; // eax
    float v3; // [esp+0h] [ebp-14h]
    float v4; // [esp+4h] [ebp-10h]

    if ( mark->ageMsec <= mark->alphaFade.fadeStartAgeMsec )
    {
        LOBYTE(v1) = mark->nativeColor[3];
    }
    else
    {
        v4 = (float)(mark->ageMsec - mark->alphaFade.fadeStartAgeMsec)
             / (float)(mark->alphaFade.ageLimitMsec - mark->alphaFade.fadeStartAgeMsec);
        if ( 1.0 <= 0.0
            && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 730, 0, "%s", "min < max") )
        {
            __debugbreak();
        }
        if ( v4 >= 0.0 )
        {
            if ( v4 <= 1.0 )
                v3 = v4;
            else
                v3 = 1.0f;
        }
        else
        {
            v3 = 0.0f;
        }
        return (int)(float)((float)mark->nativeColor[3] * (float)(1.0 - v3));
    }
    return v1;
}

void __cdecl FX_InitMarksSystem(FxMarksSystem *marksSystem)
{
    int pointIndex; // [esp+8h] [ebp-10h]
    unsigned int markIndex; // [esp+Ch] [ebp-Ch]
    int triIndex; // [esp+10h] [ebp-8h]
    unsigned int markHandleIndex; // [esp+14h] [ebp-4h]

    Sys_AssistAndWaitWorkerCmdInternal(&fx_add_markWorkerCmd);
    Sys_EnterCriticalSection(CRITSECT_ALLOC_MARK);
    for ( markHandleIndex = 0; markHandleIndex != 1536; ++markHandleIndex )
        marksSystem->entFirstMarkHandles[markHandleIndex] = -1;
    marksSystem->firstFreeMarkHandle = FX_MarkToHandle(marksSystem, marksSystem->marks);
    for ( markIndex = 0; markIndex < 0xFF; ++markIndex )
    {
        marksSystem->marks[markIndex].prevMark = -1;
        marksSystem->marks[markIndex].nextMark = FX_MarkToHandle(marksSystem, &marksSystem->marks[markIndex + 1]);
        marksSystem->marks[markIndex].frameCountDrawn = -1;
    }
    marksSystem->marks[markIndex].prevMark = -1;
    marksSystem->marks[markIndex].nextMark = -1;
    marksSystem->marks[markIndex].frameCountDrawn = -1;
    marksSystem->firstFreeTriGroup = marksSystem->triGroups;
    for ( triIndex = 0; triIndex < 1023; ++triIndex )
        marksSystem->triGroups[triIndex].nextFreeTriGroup = &marksSystem->triGroups[triIndex + 1];
    marksSystem->triGroups[triIndex].nextFreeTriGroup = 0;
    marksSystem->firstFreePointGroup = marksSystem->pointGroups;
    for ( pointIndex = 0; pointIndex < 1535; ++pointIndex )
        marksSystem->pointGroups[pointIndex].nextFreePointGroup = &marksSystem->pointGroups[pointIndex + 1];
    marksSystem->pointGroups[pointIndex].nextFreePointGroup = 0;
    marksSystem->firstActiveWorldMarkHandle = -1;
    marksSystem->allocedMarkCount = 0;
    marksSystem->freedMarkCount = 0;
    marksSystem->frameTime = 0;
    marksSystem->numVisibleWorldSeethru = 0;
    marksSystem->numVisibleEntBrushSeethru = 0;
    Sys_LeaveCriticalSection(CRITSECT_ALLOC_MARK);
}

unsigned __int16 __cdecl FX_MarkToHandle(FxMarksSystem *marksSystem, FxMark *mark)
{
    unsigned __int16 handle; // [esp+0h] [ebp-4h]

    if ( !marksSystem
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_marks.h", 166, 0, "%s", "marksSystem") )
    {
        __debugbreak();
    }
    if ( !mark && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_marks.h", 167, 0, "%s", "mark") )
        __debugbreak();
    handle = mark - marksSystem->marks;
    if ( handle >= 0x100u
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_marks.h",
                    170,
                    0,
                    "handle doesn't index FX_MARKS_LIMIT\n\t%i not in [0, %i)",
                    handle,
                    256) )
    {
        __debugbreak();
    }
    return mark - marksSystem->marks;
}

void __cdecl FX_ClearMarks(unsigned int localClientNum)
{
    FxMarksSystem *marksSystem; // [esp+0h] [ebp-4h]

    marksSystem = FX_GetMarksSystem(localClientNum);
    FX_InitMarksSystem(marksSystem);
}

FxMarksSystem *__cdecl FX_GetMarksSystem(unsigned int clientIndex)
{
    if ( clientIndex >= fx_maxLocalClients
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_marks.h",
                    157,
                    0,
                    "clientIndex doesn't index fx_maxLocalClients\n\t%i not in [0, %i)",
                    clientIndex,
                    fx_maxLocalClients) )
    {
        __debugbreak();
    }
    return &fx_marksSystemPool[clientIndex];
}

void __cdecl FX_FreeMark(FxMarksSystem *marksSystem, FxMark *mark)
{
    switch ( mark->context.modelTypeAndSurf & 0xC0 )
    {
        case 0:
            FX_FreeMarkFromList(marksSystem, mark, &marksSystem->firstActiveWorldMarkHandle);
            break;
        case 0x40:
            FX_FreeMarkFromList(marksSystem, mark, 0);
            break;
        case 0x80:
        case 0xC0:
            if ( mark->context.modelIndex >= 0x600u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                            292,
                            0,
                            "mark->context.modelIndex doesn't index ARRAY_COUNT( marksSystem->entFirstMarkHandles )\n"
                            "\t%i not in [0, %i)",
                            mark->context.modelIndex,
                            1536) )
            {
                __debugbreak();
            }
            FX_FreeMarkFromList(marksSystem, mark, &marksSystem->entFirstMarkHandles[mark->context.modelIndex]);
            break;
        default:
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                            303,
                            0,
                            "Unhandled case.\n") )
                __debugbreak();
            break;
    }
    ++marksSystem->freedMarkCount;
}

void __cdecl FX_FreeMarkFromList(FxMarksSystem *marksSystem, FxMark *mark, unsigned __int16 *listHead)
{
    const char *v3; // eax
    unsigned __int16 markHandle; // [esp+4h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_ALLOC_MARK);
    markHandle = FX_MarkToHandle(marksSystem, mark);
    FX_FreeMarkTriGroups(marksSystem, mark);
    FX_FreeMarkPointGroups(marksSystem, mark);
    if ( mark->nextMark != 0xFFFF )
        FX_MarkFromHandle(marksSystem, mark->nextMark)->prevMark = mark->prevMark;
    if ( mark->prevMark == 0xFFFF )
    {
        if ( listHead )
        {
            if ( *listHead != markHandle )
            {
                v3 = va("%i %i", *listHead, markHandle);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                                267,
                                0,
                                "%s\n\t%s",
                                "*listHead == markHandle",
                                v3) )
                    __debugbreak();
            }
            *listHead = mark->nextMark;
        }
    }
    else
    {
        FX_MarkFromHandle(marksSystem, mark->prevMark)->nextMark = mark->nextMark;
    }
    mark->frameCountDrawn = -1;
    mark->nextMark = marksSystem->firstFreeMarkHandle;
    marksSystem->firstFreeMarkHandle = markHandle;
    Sys_LeaveCriticalSection(CRITSECT_ALLOC_MARK);
}

FxMark *__cdecl FX_MarkFromHandle(FxMarksSystem *marksSystem, unsigned __int16 handle)
{
    if ( handle >= 0x100u
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_marks.h",
                    188,
                    0,
                    "handle doesn't index FX_MARKS_LIMIT\n\t%i not in [0, %i)",
                    handle,
                    256) )
    {
        __debugbreak();
    }
    if ( !marksSystem
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_marks.h", 189, 0, "%s", "marksSystem") )
    {
        __debugbreak();
    }
    return &marksSystem->marks[handle];
}

void __cdecl FX_FreeMarkTriGroups(FxMarksSystem *marksSystem, FxMark *mark)
{
    unsigned int groupHandle; // [esp+4h] [ebp-Ch]
    FxTriGroupPool *group; // [esp+Ch] [ebp-4h]

    groupHandle = mark->tris;
    do
    {
        group = FX_TriGroupFromHandle(marksSystem, groupHandle);
        groupHandle = group->triGroup.next;
        group->nextFreeTriGroup = marksSystem->firstFreeTriGroup;
        marksSystem->firstFreeTriGroup = group;
    }
    while ( groupHandle != 0xFFFF );
}

FxTriGroupPool *__cdecl FX_TriGroupFromHandle(FxMarksSystem *marksSystem, unsigned int handle)
{
    if ( handle >= 0x6000
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_marks.h",
                    212,
                    0,
                    "handle doesn't index FX_TRI_GROUP_LIMIT * sizeof( FxTriGroup )\n\t%i not in [0, %i)",
                    handle,
                    24576) )
    {
        __debugbreak();
    }
    if ( !marksSystem
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_marks.h", 213, 0, "%s", "marksSystem") )
    {
        __debugbreak();
    }
    return (FxTriGroupPool *)((char *)marksSystem->triGroups + handle);
}

void __cdecl FX_FreeMarkPointGroups(FxMarksSystem *marksSystem, FxMark *mark)
{
    unsigned int groupHandle; // [esp+4h] [ebp-Ch]
    FxPointGroupPool *group; // [esp+Ch] [ebp-4h]

    groupHandle = mark->points;
    do
    {
        group = FX_PointGroupFromHandle(marksSystem, groupHandle);
        groupHandle = group->pointGroup.next;
        group->nextFreePointGroup = marksSystem->firstFreePointGroup;
        marksSystem->firstFreePointGroup = group;
    }
    while ( groupHandle != 0xFFFF );
}

FxPointGroupPool *__cdecl FX_PointGroupFromHandle(FxMarksSystem *marksSystem, unsigned int handle)
{
    if ( handle >= 0x19800
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_marks.h",
                    237,
                    0,
                    "handle doesn't index FX_POINT_GROUP_LIMIT * sizeof( FxPointGroup )\n\t%i not in [0, %i)",
                    handle,
                    104448) )
    {
        __debugbreak();
    }
    if ( !marksSystem
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_marks.h", 238, 0, "%s", "marksSystem") )
    {
        __debugbreak();
    }
    return (FxPointGroupPool *)((char *)marksSystem->pointGroups + handle);
}

char __cdecl FX_SeeThruMarkOverlaps(
                FxMarksSystem *marksSystem,
                const float *origin,
                float radius,
                const float *hitNormal)
{
    float c2c_4; // [esp+10h] [ebp-18h]
    float c2c_8; // [esp+14h] [ebp-14h]
    unsigned int i; // [esp+24h] [ebp-4h]

    for ( i = 0; i < 0x100; ++i )
    {
        if ( marksSystem->marks[i].frameCountDrawn != -1
            && (float)((float)((float)(*hitNormal * marksSystem->marks[i].hitNormal[0])
                                             + (float)(hitNormal[1] * marksSystem->marks[i].hitNormal[1]))
                             + (float)(hitNormal[2] * marksSystem->marks[i].hitNormal[2])) >= 0.866 )
        {
            c2c_4 = origin[1] - marksSystem->marks[i].origin[1];
            c2c_8 = origin[2] - marksSystem->marks[i].origin[2];
            if ( (float)((float)((float)(marksSystem->marks[i].radius + radius) * 0.5)
                                 * (float)((float)(marksSystem->marks[i].radius + radius) * 0.5)) > (float)((float)((float)((float)(*origin - marksSystem->marks[i].origin[0]) * (float)(*origin - marksSystem->marks[i].origin[0]))
                                                                                                                                                                                                    + (float)(c2c_4 * c2c_4))
                                                                                                                                                                                    + (float)(c2c_8 * c2c_8)) )
                return 1;
        }
    }
    return 0;
}

void __cdecl FX_BeginMarks(unsigned int clientIndex)
{
    FxMarksSystem *marksSystem; // [esp+0h] [ebp-4h]

    marksSystem = FX_GetMarksSystem(clientIndex);
    if ( ++marksSystem->frameCount <= 0 )
        marksSystem->frameCount = 1;
    marksSystem->numVisibleWorldSeethru = 0;
    marksSystem->numVisibleEntBrushSeethru = 0;
}

void __cdecl FX_CreateImpactMark(
                int localClientNum,
                const FxElemDef *elemDef,
                const FxSpatialFrame *spatialFrame,
                int randomSeed,
                unsigned int markEntnum)
{
    PROF_SCOPED("FX_CreateImpactMark");
    FX_CreateImpactMarkInternal(localClientNum, elemDef, spatialFrame, randomSeed, markEntnum);
}

void __cdecl FX_CreateImpactMarkInternal(
    int localClientNum,
    const FxElemDef *elemDef,
    const FxSpatialFrame *spatialFrame,
    int randomSeed,
    unsigned int markEntnum)
{
    FxElemVisualState visState; // [esp+28h] [ebp-44h] BYREF
    FxMarkAlphaFade markAlpha; // [esp+44h] [ebp-28h] BYREF
    FxElemPreVisualState preVisState; // [esp+4Ch] [ebp-20h] BYREF
    FxElemMarkVisuals *markVisuals; // [esp+68h] [ebp-4h]

    FX_SetupVisualState(elemDef, 0, randomSeed, 0.0, &preVisState);
    visState.size[0] = (float)((float)((float)(fx_randomTable[randomSeed + 26] * preVisState.refState->amplitude.size[0])
        + preVisState.refState->base.size[0])
        * preVisState.sampleLerpInv)
        + (float)((float)((float)(fx_randomTable[randomSeed + 26] * preVisState.refState[1].amplitude.size[0])
            + preVisState.refState[1].base.size[0])
            * preVisState.sampleLerp);
    FX_EvaluateVisualState(localClientNum, &preVisState, 1.0, &visState);
    if (elemDef->lifeSpanMsec.base <= 1 || elemDef->alphaFadeTimeMsec >= elemDef->lifeSpanMsec.base)
    {
        markAlpha.ageLimitMsec = 0;
        markAlpha.fadeStartAgeMsec = 0;
    }
    else
    {
        markAlpha.ageLimitMsec = elemDef->lifeSpanMsec.base
            + (((elemDef->lifeSpanMsec.amplitude + 1) * LOWORD(fx_randomTable[randomSeed + 17])) >> 16);
        markAlpha.fadeStartAgeMsec = markAlpha.ageLimitMsec - elemDef->alphaFadeTimeMsec;
    }
    markVisuals = &elemDef->visuals.markArray[(elemDef->visualCount * LOWORD(fx_randomTable[randomSeed + 21])) >> 16];
    FX_ImpactMark(
        localClientNum,
        markVisuals->materials[1],
        markVisuals->materials[0],
        spatialFrame->origin,
        spatialFrame->quat,
        visState.rotationTotal,
        visState.color,
        visState.size[0],
        markEntnum,
        &markAlpha);
}

void __cdecl FX_ImpactMark(
                int localClientNum,
                Material *worldMaterial,
                Material *modelMaterial,
                const float *origin,
                const float *quat,
                float orientation,
                const unsigned __int8 *nativeColor,
                float radius,
                unsigned int markEntnum,
                const FxMarkAlphaFade *markAlpha)
{
    FxMarksSystem *marksSystem; // [esp+30h] [ebp-2Ch]
    float axis[3][3]; // [esp+38h] [ebp-24h] BYREF

    PROF_SCOPED("FX_ImpactMark");

    marksSystem = FX_GetMarksSystem(localClientNum);
    if ( fx_marks->current.enabled && !marksSystem->noMarks )
    {
        if ( radius >= 0.1 )
        {
            if ( radius <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                            1059,
                            0,
                            "%s\n\t(radius) = %g",
                            "(radius > 0)",
                            radius) )
            {
                __debugbreak();
            }
            UnitQuatToAxis(quat, axis);
            RotatePointAroundVector(axis[2], axis[0], axis[1], orientation * 57.295776);
            Vec3Cross(axis[0], axis[2], axis[1]);
            if ( fx_marks_draw_impact_axis->current.enabled )
                CL_AddDebugAxis(origin, axis, 20.0, 1, 700);
            if ( !Material_IsForSeeThruBulletHoleDecal(worldMaterial) )
            {
                FX_ImpactMark_Generate(
                    localClientNum,
                    MARK_FRAGMENTS_AGAINST_BRUSHES,
                    worldMaterial,
                    origin,
                    axis,
                    orientation,
                    nativeColor,
                    radius,
                    markEntnum,
                    markAlpha,
                    0);
                if ( fx_marks_smodels->current.enabled || fx_marks_ents->current.enabled )
                    FX_ImpactMark_Generate(
                        localClientNum,
                        MARK_FRAGMENTS_AGAINST_MODELS,
                        modelMaterial,
                        origin,
                        axis,
                        orientation,
                        nativeColor,
                        radius,
                        markEntnum,
                        markAlpha,
                        0);
            }
        }
    }
}

void __cdecl FX_ImpactMark_Generate(
                int localClientNum,
                MarkFragmentsAgainstEnum markAgainst,
                Material *material,
                const float *origin,
                const float (*axis)[3],
                float orientation,
                const unsigned __int8 *nativeColor,
                float radius,
                unsigned int markEntnum,
                const FxMarkAlphaFade *markAlpha,
                bool isSeeThruDecal)
{
    FxMarkPoint *FxMarkPointsBuffer; // eax
    FxMarkTri tris[256]; // [esp+230h] [ebp-10C8h] BYREF
    MarkInfo markInfo; // [esp+E28h] [ebp-4D0h] BYREF
    float *viewOffset; // [esp+12E0h] [ebp-18h]
    unsigned int callbackContext[5]; // [esp+12E4h] [ebp-14h] BYREF

    PROF_SCOPED("FX_ImpactMark_Generate");

    viewOffset = (float *)FX_GetSystem(localClientNum);
    if ( CG_IsShowingZombieMap() )
    {
        return;
    }
    else
    {
        callbackContext[0] = localClientNum;
        callbackContext[1] = (unsigned int)material;
        *(float *)&callbackContext[2] = radius;
        callbackContext[3] = (unsigned int)nativeColor;
        callbackContext[4] = (unsigned int)markAlpha;
        markInfo.isSeeThruDecal = isSeeThruDecal;
        if ( fx_marks->current.enabled
            && (markAgainst != MARK_FRAGMENTS_AGAINST_MODELS
             || fx_marks_ents->current.enabled
             || fx_marks_smodels->current.enabled) )
        {
            R_MarkFragments_Begin(&markInfo, markAgainst, origin, axis, radius, viewOffset, material);
            if ( fx_marks_ents->current.enabled )
            {
                if ( (unsigned int)markAgainst >= 2
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                                983,
                                0,
                                "%s\n\t(markAgainst) = %i",
                                "(markAgainst == MARK_FRAGMENTS_AGAINST_MODELS || markAgainst == MARK_FRAGMENTS_AGAINST_BRUSHES)",
                                markAgainst) )
                {
                    __debugbreak();
                }
                if ( markAgainst == MARK_FRAGMENTS_AGAINST_MODELS )
                    FX_ImpactMark_Generate_AddEntityModel(localClientNum, &markInfo, markEntnum, origin, radius);
                else
                    FX_ImpactMark_Generate_AddEntityBrush(localClientNum, &markInfo, markEntnum, origin, radius);
            }
            FxMarkPointsBuffer = FX_GetFxMarkPointsBuffer();
            R_MarkFragments_Go(
                &markInfo,
                FX_ImpactMark_Generate_Callback,
                callbackContext,
                255,
                tris,
                765,
                FxMarkPointsBuffer);
        }
    }
}

FxMarkPoint *__cdecl FX_GetFxMarkPointsBuffer()
{
    return &g_fxMarkPoints[765 * (_InterlockedIncrement(&g_fxMarkPointsCurrentBuffer) % 4)];
}

void __cdecl FX_ImpactMark_Generate_AddEntityBrush(
                int localClientNum,
                MarkInfo *markInfo,
                unsigned int entityIndex,
                const float *origin,
                float radius)
{
    int v5; // [esp+4h] [ebp-210h]
    int v6; // [esp+8h] [ebp-20Ch]
    int v7; // [esp+Ch] [ebp-208h]
    int v8; // [esp+14h] [ebp-200h]
    int v9; // [esp+18h] [ebp-1FCh]
    int v10; // [esp+1Ch] [ebp-1F8h]
    int v11; // [esp+24h] [ebp-1F0h]
    int v12; // [esp+28h] [ebp-1ECh]
    int v13; // [esp+2Ch] [ebp-1E8h]
    float v14; // [esp+134h] [ebp-E0h]
    float v15; // [esp+138h] [ebp-DCh]
    float v16; // [esp+13Ch] [ebp-D8h]
    float entAxis[3][3]; // [esp+170h] [ebp-A4h] BYREF
    unsigned __int16 entityIndexAsUnsignedShort; // [esp+194h] [ebp-80h]
    float4 worldModelBoundsFloat4[2]; // [esp+198h] [ebp-7Ch]
    float markMins[3]; // [esp+1B8h] [ebp-5Ch] BYREF
    float markMaxs[3]; // [esp+1C4h] [ebp-50h] BYREF
    float4 modelBounds[2]; // [esp+1D0h] [ebp-44h]
    GfxBrushModel *brushModel; // [esp+1F4h] [ebp-20h]
    centity_s *ent; // [esp+1F8h] [ebp-1Ch]
    float worldModelBoundsVec3[2][3]; // [esp+1FCh] [ebp-18h] BYREF

    if ( entityIndex != 1023 )
    {
        markMins[0] = origin[0] + (-(radius));
        markMins[1] = origin[1] + (-(radius));
        markMins[2] = origin[2] + (-(radius));
        markMaxs[0] = origin[0] + radius;
        markMaxs[1] = origin[1] + radius;
        markMaxs[2] = origin[2] + radius;
        ent = CG_GetEntity(localClientNum, entityIndex);
        if (((*((_DWORD *)ent + 201) >> 1) & 1) != 0 && ent->nextState.solid == 0xFFFFFF)
        {
            brushModel = R_GetBrushModel(ent->nextState.index.brushmodel);
            AnglesToAxis(ent->pose.angles, entAxis);
            *(_QWORD *)modelBounds[0].v = *(_QWORD *)&brushModel->bounds[0][0];
            modelBounds[0].u[2] = LODWORD(brushModel->bounds[0][2]);
            modelBounds[0].u[3] = 0;
            modelBounds[1].v[0] = brushModel->bounds[1][0];
            modelBounds[1].v[1] = brushModel->bounds[1][1];
            modelBounds[1].v[2] = brushModel->bounds[1][2];
            modelBounds[1].u[3] = 0;
            v14 = ent->pose.origin[0];
            v15 = ent->pose.origin[1];
            v16 = ent->pose.origin[2];
            v13 = entAxis[0][0] >= 0.0 ? 0 : -1;
            v12 = entAxis[0][1] >= 0.0 ? 0 : -1;
            v11 = entAxis[0][2] >= 0.0 ? 0 : -1;
            v10 = entAxis[1][0] >= 0.0 ? 0 : -1;
            v9 = entAxis[1][1] >= 0.0 ? 0 : -1;
            v8 = entAxis[1][2] >= 0.0 ? 0 : -1;
            v7 = entAxis[2][0] >= 0.0 ? 0 : -1;
            v6 = entAxis[2][1] >= 0.0 ? 0 : -1;
            v5 = entAxis[2][2] >= 0.0 ? 0 : -1;
            worldModelBoundsFloat4[0].v[0] = (float)(COERCE_FLOAT(modelBounds[1].u[0] & v13 | modelBounds[0].u[0] & ~v13)
                                                                                         * entAxis[0][0])
                                                                         + v14;
            worldModelBoundsFloat4[0].v[1] = (float)(COERCE_FLOAT(modelBounds[1].u[0] & v12 | modelBounds[0].u[0] & ~v12)
                                                                                         * entAxis[0][1])
                                                                         + v15;
            worldModelBoundsFloat4[0].v[2] = (float)(COERCE_FLOAT(modelBounds[1].u[0] & v11 | modelBounds[0].u[0] & ~v11)
                                                                                         * entAxis[0][2])
                                                                         + v16;
            worldModelBoundsFloat4[0].v[3] = (float)(modelBounds[0].v[0] * 0.0) + 0.0;
            worldModelBoundsFloat4[0].v[0] = (float)(COERCE_FLOAT(modelBounds[1].u[1] & v10 | modelBounds[0].u[1] & ~v10)
                                                                                         * entAxis[1][0])
                                                                         + worldModelBoundsFloat4[0].v[0];
            worldModelBoundsFloat4[0].v[1] = (float)(COERCE_FLOAT(modelBounds[1].u[1] & v9 | modelBounds[0].u[1] & ~v9)
                                                                                         * entAxis[1][1])
                                                                         + worldModelBoundsFloat4[0].v[1];
            worldModelBoundsFloat4[0].v[2] = (float)(COERCE_FLOAT(modelBounds[1].u[1] & v8 | modelBounds[0].u[1] & ~v8)
                                                                                         * entAxis[1][2])
                                                                         + worldModelBoundsFloat4[0].v[2];
            worldModelBoundsFloat4[0].v[3] = (float)(modelBounds[0].v[1] * 0.0) + worldModelBoundsFloat4[0].v[3];
            worldModelBoundsFloat4[0].v[0] = (float)(COERCE_FLOAT(modelBounds[1].u[2] & v7 | modelBounds[0].u[2] & ~v7)
                                                                                         * entAxis[2][0])
                                                                         + worldModelBoundsFloat4[0].v[0];
            worldModelBoundsFloat4[0].v[1] = (float)(COERCE_FLOAT(modelBounds[1].u[2] & v6 | modelBounds[0].u[2] & ~v6)
                                                                                         * entAxis[2][1])
                                                                         + worldModelBoundsFloat4[0].v[1];
            worldModelBoundsFloat4[0].v[2] = (float)(COERCE_FLOAT(modelBounds[1].u[2] & v5 | modelBounds[0].u[2] & ~v5)
                                                                                         * entAxis[2][2])
                                                                         + worldModelBoundsFloat4[0].v[2];
            worldModelBoundsFloat4[0].v[3] = (float)(modelBounds[0].v[2] * 0.0) + worldModelBoundsFloat4[0].v[3];
            worldModelBoundsFloat4[1].v[0] = (float)(COERCE_FLOAT(modelBounds[0].u[0] & v13 | modelBounds[1].u[0] & ~v13)
                                                                                         * entAxis[0][0])
                                                                         + v14;
            worldModelBoundsFloat4[1].v[1] = (float)(COERCE_FLOAT(modelBounds[0].u[0] & v12 | modelBounds[1].u[0] & ~v12)
                                                                                         * entAxis[0][1])
                                                                         + v15;
            worldModelBoundsFloat4[1].v[2] = (float)(COERCE_FLOAT(modelBounds[0].u[0] & v11 | modelBounds[1].u[0] & ~v11)
                                                                                         * entAxis[0][2])
                                                                         + v16;
            worldModelBoundsFloat4[1].v[3] = (float)(modelBounds[1].v[0] * 0.0) + 0.0;
            worldModelBoundsFloat4[1].v[0] = (float)(COERCE_FLOAT(modelBounds[0].u[1] & v10 | modelBounds[1].u[1] & ~v10)
                                                                                         * entAxis[1][0])
                                                                         + worldModelBoundsFloat4[1].v[0];
            worldModelBoundsFloat4[1].v[1] = (float)(COERCE_FLOAT(modelBounds[0].u[1] & v9 | modelBounds[1].u[1] & ~v9)
                                                                                         * entAxis[1][1])
                                                                         + worldModelBoundsFloat4[1].v[1];
            worldModelBoundsFloat4[1].v[2] = (float)(COERCE_FLOAT(modelBounds[0].u[1] & v8 | modelBounds[1].u[1] & ~v8)
                                                                                         * entAxis[1][2])
                                                                         + worldModelBoundsFloat4[1].v[2];
            worldModelBoundsFloat4[1].v[3] = (float)(modelBounds[1].v[1] * 0.0) + worldModelBoundsFloat4[1].v[3];
            worldModelBoundsFloat4[1].v[0] = (float)(COERCE_FLOAT(modelBounds[0].u[2] & v7 | modelBounds[1].u[2] & ~v7)
                                                                                         * entAxis[2][0])
                                                                         + worldModelBoundsFloat4[1].v[0];
            worldModelBoundsFloat4[1].v[1] = (float)(COERCE_FLOAT(modelBounds[0].u[2] & v6 | modelBounds[1].u[2] & ~v6)
                                                                                         * entAxis[2][1])
                                                                         + worldModelBoundsFloat4[1].v[1];
            worldModelBoundsFloat4[1].v[2] = (float)(COERCE_FLOAT(modelBounds[0].u[2] & v5 | modelBounds[1].u[2] & ~v5)
                                                                                         * entAxis[2][2])
                                                                         + worldModelBoundsFloat4[1].v[2];
            worldModelBoundsFloat4[1].v[3] = (float)(modelBounds[1].v[2] * 0.0) + worldModelBoundsFloat4[1].v[3];
            *(_QWORD *)&worldModelBoundsVec3[0][0] = *(_QWORD *)worldModelBoundsFloat4[0].v;
            LODWORD(worldModelBoundsVec3[0][2]) = worldModelBoundsFloat4[0].u[2];
            worldModelBoundsVec3[1][0] = worldModelBoundsFloat4[1].v[0];
            worldModelBoundsVec3[1][1] = worldModelBoundsFloat4[1].v[1];
            worldModelBoundsVec3[1][2] = worldModelBoundsFloat4[1].v[2];
            if ( BoundsOverlap(markMins, markMaxs, worldModelBoundsVec3[0], worldModelBoundsVec3[1]) )
            {
                entityIndexAsUnsignedShort = entityIndex;
                if ( (unsigned __int16)entityIndex != entityIndex
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                                811,
                                0,
                                "%s\n\t(entityIndex) = %i",
                                "(entityIndexAsUnsignedShort == entityIndex)",
                                entityIndex) )
                {
                    __debugbreak();
                }
                R_MarkFragments_AddBModel(markInfo, brushModel, &ent->pose, entityIndexAsUnsignedShort);
            }
        }
    }
}

void __cdecl FX_ImpactMark_Generate_AddEntityModel(
                int localClientNum,
                MarkInfo *markInfo,
                unsigned int entityIndex,
                const float *origin,
                float radius)
{
    float dObjRadius; // [esp+18h] [ebp-14h]
    centity_s *ent; // [esp+20h] [ebp-Ch]
    DObj *dObj; // [esp+28h] [ebp-4h]

    if ( entityIndex != 1023 )
    {
        ent = CG_GetEntity(localClientNum, entityIndex);
        if ( ((*((unsigned int *)ent + 201) >> 1) & 1) != 0 )
        {
            dObj = Com_GetClientDObj(ent->nextState.number, localClientNum);
            if ( dObj )
            {
                dObjRadius = DObjGetRadius(dObj);
                if ( Vec3DistanceSq(origin, ent->pose.origin) <= (float)((float)(dObjRadius + radius)
                                                                                                                             * (float)(dObjRadius + radius)) )
                {
                    if ( (unsigned __int16)entityIndex != entityIndex
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                                    861,
                                    0,
                                    "%s\n\t(entityIndex) = %i",
                                    "(entityIndexAsUnsignedShort == entityIndex)",
                                    entityIndex) )
                    {
                        __debugbreak();
                    }
                    R_MarkFragments_AddDObj(markInfo, dObj, &ent->pose, entityIndex);
                }
            }
        }
    }
}

void __cdecl FX_ImpactMark_Generate_Callback(
                void *context,
                int triCount,
                FxMarkTri *tris,
                int pointCount,
                FxMarkPoint *points,
                const float *markOrigin,
                const float *markTexCoordAxis,
                const float *markHitNormal,
                bool isSeeThruDecal)
{
    FX_AllocAndConstructMark(
        *(unsigned int *)context,
        triCount,
        pointCount,
        *((Material **)context + 1),
        tris,
        points,
        markOrigin,
        markHitNormal,
        *((float *)context + 2),
        markTexCoordAxis,
        *((const unsigned __int8 **)context + 3),
        *((const FxMarkAlphaFade **)context + 4),
        isSeeThruDecal);
}

bool __cdecl FX_CompareMarkTris(const FxMarkTri &tri0, const FxMarkTri &tri1)
{
    int contextCompareResult; // [esp+10h] [ebp-4h]

    contextCompareResult = FX_MarkContextsCompare(&tri0.context, &tri1.context);
    if (contextCompareResult)
        return contextCompareResult > 0;
    else
        return tri0.indices[0] < (int)tri1.indices[0];
}

void __cdecl FX_AllocAndConstructMark(
                unsigned int localClientNum,
                int triCount,
                int pointCount,
                Material *material,
                FxMarkTri *markTris,
                const FxMarkPoint *markPoints,
                const float *origin,
                const float *hitNormal,
                float radius,
                const float *texCoordAxis,
                const unsigned __int8 *nativeColor,
                const FxMarkAlphaFade *markAlpha,
                bool isSeeThruDecal)
{
    int fadeStartAgeMsec; // eax
    FxMark *v14; // ecx
    float *v15; // [esp+8h] [ebp-64h]
    float *v16; // [esp+Ch] [ebp-60h]
    float *v17; // [esp+10h] [ebp-5Ch]
    unsigned __int16 staticModelMarkHead; // [esp+48h] [ebp-24h] BYREF
    unsigned __int16 newMarkHandle; // [esp+4Ch] [ebp-20h]
    FxMarksSystem *marksSystem; // [esp+50h] [ebp-1Ch]
    int points; // [esp+54h] [ebp-18h]
    unsigned int limit; // [esp+58h] [ebp-14h]
    int modelType; // [esp+5Ch] [ebp-10h]
    FxMark *newMark; // [esp+60h] [ebp-Ch]
    int tris; // [esp+64h] [ebp-8h]
    unsigned __int8 flags; // [esp+6Bh] [ebp-1h]

    marksSystem = FX_GetMarksSystem(localClientNum);
    if ( !isSeeThruDecal || !FX_SeeThruMarkOverlaps(marksSystem, origin, radius, hitNormal) )
    {
        //std::_Sort<FxMarkTri *,int,bool (__cdecl *)(FxMarkTri const &,FxMarkTri const &)>(
        //    markTris,
        //    &markTris[triCount],
        //    12 * triCount / 12,
        //    FX_CompareMarkTris);

        std::sort(markTris, markTris + triCount, FX_CompareMarkTris);

        Sys_EnterCriticalSection(CRITSECT_ALLOC_MARK);
        if (_InterlockedIncrement(&g_markThread[localClientNum]) != 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                        641,
                        0,
                        "%s",
                        "Sys_InterlockedIncrement( &g_markThread[localClientNum] ) == 1") )
        {
            __debugbreak();
        }
        marksSystem = FX_GetMarksSystem(localClientNum);
        tris = FX_AllocMarkTris(marksSystem, markTris, triCount);
        points = FX_AllocMarkPoints(marksSystem, pointCount);
        if ( marksSystem->firstFreeMarkHandle == 0xFFFF )
        {
            FX_FreeLruMark(marksSystem);
            if ( marksSystem->firstFreeMarkHandle == 0xFFFF
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                            654,
                            0,
                            "%s",
                            "marksSystem->firstFreeMarkHandle != FX_HANDLE_NONE") )
            {
                __debugbreak();
            }
        }
        limit = 256;
        if ( CL_LocalClient_GetActiveCount() > 1 )
            limit >>= 1;
        while ( marksSystem->allocedMarkCount - marksSystem->freedMarkCount > limit )
            FX_FreeLruMark(marksSystem);
        newMarkHandle = marksSystem->firstFreeMarkHandle;
        newMark = FX_MarkFromHandle(marksSystem, newMarkHandle);
        if ( !newMark
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp", 666, 0, "%s", "newMark") )
        {
            __debugbreak();
        }
        marksSystem->firstFreeMarkHandle = newMark->nextMark;
        if ( marksSystem->frameCount <= 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                        673,
                        0,
                        "%s\n\t(marksSystem->frameCount) = %i",
                        "(marksSystem->frameCount > 0)",
                        marksSystem->frameCount) )
        {
            __debugbreak();
        }
        newMark->context = markTris->context;
        newMark->material = material;
        newMark->radius = radius;
        v17 = newMark->origin;
        newMark->origin[0] = *origin;
        v17[1] = origin[1];
        v17[2] = origin[2];
        v16 = newMark->texCoordAxis;
        newMark->texCoordAxis[0] = *texCoordAxis;
        v16[1] = texCoordAxis[1];
        v16[2] = texCoordAxis[2];
        v15 = newMark->hitNormal;
        newMark->hitNormal[0] = *hitNormal;
        v15[1] = hitNormal[1];
        v15[2] = hitNormal[2];
        *(unsigned int *)newMark->nativeColor = *(unsigned int *)nativeColor;
        newMark->ageMsec = 0;
        fadeStartAgeMsec = markAlpha->fadeStartAgeMsec;
        v14 = newMark;
        newMark->alphaFade.ageLimitMsec = markAlpha->ageLimitMsec;
        v14->alphaFade.fadeStartAgeMsec = fadeStartAgeMsec;
        flags = 0;
        if ( isSeeThruDecal )
            flags |= 1u;
        newMark->markFlags = flags;
        modelType = newMark->context.modelTypeAndSurf & 0xC0;
        if ( modelType == 192 || modelType == 128 )
        {
            if ( newMark->context.modelIndex >= 0x600u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                            696,
                            0,
                            "newMark->context.modelIndex doesn't index ARRAY_COUNT( marksSystem->entFirstMarkHandles )\n"
                            "\t%i not in [0, %i)",
                            newMark->context.modelIndex,
                            1536) )
            {
                __debugbreak();
            }
            FX_LinkMarkIntoList(marksSystem, &marksSystem->entFirstMarkHandles[newMark->context.modelIndex], newMark);
        }
        else if ( modelType == 64 )
        {
            staticModelMarkHead = FX_FindModelHead(marksSystem, newMark->context.modelIndex, 64);
            if ( staticModelMarkHead == newMarkHandle
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                            709,
                            0,
                            "%s",
                            "staticModelMarkHead != newMarkHandle") )
            {
                __debugbreak();
            }
            FX_LinkMarkIntoList(marksSystem, &staticModelMarkHead, newMark);
        }
        else
        {
            if ( modelType
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                            720,
                            0,
                            "%s\n\t(modelType) = %i",
                            "(modelType == MARK_MODEL_TYPE_WORLD_BRUSH)",
                            modelType) )
            {
                __debugbreak();
            }
            FX_LinkMarkIntoList(marksSystem, &marksSystem->firstActiveWorldMarkHandle, newMark);
        }
        if ( newMark->frameCountDrawn != -1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                        727,
                        0,
                        "%s",
                        "newMark->frameCountDrawn == FX_MARK_FREE") )
        {
            __debugbreak();
        }
        newMark->frameCountDrawn = marksSystem->frameCount - 1;
        newMark->frameCountAlloced = marksSystem->frameCount;
        newMark->tris = tris;
        newMark->triCount = triCount;
        if ( newMark->triCount != triCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                        734,
                        0,
                        "newMark->triCount == triCount\n\t%i, %i",
                        newMark->triCount,
                        triCount) )
        {
            __debugbreak();
        }
        newMark->points = points;
        newMark->pointCount = pointCount;
        if ( newMark->pointCount != pointCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                        738,
                        0,
                        "newMark->pointCount == pointCount\n\t%i, %i",
                        newMark->pointCount,
                        pointCount) )
        {
            __debugbreak();
        }
        FX_CopyMarkTris(marksSystem, markTris, newMark->tris, triCount);
        FX_CopyMarkPoints(marksSystem, markPoints, newMark->points, pointCount);
        ++marksSystem->allocedMarkCount;
        //if ( _InterlockedDecrement((volatile signed __int32 *)(4 * localClientNum + 63629168)) )
        if (_InterlockedDecrement(&g_markThread[localClientNum]))
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                            748,
                            0,
                            "%s",
                            "Sys_InterlockedDecrement( &g_markThread[localClientNum] ) == 0") )
                __debugbreak();
        }
        Sys_LeaveCriticalSection(CRITSECT_ALLOC_MARK);
    }
}

void __cdecl FX_FreeLruMark(FxMarksSystem *marksSystem)
{
    FxMark *lruMark; // [esp+0h] [ebp-Ch]
    FxMark *mark; // [esp+4h] [ebp-8h]

    if ( !marksSystem
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp", 318, 0, "%s", "marksSystem") )
    {
        __debugbreak();
    }
    lruMark = 0;
    for ( mark = marksSystem->marks; mark != (FxMark *)marksSystem->triGroups; ++mark )
    {
        if ( mark->frameCountDrawn != -1 )
        {
            if ( FX_MarkIsAlphaFadedOut(mark) )
            {
                FX_FreeMark(marksSystem, mark);
                return;
            }
            if ( !lruMark
                || mark->frameCountDrawn < lruMark->frameCountDrawn
                || mark->frameCountDrawn == lruMark->frameCountDrawn && mark->frameCountAlloced < lruMark->frameCountAlloced )
            {
                lruMark = mark;
            }
        }
    }
    FX_FreeMark(marksSystem, lruMark);
}

int __cdecl FX_AllocMarkTris(FxMarksSystem *marksSystem, const FxMarkTri *markTris, int triCount)
{
    int groupHandle; // [esp+Ch] [ebp-Ch]
    int usedCount; // [esp+10h] [ebp-8h]
    FxTriGroupPool *newGroup; // [esp+14h] [ebp-4h]

    groupHandle = 0xFFFF;
    do
    {
        if ( !marksSystem->firstFreeTriGroup )
        {
            FX_FreeLruMark(marksSystem);
            if ( !marksSystem->firstFreeTriGroup
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                            354,
                            0,
                            "%s",
                            "marksSystem->firstFreeTriGroup") )
            {
                __debugbreak();
            }
        }
        newGroup = marksSystem->firstFreeTriGroup;
        marksSystem->firstFreeTriGroup = newGroup->nextFreeTriGroup;
        newGroup->triGroup.next = groupHandle;
        groupHandle = FX_TriGroupToHandle(marksSystem, (FxTriGroup *)newGroup);
        if ( triCount >= 2 && FX_MarkContextsEqual(&markTris->context, &markTris[1].context) )
            usedCount = 2;
        else
            usedCount = 1;
        markTris += usedCount;
        triCount -= usedCount;
    }
    while ( triCount );
    return groupHandle;
}

bool __cdecl FX_MarkContextsEqual(const GfxMarkContext *contextA, const GfxMarkContext *contextB)
{
    return (((__int16)contextA->modelIndex - (__int16)contextB->modelIndex)
                | (*(__int16 *)&contextA->reflectionProbeIndex - *(__int16 *)&contextB->reflectionProbeIndex)
                | (*(__int16 *)&contextA->lmapIndex - *(__int16 *)&contextB->lmapIndex)) == 0;
}

int __cdecl FX_TriGroupToHandle(FxMarksSystem *marksSystem, FxTriGroup *group)
{
    unsigned int handle; // [esp+0h] [ebp-4h]

    if ( !marksSystem
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_marks.h", 199, 0, "%s", "marksSystem") )
    {
        __debugbreak();
    }
    if ( !group && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_marks.h", 200, 0, "%s", "group") )
        __debugbreak();
    handle = (char *)group - (char *)marksSystem->triGroups;
    if ( handle >= 0x6000
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_marks.h",
                    203,
                    0,
                    "handle doesn't index FX_TRI_GROUP_LIMIT * sizeof( FxTriGroup )\n\t%i not in [0, %i)",
                    handle,
                    24576) )
    {
        __debugbreak();
    }
    return (char *)group - (char *)marksSystem->triGroups;
}

int __cdecl FX_AllocMarkPoints(FxMarksSystem *marksSystem, int pointCount)
{
    int groupHandle; // [esp+4h] [ebp-Ch]
    FxPointGroupPool *newGroup; // [esp+8h] [ebp-8h]
    int pointGroupCount; // [esp+Ch] [ebp-4h]

    groupHandle = 0xFFFF;
    pointGroupCount = (pointCount + 1) / 2;
    if ( pointGroupCount < 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    382,
                    0,
                    "%s\n\t(pointGroupCount) = %i",
                    "(pointGroupCount >= 1)",
                    pointGroupCount) )
    {
        __debugbreak();
    }
    do
    {
        if ( !marksSystem->firstFreePointGroup )
        {
            FX_FreeLruMark(marksSystem);
            if ( !marksSystem->firstFreePointGroup
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                            388,
                            0,
                            "%s",
                            "marksSystem->firstFreePointGroup") )
            {
                __debugbreak();
            }
        }
        newGroup = marksSystem->firstFreePointGroup;
        marksSystem->firstFreePointGroup = newGroup->nextFreePointGroup;
        newGroup->pointGroup.next = groupHandle;
        groupHandle = FX_PointGroupToHandle(marksSystem, (FxPointGroup *)newGroup);
        --pointGroupCount;
    }
    while ( pointGroupCount );
    return groupHandle;
}

int __cdecl FX_PointGroupToHandle(FxMarksSystem *marksSystem, FxPointGroup *group)
{
    unsigned int handle; // [esp+0h] [ebp-4h]

    if ( !marksSystem
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_marks.h", 224, 0, "%s", "marksSystem") )
    {
        __debugbreak();
    }
    if ( !group && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_marks.h", 225, 0, "%s", "group") )
        __debugbreak();
    handle = (char *)group - (char *)marksSystem->pointGroups;
    if ( handle >= 0x19800
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_marks.h",
                    228,
                    0,
                    "handle doesn't index FX_POINT_GROUP_LIMIT * sizeof( FxPointGroup )\n\t%i not in [0, %i)",
                    handle,
                    104448) )
    {
        __debugbreak();
    }
    return (char *)group - (char *)marksSystem->pointGroups;
}

void __cdecl FX_LinkMarkIntoList(FxMarksSystem *marksSystem, unsigned __int16 *head, FxMark *mark)
{
    unsigned __int16 iterMarkPrev; // [esp+18h] [ebp-1Ch]
    float radiusSum; // [esp+1Ch] [ebp-18h]
    unsigned __int16 *iterHandlePrev; // [esp+20h] [ebp-14h]
    FxMark *nextMark; // [esp+24h] [ebp-10h]
    FxMark *iterMark; // [esp+28h] [ebp-Ch]
    float distSq; // [esp+2Ch] [ebp-8h]
    unsigned __int16 markHandle; // [esp+30h] [ebp-4h]

    iterMarkPrev = -1;
    for ( iterHandlePrev = head; *iterHandlePrev != 0xFFFF; iterHandlePrev = &iterMark->nextMark )
    {
        iterMark = FX_MarkFromHandle(marksSystem, *iterHandlePrev);
        if ( iterMark->material == mark->material )
        {
            if ( FX_MarkContextsEqual(&iterMark->context, &mark->context) )
                break;
        }
        distSq = Vec3DistanceSq(mark->origin, iterMark->origin);
        radiusSum = mark->radius + iterMark->radius;
        if ( (float)(radiusSum * radiusSum) > distSq )
        {
            iterMarkPrev = -1;
            iterHandlePrev = head;
            break;
        }
        iterMarkPrev = *iterHandlePrev;
    }
    markHandle = FX_MarkToHandle(marksSystem, mark);
    mark->nextMark = *iterHandlePrev;
    mark->prevMark = iterMarkPrev;
    if ( mark->nextMark != 0xFFFF )
    {
        nextMark = FX_MarkFromHandle(marksSystem, mark->nextMark);
        if ( nextMark->prevMark != mark->prevMark
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                        446,
                        0,
                        "%s",
                        "nextMark->prevMark == mark->prevMark") )
        {
            __debugbreak();
        }
        nextMark->prevMark = markHandle;
    }
    *iterHandlePrev = markHandle;
}

void __cdecl FX_CopyMarkTris(
                FxMarksSystem *marksSystem,
                const FxMarkTri *srcTris,
                unsigned int dstGroupHandle,
                int triCount)
{
    int v4; // [esp+4h] [ebp-1Ch]
    int copyIndex; // [esp+18h] [ebp-8h]
    FxTriGroupPool *dstGroup; // [esp+1Ch] [ebp-4h]

    do
    {
        dstGroup = FX_TriGroupFromHandle(marksSystem, dstGroupHandle);
        if ( triCount < 2 )
            v4 = triCount;
        else
            v4 = 2;
        copyIndex = 0;
        dstGroup->triGroup.context = srcTris->context;
        do
        {
            dstGroup->triGroup.indices[copyIndex][0] = srcTris[copyIndex].indices[0];
            dstGroup->triGroup.indices[copyIndex][1] = srcTris[copyIndex].indices[1];
            dstGroup->triGroup.indices[copyIndex][2] = srcTris[copyIndex].indices[2];
            ++copyIndex;
        }
        while ( copyIndex != v4 && FX_MarkContextsEqual(&srcTris[copyIndex].context, &dstGroup->triGroup.context) );
        dstGroupHandle = dstGroup->triGroup.next;
        dstGroup->triGroup.triCount = copyIndex;
        srcTris += dstGroup->triGroup.triCount;
        triCount -= dstGroup->triGroup.triCount;
    }
    while ( triCount );
    if ( dstGroupHandle != 0xFFFF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    481,
                    0,
                    "%s\n\t(dstGroupHandle) = %i",
                    "(dstGroupHandle == 0xffff)",
                    dstGroupHandle) )
    {
        __debugbreak();
    }
}

void __cdecl FX_CopyMarkPoints(
                FxMarksSystem *marksSystem,
                const FxMarkPoint *srcPoints,
                unsigned int dstGroupHandle,
                int pointCount)
{
    int v4; // [esp+8h] [ebp-14h]
    int copyIndex; // [esp+14h] [ebp-8h]
    FxPointGroupPool *dstGroup; // [esp+18h] [ebp-4h]

    do
    {
        dstGroup = FX_PointGroupFromHandle(marksSystem, dstGroupHandle);
        if ( pointCount > 2 )
            v4 = 2;
        else
            v4 = pointCount;
        if ( v4 <= 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                        496,
                        0,
                        "%s\n\t(copyCount) = %i",
                        "(copyCount > 0)",
                        v4) )
        {
            __debugbreak();
        }
        copyIndex = 0;
        do
        {
            memcpy((char *)dstGroup + 32 * copyIndex, &srcPoints[copyIndex], 0x20u);
            ++copyIndex;
        }
        while ( copyIndex < v4 );
        dstGroupHandle = dstGroup->pointGroup.next;
        srcPoints += v4;
        pointCount -= v4;
    }
    while ( pointCount );
    if ( dstGroupHandle != 0xFFFF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    508,
                    0,
                    "%s\n\t(dstGroupHandle) = %i",
                    "(dstGroupHandle == 0xffff)",
                    dstGroupHandle) )
    {
        __debugbreak();
    }
}

unsigned __int16 __cdecl FX_FindModelHead(FxMarksSystem *marksSystem, unsigned __int16 modelIndex, int type)
{
    FxMark *mark; // [esp+4h] [ebp-8h]

    for ( mark = marksSystem->marks; mark != (FxMark *)marksSystem->triGroups; ++mark )
    {
        if ( mark->frameCountDrawn != -1
            && mark->prevMark == 0xFFFF
            && (mark->context.modelTypeAndSurf & 0xC0) == type
            && mark->context.modelIndex == modelIndex )
        {
            return FX_MarkToHandle(marksSystem, mark);
        }
    }
    return -1;
}

int __cdecl FX_MarkContextsCompare(const GfxMarkContext *context0, const GfxMarkContext *context1)
{
    int type1; // [esp+0h] [ebp-10h]
    int type0; // [esp+8h] [ebp-8h]

    type0 = context0->modelTypeAndSurf & 0xC0;
    type1 = context1->modelTypeAndSurf & 0xC0;
    if ( type0 != type1 )
        return type1 - type0;
    if ( context0->primaryLightIndex != context1->primaryLightIndex )
        return context1->primaryLightIndex - context0->primaryLightIndex;
    if ( context0->reflectionProbeIndex != context1->reflectionProbeIndex )
        return context1->reflectionProbeIndex - context0->reflectionProbeIndex;
    if ( context0->lmapIndex != context1->lmapIndex )
        return context1->lmapIndex - context0->lmapIndex;
    if ( context0->modelIndex == context1->modelIndex )
        return (context1->modelTypeAndSurf & 0x3F) - (context0->modelTypeAndSurf & 0x3F);
    return context1->modelIndex - context0->modelIndex;
}

int __cdecl fx_add_markCallback(jqBatch *batch)
{
    const FxAddMarkCmd *cmd; // [esp+14h] [ebp-4h]

    cmd = (const FxAddMarkCmd *)jqLockData(batch);

    PROF_SCOPED("fx_add_mark");
    FX_CreateImpactMarkInternal(cmd->localClientNum, cmd->elemDef, &cmd->spatialFrame, cmd->randomSeed, cmd->markEntnum);
    jqUnlockData(batch);
    return 0;
}

void __cdecl FX_MarkEntDetachAll(unsigned int localClientNum, unsigned int entnum)
{
    FxMark *v2; // eax
    FxMarksSystem *marksSystem; // [esp+0h] [ebp-4h]

    marksSystem = FX_GetMarksSystem(localClientNum);
    if ( entnum >= 0x600
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    1220,
                    0,
                    "entnum doesn't index ARRAY_COUNT( marksSystem->entFirstMarkHandles )\n\t%i not in [0, %i)",
                    entnum,
                    1536) )
    {
        __debugbreak();
    }
    while ( marksSystem->entFirstMarkHandles[entnum] != 0xFFFF )
    {
        v2 = FX_MarkFromHandle(marksSystem, marksSystem->entFirstMarkHandles[entnum]);
        FX_FreeMark(marksSystem, v2);
    }
}

void __cdecl FX_MarkGlassDetachAll(unsigned int localClientNum, int glassIndex)
{
    FxMarksSystem *marksSystem; // [esp+0h] [ebp-Ch]
    FxMark *mark; // [esp+4h] [ebp-8h]
    unsigned __int16 markHandle; // [esp+8h] [ebp-4h]

    marksSystem = FX_GetMarksSystem(localClientNum);
    markHandle = marksSystem->firstActiveWorldMarkHandle;
    while ( markHandle != 0xFFFF )
    {
        mark = FX_MarkFromHandle(marksSystem, markHandle);
        markHandle = mark->nextMark;
        if ( mark->context.modelIndex == glassIndex )
            FX_FreeMark(marksSystem, mark);
    }
}

void __cdecl FX_MarkEntUpdateHidePartBits(
                const unsigned int *oldHidePartBits,
                const unsigned int *newHidePartBits,
                unsigned int localClientNum,
                unsigned int entnum)
{
    unsigned int v4; // edx
    FxMarksSystem *marksSystem; // [esp+0h] [ebp-24h]
    unsigned int unsetHidePartBits[5]; // [esp+8h] [ebp-1Ch] BYREF
    int hidePartIntIndex; // [esp+1Ch] [ebp-8h]
    unsigned int oredUnsetHidePartBits; // [esp+20h] [ebp-4h]

    marksSystem = FX_GetMarksSystem(localClientNum);
    oredUnsetHidePartBits = 0;
    for ( hidePartIntIndex = 0; hidePartIntIndex != 5; ++hidePartIntIndex )
    {
        v4 = newHidePartBits[hidePartIntIndex] & ~oldHidePartBits[hidePartIntIndex];
        unsetHidePartBits[hidePartIntIndex] = v4;
        oredUnsetHidePartBits |= v4;
    }
    if ( oredUnsetHidePartBits )
        FX_MarkEntDetachMatchingBones(marksSystem, entnum, unsetHidePartBits);
}

void __cdecl FX_MarkEntDetachMatchingBones(
                FxMarksSystem *marksSystem,
                unsigned int entnum,
                const unsigned int *unsetHidePartBits)
{
    unsigned __int16 handle; // [esp+0h] [ebp-Ch]
    FxMark *mark; // [esp+4h] [ebp-8h]
    int markBoneIndex; // [esp+8h] [ebp-4h]

    if ( entnum >= 0x600
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    1275,
                    0,
                    "entnum doesn't index ARRAY_COUNT( marksSystem->entFirstMarkHandles )\n\t%i not in [0, %i)",
                    entnum,
                    1536) )
    {
        __debugbreak();
    }
    handle = marksSystem->entFirstMarkHandles[entnum];
    while ( handle != 0xFFFF )
    {
        mark = FX_MarkFromHandle(marksSystem, handle);
        handle = mark->nextMark;
        if ( (mark->context.modelTypeAndSurf & 0xC0) == 0xC0 )
        {
            markBoneIndex = mark->context.lmapIndex;
            if ( markBoneIndex >> 5 >= 5
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                            1284,
                            0,
                            "%s\n\t(markBoneIndex) = %i",
                            "((markBoneIndex >> 5) < DOBJ_MAX_PART_BITS)",
                            markBoneIndex) )
            {
                __debugbreak();
            }
            if ( (mark->context.modelTypeAndSurf & 0x3F) != 0 )
            {
                FX_FreeMark(marksSystem, mark);
            }
            else if ( (unsetHidePartBits[markBoneIndex >> 5] & (0x80000000 >> (markBoneIndex & 0x1F))) != 0 )
            {
                FX_FreeMark(marksSystem, mark);
            }
        }
    }
}

void __cdecl FX_MarkEntUpdateBegin(
                FxMarkDObjUpdateContext *context,
                DObj *obj,
                bool isBrush,
                unsigned __int16 brushIndex)
{
    int modelCount; // [esp+0h] [ebp-8h]
    int modelIndex; // [esp+4h] [ebp-4h]

    if ( obj )
    {
        modelCount = DObjGetNumModels(obj);
        context->modelCount = modelCount;
        for ( modelIndex = 0; modelIndex != modelCount; ++modelIndex )
        {
            context->models[modelIndex] = DObjGetModel(obj, modelIndex);
            context->modelParentBones[modelIndex] = DObjGetModelParentBoneName(obj, modelIndex);
        }
    }
    else
    {
        context->modelCount = 0;
    }
    context->isBrush = isBrush;
    context->brushIndex = brushIndex;
}

void __cdecl FX_MarkEntUpdateEnd(
                FxMarkDObjUpdateContext *context,
                unsigned int localClientNum,
                unsigned int entnum,
                DObj *obj,
                bool isBrush,
                unsigned __int16 brushIndex)
{
    if ( context->isBrush && (!isBrush || context->brushIndex != brushIndex) )
        FX_MarkEntDetachAllOfType(localClientNum, entnum, 128);
    if ( context->modelCount )
    {
        if ( obj )
            FX_MarkEntUpdateEndDObj(context, localClientNum, entnum, obj);
        else
            FX_MarkEntDetachAllOfType(localClientNum, entnum, 192);
    }
}

void __cdecl FX_MarkEntDetachAllOfType(unsigned int localClientNum, unsigned int entnum, int markType)
{
    FxMarksSystem *marksSystem; // [esp+0h] [ebp-Ch]
    unsigned __int16 handle; // [esp+4h] [ebp-8h]
    FxMark *mark; // [esp+8h] [ebp-4h]

    marksSystem = FX_GetMarksSystem(localClientNum);
    if ( entnum >= 0x600
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    1253,
                    0,
                    "entnum doesn't index ARRAY_COUNT( marksSystem->entFirstMarkHandles )\n\t%i not in [0, %i)",
                    entnum,
                    1536) )
    {
        __debugbreak();
    }
    handle = marksSystem->entFirstMarkHandles[entnum];
    while ( handle != 0xFFFF )
    {
        mark = FX_MarkFromHandle(marksSystem, handle);
        handle = mark->nextMark;
        if ( (mark->context.modelTypeAndSurf & 0xC0) == markType )
            FX_FreeMark(marksSystem, mark);
    }
}

void __cdecl FX_MarkEntUpdateEndDObj(
                FxMarkDObjUpdateContext *context,
                unsigned int localClientNum,
                unsigned int entnum,
                DObj *obj)
{
    FxMarksSystem *marksSystem; // [esp+0h] [ebp-18h]
    int oldModelCount; // [esp+4h] [ebp-14h]
    int removedModelCount; // [esp+8h] [ebp-10h]
    int oldModelIndex; // [esp+Ch] [ebp-Ch]
    int modelCount; // [esp+10h] [ebp-8h]
    int modelIndex; // [esp+14h] [ebp-4h]

    marksSystem = FX_GetMarksSystem(localClientNum);
    oldModelCount = context->modelCount;
    modelCount = DObjGetNumModels(obj);
    modelIndex = 0;
    removedModelCount = 0;
    for ( oldModelIndex = 0; oldModelIndex != oldModelCount; ++oldModelIndex )
    {
        if ( modelIndex == modelCount )
        {
            while ( oldModelIndex != oldModelCount )
            {
                FX_MarkEntDetachModel(marksSystem, entnum, oldModelIndex - removedModelCount++);
                ++oldModelIndex;
            }
            return;
        }
        if ( DObjGetModel(obj, modelIndex) == context->models[oldModelIndex]
            && DObjGetModelParentBoneName(obj, modelIndex) == context->modelParentBones[oldModelIndex] )
        {
            ++modelIndex;
        }
        else
        {
            FX_MarkEntDetachModel(marksSystem, entnum, oldModelIndex - removedModelCount++);
        }
    }
}

void __cdecl FX_MarkEntDetachModel(FxMarksSystem *marksSystem, unsigned int entnum, int oldModelIndex)
{
    unsigned __int16 handle; // [esp+0h] [ebp-Ch]
    FxMark *mark; // [esp+4h] [ebp-8h]
    int markModelIndex; // [esp+8h] [ebp-4h]

    if ( entnum >= 0x600
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    1360,
                    0,
                    "entnum doesn't index ARRAY_COUNT( marksSystem->entFirstMarkHandles )\n\t%i not in [0, %i)",
                    entnum,
                    1536) )
    {
        __debugbreak();
    }
    handle = marksSystem->entFirstMarkHandles[entnum];
    while ( handle != 0xFFFF )
    {
        mark = FX_MarkFromHandle(marksSystem, handle);
        handle = mark->nextMark;
        if ( (mark->context.modelTypeAndSurf & 0xC0) == 0xC0 )
        {
            markModelIndex = mark->context.modelTypeAndSurf & 0x3F;
            if ( markModelIndex == oldModelIndex )
            {
                if ( mark->frameCountDrawn == -1
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                                1374,
                                0,
                                "%s",
                                "mark->frameCountDrawn != FX_MARK_FREE") )
                {
                    __debugbreak();
                }
                FX_FreeMark(marksSystem, mark);
                if ( mark->frameCountDrawn != -1
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                                1376,
                                0,
                                "%s",
                                "mark->frameCountDrawn == FX_MARK_FREE") )
                {
                    __debugbreak();
                }
            }
            else if ( markModelIndex > oldModelIndex )
            {
                --mark->context.modelTypeAndSurf;
            }
        }
    }
}

void __cdecl FX_BeginGeneratingMarkVertsForEntModels(unsigned int localClientNum, unsigned int *indexCount)
{
    if ( (!fx_marks->current.enabled || !fx_marks_ents->current.enabled)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    2153,
                    0,
                    "%s",
                    "fx_marks->current.enabled && fx_marks_ents->current.enabled") )
    {
        __debugbreak();
    }
    R_BeginMarkMeshVerts();
    if (_InterlockedIncrement(&g_markThread[localClientNum]) != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    2158,
                    0,
                    "%s",
                    "Sys_InterlockedIncrement( &g_markThread[localClientNum] ) == 1") )
    {
        __debugbreak();
    }
    FX_GetMarksSystem(localClientNum)->hasCarryIndex = 0;
    *indexCount = 0;
}

void __cdecl FX_GenerateMarkVertsForEntXModel(
                int localClientNum,
                unsigned int entId,
                unsigned int *indexCount,
                unsigned __int16 lightHandle,
                unsigned __int8 reflectionProbeIndex,
                const GfxScaledPlacement *placement,
                const GfxLight *visibleLights,
                int visibleLightCount)
{
    FxMarksSystem *marksSystem; // [esp+A0h] [ebp-10h]
    FxSystemContainer *camera; // [esp+A4h] [ebp-Ch]
    unsigned __int16 entMarkListHead; // [esp+ACh] [ebp-4h]

    marksSystem = FX_GetMarksSystem(localClientNum);
    if ( entId >= 0x600
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    2176,
                    0,
                    "entId doesn't index ARRAY_COUNT( marksSystem->entFirstMarkHandles )\n\t%i not in [0, %i)",
                    entId,
                    1536) )
    {
        __debugbreak();
    }
    entMarkListHead = marksSystem->entFirstMarkHandles[entId];
    if ( entMarkListHead != 0xFFFF )
    {
        camera = FX_GetSystem(localClientNum);
        FX_GenerateMarkVertsForList_EntXModel(
            marksSystem,
            entMarkListHead,
            &camera->system.camera,
            indexCount,
            lightHandle,
            reflectionProbeIndex,
            placement,
            visibleLights,
            visibleLightCount);
    }
}

char __cdecl FX_GenerateMarkVertsForList_EntXModel(
                FxMarksSystem *marksSystem,
                unsigned __int16 head,
                const FxCamera *camera,
                unsigned int *indexCount,
                unsigned __int16 lightHandleOverride,
                unsigned __int8 reflectionProbeIndexOverride,
                const GfxScaledPlacement *placement,
                const GfxLight *visibleLights,
                int visibleLightCount)
{
    int frameTime; // [esp+9Ch] [ebp-64h]
    FxMark *mark; // [esp+A0h] [ebp-60h]
    FxActiveMarkSurf drawSurf; // [esp+A4h] [ebp-5Ch] BYREF
    unsigned __int16 markHandle; // [esp+C8h] [ebp-38h]
    unsigned __int16 baseVertex; // [esp+CCh] [ebp-34h] BYREF
    float transformMatrix[4][3]; // [esp+D0h] [ebp-30h] BYREF
    int savedregs; // [esp+100h] [ebp+0h] BYREF

    frameTime = marksSystem->frameTime;
    FX_GenerateMarkVertsForMark_MatrixFromPlacement(
        &placement->base,
        vec3_origin,
        transformMatrix);
    for ( markHandle = head; markHandle != 0xFFFF; markHandle = mark->nextMark )
    {
        mark = FX_MarkFromHandle(marksSystem, markHandle);
        mark->ageMsec += frameTime;
        if ( !FX_MarkIsAlphaFadedOut(mark) && (mark->context.modelTypeAndSurf & 0xC0) == 0xC0 )
        {
            if ( !FX_GenerateMarkVertsForMark_Begin(marksSystem, mark, indexCount, &baseVertex, &drawSurf) )
                return 0;
            FX_GenerateMarkVertsForMark_SetLightHandle(&drawSurf, lightHandleOverride);
            FX_GenerateMarkVertsForMark_SetReflectionProbeIndex(&drawSurf, reflectionProbeIndexOverride);
            FX_GenerateMarkVertsForMark_FinishAnimated(
                marksSystem,
                mark,
                baseVertex,
                &drawSurf,
                transformMatrix,
                visibleLights,
                visibleLightCount);
        }
    }
    return 1;
}

char __cdecl FX_GenerateMarkVertsForMark_Begin(
                FxMarksSystem *marksSystem,
                FxMark *mark,
                unsigned int *indexCount,
                unsigned __int16 *outBaseVertex,
                FxActiveMarkSurf *outDrawSurf)
{
    unsigned int newIndexCount; // [esp+0h] [ebp-10h]
    unsigned int reserveIndexCount; // [esp+4h] [ebp-Ch]
    unsigned __int16 *indices; // [esp+8h] [ebp-8h]
    r_double_index_t *doubleIndices; // [esp+Ch] [ebp-4h] BYREF

    newIndexCount = *indexCount + 3 * mark->triCount;
    reserveIndexCount = ((newIndexCount + 1) & 0xFFFFFFFE) - ((*indexCount + 1) & 0xFFFFFFFE);
    if ( R_ReserveMarkMeshVerts(mark->pointCount, outBaseVertex)
        && R_ReserveMarkMeshIndices(reserveIndexCount, &doubleIndices) )
    {
        indices = (unsigned __int16 *)doubleIndices - (*indexCount & 1);
        *indexCount = newIndexCount;
        if ( mark->frameCountDrawn == -1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                        1775,
                        0,
                        "%s",
                        "mark->frameCountDrawn != FX_MARK_FREE") )
        {
            __debugbreak();
        }
        mark->frameCountDrawn = marksSystem->frameCount;
        FX_DrawMarkTris(marksSystem, mark, *outBaseVertex, indices, outDrawSurf);
        return 1;
    }
    else
    {
        FX_FreeMark(marksSystem, mark);
        return 0;
    }
}

void __cdecl FX_DrawMarkTris(
                FxMarksSystem *marksSystem,
                const FxMark *mark,
                unsigned __int16 baseVertex,
                unsigned __int16 *indices,
                FxActiveMarkSurf *outSurf)
{
    unsigned int groupHandle; // [esp+18h] [ebp-10h]
    int triCount; // [esp+1Ch] [ebp-Ch]
    FxTriGroupPool *group; // [esp+20h] [ebp-8h]
    int triIndex; // [esp+24h] [ebp-4h]

    if ( !mark && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp", 1714, 0, "%s", "mark") )
        __debugbreak();
    groupHandle = mark->tris;
    triCount = mark->triCount;
    outSurf->material = mark->material;
    outSurf->context.lmapIndex = -1;
    outSurf->context.primaryLightIndex = 0;
    outSurf->context.reflectionProbeIndex = -1;
    outSurf->context.modelTypeAndSurf = mark->context.modelTypeAndSurf;
    outSurf->context.modelIndex = mark->context.modelIndex;
    outSurf->indices = indices;
    outSurf->indexCount = 0;
    outSurf->markHitNormal[0] = mark->hitNormal[0];
    outSurf->markHitNormal[1] = mark->hitNormal[1];
    outSurf->markHitNormal[2] = mark->hitNormal[2];
    outSurf->visLightsMask = 0;
    do
    {
        group = FX_TriGroupFromHandle(marksSystem, groupHandle);
        groupHandle = group->triGroup.next;
        if ( triCount < group->triGroup.triCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                        1734,
                        0,
                        "triCount >= group->triCount\n\t%i, %i",
                        triCount,
                        group->triGroup.triCount) )
        {
            __debugbreak();
        }
        if ( !group->triGroup.triCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                        1735,
                        0,
                        "%s\n\t(group->triCount) = %i",
                        "(group->triCount > 0)",
                        group->triGroup.triCount) )
        {
            __debugbreak();
        }
        triCount -= group->triGroup.triCount;
        triIndex = 0;
        do
            FX_EmitMarkTri(
                marksSystem,
                (const unsigned __int16 *)group + 3 * triIndex++,
                &group->triGroup.context,
                baseVertex,
                outSurf);
        while ( triIndex != group->triGroup.triCount );
    }
    while ( triCount );
    if ( groupHandle != 0xFFFF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    1745,
                    0,
                    "%s\n\t(groupHandle) = %i",
                    "(groupHandle == 0xffff)",
                    groupHandle) )
    {
        __debugbreak();
    }
    if ( !outSurf->indexCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    1747,
                    0,
                    "%s",
                    "outSurf->indexCount") )
    {
        __debugbreak();
    }
}

void __cdecl FX_EmitMarkTri(
                FxMarksSystem *marksSystem,
                const unsigned __int16 *indices,
                const GfxMarkContext *markContext,
                unsigned __int16 baseVertex,
                FxActiveMarkSurf *outSurf)
{
    r_double_index_t *pIndex; // [esp+8h] [ebp-4h]
    r_double_index_t *pIndexa; // [esp+8h] [ebp-4h]

    if ( !FX_MarkContextsEqual(&outSurf->context, markContext) )
    {
        if ( outSurf->indexCount )
        {
            FX_AddMarkTrisToScene(outSurf);
            outSurf->indices += outSurf->indexCount;
            outSurf->indexCount = 0;
        }
        if ( outSurf->context.modelIndex != markContext->modelIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                        1657,
                        0,
                        "%s",
                        "outSurf->context.modelIndex == markContext->modelIndex") )
        {
            __debugbreak();
        }
        if ( (outSurf->context.modelTypeAndSurf & 0xC0) != (markContext->modelTypeAndSurf & 0xC0)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                        1658,
                        0,
                        "%s",
                        "(outSurf->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == (markContext->modelTypeAndSurf & MARK_MODEL_TYPE_MASK)") )
        {
            __debugbreak();
        }
        outSurf->context = *markContext;
    }
    if ( marksSystem->hasCarryIndex )
    {
        pIndexa = (r_double_index_t *)&outSurf->indices[outSurf->indexCount - 1];
        if ( ((unsigned __int8)pIndexa & 3) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                        1684,
                        0,
                        "%s",
                        "!((uint)pIndex & 3)") )
        {
            __debugbreak();
        }
        pIndexa->value[0] = marksSystem->carryIndex;
        pIndexa->value[1] = *indices + baseVertex;
        pIndexa[1].value[0] = indices[1] + baseVertex;
        pIndexa[1].value[1] = indices[2] + baseVertex;
        marksSystem->hasCarryIndex = 0;
    }
    else
    {
        pIndex = (r_double_index_t *)&outSurf->indices[outSurf->indexCount];
        if ( ((unsigned __int8)pIndex & 3) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                        1670,
                        0,
                        "%s",
                        "!((uint)pIndex & 3)") )
        {
            __debugbreak();
        }
        pIndex->value[0] = *indices + baseVertex;
        pIndex->value[1] = indices[1] + baseVertex;
        marksSystem->hasCarryIndex = 1;
        marksSystem->carryIndex = indices[2] + baseVertex;
    }
    outSurf->indexCount += 3;
}

void __cdecl FX_AddMarkTrisToScene(const FxActiveMarkSurf *outSurf)
{
    R_AddMarkMeshDrawSurf(
        outSurf->material,
        &outSurf->context,
        outSurf->indices,
        outSurf->indexCount,
        outSurf->markHitNormal,
        outSurf->visLightsMask);
}

void __cdecl FX_GenerateMarkVertsForMark_SetLightHandle(
                FxActiveMarkSurf *drawSurf,
                unsigned __int16 lightHandleOverride)
{
    if ( (drawSurf->context.modelTypeAndSurf & 0xC0) != 0xC0
        && (drawSurf->context.modelTypeAndSurf & 0xC0) != 0x80
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    1785,
                    0,
                    "%s\n\t(drawSurf->context.modelTypeAndSurf) = %i",
                    "((drawSurf->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == MARK_MODEL_TYPE_ENT_MODEL || (drawSurf->contex"
                    "t.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == MARK_MODEL_TYPE_ENT_BRUSH)",
                    drawSurf->context.modelTypeAndSurf) )
    {
        __debugbreak();
    }
    if ( !lightHandleOverride
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    1786,
                    0,
                    "%s",
                    "lightHandleOverride != GFX_ML_HANDLE_NONE") )
    {
        __debugbreak();
    }
    drawSurf->context.modelIndex = lightHandleOverride;
}

void __cdecl FX_GenerateMarkVertsForMark_SetReflectionProbeIndex(
                FxActiveMarkSurf *drawSurf,
                unsigned __int8 reflectionProbeIndexOverride)
{
    if ( (drawSurf->context.modelTypeAndSurf & 0xC0) != 0xC0
        && (drawSurf->context.modelTypeAndSurf & 0xC0) != 0x80
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    1793,
                    0,
                    "%s\n\t(drawSurf->context.modelTypeAndSurf) = %i",
                    "((drawSurf->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == MARK_MODEL_TYPE_ENT_MODEL || (drawSurf->contex"
                    "t.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == MARK_MODEL_TYPE_ENT_BRUSH)",
                    drawSurf->context.modelTypeAndSurf) )
    {
        __debugbreak();
    }
    if ( reflectionProbeIndexOverride == 255
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    1794,
                    0,
                    "%s",
                    "reflectionProbeIndexOverride != REFLECTION_PROBE_INVALID") )
    {
        __debugbreak();
    }
    drawSurf->context.reflectionProbeIndex = reflectionProbeIndexOverride;
}

void __cdecl FX_ExpandMarkVerts_Transform_GfxWorldVertex_(
    FxMarksSystem *marksSystem,
    const FxMark *mark,
    unsigned __int16 baseVertex,
    const float (*matrixTransform)[3])
{
    int v4; // [esp+8h] [ebp-B4h]
    PackedUnitVec v5; // [esp+Ch] [ebp-B0h]
    PackedUnitVec v6; // [esp+28h] [ebp-94h]
    const float *lmapCoord; // [esp+44h] [ebp-78h]
    GfxWorldVertex *castOutVert; // [esp+54h] [ebp-68h]
    unsigned int groupHandle; // [esp+58h] [ebp-64h]
    float delta[3]; // [esp+5Ch] [ebp-60h] BYREF
    float transformedNormal[3]; // [esp+68h] [ebp-54h] BYREF
    float texCoordScale; // [esp+74h] [ebp-48h]
    float binormal[3]; // [esp+78h] [ebp-44h] BYREF
    const FxMarkPoint *markPoint; // [esp+84h] [ebp-38h]
    GfxWorldVertex *verts; // [esp+88h] [ebp-34h]
    float transformedDelta[3]; // [esp+8Ch] [ebp-30h] BYREF
    float texCoord[2]; // [esp+98h] [ebp-24h]
    int pointCount; // [esp+A0h] [ebp-1Ch]
    int loopCount; // [esp+A4h] [ebp-18h]
    float transformedTexCoordAxis[3]; // [esp+A8h] [ebp-14h] BYREF
    const FxPointGroup *group; // [esp+B4h] [ebp-8h]
    GfxWorldVertex *outVert; // [esp+B8h] [ebp-4h]

    if (!mark && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp", 1571, 0, "%s", "mark"))
        __debugbreak();
    verts = R_GetMarkMeshVerts(baseVertex);
    if (mark->radius < 0.1
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
            1575,
            0,
            "%s\n\t(mark->radius) = %g",
            "(mark->radius >= 0.1f)",
            mark->radius))
    {
        __debugbreak();
    }
    texCoordScale = 0.5 / mark->radius;
    groupHandle = mark->points;
    pointCount = mark->pointCount;
    outVert = verts;
    do
    {
        if (groupHandle == 0xFFFF
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                1583,
                0,
                "%s",
                "groupHandle != FX_HANDLE_NONE"))
        {
            __debugbreak();
        }
        group = (const FxPointGroup *)FX_PointGroupFromHandle(marksSystem, groupHandle);
        groupHandle = group->next;
        if (pointCount > 2)
            v4 = 2;
        else
            v4 = pointCount;
        loopCount = v4;
        if (v4 <= 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                1588,
                0,
                "%s\n\t(loopCount) = %i",
                "(loopCount > 0)",
                loopCount))
        {
            __debugbreak();
        }
        pointCount -= loopCount;
        markPoint = (const FxMarkPoint *)group;
        do
        {
            castOutVert = outVert;
            delta[0] = markPoint->xyz[0] - mark->origin[0];
            delta[1] = markPoint->xyz[1] - mark->origin[1];
            delta[2] = markPoint->xyz[2] - mark->origin[2];
            MatrixTransformVector(delta, matrixTransform, transformedDelta);
            MatrixTransformVector(mark->texCoordAxis, matrixTransform, transformedTexCoordAxis);
            MatrixTransformVector(markPoint->normal, matrixTransform, transformedNormal);
            Vec3Cross(transformedTexCoordAxis, transformedNormal, binormal);
            MatrixTransformVector43(markPoint->xyz, matrixTransform, castOutVert->xyz);
            castOutVert->binormalSign = -1.0f;
            castOutVert->color.packed = *(_DWORD *)mark->nativeColor;
            if (mark->alphaFade.ageLimitMsec > 0)
                castOutVert->color.array[3] = FX_MarkGetFadedAlpha(mark);
            texCoord[0] = (float)((float)((float)((float)(transformedDelta[0] * transformedTexCoordAxis[0])
                + (float)(transformedDelta[1] * transformedTexCoordAxis[1]))
                + (float)(transformedDelta[2] * transformedTexCoordAxis[2]))
                * texCoordScale)
                + 0.5;
            texCoord[1] = (float)((float)((float)((float)(transformedDelta[0] * binormal[0])
                + (float)(transformedDelta[1] * binormal[1]))
                + (float)(transformedDelta[2] * binormal[2]))
                * texCoordScale)
                + 0.5;
            lmapCoord = markPoint->lmapCoord;
            castOutVert->texCoord[0] = texCoord[0];
            castOutVert->texCoord[1] = texCoord[1];
            *(double *)castOutVert->lmapCoord = *(double *)lmapCoord;
            v6.array[0] = (int)(float)((float)(transformedNormal[0] * 127.0) + 127.5);
            v6.array[1] = (int)(float)((float)(transformedNormal[1] * 127.0) + 127.5);
            v6.array[2] = (int)(float)((float)(transformedNormal[2] * 127.0) + 127.5);
            v6.array[3] = 63;
            castOutVert->normal = v6;
            v5.array[0] = (int)(float)((float)(transformedTexCoordAxis[0] * 127.0) + 127.5);
            v5.array[1] = (int)(float)((float)(transformedTexCoordAxis[1] * 127.0) + 127.5);
            v5.array[2] = (int)(float)((float)(transformedTexCoordAxis[2] * 127.0) + 127.5);
            v5.array[3] = 63;
            castOutVert->tangent = v5;
            ++markPoint;
            ++outVert;
            --loopCount;
        } while (loopCount);
    } while (pointCount);
    if (groupHandle != 0xFFFF
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
            1618,
            0,
            "%s\n\t(groupHandle) = %i",
            "(groupHandle == 0xffff)",
            groupHandle))
    {
        __debugbreak();
    }
}

void __cdecl setTexCoordAndLMap_GfxPackedVertex_(GfxPackedVertex *outVert, const float *texCoord)
{
    __int16 v2; // [esp+0h] [ebp-40h]
    __int16 v3; // [esp+4h] [ebp-3Ch]
    int v4; // [esp+Ch] [ebp-34h]
    int v5; // [esp+18h] [ebp-28h]
    int v6; // [esp+20h] [ebp-20h]

    if ((int)((2 * *(_DWORD *)texCoord) ^ 0x80000000) >> 14 < 0x3FFF)
        v6 = (int)((2 * *(_DWORD *)texCoord) ^ 0x80000000) >> 14;
    else
        v6 = 0x3FFF;
    if (v6 > -16384)
        v3 = v6;
    else
        v3 = -16384;
    v5 = *((_DWORD *)texCoord + 1);
    if ((int)((2 * v5) ^ 0x80000000) >> 14 < 0x3FFF)
        v4 = (int)((2 * v5) ^ 0x80000000) >> 14;
    else
        v4 = 0x3FFF;
    if (v4 > -16384)
        v2 = v4;
    else
        v2 = -16384;
    outVert->texCoord.packed = (v2 & 0x3FFF | (v5 >> 16) & 0xC000)
        + ((v3 & 0x3FFF | (*(int *)texCoord >> 16) & 0xC000) << 16);
}

void __cdecl FX_ExpandMarkVerts_Transform_GfxPackedVertex_(
    FxMarksSystem *marksSystem,
    const FxMark *mark,
    unsigned __int16 baseVertex,
    const float (*matrixTransform)[3])
{
    int v4; // [esp+8h] [ebp-E4h]
    PackedUnitVec v5; // [esp+Ch] [ebp-E0h]
    PackedUnitVec v6; // [esp+28h] [ebp-C4h]
    GfxPackedVertex *castOutVert; // [esp+84h] [ebp-68h]
    unsigned int groupHandle; // [esp+88h] [ebp-64h]
    float delta[3]; // [esp+8Ch] [ebp-60h] BYREF
    float transformedNormal[3]; // [esp+98h] [ebp-54h] BYREF
    float texCoordScale; // [esp+A4h] [ebp-48h]
    float binormal[3]; // [esp+A8h] [ebp-44h] BYREF
    const FxMarkPoint *markPoint; // [esp+B4h] [ebp-38h]
    GfxWorldVertex *verts; // [esp+B8h] [ebp-34h]
    float transformedDelta[3]; // [esp+BCh] [ebp-30h] BYREF
    float texCoord[2]; // [esp+C8h] [ebp-24h] BYREF
    int pointCount; // [esp+D0h] [ebp-1Ch]
    int loopCount; // [esp+D4h] [ebp-18h]
    float transformedTexCoordAxis[3]; // [esp+D8h] [ebp-14h] BYREF
    const FxPointGroup *group; // [esp+E4h] [ebp-8h]
    GfxWorldVertex *outVert; // [esp+E8h] [ebp-4h]

    if (!mark && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp", 1571, 0, "%s", "mark"))
        __debugbreak();
    verts = R_GetMarkMeshVerts(baseVertex);
    if (mark->radius < 0.1
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
            1575,
            0,
            "%s\n\t(mark->radius) = %g",
            "(mark->radius >= 0.1f)",
            mark->radius))
    {
        __debugbreak();
    }
    texCoordScale = 0.5 / mark->radius;
    groupHandle = mark->points;
    pointCount = mark->pointCount;
    outVert = verts;
    do
    {
        if (groupHandle == 0xFFFF
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                1583,
                0,
                "%s",
                "groupHandle != FX_HANDLE_NONE"))
        {
            __debugbreak();
        }
        group = (const FxPointGroup *)FX_PointGroupFromHandle(marksSystem, groupHandle);
        groupHandle = group->next;
        if (pointCount > 2)
            v4 = 2;
        else
            v4 = pointCount;
        loopCount = v4;
        if (v4 <= 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                1588,
                0,
                "%s\n\t(loopCount) = %i",
                "(loopCount > 0)",
                loopCount))
        {
            __debugbreak();
        }
        pointCount -= loopCount;
        markPoint = (const FxMarkPoint *)group;
        do
        {
            castOutVert = (GfxPackedVertex *)outVert;
            delta[0] = markPoint->xyz[0] - mark->origin[0];
            delta[1] = markPoint->xyz[1] - mark->origin[1];
            delta[2] = markPoint->xyz[2] - mark->origin[2];
            MatrixTransformVector(delta, matrixTransform, transformedDelta);
            MatrixTransformVector(mark->texCoordAxis, matrixTransform, transformedTexCoordAxis);
            MatrixTransformVector(markPoint->normal, matrixTransform, transformedNormal);
            Vec3Cross(transformedTexCoordAxis, transformedNormal, binormal);
            MatrixTransformVector43(markPoint->xyz, matrixTransform, castOutVert->xyz);
            castOutVert->binormalSign = -1.0f;
            castOutVert->color.packed = *(_DWORD *)mark->nativeColor;
            if (mark->alphaFade.ageLimitMsec > 0)
                castOutVert->color.array[3] = FX_MarkGetFadedAlpha(mark);
            texCoord[0] = (float)((float)((float)((float)(transformedDelta[0] * transformedTexCoordAxis[0])
                + (float)(transformedDelta[1] * transformedTexCoordAxis[1]))
                + (float)(transformedDelta[2] * transformedTexCoordAxis[2]))
                * texCoordScale)
                + 0.5;
            texCoord[1] = (float)((float)((float)((float)(transformedDelta[0] * binormal[0])
                + (float)(transformedDelta[1] * binormal[1]))
                + (float)(transformedDelta[2] * binormal[2]))
                * texCoordScale)
                + 0.5;
            setTexCoordAndLMap_GfxPackedVertex_(castOutVert, texCoord);
            v6.array[0] = (int)(float)((float)(transformedNormal[0] * 127.0) + 127.5);
            v6.array[1] = (int)(float)((float)(transformedNormal[1] * 127.0) + 127.5);
            v6.array[2] = (int)(float)((float)(transformedNormal[2] * 127.0) + 127.5);
            v6.array[3] = 63;
            castOutVert->normal = v6;
            v5.array[0] = (int)(float)((float)(transformedTexCoordAxis[0] * 127.0) + 127.5);
            v5.array[1] = (int)(float)((float)(transformedTexCoordAxis[1] * 127.0) + 127.5);
            v5.array[2] = (int)(float)((float)(transformedTexCoordAxis[2] * 127.0) + 127.5);
            v5.array[3] = 63;
            castOutVert->tangent = v5;
            ++markPoint;
            ++outVert;
            --loopCount;
        } while (loopCount);
    } while (pointCount);
    if (groupHandle != 0xFFFF
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
            1618,
            0,
            "%s\n\t(groupHandle) = %i",
            "(groupHandle == 0xffff)",
            groupHandle))
    {
        __debugbreak();
    }
}

void __cdecl FX_GenerateMarkVertsForMark_FinishAnimated(
                FxMarksSystem *marksSystem,
                FxMark *mark,
                unsigned __int16 baseVertex,
                FxActiveMarkSurf *drawSurf,
                const float (*transform)[3],
                const GfxLight *visibleLights,
                int visibleLightCount)
{
    const GfxLight *light; // [esp+B4h] [ebp-20h]
    float radius; // [esp+B8h] [ebp-1Ch]
    int visLightIndex; // [esp+BCh] [ebp-18h]
    unsigned int visLightsMask; // [esp+C0h] [ebp-14h]
    unsigned int type; // [esp+C4h] [ebp-10h]
    float transformedOrigin[3]; // [esp+C8h] [ebp-Ch] BYREF

    type = drawSurf->context.modelTypeAndSurf & 0xC0;
    if ( type != 128
        && type != 192
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    1840,
                    0,
                    "%s\n\t(type) = %i",
                    "(type == MARK_MODEL_TYPE_ENT_BRUSH || type == MARK_MODEL_TYPE_ENT_MODEL)",
                    type) )
    {
        __debugbreak();
    }
    if ( !transform
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp", 1841, 0, "%s", "transform") )
    {
        __debugbreak();
    }
    MatrixTransformVector43(mark->origin, transform, transformedOrigin);
    visLightsMask = 0;
    for ( visLightIndex = 0; visLightIndex < visibleLightCount; ++visLightIndex )
    {
        light = &visibleLights[visLightIndex];
        radius = light->radius + mark->radius;
        if ( radius * radius >= Vec3DistanceSq(light->origin, transformedOrigin) )
        {
            if ( light->type != 3
                && light->type != 2
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                            1854,
                            0,
                            "%s",
                            "light->type == GFX_LIGHT_TYPE_OMNI || light->type == GFX_LIGHT_TYPE_SPOT") )
            {
                __debugbreak();
            }
            visLightsMask |= (light->type != 3) + 1;
        }
    }
    drawSurf->visLightsMask = visLightsMask;
    FX_AddMarkTrisToScene(drawSurf);
    if ( type == 128 )
        FX_ExpandMarkVerts_Transform_GfxWorldVertex_(marksSystem, mark, baseVertex, transform);
    else
        FX_ExpandMarkVerts_Transform_GfxPackedVertex_(marksSystem, mark, baseVertex, transform);
}

void    FX_GenerateMarkVertsForMark_MatrixFromPlacement(
                const GfxPlacement *placement,
                const float *viewOffset,
                float (*outTransform)[3])
{
    DObjAnimMat animMat;
    DObjSkelMat skelMat;

    animMat.quat[0] = placement->quat[0];
    animMat.quat[1] = placement->quat[1];
    animMat.quat[2] = placement->quat[2];
    animMat.quat[3] = placement->quat[3];
    animMat.trans[0] = placement->origin[0];
    animMat.trans[1] = placement->origin[1];
    animMat.trans[2] = placement->origin[2];
    animMat.transWeight = 2.0f;
    ConvertQuatToSkelMat(&animMat, &skelMat);
    DObjSkelMatToMatrix43(&skelMat, outTransform);
}

void __cdecl ConvertQuatToSkelMat(const DObjAnimMat *mat, DObjSkelMat *skelMat)
{
    float transWeight; // [esp+8h] [ebp-48h]
    float yy; // [esp+20h] [ebp-30h]
    float xy; // [esp+24h] [ebp-2Ch]
    float zz; // [esp+28h] [ebp-28h]
    float zw; // [esp+2Ch] [ebp-24h]
    float scaledQuat; // [esp+30h] [ebp-20h]
    float scaledQuat_4; // [esp+34h] [ebp-1Ch]
    float scaledQuat_8; // [esp+38h] [ebp-18h]
    float yw; // [esp+3Ch] [ebp-14h]
    float xz; // [esp+40h] [ebp-10h]
    float yz; // [esp+44h] [ebp-Ch]
    float xx; // [esp+48h] [ebp-8h]
    float xw; // [esp+4Ch] [ebp-4h]

    if ( ((LODWORD(mat->quat[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(mat->quat[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(mat->quat[2]) & 0x7F800000) == 0x7F800000
         || (LODWORD(mat->quat[3]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h",
                    1540,
                    0,
                    "%s",
                    "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])") )
    {
        __debugbreak();
    }
    if ( (LODWORD(mat->transWeight) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h",
                    1541,
                    0,
                    "%s",
                    "!IS_NAN(mat->transWeight)") )
    {
        __debugbreak();
    }
    transWeight = mat->transWeight;
    scaledQuat = transWeight * mat->quat[0];
    scaledQuat_4 = transWeight * mat->quat[1];
    scaledQuat_8 = transWeight * mat->quat[2];
    xx = scaledQuat * mat->quat[0];
    xy = scaledQuat * mat->quat[1];
    xz = scaledQuat * mat->quat[2];
    xw = scaledQuat * mat->quat[3];
    yy = scaledQuat_4 * mat->quat[1];
    yz = scaledQuat_4 * mat->quat[2];
    yw = scaledQuat_4 * mat->quat[3];
    zz = scaledQuat_8 * mat->quat[2];
    zw = scaledQuat_8 * mat->quat[3];
    skelMat->axis[0][0] = 1.0 - (float)(yy + zz);
    skelMat->axis[0][1] = xy + zw;
    skelMat->axis[0][2] = xz - yw;
    skelMat->axis[0][3] = 0.0f;
    skelMat->axis[1][0] = xy - zw;
    skelMat->axis[1][1] = 1.0 - (float)(xx + zz);
    skelMat->axis[1][2] = yz + xw;
    skelMat->axis[1][3] = 0.0f;
    skelMat->axis[2][0] = xz + yw;
    skelMat->axis[2][1] = yz - xw;
    skelMat->axis[2][2] = 1.0 - (float)(xx + yy);
    skelMat->axis[2][3] = 0.0f;
    skelMat->origin[0] = mat->trans[0];
    skelMat->origin[1] = mat->trans[1];
    skelMat->origin[2] = mat->trans[2];
    skelMat->origin[3] = 1.0f;
}

void __cdecl DObjSkelMatToMatrix43(const DObjSkelMat *inSkelMat, float (*outMatrix)[3])
{
    float *v2; // eax
    float v3; // [esp+8h] [ebp-1Ch]
    float v4; // [esp+Ch] [ebp-18h]
    float v5; // [esp+18h] [ebp-Ch]
    float v6; // [esp+1Ch] [ebp-8h]
    int dim; // [esp+20h] [ebp-4h]

    for ( dim = 0; dim != 3; ++dim )
    {
        v5 = inSkelMat->axis[dim][1];
        v6 = inSkelMat->axis[dim][2];
        (*outMatrix)[3 * dim] = inSkelMat->axis[dim][0];
        v2 = &(*outMatrix)[3 * dim];
        v2[1] = v5;
        v2[2] = v6;
    }
    v3 = inSkelMat->origin[1];
    v4 = inSkelMat->origin[2];
    (*outMatrix)[9] = inSkelMat->origin[0];
    (*outMatrix)[10] = v3;
    (*outMatrix)[11] = v4;
}

void __cdecl FX_GenerateMarkVertsForEntDObj(
                int localClientNum,
                unsigned int entId,
                unsigned int *indexCount,
                unsigned __int16 lightHandle,
                unsigned __int8 reflectionProbeIndex,
                DObj *dobj,
                const cpose_t *pose,
                const GfxLight *visibleLights,
                int visibleLightCount)
{
    FxMarksSystem *marksSystem; // [esp+170h] [ebp-28h]
    FxSystem *camera; // [esp+174h] [ebp-24h]
    unsigned int hidePartBits[5]; // [esp+178h] [ebp-20h] BYREF
    FxSystem *system; // [esp+18Ch] [ebp-Ch]
    const DObjAnimMat *boneMtxList; // [esp+190h] [ebp-8h] BYREF
    unsigned __int16 entMarkListHead; // [esp+194h] [ebp-4h]

    if ( !dobj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp", 2204, 0, "%s", "dobj") )
        __debugbreak();
    if ( !pose && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp", 2205, 0, "%s", "pose") )
        __debugbreak();
    marksSystem = FX_GetMarksSystem(localClientNum);
    if ( entId >= 0x600
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    2208,
                    0,
                    "entId doesn't index ARRAY_COUNT( marksSystem->entFirstMarkHandles )\n\t%i not in [0, %i)",
                    entId,
                    1536) )
    {
        __debugbreak();
    }
    entMarkListHead = marksSystem->entFirstMarkHandles[entId];
    if ( entMarkListHead != 0xFFFF )
    {
        system = (FxSystem *)FX_GetSystem(localClientNum);
        camera = system;
        R_MarkUtil_GetDObjAnimMatAndHideParts(dobj, pose, &boneMtxList, hidePartBits);
        FX_GenerateMarkVertsForList_EntDObj(
            marksSystem,
            entMarkListHead,
            &camera->camera,
            indexCount,
            lightHandle,
            reflectionProbeIndex,
            dobj,
            boneMtxList,
            hidePartBits,
            visibleLights,
            visibleLightCount);
    }
}

char __cdecl FX_GenerateMarkVertsForList_EntDObj(
                FxMarksSystem *marksSystem,
                unsigned __int16 head,
                const FxCamera *camera,
                unsigned int *indexCount,
                unsigned __int16 lightHandleOverride,
                unsigned __int8 reflectionProbeIndexOverride,
                const DObj *dobj,
                const DObjAnimMat *boneMtxList,
                unsigned int *hidePartBits,
                const GfxLight *visibleLights,
                int visibleLightCount)
{
    int dObjModelIndexForMark; // [esp+158h] [ebp-68h]
    int frameTime; // [esp+15Ch] [ebp-64h]
    FxMark *mark; // [esp+160h] [ebp-60h]
    FxActiveMarkSurf drawSurf; // [esp+164h] [ebp-5Ch] BYREF
    unsigned __int16 markHandle; // [esp+188h] [ebp-38h]
    unsigned __int16 baseVertex; // [esp+18Ch] [ebp-34h] BYREF
    float transformMatrix[4][3]; // [esp+190h] [ebp-30h] BYREF
    int savedregs; // [esp+1C0h] [ebp+0h] BYREF

    frameTime = marksSystem->frameTime;
    for ( markHandle = head; markHandle != 0xFFFF; markHandle = mark->nextMark )
    {
        mark = FX_MarkFromHandle(marksSystem, markHandle);
        mark->ageMsec += frameTime;
        if ( !FX_MarkIsAlphaFadedOut(mark) && (mark->context.modelTypeAndSurf & 0xC0) == 0xC0 )
        {
            dObjModelIndexForMark = mark->context.modelTypeAndSurf & 0x3F;
            if ( dObjModelIndexForMark < DObjGetNumModels(dobj) )
            {
                if ( !FX_GenerateMarkVertsForMark_Begin(marksSystem, mark, indexCount, &baseVertex, &drawSurf) )
                    return 0;
                FX_GenerateMarkVertsForMark_SetLightHandle(&drawSurf, lightHandleOverride);
                FX_GenerateMarkVertsForMark_SetReflectionProbeIndex(&drawSurf, reflectionProbeIndexOverride);
                FX_GenerateMarkVertsForMark_MatrixFromAnim(
                    mark,
                    dobj,
                    boneMtxList,
                    camera->origin,
                    transformMatrix);
                FX_GenerateMarkVertsForMark_FinishAnimated(
                    marksSystem,
                    mark,
                    baseVertex,
                    &drawSurf,
                    transformMatrix,
                    visibleLights,
                    visibleLightCount);
            }
        }
    }
    return 1;
}

void    FX_GenerateMarkVertsForMark_MatrixFromAnim(
                const FxMark *mark,
                const DObj *dobj,
                const DObjAnimMat *boneMtxList,
                const float *viewOffset,
                float (*outTransform)[3])
{
    DObjSkelMat bonePoseSkelMat;
    DObjSkelMat invBasePoseSkelMat;
    float bonePoseMatrix[4][3];
    float invBasePoseMatrix[4][3];

    if ( !dobj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp", 1916, 0, "%s", "dobj") )
        __debugbreak();
    if ( !boneMtxList
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp", 1917, 0, "%s", "boneMtxList") )
    {
        __debugbreak();
    }
    const int dObjModelIndexIter = mark->context.lmapIndex;
    const int dObjModelIndexForMark = mark->context.modelTypeAndSurf & 0x3F;
    if ( dObjModelIndexForMark >= DObjGetNumModels(dobj)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    1922,
                    0,
                    "%s\n\t(dObjModelIndexForMark) = %i",
                    "(dObjModelIndexForMark < DObjGetNumModels( dobj ))",
                    dObjModelIndexForMark) )
    {
        __debugbreak();
    }

    int baseBoneIndex = 0;
    for ( int modelIndex = 0; modelIndex < dObjModelIndexForMark; ++modelIndex )
    {
        const XModel *model = DObjGetModel(dobj, modelIndex);
        baseBoneIndex += XModelNumBones(model);
    }

    ConvertQuatToSkelMat(&boneMtxList[baseBoneIndex + dObjModelIndexIter], &bonePoseSkelMat);
    DObjSkelMatToMatrix43(&bonePoseSkelMat, bonePoseMatrix);

    const XModel *model = DObjGetModel(dobj, dObjModelIndexForMark);
    const DObjAnimMat *basePose = XModelGetBasePose(model);
    ConvertQuatToInverseSkelMat(&basePose[dObjModelIndexIter], &invBasePoseSkelMat);
    DObjSkelMatToMatrix43(&invBasePoseSkelMat, invBasePoseMatrix);

    MatrixMultiply43(invBasePoseMatrix, bonePoseMatrix, outTransform);
}

void __cdecl ConvertQuatToInverseSkelMat(const DObjAnimMat *mat, DObjSkelMat *skelMat)
{
    float transWeight; // [esp+0h] [ebp-48h]
    float yy; // [esp+18h] [ebp-30h]
    float xy; // [esp+1Ch] [ebp-2Ch]
    float zz; // [esp+20h] [ebp-28h]
    float zw; // [esp+24h] [ebp-24h]
    float scaledQuat; // [esp+28h] [ebp-20h]
    float scaledQuat_4; // [esp+2Ch] [ebp-1Ch]
    float scaledQuat_8; // [esp+30h] [ebp-18h]
    float yw; // [esp+34h] [ebp-14h]
    float xz; // [esp+38h] [ebp-10h]
    float yz; // [esp+3Ch] [ebp-Ch]
    float xx; // [esp+40h] [ebp-8h]
    float xw; // [esp+44h] [ebp-4h]

    if ( ((LODWORD(mat->quat[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(mat->quat[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(mat->quat[2]) & 0x7F800000) == 0x7F800000
         || (LODWORD(mat->quat[3]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h",
                    1649,
                    0,
                    "%s",
                    "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])") )
    {
        __debugbreak();
    }
    if ( (LODWORD(mat->transWeight) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h",
                    1650,
                    0,
                    "%s",
                    "!IS_NAN(mat->transWeight)") )
    {
        __debugbreak();
    }
    transWeight = mat->transWeight;
    scaledQuat = transWeight * mat->quat[0];
    scaledQuat_4 = transWeight * mat->quat[1];
    scaledQuat_8 = transWeight * mat->quat[2];
    xx = scaledQuat * mat->quat[0];
    xy = scaledQuat * mat->quat[1];
    xz = scaledQuat * mat->quat[2];
    xw = scaledQuat * mat->quat[3];
    yy = scaledQuat_4 * mat->quat[1];
    yz = scaledQuat_4 * mat->quat[2];
    yw = scaledQuat_4 * mat->quat[3];
    zz = scaledQuat_8 * mat->quat[2];
    zw = scaledQuat_8 * mat->quat[3];
    skelMat->axis[0][0] = 1.0 - (float)(yy + zz);
    skelMat->axis[0][1] = xy - zw;
    skelMat->axis[0][2] = xz + yw;
    skelMat->axis[0][3] = 0.0f;
    skelMat->axis[1][0] = xy + zw;
    skelMat->axis[1][1] = 1.0 - (float)(xx + zz);
    skelMat->axis[1][2] = yz - xw;
    skelMat->axis[1][3] = 0.0f;
    skelMat->axis[2][0] = xz - yw;
    skelMat->axis[2][1] = yz + xw;
    skelMat->axis[2][2] = 1.0 - (float)(xx + yy);
    skelMat->axis[2][3] = 0.0f;
    (skelMat->origin[0]) = -((float)((float)(mat->trans[0] * skelMat->axis[0][0]) + (float)(mat->trans[1] * skelMat->axis[1][0])) + (float)(mat->trans[2] * skelMat->axis[2][0]));
    (skelMat->origin[1]) = -((float)((float)(mat->trans[0] * skelMat->axis[0][1]) + (float)(mat->trans[1] * skelMat->axis[1][1])) + (float)(mat->trans[2] * skelMat->axis[2][1]));
    (skelMat->origin[2]) = -( (float)((float)(mat->trans[0] * skelMat->axis[0][2]) + (float)(mat->trans[1] * skelMat->axis[1][2])) + (float)(mat->trans[2] * skelMat->axis[2][2]));
    skelMat->origin[3] = 1.0f;
}

void __cdecl FX_GenerateMarkVertsForEntBrush(
                int localClientNum,
                unsigned int entId,
                unsigned int *indexCount,
                unsigned __int8 reflectionProbeIndex,
                const GfxPlacement *placement,
                const GfxLight *visibleLights,
                int visibleLightCount)
{
    FxMarksSystem *marksSystem; // [esp+F8h] [ebp-10h]
    FxSystemContainer *camera; // [esp+FCh] [ebp-Ch]
    unsigned __int16 entMarkListHead; // [esp+104h] [ebp-4h]

    if ( !placement
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp", 2235, 0, "%s", "placement") )
    {
        __debugbreak();
    }
    marksSystem = FX_GetMarksSystem(localClientNum);
    if ( entId >= 0x600
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    2238,
                    0,
                    "entId doesn't index ARRAY_COUNT( marksSystem->entFirstMarkHandles )\n\t%i not in [0, %i)",
                    entId,
                    1536) )
    {
        __debugbreak();
    }
    entMarkListHead = marksSystem->entFirstMarkHandles[entId];
    if ( entMarkListHead != 0xFFFF )
    {
        camera = FX_GetSystem(localClientNum);
        FX_GenerateMarkVertsForList_EntBrush(
            marksSystem,
            entMarkListHead,
            &camera->system.camera,
            indexCount,
            placement,
            reflectionProbeIndex,
            visibleLights,
            visibleLightCount);
    }
}

char __cdecl FX_GenerateMarkVertsForList_EntBrush(
                FxMarksSystem *marksSystem,
                unsigned __int16 head,
                const FxCamera *camera,
                unsigned int *indexCount,
                const GfxPlacement *placement,
                unsigned __int8 reflectionProbeIndex,
                const GfxLight *visibleLights,
                int visibleLightCount)
{
    int frameTime; // [esp+C0h] [ebp-64h]
    FxMark *mark; // [esp+C4h] [ebp-60h]
    FxActiveMarkSurf drawSurf; // [esp+C8h] [ebp-5Ch] BYREF
    unsigned __int16 markHandle; // [esp+ECh] [ebp-38h]
    unsigned __int16 baseVertex; // [esp+F0h] [ebp-34h] BYREF
    float transformMatrix[4][3]; // [esp+F4h] [ebp-30h] BYREF
    int savedregs; // [esp+124h] [ebp+0h] BYREF

    frameTime = marksSystem->frameTime;
    FX_GenerateMarkVertsForMark_MatrixFromPlacement(placement, vec3_origin, transformMatrix);
    for ( markHandle = head; markHandle != 0xFFFF; markHandle = mark->nextMark )
    {
        mark = FX_MarkFromHandle(marksSystem, markHandle);
        mark->ageMsec += frameTime;
        if ( !FX_MarkIsAlphaFadedOut(mark) && (mark->context.modelTypeAndSurf & 0xC0) == 0x80 )
        {
            if ( !FX_GenerateMarkVertsForMark_Begin(marksSystem, mark, indexCount, &baseVertex, &drawSurf) )
                return 0;
            FX_GenerateMarkVertsForMark_SetReflectionProbeIndex(&drawSurf, reflectionProbeIndex);
            FX_GenerateMarkVertsForMark_FinishAnimated(
                marksSystem,
                mark,
                baseVertex,
                &drawSurf,
                transformMatrix,
                visibleLights,
                visibleLightCount);
            if ( (mark->markFlags & 1) != 0 )
                ++marksSystem->numVisibleEntBrushSeethru;
        }
    }
    return 1;
}

void __cdecl FX_EndGeneratingMarkVertsForEntModels(unsigned int localClientNum)
{
    FxMarksSystem *marksSystem; // [esp+8h] [ebp-4h]

    marksSystem = FX_GetMarksSystem(localClientNum);
    FX_FinishGeneratingMarkVerts(marksSystem);
    if (_InterlockedDecrement(&g_markThread[localClientNum])
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    2264,
                    0,
                    "%s",
                    "Sys_InterlockedDecrement( &g_markThread[localClientNum] ) == 0") )
    {
        __debugbreak();
    }
    R_EndMarkMeshVerts();
}

void __cdecl FX_FinishGeneratingMarkVerts(FxMarksSystem *marksSystem)
{
    r_double_index_t *doubleIndices; // [esp+4h] [ebp-4h] BYREF

    if ( marksSystem->hasCarryIndex )
    {
        if ( R_ReserveMarkMeshIndices(0, &doubleIndices) )
            *--doubleIndices = (r_double_index_t)marksSystem->carryIndex;
        else
            Com_PrintWarning(21, "FX Marks WARNING: R_ReserveMarkMeshIndices returned false\n");
    }
}

void __cdecl FX_GenerateMarkVertsForStaticModels(
                int localClientNum,
                int smodelCount,
                const unsigned __int8 *smodelVisLods,
                const GfxLight *visibleLights,
                int visibleLightCount)
{
    unsigned __int16 v5; // ax
    FxMarksSystem *marksSystem; // [esp+40h] [ebp-18h]
    FxMark *mark; // [esp+44h] [ebp-14h]
    FxSystem *camera; // [esp+48h] [ebp-10h]
    unsigned int indexCount; // [esp+4Ch] [ebp-Ch] BYREF
    FxSystem *system; // [esp+50h] [ebp-8h]
    FxMark *markEnd; // [esp+54h] [ebp-4h]

    if ( (!fx_marks->current.enabled || !fx_marks_smodels->current.enabled)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    2283,
                    0,
                    "%s",
                    "fx_marks->current.enabled && fx_marks_smodels->current.enabled") )
    {
        __debugbreak();
    }
    R_BeginMarkMeshVerts();
    if (_InterlockedIncrement(&g_markThread[localClientNum]) != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    2288,
                    0,
                    "%s",
                    "Sys_InterlockedIncrement( &g_markThread[localClientNum] ) == 1") )
    {
        __debugbreak();
    }
    marksSystem = FX_GetMarksSystem(localClientNum);
    system = (FxSystem *)FX_GetSystem(localClientNum);
    camera = system;
    marksSystem->hasCarryIndex = 0;
    indexCount = 0;
    markEnd = (FxMark *)marksSystem->triGroups;
    for ( mark = marksSystem->marks; mark != markEnd; ++mark )
    {
        if ( mark->prevMark == 0xFFFF && mark->frameCountDrawn != -1 && (mark->context.modelTypeAndSurf & 0xC0) == 0x40 )
        {
            if ( smodelVisLods[mark->context.modelIndex] )
            {
                v5 = FX_MarkToHandle(marksSystem, mark);
                FX_GenerateMarkVertsForList_WorldXModel(
                    marksSystem,
                    v5,
                    &camera->camera,
                    &indexCount,
                    visibleLights,
                    visibleLightCount);
            }
        }
    }
    FX_FinishGeneratingMarkVerts(marksSystem);
    if (_InterlockedDecrement(&g_markThread[localClientNum])
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    2320,
                    0,
                    "%s",
                    "Sys_InterlockedDecrement( &g_markThread[localClientNum] ) == 0") )
    {
        __debugbreak();
    }
    R_EndMarkMeshVerts();
}

char __cdecl FX_GenerateMarkVertsForList_WorldXModel(
                FxMarksSystem *marksSystem,
                unsigned __int16 head,
                const FxCamera *camera,
                unsigned int *indexCount,
                const GfxLight *visibleLights,
                int visibleLightCount)
{
    int frameTime; // [esp+34h] [ebp-34h]
    FxMark *mark; // [esp+38h] [ebp-30h]
    FxActiveMarkSurf drawSurf; // [esp+3Ch] [ebp-2Ch] BYREF
    unsigned __int16 markHandle; // [esp+60h] [ebp-8h]
    unsigned __int16 baseVertex; // [esp+64h] [ebp-4h] BYREF

    frameTime = marksSystem->frameTime;
    for ( markHandle = head; markHandle != 0xFFFF; markHandle = mark->nextMark )
    {
        mark = FX_MarkFromHandle(marksSystem, markHandle);
        mark->ageMsec += frameTime;
        if ( !FX_MarkIsAlphaFadedOut(mark) && !FX_CullSphere(camera, camera->frustumPlaneCount, mark->origin, mark->radius) )
        {
            if ( !FX_GenerateMarkVertsForMark_Begin(marksSystem, mark, indexCount, &baseVertex, &drawSurf) )
                return 0;
            FX_GenerateMarkVertsForMark_FinishNonAnimated(
                marksSystem,
                mark,
                baseVertex,
                &drawSurf,
                visibleLights,
                visibleLightCount);
        }
    }
    return 1;
}

void __cdecl FX_ExpandMarkVerts_NoTransform_GfxPackedVertex_(
    FxMarksSystem *marksSystem,
    const FxMark *mark,
    unsigned __int16 baseVertex)
{
    int v3; // [esp+10h] [ebp-D0h]
    PackedUnitVec v4; // [esp+1Ch] [ebp-C4h]
    PackedUnitVec v5; // [esp+3Ch] [ebp-A4h]
    GfxPackedVertex *castOutVert; // [esp+9Ch] [ebp-44h]
    unsigned int groupHandle; // [esp+A0h] [ebp-40h]
    float delta; // [esp+A4h] [ebp-3Ch]
    float delta_4; // [esp+A8h] [ebp-38h]
    float delta_8; // [esp+ACh] [ebp-34h]
    float texCoordScale; // [esp+B0h] [ebp-30h]
    float binormal[3]; // [esp+B4h] [ebp-2Ch] BYREF
    const FxMarkPoint *markPoint; // [esp+C0h] [ebp-20h]
    GfxWorldVertex *verts; // [esp+C4h] [ebp-1Ch]
    float texCoord[2]; // [esp+C8h] [ebp-18h] BYREF
    int pointCount; // [esp+D0h] [ebp-10h]
    int loopCount; // [esp+D4h] [ebp-Ch]
    const FxPointGroup *group; // [esp+D8h] [ebp-8h]
    GfxWorldVertex *outVert; // [esp+DCh] [ebp-4h]

    if (!mark && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp", 1484, 0, "%s", "mark"))
        __debugbreak();
    _mm_prefetch((const char *)mark, 1);
    verts = R_GetMarkMeshVerts(baseVertex);
    pointCount = mark->pointCount;
    if (mark->radius < 0.1
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
            1500,
            0,
            "%s\n\t(mark->radius) = %g",
            "(mark->radius >= 0.1f)",
            mark->radius))
    {
        __debugbreak();
    }
    texCoordScale = 0.5 / mark->radius;
    groupHandle = mark->points;
    outVert = verts;
    do
    {
        if (groupHandle == 0xFFFF
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                1506,
                0,
                "%s",
                "groupHandle != FX_HANDLE_NONE"))
        {
            __debugbreak();
        }
        group = (const FxPointGroup *)FX_PointGroupFromHandle(marksSystem, groupHandle);
        groupHandle = group->next;
        if (pointCount > 2)
            v3 = 2;
        else
            v3 = pointCount;
        loopCount = v3;
        if (v3 <= 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                1511,
                0,
                "%s\n\t(loopCount) = %i",
                "(loopCount > 0)",
                loopCount))
        {
            __debugbreak();
        }
        pointCount -= loopCount;
        markPoint = (const FxMarkPoint *)group;
        do
        {
            _mm_prefetch((const char *)&markPoint[5], 1);
            _mm_prefetch((const char *)&outVert[2], 1);
            castOutVert = (GfxPackedVertex *)outVert;
            delta = markPoint->xyz[0] - mark->origin[0];
            delta_4 = markPoint->xyz[1] - mark->origin[1];
            delta_8 = markPoint->xyz[2] - mark->origin[2];
            Vec3Cross(mark->texCoordAxis, markPoint->normal, binormal);
            castOutVert->xyz[0] = markPoint->xyz[0];
            castOutVert->xyz[1] = markPoint->xyz[1];
            castOutVert->xyz[2] = markPoint->xyz[2];
            castOutVert->binormalSign = -1.0f;
            castOutVert->color.packed = *(_DWORD *)mark->nativeColor;
            if (mark->alphaFade.ageLimitMsec > 0)
                castOutVert->color.array[3] = FX_MarkGetFadedAlpha(mark);
            texCoord[0] = (float)((float)((float)((float)(delta * mark->texCoordAxis[0])
                + (float)(delta_4 * mark->texCoordAxis[1]))
                + (float)(delta_8 * mark->texCoordAxis[2]))
                * texCoordScale)
                + 0.5;
            texCoord[1] = (float)((float)((float)((float)(delta * binormal[0]) + (float)(delta_4 * binormal[1]))
                + (float)(delta_8 * binormal[2]))
                * texCoordScale)
                + 0.5;
            setTexCoordAndLMap_GfxPackedVertex_(castOutVert, texCoord);
            v5.array[0] = (int)(float)((float)(markPoint->normal[0] * 127.0) + 127.5);
            v5.array[1] = (int)(float)((float)(markPoint->normal[1] * 127.0) + 127.5);
            v5.array[2] = (int)(float)((float)(markPoint->normal[2] * 127.0) + 127.5);
            v5.array[3] = 63;
            castOutVert->normal = v5;
            v4.array[0] = (int)(float)((float)(mark->texCoordAxis[0] * 127.0) + 127.5);
            v4.array[1] = (int)(float)((float)(mark->texCoordAxis[1] * 127.0) + 127.5);
            v4.array[2] = (int)(float)((float)(mark->texCoordAxis[2] * 127.0) + 127.5);
            v4.array[3] = 63;
            castOutVert->tangent = v4;
            ++markPoint;
            ++outVert;
            --loopCount;
        } while (loopCount);
    } while (pointCount);
    if (groupHandle != 0xFFFF
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
            1543,
            0,
            "%s\n\t(groupHandle) = %i",
            "(groupHandle == 0xffff)",
            groupHandle))
    {
        __debugbreak();
    }
}

void __cdecl FX_ExpandMarkVerts_NoTransform_GfxWorldVertex_(
    FxMarksSystem *marksSystem,
    const FxMark *mark,
    unsigned __int16 baseVertex)
{
    int v3; // [esp+10h] [ebp-A0h]
    PackedUnitVec v4; // [esp+1Ch] [ebp-94h]
    PackedUnitVec v5; // [esp+3Ch] [ebp-74h]
    const float *lmapCoord; // [esp+58h] [ebp-58h]
    GfxWorldVertex *castOutVert; // [esp+6Ch] [ebp-44h]
    unsigned int groupHandle; // [esp+70h] [ebp-40h]
    float delta; // [esp+74h] [ebp-3Ch]
    float delta_4; // [esp+78h] [ebp-38h]
    float delta_8; // [esp+7Ch] [ebp-34h]
    float texCoordScale; // [esp+80h] [ebp-30h]
    float binormal[3]; // [esp+84h] [ebp-2Ch] BYREF
    const FxMarkPoint *markPoint; // [esp+90h] [ebp-20h]
    GfxWorldVertex *verts; // [esp+94h] [ebp-1Ch]
    float texCoord[2]; // [esp+98h] [ebp-18h]
    int pointCount; // [esp+A0h] [ebp-10h]
    int loopCount; // [esp+A4h] [ebp-Ch]
    const FxPointGroup *group; // [esp+A8h] [ebp-8h]
    GfxWorldVertex *outVert; // [esp+ACh] [ebp-4h]

    if (!mark && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp", 1484, 0, "%s", "mark"))
        __debugbreak();
    _mm_prefetch((const char *)mark, 1);
    verts = R_GetMarkMeshVerts(baseVertex);
    pointCount = mark->pointCount;
    if (mark->radius < 0.1
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
            1500,
            0,
            "%s\n\t(mark->radius) = %g",
            "(mark->radius >= 0.1f)",
            mark->radius))
    {
        __debugbreak();
    }
    texCoordScale = 0.5 / mark->radius;
    groupHandle = mark->points;
    outVert = verts;
    do
    {
        if (groupHandle == 0xFFFF
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                1506,
                0,
                "%s",
                "groupHandle != FX_HANDLE_NONE"))
        {
            __debugbreak();
        }
        group = (const FxPointGroup *)FX_PointGroupFromHandle(marksSystem, groupHandle);
        groupHandle = group->next;
        if (pointCount > 2)
            v3 = 2;
        else
            v3 = pointCount;
        loopCount = v3;
        if (v3 <= 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                1511,
                0,
                "%s\n\t(loopCount) = %i",
                "(loopCount > 0)",
                loopCount))
        {
            __debugbreak();
        }
        pointCount -= loopCount;
        markPoint = (const FxMarkPoint *)group;
        do
        {
            _mm_prefetch((const char *)&markPoint[5], 1);
            _mm_prefetch((const char *)&outVert[2], 1);
            castOutVert = outVert;
            delta = markPoint->xyz[0] - mark->origin[0];
            delta_4 = markPoint->xyz[1] - mark->origin[1];
            delta_8 = markPoint->xyz[2] - mark->origin[2];
            Vec3Cross(mark->texCoordAxis, markPoint->normal, binormal);
            castOutVert->xyz[0] = markPoint->xyz[0];
            castOutVert->xyz[1] = markPoint->xyz[1];
            castOutVert->xyz[2] = markPoint->xyz[2];
            castOutVert->binormalSign = -1.0f;
            castOutVert->color.packed = *(_DWORD *)mark->nativeColor;
            if (mark->alphaFade.ageLimitMsec > 0)
                castOutVert->color.array[3] = FX_MarkGetFadedAlpha(mark);
            texCoord[0] = (float)((float)((float)((float)(delta * mark->texCoordAxis[0])
                + (float)(delta_4 * mark->texCoordAxis[1]))
                + (float)(delta_8 * mark->texCoordAxis[2]))
                * texCoordScale)
                + 0.5;
            texCoord[1] = (float)((float)((float)((float)(delta * binormal[0]) + (float)(delta_4 * binormal[1]))
                + (float)(delta_8 * binormal[2]))
                * texCoordScale)
                + 0.5;
            lmapCoord = markPoint->lmapCoord;
            castOutVert->texCoord[0] = texCoord[0];
            castOutVert->texCoord[1] = texCoord[1];
            *(double *)castOutVert->lmapCoord = *(double *)lmapCoord;
            v5.array[0] = (int)(float)((float)(markPoint->normal[0] * 127.0) + 127.5);
            v5.array[1] = (int)(float)((float)(markPoint->normal[1] * 127.0) + 127.5);
            v5.array[2] = (int)(float)((float)(markPoint->normal[2] * 127.0) + 127.5);
            v5.array[3] = 63;
            castOutVert->normal = v5;
            v4.array[0] = (int)(float)((float)(mark->texCoordAxis[0] * 127.0) + 127.5);
            v4.array[1] = (int)(float)((float)(mark->texCoordAxis[1] * 127.0) + 127.5);
            v4.array[2] = (int)(float)((float)(mark->texCoordAxis[2] * 127.0) + 127.5);
            v4.array[3] = 63;
            castOutVert->tangent = v4;
            ++markPoint;
            ++outVert;
            --loopCount;
        } while (loopCount);
    } while (pointCount);
    if (groupHandle != 0xFFFF
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
            1543,
            0,
            "%s\n\t(groupHandle) = %i",
            "(groupHandle == 0xffff)",
            groupHandle))
    {
        __debugbreak();
    }
}

void __cdecl FX_GenerateMarkVertsForMark_FinishNonAnimated(
                FxMarksSystem *marksSystem,
                FxMark *mark,
                unsigned __int16 baseVertex,
                FxActiveMarkSurf *drawSurf,
                const GfxLight *visibleLights,
                int visibleLightCount)
{
    const GfxLight *light; // [esp+98h] [ebp-14h]
    float radius; // [esp+9Ch] [ebp-10h]
    int visLightIndex; // [esp+A0h] [ebp-Ch]
    unsigned int visLightsMask; // [esp+A4h] [ebp-8h]
    unsigned int type; // [esp+A8h] [ebp-4h]

    type = drawSurf->context.modelTypeAndSurf & 0xC0;
    if ( (drawSurf->context.modelTypeAndSurf & 0xC0) != 0
        && type != 64
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    1804,
                    0,
                    "%s\n\t(type) = %i",
                    "(type == MARK_MODEL_TYPE_WORLD_BRUSH || type == MARK_MODEL_TYPE_WORLD_MODEL)",
                    type) )
    {
        __debugbreak();
    }
    visLightsMask = 0;
    for ( visLightIndex = 0; visLightIndex < visibleLightCount; ++visLightIndex )
    {
        light = &visibleLights[visLightIndex];
        radius = light->radius + mark->radius;
        if ( radius * radius >= Vec3DistanceSq(light->origin, mark->origin) )
        {
            if ( light->type != 3
                && light->type != 2
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                            1814,
                            0,
                            "%s",
                            "light->type == GFX_LIGHT_TYPE_OMNI || light->type == GFX_LIGHT_TYPE_SPOT") )
            {
                __debugbreak();
            }
            visLightsMask |= (light->type != 3) + 1;
        }
    }
    drawSurf->visLightsMask = visLightsMask;
    FX_AddMarkTrisToScene(drawSurf);
    if ( type )
        FX_ExpandMarkVerts_NoTransform_GfxPackedVertex_(marksSystem, mark, baseVertex);
    else
        FX_ExpandMarkVerts_NoTransform_GfxWorldVertex_(marksSystem, mark, baseVertex);
}

void __cdecl FX_GenerateMarkVertsForWorld(int localClientNum, const GfxLight *visibleLights, int visibleLightCount)
{
    FxSystemContainer *v3; // eax
    FxMarksSystem *marksSystem; // [esp+4Ch] [ebp-10h]
    unsigned int indexCount; // [esp+54h] [ebp-8h] BYREF
    FxSystem *system; // [esp+58h] [ebp-4h]

    if ( fx_marks && fx_marks->current.enabled )
    {
        R_BeginMarkMeshVerts();
        if (_InterlockedIncrement(&g_markThread[localClientNum]) != 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                        2343,
                        0,
                        "%s",
                        "Sys_InterlockedIncrement( &g_markThread[localClientNum] ) == 1") )
        {
            __debugbreak();
        }
        marksSystem = FX_GetMarksSystem(localClientNum);
        v3 = FX_GetSystem(localClientNum);
        system = &v3->system;
        marksSystem->hasCarryIndex = 0;
        indexCount = 0;
        FX_GenerateMarkVertsForList_WorldBrush(
            marksSystem,
            marksSystem->firstActiveWorldMarkHandle,
            &v3->system.camera,
            &indexCount,
            visibleLights,
            visibleLightCount);
        FX_FinishGeneratingMarkVerts(marksSystem);
        if (_InterlockedDecrement(&g_markThread[localClientNum]))
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                            2353,
                            0,
                            "%s",
                            "Sys_InterlockedDecrement( &g_markThread[localClientNum] ) == 0") )
                __debugbreak();
        }
        R_EndMarkMeshVerts();
    }
}

char __cdecl FX_GenerateMarkVertsForList_WorldBrush(
                FxMarksSystem *marksSystem,
                unsigned __int16 head,
                const FxCamera *camera,
                unsigned int *indexCount,
                const GfxLight *visibleLights,
                int visibleLightCount)
{
    int frameTime; // [esp+34h] [ebp-34h]
    FxMark *mark; // [esp+38h] [ebp-30h]
    FxActiveMarkSurf drawSurf; // [esp+3Ch] [ebp-2Ch] BYREF
    unsigned __int16 markHandle; // [esp+60h] [ebp-8h]
    unsigned __int16 baseVertex; // [esp+64h] [ebp-4h] BYREF

    frameTime = marksSystem->frameTime;
    if ( marksSystem->numVisibleWorldSeethru
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_marks.cpp",
                    2092,
                    0,
                    "%s",
                    "marksSystem->numVisibleWorldSeethru == 0") )
    {
        __debugbreak();
    }
    for ( markHandle = head; markHandle != 0xFFFF; markHandle = mark->nextMark )
    {
        mark = FX_MarkFromHandle(marksSystem, markHandle);
        mark->ageMsec += frameTime;
        if ( !FX_MarkIsAlphaFadedOut(mark) && !FX_CullSphere(camera, camera->frustumPlaneCount, mark->origin, mark->radius) )
        {
            if ( !FX_GenerateMarkVertsForMark_Begin(marksSystem, mark, indexCount, &baseVertex, &drawSurf) )
                return 0;
            FX_GenerateMarkVertsForMark_FinishNonAnimated(
                marksSystem,
                mark,
                baseVertex,
                &drawSurf,
                visibleLights,
                visibleLightCount);
            if ( (mark->markFlags & 1) != 0 )
                ++marksSystem->numVisibleWorldSeethru;
        }
    }
    return 1;
}

void __cdecl FX_MarkUpdateClientTime(unsigned int localClientNum, int frameTime)
{
    FX_GetMarksSystem(localClientNum)->frameTime = frameTime;
}

int __cdecl FX_GetFrameTotalSeeThruDecalCount(unsigned int localClientNum)
{
    FxMarksSystem *MarksSystem; // edx

    MarksSystem = FX_GetMarksSystem(localClientNum);
    return MarksSystem->numVisibleEntBrushSeethru + MarksSystem->numVisibleWorldSeethru;
}
