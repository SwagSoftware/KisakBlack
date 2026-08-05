#include "r_pretess.h"
#include <universal/q_shared.h>
#include "r_warn.h"
#include <win32/win_net.h>

void __cdecl R_InitDrawSurfListInfo(GfxDrawSurfListInfo *info)
{
    int n; // [esp+0h] [ebp-4h]

    for ( n = 0; n < 3; ++n )
    {
        if ( info->group[n].QueuedBatchCount )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_pretess.cpp",
                            232,
                            0,
                            "%s",
                            "info->group[n].QueuedBatchCount == 0") )
                __debugbreak();
        }
    }
    memset(info, 0, sizeof(GfxDrawSurfListInfo));
}

void __cdecl R_EmitDrawSurfList(const GfxDrawSurf *drawSurfs, unsigned int drawSurfCount, GfxBackEndData *data)
{
    int newDrawSurfCount; // [esp+0h] [ebp-4h]

    if ( !drawSurfs
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_pretess.cpp", 274, 0, "%s", "drawSurfs") )
    {
        __debugbreak();
    }
    if ( drawSurfCount )
    {
        newDrawSurfCount = drawSurfCount + data->drawSurfCount;
        if ( newDrawSurfCount <= 0x8000 )
        {
            Com_Memcpy(&data->drawSurfs[data->drawSurfCount], drawSurfs, 8 * drawSurfCount);
            data->drawSurfCount = newDrawSurfCount;
        }
        else
        {
            R_WarnOncePerFrame(R_WARN_MAX_DRAWSURFS);
        }
    }
}

void __cdecl R_MergeAndEmitDrawSurfLists(unsigned int firstStage, unsigned int stageCount, GfxBackEndData *data)
{
    int v3; // [esp+0h] [ebp-180h]
    unsigned int usedCount; // [esp+20h] [ebp-160h]
    unsigned int srcStageIndex; // [esp+24h] [ebp-15Ch]
    unsigned int srcStageIndexa; // [esp+24h] [ebp-15Ch]
    int freeDrawSurfCount; // [esp+28h] [ebp-158h]
    unsigned int stageIndex; // [esp+30h] [ebp-150h]
    unsigned int stageIndexa; // [esp+30h] [ebp-150h]
    int primarySortKey; // [esp+34h] [ebp-14Ch]
    GfxDrawSurf *drawSurfs[39]; // [esp+38h] [ebp-148h]
    unsigned int dstStageIndex; // [esp+DCh] [ebp-A4h]
    unsigned int drawSurfCount[40]; // [esp+E0h] [ebp-A0h]
    unsigned int stageCounta; // [esp+18Ch] [ebp+Ch]

    PROF_SCOPED("R_MergeAndEmitDrawSurfLists");

    if ( (!stageCount || stageCount > 0x27)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_pretess.cpp",
                    329,
                    0,
                    "stageCount not in [1, DRAW_SURF_TYPE_COUNT]\n\t%i not in [%i, %i]",
                    stageCount,
                    1,
                    39) )
    {
        __debugbreak();
    }
    freeDrawSurfCount = 0x8000 - data->drawSurfCount;
    if ( freeDrawSurfCount > 0 )
    {
        dstStageIndex = 0;
        for ( srcStageIndex = 0; srcStageIndex < stageCount; ++srcStageIndex )
        {
            stageIndex = srcStageIndex + firstStage;
            if ( scene.drawSurfCount[srcStageIndex + firstStage] > freeDrawSurfCount )
            {
                scene.drawSurfCount[stageIndex] = freeDrawSurfCount;
                R_WarnOncePerFrame(R_WARN_MAX_DRAWSURFS);
            }
            if ( scene.drawSurfCount[stageIndex] )
            {
                freeDrawSurfCount -= scene.drawSurfCount[stageIndex];
                drawSurfCount[dstStageIndex] = scene.drawSurfCount[stageIndex];
                drawSurfs[dstStageIndex++] = scene.drawSurfs[stageIndex];
            }
        }
        while ( dstStageIndex )
        {
            stageCounta = dstStageIndex;
            if ( dstStageIndex == 1 )
            {
                R_EmitDrawSurfList(drawSurfs[0], drawSurfCount[0], data);
                return;
            }
            primarySortKey = (drawSurfs[0]->packed >> 58) & 0x3F;
            for ( stageIndexa = 1; stageIndexa < dstStageIndex; ++stageIndexa )
            {
                if ( (unsigned __int8)((drawSurfs[stageIndexa]->packed >> 58) & 0x3F) < primarySortKey )
                    v3 = (drawSurfs[stageIndexa]->packed >> 58) & 0x3F;
                else
                    v3 = primarySortKey;
                primarySortKey = v3;
            }
            dstStageIndex = 0;
            for ( srcStageIndexa = 0; srcStageIndexa < stageCounta; ++srcStageIndexa )
            {
                usedCount = R_GetKeyCount(drawSurfs[srcStageIndexa], drawSurfCount[srcStageIndexa], primarySortKey);
                R_EmitDrawSurfList(drawSurfs[srcStageIndexa], usedCount, data);
                drawSurfs[dstStageIndex] = &drawSurfs[srcStageIndexa][usedCount];
                drawSurfCount[dstStageIndex] = drawSurfCount[srcStageIndexa] - usedCount;
                dstStageIndex += drawSurfCount[dstStageIndex] != 0;
            }
        }
    }
}

unsigned int __cdecl R_GetKeyCount(
                const GfxDrawSurf *drawSurfs,
                unsigned int drawSurfCount,
                unsigned int primarySortKey)
{
    unsigned int usedCount; // [esp+20h] [ebp-4h]

    if ( !drawSurfs
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_pretess.cpp", 299, 0, "%s", "drawSurfs") )
    {
        __debugbreak();
    }
    if ( !drawSurfCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_pretess.cpp", 300, 0, "%s", "drawSurfCount") )
    {
        __debugbreak();
    }
    usedCount = 0;
    do
    {
        if ( ((drawSurfs[usedCount].packed >> 58) & 0x3F) != primarySortKey )
            break;
        ++usedCount;
    }
    while ( usedCount < drawSurfCount );
    return usedCount;
}

void __cdecl R_BeginPreTess(bool lockBuffer)
{

    if ( lockBuffer )
    {
        while ( *frontEndDataOut->dynamicBufferCurrentFrame
                 && *frontEndDataOut->dynamicBufferCurrentFrame < frontEndDataOut->frameCount )
        {
            NET_Sleep(0);
        }
        *frontEndDataOut->dynamicBufferCurrentFrame = frontEndDataOut->frameCount;
    }
}

unsigned __int16 *__cdecl R_AllocPreTessIndices(int count)
{
    unsigned __int16 *indices; // [esp+0h] [ebp-4h]

    if ( !gfxBuf.preTessIndexBuffer->indices )
        return 0;
    if ( !count && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_pretess.cpp", 470, 0, "%s", "count") )
        __debugbreak();
    if ( count + gfxBuf.preTessIndexBuffer->used > gfxBuf.preTessIndexBuffer->total )
        return 0;
    indices = &gfxBuf.preTessIndexBuffer->indices[gfxBuf.preTessIndexBuffer->used];
    gfxBuf.preTessIndexBuffer->used += count;
    return indices;
}

