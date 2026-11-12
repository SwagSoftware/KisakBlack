#pragma once

#include "phys_local.h"
#include <universal/q_shared.h>
#include <qcommon/cm_trace.h>
#include <qcommon/statmonitor.h>

#define SURF_TYPECOUNT 31

#define FLAG_AABB_LOC_VALID 2

#define SURFACE_CLIP_EPSILON 0.125f

struct cpose_t;
struct gentity_s;
struct Glass;
struct DynEntityDef;
struct gjk_base_t;
struct cLeafBrushNode_s;
struct cbrush_t;
struct XModel;
struct centity_s;
struct DObj;
struct gjk_collision_visitor;
struct cbrushside_t;
struct gjk_geom_list_t;

inline const char *GjkTypeToString(unsigned int type)
{
    switch (type)
    {
        case GJK_BASE:
            return "GJK_BASE";
        case GJK_AABB:
            return "GJK_AABB";
        case GJK_BRUSH:
            return "GJK_BRUSH";
        case GJK_PARTITION:
            return "GJK_PARTITION";
        case GJK_DOUBLE_SPHERE:
            return "GJK_DOUBLE_SPHERE";
        case GJK_CYLINDER:
            return "GJK_CYLINDER";
        case GJK_OBB:
            return "GJK_OBB";
        case GJK_POLYGON_CYLINDER:
            return "GJK_POLYGON_CYLINDER";
        default:
        {
            iassert(0);
            return "??";
        }
    }
}

struct BrushWrapper // sizeof=0x60
{
        float mins[3];
        int contents;
        float maxs[3];
        unsigned int numsides;
        cbrushside_t *sides;
        int axial_cflags[2][3];
        int axial_sflags[2][3];
        unsigned int numverts;
        float (*verts)[3];
        cplane_s *planes;
};

struct PhysGeomInfo // sizeof=0x44
{
        BrushWrapper *brush;
        int type;
        float orientation[3][3];
        float offset[3];
        float halfLengths[3];
};

struct PhysGeomList // sizeof=0xC
{
        unsigned int count;
        PhysGeomInfo *geoms;
        int contents;
};

struct CollisionBorder // sizeof=0x1C
{
        float distEq[3];
        float zBase;
        float zSlope;
        float start;
        float length;
};

const struct CollisionPartition // sizeof=0x14
{
        unsigned __int8 triCount;
        unsigned __int8 borderCount;
        // padding byte
        // padding byte
        int firstTri;
        int nuinds;
        int fuind;
        CollisionBorder *borders;
};

struct CollisionAabbTree;


// TreyBO's geoms take neutral descriptions instead of the clipmap types. The collision tree, partition and
// border layouts are the clipmap's, so those are passed straight through.
static_assert(sizeof(CollisionBorder) == sizeof(phys_collision_border));
static_assert(sizeof(CollisionPartition) == sizeof(phys_collision_partition));
static_assert(offsetof(CollisionPartition, borders) == offsetof(phys_collision_partition, borders));

// The cbrush_t behind a brush geom (or an aabb geom made from a sideless brush); NULL for anything else.
inline const cbrush_t *gjk_get_cbrush(const gjk_base_t *geom)
{
    const phys_brush_desc *const desc = geom->get_brush();
    return desc ? (const cbrush_t *)desc->user_data : NULL;
}

inline const CollisionPartition *gjk_get_partition(const gjk_partition_t *geom)
{
    return (const CollisionPartition *)geom->partition;
}

inline const CollisionAabbTree *gjk_get_tree(const gjk_partition_t *geom)
{
    return (const CollisionAabbTree *)geom->tree;
}

PhysGeomList *__cdecl xmodel_get_geomlist(const XModel *model, int bone_index);

// Builds the phys_brush_desc for the brush (query scratch for query visitors, heap otherwise) and creates the geom.
gjk_base_t *create_brush_gjk_geom(const cbrush_t *brush, int stype, gjk_collision_visitor *allocator);
// Surface type and contents come from the tree's clipmap material.
gjk_partition_t *create_partition_gjk_geom(const CollisionAabbTree *tree, gjk_collision_visitor *allocator);
// Frees the heap brush descs made by create_brush_gjk_geom, then the geoms.
void destroy_game_gjk_geom(gjk_geom_list_t *gjk_geom_list);

void __cdecl create_brush_model_gjk_geom_r(
                cLeafBrushNode_s *node,
                int *index_base,
                gjk_collision_visitor *allocator,
                int contents_mask);
void __cdecl query_brush_model_gjk_geom(
                unsigned __int16 brushModel,
                int contents_mask,
                gjk_collision_visitor *allocator);
void __cdecl create_brush_model_gjk_geom(
                unsigned __int16 brushModel,
                int contents_mask,
                gjk_collision_visitor *allocator);
void __cdecl create_xmodel_gjk_geom(
                const XModel *model,
                gjk_collision_visitor *allocator,
                int bone_index,
                unsigned int brush_mask,
                bool b_fallback_to_bounding_box,
                int contents_for_proxy_collision,
                phys_mat44 *worldMat);
void __cdecl create_gjk_geom(
                int localClientNum,
                const centity_s *cent,
                gjk_collision_visitor *allocator,
                bool calc_bone_mats,
                unsigned int brush_mask,
                bool b_fallback_to_bounding_box,
                bool b_use_smallerbox_for_characters);
void __cdecl create_gjk_dobj_geom(
                DObj *obj,
                const cpose_t *pose,
                gentity_s *gent,
                bool calc_bone_mats,
                unsigned int brush_mask,
                bool b_fallback_to_bounding_box,
                int contents_for_proxy_collision,
                gjk_collision_visitor *allocator);
void __cdecl create_gjk_geom(
                gentity_s *gent,
                gjk_collision_visitor *allocator,
                bool calc_bone_mats,
                unsigned int brush_mask,
                bool b_fallback_to_bounding_box);


void __cdecl create_gjk_geom(const Glass *glass, gjk_collision_visitor *allocator, unsigned int brush_mask);
void __cdecl create_gjk_geom(const DynEntityDef *dynEntDef, gjk_collision_visitor *allocator, unsigned int brush_mask);
