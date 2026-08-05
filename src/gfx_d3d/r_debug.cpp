#include "r_debug.h"
#include <win32/win_common.h>
#include "r_debug_alloc.h"
#include <physics/phys_convex_hull.h>

PolySet persistentPolySet;
DebugGlobals debugGlobals;

void __cdecl R_AddDebugPolygon(
                DebugGlobals *debugGlobalsEntry,
                int pointCount,
                float (*points)[3],
                const float *faceColor,
                bool faceDepthTest,
                const float *edgeColor,
                bool edgeDepthTest)
{
    R_AddDebugPolygonInternal(
        &debugGlobalsEntry->polySet,
        pointCount,
        points,
        faceColor,
        faceDepthTest,
        edgeColor,
        edgeDepthTest);
}

void __cdecl R_AddDebugPolygonInternal(
                PolySet *polySet,
                int pointCount,
                float (*points)[3],
                const float *faceColor,
                bool faceDepthTest,
                const float *edgeColor,
                bool edgeDepthTest)
{
    GfxDebugPoly *the_poly; // [esp+4h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_DEBUG_LINE);
    if ( pointCount + polySet->vertCount <= polySet->vertLimit )
    {
        if ( polySet->polyCount + 1 <= polySet->polyLimit )
        {
            if ( !polySet->polys )
            {
                if ( polySet->verts
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_debug.cpp",
                                52,
                                0,
                                "%s",
                                "polySet->verts == NULL") )
                {
                    __debugbreak();
                }
                R_DebugAlloc((void **)&polySet->polys, 48 * polySet->polyLimit, "(debugGlobalsEntry->polys)");
                R_DebugAlloc((void **)&polySet->verts, 12 * polySet->vertLimit, "(debugGlobalsEntry->verts)");
            }
            if ( polySet->polys && polySet->verts )
            {
                the_poly = &polySet->polys[polySet->polyCount];
                the_poly->firstVert = polySet->vertCount;
                the_poly->vertCount = pointCount;
                the_poly->faceColor[0] = *faceColor;
                the_poly->faceColor[1] = faceColor[1];
                the_poly->faceColor[2] = faceColor[2];
                the_poly->faceColor[3] = faceColor[3];
                the_poly->faceDepthTest = faceDepthTest;
                the_poly->edgeColor[0] = *edgeColor;
                the_poly->edgeColor[1] = edgeColor[1];
                the_poly->edgeColor[2] = edgeColor[2];
                the_poly->edgeColor[3] = edgeColor[3];
                the_poly->edgeDepthTest = edgeDepthTest;
                ++polySet->polyCount;
                memcpy((unsigned __int8 *)polySet->verts[polySet->vertCount], (unsigned __int8 *)points, 12 * pointCount);
                polySet->vertCount += pointCount;
                Sys_LeaveCriticalSection(CRITSECT_DEBUG_LINE);
            }
            else
            {
                Sys_LeaveCriticalSection(CRITSECT_DEBUG_LINE);
            }
        }
        else
        {
            Sys_LeaveCriticalSection(CRITSECT_DEBUG_LINE);
            Com_Printf(0, "Debug polygon limit exceeded, current=%d, limit=%d\n", polySet->polyCount, polySet->polyLimit);
        }
    }
    else
    {
        Sys_LeaveCriticalSection(CRITSECT_DEBUG_LINE);
        Com_Printf(
            0,
            "Debug vertex limit exceeded, current=%d, adding=%d, limit=%d\n",
            polySet->vertCount,
            pointCount,
            polySet->vertLimit);
    }
}

void __cdecl R_AddPersistentDebugPolygon(
                DebugGlobals *debugGlobalsEntry,
                int pointCount,
                float (*points)[3],
                const float *faceColor,
                bool faceDepthTest,
                const float *edgeColor,
                bool edgeDepthTest)
{
    R_AddDebugPolygonInternal(&persistentPolySet, pointCount, points, faceColor, faceDepthTest, edgeColor, edgeDepthTest);
}

void __cdecl R_ClearPersistentDebugPolygons()
{
    Sys_EnterCriticalSection(CRITSECT_DEBUG_LINE);
    persistentPolySet.vertCount = 0;
    persistentPolySet.polyCount = 0;
    Sys_LeaveCriticalSection(CRITSECT_DEBUG_LINE);
}

void __cdecl R_AddDebugAxis(
                DebugGlobals *debugGlobalsEntry,
                const float *origin,
                const float (*axis)[3],
                float length,
                int doDepthTest)
{
    int i; // [esp+4h] [ebp-1Ch]
    float end[3]; // [esp+8h] [ebp-18h] BYREF
    const float *axisColors[3]; // [esp+14h] [ebp-Ch]

    axisColors[0] = colorRed;
    axisColors[1] = colorGreen;
    axisColors[2] = colorBlue;
    for ( i = 0; i < 3; ++i )
    {
        end[0] = (float)(length * (float)(*axis)[3 * i]) + *origin;
        end[1] = (float)(length * (float)(*axis)[3 * i + 1]) + origin[1];
        end[2] = (float)(length * (float)(*axis)[3 * i + 2]) + origin[2];
        R_AddDebugLine(debugGlobalsEntry, origin, end, axisColors[i], doDepthTest);
    }
}

void __cdecl R_AddDebugLine(
                DebugGlobals *debugGlobalsEntry,
                const float *start,
                const float *end,
                const float *color,
                int doDepthTest)
{
    trDebugLine_t *pDebugLine; // [esp+8h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_DEBUG_LINE);
    if ( debugGlobalsEntry->lineCount + 1 <= debugGlobalsEntry->lineLimit )
    {
        if ( debugGlobalsEntry->lines
            || (R_DebugAlloc((void **)&debugGlobalsEntry->lines, 44 * debugGlobalsEntry->lineLimit, "R_AddDebugLine"),
                    debugGlobalsEntry->lines) )
        {
            pDebugLine = &debugGlobalsEntry->lines[debugGlobalsEntry->lineCount];
            pDebugLine->start[0] = *start;
            pDebugLine->start[1] = start[1];
            pDebugLine->start[2] = start[2];
            pDebugLine->end[0] = *end;
            pDebugLine->end[1] = end[1];
            pDebugLine->end[2] = end[2];
            pDebugLine->color[0] = *color;
            pDebugLine->color[1] = color[1];
            pDebugLine->color[2] = color[2];
            pDebugLine->color[3] = color[3];
            pDebugLine->depthTest = doDepthTest;
            ++debugGlobalsEntry->lineCount;
            Sys_LeaveCriticalSection(CRITSECT_DEBUG_LINE);
        }
        else
        {
            Sys_LeaveCriticalSection(CRITSECT_DEBUG_LINE);
        }
    }
    else
    {
        Sys_LeaveCriticalSection(CRITSECT_DEBUG_LINE);
        Com_Printf(
            0,
            "Debug line limit exceeded, current=%d, limit=%d\n",
            debugGlobalsEntry->lineCount,
            debugGlobalsEntry->lineLimit);
    }
}

void __cdecl R_AddDebugBox(DebugGlobals *debugGlobalsEntry, const float *mins, const float *maxs, const float *color)
{
    float v4; // [esp+0h] [ebp-6Ch]
    unsigned int j; // [esp+4h] [ebp-68h]
    unsigned int i; // [esp+8h] [ebp-64h]
    unsigned int ia; // [esp+8h] [ebp-64h]
    float v[8][3]; // [esp+Ch] [ebp-60h] BYREF

    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 3; ++j)
        {
            if ((i & (1 << j)) != 0)
                v4 = maxs[j];
            else
                v4 = mins[j];
            v[i][j] = v4;
        }
    }
    for (ia = 0; ia < 0xC; ++ia)
        R_AddDebugLine(debugGlobalsEntry, v[iEdgePairs[ia][0]], v[iEdgePairs[ia][1]], color, 0);
}

void __cdecl R_AddDebugString(
                DebugGlobals *debugGlobalsEntry,
                const float *origin,
                const float *color,
                float scale,
                char *string)
{
    trDebugString_t *pDebugString; // [esp+4h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_DEBUG_LINE);
    if ( debugGlobalsEntry->stringCount + 1 <= debugGlobalsEntry->stringLimit
        && (debugGlobalsEntry->strings
         || (R_DebugAlloc((void **)&debugGlobalsEntry->strings, debugGlobalsEntry->stringLimit << 7, "R_AddDebugString"),
                 debugGlobalsEntry->strings)) )
    {
        pDebugString = &debugGlobalsEntry->strings[debugGlobalsEntry->stringCount];
        pDebugString->xyz[0] = *origin;
        pDebugString->xyz[1] = origin[1];
        pDebugString->xyz[2] = origin[2];
        pDebugString->color[0] = *color;
        pDebugString->color[1] = color[1];
        pDebugString->color[2] = color[2];
        pDebugString->color[3] = color[3];
        pDebugString->scale = scale;
        strncpy((char *)pDebugString->text, (const char *)string, 0x5Fu);
        pDebugString->text[95] = 0;
        ++debugGlobalsEntry->stringCount;
        Sys_LeaveCriticalSection(CRITSECT_DEBUG_LINE);
    }
    else
    {
        Sys_LeaveCriticalSection(CRITSECT_DEBUG_LINE);
    }
}

void __cdecl R_AddScaledDebugString(
                DebugGlobals *debugGlobalsEntry,
                const GfxViewParms *viewParms,
                const float *origin,
                const float *color,
                char *string)
{
    float delta[3]; // [esp+1Ch] [ebp-14h] BYREF
    float scale; // [esp+28h] [ebp-8h]
    float dot; // [esp+2Ch] [ebp-4h]

    delta[0] = *origin - viewParms->origin[0];
    delta[1] = origin[1] - viewParms->origin[1];
    delta[2] = origin[2] - viewParms->origin[2];
    scale = Vec3Normalize(delta);
    dot = (float)((float)(delta[0] * viewParms->axis[0][0]) + (float)(delta[1] * viewParms->axis[0][1]))
            + (float)(delta[2] * viewParms->axis[0][2]);
    scale = (float)(dot - 0.995) * scale;
    if ( scale < 1.0 )
        scale = 1.0f;
    R_AddDebugString(debugGlobalsEntry, origin, color, scale, string);
}

void __cdecl R_InitDebugEntry(DebugGlobals *debugGlobalsEntry)
{
    memset(debugGlobalsEntry, 0, sizeof(DebugGlobals));
    debugGlobalsEntry->polySet.vertLimit = 0x8000;
    debugGlobalsEntry->polySet.polyLimit = 0x2000;
    debugGlobalsEntry->stringLimit = 4096;
    debugGlobalsEntry->lineLimit = 0x4000;
    debugGlobalsEntry->sphereLimit = 1024;
    debugGlobalsEntry->plumeLimit = 4096;
}

void __cdecl R_InitDebug()
{
    R_InitDebugEntry(&debugGlobals);
    persistentPolySet.vertLimit = 4096;
    persistentPolySet.polyLimit = 1024;
}

void __cdecl R_ShutdownDebugEntry(DebugGlobals *debugGlobalsEntry)
{
    R_DebugFree((void **)&debugGlobalsEntry->polySet.polys);
    R_DebugFree((void **)&debugGlobalsEntry->polySet.verts);
    R_DebugFree((void **)&debugGlobalsEntry->strings);
    R_DebugFree((void **)&debugGlobalsEntry->externStrings);
    R_DebugFree((void **)&debugGlobalsEntry->spheres);
    R_DebugFree((void **)&debugGlobalsEntry->externSpheres);
    R_DebugFree((void **)&debugGlobalsEntry->lines);
    R_DebugFree((void **)&debugGlobalsEntry->externLines);
    R_DebugFree((void **)&debugGlobalsEntry->plumes);
    free_chull_lists();
}

void __cdecl R_TransferDebugGlobals(DebugGlobals *debugGlobalsEntry)
{
    int dt; // [esp+8h] [ebp-8h]
    int plumeIndex; // [esp+Ch] [ebp-4h]

    plumeIndex = 0;
    while ( plumeIndex < debugGlobals.plumeCount )
    {
        dt = scene.def.time - debugGlobals.plumes[plumeIndex].startTime;
        if ( dt >= 0 && dt <= debugGlobals.plumes[plumeIndex].duration )
            ++plumeIndex;
        else
            memcpy(
                &debugGlobals.plumes[plumeIndex],
                &debugGlobals.plumes[--debugGlobals.plumeCount],
                sizeof(debugGlobals.plumes[plumeIndex]));
    }
    if ( debugGlobals.plumes )
    {
        if ( !debugGlobalsEntry->plumes )
        {
            R_DebugAlloc((void **)&debugGlobalsEntry->plumes, 40 * debugGlobals.plumeLimit, "(debugGlobalsEntry->plumes)");
            if ( !debugGlobalsEntry->plumes )
                return;
        }
        memcpy(
            (unsigned __int8 *)debugGlobalsEntry->plumes,
            (unsigned __int8 *)debugGlobals.plumes,
            40 * debugGlobals.plumeCount);
    }
    debugGlobalsEntry->plumeCount = debugGlobals.plumeCount;
    if ( persistentPolySet.polys )
        R_UpdatePersistentDebugPolys(debugGlobalsEntry);
}

void __cdecl R_UpdatePersistentDebugPolys(DebugGlobals *debugGlobalsEntry)
{
    int ipoly; // [esp+4h] [ebp-4h]

    for ( ipoly = 0; ipoly < persistentPolySet.polyCount; ++ipoly )
        R_AddDebugPolygon(
            debugGlobalsEntry,
            persistentPolySet.polys[ipoly].vertCount,
            (float (*)[3])persistentPolySet.verts[persistentPolySet.polys[ipoly].firstVert],
            persistentPolySet.polys[ipoly].faceColor,
            persistentPolySet.polys[ipoly].faceDepthTest,
            persistentPolySet.polys[ipoly].edgeColor,
            persistentPolySet.polys[ipoly].edgeDepthTest);
}

void __cdecl R_ShutdownDebug()
{
    R_ShutdownDebugEntry(&debugGlobals);
    R_DebugFree((void **)&persistentPolySet.polys);
    R_DebugFree((void **)&persistentPolySet.verts);
}

void __cdecl R_CopyDebugStrings(
                trDebugString_t *clStrings,
                int clStringCnt,
                trDebugString_t *svStrings,
                int svStringCnt,
                int maxStringCount)
{
    DebugGlobals *dg; // [esp+0h] [ebp-Ch]
    unsigned __int8 *dest; // [esp+8h] [ebp-4h]

    dg = &frontEndDataOut->debugGlobals;
    frontEndDataOut->debugGlobals.externStringCount = 0;
    if ( !clStrings && !svStrings
        || dg->externStrings
        || (R_DebugAlloc((void **)&dg->externStrings, maxStringCount << 7, "R_CopyDebugStrings"), dg->externStrings) )
    {
        dest = (unsigned __int8 *)dg->externStrings;
        if ( clStrings )
        {
            memcpy(dest, (unsigned __int8 *)clStrings, clStringCnt << 7);
            dest += 128 * clStringCnt;
        }
        if ( svStrings )
            memcpy(dest, (unsigned __int8 *)svStrings, svStringCnt << 7);
        dg->externStringCount = svStringCnt + clStringCnt;
    }
}

void __cdecl R_CopyDebugLines(
                trDebugLine_t *clLines,
                int clLineCnt,
                trDebugLine_t *svLines,
                int svLineCnt,
                int maxLineCount)
{
    DebugGlobals *dg; // [esp+0h] [ebp-Ch]
    unsigned __int8 *dest; // [esp+8h] [ebp-4h]

    dg = &frontEndDataOut->debugGlobals;
    if ( !clLines && !svLines
        || frontEndDataOut->debugGlobals.externLines
        || (R_DebugAlloc((void **)&frontEndDataOut->debugGlobals.externLines, 44 * maxLineCount, "R_CopyDebugLines"),
                dg->externLines) )
    {
        dest = (unsigned __int8 *)dg->externLines;
        if ( clLines )
        {
            memcpy(dest, (unsigned __int8 *)clLines, 44 * clLineCnt);
            dest += 44 * clLineCnt;
        }
        if ( svLines )
            memcpy(dest, (unsigned __int8 *)svLines, 44 * svLineCnt);
        dg->externLineCount = svLineCnt + clLineCnt;
    }
}

void __cdecl R_CopyDebugSpheres(
                trDebugSphere_t *clSpheres,
                int clSphereCnt,
                trDebugSphere_t *svSpheres,
                int svSphereCnt,
                int maxSphereCount)
{
    DebugGlobals *dg; // [esp+0h] [ebp-Ch]
    unsigned __int8 *dest; // [esp+8h] [ebp-4h]

    dg = &frontEndDataOut->debugGlobals;
    if ( !clSpheres && !svSpheres
        || frontEndDataOut->debugGlobals.externSpheres
        || (R_DebugAlloc((void **)&frontEndDataOut->debugGlobals.externSpheres, 40 * maxSphereCount, "R_CopyDebugSpheres"),
                dg->externSpheres) )
    {
        dest = (unsigned __int8 *)dg->externSpheres;
        if ( clSpheres )
        {
            memcpy(dest, (unsigned __int8 *)clSpheres, 40 * clSphereCnt);
            dest += 40 * clSphereCnt;
        }
        if ( svSpheres )
            memcpy(dest, (unsigned __int8 *)svSpheres, 40 * svSphereCnt);
        dg->externSphereCount = svSphereCnt + clSphereCnt;
    }
}

