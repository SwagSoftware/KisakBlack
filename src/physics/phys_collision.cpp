#include "phys_collision.h"
#include <DynEntity/DynEntity_client.h>
#include <bgame/bg_slidemove.h>
#include <cgame/cg_drawtools.h>
#include "phys_render.h"
#include <universal/com_math_anglevectors.h>
#include <DynEntity/DynEntity_load_obj.h>
#include <new>
#include <qcommon/cm_load.h>
#include <cgame/cg_world.h>
#include <qcommon/dobj_management.h>
#include <xanim/dobj_utils.h>
#include <xanim/xmodel_utils.h>
#include "phys_auto_rigid_body.h"
#include <cgame_mp/cg_vehicles_mp.h>
#include <glass/glass_client.h>
#include <cgame_mp/cg_local_mp.h>
#include <algorithm>

phys_simple_allocator<generic_avl_map_node_t> g_generic_avl_map_node_allocator;
standard_query g_standard_query;

void __cdecl set_cpi_params(contact_point_info *cpi, phys_collision_pair *pcp)
{
    const phys_surface_type_info *psti; // [esp+1Ch] [ebp-20h]
    const PhysObjUserData *userData2; // [esp+20h] [ebp-1Ch]
    broad_phase_info *bpi1; // [esp+24h] [ebp-18h]
    float bounce; // [esp+28h] [ebp-14h]
    unsigned int priority; // [esp+2Ch] [ebp-10h]
    broad_phase_info *bpi2; // [esp+30h] [ebp-Ch]
    const PhysObjUserData *userData1; // [esp+34h] [ebp-8h]
    float friction; // [esp+38h] [ebp-4h]

    bpi1 = pcp->m_bpi1;
    bpi2 = pcp->m_bpi2;
    userData1 = (const PhysObjUserData *)bpi1->m_rb->m_userdata;
    userData2 = (const PhysObjUserData *)bpi2->m_rb->m_userdata;
    if (!userData1
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp", 59, 0, "%s", "userData1"))
    {
        __debugbreak();
    }
    priority = 0;
    if (userData1->vehicle)
    {
        priority = 1;
        friction = phys_vehicleFriction->current.value;
        bounce = userData1->bounce;
        if (userData1->vehicle->m_vehicle_info->type == 4 && cpi->m_normal.z < -0.69999999)
            friction = userData1->friction;
        if (bpi1->m_rb->m_mat.z.z < -0.1)
            friction = 1.0f;
    }
    else if (userData2)
    {
        friction = (float)(userData1->friction + userData2->friction) * 0.5;
        bounce = (float)(userData1->bounce + userData2->bounce) * 0.5;
    }
    else
    {
        psti = &surface_type_info_database_get(bpi1->m_surface_type, bpi2->m_surface_type);
        friction = userData1->friction * psti->m_friction_coef;
        bounce = userData1->bounce * psti->m_bounce_coef;
    }
    cpi->m_fric_coef = friction;
    cpi->m_bounce_coef = bounce;
    cpi->m_max_restitution_vel = 3400.0f;
    //contact_point_info::check_surface_properties(cpi);
    cpi->check_surface_properties();
    //contact_point_info::set_solver_priority(cpi, priority);
    cpi->set_solver_priority(priority);
    cpi->m_flags &= ~8u;
    //contact_point_info::set_rb2_entity(cpi, pcp->m_bpi2);
    cpi->set_rb2_entity(pcp->m_bpi2);
}

const centity_s *get_entity(gjk_physics_collision_visitor *collision_visitor)
{
    if (collision_visitor->cent)
        return collision_visitor->cent;
    if (collision_visitor->glass)
        return (const centity_s *)collision_visitor->glass;

    iassert(collision_visitor->dynEntDef);
    //if (!collision_visitor->dynEntDef
    //    && !Assert_MyHandler(
    //        a1,
    //        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
    //        405,
    //        0,
    //        "%s",
    //        "collision_visitor->dynEntDef"))
    //{
    //    __debugbreak();
    //}
    return (const centity_s *)collision_visitor->dynEntDef;
}

const bpei_database_id *__cdecl get_database_id(bpei_database_id *result, gjk_physics_collision_visitor *collision_visitor)
{
    if (collision_visitor->cent)
    {
        result->m_id1 = (unsigned int)collision_visitor->cent;
        result->m_id2 = 0;
        return result;
    }
    else if (collision_visitor->glass)
    {
        result->m_id1 = (unsigned int)collision_visitor->glass;
        result->m_id2 = 0;
        return result;
    }
    else
    {
        if (!collision_visitor->dynEntDef
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
                418,
                0,
                "%s",
                "collision_visitor->dynEntDef"))
        {
            __debugbreak();
        }
        result->m_id1 = (unsigned int)collision_visitor->dynEntDef;
        result->m_id2 = 0;
        return result;
    }
}

phys_mat44 *__cdecl create_ent_mat(gjk_physics_collision_visitor *collision_visitor)
{
    phys_mat44 *v3; // [esp+0h] [ebp-A8h]
    float v4[9]; // [esp+8h] [ebp-A0h] BYREF
    DynEntityDrawType v5; // [esp+2Ch] [ebp-7Ch]
    const float *origin; // [esp+30h] [ebp-78h]
    float v7[9]; // [esp+38h] [ebp-70h] BYREF
    const float *inVector; // [esp+5Ch] [ebp-4Ch]
    float axis[9]; // [esp+64h] [ebp-44h] BYREF
    int v11; // [esp+8Ch] [ebp-1Ch]
    int v12; // [esp+90h] [ebp-18h]
    phys_mat44 *v13; // [esp+94h] [ebp-14h]
    DynEntityPose *dynEntPose; // [esp+98h] [ebp-10h]
    DynEntityDrawType drawType; // [esp+9Ch] [ebp-Ch]
    unsigned __int16 dynEntId; // [esp+A0h] [ebp-8h]
    phys_mat44 *ent_mat; // [esp+A4h] [ebp-4h]

    v12 = 4;
    v11 = 16;
    v13 = (phys_mat44 *)collision_visitor->allocate(sizeof(phys_mat44), 16, 0);
    if (v13)
        v3 = v13;
    else
        v3 = 0;
    ent_mat = v3;
    if (collision_visitor->cent)
    {
        inVector = collision_visitor->cent->pose.origin;
        AnglesToAxis(collision_visitor->cent->pose.angles, (float (*)[3])axis);
        Phys_AxisToNitrousMat((float (*)[3])axis, *ent_mat);
        Phys_Vec3ToNitrousVec(inVector, ent_mat->w);
    }
    else if (collision_visitor->glass)
    {
        origin = collision_visitor->glass->origin;
        AnglesToAxis(collision_visitor->glass->angles, (float (*)[3])v7);
        Phys_AxisToNitrousMat((float (*)[3])v7, *ent_mat);
        Phys_Vec3ToNitrousVec(origin, ent_mat->w);
    }
    else
    {
        if (!collision_visitor->dynEntDef
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
                361,
                0,
                "%s",
                "collision_visitor->dynEntDef"))
        {
            __debugbreak();
        }
        v5 = (DynEntityDrawType)(collision_visitor->dynEntDef->xModel == 0);
        drawType = v5;
        dynEntId = DynEnt_GetId(collision_visitor->dynEntDef, v5);
        dynEntPose = DynEnt_GetClientPose(dynEntId, v5);
        UnitQuatToAxis(dynEntPose->pose.quat, (float (*)[3])v4);
        Phys_AxisToNitrousMat((float (*)[3])v4, *ent_mat);
        Phys_Vec3ToNitrousVec(dynEntPose->pose.origin, ent_mat->w);
    }
    return ent_mat;
}

phys_auto_activate_callback *__cdecl create_ent_aac(gjk_physics_collision_visitor *collision_visitor)
{
    DynEntityDef *dynEntDef; // [esp+10h] [ebp-30h]
    //phys_auto_activate_callback_vtbl *cent; // [esp+18h] [ebp-28h]
    dynamic_ent_aa *v6; // [esp+28h] [ebp-18h]
    destructible_ent_aa *v7; // [esp+34h] [ebp-Ch]

    if (collision_visitor->cent)
    {
        if (collision_visitor->cent->destructible && (collision_visitor->bpeqi->env_collision_flags & 8) != 0)
        {
            v7 = (destructible_ent_aa *)collision_visitor->allocate(sizeof(destructible_ent_aa), 4, 0);
            if (!v7)
                return 0;
            //cent = (phys_auto_activate_callback_vtbl *)collision_visitor->cent;
            //v7->__vftable = (destructible_ent_aa_vtbl *)&phys_auto_activate_callback::`vftable';
            //v7->__vftable = (destructible_ent_aa_vtbl *)&destructible_ent_aa::`vftable';
            new ((void *)v7) destructible_ent_aa();
            
            v7->m_has_auto_activated = 0;
            v7->m_cent = (centity_s*)collision_visitor->cent; // de-const cast
            return (dynamic_ent_aa *)v7;
        }
        else
        {
            return 0;
        }
    }
    else if (collision_visitor->glass)
    {
        return 0;
    }
    else
    {
        if (!collision_visitor->dynEntDef
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
                391,
                0,
                "%s",
                "collision_visitor->dynEntDef"))
        {
            __debugbreak();
        }
        v6 = (dynamic_ent_aa *)collision_visitor->allocate(sizeof(dynamic_ent_aa), 4, 0);
        if (!v6)
            return 0;
        dynEntDef = (DynEntityDef*)collision_visitor->dynEntDef;

        
        //v6->__vftable = (dynamic_ent_aa_vtbl *)&phys_auto_activate_callback::`vftable';
        //v6->__vftable = (dynamic_ent_aa_vtbl *)&dynamic_ent_aa::`vftable';
        new ((void *)v6) dynamic_ent_aa();
        v6->m_has_auto_activated = 0;
        v6->m_dynEntDef = dynEntDef;
        return v6;
    }
}

void create_entity_bpi(gjk_physics_collision_visitor *collision_visitor, int mask)
{
    const centity_s *entity; // eax
    bpei_database_id database_id; // [esp+34h] [ebp-10h] BYREF
    broad_phase_environment_info *bpei; // [esp+3Ch] [ebp-8h]
    entity_bpi_header *ebpih = NULL; // [esp+40h] [ebp-4h]
    //int savedregs; // [esp+44h] [ebp+0h] BYREF

    get_database_id(&database_id, collision_visitor);
    bpei = G_BPM->g_bpei_database.get_bpei_mt(database_id);
    if (!bpei->m_data)
    {
        //minspec_mutex::Lock(&bpei->m_mutex);
        bpei->m_mutex.Lock();
        if (!bpei->m_data)
        {
            ebpih = (entity_bpi_header *)collision_visitor->allocate(sizeof(entity_bpi_header), 4, 0);
            if (collision_visitor->rb->is_environment_rigid_body())
                ebpih->m_mat = create_ent_mat(collision_visitor);
            else
                ebpih->m_mat = 0;
            ebpih->m_aac = create_ent_aac(collision_visitor);

            iassert(ebpih);
            bpei->m_data = ebpih;
        }
        //minspec_mutex::Unlock(&bpei->m_mutex);
        bpei->m_mutex.Unlock();
    }

    iassert(bpei->m_data);
    ebpih = (entity_bpi_header *)bpei->m_data;

    //if (rigid_body::is_environment_rigid_body(collision_visitor->rb))
    if (collision_visitor->rb->is_environment_rigid_body())
    {
        if (collision_visitor->cg_to_world_xform
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
                463,
                0,
                "%s",
                "collision_visitor->cg_to_world_xform == NULL"))
        {
            __debugbreak();
        }
        if (collision_visitor->cg_to_rb_xform
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
                464,
                0,
                "%s",
                "collision_visitor->cg_to_rb_xform == NULL"))
        {
            __debugbreak();
        }
        if (!ebpih->m_mat
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
                465,
                0,
                "%s",
                "ebpih->m_mat"))
        {
            __debugbreak();
        }
        collision_visitor->cg_to_world_xform = ebpih->m_mat;
        collision_visitor->cg_to_rb_xform = ebpih->m_mat;
    }
    collision_visitor->auto_activate_callback = ebpih->m_aac;
    if (!collision_visitor->rb_to_world_xform
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
            472,
            0,
            "%s",
            "collision_visitor->rb_to_world_xform"))
    {
        __debugbreak();
    }
    if (!collision_visitor->cg_to_world_xform
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
            473,
            0,
            "%s",
            "collision_visitor->cg_to_world_xform"))
    {
        __debugbreak();
    }
    if (!collision_visitor->cg_to_rb_xform
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
            474,
            0,
            "%s",
            "collision_visitor->cg_to_rb_xform"))
    {
        __debugbreak();
    }
    entity = get_entity(collision_visitor);
    //gjk_physics_collision_visitor::set_local_query_info(collision_visitor, (int)&savedregs, entity);
    collision_visitor->set_local_query_info(entity);
    if (collision_visitor->cent)
    {
        create_gjk_geom(0, collision_visitor->cent, collision_visitor, 0, mask, 0, 1);
    }
    else if (collision_visitor->glass)
    {
        create_gjk_geom(collision_visitor->glass, collision_visitor, mask);
    }
    else
    {
        if (!collision_visitor->dynEntDef
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
                487,
                0,
                "%s",
                "collision_visitor->dynEntDef"))
        {
            __debugbreak();
        }
        create_gjk_geom(collision_visitor->dynEntDef, collision_visitor, mask);
    }
}


void __cdecl set_bp_standard_query()
{
    G_BPM->g_broad_phase_terrain_query_callback = &g_standard_query;
}

void __cdecl debug_callback(void*)
{
    if (phys_debugCallback->current.enabled)
    {
        if (_tlAssert("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp", 796, "0", ""))
            __debugbreak();
    }
}

void __cdecl set_debug_callback()
{
    phys_set_debug_callback(debug_callback);
}

generic_avl_map_node_t *__cdecl generic_avl_map_add(
    phys_inplace_avl_tree<unsigned int, generic_avl_map_node_t, generic_avl_map_node_t> *gam,
    void *data,
    unsigned int avl_key)
{
    //generic_avl_map_node_t *m_tree_root; // [esp+15Ch] [ebp-8h]
    generic_avl_map_node_t *gamn; // [esp+160h] [ebp-4h]

    //m_tree_root = gam->m_tree_root;
    //while (m_tree_root && avl_key != m_tree_root->m_avl_key)
    //{
    //    if (avl_key >= m_tree_root->m_avl_key)
    //        m_tree_root = m_tree_root->m_avl_tree_node.m_right;
    //    else
    //        m_tree_root = m_tree_root->m_avl_tree_node.m_left;
    //}
    //if (m_tree_root
    //    && !Assert_MyHandler(
    //        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
    //        1243,
    //        0,
    //        "%s",
    //        "gam->find(avl_key) == NULL"))
    //{
    //    __debugbreak();
    //}
    // 

    iassert(gam->find(avl_key) == NULL);

    //gamn = phys_simple_allocator<generic_avl_map_node_t>::allocate(&g_generic_avl_map_node_allocator);
    gamn = g_generic_avl_map_node_allocator.allocate();

    iassert(gamn);

    gamn->m_data = data;
    //phys_inplace_avl_tree<unsigned int, generic_avl_map_node_t, generic_avl_map_node_t>::add(gam, &avl_key, gamn);
    gam->add(avl_key, gamn);

    iassert(gam->find(avl_key) != NULL); // lwss add

    return gamn;
}

void *__cdecl generic_avl_map_destroy(
    phys_inplace_avl_tree<unsigned int, generic_avl_map_node_t, generic_avl_map_node_t> *gam,
    unsigned int avl_key)
{
    generic_avl_map_node_t *m_tree_root; // [esp+144h] [ebp-Ch]
    void *data; // [esp+148h] [ebp-8h]

    m_tree_root = gam->m_tree_root;
    while (m_tree_root && avl_key != m_tree_root->m_avl_key)
    {
        if (avl_key >= m_tree_root->m_avl_key)
            m_tree_root = m_tree_root->m_avl_tree_node.m_right;
        else
            m_tree_root = m_tree_root->m_avl_tree_node.m_left;
    }
    if (!m_tree_root)
        return 0;
    data = m_tree_root->m_data;
    //phys_inplace_avl_tree<unsigned int, generic_avl_map_node_t, generic_avl_map_node_t>::remove(gam, &avl_key);
    gam->remove(avl_key);
    //phys_simple_allocator<generic_avl_map_node_t>::free(&g_generic_avl_map_node_allocator, m_tree_root);
    g_generic_avl_map_node_allocator.free(m_tree_root);
    return data;
}

// LWSS: yes, convert the int directly to a pointer to a struct. 
PhysObjUserData *__cdecl Phys_GetUserData(int id)
{
    iassert(id);

    return (PhysObjUserData *)id;
}

int __cdecl get_physics_contents_mask(char phys_env_collision_flags)
{
    int mask; // [esp+0h] [ebp-4h]

    mask = 0x280EC93;
    if ((phys_env_collision_flags & 0x20) != 0 && (phys_env_collision_flags & 0x10) == 0)
        return 0x281EE93;
    if ((phys_env_collision_flags & 8) != 0 || (phys_env_collision_flags & 0x10) != 0)
        return 0x211;
    return mask;
}

char are_intersecting(
    const broad_phase_environment_query_input *bpeqi,
    const phys_vec3 *aabb_min,
    const phys_vec3 *aabb_max,
    const phys_vec3 *aabb_trans)
{
    float v6; // [esp-Ch] [ebp-30h] BYREF
    phys_vec3 v7; // [esp-8h] [ebp-2Ch] OVERLAPPED BYREF
    float trans_4; // [esp+8h] [ebp-1Ch]
    float trans_8; // [esp+Ch] [ebp-18h]
    float trans_12; // [esp+10h] [ebp-14h]
    const phys_vec3 *p_trace_translation; // [esp+14h] [ebp-10h]
    //int v12; // [esp+18h] [ebp-Ch]
    //void *v13; // [esp+1Ch] [ebp-8h]
    //void *retaddr; // [esp+24h] [ebp+0h]
    //
    //v12 = a1;
    //v13 = retaddr;
    p_trace_translation = &bpeqi->trace_translation;
    trans_12 = bpeqi->trace_translation.x - aabb_trans->x;
    trans_8 = bpeqi->trace_translation.y - aabb_trans->y;
    trans_4 = bpeqi->trace_translation.z - aabb_trans->z;
    v7.x = trans_12;
    v7.y = trans_8;
    v7.z = trans_4;
    return phys_are_potentially_colliding(
        bpeqi->trace_aabb_min_wace,
        bpeqi->trace_aabb_max_wace,
        v7,
        *aabb_min,
        *aabb_max,
        &v6);
}


// aislop supervised
void standard_query::query(
    const broad_phase_environment_query_input &bpeqi_ref,
    broad_phase_environement_query_results *bpeqr)
{
    const broad_phase_environment_query_input *bpeqi = &bpeqi_ref;

    // Compute swept AABB: union of start and end positions
    phys_vec3 aabb_end_min = bpeqi->trace_aabb_min_wace + bpeqi->trace_translation;
    phys_vec3 swept_min;
    swept_min = phys_min(bpeqi->trace_aabb_min_wace, aabb_end_min);

    phys_vec3 aabb_end_max = bpeqi->trace_aabb_max_wace + bpeqi->trace_translation;
    phys_vec3 swept_max;
    swept_max = phys_max(bpeqi->trace_aabb_max_wace, aabb_end_max);

    float mins[3], maxs[3];
    Phys_NitrousVecToVec3(swept_min, mins);
    Phys_NitrousVecToVec3(swept_max, maxs);

    iassert(!isnan(mins[0]) && !isnan(mins[1]) && !isnan(mins[2]));
    iassert(!isnan(maxs[0]) && !isnan(maxs[1]) && !isnan(maxs[2]));

    // Debug visualization for large queries
    if (phys_debugBigQueries->current.enabled &&
        Abs(bpeqi->trace_translation) > 500.0f)
    {
        g_render_mutex.Lock();
        render_box(&bpeqi->trace_aabb_min_wace, &bpeqi->trace_aabb_max_wace, colorBlue, 1000);

        float lineStart[3];
        Phys_NitrousVecToVec3(bpeqi->trace_aabb_min_wace, lineStart);

        phys_vec3 aabb_end = bpeqi->trace_aabb_min_wace + bpeqi->trace_translation;
        float lineEnd[3];
        Phys_NitrousVecToVec3(aabb_end, lineEnd);
        CG_DebugLine(lineStart, lineEnd, colorRed, 1, 1000);

        lineEnd[0] = lineStart[0];
        lineEnd[1] = lineStart[1];
        lineEnd[2] = lineStart[2] + 10000.0f;
        CG_DebugLine(lineStart, lineEnd, colorWhite, 1, 1000);

        g_render_mutex.Unlock();
    }

    // Initialize the collision visitor and query results
    gjk_physics_collision_visitor v83;
    v83.bpeqi = bpeqi;
    v83.bpeqr = bpeqr;

    bpeqr->m_list_bpi_env.m_list_cur = &bpeqr->m_list_bpi_env.m_list;
    bpeqr->m_list_bpi_env_count = 0;
    bpeqr->m_env_collision_flags = 0;

    environment_rigid_body *env_rb = phys_sys::get_environment_rigid_body();
    int physics_contents_mask = get_physics_contents_mask(bpeqi->env_collision_flags);

    // --- Static world geometry (brushes and partitions) ---
    if (bpeqi->env_collision_flags & 1)
    {
        static_colgeom_visitor_t v76;
        v76.intersect_box(mins, maxs, physics_contents_mask);

        // Process brushes
        for (int i = 0; i < v76.nbrushes; ++i)
        {
            const cbrush_t *brush = v76.brushes[i];

            phys_vec3 brush_min, brush_max;
            Phys_Vec3ToNitrousVec(brush->mins, brush_min);
            Phys_Vec3ToNitrousVec(brush->maxs, brush_max);

            phys_vec3 zero_trans = PHYS_ZERO_VEC;
            if (!are_intersecting(bpeqi, &brush_min, &brush_max, &zero_trans))
                continue;

            broad_phase_environment_info *bpei_mt =
                G_BPM->g_bpei_database.get_bpei_mt(bpei_database_id((unsigned int)brush, 0));

            if (!bpei_mt->m_data)
            {
                bpei_mt->m_mutex.Lock();
                if (!bpei_mt->m_data)
                {
                    int stype = (brush->axial_sflags[0][0] & 0x3F00000) >> 20;
                    gjk_base_t *geom = create_brush_gjk_geom(brush, stype, &v83);
                    geom->set_geom_id_new(bpei_mt->m_gjk_geom_id);

                    broad_phase_info *bpi_env = allocate_bpi_env();
                    bpi_env->set(
                        env_rb,
                        &PHYS_IDENTITY_MATRIX,
                        &PHYS_IDENTITY_MATRIX,
                        nullptr,
                        geom,
                        geom->get_geom_id(),
                        false,
                        geom->stype,
                        nullptr,
                        1u);
                    bpi_env->set_bpi_env(nullptr);

                    if (bpi_env)
                    {
                        iassert(bpi_env->is_bpi_env());
                        bpi_env->get_bpi_env()->collision_prolog();
                        bpei_mt->m_data = bpi_env;
                    }
                }
                bpei_mt->m_mutex.Unlock();
            }

            if (bpei_mt->m_data)
                bpeqr->add((broad_phase_base *)bpei_mt->m_data);
        }

        // Process terrain partitions (CollisionAabbTrees)
        for (int j = 0; j < v76.ntrees; ++j)
        {
            const CollisionAabbTree *tree = v76.trees[j];
            const CollisionPartition *partition = &cm.partitions[tree->u.firstChildIndex];

            phys_vec3 tree_half, tree_origin;
            Phys_Vec3ToNitrousVec(tree->halfSize, tree_half);
            Phys_Vec3ToNitrousVec(tree->origin, tree_origin);

            phys_vec3 tree_min = tree_origin - tree_half;
            phys_vec3 tree_max = tree_origin + tree_half;

            // Translation relative to static geometry is zero
            phys_vec3 rel_trans = bpeqi->trace_translation - PHYS_ZERO_VEC;

            float hit_time;
            if (!phys_are_potentially_colliding(
                bpeqi->trace_aabb_min_wace,
                bpeqi->trace_aabb_max_wace,
                rel_trans,
                tree_min,
                tree_max,
                &hit_time))
                continue;

            broad_phase_environment_info *bpei_mt =
                G_BPM->g_bpei_database.get_bpei_mt(bpei_database_id((unsigned int)partition, 0));

            if (!bpei_mt->m_data)
            {
                bpei_mt->m_mutex.Lock();
                if (!bpei_mt->m_data)
                {
                    gjk_partition_t *geom = create_partition_gjk_geom(tree, &v83);
                    geom->set_geom_id_new(bpei_mt->m_gjk_geom_id);

                    broad_phase_info *bpi_env = allocate_bpi_env();
                    bpi_env->set(
                        env_rb,
                        &PHYS_IDENTITY_MATRIX,
                        &PHYS_IDENTITY_MATRIX,
                        nullptr,
                        geom,
                        geom->get_geom_id(),
                        false,
                        geom->stype,
                        nullptr,
                        1u);
                    bpi_env->set_bpi_env(nullptr);

                    if (bpi_env)
                    {
                        iassert(bpi_env->is_bpi_env());
                        bpi_env->get_bpi_env()->collision_prolog();
                        bpei_mt->m_data = bpi_env;
                    }
                }
                bpei_mt->m_mutex.Unlock();
            }

            if (bpei_mt->m_data)
                bpeqr->add((broad_phase_base *)bpei_mt->m_data);
        }
    }

    // --- Dynamic entity collision ---
    if (phys_entityCollision->current.enabled)
    {
        // DynEntities (client-side dynamic entities)
        if (!(bpeqi->env_collision_flags & 0x10) && (bpeqi->env_collision_flags & 4))
        {
            DynEntityAreaParms areaParms;
            areaParms.mins = mins;
            areaParms.maxs = maxs;
            areaParms.contentMask = -1;
            areaParms.list = (uint16_t *)alloca(4096 * sizeof(uint16_t));
            areaParms.maxCount = 4096;

            for (DynEntityDrawType k = DYNENT_DRAW_MODEL; k < DYNENT_DRAW_COUNT; ++k)
            {
                areaParms.count = 0;
                DynEntCl_AreaEntities_r((DynEntityCollType)k, 1u, &areaParms);

                for (int m = 0; m < areaParms.count; ++m)
                {
                    const DynEntityDef *entDef = DynEnt_GetEntityDef(areaParms.list[m], k);

                    if (!(bpeqi->env_collision_flags & 8) && !(entDef->flags & 2))
                        continue;

                    unsigned short id = DynEnt_GetId(entDef, k);
                    DynEntityClient *clientEnt = DynEnt_GetClientEntity(id, k);

                    if (clientEnt->physObjId)
                        continue;

                    v83.cent = nullptr;
                    v83.dynEntDef = entDef;
                    v83.glass = nullptr;
                    v83.rb = env_rb;
                    v83.rb_to_world_xform = &PHYS_IDENTITY_MATRIX;
                    v83.cg_to_world_xform = nullptr;
                    v83.cg_to_rb_xform = nullptr;
                    v83.env_collision_flags = 4;
                    create_entity_bpi(&v83, physics_contents_mask);
                }
            }
        }

        // World entities (players, vehicles, etc.)
        if (bpeqi->env_collision_flags & 2)
        {
            int entityList[16];
            int entCount = CG_AreaEntities(mins, maxs, entityList, 16, physics_contents_mask);

            for (int n = 0; n < entCount; ++n)
            {
                const centity_s *cent = CG_GetEntity(0, entityList[n]);
                const DObj *dobj = Com_GetClientDObj(entityList[n], 0);

                if (dobj)
                {
                    const XModel *model = DObjGetModel(dobj, 0);
                    if (model)
                    {
                        int contents = XModelGetContents(model);
                        if (!(physics_contents_mask & contents) && cent->nextState.eType != 17)
                            continue;
                    }
                }

                if (cent->nextState.eType == 6)
                {
                    auto_rigid_body::add(cent, &v83, physics_contents_mask);
                }
                else if (cent->nextState.eType == 14 || cent->nextState.eType == 16)
                {
                    short vehicleInfoIndex = cent->nextState.vehicleState.vehicleInfoIndex;
                    const vehicle_info_t *vehicleInfo = CG_GetVehicleInfo(vehicleInfoIndex);
                    if (!vehicleInfo->name[0])
                        vehicleInfo = BG_GetVehicleInfo(vehicleInfoIndex);

                    iassert(vehicleInfo->name[0] > 0);

                    if (!vehicleInfo->isNitrous)
                        auto_rigid_body::add(cent, &v83, physics_contents_mask);
                }
                else if (cent->nextState.eType == 17 && dynEnt_sentientAutoActivate->current.enabled)
                {
                    auto_rigid_body::add(cent, &v83, physics_contents_mask);
                }
            }
        }

        // Glass entities
        if (bpeqi->env_collision_flags & 0x40)
        {
            const Glass *glasses[128];
            unsigned int glassCount = GlassCl_AreaGlasses(mins, maxs, glasses, 128u);

            for (unsigned int ii = 0; ii < glassCount; ++ii)
            {
                v83.cent = nullptr;
                v83.dynEntDef = nullptr;
                v83.glass = glasses[ii];
                v83.rb = phys_sys::get_environment_rigid_body();
                v83.rb_to_world_xform = &PHYS_IDENTITY_MATRIX;
                v83.cg_to_world_xform = nullptr;
                v83.cg_to_rb_xform = nullptr;
                v83.env_collision_flags = 64;
                create_entity_bpi(&v83, physics_contents_mask);
            }
        }

        // User rigid bodies (physics objects owned by the game)
        if ((bpeqi->env_collision_flags & 0x10) &&
            !(bpeqi->env_collision_flags & 8) &&
            (bpeqi->env_collision_flags & 0x100))
        {
            for (int jj = 0; jj < 16; ++jj)
            {
                PhysObjUserData *userRb = physGlob.userRigidBodies[jj];
                if (!userRb)
                    continue;

                iassert(userRb->m_gjk_geom_list.m_geom_count >= 0);

                gjk_base_t *first_geom = userRb->m_gjk_geom_list.m_first_geom;

                broad_phase_environment_info *bpei_mt =
                    G_BPM->g_bpei_database.get_bpei_mt(bpei_database_id((unsigned int)userRb, 0));

                if (!bpei_mt->m_data)
                {
                    bpei_mt->m_mutex.Lock();
                    if (!bpei_mt->m_data)
                    {
                        broad_phase_info *bpi_env = allocate_bpi_env();
                        bpi_env->set(
                            userRb->body,
                            &userRb->body->m_mat,
                            &userRb->cg2w,
                            &userRb->cg2rb,
                            first_geom,
                            first_geom->get_geom_id(),
                            false,
                            first_geom->stype,
                            nullptr,
                            2u);
                        bpi_env->set_bpi_env(nullptr);

                        if (bpi_env)
                        {
                            iassert(bpi_env->is_bpi_env());
                            bpi_env->get_bpi_env()->collision_prolog();
                            bpei_mt->m_data = bpi_env;
                        }
                    }
                    bpei_mt->m_mutex.Unlock();
                }

                if (bpei_mt->m_data)
                    bpeqr->add((broad_phase_base *)bpei_mt->m_data);
            }
        }
    }
}

void gjk_physics_collision_visitor::set_local_query_info(
    const void *entity)
{
    const phys_vec3 *v3; // eax
    phys_vec3 *p_m_local_query_trace_translation; // [esp-58h] [ebp-64h]
    phys_vec3 v5; // [esp-50h] [ebp-5Ch] BYREF
    phys_vec3 v6; // [esp-40h] [ebp-4Ch] BYREF
    float v7; // [esp-30h] [ebp-3Ch]
    float v8; // [esp-2Ch] [ebp-38h]
    float v9; // [esp-28h] [ebp-34h]
    const float *p_x; // [esp-24h] [ebp-30h]
    phys_vec3 m_moved_vec; // [esp-20h] [ebp-2Ch]
    rigid_body *rb; // [esp-8h] [ebp-14h]
    gjk_physics_collision_visitor *v13; // [esp-4h] [ebp-10h]
    //int v14; // [esp+0h] [ebp-Ch]
    //void *v15; // [esp+4h] [ebp-8h]
    //void *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v14 = a2;
    //v15 = retaddr;
    v13 = this;
    phys_calc_local_aabb(
        this->bpeqi->trace_aabb_min_wace,
        this->bpeqi->trace_aabb_max_wace,
        *this->cg_to_world_xform,
        &this->m_local_query_trace_aabb_min,
        &this->m_local_query_trace_aabb_max);
    rb = v13->rb;
    m_moved_vec = rb->m_moved_vec;
    p_x = &v13->bpeqi->trace_translation.x;
    v9 = *p_x - m_moved_vec.x;
    v8 = p_x[1] - m_moved_vec.y;
    v7 = p_x[2] - m_moved_vec.z;
    v6.x = v9;
    v6.y = v8;
    v6.z = v7;
    v5 = phys_inv_multiply(*v13->cg_to_world_xform, v6);
    v3 = &v5;
    p_m_local_query_trace_translation = &v13->m_local_query_trace_translation;
    v13->m_local_query_trace_translation.x = v3->x;
    p_m_local_query_trace_translation->y = v3->y;
    p_m_local_query_trace_translation->z = v3->z;
    v13->m_local_entity = entity;
}

void gjk_physics_collision_visitor::get_local_query_aabb(
    float *local_query_aabb_min,
    float *local_query_aabb_max)
{
    phys_vec3 v4; // [esp-Ch] [ebp-8Ch] BYREF
    phys_vec3 m_local_query_aabb_max_4; // [esp+4h] [ebp-7Ch] BYREF
    float v6; // [esp+20h] [ebp-60h]
    float v7; // [esp+24h] [ebp-5Ch]
    float v8; // [esp+28h] [ebp-58h]
    phys_vec3 *p_m_local_query_trace_aabb_max; // [esp+2Ch] [ebp-54h]
    phys_vec3 *v10; // [esp+30h] [ebp-50h]
    phys_vec3 m_local_query_aabb_min; // [esp+34h] [ebp-4Ch] BYREF
    phys_vec3 v12; // [esp+44h] [ebp-3Ch] BYREF
    float v13; // [esp+5Ch] [ebp-24h]
    float v14; // [esp+60h] [ebp-20h]
    float v15; // [esp+64h] [ebp-1Ch]
    phys_vec3 *p_m_local_query_trace_aabb_min; // [esp+68h] [ebp-18h]
    phys_vec3 *p_m_local_query_trace_translation; // [esp+6Ch] [ebp-14h]
    gjk_physics_collision_visitor *thisa; // [esp+70h] [ebp-10h]
    //_UNKNOWN *v19; // [esp+74h] [ebp-Ch]
    //float *local_query_aabb_mina; // [esp+78h] [ebp-8h]
    //int vars0; // [esp+80h] [ebp+0h]
    //
    //v19 = a2;
    //local_query_aabb_mina = (float *)vars0;
    thisa = this;
    p_m_local_query_trace_translation = &this->m_local_query_trace_translation;
    p_m_local_query_trace_aabb_min = &this->m_local_query_trace_aabb_min;
    v15 = this->m_local_query_trace_aabb_min.x + this->m_local_query_trace_translation.x;
    v14 = this->m_local_query_trace_aabb_min.y + this->m_local_query_trace_translation.y;
    v13 = this->m_local_query_trace_aabb_min.z + this->m_local_query_trace_translation.z;
    v12.x = v15;
    v12.y = v14;
    v12.z = v13;
    m_local_query_aabb_min = phys_min(this->m_local_query_trace_aabb_min, v12);
    v10 = &thisa->m_local_query_trace_translation;
    p_m_local_query_trace_aabb_max = &thisa->m_local_query_trace_aabb_max;
    v8 = thisa->m_local_query_trace_aabb_max.x + thisa->m_local_query_trace_translation.x;
    v7 = thisa->m_local_query_trace_aabb_max.y + thisa->m_local_query_trace_translation.y;
    v6 = thisa->m_local_query_trace_aabb_max.z + thisa->m_local_query_trace_translation.z;
    m_local_query_aabb_max_4.x = v8;
    m_local_query_aabb_max_4.y = v7;
    m_local_query_aabb_max_4.z = v6;
    v4 = phys_max(thisa->m_local_query_trace_aabb_max, m_local_query_aabb_max_4);
    Phys_NitrousVecToVec3(m_local_query_aabb_min, local_query_aabb_min);
    Phys_NitrousVecToVec3(v4, local_query_aabb_max);
}

bool gjk_physics_collision_visitor::query_create_prolog(const void *geom)
{
    if (!geom
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp", 290, 0, "%s", "geom"))
    {
        __debugbreak();
    }
    this->m_local_database_id.m_id1 = (unsigned int)this->m_local_entity;
    this->m_local_database_id.m_id2 = (unsigned int)geom;
    //this->m_local_bpei = bpei_database_t::get_bpei_mt(&G_BPM->g_bpei_database, this->m_local_database_id);
    this->m_local_bpei = G_BPM->g_bpei_database.get_bpei_mt(this->m_local_database_id);

    if (!this->m_local_bpei->m_data)
    {
        //minspec_mutex::Lock(&this->m_local_bpei->m_mutex);
        this->m_local_bpei->m_mutex.Lock();
        if (!this->m_local_bpei->m_data)
            return 1;
        //minspec_mutex::Unlock(&this->m_local_bpei->m_mutex);
        this->m_local_bpei->m_mutex.Unlock();
    }

    if (!this->m_local_bpei->m_data
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
            300,
            0,
            "%s",
            "m_local_bpei->m_data"))
    {
        __debugbreak();
    }

    //broad_phase_environement_query_results::add(this->bpeqr, (broad_phase_base *)this->m_local_bpei->m_data);
    this->bpeqr->add((broad_phase_base *)this->m_local_bpei->m_data);
    return 0;
}

bool gjk_physics_collision_visitor::query_create_prolog_1(
    const float *local_aabb_min,
    const float *local_aabb_max,
    const void *geom)
{
    float hit_time; // [esp-Ch] [ebp-40h] BYREF
    phys_vec3 pv_local_aabb_max; // [esp-8h] [ebp-3Ch] BYREF
    phys_vec3 pv_local_aabb_min; // [esp+8h] [ebp-2Ch] BYREF

    Phys_Vec3ToNitrousVec(local_aabb_min, pv_local_aabb_min);
    Phys_Vec3ToNitrousVec(local_aabb_max, pv_local_aabb_max);

    return phys_are_potentially_colliding(
        this->m_local_query_trace_aabb_min,
        this->m_local_query_trace_aabb_max,
        this->m_local_query_trace_translation,
        pv_local_aabb_min,
        pv_local_aabb_max,
        &hit_time)
        && this->query_create_prolog(geom);
}

void gjk_physics_collision_visitor::query_create_epilog(gjk_base_t *gjk_geom)
{
    unsigned int geom_id; // eax
    int stype; // [esp-Ch] [ebp-90h]
    unsigned int env_collision_flags; // [esp-4h] [ebp-88h]
    BOOL v6; // [esp+0h] [ebp-84h]
    broad_phase_info *bpi_env; // [esp+80h] [ebp-4h]

    if (!gjk_geom
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp", 307, 0, "%s", "gjk_geom"))
    {
        __debugbreak();
    }
    if (gjk_geom->stype > 0x22u
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
            308,
            0,
            "%s",
            "gjk_geom->stype >= 0 && gjk_geom->stype < MAX_NUM_SURFACE_TYPES"))
    {
        __debugbreak();
    }
    if (!this->m_local_bpei
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp", 309, 0, "%s", "m_local_bpei"))
    {
        __debugbreak();
    }
    if (this->m_local_bpei->m_data
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
            310,
            0,
            "%s",
            "m_local_bpei->m_data == NULL"))
    {
        __debugbreak();
    }
    v6 = this->m_local_bpei->m_database_id.m_id1 == this->m_local_database_id.m_id1
        && this->m_local_bpei->m_database_id.m_id2 == this->m_local_database_id.m_id2;
    if (!v6
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
            311,
            0,
            "%s",
            "m_local_bpei->m_database_id == m_local_database_id"))
    {
        __debugbreak();
    }
    //gjk_base_t::set_geom_id_new(gjk_geom, this->m_local_bpei->m_gjk_geom_id);
    gjk_geom->set_geom_id_new(this->m_local_bpei->m_gjk_geom_id);
    bpi_env = allocate_bpi_env();
    env_collision_flags = this->env_collision_flags;
    stype = gjk_geom->stype;
    //geom_id = gjk_base_t::get_geom_id(gjk_geom);
    geom_id = gjk_geom->get_geom_id();
    //broad_phase_info::set(
        bpi_env->set(
        this->rb,
        this->rb_to_world_xform,
        this->cg_to_world_xform,
        this->cg_to_rb_xform,
        gjk_geom,
        geom_id,
        0,
        stype,
        0,
        env_collision_flags);
    //broad_phase_info::set_bpi_env(bpi_env, this->auto_activate_callback);
    bpi_env->set_bpi_env(this->auto_activate_callback);
    //broad_phase_info::collision_prolog(bpi_env);
    bpi_env->collision_prolog();
    this->m_local_bpei->m_data = bpi_env;
    //minspec_mutex::Unlock(&this->m_local_bpei->m_mutex);
    this->m_local_bpei->m_mutex.Unlock();
    if (!this->m_local_bpei->m_data
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
            321,
            0,
            "%s",
            "m_local_bpei->m_data"))
    {
        __debugbreak();
    }
    //broad_phase_environement_query_results::add(this->bpeqr, (broad_phase_base *)this->m_local_bpei->m_data);
    this->bpeqr->add((broad_phase_base *)this->m_local_bpei->m_data);
}

void __thiscall gjk_physics_collision_visitor::query_create_epilog_1(
    gjk_base_t *gjk_geom)
{
    this->query_create_epilog(gjk_geom);
}