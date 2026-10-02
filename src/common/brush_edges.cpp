#include "brush_edges.h"
#include <universal/assertive.h>
#include <cstring>
#include <qcommon/common.h>
#include <physics/phys_local.h>

adjacencyWinding_t *__cdecl BuildBrushdAdjacencyWindingForSide(
                const char *name,
                float *sideNormal,
                int basePlaneIndex,
                const SimplePlaneIntersection *InPts,
                int InPtCount,
                adjacencyWinding_t *optionalOutWinding,
                int optionalOutWindingCount)
{
    SimplePlaneIntersection *v8; // eax
    const char *v9; // [esp+1Ch] [ebp-6058h]
    float *v10; // [esp+20h] [ebp-6054h]
    SimplePlaneIntersection *v11; // [esp+24h] [ebp-6050h]
    adjacencyWinding_t *winding; // [esp+2Ch] [ebp-6048h]
    int i2; // [esp+30h] [ebp-6044h] BYREF
    float perimiter1; // [esp+34h] [ebp-6040h]
    float plane[4]; // [esp+38h] [ebp-603Ch] BYREF
    int ptsCount; // [esp+48h] [ebp-602Ch]
    float v0[3073]; // [esp+4Ch] [ebp-6028h] BYREF
    char rv; // [esp+3053h] [ebp-3021h]
    SimplePlaneIntersection *xyz[1024]; // [esp+3054h] [ebp-3020h] BYREF
    SimplePlaneIntersection *cycle[2][1024]; // [esp+4054h] [ebp-2020h] BYREF
    int i0; // [esp+6054h] [ebp-20h] BYREF
    char v22; // [esp+6058h] [ebp-1Ch]
    char v23; // [esp+6059h] [ebp-1Bh]
    int cycleIndex; // [esp+605Ch] [ebp-18h]
    unsigned int cycleCount[2]; // [esp+6060h] [ebp-14h]
    int i1; // [esp+6068h] [ebp-Ch] BYREF
    float perimiter2; // [esp+606Ch] [ebp-8h]
    int plane2; // [esp+6070h] [ebp-4h]

    ptsCount = GetPointListAllowDupes(basePlaneIndex, InPts, InPtCount, (const SimplePlaneIntersection **)xyz, 1024);
    if ( ptsCount < 3 )
        return 0;
    if ( NumberOfUniquePoints((const SimplePlaneIntersection **)xyz, ptsCount) < 3 )
        return 0;
    ptsCount = ReduceToACycle(basePlaneIndex, (SimplePlaneIntersection **)xyz, ptsCount);
    if ( ptsCount < 3 )
        return 0;
    cycleIndex = 0;
    while ( ptsCount )
    {
        cycle[cycleIndex][0] = xyz[--ptsCount];
        cycleCount[cycleIndex] = 1;
        plane2 = SecondPlane(cycle[cycleIndex][0], basePlaneIndex);
        while ( ptsCount )
        {
            v8 = (SimplePlaneIntersection *)RemoveNextPointFormedByThisPlane(
                                                                                plane2,
                                                                                (const SimplePlaneIntersection **)xyz,
                                                                                (const SimplePlaneIntersection **)&xyz[ptsCount]);
            cycle[cycleIndex][cycleCount[cycleIndex]] = v8;
            if ( !cycle[cycleIndex][cycleCount[cycleIndex]] )
                break;
            plane2 = ThirdPlane(cycle[cycleIndex][cycleCount[cycleIndex]], basePlaneIndex, plane2);
            --ptsCount;
            ++cycleCount[cycleIndex];
        }
        if ( !IsPtFormedByThisPlane(plane2, cycle[cycleIndex][0])
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                        817,
                        1,
                        "%s",
                        "IsPtFormedByThisPlane( planeIndex, cycle[cycleIndex][0] )") )
        {
            __debugbreak();
        }
        if ( cycleIndex <= 0 )
        {
            cycleIndex = 1;
        }
        else
        {
            perimiter1 = CyclePerimiter((const SimplePlaneIntersection **)cycle[0], cycleCount[0]);
            perimiter2 = CyclePerimiter((const SimplePlaneIntersection **)cycle[1], cycleCount[1]);
            v22 = TestConvexWithoutNearPoints((const SimplePlaneIntersection **)cycle[0], cycleCount[0]);
            v23 = TestConvexWithoutNearPoints((const SimplePlaneIntersection **)cycle[1], cycleCount[1]);
            if ( CycleLess(v22, v23, perimiter1, perimiter2, cycleCount[0], cycleCount[1]) )
            {
                memcpy((unsigned __int8 *)cycle, (unsigned __int8 *)cycle[1], 4 * cycleCount[1]);
                cycleCount[0] = cycleCount[1];
            }
        }
    }
    if ( (int)cycleCount[0] <= 2
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                    835,
                    1,
                    "%s",
                    "cycleCount[0] > 2") )
    {
        __debugbreak();
    }
    winding = 0;
    if ( optionalOutWinding )
    {
        winding = optionalOutWinding;
        if ( optionalOutWindingCount < (int)cycleCount[0] )
        {
            if ( name )
                v9 = name;
            else
                v9 = "";
            Com_PrintError(
                1,
                "Brush face has too many edges. Approx pos(%7.2f,%7.2f,%7.2f). Map file %s.\n",
                InPts->xyz[0],
                InPts->xyz[1],
                InPts->xyz[2],
                v9);
            return 0;
        }
    }
    else if ( !Assert_MyHandler(
                             "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                             854,
                             0,
                             "Not supported") )
    {
        __debugbreak();
    }
    if ( !winding
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                    858,
                    0,
                    "%s",
                    "winding") )
    {
        __debugbreak();
    }
    rv = PlaneInCommonExcluding(cycle[0][0], cycle[0][cycleCount[0] - 1], basePlaneIndex, winding->sides);
    if ( !rv
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp", 860, 1, "%s", "rv") )
    {
        __debugbreak();
    }
    v0[0] = cycle[0][0]->xyz[0];
    v0[1] = cycle[0][0]->xyz[1];
    v0[2] = cycle[0][0]->xyz[2];
    for ( winding->numsides = 1; winding->numsides < (int)cycleCount[0]; ++winding->numsides )
    {
        winding->sides[winding->numsides] = ThirdPlane(
                                                                                    cycle[0][winding->numsides - 1],
                                                                                    basePlaneIndex,
                                                                                    *(&winding->numsides + winding->numsides));
        v10 = &v0[3 * winding->numsides];
        v11 = cycle[0][winding->numsides];
        *v10 = v11->xyz[0];
        v10[1] = v11->xyz[1];
        v10[2] = v11->xyz[2];
    }
    if ( winding->sides[0] != ThirdPlane(
                                                            cycle[0][cycleCount[0] - 1],
                                                            basePlaneIndex,
                                                            *(&winding->numsides + winding->numsides))
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                    869,
                    1,
                    "%s",
                    "winding->sides[0] == ThirdPlane( cycle[0][cycleCount[0]-1], basePlaneIndex, winding->sides[winding->numsides-1] )") )
    {
        __debugbreak();
    }
    if ( RepresentativeTriangleFromWinding((const float (*)[3])v0, winding->numsides, sideNormal, &i0, &i1, &i2) < 0.001 )
        return 0;
    PlaneFromPoints(plane, &v0[3 * i0], &v0[3 * i1], &v0[3 * i2]);
    if ( (float)((float)((float)(plane[0] * *sideNormal) + (float)(plane[1] * sideNormal[1]))
                         + (float)(plane[2] * sideNormal[2])) < 0.0 )
        ReverseAdjacencyWinding(winding);
    return winding;
}

void __cdecl ReverseAdjacencyWinding(adjacencyWinding_t *w)
{
    int swapTemp; // [esp+0h] [ebp-Ch]
    int *start; // [esp+4h] [ebp-8h]
    int *end; // [esp+8h] [ebp-4h]

    start = w->sides;
    for ( end = &w->numsides + w->numsides; start < end; --end )
    {
        swapTemp = *start;
        *start = *end;
        *end = swapTemp;
        ++start;
    }
}

double __cdecl RepresentativeTriangleFromWinding(
                const float (*xyz)[3],
                int pointCount,
                const float *normal,
                int *i0,
                int *i1,
                int *i2)
{
float v7; // [esp+0h] [ebp-40h]
    float v8; // [esp+4h] [ebp-3Ch]
    int j; // [esp+8h] [ebp-38h]
    float areaBest; // [esp+Ch] [ebp-34h]
    float vb[3]; // [esp+14h] [ebp-2Ch] BYREF
    int k; // [esp+20h] [ebp-20h]
    float vc[3]; // [esp+24h] [ebp-1Ch] BYREF
    float va[3]; // [esp+30h] [ebp-10h] BYREF
    int i; // [esp+3Ch] [ebp-4h]

    *i0 = 0;
    *i1 = 1;
    *i2 = 2;
    areaBest = 0.0;
    for (k = 2; k < pointCount; ++k)
    {
        for (j = 1; j < k; ++j)
        {
            Vec3Sub(&(*xyz)[3 * k], &(*xyz)[3 * j], vb);
            for (i = 0; i < j; ++i)
            {
                Vec3Sub(&(*xyz)[3 * i], &(*xyz)[3 * j], va);
                Vec3Cross(va, vb, vc);
                v8 = Vec3Dot(vc, normal);
                v7 = I_fabs(v8);
                if (areaBest < v7)
                {
                    areaBest = v7;
                    *i0 = i;
                    *i1 = j;
                    *i2 = k;
                }
            }
        }
    }
    return areaBest;
}

int __cdecl GetPointListAllowDupes(
                int planeIndex,
                const SimplePlaneIntersection *pts,
                int ptCount,
                const SimplePlaneIntersection **xyz,
                int xyzLimit)
{
    int xyzCount; // [esp+0h] [ebp-8h]
    int ptIndex; // [esp+4h] [ebp-4h]

    xyzCount = 0;
    for ( ptIndex = 0; ptIndex < ptCount; ++ptIndex )
    {
        if ( planeIndex == pts[ptIndex].planeIndex[0]
            || planeIndex == pts[ptIndex].planeIndex[1]
            || planeIndex == pts[ptIndex].planeIndex[2] )
        {
            if ( xyzCount == xyzLimit )
                return 0;
            xyz[xyzCount++] = &pts[ptIndex];
        }
    }
    return xyzCount;
}

bool __cdecl IsPtFormedByThisPlane(int plane, const SimplePlaneIntersection *pt)
{
    if ( pt->planeIndex[0] == plane )
        return 1;
    if ( pt->planeIndex[1] == plane )
        return 1;
    return pt->planeIndex[2] == plane;
}

char __cdecl PlaneInCommonExcluding(
                const SimplePlaneIntersection *pt1,
                const SimplePlaneIntersection *pt2,
                int excludePlane,
                int *result)
{
    int j; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    for ( i = 0; i < 3; ++i )
    {
        for ( j = 0; j < 3; ++j )
        {
            if ( pt1->planeIndex[i] == pt2->planeIndex[j] && pt1->planeIndex[i] != excludePlane )
            {
                *result = pt1->planeIndex[i];
                return 1;
            }
        }
    }
    return 0;
}

int __cdecl SecondPlane(const SimplePlaneIntersection *point, int plane)
{
    int planeIndex; // [esp+0h] [ebp-4h]

    for ( planeIndex = 0; planeIndex < 3; ++planeIndex )
    {
        if ( point->planeIndex[planeIndex] != plane )
            return point->planeIndex[planeIndex];
    }
    if ( !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                    198,
                    0,
                    "all planes identical") )
        __debugbreak();
    return -1;
}

int __cdecl ThirdPlane(const SimplePlaneIntersection *point, int plane1, int plane2)
{
    int planeIndex; // [esp+0h] [ebp-4h]

    for ( planeIndex = 0; planeIndex < 3; ++planeIndex )
    {
        if ( point->planeIndex[planeIndex] != plane1 && point->planeIndex[planeIndex] != plane2 )
            return point->planeIndex[planeIndex];
    }
    if ( !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                    211,
                    0,
                    "no third plane") )
        __debugbreak();
    return -1;
}

const SimplePlaneIntersection *__cdecl RemoveNextPointFormedByThisPlane(
                int planeIndex,
                const SimplePlaneIntersection **begin,
                const SimplePlaneIntersection **end)
{
    const SimplePlaneIntersection *returnVal; // [esp+0h] [ebp-4h]
    const SimplePlaneIntersection **begina; // [esp+10h] [ebp+Ch]

    begina = NextPointFormedByThisPlane(planeIndex, begin, end);
    if ( begina == end )
        return 0;
    returnVal = *begina;
    memmove((unsigned __int8 *)begina, (unsigned __int8 *)begina + 4, 4 * (end - (begina + 1)));
    return returnVal;
}

const SimplePlaneIntersection **__cdecl NextPointFormedByThisPlane(
                int planeIndex,
                const SimplePlaneIntersection **begin,
                const SimplePlaneIntersection **end)
{
    while ( begin != end && !IsPtFormedByThisPlane(planeIndex, *begin) )
        ++begin;
    return begin;
}

double __cdecl CyclePerimiter(const SimplePlaneIntersection **pts, int ptsCount)
{
    float perimiter; // [esp+20h] [ebp-8h]
    int ptsIndex; // [esp+24h] [ebp-4h]

    if ( ptsCount <= 2
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                    395,
                    0,
                    "%s",
                    "ptsCount > 2") )
    {
        __debugbreak();
    }
    perimiter = Vec3Distance((*pts)->xyz, pts[ptsCount - 1]->xyz);
    for ( ptsIndex = 1; ptsIndex < ptsCount; ++ptsIndex )
        perimiter = Vec3Distance(pts[ptsIndex]->xyz, pts[ptsIndex - 1]->xyz) + perimiter;
    return perimiter;
}

char __cdecl TestConvexWithoutNearPoints(const SimplePlaneIntersection **pts, unsigned int ptCount)
{
    float *v3; // [esp+18h] [ebp-3010h]
    const SimplePlaneIntersection *v4; // [esp+1Ch] [ebp-300Ch]
    float p1[3073]; // [esp+20h] [ebp-3008h] BYREF
    unsigned int i; // [esp+3024h] [ebp-4h]

    for ( i = 0; i < ptCount; ++i )
    {
        v3 = &p1[3 * i];
        v4 = pts[i];
        *v3 = v4->xyz[0];
        v3[1] = v4->xyz[1];
        v3[2] = v4->xyz[2];
    }
    i = 1;
    while ( i < ptCount )
    {
        if ( Vec3DistanceSq(&p1[3 * i], &p1[3 * i - 3]) >= 1.0 )
        {
            ++i;
        }
        else
        {
            memmove((unsigned __int8 *)&p1[3 * i], (unsigned __int8 *)&p1[3 * i + 3], 12 * (ptCount - i - 1));
            --ptCount;
        }
    }
    if ( Vec3DistanceSq(p1, &p1[3 * ptCount - 3]) < 1.0 )
        --ptCount;
    if ( ptCount >= 3 )
        return IsConvex((const float (*)[3])p1, ptCount);
    else
        return 0;
}

char __cdecl IsConvex(const float (*pts)[3], unsigned int ptCount)
{
    unsigned int ptIndex3; // [esp+38h] [ebp-40h]
    unsigned int ptIndex3a; // [esp+38h] [ebp-40h]
    float edge1[3]; // [esp+3Ch] [ebp-3Ch] BYREF
    float edge2[3]; // [esp+48h] [ebp-30h] BYREF
    float normal[3]; // [esp+54h] [ebp-24h] BYREF
    unsigned int ptIndex2; // [esp+60h] [ebp-18h]
    unsigned int ptIndex1; // [esp+64h] [ebp-14h]
    float normal2[3]; // [esp+68h] [ebp-10h] BYREF
    float normalMag; // [esp+74h] [ebp-4h]

    if ( ptCount <= 2
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                    415,
                    0,
                    "%s",
                    "ptCount > 2") )
    {
        __debugbreak();
    }
    ptIndex2 = ptCount - 1;
    ptIndex1 = ptCount - 2;
    for ( ptIndex3 = 0; ptIndex3 < ptCount; ++ptIndex3 )
    {
        edge1[0] = (float)(*pts)[3 * ptIndex2] - (float)(*pts)[3 * ptIndex1];
        edge1[1] = (float)(*pts)[3 * ptIndex2 + 1] - (float)(*pts)[3 * ptIndex1 + 1];
        edge1[2] = (float)(*pts)[3 * ptIndex2 + 2] - (float)(*pts)[3 * ptIndex1 + 2];
        edge2[0] = (float)(*pts)[3 * ptIndex3] - (float)(*pts)[3 * ptIndex2];
        edge2[1] = (float)(*pts)[3 * ptIndex3 + 1] - (float)(*pts)[3 * ptIndex2 + 1];
        edge2[2] = (float)(*pts)[3 * ptIndex3 + 2] - (float)(*pts)[3 * ptIndex2 + 2];
        if ( (float)((float)((float)(edge1[0] * edge1[0]) + (float)(edge1[1] * edge1[1])) + (float)(edge1[2] * edge1[2])) <= 0.0
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                        425,
                        0,
                        "%s",
                        "Vec3LengthSq( edge1 ) > 0") )
        {
            __debugbreak();
        }
        if ( (float)((float)((float)(edge2[0] * edge2[0]) + (float)(edge2[1] * edge2[1])) + (float)(edge2[2] * edge2[2])) <= 0.0
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                        426,
                        0,
                        "%s",
                        "Vec3LengthSq( edge2 ) > 0") )
        {
            __debugbreak();
        }
        Vec3Cross(edge1, edge2, normal);
        normalMag = Vec3Normalize(normal);
        if ( normalMag >= 0.01 )
            break;
        if ( (float)((float)((float)(edge1[0] * edge2[0]) + (float)(edge1[1] * edge2[1])) + (float)(edge1[2] * edge2[2])) < 0.0 )
            return 0;
        ptIndex1 = ptIndex2;
        ptIndex2 = ptIndex3;
    }
    ptIndex1 = ptIndex2;
    ptIndex2 = ptIndex3;
    for ( ptIndex3a = ptIndex3 + 1; ptIndex3a < ptCount; ++ptIndex3a )
    {
        edge1[0] = (float)(*pts)[3 * ptIndex2] - (float)(*pts)[3 * ptIndex1];
        edge1[1] = (float)(*pts)[3 * ptIndex2 + 1] - (float)(*pts)[3 * ptIndex1 + 1];
        edge1[2] = (float)(*pts)[3 * ptIndex2 + 2] - (float)(*pts)[3 * ptIndex1 + 2];
        edge2[0] = (float)(*pts)[3 * ptIndex3a] - (float)(*pts)[3 * ptIndex2];
        edge2[1] = (float)(*pts)[3 * ptIndex3a + 1] - (float)(*pts)[3 * ptIndex2 + 1];
        edge2[2] = (float)(*pts)[3 * ptIndex3a + 2] - (float)(*pts)[3 * ptIndex2 + 2];
        if ( (float)((float)((float)(edge1[0] * edge1[0]) + (float)(edge1[1] * edge1[1])) + (float)(edge1[2] * edge1[2])) <= 0.0
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                        449,
                        0,
                        "%s",
                        "Vec3LengthSq( edge1 ) > 0") )
        {
            __debugbreak();
        }
        if ( (float)((float)((float)(edge2[0] * edge2[0]) + (float)(edge2[1] * edge2[1])) + (float)(edge2[2] * edge2[2])) <= 0.0
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                        450,
                        0,
                        "%s",
                        "Vec3LengthSq( edge2 ) > 0") )
        {
            __debugbreak();
        }
        Vec3Cross(edge1, edge2, normal2);
        normalMag = Vec3Normalize(normal2);
        if ( normalMag >= 0.01 )
        {
            if ( phys_dot((const phys_vec3 *)normal, (const phys_vec3 *)normal2) < 0.5 )
                return 0;
        }
        else if ( (float)((float)((float)(edge1[0] * edge2[0]) + (float)(edge1[1] * edge2[1])) + (float)(edge1[2] * edge2[2])) < 0.0 )
        {
            return 0;
        }
        ptIndex1 = ptIndex2;
        ptIndex2 = ptIndex3a;
    }
    return 1;
}

bool __cdecl CycleLess(
                bool isConvex1,
                bool isConvex2,
                float perimiter1,
                float perimiter2,
                int nodeCount1,
                int nodeCount2)
{
    if ( isConvex1 )
    {
        if ( !isConvex2 )
            return 0;
    }
    else if ( isConvex2 )
    {
        return 1;
    }
    return (float)(perimiter2 - 1.0) > perimiter1 || (float)(perimiter1 - 1.0) <= perimiter2 && nodeCount1 > nodeCount2;
}

int __cdecl ReduceToACycle(int basePlane, SimplePlaneIntersection **pts, int ptsCount)
{
#if 0
    int v4; // eax
    int v5; // eax
    int partition[1024]; // [esp+10h] [ebp-5040h] BYREF
    int v7; // [esp+1010h] [ebp-4040h]
    int listCount; // [esp+1014h] [ebp-403Ch]
    char CycleBFS; // [esp+1018h] [ebp-4038h]
    char v10; // [esp+1019h] [ebp-4037h]
    int v11; // [esp+101Ch] [ebp-4034h]
    SimplePlaneIntersection *resultCycle[1024]; // [esp+1020h] [ebp-4030h] BYREF
    SimplePlaneIntersection *v13[1024]; // [esp+2020h] [ebp-3030h] BYREF
    int list[1024]; // [esp+3020h] [ebp-2030h] BYREF
    int v15; // [esp+4020h] [ebp-1030h]
    char v16; // [esp+4024h] [ebp-102Ch]
    char v17; // [esp+4025h] [ebp-102Bh]
    int i; // [esp+4028h] [ebp-1028h]
    int v19; // [esp+402Ch] [ebp-1024h]
    int edgeCount; // [esp+4030h] [ebp-1020h]
    int k; // [esp+4034h] [ebp-101Ch]
    int resultCycleCount; // [esp+4038h] [ebp-1018h] BYREF
    unsigned int ptCount; // [esp+403Ch] [ebp-1014h] BYREF
    float perimiter1; // [esp+4040h] [ebp-1010h]
    float perimiter2; // [esp+4044h] [ebp-100Ch]
    SimplePlaneIntersection *result; // [esp+4048h] [ebp-1008h] BYREF
    SimplePlaneIntersection *end; // [esp+404Ch] [ebp-1004h]
    SimplePlaneIntersection *start; // [esp+4050h] [ebp-1000h]
    SimplePlaneIntersection *removePoint; // [esp+4054h] [ebp-FFCh]
    int j; // [esp+504Ch] [ebp-4h]
    int ptsCounta; // [esp+5060h] [ebp+10h]

    ptsCounta = RemovePtsWithPlanesThatOccurLessThanTwice((const SimplePlaneIntersection **)pts, ptsCount);
    if ( ptsCounta < 3 )
        return ptsCounta;
    listCount = 0;
    for ( i = 0; i < ptsCounta; ++i )
    {
        for ( j = 0; j < 3; ++j )
        {
            if ( basePlane != pts[i]->planeIndex[j] && !IntAlreadyInList(list, listCount, pts[i]->planeIndex[j]) )
            {
                if ( (unsigned int)listCount >= 0x400
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                                665,
                                0,
                                "planeCount doesn't index ARRAY_COUNT( planeList )\n\t%i not in [0, %i)",
                                listCount,
                                1024) )
                {
                    __debugbreak();
                }
                list[listCount++] = pts[i]->planeIndex[j];
            }
        }
    }
    if ( listCount <= 2
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                    672,
                    1,
                    "%s",
                    "planeCount > 2") )
    {
        __debugbreak();
    }
    for ( j = 0; j < listCount; ++j )
    {
        while ( 1 )
        {
            edgeCount = GetPtsFormedByPlane(
                                        list[j],
                                        (const SimplePlaneIntersection **)pts,
                                        ptsCounta,
                                        (const SimplePlaneIntersection **)&result,
                                        1024);
            if ( edgeCount <= 2 )
                break;
            v15 = PartitionEdges(
                            basePlane,
                            list[j],
                            pts,
                            ptsCounta,
                            (const SimplePlaneIntersection **)&result,
                            edgeCount,
                            partition);
            v7 = 0;
            for ( k = 0; k < v15 && k < 2; ++k )
            {
                v11 = partition[k] - v7;
                if ( v11 <= 0
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                                686,
                                0,
                                "%s",
                                "partitionSize > 0") )
                {
                    __debugbreak();
                }
                if ( v11 == 1 )
                {
                    ptsCounta = Remove((const SimplePlaneIntersection **)pts, ptsCounta, *(&result + v7));
                    break;
                }
                if ( v11 > 2 )
                {
                    v19 = ChooseEdgeToRemove(basePlane, list[j], pts, ptsCounta, (const SimplePlaneIntersection **)&result + v7);
                    ptsCounta = Remove((const SimplePlaneIntersection **)pts, ptsCounta, *(&result + v19 + v7));
                    break;
                }
                v7 = partition[k];
            }
            if ( k == 2 )
            {
                if ( partition[0] != 2
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                                707,
                                0,
                                "%s",
                                "partitions[0] == 2") )
                {
                    __debugbreak();
                }
                if ( partition[1] != 4
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                                708,
                                0,
                                "%s",
                                "partitions[1] == 4") )
                {
                    __debugbreak();
                }
                CycleBFS = FindCycleBFS(
                                         basePlane,
                                         pts,
                                         ptsCounta,
                                         result,
                                         end,
                                         list[j],
                                         (const SimplePlaneIntersection **)resultCycle,
                                         &resultCycleCount);
                v10 = FindCycleBFS(
                                basePlane,
                                pts,
                                ptsCounta,
                                start,
                                removePoint,
                                list[j],
                                (const SimplePlaneIntersection **)v13,
                                (int *)&ptCount);
                if ( (!CycleBFS || !v10)
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                                712,
                                0,
                                "%s",
                                "isCycle[0] && isCycle[1]") )
                {
                    __debugbreak();
                }
                perimiter1 = CyclePerimiter((const SimplePlaneIntersection **)resultCycle, resultCycleCount);
                perimiter2 = CyclePerimiter((const SimplePlaneIntersection **)v13, ptCount);
                v16 = TestConvexWithoutNearPoints((const SimplePlaneIntersection **)resultCycle, resultCycleCount);
                v17 = TestConvexWithoutNearPoints((const SimplePlaneIntersection **)v13, ptCount);
                if ( CycleLess(v16, v17, perimiter1, perimiter2, resultCycleCount, ptCount) )
                {
                    v4 = Remove((const SimplePlaneIntersection **)pts, ptsCounta, result);
                    ptsCounta = Remove((const SimplePlaneIntersection **)pts, v4, end);
                }
                else
                {
                    v5 = Remove((const SimplePlaneIntersection **)pts, ptsCounta, start);
                    ptsCounta = Remove((const SimplePlaneIntersection **)pts, v5, removePoint);
                }
            }
            if ( ptsCounta < 3 )
                return ptsCounta;
        }
    }
    return ptsCounta;
#else
int partition[1024];
    int planeList[1024];
    int planeCount = 0;

    SimplePlaneIntersection *edges[1024];
    SimplePlaneIntersection *cycleA[1024];
    SimplePlaneIntersection *cycleB[1024];

    int resultCycleCount;
    unsigned int ptCount;

    ptsCount = RemovePtsWithPlanesThatOccurLessThanTwice(
        (const SimplePlaneIntersection **)pts, ptsCount);

    if (ptsCount < 3)
        return ptsCount;

    //
    // Build list of planes connected to basePlane
    //
    for (int i = 0; i < ptsCount; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            int plane = pts[i]->planeIndex[j];

            if (plane == basePlane)
                continue;

            if (!IntAlreadyInList(planeList, planeCount, plane))
            {
                iassert(planeCount < 1024);
                planeList[planeCount++] = plane;
            }
        }
    }

    iassert(planeCount > 2);

    //
    // Process each connecting plane
    //
    for (int j = 0; j < planeCount; j++)
    {
        while (1)
        {
            int edgeCount = GetPtsFormedByPlane(
                planeList[j],
                (const SimplePlaneIntersection **)pts,
                ptsCount,
                (const SimplePlaneIntersection **)edges,
                1024);

            if (edgeCount <= 2)
                break;

            int partitionCount = PartitionEdges(
                basePlane,
                planeList[j],
                (SimplePlaneIntersection **)pts,
                ptsCount,
                (const SimplePlaneIntersection **)edges,
                edgeCount,
                partition);

            int offset = 0;

            for (int k = 0; k < partitionCount && k < 2; k++)
            {
                int partSize = partition[k] - offset;

                iassert(partSize > 0);

                if (partSize == 1)
                {
                    ptsCount = Remove(
                        (const SimplePlaneIntersection **)pts,
                        ptsCount,
                        edges[offset]);
                    break;
                }

                if (partSize > 2)
                {
                    int removeIndex = ChooseEdgeToRemove(
                        basePlane,
                        planeList[j],
                        (SimplePlaneIntersection **)pts,
                        ptsCount,
                        (const SimplePlaneIntersection **)&edges[offset]);

                    ptsCount = Remove(
                        (const SimplePlaneIntersection **)pts,
                        ptsCount,
                        edges[offset + removeIndex]);

                    break;
                }

                offset = partition[k];
            }

            //
            // Special case: two partitions of size 2
            //
            if (partitionCount == 2)
            {
                iassert(partition[0] == 2);
                iassert(partition[1] == 4);

                bool cycle1 = FindCycleBFS(
                    basePlane,
                    pts,
                    ptsCount,
                    edges[0],
                    edges[1],
                    planeList[j],
                    (const SimplePlaneIntersection **)cycleA,
                    &resultCycleCount);

                bool cycle2 = FindCycleBFS(
                    basePlane,
                    pts,
                    ptsCount,
                    edges[2],
                    edges[3],
                    planeList[j],
                    (const SimplePlaneIntersection **)cycleB,
                    (int *)&ptCount);

                iassert(cycle1 && cycle2);

                float perim1 = CyclePerimiter(
                    (const SimplePlaneIntersection **)cycleA,
                    resultCycleCount);

                float perim2 = CyclePerimiter(
                    (const SimplePlaneIntersection **)cycleB,
                    ptCount);

                bool convex1 = TestConvexWithoutNearPoints(
                    (const SimplePlaneIntersection **)cycleA,
                    resultCycleCount);

                bool convex2 = TestConvexWithoutNearPoints(
                    (const SimplePlaneIntersection **)cycleB,
                    ptCount);

                if (CycleLess(convex1, convex2, perim1, perim2, resultCycleCount, ptCount))
                {
                    ptsCount = Remove((const SimplePlaneIntersection **)pts, ptsCount, edges[0]);
                    ptsCount = Remove((const SimplePlaneIntersection **)pts, ptsCount, edges[1]);
                }
                else
                {
                    ptsCount = Remove((const SimplePlaneIntersection **)pts, ptsCount, edges[2]);
                    ptsCount = Remove((const SimplePlaneIntersection **)pts, ptsCount, edges[3]);
                }
            }

            if (ptsCount < 3)
                return ptsCount;
        }
    }

    return ptsCount;

#endif
}

char __cdecl IntAlreadyInList(const int *list, int listCount, int value)
{
    int listIndex; // [esp+0h] [ebp-4h]

    for ( listIndex = 0; listIndex < listCount; ++listIndex )
    {
        if ( list[listIndex] == value )
            return 1;
    }
    return 0;
}

char __cdecl FindCycleBFS(
                int basePlane,
                SimplePlaneIntersection **pts,
                int ptsCount,
                const SimplePlaneIntersection *start,
                const SimplePlaneIntersection *end,
                int connectingPlane,
                const SimplePlaneIntersection **resultCycle,
                int *resultCycleCount)
{
    struct CycleNode
    {
        const SimplePlaneIntersection *point;
        int plane;
        int depth;
        CycleNode *parent;
    };

    CycleNode *v9; // [esp+0h] [ebp-4028h]
    const SimplePlaneIntersection **enda; // [esp+4h] [ebp-4024h]
    CycleNode queue[1024];
    int v15; // [esp+400Ch] [ebp-1Ch]
    int v16; // [esp+4010h] [ebp-18h]
    const SimplePlaneIntersection **i; // [esp+4014h] [ebp-14h]
    signed int j; // [esp+4018h] [ebp-10h]
    int v19; // [esp+401Ch] [ebp-Ch]
    signed int v20; // [esp+4020h] [ebp-8h]
    signed int v21; // [esp+4024h] [ebp-4h]

    if ( !IsPtFormedByThisPlane(connectingPlane, start)
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                    262,
                    0,
                    "%s",
                    "IsPtFormedByThisPlane( connectingPlane, start )") )
    {
        __debugbreak();
    }
    if ( !IsPtFormedByThisPlane(connectingPlane, end)
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                    263,
                    0,
                    "%s",
                    "IsPtFormedByThisPlane( connectingPlane, end )") )
    {
        __debugbreak();
    }
    queue[0].point = start;
    queue[0].plane = ThirdPlane(start, basePlane, connectingPlane);
    queue[0].depth = 1;
    queue[0].parent = 0;
    v20 = 0;
    v21 = 1;
    v19 = ThirdPlane(end, basePlane, connectingPlane);
LABEL_8:
    if ( v21 <= v20 )
    {
        *resultCycleCount = 0;
        return 0;
    }
    else
    {
        enda = (const SimplePlaneIntersection **)&pts[ptsCount];
        for ( i = NextPointFormedByThisPlane(queue[v20].plane, (const SimplePlaneIntersection **)pts, enda);
                    ;
                    i = NextPointFormedByThisPlane(queue[v20].plane, i + 1, enda) )
        {
            if ( i == enda )
            {
                ++v20;
                goto LABEL_8;
            }
            v15 = ThirdPlane(*i, basePlane, queue[v20].plane);
            if ( v15 != connectingPlane )
            {
                for ( j = 0; j < v21 && queue[j].plane != v15; ++j )
                    ;
                if ( j >= v21 )
                {
                    if ( (unsigned int)v21 >= 0x400
                        && !Assert_MyHandler(
                                    "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                                    295,
                                    0,
                                    "queueHead doesn't index ARRAY_COUNT( queue )\n\t%i not in [0, %i)",
                                    v21,
                                    1024) )
                    {
                        __debugbreak();
                    }
                    queue[v21].point = *i;
                    queue[v21].plane = v15;
                    queue[v21].depth = queue[v20].depth + 1;
                    queue[v21++].parent = &queue[v20];
                    if ( v15 == v19 )
                        break;
                }
            }
        }
        v9 = &queue[v21 - 1];
        if ( v9->plane != v19
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                        314,
                        1,
                        "%s",
                        "node->plane == goalPlane") )
        {
            __debugbreak();
        }
        *resultCycleCount = v9->depth + 1;
        v16 = v9->depth;
        while ( v9 )
        {
            resultCycle[v16] = v9->point;
            --v16;
            v9 = v9->parent;
        }
        if ( v16 )
        {
            if ( !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                            322,
                            1,
                            "%s",
                            "cycleIndex == 0") )
                __debugbreak();
        }
        *resultCycle = end;
        return 1;
    }
}

int __cdecl RemovePtsWithPlanesThatOccurLessThanTwice(const SimplePlaneIntersection **pts, int ptsCount)
{
    int ptsIndex; // [esp+0h] [ebp-4h]

    ptsIndex = 0;
    while ( ptsIndex < ptsCount )
    {
        if ( NumberOfOccurancesOfPlane(pts[ptsIndex]->planeIndex[0], pts, ptsCount) >= 2
            && NumberOfOccurancesOfPlane(pts[ptsIndex]->planeIndex[1], pts, ptsCount) >= 2
            && NumberOfOccurancesOfPlane(pts[ptsIndex]->planeIndex[2], pts, ptsCount) >= 2 )
        {
            ++ptsIndex;
        }
        else
        {
            memmove((unsigned __int8 *)&pts[ptsIndex], (unsigned __int8 *)&pts[ptsIndex + 1], 4 * (ptsCount - ptsIndex) - 4);
            --ptsCount;
            ptsIndex = 0;
        }
    }
    return ptsIndex;
}

int __cdecl NumberOfOccurancesOfPlane(int planeIndex, const SimplePlaneIntersection **pts, int ptCount)
{
    const SimplePlaneIntersection **end; // [esp+0h] [ebp-8h]
    int occurances; // [esp+4h] [ebp-4h]
    const SimplePlaneIntersection **ptsa; // [esp+14h] [ebp+Ch]

    if ( !pts
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                    229,
                    0,
                    "%s",
                    "pts") )
    {
        __debugbreak();
    }
    if ( !ptCount )
        return 0;
    end = &pts[ptCount];
    occurances = 0;
    for ( ptsa = NextPointFormedByThisPlane(planeIndex, pts, end);
                ptsa != end;
                ptsa = NextPointFormedByThisPlane(planeIndex, ptsa + 1, end) )
    {
        ++occurances;
    }
    return occurances;
}

int __cdecl GetPtsFormedByPlane(
                int planeIndex,
                const SimplePlaneIntersection **pts,
                int ptCount,
                const SimplePlaneIntersection **result,
                int maxResults)
{
    const SimplePlaneIntersection **end; // [esp+0h] [ebp-8h]
    int occurances; // [esp+4h] [ebp-4h]
    const SimplePlaneIntersection **ptsa; // [esp+14h] [ebp+Ch]

    if ( !pts
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                    368,
                    0,
                    "%s",
                    "pts") )
    {
        __debugbreak();
    }
    if ( !result
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                    369,
                    0,
                    "%s",
                    "result") )
    {
        __debugbreak();
    }
    if ( !ptCount )
        return 0;
    end = &pts[ptCount];
    occurances = 0;
    for ( ptsa = NextPointFormedByThisPlane(planeIndex, pts, end);
                ptsa != end;
                ptsa = NextPointFormedByThisPlane(planeIndex, ptsa + 1, end) )
    {
        if ( occurances >= maxResults
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                        380,
                        0,
                        "%s",
                        "occurances < maxResults") )
        {
            __debugbreak();
        }
        result[occurances++] = *ptsa;
    }
    return occurances;
}

int __cdecl ChooseEdgeToRemove(
                int basePlane,
                int connectingPlane,
                SimplePlaneIntersection **pts,
                int ptsCount,
                const SimplePlaneIntersection **edges)
{
    char CycleBFS; // [esp+10h] [ebp-302Ch]
    char v7; // [esp+11h] [ebp-302Bh]
    char v8; // [esp+12h] [ebp-302Ah]
    int v9; // [esp+14h] [ebp-3028h]
    SimplePlaneIntersection *resultCycle[1024]; // [esp+1Ch] [ebp-3020h] BYREF
    SimplePlaneIntersection *v11[1024]; // [esp+101Ch] [ebp-2020h] BYREF
    SimplePlaneIntersection *v12[1025]; // [esp+201Ch] [ebp-1020h] BYREF
    char convex[2]; // [esp+3020h] [ebp-1Ch]
    char v15; // [esp+3022h] [ebp-1Ah]
    int cycleCounts[2]; // [esp+3024h] [ebp-18h] BYREF
    unsigned int v18; // [esp+302Ch] [ebp-10h] BYREF
    float perimeters[2]; // [esp+3030h] [ebp-Ch]
    float v21; // [esp+3038h] [ebp-4h]

    CycleBFS = FindCycleBFS(
                             basePlane,
                             pts,
                             ptsCount,
                             *edges,
                             edges[1],
                             connectingPlane,
                             (const SimplePlaneIntersection **)resultCycle,
                             &cycleCounts[0]);
    v7 = FindCycleBFS(
                 basePlane,
                 pts,
                 ptsCount,
                 *edges,
                 edges[2],
                 connectingPlane,
                 (const SimplePlaneIntersection **)v11,
                 (int *)&cycleCounts[1]);
    v8 = FindCycleBFS(
                 basePlane,
                 pts,
                 ptsCount,
                 edges[1],
                 edges[2],
                 connectingPlane,
                 (const SimplePlaneIntersection **)v12,
                 (int *)&v18);
    if ( (!CycleBFS || !v7 || !v8)
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                    537,
                    0,
                    "%s",
                    "isCycle[0] && isCycle[1] && isCycle[2]") )
    {
        __debugbreak();
    }
    perimeters[0] = CyclePerimiter((const SimplePlaneIntersection **)resultCycle, cycleCounts[0]);
    perimeters[1] = CyclePerimiter((const SimplePlaneIntersection **)v11, cycleCounts[1]);
    v21 = CyclePerimiter((const SimplePlaneIntersection **)v12, v18);
    convex[0] = TestConvexWithoutNearPoints((const SimplePlaneIntersection **)resultCycle, cycleCounts[0]);
    convex[1] = TestConvexWithoutNearPoints((const SimplePlaneIntersection **)v11, cycleCounts[1]);
    v15 = TestConvexWithoutNearPoints((const SimplePlaneIntersection **)v12, v18);
    v9 = CycleLess(convex[0], convex[1], perimeters[0], perimeters[1], cycleCounts[0], cycleCounts[1]);
    if ( CycleLess(convex[v9], v15, perimeters[v9], v21, cycleCounts[v9], v18) )
        v9 = 2;
    return 2 - v9;
}

int __cdecl PartitionEdges(
                int basePlane,
                int connectingPlane,
                SimplePlaneIntersection **pts,
                int ptsCount,
                const SimplePlaneIntersection **edges,
                int edgeCount,
                int *partition)
{
    const SimplePlaneIntersection *v8; // [esp+0h] [ebp-1020h]
    SimplePlaneIntersection *resultCycle[1024]; // [esp+8h] [ebp-1018h] BYREF
    int v10; // [esp+100Ch] [ebp-14h]
    int i; // [esp+1010h] [ebp-10h]
    int resultCycleCount; // [esp+1014h] [ebp-Ch] BYREF
    int j; // [esp+1018h] [ebp-8h]
    int v14; // [esp+101Ch] [ebp-4h]

    v10 = 1;
    *partition = 1;
    while ( partition[v10 - 1] < edgeCount )
    {
        v14 = partition[v10 - 1];
        for ( i = 0; i < v10; ++i )
        {
            if ( FindCycleBFS(
                         basePlane,
                         pts,
                         ptsCount,
                         edges[partition[i] - 1],
                         edges[v14],
                         connectingPlane,
                         (const SimplePlaneIntersection **)resultCycle,
                         &resultCycleCount) )
            {
                if ( i < v10 - 1 )
                {
                    v8 = edges[v14];
                    memmove(
                        (unsigned __int8 *)&edges[partition[i] + 1],
                        (unsigned __int8 *)&edges[partition[i]],
                        4 * (v14 - partition[i]));
                    edges[partition[i]] = v8;
                }
                for ( j = i; j < v10; ++j )
                    ++partition[j];
                break;
            }
        }
        if ( i == v10 )
            partition[v10++] = v14 + 1;
    }
    return v10;
}

int __cdecl Remove(const SimplePlaneIntersection **pts, int ptsCount, const SimplePlaneIntersection *removePoint)
{
    int ptsIndex; // [esp+0h] [ebp-4h]
    int ptsCounta; // [esp+10h] [ebp+Ch]

    for ( ptsIndex = 0; ptsIndex < ptsCount && pts[ptsIndex] != removePoint; ++ptsIndex )
        ;
    if ( ptsIndex == ptsCount )
        return ptsCount;
    memmove((unsigned __int8 *)&pts[ptsIndex], (unsigned __int8 *)&pts[ptsIndex + 1], 4 * (ptsCount - ptsIndex) - 4);
    ptsCounta = ptsCount - 1;
    if ( ptsCounta >= 3 )
        return RemovePtsWithPlanesThatOccurLessThanTwice(pts, ptsCounta);
    else
        return ptsCounta;
}

int __cdecl NumberOfUniquePoints(const SimplePlaneIntersection **pts, int ptsCount)
{
    unsigned int v3[1025]; // [esp+10h] [ebp-1010h]
    int v4; // [esp+1014h] [ebp-Ch]
    int j; // [esp+1018h] [ebp-8h]
    int i; // [esp+101Ch] [ebp-4h]

    if ( !pts
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                    749,
                    0,
                    "%s",
                    "pts") )
    {
        __debugbreak();
    }
    if ( ptsCount <= 2
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../../common/brush_edges.cpp",
                    750,
                    0,
                    "%s",
                    "ptsCount > 2") )
    {
        __debugbreak();
    }
    v4 = 0;
    for ( i = 0; i < ptsCount; ++i )
    {
        for ( j = 0; j < v4 && !VecNCompareCustomEpsilon(pts[i]->xyz, (const float *)v3[j], 0.0099999998, 3); ++j )
            ;
        if ( j == v4 )
            v3[v4++] = (unsigned int)pts[i];
    }
    return v4;
}

