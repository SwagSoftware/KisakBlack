#pragma once

#include <xanim/xmodel.h>
#include "r_gfx.h"

#define BOXSIDE_FRONT 1
#define BOXSIDE_BACK 2

static const float cull_thresh = -100.0f;
static const float dist_thresh2 = 6.4e7f;


enum DpvsClipChildren : __int32
{                                       // XREF: R_VisitPortals/r
                                        // R_VisitPortalsForCell/r
    DPVS_DONT_CLIP_CHILDREN = 0x0,
    DPVS_CLIP_CHILDREN      = 0x1,
};

enum DpvsForceBevels : __int32
{                                       // XREF: R_PortalClipPlanes/r
                                        // R_AddBevelPlanes/r
    DPVS_DONT_FORCE_BEVELS = 0x0,
    DPVS_FORCE_BEVELS      = 0x1,
};

struct DpvsPlane // sizeof=0x14
{                                       // XREF: GfxPortal/r GfxScene/r ...
    float coeffs[4];                    // XREF: R_GenerateShadowMapCasterCells(void)+190/w
                                        // R_GenerateShadowMapCasterCells(void)+1A0/w ...
    unsigned __int8 side[3];            // XREF: R_SetupWorldSurfacesDpvs(GfxViewParms const *,uint)+304/r
                                        // R_SetupWorldSurfacesDpvs(GfxViewParms const *,uint)+420/r ...
    unsigned __int8 pad;
};

struct DpvsClipPlanes // sizeof=0x144
{                                       // XREF: R_AddAabbTreeSurfacesInFrustum_r/r
    DpvsPlane planes[16];
    unsigned int count;                 // XREF: R_AddAabbTreeSurfacesInFrustum_r+7D/w
};

struct GfxPortalWritable // sizeof=0xC
{                                       // XREF: GfxPortal/r
    bool isQueued;
    bool isAncestor;
    unsigned __int8 recursionDepth;
    unsigned __int8 hullPointCount;
    float (*hullPoints)[2];
    struct GfxPortal *queuedParent;
};

struct GfxPortal // sizeof=0x44
{
    GfxPortalWritable writable;
    DpvsPlane plane;
    struct GfxCell *cell;
    float (*vertices)[3];
    unsigned __int8 vertexCount;
    // padding byte
    // padding byte
    // padding byte
    float hullAxis[2][3];
};

struct DpvsGlob_sunShadow // sizeof=0x14
{                                       // XREF: DpvsGlob/r
    float viewDir[3];                   // XREF: R_SetupWorldSurfacesDpvs(GfxViewParms const *,uint)+163/w
                                        // R_SetupWorldSurfacesDpvs(GfxViewParms const *,uint)+176/w ...
    float viewDirDist;                  // XREF: R_SetupWorldSurfacesDpvs(GfxViewParms const *,uint)+1C9/w
    float sunShadowDrawDist;            // XREF: R_GetSceneExtentsAlongDir+15/r
                                        // R_GetSceneExtentsAlongDir+E6/r ...
};

struct DpvsView // sizeof=0x120
{                                       // XREF: DpvsGlob/r
    unsigned int renderFxFlagsCull;
    DpvsPlane frustumPlanes[14];        // XREF: R_CullDynamicSpotLightInCameraView(void)+11/o
                                        // R_CullDynamicPointLightsInCameraView(void)+11/o ...
    int frustumPlaneCount;              // XREF: R_CullDynamicSpotLightInCameraView(void)+25/r
};

struct PortalHeapNode // sizeof=0x8
{                                       // XREF: R_VisitPortalsNoFrustum/r
    GfxPortal *portal;
    float dist;
};

struct DpvsGlob // sizeof=0xC320
{                                       // XREF: .data:DpvsGlob dpvsGlob/r
    DpvsPlane nearPlane;                // XREF: R_GenerateShadowMapCasterCells(void)+190/w
                                        // R_GenerateShadowMapCasterCells(void)+1A0/w ...
    DpvsPlane farPlane;                 // XREF: R_ChopPortal+11E/o
                                        // R_SetupWorldSurfacesDpvs(GfxViewParms const *,uint)+357/w ...
    bool farPlaneEnabled;               // XREF: R_GenerateShadowMapCasterCells(void)+1C8/w
                                        // R_ChopPortal:loc_A44C0E/r ...
    // padding byte
    // padding byte
    // padding byte
    const GfxMatrix *viewProjMtx;       // XREF: R_SetupWorldSurfacesDpvs(GfxViewParms const *,uint)+C9/w
                                        // R_ProjectPortal+B9/r ...
    const GfxMatrix *invViewProjMtx;    // XREF: R_SetupWorldSurfacesDpvs(GfxViewParms const *,uint)+D8/w
                                        // R_UnprojectPoint+3/r ...
    const GfxMatrix *projMtx;           // XREF: R_SetupWorldSurfacesDpvs(GfxViewParms const *,uint)+E4/w
    float viewOrg[4];                   // XREF: R_GenerateShadowMapCasterCells(void)+127/w
                                        // R_GenerateShadowMapCasterCells(void)+13E/w ...
    int viewOrgIsDir;                   // XREF: R_GenerateShadowMapCasterCells(void)+16D/w
                                        // R_GetSidePlaneNormals:loc_A43D48/r ...
    DpvsGlob_sunShadow sunShadow;       // XREF: R_SetupWorldSurfacesDpvs(GfxViewParms const *,uint)+163/w
                                        // R_SetupWorldSurfacesDpvs(GfxViewParms const *,uint)+176/w ...
    int queuedCount;                    // XREF: R_VisitPortalsNoFrustum+D6/w
                                        // R_VisitPortalsNoFrustum:loc_A43756/r ...
    PortalHeapNode *portalQueue;        // XREF: R_VisitPortalsNoFrustum+D0/w
                                        // R_NextQueuedPortal:loc_A43FBC/r ...
    union GfxHullPointsPool *nextFreeHullPoints;
                                        // XREF: R_VisitPortalsNoFrustum+C5/w
                                        // R_FreeHullPoints+3B/r ...
    float cullDist;                     // XREF: R_GetFarPlaneDist(void):loc_A4AF45/r
                                        // R_SetCullDist(float)+19/w ...
    DpvsPlane childPlanes[2048];        // XREF: R_VisitPortalsNoFrustum:loc_A43A0C/o
                                        // R_VisitPortals+541/o
    DpvsView views[4][3];               // XREF: R_DrawAllSceneEnt(GfxViewInfo const *)+8E/o
                                        // R_FilterXModelIntoScene(XModel const *,GfxScaledPlacement const *,ushort,ushort *,float)+116/o ...
    DpvsPlane *sideFrustumPlanes;       // XREF: R_SetupWorldSurfacesDpvs(GfxViewParms const *,uint)+444/w
                                        // R_SetupShadowSurfacesDpvs(GfxViewParms const *,float const (* const)[4],uint,int)+2DA/r ...
    unsigned int *entVisBits[4];        // XREF: R_DrawAllSceneEnt(GfxViewInfo const *)+C/r
                                        // R_UnfilterEntFromCells(int,uint)+18F/r ...
    unsigned int *cellCasterBitsForCell;
                                        // XREF: R_GenerateShadowMapCasterCells(void)+21C/w
                                        // R_VisitPortalsForCellNoFrustum+6/r ...
    unsigned int cellVisibleBits[32];   // XREF: R_AddWorldSurfacesFrustumOnly(uint)+F3/r
                                        // R_AddWorldSurfacesPortalWalk+8C/o ...
    unsigned int cellForceInvisibleBits[32];
                                        // XREF: R_AddWorldSurfacesPortalWalk+255/o
                                        // R_AddWorldSurfacesPortalWalk+269/o ...
    // The original reserved 8 padding bytes here to 16-byte-align occluderPlanes
    // (the SSE occluder-cull path asserts (plane & 0xf) == 0 in TestOccludersPartial).
    // alignas(16) reproduces that: it pads this member to a 16-aligned offset AND
    // raises DpvsGlob's alignment to 16 so the `dpvsGlob` global itself is 16-aligned.
    alignas(16) float occluderPlanes[320][4];  // XREF: R_SetupWorldSurfacesDpvs(GfxViewParms const *,uint)+657/o
                                        // R_AddCellStaticSurfacesInFrustum+161/o ...
    int numOccluders;                   // XREF: R_SetupWorldSurfacesDpvs(GfxViewParms const *,uint)+44A/w
                                        // R_SetupWorldSurfacesDpvs(GfxViewParms const *,uint)+661/w ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct GfxViewInfo;
struct GfxWorldDraw;
struct GfxLightingInfo;
struct BModelDrawInfo;

union GfxEntCellRefInfo // sizeof=0x4
{                                       // XREF: R_FilterDObjIntoCells(int,uint,float * const,float)+10A/w
                                        // R_FilterBModelIntoCells(int,uint,GfxBrushModel *)+8D/w ...
    float radius;
    GfxBrushModel *bmodel;
};

struct FilterEntInfo // sizeof=0x10
{                                       // XREF: ?R_FilterDObjIntoCells@@YAXHIQAMM@Z/r
    int localClientNum;                 // XREF: R_FilterDObjIntoCells(int,uint,float * const,float)+FC/w
    unsigned int entnum;                // XREF: R_FilterDObjIntoCells(int,uint,float * const,float)+102/w
    GfxEntCellRefInfo info;             // XREF: R_FilterDObjIntoCells(int,uint,float * const,float)+10A/w
    unsigned int cellOffset;            // XREF: R_FilterDObjIntoCells(int,uint,float * const,float)+11D/w
};
struct mnode_t // sizeof=0x4
{
    unsigned __int16 cellIndex;
    unsigned __int16 rightChildOffset;
};
struct GfxAabbTree;
struct GfxPortal;

union GfxHullPointsPool // sizeof=0x200
{                                       // XREF: GfxHullPointsPoolArray/r
    GfxHullPointsPool *nextFree;
    float points[64][2];
};

struct GfxWorld;

struct DpvsDynamicCellCmd // sizeof=0xC
{                                       // XREF: R_AddCellSurfacesAndCullGroupsInFrustumDelayed/r
    const DpvsPlane *planes;            // XREF: R_AddCellSurfacesAndCullGroupsInFrustumDelayed+6A/w
    unsigned int cellIndex;             // XREF: R_AddCellSurfacesAndCullGroupsInFrustumDelayed+64/w
    unsigned __int16 viewIndex;         // XREF: R_AddCellSurfacesAndCullGroupsInFrustumDelayed+8F/w
    unsigned __int8 planeCount;         // XREF: R_AddCellSurfacesAndCullGroupsInFrustumDelayed+70/w
    unsigned __int8 frustumPlaneCount;  // XREF: R_AddCellSurfacesAndCullGroupsInFrustumDelayed+76/w
};



struct DpvsStaticCellCmd // sizeof=0xC
{                                       // XREF: R_AddCellSurfacesAndCullGroupsInFrustumDelayed/r
    const DpvsPlane *planes;            // XREF: R_AddCellSurfacesAndCullGroupsInFrustumDelayed+F/w
    //$FE217278708CFE8B64B0580037FD6900 ___u1;
    union //$FE217278708CFE8B64B0580037FD6900 // sizeof=0x4
    {                                       // XREF: R_AddCellSurfacesAndCullGroupsInFrustumDelayed+9/w
                                            // DpvsStaticCellCmd/r
        const GfxCell *cell;
        const GfxCell *cell_local;
    };
                                        // XREF: R_AddCellSurfacesAndCullGroupsInFrustumDelayed+9/w
    unsigned __int8 planeCount;         // XREF: R_AddCellSurfacesAndCullGroupsInFrustumDelayed+15/w
    unsigned __int8 frustumPlaneCount;  // XREF: R_AddCellSurfacesAndCullGroupsInFrustumDelayed+1B/w
    unsigned __int16 viewIndex;         // XREF: R_AddCellSurfacesAndCullGroupsInFrustumDelayed+35/w
};

struct GfxBrushModel;
struct GfxScaledPlacement;
union GfxDrawSurf;
struct GfxPlacement;
struct ShaderConstantSet;
struct GfxViewParms;
struct GfxCell;
struct GfxPortal;
struct GfxMatrix;
struct GfxLight;

enum DynEntityDrawType : __int32;

void __cdecl R_FrustumClipPlanes(
                const GfxMatrix *viewProjMtx,
                const float (*sidePlanes)[4],
                int sidePlaneCount,
                DpvsPlane *frustumPlanes);
void __cdecl R_SetDpvsPlaneSides(DpvsPlane *plane);
int __cdecl R_CellForPoint(const float *origin);
unsigned int __cdecl R_CalcReflectionProbeIndex(const float *origin);
unsigned int __cdecl R_FindNearestReflectionProbeInCell(
                const GfxWorldDraw *worldDraw,
                const GfxCell *cell,
                const float *origin);
unsigned int __cdecl R_FindNearestReflectionProbe(const GfxWorldDraw *worldDraw, const float *origin);
unsigned int __cdecl R_FindProbeFromVolume(const GfxWorldDraw *worldDraw, const float *origin);
void __cdecl R_AddAllSceneEntSurfacesCamera(const GfxViewInfo *viewInfo);
void __cdecl LaunchDobjLightingJobs(
                const GfxViewInfo *viewInfo,
                unsigned int sceneEntCount,
                unsigned __int8 *sceneEntVisData,
                GfxLightingInfo *lightingInfo);
void __cdecl FinishDobjLighting(
                unsigned int sceneEntCount,
                unsigned __int8 *sceneEntVisData,
                GfxLightingInfo *lightingInfo,
                GfxDrawSurf **drawSurfs,
                GfxDrawSurf **lastDrawSurfs,
                const GfxLight *visibleLights,
                int visibleLightCount,
                bool isMissileCamera);
void __cdecl LaunchModelLightingJobs(
                const GfxViewInfo *viewInfo,
                unsigned int sceneEntCount,
                unsigned __int8 *sceneEntVisData,
                GfxLightingInfo *lightingInfo);
void __cdecl FinishModelLighting(
                unsigned int sceneEntCount,
                unsigned __int8 *sceneEntVisData,
                GfxLightingInfo *lightingInfo,
                GfxDrawSurf **drawSurfs,
                GfxDrawSurf **lastDrawSurfs,
                const GfxLight *visibleLights,
                int visibleLightCount);
void __cdecl LaunchDynEntLightingJobs(
                const GfxViewInfo *viewInfo,
                unsigned int sceneEntCount,
                unsigned __int8 *sceneEntVisData,
                GfxLightingInfo *lightingInfo);
void __cdecl FinishDynEntLighting(
                unsigned int sceneEntCount,
                unsigned __int8 *sceneEntVisData,
                GfxLightingInfo *lightingInfo,
                GfxDrawSurf **drawSurfs,
                GfxDrawSurf **lastDrawSurfs,
                const GfxLight *visibleLights,
                int visibleLightCount);
void __cdecl R_AddAllSceneEntSurfacesSunShadow();
void __cdecl R_AddAllSceneEntSurfacesRangeSunShadow(unsigned int partitionIndex);
void __cdecl R_AddAllSceneEntSurfacesSpotShadow(
                const GfxViewInfo *viewInfo,
                unsigned int spotShadowIndex,
                unsigned int primaryLightIndex);
void __cdecl R_AddSceneDObj(unsigned int entnum, unsigned int viewIndex);
void __cdecl R_DrawAllSceneEnt(const GfxViewInfo *viewInfo);
int __cdecl R_DrawBModel(
                BModelDrawInfo *bmodelInfo,
                const GfxBrushModel *bmodel,
                const GfxPlacement *placement,
                const ShaderConstantSet *constSet);
void __cdecl R_DrawAllDynEnt(const GfxViewInfo *viewInfo);
void __cdecl R_UnfilterEntFromCells(int localClientNum, unsigned int entnum);
void __cdecl R_UnfilterDynEntFromCells(unsigned int dynEntId, DynEntityDrawType drawType);
void __cdecl R_FilterDObjIntoCells(unsigned int localClientNum, unsigned int entnum, float *origin, float radius);
void __cdecl R_FilterEntIntoCells_r(FilterEntInfo *entInfo, mnode_t *node, const float *mins, const float *maxs);
void __cdecl R_FilterBModelIntoCells(unsigned int localClientNum, unsigned int entnum, GfxBrushModel *bmodel);
void __cdecl R_FilterDynEntIntoCells(unsigned int dynEntId, DynEntityDrawType drawType, float *mins, float *maxs);
void __cdecl R_FilterDynEntIntoCells_r(
                mnode_t *node,
                unsigned int dynEntIndex,
                DynEntityDrawType drawType,
                const float *mins,
                const float *maxs);
void __cdecl R_FilterXModelIntoScene(
                const XModel *model,
                const GfxScaledPlacement *placement,
                unsigned __int16 renderFxFlags,
                unsigned __int16 *cachedLightingHandle,
                float lightToleranceSq);
int __cdecl R_CullSphereDpvs(const float *origin, float radius, const DpvsPlane *planes, int planeCount);
void __cdecl R_InitialEntityCulling();
float __cdecl R_DpvsPlaneMaxSignedDistToBox(const DpvsPlane *plane, const float *minmax);
void __cdecl R_GetStaticModels(const float *mins, float *maxs, int *models, int *models_count, int max_models);
void __cdecl R_GetStaticModelsAabb(
                const GfxAabbTree *tree,
                const float *mins,
                const float *maxs,
                int *models,
                int *models_count,
                int max_models);
void __cdecl R_AddCellDynBrushSurfacesInFrustumCmd(DpvsPlane **data);
void __cdecl R_CullDynBrushInCell(unsigned int cellIndex, DpvsPlane *planes, int planeCount);
unsigned int __cdecl R_SetVisData(unsigned int viewIndex);
void __cdecl R_GenerateShadowMapCasterCells();
void __cdecl R_VisitPortalsNoFrustum(const GfxCell *cell);
unsigned int __cdecl R_PortalClipPlanesNoFrustum(
                DpvsPlane *planes,
                unsigned int vertexCount,
                const float (*winding)[3]);
void __cdecl R_GetSidePlaneNormals(const float (*winding)[3], unsigned int vertexCount, float (*normals)[3]);
GfxPortal *R_NextQueuedPortal();
void R_AssertValidQueue();
void __cdecl R_FreeHullPoints(GfxHullPointsPool *hullPoints);
void __cdecl R_VisitPortalsForCellNoFrustum(
                const GfxCell *cell,
                struct GfxPortal *parentPortal,
                const DpvsPlane *parentPlane,
                const DpvsPlane *planes,
                int planeCount,
                signed int recursionDepth);
void __cdecl R_EnqueuePortal(GfxPortal *portal);
double __cdecl R_FurthestPointOnWinding(const float (*points)[3], int pointCount, const DpvsPlane *plane);
bool __cdecl R_ShouldSkipPortal(const GfxPortal *portal, const DpvsPlane *planes, int planeCount);
double __cdecl R_DpvsPlaneDistToEye(const DpvsPlane *plane);
char __cdecl R_PortalBehindAnyPlane(const GfxPortal *portal, const DpvsPlane *planes, int planeCount);
char __cdecl R_PortalBehindPlane(const GfxPortal *portal, const DpvsPlane *plane);
char __cdecl R_ChopPortalAndAddHullPointsNoFrustum(
                GfxPortal *portal,
                const DpvsPlane *parentPlane,
                const DpvsPlane *planes,
                int planeCount);
void __cdecl R_AddVertToPortalHullPoints(GfxPortal *portal, const float *v);
GfxHullPointsPool *__cdecl R_AllocHullPoints();
int __cdecl R_ChopPortal(
                const GfxPortal *portal,
                const DpvsPlane *parentPlane,
                const DpvsPlane *planes,
                int planeCount,
                float (*v)[128][3],
                const float (**finalVerts)[3]);
const float (*__cdecl R_ChopPortalWinding(
                const float (*vertsIn)[3],
                int *vertexCount,
                const DpvsPlane *plane,
                float (*vertsOut)[3]))[3];
void __cdecl R_SetAncestorListStatus(GfxPortal *portal, bool isAncestor);
void __cdecl R_AddWorldSurfacesFrustumOnly(unsigned int viewIndex);
void __cdecl R_AddCellSurfacesAndCullGroupsInFrustumDelayed(
                const GfxCell *cell,
                const DpvsPlane *planes,
                unsigned __int8 planeCount,
                unsigned __int8 frustumPlaneCount);
void __cdecl R_ShowCull(const float *viewOrigin);
void __cdecl R_InitSceneData(int localClientNum);
void __cdecl DynEntCl_InitFilter();
void __cdecl R_InitSceneBuffers();
void __cdecl R_ShutdownSceneBuffers();
void __cdecl R_ClearDpvsScene();
void __cdecl R_ClearDpvsScene_For_ExtraCam();
bool __cdecl R_CullDynamicSpotLightInCameraView();
void __cdecl R_CullDynamicPointLightsInCameraView();
void __cdecl PointFrom3Planes(float *a, float *b, float *c, float *r);
void __cdecl R_SetupWorldSurfacesDpvs(const GfxViewParms *viewParms, unsigned int renderCullFlags);
void __cdecl R_EnableOccluder(const char *name, bool enable);
void __cdecl R_SetViewFrustumPlanes(GfxViewInfo *viewInfo);
void __cdecl R_AddWorldSurfacesDpvs(const GfxViewParms *viewParms, int cameraCellIndex);
void __cdecl R_AddWorldSurfacesPortalWalk(int cameraCellIndex);
void __cdecl R_SetCellVisible(const GfxCell *cell);
unsigned int __cdecl R_CellIsForcedInvisible(const GfxCell *cell);
void __cdecl R_VisitPortals(const GfxCell *cell, const DpvsPlane *parentPlane, DpvsPlane *planes, int planeCount);
unsigned int __cdecl R_PortalClipPlanes(
                DpvsPlane *planes,
                unsigned int vertexCount,
                const float (*winding)[3],
                DpvsClipChildren *clipChildren);
double __cdecl R_NearestPointOnWinding(const float (*points)[3], int pointCount, const DpvsPlane *plane);
void __cdecl R_ProjectPortal(
                int vertexCount,
                const float (*winding)[3],
                float *mins,
                float *maxs,
                DpvsClipChildren *clipChildren);
char *__cdecl R_PortalAssertMsg();
unsigned int __cdecl R_AddBevelPlanes(
                DpvsPlane *planes,
                unsigned int vertexCount,
                const float (*windingNormals)[3],
                float *mins,
                float *maxs,
                DpvsForceBevels forceBevels);
void __cdecl R_UnprojectPoint(const float *projected, float *unprojected);
void __cdecl R_VisitPortalsForCell(
                const GfxCell *cell,
                GfxPortal *parentPortal,
                const DpvsPlane *parentPlane,
                DpvsPlane *planes,
                unsigned int maxPlaneCount,
                unsigned int planeCount,
                int frustumPlaneCount,
                signed int recursionDepth,
                DpvsClipChildren clipChildren);
char __cdecl R_ChopPortalAndAddHullPoints(
                GfxPortal *portal,
                const DpvsPlane *parentPlane,
                const DpvsPlane *planes,
                int planeCount);
void __cdecl R_VisitAllFurtherCells(
                const GfxCell *cell,
                const DpvsPlane *parentPlane,
                const DpvsPlane *planes,
                int planeCount,
                unsigned __int8 frustumPlaneCount);
int __cdecl R_GetFurtherCellList_r(
                const GfxCell *cell,
                const DpvsPlane *parentPlane,
                const DpvsPlane *planes,
                int planeCount,
                float (*v)[128][3],
                const GfxCell **list,
                int count);
char __cdecl R_IsCellInList(const GfxCell *cell, const GfxCell **list, int count);
int __cdecl R_AddCellToList(const GfxCell *cell, const GfxCell **list, int count);
void __cdecl R_SetupShadowSurfacesDpvs(
                const GfxViewParms *viewParms,
                const float (*sidePlanes)[4],
                unsigned int sidePlaneCount,
                int partitionIndex);
double __cdecl R_GetFarPlaneDist();
void __cdecl R_SetCullDist(float dist);
int __cdecl R_CullBoxCurDpvs_SceneSelect(const float *minmax, unsigned int viewIndex, unsigned int sceneIndex);
int __cdecl R_CullBoxCurDpvs(const float *minmax, unsigned int viewIndex);
int __cdecl R_CullPoint(int localClient, const float *p0, float cutoff);
int __cdecl R_CullPoint(const float *point, float cutoff);
int __cdecl R_CullLine(int localClient, const float *p0, const float *p1, float cutoff);
unsigned int __cdecl R_ExtraCam_SaveDpvsData(int localClientNum, unsigned __int8 *buffer, unsigned int bufferByteSize);
void __cdecl R_ExtraCam_RestoreDpvsData(int localClientNum, unsigned __int8 *buffer);
void __cdecl R_PerMap_DpvsGlobInit();


extern DpvsGlob dpvsGlob;
extern thread_local unsigned __int8 *g_smodelVisData;
extern thread_local unsigned __int8 *g_surfaceVisData;