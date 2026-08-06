#include "g_vehicle_path.h"
#include <clientscript/cscr_stringlist.h>
#include <cfloat>
#include <cstring>
#include "g_debug.h"
#include <cgame/cg_drawtools.h>
#include <universal/com_math_anglevectors.h>
#include "g_scr_helicopter.h"
#include <clientscript/cscr_vm.h>
#include <game_mp/g_spawn_mp.h>
#include "g_load_utils.h"
#include "sentient_fields.h"

static const float s_invalidAngles[3] = { 3.1415927, 3.1415927, 3.1415927 };

vn_field_t vn_fields[12] =
{
  { "targetname", 0, { 2 }, F_STRING, false },
  { "target", 2, { 2 }, F_STRING, false },
  { "target2", 4, { 2 }, F_STRING, false },
  { "script_linkname", 6, { 2 }, F_STRING, false },
  { "script_noteworthy", 8, { 2 }, F_STRING, false },
  { "origin", 24, { 12 }, F_VECTOR, true },
  { "angles", 36, { 12 }, F_VECTOR, false },
  { "speed", 16, { 4 }, F_FLOAT, false },
  { "radius", 52, { 4 }, F_FLOAT, false },
  { "lookahead", 20, { 4 }, F_FLOAT, false },
  { "spawnflags", 12, { 4 }, F_INT, false },
  { NULL, 0, { 0 }, F_INT, false }
};

int num_heli_height_lock_patches;
__int16 s_numVehicleNodeLinks;

vehicle_node_t s_nodes[2000];
__int16 s_numNodes;
vehicle_path_node_link_t s_node_links[2000];

bool pathsInitialized;
vehicle_custom_path_t gCustomPaths[10];

int s_newDebugLine = 1;
vehicle_node_t *g_radiant_selected_node;

float s_dir[3];
float s_end[3];
float s_start[3];


void __cdecl VP_ResetLinks()
{
    s_numVehicleNodeLinks = 0;
}

__int16 __cdecl VP_GetNextFreePathNode()
{
    if ( s_numVehicleNodeLinks >= 2000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\g_vehicle_path.cpp",
                    52,
                    0,
                    "%s",
                    "s_numVehicleNodeLinks < VEHICLE_MAX_NODE_LINKS") )
    {
        __debugbreak();
    }
    return ++s_numVehicleNodeLinks;
}

void __cdecl VP_ClearNode(vehicle_node_t *node)
{
    Scr_SetString(&node->name, 0, SCRIPTINSTANCE_SERVER);
    Scr_SetString(&node->target, 0, SCRIPTINSTANCE_SERVER);
    Scr_SetString(&node->target2, 0, SCRIPTINSTANCE_SERVER);
    Scr_SetString(&node->script_linkname, 0, SCRIPTINSTANCE_SERVER);
    Scr_SetString(&node->script_noteworthy, 0, SCRIPTINSTANCE_SERVER);
}

__int16 __cdecl VP_GetNodeIndex(unsigned __int16 name, float *origin)
{
    vehicle_node_t *node; // [esp+8h] [ebp-8h]
    __int16 i; // [esp+Ch] [ebp-4h]

    if ( !name )
        return -1;
    for ( i = 0; i < s_numNodes; ++i )
    {
        node = &s_nodes[i];
        if ( node->name == name )
        {
            if ( !origin )
                return i;
            if ( node->origin[0] == *origin && node->origin[1] == origin[1] && node->origin[2] == origin[2] )
                return i;
        }
    }
    return -1;
}

__int16 __cdecl VP_FindClosestPathNode(const float *pos)
{
    double v1; // st7
    vehicle_node_t *v2; // eax
    float closestDist2; // [esp+10h] [ebp-10h]
    __int16 closestNode; // [esp+14h] [ebp-Ch]
    float dist2; // [esp+18h] [ebp-8h]
    __int16 i; // [esp+1Ch] [ebp-4h]

    closestNode = -1;
    closestDist2 = FLT_MAX;
    for ( i = 0; i < s_numNodes; ++i )
    {
        v1 = Vec3DistanceSq(pos, s_nodes[i].origin);
        v2 = &s_nodes[i];
        if ( (v2->flags & 2) != 0 )
        {
            dist2 = v1 - v2->splineNode.length * v2->splineNode.length;
            if ( closestDist2 > dist2 )
            {
                closestDist2 = v1 - v2->splineNode.length * v2->splineNode.length;
                closestNode = i;
            }
        }
    }
    return closestNode;
}

vehicle_custom_path_t *__cdecl VP_GetFreeCustomPath()
{
    int j; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if ( !pathsInitialized )
    {
        pathsInitialized = 1;
        for ( i = 0; i < 10; ++i )
            gCustomPaths[i].inUse = 0;
    }
    for ( j = 0; j < 10; ++j )
    {
        if ( !gCustomPaths[j].inUse )
        {
            gCustomPaths[j].inUse = 1;
            return &gCustomPaths[j];
        }
    }
    return 0;
}

void __cdecl VP_FreeCustomPath(vehicle_custom_path_t *path)
{
    if ( !path
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\g_vehicle_path.cpp", 1409, 0, "%s", "path") )
    {
        __debugbreak();
    }
    path->inUse = 0;
}

int __cdecl VP_FindPath(float *start, float *end, vehicle_pathpos_t *vpp)
{
    double v4; // st7
    const char *v5; // eax
    double v6; // st7
    float *v7; // [esp+20h] [ebp-7EA4h]
    float *origin; // [esp+3Ch] [ebp-7E88h]
    __int16 v9; // [esp+60h] [ebp-7E64h]
    bool v10; // [esp+67h] [ebp-7E5Dh]
    float v11; // [esp+68h] [ebp-7E5Ch]
    __int16 j; // [esp+6Ch] [ebp-7E58h]
    __int16 v13; // [esp+70h] [ebp-7E54h]
    __int16 k; // [esp+7Ch] [ebp-7E48h]
    __int16 v15; // [esp+80h] [ebp-7E44h]
    __int16 v16; // [esp+84h] [ebp-7E40h]
    __int16 v17; // [esp+88h] [ebp-7E3Ch]
    vehicle_custom_path_t *path; // [esp+8Ch] [ebp-7E38h]
    char v19; // [esp+93h] [ebp-7E31h]
    int v20; // [esp+94h] [ebp-7E30h]
    __int16 v21; // [esp+98h] [ebp-7E2Ch]
    __int16 v22; // [esp+9Ch] [ebp-7E28h]
    int i; // [esp+A0h] [ebp-7E24h]
    __int16 nextIdx; // [esp+A4h] [ebp-7E20h]
    float v25; // [esp+A8h] [ebp-7E1Ch]
    __int16 v26; // [esp+ACh] [ebp-7E18h]
    __int16 prevIdx; // [esp+B0h] [ebp-7E14h]
    _WORD v28[128]; // [esp+B4h] [ebp-7E10h]
    __int16 v29; // [esp+1B8h] [ebp-7D0Ch]
    _WORD dst[16000]; // [esp+1BCh] [ebp-7D08h] BYREF
    __int16 v31; // [esp+7EBCh] [ebp-8h]
    __int16 ClosestPathNode; // [esp+7EC0h] [ebp-4h]

    ClosestPathNode = VP_FindClosestPathNode(start);
    v31 = VP_FindClosestPathNode(end);
    if ( v31 < 0 || ClosestPathNode < 0 )
        return 0;
    memset(dst, 0, sizeof(dst));
    v29 = 0;
    *(unsigned int *)&dst[8 * v31 + 2] = 0;
    v4 = Vec3Distance(s_nodes[v31].origin, s_nodes[ClosestPathNode].origin);
    *(float *)&dst[8 * v31 + 6] = v4;
    *(float *)&dst[8 * v31 + 4] = *(float *)&dst[8 * v31 + 6];
    dst[8 * v31 + 1] |= 1u;
    v28[v29++] = v31;
    while ( 1 )
    {
        if ( v29 <= 0 )
            return 0;
        v26 = v28[--v29];
        v25 = *(float *)&dst[8 * v26 + 4];
        for ( i = v29 - 1; i >= 0; --i )
        {
            v22 = v28[i];
            if ( v25 > *(float *)&dst[8 * v22 + 4] )
            {
                v25 = *(float *)&dst[8 * v22 + 4];
                v26 = v28[i];
                v21 = v28[v29];
                v28[v29] = v22;
                v28[i] = v21;
            }
        }
        dst[8 * v26 + 1] |= 2u;
        if ( v26 == ClosestPathNode )
            break;
        prevIdx = s_nodes[v26].splineNode.prevIdx;
        nextIdx = s_nodes[v26].splineNode.nextIdx;
        for ( j = 0; j < nextIdx; ++j )
        {
            v9 = s_node_links[(__int16)(j + prevIdx)].nextIdx;
            if ( (s_nodes[v9].flags & 0x80000) == 0 && (dst[8 * v9 + 1] & 2) == 0 )
            {
                v11 = *(float *)&dst[8 * v26 + 2] + s_node_links[j + prevIdx].length;
                if ( (dst[8 * v9 + 1] & 1) != 0 )
                {
                    v10 = *(float *)&dst[8 * v9 + 2] > v11;
                }
                else
                {
                    if ( v29 >= 128
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\g_vehicle_path.cpp",
                                    1574,
                                    0,
                                    "%s",
                                    "pathQueueCount < MAX_CUSTOM_PATH_LENGTH") )
                    {
                        __debugbreak();
                    }
                    dst[8 * v9 + 1] |= 1u;
                    v10 = 1;
                    v28[v29++] = v9;
                }
                if ( v10 )
                {
                    dst[8 * v9] = v26;
                    *(float *)&dst[8 * v9 + 2] = v11;
                    *(float *)&dst[8 * v9 + 6] = Vec3Distance(s_nodes[v9].origin, s_nodes[ClosestPathNode].origin);
                    *(float *)&dst[8 * v9 + 4] = *(float *)&dst[8 * v9 + 2] + *(float *)&dst[8 * v9 + 6];
                }
            }
        }
    }
    if ( vpp->customPath )
        path = vpp->customPath;
    else
        path = VP_GetFreeCustomPath();
    if ( !path )
        return 0;
    v17 = ClosestPathNode;
    v20 = 0;
    while ( v17 != v31 )
    {
        if ( v20 >= 128 )
        {
            v5 = va("Vehicle path length over %d nodes long\n", 128);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\g_vehicle_path.cpp",
                            1495,
                            0,
                            "%s\n\t%s",
                            "i < MAX_CUSTOM_PATH_LENGTH",
                            v5) )
                __debugbreak();
        }
        path->pathOrder[v20] = v17;
        v16 = s_nodes[v17].splineNode.prevIdx;
        v15 = s_nodes[v17].splineNode.nextIdx;
        v17 = dst[8 * v17];
        for ( k = 0; k < v15; ++k )
        {
            if ( s_node_links[(__int16)(k + v16)].nextIdx == v17 )
                path->pathLinkIdx[v20] = k + v16;
        }
        ++v20;
    }
    v19 = 1;
    if ( v20 )
    {
        v6 = Vec3DistanceSq(s_nodes[v31].origin, end);
        if ( s_nodes[v31].splineNode.length * s_nodes[v31].splineNode.length <= v6 )
        {
            origin = s_nodes[v31].origin;
            vpp->customGoalDir[0] = *end - *origin;
            vpp->customGoalDir[1] = end[1] - origin[1];
            vpp->customGoalDir[2] = end[2] - origin[2];
            vpp->customGoalLength = Vec3Normalize(vpp->customGoalDir);
            v13 = path->pathOrder[v20 + 127];
            if ( s_node_links[v13].nextIdx != v31
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\g_vehicle_path.cpp",
                            1529,
                            0,
                            "%s",
                            "s_node_links[linkIdx].nextIdx == startNode") )
            {
                __debugbreak();
            }
            if ( (float)((float)((float)(vpp->customGoalDir[0] * s_node_links[v13].dir[0])
                                                 + (float)(vpp->customGoalDir[1] * s_node_links[v13].dir[1]))
                                 + (float)(vpp->customGoalDir[2] * s_node_links[v13].dir[2])) < -0.30000001 )
                v19 = 0;
        }
        else
        {
            v19 = 0;
        }
        if ( v19 )
        {
            path->pathOrder[v20] = v31;
            path->pathOrder[v20 + 1] = -1;
        }
        else
        {
            if ( v20 > 0 )
                v31 = path->pathOrder[v20 - 1];
            path->pathOrder[v20] = -1;
        }
        G_VehSetUpPathPos(vpp, path->pathOrder[0]);
        vpp->nodeIdx = 0;
        vpp->customPath = path;
        v7 = s_nodes[v31].origin;
        vpp->customGoalDir[0] = *end - *v7;
        vpp->customGoalDir[1] = end[1] - v7[1];
        vpp->customGoalDir[2] = end[2] - v7[2];
        vpp->customGoalLength = Vec3Normalize(vpp->customGoalDir);
        return 1;
    }
    else
    {
        VP_FreeCustomPath(path);
        vpp->customPath = 0;
        return 0;
    }
}

void __cdecl VP_DrawCustomPath(vehicle_custom_path_t *customPath)
{
    __int16 nodeIdx; // [esp+0h] [ebp-14h]
    __int16 nextNodeIdx; // [esp+8h] [ebp-Ch]
    int i; // [esp+10h] [ebp-4h]

    if ( customPath )
    {
        for ( i = 1; i < 128; ++i )
        {
            nodeIdx = customPath->pathOrder[i - 1];
            if ( nodeIdx < 0 )
                break;
            nextNodeIdx = customPath->pathOrder[i];
            if ( nextNodeIdx < 0 )
                break;
            G_DebugLine(s_nodes[nodeIdx].origin, s_nodes[nextNodeIdx].origin, colorGreen, 1);
        }
    }
}

void __cdecl VP_DrawPath(const vehicle_pathpos_t *vpp)
{
    vehicle_node_t *node; // [esp+18h] [ebp-214h]
    float mins[3]; // [esp+1Ch] [ebp-210h] BYREF
    float maxs[3]; // [esp+28h] [ebp-204h] BYREF
    float k_boxColor2[4]; // [esp+34h] [ebp-1F8h] BYREF
    float k_boxColor1[4]; // [esp+44h] [ebp-1E8h] BYREF
    int arrowCount; // [esp+54h] [ebp-1D8h]
    int done; // [esp+58h] [ebp-1D4h]
    vehicle_pathpos_t nextVPP; // [esp+5Ch] [ebp-1D0h] BYREF
    vehicle_pathpos_t prevVPP; // [esp+13Ch] [ebp-F0h] BYREF
    __int16 loopNode; // [esp+21Ch] [ebp-10h]
    int looped; // [esp+220h] [ebp-Ch]
    int arrowTime; // [esp+224h] [ebp-8h]
    int count; // [esp+228h] [ebp-4h]

    arrowCount = 0;
    arrowTime = 1000;
    done = 0;
    count = 0;
    loopNode = -1;
    memcpy(&prevVPP, vpp, sizeof(prevVPP));
    memcpy(&nextVPP, vpp, sizeof(nextVPP));
    s_newDebugLine = 1;
    while ( !done && count <= 50000 )
    {
        ++count;
        if ( prevVPP.nodeIdx != vpp->nodeIdx )
            loopNode = vpp->nodeIdx;
        memcpy(&prevVPP, &nextVPP, sizeof(prevVPP));
        looped = G_VehUpdatePathPos(&nextVPP, loopNode);
        if ( nextVPP.endOfPath || looped )
            done = 1;
        VP_AddDebugLine(prevVPP.origin, nextVPP.origin, done);
        if ( arrowCount <= arrowTime )
        {
            arrowCount += 50;
        }
        else
        {
            VP_DebugArrow(nextVPP.origin, nextVPP.angles);
            arrowCount = 0;
        }
    }
    k_boxColor1[0] = 0.0f;
    k_boxColor1[1] = 1.0f;
    k_boxColor1[2] = 0.0f;
    k_boxColor1[3] = 1.0f;
    k_boxColor2[0] = 0.0f;
    k_boxColor2[1] = 0.0f;
    k_boxColor2[2] = 1.0f;
    k_boxColor2[3] = 1.0f;
    count = 0;
    for ( node = &s_nodes[vpp->nodeIdx];
                count < s_numNodes && vpp->nodeIdx != node->splineNode.nextIdx;
                node = &s_nodes[node->splineNode.nextIdx] )
    {
        ++count;
        mins[0] = -8.0f;
        mins[1] = -8.0f;
        mins[2] = -8.0f;
        maxs[0] = 8.0f;
        maxs[1] = 8.0f;
        maxs[2] = 8.0f;
        if ( node == &s_nodes[vpp->nodeIdx] || g_radiant_selected_node == node )
            CG_DebugBox(node->origin, mins, maxs, node->angles[1], k_boxColor1, 1, 0);
        else
            CG_DebugBox(node->origin, mins, maxs, node->angles[1], k_boxColor2, 1, 0);
        if ( node->splineNode.nextIdx < 0 || node->splineNode.nextIdx == vpp->nodeIdx )
            break;
    }
}

void __cdecl VP_AddDebugLine(const float *start, const float *end, int forceDraw)
{
    float k_lineColor[4]; // [esp+Ch] [ebp-1Ch] BYREF
    float dir[3]; // [esp+1Ch] [ebp-Ch] BYREF

    dir[0] = *end - *start;
    dir[1] = end[1] - start[1];
    dir[2] = end[2] - start[2];
    Vec3Normalize(dir);
    if (s_newDebugLine)
    {
        s_start[0] = *start;
        s_start[1] = start[1];
        s_start[2] = start[2];
        s_end[0] = *end;
        s_end[1] = end[1];
        s_end[2] = end[2];
        s_dir[0] = dir[0];
        s_dir[1] = dir[1];
        s_dir[2] = dir[2];
        s_newDebugLine = 0;
    }
    else if ((float)((float)((float)(dir[0] * s_dir[0]) + (float)(dir[1] * s_dir[1])) + (float)(dir[2] * s_dir[2])) < 0.99989998
        || forceDraw)
    {
        k_lineColor[0] = 1.0f;
        k_lineColor[1] = 0.0f;
        k_lineColor[2] = 0.0f;
        k_lineColor[3] = 1.0f;
        G_DebugLine(s_start, s_end, k_lineColor, 1);
        s_start[0] = *start;
        s_start[1] = start[1];
        s_start[2] = start[2];
        s_end[0] = *end;
        s_end[1] = end[1];
        s_end[2] = end[2];
        s_dir[0] = dir[0];
        s_dir[1] = dir[1];
        s_dir[2] = dir[2];
    }
    else
    {
        s_end[0] = *end;
        s_end[1] = end[1];
        s_end[2] = end[2];
    }
}

void __cdecl VP_DebugArrow(const float *pos, const float *angles)
{
    float *v2; // [esp+0h] [ebp-DCh]
    float pts[5][3]; // [esp+1Ch] [ebp-C0h] BYREF
    float k_lineColor[4]; // [esp+58h] [ebp-84h] BYREF
    float scale; // [esp+68h] [ebp-74h]
    int i; // [esp+6Ch] [ebp-70h]
    float axis[4][3]; // [esp+70h] [ebp-6Ch] BYREF
    float origPts[5][3]; // [esp+A0h] [ebp-3Ch] BYREF

    scale = 80.0f;
    //*(_QWORD *)&origPts[0][0] = __PAIR64__(0, LODWORD(0.5f));
    //origPts[0][0] = 0.5f;
    //origPts[0][1] = 0.0f;
    //origPts[0][2] = 0.0f;
    //*(_QWORD *)&origPts[1][0] = __PAIR64__(LODWORD(FLOAT_N0_40000001), LODWORD(-0.5f));
    //origPts[1][2] = 0.0f;
    //*(_QWORD *)&origPts[2][0] = __PAIR64__(LODWORD(0.4f), LODWORD(-0.5f));
    //origPts[2][2] = 0.0f;
    //*(_QWORD *)&origPts[3][0] = __PAIR64__(0, LODWORD(-0.5f));
    //origPts[3][2] = 0.4f;
    //*(_QWORD *)&origPts[4][0] = __PAIR64__(0, LODWORD(-0.5f));
    //origPts[4][2] = 0.0f;

    // (ai assisted)
    origPts[0][0] = 0.5f;
    origPts[0][1] = 0.0f;
    origPts[0][2] = 0.0f;

    origPts[1][0] = -0.5f;
    origPts[1][1] = -0.4f;
    origPts[1][2] = 0.0f;

    origPts[2][0] = -0.5f;
    origPts[2][1] = 0.4f;
    origPts[2][2] = 0.0f;

    origPts[3][0] = -0.5f;
    origPts[3][1] = 0.0f;
    origPts[3][2] = 0.4f;

    origPts[4][0] = -0.5f;
    origPts[4][1] = 0.0f;

    AnglesToAxis(angles, axis);
    axis[3][0] = *pos;
    axis[3][1] = pos[1];
    axis[3][2] = pos[2];
    for ( i = 0; i < 5; ++i )
    {
        v2 = origPts[i];
        *v2 = scale * *v2;
        v2[1] = scale * v2[1];
        v2[2] = scale * v2[2];
        MatrixTransformVector43(origPts[i], axis, pts[i]);
    }
    k_lineColor[0] = 1.0f;
    k_lineColor[1] = 0.0f;
    k_lineColor[2] = 0.0f;
    k_lineColor[3] = 1.0f;
    G_DebugLine(pts[0], pts[1], k_lineColor, 1);
    G_DebugLine(pts[1], pts[2], k_lineColor, 1);
    G_DebugLine(pts[2], pts[0], k_lineColor, 1);
    G_DebugLine(pts[3], pts[4], k_lineColor, 1);
    G_DebugLine(pts[4], pos, k_lineColor, 1);
    G_DebugLine(pos, pts[3], k_lineColor, 1);
}

void __cdecl G_InitVehiclePaths()
{
    s_numNodes = 0;
    s_numVehicleNodeLinks = 0;
}

void __cdecl G_FreeVehiclePaths()
{
    vehicle_node_t *node; // [esp+0h] [ebp-8h]
    __int16 i; // [esp+4h] [ebp-4h]
    __int16 ia; // [esp+4h] [ebp-4h]

    for ( i = 0; i < s_numNodes; ++i )
    {
        node = &s_nodes[i];
        Scr_FreeEntityNum(node->index, 3u, SCRIPTINSTANCE_SERVER);
        VP_ClearNode(node);
    }
    s_numNodes = 0;
    s_numVehicleNodeLinks = 0;
    for ( ia = 0; ia < num_heli_height_lock_patches; ++ia )
            Scr_SetString(&heli_height_lock_patches[ia].targetname, 0, SCRIPTINSTANCE_SERVER);
    num_heli_height_lock_patches = 0;
}

void __cdecl VP_ResetNode(vehicle_node_t *node)
{
    if ( (node->flags & 0x20000) == 0 )
        node->speed = -1.0f;
    if ( (node->flags & 0x40000) == 0 )
        node->lookAhead = -1.0f;
}

void __cdecl G_SetupSplinePaths()
{
    float *origin; // [esp+38h] [ebp-10h]
    vehicle_node_t *node; // [esp+3Ch] [ebp-Ch]
    vehicle_node_t *nodea; // [esp+3Ch] [ebp-Ch]
    vehicle_node_t *nodeb; // [esp+3Ch] [ebp-Ch]
    __int16 j; // [esp+40h] [ebp-8h]
    __int16 i; // [esp+44h] [ebp-4h]
    __int16 ia; // [esp+44h] [ebp-4h]
    __int16 ib; // [esp+44h] [ebp-4h]

    for ( i = 0; i < s_numNodes; ++i )
    {
        node = &s_nodes[i];
        VP_ResetNode(node);
        if ( (node->flags & 2) == 0 )
        {
            if ( node->target )
                node->splineNode.nextIdx = VP_GetNodeIndex(node->target, 0);
            for ( j = 0; j < s_numNodes; ++j )
            {
                if ( node->name && i != j && node->name == s_nodes[j].target )
                {
                    node->splineNode.prevIdx = j;
                    break;
                }
            }
            if ( node->splineNode.nextIdx == i )
                node->splineNode.nextIdx = -1;
            if ( node->splineNode.prevIdx == i )
                node->splineNode.prevIdx = -1;
        }
    }
    for ( ia = 0; ia < s_numNodes; ++ia )
    {
        nodea = &s_nodes[ia];
        if ( (nodea->flags & 2) == 0 && nodea->splineNode.nextIdx >= 0 )
        {
            origin = s_nodes[nodea->splineNode.nextIdx].origin;
            nodea->splineNode.dir[0] = *origin - nodea->origin[0];
            nodea->splineNode.dir[1] = origin[1] - nodea->origin[1];
            nodea->splineNode.dir[2] = origin[2] - nodea->origin[2];
            nodea->splineNode.length = Vec3Normalize(nodea->splineNode.dir);
            if ( (nodea->flags & 0x10000) == 0 )
                vectoangles(nodea->splineNode.dir, nodea->angles);
        }
    }
    for ( ib = 0; ib < s_numNodes; ++ib )
    {
        nodeb = &s_nodes[ib];
        if ( (nodeb->flags & 2) == 0 )
        {
            nodeb->speed = VP_CalcNodeSpeed(ib);
            nodeb->lookAhead = VP_CalcNodeLookAhead(ib);
            if ( nodeb->speed < 0.0 )
                Com_Error(ERR_DROP, "Vehicle path node at( %f, %f, %f ) has negative speed", nodeb->origin[0], nodeb->origin[1], nodeb->origin[2]);
            if ( (nodeb->flags & 0x10000) != 0 )
                VP_CalcNodeAngles(ib, nodeb->angles);
            nodeb->angles[0] = AngleNormalize180(nodeb->angles[0]);
            nodeb->angles[1] = AngleNormalize180(nodeb->angles[1]);
            nodeb->angles[2] = AngleNormalize180(nodeb->angles[2]);
            if ( nodeb->speed <= 0.0 || nodeb->lookAhead <= 0.0 )
                nodeb->splineNode.nextIdx = -1;
            if ( nodeb->splineNode.nextIdx < 0 )
            {
                if ( nodeb->speed <= 0.0 )
                    nodeb->speed = 1.0f;
                if ( nodeb->lookAhead <= 0.0 )
                    nodeb->lookAhead = 1.0f;
            }
        }
    }
}

double __cdecl VP_CalcNodeSpeed(__int16 nodeIdx)
{
    vehicle_node_t *node; // [esp+0h] [ebp-28h]
    float prevDist; // [esp+4h] [ebp-24h]
    float nextDist; // [esp+8h] [ebp-20h]
    float nextSpeed; // [esp+14h] [ebp-14h]
    vehicle_node_t *temp; // [esp+18h] [ebp-10h]
    vehicle_node_t *tempa; // [esp+18h] [ebp-10h]
    float prevSpeed; // [esp+1Ch] [ebp-Ch]
    __int16 count; // [esp+24h] [ebp-4h]
    __int16 counta; // [esp+24h] [ebp-4h]

    node = &s_nodes[nodeIdx];
    if ( (node->flags & 0x20000) != 0 )
        return node->speed;
    nextDist = 0.0f;
    prevDist = 0.0f;
    nextSpeed = -1.0f;
    prevSpeed = -1.0f;
    if ( node->splineNode.prevIdx >= 0 )
    {
        count = 0;
        temp = &s_nodes[node->splineNode.prevIdx];
        while ( count < s_numNodes )
        {
            ++count;
            prevDist = prevDist + temp->splineNode.length;
            if ( temp->speed >= 0.0 )
            {
                prevSpeed = temp->speed;
                break;
            }
            if ( temp->splineNode.prevIdx < 0 || temp->splineNode.prevIdx == nodeIdx )
                break;
            temp = &s_nodes[temp->splineNode.prevIdx];
        }
    }
    counta = 0;
    tempa = &s_nodes[nodeIdx];
    while ( counta < s_numNodes )
    {
        ++counta;
        if ( tempa->speed >= 0.0 )
        {
            nextSpeed = tempa->speed;
            break;
        }
        if ( tempa->splineNode.nextIdx < 0 || tempa->splineNode.nextIdx == nodeIdx )
            break;
        nextDist = nextDist + tempa->splineNode.length;
        tempa = &s_nodes[tempa->splineNode.nextIdx];
    }
    if ( prevSpeed < 0.0 && nextSpeed < 0.0 )
        return 0.0;
    if ( prevSpeed < 0.0 )
        return nextSpeed;
    if ( nextSpeed < 0.0 )
        return prevSpeed;
    if ( (float)(prevDist + nextDist) > 0.0 )
        return (float)((float)((float)(nextSpeed - prevSpeed) * (float)(prevDist / (float)(prevDist + nextDist))) + prevSpeed);
    return 0.0;
}

double __cdecl VP_CalcNodeLookAhead(__int16 nodeIdx)
{
    vehicle_node_t *node; // [esp+0h] [ebp-28h]
    float prevDist; // [esp+4h] [ebp-24h]
    float nextDist; // [esp+8h] [ebp-20h]
    vehicle_node_t *temp; // [esp+18h] [ebp-10h]
    vehicle_node_t *tempa; // [esp+18h] [ebp-10h]
    float prevLook; // [esp+1Ch] [ebp-Ch]
    float nextLook; // [esp+20h] [ebp-8h]
    __int16 count; // [esp+24h] [ebp-4h]
    __int16 counta; // [esp+24h] [ebp-4h]

    node = &s_nodes[nodeIdx];
    if ( node->lookAhead >= 0.0 )
        return node->lookAhead;
    nextDist = 0.0f;
    prevDist = 0.0f;
    nextLook = -1.0f;
    prevLook = -1.0f;
    if ( node->splineNode.prevIdx >= 0 )
    {
        count = 0;
        temp = &s_nodes[node->splineNode.prevIdx];
        while ( count < s_numNodes )
        {
            ++count;
            prevDist = prevDist + temp->splineNode.length;
            if ( temp->lookAhead > 0.0 )
            {
                prevLook = temp->lookAhead;
                break;
            }
            if ( temp->splineNode.prevIdx < 0 || temp->splineNode.prevIdx == nodeIdx )
                break;
            temp = &s_nodes[temp->splineNode.prevIdx];
        }
    }
    counta = 0;
    tempa = &s_nodes[nodeIdx];
    while ( counta < s_numNodes )
    {
        ++counta;
        if ( tempa->lookAhead > 0.0 )
        {
            nextLook = tempa->lookAhead;
            break;
        }
        if ( tempa->splineNode.nextIdx < 0 || tempa->splineNode.nextIdx == nodeIdx )
            break;
        nextDist = nextDist + tempa->splineNode.length;
        tempa = &s_nodes[tempa->splineNode.nextIdx];
    }
    if ( prevLook < 0.0 && nextLook < 0.0 )
        return 0.0;
    if ( prevLook < 0.0 )
        return nextLook;
    if ( nextLook < 0.0 )
        return prevLook;
    if ( (float)(prevDist + nextDist) > 0.0 )
        return (float)((float)((float)(nextLook - prevLook) * (float)(prevDist / (float)(prevDist + nextDist))) + prevLook);
    return 0.0;
}

void __cdecl VP_CalcNodeAngles(__int16 nodeIdx, float *angles)
{
    vehicle_node_t *node; // [esp+5Ch] [ebp-34h]
    float prevDist; // [esp+60h] [ebp-30h]
    float nextDist; // [esp+64h] [ebp-2Ch]
    float totalDist; // [esp+6Ch] [ebp-24h]
    float nextAngles; // [esp+70h] [ebp-20h]
    float nextAngles_4; // [esp+74h] [ebp-1Ch]
    float nextAngles_8; // [esp+78h] [ebp-18h]
    vehicle_node_t *temp; // [esp+7Ch] [ebp-14h]
    vehicle_node_t *tempa; // [esp+7Ch] [ebp-14h]
    float prevAngles; // [esp+80h] [ebp-10h]
    float prevAngles_4; // [esp+84h] [ebp-Ch]
    float prevAngles_8; // [esp+88h] [ebp-8h]
    __int16 count; // [esp+8Ch] [ebp-4h]
    __int16 counta; // [esp+8Ch] [ebp-4h]

    node = &s_nodes[nodeIdx];
    if ( node->angles[0] != s_invalidAngles[0]
        || s_nodes[nodeIdx].angles[1] != s_invalidAngles[1]
        || s_nodes[nodeIdx].angles[2] != s_invalidAngles[2] )
    {
        *angles = node->angles[0];
        angles[1] = node->angles[1];
        angles[2] = node->angles[2];
        return;
    }
    nextDist = 0.0f;
    prevDist = 0.0f;
    prevAngles = s_invalidAngles[0];
    prevAngles_4 = s_invalidAngles[1];
    prevAngles_8 = s_invalidAngles[2];
    nextAngles = s_invalidAngles[0];
    nextAngles_4 = s_invalidAngles[1];
    nextAngles_8 = s_invalidAngles[2];
    if ( node->splineNode.prevIdx >= 0 )
    {
        count = 0;
        temp = &s_nodes[node->splineNode.prevIdx];
        while ( count < s_numNodes )
        {
            ++count;
            prevDist = prevDist + temp->splineNode.length;
            if ( temp->angles[0] != s_invalidAngles[0]
                || temp->angles[1] != s_invalidAngles[1]
                || temp->angles[2] != s_invalidAngles[2] )
            {
                prevAngles = temp->angles[0];
                prevAngles_4 = temp->angles[1];
                prevAngles_8 = temp->angles[2];
                break;
            }
            if ( temp->splineNode.prevIdx < 0 || temp->splineNode.prevIdx == nodeIdx )
                break;
            temp = &s_nodes[temp->splineNode.prevIdx];
        }
    }
    counta = 0;
    tempa = &s_nodes[nodeIdx];
    while ( counta < s_numNodes )
    {
        ++counta;
        if ( tempa->angles[0] != s_invalidAngles[0]
            || tempa->angles[1] != s_invalidAngles[1]
            || tempa->angles[2] != s_invalidAngles[2] )
        {
            nextAngles = tempa->angles[0];
            nextAngles_4 = tempa->angles[1];
            nextAngles_8 = tempa->angles[2];
            break;
        }
        if ( tempa->splineNode.nextIdx < 0 || tempa->splineNode.nextIdx == nodeIdx )
            break;
        nextDist = nextDist + tempa->splineNode.length;
        tempa = &s_nodes[tempa->splineNode.nextIdx];
    }
    if ( prevAngles == s_invalidAngles[0]
        && prevAngles_4 == s_invalidAngles[1]
        && prevAngles_8 == s_invalidAngles[2]
        && nextAngles == s_invalidAngles[0]
        && nextAngles_4 == s_invalidAngles[1]
        && nextAngles_8 == s_invalidAngles[2] )
    {
        goto LABEL_47;
    }
    if ( prevAngles == s_invalidAngles[0] && prevAngles_4 == s_invalidAngles[1] && prevAngles_8 == s_invalidAngles[2] )
    {
        *angles = nextAngles;
        angles[1] = nextAngles_4;
        angles[2] = nextAngles_8;
    }
    else if ( nextAngles == s_invalidAngles[0] && nextAngles_4 == s_invalidAngles[1] && nextAngles_8 == s_invalidAngles[2] )
    {
        *angles = prevAngles;
        angles[1] = prevAngles_4;
        angles[2] = prevAngles_8;
    }
    else
    {
        totalDist = prevDist + nextDist;
        if ( (float)(prevDist + nextDist) <= 0.0 )
        {
LABEL_47:
            *angles = 0.0f;
            angles[1] = 0.0f;
            angles[2] = 0.0f;
            return;
        }
        *angles = AngleNormalize180(nextAngles - prevAngles) * (float)(prevDist / totalDist) + prevAngles;
        angles[1] = AngleNormalize180(nextAngles_4 - prevAngles_4) * (float)(prevDist / totalDist) + prevAngles_4;
        angles[2] = AngleNormalize180(nextAngles_8 - prevAngles_8) * (float)(prevDist / totalDist) + prevAngles_8;
    }
}

void __cdecl G_ConnectVehiclePaths()
{
    vehicle_path_node_link_t *v0; // [esp+50h] [ebp-1Ch]
    vehicle_path_node_link_t *link; // [esp+54h] [ebp-18h]
    vehicle_node_t *node; // [esp+58h] [ebp-14h]
    __int16 j; // [esp+5Ch] [ebp-10h]
    __int16 pathNodeIdx; // [esp+60h] [ebp-Ch]
    __int16 pathNodeIdxa; // [esp+60h] [ebp-Ch]
    vehicle_node_t *other; // [esp+64h] [ebp-8h]
    __int16 i; // [esp+68h] [ebp-4h]

    VP_ResetLinks();
    for ( i = 0; i < s_numNodes; ++i )
    {
        node = &s_nodes[i];
        if ( (node->flags & 2) != 0 )
        {
            VP_ResetNode(node);
            node->splineNode.nextIdx = 0;
            if ( node->speed <= 0.0 )
                node->speed = 100.0 * 17.6;
            if ( node->splineNode.length <= 0.0 )
                node->splineNode.length = 200.0f;
            if ( node->lookAhead <= 0.0 )
                node->lookAhead = (float)(node->splineNode.length * 2.0) - 50.0;
            if ( node->lookAhead <= 0.0 )
                node->lookAhead = 200.0f;
            for ( j = 0; j < s_numNodes; ++j )
            {
                other = &s_nodes[j];
                if ( (other->flags & 2) != 0 && other != node )
                {
                    if ( other->name && (node->target == other->name || node->target2 == other->name) )
                    {
                        pathNodeIdx = VP_GetNextFreePathNode();
                        if ( !node->splineNode.nextIdx )
                            node->splineNode.prevIdx = pathNodeIdx;
                        ++node->splineNode.nextIdx;
                        link = &s_node_links[pathNodeIdx];
                        link->nextIdx = other->index;
                        link->length = Vec3Distance(node->origin, other->origin);
                        link->dir[0] = other->origin[0] - node->origin[0];
                        link->dir[1] = other->origin[1] - node->origin[1];
                        link->dir[2] = other->origin[2] - node->origin[2];
                        Vec3Normalize(link->dir);
                    }
                    if ( node->name && (other->target == node->name || other->target2 == node->name) )
                    {
                        pathNodeIdxa = VP_GetNextFreePathNode();
                        if ( !node->splineNode.nextIdx )
                            node->splineNode.prevIdx = pathNodeIdxa;
                        ++node->splineNode.nextIdx;
                        v0 = &s_node_links[pathNodeIdxa];
                        v0->nextIdx = other->index;
                        v0->length = Vec3Distance(node->origin, other->origin);
                        v0->dir[0] = other->origin[0] - node->origin[0];
                        v0->dir[1] = other->origin[1] - node->origin[1];
                        v0->dir[2] = other->origin[2] - node->origin[2];
                        Vec3Normalize(v0->dir);
                    }
                }
            }
        }
    }
}

void __cdecl G_SetupVehiclePaths()
{
    G_SetupSplinePaths();
    G_ConnectVehiclePaths();
}

void __cdecl VP_ZeroNode(vehicle_node_t *node)
{
    node->name = 0;
    node->target = 0;
    node->target2 = 0;
    node->script_linkname = 0;
    node->script_noteworthy = 0;
}

void __cdecl G_VehInitPathPos(vehicle_pathpos_t *vpp)
{
    vpp->lastNodeIdx = -1;
    vpp->nodeIdx = -1;
    vpp->endOfPath = 0;
    vpp->frac = 0.0f;
    vpp->speed = 0.0f;
    vpp->lookAhead = 0.0f;
    vpp->slide = 0.0f;
    vpp->customPath = 0;
    vpp->origin[0] = 0.0f;
    vpp->origin[1] = 0.0f;
    vpp->origin[2] = 0.0f;
    vpp->angles[0] = 0.0f;
    vpp->angles[1] = 0.0f;
    vpp->angles[2] = 0.0f;
    vpp->lookPos[0] = 0.0f;
    vpp->lookPos[1] = 0.0f;
    vpp->lookPos[2] = 0.0f;
    VP_InitNode(vpp->switchNode, -1);
    VP_InitNode(&vpp->switchNode[1], -1);
}

void __cdecl VP_InitNode(vehicle_node_t *node, __int16 nodeIdx)
{
    VP_ZeroNode(node);
    node->index = nodeIdx;
    node->flags = 0;
    node->speed = -1.0f;
    node->lookAhead = -1.0f;
    node->origin[0] = 0.0f;
    node->origin[1] = 0.0f;
    node->origin[2] = 0.0f;
    node->splineNode.dir[0] = 0.0f;
    node->splineNode.dir[1] = 0.0f;
    node->splineNode.dir[2] = 0.0f;
    node->angles[0] = s_invalidAngles[0];
    node->angles[1] = s_invalidAngles[1];
    node->angles[2] = s_invalidAngles[2];
    node->splineNode.length = 0.0f;
    node->splineNode.nextIdx = -1;
    node->splineNode.prevIdx = -1;
}

void __cdecl G_VehFreePathPos(vehicle_pathpos_t *vpp)
{
    VP_ClearNode(vpp->switchNode);
    VP_ClearNode(&vpp->switchNode[1]);
}

void __cdecl G_VehSetUpPathPos(vehicle_pathpos_t *vpp, __int16 nodeIdx)
{
    float v2; // [esp+0h] [ebp-20h]
    vehicle_node_t *node; // [esp+1Ch] [ebp-4h]

    node = &s_nodes[nodeIdx];
    vpp->lastNodeIdx = nodeIdx;
    vpp->nodeIdx = nodeIdx;
    vpp->endOfPath = 0;
    vpp->frac = 0.0f;
    vpp->speed = node->speed;
    vpp->lookAhead = node->lookAhead;
    if ( (node->flags & 0x10000) != 0 )
        v2 = 1.0f;
    else
        v2 = 0.0f;
    vpp->slide = v2;
    vpp->flags = 0;
    vpp->customPath = 0;
    vpp->origin[0] = node->origin[0];
    vpp->origin[1] = node->origin[1];
    vpp->origin[2] = node->origin[2];
    vpp->angles[0] = node->angles[0];
    vpp->angles[1] = node->angles[1];
    vpp->angles[2] = node->angles[2];
    vpp->lookPos[0] = node->origin[0];
    vpp->lookPos[1] = node->origin[1];
    vpp->lookPos[2] = node->origin[2];
    VP_InitNode(vpp->switchNode, -1);
    VP_InitNode(&vpp->switchNode[1], -1);
}

int __cdecl G_VehUpdatePathPos(vehicle_pathpos_t *vpp, __int16 testNode)
{
    float v3; // [esp+8h] [ebp-3Ch]
    float dist; // [esp+30h] [ebp-14h]
    float lookDir[3]; // [esp+34h] [ebp-10h] BYREF
    int test; // [esp+40h] [ebp-4h]

    test = 0;
    if ( vpp->endOfPath )
        return 0;
    VP_BeginSwitchNode(vpp);
    if ( vpp->customPath )
        VP_GetLookAheadXYZCustom(vpp, vpp->lookPos);
    else
        VP_GetLookAheadXYZ(vpp, vpp->lookPos);
    lookDir[0] = vpp->lookPos[0] - vpp->origin[0];
    lookDir[1] = vpp->lookPos[1] - vpp->origin[1];
    lookDir[2] = vpp->lookPos[2] - vpp->origin[2];
    dist = Vec3Normalize(lookDir);
    if ( dist <= 0.0 )
    {
        vpp->endOfPath = 1;
    }
    else
    {
        vectoangles(lookDir, vpp->angles);
        vpp->angles[0] = AngleNormalize180(vpp->angles[0]);
        vpp->angles[1] = AngleNormalize180(vpp->angles[1]);
        vpp->angles[2] = AngleNormalize180(vpp->angles[2]);
        v3 = vpp->speed * 0.05f;
        vpp->origin[0] = (float)(v3 * lookDir[0]) + vpp->origin[0];
        vpp->origin[1] = (float)(v3 * lookDir[1]) + vpp->origin[1];
        vpp->origin[2] = (float)(v3 * lookDir[2]) + vpp->origin[2];
        if ( vpp->customPath )
            test = VP_UpdatePathPosCustom(vpp, lookDir, testNode);
        else
            test = VP_UpdatePathPos(vpp, lookDir, testNode);
        VP_GetAngles(vpp, vpp->angles);
    }
    VP_EndSwitchNode(vpp);
    return test;
}

void __cdecl VP_GetAngles(const vehicle_pathpos_t *vpp, float *angles)
{
    double v2; // st7
    double v3; // st7
    float v4; // [esp+8h] [ebp-50h]
    float frac; // [esp+10h] [ebp-48h]
    vehicle_node_t *node; // [esp+2Ch] [ebp-2Ch]
    float a2[3]; // [esp+30h] [ebp-28h]
    __int16 nodeIdx; // [esp+3Ch] [ebp-1Ch]
    vehicle_node_t *next; // [esp+40h] [ebp-18h]
    __int16 nextNodeIdx; // [esp+44h] [ebp-14h]
    float a1[3]; // [esp+48h] [ebp-10h]
    int i; // [esp+54h] [ebp-4h]

    nodeIdx = VPP_GetCurrentNode(vpp, vpp->nodeIdx);
    node = &s_nodes[nodeIdx];
    if ( (node->flags & 2) != 0 )
        nextNodeIdx = VPP_GetCurrentNode(vpp, vpp->nodeIdx + 1);
    else
        nextNodeIdx = node->splineNode.nextIdx;
    if ( nextNodeIdx >= 0 )
    {
        next = &s_nodes[nextNodeIdx];
        if ( (node->flags & 0x10000) != 0 || (next->flags & 0x10000) != 0 )
        {
            if ( (node->flags & 0x10000) != 0 && (next->flags & 0x10000) != 0 )
            {
                a1[0] = node->angles[0];
                a1[1] = node->angles[1];
                a1[2] = node->angles[2];
                a2[0] = next->angles[0];
                a2[1] = next->angles[1];
                a2[2] = next->angles[2];
            }
            else if ( (node->flags & 0x10000) != 0 || (next->flags & 0x10000) == 0 )
            {
                a1[0] = node->angles[0];
                a1[1] = node->angles[1];
                a1[2] = node->angles[2];
                a2[0] = *angles;
                a2[1] = angles[1];
                a2[2] = angles[2];
            }
            else
            {
                a1[0] = *angles;
                a1[1] = angles[1];
                a1[2] = angles[2];
                a2[0] = next->angles[0];
                a2[1] = next->angles[1];
                a2[2] = next->angles[2];
            }
            for ( i = 0; i < 3; ++i )
            {
                v4 = a1[i];
                frac = vpp->frac;
                v2 = AngleNormalize180(a2[i] - v4);
                angles[i] = v2 * frac + v4;
                v3 = AngleNormalize180(angles[i]);
                angles[i] = v3;
            }
        }
    }
    else if ( (node->flags & 0x10000) != 0 )
    {
        *angles = node->angles[0];
        angles[1] = node->angles[1];
        angles[2] = node->angles[2];
    }
}

__int16 __cdecl VPP_GetCurrentNode(const vehicle_pathpos_t *vpp, __int16 currentNode)
{
    if ( vpp->customPath )
        return vpp->customPath->pathOrder[currentNode];
    else
        return currentNode;
}

void __cdecl VP_GetLookAheadXYZCustom(const vehicle_pathpos_t *vpp, float *lookXYZ)
{
    vehicle_node_t *node; // [esp+10h] [ebp-20h]
    __int16 nodeIdx; // [esp+14h] [ebp-1Ch]
    vehicle_path_node_link_t *link; // [esp+1Ch] [ebp-14h]
    float dist; // [esp+20h] [ebp-10h]
    __int16 pathIdx; // [esp+28h] [ebp-8h]
    float length; // [esp+2Ch] [ebp-4h]

    pathIdx = vpp->nodeIdx;
    node = &s_nodes[vpp->customPath->pathOrder[vpp->nodeIdx]];
    link = &s_node_links[vpp->customPath->pathLinkIdx[vpp->nodeIdx]];
    dist = (float)(vpp->frac * link->length) + vpp->lookAhead;
    while ( 1 )
    {
        length = link->length;
        nodeIdx = vpp->customPath->pathOrder[++pathIdx];
        if ( nodeIdx < 0 || length == 0.0 )
            break;
        if ( length > dist )
        {
            *lookXYZ = (float)(dist * link->dir[0]) + node->origin[0];
            lookXYZ[1] = (float)(dist * link->dir[1]) + node->origin[1];
            lookXYZ[2] = (float)(dist * link->dir[2]) + node->origin[2];
            return;
        }
        dist = dist - length;
        node = &s_nodes[nodeIdx];
        link = &s_node_links[vpp->customPath->pathLinkIdx[pathIdx]];
    }
    if ( dist < 0.0 )
        dist = 0.0f;
    *lookXYZ = (float)(dist * vpp->customGoalDir[0]) + node->origin[0];
    lookXYZ[1] = (float)(dist * vpp->customGoalDir[1]) + node->origin[1];
    lookXYZ[2] = (float)(dist * vpp->customGoalDir[2]) + node->origin[2];
}

void __cdecl VP_GetLookAheadXYZ(const vehicle_pathpos_t *vpp, float *lookXYZ)
{
    vehicle_node_t *node; // [esp+8h] [ebp-10h]
    float dist; // [esp+Ch] [ebp-Ch]
    __int16 count; // [esp+14h] [ebp-4h]

    node = &s_nodes[vpp->nodeIdx];
    dist = (float)(vpp->frac * node->splineNode.length) + (float)(vpp->lookAhead * vpp->speed);
    count = 0;
    while ( count < s_numNodes )
    {
        ++count;
        if ( node->splineNode.nextIdx < 0 || node->splineNode.length == 0.0 )
        {
            dist = 0.0f;
            break;
        }
        if ( node->splineNode.length > dist )
            break;
        dist = dist - node->splineNode.length;
        node = &s_nodes[node->splineNode.nextIdx];
    }
    *lookXYZ = (float)(dist * node->splineNode.dir[0]) + node->origin[0];
    lookXYZ[1] = (float)(dist * node->splineNode.dir[1]) + node->origin[1];
    lookXYZ[2] = (float)(dist * node->splineNode.dir[2]) + node->origin[2];
}

int __cdecl VP_UpdatePathPosCustom(vehicle_pathpos_t *vpp, const float *dir, __int16 nodeTest)
{
    float v4; // [esp+0h] [ebp-9Ch]
    float v5; // [esp+4h] [ebp-98h]
    float nnodePlane; // [esp+30h] [ebp-6Ch]
    float nnodePlane_4; // [esp+34h] [ebp-68h]
    float nnodePlane_8; // [esp+38h] [ebp-64h]
    float d1; // [esp+48h] [ebp-54h]
    __int16 nodeIdx; // [esp+50h] [ebp-4Ch]
    float frac; // [esp+54h] [ebp-48h]
    float fraca; // [esp+54h] [ebp-48h]
    __int16 linkIdx; // [esp+58h] [ebp-44h]
    vehicle_path_node_link_t *nlink; // [esp+60h] [ebp-3Ch]
    int test; // [esp+64h] [ebp-38h]
    vehicle_path_node_link_t *clink; // [esp+68h] [ebp-34h]
    float cnodeToVPP; // [esp+6Ch] [ebp-30h]
    float cnodeToVPP_4; // [esp+70h] [ebp-2Ch]
    float cnodeToVPP_8; // [esp+74h] [ebp-28h]
    float d2; // [esp+78h] [ebp-24h]
    __int16 nextNodeIdx; // [esp+7Ch] [ebp-20h]
    __int16 pathIdx; // [esp+80h] [ebp-1Ch]
    __int16 prevLinkIdx; // [esp+84h] [ebp-18h]
    vehicle_node_t *cnode; // [esp+88h] [ebp-14h]
    vehicle_node_t *nnode; // [esp+8Ch] [ebp-10h]
    float VPPToNNode[3]; // [esp+90h] [ebp-Ch] BYREF

    pathIdx = vpp->nodeIdx;
    nodeIdx = vpp->customPath->pathOrder[vpp->nodeIdx];
    linkIdx = vpp->customPath->pathLinkIdx[vpp->nodeIdx];
    test = 0;
    while ( 1 )
    {
        while ( 1 )
        {
            cnode = &s_nodes[nodeIdx];
            clink = &s_node_links[linkIdx];
            nextNodeIdx = vpp->customPath->pathOrder[pathIdx + 1];
            nnode = &s_nodes[nextNodeIdx];
            nlink = &s_node_links[vpp->customPath->pathLinkIdx[pathIdx + 1]];
            if ( (nnode->flags & 0x80000) != 0 )
                nextNodeIdx = -1;
            if ( pathIdx <= 0 )
                prevLinkIdx = linkIdx;
            else
                prevLinkIdx = vpp->customPath->pathOrder[pathIdx + 127];
            if ( nodeIdx == nodeTest )
                test = 1;
            if ( nextNodeIdx < 0 || clink->length == 0.0 )
            {
                frac = 0.0f;
                goto LABEL_30;
            }
            nnodePlane = 0.5 * (float)(clink->dir[0] + nlink->dir[0]);
            nnodePlane_4 = 0.5 * (float)(clink->dir[1] + nlink->dir[1]);
            nnodePlane_8 = 0.5 * (float)(clink->dir[2] + nlink->dir[2]);
            cnodeToVPP = vpp->origin[0] - cnode->origin[0];
            cnodeToVPP_4 = vpp->origin[1] - cnode->origin[1];
            cnodeToVPP_8 = vpp->origin[2] - cnode->origin[2];
            d1 = (float)((float)(cnodeToVPP * (float)(0.5 * (float)(s_node_links[prevLinkIdx].dir[0] + clink->dir[0])))
                                 + (float)(cnodeToVPP_4 * (float)(0.5 * (float)(s_node_links[prevLinkIdx].dir[1] + clink->dir[1]))))
                 + (float)(cnodeToVPP_8 * (float)(0.5 * (float)(s_node_links[prevLinkIdx].dir[2] + clink->dir[2])));
            VPPToNNode[0] = nnode->origin[0] - vpp->origin[0];
            VPPToNNode[1] = nnode->origin[1] - vpp->origin[1];
            VPPToNNode[2] = nnode->origin[2] - vpp->origin[2];
            d2 = (float)((float)(VPPToNNode[0] * nnodePlane) + (float)(VPPToNNode[1] * nnodePlane_4))
                 + (float)(VPPToNNode[2] * nnodePlane_8);
            if ( d1 == 0.0 && d2 == 0.0 )
            {
                frac = 0.0f;
                goto LABEL_30;
            }
            if ( d2 >= 0.0 )
                break;
            ++pathIdx;
            nodeIdx = nextNodeIdx;
        }
        if ( d1 >= 0.0 )
            break;
        if ( (float)((float)((float)(clink->dir[0] * cnodeToVPP) + (float)(clink->dir[1] * cnodeToVPP_4))
                             + (float)(clink->dir[2] * cnodeToVPP_8)) < 0.0 )
        {
            frac = 0.0f;
            goto LABEL_30;
        }
        fraca = 1.0 - Vec3Length(VPPToNNode) / clink->length;
        if ( (float)(fraca - 1.0) < 0.0 )
            v5 = fraca;
        else
            v5 = 1.0f;
        if ( (float)(0.0 - fraca) < 0.0 )
            v4 = v5;
        else
            v4 = 0.0f;
        frac = v4;
        if ( v4 <= 0.94999999 )
            goto LABEL_30;
        ++pathIdx;
        nodeIdx = nextNodeIdx;
    }
    frac = d1 / (float)(d1 + d2);
LABEL_30:
    vpp->endOfPath = nextNodeIdx < 0;
    vpp->nodeIdx = pathIdx;
    vpp->frac = frac;
    vpp->speed = VP_GetSpeed(vpp);
    vpp->lookAhead = VP_GetLookAhead(vpp);
    vpp->slide = VP_GetSlide(vpp);
    return test;
}

double __cdecl VP_GetSpeed(const vehicle_pathpos_t *vpp)
{
    vehicle_node_t *node; // [esp+0h] [ebp-10h]
    __int16 nodeIdx; // [esp+4h] [ebp-Ch]
    __int16 nextNodeIdx; // [esp+8h] [ebp-8h]

    if ( vpp->customPath )
    {
        nodeIdx = vpp->customPath->pathOrder[vpp->nodeIdx];
        nextNodeIdx = vpp->customPath->pathOrder[vpp->nodeIdx + 1];
    }
    else
    {
        nodeIdx = vpp->nodeIdx;
        nextNodeIdx = s_nodes[vpp->nodeIdx].splineNode.nextIdx;
    }
    node = &s_nodes[nodeIdx];
    if ( nextNodeIdx >= 0 )
        return (float)((float)((float)(s_nodes[nextNodeIdx].speed - node->speed) * vpp->frac) + node->speed);
    else
        return node->speed;
}

double __cdecl VP_GetLookAhead(const vehicle_pathpos_t *vpp)
{
    vehicle_node_t *node; // [esp+0h] [ebp-10h]
    __int16 nodeIdx; // [esp+4h] [ebp-Ch]
    __int16 nextNodeIdx; // [esp+Ch] [ebp-4h]

    if ( vpp->customPath )
    {
        nodeIdx = vpp->customPath->pathOrder[vpp->nodeIdx];
        nextNodeIdx = vpp->customPath->pathOrder[vpp->nodeIdx + 1];
    }
    else
    {
        nodeIdx = vpp->nodeIdx;
        nextNodeIdx = s_nodes[vpp->nodeIdx].splineNode.nextIdx;
    }
    node = &s_nodes[nodeIdx];
    if ( node->splineNode.nextIdx >= 0 )
        return (float)((float)((float)(s_nodes[nextNodeIdx].lookAhead - node->lookAhead) * vpp->frac) + node->lookAhead);
    else
        return node->lookAhead;
}

double __cdecl VP_GetSlide(const vehicle_pathpos_t *vpp)
{
    vehicle_node_t *node; // [esp+4h] [ebp-10h]
    __int16 nodeIdx; // [esp+8h] [ebp-Ch]
    vehicle_node_t *next; // [esp+Ch] [ebp-8h]
    __int16 nextNodeIdx; // [esp+10h] [ebp-4h]

    if ( vpp->customPath )
    {
        nodeIdx = vpp->customPath->pathOrder[vpp->nodeIdx];
        nextNodeIdx = vpp->customPath->pathOrder[vpp->nodeIdx + 1];
    }
    else
    {
        nodeIdx = vpp->nodeIdx;
        nextNodeIdx = s_nodes[vpp->nodeIdx].splineNode.nextIdx;
    }
    node = &s_nodes[nodeIdx];
    if ( nextNodeIdx >= 0 )
    {
        next = &s_nodes[nextNodeIdx];
        if ( (node->flags & 0x10000) != 0 && (next->flags & 0x10000) != 0 )
        {
            return 1.0;
        }
        else if ( (node->flags & 0x10000) != 0 || (next->flags & 0x10000) == 0 )
        {
            if ( (node->flags & 0x10000) == 0 || (next->flags & 0x10000) != 0 )
                return 0.0;
            else
                return 1.0 - vpp->frac;
        }
        else
        {
            return vpp->frac;
        }
    }
    else if ( (node->flags & 0x10000) != 0 )
    {
        return 1.0f;
    }
    else
    {
        return 0.0f;
    }
}

static const float tightTurnThreshold = 0.866;
int __cdecl VP_UpdatePathPos(vehicle_pathpos_t *vpp, const float *dir, __int16 nodeTest)
{
    double v3; // st7
    float v5; // [esp+0h] [ebp-B0h]
    float v6; // [esp+4h] [ebp-ACh]
    float nnodePlane[3]; // [esp+54h] [ebp-5Ch] BYREF
    vehicle_node_t *pnode; // [esp+60h] [ebp-50h]
    float cnodePlane[3]; // [esp+64h] [ebp-4Ch] BYREF
    float angleDiff; // [esp+70h] [ebp-40h]
    float d1; // [esp+74h] [ebp-3Ch]
    __int16 nodeIdx; // [esp+78h] [ebp-38h]
    float frac; // [esp+7Ch] [ebp-34h]
    __int16 lastNodeIdx; // [esp+80h] [ebp-30h]
    int test; // [esp+84h] [ebp-2Ch]
    float cnodeToVPP[3]; // [esp+88h] [ebp-28h]
    float d2; // [esp+94h] [ebp-1Ch]
    vehicle_node_t *cnode; // [esp+98h] [ebp-18h]
    vehicle_node_t *nnode; // [esp+9Ch] [ebp-14h]
    float VPPToNNode[3]; // [esp+A0h] [ebp-10h] BYREF
    __int16 count; // [esp+ACh] [ebp-4h]

    nodeIdx = vpp->nodeIdx;
    lastNodeIdx = vpp->lastNodeIdx;
    cnode = &s_nodes[nodeIdx];
    frac = vpp->frac;
    test = 0;
    count = 0;
    while ( count < s_numNodes )
    {
        ++count;
        cnode = &s_nodes[nodeIdx];
        if ( nodeIdx == nodeTest )
            test = 1;
        if ( cnode->splineNode.nextIdx < 0 || cnode->splineNode.length == 0.0 )
        {
            frac = 0.0f;
            break;
        }
        nnode = &s_nodes[cnode->splineNode.nextIdx];
        pnode = &s_nodes[lastNodeIdx];
        nnodePlane[0] = cnode->splineNode.dir[0] + nnode->splineNode.dir[0];
        nnodePlane[1] = cnode->splineNode.dir[1] + nnode->splineNode.dir[1];
        nnodePlane[2] = cnode->splineNode.dir[2] + nnode->splineNode.dir[2];
        Vec3Normalize(nnodePlane);
        angleDiff = (float)((float)(pnode->splineNode.dir[0] * cnode->splineNode.dir[0])
                                            + (float)(pnode->splineNode.dir[1] * cnode->splineNode.dir[1]))
                            + (float)(pnode->splineNode.dir[2] * cnode->splineNode.dir[2]);
        if ( (-(tightTurnThreshold)) <= angleDiff )
        {
            cnodePlane[0] = pnode->splineNode.dir[0] + cnode->splineNode.dir[0];
            cnodePlane[1] = pnode->splineNode.dir[1] + cnode->splineNode.dir[1];
            cnodePlane[2] = pnode->splineNode.dir[2] + cnode->splineNode.dir[2];
            Vec3Normalize(cnodePlane);
        }
        else
        {
            cnodePlane[0] = cnode->splineNode.dir[0];
            cnodePlane[1] = cnode->splineNode.dir[1];
            cnodePlane[2] = cnode->splineNode.dir[2];
        }
        cnodeToVPP[0] = vpp->origin[0] - cnode->origin[0];
        cnodeToVPP[1] = vpp->origin[1] - cnode->origin[1];
        cnodeToVPP[2] = vpp->origin[2] - cnode->origin[2];
        d1 = (float)((float)(cnodeToVPP[0] * cnodePlane[0]) + (float)(cnodeToVPP[1] * cnodePlane[1]))
             + (float)(cnodeToVPP[2] * cnodePlane[2]);
        VPPToNNode[0] = nnode->origin[0] - vpp->origin[0];
        VPPToNNode[1] = nnode->origin[1] - vpp->origin[1];
        VPPToNNode[2] = nnode->origin[2] - vpp->origin[2];
        d2 = (float)((float)(VPPToNNode[0] * nnodePlane[0]) + (float)(VPPToNNode[1] * nnodePlane[1]))
             + (float)(VPPToNNode[2] * nnodePlane[2]);
        if ( d2 >= 0.0 )
        {
            if ( d1 >= 0.0 )
            {
                frac = d1 / (float)(d1 + d2);
                break;
            }
            d1 = (float)((float)(cnode->splineNode.dir[0] * cnodeToVPP[0]) + (float)(cnode->splineNode.dir[1] * cnodeToVPP[1]))
                 + (float)(cnode->splineNode.dir[2] * cnodeToVPP[2]);
            if ( d1 < 0.0 )
            {
                frac = 0.0f;
                break;
            }
            v3 = Vec3Length(VPPToNNode);
            frac = 1.0 - v3 / cnode->splineNode.length;
            if ( (float)(frac - 1.0) < 0.0 )
                v6 = frac;
            else
                v6 = 1.0f;
            if ( (float)(0.0 - frac) < 0.0 )
                v5 = v6;
            else
                v5 = 0.0f;
            frac = v5;
            if ( v5 <= 0.89999998 )
                break;
            lastNodeIdx = nodeIdx;
            nodeIdx = cnode->splineNode.nextIdx;
        }
        else
        {
            lastNodeIdx = nodeIdx;
            nodeIdx = cnode->splineNode.nextIdx;
        }
    }
    vpp->endOfPath = cnode->splineNode.nextIdx < 0;
    vpp->lastNodeIdx = lastNodeIdx;
    vpp->nodeIdx = nodeIdx;
    vpp->frac = frac;
    vpp->speed = VP_GetSpeed(vpp);
    vpp->lookAhead = VP_GetLookAhead(vpp);
    vpp->slide = VP_GetSlide(vpp);
    return test;
}

void __cdecl VP_BeginSwitchNode(const vehicle_pathpos_t *vpp)
{
    __int16 nodeIdx; // [esp+0h] [ebp-4h]

    nodeIdx = VP_GetNodeIndex(vpp->switchNode[0].name, 0);
    if ( nodeIdx >= 0 )
        VP_CopyNode(vpp->switchNode, &s_nodes[nodeIdx]);
}

void __cdecl VP_CopyNode(const vehicle_node_t *src, vehicle_node_t *dst)
{
    Scr_SetString(&dst->name, src->name, SCRIPTINSTANCE_SERVER);
    Scr_SetString(&dst->target, src->target, SCRIPTINSTANCE_SERVER);
    Scr_SetString(&dst->target2, src->target2, SCRIPTINSTANCE_SERVER);
    Scr_SetString(&dst->script_linkname, src->script_linkname, SCRIPTINSTANCE_SERVER);
    Scr_SetString(&dst->script_noteworthy, src->script_noteworthy, SCRIPTINSTANCE_SERVER);
    dst->index = src->index;
    dst->flags = src->flags;
    dst->speed = src->speed;
    dst->lookAhead = src->lookAhead;
    dst->origin[0] = src->origin[0];
    dst->origin[1] = src->origin[1];
    dst->origin[2] = src->origin[2];
    dst->splineNode.dir[0] = src->splineNode.dir[0];
    dst->splineNode.dir[1] = src->splineNode.dir[1];
    dst->splineNode.dir[2] = src->splineNode.dir[2];
    dst->angles[0] = src->angles[0];
    dst->angles[1] = src->angles[1];
    dst->angles[2] = src->angles[2];
    dst->splineNode.length = src->splineNode.length;
    *(unsigned int *)&dst->splineNode.nextIdx = *(unsigned int *)&src->splineNode.nextIdx;
}

void __cdecl VP_EndSwitchNode(const vehicle_pathpos_t *vpp)
{
    __int16 nodeIdx; // [esp+0h] [ebp-4h]

    nodeIdx = VP_GetNodeIndex(vpp->switchNode[0].name, 0);
    if ( nodeIdx >= 0 )
        VP_CopyNode(&vpp->switchNode[1], &s_nodes[nodeIdx]);
}

void __cdecl G_VehSetSwitchNode(vehicle_pathpos_t *vpp, __int16 srcNodeIdx, __int16 dstNodeIdx)
{
    vehicle_node_t *srcNode; // [esp+18h] [ebp-8h]
    vehicle_node_t *dstNode; // [esp+1Ch] [ebp-4h]

    G_VehFreePathPos(vpp);
    VP_InitNode(vpp->switchNode, -1);
    VP_InitNode(&vpp->switchNode[1], -1);
    if ( srcNodeIdx >= 0 && dstNodeIdx >= 0 )
    {
        if ( vpp->nodeIdx == srcNodeIdx )
            vpp->frac = 0.0f;
        srcNode = &s_nodes[srcNodeIdx];
        dstNode = &s_nodes[dstNodeIdx];
        VP_CopyNode(srcNode, vpp->switchNode);
        VP_CopyNode(srcNode, &vpp->switchNode[1]);
        vpp->switchNode[0].splineNode.nextIdx = dstNodeIdx;
        vpp->switchNode[0].splineNode.dir[0] = dstNode->origin[0] - srcNode->origin[0];
        vpp->switchNode[0].splineNode.dir[1] = dstNode->origin[1] - srcNode->origin[1];
        vpp->switchNode[0].splineNode.dir[2] = dstNode->origin[2] - srcNode->origin[2];
        vpp->switchNode[0].splineNode.length = Vec3Normalize(vpp->switchNode[0].splineNode.dir);
    }
}

vehicle_node_t *__cdecl G_FindVehicleNode(const SpawnVar *spawnVar)
{
    vehicle_node_t *node; // [esp+Ch] [ebp-64h]
    float dist2; // [esp+10h] [ebp-60h]
    int score; // [esp+14h] [ebp-5Ch]
    int i; // [esp+18h] [ebp-58h]
    int best_score; // [esp+1Ch] [ebp-54h]
    vehicle_node_t compare_node; // [esp+20h] [ebp-50h] BYREF
    vehicle_node_t *best_node; // [esp+6Ch] [ebp-4h]

    VP_InitNode(&compare_node, 0);
    VP_ParseFields(&compare_node, spawnVar, 0);
    best_score = 1;
    best_node = 0;
    for ( i = 0; i < s_numNodes; ++i )
    {
        score = 0;
        node = &s_nodes[i];
        dist2 = Vec3DistanceSq(node->origin, compare_node.origin);
        if ( dist2 < 100.0 )
        {
            score = 1;
            if ( dist2 < 0.5 )
                score = 4;
        }
        if ( node->name && node->name == compare_node.name )
            ++score;
        if ( node->target && node->target == compare_node.target )
            ++score;
        if ( node->target2 && node->target == compare_node.target2 )
            ++score;
        if ( node->script_linkname && node->script_linkname == compare_node.script_linkname )
            ++score;
        if ( node->script_noteworthy && node->script_noteworthy == compare_node.script_noteworthy )
            ++score;
        if ( score > best_score )
        {
            best_node = &s_nodes[i];
            best_score = score;
        }
    }
    return best_node;
}

void __cdecl VP_ParseFields(vehicle_node_t *node, const SpawnVar *spawnVar, bool setScriptVariable)
{
    int i; // [esp+0h] [ebp-4h]

    if ( !spawnVar->spawnVarsValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\g_vehicle_path.cpp",
                    339,
                    0,
                    "%s",
                    "spawnVar.spawnVarsValid") )
    {
        __debugbreak();
    }
    for ( i = 0; i < spawnVar->numSpawnVars; ++i )
        VP_ParseField(spawnVar->spawnVars[i][0], spawnVar->spawnVars[i][1], node, setScriptVariable);
}

void __cdecl VP_ParseField(const char *key, char *value, vehicle_node_t *node, bool setScriptVariable)
{
    float v4; // [esp+4h] [ebp-1Ch]
    vn_field_t *f; // [esp+10h] [ebp-10h]
    float vec[3]; // [esp+14h] [ebp-Ch] BYREF

    for ( f = vn_fields; f->name; ++f )
    {
        if ( !I_stricmp(f->name, key) )
        {
            switch ( f->type )
            {
                case F_INT:
                    *(unsigned int *)((char *)&node->name + f->ofs) = atoi(value);
                    break;
                case F_SHORT:
                    *(unsigned __int16 *)((char *)&node->name + f->ofs) = atoi(value);
                    break;
                case F_BYTE:
                    *((_BYTE *)&node->name + f->ofs) = atoi(value);
                    break;
                case F_FLOAT:
                    v4 = atof(value);
                    *(float *)((char *)&node->name + f->ofs) = v4;
                    break;
                case F_STRING:
                    Scr_SetStringLiveUpdateSafe((unsigned __int16 *)((char *)&node->name + f->ofs), value, SCRIPTINSTANCE_SERVER);
                    break;
                case F_VECTOR:
                    memset(vec, 0, sizeof(vec));
                    sscanf(value, "%f %f %f", vec, &vec[1], &vec[2]);
                    *(float *)((char *)&node->name + f->ofs) = vec[0];
                    *(float *)((char *)&node->target2 + f->ofs) = vec[1];
                    *(float *)((char *)&node->script_noteworthy + f->ofs) = vec[2];
                    break;
                case F_VECTORHACK:
                    memset(vec, 0, sizeof(vec));
                    sscanf(value, "%f %f %f", vec, &vec[1], &vec[2]);
                    *(float *)((char *)&node->name + f->ofs) = AngleNormalize360(vec[1]);
                    break;
                default:
                    return;
            }
            return;
        }
    }
    if ( setScriptVariable )
        VP_SetScriptVariable(key, value, node);
}

void __cdecl VP_SetScriptVariable(const char *key, char *value, vehicle_node_t *node)
{
    int v3; // eax
    const char *v4; // eax
    float v6; // [esp+8h] [ebp-10h]
    unsigned int index; // [esp+10h] [ebp-8h]
    int type; // [esp+14h] [ebp-4h] BYREF

    index = Scr_FindField(key, &type, SCRIPTINSTANCE_SERVER);
    if ( index )
    {
        switch ( type )
        {
            case 2:
                Scr_AddString(value, SCRIPTINSTANCE_SERVER);
                break;
            case 5:
                v6 = atof(value);
                Scr_AddFloat(v6, SCRIPTINSTANCE_SERVER);
                break;
            case 6:
                v3 = atoi(value);
                Scr_AddInt(v3, SCRIPTINSTANCE_SERVER);
                break;
            default:
                v4 = va("VP_SetScriptVariable: bad case %d", type);
                if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\g_vehicle_path.cpp", 249, 0, v4) )
                    __debugbreak();
                return;
        }
        Scr_SetDynamicEntityField(node->index, 3u, index, SCRIPTINSTANCE_SERVER, 0);
    }
}

void __cdecl SP_info_vehicle_node(const SpawnVar *spawnVar, int rotated, int radiantLiveUpdate)
{
    vehicle_node_t *node; // [esp+18h] [ebp-4h]

    if ( s_numNodes >= 2000 )
        Com_Error(ERR_DROP, "Hit max vehicle path node count [%d]", 2000);
    node = &s_nodes[s_numNodes];
    VP_InitNode(node, s_numNodes);
    ++s_numNodes;
    VP_ParseFields(node, spawnVar, 1);
    if ( rotated )
        node->flags |= 0x10000u;
    if ( !node->name && !radiantLiveUpdate && (node->flags & 2) == 0 )
        Com_Error(ERR_DROP, "Vehicle spline path node( %f, %f, %f ) found with no name", node->origin[0], node->origin[1], node->origin[2]);
    if ( node->speed >= 0.0 )
    {
        node->flags |= 0x20000u;
        node->speed = node->speed * 17.6;
    }
    if ( node->lookAhead >= 0.0 )
        node->flags |= 0x40000u;
}

void __cdecl G_ClearSelectedVehicleNode()
{
    g_radiant_selected_node = 0;
}

void __cdecl G_ProcessVehicleNodeCommand(const RadiantCommand *command, SpawnVar *spawnVar)
{
    unsigned int v2; // eax
    vehicle_node_t *node; // [esp+0h] [ebp-10h]
    RadiantCommandType commandType; // [esp+4h] [ebp-Ch]
    char *classname; // [esp+8h] [ebp-8h]
    bool rotated; // [esp+Fh] [ebp-1h]

    node = 0;
    commandType = command->type;
    classname = GetPairValue(spawnVar, "classname");
    rotated = I_strcmp("info_vehicle_node_rotate", classname) == 0;
    if ( commandType == RADIANT_COMMAND_CREATE )
    {
        SP_info_vehicle_node(spawnVar, rotated, 1);
        g_radiant_selected_node = &s_nodes[s_numNodes - 1];
        G_SetupVehiclePaths();
        Com_Printf(5, "Radiant Live Update: Created new vehicle node\n");
        G_AssignGameIdMapping(command->liveUpdateId, g_radiant_selected_node->index);
        AddSavedCommand(command);
    }
    else if ( commandType == RADIANT_COMMAND_DELETE )
    {
        g_radiant_selected_node = G_FindVehicleNode(spawnVar);
        if ( g_radiant_selected_node )
        {
            G_AssignGameIdMapping(command->liveUpdateId, g_radiant_selected_node->index);
            AddSavedCommand(command);
            VP_InitNode(g_radiant_selected_node, -1);
            g_radiant_selected_node = 0;
            G_SetupVehiclePaths();
        }
    }
    else
    {
        if ( commandType )
        {
            if ( commandType == RADIANT_COMMAND_UPDATE_SELECTED )
            {
                node = g_radiant_selected_node;
            }
            else if ( commandType == RADIANT_COMMAND_UPDATE )
            {
                node = G_FindVehicleNode(spawnVar);
            }
        }
        else
        {
            g_radiant_selected_node = G_FindVehicleNode(spawnVar);
            node = g_radiant_selected_node;
            if ( !g_radiant_selected_node )
                Com_Printf(5, "Radiant Live Update: Can't find vehicle node.    Maps out of sync (Radiant/Game), re-bsp!\n");
        }
        if ( node )
        {
            node->speed = -1.0f;
            node->lookAhead = -1.0f;
            node->flags = 0;
            VP_ParseFields(node, spawnVar, 1);
            if ( rotated )
                v2 = node->flags | 0x10000;
            else
                v2 = node->flags & 0xFFFEFFFF;
            node->flags = v2;
            if ( node->speed >= 0.0 )
            {
                node->flags |= 0x20000u;
                node->speed = node->speed * 17.6;
            }
            if ( node->lookAhead >= 0.0 )
                node->flags |= 0x40000u;
            G_SetupVehiclePaths();
            G_AssignGameIdMapping(command->liveUpdateId, node->index);
            AddSavedCommand(command);
        }
    }
}

unsigned __int16 __cdecl GScr_GetVehicleNodeIndex(unsigned int index)
{
    scr_entref_t v2; // [esp+0h] [ebp-18h] BYREF
    scr_entref_t v3; // [esp+Ah] [ebp-Eh]
    scr_entref_t entref; // [esp+10h] [ebp-8h]

    v3 = Scr_GetEntityRef(index, SCRIPTINSTANCE_SERVER);
    entref = v3;
    if ( v3.classnum == 3 )
    {
        if ( entref.entnum >= s_numNodes
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\g_vehicle_path.cpp",
                        2567,
                        0,
                        "%s",
                        "entref.entnum < s_numNodes") )
        {
            __debugbreak();
        }
        return entref.entnum;
    }
    else
    {
        Scr_ParamError(index, "Not a vehicle node", SCRIPTINSTANCE_SERVER);
        return -1;
    }
}

void __cdecl GScr_AddFieldsForVehicleNode()
{
    vn_field_t *f; // [esp+4h] [ebp-4h]

    for ( f = vn_fields; f->name; ++f )
    {
        if ( f - vn_fields != (unsigned __int16)(f - vn_fields)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\g_vehicle_path.cpp",
                        2586,
                        0,
                        "%s",
                        "(f - vn_fields) == (unsigned short)( f - vn_fields )") )
        {
            __debugbreak();
        }
        Scr_AddClassField(3u, (char*)f->name, (unsigned __int16)(f - vn_fields), SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl GScr_GetVehicleNodeField(unsigned int entnum, unsigned int offset)
{
    if ( offset >= 0xB
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\g_vehicle_path.cpp",
                    2601,
                    0,
                    "offset doesn't index ARRAY_COUNT( vn_fields ) - 1\n\t%i not in [0, %i)",
                    offset,
                    11) )
    {
        __debugbreak();
    }
    if ( entnum >= s_numNodes
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\g_vehicle_path.cpp",
                    2602,
                    0,
                    "entnum doesn't index s_numNodes\n\t%i not in [0, %i)",
                    entnum,
                    s_numNodes) )
    {
        __debugbreak();
    }
    GScr_GetGenericField((unsigned __int8 *)&s_nodes[entnum], vn_fields[offset].type, vn_fields[offset].ofs, 0);
}

void __cdecl GScr_GetVehicleNode()
{
    vehicle_node_t *result; // [esp+4h] [ebp-20h]
    unsigned int name; // [esp+Ch] [ebp-18h]
    int offset; // [esp+10h] [ebp-14h]
    char *key; // [esp+14h] [ebp-10h]
    vn_field_t *f; // [esp+18h] [ebp-Ch]
    __int16 i; // [esp+1Ch] [ebp-8h]

    name = Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    key = Scr_GetString(1u, SCRIPTINSTANCE_SERVER);
    offset = Scr_GetOffset(3u, key, SCRIPTINSTANCE_SERVER);
    if ( offset >= 0 )
    {
        if ( (unsigned int)offset >= 0xB
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\g_vehicle_path.cpp",
                        2668,
                        0,
                        "offset doesn't index ARRAY_COUNT( vn_fields ) - 1\n\t%i not in [0, %i)",
                        offset,
                        11) )
        {
            __debugbreak();
        }
        f = &vn_fields[offset];
        if ( f->type != F_STRING )
            Scr_ParamError(1u, "key is not internally a string", SCRIPTINSTANCE_SERVER);
        result = 0;
        for ( i = 0; i < s_numNodes; ++i )
        {
            if ( *(unsigned __int16 *)((char *)&s_nodes[i].name + f->ofs)
                && *(unsigned __int16 *)((char *)&s_nodes[i].name + f->ofs) == name )
            {
                if ( result )
                    Scr_Error("GetVehicleNode used with more than one node", 0);
                result = &s_nodes[i];
            }
        }
        if ( result )
            Scr_AddEntityNum(result->index, 3u, SCRIPTINSTANCE_SERVER, 0);
    }
}

void __cdecl G_SpawnHeliHeightLock(SpawnVar *spawnVar)
{
    float *v1; // [esp+0h] [ebp-24h]
    const char *originString; // [esp+4h] [ebp-20h] BYREF
    int brushmodel; // [esp+8h] [ebp-1Ch]
    float origin[3]; // [esp+Ch] [ebp-18h] BYREF
    const char *modelString; // [esp+18h] [ebp-Ch] BYREF
    const char *targetnameString; // [esp+1Ch] [ebp-8h] BYREF
    int modelIndex; // [esp+20h] [ebp-4h]

    if (num_heli_height_lock_patches <= 0)
    {
        if (num_heli_height_lock_patches >= 32
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\g_vehicle_path.cpp",
                2832,
                0,
                "%s",
                "num_heli_height_lock_patches < MAX_HELI_HEIGHT_LOCK_PATCHES"))
        {
            __debugbreak();
        }
        if (num_heli_height_lock_patches < 32)
        {
            G_SpawnString(spawnVar, "origin", 0, &originString);
            G_SpawnString(spawnVar, "model", 0, &modelString);
            G_SpawnString(spawnVar, "targetname", 0, &targetnameString);
            if (*modelString != 42
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\g_vehicle_path.cpp",
                    2842,
                    0,
                    "%s",
                    "modelString[0] == '*'"))
            {
                __debugbreak();
            }
            modelIndex = atoi(modelString + 1);
            if (modelIndex != (unsigned __int16)modelIndex
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\g_vehicle_path.cpp",
                    2846,
                    0,
                    "%s",
                    "modelIndex == (modelNameIndex_t)modelIndex"))
            {
                __debugbreak();
            }
            brushmodel = (unsigned __int16)modelIndex;
            memset(origin, 0, sizeof(origin));
            sscanf(originString, "%f %f %f", origin, &origin[1], &origin[2]);
            heli_height_lock_patches[num_heli_height_lock_patches].brushmodel = brushmodel;
            if (targetnameString)
                heli_height_lock_patches[num_heli_height_lock_patches].targetname = G_NewString(targetnameString);
            else
                heli_height_lock_patches[num_heli_height_lock_patches].targetname = 0;
            heli_height_lock_patches[num_heli_height_lock_patches].enabled = 1;
            v1 = heli_height_lock_patches[num_heli_height_lock_patches].origin;
            *v1 = origin[0];
            v1[1] = origin[1];
            v1[2] = origin[2];
            ++num_heli_height_lock_patches;
        }
    }
}

