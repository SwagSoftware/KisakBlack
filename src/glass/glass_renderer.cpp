#include "glass_renderer.h"
#include "glass_client.h"

#include <gfx_d3d/r_drawsurf.h>
#include <gfx_d3d/r_model_lighting.h>
#include <client/splitscreen.h>
#include <client/client.h>
#include <bgame/bg_misc.h>
#include <gfx_d3d/r_stream.h>
#include <cgame/cg_drawtools.h>
#include <tl/tl_system.h>
#include <client/cl_debugdata.h>
#include <gfx_d3d/r_model_lod.h>
#include <gfx_d3d/r_primarylights.h>
#include <universal/com_workercmds.h>

// KISAKTODO: uses too much aislop to get rid of the horrible STL iterators and such (should be done manually with another pass)

cmd_function_s MemInfoCmd_VAR;
cmd_function_s CrashGlassCmd_VAR;
cmd_function_s PrintHwmCmd_VAR;

float MIN_SHARD_GROUP_VOLUME_SIZE = 32.0;


void *GlassRenderer::operator new(size_t size)
{
    return GlassesClient::Allocate(
        size,
        "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp",
        72);
}

void GlassRenderer::operator delete(void *ptr)
{
    GlassesClient::Free((char *)ptr);
}

GlassRenderer::GlassRenderer(const Glasses *glasses)
{
    unsigned int *v2; // eax
    ShardGroup *v3; // eax
    GlassShard *v4; // eax
    GlassPhysics *v5; // eax
    unsigned int *v6; // eax
    unsigned int FreeMem; // eax
    //unsigned int maxGroups; // [esp+10h] [ebp-E8h]
    //unsigned int maxShards; // [esp+10h] [ebp-E8h]
    //unsigned int maxPhysics; // [esp+10h] [ebp-E8h]
    unsigned int smallAllocatorBlocks; // [esp+14h] [ebp-E4h]
    unsigned int shardMemorySize; // [esp+14h] [ebp-E4h]
    FixedSizeAllocator<GlassPhysics *> *v14; // [esp+2Ch] [ebp-CCh]
    FixedSizeAllocator<GlassShard *> *v15; // [esp+30h] [ebp-C8h]
    FixedSizeAllocator<ShardGroup *> *v16; // [esp+34h] [ebp-C4h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *v17; // [esp+38h] [ebp-C0h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *v18; // [esp+3Ch] [ebp-BCh]
    //SmallAllocatorTemplate<GlassPhysics *> v20; // [esp+90h] [ebp-68h] BYREF
    unsigned int *v21; // [esp+94h] [ebp-64h]
    //SmallAllocatorTemplate<GlassPhysics *> _Al; // [esp+A0h] [ebp-58h] BYREF
    unsigned int *v23; // [esp+A4h] [ebp-54h]
    SmallAllocator *p_smallAllocator; // [esp+A8h] [ebp-50h]
    GlassLock *p_rendererLock; // [esp+ACh] [ebp-4Ch]
    FixedSizeAllocator<GlassPhysics *> *v26; // [esp+B0h] [ebp-48h]
    FixedSizeAllocator<GlassShard *> *v27; // [esp+B4h] [ebp-44h]
    FixedSizeAllocator<ShardGroup *> *v28; // [esp+B8h] [ebp-40h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *v29; // [esp+BCh] [ebp-3Ch]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *v30; // [esp+C0h] [ebp-38h]
    unsigned int i; // [esp+C4h] [ebp-34h]
    float minGridSize[3]; // [esp+C8h] [ebp-30h] BYREF
    float gridSize[3]; // [esp+D4h] [ebp-24h] BYREF
    float allExtent[3]; // [esp+E0h] [ebp-18h]
    float scale[3]; // [esp+ECh] [ebp-Ch]

    p_rendererLock = &this->rendererLock;
    this->rendererLock.lock = 0;
    p_smallAllocator = &this->smallAllocator;
    this->smallAllocator.memory = 0;
    smallAllocatorBlocks = glasses->smallAllocatorBlocks;
    v2 = GlassesClient::Allocate(
                 16 * smallAllocatorBlocks,
                 "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp",
                 70);
    this->smallAllocator.Init(v2, 0x10u, smallAllocatorBlocks);


    //v23 = GlassesClient::Allocate(28, "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp", 72);
    //v30 = (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)v23;
    //if ( v23 )
    //{
    //    _Al.alloc = &this->smallAllocator;
    //    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>(v30, &_Al);
    //    this->colidingShards = v30;
    //}
    //else
    //{
    //    this->colidingShards = 0;
    //}
    this->colidingShards = new GlassRenderer::SortedShardsList(&this->smallAllocator);


    //v21 = GlassesClient::Allocate(28, "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp", 73);
    //v29 = (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)v21;
    //if ( v21 )
    //{
    //    v20.alloc = &this->smallAllocator;
    //    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>(v29, &v20);
    //    this->tempShardsList = v29;
    //}
    //else
    //{
    //    this->tempShardsList = 0;
    //}
    this->tempShardsList = new GlassRenderer::SortedShardsList(&this->smallAllocator);


    //v28 = (FixedSizeAllocator<ShardGroup *> *)GlassesClient::Allocate(64, "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp", 75);
    //if ( v28 )
    //{
    //    maxGroups = glasses->maxGroups;
    //    v3 = (ShardGroup *)GlassesClient::Allocate(
    //                                             84 * maxGroups,
    //                                             "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp",
    //                                             75);
    //    this->groupsAllocator = FixedSizeAllocator<ShardGroup *>::FixedSizeAllocator<ShardGroup *>(v28, v3, maxGroups, &this->smallAllocator);
    //}
    //else
    //{
    //    this->groupsAllocator = 0;
    //}

    v3 = (ShardGroup *)GlassesClient::Allocate(sizeof(ShardGroup) * glasses->maxGroups, "blah", 123);
    this->groupsAllocator = new FixedSizeAllocator<ShardGroup *>(v3, glasses->maxGroups, &this->smallAllocator);


    //v27 = (FixedSizeAllocator<GlassShard *> *)GlassesClient::Allocate(64, "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp", 77);
    //if ( v27 )
    //{
    //    maxShards = glasses->maxShards;
    //    v4 = GlassesClient::Allocate(144 * maxShards, "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp", 77);
    //    this->shardsAllocator = FixedSizeAllocator<GlassShard *>::FixedSizeAllocator<GlassShard *>(
    //                    v27,
    //                    (GlassShard *)v4,
    //                    maxShards,
    //                    &this->smallAllocator);
    //}
    //else
    //{
    //    this->shardsAllocator = 0;
    //}
    v4 = (GlassShard*)GlassesClient::Allocate(sizeof(GlassShard) * glasses->maxShards, "asdf", 1243);
    this->shardsAllocator = new FixedSizeAllocator<GlassShard *>(v4, glasses->maxShards, &this->smallAllocator);


    //v26 = (FixedSizeAllocator<GlassPhysics *> *)GlassesClient::Allocate(64, "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp", 79);
    //if ( v26 )
    //{
    //    maxPhysics = glasses->maxPhysics;
    //    v5 = GlassesClient::Allocate(160 * maxPhysics, "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp", 79);
    //    this->physicsAllocator = FixedSizeAllocator<GlassPhysics *>::FixedSizeAllocator<GlassPhysics *>(
    //                    v26,
    //                    (GlassPhysics *)v5,
    //                    maxPhysics,
    //                    &this->smallAllocator);
    //}
    //else
    //{
    //    this->physicsAllocator = 0;
    //}
    v5 = (GlassPhysics*)GlassesClient::Allocate(sizeof(GlassPhysics) * glasses->maxPhysics, "grug", 21354);
    this->physicsAllocator = new FixedSizeAllocator<GlassPhysics *>(v5, glasses->maxPhysics, &this->smallAllocator);

    shardMemorySize = glasses->shardMemorySize;
    v6 = GlassesClient::Allocate(shardMemorySize, "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp", 81);
    this->shardMemoryAllocator.Init(v6, shardMemorySize);
    GlassRenderer::InitShardMeshVertexLists();
    R_InitGlassRenderBuffers(glasses->numIndices, glasses->numVerts, 0x20u);
    FreeMem = GlassesClient::GetFreeMem();
    Com_Printf(0, "GlassRenderer init: unused memory: %d\n", FreeMem);
    Cmd_AddCommandInternal("glassMemInfo", BLOPS_NULLSUB, &MemInfoCmd_VAR);
    Cmd_AddCommandInternal("crashGlass", GlassRenderer::CrashGlassCmd, &CrashGlassCmd_VAR);
    Cmd_AddCommandInternal("glassHWM", GlassRenderer::PrintHwmCmd, &PrintHwmCmd_VAR);
    this->drawBBox = _Dvar_RegisterBool("drawGlassBBox", 0, 0x80u, "Draw glass bounding box");
    this->drawDebug = _Dvar_RegisterBool("drawGlassDebug", 0, 0x80u, "Draw glass client debug");
    this->cullShards = _Dvar_RegisterBool("cullGlassShards", 1, 0x80u, "Enable glass shards culling (draw all if false)");
    this->drawShardOutline = _Dvar_RegisterInt(
                                                         "drawShardOutline",
                                                         0,
                                                         0,
                                                         2,
                                                         0x80u,
                                                         "draw shard outline (0-off, 1-on, 2-w/text)");
    this->pickShards = _Dvar_RegisterBool("pickShards", 0, 0x80u, "pick a shard");
    this->debugSplit = _Dvar_RegisterBool("debugSplit", 0, 0x80u, "print shard split debug info");
    this->freezeShards = _Dvar_RegisterBool("freezeShards", 0, 0x80u, "Freeze the shards after the shatter");
    this->broom = _Dvar_RegisterBool("glassBroom", 0, 0x80u, "Enable the 'broom'");
    this->maxShardSplit = _Dvar_RegisterInt(
                                                    "maxShardSplit",
                                                    75,
                                                    0,
                                                    400,
                                                    0x1000u,
                                                    "Max number of shards that a glass can be split into");
    this->shardShatterSizeLimitScale = _Dvar_RegisterFloat(
                                                                             "shardShatterSizeLimitScale",
                                                                             0.2,
                                                                             0.0099999998,
                                                                             1.0,
                                                                             0,
                                                                             "Scale of maxShardSize & minShardSize when shattering a shard.");
    this->defragMemory = _Dvar_RegisterBool("defragGlassMemory", 1, 0x80u, "Enable glass memory defragmentation");
    this->defragIndices = _Dvar_RegisterBool("defragGlassIndices", 1, 0x80u, "Enable glass memory defragmentation");
    this->smpGlass = _Dvar_RegisterBool("smpGlass", 0, 0x80u, "Update & generate glass verts in a worker thread");
    this->doMaintenance = _Dvar_RegisterBool("doMaintenance", 1, 0x80u, "Do cleanup of the glass");
    this->shardSplitDir = _Dvar_RegisterVec2(
                                                    "shardSplitDir",
                                                    (30.0),
                                                    (60.0),
                                                    0.0,
                                                    180.0,
                                                    0,
                                                    "Angle range of the first edge of the shard");
    this->shardSplitDir2 = _Dvar_RegisterVec2(
                                                     "shardSplitDir2",
                                                     (10.0),
                                                     (30.0),
                                                     0.0,
                                                     180.0,
                                                     0,
                                                     "Angle range of the next edges of the shard");
    this->shardEdgeSize = _Dvar_RegisterVec2(
                                                    "shardEdgeSize",
                                                    (0.02),
                                                    (0.15000001),
                                                    0.0,
                                                    0.30000001,
                                                    0,
                                                    "Range of the new shard edge length as a fraction of the total shard edges lengths");
    this->forceMultiplier = _Dvar_RegisterFloat(
                                                        "glassForceMultiplier",
                                                        500.0,
                                                        10.0,
                                                        100000.0,
                                                        0,
                                                        "Multiplier of the force to apply to shards.");
    this->forceOriginMult = _Dvar_RegisterFloat(
                                                        "glassForceOriginMult",
                                                        0.5,
                                                        0.0,
                                                        10.0,
                                                        0,
                                                        "Multiplier of the distance of the origin of the shards spread point - the smaller it is, clo"
                                                        "ser to the glass then the shards will spread to the sides. The farther it is the shards will"
                                                        " fly in the same direction.");
    this->forceAttenuation = _Dvar_RegisterFloat(
                                                         "glassForceAttenuation",
                                                         1.0,
                                                         0.0,
                                                         10.0,
                                                         0,
                                                         "Attenuation of the force as the shard is farther from the hit position.");
    this->extraVelocity = _Dvar_RegisterFloat(
                                                    "glassExtraVelocity",
                                                    4.0,
                                                    0.0,
                                                    100.0,
                                                    0,
                                                    "Additional random extra angular velocity to the shards when shattering, and extra linear veloc"
                                                    "ity when cracked glass times out.");
    this->freeBuffersDelay = _Dvar_RegisterInt(
                                                         "freeBuffersDelay",
                                                         1,
                                                         0,
                                                         10,
                                                         0,
                                                         "Frames delay until vertex & index buffers are freed");
    this->lowLodDist = _Dvar_RegisterFloat(
                                             "glassLowLodDist",
                                             750.0,
                                             10.0,
                                             100000.0,
                                             0,
                                             "Adjusted (by fov) distance to switch to low glass lod");
    this->maxShardLife = _Dvar_RegisterFloat(
                                                 "glassMaxShardLife",
                                                 3.0,
                                                 0.0,
                                                 1000.0,
                                                 0,
                                                 "Maximum life time of a shard that is contolled by the glass physics");
    this->timeUntilDropRange = _Dvar_RegisterVec2(
                                                             "timeUntilDropRange",
                                                             (0.25),
                                                             (1.0),
                                                             0.0,
                                                             10.0,
                                                             0,
                                                             "Range of how much time a glass shard will stay on the frame before falling");
    this->narrowShardRatio = _Dvar_RegisterFloat(
                                                         "narrowShardRatio",
                                                         3.0,
                                                         1.0,
                                                         1000.0,
                                                         0,
                                                         "minimum ratio between a shard's local bbox edges or between the shard's bbox area vs the sh"
                                                         "ard's area to cause the shard to be rejected");
    this->shatterFxMaxDist = _Dvar_RegisterFloat(
                                                         "shatterFxMaxDist",
                                                         55.0,
                                                         1.0,
                                                         1000.0,
                                                         0,
                                                         "Distance between FX spawning points along the window edges");
    this->shatterFxMinEdgeLength = _Dvar_RegisterFloat(
                                                                     "shatterFxMinEdgeLength",
                                                                     20.0,
                                                                     1.0,
                                                                     1000.0,
                                                                     0,
                                                                     "Minimum length that the window edge has to be to have a shatter fx spawned on it.");
    this->allBBoxMin[0] = FLT_MAX;
    this->allBBoxMin[1] = FLT_MAX;
    this->allBBoxMin[2] = FLT_MAX;
    this->allBBoxMax[0] = -FLT_MAX;
    this->allBBoxMax[1] = -FLT_MAX;
    this->allBBoxMax[2] = -FLT_MAX;
    for ( i = 0; i < glasses->numGlasses; ++i )
    {
        Vec3Min(this->allBBoxMin, glasses->glasses[i].absmin, this->allBBoxMin);
        Vec3Max(this->allBBoxMax, glasses->glasses[i].absmax, this->allBBoxMax);
    }
    allExtent[0] = this->allBBoxMax[0] - this->allBBoxMin[0];
    allExtent[1] = this->allBBoxMax[1] - this->allBBoxMin[1];
    allExtent[2] = this->allBBoxMax[2] - this->allBBoxMin[2];
    this->allBBoxMin[0] = (float)(-0.5 * allExtent[0]) + this->allBBoxMin[0];
    this->allBBoxMin[1] = (float)(-0.5 * allExtent[1]) + this->allBBoxMin[1];
    this->allBBoxMin[2] = (float)(-0.5 * allExtent[2]) + this->allBBoxMin[2];
    this->allBBoxMax[0] = (float)(0.5 * allExtent[0]) + this->allBBoxMax[0];
    this->allBBoxMax[1] = (float)(0.5 * allExtent[1]) + this->allBBoxMax[1];
    this->allBBoxMax[2] = (float)(0.5 * allExtent[2]) + this->allBBoxMax[2];
    Vec3Max(this->allBBoxMin, s_world.mins, this->allBBoxMin);
    Vec3Min(this->allBBoxMax, s_world.maxs, this->allBBoxMax);
    allExtent[0] = this->allBBoxMax[0] - this->allBBoxMin[0];
    allExtent[1] = this->allBBoxMax[1] - this->allBBoxMin[1];
    allExtent[2] = this->allBBoxMax[2] - this->allBBoxMin[2];
    if ( !strcmp(ui_mapname->current.string, "mp_mountain") )
        MIN_SHARD_GROUP_VOLUME_SIZE = 128.0f;
    else
        MIN_SHARD_GROUP_VOLUME_SIZE = 32.0f;
    scale[0] = 0.00048828125f;
    scale[1] = 0.00048828125f;
    scale[2] = 0.0009765625f;
    gridSize[0] = 1.1 * (float)(allExtent[0] * 0.00048828125);
    gridSize[1] = 1.1 * (float)(allExtent[1] * 0.00048828125);
    gridSize[2] = 1.1 * (float)(allExtent[2] * 0.0009765625);
    minGridSize[0] = MIN_SHARD_GROUP_VOLUME_SIZE;
    minGridSize[1] = MIN_SHARD_GROUP_VOLUME_SIZE;
    minGridSize[2] = MIN_SHARD_GROUP_VOLUME_SIZE;
    Vec3Max(gridSize, minGridSize, gridSize);
    this->coordScale[0] = 1.0 / gridSize[0];
    this->coordScale[1] = 1.0 / gridSize[1];
    this->coordScale[2] = 1.0 / gridSize[2];
    GlassRenderer::Reset();
}

GlassRenderer::~GlassRenderer()
{
    unsigned __int8 i; // [esp+27h] [ebp-1h]

    GlassesClient::Free((char *)this->groupsAllocator->memory);
    GlassesClient::Free((char *)this->groupsAllocator);
    GlassesClient::Free((char *)this->shardsAllocator->memory);
    GlassesClient::Free((char *)this->shardsAllocator);
    GlassesClient::Free((char *)this->physicsAllocator->memory);
    GlassesClient::Free((char *)this->physicsAllocator);
    GlassesClient::Free((char *)this->smallAllocator.memory);
    GlassesClient::Free((char *)this->colidingShards);
    GlassesClient::Free((char *)this->tempShardsList);
    GlassesClient::Free((char *)this->shardMemoryAllocator.buffer);
    for ( i = 0; i < 0x16u; ++i )
        GlassesClient::Free((char *)this->vertexList[i]);
    R_FreeGlassRenderBuffers();
}

#if 0
void GlassRenderer::Reset()
{
    GlassPhysics **v1; // eax
    std::_List_nod<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Node *Myhead; // [esp+Ch] [ebp-3Ch]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> > *p_used; // [esp+10h] [ebp-38h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> v5; // [esp+1Ch] [ebp-2Ch]
    GlassLock *p_rendererLock; // [esp+2Ch] [ebp-1Ch]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> _Right; // [esp+30h] [ebp-18h] BYREF
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> v8; // [esp+38h] [ebp-10h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> grp; // [esp+40h] [ebp-8h] BYREF

    p_rendererLock = &this->rendererLock;
    do
    {
        while ( p_rendererLock->lock )
            ;
    }
    while ( _InterlockedCompareExchange(&p_rendererLock->lock, 1, 0) );
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::begin(
        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)&this->groupsAllocator->used,
        (std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> *)&grp);
    while ( 1 )
    {
        p_used = &this->groupsAllocator->used;
        Myhead = this->groupsAllocator->used._Myhead;
        _Right._Myaux = 0;
        _Right._Ptr = (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)Myhead;
        if ( !p_used )
            _invalid_parameter_noinfo();
        _Right._Myaux = p_used->_Myownedaux;
        if ( !std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator!=(
                        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp,
                        &_Right) )
            break;
        v1 = std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp);
        ShardGroup::Reset((ShardGroup *)*v1);
        v5 = grp;
        std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::_Const_iterator<1>::operator++((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp);
        v8 = v5;
    }
    FixedSizeAllocator<GlassShard *>::FreeAll(this->groupsAllocator);
    FixedSizeAllocator<GlassShard *>::FreeAll((FixedSizeAllocator<ShardGroup *> *)this->shardsAllocator);
    Allocator::FreeAll(&this->shardMemoryAllocator);
    this->numGroupChanges = 0;
    this->maxNumGroupChanges = 0;
    this->actionInputIndex = 0;
    this->actionOutputIndex = 0;
    memset(this->actions, 0, sizeof(this->actions));
    this->numUsedMaterials = 0;
    this->numShatters = 0;
    LODWORD(this->shatterTimer) = 0;
    HIDWORD(this->shatterTimer) = 0;
    this->numSplits = 0;
    LODWORD(this->splitTimer) = 0;
    HIDWORD(this->splitTimer) = 0;
    LODWORD(this->triangulateTimer) = 0;
    HIDWORD(this->triangulateTimer) = 0;
    this->genVertsCount = 0;
    LODWORD(this->genVertsTimer) = 0;
    HIDWORD(this->genVertsTimer) = 0;
    this->maxCrashShards = 0;
    GlassShard::removeReasonsCount[0] = 0;
    dword_A707094 = 0;
    dword_A707098 = 0;
    dword_A70709C = 0;
    dword_A7070A0 = 0;
    dword_A7070A4 = 0;
    dword_A7070A8 = 0;
    this->rendererLock.lock = 0;
}
#endif
void GlassRenderer::Reset()
{
    // Acquire spin lock
    while (rendererLock.lock ||
        _InterlockedCompareExchange(&rendererLock.lock, 1, 0))
    {
        /* spin */
    }

    // Reset all shard groups
    for (ShardGroup *group : groupsAllocator->used)
    {
        group->Reset();
    }

    // Free all allocator memory
    groupsAllocator->FreeAll();
    shardsAllocator->FreeAll();
    shardMemoryAllocator.FreeAll();

    // Reset counters and indices
    numGroupChanges = 0;
    maxNumGroupChanges = 0;
    actionInputIndex = 0;
    actionOutputIndex = 0;
    numUsedMaterials = 0;
    numShatters = 0;
    numSplits = 0;
    genVertsCount = 0;
    maxCrashShards = 0;

    // Clear action buffer
    memset(actions, 0, sizeof(actions));

    // Reset timers
    shatterTimer = 0;
    splitTimer = 0;
    triangulateTimer = 0;
    genVertsTimer = 0;

    // Reset global shard removal stats
    for (int i = 0; i < 7; ++i)
    {
        GlassShard::removeReasonsCount[i] = 0;
    }

    // Release lock
    rendererLock.lock = 0;
}

void __thiscall GlassRenderer::InitShardMeshVertexLists()
{
    unsigned __int8 numVerts; // [esp+6h] [ebp-2h]
    unsigned __int8 i; // [esp+7h] [ebp-1h]

    for ( i = 0; i < 0x16u; ++i )
    {
        numVerts = i + 1;
        if ( (unsigned __int8)(i + 1) <= 2u )
        {
            this->vertexList[i] = 0;
        }
        else
        {
            this->vertexList[i] = (GlassShardMeshVertex *)GlassesClient::Allocate(
                                                                                                            12 * numVerts,
                                                                                                            "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp",
                                                                                                            271);
            GlassShard::Mesh::InitVertexList(numVerts, this->vertexList[i]);
        }
    }
}

#if 0
void __thiscall GlassRenderer::RemoveGlassShards(unsigned int glassIndex)
{
    GlassPhysics **v2; // eax
    std::_List_nod<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Node *Myhead; // [esp+Ch] [ebp-3Ch]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> > *p_used; // [esp+10h] [ebp-38h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> v6; // [esp+1Ch] [ebp-2Ch]
    GlassLock *p_rendererLock; // [esp+2Ch] [ebp-1Ch]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> _Right; // [esp+30h] [ebp-18h] BYREF
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> v9; // [esp+38h] [ebp-10h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> grp; // [esp+40h] [ebp-8h] BYREF

    p_rendererLock = &this->rendererLock;
    do
    {
        while ( p_rendererLock->lock )
            ;
    }
    while ( _InterlockedCompareExchange(&p_rendererLock->lock, 1, 0) );
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::begin(
        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)&this->groupsAllocator->used,
        (std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> *)&grp);
    while ( 1 )
    {
        p_used = &this->groupsAllocator->used;
        Myhead = this->groupsAllocator->used._Myhead;
        _Right._Myaux = 0;
        _Right._Ptr = (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)Myhead;
        if ( !p_used )
            _invalid_parameter_noinfo();
        _Right._Myaux = p_used->_Myownedaux;
        if ( !std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator!=(
                        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp,
                        &_Right) )
            break;
        v2 = std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp);
        ShardGroup::RemoveGlassShards((ShardGroup *)*v2, glassIndex);
        v6 = grp;
        std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::_Const_iterator<1>::operator++((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp);
        v9 = v6;
    }
    GlassRenderer::DoGroupChanges(this);
    this->rendererLock.lock = 0;
}
#endif
void GlassRenderer::RemoveGlassShards(unsigned int glassIndex)
{
    // Acquire spin lock
    while (rendererLock.lock ||
           _InterlockedCompareExchange(&rendererLock.lock, 1, 0))
    {
        /* spin */
    }

    // Remove shards from all active groups
    for (ShardGroup* group : groupsAllocator->used)
    {
        group->RemoveGlassShards(glassIndex);
    }

    // Apply pending structural changes
    DoGroupChanges();

    // Release lock
    rendererLock.lock = 0;
}

GlassShard *__thiscall GlassRenderer::AllocShard()
{
    GlassShard *shard; // [esp+4h] [ebp-4h]

    //shard = (GlassShard *)FixedSizeAllocator<ShardGroup *>::Allocate((FixedSizeAllocator<GlassPhysics *> *)this->shardsAllocator);
    shard = this->shardsAllocator->Allocate();
    if ( shard )
        shard->Init();
    return shard;
}

void __thiscall GlassRenderer::FreeShard(GlassShard *shard)
{
    if ( shard->group
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp", 305, 0, "%s", "!shard->group") )
    {
        __debugbreak();
    }
    shard->Destroy();
    this->shardsAllocator->Free(shard);
    //FixedSizeAllocator<GlassShard *>::Free(
    //    (FixedSizeAllocator<GlassPhysics *> *)this->shardsAllocator,
    //    (GlassPhysics *)shard);
}

GlassPhysics *__thiscall GlassRenderer::AllocPhysics()
{
    //return FixedSizeAllocator<ShardGroup *>::Allocate(this->physicsAllocator);
    return this->physicsAllocator->Allocate();
}

void __thiscall GlassRenderer::FreePhysics(GlassPhysics *phys)
{
    //FixedSizeAllocator<GlassShard *>::Free(this->physicsAllocator, phys);
    this->physicsAllocator->Free(phys);
}

// aislopped and chopped
#if 0
void __thiscall GlassRenderer::RemovePhysicsShards()
{
    GlassShard **v2; // [esp+4h] [ebp-88h]
    GlassRenderer::SortedShardsList *tempShardsList; // [esp+Ch] [ebp-80h]
    std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *Myhead; // [esp+10h] [ebp-7Ch]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Iterator<1> v5; // [esp+1Ch] [ebp-70h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> v6; // [esp+34h] [ebp-58h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> v7; // [esp+54h] [ebp-38h] BYREF
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Iterator<1> v8; // [esp+5Ch] [ebp-30h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> _Right; // [esp+64h] [ebp-28h] BYREF
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> v10; // [esp+6Ch] [ebp-20h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Iterator<1> result; // [esp+74h] [ebp-18h] BYREF
    GlassShard *shard; // [esp+7Ch] [ebp-10h]
    ShardGroup *grp; // [esp+80h] [ebp-Ch]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> g; // [esp+84h] [ebp-8h] BYREF

    GlassRenderer::DoGroupChanges(this);
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::clear((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this->tempShardsList);
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::_Const_iterator<1>(
        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&g,
        (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->groupsAllocator->used._Myhead->_Next,
        (const std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)&this->groupsAllocator->used);
    while ( 1 )
    {
        std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::_Const_iterator<1>(
            &_Right,
            (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->groupsAllocator->used._Myhead,
            (const std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)&this->groupsAllocator->used);
        if ( !std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator!=(
                        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&g,
                        &_Right) )
            break;
        grp = (ShardGroup *)*std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&g);
        for ( shard = grp->head; shard; shard = shard->groupNext )
        {
            if ( shard->glassPhysics )
                GlassRenderer::SortedShardsList::Insert(this->tempShardsList, shard);
        }
        v6 = g;
        std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::_Const_iterator<1>::operator++((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&g);
        v10 = v6;
    }
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::begin(
        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this->tempShardsList,
        (std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> *)&result);
    while ( 1 )
    {
        tempShardsList = this->tempShardsList;
        Myhead = (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)tempShardsList->_Myhead;
        v7._Myaux = 0;
        v7._Ptr = Myhead;
        if ( !tempShardsList )
            _invalid_parameter_noinfo();
        v7._Myaux = tempShardsList->_Myownedaux;
        if ( !std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator!=(&result, &v7) )
            break;
        v2 = (GlassShard **)std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*(&result);
        GlassShard::Remove(*v2, NUM_REMOVE_REASONS, 0);
        v5 = result;
        std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::_Const_iterator<1>::operator++(&result);
        v8 = v5;
    }
}
#endif

void GlassRenderer::RemovePhysicsShards()
{
    // First, process any pending group changes
    DoGroupChanges();

    // Clear the temporary shards list
    tempShardsList->clear();

    // Iterate over all shard groups
    for (std::list<ShardGroup *, SmallAllocatorTemplate<ShardGroup *> >::iterator grpIt = groupsAllocator->used.begin();
        grpIt != groupsAllocator->used.end();
        ++grpIt)
    {
        ShardGroup &grp = **grpIt;

        // Iterate over all shards in this group
        for (GlassShard *shard = grp.head; shard; shard = shard->groupNext)
        {
            if (shard->glassPhysics)
            {
                tempShardsList->Insert(shard);
            }
        }
    }

    // Now remove all shards in the tempShardsList
    while (!tempShardsList->empty())
    {
        GlassShard *shard = tempShardsList->front();
        tempShardsList->pop_front();

        //GlassShard::Remove(shard, NUM_REMOVE_REASONS, 0);
        shard->Remove(GlassShard::RemoveReason::NUM_REMOVE_REASONS, 0);
    }
}

void __thiscall GlassRenderer::AddGroupChange(GlassShard *shard)
{
    if ( !shard->inGroupChange )
    {
        if ( this->numGroupChanges >= 0x800u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp",
                        345,
                        0,
                        "%s",
                        "numGroupChanges < ARRAY_COUNT(groupChanges)") )
        {
            __debugbreak();
        }
        if ( this->numGroupChanges < 0x800u )
        {
            this->groupChanges[this->numGroupChanges++] = shard;
            shard->inGroupChange = 1;
        }
    }
}

void __thiscall GlassRenderer::DoGroupChanges()
{
    int numGroupChanges; // [esp+0h] [ebp-14h]
    int i; // [esp+10h] [ebp-4h]

    for ( i = 0; i < this->numGroupChanges; ++i )
        this->groupChanges[i]->ChangeGroup();
    if ( this->maxNumGroupChanges < this->numGroupChanges )
        numGroupChanges = this->numGroupChanges;
    else
        numGroupChanges = this->maxNumGroupChanges;
    this->maxNumGroupChanges = numGroupChanges;
    this->numGroupChanges = 0;
}

#if 0
void __thiscall GlassRenderer::Update(int threadId)
{
    GlassPhysics **v2; // eax
    float deltaTime; // [esp+0h] [ebp-78h]
    std::_List_nod<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Node *Myhead; // [esp+20h] [ebp-58h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> > *p_used; // [esp+24h] [ebp-54h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> v7; // [esp+28h] [ebp-50h]
    float v8; // [esp+38h] [ebp-40h]
    float v9; // [esp+3Ch] [ebp-3Ch]
    GlassLock *p_rendererLock; // [esp+48h] [ebp-30h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> _Right; // [esp+4Ch] [ebp-2Ch] BYREF
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> v12; // [esp+54h] [ebp-24h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> grp; // [esp+5Ch] [ebp-1Ch] BYREF
    cg_s *cgameGlob; // [esp+64h] [ebp-14h]
    int now; // [esp+68h] [ebp-10h]
    int i; // [esp+6Ch] [ebp-Ch]
    cg_s *cg; // [esp+70h] [ebp-8h]

    if ( threadId
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp", 386, 0, "%s", "threadId == 0") )
    {
        __debugbreak();
    }
    //PIXBeginNamedEvent(-1, "Glasses Update");
    GlassRenderer::CrashGlass(this);
    GlassRenderer::Broom(this);
    p_rendererLock = &this->rendererLock;
    do
    {
        while ( p_rendererLock->lock )
            ;
    }
    while ( _InterlockedCompareExchange(&p_rendererLock->lock, 1, 0) );
    if ( !threadId )
    {
        ++this->frame;
        this->deltaTime = 0.016666668f;
        cg = CG_GetLocalClientGlobals(0);
        if ( cg && cg->snap )
        {
            for ( i = 0; i < 1; ++i )
            {
                if ( CL_LocalClient_IsActive(i) && CL_GetLocalClientConnectionState(i) >= 4 )
                {
                    cgameGlob = CG_GetLocalClientGlobals(i);
                    if ( !cgameGlob )
                    {
                        //if ( g_DXDeviceThread == GetCurrentThreadId() )
                            //D3DPERF_EndEvent();
                        return;
                    }
                    now = cgameGlob->physicsTime;
                    v9 = (float)(now - this->timeLastUpdate) * 0.001;
                    if ( 0.1 <= 0.0
                        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 730, 0, "%s", "min < max") )
                    {
                        __debugbreak();
                    }
                    if ( v9 >= 0.0 )
                    {
                        if ( v9 <= 0.1 )
                            v8 = v9;
                        else
                            v8 = 0.1f;
                    }
                    else
                    {
                        v8 = 0.0f;
                    }
                    this->deltaTime = v8;
                    this->timeLastUpdate = now;
                    break;
                }
            }
        }
        GlassRenderer::DoGroupChanges(this);
        GlassRenderer::ExecuteActions(this);
        this->prevStat.numMovingShards = this->stat.numMovingShards;
        this->prevStat.numVisGroups = this->stat.numVisGroups;
        this->prevStat.numVisShards = this->stat.numVisShards;
        this->prevStat.numOOMGroups = this->stat.numOOMGroups;
        this->stat.numVisGroups = 0;
        this->stat.numVisShards = 0;
        this->stat.numOOMGroups = 0;
        this->stat.numMovingShards = 0;
    }
    //PIXBeginNamedEvent(0, "ShardGroup.Update");
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::_Const_iterator<1>(
        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp,
        (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->groupsAllocator->used._Myhead->_Next,
        (const std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)&this->groupsAllocator->used);
    while ( 1 )
    {
        p_used = &this->groupsAllocator->used;
        Myhead = this->groupsAllocator->used._Myhead;
        _Right._Myaux = 0;
        _Right._Ptr = (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)Myhead;
        if ( !p_used )
            _invalid_parameter_noinfo();
        _Right._Myaux = p_used->_Myownedaux;
        if ( !std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator!=(
                        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp,
                        &_Right) )
            break;
        deltaTime = this->deltaTime;
        v2 = std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp);
        ShardGroup::Update((ShardGroup *)*v2, deltaTime);
        v7 = grp;
        std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::_Const_iterator<1>::operator++((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp);
        v12 = v7;
    }
    //if ( GetCurrentThreadId() == g_DXDeviceThread )
        //D3DPERF_EndEvent();
    if ( !threadId )
        GlassRenderer::DoGroupChanges(this);
    this->rendererLock.lock = 0;
    //if ( g_DXDeviceThread == GetCurrentThreadId() )
        //D3DPERF_EndEvent();
}
#endif

// aislop
void GlassRenderer::Update(int threadId)
{
    iassert(threadId == 0);

    PROF_SCOPED("Glasses Update");


    CrashGlass();
    Broom();

    // Acquire renderer spin lock
    while (_InterlockedCompareExchange(&rendererLock.lock, 1, 0))
        ;

    if (!threadId)
    {
        ++frame;
        deltaTime = 1.0f / 60.0f; // 0.016666668f

        cg_s *cg = CG_GetLocalClientGlobals(0);
        if (cg && cg->snap)
        {
            for (int i = 0; i < 1; ++i)
            {
                if (!CL_LocalClient_IsActive(i))
                    continue;
                if (CL_GetLocalClientConnectionState(i) < 4)
                    continue;

                cg_s *cgameGlob = CG_GetLocalClientGlobals(i);
                if (!cgameGlob)
                {
                    return;
                }

                int now = cgameGlob->physicsTime;
                float dt = (float)(now - timeLastUpdate) * 0.001f;

                // Clamp to [0, 0.1]
                dt = (dt < 0.0f) ? 0.0f : (dt > 0.1f) ? 0.1f : dt;

                deltaTime = dt;
                timeLastUpdate = now;
                break;
            }
        }

        DoGroupChanges();
        ExecuteActions();

        // prevStat = stat (4 fields copied individually)
        prevStat.numMovingShards = stat.numMovingShards;
        prevStat.numVisGroups = stat.numVisGroups;
        prevStat.numVisShards = stat.numVisShards;
        prevStat.numOOMGroups = stat.numOOMGroups;

        stat.numMovingShards = 0;
        stat.numVisGroups = 0;
        stat.numVisShards = 0;
        stat.numOOMGroups = 0;
    }

    {
        PROF_SCOPED("ShardGroup.Update");

        for (ShardGroup *grp : groupsAllocator->used)
            grp->Update(deltaTime);
    }

    if (!threadId)
        DoGroupChanges();

    rendererLock.lock = 0;
}

#if 0
void __thiscall GlassRenderer::GenerateVerts(
                int localClientNum,
                unsigned int viewIndex,
                unsigned int threadId)
{
    GlassPhysics **v4; // eax
    bool v5; // [esp-8h] [ebp-5Ch]
    bool v6; // [esp+0h] [ebp-54h]
    std::_List_nod<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Node *Myhead; // [esp+18h] [ebp-3Ch]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> > *p_used; // [esp+1Ch] [ebp-38h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> v10; // [esp+20h] [ebp-34h]
    GlassLock *p_rendererLock; // [esp+30h] [ebp-24h]
    char *Name; // [esp+34h] [ebp-20h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> _Right; // [esp+38h] [ebp-1Ch] BYREF
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> v14; // [esp+40h] [ebp-14h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> grp; // [esp+48h] [ebp-Ch] BYREF
    bool firstView; // [esp+53h] [ebp-1h]

    if ( threadId
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp", 472, 0, "%s", "threadId == 0") )
    {
        __debugbreak();
    }
    Name = va("Glass.GenVerts(c=%d v=%d t=%d)", localClientNum, viewIndex, threadId);
    //PIXBeginNamedEvent(-1, Name);
    p_rendererLock = &this->rendererLock;
    do
    {
        while ( p_rendererLock->lock )
            ;
    }
    while ( _InterlockedCompareExchange(&p_rendererLock->lock, 1, 0) );
    v6 = CL_LocalClient_IsFirstActive(localClientNum) && !viewIndex;
    firstView = v6;
    if ( v6 )
        this->numUsedMaterials = 0;
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::begin(
        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)&this->groupsAllocator->used,
        (std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> *)&grp);
    while ( 1 )
    {
        p_used = &this->groupsAllocator->used;
        Myhead = this->groupsAllocator->used._Myhead;
        _Right._Myaux = 0;
        _Right._Ptr = (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)Myhead;
        if ( !p_used )
            _invalid_parameter_noinfo();
        _Right._Myaux = p_used->_Myownedaux;
        if ( !std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator!=(
                        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp,
                        &_Right) )
            break;
        v5 = firstView;
        v4 = std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp);
        ShardGroup::GenerateVerts((ShardGroup *)*v4, v5, localClientNum);
        v10 = grp;
        std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::_Const_iterator<1>::operator++((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp);
        v14 = v10;
    }
    if (!threadId)
    {
        //BLOPS_NULLSUB();
    }
    this->rendererLock.lock = 0;
    GlassRenderer::StartMaintenance();
    //if ( g_DXDeviceThread == GetCurrentThreadId() )
        //D3DPERF_EndEvent();
}
#endif

void GlassRenderer::GenerateVerts(int localClientNum, unsigned int viewIndex, unsigned int threadId)
{
    if (threadId && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp", 472, 0, "threadId == 0"))
        __debugbreak();


    char *Name = va("Glass.GenVerts(c=%d v=%d t=%d)", localClientNum, viewIndex, threadId);
    PROF_SCOPED_RUNTIME_NAME(Name);


    // Acquire renderer lock
    while (_InterlockedCompareExchange(&rendererLock.lock, 1, 0))
        ;


    // Determine if this is the first view
    bool firstView = CL_LocalClient_IsFirstActive(localClientNum) && (viewIndex == 0);
    if (firstView)
        numUsedMaterials = 0;


    // Iterate all shard groups
    for (std::list<ShardGroup *, SmallAllocatorTemplate<ShardGroup *> >::iterator it = groupsAllocator->used.begin();
        it != groupsAllocator->used.end();
        ++it)
    {
        ShardGroup *grp = *it;
        grp->GenerateVerts(firstView, localClientNum);
    }

    // Release renderer lock
    rendererLock.lock = 0;

    // Start maintenance tasks
    StartMaintenance();
}

#if 0
void __thiscall GlassRenderer::ExplosionEvent(
                const float *origin,
                float damageInner,
                float damageOuter,
                float radius,
                int mod)
{
    GlassPhysics **v6; // eax
    std::_List_nod<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Node *Myhead; // [esp+20h] [ebp-3Ch]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> > *p_used; // [esp+24h] [ebp-38h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> v10; // [esp+30h] [ebp-2Ch]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> _Right; // [esp+40h] [ebp-1Ch] BYREF
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> v12; // [esp+48h] [ebp-14h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> grp; // [esp+50h] [ebp-Ch] BYREF

    //PIXBeginNamedEvent(-1, "GlassRenderer.ExplosionEvent");
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::begin(
        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)&this->groupsAllocator->used,
        (std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> *)&grp);
    while ( 1 )
    {
        p_used = &this->groupsAllocator->used;
        Myhead = this->groupsAllocator->used._Myhead;
        _Right._Myaux = 0;
        _Right._Ptr = (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)Myhead;
        if ( !p_used )
            _invalid_parameter_noinfo();
        _Right._Myaux = p_used->_Myownedaux;
        if ( !std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator!=(
                        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp,
                        &_Right) )
            break;
        v6 = std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp);
        ShardGroup::ExplosionEvent((ShardGroup *)*v6, origin, damageInner, damageOuter, radius, mod);
        v10 = grp;
        std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::_Const_iterator<1>::operator++((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp);
        v12 = v10;
    }
    //if ( g_DXDeviceThread == GetCurrentThreadId() )
        //D3DPERF_EndEvent();
}
#endif

void GlassRenderer::ExplosionEvent(
    const float *origin,
    float damageInner,
    float damageOuter,
    float radius,
    int mod)
{
    PROF_SCOPED("GlassRenderer.ExplosionEvent");

    // Iterate over all shard groups
    for (std::list<ShardGroup *, SmallAllocatorTemplate<ShardGroup *> >::iterator it = groupsAllocator->used.begin();
        it != groupsAllocator->used.end();
        ++it)
    {
        ShardGroup *grp = *it;
        grp->ExplosionEvent(origin, damageInner, damageOuter, radius, mod);
    }
}

#if 0
int __thiscall GlassRenderer::TracePoint(float *p0, const float *p1)
{
    GlassPhysics **v3; // eax
    int v4; // eax
    std::_List_nod<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Node *Myhead; // [esp+10h] [ebp-44h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> > *p_used; // [esp+14h] [ebp-40h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> v9; // [esp+20h] [ebp-34h]
    int v10; // [esp+30h] [ebp-24h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> _Right; // [esp+34h] [ebp-20h] BYREF
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> v12; // [esp+3Ch] [ebp-18h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> grp; // [esp+44h] [ebp-10h] BYREF
    int numHits; // [esp+50h] [ebp-4h]

    //PIXBeginNamedEvent(-1, "GlassRenderer.TracePoint");
    numHits = 0;
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::begin(
        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)&this->groupsAllocator->used,
        (std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> *)&grp);
    while ( 1 )
    {
        p_used = &this->groupsAllocator->used;
        Myhead = this->groupsAllocator->used._Myhead;
        _Right._Myaux = 0;
        _Right._Ptr = (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)Myhead;
        if ( !p_used )
            _invalid_parameter_noinfo();
        _Right._Myaux = p_used->_Myownedaux;
        if ( !std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator!=(
                        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp,
                        &_Right) )
            break;
        v3 = std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp);
        v4 = ShardGroup::TracePoint((ShardGroup *)*v3, p0, p1);
        numHits += v4;
        v9 = grp;
        std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::_Const_iterator<1>::operator++((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp);
        v12 = v9;
    }
    v10 = numHits;
    //if ( g_DXDeviceThread == GetCurrentThreadId() )
        //D3DPERF_EndEvent();
    return v10;
}
#endif

// aislop
int GlassRenderer::TracePoint(float *p0, const float *p1)
{
    int numHits = 0;

    // Iterate over all ShardGroups
    for (ShardGroup *grp : groupsAllocator->used)
    {
        numHits += grp->TracePoint(p0, p1);
    }

    return numHits;
}


void __thiscall GlassRenderer::Melee(
                float *eyePos,
                const float *forward,
                const float *right,
                const float *up,
                float range)
{
    float v8; // [esp+4h] [ebp-18h]
    float v9; // [esp+8h] [ebp-14h]
    unsigned int traceIndex; // [esp+Ch] [ebp-10h]
    float end[3]; // [esp+10h] [ebp-Ch] BYREF

    for (traceIndex = 0; traceIndex < 5; ++traceIndex)
    {
        end[0] = (float)(range * *forward) + *eyePos;
        end[1] = (float)(range * forward[1]) + eyePos[1];
        end[2] = (float)(range * forward[2]) + eyePos[2];
        v9 = player_meleeWidth->current.value * (float)traceOffsets_0[traceIndex][0];
        end[0] = (float)(v9 * *right) + end[0];
        end[1] = (float)(v9 * right[1]) + end[1];
        end[2] = (float)(v9 * right[2]) + end[2];
        v8 = player_meleeHeight->current.value * (float)traceOffsets_0[traceIndex][1];
        end[0] = (float)(v8 * *up) + end[0];
        end[1] = (float)(v8 * up[1]) + end[1];
        end[2] = (float)(v8 * up[2]) + end[2];
        if (GlassRenderer::TracePoint(eyePos, end) > 0)
            break;
    }
}

unsigned int __thiscall GlassRenderer::CalcPackedPos(const float *pos)
{
    float p[3]; // [esp+74h] [ebp-Ch] BYREF

    Vec3Max(pos, this->allBBoxMin, p);
    Vec3Min(p, this->allBBoxMax, p);
    p[0] = p[0] - this->allBBoxMin[0];
    p[1] = p[1] - this->allBBoxMin[1];
    p[2] = p[2] - this->allBBoxMin[2];
    p[0] = p[0] * this->coordScale[0];
    p[1] = p[1] * this->coordScale[1];
    p[2] = p[2] * this->coordScale[2];
    if ( (unsigned int)(__int64)p[0] >= 0x800
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp",
                    627,
                    0,
                    "%s\n\t(p[0]) = %g",
                    "(uint(p[0]) < (1<<COORD_SCALE_BITS_X))",
                    p[0]) )
    {
        __debugbreak();
    }
    if ( (unsigned int)(__int64)p[1] >= 0x800
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp",
                    628,
                    0,
                    "%s\n\t(p[1]) = %g",
                    "(uint(p[1]) < (1<<COORD_SCALE_BITS_Y))",
                    p[1]) )
    {
        __debugbreak();
    }
    if ( (unsigned int)(__int64)p[2] >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp",
                    629,
                    0,
                    "%s\n\t(p[2]) = %g",
                    "(uint(p[2]) < (1<<COORD_SCALE_BITS_Z))",
                    p[2]) )
    {
        __debugbreak();
    }
    return ((__int64)p[2] << 22) | ((__int64)p[1] << 11) | (__int64)p[0];
}

GlassPhysics *__thiscall GlassRenderer::GetShardGroup(const float *pos, const GlassDef *glassDef)
{
    unsigned int v3; // eax

    v3 = GlassRenderer::CalcPackedPos(pos);
    return this->GetShardGroup(v3, glassDef);
}

#if 0
GlassPhysics *__thiscall GlassRenderer::GetShardGroup(
                unsigned int packedPos,
                const GlassDef *glassDef)
{
    std::_List_nod<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Node *Myhead; // [esp+10h] [ebp-34h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> > *p_used; // [esp+14h] [ebp-30h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Iterator<1> v7; // [esp+18h] [ebp-2Ch]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> _Right; // [esp+28h] [ebp-1Ch] BYREF
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Iterator<1> v9; // [esp+30h] [ebp-14h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Iterator<1> result; // [esp+38h] [ebp-Ch] BYREF
    ShardGroup *grp; // [esp+40h] [ebp-4h]

    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::begin(
        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)&this->groupsAllocator->used,
        (std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> *)&result);
    while ( 1 )
    {
        p_used = &this->groupsAllocator->used;
        Myhead = this->groupsAllocator->used._Myhead;
        _Right._Myaux = 0;
        _Right._Ptr = (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)Myhead;
        if ( !p_used )
            _invalid_parameter_noinfo();
        _Right._Myaux = p_used->_Myownedaux;
        if ( !std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator!=(&result, &_Right) )
            break;
        if ( LODWORD((*std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*(&result))->m_mat.x.x) == packedPos
            && (const GlassDef *)LODWORD((*std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*(&result))->m_mat.x.y) == glassDef )
        {
            return *std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*(&result);
        }
        v7 = result;
        std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::_Const_iterator<1>::operator++(&result);
        v9 = v7;
    }
    grp = (ShardGroup *)FixedSizeAllocator<ShardGroup *>::Allocate((FixedSizeAllocator<GlassPhysics *> *)this->groupsAllocator);
    if ( grp )
        ShardGroup::Init(grp, packedPos, glassDef);
    return (GlassPhysics *)grp;
}
#endif

GlassPhysics *GlassRenderer::GetShardGroup(
    unsigned int packedPos,
    const GlassDef *glassDef)
{
    // First, check if a group already exists for this position and GlassDef
    for (ShardGroup *grp : groupsAllocator->used)
    {
        if (grp->packedPos == packedPos && grp->glassDef == glassDef)
        {
            return (GlassPhysics *)grp;
        }
    }

    ShardGroup *grp = groupsAllocator->Allocate();
    if (!grp)
        return nullptr;


    // Initialize the new group
    grp->Init(packedPos, glassDef);


    return (GlassPhysics *)grp;
}

void __thiscall GlassRenderer::FreeShardGroup(ShardGroup *grp)
{
    //FixedSizeAllocator<GlassShard *>::Free((FixedSizeAllocator<GlassPhysics *> *)this->groupsAllocator, grp);
    this->groupsAllocator->Free(grp);
}

Allocator::Memory **__thiscall GlassRenderer::AllocateShardMemory(
                unsigned int size,
                GlassShard *shard)
{
    //return Allocator::Allocate(&this->shardMemoryAllocator, size, shard);
    return shardMemoryAllocator.Allocate(size, shard);
}

void __thiscall GlassRenderer::FreeShardMemory(unsigned int *ptr)
{
    //Allocator::Free(&this->shardMemoryAllocator, ptr);
    this->shardMemoryAllocator.Free(ptr);
}

unsigned int __thiscall GlassRenderer::ShardMemorySize(unsigned int *ptr)
{
    //return Allocator::GetMemorySize(&this->shardMemoryAllocator, ptr);
    return this->shardMemoryAllocator.GetMemorySize(ptr);
}

#if 0
void __thiscall GlassRenderer::SortedShardsList::Insert(GlassRenderer::SortedShardsList *this, GlassShard *shard)
{
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> v2; // [esp-Ch] [ebp-15Ch]
    std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *Myhead; // [esp+5Ch] [ebp-F4h]
    float v5; // [esp+BCh] [ebp-94h]
    GlassPhysics *v6; // [esp+C0h] [ebp-90h]
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> v7; // [esp+D0h] [ebp-80h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> v8; // [esp+F0h] [ebp-60h] BYREF
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> v9; // [esp+F8h] [ebp-58h]
    GlassShard *v10; // [esp+100h] [ebp-50h]
    float v11; // [esp+104h] [ebp-4Ch]
    GlassPhysics *v12; // [esp+108h] [ebp-48h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> v13; // [esp+120h] [ebp-30h] BYREF
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> _Where; // [esp+128h] [ebp-28h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> v15; // [esp+130h] [ebp-20h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> _Right; // [esp+138h] [ebp-18h] BYREF
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> v17; // [esp+140h] [ebp-10h]
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> iter; // [esp+148h] [ebp-8h] BYREF

    if ( this->_Mysize )
    {
        v12 = *std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::back(this);
        v11 = GlassShard::Outline::Area((GlassShard::Outline *)&v12->m_mat.x.w);
        v10 = shard;
        if ( GlassShard::Outline::Area(&shard->outline) <= v11 )
        {
            std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::_Const_iterator<1>(
                (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&iter,
                (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->_Myhead->_Next,
                (const std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this);
            while ( 1 )
            {
                std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::_Const_iterator<1>(
                    &_Right,
                    (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->_Myhead,
                    (const std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this);
                if ( !std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator!=(
                                (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&iter,
                                &_Right) )
                    break;
                v6 = *std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&iter);
                v5 = GlassShard::Outline::Area((GlassShard::Outline *)&v6->m_mat.x.w);
                if ( v5 > GlassShard::Outline::Area(&shard->outline) )
                {
                    v15 = (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1>)iter;
                    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *>>::_Insert(
                        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this,
                        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1>)iter,
                        (GlassPhysics **)&shard);
                    return;
                }
                v7 = iter;
                std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::_Const_iterator<1>::operator++((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&iter);
                v17 = v7;
            }
            Myhead = (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->_Myhead;
            if ( !this )
                _invalid_parameter_noinfo();
            v2._Ptr = Myhead;
            v2._Myaux = this->_Myownedaux;
            std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *>>::_Insert(
                (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this,
                v2,
                (GlassPhysics **)&shard);
        }
        else
        {
            std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::_Const_iterator<1>(
                &v8,
                (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->_Myhead,
                (const std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this);
            v9 = v8;
            std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *>>::_Insert(
                (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this,
                v8,
                (GlassPhysics **)&shard);
        }
    }
    else
    {
        std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::_Const_iterator<1>(
            &v13,
            (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->_Myhead->_Next,
            (const std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this);
        _Where = v13;
        std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *>>::_Insert(
            (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this,
            v13,
            (GlassPhysics **)&shard);
    }
}
#endif

void GlassRenderer::SortedShardsList::Insert(GlassShard *shard)
{
    const float shardArea = shard->outline.Area();

    // If the list is empty, just add it
    if (empty())
    {
        push_back(shard);
        return;
    }

    // Compare with the back element's area
    const float backArea = back()->outline.Area();

    if (shardArea <= backArea)
    {
        // Insert somewhere in the middle (or at the end)
        for (auto it = begin(); it != end(); ++it)
        {
            const float curArea = (*it)->outline.Area();

            if (curArea > shardArea)
            {
                insert(it, shard);
                return;
            }
        }

        push_back(shard);
    }
    else
    {
        push_front(shard);
    }
}

#if 0
void __thiscall GlassRenderer::SortedShardsList::InsertReverse(GlassShard *shard)
{
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> v2; // [esp-Ch] [ebp-15Ch]
    std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *Myhead; // [esp+5Ch] [ebp-F4h]
    float v5; // [esp+BCh] [ebp-94h]
    GlassPhysics *v6; // [esp+C0h] [ebp-90h]
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> v7; // [esp+D0h] [ebp-80h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> v8; // [esp+F0h] [ebp-60h] BYREF
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> v9; // [esp+F8h] [ebp-58h]
    GlassShard *v10; // [esp+100h] [ebp-50h]
    float v11; // [esp+104h] [ebp-4Ch]
    GlassPhysics *v12; // [esp+108h] [ebp-48h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> v13; // [esp+120h] [ebp-30h] BYREF
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> _Where; // [esp+128h] [ebp-28h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> v15; // [esp+130h] [ebp-20h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> _Right; // [esp+138h] [ebp-18h] BYREF
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> v17; // [esp+140h] [ebp-10h]
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> iter; // [esp+148h] [ebp-8h] BYREF

    if ( this->_Mysize )
    {
        v12 = *std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::back(this);
        v11 = GlassShard::Outline::Area((GlassShard::Outline *)&v12->m_mat.x.w);
        v10 = shard;
        if ( v11 <= GlassShard::Outline::Area(&shard->outline) )
        {
            std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::_Const_iterator<1>(
                (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&iter,
                (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->_Myhead->_Next,
                (const std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this);
            while ( 1 )
            {
                std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::_Const_iterator<1>(
                    &_Right,
                    (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->_Myhead,
                    (const std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this);
                if ( !std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator!=(
                                (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&iter,
                                &_Right) )
                    break;
                v6 = *std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&iter);
                v5 = GlassShard::Outline::Area((GlassShard::Outline *)&v6->m_mat.x.w);
                if ( GlassShard::Outline::Area(&shard->outline) > v5 )
                {
                    v15 = (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1>)iter;
                    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *>>::_Insert(
                        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this,
                        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1>)iter,
                        (GlassPhysics **)&shard);
                    return;
                }
                v7 = iter;
                std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::_Const_iterator<1>::operator++((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&iter);
                v17 = v7;
            }
            Myhead = (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->_Myhead;
            if ( !this )
                _invalid_parameter_noinfo();
            v2._Ptr = Myhead;
            v2._Myaux = this->_Myownedaux;
            std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *>>::_Insert(
                (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this,
                v2,
                (GlassPhysics **)&shard);
        }
        else
        {
            std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::_Const_iterator<1>(
                &v8,
                (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->_Myhead,
                (const std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this);
            v9 = v8;
            std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *>>::_Insert(
                (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this,
                v8,
                (GlassPhysics **)&shard);
        }
    }
    else
    {
        std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::_Const_iterator<1>(
            &v13,
            (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->_Myhead->_Next,
            (const std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this);
        _Where = v13;
        std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *>>::_Insert(
            (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this,
            v13,
            (GlassPhysics **)&shard);
    }
}
#endif

void GlassRenderer::SortedShardsList::InsertReverse(GlassShard *shard)
{
    const float shardArea = shard->outline.Area();


    // If the list is empty, just add it
    if (empty())
    {
        push_back(shard);
        return;
    }


    // Compare with the back element's area
    const float backArea = back()->outline.Area();


    if (shardArea >= backArea)
    {
        // Insert somewhere in the middle (or at the end)
        for (auto it = begin(); it != end(); ++it)
        {
            const float curArea = (*it)->outline.Area();


            if (shardArea > curArea)
            {
                insert(it, shard);
                return;
            }
        }

        push_back(shard);
    }
    else
    {
        push_front(shard);
    }
}

bool __thiscall GlassRenderer::AddColidingShard(GlassShard *shard)
{
    //GlassRenderer::SortedShardsList::Insert(this->colidingShards, shard);
    this->colidingShards->Insert(shard);
    //return this->colidingShards->_Mysize > 0x64;
    return this->colidingShards->size() > 100;
}

void __thiscall GlassRenderer::RemoveColidingShard(GlassShard *shard)
{
    this->colidingShards->remove(shard);
    //std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::remove(
    //    (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this->colidingShards,
    //    (GlassPhysics **)&shard);
}

GlassShard *__thiscall GlassRenderer::GetSmallestColidingShard()
{
    //return *std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::front(this->colidingShards);
    return this->colidingShards->front();
}

bool __thiscall GlassRenderer::IsVisible(const float *minmax, unsigned int localClientNum)
{
    return !this->cullShards->current.enabled || R_CullBoxCurDpvs_SceneSelect(minmax, localClientNum, 0) == 0;
}

void __thiscall GlassRenderer::AddShatterAction(
                GlassClient *glass,
                const float *hitPos,
                const float *hitDir,
                bool gameState)
{
    GlassRenderer::Action *action; // [esp+Ch] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_GLASS_ACTIONS);
    action = &this->actions[this->actionInputIndex % 0xC8u];
    action->type = SHATTER;
    action->shatter.glass = glass;
    action->shatter.gameState = gameState;
    action->shatter.hitPos[0] = *hitPos;
    action->shatter.hitPos[1] = hitPos[1];
    action->shatter.hitPos[2] = hitPos[2];
    action->shatter.hitDir[0] = *hitDir;
    action->shatter.hitDir[1] = hitDir[1];
    action->shatter.hitDir[2] = hitDir[2];
    _InterlockedExchangeAdd(&this->actionInputIndex, 1u);
    Sys_LeaveCriticalSection(CRITSECT_GLASS_ACTIONS);
}

void __thiscall GlassRenderer::AddExplosionAction(
                const float *origin,
                float damageInner,
                float damageOuter,
                float radius,
                int mod)
{
    GlassRenderer::Action *action; // [esp+8h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_GLASS_ACTIONS);
    action = &this->actions[this->actionInputIndex % 0xC8u];
    action->type = EXPLOSION;
    action->trace.p0[0] = *origin;
    action->trace.p0[1] = origin[1];
    action->shatter.hitPos[0] = origin[2];
    action->shatter.hitPos[1] = damageInner;
    action->shatter.hitPos[2] = damageOuter;
    action->shatter.hitDir[0] = radius;
    action->explosion.mod = mod;
    _InterlockedExchangeAdd(&this->actionInputIndex, 1u);
    Sys_LeaveCriticalSection(CRITSECT_GLASS_ACTIONS);
}

void __thiscall GlassRenderer::AddTraceAction(const float *p0, const float *p1)
{
    GlassRenderer::Action *action; // [esp+Ch] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_GLASS_ACTIONS);
    action = &this->actions[this->actionInputIndex % 0xC8u];
    action->type = TRACE_POINT;
    action->trace.p0[0] = *p0;
    action->trace.p0[1] = p0[1];
    action->shatter.hitPos[0] = p0[2];
    action->shatter.hitPos[1] = *p1;
    action->shatter.hitPos[2] = p1[1];
    action->shatter.hitDir[0] = p1[2];
    _InterlockedExchangeAdd(&this->actionInputIndex, 1u);
    Sys_LeaveCriticalSection(CRITSECT_GLASS_ACTIONS);
}

void __thiscall GlassRenderer::AddMeleeAction(
                const float *eyePos,
                const float *forward,
                const float *right,
                const float *up,
                float range)
{
    GlassRenderer::Action *action; // [esp+14h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_GLASS_ACTIONS);
    action = &this->actions[this->actionInputIndex % 0xC8u];
    action->type = MELEE;
    action->trace.p0[0] = *eyePos;
    action->trace.p0[1] = eyePos[1];
    action->shatter.hitPos[0] = eyePos[2];
    action->shatter.hitPos[1] = *forward;
    action->shatter.hitPos[2] = forward[1];
    action->shatter.hitDir[0] = forward[2];
    action->shatter.hitDir[1] = *right;
    action->shatter.hitDir[2] = right[1];
    action->melee.right[2] = right[2];
    action->melee.up[0] = *up;
    action->melee.up[1] = up[1];
    action->melee.up[2] = up[2];
    action->melee.range = range;
    _InterlockedExchangeAdd(&this->actionInputIndex, 1u);
    Sys_LeaveCriticalSection(CRITSECT_GLASS_ACTIONS);
}

void __thiscall GlassRenderer::ExecuteActions()
{
    GlassRenderer::Action *action; // [esp+20h] [ebp-8h]

    PROF_SCOPED("GlassRenderer.ExecuteActions");

    while ( this->actionOutputIndex != this->actionInputIndex )
    {
        action = &this->actions[this->actionOutputIndex % 0xC8u];
        switch ( action->type )
        {
            case SHATTER:
                action->shatter.glass->Shatter(action->shatter.hitPos, action->shatter.hitDir);
                if ( action->shatter.gameState )
                    GlassRenderer::RemovePhysicsShards();
                break;
            case TRACE_POINT:
                GlassRenderer::TracePoint(action->trace.p0, action->trace.p1);
                break;
            case EXPLOSION:
                this->ExplosionEvent(action->explosion.origin, 
                    action->explosion.damageInner, 
                    action->explosion.damageOuter, 
                    action->explosion.radius, 
                    action->explosion.mod);
                break;
            case MELEE:
                this->Melee(action->melee.eyePos, action->melee.forward, action->melee.right, action->melee.up, action->melee.range);
                break;
            default:
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp",
                                894,
                                0,
                                "Unknown glass action command") )
                    __debugbreak();
                break;
        }
        _InterlockedExchangeAdd(&this->actionOutputIndex, 1u);
        GlassRenderer::DoGroupChanges();
    }
}

void __thiscall GlassRenderer::StartMaintenance()
{
    GlassRenderer::DoMaintenance();
}

#if 0
void __thiscall GlassRenderer::GetSmallestShards(
                unsigned int maxShards,
                bool checkVisible,
                bool isVisible)
{
    std::_List_nod<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Node *Myhead; // [esp+4Ch] [ebp-48h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> > *p_used; // [esp+50h] [ebp-44h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> v7; // [esp+54h] [ebp-40h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> _Right; // [esp+74h] [ebp-20h] BYREF
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> v9; // [esp+7Ch] [ebp-18h]
    GlassShard *shard; // [esp+84h] [ebp-10h]
    ShardGroup *grp; // [esp+88h] [ebp-Ch]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> g; // [esp+8Ch] [ebp-8h] BYREF

    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::clear((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this->tempShardsList);
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::begin(
        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)&this->groupsAllocator->used,
        (std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> *)&g);
    while ( 1 )
    {
        p_used = &this->groupsAllocator->used;
        Myhead = this->groupsAllocator->used._Myhead;
        _Right._Myaux = 0;
        _Right._Ptr = (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)Myhead;
        if ( !p_used )
            _invalid_parameter_noinfo();
        _Right._Myaux = p_used->_Myownedaux;
        if ( !std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator!=(
                        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&g,
                        &_Right) )
            break;
        grp = (ShardGroup *)*std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&g);
        if ( !checkVisible || grp->visible == isVisible )
        {
            for ( shard = grp->head; shard; shard = shard->groupNext )
            {
                GlassRenderer::SortedShardsList::Insert(this->tempShardsList, shard);
                if ( this->tempShardsList->_Mysize > maxShards )
                    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::pop_back(this->tempShardsList);
            }
        }
        v7 = g;
        std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::_Const_iterator<1>::operator++((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&g);
        v9 = v7;
    }
}
#endif

void GlassRenderer::GetSmallestShards(unsigned int maxShards, bool checkVisible, bool isVisible)
{
    // Clear the temporary shard list
    tempShardsList->clear();

    // Iterate over all shard groups
    for (ShardGroup *grp : groupsAllocator->used)
    {
        // Skip invisible groups if requested
        if (checkVisible && grp->visible != isVisible)
            continue;

        // Iterate over all shards in the group
        for (GlassShard *shard = grp->head; shard; shard = shard->groupNext)
        {
            // Insert the shard into the sorted temporary list
            //SortedShardsList::Insert(tempShardsList, shard);
            tempShardsList->Insert(shard);

            // Keep only the smallest `maxShards` shards
            if (tempShardsList->size() > maxShards)
                tempShardsList->pop_back();
        }
    }
}

#if 0
void __thiscall GlassRenderer::GetLargestShards(
                unsigned int maxShards,
                bool checkVisible,
                bool isVisible)
{
    std::_List_nod<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Node *Myhead; // [esp+4Ch] [ebp-48h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> > *p_used; // [esp+50h] [ebp-44h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> v7; // [esp+54h] [ebp-40h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> _Right; // [esp+74h] [ebp-20h] BYREF
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> v9; // [esp+7Ch] [ebp-18h]
    GlassShard *shard; // [esp+84h] [ebp-10h]
    ShardGroup *grp; // [esp+88h] [ebp-Ch]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> g; // [esp+8Ch] [ebp-8h] BYREF

    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::clear((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this->tempShardsList);
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::begin(
        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)&this->groupsAllocator->used,
        (std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> *)&g);
    while ( 1 )
    {
        p_used = &this->groupsAllocator->used;
        Myhead = this->groupsAllocator->used._Myhead;
        _Right._Myaux = 0;
        _Right._Ptr = (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)Myhead;
        if ( !p_used )
            _invalid_parameter_noinfo();
        _Right._Myaux = p_used->_Myownedaux;
        if ( !std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator!=(
                        (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&g,
                        &_Right) )
            break;
        grp = (ShardGroup *)*std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&g);
        if ( !checkVisible || grp->visible == isVisible )
        {
            for ( shard = grp->head; shard; shard = shard->groupNext )
            {
                GlassRenderer::SortedShardsList::InsertReverse(this->tempShardsList, shard);
                if ( this->tempShardsList->_Mysize > maxShards )
                    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::pop_back(this->tempShardsList);
            }
        }
        v7 = g;
        std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::_Const_iterator<1>::operator++((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&g);
        v9 = v7;
    }
}
#endif

void GlassRenderer::GetLargestShards(unsigned int maxShards, bool checkVisible, bool isVisible)
{
    // Clear the temporary list first
    tempShardsList->clear();


    // Iterate over all shard groups
    for (ShardGroup *grp : groupsAllocator->used)
    {
        // Skip this group if visibility filtering is enabled and it doesn't match
        if (checkVisible && grp->visible != isVisible)
            continue;


        // Iterate over all shards in the group
        for (GlassShard *shard = grp->head; shard; shard = shard->groupNext)
        {
            // Insert shard into tempShardsList in descending order (largest first)
            tempShardsList->InsertReverse(shard);


            // Keep list size bounded to maxShards
            if (tempShardsList->size() > maxShards)
                tempShardsList->pop_back();
        }
    }
}


void __thiscall GlassRenderer::AddUsedShardMaterial(Material *material)
{
    unsigned int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < this->numUsedMaterials; ++i )
    {
        if ( this->usedMaterials[i] == material )
            return;
    }
    if ( this->numUsedMaterials >= 0x20 )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp",
                        978,
                        0,
                        "GlassRenderer::usedMaterials is too small") )
            __debugbreak();
    }
    else
    {
        R_StreamTouchMaterial(material);
        this->usedMaterials[this->numUsedMaterials++] = material;
    }
}

#if 0
void __thiscall GlassRenderer::DoMaintenance()
{
    int Free; // eax
    GlassShard **v3; // [esp+58h] [ebp-E0h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> v4; // [esp+64h] [ebp-D4h]
    int v5; // [esp+80h] [ebp-B8h]
    GlassShard **v6; // [esp+A4h] [ebp-94h]
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> v7; // [esp+B0h] [ebp-88h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> _Right; // [esp+F0h] [ebp-48h] BYREF
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> v9; // [esp+F8h] [ebp-40h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> v10; // [esp+100h] [ebp-38h] BYREF
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> v11; // [esp+108h] [ebp-30h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> v12; // [esp+114h] [ebp-24h] BYREF
    int targetFree; // [esp+11Ch] [ebp-1Ch]
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> shard; // [esp+124h] [ebp-14h] BYREF
    unsigned int numToFree; // [esp+130h] [ebp-8h]

    Sys_WaitInterlockedCompareExchange(&this->rendererLock.lock, 1, 0);
    //PIXBeginNamedEvent(-1, "GlassRenderer.DoMaintenance");
    if ( this->doMaintenance->current.enabled )
    {
        if ( (double)this->shardsAllocator->used.size()
             / (double)(this->shardsAllocator->free.size() + this->shardsAllocator->used.size()) <= 0.89999998 )
        {
            v5 = (char *)this->shardMemoryAllocator.tail - (char *)this->shardMemoryAllocator.head;
            if ( (float)(1.0 - 0.89999998) > (float)((float)Allocator::GetFree(&this->shardMemoryAllocator) / (float)v5) )
            {
                //PIXBeginNamedEvent(-1, "Too much shard memory");
                GlassRenderer::GetSmallestShards(this, 0x32u, 1, 0);
                targetFree = (int)(float)((float)((char *)this->shardMemoryAllocator.tail
                                                                                - (char *)this->shardMemoryAllocator.head)
                                                                * (float)(1.0 - 0.89999998));
                std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::_Const_iterator<1>(
                    &v12,
                    (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->tempShardsList->_Myhead->_Next,
                    (const std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this->tempShardsList);
                while ( 1 )
                {
                    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::_Const_iterator<1>(
                        &_Right,
                        (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->tempShardsList->_Myhead,
                        (const std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this->tempShardsList);
                    if ( !std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator!=(
                                    &v12,
                                    &_Right) )
                        break;
                    Free = Allocator::GetFree(&this->shardMemoryAllocator);
                    if ( Free >= targetFree )
                        break;
                    v3 = (GlassShard **)std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*(&v12);
                    GlassShard::Remove(*v3, REMOVE_OUT_OF_SHARD_MEMORY, 1);
                    v4 = v12;
                    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::_Const_iterator<1>::operator++(&v12);
                    v9 = v4;
                }
                std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::clear((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this->tempShardsList);
                //if ( g_DXDeviceThread == GetCurrentThreadId() )
                    //D3DPERF_EndEvent();
            }
        }
        else
        {
            //PIXBeginNamedEvent(-1, "Too many shards");
            numToFree = this->shardsAllocator->used.size()
                                - (__int64)((double)(this->shardsAllocator->free.size() + this->shardsAllocator->used.size())
                                                    * 0.89999998
                                                    - 5.0);
            GlassRenderer::GetSmallestShards(this, numToFree, 1, 0);
            std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::_Const_iterator<1>(
                (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&shard,
                (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->tempShardsList->_Myhead->_Next,
                (const std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this->tempShardsList);
            while ( 1 )
            {
                std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::_Const_iterator<1>(
                    &v10,
                    (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->tempShardsList->_Myhead,
                    (const std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this->tempShardsList);
                if ( std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator==(
                             &shard,
                             (const std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Const_iterator<1> *)&v10) )
                {
                    break;
                }
                v6 = (GlassShard **)std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&shard);
                GlassShard::Remove(*v6, REMOVE_OUT_OF_SHARDS, 1);
                v7 = shard;
                std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::_Const_iterator<1>::operator++((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&shard);
                v11 = v7;
            }
            std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::clear((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this->tempShardsList);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                //D3DPERF_EndEvent();
        }
        //PIXBeginNamedEvent(-1, "Defrag shard work memory");
        Allocator::Defrag(&this->shardMemoryAllocator, (void (__cdecl *)(void *))GlassShard::Defrag, 10);
        //if ( GetCurrentThreadId() == g_DXDeviceThread )
            //D3DPERF_EndEvent();
        GlassesClient::PreShatterNext(clGlasses);
        this->rendererLock.lock = 0;
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
    }
    else
    {
        this->rendererLock.lock = 0;
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
    }
}
#endif

void __cdecl Sys_WaitInterlockedCompareExchange(volatile unsigned int *destination, int value, int comperand)
{
    do
    {
        while (*destination != comperand)
            ;
    } while (_InterlockedCompareExchange(destination, value, comperand) != comperand);
}

void GlassRenderer::DoMaintenance()
{
    // Acquire renderer lock
    Sys_WaitInterlockedCompareExchange(&rendererLock.lock, 1, 0);


    if (!doMaintenance->current.enabled)
    {
        rendererLock.lock = 0;
        return;
    }


    double totalShards = static_cast<double>(shardsAllocator->used.size() + shardsAllocator->free.size());
    double usedFraction = static_cast<double>(shardsAllocator->used.size()) / totalShards;


    const double maxFraction = 0.9; // 90%
    if (usedFraction <= maxFraction)
    {
        // Too much shard memory reserved
        size_t totalMemory = reinterpret_cast<char *>(shardMemoryAllocator.tail) - reinterpret_cast<char *>(shardMemoryAllocator.head);
        float freeFraction = static_cast<float>(shardMemoryAllocator.GetFree()) / static_cast<float>(totalMemory);


        if ((1.0f - maxFraction) > freeFraction)
        {
            // Free some smallest shards to reclaim memory
            GetSmallestShards(0x32u, true, false);


            int targetFree = static_cast<int>(totalMemory * (1.0f - static_cast<float>(maxFraction)));


            for (GlassShard *shard : *tempShardsList)
            {
                if (shardMemoryAllocator.GetFree() >= targetFree)
                    break;


                //GlassShard::Remove(shard, REMOVE_OUT_OF_SHARD_MEMORY, true);
                shard->Remove(GlassShard::RemoveReason::REMOVE_OUT_OF_SHARD_MEMORY, true);
            }


            tempShardsList->clear();
        }
    }
    else
    {
        // Too many shards allocated, remove excess
        int numToFree = static_cast<int>(shardsAllocator->used.size() - static_cast<int>(totalShards * maxFraction - 5.0));


        GetSmallestShards(numToFree, true, false);


        for (GlassShard *shard : *tempShardsList)
        {
            //GlassShard::Remove(shard, REMOVE_OUT_OF_SHARDS, true);
            shard->Remove(GlassShard::RemoveReason::REMOVE_OUT_OF_SHARDS, true);
        }


        tempShardsList->clear();
    }


    // Defrag shard memory
    //Allocator::Defrag(&shardMemoryAllocator, reinterpret_cast<void(*)(void *)>(GlassShard::Defrag), 10);
    shardMemoryAllocator.Defrag(GlassShard_Defrag, 10);


    // Pre-shatter step
    clGlasses->PreShatterNext();


    // Release renderer lock
    rendererLock.lock = 0;
}

void __cdecl GlassRenderer::CrashGlassCmd()
{
    clGlasses->renderer->StartCrashGlass();
}

void __thiscall GlassRenderer::StartCrashGlass()
{
    const char *v1; // eax
    int v2; // [esp+0h] [ebp-1Ch]
    int v4; // [esp+8h] [ebp-14h]

    if ( Cmd_Argc() <= 1 )
    {
        this->maxCrashShards = 0;
    }
    else
    {
        v1 = Cmd_Argv(1);
        v4 = atoi(v1);
        if ( v4 > 100 )
            v2 = 100;
        else
            v2 = v4;
        this->maxCrashShards = v2;
    }
}

#if 0
void __thiscall GlassRenderer::CrashGlass()
{
    float *p_z; // eax
    float *v3; // [esp+1Ch] [ebp-98h]
    float *p_w; // [esp+34h] [ebp-80h]
    float v5; // [esp+38h] [ebp-7Ch]
    float *p_y; // [esp+3Ch] [ebp-78h]
    GlassPhysics **v7; // [esp+44h] [ebp-70h]
    GlassShard **v8; // [esp+48h] [ebp-6Ch]
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> v9; // [esp+54h] [ebp-60h]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> v10; // [esp+78h] [ebp-3Ch] BYREF
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> v11; // [esp+80h] [ebp-34h]
    float delta[3]; // [esp+88h] [ebp-2Ch]
    float p1[3]; // [esp+94h] [ebp-20h] BYREF
    float p2[3]; // [esp+A0h] [ebp-14h] BYREF
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> shard; // [esp+ACh] [ebp-8h] BYREF

    if ( this->maxCrashShards > 0 )
    {
        std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::clear((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this->tempShardsList);
        GlassRenderer::GetLargestShards(this, this->maxCrashShards, 0, 0);
        std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::_Const_iterator<1>(
            (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&shard,
            (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->tempShardsList->_Myhead->_Next,
            (const std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this->tempShardsList);
        while ( 1 )
        {
            std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::_Const_iterator<1>(
                &v10,
                (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->tempShardsList->_Myhead,
                (const std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this->tempShardsList);
            if ( std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator==(
                         &shard,
                         (const std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Const_iterator<1> *)&v10) )
            {
                break;
            }
            v8 = (GlassShard **)std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&shard);
            if ( GlassShard::CanSplit(*v8, 0) )
            {
                v7 = std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&shard);
                p_w = &(*std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&shard))->m_inv_inertia.w;
                v5 = flrand(-1.0, 1.0);
                p_y = &(*v7)->m_mat.w.y;
                delta[0] = (float)(10.0 * *p_w) + (float)(v5 * *p_y);
                delta[1] = (float)(10.0 * p_w[1]) + (float)(v5 * p_y[1]);
                delta[2] = (float)(10.0 * p_w[2]) + (float)(v5 * p_y[2]);
                p_z = &(*std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&shard))->m_t_vel.z;
                p1[0] = *p_z + delta[0];
                p1[1] = p_z[1] + delta[1];
                p1[2] = p_z[2] + delta[2];
                v3 = &(*std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&shard))->m_t_vel.z;
                p2[0] = *v3 - delta[0];
                p2[1] = v3[1] - delta[1];
                p2[2] = v3[2] - delta[2];
                GlassRenderer::AddTraceAction(this, p1, p2);
            }
            v9 = shard;
            std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::_Const_iterator<1>::operator++((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&shard);
            v11 = v9;
        }
        std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::clear((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)this->tempShardsList);
    }
}
#endif

void GlassRenderer::CrashGlass()
{
    if (maxCrashShards == 0)
        return;


    // Clear temp list and get largest shards
    tempShardsList->clear();
    GetLargestShards(maxCrashShards, false, false);


    for (const GlassShard *shard : *tempShardsList)
    {
        if (!shard->CanSplit(false))
            continue;


        GlassPhysics *phys = shard->glassPhysics;
        if (!phys)
            continue;


        // Random factor
        float r = flrand(-1.0f, 1.0f);


        // Compute delta vector based on shard physics
        float delta[3] = {
        10.0f * phys->m_inv_inertia.x + r * phys->m_mat.w.x,
        10.0f * phys->m_inv_inertia.y + r * phys->m_mat.w.y,
        10.0f * phys->m_inv_inertia.z + r * phys->m_mat.w.z
        };


        // Two velocity vectors
        float p1[3] = {
        phys->m_t_vel.x + delta[0],
        phys->m_t_vel.y + delta[1],
        phys->m_t_vel.z + delta[2]
        };
        float p2[3] = {
        phys->m_t_vel.x - delta[0],
        phys->m_t_vel.y - delta[1],
        phys->m_t_vel.z - delta[2]
        };


        // Add trace action for shard split
        AddTraceAction(p1, p2);
    }

    tempShardsList->clear();
}

void __cdecl GlassRenderer::PrintHwmCmd()
{
    clGlasses->renderer->PrintHwm();
}

void __thiscall GlassRenderer::PrintHwm()
{
    Com_Printf(0, "glasses:                                %7d\n", clGlasses->numGlasses);
    Com_Printf(
        0,
        "glass_smallAllocator:     %7d [%5.1f%%]\n",
        this->smallAllocator.maxUsed,
        (double)this->smallAllocator.maxUsed * 100.0 / (double)this->smallAllocator.numBlocks);
    Com_Printf(
        0,
        "glass_groupsAllocator:    %7d [%5.1f%%]\n",
        this->groupsAllocator->maxUsed,
        (double)this->groupsAllocator->maxUsed
    * 100.0
    / (double)(this->groupsAllocator->free.size() + this->groupsAllocator->used.size()));
    Com_Printf(
        0,
        "glass_shardsAllocator:    %7d [%5.1f%%]\n",
        this->shardsAllocator->maxUsed,
        (double)this->shardsAllocator->maxUsed
    * 100.0
    / (double)(this->shardsAllocator->free.size() + this->shardsAllocator->used.size()));
    Com_Printf(
        0,
        "glass_physicsAllocator: %7d [%5.1f%%]\n",
        this->physicsAllocator->maxUsed,
        (double)this->physicsAllocator->maxUsed
    * 100.0
    / (double)(this->physicsAllocator->free.size() + this->physicsAllocator->used.size()));
    Com_Printf(
        0,
        "glass_sharedMemory:         %7d [%5.1f%%]\n",
        this->shardMemoryAllocator.maxUsed,
        (float)((float)((float)this->shardMemoryAllocator.maxUsed * 100.0)
                    / (float)((char *)this->shardMemoryAllocator.tail - (char *)this->shardMemoryAllocator.head)));
    Com_Printf(0, "sizeof(GlassClient)         %7d\n", 12);
    Com_Printf(0, "sizeof(GlassRenderer)     %7d\n", 19960);
    Com_Printf(0, "sizeof(ShardGroup)            %7d\n", 84);
    Com_Printf(0, "sizeof(GlassShard)            %7d\n", 144);
    Com_Printf(0, "sizeof(GlassPhysics)        %7d\n", 160);
}

#if 0
void __thiscall GlassRenderer::Broom()
{
    float v2; // [esp+2Ch] [ebp-90h]
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *> >::_Const_iterator<1> v3; // [esp+38h] [ebp-84h]
    float v4; // [esp+58h] [ebp-64h]
    float v5; // [esp+60h] [ebp-5Ch]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> _Right; // [esp+64h] [ebp-58h] BYREF
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *> >::_Const_iterator<1> v7; // [esp+6Ch] [ebp-50h]
    float d3[3]; // [esp+74h] [ebp-48h] BYREF
    float d2[2]; // [esp+80h] [ebp-3Ch] BYREF
    float pos[2]; // [esp+88h] [ebp-34h]
    GlassShard *shrd; // [esp+90h] [ebp-2Ch]
    std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *> >::_Const_iterator<1> shard; // [esp+94h] [ebp-28h] BYREF
    float dir[2]; // [esp+9Ch] [ebp-20h] BYREF
    float origin[2]; // [esp+A4h] [ebp-18h]
    const cg_s *cgameGlob; // [esp+ACh] [ebp-10h]
    float DISTANCE; // [esp+B0h] [ebp-Ch]
    float FORCE_SCALE; // [esp+B4h] [ebp-8h]
    float RADIUS; // [esp+B8h] [ebp-4h]

    if ( this->broom->current.enabled )
    {
        DISTANCE = 150.0f;
        RADIUS = 120.0f;
        FORCE_SCALE = 0.05f;
        cgameGlob = CG_GetLocalClientGlobals(0);
        v5 = cgameGlob->refdef.vieworg[1];
        origin[0] = cgameGlob->refdef.vieworg[0];
        origin[1] = v5;
        v4 = cgameGlob->refdef.viewaxis[2][1];
        dir[0] = cgameGlob->refdef.viewaxis[2][0];
        dir[1] = v4;
        Vec2Normalize(dir);
        origin[0] = (float)(DISTANCE * dir[0]) + origin[0];
        origin[1] = (float)(DISTANCE * dir[1]) + origin[1];
        std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::_Const_iterator<1>(
            (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&shard,
            (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->shardsAllocator->used._Myhead->_Next,
            (const std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)&this->shardsAllocator->used);
        while ( 1 )
        {
            std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::_Const_iterator<1>(
                &_Right,
                (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)this->shardsAllocator->used._Myhead,
                (const std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)&this->shardsAllocator->used);
            if ( !std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator!=(
                            (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&shard,
                            &_Right) )
                break;
            shrd = (GlassShard *)*std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&shard);
            if ( shrd->group )
            {
                v2 = shrd->origin[1];
                pos[0] = shrd->origin[0];
                pos[1] = v2;
                if ( (float)(RADIUS * RADIUS) > (float)((float)((float)(pos[0] - origin[0]) * (float)(pos[0] - origin[0]))
                                                                                            + (float)((float)(v2 - origin[1]) * (float)(v2 - origin[1]))) )
                {
                    d2[0] = origin[0] - pos[0];
                    d2[1] = origin[1] - pos[1];
                    Vec2Normalize(d2);
                    if ( (float)((float)(dir[0] * d2[0]) + (float)(dir[1] * d2[1])) > 0.0 )
                    {
                        d3[0] = FORCE_SCALE * d2[0];
                        d3[1] = FORCE_SCALE * d2[1];
                        d3[2] = FORCE_SCALE * 0.0;
                        GlassShard::InitPhysics(shrd, 1, shrd->origin, d3, 0.0, 0.0);
                    }
                }
            }
            v3 = shard;
            std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::_Const_iterator<1>::operator++((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&shard);
            v7 = v3;
        }
    }
}
#endif

void GlassRenderer::Broom()
{
    if (!broom->current.enabled)
        return;


    constexpr float DISTANCE = 150.0f;
    constexpr float RADIUS = 120.0f;
    constexpr float FORCE_SCALE = 0.05f;


    const cg_s *cgameGlob = CG_GetLocalClientGlobals(0);


    // Compute broom origin in front of player
    float origin[2] = {
    cgameGlob->refdef.vieworg[0] + DISTANCE * cgameGlob->refdef.viewaxis[2][0],
    cgameGlob->refdef.vieworg[1] + DISTANCE * cgameGlob->refdef.viewaxis[2][1]
    };


    // Broom direction
    float dir[2] = { cgameGlob->refdef.viewaxis[2][0], cgameGlob->refdef.viewaxis[2][1] };
    Vec2Normalize(dir);


    // Loop over all shards
    for (GlassShard *shrd : shardsAllocator->used)
    {
        if (!shrd->group)
            continue;


        float dx = shrd->origin[0] - origin[0];
        float dy = shrd->origin[1] - origin[1];


        // Check if within broom radius
        if ((dx * dx + dy * dy) <= (RADIUS * RADIUS))
        {
            float d2[2] = { -dx, -dy }; // direction from shard to origin
            Vec2Normalize(d2);


            // Apply only if shard roughly in broom direction
            float dot = dir[0] * d2[0] + dir[1] * d2[1];
            if (dot > 0.0f)
            {
                float force[3] = { FORCE_SCALE * d2[0], FORCE_SCALE * d2[1], 0.0f };
                //GlassShard::InitPhysics(shrd, 1, shrd->origin, force, 0.0f, 0.0f);
                shrd->InitPhysics(1, shrd->origin, force, 0.0f, 0.0f);
            }
        }
    }
}

#if 0
void __thiscall GlassRenderer::DrawDebug()
{
    int Free; // eax
    GlassPhysics **v2; // eax
    const GlassClient *Glass; // eax
    double xScale; // [esp+8h] [ebp-19Ch]
    float *xScale_4; // [esp+Ch] [ebp-198h]
    float *xScale_4a; // [esp+Ch] [ebp-198h]
    int yaw; // [esp+10h] [ebp-194h]
    int yaw_4; // [esp+14h] [ebp-190h]
    double v9; // [esp+18h] [ebp-18Ch]
    unsigned int v10; // [esp+24h] [ebp-180h]
    float v11; // [esp+28h] [ebp-17Ch]
    float v12; // [esp+44h] [ebp-160h]
    float v13; // [esp+6Ch] [ebp-138h]
    float v14; // [esp+88h] [ebp-11Ch]
    std::_List_nod<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Node *Myhead; // [esp+B8h] [ebp-ECh]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> > *p_used; // [esp+BCh] [ebp-E8h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> v18; // [esp+C0h] [ebp-E4h]
    char *v19; // [esp+D0h] [ebp-D4h]
    int v20; // [esp+E4h] [ebp-C0h]
    char *v21; // [esp+FCh] [ebp-A8h]
    char *v22; // [esp+114h] [ebp-90h]
    char *v23; // [esp+130h] [ebp-74h]
    char *v24; // [esp+138h] [ebp-6Ch]
    int v25; // [esp+14Ch] [ebp-58h]
    char *s; // [esp+158h] [ebp-4Ch]
    std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> _Right; // [esp+17Ch] [ebp-28h] BYREF
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> v28; // [esp+184h] [ebp-20h]
    unsigned int i; // [esp+18Ch] [ebp-18h]
    std::list<ShardGroup *,SmallAllocatorTemplate<ShardGroup * *> >::_Const_iterator<1> grp; // [esp+190h] [ebp-14h] BYREF
    float idx2vtx; // [esp+198h] [ebp-Ch]
    float x; // [esp+19Ch] [ebp-8h]
    float y; // [esp+1A0h] [ebp-4h]

    if ( this->drawDebug && this->drawDebug->current.enabled )
    {
        x = 0.0f;
        y = 0.0f;
        idx2vtx = 0.0f;
        s = va(
                    "glasses:%d shards:%d(free:%d) moving:%d(%d) vis:%d idx/vtx=%.1f",
                    clGlasses->numGlasses,
                    this->shardsAllocator->used.size(),
                    this->shardsAllocator->free.size(),
                    this->prevStat.numMovingShards,
                    this->colidingShards->size(),
                    this->prevStat.numVisShards,
                    0.0);
        y = (float)CG_DrawDevString(scrPlaceView, 0.0, 0.0, 0.75, 0.75, s, colorWhite, 5, cgMedia.smallDevFont) + 0.0;
        if ( this->genVertsCount )
            v14 = (double)this->genVertsTimer * 1000000.0 / tlPcTicksPerMS / (double)this->genVertsCount;
        else
            v14 = 0.0f;
        if ( this->numSplits )
            v13 = (double)this->splitTimer * 1000.0 / tlPcTicksPerMS / (double)this->numSplits;
        else
            v13 = 0.0f;
        if ( HIDWORD(this->triangulateTimer) || LODWORD(this->triangulateTimer) )
            v12 = (double)this->triangulateTimer * 100.0 / (double)this->splitTimer;
        else
            v12 = 0.0f;
        if ( this->numShatters )
            v11 = (double)this->shatterTimer / tlPcTicksPerMS / (double)this->numShatters;
        else
            v11 = 0.0f;
        if ( this->shardsAllocator->used.size() )
        {
            v25 = (char *)this->shardMemoryAllocator.tail - (char *)this->shardMemoryAllocator.head;
            v10 = (v25 - this->shardMemoryAllocator.GetFree()) / this->shardsAllocator->used.size();
        }
        else
        {
            v10 = 0;
        }
        v24 = va("Shard: bytes:%d shatter:%.2fms triang=%.1f%% split=%.1fus genV=%.1fns", v10, v11, v12, v13, v14);
        y = (float)CG_DrawDevString(scrPlaceView, x, y, 0.75, 0.75, v24, colorWhite, 5, cgMedia.smallDevFont) + y;
        v23 = va(
                        "ShardRemoves: bottom:%d shards:%d shardMem:%d vtx:%d phys:%d init:%d",
                        GlassShard::removeReasonsCount[0],
                        dword_A707094,
                        dword_A70709C,
                        dword_A707098,
                        dword_A7070A0,
                        dword_A7070A8);
        y = (float)CG_DrawDevString(scrPlaceView, x, y, 0.75, 0.75, v23, colorWhite, 5, cgMedia.smallDevFont) + y;
        v22 = va(
                        "ShardGroups: num:%d (free:%d) vis:%d oom:%d maxChng:%d",
                        this->groupsAllocator->used.size(),
                        this->groupsAllocator->free.size(),
                        this->prevStat.numVisGroups,
                        this->prevStat.numOOMGroups,
                        this->maxNumGroupChanges);
        y = (float)CG_DrawDevString(scrPlaceView, x, y, 0.75, 0.75, v22, colorWhite, 5, cgMedia.smallDevFont) + y;
        v21 = va(
                        "Allocator: used:%d(%d) free:%d",
                        this->smallAllocator.numUsed,
                        this->smallAllocator.maxUsed,
                        this->smallAllocator.numBlocks - this->smallAllocator.numUsed);
        y = (float)CG_DrawDevString(scrPlaceView, x, y, 0.75, 0.75, v21, colorWhite, 5, cgMedia.smallDevFont) + y;
        v20 = (char *)this->shardMemoryAllocator.tail - (char *)this->shardMemoryAllocator.head;
        v9 = (float)((float)((float)this->minFreeShardsMemory * 100.0) / (float)v20);
        yaw_4 = this->minFreeShardsMemory;
        yaw = Allocator::GetLargestFree(&this->shardMemoryAllocator);
        xScale = (float)((float)((float)Allocator::GetFree(&this->shardMemoryAllocator) * 100.0) / (float)v20);
        Free = Allocator::GetFree(&this->shardMemoryAllocator);
        v19 = va("shards:    sz:%d free:%d(%.0f%%) largest:%d min:%d(%.0f%%)", v20, Free, xScale, yaw, yaw_4, v9);
        y = (float)CG_DrawDevString(scrPlaceView, x, y, 0.75, 0.75, v19, colorWhite, 5, cgMedia.smallDevFont) + y;
    }
    if ( this->drawBBox && this->drawBBox->current.enabled )
    {
        std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::begin(
            (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> > *)&this->groupsAllocator->used,
            (std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *> >::_Iterator<1> *)&grp);
        while ( 1 )
        {
            p_used = &this->groupsAllocator->used;
            Myhead = this->groupsAllocator->used._Myhead;
            _Right._Myaux = 0;
            _Right._Ptr = (std::_List_nod<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Node *)Myhead;
            if ( !p_used )
                _invalid_parameter_noinfo();
            _Right._Myaux = p_used->_Myownedaux;
            if ( !std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator!=(
                            (std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp,
                            &_Right) )
                break;
            if ( BYTE1((*std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp))->m_mat.w.w) )
            {
                xScale_4 = &(*std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp))->m_mat.y.z;
                v2 = std::list<GlassShard *,SmallAllocatorTemplate<GlassShard *>>::_Const_iterator<1>::operator*((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp);
                CL_AddDebugBox(vec3_origin, &(*v2)->m_mat.x.w, xScale_4, 0.0, colorWhite, 1, 1);
            }
            v18 = grp;
            std::list<GlassShard *,SmallAllocatorTemplate<GlassShard * *>>::_Const_iterator<1>::operator++((std::list<GlassPhysics *,SmallAllocatorTemplate<GlassPhysics * *> >::_Const_iterator<1> *)&grp);
            v28 = v18;
        }
        for ( i = 0; i < clGlasses->numGlasses; ++i )
        {
            xScale_4a = GlassesClient::GetGlass(clGlasses, i)->glass->absmax;
            Glass = GlassesClient::GetGlass(clGlasses, i);
            CL_AddDebugBox(vec3_origin, Glass->glass->absmin, xScale_4a, 0.0, colorBlue, 1, 1);
        }
    }
}
#endif

void GlassRenderer::DrawDebug()
{
    if (drawDebug && drawDebug->current.enabled)
    {
        float x = 0.0f, y = 0.0f;

        // Display general shard stats
        char *s = va(
            "glasses:%d shards:%d(free:%d) moving:%d(%d) vis:%d idx/vtx=%.1f",
            clGlasses->numGlasses,
            (int)shardsAllocator->used.size(),
            (int)shardsAllocator->free.size(),
            prevStat.numMovingShards,
            (int)colidingShards->size(),
            prevStat.numVisShards,
            0.0f);

        y = (float)CG_DrawDevString(scrPlaceView, 0.0f, 0.0f, 0.75f, 0.75f, s, colorWhite, 5, cgMedia.smallDevFont);

        // Compute timings
        float genV = genVertsCount ? (float)(genVertsTimer * 1e6 / tlPcTicksPerMS / genVertsCount) : 0.0f;
        float splitUs = numSplits ? (float)(splitTimer * 1000.0 / tlPcTicksPerMS / numSplits) : 0.0f;
        float triangPct = splitTimer ? (float)(triangulateTimer * 100.0 / splitTimer) : 0.0f;
        float shatterMs = numShatters ? (float)(shatterTimer / tlPcTicksPerMS / numShatters) : 0.0f;

        // Memory per shard
        unsigned int memPerShard = shardsAllocator->used.size()
            ? (unsigned int)(((char *)shardMemoryAllocator.tail - (char *)shardMemoryAllocator.head - shardMemoryAllocator.GetFree()) / shardsAllocator->used.size())
            : 0;

        s = va("Shard: bytes:%d shatter:%.2fms triang=%.1f%% split=%.1fus genV=%.1fns", memPerShard, shatterMs, triangPct, splitUs, genV);
        y = (float)CG_DrawDevString(scrPlaceView, x, y, 0.75f, 0.75f, s, colorWhite, 5, cgMedia.smallDevFont);

        // Shard removes
        s = va(
            "ShardRemoves: bottom:%d shards:%d shardMem:%d vtx:%d phys:%d init:%d",
            GlassShard::removeReasonsCount[0],
            GlassShard::removeReasonsCount[1],
            GlassShard::removeReasonsCount[3],
            GlassShard::removeReasonsCount[2],
            GlassShard::removeReasonsCount[4],

            GlassShard::removeReasonsCount[6]);
        y = (float)CG_DrawDevString(scrPlaceView, x, y, 0.75f, 0.75f, s, colorWhite, 5, cgMedia.smallDevFont);

        // Shard group info
        s = va(
            "ShardGroups: num:%d (free:%d) vis:%d oom:%d maxChng:%d",
            (int)groupsAllocator->used.size(),
            (int)groupsAllocator->free.size(),
            prevStat.numVisGroups,
            prevStat.numOOMGroups,
            maxNumGroupChanges);
        y = (float)CG_DrawDevString(scrPlaceView, x, y, 0.75f, 0.75f, s, colorWhite, 5, cgMedia.smallDevFont);

        // Small allocator
        s = va("Allocator: used:%d(%d) free:%d", smallAllocator.numUsed, smallAllocator.maxUsed, smallAllocator.numBlocks - smallAllocator.numUsed);
        y = (float)CG_DrawDevString(scrPlaceView, x, y, 0.75f, 0.75f, s, colorWhite, 5, cgMedia.smallDevFont);

        // Shard memory
        int totalMem = (char *)shardMemoryAllocator.tail - (char *)shardMemoryAllocator.head;
        float freePct = totalMem ? (float)(shardMemoryAllocator.GetFree() * 100) / totalMem : 0.0f;
        float minPct = totalMem ? (float)(minFreeShardsMemory * 100) / totalMem : 0.0f;
        int largestFree = shardMemoryAllocator.GetLargestFree();

        s = va("shards: sz:%d free:%d(%.0f%%) largest:%d min:%d(%.0f%%)", totalMem, shardMemoryAllocator.GetFree(), freePct, largestFree, minFreeShardsMemory, minPct);
        y = (float)CG_DrawDevString(scrPlaceView, x, y, 0.75f, 0.75f, s, colorWhite, 5, cgMedia.smallDevFont);
    }

    if (drawBBox && drawBBox->current.enabled)
    {
        // Draw all shard group boxes
        for (ShardGroup *grp : groupsAllocator->used)
        {
            if (!grp)
                continue;

            CL_AddDebugBox(vec3_origin, grp->worldBBoxMin, grp->worldBBoxMax, 0.0f, colorWhite, 1, 1);
        }

        // Draw all glasses boxes
        for (unsigned int i = 0; i < clGlasses->numGlasses; ++i)
        {
            //const Glass *glass = GlassesClient::GetGlass(clGlasses, i)->glass;
            const Glass *glass = clGlasses->GetGlass(i)->glass;
            CL_AddDebugBox(vec3_origin, glass->absmin, glass->absmax, 0.0f, colorBlue, 1, 1);
        }
    }
}

void __thiscall GlassRenderer::BeginUpdate()
{
    ;
}

void __thiscall ShardGroup::Init(unsigned int p, const GlassDef *gd)
{
    this->packedPos = p;
    this->glassDef = gd;
    this->head = 0;
    this->lightingHandle = 0;
    this->invalidBBox = 1;
    this->renderIndices = 0;
    this->numShards = 0;
}

void __thiscall ShardGroup::Reset()
{
    GlassShard *shard; // [esp+8h] [ebp-4h]

    for ( shard = this->head; shard; shard = shard->groupNext )
    {
        if (shard->physObjId || shard->glassPhysics)
            shard->DestroyPhysicsObj();
    }
    this->head = 0;
}

void __thiscall ShardGroup::Add(GlassShard *shard)
{
    if ( shard->group
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp", 1460, 0, "%s", "!shard->group") )
    {
        __debugbreak();
    }
    this->invalidBBox = 1;
    shard->group = this;
    shard->groupNext = this->head;
    this->head = shard;
    ++this->numShards;
}

void __thiscall ShardGroup::Remove(GlassShard *shard)
{
    GlassShard *s; // [esp+8h] [ebp-4h]

    if ( shard->group != this
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp",
                    1471,
                    0,
                    "%s",
                    "shard->group == this") )
    {
        __debugbreak();
    }
    --this->numShards;
    this->invalidBBox = 1;
    if ( this->head == shard )
    {
        this->head = shard->groupNext;
        shard->group = 0;
        shard->groupNext = 0;
    }
    else
    {
        for ( s = this->head; s; s = s->groupNext )
        {
            if ( s->groupNext == shard )
            {
                s->groupNext = shard->groupNext;
                shard->group = 0;
                shard->groupNext = 0;
                break;
            }
        }
    }
    if ( shard->group
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp", 1495, 0, "%s", "!shard->group") )
    {
        __debugbreak();
    }
    if ( !this->head )
    {
        if ( this->numShards )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp",
                            1499,
                            0,
                            "%s\n\t(numShards) = %i",
                            "(numShards == 0)",
                            this->numShards) )
                __debugbreak();
        }
        ShardGroup::FreeRenderMemory();
        //GlassRenderer::FreeShardGroup(clGlasses->renderer, (GlassPhysics *)this);
        clGlasses->renderer->FreeShardGroup(this);
    }
}

void __thiscall ShardGroup::RemoveGlassShards(unsigned int glassIndex)
{
    GlassShard *shard; // [esp+4h] [ebp-4h]

    for ( shard = this->head; shard; shard = shard->groupNext )
    {
        if (shard->glassIndex == glassIndex)
            shard->Remove(GlassShard::RemoveReason::REMOVE_ROLLBACK_TIME, true);
    }
}

void __thiscall ShardGroup::UpdateBBox()
{
    float *worldBBoxMax; // [esp+38h] [ebp-14h]
    float *worldBBoxMin; // [esp+40h] [ebp-Ch]
    GlassShard *s; // [esp+44h] [ebp-8h]
    bool foundPhysical; // [esp+4Bh] [ebp-1h]

    if ( this->head )
    {
        worldBBoxMin = this->head->worldBBoxMin;
        this->worldBBoxMin[0] = *worldBBoxMin;
        this->worldBBoxMin[1] = worldBBoxMin[1];
        this->worldBBoxMin[2] = worldBBoxMin[2];
        worldBBoxMax = this->head->worldBBoxMax;
        this->worldBBoxMax[0] = *worldBBoxMax;
        this->worldBBoxMax[1] = worldBBoxMax[1];
        this->worldBBoxMax[2] = worldBBoxMax[2];
        foundPhysical = this->head->glassPhysics == 0;
        for ( s = this->head->groupNext; s; s = s->groupNext )
        {
            if ( s->glassPhysics )
            {
                if ( !foundPhysical )
                {
                    Vec3Min(this->worldBBoxMin, s->worldBBoxMin, this->worldBBoxMin);
                    Vec3Max(this->worldBBoxMax, s->worldBBoxMax, this->worldBBoxMax);
                }
            }
            else if ( foundPhysical )
            {
                Vec3Min(this->worldBBoxMin, s->worldBBoxMin, this->worldBBoxMin);
                Vec3Max(this->worldBBoxMax, s->worldBBoxMax, this->worldBBoxMax);
            }
            else
            {
                this->worldBBoxMin[0] = s->worldBBoxMin[0];
                this->worldBBoxMin[1] = s->worldBBoxMin[1];
                this->worldBBoxMin[2] = s->worldBBoxMin[2];
                this->worldBBoxMax[0] = s->worldBBoxMax[0];
                this->worldBBoxMax[1] = s->worldBBoxMax[1];
                this->worldBBoxMax[2] = s->worldBBoxMax[2];
                foundPhysical = 1;
            }
        }
        this->origin[0] = (float)(this->worldBBoxMin[0] + this->worldBBoxMax[0]) * 0.5;
        this->origin[1] = (float)(this->worldBBoxMin[1] + this->worldBBoxMax[1]) * 0.5;
        this->origin[2] = (float)(this->worldBBoxMin[2] + this->worldBBoxMax[2]) * 0.5;
        this->extent[0] = this->worldBBoxMax[0] - this->worldBBoxMin[0];
        this->extent[1] = this->worldBBoxMax[1] - this->worldBBoxMin[1];
        this->extent[2] = this->worldBBoxMax[2] - this->worldBBoxMin[2];
        this->extent[0] = 0.5 * this->extent[0];
        this->extent[1] = 0.5 * this->extent[1];
        this->extent[2] = 0.5 * this->extent[2];
    }
    this->invalidBBox = 0;
}

void __thiscall ShardGroup::Update(float deltaTime)
{
    GlassShard *shard; // [esp+14h] [ebp-4h]

    if ( !this->visible )
        ShardGroup::FreeRenderMemory();
    this->visible = 0;
    for ( shard = this->head; shard; shard = shard->groupNext )
    {
        if ( shard->physObjId || shard->glassPhysics || shard->delayedDrop )
        {
            shard->Update(deltaTime);
            ++clGlasses->renderer->stat.numMovingShards;
            this->invalidBBox = 1;
            ShardGroup::FreeRenderMemory();
        }
    }
}

void __thiscall ShardGroup::GenerateVerts(bool firstView, unsigned int localClientNum)
{
    float v3; // [esp+0h] [ebp-B8h]
    unsigned __int8 v4; // [esp+11h] [ebp-A7h]
    unsigned __int8 numVertsLow; // [esp+12h] [ebp-A6h]
    unsigned __int8 numIndicesLow; // [esp+13h] [ebp-A5h]
    unsigned __int8 numVerts; // [esp+1Eh] [ebp-9Ah]
    unsigned __int8 numIndices; // [esp+1Fh] [ebp-99h]
    GlassShard *j; // [esp+74h] [ebp-44h]
    GlassShard *i; // [esp+78h] [ebp-40h]
    GlassShard *shard; // [esp+7Ch] [ebp-3Ch]
    unsigned __int16 vertsBaseIndex; // [esp+80h] [ebp-38h] BYREF
    unsigned __int16 vertsIndex; // [esp+84h] [ebp-34h]
    GfxPackedVertex *verts; // [esp+88h] [ebp-30h]
    unsigned __int16 *indices; // [esp+8Ch] [ebp-2Ch]
    GlassTimer timer; // [esp+90h] [ebp-28h]
    GfxPackedVertex *baseVerts; // [esp+A4h] [ebp-14h]
    bool needHighLod; // [esp+ABh] [ebp-Dh]
    float dist; // [esp+ACh] [ebp-Ch]
    unsigned int primaryLightIndex; // [esp+B0h] [ebp-8h]
    unsigned int lightHandle; // [esp+B4h] [ebp-4h]

    if ( firstView && this->invalidBBox )
        ShardGroup::UpdateBBox();

    if ( clGlasses->renderer->IsVisible(this->worldBBoxMin, localClientNum) )
    {
        this->visible = 1;
        v3 = Vec3Distance(this->origin, rg.viewOrg);
        dist = R_GetAdjustedLodDist(v3, XMODEL_LOD_RAMP_RIGID);
        needHighLod = clGlasses->renderer->lowLodDist->current.value > dist;
        if ( needHighLod && !this->highLod && this->renderIndices )
            ShardGroup::FreeRenderMemory();
        if ( !this->renderIndices )
        {
            PROF_SCOPED("ShardGroup.GenerateVerts");

            this->highLod = needHighLod;
            ShardGroup::FreeRenderMemory();
            this->numIndices = 0;
            this->numVerts = 0;
            for ( shard = this->head; shard; shard = shard->groupNext )
            {
                if ( this->highLod )
                    numIndices = shard->mesh.numIndices;
                else
                    numIndices = shard->mesh.numIndicesLow;
                this->numIndices += numIndices;
                if ( this->highLod )
                    numVerts = shard->mesh.numVerts;
                else
                    numVerts = shard->mesh.numVertsLow;
                this->numVerts += numVerts;
            }
            this->renderIndices = (unsigned __int16 *)ShardGroup::AllocateIndices(this->numIndices);
            baseVerts = ShardGroup::AllocateVerts(this->numVerts, &vertsBaseIndex);
            if ( !this->renderIndices || !baseVerts )
            {
                ++clGlasses->renderer->stat.numOOMGroups;
                ShardGroup::FreeRenderMemory();
                baseVerts = 0;
                this->renderIndices = 0;
                return;
            }
            indices = this->renderIndices;
            verts = baseVerts;
            vertsIndex = vertsBaseIndex;
            timer.counter = &clGlasses->renderer->genVertsTimer;
            timer.start = tlPcGetTick().QuadPart;
            clGlasses->renderer->genVertsCount += this->numVerts;
            for ( i = this->head; i; i = i->groupNext )
            {
                i->GenerateVerts(this->highLod, verts, vertsIndex, indices);
                if ( this->highLod )
                    numIndicesLow = i->mesh.numIndices;
                else
                    numIndicesLow = i->mesh.numIndicesLow;
                indices += numIndicesLow;
                if ( this->highLod )
                    numVertsLow = i->mesh.numVerts;
                else
                    numVertsLow = i->mesh.numVertsLow;
                verts += numVertsLow;
                if ( this->highLod )
                    v4 = i->mesh.numVerts;
                else
                    v4 = i->mesh.numVertsLow;
                vertsIndex += v4;
            }
            //*timer.counter += tlPcGetTick().QuadPart - timer.start;
            *timer.counter += tlPcGetTick().QuadPart - timer.start;
        }
        if ( !this->renderIndices
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_renderer.cpp",
                        1662,
                        0,
                        "%s",
                        "renderIndices") )
        {
            __debugbreak();
        }
        primaryLightIndex = R_GetNonSunPrimaryLightForBox(0, this->origin, this->extent);
        if ( !primaryLightIndex )
            primaryLightIndex = rgp.world->sunPrimaryLightIndex;
        //lightHandle = R_AllocModelLighting_PrimaryLight(
        //                                this->origin,
        //                                primaryLightIndex,
        //                                0,
        //                                &this->lightingHandle,
        //                                &this->lightingInfo);

        lightHandle = R_AllocModelLighting( // LWSS HACK: increase lighting tolerance here
            this->origin,
            256.0f,           // 16-unit movement before re-alloc; light grid is ~32 units
            0,
            &this->lightingHandle,
            R_PassbackPrimaryLightCallback,
            &primaryLightIndex,
            &this->lightingInfo);

        //Sys_AssistAndWaitWorkerCmdInternal(&r_model_lightingWorkerCmd);

        if ( lightHandle )
        {
            R_AddGlassDrawSurf(
                this->glassDef->shardMaterial,
                this->renderIndices,
                this->numIndices,
                lightHandle,
                primaryLightIndex,
                this->lightingInfo.reflectionProbeIndex,
                9u);
            clGlasses->renderer->AddUsedShardMaterial(this->glassDef->shardMaterial);
        }
        clGlasses->renderer->stat.numVisShards += this->numShards;
        ++clGlasses->renderer->stat.numVisGroups;
        if ( clGlasses->renderer->drawShardOutline->current.integer > 0 )
        {
            for (j = this->head; j; j = j->groupNext)
                j->DrawOutline();
        }
        ShardGroup::FreeRenderMemory();
    }
}

void __thiscall ShardGroup::FreeRenderMemory()
{
    this->renderIndices = 0;
}

r_double_index_t *__thiscall ShardGroup::AllocateIndices(int numIndices)
{
    r_double_index_t *baseIndices; // [esp+4h] [ebp-4h] BYREF

    if ( R_ReserveCodeMeshIndices(numIndices, &baseIndices) )
        return baseIndices;
    else
        return 0;
}

GfxPackedVertex *__thiscall ShardGroup::AllocateVerts(int numVerts, unsigned __int16 *vertsBaseIndex)
{
    if ( R_ReserveCodeMeshVerts(numVerts, vertsBaseIndex) )
        return R_GetCodeMeshVerts(*vertsBaseIndex);
    else
        return 0;
}

void __thiscall ShardGroup::ExplosionEvent(
                const float *origin,
                float damageInner,
                float damageOuter,
                float radius,
                int mod)
{
    GlassShard *shard; // [esp+24h] [ebp-1Ch]

    if ( this->worldBBoxMax[0] >= (float)(*origin - radius)
        && this->worldBBoxMax[1] >= (float)(origin[1] - radius)
        && this->worldBBoxMax[2] >= (float)(origin[2] - radius)
        && (float)(*origin + radius) >= this->worldBBoxMin[0]
        && (float)(origin[1] + radius) >= this->worldBBoxMin[1]
        && (float)(origin[2] + radius) >= this->worldBBoxMin[2] )
    {
        for ( shard = this->head; shard; shard = shard->groupNext )
        {
            if ( !shard->physObjId && !shard->glassPhysics )
                shard->ExplosionEvent(origin, damageInner, damageOuter, radius, mod);
        }
    }
}

int __thiscall ShardGroup::TracePoint(float *p0, const float *p1)
{
    GlassShard *shard; // [esp+34h] [ebp-34h]
    float dir[3]; // [esp+38h] [ebp-30h] BYREF
    float length; // [esp+48h] [ebp-20h]
    float mins[3]; // [esp+4Ch] [ebp-1Ch] BYREF
    float maxs[3]; // [esp+58h] [ebp-10h] BYREF
    int numHits; // [esp+64h] [ebp-4h]

    numHits = 0;
    Vec3Min(p0, p1, mins);
    Vec3Max(p0, p1, maxs);
    if ( this->worldBBoxMax[0] >= mins[0]
        && this->worldBBoxMax[1] >= mins[1]
        && this->worldBBoxMax[2] >= mins[2]
        && maxs[0] >= this->worldBBoxMin[0]
        && maxs[1] >= this->worldBBoxMin[1]
        && maxs[2] >= this->worldBBoxMin[2] )
    {
        PROF_SCOPED("ShardGroup.TracePoint");

        dir[0] = *p1 - *p0;
        dir[1] = p1[1] - p0[1];
        dir[2] = p1[2] - p0[2];
        length = Vec3Normalize(dir);
        for ( shard = this->head; shard; shard = shard->groupNext )
        {
            if ( !shard->physObjId && !shard->glassPhysics )
            {
                if ( shard->TracePoint(p0, p1, dir, length, mins, maxs) )
                    ++numHits;
            }
        }
    }
    return numHits;
}


