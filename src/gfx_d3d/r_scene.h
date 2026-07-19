#pragma once
#include "r_dpvs.h"
#include "r_init.h"
#include "r_gfx.h"

#include <cgame_mp/cg_local_mp.h>
#include <DynEntity/DynEntity_gamestate.h>
#include <cgame/cg_visionsets.h>

struct GfxCmdBufInput;
struct GfxViewParms;

struct BModelDrawInfo // sizeof=0x2
{                                       // XREF: GfxSceneDynBrush/r
    unsigned __int16 surfId;
};

struct __declspec(align(4)) GfxSceneBrush // sizeof=0x2C
{                                       // XREF: GfxScene/r
    BModelDrawInfo info;
    unsigned __int16 entnum;
    const GfxBrushModel *bmodel;
    GfxPlacement placement;
    const ShaderConstantSet *brushConstantSet;
    unsigned __int8 reflectionProbeIndex;
    // padding byte
    // padding byte
    // padding byte
};

struct GfxVisibleLight // sizeof=0x2008
{                                       // XREF: GfxScene/r GfxScene/r
    int drawSurfCount;                  // XREF: R_GetPointLightPartitions(GfxLight *)+167/w
                                        // R_GetPointLightPartitions(GfxLight *)+194/w ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    GfxDrawSurf drawSurfs[1024];        // XREF: R_GetBspSpotLightSurfs+DA/o
};

struct GfxSceneDpvs // sizeof=0x38
{                                       // XREF: GfxScene/r
    int localClientNum;                 // XREF: R_DrawAllSceneEnt(GfxViewInfo const *)+7/r
                                        // R_DrawAllSceneEnt(GfxViewInfo const *)+4E/r ...
    unsigned __int8 *entVisData[7];     // XREF: R_AddSceneDObj(uint,uint)+37/r
                                        // R_DrawAllSceneEnt(GfxViewInfo const *)+161/r ...
    unsigned __int16 *sceneXModelIndex; // XREF: R_InitSceneBuffers(void)+9F/w
                                        // R_ShutdownSceneBuffers(void):loc_A46243/r ...
    unsigned __int16 *sceneDObjIndex;   // XREF: R_InitSceneBuffers(void)+B9/w
                                        // R_ShutdownSceneBuffers(void)+41/r ...
    union GfxEntCellRefInfo *entInfo[4];      // XREF: R_FilterEntIntoCells_r+246/r
                                        // R_ShowCull(float const * const)+310/r ...
};

struct GfxSkinnedXModelSurfs // sizeof=0x4
{                                       // XREF: GfxSceneEntityCull/r
    void *firstSurf;
};

struct GfxSceneEntityCull // sizeof=0x40
{                                       // XREF: GfxSceneEntity/r
    volatile unsigned int state;
    float mins[3];
    float maxs[3];
    char lods[32];
    GfxSkinnedXModelSurfs skinnedSurfs;
};

union GfxSceneEntityInfo // sizeof=0x4
{                                       // XREF: GfxSceneEntity/r
    cpose_t *pose;
    unsigned __int16 *cachedLightingHandle;
};

struct __declspec(align(4)) GfxSceneEntity // sizeof=0x84
{                                       // XREF: GfxScene/r
    float lightingOrigin[3];
    GfxScaledPlacement placement;
    GfxSceneEntityCull cull;
    unsigned __int16 gfxEntIndex;
    unsigned __int16 entnum;
    DObj *obj;
    GfxSceneEntityInfo info;
    float lightingOriginToleranceSq;
    unsigned __int16 gfxEntIndex2;
    unsigned __int8 reflectionProbeIndex;
    unsigned __int8 altXModelIndex;
    unsigned __int8 entShaderConstantSetIndex;
    unsigned __int8 entShaderConstantSetIndexExtraCam;
    // padding byte
    // padding byte
};

struct XModelDrawInfo // sizeof=0x4
{                                       // XREF: GfxSceneDynModel/r
                                        // GfxSceneModel/r
    unsigned __int16 lod;
    unsigned __int16 surfId;
};

struct __declspec(align(2)) GfxSceneModel // sizeof=0x4C
{                                       // XREF: GfxScene/r
    XModelDrawInfo info;
    const XModel *model;
    const DObj *obj;
    GfxScaledPlacement placement;
    unsigned __int16 gfxEntIndex;
    unsigned __int16 entnum;
    float radius;
    unsigned __int16 *cachedLightingHandle;
    float lightingOrigin[3];
    float lightingOriginToleranceSq;
    unsigned __int8 modelShaderConstantSetIndex;
    unsigned __int8 modelShaderConstantSetIndexExtraCam;
    unsigned __int8 reflectionProbeIndex;
    // padding byte
};

struct __declspec(align(2)) GfxSceneGlassBrush // sizeof=0x28
{                                       // XREF: GfxScene/r
    GfxPlacement placement;
    const GfxBrushModel *bmodel;
    Material *altStreamingMaterial;
    BModelDrawInfo info;
    unsigned __int8 reflectionProbeIndex;
    // padding byte
};

struct __declspec(align(64)) GfxScene // sizeof=0x1B0000
{                                       // XREF: .data:GfxScene scene/r
    GfxDrawSurf bspDrawSurfs[8192];
    GfxDrawSurf smodelDrawSurfsLight[8192]; // XREF: R_InitScene(void)+D/o
    GfxDrawSurf entDrawSurfsLight[8192]; // XREF: R_InitScene(void)+17/o
    GfxDrawSurf codemeshDrawSurfsLight[8192];
                                        // XREF: R_InitScene(void)+21/o
    GfxDrawSurf bspDrawSurfsDecal[512]; // XREF: R_InitScene(void)+2B/o
    GfxDrawSurf smodelDrawSurfsDecal[512]; // XREF: R_InitScene(void)+35/o
    GfxDrawSurf entDrawSurfsDecal[512]; // XREF: R_InitScene(void)+3F/o
    GfxDrawSurf bspDrawSurfsEmissive[8192];
                                        // XREF: R_InitScene(void)+67/o
    GfxDrawSurf smodelDrawSurfsEmissive[8192];
                                        // XREF: R_InitScene(void)+71/o
    GfxDrawSurf entDrawSurfsEmissive[8192];
                                        // XREF: R_InitScene(void)+7B/o
    GfxDrawSurf fxDrawSurfsEmissive[8192]; // XREF: R_InitScene(void)+85/o
    GfxDrawSurf fxDrawSurfsEmissiveAuto[8192];
                                        // XREF: R_InitScene(void)+8F/o
    GfxDrawSurf fxDrawSurfsEmissiveDecal[8192];
                                        // XREF: R_InitScene(void)+99/o
    GfxDrawSurf reflectedFxDrawSurfsEmissive[8192];
                                        // XREF: R_InitScene(void)+A3/o
    GfxDrawSurf reflectedFxDrawSurfsEmissiveAuto[8192];
                                        // XREF: R_InitScene(void)+AD/o
    GfxDrawSurf reflectedFxDrawSurfsEmissiveDecal[8192];
                                        // XREF: R_InitScene(void)+B7/o
    GfxDrawSurf bspSunShadowDrawSurfs0[4096];
                                        // XREF: R_InitScene(void)+C1/o
    GfxDrawSurf smodelSunShadowDrawSurfs0[4096];
                                        // XREF: R_InitScene(void)+CB/o
    GfxDrawSurf entSunShadowDrawSurfs0[4096];
                                        // XREF: R_InitScene(void)+D5/o
    GfxDrawSurf codemeshSunShadowDrawSurfs0[64];
                                        // XREF: R_InitScene(void)+DF/o
    GfxDrawSurf bspSunShadowDrawSurfs1[8192];
                                        // XREF: R_InitScene(void)+E9/o
    GfxDrawSurf smodelSunShadowDrawSurfs1[8192];
                                        // XREF: R_InitScene(void)+F3/o
    GfxDrawSurf entSunShadowDrawSurfs1[8192];
                                        // XREF: R_InitScene(void)+FD/o
    GfxDrawSurf codemeshSunShadowDrawSurfs1[64];
                                        // XREF: R_InitScene(void)+107/o
    GfxDrawSurf bspSpotShadowDrawSurfs0[256];
                                        // XREF: R_InitScene(void)+111/o
    GfxDrawSurf smodelSpotShadowDrawSurfs0[256];
                                        // XREF: R_InitScene(void)+11B/o
    GfxDrawSurf entSpotShadowDrawSurfs0[512];
                                        // XREF: R_InitScene(void)+125/o
    GfxDrawSurf bspSpotShadowDrawSurfs1[256];
                                        // XREF: R_InitScene(void)+12F/o
    GfxDrawSurf smodelSpotShadowDrawSurfs1[256];
                                        // XREF: R_InitScene(void)+139/o
    GfxDrawSurf entSpotShadowDrawSurfs1[512];
                                        // XREF: R_InitScene(void)+143/o
    GfxDrawSurf bspSpotShadowDrawSurfs2[256];
                                        // XREF: R_InitScene(void)+14D/o
    GfxDrawSurf smodelSpotShadowDrawSurfs2[256];
                                        // XREF: R_InitScene(void)+157/o
    GfxDrawSurf entSpotShadowDrawSurfs2[512];
                                        // XREF: R_InitScene(void)+161/o
    GfxDrawSurf bspSpotShadowDrawSurfs3[256];
                                        // XREF: R_InitScene(void)+16B/o
    GfxDrawSurf smodelSpotShadowDrawSurfs3[256];
                                        // XREF: R_InitScene(void)+175/o
    GfxDrawSurf entSpotShadowDrawSurfs3[512];
                                        // XREF: R_InitScene(void)+17F/o
    unsigned int shadowableLightIsUsed[32];
                                        // XREF: R_GenerateSortedDrawSurfs+949/w
                                        // R_GenerateSortedDrawSurfs+94E/w ...
    int maxDrawSurfCount[39];           // XREF: R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+20/r
                                        // R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+3B/r ...
    volatile unsigned int drawSurfCount[39];     // XREF: R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+96D/w
                                        // R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+9DE/w ...
    GfxDrawSurf *drawSurfs[39];         // XREF: R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+18/r
                                        // R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+26/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    GfxDrawSurf fxDrawSurfsLight[8192]; // XREF: R_InitScene(void)+49/o
    GfxDrawSurf fxDrawSurfsLightAuto[8192];
                                        // XREF: R_InitScene(void)+53/o
    GfxDrawSurf fxDrawSurfsLightDecal[8192];
                                        // XREF: R_InitScene(void)+5D/o
    GfxSceneDef def;                    // XREF: R_SetLodOrigin(refdef_s const *)+52/w
                                        // R_SetLodOrigin(refdef_s const *)+62/w ...
    int addedLightCount;                // XREF: R_CullDynamicSpotLightInCameraView(void)+2E/r
                                        // R_CullDynamicPointLightsInCameraView(void)+43/r ...
    GfxLight addedLight[32];            // XREF: R_CullDynamicSpotLightInCameraView(void):loc_A4682B/o
                                        // R_CullDynamicPointLightsInCameraView(void)+54/o ...
    bool isAddedLightCulled[32];        // XREF: R_CullDynamicSpotLightInCameraView(void)+72/w
                                        // R_CullDynamicSpotLightInCameraView(void)+77/r ...
    float dynamicSpotLightNearPlaneOffset;
                                        // XREF: R_AddSpotLightToScene(float const * const,float const (* const)[3],float,float,float,float)+630/w
                                        // R_AddSpotLightToScene(float const * const,float const (* const)[3],float,float,float,float)+63E/r ...
    float dynamicSpotLightPlanes[6][4]; // XREF: R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+4C7/o
                                        // R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+890/o ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    GfxVisibleLight visLight[4];        // XREF: R_GetPointLightPartitions(GfxLight *)+167/w
                                        // R_GetBspSpotLightSurfs+12C/w ...
    GfxVisibleLight visLightShadow[2];  // XREF: R_GenerateSortedDrawSurfs+153E/o
                                        // R_GetPointLightPartitions(GfxLight *)+194/w ...
    unsigned int *entOverflowedDrawBuf; // XREF: R_AddAllSceneEntSurfacesSpotShadow(GfxViewInfo const *,uint,uint)+27F/r
                                        // R_DrawAllSceneEnt(GfxViewInfo const *)+C4E/r ...
    volatile unsigned int sceneDObjCount;        // XREF: R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+B6/r
                                        // R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+CB/r ...
    GfxSceneEntity sceneDObj[1024];     // XREF: LaunchDobjLightingJobs+3D/o
                                        // FinishDobjLighting+4C/o ...
    unsigned __int8 sceneDObjVisData[7][1024];
                                        // XREF: R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+B1/o
                                        // R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+110/o ...
    //volatile int sceneModelCount;       // XREF: R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+D7/r
    volatile unsigned int sceneModelCount;       // XREF: R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+D7/r
                                        // R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+18B/r ...
    GfxSceneModel sceneModel[1024];     // XREF: LaunchModelLightingJobs+36/o
                                        // FinishModelLighting+51/o ...
    unsigned __int8 sceneModelVisData[7][1024];
                                        // XREF: R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+186/o
                                        // R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+1DC/o ...
    //volatile int sceneBrushCount;       // XREF: R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+3F2/r
    volatile unsigned int sceneBrushCount;       // XREF: R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+3F2/r
                                        // R_AddAllSceneEntSurfacesRangeSunShadow:loc_A3FE13/r ...
    GfxSceneBrush sceneBrush[1024];     // XREF: R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+436/o
                                        // R_AddAllSceneEntSurfacesRangeSunShadow+29D/o ...
    unsigned __int8 sceneBrushVisData[3][1024];
                                        // XREF: R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+3FB/o
                                        // R_AddAllSceneEntSurfacesRangeSunShadow+264/o ...
    //volatile int glassBrushCount;       // XREF: R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+60F/r
    volatile unsigned int glassBrushCount;       // XREF: R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+60F/r
                                        // R_DrawAllSceneEnt(GfxViewInfo const *):loc_A4106A/r ...
    GfxSceneGlassBrush glassBrush[1024];
                                        // XREF: R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+66B/o
                                        // R_DrawAllSceneEnt(GfxViewInfo const *)+CBA/o ...
    unsigned __int8 glassBrushVisData[1024];
                                        // XREF: R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+618/o
                                        // R_DrawAllSceneEnt(GfxViewInfo const *)+C93/o ...
    unsigned int sceneDynModelCount;    // XREF: R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+1AD/r
                                        // R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+24E/r ...
    unsigned int sceneDynBrushCount;    // XREF: R_AddAllSceneEntSurfacesCamera(GfxViewInfo const *)+751/r
                                        // R_AddAllSceneEntSurfacesRangeSunShadow:loc_A3FE8A/r ...
    DpvsPlane shadowFarPlane[2];        // XREF: R_SetupShadowSurfacesDpvs(GfxViewParms const *,float const (* const)[4],uint,int)+1F3/o
                                        // R_SetupShadowSurfacesDpvs(GfxViewParms const *,float const (* const)[4],uint,int)+281/o ...
    DpvsPlane shadowNearPlane[2];       // XREF: R_SetupShadowSurfacesDpvs(GfxViewParms const *,float const (* const)[4],uint,int)+115/o
                                        // R_SetupShadowSurfacesDpvs(GfxViewParms const *,float const (* const)[4],uint,int)+1A2/o ...
    GfxSceneDpvs dpvs;                  // XREF: R_AddSceneDObj(uint,uint)+37/r
                                        // R_DrawAllSceneEnt(GfxViewInfo const *)+7/r ...
    unsigned int dynSModelVisBitsCamera[64];
                                        // XREF: R_ClearSceneDynSModelVisBits+A/o
                                        // R_GenerateSortedDrawSurfs+16E6/o ...
    unsigned int dynSModelVisBitsSunShadow[64];
                                        // XREF: R_ClearSceneDynSModelVisBits+1E/o
                                        // R_GenerateSortedDrawSurfs+16ED/o ...
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
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct GfxSceneParms // sizeof=0x14E0
{                                       // XREF: ?R_RenderScene@@YAXPBUrefdef_s@@H@Z/r
                                        // ?R_RenderMissileCam@@YAXPBUrefdef_s@@H@Z/r
    int localClientNum;
    float blurRadius;
    float sunVisibility;
    GfxDepthOfField dof;
    GfxLightScale charPrimaryLightScale;
    GfxFilm film;
    GfxBloom bloom;
    GfxCompositeFx flameFx;
    GfxReviveFx reviveFx;
    GfxCompositeFx waterSheetingFx;
    GfxDoubleVision doubleVision;
    GfxGenericFilter genericFilter;
    GfxPoison poisonFx;
    GfxCompositeFx electrifiedFx;
    GfxCompositeFx transportedFx;
    GfxSaveScreenFx saveScreenFx;
    bool isRenderingFullScreen;
    bool playerTeleported;
    // padding byte
    // padding byte
    GfxViewport sceneViewport;
    GfxViewport displayViewport;
    GfxViewport scissorViewport;
    const GfxLight *primaryLights;
    float sceneX;
    float sceneY;
    float sceneW;
    float sceneH;
};

struct GfxSceneDynModel // sizeof=0x6
{
    XModelDrawInfo info;
    unsigned __int16 dynEntId;
};

struct GfxSceneDynBrush // sizeof=0x4
{
    BModelDrawInfo info;
    unsigned __int16 dynEntId;
};

struct SceneEntCmd // sizeof=0x4
{                                       // XREF: R_GenerateSortedDrawSurfs/r
    const GfxViewInfo *viewInfo;        // XREF: R_GenerateSortedDrawSurfs+14BE/w
};

struct GfxLodParms;
struct GfxCmdBufInput;
struct GfxSunShadowProjection;
struct GfxBackEndData;

GfxScene *__cdecl R_GetScene();
unsigned int __cdecl R_AllocSceneModel();
int __cdecl R_AllocTextureOverride(
                unsigned __int16 modelIndexMask,
                const GfxImage *img1,
                GfxImage *img2,
                int prevOverride);
int __cdecl R_AllocTextureOverride(
                const Material *material,
                unsigned __int16 modelIndexMask,
                const GfxImage *img1,
                GfxImage *img2,
                int prevOverride);
GfxBrushModel *__cdecl R_GetBrushModel(unsigned int modelIndex);
void __cdecl R_AddBrushModelToSceneFromAngles(
                const GfxBrushModel *bmodel,
                const float *origin,
                const float *angles,
                unsigned __int16 entnum,
                const ShaderConstantSet *constants);
unsigned int __cdecl R_AllocSceneBrush();
void __cdecl R_AddGlassBrushToScene(
                const GfxBrushModel *bmodel,
                const float *origin,
                const float *angles,
                Material *altStreamingMaterial);
void __cdecl R_AddDObjToScene(
                const DObj *obj,
                const cpose_t *pose,
                unsigned int entnum,
                unsigned int renderFxFlags,
                float *lightingOrigin,
                float materialTime,
                float materialTime2,
                float burnFraction,
                float wetness,
                char altXModel,
                int textureOverrideIndex,
                const ShaderConstantSet *dobjConstantSet,
                const ShaderConstantSet *dobjConstantSetExtraCam,
                float lightingOriginToleranceSq,
                float scale);
unsigned int __cdecl R_AllocSceneDObj();
struct GfxParticleCloud *__cdecl R_AddParticleCloudToScene(Material *material);
void __cdecl R_AddOmniLightToScene(const float *org, const float (*axis)[3], int radius, float r, float g, float b);
void __cdecl R_AddSpotLightToScene(const float *org, const float (*axis)[3], float radius, float r, float g, float b);
float __cdecl R_GetDefaultNearClip();
void __cdecl R_SetupViewProjectionMatrices(GfxViewParms *viewParms, bool offsetmatrix);
void __cdecl R_AddBModelSurfacesCamera(
                BModelDrawInfo *bmodelInfo,
                const GfxBrushModel *bmodel,
                GfxDrawSurf **drawSurfs,
                GfxDrawSurf **lastDrawSurfs,
                unsigned int reflectionProbeIndex,
                float burnAmt,
                float fadeAmt,
                char visLightsMask);
GfxDrawSurf *__cdecl R_AddBModelSurfaces(
                BModelDrawInfo *bmodelInfo,
                const GfxBrushModel *bmodel,
                unsigned __int8 techType,
                GfxDrawSurf *drawSurf,
                GfxDrawSurf *lastDrawSurf);
const MaterialTechnique *__cdecl Material_GetTechnique(const Material *material, unsigned __int8 techType);
MaterialTechniqueSet *__cdecl Material_GetTechniqueSet(const Material *material);
void __cdecl R_SetEnablePlayerShadowFlag(bool flag);
void __cdecl R_AddXModelSurfacesCamera(
                XModelDrawInfo *modelInfo,
                const XModel *model,
                float *origin,
                unsigned int gfxEntIndex,
                unsigned int lightingHandle,
                unsigned __int8 primaryLightIndex,
                int isShadowReceiver,
                int depthHack,
                GfxDrawSurf **drawSurfs,
                GfxDrawSurf **lastDrawSurfs,
                unsigned int reflectionProbeIndex,
                unsigned int constantSetIndex,
                char visLightsMask);
const XSurface *__cdecl R_GetXSurface(unsigned int *modelSurf, surfaceType_t surfType);
GfxDrawSurf *__cdecl R_AddXModelSurfaces(
                XModelDrawInfo *modelInfo,
                const XModel *model,
                unsigned __int8 techType,
                GfxDrawSurf *drawSurf,
                GfxDrawSurf *lastDrawSurf);
void __cdecl R_AddDObjSurfacesCamera(
                GfxSceneEntity *sceneEnt,
                unsigned __int16 lightingHandle,
                unsigned __int8 primaryLightIndex,
                GfxDrawSurf **drawSurfs,
                GfxDrawSurf **lastDrawSurfs,
                char visLightsMask,
                bool isMissileCamera);
GfxDrawSurf *__cdecl R_AddDObjSurfaces(
                GfxSceneEntity *sceneEnt,
                unsigned __int8 techType,
                GfxDrawSurf *drawSurf,
                GfxDrawSurf *lastDrawSurf);
void __cdecl R_SetCharacterPrimaryLightScaleInfo(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms);
bool __cdecl R_EndFencePending();
bool __cdecl R_DynamicFencePending();
void __cdecl R_SetEndTime(int endTime);
void __cdecl R_WaitEndTime();
void __cdecl R_SetSunConstants(GfxCmdBufInput *input);
void __cdecl R_SetInputCodeConstant(GfxCmdBufInput *input, unsigned int constant, float x, float y, float z, float w);
void __cdecl R_SetInputCodeConstantFromVec4(GfxCmdBufInput *input, unsigned int constant, const float *value);
void    R_SetHDRControlConstants(GfxCmdBufInput *input, const GfxViewInfo *viewInfo);
void __cdecl R_GenerateMarkVertsForDynamicModels(const GfxLight *visibleLights, int visibleLightCount);
void    R_SetSkyDynamicIntensity(const float *viewForward, GfxCmdBufInput *input);
void __cdecl R_InitScene();
void __cdecl R_ClearScene(int localClientNum);
void R_ClearSceneDynSModelVisBits();
void __cdecl R_ClearScene_For_ExtraCam(int localClientNum);
int __cdecl R_GetLocalClientNum();
void __cdecl R_SetLodOrigin(const refdef_s *refdef);
void __cdecl R_UpdateFrameFog(unsigned int localClientNum);
double __cdecl Lerp(float from, float to, float t);
void __cdecl R_SetViewParmsForScene(const refdef_s *refdef, GfxViewParms *viewParms);
void __cdecl R_SetupProjection(float tanHalfFovX, float tanHalfFovY, GfxViewParms *viewParms);
void R_UpdateFrameSun();
void __cdecl R_LerpDir(
                const float *dirBegin,
                const float *dirEnd,
                int beginLerpTime,
                int endLerpTime,
                int currTime,
                float *result);
void __cdecl R_UpdateLodParms(const refdef_s *refdef, GfxLodParms *lodParms);
void __cdecl R_RenderScene(refdef_s *refdef, int frameTime);
void __cdecl R_CheckShadowMapVolumes(const float *position);
void __cdecl R_UpdateExposureValue(refdef_s *refdef);
unsigned int __cdecl R_CheckExposureVolumes(float *position, float *feather);
void __cdecl R_GenerateSortedDrawSurfs(
                const GfxSceneParms *sceneParms,
                const GfxViewParms *viewParmsDpvs,
                const GfxViewParms *viewParmsDraw,
                int frameTime,
                const refdef_s *refdef,
                bool forMissileCamView);
bool __cdecl R_GetAllowShadowMaps();
bool __cdecl R_DynamicShadowType();
void __cdecl R_SetSceneComposition(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms, bool extraCam);
void __cdecl R_SetExposure(GfxViewInfo *viewInfo, const refdef_s *refdef);
void __cdecl R_SetDepthOfField(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms);
void __cdecl R_SetDoubleVision(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms);
void __cdecl R_SetWaterSheetingFx(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms);
void __cdecl R_SetFlameFx(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms);
void __cdecl R_SetElectrifiedFx(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms);
void __cdecl R_SetTransportedFx(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms);
void __cdecl R_SetReviveFx(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms);
void __cdecl R_SetGenericFilter(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms);
void __cdecl R_SetPoisonFx(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms);
void __cdecl R_SetFilmInfo(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms);
void __cdecl R_SetBloomInfo(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms);
void __cdecl R_SetFullSceneViewMesh(int viewInfoIndex, GfxViewInfo *viewInfo);
void __cdecl R_GenerateSortedSunShadowDrawSurfs(GfxViewInfo *viewInfo);
void __cdecl R_AddEmissiveSpotLight(GfxViewInfo *viewInfo);
int __cdecl R_GetVisibleDLights(GfxLight *visibleLights);
void __cdecl R_GetLightSurfs(const GfxLight *visibleLights, int visibleLightCount);
void __cdecl R_GetPointLightShadowSurfs(
                GfxViewInfo *viewInfo,
                GfxVisibleLight *visibleLights,
                const GfxLight *lights,
                int numLights);
void __cdecl R_SetSunShadowConstants(GfxCmdBufInput *input, const GfxSunShadowProjection *sunProj);
// local variable allocation has failed, the output may be wrong!
void    R_SetHeroLighting(GfxCmdBufInput *input, GfxViewInfo *viewInfo);
void __cdecl R_SetDLightsConstantsDefaults(GfxCmdBufInput *input);

// local variable allocation has failed, the output may be wrong!
void    R_SetDLightsConstants(
                GfxCmdBufInput *input,
                const GfxViewInfo *viewInfo,
                const GfxLight *visibleLights,
                int visibleLightCount);
void R_DrawCineWarning();
void __cdecl R_SplitDrawSurfacesPrimarySortKey(
                struct GfxDrawSurfListInfo *srcList,
                struct GfxDrawSurfListInfo *destList,
                int sortkeyID);
void __cdecl DrawLightDebug(const GfxViewInfo *viewInfo, const GfxLight *L, const float *debugColor);
void DrawOutdoorBoundsVolumeDebug();
void __cdecl DrawSunDirectionDebug(const float *viewOrg, const float *viewForward);
void R_SortAllCodeMeshSurfacesSunShadow();
void R_DrawFogParams();
void R_WaitForFXUpdateWorkerCmds();
void __cdecl R_FinishDecalAndEmissiveDrawSurfs(
                GfxViewInfo *viewInfo,
                const GfxViewParms *viewParmsDraw,
                GfxBackEndData *backendData,
                bool forMissileCamView);
unsigned __int8 __cdecl R_GetEmissiveTechnique(const GfxViewInfo *viewInfo, unsigned __int8 baseTech);
char __cdecl R_DoesDrawSurfListInfoNeedFloatz(GfxViewInfo *viewInfo, GfxDrawSurfListInfo *emissiveInfo);
void __cdecl R_SetSceneParms(const refdef_s *refdef, GfxSceneParms *sceneParms);
void __cdecl R_CorrectLodScale(const refdef_s *refdef);
void __cdecl R_RenderMissileCam(const refdef_s *refdef, int frameTime);
void __cdecl R_MissileCam_SetSceneParms(const refdef_s *refdef, GfxSceneParms *sceneParms);
void __cdecl R_LinkDObjEntity(int localClientNum, unsigned int entnum, float *origin, float radius);
void __cdecl R_LinkBModelEntity(int localClientNum, unsigned int entnum, GfxBrushModel *bmodel);
void __cdecl R_UnlinkEntity(int localClientNum, unsigned int entnum);
void __cdecl R_LinkDynEnt(unsigned int dynEntId, DynEntityDrawType drawType, float *mins, float *maxs);
void __cdecl R_UnlinkDynEnt(unsigned int dynEntId, DynEntityDrawType drawType);
void __cdecl R_PvsLock_GetViewOrigin(float *out);
void __cdecl R_PvsLock_GetViewAxis(float (*out)[3]);
void __cdecl ShodLodInfo(
                const float *origin,
                int lod,
                int numLods,
                float curDist,
                float lodDist,
                bool lodDistAutoGenerated);
void __cdecl R_PerMap_Init();
void __cdecl R_AddXModelDebugString(const float *origin, char *string);


extern GfxScene scene;