#include "pathnode_load_obj.h"
#include <universal/com_memory.h>
#include "g_load_utils.h"
#include <clientscript/cscr_stringlist.h>
#include <qcommon/com_bsp_load_obj.h>
#include "actor_exposed.h"
#include <game_mp/g_main_mp.h>
#include <bgame/bg_misc.h>
#include <server/sv_game.h>
#include "g_bsp.h"

const float nodeAttackOffset[21] =
{
  0.0,
  0.0,
  0.0,
  0.0,
  0.0,
  0.0,
  -90.0,
  90.0,
  -90.0,
  90.0,
  0.0,
  0.0,
  0.0,
  0.0,
  0.0,
  0.0,
  0.0,
  0.0,
  0.0,
  0.0,
  0.0
};

const int nodeFlagsCleared[21] =
{ 0, 0, 4, 8, 8, 16, 0, 0, 0, 0, 0, 4, 8, 16, 0, 4, 0, 1, 1, 8, 0 };

struct node_field_t_HACK // sizeof=0xC
{                                       // XREF: .data:fields_4/r
    const char *name;
    int ofs;
    int type;
};

node_field_t_HACK fields_4[7] =
{
  { "targetname", 6, 5 },
  { "target", 12, 5 },
  { "animscript", 14, 5 },
  { "script_noteworthy", 10, 5 },
  { "script_linkname", 8, 5 },
  { "origin", 20, 6 },
  { "angles", 32, 15 }
};

struct nodespawn_t // sizeof=0x8
{                                       // XREF: .rdata:nodespawns/r
    const char *name;
    nodeType type;
};

const nodespawn_t nodespawns[21] =
{
  { "node_pathnode", NODE_PATHNODE },
  { "node_cover_stand", NODE_COVER_STAND },
  { "node_cover_crouch", NODE_COVER_CROUCH },
  { "node_cover_crouch_window", NODE_COVER_CROUCH_WINDOW },
  { "node_cover_prone", NODE_COVER_PRONE },
  { "node_cover_right", NODE_COVER_RIGHT },
  { "node_cover_left", NODE_COVER_LEFT },
  { "node_cover_wide_right", NODE_COVER_WIDE_RIGHT },
  { "node_cover_wide_left", NODE_COVER_WIDE_LEFT },
  { "node_cover_pillar", NODE_COVER_PILLAR },
  { "node_concealment_stand", NODE_CONCEALMENT_STAND },
  { "node_concealment_crouch", NODE_CONCEALMENT_CROUCH },
  { "node_concealment_prone", NODE_CONCEALMENT_PRONE },
  { "node_reacquire", NODE_REACQUIRE },
  { "node_balcony", NODE_BALCONY },
  { "node_scripted", NODE_SCRIPTED },
  { "node_negotiation_begin", NODE_NEGOTIATION_BEGIN },
  { "node_negotiation_end", NODE_NEGOTIATION_END },
  { "node_turret", NODE_TURRET },
  { "node_guard", NODE_GUARD },
  { NULL, NODE_BADNODE }
};


struct NodeNums // sizeof=0x10000
{
    int array[2][8192];
};

void __cdecl Path_CreateNodes()
{
    pathstatic.pathLinks = 0;
    pathstatic.indirectNodes = 0;
    pathstatic.pathbuf = 0;
    gameWorldCurrent->path.nodes = (pathnode_t *)Hunk_Alloc(0x100000u, "Path_CreateNodes", 6);
    gameWorldCurrent->path.basenodes = (pathbasenode_t *)Hunk_Alloc(0x20000u, "Path_CreateNodes", 6);
}

void __cdecl Path_InitStatic(int restart)
{
    PathData *p_path; // edx

    if ( restart )
    {
        gameWorldCurrent->path.nodeCount = 0;
    }
    else
    {
        p_path = &gameWorldCurrent->path;
        gameWorldCurrent->path.nodeCount = 0;
        p_path->nodes = 0;
        p_path->basenodes = 0;
        p_path->chainNodeCount = 0;
        p_path->chainNodeForNode = 0;
        p_path->nodeForChainNode = 0;
        p_path->visBytes = 0;
        p_path->pathVis = 0;
        p_path->nodeTreeCount = 0;
        p_path->nodeTree = 0;
    }
    g_pathsError = 0;
}

void __cdecl Path_ShutdownStatic()
{
    gameWorldCurrent->path.nodeCount = 0;
    if ( pathstatic.pathbuf )
    {
        Z_Free((char *)pathstatic.pathbuf, 0);
        pathstatic.pathbuf = 0;
    }
}

void __cdecl G_ParsePathnodeFields(SpawnVar *spawnVar, pathnode_t *node, nodeType type)
{
    const char *v3; // eax
    float forward[3]; // [esp+14h] [ebp-10h] BYREF
    int i; // [esp+20h] [ebp-4h]

    if ( !spawnVar->spawnVarsValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                    328,
                    0,
                    "%s",
                    "spawnVar->spawnVarsValid") )
    {
        __debugbreak();
    }
    node->constant.fAngle = -1.0f;
    for ( i = 0; i < spawnVar->numSpawnVars; ++i )
        G_ParsePathnodeField(spawnVar->spawnVars[i][0], spawnVar->spawnVars[i][1], node);
    if ( node->constant.fAngle == -1.0 )
        node->constant.fAngle = 0.0f;
    else
        node->constant.spawnflags |= 0x8000u;
    YawVectors(node->constant.fAngle, forward, 0);
    *(_QWORD *)node->constant.forward = *(_QWORD *)forward;
    node->constant.spawnflags &= ~(unsigned __int16)nodeFlagsCleared[type];
    node->constant.wOverlapNode[0] = -1;
    node->constant.wOverlapNode[1] = -1;
    if ( (~(unsigned __int8)node->constant.spawnflags & 0x1C) == 0 )
    {
        v3 = vtos(node->constant.vOrigin);
        Com_PrintError(18, "node at %s has no allowed stances; allowing all instead\n", v3);
        node->constant.spawnflags &= 0xFFE3u;
    }
    if ( type >= NODE_NEGOTIATION_BEGIN )
    {
        if ( type <= NODE_NEGOTIATION_END )
        {
            node->constant.spawnflags |= 2u;
        }
        else if ( type == NODE_TURRET )
        {
            node->constant.spawnflags |= 0x40u;
        }
    }
    node->constant.type = type;
}

void __cdecl G_ParsePathnodeField(const char *key, char *value, pathnode_t *node)
{
    float v3; // [esp+4h] [ebp-1Ch]
    node_field_t_HACK *f; // [esp+10h] [ebp-10h]
    float vec[3]; // [esp+14h] [ebp-Ch] BYREF

    for (f = fields_4; f->name; ++f)
    {
        if (!I_stricmp(f->name, key))
        {
            switch (f->type)
            {
            case 0:
                *(nodeType *)((char *)&node->constant.type + f->ofs) = (nodeType)atoi(value);
                break;
            case 1:
                *(_WORD *)((char *)&node->constant.type + f->ofs) = atoi(value);
                break;
            case 2:
                *((_BYTE *)&node->constant.type + f->ofs) = atoi(value);
                break;
            case 3:
                v3 = atof(value);
                *(float *)((char *)&node->constant.type + f->ofs) = v3;
                break;
            case 5:
                Scr_SetStringLiveUpdateSafe((unsigned __int16 *)((char *)node + f->ofs), value, SCRIPTINSTANCE_SERVER);
                break;
            case 6:
                memset(vec, 0, sizeof(vec));
                sscanf(value, "%f %f %f", vec, &vec[1], &vec[2]);
                *(float *)((char *)&node->constant.type + f->ofs) = vec[0];
                *(float *)((char *)&node->constant.spawnflags + f->ofs) = vec[1];
                *(float *)((char *)&node->constant.script_linkName + f->ofs) = vec[2];
                break;
            case 0xF:
                memset(vec, 0, sizeof(vec));
                sscanf(value, "%f %f %f", vec, &vec[1], &vec[2]);
                *(float *)((char *)&node->constant.type + f->ofs) = AngleNormalize360(vec[1]);
                break;
            default:
                return;
            }
            return;
        }
    }
}

int __cdecl G_GetNodeTypeFromClassname(const char *classname)
{
    const nodespawn_t *ns; // [esp+14h] [ebp-4h]

    for ( ns = nodespawns; ns->name; ++ns )
    {
        if ( !strcmp(ns->name, classname) )
            return ns->type;
    }
    Com_Error(ERR_DROP, "%s doesn't have a spawn function", classname);
    return 1;
}

void __cdecl G_SpawnPathnodeStatic(SpawnVar *spawnVar, const char *classname)
{
    nodeType NodeTypeFromClassname; // eax

    NodeTypeFromClassname = (nodeType)G_GetNodeTypeFromClassname(classname);
    SP_spawn_node(spawnVar, NodeTypeFromClassname);
}

int overCount_0;
void __cdecl SP_spawn_node(SpawnVar *spawnVar, nodeType type)
{
    pathnode_t *loadNode; // [esp+0h] [ebp-4h]

    if ( type >= NODE_NUMTYPES
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                    384,
                    0,
                    "%s",
                    "type < NODE_NUMTYPES") )
    {
        __debugbreak();
    }
    if ( !gameWorldCurrent->path.nodes
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                    385,
                    0,
                    "%s",
                    "gameWorldCurrent.path.nodes") )
    {
        __debugbreak();
    }
    if ( gameWorldCurrent->path.nodeCount < 0x1F80 )
    {
        loadNode = &gameWorldCurrent->path.nodes[gameWorldCurrent->path.nodeCount];
        memset(loadNode, 0, sizeof(pathnode_t));
        if ( gameWorldCurrent->path.nodeCount != LOWORD(gameWorldCurrent->path.nodeCount)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                        399,
                        0,
                        "%s",
                        "gameWorldCurrent.path.nodeCount == (unsigned short)gameWorldCurrent.path.nodeCount") )
        {
            __debugbreak();
        }
        G_ParsePathnodeFields(spawnVar, loadNode, type);
        ++gameWorldCurrent->path.nodeCount;
    }
    else
    {
        Com_Printf(
            18,
            "PATH_MAX_NODES (%i) exceeded.    Nodecount: %d\n",
            0x2000,
            ++overCount_0 + gameWorldCurrent->path.nodeCount);
    }
}

void __cdecl Scr_FreePathnodeFields(pathnode_t *node)
{
    node_field_t_HACK *f; // [esp+8h] [ebp-4h]

    for (f = fields_4; f->name; ++f)
    {
        if (f->type == 5)
            Scr_SetString((unsigned __int16 *)((char *)node + f->ofs), 0, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl Path_BuildChains()
{
    char *v0; // eax
    char *v1; // eax
    char *v2; // eax
    unsigned __int16 j; // [esp+18h] [ebp-1Ch]
    unsigned __int16 ja; // [esp+18h] [ebp-1Ch]
    pathnode_t *node; // [esp+1Ch] [ebp-18h]
    pathnode_t *nodea; // [esp+1Ch] [ebp-18h]
    pathnode_t *nodeb; // [esp+1Ch] [ebp-18h]
    pathnode_t *nodec; // [esp+1Ch] [ebp-18h]
    __int16 wChainId; // [esp+20h] [ebp-14h]
    unsigned int chainNodeCount; // [esp+24h] [ebp-10h]
    unsigned int badNodeCount; // [esp+28h] [ebp-Ch]
    unsigned __int16 i; // [esp+2Ch] [ebp-8h]
    unsigned __int16 ia; // [esp+2Ch] [ebp-8h]
    unsigned __int16 ib; // [esp+2Ch] [ebp-8h]
    unsigned __int16 ic; // [esp+2Ch] [ebp-8h]
    unsigned __int16 id; // [esp+2Ch] [ebp-8h]
    unsigned __int16 ie; // [esp+2Ch] [ebp-8h]
    pathnode_t *pNodeParent; // [esp+30h] [ebp-4h]

    if ( !gameWorldCurrent->path.nodeForChainNode )
        gameWorldCurrent->path.nodeForChainNode = (unsigned __int16 *)Hunk_Alloc(0x4000u, "Path_BuildChains", 6);
    if ( !gameWorldCurrent->path.chainNodeForNode )
        gameWorldCurrent->path.chainNodeForNode = (unsigned __int16 *)Hunk_Alloc(0x4000u, "Path_BuildChains", 6);
    for ( i = 0; i < gameWorldCurrent->path.nodeCount; ++i )
    {
        node = &gameWorldCurrent->path.nodes[i];
        node->constant.wChainParent = -1;
        if ( node->constant.targetname && (node->constant.spawnflags & 2) == 0 )
        {
            for ( j = 0; j < gameWorldCurrent->path.nodeCount; ++j )
            {
                pNodeParent = &gameWorldCurrent->path.nodes[j];
                if ( pNodeParent->constant.target == node->constant.targetname )
                {
                    if ( node->constant.wChainParent >= 0 )
                    {
                        v0 = SL_ConvertToString(node->constant.targetname, SCRIPTINSTANCE_SERVER);
                        Com_Error(ERR_DROP, "multiple nodes target node with name %s", v0);
                    }
                    if ( i == j )
                    {
                        v1 = SL_ConvertToString(node->constant.targetname, SCRIPTINSTANCE_SERVER);
                        Com_Error(ERR_DROP, "node with name %s targets itself", v1);
                    }
                    node->constant.wChainParent = j;
                    pNodeParent->constant.wChainId = -1;
                }
            }
        }
    }
    for ( ia = 0; ia < gameWorldCurrent->path.nodeCount; ++ia )
    {
        nodea = &gameWorldCurrent->path.nodes[ia];
        for ( ja = 0; nodea->constant.wChainParent >= 0 && ja < gameWorldCurrent->path.nodeCount; ++ja )
        {
            if ( nodea->constant.wChainParent == ia )
            {
                v2 = SL_ConvertToString(nodea->constant.targetname, SCRIPTINSTANCE_SERVER);
                Com_Error(ERR_DROP, "found chain loop including node with name %s", v2);
            }
            nodea = &gameWorldCurrent->path.nodes[nodea->constant.wChainParent];
        }
    }
    wChainId = 1;
    for ( ib = 0; ib < gameWorldCurrent->path.nodeCount; ++ib )
    {
        nodeb = &gameWorldCurrent->path.nodes[ib];
        if ( nodeb->constant.wChainId < 0 && nodeb->constant.wChainParent < 0 )
        {
            if ( (nodeb->constant.spawnflags & 2) != 0 )
                Path_BuildChain_r(nodeb, 0, 0);
            else
                Path_BuildChain_r(nodeb, wChainId++, 1);
        }
        gameWorldCurrent->path.nodeForChainNode[ib] = ib;
    }
    for ( ic = 0; ic < gameWorldCurrent->path.nodeCount; ++ic )
    {
        if ( gameWorldCurrent->path.nodes[ic].constant.wChainId < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                        634,
                        0,
                        "%s",
                        "gameWorldCurrent.path.nodes[i].constant.wChainId >= 0") )
        {
            __debugbreak();
        }
    }
    qsort(
        gameWorldCurrent->path.nodeForChainNode,
        gameWorldCurrent->path.nodeCount,
        2u,
        (int (__cdecl *)(const void *, const void *))compare_pathnodes);
    for ( id = 0; id < gameWorldCurrent->path.nodeCount; ++id )
        gameWorldCurrent->path.chainNodeForNode[gameWorldCurrent->path.nodeForChainNode[id]] = id;
    badNodeCount = 0;
    for ( ie = 0; ie < gameWorldCurrent->path.nodeCount; ++ie )
    {
        nodec = &gameWorldCurrent->path.nodes[gameWorldCurrent->path.nodeForChainNode[ie]];
        if ( !nodec->constant.wChainId )
            break;
        if ( nodec->constant.type == NODE_BADNODE )
        {
            ++badNodeCount;
            Com_Error(
                ERR_DROP,
                "Error: bad node [%d] in friendly chain at (%g %g %g)",
                ie,
                nodec->constant.vOrigin[0],
                nodec->constant.vOrigin[1],
                nodec->constant.vOrigin[2]);
        }
    }
    chainNodeCount = ie;
    while ( ie < gameWorldCurrent->path.nodeCount )
    {
        if ( gameWorldCurrent->path.nodes[gameWorldCurrent->path.nodeForChainNode[ie]].constant.wChainId
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                        662,
                        0,
                        "%s",
                        "node->constant.wChainId == 0") )
        {
            __debugbreak();
        }
        ++ie;
    }
    if ( badNodeCount )
        Path_ErasePaths("\n\nBad node in friendly chain\n");
    gameWorldCurrent->path.chainNodeCount = chainNodeCount;
}

void __cdecl Path_BuildChain_r(pathnode_t *node, __int16 wChainId, __int16 wChainDepth)
{
    char *v3; // eax
    unsigned int i; // [esp+4h] [ebp-4h]

    node->constant.wChainId = wChainId;
    node->constant.wChainDepth = wChainDepth;
    if ( node->constant.target )
    {
        for ( i = 0; i < gameWorldCurrent->path.nodeCount; ++i )
        {
            if ( gameWorldCurrent->path.nodes[i].constant.targetname == node->constant.target )
            {
                if ( node == &gameWorldCurrent->path.nodes[i] )
                {
                    v3 = SL_ConvertToString(node->constant.targetname, SCRIPTINSTANCE_SERVER);
                    Com_Error(ERR_DROP, "node %s targets itself", v3);
                }
                if ( (gameWorldCurrent->path.nodes[i].constant.spawnflags & 2) == 0 )
                {
                    if ( wChainDepth )
                        Path_BuildChain_r(&gameWorldCurrent->path.nodes[i], wChainId, wChainDepth + 1);
                    else
                        Path_BuildChain_r(&gameWorldCurrent->path.nodes[i], wChainId, 0);
                }
            }
        }
    }
}

int __cdecl compare_pathnodes(unsigned __int16 *e0, unsigned __int16 *e1)
{
    int index0; // [esp+4h] [ebp-10h]
    const pathnode_t *node0; // [esp+8h] [ebp-Ch]
    int index1; // [esp+Ch] [ebp-8h]
    const pathnode_t *node1; // [esp+10h] [ebp-4h]

    index0 = *e0;
    index1 = *e1;
    node0 = &gameWorldCurrent->path.nodes[index0];
    node1 = &gameWorldCurrent->path.nodes[index1];
    if ( node1->constant.wChainId != node0->constant.wChainId )
        return node1->constant.wChainId - node0->constant.wChainId;
    if ( node0->constant.wChainDepth != node1->constant.wChainDepth )
        return node0->constant.wChainDepth - node1->constant.wChainDepth;
    if ( node0->constant.wChainParent != node1->constant.wChainParent )
        return node0->constant.wChainParent - node1->constant.wChainParent;
    if ( node1->constant.type == node0->constant.type )
        return index0 - index1;
    return node1->constant.type - node0->constant.type;
}

void __cdecl Path_ErasePaths(const char *pathsError)
{
    pathnode_t *node; // [esp+0h] [ebp-8h]
    unsigned int i; // [esp+4h] [ebp-4h]

    if ( !g_pathsError )
    {
        g_pathsError = pathsError;
        for ( i = 0; i < gameWorldCurrent->path.nodeCount; ++i )
        {
            node = &gameWorldCurrent->path.nodes[i];
            node->constant.totalLinkCount = 0;
            node->dynamic.wLinkCount = 0;
            node->constant.wOverlapNode[0] = -1;
            node->constant.wOverlapNode[1] = -1;
            node->constant.wChainParent = -1;
            node->constant.wChainId = 0;
            node->constant.wChainDepth = 0;
        }
    }
}

bool __cdecl Path_FindOverlappingNodes()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v4; // [esp+28h] [ebp-48h]
    const char *v5; // [esp+28h] [ebp-48h]
    const char *v6; // [esp+2Ch] [ebp-44h]
    const char *v7; // [esp+2Ch] [ebp-44h]
    const char *v8; // [esp+30h] [ebp-40h]
    const char *v9; // [esp+30h] [ebp-40h]
    unsigned int j; // [esp+3Ch] [ebp-34h]
    pathnode_t *node; // [esp+40h] [ebp-30h]
    int iErrorCount; // [esp+48h] [ebp-28h]
    unsigned int i; // [esp+4Ch] [ebp-24h]
    unsigned int ia; // [esp+4Ch] [ebp-24h]
    float actorSize_8; // [esp+58h] [ebp-18h]
    float distSq; // [esp+5Ch] [ebp-14h]
    pathnode_t *otherNode; // [esp+60h] [ebp-10h]
    float vDelta; // [esp+64h] [ebp-Ch]
    float vDelta_4; // [esp+68h] [ebp-8h]
    float vDelta_8; // [esp+6Ch] [ebp-4h]

    for ( i = 0; i < gameWorldCurrent->path.nodeCount; ++i )
    {
        gameWorldCurrent->path.nodes[i].constant.wOverlapNode[0] = -1;
        gameWorldCurrent->path.nodes[i].constant.wOverlapNode[1] = -1;
    }
    actorSize_8 = (float)(48.0 - 0.0) + 1.0;
    iErrorCount = 0;
    for ( ia = 0; ia < gameWorldCurrent->path.nodeCount; ++ia )
    {
        node = &gameWorldCurrent->path.nodes[ia];
        for ( j = 0; j < ia; ++j )
        {
            otherNode = &gameWorldCurrent->path.nodes[j];
            vDelta = node->constant.vOrigin[0] - otherNode->constant.vOrigin[0];
            vDelta_4 = node->constant.vOrigin[1] - otherNode->constant.vOrigin[1];
            vDelta_8 = node->constant.vOrigin[2] - otherNode->constant.vOrigin[2];
            distSq = (float)(vDelta * vDelta) + (float)(vDelta_4 * vDelta_4);
            if ( distSq <= (float)((float)((float)(15.0 - -15.0) + 1.0) * (float)((float)(15.0 - -15.0) + 1.0))
                && (-(actorSize_8)) <= vDelta_8
                && vDelta_8 <= actorSize_8 )
            {
                if ( distSq < 1.0
                    && (((unsigned __int8)otherNode->constant.spawnflags | (unsigned __int8)node->constant.spawnflags) & 1) == 0 )
                {
                    Com_PrintError(
                        18,
                        "ERROR:    Duplicate linking nodes at %d: %f %f %f vs. %d: %f %f %f.    Removed dupe node or set DONTLINK.\n",
                        ia,
                        node->constant.vOrigin[0],
                        node->constant.vOrigin[1],
                        node->constant.vOrigin[2],
                        j,
                        otherNode->constant.vOrigin[0],
                        otherNode->constant.vOrigin[1],
                        otherNode->constant.vOrigin[2]);
                    ++iErrorCount;
                }
                if ( node->constant.type == NODE_NEGOTIATION_BEGIN || node->constant.type == NODE_NEGOTIATION_END )
                {
                    v0 = vtosf(node->constant.vOrigin);
                    Com_PrintWarning(18, "WARNING: NEGOTIATION node at %s overlaps other nodes\n", v0);
                }
                if ( node->constant.wOverlapNode[0] >= 0 )
                {
                    if ( node->constant.wOverlapNode[1] >= 0 )
                    {
                        v8 = vtosf(otherNode->constant.vOrigin);
                        v6 = vtosf(gameWorldCurrent->path.nodes[node->constant.wOverlapNode[1]].constant.vOrigin);
                        v4 = vtosf(gameWorldCurrent->path.nodes[node->constant.wOverlapNode[0]].constant.vOrigin);
                        v1 = vtosf(node->constant.vOrigin);
                        Com_PrintWarning(18, "WARNING: node at %s overlaps more than 2 other nodes %s %s %s\n", v1, v4, v6, v8);
                        ++iErrorCount;
                    }
                    else
                    {
                        node->constant.wOverlapNode[1] = j;
                    }
                }
                else
                {
                    node->constant.wOverlapNode[0] = j;
                }
                if ( otherNode->constant.wOverlapNode[0] >= 0 )
                {
                    if ( otherNode->constant.wOverlapNode[1] >= 0 )
                    {
                        v9 = vtosf(node->constant.vOrigin);
                        v7 = vtosf(gameWorldCurrent->path.nodes[otherNode->constant.wOverlapNode[1]].constant.vOrigin);
                        v5 = vtosf(gameWorldCurrent->path.nodes[otherNode->constant.wOverlapNode[0]].constant.vOrigin);
                        v2 = vtosf(otherNode->constant.vOrigin);
                        Com_PrintWarning(18, "WARNING: node at %s overlaps more than 2 other nodes %s %s %s\n", v2, v5, v7, v9);
                        ++iErrorCount;
                    }
                    else
                    {
                        otherNode->constant.wOverlapNode[1] = ia;
                    }
                }
                else
                {
                    otherNode->constant.wOverlapNode[0] = ia;
                }
            }
        }
    }
    if ( iErrorCount )
        Path_ErasePaths("\n\nDuplicate/overlap node errors.    Check log for list to fix.\n");
    return iErrorCount > 0;
}

char str_0[8][64];
int index_0;
char *__cdecl vtosf(const float *v)
{
    char *s; // [esp+18h] [ebp-4h]

    s = str_0[index_0];
    index_0 = ((_BYTE)index_0 + 1) & 7;
    Com_sprintf(s, 0x40u, "(%f %f %f)", *v, v[1], v[2]);
    return s;
}

void __cdecl Path_LoadPaths()
{
    if ( !Path_LoadPathsInternal() )
        Path_ErasePaths(
            "\n"
            "\n"
            "Map does not have paths generated.\n"
            "Run the game with \"g_connectpaths 1\" to generate paths.\n"
            "Run the game with \"g_connectpaths 2\" to generate paths and auto quit.\n");
}

int __cdecl Path_LoadPathsInternal()
{
    float v1; // [esp+0h] [ebp-40h]
    unsigned __int16 v2; // [esp+4h] [ebp-3Ch]
    unsigned __int16 v3; // [esp+8h] [ebp-38h]
    const unsigned __int8 *pos; // [esp+14h] [ebp-2Ch]
    const unsigned __int8 *posa; // [esp+14h] [ebp-2Ch]
    const unsigned __int8 *posb; // [esp+14h] [ebp-2Ch]
    int j; // [esp+18h] [ebp-28h]
    int ja; // [esp+18h] [ebp-28h]
    unsigned int jb; // [esp+18h] [ebp-28h]
    pathlink_s *link; // [esp+1Ch] [ebp-24h]
    unsigned int totalLinkCount; // [esp+20h] [ebp-20h]
    pathlink_s *linksBuffer; // [esp+24h] [ebp-1Ch]
    const unsigned __int8 *buf; // [esp+28h] [ebp-18h]
    unsigned __int8 *visbuf; // [esp+2Ch] [ebp-14h]
    unsigned int len; // [esp+30h] [ebp-10h] BYREF
    unsigned int i; // [esp+34h] [ebp-Ch]
    pathnode_t *nodeFrom; // [esp+38h] [ebp-8h]
    unsigned int visBytes; // [esp+3Ch] [ebp-4h]

    if ( !gameWorldCurrent->path.nodeCount )
        return 1;
    buf = (const unsigned __int8 *)Com_LoadBspLump((char *)gameWorldCurrent->name, LUMP_PATHCONNECTIONS, 1u, &len);
    if ( !len )
        return 0;
    if ( *(unsigned int *)buf == 8 )
    {
        pos = buf + 6;
        if ( gameWorldCurrent->path.nodeCount != *((unsigned __int16 *)buf + 2) )
            Com_Error(ERR_DROP, "Path node count mismatch in bsp file. Recompile the map to fix.");
        linksBuffer = (pathlink_s *)Z_Malloc(0x600000, "Path_LoadPaths", 6);
        totalLinkCount = 0;
        for ( i = 0; i < gameWorldCurrent->path.nodeCount; ++i )
        {
            nodeFrom = &gameWorldCurrent->path.nodes[i];
            v3 = *(_WORD *)pos;
            pos += 2;
            nodeFrom->constant.totalLinkCount = v3;
            for ( j = 0; j < nodeFrom->constant.totalLinkCount; ++j )
            {
                link = &linksBuffer[j + totalLinkCount];
                if ( j
                    && link <= &linksBuffer[totalLinkCount - 1 + j]
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                                1067,
                                0,
                                "%s",
                                "!j || link > &linksBuffer[totalLinkCount + (j - 1)]") )
                {
                    __debugbreak();
                }
                v2 = *(_WORD *)pos;
                posa = pos + 2;
                link->nodeNum = v2;
                v1 = *(float *)posa;
                pos = posa + 4;
                link->fDist = v1;
                link->disconnectCount = 0;
                link->negotiationLink = Path_IsNegotiationLink(nodeFrom, &gameWorldCurrent->path.nodes[link->nodeNum]);
            }
            totalLinkCount += nodeFrom->constant.totalLinkCount;
        }
        visBytes = (gameWorldCurrent->path.nodeCount * (gameWorldCurrent->path.nodeCount - 1) + 7) >> 3;
        i = 0;
        visbuf = (unsigned __int8 *)Z_Malloc(visBytes, "Path_LoadPaths", 6);
        while ( i < visBytes )
        {
            if ( (*pos & 0x80) != 0 )
            {
                ja = (unsigned __int8)~*pos++;
                do
                {
                    visbuf[i++] = *pos++;
                    --ja;
                }
                while ( ja );
            }
            else
            {
                jb = *pos;
                posb = pos + 1;
                memset(&visbuf[i], 0, jb);
                i += jb;
                visbuf[i++] = *posb;
                pos = posb + 1;
            }
        }
        if ( i != visBytes
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                        1105,
                        0,
                        "%s",
                        "i == visBytes") )
        {
            __debugbreak();
        }
        Path_MakePathDataPermanent(linksBuffer, totalLinkCount, visbuf, visBytes);
        Z_Free((char *)visbuf, 6);
        Z_Free((char *)linksBuffer, 6);
        Com_UnloadBspLump(LUMP_PATHCONNECTIONS);
        return 1;
    }
    else
    {
        Com_UnloadBspLump(LUMP_PATHCONNECTIONS);
        return 0;
    }
}

bool __cdecl Path_IsNegotiationLink(pathnode_t *pNodeFrom, pathnode_t *pNodeTo)
{
    return pNodeFrom->constant.type == NODE_NEGOTIATION_BEGIN
            && pNodeTo->constant.type == NODE_NEGOTIATION_END
            && pNodeFrom->constant.target == pNodeTo->constant.targetname;
}

void __cdecl Path_MakePathDataPermanent(
                pathlink_s *pSourceLinks,
                unsigned int totalLinkCount,
                unsigned __int8 *pVis,
                int visBytes)
{
    unsigned __int8 *visbuf; // [esp+0h] [ebp-10h]
    unsigned int accountedLinks; // [esp+4h] [ebp-Ch]
    unsigned int i; // [esp+8h] [ebp-8h]
    unsigned int ia; // [esp+8h] [ebp-8h]
    unsigned int ib; // [esp+8h] [ebp-8h]
    int totalBytes; // [esp+Ch] [ebp-4h]

    totalBytes = 12 * totalLinkCount;
    if ( !pathstatic.pathLinks )
        pathstatic.pathLinks = (pathlink_s *)Hunk_Alloc(totalBytes, "Path_MakeLinksPermanent", 6);
    memcpy((unsigned __int8 *)pathstatic.pathLinks, (unsigned __int8 *)pSourceLinks, totalBytes);
    for ( i = 0; i < totalLinkCount; ++i )
    {
        if ( pathstatic.pathLinks[i].negotiationLink != (pathstatic.pathLinks[i].negotiationLink != 0)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                        960,
                        0,
                        "%s\n\t(pathstatic.pathLinks[i].negotiationLink) = %i",
                        "(pathstatic.pathLinks[i].negotiationLink == (pathstatic.pathLinks[i].negotiationLink != 0))",
                        pathstatic.pathLinks[i].negotiationLink) )
        {
            __debugbreak();
        }
    }
    accountedLinks = 0;
    for ( ia = 0; ia < gameWorldCurrent->path.nodeCount; ++ia )
    {
        if ( gameWorldCurrent->path.nodes[ia].constant.totalLinkCount )
        {
            gameWorldCurrent->path.nodes[ia].constant.Links = &pathstatic.pathLinks[accountedLinks];
            accountedLinks += gameWorldCurrent->path.nodes[ia].constant.totalLinkCount;
        }
    }
    if ( accountedLinks != totalLinkCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                    973,
                    0,
                    "accountedLinks == totalLinkCount\n\t%i, %i",
                    accountedLinks,
                    totalLinkCount) )
    {
        __debugbreak();
    }
    if ( pVis )
    {
        visbuf = 0;
    }
    else
    {
        visBytes = (gameWorldCurrent->path.nodeCount * (gameWorldCurrent->path.nodeCount - 1) + 7) >> 3;
        visbuf = (unsigned __int8 *)Z_Malloc(visBytes, "Path_MakePathDataPermanent", 0);
        Path_BuildNodeVis(visbuf, visBytes);
        pVis = visbuf;
    }
    if ( !gameWorldCurrent->path.pathVis )
    {
        gameWorldCurrent->path.visBytes = visBytes;
        gameWorldCurrent->path.pathVis = Hunk_Alloc(visBytes, "VisData", 6);
    }
    memcpy(gameWorldCurrent->path.pathVis, pVis, visBytes);
    if ( visbuf )
        Z_Free((char *)visbuf, 0);
    if ( !pathstatic.indirectNodes )
    {
        pathstatic.indirectNodes = (unsigned __int16 *)Hunk_Alloc(
                                                                                                         2 * gameWorldCurrent->path.nodeCount,
                                                                                                         "Path_MakeNodeBsp",
                                                                                                         6);
        for ( ib = 0; ib < gameWorldCurrent->path.nodeCount; ++ib )
            pathstatic.indirectNodes[ib] = ib;
        gameWorldCurrent->path.nodeTree = Path_BuildNodeBsp_r(pathstatic.indirectNodes, gameWorldCurrent->path.nodeCount);
    }
}

pathnode_tree_t *__cdecl Path_BuildNodeBsp_r(unsigned __int16 *nodes, int numNodes)
{
    pathnode_tree_t *result; // eax
    pathnode_t *nodea; // [esp+8h] [ebp-38h]
    pathnode_t *node; // [esp+8h] [ebp-38h]
    float mins[2]; // [esp+10h] [ebp-30h]
    float dist; // [esp+18h] [ebp-28h]
    int right; // [esp+1Ch] [ebp-24h]
    int left; // [esp+20h] [ebp-20h]
    float maxs[2]; // [esp+24h] [ebp-1Ch]
    unsigned __int16 swapNode; // [esp+2Ch] [ebp-14h]
    pathnode_tree_t *child[2]; // [esp+30h] [ebp-10h]
    int i; // [esp+38h] [ebp-8h]
    int axis; // [esp+3Ch] [ebp-4h]

    if ( numNodes >= 4 )
    {
        nodea = &gameWorldCurrent->path.nodes[*nodes];
        maxs[0] = nodea->constant.vOrigin[0];
        mins[0] = maxs[0];
        maxs[1] = nodea->constant.vOrigin[1];
        mins[1] = maxs[1];
        for ( i = 1; i < numNodes; ++i )
        {
            node = &gameWorldCurrent->path.nodes[nodes[i]];
            if ( mins[0] <= node->constant.vOrigin[0] )
            {
                if ( node->constant.vOrigin[0] > maxs[0] )
                    maxs[0] = node->constant.vOrigin[0];
            }
            else
            {
                mins[0] = node->constant.vOrigin[0];
            }
            if ( mins[1] <= node->constant.vOrigin[1] )
            {
                if ( node->constant.vOrigin[1] > maxs[1] )
                    maxs[1] = node->constant.vOrigin[1];
            }
            else
            {
                mins[1] = node->constant.vOrigin[1];
            }
        }
        axis = (float)(maxs[1] - mins[1]) > (float)(maxs[0] - mins[0]);
        if ( (float)(maxs[axis] - mins[axis]) > 192.0 )
        {
            dist = (float)(maxs[axis] + mins[axis]) * 0.5;
            left = 0;
            for ( right = numNodes - 1; ; --right )
            {
                if ( left >= numNodes
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                                899,
                                0,
                                "%s",
                                "left < numNodes") )
                {
                    __debugbreak();
                }
                if ( right < 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                                900,
                                0,
                                "%s",
                                "right >= 0") )
                {
                    __debugbreak();
                }
                while ( dist > gameWorldCurrent->path.nodes[nodes[left]].constant.vOrigin[axis] )
                {
                    if ( ++left >= numNodes
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                                    905,
                                    0,
                                    "%s",
                                    "left < numNodes") )
                    {
                        __debugbreak();
                    }
                }
                while ( gameWorldCurrent->path.nodes[nodes[right]].constant.vOrigin[axis] > dist )
                {
                    if ( --right < 0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                                    910,
                                    0,
                                    "%s",
                                    "right >= 0") )
                    {
                        __debugbreak();
                    }
                }
                if ( left >= right )
                    break;
                swapNode = nodes[left];
                nodes[left] = nodes[right];
                nodes[right] = swapNode;
                ++left;
            }
            while ( 2 * left < numNodes && gameWorldCurrent->path.nodes[nodes[left]].constant.vOrigin[axis] == dist )
                ++left;
            while ( 2 * left > numNodes && gameWorldCurrent->path.nodes[nodes[left - 1]].constant.vOrigin[axis] == dist )
                --left;
            child[0] = Path_BuildNodeBsp_r(nodes, left);
            child[1] = Path_BuildNodeBsp_r(&nodes[left], numNodes - left);
            result = Path_AllocPathnodeTree();
            result->axis = axis;
            result->dist = dist;
            result->u = *(pathnode_tree_info_t *)child;
        }
        else
        {
            result = Path_AllocPathnodeTree();
            result->axis = -1;
            result->u.s.nodeCount = numNodes;
            result->u.child[1] = (pathnode_tree_t *)nodes;
        }
    }
    else
    {
        result = Path_AllocPathnodeTree();
        result->axis = -1;
        result->u.s.nodeCount = numNodes;
        result->u.child[1] = (pathnode_tree_t *)nodes;
    }
    return result;
}

pathnode_tree_t *__cdecl Path_AllocPathnodeTree()
{
    ++gameWorldCurrent->path.nodeTreeCount;
    return (pathnode_tree_t *)Hunk_AllocAlign(0x10u, 4, "Path_AllocPathnodeTree", 6);
}

int __cdecl ExpandedNodeVisCacheEntry_0(int i, int j)
{
    if (i <= j
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp", 1428, 0, "%s", "i > j"))
    {
        __debugbreak();
    }
    return j + gameWorldCurrent->path.nodeCount * (i - 1);
}

int __cdecl NodeVisCacheEntry_0(int i, int j)
{
    if (i >= j
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp", 1416, 0, "%s", "i < j"))
    {
        __debugbreak();
    }
    return j + gameWorldCurrent->path.nodeCount * i;
}

void __cdecl Path_BuildNodeVis(unsigned __int8 *visbuf, unsigned int iVisCacheSize)
{
    int v2; // eax
    unsigned int j; // [esp+Ch] [ebp-44h]
    unsigned int ja; // [esp+Ch] [ebp-44h]
    int entrya; // [esp+10h] [ebp-40h]
    int entry; // [esp+10h] [ebp-40h]
    NodeNums *nodeNums; // [esp+14h] [ebp-3Ch]
    float start[3]; // [esp+1Ch] [ebp-34h] BYREF
    float end[3]; // [esp+28h] [ebp-28h] BYREF
    int nodeCount[2]; // [esp+34h] [ebp-1Ch] BYREF
    int totalCount; // [esp+3Ch] [ebp-14h]
    int prevPercent; // [esp+40h] [ebp-10h]
    unsigned int i; // [esp+44h] [ebp-Ch]
    int count; // [esp+4Ch] [ebp-4h]

    Com_Printf(18, "Building node vis...\n");
    memset(visbuf, 0, iVisCacheSize);
    totalCount = 2 * gameWorldCurrent->path.nodeCount;
    count = 0;
    prevPercent = 0;
    nodeNums = (NodeNums *)Hunk_AllocateTempMemory(0x10000, "Path_BuildNodeVis");
    if ( !nodeNums
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp", 1529, 0, "%s", "nodeNums") )
    {
        __debugbreak();
    }
    for ( i = 0; i < gameWorldCurrent->path.nodeCount; ++i )
    {
        GetNodeAttackPoint(&gameWorldCurrent->path.nodes[i], start);
        for ( j = i + 1; j < gameWorldCurrent->path.nodeCount; ++j )
        {
            GetNodeAttackPoint(&gameWorldCurrent->path.nodes[j], end);
            if ( !CM_BoxSightTrace(0, start, end, vec3_origin, vec3_origin, 0, 0x2809803) )
            {
                v2 = NodeVisCacheEntry_0(i, j);
                visbuf[v2 >> 3] |= 1 << (v2 & 7);
                entrya = ExpandedNodeVisCacheEntry_0(j, i);
                visbuf[entrya >> 3] |= 1 << (entrya & 7);
            }
        }
        ++count;
        if ( 5 * (20 * count / totalCount) != prevPercent )
        {
            prevPercent = 5 * (20 * count / totalCount);
            Com_Printf(18, "%d%%\n", prevPercent);
        }
    }
    for ( i = 0; i < gameWorldCurrent->path.nodeCount; ++i )
    {
        nodeNums->array[0][0] = i;
        nodeCount[0] = Path_GetExpandedNodeNums_r(i, 256.0, (int *)nodeNums, 1);
        for ( ja = i + 1; ja < gameWorldCurrent->path.nodeCount; ++ja )
        {
            entry = ExpandedNodeVisCacheEntry_0(ja, i);
            if ( ((1 << (entry & 7)) & visbuf[entry >> 3]) == 0 )
            {
                nodeNums->array[1][0] = ja;
                nodeCount[1] = Path_GetExpandedNodeNums_r(ja, 256.0, nodeNums->array[1], 1);
                if ( Path_CheckExpandedNodeVis(nodeNums->array, nodeCount, visbuf) )
                    visbuf[entry >> 3] |= 1 << (entry & 7);
            }
        }
        ++count;
        if ( 5 * (20 * count / totalCount) != prevPercent )
        {
            prevPercent = 5 * (20 * count / totalCount);
            Com_Printf(18, "%d%%\n", prevPercent);
        }
    }
    Hunk_FreeTempMemory((char *)nodeNums);
    Com_Printf(18, "Building node vis done.\n");
}

void __cdecl GetNodeAttackPoint(const pathnode_t *node, float *attackPos)
{
    long double v2; // st7
    float fCos; // [esp+14h] [ebp-8h]
    float fSin; // [esp+18h] [ebp-4h]

    if ( !node
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp", 1384, 0, "%s", "node") )
    {
        __debugbreak();
    }
    if ( Path_IsValidClaimNode(node) && Path_IsCoverNode(node) && nodeAttackOffset[node->constant.type] != 0.0 )
    {
        if ( (node->constant.spawnflags & 0x8000) != 0 )
        {
            v2 = (float)((float)(node->constant.fAngle + nodeAttackOffset[node->constant.type]) * 0.017453292);
            fCos = cos(v2);
            fSin = sin(v2);
            *attackPos = (float)(44.0 * fCos) + node->constant.vOrigin[0];
            attackPos[1] = (float)(44.0 * fSin) + node->constant.vOrigin[1];
            attackPos[2] = node->constant.vOrigin[2] + 64.0;
        }
    }
    else
    {
        *attackPos = node->constant.vOrigin[0];
        attackPos[1] = node->constant.vOrigin[1];
        attackPos[2] = node->constant.vOrigin[2];
        attackPos[2] = attackPos[2] + 64.0;
    }
}

int __cdecl Path_GetExpandedNodeNums_r(unsigned int iNodeStart, float fDistLeft, int *iNodeNums, int iNodeCount)
{
    pathnode_t *node; // [esp+Ch] [ebp-10h]
    int j; // [esp+10h] [ebp-Ch]
    int iNewNodeNum; // [esp+14h] [ebp-8h]
    int i; // [esp+18h] [ebp-4h]

    node = Path_ConvertIndexToNode(iNodeStart);
    for ( i = 0; i < node->constant.totalLinkCount; ++i )
    {
        if ( node->constant.Links[i].fDist <= fDistLeft )
        {
            iNewNodeNum = node->constant.Links[i].nodeNum;
            for ( j = 0; j < iNodeCount; ++j )
            {
                if ( iNodeNums[j] == iNewNodeNum )
                    goto LABEL_2;
            }
            iNodeNums[iNodeCount] = iNewNodeNum;
            iNodeCount = Path_GetExpandedNodeNums_r(
                                         iNewNodeNum,
                                         fDistLeft - node->constant.Links[i].fDist,
                                         iNodeNums,
                                         iNodeCount + 1);
        }
LABEL_2:
        ;
    }
    return iNodeCount;
}

int __cdecl Path_CheckExpandedNodeVis(int (*iNodeNums)[8192], int *iNodeCount, unsigned __int8 *visbuf)
{
    int j; // [esp+4h] [ebp-Ch]
    int iEntry; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    for ( i = 0; i < *iNodeCount; ++i )
    {
        for ( j = 0; j < iNodeCount[1]; ++j )
        {
            if ( (*iNodeNums)[i] == (*iNodeNums)[j + 0x2000] )
                return 1;
            if ( (*iNodeNums)[i] >= (*iNodeNums)[j + 0x2000] )
                iEntry = NodeVisCacheEntry_0((*iNodeNums)[j + 0x2000], (*iNodeNums)[i]);
            else
                iEntry = NodeVisCacheEntry_0((*iNodeNums)[i], (*iNodeNums)[j + 0x2000]);
            if ( ((1 << (iEntry & 7)) & visbuf[iEntry >> 3]) != 0 )
                return 1;
        }
    }
    return 0;
}

pathlink_s *__cdecl G_GetNextAvailableTempLinks()
{
    pathlink_s *links; // [esp+0h] [ebp-4h]

    if ( g_tempPathNodeLinksCount >= 128
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                    1606,
                    0,
                    "%s",
                    "g_tempPathNodeLinksCount < MAX_PATH_DYNAMIC_NODES") )
    {
        __debugbreak();
    }
    links = &g_tempPathNodeLinks[16 * g_tempPathNodeLinksCount];
    memset(links, 0, 0xC0u);
    ++g_tempPathNodeLinksCount;
    return links;
}

void __cdecl Path_ConnectPathsForSingleNode(pathnode_t *node)
{
    unsigned int *p_fDist; // eax
    unsigned int *v2; // edx
    unsigned int j; // [esp+20h] [ebp-14h]
    unsigned int ilink; // [esp+24h] [ebp-10h]
    unsigned int nodeindex; // [esp+28h] [ebp-Ch]
    pathlink_s *linksBuffer; // [esp+2Ch] [ebp-8h]
    pathnode_t *othernode; // [esp+30h] [ebp-4h]

    if ( (node->constant.spawnflags & 1) == 0 && node->constant.type && g_tempPathNodeLinksCount < 128 )
    {
        if ( !node->constant.Links || (node->constant.spawnflags & 0x4000) == 0 )
        {
            node->constant.spawnflags |= 0x4000u;
            node->constant.Links = G_GetNextAvailableTempLinks();
        }
        node->constant.totalLinkCount = 0;
        nodeindex = Path_ConvertNodeToIndex(node);
        for ( j = 0; j < gameWorldCurrent->path.nodeCount; ++j )
        {
            othernode = &gameWorldCurrent->path.nodes[j];
            if ( (othernode->constant.spawnflags & 1) == 0 && othernode->constant.type && othernode != node )
            {
                for ( ilink = 0; ilink < othernode->constant.totalLinkCount; ++ilink )
                {
                    if ( othernode->constant.Links[ilink].nodeNum == nodeindex )
                    {
                        p_fDist = (unsigned int *)&othernode->constant.Links[--othernode->constant.totalLinkCount].fDist;
                        v2 = (unsigned int *)&othernode->constant.Links[ilink].fDist;
                        *v2 = *p_fDist;
                        v2[1] = p_fDist[1];
                        v2[2] = p_fDist[2];
                        othernode->dynamic.wLinkCount = othernode->constant.totalLinkCount;
                    }
                }
                if ( Path_AttemptLink(node, othernode, node->constant.Links, 16) )
                {
                    if ( !othernode->constant.Links || (othernode->constant.spawnflags & 0x4000) == 0 )
                    {
                        othernode->constant.spawnflags |= 0x4000u;
                        linksBuffer = G_GetNextAvailableTempLinks();
                        if ( othernode->constant.Links )
                            memcpy(
                                (unsigned __int8 *)linksBuffer,
                                (unsigned __int8 *)othernode->constant.Links,
                                12 * othernode->constant.totalLinkCount);
                        othernode->constant.Links = linksBuffer;
                    }
                    if ( othernode->constant.totalLinkCount < 0x10u
                        && Path_AttemptLink(othernode, node, othernode->constant.Links, 16) )
                    {
                        othernode->dynamic.wLinkCount = othernode->constant.totalLinkCount;
                    }
                }
                if ( node->constant.totalLinkCount >= 0x10u )
                    break;
            }
        }
        node->dynamic.wLinkCount = node->constant.totalLinkCount;
    }
}

char __cdecl Path_AttemptLink(pathnode_t *pNodeFrom, pathnode_t *pNodeTo, pathlink_s *pLinks, int iMaxLinks)
{
    int p1ent; // [esp+4h] [ebp-Ch]
    char retv; // [esp+Bh] [ebp-5h]
    int contents_restore_ent; // [esp+Ch] [ebp-4h]

    contents_restore_ent = -1;
    if ( enable_moving_paths->current.integer == 1 )
    {
        p1ent = get_pathnode_parent(pNodeFrom)->entnum;
        if ( p1ent != get_pathnode_parent(pNodeTo)->entnum )
            return 0;
        if ( p1ent != 1022 && g_oldContents )
        {
            contents_restore_ent = p1ent;
            if ( (p1ent < 0 || p1ent >= level.num_entities)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                            1360,
                            0,
                            "%s",
                            "contents_restore_ent >= 0 && contents_restore_ent < level.num_entities") )
            {
                __debugbreak();
            }
            g_entities[p1ent].r.contents = *((unsigned int *)g_oldContents + p1ent);
        }
    }
    retv = Path_AttemptLink1(pNodeFrom, pNodeTo, pLinks, iMaxLinks);
    if ( contents_restore_ent != -1 )
        g_entities[contents_restore_ent].r.contents = 0;
    return retv;
}

char __cdecl Path_AttemptLink1(pathnode_t *pNodeFrom, pathnode_t *pNodeTo, pathlink_s *pLinks, int iMaxLinks)
{
    float fDist; // [esp+0h] [ebp-Ch] BYREF
    pathlink_s *pLink; // [esp+4h] [ebp-8h]
    int bNegotiationLink; // [esp+8h] [ebp-4h] BYREF

    fDist = 0.0f;
    bNegotiationLink = 0;
    if ( !pNodeFrom
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp", 1303, 0, "%s", "pNodeFrom") )
    {
        __debugbreak();
    }
    if ( !pNodeTo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp", 1304, 0, "%s", "pNodeTo") )
    {
        __debugbreak();
    }
    if ( !pLinks
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp", 1305, 0, "%s", "pLinks") )
    {
        __debugbreak();
    }
    if ( iMaxLinks < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                    1306,
                    0,
                    "%s",
                    "iMaxLinks >= 0") )
    {
        __debugbreak();
    }
    if ( !Path_CanLinkNodes(pNodeFrom, pNodeTo, &fDist, &bNegotiationLink) )
        return 0;
    if ( pNodeFrom->constant.totalLinkCount >= iMaxLinks )
        Com_Error(ERR_DROP, "PATH_MAX_TOTAL_LINKS (%i) exceeded", 0x80000);
    pLink = &pLinks[pNodeFrom->constant.totalLinkCount++];
    if ( pNodeTo - gameWorldCurrent->path.nodes != (unsigned __int16)(pNodeTo - gameWorldCurrent->path.nodes)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                    1336,
                    0,
                    "%s",
                    "pNodeTo - gameWorldCurrent.path.nodes == (unsigned short)( pNodeTo - gameWorldCurrent.path.nodes )") )
    {
        __debugbreak();
    }
    pLink->nodeNum = pNodeTo - gameWorldCurrent->path.nodes;
    pLink->fDist = fDist;
    pLink->disconnectCount = 0;
    pLink->negotiationLink = bNegotiationLink;
    return 1;
}

bool __cdecl Path_CanLinkNodes(pathnode_t *pNodeFrom, pathnode_t *pNodeTo, float *pfDist, int *pbNegotiationLink)
{
    bool v5; // [esp+5Ch] [ebp-824h]
    actor_physics_t phys; // [esp+88h] [ebp-7F8h] BYREF
    float fLastGroundPlaneAltitude; // [esp+85Ch] [ebp-24h]
    float fDistSqrd; // [esp+860h] [ebp-20h]
    float fDist; // [esp+864h] [ebp-1Ch]
    float vMoveDir[2]; // [esp+868h] [ebp-18h] BYREF
    int i; // [esp+870h] [ebp-10h]
    float vDelta[3]; // [esp+874h] [ebp-Ch]

    //colgeom_visitor_inlined_t<200>::colgeom_visitor_inlined_t<200>(&phys.proximity_data);
    if ( !pNodeFrom
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp", 1164, 0, "%s", "pNodeFrom") )
    {
        __debugbreak();
    }
    if ( !pNodeTo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp", 1165, 0, "%s", "pNodeTo") )
    {
        __debugbreak();
    }
    if ( !pfDist
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp", 1166, 0, "%s", "pfDist") )
    {
        __debugbreak();
    }
    if ( Path_IsNegotiationLink(pNodeFrom, pNodeTo) )
    {
        *pbNegotiationLink = 1;
        *pfDist = 15.0f;
        //phys.proximity_data.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&visitor_base_t::`vftable';
        return 1;
    }
    else
    {
        vDelta[0] = pNodeTo->constant.vOrigin[0] - pNodeFrom->constant.vOrigin[0];
        vDelta[1] = pNodeTo->constant.vOrigin[1] - pNodeFrom->constant.vOrigin[1];
        vDelta[2] = pNodeTo->constant.vOrigin[2] - pNodeFrom->constant.vOrigin[2];
        if ( vDelta[2] >= -128.0 && vDelta[2] <= 128.0 )
        {
            if ( (float)((float)(vDelta[0] * vDelta[0]) + (float)(vDelta[1] * vDelta[1])) <= 65536.0 )
            {
                vMoveDir[0] = pNodeTo->constant.vOrigin[0] - pNodeFrom->constant.vOrigin[0];
                vMoveDir[1] = pNodeTo->constant.vOrigin[1] - pNodeFrom->constant.vOrigin[1];
                *pfDist = Vec2Normalize(vMoveDir);
                *pbNegotiationLink = 0;
                phys.vOrigin[0] = pNodeFrom->constant.vOrigin[0];
                phys.vOrigin[1] = pNodeFrom->constant.vOrigin[1];
                phys.vOrigin[2] = pNodeFrom->constant.vOrigin[2];
                memset(phys.vVelocity, 0, sizeof(phys.vVelocity));
                phys.groundEntNum = 1022;
                phys.vWishDelta[0] = 4.0 * vMoveDir[0];
                phys.vWishDelta[1] = 4.0 * vMoveDir[1];
                phys.vWishDelta[2] = 0.0f;
                phys.bIsAlive = 1;
                phys.iEntNum = 1023;
                phys.ePhysicsType = AIPHYS_NORMAL_ABSOLUTE;
                LODWORD(phys.fGravity) = bg_gravity->current.integer;
                phys.iMsec = 50;
                phys.vMins[2] = actorMins[2];
                phys.vMaxs[2] = actorMaxs[2];
                phys.vMins[0] = -15.0 - 0.5;
                phys.vMins[1] = -15.0 - 0.5;
                phys.vMaxs[0] = 15.0 + 0.5;
                phys.vMaxs[1] = 15.0 + 0.5;
                phys.iTraceMask = 0x2820011;
                fLastGroundPlaneAltitude = -FLT_MAX;
                for ( i = 0; ; ++i )
                {
                    if ( i >= 96 )
                        return 0;
                    fDistSqrd = (float)((float)(pNodeTo->constant.vOrigin[0] - phys.vOrigin[0])
                                                        * (float)(pNodeTo->constant.vOrigin[0] - phys.vOrigin[0]))
                                        + (float)((float)(pNodeTo->constant.vOrigin[1] - phys.vOrigin[1])
                                                        * (float)(pNodeTo->constant.vOrigin[1] - phys.vOrigin[1]));
                    if ( fDistSqrd <= 16.0 )
                        break;
                    if ( !Actor_Physics_z(&phys) )
                    {
                        //phys.proximity_data.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&visitor_base_t::`vftable';
                        return 0;
                    }
                    if ( phys.bHasGroundPlane )
                        fLastGroundPlaneAltitude = phys.vOrigin[2];
                    if ( (float)(fLastGroundPlaneAltitude - phys.vOrigin[2]) > 32.0 )
                    {
                       // phys.proximity_data.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&visitor_base_t::`vftable';
                        return 0;
                    }
                    if ( phys.iHitEntnum != 1023
                        && (phys_ai_collision_mode->current.integer != 1
                         || !isDeflectionOK(pNodeFrom->constant.vOrigin, phys.vOrigin, vMoveDir)) )
                    {
                        //phys.proximity_data.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&visitor_base_t::`vftable';
                        return 0;
                    }
                }
                if ( (float)(fLastGroundPlaneAltitude - pNodeTo->constant.vOrigin[2]) > 32.0 )
                {
                    //phys.proximity_data.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&visitor_base_t::`vftable';
                    return 0;
                }
                fDist = sqrtf(fDistSqrd);
                phys.vWishDelta[0] = fDist * vMoveDir[0];
                phys.vWishDelta[1] = fDist * vMoveDir[1];
                if ( !Actor_Physics_z(&phys) )
                {
                    //phys.proximity_data.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&visitor_base_t::`vftable';
                    return 0;
                }
                if ( phys.iHitEntnum != 1023 )
                {
                    if ( phys_ai_collision_mode->current.integer != 1 )
                    {
                       // phys.proximity_data.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&visitor_base_t::`vftable';
                        return 0;
                    }
                    if ( !isDeflectionOK(pNodeFrom->constant.vOrigin, phys.vOrigin, vMoveDir) )
                    {
                        //phys.proximity_data.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&visitor_base_t::`vftable';
                        return 0;
                    }
                }
                v5 = fabs(phys.vOrigin[2] - pNodeTo->constant.vOrigin[2]) <= 18.0;
                //phys.proximity_data.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&visitor_base_t::`vftable';
                return v5;
            }
            else
            {
                //phys.proximity_data.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&visitor_base_t::`vftable';
                return 0;
            }
        }
        else
        {
            //phys.proximity_data.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&visitor_base_t::`vftable';
            return 0;
        }
    }
}

static float MAX_ALLOWED_DEFLECTION_SQ = 0.30000001;
bool __cdecl isDeflectionOK(const float *start, const float *origin, const float *movedir)
{
    float v4; // [esp+0h] [ebp-14h]
    float deflection; // [esp+4h] [ebp-10h]
    float deflection_4; // [esp+8h] [ebp-Ch]

    deflection = *origin - *start;
    deflection_4 = origin[1] - start[1];
    //LODWORD(v4) = COERCE_UNSIGNED_INT((float)(deflection * *movedir) + (float)(deflection_4 * movedir[1])) ^ _mask__NegFloat_;
    (v4) = -((float)(deflection * *movedir) + (float)(deflection_4 * movedir[1]));
    return MAX_ALLOWED_DEFLECTION_SQ >= (float)((float)((float)((float)(v4 * *movedir) + deflection)
                                                                                                        * (float)((float)(v4 * *movedir) + deflection))
                                                                                        + (float)((float)((float)(v4 * movedir[1]) + deflection_4)
                                                                                                        * (float)((float)(v4 * movedir[1]) + deflection_4)));
}

void __cdecl Path_ConnectPaths()
{
    unsigned int j; // [esp+28h] [ebp-2Ch]
    pathnode_t *node; // [esp+2Ch] [ebp-28h]
    pathnode_t *nodea; // [esp+2Ch] [ebp-28h]
    unsigned int totalLinkCount; // [esp+30h] [ebp-24h]
    pathlink_s *linksBuffer; // [esp+34h] [ebp-20h]
    int entIndex; // [esp+3Ch] [ebp-18h]
    int entIndexa; // [esp+3Ch] [ebp-18h]
    gentity_s *ent; // [esp+40h] [ebp-14h]
    int totalCount; // [esp+44h] [ebp-10h]
    int prevPercent; // [esp+48h] [ebp-Ch]
    unsigned int i; // [esp+4Ch] [ebp-8h]
    unsigned int ia; // [esp+4Ch] [ebp-8h]
    int count; // [esp+50h] [ebp-4h]

    Com_Printf(18, "Connecting paths...\n");
    if ( pathstatic.pathLinks )
        Com_Error(ERR_DROP, "Cannot calculate paths on a map_restart if paths already exist");
    if ( gameWorldCurrent->path.nodeCount >= 0x2000 )
        Com_Error(ERR_DROP, "PATH_MAX_NODES (%i) exceeded.  Check log for nodecount", 0x2000);
    if ( g_oldContents
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                    1729,
                    0,
                    "%s",
                    "g_oldContents == NULL") )
    {
        __debugbreak();
    }
    g_oldContents = Hunk_AllocateTempMemory(4096, "Path_ConnectPaths");
    if ( !g_oldContents
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                    1731,
                    0,
                    "%s",
                    "g_oldContents") )
    {
        __debugbreak();
    }
    for ( entIndex = 0; entIndex < level.num_entities; ++entIndex )
    {
        ent = &level.gentities[entIndex];
        if ( ent->r.inuse )
        {
            *((unsigned int *)g_oldContents + entIndex) = ent->r.contents;
            if ( Path_IsDynamicBlockingEntity(ent) )
                ent->r.contents = 0;
        }
    }
    totalCount = 0;
    for ( i = 0; i < gameWorldCurrent->path.nodeCount; ++i )
    {
        node = &gameWorldCurrent->path.nodes[i];
        if ( node->constant.totalLinkCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                        1748,
                        0,
                        "%s",
                        "node->constant.totalLinkCount == 0") )
        {
            __debugbreak();
        }
        if ( node->constant.Links
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                        1749,
                        0,
                        "%s",
                        "node->constant.Links == NULL") )
        {
            __debugbreak();
        }
        if ( (node->constant.spawnflags & 1) == 0 && node->constant.type )
            ++totalCount;
    }
    count = 0;
    prevPercent = 0;
    linksBuffer = (pathlink_s *)Z_Malloc(0x600000, "Path_ConnectPaths", 6);
    totalLinkCount = 0;
    for ( ia = 0; ia < gameWorldCurrent->path.nodeCount; ++ia )
    {
        nodea = &gameWorldCurrent->path.nodes[ia];
        if ( nodea->constant.totalLinkCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                        1778,
                        0,
                        "%s",
                        "node->constant.totalLinkCount == 0") )
        {
            __debugbreak();
        }
        if ( nodea->constant.Links
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                        1779,
                        0,
                        "%s",
                        "node->constant.Links == NULL") )
        {
            __debugbreak();
        }
        if ( (nodea->constant.spawnflags & 1) == 0 && nodea->constant.type )
        {
            for ( j = 0; j < gameWorldCurrent->path.nodeCount; ++j )
            {
                if ( (gameWorldCurrent->path.nodes[j].constant.spawnflags & 1) == 0
                    && gameWorldCurrent->path.nodes[j].constant.type
                    && ia != j )
                {
                    Path_AttemptLink(
                        nodea,
                        &gameWorldCurrent->path.nodes[j],
                        &linksBuffer[totalLinkCount],
                        0x80000 - totalLinkCount);
                }
            }
            totalLinkCount += nodea->constant.totalLinkCount;
            if ( !gameWorldCurrent->path.nodes[ia].constant.totalLinkCount )
                Com_PrintError(
                    18,
                    "ERROR: Pathnode (%s) at (%g %g %g) has no connections leaving it\n",
                    nodeStringTable[nodea->constant.type],
                    nodea->constant.vOrigin[0],
                    nodea->constant.vOrigin[1],
                    nodea->constant.vOrigin[2]);
            ++count;
            if ( 5 * (20 * count / totalCount) != prevPercent )
            {
                prevPercent = 5 * (20 * count / totalCount);
                Com_Printf(18, "%d%%\n", prevPercent);
            }
        }
    }
    Com_Printf(18, "Connecting paths done.\n");
    Path_MakePathDataPermanent(linksBuffer, totalLinkCount, 0, 0);
    Z_Free((char *)linksBuffer, 6);
    for ( entIndexa = 0; entIndexa < level.num_entities; ++entIndexa )
    {
        if ( level.gentities[entIndexa].r.inuse )
            level.gentities[entIndexa].r.contents = *((unsigned int *)g_oldContents + entIndexa);
    }
    Hunk_FreeTempMemory((char *)g_oldContents);
    g_oldContents = 0;
}

void __cdecl Path_SavePaths()
{
    unsigned __int8 *v0; // eax
    unsigned __int8 *v1; // eax
    unsigned int j; // [esp+10h] [ebp-24h]
    unsigned int ja; // [esp+10h] [ebp-24h]
    unsigned int jb; // [esp+10h] [ebp-24h]
    unsigned __int8 *pos; // [esp+14h] [ebp-20h]
    unsigned __int8 *posa; // [esp+14h] [ebp-20h]
    unsigned __int8 *posb; // [esp+14h] [ebp-20h]
    int numpaths; // [esp+18h] [ebp-1Ch]
    pathlink_s *pLink; // [esp+20h] [ebp-14h]
    unsigned int i; // [esp+28h] [ebp-Ch]
    unsigned int ia; // [esp+28h] [ebp-Ch]
    unsigned int ib; // [esp+28h] [ebp-Ch]
    pathnode_t *pNodeFrom; // [esp+2Ch] [ebp-8h]
    unsigned int visBytes; // [esp+30h] [ebp-4h]

    if ( gameWorldCurrent->path.nodeCount )
    {
        Com_Printf(18, "Saving paths...\n");
        numpaths = 0;
        for ( i = 0; i < gameWorldCurrent->path.nodeCount; ++i )
            numpaths += gameWorldCurrent->path.nodes[i].constant.totalLinkCount;
        visBytes = (gameWorldCurrent->path.nodeCount * (gameWorldCurrent->path.nodeCount - 1) + 7) >> 3;
        v0 = (unsigned __int8 *)Z_Malloc(
                                                            6 * numpaths + 2 * gameWorldCurrent->path.nodeCount + 6 + 2 * visBytes,
                                                            "Path_SavePaths",
                                                            0);
        pathstatic.pathbuf = v0;
        *(unsigned int *)v0 = 8;
        pos = v0 + 4;
        if ( gameWorldCurrent->path.nodeCount != LOWORD(gameWorldCurrent->path.nodeCount)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                        1886,
                        0,
                        "%s",
                        "gameWorldCurrent.path.nodeCount == (unsigned short)gameWorldCurrent.path.nodeCount") )
        {
            __debugbreak();
        }
        *(_WORD *)pos = gameWorldCurrent->path.nodeCount;
        posa = pos + 2;
        for ( ia = 0; ia < gameWorldCurrent->path.nodeCount; ++ia )
        {
            pNodeFrom = &gameWorldCurrent->path.nodes[ia];
            *(_WORD *)posa = pNodeFrom->constant.totalLinkCount;
            posa += 2;
            for ( j = 0; j < pNodeFrom->constant.totalLinkCount; ++j )
            {
                pLink = &pNodeFrom->constant.Links[j];
                if ( j
                    && pLink <= &pNodeFrom->constant.Links[j - 1]
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode_load_obj.cpp",
                                1902,
                                0,
                                "%s",
                                "!j || pLink > &pNodeFrom->constant.Links[j - 1]") )
                {
                    __debugbreak();
                }
                v1 = WriteShort(posa, pLink->nodeNum);
                posa = WriteInt(v1, LODWORD(pLink->fDist));
            }
        }
        ib = 0;
        while ( ib < visBytes )
        {
            if ( ib + 1 >= visBytes || gameWorldCurrent->path.pathVis[ib] )
            {
                for ( jb = ib + 1;
                            jb < visBytes
                     && jb - ib < 0x7F
                     && (gameWorldCurrent->path.pathVis[jb]
                        || jb + 1 != visBytes && jb - ib < 0x7E && gameWorldCurrent->path.pathVis[jb + 1]);
                            ++jb )
                {
                    ;
                }
                *posa++ = ~(jb - ib);
                while ( ib < jb )
                    *posa++ = gameWorldCurrent->path.pathVis[ib++];
            }
            else
            {
                for ( ja = ib + 1; ja < visBytes - 1 && !gameWorldCurrent->path.pathVis[ja] && ja - ib < 0x7F; ++ja )
                    ;
                *posa = ja - ib;
                posb = posa + 1;
                *posb = gameWorldCurrent->path.pathVis[ja];
                posa = posb + 1;
                ib = ja + 1;
            }
        }
        SV_SavePaths(pathstatic.pathbuf, posa - pathstatic.pathbuf);
        Z_Free((char *)pathstatic.pathbuf, 0);
        pathstatic.pathbuf = 0;
        Com_Printf(18, "Saving paths done.\n");
    }
}

unsigned __int8 *__cdecl WriteShort(unsigned __int8 *buffer, unsigned __int16 data)
{
    *(_WORD *)buffer = data;
    return buffer + 2;
}

unsigned __int8 *__cdecl WriteInt(unsigned __int8 *buffer, int data)
{
    *(unsigned int *)buffer = data;
    return buffer + 4;
}

