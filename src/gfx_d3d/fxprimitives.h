#pragma once
#include "r_material.h"
#include <universal/q_shared.h>

enum FxRandKey : __int32
{                                       // XREF: FX_IntegrateRotationFromZero/r
    FXRAND_VELOCITY_X             = 0x0,
    FXRAND_VELOCITY_Y             = 0x1,
    FXRAND_VELOCITY_Z             = 0x2,
    FXRAND_ANGULAR_VELOCITY_PITCH = 0x3,
    FXRAND_ANGULAR_VELOCITY_YAW   = 0x4,
    FXRAND_ANGULAR_VELOCITY_ROLL  = 0x5,
    FXRAND_ORIGIN_X               = 0x6,
    FXRAND_ORIGIN_Y               = 0x7,
    FXRAND_ORIGIN_Z               = 0x8,
    FXRAND_OFFSET_YAW             = 0x9,
    FXRAND_OFFSET_HEIGHT          = 0xA,
    FXRAND_OFFSET_RADIUS          = 0xB,
    FXRAND_ANGLES_PITCH           = 0xC,
    FXRAND_ANGLES_YAW             = 0xD,
    FXRAND_ANGLES_ROLL            = 0xE,
    FXRAND_GRAVITY                = 0xF,
    FXRAND_REFLECTION_FACTOR      = 0x10,
    FXRAND_LIFE_SPAN              = 0x11,
    FXRAND_SPAWN_DELAY            = 0x12,
    FXRAND_SPAWN_COUNT            = 0x13,
    FXRAND_EMIT_DIST              = 0x14,
    FXRAND_VISUAL                 = 0x15,
    FXRAND_TILE_START             = 0x16,
    FXRAND_COLOR                  = 0x17,
    FXRAND_ROTATION               = 0x18,
    FXRAND_ROTATION_DELTA         = 0x19,
    FXRAND_SIZE_0                 = 0x1A,
    FXRAND_SIZE_1                 = 0x1B,
    FXRAND_SCALE                  = 0x1C,
    FXRAND_CLOUD_DENSITY          = 0x1D,
    FXRAND_COUNT                  = 0x1E,
};

struct FxSpawnDefLooping // sizeof=0x8
{                                       // XREF: FxSpawnDef/r
                                        // FxEditorElemDef/r
    int intervalMsec;
    int count;
};

struct FxIntRange // sizeof=0x8
{                                       // XREF: FxSpawnDefOneShot/r
                                        // FxElemDefUnion/r ...
    int base;                           // XREF: FX_DrawElem_Cloud_Main+12/w
                                        // FX_DrawElem_Cloud_Main+42/r ...
    int amplitude;                      // XREF: FX_DrawElem_Cloud_Main+21/w
                                        // FX_DrawElem_Cloud_Main+24/r ...
};

struct FxSpawnDefOneShot // sizeof=0x8
{                                       // XREF: FxSpawnDef/r
                                        // FxEditorElemDef/r
    FxIntRange count;
};

union FxSpawnDef // sizeof=0x8
{                                       // XREF: FxElemDef/r
    FxSpawnDefLooping looping;
    FxSpawnDefOneShot oneShot;
};

struct FxFloatRange // sizeof=0x8
{                                       // XREF: FxElemDef/r
                                        // FxElemDef/r ...
    float base;
    float amplitude;
};

struct FxElemAtlas // sizeof=0x8
{                                       // XREF: FxElemDef/r
    unsigned __int8 behavior;
    unsigned __int8 index;
    unsigned __int8 fps;
    unsigned __int8 loopCount;
    unsigned __int8 colIndexBits;
    unsigned __int8 rowIndexBits;
    unsigned __int16 entryCountAndIndexRange;
};

struct FxElemVec3Range // sizeof=0x18
{                                       // XREF: FxElemVelStateInFrame/r
                                        // FxElemVelStateInFrame/r
    float base[3];
    float amplitude[3];
};

struct FxElemVelStateInFrame // sizeof=0x30
{                                       // XREF: FxElemVelStateSample/r
                                        // FxElemVelStateSample/r
    FxElemVec3Range velocity;
    FxElemVec3Range totalDelta;
};

struct FxElemVelStateSample // sizeof=0x60
{
    FxElemVelStateInFrame local;
    FxElemVelStateInFrame world;
};

struct FxElemVisualState // sizeof=0x18
{                                       // XREF: FxElemVisStateSample/r
                                        // FxElemVisStateSample/r ...
    unsigned __int8 color[4];
    float rotationDelta;
    float rotationTotal;                // XREF: FX_CreateImpactMarkInternal+174/r
    float size[2];                      // XREF: FX_CreateImpactMarkInternal+A1/w
                                        // FX_CreateImpactMarkInternal+169/r
    float scale;
};

struct FxElemVisStateSample // sizeof=0x30
{
    FxElemVisualState base;
    FxElemVisualState amplitude;
};

struct FxElemMarkVisuals // sizeof=0x8
{                                       // XREF: $02AC52774A03C1B6F49B7C17F56F8A85/r
    Material *materials[2];
};

union FxEffectDefRef // sizeof=0x4
{                                       // XREF: FxElemVisuals/r
                                        // FxElemDef/r ...
    const FxEffectDef *handle;
    const char *name;
};

union FxElemVisuals // sizeof=0x4
{                                       // XREF: FX_GenSpriteVerts+4C/w
                                        // FX_GenSpriteVerts+52/r ...
    const void *anonymous;
    Material *material;
    XModel *model;
    FxEffectDefRef effectDef;
    const char *soundName;
};

union FxElemDefVisuals // sizeof=0x4
{                                       // XREF: FxElemDef/r
    FxElemMarkVisuals *markArray;
    FxElemVisuals *array;
    FxElemVisuals instance;
};

struct FxTrailVertex // sizeof=0x14
{                                       // XREF: FxEditorTrailDef/r
    float pos[2];
    float normal[2];
    float texCoord;
};

struct FxTrailDef // sizeof=0x1C
{
    int scrollTimeMsec;
    int repeatDist;
    int splitDist;
    int vertCount;
    //$A41F6DDA83A77139D9FD89879C7CFB09 ___u4;
    union //$A41F6DDA83A77139D9FD89879C7CFB09 // sizeof=0x4
    {                                       // XREF: FxTrailDef/r
        FxTrailVertex *vertsEA;
        FxTrailVertex *verts;
    };
    int indCount;
    //$D31B43AE1E46D24CA4A859818848B5E1 ___u6;
    union //$D31B43AE1E46D24CA4A859818848B5E1 // sizeof=0x4
    {                                       // XREF: FxTrailDef/r
        unsigned __int16 *indsEA;
        unsigned __int16 *inds;
    };
};

struct FxBillboardTrim // sizeof=0x8
{                                       // XREF: FxElemDefUnion/r
    float topWidth;
    float bottomWidth;
};

union FxElemDefUnion // sizeof=0x8
{                                       // XREF: FxElemDef/r
    FxBillboardTrim billboard;
    FxIntRange cloudDensityRange;
};

struct FxElemSpawnSound // sizeof=0x4
{                                       // XREF: FxElemDef/r
                                        // FxEditorElemDef/r
    const char *spawnSound;
};

struct FxElemDef // sizeof=0x124
{
    int flags;
    FxSpawnDef spawn;
    FxFloatRange spawnRange;
    FxFloatRange fadeInRange;
    FxFloatRange fadeOutRange;
    float spawnFrustumCullRadius;
    FxIntRange spawnDelayMsec;
    FxIntRange lifeSpanMsec;
    FxFloatRange spawnOrigin[3];
    FxFloatRange spawnOffsetRadius;
    FxFloatRange spawnOffsetHeight;
    FxFloatRange spawnAngles[3];
    FxFloatRange angularVelocity[3];
    FxFloatRange initialRotation;
    unsigned int rotationAxis;
    FxFloatRange gravity;
    FxFloatRange reflectionFactor;
    FxElemAtlas atlas;
    float windInfluence;
    unsigned __int8 elemType;
    unsigned __int8 visualCount;
    unsigned __int8 velIntervalCount;
    unsigned __int8 visStateIntervalCount;
    const FxElemVelStateSample *velSamples;
    const FxElemVisStateSample *visSamples;
    FxElemDefVisuals visuals;
    float collMins[3];
    float collMaxs[3];
    FxEffectDefRef effectOnImpact;
    FxEffectDefRef effectOnDeath;
    FxEffectDefRef effectEmitted;
    FxFloatRange emitDist;
    FxFloatRange emitDistVariance;
    FxEffectDefRef effectAttached;
    //$6560A5492D3E60888A9CD1F2E81D900E ___u34;
    union //$6560A5492D3E60888A9CD1F2E81D900E // sizeof=0x4
    {                                       // XREF: FxElemDef/r
        FxTrailDef *localTrailDef;
        FxTrailDef *trailDef;
    };
    unsigned __int8 sortOrder;
    unsigned __int8 lightingFrac;
    unsigned __int8 unused[2];
    unsigned __int16 alphaFadeTimeMsec;
    unsigned __int16 maxWindStrength;
    unsigned __int16 spawnIntervalAtMaxWind;
    unsigned __int16 lifespanAtMaxWind;
    FxElemDefUnion u;
    FxElemSpawnSound spawnSound;
    float billboardPivot[2];
};

struct FxEffectDef // sizeof=0x3C
{                                       // XREF: XAssetPoolEntry<FxEffectDef>/r
    const char *name;
    unsigned __int8 flags;
    unsigned __int8 efPriority;
    unsigned __int8 reserved[2];
    int totalSize;
    int msecLoopingLife;
    int elemDefCountLooping;
    int elemDefCountOneShot;
    int elemDefCountEmission;
    //$7BD2F22D4D2E155DDD2F5AE6C1F591D8 ___u9;
    union //$7BD2F22D4D2E155DDD2F5AE6C1F591D8 // sizeof=0x4
    {                                       // XREF: FxEffectDef/r
        const FxElemDef *elemDefsEA;
        const FxElemDef *elemDefs;
    };
    float boundingBoxDim[3];
    float boundingSphere[4];
};

struct FxCamera // sizeof=0xA0
{                                       // XREF: FxSystem/r FxSystem/r ...
    float origin[3];
    volatile unsigned int isValid;
    float frustum[6][4];
    float axis[3][3];
    unsigned int frustumPlaneCount;
    unsigned int pad[2];
};

struct r_double_index_t // sizeof=0x4
{                                       // XREF: FX_BuildQuadStampCodeMeshVerts/r
                                        // ?FX_PostLight_GenerateVerts@@YAXPAUFxPostLightInfo@@PAUFxSystem@@@Z/r ...
    //unsigned __int16 value[2];          // XREF: FX_Beam_GenerateVerts(FxGenerateVertsCmd *)+5AB/w
    // FX_Beam_GenerateVerts(FxGenerateVertsCmd *)+5C2/w ...
    union
    {
        unsigned __int16 value[2];          // ...
        unsigned int kisak;
    };
    r_double_index_t()
    {
        kisak = 0;
    }
    r_double_index_t(int val)
    {
        kisak = val;
    }
};

struct FxSpriteInfo // sizeof=0x10
{                                       // XREF: .data:g_spriteInfo/r
                                        // FxSystem/r
    r_double_index_t *indices;          // XREF: FX_SpriteBegin(void)+3/w
    // FX_SpriteAdd(FxSprite *)+5/r ...
    unsigned int indexCount;
    Material *material;
    const char *name;
};


union FxBoltAndSortOrder // sizeof=0x4
{                                       // XREF: FxEffect/r
    //$88D60DA77564256B22C3F95D9DD45A24 __s0;
    struct //$88D60DA77564256B22C3F95D9DD45A24 // sizeof=0x4
    {                                       // XREF: FxBoltAndSortOrder/r
        unsigned __int32 sortOrder : 8;
        unsigned __int32 dobjHandle : 11;
        unsigned __int32 temporalBits : 2;
        unsigned __int32 boneIndex : 8;
        unsigned __int32 type : 2;
        unsigned __int32 rewindKill : 1;
    };

    //$58A480E6171600052C982635434E76CC __s1;
    struct //$58A480E6171600052C982635434E76CC // sizeof=0x4
    {                                       // XREF: FxBoltAndSortOrder/r
        unsigned __int32 _sortOrder : 8;
        unsigned __int32 absDynEntId : 21;
        unsigned __int32 _type : 2;
        // padding bit
    };
};

struct FxSpatialFrame // sizeof=0x1C
{                                       // XREF: FxEffect/r FxEffect/r ...
    float quat[4];
    float origin[3];                    // XREF: FX_SpawnLoopingElems+355/o
};

struct __declspec(align(16)) FxEffect // sizeof=0xC0
{                                       // XREF: FxEffectContainer/r
    //$A7FC777CB20AB0354B9D4E9742D6DE43 ___u0;
    union //$A7FC777CB20AB0354B9D4E9742D6DE43 // sizeof=0x4
    {                                       // XREF: FxEffect/r
        const FxEffectDef *def;
        const FxEffectDef *localDef;
    };
    unsigned __int16 firstElemHandle[3];
    unsigned __int16 firstSortedElemHandle;
    unsigned __int16 firstTrailHandle;
    unsigned __int16 randomSeed;
    unsigned __int16 owner;
    __int16 lightR;
    __int16 lightG;
    __int16 lightB;
    FxBoltAndSortOrder boltAndSortOrder;
    volatile unsigned int frameCount;
    int msecBegin;
    int msecLastUpdate;
    FxSpatialFrame frameAtSpawn;
    FxSpatialFrame frameNow;
    FxSpatialFrame framePrev;
    float distanceTraveled;
    orientation_t boneOffset;
    unsigned int uniqueHandle;
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

struct FxEffectAtomics // sizeof=0x4
{                                       // XREF: FxEffectContainer/r
    volatile unsigned int status;
};

struct __declspec(align(16)) FxEffectContainer // sizeof=0xD0
{                                       // XREF: FxSystemBuffers/r
    FxEffect effect;
    FxEffectAtomics atomics;
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

// kinda bizarre, why is there 2 Types now
template <typename T1, typename T2>
struct FxPool
{
    union
    {
        int nextFree;
        T2 item;
    };
};



struct FxElem // sizeof=0x2C
{                                       // XREF: FxElemContainer/r
    unsigned __int8 defIndex;
    unsigned __int8 sequence;
    unsigned __int8 atRestFraction;
    unsigned __int8 emitResidual;
    int msecBegin;
    float baseVel[3];
    //$A58BA6DA60295001BBA5E9F807131CF1 ___u6;
    union //$A58BA6DA60295001BBA5E9F807131CF1 // sizeof=0xC
    {                                       // XREF: FxElem/r
                                            // FX_UpdateElement/r
        int physObjId;
        float origin[3];
    };
    //FxElem::<unnamed_type_u> u;
    union //FxElem::<unnamed_type_u> // sizeof=0x4
    {                                       // XREF: FxElem/r
        float trailTexCoord;
        //FxElem::<unnamed_type_u>::<unnamed_type_s> s;
        struct //FxElem::<unnamed_type_u>::<unnamed_type_s> // sizeof=0x4
        {                                       // XREF: FxElem::<unnamed_type_u>/r
            unsigned __int16 lightingHandle;
            //FxElem::<unnamed_type_u>::<unnamed_type_s>::<unnamed_type_fade> fade;
            struct //FxElem::<unnamed_type_u>::<unnamed_type_s>::<unnamed_type_fade> // sizeof=0x2
            {                                       // XREF: FxElem::<unnamed_type_u>::<unnamed_type_s>/r
                unsigned __int8 fadeAlpha;
                unsigned __int8 fadeAlphaPerSec;
            } fade;
        } s;
    } u ;
    unsigned int attachedEffectHandle;
    unsigned __int16 windInterp;
    unsigned __int8 elemSpawnVisBits;
    unsigned __int8 pad;

    static constexpr size_t HANDLE_SCALE = 4;
};

struct FxElemShared // sizeof=0x4
{                                       // XREF: FxElemContainer/r
    unsigned __int16 nextElemHandleInEffect;
    unsigned __int16 prevElemHandleInEffect;
};

struct FxElemContainer // sizeof=0x30
{                                       // XREF: $C3A980BD185A85C14A8D8F3ED1F51981/r
    FxElem elem;
    FxElemShared shared;
};

struct FxTrail // sizeof=0x8
{                                       // XREF: $FE553BDB23E05BFD5781B8DE83CDF494/r
                                        // ?FX_SpawnEffect_AllocTrails@@YAXPAUFxSystem@@PAUFxEffect@@1@Z/r
    unsigned __int16 nextTrailHandle;   // XREF: FX_SpawnEffect_AllocTrails(FxSystem *,FxEffect *,FxEffect *)+C5/w
                                        // FX_SpawnEffect_AllocTrails(FxSystem *,FxEffect *,FxEffect *)+132/r
    unsigned __int16 firstElemHandle;   // XREF: FX_SpawnEffect_AllocTrails(FxSystem *,FxEffect *,FxEffect *)+107/w
    unsigned __int16 lastElemHandle;    // XREF: FX_SpawnEffect_AllocTrails(FxSystem *,FxEffect *,FxEffect *)+110/w
                                        // FX_SpawnEffect_AllocTrails(FxSystem *,FxEffect *,FxEffect *)+137/r
    char defIndex;                      // XREF: FX_SpawnEffect_AllocTrails(FxSystem *,FxEffect *,FxEffect *)+CC/w
                                        // FX_SpawnEffect_AllocTrails(FxSystem *,FxEffect *,FxEffect *)+CF/r
    char sequence;                      // XREF: FX_SpawnEffect_AllocTrails(FxSystem *,FxEffect *,FxEffect *)+114/w

    static constexpr size_t HANDLE_SCALE = 4;
};

struct FxTrailElem // sizeof=0x20
{                                       // XREF: $BA950AF5371231A1FA5C91E603D2D7D7/r
    float origin[3];
    float spawnDist;
    int msecBegin;
    unsigned __int16 nextTrailElemHandle;
    __int16 baseVelZ;
    char basis[2][3];
    unsigned __int8 sequence;
    unsigned __int8 unused;

    static constexpr size_t HANDLE_SCALE = 4;
};

struct FxVisBlocker // sizeof=0x10
{                                       // XREF: FxVisState/r
    float origin[3];
    unsigned __int16 radius;
    unsigned __int16 visibility;
};

struct FxVisState // sizeof=0x1010
{                                       // XREF: FxSystemBuffers/r
    FxVisBlocker blocker[256];
    volatile unsigned int blockerCount;
    unsigned int pad[3];
};

struct FxPriorityInfo // sizeof=0x10
{                                       // XREF: FxSystem/r
    int warningHigh[2];
    int readIndex;
    int pad[1];
};

struct FxUniqueHandleDb // sizeof=0x4010
{                                       // XREF: FxSystemBuffers/r
    struct Node
    {
        int nextNode;
        unsigned int handle;
        unsigned __int16 effect;
        unsigned __int16 pad[3];
    };

    FxUniqueHandleDb::Node nodes[1024];
    int firstFreeNode;
    volatile unsigned int numNodesInUse;
    int pad[2];
};

struct __declspec(align(16)) FxSystemShared // sizeof=0x840
{                                       // XREF: FxSystemContainer/r
    volatile unsigned int firstFreeElem;
    volatile unsigned int firstFreeTrailElem;
    volatile unsigned int firstFreeTrail;
    volatile unsigned int activeElemCount;
    volatile unsigned int activeTrailElemCount;
    volatile unsigned int activeTrailCount;
    volatile unsigned int firstActiveEffect;
    volatile unsigned int firstNewEffect;
    volatile unsigned int firstFreeEffect;
    unsigned __int16 activeSpotLightEffectHandle;
    unsigned __int16 activeSpotLightElemHandle;
    volatile unsigned int activeSpotLightEffectCount;
    volatile unsigned int activeSpotLightElemCount;
    unsigned __int16 allEffectHandles[1024];
    volatile unsigned int iteratorCount;
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

struct FxSystem // sizeof=0x360
{                                       // XREF: FxSystemContainer/r
    FxCamera camera;
    FxCamera cameraPrev;
    FxCamera extraCamera;
    FxCamera extraCameraPre;
    FxSpriteInfo sprite;
    FxEffectContainer *effects;
    FxPool<FxElem, FxElemContainer> *elems;
    FxPool<FxTrail, FxTrail> *trails;
    FxPool<FxTrailElem, FxTrailElem> *trailElems;
    volatile unsigned int gfxCloudCount;
    FxVisState *visState;
    const FxVisState *visStateBufferRead;
    FxVisState *visStateBufferWrite;
    __int16 activeSpotLightBoltDobj;
    // padding byte
    // padding byte
    int msecNow;
    volatile unsigned int msecDraw;
    int frameCount;
    bool isInitialized;
    bool needsGarbageCollection;
    bool isArchiving;
    unsigned __int8 localClientNum;
    unsigned int restartList[32];
    FxPriorityInfo priorityInfo;
    FxUniqueHandleDb *uniqueHandleDb;
    bool hasStaleAttachedFx;
    // padding byte
    // padding byte
    // padding byte
    FxSystemShared *shared;
};

struct FxBeam // sizeof=0x48
{                                       // XREF: FxBeamInfo/r
    float begin[3];                     // XREF: CG_Laser_Add_Core+2C7/w
    float end[3];                       // XREF: CG_Laser_Add_Core+2EE/w
    GfxColor beginColor;                // XREF: CG_Laser_Add_Core+310/w
    GfxColor endColor;                  // XREF: CG_Laser_Add_Core+3E6/w
    float beginRadius;                  // XREF: CG_Laser_Add_Core+434/w
    float endRadius;                    // XREF: CG_Laser_Add_Core+44C/w
    Material *material;                 // XREF: CG_Laser_Add_Core+459/w
    Material *material2;                // XREF: Flame_Class_Stream_Render_Item(int,flameStream_s const *,Material *,Material *,float (*)[3],bool)+580/w
    int segmentCount;                   // XREF: CG_Laser_Add_Core+45F/w
    int perpSegmentCount;               // XREF: Flame_Class_Stream_Render_Item(int,flameStream_s const *,Material *,Material *,float (*)[3],bool)+595/w
    float wiggleDist;                   // XREF: CG_Laser_Add_Core+471/w
    float *segmentVerts;                // XREF: CG_DrawTracer(float const * const,float const * const,refdef_s const *,localEntity_s *)+15A/w
    int flags;                          // XREF: CG_DrawTracer(float const * const,float const * const,refdef_s const *,localEntity_s *)+197/w
    float animFrac;                     // XREF: Flame_Class_Stream_Render_Item(int,flameStream_s const *,Material *,Material *,float (*)[3],bool)+61A/w
};

struct FxBeamInfo // sizeof=0x1B04
{                                       // XREF: .data:g_beamInfo/r
    FxBeam beams[96];
    volatile int beamCount;             // XREF: FX_Beam_Begin(void)+3/w
};

struct FxPostLight;
struct FxPostLightInfo;

struct FxSprite // sizeof=0x20
{                                       // XREF: CG_AddPlayerSpriteDrawSurf/r
                                        // AddDrawSurfForHudElemWaypoint/r
    Material *material;
    float pos[3];
    unsigned __int8 rgbaColor[4];
    float radius;
    float minScreenRadius;
    int flags;
};

template<typename ITEM_TYPE, typename ITEM_TYPE2, size_t LIMIT>
uint16 FX_PoolToHandle_Generic(FxPool<ITEM_TYPE, ITEM_TYPE2> *poolArray, ITEM_TYPE *item)
{
    static_assert((LIMIT * ITEM_TYPE::HANDLE_SCALE) <= 0xFFFF, "do not support huge pools at the moment");

    //vassert(item && item >= &poolArray[0].item && item < &poolArray[LIMIT].item, "%p %p", poolArray, item);
    return ((char *)item - (char *)poolArray) / ITEM_TYPE::HANDLE_SCALE;
}

template<typename ITEM_TYPE, typename ITEM_TYPE2, size_t LIMIT>
FxPool<ITEM_TYPE, ITEM_TYPE2> *FX_PoolFromHandle_Generic(FxPool<ITEM_TYPE, ITEM_TYPE2> *poolArray, uint handle)
{
    //vassert(handle < (LIMIT * sizeof(ITEM_TYPE) / ITEM_TYPE::HANDLE_SCALE) && handle % (sizeof(ITEM_TYPE) / ITEM_TYPE::HANDLE_SCALE) == 0, "%p %u", poolArray, handle);
    return (FxPool<ITEM_TYPE, ITEM_TYPE2> *)((char *)poolArray + (handle * ITEM_TYPE::HANDLE_SCALE));
}
