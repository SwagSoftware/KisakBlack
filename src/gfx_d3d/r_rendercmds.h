#pragma once

#include "r_bsp.h"

#include "r_scene.h"
#include "r_meshdata.h"

#include "r_shader_constant_set.h"

#define MAX_VISIBLE_DLIGHTS 4

struct GfxImage; 

enum GfxProjectionTypes : __int32
{                                       // XREF: GfxCmdProjectionSet/r
                                        // R_AddCmdProjectionSet/r
    GFX_PROJECTION_2D = 0x0,
    GFX_PROJECTION_3D = 0x1,
};

enum GfxRenderCommand : __int32
{                                       // XREF: R_GetCommandBuffer/r
    RC_END_OF_LIST                   = 0x0,
    RC_SET_CUSTOM_CONSTANT           = 0x1,
    RC_SET_MATERIAL_COLOR            = 0x2,
    RC_SAVE_SCREEN                   = 0x3,
    RC_SAVE_SCREEN_SECTION           = 0x4,
    RC_CLEAR_SCREEN                  = 0x5,
    RC_SET_VIEWPORT                  = 0x6,
    RC_SET_SCISSOR                   = 0x7,
    RC_RESOLVE_COMPOSITE             = 0x8,
    RC_PC_COPY_IMAGE_GEN_MIP         = 0x9,
    RC_FIRST_NONCRITICAL             = 0xA,
    RC_STRETCH_PIC                   = 0xA,
    RC_STRETCH_PIC_FLIP_ST           = 0xB,
    RC_STRETCH_PIC_ROTATE_XY         = 0xC,
    RC_STRETCH_PIC_ROTATE_ST         = 0xD,
    RC_STRETCH_RAW                   = 0xE,
    RC_DRAW_QUAD_PIC                 = 0xF,
    RC_DRAW_FULL_SCREEN_COLORED_QUAD = 0x10,
    RC_DRAW_TEXT_2D                  = 0x11,
    RC_DRAW_TEXT_3D                  = 0x12,
    RC_BLEND_SAVED_SCREEN_BLURRED    = 0x13,
    RC_BLEND_SAVED_SCREEN_FLASHED    = 0x14,
    RC_DRAW_POINTS                   = 0x15,
    RC_DRAW_LINES                    = 0x16,
    RC_DRAW_TRIANGLES                = 0x17,
    RC_DRAW_QUADLIST_2D              = 0x18,
    RC_DRAW_EMBLEM_LAYER             = 0x19,
    RC_STRETCH_COMPOSITE             = 0x1A,
    RC_PROJECTION_SET                = 0x1B,
    RC_DRAW_FRAMED                   = 0x1C,
    RC_COUNT                         = 0x1D,
};

enum $FA8CAEF21A4B336C234573FF5E09E0DB : __int32
{
    DRAWLIST_LIT                = 0x0,
    DRAWLIST_LIT_POST_RESOLVE   = 0x1,
    DRAWLIST_LIT_PUNCHTHROUGH   = 0x2,
    DRAWLIST_LIT_COUNT          = 0x3,
    DRAWLIST_DECAL              = 0x3,
    DRAWLIST_EMISSIVE           = 0x4,
    DRAWLIST_REFLECTED          = 0x5,
    DRAWLIST_CLOAKED            = 0x6,
    DRAWLIST_EMISSIVE_VIEWMODEL = 0x7,
    DRAWLIST_SUN_SHADOW_MAP0    = 0x8,
    DRAWLIST_SUN_SHADOW_MAP1    = 0x9,
    DRAWLIST_SPOT_SHADOW_MAP0   = 0xA,
    DRAWLIST_SPOT_SHADOW_MAP1   = 0xB,
    DRAWLIST_SPOT_SHADOW_MAP2   = 0xC,
    DRAWLIST_SPOT_SHADOW_MAP3   = 0xD,
    DRAWLIST_COUNT              = 0xE,
};

enum ShadowType : __int32
{                                       // XREF: GfxViewInfo/r
                                        // R_GenerateSortedDrawSurfs/r ...
    SHADOW_NONE = 0x0,
    SHADOW_MAP  = 0x1,
};

struct GfxSModelCachedVertex // sizeof=0x20
{                                       // XREF: GfxBackEndData/r
    float xyz[3];
    GfxColor color;
    PackedTexCoords texCoord;
    PackedUnitVec normal;
    PackedUnitVec tangent;
    PackedLightingCoords baseLighting;
};

struct FxCodeMeshData // sizeof=0x10
{                                       // XREF: GfxBackEndData/r
    unsigned int triCount;
    unsigned __int16 *indices;
    unsigned __int16 argOffset;
    unsigned __int16 argCount;
    unsigned int lightHandle;
};

struct GfxParticleCloud // sizeof=0x40
{                                       // XREF: GfxBackEndData/r
    GfxScaledPlacement placement;
    float endpos[3];
    GfxColor color;
    float radius[2];
    unsigned __int16 count;
    unsigned __int16 offset;
    unsigned __int8 gaussian;
    unsigned __int8 pad[3];
};

struct GfxModelLightingPatch // sizeof=0x4C
{                                       // XREF: GfxBackEndData/r
    unsigned __int16 modelLightingIndex;
    // padding byte
    // padding byte
    float primaryLightWeight;
    unsigned __int8 useHeroLighting;
    unsigned __int8 colorsCount;
    unsigned __int8 groundLighting[4];
    // padding byte
    // padding byte
    float colorsWeight[8];
    unsigned __int16 colorsIndex[8];
    float heroPos[3];
};

struct GfxBackEndPrimitiveData // sizeof=0x4
{                                       // XREF: GfxBackEndData/r
    int hasSunDirChanged;
};

struct GfxEntity // sizeof=0x18
{                                       // XREF: GfxBackEndData/r
                                        // localEntity_s/r
    unsigned int renderFxFlags;
    float materialTime;
    float destructibleBurnAmount;
    float destructibleFadeAmount;
    float wetness;
    int textureOverrideIndex;
};

struct GfxTextureOverride // sizeof=0x10
{                                       // XREF: GfxBackEndData/r
    const Material *material;
    const GfxImage *img1;
    const GfxImage *img2;
    unsigned __int16 dobjModelMask;
    __int16 prev;
};

struct FxMarkMeshData // sizeof=0x18
{                                       // XREF: GfxBackEndData/r
    unsigned int triCount;
    unsigned __int16 *indices;
    float normal[3];
    unsigned __int16 modelIndex;
    unsigned __int8 modelTypeAndSurf;
    unsigned __int8 pad0;
};

struct GfxCmdHeader // sizeof=0x4
{                                       // XREF: GfxCmdSaveScreen/r
                                        // GfxCmdStretchPicRotateXY/r ...
    unsigned __int16 byteCount;
    unsigned __int8 id;
    unsigned __int8 ui3d;
};

struct GfxCmdArray // sizeof=0x18
{                                       // XREF: .data:g_frontEndCmds/r
    unsigned __int8 *cmds;              // XREF: R_InitRenderCommands(void)+8F/w
    int usedTotal;
    int usedCritical;
    GfxCmdHeader *lastCmd;
    int byteSize;                       // XREF: R_InitRenderCommands(void)+63/w
    int warnSize;                       // XREF: R_InitRenderCommands(void)+72/w
};

struct GfxCullViewInfo // sizeof=0x170
{                                       // XREF: $29D6D5D109C236443008C80F9E8DF0DC/r
    GfxViewParms viewParms;
    GfxViewport sceneViewport;
    GfxViewport displayViewport;
    GfxViewport scissorViewport;
};

struct __declspec(align(4)) GfxDrawSurfListInfo // sizeof=0x50
{                                       // XREF: PointLightPartition/r
                                        // GfxViewInfo/r ...
    const GfxDrawSurf *drawSurfs;
    unsigned int drawSurfCount;
    unsigned __int8 baseTechType;       // XREF: R_ShowTris(GfxCmdBufContext,GfxDrawSurfListInfo const *)+A2/w
                                        // R_DrawCloakPrePassCallbackHDR+BD/w ...
    // padding byte
    // padding byte
    // padding byte
    const GfxViewInfo *viewInfo;
    unsigned int viewInfoIndex;
    float viewOrigin[4];
    const GfxLight *light;
    int cameraView;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    jqBatchGroup group[3];              // XREF: R_ShowTris(GfxCmdBufContext,GfxDrawSurfListInfo const *)+24/o
                                        // R_DrawCloakPrePassCallbackHDR+12/o ...
    int shaderHDRmode;
    bool isMissileCamera;
    // padding byte
    // padding byte
    // padding byte
};
static_assert(alignof(GfxDrawSurfListInfo) >= 8);

struct PointLightPartition // sizeof=0x1C0
{                                       // XREF: GfxViewInfo/r
    GfxLight light;
    GfxDrawSurfListInfo info;
};

struct GfxExposureShaderRemap // sizeof=0x18
{                                       // XREF: GfxViewInfo/r
    float remapMul[3];
    float remapAdd[3];
};

struct GfxViewRenderControl // sizeof=0x38
{                                       // XREF: GfxViewInfo/r
    GfxRenderTargetId mainSceneMSAA;
    GfxRenderTargetId mainScene;
    GfxRenderTargetId mainSceneFinal;
    GfxRenderTargetId mainSceneSaved;
    GfxRenderTargetId extraCam;
    GfxRenderTargetId ui3d;
    GfxRenderTargetId seeThruDecal;
    GfxRenderTargetId frameBuffer;
    float sceneWindowU0;
    float sceneWindowV0;
    float sceneWindowU1;
    float sceneWindowV1;
    float framebufferWindowY0;
    float framebufferWindowX1;
    float framebufferWindowY1;
    int opaqueShaderRemap;
    int alphaShaderRemap;
    int emissiveShaderRemap;
    unsigned int renderingMode;
    unsigned int additionalPostFX;
};
static_assert(sizeof(GfxViewRenderControl) == 0x38);

struct GfxUI3DBackend // sizeof=0xE0
{                                       // XREF: GfxViewInfo/r
    GfxViewport viewport[6];
    float uvSetup[6][4];
    int renderCmdCount[6];
    int totalRenderCmds;
    float blurRadius;
};

struct DynSModelClient // sizeof=0x14
{                                       // XREF: R_GenerateSortedDrawSurfs/r
    int localClientNum;                 // XREF: R_GenerateSortedDrawSurfs+16E3/w
    unsigned int *visBitsCamera;        // XREF: R_GenerateSortedDrawSurfs+16E6/w
    unsigned int *visBitsSunShadow;     // XREF: R_GenerateSortedDrawSurfs+16ED/w
    unsigned int numDynSModel;          // XREF: R_GenerateSortedDrawSurfs+1730/w
    unsigned int numDynSModelInMap;     // XREF: R_GenerateSortedDrawSurfs+173F/w
};

struct DynSModelClientView // sizeof=0x11F4
{                                       // XREF: .data:g_dynSModelClientView/r
    unsigned __int16 drawStateIndex[2048];
    float grassForceConstants[25][4];
    float forceFrequency[25];
};

struct DynSModelDrawState // sizeof=0xC
{                                       // XREF: DynSModelGfxState/r
    float squash;
    float windForceStrength;
    unsigned __int16 force0Index;
    unsigned __int16 force1Index;
};

struct DynSModelGfxState // sizeof=0x180C
{                                       // XREF: GfxBackEndData/r
    unsigned int maxDynSModelIndex;
    unsigned int maxDynSModelIndexInMap;
    volatile unsigned int drawStateCount;
    DynSModelDrawState drawState[512];
};

struct __declspec(align(16)) GfxViewInfo // sizeof=0x38E0
{                                       // XREF: .data:g_viewInfo/r
    //GfxViewInfo()
    //{
    // // some garbage for the jqBatchGroup?
    //}

    //$29D6D5D109C236443008C80F9E8DF0DC ___u0;
    union //$29D6D5D109C236443008C80F9E8DF0DC // sizeof=0x170
    {                                       // XREF: GfxViewInfo/r
        GfxCullViewInfo cullViewInfo;
        //$FED4BC5DDB3EC43BBC2EB57FD171E8A4 __s1;
        struct //$FED4BC5DDB3EC43BBC2EB57FD171E8A4 // sizeof=0x170 // Dont even ask me man, I just fill the structs
        {                                       // XREF: $29D6D5D109C236443008C80F9E8DF0DC/r
            GfxViewParms viewParms;
            GfxViewport sceneViewport;
            GfxViewport displayViewport;
            GfxViewport scissorViewport;
        };
    };
    GfxSceneDef sceneDef;
    ShadowType dynamicShadowType;
    int localClientNum;
    int isRenderingFullScreen;
    int playerTeleported;
    bool needsFloatZ;
    bool renderingFloatZ;
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
    PointLightPartition pointLightPartitions[4];
    GfxMeshData pointLightMeshData[4];
    int pointLightCount;
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
    GfxLight visibleLights[4];
    int visibleLightCount;
    float blurRadius;
    float frustumPlanes[4][4];
    GfxLightScale charPrimaryLightScale;
    GfxDepthOfField dof;
    GfxFilm film;
    GfxCompositeFx flameFx;
    GfxReviveFx reviveFx;
    GfxCompositeFx waterSheetingFx;
    GfxDoubleVision doubleVision;
    GfxGenericFilter genericFilter;
    GfxPoison poisonFx;
    GfxCompositeFx electrifiedFx;
    GfxCompositeFx transportedFx;
    GfxSaveScreenFx saveScreenFx;
    const void *cmds;
    float sunVisibility;
    GfxQuadMeshData *fullSceneViewMesh;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    GfxDrawSurfListInfo drawList[14];
    GfxCmdBufInput input;
    int renderSeeThruDecals;
    unsigned __int8 hdrRenderingMode;
    // padding byte
    // padding byte
    // padding byte
    GfxExposureValue exposureValue;
    GfxExposureShaderRemap exposureRemap;
    GfxViewRenderControl sceneComposition;
    GfxBloom bloom;
    float bloomExpansionXpos;
    float bloomExpansionYpos;
    GfxUI3DBackend rbUI3D;
    bool isMissileCamera;
    bool hasCmdBuf;
    // padding byte
    // padding byte
    float postEmissiveBrightening;
    bool noLodCullOut;
    // padding byte
    // padding byte
    // padding byte
    DynSModelClientView *dynSModelView;
    DynSModelGfxState *dynSModelState;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct GfxDebugPoly // sizeof=0x30
{
    float faceColor[4];
    bool faceDepthTest;
    // padding byte
    // padding byte
    // padding byte
    float edgeColor[4];
    bool edgeDepthTest;
    // padding byte
    // padding byte
    // padding byte
    int firstVert;
    int vertCount;
};

struct PolySet // sizeof=0x18
{                                       // XREF: .data:persistentPolySet/r
                                        // DebugGlobals/r
    float (*verts)[3];                  // XREF: R_UpdatePersistentDebugPolys+56/r
    int vertCount;                      // XREF: R_ClearPersistentDebugPolygons(void)+D/w
    int vertLimit;                      // XREF: R_InitDebug(void)+10/w
    GfxDebugPoly *polys;                // XREF: R_TransferDebugGlobals(DebugGlobals *)+ED/r
                                        // R_UpdatePersistentDebugPolys+29/r ...
    int polyCount;                      // XREF: R_ClearPersistentDebugPolygons(void)+17/w
                                        // R_UpdatePersistentDebugPolys+1B/r
    int polyLimit;                      // XREF: R_InitDebug(void)+1A/w
};

struct GfxDebugPlume // sizeof=0x28
{
    float origin[3];
    float color[4];
    int score;
    int startTime;
    int duration;
};

struct trDebugString_t;
struct trDebugLine_t;
struct trDebugSphere_t;

struct DebugGlobals // sizeof=0x6C
{                                       // XREF: .data:debugGlobals/r
                                        // GfxBackEndData/r
    PolySet polySet;
    trDebugString_t *strings;
    int stringCount;
    int stringLimit;
    trDebugString_t *externStrings;
    int externStringCount;
    int externMaxStringCount;
    trDebugLine_t *lines;
    int lineCount;
    int lineLimit;
    trDebugLine_t *externLines;
    int externLineCount;
    int externMaxLineCount;
    GfxDebugPlume *plumes;              // XREF: R_TransferDebugGlobals(DebugGlobals *)+20/r
                                        // R_TransferDebugGlobals(DebugGlobals *)+3A/r ...
    int plumeCount;                     // XREF: R_TransferDebugGlobals(DebugGlobals *)+12/r
                                        // R_TransferDebugGlobals(DebugGlobals *):loc_ABCE69/r ...
    int plumeLimit;                     // XREF: R_TransferDebugGlobals(DebugGlobals *)+9E/r
    trDebugSphere_t *spheres;
    int sphereCount;
    int sphereLimit;
    trDebugSphere_t *externSpheres;
    int externSphereCount;
    int externMaxSphereCount;
};

struct GfxSunShadowProjection // sizeof=0x60
{                                       // XREF: GfxSunShadow/r
    float viewMatrix[4][4];
    float switchPartition[4];
    float shadowmapScale[4];
};

struct GfxSunShadowBoundingPoly // sizeof=0x78
{                                       // XREF: GfxSunShadowPartition/r
    float snapDelta[2];
    int pointCount;
    float points[9][2];
    int pointIsNear[9];
};

struct __declspec(align(8)) GfxSunShadowPartition // sizeof=0x1D0
{                                       // XREF: GfxSunShadow/r
    GfxViewParms shadowViewParms;
    int partitionIndex;
    GfxViewport viewport;
    GfxSunShadowBoundingPoly boundingPoly;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct GfxSunShadow // sizeof=0x450
{                                       // XREF: GfxBackEndData/r
    GfxMatrix lookupMatrix;
    GfxSunShadowProjection sunProj;
    int sunShadowCount;
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
    GfxSunShadowPartition partition[2];
};

struct __declspec(align(16)) GfxSpotShadow // sizeof=0x1D0
{                                       // XREF: GfxBackEndData/r
    GfxViewParms shadowViewParms;
    GfxMatrix lookupMatrix;
    unsigned __int8 shadowableLightIndex;
    unsigned __int8 flags;
    unsigned __int8 pad[2];
    const GfxLight *light;
    float fade;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    GfxCodeImageRenderTarget spotShadowRenderTarget;
    GfxRenderTargetId renderTargetId;
    // padding byte
    // padding byte
    // padding byte
    GfxViewport viewport;
    GfxImage *image;
    float pixelAdjust[4];
    int clearScreen;
    GfxMeshData *clearMesh;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(32)) GfxBackEndData // sizeof=0x186300
{                                       // XREF: .data:s_backEndData/r
                                        // GfxBackEndDataArray/r
    unsigned __int8 surfsBuffer[262144];
    FxCodeMeshData codeMeshes[2048];
    unsigned int primDrawSurfsBuf[65536];
    GfxViewParms viewParms[4];
    unsigned __int8 primaryLightTechType[16][2][2][256];
    float codeMeshArgs[256][4];
    GfxParticleCloud clouds[256];
    GfxDrawSurf drawSurfs[32768];
    GfxMeshData *codeMeshPtr;
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
    GfxMeshData markMesh;
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
    GfxMeshData glassMesh;              // XREF: R_RecreateGlassRenderBuffers(void)+27/o
                                        // R_InitGlassRenderBuffers(int,int,int)+35/o ...
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
    GfxSModelCachedVertex smcPatchVerts[8192];
    unsigned __int16 smcPatchList[256];
    unsigned int smcPatchCount;
    unsigned int smcPatchVertsUsed;
    GfxModelLightingPatch modelLightingPatchList[4096];
    volatile unsigned int modelLightingPatchCount;
    GfxBackEndPrimitiveData prim;
    unsigned int shadowableLightHasShadowMap[8];
    unsigned int frameCount;
    int drawSurfCount;
    //volatile int surfPos;
    volatile unsigned int surfPos;
    //volatile int gfxEntCount;
    volatile unsigned int gfxEntCount;
    GfxEntity gfxEnts[256];
    //volatile int shaderConstantSetCount;
    volatile unsigned int shaderConstantSetCount;
    ShaderConstantSet shaderConstantSets[128];
    //volatile int textureOverrideCount;
    volatile unsigned int textureOverrideCount;
    GfxTextureOverride textureOverrides[256];
    //volatile int cloudCount;
    //volatile int codeMeshCount;
    //volatile int codeMeshArgsCount;
    //volatile int markMeshCount;
    volatile unsigned int cloudCount;
    volatile unsigned int codeMeshCount;
    volatile unsigned int codeMeshArgsCount;
    volatile unsigned int markMeshCount;
    FxMarkMeshData markMeshes[1536];
    volatile unsigned int *dynamicBufferCurrentFrame;
    struct GfxVertexBufferState *skinnedCacheVb;
    IDirect3DQuery9 *endFence;
    unsigned __int8 *tempSkinBuf;
    //volatile int tempSkinPos;
    volatile unsigned int tempSkinPos;
    GfxIndexBufferState *preTessIb;
    int viewParmCount;
    GfxFog fogSettings;
    GfxCmdArray *commands;
    unsigned int viewInfoIndex;
    unsigned int viewInfoCount;
    GfxViewInfo *viewInfo;
    GfxUI3DBackend rbUI3D;
    const void *cmds;
    const void *compositingCmds;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    GfxLight sunLight;
    int hasApproxSunDirChanged;
    volatile unsigned int primDrawSurfPos;
    DebugGlobals debugGlobals;
    unsigned int drawType;
    int hideMatureContent;
    int splitscreen;
    GfxLight shadowableLights[255];
    unsigned int shadowableLightCount;
    unsigned int emissiveSpotLightIndex;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    GfxLight emissiveSpotLight;
    int emissiveSpotDrawSurfCount;
    GfxDrawSurf *emissiveSpotDrawSurfs;
    unsigned int emissiveSpotLightCount;
    bool extraCamLargeFrameSize;
    // padding byte
    // padding byte
    // padding byte
    GfxSunShadow sunShadow;
    unsigned int spotShadowCount;
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
    GfxSpotShadow spotShadows[4];
    DynSModelGfxState dynSModelState;
    DynSModelClientView *dynSModelClientViewArray;
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

struct __declspec(align(2)) GfxCmdDrawText2D // sizeof=0x60
{
    GfxCmdHeader header;
    float x;
    float y;
    float w;
    float rotation;
    Font_s *font;
    float xScale;
    float yScale;
    GfxColor color;
    int maxChars;
    int renderFlags;
    int cursorPos;
    char cursorLetter;
    // padding byte
    // padding byte
    // padding byte
    GfxColor glowForceColor;
    int fxBirthTime;
    int fxLetterTime;
    int fxDecayStartTime;
    int fxDecayDuration;
    int fxRedactDecayStartTime;
    int fxRedactDecayDuration;
    const Material *fxMaterial;
    const Material *fxMaterialGlow;
    float padding;
    char text[3];
    // padding byte
};

struct GfxCmdDrawQuadList2D // sizeof=0xC
{
    GfxCmdHeader header;
    const Material *material;
    int quadCount;
};

struct GfxQuadVertex // sizeof=0x14
{                                       // XREF: GfxCmdDrawEmblemLayer/r
                                        // ?CL_CompositeDrawEmblemPhysical@@YA_NMMMMQBMPAUCompositeEmblemLayer@@H@Z/r
    float xy[2];
    float st[2];
    GfxColor color;
};

struct GfxCmdSetScissor // sizeof=0x18
{
    GfxCmdHeader header;
    int enabled;
    GfxViewport scissor;
};

struct GfxCmdClearScreen // sizeof=0x1C
{
    GfxCmdHeader header;
    unsigned __int8 whichToClear;
    unsigned __int8 stencil;
    // padding byte
    // padding byte
    float depth;
    float color[4];
};

struct GfxCmdProjectionSet // sizeof=0x8
{
    GfxCmdHeader header;
    GfxProjectionTypes projection;
};

void __cdecl R_InitRenderCommands();
void __cdecl R_ShutdownRenderCommands();
void __cdecl R_RecreateGlassRenderBuffers();
void __cdecl R_InitRenderBuffers();
void __cdecl R_InitDynamicMesh(
                GfxMeshData *mesh,
                unsigned int indexCount,
                unsigned int vertCount,
                unsigned int vertSize);
unsigned int R_InitSpotShadowMeshes();
void __cdecl R_InitGlassRenderBuffers(unsigned int numIndices, unsigned int numVerts, unsigned int vertexSize);
void __cdecl R_FreeGlassRenderBuffers();
void __cdecl R_ShutdownDynamicMesh(GfxMeshData *mesh);
void __cdecl R_ReleaseGlassRenderBuffers();
void __cdecl R_ShutdownRenderBuffers();
void R_ShutdownSpotShadowMeshes();
void __cdecl R_InitTempSkinBuf();
void __cdecl R_ShutdownTempSkinBuf();
void __cdecl R_InitRenderThread();
void __cdecl R_SyncRenderThread();
GfxCmdArray *R_ClearCmdList();
void __cdecl R_IssueRenderCommands(unsigned int type);
void R_PerformanceCounters();
void R_UpdateSkinCacheUsage();
char __cdecl R_HandOffToBackend(char type);
void __cdecl R_ToggleSmpFrameCmd(char type);
void __cdecl R_AbortRenderCommands();
void __cdecl R_BeginClientCmdList2D();
void __cdecl R_ClearClientCmdList2D();
void __cdecl R_BeginSharedCmdList();
void __cdecl R_BeginCompositingCmdList();
void __cdecl R_AddCmdEndOfList();
GfxCmdHeader *__cdecl R_GetCommandBuffer(GfxRenderCommand renderCmd, int bytes);
void R_ToggleSmpFrame();
void R_FreeTempSkinBuffer();
unsigned int __cdecl R_GetFrameCount();
GfxViewParms *__cdecl R_AllocViewParms();
void __cdecl R_AddCmdDrawStretchComposite(
                float x,
                float y,
                float w,
                float h,
                float s0,
                float t0,
                float s1,
                float t1,
                const float *color,
                GfxImage *composite);
void __cdecl R_AddCmdDrawStretchPicW(
                float x,
                float y,
                float w0,
                float w,
                float h,
                float s0,
                float t0,
                float s1,
                float t1,
                const float *color,
                Material *material);
bool __cdecl Material_HasAnyFogableTechnique(const Material *material);
void __cdecl R_AddCmdDrawStretchPic(
                float x,
                float y,
                float w,
                float h,
                float s0,
                float t0,
                float s1,
                float t1,
                const float *color,
                Material *material);
void __cdecl R_AddCmdDrawStretchPicRotateXYW(
                float x,
                float y,
                float w0,
                float w,
                float h,
                float s0,
                float t0,
                float s1,
                float t1,
                float angle,
                const float *color,
                Material *material);
void __cdecl R_AddCmdDrawStretchPicRotateXY(
                float x,
                float y,
                float w,
                float h,
                float s0,
                float t0,
                float s1,
                float t1,
                float angle,
                const float *color,
                Material *material);
void __cdecl R_AddCmdDrawStretchPicRotateST(
                float x,
                float y,
                float w,
                float h,
                float centerS,
                float centerT,
                float radiusST,
                float scaleFinalS,
                float scaleFinalT,
                float angle,
                const float *color,
                Material *material);
void __cdecl R_AddCmdDrawTextWithCursor(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float w,
                float xScale,
                float yScale,
                float rotation,
                const float *color,
                int style,
                int cursorPos,
                char cursor);
GfxCmdDrawText2D *__cdecl AddBaseDrawTextCmd(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float w,
                float xScale,
                float yScale,
                float rotation,
                const float *color,
                int style,
                int cursorPos,
                char cursor);
void __cdecl R_AddCmdDrawText(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                float rotation,
                const float *color,
                int style);
void __cdecl R_AddCmdDrawTextW(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float w,
                float xScale,
                float yScale,
                float rotation,
                const float *color,
                int style);
void __cdecl R_AddCmdDrawTextSubtitle(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                float rotation,
                const float *color,
                int style,
                const float *glowColor,
                bool cinematic);
char __cdecl SetDrawText2DGlowParms(GfxCmdDrawText2D *cmd, const float *color, const float *glowColor);
void __cdecl R_AddCmdDrawTextWithEffects(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float w,
                float xScale,
                float yScale,
                float rotation,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration);
char __cdecl SetDrawText2DTypewriterFXParms(
                GfxCmdDrawText2D *cmd,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration);
void __cdecl R_AddCmdDrawTextWithCOD7DecodeEffects(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                float rotation,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration);
char __cdecl SetDrawText2DCOD7DecodeFXParms(
                GfxCmdDrawText2D *cmd,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration);
void __cdecl R_AddCmdDrawTextWithCOD7TypeWriterEffects(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                float rotation,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration);
char __cdecl SetDrawText2DCOD7TypeWriterFXParms(
                GfxCmdDrawText2D *cmd,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration);
void __cdecl R_AddCmdDrawTextWithRedactEffects(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                float rotation,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                int fxRedactDecayStartTime,
                int fxRedactDecayDuration);
char __cdecl SetDrawText2DRedactFXParms(
                GfxCmdDrawText2D *cmd,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                int fxRedactDecayStartTime,
                int fxRedactDecayDuration);
void __cdecl R_AddCmdDrawTextWithPopInEffects(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                float rotation,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration);
char __cdecl SetDrawText2DPopInWriteFXParms(
                GfxCmdDrawText2D *cmd,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration);
void __cdecl R_AddCmdDrawConsoleText(
                char *textPool,
                int poolSize,
                int firstChar,
                int charCount,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                const float *color,
                int style);
GfxCmdDrawText2D *__cdecl AddBaseDrawConsoleTextCmd(
                char *textPool,
                int poolSize,
                int firstChar,
                int charCount,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                const float *color,
                int style);
void __cdecl CopyPoolTextToCmd(char *textPool, int poolSize, int firstChar, int charCount, GfxCmdDrawText2D *cmd);
void __cdecl R_AddCmdDrawConsoleTextTypewriterFX(
                char *textPool,
                int poolSize,
                int firstChar,
                int charCount,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                const float *color,
                int style,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration);
void __cdecl R_AddCmdDrawConsoleTextSubtitle(
                char *textPool,
                int poolSize,
                int firstChar,
                int charCount,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                const float *color,
                int style,
                const float *glowColor);
void __cdecl R_AddCmdDrawConsoleTextPulseFX(
                char *textPool,
                int poolSize,
                int firstChar,
                int charCount,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                const float *color,
                int style,
                const float *glowColor,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                Material *fxMaterial,
                Material *fxMaterialGlow);
char __cdecl SetDrawText2DPulseFXParms(
                GfxCmdDrawText2D *cmd,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration);
void __cdecl R_AddCmdDrawQuadPicW(const float (*verts)[2], float w, const float *color, Material *material);
void __cdecl R_AddCmdDrawQuadPic(const float (*verts)[2], const float *color, Material *material);
void __cdecl R_AddCmdSetCustomConstant(unsigned int type, const float *vec);
void __cdecl R_AddCmdSetScissorValues(bool enabled, int x, int y, int width, int height);
void __cdecl R_AddCmdResolveComposite(void (__cdecl *callback)(GfxImage *));
void __cdecl R_AddCmdPCCopyImageGenMIP(void (__cdecl *callback)(void *), GfxImage *image, void *job);
void __cdecl R_BeginFrame();
void R_UpdateFrontEndDvarOptions();
bool __cdecl R_LightTweaksModified();
bool __cdecl R_GpuSyncModified();
void R_SetTestLods();
bool __cdecl R_AreAnyImageOverridesActive();
void R_SetOutdoorFeatherConst();
void R_EnvMapOverrideConstants();
void __cdecl R_EndFrame();
void __cdecl R_AddCmdClearScreen(int whichToClear, const float *color, float depth, unsigned __int8 stencil);
GfxCmdDrawQuadList2D *__cdecl R_AddCmdDrawQuadList2D(Material *materialHandle, int quadCount);
void __cdecl R_AddCmdDrawEmblemLayer(
                Material *materialHandle,
                const GfxImage *image,
                int colorIdx,
                float outlineSize,
                float smoothSize,
                GfxQuadVertex *verts);
void __cdecl R_AddCmdProjectionSet2D();
void __cdecl R_AddCmdProjectionSet(GfxProjectionTypes projection);
void __cdecl R_AddCmdDrawFramed(
                float x,
                float y,
                float w,
                float h,
                float thicknessW,
                float thicknessH,
                float thicknessTex,
                int sides,
                const float *color,
                Material *material);
void __cdecl R_BeginRemoteScreenUpdate();
void __cdecl R_EndRemoteScreenUpdate(void (__cdecl *pumpfunc)());
void __cdecl R_PushRemoteScreenUpdate(int remoteScreenUpdateNesting);
int __cdecl R_PopRemoteScreenUpdate();
bool __cdecl R_IsInRemoteScreenUpdate();
bool __cdecl R_IsRemoteScreenUpdateActive();



extern GfxBackEndData *frontEndDataOut;