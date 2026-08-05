#include "pathnode.h"
#include <clientscript/cscr_vm.h>
#include <game_mp/g_spawn_mp.h>
#include <bgame/bg_slidemove.h>
#include <universal/com_math_anglevectors.h>
#include <clientscript/cscr_stringlist.h>
#include <universal/com_memory.h>
#include <gfx_d3d/r_dvars.h>
#include "pathnode_load_obj.h"
#include <qcommon/com_bsp_load_obj.h>
#include <qcommon/cm_world.h>
#include <win32/win_main.h>
#include "actor_navigation.h"
#include <clientscript/scr_const.h>
#include <cgame/cg_drawtools.h>
#include <game_mp/g_utils_mp.h>
#include "g_debug.h"
#include <client_mp/cl_cgame_mp.h>
#include "actor_exposed.h"

#include <algorithm>
#include <game_mp/actor_mp.h>
#include <qcommon/dobj_management.h>
#include <server/sv_game.h>
#include "g_bsp.h"

node_field_t fields_3[12] = // "fields"
{
  { "targetname", 6, { 2 }, F_STRING, NULL },
  { "target", 12, { 2 }, F_STRING, NULL },
  { "animscript", 14, { 2 }, F_STRING, NULL },
  { "script_linkname", 8, { 2 }, F_STRING, NULL },
  { "script_noteworthy", 10, { 2 }, F_STRING, NULL },
  { "origin", 20, { 12 }, F_VECTOR, NULL },
  { "angles", 32, { 4 }, F_VECTORHACK, NULL },
  { "radius", 44, { 4 }, F_FLOAT, NULL },
  { "minusedistsq", 48, { 4 }, F_FLOAT, NULL },
  { "spawnflags", 4, { 2 }, F_SHORT, NULL },
  { "type", 0, { 4 }, F_INT, Path_GetType },
  { NULL, 0, { 0 }, F_INT, NULL }
};

const char *nodeStringTable[21] =
{
  "BAD NODE",
  "Path",
  "Cover Stand",
  "Cover Crouch",
  "Cover Crouch Window",
  "Cover Prone",
  "Cover Right",
  "Cover Left",
  "Cover Wide Right",
  "Cover Wide Left",
  "Cover Pillar",
  "Conceal Stand",
  "Conceal Crouch",
  "Conceal Prone",
  "Reacquire",
  "Balcony",
  "Scripted",
  "Begin",
  "End",
  "Turret",
  "Guard"
};

const float nodeColorTable[21][4] =
{
  { 1.0, 0.0, 0.0, 1.0 },
  { 1.0, 0.0, 1.0, 1.0 },
  { 0.0, 0.54000002, 0.66000003, 1.0 },
  { 0.0, 0.93000001, 0.72000003, 1.0 },
  { 0.0, 0.69999999, 0.5, 1.0 },
  { 0.0, 0.60000002, 0.46000001, 1.0 },
  { 0.85000002, 0.85000002, 0.1, 1.0 },
  { 1.0, 0.69999999, 0.0, 1.0 },
  { 0.75, 0.75, 0.0, 1.0 },
  { 0.75, 0.52999997, 0.38, 1.0 },
  { 0.1, 0.85000002, 0.1, 1.0 },
  { 0.0, 0.0, 1.0, 1.0 },
  { 0.0, 0.0, 0.75, 1.0 },
  { 0.0, 0.0, 0.5, 1.0 },
  { 0.51999998, 0.51999998, 0.60000002, 1.0 },
  { 0.5, 0.5, 0.0, 1.0 },
  { 0.72000003, 0.72000003, 0.82999998, 1.0 },
  { 0.5, 0.60000002, 0.5, 1.0 },
  { 0.60000002, 0.5, 0.5, 1.0 },
  { 0.0, 0.93000001, 0.72000003, 1.0 },
  { 0.69999999, 0.0, 0.0, 1.0 }
};

pathlocal_t g_path;
path_t *debugPath;
path_t debugPathBuf;
phys_inplace_avl_tree<unsigned int, generic_avl_map_node_t, generic_avl_map_node_t> g_pathnode_resized_links_map;
phys_simple_allocator<pathnode_parent_t> g_pathnode_parent_allocator;
phys_inplace_avl_tree<unsigned int, generic_avl_map_node_t, generic_avl_map_node_t> g_parented_pathnode_list_map;
phys_inplace_avl_tree<unsigned int, generic_avl_map_node_t, generic_avl_map_node_t> g_pathnode_parent_map;

pathnode_parent_t node_parent_world;

int overCount;
float adjust_maxs[3];
float adjust_mins[3];
float perp[2];
float width;
float vGoalPos[3];
int iValidBits;
float vStartPos[3];
const char *g_pathsError;
pathstatic_t pathstatic;
GameWorldMp *gameWorldCurrent = &gameWorldMp;
void *g_oldContents;
pathlink_s g_tempPathNodeLinks[2048];
int g_tempPathNodeLinksCount;

pathnode_t *g_radiant_selected_pathnode;

// *WARNING* One or more selections were skipped as they could not be interpreted as c data


void __cdecl Path_GetType(pathnode_t *node, int)
{
    if ( node->constant.type > (unsigned int)NODE_GUARD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    711,
                    0,
                    "%s",
                    "node->constant.type >= NODE_BADNODE && node->constant.type < NODE_NUMTYPES") )
    {
        __debugbreak();
    }
    Scr_AddString((char *)nodeStringTable[node->constant.type], SCRIPTINSTANCE_SERVER);
}

void __cdecl Scr_SetPathnodeField(unsigned int entnum, unsigned int offset)
{
    if ( offset >= 0xB
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    718,
                    0,
                    "%s",
                    "(unsigned)offset < ARRAY_COUNT( fields ) - 1") )
    {
        __debugbreak();
    }
    if ( entnum >= 0x2000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    719,
                    0,
                    "%s",
                    "(unsigned)entnum < PATH_MAX_NODES") )
    {
        __debugbreak();
    }
    Path_ReadOnly(offset);
}

void __cdecl Path_ReadOnly(int offset)
{
    const char *v1; // eax

    v1 = va("pathnode property '%s' is read-only", fields_3[offset].name);
    Scr_Error(v1, 0);
}

void __cdecl Scr_GetPathnodeField(unsigned int entnum, unsigned int offset)
{
    pathnode_t *node; // [esp+0h] [ebp-8h]
    node_field_t *f; // [esp+4h] [ebp-4h]

    if ( offset >= 0xB
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    730,
                    0,
                    "offset doesn't index ARRAY_COUNT( fields ) - 1\n\t%i not in [0, %i)",
                    offset,
                    11) )
    {
        __debugbreak();
    }
    if ( entnum >= g_path.actualNodeCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    731,
                    0,
                    "entnum doesn't index g_path.actualNodeCount\n\t%i not in [0, %i)",
                    entnum,
                    g_path.actualNodeCount) )
    {
        __debugbreak();
    }
    f = &fields_3[offset];
    node = &gameWorldCurrent->path.nodes[entnum];
    if ( f->getter )
        f->getter(node, offset);
    else
        GScr_GetGenericField((unsigned __int8 *)node, f->type, f->ofs, 0);
}

void __cdecl Path_CallFunctionForNodes(scriptInstance_t inst, void (__cdecl *function)(scriptInstance_t, pathnode_t *))
{
    unsigned int nodeIndex; // [esp+0h] [ebp-8h]
    unsigned int nodeCount; // [esp+4h] [ebp-4h]

    if ( !function && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 764, 0, "%s", "function") )
        __debugbreak();
    nodeCount = gameWorldCurrent->path.nodeCount;
    for ( nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex )
        function(inst, &gameWorldCurrent->path.nodes[nodeIndex]);
}

void __cdecl GScr_SetDynamicPathnodeField(pathnode_t *node, unsigned int index)
{
    unsigned int v2; // eax

    v2 = Path_ConvertNodeToIndex(node);
    Scr_SetDynamicEntityField(v2, 2u, index, SCRIPTINSTANCE_SERVER, 0);
}

void    parented_pathnode_list_update(gentity_s *gent, const phys_mat44 *mat)
{
    const phys_vec3 *v3; // [esp-20h] [ebp-40h]
    phys_vec3 v4; // [esp-1Ch] [ebp-3Ch] BYREF
    phys_vec3 v5; // [esp-Ch] [ebp-2Ch] BYREF
    pathnode_parent_t *node_parent; // [esp+4h] [ebp-1Ch]
    generic_avl_map_node_t *gamn; // [esp+8h] [ebp-18h]
    generic_avl_map_node_t *m_tree_root; // [esp+Ch] [ebp-14h]
    unsigned int avl_key; // [esp+10h] [ebp-10h]
    //_UNKNOWN *v10[2]; // [esp+14h] [ebp-Ch] BYREF
    //const float *__formal; // [esp+20h] [ebp+0h]
    //
    //v10[0] = a1;
    //v10[1] = __formal;
    if (enable_moving_paths->current.integer == 1)
    {
        avl_key = gent->s.number;
        m_tree_root = g_parented_pathnode_list_map.m_tree_root;
        while (m_tree_root && avl_key != m_tree_root->m_avl_key)
        {
            if (avl_key >= m_tree_root->m_avl_key)
                m_tree_root = m_tree_root->m_avl_tree_node.m_right;
            else
                m_tree_root = m_tree_root->m_avl_tree_node.m_left;
        }
        gamn = m_tree_root;
        if (m_tree_root)
        {
            for (node_parent = (pathnode_parent_t *)gamn->m_data; node_parent; node_parent = node_parent->m_next)
            {
                Phys_Vec3ToNitrousVec(node_parent->origin_loc, &v5);
                v3 = phys_full_multiply(&v4, mat, &v5);
                v5.x = v3->x;
                v5.y = v3->y;
                v5.z = v3->z;
                Phys_NitrousVecToVec3(&v5, node_parent->m_node->constant.vOrigin);
            }
        }
    }
}

int first_time_called = 1;
const pathnode_parent_t *__cdecl get_pathnode_parent(const pathnode_t *node)
{
    generic_avl_map_node_t *m_tree_root; // [esp+0h] [ebp-Ch]

    if ( first_time_called )
    {
        node_parent_world.origin_loc[0] = 0.0f;
        node_parent_world.origin_loc[1] = 0.0f;
        node_parent_world.origin_loc[2] = 0.0f;
        node_parent_world.entnum = 1022;
        node_parent_world.m_node = 0;
        node_parent_world.m_next = 0;
        first_time_called = 0;
    }
    if ( (node->constant.spawnflags & 0x100) == 0 )
        return &node_parent_world;
    m_tree_root = g_pathnode_parent_map.m_tree_root;
    while ( m_tree_root && node != (const pathnode_t *)m_tree_root->m_avl_key )
    {
        if ( (unsigned int)node >= m_tree_root->m_avl_key )
            m_tree_root = m_tree_root->m_avl_tree_node.m_right;
        else
            m_tree_root = m_tree_root->m_avl_tree_node.m_left;
    }
    if ( m_tree_root )
        return (const pathnode_parent_t *)m_tree_root->m_data;
    else
        return &node_parent_world;
}

void __cdecl G_UpdateTrackExtraNodes()
{
    if ( g_path.originErrors )
        Com_PrintError(
            1,
            "There are %d path node origins that don't match.    If this number is higher than expected then you may want to reco"
            "mpile the map before using MyMapEnts..\n",
            g_path.originErrors);
    if ( g_path.extraNodes )
        Com_PrintError(
            18,
            "There are %d extra path nodes in the entity string.    Ignoring extra path nodes, and some of the key value pairs ar"
            "e likely mapped to the wrong nodes.\n",
            g_path.extraNodes);
    if ( g_path.actualNodeCount < gameWorldCurrent->path.nodeCount )
        Com_PrintError(
            18,
            "There %d less path nodes in the entity string than in the compiled map.    Some of the key value pairs are likely ma"
            "pped to the wrong nodes.\n",
            gameWorldCurrent->path.nodeCount - g_path.actualNodeCount);
    g_path.actualNodeCount = gameWorldCurrent->path.nodeCount;
}

void __cdecl G_DropPathNodeToFloor(unsigned int nodeIndex)
{
    node_droptofloor(&gameWorldCurrent->path.nodes[nodeIndex]);
    G_InitPathBaseNode(&gameWorldCurrent->path.basenodes[nodeIndex], &gameWorldCurrent->path.nodes[nodeIndex]);
}

void    node_droptofloor(pathnode_t *node)
{
    float origin_loc[3]; // [esp+18h] [ebp-15Ch] BYREF
    phys_vec3 *v3; // [esp+24h] [ebp-150h]
    phys_vec3 v4; // [esp+28h] [ebp-14Ch] BYREF
    phys_vec3 node_pos; // [esp+38h] [ebp-13Ch] BYREF
    phys_mat44 gent_mat; // [esp+48h] [ebp-12Ch] BYREF
    float v7[6]; // [esp+8Ch] [ebp-E8h] BYREF
    gentity_s *gent; // [esp+B0h] [ebp-C4h]
    int gentnum; // [esp+B4h] [ebp-C0h]
    float *v10; // [esp+B8h] [ebp-BCh]
    float save_hitId; // [esp+BCh] [ebp-B8h]
    TraceHitType save_hitType; // [esp+C0h] [ebp-B4h]
    float dropMaxs[3]; // [esp+C4h] [ebp-B0h] BYREF
    float dropMins[3]; // [esp+D0h] [ebp-A4h] BYREF
    float endpos[3]; // [esp+DCh] [ebp-98h] BYREF
    float vEnd[3]; // [esp+E8h] [ebp-8Ch] BYREF
    float vOrigin[4]; // [esp+F4h] [ebp-80h]
    col_context_t context; // [esp+104h] [ebp-70h] BYREF
    trace_t trace; // [esp+12Ch] [ebp-48h] BYREF

    Vec3Copy(node->constant.vOrigin, vOrigin);

    Vec3Copy(vOrigin, vEnd);
    vEnd[2] -= 256.0f;

    Vec3Copy(vOrigin, endpos);
    endpos[2] += 1.0f;
    
    Vec3Copy(actorMins, dropMins);
    Vec3Copy(actorMaxs, dropMaxs);

    dropMaxs[2] = (float)(15.0 - -15.0) + 0.0;

    G_TraceCapsule(&trace, endpos, dropMins, dropMaxs, vEnd, 1023, 0x820011, &context);
    if (trace.startsolid || trace.allsolid)
    {
        printf(
            "ERROR: Pathnode (%s) at (%g %g %g) is in solid\n",
            nodeStringTable[node->constant.type],
            vOrigin[0],
            vOrigin[1],
            vOrigin[2]);
        Com_PrintError(
            1,
            "ERROR: Pathnode (%s) at (%g %g %g) is in solid\n",
            nodeStringTable[node->constant.type],
            vOrigin[0],
            vOrigin[1],
            vOrigin[2]);
        node->constant.type = NODE_BADNODE;
        return;
    }
    if (trace.fraction == 1.0)
    {
        printf(
            "ERROR: Pathnode (%s) at (%g %g %g) is floating\n",
            nodeStringTable[node->constant.type],
            vOrigin[0],
            vOrigin[1],
            vOrigin[2]);
        Com_PrintError(
            1,
            "ERROR: Pathnode (%s) at (%g %g %g) is floating\n",
            nodeStringTable[node->constant.type],
            vOrigin[0],
            vOrigin[1],
            vOrigin[2]);
        node->constant.type = NODE_BADNODE;
    }
    else
    {
        Vec3Lerp(endpos, vEnd, trace.fraction, endpos);
        save_hitType = trace.hitType;
        LOWORD(save_hitId) = trace.hitId;
        G_TraceCapsule(&trace, endpos, actorMins, actorMaxs, endpos, 1023, 0x820011, &context);
        if (trace.startsolid || trace.allsolid)
        {
            printf(
                "ERROR: Pathnode (%s) at (%g %g %g) is in solid\n",
                nodeStringTable[node->constant.type],
                vOrigin[0],
                vOrigin[1],
                vOrigin[2]);
            Com_PrintError(
                1,
                "ERROR: Pathnode (%s) at (%g %g %g) is in solid\n",
                nodeStringTable[node->constant.type],
                vOrigin[0],
                vOrigin[1],
                vOrigin[2]);
            node->constant.type = NODE_BADNODE;
            return;
        }
        v10 = node->constant.vOrigin;
        node->constant.vOrigin[0] = endpos[0];
        v10[1] = endpos[1];
        v10[2] = endpos[2];
        if ((node->constant.spawnflags & 0x100) != 0
            && save_hitType == TRACE_HITTYPE_ENTITY
            && LOWORD(save_hitId) < 0x3FEu
            && enable_moving_paths->current.integer == 1)
        {
            gentnum = LOWORD(save_hitId);
            gent = &g_entities[LOWORD(save_hitId)];
            if (is_moving_entity(gent))
            {
                AnglesToAxis(gent->r.currentAngles, (float (*)[3])v7);
                Phys_AxisToNitrousMat((float (*)[3])v7, &gent_mat);
                Phys_Vec3ToNitrousVec(gent->r.currentOrigin, &gent_mat.w);
                Phys_Vec3ToNitrousVec(node->constant.vOrigin, &node_pos);
                v3 = phys_full_inv_multiply(&v4, &gent_mat, &node_pos);
                node_pos.x = v3->x;
                node_pos.y = v3->y;
                node_pos.z = v3->z;
                Phys_NitrousVecToVec3(&node_pos, origin_loc);
                setup_pathnode_parent(node, gentnum, origin_loc);
            }
        }
    }
}

void __cdecl setup_pathnode_parent(pathnode_t *node, int entnum, const float *origin_loc)
{
    pathnode_parent_t *node_parent; // [esp+14h] [ebp-4h]

    if ( (node->constant.spawnflags & 0x100) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    1217,
                    0,
                    "%s",
                    "node->constant.spawnflags & PNF_CAN_PARENT") )
    {
        __debugbreak();
    }
    //node_parent = phys_simple_allocator<pathnode_parent_t>::allocate(&g_pathnode_parent_allocator);
    node_parent = g_pathnode_parent_allocator.allocate();
    if ( !node_parent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 1219, 0, "%s", "node_parent") )
    {
        __debugbreak();
    }
    node_parent->origin_loc[0] = *origin_loc;
    node_parent->origin_loc[1] = origin_loc[1];
    node_parent->origin_loc[2] = origin_loc[2];
    node_parent->entnum = entnum;
    node_parent->m_node = node;
    node_parent->m_next = 0;
    generic_avl_map_add(&g_pathnode_parent_map, node_parent, (unsigned int)node);
}

bool __cdecl is_moving_entity(gentity_s *gent)
{
    return gent->s.eType == 6 || gent->s.eType == 14;
}

void __cdecl G_InitPathBaseNode(pathbasenode_t *pbnode, const pathnode_t *pnode)
{
    pbnode->vOrigin[0] = pnode->constant.vOrigin[0];
    pbnode->vOrigin[1] = pnode->constant.vOrigin[1];
    pbnode->vOrigin[2] = pnode->constant.vOrigin[2];
    pbnode->type = 1 << pnode->constant.type;
    if ( (pnode->constant.spawnflags & 1) != 0 )
        pbnode->type |= 0x200000u;
}

void __cdecl G_DropPathnodesToFloor()
{
    unsigned int backupEntContents[MAX_GENTITIES]; // [esp+Ch] [ebp-1008h]

    iassert(g_path.actualNodeCount <= gameWorldCurrent->path.nodeCount);

    for ( int i = 0; i < level.num_entities; ++i )
    {
        gentity_s *ent = &level.gentities[i];
        if ( ent->r.inuse )
        {
            backupEntContents[i] = ent->r.contents;
            if ( Path_IsDynamicBlockingEntity(ent) )
                ent->r.contents = 0;
        }
    }
    for ( unsigned int nodeIndex = 0; nodeIndex < gameWorldCurrent->path.nodeCount; ++nodeIndex )
    {
        if ( (gameWorldCurrent->path.nodes[nodeIndex].constant.spawnflags & 0x100) == 0 )
            G_DropPathNodeToFloor(nodeIndex);
    }
    for ( int i = 0; i < level.num_entities; ++i )
    {
        gentity_s *ent = &level.gentities[i];
        if ( ent->r.inuse )
            ent->r.contents = backupEntContents[i];
    }
    for (unsigned int nodeIndex = 0; nodeIndex < gameWorldCurrent->path.nodeCount; ++nodeIndex )
    {
        if ( (gameWorldCurrent->path.nodes[nodeIndex].constant.spawnflags & 0x100) != 0 )
            G_DropPathNodeToFloor(nodeIndex);
    }
}

void __cdecl G_SpawnPathnodeDynamic(SpawnVar *spawnVar)
{
    pathnode_dynamic_t *p_dynamic; // ecx
    pathnode_transient_t *p_transient; // eax
    void *DataForFile; // eax
    char *animscript; // [esp+28h] [ebp-18h]
    float forward[3]; // [esp+2Ch] [ebp-14h] BYREF
    nodeType type; // [esp+38h] [ebp-8h]
    pathnode_t *loadNode; // [esp+3Ch] [ebp-4h]

    if ( g_spawnai->current.enabled )
    {
        if ( g_path.actualNodeCount < 0x2000 )
        {
            if ( useFastFile->current.enabled && g_path.actualNodeCount == gameWorldCurrent->path.nodeCount )
            {
                ++g_path.extraNodes;
            }
            else
            {
                loadNode = &gameWorldCurrent->path.nodes[g_path.actualNodeCount++];
                if ( g_path.actualNodeCount > gameWorldCurrent->path.nodeCount )
                {
                    p_dynamic = &loadNode->dynamic;
                    SentientHandle shit; // lwss hack
                    shit.infoIndex = 0;
                    shit.number = 0;
                    //loadNode->dynamic.pOwner = 0;
                    loadNode->dynamic.pOwner = shit;
                    p_dynamic->iFreeTime = 0;
                    p_dynamic->iValidTime[0] = 0;
                    p_dynamic->iValidTime[1] = 0;
                    p_dynamic->iValidTime[2] = 0;
                    p_dynamic->inPlayerLOSTime = 0;
                    *(unsigned int *)&p_dynamic->wLinkCount = 0;
                    *(unsigned int *)&p_dynamic->turretEntNumber = 0;
                    p_transient = &loadNode->transient;
                    loadNode->transient.iSearchFrame = 0;
                    p_transient->pNextOpen = 0;
                    p_transient->pPrevOpen = 0;
                    p_transient->pParent = 0;
                    p_transient->fCost = 0.0;
                    p_transient->fHeuristic = 0.0;
                    p_transient->costFactor = 0.0;
                }
                G_ParsePathnodeScriptFields(loadNode, spawnVar);
                YawVectors(loadNode->constant.fAngle, forward, 0);
                *(_QWORD *)loadNode->constant.forward = *(_QWORD *)forward;
                type = loadNode->constant.type;
                if ( type >= NODE_NUMTYPES
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                                1547,
                                0,
                                "%s",
                                "type < NODE_NUMTYPES") )
                {
                    __debugbreak();
                }
                if ( type == NODE_NEGOTIATION_BEGIN )
                {
                    if ( loadNode->constant.animscript )
                    {
                        if ( g_connectpaths->current.integer < 2 && !r_reflectionProbeGenerate->current.enabled )
                        {
                            animscript = SL_ConvertToString(loadNode->constant.animscript, SCRIPTINSTANCE_SERVER);
                            DataForFile = Hunk_FindDataForFile(1, animscript);
                            loadNode->constant.animscriptfunc = (int)DataForFile;
                            if ( !loadNode->constant.animscriptfunc )
                            {
                                printf(
                                    "ERROR: Pathnode (%s) at (%g %g %g) cannot find animscript '%s'\n",
                                    nodeStringTable[type],
                                    loadNode->constant.vOrigin[0],
                                    loadNode->constant.vOrigin[1],
                                    loadNode->constant.vOrigin[2],
                                    animscript);
                                Com_PrintError(
                                    1,
                                    "ERROR: Pathnode (%s) at (%g %g %g) cannot find animscript '%s'\n",
                                    nodeStringTable[type],
                                    loadNode->constant.vOrigin[0],
                                    loadNode->constant.vOrigin[1],
                                    loadNode->constant.vOrigin[2],
                                    animscript);
                                loadNode->constant.type = NODE_BADNODE;
                            }
                        }
                    }
                    else
                    {
                        printf(
                            "ERROR: Pathnode (%s) at (%g %g %g) has no animscript specified\n",
                            nodeStringTable[type],
                            loadNode->constant.vOrigin[0],
                            loadNode->constant.vOrigin[1],
                            loadNode->constant.vOrigin[2]);
                        Com_PrintError(
                            1,
                            "ERROR: Pathnode (%s) at (%g %g %g) has no animscript specified\n",
                            nodeStringTable[type],
                            loadNode->constant.vOrigin[0],
                            loadNode->constant.vOrigin[1],
                            loadNode->constant.vOrigin[2]);
                        loadNode->constant.type = NODE_BADNODE;
                    }
                }
                else if ( type == NODE_TURRET )
                {
                    loadNode->dynamic.turretEntNumber = -1;
                }
            }
        }
        else
        {
            Com_Printf(18, "PATH_MAX_NODES (%i) exceeded.    Nodecount: %d\n", 0x2000, ++overCount + g_path.actualNodeCount);
        }
    }
}

void __cdecl G_ParsePathnodeScriptFields(pathnode_t *node, SpawnVar *spawnVar)
{
    int i; // [esp+0h] [ebp-4h]

    if ( (!spawnVar || !spawnVar->spawnVarsValid)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    915,
                    0,
                    "%s",
                    "spawnVar && spawnVar->spawnVarsValid") )
    {
        __debugbreak();
    }
    for ( i = 0; i < spawnVar->numSpawnVars; ++i )
        G_ParsePathnodeScriptField(spawnVar->spawnVars[i][0], spawnVar->spawnVars[i][1], node);
}

void __cdecl G_ParsePathnodeScriptField(const char *key, char *value, pathnode_t *node)
{
    node_field_t *f; // [esp+4h] [ebp-4h]

    if ( useFastFile->current.enabled )
    {
        PathNode_UpdateStringField("target", &node->constant.target, key, value);
        PathNode_UpdateStringField("targetname", &node->constant.targetname, key, value);
        PathNode_UpdateStringField("script_linkname", &node->constant.script_linkName, key, value);
        PathNode_UpdateStringField("script_noteworthy", &node->constant.script_noteworthy, key, value);
        PathNode_UpdateFloatField("radius", &node->constant.fRadius, key, value);
        if ( !I_stricmp(key, "origin") )
            PathNode_OriginMatches(value, node->constant.vOrigin);
    }
    for ( f = fields_3; f->name; ++f )
    {
        if ( !I_stricmp(f->name, key) )
            return;
    }
    G_SetPathnodeScriptVariable(key, value, node);
}

void __cdecl G_SetPathnodeScriptVariable(const char *key, char *value, pathnode_t *ent)
{
    int v3; // eax
    const char *v4; // eax
    float v5; // [esp+8h] [ebp-10h]
    unsigned int index; // [esp+10h] [ebp-8h]
    int type; // [esp+14h] [ebp-4h] BYREF

    index = Scr_FindField(key, &type, SCRIPTINSTANCE_SERVER);
    if ( index )
    {
        switch ( type )
        {
            case 2:
                Scr_AddString(value, SCRIPTINSTANCE_SERVER);
                goto LABEL_10;
            case 4:
                Scr_Error("G_SetPathnodeScriptVariable: vector is an unsupported script variable type for pathnodes", 0);
                goto LABEL_10;
            case 5:
                v5 = atof(value);
                Scr_AddFloat(v5, SCRIPTINSTANCE_SERVER);
                goto LABEL_10;
            case 6:
                v3 = atoi(value);
                Scr_AddInt(v3, SCRIPTINSTANCE_SERVER);
LABEL_10:
                GScr_SetDynamicPathnodeField(ent, index);
                break;
            default:
                v4 = va("G_SetPathnodeScriptVariable: bad case %d", type);
                if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 801, 0, v4) )
                    __debugbreak();
                break;
        }
    }
}

void __cdecl PathNode_UpdateStringField(
                const char *destKey,
                unsigned __int16 *destScrString,
                const char *key,
                char *value)
{
    char *destString; // [esp+0h] [ebp-4h]

    if ( !key && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 833, 0, "%s", "key") )
        __debugbreak();
    if ( !destKey && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 834, 0, "%s", "destKey") )
        __debugbreak();
    if ( !I_stricmp(key, destKey) )
    {
        destString = SL_ConvertToString(*destScrString, SCRIPTINSTANCE_SERVER);
        if ( !destString || I_stricmp(destString, value) )
            *destScrString = SL_GetString(value, 1u, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl PathNode_UpdateFloatField(const char *destKey, float *destFloat, const char *key, const char *value)
{
    float floatValue; // [esp+0h] [ebp-4h] BYREF

    iassert(key);
    iassert(destKey);

    if ( !I_stricmp(key, destKey) )
    {
        if ( sscanf(value, "%f", &floatValue) != 1 )
            Com_Error(ERR_DROP, "Malformed '%s' float field for node '%s'\n", key, value);
        *destFloat = floatValue;
    }
}

void __cdecl PathNode_OriginMatches(const char *value, const float *nodeOrigin)
{
    float origin[3]; // [esp+0h] [ebp-Ch] BYREF

    if ( sscanf(value, "%f %f %f", origin, &origin[1], &origin[2]) != 3 )
        Com_Error(ERR_DROP, "Malformed origin for path node '%s'\n", value);
    if ( origin[0] != *nodeOrigin || origin[1] != nodeOrigin[1] )
        ++g_path.originErrors;
}

void __cdecl GScr_AddFieldsForPathnode()
{
    for ( node_field_t *f = fields_3; f->name; ++f )
    {
        iassert((f - fields_3) == (unsigned short)(f - fields_3));
        Scr_AddClassField(2, (char *)f->name, (unsigned __int16)(f - fields_3), SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl Scr_FreePathnode(pathnode_t *node)
{
    if ( !useFastFile->current.enabled )
        Scr_FreePathnodeFields(node);

    iassert(!node->dynamic.pOwner.isDefined());

    Scr_FreeEntityNum(Path_ConvertNodeToIndex(node), 2u, SCRIPTINSTANCE_SERVER);
}

void __cdecl Scr_AddPathnode(pathnode_t *node)
{
    Scr_AddEntityNum(Path_ConvertNodeToIndex(node), 2u, SCRIPTINSTANCE_SERVER, 0);
}

void __cdecl Scr_GetNode()
{
    pathnode_t *node; // [esp+0h] [ebp-20h]
    pathnode_t *result; // [esp+4h] [ebp-1Ch]
    unsigned int name; // [esp+Ch] [ebp-14h]
    int offset; // [esp+10h] [ebp-10h]
    char *key; // [esp+14h] [ebp-Ch]
    node_field_t *f; // [esp+18h] [ebp-8h]

    name = Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    key = Scr_GetString(1u, SCRIPTINSTANCE_SERVER);
    offset = Scr_GetOffset(2u, key, SCRIPTINSTANCE_SERVER);
    if ( offset >= 0 )
    {
        if ( (unsigned int)offset >= 0xB
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                        1656,
                        0,
                        "offset doesn't index ARRAY_COUNT( fields ) - 1\n\t%i not in [0, %i)",
                        offset,
                        11) )
        {
            __debugbreak();
        }
        f = &fields_3[offset];
        if ( f->type != F_STRING )
            Scr_ParamError(1u, "key is not internally a string", SCRIPTINSTANCE_SERVER);
        result = 0;
        for ( node = gameWorldCurrent->path.nodes; node != &gameWorldCurrent->path.nodes[g_path.actualNodeCount]; ++node )
        {
            if ( *(_WORD *)((char *)&node->constant.type + f->ofs)
                && *(unsigned __int16 *)((char *)&node->constant.type + f->ofs) == name )
            {
                if ( result )
                    Scr_Error("getnode used with more than one node", 0);
                result = node;
            }
        }
        if ( result )
            Scr_AddPathnode(result);
    }
}

void __cdecl Scr_GetNodeArray()
{
    const char *v0; // eax
    pathnode_t *node; // [esp+0h] [ebp-1Ch]
    unsigned int name; // [esp+8h] [ebp-14h]
    int offset; // [esp+Ch] [ebp-10h]
    char *key; // [esp+10h] [ebp-Ch]
    node_field_t *f; // [esp+14h] [ebp-8h]

    name = Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    key = Scr_GetString(1u, SCRIPTINSTANCE_SERVER);
    offset = Scr_GetOffset(2u, key, SCRIPTINSTANCE_SERVER);
    if ( offset < 0 )
    {
        v0 = va("key '%s' does not internally belong to nodes", key);
        Scr_ParamError(1u, v0, SCRIPTINSTANCE_SERVER);
    }
    if ( (unsigned int)offset >= 0xB
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    1709,
                    0,
                    "offset doesn't index ARRAY_COUNT( fields ) - 1\n\t%i not in [0, %i)",
                    offset,
                    11) )
    {
        __debugbreak();
    }
    f = &fields_3[offset];
    if ( f->type != F_STRING )
        Scr_ParamError(1u, "key is not internally a string", SCRIPTINSTANCE_SERVER);
    Scr_MakeArray(SCRIPTINSTANCE_SERVER);
    for ( node = gameWorldCurrent->path.nodes; node != &gameWorldCurrent->path.nodes[g_path.actualNodeCount]; ++node )
    {
        if ( *(_WORD *)((char *)&node->constant.type + f->ofs) )
        {
            if ( *(unsigned __int16 *)((char *)&node->constant.type + f->ofs) == name )
            {
                Scr_AddPathnode(node);
                Scr_AddArray(SCRIPTINSTANCE_SERVER);
            }
        }
    }
}

void __cdecl Scr_GetAllNodes()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    Scr_MakeArray(SCRIPTINSTANCE_SERVER);
    for ( i = 0; i < g_path.actualNodeCount; ++i )
    {
        Scr_AddPathnode(&gameWorldCurrent->path.nodes[i]);
        Scr_AddArray(SCRIPTINSTANCE_SERVER);
    }
}

pathnode_t *__cdecl Scr_GetPathnode(unsigned int index, scriptInstance_t inst)
{
    scr_entref_t v3; // [esp+0h] [ebp-18h] BYREF
    scr_entref_t v4; // [esp+Ah] [ebp-Eh]
    scr_entref_t entref; // [esp+10h] [ebp-8h]

    //v4 = *Scr_GetEntityRef(&v3, index, inst);
    v4 = Scr_GetEntityRef(index, inst);
    entref = v4;
    if ( v4.classnum == 2 )
    {
        if ( entref.entnum >= g_path.actualNodeCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                        1908,
                        0,
                        "entref.entnum doesn't index g_path.actualNodeCount\n\t%i not in [0, %i)",
                        entref.entnum,
                        g_path.actualNodeCount) )
        {
            __debugbreak();
        }
        return &gameWorldCurrent->path.nodes[entref.entnum];
    }
    else
    {
        Scr_ParamError(index, "not a pathnode", inst);
        return 0;
    }
}

bool __cdecl Path_CompareNodesIncreasing(const pathsort_t &ps1, const pathsort_t &ps2)
{
    if ( ps1.node->dynamic.wLinkCount )
    {
        if ( !ps2.node->dynamic.wLinkCount )
            return 1;
    }
    else if ( ps2.node->dynamic.wLinkCount )
    {
        return 0;
    }
    return ps2.metric > ps1.metric;
}

unsigned int __cdecl Path_ConvertNodeToIndex(const pathnode_t *node)
{
    unsigned int nodeIndex; // [esp+0h] [ebp-4h]

    if ( !node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 1951, 0, "%s", "node") )
        __debugbreak();
    nodeIndex = node - gameWorldCurrent->path.nodes;
    if ( nodeIndex >= g_path.actualNodeCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    1954,
                    0,
                    "nodeIndex doesn't index g_path.actualNodeCount\n\t%i not in [0, %i)",
                    nodeIndex,
                    g_path.actualNodeCount) )
    {
        __debugbreak();
    }
    return nodeIndex;
}

pathnode_t *__cdecl Path_ConvertIndexToNode(unsigned int index)
{
    if ( index >= g_path.actualNodeCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    1961,
                    0,
                    "index doesn't index g_path.actualNodeCount\n\t%i not in [0, %i)",
                    index,
                    g_path.actualNodeCount) )
    {
        __debugbreak();
    }
    return &gameWorldCurrent->path.nodes[index];
}

void __cdecl Path_Init(int restart)
{
    char *String; // eax

    if ( !useFastFile->current.enabled )
    {
        String = (char *)Dvar_GetString("mapname");
        gameWorldCurrent->name = Com_GetHunkStringCopy(String);
        Path_InitStatic(restart);
        if ( !restart )
            Path_CreateNodes();
    }
    memset(&g_path, 0, sizeof(g_path));
    if ( !gameWorldCurrent->path.nodes
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    1986,
                    0,
                    "%s",
                    "gameWorldCurrent.path.nodes") )
    {
        __debugbreak();
    }
    debugPath = 0;
    Path_InitBadPlaces();
}

void __cdecl Path_Shutdown()
{
    pathnode_t *node; // [esp+4h] [ebp-4h]
    pathnode_t *nodea; // [esp+4h] [ebp-4h]

    Path_ShutdownBadPlaces();
    if ( useFastFile->current.enabled )
    {
        for ( node = gameWorldCurrent->path.nodes; node != &gameWorldCurrent->path.nodes[g_path.actualNodeCount]; ++node )
        {
            PathNode_ClearStringField(&node->constant.target);
            PathNode_ClearStringField(&node->constant.targetname);
            PathNode_ClearStringField(&node->constant.script_linkName);
            PathNode_ClearStringField(&node->constant.script_noteworthy);
        }
        g_path.extraNodes = 0;
        g_path.originErrors = 0;
    }
    for ( nodea = gameWorldCurrent->path.nodes; nodea != &gameWorldCurrent->path.nodes[g_path.actualNodeCount]; ++nodea )
        Scr_FreePathnode(nodea);
    g_path.actualNodeCount = 0;
    debugPath = 0;
    if ( !useFastFile->current.enabled )
        Path_ShutdownStatic();
}

void __cdecl PathNode_ClearStringField(unsigned __int16 *destScrString)
{
    if ( *destScrString )
    {
        if ( (SL_GetUser(*destScrString, SCRIPTINSTANCE_SERVER) & 1) != 0 )
            *destScrString = 0;
    }
}

void __cdecl Path_NodesInCylinder_process(pathnode_t *pnode)
{
    float distSq; // [esp+Ch] [ebp-4h]

    distSq = (float)((float)(pnode->constant.vOrigin[0] - g_path.circle.origin[0])
                                 * (float)(pnode->constant.vOrigin[0] - g_path.circle.origin[0]))
                 + (float)((float)(pnode->constant.vOrigin[1] - g_path.circle.origin[1])
                                 * (float)(pnode->constant.vOrigin[1] - g_path.circle.origin[1]));
    if ( g_path.circle.maxDistSq >= distSq
        && (g_path.circle.typeFlags & (1 << pnode->constant.type)) != 0
        && ((pnode->constant.spawnflags & 1) == 0 || (g_path.circle.typeFlags & 1) != 0)
        && (float)((float)(pnode->constant.vOrigin[2] - g_path.circle.origin[2])
                         * (float)(pnode->constant.vOrigin[2] - g_path.circle.origin[2])) <= g_path.circle.maxHeightSq )
    {
        if ( g_path.circle.nodeCount >= g_path.circle.maxNodes
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                        2119,
                        0,
                        "%s",
                        "g_path.circle.nodeCount < g_path.circle.maxNodes") )
        {
            __debugbreak();
        }
        g_path.circle.nodes[g_path.circle.nodeCount].node = pnode;
        g_path.circle.nodes[g_path.circle.nodeCount++].metric = distSq;
    }
}

bool first_time_called_0 = true;
void __cdecl Path_MovingNodesInCylinder()
{
    generic_avl_map_node_t *m_tree_root; // [esp+0h] [ebp-106Ch]
    void *j; // [esp+18h] [ebp-1054h]
    unsigned int v3; // [esp+20h] [ebp-104Ch]
    int i; // [esp+24h] [ebp-1048h]
    int entityList[1025]; // [esp+4Ch] [ebp-1020h] BYREF
    float maxs[3]; // [esp+1050h] [ebp-1Ch] BYREF
    int v7; // [esp+105Ch] [ebp-10h]
    float mins[3]; // [esp+1060h] [ebp-Ch] BYREF

    if (enable_moving_paths->current.integer == 1 && g_pathnode_parent_allocator.m_count > 0)
    {
        if (first_time_called_0)
        {
            first_time_called_0 = 0;
            adjust_mins[0] = -15.0 - 2.0;
            adjust_mins[1] = -15.0 - 2.0;
            adjust_mins[2] = 0.0 - 2.0;
            adjust_maxs[0] = 2.0 + 15.0;
            adjust_maxs[1] = 2.0 + 15.0;
            adjust_maxs[2] = (float)((float)(15.0 - -15.0) + 0.0) + 2.0;
        }
        mins[0] = (float)((-(g_path.circle.maxDist)) + g_path.circle.origin[0]) + adjust_mins[0];
        mins[1] = (float)((-(g_path.circle.maxDist)) + g_path.circle.origin[1]) + adjust_mins[1];
        mins[2] = (float)(g_path.circle.origin[2] + 0.0) + adjust_mins[2];
        maxs[0] = (float)(g_path.circle.maxDist + g_path.circle.origin[0]) + adjust_maxs[0];
        maxs[1] = (float)(g_path.circle.maxDist + g_path.circle.origin[1]) + adjust_maxs[1];
        maxs[2] = (float)(g_path.circle.maxHeight + g_path.circle.origin[2]) + adjust_maxs[2];
        entityList[1024] = (int)0x82000C + 5;
        v7 = CM_AreaEntities(mins, maxs, entityList, 1024, (int)0x82000C + 5);
        for (i = 0; i < v7; ++i)
        {
            v3 = entityList[i];
            m_tree_root = g_parented_pathnode_list_map.m_tree_root;
            while (m_tree_root && v3 != m_tree_root->m_avl_key)
            {
                if (v3 >= m_tree_root->m_avl_key)
                    m_tree_root = m_tree_root->m_avl_tree_node.m_right;
                else
                    m_tree_root = m_tree_root->m_avl_tree_node.m_left;
            }
            if (m_tree_root)
            {
                for (j = m_tree_root->m_data; j; j = (void *)*((_DWORD *)j + 5))
                    Path_NodesInCylinder_process(*((pathnode_t **)j + 4));
            }
        }
    }
}

int __cdecl Path_NodesInCylinder(
                const float *origin,
                float maxDist,
                float maxHeight,
                pathsort_t *nodes,
                int maxNodes,
                int typeFlags)
{
    if ( !gameWorldCurrent->path.nodeTree )
        return 0;
    g_path.circle.origin[0] = *origin;
    g_path.circle.origin[1] = origin[1];
    g_path.circle.origin[2] = origin[2];
    g_path.circle.maxDist = maxDist;
    g_path.circle.maxDistSq = maxDist * maxDist;
    g_path.circle.maxHeight = maxHeight;
    g_path.circle.maxHeightSq = maxHeight * maxHeight;
    g_path.circle.typeFlags = typeFlags;
    g_path.circle.nodes = nodes;
    g_path.circle.maxNodes = maxNodes;
    g_path.circle.nodeCount = 0;
    Path_NodesInCylinder_r(gameWorldCurrent->path.nodeTree);
    Path_MovingNodesInCylinder();
    return g_path.circle.nodeCount;
}

void __cdecl Path_NodesInCylinder_r(pathnode_tree_t *tree)
{
    pathnode_t *pnode; // [esp+8h] [ebp-10h]
    int i; // [esp+Ch] [ebp-Ch]
    unsigned __int16 *leafNodes; // [esp+10h] [ebp-8h]
    float dist; // [esp+14h] [ebp-4h]

    while ( tree->axis >= 0 )
    {
        dist = g_path.circle.origin[tree->axis] - tree->dist;
        if ( g_path.circle.maxDist < dist )
            goto LABEL_6;
        if ( (-(g_path.circle.maxDist)) <= dist )
        {
            Path_NodesInCylinder_r(tree->u.child[0]);
LABEL_6:
            tree = tree->u.child[1];
        }
        else
        {
            tree = tree->u.child[0];
        }
    }
    leafNodes = tree->u.s.nodes;
    for ( i = 0; i < tree->u.s.nodeCount && g_path.circle.nodeCount < g_path.circle.maxNodes; ++i )
    {
        _mm_prefetch((const char *)&gameWorldCurrent->path.nodes[leafNodes[i]], 1);
        pnode = &gameWorldCurrent->path.nodes[leafNodes[i]];
        if ( get_pathnode_parent(pnode)->entnum == 1022 )
            Path_NodesInCylinder_process(pnode);
    }
}

int __cdecl Path_NodesInRadius(const float *origin, float maxDist, pathsort_t *nodes, int maxNodes, int typeFlags)
{
    return Path_NodesInCylinder(origin, maxDist, 1000000000.0, nodes, maxNodes, typeFlags);
}

int __cdecl Path_IsDynamicBlockingEntity(gentity_s *ent)
{
    return ent->flags & 0x800;
}

bool __cdecl Path_IsBadPlaceLink(unsigned int nodeNumFrom, unsigned int nodeNumTo, team_t eTeam)
{
    const char *v4; // eax
    pathnode_t *from; // [esp+0h] [ebp-8h]
    unsigned int i; // [esp+4h] [ebp-4h]

    if ( nodeNumFrom >= g_path.actualNodeCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    2272,
                    0,
                    "nodeNumFrom doesn't index g_path.actualNodeCount\n\t%i not in [0, %i)",
                    nodeNumFrom,
                    g_path.actualNodeCount) )
    {
        __debugbreak();
    }
    if ( nodeNumTo >= g_path.actualNodeCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    2273,
                    0,
                    "nodeNumTo doesn't index g_path.actualNodeCount\n\t%i not in [0, %i)",
                    nodeNumTo,
                    g_path.actualNodeCount) )
    {
        __debugbreak();
    }
    if ( (unsigned int)eTeam >= TEAM_NUM_TEAMS
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    2274,
                    0,
                    "eTeam doesn't index ARRAY_COUNT( ((pathlink_t *) 0)->ubBadPlaceCount )\n\t%i not in [0, %i)",
                    eTeam,
                    4) )
    {
        __debugbreak();
    }
    from = &gameWorldCurrent->path.nodes[nodeNumFrom];
    for ( i = 0; i < from->constant.totalLinkCount; ++i )
    {
        if ( from->constant.Links[i].nodeNum == nodeNumTo )
            return from->constant.Links[i].ubBadPlaceCount[eTeam] != 0;
    }
    v4 = va("Path_IsBadPlaceLink called from %i to %i, but there is no such link", nodeNumFrom, nodeNumTo);
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 2283, 0, v4) )
        __debugbreak();
    return 0;
}

void __cdecl Path_AutoDisconnectPaths()
{
    gentity_s *ent; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Path_ValidateAllNodes();
    for ( i = 0; i < level.num_entities; ++i )
    {
        ent = &level.gentities[i];
        if ( ent->r.inuse && Path_IsDynamicBlockingEntity(ent) && (ent->flags & 0x40000000) != 0 )
            Path_DisconnectPathsForEntity(ent);
    }
}

void __cdecl Path_PreSpawnInitPaths()
{
    Path_InitNodesDynamic();
    Path_InitLinkCounts();
}

unsigned int Path_InitLinkCounts()
{
    unsigned int result; // eax
    unsigned int j; // [esp+4h] [ebp-10h]
    unsigned int k; // [esp+8h] [ebp-Ch]
    unsigned int i; // [esp+10h] [ebp-4h]

    for ( i = 0; i < gameWorldCurrent->path.nodeCount; ++i )
    {
        gameWorldCurrent->path.nodes[i].dynamic.wLinkCount = gameWorldCurrent->path.nodes[i].constant.totalLinkCount;
        for ( j = 0; j < gameWorldCurrent->path.nodes[i].constant.totalLinkCount; ++j )
        {
            for ( k = 0; k < 4; ++k )
                gameWorldCurrent->path.nodes[i].constant.Links[j].ubBadPlaceCount[k] = 0;
        }
        result = i + 1;
    }
    return result;
}

unsigned int Path_InitNodesDynamic()
{
    unsigned int result; // eax
    unsigned int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < gameWorldCurrent->path.nodeCount; ++i )
    {
        Path_InitNodeDynamic(&gameWorldCurrent->path.nodes[i]);
        result = i + 1;
    }
    return result;
}

void __cdecl Path_InitNodeDynamic(pathnode_t *loadNode)
{
    float forward[3]; // [esp+10h] [ebp-Ch] BYREF

    SentientHandle shit; // lwss hack
    shit.number = 0;
    shit.infoIndex = 0;

    //loadNode->dynamic.pOwner = 0;
    loadNode->dynamic.pOwner = shit;
    loadNode->dynamic.iFreeTime = 0;
    loadNode->dynamic.iValidTime[0] = 0;
    loadNode->dynamic.iValidTime[1] = 0;
    loadNode->dynamic.iValidTime[2] = 0;
    loadNode->dynamic.inPlayerLOSTime = 0;
    *(unsigned int *)&loadNode->dynamic.wLinkCount = 0;
    *(unsigned int *)&loadNode->dynamic.turretEntNumber = 0;
    loadNode->transient.iSearchFrame = 0;
    loadNode->transient.pNextOpen = 0;
    loadNode->transient.pPrevOpen = 0;
    loadNode->transient.pParent = 0;
    loadNode->transient.fCost = 0.0;
    loadNode->transient.fHeuristic = 0.0;
    loadNode->transient.costFactor = 0.0;
    YawVectors(loadNode->constant.fAngle, forward, 0);
    *(_QWORD *)loadNode->constant.forward = *(_QWORD *)forward;
    loadNode->dynamic.turretEntNumber = -1;
}

void __cdecl Path_InitPaths()
{
    _iobuf *v0; // eax

    if ( !useFastFile->current.enabled )
    {
        Path_BuildChains();
        if ( g_connectpaths->current.integer )
            Path_ConnectPaths();
        else
            Path_LoadPaths();
        Path_InitLinkCounts();
    }
    Path_InitLinkInfoArray();
    if ( !useFastFile->current.enabled && Path_FindOverlappingNodes() && g_connectpaths->current.integer >= 2 )
    {
        printf("FATAL ERROR: Overlapping path nodes. Check console log.\n");
        Com_Error(ERR_FATAL, "Overlapping path nodes. Check console log.");
    }
    Path_ValidateAllNodes();
    if ( g_connectpaths->current.integer )
    {
        Path_CheckSpawnExitNodesConnectivity();
        Path_SavePaths();
        if ( g_connectpaths->current.integer >= 2 )
        {
            Sys_NormalExit();
            //v0 = __iob_func();
            //fflush(v0 + 1);
            fflush(stdout);
            ExitProcess(0);
        }
        Dvar_SetInt((dvar_s *)g_connectpaths, 0);
    }
}

int Path_InitLinkInfoArray()
{
    int result; // eax
    unsigned __int16 v1; // [esp+0h] [ebp-Ch]
    unsigned __int16 v2; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    g_path.pathLinkInfoArrayInited = 1;
    for ( i = 0; i < 2048; ++i )
    {
        if ( i )
            v2 = i - 1;
        else
            v2 = 2047;
        g_path.pathLinkInfoArray[i].prev = v2;
        if ( i == 2047 )
            v1 = 0;
        else
            v1 = i + 1;
        g_path.pathLinkInfoArray[i].next = v1;
        result = i + 1;
    }
    return result;
}

void __cdecl Path_CheckSpawnExitNodesConnectivity()
{
    int exitCounter; // [esp+30h] [ebp-724h]
    int spawnCounter; // [esp+34h] [ebp-720h]
    unsigned int nodeCount; // [esp+38h] [ebp-71Ch]
    int exitCount; // [esp+3Ch] [ebp-718h]
    int spawnCount; // [esp+40h] [ebp-714h]
    path_t spawn_exit_path; // [esp+44h] [ebp-710h] BYREF
    pathnode_t *spawnNodes[100]; // [esp+42Ch] [ebp-328h]
    int connectionFlag; // [esp+5BCh] [ebp-198h]
    team_t eTeam; // [esp+5C0h] [ebp-194h]
    pathnode_t *exitNodes[100]; // [esp+5C4h] [ebp-190h]

    eTeam = TEAM_ALLIES;
    connectionFlag = 0;
    Path_Begin(&spawn_exit_path);
    spawnCount = 0;
    exitCount = 0;
    for ( nodeCount = 0; nodeCount < g_path.actualNodeCount; ++nodeCount )
    {
        if ( spawnCount >= 100
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                        2487,
                        0,
                        "%s",
                        "spawnCount < MAX_SPAWN_EXIT_NODES") )
        {
            __debugbreak();
        }
        if ( exitCount >= 100
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                        2488,
                        0,
                        "%s",
                        "exitCount < MAX_SPAWN_EXIT_NODES") )
        {
            __debugbreak();
        }
        if ( scr_const.spawn == gameWorldCurrent->path.nodes[nodeCount].constant.script_noteworthy )
            spawnNodes[spawnCount++] = &gameWorldCurrent->path.nodes[nodeCount];
        if ( scr_const.exit == gameWorldCurrent->path.nodes[nodeCount].constant.script_noteworthy )
            exitNodes[exitCount++] = &gameWorldCurrent->path.nodes[nodeCount];
    }
    if ( !spawnCount )
        Com_PrintWarning(18, "WARNING: %s \n", "No Spawn Nodes in the Map");
    if ( !exitCount )
        Com_PrintWarning(18, "WARNING: %s \n", "No Exit Nodes in the Map");
    for ( spawnCounter = 0; spawnCounter < spawnCount; ++spawnCounter )
    {
        for ( exitCounter = 0; exitCounter < exitCount; ++exitCounter )
        {
            connectionFlag = Path_FindPathFromTo(
                                                 &spawn_exit_path,
                                                 eTeam,
                                                 spawnNodes[spawnCounter],
                                                 spawnNodes[spawnCounter]->constant.vOrigin,
                                                 exitNodes[exitCounter],
                                                 exitNodes[exitCounter]->constant.vOrigin,
                                                 1,
                                                 0);
            if ( !connectionFlag )
                Com_PrintError(
                    1,
                    "ERROR: Dog at SpawnNode (%g %g %g) is not connected to ExitNode (%g %g %g)\n",
                    spawnNodes[spawnCounter]->constant.vOrigin[0],
                    spawnNodes[spawnCounter]->constant.vOrigin[1],
                    spawnNodes[spawnCounter]->constant.vOrigin[2],
                    exitNodes[exitCounter]->constant.vOrigin[0],
                    exitNodes[exitCounter]->constant.vOrigin[1],
                    exitNodes[exitCounter]->constant.vOrigin[2]);
        }
    }
}

void __cdecl Path_DrawDebugNoLinks(const pathnode_t *node, const float (*color)[4], int duration)
{
    float vOrg; // [esp+1Ch] [ebp-24h]
    float vOrg_4; // [esp+20h] [ebp-20h]
    float vOrg_8; // [esp+24h] [ebp-1Ch]
    float vEnd[3]; // [esp+28h] [ebp-18h] BYREF
    float vStart[3]; // [esp+34h] [ebp-Ch] BYREF

    vOrg = node->constant.vOrigin[0];
    vOrg_4 = node->constant.vOrigin[1];
    vOrg_8 = (float)(0.0 + 1.0) + node->constant.vOrigin[2];
    vStart[0] = (float)(8.0 * 0.86602539) + vOrg;
    vStart[1] = (float)(8.0 * 0.5) + vOrg_4;
    vStart[2] = (float)(8.0 * 0.0) + vOrg_8;
    vEnd[0] = (float)((-(8.0f)) * 0.86602539) + vOrg;
    vEnd[1] = (float)((-(8.0f)) * 0.5) + vOrg_4;
    vEnd[2] = (float)((-(8.0f)) * 0.0) + vOrg_8;
    CG_DebugLine(vStart, vEnd, (const float *)color, 0, duration);
    vStart[0] = (float)(8.0 * -0.5) + vOrg;
    vStart[1] = (float)(8.0 * 0.86602539) + vOrg_4;
    vStart[2] = (float)(8.0 * 0.0) + vOrg_8;
    vEnd[0] = (float)((-(8.0f)) * -0.5) + vOrg;
    vEnd[1] = (float)((-(8.0f)) * 0.86602539) + vOrg_4;
    vEnd[2] = (float)((-(8.0f)) * 0.0) + vOrg_8;
    CG_DebugLine(vStart, vEnd, (const float *)color, 0, duration);
}

void __cdecl Path_DrawDebugNode(const float *cameraPos, const pathnode_t *node)
{
    char *v4; // [esp+14h] [ebp-178h]
    char *v6; // [esp+1Ch] [ebp-170h]
    char *v8; // [esp+24h] [ebp-168h]
    char *v10; // [esp+2Ch] [ebp-160h]
    char *v12; // [esp+34h] [ebp-158h]
    char *v14; // [esp+3Ch] [ebp-150h]
    char *v16; // [esp+44h] [ebp-148h]
    char *v18; // [esp+4Ch] [ebp-140h]
    char *v20; // [esp+54h] [ebp-138h]
    float boxsizeMin[3]; // [esp+68h] [ebp-124h] BYREF
    char temp[260]; // [esp+74h] [ebp-118h] BYREF
    float org[3]; // [esp+17Ch] [ebp-10h] BYREF
    float scale; // [esp+188h] [ebp-4h]

    boxsizeMin[2] = 0.0f;
    if (!cameraPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 2736, 0, "%s", "cameraPos"))
    {
        __debugbreak();
    }
    if (!node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 2737, 0, "%s", "node"))
        __debugbreak();
    Path_DrawDebugNodeBox(node);
    org[0] = node->constant.vOrigin[0];
    org[1] = node->constant.vOrigin[1];
    org[2] = node->constant.vOrigin[2];
    org[2] = (float)((float)(16.0 - boxsizeMin[2]) * 0.5) + org[2];
    scale = Path_GetDebugStringScale(cameraPos, org);
    sprintf(temp, "%s", nodeStringTable[node->constant.type]);
    if (!IsNodeEnabled(node))
    {
        v20 = (char *)&boxsizeMin[2] + 3;
        while (*++v20)
            ;
        strcpy(v20, ": DL");
    }
    if ((node->constant.spawnflags & 4) != 0)
    {
        v18 = (char *)&boxsizeMin[2] + 3;
        while (*++v18)
            ;
        strcpy(v18, ":DS");
    }
    if ((node->constant.spawnflags & 8) != 0)
    {
        v16 = (char *)&boxsizeMin[2] + 3;
        while (*++v16)
            ;
        strcpy(v16, ":DC");
    }
    if ((node->constant.spawnflags & 0x10) != 0)
    {
        v14 = (char *)&boxsizeMin[2] + 3;
        while (*++v14)
            ;
        strcpy(v14, ":DP");
    }
    if ((node->constant.spawnflags & 0x800) != 0)
    {
        if (node->constant.type == NODE_COVER_PILLAR)
        {
            v12 = (char *)&boxsizeMin[2] + 3;
            while (*++v12)
                ;
            strcpy(v12, ":DR");
        }
        else if (node->constant.type == NODE_NEGOTIATION_BEGIN || node->constant.type == NODE_NEGOTIATION_END)
        {
            v10 = (char *)&boxsizeMin[2] + 3;
            while (*++v10)
                ;
            strcpy(v10, ":STRICT");
        }
        else
        {
            v8 = (char *)&boxsizeMin[2] + 3;
            while (*++v8)
                ;
            strcpy(v8, ":BYNR");
        }
    }
    if ((node->constant.spawnflags & 0x400) != 0)
    {
        if (node->constant.type == NODE_COVER_PILLAR)
        {
            v6 = (char *)&boxsizeMin[2] + 3;
            while (*++v6)
                ;
            strcpy(v6, ":DL");
        }
        else
        {
            v4 = (char *)&boxsizeMin[2] + 3;
            while (*++v4)
                ;
            strcpy(v4, ":BY");
        }
    }
    G_AddDebugString(org, colorYellow, scale, temp, 0);
}

double __cdecl Path_GetDebugStringScale(const float *cameraPos, const float *origin)
{
    double v2; // st7
    float delta[3]; // [esp+8h] [ebp-14h] BYREF
    float scale; // [esp+14h] [ebp-8h]
    gentity_s *player; // [esp+18h] [ebp-4h]

    if ( !cameraPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 2659, 0, "%s", "cameraPos") )
    {
        __debugbreak();
    }
    if ( !origin && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 2660, 0, "%s", "origin") )
        __debugbreak();
    scale = 1.0f;
    player = G_Find(0, 356, scr_const.player);
    if ( player )
    {
        delta[0] = *cameraPos - *origin;
        delta[1] = cameraPos[1] - origin[1];
        delta[2] = cameraPos[2] - origin[2];
        delta[2] = delta[2] + player->client->ps.viewHeightCurrent;
        v2 = Vec3Length(delta);
        scale = v2 / 3600.0 * scale;
    }
    if ( (float)(scale - 0.5) < 0.0 )
        return 0.5f;
    else
        return scale;
}

void Path_DrawDebugNodeBox_0(const pathnode_t *node, float size, const float *color_)
{
    float v4; // [esp+10h] [ebp-78h]
    float v5; // [esp+1Ch] [ebp-6Ch]
    float mins; // [esp+28h] [ebp-60h]
    float mins_4; // [esp+2Ch] [ebp-5Ch]
    float mins_8; // [esp+30h] [ebp-58h]
    float start[3]; // [esp+34h] [ebp-54h] BYREF
    float end[3]; // [esp+40h] [ebp-48h] BYREF
    float maxs[3]; // [esp+4Ch] [ebp-3Ch]
    float fCos; // [esp+58h] [ebp-30h]
    float fSin; // [esp+5Ch] [ebp-2Ch]
    float boxsizeMax[3]; // [esp+60h] [ebp-28h] BYREF
    float boxsizeMin[3]; // [esp+6Ch] [ebp-1Ch] BYREF
    float color[4]; // [esp+78h] [ebp-10h] BYREF

    if (get_pathnode_parent(node)->entnum == 1022)
    {
        color[0] = *color_;
        color[1] = color_[1];
        color[2] = color_[2];
        color[3] = color_[3];
    }
    else
    {
        color[0] = 0.0f;
        color[1] = 1.0f;
        color[2] = 0.0f;
        color[3] = 1.0f;
    }
    if (size > 0.0)
        v4 = size;
    else
        v4 = 16.0f;
    boxsizeMin[0] = -(v4);
    boxsizeMin[1] = -(v4);
    boxsizeMin[2] = 0.0f;
    boxsizeMax[0] = v4;
    boxsizeMax[1] = v4;
    boxsizeMax[2] = v4;
    if (!node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 2692, 0, "%s", "node"))
        __debugbreak();
    CG_DebugBox(node->constant.vOrigin, boxsizeMin, boxsizeMax, node->constant.fAngle, color, 1, 0);
    if (Path_IsValidClaimNode(node))
    {
        mins = node->constant.vOrigin[0] + boxsizeMin[0];
        mins_4 = node->constant.vOrigin[1] + boxsizeMin[1];
        mins_8 = node->constant.vOrigin[2] + boxsizeMin[2];
        maxs[0] = node->constant.vOrigin[0] + boxsizeMax[0];
        maxs[1] = node->constant.vOrigin[1] + boxsizeMax[1];
        maxs[2] = node->constant.vOrigin[2] + boxsizeMax[2];
        start[0] = 0.5 * (float)(mins + maxs[0]);
        start[1] = 0.5 * (float)(mins_4 + maxs[1]);
        start[2] = 0.5 * (float)(mins_8 + maxs[2]);
        v5 = node->constant.fAngle * 0.017453292;
        fCos = cos(v5);
        fSin = sin(v5);
        end[0] = (float)(boxsizeMax[0] * fCos) + start[0];
        end[1] = (float)(boxsizeMax[0] * fSin) + start[1];
        end[2] = start[2];
        G_DebugLine(start, end, color, 1);
    }
}

void __cdecl Path_DrawDebugNodeBox(const pathnode_t *node)
{
    Path_DrawDebugNodeBox_0(node, -1.0, nodeColorTable[node->constant.type]);
}

void __cdecl Path_DrawDebugNearestNode(const float *vOrigin)
{
    int i; // [esp+14h] [ebp-C10h]
    pathnode_t *node; // [esp+18h] [ebp-C0Ch]
    pathsort_t nodes[256]; // [esp+1Ch] [ebp-C08h] BYREF
    int nodeCount; // [esp+C20h] [ebp-4h] BYREF

    node = Path_NearestNode(
                     vOrigin,
                     nodes,
                     -1,
                     ai_showNodesDist->current.value,
                     &nodeCount,
                     256,
                     NEAREST_NODE_DONT_DO_HEIGHT_CHECK);
    if ( ai_showNearestNode->current.integer <= 1 )
    {
        if ( node )
            Path_DrawDebugNode(vOrigin, node);
        else
            G_DebugCircle(vOrigin, 192.0, colorRed, 1, 1, 0);
    }
    else
    {
        for ( i = 0; i < nodeCount && i < ai_showNearestNode->current.integer; ++i )
            Path_DrawDebugNode(vOrigin, nodes[i].node);
    }
}

void __cdecl Path_DrawDebugFindPath(const float *vOrigin)
{
    char *pszText; // [esp+10h] [ebp-44h]
    pathnode_t *node; // [esp+20h] [ebp-34h]
    float delta[2]; // [esp+24h] [ebp-30h] BYREF
    float p1[3]; // [esp+2Ch] [ebp-28h] BYREF
    float startPos[3]; // [esp+38h] [ebp-1Ch] BYREF
    float p2[3]; // [esp+44h] [ebp-10h] BYREF
    unsigned int i; // [esp+50h] [ebp-4h]

    switch (ai_debugFindPath->current.integer)
    {
    case 0:
        iValidBits = 0;
        return;
    case 1:
    case 4:
    case 5:
        vStartPos[0] = *vOrigin;
        vStartPos[1] = vOrigin[1];
        vStartPos[2] = vOrigin[2];
        iValidBits |= 1u;
        break;
    case 2:
        vGoalPos[0] = *vOrigin;
        vGoalPos[1] = vOrigin[1];
        vGoalPos[2] = vOrigin[2];
        iValidBits |= 2u;
        break;
    default:
        if (ai_debugFindPath->current.integer != 3)
        {
            Dvar_SetInt((dvar_s*)ai_debugFindPath, 0);
            Com_Printf(
                18,
                "^51 continuously copies your position to path start\n"
                "2 continuously copies your position to path goal\n"
                "3 doesn't change path start or path goal, but will still find a path\n"
                "4 continuously moves through the path simulating AI movement\n"
                "5 continuously moves through the path simulating AI movement, but continuously recalculates the path\n"
                "0 resets start and end and disables path finding\n"
                "While there are valid start and goal positions, it will find and draw a path\n");
        }
        break;
    }
    if (iValidBits == 3 && (vStartPos[0] != vGoalPos[0] || vStartPos[1] != vGoalPos[1] || vStartPos[2] != vGoalPos[2]))
    {
        if (!debugPath)
        {
            debugPath = &debugPathBuf;
            Path_Begin(&debugPathBuf);
            Path_FindPath(debugPath, TEAM_FREE, vStartPos, vGoalPos, 1);
        }
        if (ai_debugFindPath->current.integer != 4)
        {
            if (ai_debugFindPathDirect->current.enabled)
            {
                if (!Path_FindPath(debugPath, TEAM_FREE, vStartPos, vGoalPos, 1) || !Path_Exists(debugPath))
                    return;
            }
            else
            {
                if (!ai_debugFindPathLock->current.enabled)
                {
                    width = ai_debugFindPathWidth->current.value;
                    if (width <= 5000.0)
                    {
                        if (width < -5000.0)
                            width = -5000.0;
                    }
                    else
                    {
                        width = 5000.0f;
                    }
                    delta[0] = vStartPos[0] - vGoalPos[0];
                    delta[1] = vStartPos[1] - vGoalPos[1];
                    Vec2Normalize(delta);
                    //LODWORD(perp[0]) = LODWORD(delta[1]) ^ _mask__NegFloat_;
                    perp[0] = -delta[1];
                    perp[1] = delta[0];
                }
                startPos[2] = vGoalPos[2];
                startPos[0] = vStartPos[0];
                startPos[1] = vStartPos[1];
                G_DebugLine(startPos, vGoalPos, colorCyan, 0);
                p1[0] = (float)(startPos[0] - (float)(5000.0 * perp[1])) + (float)(perp[0] * width);
                p1[1] = (float)((float)(5000.0 * perp[0]) + startPos[1]) + (float)(perp[1] * width);
                p1[2] = startPos[2];
                p2[0] = (float)((float)(5000.0 * perp[1]) + startPos[0]) + (float)(perp[0] * width);
                p2[1] = (float)(startPos[1] - (float)(5000.0 * perp[0])) + (float)(perp[1] * width);
                p2[2] = startPos[2];
                G_DebugLine(p1, p2, colorCyan, 0);
                if (!Path_FindPathWithWidth(debugPath, TEAM_FREE, vStartPos, vGoalPos, 1, width, perp)
                    || !Path_Exists(debugPath))
                {
                    return;
                }
            }
            for (i = 0; i < g_path.actualNodeCount; ++i)
            {
                node = &gameWorldCurrent->path.nodes[i];
                if (node->transient.iSearchFrame == level.iSearchFrame)
                {
                    pszText = va("%i", i);
                    G_AddDebugString(node->constant.vOrigin, colorWhite, 1.0, pszText, 0);
                }
            }
        }
        if (ai_debugFindPath->current.integer == 4 || ai_debugFindPath->current.integer == 5)
        {
            Path_UpdateLookahead(debugPath, vStartPos, 0, 0, 1, 1);
            Path_DebugDraw(debugPath, vStartPos, 1, -1);
        }
        else
        {
            Path_DebugDraw(debugPath, vStartPos, 0, -1);
        }
    }
}

void __cdecl Path_DrawVisData()
{
    float v0; // [esp+0h] [ebp-34h]
    float v[2]; // [esp+10h] [ebp-24h] BYREF
    const pathnode_t *node; // [esp+18h] [ebp-1Ch]
    float viewPos[3]; // [esp+1Ch] [ebp-18h] BYREF
    const pathnode_t *pSourceNode; // [esp+28h] [ebp-Ch]
    unsigned int i; // [esp+2Ch] [ebp-8h]
    gentity_s *player; // [esp+30h] [ebp-4h]

    node = 0;
    pSourceNode = 0;
    CL_GetDebugViewPos(viewPos);
    player = G_Find(0, 356, scr_const.player);
    if ( player )
    {
        pSourceNode = Sentient_NearestNode(player->sentient);
        if ( pSourceNode )
        {
            Path_DrawDebugNode(viewPos, pSourceNode);
            if ( pSourceNode )
            {
                for ( i = 0; i < g_path.actualNodeCount; ++i )
                {
                    node = &gameWorldCurrent->path.nodes[i];
                    if ( node != pSourceNode && (ai_showVisData->current.integer != 2 || Path_IsCoverNode(node)) )
                    {
                        v[0] = node->constant.vOrigin[0] - pSourceNode->constant.vOrigin[0];
                        v[1] = node->constant.vOrigin[1] - pSourceNode->constant.vOrigin[1];
                        v0 = Vec2Length(v);
                        if ( v0 <= ai_showVisDataDist->current.value )
                        {
                            if ( Path_ExpandedNodeVisible(pSourceNode, node) && Path_NodesVisible(pSourceNode, node) )
                            {
                                Path_DrawDebugNode(viewPos, node);
                                G_DebugLine(pSourceNode->constant.vOrigin, node->constant.vOrigin, colorGreen, 0);
                            }
                            else if ( Path_ExpandedNodeVisible(pSourceNode, node) )
                            {
                                Path_DrawDebugNode(viewPos, node);
                                G_DebugLine(pSourceNode->constant.vOrigin, node->constant.vOrigin, colorRed, 0);
                            }
                            else if ( Path_NodesVisible(pSourceNode, node) )
                            {
                                Path_DrawDebugNode(viewPos, node);
                                G_DebugLine(pSourceNode->constant.vOrigin, node->constant.vOrigin, colorYellow, 0);
                            }
                        }
                    }
                }
            }
        }
    }
}

void __cdecl Path_Update()
{
    Path_DrawDebug();
    Path_UpdateBadPlaces();
}

void __cdecl Path_DrawDebug()
{
    unsigned int wLinkCount; // [esp+Ch] [ebp-38h]
    char *pszText; // [esp+10h] [ebp-34h]
    unsigned int j; // [esp+18h] [ebp-2Ch]
    const pathnode_t *node; // [esp+1Ch] [ebp-28h]
    bool bShowAll; // [esp+23h] [ebp-21h]
    float viewPos[3]; // [esp+24h] [ebp-20h] BYREF
    unsigned int linkCount; // [esp+30h] [ebp-14h]
    unsigned int i; // [esp+34h] [ebp-10h]
    float vDelta[3]; // [esp+38h] [ebp-Ch]

    if ( level.gentities->client )
    {
        CL_GetDebugViewPos(viewPos);
        if ( g_radiant_selected_pathnode )
            Path_DrawDebugNode(viewPos, g_radiant_selected_pathnode);
        if ( ai_showNodes->current.integer )
        {
            for ( i = 0; i < g_path.actualNodeCount; ++i )
            {
                node = &gameWorldCurrent->path.nodes[i];
                if ( ai_showNodesDist->current.value != 0.0 )
                {
                    vDelta[0] = node->constant.vOrigin[0] - viewPos[0];
                    vDelta[1] = node->constant.vOrigin[1] - viewPos[1];
                    vDelta[2] = node->constant.vOrigin[2] - viewPos[2];
                    if ( (float)((float)(vDelta[0] * vDelta[0]) + (float)(vDelta[1] * vDelta[1])) > (float)(ai_showNodesDist->current.value
                                                                                                                                                                                                * ai_showNodesDist->current.value) )
                        continue;
                }
                if ( ai_showNodes->current.integer == 2 || ai_showNodes->current.integer == 4 )
                {
                    pszText = va("%i", i);
                    G_AddDebugString(node->constant.vOrigin, colorWhite, 1.0, pszText, 0);
                }
                bShowAll = ai_showNodes->current.integer >= 3;
                if ( ai_showNodes->current.integer < 3 )
                    wLinkCount = node->dynamic.wLinkCount;
                else
                    wLinkCount = node->constant.totalLinkCount;
                linkCount = wLinkCount;
                if ( wLinkCount )
                {
                    for ( j = 0; j < linkCount; ++j )
                        Path_DrawDebugLink(node, j, bShowAll);
                }
                else
                {
                    Path_DrawDebugNoLinks(node, (const float (*)[4])colorRed, 0);
                }
            }
        }
        if ( ai_showNearestNode->current.integer )
            Path_DrawDebugNearestNode(g_entities[0].r.currentOrigin);
        if ( ai_debugClaimedNodes->current.integer )
            Path_DrawDebugClaimedNodes(viewPos, ai_debugClaimedNodes->current.integer);
        if ( ai_debugFindPath->current.integer )
            Path_DrawDebugFindPath(level.gentities->client->ps.origin);
        if ( ai_showVisData->current.integer )
            Path_DrawVisData();
        if ( ai_showBadPlaces->current.enabled )
            Path_DrawBadPlaces();
    }
}

void __cdecl Path_DrawDebugLink(const pathnode_t *node, int i, bool bShowAll)
{
    const float *v3; // [esp+4h] [ebp-8Ch]
    int totalLinkCount; // [esp+Ch] [ebp-84h]
    int iBadDirs; // [esp+30h] [ebp-60h]
    int j; // [esp+34h] [ebp-5Ch]
    float vDir[3]; // [esp+38h] [ebp-58h] BYREF
    int k; // [esp+44h] [ebp-4Ch]
    float vMid[3]; // [esp+48h] [ebp-48h] BYREF
    float vSelfOrg[3]; // [esp+54h] [ebp-3Ch] BYREF
    int linkCount; // [esp+60h] [ebp-30h]
    float vTop[3]; // [esp+64h] [ebp-2Ch] BYREF
    float vOtherOrg[3]; // [esp+70h] [ebp-20h] BYREF
    float vBot[3]; // [esp+7Ch] [ebp-14h] BYREF
    pathnode_t *otherNode; // [esp+88h] [ebp-8h]
    float fArrowSize; // [esp+8Ch] [ebp-4h]

    fArrowSize = 8.0f;
    otherNode = &gameWorldCurrent->path.nodes[node->constant.Links[i].nodeNum];
    if ( node->constant.type != NODE_NEGOTIATION_BEGIN
        || otherNode->constant.type != NODE_NEGOTIATION_END
        || !node->dynamic.wOverlapCount && !otherNode->dynamic.wOverlapCount )
    {
        vSelfOrg[0] = node->constant.vOrigin[0];
        vSelfOrg[1] = node->constant.vOrigin[1];
        vSelfOrg[2] = node->constant.vOrigin[2];
        vOtherOrg[0] = otherNode->constant.vOrigin[0];
        vOtherOrg[1] = otherNode->constant.vOrigin[1];
        vOtherOrg[2] = otherNode->constant.vOrigin[2];
        vSelfOrg[2] = vSelfOrg[2] + 16.0;
        vOtherOrg[2] = vOtherOrg[2] + 16.0;
        if ( bShowAll )
            totalLinkCount = otherNode->constant.totalLinkCount;
        else
            totalLinkCount = otherNode->dynamic.wLinkCount;
        linkCount = totalLinkCount;
        for ( j = 0; ; ++j )
        {
            if ( j >= linkCount )
            {
                for ( k = 3; k >= 0 && !node->constant.Links[i].ubBadPlaceCount[k]; --k )
                    ;
                if ( k >= 0 )
                    G_DebugLine(vSelfOrg, vOtherOrg, colorMagenta, 1);
                else
                    G_DebugLine(vSelfOrg, vOtherOrg, colorRed, 1);
                vMid[0] = 0.5 * (float)(vSelfOrg[0] + vOtherOrg[0]);
                vMid[1] = 0.5 * (float)(vSelfOrg[1] + vOtherOrg[1]);
                vMid[2] = 0.5 * (float)(vSelfOrg[2] + vOtherOrg[2]);
                vDir[0] = vSelfOrg[0] - vOtherOrg[0];
                vDir[1] = vSelfOrg[1] - vOtherOrg[1];
                vDir[2] = vSelfOrg[2] - vOtherOrg[2];
                Vec3Normalize(vDir);
                vTop[0] = (float)(fArrowSize * vDir[0]) + vMid[0];
                vTop[1] = (float)(fArrowSize * vDir[1]) + vMid[1];
                vBot[0] = vTop[0];
                vBot[1] = vTop[1];
                vBot[2] = (float)(fArrowSize * vDir[2]) + vMid[2];
                vTop[2] = vBot[2] + fArrowSize;
                vBot[2] = vBot[2] - fArrowSize;
                G_DebugLine(vTop, vMid, colorRed, 0);
                G_DebugLine(vBot, vMid, colorRed, 0);
                return;
            }
            if ( node == &gameWorldCurrent->path.nodes[otherNode->constant.Links[j].nodeNum] )
                break;
        }
        if ( node > otherNode )
        {
            iBadDirs = 0;
            for ( k = 3; k >= 0; --k )
            {
                if ( node->constant.Links[i].ubBadPlaceCount[k] )
                {
                    iBadDirs = 1;
                    break;
                }
            }
            for ( k = 3; k >= 0; --k )
            {
                if ( otherNode->constant.Links[j].ubBadPlaceCount[k] )
                {
                    iBadDirs |= 2u;
                    break;
                }
            }
            if ( iBadDirs )
            {
                if ( iBadDirs == 3 )
                    v3 = colorYellow;
                else
                    v3 = colorBlue;
                G_DebugLine(vSelfOrg, vOtherOrg, v3, 1);
            }
            else
            {
                G_DebugLine(vSelfOrg, vOtherOrg, colorCyan, 1);
            }
            if ( iBadDirs == 1 || iBadDirs == 2 )
            {
                vMid[0] = 0.5 * (float)(vSelfOrg[0] + vOtherOrg[0]);
                vMid[1] = 0.5 * (float)(vSelfOrg[1] + vOtherOrg[1]);
                vMid[2] = 0.5 * (float)(vSelfOrg[2] + vOtherOrg[2]);
                vDir[0] = vSelfOrg[0] - vOtherOrg[0];
                vDir[1] = vSelfOrg[1] - vOtherOrg[1];
                vDir[2] = vSelfOrg[2] - vOtherOrg[2];
                Vec3Normalize(vDir);
                if ( iBadDirs == 1 )
                {
                    vDir[0] = -vDir[0];
                    vDir[1] = -vDir[1];
                    vDir[2] = -vDir[2];
                }
                vTop[0] = (float)(fArrowSize * vDir[0]) + vMid[0];
                vTop[1] = (float)(fArrowSize * vDir[1]) + vMid[1];
                vBot[0] = vTop[0];
                vBot[1] = vTop[1];
                vBot[2] = (float)(fArrowSize * vDir[2]) + vMid[2];
                vTop[2] = vBot[2] + fArrowSize;
                vBot[2] = vBot[2] - fArrowSize;
                G_DebugLine(vTop, vMid, colorBlue, 1);
                G_DebugLine(vBot, vMid, colorBlue, 1);
            }
        }
    }
}

void __cdecl Path_DrawDebugClaimedNodes(const float *origin, int numNodes)
{
    sentient_s *v2; // eax
    char *v3; // [esp+10h] [ebp-C38h]
    char *v4; // [esp+14h] [ebp-C34h]
    char *v5; // [esp+18h] [ebp-C30h]
    char *pszText; // [esp+1Ch] [ebp-C2Ch]
    pathnode_t *node; // [esp+24h] [ebp-C24h]
    float pos[3]; // [esp+28h] [ebp-C20h] BYREF
    int nodeIndex; // [esp+34h] [ebp-C14h]
    pathsort_t nodes[256]; // [esp+38h] [ebp-C10h] BYREF
    float time; // [esp+C3Ch] [ebp-Ch]
    int nodeCount; // [esp+C40h] [ebp-8h] BYREF
    float scale; // [esp+C44h] [ebp-4h]

    if ( !origin && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 3330, 0, "%s", "origin") )
        __debugbreak();
    Path_NearestNode(
        origin,
        nodes,
        -1,
        ai_showNodesDist->current.value,
        &nodeCount,
        256,
        NEAREST_NODE_DONT_DO_HEIGHT_CHECK);
    for ( nodeIndex = 0; nodeIndex < nodeCount && nodeIndex < numNodes; ++nodeIndex )
    {
        node = nodes[nodeIndex].node;
        Path_DrawDebugNodeBox(node);
        scale = Path_GetDebugStringScale(origin, node->constant.vOrigin);
        scale = scale * 0.5;
        pos[0] = node->constant.vOrigin[0];
        pos[1] = node->constant.vOrigin[1];
        pos[2] = node->constant.vOrigin[2];
        //if ( SentientHandle::isDefined(&node->dynamic.pOwner) )
        if ( node->dynamic.pOwner.isDefined() )
        {
            iassert(node->dynamic.pOwner.sentient()->ent);
            //v2 = SentientHandle::sentient(&node->dynamic.pOwner);
            v2 = node->dynamic.pOwner.sentient();;
            pszText = va("Owner: %d", v2->ent->s.number);
            G_AddDebugString(pos, colorGreen, scale, pszText, 0);
        }
        else
        {
            G_AddDebugString(pos, colorWhite, scale, (char*)"Owner: None", 0);
        }
        pos[2] = (float)(12.0 * scale) + pos[2];
        if ( level.time < node->dynamic.iValidTime[1] )
        {
            time = (float)(node->dynamic.iValidTime[1] - level.time) * 0.001;
            v5 = va("Invalid Ally: %2.1f", time);
            G_AddDebugString(pos, colorYellow, scale, v5, 0);
            pos[2] = (float)(12.0 * scale) + pos[2];
        }
        if ( level.time <= node->dynamic.iValidTime[0] )
        {
            time = (float)(node->dynamic.iValidTime[0] - level.time) * 0.001;
            v4 = va("Invalid Axis: %2.1f", time);
            G_AddDebugString(pos, colorYellow, scale, v4, 0);
            pos[2] = (float)(12.0 * scale) + pos[2];
        }
        if ( node->dynamic.iFreeTime != 0x7FFFFFFF && level.time < node->dynamic.iFreeTime )
        {
            time = (float)(node->dynamic.iFreeTime - level.time) * 0.001;
            v3 = va("Delay: %2.1f", time);
            G_AddDebugString(pos, colorYellow, scale, v3, 0);
            pos[2] = (float)(12.0 * scale) + pos[2];
        }
    }
}

bool __cdecl Path_NodesVisible(const pathnode_t *node0, const pathnode_t *node1)
{
    int entry; // [esp+4h] [ebp-Ch]
    unsigned int nodeIndex0; // [esp+8h] [ebp-8h]
    unsigned int nodeIndex1; // [esp+Ch] [ebp-4h]

    if ( !node0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 3752, 0, "%s", "node0") )
        __debugbreak();
    if ( !node1 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 3753, 0, "%s", "node1") )
        __debugbreak();
    nodeIndex0 = Path_ConvertNodeToIndex(node0);
    nodeIndex1 = Path_ConvertNodeToIndex(node1);
    if ( nodeIndex0 >= g_path.actualNodeCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    3758,
                    0,
                    "nodeIndex0 doesn't index g_path.actualNodeCount\n\t%i not in [0, %i)",
                    nodeIndex0,
                    g_path.actualNodeCount) )
    {
        __debugbreak();
    }
    if ( nodeIndex1 >= g_path.actualNodeCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    3759,
                    0,
                    "nodeIndex1 doesn't index g_path.actualNodeCount\n\t%i not in [0, %i)",
                    nodeIndex1,
                    g_path.actualNodeCount) )
    {
        __debugbreak();
    }
    if ( nodeIndex0 >= nodeIndex1 )
    {
        if ( nodeIndex0 <= nodeIndex1 )
            return 1;
        entry = NodeVisCacheEntry(nodeIndex1, nodeIndex0);
    }
    else
    {
        entry = NodeVisCacheEntry(nodeIndex0, nodeIndex1);
    }
    return gameWorldCurrent->path.pathVis && ((1 << (entry & 7)) & gameWorldCurrent->path.pathVis[entry >> 3]) != 0;
}

int __cdecl NodeVisCacheEntry(int i, int j)
{
    if ( i >= j && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 2033, 0, "%s", "i < j") )
        __debugbreak();
    return j + g_path.actualNodeCount * i;
}

int ExpandedNodeVisCacheEntry(int i, int j)
{
    if (i <= j && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 2040, 0, "%s", "i > j"))
        __debugbreak();
    return j + g_path.actualNodeCount * (i - 1);
}

bool __cdecl Path_ExpandedNodeVisible(const pathnode_t *node0, const pathnode_t *node1)
{
    int entry; // [esp+4h] [ebp-Ch]
    unsigned int nodeIndex0; // [esp+8h] [ebp-8h]
    unsigned int nodeIndex1; // [esp+Ch] [ebp-4h]

    if ( !node0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 3780, 0, "%s", "node0") )
        __debugbreak();
    if ( !node1 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 3781, 0, "%s", "node1") )
        __debugbreak();
    nodeIndex0 = Path_ConvertNodeToIndex(node0);
    nodeIndex1 = Path_ConvertNodeToIndex(node1);
    if ( nodeIndex0 >= g_path.actualNodeCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    3786,
                    0,
                    "nodeIndex0 doesn't index g_path.actualNodeCount\n\t%i not in [0, %i)",
                    nodeIndex0,
                    g_path.actualNodeCount) )
    {
        __debugbreak();
    }
    if ( nodeIndex1 >= g_path.actualNodeCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    3787,
                    0,
                    "nodeIndex1 doesn't index g_path.actualNodeCount\n\t%i not in [0, %i)",
                    nodeIndex1,
                    g_path.actualNodeCount) )
    {
        __debugbreak();
    }
    if ( nodeIndex0 <= nodeIndex1 )
    {
        if ( nodeIndex0 >= nodeIndex1 )
            return 1;
        entry = ExpandedNodeVisCacheEntry(nodeIndex1, nodeIndex0);
    }
    else
    {
        entry = ExpandedNodeVisCacheEntry(nodeIndex0, nodeIndex1);
    }
    return gameWorldCurrent->path.pathVis && ((1 << (entry & 7)) & gameWorldCurrent->path.pathVis[entry >> 3]) != 0;
}

pathnode_t *__cdecl Path_NearestNode(
                const float *vOrigin,
                pathsort_t *nodes,
                int typeFlags,
                float fMaxDist,
                int *returnCount,
                int maxNodes,
                nearestNodeHeightCheck heightCheck)
{
    return Path_NearestNodeNotCrossPlanes(
                     vOrigin,
                     nodes,
                     typeFlags,
                     fMaxDist,
                     0,
                     0,
                     0,
                     returnCount,
                     maxNodes,
                     heightCheck);
}

pathnode_t *__cdecl Path_NearestNodeNotCrossPlanes(
                const float *vOrigin,
                pathsort_t *nodes,
                int typeFlags,
                float fMaxDist,
                float (*vNormal)[2],
                float *fDist,
                int iPlaneCount,
                int *returnCount,
                int maxNodes,
                nearestNodeHeightCheck heightCheck)
{
    pathnode_t *closestNode; // [esp+74h] [ebp-470h]
    float distSq; // [esp+78h] [ebp-46Ch]
    float adjustedOrigin[3]; // [esp+7Ch] [ebp-468h] BYREF
    int j; // [esp+88h] [ebp-45Ch]
    pathnode_t *node; // [esp+8Ch] [ebp-458h]
    int iNodeCount; // [esp+B8h] [ebp-42Ch]
    float mins[3]; // [esp+BCh] [ebp-428h] BYREF
    float maxs[3]; // [esp+C8h] [ebp-41Ch] BYREF
    pathnode_t *failedNodes[256]; // [esp+D4h] [ebp-410h]
    int i; // [esp+4D8h] [ebp-Ch]
    int hitNum; // [esp+4DCh] [ebp-8h] BYREF
    int iFailedNodeCount; // [esp+4E0h] [ebp-4h]

    //col_context_t::col_context_t(&context, (int)&loc_82000C + 5);
    col_context_t context(0x820011); // [esp+90h] [ebp-454h] BYREF
    if ( heightCheck )
    {
        iNodeCount = Path_NodesInRadius(vOrigin, fMaxDist, nodes, maxNodes, typeFlags);
    }
    else
    {
        adjustedOrigin[0] = *vOrigin;
        adjustedOrigin[1] = vOrigin[1];
        adjustedOrigin[2] = vOrigin[2];
        adjustedOrigin[2] = adjustedOrigin[2] - 120.0;
        iNodeCount = Path_NodesInCylinder(adjustedOrigin, fMaxDist, 184.0, nodes, maxNodes, typeFlags);
    }

    //std::_Sort<GfxCachedShaderText *,int,bool (__cdecl *)(GfxCachedShaderText const &,GfxCachedShaderText const &)>(
    //    (GfxCachedShaderText *)nodes,
    //    (GfxCachedShaderText *)&nodes[iNodeCount],
    //    12 * iNodeCount / 12,
    //    (bool (__cdecl *)(const GfxCachedShaderText *, const GfxCachedShaderText *))Path_CompareNodesIncreasing);

    std::sort(&nodes[0], &nodes[iNodeCount], Path_CompareNodesIncreasing);
    
    mins[0] = actorMins[0];
    mins[1] = -15.0;
    maxs[0] = actorMaxs[0];
    maxs[1] = 15.0;
    maxs[2] = 48.0;
    mins[2] = 0.0 + 17.0;

    static const float zombie_fudge = 14.0f;
    static const float fudge_0 = 5.0f;

    if ( zombiemode->current.enabled )
    {
        mins[0] = mins[0] + zombie_fudge;
        mins[1] = mins[1] + zombie_fudge;
        maxs[0] = maxs[0] - zombie_fudge;
        maxs[1] = maxs[1] - zombie_fudge;
    }
    else
    {
        mins[0] = mins[0] + fudge_0;
        mins[1] = mins[1] + fudge_0;
        maxs[0] = maxs[0] - fudge_0;
        maxs[1] = maxs[1] - fudge_0;
    }
    iFailedNodeCount = 0;
    *returnCount = iNodeCount;
    for ( i = 0; i < iNodeCount; ++i )
    {
        node = nodes[i].node;
        if ( node->dynamic.wLinkCount )
        {
            if ( *vOrigin > (float)((float)(node->constant.vOrigin[0] + -15.0) - 1.0)
                && (float)((float)(node->constant.vOrigin[0] + 15.0) + 1.0) > *vOrigin
                && vOrigin[1] > (float)((float)(node->constant.vOrigin[1] + -15.0) - 1.0)
                && (float)((float)(node->constant.vOrigin[1] + 15.0) + 1.0) > vOrigin[1]
                && vOrigin[2] > (float)((float)(node->constant.vOrigin[2] + 0.0) - 1.0)
                && (float)((float)(node->constant.vOrigin[2] + 48.0) + 1.0) > vOrigin[2] )
            {
                return node;
            }
            for ( j = 0; j < iPlaneCount; ++j )
            {
                if ( (float)((float)(node->constant.vOrigin[0] * (*vNormal)[2 * j])
                                     + (float)(node->constant.vOrigin[1] * (*vNormal)[2 * j + 1])) > fDist[j] )
                    goto failed_node;
            }
            hitNum = 0;
            SV_SightTraceCapsule(&hitNum, vOrigin, mins, maxs, node->constant.vOrigin, &context);
            if ( !hitNum )
                return node;
        }
        else
        {
failed_node:
            failedNodes[iFailedNodeCount++] = node;
        }
    }
    for ( i = 0; i < iFailedNodeCount; ++i )
    {
        hitNum = 0;
        SV_SightTraceCapsule(&hitNum, vOrigin, mins, maxs, failedNodes[i]->constant.vOrigin, &context);
        if ( !hitNum )
            return failedNodes[i];
    }
    if ( !zombiemode->current.enabled )
        return 0;
    closestNode = 0;
    for ( i = 0; i < iNodeCount; ++i )
    {
        node = nodes[i].node;
        distSq = Vec3DistanceSq(node->constant.vOrigin, vOrigin);
        if ( distSq < 3.4028235e38 )
            closestNode = node;
    }
    return closestNode;
}

bool __cdecl Path_CanClaimNode(const pathnode_t *node, sentient_s *claimer)
{
    sentient_s *NodeOwner; // eax

    if ( !node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 4669, 0, "%s", "node") )
        __debugbreak();
    if ( !claimer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 4670, 0, "%s", "claimer") )
        __debugbreak();
    if ( claimer->eTeam != TEAM_AXIS
        && claimer->eTeam != TEAM_ALLIES
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    4671,
                    0,
                    "%s\n\t(claimer->eTeam) = %i",
                    "((claimer->eTeam == TEAM_AXIS || claimer->eTeam == TEAM_ALLIES))",
                    claimer->eTeam) )
    {
        __debugbreak();
    }
    //if ( SentientHandle::isDefined(&node->dynamic.pOwner) && SentientHandle::sentient(&node->dynamic.pOwner) == claimer )
    if ( node->dynamic.pOwner.isDefined() && node->dynamic.pOwner.sentient() == claimer)
        return 1;
    if ( level.time <= *(&node->dynamic.iFreeTime + claimer->eTeam) )
        return 0;
    if ( level.time > node->dynamic.iFreeTime )
        return 1;
    if ( node->dynamic.iFreeTime != 0x7FFFFFFF || !Path_CanStealNode((pathnode_t*)node, claimer) )
        return (node->constant.spawnflags & 0x40) != 0 && Path_CanStealPriorityNode((pathnode_t *)node, claimer);
    NodeOwner = Path_GetNodeOwner((pathnode_t *)node);
    Path_RelinquishNodeNow(NodeOwner);
    return 1;
}

sentient_s *__cdecl Path_GetNodeOwner(pathnode_t *node)
{
    pathnode_t *overlappedNode; // [esp+0h] [ebp-4h]

    if ( !node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 4493, 0, "%s", "node") )
        __debugbreak();
    if ( node->dynamic.pOwner.isDefined() )
        return node->dynamic.pOwner.sentient();
    if ( node->constant.wOverlapNode[0] >= 0 )
    {
        overlappedNode = &gameWorldCurrent->path.nodes[node->constant.wOverlapNode[0]];
        if ( overlappedNode->dynamic.pOwner.isDefined() )
            return overlappedNode->dynamic.pOwner.sentient();
        if ( node->constant.wOverlapNode[1] >= 0 )
        {
            overlappedNode = &gameWorldCurrent->path.nodes[node->constant.wOverlapNode[1]];
            if ( overlappedNode->dynamic.pOwner.isDefined() )
                return overlappedNode->dynamic.pOwner.sentient();
        }
    }
    return 0;
}

bool __cdecl Path_CanStealPriorityNode(pathnode_t *node, sentient_s *claimer)
{
    float v3; // [esp+Ch] [ebp-20h]
    float v4; // [esp+1Ch] [ebp-10h]
    float ownerToNodeDistSq; // [esp+20h] [ebp-Ch]
    sentient_s *owner; // [esp+24h] [ebp-8h]

    if ( !node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 4524, 0, "%s", "node") )
        __debugbreak();
    if ( (node->constant.spawnflags & 0x40) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    4525,
                    0,
                    "%s",
                    "node->constant.spawnflags & PNF_PRIORITY") )
    {
        __debugbreak();
    }
    if ( !claimer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 4526, 0, "%s", "claimer") )
        __debugbreak();
    owner = Path_GetNodeOwner(node);
    if ( !owner )
        return 1;
    if ( node->dynamic.wOverlapCount <= 1 )
    {
        if ( owner->ent->actor )
        {
            if ( owner == claimer )
            {
                return 1;
            }
            else if ( owner->eTeam == claimer->eTeam )
            {
                if ( Actor_IsDying(owner->ent->actor) )
                {
                    return 1;
                }
                else
                {
                    v4 = node->constant.vOrigin[1] - owner->ent->r.currentOrigin[1];
                    ownerToNodeDistSq = (float)((float)(node->constant.vOrigin[0] - owner->ent->r.currentOrigin[0])
                                                                        * (float)(node->constant.vOrigin[0] - owner->ent->r.currentOrigin[0]))
                                                        + (float)(v4 * v4);
                    if ( ownerToNodeDistSq >= 225.0 )
                    {
                        v3 = node->constant.vOrigin[1] - claimer->ent->r.currentOrigin[1];
                        return ownerToNodeDistSq > (float)((float)((float)((float)(node->constant.vOrigin[0]
                                                                                                                                         - claimer->ent->r.currentOrigin[0])
                                                                                                                         * (float)(node->constant.vOrigin[0]
                                                                                                                                         - claimer->ent->r.currentOrigin[0]))
                                                                                                         + (float)(v3 * v3))
                                                                                         + 225.0);
                    }
                    else
                    {
                        return 0;
                    }
                }
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        iassert(!node->dynamic.pOwner.isDefined());
        return 0;
    }
}

bool __cdecl Path_CanStealNode(pathnode_t *node, sentient_s *claimer)
{
    sentient_s *owner; // [esp+0h] [ebp-4h]

    if ( !node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 4601, 0, "%s", "node") )
        __debugbreak();
    if ( !claimer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 4602, 0, "%s", "claimer") )
        __debugbreak();
    owner = Path_GetNodeOwner(node);
    if ( !owner )
        return 1;
    if ( node->dynamic.wOverlapCount <= 1 )
    {
        if ( owner->ent->actor )
        {
            if ( owner == claimer )
            {
                return 1;
            }
            else if ( claimer->ent->actor )
            {
                return !claimer->pClaimedNode
                        && !Actor_IsMoving(claimer->ent->actor)
                        && Actor_PointNearNode(claimer->ent->r.currentOrigin, node)
                        && !Actor_PointNearNode(owner->ent->r.currentOrigin, node);
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        iassert(!node->dynamic.pOwner.isDefined());
        return 0;
    }
}

void __cdecl Path_ClaimNode(pathnode_t *node, sentient_s *claimer)
{
    const char *v2; // eax
    int v3; // [esp+0h] [ebp-Ch]
    pathnode_t *otherNode; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    if ( !node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 4751, 0, "%s", "node") )
        __debugbreak();
    if ( !claimer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 4752, 0, "%s", "claimer") )
        __debugbreak();
    if ( claimer->eTeam != TEAM_AXIS
        && claimer->eTeam != TEAM_ALLIES
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    4753,
                    0,
                    "%s\n\t(claimer->eTeam) = %i",
                    "((claimer->eTeam == TEAM_AXIS || claimer->eTeam == TEAM_ALLIES))",
                    claimer->eTeam) )
    {
        __debugbreak();
    }
    iassert(((node->dynamic.pOwner.isDefined() && (node->dynamic.pOwner.sentient() == claimer)) || node->dynamic.iFreeTime < level.time));
    iassert(!node->dynamic.pOwner.isDefined() || node->dynamic.pOwner.sentient() == claimer || level.time > node->dynamic.iValidTime[claimer->eTeam - TEAM_AXIS]);
    
    //SentientHandle::setSentient(&node->dynamic.pOwner, claimer);
    node->dynamic.pOwner.setSentient(claimer);
    node->dynamic.iFreeTime = 0x7FFFFFFF;
    for ( i = 0; i < 2 && node->constant.wOverlapNode[i] >= 0; ++i )
    {
        otherNode = &gameWorldCurrent->path.nodes[node->constant.wOverlapNode[i]];
        //if ( (!SentientHandle::isDefined(&otherNode->dynamic.pOwner)
        if ( (!otherNode->dynamic.pOwner.isDefined()
             //|| SentientHandle::sentient(&otherNode->dynamic.pOwner) != claimer)
             || otherNode->dynamic.pOwner.sentient() != claimer)
            && otherNode->dynamic.iFreeTime == 0x7FFFFFFF
            && !otherNode->dynamic.wOverlapCount )
        {
            if ( otherNode->dynamic.pOwner.isDefined() )
            {
                //v3 = SentientHandle::sentient(&otherNode->dynamic.pOwner) - level.sentients;
                v3 = otherNode->dynamic.pOwner.sentient() - level.sentients;
                v2 = va(
                             "node = %i, owner = %i, free time = %i",
                             otherNode - gameWorldCurrent->path.nodes,
                             v3,
                             otherNode->dynamic.iFreeTime);
            }
            else
            {
                v2 = va(
                             "node = %i, owner = %i, free time = %i",
                             otherNode - gameWorldCurrent->path.nodes,
                             -1,
                             otherNode->dynamic.iFreeTime);
            }
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                            4768,
                            0,
                            "%s\n\t%s",
                            "((otherNode->dynamic.pOwner.isDefined() && (otherNode->dynamic.pOwner.sentient() == claimer)) || (otherNod"
                            "e->dynamic.iFreeTime != INT_MAX) || (otherNode->dynamic.wOverlapCount))",
                            v2) )
                __debugbreak();
        }
        Path_MarkNodeOverlap(otherNode);
        if ( (unsigned int)otherNode->dynamic.wOverlapCount >= 3
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                        4772,
                        0,
                        "otherNode->dynamic.wOverlapCount doesn't index ARRAY_COUNT( otherNode->constant.wOverlapNode ) + 1\n"
                        "\t%i not in [0, %i)",
                        otherNode->dynamic.wOverlapCount,
                        3) )
        {
            __debugbreak();
        }
        if ( *((__int16 *)&otherNode->constant.minUseDistSq + otherNode->dynamic.wOverlapCount + 1) < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                        4773,
                        0,
                        "%s",
                        "otherNode->constant.wOverlapNode[otherNode->dynamic.wOverlapCount - 1] >= 0") )
        {
            __debugbreak();
        }
    }
    Path_ClaimNodeInternal(node, claimer);
}

void __cdecl Path_ClaimNodeInternal(pathnode_t *node, sentient_s *claimer)
{
    if ( claimer->ent->actor
        && Actor_KeepClaimedNode(claimer->ent->actor)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    4725,
                    0,
                    "%s",
                    "!claimer->ent->actor || !Actor_KeepClaimedNode(claimer->ent->actor)") )
    {
        __debugbreak();
    }
    claimer->pClaimedNode = node;
    claimer->banNodeTime = 0;
    Sentient_BanNearNodes(claimer);
}

void __cdecl Path_MarkNodeOverlap(pathnode_t *node)
{
    if ( !node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 4734, 0, "%s", "node") )
        __debugbreak();
    //SentientHandle::setSentient(&node->dynamic.pOwner, 0);
    node->dynamic.pOwner.setSentient(0);
    node->dynamic.iFreeTime = 0x7FFFFFFF;
    ++node->dynamic.wOverlapCount;
}

void __cdecl Path_RevokeClaim(pathnode_t *node, sentient_s *pNewClaimer)
{
    sentient_s *v2; // eax

    if ( !node->dynamic.pOwner.isDefined()
        || !node->dynamic.pOwner.sentient()->ent
        || node->dynamic.pOwner.sentient()->ent->actor)
    {
        if ( node->dynamic.pOwner.isDefined() )
        {
            if ( node->dynamic.pOwner.sentient() != pNewClaimer && node->dynamic.iFreeTime == 0x7FFFFFFF )
            {
                v2 = node->dynamic.pOwner.sentient();
                Sentient_NodeClaimRevoked(v2, node);
                if ( node->dynamic.iFreeTime == 0x7FFFFFFF
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                                4792,
                                0,
                                "%s",
                                "node->dynamic.iFreeTime != INT_MAX") )
                {
                    __debugbreak();
                }
            }
        }
    }
}

void __cdecl Path_ForceClaimNode(pathnode_t *node, sentient_s *claimer)
{
    if ( !node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 4806, 0, "%s", "node") )
        __debugbreak();
    if ( !claimer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 4807, 0, "%s", "claimer") )
        __debugbreak();
    if ( claimer->eTeam != TEAM_AXIS
        && claimer->eTeam != TEAM_ALLIES
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    4808,
                    0,
                    "%s\n\t(claimer->eTeam) = %i",
                    "((claimer->eTeam == TEAM_AXIS || claimer->eTeam == TEAM_ALLIES))",
                    claimer->eTeam) )
    {
        __debugbreak();
    }
    if ( claimer->pClaimedNode && claimer->pClaimedNode != node )
        Path_RelinquishNodeSoon(claimer);
    if ( node->dynamic.pOwner.isDefined() && node->dynamic.pOwner.sentient() == claimer)
    {
        if ( claimer->pClaimedNode != node )
            Path_ClaimNode(node, claimer);
    }
    else
    {
        Path_RevokeClaim(node, claimer);
        if ( node->constant.wOverlapNode[0] >= 0 )
        {
            Path_RevokeClaim(&gameWorldCurrent->path.nodes[node->constant.wOverlapNode[0]], claimer);
            if ( node->constant.wOverlapNode[1] >= 0 )
                Path_RevokeClaim(&gameWorldCurrent->path.nodes[node->constant.wOverlapNode[1]], claimer);
        }
        if ( node->dynamic.iFreeTime != 0x7FFFFFFF )
        {
            node->dynamic.pOwner.setSentient(claimer);
            node->dynamic.iFreeTime = 0x7FFFFFFF;
            if ( node->constant.wOverlapNode[0] >= 0 )
            {
                Path_MarkNodeOverlap(&gameWorldCurrent->path.nodes[node->constant.wOverlapNode[0]]);
                if ( node->constant.wOverlapNode[1] >= 0 )
                    Path_MarkNodeOverlap(&gameWorldCurrent->path.nodes[node->constant.wOverlapNode[1]]);
            }
            Path_ClaimNodeInternal(node, claimer);
        }
    }
}

void __cdecl Path_RelinquishNodeNow(sentient_s *claimer)
{
    Path_RelinquishNode(claimer, 0);
}

void __cdecl Path_RelinquishNodeSoon(sentient_s *claimer)
{
    Path_RelinquishNode(claimer, 5000);
}

void __cdecl Path_RelinquishNode(sentient_s *claimer, int timeUntilRelinquished)
{
    pathnode_t *otherNode; // [esp+0h] [ebp-18h]
    int i; // [esp+4h] [ebp-14h]
    pathnode_t *node; // [esp+8h] [ebp-10h]
    actor_s *claimActor; // [esp+Ch] [ebp-Ch]
    sentient_s *newOwner; // [esp+10h] [ebp-8h]
    int newFreeTime; // [esp+14h] [ebp-4h]

    node = claimer->pClaimedNode;
    if ( !node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 4896, 0, "%s", "node") )
        __debugbreak();

    iassert(node->dynamic.pOwner.isDefined());
    iassert(node->dynamic.pOwner.sentient() == claimer);

    if ( !claimer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 4899, 0, "%s", "claimer") )
        __debugbreak();
    if ( claimer->eTeam != TEAM_AXIS
        && claimer->eTeam != TEAM_ALLIES
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    4900,
                    0,
                    "%s\n\t(claimer->eTeam) = %i",
                    "((claimer->eTeam == TEAM_AXIS || claimer->eTeam == TEAM_ALLIES))",
                    claimer->eTeam) )
    {
        __debugbreak();
    }
    claimer->pClaimedNode = 0;
    Scr_Notify(claimer->ent, scr_const.node_relinquished, 0);
    claimer->bNearestNodeValid = 0;
    claimActor = claimer->ent->actor;
    if ( claimActor )
        claimActor->arrivalInfo.animscriptOverrideRunTo = 0;
    if ( timeUntilRelinquished > 0 )
    {
        newFreeTime = timeUntilRelinquished + level.time;
        newOwner = claimer;
    }
    else
    {
        newFreeTime = 0;
        newOwner = 0;
    }
    node->dynamic.iFreeTime = newFreeTime;
    node->dynamic.pOwner.setSentient(newOwner);
    for ( i = 0; i < 2 && node->constant.wOverlapNode[i] >= 0; ++i )
    {
        otherNode = &gameWorldCurrent->path.nodes[node->constant.wOverlapNode[i]];
        iassert(!otherNode->dynamic.pOwner.isDefined());

        if ( !--otherNode->dynamic.wOverlapCount )
        {
            //SentientHandle::setSentient(&otherNode->dynamic.pOwner, newOwner);
            otherNode->dynamic.pOwner.setSentient(newOwner);
            otherNode->dynamic.iFreeTime = newFreeTime;
        }
    }
}

void __cdecl Path_MarkNodeInvalid(pathnode_t *node, team_t eTeam)
{
    sentient_s *v2; // eax
    pathnode_t *otherNode; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if ( !node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 4953, 0, "%s", "node") )
        __debugbreak();
    if ( eTeam != TEAM_AXIS
        && eTeam != TEAM_ALLIES
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    4954,
                    0,
                    "%s\n\t(eTeam) = %i",
                    "((eTeam == TEAM_AXIS || eTeam == TEAM_ALLIES))",
                    eTeam) )
    {
        __debugbreak();
    }
    //if ( SentientHandle::isDefined(&node->dynamic.pOwner) && SentientHandle::sentient(&node->dynamic.pOwner)->eTeam == eTeam )
    if ( node->dynamic.pOwner.isDefined() && node->dynamic.pOwner.sentient()->eTeam == eTeam)
    {
        //if ( SentientHandle::sentient(&node->dynamic.pOwner)->pClaimedNode )
        if ( node->dynamic.pOwner.sentient()->pClaimedNode)
        {
            //v2 = SentientHandle::sentient(&node->dynamic.pOwner);
            v2 = node->dynamic.pOwner.sentient();
            Path_RelinquishNodeSoon(v2);
        }
        node->dynamic.pOwner.setSentient(0);
    }
    *(&node->dynamic.iFreeTime + eTeam) = level.time + 5000;
    if ( !node->dynamic.wOverlapCount )
    {
        node->dynamic.iFreeTime = 0;
        for ( i = 0; i < 2 && node->constant.wOverlapNode[i] >= 0; ++i )
        {
            otherNode = &gameWorldCurrent->path.nodes[node->constant.wOverlapNode[i]];
            if ( !otherNode->dynamic.wOverlapCount && otherNode->dynamic.iFreeTime != 0x7FFFFFFF )
            {
                if ( otherNode->dynamic.pOwner.isDefined() )
                {
                    if ( otherNode->dynamic.pOwner.sentient()->eTeam == eTeam)
                        otherNode->dynamic.pOwner.setSentient(0);
                }
                otherNode->dynamic.iFreeTime = 0;
            }
        }
    }

    iassert(!node->dynamic.pOwner.isDefined() || node->dynamic.pOwner.sentient()->eTeam != eTeam);
    //if ( SentientHandle::isDefined(&node->dynamic.pOwner) && SentientHandle::sentient(&node->dynamic.pOwner)->eTeam == eTeam
    //    && !Assert_MyHandler(
    //                "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
    //                4984,
    //                0,
    //                "%s",
    //                "!node->dynamic.pOwner.isDefined() || node->dynamic.pOwner.sentient()->eTeam != eTeam") )
    //{
    //    __debugbreak();
    //}
}

ai_stance_e __cdecl Path_AllowedStancesForNode(pathnode_t *node)
{
    int eAllowedStances; // [esp+0h] [ebp-4h]

    if ( !node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 4992, 0, "%s", "node") )
        __debugbreak();
    eAllowedStances = STANCE_ANY;
    if ( (node->constant.spawnflags & 4) != 0 )
        eAllowedStances = STANCE_PRONE|STANCE_CROUCH;
    if ( (node->constant.spawnflags & 8) != 0 )
        eAllowedStances &= ~2u;
    if ( (node->constant.spawnflags & 0x10) != 0 )
        eAllowedStances &= ~4u;
    if ( eAllowedStances == STANCE_BAD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    5003,
                    0,
                    "%s\n\t(node->constant.spawnflags) = %i",
                    "(eAllowedStances)",
                    node->constant.spawnflags) )
    {
        __debugbreak();
    }
    return (ai_stance_e)eAllowedStances;
}

void __cdecl Path_ValidateNode(pathnode_t *node)
{
    const char *v1; // eax
    const char *v2; // eax
    int j; // [esp+0h] [ebp-8h]
    pathlink_s *link; // [esp+4h] [ebp-4h]
    pathlink_s *linka; // [esp+4h] [ebp-4h]

    for ( j = node->constant.totalLinkCount - 1; j >= node->dynamic.wLinkCount; --j )
    {
        link = &node->constant.Links[j];
        if ( !link->disconnectCount )
        {
            v1 = va("%d, %d, %d", node - gameWorldCurrent->path.nodes, j, link->nodeNum);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                            5041,
                            0,
                            "%s\n\t%s",
                            "link->disconnectCount > 0",
                            v1) )
                __debugbreak();
        }
    }
    if ( j != node->dynamic.wLinkCount - 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    5044,
                    0,
                    "%s",
                    "j == node->dynamic.wLinkCount - 1") )
    {
        __debugbreak();
    }
    while ( j >= 0 )
    {
        linka = &node->constant.Links[j];
        if ( linka->disconnectCount )
        {
            v2 = va("%d, %d, %d", node - gameWorldCurrent->path.nodes, j, linka->nodeNum);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                            5048,
                            0,
                            "%s\n\t%s",
                            "!link->disconnectCount",
                            v2) )
                __debugbreak();
        }
        --j;
    }
}

void __cdecl Path_ValidateAllNodes()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < g_path.actualNodeCount; ++i )
        Path_ValidateNode(&gameWorldCurrent->path.nodes[i]);
}

void __cdecl Path_CheckLinkLeaks()
{
    unsigned int infoIndex; // [esp+0h] [ebp-8h]
    unsigned int infoIndexa; // [esp+0h] [ebp-8h]
    int count; // [esp+4h] [ebp-4h]
    int counta; // [esp+4h] [ebp-4h]

    if ( g_path.pathLinkInfoArrayInited )
    {
        count = 0;
        for ( infoIndex = g_path.pathLinkInfoArray[0].next; infoIndex; infoIndex = g_path.pathLinkInfoArray[infoIndex].next )
            ++count;
        if ( count != 2047
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                        5073,
                        0,
                        "%s",
                        "count == PATHLINK_BUFFERSIZE - 1") )
        {
            __debugbreak();
        }
        counta = 0;
        for ( infoIndexa = g_path.pathLinkInfoArray[0].prev; infoIndexa; infoIndexa = g_path.pathLinkInfoArray[infoIndexa].prev )
            ++counta;
        if ( counta != 2047
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                        5078,
                        0,
                        "%s",
                        "count == PATHLINK_BUFFERSIZE - 1") )
        {
            __debugbreak();
        }
    }
}

void __cdecl Path_CheckUserCountLeaks()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < g_path.actualNodeCount; ++i )
    {
        if ( gameWorldCurrent->path.nodes[i].dynamic.userCount )
        {
            Com_Printf(
                15,
                "WARNING: Path node %d has a userCount of %d\n",
                i,
                gameWorldCurrent->path.nodes[i].dynamic.userCount);
            gameWorldCurrent->path.nodes[i].dynamic.userCount = 0;
        }
    }
}

void __cdecl Path_ConnectPathsForEntity(gentity_s *ent)
{
    unsigned __int16 prev; // [esp+0h] [ebp-8h]
    unsigned __int16 oldInfoIndex; // [esp+4h] [ebp-4h]

    if ( !Path_IsDynamicBlockingEntity(ent)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    5228,
                    0,
                    "%s",
                    "Path_IsDynamicBlockingEntity( ent )") )
    {
        __debugbreak();
    }
    ent->flags |= (unsigned int)&objBuf[1758][2];
    oldInfoIndex = ent->disconnectedLinks;
    if ( oldInfoIndex )
    {
        ent->disconnectedLinks = 0;
        g_path.pathLinkInfoArray[g_path.pathLinkInfoArray[oldInfoIndex].prev].next = 0;
        g_path.pathLinkInfoArray[g_path.pathLinkInfoArray[0].prev].next = oldInfoIndex;
        prev = g_path.pathLinkInfoArray[0].prev;
        g_path.pathLinkInfoArray[0].prev = g_path.pathLinkInfoArray[oldInfoIndex].prev;
        g_path.pathLinkInfoArray[oldInfoIndex].prev = prev;
        while ( oldInfoIndex )
        {
            Path_ConnectPath(
                &gameWorldCurrent->path.nodes[g_path.pathLinkInfoArray[oldInfoIndex].from],
                g_path.pathLinkInfoArray[oldInfoIndex].to);
            oldInfoIndex = g_path.pathLinkInfoArray[oldInfoIndex].next;
        }
    }
}

void __cdecl Path_ConnectPath_0(pathnode_t *node, pathlink_s *link)
{
    pathlink_s tempLink; // [esp+0h] [ebp-10h]

    if (!link->disconnectCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
            5140,
            0,
            "%s",
            "link->disconnectCount"))
    {
        __debugbreak();
    }
    if (&node->constant.Links[node->dynamic.wLinkCount] > link
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
            5141,
            0,
            "%s",
            "&node->constant.Links[node->dynamic.wLinkCount] <= link"))
    {
        __debugbreak();
    }
    Path_ValidateNode(node);
    if (!--link->disconnectCount)
    {
        tempLink = *link;
        *link = node->constant.Links[node->dynamic.wLinkCount];
        node->constant.Links[node->dynamic.wLinkCount++] = tempLink;
    }
    Path_ValidateNode(node);
}

void __cdecl Path_ConnectPath(pathnode_t *node, int toNodeNum)
{
    int j; // [esp+0h] [ebp-8h]
    pathlink_s *link; // [esp+4h] [ebp-4h]

    if ( (node->constant.spawnflags & 0x4000) == 0 )
    {
        for ( j = node->dynamic.wLinkCount; j < node->constant.totalLinkCount; ++j )
        {
            link = &node->constant.Links[j];
            if ( link->nodeNum == toNodeNum )
            {
                Path_ConnectPath_0(node, link);
                return;
            }
        }
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                        5181,
                        0,
                        "Path_ConnectPath: should be unreachable") )
            __debugbreak();
    }
}

const float dist_cutoff = 266.0f;
const float disconnectMins[3] = { -15.0, -15.0, 18.0 };
const float disconnectMaxs[3] = { 15.0, 15.0, 48.0 };

void __cdecl Path_DisconnectPathsForEntity(gentity_s *ent)
{
    unsigned int j; // [esp+1Ch] [ebp-94h]
    pathnode_t *node; // [esp+20h] [ebp-90h]
    pathnode_t *nodeTo; // [esp+24h] [ebp-8Ch]
    sightclip_t clip; // [esp+28h] [ebp-88h] BYREF
    int entityNum; // [esp+74h] [ebp-3Ch]
    float rdir[3]; // [esp+78h] [ebp-38h]
    pathlink_s *link; // [esp+84h] [ebp-2Ch]
    float start[3]; // [esp+88h] [ebp-28h]
    float mx[3]; // [esp+94h] [ebp-1Ch]
    unsigned int i; // [esp+A0h] [ebp-10h]
    float mn[3]; // [esp+A4h] [ebp-Ch]

    if ( !Path_IsDynamicBlockingEntity(ent)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    5261,
                    0,
                    "%s",
                    "Path_IsDynamicBlockingEntity( ent )") )
    {
        __debugbreak();
    }
    Path_ConnectPathsForEntity(ent);
    ent->flags &= ~0x4000000u;
    ent->iDisconnectTime = level.time;
    entityNum = ent->s.number;
    rdir[0] = dist_cutoff;
    rdir[1] = dist_cutoff;
    rdir[2] = dist_cutoff;
    mn[0] = ent->r.absmin[0] - 266.0;
    mn[1] = ent->r.absmin[1] - 266.0;
    mn[2] = ent->r.absmin[2] - 266.0;
    mx[0] = ent->r.absmax[0] + 266.0;
    mx[1] = ent->r.absmax[1] + 266.0;
    mx[2] = ent->r.absmax[2] + 266.0;
    clip.mins[0] = disconnectMins[0];
    clip.mins[1] = disconnectMins[1];
    clip.mins[2] = disconnectMins[2];
    clip.maxs[0] = disconnectMaxs[0];
    clip.maxs[1] = disconnectMaxs[1];
    clip.maxs[2] = disconnectMaxs[2];
    clip.contentmask = 0x2820011;
    clip.passEntityNum[1] = 1023;
    clip.passEntityNum[0] = 1023;
    for ( i = 0; i < g_path.actualNodeCount; ++i )
    {
        node = &gameWorldCurrent->path.nodes[i];
        start[0] = node->constant.vOrigin[0];
        start[1] = node->constant.vOrigin[1];
        start[2] = node->constant.vOrigin[2];
        if ( node->constant.type == NODE_NEGOTIATION_BEGIN
            || node->constant.type == NODE_NEGOTIATION_END
            || start[0] <= mx[0]
            && start[1] <= mx[1]
            && start[2] <= mx[2]
            && mn[0] <= start[0]
            && mn[1] <= start[1]
            && mn[2] <= start[2] )
        {
            clip.start[0] = start[0];
            clip.start[1] = start[1];
            clip.start[2] = start[2];
            for ( j = node->constant.totalLinkCount - 1; j != -1; --j )
            {
                link = &node->constant.Links[j];
                nodeTo = &gameWorldCurrent->path.nodes[link->nodeNum];
                clip.end[0] = nodeTo->constant.vOrigin[0];
                clip.end[1] = nodeTo->constant.vOrigin[1];
                clip.end[2] = nodeTo->constant.vOrigin[2];
                if ( SV_SightTraceCapsuleToEntity(&clip, entityNum) )
                    Path_DisconnectPath(ent, node, link);
            }
        }
    }
}

void Path_DisconnectPath_0(pathnode_t *node, pathlink_s *link)
{
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    pathlink_s tempLink; // [esp+0h] [ebp-10h]

    Path_ValidateNode(node);
    if (!++link->disconnectCount)
        Scr_Error("too many disconnects on a single path link (overflow on disconnect count)", 0);
    if (!link->disconnectCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
            5114,
            0,
            "%s",
            "link->disconnectCount"))
    {
        __debugbreak();
    }
    if (link->disconnectCount <= 1u)
    {
        if (--node->dynamic.wLinkCount < 0)
        {
            v4 = va("node: %d, %d", node - gameWorldCurrent->path.nodes, node->dynamic.wLinkCount);
            if (!Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                5125,
                0,
                "%s\n\t%s",
                "node->dynamic.wLinkCount >= 0",
                v4))
                __debugbreak();
        }
        if (&node->constant.Links[node->dynamic.wLinkCount] < link)
        {
            v5 = va(
                "node: %d, %d (%d) %d (%d)",
                node - gameWorldCurrent->path.nodes,
                node->dynamic.wLinkCount,
                node->constant.Links[node->dynamic.wLinkCount].nodeNum,
                link - node->constant.Links,
                link->nodeNum);
            if (!Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                5126,
                0,
                "%s\n\t%s",
                "&node->constant.Links[node->dynamic.wLinkCount] >= link",
                v5))
                __debugbreak();
        }
        tempLink = *link;
        *link = node->constant.Links[node->dynamic.wLinkCount];
        node->constant.Links[node->dynamic.wLinkCount] = tempLink;
        Path_ValidateNode(node);
    }
    else
    {
        if (&node->constant.Links[node->dynamic.wLinkCount] > link)
        {
            v3 = va(
                "node: %d, %d (%d) %d (%d)",
                node - gameWorldCurrent->path.nodes,
                node->dynamic.wLinkCount,
                node->constant.Links[node->dynamic.wLinkCount].nodeNum,
                link - node->constant.Links,
                link->nodeNum);
            if (!Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                5118,
                0,
                "%s\n\t%s",
                "&node->constant.Links[node->dynamic.wLinkCount] <= link",
                v3))
                __debugbreak();
        }
        Path_ValidateNode(node);
    }
}

void __cdecl Path_DisconnectPath(gentity_s *ent, pathnode_t *node, pathlink_s *link)
{
    PathLinkInfo *newInfo; // [esp+0h] [ebp-8h]
    unsigned __int16 newInfoIndex; // [esp+4h] [ebp-4h]

    newInfoIndex = g_path.pathLinkInfoArray[0].next;
    if (!g_path.pathLinkInfoArray[0].next)
        Com_Error(ERR_DROP, "Max number of disconnected paths exceeded");
    newInfo = &g_path.pathLinkInfoArray[newInfoIndex];
    g_path.pathLinkInfoArray[0].next = newInfo->next;
    g_path.pathLinkInfoArray[newInfo->next].prev = 0;
    newInfo->from = node - gameWorldCurrent->path.nodes;
    newInfo->to = link->nodeNum;
    if (ent->disconnectedLinks)
    {
        newInfo->prev = ent->disconnectedLinks;
        newInfo->next = g_path.pathLinkInfoArray[ent->disconnectedLinks].next;
        g_path.pathLinkInfoArray[newInfo->next].prev = newInfoIndex;
        g_path.pathLinkInfoArray[newInfo->prev].next = newInfoIndex;
    }
    else
    {
        ent->disconnectedLinks = newInfoIndex;
        newInfo->next = newInfoIndex;
        newInfo->prev = newInfoIndex;
    }
    Path_DisconnectPath_0(node, link);
}

void    Path_UpdateArcBadPlaceCount(badplace_arc_t *arc, int teamflags, int delta)
{
    float v4; // xmm0_4
    float fDistOther1; // [esp+14h] [ebp-114h]
    float fDistOther0; // [esp+1Ch] [ebp-10Ch]
    float fDistNode1; // [esp+24h] [ebp-104h]
    float fDistNode0; // [esp+2Ch] [ebp-FCh]
    float fHeight1; // [esp+34h] [ebp-F4h]
    float t1; // [esp+38h] [ebp-F0h]
    float fHeight0; // [esp+3Ch] [ebp-ECh]
    float t0; // [esp+40h] [ebp-E8h]
    float fSqrtDisc; // [esp+44h] [ebp-E4h]
    float fDiscriminant; // [esp+48h] [ebp-E0h]
    float fA; // [esp+50h] [ebp-D8h]
    float fB; // [esp+54h] [ebp-D4h]
    float vLineDelta; // [esp+58h] [ebp-D0h]
    float vLineDelta_4; // [esp+5Ch] [ebp-CCh]
    float vLineDelta_8; // [esp+60h] [ebp-C8h]
    float vOtherDelta_4; // [esp+80h] [ebp-A8h]
    float vOtherDelta_8; // [esp+84h] [ebp-A4h]
    pathnode_t *otherNode; // [esp+8Ch] [ebp-9Ch]
    pathlink_s *link; // [esp+90h] [ebp-98h]
    unsigned int j; // [esp+94h] [ebp-94h]
    float fCentroidDeltaSqrd; // [esp+98h] [ebp-90h]
    float fPosDeltaSqrd; // [esp+ACh] [ebp-7Ch]
    float vPosDelta; // [esp+B0h] [ebp-78h]
    float vPosDelta_4; // [esp+B4h] [ebp-74h]
    float vPosDelta_8; // [esp+B8h] [ebp-70h]
    pathnode_t *node; // [esp+C0h] [ebp-68h]
    unsigned int i; // [esp+C4h] [ebp-64h]
    float fMaxHeightSqrd; // [esp+C8h] [ebp-60h]
    float fMaxRadiusSqrd; // [esp+CCh] [ebp-5Ch]
    float fHeightSqrd; // [esp+D0h] [ebp-58h]
    float fRadiusSqrd; // [esp+D4h] [ebp-54h]
    float centroid; // [esp+D8h] [ebp-50h]
    float centroid_4; // [esp+DCh] [ebp-4Ch]
    float scale; // [esp+E4h] [ebp-44h]
    float forward[3]; // [esp+ECh] [ebp-3Ch] BYREF
    int bArcLessThan180; // [esp+F8h] [ebp-30h]
    BOOL v41; // [esp+FCh] [ebp-2Ch]
    float angle; // [esp+100h] [ebp-28h]
    float side1[3]; // [esp+104h] [ebp-24h] BYREF
    float side0[3]; // [esp+110h] [ebp-18h] BYREF
    //_UNKNOWN *v45; // [esp+11Ch] [ebp-Ch]
    //badplace_arc_t *arca; // [esp+120h] [ebp-8h]
    //int teamflagsa; // [esp+124h] [ebp-4h] BYREF
    //int deltaa; // [esp+128h] [ebp+0h]
    //
    //v45 = a1;
    //arca = (badplace_arc_t *)deltaa;
    YawVectors(arc->angle0, 0, side0);
    YawVectors(arc->angle1, 0, side1);
    //LODWORD(side0[0]) ^= _mask__NegFloat_;
    //LODWORD(side0[1]) ^= _mask__NegFloat_;
    //LODWORD(side0[2]) ^= _mask__NegFloat_;
    side0[0] = -side0[0];
    side0[1] = -side0[1];
    side0[2] = -side0[2];
    side0[2] = (float)(side0[0] * arc->origin[0]) + (float)(side0[1] * arc->origin[1]);
    side1[2] = (float)(side1[0] * arc->origin[0]) + (float)(side1[1] * arc->origin[1]);
    angle = arc->angle1 - arc->angle0;
    if (angle < 0.0)
        angle = angle + 360.0;
    v41 = angle < 180.0;
    bArcLessThan180 = v41;
    YawVectors((float)(angle * 0.5) + arc->angle0, forward, 0);
    //v4 = __libm_sse2_sin((float)(angle * 0.0087266462));
    v4 = sin((float)(angle * 0.0087266462));
    scale = (float)((float)(v4 / angle) * 76.394371) * arc->radius;
    centroid = (float)(scale * forward[0]) + arc->origin[0];
    centroid_4 = (float)(scale * forward[1]) + arc->origin[1];
    fRadiusSqrd = arc->radius * arc->radius;
    fHeightSqrd = arc->halfheight * arc->halfheight;
    fMaxRadiusSqrd = (float)(arc->radius + 256.0) * (float)(arc->radius + 256.0);
    fMaxHeightSqrd = (float)(arc->halfheight + 128.0) * (float)(arc->halfheight + 128.0);
    for (i = 0; i < g_path.actualNodeCount; ++i)
    {
        node = &gameWorldCurrent->path.nodes[i];
        vPosDelta = node->constant.vOrigin[0] - arc->origin[0];
        vPosDelta_4 = node->constant.vOrigin[1] - arc->origin[1];
        vPosDelta_8 = node->constant.vOrigin[2] - arc->origin[2];
        fPosDeltaSqrd = (float)(vPosDelta * vPosDelta) + (float)(vPosDelta_4 * vPosDelta_4);
        if (fPosDeltaSqrd < fMaxRadiusSqrd && (float)(vPosDelta_8 * vPosDelta_8) < fMaxHeightSqrd)
        {
            fCentroidDeltaSqrd = (float)((float)(centroid - node->constant.vOrigin[0])
                * (float)(centroid - node->constant.vOrigin[0]))
                + (float)((float)(centroid_4 - node->constant.vOrigin[1])
                    * (float)(centroid_4 - node->constant.vOrigin[1]));
            for (j = 0; j < node->constant.totalLinkCount; ++j)
            {
                link = &node->constant.Links[j];
                otherNode = &gameWorldCurrent->path.nodes[link->nodeNum];
                vOtherDelta_8 = otherNode->constant.vOrigin[2] - arc->origin[2];
                if ((float)((float)((float)(centroid - otherNode->constant.vOrigin[0])
                    * (float)(centroid - otherNode->constant.vOrigin[0]))
                    + (float)((float)(centroid_4 - otherNode->constant.vOrigin[1])
                        * (float)(centroid_4 - otherNode->constant.vOrigin[1]))) <= fCentroidDeltaSqrd)
                {
                    if (vPosDelta_8 < arc->halfheight)
                    {
                        //if (COERCE_FLOAT(LODWORD(arc->halfheight) ^ _mask__NegFloat_) >= vPosDelta_8 && COERCE_FLOAT(LODWORD(arc->halfheight) ^ _mask__NegFloat_) >= vOtherDelta_8)
                        if ((-(arc->halfheight)) >= vPosDelta_8 && (-(arc->halfheight)) >= vOtherDelta_8)
                        {
                            continue;
                        }
                    }
                    else if (vOtherDelta_8 >= arc->halfheight)
                    {
                        continue;
                    }
                    if (fPosDeltaSqrd <= fRadiusSqrd)
                        goto LABEL_27;
                    vOtherDelta_4 = otherNode->constant.vOrigin[1] - arc->origin[1];
                    if ((float)((float)((float)(otherNode->constant.vOrigin[0] - arc->origin[0])
                        * (float)(otherNode->constant.vOrigin[0] - arc->origin[0]))
                        + (float)(vOtherDelta_4 * vOtherDelta_4)) <= fRadiusSqrd)
                        goto LABEL_27;
                    vLineDelta = otherNode->constant.vOrigin[0] - node->constant.vOrigin[0];
                    vLineDelta_4 = otherNode->constant.vOrigin[1] - node->constant.vOrigin[1];
                    vLineDelta_8 = otherNode->constant.vOrigin[2] - node->constant.vOrigin[2];
                    //LODWORD(fB) = COERCE_UNSIGNED_INT((float)(vLineDelta * vPosDelta) + (float)(vLineDelta_4 * vPosDelta_4)) ^ _mask__NegFloat_;
                    (fB) = -((float)(vLineDelta * vPosDelta) + (float)(vLineDelta_4 * vPosDelta_4));
                    if (fB > 0.0)
                    {
                        fA = (float)(vLineDelta * vLineDelta) + (float)(vLineDelta_4 * vLineDelta_4);
                        fDiscriminant = (float)(fB * fB) - (float)(fA * (float)(fPosDeltaSqrd - fRadiusSqrd));
                        if (fDiscriminant > 0.0)
                        {
                            fSqrtDisc = sqrtf(fDiscriminant);
                            t0 = (float)(fB - fSqrtDisc) / fA;
                            if (t0 < 1.0)
                            {
                                fHeight0 = (float)(vLineDelta_8 * t0) + vPosDelta_8;
                                if ((float)(fHeight0 * fHeight0) <= fHeightSqrd
                                    || (t1 = (float)(fB + fSqrtDisc) / fA, t1 < 1.0)
                                    && ((fHeight1 = (float)(vLineDelta_8 * t1) + vPosDelta_8, (float)(fHeight1 * fHeight1) <= fHeightSqrd)
                                        || (float)(fHeight0 * fHeight1) < 0.0))
                                {
                                LABEL_27:
                                    if (arc->angle0 == 0.0 && arc->angle1 == 360.0)
                                        goto LABEL_38;
                                    fDistNode0 = (float)((float)(node->constant.vOrigin[0] * side0[0])
                                        + (float)(node->constant.vOrigin[1] * side0[1]))
                                        - side0[2];
                                    fDistNode1 = (float)((float)(node->constant.vOrigin[0] * side1[0])
                                        + (float)(node->constant.vOrigin[1] * side1[1]))
                                        - side1[2];
                                    fDistOther0 = (float)((float)(otherNode->constant.vOrigin[0] * side0[0])
                                        + (float)(otherNode->constant.vOrigin[1] * side0[1]))
                                        - side0[2];
                                    fDistOther1 = (float)((float)(otherNode->constant.vOrigin[0] * side1[0])
                                        + (float)(otherNode->constant.vOrigin[1] * side1[1]))
                                        - side1[2];
                                    if (bArcLessThan180)
                                    {
                                        if ((fDistNode0 >= 0.0 || fDistOther0 >= 0.0) && (fDistNode1 >= 0.0 || fDistOther1 >= 0.0))
                                        {
                                        LABEL_38:
                                            Path_UpdateBadPlaceCountForLink(link, teamflags, delta);
                                            continue;
                                        }
                                    }
                                    else if (fDistNode0 >= 0.0 || fDistOther0 >= 0.0 || fDistNode1 >= 0.0 || fDistOther1 >= 0.0)
                                    {
                                        goto LABEL_38;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void __cdecl Path_UpdateBadPlaceCountForLink(pathlink_s *link, int teamflags, int delta)
{
    const char *v3; // eax
    unsigned __int8 v4; // [esp+0h] [ebp-10h]
    int v5; // [esp+4h] [ebp-Ch]
    int iNewCount; // [esp+8h] [ebp-8h]
    unsigned int i; // [esp+Ch] [ebp-4h]

    for ( i = 0; i < 4; ++i )
    {
        if ( (teamflags & (1 << i)) != 0 )
        {
            iNewCount = delta + link->ubBadPlaceCount[i];
            if ( iNewCount >= 0 )
            {
                if ( iNewCount > 255 )
                    Scr_Error("Bad place overflow -- count exceeds 255", 0);
            }
            else
            {
                v3 = va("Bad place underflow -- negative count in slot (%i) val (%i)", i, iNewCount);
                Scr_Error(v3, 0);
            }
            if ( iNewCount < 255 )
                v5 = iNewCount;
            else
                v5 = 255;
            if ( v5 > 0 )
                v4 = v5;
            else
                v4 = 0;
            link->ubBadPlaceCount[i] = v4;
        }
    }
}

void __cdecl Path_CheckForInwardLinks(
                pathnode_t *node,
                int currDepth,
                int maxDepth,
                float fRadiusSqrd,
                float fHeightSqrd,
                float fMaxRadiusSqrd,
                float fMaxHeightSqrd,
                float *centroid,
                badplace_arc_t *arc,
                float *side0,
                float *side1,
                int teamflags,
                int delta)
{
    float v13; // [esp+4Ch] [ebp-A4h]
    float v14; // [esp+5Ch] [ebp-94h]
    int j; // [esp+64h] [ebp-8Ch]
    float fDistNode1; // [esp+68h] [ebp-88h]
    float fDistNode0; // [esp+6Ch] [ebp-84h]
    float fDistOther0; // [esp+70h] [ebp-80h]
    float fDistOther1; // [esp+74h] [ebp-7Ch]
    float fHeight1; // [esp+78h] [ebp-78h]
    float t1; // [esp+7Ch] [ebp-74h]
    float t0; // [esp+80h] [ebp-70h]
    float fDiscriminant; // [esp+84h] [ebp-6Ch]
    float fSqrtDisc; // [esp+88h] [ebp-68h]
    float fB; // [esp+90h] [ebp-60h]
    float fA; // [esp+94h] [ebp-5Ch]
    float fHeight0; // [esp+98h] [ebp-58h]
    int i; // [esp+9Ch] [ebp-54h]
    pathnode_t *finalNode; // [esp+A0h] [ebp-50h]
    pathlink_s *link; // [esp+A8h] [ebp-48h]
    float vPosDelta; // [esp+ACh] [ebp-44h]
    float vPosDelta_4; // [esp+B0h] [ebp-40h]
    float vPosDelta_8; // [esp+B4h] [ebp-3Ch]
    float angle; // [esp+BCh] [ebp-34h]
    float vOtherDelta_4; // [esp+C4h] [ebp-2Ch]
    float vOtherDelta_8; // [esp+C8h] [ebp-28h]
    float fCentroidDeltaSqrd; // [esp+CCh] [ebp-24h]
    pathnode_t *currNode; // [esp+D4h] [ebp-1Ch]
    pathnode_t *otherNode; // [esp+D8h] [ebp-18h]
    pathnode_t *otherNodea; // [esp+D8h] [ebp-18h]
    float vLineDelta; // [esp+DCh] [ebp-14h]
    float vLineDelta_4; // [esp+E0h] [ebp-10h]
    float vLineDelta_8; // [esp+E4h] [ebp-Ch]
    float fPosDeltaSqrd; // [esp+E8h] [ebp-8h]
    bool allNodesDone; // [esp+EFh] [ebp-1h]

    currNode = node;
    finalNode = node;
    angle = arc->angle1 - arc->angle0;
    if ( angle < 0.0 )
        angle = angle + 360.0;
LABEL_3:
    if ( currNode )
    {
        vPosDelta = currNode->constant.vOrigin[0] - arc->origin[0];
        vPosDelta_4 = currNode->constant.vOrigin[1] - arc->origin[1];
        vPosDelta_8 = currNode->constant.vOrigin[2] - arc->origin[2];
        fPosDeltaSqrd = (float)(vPosDelta * vPosDelta) + (float)(vPosDelta_4 * vPosDelta_4);
        if ( fPosDeltaSqrd < fMaxRadiusSqrd && (float)(vPosDelta_8 * vPosDelta_8) < fMaxHeightSqrd )
        {
            v14 = centroid[1] - currNode->constant.vOrigin[1];
            fCentroidDeltaSqrd = (float)((float)(*centroid - currNode->constant.vOrigin[0])
                                                                 * (float)(*centroid - currNode->constant.vOrigin[0]))
                                                 + (float)(v14 * v14);
            for ( i = 0; ; ++i )
            {
                if ( i >= currNode->constant.totalLinkCount )
                {
                    if ( !currNode->transient.pNextOpen )
                        finalNode = currNode;
                    currNode = currNode->transient.pNextOpen;
                    goto LABEL_3;
                }
                link = &currNode->constant.Links[i];
                otherNode = &gameWorldCurrent->path.nodes[link->nodeNum];
                vOtherDelta_8 = otherNode->constant.vOrigin[2] - arc->origin[2];
                v13 = centroid[1] - otherNode->constant.vOrigin[1];
                if ( (float)((float)((float)(*centroid - otherNode->constant.vOrigin[0])
                                                     * (float)(*centroid - otherNode->constant.vOrigin[0]))
                                     + (float)(v13 * v13)) <= fCentroidDeltaSqrd )
                {
                    if ( vPosDelta_8 < arc->halfheight )
                    {
                        if ( (-(arc->halfheight)) >= vPosDelta_8 && (-(arc->halfheight)) >= vOtherDelta_8 )
                        {
                            continue;
                        }
                    }
                    else if ( vOtherDelta_8 >= arc->halfheight )
                    {
                        continue;
                    }
                    if ( fPosDeltaSqrd <= fRadiusSqrd )
                        goto LABEL_25;
                    vOtherDelta_4 = otherNode->constant.vOrigin[1] - arc->origin[1];
                    if ( (float)((float)((float)(otherNode->constant.vOrigin[0] - arc->origin[0])
                                                         * (float)(otherNode->constant.vOrigin[0] - arc->origin[0]))
                                         + (float)(vOtherDelta_4 * vOtherDelta_4)) <= fRadiusSqrd )
                        goto LABEL_25;
                    vLineDelta = otherNode->constant.vOrigin[0] - node->constant.vOrigin[0];
                    vLineDelta_4 = otherNode->constant.vOrigin[1] - node->constant.vOrigin[1];
                    vLineDelta_8 = otherNode->constant.vOrigin[2] - node->constant.vOrigin[2];
                    //LODWORD(fB) = COERCE_UNSIGNED_INT((float)(vLineDelta * vPosDelta) + (float)(vLineDelta_4 * vPosDelta_4)) ^ _mask__NegFloat_;
                    (fB) = -((float)(vLineDelta * vPosDelta) + (float)(vLineDelta_4 * vPosDelta_4));
                    if ( fB > 0.0 )
                    {
                        fA = (float)(vLineDelta * vLineDelta) + (float)(vLineDelta_4 * vLineDelta_4);
                        fDiscriminant = (float)(fB * fB) - (float)(fA * (float)(fPosDeltaSqrd - fRadiusSqrd));
                        if ( fDiscriminant > 0.0 )
                        {
                            fSqrtDisc = sqrtf(fDiscriminant);
                            t0 = (float)(fB - fSqrtDisc) / fA;
                            if ( t0 < 1.0 )
                            {
                                fHeight0 = (float)(vLineDelta_8 * t0) + vPosDelta_8;
                                if ( (float)(fHeight0 * fHeight0) <= fHeightSqrd
                                    || (t1 = (float)(fB + fSqrtDisc) / fA, t1 < 1.0)
                                    && ((fHeight1 = (float)(vLineDelta_8 * t1) + vPosDelta_8, (float)(fHeight1 * fHeight1) <= fHeightSqrd)
                                     || (float)(fHeight0 * fHeight1) < 0.0) )
                                {
LABEL_25:
                                    if ( arc->angle0 == 0.0 && arc->angle1 == 360.0 )
                                        goto LABEL_36;
                                    fDistNode0 = (float)((float)(currNode->constant.vOrigin[0] * *side0)
                                                                         + (float)(currNode->constant.vOrigin[1] * side0[1]))
                                                         - side0[2];
                                    fDistNode1 = (float)((float)(currNode->constant.vOrigin[0] * *side1)
                                                                         + (float)(currNode->constant.vOrigin[1] * side1[1]))
                                                         - side1[2];
                                    fDistOther0 = (float)((float)(otherNode->constant.vOrigin[0] * *side0)
                                                                            + (float)(otherNode->constant.vOrigin[1] * side0[1]))
                                                            - side0[2];
                                    fDistOther1 = (float)((float)(otherNode->constant.vOrigin[0] * *side1)
                                                                            + (float)(otherNode->constant.vOrigin[1] * side1[1]))
                                                            - side1[2];
                                    if ( angle >= 180.0 )
                                    {
                                        if ( fDistNode0 >= 0.0 || fDistOther0 >= 0.0 || fDistNode1 >= 0.0 || fDistOther1 >= 0.0 )
                                            goto LABEL_36;
                                    }
                                    else if ( (fDistNode0 >= 0.0 || fDistOther0 >= 0.0) && (fDistNode1 >= 0.0 || fDistOther1 >= 0.0) )
                                    {
LABEL_36:
                                        Path_UpdateBadPlaceCountForLink(link, teamflags, delta);
                                        continue;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        allNodesDone = 0;
        while ( !allNodesDone )
        {
            allNodesDone = node == finalNode;
            for ( j = 0; j < node->constant.totalLinkCount; ++j )
            {
                otherNodea = &gameWorldCurrent->path.nodes[node->constant.Links[j].nodeNum];
                if ( currDepth != maxDepth && otherNodea->transient.iSearchFrame <= level.iSearchFrame )
                {
                    otherNodea->transient.pNextOpen = finalNode->transient.pNextOpen;
                    otherNodea->transient.pPrevOpen = finalNode;
                    finalNode->transient.pNextOpen = otherNodea;
                    otherNodea->transient.iSearchFrame = level.iSearchFrame + 1;
                }
            }
            node = node->transient.pNextOpen;
            if ( !node )
                allNodesDone = 1;
        }
        if ( finalNode->transient.pNextOpen )
            Path_CheckForInwardLinks(
                finalNode->transient.pNextOpen,
                currDepth + 1,
                maxDepth,
                fRadiusSqrd,
                fHeightSqrd,
                fMaxRadiusSqrd,
                fMaxHeightSqrd,
                centroid,
                arc,
                side0,
                side1,
                teamflags,
                delta);
    }
}

// local variable allocation has failed, the output may be wrong!
void    Path_UpdateLimitedDepthArcBadPlaceCount(
                badplace_arc_t *arc,
                int teamflags,
                int delta,
                int depth)
{
    double v5; // xmm0_8
    float side1end[3]; // [esp+28h] [ebp-3FCh] BYREF
    float side0end[3]; // [esp+34h] [ebp-3F0h] BYREF
    pathnode_t TopParent; // [esp+40h] [ebp-3E4h] BYREF
    pathnode_t *node; // [esp+C0h] [ebp-364h]
    int nodeCount; // [esp+C4h] [ebp-360h] BYREF
    pathsort_t pathReturn[64]; // [esp+C8h] [ebp-35Ch] BYREF
    float fMaxHeightSqrd; // [esp+3C8h] [ebp-5Ch]
    float fMaxRadiusSqrd; // [esp+3CCh] [ebp-58h]
    float fHeightSqrd; // [esp+3D0h] [ebp-54h]
    float fRadiusSqrd; // [esp+3D4h] [ebp-50h]
    float centroid[3]; // [esp+3D8h] [ebp-4Ch] BYREF
    float scale; // [esp+3E4h] [ebp-40h]
    int v18; // [esp+3E8h] [ebp-3Ch]
    float forward[3]; // [esp+3ECh] [ebp-38h] BYREF
    float side1[3]; // [esp+3F8h] [ebp-2Ch] BYREF
    float side0[3]; // [esp+404h] [ebp-20h] BYREF
    float angle; // [esp+410h] [ebp-14h]
    //_UNKNOWN *v23; // [esp+418h] [ebp-Ch]
    //badplace_arc_t *arca; // [esp+41Ch] [ebp-8h]
    //int teamflagsa; // [esp+420h] [ebp-4h] BYREF
    //int deltaa; // [esp+424h] [ebp+0h]
    //
    //v23 = a1;
    //arca = (badplace_arc_t *)deltaa;
    angle = arc->angle1 - arc->angle0;
    AngleNormalize360(angle);
    YawVectors(arc->angle0, 0, side0);
    YawVectors(arc->angle1, 0, side1);
    side0[0] = -side0[0];
    side0[1] = -side0[1];
    side0[2] = -side0[2];
    side0[2] = (float)(side0[0] * arc->origin[0]) + (float)(side0[1] * arc->origin[1]);
    side1[2] = (float)(side1[0] * arc->origin[0]) + (float)(side1[1] * arc->origin[1]);
    YawVectors((float)(angle * 0.5) + arc->angle0, forward, 0);
    v5 = (float)(angle * 0.0087266462);
    //__libm_sse2_sin(*(long double *)side1end);
    //*(float *)&v5 = v5;
    v5 = sin(v5);
    v18 = LODWORD(v5);
    scale = (float)((float)(*(float *)&v5 / angle) * 76.394371) * arc->radius;
    centroid[0] = (float)(scale * forward[0]) + arc->origin[0];
    centroid[1] = (float)(scale * forward[1]) + arc->origin[1];
    centroid[2] = (float)(scale * forward[2]) + arc->origin[2];
    fRadiusSqrd = arc->radius * arc->radius;
    fHeightSqrd = arc->halfheight * arc->halfheight;
    fMaxRadiusSqrd = (float)(arc->radius + 256.0) * (float)(arc->radius + 256.0);
    fMaxHeightSqrd = (float)(arc->halfheight + 128.0) * (float)(arc->halfheight + 128.0);
    memset(pathReturn, 0, sizeof(pathReturn));
    node = Path_NearestNode(
        arc->origin,
        pathReturn,
        -2,
        arc->radius,
        &nodeCount,
        64,
        NEAREST_NODE_DONT_DO_HEIGHT_CHECK);
    if (node || nodeCount > 0)
    {
        if (!node)
            node = pathReturn[0].node;
        goto LABEL_10;
    }
    node = Path_NearestNode(
        centroid,
        pathReturn,
        -2,
        arc->radius * 0.5,
        &nodeCount,
        64,
        NEAREST_NODE_DONT_DO_HEIGHT_CHECK);
    if (node || nodeCount > 0)
    {
        if (!node)
            node = pathReturn[0].node;
    LABEL_10:
        node->transient.pPrevOpen = &TopParent;
        node->transient.pNextOpen = 0;
        node->transient.iSearchFrame = level.iSearchFrame + 1;
        TopParent.transient.pNextOpen = node;
        Path_CheckForInwardLinks(
            node,
            0,
            depth,
            fRadiusSqrd,
            fHeightSqrd,
            fMaxRadiusSqrd,
            fMaxHeightSqrd,
            centroid,
            arc,
            side0,
            side1,
            teamflags,
            delta);
        while (node)
        {
            node->transient.iSearchFrame = level.iSearchFrame;
            node = node->transient.pNextOpen;
        }
        side0end[0] = arc->origin[0] + side0[0];
        side0end[1] = arc->origin[1] + side0[1];
        side0end[2] = arc->origin[2] + side0[2];
        side1end[0] = arc->origin[0] + side1[0];
        side1end[1] = arc->origin[1] + side1[1];
        side1end[2] = arc->origin[2] + side1[2];
        side1end[2] = arc->origin[2];
        side0end[2] = side1end[2];
        G_DebugLine(arc->origin, side1end, colorOrange, 0);
        G_DebugLine(arc->origin, side0end, colorOrange, 0);
    }
}

void __cdecl Path_UpdateBrushBadPlaceCount(gentity_s *brushEnt, int teamflags, int delta)
{
    pathlink_s *link; // [esp+14h] [ebp-3828h]
    pathnode_t *node; // [esp+18h] [ebp-3824h]
    pathnode_t *v5; // [esp+18h] [ebp-3824h]
    unsigned int j; // [esp+1Ch] [ebp-3820h]
    pathsort_t nodes[512]; // [esp+24h] [ebp-3818h] BYREF
    float maxHeight; // [esp+1824h] [ebp-2018h]
    float maxDist; // [esp+1828h] [ebp-2014h]
    unsigned int v10; // [esp+182Ch] [ebp-2010h]
    unsigned int i; // [esp+1830h] [ebp-200Ch]
    unsigned __int8 dst[8196]; // [esp+1834h] [ebp-2008h] BYREF

    if ( !brushEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 5763, 0, "%s", "brushEnt") )
    {
        __debugbreak();
    }
    if ( (brushEnt->r.currentAngles[0] != 0.0 || brushEnt->r.currentAngles[2] != 0.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    5764,
                    0,
                    "%s",
                    "brushEnt->r.currentAngles[PITCH] == 0.f && brushEnt->r.currentAngles[ROLL] == 0.f") )
    {
        __debugbreak();
    }
    memset(dst, 0, 0x2000u);
    maxDist = RadiusFromBounds2D(brushEnt->r.mins, brushEnt->r.maxs);
    maxHeight = brushEnt->r.maxs[2] - brushEnt->r.mins[2];
    if ( maxDist <= 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 5771, 0, "%s", "radius > 0.f") )
    {
        __debugbreak();
    }
    if ( maxHeight <= 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp", 5772, 0, "%s", "height > 0.f") )
    {
        __debugbreak();
    }
    v10 = Path_NodesInCylinder(brushEnt->r.currentOrigin, maxDist, maxHeight, nodes, 512, -2);
    if ( v10 >= 0x200
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.cpp",
                    5775,
                    0,
                    "%s",
                    "nodeCount < static_cast< uint >( MAX_NODES_IN_BRUSH )") )
    {
        __debugbreak();
    }
    for ( i = 0; i < v10; ++i )
    {
        node = nodes[i].node;
        if ( SV_EntityContact(node->constant.vOrigin, node->constant.vOrigin, brushEnt) )
            dst[node - gameWorldCurrent->path.nodes] = 1;
    }
    for ( i = 0; i < g_path.actualNodeCount; ++i )
    {
        v5 = &gameWorldCurrent->path.nodes[i];
        if ( !dst[i] )
        {
            for ( j = 0; j < v5->constant.totalLinkCount; ++j )
            {
                link = &v5->constant.Links[j];
                if ( dst[link->nodeNum] )
                    Path_UpdateBadPlaceCountForLink(link, teamflags, delta);
            }
        }
    }
}

int __cdecl Path_IsNodeInArc(
                pathnode_t *pNode,
                const float *origin,
                float radius,
                float angle0,
                float angle1,
                float halfHeight)
{
    return IsPosInsideArc(pNode->constant.vOrigin, 15.0, origin, radius, angle0, angle1, halfHeight);
}

bool __cdecl IsNodeEnabled(const pathnode_t *pNode)
{
    return pNode && (pNode->constant.spawnflags & 0x200) == 0;
}

pathnode_t *__cdecl G_FindPathNode(SpawnVar *spawnVar, nodeType type, int gameId)
{
    pathnode_t *node; // [esp+Ch] [ebp-9Ch]
    float dist2; // [esp+10h] [ebp-98h]
    int score; // [esp+14h] [ebp-94h]
    unsigned int i; // [esp+18h] [ebp-90h]
    int best_score; // [esp+1Ch] [ebp-8Ch]
    pathnode_t compare_node; // [esp+20h] [ebp-88h] BYREF
    pathnode_t *best_node; // [esp+A4h] [ebp-4h]

    memset(&compare_node, 0, sizeof(compare_node));
    G_ParsePathnodeFields(spawnVar, &compare_node, type);
    best_score = 1;
    best_node = 0;
    for ( i = 0; i < gameWorldCurrent->path.nodeCount; ++i )
    {
        score = 0;
        node = &gameWorldCurrent->path.nodes[i];
        dist2 = (float)((float)(compare_node.constant.vOrigin[0] - node->constant.vOrigin[0])
                                    * (float)(compare_node.constant.vOrigin[0] - node->constant.vOrigin[0]))
                    + (float)((float)(compare_node.constant.vOrigin[1] - node->constant.vOrigin[1])
                                    * (float)(compare_node.constant.vOrigin[1] - node->constant.vOrigin[1]));
        if ( dist2 < 100.0 )
        {
            score = 1;
            if ( dist2 < 0.5 )
                score = 2;
        }
        if ( node->constant.target && node->constant.target == compare_node.constant.target )
            ++score;
        if ( node->constant.targetname && node->constant.targetname == compare_node.constant.targetname )
            ++score;
        if ( node->constant.script_noteworthy && node->constant.script_noteworthy == compare_node.constant.script_noteworthy )
            ++score;
        if ( score > best_score )
        {
            best_node = &gameWorldCurrent->path.nodes[i];
            best_score = score;
        }
    }
    if ( gameId > 0 && gameId < (signed int)gameWorldCurrent->path.nodeCount && !best_node )
        return &gameWorldCurrent->path.nodes[gameId];
    return best_node;
}

void __cdecl G_ProcessPathnodeCommand(const RadiantCommand *command, SpawnVar *spawnVar)
{
    unsigned int *p_fDist; // edx
    unsigned int *v3; // ecx
    unsigned int v4; // [esp+0h] [ebp-30h]
    unsigned int tempLinks; // [esp+4h] [ebp-2Ch]
    unsigned int ilink; // [esp+8h] [ebp-28h]
    pathnode_t *othernode; // [esp+Ch] [ebp-24h]
    unsigned int j; // [esp+10h] [ebp-20h]
    unsigned int nodeindex; // [esp+14h] [ebp-1Ch]
    int nodeIndex; // [esp+18h] [ebp-18h]
    pathnode_t *node; // [esp+1Ch] [ebp-14h]
    RadiantCommandType commandType; // [esp+20h] [ebp-10h]
    char *classname; // [esp+24h] [ebp-Ch]
    nodeType nodetype; // [esp+28h] [ebp-8h]
    int gameId; // [esp+2Ch] [ebp-4h]

    node = 0;
    classname = GetPairValue(spawnVar, "classname");
    nodetype = (nodeType)G_GetNodeTypeFromClassname(classname);
    commandType = command->type;
    gameId = G_GetGameIdMapping(command->liveUpdateId);
    if ( commandType == RADIANT_COMMAND_CREATE )
    {
        G_SpawnPathnodeStatic(spawnVar, classname);
        G_SpawnPathnodeDynamic(spawnVar);
        g_radiant_selected_pathnode = &gameWorldCurrent->path.nodes[gameWorldCurrent->path.nodeCount - 1];
        nodeIndex = Path_ConvertNodeToIndex(g_radiant_selected_pathnode);
        G_DropPathNodeToFloor(nodeIndex);
        Path_ConnectPathsForSingleNode(g_radiant_selected_pathnode);
        //BLOPS_NULLSUB();
        Com_Printf(5, "Radiant Live Update: Created new path node\n");
        G_AssignGameIdMapping(command->liveUpdateId, nodeIndex);
    }
    else if ( commandType == RADIANT_COMMAND_DELETE )
    {
        g_radiant_selected_pathnode = G_FindPathNode(spawnVar, nodetype, gameId);
        if ( g_radiant_selected_pathnode )
        {
            nodeindex = Path_ConvertNodeToIndex(g_radiant_selected_pathnode);
            for ( j = 0; j < gameWorldCurrent->path.nodeCount; ++j )
            {
                othernode = &gameWorldCurrent->path.nodes[j];
                for ( ilink = 0; ilink < othernode->constant.totalLinkCount; ++ilink )
                {
                    if ( othernode->constant.Links[ilink].nodeNum == nodeindex )
                    {
                        p_fDist = (unsigned int *)&othernode->constant.Links[--othernode->constant.totalLinkCount].fDist;
                        v3 = (unsigned int *)&othernode->constant.Links[ilink].fDist;
                        *v3 = *p_fDist;
                        v3[1] = p_fDist[1];
                        v3[2] = p_fDist[2];
                        othernode->dynamic.wLinkCount = othernode->constant.totalLinkCount;
                    }
                }
            }
            g_radiant_selected_pathnode->constant.totalLinkCount = 0;
            g_radiant_selected_pathnode->dynamic.wLinkCount = 0;
            g_radiant_selected_pathnode->constant.type = NODE_BADNODE;
        }
        g_radiant_selected_pathnode = 0;
        //BLOPS_NULLSUB();
    }
    else
    {
        if ( commandType )
        {
            if ( commandType == RADIANT_COMMAND_UPDATE_SELECTED )
            {
                node = g_radiant_selected_pathnode;
            }
            else if ( commandType == RADIANT_COMMAND_UPDATE )
            {
                node = G_FindPathNode(spawnVar, nodetype, gameId);
            }
        }
        else
        {
            g_radiant_selected_pathnode = G_FindPathNode(spawnVar, nodetype, gameId);
            node = g_radiant_selected_pathnode;
            //BLOPS_NULLSUB();
            if ( !node )
                Com_Printf(5, "Radiant Live Update: Can't find pathnode.    Maps out of sync (Radiant/Game), re-bsp!\n");
        }
        if ( node )
        {
            tempLinks = node->constant.spawnflags & 0x4000;
            G_ParsePathnodeFields(spawnVar, node, nodetype);
            if ( tempLinks )
                node->constant.spawnflags |= 0x4000u;
            v4 = Path_ConvertNodeToIndex(node);
            G_DropPathNodeToFloor(v4);
            Path_ConnectPathsForSingleNode(node);
            G_AssignGameIdMapping(command->liveUpdateId, v4);
        }
    }
}

void __cdecl G_ClearSelectedPathNode()
{
    g_radiant_selected_pathnode = 0;
    //BLOPS_NULLSUB();
}

//pathnode_parent_t *__thiscall phys_simple_allocator<pathnode_parent_t>::allocate(
//                phys_simple_allocator<pathnode_parent_t> *this)
//{
//    char *slot; // [esp+14h] [ebp-4h]
//
//    slot = PMM_ALLOC(0x18u, 4u);
//    if ( !slot )
//        return 0;
//    ++this->m_count;
//    return (pathnode_parent_t *)slot;
//}
//
