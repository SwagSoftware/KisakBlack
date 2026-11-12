#include "phys_colgeom.h"

#include <xanim/xmodel_utils.h>

#include <new>
#include <cgame/cg_world.h>
#include <qcommon/cm_load.h>
#include <bgame/bg_slidemove.h>
#include <qcommon/dobj_management.h>
#include <xanim/dobj_utils.h>
#include <clientscript/scr_const.h>
#include <DynEntity/DynEntity_client.h>
#include "phys_main.h"
#include <glass/glass.h>

create_gjk_geom_collision_visitor g_empty_collision_visitor;

PhysGeomList *__cdecl xmodel_get_geomlist(const XModel *model, int bone_index)
{
    iassert(model);

    if ( bone_index < 0 )
        return XModelGetCollmap(model, 0);
    else
        return XModelGetCollmapForBoneIndex(model, bone_index);
}


// ---------------------------------------------------------------------------------------------
// Clipmap -> TreyBO geom adapters
// ---------------------------------------------------------------------------------------------

gjk_base_t *create_brush_gjk_geom(const cbrush_t *brush, int stype, gjk_collision_visitor *allocator)
{
    iassert(brush);
    iassert(allocator);
    iassert(stype < SURF_TYPECOUNT);

    // The desc lives as long as the geom: query scratch for query geoms, heap (freed in destroy_game_gjk_geom) otherwise.
    phys_brush_desc *desc;
    if (allocator->is_query())
        desc = (phys_brush_desc *)allocator->allocate(sizeof(phys_brush_desc), 16, false);
    else
        desc = (phys_brush_desc *)PMM_ALLOC(sizeof(phys_brush_desc), alignof(phys_brush_desc));

    desc->mins[0] = brush->mins[0];
    desc->mins[1] = brush->mins[1];
    desc->mins[2] = brush->mins[2];
    desc->contents = brush->contents;
    desc->maxs[0] = brush->maxs[0];
    desc->maxs[1] = brush->maxs[1];
    desc->maxs[2] = brush->maxs[2];
    desc->numsides = brush->numsides;
    desc->numverts = brush->numverts;
    desc->verts = brush->verts;
    desc->user_data = brush;

    return gjk_brush_t::create(desc, stype, allocator);
}

gjk_partition_t *create_partition_gjk_geom(const CollisionAabbTree *tree, gjk_collision_visitor *allocator)
{
    iassert(tree);
    iassert(allocator);

    const dmaterial_t *materialInfo = &cm.materials[tree->materialIndex];
    const int stype = (materialInfo->surfaceFlags & 0x3F00000) >> 20;
    iassert(stype < SURF_TYPECOUNT);

    phys_partition_world_desc world;
    world.verts = cm.verts;
    world.uinds = cm.uinds;
    world.partitions = (const phys_collision_partition *)cm.partitions;

    return gjk_partition_t::create((const phys_collision_aabb_tree *)tree, world, stype, materialInfo->contentFlags, allocator);
}

void destroy_game_gjk_geom(gjk_geom_list_t *gjk_geom_list)
{
    for (gjk_base_t *geom = gjk_geom_list->m_first_geom; geom; geom = geom->m_next_geom)
    {
        const phys_brush_desc *desc = geom->get_brush();
        if (desc)
            PMM_FREE((void *)desc, sizeof(phys_brush_desc), alignof(phys_brush_desc));
    }
    destroy_gjk_geom(gjk_geom_list);
}

void __cdecl create_brush_model_gjk_geom_r(
                cLeafBrushNode_s *node,
                int *index_base,
                gjk_collision_visitor *allocator,
                int contents_mask)
{
    gjk_base_t *gjk_geom; // [esp+0h] [ebp-18h]
    cbrush_t *brush; // [esp+Ch] [ebp-Ch]
    int i; // [esp+14h] [ebp-4h]
    int savedregs; // [esp+18h] [ebp+0h] BYREF

    if ( node->leafBrushCount )
    {
        if ( node->leafBrushCount > 0 )
        {
            for ( i = 0; i < node->leafBrushCount; ++i )
            {
                brush = &cm.brushes[node->data.leaf.brushes[i]];
                if ( (contents_mask & brush->contents) != 0
                    && allocator->query_create_prolog_1(brush->mins, brush->maxs, brush) )
                {
                    gjk_geom = create_brush_gjk_geom(
                        brush,
                        (brush->axial_sflags[0][0] & 0x3F00000) >> 20,
                        allocator);
                    allocator->query_create_epilog_1(gjk_geom);
                }
            }
            *index_base += node->leafBrushCount;
            return;
        }
        create_brush_model_gjk_geom_r(node + 1, index_base, allocator, contents_mask);
    }
    if ( node->data.children.childOffset[0] || node->data.children.childOffset[1] )
    {
        create_brush_model_gjk_geom_r(&node[node->data.children.childOffset[0]], index_base, allocator, contents_mask);
        create_brush_model_gjk_geom_r(&node[node->data.children.childOffset[1]], index_base, allocator, contents_mask);
    }
}

void __cdecl query_brush_model_gjk_geom(
                unsigned __int16 brushModel,
                int contents_mask,
                gjk_collision_visitor *allocator)
{
    cmodel_t *model; // [esp+94h] [ebp-4h]

    if ( (brushModel == 0x3FFF || brushModel == 0x4000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.cpp",
                    516,
                    0,
                    "%s",
                    "( brushModel != CAPSULE_MODEL_HANDLE ) && ( brushModel != TEMP_BRUSH_MODEL_HANDLE )") )
    {
        __debugbreak();
    }
    if ( !allocator
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.cpp", 517, 0, "%s", "allocator") )
    {
        __debugbreak();
    }
    query_brush_model_gjk_geom_visitor query_visitor; // [esp+18h] [ebp-80h] BYREF
    //colgeom_visitor_t::colgeom_visitor_t(&query_visitor);
    //query_visitor.__vftable = (query_brush_model_gjk_geom_visitor_vtbl *)&query_brush_model_gjk_geom_visitor::`vftable';
    allocator->get_local_query_aabb((float *)&query_visitor.m_mn, (float *)&query_visitor.m_mx);
    query_visitor.m_mask = contents_mask;
    query_visitor.m_allocator = allocator;
    model = CM_ClipHandleToModel(brushModel);
    //colgeom_visitor_t::intersect_box_brushes(&query_visitor, &model->leaf);
    query_visitor.intersect_box_brushes(&model->leaf);
}

void query_brush_model_gjk_geom_visitor::visit(const CollisionAabbTree *)
{
    iassert(0);
}

void query_brush_model_gjk_geom_visitor::visit(const cbrush_t *brush)
{
    gjk_base_t *gjk_geom; // [esp+4h] [ebp-Ch]
    int savedregs; // [esp+10h] [ebp+0h] BYREF

    if ( !this->m_allocator
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.cpp", 497, 0, "%s", "m_allocator") )
    {
        __debugbreak();
    }
    if ( this->m_allocator->query_create_prolog_1(brush->mins, brush->maxs, brush) )
    {
        gjk_geom = create_brush_gjk_geom(brush, (brush->axial_sflags[0][0] & 0x3F00000) >> 20, this->m_allocator);
        this->m_allocator->query_create_epilog_1(gjk_geom);
    }
}

void query_brush_model_gjk_geom_visitor::update(
                const float *_mn,
                const float *_mx,
                int _mask,
                const float *expand_vec)
{
    ;
}

void __cdecl create_brush_model_gjk_geom(
                unsigned __int16 brushModel,
                int contents_mask,
                gjk_collision_visitor *allocator)
{
    cLeafBrushNode_s *node; // [esp+0h] [ebp-Ch]
    int index_base; // [esp+8h] [ebp-4h] BYREF

    if ( !allocator
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.cpp", 532, 0, "%s", "allocator") )
    {
        __debugbreak();
    }
    if ( allocator->is_query() )
    {
        query_brush_model_gjk_geom(brushModel, contents_mask, allocator);
    }
    else
    {
        if ( (brushModel == 0x3FFF || brushModel == 0x4000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.cpp",
                        539,
                        0,
                        "%s",
                        "( brushModel != CAPSULE_MODEL_HANDLE ) && ( brushModel != TEMP_BRUSH_MODEL_HANDLE )") )
        {
            __debugbreak();
        }
        node = &cm.leafbrushNodes[CM_ClipHandleToModel(brushModel)->leaf.leafBrushNode];
        index_base = 0;
        create_brush_model_gjk_geom_r(node, &index_base, allocator, contents_mask);
    }
}

float r_0 = 5.0f;
void __cdecl create_xmodel_gjk_geom(
                const XModel *model,
                gjk_collision_visitor *allocator,
                int bone_index,
                unsigned int brush_mask,
                bool b_fallback_to_bounding_box,
                int contents_for_proxy_collision,
                phys_mat44 *worldMat)
{
    float obj; // [esp+1Ch] [ebp-E0h]
    int surfFlags; // [esp+20h] [ebp-DCh]
    gjk_base_t *v9; // [esp+68h] [ebp-94h]
    gjk_base_t *cylinder_gjk_geom; // [esp+6Ch] [ebp-90h]
    float hheight; // [esp+70h] [ebp-8Ch]
    float v12[3]; // [esp+74h] [ebp-88h] BYREF
    float rvec[3]; // [esp+80h] [ebp-7Ch]
    float radius; // [esp+8Ch] [ebp-70h]
    float v15[3]; // [esp+90h] [ebp-6Ch] BYREF
    float bs_radius; // [esp+9Ch] [ebp-60h]
    gjk_base_t *aabb_gjk_geom; // [esp+A0h] [ebp-5Ch]
    gjk_base_t *obb_gjk_geom; // [esp+A4h] [ebp-58h]
    int j; // [esp+A8h] [ebp-54h]
    float trasformed_v[3]; // [esp+ACh] [ebp-50h] BYREF
    float v[3]; // [esp+B8h] [ebp-44h] BYREF
    int i; // [esp+C4h] [ebp-38h]
    gjk_base_t *gjk_geom; // [esp+C8h] [ebp-34h]
    const cbrush_t *brush; // [esp+CCh] [ebp-30h]
    float mins[3]; // [esp+D0h] [ebp-2Ch] BYREF
    PhysGeomInfo *geom; // [esp+DCh] [ebp-20h]
    int stype; // [esp+E0h] [ebp-1Ch]
    float maxs[3]; // [esp+E4h] [ebp-18h] BYREF
    bool b_geom_eligible; // [esp+F3h] [ebp-9h]
    PhysGeomList *geomList; // [esp+F4h] [ebp-8h]
    unsigned int geomIndex; // [esp+F8h] [ebp-4h]
    int savedregs; // [esp+FCh] [ebp+0h] BYREF

    if (worldMat)
    {
        assert_mat44_initialized(*worldMat);
    }

    if ( !allocator
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.cpp", 562, 0, "%s", "allocator") )
    {
        __debugbreak();
    }
    if ( (bone_index < -1 || bone_index >= model->numBones)
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.cpp",
                 569,
                 "bone_index >= -1 && bone_index < model->numBones",
                 "Bad param to create_gjk_geom xmodel version.") )
    {
        __debugbreak();
    }
    if ( model->collSurfs )
        surfFlags = model->collSurfs->surfFlags;
    else
        surfFlags = 0xD00000;
    stype = (surfFlags & 0x3F00000) >> 20;
    geomList = xmodel_get_geomlist(model, bone_index);
    b_geom_eligible = 0;
    if ( geomList )
    {
        geomIndex = 0;
        geom = geomList->geoms;
        while ( geomIndex < geomList->count )
        {
            if ( geom->brush )
            {
                if ( (brush_mask & geom->brush->contents) != 0 )
                {
                    b_geom_eligible = 1;
                    brush = (const cbrush_t *)geom->brush;
                    if ( allocator->query_create_prolog_1(brush->mins, brush->maxs, geom) )
                    {
                        gjk_geom = create_brush_gjk_geom(brush, stype, allocator);
                        if (worldMat)
                        {
                            //gjk_base_t::set_xform(gjk_geom, worldMat);
                            gjk_geom->set_xform(worldMat);
                        }
                        allocator->query_create_epilog_1(gjk_geom);
                    }
                }
            }
            else
            {
                b_geom_eligible = 1;
                if ( geom->type == 1 )
                {
                    if ( (float)((float)((float)(geom->orientation[0][0] * geom->orientation[0][0])
                                                         + (float)(geom->orientation[0][1] * geom->orientation[0][1]))
                                         + (float)(geom->orientation[0][2] * geom->orientation[0][2])) <= 0.0099999998
                        && (float)((float)((float)(geom->orientation[1][0] * geom->orientation[1][0])
                                                         + (float)(geom->orientation[1][1] * geom->orientation[1][1]))
                                         + (float)(geom->orientation[1][2] * geom->orientation[1][2])) <= 0.0099999998
                        && (float)((float)((float)(geom->orientation[2][0] * geom->orientation[2][0])
                                                         + (float)(geom->orientation[2][1] * geom->orientation[2][1]))
                                         + (float)(geom->orientation[2][2] * geom->orientation[2][2])) <= 0.0099999998 )
                    {
                        mins[0] = geom->offset[0] - geom->halfLengths[0];
                        mins[1] = geom->offset[1] - geom->halfLengths[1];
                        mins[2] = geom->offset[2] - geom->halfLengths[2];
                        maxs[0] = geom->offset[0] + geom->halfLengths[0];
                        maxs[1] = geom->offset[1] + geom->halfLengths[1];
                        maxs[2] = geom->offset[2] + geom->halfLengths[2];
                        if ( allocator->query_create_prolog_1(mins, maxs, geom) )
                        {
                            //aabb_gjk_geom = create_aabb_gjk_geom(COERCE_FLOAT(&savedregs), mins, maxs, stype, allocator);
                            aabb_gjk_geom = create_aabb_gjk_geom(mins, maxs, stype, allocator);
                            if (worldMat)
                            {
                                //gjk_base_t::set_xform(aabb_gjk_geom, worldMat);
                                aabb_gjk_geom->set_xform(worldMat);
                            }
                            //gjk_base_t::set_contents(aabb_gjk_geom, contents_for_proxy_collision);
                            aabb_gjk_geom->set_contents(contents_for_proxy_collision);

                            allocator->query_create_epilog_1(aabb_gjk_geom);
                        }
                    }
                    else
                    {
                        for ( i = 0; i < 8; ++i )
                        {
                            for ( j = 0; j < 3; ++j )
                            {
                                if ( (i & (1 << j)) != 0 )
                                    obj = geom->halfLengths[j];
                                else
                                    obj = -geom->halfLengths[j];
                                v[j] = obj;
                            }
                            MatrixTransformVector(v, geom->orientation, trasformed_v);
                            Vec3Min(mins, trasformed_v, mins);
                            Vec3Max(maxs, trasformed_v, maxs);
                        }
                        mins[0] = mins[0] + geom->offset[0];
                        mins[1] = mins[1] + geom->offset[1];
                        mins[2] = mins[2] + geom->offset[2];
                        maxs[0] = maxs[0] + geom->offset[0];
                        maxs[1] = maxs[1] + geom->offset[1];
                        maxs[2] = maxs[2] + geom->offset[2];
                        //if ( allocator->query_create_prolog_1(allocator, mins, maxs, geom) )
                        if ( allocator->query_create_prolog_1(mins, maxs, geom) )
                        {
                            obb_gjk_geom = create_obb_gjk_geom(
                                                             geom->orientation,
                                                             geom->offset,
                                                             geom->halfLengths,
                                                             stype,
                                                             allocator);
                            if (worldMat)
                            {
                                //gjk_base_t::set_xform(obb_gjk_geom, worldMat);
                                obb_gjk_geom->set_xform(worldMat);
                            }
                            //gjk_base_t::set_contents(obb_gjk_geom, contents_for_proxy_collision);
                            obb_gjk_geom->set_contents(contents_for_proxy_collision);
                            allocator->query_create_epilog_1(obb_gjk_geom);
                        }
                    }
                }
                else
                {
                    radius = geom->halfLengths[1];
                    hheight = geom->halfLengths[0];
                    bs_radius = radius + hheight;
                    if ( (float)((float)(radius + hheight) * (float)(radius + hheight)) < (float)((float)(radius * radius)
                                                                                                                                                                            + (float)(hheight * hheight))
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.cpp",
                                    652,
                                    0,
                                    "%s",
                                    "bs_radius * bs_radius >= radius * radius + hheight * hheight") )
                    {
                        __debugbreak();
                    }
                    rvec[0] = bs_radius;
                    rvec[1] = bs_radius;
                    rvec[2] = bs_radius;
                    v12[0] = geom->offset[0] - bs_radius;
                    v12[1] = geom->offset[1] - bs_radius;
                    v12[2] = geom->offset[2] - bs_radius;
                    v15[0] = geom->offset[0] + bs_radius;
                    v15[1] = geom->offset[1] + bs_radius;
                    v15[2] = geom->offset[2] + bs_radius;
                    if ( allocator->query_create_prolog_1(v12, v15, geom) )
                    {
                        cylinder_gjk_geom = create_cylinder_gjk_geom(
                                                                    geom->orientation,
                                                                    geom->offset,
                                                                    radius,
                                                                    hheight,
                                                                    stype,
                                                                    allocator);
                        if (worldMat)
                        {
                            //gjk_base_t::set_xform(cylinder_gjk_geom, worldMat);
                            cylinder_gjk_geom->set_xform(worldMat);
                        }
                        //gjk_base_t::set_contents(cylinder_gjk_geom, contents_for_proxy_collision);
                        cylinder_gjk_geom->set_contents(contents_for_proxy_collision);

                        allocator->query_create_epilog_1(cylinder_gjk_geom);
                    }
                }
            }
            ++geomIndex;
            ++geom;
        }
    }
    if ( !b_geom_eligible && b_fallback_to_bounding_box )
    {
        XModelGetBounds(model, mins, maxs);
        if ( mins[0] == maxs[0] || mins[1] == maxs[1] || mins[2] == maxs[2] )
        {
            mins[0] = -r_0;
            mins[1] = -r_0;
            mins[2] = -r_0;
            maxs[0] = r_0;
            maxs[1] = r_0;
            maxs[2] = r_0;
        }
        if ( allocator->query_create_prolog_1(mins, maxs, model) )
        {
            v9 = create_aabb_gjk_geom(mins, maxs, stype, allocator);
            if (worldMat)
            {
                //gjk_base_t::set_xform(v9, worldMat);
                v9->set_xform(worldMat);
            }
            //gjk_base_t::set_contents(v9, contents_for_proxy_collision);
            v9->set_contents(contents_for_proxy_collision);
            allocator->query_create_epilog_1(v9);
        }
    }
}

void __cdecl adjust_gjk_polygon_cylinder_query_aabb(float *mins, float *maxs)
{
    //float v2; // xmm0_4

    //if ((_S8 & 1) == 0)
    //{
    //    _S8 |= 1u;
    //    v2 = __libm_sse2_cos((float)((float)(6.2831855 / 12.0) / 2.0));
    //    radius_scale_factor_0 = 1.0 / v2;
    //}

    static float radius_scale_factor_0 = 1.0f / cosf((6.2831855f / 12.0f) / 2.0f);

    mins[0] -= 0.125f;
    mins[1] -= 0.125f;
    mins[2] -= 0.125f;

    maxs[0] += 0.125f;
    maxs[1] += 0.125f;
    maxs[2] += 0.125f;

    mins[0] *= radius_scale_factor_0;
    mins[1] *= radius_scale_factor_0;
    mins[2] *= radius_scale_factor_0;

    maxs[0] *= radius_scale_factor_0;
    maxs[1] *= radius_scale_factor_0;
    maxs[2] *= radius_scale_factor_0;
}

void __cdecl create_gjk_geom(
                int localClientNum,
                const centity_s *cent,
                gjk_collision_visitor *allocator,
                bool calc_bone_mats,
                unsigned int brush_mask,
                bool b_fallback_to_bounding_box,
                bool b_use_smallerbox_for_characters)
{
    DObj *ClientDObj; // eax
    const DObj *dobj; // [esp+0h] [ebp-48h]
    gjk_polygon_cylinder_t *gjk_geom; // [esp+Ch] [ebp-3Ch]
    float mins[3]; // [esp+10h] [ebp-38h] BYREF
    int stype; // [esp+1Ch] [ebp-2Ch]
    float maxs[3]; // [esp+20h] [ebp-28h] BYREF
    float query_maxs[3]; // [esp+2Ch] [ebp-1Ch] BYREF
    float query_mins[3]; // [esp+38h] [ebp-10h] BYREF
    int contents_for_proxy_collision; // [esp+44h] [ebp-4h]
    int savedregs; // [esp+48h] [ebp+0h] BYREF

    if ( !allocator
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.cpp", 765, 0, "%s", "allocator") )
    {
        __debugbreak();
    }
    dobj = Com_GetClientDObj(cent->nextState.number, localClientNum);
    if ( dobj )
        contents_for_proxy_collision = CG_GetEntityDObjContents(cent, dobj);
    else
        contents_for_proxy_collision = -1;
    if ( cent->nextState.eType == 17 || cent->nextState.eType == 1 )
    {
        stype = 29;
        if ( cent->nextState.solid )
        {
            CG_GetEntityBModelBounds(cent, mins, maxs, 0, 0);
            mins[0] = mins[0] - 1.0;
            mins[1] = mins[1] - 1.0;
            mins[2] = mins[2] - 1.0;
            maxs[0] = maxs[0] + 1.0;
            maxs[1] = maxs[1] + 1.0;
            maxs[2] = maxs[2] + 1.0;
        }
        else
        {
            mins[0] = -15.0f;
            mins[1] = -15.0f;
            mins[2] = 0.0f;
            maxs[0] = 15.0f;
            maxs[1] = 15.0f;
            maxs[2] = 72.0f;
        }
        if ( b_use_smallerbox_for_characters )
            mins[2] = mins[2] + 15.0;
        query_mins[0] = mins[0];
        query_mins[1] = mins[1];
        query_mins[2] = mins[2];
        query_maxs[0] = maxs[0];
        query_maxs[1] = maxs[1];
        query_maxs[2] = maxs[2];
        adjust_gjk_polygon_cylinder_query_aabb(query_mins, query_maxs);
        if ( allocator->query_create_prolog_1(query_mins, query_maxs, cent) )
        {
            gjk_geom = create_gjkcc_gjk_geom((float (*)[3])mins, (float (*)[3])maxs, stype, allocator);
            //gjk_base_t::set_contents(gjk_geom, contents_for_proxy_collision);
            gjk_geom->set_contents(contents_for_proxy_collision);
            allocator->query_create_epilog_1(gjk_geom);
        }
    }
    else if (cent->nextState.solid != 0xFFFFFF || cent->nextState.eType == 14 || cent->nextState.eType == 16)
    {
        ClientDObj = Com_GetClientDObj(cent->nextState.number, localClientNum);
        create_gjk_dobj_geom(
            ClientDObj,
            &cent->pose,
            0,
            calc_bone_mats,
            brush_mask,
            b_fallback_to_bounding_box,
            contents_for_proxy_collision,
            allocator);
    }
    else
    {
        create_brush_model_gjk_geom(cent->nextState.index.bone, brush_mask, allocator);
    }
}

void __cdecl create_gjk_dobj_geom(
                DObj *obj,
                const cpose_t *pose,
                gentity_s *gent,
                bool calc_bone_mats,
                unsigned int brush_mask,
                bool b_fallback_to_bounding_box,
                int contents_for_proxy_collision,
                gjk_collision_visitor *allocator)
{
    XModel *model; // [esp+0h] [ebp-4h]

    if ( !allocator
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.cpp", 693, 0, "%s", "allocator") )
    {
        __debugbreak();
    }
    if ( obj )
    {
        model = DObjGetModel(obj, 0);
        if ( model->numCollmaps )
        {
            if ( xmodel_get_geomlist(model, -1) )
                create_xmodel_gjk_geom(
                    model,
                    allocator,
                    -1,
                    brush_mask,
                    b_fallback_to_bounding_box,
                    contents_for_proxy_collision,
                    0);
        }
        else if ( b_fallback_to_bounding_box )
        {
            create_xmodel_gjk_geom(
                model,
                allocator,
                -1,
                brush_mask,
                b_fallback_to_bounding_box,
                contents_for_proxy_collision,
                0);
        }
    }
}

void __cdecl create_gjk_geom(
                gentity_s *gent,
                gjk_collision_visitor *allocator,
                bool calc_bone_mats,
                unsigned int brush_mask,
                bool b_fallback_to_bounding_box)
{
    DObj *ServerDObj; // eax
    gjk_polygon_cylinder_t *gjk_geom; // [esp+10h] [ebp-24h]
    float query_maxs[3]; // [esp+18h] [ebp-1Ch] BYREF
    float query_mins[3]; // [esp+24h] [ebp-10h] BYREF
    int contents_for_proxy_collision; // [esp+30h] [ebp-4h]

    iassert(allocator);

    contents_for_proxy_collision = gent->r.contents;
    if ( gent->s.eType == ET_ACTOR || gent->s.eType == ET_PLAYER )
    {
        query_mins[0] = gent->r.mins[0];
        query_mins[1] = gent->r.mins[1];
        query_mins[2] = gent->r.mins[2];
        query_maxs[0] = gent->r.maxs[0];
        query_maxs[1] = gent->r.maxs[1];
        query_maxs[2] = gent->r.maxs[2];
        adjust_gjk_polygon_cylinder_query_aabb(query_mins, query_maxs);
        if ( allocator->query_create_prolog_1(query_mins, query_maxs, gent) )
        {
            gjk_geom = create_gjkcc_gjk_geom((float (*)[3])gent->r.mins, (float (*)[3])gent->r.maxs, 29, allocator);
            //gjk_base_t::set_contents(gjk_geom, contents_for_proxy_collision);
            gjk_geom->set_contents(contents_for_proxy_collision);
            allocator->query_create_epilog_1(gjk_geom);
        }
    }
    else if ( gent->classname == scr_const.script_brushmodel || gent->classname == scr_const.glass )
    {
        create_brush_model_gjk_geom(gent->s.index.bone, brush_mask, allocator);
    }
    else
    {
        ServerDObj = Com_GetServerDObj(gent->s.number);
        create_gjk_dobj_geom(
            ServerDObj,
            0,
            gent,
            calc_bone_mats,
            brush_mask,
            b_fallback_to_bounding_box,
            contents_for_proxy_collision,
            allocator);
    }
}

void __cdecl create_gjk_geom(const Glass *glass, gjk_collision_visitor *allocator, unsigned int brush_mask)
{
    iassert(allocator);

    create_brush_model_gjk_geom(glass->brushModel, brush_mask, allocator);
}

void __cdecl create_gjk_geom(const DynEntityDef *dynEntDef, gjk_collision_visitor *allocator, unsigned int brush_mask)
{
    gjk_base_t *gjk_geom; // [esp+0h] [ebp-24h]
    int stype; // [esp+4h] [ebp-20h]
    float mins[3]; // [esp+8h] [ebp-1Ch] BYREF
    float maxs[3]; // [esp+14h] [ebp-10h] BYREF
    int contents_for_proxy_collision; // [esp+20h] [ebp-4h]
    int savedregs; // [esp+24h] [ebp+0h] BYREF

    if ( !allocator
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.cpp", 869, 0, "%s", "allocator") )
    {
        __debugbreak();
    }
    contents_for_proxy_collision = dynEntDef->contents;
    if ( dynEntDef->xModel && XModelHasCollmap(dynEntDef->xModel) )
    {
        create_xmodel_gjk_geom(dynEntDef->xModel, allocator, -1, brush_mask, 1, contents_for_proxy_collision, 0);
    }
    else if ( !dynEntDef->xModel || dynEntDef->physicsBrushModel )
    {
        if ( dynEntDef->physicsBrushModel )
            create_brush_model_gjk_geom(dynEntDef->physicsBrushModel, brush_mask, allocator);
        else
            create_brush_model_gjk_geom(dynEntDef->brushModel, brush_mask, allocator);
    }
    else
    {
        DynEnt_GetLocalBounds(dynEntDef, mins, maxs);
        if ( allocator->query_create_prolog_1(mins, maxs, dynEntDef) )
        {
            stype = DynEnt_GetSurfaceType(dynEntDef);
            gjk_geom = create_aabb_gjk_geom(mins, maxs, stype, allocator);
            //gjk_base_t::set_contents(gjk_geom, contents_for_proxy_collision);
            gjk_geom->set_contents(contents_for_proxy_collision);
            allocator->query_create_epilog_1(gjk_geom);
        }
    }
}
