#pragma once
#include <gfx_d3d/r_material.h>
#include <gfx_d3d/fxprimitives.h>

struct GlassState // sizeof=0x4
{                                       // XREF: GlassClient/r

    enum State : __int32
    {                                       // XREF: ?SetState@GlassServer@@QAEXW4State@GlassState@@QBM1@Z/r
        PRISTINE  = 0x0,
        CRACKED   = 0x1,
        SHATTERED = 0x2,
        INVALID   = 0x3,
    };
    //GlassState::<unnamed_type_val> val;
    union //GlassState::<unnamed_type_val> // sizeof=0x4
    {                                       // XREF: GlassState/r
        unsigned int i;
        //$01920CFBCEBBC7D8417C03082B059610 __s1;
        struct
        {
            unsigned __int32 state : 4;
            unsigned __int32 id : 12;
        };
    } val;
};

struct GlassDef // sizeof=0x3C
{                                       // XREF: GlassDefLoad/r
    const char *name;
    int maxHealth;
    float thickness;
    float minShardSize;
    float maxShardSize;
    float shardLifeProbablility;
    int maxShards;
    Material *pristineMaterial;
    Material *crackedMaterial;
    Material *shardMaterial;
    const char *crackSound;
    const char *shatterShound;
    const char *autoShatterShound;
    const FxEffectDef *crackEffect;
    const FxEffectDef *shatterEffect;
};

struct Glass // sizeof=0x7C
{
    GlassDef *glassDef;
    unsigned int index;
    unsigned int brushModel;
    float origin[3];
    float angles[3];
    float absmin[3];
    float absmax[3];
    bool isPlanar;
    unsigned __int8 numOutlineVerts;
    // padding byte
    // padding byte
    float (*outline)[2];
    float outlineAxis[3][3];
    float outlineOrigin[3];
    float uvScale;
    float thickness;
};

struct Glasses // sizeof=0x38
{                                       // XREF: .data:glasses/r
    const char *name;                   // XREF: GetGlasses_LoadObj(void)+4A/w
    unsigned int numGlasses;            // XREF: GetGlasses_LoadObj(void)+40/w
    Glass *glasses;                     // XREF: GetGlasses_LoadObj(void)+3A/w
    unsigned __int8 *workMemory;
    unsigned int workMemorySize;
    unsigned int smallAllocatorBlocks;
    unsigned int maxGroups;
    unsigned int maxShards;
    unsigned int maxPhysics;
    unsigned int shardMemorySize;
    unsigned int maxFreeCmd;
    unsigned int numSlots;
    unsigned int numVerts;
    unsigned int numIndices;
};

struct __declspec(align(8)) GlassTimer // sizeof=0x10
{                                       // XREF: ?Triangulate@Triangles@GlassShard@@QAE_NXZ/r
                                        // ?Shatter@GlassShard@@QAEHQAPAU1@H@Z/r ...
    unsigned __int64 start;             // XREF: ShardGroup::GenerateVerts(bool,int)+2F5/w
                                        // ShardGroup::GenerateVerts(bool,int)+2F8/w ...
    unsigned __int64 *counter;          // XREF: ShardGroup::GenerateVerts(bool,int)+2ED/w
                                        // ShardGroup::GenerateVerts(bool,int)+426/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};