#include "r_dpvs.h"

#include "r_reflection_probe.h"
#include "r_init.h"
#include "r_model_lighting.h"
#include "r_dvars.h"
#include "r_shader_constant_set.h"
#include "r_drawsurf.h"
#include "r_model_lod.h"
#include "r_skybox.h"
#include "r_warn.h"
#include "r_debug.h"
#include "r_model.h"
#include "r_dpvs_entity.h"
#include "r_dpvs_static.h"
#include "r_bsp.h"

#include <qcommon/threads.h>
#include <EffectsCore/fx_beam.h>
#include <xanim/xmodel_utils.h>
#include <ragdoll/ragdoll_controller.h>
#include <DynEntity/DynEntity_load_obj.h>
#include <physics/rope.h>

#include <client/splitscreen.h>
#include <cgame_mp/cg_ents_mp.h>
#include <universal/com_workercmds.h>
#include <universal/mem_largelocal.h>
#include <universal/com_convexhull.h>
#include "r_primarylights.h"



const float standardFrustumSidePlanes[4][4] =
{
  { -1.0, 0.0, 0.0, 1.0 },
  { 1.0, 0.0, 0.0, 1.0 },
  { 0.0, -1.0, 0.0, 1.0 },
  { 0.0, 1.0, 0.0, 1.0 }
};

const float color_0[4] = { 0.5, 0.5, 0.0, 0.25 };
const float color_1[4] = { 0.0, 1.0, 1.0, 0.25 };


DpvsGlob dpvsGlob;
thread_local unsigned __int8 *g_dynEntVisData[2];
thread_local DpvsView *g_dpvsView;
thread_local int g_viewIndex;
thread_local unsigned __int8 *g_smodelVisData;
thread_local unsigned __int8 *g_surfaceVisData;

void __cdecl R_FrustumClipPlanes(
                const GfxMatrix *viewProjMtx,
                const float (*sidePlanes)[4],
                int sidePlaneCount,
                DpvsPlane *frustumPlanes)
{
    DpvsPlane *v4; // [esp+4h] [ebp-24h]
    int term; // [esp+18h] [ebp-10h]
    float length; // [esp+20h] [ebp-8h]
    int planeIndex; // [esp+24h] [ebp-4h]

    for ( planeIndex = 0; planeIndex < sidePlaneCount; ++planeIndex )
    {
        for ( term = 0; term < 4; ++term )
            frustumPlanes[planeIndex].coeffs[term] = (float)((float)((float)((float)(*sidePlanes)[4 * planeIndex]
                                                                                                                                         * viewProjMtx->m[term][0])
                                                                                                                         + (float)((float)(*sidePlanes)[4 * planeIndex + 1]
                                                                                                                                         * viewProjMtx->m[term][1]))
                                                                                                         + (float)((float)(*sidePlanes)[4 * planeIndex + 2]
                                                                                                                         * viewProjMtx->m[term][2]))
                                                                                         + (float)((float)(*sidePlanes)[4 * planeIndex + 3] * viewProjMtx->m[term][3]);
        length = Vec3Length(frustumPlanes[planeIndex].coeffs);
        if ( length <= 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 508, 0, "%s", "length > 0") )
        {
            __debugbreak();
        }
        v4 = &frustumPlanes[planeIndex];
        v4->coeffs[0] = (float)(1.0 / length) * v4->coeffs[0];
        v4->coeffs[1] = (float)(1.0 / length) * v4->coeffs[1];
        v4->coeffs[2] = (float)(1.0 / length) * v4->coeffs[2];
        v4->coeffs[3] = (float)(1.0 / length) * v4->coeffs[3];
        R_SetDpvsPlaneSides(v4);
    }
}

void __cdecl R_SetDpvsPlaneSides(DpvsPlane *plane)
{
    plane->side[0] = SLODWORD(plane->coeffs[0]) <= 0 ? 0 : 12;
    plane->side[1] = SLODWORD(plane->coeffs[1]) <= 0 ? 4 : 16;
    plane->side[2] = SLODWORD(plane->coeffs[2]) <= 0 ? 8 : 20;
}

int __cdecl R_CellForPoint(const float *origin)
{
    cplane_s *v1; // edx
    mnode_t *node; // [esp+4h] [ebp-1Ch]
    int cellIndex; // [esp+14h] [ebp-Ch]
    int cellCount; // [esp+18h] [ebp-8h]

    if ( !rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 925, 0, "%s", "rgp.world") )
    {
        __debugbreak();
    }
    node = (mnode_t *)rgp.world->dpvsPlanes.nodes;
    cellCount = rgp.world->dpvsPlanes.cellCount + 1;
    while ( 1 )
    {
        cellIndex = node->cellIndex;
        if ( cellIndex - cellCount < 0 )
            break;
        v1 = &rgp.world->dpvsPlanes.planes[cellIndex - cellCount];
        node = (mnode_t *)((char *)node
                                         + 2
                                         * ((float)((float)((float)((float)(*origin * v1->normal[0]) + (float)(origin[1] * v1->normal[1]))
                                                                            + (float)(origin[2] * v1->normal[2]))
                                                            - v1->dist) <= 0.0)
                                         * (node->rightChildOffset - 2)
                                         + 4);
    }
    return cellIndex - 1;
}

unsigned int __cdecl R_CalcReflectionProbeIndex(const float *origin)
{
    unsigned int cellIndex; // [esp+0h] [ebp-8h]
    unsigned int bestProbe; // [esp+4h] [ebp-4h]

    bestProbe = R_FindProbeFromVolume(g_worldDraw, origin);
    if ( bestProbe )
        return bestProbe;
    cellIndex = R_CellForPoint(origin);
    if ( cellIndex == -1 )
        return R_FindNearestReflectionProbe(g_worldDraw, origin);
    if ( cellIndex >= rgp.world->dpvsPlanes.cellCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    1221,
                    0,
                    "cellIndex doesn't index rgp.world->dpvsPlanes.cellCount\n\t%i not in [0, %i)",
                    cellIndex,
                    rgp.world->dpvsPlanes.cellCount) )
    {
        __debugbreak();
    }
    return R_FindNearestReflectionProbeInCell(g_worldDraw, &rgp.world->cells[cellIndex], origin);
}

unsigned int __cdecl R_FindNearestReflectionProbeInCell(
                const GfxWorldDraw *worldDraw,
                const GfxCell *cell,
                const float *origin)
{
    float bestProbeDist; // [esp+10h] [ebp-10h]
    unsigned __int8 bestProbe; // [esp+16h] [ebp-Ah]
    unsigned __int8 probeIndex; // [esp+17h] [ebp-9h]
    float testProbeDist; // [esp+18h] [ebp-8h]
    unsigned int cellProbeIndex; // [esp+1Ch] [ebp-4h]

    if ( !worldDraw
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 1076, 0, "%s", "worldDraw") )
    {
        __debugbreak();
    }
    if ( worldDraw->reflectionProbeCount >= 0xFF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    1077,
                    0,
                    "%s",
                    "worldDraw->reflectionProbeCount < 0xff") )
    {
        __debugbreak();
    }
    bestProbe = 0;
    bestProbeDist = FLT_MAX;
    if ( !cell->reflectionProbeCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    1085,
                    0,
                    "%s",
                    "cell->reflectionProbeCount > 0") )
    {
        __debugbreak();
    }
    for ( cellProbeIndex = 0; cellProbeIndex < cell->reflectionProbeCount; ++cellProbeIndex )
    {
        probeIndex = cell->reflectionProbes[cellProbeIndex];
        if ( probeIndex == 255
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                        1089,
                        0,
                        "%s",
                        "probeIndex != REFLECTION_PROBE_INVALID") )
        {
            __debugbreak();
        }
        if ( !worldDraw->reflectionProbes[probeIndex].probeVolumeCount )
        {
            if ( probeIndex >= worldDraw->reflectionProbeCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                            1095,
                            0,
                            "probeIndex doesn't index worldDraw->reflectionProbeCount\n\t%i not in [0, %i)",
                            probeIndex,
                            worldDraw->reflectionProbeCount) )
            {
                __debugbreak();
            }
            testProbeDist = Vec3DistanceSq(worldDraw->reflectionProbes[probeIndex].origin, origin);
            if ( bestProbeDist > testProbeDist )
            {
                bestProbeDist = testProbeDist;
                bestProbe = probeIndex;
            }
        }
    }
    return bestProbe;
}

unsigned int __cdecl R_FindNearestReflectionProbe(const GfxWorldDraw *worldDraw, const float *origin)
{
    float bestProbeDist; // [esp+10h] [ebp-Ch]
    unsigned __int8 bestProbe; // [esp+16h] [ebp-6h]
    unsigned __int8 probeIndex; // [esp+17h] [ebp-5h]
    float testProbeDist; // [esp+18h] [ebp-4h]

    if ( !worldDraw
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 1119, 0, "%s", "worldDraw") )
    {
        __debugbreak();
    }
    if ( worldDraw->reflectionProbeCount >= 0xFF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    1120,
                    0,
                    "%s",
                    "worldDraw->reflectionProbeCount < 0xff") )
    {
        __debugbreak();
    }
    bestProbe = 0;
    bestProbeDist = FLT_MAX;
    for ( probeIndex = 1; probeIndex < worldDraw->reflectionProbeCount; ++probeIndex )
    {
        if ( !worldDraw->reflectionProbes[probeIndex].probeVolumeCount )
        {
            testProbeDist = Vec3DistanceSq(worldDraw->reflectionProbes[probeIndex].origin, origin);
            if ( bestProbeDist > testProbeDist )
            {
                bestProbeDist = testProbeDist;
                bestProbe = probeIndex;
            }
        }
    }
    return bestProbe;
}

unsigned int __cdecl R_FindProbeFromVolume(const GfxWorldDraw *worldDraw, const float *origin)
{
    unsigned int sideIndex; // [esp+4h] [ebp-1Ch]
    bool originNotInVolume; // [esp+Bh] [ebp-15h]
    GfxReflectionProbeVolumeData *probeVolumeData; // [esp+Ch] [ebp-14h]
    GfxReflectionProbe *ReflectionProbe; // [esp+10h] [ebp-10h]
    unsigned int probeVolumeIndex; // [esp+14h] [ebp-Ch]
    unsigned int bestProbe; // [esp+18h] [ebp-8h]
    unsigned int probeIndex; // [esp+1Ch] [ebp-4h]

    if ( worldDraw->reflectionProbeCount >= 0xFF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    1152,
                    0,
                    "%s",
                    "worldDraw->reflectionProbeCount < 0xff") )
    {
        __debugbreak();
    }
    bestProbe = 0;
    for ( probeIndex = 1; probeIndex < worldDraw->reflectionProbeCount; ++probeIndex )
    {
        ReflectionProbe = &worldDraw->reflectionProbes[probeIndex];
        if ( ReflectionProbe->probeVolumeCount )
        {
            for ( probeVolumeIndex = 0; probeVolumeIndex < ReflectionProbe->probeVolumeCount; ++probeVolumeIndex )
            {
                originNotInVolume = 0;
                for ( sideIndex = 0; sideIndex < 6; ++sideIndex )
                {
                    probeVolumeData = &ReflectionProbe->probeVolumes[probeVolumeIndex];
                    if ( (float)((float)((float)(probeVolumeData->volumePlanes[sideIndex][2] * origin[2])
                                                         + (float)((float)(probeVolumeData->volumePlanes[sideIndex][1] * origin[1])
                                                                         + (float)(probeVolumeData->volumePlanes[sideIndex][0] * *origin)))
                                         + probeVolumeData->volumePlanes[sideIndex][3]) < 0.0 )
                    {
                        originNotInVolume = 1;
                        break;
                    }
                }
                if ( !originNotInVolume )
                {
                    bestProbe = probeIndex;
                    break;
                }
            }
            if ( bestProbe )
                break;
        }
    }
    return bestProbe;
}

void __cdecl R_AddAllSceneEntSurfacesCamera(const GfxViewInfo *viewInfo)
{
    const GfxLight *v1; // [esp+A4h] [ebp-A4h]
    int i; // [esp+A8h] [ebp-A0h]
    float fadeVar; // [esp+ACh] [ebp-9Ch]
    float burnVar; // [esp+B0h] [ebp-98h]
    GfxSceneGlassBrush *glassBrush; // [esp+B8h] [ebp-90h]
    unsigned int glassIndex; // [esp+BCh] [ebp-8Ch]
    volatile unsigned int glassBrushCount; // [esp+C8h] [ebp-80h]
    float brushRadius; // [esp+CCh] [ebp-7Ch]
    const GfxLight *light; // [esp+D0h] [ebp-78h]
    int visLightIndex; // [esp+D4h] [ebp-74h]
    GfxSceneDynBrush *sceneDynBrush; // [esp+DCh] [ebp-6Ch]
    DynEntityPose *dynEntPose; // [esp+E0h] [ebp-68h]
    unsigned int visLightsMask; // [esp+E4h] [ebp-64h]
    unsigned int visLightsMaska; // [esp+E4h] [ebp-64h]
    volatile int lightingInfoUsed; // [esp+E8h] [ebp-60h]
    //int lightingInfoUseda; // [esp+E8h] [ebp-60h]
    bool dobjLightingFinished; // [esp+EFh] [ebp-59h]
    GfxLightingInfo *modelLightingInfo; // [esp+F0h] [ebp-58h]
    GfxDrawSurf *lastDrawSurfs[3]; // [esp+F4h] [ebp-54h] BYREF
    unsigned int sceneEntCount; // [esp+100h] [ebp-48h]
    bool modelLightingFinished; // [esp+107h] [ebp-41h]
    unsigned int reflectionProbeIndex; // [esp+108h] [ebp-40h]
    const DynEntityDef *dynEntDef; // [esp+10Ch] [ebp-3Ch]
    unsigned int sceneEntIndex; // [esp+110h] [ebp-38h]
    const GfxLight *visibleLights; // [esp+114h] [ebp-34h]
    GfxLightingInfo *dobjLightingInfo; // [esp+118h] [ebp-30h]
    GfxSceneBrush *sceneBrush; // [esp+11Ch] [ebp-2Ch]
    GfxDrawSurf *drawSurfs[3]; // [esp+120h] [ebp-28h] BYREF
    unsigned __int16 dynEntId; // [esp+12Ch] [ebp-1Ch]
    unsigned __int8 *sceneEntVisData; // [esp+134h] [ebp-14h]
    const GfxBrushModel *bmodel; // [esp+138h] [ebp-10h]
    int drawSurfCount; // [esp+13Ch] [ebp-Ch]
    int visibleLightCount; // [esp+140h] [ebp-8h]
    GfxLightingInfo *dynentLightingInfo; // [esp+144h] [ebp-4h]

    PROF_SCOPED("R_AddAllSceneEntSurfacesCamera");

    drawSurfs[0] = scene.drawSurfs[2];
    lastDrawSurfs[0] = &scene.drawSurfs[2][scene.maxDrawSurfCount[2]];
    drawSurfs[1] = scene.drawSurfs[5];
    lastDrawSurfs[1] = &scene.drawSurfs[5][scene.maxDrawSurfCount[5]];
    drawSurfs[2] = scene.drawSurfs[12];
    lastDrawSurfs[2] = &scene.drawSurfs[12][scene.maxDrawSurfCount[12]];
    dobjLightingFinished = 0;
    modelLightingFinished = 0;
    dobjLightingInfo = lightingInfo;

    {
        PROF_SCOPED("sceneDObj lighting");
        visibleLightCount = viewInfo->visibleLightCount;
        visibleLights = viewInfo->visibleLights;
        LaunchDobjLightingJobs(viewInfo, scene.sceneDObjCount, scene.sceneDObjVisData[0], lightingInfo);
        lightingInfoUsed = scene.sceneDObjCount;
        if ((unsigned int)(scene.sceneModelCount + scene.sceneDObjCount) > 0x400)
        {
            Sys_AssistAndWaitWorkerCmdInternal(&r_model_lightingWorkerCmd);
            FinishDobjLighting(
                scene.sceneDObjCount,
                scene.sceneDObjVisData[0],
                dobjLightingInfo,
                drawSurfs,
                lastDrawSurfs,
                visibleLights,
                visibleLightCount,
                viewInfo->isMissileCamera);
            dobjLightingFinished = 1;
            lightingInfoUsed = 0;
        }
    }

    modelLightingInfo = &lightingInfo[lightingInfoUsed];

    {
        PROF_SCOPED("sceneModel lighting");
        LaunchModelLightingJobs(viewInfo, scene.sceneModelCount, scene.sceneModelVisData[0], modelLightingInfo);
        lightingInfoUsed = scene.sceneModelCount + lightingInfoUsed;
        if (scene.sceneDynModelCount + lightingInfoUsed > 0x400)
        {
            Sys_AssistAndWaitWorkerCmdInternal(&r_model_lightingWorkerCmd);
            FinishModelLighting(
                scene.sceneModelCount,
                scene.sceneModelVisData[0],
                modelLightingInfo,
                drawSurfs,
                lastDrawSurfs,
                visibleLights,
                visibleLightCount);
            modelLightingFinished = 1;
            lightingInfoUsed = 0;
        }
    }

    dynentLightingInfo = &lightingInfo[lightingInfoUsed];

    {
        PROF_SCOPED("sceneDynModel lighting");

        iassert(scene.sceneDynModelCount < 1024);
        LaunchDynEntLightingJobs(
            viewInfo,
            scene.sceneDynModelCount,
            rgp.world->dpvsDyn.dynEntVisData[0][0],
            dynentLightingInfo);
    }

    {
        PROF_SCOPED("wait r_model_lighting");
        Sys_AssistAndWaitWorkerCmdInternal(&r_model_lightingWorkerCmd);
    }
    
    if ( !dobjLightingFinished )
        FinishDobjLighting(
            scene.sceneDObjCount,
            scene.sceneDObjVisData[0],
            dobjLightingInfo,
            drawSurfs,
            lastDrawSurfs,
            visibleLights,
            visibleLightCount,
            viewInfo->isMissileCamera);
    if ( !modelLightingFinished )
    {
        FinishModelLighting(
            scene.sceneModelCount,
            scene.sceneModelVisData[0],
            modelLightingInfo,
            drawSurfs,
            lastDrawSurfs,
            visibleLights,
            visibleLightCount);
        modelLightingFinished = 1;
    }
    FinishDynEntLighting(
        scene.sceneDynModelCount,
        rgp.world->dpvsDyn.dynEntVisData[0][0],
        dynentLightingInfo,
        drawSurfs,
        lastDrawSurfs,
        visibleLights,
        visibleLightCount);

    {
        PROF_SCOPED("sceneBrush addsurfaces");

        sceneEntCount = scene.sceneBrushCount;
        sceneEntVisData = scene.sceneBrushVisData[0];
        for (sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex)
        {
            if (sceneEntVisData[sceneEntIndex] == 1)
            {
                sceneBrush = &scene.sceneBrush[sceneEntIndex];
                bmodel = sceneBrush->bmodel;
                visLightsMask = 0;
                for (visLightIndex = 0; visLightIndex < visibleLightCount; ++visLightIndex)
                {
                    light = &visibleLights[visLightIndex];
                    if (light->radius * light->radius >= PointToBoxDistSq(
                        light->origin,
                        bmodel->writable.mins,
                        bmodel->writable.maxs))
                    {
                        if (light->type == 3)
                        {
                            visLightsMask |= 1u;
                        }
                        else if (BoxInPlanes(scene.dynamicSpotLightPlanes, 6u, bmodel->writable.mins, bmodel->writable.maxs))
                        {
                            visLightsMask |= 2u;
                        }
                    }
                }
                reflectionProbeIndex = R_CalcReflectionProbeIndex(sceneBrush->placement.origin);
                sceneBrush->reflectionProbeIndex = reflectionProbeIndex;
                if (sceneBrush->reflectionProbeIndex != reflectionProbeIndex
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                        1703,
                        0,
                        "%s\n\t(reflectionProbeIndex) = %i",
                        "(sceneBrush->reflectionProbeIndex == reflectionProbeIndex)",
                        reflectionProbeIndex))
                {
                    __debugbreak();
                }
                R_AddBModelSurfacesCamera(
                    &sceneBrush->info,
                    bmodel,
                    drawSurfs,
                    lastDrawSurfs,
                    reflectionProbeIndex,
                    0.0,
                    0.0,
                    visLightsMask);
                if (sceneBrush->brushConstantSet && r_shader_constant_set_debug_range->current.value > 0.0)
                {
                    brushRadius = Vec3Distance(bmodel->bounds[0], bmodel->bounds[1]) * 0.5;
                    R_ShaderConstantShowDebug(
                        viewInfo->cullViewInfo.viewParms.origin,
                        sceneBrush->placement.origin,
                        brushRadius,
                        sceneBrush->brushConstantSet);
                }
            }
        }
    }
    
    {
        PROF_SCOPED("glassBrush addsurfaces");

        glassBrushCount = scene.glassBrushCount;
        for (glassIndex = 0; glassIndex < glassBrushCount; ++glassIndex)
        {
            if (scene.glassBrushVisData[glassIndex] == 1)
            {
                glassBrush = &scene.glassBrush[glassIndex];
                reflectionProbeIndex = R_CalcReflectionProbeIndex(glassBrush->placement.origin);
                glassBrush->reflectionProbeIndex = reflectionProbeIndex;
                if (glassBrush->reflectionProbeIndex != reflectionProbeIndex
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                        1733,
                        0,
                        "%s\n\t(reflectionProbeIndex) = %i",
                        "(glassBrush->reflectionProbeIndex == reflectionProbeIndex)",
                        reflectionProbeIndex))
                {
                    __debugbreak();
                }
                R_AddBModelSurfacesCamera(
                    &glassBrush->info,
                    glassBrush->bmodel,
                    drawSurfs,
                    lastDrawSurfs,
                    reflectionProbeIndex,
                    0.0,
                    0.0,
                    0);
            }
        }
    }
    
    {
        PROF_SCOPED("sceneDynBrush addsurfaces");

        sceneEntCount = scene.sceneDynBrushCount;
        sceneEntVisData = rgp.world->dpvsDyn.dynEntVisData[1][0];
        for (sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex)
        {
            sceneDynBrush = &rgp.world->sceneDynBrush[sceneEntIndex];
            dynEntId = sceneDynBrush->dynEntId;
            if (sceneEntVisData[dynEntId] == 1)
            {
                dynEntPose = DynEnt_GetClientPose(dynEntId, DYNENT_DRAW_BRUSH);
                dynEntDef = DynEnt_GetEntityDef(dynEntId, DYNENT_DRAW_BRUSH);
                bmodel = R_GetBrushModel(dynEntDef->brushModel);
                visLightsMaska = 0;
                for (i = 0; i < visibleLightCount; ++i)
                {
                    v1 = &visibleLights[i];
                    if (v1->radius * v1->radius >= PointToBoxDistSq(v1->origin, bmodel->writable.mins, bmodel->writable.maxs))
                    {
                        if (v1->type == 3)
                        {
                            visLightsMaska |= 1u;
                        }
                        else if (BoxInPlanes(scene.dynamicSpotLightPlanes, 6u, bmodel->writable.mins, bmodel->writable.maxs))
                        {
                            visLightsMaska |= 2u;
                        }
                    }
                }
                reflectionProbeIndex = R_CalcReflectionProbeIndex(dynEntPose->pose.origin);
                burnVar = DynEntCl_GetBurnTime(dynEntId, DYNENT_DRAW_BRUSH);
                fadeVar = DynEntCl_GetFadeTime(dynEntId, DYNENT_DRAW_BRUSH);
                R_AddBModelSurfacesCamera(
                    (BModelDrawInfo *)sceneDynBrush,
                    bmodel,
                    drawSurfs,
                    lastDrawSurfs,
                    reflectionProbeIndex,
                    burnVar,
                    fadeVar,
                    visLightsMaska);
            }
        }
    }
    
    drawSurfCount = drawSurfs[0] - scene.drawSurfs[2];
    scene.drawSurfCount[2] = drawSurfCount;

    {
        PROF_SCOPED("sort surfs ENT_CAMERA_LIT");
        R_SortDrawSurfs(scene.drawSurfs[2], drawSurfCount);
    }

    drawSurfCount = drawSurfs[1] - scene.drawSurfs[5];
    scene.drawSurfCount[5] = drawSurfCount;

    {
        PROF_SCOPED("sort surfs ENT_CAMERA_DECAL");
        R_SortDrawSurfs(scene.drawSurfs[5], drawSurfCount);
    }
    
    drawSurfCount = drawSurfs[2] - scene.drawSurfs[12];
    scene.drawSurfCount[12] = drawSurfCount;

    {
        PROF_SCOPED("sort surfs ENT_CAMERA_EMISSIVE");
        R_SortDrawSurfs(scene.drawSurfs[12], drawSurfCount);
    }
}

void __cdecl LaunchDobjLightingJobs(
                const GfxViewInfo *viewInfo,
                unsigned int sceneEntCount,
                unsigned __int8 *sceneEntVisData,
                GfxLightingInfo *lightingInfo)
{
    bool useHeroLighting; // [esp+1Bh] [ebp-15h]
    GfxEntity *gfxEnt; // [esp+1Ch] [ebp-14h]
    GfxSceneEntity *sceneEnt; // [esp+20h] [ebp-10h]
    cpose_t *pose; // [esp+24h] [ebp-Ch]
    unsigned int sceneEntIndex; // [esp+2Ch] [ebp-4h]

    for ( sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex )
    {
        if ( sceneEntVisData[sceneEntIndex] == 1 )
        {
            sceneEnt = &scene.sceneDObj[sceneEntIndex];
            gfxEnt = &frontEndDataOut->gfxEnts[sceneEnt->gfxEntIndex];
            if ( sceneEnt->cull.state < 2
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                            1236,
                            0,
                            "%s",
                            "sceneEnt->cull.state >= CULL_STATE_BOUNDED") )
            {
                __debugbreak();
            }
            useHeroLighting = (gfxEnt->renderFxFlags & 0x400000) != 0;
            //pose = (cpose_t *)Ragdoll_HandleBody(sceneEnt->info.pose);
            pose = (cpose_t *)(sceneEnt->info.pose);
            if ( !R_AllocModelLighting_Box(
                            viewInfo,
                            sceneEnt->lightingOrigin,
                            sceneEnt->lightingOriginToleranceSq,
                            sceneEnt->cull.mins,
                            sceneEnt->cull.maxs,
                            useHeroLighting,
                            &pose->lightingHandle,
                            &lightingInfo[sceneEntIndex]) )
                sceneEntVisData[sceneEntIndex] = 0;
        }
    }
}

void __cdecl FinishDobjLighting(
                unsigned int sceneEntCount,
                unsigned __int8 *sceneEntVisData,
                GfxLightingInfo *lightingInfo,
                GfxDrawSurf **drawSurfs,
                GfxDrawSurf **lastDrawSurfs,
                const GfxLight *visibleLights,
                int visibleLightCount,
                bool isMissileCamera)
{
    float objRad; // [esp+0h] [ebp-40h]
    const ShaderConstantSet *v9; // [esp+4h] [ebp-3Ch]
    const GfxLight *light; // [esp+28h] [ebp-18h]
    int visLightIndex; // [esp+2Ch] [ebp-14h]
    unsigned int visLightsMask; // [esp+30h] [ebp-10h]
    GfxSceneEntity *sceneEnt; // [esp+34h] [ebp-Ch]
    unsigned int sceneEntIndex; // [esp+38h] [ebp-8h]

    PROF_SCOPED("FinishDobjLighting");
    for ( sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex )
    {
        if ( sceneEntVisData[sceneEntIndex] == 1 )
        {
            sceneEnt = &scene.sceneDObj[sceneEntIndex];
            visLightsMask = 0;
            for ( visLightIndex = 0; visLightIndex < visibleLightCount; ++visLightIndex )
            {
                light = &visibleLights[visLightIndex];
                if ( light->radius * light->radius >= PointToBoxDistSq(light->origin, sceneEnt->cull.mins, sceneEnt->cull.maxs) )
                {
                    if ( light->type == 3 )
                    {
                        visLightsMask |= 1u;
                    }
                    else if ( BoxInPlanes(scene.dynamicSpotLightPlanes, 6u, sceneEnt->cull.mins, sceneEnt->cull.maxs) )
                    {
                        visLightsMask |= 2u;
                    }
                }
            }
            R_ShowPrimaryLightDebugLine(sceneEnt->lightingOrigin, lightingInfo[sceneEntIndex].primaryLightIndex, 3);
            sceneEnt->reflectionProbeIndex = lightingInfo[sceneEntIndex].reflectionProbeIndex;
            v9 = R_ShaderConstantSet_FromFrontEndDataOut(sceneEnt->entShaderConstantSetIndex);
            objRad = DObjGetRadius(sceneEnt->obj);
            R_ShaderConstantShowDebug(
                frontEndDataOut->viewInfo[frontEndDataOut->viewInfoIndex].cullViewInfo.viewParms.origin,
                sceneEnt->placement.base.origin,
                objRad,
                v9);
            R_AddDObjSurfacesCamera(
                sceneEnt,
                lightingInfo[sceneEntIndex].lightingHandle,
                lightingInfo[sceneEntIndex].primaryLightIndex,
                drawSurfs,
                lastDrawSurfs,
                visLightsMask,
                isMissileCamera);
        }
    }
}

void __cdecl LaunchModelLightingJobs(
                const GfxViewInfo *viewInfo,
                unsigned int sceneEntCount,
                unsigned __int8 *sceneEntVisData,
                GfxLightingInfo *lightingInfo)
{
    unsigned int sceneEntIndex; // [esp+24h] [ebp-4h]

    for ( sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex )
    {
        if ( sceneEntVisData[sceneEntIndex] == 1
            && !R_AllocModelLighting_Sphere(
                        viewInfo,
                        scene.sceneModel[sceneEntIndex].lightingOrigin,
                        scene.sceneModel[sceneEntIndex].lightingOriginToleranceSq,
                        scene.sceneModel[sceneEntIndex].placement.base.origin,
                        scene.sceneModel[sceneEntIndex].radius,
                        0,
                        scene.sceneModel[sceneEntIndex].cachedLightingHandle,
                        &lightingInfo[sceneEntIndex]) )
        {
            sceneEntVisData[sceneEntIndex] = 0;
        }
    }
}

void __cdecl FinishModelLighting(
                unsigned int sceneEntCount,
                unsigned __int8 *sceneEntVisData,
                GfxLightingInfo *lightingInfo,
                GfxDrawSurf **drawSurfs,
                GfxDrawSurf **lastDrawSurfs,
                const GfxLight *visibleLights,
                int visibleLightCount)
{
    const ShaderConstantSet *v7; // eax
    XModelLodRampType LodRampType; // eax
    float val; // [esp+0h] [ebp-5Ch]
    float objRad; // [esp+4h] [ebp-58h]
    bool v11; // [esp+8h] [ebp-54h]
    const GfxLight *light; // [esp+20h] [ebp-3Ch]
    float radius; // [esp+24h] [ebp-38h]
    int visLightIndex; // [esp+28h] [ebp-34h]
    unsigned int visLightsMask; // [esp+30h] [ebp-2Ch]
    GfxSceneModel *sceneModel; // [esp+34h] [ebp-28h]
    float dist; // [esp+38h] [ebp-24h]
    float dista; // [esp+38h] [ebp-24h]
    bool depthHack; // [esp+3Ch] [ebp-20h]
    unsigned int scsIndex; // [esp+48h] [ebp-14h]
    unsigned int sceneEntIndex; // [esp+50h] [ebp-Ch]
    XModel *skyModel; // [esp+58h] [ebp-4h]

    PROF_SCOPED("FinishModelLighting");

    skyModel = R_MapGetSkyboxModel();
    for ( sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex )
    {
        if ( sceneEntVisData[sceneEntIndex] == 1 )
        {
            sceneModel = &scene.sceneModel[sceneEntIndex];
            depthHack = 0;
            if ( sceneModel->gfxEntIndex )
                depthHack = (frontEndDataOut->gfxEnts[scene.sceneModel[sceneEntIndex].gfxEntIndex].renderFxFlags & 2) != 0;
            visLightsMask = 0;
            if ( sceneModel->model != skyModel )
            {
                for ( visLightIndex = 0; visLightIndex < visibleLightCount; ++visLightIndex )
                {
                    light = &visibleLights[visLightIndex];
                    radius = light->radius + sceneModel->radius;
                    if ( radius * radius >= Vec3DistanceSq(light->origin, sceneModel->placement.base.origin) )
                    {
                        if ( light->type == 3 )
                        {
                            visLightsMask |= 1u;
                        }
                        else
                        {
                            if ( light->type != 2
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                                            1367,
                                            0,
                                            "%s",
                                            "light->type == GFX_LIGHT_TYPE_SPOT") )
                            {
                                __debugbreak();
                            }
                            if ( SphereInPlanes(
                                         scene.dynamicSpotLightPlanes,
                                         6u,
                                         sceneModel->placement.base.origin,
                                         sceneModel->radius) )
                            {
                                visLightsMask |= 2u;
                            }
                        }
                    }
                }
            }
            R_ShowPrimaryLightDebugLine(sceneModel->lightingOrigin, lightingInfo[sceneEntIndex].primaryLightIndex, 2);
            sceneModel->reflectionProbeIndex = lightingInfo[sceneEntIndex].reflectionProbeIndex;
            scsIndex = sceneModel->modelShaderConstantSetIndex;
            v7 = R_ShaderConstantSet_FromFrontEndDataOut(scsIndex);
            R_ShaderConstantShowDebug(
                frontEndDataOut->viewInfo[frontEndDataOut->viewInfoIndex].cullViewInfo.viewParms.origin,
                sceneModel->placement.base.origin,
                sceneModel->radius,
                v7);
            R_AddXModelSurfacesCamera(
                &sceneModel->info,
                sceneModel->model,
                sceneModel->placement.base.origin,
                sceneModel->gfxEntIndex,
                lightingInfo[sceneEntIndex].lightingHandle,
                lightingInfo[sceneEntIndex].primaryLightIndex,
                0,
                depthHack,
                drawSurfs,
                lastDrawSurfs,
                lightingInfo[sceneEntIndex].reflectionProbeIndex,
                scsIndex,
                visLightsMask);
            dist = R_GetBaseLodDist(sceneModel->placement.base.origin);
            LodRampType = XModelGetLodRampType(sceneModel->model);
            dista = R_GetAdjustedLodDist(dist, LodRampType);
            v11 = (sceneModel->model->flags & 0x80000) != 0;
            objRad = sceneModel->model->lodInfo[sceneModel->info.lod].dist;
            val = I_fres(sceneModel->placement.scale) * dista;
            ShodLodInfo(sceneModel->placement.base.origin, sceneModel->info.lod, sceneModel->model->numLods, val, objRad, v11);
        }
    }
}

void __cdecl LaunchDynEntLightingJobs(
                const GfxViewInfo *viewInfo,
                unsigned int sceneEntCount,
                unsigned __int8 *sceneEntVisData,
                GfxLightingInfo *lightingInfo)
{
    DynEntityPose *dynEntPose; // [esp+0h] [ebp-1Ch]
    DynEntityClient *dynEntClient; // [esp+8h] [ebp-14h]
    unsigned __int16 dynEntId; // [esp+Ch] [ebp-10h]
    unsigned int sceneEntIndex; // [esp+18h] [ebp-4h]

    for ( sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex )
    {
        dynEntId = rgp.world->sceneDynModel[sceneEntIndex].dynEntId;
        if ( sceneEntVisData[dynEntId] == 1 )
        {
            dynEntPose = DynEnt_GetClientPose(dynEntId, DYNENT_DRAW_MODEL);
            dynEntClient = DynEnt_GetClientEntity(dynEntId, DYNENT_DRAW_MODEL);
            if ( !R_AllocModelLighting_DynEnts(
                            dynEntPose->pose.origin,
                            dynEntId,
                            0,
                            &dynEntClient->lightingHandle,
                            &lightingInfo[sceneEntIndex]) )
                sceneEntVisData[sceneEntIndex] = 0;
        }
    }
}

void __cdecl FinishDynEntLighting(
                unsigned int sceneEntCount,
                unsigned __int8 *sceneEntVisData,
                GfxLightingInfo *lightingInfo,
                GfxDrawSurf **drawSurfs,
                GfxDrawSurf **lastDrawSurfs,
                const GfxLight *visibleLights,
                int visibleLightCount)
{
    XModelLodRampType LodRampType; // eax
    int integer; // [esp+Ch] [ebp-64h]
    const GfxLight *light; // [esp+2Ch] [ebp-44h]
    float radius; // [esp+30h] [ebp-40h]
    int visLightIndex; // [esp+34h] [ebp-3Ch]
    float fadeVar; // [esp+38h] [ebp-38h]
    GfxEntity *gfxEnt; // [esp+3Ch] [ebp-34h]
    DynEntityPose *dynEntPose; // [esp+40h] [ebp-30h]
    unsigned int visLightsMask; // [esp+44h] [ebp-2Ch]
    XModel *model; // [esp+48h] [ebp-28h]
    float dist; // [esp+4Ch] [ebp-24h]
    float dista; // [esp+4Ch] [ebp-24h]
    float burnVar; // [esp+50h] [ebp-20h]
    DynEntityClient *dynEntClient; // [esp+54h] [ebp-1Ch]
    const DynEntityDef *dynEntDef; // [esp+58h] [ebp-18h]
    unsigned __int16 dynEntId; // [esp+5Ch] [ebp-14h]
    unsigned int gfxEntIndex; // [esp+60h] [ebp-10h]
    GfxSceneDynModel *sceneDynModel; // [esp+64h] [ebp-Ch]
    unsigned int sceneEntIndex; // [esp+6Ch] [ebp-4h]

    PROF_SCOPED("FinishDynEntLighting");

    for ( sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex )
    {
        sceneDynModel = &rgp.world->sceneDynModel[sceneEntIndex];
        dynEntId = sceneDynModel->dynEntId;
        if ( sceneEntVisData[dynEntId] == 1 )
        {
            dynEntPose = DynEnt_GetClientPose(dynEntId, DYNENT_DRAW_MODEL);
            burnVar = DynEntCl_GetBurnTime(dynEntId, DYNENT_DRAW_MODEL);
            dynEntDef = DynEnt_GetEntityDef(dynEntId, DYNENT_DRAW_MODEL);
            gfxEntIndex = 0;
            if ( burnVar != 0.0 )
            {
                gfxEntIndex = _InterlockedExchangeAdd(&frontEndDataOut->gfxEntCount, 1u);
                if ( gfxEntIndex < 0x100 )
                {
                    fadeVar = DynEntCl_GetFadeTime(dynEntId, DYNENT_DRAW_MODEL);
                    gfxEnt = &frontEndDataOut->gfxEnts[gfxEntIndex];
                    frontEndDataOut->gfxEnts[gfxEntIndex].materialTime = 0.0f;
                    gfxEnt->renderFxFlags = 0;
                    if ( r_swrk_override_enable->current.enabled )
                        integer = r_swrk_override_characterCharredAmount->current.integer;
                    else
                        integer = LODWORD(burnVar);
                    LODWORD(gfxEnt->destructibleBurnAmount) = integer;
                    gfxEnt->destructibleFadeAmount = fadeVar;
                    gfxEnt->wetness = 0.0f;
                    gfxEnt->textureOverrideIndex = -1;
                }
                else
                {
                    frontEndDataOut->gfxEntCount = 256;
                    R_WarnOncePerFrame(R_WARN_KNOWN_SPECIAL_MODELS, 256);
                    gfxEntIndex = 0;
                }
            }
            visLightsMask = 0;
            for ( visLightIndex = 0; visLightIndex < visibleLightCount; ++visLightIndex )
            {
                light = &visibleLights[visLightIndex];
                radius = light->radius + dynEntPose->radius;
                if ( radius * radius >= Vec3DistanceSq(light->origin, dynEntPose->pose.origin) )
                {
                    if ( light->type == 3 )
                    {
                        visLightsMask |= 1u;
                    }
                    else
                    {
                        if ( light->type != 2
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                                        1486,
                                        0,
                                        "%s",
                                        "light->type == GFX_LIGHT_TYPE_SPOT") )
                        {
                            __debugbreak();
                        }
                        if ( SphereInPlanes(scene.dynamicSpotLightPlanes, 6u, dynEntPose->pose.origin, dynEntPose->radius) )
                            visLightsMask |= 2u;
                    }
                }
            }
            R_ShowPrimaryLightDebugLine(dynEntPose->pose.origin, lightingInfo[sceneEntIndex].primaryLightIndex, 4);
            dynEntClient = DynEnt_GetClientEntity(dynEntId, DYNENT_DRAW_MODEL);
            model = DynEntCl_GetCurrentXModel(dynEntDef, dynEntClient);
            R_AddXModelSurfacesCamera(
                &sceneDynModel->info,
                model,
                dynEntPose->pose.origin,
                gfxEntIndex,
                lightingInfo[sceneEntIndex].lightingHandle,
                lightingInfo[sceneEntIndex].primaryLightIndex,
                0,
                0,
                drawSurfs,
                lastDrawSurfs,
                lightingInfo[sceneEntIndex].reflectionProbeIndex,
                0,
                visLightsMask);
            dist = R_GetBaseLodDist(dynEntPose->pose.origin);
            LodRampType = XModelGetLodRampType(model);
            dista = R_GetAdjustedLodDist(dist, LodRampType);
            ShodLodInfo(
                dynEntPose->pose.origin,
                sceneDynModel->info.lod,
                model->numLods,
                dista,
                model->lodInfo[sceneDynModel->info.lod].dist,
                (model->flags & 0x80000) != 0);
        }
    }
}

void __cdecl R_AddAllSceneEntSurfacesSunShadow()
{
    unsigned int partitionIndex; // [esp+0h] [ebp-4h]

    for ( partitionIndex = 0; partitionIndex < 2; ++partitionIndex )
        R_AddAllSceneEntSurfacesRangeSunShadow(partitionIndex);
}

void __cdecl R_AddAllSceneEntSurfacesRangeSunShadow(unsigned int partitionIndex)
{
    char *Name; // eax
    DynEntityPose *dynEntPose; // [esp+10h] [ebp-4Ch]
    XModel *dynEntXModel; // [esp+14h] [ebp-48h]
    GfxSceneDynBrush *sceneDynBrush; // [esp+18h] [ebp-44h]
    GfxDrawSurf *drawSurf; // [esp+20h] [ebp-3Ch]
    unsigned __int8 shadowmapBuildTechType; // [esp+27h] [ebp-35h]
    volatile unsigned int sceneEntCount; // [esp+28h] [ebp-34h]
    volatile unsigned int sceneEntCounta; // [esp+28h] [ebp-34h]
    unsigned int sceneEntCountb; // [esp+28h] [ebp-34h]
    volatile unsigned int sceneEntCountc; // [esp+28h] [ebp-34h]
    unsigned int sceneEntCountd; // [esp+28h] [ebp-34h]
    DynEntityClient *dynEntClient; // [esp+2Ch] [ebp-30h]
    unsigned int stage; // [esp+30h] [ebp-2Ch]
    const DynEntityDef *dynEntDef; // [esp+34h] [ebp-28h]
    const DynEntityDef *dynEntDefa; // [esp+34h] [ebp-28h]
    unsigned int sceneEntIndex; // [esp+38h] [ebp-24h]
    unsigned int sceneEntIndexa; // [esp+38h] [ebp-24h]
    unsigned int sceneEntIndexb; // [esp+38h] [ebp-24h]
    unsigned int sceneEntIndexc; // [esp+38h] [ebp-24h]
    unsigned int sceneEntIndexd; // [esp+38h] [ebp-24h]
    unsigned __int16 dynEntId; // [esp+44h] [ebp-18h]
    unsigned __int16 dynEntIda; // [esp+44h] [ebp-18h]
    unsigned __int8 *sceneEntVisData; // [esp+48h] [ebp-14h]
    unsigned __int8 *sceneEntVisDataa; // [esp+48h] [ebp-14h]
    GfxSceneDynModel *sceneDynModel; // [esp+4Ch] [ebp-10h]
    GfxBrushModel *bmodel; // [esp+50h] [ebp-Ch]
    signed int drawSurfCount; // [esp+54h] [ebp-8h]
    GfxDrawSurf *lastDrawSurf; // [esp+58h] [ebp-4h]

    stage = 4 * partitionIndex + 21;
    drawSurf = scene.drawSurfs[stage];
    lastDrawSurf = &drawSurf[scene.maxDrawSurfCount[stage]];
    shadowmapBuildTechType = gfxMetrics.shadowmapBuildTechType;
    sceneEntCount = scene.sceneDObjCount;
    for ( sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex )
    {
        if ( scene.sceneDObjVisData[partitionIndex + 1][sceneEntIndex] == 1 )
            drawSurf = R_AddDObjSurfaces(&scene.sceneDObj[sceneEntIndex], shadowmapBuildTechType, drawSurf, lastDrawSurf);
    }
    sceneEntCounta = scene.sceneModelCount;
    for ( sceneEntIndexa = 0; sceneEntIndexa < sceneEntCounta; ++sceneEntIndexa )
    {
        if ( scene.sceneModelVisData[partitionIndex + 1][sceneEntIndexa] == 1 )
            drawSurf = R_AddXModelSurfaces(
                                     &scene.sceneModel[sceneEntIndexa].info,
                                     scene.sceneModel[sceneEntIndexa].model,
                                     shadowmapBuildTechType,
                                     drawSurf,
                                     lastDrawSurf);
    }
    sceneEntCountb = scene.sceneDynModelCount;
    sceneEntVisData = rgp.world->dpvsDyn.dynEntVisData[0][partitionIndex + 1];
    for ( sceneEntIndexb = 0; sceneEntIndexb < sceneEntCountb; ++sceneEntIndexb )
    {
        sceneDynModel = &rgp.world->sceneDynModel[sceneEntIndexb];
        dynEntId = sceneDynModel->dynEntId;
        if ( sceneEntVisData[dynEntId] == 1 )
        {
            dynEntDef = DynEnt_GetEntityDef(dynEntId, DYNENT_DRAW_MODEL);
            dynEntClient = DynEnt_GetClientEntity(dynEntId, DYNENT_DRAW_MODEL);
            dynEntXModel = DynEntCl_GetCurrentXModel(dynEntDef, dynEntClient);
            if ( r_showDynEntModelNames->current.enabled )
            {
                dynEntPose = DynEnt_GetClientPose(dynEntId, DYNENT_DRAW_MODEL);
                if ( dynEntPose )
                {
                    Name = (char *)XModelGetName(dynEntDef->xModel);
                    R_AddDebugString(&frontEndDataOut->debugGlobals, dynEntPose->pose.origin, colorWhite, 0.3f, Name);
                }
            }
            drawSurf = R_AddXModelSurfaces(&sceneDynModel->info, dynEntXModel, shadowmapBuildTechType, drawSurf, lastDrawSurf);
        }
    }
    sceneEntCountc = scene.sceneBrushCount;
    for ( sceneEntIndexc = 0; sceneEntIndexc < sceneEntCountc; ++sceneEntIndexc )
    {
        if ( scene.sceneBrushVisData[partitionIndex + 1][sceneEntIndexc] == 1 )
            drawSurf = R_AddBModelSurfaces(
                                     &scene.sceneBrush[sceneEntIndexc].info,
                                     scene.sceneBrush[sceneEntIndexc].bmodel,
                                     shadowmapBuildTechType,
                                     drawSurf,
                                     lastDrawSurf);
    }
    sceneEntCountd = scene.sceneDynBrushCount;
    sceneEntVisDataa = rgp.world->dpvsDyn.dynEntVisData[1][partitionIndex + 1];
    for ( sceneEntIndexd = 0; sceneEntIndexd < sceneEntCountd; ++sceneEntIndexd )
    {
        sceneDynBrush = &rgp.world->sceneDynBrush[sceneEntIndexd];
        dynEntIda = sceneDynBrush->dynEntId;
        if ( sceneEntVisDataa[dynEntIda] == 1 )
        {
            dynEntDefa = DynEnt_GetEntityDef(dynEntIda, DYNENT_DRAW_BRUSH);
            bmodel = R_GetBrushModel(dynEntDefa->brushModel);
            drawSurf = R_AddBModelSurfaces(
                                     (BModelDrawInfo *)sceneDynBrush,
                                     bmodel,
                                     shadowmapBuildTechType,
                                     drawSurf,
                                     lastDrawSurf);
        }
    }
    drawSurfCount = drawSurf - scene.drawSurfs[stage];
    scene.drawSurfCount[stage] = drawSurfCount;
    {
        PROF_SCOPED("sort surfs");
        R_SortDrawSurfs(scene.drawSurfs[stage], drawSurfCount);
    }
    
}

void __cdecl R_AddAllSceneEntSurfacesSpotShadow(
                const GfxViewInfo *viewInfo,
                unsigned int spotShadowIndex,
                unsigned int primaryLightIndex)
{
    XModel *dynEntXModel; // [esp+8h] [ebp-48h]
    GfxSceneDynBrush *sceneDynBrush; // [esp+Ch] [ebp-44h]
    GfxDrawSurf *drawSurf; // [esp+14h] [ebp-3Ch]
    unsigned __int8 shadowmapBuildTechType; // [esp+1Bh] [ebp-35h]
    volatile unsigned int sceneEntCount; // [esp+1Ch] [ebp-34h]
    volatile unsigned int sceneEntCounta; // [esp+1Ch] [ebp-34h]
    unsigned int sceneEntCountb; // [esp+1Ch] [ebp-34h]
    volatile unsigned int sceneEntCountc; // [esp+1Ch] [ebp-34h]
    unsigned int sceneEntCountd; // [esp+1Ch] [ebp-34h]
    DynEntityClient *dynEntClient; // [esp+20h] [ebp-30h]
    unsigned int stage; // [esp+24h] [ebp-2Ch]
    const DynEntityDef *dynEntDef; // [esp+28h] [ebp-28h]
    const DynEntityDef *dynEntDefa; // [esp+28h] [ebp-28h]
    unsigned int sceneEntIndex; // [esp+2Ch] [ebp-24h]
    unsigned int sceneEntIndexa; // [esp+2Ch] [ebp-24h]
    unsigned int sceneEntIndexb; // [esp+2Ch] [ebp-24h]
    unsigned int sceneEntIndexc; // [esp+2Ch] [ebp-24h]
    unsigned int sceneEntIndexd; // [esp+2Ch] [ebp-24h]
    GfxSceneBrush *sceneBrush; // [esp+34h] [ebp-1Ch]
    unsigned __int16 dynEntId; // [esp+38h] [ebp-18h]
    unsigned __int16 dynEntIda; // [esp+38h] [ebp-18h]
    GfxSceneDynModel *sceneDynModel; // [esp+40h] [ebp-10h]
    GfxBrushModel *bmodel; // [esp+44h] [ebp-Ch]
    signed int drawSurfCount; // [esp+48h] [ebp-8h]
    GfxDrawSurf *lastDrawSurf; // [esp+4Ch] [ebp-4h]

    if ( !R_IsPrimaryLight(primaryLightIndex)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    2000,
                    0,
                    "%s",
                    "R_IsPrimaryLight( primaryLightIndex )") )
    {
        __debugbreak();
    }
    stage = 3 * spotShadowIndex + 29;
    drawSurf = scene.drawSurfs[stage];
    lastDrawSurf = &drawSurf[scene.maxDrawSurfCount[stage]];
    shadowmapBuildTechType = gfxMetrics.shadowmapBuildTechType;
    sceneEntCount = scene.sceneDObjCount;
    for ( sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex )
    {
        if ( scene.sceneDObjVisData[spotShadowIndex + 3][sceneEntIndex] == 1 )
            drawSurf = R_AddDObjSurfaces(&scene.sceneDObj[sceneEntIndex], shadowmapBuildTechType, drawSurf, lastDrawSurf);
    }
    sceneEntCounta = scene.sceneModelCount;
    for ( sceneEntIndexa = 0; sceneEntIndexa < sceneEntCounta; ++sceneEntIndexa )
    {
        if ( scene.sceneModelVisData[spotShadowIndex + 3][sceneEntIndexa] == 1 )
            drawSurf = R_AddXModelSurfaces(
                                     &scene.sceneModel[sceneEntIndexa].info,
                                     scene.sceneModel[sceneEntIndexa].model,
                                     shadowmapBuildTechType,
                                     drawSurf,
                                     lastDrawSurf);
    }
    sceneEntCountb = scene.sceneDynModelCount;
    for ( sceneEntIndexb = 0; sceneEntIndexb < sceneEntCountb; ++sceneEntIndexb )
    {
        sceneDynModel = &rgp.world->sceneDynModel[sceneEntIndexb];
        dynEntId = sceneDynModel->dynEntId;
        if ( R_IsDynEntVisibleToPrimaryLight(dynEntId, DYNENT_DRAW_MODEL, primaryLightIndex) )
        {
            dynEntDef = DynEnt_GetEntityDef(dynEntId, DYNENT_DRAW_MODEL);
            dynEntClient = DynEnt_GetClientEntity(dynEntId, DYNENT_DRAW_MODEL);
            dynEntXModel = DynEntCl_GetCurrentXModel(dynEntDef, dynEntClient);
            drawSurf = R_AddXModelSurfaces(&sceneDynModel->info, dynEntXModel, shadowmapBuildTechType, drawSurf, lastDrawSurf);
        }
    }
    sceneEntCountc = scene.sceneBrushCount;
    for ( sceneEntIndexc = 0; sceneEntIndexc < sceneEntCountc; ++sceneEntIndexc )
    {
        sceneBrush = &scene.sceneBrush[sceneEntIndexc];
        if ( R_IsEntityVisibleToPrimaryLight(viewInfo->localClientNum, sceneBrush->entnum, primaryLightIndex)
            && !Com_BitCheckAssert(scene.entOverflowedDrawBuf, sceneBrush->entnum, 0xFFFFFFF) )
        {
            drawSurf = R_AddBModelSurfaces(
                                     &sceneBrush->info,
                                     sceneBrush->bmodel,
                                     shadowmapBuildTechType,
                                     drawSurf,
                                     lastDrawSurf);
        }
    }
    sceneEntCountd = scene.sceneDynBrushCount;
    for ( sceneEntIndexd = 0; sceneEntIndexd < sceneEntCountd; ++sceneEntIndexd )
    {
        sceneDynBrush = &rgp.world->sceneDynBrush[sceneEntIndexd];
        dynEntIda = sceneDynBrush->dynEntId;
        if ( R_IsDynEntVisibleToPrimaryLight(dynEntIda, DYNENT_DRAW_BRUSH, primaryLightIndex) )
        {
            dynEntDefa = DynEnt_GetEntityDef(dynEntIda, DYNENT_DRAW_BRUSH);
            bmodel = R_GetBrushModel(dynEntDefa->brushModel);
            R_AddBModelSurfaces((BModelDrawInfo *)sceneDynBrush, bmodel, shadowmapBuildTechType, drawSurf, lastDrawSurf);
        }
    }
    drawSurfCount = drawSurf - scene.drawSurfs[stage];
    scene.drawSurfCount[stage] = drawSurfCount;

    {
        PROF_SCOPED("sort surfs");
        R_SortDrawSurfs(scene.drawSurfs[stage], drawSurfCount);
    }
}

void __cdecl R_AddSceneDObj(unsigned int entnum, unsigned int viewIndex)
{
    if ( entnum == gfxCfg.entnumNone
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    2096,
                    0,
                    "%s",
                    "entnum != gfxCfg.entnumNone") )
    {
        __debugbreak();
    }
    *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnum) = 1;
}

void __cdecl R_DrawAllSceneEnt(const GfxViewInfo *viewInfo)
{
    unsigned __int8 v1; // [esp+10h] [ebp-70h]
    unsigned __int8 v2; // [esp+14h] [ebp-6Ch]
    unsigned int glassIndex; // [esp+1Ch] [ebp-64h]
    const ShaderConstantSet *constSet; // [esp+20h] [ebp-60h]
    char viewVisData; // [esp+28h] [ebp-58h]
    unsigned __int8 viewVisDataa; // [esp+28h] [ebp-58h]
    char viewVisDatab; // [esp+28h] [ebp-58h]
    unsigned __int8 viewVisDatac; // [esp+28h] [ebp-58h]
    unsigned __int8 viewVisDatad; // [esp+28h] [ebp-58h]
    unsigned int *entVisBits; // [esp+2Ch] [ebp-54h]
    GfxSceneModel *sceneModel; // [esp+30h] [ebp-50h]
    volatile unsigned int sceneEntCount; // [esp+38h] [ebp-48h]
    volatile unsigned int sceneEntCounta; // [esp+38h] [ebp-48h]
    volatile unsigned int sceneEntCountb; // [esp+38h] [ebp-48h]
    DpvsView *views; // [esp+3Ch] [ebp-44h]
    GfxEntity *gfxEnt; // [esp+40h] [ebp-40h]
    GfxEntity *gfxEnta; // [esp+40h] [ebp-40h]
    unsigned int sceneEntIndex; // [esp+44h] [ebp-3Ch]
    unsigned int sceneEntIndexa; // [esp+44h] [ebp-3Ch]
    unsigned int sceneEntIndexb; // [esp+44h] [ebp-3Ch]
    GfxSceneEntity *sceneEnt; // [esp+48h] [ebp-38h]
    GfxSceneBrush *sceneBrush; // [esp+4Ch] [ebp-34h]
    volatile unsigned int glassBrushCount; // [esp+54h] [ebp-2Ch]
    unsigned int entnum; // [esp+58h] [ebp-28h]
    unsigned int entnuma; // [esp+58h] [ebp-28h]
    unsigned int entnumb; // [esp+58h] [ebp-28h]
    unsigned __int8 *sceneEntVisData[7]; // [esp+5Ch] [ebp-24h]
    unsigned int viewIndex; // [esp+78h] [ebp-8h]
    unsigned int visData; // [esp+7Ch] [ebp-4h]
    int savedregs; // [esp+80h] [ebp+0h] BYREF

    entVisBits = dpvsGlob.entVisBits[scene.dpvs.localClientNum];
    for ( viewIndex = 0; viewIndex < 7; ++viewIndex )
        sceneEntVisData[viewIndex] = scene.sceneDObjVisData[viewIndex];
    sceneEntCount = scene.sceneDObjCount;
    if ( scene.dpvs.localClientNum != viewInfo->localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    2262,
                    0,
                    "%s",
                    "scene.dpvs.localClientNum == viewInfo->localClientNum") )
    {
        __debugbreak();
    }
    views = dpvsGlob.views[scene.dpvs.localClientNum];
    for ( sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex )
    {
        sceneEnt = &scene.sceneDObj[sceneEntIndex];
        entnum = sceneEnt->entnum;
        if ( sceneEnt->gfxEntIndex )
        {
            gfxEnt = &frontEndDataOut->gfxEnts[scene.sceneDObj[sceneEntIndex].gfxEntIndex];
            if ( entnum == gfxCfg.entnumNone )
            {
                visData = 0;
                for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
                {
                    sceneEntVisData[viewIndex][sceneEntIndex] = (views[viewIndex].renderFxFlagsCull & gfxEnt->renderFxFlags) == 0;
                    visData |= sceneEntVisData[viewIndex][sceneEntIndex];
                }
                while ( viewIndex < 7 )
                {
                    sceneEntVisData[viewIndex][sceneEntIndex] = *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnum);
                    visData |= sceneEntVisData[viewIndex++][sceneEntIndex];
                }
            }
            else
            {
                if ( (entVisBits[entnum >> 5] & (0x80000000 >> (entnum & 0x1F))) != 0 )
                {
                    visData = 0;
                    for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
                    {
                        if ( (views[viewIndex].renderFxFlagsCull & gfxEnt->renderFxFlags) != 0 )
                            v2 = 0;
                        else
                            v2 = *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnum);
                        sceneEntVisData[viewIndex][sceneEntIndex] = v2;
                        visData |= sceneEntVisData[viewIndex][sceneEntIndex];
                    }
                    while ( viewIndex < 7 )
                    {
                        sceneEntVisData[viewIndex][sceneEntIndex] = *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnum);
                        visData |= sceneEntVisData[viewIndex++][sceneEntIndex];
                    }
                    if ( (visData & 1) != 0
                        && sceneEnt->cull.state < 2
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                                    2302,
                                    0,
                                    "sceneEnt->cull.state >= CULL_STATE_BOUNDED\n\t%i, %i",
                                    sceneEnt->cull.state,
                                    2) )
                    {
                        __debugbreak();
                    }
                    continue;
                }
                visData = 0;
                for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
                {
                    viewVisData = *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnum);
                    if ( !viewVisData )
                        viewVisData = 1;
                    sceneEntVisData[viewIndex][sceneEntIndex] = (views[viewIndex].renderFxFlagsCull & gfxEnt->renderFxFlags) == 0
                                                                                                        ? viewVisData
                                                                                                        : 0;
                    visData |= sceneEntVisData[viewIndex][sceneEntIndex];
                }
                while ( viewIndex < 7 )
                {
                    sceneEntVisData[viewIndex][sceneEntIndex] = *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnum);
                    visData |= sceneEntVisData[viewIndex++][sceneEntIndex];
                }
            }
            goto LABEL_65;
        }
        if ( entnum == gfxCfg.entnumNone )
        {
            visData = 0;
            for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
            {
                sceneEntVisData[viewIndex][sceneEntIndex] = 1;
                visData |= sceneEntVisData[viewIndex][sceneEntIndex];
            }
            while ( viewIndex < 7 )
            {
                sceneEntVisData[viewIndex][sceneEntIndex] = *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnum);
                visData |= sceneEntVisData[viewIndex++][sceneEntIndex];
            }
LABEL_65:
            if ( (visData & 1) != 0 )
            {
                if ( R_SkinAndBoundSceneEnt(sceneEnt) )
                {
                    if ( sceneEnt->cull.state < 2
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                                    2414,
                                    0,
                                    "sceneEnt->cull.state >= CULL_STATE_BOUNDED\n\t%i, %i",
                                    sceneEnt->cull.state,
                                    2) )
                    {
                        __debugbreak();
                    }
                }
                else
                {
                    for ( viewIndex = 0; viewIndex < 7; ++viewIndex )
                        sceneEntVisData[viewIndex][sceneEntIndex] = 0;
                }
            }
            continue;
        }
        if ( (entVisBits[entnum >> 5] & (0x80000000 >> (entnum & 0x1F))) == 0 )
        {
            visData = 0;
            for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
            {
                viewVisDataa = *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnum);
                if ( !viewVisDataa )
                    viewVisDataa = 1;
                sceneEntVisData[viewIndex][sceneEntIndex] = viewVisDataa;
                visData |= sceneEntVisData[viewIndex][sceneEntIndex];
            }
            while ( viewIndex < 7 )
            {
                sceneEntVisData[viewIndex][sceneEntIndex] = *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnum);
                visData |= sceneEntVisData[viewIndex++][sceneEntIndex];
            }
            goto LABEL_65;
        }
        visData = 0;
        for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
        {
            sceneEntVisData[viewIndex][sceneEntIndex] = *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnum);
            visData |= sceneEntVisData[viewIndex][sceneEntIndex];
        }
        while ( viewIndex < 7 )
        {
            sceneEntVisData[viewIndex][sceneEntIndex] = *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnum);
            visData |= sceneEntVisData[viewIndex++][sceneEntIndex];
        }
        if ( (visData & 1) != 0
            && sceneEnt->cull.state < 2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                        2369,
                        0,
                        "sceneEnt->cull.state >= CULL_STATE_BOUNDED\n\t%i, %i",
                        sceneEnt->cull.state,
                        2) )
        {
            __debugbreak();
        }
    }
    for ( viewIndex = 0; viewIndex < 7; ++viewIndex )
        sceneEntVisData[viewIndex] = scene.sceneModelVisData[viewIndex];
    sceneEntCounta = scene.sceneModelCount;
    for ( sceneEntIndexa = 0; sceneEntIndexa < sceneEntCounta; ++sceneEntIndexa )
    {
        entnuma = scene.sceneModel[sceneEntIndexa].entnum;
        visData = 0;
        if ( scene.sceneModel[sceneEntIndexa].gfxEntIndex )
        {
            gfxEnta = &frontEndDataOut->gfxEnts[scene.sceneModel[sceneEntIndexa].gfxEntIndex];
            if ( entnuma == gfxCfg.entnumNone )
            {
                for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
                {
                    if ( (views[viewIndex].renderFxFlagsCull & gfxEnta->renderFxFlags) != 0 )
                        sceneEntVisData[viewIndex][sceneEntIndexa] = 0;
                    else
                        visData |= sceneEntVisData[viewIndex][sceneEntIndexa];
                }
            }
            else if ( (entVisBits[entnuma >> 5] & (0x80000000 >> (entnuma & 0x1F))) != 0 )
            {
                for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
                {
                    if ( (views[viewIndex].renderFxFlagsCull & gfxEnta->renderFxFlags) != 0 )
                        v1 = 0;
                    else
                        v1 = *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnuma);
                    sceneEntVisData[viewIndex][sceneEntIndexa] = v1;
                    visData |= sceneEntVisData[viewIndex][sceneEntIndexa];
                }
            }
            else
            {
                for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
                {
                    viewVisDatab = *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnuma);
                    if ( !viewVisDatab )
                        viewVisDatab = 1;
                    sceneEntVisData[viewIndex][sceneEntIndexa] = (views[viewIndex].renderFxFlagsCull & gfxEnta->renderFxFlags) == 0
                                                                                                         ? viewVisDatab
                                                                                                         : 0;
                    visData |= sceneEntVisData[viewIndex][sceneEntIndexa];
                }
            }
            while ( viewIndex < 7 )
            {
                sceneEntVisData[viewIndex][sceneEntIndexa] = *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnuma);
                visData |= sceneEntVisData[viewIndex++][sceneEntIndexa];
            }
        }
        else
        {
            if ( entnuma == gfxCfg.entnumNone )
            {
                for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
                    visData |= sceneEntVisData[viewIndex][sceneEntIndexa];
            }
            else if ( (entVisBits[entnuma >> 5] & (0x80000000 >> (entnuma & 0x1F))) != 0 )
            {
                for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
                {
                    sceneEntVisData[viewIndex][sceneEntIndexa] = *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnuma);
                    visData |= sceneEntVisData[viewIndex][sceneEntIndexa];
                }
            }
            else
            {
                for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
                {
                    viewVisDatac = *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnuma);
                    if ( !viewVisDatac )
                        viewVisDatac = 1;
                    sceneEntVisData[viewIndex][sceneEntIndexa] = viewVisDatac;
                    visData |= sceneEntVisData[viewIndex][sceneEntIndexa];
                }
            }
            while ( viewIndex < 7 )
            {
                sceneEntVisData[viewIndex][sceneEntIndexa] = *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnuma);
                visData |= sceneEntVisData[viewIndex++][sceneEntIndexa];
            }
        }
        if ( (visData & 1) != 0 )
        {
            sceneModel = &scene.sceneModel[sceneEntIndexa];
            if ( !R_SkinXModel(
                            &sceneModel->info,
                            sceneModel->model,
                            sceneModel->obj,
                            &sceneModel->placement.base,
                            sceneModel->placement.scale,
                            sceneModel->gfxEntIndex,
                            (sceneEntVisData[0][sceneEntIndexa] & 1) == 0) )
            {
                for ( viewIndex = 0; viewIndex < 7; ++viewIndex )
                    sceneEntVisData[viewIndex][sceneEntIndexa] = 0;
            }
        }
    }
    for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
        sceneEntVisData[viewIndex] = scene.sceneBrushVisData[viewIndex];
    sceneEntCountb = scene.sceneBrushCount;
    for ( sceneEntIndexb = 0; sceneEntIndexb < sceneEntCountb; ++sceneEntIndexb )
    {
        sceneBrush = &scene.sceneBrush[sceneEntIndexb];
        entnumb = sceneBrush->entnum;
        constSet = sceneBrush->brushConstantSet;
        if ( entnumb == gfxCfg.entnumNone
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                        2542,
                        0,
                        "%s",
                        "entnum != gfxCfg.entnumNone") )
        {
            __debugbreak();
        }
        visData = 0;
        if ( (entVisBits[entnumb >> 5] & (0x80000000 >> (entnumb & 0x1F))) != 0 )
        {
            for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
            {
                sceneEntVisData[viewIndex][sceneEntIndexb] = *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnumb);
                visData |= sceneEntVisData[viewIndex][sceneEntIndexb];
            }
        }
        else
        {
            for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
            {
                viewVisDatad = *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnumb);
                if ( !viewVisDatad )
                    viewVisDatad = 1;
                sceneEntVisData[viewIndex][sceneEntIndexb] = viewVisDatad;
                visData |= sceneEntVisData[viewIndex][sceneEntIndexb];
            }
        }
        if ( ((visData & 1) != 0 || R_IsEntityVisibleToAnyShadowedPrimaryLight(viewInfo, entnumb))
            && !R_DrawBModel(&sceneBrush->info, sceneBrush->bmodel, &sceneBrush->placement, constSet) )
        {
            Com_BitSetAssert(scene.entOverflowedDrawBuf, sceneBrush->entnum, 0xFFFFFFF);
            for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
                sceneEntVisData[viewIndex][sceneEntIndexb] = 0;
        }
    }
    glassBrushCount = scene.glassBrushCount;
    for ( glassIndex = 0; glassIndex < glassBrushCount; ++glassIndex )
    {
        if ( (scene.glassBrushVisData[glassIndex] & 1) != 0
            //&& !R_DrawBModel(
            //            (BModelDrawInfo *)&scene.glassBrushVisData[40 * glassIndex - 40924],
            //            *(const GfxBrushModel **)&scene.glassBrushVisData[40 * glassIndex - 40932],
            //            (const GfxPlacement *)&scene.glassBrush[glassIndex].placement,
            //            NULL) )
            && !R_DrawBModel(&scene.glassBrush[glassIndex].info, scene.glassBrush[glassIndex].bmodel, &scene.glassBrush[glassIndex].placement, NULL))
        {
            scene.glassBrushVisData[glassIndex] = 0;
        }
    }
}

int __cdecl R_DrawBModel(
                BModelDrawInfo *bmodelInfo,
                const GfxBrushModel *bmodel,
                const GfxPlacement *placement,
                const ShaderConstantSet *constSet)
{
    unsigned int surfId; // [esp+Ch] [ebp-20h]
    unsigned int startSurfPos; // [esp+10h] [ebp-1Ch]
    GfxScaledPlacement *newPlacement; // [esp+14h] [ebp-18h]
    unsigned int surfIndex; // [esp+18h] [ebp-14h]
    unsigned int surfaceCount; // [esp+1Ch] [ebp-10h]
    int size; // [esp+20h] [ebp-Ch]
    BModelSurface *bmodelSurf; // [esp+24h] [ebp-8h]
    ShaderConstantSet *newConstantSet; // [esp+28h] [ebp-4h]

    surfaceCount = bmodel->surfaceCount;
    if ( !surfaceCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 2113, 0, "%s", "surfaceCount") )
    {
        __debugbreak();
    }
    size = 20 * surfaceCount + 32;
    if ( constSet )
        size = 20 * surfaceCount + 152;
    newConstantSet = 0;
    startSurfPos = _InterlockedExchangeAdd(&frontEndDataOut->surfPos, size);
    if ( size + startSurfPos <= 0x40000 )
    {
        if ( (startSurfPos & 3) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                        2137,
                        0,
                        "%s",
                        "!(startSurfPos & 3)") )
        {
            __debugbreak();
        }
        if ( constSet )
        {
            newConstantSet = (ShaderConstantSet *)&frontEndDataOut->surfsBuffer[startSurfPos];
            memcpy(&frontEndDataOut->surfsBuffer[startSurfPos], constSet, 0x78u);
            startSurfPos += 120;
        }
        newPlacement = (GfxScaledPlacement *)&frontEndDataOut->surfsBuffer[startSurfPos];
        memcpy(&frontEndDataOut->surfsBuffer[startSurfPos], placement, 0x1Cu);
        newPlacement->scale = 1.0f;
        bmodelSurf = (BModelSurface *)&newPlacement[1];
        surfId = (char *)&newPlacement[1] - (char *)frontEndDataOut;
        if ( (((_BYTE)newPlacement + 32 - (_BYTE)frontEndDataOut) & 3) != 0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 2154, 0, "%s", "!(surfId & 3)") )
        {
            __debugbreak();
        }
        bmodelInfo->surfId = surfId >> 2;
        for ( surfIndex = 0; surfIndex < surfaceCount; ++surfIndex )
        {
            bmodelSurf->placement = newPlacement;
            bmodelSurf->surf = &rgp.world->dpvs.surfaces[surfIndex + bmodel->startSurfIndex];
            bmodelSurf->shaderConstSet = newConstantSet;
            ++bmodelSurf;
        }
        return 1;
    }
    else
    {
        R_WarnOncePerFrame(R_WARN_MAX_SCENE_SURFS_SIZE);
        return 0;
    }
}

void __cdecl R_DrawAllDynEnt(const GfxViewInfo *viewInfo)
{
    XModel *model; // [esp+Ch] [ebp-44h]
    unsigned int visDataCamera; // [esp+10h] [ebp-40h]
    DynEntityPose *dynEntPose; // [esp+14h] [ebp-3Ch]
    DynEntityPose *dynEntPosea; // [esp+14h] [ebp-3Ch]
    GfxSceneDynBrush *sceneDynBrush; // [esp+18h] [ebp-38h]
    unsigned int dynEntIndex; // [esp+1Ch] [ebp-34h]
    unsigned int dynEntIndexa; // [esp+1Ch] [ebp-34h]
    DynEntityClient *dynEntClient; // [esp+20h] [ebp-30h]
    unsigned int dynEntCount; // [esp+24h] [ebp-2Ch]
    unsigned int dynEntCounta; // [esp+24h] [ebp-2Ch]
    const DynEntityDef *dynEntDef; // [esp+28h] [ebp-28h]
    const DynEntityDef *dynEntDefa; // [esp+28h] [ebp-28h]
    unsigned __int8 *dynEntVisData[3]; // [esp+34h] [ebp-1Ch]
    GfxSceneDynModel *sceneDynModel; // [esp+40h] [ebp-10h]
    unsigned int viewIndex; // [esp+44h] [ebp-Ch]
    unsigned int visData; // [esp+48h] [ebp-8h]
    GfxBrushModel *bmodel; // [esp+4Ch] [ebp-4h]
    int savedregs; // [esp+50h] [ebp+0h] BYREF

    for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
        dynEntVisData[viewIndex] = rgp.world->dpvsDyn.dynEntVisData[0][viewIndex];
    dynEntCount = rgp.world->dpvsDyn.dynEntClientCount[0];
    for ( dynEntIndex = 0; dynEntIndex < dynEntCount; ++dynEntIndex )
    {
        visDataCamera = dynEntVisData[0][dynEntIndex];
        visData = dynEntVisData[2][dynEntIndex] | visDataCamera | dynEntVisData[1][dynEntIndex];
        if ( (visData & 1) != 0 || R_IsDynEntVisibleToAnyShadowedPrimaryLight(viewInfo, dynEntIndex, DYNENT_DRAW_MODEL) )
        {
            dynEntPose = DynEnt_GetClientPose(dynEntIndex, DYNENT_DRAW_MODEL);
            dynEntDef = DynEnt_GetEntityDef(dynEntIndex, DYNENT_DRAW_MODEL);
            dynEntClient = DynEnt_GetClientEntity(dynEntIndex, DYNENT_DRAW_MODEL);
            if ( !dynEntDef->xModel
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                            2641,
                            0,
                            "%s",
                            "dynEntDef->xModel") )
            {
                __debugbreak();
            }
            sceneDynModel = &rgp.world->sceneDynModel[scene.sceneDynModelCount];
            model = DynEntCl_GetCurrentXModel(dynEntDef, dynEntClient);
            if ( R_SkinXModel(
                         &sceneDynModel->info,
                         model,
                         0,
                         &dynEntPose->pose,
                         1.0,
                         0,
                         (visDataCamera & 1) == 0) )
            {
                sceneDynModel->dynEntId = dynEntIndex;
                ++scene.sceneDynModelCount;
            }
            else
            {
                dynEntVisData[0][dynEntIndex] = 0;
                dynEntVisData[1][dynEntIndex] = 0;
                dynEntVisData[2][dynEntIndex] = 0;
            }
        }
    }
    if ( !rg.drawXModels )
        scene.sceneDynModelCount = 0;
    for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
        dynEntVisData[viewIndex] = rgp.world->dpvsDyn.dynEntVisData[1][viewIndex];
    dynEntCounta = rgp.world->dpvsDyn.dynEntClientCount[1];
    for ( dynEntIndexa = 0; dynEntIndexa < dynEntCounta; ++dynEntIndexa )
    {
        visData = dynEntVisData[2][dynEntIndexa] | dynEntVisData[1][dynEntIndexa] | dynEntVisData[0][dynEntIndexa];
        if ( (visData & 1) != 0 )
        {
            dynEntPosea = DynEnt_GetClientPose(dynEntIndexa, DYNENT_DRAW_BRUSH);
            dynEntDefa = DynEnt_GetEntityDef(dynEntIndexa, DYNENT_DRAW_BRUSH);
            if ( dynEntDefa->xModel
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                            2678,
                            0,
                            "%s",
                            "!dynEntDef->xModel") )
            {
                __debugbreak();
            }
            if ( !dynEntDefa->brushModel
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                            2679,
                            0,
                            "%s",
                            "dynEntDef->brushModel") )
            {
                __debugbreak();
            }
            bmodel = R_GetBrushModel(dynEntDefa->brushModel);
            if ( bmodel->surfaceCount )
            {
                sceneDynBrush = &rgp.world->sceneDynBrush[scene.sceneDynBrushCount];
                if ( R_DrawBModel(&sceneDynBrush->info, bmodel, &dynEntPosea->pose, 0) )
                {
                    sceneDynBrush->dynEntId = dynEntIndexa;
                    ++scene.sceneDynBrushCount;
                }
                else
                {
                    dynEntVisData[0][dynEntIndexa] = 0;
                    dynEntVisData[1][dynEntIndexa] = 0;
                    dynEntVisData[2][dynEntIndexa] = 0;
                }
            }
        }
    }
    if ( !rg.drawBModels )
        scene.sceneDynBrushCount = 0;
}

void __cdecl R_UnfilterEntFromCells(int localClientNum, unsigned int entnum)
{
    unsigned int cellIndex; // [esp+0h] [ebp-18h]
    unsigned int invBit; // [esp+4h] [ebp-14h]
    unsigned int offset; // [esp+8h] [ebp-10h]
    unsigned int *entCellBits; // [esp+Ch] [ebp-Ch]
    unsigned int cellCount; // [esp+10h] [ebp-8h]
    unsigned int cellCounta; // [esp+10h] [ebp-8h]
    unsigned int wordIndex; // [esp+14h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 3010, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 3011, 0, "%s", "rgp.world") )
    {
        __debugbreak();
    }
    if ( entnum == gfxCfg.entnumNone
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    3012,
                    0,
                    "%s",
                    "entnum != gfxCfg.entnumNone") )
    {
        __debugbreak();
    }
    if ( gfxCfg.entCount * gfxCfg.maxClientViews > 0x2000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    3014,
                    0,
                    "%s",
                    "gfxCfg.maxClientViews * gfxCfg.entCount <= MAX_TOTAL_ENT_COUNT") )
    {
        __debugbreak();
    }
    cellCount = rgp.world->dpvsPlanes.cellCount;
    if ( (gfxCfg.entCount & 0x1F) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    3016,
                    0,
                    "%s\n\t(gfxCfg.entCount) = %i",
                    "(!(gfxCfg.entCount & 31))",
                    gfxCfg.entCount) )
    {
        __debugbreak();
    }
    offset = localClientNum * (gfxCfg.entCount >> 5);
    if ( offset >= 0x100
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    3019,
                    0,
                    "offset doesn't index MAX_TOTAL_ENT_COUNT >> 5\n\t%i not in [0, %i)",
                    offset,
                    256) )
    {
        __debugbreak();
    }
    entCellBits = &rgp.world->dpvsPlanes.sceneEntCellBits[offset];
    wordIndex = entnum >> 5;
    invBit = ~(0x80000000 >> (entnum & 0x1F));
    dpvsGlob.entVisBits[localClientNum][entnum >> 5] &= invBit;
    cellCounta = 2 * cellCount;
    for ( cellIndex = 0; cellIndex < cellCounta; ++cellIndex )
    {
        entCellBits[wordIndex] &= invBit;
        wordIndex += 256;
    }
}

void __cdecl R_UnfilterDynEntFromCells(unsigned int dynEntId, DynEntityDrawType drawType)
{
    unsigned int cellIndex; // [esp+0h] [ebp-18h]
    unsigned int cellCount; // [esp+8h] [ebp-10h]
    unsigned int dynEntClientWordCount; // [esp+Ch] [ebp-Ch]
    unsigned int *dynEntCellBits; // [esp+10h] [ebp-8h]
    unsigned int wordIndex; // [esp+14h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 3044, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    dynEntCellBits = rgp.world->dpvsDyn.dynEntCellBits[drawType];
    dynEntClientWordCount = rgp.world->dpvsDyn.dynEntClientWordCount[drawType];
    cellCount = rgp.world->dpvsPlanes.cellCount;
    wordIndex = dynEntId >> 5;
    for ( cellIndex = 0; cellIndex < cellCount; ++cellIndex )
    {
        dynEntCellBits[wordIndex] &= ~(0x80000000 >> (dynEntId & 0x1F));
        wordIndex += dynEntClientWordCount;
    }
}

void __cdecl R_FilterDObjIntoCells(unsigned int localClientNum, unsigned int entnum, float *origin, float radius)
{
    float mins[3]; // [esp+4h] [ebp-28h] BYREF
    FilterEntInfo entInfo; // [esp+10h] [ebp-1Ch] BYREF
    float maxs[3]; // [esp+20h] [ebp-Ch] BYREF

    if ( entnum == gfxCfg.entnumNone
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    3064,
                    0,
                    "%s",
                    "entnum != gfxCfg.entnumNone") )
    {
        __debugbreak();
    }
    if ( localClientNum >= gfxCfg.maxClientViews
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    3065,
                    0,
                    "localClientNum doesn't index gfxCfg.maxClientViews\n\t%i not in [0, %i)",
                    localClientNum,
                    gfxCfg.maxClientViews) )
    {
        __debugbreak();
    }
    R_UnfilterEntFromCells(localClientNum, entnum);
    mins[0] = origin[0] + (-(radius));
    mins[1] = origin[1] + (-(radius));
    mins[2] = origin[2] + (-(radius));
    maxs[0] = origin[0] + radius;
    maxs[1] = origin[1] + radius;
    maxs[2] = origin[2] + radius;
    entInfo.localClientNum = localClientNum;
    entInfo.entnum = entnum;
    entInfo.info.radius = radius;
    entInfo.cellOffset = 0;
    R_FilterEntIntoCells_r(&entInfo, (mnode_t *)rgp.world->dpvsPlanes.nodes, mins, maxs);
}

void __cdecl R_FilterEntIntoCells_r(FilterEntInfo *entInfo, mnode_t *node, const float *mins, const float *maxs)
{
    float localmaxs[3]; // [esp+0h] [ebp-6Ch]
    float dist; // [esp+Ch] [ebp-60h]
    float localmins[3]; // [esp+10h] [ebp-5Ch] BYREF
    unsigned int type; // [esp+1Ch] [ebp-50h]
    unsigned int bit; // [esp+20h] [ebp-4Ch]
    int localClientNum; // [esp+24h] [ebp-48h]
    unsigned int offset; // [esp+28h] [ebp-44h]
    unsigned int *entCellBits; // [esp+2Ch] [ebp-40h]
    int v12; // [esp+30h] [ebp-3Ch]
    unsigned int entnum; // [esp+34h] [ebp-38h]
    unsigned int wordIndex; // [esp+38h] [ebp-34h]
    int side; // [esp+3Ch] [ebp-30h]
    cplane_s *plane; // [esp+40h] [ebp-2Ch]
    int cellIndex; // [esp+44h] [ebp-28h]
    float mins2[3]; // [esp+48h] [ebp-24h] BYREF
    int cellCount; // [esp+54h] [ebp-18h]
    float maxs2[3]; // [esp+58h] [ebp-14h] BYREF
    mnode_t *rightNode; // [esp+64h] [ebp-8h]
    int planeIndex; // [esp+68h] [ebp-4h]

    cellCount = rgp.world->dpvsPlanes.cellCount + 1;
    mins2[0] = *mins;
    mins2[1] = mins[1];
    mins2[2] = mins[2];
    maxs2[0] = *maxs;
    maxs2[1] = maxs[1];
    maxs2[2] = maxs[2];
    while ( 1 )
    {
        cellIndex = node->cellIndex;
        planeIndex = cellIndex - cellCount;
        if ( cellIndex - cellCount < 0 )
            break;
        plane = &rgp.world->dpvsPlanes.planes[planeIndex];
        side = BoxOnPlaneSide(
                         mins2,
                         maxs2,
                         plane);
        if ( side == 3 )
        {
            type = plane->type;
            rightNode = (mnode_t *)((char *)node + 2 * node->rightChildOffset);
            if ( type >= 3 )
            {
                R_FilterEntIntoCells_r(entInfo, node + 1, mins2, maxs2);
            }
            else
            {
                dist = plane->dist;
                localmins[0] = mins2[0];
                localmins[1] = mins2[1];
                localmins[2] = mins2[2];
                localmins[type] = dist;
                localmaxs[0] = maxs2[0];
                localmaxs[1] = maxs2[1];
                localmaxs[2] = maxs2[2];
                localmaxs[type] = dist;
                if ( BoxOnPlaneSide(
                             localmins,
                             maxs2,
                             plane) != 1
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                                2833,
                                0,
                                "%s",
                                "BoxOnPlaneSide( localmins, maxs2, plane ) == BOXSIDE_FRONT") )
                {
                    __debugbreak();
                }
                if ( maxs2[type] > dist )
                    R_FilterEntIntoCells_r(entInfo, node + 1, localmins, maxs2);
                maxs2[0] = localmaxs[0];
                maxs2[1] = localmaxs[1];
                maxs2[2] = localmaxs[2];
            }
            node = rightNode;
        }
        else
        {
            if ( !side )
                side = 1;
            node = (mnode_t *)((char *)node + 2 * (side - 1) * (node->rightChildOffset - 2) + 4);
        }
    }
    if ( cellIndex )
    {
        if ( !Sys_IsMainThread()
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                        2767,
                        0,
                        "%s",
                        "Sys_IsMainThread()") )
        {
            __debugbreak();
        }
        localClientNum = entInfo->localClientNum;
        if ( localClientNum >= gfxCfg.maxClientViews
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                        2770,
                        0,
                        "localClientNum doesn't index gfxCfg.maxClientViews\n\t%i not in [0, %i)",
                        localClientNum,
                        gfxCfg.maxClientViews) )
        {
            __debugbreak();
        }
        entnum = entInfo->entnum;
        if ( gfxCfg.entCount * gfxCfg.maxClientViews > 0x2000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                        2776,
                        0,
                        "%s",
                        "gfxCfg.maxClientViews * gfxCfg.entCount <= MAX_TOTAL_ENT_COUNT") )
        {
            __debugbreak();
        }
        v12 = rgp.world->dpvsPlanes.cellCount;
        if ( (gfxCfg.entCount & 7) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                        2778,
                        0,
                        "%s\n\t(gfxCfg.entCount) = %i",
                        "(!(gfxCfg.entCount & 7))",
                        gfxCfg.entCount) )
        {
            __debugbreak();
        }
        offset = localClientNum * (gfxCfg.entCount >> 5);
        if ( offset >= 0x100
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                        2781,
                        0,
                        "offset doesn't index MAX_TOTAL_ENT_COUNT >> 5\n\t%i not in [0, %i)",
                        offset,
                        256) )
        {
            __debugbreak();
        }
        entCellBits = &rgp.world->dpvsPlanes.sceneEntCellBits[256 * cellIndex - 256 + 256 * entInfo->cellOffset + offset];
        wordIndex = entnum >> 5;
        bit = 0x80000000 >> (entnum & 0x1F);
        entCellBits[entnum >> 5] |= bit;
        dpvsGlob.entVisBits[localClientNum][wordIndex] |= bit;
        *(unsigned int *)(scene.dynSModelVisBitsCamera[localClientNum - 4] + 4 * entnum) = (unsigned int)entInfo->info.bmodel;
    }
}

void __cdecl R_FilterBModelIntoCells(unsigned int localClientNum, unsigned int entnum, GfxBrushModel *bmodel)
{
    FilterEntInfo entInfo; // [esp+0h] [ebp-10h] BYREF

    if ( entnum == gfxCfg.entnumNone
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    3086,
                    0,
                    "%s",
                    "entnum != gfxCfg.entnumNone") )
    {
        __debugbreak();
    }
    if ( localClientNum >= gfxCfg.maxClientViews
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    3087,
                    0,
                    "localClientNum doesn't index gfxCfg.maxClientViews\n\t%i not in [0, %i)",
                    localClientNum,
                    gfxCfg.maxClientViews) )
    {
        __debugbreak();
    }
    R_UnfilterEntFromCells(localClientNum, entnum);
    entInfo.localClientNum = localClientNum;
    entInfo.entnum = entnum;
    entInfo.info.bmodel = bmodel;
    entInfo.cellOffset = rgp.world->dpvsPlanes.cellCount;
    R_FilterEntIntoCells_r(&entInfo, (mnode_t *)rgp.world->dpvsPlanes.nodes, bmodel->writable.mins, bmodel->writable.maxs);
}

void __cdecl R_FilterDynEntIntoCells(unsigned int dynEntId, DynEntityDrawType drawType, float *mins, float *maxs)
{
    R_UnfilterDynEntFromCells(dynEntId, drawType);
    R_FilterDynEntIntoCells_r((mnode_t *)rgp.world->dpvsPlanes.nodes, dynEntId, drawType, mins, maxs);
}

static void __cdecl R_AddDynEntToCell(unsigned int cellIndex, unsigned int dynEntIndex, DynEntityDrawType drawType)
{
    unsigned int wordIndex; // [esp+Ch] [ebp-4h]

    iassert(Sys_IsMainThread());
    wordIndex = rgp.world->dpvsDyn.dynEntClientWordCount[drawType] * cellIndex + (dynEntIndex >> 5);
    rgp.world->dpvsDyn.dynEntCellBits[drawType][wordIndex] |= 0x80000000 >> (dynEntIndex & 0x1F);


    //dynEntCellBits = rgp.world->dpvsDyn.dynEntCellBits[drawType];
    //wordIndex = rgp.world->dpvsDyn.dynEntClientWordCount[drawType] * (cellIndex - 1) + (dynEntIndex >> 5);
    //bit = 0x80000000 >> (dynEntIndex & 0x1F);
    //dynEntCellBits[wordIndex] |= bit;
}

void __cdecl R_FilterDynEntIntoCells_r(
                mnode_t *node,
                unsigned int dynEntIndex,
                DynEntityDrawType drawType,
                const float *mins,
                const float *maxs)
{
    float localmaxs[3]; // [esp+0h] [ebp-50h]
    float dist; // [esp+Ch] [ebp-44h]
    float localmins[3]; // [esp+10h] [ebp-40h] BYREF
    unsigned int type; // [esp+1Ch] [ebp-34h]
    int side; // [esp+20h] [ebp-30h]
    cplane_s *plane; // [esp+24h] [ebp-2Ch]
    int cellIndex; // [esp+28h] [ebp-28h]
    float mins2[3]; // [esp+2Ch] [ebp-24h] BYREF
    int cellCount; // [esp+38h] [ebp-18h]
    float maxs2[3]; // [esp+3Ch] [ebp-14h] BYREF
    mnode_t *rightNode; // [esp+48h] [ebp-8h]
    int planeIndex; // [esp+4Ch] [ebp-4h]

    cellCount = rgp.world->dpvsPlanes.cellCount + 1;

    mins2[0] = mins[0];
    mins2[1] = mins[1];
    mins2[2] = mins[2];

    maxs2[0] = maxs[0];
    maxs2[1] = maxs[1];
    maxs2[2] = maxs[2];

    while (1)
    {
        cellIndex = node->cellIndex;
        planeIndex = cellIndex - cellCount;
        if (cellIndex - cellCount < 0)
            break;
        plane = &rgp.world->dpvsPlanes.planes[planeIndex];
        side = BoxOnPlaneSide(mins2, maxs2, plane);
        if (side == 3)
        {
            type = plane->type;
            rightNode = (mnode_t *)((char *)node + 2 * node->rightChildOffset);
            if (type >= 3)
            {
                R_FilterDynEntIntoCells_r(node + 1, dynEntIndex, drawType, mins2, maxs2);
            }
            else
            {
                dist = plane->dist;

                localmins[0] = mins2[0];
                localmins[1] = mins2[1];
                localmins[2] = mins2[2];
                localmins[type] = dist;

                localmaxs[0] = maxs2[0];
                localmaxs[1] = maxs2[1];
                localmaxs[2] = maxs2[2];
                localmaxs[type] = dist;

                iassert(BoxOnPlaneSide(localmins, maxs2, plane) == BOXSIDE_FRONT);

                if (maxs2[type] > (double)dist)
                    R_FilterDynEntIntoCells_r(node + 1, dynEntIndex, drawType, localmins, maxs2);
                maxs2[0] = localmaxs[0];
                maxs2[1] = localmaxs[1];
                maxs2[2] = localmaxs[2];
            }
            node = rightNode;
        }
        else
        {
            iassert((side == BOXSIDE_FRONT) || (side == BOXSIDE_BACK));
            node = (mnode_t *)((char *)node + ((side - 1) * (node->rightChildOffset - 2)) * 2 + 4);
        }
    }
    if (cellIndex)
        R_AddDynEntToCell(cellIndex - 1, dynEntIndex, drawType);
}

void __cdecl R_FilterXModelIntoScene(
                const XModel *model,
                const GfxScaledPlacement *placement,
                unsigned __int16 renderFxFlags,
                unsigned __int16 *cachedLightingHandle,
                float lightToleranceSq)
{
    const char *v5; // eax
    GfxSceneModel *sceneModel; // [esp+40h] [ebp-28h]
    float radius; // [esp+48h] [ebp-20h]
    DpvsView *views; // [esp+4Ch] [ebp-1Ch]
    GfxEntity *gfxEnt; // [esp+50h] [ebp-18h]
    unsigned int sceneEntIndex; // [esp+54h] [ebp-14h]
    unsigned int gfxEntIndex; // [esp+58h] [ebp-10h]
    unsigned int cullCount; // [esp+5Ch] [ebp-Ch]
    unsigned __int8 sceneEntVisData[4]; // [esp+60h] [ebp-8h]
    unsigned int viewIndex; // [esp+64h] [ebp-4h]

    if ( !model && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 3136, 0, "%s", "model") )
        __debugbreak();
    if ( placement->scale <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    3137,
                    0,
                    "%s",
                    "placement->scale > 0") )
    {
        __debugbreak();
    }
    if ( !Vec4IsNormalized(placement->base.quat) )
    {
        v5 = va(
                     "%g %g %g %g",
                     placement->base.quat[0],
                     placement->base.quat[1],
                     placement->base.quat[2],
                     placement->base.quat[3]);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                        3138,
                        0,
                        "%s\n\t%s",
                        "Vec4IsNormalized( placement->base.quat )",
                        v5) )
            __debugbreak();
    }
    radius = XModelGetRadius(model) * placement->scale;
    cullCount = 0;
    views = dpvsGlob.views[scene.dpvs.localClientNum];
    for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
    {
        if ( R_CullSphereDpvs(
                     placement->base.origin,
                     radius,
                     views[viewIndex].frustumPlanes,
                     views[viewIndex].frustumPlaneCount) )
        {
            ++cullCount;
            sceneEntVisData[viewIndex] = 2;
        }
        else
        {
            sceneEntVisData[viewIndex] = 1;
        }
    }
    if ( cullCount != 3 && r_drawXModels->current.enabled )
    {
        if ( renderFxFlags )
        {
            gfxEntIndex = _InterlockedExchangeAdd(&frontEndDataOut->gfxEntCount, 1u);
            if ( gfxEntIndex >= 0x100 )
            {
                frontEndDataOut->gfxEntCount = 256;
                R_WarnOncePerFrame(R_WARN_KNOWN_SPECIAL_MODELS, 256);
                return;
            }
            gfxEnt = &frontEndDataOut->gfxEnts[gfxEntIndex];
            frontEndDataOut->gfxEnts[gfxEntIndex].materialTime = 0.0f;
            gfxEnt->renderFxFlags = renderFxFlags;
            gfxEnt->destructibleBurnAmount = 0.0f;
            gfxEnt->destructibleFadeAmount = 0.0f;
            gfxEnt->wetness = 0.0f;
            gfxEnt->textureOverrideIndex = -1;
        }
        else
        {
            //LOWORD(gfxEntIndex) = 0;
            (gfxEntIndex) = 0;
        }
        sceneEntIndex = R_AllocSceneModel();
        if ( sceneEntIndex < 0x400 )
        {
            sceneModel = &scene.sceneModel[sceneEntIndex];
            sceneModel->model = model;
            if ( sceneModel->obj
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                            3194,
                            1,
                            "%s",
                            "!sceneModel->obj") )
            {
                __debugbreak();
            }
            sceneModel->gfxEntIndex = gfxEntIndex;
            memcpy(&sceneModel->placement, placement, sizeof(sceneModel->placement));
            sceneModel->entnum = gfxCfg.entnumNone;
            sceneModel->cachedLightingHandle = cachedLightingHandle;
            sceneModel->lightingOriginToleranceSq = lightToleranceSq;
            sceneModel->radius = radius;
            sceneModel->lightingOrigin[0] = sceneModel->placement.base.origin[0];
            sceneModel->lightingOrigin[1] = sceneModel->placement.base.origin[1];
            sceneModel->lightingOrigin[2] = sceneModel->placement.base.origin[2];
            sceneModel->lightingOrigin[2] = sceneModel->lightingOrigin[2] + 4.0;
            sceneModel->modelShaderConstantSetIndex = 0;
            for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
                scene.sceneModelVisData[viewIndex][sceneEntIndex] = sceneEntVisData[viewIndex];
        }
    }
}

// The decompiler emitted this same helper into r_dpvs_sceneent.cpp too; under GCC
// -fpermissive both end up external (a header forward-declares it), so mark this copy
// weak to let the linker keep one. MSVC (the Windows build) tolerates the duplicate.
#ifdef __GNUC__
__attribute__((weak))
#endif
int __cdecl R_CullSphereDpvs(const float *origin, float radius, const DpvsPlane *planes, int planeCount)
{
    int planeIndex; // [esp+8h] [ebp-4h]

    for ( planeIndex = 0; planeIndex < planeCount; ++planeIndex )
    {
        if ( (float)((float)((float)((float)((float)(planes->coeffs[0] * *origin) + (float)(planes->coeffs[1] * origin[1]))
                                                             + (float)(planes->coeffs[2] * origin[2]))
                                             + planes->coeffs[3])
                             + radius) <= 0.0 )
            return 1;
        ++planes;
    }
    return 0;
}

void __cdecl R_InitialEntityCulling()
{
    int v0; // [esp+18h] [ebp-C0h]
    int v1; // [esp+1Ch] [ebp-BCh]
    const float *minmax; // [esp+20h] [ebp-B8h]
    const DpvsPlane *v3; // [esp+28h] [ebp-B0h]
    int v4; // [esp+2Ch] [ebp-ACh]
    int v5; // [esp+30h] [ebp-A8h]
    int v6; // [esp+34h] [ebp-A4h]
    const DpvsPlane *frustumPlanes; // [esp+3Ch] [ebp-9Ch]
    int v8; // [esp+40h] [ebp-98h]
    float radius; // [esp+44h] [ebp-94h]
    float v10; // [esp+4Ch] [ebp-8Ch]
    int v11; // [esp+5Ch] [ebp-7Ch]
    int frustumPlaneCount; // [esp+60h] [ebp-78h]
    const DpvsPlane *plane; // [esp+68h] [ebp-70h]
    int v14; // [esp+6Ch] [ebp-6Ch]
    unsigned int glassIndex; // [esp+74h] [ebp-64h]
    float mins[3]; // [esp+78h] [ebp-60h] BYREF
    float maxs[3]; // [esp+84h] [ebp-54h] BYREF
    GfxEntity *v18; // [esp+90h] [ebp-48h]
    int v19; // [esp+94h] [ebp-44h]
    GfxEntity *gfxEnt; // [esp+98h] [ebp-40h]
    unsigned int gfxEntIndex; // [esp+9Ch] [ebp-3Ch]
    int sceneModelCount; // [esp+A0h] [ebp-38h]
    GfxSceneModel *sceneModel; // [esp+A4h] [ebp-34h]
    DpvsView *dpvsView; // [esp+A8h] [ebp-30h]
    unsigned __int8 *glassBrushVisData; // [esp+ACh] [ebp-2Ch]
    DpvsView *views; // [esp+B0h] [ebp-28h]
    int sceneEntIndex; // [esp+B4h] [ebp-24h]
    GfxSceneEntity *sceneEnt; // [esp+B8h] [ebp-20h]
    GfxSceneBrush *sceneBrush; // [esp+BCh] [ebp-1Ch]
    int sceneDObjCount; // [esp+C0h] [ebp-18h]
    int sceneBrushCount; // [esp+C4h] [ebp-14h]
    unsigned int glassBrushCount; // [esp+C8h] [ebp-10h]
    unsigned int entnum; // [esp+CCh] [ebp-Ch]
    unsigned int viewIndex; // [esp+D0h] [ebp-8h]
    const GfxBrushModel *bmodel; // [esp+D4h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 3227, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    views = dpvsGlob.views[scene.dpvs.localClientNum];
    sceneDObjCount = scene.sceneDObjCount;
    for ( sceneEntIndex = 0; sceneEntIndex < sceneDObjCount; ++sceneEntIndex )
    {
        sceneEnt = &scene.sceneDObj[sceneEntIndex];
        if ( sceneEnt->cull.state != 4 )
        {
            entnum = sceneEnt->entnum;
            gfxEntIndex = sceneEnt->gfxEntIndex;
            if ( gfxEntIndex && (gfxEnt = &frontEndDataOut->gfxEnts[gfxEntIndex], (gfxEnt->renderFxFlags & 0x200000) != 0) )
            {
                R_ForceAddEntitySurfacesInFrustumCmd(sceneEnt, scene.dpvs.entVisData[0]);
            }
            else
            {
                for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
                {
                    dpvsView = &views[viewIndex];
                    if ( sceneEnt->cull.state >= 2 )
                    {
                        frustumPlaneCount = dpvsView->frustumPlaneCount;
                        v14 = 0;
                        plane = dpvsView->frustumPlanes;
                        while ( v14 < frustumPlaneCount )
                        {
                            if ( R_DpvsPlaneMaxSignedDistToBox(plane, sceneEnt->cull.mins) <= 0.0 )
                            {
                                v11 = 1;
                                goto LABEL_20;
                            }
                            ++v14;
                            ++plane;
                        }
                        v11 = 0;
LABEL_20:
                        if ( v11 )
                            *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnum) = 2;
                    }
                }
                if ( r_showCullXModels->current.enabled )
                    R_AddDebugBox(&frontEndDataOut->debugGlobals, sceneEnt->cull.mins, sceneEnt->cull.maxs, colorCyan);
            }
        }
    }
    sceneModelCount = scene.sceneModelCount;
    for ( sceneEntIndex = 0; sceneEntIndex < sceneModelCount; ++sceneEntIndex )
    {
        sceneModel = &scene.sceneModel[sceneEntIndex];
        entnum = sceneModel->entnum;
        v19 = sceneModel->gfxEntIndex;
        if ( v19 && (v18 = &frontEndDataOut->gfxEnts[v19], (v18->renderFxFlags & 0x200000) != 0) )
        {
            scene.dpvs.entVisData[0][entnum] = 1;
        }
        else
        {
            for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
            {
                dpvsView = &views[viewIndex];
                if ( R_CullSphereDpvs(
                             sceneModel->placement.base.origin,
                             sceneModel->radius,
                             dpvsView->frustumPlanes,
                             dpvsView->frustumPlaneCount) )
                {
                    *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnum) = 2;
                }
            }
            if ( r_showCullXModels->current.enabled )
            {
                v10 = -sceneModel->radius;
                mins[0] = sceneModel->placement.base.origin[0] + v10;
                mins[1] = sceneModel->placement.base.origin[1] + v10;
                mins[2] = sceneModel->placement.base.origin[2] + v10;
                radius = sceneModel->radius;
                maxs[0] = sceneModel->placement.base.origin[0] + radius;
                maxs[1] = sceneModel->placement.base.origin[1] + radius;
                maxs[2] = sceneModel->placement.base.origin[2] + radius;
                R_AddDebugBox(&frontEndDataOut->debugGlobals, mins, maxs, colorCyan);
            }
        }
    }
    sceneBrushCount = scene.sceneBrushCount;
    for ( sceneEntIndex = 0; sceneEntIndex < sceneBrushCount; ++sceneEntIndex )
    {
        sceneBrush = &scene.sceneBrush[sceneEntIndex];
        entnum = sceneBrush->entnum;
        bmodel = sceneBrush->bmodel;
        for ( viewIndex = 0; viewIndex < 3; ++viewIndex )
        {
            dpvsView = &views[viewIndex];
            v6 = dpvsView->frustumPlaneCount;
            v8 = 0;
            frustumPlanes = dpvsView->frustumPlanes;
            while ( v8 < v6 )
            {
                if ( R_DpvsPlaneMaxSignedDistToBox(frustumPlanes, bmodel->writable.mins) <= 0.0 )
                {
                    v5 = 1;
                    goto LABEL_51;
                }
                ++v8;
                ++frustumPlanes;
            }
            v5 = 0;
LABEL_51:
            if ( v5 )
                *(_BYTE *)(scene.dynSModelVisBitsCamera[viewIndex - 13] + entnum) = 2;
        }
    }
    glassBrushCount = scene.glassBrushCount;
    glassBrushVisData = scene.glassBrushVisData;
    for ( glassIndex = 0; glassIndex < glassBrushCount; ++glassIndex )
    {
        v1 = views->frustumPlaneCount;
        //minmax = *(float **)&scene.glassBrushVisData[40 * glassIndex - 40932];
        minmax = scene.glassBrush[glassIndex].bmodel->writable.mins;
        v4 = 0;
        v3 = views->frustumPlanes;
        while ( v4 < v1 )
        {
            if ( R_DpvsPlaneMaxSignedDistToBox(v3, minmax) <= 0.0 )
            {
                v0 = 1;
                goto LABEL_63;
            }
            ++v4;
            ++v3;
        }
        v0 = 0;
LABEL_63:
        if ( v0 )
            glassBrushVisData[glassIndex] = 2;
        else
            glassBrushVisData[glassIndex] = 1;
    }
}

float __cdecl R_DpvsPlaneMaxSignedDistToBox(const DpvsPlane *plane, const float *minmax)
{
    float x = *(const float *)((const char *)minmax + plane->side[0]);
    float y = *(const float *)((const char *)minmax + plane->side[1]);
    float z = *(const float *)((const char *)minmax + plane->side[2]);

    return x * plane->coeffs[0] +
           y * plane->coeffs[1] +
           z * plane->coeffs[2] +
           plane->coeffs[3];
}

void __cdecl R_GetStaticModels(const float *mins, float *maxs, int *models, int *models_count, int max_models)
{
    float *v6; // [esp+8h] [ebp-Ch]
    GfxCell *cell; // [esp+Ch] [ebp-8h]
    int ci; // [esp+10h] [ebp-4h]

    *models_count = 0;
    if ( rgp.world )
    {
        for ( ci = 0; ci < rgp.world->dpvsPlanes.cellCount; ++ci )
        {
            cell = &rgp.world->cells[ci];
            if ( cell->aabbTree )
            {
                v6 = cell->aabbTree->mins;
                if ( *maxs >= *v6
                    && maxs[1] >= v6[1]
                    && maxs[2] >= v6[2]
                    && cell->aabbTree->maxs[0] >= *mins
                    && cell->aabbTree->maxs[1] >= mins[1]
                    && cell->aabbTree->maxs[2] >= mins[2] )
                {
                    R_GetStaticModelsAabb(cell->aabbTree, mins, maxs, models, models_count, max_models);
                }
            }
        }
    }
    else
    {
        *models_count = 0;
    }
}

void __cdecl R_GetStaticModelsAabb(
                const GfxAabbTree *tree,
                const float *mins,
                const float *maxs,
                int *models,
                int *models_count,
                int max_models)
{
    bool v6; // [esp+0h] [ebp-28h]
    const GfxStaticModelInst *inst; // [esp+10h] [ebp-18h]
    int model_index; // [esp+14h] [ebp-14h]
    int smodelChildIndex; // [esp+18h] [ebp-10h]
    const GfxAabbTree *child; // [esp+1Ch] [ebp-Ch]
    int ci; // [esp+20h] [ebp-8h]
    GfxAabbTree *children; // [esp+24h] [ebp-4h]

    if ( tree->childCount )
    {
        children = (GfxAabbTree *)((char *)tree + tree->childrenOffset);
        for ( ci = 0; ci < tree->childCount; ++ci )
        {
            child = &children[ci];
            if ( *maxs >= child->mins[0]
                && maxs[1] >= child->mins[1]
                && maxs[2] >= child->mins[2]
                && children[ci].maxs[0] >= *mins
                && children[ci].maxs[1] >= mins[1]
                && children[ci].maxs[2] >= mins[2] )
            {
                R_GetStaticModelsAabb(child, mins, maxs, models, models_count, max_models);
            }
        }
    }
    else
    {
        for ( smodelChildIndex = 0; smodelChildIndex < tree->smodelIndexCount; ++smodelChildIndex )
        {
            model_index = tree->smodelIndexes[smodelChildIndex];
            inst = &rgp.world->dpvs.smodelInsts[model_index];
            v6 = inst->maxs[0] >= *mins
                && inst->maxs[1] >= mins[1]
                && inst->maxs[2] >= mins[2]
                && *maxs >= inst->mins[0]
                && maxs[1] >= inst->mins[1]
                && maxs[2] >= inst->mins[2];
            if ( v6 && *models_count < max_models )
                models[(*models_count)++] = model_index;
        }
    }
}

void __cdecl R_AddCellDynBrushSurfacesInFrustumCmd(DpvsPlane **data)
{
    unsigned int oldViewIndex; // [esp+0h] [ebp-8h]

    oldViewIndex = R_SetVisData(*((unsigned __int16 *)data + 4));
    if ( r_drawDynEnts->current.enabled )
        R_CullDynBrushInCell((unsigned int)data[1], *data, *((unsigned __int8 *)data + 10));
    R_SetVisData(oldViewIndex);
}

void __cdecl R_CullDynBrushInCell(unsigned int cellIndex, DpvsPlane *planes, int planeCount)
{
    DWORD v4; // eax
    int v5; // [esp+4h] [ebp-38h]
    DpvsPlane *plane; // [esp+Ch] [ebp-30h]
    int v7; // [esp+10h] [ebp-2Ch]
    unsigned int dynEntIndex; // [esp+18h] [ebp-24h]
    const DynEntityDef *dynEntDef; // [esp+1Ch] [ebp-20h]
    unsigned int bits; // [esp+20h] [ebp-1Ch]
    unsigned int dynEntClientWordCount; // [esp+24h] [ebp-18h]
    unsigned int indexLow; // [esp+28h] [ebp-14h]
    unsigned __int8 *dynEntVisData; // [esp+2Ch] [ebp-10h]
    unsigned int *dynEntCellBits; // [esp+30h] [ebp-Ch]
    unsigned int wordIndex; // [esp+34h] [ebp-8h]
    const GfxBrushModel *bmodel; // [esp+38h] [ebp-4h]

    if ( cellIndex >= rgp.world->dpvsPlanes.cellCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    2194,
                    0,
                    "cellIndex doesn't index rgp.world->dpvsPlanes.cellCount\n\t%i not in [0, %i)",
                    cellIndex,
                    rgp.world->dpvsPlanes.cellCount) )
    {
        __debugbreak();
    }
    //dynEntVisData = *(unsigned __int8 **)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 8332);
    dynEntVisData = g_dynEntVisData[1];
    dynEntClientWordCount = rgp.world->dpvsDyn.dynEntClientWordCount[1];
    dynEntCellBits = &rgp.world->dpvsDyn.dynEntCellBits[1][dynEntClientWordCount * cellIndex];
    for ( wordIndex = 0; wordIndex < dynEntClientWordCount; ++wordIndex )
    {
        bits = dynEntCellBits[wordIndex];
        while ( 1 )
        {
            if (!_BitScanReverse(&v4, bits))
                v4 = 0x3F;
            indexLow = v4 ^ 0x1F;
            if ((v4 ^ 0x1Fu) >= 0x20)
                break;
            dynEntIndex = indexLow + 32 * wordIndex;
            if ( ((0x80000000 >> indexLow) & bits) == 0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 2213, 0, "%s", "bits & bit") )
            {
                __debugbreak();
            }
            bits &= ~(0x80000000 >> indexLow);
            if ( !dynEntVisData[dynEntIndex] )
            {
                dynEntDef = DynEnt_GetEntityDef(dynEntIndex, DYNENT_DRAW_BRUSH);
                if ( dynEntDef->xModel
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                                2222,
                                0,
                                "%s",
                                "!dynEntDef->xModel") )
                {
                    __debugbreak();
                }
                if ( !dynEntDef->brushModel
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                                2223,
                                0,
                                "%s",
                                "dynEntDef->brushModel") )
                {
                    __debugbreak();
                }
                bmodel = R_GetBrushModel(dynEntDef->brushModel);
                v7 = 0;
                plane = planes;
                while ( v7 < planeCount )
                {
                    if ( R_DpvsPlaneMaxSignedDistToBox(plane, bmodel->writable.mins) <= 0.0 )
                    {
                        v5 = 1;
                        goto LABEL_26;
                    }
                    ++v7;
                    ++plane;
                }
                v5 = 0;
LABEL_26:
                if ( !v5 )
                    dynEntVisData[dynEntIndex] = 1;
            }
        }
    }
}

unsigned int __cdecl R_SetVisData(unsigned int viewIndex)
{
    unsigned int oldViewIndex; // [esp+4h] [ebp-8h]
    unsigned int drawType; // [esp+8h] [ebp-4h]

    oldViewIndex = g_viewIndex;
    g_viewIndex = viewIndex;
    for (drawType = 0; drawType < 2; ++drawType)
        g_dynEntVisData[drawType] = rgp.world->dpvsDyn.dynEntVisData[drawType][viewIndex];
    g_dpvsView = &dpvsGlob.views[scene.dpvs.localClientNum][viewIndex];
    return oldViewIndex;
}

void __cdecl R_GenerateShadowMapCasterCells()
{
    GfxLight *sunLight; // edx
    float *dir; // [esp+4h] [ebp-14h]
    GfxCell *cell; // [esp+Ch] [ebp-Ch]
    int cellIndex; // [esp+10h] [ebp-8h]
    unsigned int cellCasterBitsCount; // [esp+14h] [ebp-4h]

    if ( !rgp.world->sunLight
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 4541, 0, "%s", "rgp.world->sunLight") )
    {
        __debugbreak();
    }
    cellCasterBitsCount = (rgp.world->dpvsPlanes.cellCount + 31) >> 5;
    memset((unsigned __int8 *)rgp.world->cellCasterBits, 0, 4 * cellCasterBitsCount * rgp.world->dpvsPlanes.cellCount);
    if ( rgp.world->sunPrimaryLightIndex )
    {
        if ( (float)((float)((float)(rgp.world->sunLight->dir[0] * rgp.world->sunLight->dir[0])
                                             + (float)(rgp.world->sunLight->dir[1] * rgp.world->sunLight->dir[1]))
                             + (float)(rgp.world->sunLight->dir[2] * rgp.world->sunLight->dir[2])) == 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                        4551,
                        0,
                        "%s",
                        "Vec3LengthSq( rgp.world->sunLight->dir )") )
        {
            __debugbreak();
        }
        dir = rgp.world->sunLight->dir;
        dpvsGlob.viewOrg[0] = -dir[0];
        dpvsGlob.viewOrg[1] = -dir[1];
        dpvsGlob.viewOrg[2] = -dir[2];
        dpvsGlob.viewOrg[3] = 0.0f;
        dpvsGlob.viewOrgIsDir = 1;
        sunLight = rgp.world->sunLight;
        dpvsGlob.nearPlane.coeffs[0] = sunLight->dir[0];
        dpvsGlob.nearPlane.coeffs[1] = sunLight->dir[1];
        dpvsGlob.nearPlane.coeffs[2] = sunLight->dir[2];
        dpvsGlob.nearPlane.coeffs[3] = 0.0f;
        dpvsGlob.farPlaneEnabled = 0;
        for ( cellIndex = 0; cellIndex < rgp.world->dpvsPlanes.cellCount; ++cellIndex )
        {
            cell = &rgp.world->cells[cellIndex];
            dpvsGlob.cellCasterBitsForCell = &rgp.world->cellCasterBits[cellCasterBitsCount * cellIndex];
            R_VisitPortalsNoFrustum(cell);
        }
    }
}

void __cdecl R_VisitPortalsNoFrustum(const GfxCell *cell)
{
    float *v1; // [esp+0h] [ebp-D58h]
    float *v2; // [esp+4h] [ebp-D54h]
    float v3; // [esp+8h] [ebp-D50h]
    float *v4; // [esp+10h] [ebp-D48h]
    float *v5; // [esp+14h] [ebp-D44h]
    float v6; // [esp+18h] [ebp-D40h]
    float v7; // [esp+20h] [ebp-D38h]
    LargeLocal hullPointsPoolArray_large_local(0x20000); // [esp+28h] [ebp-D30h] BYREF
    GfxHullPointsPool (*hullPointsPoolArray)[256]; // [esp+30h] [ebp-D28h]
    int childPlaneCount; // [esp+34h] [ebp-D24h]
    GfxPortal *portal; // [esp+38h] [ebp-D20h]
    int queueIndex; // [esp+3Ch] [ebp-D1Ch]
    float portalVerts[64][3]; // [esp+40h] [ebp-D18h] BYREF
    float hullOrigin[3]; // [esp+340h] [ebp-A18h]
    unsigned int vertIndex; // [esp+34Ch] [ebp-A0Ch]
    PortalHeapNode portalQueue[256]; // [esp+350h] [ebp-A08h] BYREF
    float hull[64][2]; // [esp+B50h] [ebp-208h] BYREF
    unsigned int hullPointCount; // [esp+D54h] [ebp-4h]

    //LargeLocal::LargeLocal(&hullPointsPoolArray_large_local, 0x20000);
    hullPointsPoolArray = (GfxHullPointsPool(*)[256])hullPointsPoolArray_large_local.GetBuf();// (GfxHullPointsPool(*)[256])LargeLocal::GetBuf(&hullPointsPoolArray_large_local);
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 4465, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    for ( queueIndex = 0; queueIndex < 255; ++queueIndex )
        (*hullPointsPoolArray)[queueIndex].nextFree = &(*hullPointsPoolArray)[queueIndex + 1];
    (*hullPointsPoolArray)[queueIndex].nextFree = 0;
    dpvsGlob.nextFreeHullPoints = (GfxHullPointsPool *)hullPointsPoolArray;
    dpvsGlob.portalQueue = portalQueue;
    dpvsGlob.queuedCount = 0;
    R_VisitPortalsForCellNoFrustum(cell, 0, 0, 0, 0, 0);
    while ( dpvsGlob.queuedCount )
    {
        portal = R_NextQueuedPortal();
        if ( !portal && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 4486, 0, "%s", "portal") )
            __debugbreak();
        hullPointCount = Com_ConvexHull(
                                             portal->writable.hullPoints,
                                             portal->writable.hullPointCount,
                                             hull);
        R_FreeHullPoints((GfxHullPointsPool *)portal->writable.hullPoints);
        portal->writable.hullPoints = 0;
        if ( hullPointCount )
        {
            v7 = -portal->plane.coeffs[3];
            hullOrigin[0] = v7 * portal->plane.coeffs[0];
            hullOrigin[1] = v7 * portal->plane.coeffs[1];
            hullOrigin[2] = v7 * portal->plane.coeffs[2];
            for ( vertIndex = 0; vertIndex < hullPointCount; ++vertIndex )
            {
                v4 = portalVerts[vertIndex];
                v5 = portal->hullAxis[0];
                v6 = hull[vertIndex][0];
                *v4 = (float)(v6 * portal->hullAxis[0][0]) + hullOrigin[0];
                v4[1] = (float)(v6 * v5[1]) + hullOrigin[1];
                v4[2] = (float)(v6 * v5[2]) + hullOrigin[2];
                v1 = portalVerts[vertIndex];
                v2 = portal->hullAxis[1];
                v3 = hull[vertIndex][1];
                *v1 = (float)(v3 * portal->hullAxis[1][0]) + *v1;
                v1[1] = (float)(v3 * v2[1]) + v1[1];
                v1[2] = (float)(v3 * v2[2]) + v1[2];
            }
            childPlaneCount = R_PortalClipPlanesNoFrustum(dpvsGlob.childPlanes, hullPointCount, portalVerts);
            if ( childPlaneCount > 16
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                            4519,
                            0,
                            "%s",
                            "childPlaneCount <= DPVS_PORTAL_MAX_PLANES") )
            {
                __debugbreak();
            }
            R_VisitPortalsForCellNoFrustum(
                portal->cell,
                portal,
                &portal->plane,
                dpvsGlob.childPlanes,
                childPlaneCount,
                portal->writable.recursionDepth + 1);
        }
    }
    //LargeLocal::~LargeLocal(&hullPointsPoolArray_large_local);
}

unsigned int __cdecl R_PortalClipPlanesNoFrustum(
                DpvsPlane *planes,
                unsigned int vertexCount,
                const float (*winding)[3])
{
    DpvsPlane *plane; // [esp+0h] [ebp-628h]
    DpvsPlane *v5; // [esp+Ch] [ebp-61Ch]
    float *v6; // [esp+10h] [ebp-618h]
    float normals[128][3]; // [esp+18h] [ebp-610h] BYREF
    unsigned int windingVertIndex; // [esp+618h] [ebp-10h]
    unsigned int planeCount; // [esp+61Ch] [ebp-Ch]
    unsigned int stepbump; // [esp+620h] [ebp-8h]
    unsigned int step; // [esp+624h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 839, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( vertexCount < 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    840,
                    0,
                    "%s\n\t(vertexCount) = %i",
                    "(vertexCount >= 3)",
                    vertexCount) )
    {
        __debugbreak();
    }
    planeCount = 0;
    if ( vertexCount > 0x10 )
    {
        step = vertexCount >> 4;
        stepbump = ((vertexCount >> 4) + 1) * (vertexCount % 0x10);
    }
    else
    {
        step = 1;
        stepbump = 0;
    }
    R_GetSidePlaneNormals(winding, vertexCount, normals);
    for ( windingVertIndex = 0; windingVertIndex < vertexCount; windingVertIndex += step + (windingVertIndex < stepbump) )
    {
        if ( (float)((float)((float)(normals[windingVertIndex][0] * normals[windingVertIndex][0])
                                             + (float)(normals[windingVertIndex][1] * normals[windingVertIndex][1]))
                             + (float)(normals[windingVertIndex][2] * normals[windingVertIndex][2])) != 0.0 )
        {
            v5 = &planes[planeCount];
            v6 = normals[windingVertIndex];
            v5->coeffs[0] = *v6;
            v5->coeffs[1] = v6[1];
            v5->coeffs[2] = v6[2];
            plane = &planes[planeCount];
            plane->coeffs[3] = 0.001
                                             - (float)((float)((float)(plane->coeffs[0] * (float)(*winding)[3 * windingVertIndex])
                                                                             + (float)(plane->coeffs[1] * (float)(*winding)[3 * windingVertIndex + 1]))
                                                             + (float)(plane->coeffs[2] * (float)(*winding)[3 * windingVertIndex + 2]));
            R_SetDpvsPlaneSides(plane);
            ++planeCount;
        }
    }
    return planeCount;
}

void __cdecl R_GetSidePlaneNormals(const float (*winding)[3], unsigned int vertexCount, float (*normals)[3])
{
    float *v3; // [esp+24h] [ebp-620h]
    float *v4; // [esp+28h] [ebp-61Ch]
    float delta[388]; // [esp+2Ch] [ebp-618h] BYREF
    unsigned int vertexIndex; // [esp+63Ch] [ebp-8h]
    unsigned int vertexIndexNext; // [esp+640h] [ebp-4h]

    if ( vertexCount >= 0x81
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    360,
                    0,
                    "%s",
                    "vertexCount < ARRAY_COUNT( delta )") )
    {
        __debugbreak();
    }
    if ( dpvsGlob.viewOrgIsDir )
    {
        vertexIndex = vertexCount - 1;
        for ( vertexIndexNext = 0; vertexIndexNext < vertexCount; ++vertexIndexNext )
        {
            delta[0] = (float)(*winding)[3 * vertexIndexNext] - (float)(*winding)[3 * vertexIndex];
            delta[1] = (float)(*winding)[3 * vertexIndexNext + 1] - (float)(*winding)[3 * vertexIndex + 1];
            delta[2] = (float)(*winding)[3 * vertexIndexNext + 2] - (float)(*winding)[3 * vertexIndex + 2];
            Vec3Cross(dpvsGlob.viewOrg, delta, &(*normals)[3 * vertexIndex]);
            Vec3Normalize(&(*normals)[3 * vertexIndex]);
            vertexIndex = vertexIndexNext;
        }
    }
    else
    {
        for ( vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex )
        {
            v3 = &delta[3 * vertexIndex];
            v4 = (float *)&(*winding)[3 * vertexIndex];
            *v3 = *v4 - dpvsGlob.viewOrg[0];
            v3[1] = v4[1] - dpvsGlob.viewOrg[1];
            v3[2] = v4[2] - dpvsGlob.viewOrg[2];
        }
        *(_QWORD *)&delta[3 * vertexCount] = *(_QWORD *)delta;
        delta[3 * vertexCount + 2] = delta[2];
        for ( vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex )
        {
            Vec3Cross(&delta[3 * vertexIndex + 3], &delta[3 * vertexIndex], &(*normals)[3 * vertexIndex]);
            Vec3Normalize(&(*normals)[3 * vertexIndex]);
        }
    }
}

GfxPortal *R_NextQueuedPortal()
{
    float dist; // eax
    PortalHeapNode *portalQueue; // esi
    float v2; // eax
    PortalHeapNode *v3; // esi
    int heapIndex; // [esp+4h] [ebp-Ch]
    int chosenChildIndex; // [esp+8h] [ebp-8h]
    GfxPortal *portal; // [esp+Ch] [ebp-4h]

    if ( dpvsGlob.queuedCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    3673,
                    0,
                    "%s\n\t(dpvsGlob.queuedCount) = %i",
                    "(dpvsGlob.queuedCount > 0)",
                    dpvsGlob.queuedCount) )
    {
        __debugbreak();
    }
    portal = dpvsGlob.portalQueue->portal;
    dpvsGlob.portalQueue->portal->writable.isQueued = 0;
    --dpvsGlob.queuedCount;
    for ( heapIndex = 0; ; heapIndex = chosenChildIndex )
    {
        chosenChildIndex = 2 * heapIndex + 1;
        if ( chosenChildIndex > dpvsGlob.queuedCount )
            break;
        if ( chosenChildIndex < dpvsGlob.queuedCount
            && dpvsGlob.portalQueue[chosenChildIndex].dist > dpvsGlob.portalQueue[chosenChildIndex + 1].dist )
        {
            chosenChildIndex = 2 * heapIndex + 2;
        }
        if ( dpvsGlob.portalQueue[chosenChildIndex].dist >= dpvsGlob.portalQueue[dpvsGlob.queuedCount].dist )
            break;
        dist = dpvsGlob.portalQueue[chosenChildIndex].dist;
        portalQueue = dpvsGlob.portalQueue;
        dpvsGlob.portalQueue[heapIndex].portal = dpvsGlob.portalQueue[chosenChildIndex].portal;
        portalQueue[heapIndex].dist = dist;
    }
    v2 = dpvsGlob.portalQueue[dpvsGlob.queuedCount].dist;
    v3 = dpvsGlob.portalQueue;
    dpvsGlob.portalQueue[heapIndex].portal = dpvsGlob.portalQueue[dpvsGlob.queuedCount].portal;
    v3[heapIndex].dist = v2;
    R_AssertValidQueue();
    return portal;
}

void R_AssertValidQueue()
{
    int queueIndex; // [esp+8h] [ebp-4h]

    for ( queueIndex = 1; queueIndex < dpvsGlob.queuedCount; ++queueIndex )
    {
        if ( dpvsGlob.portalQueue[queueIndex].dist < dpvsGlob.portalQueue[(queueIndex - 1) >> 1].dist
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                        3601,
                        0,
                        "%s",
                        "dpvsGlob.portalQueue[parentIndex].dist <= dpvsGlob.portalQueue[queueIndex].dist") )
        {
            __debugbreak();
        }
    }
}

void __cdecl R_FreeHullPoints(GfxHullPointsPool *hullPoints)
{
    hullPoints->nextFree = dpvsGlob.nextFreeHullPoints;
    dpvsGlob.nextFreeHullPoints = hullPoints;
}

void __cdecl R_VisitPortalsForCellNoFrustum(
                const GfxCell *cell,
                GfxPortal *parentPortal,
                const DpvsPlane *parentPlane,
                const DpvsPlane *planes,
                int planeCount,
                signed int recursionDepth)
{
    unsigned __int8 v6; // [esp+0h] [ebp-14h]
    GfxPortal *portal; // [esp+Ch] [ebp-8h]
    int portalIndex; // [esp+10h] [ebp-4h]

    if ( !dpvsGlob.cellCasterBitsForCell
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    4387,
                    0,
                    "%s",
                    "dpvsGlob.cellCasterBitsForCell") )
    {
        __debugbreak();
    }
    dpvsGlob.cellCasterBitsForCell[(unsigned int)(cell - rgp.world->cells) >> 5] |= 1 << ((cell - rgp.world->cells) & 0x1F);
    R_SetAncestorListStatus(parentPortal, 1);
    for ( portalIndex = 0; portalIndex < cell->portalCount; ++portalIndex )
    {
        portal = &cell->portals[portalIndex];
        if ( !R_ShouldSkipPortal(portal, planes, planeCount)
            && R_ChopPortalAndAddHullPointsNoFrustum(portal, parentPlane, planes, planeCount) )
        {
            if ( portal->writable.isQueued )
            {
                if ( portal->writable.recursionDepth < recursionDepth )
                    v6 = portal->writable.recursionDepth;
                else
                    v6 = recursionDepth;
                portal->writable.recursionDepth = v6;
                if ( portal->writable.queuedParent != parentPortal )
                    portal->writable.queuedParent = 0;
            }
            else
            {
                portal->writable.recursionDepth = recursionDepth;
                portal->writable.queuedParent = parentPortal;
                R_EnqueuePortal(portal);
            }
        }
    }
    R_SetAncestorListStatus(parentPortal, 0);
}

void __cdecl R_EnqueuePortal(GfxPortal *portal)
{
    float v1; // ecx
    PortalHeapNode *portalQueue; // esi
    int heapIndex; // [esp+4h] [ebp-Ch]
    float dist; // [esp+8h] [ebp-8h]
    int parentIndex; // [esp+Ch] [ebp-4h]

    if ( !portal && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 3618, 0, "%s", "portal") )
        __debugbreak();
    if ( portal->writable.isQueued
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    3619,
                    0,
                    "%s",
                    "!portal->writable.isQueued") )
    {
        __debugbreak();
    }
    if ( !portal->writable.hullPoints
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    3620,
                    0,
                    "%s",
                    "portal->writable.hullPoints") )
    {
        __debugbreak();
    }
    if ( portal->writable.hullPointCount < 3u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    3621,
                    0,
                    "%s",
                    "portal->writable.hullPointCount >= 3") )
    {
        __debugbreak();
    }
    if ( dpvsGlob.queuedCount >= 256 )
        Com_Error(ERR_DROP, "More than %i queued portals", 256);
    portal->writable.isQueued = 1;
    dist = R_FurthestPointOnWinding(portal->vertices, portal->vertexCount, &dpvsGlob.nearPlane);
    for ( heapIndex = dpvsGlob.queuedCount; ; heapIndex = (heapIndex - 1) >> 1 )
    {
        parentIndex = (heapIndex - 1) >> 1;
        if ( parentIndex < 0 || dist >= dpvsGlob.portalQueue[parentIndex].dist )
            break;
        v1 = dpvsGlob.portalQueue[parentIndex].dist;
        portalQueue = dpvsGlob.portalQueue;
        dpvsGlob.portalQueue[heapIndex].portal = dpvsGlob.portalQueue[parentIndex].portal;
        portalQueue[heapIndex].dist = v1;
    }
    if ( (heapIndex < 0 || heapIndex > dpvsGlob.queuedCount)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    3652,
                    1,
                    "%s\n\t(heapIndex) = %i",
                    "(heapIndex >= 0 && heapIndex <= dpvsGlob.queuedCount)",
                    heapIndex) )
    {
        __debugbreak();
    }
    dpvsGlob.portalQueue[heapIndex].portal = portal;
    dpvsGlob.portalQueue[heapIndex].dist = dist;
    ++dpvsGlob.queuedCount;
    R_AssertValidQueue();
}

double __cdecl R_FurthestPointOnWinding(const float (*points)[3], int pointCount, const DpvsPlane *plane)
{
    int pointIndex; // [esp+24h] [ebp-Ch]
    int pointIndexa; // [esp+24h] [ebp-Ch]
    float distMax; // [esp+2Ch] [ebp-4h]

    if ( (float)((float)((float)((float)(plane->coeffs[0] * (*points)[0]) + (float)(plane->coeffs[1] * (float)(*points)[1]))
                                         + (float)(plane->coeffs[2] * (float)(*points)[2]))
                         + plane->coeffs[3]) <= (float)((float)((float)((float)(plane->coeffs[0]
                                                                                                                                    * (float)(*points)[3 * pointCount - 3])
                                                                                                                    + (float)(plane->coeffs[1]
                                                                                                                                    * (float)(*points)[3 * pointCount - 2]))
                                                                                                    + (float)(plane->coeffs[2] * (float)(*points)[3 * pointCount - 1]))
                                                                                    + plane->coeffs[3]) )
    {
        distMax = (float)((float)((float)(plane->coeffs[0] * (float)(*points)[3 * pointCount - 3])
                                                        + (float)(plane->coeffs[1] * (float)(*points)[3 * pointCount - 2]))
                                        + (float)(plane->coeffs[2] * (float)(*points)[3 * pointCount - 1]))
                        + plane->coeffs[3];
        for ( pointIndexa = pointCount - 2;
                    pointIndexa > 0
             && distMax <= (float)((float)((float)((float)(plane->coeffs[0] * (float)(*points)[3 * pointIndexa])
                                                                                     + (float)(plane->coeffs[1] * (float)(*points)[3 * pointIndexa + 1]))
                                                                     + (float)(plane->coeffs[2] * (float)(*points)[3 * pointIndexa + 2]))
                                                     + plane->coeffs[3]);
                    --pointIndexa )
        {
            distMax = (float)((float)((float)(plane->coeffs[0] * (float)(*points)[3 * pointIndexa])
                                                            + (float)(plane->coeffs[1] * (float)(*points)[3 * pointIndexa + 1]))
                                            + (float)(plane->coeffs[2] * (float)(*points)[3 * pointIndexa + 2]))
                            + plane->coeffs[3];
        }
    }
    else
    {
        distMax = (float)((float)((float)(plane->coeffs[0] * (*points)[0]) + (float)(plane->coeffs[1] * (float)(*points)[1]))
                                        + (float)(plane->coeffs[2] * (float)(*points)[2]))
                        + plane->coeffs[3];
        for ( pointIndex = 1;
                    pointIndex < pointCount - 1
             && distMax <= (float)((float)((float)((float)(plane->coeffs[0] * (float)(*points)[3 * pointIndex])
                                                                                     + (float)(plane->coeffs[1] * (float)(*points)[3 * pointIndex + 1]))
                                                                     + (float)(plane->coeffs[2] * (float)(*points)[3 * pointIndex + 2]))
                                                     + plane->coeffs[3]);
                    ++pointIndex )
        {
            distMax = (float)((float)((float)(plane->coeffs[0] * (float)(*points)[3 * pointIndex])
                                                            + (float)(plane->coeffs[1] * (float)(*points)[3 * pointIndex + 1]))
                                            + (float)(plane->coeffs[2] * (float)(*points)[3 * pointIndex + 2]))
                            + plane->coeffs[3];
        }
    }
    return distMax;
}

bool __cdecl R_ShouldSkipPortal(const GfxPortal *portal, const DpvsPlane *planes, int planeCount)
{
    if ( portal->writable.isAncestor )
        return 1;
    if ( R_DpvsPlaneDistToEye(&portal->plane) <= 0.0 )
        return R_PortalBehindAnyPlane(portal, planes, planeCount) != 0;
    return 1;
}

double __cdecl R_DpvsPlaneDistToEye(const DpvsPlane *plane)
{
    return (float)((float)((float)((float)(plane->coeffs[0] * dpvsGlob.viewOrg[0])
                                                             + (float)(plane->coeffs[1] * dpvsGlob.viewOrg[1]))
                                             + (float)(plane->coeffs[2] * dpvsGlob.viewOrg[2]))
                             + (float)(plane->coeffs[3] * dpvsGlob.viewOrg[3]));
}

char __cdecl R_PortalBehindAnyPlane(const GfxPortal *portal, const DpvsPlane *planes, int planeCount)
{
    while ( planeCount )
    {
        if ( R_PortalBehindPlane(portal, planes) )
            return 1;
        --planeCount;
        ++planes;
    }
    return 0;
}

char __cdecl R_PortalBehindPlane(const GfxPortal *portal, const DpvsPlane *plane)
{
    int c; // [esp+4h] [ebp-8h]
    float *v; // [esp+8h] [ebp-4h]

    v = (float *)portal->vertices;
    for ( c = portal->vertexCount; c; --c )
    {
        if ( (float)((float)((float)((float)(plane->coeffs[0] * *v) + (float)(plane->coeffs[1] * v[1]))
                                             + (float)(plane->coeffs[2] * v[2]))
                             + plane->coeffs[3]) > 0.0 )
            return 0;
        v += 3;
    }
    return 1;
}

char __cdecl R_ChopPortalAndAddHullPointsNoFrustum(
                GfxPortal *portal,
                const DpvsPlane *parentPlane,
                const DpvsPlane *planes,
                int planeCount)
{
    int vertCount; // [esp+0h] [ebp-C10h]
    int vertIndex; // [esp+4h] [ebp-C0Ch]
    float v[2][128][3]; // [esp+8h] [ebp-C08h] BYREF
    const float (*w)[3]; // [esp+C0Ch] [ebp-4h] BYREF

    if ( parentPlane )
    {
        vertCount = R_ChopPortal(portal, parentPlane, planes, planeCount, v, &w);
        if ( !vertCount )
            return 0;
    }
    else
    {
        vertCount = portal->vertexCount;
        if ( !portal->vertexCount
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 3923, 0, "%s", "vertCount") )
        {
            __debugbreak();
        }
        w = portal->vertices;
    }
    for ( vertIndex = 0; vertIndex < vertCount; ++vertIndex )
        R_AddVertToPortalHullPoints(portal, w[vertIndex]);
    return 1;
}

void __cdecl R_AddVertToPortalHullPoints(GfxPortal *portal, const float *v)
{
    float hull[64][2]; // [esp+Ch] [ebp-208h] BYREF
    int hullPointCount; // [esp+210h] [ebp-4h]

    if ( portal->writable.hullPoints )
    {
        if ( portal->writable.hullPointCount == 64 )
        {
            hullPointCount = Com_ConvexHull(
                                                 portal->writable.hullPoints,
                                                 portal->writable.hullPointCount,
                                                 hull);
            if ( hullPointCount == 64 )
                Com_Error(ERR_DROP, "More than %i points on a clipped portal's convex hull\n", 64);
            portal->writable.hullPointCount = hullPointCount;
            memcpy(
                (unsigned __int8 *)portal->writable.hullPoints,
                (unsigned __int8 *)hull,
                8 * portal->writable.hullPointCount);
        }
    }
    else
    {
        portal->writable.hullPoints = (float (*)[2])R_AllocHullPoints();
        portal->writable.hullPointCount = 0;
    }
    portal->writable.hullPoints[portal->writable.hullPointCount][0] = (float)((float)(*v * portal->hullAxis[0][0])
                                                                                                                                                    + (float)(v[1] * portal->hullAxis[0][1]))
                                                                                                                                    + (float)(v[2] * portal->hullAxis[0][2]);
    portal->writable.hullPoints[portal->writable.hullPointCount++][1] = (float)((float)(*v * portal->hullAxis[1][0])
                                                                                                                                                        + (float)(v[1] * portal->hullAxis[1][1]))
                                                                                                                                        + (float)(v[2] * portal->hullAxis[1][2]);
}

GfxHullPointsPool *__cdecl R_AllocHullPoints()
{
    GfxHullPointsPool *hullPointsPool; // [esp+0h] [ebp-4h]

    hullPointsPool = dpvsGlob.nextFreeHullPoints;
    if ( !dpvsGlob.nextFreeHullPoints )
        Com_Error(ERR_DROP, "more than %i queued portals", 256);
    dpvsGlob.nextFreeHullPoints = hullPointsPool->nextFree;
    return hullPointsPool;
}

int __cdecl R_ChopPortal(
                const GfxPortal *portal,
                const DpvsPlane *parentPlane,
                const DpvsPlane *planes,
                int planeCount,
                float (*v)[128][3],
                const float (**finalVerts)[3])
{
    int vertCount; // [esp+0h] [ebp-Ch] BYREF
    int planeIndex; // [esp+4h] [ebp-8h]
    const float (*w)[3]; // [esp+8h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 3832, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !portal && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 3833, 0, "%s", "portal") )
        __debugbreak();
    if ( !parentPlane
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 3834, 0, "%s", "parentPlane") )
    {
        __debugbreak();
    }
    if ( !planes
        && planeCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    3835,
                    0,
                    "%s",
                    "planes || planeCount == 0") )
    {
        __debugbreak();
    }
    vertCount = portal->vertexCount;
    w = portal->vertices;
    w = R_ChopPortalWinding(w, &vertCount, parentPlane, (float (*)[3])v);
    if ( !vertCount )
        return 0;
    if ( dpvsGlob.farPlaneEnabled )
    {
        w = R_ChopPortalWinding(w, &vertCount, &dpvsGlob.farPlane, (float (*)[3])(*v)[128 * (w == (const float (*)[3])v)]);
        if ( !vertCount )
            return 0;
    }
    for ( planeIndex = 0; planeIndex < planeCount; ++planeIndex )
    {
        w = R_ChopPortalWinding(w, &vertCount, &planes[planeIndex], (float (*)[3])(*v)[128 * (w == (const float (*)[3])v)]);
        if ( !vertCount )
            return 0;
    }
    if ( finalVerts )
        *finalVerts = w;
    return vertCount;
}

const float (*__cdecl R_ChopPortalWinding(
                const float (*vertsIn)[3],
                int *vertexCount,
                const DpvsPlane *plane,
                float (*vertsOut)[3]))[3]
{
    float *v5; // [esp+4h] [ebp-2C8h]
    float *v6; // [esp+8h] [ebp-2C4h]
    float *v7; // [esp+Ch] [ebp-2C0h]
    float *v8; // [esp+10h] [ebp-2BCh]
    unsigned __int8 sideForVert[136]; // [esp+1Ch] [ebp-2B0h]
    float lerpFactor; // [esp+A8h] [ebp-224h]
    int backCount; // [esp+ACh] [ebp-220h]
    int vertexIndex; // [esp+B0h] [ebp-21Ch]
    float distForVert[131]; // [esp+B4h] [ebp-218h]
    int newVertCount; // [esp+2C0h] [ebp-Ch]
    int frontCount; // [esp+2C4h] [ebp-8h]
    const float *v; // [esp+2C8h] [ebp-4h]

    frontCount = 0;
    backCount = 0;
    for ( vertexIndex = 0; vertexIndex < *vertexCount; ++vertexIndex )
    {
        distForVert[vertexIndex] = (float)((float)((float)((float)(plane->coeffs[0] * (float)(*vertsIn)[3 * vertexIndex])
                                                                                                         + (float)(plane->coeffs[1] * (float)(*vertsIn)[3 * vertexIndex + 1]))
                                                                                         + (float)(plane->coeffs[2] * (float)(*vertsIn)[3 * vertexIndex + 2]))
                                                                         + plane->coeffs[3])
                                                         - 0.001;
        sideForVert[vertexIndex] = 2;
        if ( distForVert[vertexIndex] >= -0.001 )
        {
            if ( distForVert[vertexIndex] > 0.001 )
            {
                sideForVert[vertexIndex] = 0;
                ++frontCount;
            }
        }
        else
        {
            sideForVert[vertexIndex] = 1;
            ++backCount;
        }
    }
    if ( frontCount )
    {
        if ( backCount )
        {
            sideForVert[vertexIndex] = sideForVert[0];
            distForVert[vertexIndex] = distForVert[0];
            newVertCount = 0;
            for ( vertexIndex = 0; vertexIndex < *vertexCount && newVertCount < 128; ++vertexIndex )
            {
                if ( sideForVert[vertexIndex] == 2 )
                {
                    v7 = &(*vertsOut)[3 * newVertCount];
                    v8 = (float *)&(*vertsIn)[3 * vertexIndex];
                    *v7 = *v8;
                    v7[1] = v8[1];
                    v7[2] = v8[2];
                    ++newVertCount;
                }
                else
                {
                    if ( !sideForVert[vertexIndex] )
                    {
                        v5 = &(*vertsOut)[3 * newVertCount];
                        v6 = (float *)&(*vertsIn)[3 * vertexIndex];
                        *v5 = *v6;
                        v5[1] = v6[1];
                        v5[2] = v6[2];
                        ++newVertCount;
                    }
                    if ( sideForVert[vertexIndex + 1] != 2 && sideForVert[vertexIndex + 1] != sideForVert[vertexIndex] )
                    {
                        lerpFactor = distForVert[vertexIndex] / (float)(distForVert[vertexIndex] - distForVert[vertexIndex + 1]);
                        v = &(*vertsIn)[3 * ((vertexIndex + 1) % *vertexCount)];
                        (*vertsOut)[3 * newVertCount] = (float)((float)(*v - (float)(*vertsIn)[3 * vertexIndex]) * lerpFactor)
                                                                                    + (float)(*vertsIn)[3 * vertexIndex];
                        (*vertsOut)[3 * newVertCount + 1] = (float)((float)(v[1] - (float)(*vertsIn)[3 * vertexIndex + 1])
                                                                                                            * lerpFactor)
                                                                                            + (float)(*vertsIn)[3 * vertexIndex + 1];
                        (*vertsOut)[3 * newVertCount++ + 2] = (float)((float)(v[2] - (float)(*vertsIn)[3 * vertexIndex + 2])
                                                                                                                * lerpFactor)
                                                                                                + (float)(*vertsIn)[3 * vertexIndex + 2];
                    }
                }
            }
            if ( newVertCount < 3
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                            1059,
                            0,
                            "%s",
                            "newVertCount >= 3") )
            {
                __debugbreak();
            }
            *vertexCount = newVertCount;
            return vertsOut;
        }
        else
        {
            return vertsIn;
        }
    }
    else
    {
        *vertexCount = 0;
        return 0;
    }
}

void __cdecl R_SetAncestorListStatus(GfxPortal *portal, bool isAncestor)
{
    while ( portal )
    {
        if ( portal->writable.isAncestor == isAncestor
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                        4070,
                        0,
                        "%s",
                        "portal->writable.isAncestor != isAncestor") )
        {
            __debugbreak();
        }
        portal->writable.isAncestor = isAncestor;
        portal = portal->writable.queuedParent;
    }
}

void __cdecl R_AddWorldSurfacesFrustumOnly(unsigned int viewIndex)
{
    int v1; // [esp+8h] [ebp-D0h]
    int v2; // [esp+Ch] [ebp-CCh]
    const DpvsPlane *frustumPlanes; // [esp+18h] [ebp-C0h]
    int v4; // [esp+1Ch] [ebp-BCh]
    int v5; // [esp+20h] [ebp-B8h]
    int frustumPlaneCount; // [esp+24h] [ebp-B4h]
    const DpvsPlane *plane; // [esp+30h] [ebp-A8h]
    int v8; // [esp+34h] [ebp-A4h]
    GfxCell *cell; // [esp+38h] [ebp-A0h]
    GfxCell *cella; // [esp+38h] [ebp-A0h]
    unsigned int oldViewIndex; // [esp+3Ch] [ebp-9Ch]
    unsigned int cellIndex; // [esp+40h] [ebp-98h]
    unsigned int cellIndexa; // [esp+40h] [ebp-98h]
    unsigned int casterIndex; // [esp+44h] [ebp-94h]
    unsigned int cellDrawBits[32]; // [esp+48h] [ebp-90h] BYREF
    unsigned int cellCount; // [esp+C8h] [ebp-10h]
    unsigned int visibleCellIndex; // [esp+CCh] [ebp-Ch]
    unsigned int *cellCasterBits; // [esp+D0h] [ebp-8h]
    unsigned int cellBitsCount; // [esp+D4h] [ebp-4h]

    if ((!viewIndex || viewIndex > 2)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
            4610,
            0,
            "%s\n\t(viewIndex) = %i",
            "((viewIndex >= SCENE_VIEW_SUNSHADOW_0) && (viewIndex <= SCENE_VIEW_SUNSHADOW_1))",
            viewIndex))
    {
        __debugbreak();
    }
    oldViewIndex = R_SetVisData(viewIndex);
    cellCount = rgp.world->dpvsPlanes.cellCount;
    if (!cellCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 4615, 0, "%s", "cellCount"))
    {
        __debugbreak();
    }
    if (sm_strictCull->current.enabled)
    {
        cellBitsCount = (cellCount + 31) >> 5;
        memset((unsigned __int8 *)cellDrawBits, 0, 4 * cellBitsCount);
        for (visibleCellIndex = 0; visibleCellIndex < cellCount; ++visibleCellIndex)
        {
            if ((dpvsGlob.cellVisibleBits[visibleCellIndex >> 5] & (1 << (visibleCellIndex & 0x1F))) != 0)
            {
                cellCasterBits = &rgp.world->cellCasterBits[cellBitsCount * visibleCellIndex];
                for (casterIndex = 0; casterIndex < cellBitsCount; ++casterIndex)
                    cellDrawBits[casterIndex] |= cellCasterBits[casterIndex];
            }
        }
        for (cellIndex = 0; cellIndex < cellCount; ++cellIndex)
        {
            if ((cellDrawBits[cellIndex >> 5] & (1 << (cellIndex & 0x1F))) != 0)
            {
                cell = &rgp.world->cells[cellIndex];
                frustumPlaneCount = g_dpvsView->frustumPlaneCount;
                v8 = 0;
                plane = g_dpvsView->frustumPlanes;
                while (v8 < frustumPlaneCount)
                {
                    if (R_DpvsPlaneMaxSignedDistToBox(plane, cell->mins) <= 0.0)
                    {
                        v5 = 1;
                        goto LABEL_27;
                    }
                    ++v8;
                    ++plane;
                }
                v5 = 0;
            LABEL_27:
                if (!v5)
                    R_AddCellSurfacesAndCullGroupsInFrustumDelayed(
                        cell,
                        g_dpvsView->frustumPlanes,
                        g_dpvsView->frustumPlaneCount,
                        g_dpvsView->frustumPlaneCount);
            }
        }
    }
    else
    {
        for (cellIndexa = 0; cellIndexa < cellCount; ++cellIndexa)
        {
            cella = &rgp.world->cells[cellIndexa];
            v2 = g_dpvsView->frustumPlaneCount;
            v4 = 0;
            frustumPlanes = g_dpvsView->frustumPlanes;
            while (v4 < v2)
            {
                if (R_DpvsPlaneMaxSignedDistToBox(frustumPlanes, cella->mins) <= 0.0)
                {
                    v1 = 1;
                    goto LABEL_39;
                }
                ++v4;
                ++frustumPlanes;
            }
            v1 = 0;
        LABEL_39:
            if (!v1)
                R_AddCellSurfacesAndCullGroupsInFrustumDelayed(
                    cella,
                    g_dpvsView->frustumPlanes,
                    g_dpvsView->frustumPlaneCount,
                    g_dpvsView->frustumPlaneCount);
        }
    }
    R_SetVisData(oldViewIndex);
}

void __cdecl R_AddCellSurfacesAndCullGroupsInFrustumDelayed(
    const GfxCell *cell,
    const DpvsPlane *planes,
    unsigned __int8 planeCount,
    unsigned __int8 frustumPlaneCount)
{
    DpvsDynamicCellCmd dpvsDynamicCell; // [esp+0h] [ebp-18h] BYREF
    DpvsStaticCellCmd dpvsStaticCell; // [esp+Ch] [ebp-Ch] BYREF

    dpvsStaticCell.cell = cell;
    dpvsStaticCell.planes = planes;
    dpvsStaticCell.planeCount = planeCount;
    dpvsStaticCell.frustumPlaneCount = frustumPlaneCount;
    dpvsStaticCell.viewIndex = g_viewIndex;
    Sys_AddWorkerCmdInternal(&r_dpvs_staticWorkerCmd, (unsigned __int8 *)&dpvsStaticCell, 0);
    dpvsDynamicCell.cellIndex = cell - rgp.world->cells;
    dpvsDynamicCell.planes = planes;
    dpvsDynamicCell.planeCount = planeCount;
    dpvsDynamicCell.frustumPlaneCount = frustumPlaneCount;
    dpvsDynamicCell.viewIndex = g_viewIndex;
    Sys_AddWorkerCmdInternal(&r_dpvs_dynmodelWorkerCmd, (unsigned __int8 *)&dpvsDynamicCell, 0);
    Sys_AddWorkerCmdInternal(&r_dpvs_sceneentWorkerCmd, (unsigned __int8 *)&dpvsDynamicCell, 0);
    Sys_AddWorkerCmdInternal(&r_dpvs_dynbrushWorkerCmd, (unsigned __int8 *)&dpvsDynamicCell, 0);
}

void __cdecl R_ShowCull(const float *viewOrigin)
{
    float *v1; // [esp+18h] [ebp-4ACh]
    const float *v2; // [esp+20h] [ebp-4A4h]
    float v3; // [esp+28h] [ebp-49Ch]
    float v4; // [esp+30h] [ebp-494h]
    float entMaxs[3]; // [esp+50h] [ebp-474h] BYREF
    float entMins[3]; // [esp+5Ch] [ebp-468h] BYREF
    float *color; // [esp+68h] [ebp-45Ch]
    float v8; // [esp+6Ch] [ebp-458h]
    GfxSceneModel *v9; // [esp+70h] [ebp-454h]
    char string[512]; // [esp+74h] [ebp-450h] BYREF
    const float *textColor; // [esp+274h] [ebp-250h]
    float rangeSq; // [esp+278h] [ebp-24Ch]
    char text[516]; // [esp+27Ch] [ebp-248h] BYREF
    const float *origin; // [esp+484h] [ebp-40h]
    GfxSceneModel *sceneModel; // [esp+488h] [ebp-3Ch]
    float mins[3]; // [esp+48Ch] [ebp-38h] BYREF
    float radius; // [esp+498h] [ebp-2Ch]
    unsigned int sceneEntCount; // [esp+49Ch] [ebp-28h]
    float maxs[3]; // [esp+4A0h] [ebp-24h] BYREF
    unsigned int sceneEntIndex; // [esp+4ACh] [ebp-18h]
    GfxSceneEntity *sceneEnt; // [esp+4B0h] [ebp-14h]
    GfxEntCellRefInfo *entInfo; // [esp+4B4h] [ebp-10h]
    GfxSceneBrush *sceneBrush; // [esp+4B8h] [ebp-Ch]
    unsigned __int8 *sceneEntVisData; // [esp+4BCh] [ebp-8h]
    const GfxBrushModel *bmodel; // [esp+4C0h] [ebp-4h]

    if ( r_showDObjModelNames->current.value > 0.0 )
    {
        sceneEntCount = scene.sceneDObjCount;
        sceneEntVisData = scene.sceneDObjVisData[0];
        rangeSq = r_showDObjModelNames->current.value * r_showDObjModelNames->current.value;
        for ( sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex )
        {
            sceneEnt = &scene.sceneDObj[sceneEntIndex];
            if ( sceneEntVisData[sceneEntIndex] == 1 )
                v2 = colorGreen;
            else
                v2 = colorRed;
            textColor = v2;
            if ( Vec3DistanceSq(sceneEnt->placement.base.origin, viewOrigin) <= rangeSq )
            {
                if ( sceneEnt->obj && sceneEnt->obj->numModels )
                    _snprintf(
                        text,
                        0x200u,
                        "%d:%d:%d:%s",
                        sceneEntIndex,
                        sceneEnt->entnum,
                        sceneEnt->obj->numModels,
                        **(const char ***)sceneEnt->obj->localModels);
                else
                    _snprintf(
                        text,
                        0x200u,
                        "%d:%d:%d:%s",
                        sceneEntIndex,
                        sceneEnt->entnum,
                        sceneEnt->obj->numModels,
                        "");
                R_AddDebugString(&frontEndDataOut->debugGlobals, sceneEnt->placement.base.origin, textColor, 0.25, text);
            }
        }
    }
    if ( r_showEntModelNames->current.value > 0.0 )
    {
        sceneEntCount = scene.sceneModelCount;
        sceneEntVisData = scene.sceneModelVisData[0];
        v8 = r_showEntModelNames->current.value * r_showEntModelNames->current.value;
        for ( sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex )
        {
            v9 = &scene.sceneModel[sceneEntIndex];
            if ( sceneEntVisData[sceneEntIndex] == 1 )
                v1 = (float *)colorGreen;
            else
                v1 = (float *)colorRed;
            color = v1;
            if ( Vec3DistanceSq(v9->placement.base.origin, viewOrigin) <= v8 )
            {
                if ( v9->model && v9->model->name )
                    _snprintf(string, 0x200u, "%d:%d:%s", sceneEntIndex, v9->entnum, v9->model->name);
                else
                    _snprintf(string, 0x200u, "%d:%d:%s", sceneEntIndex, v9->entnum, "");
                R_AddDebugString(&frontEndDataOut->debugGlobals, v9->placement.base.origin, color, 0.25, string);
            }
        }
    }
    if ( r_showCullXModels->current.enabled )
    {
        sceneEntCount = scene.sceneDObjCount;
        sceneEntVisData = scene.sceneDObjVisData[0];
        entInfo = (GfxEntCellRefInfo *)scene.dynSModelVisBitsCamera[scene.dpvs.localClientNum - 4];
        for ( sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex )
        {
            sceneEnt = &scene.sceneDObj[sceneEntIndex];
            if ( sceneEnt->cull.state >= 2 )
            {
                if ( sceneEntVisData[sceneEntIndex] == 1 )
                    R_AddDebugBox(&frontEndDataOut->debugGlobals, sceneEnt->cull.mins, sceneEnt->cull.maxs, colorGreen);
                else
                    R_AddDebugBox(&frontEndDataOut->debugGlobals, sceneEnt->cull.mins, sceneEnt->cull.maxs, colorRed);
                R_AddDebugLine(&frontEndDataOut->debugGlobals, sceneEnt->cull.mins, sceneEnt->lightingOrigin, colorLtOrange, 0);
                R_AddDebugLine(&frontEndDataOut->debugGlobals, sceneEnt->lightingOrigin, sceneEnt->cull.maxs, colorDkRed, 0);
            }
            //v4 = (__int128)entInfo[sceneEnt->entnum].bmodel ^ _mask__NegFloat_;
            v4 = -entInfo[sceneEnt->entnum].radius;
            entMins[0] = sceneEnt->placement.base.origin[0] + v4;
            entMins[1] = sceneEnt->placement.base.origin[1] + v4;
            entMins[2] = sceneEnt->placement.base.origin[2] + v4;
            v3 = entInfo[sceneEnt->entnum].radius;
            entMaxs[0] = sceneEnt->placement.base.origin[0] + v3;
            entMaxs[1] = sceneEnt->placement.base.origin[1] + v3;
            entMaxs[2] = sceneEnt->placement.base.origin[2] + v3;
            R_AddDebugBox(&frontEndDataOut->debugGlobals, entMins, entMaxs, colorBlue);
        }
        sceneEntCount = scene.sceneModelCount;
        sceneEntVisData = scene.sceneModelVisData[0];
        for ( sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex )
        {
            sceneModel = &scene.sceneModel[sceneEntIndex];
            origin = CG_GetEntityOrigin(scene.dpvs.localClientNum, sceneModel->entnum);
            radius = XModelGetRadius(sceneModel->model);
            mins[0] = origin[0] + (-(radius));
            mins[1] = origin[1] + (-(radius));
            mins[2] = origin[2] + (-(radius));
            maxs[0] = *origin + radius;
            maxs[1] = origin[1] + radius;
            maxs[2] = origin[2] + radius;
            if ( sceneEntVisData[sceneEntIndex] == 1 )
                R_AddDebugBox(&frontEndDataOut->debugGlobals, mins, maxs, colorGreen);
            else
                R_AddDebugBox(&frontEndDataOut->debugGlobals, mins, maxs, colorRed);
        }
    }
    if ( r_showCullBModels->current.enabled )
    {
        sceneEntCount = scene.sceneBrushCount;
        sceneEntVisData = scene.sceneBrushVisData[0];
        for ( sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex )
        {
            sceneBrush = &scene.sceneBrush[sceneEntIndex];
            bmodel = sceneBrush->bmodel;
            if ( sceneEntVisData[sceneEntIndex] == 1 )
                R_AddDebugBox(&frontEndDataOut->debugGlobals, bmodel->writable.mins, bmodel->writable.maxs, colorGreen);
            else
                R_AddDebugBox(&frontEndDataOut->debugGlobals, bmodel->writable.mins, bmodel->writable.maxs, colorRed);
        }
    }
}

void __cdecl R_InitSceneData(int localClientNum)
{
    unsigned int cellIndex; // [esp+0h] [ebp-10h]
    unsigned int offset; // [esp+4h] [ebp-Ch]
    unsigned int cellCount; // [esp+Ch] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 4820, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 4821, 0, "%s", "rgp.world") )
    {
        __debugbreak();
    }
    if ( gfxCfg.entCount * gfxCfg.maxClientViews > 0x2000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    4823,
                    0,
                    "%s",
                    "gfxCfg.maxClientViews * gfxCfg.entCount <= MAX_TOTAL_ENT_COUNT") )
    {
        __debugbreak();
    }
    cellCount = rgp.world->dpvsPlanes.cellCount;
    if ( (gfxCfg.entCount & 0x1F) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    4825,
                    0,
                    "%s\n\t(gfxCfg.entCount) = %i",
                    "(!(gfxCfg.entCount & 31))",
                    gfxCfg.entCount) )
    {
        __debugbreak();
    }
    offset = localClientNum * (gfxCfg.entCount >> 5);
    if ( offset >= 0x100
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    4828,
                    0,
                    "offset doesn't index MAX_TOTAL_ENT_COUNT >> 5\n\t%i not in [0, %i)",
                    offset,
                    256) )
    {
        __debugbreak();
    }
    for ( cellIndex = 0; cellIndex < 2 * cellCount; ++cellIndex )
        Com_Memset(&rgp.world->dpvsPlanes.sceneEntCellBits[256 * cellIndex + offset], 0, 4 * (gfxCfg.entCount >> 5));
    memset((unsigned __int8 *)dpvsGlob.entVisBits[localClientNum], 0, 4 * (gfxCfg.entCount >> 5));
    memset((unsigned __int8 *)scene.dynSModelVisBitsCamera[localClientNum - 4], 0, 4 * gfxCfg.entCount);
}

void __cdecl DynEntCl_InitFilter()
{
    unsigned int drawType; // [esp+0h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 4850, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 4851, 0, "%s", "rgp.world") )
    {
        __debugbreak();
    }
    scene.sceneDynModelCount = 0;
    scene.sceneDynBrushCount = 0;
    for ( drawType = 0; drawType < 2; ++drawType )
        Com_Memset(
            rgp.world->dpvsDyn.dynEntCellBits[drawType],
            0,
            4 * rgp.world->dpvsPlanes.cellCount * rgp.world->dpvsDyn.dynEntClientWordCount[drawType]);
}

void __cdecl R_InitSceneBuffers()
{
    unsigned int localClientNum; // [esp+0h] [ebp-8h]
    unsigned int viewIndex; // [esp+4h] [ebp-4h]

    if ( (gfxCfg.entCount & 0x1F) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    4868,
                    1,
                    "%s",
                    "(gfxCfg.entCount & 31) == 0") )
    {
        __debugbreak();
    }
    scene.entOverflowedDrawBuf = (unsigned int *)R_AllocGlobalVariable(gfxCfg.entCount >> 3, "R_InitSceneBuffers");
    for ( viewIndex = 0; viewIndex < 7; ++viewIndex )
        scene.dynSModelVisBitsCamera[viewIndex - 13] = (unsigned int)R_AllocGlobalVariable(
                                                                                                                                     gfxCfg.entCount,
                                                                                                                                     "R_InitSceneBuffers");
    scene.dpvs.sceneXModelIndex = (unsigned __int16 *)R_AllocGlobalVariable(2 * gfxCfg.entCount, "R_InitSceneBuffers");
    scene.dpvs.sceneDObjIndex = (unsigned __int16 *)R_AllocGlobalVariable(2 * gfxCfg.entCount, "R_InitSceneBuffers");
    for ( localClientNum = 0; localClientNum < gfxCfg.maxClientViews; ++localClientNum )
    {
        dpvsGlob.entVisBits[localClientNum] = (unsigned int *)R_AllocGlobalVariable(
                                                                                                                        4 * (gfxCfg.entCount >> 5),
                                                                                                                        "R_InitSceneBuffers");
        scene.dynSModelVisBitsCamera[localClientNum - 4] = (unsigned int)R_AllocGlobalVariable(
                                                                                                                                             4 * gfxCfg.entCount,
                                                                                                                                             "R_InitSceneBuffers");
    }
}

void __cdecl R_ShutdownSceneBuffers()
{
    unsigned int localClientNum; // [esp+0h] [ebp-8h]
    unsigned int viewIndex; // [esp+4h] [ebp-4h]

    for ( viewIndex = 0; viewIndex < 7; ++viewIndex )
        R_FreeGlobalVariable((void *)scene.dynSModelVisBitsCamera[viewIndex - 13]);
    R_FreeGlobalVariable(scene.dpvs.sceneXModelIndex);
    R_FreeGlobalVariable(scene.dpvs.sceneDObjIndex);
    for ( localClientNum = 0; localClientNum < gfxCfg.maxClientViews; ++localClientNum )
    {
        R_FreeGlobalVariable(dpvsGlob.entVisBits[localClientNum]);
        R_FreeGlobalVariable((void *)scene.dynSModelVisBitsCamera[localClientNum - 4]);
    }
}

void __cdecl R_ClearDpvsScene()
{
    unsigned int drawType; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]
    int ia; // [esp+4h] [ebp-4h]

    if ( !rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 4922, 0, "%s", "rgp.world") )
    {
        __debugbreak();
    }
    if ( !rgp.world->cells
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 4923, 0, "%s", "rgp.world->cells") )
    {
        __debugbreak();
    }
    Com_Memset((unsigned int *)scene.dpvs.sceneXModelIndex, 255, 2 * gfxCfg.entCount);
    if ( *scene.dpvs.sceneXModelIndex != 0xFFFF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    4926,
                    0,
                    "%s\n\t(scene.dpvs.sceneXModelIndex[0]) = %i",
                    "(scene.dpvs.sceneXModelIndex[0] == (65535))",
                    *scene.dpvs.sceneXModelIndex) )
    {
        __debugbreak();
    }
    Com_Memset((unsigned int *)scene.dpvs.sceneDObjIndex, 255, 2 * gfxCfg.entCount);
    if ( *scene.dpvs.sceneDObjIndex != 0xFFFF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    4929,
                    0,
                    "%s\n\t(scene.dpvs.sceneDObjIndex[0]) = %i",
                    "(scene.dpvs.sceneDObjIndex[0] == (65535))",
                    *scene.dpvs.sceneDObjIndex) )
    {
        __debugbreak();
    }
    if ( (gfxCfg.entCount & 0x1F) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    4931,
                    1,
                    "%s",
                    "(gfxCfg.entCount & 31) == 0") )
    {
        __debugbreak();
    }
    Com_Memset(scene.entOverflowedDrawBuf, 0, gfxCfg.entCount >> 3);
    for ( i = 0; i < 7; ++i )
        Com_Memset((unsigned int *)scene.dynSModelVisBitsCamera[i - 13], 0, gfxCfg.entCount);
    Com_Memcpy(rgp.world->dpvs.smodelVisDataCameraSaved, rgp.world->dpvs.smodelVisData[0], rgp.world->dpvs.smodelCount);
    Com_Memcpy(
        rgp.world->dpvs.surfaceVisDataCameraSaved,
        rgp.world->dpvs.surfaceVisData[0],
        rgp.world->models->surfaceCount);
    for ( ia = 0; ia < 3; ++ia )
    {
        Com_Memset((unsigned int *)rgp.world->dpvs.smodelVisData[ia], 0, rgp.world->dpvs.smodelCount);
        Com_Memset((unsigned int *)rgp.world->dpvs.surfaceVisData[ia], 0, rgp.world->models->surfaceCount);
        for ( drawType = 0; drawType < 2; ++drawType )
            Com_Memset(
                (unsigned int *)rgp.world->dpvsDyn.dynEntVisData[drawType][ia],
                0,
                rgp.world->dpvsDyn.dynEntClientCount[drawType]);
    }
    Com_Memset((unsigned int *)&rgp.world->sceneDynModel->info, 0, 6 * scene.sceneDynModelCount);
    scene.sceneDynModelCount = 0;
    Com_Memset((unsigned int *)rgp.world->sceneDynBrush, 0, 4 * scene.sceneDynBrushCount);
    scene.sceneDynBrushCount = 0;
    R_SetVisData(0);
}

void __cdecl R_ClearDpvsScene_For_ExtraCam()
{
    unsigned int drawType; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]
    int ia; // [esp+4h] [ebp-4h]

    if ( !rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 4982, 0, "%s", "rgp.world") )
    {
        __debugbreak();
    }
    if ( !rgp.world->cells
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 4983, 0, "%s", "rgp.world->cells") )
    {
        __debugbreak();
    }
    if ( (gfxCfg.entCount & 0x1F) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    4991,
                    1,
                    "%s",
                    "(gfxCfg.entCount & 31) == 0") )
    {
        __debugbreak();
    }
    Com_Memset(scene.entOverflowedDrawBuf, 0, gfxCfg.entCount >> 3);
    for ( i = 0; i < 7; ++i )
        Com_Memset((unsigned int *)scene.dynSModelVisBitsCamera[i - 13], 0, gfxCfg.entCount);
    for ( ia = 0; ia < 3; ++ia )
    {
        Com_Memset((unsigned int *)rgp.world->dpvs.smodelVisData[ia], 0, rgp.world->dpvs.smodelCount);
        Com_Memset((unsigned int *)rgp.world->dpvs.surfaceVisData[ia], 0, rgp.world->models->surfaceCount);
        for ( drawType = 0; drawType < 2; ++drawType )
            Com_Memset(
                (unsigned int *)rgp.world->dpvsDyn.dynEntVisData[drawType][ia],
                0,
                rgp.world->dpvsDyn.dynEntClientCount[drawType]);
    }
    Com_Memset((unsigned int *)&rgp.world->sceneDynModel->info, 0, 6 * scene.sceneDynModelCount);
    scene.sceneDynModelCount = 0;
    Com_Memset((unsigned int *)rgp.world->sceneDynBrush, 0, 4 * scene.sceneDynBrushCount);
    scene.sceneDynBrushCount = 0;
    R_SetVisData(0);
}

bool __cdecl R_CullDynamicSpotLightInCameraView()
{
    if ( !scene.addedLightCount )
        return 1;
    if ( scene.addedLight[0].type != 2 )
        return 1;
    scene.isAddedLightCulled[0] = R_CullPointAndRadius(
                                                                    scene.addedLight[0].origin,
                                                                    scene.addedLight[0].radius,
                                                                    dpvsGlob.views[scene.dpvs.localClientNum][0].frustumPlanes,
                                                                    dpvsGlob.views[scene.dpvs.localClientNum][0].frustumPlaneCount);
    return scene.isAddedLightCulled[0];
}

void __cdecl R_CullDynamicPointLightsInCameraView()
{
    int planeCount; // [esp+Ch] [ebp-10h]
    DpvsPlane *planes; // [esp+10h] [ebp-Ch]
    int lightIndex; // [esp+18h] [ebp-4h]

    planes = dpvsGlob.views[scene.dpvs.localClientNum][0].frustumPlanes;
    planeCount = dpvsGlob.views[scene.dpvs.localClientNum][0].frustumPlaneCount;
    for ( lightIndex = 0; lightIndex < scene.addedLightCount; ++lightIndex )
    {
        if ( scene.addedLight[lightIndex].type != 2 || lightIndex )
            scene.isAddedLightCulled[lightIndex] = R_CullPointAndRadius(
                                                                                             (const float *)&scene.isAddedLightCulled[368 * lightIndex - 11748],
                                                                                             *(float *)&scene.isAddedLightCulled[368 * lightIndex - 11736],
                                                                                             planes,
                                                                                             planeCount);
    }
}

void __cdecl PointFrom3Planes(float *a, float *b, float *c, float *r)
{
    float v4; // [esp+0h] [ebp-38h]
    float v5; // [esp+4h] [ebp-34h]
    float v6; // [esp+8h] [ebp-30h]
    float bXc[3]; // [esp+Ch] [ebp-2Ch] BYREF
    float invDiv; // [esp+18h] [ebp-20h]
    float cXa[3]; // [esp+1Ch] [ebp-1Ch] BYREF
    float div; // [esp+28h] [ebp-10h]
    float aXb[3]; // [esp+2Ch] [ebp-Ch] BYREF

    Vec3Cross(a, b, aXb);
    Vec3Cross(c, a, cXa);
    Vec3Cross(b, c, bXc);
    div = (float)((float)(*a * bXc[0]) + (float)(a[1] * bXc[1])) + (float)(a[2] * bXc[2]);
    invDiv = -1.0 / div;
    v6 = c[3] * (float)(-1.0 / div);
    *r = v6 * aXb[0];
    r[1] = v6 * aXb[1];
    r[2] = v6 * aXb[2];
    v5 = b[3] * invDiv;
    *r = (float)(v5 * cXa[0]) + *r;
    r[1] = (float)(v5 * cXa[1]) + r[1];
    r[2] = (float)(v5 * cXa[2]) + r[2];
    v4 = a[3] * invDiv;
    *r = (float)(v4 * bXc[0]) + *r;
    r[1] = (float)(v4 * bXc[1]) + r[1];
    r[2] = (float)(v4 * bXc[2]) + r[2];
}

void __cdecl R_SetupWorldSurfacesDpvs(const GfxViewParms *viewParms, unsigned int renderCullFlags)
{
    DpvsPlane *v2; // eax
    int v3; // ecx
    float *v4; // [esp+4Ch] [ebp-2CCh]
    float *v5; // [esp+50h] [ebp-2C8h]
    float *v6; // [esp+58h] [ebp-2C0h]
    float *v7; // [esp+68h] [ebp-2B0h]
    float *v8; // [esp+78h] [ebp-2A0h]
    float *v9; // [esp+90h] [ebp-288h]
    float *v10; // [esp+94h] [ebp-284h]
    float *v11; // [esp+DCh] [ebp-23Ch]
    float *v12; // [esp+E0h] [ebp-238h]
    float hull[4][3]; // [esp+100h] [ebp-218h] BYREF
    float v14[3]; // [esp+130h] [ebp-1E8h] BYREF
    float cross[3]; // [esp+13Ch] [ebp-1DCh] BYREF
    //float v16; // [esp+140h] [ebp-1D8h]
    //float v17; // [esp+144h] [ebp-1D4h]
    float v18; // [esp+148h] [ebp-1D0h]
    float v19[3]; // [esp+14Ch] [ebp-1CCh] BYREF
    int j; // [esp+158h] [ebp-1C0h]
    float v1[3]; // [esp+15Ch] [ebp-1BCh] BYREF
    float occluderPlane[4]; // [esp+168h] [ebp-1B0h] BYREF
    float v0[6]; // [esp+178h] [ebp-1A0h] BYREF
    float v24; // [esp+190h] [ebp-188h]
    float diff[3]; // [esp+194h] [ebp-184h]
    float div; // [esp+1A0h] [ebp-178h]
    DpvsPlane *plane; // [esp+1A4h] [ebp-174h]
    float distA; // [esp+1A8h] [ebp-170h]
    float distB; // [esp+1ACh] [ebp-16Ch]
    bool sideB; // [esp+1B1h] [ebp-167h]
    bool sideA; // [esp+1B2h] [ebp-166h]
    bool outside; // [esp+1B3h] [ebp-165h]
    float b[3]; // [esp+1B4h] [ebp-164h]
    float a[3]; // [esp+1C0h] [ebp-158h]
    int planeIdx; // [esp+1CCh] [ebp-14Ch]
    int lineIdx; // [esp+1D0h] [ebp-148h]
    int i; // [esp+1D4h] [ebp-144h]
    Occluder *occluder; // [esp+1D8h] [ebp-140h]
    float points[4][3]; // [esp+1DCh] [ebp-13Ch] BYREF
    bool neg; // [esp+20Eh] [ebp-10Ah]
    bool inside; // [esp+20Fh] [ebp-109h]
    unsigned int usePlanes; // [esp+210h] [ebp-108h]
    bool viewPointInside; // [esp+217h] [ebp-101h]
    float lines[4][2][3]; // [esp+218h] [ebp-100h] BYREF
    unsigned int idxOccluder; // [esp+278h] [ebp-A0h]
    float (*out)[4]; // [esp+27Ch] [ebp-9Ch]
    DpvsPlane farPlane; // [esp+280h] [ebp-98h] BYREF
    DpvsPlane *f; // [esp+298h] [ebp-80h]
    float zfar; // [esp+29Ch] [ebp-7Ch]
    int planeCount; // [esp+2A0h] [ebp-78h]
    DpvsView *dpvsView; // [esp+2A4h] [ebp-74h]
    float frustrumPoints[25]; // [esp+2A8h] [ebp-70h] BYREF
    float insideViewPos[3]; // [esp+30Ch] [ebp-Ch]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 5116, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 5117, 0, "%s", "rgp.world") )
    {
        __debugbreak();
    }
    if ( !viewParms
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 5118, 0, "%s", "viewParms") )
    {
        __debugbreak();
    }
    dpvsView = dpvsGlob.views[scene.dpvs.localClientNum];
    dpvsView->renderFxFlagsCull = renderCullFlags;
    dpvsGlob.viewProjMtx = &viewParms->viewProjectionMatrix;
    dpvsGlob.invViewProjMtx = &viewParms->inverseViewProjectionMatrix;
    dpvsGlob.projMtx = &viewParms->projectionMatrix;
    dpvsGlob.viewOrg[0] = viewParms->origin[0];
    dpvsGlob.viewOrg[1] = viewParms->origin[1];
    dpvsGlob.viewOrg[2] = viewParms->origin[2];
    dpvsGlob.viewOrg[3] = 1.0f;
    dpvsGlob.viewOrgIsDir = 0;
    *(_QWORD *)dpvsGlob.sunShadow.viewDir = *(_QWORD *)&viewParms->axis[0][0];
    dpvsGlob.sunShadow.viewDir[2] = viewParms->axis[0][2];
    dpvsGlob.sunShadow.viewDirDist = (float)((float)(dpvsGlob.viewOrg[0] * dpvsGlob.sunShadow.viewDir[0])
                                                                                 + (float)(dpvsGlob.viewOrg[1] * dpvsGlob.sunShadow.viewDir[1]))
                                                                 + (float)(dpvsGlob.viewOrg[2] * dpvsGlob.sunShadow.viewDir[2]);
    if ( viewParms->projectionMatrix.m[3][3] != 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    5148,
                    0,
                    "%s",
                    "viewParms->projectionMatrix.m[3][3] == 0") )
    {
        __debugbreak();
    }
    planeCount = 4;
    R_FrustumClipPlanes(&viewParms->viewProjectionMatrix, standardFrustumSidePlanes, 4, dpvsView->frustumPlanes);
    *(_QWORD *)dpvsGlob.nearPlane.coeffs = *(_QWORD *)&viewParms->axis[0][0];
    dpvsGlob.nearPlane.coeffs[2] = viewParms->axis[0][2];
    dpvsGlob.nearPlane.coeffs[3] = 0.1
                                                             - (float)((float)((float)(dpvsGlob.nearPlane.coeffs[0] * dpvsGlob.viewOrg[0])
                                                                                             + (float)(dpvsGlob.nearPlane.coeffs[1] * dpvsGlob.viewOrg[1]))
                                                                             + (float)(dpvsGlob.nearPlane.coeffs[2] * dpvsGlob.viewOrg[2]));
    R_SetDpvsPlaneSides(&dpvsGlob.nearPlane);
    dpvsView->frustumPlanes[4] = dpvsGlob.nearPlane;
    ++planeCount;
    zfar = R_GetFarPlaneDist();
    if ( zfar > 0.0 )
    {
        dpvsGlob.farPlane.coeffs[0] = -viewParms->axis[0][0];
        dpvsGlob.farPlane.coeffs[1] = -viewParms->axis[0][1];
        dpvsGlob.farPlane.coeffs[2] = -viewParms->axis[0][2];
        dpvsGlob.farPlane.coeffs[3] = zfar
                                                                - (float)((float)((float)(dpvsGlob.farPlane.coeffs[0] * dpvsGlob.viewOrg[0])
                                                                                                + (float)(dpvsGlob.farPlane.coeffs[1] * dpvsGlob.viewOrg[1]))
                                                                                + (float)(dpvsGlob.farPlane.coeffs[2] * dpvsGlob.viewOrg[2]));
        R_SetDpvsPlaneSides(&dpvsGlob.farPlane);
        dpvsGlob.farPlaneEnabled = 1;
        v2 = &dpvsView->frustumPlanes[planeCount];
        v2->coeffs[0] = dpvsGlob.farPlane.coeffs[0];
        v2->coeffs[1] = dpvsGlob.farPlane.coeffs[1];
        v2->coeffs[2] = dpvsGlob.farPlane.coeffs[2];
        v2->coeffs[3] = dpvsGlob.farPlane.coeffs[3];
        *(unsigned int *)v2->side = *(unsigned int *)dpvsGlob.farPlane.side;
        ++planeCount;
    }
    else
    {
        dpvsGlob.farPlaneEnabled = 0;
    }
    dpvsView->frustumPlaneCount = planeCount;
    dpvsGlob.sideFrustumPlanes = dpvsView->frustumPlanes;
    dpvsGlob.numOccluders = 0;
    f = dpvsView->frustumPlanes;
    PointFrom3Planes(
        dpvsView->frustumPlanes[4].coeffs,
        dpvsView->frustumPlanes[0].coeffs,
        dpvsView->frustumPlanes[2].coeffs,
        frustrumPoints);
    PointFrom3Planes(
        dpvsView->frustumPlanes[4].coeffs,
        dpvsView->frustumPlanes[1].coeffs,
        dpvsView->frustumPlanes[2].coeffs,
        &frustrumPoints[3]);
    PointFrom3Planes(
        dpvsView->frustumPlanes[4].coeffs,
        dpvsView->frustumPlanes[0].coeffs,
        dpvsView->frustumPlanes[3].coeffs,
        &frustrumPoints[6]);
    PointFrom3Planes(
        dpvsView->frustumPlanes[4].coeffs,
        dpvsView->frustumPlanes[1].coeffs,
        dpvsView->frustumPlanes[3].coeffs,
        &frustrumPoints[9]);
    farPlane = dpvsView->frustumPlanes[4];
    farPlane.coeffs[3] = farPlane.coeffs[3] - 100000.0;
    PointFrom3Planes(
        farPlane.coeffs,
        dpvsView->frustumPlanes[0].coeffs,
        dpvsView->frustumPlanes[2].coeffs,
        &frustrumPoints[12]);
    PointFrom3Planes(farPlane.coeffs, f[1].coeffs, f[2].coeffs, &frustrumPoints[15]);
    PointFrom3Planes(farPlane.coeffs, f->coeffs, f[3].coeffs, &frustrumPoints[18]);
    PointFrom3Planes(farPlane.coeffs, f[1].coeffs, f[3].coeffs, &frustrumPoints[21]);
    insideViewPos[0] = viewParms->origin[0];
    insideViewPos[1] = viewParms->origin[1];
    insideViewPos[2] = viewParms->origin[2];
    insideViewPos[0] = (float)(10.0 * viewParms->axis[0][0]) + insideViewPos[0];
    insideViewPos[1] = (float)(10.0 * viewParms->axis[0][1]) + insideViewPos[1];
    insideViewPos[2] = (float)(10.0 * viewParms->axis[0][2]) + insideViewPos[2];
    if ( r_enableOccluders->current.enabled )
    {
        out = dpvsGlob.occluderPlanes;
        dpvsGlob.numOccluders = 0;
        for ( idxOccluder = 0; idxOccluder < s_world.numOccluders; ++idxOccluder )
        {
            occluder = &s_world.occluders[idxOccluder];
            if ( (occluder->flags & 1) != 0 )
            {
                for ( i = 0; i < 4; ++i )
                {
                    v11 = points[i];
                    v12 = occluder->points[i];
                    *v11 = *v12;
                    v11[1] = v12[1];
                    v11[2] = v12[2];
                }
                *(_QWORD *)&lines[0][0][0] = *(_QWORD *)&points[0][0];
                lines[0][0][2] = points[0][2];
                lines[0][1][0] = points[1][0];
                lines[0][1][1] = points[1][1];
                lines[0][1][2] = points[1][2];
                lines[1][0][0] = points[1][0];
                lines[1][0][1] = points[1][1];
                lines[1][0][2] = points[1][2];
                lines[1][1][0] = points[2][0];
                lines[1][1][1] = points[2][1];
                lines[1][1][2] = points[2][2];
                lines[2][0][0] = points[2][0];
                lines[2][0][1] = points[2][1];
                lines[2][0][2] = points[2][2];
                lines[2][1][0] = points[3][0];
                lines[2][1][1] = points[3][1];
                lines[2][1][2] = points[3][2];
                lines[3][0][0] = points[3][0];
                lines[3][0][1] = points[3][1];
                lines[3][0][2] = points[3][2];
                *(_QWORD *)&lines[3][1][0] = *(_QWORD *)&points[0][0];
                lines[3][1][2] = points[0][2];
                usePlanes = 0;
                inside = 0;
                viewPointInside = 1;
                for ( lineIdx = 0; (unsigned int)lineIdx < 4; ++lineIdx )
                {
                    a[0] = lines[lineIdx][0][0];
                    a[1] = lines[lineIdx][0][1];
                    a[2] = lines[lineIdx][0][2];
                    *(_QWORD *)b = *(_QWORD *)&lines[lineIdx][1][0];
                    b[2] = lines[lineIdx][1][2];
                    outside = 0;
                    for ( planeIdx = 0; planeIdx < dpvsView->frustumPlaneCount; ++planeIdx )
                    {
                        plane = &dpvsView->frustumPlanes[planeIdx];
                        distA = (float)((float)((float)(plane->coeffs[0] * a[0])
                                                                    + (float)(dpvsView->frustumPlanes[planeIdx].coeffs[1] * a[1]))
                                                    + (float)(dpvsView->frustumPlanes[planeIdx].coeffs[2] * a[2]))
                                    + dpvsView->frustumPlanes[planeIdx].coeffs[3];
                        distB = (float)((float)((float)(plane->coeffs[0] * b[0])
                                                                    + (float)(dpvsView->frustumPlanes[planeIdx].coeffs[1] * b[1]))
                                                    + (float)(dpvsView->frustumPlanes[planeIdx].coeffs[2] * b[2]))
                                    + dpvsView->frustumPlanes[planeIdx].coeffs[3];
                        sideA = distA < 0.0;
                        sideB = distB < 0.0;
                        if ( distA < 0.0 && sideB )
                        {
                            outside = 1;
                            break;
                        }
                        if ( sideA != sideB )
                        {
                            if ( sideA )
                            {
                                div = distB / (float)(distB - distA);
                                diff[0] = a[0] - b[0];
                                diff[1] = a[1] - b[1];
                                diff[2] = a[2] - b[2];
                                a[0] = (float)(div * (float)(a[0] - b[0])) + b[0];
                                a[1] = (float)(div * (float)(a[1] - b[1])) + b[1];
                                a[2] = (float)(div * (float)(a[2] - b[2])) + b[2];
                            }
                            else
                            {
                                v24 = distA / (float)(distA - distB);
                                v0[3] = b[0] - a[0];
                                v0[4] = b[1] - a[1];
                                v0[5] = b[2] - a[2];
                                b[0] = (float)(v24 * (float)(b[0] - a[0])) + a[0];
                                b[1] = (float)(v24 * (float)(b[1] - a[1])) + a[1];
                                b[2] = (float)(v24 * (float)(b[2] - a[2])) + a[2];
                            }
                        }
                    }
                    if ( !outside )
                    {
                        usePlanes |= 1 << lineIdx;
                        inside = 1;
                        v10 = lines[lineIdx][0];
                        *v10 = a[0];
                        v10[1] = a[1];
                        v10[2] = a[2];
                        v9 = lines[lineIdx][1];
                        *v9 = b[0];
                        v9[1] = b[1];
                        v9[2] = b[2];
                    }
                }
                v0[0] = points[1][0] - points[0][0];
                v0[1] = points[1][1] - points[0][1];
                v0[2] = points[1][2] - points[0][2];
                v1[0] = points[3][0] - points[0][0];
                v1[1] = points[3][1] - points[0][1];
                v1[2] = points[3][2] - points[0][2];
                Vec3Cross(v0, v1, occluderPlane);
                Vec3NormalizeFast(occluderPlane);
                occluderPlane[3] = (float)((float)(occluderPlane[0] * points[0][0]) + (float)(occluderPlane[1] * points[0][1]))
                                                 + (float)(occluderPlane[2] * points[0][2]);
                neg = (float)((float)((float)((float)(occluderPlane[0] * viewParms->origin[0])
                                                                        + (float)(occluderPlane[1] * viewParms->origin[1]))
                                                        + (float)(occluderPlane[2] * viewParms->origin[2]))
                                        - occluderPlane[3]) < 0.0;
                if ( neg )
                {
                    occluderPlane[0] = -occluderPlane[0];
                    occluderPlane[1] = -occluderPlane[1];
                    occluderPlane[2] = -occluderPlane[2];
                    occluderPlane[3] = -occluderPlane[3];
                }
                v8 = (float *)out;
                (*out)[0] = occluderPlane[0];
                v8[1] = occluderPlane[1];
                v8[2] = occluderPlane[2];
                v8[3] = occluderPlane[3];
                ++out;
                for ( j = 0; j < 4; ++j )
                {
                    v19[0] = viewParms->origin[0] - points[j][0];
                    v19[1] = viewParms->origin[1] - points[j][1];
                    v19[2] = viewParms->origin[2] - points[j][2];
                    v7 = points[j];
                    v3 = (j + 1) % 4;
                    v14[0] = points[v3][0] - *v7;
                    v14[1] = points[v3][1] - v7[1];
                    v14[2] = points[v3][2] - v7[2];
                    Vec3Cross(v14, v19, cross);
                    Vec3NormalizeFast(cross);
                    v18 = (float)((float)(cross[0] * points[j][0]) + (float)(cross[1] * points[j][1])) + (float)(cross[2] * points[j][2]);
                    if ( neg )
                    {
                        cross[0] = -cross[0];
                        cross[1] = -cross[1];
                        cross[2] = -cross[2];
                        v18 = -v18;
                    }
                    v6 = (float *)out;
                    (*out)[0] = cross[0];
                    v6[1] = cross[1];
                    v6[2] = cross[2];
                    v6[3] = v18;
                    ++out;
                    if ( (float)((float)((float)((*out)[-4] * insideViewPos[0]) + (float)((*out)[-3] * insideViewPos[1]))
                                         + (float)((*out)[-2] * insideViewPos[2])) > (*out)[-1] )
                        viewPointInside = 0;
                    if ( (usePlanes & (1 << j)) != 0 )
                    {
                        if ( r_showOccluders->current.integer >= 1 )
                            R_AddDebugLine(&frontEndDataOut->debugGlobals, lines[j][0], lines[j][1], colorCyan, 0);
                    }
                    else
                    {
                        v4 = &(*out)[-4];
                        v5 = &(*out)[-8];
                        (*out)[-4] = (*out)[-8];
                        v4[1] = v5[1];
                        v4[2] = v5[2];
                        v4[3] = v5[3];
                    }
                }
                if ( (inside || viewPointInside) && r_showOccluders->current.integer >= 1 )
                {
                    if ( neg )
                    {
                        hull[0][0] = occluder->points[3][0];
                        hull[0][1] = occluder->points[3][1];
                        hull[0][2] = occluder->points[3][2];
                        hull[1][0] = occluder->points[2][0];
                        hull[1][1] = occluder->points[2][1];
                        hull[1][2] = occluder->points[2][2];
                        hull[2][0] = occluder->points[1][0];
                        hull[2][1] = occluder->points[1][1];
                        hull[2][2] = occluder->points[1][2];
                        hull[3][0] = occluder->points[0][0];
                        hull[3][1] = occluder->points[0][1];
                        hull[3][2] = occluder->points[0][2];
                    }
                    else
                    {
                        hull[0][0] = occluder->points[0][0];
                        hull[0][1] = occluder->points[0][1];
                        hull[0][2] = occluder->points[0][2];
                        hull[1][0] = occluder->points[1][0];
                        hull[1][1] = occluder->points[1][1];
                        hull[1][2] = occluder->points[1][2];
                        hull[2][0] = occluder->points[2][0];
                        hull[2][1] = occluder->points[2][1];
                        hull[2][2] = occluder->points[2][2];
                        hull[3][0] = occluder->points[3][0];
                        hull[3][1] = occluder->points[3][1];
                        hull[3][2] = occluder->points[3][2];
                    }
                    R_AddDebugPolygon(&frontEndDataOut->debugGlobals, 4, hull, color_0, 0, color_0, 0);
                }
                if ( inside || viewPointInside )
                {
                    if ( (unsigned int)(5 * dpvsGlob.numOccluders) >= 0x140 )
                    {
                        if ( !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                                        5399,
                                        0,
                                        "To many occluders on screen at one time.\n") )
                            __debugbreak();
                        return;
                    }
                    ++dpvsGlob.numOccluders;
                }
                else
                {
                    out -= 5;
                }
            }
        }
    }
}

void __cdecl R_EnableOccluder(const char *name, bool enable)
{
    unsigned int i; // [esp+14h] [ebp-4h]

    for ( i = 0; i < s_world.numOccluders; ++i )
    {
        if ( !strcmp(name, s_world.occluders[i].name) )
        {
            if ( enable )
                s_world.occluders[i].flags |= 1u;
            else
                s_world.occluders[i].flags &= ~1u;
        }
    }
}

void __cdecl R_SetViewFrustumPlanes(GfxViewInfo *viewInfo)
{
    float *v1; // [esp+0h] [ebp-Ch]
    DpvsPlane *v2; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    if ( dpvsGlob.views[scene.dpvs.localClientNum][0].frustumPlaneCount < 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    5436,
                    0,
                    "%s\n\t(dpvsGlob.views[scene.dpvs.localClientNum][SCENE_VIEW_CAMERA].frustumPlaneCount) = %i",
                    "(dpvsGlob.views[scene.dpvs.localClientNum][SCENE_VIEW_CAMERA].frustumPlaneCount >= 4)",
                    dpvsGlob.views[scene.dpvs.localClientNum][0].frustumPlaneCount) )
    {
        __debugbreak();
    }
    for ( i = 0; i < 4; ++i )
    {
        v1 = viewInfo->frustumPlanes[i];
        v2 = &dpvsGlob.views[scene.dpvs.localClientNum][0].frustumPlanes[i];
        *v1 = v2->coeffs[0];
        v1[1] = v2->coeffs[1];
        v1[2] = v2->coeffs[2];
        v1[3] = v2->coeffs[3];
    }
}

void __cdecl R_AddWorldSurfacesDpvs(const GfxViewParms *viewParms, int cameraCellIndex)
{
    bool v2; // [esp+0h] [ebp-14h]
    DpvsView *dpvsView; // [esp+10h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 5551, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 5552, 0, "%s", "rgp.world") )
    {
        __debugbreak();
    }
    if ( !viewParms
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 5553, 0, "%s", "viewParms") )
    {
        __debugbreak();
    }
    rg.debugViewParms = viewParms;

    {
        PROF_SCOPED("R_AddWorldSurfacesPortalWalk");
        R_AddWorldSurfacesPortalWalk(cameraCellIndex);
    }
    
    dpvsView = dpvsGlob.views[scene.dpvs.localClientNum];
    if ( dpvsGlob.farPlaneEnabled )
    {
        if ( dpvsView->frustumPlaneCount <= 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                        5574,
                        0,
                        "%s\n\t(dpvsView->frustumPlaneCount) = %i",
                        "(dpvsView->frustumPlaneCount > 0)",
                        dpvsView->frustumPlaneCount) )
        {
            __debugbreak();
        }
        v2 = dpvsView->frustumPlanes[dpvsView->frustumPlaneCount - 1].coeffs[0] == dpvsGlob.farPlane.coeffs[0]
            && dpvsView->frustumPlanes[dpvsView->frustumPlaneCount - 1].coeffs[1] == dpvsGlob.farPlane.coeffs[1]
            && dpvsView->frustumPlanes[dpvsView->frustumPlaneCount - 1].coeffs[2] == dpvsGlob.farPlane.coeffs[2]
            && dpvsView->frustumPlanes[dpvsView->frustumPlaneCount - 1].coeffs[3] == dpvsGlob.farPlane.coeffs[3];
        if ( !v2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                        5575,
                        0,
                        "%s",
                        "Vec4Compare( dpvsView->frustumPlanes[dpvsView->frustumPlaneCount - 1].coeffs, dpvsGlob.farPlane.coeffs )") )
        {
            __debugbreak();
        }
        R_AddSkySurfacesDpvs(dpvsView->frustumPlanes, dpvsView->frustumPlaneCount - 1);
    }
    if ( r_vc_makelog->current.integer )
        R_ShowLightVisCachePoints(viewParms->origin, dpvsView->frustumPlanes, dpvsView->frustumPlaneCount);
}

void __cdecl R_AddWorldSurfacesPortalWalk(int cameraCellIndex)
{
    GfxCell *v1; // [esp+8h] [ebp-10A8h]
    signed int k; // [esp+Ch] [ebp-10A4h]
    int j; // [esp+14h] [ebp-109Ch]
    _BYTE v4[128]; // [esp+18h] [ebp-1098h] BYREF
    unsigned int v5; // [esp+9Ch] [ebp-1014h]
    GfxCell *v6[1025]; // [esp+A0h] [ebp-1010h]
    GfxCell *cell; // [esp+10A4h] [ebp-Ch]
    int i; // [esp+10A8h] [ebp-8h]
    DpvsView *dpvsView; // [esp+10ACh] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 5450, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !rgp.world->dpvsPlanes.cellCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    5452,
                    0,
                    "%s",
                    "rgp.world->dpvsPlanes.cellCount") )
    {
        __debugbreak();
    }
    memset((unsigned __int8 *)dpvsGlob.cellVisibleBits, 0, 4 * ((rgp.world->dpvsPlanes.cellCount + 31) >> 5));
    if ( !r_skipPvs->current.enabled )
    {
        dpvsView = dpvsGlob.views[scene.dpvs.localClientNum];

        iassert(dpvsView == g_dpvsView);
        
        if ( cameraCellIndex < 0 )
        {
            for ( i = 0; i < rgp.world->dpvsPlanes.cellCount; ++i )
            {
                R_AddCellSurfacesAndCullGroupsInFrustumDelayed(
                    &rgp.world->cells[i],
                    dpvsView->frustumPlanes,
                    dpvsView->frustumPlaneCount,
                    dpvsView->frustumPlaneCount);
                R_SetCellVisible(&rgp.world->cells[i]);
            }
        }
        else
        {
            cell = &rgp.world->cells[cameraCellIndex];
            if ( r_singleCell->current.enabled )
            {
                dpvsGlob.farPlaneEnabled = 0;
                R_AddCellSurfacesAndCullGroupsInFrustumDelayed(
                    cell,
                    dpvsView->frustumPlanes,
                    dpvsView->frustumPlaneCount,
                    dpvsView->frustumPlaneCount);
                R_SetCellVisible(cell);
            }
            else if ( r_showForcedInvisibleCells->current.enabled )
            {
                v5 = 0;
                for ( j = 0; j < rgp.world->dpvsPlanes.cellCount; ++j )
                {
                    if ( R_CellIsForcedInvisible(&rgp.world->cells[j]) )
                    {
                        if ( v5 >= 0x400
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                                        5483,
                                        0,
                                        "invisibleCellsCount doesn't index ARRAY_COUNT(invisibleCells)\n\t%i not in [0, %i)",
                                        v5,
                                        1024) )
                        {
                            __debugbreak();
                        }
                        v6[v5++] = (GfxCell *)j;
                    }
                }
                dpvsGlob.farPlaneEnabled = 0;
                memcpy(v4, dpvsGlob.cellForceInvisibleBits, sizeof(v4));
                memset((unsigned __int8 *)dpvsGlob.cellForceInvisibleBits, 0, sizeof(dpvsGlob.cellForceInvisibleBits));
                for ( k = 0; k < (int)v5; ++k )
                {
                    v1 = &rgp.world->cells[(unsigned int)v6[k]];
                    R_AddCellSurfacesAndCullGroupsInFrustumDelayed(
                        v1,
                        dpvsView->frustumPlanes,
                        dpvsView->frustumPlaneCount,
                        dpvsView->frustumPlaneCount);
                    R_SetCellVisible(v1);
                }
                Sys_WaitWorkerCmdInternal(&r_dpvs_staticWorkerCmd);
                Sys_WaitWorkerCmdInternal(&r_dpvs_dynmodelWorkerCmd);
                Sys_WaitWorkerCmdInternal(&r_dpvs_sceneentWorkerCmd);
                Sys_WaitWorkerCmdInternal(&r_dpvs_entityWorkerCmd);
                memcpy(dpvsGlob.cellForceInvisibleBits, v4, sizeof(dpvsGlob.cellForceInvisibleBits));
            }
            else
            {
                R_VisitPortals(cell, &dpvsGlob.nearPlane, dpvsView->frustumPlanes, dpvsView->frustumPlaneCount);
            }
        }
    }
}

void __cdecl R_SetCellVisible(const GfxCell *cell)
{
    unsigned int cellIndex; // [esp+8h] [ebp-4h]

    cellIndex = cell - rgp.world->cells;
    dpvsGlob.cellVisibleBits[cellIndex >> 5] |= ~(dpvsGlob.cellForceInvisibleBits[cellIndex >> 5]
                                                                                            & (1 << (cellIndex & 0x1F)))
                                                                                        & (1 << (cellIndex & 0x1F));
}

unsigned int __cdecl R_CellIsForcedInvisible(const GfxCell *cell)
{
    if ( r_dpvs_useCellForceInvisibleBits->current.enabled )
        return dpvsGlob.cellForceInvisibleBits[(unsigned int)(cell - rgp.world->cells) >> 5]
                 & (1 << ((cell - rgp.world->cells) & 0x1F));
    else
        return 0;
}

void __cdecl R_VisitPortals(const GfxCell *cell, const DpvsPlane *parentPlane, DpvsPlane *planes, int planeCount)
{
    double v4; // st7
    float *v5; // [esp+8h] [ebp-D70h]
    float *v6; // [esp+Ch] [ebp-D6Ch]
    float v7; // [esp+10h] [ebp-D68h]
    float *v8; // [esp+18h] [ebp-D60h]
    float *v9; // [esp+1Ch] [ebp-D5Ch]
    float v10; // [esp+20h] [ebp-D58h]
    float v11; // [esp+28h] [ebp-D50h]
    DpvsPlane *childPlanes; // [esp+34h] [ebp-D44h]
    LargeLocal hullPointsPoolArray_large_local(0x20000); // [esp+38h] [ebp-D40h] BYREF
    GfxHullPointsPool (*hullPointsPoolArray)[256]; // [esp+40h] [ebp-D38h]
    unsigned int childPlanesCount; // [esp+44h] [ebp-D34h]
    int childPlaneCount; // [esp+48h] [ebp-D30h]
    int iteration; // [esp+4Ch] [ebp-D2Ch]
    GfxPortal *portal; // [esp+50h] [ebp-D28h]
    int queueIndex; // [esp+54h] [ebp-D24h]
    float portalVerts[64][3]; // [esp+58h] [ebp-D20h] BYREF
    float hullOrigin[3]; // [esp+35Ch] [ebp-A1Ch]
    DpvsClipChildren clipChildren; // [esp+368h] [ebp-A10h] BYREF
    unsigned int vertIndex; // [esp+36Ch] [ebp-A0Ch]
    PortalHeapNode portalQueue[256]; // [esp+370h] [ebp-A08h] BYREF
    float hull[64][2]; // [esp+B70h] [ebp-208h] BYREF
    unsigned int hullPointCount; // [esp+D74h] [ebp-4h]

    //LargeLocal::LargeLocal(&hullPointsPoolArray_large_local, 0x20000);
    hullPointsPoolArray = (GfxHullPointsPool(*)[256])hullPointsPoolArray_large_local.GetBuf();// LargeLocal::GetBuf(&hullPointsPoolArray_large_local);
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 4239, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    childPlanesCount = 0;
    for ( queueIndex = 0; queueIndex < 255; ++queueIndex )
        (*hullPointsPoolArray)[queueIndex].nextFree = &(*hullPointsPoolArray)[queueIndex + 1];
    (*hullPointsPoolArray)[queueIndex].nextFree = 0;
    dpvsGlob.nextFreeHullPoints = (GfxHullPointsPool *)hullPointsPoolArray;
    dpvsGlob.portalQueue = portalQueue;
    dpvsGlob.queuedCount = 0;
    R_VisitPortalsForCell(cell, 0, parentPlane, planes, 0x10u, planeCount, planeCount, 0, DPVS_CLIP_CHILDREN);
    iteration = 0;
    while ( dpvsGlob.queuedCount )
    {
        portal = R_NextQueuedPortal();
        if ( !portal && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 4264, 0, "%s", "portal") )
            __debugbreak();
        hullPointCount = Com_ConvexHull(
                                             portal->writable.hullPoints,
                                             portal->writable.hullPointCount,
                                             hull);
        R_FreeHullPoints((GfxHullPointsPool *)portal->writable.hullPoints);
        portal->writable.hullPoints = 0;
        if ( hullPointCount )
        {
            if ( ++iteration == r_portalWalkLimit->current.integer )
            {
                while ( dpvsGlob.queuedCount )
                {
                    portal = R_NextQueuedPortal();
                    R_FreeHullPoints((GfxHullPointsPool *)portal->writable.hullPoints);
                    portal->writable.hullPoints = 0;
                }
                break;
            }
            if ( R_DpvsPlaneDistToEye(&portal->plane) > 0.124875 )
            {
                v4 = R_DpvsPlaneDistToEye(&portal->plane);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                                4302,
                                0,
                                "%s\n\t(R_DpvsPlaneDistToEye( &portal->plane )) = %g",
                                "(R_DpvsPlaneDistToEye( &portal->plane ) <= 0.125f * 0.999f)",
                                v4) )
                    __debugbreak();
            }
            v11 = -portal->plane.coeffs[3];
            hullOrigin[0] = v11 * portal->plane.coeffs[0];
            hullOrigin[1] = v11 * portal->plane.coeffs[1];
            hullOrigin[2] = v11 * portal->plane.coeffs[2];
            for ( vertIndex = 0; vertIndex < hullPointCount; ++vertIndex )
            {
                v8 = portalVerts[vertIndex];
                v9 = portal->hullAxis[0];
                v10 = hull[vertIndex][0];
                *v8 = (float)(v10 * portal->hullAxis[0][0]) + hullOrigin[0];
                v8[1] = (float)(v10 * v9[1]) + hullOrigin[1];
                v8[2] = (float)(v10 * v9[2]) + hullOrigin[2];
                v5 = portalVerts[vertIndex];
                v6 = portal->hullAxis[1];
                v7 = hull[vertIndex][1];
                *v5 = (float)(v7 * portal->hullAxis[1][0]) + *v5;
                v5[1] = (float)(v7 * v6[1]) + v5[1];
                v5[2] = (float)(v7 * v6[2]) + v5[2];
            }
            if ( r_showPortals->current.integer && !r_portalBevelsOnly->current.enabled )
                R_AddDebugPolygon(&frontEndDataOut->debugGlobals, hullPointCount, portalVerts, color_1, 0, color_1, 0);
            if ( childPlanesCount + 16 > 0x800 )
            {
                R_WarnOncePerFrame(R_WARN_PORTAL_PLANES);
                Sys_WaitWorkerCmdInternal(&r_dpvs_staticWorkerCmd);
                Sys_WaitWorkerCmdInternal(&r_dpvs_dynmodelWorkerCmd);
                Sys_WaitWorkerCmdInternal(&r_dpvs_sceneentWorkerCmd);
                Sys_WaitWorkerCmdInternal(&r_dpvs_entityWorkerCmd);
                Sys_WaitWorkerCmdInternal(&r_dpvs_dynbrushWorkerCmd);
                childPlanesCount = 0;
            }
            childPlanes = &dpvsGlob.childPlanes[childPlanesCount];
            childPlaneCount = R_PortalClipPlanes(childPlanes, hullPointCount, portalVerts, &clipChildren);
            if ( childPlaneCount > 16
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                            4352,
                            0,
                            "%s",
                            "childPlaneCount <= DPVS_PORTAL_MAX_PLANES") )
            {
                __debugbreak();
            }
            childPlanesCount += childPlaneCount;
            if ( portal->writable.recursionDepth < r_portalMinRecurseDepth->current.integer )
                clipChildren = DPVS_CLIP_CHILDREN;
            R_VisitPortalsForCell(
                portal->cell,
                portal,
                &portal->plane,
                childPlanes,
                0x10u,
                childPlaneCount,
                0,
                portal->writable.recursionDepth + 1,
                clipChildren);
        }
    }
    //LargeLocal::~LargeLocal(&hullPointsPoolArray_large_local);
}

unsigned int __cdecl R_PortalClipPlanes(
                DpvsPlane *planes,
                unsigned int vertexCount,
                const float (*winding)[3],
                DpvsClipChildren *clipChildren)
{
    DpvsPlane *v4; // eax
    bool v6; // [esp+0h] [ebp-648h]
    DpvsForceBevels v7; // [esp+4h] [ebp-644h]
    DpvsPlane *plane; // [esp+8h] [ebp-640h]
    DpvsPlane *v9; // [esp+14h] [ebp-634h]
    float *v10; // [esp+18h] [ebp-630h]
    float normals[128][3]; // [esp+20h] [ebp-628h] BYREF
    unsigned int windingVertIndex; // [esp+620h] [ebp-28h]
    unsigned int planeCount; // [esp+624h] [ebp-24h]
    float clipSpaceMins[2]; // [esp+628h] [ebp-20h] BYREF
    float clipSpaceMaxs[2]; // [esp+630h] [ebp-18h] BYREF
    bool useNormalPlanes; // [esp+63Bh] [ebp-Dh]
    float distMin; // [esp+63Ch] [ebp-Ch]
    DpvsForceBevels forceBevels; // [esp+640h] [ebp-8h]
    bool useBevelPlanes; // [esp+647h] [ebp-1h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 740, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( vertexCount < 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    741,
                    0,
                    "%s\n\t(vertexCount) = %i",
                    "(vertexCount >= 3)",
                    vertexCount) )
    {
        __debugbreak();
    }
    useNormalPlanes = vertexCount <= 0xA;
    v7 = (DpvsForceBevels)(vertexCount > 0xA || r_portalBevelsOnly->current.enabled);
    forceBevels = v7;
    v6 = v7 == DPVS_FORCE_BEVELS || r_portalBevels->current.value > 0.0;
    useBevelPlanes = v6;
    R_GetSidePlaneNormals(winding, vertexCount, normals);
    planeCount = 0;
    if ( useBevelPlanes || r_portalMinClipArea->current.value > 0.0 )
    {
        R_ProjectPortal(vertexCount, winding, clipSpaceMins, clipSpaceMaxs, clipChildren);
        if ( useBevelPlanes )
            planeCount = R_AddBevelPlanes(planes, vertexCount, normals, clipSpaceMins, clipSpaceMaxs, forceBevels);
    }
    else
    {
        *clipChildren = DPVS_CLIP_CHILDREN;
    }
    if ( useNormalPlanes )
    {
        for ( windingVertIndex = 0; windingVertIndex < vertexCount; ++windingVertIndex )
        {
            if ( (float)((float)((float)(normals[windingVertIndex][0] * normals[windingVertIndex][0])
                                                 + (float)(normals[windingVertIndex][1] * normals[windingVertIndex][1]))
                                 + (float)(normals[windingVertIndex][2] * normals[windingVertIndex][2])) != 0.0 )
            {
                v9 = &planes[planeCount];
                v10 = normals[windingVertIndex];
                v9->coeffs[0] = *v10;
                v9->coeffs[1] = v10[1];
                v9->coeffs[2] = v10[2];
                plane = &planes[planeCount];
                plane->coeffs[3] = 0.001
                                                 - (float)((float)((float)(plane->coeffs[0] * (float)(*winding)[3 * windingVertIndex])
                                                                                 + (float)(plane->coeffs[1] * (float)(*winding)[3 * windingVertIndex + 1]))
                                                                 + (float)(plane->coeffs[2] * (float)(*winding)[3 * windingVertIndex + 2]));
                R_SetDpvsPlaneSides(plane);
                ++planeCount;
            }
        }
    }
    v4 = &planes[planeCount];
    v4->coeffs[0] = dpvsGlob.nearPlane.coeffs[0];
    v4->coeffs[1] = dpvsGlob.nearPlane.coeffs[1];
    v4->coeffs[2] = dpvsGlob.nearPlane.coeffs[2];
    v4->coeffs[3] = dpvsGlob.nearPlane.coeffs[3];
    *(unsigned int *)v4->side = *(unsigned int *)dpvsGlob.nearPlane.side;
    distMin = R_NearestPointOnWinding(winding, vertexCount, &planes[planeCount]);
    if ( distMin > 0.0 )
        planes[planeCount].coeffs[3] = planes[planeCount].coeffs[3] - distMin;
    ++planeCount;
    if ( dpvsGlob.farPlaneEnabled )
        planes[planeCount++] = dpvsGlob.farPlane;
    return planeCount;
}

double __cdecl R_NearestPointOnWinding(const float (*points)[3], int pointCount, const DpvsPlane *plane)
{
    float distMin; // [esp+24h] [ebp-Ch]
    int pointIndex; // [esp+28h] [ebp-8h]
    int pointIndexa; // [esp+28h] [ebp-8h]

    if ( (float)((float)((float)((float)(plane->coeffs[0] * (float)(*points)[3 * pointCount - 3])
                                                         + (float)(plane->coeffs[1] * (float)(*points)[3 * pointCount - 2]))
                                         + (float)(plane->coeffs[2] * (float)(*points)[3 * pointCount - 1]))
                         + plane->coeffs[3]) <= (float)((float)((float)((float)(plane->coeffs[0] * (*points)[0])
                                                                                                                    + (float)(plane->coeffs[1] * (float)(*points)[1]))
                                                                                                    + (float)(plane->coeffs[2] * (float)(*points)[2]))
                                                                                    + plane->coeffs[3]) )
    {
        distMin = (float)((float)((float)(plane->coeffs[0] * (float)(*points)[3 * pointCount - 3])
                                                        + (float)(plane->coeffs[1] * (float)(*points)[3 * pointCount - 2]))
                                        + (float)(plane->coeffs[2] * (float)(*points)[3 * pointCount - 1]))
                        + plane->coeffs[3];
        for ( pointIndexa = pointCount - 2;
                    pointIndexa > 0
             && (float)((float)((float)((float)(plane->coeffs[0] * (float)(*points)[3 * pointIndexa])
                                                                + (float)(plane->coeffs[1] * (float)(*points)[3 * pointIndexa + 1]))
                                                + (float)(plane->coeffs[2] * (float)(*points)[3 * pointIndexa + 2]))
                                + plane->coeffs[3]) <= distMin;
                    --pointIndexa )
        {
            distMin = (float)((float)((float)(plane->coeffs[0] * (float)(*points)[3 * pointIndexa])
                                                            + (float)(plane->coeffs[1] * (float)(*points)[3 * pointIndexa + 1]))
                                            + (float)(plane->coeffs[2] * (float)(*points)[3 * pointIndexa + 2]))
                            + plane->coeffs[3];
        }
    }
    else
    {
        distMin = (float)((float)((float)(plane->coeffs[0] * (*points)[0]) + (float)(plane->coeffs[1] * (float)(*points)[1]))
                                        + (float)(plane->coeffs[2] * (float)(*points)[2]))
                        + plane->coeffs[3];
        for ( pointIndex = 1;
                    pointIndex < pointCount - 1
             && (float)((float)((float)((float)(plane->coeffs[0] * (float)(*points)[3 * pointIndex])
                                                                + (float)(plane->coeffs[1] * (float)(*points)[3 * pointIndex + 1]))
                                                + (float)(plane->coeffs[2] * (float)(*points)[3 * pointIndex + 2]))
                                + plane->coeffs[3]) <= distMin;
                    ++pointIndex )
        {
            distMin = (float)((float)((float)(plane->coeffs[0] * (float)(*points)[3 * pointIndex])
                                                            + (float)(plane->coeffs[1] * (float)(*points)[3 * pointIndex + 1]))
                                            + (float)(plane->coeffs[2] * (float)(*points)[3 * pointIndex + 2]))
                            + plane->coeffs[3];
        }
    }
    return distMin;
}

void __cdecl R_ProjectPortal(
                int vertexCount,
                const float (*winding)[3],
                float *mins,
                float *maxs,
                DpvsClipChildren *clipChildren)
{
    int windingVertIndex; // [esp+40h] [ebp-440h]
    int windingVertIndexa; // [esp+40h] [ebp-440h]
    float area; // [esp+44h] [ebp-43Ch]
    const float *xyz; // [esp+48h] [ebp-438h]
    float x; // [esp+50h] [ebp-430h]
    float y; // [esp+54h] [ebp-42Ch]
    float screenSpaceWinding[132][2]; // [esp+58h] [ebp-428h] BYREF

    iassert(vertexCount >= 3);

    mins[0] = 1.0f;
    mins[1] = 1.0f;
    maxs[0] = -1.0f;
    maxs[1] = -1.0f;

    for ( windingVertIndex = 0; windingVertIndex < vertexCount; ++windingVertIndex )
    {
        xyz = &(*winding)[3 * windingVertIndex];

        float w = xyz[0] * dpvsGlob.viewProjMtx->m[0][3]
                + xyz[1] * dpvsGlob.viewProjMtx->m[1][3]
                + xyz[2] * dpvsGlob.viewProjMtx->m[2][3]
                + dpvsGlob.viewProjMtx->m[3][3];

        if ( w < 0.125 )
        {
            mins[0] = -1.0f;
            mins[1] = -1.0f;
            maxs[0] = 1.0f;
            maxs[1] = 1.0f;
            *clipChildren = DPVS_CLIP_CHILDREN;
            return;
        }
        x = (((*xyz * dpvsGlob.viewProjMtx->m[0][0]) + (xyz[1] * dpvsGlob.viewProjMtx->m[1][0])) + (xyz[2] * dpvsGlob.viewProjMtx->m[2][0])) + dpvsGlob.viewProjMtx->m[3][0];
        y = (((*xyz * dpvsGlob.viewProjMtx->m[0][1]) + (xyz[1] * dpvsGlob.viewProjMtx->m[1][1])) + (xyz[2] * dpvsGlob.viewProjMtx->m[2][1])) + dpvsGlob.viewProjMtx->m[3][1];
        
        float invW = 1.0 / w;
        screenSpaceWinding[windingVertIndex][0] = x * (1.0 / w);
        screenSpaceWinding[windingVertIndex][1] = y * invW;

        // Update min/max values
        if (mins[0] > screenSpaceWinding[windingVertIndex][0]) { mins[0] = screenSpaceWinding[windingVertIndex][0]; }
        if (maxs[0] < screenSpaceWinding[windingVertIndex][0]) { maxs[0] = screenSpaceWinding[windingVertIndex][0]; }
        if (mins[1] > screenSpaceWinding[windingVertIndex][1]) { mins[1] = screenSpaceWinding[windingVertIndex][1]; }
        if (maxs[1] < screenSpaceWinding[windingVertIndex][1]) { maxs[1] = screenSpaceWinding[windingVertIndex][1]; }
    }

    area = ((maxs[0] - mins[0]) * (maxs[1] - mins[1])) * 0.25;
    iassert(area >= 0);

    // Portal is too small, who cares
    if (area < r_portalMinClipArea->current.value)
    {
        *clipChildren = DPVS_DONT_CLIP_CHILDREN;
        return;
    }

    screenSpaceWinding[vertexCount    ][0] = screenSpaceWinding[0][0];
    screenSpaceWinding[vertexCount    ][1] = screenSpaceWinding[0][1];
    screenSpaceWinding[vertexCount + 1][0] = screenSpaceWinding[1][0];
    screenSpaceWinding[vertexCount + 1][1] = screenSpaceWinding[1][1];

    area = 0.0f;
    for (int i = 1; i <= vertexCount; ++i)
        area += (screenSpaceWinding[i + 1][1] - screenSpaceWinding[i - 1][1]) * screenSpaceWinding[i][0];

    //for ( windingVertIndexa = 1; windingVertIndexa <= vertexCount; ++windingVertIndexa )
    //    area = ((screenSpaceWinding[windingVertIndexa + 1][1] - *(&y + 2 * windingVertIndexa)) * screenSpaceWinding[windingVertIndexa][0]) + area;

    area *= 0.125;
    iassert(area >= 0);
        
    *clipChildren = (DpvsClipChildren)(r_portalMinClipArea->current.value <= area);
}

char *__cdecl R_PortalAssertMsg()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax

    v0 = va("\torg %.8gf, %.8gf, %.8gf\n", dpvsGlob.viewOrg[0], dpvsGlob.viewOrg[1], dpvsGlob.viewOrg[2]);
    v1 = va(
                 "%s\tplane %.8gf, %.8gf, %.8gf, %.8gf\n",
                 v0,
                 dpvsGlob.nearPlane.coeffs[0],
                 dpvsGlob.nearPlane.coeffs[1],
                 dpvsGlob.nearPlane.coeffs[2],
                 dpvsGlob.nearPlane.coeffs[3]);
    v2 = va(
                 "%s\t%.8gf, %.8gf, %.8gf, %.8gf\n",
                 v1,
                 dpvsGlob.viewProjMtx->m[0][0],
                 dpvsGlob.viewProjMtx->m[0][1],
                 dpvsGlob.viewProjMtx->m[0][2],
                 dpvsGlob.viewProjMtx->m[0][3]);
    v3 = va(
                 "%s\t%.8gf, %.8gf, %.8gf, %.8gf\n",
                 v2,
                 dpvsGlob.viewProjMtx->m[1][0],
                 dpvsGlob.viewProjMtx->m[1][1],
                 dpvsGlob.viewProjMtx->m[1][2],
                 dpvsGlob.viewProjMtx->m[1][3]);
    v4 = va(
                 "%s\t%.8gf, %.8gf, %.8gf, %.8gf\n",
                 v3,
                 dpvsGlob.viewProjMtx->m[2][0],
                 dpvsGlob.viewProjMtx->m[2][1],
                 dpvsGlob.viewProjMtx->m[2][2],
                 dpvsGlob.viewProjMtx->m[2][3]);
    return va(
                     "%s\t%.8gf, %.8gf, %.8gf, %.8gf\n",
                     v4,
                     dpvsGlob.viewProjMtx->m[3][0],
                     dpvsGlob.viewProjMtx->m[3][1],
                     dpvsGlob.viewProjMtx->m[3][2],
                     dpvsGlob.viewProjMtx->m[3][3]);
}

unsigned int __cdecl R_AddBevelPlanes(
                DpvsPlane *planes,
                unsigned int vertexCount,
                const float (*windingNormals)[3],
                float *mins,
                float *maxs,
                DpvsForceBevels forceBevels)
{
    float v7; // [esp+0h] [ebp-BCh]
    float v8; // [esp+4h] [ebp-B8h]
    DpvsPlane *plane; // [esp+8h] [ebp-B4h]
    DpvsPlane *v10; // [esp+1Ch] [ebp-A0h]
    float *v11; // [esp+20h] [ebp-9Ch]
    unsigned int windingVertIndex; // [esp+28h] [ebp-94h]
    float projected[2]; // [esp+2Ch] [ebp-90h] BYREF
    float bevelVerts[5][3]; // [esp+34h] [ebp-88h] BYREF
    unsigned int planeCount; // [esp+70h] [ebp-4Ch]
    float bevelNormals[4][3]; // [esp+74h] [ebp-48h] BYREF
    float invW; // [esp+A4h] [ebp-18h]
    unsigned int bevelVertIndex; // [esp+A8h] [ebp-14h]
    float unprojected[4]; // [esp+ACh] [ebp-10h] BYREF

    for ( bevelVertIndex = 0; bevelVertIndex < 4; ++bevelVertIndex )
    {
        if ( bevelVertIndex >= 2 )
            v8 = *maxs;
        else
            v8 = *mins;
        projected[0] = v8;
        if ( bevelVertIndex == 1 || bevelVertIndex == 2 )
            v7 = mins[1];
        else
            v7 = maxs[1];
        projected[1] = v7;
        R_UnprojectPoint(projected, unprojected);
        invW = 1.0 / unprojected[3];
        bevelVerts[bevelVertIndex][0] = unprojected[0] * (float)(1.0 / unprojected[3]);
        bevelVerts[bevelVertIndex][1] = unprojected[1] * invW;
        bevelVerts[bevelVertIndex][2] = unprojected[2] * invW;
    }
    *(_QWORD *)&bevelVerts[4][0] = *(_QWORD *)&bevelVerts[0][0];
    bevelVerts[4][2] = bevelVerts[0][2];
    R_GetSidePlaneNormals(bevelVerts, 4u, bevelNormals);
    planeCount = 0;
    for ( bevelVertIndex = 0; bevelVertIndex < 4; ++bevelVertIndex )
    {
        v10 = &planes[planeCount];
        v11 = bevelNormals[bevelVertIndex];
        v10->coeffs[0] = *v11;
        v10->coeffs[1] = v11[1];
        v10->coeffs[2] = v11[2];
        if ( forceBevels == DPVS_DONT_FORCE_BEVELS )
        {
            for ( windingVertIndex = 0; windingVertIndex < vertexCount; ++windingVertIndex )
            {
                if ( (float)((float)((float)((float)(*windingNormals)[3 * windingVertIndex] * planes[planeCount].coeffs[0])
                                                     + (float)((float)(*windingNormals)[3 * windingVertIndex + 1] * planes[planeCount].coeffs[1]))
                                     + (float)((float)(*windingNormals)[3 * windingVertIndex + 2] * planes[planeCount].coeffs[2])) > r_portalBevels->current.value )
                {
                    if ( (r_showPortals->current.integer & 2) != 0 )
                        R_AddDebugLine(
                            &frontEndDataOut->debugGlobals,
                            bevelVerts[bevelVertIndex],
                            bevelVerts[bevelVertIndex + 1],
                            colorMdCyan,
                            0);
                    goto LABEL_12;
                }
            }
        }
        if ( r_showPortals->current.integer )
            R_AddDebugLine(
                &frontEndDataOut->debugGlobals,
                bevelVerts[bevelVertIndex],
                bevelVerts[bevelVertIndex + 1],
                colorLtCyan,
                0);
        plane = &planes[planeCount];
        plane->coeffs[3] = 0.001
                                         - (float)((float)((float)(plane->coeffs[0] * bevelVerts[bevelVertIndex][0])
                                                                         + (float)(plane->coeffs[1] * bevelVerts[bevelVertIndex][1]))
                                                         + (float)(plane->coeffs[2] * bevelVerts[bevelVertIndex][2]));
        R_SetDpvsPlaneSides(plane);
        ++planeCount;
LABEL_12:
        ;
    }
    return planeCount;
}

void __cdecl R_UnprojectPoint(const float *projected, float *unprojected)
{
    *unprojected = (float)((float)(dpvsGlob.invViewProjMtx->m[0][0] * *projected)
                                             + (float)(dpvsGlob.invViewProjMtx->m[1][0] * projected[1]))
                             + dpvsGlob.invViewProjMtx->m[3][0];
    unprojected[1] = (float)((float)(dpvsGlob.invViewProjMtx->m[0][1] * *projected)
                                                 + (float)(dpvsGlob.invViewProjMtx->m[1][1] * projected[1]))
                                 + dpvsGlob.invViewProjMtx->m[3][1];
    unprojected[2] = (float)((float)(dpvsGlob.invViewProjMtx->m[0][2] * *projected)
                                                 + (float)(dpvsGlob.invViewProjMtx->m[1][2] * projected[1]))
                                 + dpvsGlob.invViewProjMtx->m[3][2];
    unprojected[3] = (float)((float)(dpvsGlob.invViewProjMtx->m[0][3] * *projected)
                                                 + (float)(dpvsGlob.invViewProjMtx->m[1][3] * projected[1]))
                                 + dpvsGlob.invViewProjMtx->m[3][3];
}

void __cdecl R_VisitPortalsForCell(
                const GfxCell *cell,
                GfxPortal *parentPortal,
                const DpvsPlane *parentPlane,
                DpvsPlane *planes,
                unsigned int maxPlaneCount,
                unsigned int planeCount,
                int frustumPlaneCount,
                signed int recursionDepth,
                DpvsClipChildren clipChildren)
{
    unsigned __int8 v9; // [esp+0h] [ebp-24h]
    int vertCount; // [esp+8h] [ebp-1Ch]
    const float *verts; // [esp+Ch] [ebp-18h]
    int xCoord; // [esp+10h] [ebp-14h] BYREF
    int yCoord; // [esp+14h] [ebp-10h] BYREF
    GfxPortal *portal; // [esp+1Ch] [ebp-8h]
    int portalIndex; // [esp+20h] [ebp-4h]

    if ( !R_CellIsForcedInvisible(cell) )
    {
        R_AddCellSurfacesAndCullGroupsInFrustumDelayed(cell, planes, planeCount, frustumPlaneCount);
        R_SetCellVisible(cell);
    }
    R_SetAncestorListStatus(parentPortal, 1);
    if ( clipChildren == DPVS_DONT_CLIP_CHILDREN )
    {
        R_VisitAllFurtherCells(cell, parentPlane, planes, planeCount, frustumPlaneCount);
        R_SetAncestorListStatus(parentPortal, 0);
        return;
    }
    if ( clipChildren != DPVS_CLIP_CHILDREN
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    4134,
                    0,
                    "%s\n\t(clipChildren) = %i",
                    "(clipChildren == DPVS_CLIP_CHILDREN)",
                    clipChildren) )
    {
        __debugbreak();
    }
    for ( portalIndex = 0; portalIndex < cell->portalCount; ++portalIndex )
    {
        portal = &cell->portals[portalIndex];
        if ( R_ShouldSkipPortal(portal, planes, planeCount) )
            continue;
        if ( dpvsGlob.viewOrgIsDir
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                        4148,
                        0,
                        "%s",
                        "!dpvsGlob.viewOrgIsDir") )
        {
            __debugbreak();
        }
        if ( R_DpvsPlaneDistToEye(&portal->plane) <= -0.125 )
        {
            if ( R_ChopPortalAndAddHullPoints(portal, parentPlane, planes, planeCount) )
            {
                if ( portal->writable.isQueued )
                {
                    if ( portal->writable.recursionDepth < recursionDepth )
                        v9 = portal->writable.recursionDepth;
                    else
                        v9 = recursionDepth;
                    portal->writable.recursionDepth = v9;
                    if ( portal->writable.queuedParent != parentPortal )
                        portal->writable.queuedParent = 0;
                }
                else
                {
                    portal->writable.recursionDepth = recursionDepth;
                    portal->writable.queuedParent = parentPortal;
                    R_EnqueuePortal(portal);
                }
            }
        }
        else
        {
            if ( portal->writable.isAncestor
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                            4154,
                            1,
                            "%s",
                            "!portal->writable.isAncestor") )
            {
                __debugbreak();
            }
            if ( portal->writable.isQueued
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                            4155,
                            1,
                            "%s",
                            "!portal->writable.isQueued") )
            {
                __debugbreak();
            }
            vertCount = portal->vertexCount;
            verts = (const float *)portal->vertices;
            Vec3ProjectionCoords(portal->plane.coeffs, &xCoord, &yCoord);
            if ( ProjectedWindingContainsCoplanarPoint((const float (*)[3])verts, vertCount, xCoord, yCoord, dpvsGlob.viewOrg) )
            {
                portal->writable.queuedParent = 0;
                if ( planeCount >= maxPlaneCount )
                    continue;
                planes[planeCount++] = *parentPlane;
                R_VisitPortalsForCell(
                    portal->cell,
                    portal,
                    &portal->plane,
                    planes,
                    maxPlaneCount,
                    planeCount,
                    frustumPlaneCount,
                    portal->writable.recursionDepth + 1,
                    clipChildren);
            }
            if ( portal->writable.isAncestor
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                            4174,
                            1,
                            "%s",
                            "!portal->writable.isAncestor") )
            {
                __debugbreak();
            }
            if ( parentPortal
                && !parentPortal->writable.isAncestor
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                            4176,
                            1,
                            "%s",
                            "parentPortal == NULL || parentPortal->writable.isAncestor") )
            {
                __debugbreak();
            }
        }
    }
    R_SetAncestorListStatus(parentPortal, 0);
}

char __cdecl R_ChopPortalAndAddHullPoints(
                GfxPortal *portal,
                const DpvsPlane *parentPlane,
                const DpvsPlane *planes,
                int planeCount)
{
    int vertCount; // [esp+0h] [ebp-C10h]
    int vertIndex; // [esp+4h] [ebp-C0Ch]
    float v[2][128][3]; // [esp+8h] [ebp-C08h] BYREF
    const float (*w)[3]; // [esp+C0Ch] [ebp-4h] BYREF

    vertCount = R_ChopPortal(portal, parentPlane, planes, planeCount, v, &w);
    if ( !vertCount )
        return 0;
    for ( vertIndex = 0; vertIndex < vertCount; ++vertIndex )
        R_AddVertToPortalHullPoints(portal, w[vertIndex]);
    return 1;
}

void __cdecl R_VisitAllFurtherCells(
                const GfxCell *cell,
                const DpvsPlane *parentPlane,
                const DpvsPlane *planes,
                int planeCount,
                unsigned __int8 frustumPlaneCount)
{
    int i; // [esp+0h] [ebp-1C0Ch]
    GfxCell *list[1025]; // [esp+4h] [ebp-1C08h] BYREF
    int FurtherCellList_r; // [esp+1008h] [ebp-C04h]
    float v[256][3]; // [esp+100Ch] [ebp-C00h] BYREF

    FurtherCellList_r = R_GetFurtherCellList_r(
                                                cell,
                                                parentPlane,
                                                planes,
                                                planeCount,
                                                (float (*)[128][3])v,
                                                (const GfxCell **)list,
                                                0);
    for ( i = 0; i < FurtherCellList_r; ++i )
    {
        if ( !R_CellIsForcedInvisible(list[i]) )
        {
            R_AddCellSurfacesAndCullGroupsInFrustumDelayed(list[i], planes, planeCount, frustumPlaneCount);
            R_SetCellVisible(list[i]);
        }
    }
}

int __cdecl R_GetFurtherCellList_r(
                const GfxCell *cell,
                const DpvsPlane *parentPlane,
                const DpvsPlane *planes,
                int planeCount,
                float (*v)[128][3],
                const GfxCell **list,
                int count)
{
    int v7; // eax
    const GfxPortal *portal; // [esp+0h] [ebp-8h]
    int portalIndex; // [esp+4h] [ebp-4h]

    for ( portalIndex = 0; portalIndex < cell->portalCount; ++portalIndex )
    {
        portal = &cell->portals[portalIndex];
        if ( !R_IsCellInList(portal->cell, list, count) && !R_ShouldSkipPortal(portal, planes, planeCount) )
        {
            if ( R_ChopPortal(portal, parentPlane, planes, planeCount, v, 0) )
            {
                v7 = R_AddCellToList(portal->cell, list, count);
                count = R_GetFurtherCellList_r(portal->cell, parentPlane, planes, planeCount, v, list, v7);
            }
        }
    }
    return count;
}

char __cdecl R_IsCellInList(const GfxCell *cell, const GfxCell **list, int count)
{
    int index; // [esp+0h] [ebp-4h]

    for ( index = 0; index < count; ++index )
    {
        if ( cell == list[index] )
            return 1;
    }
    return 0;
}

int __cdecl R_AddCellToList(const GfxCell *cell, const GfxCell **list, int count)
{
    list[count] = cell;
    return count + 1;
}

void __cdecl R_SetupShadowSurfacesDpvs(
                const GfxViewParms *viewParms,
                const float (*sidePlanes)[4],
                unsigned int sidePlaneCount,
                int partitionIndex)
{
    DpvsPlane *v4; // edx
    DpvsPlane *v5; // eax
    DpvsPlane *v6; // eax
    DpvsPlane *v7; // ecx
    DpvsPlane *v8; // edx
    DpvsPlane *v9; // eax
    DpvsView *dpvsView; // [esp+18h] [ebp-8h]
    unsigned int planeIndex; // [esp+1Ch] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 5597, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 5598, 0, "%s", "rgp.world") )
    {
        __debugbreak();
    }
    if ( !viewParms
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp", 5599, 0, "%s", "viewParms") )
    {
        __debugbreak();
    }
    dpvsView = &dpvsGlob.views[scene.dpvs.localClientNum][partitionIndex + 1];
    dpvsView->renderFxFlagsCull = 1;
    if ( sidePlaneCount > 0xE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    5609,
                    0,
                    "%s\n\t(sidePlaneCount) = %i",
                    "(sidePlaneCount <= (sizeof( dpvsView->frustumPlanes ) / (sizeof( dpvsView->frustumPlanes[0] ) * (sizeof( dpvsV"
                    "iew->frustumPlanes ) != 4 || sizeof( dpvsView->frustumPlanes[0] ) <= 4))))",
                    sidePlaneCount) )
    {
        __debugbreak();
    }
    R_FrustumClipPlanes(&viewParms->viewProjectionMatrix, sidePlanes, sidePlaneCount, dpvsView->frustumPlanes);
    if ( (float)((float)((float)(viewParms->axis[0][0] * scene.shadowNearPlane[partitionIndex].coeffs[0])
                                         + (float)(viewParms->axis[0][1] * scene.shadowNearPlane[partitionIndex].coeffs[1]))
                         + (float)(viewParms->axis[0][2] * scene.shadowNearPlane[partitionIndex].coeffs[2])) < 0.0 )
    {
        if ( sidePlaneCount >= 0xE
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                        5620,
                        0,
                        "%s\n\t(sidePlaneCount) = %i",
                        "(sidePlaneCount < (sizeof( dpvsView->frustumPlanes ) / (sizeof( dpvsView->frustumPlanes[0] ) * (sizeof( dpvs"
                        "View->frustumPlanes ) != 4 || sizeof( dpvsView->frustumPlanes[0] ) <= 4))))",
                        sidePlaneCount) )
        {
            __debugbreak();
        }
        v4 = &scene.shadowNearPlane[partitionIndex];
        v5 = &dpvsView->frustumPlanes[sidePlaneCount];
        v5->coeffs[0] = v4->coeffs[0];
        v5->coeffs[1] = v4->coeffs[1];
        v5->coeffs[2] = v4->coeffs[2];
        v5->coeffs[3] = v4->coeffs[3];
        *(unsigned int *)v5->side = *(unsigned int *)v4->side;
        ++sidePlaneCount;
    }
    if ( (!partitionIndex || !rg.sunShadowFull)
        && (float)((float)((float)(viewParms->axis[0][0] * scene.shadowFarPlane[partitionIndex].coeffs[0])
                                         + (float)(viewParms->axis[0][1] * scene.shadowFarPlane[partitionIndex].coeffs[1]))
                         + (float)(viewParms->axis[0][2] * scene.shadowFarPlane[partitionIndex].coeffs[2])) < 0.0 )
    {
        if ( sidePlaneCount >= 0xE
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                        5631,
                        0,
                        "%s\n\t(sidePlaneCount) = %i",
                        "(sidePlaneCount < (sizeof( dpvsView->frustumPlanes ) / (sizeof( dpvsView->frustumPlanes[0] ) * (sizeof( dpvs"
                        "View->frustumPlanes ) != 4 || sizeof( dpvsView->frustumPlanes[0] ) <= 4))))",
                        sidePlaneCount) )
        {
            __debugbreak();
        }
        v6 = &scene.shadowFarPlane[partitionIndex];
        v7 = &dpvsView->frustumPlanes[sidePlaneCount];
        v7->coeffs[0] = v6->coeffs[0];
        v7->coeffs[1] = v6->coeffs[1];
        v7->coeffs[2] = v6->coeffs[2];
        v7->coeffs[3] = v6->coeffs[3];
        *(unsigned int *)v7->side = *(unsigned int *)v6->side;
        ++sidePlaneCount;
    }
    for ( planeIndex = 0; planeIndex < 4; ++planeIndex )
    {
        if ( (float)((float)((float)(viewParms->axis[0][0] * dpvsGlob.sideFrustumPlanes[planeIndex].coeffs[0])
                                             + (float)(viewParms->axis[0][1] * dpvsGlob.sideFrustumPlanes[planeIndex].coeffs[1]))
                             + (float)(viewParms->axis[0][2] * dpvsGlob.sideFrustumPlanes[planeIndex].coeffs[2])) < 0.0 )
        {
            if ( sidePlaneCount >= 0xE
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                            5642,
                            0,
                            "%s\n\t(sidePlaneCount) = %i",
                            "(sidePlaneCount < (sizeof( dpvsView->frustumPlanes ) / (sizeof( dpvsView->frustumPlanes[0] ) * (sizeof( dp"
                            "vsView->frustumPlanes ) != 4 || sizeof( dpvsView->frustumPlanes[0] ) <= 4))))",
                            sidePlaneCount) )
            {
                __debugbreak();
            }
            v8 = &dpvsGlob.sideFrustumPlanes[planeIndex];
            v9 = &dpvsView->frustumPlanes[sidePlaneCount];
            v9->coeffs[0] = v8->coeffs[0];
            v9->coeffs[1] = v8->coeffs[1];
            v9->coeffs[2] = v8->coeffs[2];
            v9->coeffs[3] = v8->coeffs[3];
            *(unsigned int *)v9->side = *(unsigned int *)v8->side;
            ++sidePlaneCount;
        }
    }
    dpvsView->frustumPlaneCount = sidePlaneCount;
}

double __cdecl R_GetFarPlaneDist()
{
    if ( r_zfar->current.value == 0.0 )
        return dpvsGlob.cullDist;
    else
        return r_zfar->current.value;
}

void __cdecl R_SetCullDist(float dist)
{
    if ( dist > 0.0 )
        dpvsGlob.cullDist = dist;
    else
        dpvsGlob.cullDist = 0.0f;
}

int __cdecl R_CullBoxCurDpvs_SceneSelect(const float *minmax, unsigned int viewIndex, unsigned int sceneIndex)
{
    int frustumPlaneCount; // [esp+8h] [ebp-14h]
    const DpvsPlane *plane; // [esp+10h] [ebp-Ch]
    int v7; // [esp+14h] [ebp-8h]

    if ( viewIndex >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    5689,
                    0,
                    "%s",
                    "viewIndex < GFX_MAX_CLIENT_VIEWS") )
    {
        __debugbreak();
    }
    if ( sceneIndex >= 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    5690,
                    0,
                    "%s",
                    "sceneIndex < SCENE_VIEW_COUNT") )
    {
        __debugbreak();
    }
    frustumPlaneCount = dpvsGlob.views[viewIndex][sceneIndex].frustumPlaneCount;
    v7 = 0;
    plane = dpvsGlob.views[viewIndex][sceneIndex].frustumPlanes;
    while ( v7 < frustumPlaneCount )
    {
        if ( R_DpvsPlaneMaxSignedDistToBox(plane, minmax) <= 0.0 )
            return 1;
        ++v7;
        ++plane;
    }
    return 0;
}

int __cdecl R_CullBoxCurDpvs(const float *minmax, unsigned int viewIndex)
{
    int frustumPlaneCount; // [esp+8h] [ebp-14h]
    const DpvsPlane *plane; // [esp+10h] [ebp-Ch]
    int v6; // [esp+14h] [ebp-8h]

    if ( viewIndex >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    5701,
                    0,
                    "%s",
                    "viewIndex < GFX_MAX_CLIENT_VIEWS") )
    {
        __debugbreak();
    }
    frustumPlaneCount = dpvsGlob.views[viewIndex][0].frustumPlaneCount;
    v6 = 0;
    plane = dpvsGlob.views[viewIndex][0].frustumPlanes;
    while ( v6 < frustumPlaneCount )
    {
        if ( R_DpvsPlaneMaxSignedDistToBox(plane, minmax) <= 0.0 )
            return 1;
        ++v6;
        ++plane;
    }
    return 0;
}

int __cdecl R_CullPoint(int localClient, const float *p0, float cutoff)
{
    DpvsPlane *plane; // [esp+4h] [ebp-Ch]
    int pi; // [esp+Ch] [ebp-4h]

    pi = 0;
    plane = dpvsGlob.views[localClient][0].frustumPlanes;
    while ( pi < dpvsGlob.views[localClient][0].frustumPlaneCount )
    {
        if ( cutoff > (float)((float)((float)((float)(plane->coeffs[0] * *p0) + (float)(plane->coeffs[1] * p0[1]))
                                                                + (float)(plane->coeffs[2] * p0[2]))
                                                + plane->coeffs[3]) )
            return 1;
        ++pi;
        ++plane;
    }
    return 0;
}

int __cdecl R_CullPoint(const float *point, float cutoff)
{
    int culled; // [esp+8h] [ebp-10h]
    int ci; // [esp+Ch] [ebp-Ch]
    DpvsPlane *plane; // [esp+10h] [ebp-8h]
    int pi; // [esp+14h] [ebp-4h]

    for ( ci = 0; ci < 1; ++ci )
    {
        culled = 0;
        if ( CL_LocalClient_IsActive(ci) )
        {
            pi = 0;
            plane = dpvsGlob.views[ci][0].frustumPlanes;
            while ( pi < dpvsGlob.views[ci][0].frustumPlaneCount )
            {
                if ( cutoff > (float)((float)((float)((float)(plane->coeffs[0] * *point) + (float)(plane->coeffs[1] * point[1]))
                                                                        + (float)(plane->coeffs[2] * point[2]))
                                                        + plane->coeffs[3]) )
                {
                    culled = 1;
                    break;
                }
                ++pi;
                ++plane;
            }
            if ( !culled )
                return 0;
        }
    }
    return 1;
}

int __cdecl R_CullLine(int localClient, const float *p0, const float *p1, float cutoff)
{
    DpvsPlane *plane; // [esp+8h] [ebp-Ch]
    int pi; // [esp+10h] [ebp-4h]

    pi = 0;
    plane = dpvsGlob.views[localClient][0].frustumPlanes;
    while ( pi < dpvsGlob.views[localClient][0].frustumPlaneCount )
    {
        if ( cutoff > (float)((float)((float)((float)(plane->coeffs[0] * *p0) + (float)(plane->coeffs[1] * p0[1]))
                                                                + (float)(plane->coeffs[2] * p0[2]))
                                                + plane->coeffs[3])
            && cutoff > (float)((float)((float)((float)(plane->coeffs[0] * *p1) + (float)(plane->coeffs[1] * p1[1]))
                                                                + (float)(plane->coeffs[2] * p1[2]))
                                                + plane->coeffs[3]) )
        {
            return 1;
        }
        ++pi;
        ++plane;
    }
    return 0;
}

unsigned int __cdecl R_ExtraCam_SaveDpvsData(int localClientNum, unsigned __int8 *buffer, unsigned int bufferByteSize)
{
    if ( bufferByteSize < 0x360
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs.cpp",
                    5791,
                    0,
                    "%s",
                    "bytesNeeded <= bufferByteSize") )
    {
        __debugbreak();
    }
    memcpy(buffer, dpvsGlob.views[localClientNum], 0x360u);
    return 864;
}

void __cdecl R_ExtraCam_RestoreDpvsData(int localClientNum, unsigned __int8 *buffer)
{
    memcpy(dpvsGlob.views[localClientNum], buffer, sizeof(dpvsGlob.views[localClientNum]));
}

void __cdecl R_PerMap_DpvsGlobInit()
{
    memset((unsigned __int8 *)dpvsGlob.cellForceInvisibleBits, 0, sizeof(dpvsGlob.cellForceInvisibleBits));
    dpvsGlob.cullDist = 0.0f;
}
