#include "r_staticmodelcache_load_obj.h"

#include <algorithm>
#include <xanim/xmodel_utils.h>
#include <universal/com_memory.h>
#include "r_xsurface.h"
#include <intrin.h> // _BitScanReverse

void __cdecl R_AssignSModelCacheResources(GfxWorld *world)
{
    unsigned __int8 v1; // [esp+17Bh] [ebp-2029h]
    XModel *model; // [esp+17Ch] [ebp-2028h]
    unsigned int smcUseCount[4]; // [esp+180h] [ebp-2024h] BYREF
    unsigned int lod; // [esp+190h] [ebp-2014h]
    GfxSModelSurfStats stats[512]; // [esp+194h] [ebp-2010h] BYREF
    unsigned int maxEntryCount; // [esp+2198h] [ebp-Ch]
    unsigned int v7; // [esp+219Ch] [ebp-8h]
    unsigned int i; // [esp+21A0h] [ebp-4h]

    if ( !world
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache_load_obj.cpp",
                    328,
                    0,
                    "%s",
                    "world") )
    {
        __debugbreak();
    }
    memset(smcUseCount, 0, sizeof(smcUseCount));
    v7 = R_OptimalSModelResourceStats(world, stats, 0x200u);
    //std::_Sort<DBReorderAssetEntry *,int,bool (__cdecl *)(DBReorderAssetEntry const &,DBReorderAssetEntry const &)>(
    //    stats,
    //    &stats[v7],
    //    (int)(16 * v7) >> 4,
    //    (bool (__cdecl *)(GfxSModelSurfStats *, GfxSModelSurfStats *))R_CompareSModelStats_Score);
    std::sort(stats, stats + v7, R_CompareSModelStats_Score);
    for ( i = 0; i < v7; ++i )
    {
        model = stats[i].model;
        lod = stats[i].lod;
        maxEntryCount = R_GetEntryCount(&stats[i]);
        v1 = R_AssignSModelCacheIndex(stats[i].smcAllocBits, maxEntryCount, smcUseCount);
        XModelSetSModelCacheForLod(model, lod, v1, stats[i].smcAllocBits);
    }
}

unsigned __int8 __cdecl R_AssignSModelCacheIndex(
                char smcAllocBits,
                unsigned int maxEntryCount,
                unsigned int *smcUseCount)
{
    unsigned int leastUsedCache; // [esp+8h] [ebp-8h]
    unsigned int cacheIter; // [esp+Ch] [ebp-4h]

    leastUsedCache = 0;
    for ( cacheIter = 1; cacheIter < 4; ++cacheIter )
    {
        if ( smcUseCount[cacheIter] < smcUseCount[leastUsedCache] )
            leastUsedCache = cacheIter;
    }
    smcUseCount[leastUsedCache] += maxEntryCount << smcAllocBits;
    return leastUsedCache;
}

bool __cdecl R_CompareSModelStats_Score(GfxSModelSurfStats &s0, GfxSModelSurfStats &s1)
{
    return s1.useCount << s0.smcAllocBits < s0.useCount << s1.smcAllocBits;
}

unsigned int __cdecl R_GetEntryCount(GfxSModelSurfStats *stats)
{
    if ( 1 << (16 - LOBYTE(stats->smcAllocBits)) < (signed int)stats->useCount )
        return 1 << (16 - LOBYTE(stats->smcAllocBits));
    else
        return stats->useCount;
}

unsigned int __cdecl R_OptimalSModelResourceStats(GfxWorld *world, GfxSModelSurfStats *stats, unsigned int statLimit)
{
    const GfxStaticModelDrawInst **drawInstArray; // [esp+ECh] [ebp-14h]
    unsigned int smodelIter; // [esp+F0h] [ebp-10h]
    unsigned int smodelItera; // [esp+F0h] [ebp-10h]
    unsigned int smodelIterNext; // [esp+F8h] [ebp-8h]
    unsigned int statCount; // [esp+FCh] [ebp-4h]

    if ( !world
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache_load_obj.cpp",
                    268,
                    0,
                    "%s",
                    "world") )
    {
        __debugbreak();
    }
    if ( !world->dpvs.smodelCount )
        return 0;
    drawInstArray = (const GfxStaticModelDrawInst **)Hunk_AllocateTempMemory(4 * world->dpvs.smodelCount, "R_AssignSModelCacheResources");
    for ( smodelIter = 0; smodelIter != world->dpvs.smodelCount; ++smodelIter )
        drawInstArray[smodelIter] = &world->dpvs.smodelDrawInsts[smodelIter];
    //std::_Sort<Material * *,int,bool (__cdecl *)(Material const *,Material const *)>(
    //    drawInstArray,
    //    &drawInstArray[world->dpvs.smodelCount],
    //    (signed int)(4 * world->dpvs.smodelCount) >> 2,
    //    (bool (__cdecl *)(const GfxStaticModelDrawInst *, const GfxStaticModelDrawInst *))R_CompareSModels_Model);
    std::sort(drawInstArray, drawInstArray + world->dpvs.smodelCount, R_CompareSModels_Model);

    statCount = 0;
    for ( smodelItera = 0; smodelItera != world->dpvs.smodelCount; smodelItera = smodelIterNext )
    {
        for ( smodelIterNext = smodelItera;
                    smodelIterNext != world->dpvs.smodelCount
             && drawInstArray[smodelIterNext]->model == drawInstArray[smodelItera]->model;
                    ++smodelIterNext )
        {
            ;
        }
        statCount = R_AddSModelListStats(
                                    &drawInstArray[smodelItera],
                                    smodelIterNext - smodelItera,
                                    stats,
                                    statCount,
                                    statLimit);
    }
    Hunk_FreeTempMemory((char *)drawInstArray);
    return statCount;
}

unsigned int __cdecl R_AddSModelListStats(
                const GfxStaticModelDrawInst **drawInstArray,
                unsigned int drawInstCount,
                GfxSModelSurfStats *stats,
                unsigned int statsCount,
                unsigned int statsLimit)
{
    unsigned int lodIter; // [esp+8h] [ebp-30h]
    XModel *model; // [esp+Ch] [ebp-2Ch]
    float mins[2]; // [esp+10h] [ebp-28h] BYREF
    float prevLodDist; // [esp+18h] [ebp-20h]
    unsigned int lodCount; // [esp+1Ch] [ebp-1Ch]
    float maxs[2]; // [esp+20h] [ebp-18h] BYREF
    unsigned int drawInstIter; // [esp+28h] [ebp-10h]
    unsigned int useCount; // [esp+2Ch] [ebp-Ch]
    unsigned int smcAllocBits; // [esp+30h] [ebp-8h]
    float lodDist; // [esp+34h] [ebp-4h]

    if ( drawInstCount < 0x10 )
        return statsCount;
    model = (*drawInstArray)->model;
    lodCount = XModelGetNumLods(model);
    if ( lodCount + statsCount > statsLimit )
        return statsCount;
    ClearBounds2D(mins, maxs);
    for ( drawInstIter = 0; drawInstIter < drawInstCount; ++drawInstIter )
        AddPointToBounds2D(drawInstArray[drawInstIter]->placement.origin, mins, maxs);
    lodIter = 0;
    prevLodDist = 0.0f;
    while ( lodIter < lodCount )
    {
        lodDist = XModelGetLodDist(model, lodIter);
        smcAllocBits = R_GetSModelCacheAllocBits(model, lodIter);
        if ( smcAllocBits <= 9 )
        {
            useCount = R_MaxModelsInDistRange(drawInstArray, drawInstCount, mins, maxs, prevLodDist, lodDist);
            if ( useCount >= 0x10 )
            {
                stats[statsCount].model = model;
                stats[statsCount].lod = lodIter;
                stats[statsCount].useCount = useCount;
                stats[statsCount++].smcAllocBits = smcAllocBits;
            }
        }
        ++lodIter;
        prevLodDist = lodDist;
    }
    return statsCount;
}

// (aislop for countleadingzeroes)
int R_GetSModelCacheAllocBits(const XModel *model, unsigned int lod)
{
    unsigned int surfCount = XModelGetSurfCount(model, lod);

    XSurface *surfs;
    XModelGetSurfaces(model, &surfs, lod);

    unsigned int triCount = 0;
    unsigned int vertCount = 0;

    for (unsigned int i = 0; i < surfCount; ++i)
    {
        vertCount += XSurfaceGetNumVerts(&surfs[i]);
        triCount += XSurfaceGetNumTris(&surfs[i]);
    }

    unsigned int v6 = (4 * vertCount < 3 * triCount) ? 3 * triCount : 4 * vertCount;

    unsigned int minPoolSize = (v6 + 3) / 4; // number of 32-bit words

    // compute ceil(log2(minPoolSize))
    unsigned long index;
    if (minPoolSize == 0)
        return 4; // safety
    if (_BitScanReverse(&index, minPoolSize - 1))
    {
        unsigned int bits = 32 - (index ^ 0x1F); // original xor trick
        return (bits > 4) ? bits : 4;
    }
    return 4;
}

unsigned int __cdecl R_MaxModelsInDistRange(
                const GfxStaticModelDrawInst **drawInstArray,
                unsigned int drawInstCount,
                const float *mins,
                const float *maxs,
                float distMin,
                float distMax)
{
    float radiusDeviate; // [esp+0h] [ebp-5Ch]
    float yawDeviate; // [esp+4h] [ebp-58h]
    unsigned int v9; // [esp+Ch] [ebp-50h]
    float v10; // [esp+14h] [ebp-48h]
    float v11; // [esp+18h] [ebp-44h]
    unsigned int maxModelsInRange; // [esp+20h] [ebp-3Ch]
    unsigned int drawInstCutoff; // [esp+24h] [ebp-38h]
    float size; // [esp+2Ch] [ebp-30h]
    float size_4; // [esp+30h] [ebp-2Ch]
    unsigned int modelsInRange; // [esp+34h] [ebp-28h]
    unsigned int drawInstIter; // [esp+38h] [ebp-24h]
    unsigned int dartIndex; // [esp+40h] [ebp-1Ch]
    float distSq; // [esp+48h] [ebp-14h]
    float mid; // [esp+4Ch] [ebp-10h]
    float mid_4; // [esp+50h] [ebp-Ch]
    float testPos[2]; // [esp+54h] [ebp-8h] BYREF

    mid = (float)(*mins + *maxs) * 0.5;
    mid_4 = (float)(mins[1] + maxs[1]) * 0.5;
    size = mid - *mins;
    size_4 = mid_4 - mins[1];
    maxModelsInRange = 0;
    if ( drawInstCount >> 2 > 0x64 )
        v9 = drawInstCount >> 2;
    else
        v9 = 100;
    for ( dartIndex = 0; dartIndex < v9; ++dartIndex )
    {
        yawDeviate = Com_Random();
        radiusDeviate = Com_Random();
        PointInCircleFromUniformDeviates(radiusDeviate, yawDeviate, testPos);
        testPos[0] = (float)(testPos[0] * size) + mid;
        testPos[1] = (float)(testPos[1] * size_4) + mid_4;
        modelsInRange = 0;
        drawInstCutoff = drawInstCount - maxModelsInRange;
        for ( drawInstIter = 0; drawInstIter < drawInstCutoff; ++drawInstIter )
        {
            v10 = drawInstArray[drawInstIter]->placement.origin[0] - testPos[0];
            v11 = drawInstArray[drawInstIter]->placement.origin[1] - testPos[1];
            distSq = (float)(v10 * v10) + (float)(v11 * v11);
            if ( distSq > (float)(distMin * distMin) && (float)(distMax * distMax) > distSq )
            {
                ++modelsInRange;
                if ( drawInstCutoff < drawInstCount )
                    ++drawInstCutoff;
            }
        }
        if ( maxModelsInRange < modelsInRange )
        {
            maxModelsInRange = modelsInRange;
            if ( modelsInRange == drawInstCount )
                break;
        }
    }
    return maxModelsInRange;
}

bool R_CompareSModels_Model(const GfxStaticModelDrawInst *s0, const GfxStaticModelDrawInst *s1)
{
    return s0->model < s1->model;
}
