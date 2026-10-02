#pragma once

#include <tl/jobqueue/jobqueue_all.h>
#include <gfx_d3d/r_material.h>
#include <gfx_d3d/r_marks.h>
#include <gfx_d3d/fxprimitives.h>


struct FxElemDef;
struct FxSpatialFrame;
struct FxCamera;

struct __declspec(align(4)) FxMark // sizeof=0x5C
{                                       // XREF: FxMarksSystem/r
    unsigned __int16 prevMark;
    unsigned __int16 nextMark;
    int frameCountDrawn;
    int frameCountAlloced;
    int ageMsec;
    float origin[3];
    float radius;
    float texCoordAxis[3];
    float hitNormal[3];
    unsigned __int8 nativeColor[4];
    Material *material;
    int tris;
    int points;
    FxMarkAlphaFade alphaFade;
    GfxMarkContext context;
    unsigned __int16 pointCount;
    unsigned __int8 triCount;
    unsigned __int8 markFlags;
    // padding byte
    // padding byte
};

struct FxTriGroup // sizeof=0x18
{                                       // XREF: FxTriGroupPool/r
    unsigned __int16 indices[2][3];
    GfxMarkContext context;
    unsigned __int8 triCount;
    unsigned __int8 unused[1];
    int next;
};

union FxTriGroupPool // sizeof=0x18
{                                       // XREF: FxMarksSystem/r
    FxTriGroupPool *nextFreeTriGroup;
    FxTriGroup triGroup;
};

struct FxMarkPoint // sizeof=0x20
{                                       // XREF: .data:g_fxMarkPoints/r
                                        // FxPointGroup/r
    float xyz[3];
    float lmapCoord[2];
    float normal[3];
};

struct FxPointGroup // sizeof=0x44
{                                       // XREF: FxPointGroupPool/r
    FxMarkPoint points[2];
    int next;
};

union FxPointGroupPool // sizeof=0x44
{                                       // XREF: FxMarksSystem/r
    FxPointGroupPool *nextFreePointGroup;
    FxPointGroup pointGroup;
};

struct FxMarksSystem // sizeof=0x26028
{
    int frameCount;
    unsigned __int16 firstFreeMarkHandle;
    unsigned __int16 firstActiveWorldMarkHandle;
    unsigned __int16 entFirstMarkHandles[1536];
    FxTriGroupPool *firstFreeTriGroup;
    FxPointGroupPool *firstFreePointGroup;
    FxMark marks[256];
    FxTriGroupPool triGroups[1024];
    FxPointGroupPool pointGroups[1536];
    bool noMarks;
    bool hasCarryIndex;
    unsigned __int16 carryIndex;
    unsigned int allocedMarkCount;
    unsigned int freedMarkCount;
    unsigned int numVisibleWorldSeethru;
    unsigned int numVisibleEntBrushSeethru;
    int frameTime;
};

struct FxMarkDObjUpdateContext // sizeof=0x108
{                                       // XREF: ?CG_Corpse@@YAXHPAUcentity_s@@@Z/r
                                        // ?CG_UpdatePlayerDObj@@YAXHPAUcentity_s@@@Z/r ...
    XModel *models[32];
    const char *modelParentBones[32];
    int modelCount;
    bool isBrush;
    // padding byte
    unsigned __int16 brushIndex;
};

struct FxActiveMarkSurf // sizeof=0x24
{                                       // XREF: FX_GenerateMarkVertsForList_EntXModel/r
                                        // FX_GenerateMarkVertsForList_EntDObj/r ...
    Material *material;
    GfxMarkContext context;
    // padding byte
    // padding byte
    int indexCount;
    unsigned __int16 *indices;
    float markHitNormal[3];
    unsigned int visLightsMask;
};

struct FxAddMarkCmd // sizeof=0x2C
{
    int localClientNum;
    const FxElemDef *elemDef;
    FxSpatialFrame spatialFrame;
    int randomSeed;
    unsigned int markEntnum;
};

bool __cdecl FX_MarkIsAlphaFadedOut(const FxMark *mark);
unsigned __int8 __cdecl FX_MarkGetFadedAlpha(const FxMark *mark);
void __cdecl FX_InitMarksSystem(FxMarksSystem *marksSystem);
unsigned __int16 __cdecl FX_MarkToHandle(FxMarksSystem *marksSystem, FxMark *mark);
void __cdecl FX_ClearMarks(unsigned int localClientNum);
FxMarksSystem *__cdecl FX_GetMarksSystem(unsigned int clientIndex);
void __cdecl FX_FreeMark(FxMarksSystem *marksSystem, FxMark *mark);
void __cdecl FX_FreeMarkFromList(FxMarksSystem *marksSystem, FxMark *mark, unsigned __int16 *listHead);
FxMark *__cdecl FX_MarkFromHandle(FxMarksSystem *marksSystem, unsigned __int16 handle);
void __cdecl FX_FreeMarkTriGroups(FxMarksSystem *marksSystem, FxMark *mark);
FxTriGroupPool *__cdecl FX_TriGroupFromHandle(FxMarksSystem *marksSystem, unsigned int handle);
void __cdecl FX_FreeMarkPointGroups(FxMarksSystem *marksSystem, FxMark *mark);
FxPointGroupPool *__cdecl FX_PointGroupFromHandle(FxMarksSystem *marksSystem, unsigned int handle);
char __cdecl FX_SeeThruMarkOverlaps(
                FxMarksSystem *marksSystem,
                const float *origin,
                float radius,
                const float *hitNormal);
void __cdecl FX_BeginMarks(unsigned int clientIndex);
void __cdecl FX_CreateImpactMark(
                int localClientNum,
                const FxElemDef *elemDef,
                const FxSpatialFrame *spatialFrame,
                int randomSeed,
                unsigned int markEntnum);
void __cdecl FX_CreateImpactMarkInternal(
                int localClientNum,
                const FxElemDef *elemDef,
                const FxSpatialFrame *spatialFrame,
                int randomSeed,
                unsigned int markEntnum);
void __cdecl FX_ImpactMark(
                int localClientNum,
                Material *worldMaterial,
                Material *modelMaterial,
                const float *origin,
                const float *quat,
                float orientation,
                const unsigned __int8 *nativeColor,
                float radius,
                unsigned int markEntnum,
                const FxMarkAlphaFade *markAlpha);
void __cdecl FX_ImpactMark_Generate(
                int localClientNum,
                MarkFragmentsAgainstEnum markAgainst,
                Material *material,
                const float *origin,
                const float (*axis)[3],
                float orientation,
                const unsigned __int8 *nativeColor,
                float radius,
                unsigned int markEntnum,
                const FxMarkAlphaFade *markAlpha,
                bool isSeeThruDecal);
FxMarkPoint *__cdecl FX_GetFxMarkPointsBuffer();
void __cdecl FX_ImpactMark_Generate_AddEntityBrush(
                int localClientNum,
                MarkInfo *markInfo,
                unsigned int entityIndex,
                const float *origin,
                float radius);
void __cdecl FX_ImpactMark_Generate_AddEntityModel(
                int localClientNum,
                MarkInfo *markInfo,
                unsigned int entityIndex,
                const float *origin,
                float radius);
void __cdecl FX_ImpactMark_Generate_Callback(
                void *context,
                int triCount,
                FxMarkTri *tris,
                int pointCount,
                FxMarkPoint *points,
                const float *markOrigin,
                const float *markTexCoordAxis,
                const float *markHitNormal,
                bool isSeeThruDecal);
void __cdecl FX_AllocAndConstructMark(
                unsigned int localClientNum,
                int triCount,
                int pointCount,
                Material *material,
                FxMarkTri *markTris,
                const FxMarkPoint *markPoints,
                const float *origin,
                const float *hitNormal,
                float radius,
                const float *texCoordAxis,
                const unsigned __int8 *nativeColor,
                const FxMarkAlphaFade *markAlpha,
                bool isSeeThruDecal);
void __cdecl FX_FreeLruMark(FxMarksSystem *marksSystem);
int __cdecl FX_AllocMarkTris(FxMarksSystem *marksSystem, const FxMarkTri *markTris, int triCount);
bool __cdecl FX_MarkContextsEqual(const GfxMarkContext *contextA, const GfxMarkContext *contextB);
int __cdecl FX_TriGroupToHandle(FxMarksSystem *marksSystem, FxTriGroup *group);
int __cdecl FX_AllocMarkPoints(FxMarksSystem *marksSystem, int pointCount);
int __cdecl FX_PointGroupToHandle(FxMarksSystem *marksSystem, FxPointGroup *group);
void __cdecl FX_LinkMarkIntoList(FxMarksSystem *marksSystem, unsigned __int16 *head, FxMark *mark);
void __cdecl FX_CopyMarkTris(
                FxMarksSystem *marksSystem,
                const FxMarkTri *srcTris,
                unsigned int dstGroupHandle,
                int triCount);
void __cdecl FX_CopyMarkPoints(
                FxMarksSystem *marksSystem,
                const FxMarkPoint *srcPoints,
                unsigned int dstGroupHandle,
                int pointCount);
unsigned __int16 __cdecl FX_FindModelHead(FxMarksSystem *marksSystem, unsigned __int16 modelIndex, int type);
int __cdecl FX_MarkContextsCompare(const GfxMarkContext *context0, const GfxMarkContext *context1);
int __cdecl fx_add_markCallback(jqBatch *batch);
void __cdecl FX_MarkEntDetachAll(unsigned int localClientNum, unsigned int entnum);
void __cdecl FX_MarkGlassDetachAll(unsigned int localClientNum, int glassIndex);
void __cdecl FX_MarkEntUpdateHidePartBits(
                const unsigned int *oldHidePartBits,
                const unsigned int *newHidePartBits,
                unsigned int localClientNum,
                unsigned int entnum);
void __cdecl FX_MarkEntDetachMatchingBones(
                FxMarksSystem *marksSystem,
                unsigned int entnum,
                const unsigned int *unsetHidePartBits);
void __cdecl FX_MarkEntUpdateBegin(
                FxMarkDObjUpdateContext *context,
                DObj *obj,
                bool isBrush,
                unsigned __int16 brushIndex);
void __cdecl FX_MarkEntUpdateEnd(
                FxMarkDObjUpdateContext *context,
                unsigned int localClientNum,
                unsigned int entnum,
                DObj *obj,
                bool isBrush,
                unsigned __int16 brushIndex);
void __cdecl FX_MarkEntDetachAllOfType(unsigned int localClientNum, unsigned int entnum, int markType);
void __cdecl FX_MarkEntUpdateEndDObj(
                FxMarkDObjUpdateContext *context,
                unsigned int localClientNum,
                unsigned int entnum,
                DObj *obj);
void __cdecl FX_MarkEntDetachModel(FxMarksSystem *marksSystem, unsigned int entnum, int oldModelIndex);
void __cdecl FX_BeginGeneratingMarkVertsForEntModels(unsigned int localClientNum, unsigned int *indexCount);
void __cdecl FX_GenerateMarkVertsForEntXModel(
                int localClientNum,
                unsigned int entId,
                unsigned int *indexCount,
                unsigned __int16 lightHandle,
                unsigned __int8 reflectionProbeIndex,
                const GfxScaledPlacement *placement,
                const GfxLight *visibleLights,
                int visibleLightCount);
char __cdecl FX_GenerateMarkVertsForList_EntXModel(
                FxMarksSystem *marksSystem,
                unsigned __int16 head,
                const FxCamera *camera,
                unsigned int *indexCount,
                unsigned __int16 lightHandleOverride,
                unsigned __int8 reflectionProbeIndexOverride,
                const GfxScaledPlacement *placement,
                const GfxLight *visibleLights,
                int visibleLightCount);
char __cdecl FX_GenerateMarkVertsForMark_Begin(
                FxMarksSystem *marksSystem,
                FxMark *mark,
                unsigned int *indexCount,
                unsigned __int16 *outBaseVertex,
                FxActiveMarkSurf *outDrawSurf);
void __cdecl FX_DrawMarkTris(
                FxMarksSystem *marksSystem,
                const FxMark *mark,
                unsigned __int16 baseVertex,
                unsigned __int16 *indices,
                FxActiveMarkSurf *outSurf);
void __cdecl FX_EmitMarkTri(
                FxMarksSystem *marksSystem,
                const unsigned __int16 *indices,
                const GfxMarkContext *markContext,
                unsigned __int16 baseVertex,
                FxActiveMarkSurf *outSurf);
void __cdecl FX_AddMarkTrisToScene(const FxActiveMarkSurf *outSurf);
void __cdecl FX_GenerateMarkVertsForMark_SetLightHandle(
                FxActiveMarkSurf *drawSurf,
                unsigned __int16 lightHandleOverride);
void __cdecl FX_GenerateMarkVertsForMark_SetReflectionProbeIndex(
                FxActiveMarkSurf *drawSurf,
                unsigned __int8 reflectionProbeIndexOverride);
void __cdecl FX_GenerateMarkVertsForMark_FinishAnimated(
                FxMarksSystem *marksSystem,
                FxMark *mark,
                unsigned __int16 baseVertex,
                FxActiveMarkSurf *drawSurf,
                const float (*transform)[3],
                const GfxLight *visibleLights,
                int visibleLightCount);
void    FX_GenerateMarkVertsForMark_MatrixFromPlacement(
                const GfxPlacement *placement,
                const float *viewOffset,
                float (*outTransform)[3]);
void __cdecl ConvertQuatToSkelMat(const DObjAnimMat *mat, DObjSkelMat *skelMat);
void __cdecl DObjSkelMatToMatrix43(const DObjSkelMat *inSkelMat, float (*outMatrix)[3]);
void __cdecl FX_GenerateMarkVertsForEntDObj(
                int localClientNum,
                unsigned int entId,
                unsigned int *indexCount,
                unsigned __int16 lightHandle,
                unsigned __int8 reflectionProbeIndex,
                DObj *dobj,
                const cpose_t *pose,
                const GfxLight *visibleLights,
                int visibleLightCount);
char __cdecl FX_GenerateMarkVertsForList_EntDObj(
                FxMarksSystem *marksSystem,
                unsigned __int16 head,
                const FxCamera *camera,
                unsigned int *indexCount,
                unsigned __int16 lightHandleOverride,
                unsigned __int8 reflectionProbeIndexOverride,
                const DObj *dobj,
                const DObjAnimMat *boneMtxList,
                unsigned int *hidePartBits,
                const GfxLight *visibleLights,
                int visibleLightCount);
void    FX_GenerateMarkVertsForMark_MatrixFromAnim(
                const FxMark *mark,
                const DObj *dobj,
                const DObjAnimMat *boneMtxList,
                const float *viewOffset,
                float (*outTransform)[3]);
void __cdecl ConvertQuatToInverseSkelMat(const DObjAnimMat *mat, DObjSkelMat *skelMat);
void __cdecl FX_GenerateMarkVertsForEntBrush(
                int localClientNum,
                unsigned int entId,
                unsigned int *indexCount,
                unsigned __int8 reflectionProbeIndex,
                const GfxPlacement *placement,
                const GfxLight *visibleLights,
                int visibleLightCount);
char __cdecl FX_GenerateMarkVertsForList_EntBrush(
                FxMarksSystem *marksSystem,
                unsigned __int16 head,
                const FxCamera *camera,
                unsigned int *indexCount,
                const GfxPlacement *placement,
                unsigned __int8 reflectionProbeIndex,
                const GfxLight *visibleLights,
                int visibleLightCount);
void __cdecl FX_EndGeneratingMarkVertsForEntModels(unsigned int localClientNum);
void __cdecl FX_FinishGeneratingMarkVerts(FxMarksSystem *marksSystem);
void __cdecl FX_GenerateMarkVertsForStaticModels(
                int localClientNum,
                int smodelCount,
                const unsigned __int8 *smodelVisLods,
                const GfxLight *visibleLights,
                int visibleLightCount);
char __cdecl FX_GenerateMarkVertsForList_WorldXModel(
                FxMarksSystem *marksSystem,
                unsigned __int16 head,
                const FxCamera *camera,
                unsigned int *indexCount,
                const GfxLight *visibleLights,
                int visibleLightCount);
void __cdecl FX_GenerateMarkVertsForMark_FinishNonAnimated(
                FxMarksSystem *marksSystem,
                FxMark *mark,
                unsigned __int16 baseVertex,
                FxActiveMarkSurf *drawSurf,
                const GfxLight *visibleLights,
                int visibleLightCount);
void __cdecl FX_GenerateMarkVertsForWorld(int localClientNum, const GfxLight *visibleLights, int visibleLightCount);
char __cdecl FX_GenerateMarkVertsForList_WorldBrush(
                FxMarksSystem *marksSystem,
                unsigned __int16 head,
                const FxCamera *camera,
                unsigned int *indexCount,
                const GfxLight *visibleLights,
                int visibleLightCount);
void __cdecl FX_MarkUpdateClientTime(unsigned int localClientNum, int frameTime);
int __cdecl FX_GetFrameTotalSeeThruDecalCount(unsigned int localClientNum);

extern jqWorkerCmd fx_add_markWorkerCmd;

extern FxMarksSystem *fx_marksSystemPool;
extern int fx_maxLocalClients;
