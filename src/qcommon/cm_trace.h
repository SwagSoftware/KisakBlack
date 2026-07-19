#pragma once
#include <DynEntity/DynEntity_gamestate.h>
//#include <physics/phys_colgeom.h>

struct heli_height_lock_patches_t // sizeof=0x18
{                                       // XREF: .data:heli_height_lock_patches_t * heli_height_lock_patches/r
    int brushmodel;                     // XREF: CMD_Heli_IsInsideHeliHeightLock+BB/r
                                        // GScr_IsMissileInsideHeightLock+7E/r ...
    float origin[3];                    // XREF: CMD_Heli_IsInsideHeliHeightLock+D0/o
                                        // CMD_Heli_IsInsideHeliHeightLock+114/o ...
    unsigned __int16 targetname;        // XREF: G_FreeVehiclePaths(void)+97/o
                                        // G_SpawnHeliHeightLock(SpawnVar *)+17E/w ...
    // padding byte
    // padding byte
    int enabled;                        // XREF: CM_GetHeliHeight(float const * const,float,float *)+CD/r
                                        // G_SpawnHeliHeightLock(SpawnVar *)+1A2/w
};

struct IgnoreEntParams // sizeof=0xC
{                                       // XREF: Flame_Server_Trace/r
    int baseEntity;                     // XREF: setup_query_input(gjkcc_input_t const &,phys_vec3 const &,phys_vec3 const &,phys_vec3 const &,phys_vec3 const &,int,int,gjk_query_input &)+108/r
    int parentEntity;                   // XREF: setup_query_input(gjkcc_input_t const &,phys_vec3 const &,phys_vec3 const &,phys_vec3 const &,phys_vec3 const &,int,int,gjk_query_input &)+111/r
    bool ignoreSelf;                    // XREF: CG_ScanForCrosshairEntityInternal+223/w
    bool ignoreParent;                  // XREF: CG_ScanForCrosshairEntityInternal+242/w
    bool ignoreSiblings;                // XREF: CG_ScanForCrosshairEntityInternal+231/w
    bool ignoreChildren;                // XREF: CG_ScanForCrosshairEntityInternal+22A/w
};

struct col_context_t // sizeof=0x28
{                                       // XREF: ?Turret_PlaceTurret_Internal@@YAHW4turret_handler@@HHQBM11QAM2@Z/r
                                        // Turret_PlaceTurret_UpdateFooting/r ...
    int mask;                           // XREF: turret_SightTrace(float const * const,float const * const,int,int,gentity_s *):loc_6A21DC/r
                                        // turret_SightTrace(float const * const,float const * const,int,int,gentity_s *)+92/w ...
    const struct col_prim_t *prims;            // XREF: PM_trace(pmove_t *,trace_t *,float const * const,float const * const,float const * const,float const * const,int,int)+AC/w
                                        // PM_playerTrace(pmove_t *,trace_t *,float const * const,float const * const,float const * const,float const * const,int,int)+BD/w ...
    int nprims;                         // XREF: PM_trace(pmove_t *,trace_t *,float const * const,float const * const,float const * const,float const * const,int,int)+B8/w
                                        // PM_playerTrace(pmove_t *,trace_t *,float const * const,float const * const,float const * const,float const * const,int,int)+C9/w ...
    IgnoreEntParams *ignoreEntParams;   // XREF: CG_ScanForCrosshairEntityInternal+2FB/w
                                        // G_LocationalTrace(trace_t *,float const * const,float const * const,int,int,uchar *,int (*)(int,col_context_t &))+25/w ...
    int passEntityNum0;                 // XREF: FlashbangBlastEnt+80/w
                                        // G_RunCorpseMove(gentity_s *)+60D/w ...
    int passEntityNum1;                 // XREF: VEH_UpdateAim+897/w
                                        // Player_GetUseList+E04/w ...
    int staticmodels;
    int locational;
    unsigned __int8 *priorityMap;       // XREF: G_LocationalTrace(trace_t *,float const * const,float const * const,int,int,uchar *,int (*)(int,col_context_t &))+37/w
                                        // G_LocationalTraceAllowChildren(trace_t *,float const * const,float const * const,int,int,uchar *)+3B/w
    int (__cdecl *collide_entity_func)(int, col_context_t *);
                                        // XREF: G_LocationalTrace(trace_t *,float const * const,float const * const,int,int,uchar *,int (*)(int,col_context_t &))+3D/w

    col_context_t();
    col_context_t(int mask);

    void init_locational(int ent0);
    void init_locational(int ent0, int ent1);
};

struct TraceExtents // sizeof=0x30
{                                       // XREF: moveclip_t/r
                                        // pointtrace_t/r ...
    hybrid_vector start;                // XREF: PM_playerTrace(pmove_t *,trace_t *,float const * const,float const * const,float const * const,float const * const,int,int)+1B7/w
                                        // PM_playerTrace(pmove_t *,trace_t *,float const * const,float const * const,float const * const,float const * const,int,int)+1C4/w ...
    hybrid_vector end;                  // XREF: PM_playerTrace(pmove_t *,trace_t *,float const * const,float const * const,float const * const,float const * const,int,int)+1DD/w
                                        // PM_playerTrace(pmove_t *,trace_t *,float const * const,float const * const,float const * const,float const * const,int,int)+1EA/w ...
    hybrid_vector invDelta;

    TraceExtents()
    {
        this->start.vec.u[0] = 0.0f;
        this->start.vec.u[1] = 0.0f;
        this->start.vec.u[2] = 0.0f;
        this->start.vec.u[3] = 0.0f;

        this->end.vec.u[0] = 0.0f;
        this->end.vec.u[1] = 0.0f;
        this->end.vec.u[2] = 0.0f;
        this->end.vec.u[3] = 0.0f;

        this->invDelta.vec.u[0] = 0.0f;
        this->invDelta.vec.u[1] = 0.0f;
        this->invDelta.vec.u[2] = 0.0f;
        this->invDelta.vec.u[3] = 0.0f;
    }
};

struct pointtrace_t // sizeof=0x40
{                                       // XREF: Flame_Server_Trace/r
                                        // Flame_Client_Trace/r ...
    TraceExtents extents;               // XREF: CG_TracePoint+390/w
                                        // CG_TracePoint+39D/w ...
    const IgnoreEntParams *ignoreEntParams; // XREF: CG_TracePoint+407/w
                                        // DynEntCl_DynEntImpactEvent(int,int,float const * const,float const * const,int,bool)+284/w ...
    int contentmask;                    // XREF: CG_TracePoint+40D/w
                                        // DynEntCl_DynEntImpactEvent(int,int,float const * const,float const * const,int,bool)+28B/w ...
    int bLocational;                    // XREF: CG_TracePoint+414/w
                                        // DynEntCl_DynEntImpactEvent(int,int,float const * const,float const * const,int,bool)+292/w ...
    unsigned __int8 *priorityMap;       // XREF: CG_TracePoint+417/w
                                        // DynEntCl_DynEntImpactEvent(int,int,float const * const,float const * const,int,bool)+299/w ...
};

enum TraceHitType : __int32
{                                                                             // XREF: vehicle_cache_t/r
                                                                                // trace_t/r ...
    TRACE_HITTYPE_NONE = 0x0,
    TRACE_HITTYPE_ENTITY = 0x1,
    TRACE_HITTYPE_DYNENT_MODEL = 0x2,
    TRACE_HITTYPE_DYNENT_BRUSH = 0x3,
    TRACE_HITTYPE_GLASS = 0x4,
};

struct TraceCheckCount // sizeof=0x8
{                                                                             // XREF: TraceThreadInfo/r
    int global;
    int *partitions;
};

struct TraceThreadInfo // sizeof=0x14
{                                                                             // XREF: .data:TraceThreadInfo * g_traceThreadInfo/r
    TraceCheckCount checkcount;
    struct cbrush_t *box_brush;                                // XREF: CM_Trace(trace_t *,float const * const,float const * const,float const * const,float const * const,uint,int,col_context_t &)+66E/r
    // CM_Trace(trace_t *,float const * const,float const * const,float const * const,float const * const,uint,int,col_context_t &)+94C/r ...
    struct cmodel_t *box_model;
    struct PhysGeomList **geoms;
};

struct trace_t // sizeof=0x38
{                                                                             // XREF: actor_physics_local_t/r
    hybrid_vector normal;                             // XREF: AimTarget_IsTargetVisible+1B/w
    // AimTarget_IsTargetVisible+28/w ...
    float fraction;                                         // XREF: AimTarget_IsTargetVisible+12B/r
    // AimTarget_IsTargetVisible+16B/r ...
    int sflags;                                                 // XREF: Mantle_Check(pmove_t *,pml_t *)+1EF/r
    // Mantle_Check(pmove_t *,pml_t *):loc_41E705/r ...
    int cflags;                                                 // XREF: CG_Laser_Add_Core+4C8/r
    // FireBulletPenetrate+113F/r ...
    TraceHitType hitType;                             // XREF: Mantle_CheckLedge+4AE/r
    // Weapon_Melee_internal:loc_6955C4/r ...
    unsigned __int16 hitId;                         // XREF: Bullet_FirePenetrate+541/r
    // Weapon_Melee_internal+139/r ...
    unsigned __int16 modelIndex;                // XREF: Weapon_Melee_internal+43F/r
    // Flame_Phys_Update_Items_PerStream(bool,int,flameGeneric_s * *,int)+A27/w ...
    unsigned __int16 partName;                    // XREF: Weapon_Melee_internal+43A/r
    // Flame_Phys_Update_Items_PerStream(bool,int,flameGeneric_s * *,int)+A30/w ...
    unsigned __int16 boneIndex;                 // XREF: FireBulletPenetrate+1133/r
    // Weapon_Melee_internal+400/r ...
    unsigned __int16 partGroup; // hitLocation_t ?                 // XREF: Weapon_Melee_internal+444/r
    // Flame_Phys_Update_Items_PerStream(bool,int,flameGeneric_s * *,int)+A39/w ...
    bool allsolid;                                            // XREF: is_not_penetrating+9B/r
    // PM_GroundTrace:loc_432679/r ...
    bool startsolid;                                        // XREF: Mantle_CheckLedge+1C8/r
    // Mantle_CheckLedge+254/r ...
    bool walkable;                                            // XREF: Mantle_CheckLedge:loc_41EFB2/r
    // PM_GroundTrace+3FD/r ...
// padding byte
// padding byte
// padding byte
    struct cStaticModel_s *staticModel;                // XREF: Phys_FindAndRenderBulletMesh(float const * const,float const * const,int,bool)+47/w
    // Phys_FindAndRenderBulletMesh(float const * const,float const * const,int,bool)+334/r ...
    int hitPartition;                                     // XREF: Mount_CheckLedge+10/w
    // BG_CheckProne(playerState_s const *,int,float const * const,float,float,float,float *,float *,bool,bool,bool,uchar,proneCheckType_t,float)+10/w ...

    trace_t()
    {
        memset(this, 0, sizeof(trace_t)); // Blops added this, might help
    }
};

struct cbrushside_t // sizeof=0xC
{                                                                             // XREF: CM_TraceThroughBrush/r
                                                                                // ?trace_sphere_through_brush@@YAXQBM0AAMMPBUcbrush_t@@QAMAAHPAUclipMap_t@@@Z/r
    cplane_s *plane;                                        // XREF: CM_TraceThroughBrush+659/w
    // trace_sphere_through_brush(float const * const,float const * const,float &,float,cbrush_t const *,float * const,int &,clipMap_t *)+5E4/w
    int cflags;                                                 // XREF: CM_TraceThroughBrush+608/w
    // trace_sphere_through_brush(float const * const,float const * const,float &,float,cbrush_t const *,float * const,int &,clipMap_t *)+58D/w
    int sflags;                                                 // XREF: CM_TraceThroughBrush+61E/w
    // trace_sphere_through_brush(float const * const,float const * const,float &,float,cbrush_t const *,float * const,int &,clipMap_t *)+5A6/w
};

struct __declspec(align(4)) cLeaf_s // sizeof=0x2C
{                                       // XREF: cmodel_t/r cLeaf_t/r
    unsigned __int16 firstCollAabbIndex;
    unsigned __int16 collAabbCount;
    int brushContents;                  // XREF: CM_InitBoxHull+3F/w
    int terrainContents;                // XREF: CM_InitBoxHull+49/w
    float mins[3];                      // XREF: CM_InitBoxHull+5B/w
    // CM_InitBoxHull+6B/w ...
    float maxs[3];                      // XREF: CM_InitBoxHull+8B/w
    // CM_InitBoxHull+9B/w ...
    int leafBrushNode;                  // XREF: CM_InitBoxHull+164/w
    __int16 cluster;
    // padding byte
    // padding byte
};

struct cmodel_t // sizeof=0x48
{                                       // XREF: .data:g_box_model/r
                                        // clipMap_t/r
    float mins[3];
    float maxs[3];
    float radius;
    cLeaf_s leaf;                       // XREF: CM_InitBoxHull+3F/w
    // CM_InitBoxHull+49/w ...
};

struct cLeafBrushNodeLeaf_t // sizeof=0x4
{                                       // XREF: cLeafBrushNodeData_t/r
    unsigned __int16 *brushes;
};

struct cLeafBrushNodeChildren_t // sizeof=0xC
{                                       // XREF: cLeafBrushNodeData_t/r
    float dist;
    float range;
    unsigned __int16 childOffset[2];
};

union cLeafBrushNodeData_t // sizeof=0xC
{                                       // XREF: cLeafBrushNode_s/r
    cLeafBrushNodeLeaf_t leaf;
    cLeafBrushNodeChildren_t children;
};

struct cLeafBrushNode_s // sizeof=0x14
{                                       // XREF: cLeafBrushNode_t/r
    unsigned __int8 axis;
    // padding byte
    __int16 leafBrushCount;
    int contents;
    cLeafBrushNodeData_t data;
};

struct leafList_s // sizeof=0x2C
{                                       // XREF: leafList_t/r
                                        // CM_PositionTest/r ...
    int count;                          // XREF: CM_BoxLeafnums(float const * const,float const * const,ushort *,int,int *)+54/w
    // CM_BoxLeafnums(float const * const,float const * const,ushort *,int,int *)+8B/r ...
    int maxcount;                       // XREF: CM_BoxLeafnums(float const * const,float const * const,ushort *,int,int *)+5E/w
    // CM_PositionTest+258/w ...
    int overflowed;                     // XREF: CM_BoxLeafnums(float const * const,float const * const,ushort *,int,int *)+6E/w
    // CM_PositionTest+278/w ...
    unsigned __int16 *list;             // XREF: CM_BoxLeafnums(float const * const,float const * const,ushort *,int,int *)+64/w
    // CM_PositionTest+268/w ...
    float bounds[2][3];                 // XREF: CM_BoxLeafnums(float const * const,float const * const,ushort *,int,int *)+D/w
    // CM_BoxLeafnums(float const * const,float const * const,ushort *,int,int *)+1A/w ...
    int lastLeaf;                       // XREF: CM_BoxLeafnums(float const * const,float const * const,ushort *,int,int *)+67/w
    // CM_BoxLeafnums(float const * const,float const * const,ushort *,int,int *)+86/r ...
};

struct __declspec(align(8)) cbrush_t // sizeof=0x60
{                                                                             // XREF: .data:g_box_brush/r
    float mins[3];
    int contents;
    float maxs[3];
    unsigned int numsides;
    cbrushside_t *sides;
    int axial_cflags[2][3];
    int axial_sflags[2][3];
    unsigned int numverts;
    float (*verts)[3];
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct moveclip_t // sizeof=0x60
{                                       // XREF: ?SV_TraceCapsule@@YAXPAUtrace_t@@QBM111AAUcol_context_t@@@Z/r
                                        // ?CG_TraceCapsule@@YAXPAUtrace_t@@QBM111HHAAUcol_context_t@@@Z/r ...
    float mins[3];                      // XREF: PM_playerTrace(pmove_t *,trace_t *,float const * const,float const * const,float const * const,float const * const,int,int)+203/w
                                        // PM_playerTrace(pmove_t *,trace_t *,float const * const,float const * const,float const * const,float const * const,int,int)+213/w ...
    float maxs[3];                      // XREF: PM_playerTrace(pmove_t *,trace_t *,float const * const,float const * const,float const * const,float const * const,int,int)+232/w
                                        // PM_playerTrace(pmove_t *,trace_t *,float const * const,float const * const,float const * const,float const * const,int,int)+242/w ...
    float outerSize[3];                 // XREF: CG_TraceCapsule(trace_t *,float const * const,float const * const,float const * const,float const * const,int,int,col_context_t &)+556/w
                                        // CG_TraceCapsule(trace_t *,float const * const,float const * const,float const * const,float const * const,int,int,col_context_t &)+56B/w ...
    TraceExtents extents;               // XREF: PM_playerTrace(pmove_t *,trace_t *,float const * const,float const * const,float const * const,float const * const,int,int)+1A8/o
                                        // PM_playerTrace(pmove_t *,trace_t *,float const * const,float const * const,float const * const,float const * const,int,int)+1B7/w ...
    int passEntityNum;                  // XREF: CG_TraceCapsule(trace_t *,float const * const,float const * const,float const * const,float const * const,int,int,col_context_t &)+53E/w
                                        // SV_TraceCapsule(trace_t *,float const * const,float const * const,float const * const,float const * const,col_context_t &)+51A/w
    int passOwnerNum;                   // XREF: CG_TraceCapsule(trace_t *,float const * const,float const * const,float const * const,float const * const,int,int,col_context_t &)+541/w
                                        // SV_TraceCapsule(trace_t *,float const * const,float const * const,float const * const,float const * const,col_context_t &)+66D/w
    int contentmask;                    // XREF: PM_playerTrace(pmove_t *,trace_t *,float const * const,float const * const,float const * const,float const * const,int,int)+257/w
                                        // CG_TraceCapsule(trace_t *,float const * const,float const * const,float const * const,float const * const,int,int,col_context_t &)+538/w ...
};

struct cNode_t // sizeof=0x8
{
    cplane_s *plane;
    __int16 children[2];
};

struct clipMap_t // sizeof=0x14C
{                                       // XREF: .data:clipMap_t cm/r
    const char *name;                   // XREF: CM_LoadMapData_LoadObj+10/r
                                        // CM_Shutdown(void)+4/r ...
    int isInUse;                        // XREF: DynEnt_ClearCollWorld(DynEntityCollType)+7/r
                                        // CM_LoadMap(char const *,int *)+33/w ...
    int planeCount;                     // XREF: CMod_LoadPlanes(void)+6B/w
                                        // CM_GetPlaneCount(void)+3/r
    cplane_s *planes;                   // XREF: CMod_LoadPlanes(void)+63/w
                                        // CMod_LoadPlanes(void)+70/r ...
    unsigned int numStaticModels;       // XREF: CM_LoadStaticModels(void):loc_6B39D9/w
                                        // CM_LoadStaticModels(void)+499/r ...
    struct cStaticModel_s *staticModelList;    // XREF: CM_LoadStaticModels(void)+53/w
                                        // CM_LoadStaticModels(void)+159/w ...
    unsigned int numMaterials;          // XREF: CMod_LoadMaterials+63/w
                                        // CMod_LoadMaterials+98/r ...
    struct dmaterial_t *materials;             // XREF: CMod_LoadMaterials+5B/w
                                        // CMod_LoadMaterials+74/r ...
    unsigned int numBrushSides;         // XREF: CMod_LoadBrushes+BA/w
    struct cbrushside_t *brushsides;           // XREF: CMod_LoadBrushes+B1/w
                                        // CMod_LoadBrushes+BF/r
    unsigned int numNodes;              // XREF: CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *):loc_53EAB4/r
                                        // CMod_LoadNodes+52/w ...
    cNode_t *nodes;                     // XREF: CMod_LoadNodes+4A/w
                                        // CMod_LoadNodes+57/r ...
    unsigned int numLeafs;              // XREF: CM_LeafCluster(int)+6/r
                                        // CM_LeafCluster(int)+E/r ...
    struct cLeaf_s *leafs;                     // XREF: CM_LeafCluster(int)+40/r
                                        // CMod_LoadLeafs+4A/w ...
    unsigned int leafbrushNodesCount;   // XREF: CMod_LoadBrushRelated+CA/w
                                        // CMod_LoadBrushRelated+D6/r
    struct cLeafBrushNode_s *leafbrushNodes;   // XREF: G_DebugDrawBrushModel(gentity_s *,float const * const,int,int)+EF/r
                                        // ScriptEntCmd_PhysicsLaunch(scr_entref_t)+197/r ...
    unsigned int numLeafBrushes;        // XREF: CMod_LoadLeafBrushes+3E/w
                                        // CM_InitBoxHull+175/r ...
    unsigned __int16 *leafbrushes;      // XREF: CMod_LoadLeafBrushNodes+C4/r
                                        // CMod_LoadLeafBrushNodes+10D/r ...
    unsigned int numLeafSurfaces;       // XREF: CMod_LoadLeafSurfaces+3D/w
    unsigned int *leafsurfaces;         // XREF: CMod_LoadLeafSurfaces+35/w
                                        // CMod_LoadLeafSurfaces+42/r
    unsigned int vertCount;             // XREF: CMod_LoadCollisionVerts+3D/w
    float (*verts)[3];                  // XREF: CMod_LoadCollisionVerts+35/w
                                        // CMod_LoadCollisionVerts+42/r ...
    unsigned int numBrushVerts;         // XREF: CMod_LoadBrushes+153/w
    float (*brushVerts)[3];             // XREF: CMod_LoadBrushes+134/w
                                        // CMod_LoadBrushes+141/r ...
    unsigned int nuinds;                // XREF: CMod_LoadPartitionIndices+3C/w
    unsigned __int16 *uinds;            // XREF: CMod_LoadPartitionIndices+34/w
                                        // CMod_LoadPartitionIndices+41/r ...
    int triCount;                       // XREF: CMod_LoadCollisionTriangles+45/w
                                        // CMod_LoadCollisionPartitions+D7/r
    unsigned __int16 *triIndices;       // XREF: CMod_LoadCollisionTriangles+34/w
                                        // CMod_LoadCollisionTriangles+4A/r ...
    unsigned __int8 *triEdgeIsWalkable; // XREF: CMod_LoadCollisionEdgeWalkable+2C/w
                                        // CMod_LoadCollisionEdgeWalkable+39/r ...
    int borderCount;                    // XREF: CMod_LoadCollisionBorders+3D/w
    struct CollisionBorder *borders;           // XREF: CMod_LoadCollisionBorders+35/w
                                        // CMod_LoadCollisionBorders+42/r ...
    int partitionCount;                 // XREF: CM_InitThreadData+4E/r
                                        // CMod_LoadCollisionPartitions+3D/w ...
    struct CollisionPartition *partitions;     // XREF: CMod_LoadCollisionPartitions+35/w
                                        // CMod_LoadCollisionPartitions+42/r ...
    int aabbTreeCount;                  // XREF: CMod_LoadCollisionAabbTrees+3D/w
    struct CollisionAabbTree *aabbTrees;       // XREF: CMod_GetLeafTerrainContents+37/r
                                        // CMod_LoadCollisionAabbTrees+35/w ...
    unsigned int numSubModels;          // XREF: DynEnt_GetSurfaceType(DynEntityDef const *)+40/r
                                        // DynEnt_Create+254/r ...
    cmodel_t *cmodels;                  // XREF: DynEnt_Create+28D/r
                                        // CMod_LoadSubmodels+4A/w ...
    unsigned __int16 numBrushes;        // XREF: CG_SightTracePoint(int *,float const * const,float const * const,int,trace_t *)+93/r
                                        // CMod_LoadBrushes+F6/w ...
    // padding byte
    // padding byte
    cbrush_t *brushes;                  // XREF: CG_SightTracePoint(int *,float const * const,float const * const,int,trace_t *)+A5/r
                                        // G_DebugDrawBrush_r+77/r ...
    int numClusters;                    // XREF: CMod_LoadVisibility+1F/r
                                        // CMod_LoadVisibility+34/w ...
    int clusterBytes;                   // XREF: CMod_LoadVisibility+2E/w
                                        // CMod_LoadVisibility+45/r ...
    unsigned __int8 *visibility;        // XREF: CMod_LoadVisibility+54/w
                                        // CMod_LoadVisibility+64/r ...
    int vised;                          // XREF: CMod_LoadVisibility:loc_6AA4A8/w
                                        // CM_ClusterPVS(int)+3/r
    struct MapEnts *mapEnts;                   // XREF: CMod_LoadEntityString+11/w
                                        // CM_EntityString(void)+3/r ...
    cbrush_t *box_brush;                // XREF: CM_InitThreadData+78/r
                                        // CM_InitBoxHull+14/w ...
    cmodel_t box_model;                 // XREF: CM_InitThreadData+A8/o
                                        // CM_InitBoxHull+3F/w ...
    unsigned __int16 originalDynEntCount;
                                        // XREF: DynEntCl_Shutdown(int)+2D/r
                                        // DynEntCl_Shutdown(int):loc_5AFE5F/r ...
    unsigned __int16 dynEntCount[4];    // XREF: DynEnt_GetEntityDef(ushort,DynEntityDrawType)+A/r
                                        // DynEnt_GetEntityDef(ushort,DynEntityDrawType)+19/r ...
    // padding byte
    // padding byte
    struct DynEntityDef *dynEntDefList[2];     // XREF: DynEnt_GetEntityDef(ushort,DynEntityDrawType)+4F/r
    struct DynEntityPose *dynEntPoseList[2];   // XREF: DynEnt_LoadEntities(void)+6C/w
    struct DynEntityClient *dynEntClientList[2];
    struct DynEntityServer *dynEntServerList[2];
    struct DynEntityColl *dynEntCollList[4];   // XREF: DynEnt_GetEntityColl(DynEntityCollType,ushort)+79/r
                                        // DynEnt_LoadEntities(void)+94/w ...
    int num_constraints;                // XREF: DynEnt_FixupLightConstraints(int,int,int)+1B/r
                                        // CreateRopes(int)+52/r ...
    struct PhysConstraint *constraints;        // XREF: DynEnt_DestroyPhysics(PhysWorld,DynEntityDef const *,DynEntityClient *)+69/r
                                        // DynEnt_FixupLightConstraints(int,int,int)+30/r ...
    int max_ropes;                      // XREF: DynEnt_LoadEntities(void):loc_5B99EE/w
                                        // DynEnt_LoadEntities(void):loc_5B9B80/r ...
    struct rope_t *ropes;                      // XREF: DynEnt_LoadEntities(void)+1147/w
                                        // Rope_Reset(void)+10/r
    unsigned int checksum;              // XREF: CM_LoadMap(char const *,int *)+40/r
};

struct traceWork_t // sizeof=0xE0
{                                       // XREF: ?CM_Trace@@YAXPAUtrace_t@@QBM111IHAAUcol_context_t@@@Z/r
                                        // ?CM_BoxSightTrace@@YAHHQBM000IH@Z/r ...
    hybrid_vector delta;                // XREF: CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+2AF/w
                                        // CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+2C1/w ...
    hybrid_vector midpoint;             // XREF: CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+251/w
                                        // CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+277/w ...
    hybrid_vector halfDelta;            // XREF: CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+2EB/w
                                        // CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+303/w ...
    hybrid_vector halfDeltaAbs;         // XREF: CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+384/w
                                        // CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+3CD/w ...
    hybrid_vector size;                 // XREF: CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+1BF/w
                                        // CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+1CF/w ...
    hybrid_vector bounds[2];            // XREF: CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+482/o
                                        // CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+499/o ...
    hybrid_vector radiusOffset;         // XREF: CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+32B/w
                                        // CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+338/w ...
    TraceExtents extents;               // XREF: CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+1EE/w
                                        // CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+1FB/w ...
    float deltaLen;                     // XREF: CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+47D/w
                                        // CM_Trace(trace_t *,float const * const,float const * const,float const * const,float const * const,uint,int,col_context_t &)+368/w ...
    float deltaLenSq;                   // XREF: CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+463/w
                                        // CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+468/r ...
    int contents;                       // XREF: CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+1B4/w
                                        // CM_Trace(trace_t *,float const * const,float const * const,float const * const,float const * const,uint,int,col_context_t &)+170/w ...
    bool isPoint;                       // XREF: CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+4B4/w
                                        // CM_Trace(trace_t *,float const * const,float const * const,float const * const,float const * const,uint,int,col_context_t &)+64C/w ...
    bool axialCullOnly;                 // XREF: CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+4B0/w
                                        // CM_SightTracePoint(int,float const * const,float const * const,col_context_t &):loc_6BE4CF/w
    bool doublesided;                   // XREF: CM_Trace(trace_t *,float const * const,float const * const,float const * const,float const * const,uint,int,col_context_t &)+658/w
                                        // CM_Trace(trace_t *,float const * const,float const * const,float const * const,float const * const,uint,int,col_context_t &)+8C2/w ...
    // padding byte
    float radius;                       // XREF: CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+4C0/w
                                        // CM_Trace(trace_t *,float const * const,float const * const,float const * const,float const * const,uint,int,col_context_t &)+3C6/w ...
    float offsetZ;                      // XREF: CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+4DA/w
                                        // CM_Trace(trace_t *,float const * const,float const * const,float const * const,float const * const,uint,int,col_context_t &)+3F3/w ...
    float boundingRadius;               // XREF: CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+4CD/w
                                        // CM_Trace(trace_t *,float const * const,float const * const,float const * const,float const * const,uint,int,col_context_t &)+3DA/w ...
    TraceThreadInfo threadInfo;         // XREF: CG_SightTracePointInternal(int *,float const * const,float const * const,int,trace_t *)+4DF/o
                                        // CM_Trace(trace_t *,float const * const,float const * const,float const * const,float const * const,uint,int,col_context_t &)+5C0/o ...

    traceWork_t()
    {
        this->delta.vec.u[0] = 0.0f;
        this->delta.vec.u[1] = 0.0f;
        this->delta.vec.u[2] = 0.0f;
        this->delta.vec.u[3] = 0.0f;
        this->midpoint.vec.u[0] = 0.0f;
        this->midpoint.vec.u[1] = 0.0f;
        this->midpoint.vec.u[2] = 0.0f;
        this->midpoint.vec.u[3] = 0.0f;
        this->halfDelta.vec.u[0] = 0.0f;
        this->halfDelta.vec.u[1] = 0.0f;
        this->halfDelta.vec.u[2] = 0.0f;
        this->halfDelta.vec.u[3] = 0.0f;
        this->halfDeltaAbs.vec.u[0] = 0.0f;
        this->halfDeltaAbs.vec.u[1] = 0.0f;
        this->halfDeltaAbs.vec.u[2] = 0.0f;
        this->halfDeltaAbs.vec.u[3] = 0.0f;
        this->size.vec.u[0] = 0.0f;
        this->size.vec.u[1] = 0.0f;

        this->size.vec.u[2] = 0.0f;
        this->size.vec.u[3] = 0.0f;

        bounds[0].vec.u[0] = 0.0f;
        bounds[0].vec.u[1] = 0.0f;
        bounds[0].vec.u[2] = 0.0f;
        bounds[0].vec.u[3] = 0.0f;


        bounds[1].vec.u[0] = 0.0f;
        bounds[1].vec.u[1] = 0.0f;
        bounds[1].vec.u[2] = 0.0f;
        bounds[1].vec.u[3] = 0.0f;

        this->radiusOffset.vec.u[0] = 0.0f;
        this->radiusOffset.vec.u[1] = 0.0f;
        this->radiusOffset.vec.u[2] = 0.0f;
        this->radiusOffset.vec.u[3] = 0.0f;

        ////TraceExtents::TraceExtents(&this->extents);
    }
};

struct BulletTraceResults // sizeof=0x50
{                                                                             // XREF: Bullet_FireExtended/r
    trace_t trace;                                            // XREF: BulletTrace_Start(int,BulletFireParams *,uint,centity_s const *,float * const,bool,bool)+1C/w
    // BulletTrace_Start(int,BulletFireParams *,uint,centity_s const *,float * const,bool,bool)+29/w ...
    struct gentity_s *hitEnt;                                    // XREF: Bullet_FireExtended:loc_5F3D1A/r
    // Bullet_FireExtended+174/r ...
    float hitPos[3];                                        // XREF: FireBulletPenetrate+DE5/o
    // FireBulletPenetrate+10E4/o ...
    bool ignoreHitEnt;                                    // XREF: Bullet_FireExtended+186/r
    // padding byte
    // padding byte
    // padding byte
    int depthSurfaceType;                             // XREF: FireBulletPenetrate:loc_52B4A6/r
};

unsigned __int16 __cdecl Trace_GetEntityHitId(const trace_t *trace);
unsigned __int16 __cdecl Trace_GetDynEntHitId(const trace_t *trace, DynEntityDrawType *drawType);
int __cdecl Trace_GetGlassHitId(const trace_t *trace);
void __cdecl CM_GetTraceThreadInfo(TraceThreadInfo *threadInfo);
clipMap_t *__cdecl CM_GetClipMap();
unsigned int __cdecl CM_TempBoxModel(const float *mins, const float *maxs, int contents);
void __cdecl CM_GetBox(cbrush_t **box_brush, cmodel_t **box_model, PhysGeomList ***geoms);
unsigned int __cdecl CM_TempBrushModel(struct PhysGeomList *geoms);
bool __cdecl CM_ClipHandleIsValid(unsigned int handle);
cmodel_t *__cdecl CM_ClipHandleToModel(unsigned int handle);
int __cdecl CM_ContentsOfModel(unsigned int handle);
void __cdecl CM_Trace(
                trace_t *results,
                const float *start,
                const float *end,
                const float *mins,
                const float *maxs,
                unsigned int model,
                int brushmask,
                col_context_t *context);
void __cdecl CM_TestInLeaf(traceWork_t *tw, struct cLeaf_s *leaf, trace_t *trace);
bool __cdecl CM_TestInLeafBrushNode(traceWork_t *tw, struct cLeaf_s *leaf, trace_t *trace);
void __cdecl CM_TestInLeafBrushNode_r(const traceWork_t *tw, struct cLeafBrushNode_s *node, trace_t *trace);
void __cdecl CM_TestBoxInBrush(const traceWork_t *tw, const cbrush_t *brush, trace_t *trace);
void __cdecl CM_TestCapsuleInCapsule(const traceWork_t *tw, trace_t *trace);
void __cdecl CM_PositionTest(traceWork_t *tw, trace_t *trace, col_context_t *context);
void __cdecl CM_TraceThroughLeaf(const traceWork_t *tw, struct cLeaf_s *leaf, trace_t *trace);
bool __cdecl CM_TraceThroughLeafBrushNode(const traceWork_t *tw, struct cLeaf_s *leaf, trace_t *trace);
void __cdecl CM_TraceThroughLeafBrushNode_r(
                const traceWork_t *tw,
    struct cLeafBrushNode_s *node,
                const float *p1_,
                const float *p2,
                trace_t *trace);
void __cdecl CM_TraceThroughBrush(const traceWork_t *tw, const cbrush_t *brush, trace_t *trace);
void __cdecl CM_TestInTempBrush(const traceWork_t *tw, trace_t *trace);
void __cdecl CM_TraceThroughTempBrush(const traceWork_t *tw, trace_t *trace);
void __cdecl CM_TraceCapsuleThroughCapsule(const traceWork_t *tw, trace_t *trace);
int __cdecl CM_TraceSphereThroughSphere(
                const traceWork_t *tw,
                const float *vStart,
                const float *vEnd,
                const float *vStationary,
                float radius,
                trace_t *trace);
int __cdecl CM_TraceCylinderThroughCylinder(
                const traceWork_t *tw,
                const float *vStationary,
                float fStationaryHalfHeight,
                float radius,
                trace_t *trace);
void __cdecl CM_TraceThroughTree(const traceWork_t *tw, int num, const float *p1_, const float *p2, trace_t *trace);
void __cdecl CM_TraceThroughPrimitives(
                const traceWork_t *tw,
                const float *p1_,
                const float *p2,
                trace_t *trace,
                col_context_t *context);
void __cdecl CM_SetAxialCullOnly(traceWork_t *tw);
void __cdecl CM_BoxTrace(
                trace_t *results,
                const float *start,
                const float *end,
                const float *mins,
                const float *maxs,
                int brushmask,
                col_context_t *context);
void __cdecl CM_TransformedBoxTraceRotated(
                trace_t *results,
                const float *start,
                const float *end,
                const float *mins,
                const float *maxs,
                unsigned int model,
                int brushmask,
                const float *origin,
                const float (*matrix)[3]);
void __cdecl TransposeMatrix(const float (*matrix)[3], float (*transpose)[3]);
void __cdecl RotatePoint(float *point, const float (*mat)[3]);
void __cdecl CM_TransformedBoxTrace(
                trace_t *results,
                const float *start,
                const float *end,
                const float *mins,
                const float *maxs,
                unsigned int model,
                int brushmask,
                const float *origin,
                const float *angles);
void __cdecl CM_TransformedBoxTraceExternal(
                trace_t *results,
                const float *start,
                const float *end,
                const float *mins,
                const float *maxs,
                unsigned int model,
                int brushmask,
                const float *origin,
                const float *angles);
int __cdecl CM_SightTraceThroughBrush(const traceWork_t *tw, const cbrush_t *brush, int brushNum, trace_t *trace);
int __cdecl CM_SightTracePointThroughBrush(const TraceExtents *extents, const cbrush_t *brush);
int __cdecl CM_SightTraceThroughTree(const traceWork_t *tw, int num, const float *p1_, const float *p2, trace_t *trace);
int __cdecl CM_SightTraceThroughLeaf(const traceWork_t *tw, const cLeaf_s *leaf, trace_t *trace);
int __cdecl CM_SightTraceThroughLeafBrushNode(const traceWork_t *tw, const cLeaf_s *leaf, trace_t *trace);
int __cdecl CM_SightTraceThroughLeafBrushNode_r(
                const traceWork_t *tw,
    struct cLeafBrushNode_s *remoteNode,
                const float *p1_,
                const float *p2,
                trace_t *trace);
int __cdecl CM_BoxSightTrace(
                int oldHitNum,
                const float *start,
                const float *end,
                const float *mins,
                const float *maxs,
                unsigned int model,
                int brushmask);
int __cdecl CM_SightTraceThroughTempBrush(const traceWork_t *tw, trace_t *trace);
int __cdecl CM_SightTraceCapsuleThroughCapsule(const traceWork_t *tw, trace_t *trace);
bool __cdecl CM_SightTraceSphereThroughSphere(
                const traceWork_t *tw,
                const float *vStart,
                const float *vEnd,
                const float *vStationary,
                float radius,
                trace_t *trace);
bool __cdecl CM_SightTraceCylinderThroughCylinder(
                const traceWork_t *tw,
                const float *vStationary,
                float fStationaryHalfHeight,
                float radius,
                trace_t *trace);
int __cdecl CM_SightTracePoint(int oldHitNum, const float *start, const float *end, col_context_t *context);
int __cdecl CM_TransformedBoxSightTrace(
                int hitNum,
                const float *start,
                const float *end,
                const float *mins,
                const float *maxs,
                unsigned int model,
                int brushmask,
                const float *origin,
                const float *angles);
int __cdecl CM_TracePointDown(
                const float *start,
                const float *end,
                int contentmask,
                int surf_type,
                float *endpos,
                float *fraction,
                float *normal);
double __cdecl CM_GetWaterHeight(const float *pos, float z_up, float z_down);
void __cdecl calc_closest(const float *v0, const float *v1, const float *v2, const float *p, float *res);
bool __cdecl is_inside(const float *a, const float *b, const float *n, const float *p);
bool __cdecl point_in_triangle(const float *p, const float *v0, const float *v1, const float *v2, const float *n);
char __cdecl trace_sphere_through_triangle(
                const float *c0,
                const float *dir,
                float *t,
                float r,
                const float *v0,
                const float *v1,
                const float *v2,
                const float *n);
void __cdecl trace_sphere_through_partition(
                const float *c,
                const float *dir,
                float *t,
                float r,
                const CollisionAabbTree *tree,
                float *hitn,
                int *sflags);
void __cdecl trace_sphere_through_brush(
                const float *c0,
                const float *dir,
                float *t,
                float r,
                const cbrush_t *brush,
                float *hitn,
                int *sflags);
void __cdecl trace_sphere_through_brush(
                const float *c0,
                const float *dir,
                float *t,
                float r,
                const cbrush_t *brush,
                float *hitn,
                int *sflags,
                clipMap_t *pClipMap = NULL); // this arg seems unused, which is confusing
void __cdecl trace_point_through_brush(
                const float *c0,
                const float *dir,
                float *t,
                const cbrush_t *brush,
                float *hitn,
                int *sflags);
char __cdecl trace_point_through_triangle(
                const float *p0,
                const float *u0,
                const float *v0,
                const float *v1,
                const float *v2,
                float cur_t,
                float *t);
void __cdecl trace_sphere_through_partition(
                const float *p,
                const float *dir,
                float *t,
                float r,
                const CollisionPartition *partition,
                float *hitn,
                int *hint,
                clipMap_t *pClipMap);
void __cdecl trace_point_through_partition(
                const float *p,
                const float *dir,
                float *t,
                const CollisionPartition *partition,
                float *hitn,
                int *hint,
                clipMap_t *pClipMap);
void __cdecl trace_point_vs_env(
                trace_t *trace,
                const float *start,
                const float *end,
                col_context_t *context,
                int *index_hint,
                clipMap_t *pClipMap);
void __cdecl trace_point_vs_env(
                trace_t *trace,
                const float *start,
                const float *end,
                col_context_t *context,
                int *index_hint);
void __cdecl trace_point_vs_env(
                trace_t *trace,
                const float *start,
                const float *end,
                col_context_t *context,
                int *index_hint,
                unsigned __int8 *pClipMap);
void __cdecl trace_sphere_vs_env(
                trace_t *trace,
                const float *start,
                const float *end,
                float radius,
                col_context_t *context,
                int *index_hint,
                clipMap_t *pClipMap);
void __cdecl trace_sphere_vs_env(
                trace_t *trace,
                const float *start,
                const float *end,
                float radius,
                col_context_t *context,
                int *index_hint,
                unsigned __int8 *pClipMap);
int __cdecl collide_segment_brush(const float *p0, const float *p1, const cbrush_t *brush);
bool __cdecl collide_segment_triangle(
                const float *p0,
                const float *u0,
                const float *v0,
                const float *v1,
                const float *v2);
int __cdecl collide_segment(const float *p0, const float *p1, col_context_t *context);
char __cdecl CM_TestPointInBrushModel(const float *point, unsigned int brushmodel, const float *trans);
char __cdecl CM_GetWaterForce(const float *pt, float *dir, float *force);
char __cdecl CM_GetHeliHeight(const float *pt, float checkdist, float *result);


static const float traceOffsets_0[5][2] = { { 0.0, 0.0 }, { 1.0, 1.0 }, { -1.0, 1.0 }, { 1.0, -1.0 }, { -1.0, -1.0 } };