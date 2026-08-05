#include "r_bsp.h"
#include <universal/com_memory.h>
#include "r_bsp_load_obj.h"
#include "r_dvars.h"
#include "rb_light.h"
#include "r_world_lod.h"
#include "r_foliage.h"
#include "r_sky.h"
#include "r_skybox.h"
#include "r_model_lighting.h"
#include "rb_corona.h"
#include "r_vertexstream2.h"
#include "r_staticmodelcache.h"
#include "r_reflection_probe.h"
#include "r_model.h"

GfxWorld s_world;
int g_pmemLocation;

MaterialUsage *__cdecl R_GetMaterialUsageData(Material *material)
{
    MaterialUsage *materialUsage; // [esp+0h] [ebp-Ch]
    bool exists; // [esp+7h] [ebp-5h] BYREF
    unsigned __int16 hashIndex; // [esp+8h] [ebp-4h] BYREF

    Material_GetHashIndex(material->info.name, &hashIndex, &exists);
    if ( !exists )
        return 0;
    materialUsage = &rg.materialUsage[hashIndex];
    if ( material != rg.materialHashTable[hashIndex]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp.cpp",
                    52,
                    0,
                    "%s",
                    "material == rg.materialHashTable[hashIndex]") )
    {
        __debugbreak();
    }
    materialUsage->material = material;
    return materialUsage;
}

void __cdecl R_CreateMaterialList()
{
    int memory; // [esp+4h] [ebp-10h]
    unsigned __int16 hashIndex; // [esp+8h] [ebp-Ch]
    unsigned __int16 hashIndexa; // [esp+8h] [ebp-Ch]
    MaterialMemory *materialMemory; // [esp+Ch] [ebp-8h]
    int index; // [esp+10h] [ebp-4h]

    s_world.materialMemoryCount = 0;
    for ( hashIndex = 0; hashIndex < 0x1000u; ++hashIndex )
    {
        if ( rg.materialUsage[hashIndex].memory )
            ++s_world.materialMemoryCount;
    }
    if ( s_world.materialMemoryCount )
    {
        s_world.materialMemory = (MaterialMemory *)Hunk_Alloc(8 * s_world.materialMemoryCount, "R_CreateMaterialList", 21);
        index = 0;
        for ( hashIndexa = 0; hashIndexa < 0x1000u; ++hashIndexa )
        {
            memory = rg.materialUsage[hashIndexa].memory;
            if ( memory )
            {
                if ( index >= s_world.materialMemoryCount
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp.cpp",
                                89,
                                0,
                                "%s",
                                "index < s_world.materialMemoryCount") )
                {
                    __debugbreak();
                }
                materialMemory = &s_world.materialMemory[index];
                materialMemory->material = rg.materialUsage[hashIndexa].material;
                materialMemory->memory = memory;
                ++index;
            }
        }
        if ( index != s_world.materialMemoryCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp.cpp",
                        96,
                        0,
                        "%s",
                        "index == s_world.materialMemoryCount") )
        {
            __debugbreak();
        }
    }
}

void __cdecl R_ShutdownMaterialUsage()
{
    VertUsage *vertUsage; // [esp+0h] [ebp-Ch]
    unsigned __int16 hashIndex; // [esp+4h] [ebp-8h]
    VertUsage *nextVertUsage; // [esp+8h] [ebp-4h]

    for ( hashIndex = 0; hashIndex < 0x1000u; ++hashIndex )
    {
        for ( vertUsage = rg.materialUsage[hashIndex].verts; vertUsage; vertUsage = nextVertUsage )
        {
            nextVertUsage = vertUsage->next;
            Z_Free((char *)vertUsage, 0);
        }
        rg.materialUsage[hashIndex].verts = 0;
    }
}

void __cdecl R_InterpretSunLightParseParams(SunLightParseParams *sunParse)
{
    GfxLight *sunLight; // ecx
    float *sunColorFromBsp; // [esp+0h] [ebp-8h]

    if ( !rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp.cpp", 177, 0, "%s", "rgp.world") )
    {
        __debugbreak();
    }
    if ( !rgp.world->sunLight
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp.cpp", 178, 0, "%s", "rgp.world->sunLight") )
    {
        __debugbreak();
    }
    R_InterpretSunLightParseParamsIntoLights(sunParse, rgp.world->sunLight);
    sunColorFromBsp = rgp.world->sunColorFromBsp;
    sunLight = rgp.world->sunLight;
    rgp.world->sunColorFromBsp[0] = sunLight->color[0];
    sunColorFromBsp[1] = sunLight->color[1];
    sunColorFromBsp[2] = sunLight->color[2];
}

void __cdecl R_UpdateLightsFromDvars()
{
    SunLightParseParams sunParse; // [esp+10h] [ebp-C0h] BYREF

    if (r_lightTweakSunDirection->reset.vector[0] != r_lightTweakSunDirection->current.vector[0]
        || r_lightTweakSunDirection->reset.vector[1] != r_lightTweakSunDirection->current.vector[1]
        || r_lightTweakSunDirection->reset.vector[2] != r_lightTweakSunDirection->current.vector[2])
    {
        Dvar_SetFloat((dvar_s *)sm_sunSampleSizeNear, 1.0);
    }
    R_CopyParseParamsFromDvars(&sunParse);
    R_InterpretSunLightParseParams(&sunParse);
}

void __cdecl R_CopyParseParamsFromDvars(SunLightParseParams *sunParse)
{
    memcpy(sunParse, &rgp.world->sunParse, sizeof(SunLightParseParams));
    sunParse->sunSettings[0].sunDiffuseColor[0] = r_lightTweakSunColor->current.value;
    sunParse->sunSettings[0].sunDiffuseColor[1] = r_lightTweakSunColor->current.vector[1];
    sunParse->sunSettings[0].sunDiffuseColor[2] = r_lightTweakSunColor->current.vector[2];
    sunParse->sunSettings[0].sunDiffuseColor[3] = r_lightTweakSunLight->current.value;

    iassert(r_lightTweakSunDirection->current.vector);

    sunParse->sunSettings[0].angles[0] = r_lightTweakSunDirection->current.vector[0];
    sunParse->sunSettings[0].angles[1] = r_lightTweakSunDirection->current.vector[1];
    sunParse->sunSettings[0].angles[2] = r_lightTweakSunDirection->current.vector[2];

}

void __cdecl R_LoadWorld(char *name, int *checksum, int savegame, int location)
{
    unsigned int reflectionProbeIndex; // [esp+10h] [ebp-8h]
    int lightmapIndex; // [esp+14h] [ebp-4h]

    if ( rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp.cpp", 350, 0, "%s", "!rgp.world") )
    {
        __debugbreak();
    }
    R_InitLightVisHistory(name);
    if ( useFastFile->current.enabled )
        R_SetWorldPtr_FastFile(name);
    else
        R_SetWorldPtr_LoadObj(name);
    R_WorldLod_Init();
    R_FoliageInitModels(rgp.world);
    g_worldDpvsPlanes = &rgp.world->dpvsPlanes;
    g_worldDpvs = &rgp.world->dpvs;
    g_worldDraw = &rgp.world->draw;
    memcpy(&g_drawConsts, rgp.world->outdoorLookupMatrix, 0x40u);
    if ( checksum )
        *checksum = rgp.world->checksum;
    R_CopyParseParamsToDvars(&rgp.world->sunParse, savegame);
    R_UpdateLightsFromDvars();
    R_FlushSun();
    R_ClearSkyboxModel();
    R_RegisterSkyboxModel();
    R_InitDynamicData();
    R_ResetModelLighting();
    RB_ResetCoronas();
    RB_SetBspImages();
    DynEntCl_InitFilter();
    R_GenerateShadowMapCasterCells();
    for ( reflectionProbeIndex = 0; reflectionProbeIndex < g_worldDraw->reflectionProbeCount; ++reflectionProbeIndex )
        g_worldDraw->reflectionProbeTextures[reflectionProbeIndex].basemap = g_worldDraw->reflectionProbes[reflectionProbeIndex].reflectionImage->texture.basemap;
    for ( lightmapIndex = 0; lightmapIndex < g_worldDraw->lightmapCount; ++lightmapIndex )
    {
        g_worldDraw->lightmapPrimaryTextures[lightmapIndex].basemap = g_worldDraw->lightmaps[lightmapIndex].primary->texture.basemap;
        g_worldDraw->lightmapSecondaryTextures[lightmapIndex].basemap = g_worldDraw->lightmaps[lightmapIndex].secondary->texture.basemap;
        g_worldDraw->lightmapSecondaryTexturesB[lightmapIndex].basemap = g_worldDraw->lightmaps[lightmapIndex].secondaryB->texture.basemap;
    }
    g_pmemLocation = location;
    R_VertexStream2_Init(g_worldDraw->vertexStream2DataSize);
}

void __cdecl R_CopyParseParamsToDvars(const SunLightParseParams *sunParse, int savegame)
{
    float saveDirection[3];


    iassert(r_lightTweakSunDirection->flags & (1 << 12));

    saveDirection[0] = r_lightTweakSunDirection->current.vector[0];
    saveDirection[1] = r_lightTweakSunDirection->current.vector[1];
    saveDirection[2] = r_lightTweakSunDirection->current.vector[2];
    Dvar_SetFloat((dvar_s *)r_lightTweakSunLight, sunParse->sunSettings[0].sunDiffuseColor[3]);
    Dvar_SetVec3(
        (dvar_s *)r_lightTweakSunColor,
        (sunParse->sunSettings[0].sunDiffuseColor[0]),
        (sunParse->sunSettings[0].sunDiffuseColor[1]),
        sunParse->sunSettings[0].sunDiffuseColor[2]);
    Dvar_SetVec3(
        (dvar_s *)r_lightTweakSunDirection,
        (sunParse->sunSettings[0].angles[0]),
        (sunParse->sunSettings[0].angles[1]),
        sunParse->sunSettings[0].angles[2]);
    Dvar_ChangeResetValue((dvar_s *)r_lightTweakSunLight, r_lightTweakSunLight->current);
    Dvar_ChangeResetValue((dvar_s *)r_lightTweakSunColor, r_lightTweakSunColor->current);
    Dvar_ChangeResetValue((dvar_s *)r_lightTweakSunDirection, r_lightTweakSunDirection->current);
    if (savegame)
        Dvar_SetVec3((dvar_s *)r_lightTweakSunDirection, saveDirection[0], saveDirection[1], saveDirection[2]);

    Dvar_SetFloat((dvar_s *)r_lightTweakAmbient, 0.0);
    Dvar_SetFloat((dvar_s *)r_lightTweakDiffuseFraction, 0.0);
    Dvar_SetColor((dvar_s *)r_lightTweakSunDiffuseColor, 0.0, 0.0, 0.0, 1.0);
    Dvar_ChangeResetValue((dvar_s *)r_lightTweakAmbient, r_lightTweakAmbient->current);
    Dvar_ChangeResetValue((dvar_s *)r_lightTweakDiffuseFraction, r_lightTweakDiffuseFraction->current);
    Dvar_ChangeResetValue((dvar_s *)r_lightTweakAmbientColor, r_lightTweakAmbientColor->current);
    Dvar_ChangeResetValue((dvar_s *)r_lightTweakSunDiffuseColor, r_lightTweakSunDiffuseColor->current);
}

void R_InitDynamicData()
{
    R_InitStaticModelLighting();
}

void __cdecl R_SetWorldPtr_LoadObj(const char *name)
{
    if ( rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp.cpp", 313, 0, "%s", "!rgp.world") )
    {
        __debugbreak();
    }
    rgp.world = R_LoadWorldInternal(name);
    if ( !rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp.cpp", 317, 0, "%s", "rgp.world") )
    {
        __debugbreak();
    }
}

void __cdecl R_SetWorldPtr_FastFile(const char *name)
{
    rgp.world = DB_FindXAssetHeader(ASSET_TYPE_GFXWORLD, (char*)name, 1, -1).gfxWorld;
    rgp.needSortMaterials = 1;
}

void __cdecl R_ShutdownWorld()
{
    R_ReleaseWorld();
    rgp.world = 0;
    g_worldDraw = 0;
    memset(&g_drawConsts, 0, 0x40u);
    if ( s_world.draw.vd.worldVb
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp.cpp",
                    418,
                    0,
                    "%s",
                    "s_world.draw.vd.worldVb == NULL_VERTEX_BUFFER") )
    {
        __debugbreak();
    }
    if ( s_world.draw.vld.layerVb
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp.cpp",
                    419,
                    0,
                    "%s",
                    "s_world.draw.vld.layerVb == NULL_VERTEX_BUFFER") )
    {
        __debugbreak();
    }
    s_world.draw.vertexCount = 0;
    s_world.draw.vertexLayerDataSize = 0;
}

void __cdecl R_ReleaseWorld()
{
    if ( rgp.world )
    {
        R_ResetModelLighting();
        R_FlushStaticModelCache();
    }
    if ( s_world.draw.vertexCount )
    {
        if ( !s_world.draw.vd.worldVb
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp.cpp",
                        442,
                        0,
                        "%s",
                        "s_world.draw.vd.worldVb != NULL_VERTEX_BUFFER") )
        {
            __debugbreak();
        }
        R_FreeStaticVertexBuffer(s_world.draw.vd.worldVb);
        s_world.draw.vd.worldVb = 0;
    }
    if ( s_world.draw.vd.worldVb
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp.cpp",
                    448,
                    0,
                    "%s",
                    "s_world.draw.vd.worldVb == NULL_VERTEX_BUFFER") )
    {
        __debugbreak();
    }
    if ( s_world.draw.vertexLayerDataSize )
    {
        if ( !s_world.draw.vld.layerVb
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp.cpp",
                        452,
                        0,
                        "%s",
                        "s_world.draw.vld.layerVb != NULL_VERTEX_BUFFER") )
        {
            __debugbreak();
        }
        R_FreeStaticVertexBuffer(s_world.draw.vld.layerVb);
        s_world.draw.vld.layerVb = 0;
    }
    if ( s_world.draw.vld.layerVb
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp.cpp",
                    458,
                    0,
                    "%s",
                    "s_world.draw.vld.layerVb == NULL_VERTEX_BUFFER") )
    {
        __debugbreak();
    }
    R_VertexStream2_Shutdown();
}

void __cdecl R_ReloadWorld()
{
    if ( s_world.draw.vd.worldVb
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp.cpp",
                    470,
                    0,
                    "%s",
                    "s_world.draw.vd.worldVb == NULL_VERTEX_BUFFER") )
    {
        __debugbreak();
    }
    if ( s_world.draw.vertexCount )
        R_CreateWorldVertexBuffer(&s_world.draw.vd.worldVb, (int *)s_world.draw.vd.vertices, 44 * s_world.draw.vertexCount);
    if ( s_world.draw.vld.layerVb
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp.cpp",
                    474,
                    0,
                    "%s",
                    "s_world.draw.vld.layerVb == NULL_VERTEX_BUFFER") )
    {
        __debugbreak();
    }
    if ( s_world.draw.vertexLayerDataSize )
        R_CreateWorldVertexBuffer(&s_world.draw.vld.layerVb, (int *)s_world.draw.vld.data, s_world.draw.vertexLayerDataSize);
    if ( g_worldDraw )
        R_VertexStream2_Init(g_worldDraw->vertexStream2DataSize);
}

unsigned int __cdecl R_GetDebugReflectionProbeLocs(float (*locArray)[3], unsigned int maxCount)
{
    unsigned int result; // r3
    int v4; // r9
    unsigned int v5; // r10
    GfxReflectionProbe *v6; // r8

    result = maxCount;
    if (s_world.draw.reflectionProbeCount - 1 < maxCount)
        result = s_world.draw.reflectionProbeCount - 1;
    if (result)
    {
        v4 = 0;
        v5 = result;
        do
        {
            --v5;
            v6 = &s_world.draw.reflectionProbes[++v4];
            (*locArray)[0] = v6->origin[0];
            (*locArray)[1] = v6->origin[1];
            (*locArray++)[2] = v6->origin[2];
        } while (v5);
    }
    return result;
}

void __cdecl R_BspGenerateReflections()
{
    R_GenerateReflections(
        (char *)rgp.world->name,
        rgp.world->draw.reflectionProbes + 1,
        rgp.world->draw.reflectionProbeCount - 1);
}

void __cdecl R_RegisterSkyboxModel(char *xmodelName)
{
    g_skyboxModel = R_RegisterModel(xmodelName);
}

