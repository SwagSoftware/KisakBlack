#include "r_bsp_load_obj.h"
#include <universal/q_shared.h>
#include "r_dvars.h"
#include "r_buffers.h"
#include <universal/com_memory.h>
#include "r_bsp.h"
#include <universal/q_parse.h>
#include <DynEntity/DynEntity_coll.h>
#include <qcommon/com_profilemapload.h>
#include <qcommon/com_bsp_load_obj.h>
#include <qcommon/cm_load_obj.h>
#include <DynEntity/DynEntity_load_obj.h>
#include "r_material_load_obj.h"
#include "r_sky_load_obj.h"
#include "r_outdoor.h"
#include "r_staticmodelcache_load_obj.h"
#include "r_image.h"
#include "r_image_load_obj.h"

#include <algorithm>
#include <cgame_mp/cg_ents_mp.h>
#include <monkey/monkey.h>
#include "r_stream_util.h"
#include "r_light.h"
#include "r_staticmodel_load_obj.h"
#include <xanim/xmodel_utils.h>
#include <universal/com_math_anglevectors.h>
#include <xanim/xmodel_load_obj.h>
#include "r_model.h"
#include <client_mp/cl_scrn_mp.h>
#include "r_reflection_probe.h"

struct r_globals_load_t // sizeof=0x2C4
{                                       // XREF: .data:rgl/r
    int *cullGroupIndices;              // XREF: R_LoadCells+135/r
                                        // R_LoadCullGroupIndices+35/w
    float (*portalVerts)[3];            // XREF: R_LoadPortalVerts+35/w
                                        // R_LoadPortals+19B/r ...
    GfxAabbTree *aabbTrees;             // XREF: R_LoadAabbTrees+36/w
                                        // R_LoadAabbTrees+1B2/r ...
    int aabbTreeCount;                  // XREF: R_LoadAabbTrees+3F/w
    int nodeCount;                      // XREF: R_LoadNodesAndLeafs+83/w
                                        // R_SetParentAndCell_r+12/r ...
    mnode_load_t *nodes;                // XREF: R_LoadNodesAndLeafs+7A/w
                                        // R_LoadNodesAndLeafs:loc_A9BE52/r ...
    int reflectionProbesLoaded;         // XREF: R_CreateDefaultProbes+3E/w
                                        // R_LoadReflectionProbes:loc_A95321/w ...
    int staticModelReflectionProbesLoaded; // XREF: R_PostLoadEntities+B/r
                                        // R_SetStaticModelReflectionProbes:loc_A9CFE9/w
    GfxBspLoad load;                    // XREF: R_GetBspMaterial(uint,GfxSurface *,GfxWorldVertex *)+4C/r
                                        // R_LoadWorldInternal(char const *)+4D/w ...
};

r_globals_load_t rgl;

DiskGfxReflectionProbe defaultReflectionProbeRawData;
int r_heroLightAxis;

const Material *__cdecl R_GetBspMaterial(unsigned int materialIndex)
{
    const dmaterial_t *name; // [esp+2Ch] [ebp-110h]
    char materialName[256]; // [esp+34h] [ebp-108h] BYREF
    const dmaterial_t *diskMaterial; // [esp+138h] [ebp-4h]

    if ( materialIndex >= 0x800
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    290,
                    0,
                    "materialIndex doesn't index MAX_MAP_MATERIALS\n\t%i not in [0, %i)",
                    materialIndex,
                    2048) )
    {
        __debugbreak();
    }
    diskMaterial = &rgl.load.diskMaterials[materialIndex];
    name = diskMaterial;
    if ( !diskMaterial->material[0]
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp", 299, 0, "%s", "name[0]") )
    {
        __debugbreak();
    }
    if ( !strcmp(diskMaterial->material, "noshader")
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    300,
                    0,
                    "%s",
                    "strcmp( name, \"noshader\" )") )
    {
        __debugbreak();
    }
    if ( !strcmp(diskMaterial->material, "$default") )
        name = (const dmaterial_t *)"$default3d";
    if ( name->material[0] == 42 )
        Com_sprintf(materialName, 0x100u, "%s%s", "", name->material);
    else
        Com_sprintf(materialName, 0x100u, "%s%s", "wc/", name->material);
    return Material_Register(materialName, 9);
}

void __cdecl R_CreateWorldVertexBuffer(IDirect3DVertexBuffer9 **vb, int *srcData, unsigned int sizeInBytes)
{
    int dummyData; // [esp+0h] [ebp-8h] BYREF
    void *dstData; // [esp+4h] [ebp-4h]

    if ( r_loadForRenderer->current.enabled )
    {
        if ( (srcData == 0) != (sizeInBytes == 0)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        383,
                        0,
                        "%s",
                        "(srcData == NULL) == (sizeInBytes == 0)") )
        {
            __debugbreak();
        }
        if ( !sizeInBytes )
        {
            dummyData = 0;
            srcData = &dummyData;
            sizeInBytes = 4;
        }
        dstData = R_AllocStaticVertexBuffer(vb, sizeInBytes);
        Com_Memcpy(dstData, srcData, sizeInBytes);
        R_FinishStaticVertexBuffer(*vb);
    }
    else
    {
        *vb = 0;
    }
}

unsigned __int8 *__cdecl R_LoadSurfaceAlloc(unsigned int bytes)
{
    return Hunk_Alloc(bytes, "R_LoadSurfaces", 21);
}

char *__cdecl R_ParseSunLight(SunLightParseParams *params, char *text)
{
    char v3; // [esp+3h] [ebp-10A9h]
    char *skyBoxModel; // [esp+8h] [ebp-10A4h]
    char *p_nptr; // [esp+Ch] [ebp-10A0h]
    float v6; // [esp+20h] [ebp-108Ch]
    float v7; // [esp+24h] [ebp-1088h]
    char v8; // [esp+2Bh] [ebp-1081h]
    float v9; // [esp+2Ch] [ebp-1080h] BYREF
    float v10; // [esp+30h] [ebp-107Ch] BYREF
    float v11; // [esp+34h] [ebp-1078h] BYREF
    float v12; // [esp+38h] [ebp-1074h]
    char v13; // [esp+3Fh] [ebp-106Dh]
    float v14; // [esp+40h] [ebp-106Ch]
    float v15; // [esp+44h] [ebp-1068h]
    float v16; // [esp+48h] [ebp-1064h]
    float v17; // [esp+4Ch] [ebp-1060h]
    float v18; // [esp+50h] [ebp-105Ch] BYREF
    float v19; // [esp+54h] [ebp-1058h] BYREF
    float v20; // [esp+58h] [ebp-1054h] BYREF
    float v21; // [esp+5Ch] [ebp-1050h] BYREF
    float v22; // [esp+60h] [ebp-104Ch] BYREF
    float v23; // [esp+64h] [ebp-1048h] BYREF
    char *src; // [esp+68h] [ebp-1044h]
    char dest[2052]; // [esp+6Ch] [ebp-1040h] BYREF
    float v26; // [esp+870h] [ebp-83Ch] BYREF
    float v27; // [esp+874h] [ebp-838h] BYREF
    float v28; // [esp+878h] [ebp-834h] BYREF
    float v29; // [esp+87Ch] [ebp-830h] BYREF
    float v30; // [esp+880h] [ebp-82Ch] BYREF
    float v31; // [esp+884h] [ebp-828h] BYREF
    float out; // [esp+888h] [ebp-824h] BYREF
    float v33; // [esp+88Ch] [ebp-820h] BYREF
    float v34; // [esp+890h] [ebp-81Ch] BYREF
    char nptr; // [esp+894h] [ebp-818h] BYREF
    _BYTE v36[3]; // [esp+895h] [ebp-817h] BYREF
    float v37; // [esp+109Ch] [ebp-10h]
    float in; // [esp+10A0h] [ebp-Ch] BYREF
    float v39; // [esp+10A4h] [ebp-8h] BYREF
    float v40; // [esp+10A8h] [ebp-4h] BYREF

    if ( !params
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp", 4802, 0, "%s", "params") )
    {
        __debugbreak();
    }
    v8 = 0;
    v12 = 8.0f;
    v16 = 0.25f;
    v7 = 1.0f;
    v37 = 0.0f;
    v21 = 0.0f;
    v22 = 0.0f;
    v23 = 0.0f;
    v26 = 0.0f;
    v27 = 0.0f;
    v28 = 0.0f;
    v17 = 0.0f;
    in = 0.0f;
    v39 = 0.0f;
    v40 = 0.0f;
    v14 = 0.0f;
    out = 0.0f;
    v33 = 0.0f;
    v34 = 0.0f;
    v6 = 0.0f;
    v9 = 0.0f;
    v10 = 0.0f;
    v11 = 0.0f;
    v13 = 0;
    v18 = 0.0f;
    v19 = 0.0f;
    v20 = 0.0f;
    v15 = 0.0f;
    while ( 1 )
    {
        src = (char *)Com_Parse((const char **)&text)->token;
        if ( !*src || *src == 125 )
            break;
        if ( *src != 123 )
        {
            I_strncpyz(dest, src, 2048);
            src = (char *)Com_Parse((const char **)&text);
            if ( !*src || *src == 125 )
                break;
            I_strncpyz(&nptr, src, 2048);
            if ( I_stricmp(dest, "newsun") )
            {
                if ( I_stricmp(dest, "exposure") )
                {
                    if ( !I_stricmp(dest, "skyboxhdrscale") )
                        v7 = atof(&nptr);
                    if ( I_stricmp(dest, "ambient") )
                    {
                        if ( I_stricmp(dest, "_color") )
                        {
                            if ( I_stricmp(dest, "diffuseFraction") )
                            {
                                if ( I_stricmp(dest, "sundiffusecolor") )
                                {
                                    if ( I_stricmp(dest, "suncolor") )
                                    {
                                        if ( I_stricmp(dest, "sunlight") )
                                        {
                                            if ( I_stricmp(dest, "_sunspecularcolor") )
                                            {
                                                if ( I_stricmp(dest, "_ambientcolor") )
                                                {
                                                    if ( I_stricmp(dest, "ambientintensity") )
                                                    {
                                                        if ( I_stricmp(dest, "_sunshadowcolor") )
                                                        {
                                                            if ( I_stricmp(dest, "sunshadowintensity") )
                                                            {
                                                                if ( I_stricmp(dest, "sundirection") )
                                                                {
                                                                    if ( I_stricmp(dest, "name") )
                                                                    {
                                                                        if ( I_stricmp(dest, "skyBoxModel") )
                                                                        {
                                                                            if ( I_stricmp(dest, "treeScatterIntensity") )
                                                                            {
                                                                                if ( !I_stricmp(dest, "treeScatterAmount") )
                                                                                    params->treeScatterAmount = atof(&nptr);
                                                                            }
                                                                            else
                                                                            {
                                                                                params->treeScatterIntensity = atof(&nptr);
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                            s_world.skyBoxModel = (const char *)Hunk_Alloc(
                                                                                                                                                        &v36[strlen(&nptr)] - v36 + 1,
                                                                                                                                                        "R_ParseSunLight",
                                                                                                                                                        21);
                                                                            p_nptr = &nptr;
                                                                            skyBoxModel = (char *)s_world.skyBoxModel;
                                                                            do
                                                                            {
                                                                                v3 = *p_nptr;
                                                                                *skyBoxModel++ = *p_nptr++;
                                                                            }
                                                                            while ( v3 );
                                                                        }
                                                                    }
                                                                    else
                                                                    {
                                                                        I_strncpyz(params->name, &nptr, 64);
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    sscanf(&nptr, "%f %f %f", &v21, &v22, &v23);
                                                                }
                                                            }
                                                            else
                                                            {
                                                                v15 = atof(&nptr);
                                                            }
                                                        }
                                                        else
                                                        {
                                                            sscanf(&nptr, "%f %f %f", &v18, &v19, &v20);
                                                        }
                                                    }
                                                    else
                                                    {
                                                        v14 = atof(&nptr);
                                                    }
                                                }
                                                else
                                                {
                                                    sscanf(&nptr, "%f %f %f", &in, &v39, &v40);
                                                }
                                            }
                                            else
                                            {
                                                sscanf(&nptr, "%f %f %f", &v9, &v10, &v11);
                                                v13 = 1;
                                            }
                                        }
                                        else
                                        {
                                            v6 = atof(&nptr);
                                        }
                                    }
                                    else
                                    {
                                        sscanf(&nptr, "%f %f %f", &out, &v33, &v34);
                                    }
                                }
                                else
                                {
                                    sscanf(&nptr, "%f %f %f", &v29, &v30, &v31);
                                }
                            }
                            else
                            {
                                v37 = atof(&nptr);
                            }
                        }
                        else
                        {
                            sscanf(&nptr, "%f %f %f", &v26, &v27, &v28);
                        }
                    }
                    else
                    {
                        v17 = atof(&nptr);
                    }
                }
                else
                {
                    v12 = atof(&nptr);
                    if ( v12 < 0.0 )
                    {
                        v12 = 0.0f;
                        printf("WARNING: Negative exposure in worldspwan\n");
                    }
                }
            }
            else
            {
                v8 = 1;
            }
        }
    }
    if ( v8 == 1 )
    {
        ColorSRGBtoLinear(&in, &in);
        params->sunSettings[0].ambientColor[0] = in;
        params->sunSettings[0].ambientColor[1] = v39;
        params->sunSettings[0].ambientColor[2] = v40;
        params->sunSettings[0].ambientColor[3] = v14;
        ColorSRGBtoLinear(&out, &out);
        params->sunSettings[0].sunDiffuseColor[0] = out;
        params->sunSettings[0].sunDiffuseColor[1] = v33;
        params->sunSettings[0].sunDiffuseColor[2] = v34;
        params->sunSettings[0].sunDiffuseColor[3] = v6;
        if ( v13 )
        {
            ColorSRGBtoLinear(&v9, &v9);
            params->sunSettings[0].sunSpecularColor[0] = v9;
            params->sunSettings[0].sunSpecularColor[1] = v10;
            params->sunSettings[0].sunSpecularColor[2] = v11;
        }
        else
        {
            params->sunSettings[0].sunSpecularColor[0] = out;
            params->sunSettings[0].sunSpecularColor[1] = v33;
            params->sunSettings[0].sunSpecularColor[2] = v34;
        }
        params->sunSettings[0].sunSpecularColor[3] = v6;
        ColorSRGBtoLinear(&v18, &v18);
        params->sunSettings[0].groundColor[0] = v18;
        params->sunSettings[0].groundColor[1] = v19;
        params->sunSettings[0].groundColor[2] = v20;
        params->sunSettings[0].groundColor[3] = v15;
    }
    else
    {
        ColorNormalize(&v26, &v26);
        ColorNormalize(&v29, &v29);
        ColorNormalize(&out, &out);
        ColorSRGBtoLinear(&v26, &v26);
        ColorSRGBtoLinear(&v29, &v29);
        ColorSRGBtoLinear(&out, &out);
        params->sunSettings[0].ambientColor[0] = v26;
        params->sunSettings[0].ambientColor[1] = v27;
        params->sunSettings[0].ambientColor[2] = v28;
        params->sunSettings[0].ambientColor[3] = v17;
        params->sunSettings[0].sunDiffuseColor[0] = out;
        params->sunSettings[0].sunDiffuseColor[1] = v33;
        params->sunSettings[0].sunDiffuseColor[2] = v34;
        params->sunSettings[0].sunDiffuseColor[3] = (float)(1.0 - v37) * (float)(v6 - v17);
        params->sunSettings[0].sunSpecularColor[0] = out;
        params->sunSettings[0].sunSpecularColor[1] = v33;
        params->sunSettings[0].sunSpecularColor[2] = v34;
        params->sunSettings[0].sunSpecularColor[3] = (float)(1.0 - v37) * (float)(v6 - v17);
        params->sunSettings[0].groundColor[0] = v29;
        params->sunSettings[0].groundColor[1] = v30;
        params->sunSettings[0].groundColor[2] = v31;
        params->sunSettings[0].groundColor[3] = (float)(v6 - v17) * v37;
    }
    params->sunSettings[0].angles[0] = v21;
    params->sunSettings[0].angles[1] = v22;
    params->sunSettings[0].angles[2] = v23;
    params->sunSettings[0].exposure = v12;
    params->sunSettings[0].sunShadowSampleSizeNear = v16;
    params->sunSettings[0].skyboxHDRScale = v7;
    return text;
}

GfxWorld *__cdecl R_LoadWorldInternal(const char *name)
{
    const char *FilenameSubString; // eax
    unsigned __int16 EntityCount; // ax
    char v4; // [esp+3h] [ebp-8Dh]
    char *v5; // [esp+8h] [ebp-88h]
    char *v6; // [esp+Ch] [ebp-84h]
    char v7; // [esp+23h] [ebp-6Dh]
    char *v8; // [esp+28h] [ebp-68h]
    const char *v9; // [esp+2Ch] [ebp-64h]
    char baseName[68]; // [esp+40h] [ebp-50h] BYREF
    unsigned int drawType; // [esp+88h] [ebp-8h]
    DynEntityCollType collType; // [esp+8Ch] [ebp-4h]

    memset(&s_world, 0, sizeof(s_world));
    memset(&rgl, 0, sizeof(rgl));
    ProfLoad_Begin("Load world initialization");
    rgl.load.bspVersion = Com_GetBspVersion();
    s_world.name = (const char *)Hunk_Alloc(strlen(name) + 1, "R_LoadWorldInternal", 21);
    v9 = name;
    v8 = (char *)s_world.name;
    do
    {
        v7 = *v9;
        *v8++ = *v9++;
    }
    while ( v7 );
    FilenameSubString = Com_GetFilenameSubString(s_world.name);
    I_strncpyz(baseName, FilenameSubString, 64);
    Com_StripExtension(baseName, baseName);
    s_world.baseName = (const char *)Hunk_Alloc(
                                                                         &baseName[strlen(baseName) + 1] - &baseName[1] + 1,
                                                                         "R_LoadWorldInternal",
                                                                         21);
    v6 = baseName;
    v5 = (char *)s_world.baseName;
    do
    {
        v4 = *v6;
        *v5++ = *v6++;
    }
    while ( v4 );
    s_world.dpvsPlanes.planes = CM_GetPlanes();
    s_world.planeCount = CM_GetPlaneCount();
    ProfLoad_End();
    for ( drawType = 0; drawType < 2; ++drawType )
    {
        collType = (DynEntityCollType)drawType;
        EntityCount = DynEnt_GetEntityCount((DynEntityCollType)drawType);
        s_world.dpvsDyn.dynEntClientCount[drawType] = EntityCount;
        s_world.dpvsDyn.dynEntClientWordCount[drawType] = (s_world.dpvsDyn.dynEntClientCount[drawType] + 31) >> 5;
    }
    ProfLoad_Begin("Load world materials");
    R_LoadStep("materials");
    R_LoadMaterials(&rgl.load);
    ProfLoad_End();
    ProfLoad_Begin("Load lighting");
    R_LoadStep("sun settings");
    R_LoadSunSettings();
    R_LoadStep("primary lights");
    R_LoadPrimaryLights(rgl.load.bspVersion);
    R_LoadStep("light regions");
    R_LoadLightRegions();
    R_LoadStep("lightmaps");
    R_LoadLightmaps(&rgl.load);
    ProfLoad_End();
    ProfLoad_Begin("Load light grid");
    if ( rgl.load.bspVersion > 0xF )
    {
        R_LoadStep("lightgrid colors");
        R_LoadLightGridColors(rgl.load.bspVersion);
        R_LoadStep("lightgrid row data");
        R_LoadLightGridRowData();
        R_LoadStep("lightgrid entries");
        R_LoadLightGridEntries();
        R_LoadStep("lightgrid header");
        R_LoadLightGridHeader();
    }
    else
    {
        R_LoadStep("lightgrid colors");
        R_LoadLightGridColors(rgl.load.bspVersion);
        R_LoadStep("lightgrid points");
        R_LoadLightGridPoints_Version15(rgl.load.bspVersion);
    }
    ProfLoad_End();
    R_AllocShadowGeometryHeaderMemory();
    ProfLoad_Begin("Load world submodels");
    R_LoadStep("submodels");
    R_LoadSubmodels();
    ProfLoad_End();
    ProfLoad_Begin("Load world surfaces");
    R_LoadStep("surfaces");
    R_LoadSurfaces(&rgl.load);
    ProfLoad_End();
    ProfLoad_Begin("Load cull groups");
    R_LoadStep("cull groups");
    R_LoadCullGroups();
    R_LoadStep("cull group indices");
    R_LoadCullGroupIndices();
    ProfLoad_End();
    ProfLoad_Begin("Load portals");
    R_LoadStep("portal vertices");
    R_LoadPortalVerts();
    R_LoadStep("AABB trees");
    R_LoadAabbTrees();
    R_LoadStep("cells");
    R_LoadCells(rgl.load.bspVersion);
    R_LoadStep("portals");
    R_LoadPortals();
    R_LoadStep("nodes and leafs");
    R_LoadNodesAndLeafs(rgl.load.bspVersion);
    ProfLoad_End();
    Material_Sort();
    R_SortSurfaces();
    R_CalculateVertexStream2Usage();
    ProfLoad_Begin("Load renderer entities");
    R_LoadStep("entities");
    R_LoadEntities();
    ProfLoad_End();
    ProfLoad_Begin("Load reflection probes");
    R_LoadStep("reflection probes");
    if ( rgl.load.bspVersion > 7 )
        R_LoadReflectionProbes(rgl.load.bspVersion);
    else
        R_CreateDefaultProbes();
    if ( rgl.load.bspVersion <= 0x15 )
        R_AddAllProbesToAllCells();
    ProfLoad_End();
    R_LoadCoronas();
    R_LoadShadowMapVolumes();
    R_LoadExposureVolumes();
    R_LoadWorldLodData();
    R_LoadOccluders();
    R_SetStaticModelReflectionProbes();
    R_LoadHeroOnlyLights(rgl.load.bspVersion);
    R_PostLoadEntities();
    R_InitShadowGeometryArrays();
    //BLOPS_NULLSUB();
    ProfLoad_Begin("Generate high mip aabbs");
    R_GenerateHighmipAabbs();
    ProfLoad_End();
    ProfLoad_Begin("Load sun");
    R_LoadSun(name, &s_world.sun);
    ProfLoad_End();
    ProfLoad_Begin("Register outdoor image");
    R_LoadOutdoorBounds();
    R_RegisterOutdoorImage(&s_world, rgl.load.outdoorMins, rgl.load.outdoorMaxs);
    ProfLoad_End();
    R_LoadWorldRuntime();
    R_LoadInitSkyIntensity();
    R_AssignSModelCacheResources(&s_world);
    memset(&rgl, 0, sizeof(rgl));
    return &s_world;
}

void __cdecl R_LoadLightmaps(GfxBspLoad *load)
{
    char *v1; // eax
    GfxImage *v2; // eax
    char *v3; // eax
    GfxImage *v4; // eax
    char *v5; // eax
    GfxImage *v6; // eax
    float v7; // [esp+4h] [ebp-1ECh]
    float v8; // [esp+8h] [ebp-1E8h]
    float v9; // [esp+Ch] [ebp-1E4h]
    float v10; // [esp+10h] [ebp-1E0h]
    float v11; // [esp+14h] [ebp-1DCh]
    float v12; // [esp+18h] [ebp-1D8h]
    int v13; // [esp+1Ch] [ebp-1D4h]
    int v14; // [esp+20h] [ebp-1D0h]
    float v15; // [esp+34h] [ebp-1BCh]
    float v16; // [esp+38h] [ebp-1B8h]
    float v17; // [esp+3Ch] [ebp-1B4h]
    float v18; // [esp+40h] [ebp-1B0h]
    float v19; // [esp+44h] [ebp-1ACh]
    float v20; // [esp+48h] [ebp-1A8h]
    float v21; // [esp+5Ch] [ebp-194h]
    float v22; // [esp+60h] [ebp-190h]
    float v23; // [esp+64h] [ebp-18Ch]
    float v24; // [esp+68h] [ebp-188h]
    float v25; // [esp+6Ch] [ebp-184h]
    float v26; // [esp+70h] [ebp-180h]
    unsigned __int8 dir; // [esp+7Fh] [ebp-171h]
    unsigned __int8 loB; // [esp+86h] [ebp-16Ah]
    unsigned __int8 loR; // [esp+87h] [ebp-169h]
    int k; // [esp+88h] [ebp-168h]
    int dIndex; // [esp+94h] [ebp-15Ch]
    int j; // [esp+9Ch] [ebp-154h]
    unsigned __int8 *lightmapLum; // [esp+A0h] [ebp-150h]
    unsigned __int8 *v34; // [esp+A4h] [ebp-14Ch]
    float lum; // [esp+A8h] [ebp-148h]
    unsigned __int8 *lightmapColor; // [esp+B0h] [ebp-140h]
    float a; // [esp+C0h] [ebp-130h]
    int h; // [esp+C4h] [ebp-12Ch]
    int ha; // [esp+C4h] [ebp-12Ch]
    unsigned __int8 *dst8; // [esp+C8h] [ebp-128h]
    int w; // [esp+CCh] [ebp-124h]
    int wa; // [esp+CCh] [ebp-124h]
    unsigned int *dst32; // [esp+D0h] [ebp-120h]
    unsigned int i; // [esp+D4h] [ebp-11Ch]
    float *src; // [esp+D8h] [ebp-118h]
    unsigned __int8 *primaryImage; // [esp+E8h] [ebp-108h]
    int reorder[16]; // [esp+F0h] [ebp-100h] BYREF
    int tileIndex; // [esp+130h] [ebp-C0h]
    int totalImageSize; // [esp+134h] [ebp-BCh]
    const unsigned __int8 *buf; // [esp+138h] [ebp-B8h]
    int width; // [esp+13Ch] [ebp-B4h]
    int height; // [esp+140h] [ebp-B0h]
    const unsigned __int8 *buf_p; // [esp+144h] [ebp-ACh]
    r_lightmapGroup_t groupInfo[16]; // [esp+148h] [ebp-A8h] BYREF
    unsigned __int8 newLmapIndex; // [esp+1CBh] [ebp-25h]
    int groupCount; // [esp+1CCh] [ebp-24h]
    unsigned int len; // [esp+1D0h] [ebp-20h] BYREF
    int oldLmapBaseIndex; // [esp+1D4h] [ebp-1Ch]
    int x; // [esp+1D8h] [ebp-18h]
    int y; // [esp+1DCh] [ebp-14h]
    int imageFlags; // [esp+1E0h] [ebp-10h]
    unsigned __int8 *secondaryImage; // [esp+1E4h] [ebp-Ch]
    int oldLmapCount; // [esp+1E8h] [ebp-8h]
    int oldLmapIndex; // [esp+1ECh] [ebp-4h]

    printf("Loading LightMaps\n");
    if ( !load
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp", 965, 0, "%s", "load") )
    {
        __debugbreak();
    }
    load->lmapMergeInfo[31].index = 31;
    load->lmapMergeInfo[31].shift[0] = 0.0f;
    load->lmapMergeInfo[31].shift[1] = 0.0f;
    load->lmapMergeInfo[31].scale[0] = 1.0f;
    load->lmapMergeInfo[31].scale[1] = 1.0f;
    oldLmapCount = R_BuildLightmapMergability(load, groupInfo, reorder, LUMP_LIGHTBYTES);
    if ( oldLmapCount )
    {
        totalImageSize = groupInfo[0].highCount * 9437184 * groupInfo[0].wideCount;
        primaryImage = (unsigned __int8 *)Hunk_AllocateTempMemory(totalImageSize, "R_LoadLightmaps");
        secondaryImage = &primaryImage[groupInfo[0].highCount * (groupInfo[0].wideCount << 20)];
        buf = (const unsigned __int8 *)Com_GetBspLump(LUMP_LIGHTBYTES, 1u, &len);
        if ( load->bspVersion < 7 )
            len = 0;
        if ( !len )
        {
            printf("\nNo Lightmap found - setting to white\n");
            memset(primaryImage, 0xFFu, groupInfo[0].highCount * (groupInfo[0].wideCount << 20));
            src = (float *)secondaryImage;
            for ( i = 0; i < (unsigned int)(groupInfo[0].highCount * (groupInfo[0].wideCount << 23)) >> 2; ++i )
                src[i] = 1.0f;
        }
        imageFlags = 56;
        s_world.draw.lightmaps = (GfxLightmapArray *)Hunk_Alloc(0xCCu, "R_LoadLightmaps", 21);
        newLmapIndex = 0;
        oldLmapBaseIndex = 0;
        while ( oldLmapBaseIndex < oldLmapCount )
        {
            if ( newLmapIndex
                && groupInfo[newLmapIndex].wideCount > *(&height + 2 * newLmapIndex)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            1014,
                            0,
                            "%s",
                            "newLmapIndex == 0 || groupInfo[newLmapIndex].wideCount <= groupInfo[newLmapIndex - 1].wideCount") )
            {
                __debugbreak();
            }
            if ( newLmapIndex
                && groupInfo[newLmapIndex].highCount > (int)(&buf_p)[2 * newLmapIndex]
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            1015,
                            0,
                            "%s",
                            "newLmapIndex == 0 || groupInfo[newLmapIndex].highCount <= groupInfo[newLmapIndex - 1].highCount") )
            {
                __debugbreak();
            }
            groupCount = groupInfo[newLmapIndex].highCount * groupInfo[newLmapIndex].wideCount;
            for ( tileIndex = 0; tileIndex < groupCount; ++tileIndex )
            {
                oldLmapIndex = reorder[tileIndex + oldLmapBaseIndex];
                x = tileIndex % groupInfo[newLmapIndex].wideCount;
                y = tileIndex / groupInfo[newLmapIndex].wideCount;
                if ( len )
                {
                    buf_p = &buf[9437184 * oldLmapIndex];
                    R_CopyLightmap(buf_p, 512, 512, 16, secondaryImage, x, y, groupInfo[newLmapIndex].wideCount);
                    buf_p += 0x400000;
                    R_CopyLightmap(
                        buf_p,
                        512,
                        512,
                        16,
                        secondaryImage,
                        x,
                        groupInfo[newLmapIndex].highCount + y,
                        groupInfo[newLmapIndex].wideCount);
                    buf_p += 0x400000;
                    R_CopyLightmap(buf_p, 1024, 1024, 1, primaryImage, x, y, groupInfo[newLmapIndex].wideCount);
                }
                load->lmapMergeInfo[oldLmapIndex].index = newLmapIndex;
                load->lmapMergeInfo[oldLmapIndex].scale[0] = 1.0 / (float)groupInfo[newLmapIndex].wideCount;
                load->lmapMergeInfo[oldLmapIndex].scale[1] = 1.0 / (float)groupInfo[newLmapIndex].highCount;
                load->lmapMergeInfo[oldLmapIndex].shift[0] = (float)x * load->lmapMergeInfo[oldLmapIndex].scale[0];
                load->lmapMergeInfo[oldLmapIndex].shift[1] = (float)y * load->lmapMergeInfo[oldLmapIndex].scale[1];
            }
            v1 = va("*lightmap%i_primary", newLmapIndex);
            v2 = Image_Alloc(v1, 2u, 1u, 4u);
            s_world.draw.lightmaps[newLmapIndex].primary = v2;
            if ( !s_world.draw.lightmaps[newLmapIndex].primary
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            1059,
                            1,
                            "%s",
                            "s_world.draw.lightmaps[newLmapIndex].primary") )
            {
                __debugbreak();
            }
            Image_Generate2D(
                s_world.draw.lightmaps[newLmapIndex].primary,
                primaryImage,
                groupInfo[newLmapIndex].wideCount << 10,
                groupInfo[newLmapIndex].highCount << 10,
                D3DFMT_L8);
            v3 = va("*lightmap%i_secondary", newLmapIndex);
            v4 = Image_Alloc(v3, 2u, 1u, 4u);
            s_world.draw.lightmaps[newLmapIndex].secondary = v4;
            if ( !s_world.draw.lightmaps[newLmapIndex].secondary
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            1106,
                            1,
                            "%s",
                            "s_world.draw.lightmaps[newLmapIndex].secondary") )
            {
                __debugbreak();
            }
            v5 = va("*lightmap%i_secondaryb", newLmapIndex);
            v6 = Image_Alloc(v5, 2u, 1u, 4u);
            s_world.draw.lightmaps[newLmapIndex].secondaryB = v6;
            if ( !s_world.draw.lightmaps[newLmapIndex].secondaryB
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            1109,
                            1,
                            "%s",
                            "s_world.draw.lightmaps[newLmapIndex].secondaryB") )
            {
                __debugbreak();
            }
            width = groupInfo[newLmapIndex].wideCount << 9;
            height = groupInfo[newLmapIndex].highCount << 10;
            w = width;
            h = height / 2;
            v34 = secondaryImage;
            lightmapLum = (unsigned __int8 *)Hunk_AllocateTempMemory(4 * height / 2 * width, "R_LightMapLum");
            dst32 = (unsigned int *)lightmapLum;
            for ( j = 0; j < h * w; ++j )
            {
                dIndex = 4 * j + 4 * h * w;
                if ( *(float *)&v34[16 * j] >= 0.0024411406 )
                    v26 = *(float *)&v34[16 * j];
                else
                    v26 = 0.0024411406f;
                if ( *(float *)&v34[16 * j + 4] >= 0.0024411406 )
                    v25 = *(float *)&v34[16 * j + 4];
                else
                    v25 = 0.0024411406f;
                if ( *(float *)&v34[16 * j + 8] >= 0.0024411406 )
                    v24 = *(float *)&v34[16 * j + 8];
                else
                    v24 = 0.0024411406f;
                if ( v26 <= 31.875 )
                    v23 = v26;
                else
                    v23 = 31.875f;
                if ( v25 <= 31.875 )
                    v22 = v25;
                else
                    v22 = 31.875f;
                if ( v24 <= 31.875 )
                    v21 = v24;
                else
                    v21 = 31.875f;
                if ( *(float *)&v34[4 * dIndex] >= 0.0024411406 )
                    v20 = *(float *)&v34[4 * dIndex];
                else
                    v20 = 0.0024411406f;
                if ( *(float *)&v34[4 * dIndex + 4] >= 0.0024411406 )
                    v19 = *(float *)&v34[4 * dIndex + 4];
                else
                    v19 = 0.0024411406f;
                if ( *(float *)&v34[4 * dIndex + 8] >= 0.0024411406 )
                    v18 = *(float *)&v34[4 * dIndex + 8];
                else
                    v18 = 0.0024411406f;
                if ( v20 <= 31.875 )
                    v17 = v20;
                else
                    v17 = 31.875f;
                if ( v19 <= 31.875 )
                    v16 = v19;
                else
                    v16 = 31.875f;
                if ( v18 <= 31.875 )
                    v15 = v18;
                else
                    v15 = 31.875f;
                if ( (unsigned int)(__int64)((float)((float)((float)((float)(v23 * 0.25) + (float)(v22 * 0.5))
                                                                                                     + (float)(v21 * 0.25))
                                                                                     / 31.875)
                                                                     * 65535.0) <= 0xFFFF )
                    v14 = (__int64)((float)((float)((float)((float)(v23 * 0.25) + (float)(v22 * 0.5)) + (float)(v21 * 0.25))
                                                                / 31.875)
                                                * 65535.0);
                else
                    v14 = 0xFFFF;
                if ( (unsigned int)(__int64)((float)((float)((float)((float)(v17 * 0.25) + (float)(v16 * 0.5))
                                                                                                     + (float)(v15 * 0.25))
                                                                                     / 31.875)
                                                                     * 65535.0) <= 0xFFFF )
                    v13 = (__int64)((float)((float)((float)((float)(v17 * 0.25) + (float)(v16 * 0.5)) + (float)(v15 * 0.25))
                                                                / 31.875)
                                                * 65535.0);
                else
                    v13 = 0xFFFF;
                *dst32++ = v14 | (v13 << 16);
            }
            Image_Generate2D(s_world.draw.lightmaps[newLmapIndex].secondaryB, lightmapLum, w, h, D3DFMT_G16R16);
            Hunk_FreeTempMemory((char *)lightmapLum);
            wa = width;
            ha = height;
            lightmapColor = (unsigned __int8 *)Hunk_AllocateTempMemory(2 * height * width, "R_LightMapColor");
            dst8 = lightmapColor;
            for ( k = 0; k < ha * wa; ++k )
            {
                a = *(float *)&v34[16 * k + 12];
                if ( *(float *)&v34[16 * k] >= 0.0024411406 )
                    v12 = *(float *)&v34[16 * k];
                else
                    v12 = 0.0024411406f;
                if ( *(float *)&v34[16 * k + 4] >= 0.0024411406 )
                    v11 = *(float *)&v34[16 * k + 4];
                else
                    v11 = 0.0024411406f;
                if ( *(float *)&v34[16 * k + 8] >= 0.0024411406 )
                    v10 = *(float *)&v34[16 * k + 8];
                else
                    v10 = 0.0024411406f;
                if ( v12 <= 31.875 )
                    v9 = v12;
                else
                    v9 = 31.875f;
                if ( v11 <= 31.875 )
                    v8 = v11;
                else
                    v8 = 31.875f;
                if ( v10 <= 31.875 )
                    v7 = v10;
                else
                    v7 = 31.875f;
                lum = (float)((float)(v9 * 0.25) + (float)(v8 * 0.5)) + (float)(v7 * 0.25);
                loR = colorRound8Bit((unsigned __int8)(int)(float)((float)((float)(v9 * 0.25) / lum) * 255.0), 0x1Fu);
                loB = colorRound8Bit((unsigned __int8)(int)(float)((float)((float)(v7 * 0.25) / lum) * 255.0), 0x1Fu);
                dir = colorRound8Bit((unsigned __int8)(int)(float)(a * 255.0), 0x3Fu);
                *dst8 = loR | (32 * dir);
                dst8[1] = (unsigned __int16)((32 * dir) | (loB << 11)) >> 8;
                dst8 += 2;
            }
            Image_Generate2D(s_world.draw.lightmaps[newLmapIndex].secondary, lightmapColor, wa, ha, D3DFMT_R5G6B5);
            Hunk_FreeTempMemory((char *)lightmapColor);
            oldLmapBaseIndex += groupCount;
            ++newLmapIndex;
        }
        s_world.draw.lightmapCount = newLmapIndex;
        Hunk_FreeTempMemory((char *)primaryImage);
        if ( s_world.draw.lightmapCount > 16
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        1287,
                        0,
                        "%s\n\t(s_world.draw.lightmapCount) = %i",
                        "(s_world.draw.lightmapCount <= ((144 * 1024 * 1024) / ((1024 * 1024 * 1 * 1) + (512 * 512 * 16 * 2))))",
                        s_world.draw.lightmapCount) )
        {
            __debugbreak();
        }
        s_world.draw.lightmapPrimaryTextures = (GfxTexture *)Hunk_Alloc(
                                                                                                                     4 * s_world.draw.lightmapCount,
                                                                                                                     "R_LoadLightmaps",
                                                                                                                     21);
        s_world.draw.lightmapSecondaryTextures = (GfxTexture *)Hunk_Alloc(
                                                                                                                         4 * s_world.draw.lightmapCount,
                                                                                                                         "R_LoadLightmaps",
                                                                                                                         21);
        s_world.draw.lightmapSecondaryTexturesB = (GfxTexture *)Hunk_Alloc(
                                                                                                                            4 * s_world.draw.lightmapCount,
                                                                                                                            "R_LoadLightmaps",
                                                                                                                            21);
    }
    else
    {
        s_world.draw.lightmapCount = 0;
    }
}

void __cdecl R_CopyLightmap(
                const unsigned __int8 *srcImage,
                int srcWidth,
                int srcHeight,
                int bytesPerPixel,
                unsigned __int8 *dstImage,
                int tileX,
                int tileY,
                int tilesWide)
{
    int y; // [esp+0h] [ebp-4h]
    unsigned __int8 *dstImagea; // [esp+1Ch] [ebp+18h]

    dstImagea = &dstImage[bytesPerPixel * (srcWidth * tilesWide * srcHeight * tileY + srcWidth * tileX)];
    for ( y = 0; y < srcHeight; ++y )
    {
        Com_Memcpy(dstImagea, srcImage, bytesPerPixel * srcWidth);
        srcImage += bytesPerPixel * srcWidth;
        dstImagea += tilesWide * bytesPerPixel * srcWidth;
    }
}

int __cdecl R_BuildLightmapMergability(GfxBspLoad *load, r_lightmapGroup_t *groupInfo, int *reorder, LumpType lumpType)
{
    int otherLmapIndex; // [esp+Ch] [ebp-440h]
    int otherLmapIndexa; // [esp+Ch] [ebp-440h]
    bool used[16]; // [esp+10h] [ebp-43Ch] BYREF
    int coupling[16][16]; // [esp+24h] [ebp-428h] BYREF
    int usedCount; // [esp+424h] [ebp-28h]
    int highCount; // [esp+428h] [ebp-24h]
    int maxTextureSize; // [esp+42Ch] [ebp-20h]
    int lmapIndex; // [esp+430h] [ebp-1Ch]
    int origLmapCount; // [esp+434h] [ebp-18h]
    int mergedCount; // [esp+438h] [ebp-14h]
    int newLmapCount; // [esp+43Ch] [ebp-10h]
    int bestLmapIndex; // [esp+440h] [ebp-Ch]
    int bestOtherLmapIndex; // [esp+444h] [ebp-8h]
    int wideCount; // [esp+448h] [ebp-4h]

    origLmapCount = R_DetermineLightmapCoupling(load, coupling, lumpType);
    memset(used, 0, sizeof(used));
    newLmapCount = 0;
    maxTextureSize = 2048;
    wideCount = 2;
    highCount = 2;
    usedCount = 0;
    while ( usedCount < origLmapCount )
    {
        while ( highCount * wideCount > origLmapCount - usedCount )
        {
            if ( wideCount < highCount )
                highCount >>= 1;
            else
                wideCount >>= 1;
        }
        if ( highCount * wideCount < 2 )
        {
            mergedCount = 1;
            for ( lmapIndex = 0; lmapIndex < origLmapCount; ++lmapIndex )
            {
                if ( !used[lmapIndex] )
                {
                    reorder[usedCount++] = lmapIndex;
                    used[lmapIndex] = 1;
                    break;
                }
            }
        }
        else
        {
            bestLmapIndex = 31;
            bestOtherLmapIndex = 31;
            for ( lmapIndex = 0; lmapIndex < origLmapCount; ++lmapIndex )
            {
                if ( !used[lmapIndex] )
                {
                    for ( otherLmapIndex = lmapIndex + 1; otherLmapIndex < origLmapCount; ++otherLmapIndex )
                    {
                        if ( !used[otherLmapIndex]
                            && (bestLmapIndex == 31
                             || coupling[lmapIndex][otherLmapIndex] > coupling[bestLmapIndex][bestOtherLmapIndex]) )
                        {
                            bestLmapIndex = lmapIndex;
                            bestOtherLmapIndex = otherLmapIndex;
                        }
                    }
                }
            }
            reorder[usedCount++] = bestOtherLmapIndex;
            reorder[usedCount++] = bestLmapIndex;
            used[bestOtherLmapIndex] = 1;
            used[bestLmapIndex] = 1;
            for ( mergedCount = 2; mergedCount < highCount * wideCount; ++mergedCount )
            {
                for ( lmapIndex = 0; lmapIndex < origLmapCount; ++lmapIndex )
                {
                    coupling[bestLmapIndex][lmapIndex] += coupling[bestOtherLmapIndex][lmapIndex];
                    coupling[lmapIndex][bestLmapIndex] = coupling[bestLmapIndex][lmapIndex];
                }
                bestOtherLmapIndex = 31;
                for ( otherLmapIndexa = 0; otherLmapIndexa < origLmapCount; ++otherLmapIndexa )
                {
                    if ( !used[otherLmapIndexa]
                        && (bestOtherLmapIndex == 31
                         || coupling[bestLmapIndex][otherLmapIndexa] > coupling[bestLmapIndex][bestOtherLmapIndex]) )
                    {
                        bestOtherLmapIndex = otherLmapIndexa;
                    }
                }
                reorder[usedCount++] = bestOtherLmapIndex;
                used[bestOtherLmapIndex] = 1;
            }
        }
        groupInfo[newLmapCount].wideCount = wideCount;
        groupInfo[newLmapCount++].highCount = highCount;
    }
    Com_Printf(8, "%i merged lightmaps from %i original lightmaps\n", newLmapCount, origLmapCount);
    return origLmapCount;
}

unsigned int __cdecl R_DetermineLightmapCoupling(GfxBspLoad *load, int (*coupling)[16], LumpType lumpType)
{
    unsigned int otherLmapIndex; // [esp+4h] [ebp-64h]
    unsigned int lmapIndex; // [esp+8h] [ebp-60h]
    unsigned int lmapIndexa; // [esp+8h] [ebp-60h]
    unsigned int lmapIndexb; // [esp+8h] [ebp-60h]
    unsigned int materialIndex; // [esp+Ch] [ebp-5Ch]
    int lmapVertCount[16]; // [esp+10h] [ebp-58h] BYREF
    unsigned int origLmapCount; // [esp+54h] [ebp-14h]
    const DiskTriangleSoup *triSurfs; // [esp+58h] [ebp-10h] BYREF
    unsigned int triSurfCount; // [esp+5Ch] [ebp-Ch] BYREF
    unsigned int diskLmapCount; // [esp+60h] [ebp-8h] BYREF
    unsigned int triSurfIndex; // [esp+64h] [ebp-4h]

    if ( !load
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp", 557, 0, "%s", "load") )
    {
        __debugbreak();
    }
    if ( !coupling
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp", 558, 0, "%s", "coupling") )
    {
        __debugbreak();
    }
    R_LoadTriangleSurfaces(load->bspVersion, (DiskTriangleSoup **)&triSurfs, &triSurfCount);
    if ( load->bspVersion >= 7 )
        Com_GetBspLump(lumpType, (unsigned int)0x900000 + (lumpType != LUMP_LIGHTBYTES ? 0xFF7C0000 : 0), &diskLmapCount);
    else
        diskLmapCount = 0;
    origLmapCount = 0;
    for ( triSurfIndex = 0; triSurfIndex < triSurfCount; ++triSurfIndex )
    {
        lmapIndex = triSurfs[triSurfIndex].lightmapIndex;
        if ( lmapIndex != 31 && origLmapCount <= lmapIndex )
            origLmapCount = lmapIndex + 1;
    }
    if ( diskLmapCount && diskLmapCount != origLmapCount )
        Com_Error(ERR_DROP, "LoadMap: funny lump size in %s", s_world.name);
    memset(lmapVertCount, 0, sizeof(lmapVertCount));
    memset(coupling, 0, 0x400u);
    for ( materialIndex = 0; materialIndex < load->materialCount; ++materialIndex )
    {
        for ( triSurfIndex = 0; triSurfIndex < triSurfCount; ++triSurfIndex )
        {
            if ( triSurfs[triSurfIndex].materialIndex == materialIndex )
            {
                lmapIndexa = triSurfs[triSurfIndex].lightmapIndex;
                if ( lmapIndexa != 31 )
                    lmapVertCount[lmapIndexa] += triSurfs[triSurfIndex].vertexCount;
            }
        }
        for ( lmapIndexb = 0; lmapIndexb < origLmapCount; ++lmapIndexb )
        {
            if ( lmapVertCount[lmapIndexb] )
            {
                for ( otherLmapIndex = lmapIndexb + 1; otherLmapIndex < origLmapCount; ++otherLmapIndex )
                {
                    if ( lmapVertCount[otherLmapIndex] )
                    {
                        (*coupling)[16 * lmapIndexb + otherLmapIndex] += lmapVertCount[otherLmapIndex] + lmapVertCount[lmapIndexb];
                        if ( (*coupling)[16 * lmapIndexb + otherLmapIndex] < 0 )
                            (*coupling)[16 * lmapIndexb + otherLmapIndex] = 0x7FFFFFFF;
                        (*coupling)[16 * otherLmapIndex + lmapIndexb] = (*coupling)[16 * lmapIndexb + otherLmapIndex];
                    }
                }
                lmapVertCount[lmapIndexb] = 0;
            }
        }
    }
    if ( triSurfCount )
        R_UnloadTriangleSurfaces(load->bspVersion, (DiskTriangleSoup *)triSurfs);
    return origLmapCount;
}

void __cdecl R_LoadTriangleSurfaces(unsigned int bspVersion, DiskTriangleSoup **diskSurfaces, unsigned int *surfCount)
{
    const DiskTriangleSoup *BspLump; // eax

    if ( bspVersion > 8 )
    {
        if ( bspVersion > 0xC )
        {
            if ( bspVersion > 0xE )
            {
                BspLump = (const DiskTriangleSoup *)Com_GetBspLump(LUMP_TRIANGLES, 0x1Cu, surfCount);
            }
            else
            {
                *diskSurfaces = (DiskTriangleSoup *)Com_GetBspLump(LUMP_TRIANGLES, 0x1Cu, surfCount);
                BspLump = R_UpdateDiskSurfaces_Version14(*diskSurfaces, *surfCount);
            }
            *diskSurfaces = (DiskTriangleSoup *)BspLump;
        }
        else
        {
            *diskSurfaces = (DiskTriangleSoup *)Com_GetBspLump(LUMP_TRIANGLES, 0x14u, surfCount);
            *diskSurfaces = (DiskTriangleSoup *)R_UpdateDiskSurfaces_Version12(
                                                                                        (const DiskTriangleSoup_Version12 *)*diskSurfaces,
                                                                                        *surfCount);
        }
    }
    else
    {
        *diskSurfaces = (DiskTriangleSoup *)Com_GetBspLump(LUMP_TRIANGLES, 0x10u, surfCount);
        *diskSurfaces = (DiskTriangleSoup *)R_UpdateDiskSurfaces_Version8(
                                                                                    (const DiskTriangleSoup_Version8 *)*diskSurfaces,
                                                                                    *surfCount);
    }
}

const DiskTriangleSoup *__cdecl R_UpdateDiskSurfaces_Version14(DiskTriangleSoup *oldSurfs, int surfCount)
{
    int surfIndex; // [esp+0h] [ebp-8h]
    unsigned __int8 *newSurfs; // [esp+4h] [ebp-4h]

    newSurfs = (unsigned __int8 *)Hunk_AllocateTempMemory(28 * surfCount, "R_UpdateDiskSurfaces");
    memcpy(newSurfs, (unsigned __int8 *)oldSurfs, 28 * surfCount);
    for ( surfIndex = 0; surfIndex < surfCount; ++surfIndex )
    {
        if ( newSurfs[28 * surfIndex + 4] == 255 )
            newSurfs[28 * surfIndex + 4] = 0;
        else
            ++newSurfs[28 * surfIndex + 4];
    }
    return (const DiskTriangleSoup *)newSurfs;
}

const DiskTriangleSoup *__cdecl R_UpdateDiskSurfaces_Version12(
                const DiskTriangleSoup_Version12 *oldSurfs,
                int surfCount)
{
    int surfIndex; // [esp+4h] [ebp-8h]
    unsigned int *newSurfs; // [esp+8h] [ebp-4h]

    newSurfs = Hunk_AllocateTempMemory(28 * surfCount, "R_UpdateDiskSurfaces");
    for ( surfIndex = 0; surfIndex < surfCount; ++surfIndex )
    {
        LOWORD(newSurfs[7 * surfIndex]) = oldSurfs[surfIndex].materialIndex;
        BYTE2(newSurfs[7 * surfIndex]) = oldSurfs[surfIndex].lightmapIndex;
        HIBYTE(newSurfs[7 * surfIndex]) = oldSurfs[surfIndex].reflectionProbeIndex;
        LOBYTE(newSurfs[7 * surfIndex + 1]) = 1;
        newSurfs[7 * surfIndex + 3] = oldSurfs[surfIndex].vertexLayerData;
        newSurfs[7 * surfIndex + 4] = oldSurfs[surfIndex].firstVertex;
        LOWORD(newSurfs[7 * surfIndex + 5]) = oldSurfs[surfIndex].vertexCount;
        HIWORD(newSurfs[7 * surfIndex + 5]) = oldSurfs[surfIndex].indexCount;
        newSurfs[7 * surfIndex + 6] = oldSurfs[surfIndex].firstIndex;
    }
    return (const DiskTriangleSoup *)newSurfs;
}

const DiskTriangleSoup *__cdecl R_UpdateDiskSurfaces_Version8(const DiskTriangleSoup_Version8 *oldSurfs, int surfCount)
{
    int surfIndex; // [esp+4h] [ebp-8h]
    unsigned int *newSurfs; // [esp+8h] [ebp-4h]

    newSurfs = Hunk_AllocateTempMemory(28 * surfCount, "R_UpdateDiskSurfaces");
    for ( surfIndex = 0; surfIndex < surfCount; ++surfIndex )
    {
        LOWORD(newSurfs[7 * surfIndex]) = oldSurfs[surfIndex].materialIndex;
        BYTE2(newSurfs[7 * surfIndex]) = oldSurfs[surfIndex].lightmapIndex;
        HIBYTE(newSurfs[7 * surfIndex]) = oldSurfs[surfIndex].reflectionProbeIndex;
        LOBYTE(newSurfs[7 * surfIndex + 1]) = 1;
        newSurfs[7 * surfIndex + 3] = 0;
        newSurfs[7 * surfIndex + 4] = oldSurfs[surfIndex].firstVertex;
        LOWORD(newSurfs[7 * surfIndex + 5]) = oldSurfs[surfIndex].vertexCount;
        HIWORD(newSurfs[7 * surfIndex + 5]) = oldSurfs[surfIndex].indexCount;
        newSurfs[7 * surfIndex + 6] = oldSurfs[surfIndex].firstIndex;
    }
    return (const DiskTriangleSoup *)newSurfs;
}

void __cdecl R_UnloadTriangleSurfaces(unsigned int bspVersion, DiskTriangleSoup *diskSurfaces)
{
    if ( bspVersion <= 0xE )
        Hunk_FreeTempMemory((char *)diskSurfaces);
}

unsigned int __cdecl colorRound8Bit(unsigned int a, unsigned int b)
{
    return (b * a + 128 + ((b * a + 128) >> 8)) >> 8;
}

void R_LoadLightGridRowData()
{
    char *rawData; // [esp+0h] [ebp-4h]

    rawData = Com_GetBspLump(LUMP_LIGHTGRIDROWS, 1u, &s_world.lightGrid.rawRowDataSize);
    if ( s_world.lightGrid.rawRowDataSize )
    {
        s_world.lightGrid.rawRowData = Hunk_Alloc(s_world.lightGrid.rawRowDataSize, "R_LoadLightGridRowData", 21);
        Com_Memcpy(s_world.lightGrid.rawRowData, rawData, s_world.lightGrid.rawRowDataSize);
    }
    else
    {
        s_world.lightGrid.rawRowData = 0;
    }
}

void __cdecl R_LoadLightGridPoints_Version15(unsigned int bspVersion)
{
    unsigned __int8 v1; // [esp+8h] [ebp-2E4h]
    unsigned int dstEntryIndex; // [esp+1CCh] [ebp-120h]
    unsigned int bestDefaultScore; // [esp+1D0h] [ebp-11Ch]
    GfxCompressedLightGridColors swapColors; // [esp+1D4h] [ebp-118h] BYREF
    float worldMaxs[3]; // [esp+27Ch] [ebp-70h] BYREF
    AnnotatedLightGridPoint *points; // [esp+288h] [ebp-64h]
    unsigned int entryIndex; // [esp+28Ch] [ebp-60h]
    unsigned int *defaultScore; // [esp+290h] [ebp-5Ch]
    unsigned __int8 needsTrace; // [esp+297h] [ebp-55h]
    unsigned int cornerIndex; // [esp+298h] [ebp-54h]
    const GfxLightGridEntry_Version15 *diskEntries; // [esp+29Ch] [ebp-50h]
    unsigned int colorsIndex; // [esp+2A0h] [ebp-4Ch]
    unsigned int rowCount; // [esp+2A4h] [ebp-48h]
    const DiskGfxCell *diskCells; // [esp+2A8h] [ebp-44h]
    unsigned int diskCellCount; // [esp+2ACh] [ebp-40h] BYREF
    float worldPos[3]; // [esp+2B0h] [ebp-3Ch] BYREF
    unsigned int entryCount; // [esp+2BCh] [ebp-30h] BYREF
    unsigned int defaultColorsIndex; // [esp+2C0h] [ebp-2Ch]
    unsigned int diskCellIndex; // [esp+2C4h] [ebp-28h]
    const DiskGfxCell_Version14 *diskCellsV14; // [esp+2C8h] [ebp-24h]
    float worldMins[3]; // [esp+2CCh] [ebp-20h] BYREF
    unsigned __int8 needsTraceSwizzle[2][8]; // [esp+2D8h] [ebp-14h] BYREF

    *(_QWORD *)&needsTraceSwizzle[0][0] = 0x703050106020400LL;
    *(_QWORD *)&needsTraceSwizzle[1][0] = 0x705030106040200LL;
    diskEntries = (const GfxLightGridEntry_Version15 *)Com_GetBspLump(LUMP_LIGHTGRIDENTRIES, 8u, &entryCount);
    if ( entryCount )
    {
        if ( bspVersion > 0xE )
        {
            diskCells = (const DiskGfxCell *)Com_GetBspLump(LUMP_CELLS, 0x70u, &diskCellCount);
            ClearBounds(worldMins, worldMaxs);
            for ( diskCellIndex = 0; diskCellIndex < diskCellCount; ++diskCellIndex )
                ExpandBounds(diskCells[diskCellIndex].mins, diskCells[diskCellIndex].maxs, worldMins, worldMaxs);
        }
        else
        {
            diskCellsV14 = (const DiskGfxCell_Version14 *)Com_GetBspLump(LUMP_CELLS, 0x34u, &diskCellCount);
            ClearBounds(worldMins, worldMaxs);
            for ( diskCellIndex = 0; diskCellIndex < diskCellCount; ++diskCellIndex )
                ExpandBounds(diskCellsV14[diskCellIndex].mins, diskCellsV14[diskCellIndex].maxs, worldMins, worldMaxs);
        }
        if ( !diskCellCount )
        {
            worldMins[0] = -131072.0f;
            worldMins[1] = -131072.0f;
            worldMins[2] = -131072.0f;
            worldMaxs[0] = 131072.0f;
            worldMaxs[1] = 131072.0f;
            worldMaxs[2] = 131072.0f;
        }
        s_world.lightGrid.sunPrimaryLightIndex = s_world.sunPrimaryLightIndex;
        s_world.lightGrid.mins[0] = -1;
        s_world.lightGrid.mins[1] = -1;
        s_world.lightGrid.mins[2] = -1;
        s_world.lightGrid.maxs[0] = 0;
        s_world.lightGrid.maxs[1] = 0;
        s_world.lightGrid.maxs[2] = 0;
        points = (AnnotatedLightGridPoint *)Hunk_AllocateTempMemory(10 * entryCount, "R_LoadLightGridPoints_Version15");
        defaultScore = Hunk_AllocateTempMemory(4 * s_world.lightGrid.colorCount, "R_LoadLightGridPoints_Version15");
        memset(defaultScore, 0, 4 * s_world.lightGrid.colorCount);
        dstEntryIndex = 0;
        for ( entryIndex = 0; entryIndex < entryCount; ++entryIndex )
        {
            if ( !entryIndex
                || diskEntries[entryIndex].xyzHighBits != diskEntries[entryIndex - 1].xyzHighBits
                || ((diskEntries[entryIndex - 1].xyzLowBitsAndPrimaryVis
                     ^ diskEntries[entryIndex].xyzLowBitsAndPrimaryVis)
                    & 0xFFFFFFFE) != 0 )
            {
                points[dstEntryIndex].entry.colorsIndex = diskEntries[entryIndex].colorsIndex;
                v1 = (diskEntries[entryIndex].xyzLowBitsAndPrimaryVis & 1) != 0
                     ? LOBYTE(s_world.lightGrid.sunPrimaryLightIndex)
                     : 0;
                points[dstEntryIndex].entry.primaryLightIndex = v1;
                points[dstEntryIndex].entry.needsTrace = diskEntries[entryIndex].needsTrace;
                points[dstEntryIndex].pos[0] = ((int)diskEntries[entryIndex].xyzLowBitsAndPrimaryVis >> 6) & 3
                                                                         | (diskEntries[entryIndex].xyzHighBits >> 19) & 0x1FFC;
                points[dstEntryIndex].pos[1] = ((int)diskEntries[entryIndex].xyzLowBitsAndPrimaryVis >> 4) & 3
                                                                         | (diskEntries[entryIndex].xyzHighBits >> 8) & 0x1FFC;
                points[dstEntryIndex].pos[2] = ((int)diskEntries[entryIndex].xyzLowBitsAndPrimaryVis >> 2) & 3
                                                                         | (4 * diskEntries[entryIndex].xyzHighBits) & 0xFFC;
                worldPos[0] = (float)(32 * points[dstEntryIndex].pos[0] - 0x20000);
                worldPos[1] = (float)(32 * points[dstEntryIndex].pos[1] - 0x20000);
                worldPos[2] = (float)((points[dstEntryIndex].pos[2] << 6) - 0x20000);
                if ( PointInBounds(worldPos, worldMins, worldMaxs) )
                {
                    if ( s_world.lightGrid.mins[0] > (int)points[dstEntryIndex].pos[0] )
                        s_world.lightGrid.mins[0] = points[dstEntryIndex].pos[0];
                    if ( s_world.lightGrid.maxs[0] < (int)points[dstEntryIndex].pos[0] )
                        s_world.lightGrid.maxs[0] = points[dstEntryIndex].pos[0];
                    if ( s_world.lightGrid.mins[1] > (int)points[dstEntryIndex].pos[1] )
                        s_world.lightGrid.mins[1] = points[dstEntryIndex].pos[1];
                    if ( s_world.lightGrid.maxs[1] < (int)points[dstEntryIndex].pos[1] )
                        s_world.lightGrid.maxs[1] = points[dstEntryIndex].pos[1];
                    if ( s_world.lightGrid.mins[2] > (int)points[dstEntryIndex].pos[2] )
                        s_world.lightGrid.mins[2] = points[dstEntryIndex].pos[2];
                    if ( s_world.lightGrid.maxs[2] < (int)points[dstEntryIndex].pos[2] )
                        s_world.lightGrid.maxs[2] = points[dstEntryIndex].pos[2];
                    if ( points[dstEntryIndex].entry.primaryLightIndex == s_world.lightGrid.sunPrimaryLightIndex )
                        ++defaultScore[points[dstEntryIndex].entry.colorsIndex];
                    ++dstEntryIndex;
                }
            }
        }
        entryCount = dstEntryIndex;
        if ( dstEntryIndex )
        {
            bestDefaultScore = 0;
            defaultColorsIndex = 0;
            for ( colorsIndex = 0; colorsIndex < s_world.lightGrid.colorCount; ++colorsIndex )
            {
                if ( bestDefaultScore < defaultScore[colorsIndex] )
                {
                    bestDefaultScore = defaultScore[colorsIndex];
                    defaultColorsIndex = colorsIndex;
                }
            }
            memcpy(&swapColors, &s_world.lightGrid.colors[defaultColorsIndex], sizeof(swapColors));
            memcpy(
                &s_world.lightGrid.colors[defaultColorsIndex],
                s_world.lightGrid.colors,
                sizeof(s_world.lightGrid.colors[defaultColorsIndex]));
            memcpy(s_world.lightGrid.colors, &swapColors, sizeof(GfxCompressedLightGridColors));
            for ( entryIndex = 0; entryIndex < entryCount; ++entryIndex )
            {
                if ( points[entryIndex].entry.colorsIndex )
                {
                    if ( points[entryIndex].entry.colorsIndex == defaultColorsIndex )
                        points[entryIndex].entry.colorsIndex = 0;
                }
                else
                {
                    points[entryIndex].entry.colorsIndex = defaultColorsIndex;
                }
            }
            Hunk_FreeTempMemory((char *)defaultScore);
            defaultScore = 0;
            if ( s_world.lightGrid.maxs[1] - s_world.lightGrid.mins[1] >= s_world.lightGrid.maxs[0]
                                                                                                                                    - s_world.lightGrid.mins[0] )
            {
                s_world.lightGrid.rowAxis = 0;
                s_world.lightGrid.colAxis = 1;
            }
            else
            {
                s_world.lightGrid.rowAxis = 1;
                s_world.lightGrid.colAxis = 0;
            }

            //std::_Sort<AnnotatedLightGridPoint *,int,bool (__cdecl *)(AnnotatedLightGridPoint const &,AnnotatedLightGridPoint const &)>(
            //    points,
            //    &points[entryCount],
            //    (int)(10 * entryCount) / 10,
            //    R_AnnotatedLightGridPointSortsBefore);

            std::sort(points, points + entryCount, R_AnnotatedLightGridPointSortsBefore);

            for ( entryIndex = 0; entryIndex < entryCount; ++entryIndex )
            {
                if ( points[entryIndex].entry.needsTrace )
                {
                    needsTrace = 0;
                    for ( cornerIndex = 0; cornerIndex < 8; ++cornerIndex )
                    {
                        if ( ((1 << cornerIndex) & points[entryIndex].entry.needsTrace) != 0 )
                            needsTrace |= 1 << needsTraceSwizzle[s_world.lightGrid.rowAxis][cornerIndex];
                    }
                    points[entryIndex].entry.needsTrace = needsTrace;
                }
            }
            rowCount = s_world.lightGrid.maxs[s_world.lightGrid.rowAxis]
                             - s_world.lightGrid.mins[s_world.lightGrid.rowAxis]
                             + 1;
            s_world.lightGrid.rowDataStart = (unsigned __int16 *)Hunk_Alloc(2 * rowCount, "R_LoadLightGridHeader", 21);
            memset(s_world.lightGrid.rowDataStart, 0xFFu, 2 * rowCount);
            s_world.lightGrid.rawRowData = Hunk_Alloc(0x40000u, "R_LoadLightGridRowData", 21);
            s_world.lightGrid.rawRowDataSize = 0;
            s_world.lightGrid.entries = (GfxLightGridEntry *)Hunk_Alloc(8 * entryCount, "R_LoadLightGridPoints", 21);
            s_world.lightGrid.entryCount = 0;
            if ( !R_EncodeLightGrid_Version15(points, entryCount) )
            {
                s_world.lightGrid.entryCount = 0;
                s_world.lightGrid.mins[0] = 0;
                s_world.lightGrid.mins[1] = 0;
                s_world.lightGrid.mins[2] = 0;
                s_world.lightGrid.maxs[0] = 0;
                s_world.lightGrid.maxs[1] = 0;
                s_world.lightGrid.maxs[2] = 0;
            }
            Hunk_FreeTempMemory((char *)points);
        }
        else
        {
            Hunk_FreeTempMemory((char *)defaultScore);
            Hunk_FreeTempMemory((char *)points);
            R_InitEmptyLightGrid();
        }
    }
    else
    {
        R_InitEmptyLightGrid();
    }
}

int R_InitEmptyLightGrid()
{
    int result; // eax

    s_world.lightGrid.mins[0] = 0;
    s_world.lightGrid.mins[1] = 0;
    s_world.lightGrid.mins[2] = 0;
    s_world.lightGrid.maxs[0] = 0;
    s_world.lightGrid.maxs[1] = 0;
    s_world.lightGrid.maxs[2] = 0;
    s_world.lightGrid.rowAxis = 0;
    s_world.lightGrid.colAxis = 1;
    s_world.lightGrid.rowDataStart = (unsigned __int16 *)Hunk_Alloc(2u, "R_InitEmptyLightGrid", 21);
    result = 0xFFFF;
    *s_world.lightGrid.rowDataStart = -1;
    return result;
}

bool __cdecl R_AnnotatedLightGridPointSortsBefore(const AnnotatedLightGridPoint &p0, const AnnotatedLightGridPoint &p1)
{
    if ( p0.pos[s_world.lightGrid.rowAxis] < (int)p1.pos[s_world.lightGrid.rowAxis] )
        return 1;
    if ( p0.pos[s_world.lightGrid.rowAxis] > (int)p1.pos[s_world.lightGrid.rowAxis] )
        return 0;
    if ( p0.pos[s_world.lightGrid.colAxis] < (int)p1.pos[s_world.lightGrid.colAxis] )
        return 1;
    if ( p0.pos[s_world.lightGrid.colAxis] <= (int)p1.pos[s_world.lightGrid.colAxis] )
        return p0.pos[2] < (int)p1.pos[2];
    return 0;
}

char __cdecl R_EncodeLightGrid_Version15(const AnnotatedLightGridPoint *pointsArray, unsigned int pointsArrayCount)
{
    unsigned int pointIndex; // [esp+0h] [ebp-18h]
    unsigned __int16 zRange[2]; // [esp+4h] [ebp-14h] BYREF
    unsigned int pointCount; // [esp+8h] [ebp-10h]
    unsigned __int16 row; // [esp+Ch] [ebp-Ch]
    const AnnotatedLightGridPoint *point; // [esp+10h] [ebp-8h]
    unsigned __int16 rowIndex; // [esp+14h] [ebp-4h]

    if ( s_world.lightGrid.entryCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    1692,
                    0,
                    "%s",
                    "s_world.lightGrid.entryCount == 0") )
    {
        __debugbreak();
    }
    for ( pointIndex = 0; pointIndex < pointsArrayCount; pointIndex += pointCount )
    {
        point = &pointsArray[pointIndex];
        row = point->pos[s_world.lightGrid.rowAxis];
        zRange[0] = point->pos[2];
        zRange[1] = point->pos[2];
        for ( pointCount = 1; pointCount + pointIndex < pointsArrayCount; ++pointCount )
        {
            point = &pointsArray[pointCount + pointIndex];
            if ( point->pos[s_world.lightGrid.rowAxis] != row )
                break;
            if ( zRange[0] > (int)point->pos[2] )
                zRange[0] = point->pos[2];
            if ( zRange[1] < (int)point->pos[2] )
                zRange[1] = point->pos[2];
        }
        rowIndex = row - s_world.lightGrid.mins[s_world.lightGrid.rowAxis];
        s_world.lightGrid.rowDataStart[rowIndex] = s_world.lightGrid.rawRowDataSize >> 2;
        if ( 4 * s_world.lightGrid.rowDataStart[rowIndex] != s_world.lightGrid.rawRowDataSize
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        1713,
                        1,
                        "static_cast< uint >( s_world.lightGrid.rowDataStart[rowIndex] * 4 ) == s_world.lightGrid.rawRowDataSize\n"
                        "\t%i, %i",
                        4 * s_world.lightGrid.rowDataStart[rowIndex],
                        s_world.lightGrid.rawRowDataSize) )
        {
            __debugbreak();
        }
        if ( !R_CompressLightGridRow_Version15(pointsArray, pointIndex, pointCount + pointIndex, zRange) )
            return 0;
        if ( s_world.lightGrid.rowDataStart[rowIndex] == s_world.lightGrid.rawRowDataSize
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        1716,
                        0,
                        "%s",
                        "s_world.lightGrid.rowDataStart[rowIndex] != s_world.lightGrid.rawRowDataSize") )
        {
            __debugbreak();
        }
    }
    if ( s_world.lightGrid.entryCount < pointsArrayCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    1719,
                    0,
                    "s_world.lightGrid.entryCount >= pointsArrayCount\n\t%i, %i",
                    s_world.lightGrid.entryCount,
                    pointsArrayCount) )
    {
        __debugbreak();
    }
    return 1;
}

char __cdecl R_CompressLightGridRow_Version15(
                const AnnotatedLightGridPoint *pointsArray,
                unsigned int beginRow,
                unsigned int endRow,
                unsigned __int16 *zRangeGlobal)
{
    unsigned __int16 run; // [esp+0h] [ebp-2Ch]
    unsigned __int16 zSubRangeRun[2]; // [esp+4h] [ebp-28h] BYREF
    unsigned int beginBlock; // [esp+8h] [ebp-24h]
    GfxLightGridRow rowHeader; // [esp+Ch] [ebp-20h]
    unsigned __int16 zSubRange[2]; // [esp+18h] [ebp-14h]
    unsigned int beginCol; // [esp+1Ch] [ebp-10h]
    unsigned int endCol; // [esp+20h] [ebp-Ch]
    unsigned __int16 colRun; // [esp+24h] [ebp-8h]
    unsigned __int16 col; // [esp+28h] [ebp-4h]

    rowHeader.firstEntry = s_world.lightGrid.entryCount;
    rowHeader.colStart = pointsArray[beginRow].pos[s_world.lightGrid.colAxis];
    rowHeader.colCount = pointsArray[endRow - 1].pos[s_world.lightGrid.colAxis] - rowHeader.colStart + 1;
    rowHeader.zStart = *zRangeGlobal;
    rowHeader.zCount = zRangeGlobal[1] - rowHeader.zStart + 1;
    *(GfxLightGridRow *)&s_world.lightGrid.rawRowData[s_world.lightGrid.rawRowDataSize] = rowHeader;
    s_world.lightGrid.rawRowDataSize += 12;
    beginBlock = 0;
    zSubRangeRun[0] = -1;
    zSubRangeRun[1] = 0;
    colRun = -1;
    run = 0;
    for ( beginCol = beginRow; beginCol != endRow; beginCol = endCol )
    {
        col = pointsArray[beginCol].pos[s_world.lightGrid.colAxis];
        for ( endCol = beginCol + 1; endCol != endRow && pointsArray[endCol].pos[s_world.lightGrid.colAxis] == col; ++endCol )
            ;
        zSubRange[0] = pointsArray[beginCol].pos[2];
        zSubRange[1] = pointsArray[endCol - 1].pos[2];
        if ( col == run + colRun && zSubRange[0] == zSubRangeRun[0] && zSubRange[1] == zSubRangeRun[1] && run < 0xFFu )
        {
            ++run;
        }
        else
        {
            if ( run )
            {
                if ( !R_EmitLightGridBlock_Version15(pointsArray, run, zSubRangeRun, zRangeGlobal, beginBlock, beginCol) )
                    return 0;
                if ( col != run + colRun )
                    R_EmitEmptyLightGridBlock_Version15(col - (run + colRun));
            }
            beginBlock = beginCol;
            zSubRangeRun[0] = zSubRange[0];
            zSubRangeRun[1] = zSubRange[1];
            colRun = col;
            run = 1;
        }
    }
    if ( !R_EmitLightGridBlock_Version15(pointsArray, run, zSubRangeRun, zRangeGlobal, beginBlock, endRow) )
        return 0;
    s_world.lightGrid.rawRowDataSize = (s_world.lightGrid.rawRowDataSize + 3) & 0xFFFFFFFC;
    return 1;
}

void __cdecl R_EmitEmptyLightGridBlock_Version15(unsigned int emptyCount)
{
    while ( emptyCount > 0xFF )
    {
        if ( s_world.lightGrid.rawRowDataSize + 2 >= 0x300000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        1522,
                        0,
                        "%s",
                        "s_world.lightGrid.rawRowDataSize + 2 < MAX_MAP_LIGHTGRID_POINTS * 3") )
        {
            __debugbreak();
        }
        s_world.lightGrid.rawRowData[s_world.lightGrid.rawRowDataSize] = -1;
        s_world.lightGrid.rawRowData[s_world.lightGrid.rawRowDataSize + 1] = 0;
        s_world.lightGrid.rawRowDataSize += 2;
        emptyCount -= 255;
    }
    if ( s_world.lightGrid.rawRowDataSize + 2 >= 0x300000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    1529,
                    0,
                    "%s",
                    "s_world.lightGrid.rawRowDataSize + 2 < MAX_MAP_LIGHTGRID_POINTS * 3") )
    {
        __debugbreak();
    }
    s_world.lightGrid.rawRowData[s_world.lightGrid.rawRowDataSize] = emptyCount;
    s_world.lightGrid.rawRowData[s_world.lightGrid.rawRowDataSize + 1] = 0;
    s_world.lightGrid.rawRowDataSize += 2;
}

char __cdecl R_EmitLightGridBlock_Version15(
                const AnnotatedLightGridPoint *pointsArray,
                signed int runCount,
                const unsigned __int16 *zSubRange,
                const unsigned __int16 *zRangeGlobal,
                unsigned int beginBlock,
                unsigned int endBlock)
{
    signed int v7; // [esp+0h] [ebp-20h]
    __int16 zBase; // [esp+8h] [ebp-18h]
    unsigned int zOffset; // [esp+Ch] [ebp-14h]
    unsigned int height; // [esp+10h] [ebp-10h]
    unsigned int pointIndex; // [esp+14h] [ebp-Ch]
    unsigned int colOffset; // [esp+18h] [ebp-8h]
    bool zBaseUsesShort; // [esp+1Fh] [ebp-1h]

    height = zSubRange[1] - *zSubRange + 1;
    if ( height <= 0xFF )
    {
        if ( height * runCount < endBlock - beginBlock
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        1570,
                        0,
                        "runCount * height >= endBlock - beginBlock\n\t%i, %i",
                        height * runCount,
                        endBlock - beginBlock) )
        {
            __debugbreak();
        }
        pointIndex = beginBlock;
        for ( colOffset = 0; colOffset < runCount; ++colOffset )
        {
            for ( zOffset = 0; zOffset < height; ++zOffset )
            {
                if ( pointsArray[pointIndex].pos[2] == zOffset + pointsArray[beginBlock].pos[2] )
                {
                    if ( pointsArray[pointIndex].pos[s_world.lightGrid.colAxis] != colOffset
                                                                                                                                             + pointsArray[beginBlock].pos[s_world.lightGrid.colAxis]
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                                    1579,
                                    0,
                                    "%s",
                                    "pointsArray[pointIndex].pos[s_world.lightGrid.colAxis] == pointsArray[beginBlock].pos[s_world.lightGri"
                                    "d.colAxis] + colOffset") )
                    {
                        __debugbreak();
                    }
                    R_EmitLightGridEntry_Version15(&pointsArray[pointIndex++]);
                }
                else
                {
                    R_EmitDefaultLightGridEntry_Version15();
                }
            }
        }
        if ( pointIndex != endBlock
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        1589,
                        0,
                        "%s",
                        "pointIndex == endBlock") )
        {
            __debugbreak();
        }
        zBase = *zSubRange - *zRangeGlobal;
        zBaseUsesShort = zRangeGlobal[1] - *zRangeGlobal + 1 > 255;
        do
        {
            if ( runCount > 255 )
                v7 = 255;
            else
                v7 = runCount;
            if ( s_world.lightGrid.rawRowDataSize + 4 >= 0x40000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            1597,
                            0,
                            "%s",
                            "s_world.lightGrid.rawRowDataSize + 4 < MAX_MAP_LIGHTGRID_ROWDATA") )
            {
                __debugbreak();
            }
            s_world.lightGrid.rawRowData[s_world.lightGrid.rawRowDataSize] = v7;
            s_world.lightGrid.rawRowData[s_world.lightGrid.rawRowDataSize + 1] = height;
            s_world.lightGrid.rawRowData[s_world.lightGrid.rawRowDataSize + 2] = zBase;
            s_world.lightGrid.rawRowDataSize += 3;
            if ( zBaseUsesShort )
                s_world.lightGrid.rawRowData[s_world.lightGrid.rawRowDataSize++] = HIBYTE(zBase);
            runCount -= v7;
        }
        while ( runCount );
        return 1;
    }
    else
    {
        Com_PrintWarning(8, "light grid vertical variation is too extreme -- ignoring light grid");
        return 0;
    }
}

unsigned int R_EmitDefaultLightGridEntry_Version15()
{
    unsigned int result; // eax

    s_world.lightGrid.entries[s_world.lightGrid.entryCount].colorsIndex = 0;
    s_world.lightGrid.entries[s_world.lightGrid.entryCount].primaryLightIndex = s_world.lightGrid.sunPrimaryLightIndex;
    result = s_world.lightGrid.entryCount;
    s_world.lightGrid.entries[s_world.lightGrid.entryCount++].needsTrace = 0;
    return result;
}

void __cdecl R_EmitLightGridEntry_Version15(const AnnotatedLightGridPoint *point)
{
    s_world.lightGrid.entries[s_world.lightGrid.entryCount++] = point->entry;
}

void R_LoadLightGridHeader()
{
    char *header; // [esp+0h] [ebp-Ch]
    unsigned int len; // [esp+4h] [ebp-8h] BYREF
    unsigned int rowCount; // [esp+8h] [ebp-4h]

    header = Com_GetBspLump(LUMP_LIGHTGRIDHEADER, 1u, &len);
    if ( len < 0x14 )
        Com_Error(ERR_DROP, "light grid header is truncated");
    rowCount = *(unsigned __int16 *)&header[2 * *((unsigned int *)header + 3) + 6]
                     - *(unsigned __int16 *)&header[2 * *((unsigned int *)header + 3)]
                     + 1;
    if ( len != 2 * rowCount + 20 )
        Com_Error(ERR_DROP, "light grid header has unexpected size");
    s_world.lightGrid.sunPrimaryLightIndex = s_world.sunPrimaryLightIndex;
    s_world.lightGrid.mins[0] = *(_WORD *)header;
    s_world.lightGrid.mins[1] = *((_WORD *)header + 1);
    s_world.lightGrid.mins[2] = *((_WORD *)header + 2);
    s_world.lightGrid.maxs[0] = *((_WORD *)header + 3);
    s_world.lightGrid.maxs[1] = *((_WORD *)header + 4);
    s_world.lightGrid.maxs[2] = *((_WORD *)header + 5);
    s_world.lightGrid.rowAxis = *((unsigned int *)header + 3);
    s_world.lightGrid.colAxis = *((unsigned int *)header + 4);
    s_world.lightGrid.rowDataStart = (unsigned __int16 *)Hunk_Alloc(2 * rowCount, "R_LoadLightGridHeader", 21);
    Com_Memcpy(s_world.lightGrid.rowDataStart, header + 20, 2 * rowCount);
    if ( s_world.lightGrid.entryCount )
        R_AssertLightGridValid(&s_world.lightGrid);
    else
        R_InitEmptyLightGrid();
}

void __cdecl R_AssertLightGridValid(const GfxLightGrid *lightGrid)
{
    unsigned int rowCount; // [esp+0h] [ebp-Ch]
    const GfxLightGridRow *row; // [esp+4h] [ebp-8h]
    unsigned int rowIndex; // [esp+8h] [ebp-4h]

    if ( lightGrid->mins[0] > (int)lightGrid->maxs[0]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    1933,
                    0,
                    "lightGrid->mins[0] <= lightGrid->maxs[0]\n\t%i, %i",
                    lightGrid->mins[0],
                    lightGrid->maxs[0]) )
    {
        __debugbreak();
    }
    if ( lightGrid->mins[1] > (int)lightGrid->maxs[1]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    1934,
                    0,
                    "lightGrid->mins[1] <= lightGrid->maxs[1]\n\t%i, %i",
                    lightGrid->mins[1],
                    lightGrid->maxs[1]) )
    {
        __debugbreak();
    }
    if ( lightGrid->mins[2] > (int)lightGrid->maxs[2]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    1935,
                    0,
                    "lightGrid->mins[2] <= lightGrid->maxs[2]\n\t%i, %i",
                    lightGrid->mins[2],
                    lightGrid->maxs[2]) )
    {
        __debugbreak();
    }
    rowCount = lightGrid->maxs[lightGrid->rowAxis] + 1 - lightGrid->mins[lightGrid->rowAxis];
    if ( rowCount > 0x2000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    1939,
                    0,
                    "rowCount <= WORLD_SIZE >> GFX_LIGHTGRID_BITS_XY\n\t%i, %i",
                    rowCount,
                    0x2000) )
    {
        __debugbreak();
    }
    for ( rowIndex = 0; rowIndex < rowCount; ++rowIndex )
    {
        if ( lightGrid->rowDataStart[rowIndex] != 0xFFFF )
        {
            if ( 4 * (unsigned int)lightGrid->rowDataStart[rowIndex] >= lightGrid->rawRowDataSize
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            1950,
                            0,
                            "lightGrid->rowDataStart[rowIndex] * 4 doesn't index lightGrid->rawRowDataSize\n\t%i not in [0, %i)",
                            4 * lightGrid->rowDataStart[rowIndex],
                            lightGrid->rawRowDataSize) )
            {
                __debugbreak();
            }
            row = (const GfxLightGridRow *)&lightGrid->rawRowData[4 * lightGrid->rowDataStart[rowIndex]];
            if ( row->firstEntry >= lightGrid->entryCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            1952,
                            0,
                            "row->firstEntry doesn't index lightGrid->entryCount\n\t%i not in [0, %i)",
                            row->firstEntry,
                            lightGrid->entryCount) )
            {
                __debugbreak();
            }
        }
    }
}

void __cdecl R_LoadLightGridColors(unsigned int bspVersion)
{
    float v1; // [esp+30h] [ebp-64h]
    float v2; // [esp+34h] [ebp-60h]
    float v3; // [esp+38h] [ebp-5Ch]
    float v4; // [esp+3Ch] [ebp-58h]
    float v5; // [esp+40h] [ebp-54h]
    float v6; // [esp+44h] [ebp-50h]
    unsigned int j; // [esp+4Ch] [ebp-48h]
    float lum; // [esp+5Ch] [ebp-38h]
    unsigned int loB; // [esp+70h] [ebp-24h]
    unsigned int loR; // [esp+74h] [ebp-20h]
    unsigned int iLum; // [esp+78h] [ebp-1Ch]
    unsigned int i; // [esp+7Ch] [ebp-18h]
    unsigned __int16 *src; // [esp+80h] [ebp-14h]
    GfxCompressedLightGridColors *dst; // [esp+84h] [ebp-10h]
    char *rawColorData; // [esp+88h] [ebp-Ch]
    unsigned int colorIndex; // [esp+90h] [ebp-4h]

    rawColorData = Com_GetBspLump(LUMP_LIGHTGRIDCOLORS, bspVersion > 0xA ? 336 : 24, &s_world.lightGrid.colorCount);
    s_world.lightGrid.colors = (GfxCompressedLightGridColors *)Hunk_Alloc(
                                                                                                                             168 * (s_world.lightGrid.colorCount + 1),
                                                                                                                             "R_LoadLightGridColors",
                                                                                                                             21);
    if ( bspVersion > 0xA )
    {
        printf("GFX: Compressing HDR LightGrid (elemSize:%d) (number of colors:%d)\n", 168, s_world.lightGrid.colorCount);
        src = (unsigned __int16 *)rawColorData;
        dst = s_world.lightGrid.colors;
        for ( i = 0; i < 56 * s_world.lightGrid.colorCount; ++i )
        {
            if ( (float)((float)*src / 1024.0) >= 0.0024411406 )
                v6 = (float)*src / 1024.0;
            else
                v6 = 0.0024411406f;
            if ( (float)((float)src[1] / 1024.0) >= 0.0024411406 )
                v5 = (float)src[1] / 1024.0;
            else
                v5 = 0.0024411406f;
            if ( (float)((float)src[2] / 1024.0) >= 0.0024411406 )
                v4 = (float)src[2] / 1024.0;
            else
                v4 = 0.0024411406f;
            if ( v6 <= 31.875 )
                v3 = v6;
            else
                v3 = 31.875f;
            if ( v5 <= 31.875 )
                v2 = v5;
            else
                v2 = 31.875f;
            if ( v4 <= 31.875 )
                v1 = v4;
            else
                v1 = 31.875f;
            lum = (float)((float)(v3 * 0.25) + (float)(v2 * 0.5)) + (float)(v1 * 0.25);
            loR = colorRound8Bit((__int64)((float)((float)(v3 * 0.25) / lum) * 255.0), 0x3Fu);
            loB = colorRound8Bit((__int64)((float)((float)(v1 * 0.25) / lum) * 255.0), 0x3Fu);
            iLum = colorRound16Bit((__int64)(sqrtf(lum / 31.875) * 65535.0), 0xFFFu);
            dst->rgb[0][0] = loB | ((_BYTE)loR << 6);
            *(_WORD *)&dst->rgb[0][1] = (loB | (loR << 6) | (iLum << 12)) >> 8;
            src += 3;
            dst = (GfxCompressedLightGridColors *)((char *)dst + 3);
        }
        for ( j = 0; j < 0x38; ++j )
        {
            dst->rgb[0][0] = 21;
            dst->rgb[0][1] = 0;
            dst->rgb[0][2] = -99;
            dst = (GfxCompressedLightGridColors *)((char *)dst + 3);
        }
    }
    else
    {
        for (colorIndex = 0; colorIndex < s_world.lightGrid.colorCount; ++colorIndex)
        {
            //BLOPS_NULLSUB();
        }
    }
    ++s_world.lightGrid.colorCount;
}

unsigned int __cdecl colorRound16Bit(unsigned int a, unsigned int b)
{
    return (b * a + 2048 + ((b * a + 2048) >> 16)) >> 16;
}

unsigned __int8 *R_LoadLightGridEntries()
{
    unsigned __int8 *result; // eax
    GfxLightGridEntry *out; // [esp+0h] [ebp-Ch]
    unsigned int entryIndex; // [esp+4h] [ebp-8h]
    char *in; // [esp+8h] [ebp-4h]

    in = Com_GetBspLump(LUMP_LIGHTGRIDENTRIES, 4u, &s_world.lightGrid.entryCount);
    result = Hunk_Alloc(4 * s_world.lightGrid.entryCount, "R_LoadLightGridPoints", 21);
    s_world.lightGrid.entries = (GfxLightGridEntry *)result;
    out = (GfxLightGridEntry *)result;
    entryIndex = 0;
    while ( entryIndex < s_world.lightGrid.entryCount )
    {
        out->colorsIndex = *(_WORD *)in;
        out->primaryLightIndex = in[2];
        out->needsTrace = in[3];
        ++entryIndex;
        result = (unsigned __int8 *)(in + 4);
        in += 4;
        ++out;
    }
    return result;
}

void R_CreateDefaultProbes()
{
    s_world.draw.reflectionProbeCount = 1;
    s_world.draw.reflectionProbes = (GfxReflectionProbe *)Hunk_Alloc(0x18u, "R_CreateDefaultProbe", 21);
    s_world.draw.reflectionProbeTextures = (GfxTexture *)Hunk_Alloc(4u, "R_CreateDefaultProbe", 21);
    R_CreateDefaultProbe();
    rgl.reflectionProbesLoaded = 1;
}

void R_CreateDefaultProbe()
{
    defaultReflectionProbeRawData.origin[0] = 0.0f;
    defaultReflectionProbeRawData.origin[1] = 0.0f;
    defaultReflectionProbeRawData.origin[2] = 0.0f;
    memset(
        (unsigned __int8 *)defaultReflectionProbeRawData.colorCorrectionFilename,
        0,
        sizeof(defaultReflectionProbeRawData.colorCorrectionFilename));
    R_SetBlackProbe(defaultReflectionProbeRawData.pixels, 0x40050u);
    R_GenerateReflectionImages(s_world.draw.reflectionProbes, &defaultReflectionProbeRawData, 1, 0, 256);
}

void __cdecl R_LoadReflectionProbes(unsigned int bspVersion)
{
    char *v1; // eax
    DiskGfxReflectionProbe *v2; // ecx
    const DiskGfxReflectionProbe *reflectionProbeRawData; // [esp+0h] [ebp-10h]
    unsigned int i; // [esp+4h] [ebp-Ch]
    DiskGfxReflectionProbe *reflectionProbeRawData12; // [esp+8h] [ebp-8h]
    char *reflectionProbeRawData11; // [esp+Ch] [ebp-4h]

    if ( bspVersion > 0xB )
    {
        reflectionProbeRawData = (const DiskGfxReflectionProbe *)Com_GetBspLump(
                                                                                                                             LUMP_REFLECTION_PROBES,
                                                                                                                             0x406E0u,
                                                                                                                             &s_world.draw.reflectionProbeCount);
        ++s_world.draw.reflectionProbeCount;
        s_world.draw.reflectionProbes = (GfxReflectionProbe *)Hunk_Alloc(
                                                                                                                        24 * s_world.draw.reflectionProbeCount,
                                                                                                                        "R_LoadReflectionProbes",
                                                                                                                        21);
        s_world.draw.reflectionProbeTextures = (GfxTexture *)Hunk_Alloc(
                                                                                                                     4 * s_world.draw.reflectionProbeCount,
                                                                                                                     "R_LoadReflectionProbes",
                                                                                                                     21);
        R_CreateDummyProbe();
        R_GenerateReflectionImages(
            s_world.draw.reflectionProbes + 1,
            reflectionProbeRawData,
            s_world.draw.reflectionProbeCount - 1,
            1,
            256);
    }
    else
    {
        reflectionProbeRawData11 = Com_GetBspLump(LUMP_REFLECTION_PROBES, 0x4005Cu, &s_world.draw.reflectionProbeCount);
        ++s_world.draw.reflectionProbeCount;
        s_world.draw.reflectionProbes = (GfxReflectionProbe *)Hunk_Alloc(
                                                                                                                        24 * s_world.draw.reflectionProbeCount,
                                                                                                                        "R_LoadReflectionProbes",
                                                                                                                        21);
        s_world.draw.reflectionProbeTextures = (GfxTexture *)Hunk_Alloc(
                                                                                                                     4 * s_world.draw.reflectionProbeCount,
                                                                                                                     "R_LoadReflectionProbes",
                                                                                                                     21);
        reflectionProbeRawData12 = (DiskGfxReflectionProbe *)Hunk_AllocateTempMemory(
                                                                                                                     263904 * s_world.draw.reflectionProbeCount,
                                                                                                                     "R_LoadReflectionProbes");
        for ( i = 0; i < s_world.draw.reflectionProbeCount - 1; ++i )
        {
            memset(
                (unsigned __int8 *)reflectionProbeRawData12[i].colorCorrectionFilename,
                0,
                sizeof(reflectionProbeRawData12[i].colorCorrectionFilename));
            v1 = &reflectionProbeRawData11[262236 * i];
            v2 = &reflectionProbeRawData12[i];
            v2->origin[0] = *(float *)v1;
            v2->origin[1] = *((float *)v1 + 1);
            v2->origin[2] = *((float *)v1 + 2);
            memcpy(v2->pixels, (unsigned __int8 *)v1 + 12, sizeof(v2->pixels));
        }
        R_CreateDefaultProbe();
        R_GenerateReflectionImages(
            s_world.draw.reflectionProbes + 1,
            reflectionProbeRawData12,
            s_world.draw.reflectionProbeCount - 1,
            1,
            256);
        Hunk_FreeTempMemory((char *)reflectionProbeRawData12);
    }
    rgl.reflectionProbesLoaded = 1;
}

void R_CreateDummyProbe()
{
    DiskGfxDummyReflectionProbe reflectionDummyProbeRawData; // [esp+4h] [ebp-E8h] BYREF

    memset(&reflectionDummyProbeRawData, 0, 76);
    R_SetBlackProbe(reflectionDummyProbeRawData.pixels, 0x90u);
    R_GenerateDummyReflectionImages(s_world.draw.reflectionProbes, &reflectionDummyProbeRawData, 1, 0, 4);
}

void __cdecl R_LoadHeroOnlyLights(unsigned int bspVersion)
{
    float *origin; // [esp+0h] [ebp-40h]
    const float *v2; // [esp+4h] [ebp-3Ch]
    float *dir; // [esp+8h] [ebp-38h]
    const float *v4; // [esp+Ch] [ebp-34h]
    float *color; // [esp+10h] [ebp-30h]
    const float *v6; // [esp+14h] [ebp-2Ch]
    float mins[3]; // [esp+18h] [ebp-28h] BYREF
    float maxs[3]; // [esp+24h] [ebp-1Ch] BYREF
    const DiskHeroOnlyLight *src; // [esp+30h] [ebp-10h]
    GfxHeroLight *dest; // [esp+34h] [ebp-Ch]
    unsigned int index; // [esp+38h] [ebp-8h]
    const DiskHeroOnlyLight *rawData; // [esp+3Ch] [ebp-4h]

    s_world.heroLightCount = 0;
    s_world.heroLightTreeCount = 0;
    s_world.heroLights = 0;
    s_world.heroLightTree = 0;
    if ( bspVersion >= 0x2D )
    {
        rawData = (const DiskHeroOnlyLight *)Com_GetBspLump(LUMP_HERO_ONLY_LIGHTS, 0x38u, &s_world.heroLightCount);
        if ( s_world.heroLightCount )
        {
            s_world.heroLightTreeCount = nextPowerOf2(s_world.heroLightCount) - 1;
            s_world.heroLights = (GfxHeroLight *)Hunk_Alloc(56 * s_world.heroLightCount, "R_LoadHeroOnlyLights", 21);
            for ( index = 0; index < s_world.heroLightCount; ++index )
            {
                src = &rawData[index];
                dest = &s_world.heroLights[index];
                dest->type = src->type;
                color = dest->color;
                v6 = src->color;
                dest->color[0] = src->color[0];
                color[1] = v6[1];
                color[2] = v6[2];
                dir = dest->dir;
                v4 = src->dir;
                dest->dir[0] = src->dir[0];
                dir[1] = v4[1];
                dir[2] = v4[2];
                origin = dest->origin;
                v2 = src->origin;
                dest->origin[0] = src->origin[0];
                origin[1] = v2[1];
                origin[2] = v2[2];
                dest->radius = src->radius;
                dest->cosHalfFovOuter = src->cosHalfFovOuter;
                dest->cosHalfFovInner = src->cosHalfFovInner;
                dest->exponent = src->exponent;
            }
            if ( s_world.heroLightTreeCount )
            {
                s_world.heroLightTree = (GfxHeroLightTree *)Hunk_Alloc(
                                                                                                            24 * s_world.heroLightTreeCount,
                                                                                                            "R_LoadHeroOnlyLights",
                                                                                                            21);
                R_BuildHeroLightTree(0, 0, s_world.heroLightCount, mins, maxs);
            }
        }
    }
}

void __cdecl R_BuildHeroLightTree(
                unsigned int nodeIndex,
                unsigned int firstLight,
                unsigned int lightCount,
                float *mins,
                float *maxs)
{
    float v5; // [esp+Ch] [ebp-70h]
    float v6; // [esp+10h] [ebp-6Ch]
    float v7; // [esp+18h] [ebp-64h]
    float v8; // [esp+1Ch] [ebp-60h]
    unsigned int axis; // [esp+24h] [ebp-58h]
    GfxHeroLight *v10; // [esp+28h] [ebp-54h]
    unsigned int n; // [esp+2Ch] [ebp-50h]
    GfxHeroLight *light; // [esp+30h] [ebp-4Ch]
    GfxHeroLightTree *tree; // [esp+34h] [ebp-48h]
    float rightMaxs[3]; // [esp+3Ch] [ebp-40h] BYREF
    unsigned int rightIndex; // [esp+48h] [ebp-34h]
    unsigned int numRight; // [esp+4Ch] [ebp-30h]
    float leftMaxs[3]; // [esp+50h] [ebp-2Ch] BYREF
    unsigned int leftIndex; // [esp+5Ch] [ebp-20h]
    float leftMins[3]; // [esp+60h] [ebp-1Ch] BYREF
    float longestLen; // [esp+6Ch] [ebp-10h]
    float rightMins[3]; // [esp+70h] [ebp-Ch] BYREF

    *mins = FLT_MAX;
    mins[1] = FLT_MAX;
    mins[2] = FLT_MAX;
    *maxs = -FLT_MAX;
    maxs[1] = -FLT_MAX;
    maxs[2] = -FLT_MAX;
    if ( lightCount )
    {
        if ( nodeIndex < s_world.heroLightTreeCount )
        {
            for ( n = 0; n < lightCount; ++n )
            {
                v10 = &s_world.heroLights[n + firstLight];
                Vec3Min(mins, v10->origin, mins);
                Vec3Max(maxs, v10->origin, maxs);
            }
            r_heroLightAxis = 0;
            longestLen = 0.0f;
            for ( axis = 0; axis < 3; ++axis )
            {
                if ( (float)(maxs[axis] - mins[axis]) >= longestLen )
                {
                    longestLen = maxs[axis] - mins[axis];
                    r_heroLightAxis = axis;
                }
            }
            qsort(
                &s_world.heroLights[firstLight],
                lightCount,
                0x38u,
                (int (__cdecl *)(const void *, const void *))R_HeroLightSorter);
            numRight = lightCount >> 1;
            leftIndex = 2 * nodeIndex + 1;
            rightIndex = 2 * nodeIndex + 2;
            R_BuildHeroLightTree(leftIndex, firstLight, lightCount - (lightCount >> 1), leftMins, leftMaxs);
            R_BuildHeroLightTree(
                rightIndex,
                lightCount - (lightCount >> 1) + firstLight,
                lightCount >> 1,
                rightMins,
                rightMaxs);
            if ( nodeIndex >= s_world.heroLightTreeCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            2334,
                            0,
                            "%s",
                            "nodeIndex < s_world.heroLightTreeCount") )
            {
                __debugbreak();
            }
            tree = &s_world.heroLightTree[nodeIndex];
            Vec3Min(leftMins, rightMins, tree->mins);
            Vec3Max(leftMaxs, rightMaxs, tree->maxs);
            *mins = tree->mins[0];
            mins[1] = tree->mins[1];
            mins[2] = tree->mins[2];
            *maxs = tree->maxs[0];
            maxs[1] = tree->maxs[1];
            maxs[2] = tree->maxs[2];
        }
        else
        {
            if ( lightCount != 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            2290,
                            0,
                            "%s",
                            "lightCount == 1") )
            {
                __debugbreak();
            }
            light = &s_world.heroLights[firstLight];
            v7 = light->origin[1] - light->radius;
            v8 = light->origin[2] - light->radius;
            *mins = light->origin[0] - light->radius;
            mins[1] = v7;
            mins[2] = v8;
            v5 = light->origin[1] + light->radius;
            v6 = light->origin[2] + light->radius;
            *maxs = light->origin[0] + light->radius;
            maxs[1] = v5;
            maxs[2] = v6;
        }
    }
}

int __cdecl R_HeroLightSorter(float *a, float *b)
{
    if ( b[r_heroLightAxis + 7] <= a[r_heroLightAxis + 7] )
        return a[r_heroLightAxis + 7] > b[r_heroLightAxis + 7];
    else
        return -1;
}

int __cdecl nextPowerOf2(int v)
{
    int va; // [esp+8h] [ebp+8h]
    int vb; // [esp+8h] [ebp+8h]

    va = (v - 1) | ((v - 1) >> 1) | (((v - 1) | ((v - 1) >> 1)) >> 2);
    vb = va | (va >> 4) | ((va | (va >> 4)) >> 8);
    return (vb | (vb >> 16)) + 1;
}

unsigned int R_LoadCoronas()
{
    unsigned int result; // eax
    float *origin; // [esp+Ch] [ebp-14h]
    float *v2; // [esp+10h] [ebp-10h]
    unsigned int n; // [esp+14h] [ebp-Ch]
    char *src; // [esp+18h] [ebp-8h]
    GfxLightCorona *dest; // [esp+1Ch] [ebp-4h]

    src = Com_GetBspLump(LUMP_LIGHT_CORONAS, 0x20u, &s_world.coronaCount);
    s_world.coronas = (GfxLightCorona *)Hunk_Alloc(32 * s_world.coronaCount, "R_LoadCoronas", 21);
    dest = s_world.coronas;
    for ( n = 0; ; ++n )
    {
        result = n;
        if ( n >= s_world.coronaCount )
            break;
        origin = dest[n].origin;
        v2 = (float *)&src[32 * n];
        *origin = *v2;
        origin[1] = v2[1];
        origin[2] = v2[2];
        origin[4] = v2[4];
        origin[5] = v2[5];
        origin[6] = v2[6];
        origin[3] = v2[3];
        origin[7] = v2[7];
    }
    return result;
}

char *R_LoadOutdoorBounds()
{
    char *result; // eax
    GfxOutdoorBounds *v1; // eax
    float *v2; // [esp+0h] [ebp-10h]
    float *outdoorBoundsRawData; // [esp+Ch] [ebp-4h]

    result = Com_GetBspLump(LUMP_OUTDOORBOUNDS, 0x18u, &s_world.numOutdoorBounds);
    outdoorBoundsRawData = (float *)result;
    if ( s_world.numOutdoorBounds )
    {
        v1 = (GfxOutdoorBounds *)Hunk_Alloc(24 * s_world.numOutdoorBounds, "R_LoadOutdoorBounds", 21);
        s_world.outdoorBounds = v1;
        v1->bounds[0][0] = *outdoorBoundsRawData;
        v1->bounds[0][1] = outdoorBoundsRawData[1];
        v1->bounds[0][2] = outdoorBoundsRawData[2];
        v2 = s_world.outdoorBounds->bounds[1];
        s_world.outdoorBounds->bounds[1][0] = outdoorBoundsRawData[3];
        v2[1] = outdoorBoundsRawData[4];
        result = (char *)(outdoorBoundsRawData + 3);
        v2[2] = outdoorBoundsRawData[5];
    }
    return result;
}

unsigned __int8 *R_LoadOccluders()
{
    unsigned __int8 *result; // eax
    const DiskGfxOccluder *v1; // ecx
    Occluder *out; // [esp+8h] [ebp-10h]
    unsigned int idx; // [esp+10h] [ebp-8h]
    const DiskGfxOccluder *src; // [esp+14h] [ebp-4h]

    src = (const DiskGfxOccluder *)Com_GetBspLump(LUMP_OCCLUDERS, 0x40u, &s_world.numOccluders);
    result = Hunk_Alloc(68 * s_world.numOccluders, "R_LoadOccluders", 21);
    s_world.occluders = (Occluder *)result;
    for ( idx = 0; idx < s_world.numOccluders; ++idx )
    {
        out = &s_world.occluders[idx];
        out->flags = 1;
        result = (unsigned __int8 *)out->name;
        v1 = &src[idx];
        *(unsigned int *)out->name = *(unsigned int *)v1->name;
        *(unsigned int *)&out->name[4] = *(unsigned int *)&v1->name[4];
        *(unsigned int *)&out->name[8] = *(unsigned int *)&v1->name[8];
        *(unsigned int *)&out->name[12] = *(unsigned int *)&v1->name[12];
        memcpy(out->points, v1->pts, sizeof(out->points));
    }
    return result;
}

int R_LoadShadowMapVolumes()
{
    float *plane; // [esp+4h] [ebp-20h]
    float *v2; // [esp+8h] [ebp-1Ch]
    unsigned int j; // [esp+Ch] [ebp-18h]
    unsigned int i; // [esp+10h] [ebp-14h]
    GfxShadowMapVolume *dstVolume; // [esp+14h] [ebp-10h]
    char *srcPlane; // [esp+18h] [ebp-Ch]
    GfxVolumePlane *dstPlane; // [esp+1Ch] [ebp-8h]
    char *srcVolume; // [esp+20h] [ebp-4h]

    srcVolume = Com_GetBspLump(LUMP_SHADOWMAP_VOLUMES, 0x10u, &s_world.shadowMapVolumeCount);
    s_world.shadowMapVolumes = (GfxShadowMapVolume *)Hunk_Alloc(
                                                                                                         16 * s_world.shadowMapVolumeCount,
                                                                                                         "R_LoadShadowMapVolumes",
                                                                                                         21);
    dstVolume = s_world.shadowMapVolumes;
    for ( i = 0; i < s_world.shadowMapVolumeCount; ++i )
    {
        dstVolume[i].control = *(unsigned int *)&srcVolume[16 * i];
        dstVolume[i].padding1 = *(unsigned int *)&srcVolume[16 * i + 4];
        dstVolume[i].padding2 = *(unsigned int *)&srcVolume[16 * i + 8];
        dstVolume[i].padding3 = *(unsigned int *)&srcVolume[16 * i + 12];
    }
    srcPlane = Com_GetBspLump(LUMP_SHADOWMAP_VOLUME_PLANES, 0x10u, &s_world.shadowMapVolumePlaneCount);
    s_world.shadowMapVolumePlanes = (GfxVolumePlane *)Hunk_Alloc(
                                                                                                            16 * s_world.shadowMapVolumePlaneCount,
                                                                                                            "R_LoadShadowMapVolumePlanes",
                                                                                                            21);
    dstPlane = s_world.shadowMapVolumePlanes;
    for ( j = 0; j < s_world.shadowMapVolumePlaneCount; ++j )
    {
        plane = dstPlane[j].plane;
        v2 = (float *)&srcPlane[16 * j];
        *plane = *v2;
        plane[1] = v2[1];
        plane[2] = v2[2];
        plane[3] = v2[3];
    }
    return printf(
                     "GFX: ShadowMapVolumes: %d\tPlanes: %d\n",
                     s_world.shadowMapVolumeCount,
                     s_world.shadowMapVolumePlaneCount);
}

int R_LoadExposureVolumes()
{
    float *plane; // [esp+4h] [ebp-20h]
    float *v2; // [esp+8h] [ebp-1Ch]
    unsigned int j; // [esp+Ch] [ebp-18h]
    unsigned int i; // [esp+10h] [ebp-14h]
    GfxExposureVolume *dstVolume; // [esp+14h] [ebp-10h]
    char *srcPlane; // [esp+18h] [ebp-Ch]
    GfxVolumePlane *dstPlane; // [esp+1Ch] [ebp-8h]
    char *srcVolume; // [esp+20h] [ebp-4h]

    srcVolume = Com_GetBspLump(LUMP_EXPOSURE_VOLUMES, 0x28u, &s_world.exposureVolumeCount);
    s_world.exposureVolumes = (GfxExposureVolume *)Hunk_Alloc(
                                                                                                     24 * s_world.exposureVolumeCount,
                                                                                                     "R_LoadExposureVolumes",
                                                                                                     21);
    dstVolume = s_world.exposureVolumes;
    for ( i = 0; i < s_world.exposureVolumeCount; ++i )
    {
        dstVolume[i].control = *(unsigned int *)&srcVolume[40 * i];
        dstVolume[i].exposure = *(float *)&srcVolume[40 * i + 4];
        dstVolume[i].luminanceIncreaseScale = *(float *)&srcVolume[40 * i + 8];
        dstVolume[i].luminanceDecreaseScale = *(float *)&srcVolume[40 * i + 12];
        dstVolume[i].featherRange = *(float *)&srcVolume[40 * i + 16];
        dstVolume[i].featherAdjust = *(float *)&srcVolume[40 * i + 20];
    }
    srcPlane = Com_GetBspLump(LUMP_EXPOSURE_VOLUME_PLANES, 0x10u, &s_world.exposureVolumePlaneCount);
    s_world.exposureVolumePlanes = (GfxVolumePlane *)Hunk_Alloc(
                                                                                                         16 * s_world.exposureVolumePlaneCount,
                                                                                                         "R_LoadExposureVolumePlanes",
                                                                                                         21);
    dstPlane = s_world.exposureVolumePlanes;
    for ( j = 0; j < s_world.exposureVolumePlaneCount; ++j )
    {
        plane = dstPlane[j].plane;
        v2 = (float *)&srcPlane[16 * j];
        *plane = *v2;
        plane[1] = v2[1];
        plane[2] = v2[2];
        plane[3] = v2[3];
    }
    return printf("GFX: ExpsoureVolumes: %d\tPlanes: %d\n", s_world.exposureVolumeCount, s_world.exposureVolumePlaneCount);
}

unsigned __int8 *R_LoadWorldLodData()
{
    unsigned __int8 *result; // eax
    GfxWorldLodChain *v1; // [esp+4h] [ebp-20h]
    char *v2; // [esp+8h] [ebp-1Ch]
    unsigned int k; // [esp+Ch] [ebp-18h]
    unsigned int j; // [esp+10h] [ebp-14h]
    unsigned int i; // [esp+14h] [ebp-10h]
    char *lodInfos; // [esp+18h] [ebp-Ch]
    char *lodSurfs; // [esp+1Ch] [ebp-8h]
    char *lodChains; // [esp+20h] [ebp-4h]

    lodChains = Com_GetBspLump(LUMP_LODCHAINS, 0x18u, &s_world.worldLodChainCount);
    if ( !lodChains || !s_world.worldLodChainCount )
    {
        s_world.worldLodChains = 0;
        s_world.worldLodChainCount = 0;
        s_world.worldLodInfos = 0;
        s_world.worldLodInfoCount = 0;
        s_world.worldLodSurfaces = 0;
        s_world.worldLodSurfaceCount = 0;
    }
    s_world.worldLodChains = (GfxWorldLodChain *)Hunk_Alloc(24 * s_world.worldLodChainCount, "R_LoadWorldLodData", 21);
    lodInfos = Com_GetBspLump(LUMP_LODINFOS, 0xCu, &s_world.worldLodInfoCount);
    s_world.worldLodInfos = (GfxWorldLodInfo *)Hunk_Alloc(12 * s_world.worldLodInfoCount, "R_LoadWorldLodData", 21);
    lodSurfs = Com_GetBspLump(LUMP_LODSURFACES, 4u, &s_world.worldLodSurfaceCount);
    result = Hunk_Alloc(4 * s_world.worldLodSurfaceCount, "R_LoadWorldLodData", 21);
    s_world.worldLodSurfaces = (unsigned int *)result;
    for ( i = 0; i < s_world.worldLodChainCount; ++i )
    {
        v1 = &s_world.worldLodChains[i];
        v2 = &lodChains[24 * i];
        v1->origin[0] = *(float *)v2;
        v1->origin[1] = *((float *)v2 + 1);
        v1->origin[2] = *((float *)v2 + 2);
        s_world.worldLodChains[i].lastDist = *((float *)v2 + 3);
        s_world.worldLodChains[i].firstLodInfo = *((unsigned int *)v2 + 4);
        s_world.worldLodChains[i].lodInfoCount = *((_WORD *)v2 + 10);
        result = (unsigned __int8 *)(i + 1);
    }
    for ( j = 0; j < s_world.worldLodInfoCount; ++j )
    {
        s_world.worldLodInfos[j].dist = *(float *)&lodInfos[12 * j];
        s_world.worldLodInfos[j].firstSurf = *(unsigned int *)&lodInfos[12 * j + 4];
        s_world.worldLodInfos[j].surfCount = *(_WORD *)&lodInfos[12 * j + 8];
        result = (unsigned __int8 *)(j + 1);
    }
    for ( k = 0; k < s_world.worldLodSurfaceCount; ++k )
    {
        result = (unsigned __int8 *)k;
        s_world.worldLodSurfaces[k] = *(unsigned int *)&lodSurfs[4 * k];
    }
    return result;
}

void __cdecl R_LoadMaterials(GfxBspLoad *load)
{
    load->diskMaterials = (const dmaterial_t *)Com_GetBspLump(LUMP_MATERIALS, 0x48u, &load->materialCount);
}

signed int R_SortSurfaces()
{
    signed int result; // eax
    unsigned __int8 *v1; // [esp+0h] [ebp-7Ch]
    int origSurfIndex; // [esp+6Ch] [ebp-10h]
    int surfIndex; // [esp+70h] [ebp-Ch]
    int surfIndexa; // [esp+70h] [ebp-Ch]
    signed int surfIndexb; // [esp+70h] [ebp-Ch]
    signed int surfaceCount; // [esp+74h] [ebp-8h]
    signed int surfaceCounta; // [esp+74h] [ebp-8h]
    GfxSurface *surface; // [esp+78h] [ebp-4h]

    if ( s_world.modelCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    3103,
                    0,
                    "%s\n\t(s_world.modelCount) = %i",
                    "(s_world.modelCount > 0)",
                    s_world.modelCount) )
    {
        __debugbreak();
    }
    if ( s_world.models->startSurfIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    3104,
                    0,
                    "%s\n\t(s_world.models[0].startSurfIndex) = %i",
                    "(s_world.models[0].startSurfIndex == 0)",
                    s_world.models->startSurfIndex) )
    {
        __debugbreak();
    }
    surfaceCount = s_world.models->surfaceCount;
    if ( surfaceCount )
        v1 = Hunk_Alloc(2 * surfaceCount, "R_InitDynamicData", 21);
    else
        v1 = 0;
    s_world.dpvs.sortedSurfIndex = (unsigned __int16 *)v1;
    for ( surfIndex = 0; surfIndex < surfaceCount; ++surfIndex )
    {
        surface = &s_world.dpvs.surfaces[surfIndex];
        s_world.dpvs.sortedSurfIndex[surfIndex] = surface->tris.vertexCount;
        surface->tris.vertexCount = surfIndex;
        if ( surface->tris.vertexCount != surfIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        3115,
                        0,
                        "%s",
                        "surface->tris.vertexCount == surfIndex") )
        {
            __debugbreak();
        }
    }
    //std::_Sort<GfxSurface *,int,bool (__cdecl *)(GfxSurface const &,GfxSurface const &)>(
    //    s_world.dpvs.surfaces,
    //    &s_world.dpvs.surfaces[surfaceCount],
    //    80 * surfaceCount / 80,
    //    (unsigned __int8 (*)(void))R_CompareSurfaces);

    std::sort(s_world.dpvs.surfaces, s_world.dpvs.surfaces + surfaceCount, R_CompareSurfaces);

    for ( surfIndexa = 0; surfIndexa < surfaceCount; ++surfIndexa )
    {
        origSurfIndex = s_world.dpvs.surfaces[surfIndexa].tris.vertexCount;
        s_world.dpvs.surfaces[surfIndexa].tris.vertexCount = s_world.dpvs.sortedSurfIndex[origSurfIndex];
        s_world.dpvs.sortedSurfIndex[origSurfIndex] = surfIndexa;
        if ( s_world.dpvs.sortedSurfIndex[origSurfIndex] != surfIndexa
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        3127,
                        0,
                        "%s",
                        "s_world.dpvs.sortedSurfIndex[origSurfIndex] == surfIndex") )
        {
            __debugbreak();
        }
    }
    surfIndexb = 0;
    surfaceCounta = s_world.models->surfaceCount;
    s_world.dpvs.litSurfsBegin = 0;
    while ( surfIndexb < surfaceCounta
             && s_world.dpvs.surfaces[surfIndexb].material->localTechniqueSet
             && (Material_GetTechnique(s_world.dpvs.surfaces[surfIndexb].material, 0xAu)
                || Material_GetTechnique(s_world.dpvs.surfaces[surfIndexb].material, 2u))
             && s_world.dpvs.surfaces[surfIndexb].material->info.sortKey < 0x18u )
        ++surfIndexb;
    s_world.dpvs.litSurfsEnd = surfIndexb;
    s_world.dpvs.decalSurfsBegin = surfIndexb;
    while ( surfIndexb < surfaceCounta
             && s_world.dpvs.surfaces[surfIndexb].material->localTechniqueSet
             && Material_GetTechnique(s_world.dpvs.surfaces[surfIndexb].material, 0xAu) )
    {
        if ( s_world.dpvs.surfaces[surfIndexb].material->info.sortKey < 0x18u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        3328,
                        0,
                        "%s",
                        "s_world.dpvs.surfaces[surfIndex].material->info.sortKey >= SORTKEY_DECAL") )
        {
            __debugbreak();
        }
        ++surfIndexb;
    }
    s_world.dpvs.decalSurfsEnd = surfIndexb;
    s_world.dpvs.emissiveSurfsBegin = surfIndexb;
    while ( surfIndexb < surfaceCounta
             && s_world.dpvs.surfaces[surfIndexb].material->localTechniqueSet
             && Material_GetTechnique(s_world.dpvs.surfaces[surfIndexb].material, 5u) )
        ++surfIndexb;
    result = surfIndexb;
    s_world.dpvs.emissiveSurfsEnd = surfIndexb;
    return result;
}

bool __cdecl R_CompareSurfaces(const GfxSurface &surf0, const GfxSurface &surf1)
{
    int v3; // [esp+0h] [ebp-ACh]
    int v4; // [esp+4h] [ebp-A8h]
    int materialSortedIndex[2]; // [esp+38h] [ebp-74h]
    const MaterialTechnique *techniqueBuildDepth[2]; // [esp+40h] [ebp-6Ch]
    int surfIndex[2]; // [esp+50h] [ebp-5Ch]
    const MaterialTechnique *techniqueLit[2]; // [esp+58h] [ebp-54h]
    Material *material[2]; // [esp+60h] [ebp-4Ch]
    int firstVertex[2]; // [esp+68h] [ebp-44h]
    int reflectionProbeIndex[2]; // [esp+78h] [ebp-34h]
    int hasTechniqueEmissive[2]; // [esp+80h] [ebp-2Ch]
    int primarySortKey[2]; // [esp+88h] [ebp-24h]
    int lightmapIndex[2]; // [esp+98h] [ebp-14h]
    MaterialTechniqueSet *techSet[2]; // [esp+A0h] [ebp-Ch]
    int comparison; // [esp+A8h] [ebp-4h]

    material[0] = surf0.material;
    material[1] = surf1.material;

    techSet[0] = Material_GetTechniqueSet(material[0]);
    techSet[1] = Material_GetTechniqueSet(material[1]);

    iassert(techSet[0] && techSet[1]);

    techniqueLit[0] = Material_GetTechnique(material[0], 0xAu);
    techniqueLit[1] = Material_GetTechnique(material[1], 0xAu);
    techniqueBuildDepth[0] = Material_GetTechnique(material[0], 2u);
    techniqueBuildDepth[1] = Material_GetTechnique(material[1], 2u);

    v4 = techniqueLit[0] || techniqueBuildDepth[0];
    v3 = techniqueLit[1] || techniqueBuildDepth[1];
    if ( v3 != v4 )
        return v3 - v4 < 0;

    if ( !v4 )
    {
        hasTechniqueEmissive[0] = Material_GetTechnique(material[0], 5u) != 0;
        hasTechniqueEmissive[1] = Material_GetTechnique(material[1], 5u) != 0;
        if (hasTechniqueEmissive[1] != hasTechniqueEmissive[0])
            return hasTechniqueEmissive[1] - hasTechniqueEmissive[0] < 0;
    }
    primarySortKey[0] = material[0]->info.drawSurf.fields.primarySortKey;
    primarySortKey[1] = material[1]->info.drawSurf.fields.primarySortKey;
    if ( primarySortKey[0] != primarySortKey[1])
        return primarySortKey[0] - primarySortKey[1] < 0;
    Com_GetPrimaryLight(surf0.primaryLightIndex);
    Com_GetPrimaryLight(surf1.primaryLightIndex);
    comparison = surf0.primaryLightIndex - surf1.primaryLightIndex;
    if ( comparison )
        return comparison < 0;
    materialSortedIndex[0] = material[0]->info.drawSurf.fields.materialSortedIndex;
    materialSortedIndex[1] = material[1]->info.drawSurf.fields.materialSortedIndex;
    if ( materialSortedIndex[0] == materialSortedIndex[1])
    {
        if (material[0] != material[1]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        3041,
                        0,
                        "%s",
                        "material[0] == material[1]") )
        {
            __debugbreak();
        }
        reflectionProbeIndex[0] = surf0.reflectionProbeIndex;
        reflectionProbeIndex[1] = surf1.reflectionProbeIndex;
        if ( reflectionProbeIndex[0] == reflectionProbeIndex[1])
        {
            lightmapIndex[0] = surf0.lightmapIndex;
            lightmapIndex[1] = surf1.lightmapIndex;
            if ( lightmapIndex[0] == lightmapIndex[1] )
            {
                firstVertex[0] = surf0.tris.firstVertex;
                firstVertex[1] = surf1.tris.firstVertex;
                if ( firstVertex[0] == firstVertex[1])
                {
                    surfIndex[0] = surf0.tris.vertexCount;
                    surfIndex[1] = surf1.tris.vertexCount;
                    if (surfIndex[0] == surfIndex[1]
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                                    3065,
                                    0,
                                    "%s",
                                    "comparison") )
                    {
                        __debugbreak();
                    }
                    return surfIndex[0] - surfIndex[1] < 0;
                }
                else
                {
                    return firstVertex[0] - firstVertex[1] < 0;
                }
            }
            else
            {
                return lightmapIndex[0] - lightmapIndex[1] < 0;
            }
        }
        else
        {
            return reflectionProbeIndex[0] - reflectionProbeIndex[1] < 0;
        }
    }
    else
    {
        iassert(surf0.tris.firstVertex != surf1.tris.firstVertex);
        return materialSortedIndex[0] - materialSortedIndex[1] < 0;
    }
}

unsigned int R_CalculateVertexStream2Usage()
{
    Stream2Usage *inserted; // eax
    unsigned int result; // eax
    GfxSurface *v2; // [esp+8h] [ebp-58h]
    int i; // [esp+Ch] [ebp-54h]
    Stream2Usage newUsage; // [esp+10h] [ebp-50h] BYREF
    Stream2Usage *existingUse; // [esp+20h] [ebp-40h]
    int vertexCount; // [esp+24h] [ebp-3Ch]
    const MaterialStreamRouting *routing; // [esp+28h] [ebp-38h]
    unsigned __int8 streamIndex; // [esp+2Fh] [ebp-31h]
    const MaterialPass *pass; // [esp+30h] [ebp-30h]
    unsigned __int16 passIndex; // [esp+34h] [ebp-2Ch]
    const MaterialTechnique *tech; // [esp+38h] [ebp-28h]
    int techIndex; // [esp+3Ch] [ebp-24h]
    const MaterialTechniqueSet *techSet; // [esp+40h] [ebp-20h]
    const Material *material; // [esp+44h] [ebp-1Ch]
    srfTriangles_t *tris; // [esp+48h] [ebp-18h]
    bool surfUsesStream2; // [esp+4Fh] [ebp-11h]
    int surfIndex; // [esp+50h] [ebp-10h]
    unsigned int stream2ByteSize; // [esp+54h] [ebp-Ch]
    Stream2Usage *cur; // [esp+58h] [ebp-8h]
    Stream2Usage *usageList; // [esp+5Ch] [ebp-4h] BYREF

    s_world.draw.vertexStream2DataSize = 0;
    usageList = 0;
    for ( surfIndex = 0; surfIndex < s_world.surfaceCount; ++surfIndex )
    {
        surfUsesStream2 = 0;
        material = s_world.dpvs.surfaces[surfIndex].material;
        if ( material && material->localTechniqueSet )
        {
            techSet = Material_GetTechniqueSet(material);
            for ( techIndex = 0; techIndex < 130 && !surfUsesStream2; ++techIndex )
            {
                tech = techSet->techniques[techIndex];
                if ( tech )
                {
                    for ( passIndex = 0; passIndex < (int)tech->passCount && !surfUsesStream2; ++passIndex )
                    {
                        pass = &tech->passArray[passIndex];
                        for ( streamIndex = 0; streamIndex < (int)pass->vertexDecl->streamCount && !surfUsesStream2; ++streamIndex )
                        {
                            routing = &pass->vertexDecl->routing.data[streamIndex];
                            surfUsesStream2 = routing->source == 9;
                        }
                    }
                }
            }
        }
        tris = &s_world.dpvs.surfaces[surfIndex].tris;
        if ( surfUsesStream2 )
        {
            existingUse = FindExistingUsage(usageList, tris->firstVertex);
            vertexCount = tris->vertexCount;
            if ( existingUse )
            {
                if ( vertexCount > existingUse->vertexCount )
                    existingUse->vertexCount = vertexCount;
                tris->stream2ByteOffset = (int)existingUse;
            }
            else
            {
                newUsage.firstVertex = tris->firstVertex;
                newUsage.byteOffset = -1;
                newUsage.vertexCount = vertexCount;
                newUsage.next = 0;
                inserted = InsertNewUsage(&usageList, &newUsage);
                tris->stream2ByteOffset = (int)inserted;
            }
        }
        else
        {
            tris->stream2ByteOffset = 0;
        }
    }
    stream2ByteSize = 0;
    for ( cur = usageList; cur; cur = cur->next )
    {
        cur->byteOffset = stream2ByteSize;
        stream2ByteSize += 4 * cur->vertexCount;
    }
    for ( i = 0; i < s_world.surfaceCount; ++i )
    {
        v2 = &s_world.dpvs.surfaces[i];
        if ( v2->tris.stream2ByteOffset )
            v2->tris.stream2ByteOffset = *(unsigned int *)(v2->tris.stream2ByteOffset + 4);
        else
            v2->tris.stream2ByteOffset = -1;
    }
    FreeUsageList(&usageList);
    result = stream2ByteSize;
    s_world.draw.vertexStream2DataSize = stream2ByteSize;
    return result;
}

Stream2Usage *__cdecl FindExistingUsage(Stream2Usage *list, int firstVertex)
{
    while ( list )
    {
        if ( list->firstVertex == firstVertex )
            return list;
        list = list->next;
    }
    return 0;
}

Stream2Usage *__cdecl InsertNewUsage(Stream2Usage **list, const Stream2Usage *copy)
{
    Stream2Usage *Usage; // eax

    Usage = AllocateUsage();
    *Usage = *copy;
    Usage->next = *list;
    *list = Usage;
    return *list;
}

Stream2Usage *__cdecl AllocateUsage()
{
    unsigned int *v2; // [esp+4h] [ebp-4h]

    v2 = (unsigned int*)operator new(0x10u);
    if ( !v2 )
        return 0;
    *v2 = -1;
    v2[1] = -1;
    *((_WORD *)v2 + 4) = 0;
    v2[3] = 0;
    return (Stream2Usage *)v2;
}

void __cdecl FreeUsageList(Stream2Usage **list)
{
    Stream2Usage *next; // [esp+0h] [ebp-4h]

    while ( *list )
    {
        next = (*list)->next;
        FreeUsage(*list);
        *list = next;
    }
}

void __cdecl FreeUsage(Stream2Usage *usage)
{
    operator delete(usage);
}

void __cdecl R_LoadSurfaces(GfxBspLoad *load)
{
    float v1; // [esp+4h] [ebp-94h]
    PackedUnitVec v2; // [esp+14h] [ebp-84h]
    PackedUnitVec v3; // [esp+18h] [ebp-80h]
    unsigned __int8 *vertLayerDataMem; // [esp+1Ch] [ebp-7Ch]
    unsigned int firstSurfIndex; // [esp+20h] [ebp-78h]
    unsigned __int8 dummyData[4]; // [esp+24h] [ebp-74h] BYREF
    unsigned int surfIndex; // [esp+28h] [ebp-70h]
    int baseIndex; // [esp+2Ch] [ebp-6Ch]
    int lmapIndex; // [esp+30h] [ebp-68h]
    const unsigned __int8 *vertLayerDataDisk; // [esp+34h] [ebp-64h]
    const Material *material; // [esp+38h] [ebp-60h]
    srfTriangles_t *tris; // [esp+3Ch] [ebp-5Ch]
    float normal[3]; // [esp+40h] [ebp-58h] BYREF
    float tangent[3]; // [esp+4Ch] [ebp-4Ch] BYREF
    unsigned int surfCount; // [esp+58h] [ebp-40h] BYREF
    float binormal[3]; // [esp+5Ch] [ebp-3Ch] BYREF
    const DiskTriangleSoup *diskSurfaces; // [esp+68h] [ebp-30h] BYREF
    unsigned int indexCount; // [esp+6Ch] [ebp-2Ch] BYREF
    const unsigned __int16 *indices; // [esp+70h] [ebp-28h]
    const DiskGfxVertex *vertsDisk; // [esp+74h] [ebp-24h]
    unsigned int vertCount; // [esp+78h] [ebp-20h] BYREF
    const Material *skyMaterial; // [esp+7Ch] [ebp-1Ch]
    GfxSurface *surface; // [esp+80h] [ebp-18h]
    GfxWorldVertex *vertsMem; // [esp+84h] [ebp-14h]
    unsigned int vertIndex; // [esp+88h] [ebp-10h]
    unsigned int vertLayerDataSize; // [esp+8Ch] [ebp-Ch] BYREF
    unsigned __int16 *worldIndices; // [esp+90h] [ebp-8h]
    unsigned __int16 surfIndexCount; // [esp+94h] [ebp-4h]

    if ( !load
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp", 4025, 0, "%s", "load") )
    {
        __debugbreak();
    }
    if ( load->bspVersion < 0x2D )
        Com_PrintWarning(8, "Bsp compiled with old version of cod2map.\n");
    R_LoadTriangleSurfaces(load->bspVersion, (DiskTriangleSoup **)&diskSurfaces, &surfCount);
    if ( !surfCount )
        Com_Error(ERR_DROP, "LoadMap: no surfaces in %s", s_world.name);
    vertsDisk = (const DiskGfxVertex *)Com_GetBspLump(LUMP_DRAWVERTS, 0x44u, &vertCount);
    if ( !vertCount )
        Com_Error(ERR_DROP, "LoadMap: no vertices in %s", s_world.name);
    s_world.draw.vertexCount = vertCount;
    vertLayerDataDisk = (const unsigned __int8 *)Com_GetBspLump(LUMP_VERTEX_LAYER_DATA, 1u, &vertLayerDataSize);
    if ( !vertLayerDataSize )
    {
        vertLayerDataSize = 4;
        vertLayerDataDisk = dummyData;
    }
    vertLayerDataMem = R_LoadSurfaceAlloc(vertLayerDataSize);
    Com_Memcpy(vertLayerDataMem, vertLayerDataDisk, vertLayerDataSize);
    s_world.draw.vertexLayerDataSize = vertLayerDataSize;
    s_world.draw.vld.data = vertLayerDataMem;
    vertsMem = (GfxWorldVertex *)R_LoadSurfaceAlloc(44 * vertCount);
    if ( !vertsMem
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp", 4054, 0, "%s", "vertsMem") )
    {
        __debugbreak();
    }
    s_world.draw.vd.vertices = vertsMem;
    for ( vertIndex = 0; vertIndex < vertCount; ++vertIndex )
    {
        vertsMem[vertIndex].xyz[0] = vertsDisk[vertIndex].xyz[0];
        vertsMem[vertIndex].xyz[1] = vertsDisk[vertIndex].xyz[1];
        vertsMem[vertIndex].xyz[2] = vertsDisk[vertIndex].xyz[2];
        Byte4CopyBgraToVertexColor(vertsDisk[vertIndex].color, (unsigned __int8 *)&vertsMem[vertIndex].color);
        vertsMem[vertIndex].texCoord[0] = vertsDisk[vertIndex].texCoord[0];
        vertsMem[vertIndex].texCoord[1] = vertsDisk[vertIndex].texCoord[1];
        vertsMem[vertIndex].lmapCoord[0] = vertsDisk[vertIndex].lmapCoord[0];
        vertsMem[vertIndex].lmapCoord[1] = vertsDisk[vertIndex].lmapCoord[1];
        tangent[0] = vertsDisk[vertIndex].tangent[0];
        tangent[1] = vertsDisk[vertIndex].tangent[1];
        tangent[2] = vertsDisk[vertIndex].tangent[2];
        v3.packed = Vec3PackUnitVec(tangent).packed;
        vertsMem[vertIndex].tangent = v3;
        normal[0] = vertsDisk[vertIndex].normal[0];
        normal[1] = vertsDisk[vertIndex].normal[1];
        normal[2] = vertsDisk[vertIndex].normal[2];
        v2.packed = Vec3PackUnitVec(normal).packed;
        vertsMem[vertIndex].normal = v2;
        Vec3Cross(vertsDisk[vertIndex].normal, vertsDisk[vertIndex].tangent, binormal);
        if ( (float)((float)((float)(binormal[0] * vertsDisk[vertIndex].binormal[0])
                                             + (float)(binormal[1] * vertsDisk[vertIndex].binormal[1]))
                             + (float)(binormal[2] * vertsDisk[vertIndex].binormal[2])) < 0.0 )
            v1 = -1.0f;
        else
            v1 = 1.0f;
        vertsMem[vertIndex].binormalSign = v1;
        if ( vertIndex % 50000 )
            Monkey_KeepAlive();
    }

    indices = (const unsigned __int16 *)Com_GetBspLump(LUMP_DRAWINDICES, 2u, &indexCount);
    if ( surfCount > 0x10000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    4100,
                    0,
                    "%s\n\t(surfCount) = %i",
                    "(surfCount <= 65536)",
                    surfCount) )
    {
        __debugbreak();
    }
    s_world.surfaceCount = surfCount;
    s_world.dpvs.surfaces = (GfxSurface *)Hunk_Alloc(80 * surfCount, "R_LoadSurfaces", 21);
    s_world.draw.indexCount = 0;
    for ( surfIndex = 0; surfIndex < surfCount; ++surfIndex )
    {
        s_world.draw.indexCount += diskSurfaces[surfIndex].indexCount;
        s_world.dpvs.surfaces[surfIndex].tris.baseIndex = -1;
    }
    worldIndices = (unsigned __int16 *)R_LoadSurfaceAlloc(2 * s_world.draw.indexCount);
    s_world.draw.indices = worldIndices;
    baseIndex = 0;
    for ( surfIndex = 0; surfIndex < surfCount; ++surfIndex )
    {
        if ( diskSurfaces[surfIndex].firstVertex + diskSurfaces[surfIndex].vertexCount > s_world.draw.vertexCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        4120,
                        0,
                        "%s",
                        "diskSurfaces[surfIndex].firstVertex + diskSurfaces[surfIndex].vertexCount <= s_world.draw.vertexCount") )
        {
            __debugbreak();
        }
        if ( diskSurfaces[surfIndex].reflectionProbeIndex == 255
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        4121,
                        0,
                        "%s",
                        "diskSurfaces[surfIndex].reflectionProbeIndex != REFLECTION_PROBE_INVALID") )
        {
            __debugbreak();
        }
    }
    for ( firstSurfIndex = 0; firstSurfIndex < surfCount; ++firstSurfIndex )
    {
        surface = &s_world.dpvs.surfaces[firstSurfIndex];
        if ( surface->tris.baseIndex < 0 )
        {
            for ( surfIndex = firstSurfIndex; surfIndex < surfCount; ++surfIndex )
            {
                surface = &s_world.dpvs.surfaces[surfIndex];
                if ( surface->tris.baseIndex < 0
                    && (diskSurfaces[surfIndex].materialIndex == diskSurfaces[firstSurfIndex].materialIndex
                     || !I_stricmp(
                                 load->diskMaterials[diskSurfaces[surfIndex].materialIndex].material,
                                 load->diskMaterials[firstSurfIndex].material))
                    && diskSurfaces[surfIndex].reflectionProbeIndex == diskSurfaces[firstSurfIndex].reflectionProbeIndex
                    && diskSurfaces[surfIndex].lightmapIndex == diskSurfaces[firstSurfIndex].lightmapIndex )
                {
                    tris = &surface->tris;
                    surfIndexCount = diskSurfaces[surfIndex].indexCount;
                    surface->tris.baseIndex = baseIndex;
                    surface->flags = 0;
                    if ( load->bspVersion <= 0x13 )
                        surface->flags |= 1u;
                    if ( diskSurfaces[surfIndex].castsSunShadow )
                        surface->flags |= 1u;
                    if ( diskSurfaces[surfIndex].hasPrimaryLightConflict )
                        surface->flags |= 2u;
                    if ( surfIndexCount % 3
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                                    4164,
                                    0,
                                    "%s\n\t(surfIndexCount) = %i",
                                    "(!(surfIndexCount % 3))",
                                    surfIndexCount) )
                    {
                        __debugbreak();
                    }
                    tris->triCount = surfIndexCount / 3;
                    if ( !tris->triCount
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                                    4166,
                                    0,
                                    "%s\n\t(tris->triCount) = %i",
                                    "(tris->triCount > 0)",
                                    tris->triCount) )
                    {
                        __debugbreak();
                    }
                    Com_Memcpy(&worldIndices[baseIndex], &indices[diskSurfaces[surfIndex].firstIndex], 2 * surfIndexCount);
                    baseIndex += surfIndexCount;
                }
            }
        }
    }
    skyMaterial = 0;
    s_world.skySurfCount = 0;
    for ( surfIndex = 0; surfIndex < surfCount; ++surfIndex )
    {
        surface = &s_world.dpvs.surfaces[surfIndex];
        tris = &surface->tris;
        surface->tris.vertexLayerData = diskSurfaces[surfIndex].vertexLayerData;
        tris->firstVertex = diskSurfaces[surfIndex].firstVertex;
        tris->vertexCount = diskSurfaces[surfIndex].vertexCount;
        lmapIndex = diskSurfaces[surfIndex].lightmapIndex;
        if ( lmapIndex >= 16
            && lmapIndex != 31
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        4186,
                        0,
                        "%s\n\t(lmapIndex) = %i",
                        "((lmapIndex >= 0 && lmapIndex < ((144 * 1024 * 1024) / ((1024 * 1024 * 1 * 1) + (512 * 512 * 16 * 2)))) || lmapIndex == 31)",
                        lmapIndex) )
        {
            __debugbreak();
        }
        if ( load->lmapMergeInfo[lmapIndex].index >= 0x10u
            && load->lmapMergeInfo[lmapIndex].index != 31
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        4187,
                        0,
                        "%s\n\t(load->lmapMergeInfo[lmapIndex].index) = %i",
                        "((load->lmapMergeInfo[lmapIndex].index >= 0 && load->lmapMergeInfo[lmapIndex].index < ((144 * 1024 * 1024) /"
                        " ((1024 * 1024 * 1 * 1) + (512 * 512 * 16 * 2)))) || load->lmapMergeInfo[lmapIndex].index == 31)",
                        load->lmapMergeInfo[lmapIndex].index) )
        {
            __debugbreak();
        }
        material = R_GetBspMaterial(diskSurfaces[surfIndex].materialIndex);
        R_FinalizeSurfVerts(material, surface, vertsDisk, &load->lmapMergeInfo[lmapIndex], vertsMem, vertCount);
        surface->material = (Material *)material;
        surface->lightmapIndex = load->lmapMergeInfo[lmapIndex].index;
        surface->reflectionProbeIndex = diskSurfaces[surfIndex].reflectionProbeIndex;
        if ( surface->reflectionProbeIndex == 255
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        4199,
                        0,
                        "%s",
                        "surface->reflectionProbeIndex != REFLECTION_PROBE_INVALID") )
        {
            __debugbreak();
        }
        surface->primaryLightIndex = diskSurfaces[surfIndex].primaryLightIndex;
        R_MaterialUsage((Material *)material, tris->firstVertex, tris->vertexCount, 6 * tris->triCount + 48);
        if ( (material->info.gameFlags & 8) != 0 )
        {
            surface->flags |= 4u;
            if ( skyMaterial && skyMaterial != material )
                Com_Error(
                    ERR_DROP,
                    "map has at least two different skies: %s and %s\nOnly one sky per map is supported\n",
                    material->info.name,
                    skyMaterial->info.name);
            skyMaterial = material;
            ++s_world.skySurfCount;
            if ( surface->primaryLightIndex
                && surface->primaryLightIndex != s_world.sunPrimaryLightIndex
                && (load->bspVersion > 0xC || surface->primaryLightIndex != 1)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            4224,
                            0,
                            "%s\n\t(surface->primaryLightIndex) = %i",
                            "(surface->primaryLightIndex == 0 || surface->primaryLightIndex == s_world.sunPrimaryLightIndex || (load->b"
                            "spVersion <= 12 && surface->primaryLightIndex == 1))",
                            surface->primaryLightIndex) )
            {
                __debugbreak();
            }
            surface->primaryLightIndex = s_world.sunPrimaryLightIndex;
        }
        if ( (material->info.gameFlags & 1) != 0 )
            surface->flags |= 8u;
        R_ValidateSurfaceLightmapUsage(surface);
    }
    if ( s_world.skySurfCount )
    {
        if ( !skyMaterial
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        4290,
                        0,
                        "%s",
                        "skyMaterial != NULL") )
        {
            __debugbreak();
        }
        R_SetSkyImage(skyMaterial);
        s_world.skyStartSurfs = (int *)Hunk_Alloc(4 * s_world.skySurfCount, "Sky surfaces", 21);
        s_world.skySurfCount = 0;
        for ( surfIndex = 0; surfIndex < surfCount; ++surfIndex )
        {
            surface = &s_world.dpvs.surfaces[surfIndex];
            if ( surface->material == skyMaterial )
                s_world.skyStartSurfs[s_world.skySurfCount++] = surfIndex;
        }
    }
    else
    {
        if ( skyMaterial
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        4284,
                        0,
                        "%s",
                        "skyMaterial == NULL") )
        {
            __debugbreak();
        }
        s_world.skyImage = 0;
        s_world.skyStartSurfs = 0;
    }
    R_CalculateWorldBounds(s_world.mins, s_world.maxs);
    R_CalculateOutdoorBounds(load, diskSurfaces);
    R_UnloadTriangleSurfaces(load->bspVersion, (DiskTriangleSoup *)diskSurfaces);
    R_CreateWorldVertexBuffer(&s_world.draw.vd.worldVb, (int *)s_world.draw.vd.vertices, 44 * s_world.draw.vertexCount);
    R_CreateWorldVertexBuffer(&s_world.draw.vld.layerVb, (int *)s_world.draw.vld.data, s_world.draw.vertexLayerDataSize);
    R_ShutdownMaterialUsage();
    R_CreateMaterialList();
}

void __cdecl R_FinalizeSurfVerts(
                const Material *material,
                GfxSurface *surface,
                const DiskGfxVertex *vertsDisk,
                const r_lightmapMerge_t *merge,
                GfxWorldVertex *vertsMem,
                unsigned int vertCount)
{
    unsigned int indexCount; // [esp+0h] [ebp-Ch]
    unsigned int vertIndex; // [esp+4h] [ebp-8h]
    unsigned int indexIndex; // [esp+8h] [ebp-4h]

    if ( !material
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp", 2748, 0, "%s", "material") )
    {
        __debugbreak();
    }
    if ( !surface
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp", 2749, 0, "%s", "surface") )
    {
        __debugbreak();
    }
    if ( !vertsDisk
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp", 2750, 0, "%s", "vertsDisk") )
    {
        __debugbreak();
    }
    if ( !merge
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp", 2751, 0, "%s", "merge") )
    {
        __debugbreak();
    }
    if ( !vertsMem
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp", 2752, 0, "%s", "vertsMem") )
    {
        __debugbreak();
    }
    ClearBounds(surface->bounds[0], surface->bounds[1]);
    indexCount = 3 * surface->tris.triCount;
    if ( surface->tris.baseIndex + indexCount - 1 >= s_world.draw.indexCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    2756,
                    0,
                    "surface->tris.baseIndex + indexCount - 1 doesn't index s_world.draw.indexCount\n\t%i not in [0, %i)",
                    surface->tris.baseIndex + indexCount - 1,
                    s_world.draw.indexCount) )
    {
        __debugbreak();
    }
    for ( indexIndex = 0; indexIndex < indexCount; ++indexIndex )
    {
        vertIndex = surface->tris.firstVertex + s_world.draw.indices[indexIndex + surface->tris.baseIndex];
        if ( vertIndex >= vertCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        2760,
                        0,
                        "vertIndex doesn't index vertCount\n\t%i not in [0, %i)",
                        vertIndex,
                        vertCount) )
        {
            __debugbreak();
        }
        AddPointToBounds(vertsMem[vertIndex].xyz, surface->bounds[0], surface->bounds[1]);
        vertsMem[vertIndex].lmapCoord[0] = (float)(vertsDisk[vertIndex].lmapCoord[0] * merge->scale[0]) + merge->shift[0];
        vertsMem[vertIndex].lmapCoord[1] = (float)(vertsDisk[vertIndex].lmapCoord[1] * merge->scale[1]) + merge->shift[1];
    }
    if ( (material->info.gameFlags & 0x20) != 0 )
        R_SurfCalculateMagicPortalVerts(material, surface, vertsDisk, merge, vertsMem);
    R_CalculateStreamingVolume(material, &surface->tris, vertsDisk);
}

void __cdecl R_CalculateStreamingVolume(const Material *material, srfTriangles_t *tris, const DiskGfxVertex *vertsDisk)
{
    float v3; // [esp+0h] [ebp-ACh]
    float *v4; // [esp+10h] [ebp-9Ch]
    const DiskGfxVertex *v5; // [esp+14h] [ebp-98h]
    int triCorner; // [esp+1Ch] [ebp-90h]
    float triPos[3][3]; // [esp+20h] [ebp-8Ch] BYREF
    float radius; // [esp+44h] [ebp-68h] BYREF
    float triMins[3]; // [esp+48h] [ebp-64h] BYREF
    float triTexCoord[3][2]; // [esp+54h] [ebp-58h] BYREF
    float triMaxs[3]; // [esp+6Ch] [ebp-40h] BYREF
    int triIndex; // [esp+78h] [ebp-34h]
    int triangleIndex; // [esp+7Ch] [ebp-30h]
    int texIter; // [esp+80h] [ebp-2Ch]
    unsigned int filterState; // [esp+84h] [ebp-28h] BYREF
    float surfBoundBoxMins[3]; // [esp+88h] [ebp-24h] BYREF
    float surfBoundBoxMaxs[3]; // [esp+94h] [ebp-18h] BYREF
    float maxRadius; // [esp+A0h] [ebp-Ch]
    float texSize[2]; // [esp+A4h] [ebp-8h] BYREF

    if ( tris->triCount >= 0x4000u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    2572,
                    0,
                    "%s",
                    "tris->triCount < 16384") )
    {
        __debugbreak();
    }
    tris->himipRadiusSq = 0.0f;
    ClearBounds(surfBoundBoxMins, surfBoundBoxMaxs);
    maxRadius = 0.0f;
    for ( texIter = 0; texIter != material->textureCount; ++texIter )
    {
        if ( R_StreamGetMaterialTextureSize(material, texIter, texSize, &filterState) )
        {
            triangleIndex = 0;
            for ( triIndex = 0; triIndex < tris->triCount; ++triIndex )
            {
                for ( triCorner = 0; triCorner != 3; ++triCorner )
                {
                    v4 = triPos[triCorner];
                    v5 = &vertsDisk[tris->firstVertex + s_world.draw.indices[3 * triIndex + triCorner + tris->baseIndex]];
                    *v4 = v5->xyz[0];
                    v4[1] = v5->xyz[1];
                    v4[2] = v5->xyz[2];
                    triTexCoord[triCorner][0] = v5->texCoord[0];
                    triTexCoord[triCorner][1] = v5->texCoord[1];
                }
                if ( R_CalculateTriangleTopMipAabb(triPos, triTexCoord, texSize, filterState, triMins, triMaxs, &radius) )
                {
                    if ( (float)(radius - maxRadius) < 0.0 )
                        v3 = maxRadius;
                    else
                        v3 = radius;
                    maxRadius = v3;
                    AddPointToBounds(triPos[0], surfBoundBoxMins, surfBoundBoxMaxs);
                    AddPointToBounds(triPos[1], surfBoundBoxMins, surfBoundBoxMaxs);
                    AddPointToBounds(triPos[2], surfBoundBoxMins, surfBoundBoxMaxs);
                }
                ++triangleIndex;
            }
        }
    }
    tris->himipRadiusSq = maxRadius * maxRadius;
    surfBoundBoxMins[0] = surfBoundBoxMins[0] + -1.0;
    surfBoundBoxMins[1] = surfBoundBoxMins[1] + -1.0;
    surfBoundBoxMins[2] = surfBoundBoxMins[2] + -1.0;
    surfBoundBoxMaxs[0] = surfBoundBoxMaxs[0] + 1.0;
    surfBoundBoxMaxs[1] = surfBoundBoxMaxs[1] + 1.0;
    surfBoundBoxMaxs[2] = surfBoundBoxMaxs[2] + 1.0;
    tris->mins[0] = surfBoundBoxMins[0];
    tris->mins[1] = surfBoundBoxMins[1];
    tris->mins[2] = surfBoundBoxMins[2];
    tris->maxs[0] = surfBoundBoxMaxs[0];
    tris->maxs[1] = surfBoundBoxMaxs[1];
    tris->maxs[2] = surfBoundBoxMaxs[2];
}

void __cdecl R_SurfCalculateMagicPortalVerts(
                const Material *material,
                GfxSurface *surface,
                const DiskGfxVertex *vertsDisk,
                const r_lightmapMerge_t *merge,
                GfxWorldVertex *vertsMem)
{
    float *v5; // [esp+0h] [ebp-506Ch]
    float v6; // [esp+4h] [ebp-5068h]
    float *v7; // [esp+Ch] [ebp-5060h]
    GfxWorldVertex *v8; // [esp+10h] [ebp-505Ch]
    float *v9; // [esp+18h] [ebp-5054h]
    int v10; // [esp+1Ch] [ebp-5050h]
    int kk; // [esp+20h] [ebp-504Ch]
    int v12; // [esp+24h] [ebp-5048h]
    int v13; // [esp+28h] [ebp-5044h]
    int jj; // [esp+30h] [ebp-503Ch]
    int ii; // [esp+34h] [ebp-5038h]
    unsigned int v16[3]; // [esp+38h] [ebp-5034h]
    int n; // [esp+44h] [ebp-5028h]
    int k; // [esp+48h] [ebp-5024h]
    unsigned int m; // [esp+4Ch] [ebp-5020h]
    unsigned int v20[3]; // [esp+50h] [ebp-501Ch]
    unsigned int j; // [esp+5Ch] [ebp-5010h]
    char v22; // [esp+63h] [ebp-5009h]
    float v23[4096]; // [esp+64h] [ebp-5008h] BYREF
    unsigned int triCount; // [esp+4064h] [ebp-1008h]
    unsigned int i; // [esp+4068h] [ebp-1004h]
    unsigned int v26[1024]; // [esp+406Ch] [ebp-1000h]

    triCount = surface->tris.triCount;
    if ( triCount > 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    2648,
                    0,
                    "%s\n\t%s",
                    "triCount <= ARRAY_COUNT( triFillId )",
                    material->info.name) )
    {
        __debugbreak();
    }
    if ( triCount > 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    2649,
                    0,
                    "%s\n\t%s",
                    "triCount <= ARRAY_COUNT( centerAccum )",
                    material->info.name) )
    {
        __debugbreak();
    }
    if ( triCount > 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    2650,
                    0,
                    "%s\n\t%s",
                    "triCount <= ARRAY_COUNT( centerWeight )",
                    material->info.name) )
    {
        __debugbreak();
    }
    for ( i = 0; i < triCount; ++i )
    {
        v26[i] = i;
        v9 = &v23[3 * i];
        *v9 = 0.0f;
        v9[1] = 0.0f;
        v9[2] = 0.0f;
        v23[i + 3072] = 0.0f;
    }
    v22 = 0;
    while ( !v22 )
    {
        v22 = 1;
        for ( j = 0; j < triCount; ++j )
        {
            for ( k = 0; k != 3; ++k )
                v20[k] = surface->tris.firstVertex + s_world.draw.indices[3 * j + k + surface->tris.baseIndex];
            for ( m = 0; m < triCount; ++m )
            {
                for ( k = 0; k != 3; ++k )
                    v16[k] = surface->tris.firstVertex + s_world.draw.indices[3 * m + k + surface->tris.baseIndex];
                for ( n = 0; n != 3; ++n )
                {
                    for ( ii = 0; ii != 3; ++ii )
                    {
                        if ( v20[n] == v16[ii] && v26[j] != v26[m] )
                        {
                            if ( v26[j] >= v26[m] )
                                v26[j] = v26[m];
                            else
                                v26[m] = v26[j];
                            v22 = 0;
                        }
                    }
                }
            }
        }
    }
    for ( i = 0; i < triCount; ++i )
    {
        for ( jj = 0; jj != 3; ++jj )
        {
            v13 = v26[i];
            v7 = &v23[3 * v13];
            v8 = &vertsMem[surface->tris.firstVertex + s_world.draw.indices[3 * i + jj + surface->tris.baseIndex]];
            *v7 = *v7 + v8->xyz[0];
            v7[1] = v7[1] + v8->xyz[1];
            v7[2] = v7[2] + v8->xyz[2];
            v23[v13 + 3072] = v23[v13 + 3072] + 1.0;
        }
    }
    for ( i = 0; i < triCount; ++i )
    {
        if ( v23[i + 3072] > 0.0 )
        {
            v5 = &v23[3 * i];
            v6 = 1.0 / v23[i + 3072];
            *v5 = v6 * *v5;
            v5[1] = v6 * v5[1];
            v5[2] = v6 * v5[2];
        }
    }
    for ( i = 0; i < triCount; ++i )
    {
        v12 = v26[i];
        for ( kk = 0; kk != 3; ++kk )
        {
            v10 = surface->tris.firstVertex + s_world.draw.indices[3 * i + kk + surface->tris.baseIndex];
            vertsMem[v10].texCoord[0] = v23[3 * v12];
            vertsMem[v10].texCoord[1] = v23[3 * v12 + 1];
            vertsMem[v10].lmapCoord[0] = v23[3 * v12 + 2];
            vertsMem[v10].lmapCoord[1] = 1.0f;
        }
    }
}

void __cdecl R_CalculateOutdoorBounds(GfxBspLoad *load, const DiskTriangleSoup *diskSurfaces)
{
    int surfIndex; // [esp+0h] [ebp-10h]
    signed int surfCount; // [esp+4h] [ebp-Ch]

    ClearBounds(load->outdoorMins, load->outdoorMaxs);
    if ( s_world.modelCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    2855,
                    0,
                    "%s",
                    "s_world.modelCount > 0") )
    {
        __debugbreak();
    }
    if ( s_world.models->startSurfIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    2856,
                    0,
                    "%s\n\t(s_world.models[0].startSurfIndex) = %i",
                    "(s_world.models[0].startSurfIndex == 0)",
                    s_world.models->startSurfIndex) )
    {
        __debugbreak();
    }
    surfCount = s_world.models->surfaceCount;
    for ( surfIndex = 0; surfIndex < surfCount; ++surfIndex )
    {
        if ( (s_world.dpvs.surfaces[surfIndex].material->info.gameFlags & 8) == 0
            && (load->diskMaterials[diskSurfaces[surfIndex].materialIndex].contentFlags & 0x2001) != 0 )
        {
            ExpandBounds(
                s_world.dpvs.surfaces[surfIndex].bounds[0],
                s_world.dpvs.surfaces[surfIndex].bounds[1],
                load->outdoorMins,
                load->outdoorMaxs);
        }
    }
}

void __cdecl R_CalculateWorldBounds(float *mins, float *maxs)
{
    int surfIndex; // [esp+0h] [ebp-8h]

    if ( s_world.surfaceCount )
    {
        ClearBounds(mins, maxs);
        for ( surfIndex = 0; surfIndex < s_world.surfaceCount; ++surfIndex )
            ExpandBounds(s_world.dpvs.surfaces[surfIndex].bounds[0], s_world.dpvs.surfaces[surfIndex].bounds[1], mins, maxs);
    }
    else
    {
        *mins = 0.0f;
        mins[1] = 0.0f;
        mins[2] = 0.0f;
        *maxs = 0.0f;
        maxs[1] = 0.0f;
        maxs[2] = 0.0f;
    }
}

void __cdecl R_MaterialUsage(Material *material, unsigned int firstVertex, int vertexCount, int surfPlusIndexSize)
{
    unsigned int *v4; // eax
    VertUsage *vertUsage; // [esp+0h] [ebp-8h]
    MaterialUsage *materialUsage; // [esp+4h] [ebp-4h]

    materialUsage = R_GetMaterialUsageData(material);
    if ( materialUsage )
    {
        materialUsage->memory += surfPlusIndexSize + 80;
        for ( vertUsage = materialUsage->verts; vertUsage; vertUsage = vertUsage->next )
        {
            if ( firstVertex == vertUsage->index )
                return;
        }
        v4 = Z_Malloc(8, "R_MaterialUsage", 0);
        *v4 = firstVertex;
        v4[1] = (unsigned int)materialUsage->verts;
        materialUsage->verts = (VertUsage *)v4;
        materialUsage->memory += 44 * vertexCount;
    }
}

void __cdecl R_SetSkyImage(const Material *skyMaterial)
{
    unsigned int colorMapHash; // [esp+0h] [ebp-Ch]
    int textureIndex; // [esp+4h] [ebp-8h]
    const MaterialTextureDef *texdef; // [esp+8h] [ebp-4h]

    colorMapHash = R_HashString("colorMap");
    for ( textureIndex = 0; textureIndex < skyMaterial->textureCount; ++textureIndex )
    {
        texdef = &skyMaterial->textureTable[textureIndex];
        if ( texdef->nameHash == colorMapHash )
        {
            if ( texdef->semantic == 11 || texdef->u.image->mapType != 5 )
                Com_Error(
                    ERR_DROP,
                    "colorMap '%s' for sky material '%s' is not a cubemap\n",
                    texdef->u.image->name,
                    skyMaterial->info.name);
            s_world.skyImage = texdef->u.image;
            s_world.skySamplerState = texdef->samplerState;
            return;
        }
    }
}

void __cdecl R_ValidateSurfaceLightmapUsage(const GfxSurface *surface)
{
    const MaterialTechnique *technique; // [esp+8h] [ebp-Ch]
    unsigned int techType; // [esp+Ch] [ebp-8h]
    unsigned int passIter; // [esp+10h] [ebp-4h]

    if ( surface->lightmapIndex == 31 )
    {
        for ( techType = 0; techType < 0x82; ++techType )
        {
            technique = Material_GetTechnique(surface->material, techType);
            if ( technique )
            {
                for ( passIter = 0; passIter < technique->passCount; ++passIter )
                {
                    if ( (technique->passArray[passIter].customSamplerFlags & 6) != 0 )
                        Com_Error(
                            ERR_DROP,
                            "World surface using material '%s' doesn't have a lightmap.    This usually means the map was compiled with a"
                            " different version of this material than you have locally.",
                            surface->material->info.name);
                }
            }
        }
    }
}

GfxBrushModel *R_LoadSubmodels()
{
    GfxBrushModel *result; // eax
    int firstTriSoup; // [esp+4h] [ebp-18h]
    GfxBrushModel *out; // [esp+8h] [ebp-14h]
    const DiskBrushModel *in; // [esp+Ch] [ebp-10h]
    int axis; // [esp+10h] [ebp-Ch]
    unsigned int modelIndex; // [esp+14h] [ebp-8h]
    unsigned int modelCount; // [esp+18h] [ebp-4h] BYREF

    in = (const DiskBrushModel *)Com_GetBspLump(LUMP_MODELS, 0x2Cu, &modelCount);
    out = (GfxBrushModel *)Hunk_Alloc(60 * modelCount, "R_LoadSubmodels", 21);
    s_world.models = out;
    result = (GfxBrushModel *)modelCount;
    s_world.modelCount = modelCount;
    for ( modelIndex = 0; modelIndex < modelCount; ++modelIndex )
    {
        for ( axis = 0; axis < 3; ++axis )
        {
            out->bounds[0][axis] = in->mins[axis];
            out->bounds[1][axis] = in->maxs[axis];
        }
        out->surfaceCount = in->triSoupCount;
        if ( out->surfaceCount )
            firstTriSoup = in->firstTriSoup;
        else
            firstTriSoup = -1;
        out->startSurfIndex = firstTriSoup;
        ++in;
        result = ++out;
    }
    return result;
}

void R_LoadSunSettings()
{
    unsigned int size; // [esp+0h] [ebp-8h] BYREF
    const char *text; // [esp+4h] [ebp-4h]

    s_world.sunParse.treeScatterAmount = 1.0f;
    s_world.sunParse.treeScatterIntensity = 1.0f;
    text = Com_GetBspLump(LUMP_ENTITIES, 1u, &size);
    R_ParseSunLight(&s_world.sunParse, (char *)text);
    s_world.sunLight = (GfxLight *)Hunk_Alloc(0x170u, "R_LoadSunSettings", 21);
    R_InterpretSunLightParseParamsIntoLights(&s_world.sunParse, s_world.sunLight);
}

void __cdecl R_LoadPrimaryLights(unsigned int bspVersion)
{
    const ComPrimaryLight *primaryLight; // [esp+0h] [ebp-8h]
    unsigned int lightIndex; // [esp+4h] [ebp-4h]

    if ( bspVersion > 0xE )
    {
        s_world.primaryLightCount = Com_GetPrimaryLightCount();
        s_world.sunPrimaryLightIndex = s_world.primaryLightCount > 1 && Com_GetPrimaryLight(1u)->type == 1;
        for ( lightIndex = 0; lightIndex < s_world.primaryLightCount; ++lightIndex )
        {
            primaryLight = Com_GetPrimaryLight(lightIndex);
            if ( primaryLight->defName )
                R_RegisterLightDef(primaryLight->defName);
        }
    }
    else
    {
        s_world.sunPrimaryLightIndex = 1;
        s_world.primaryLightCount = 2;
    }
}

void R_LoadLightRegions()
{
    GfxLightRegionAxis *v0; // [esp+8h] [ebp-34h]
    char *diskHulls; // [esp+Ch] [ebp-30h]
    unsigned int hullIter; // [esp+10h] [ebp-2Ch]
    unsigned __int8 *diskAxes; // [esp+14h] [ebp-28h]
    unsigned int usedAxisCount; // [esp+18h] [ebp-24h]
    unsigned __int8 *axes; // [esp+1Ch] [ebp-20h]
    unsigned int hullCount; // [esp+20h] [ebp-1Ch] BYREF
    unsigned int axisCount; // [esp+24h] [ebp-18h] BYREF
    unsigned int regionCount; // [esp+28h] [ebp-14h] BYREF
    unsigned int usedHullCount; // [esp+2Ch] [ebp-10h]
    GfxLightRegionHull *hulls; // [esp+30h] [ebp-Ch]
    const DiskLightRegion *diskRegions; // [esp+34h] [ebp-8h]
    unsigned int regionIter; // [esp+38h] [ebp-4h]

    s_world.lightRegion = (GfxLightRegion *)Hunk_Alloc(8 * s_world.primaryLightCount, "R_LoadLightRegions", 21);
    diskRegions = (const DiskLightRegion *)Com_GetBspLump(LUMP_LIGHTREGIONS, 1u, &regionCount);
    s_world.lightGrid.hasLightRegions = diskRegions != 0;
    if ( diskRegions )
    {
        diskHulls = Com_GetBspLump(LUMP_LIGHTREGION_HULLS, 0x4Cu, &hullCount);
        diskAxes = (unsigned __int8 *)Com_GetBspLump(LUMP_LIGHTREGION_AXES, 0x14u, &axisCount);
        hulls = (GfxLightRegionHull *)Hunk_Alloc(80 * hullCount, "R_LoadLightRegionHulls", 21);
        axes = Hunk_Alloc(20 * axisCount, "R_LoadLightRegionAxes", 21);
        if ( regionCount != s_world.primaryLightCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        5159,
                        0,
                        "regionCount == s_world.primaryLightCount\n\t%i, %i",
                        regionCount,
                        s_world.primaryLightCount) )
        {
            __debugbreak();
        }
        usedHullCount = 0;
        for ( regionIter = 0; regionIter < regionCount; ++regionIter )
        {
            s_world.lightRegion[regionIter].hullCount = diskRegions[regionIter].hullCount;
            if ( s_world.lightRegion[regionIter].hulls
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            5164,
                            1,
                            "%s",
                            "s_world.lightRegion[regionIter].hulls == NULL") )
            {
                __debugbreak();
            }
            if ( s_world.lightRegion[regionIter].hullCount )
            {
                s_world.lightRegion[regionIter].hulls = &hulls[usedHullCount];
                usedHullCount += diskRegions[regionIter].hullCount;
            }
        }
        if ( usedHullCount != hullCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        5171,
                        0,
                        "usedHullCount == hullCount\n\t%i, %i",
                        usedHullCount,
                        hullCount) )
        {
            __debugbreak();
        }
        usedAxisCount = 0;
        for ( hullIter = 0; hullIter < hullCount; ++hullIter )
        {
            memcpy(&hulls[hullIter], &diskHulls[76 * hullIter], 0x24u);
            memcpy(hulls[hullIter].kdopHalfSize, &diskHulls[76 * hullIter + 36], 0x28u);
            if ( hulls[hullIter].axisCount )
                v0 = (GfxLightRegionAxis *)&axes[20 * usedAxisCount];
            else
                v0 = 0;
            hulls[hullIter].axis = v0;
            usedAxisCount += *(unsigned int *)&diskHulls[76 * hullIter + 72];
        }
        if ( usedAxisCount != axisCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        5184,
                        0,
                        "usedAxisCount == axisCount\n\t%i, %i",
                        usedAxisCount,
                        axisCount) )
        {
            __debugbreak();
        }
        memcpy(axes, diskAxes, 20 * axisCount);
    }
}

int R_PostLoadEntities()
{
    int result; // eax
    int idxCell; // [esp+5BCh] [ebp-1Ch]
    int cellIndex; // [esp+5C0h] [ebp-18h]
    int cellIndexa; // [esp+5C0h] [ebp-18h]
    int cellIndexb; // [esp+5C0h] [ebp-18h]
    GfxStaticModelCombinedInst *smodelCombinedInsts; // [esp+5C8h] [ebp-10h]
    unsigned int dynModelCount; // [esp+5D0h] [ebp-8h]
    unsigned int smodelIndex; // [esp+5D4h] [ebp-4h]
    unsigned int smodelIndexa; // [esp+5D4h] [ebp-4h]
    unsigned int smodelIndexb; // [esp+5D4h] [ebp-4h]

    if ( !rgl.staticModelReflectionProbesLoaded
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    5408,
                    0,
                    "%s",
                    "rgl.staticModelReflectionProbesLoaded") )
    {
        __debugbreak();
    }
    smodelCombinedInsts = (GfxStaticModelCombinedInst *)Z_Malloc(120 * s_world.dpvs.smodelCount, "R_PostLoadEntities", 22);
    dynModelCount = 0;
    for ( smodelIndex = 0; smodelIndex < s_world.dpvs.smodelCount; ++smodelIndex )
    {
        memcpy(&smodelCombinedInsts[smodelIndex], &s_world.dpvs.smodelDrawInsts[smodelIndex], 0x4Cu);
        memcpy(
            &smodelCombinedInsts[smodelIndex].smodelInst,
            &s_world.dpvs.smodelInsts[smodelIndex],
            sizeof(smodelCombinedInsts[smodelIndex].smodelInst));
        if ( IsDynamicModel(smodelCombinedInsts[smodelIndex].smodelDrawInst.model) )
        {
            smodelCombinedInsts[smodelIndex].isDynamicModel = 1;
            ++dynModelCount;
        }
        else
        {
            smodelCombinedInsts[smodelIndex].isDynamicModel = 0;
        }
    }
    //std::_Sort<GfxStaticModelCombinedInst *,int,bool (__cdecl *)(GfxStaticModelCombinedInst const &,GfxStaticModelCombinedInst const &)>(
    //    smodelCombinedInsts,
    //    &smodelCombinedInsts[s_world.dpvs.smodelCount],
    //    (signed int)(120 * s_world.dpvs.smodelCount) / 120,
    //    (bool (__cdecl *)(const GfxStaticModelCombinedInst *, const GfxStaticModelCombinedInst *))R_StaticModelCompare);
    std::sort(smodelCombinedInsts, smodelCombinedInsts + s_world.dpvs.smodelCount, R_StaticModelCompare);
    for ( smodelIndexa = 0; smodelIndexa < s_world.dpvs.smodelCount; ++smodelIndexa )
    {
        memcpy(
            &s_world.dpvs.smodelDrawInsts[smodelIndexa],
            &smodelCombinedInsts[smodelIndexa],
            sizeof(s_world.dpvs.smodelDrawInsts[smodelIndexa]));
        memcpy(
            &s_world.dpvs.smodelInsts[smodelIndexa],
            &smodelCombinedInsts[smodelIndexa].smodelInst,
            sizeof(s_world.dpvs.smodelInsts[smodelIndexa]));
    }
    Z_Free((char *)smodelCombinedInsts, 22);
    if ( s_world.dpvs.dynamicSModelCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    5443,
                    0,
                    "%s",
                    "s_world.dpvs.dynamicSModelCount == 0") )
    {
        __debugbreak();
    }
    if ( DynamicModelsSortedCorrectly(s_world.dpvs.smodelDrawInsts, s_world.dpvs.smodelCount, dynModelCount) )
        s_world.dpvs.dynamicSModelCount = dynModelCount;
    if ( !s_world.dpvsPlanes.nodes
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    5449,
                    0,
                    "%s",
                    "s_world.dpvsPlanes.nodes") )
    {
        __debugbreak();
    }
    for ( smodelIndexb = 0; smodelIndexb < s_world.dpvs.smodelCount; ++smodelIndexb )
        R_FilterStaticModelIntoCells_r(
            &s_world,
            (mnode_t *)s_world.dpvsPlanes.nodes,
            &s_world.dpvs.smodelInsts[smodelIndexb],
            s_world.dpvs.smodelInsts[smodelIndexb].mins,
            s_world.dpvs.smodelInsts[smodelIndexb].maxs);
    for ( cellIndex = 0; cellIndex < s_world.dpvsPlanes.cellCount; ++cellIndex )
        R_AllocStaticModels(s_world.cells[cellIndex].aabbTree);
    for ( idxCell = 0; idxCell < s_world.dpvsPlanes.cellCount; ++idxCell )
        R_AabbTreeFixAABBSizes_r(s_world.cells[idxCell].aabbTree);
    for ( cellIndexa = 0; cellIndexa < s_world.dpvsPlanes.cellCount; ++cellIndexa )
        R_SortGfxAabbTree(&s_world, s_world.cells[cellIndexa].aabbTree);
    Hunk_ClearTempMemory();
    for ( cellIndexb = 0; ; ++cellIndexb )
    {
        result = cellIndexb;
        if ( cellIndexb >= s_world.dpvsPlanes.cellCount )
            break;
        R_FixupGfxAabbTrees(&s_world.cells[cellIndexb]);
    }
    return result;
}

bool __cdecl R_StaticModelCompare(
                const GfxStaticModelCombinedInst &smodelInst0,
                const GfxStaticModelCombinedInst &smodelInst1)
{
    const ComPrimaryLight *primaryLight; // [esp+0h] [ebp-Ch]
    int comparison; // [esp+8h] [ebp-4h]
    int comparisona; // [esp+8h] [ebp-4h]
    int comparisonb; // [esp+8h] [ebp-4h]
    int comparisonc; // [esp+8h] [ebp-4h]

    comparison = smodelInst0.isDynamicModel - smodelInst1.isDynamicModel;
    if ( comparison )
        return comparison > 0;
    primaryLight = Com_GetPrimaryLight(smodelInst0.smodelDrawInst.primaryLightIndex);
    comparisona = primaryLight->type - Com_GetPrimaryLight(smodelInst1.smodelDrawInst.primaryLightIndex)->type;
    if ( comparisona )
        return comparisona < 0;
    comparisonb = smodelInst0.smodelDrawInst.primaryLightIndex - smodelInst1.smodelDrawInst.primaryLightIndex;
    if ( comparisonb )
        return comparisonb < 0;
    comparisonc = smodelInst0.smodelDrawInst.model - smodelInst1.smodelDrawInst.model;
    if ( !comparisonc )
        comparisonc = smodelInst0.smodelDrawInst.reflectionProbeIndex - smodelInst1.smodelDrawInst.reflectionProbeIndex;
    return comparisonc < 0;
}

void __cdecl R_AabbTreeFixAABBSizes_r(GfxAabbTree *tree)
{
    unsigned int childIndex; // [esp+0h] [ebp-14h]
    GfxStaticModelInst *inst; // [esp+4h] [ebp-10h]
    unsigned __int16 idxSModel; // [esp+Ch] [ebp-8h]
    GfxAabbTree *children; // [esp+10h] [ebp-4h]

    for ( idxSModel = 0; idxSModel < (int)tree->smodelIndexCount; ++idxSModel )
    {
        inst = &s_world.dpvs.smodelInsts[tree->smodelIndexes[idxSModel]];
        Vec3Min(tree->mins, inst->mins, tree->mins);
        Vec3Max(tree->maxs, inst->maxs, tree->maxs);
    }
    children = (GfxAabbTree *)((char *)tree + tree->childrenOffset);
    for ( childIndex = 0; childIndex < tree->childCount; ++childIndex )
        R_AabbTreeFixAABBSizes_r(&children[childIndex]);
}

void __cdecl R_FixupGfxAabbTrees(GfxCell *cell)
{
    GfxAabbTree *tree; // [esp+0h] [ebp-Ch]
    GfxAabbTree *newTree; // [esp+4h] [ebp-8h]

    tree = cell->aabbTree;
    if ( !tree
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp", 5296, 0, "%s", "tree") )
    {
        __debugbreak();
    }
    cell->aabbTreeCount = R_AabbTreeChildrenCount_r(tree);
    newTree = (GfxAabbTree *)Hunk_AllocAlign(40 * cell->aabbTreeCount, 4, "R_FixupGfxAabbTrees", 22);
    if ( R_AabbTreeMove_r(tree, newTree, newTree + 1) - newTree != cell->aabbTreeCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    5301,
                    0,
                    "%s",
                    "allocChildren - newTree == cell->aabbTreeCount") )
    {
        __debugbreak();
    }
    cell->aabbTree = newTree;
}

int __cdecl R_AabbTreeChildrenCount_r(GfxAabbTree *tree)
{
    GfxAabbTree *children; // [esp+0h] [ebp-Ch]
    unsigned int childIndex; // [esp+4h] [ebp-8h]
    int count; // [esp+8h] [ebp-4h]

    count = 1;
    children = (GfxAabbTree *)((char *)tree + tree->childrenOffset);
    for ( childIndex = 0; childIndex < tree->childCount; ++childIndex )
        count += R_AabbTreeChildrenCount_r(&children[childIndex]);
    return count;
}

GfxAabbTree *__cdecl R_AabbTreeMove_r(GfxAabbTree *tree, GfxAabbTree *newTree, GfxAabbTree *newChildren)
{
    GfxAabbTree *children; // [esp+8h] [ebp-Ch]
    unsigned int childIndex; // [esp+Ch] [ebp-8h]
    GfxAabbTree *allocChildren; // [esp+10h] [ebp-4h]

    memcpy(newTree, tree, sizeof(GfxAabbTree));
    children = (GfxAabbTree *)((char *)tree + tree->childrenOffset);
    newTree->childrenOffset = (char *)newChildren - (char *)newTree;
    allocChildren = &newChildren[tree->childCount];
    for ( childIndex = 0; childIndex < tree->childCount; ++childIndex )
        allocChildren = R_AabbTreeMove_r(&children[childIndex], &newChildren[childIndex], allocChildren);
    return allocChildren;
}

bool __cdecl IsDynamicModel(const XModel *const xm)
{
    const MaterialTechnique *tech; // [esp+0h] [ebp-28h]
    int techIdx; // [esp+4h] [ebp-24h]
    int surfIdx; // [esp+Ch] [ebp-1Ch]
    Material **materialForSurf; // [esp+10h] [ebp-18h]
    int surfaceCount; // [esp+14h] [ebp-14h]
    XSurface *surfaces; // [esp+20h] [ebp-8h] BYREF
    bool hasDynModelTech; // [esp+27h] [ebp-1h]

    if ( !xm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp", 5317, 0, "%s", "xm") )
        __debugbreak();
    surfaceCount = XModelGetSurfaces(xm, &surfaces, 0);
    materialForSurf = XModelGetSkins(xm, 0);
    hasDynModelTech = 0;
    for ( surfIdx = 0; surfIdx < surfaceCount; ++surfIdx )
    {
        if ( (*materialForSurf)->localTechniqueSet )
        {
            for ( techIdx = 0; techIdx < 130; ++techIdx )
            {
                tech = (*materialForSurf)->localTechniqueSet->techniques[techIdx];
                if ( tech && (tech->flags & 0x220) != 0 )
                {
                    hasDynModelTech = 1;
                    break;
                }
            }
            if ( hasDynModelTech )
                break;
        }
        ++materialForSurf;
    }
    return hasDynModelTech;
}

char __cdecl DynamicModelsSortedCorrectly(
                GfxStaticModelDrawInst *drawInsts,
                unsigned int count,
                unsigned int dynModelCount)
{
    unsigned int i; // [esp+0h] [ebp-8h]
    unsigned int drawInstIndex; // [esp+4h] [ebp-4h]

    if ( dynModelCount > count
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    5370,
                    0,
                    "%s",
                    "dynModelCount <= count") )
    {
        __debugbreak();
    }
    for ( drawInstIndex = 0; drawInstIndex < dynModelCount; ++drawInstIndex )
    {
        if ( !IsDynamicModel(drawInsts[drawInstIndex].model) )
        {
LABEL_7:
            Com_Error(ERR_DROP, "VerifyDynamicModelSorting failed");
            return 0;
        }
    }
    for ( i = dynModelCount; i < count; ++i )
    {
        if ( IsDynamicModel(drawInsts[i].model) )
            goto LABEL_7;
    }
    return 1;
}

void __cdecl R_LoadEntities()
{
    unsigned int v0; // [esp+0h] [ebp-27Ch]
    unsigned int v1; // [esp+10h] [ebp-26Ch]
    unsigned int v2; // [esp+20h] [ebp-25Ch]
    unsigned int v3; // [esp+30h] [ebp-24Ch]
    int j; // [esp+40h] [ebp-23Ch]
    int k; // [esp+44h] [ebp-238h]
    int i; // [esp+48h] [ebp-234h]
    INFO *info; // [esp+4Ch] [ebp-230h]
    int spawnVarCount; // [esp+54h] [ebp-228h]
    int spawnVarCounta; // [esp+54h] [ebp-228h]
    char *startPos; // [esp+58h] [ebp-224h]
    const char *spawnVars[64][2]; // [esp+5Ch] [ebp-220h] BYREF
    unsigned int smodelCount; // [esp+25Ch] [ebp-20h]
    unsigned int textLen; // [esp+260h] [ebp-1Ch] BYREF
    char *textPool; // [esp+264h] [ebp-18h]
    int infoCount; // [esp+268h] [ebp-14h]
    const char *token; // [esp+26Ch] [ebp-10h]
    int charsUsed; // [esp+270h] [ebp-Ch]
    const char *text; // [esp+274h] [ebp-8h] BYREF
    int spawnVarIndex; // [esp+278h] [ebp-4h]

    spawnVarIndex = 0;
    infoCount = 0;
    startPos = Com_GetBspLump(LUMP_ENTITIES, 1u, &textLen);
    Hunk_CheckTempMemoryClear();
    textPool = (char *)Hunk_AllocateTempMemory(textLen + 0x100000, "R_LoadEntities");
    info = (INFO *)textPool;
    smodelCount = 0;
    text = startPos;
    while ( 1 )
    {
        token = (const char *)Com_Parse(&text);
        if ( !text || *token != 123 )
            break;
        spawnVars[0][0] = (char *)"";
        spawnVarCount = 1;
        charsUsed = 0x100000;
        while ( 1 )
        {
            token = (const char *)Com_Parse(&text);
            if ( !*token || *token == 125 )
                break;
            if ( I_stricmp(token, "classname") )
            {
                if ( spawnVarCount == 64 )
                    Com_Error(ERR_DROP, "R_LoadEntities: MAX_SPAWN_VARS (%i) reached", 64);
                spawnVarIndex = spawnVarCount++;
            }
            else
            {
                spawnVarIndex = 0;
            }
            spawnVars[spawnVarIndex][0] = &textPool[charsUsed];
            v3 = strlen(token);
            charsUsed += v3 + 1;
            memcpy((unsigned __int8 *)spawnVars[spawnVarIndex][0], (unsigned __int8 *)token, v3 + 1);
            token = (const char *)Com_Parse(&text);
            spawnVars[spawnVarIndex][1] = &textPool[charsUsed];
            v2 = strlen(token);
            charsUsed += v2 + 1;
            memcpy((unsigned __int8 *)spawnVars[spawnVarIndex][1], (unsigned __int8 *)token, v2 + 1);
        }
        if ( !*spawnVars[0][0] )
        {
            while ( spawnVarIndex >= 0 )
            {
                Com_Printf(0, "key: '%s'     value: '%s'\n", spawnVars[spawnVarIndex][0], spawnVars[spawnVarIndex][1]);
                --spawnVarIndex;
            }
            Com_Error(ERR_DROP, "R_LoadEntities: entity without a classname. See output above for list of key/value pairs that were found.");
        }
        if ( !I_stricmp(spawnVars[0][1], "info_null") )
        {
            I_strncpyz(info[infoCount].name, "---", 64);
            I_strncpyz(info[infoCount].origin, "0 0 0", 64);
            for ( i = 1; i < spawnVarCount; ++i )
            {
                if ( !I_stricmp(spawnVars[i][0], "targetname") )
                    I_strncpyz(info[infoCount].name, spawnVars[i][1], 64);
                if ( !I_stricmp(spawnVars[i][0], "origin") )
                    I_strncpyz(info[infoCount].origin, spawnVars[i][1], 64);
            }
            if ( ++infoCount >= 0x2000 )
                Com_Error(ERR_DROP, "R_LoadEntities: MAX_INFO_NULLS (%i) reached", 64);
        }
        if ( !I_stricmp(spawnVars[0][1], "misc_model") )
            ++smodelCount;
    }
    s_world.dpvs.smodelDrawInsts = (GfxStaticModelDrawInst *)Hunk_Alloc(76 * smodelCount, "R_LoadEntities", 22);
    s_world.dpvs.smodelInsts = (GfxStaticModelInst *)Hunk_Alloc(40 * smodelCount, "R_LoadEntities", 22);
    s_world.dpvs.smodelCount = 0;
    s_world.dpvs.dynamicSModelCount = 0;
    text = startPos;
    if ( !s_world.cells
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    5599,
                    0,
                    "%s",
                    "s_world.cells") )
    {
        __debugbreak();
    }
    while ( 1 )
    {
        token = (const char *)Com_Parse(&text);
        if ( *token != 123 )
            break;
        spawnVars[0][0] = (char *)"";
        spawnVarCounta = 1;
        charsUsed = 0x100000;
        while ( 1 )
        {
            token = (const char *)Com_Parse(&text);
            if ( !*token || *token == 125 )
                break;
            if ( I_stricmp(token, "classname") )
            {
                if ( spawnVarCounta == 64 )
                    Com_Error(ERR_DROP, "R_LoadEntities: MAX_SPAWN_VARS (%i) reached", 64);
                spawnVarIndex = spawnVarCounta++;
            }
            else
            {
                spawnVarIndex = 0;
            }
            spawnVars[spawnVarIndex][0] = &textPool[charsUsed];
            v1 = strlen(token);
            charsUsed += v1 + 1;
            memcpy((unsigned __int8 *)spawnVars[spawnVarIndex][0], (unsigned __int8 *)token, v1 + 1);
            token = (const char *)Com_Parse(&text);
            spawnVars[spawnVarIndex][1] = &textPool[charsUsed];
            v0 = strlen(token);
            charsUsed += v0 + 1;
            memcpy((unsigned __int8 *)spawnVars[spawnVarIndex][1], (unsigned __int8 *)token, v0 + 1);
        }
        if ( !*spawnVars[0][0] )
            Com_Error(ERR_DROP, "R_LoadEntities: entity without a classname");
        if ( !I_stricmp(spawnVars[0][1], "misc_model") )
        {
            for ( k = 1; k < spawnVarCounta; ++k )
            {
                if ( !I_stricmp(spawnVars[k][0], "target") )
                {
                    for ( j = 0; j < infoCount; ++j )
                    {
                        if ( !I_stricmp(spawnVars[k][1], info[j].name) )
                        {
                            spawnVars[spawnVarCounta][0] = "lightgrid_center";
                            spawnVars[spawnVarCounta++][1] = info[j].origin;
                        }
                    }
                }
            }
            R_LoadMiscModel((char*(*)[2])spawnVars, spawnVarCounta);
        }
    }
    if ( s_world.dpvs.smodelCount > 0x10000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    5667,
                    0,
                    "%s\n\t(s_world.dpvs.smodelCount) = %i",
                    "(s_world.dpvs.smodelCount <= 65536)",
                    s_world.dpvs.smodelCount) )
    {
        __debugbreak();
    }
    Hunk_FreeTempMemory(textPool);
}

void __cdecl R_LoadMiscModel(char *(*spawnVars)[2], int spawnVarCount)
{
    const char *v2; // eax
    float v3; // [esp+18h] [ebp-1E0h]
    float v4; // [esp+1Ch] [ebp-1DCh]
    float v5; // [esp+20h] [ebp-1D8h]
    float v6; // [esp+24h] [ebp-1D4h]
    float v7; // [esp+28h] [ebp-1D0h]
    float v8; // [esp+2Ch] [ebp-1CCh]
    float *v9; // [esp+BCh] [ebp-13Ch]
    float lightDistance; // [esp+C4h] [ebp-134h]
    unsigned int k; // [esp+C8h] [ebp-130h]
    float distanceOld; // [esp+CCh] [ebp-12Ch]
    int j; // [esp+D4h] [ebp-124h]
    float adjustedPosition[3]; // [esp+D8h] [ebp-120h] BYREF
    const ComPrimaryLight *oldlight; // [esp+E4h] [ebp-114h]
    ComPrimaryLight *v16; // [esp+E8h] [ebp-110h]
    float wx[8]; // [esp+ECh] [ebp-10Ch]
    float boxHalfSize[3]; // [esp+10Ch] [ebp-ECh] BYREF
    float wy[8]; // [esp+118h] [ebp-E0h]
    unsigned __int8 lightBoundingBox; // [esp+13Bh] [ebp-BDh]
    float wz[8]; // [esp+13Ch] [ebp-BCh] BYREF
    float boxMidPoint[3]; // [esp+15Ch] [ebp-9Ch] BYREF
    unsigned int i; // [esp+168h] [ebp-90h]
    const ComPrimaryLight *light; // [esp+16Ch] [ebp-8Ch]
    bool found; // [esp+173h] [ebp-85h]
    float primaryLight[3]; // [esp+174h] [ebp-84h] BYREF
    float originWeights[3]; // [esp+180h] [ebp-78h] BYREF
    GfxStaticModelDrawInst *smodelDrawInst; // [esp+18Ch] [ebp-6Ch]
    unsigned int modelFlags; // [esp+190h] [ebp-68h]
    float origin[3]; // [esp+194h] [ebp-64h] BYREF
    XModel *model; // [esp+1A0h] [ebp-58h] BYREF
    GfxStaticModelInst *smodelInst; // [esp+1A4h] [ebp-54h]
    unsigned __int8 primaryLightIndex; // [esp+1ABh] [ebp-4Dh]
    float angle; // [esp+1ACh] [ebp-4Ch]
    float angles[3]; // [esp+1B0h] [ebp-48h] BYREF
    int spawnflags; // [esp+1BCh] [ebp-3Ch]
    float scale; // [esp+1C0h] [ebp-38h]
    float lightingOrigin[3]; // [esp+1C4h] [ebp-34h] BYREF
    float axis[3][3]; // [esp+1D0h] [ebp-28h] BYREF
    unsigned __int8 staticModelFlags; // [esp+1F7h] [ebp-1h]

    if ( R_CheckValidStaticModel(spawnVars, spawnVarCount, &model, origin) )
    {
        smodelDrawInst = &s_world.dpvs.smodelDrawInsts[s_world.dpvs.smodelCount];
        smodelInst = &s_world.dpvs.smodelInsts[s_world.dpvs.smodelCount++];
        angle = R_FloatForKey("angle", 0.0, spawnVars, spawnVarCount);
        if ( angle == 0.0 )
        {
            R_VectorForKey("angles", (char*)"0 0 0", spawnVars, spawnVarCount, angles);
        }
        else
        {
            angles[0] = 0.0f;
            angles[1] = angle;
            angles[2] = 0.0f;
        }
        AnglesToAxis(angles, axis);
        scale = R_FloatForKey("modelscale", 1.0, spawnVars, spawnVarCount);
        if ( scale <= 0.0 )
        {
            v2 = va("Model %s has negative scale at %g %g %g\n", model->name, origin[0], origin[1], origin[2]);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            4559,
                            0,
                            "%s\n\t%s",
                            "scale > 0.0f",
                            v2) )
                __debugbreak();
        }
        spawnflags = R_IntForKey("spawnflags", 0, spawnVars, spawnVarCount);
        staticModelFlags = 0;
        modelFlags = XModelGetFlags(model);
        if ( (spawnflags & 2) != 0 || (modelFlags & 0x40000) != 0 )
            staticModelFlags |= 1u;
        staticModelFlags |= modelFlags & 0xF0;
        R_CreateStaticModel(model, origin, axis, scale, smodelDrawInst, smodelInst, staticModelFlags);
        smodelInst->groundLighting.packed = 0;
        if ( !R_VectorForKey("lightgrid_center", (char *)"0 0 0", spawnVars, spawnVarCount, lightingOrigin) )
        {
            R_VectorForKey("lightorigin", (char *)"0.5 0.5 0.75", spawnVars, spawnVarCount, originWeights);
            if ( originWeights[0] >= 0.0 )
                v8 = originWeights[0];
            else
                v8 = 0.0f;
            originWeights[0] = v8;
            if ( v8 <= 1.0 )
                v7 = originWeights[0];
            else
                v7 = 1.0f;
            originWeights[0] = v7;
            if ( originWeights[1] >= 0.0 )
                v6 = originWeights[1];
            else
                v6 = 0.0f;
            originWeights[1] = v6;
            if ( v6 <= 1.0 )
                v5 = originWeights[1];
            else
                v5 = 1.0f;
            originWeights[1] = v5;
            if ( originWeights[2] >= 0.0 )
                v4 = originWeights[2];
            else
                v4 = 0.0f;
            originWeights[2] = v4;
            if ( v4 <= 1.0 )
                v3 = originWeights[2];
            else
                v3 = 1.0f;
            originWeights[2] = v3;
            lightingOrigin[0] = (float)((float)(1.0 - originWeights[0]) * smodelInst->mins[0])
                                                + (float)(smodelInst->maxs[0] * originWeights[0]);
            lightingOrigin[1] = (float)((float)(1.0 - originWeights[1]) * smodelInst->mins[1])
                                                + (float)(smodelInst->maxs[1] * originWeights[1]);
            lightingOrigin[2] = (float)((float)(1.0 - v3) * smodelInst->mins[2]) + (float)(smodelInst->maxs[2] * v3);
        }
        v9 = smodelInst->lightingOrigin;
        smodelInst->lightingOrigin[0] = lightingOrigin[0];
        v9[1] = lightingOrigin[1];
        v9[2] = lightingOrigin[2];
        if ( (spawnflags & 4) != 0 )
        {
            primaryLightIndex = 0;
        }
        else if ( (spawnflags & 0x10) != 0 )
        {
            primaryLightIndex = 1;
        }
        else
        {
            primaryLightIndex = R_GetPrimaryLightFromGrid(&s_world.lightGrid, lightingOrigin, 1u);
            if ( R_VectorForKey("forcelight", (char *)"0.0 0.0 0.0", spawnVars, spawnVarCount, primaryLight) )
            {
                found = 0;
                for ( i = s_world.sunPrimaryLightIndex + 1; i < s_world.primaryLightCount; ++i )
                {
                    light = Com_GetPrimaryLight(i);
                    if ( light->origin[0] == primaryLight[0]
                        && light->origin[1] == primaryLight[1]
                        && light->origin[2] == primaryLight[2] )
                    {
                        primaryLightIndex = i;
                        found = 1;
                        break;
                    }
                }
            }
            if ( !primaryLightIndex )
            {
                wx[0] = 0.0f;
                wx[1] = 1.0f;
                wx[2] = 0.0f;
                wx[3] = 1.0f;
                wx[4] = 0.0f;
                wx[5] = 1.0f;
                wx[6] = 0.0f;
                wx[7] = 1.0f;
                wy[0] = 0.0f;
                wy[1] = 0.0f;
                wy[2] = 1.0f;
                wy[3] = 1.0f;
                wy[4] = 0.0f;
                wy[5] = 0.0f;
                wy[6] = 1.0f;
                wy[7] = 1.0f;
                memset(wz, 0, 16);
                wz[4] = 1.0f;
                wz[5] = 1.0f;
                wz[6] = 1.0f;
                wz[7] = 1.0f;
                for ( j = 0; j < 8; ++j )
                {
                    adjustedPosition[0] = (float)((float)(1.0 - wx[j]) * smodelInst->mins[0])
                                                            + (float)(smodelInst->maxs[0] * wx[j]);
                    adjustedPosition[1] = (float)((float)(1.0 - wy[j]) * smodelInst->mins[1])
                                                            + (float)(smodelInst->maxs[1] * wy[j]);
                    adjustedPosition[2] = (float)((float)(1.0 - wz[j]) * smodelInst->mins[2])
                                                            + (float)(smodelInst->maxs[2] * wz[j]);
                    lightBoundingBox = R_GetPrimaryLightFromGrid(&s_world.lightGrid, adjustedPosition, 1u);
                    if ( lightBoundingBox && lightBoundingBox != 1 )
                    {
                        if ( !primaryLightIndex || lightBoundingBox == primaryLightIndex )
                        {
                            primaryLightIndex = lightBoundingBox;
                        }
                        else
                        {
                            oldlight = Com_GetPrimaryLight(primaryLightIndex);
                            distanceOld = (float)((float)(oldlight->origin[2] - adjustedPosition[2])
                                                                    * (float)(oldlight->origin[2] - adjustedPosition[2]))
                                                    + (float)((float)((float)(oldlight->origin[1] - adjustedPosition[1])
                                                                                    * (float)(oldlight->origin[1] - adjustedPosition[1]))
                                                                    + (float)((float)(oldlight->origin[0] - adjustedPosition[0])
                                                                                    * (float)(oldlight->origin[0] - adjustedPosition[0])));
                            v16 = (ComPrimaryLight *)Com_GetPrimaryLight(lightBoundingBox);
                            if ( distanceOld > (float)((float)((float)(v16->origin[2] - adjustedPosition[2])
                                                                                             * (float)(v16->origin[2] - adjustedPosition[2]))
                                                                             + (float)((float)((float)(v16->origin[1] - adjustedPosition[1])
                                                                                                             * (float)(v16->origin[1] - adjustedPosition[1]))
                                                                                             + (float)((float)(v16->origin[0] - adjustedPosition[0])
                                                                                                             * (float)(v16->origin[0] - adjustedPosition[0])))) )
                                primaryLightIndex = lightBoundingBox;
                        }
                    }
                }
                if ( !primaryLightIndex )
                {
                    boxHalfSize[0] = (float)(smodelInst->maxs[0] - smodelInst->mins[0]) * 0.5;
                    boxHalfSize[1] = (float)(smodelInst->maxs[1] - smodelInst->mins[1]) * 0.5;
                    boxHalfSize[2] = (float)(smodelInst->maxs[2] - smodelInst->mins[2]) * 0.5;
                    boxMidPoint[0] = (float)(smodelInst->mins[0] * 0.5) + (float)(smodelInst->maxs[0] * 0.5);
                    boxMidPoint[1] = (float)(smodelInst->mins[1] * 0.5) + (float)(smodelInst->maxs[1] * 0.5);
                    boxMidPoint[2] = (float)(smodelInst->mins[2] * 0.5) + (float)(smodelInst->maxs[2] * 0.5);
                    for ( k = s_world.sunPrimaryLightIndex + 1; k < s_world.primaryLightCount; ++k )
                    {
                        v16 = (ComPrimaryLight *)Com_GetPrimaryLight(k);
                        if ( !Com_CullBoxFromPrimaryLight(v16, boxMidPoint, boxHalfSize)
                            && !R_CullBoxFromLightRegion(&s_world.lightRegion[k], v16->origin, boxMidPoint, boxHalfSize) )
                        {
                            lightDistance = (float)((float)(v16->origin[2] - lightingOrigin[2])
                                                                        * (float)(v16->origin[2] - lightingOrigin[2]))
                                                        + (float)((float)((float)(v16->origin[1] - lightingOrigin[1])
                                                                                        * (float)(v16->origin[1] - lightingOrigin[1]))
                                                                        + (float)((float)(v16->origin[0] - lightingOrigin[0])
                                                                                        * (float)(v16->origin[0] - lightingOrigin[0])));
                            if ( primaryLightIndex )
                            {
                                oldlight = Com_GetPrimaryLight(primaryLightIndex);
                                if ( (float)((float)((float)(oldlight->origin[2] - lightingOrigin[2])
                                                                     * (float)(oldlight->origin[2] - lightingOrigin[2]))
                                                     + (float)((float)((float)(oldlight->origin[1] - lightingOrigin[1])
                                                                                     * (float)(oldlight->origin[1] - lightingOrigin[1]))
                                                                     + (float)((float)(oldlight->origin[0] - lightingOrigin[0])
                                                                                     * (float)(oldlight->origin[0] - lightingOrigin[0])))) > lightDistance )
                                    primaryLightIndex = k;
                            }
                            else
                            {
                                primaryLightIndex = k;
                            }
                        }
                    }
                }
                if ( primaryLightIndex )
                    v16 = (ComPrimaryLight *)Com_GetPrimaryLight(primaryLightIndex);
            }
        }
        smodelDrawInst->primaryLightIndex = primaryLightIndex;
        ++s_world.shadowGeom[smodelDrawInst->primaryLightIndex].smodelCount;
    }
}

double __cdecl R_FloatForKey(const char *key, float defaultValue, char *(*spawnVars)[2], int spawnVarCount)
{
    char *string; // [esp+4h] [ebp-4h]

    string = R_ValueForKey(key, spawnVars, spawnVarCount);
    if ( !string )
        return defaultValue;
    return (float)atof(string);
}

char *__cdecl R_ValueForKey(const char *key, char *(*spawnVars)[2], int spawnVarCount)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 1; i < spawnVarCount; ++i )
    {
        if ( !I_stricmp((*spawnVars)[2 * i], key) )
            return (*spawnVars)[2 * i + 1];
    }
    return 0;
}

int __cdecl R_IntForKey(const char *key, int defaultValue, char *(*spawnVars)[2], int spawnVarCount)
{
    char *string; // [esp+0h] [ebp-4h]

    string = R_ValueForKey(key, spawnVars, spawnVarCount);
    if ( string )
        return atoi(string);
    else
        return defaultValue;
}

bool __cdecl R_VectorForKey(const char *key, char *defaultString, char *(*spawnVars)[2], int spawnVarCount, float *v)
{
    char *string; // [esp+0h] [ebp-8h]
    bool success; // [esp+7h] [ebp-1h]

    if ( !defaultString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    4388,
                    0,
                    "%s",
                    "defaultString") )
    {
        __debugbreak();
    }
    success = 1;
    string = R_ValueForKey(key, spawnVars, spawnVarCount);
    if ( !string )
    {
        success = 0;
        string = defaultString;
    }
    *v = 0.0f;
    v[1] = 0.0f;
    v[2] = 0.0f;
    sscanf(string, "%f %f %f", v, v + 1, v + 2);
    return success;
}

char __cdecl R_CheckValidStaticModel(char *(*spawnVars)[2], int spawnVarCount, XModel **model, float *origin)
{
    bool v6; // [esp+1Bh] [ebp-15h]
    char *modelName; // [esp+1Ch] [ebp-14h]
    float tempOrigin[3]; // [esp+20h] [ebp-10h] BYREF
    XModel *tempModel; // [esp+2Ch] [ebp-4h]

    if ( !R_VectorForKey("origin", (char *)"0 0 0", spawnVars, spawnVarCount, tempOrigin) )
        Com_Error(ERR_DROP, "R_CheckValidStaticModel: no origin specified");
    if ( R_ValueForKey("bake", spawnVars, spawnVarCount) )
        return 0;
    modelName = R_ValueForKey("model", spawnVars, spawnVarCount);
    if ( !modelName )
        Com_Error(ERR_DROP, "R_CheckValidStaticModel: no model specified in misc_model at (%.0f %.0f %.0f)", tempOrigin[0], tempOrigin[1], tempOrigin[2]);
    if ( Com_IsLegacyXModelName(modelName) )
        modelName += 7;
    tempModel = R_RegisterModel(modelName);
    if ( tempModel )
        v6 = XModelBad(tempModel);
    else
        v6 = 1;
    if ( v6 )
    {
        Com_PrintError(
            8,
            "bad static model '%s' at (%.0f %.0f %.0f), using default_static_model\n",
            modelName,
            tempOrigin[0],
            tempOrigin[1],
            tempOrigin[2]);
        tempModel = R_RegisterModel((char*)"default_static_model");
        if ( !tempModel || XModelBad(tempModel) )
            Com_Error(ERR_DROP, "R_CheckValidStaticModel: could not find xmodel 'default_static_model'");
    }
    if ( !model
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp", 4512, 0, "%s", "model") )
    {
        __debugbreak();
    }
    *model = tempModel;
    if ( !origin
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp", 4515, 0, "%s", "origin") )
    {
        __debugbreak();
    }
    *origin = tempOrigin[0];
    origin[1] = tempOrigin[1];
    origin[2] = tempOrigin[2];
    return 1;
}

unsigned int R_LoadAabbTrees()
{
    unsigned int result; // eax
    GfxAabbTree *out; // [esp+4h] [ebp-14h]
    unsigned int aabbTreeIndex; // [esp+8h] [ebp-10h]
    unsigned int aabbTreeIndexa; // [esp+8h] [ebp-10h]
    unsigned int aabbTreeCount; // [esp+Ch] [ebp-Ch] BYREF
    int surfaceCount; // [esp+10h] [ebp-8h]
    const DiskGfxAabbTree *in; // [esp+14h] [ebp-4h]

    in = (const DiskGfxAabbTree *)Com_GetBspLump(LUMP_AABBTREES, 0xCu, &aabbTreeCount);
    out = (GfxAabbTree *)Hunk_Alloc(40 * aabbTreeCount, "R_LoadAabbTrees", 23);
    rgl.aabbTrees = out;
    rgl.aabbTreeCount = aabbTreeCount;
    for ( aabbTreeIndex = 0; aabbTreeIndex < aabbTreeCount; ++aabbTreeIndex )
    {
        surfaceCount = in[aabbTreeIndex].surfaceCount;
        if ( surfaceCount )
        {
            out[aabbTreeIndex].startSurfIndex = in[aabbTreeIndex].firstSurface;
            if ( out[aabbTreeIndex].startSurfIndex != in[aabbTreeIndex].firstSurface
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            5727,
                            0,
                            "%s",
                            "out[aabbTreeIndex].startSurfIndex == in[aabbTreeIndex].firstSurface") )
            {
                __debugbreak();
            }
        }
        else
        {
            out[aabbTreeIndex].startSurfIndex = 0;
        }
        out[aabbTreeIndex].surfaceCount = surfaceCount;
        if ( out[aabbTreeIndex].surfaceCount != surfaceCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        5735,
                        0,
                        "%s",
                        "out[aabbTreeIndex].surfaceCount == surfaceCount") )
        {
            __debugbreak();
        }
        out[aabbTreeIndex].childCount = in[aabbTreeIndex].childCount;
        if ( out[aabbTreeIndex].childCount != in[aabbTreeIndex].childCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        5738,
                        0,
                        "%s",
                        "out[aabbTreeIndex].childCount == in[aabbTreeIndex].childCount") )
        {
            __debugbreak();
        }
    }
    for ( aabbTreeIndexa = 0;
                ;
                aabbTreeIndexa = R_FinishLoadingAabbTrees_r(&rgl.aabbTrees[aabbTreeIndexa], aabbTreeIndexa + 1) )
    {
        result = aabbTreeIndexa;
        if ( aabbTreeIndexa >= aabbTreeCount )
            break;
    }
    return result;
}

int __cdecl R_FinishLoadingAabbTrees_r(GfxAabbTree *tree, int totalTreesUsed)
{
    GfxAabbTree *children; // [esp+0h] [ebp-10h]
    int childIndex; // [esp+4h] [ebp-Ch]
    const GfxSurface *surf; // [esp+8h] [ebp-8h]
    int surfNodeIndex; // [esp+Ch] [ebp-4h]

    if ( !tree
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp", 5679, 0, "%s", "tree") )
    {
        __debugbreak();
    }
    if ( !rgl.aabbTrees
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    5680,
                    0,
                    "%s",
                    "rgl.aabbTrees") )
    {
        __debugbreak();
    }
    ClearBounds(tree->mins, tree->maxs);
    if ( tree->childCount )
    {
        tree->childrenOffset = (char *)&rgl.aabbTrees[totalTreesUsed] - (char *)tree;
        children = (GfxAabbTree *)((char *)tree + tree->childrenOffset);
        if ( children->startSurfIndex != tree->startSurfIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        5693,
                        0,
                        "%s",
                        "children[0].startSurfIndex == tree->startSurfIndex") )
        {
            __debugbreak();
        }
        totalTreesUsed += tree->childCount;
        for ( childIndex = 0; childIndex < tree->childCount; ++childIndex )
        {
            totalTreesUsed = R_FinishLoadingAabbTrees_r(&children[childIndex], totalTreesUsed);
            ExpandBounds(children[childIndex].mins, children[childIndex].maxs, tree->mins, tree->maxs);
        }
    }
    else
    {
        surfNodeIndex = 0;
        surf = &s_world.dpvs.surfaces[tree->startSurfIndex];
        while ( surfNodeIndex < tree->surfaceCount )
        {
            ExpandBounds(surf->bounds[0], surf->bounds[1], tree->mins, tree->maxs);
            ++surfNodeIndex;
            ++surf;
        }
    }
    return totalTreesUsed;
}

void __cdecl R_LoadCells(unsigned int bspVersion)
{
    int *v1; // [esp+0h] [ebp-18h]
    GfxCell *out; // [esp+4h] [ebp-14h]
    unsigned int cellIndex; // [esp+8h] [ebp-10h]
    char *in; // [esp+Ch] [ebp-Ch]
    int cullGroupCount; // [esp+10h] [ebp-8h]
    unsigned int cellCount; // [esp+14h] [ebp-4h] BYREF

    if ( bspVersion > 0xE )
    {
        if ( bspVersion > 0x15 )
            in = Com_GetBspLump(LUMP_CELLS, 0x70u, &cellCount);
        else
            in = Com_GetBspLump(LUMP_CELLS, 0x2Cu, &cellCount);
    }
    else
    {
        in = Com_GetBspLump(LUMP_CELLS, 0x34u, &cellCount);
    }
    out = (GfxCell *)Hunk_Alloc(56 * cellCount, "R_LoadCells", 23);
    s_world.cells = out;
    s_world.dpvsPlanes.cellCount = cellCount;
    s_world.cellBitsCount = 16 * ((cellCount + 127) >> 7);
    for ( cellIndex = 0; cellIndex < cellCount; ++cellIndex )
    {
        out->mins[0] = *(float *)in;
        out->mins[1] = *((float *)in + 1);
        out->mins[2] = *((float *)in + 2);
        out->maxs[0] = *((float *)in + 3);
        out->maxs[1] = *((float *)in + 4);
        out->maxs[2] = *((float *)in + 5);
        out->aabbTree = &rgl.aabbTrees[*((unsigned __int16 *)in + 12)];
        out->portals = (GfxPortal *)(68 * *((unsigned int *)in + 7));
        out->portalCount = *((unsigned int *)in + 8);
        cullGroupCount = *((unsigned int *)in + 10);
        if ( cullGroupCount )
            v1 = &rgl.cullGroupIndices[*((unsigned int *)in + 9)];
        else
            v1 = 0;
        out->cullGroups = v1;
        out->cullGroupCount = cullGroupCount;
        out->reflectionProbeCount = 0;
        out->reflectionProbes = 0;
        if ( bspVersion > 0xE )
        {
            if ( bspVersion > 0x15 )
            {
                out->reflectionProbeCount = in[44];
                if ( out->reflectionProbeCount )
                {
                    out->reflectionProbes = Hunk_Alloc((unsigned __int8)in[44], "R_LoadCells", 23);
                    memcpy(out->reflectionProbes, (unsigned __int8 *)in + 45, (unsigned __int8)in[44]);
                }
                else
                {
                    out->reflectionProbes = Hunk_Alloc(1u, "R_LoadCells", 23);
                    out->reflectionProbeCount = 1;
                    *out->reflectionProbes = 0;
                }
                in += 112;
            }
            else
            {
                in += 44;
            }
        }
        else
        {
            in += 52;
        }
        ++out;
    }
}

void R_LoadPortalVerts()
{
    char *in; // [esp+4h] [ebp-8h]
    unsigned int vertCount; // [esp+8h] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_PORTALVERTS, 0xCu, &vertCount);
    rgl.portalVerts = (float (*)[3])Hunk_Alloc(12 * vertCount, "R_LoadPortalVerts", 23);
    Com_Memcpy(rgl.portalVerts, in, 12 * vertCount);
}

int R_LoadPortals()
{
    int result; // eax
    GfxPortal *v1; // [esp+4h] [ebp-24h]
    DpvsPlane *p_plane; // [esp+Ch] [ebp-1Ch]
    GfxPortal *out; // [esp+10h] [ebp-18h]
    cplane_s *plane; // [esp+14h] [ebp-14h]
    int cellIndex; // [esp+18h] [ebp-10h]
    const DiskGfxPortal *in; // [esp+1Ch] [ebp-Ch]
    unsigned int portalIndex; // [esp+20h] [ebp-8h]
    unsigned int portalCount; // [esp+24h] [ebp-4h] BYREF

    in = (const DiskGfxPortal *)Com_GetBspLump(LUMP_PORTALS, 0x10u, &portalCount);
    out = (GfxPortal *)Hunk_Alloc(68 * portalCount, "R_LoadPortals", 23);
    if ( !s_world.cells
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    5871,
                    0,
                    "%s",
                    "s_world.cells") )
    {
        __debugbreak();
    }
    if ( !s_world.dpvsPlanes.planes
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    5872,
                    0,
                    "%s",
                    "s_world.dpvsPlanes.planes") )
    {
        __debugbreak();
    }
    for ( portalIndex = 0; portalIndex < portalCount; ++portalIndex )
    {
        plane = &s_world.dpvsPlanes.planes[in[portalIndex].planeIndex];
        p_plane = &out[portalIndex].plane;
        p_plane->coeffs[0] = plane->normal[0];
        p_plane->coeffs[1] = plane->normal[1];
        p_plane->coeffs[2] = plane->normal[2];
        out[portalIndex].plane.coeffs[3] = -plane->dist;
        R_SetDpvsPlaneSides(p_plane);
        if ( in[portalIndex].cellIndex >= (unsigned int)s_world.dpvsPlanes.cellCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        5880,
                        0,
                        "in[portalIndex].cellIndex doesn't index s_world.dpvsPlanes.cellCount\n\t%i not in [0, %i)",
                        in[portalIndex].cellIndex,
                        s_world.dpvsPlanes.cellCount) )
        {
            __debugbreak();
        }
        out[portalIndex].cell = &s_world.cells[in[portalIndex].cellIndex];
        if ( !rgl.portalVerts
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        5882,
                        0,
                        "%s",
                        "rgl.portalVerts") )
        {
            __debugbreak();
        }
        out[portalIndex].vertices = (float (*)[3])rgl.portalVerts[in[portalIndex].firstPortalVertex];
        out[portalIndex].vertexCount = in[portalIndex].portalVertexCount;
        PerpendicularVector(plane->normal, out[portalIndex].hullAxis[0]);
        Vec3Cross(plane->normal, out[portalIndex].hullAxis[0], out[portalIndex].hullAxis[1]);
    }
    for ( cellIndex = 0; ; ++cellIndex )
    {
        result = cellIndex;
        if ( cellIndex >= s_world.dpvsPlanes.cellCount )
            break;
        if ( s_world.cells[cellIndex].portalCount )
            v1 = &out[(int)s_world.cells[cellIndex].portals / 68];
        else
            v1 = 0;
        s_world.cells[cellIndex].portals = v1;
    }
    return result;
}

unsigned int R_LoadCullGroups()
{
    unsigned int result; // eax
    unsigned int firstSurface; // [esp+4h] [ebp-1Ch]
    GfxCullGroup *out; // [esp+8h] [ebp-18h]
    unsigned int surfaceCount; // [esp+Ch] [ebp-14h]
    unsigned int cullGroupCount; // [esp+10h] [ebp-10h] BYREF
    const DiskGfxCullGroup *in; // [esp+14h] [ebp-Ch]
    unsigned int cullGroupIndex; // [esp+18h] [ebp-8h]
    unsigned int axis; // [esp+1Ch] [ebp-4h]

    in = (const DiskGfxCullGroup *)Com_GetBspLump(LUMP_CULLGROUPS, 0x20u, &cullGroupCount);
    out = (GfxCullGroup *)Hunk_Alloc(32 * cullGroupCount, "R_LoadCullGroups", 23);
    s_world.dpvs.cullGroups = out;
    result = cullGroupCount;
    s_world.cullGroupCount = cullGroupCount;
    for ( cullGroupIndex = 0; cullGroupIndex < cullGroupCount; ++cullGroupIndex )
    {
        for ( axis = 0; axis < 3; ++axis )
        {
            out[cullGroupIndex].mins[axis] = in[cullGroupIndex].mins[axis];
            out[cullGroupIndex].maxs[axis] = in[cullGroupIndex].maxs[axis];
        }
        surfaceCount = in[cullGroupIndex].surfaceCount;
        if ( surfaceCount )
            firstSurface = in[cullGroupIndex].firstSurface;
        else
            firstSurface = -1;
        out[cullGroupIndex].startSurfIndex = firstSurface;
        result = surfaceCount;
        out[cullGroupIndex].surfaceCount = surfaceCount;
    }
    return result;
}

void R_LoadCullGroupIndices()
{
    int *out; // [esp+0h] [ebp-Ch]
    unsigned int indexCount; // [esp+4h] [ebp-8h] BYREF
    const int *in; // [esp+8h] [ebp-4h]

    in = (const int *)Com_GetBspLump(LUMP_CULLGROUPINDICES, 4u, &indexCount);
    out = (int *)Hunk_Alloc(4 * indexCount, "R_LoadCullGroupIndices", 23);
    rgl.cullGroupIndices = out;
    if ( indexCount
        && !s_world.dpvs.cullGroups
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    5935,
                    0,
                    "%s",
                    "!indexCount || s_world.dpvs.cullGroups") )
    {
        __debugbreak();
    }
    Com_Memcpy(out, in, 4 * indexCount);
}

void __cdecl R_LoadNodesAndLeafs(unsigned int bspVersion)
{
    char *inNode; // [esp+0h] [ebp-30h]
    mnode_load_t *out; // [esp+4h] [ebp-2Ch]
    unsigned int nodeIndex; // [esp+8h] [ebp-28h]
    unsigned int leafIndex; // [esp+Ch] [ebp-24h]
    unsigned int leafIndexa; // [esp+Ch] [ebp-24h]
    int nodeOrLeafIndex; // [esp+10h] [ebp-20h]
    int childIndex; // [esp+18h] [ebp-18h]
    char *inLeaf_v14; // [esp+1Ch] [ebp-14h]
    unsigned int leafCount; // [esp+20h] [ebp-10h] BYREF
    unsigned int nodeCount; // [esp+24h] [ebp-Ch] BYREF
    const DiskLeaf *inLeaf; // [esp+28h] [ebp-8h]
    int totalNodeCount; // [esp+2Ch] [ebp-4h]

    inNode = Com_GetBspLump(LUMP_NODES, 0x24u, &nodeCount);
    if ( bspVersion > 0xE )
    {
        inLeaf = (const DiskLeaf *)Com_GetBspLump(LUMP_LEAFS, 0x18u, &leafCount);
        inLeaf_v14 = 0;
    }
    else
    {
        inLeaf_v14 = Com_GetBspLump(LUMP_LEAFS, 0x24u, &leafCount);
        inLeaf = 0;
    }
    totalNodeCount = leafCount + nodeCount;
    out = (mnode_load_t *)Z_Malloc(16 * (leafCount + nodeCount), "R_LoadNodesAndLeafs", 23);
    rgl.nodes = out;
    rgl.nodeCount = nodeCount;
    for ( nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex )
    {
        out->planeIndex = *(unsigned int *)inNode;
        for ( childIndex = 0; childIndex < 2; ++childIndex )
        {
            nodeOrLeafIndex = *(unsigned int *)&inNode[4 * childIndex + 4];
            if ( nodeOrLeafIndex < 0 )
                out->children[childIndex] = nodeCount + -1 - nodeOrLeafIndex;
            else
                out->children[childIndex] = nodeOrLeafIndex;
        }
        inNode += 36;
        ++out;
    }
    if ( bspVersion > 0xE )
    {
        for ( leafIndexa = 0; leafIndexa < leafCount; ++leafIndexa )
        {
            out->cellIndex = SLOWORD(inLeaf->cellNum);
            if ( out->cellIndex != inLeaf->cellNum
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            6064,
                            0,
                            "%s",
                            "out->cellIndex == inLeaf->cellNum") )
            {
                __debugbreak();
            }
            ++inLeaf;
            ++out;
        }
    }
    else
    {
        for ( leafIndex = 0; leafIndex < leafCount; ++leafIndex )
        {
            out->cellIndex = *((__int16 *)inLeaf_v14 + 12);
            if ( out->cellIndex != *((unsigned int *)inLeaf_v14 + 6)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            6054,
                            0,
                            "%s",
                            "out->cellIndex == inLeaf_v14->cellNum") )
            {
                __debugbreak();
            }
            inLeaf_v14 += 36;
            ++out;
        }
    }
    R_SetParentAndCell_r(rgl.nodes);
    s_world.nodeCount = R_CountNodes_r(rgl.nodes);
    s_world.dpvsPlanes.nodes = (unsigned __int16 *)Hunk_Alloc(16 * s_world.nodeCount, "R_LoadNodesAndLeafs", 23);
    if ( ((char *)R_SortNodes_r(rgl.nodes, (mnode_t *)s_world.dpvsPlanes.nodes) - (char *)s_world.dpvsPlanes.nodes) >> 1 != s_world.nodeCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    6076,
                    0,
                    "%s",
                    "reinterpret_cast< ushort * >( out2 ) - s_world.dpvsPlanes.nodes == s_world.nodeCount") )
    {
        __debugbreak();
    }
    Z_Free((char *)rgl.nodes, 23);
}

void __cdecl R_SetParentAndCell_r(mnode_load_t *node)
{
    int cellIndex; // [esp+0h] [ebp-8h]

    if ( node - rgl.nodes < rgl.nodeCount )
    {
        R_SetParentAndCell_r(&rgl.nodes[node->children[0]]);
        R_SetParentAndCell_r(&rgl.nodes[node->children[1]]);
        node->cellIndex = -2;
        cellIndex = rgl.nodes[node->children[0]].cellIndex;
        if ( cellIndex == rgl.nodes[node->children[1]].cellIndex )
            node->cellIndex = cellIndex;
    }
}

unsigned int __cdecl R_CountNodes_r(mnode_load_t *node)
{
    unsigned int v2; // esi

    if ( node->cellIndex != -2 )
        return 1;
    v2 = R_CountNodes_r(&rgl.nodes[node->children[0]]);
    return v2 + R_CountNodes_r(&rgl.nodes[node->children[1]]) + 2;
}

mnode_t *__cdecl R_SortNodes_r(mnode_load_t *node, mnode_t *out)
{
    mnode_t *outa; // [esp+14h] [ebp+Ch]
    mnode_t *outb; // [esp+14h] [ebp+Ch]

    if ( node->cellIndex == -2 )
    {
        outa = out + 1;
        out->cellIndex = LOWORD(s_world.dpvsPlanes.cellCount) + node->planeIndex + 1;
        if ( out->cellIndex != s_world.dpvsPlanes.cellCount + node->planeIndex + 1 )
            Com_Error(ERR_DROP, "Max planes exceeded");
        outb = R_SortNodes_r(&rgl.nodes[node->children[0]], outa);
        out->rightChildOffset = ((char *)outb - (char *)out) >> 1;

        if ( out->rightChildOffset != ((char *)outb - (char *)out) >> 1 )
            Com_Error(ERR_DROP, "Max cells exceeded");

        return R_SortNodes_r(&rgl.nodes[node->children[1]], outb);
    }
    else
    {
        out->cellIndex = node->cellIndex + 1;
        if ( out->cellIndex != node->cellIndex + 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        5974,
                        0,
                        "%s",
                        "out->cellIndex == node->cellIndex + 1") )
        {
            __debugbreak();
        }
        return (mnode_t *)&out->rightChildOffset;
    }
}

void __cdecl R_LoadStep(const char *description)
{
    Com_Printf(8, "Loading %s...\n", description);
    SCR_UpdateLoadScreen();
}

void __cdecl R_GenerateHighmipAabbs()
{
    GenericAabbTreeOptions options; // [esp+28h] [ebp-40h] BYREF
    int maxElemCount; // [esp+50h] [ebp-18h]
    int nodeCount; // [esp+54h] [ebp-14h]
    int *elemArray; // [esp+58h] [ebp-10h]
    int MIN_HIGHMIP_PER_LEAF; // [esp+5Ch] [ebp-Ch]
    int elemCount; // [esp+60h] [ebp-8h] BYREF
    int MAX_HIGHMIP_AABBS; // [esp+64h] [ebp-4h]

    MAX_HIGHMIP_AABBS = 0x2000;
    MIN_HIGHMIP_PER_LEAF = 8;
    maxElemCount = s_world.dpvs.smodelCount + s_world.surfaceCount;
    memset(&options, 0, 12);
    options.maintainValidBounds = 1;
    options.mins = (float (*)[3])operator new[](
                                                                 4
                                                             * ((3 * (unsigned __int64)(s_world.dpvs.smodelCount + s_world.surfaceCount)) >> 32 != 0
                                                                ? -1
                                                                : 3 * (s_world.dpvs.smodelCount + s_world.surfaceCount)));
    options.maxs = (float (*)[3])operator new[](
                                                                 4
                                                             * ((3 * (unsigned __int64)(unsigned int)maxElemCount) >> 32 != 0 ? -1 : 3 * maxElemCount));
    options.treeNodePool = (GenericAabbTree *)operator new[](0x20000u);
    options.treeNodeLimit = 0x2000;
    options.minItemsPerLeaf = 8;
    options.maxItemsPerLeaf = 32;
    elemCount = 0;
    elemArray = (int *)operator new[](4 * maxElemCount);
    R_GenerateHighmipAabbs_AddWorldSurfaces(&elemCount, elemArray, &options);
    R_GenerateHighmipAabbs_AddStaticModelSurfaces(&elemCount, elemArray, &options);
    options.items = elemArray;
    options.itemCount = elemCount;
    options.itemSize = 4;
    if ( elemCount <= 0 )
        nodeCount = 0;
    else
        nodeCount = BuildAabbTree(&options);
    s_world.streamInfo.aabbTreeCount = nodeCount;
    s_world.streamInfo.aabbTrees = (GfxStreamingAabbTree *)Hunk_Alloc(32 * nodeCount, "R_GenerateHighmipAabbs", 23);
    s_world.streamInfo.leafRefCount = elemCount;
    s_world.streamInfo.leafRefs = (int *)Hunk_Alloc(4 * elemCount, "R_GenerateHighmipAabbs", 23);
    memcpy((unsigned __int8 *)s_world.streamInfo.leafRefs, (unsigned __int8 *)elemArray, 4 * elemCount);
    R_SetupStreamAabbNodes(&options, s_world.streamInfo.aabbTrees, nodeCount);
    operator delete[](options.mins);
    operator delete[](options.maxs);
    operator delete[](elemArray);
    operator delete[](options.treeNodePool);
}

void __cdecl R_SetupStreamAabbNodes(GenericAabbTreeOptions *options, GfxStreamingAabbTree *destNodes, int count)
{
    int itemIter; // [esp+0h] [ebp-14h]
    const GenericAabbTree *srcNode; // [esp+4h] [ebp-10h]
    int itemEnd; // [esp+8h] [ebp-Ch]
    GfxStreamingAabbTree *destNode; // [esp+Ch] [ebp-8h]
    int nodeIter; // [esp+10h] [ebp-4h]

    for ( nodeIter = 0; nodeIter != count; ++nodeIter )
    {
        srcNode = &options->treeNodePool[nodeIter];
        destNode = &destNodes[nodeIter];
        destNode->firstItem = srcNode->firstItem;
        if ( srcNode->firstItem != destNode->firstItem
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        6099,
                        0,
                        "%s\n\t(srcNode.firstItem) = %i",
                        "(srcNode.firstItem == destNode.firstItem)",
                        srcNode->firstItem) )
        {
            __debugbreak();
        }
        destNode->itemCount = srcNode->itemCount;
        if ( srcNode->itemCount != destNode->itemCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        6102,
                        0,
                        "%s\n\t(srcNode.itemCount) = %i",
                        "(srcNode.itemCount == destNode.itemCount)",
                        srcNode->itemCount) )
        {
            __debugbreak();
        }
        destNode->firstChild = srcNode->firstChild;
        if ( srcNode->firstChild != destNode->firstChild
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        6105,
                        0,
                        "%s\n\t(srcNode.firstChild) = %i",
                        "(srcNode.firstChild == destNode.firstChild)",
                        srcNode->firstChild) )
        {
            __debugbreak();
        }
        destNode->childCount = srcNode->childCount;
        if ( srcNode->childCount != destNode->childCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        6108,
                        0,
                        "%s\n\t(srcNode.childCount) = %i",
                        "(srcNode.childCount == destNode.childCount)",
                        srcNode->childCount) )
        {
            __debugbreak();
        }
        ClearBounds(destNode->mins, destNode->maxs);
        itemEnd = srcNode->itemCount + srcNode->firstItem;
        for ( itemIter = srcNode->firstItem; itemIter != itemEnd; ++itemIter )
            ExpandBounds(options->mins[itemIter], options->maxs[itemIter], destNode->mins, destNode->maxs);
    }
}

void __cdecl R_GenerateHighmipAabbs_AddWorldSurfaces(int *elemCount, int *elemArray, GenericAabbTreeOptions *options)
{
    float *v3; // [esp+0h] [ebp-1Ch]
    float *v4; // [esp+4h] [ebp-18h]
    GfxSurface *mins; // [esp+8h] [ebp-14h]
    float *maxs; // [esp+Ch] [ebp-10h]
    int surfCount; // [esp+10h] [ebp-Ch]
    int surfIter; // [esp+14h] [ebp-8h]

    surfCount = s_world.surfaceCount;
    for ( surfIter = 0; surfIter != surfCount; ++surfIter )
    {
        mins = &s_world.dpvs.surfaces[surfIter];
        maxs = mins->tris.maxs;
        if ( (float)(mins->tris.mins[0] + 0.000099999997) < mins->tris.maxs[0]
            && (float)(mins->tris.mins[1] + 0.000099999997) < s_world.dpvs.surfaces[surfIter].tris.maxs[1]
            && (float)(mins->tris.mins[2] + 0.000099999997) < s_world.dpvs.surfaces[surfIter].tris.maxs[2] )
        {
            v4 = options->mins[*elemCount];
            *v4 = mins->tris.mins[0];
            v4[1] = mins->tris.mins[1];
            v4[2] = mins->tris.mins[2];
            v3 = options->maxs[*elemCount];
            *v3 = *maxs;
            v3[1] = mins->tris.maxs[1];
            v3[2] = mins->tris.maxs[2];
            elemArray[(*elemCount)++] = surfIter;
        }
        else
        {
            ClearBounds(mins->tris.mins, maxs);
            if ( !IsClearedBounds(mins->tris.mins, maxs)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            6138,
                            0,
                            "%s",
                            "IsClearedBounds( mins, maxs )") )
            {
                __debugbreak();
            }
        }
    }
}

void __cdecl R_GenerateHighmipAabbs_AddStaticModelSurfaces(
                int *elemCount,
                int *elemArray,
                GenericAabbTreeOptions *options)
{
    float *v3; // [esp+0h] [ebp-E8h]
    float *v4; // [esp+4h] [ebp-E4h]
    float v5; // [esp+34h] [ebp-B4h]
    float v6; // [esp+3Ch] [ebp-ACh]
    float v7; // [esp+40h] [ebp-A8h]
    float v8; // [esp+44h] [ebp-A4h]
    float v9; // [esp+4Ch] [ebp-9Ch]
    float v10; // [esp+50h] [ebp-98h]
    float v11; // [esp+54h] [ebp-94h]
    float rotatedBounds[2][3]; // [esp+60h] [ebp-88h] BYREF
    float *origin; // [esp+78h] [ebp-70h]
    float originalBounds[2][3]; // [esp+7Ch] [ebp-6Ch] BYREF
    float axis[3][3]; // [esp+94h] [ebp-54h] BYREF
    GfxStaticModelDrawInst *modelInst; // [esp+B8h] [ebp-30h]
    float modelMaxs[3]; // [esp+BCh] [ebp-2Ch] BYREF
    int surfCount; // [esp+C8h] [ebp-20h]
    int surfIter; // [esp+CCh] [ebp-1Ch]
    float modelMins[3]; // [esp+D0h] [ebp-18h] BYREF
    int smodelIter; // [esp+DCh] [ebp-Ch]
    int smodelCount; // [esp+E0h] [ebp-8h]
    float SIZE_EPSILON; // [esp+E4h] [ebp-4h]

    SIZE_EPSILON = 0.000099999997f;
    smodelCount = s_world.dpvs.smodelCount;
    for ( smodelIter = 0; smodelIter != smodelCount; ++smodelIter )
    {
        modelInst = &s_world.dpvs.smodelDrawInsts[smodelIter];
        surfCount = modelInst->model->numsurfs;
        if ( surfCount )
        {
            ClearBounds(modelMins, modelMaxs);
            for ( surfIter = 0; surfIter != surfCount; ++surfIter )
            {
                v5 = modelInst->placement.axis[0][2];
                v6 = modelInst->placement.axis[1][0];
                v7 = modelInst->placement.axis[1][1];
                v8 = modelInst->placement.axis[1][2];
                v9 = modelInst->placement.axis[2][0];
                v10 = modelInst->placement.axis[2][1];
                v11 = modelInst->placement.axis[2][2];
                *(_QWORD *)&axis[0][0] = *(_QWORD *)&modelInst->placement.axis[0][0];
                axis[0][2] = v5;
                axis[1][0] = v6;
                axis[1][1] = v7;
                axis[1][2] = v8;
                axis[2][0] = v9;
                axis[2][1] = v10;
                axis[2][2] = v11;
                origin = modelInst->placement.origin;
                XModelGetSurfaceStreamBounds(modelInst->model, surfIter, originalBounds[0], originalBounds[1]);
                GetRotatedBounds(originalBounds, origin, axis, rotatedBounds);
                ExpandBounds(rotatedBounds[0], rotatedBounds[1], modelMins, modelMaxs);
            }
            if ( (float)(modelMins[0] + SIZE_EPSILON) < modelMaxs[0]
                && (float)(modelMins[1] + SIZE_EPSILON) < modelMaxs[1]
                && (float)(modelMins[2] + SIZE_EPSILON) < modelMaxs[2] )
            {
                v4 = options->mins[*elemCount];
                *v4 = modelMins[0];
                v4[1] = modelMins[1];
                v4[2] = modelMins[2];
                v3 = options->maxs[*elemCount];
                *v3 = modelMaxs[0];
                v3[1] = modelMaxs[1];
                v3[2] = modelMaxs[2];
                elemArray[(*elemCount)++] = ~smodelIter;
            }
        }
    }
}

unsigned __int8 *R_LoadWorldRuntime()
{
    unsigned __int8 *v1; // [esp+0h] [ebp-2Ch]
    unsigned __int8 *v2; // [esp+4h] [ebp-28h]
    unsigned __int8 *v3; // [esp+8h] [ebp-24h]
    unsigned __int8 *v4; // [esp+Ch] [ebp-20h]
    unsigned __int8 *v5; // [esp+10h] [ebp-1Ch]
    unsigned __int8 *v6; // [esp+14h] [ebp-18h]
    unsigned __int8 *v7; // [esp+18h] [ebp-14h]
    unsigned __int8 *v8; // [esp+1Ch] [ebp-10h]
    unsigned __int8 *v9; // [esp+20h] [ebp-Ch]
    unsigned int drawType; // [esp+24h] [ebp-8h]
    unsigned int drawTypea; // [esp+24h] [ebp-8h]
    int i; // [esp+28h] [ebp-4h]

    for ( i = 0; i < 3; ++i )
    {
        s_world.dpvs.smodelVisDataCount = 4 * ((s_world.dpvs.smodelCount + 127) >> 7);
        s_world.dpvs.surfaceVisDataCount = 4 * ((s_world.models->surfaceCount + 127) >> 7);
        if ( s_world.dpvs.smodelCount )
            v9 = Hunk_Alloc(s_world.dpvs.smodelCount, "R_InitDynamicData", 22);
        else
            v9 = 0;
        s_world.dpvs.smodelVisData[i] = v9;
        if ( s_world.models->surfaceCount )
            v8 = Hunk_Alloc(s_world.models->surfaceCount, "R_InitDynamicData", 21);
        else
            v8 = 0;
        s_world.dpvs.surfaceVisData[i] = v8;
        for ( drawType = 0; drawType < 2; ++drawType )
            s_world.dpvsDyn.dynEntVisData[drawType][i] = Hunk_Alloc(
                                                                                                         32 * s_world.dpvsDyn.dynEntClientWordCount[drawType],
                                                                                                         "R_InitDynamicData",
                                                                                                         21);
    }
    if ( s_world.dpvs.smodelCount )
        v7 = Hunk_Alloc(s_world.dpvs.smodelCount, "R_InitDynamicData", 22);
    else
        v7 = 0;
    s_world.dpvs.smodelVisDataCameraSaved = v7;
    if ( s_world.models->surfaceCount )
        v6 = Hunk_Alloc(s_world.models->surfaceCount, "R_InitDynamicData", 21);
    else
        v6 = 0;
    s_world.dpvs.surfaceVisDataCameraSaved = v6;
    if ( s_world.dpvs.smodelCount )
        v5 = Hunk_Alloc(8 * s_world.dpvs.smodelVisDataCount, "R_InitDynamicData", 22);
    else
        v5 = 0;
    s_world.dpvs.lodData = (unsigned int *)v5;
    s_world.dpvs.staticSurfaceCount = s_world.models->surfaceCount;
    if ( s_world.dpvs.staticSurfaceCount )
        v4 = Hunk_Alloc(8 * s_world.dpvs.staticSurfaceCount, "R_InitDynamicData", 21);
    else
        v4 = 0;
    s_world.dpvs.surfaceMaterials = (GfxDrawSurf *)v4;
    if ( s_world.dpvs.staticSurfaceCount )
        v3 = Hunk_Alloc(4 * s_world.dpvs.surfaceVisDataCount, "R_InitDynamicData", 21);
    else
        v3 = 0;
    s_world.dpvs.surfaceCastsSunShadow = (unsigned int *)v3;
    if ( s_world.dpvsPlanes.cellCount )
        v2 = Hunk_Alloc(
                     4 * s_world.dpvsPlanes.cellCount * ((s_world.dpvsPlanes.cellCount + 31) >> 5),
                     "R_InitDynamicData",
                     21);
    else
        v2 = 0;
    s_world.cellCasterBits = (unsigned int *)v2;
    if ( s_world.dpvsPlanes.cellCount )
        v1 = Hunk_Alloc(s_world.dpvsPlanes.cellCount << 11, "R_InitDynamicData", 21);
    else
        v1 = 0;
    s_world.dpvsPlanes.sceneEntCellBits = (unsigned int *)v1;
    for ( drawTypea = 0; drawTypea < 2; ++drawTypea )
        s_world.dpvsDyn.dynEntCellBits[drawTypea] = (unsigned int *)Hunk_Alloc(
                                                                                                                                    4
                                                                                                                                * s_world.dpvsPlanes.cellCount
                                                                                                                                * s_world.dpvsDyn.dynEntClientWordCount[drawTypea],
                                                                                                                                    "R_InitDynamicData",
                                                                                                                                    21);
    s_world.sceneDynModel = (GfxSceneDynModel *)Hunk_Alloc(
                                                                                                6 * s_world.dpvsDyn.dynEntClientCount[0],
                                                                                                "R_InitDynamicData",
                                                                                                21);
    s_world.sceneDynBrush = (GfxSceneDynBrush *)Hunk_Alloc(
                                                                                                4 * s_world.dpvsDyn.dynEntClientCount[1],
                                                                                                "R_InitDynamicData",
                                                                                                21);
    return R_AllocPrimaryLightBuffers();
}

unsigned __int8 *R_AllocPrimaryLightBuffers()
{
    unsigned __int16 EntityCount; // ax
    unsigned __int16 v1; // ax
    unsigned __int8 *result; // eax
    unsigned int relevantPrimaryLightCount; // [esp+4h] [ebp-Ch]
    unsigned int totalDynEntBits; // [esp+8h] [ebp-8h]

    if ( s_world.sunPrimaryLightIndex >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    6742,
                    0,
                    "%s",
                    "s_world.sunPrimaryLightIndex == PRIMARY_LIGHT_SUN || s_world.sunPrimaryLightIndex == PRIMARY_LIGHT_NONE") )
    {
        __debugbreak();
    }
    relevantPrimaryLightCount = s_world.primaryLightCount - (s_world.sunPrimaryLightIndex + 1);
    s_world.primaryLightEntityShadowVis = (unsigned int *)Hunk_Alloc(
                                                                                                                    4 * (((relevantPrimaryLightCount << 13) + 31) >> 5),
                                                                                                                    "R_AllocPrimaryLightBuffers",
                                                                                                                    21);
    EntityCount = DynEnt_GetEntityCount(DYNENT_COLL_CLIENT_FIRST);
    s_world.nonSunPrimaryLightForModelDynEnt = Hunk_Alloc(EntityCount, "R_AllocPrimaryLightBuffers", 21);
    totalDynEntBits = relevantPrimaryLightCount * DynEnt_GetEntityCount(DYNENT_COLL_CLIENT_FIRST);
    s_world.primaryLightDynEntShadowVis[0] = (unsigned int *)Hunk_Alloc(
                                                                                                                         4 * ((totalDynEntBits + 31) >> 5),
                                                                                                                         "R_AllocPrimaryLightBuffers",
                                                                                                                         21);
    v1 = DynEnt_GetEntityCount(DYNENT_COLL_CLIENT_BRUSH);
    result = Hunk_Alloc(4 * ((relevantPrimaryLightCount * v1 + 31) >> 5), "R_AllocPrimaryLightBuffers", 21);
    s_world.primaryLightDynEntShadowVis[1] = (unsigned int *)result;
    return result;
}

void R_LoadInitSkyIntensity()
{
    s_world.skyDynIntensity.angle0 = 0.0f;
    s_world.skyDynIntensity.angle1 = 0.0f;
    s_world.skyDynIntensity.factor0 = 1.0f;
    s_world.skyDynIntensity.factor1 = 1.0f;
}

unsigned int R_SetStaticModelReflectionProbes()
{
    unsigned int result; // eax
    unsigned int smodelIndex; // [esp+0h] [ebp-4h]

    if ( !rgl.reflectionProbesLoaded
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    6862,
                    0,
                    "%s",
                    "rgl.reflectionProbesLoaded") )
    {
        __debugbreak();
    }
    for ( smodelIndex = 0; ; ++smodelIndex )
    {
        result = smodelIndex;
        if ( smodelIndex >= s_world.dpvs.smodelCount )
            break;
        R_SetStaticModelReflectionProbe(
            &s_world,
            &s_world.dpvs.smodelInsts[smodelIndex],
            &s_world.dpvs.smodelDrawInsts[smodelIndex]);
    }
    rgl.staticModelReflectionProbesLoaded = 1;
    return result;
}

unsigned __int8 *R_AllocShadowGeometryHeaderMemory()
{
    unsigned __int8 *result; // eax

    if ( s_world.shadowGeom
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    6872,
                    1,
                    "%s",
                    "s_world.shadowGeom == NULL") )
    {
        __debugbreak();
    }
    result = Hunk_Alloc(12 * s_world.primaryLightCount, "R_AllocShadowGeometryHeaderMemory", 21);
    s_world.shadowGeom = (GfxShadowGeometry *)result;
    return result;
}

GfxShadowGeometry *R_InitShadowGeometryArrays()
{
    GfxShadowGeometry *result = NULL; // eax
    GfxStaticModelDrawInst *smodelDrawInst; // [esp+0h] [ebp-10h]
    unsigned int primaryLightIndex; // [esp+4h] [ebp-Ch]
    GfxShadowGeometry *shadowGeom; // [esp+8h] [ebp-8h]
    GfxShadowGeometry *shadowGeoma; // [esp+8h] [ebp-8h]
    unsigned int smodelIndex; // [esp+Ch] [ebp-4h]

    if ( !s_world.shadowGeom
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    6904,
                    1,
                    "%s",
                    "s_world.shadowGeom") )
    {
        __debugbreak();
    }
    R_ForEachShadowCastingSurfaceOnEachLight((void (__cdecl *)(GfxWorld *, unsigned int, unsigned int))R_IncrementShadowGeometryCount);
    s_world.shadowGeom->surfaceCount = 0;
    s_world.shadowGeom[s_world.sunPrimaryLightIndex].surfaceCount = 0;
    for ( primaryLightIndex = 0; primaryLightIndex < s_world.primaryLightCount; ++primaryLightIndex )
    {
        shadowGeom = &s_world.shadowGeom[primaryLightIndex];
        if ( shadowGeom->surfaceCount )
        {
            shadowGeom->sortedSurfIndex = (unsigned __int16 *)Hunk_Alloc(
                                                                                                                    2 * shadowGeom->surfaceCount,
                                                                                                                    "R_AllocShadowGeometryArrayMemory",
                                                                                                                    21);
            shadowGeom->surfaceCount = 0;
        }
        if ( shadowGeom->smodelCount )
        {
            shadowGeom->smodelIndex = (unsigned __int16 *)Hunk_Alloc(
                                                                                                            2 * shadowGeom->smodelCount,
                                                                                                            "R_AllocShadowGeometryArrayMemory",
                                                                                                            21);
            shadowGeom->smodelCount = 0;
        }
    }
    R_ForEachShadowCastingSurfaceOnEachLight(R_AddShadowSurfaceToPrimaryLight);
    for ( smodelIndex = 0; smodelIndex < s_world.dpvs.smodelCount; ++smodelIndex )
    {
        smodelDrawInst = &s_world.dpvs.smodelDrawInsts[smodelIndex];
        if ( smodelDrawInst->primaryLightIndex >= s_world.primaryLightCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                        6931,
                        0,
                        "smodelDrawInst->primaryLightIndex doesn't index s_world.primaryLightCount\n\t%i not in [0, %i)",
                        smodelDrawInst->primaryLightIndex,
                        s_world.primaryLightCount) )
        {
            __debugbreak();
        }
        result = &s_world.shadowGeom[smodelDrawInst->primaryLightIndex];
        shadowGeoma = result;
        if ( result->smodelIndex )
        {
            result->smodelIndex[result->smodelCount] = smodelIndex;
            if ( result->smodelIndex[result->smodelCount] != smodelIndex
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            6937,
                            1,
                            "shadowGeom->smodelIndex[shadowGeom->smodelCount] == smodelIndex\n\t%i, %i",
                            result->smodelIndex[result->smodelCount],
                            smodelIndex) )
            {
                __debugbreak();
            }
            result = shadowGeoma;
            ++shadowGeoma->smodelCount;
        }
    }
    return result;
}

void __cdecl R_ForEachShadowCastingSurfaceOnEachLight(void (__cdecl *Callback)(GfxWorld *, unsigned int, unsigned int))
{
    unsigned int sortedSurfIndex; // [esp+0h] [ebp-8h]

    if ( !s_world.shadowGeom
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    6881,
                    1,
                    "%s",
                    "s_world.shadowGeom") )
    {
        __debugbreak();
    }
    if ( !s_world.lightRegion
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    6882,
                    1,
                    "%s",
                    "s_world.lightRegion") )
    {
        __debugbreak();
    }
    for ( sortedSurfIndex = 0; sortedSurfIndex < s_world.models->surfaceCount; ++sortedSurfIndex )
    {
        if ( (s_world.dpvs.surfaces[sortedSurfIndex].material->info.gameFlags & 0x40) != 0 )
            R_ForEachPrimaryLightAffectingSurface(
                &s_world,
                &s_world.dpvs.surfaces[sortedSurfIndex],
                sortedSurfIndex,
                Callback);
    }
}

void __cdecl R_IncrementShadowGeometryCount(GfxWorld *world, unsigned int primaryLightIndex)
{
    ++world->shadowGeom[primaryLightIndex].surfaceCount;
}

void R_AddAllProbesToAllCells()
{
    GfxCell *cell; // [esp+0h] [ebp-Ch]
    GfxCell *cella; // [esp+0h] [ebp-Ch]
    int cellIndex; // [esp+4h] [ebp-8h]
    int cellIndexa; // [esp+4h] [ebp-8h]
    unsigned __int8 reflectionProbeIndex; // [esp+Bh] [ebp-1h]

    if ( !s_world.draw.reflectionProbeCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                    6953,
                    0,
                    "%s",
                    "s_world.draw.reflectionProbeCount > 0") )
    {
        __debugbreak();
    }
    if ( s_world.draw.reflectionProbeCount == 1 )
    {
        for ( cellIndex = 0; cellIndex < s_world.dpvsPlanes.cellCount; ++cellIndex )
        {
            cell = &s_world.cells[cellIndex];
            if ( cell->reflectionProbeCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            6960,
                            0,
                            "%s",
                            "cell->reflectionProbeCount == 0") )
            {
                __debugbreak();
            }
            if ( cell->reflectionProbes )
            {
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                                6961,
                                0,
                                "%s",
                                "cell->reflectionProbes == NULL") )
                    __debugbreak();
            }
            cell->reflectionProbeCount = 1;
            cell->reflectionProbes = Hunk_Alloc(1u, "R_AddAllProbesToAllCells", 23);
            *cell->reflectionProbes = 0;
        }
    }
    else
    {
        for ( cellIndexa = 0; cellIndexa < s_world.dpvsPlanes.cellCount; ++cellIndexa )
        {
            cella = &s_world.cells[cellIndexa];
            if ( cella->reflectionProbeCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            6972,
                            0,
                            "%s",
                            "cell->reflectionProbeCount == 0") )
            {
                __debugbreak();
            }
            if ( cella->reflectionProbes
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            6973,
                            0,
                            "%s",
                            "cell->reflectionProbes == NULL") )
            {
                __debugbreak();
            }
            cella->reflectionProbeCount = LOBYTE(s_world.draw.reflectionProbeCount) - 1;
            if ( cella->reflectionProbeCount != s_world.draw.reflectionProbeCount - 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp",
                            6975,
                            0,
                            "%s",
                            "cell->reflectionProbeCount == s_world.draw.reflectionProbeCount - 1") )
            {
                __debugbreak();
            }
            cella->reflectionProbes = Hunk_Alloc(cella->reflectionProbeCount, "R_AddAllProbesToAllCells", 23);
            for ( reflectionProbeIndex = 0; reflectionProbeIndex < s_world.draw.reflectionProbeCount - 1; ++reflectionProbeIndex )
                cella->reflectionProbes[reflectionProbeIndex] = reflectionProbeIndex + 1;
        }
    }
}

void __cdecl R_InterpretSunLightParseParamsIntoLights(SunLightParseParams *sunParse, GfxLight *sunLight)
{
    float sunColor[3]; // [esp+0h] [ebp-1Ch] BYREF
    float sunScale; // [esp+Ch] [ebp-10h]
    float sunDirection[3]; // [esp+10h] [ebp-Ch] BYREF

    AngleVectors(sunParse->sunSettings[0].angles, sunDirection, 0, 0);
    sunScale = sunParse->sunSettings[0].sunDiffuseColor[3];
    sunColor[0] = sunParse->sunSettings[0].sunDiffuseColor[0] * sunScale;
    sunColor[1] = sunParse[1].sunSettings[0].skyColor[0] * sunScale;
    sunColor[2] = sunParse[1].sunSettings[0].skyColor[1] * sunScale;
    if ( sunLight )
    {
        R_SetUpSunLight(sunColor, sunDirection, sunLight);
        sunLight->diffuseColor[0] = sunParse->sunSettings[0].sunDiffuseColor[0]
                                                            * sunParse->sunSettings[0].sunDiffuseColor[3];
        sunLight->diffuseColor[1] = sunParse->sunSettings[0].sunDiffuseColor[1]
                                                            * sunParse->sunSettings[0].sunDiffuseColor[3];
        sunLight->diffuseColor[2] = sunParse->sunSettings[0].sunDiffuseColor[2]
                                                            * sunParse->sunSettings[0].sunDiffuseColor[3];
        sunLight->diffuseColor[3] = sunParse->sunSettings[0].sunDiffuseColor[3];
        sunLight->specularColor[0] = sunLight->diffuseColor[0];
        sunLight->specularColor[1] = sunLight->diffuseColor[1];
        sunLight->specularColor[2] = sunLight->diffuseColor[2];
        sunLight->specularColor[3] = sunLight->diffuseColor[3];
    }
}

void __cdecl R_SetUpSunLight(const float *sunColor, const float *sunDirection, GfxLight *light)
{
    if ( !light
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp", 7212, 0, "%s", "light") )
    {
        __debugbreak();
    }
    memset(&light->type, 0, sizeof(GfxLight));
    light->type = 1;
    light->dir[0] = *sunDirection;
    light->dir[1] = sunDirection[1];
    light->dir[2] = sunDirection[2];
    light->color[0] = *sunColor;
    light->color[1] = sunColor[1];
    light->color[2] = sunColor[2];
}

void __cdecl R_InitPrimaryLights(GfxLight *primaryLights)
{
    GfxLight *out; // [esp+70h] [ebp-Ch]
    const ComPrimaryLight *in; // [esp+74h] [ebp-8h]
    unsigned int lightIndex; // [esp+78h] [ebp-4h]
    int savedregs; // [esp+7Ch] [ebp+0h] BYREF

    if ( !rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_bsp_load_obj.cpp", 7283, 0, "%s", "rgp.world") )
    {
        __debugbreak();
    }
    for ( lightIndex = 0; lightIndex < rgp.world->primaryLightCount; ++lightIndex )
    {
        in = Com_GetPrimaryLight(lightIndex);
        out = &primaryLights[lightIndex];
        out->type = in->type;
        out->canUseShadowMap = in->canUseShadowMap;
        out->cullDist = in->cullDist;
        out->color[0] = in->color[0];
        out->color[1] = in->color[1];
        out->color[2] = in->color[2];
        out->dir[0] = in->dir[0];
        out->dir[1] = in->dir[1];
        out->dir[2] = in->dir[2];
        out->origin[0] = in->origin[0];
        out->origin[1] = in->origin[1];
        out->origin[2] = in->origin[2];
        out->radius = in->radius;
        out->cosHalfFovOuter = in->cosHalfFovOuter;
        out->cosHalfFovInner = in->cosHalfFovInner;
        out->exponent = in->exponent;
        if ( in->defName )
            out->def = R_RegisterLightDef(in->defName);
        else
            out->def = 0;
        out->diffuseColor[0] = in->diffuseColor[0];
        out->diffuseColor[1] = in->diffuseColor[1];
        out->diffuseColor[2] = in->diffuseColor[2];
        out->diffuseColor[3] = in->diffuseColor[3];
        out->specularColor[0] = in->specularColor[0];
        out->specularColor[1] = in->specularColor[1];
        out->specularColor[2] = in->specularColor[2];
        out->specularColor[3] = in->specularColor[3];
        out->attenuation[0] = in->attenuation[0];
        out->attenuation[1] = in->attenuation[1];
        out->attenuation[2] = in->attenuation[2];
        out->attenuation[3] = in->attenuation[3];
        out->falloff[0] = in->falloff[0];
        out->falloff[1] = in->falloff[1];
        out->falloff[2] = in->falloff[2];
        out->falloff[3] = in->falloff[3];
        out->aAbB[0] = in->aAbB[0];
        out->aAbB[1] = in->aAbB[1];
        out->aAbB[2] = in->aAbB[2];
        out->aAbB[3] = in->aAbB[3];
        out->angles[0] = in->angle[0];
        out->angles[1] = in->angle[1];
        out->angles[2] = in->angle[2];
        out->attenuation[3] = in->angle[3];
        out->spotShadowHiDistance = in->mipDistance;
        out->cookieControl0[0] = in->cookieControl0[0];
        out->cookieControl0[1] = in->cookieControl0[1];
        out->cookieControl0[2] = in->cookieControl0[2];
        out->cookieControl0[3] = in->cookieControl0[3];
        out->cookieControl1[0] = in->cookieControl1[0];
        out->cookieControl1[1] = in->cookieControl1[1];
        out->cookieControl1[2] = in->cookieControl1[2];
        out->cookieControl1[3] = in->cookieControl1[3];
        out->cookieControl2[0] = in->cookieControl2[0];
        out->cookieControl2[1] = in->cookieControl2[1];
        out->cookieControl2[2] = in->cookieControl2[2];
        out->cookieControl2[3] = in->cookieControl2[3];
        if ( out->type == 2 )
        {
            SpotLightViewMatrix(out->dir, out->angles[2], out->viewMatrix.m);
            SpotLightProjectionMatrix(out->cosHalfFovOuter, out->falloff[0], out->falloff[1], out->projMatrix.m);
        }
    }
    if ( rgp.world->sunPrimaryLightIndex )
        memcpy(&primaryLights[rgp.world->sunPrimaryLightIndex], rgp.world->sunLight, sizeof(GfxLight));
}

void __cdecl R_AddShadowSurfaceToPrimaryLight(
                GfxWorld *world,
                unsigned int primaryLightIndex,
                unsigned int sortedSurfIndex)
{
    GfxShadowGeometry *shadowGeom; // [esp+0h] [ebp-4h]

    shadowGeom = &world->shadowGeom[primaryLightIndex];
    if ( shadowGeom->sortedSurfIndex )
        shadowGeom->sortedSurfIndex[shadowGeom->surfaceCount++] = truncate_cast<unsigned short>(sortedSurfIndex);
}

void __cdecl R_ForEachPrimaryLightAffectingSurface(
                GfxWorld *world,
                const GfxSurface *surface,
                unsigned int sortedSurfIndex,
                void (__cdecl *Callback)(GfxWorld *, unsigned int, unsigned int))
{
    unsigned int primaryLightIndex; // [esp+24h] [ebp-20h]
    const ComPrimaryLight *light; // [esp+28h] [ebp-1Ch]
    float boxHalfSize[3]; // [esp+2Ch] [ebp-18h] BYREF
    float boxMidPoint[3]; // [esp+38h] [ebp-Ch] BYREF

    if ( (surface->flags & 2) != 0 || (surface->material->info.gameFlags & 2) == 0 )
    {
        boxMidPoint[0] = (float)(surface->bounds[0][0] + surface->bounds[1][0]) * 0.5;
        boxMidPoint[1] = (float)(surface->bounds[0][1] + surface->bounds[1][1]) * 0.5;
        boxMidPoint[2] = (float)(surface->bounds[0][2] + surface->bounds[1][2]) * 0.5;
        boxHalfSize[0] = boxMidPoint[0] - surface->bounds[0][0];
        boxHalfSize[1] = boxMidPoint[1] - surface->bounds[0][1];
        boxHalfSize[2] = boxMidPoint[2] - surface->bounds[0][2];
        boxHalfSize[0] = boxHalfSize[0] + 1.0;
        boxHalfSize[1] = boxHalfSize[1] + 1.0;
        boxHalfSize[2] = boxHalfSize[2] + 1.0;
        for ( primaryLightIndex = world->sunPrimaryLightIndex + 1;
                    primaryLightIndex < world->primaryLightCount;
                    ++primaryLightIndex )
        {
            light = Com_GetPrimaryLight(primaryLightIndex);
            if ( !Com_CullBoxFromPrimaryLight(light, boxMidPoint, boxHalfSize)
                && !R_CullBoxFromLightRegion(&s_world.lightRegion[primaryLightIndex], light->origin, boxMidPoint, boxHalfSize) )
            {
                Callback(world, primaryLightIndex, sortedSurfIndex);
            }
        }
    }
    else
    {
        Callback(world, surface->primaryLightIndex, sortedSurfIndex);
    }
}

