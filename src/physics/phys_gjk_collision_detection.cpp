#include "phys_gjk_collision_detection.h"
#include <universal/com_math.h>
#include "phys_colgeom.h"
#include <universal/com_math_anglevectors.h>
#include <cgame/cg_world.h>
#include <DynEntity/DynEntity_load_obj.h>
#include <qcommon/cm_load.h>
#include <bgame/bg_slidemove.h>
#include <qcommon/cm_world.h>
#include <game_mp/g_main_mp.h>
#include <bgame/bg_misc.h>
#include <qcommon/dobj_management.h>
#include <glass/glass_server.h>
#include <glass/glass_client.h>
#include <new>
#include <cgame_mp/cg_local_mp.h>

int s_caulk_sflags = 0x7FFFFFFF;


void __cdecl gjk_collision_epilog(bool is_server_thread)
{
    //if ( is_server_thread )
        //BLOPS_NULLSUB();
}

static int g_use_gjk_cache = 1;
phys_gjk_cache_info *__cdecl get_gjk_cache_info(
                phys_heap_gjk_cache_system_avl_tree *gjk_cache,
                const gjk_base_t *cg1,
                const gjk_base_t *cg2)
{
    if ( !gjk_cache || !g_use_gjk_cache )
        return 0;

    return gjk_cache->get_gjk_cache_info(cg1->get_geom_id(), cg2->get_geom_id(), 1);
}

void gjk_query_output::reset_cache()
{
    //bpei_database_t::update_database(&this->m_bpei_database);
    this->m_bpei_database.update_database();
    this->m_ent_count = 0;
    this->m_geom_count = 0;
    //this->m_list_geom_info.m_first = 0;
    //this->m_list_geom_info.m_last_next_ptr = &this->m_list_geom_info.m_first;
    //this->m_list_geom_info.m_alloc_count = 0;
    this->m_list_geom_info.remove_all();
    this->m_query_visitor_count = 0;
    this->m_gent_query_visitor_count = 0;
    this->m_cent_query_visitor_count = 0;
    this->m_dent_query_visitor_count = 0;
    //phys_transient_allocator::reset(&this->m_allocator);
    this->m_allocator.reset();
    this->m_total_list_geom_info = 0;
    this->m_cached_query_info.m_query_contents = 0;
    this->m_cached_query_info.m_query_flags = 0;
}

void gjk_query_output::query_prolog(const gjk_query_input *input)
{
    //this->m_list_geom_info.m_first = 0;
    //this->m_list_geom_info.m_last_next_ptr = &this->m_list_geom_info.m_first;
    //this->m_list_geom_info.m_alloc_count = 0;
    this->m_list_geom_info.remove_all();

    //gjk_query_input::visit_skip_list(input, ++this->m_query_visitor_count);
    input->visit_skip_list(++this->m_query_visitor_count);
    gjk_query_output::calc_query_aabb(input);
    //phys_transient_allocator::reset_to_state(&this->m_allocator, &this->m_allocator_state);
    this->m_allocator.reset_to_state(this->m_allocator_state);
}

void gjk_query_input::visit_skip_list(int query_visitor_count) const 
{
    gjk_geom_info_t *gi_i; // [esp+14h] [ebp-4h]

    for ( gi_i = this->m_geom_skip_list.m_first; gi_i; gi_i = gi_i->m_next_link )
    {
        if ( gi_i->m_query_visitor_count == query_visitor_count
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.h",
                        182,
                        0,
                        "%s",
                        "gi->m_query_visitor_count != query_visitor_count") )
        {
            __debugbreak();
        }
        gi_i->m_query_visitor_count = query_visitor_count;
    }
}

void gjk_query_output::calc_query_aabb(const gjk_query_input *input)
{
    const phys_vec3 *v3; // eax
    phys_vec3 *v4; // ecx
    float v5; // [esp-13Ch] [ebp-148h]
    float v6; // [esp-138h] [ebp-144h]
    phys_vec3 v7; // [esp-120h] [ebp-12Ch] BYREF
    const phys_vec3 *v8; // [esp-104h] [ebp-110h]
    phys_vec3 v9; // [esp-100h] [ebp-10Ch] BYREF
    float v10; // [esp-E8h] [ebp-F4h]
    float v11; // [esp-E4h] [ebp-F0h]
    float v12; // [esp-E0h] [ebp-ECh]
    phys_vec3 *v13; // [esp-DCh] [ebp-E8h]
    const phys_vec3 *v14; // [esp-D8h] [ebp-E4h]
    phys_vec3 *v15; // [esp-D4h] [ebp-E0h]
    float v16; // [esp-D0h] [ebp-DCh]
    float v17; // [esp-CCh] [ebp-D8h]
    float v18; // [esp-C8h] [ebp-D4h]
    float v19; // [esp-C0h] [ebp-CCh]
    float v20; // [esp-BCh] [ebp-C8h]
    float v21; // [esp-B8h] [ebp-C4h]
    const phys_vec3 *v22; // [esp-B4h] [ebp-C0h]
    phys_vec3 v23; // [esp-B0h] [ebp-BCh] BYREF
    const phys_vec3 *p_m_ac_eps_vec; // [esp-94h] [ebp-A0h]
    phys_vec3 v25; // [esp-90h] [ebp-9Ch] BYREF
    float v26; // [esp-78h] [ebp-84h]
    float v27; // [esp-74h] [ebp-80h]
    float v28; // [esp-70h] [ebp-7Ch]
    phys_vec3 *v29; // [esp-6Ch] [ebp-78h]
    const phys_vec3 *p_m_cg_translation; // [esp-68h] [ebp-74h]
    phys_vec3 *p_m_query_aabb_max; // [esp-64h] [ebp-70h]
    float v32; // [esp-60h] [ebp-6Ch]
    float v33; // [esp-5Ch] [ebp-68h]
    float v34; // [esp-58h] [ebp-64h]
    float v35; // [esp-48h] [ebp-54h]
    float v36; // [esp-44h] [ebp-50h]
    float v37; // [esp-40h] [ebp-4Ch]
    const phys_vec3 *v38; // [esp-3Ch] [ebp-48h]
    const phys_vec3 *p_m_cg_aabb_max; // [esp-38h] [ebp-44h]
    phys_vec3 *p_m_query_aabb_min; // [esp-34h] [ebp-40h]
    float v41; // [esp-30h] [ebp-3Ch]
    float v42; // [esp-2Ch] [ebp-38h]
    float v43; // [esp-28h] [ebp-34h]
    float v44; // [esp-14h] [ebp-20h]
    float v45; // [esp-10h] [ebp-1Ch]
    float v46; // [esp-Ch] [ebp-18h]
    const phys_vec3 *p_m_cg_position; // [esp-8h] [ebp-14h]
    gjk_query_output *v48; // [esp-4h] [ebp-10h]
    //int v49; // [esp+0h] [ebp-Ch]
    //void *v50; // [esp+4h] [ebp-8h]
    //void *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v49 = a2;
    //v50 = retaddr;
    v48 = this;
    p_m_cg_position = &input->m_cg_position;
    v46 = input->m_cg_position.x + input->m_cg_aabb_min.x;
    v45 = input->m_cg_position.y + input->m_cg_aabb_min.y;
    v44 = input->m_cg_position.z + input->m_cg_aabb_min.z;
    v41 = v46;
    v42 = v45;
    v43 = v44;
    p_m_query_aabb_min = &this->m_query_aabb_min;
    this->m_query_aabb_min.x = v46;
    p_m_query_aabb_min->y = v42;
    p_m_query_aabb_min->z = v43;
    p_m_cg_aabb_max = &input->m_cg_aabb_max;
    v38 = &input->m_cg_position;
    v37 = input->m_cg_position.x + input->m_cg_aabb_max.x;
    v36 = input->m_cg_position.y + input->m_cg_aabb_max.y;
    v35 = input->m_cg_position.z + input->m_cg_aabb_max.z;
    v32 = v37;
    v33 = v36;
    v34 = v35;
    p_m_query_aabb_max = &v48->m_query_aabb_max;
    v48->m_query_aabb_max.x = v37;
    p_m_query_aabb_max->y = v33;
    p_m_query_aabb_max->z = v34;
    p_m_cg_translation = &input->m_cg_translation;
    v29 = &v48->m_query_aabb_min;
    v28 = v48->m_query_aabb_min.x + input->m_cg_translation.x;
    v27 = v48->m_query_aabb_min.y + input->m_cg_translation.y;
    v26 = v48->m_query_aabb_min.z + input->m_cg_translation.z;
    v25.x = v28;
    v25.y = v27;
    v25.z = v26;
    p_m_ac_eps_vec = &input->m_ac_eps_vec;
    v23 = phys_min(v48->m_query_aabb_min, v25);
    v22 = &v23;
    v21 = v22->x - p_m_ac_eps_vec->x;
    v20 = v22->y - p_m_ac_eps_vec->y;
    v19 = v22->z - p_m_ac_eps_vec->z;
    v16 = v21;
    v17 = v20;
    v18 = v19;
    v15 = &v48->m_query_aabb_min;
    v48->m_query_aabb_min.x = v21;
    v15->y = v17;
    v15->z = v18;
    v14 = &input->m_cg_translation;
    v13 = &v48->m_query_aabb_max;
    v12 = v48->m_query_aabb_max.x + input->m_cg_translation.x;
    v11 = v48->m_query_aabb_max.y + input->m_cg_translation.y;
    v10 = v48->m_query_aabb_max.z + input->m_cg_translation.z;
    v9.x = v12;
    v9.y = v11;
    v9.z = v10;
    v8 = &input->m_ac_eps_vec;
    v7 = phys_max(v48->m_query_aabb_max, v9);
    v3 = &v7;
    v5 = v3->y + v8->y;
    v6 = v3->z + v8->z;
    v4 = &v48->m_query_aabb_max;
    v48->m_query_aabb_max.x = v3->x + v8->x;
    v4->y = v5;
    v4->z = v6;
}

void gjk_query_output::query_epilog()
{
    m_allocator_state.m_first_block = m_allocator.m_first_block;
    m_allocator_state.m_cur = m_allocator.m_cur;
    m_allocator_state.m_end = m_allocator.m_end;
    m_allocator_state.m_total_memory_allocated = m_allocator.m_total_memory_allocated;
}

void *gjk_query_output::allocate(int size, int alignment, bool no_error)
{
    return this->m_allocator.allocate(size, alignment, no_error, "gjk player trace out of memory");
}

void gjk_query_output::get_local_query_aabb(
                float *local_query_aabb_min,
                float *local_query_aabb_max)
{
    Phys_NitrousVecToVec3(this->m_local_query_aabb_min, local_query_aabb_min);
    Phys_NitrousVecToVec3(this->m_local_query_aabb_max, local_query_aabb_max);
}

bool gjk_query_output::query_create_prolog(const void *geom)
{
    iassert(geom);
    iassert(m_local_ent_info);
    iassert(m_local_ent_info->get_ent());

    //this->m_local_database_id.m_id1 = (unsigned int)gjk_entity_info_t::get_ent(this->m_local_ent_info);
    this->m_local_database_id.m_id1 = (unsigned int)this->m_local_ent_info->get_ent();
    this->m_local_database_id.m_id2 = (unsigned int)geom;
    //this->m_local_bpei = bpei_database_t::get_bpei(&this->m_bpei_database, this->m_local_database_id);
    this->m_local_bpei = this->m_bpei_database.get_bpei(this->m_local_database_id);
    return this->m_local_bpei->m_data == 0;
}

void gjk_query_output::query_create_epilog(gjk_base_t *gjk_geom)
{
    bool v2; // [esp+0h] [ebp-10h]

    if ( !gjk_geom
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.cpp",
                    472,
                    0,
                    "%s",
                    "gjk_geom") )
    {
        __debugbreak();
    }
    if ( !this->m_local_bpei
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.cpp",
                    473,
                    0,
                    "%s",
                    "m_local_bpei") )
    {
        __debugbreak();
    }
    if ( this->m_local_bpei->m_data
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.cpp",
                    474,
                    0,
                    "%s",
                    "m_local_bpei->m_data == NULL") )
    {
        __debugbreak();
    }
    v2 = this->m_local_bpei->m_database_id.m_id1 == this->m_local_database_id.m_id1
        && this->m_local_bpei->m_database_id.m_id2 == this->m_local_database_id.m_id2;
    if ( !v2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.cpp",
                    475,
                    0,
                    "%s",
                    "m_local_bpei->m_database_id == m_local_database_id") )
    {
        __debugbreak();
    }
    //gjk_base_t::set_geom_id_new(gjk_geom, this->m_local_bpei->m_gjk_geom_id);
    gjk_geom->set_geom_id_new(this->m_local_bpei->m_gjk_geom_id);
    this->m_local_bpei->m_data = gjk_query_output::create_geom_info(gjk_geom, this->m_local_ent_info, 0, 0);
}

void gjk_query_output::query_create_epilog_1(gjk_base_t *gjk_geom)
{
    query_create_epilog(gjk_geom);
}

bool gjk_query_output::query_create_prolog_1(
                const float *local_aabb_min,
                const float *local_aabb_max,
                const void *geom)
{
    phys_vec3 v6; // [esp-Ch] [ebp-3Ch] BYREF
    phys_vec3 pv_local_aabb_max_4; // [esp+4h] [ebp-2Ch] BYREF
    //gjk_query_output *v8; // [esp+20h] [ebp-10h]
    //int v9; // [esp+24h] [ebp-Ch]
    //void *v10; // [esp+28h] [ebp-8h]
    //void *retaddr; // [esp+30h] [ebp+0h]
    //
    //v9 = a2;
    //v10 = retaddr;
    //v8 = this;
    //if (!this->m_local_ent_info
    //    && !Assert_MyHandler(
    //        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.cpp",
    //        484,
    //        0,
    //        "%s",
    //        "m_local_ent_info"))
    //{
    //    __debugbreak();
    //}
    iassert(m_local_ent_info);
    iassert(m_local_ent_info->get_ent());
    //if (!gjk_entity_info_t::get_ent(this->m_local_ent_info)
    //    && !Assert_MyHandler(
    //        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.cpp",
    //        485,
    //        0,
    //        "%s",
    //        "m_local_ent_info->get_ent()"))
    //{
    //    __debugbreak();
    //}
    Phys_Vec3ToNitrousVec(local_aabb_min, pv_local_aabb_max_4);
    Phys_Vec3ToNitrousVec(local_aabb_max, v6);
    return phys_are_aabb_overlapping(this->m_local_query_aabb_min, this->m_local_query_aabb_max, pv_local_aabb_max_4, v6) && this->query_create_prolog(geom);
}

broad_phase_environment_info *gjk_query_output::get_ent_info(unsigned int ent_id)
{
    //return bpei_database_t::get_bpei(&this->m_bpei_database, (bpei_database_id)ent_id);
    return this->m_bpei_database.get_bpei(bpei_database_id(ent_id, 0));
}

void gjk_query_output::set_local_query_info(const gjk_query_input &input, gjk_entity_info_t *ent_info)
{
    phys_calc_local_aabb(
        this->m_query_aabb_min,
        this->m_query_aabb_max,
        ent_info->m_mat,
        &this->m_local_query_aabb_min,
        &this->m_local_query_aabb_max);

    this->m_local_query_input = &input;
    this->m_local_ent_info = ent_info;

    iassert(ent_info->get_ent());

}

gjk_geom_info_t *gjk_query_output::create_geom_info(
                gjk_base_t *cg,
                gjk_entity_info_t *ent_info,
                float *aabb_min,
                float *aabb_max)
{
    gjk_geom_info_t *geomInfo; // [esp+0h] [ebp-3Ch]

    ++this->m_geom_count;
    //v8 = (gjk_geom_info_t *)phys_transient_allocator::allocate(
    //                                                    &this->m_allocator,
    //                                                    64,
    //                                                    16,
    //                                                    0,
    //                                                    "phys_transient_allocator out of memory.");

    geomInfo = (gjk_geom_info_t *)this->m_allocator.allocate(sizeof(gjk_geom_info_t), 16, 0, "phys_transient_allocator out of memory.");
    geomInfo->m_cg = cg;
    geomInfo->m_ent_info = ent_info;
    geomInfo->m_query_visitor_count = -1;

    if ( aabb_min )
    {
        iassert(aabb_max);

        Phys_Vec3ToNitrousVec(aabb_min, geomInfo->m_aabb_min);
        Phys_Vec3ToNitrousVec(aabb_max, geomInfo->m_aabb_max);
    }
    else
    {
        //gjk_geom_info_t::calc_aabb(v6);
        geomInfo->calc_aabb();
    }

    geomInfo->m_total_next_link = this->m_total_list_geom_info;
    this->m_total_list_geom_info = geomInfo;
    return geomInfo;
}

void gjk_geom_info_t::calc_aabb()
{
    gjk_entity_info_t *xform; // eax

    xform = gjk_geom_info_t::get_xform();
    this->m_cg->calc_aabb(xform->m_mat, &this->m_aabb_min, &this->m_aabb_max);
}

gjk_entity_info_t *gjk_query_output::create_entity_info()
{
    gjk_entity_info_t *info; // [esp+2Ch] [ebp-8h]

    ++this->m_ent_count;
    info = (gjk_entity_info_t *)this->m_allocator.allocate(sizeof(gjk_entity_info_t), 16, 0, "phys_transient_allocator out of memory.");

    info->m_ent_type = gjk_entity_info_t::ENTITY_TYPE::ET_NONE;
    info->m_ent = 0;
    info->m_query_visitor_count = -1;

    return info;
}

void gjk_query_output::add(const gjk_query_input &input, const CollisionPartition *partition, const CollisionAabbTree *tree)
{
    gjk_geom_info_t *geom_info; // eax
    float aabb_max[3]; // [esp+20h] [ebp-28h] BYREF
    gjk_base_t *cg; // [esp+2Ch] [ebp-1Ch]
    float aabb_min[3]; // [esp+30h] [ebp-18h] BYREF
    bpei_database_id database_id; // [esp+3Ch] [ebp-Ch]
    broad_phase_environment_info *bpei; // [esp+44h] [ebp-4h]

    database_id.m_id1 = (unsigned int)partition;
    database_id.m_id2 = 0;
    //bpei = bpei_database_t::get_bpei(&this->m_bpei_database, (bpei_database_id)(unsigned int)partition);
    bpei = this->m_bpei_database.get_bpei(database_id);
    if ( !bpei->m_data ) // lwss: I have confirmed that this cache works if the same partition is passed in
    {
        cg = create_partition_gjk_geom(tree, this);
        //gjk_base_t::set_geom_id_new(cg, bpei->m_gjk_geom_id);
        cg->set_geom_id_new(bpei->m_gjk_geom_id);

        aabb_min[0] = tree->origin[0] - tree->halfSize[0];
        aabb_min[1] = tree->origin[1] - tree->halfSize[1];
        aabb_min[2] = tree->origin[2] - tree->halfSize[2];

        aabb_max[0] = tree->origin[0] + tree->halfSize[0];
        aabb_max[1] = tree->origin[1] + tree->halfSize[1];
        aabb_max[2] = tree->origin[2] + tree->halfSize[2];

        geom_info = gjk_query_output::create_geom_info(cg, 0, aabb_min, aabb_max);
        bpei->m_data = geom_info;
    }
}

void gjk_query_output::add(const gjk_query_input &input, const cbrush_t *brush, const float *query_mins, const float *query_maxs)
{
    gjk_base_t *cg; // [esp+30h] [ebp-10h]
    broad_phase_environment_info *bpei; // [esp+3Ch] [ebp-4h]
    int savedregs; // [esp+40h] [ebp+0h] BYREF

    //bpei = bpei_database_t::get_bpei(&this->m_bpei_database, (bpei_database_id)(unsigned int)brush);
    bpei = this->m_bpei_database.get_bpei(bpei_database_id((unsigned int)brush, 0));
    if ( !bpei->m_data )
    {
        cg = create_brush_gjk_geom(
                     brush,
                    (brush->axial_sflags[0][0] & 0x3F00000) >> 20,
                     this);
        //gjk_base_t::set_geom_id_new(cg, bpei->m_gjk_geom_id);
        cg->set_geom_id_new(bpei->m_gjk_geom_id);
        //bpei->m_data = gjk_query_output::create_geom_info(this, cg, 0, brush->mins, brush->maxs);
        bpei->m_data = gjk_query_output::create_geom_info(cg, 0, (float*)brush->mins, (float *)brush->maxs);
    }
}

void gjk_query_output::add(const gjk_query_input &input, gentity_s *gent)
{
    gjk_entity_info_t *ent_info; // [esp-Ch] [ebp-5Ch]
    float axis[3][3]; // [esp-8h] [ebp-58h] OVERLAPPED BYREF
    broad_phase_environment_info *bpei; // [esp+38h] [ebp-18h]
    unsigned int gent_id; // [esp+3Ch] [ebp-14h]

    gent_id = (unsigned int)gent;
    bpei = gjk_query_output::get_ent_info((unsigned int)gent);
    if (!bpei->m_data)
    {
        gjk_entity_info_t *entity_info;
        entity_info = gjk_query_output::create_entity_info();
        entity_info->m_ent_type = gjk_entity_info_t::ENTITY_TYPE::ET_GENT;
        entity_info->m_ent = gent;
        if (gent->s.eType == ET_ACTOR || gent->s.eType == ET_PLAYER)
        {
            //phys_mat44::operator=(&entity_info->m_mat, &PHYS_IDENTITY_MATRIX_42);
            entity_info->m_mat = PHYS_IDENTITY_MATRIX;
            Phys_Vec3ToNitrousVec(gent->r.currentOrigin, entity_info->m_mat.w);
        }
        else
        {
            AnglesToAxis(gent->r.currentAngles, axis);
            Phys_AxisToNitrousMat(axis, entity_info->m_mat);
            Phys_Vec3ToNitrousVec(gent->r.currentOrigin, entity_info->m_mat.w);
        }
        bpei->m_data = entity_info;
    }

    iassert(bpei->m_data);
    ent_info = (gjk_entity_info_t *)bpei->m_data;

    iassert(ent_info->is_gent());

    if (ent_info->m_query_visitor_count != this->m_gent_query_visitor_count)
    {
        ent_info->m_query_visitor_count = this->m_gent_query_visitor_count;
        gjk_query_output::set_local_query_info(input, ent_info);
        create_gjk_geom(gent, this, 1, input.m_contents, 0);
    }
}

// local variable allocation has failed, the output may be wrong!
void gjk_query_output::add(const gjk_query_input &input, centity_s *cent)
{
    gjk_entity_info_t *m_data; // [esp-8h] [ebp-5Ch]
    float axis[3][3]; // [esp-4h] [ebp-58h] OVERLAPPED BYREF
    phys_vec3 *p_w; // [esp+20h] [ebp-34h]
    const phys_vec3 *v7; // [esp+24h] [ebp-30h]
    phys_vec3 v8; // [esp+28h] [ebp-2Ch] BYREF
    gjk_entity_info_t *entity_info; // [esp+38h] [ebp-1Ch]
    broad_phase_environment_info *ent_info; // [esp+3Ch] [ebp-18h]
    centity_s *v11; // [esp+40h] [ebp-14h]
    gjk_query_output *v12; // [esp+44h] [ebp-10h]
    //broad_phase_environment_info *bpei; // [esp+48h] [ebp-Ch]
    //unsigned int cent_id; // [esp+4Ch] [ebp-8h]
    //unsigned int retaddr; // [esp+54h] [ebp+0h]
    //
    //bpei = a2;
    //cent_id = retaddr;
    v12 = this;
    v11 = cent;
    ent_info = gjk_query_output::get_ent_info((unsigned int)cent);
    if ( !ent_info->m_data )
    {
        entity_info = gjk_query_output::create_entity_info();
        entity_info->m_ent_type = gjk_entity_info_t::ENTITY_TYPE::ET_CENT;
        entity_info->m_ent = cent;
        if ( cent->nextState.eType == 17 || cent->nextState.eType == 1 )
        {
            //phys_mat44::operator=(&entity_info->m_mat, &PHYS_IDENTITY_MATRIX_42);
            entity_info->m_mat = PHYS_IDENTITY_MATRIX;
            //v7 = Phys_Vec3ToNitrousVec(&v8, cent->pose.origin);
            Phys_Vec3ToNitrousVec(cent->pose.origin, v8);
            v7 = &v8;
            p_w = &entity_info->m_mat.w;
            entity_info->m_mat.w.x = v7->x;
            p_w->y = v7->y;
            p_w->z = v7->z;
        }
        else
        {
            AnglesToAxis(cent->pose.angles, axis);
            Phys_AxisToNitrousMat(axis, entity_info->m_mat);
            Phys_Vec3ToNitrousVec(cent->pose.origin, entity_info->m_mat.w);
        }
        ent_info->m_data = entity_info;
    }
    if ( !ent_info->m_data
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.cpp",
                    619,
                    0,
                    "%s",
                    "bpei->m_data") )
    {
        __debugbreak();
    }
    m_data = (gjk_entity_info_t *)ent_info->m_data;
    if ( *((unsigned int *)ent_info->m_data + 16) != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.cpp",
                    621,
                    0,
                    "%s",
                    "ent_info->is_cent()") )
    {
        __debugbreak();
    }
    if ( m_data->m_query_visitor_count != v12->m_cent_query_visitor_count )
    {
        m_data->m_query_visitor_count = v12->m_cent_query_visitor_count;
        gjk_query_output::set_local_query_info(input, m_data);
        create_gjk_geom(cgCollWorldLocalClientNum, cent, v12, 1, input.m_contents, 0, 0);
    }
}

void gjk_query_output::add(const gjk_query_input &input, const Glass *glass)
{
    float axis[3][3]; // [esp+4h] [ebp-34h] BYREF
    gjk_entity_info_t *glass_id; // [esp+28h] [ebp-10h]
    broad_phase_environment_info *ent_info; // [esp+2Ch] [ebp-Ch]
    gjk_entity_info_t *m_data; // [esp+30h] [ebp-8h]
    const gjk_query_input *inputa; // [esp+34h] [ebp-4h]

    inputa = (const gjk_query_input *)glass;
    ent_info = gjk_query_output::get_ent_info((unsigned int)glass);
    if (!ent_info->m_data)
    {
        glass_id = gjk_query_output::create_entity_info();
        glass_id->m_ent_type = gjk_entity_info_t::ENTITY_TYPE::ET_GLASS;
        glass_id->m_ent = glass;
        AnglesToAxis(glass->angles, axis);
        Phys_AxisToNitrousMat(axis, glass_id->m_mat);
        Phys_Vec3ToNitrousVec(glass->origin, glass_id->m_mat.w);
        ent_info->m_data = glass_id;
    }
    if (!ent_info->m_data
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.cpp",
            642,
            0,
            "%s",
            "bpei->m_data"))
    {
        __debugbreak();
    }
    m_data = (gjk_entity_info_t *)ent_info->m_data;
    if (m_data->m_ent_type != gjk_entity_info_t::ENTITY_TYPE::ET_GLASS
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.cpp",
            644,
            0,
            "%s",
            "ent_info->is_glass()"))
    {
        __debugbreak();
    }
    if (m_data->m_query_visitor_count != this->m_gent_query_visitor_count)
    {
        m_data->m_query_visitor_count = this->m_gent_query_visitor_count;
        gjk_query_output::set_local_query_info(input, m_data);
        create_gjk_geom(glass, this, input.m_contents);
    }
}

void gjk_query_output::add(const gjk_query_input &input, const DynEntityDef *dent)
{
    float axis[3][3]; // [esp+4h] [ebp-44h] BYREF
    DynEntityDrawType v5; // [esp+28h] [ebp-20h]
    DynEntityPose *dynEntPose; // [esp+2Ch] [ebp-1Ch]
    DynEntityDrawType drawType; // [esp+30h] [ebp-18h]
    gjk_entity_info_t *ent_info_; // [esp+34h] [ebp-14h]
    unsigned __int16 dynEntId; // [esp+38h] [ebp-10h]
    unsigned int dent_id; // [esp+3Ch] [ebp-Ch]
    broad_phase_environment_info *bpei; // [esp+40h] [ebp-8h]
    gjk_entity_info_t *ent_info; // [esp+44h] [ebp-4h]

    dent_id = (unsigned int)dent;
    bpei = gjk_query_output::get_ent_info((unsigned int)dent);
    if (!bpei->m_data)
    {
        ent_info_ = gjk_query_output::create_entity_info();
        ent_info_->m_ent_type = gjk_entity_info_t::ENTITY_TYPE::ET_DENT;
        ent_info_->m_ent = dent;
        v5 = (DynEntityDrawType)(dent->xModel == 0);
        drawType = v5;
        dynEntId = DynEnt_GetId(dent, v5);
        dynEntPose = DynEnt_GetClientPose(dynEntId, v5);
        UnitQuatToAxis(dynEntPose->pose.quat, axis);
        Phys_AxisToNitrousMat(axis, ent_info_->m_mat);
        Phys_Vec3ToNitrousVec(dynEntPose->pose.origin, ent_info_->m_mat.w);
        bpei->m_data = ent_info_;
    }
    if (!bpei->m_data
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.cpp",
            667,
            0,
            "%s",
            "bpei->m_data"))
    {
        __debugbreak();
    }
    ent_info = (gjk_entity_info_t *)bpei->m_data;
    if (ent_info->m_ent_type != gjk_entity_info_t::ENTITY_TYPE::ET_DENT
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.cpp",
            669,
            0,
            "%s",
            "ent_info->is_dent()"))
    {
        __debugbreak();
    }
    if (ent_info->m_query_visitor_count != this->m_dent_query_visitor_count)
    {
        ent_info->m_query_visitor_count = this->m_dent_query_visitor_count;
        gjk_query_output::set_local_query_info(input, ent_info);
        create_gjk_geom(dent, this, input.m_contents);
    }
}

void __cdecl gjk_query_prims(const gjk_query_input &input, gjk_query_output *output)
{
    const cbrush_t *brush; // [esp+14h] [ebp-50h]
    const CollisionAabbTree *tree; // [esp+38h] [ebp-2Ch]
    int i; // [esp+3Ch] [ebp-28h]
    const col_prim_t *prim; // [esp+40h] [ebp-24h]
    col_prim_t *prims; // [esp+44h] [ebp-20h]
    float query_maxs[3]; // [esp+48h] [ebp-1Ch] BYREF
    int nprims; // [esp+54h] [ebp-10h]
    float query_mins[3]; // [esp+58h] [ebp-Ch] BYREF

    iassert(input.m_proximity_data->overflow == false);
    
    prims = input.m_proximity_data->prims;
    nprims = input.m_proximity_data->nprims;
    Phys_NitrousVecToVec3(output->m_query_aabb_min,query_mins);
    Phys_NitrousVecToVec3(output->m_query_aabb_max,query_maxs);
    prim = prims;
    for ( i = 0; i < nprims; ++i )
    {
        if ( prim->type )
        {
            iassert(prim->type == COL_BRUSH);
            brush = prim->brush;
            if ( (brush->contents & input.m_contents) != 0
                && query_maxs[0] >= brush->mins[0]
                && query_maxs[1] >= brush->mins[1]
                && query_maxs[2] >= brush->mins[2]
                && brush->maxs[0] >= query_mins[0]
                && brush->maxs[1] >= query_mins[1]
                && brush->maxs[2] >= query_mins[2] )
            {
                //gjk_query_output::add(output, input, brush, query_mins, query_maxs);
                output->add(input, brush, query_mins, query_maxs);
            }
        }
        else
        {
            tree = prim->tree;
            if ( (cm.materials[tree->materialIndex].contentFlags & input.m_contents) != 0
                && query_maxs[0] >= (float)(tree->origin[0] - tree->halfSize[0])
                && query_maxs[1] >= (float)(tree->origin[1] - tree->halfSize[1])
                && query_maxs[2] >= (float)(tree->origin[2] - tree->halfSize[2])
                && (float)(tree->origin[0] + tree->halfSize[0]) >= query_mins[0]
                && (float)(tree->origin[1] + tree->halfSize[1]) >= query_mins[1]
                && (float)(tree->origin[2] + tree->halfSize[2]) >= query_mins[2] )
            {
                //gjk_query_output::add(output, input, &cm.partitions[tree->u.firstChildIndex], tree);
                output->add(input, &cm.partitions[tree->u.firstChildIndex], tree);
            }
        }
        ++prim;
    }
}

void __cdecl gjk_query_terrain(const gjk_query_input &input, gjk_query_output *output)
{
    static_colgeom_visitor_t visitor; // [esp+2Ch] [ebp-1098h] BYREF
    float mx[3]; // [esp+10ACh] [ebp-18h] BYREF
    float mn[3]; // [esp+10B8h] [ebp-Ch] BYREF

    Phys_NitrousVecToVec3(output->m_query_aabb_min,mn);
    Phys_NitrousVecToVec3(output->m_query_aabb_max,mx);

    visitor.intersect_box(mn, mx, input.m_contents);

    for (int i = 0; i < visitor.nbrushes; ++i)
        output->add(input, visitor.brushes[i], mn, mx);

    for (int j = 0; j < visitor.ntrees; ++j)
        output->add(input, &cm.partitions[visitor.trees[j]->u.firstChildIndex], visitor.trees[j]);
}

void __cdecl gjk_query_gents(const gjk_query_input &input, gjk_query_output *output)
{
    DObj *ServerDObj; // [esp+0h] [ebp-1034h]
    gentity_s *gent; // [esp+8h] [ebp-102Ch]
    int i; // [esp+10h] [ebp-1024h]
    int entityList[1025]; // [esp+14h] [ebp-1020h] BYREF
    float maxs[3]; // [esp+1018h] [ebp-1Ch] BYREF
    int v7; // [esp+1024h] [ebp-10h]
    float outVector[3]; // [esp+1028h] [ebp-Ch] BYREF
    int savedregs; // [esp+1034h] [ebp+0h] BYREF

    ++output->m_gent_query_visitor_count;
    Phys_NitrousVecToVec3(output->m_query_aabb_min,outVector);
    Phys_NitrousVecToVec3(output->m_query_aabb_max,maxs);
    v7 = CM_AreaEntities(outVector, maxs, entityList, 1024, input.m_contents);
    for ( i = 0; i < v7; ++i )
    {
        gent = &g_entities[entityList[i]];
        if ( !query_should_pass_gentity(input, gent) )
        {
            if ( BG_ClipMoveToDobj(&gent->s) )
                ServerDObj = Com_GetServerDObj(gent->s.number);
            else
                ServerDObj = 0;
            if (!ServerDObj || DObjHasCollmap(ServerDObj) && DObjHasContents(ServerDObj, input.m_contents))
            {
                //gjk_query_output::add(output, (broad_phase_environment_info *)&savedregs, input, gent);
                output->add(input, gent);
            }
        }
    }
}

bool __cdecl query_should_pass_gentity(const gjk_query_input &input, const gentity_s *gent)
{
    if (input.m_pass_entity_num == 1023)
        return (gent->s.lerp.eFlags2 & 0x1000000) != 0;
    if (gent->s.number == input.m_pass_entity_num)
        return 1;
    //if (!EntHandle::isDefined(&gent->r.ownerNum))
    if (!gent->r.ownerNum.isDefined())
        return (gent->s.lerp.eFlags2 & 0x1000000) != 0;
    //if (EntHandle::entnum(&gent->r.ownerNum) == input->m_pass_entity_num)
    if (gent->r.ownerNum.entnum() == input.m_pass_entity_num)
        return 1;
    //return EntHandle::entnum(&gent->r.ownerNum) == input->m_pass_owner_num || (gent->s.lerp.eFlags2 & 0x1000000) != 0;
    return gent->r.ownerNum.entnum() == input.m_pass_owner_num || (gent->s.lerp.eFlags2 & 0x1000000) != 0;
}

void __cdecl gjk_query_cents(const gjk_query_input &input, gjk_query_output *output)
{
    DObj *ClientDObj; // [esp+0h] [ebp-1058h]
    int entityIndex; // [esp+24h] [ebp-1034h]
    int localClientNum; // [esp+28h] [ebp-1030h]
    centity_s *cent; // [esp+2Ch] [ebp-102Ch]
    int i; // [esp+34h] [ebp-1024h]
    int entityList[1025]; // [esp+38h] [ebp-1020h] BYREF
    float maxs[3]; // [esp+103Ch] [ebp-1Ch] BYREF
    int v10; // [esp+1048h] [ebp-10h]
    float outVector[3]; // [esp+104Ch] [ebp-Ch] BYREF
    int savedregs; // [esp+1058h] [ebp+0h] BYREF

    ++output->m_cent_query_visitor_count;
    Phys_NitrousVecToVec3(output->m_query_aabb_min,outVector);
    Phys_NitrousVecToVec3(output->m_query_aabb_max,maxs);
    v10 = CG_AreaEntities(outVector, maxs, entityList, 1024, input.m_contents);
    for ( i = 0; i < v10; ++i )
    {
        entityIndex = entityList[i];
        if ((CG_GetEntity(0, entityIndex)->nextState.lerp.eFlags2 & 0x1000000) == 0
            && entityIndex != input.m_pass_entity_num
            && entityIndex < 1024 )
        {
            localClientNum = cgCollWorldLocalClientNum;
            cent = CG_GetEntity(cgCollWorldLocalClientNum, entityIndex);
            if ( (cent->nextState.lerp.eFlags & 1) == 0 )
            {
                if ( BG_ClipMoveToDobj(&cent->nextState) )
                    ClientDObj = Com_GetClientDObj(entityIndex, localClientNum);
                else
                    ClientDObj = 0;
                if ( ClientDObj )
                {
                    if ( DObjHasCollmap(ClientDObj) && DObjHasContents(ClientDObj, input.m_contents) )
                        goto LABEL_22;
                }
                else if (cent->nextState.solid
                    && (cent->nextState.solid != 0xFFFFFF || (cent->nextState.lerp.eFlags & 1) == 0)
                    && (input.m_contents & CG_GetEntityBModelContents(cent)) != 0)
                {
LABEL_22:
                    //gjk_query_output::add(output, (broad_phase_environment_info *)&savedregs, input, cent);
                    output->add(input, cent);
                    continue;
                }
            }
        }
    }
}

void __cdecl gjk_query_glass(const gjk_query_input &input, gjk_query_output *output)
{
    unsigned int i; // [esp+0h] [ebp-1024h]
    float maxs[3]; // [esp+8h] [ebp-101Ch] BYREF
    unsigned int numGlasses; // [esp+14h] [ebp-1010h]
    float outVector[3]; // [esp+18h] [ebp-100Ch] BYREF
    const Glass *glasses[1024]; // [esp+24h] [ebp-1000h] BYREF

    if ( (input.m_contents & 0x10) != 0 )
    {
        Phys_NitrousVecToVec3(output->m_query_aabb_min,outVector);
        Phys_NitrousVecToVec3(output->m_query_aabb_max,maxs);
        if ( input.m_is_server_thread )
            numGlasses = GlassSv_AreaGlasses(outVector, maxs, glasses, 0x400u);
        else
            numGlasses = GlassCl_AreaGlasses(outVector, maxs, glasses, 0x400u);

        for ( i = 0; i < numGlasses; ++i )
            output->add(input, glasses[i]);
    }
}

void __cdecl gjk_query_dents(const gjk_query_input &input, gjk_query_output *output)
{
    const DynEntityDef *dynEntDef; // [esp+4h] [ebp-2044h]
    unsigned __int16 Id; // [esp+8h] [ebp-2040h]
    int i; // [esp+Ch] [ebp-203Ch]
    DynEntityDrawType drawType; // [esp+10h] [ebp-2038h]
    DynEntityAreaParms areaParms; // [esp+14h] [ebp-2034h] BYREF
    _WORD v7[4098]; // [esp+28h] [ebp-2020h] BYREF
    float v8[3]; // [esp+2030h] [ebp-18h] BYREF
    float outVector[3]; // [esp+203Ch] [ebp-Ch] BYREF

    ++output->m_dent_query_visitor_count;
    Phys_NitrousVecToVec3(output->m_query_aabb_min,outVector);
    Phys_NitrousVecToVec3(output->m_query_aabb_max,v8);
    areaParms.mins = outVector;
    areaParms.maxs = v8;
    areaParms.contentMask = input.m_contents;
    areaParms.list = v7;
    areaParms.maxCount = 4096;
    for ( drawType = DYNENT_DRAW_MODEL; drawType < DYNENT_DRAW_COUNT; ++drawType )
    {
        areaParms.count = 0;
        DynEntCl_AreaEntities_r((DynEntityCollType)drawType, 1u, &areaParms);
        for ( i = 0; i < areaParms.count; ++i )
        {
            dynEntDef = DynEnt_GetEntityDef(v7[i], drawType);
            if ( (dynEntDef->flags & 2) != 0 && DynEnt_GetEntityProps(dynEntDef->type)->usePhysics )
            {
                Id = DynEnt_GetId(dynEntDef, drawType);
                if ( !DynEnt_GetClientEntity(Id, drawType)->physObjId )
                    output->add(input, dynEntDef);
            }
        }
    }
}

void __cdecl gjk_query(const gjk_query_input &input, gjk_query_output *output)
{
    float mins[3]; // [esp+0h] [ebp-24h] BYREF
    float expand_vec[3]; // [esp+Ch] [ebp-18h] BYREF
    float maxs[3]; // [esp+18h] [ebp-Ch] BYREF

    ++output->m_total_query_count;
    if ( (input.m_gjk_query_flags & 1) != 0 )
    {
        if ( input.m_proximity_data )
        {
            Phys_NitrousVecToVec3(output->m_query_aabb_min,mins);
            Phys_NitrousVecToVec3(output->m_query_aabb_max,maxs);
            expand_vec[0] = 70.0f;
            expand_vec[1] = 70.0f;
            expand_vec[2] = 20.0f;
            input.m_proximity_data->update(mins, maxs, input.m_proximity_mask, expand_vec);

            iassert((input.m_proximity_mask & input.m_contents) == input.m_contents);

            if ( input.m_proximity_data->overflow )
                gjk_query_terrain(input, output);
            else
                gjk_query_prims(input, output);
        }
        else
        {
            gjk_query_terrain(input, output);
        }
    }
    if ( (input.m_gjk_query_flags & 2) != 0 )
    {
        if ( input.m_is_server_thread )
            gjk_query_gents(input, output);
        else
            gjk_query_cents(input, output);
        gjk_query_glass(input, output);
    }
    if ( (input.m_gjk_query_flags & 4) != 0 )
        gjk_query_dents(input, output);
}

void gjk_query_output::cached_query_resize(
                bool is_server_thread,
                colgeom_visitor_inlined_t<200> *proximity_data,
                int proximity_mask)
{
    phys_vec3 *v5; // eax
    phys_vec3 *v6; // [esp-20h] [ebp-E0h]
    phys_vec3 *v7; // [esp-1Ch] [ebp-DCh]
    phys_vec3 *p_m_query_aabb_max; // [esp-18h] [ebp-D8h]
    phys_vec3 *p_m_query_aabb_min; // [esp-14h] [ebp-D4h]
    cached_query_info_t *p_m_cached_query_info; // [esp-10h] [ebp-D0h]
    float x; // [esp-Ch] [ebp-CCh]
    float y; // [esp-8h] [ebp-C8h]
    float z; // [esp-4h] [ebp-C4h]
    float saved_query_aabb_max_4; // [esp+4h] [ebp-BCh]
    float saved_query_aabb_max_8; // [esp+8h] [ebp-B8h]
    float saved_query_aabb_max_12; // [esp+Ch] [ebp-B4h]
    gjk_query_input resize_input; // [esp+24h] [ebp-9Ch] BYREF
    gjk_query_output *thisa; // [esp+B0h] [ebp-10h]
    //_UNKNOWN *v19; // [esp+B4h] [ebp-Ch]
    //int is_server_threada; // [esp+B8h] [ebp-8h]
    //int proximity_maska; // [esp+C0h] [ebp+0h]
    //
    //v19 = a2;
    //is_server_threada = proximity_maska;
    thisa = this;

    iassert(!m_cached_query_info.is_empty());
    //if (cached_query_info_t::is_empty(&this->m_cached_query_info)
    //    && !Assert_MyHandler(
    //        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.cpp",
    //        1023,
    //        0,
    //        "%s",
    //        "!m_cached_query_info.is_empty()"))
    //{
    //    __debugbreak();
    //}
    iassert(m_cached_query_info.aabb_is_valid());
    //if (!cached_query_info_t::aabb_is_valid(&thisa->m_cached_query_info)
    //    && !Assert_MyHandler(
    //        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.cpp",
    //        1024,
    //        0,
    //        "%s",
    //        "m_cached_query_info.aabb_is_valid()"))
    //{
    //    __debugbreak();
    //}
    //resize_input.m_geom_skip_list.m_first = 0;
    //resize_input.m_geom_skip_list.m_last_next_ptr = &resize_input.m_geom_skip_list.m_first;
    //resize_input.m_geom_skip_list.m_alloc_count = 0;
    resize_input.m_geom_skip_list.remove_all();

    resize_input.m_contents = thisa->m_cached_query_info.m_query_contents;
    resize_input.m_pass_entity_num = 1023;
    resize_input.m_pass_owner_num = 1023;
    resize_input.m_is_server_thread = is_server_thread;
    resize_input.m_proximity_data = proximity_data;
    resize_input.m_proximity_mask = proximity_mask;
    resize_input.m_gjk_query_flags = thisa->m_cached_query_info.m_query_flags;
    //resize_input.m_geom_skip_list.m_first = 0;
    //resize_input.m_geom_skip_list.m_last_next_ptr = &resize_input.m_geom_skip_list.m_first;
    //resize_input.m_geom_skip_list.m_alloc_count = 0;
    resize_input.m_geom_skip_list.remove_all();

    saved_query_aabb_max_4 = thisa->m_query_aabb_min.x;
    saved_query_aabb_max_8 = thisa->m_query_aabb_min.y;
    saved_query_aabb_max_12 = thisa->m_query_aabb_min.z;
    x = thisa->m_query_aabb_max.x;
    y = thisa->m_query_aabb_max.y;
    z = thisa->m_query_aabb_max.z;
    p_m_cached_query_info = &thisa->m_cached_query_info;
    p_m_query_aabb_min = &thisa->m_query_aabb_min;
    thisa->m_query_aabb_min.x = thisa->m_cached_query_info.m_query_aabb_min.x;
    p_m_query_aabb_min->y = p_m_cached_query_info->m_query_aabb_min.y;
    p_m_query_aabb_min->z = p_m_cached_query_info->m_query_aabb_min.z;
    p_m_query_aabb_max = &thisa->m_cached_query_info.m_query_aabb_max;
    v7 = &thisa->m_query_aabb_max;
    thisa->m_query_aabb_max.x = thisa->m_cached_query_info.m_query_aabb_max.x;
    v7->y = p_m_query_aabb_max->y;
    v7->z = p_m_query_aabb_max->z;
    gjk_query(resize_input, thisa);
    v6 = &thisa->m_query_aabb_min;
    thisa->m_query_aabb_min.x = saved_query_aabb_max_4;
    v6->y = saved_query_aabb_max_8;
    v6->z = saved_query_aabb_max_12;
    v5 = &thisa->m_query_aabb_max;
    thisa->m_query_aabb_max.x = x;
    v5->y = y;
    v5->z = z;
}

bool cached_query_info_t::aabb_is_valid()
{
    return this->m_query_aabb_max.x >= this->m_query_aabb_min.x
            && this->m_query_aabb_max.y >= this->m_query_aabb_min.y
            && this->m_query_aabb_max.z >= this->m_query_aabb_min.z;
}

phys_vec3 CACHED_QUERY_RESIZE_EXTRA = { 10.0f, 10.0f, 10.0f };

void gjk_query_output::cached_query_prolog(
                bool is_server_thread,
                colgeom_visitor_inlined_t<200> *proximity_data,
                int proximity_mask,
                const phys_vec3 *start_origin)
{
    phys_transient_allocator::allocator_state *p_m_allocator_state; // edx
    char *m_cur; // [esp-70h] [ebp-7Ch]
    char *m_end; // [esp-6Ch] [ebp-78h]
    unsigned int m_total_memory_allocated; // [esp-68h] [ebp-74h]
    phys_vec3 v10; // [esp-50h] [ebp-5Ch] BYREF
    float v11; // [esp-40h] [ebp-4Ch]
    float v12; // [esp-3Ch] [ebp-48h]
    float v13; // [esp-38h] [ebp-44h]
    cached_query_info_t *p_m_accum_query_info; // [esp-34h] [ebp-40h]
    phys_vec3 v15; // [esp-30h] [ebp-3Ch] BYREF
    float v16; // [esp-14h] [ebp-20h]
    float v17; // [esp-10h] [ebp-1Ch]
    float v18; // [esp-Ch] [ebp-18h]
    phys_vec3 *p_m_query_aabb_max; // [esp-8h] [ebp-14h]
    gjk_query_output *v20; // [esp-4h] [ebp-10h]
    //unsigned int v21[2]; // [esp+0h] [ebp-Ch] BYREF
    //_UNKNOWN *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v21[0] = a2;
    //v21[1] = retaddr;
    v20 = this;
    gjk_query_output::verify_empty();

    if ( v20->m_accum_query_info.is_empty() )
    {
        iassert(m_cached_query_info.is_empty());

        //if ( !cached_query_info_t::is_empty(&v20->m_cached_query_info)
        //    && !Assert_MyHandler(
        //                "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.cpp",
        //                1069,
        //                0,
        //                "%s",
        //                "m_cached_query_info.is_empty()") )
        //{
        //    __debugbreak();
        //}
    }
    else
    {
        iassert(m_accum_query_info.aabb_is_valid());

        //if ( !cached_query_info_t::aabb_is_valid(&v20->m_accum_query_info)
        //    && !Assert_MyHandler(
        //                "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.cpp",
        //                1062,
        //                0,
        //                "%s",
        //                "m_accum_query_info.aabb_is_valid()") )
        //{
        //    __debugbreak();
        //}
        p_m_query_aabb_max = &v20->m_accum_query_info.m_query_aabb_max;
        v18 = v20->m_accum_query_info.m_query_aabb_max.x + start_origin->x;
        v17 = v20->m_accum_query_info.m_query_aabb_max.y + start_origin->y;
        v16 = v20->m_accum_query_info.m_query_aabb_max.z + start_origin->z;
        v15.x = v18;
        v15.y = v17;
        v15.z = v16;
        p_m_accum_query_info = &v20->m_accum_query_info;
        v13 = v20->m_accum_query_info.m_query_aabb_min.x + start_origin->x;
        v12 = v20->m_accum_query_info.m_query_aabb_min.y + start_origin->y;
        v11 = v20->m_accum_query_info.m_query_aabb_min.z + start_origin->z;
        v10.x = v13;
        v10.y = v12;
        v10.z = v11;

        //cached_query_info_t::init_query(
        //    &v20->m_cached_query_info,
        //    &v10,
        //    &v15,
        //    &CACHED_QUERY_RESIZE_EXTRA,
        //    v20->m_accum_query_info.m_query_contents,
        //    v20->m_accum_query_info.m_query_flags);
        //
            v20->m_cached_query_info.init_query(
            &v10,
            &v15,
            &CACHED_QUERY_RESIZE_EXTRA,
            v20->m_accum_query_info.m_query_contents,
            v20->m_accum_query_info.m_query_flags);

        gjk_query_output::cached_query_resize(is_server_thread, proximity_data, proximity_mask);
    }
    gjk_query_output::accum_query_reset(start_origin);
    m_cur = v20->m_allocator.m_cur;
    m_end = v20->m_allocator.m_end;
    m_total_memory_allocated = v20->m_allocator.m_total_memory_allocated;
    p_m_allocator_state = &v20->m_allocator_state;
    v20->m_allocator_state.m_first_block = v20->m_allocator.m_first_block;
    p_m_allocator_state->m_cur = m_cur;
    p_m_allocator_state->m_end = m_end;
    p_m_allocator_state->m_total_memory_allocated = m_total_memory_allocated;
}

void cached_query_info_t::init_query(
                const phys_vec3 *query_aabb_min,
                const phys_vec3 *query_aabb_max,
                const phys_vec3 *extra,
                int query_contents,
                unsigned int query_flags)
{
    float v6; // [esp-3Ch] [ebp-48h]
    float v7; // [esp-38h] [ebp-44h]
    float v8; // [esp-1Ch] [ebp-28h]
    float v9; // [esp-18h] [ebp-24h]

    v8 = query_aabb_min->y - extra->y;
    v9 = query_aabb_min->z - extra->z;
    this->m_query_aabb_min.x = query_aabb_min->x - extra->x;
    this->m_query_aabb_min.y = v8;
    this->m_query_aabb_min.z = v9;
    v6 = query_aabb_max->y + extra->y;
    v7 = query_aabb_max->z + extra->z;
    this->m_query_aabb_max.x = query_aabb_max->x + extra->x;
    this->m_query_aabb_max.y = v6;
    this->m_query_aabb_max.z = v7;
    this->m_query_contents = query_contents;
    this->m_query_flags = query_flags;
}

void gjk_query_output::cached_query_epilog()
{
    char *m_cur; // [esp+2Ch] [ebp-Ch]
    char *m_end; // [esp+30h] [ebp-8h]
    unsigned int m_total_memory_allocated; // [esp+34h] [ebp-4h]

    this->m_accum_query_info.m_query_aabb_min.x = this->m_accum_query_info.m_query_aabb_min.x
                                                                                            - this->m_accum_start_origin.x;
    this->m_accum_query_info.m_query_aabb_min.y = this->m_accum_query_info.m_query_aabb_min.y
                                                                                            - this->m_accum_start_origin.y;
    this->m_accum_query_info.m_query_aabb_min.z = this->m_accum_query_info.m_query_aabb_min.z
                                                                                            - this->m_accum_start_origin.z;
    this->m_accum_query_info.m_query_aabb_max.x = this->m_accum_query_info.m_query_aabb_max.x
                                                                                            - this->m_accum_start_origin.x;
    this->m_accum_query_info.m_query_aabb_max.y = this->m_accum_query_info.m_query_aabb_max.y
                                                                                            - this->m_accum_start_origin.y;
    this->m_accum_query_info.m_query_aabb_max.z = this->m_accum_query_info.m_query_aabb_max.z
                                                                                            - this->m_accum_start_origin.z;
    gjk_query_output::reset_cache();
    m_cur = this->m_allocator.m_cur;
    m_end = this->m_allocator.m_end;
    m_total_memory_allocated = this->m_allocator.m_total_memory_allocated;
    this->m_allocator_state.m_first_block = this->m_allocator.m_first_block;
    this->m_allocator_state.m_cur = m_cur;
    this->m_allocator_state.m_end = m_end;
    this->m_allocator_state.m_total_memory_allocated = m_total_memory_allocated;
}

void gjk_query_output::accum_query_reset(const phys_vec3 *start_origin)
{
    this->m_accum_start_origin.x = start_origin->x;
    this->m_accum_start_origin.y = start_origin->y;
    this->m_accum_start_origin.z = start_origin->z;
    this->m_accum_query_info.m_query_contents = 0;
    this->m_accum_query_info.m_query_flags = 0;
}

void    gjk_query_cached(const gjk_query_input &input, gjk_query_output *output)
{
    phys_vec3 mx; // [esp-Ch] [ebp-3Ch] BYREF
    phys_vec3 mn; // [esp+4h] [ebp-2Ch] BYREF
    float hit_time; // [esp+18h] [ebp-18h] BYREF
    int flags; // [esp+1Ch] [ebp-14h]
    gjk_geom_info_t *gi; // [esp+20h] [ebp-10h]

    iassert(input.m_contents != 0);

    ++output->m_total_cached_query_count;
    //if (!cached_query_info_t::is_subset(
      if (!output->m_cached_query_info.is_subset(
        &output->m_query_aabb_min,
        &output->m_query_aabb_max,
        input.m_contents,
        input.m_gjk_query_flags))
    {
        //cached_query_info_t::add_query(
            output->m_cached_query_info.add_query(
            &output->m_query_aabb_min,
            &output->m_query_aabb_max,
            &CACHED_QUERY_RESIZE_EXTRA,
            input.m_contents,
            input.m_gjk_query_flags);
        //gjk_query_output::cached_query_resize(
            output->cached_query_resize(
            input.m_is_server_thread,
            input.m_proximity_data,
            input.m_proximity_mask);
    }
    //cached_query_info_t::add_query(
        output->m_accum_query_info.add_query(
        &output->m_query_aabb_min,
        &output->m_query_aabb_max,
        &PHYS_ZERO_VEC,
        input.m_contents,
        input.m_gjk_query_flags);

    for (gi = output->m_total_list_geom_info; gi; gi = gi->m_total_next_link)
    {
        //if ((input->m_contents & gjk_base_t::get_contents(i->m_cg)) != 0)
        if ((input.m_contents & gi->m_cg->get_contents()) != 0)
        {
            if (gi->m_ent_info)
                flags = gi->m_ent_info->m_ent_type == gjk_entity_info_t::ENTITY_TYPE::ET_DENT ? 4 : 2;
            else
                flags = 1;
            if ((input.m_gjk_query_flags & flags) != 0
                && !query_should_pass_entity(input, gi->m_ent_info)
                && gi->m_query_visitor_count != output->m_query_visitor_count)
            {
                iassert(!input.is_in_skip_list(gi));

                gi->m_query_visitor_count = output->m_query_visitor_count;
                hit_time = 0.0f;
                //operator-(&mn, &i->m_aabb_min, &input->m_cg_aabb_max);
                mn = gi->m_aabb_min - input.m_cg_aabb_max;
                mx = gi->m_aabb_max - input.m_cg_aabb_min;
                //operator-(&mx, &i->m_aabb_max, &input->m_cg_aabb_min);
                if (gjk_trace_aabb(&input.m_cg_position, &input.m_cg_translation, &mn, &mx, &hit_time))
                {
                    gi->m_hit_time = hit_time;
                    //phys_link_list<gjk_geom_info_t>::add(&output->m_list_geom_info, i);
                    output->m_list_geom_info.add(gi);
                }
            }
        }
    }
}

bool gjk_query_input::is_in_skip_list(const gjk_geom_info_t *gi_) const
{
    gjk_geom_info_t *gi_i; // [esp+14h] [ebp-4h]

    for ( gi_i = this->m_geom_skip_list.m_first; gi_i; gi_i = gi_i->m_next_link )
    {
        if ( gi_i == gi_ )
            return 1;
    }
    return 0;
}

void cached_query_info_t::add_query(
                const phys_vec3 *query_aabb_min,
                const phys_vec3 *query_aabb_max,
                const phys_vec3 *extra,
                int query_contents,
                unsigned int query_flags)
{
    const phys_vec3 *v7; // eax
    phys_vec3 *v8; // [esp-C4h] [ebp-D0h]
    float v9; // [esp-BCh] [ebp-C8h]
    float v10; // [esp-B8h] [ebp-C4h]
    phys_vec3 v11; // [esp-A0h] [ebp-ACh] BYREF
    phys_vec3 v12; // [esp-90h] [ebp-9Ch] BYREF
    float v13; // [esp-80h] [ebp-8Ch]
    float v14; // [esp-7Ch] [ebp-88h]
    float v15; // [esp-78h] [ebp-84h]
    phys_vec3 *p_m_query_aabb_max; // [esp-74h] [ebp-80h]
    float v17; // [esp-70h] [ebp-7Ch]
    float v18; // [esp-6Ch] [ebp-78h]
    float v19; // [esp-68h] [ebp-74h]
    float v20; // [esp-60h] [ebp-6Ch]
    float v21; // [esp-5Ch] [ebp-68h]
    float v22; // [esp-58h] [ebp-64h]
    const phys_vec3 *v23; // [esp-54h] [ebp-60h]
    phys_vec3 v24; // [esp-50h] [ebp-5Ch] BYREF
    phys_vec3 v25; // [esp-40h] [ebp-4Ch] BYREF
    float v26; // [esp-2Ch] [ebp-38h]
    float v27; // [esp-28h] [ebp-34h]
    float v28; // [esp-24h] [ebp-30h]
    bool v29; // [esp-20h] [ebp-2Ch]
    float v30; // [esp-1Ch] [ebp-28h]
    float z; // [esp-18h] [ebp-24h]
    float v32; // [esp-14h] [ebp-20h]
    float y; // [esp-10h] [ebp-1Ch]
    float v34; // [esp-Ch] [ebp-18h]
    float x; // [esp-8h] [ebp-14h]
    cached_query_info_t *v36; // [esp-4h] [ebp-10h]
    //int v37; // [esp+0h] [ebp-Ch]
    //void *v38; // [esp+4h] [ebp-8h]
    //void *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v37 = a2;
    //v38 = retaddr;
    v36 = this;
    x = query_aabb_min->x;
    v34 = query_aabb_max->x;
    v29 = v34 >= x
         && (y = query_aabb_min->y, v32 = query_aabb_max->y, v32 >= y)
         && (z = query_aabb_min->z, v30 = query_aabb_max->z, v30 >= z);
    if ( !v29
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.h",
                    234,
                    0,
                    "%s",
                    "::aabb_is_valid(query_aabb_min,query_aabb_max)") )
    {
        __debugbreak();
    }
    if ( !query_contents
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.h",
                    235,
                    0,
                    "%s",
                    "query_contents != 0") )
    {
        __debugbreak();
    }
    if ( !query_flags
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.h",
                    236,
                    0,
                    "%s",
                    "query_flags != 0") )
    {
        __debugbreak();
    }
    if ( cached_query_info_t::is_empty() )
    {
        cached_query_info_t::init_query(query_aabb_min, query_aabb_max, extra, query_contents, query_flags);
    }
    else
    {
        v28 = v36->m_query_aabb_min.x + extra->x;
        v27 = v36->m_query_aabb_min.y + extra->y;
        v26 = v36->m_query_aabb_min.z + extra->z;
        v25.x = v28;
        v25.y = v27;
        v25.z = v26;
        v24 = phys_min(v25, *query_aabb_min);
        v23 = &v24;
        v22 = v23->x - extra->x;
        v21 = v23->y - extra->y;
        v20 = v23->z - extra->z;
        v17 = v22;
        v18 = v21;
        v19 = v20;
        v36->m_query_aabb_min.x = v22;
        v36->m_query_aabb_min.y = v18;
        v36->m_query_aabb_min.z = v19;
        p_m_query_aabb_max = &v36->m_query_aabb_max;
        v15 = v36->m_query_aabb_max.x - extra->x;
        v14 = v36->m_query_aabb_max.y - extra->y;
        v13 = v36->m_query_aabb_max.z - extra->z;
        v12.x = v15;
        v12.y = v14;
        v12.z = v13;
        v11 = phys_max(v12, *query_aabb_max);
        v7 = &v11;
        v9 = v7->y + extra->y;
        v10 = v7->z + extra->z;
        v8 = &v36->m_query_aabb_max;
        v36->m_query_aabb_max.x = v7->x + extra->x;
        v8->y = v9;
        v8->z = v10;
        v36->m_query_contents |= query_contents;
        v36->m_query_flags |= query_flags;
    }
}

bool cached_query_info_t::is_subset(
                const phys_vec3 *query_aabb_min,
                const phys_vec3 *query_aabb_max,
                int query_contents,
                unsigned int query_flags)
{
    bool v7; // [esp+4h] [ebp-50h]

    v7 = query_aabb_max->x >= query_aabb_min->x
        && query_aabb_max->y >= query_aabb_min->y
        && query_aabb_max->z >= query_aabb_min->z;
    if ( !v7
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.h",
                    268,
                    0,
                    "%s",
                    "::aabb_is_valid(query_aabb_min,query_aabb_max)") )
    {
        __debugbreak();
    }
    if ( !query_contents
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.h",
                    269,
                    0,
                    "%s",
                    "query_contents != 0") )
    {
        __debugbreak();
    }
    if ( !query_flags
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.h",
                    270,
                    0,
                    "%s",
                    "query_flags != 0") )
    {
        __debugbreak();
    }
    return !cached_query_info_t::is_empty()
            && cached_query_info_t::is_subset_aabb(query_aabb_min, query_aabb_max)
            && (query_contents & this->m_query_contents) == query_contents
            && (query_flags & this->m_query_flags) == query_flags;
}

bool cached_query_info_t::is_subset_aabb(
                const phys_vec3 *query_aabb_min,
                const phys_vec3 *query_aabb_max)
{
    return query_aabb_min->x >= this->m_query_aabb_min.x
            && query_aabb_min->y >= this->m_query_aabb_min.y
            && query_aabb_min->z >= this->m_query_aabb_min.z
            && this->m_query_aabb_max.x >= query_aabb_max->x
            && this->m_query_aabb_max.y >= query_aabb_max->y
            && this->m_query_aabb_max.z >= query_aabb_max->z;
}

static bool gjk_trace_plane_1d(float d1, float d2, float *min_t, float *max_t, bool use_iw_logic)
{
    if (d1 <= 0.0f)
    {
        if (d2 > 0.0f)
        {
            float delta = d1 - d2;
            iassert(delta < 0.0f); // "delta < 0.0f"

            if (!use_iw_logic)
                d1 -= 0.0625f;

            if (*max_t * delta > d1)
            {
                *max_t = d1 / delta;
                if (*min_t >= *max_t)
                    return false;
            }
        }
        return true;
    }

    float threshold = (d1 >= 0.125f) ? 0.125f : d1;

    if (d2 < threshold)
    {
        float delta = d1 - d2;
        iassert(delta > 0.0f); // "delta > 0.0f"

        float d1_biased = d1 - 0.125f;
        if (d1_biased > *min_t * delta)
        {
            *min_t = d1_biased / delta;
            if (*min_t >= *max_t)
                return false;
        }
        return true;
    }

    return d1 <= 0.0625f && !use_iw_logic;
}

char __cdecl gjk_trace_aabb(
                const phys_vec3 *p0,
                const phys_vec3 *dir,
                const phys_vec3 *aabb_min,
                const phys_vec3 *aabb_max,
                float *t_)
{
    float min_t = 0.0f;
    float max_t = 1.0f;

    for (int i = 0; i < 3; ++i)
    {
        float d_max = (*p0)[i] - (*aabb_max)[i];
        if (!gjk_trace_plane_1d(d_max, d_max + (*dir)[i], &min_t, &max_t, 0))
            return 0;

        float d_min = -((*p0)[i] - (*aabb_min)[i]);
        if (!gjk_trace_plane_1d(d_min, d_min - (*dir)[i], &min_t, &max_t, 0))
            return 0;
    }

    *t_ = min_t;
    return 1;
}

char __cdecl query_should_pass_entity(const gjk_query_input &input, gjk_entity_info_t *ent_info)
{
    const gentity_s *gent; // eax
    int entityIndex; // [esp+8h] [ebp-8h]

    if ( !ent_info )
        return 0;
    if ( ent_info->m_ent_type )
    {
        if ( ent_info->m_ent_type == gjk_entity_info_t::ENTITY_TYPE::ET_CENT )
        {
            //entityIndex = gjk_entity_info_t::get_cent(ent_info)->nextState.number;
            entityIndex = ent_info->get_cent()->nextState.number;
            return (CG_GetEntity(0, entityIndex)->nextState.lerp.eFlags2 & 0x1000000) != 0 || entityIndex == input.m_pass_entity_num;
        }
        else if ( ent_info->m_ent_type == gjk_entity_info_t::ENTITY_TYPE::ET_GLASS )
        {
            return 0;
        }
        else
        {
            if ( ent_info->m_ent_type != gjk_entity_info_t::ENTITY_TYPE::ET_DENT
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.cpp",
                            1112,
                            0,
                            "%s",
                            "ent_info->is_dent()") )
            {
                __debugbreak();
            }
            //gjk_entity_info_t::get_dent(ent_info);
            ent_info->get_dent();
            return 0;
        }
    }
    else
    {
        //gent = gjk_entity_info_t::get_gent(ent_info);
        gent = ent_info->get_gent();
        return query_should_pass_gentity(input, gent);
    }
}

void    setup_gjk_capsule(
                float *mins,
                float *maxs,
                float radius_adjust,
                gjk_double_sphere_t *gjk_capsule)
{
    phys_vec3 pv_mins, pv_maxs;
    Phys_Vec3ToNitrousVec(mins, pv_mins);
    Phys_Vec3ToNitrousVec(maxs, pv_maxs);

    // Compute dims, find the smallest horizontal extent (x vs z)
    phys_vec3 dim;
    dim.x = pv_maxs.x - pv_mins.x;
    dim.y = pv_maxs.y - pv_mins.y;
    dim.z = pv_maxs.z - pv_mins.z;

    float min_extent = (dim.x <= dim.z) ? dim.x : dim.z;
    float half_extent = 0.5f * min_extent;

    // Top sphere center: maxs pushed inward by half_extent
    phys_vec3 ptop;
    ptop.x = pv_maxs.x - half_extent;
    ptop.y = pv_maxs.y - half_extent;
    ptop.z = pv_maxs.z - half_extent;

    // Bottom sphere center: mins pushed inward by half_extent
    phys_vec3 pbot;
    pbot.x = pv_mins.x + half_extent;
    pbot.y = pv_mins.y + half_extent;
    pbot.z = pv_mins.z + half_extent;

    gjk_capsule->m_list_center[0] = ptop;
    gjk_capsule->m_list_center[1] = pbot;

    gjk_capsule->m_list_radius[0] = 0.0f;
    gjk_capsule->m_list_radius[1] = 0.0f;

    gjk_capsule->m_geom_radius = half_extent + radius_adjust;
    iassert(gjk_capsule->m_geom_radius > 0.0f);

    // Overall center is midpoint between the two sphere centers
    gjk_capsule->m_center.x = 0.5f * (ptop.x + pbot.x);
    gjk_capsule->m_center.y = 0.5f * (ptop.y + pbot.y);
    gjk_capsule->m_center.z = 0.5f * (ptop.z + pbot.z);

    gjk_capsule->m_count = 2;
}

void    setup_gjk_cylinder(
                float *mins,
                float *maxs,
                float radius_adjust,
                gjk_cylinder_t *gjk_cylinder)
{
    float v6; // [esp-8h] [ebp-50h]
    phys_vec3 pv_maxs; // [esp+1Ch] [ebp-2Ch] OVERLAPPED BYREF
    phys_vec3 pv_mins; // [esp+2Ch] [ebp-1Ch] BYREF
    //float pv_mins_4; // [esp+3Ch] [ebp-Ch]
    //void *pv_mins_8; // [esp+40h] [ebp-8h]
    //void *retaddr; // [esp+48h] [ebp+0h]
    //
    //pv_mins_4 = a1;
    //pv_mins_8 = retaddr;
    Phys_Vec3ToNitrousVec(mins, pv_mins);
    Phys_Vec3ToNitrousVec(maxs, pv_maxs);
    if ((float)(pv_maxs.x - pv_mins.x) <= (float)(pv_maxs.z - pv_mins.z))
        v6 = pv_maxs.x - pv_mins.x;
    else
        v6 = pv_maxs.z - pv_mins.z;
    gjk_cylinder->halfHeight = 0.5 * (float)(pv_maxs.z - pv_mins.z);
    gjk_cylinder->radius = 0.5 * v6;
    gjk_cylinder->direction = 2;
    gjk_cylinder->m_geom_radius = radius_adjust;
    //phys_mat44::operator=(&gjk_cylinder->xform, &PHYS_IDENTITY_MATRIX_42);
    gjk_cylinder->xform = PHYS_IDENTITY_MATRIX;
    gjk_cylinder->xform.w.x = 0.0f;
    gjk_cylinder->xform.w.y = 0.0f;
    gjk_cylinder->xform.w.z = (pv_mins.z + pv_maxs.z) * 0.5;
}

static bool gjk_aabb_is_walkable(
                const gjk_aabb_t *aabb,
                const phys_vec3 *hit_point,
                const phys_vec3 *up)
{
    char v5; // [esp-36h] [ebp-9Ah]
    bool v6; // [esp-35h] [ebp-99h]
    float v7; // [esp-30h] [ebp-94h]
    float v8; // [esp-2Ch] [ebp-90h]
    float v9; // [esp-28h] [ebp-8Ch]
    char v10; // [esp-1Eh] [ebp-82h]
    bool v11; // [esp-1Dh] [ebp-81h]
    float v12; // [esp-18h] [ebp-7Ch]
    float v13; // [esp-14h] [ebp-78h]
    float v14; // [esp-10h] [ebp-74h]
    int i; // [esp-Ch] [ebp-70h]
    phys_vec3 aabb_min; // [esp+28h] [ebp-3Ch]
    phys_vec3 aabb_max;
    float furthest_walk_normal; // [esp+4Ch] [ebp-18h]
    float furthest_dist; // [esp+50h] [ebp-14h]

    furthest_dist = -100000.0;
    furthest_walk_normal = 0.0f;

    aabb_min.x = aabb->m_center_local.x - aabb->m_dims.x;
    aabb_min.y = aabb->m_center_local.y - aabb->m_dims.y;
    aabb_min.z = aabb->m_center_local.z - aabb->m_dims.z;

    aabb_max.x = aabb->m_center_local.x + aabb->m_dims.x;
    aabb_max.y = aabb->m_center_local.y + aabb->m_dims.y;
    aabb_max.z = aabb->m_center_local.z + aabb->m_dims.z;

    for (i = 0; i < 3; ++i)
    {
        iassert(i >= 0 && i < 3);

        v14 = -hit_point->operator[](i);
        v13 = -aabb_min[i];
        v12 = -up->operator[](i);

        v11 = v14 >= (float)(0.99900001 * v13) || (float)(0.99900001 * v14) >= v13;
        if (v11 && v12 >= 0.69999999)
        {
            v10 = 1;
        }
        else
        {
            if ((float)(v14 - v13) > furthest_dist)
            {
                furthest_dist = v14 - v13;
                furthest_walk_normal = v12;
            }
            v10 = 0;
        }
        if (v10)
            return 1;

        iassert(i >= 0 && i < 3);

        v9 = hit_point->operator[](i);
        v8 = aabb_max[i];
        v7 = up->operator[](i);

        v6 = v9 >= (float)(0.99900001 * v8) || (float)(0.99900001 * v9) >= v8;
        if (v6 && v7 >= 0.69999999)
        {
            v5 = 1;
        }
        else
        {
            if ((float)(v9 - v8) > furthest_dist)
            {
                furthest_dist = v9 - v8;
                furthest_walk_normal = v7;
            }
            v5 = 0;
        }
        if (v5)
            return 1;
    }
    return furthest_walk_normal >= 0.69999999;
}

bool    is_walkable(
                const cbrush_t *brush,
                const phys_vec3 *hit_point_loc,
                const phys_vec3 *up_loc)
{
    char v5; // [esp-1Eh] [ebp-7Eh]
    bool v6; // [esp-1Dh] [ebp-7Dh]
    float dist; // [esp-18h] [ebp-78h]
    float v8; // [esp-14h] [ebp-74h]
    float v9; // [esp-10h] [ebp-70h]
    phys_vec3 normalVec; // [esp-Ch] [ebp-6Ch] BYREF
    cbrushside_t *side; // [esp+10h] [ebp-50h]
    unsigned int j; // [esp+14h] [ebp-4Ch]
    float v13; // [esp+18h] [ebp-48h]
    char v14; // [esp+1Eh] [ebp-42h]
    bool v15; // [esp+1Fh] [ebp-41h]
    float v16; // [esp+20h] [ebp-40h]
    float v17; // [esp+24h] [ebp-3Ch]
    float v18; // [esp+28h] [ebp-38h]
    float v19; // [esp+2Ch] [ebp-34h]
    float v20; // [esp+30h] [ebp-30h]
    char v21; // [esp+36h] [ebp-2Ah]
    bool v22; // [esp+37h] [ebp-29h]
    float v23; // [esp+38h] [ebp-28h]
    float v24; // [esp+3Ch] [ebp-24h]
    float mins; // [esp+40h] [ebp-20h]
    float v26; // [esp+44h] [ebp-1Ch]
    unsigned int i; // [esp+48h] [ebp-18h]
    float furthest_walk_normal; // [esp+4Ch] [ebp-14h]
    float furthest_dist; // [esp+50h] [ebp-10h]
    //_UNKNOWN *v30; // [esp+54h] [ebp-Ch]
    //const cbrush_t *brusha; // [esp+58h] [ebp-8h]
    //const phys_vec3 *up_loca; // [esp+60h] [ebp+0h]
    //
    //v30 = a1;
    //brusha = (const cbrush_t *)up_loca;
    furthest_dist = -100000.0;
    furthest_walk_normal = 0.0f;
    for (i = 0; i < 3; ++i)
    {
        (v26) =  -*((int *)&hit_point_loc->x + i) ;
        (mins) = -(brush->mins[i]) ;
        (v24) =  -*((int *)&up_loc->x + i) ;
        v23 = 0.99900001;
        v22 = v26 >= (float)(0.99900001 * mins) || (float)(v23 * v26) >= mins;
        if (v22 && v24 >= 0.69999999)
        {
            v21 = 1;
        }
        else
        {
            v20 = v26 - mins;
            if ((float)(v26 - mins) > furthest_dist)
            {
                furthest_dist = v20;
                furthest_walk_normal = v24;
            }
            v21 = 0;
        }
        if (v21)
            return 1;
        v19 = *(&hit_point_loc->x + i);
        v18 = brush->maxs[i];
        v17 = *(&up_loc->x + i);
        v16 = 0.99900001;
        v15 = v19 >= (float)(0.99900001 * v18) || (float)(v16 * v19) >= v18;
        if (v15 && v17 >= 0.69999999)
        {
            v14 = 1;
        }
        else
        {
            v13 = v19 - v18;
            if ((float)(v19 - v18) > furthest_dist)
            {
                furthest_dist = v13;
                furthest_walk_normal = v17;
            }
            v14 = 0;
        }
        if (v14)
            return 1;
    }
    for (j = 0; j < brush->numsides; ++j)
    {
        side = &brush->sides[j];
        Phys_Vec3ToNitrousVec(side->plane->normal, normalVec);
        v9 = (float)((float)(normalVec.x * up_loc->x) + (float)(normalVec.y * up_loc->y)) + (float)(normalVec.z * up_loc->z);
        v8 = (float)((float)(hit_point_loc->x * normalVec.x) + (float)(hit_point_loc->y * normalVec.y))
            + (float)(hit_point_loc->z * normalVec.z);
        dist = side->plane->dist;
        v6 = v8 >= (float)(0.99900001 * dist) || (float)(0.99900001 * v8) >= dist;
        if (v6 && v9 >= 0.69999999)
        {
            v5 = 1;
        }
        else
        {
            if ((float)(v8 - dist) > furthest_dist)
            {
                furthest_dist = v8 - dist;
                furthest_walk_normal = v9;
            }
            v5 = 0;
        }
        if (v5)
            return 1;
    }
    return furthest_walk_normal >= 0.69999999;
}

bool    is_walkable(
                const CollisionPartition *partition,
                const phys_vec3 *hit_point_loc,
                const phys_vec3 *up_loc)
{
    char v5; // [esp-12h] [ebp-82h]
    bool v6; // [esp-11h] [ebp-81h]
    float v7; // [esp-Ch] [ebp-7Ch]
    float v8; // [esp-8h] [ebp-78h]
    float v9; // [esp-4h] [ebp-74h]
    float walk_normal; // [esp+0h] [ebp-70h]
    phys_vec3 plane_dist; // [esp+4h] [ebp-6Ch] BYREF
    float triNormalScaledByAreaX2[3]; // [esp+20h] [ebp-50h] BYREF
    float v0_v2[3]; // [esp+2Ch] [ebp-44h] BYREF
    float v0_v1[3]; // [esp+38h] [ebp-38h] BYREF
    const float *v2; // [esp+44h] [ebp-2Ch]
    const float *v1; // [esp+48h] [ebp-28h]
    const float *v0; // [esp+4Ch] [ebp-24h]
    unsigned __int16 *indices; // [esp+50h] [ebp-20h]
    int triIndex; // [esp+54h] [ebp-1Ch]
    int i; // [esp+58h] [ebp-18h]
    float furthest_walk_normal; // [esp+5Ch] [ebp-14h]
    float furthest_dist; // [esp+60h] [ebp-10h]

    furthest_dist = -100000.0f;
    furthest_walk_normal = 0.0f;
    for (i = 0; i < partition->triCount; ++i)
    {
        triIndex = i + partition->firstTri;
        indices = &cm.triIndices[3 * triIndex];

        v0 = cm.verts[indices[0]];
        v1 = cm.verts[indices[1]];
        v2 = cm.verts[indices[2]];

        v0_v1[0] = v0[0] - v1[0];
        v0_v1[1] = v0[1] - v1[1];
        v0_v1[2] = v0[2] - v1[2];

        v0_v2[0] = v0[0] - v2[0];
        v0_v2[1] = v0[1] - v2[1];
        v0_v2[2] = v0[2] - v2[2];

        Vec3Cross(v0_v2, v0_v1, triNormalScaledByAreaX2);
        Phys_Vec3ToNitrousVec(triNormalScaledByAreaX2, plane_dist);
        walk_normal = Vec3Length(&plane_dist.x);
        if (walk_normal > 0.000099999997)
        {
            v9 = (float)((float)((float)(hit_point_loc->x * plane_dist.x) + (float)(hit_point_loc->y * plane_dist.y))
                + (float)(hit_point_loc->z * plane_dist.z))
                / walk_normal;
            v8 = (float)((float)((float)(*v0 * triNormalScaledByAreaX2[0]) + (float)(v0[1] * triNormalScaledByAreaX2[1]))
                + (float)(v0[2] * triNormalScaledByAreaX2[2]))
                / walk_normal;
            v7 = (float)((float)((float)(plane_dist.x * up_loc->x) + (float)(plane_dist.y * up_loc->y))
                + (float)(plane_dist.z * up_loc->z))
                / walk_normal;
            v6 = v9 >= (float)(0.99900001 * v8) || (float)(0.99900001 * v9) >= v8;
            if (v6 && v7 >= 0.69999999)
            {
                v5 = 1;
            }
            else
            {
                if ((float)(v9 - v8) > furthest_dist)
                {
                    furthest_dist = v9 - v8;
                    furthest_walk_normal = v7;
                }
                v5 = 0;
            }
            if (v5)
                return 1;
        }
    }
    return furthest_walk_normal >= 0.69999999;
}

// Was the gjk_base_t::is_walkable virtual (base returned false).
bool gjk_is_walkable(const gjk_base_t *geom, const phys_vec3 *hit_point, const phys_vec3 *up)
{
    switch (geom->get_type())
    {
    case GJK_AABB:
        return gjk_aabb_is_walkable((const gjk_aabb_t *)geom, hit_point, up);
    case GJK_BRUSH:
        return ::is_walkable(gjk_get_cbrush(geom), hit_point, up);
    case GJK_PARTITION:
        return ::is_walkable(gjk_get_partition((const gjk_partition_t *)geom), hit_point, up);
    default:
        return false;
    }
}

// Was the gjk_base_t::is_foot virtual (base returned false).
bool gjk_is_foot(const gjk_base_t *geom, const phys_vec3 *hit_point)
{
    switch (geom->get_type())
    {
    case GJK_DOUBLE_SPHERE:
    {
        const gjk_double_sphere_t *sphere = (const gjk_double_sphere_t *)geom;
        return (float)(sphere->m_list_center[1].z - 1.0) >= hit_point->z;
    }
    case GJK_CYLINDER:
    {
        const gjk_cylinder_t *cylinder = (const gjk_cylinder_t *)geom;
        phys_vec3 local_hit;
        local_hit = phys_full_inv_multiply(cylinder->xform, *hit_point);

        return (-cylinder->halfHeight) >= local_hit[cylinder->direction];
    }
    case GJK_POLYGON_CYLINDER:
    {
        const gjk_polygon_cylinder_t *pcyl = (const gjk_polygon_cylinder_t *)geom;
        if (pcyl->m_mode)
        {
            float foot_z = (pcyl->m_center.z - pcyl->m_half_height + pcyl->m_capsule_radius) - 1.0f;
            return foot_z >= hit_point->z;
        }

        static float offset = pcyl->m_geom_radius * sinf(DEG2RAD(30.0f)); // KISAKTODO: consider making this non-static? looks odd, does the radius never change?

        float foot_z = (pcyl->m_center.z - pcyl->m_half_height + pcyl->m_foot_offset) - offset;
        return foot_z >= hit_point->z;
    }
    default:
        return false;
    }
}

void    project(
                const phys_vec3 *point,
                phys_static_array<geom_plane,128> *list_geom_plane,
                phys_vec3 *result)
{
#if 0
    float v4; // [esp-2Ch] [ebp-178h]
    float v5; // [esp-28h] [ebp-174h]
    float v6; // [esp-14h] [ebp-160h]
    int m; // [esp-Ch] [ebp-158h]
    float v8; // [esp-8h] [ebp-154h]
    float v9; // [esp-4h] [ebp-150h]
    float v10; // [esp+0h] [ebp-14Ch]
    float v11; // [esp+4h] [ebp-148h]
    float v12; // [esp+8h] [ebp-144h]
    float ck_[3]; // [esp+Ch] [ebp-140h]
    float temp1[4]; // [esp+18h] [ebp-134h]
    int k; // [esp+28h] [ebp-124h]
    float ndist_sq; // [esp+2Ch] [ebp-120h]
    float det; // [esp+30h] [ebp-11Ch]
    int j; // [esp+34h] [ebp-118h]
    float v20[4]; // [esp+44h] [ebp-108h]
    int new_set; // [esp+54h] [ebp-F8h]
    float *new_dist_sq; // [esp+58h] [ebp-F4h]
    int i; // [esp+5Ch] [ebp-F0h]
    unsigned int v24[2]; // [esp+60h] [ebp-ECh]
    geom_plane *list_gp[4]; // [esp+6Ch] [ebp-E0h] BYREF
    float mat[4][4]; // [esp+7Ch] [ebp-D0h]
    float rs[4]; // [esp+BCh] [ebp-90h]
    int max_index; // [esp+CCh] [ebp-80h]
    int v29; // [esp+D0h] [ebp-7Ch]
    int v30; // [esp+D4h] [ebp-78h]
    int v31; // [esp+D8h] [ebp-74h]
    int new_index; // [esp+DCh] [ebp-70h]
    float *p_x; // [esp+E0h] [ebp-6Ch]
    float v34; // [esp+E4h] [ebp-68h]
    float dist; // [esp+E8h] [ebp-64h]
    geom_plane *v36; // [esp+ECh] [ebp-60h]
    geom_plane *m_slot_array; // [esp+F0h] [ebp-5Ch]
    float v38; // [esp+F4h] [ebp-58h]
    float *v39; // [esp+F8h] [ebp-54h]
    int m_alloc_count; // [esp+FCh] [ebp-50h]
    int support_gp_dist; // [esp+100h] [ebp-4Ch]
    geom_plane *support_gp; // [esp+104h] [ebp-48h]
    float v43; // [esp+108h] [ebp-44h]
    int iter; // [esp+10Ch] [ebp-40h]
    int break_val; // [esp+110h] [ebp-3Ch]
    int cur_set; // [esp+114h] [ebp-38h]
    float cur_dist_sq; // [esp+118h] [ebp-34h]
    phys_vec3 cur_result; // [esp+11Ch] [ebp-30h]
    geom_plane *v49; // [esp+12Ch] [ebp-20h]
    geom_plane *v50; // [esp+130h] [ebp-1Ch]
    geom_plane *v51; // [esp+134h] [ebp-18h]
    geom_plane *gp; // [esp+138h] [ebp-14h]
    phys_static_array<geom_plane,128>::iterator gp_i_end; // [esp+13Ch] [ebp-10h]
    //int v54; // [esp+140h] [ebp-Ch]
    //phys_static_array<geom_plane,128>::iterator gp_i; // [esp+144h] [ebp-8h]
    //geom_plane *retaddr; // [esp+14Ch] [ebp+0h]
    //
    //v54 = a1;
    //gp_i.m_ptr = retaddr;
    gp_i_end.m_ptr = list_geom_plane->m_slot_array;
    gp = gp_i_end.m_ptr;
    v51 = &list_geom_plane->m_slot_array[list_geom_plane->m_alloc_count];
    v50 = v51;
    while ( v50 != gp )
    {
        v49 = gp;
        gp->m_lambda = 0.0f;
        ++gp;
    }
    break_val = SLODWORD(point->x);
    cur_set = SLODWORD(point->y);
    cur_dist_sq = point->z;
    cur_result.x = point->w;
    iter = 0;
    v43 = 0.0;
    support_gp = (geom_plane *)-1;
    for ( support_gp_dist = 0; ; ++support_gp_dist )
    {
        m_alloc_count = list_geom_plane->m_alloc_count;
        if ( support_gp_dist >= m_alloc_count )
            break;
        v39 = 0;
        v38 = 9.9999998e12;
        m_slot_array = list_geom_plane->m_slot_array;
        v36 = m_slot_array;
        LODWORD(dist) = &list_geom_plane->m_slot_array[list_geom_plane->m_alloc_count];
        v34 = dist;
        while ( (geom_plane *)LODWORD(v34) != v36 )
        {
            p_x = &v36->m_normal.x;
            if ( v36->m_active && p_x[9] == 0.0 )
            {
                *(float *)&new_index = (float)((float)((float)(*(float *)&break_val * *p_x)
                                                                                         + (float)(*(float *)&cur_set * p_x[1]))
                                                                         + (float)(cur_dist_sq * p_x[2]))
                                                         - p_x[10];
                if ( v38 > *(float *)&new_index )
                {
                    v38 = *(float *)&new_index;
                    v39 = p_x;
                }
            }
            ++v36;
        }
        if ( !v39 )
        {
            support_gp = 0;
            break;
        }
        if ( v38 >= 0.0 )
        {
            support_gp = (geom_plane *)1;
            break;
        }
        v39[9] = -1.0f;
        if ( LODWORD(v43) > 0xE
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.cpp",
                        1494,
                        0,
                        "%s",
                        "cur_set >= 0 && cur_set < 15") )
        {
            __debugbreak();
        }
        if ( (LOBYTE(v43) & 1) != 0 )
        {
            if ( (LOBYTE(v43) & 2) != 0 )
                v31 = ((LOBYTE(v43) & 4) != 0) + 2;
            else
                v31 = 1;
            v30 = v31;
        }
        else
        {
            v30 = 0;
        }
        v29 = v30;
        if ( (LOBYTE(v43) & 8) != 0 )
        {
            max_index = 4;
        }
        else
        {
            if ( (LOBYTE(v43) & 4) != 0 )
            {
                LODWORD(rs[3]) = 3;
            }
            else
            {
                if ( (LOBYTE(v43) & 2) != 0 )
                    LODWORD(rs[2]) = 2;
                else
                    LODWORD(rs[2]) = (LOBYTE(v43) & 1) != 0;
                rs[3] = rs[2];
            }
            max_index = LODWORD(rs[3]);
        }
        LODWORD(rs[1]) = max_index;
        mat[3][v29 + 1] = v39[10]
                                        - (float)((float)((float)(point->x * *v39) + (float)(point->y * v39[1])) + (float)(point->z * v39[2]));
        *(&list_gp[4 * v29 + 1] + v29) = (geom_plane *)LODWORD(1.0f);
        v24[v29] = v39;
        for ( i = 0; i < SLODWORD(rs[1]); ++i )
        {
            if ( (LODWORD(v43) & (1 << i)) != 0 )
            {
                new_dist_sq = (float *)v24[i];
                new_set = v24[v29];
                *((float *)&list_gp[4 * v29 + 1] + i) = (float)((float)(*(float *)new_set * *new_dist_sq)
                                                                                                            + (float)(*(float *)(new_set + 4) * new_dist_sq[1]))
                                                                                            + (float)(*(float *)(new_set + 8) * new_dist_sq[2]);
                *(&list_gp[4 * i + 1] + v29) = *(&list_gp[4 * v29 + 1] + i);
            }
        }
        v20[3] = mat[3][v29 + 1];
        v20[2] = v20[3] * v20[3];
        LODWORD(v20[1]) = 1 << v29;
        v20[v29 + 1] = mat[3][v29 + 1];
        for ( j = 0; j < SLODWORD(rs[1]); ++j )
        {
            if ( (LODWORD(v43) & (1 << j)) != 0 )
            {
                det = mat[3][v29 + 1] - (float)(mat[3][j + 1] * *((float *)&list_gp[4 * v29 + 1] + j));
                if ( det >= 0.0 )
                {
                    ndist_sq = mat[3][j + 1] - (float)(mat[3][v29 + 1] * *((float *)&list_gp[4 * v29 + 1] + j));
                    if ( ndist_sq >= 0.0 )
                    {
                        k = *((int *)&list_gp[4 * v29 + 1] + j);
                        temp1[3] = 1.0 - (float)(*(float *)&k * *(float *)&k);
                        if ( temp1[3] > 0.0000099999997 )
                        {
                            temp1[2] = (float)((float)((float)((float)((float)(2.0 * ndist_sq) * *((float *)&list_gp[4 * v29 + 1] + j))
                                                                                             + det)
                                                                             * det)
                                                             + (float)(ndist_sq * ndist_sq))
                                             / (float)(temp1[3] * temp1[3]);
                            if ( temp1[2] > v20[2] )
                            {
                                v20[v29 + 1] = det / temp1[3];
                                v20[j + 1] = ndist_sq / temp1[3];
                                v20[2] = temp1[2];
                                LODWORD(v20[1]) = (1 << j) | (1 << v29);
                            }
                        }
                    }
                }
                LODWORD(temp1[1]) = j + 1;
                while ( SLODWORD(temp1[1]) < SLODWORD(rs[1]) )
                {
                    if ( (LODWORD(v43) & (1 << SLOBYTE(temp1[1]))) != 0 )
                    {
                        ck_[v29] = (float)(*((float *)&list_gp[4 * LODWORD(temp1[1]) + 1] + j) * mat[3][LODWORD(temp1[1]) + 1])
                                         - (float)(mat[3][j + 1] * *((float *)&list_gp[4 * LODWORD(temp1[1]) + 1] + LODWORD(temp1[1])));
                        ck_[j] = (float)(COERCE_FLOAT((unsigned int)*(&list_gp[4 * LODWORD(temp1[1]) + 1] + v29) ^ _mask__NegFloat_)
                                                     * mat[3][LODWORD(temp1[1]) + 1])
                                     + (float)(mat[3][v29 + 1] * *((float *)&list_gp[4 * LODWORD(temp1[1]) + 1] + LODWORD(temp1[1])));
                        ck_[LODWORD(temp1[1])] = (float)(*((float *)&list_gp[4 * LODWORD(temp1[1]) + 1] + v29) * mat[3][j + 1])
                                                                     - (float)(mat[3][v29 + 1] * *((float *)&list_gp[4 * LODWORD(temp1[1]) + 1] + j));
                        v12 = (float)((float)(ck_[v29] * *((float *)&list_gp[4 * j + 1] + v29))
                                                + (float)(ck_[j] * *((float *)&list_gp[4 * j + 1] + j)))
                                + (float)(ck_[LODWORD(temp1[1])] * *((float *)&list_gp[4 * j + 1] + LODWORD(temp1[1])));
                        if ( v12 >= 0.0 )
                        {
                            LODWORD(v11) = COERCE_UNSIGNED_INT(
                                                             (float)((float)(ck_[v29] * *((float *)&list_gp[4 * v29 + 1] + v29))
                                                                         + (float)(ck_[j] * *((float *)&list_gp[4 * v29 + 1] + j)))
                                                         + (float)(ck_[LODWORD(temp1[1])] * *((float *)&list_gp[4 * v29 + 1] + LODWORD(temp1[1]))))
                                                     ^ _mask__NegFloat_;
                            if ( v11 >= 0.0 )
                            {
                                ck_[v29] = (float)(*((float *)&list_gp[4 * v29 + 1] + j)
                                                                 * *((float *)&list_gp[4 * j + 1] + LODWORD(temp1[1])))
                                                 - (float)(*((float *)&list_gp[4 * j + 1] + j)
                                                                 * *((float *)&list_gp[4 * v29 + 1] + LODWORD(temp1[1])));
                                ck_[j] = (float)(COERCE_FLOAT((unsigned int)*(&list_gp[4 * v29 + 1] + v29) ^ _mask__NegFloat_)
                                                             * *((float *)&list_gp[4 * j + 1] + LODWORD(temp1[1])))
                                             + (float)(*((float *)&list_gp[4 * j + 1] + v29)
                                                             * *((float *)&list_gp[4 * v29 + 1] + LODWORD(temp1[1])));
                                ck_[LODWORD(temp1[1])] = (float)(*((float *)&list_gp[4 * v29 + 1] + v29)
                                                                                             * *((float *)&list_gp[4 * j + 1] + j))
                                                                             - (float)(*((float *)&list_gp[4 * j + 1] + v29)
                                                                                             * *((float *)&list_gp[4 * v29 + 1] + j));
                                v10 = (float)((float)(ck_[v29] * mat[3][v29 + 1]) + (float)(ck_[j] * mat[3][j + 1]))
                                        + (float)(ck_[LODWORD(temp1[1])] * mat[3][LODWORD(temp1[1]) + 1]);
                                if ( v10 >= 0.0 )
                                {
                                    v9 = (float)((float)(ck_[v29] * *((float *)&list_gp[4 * LODWORD(temp1[1]) + 1] + v29))
                                                         + (float)(ck_[j] * *((float *)&list_gp[4 * LODWORD(temp1[1]) + 1] + j)))
                                         + (float)(ck_[LODWORD(temp1[1])]
                                                         * *((float *)&list_gp[4 * LODWORD(temp1[1]) + 1] + LODWORD(temp1[1])));
                                    if ( v9 > 0.0000099999997 )
                                    {
                                        v8 = (float)((float)((float)((float)(v12 * v12) + (float)(v11 * v11)) + (float)(v10 * v10))
                                                             + (float)((float)((float)((float)((float)(v11 * *((float *)&list_gp[4 * v29 + 1] + j))
                                                                                                                             + (float)(v10
                                                                                                                                             * *((float *)&list_gp[4 * v29 + 1]
                                                                                                                                                 + LODWORD(temp1[1]))))
                                                                                                             * v12)
                                                                                             + (float)((float)(v11 * v10)
                                                                                                             * *((float *)&list_gp[4 * j + 1] + LODWORD(temp1[1]))))
                                                                             * 2.0))
                                             / (float)(v9 * v9);
                                        if ( v8 > v20[2] )
                                        {
                                            v20[v29 + 1] = v12 / v9;
                                            v20[j + 1] = v11 / v9;
                                            v20[LODWORD(temp1[1]) + 1] = v10 / v9;
                                            v20[2] = v8;
                                            LODWORD(v20[1]) = (1 << SLOBYTE(temp1[1])) | (1 << j) | (1 << v29);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    ++LODWORD(temp1[1]);
                }
            }
        }
        if ( *(float *)&iter > v20[2] )
        {
            support_gp = (geom_plane *)2;
            break;
        }
        iter = SLODWORD(v20[2]);
        v43 = v20[1];
        break_val = SLODWORD(point->x);
        cur_set = SLODWORD(point->y);
        cur_dist_sq = point->z;
        for ( m = 0; m < 4; ++m )
        {
            if ( (LODWORD(v43) & (1 << m)) != 0 )
            {
                v6 = v20[m + 1];
                v4 = v6 * *(float *)(v24[m] + 4);
                v5 = v6 * *(float *)(v24[m] + 8);
                *(float *)&break_val = *(float *)&break_val + (float)(v6 * *(float *)v24[m]);
                *(float *)&cur_set = *(float *)&cur_set + v4;
                cur_dist_sq = cur_dist_sq + v5;
            }
        }
    }
    result->x = *(float *)&break_val;
    result->y = *(float *)&cur_set;
    result->z = cur_dist_sq;
#else // aislop

    // Reset all lambdas to 0 (= "not in simplex" sentinel)
    for (geom_plane *gp = &*list_geom_plane->begin(),
        *end = &*list_geom_plane->begin() + list_geom_plane->get_count();
        gp != end; ++gp)
    {
        gp->m_lambda = 0.0f;
    }

    // cur_result accumulates: starts at point, gets lambda_i * normal_i added per simplex update
    phys_vec3 cur_result;
    cur_result.x = point->x;
    cur_result.y = point->y;
    cur_result.z = point->z;

    // cur_set: bitmask of planes currently in best simplex (0-3 bits)
    int   cur_set = 0;
    // iter / cur_dist_sq: best squared distance found so far (stored as float, compared as float)
    float cur_dist_sq = 0.0f;

    // rhs[i]        = right_side_i - dot(point, normal_i)   for simplex plane i (0-based slot)
    // cross[i][j]   = dot(normal_i, normal_j)               for i != j
    // lambda[i]     = barycentric weight for best candidate simplex
    float rhs[4];          // [ebp-90h] indexed by slot
    float cross[4][4];     // [ebp-0D0h] indexed [row][col], row/col are 0-based simplex slots
    float lambda[4];       // [ebp-108h] best lambdas for current best candidate
    float new_dist_sq;     // [ebp-0F4h] best dist_sq for current support plane's candidates
    int   new_set;         // [ebp-0F8h] bitmask for current best candidate

    // list_gp[i]: pointer to the geom_plane for simplex slot i
    geom_plane *list_gp[4];

    // support_gp: exit sentinel
    //   (geom_plane*)-1 = initial / no break yet
    //   (geom_plane*) 0 = no violated plane found (converged)
    //   (geom_plane*) 1 = all violated planes satisfied (feasible)
    //   (geom_plane*) 2 = dist stopped improving
    geom_plane *support_gp = (geom_plane *)-1;

    for (int support_iter = 0; support_iter < list_geom_plane->get_count(); ++support_iter)
    {
        // -------------------------------------------------------
        // Find most violated active plane not yet in simplex
        // "active" = m_active != 0, "not in simplex" = m_lambda == 0.0
        // Violation = dot(cur_result, normal) - right_side  (most negative = most violated)
        // -------------------------------------------------------
        geom_plane *best_gp = nullptr;
        float       best_viol = 9.9999998e12f;

        for (geom_plane *gp = &*list_geom_plane->begin(),
            *end = &*list_geom_plane->begin() + list_geom_plane->get_count();
            gp != end; ++gp)
        {
            if (gp->m_active && gp->m_lambda == 0.0f)
            {
                float viol = cur_result.x * gp->m_normal.x
                    + cur_result.y * gp->m_normal.y
                    + cur_result.z * gp->m_normal.z
                    - gp->m_right_side;
                if (best_viol > viol)
                {
                    best_viol = viol;
                    best_gp = gp;
                }
            }
        }

        if (!best_gp)
        {
            support_gp = nullptr; // no active planes at all
            break;
        }
        if (best_viol >= 0.0f)
        {
            support_gp = (geom_plane *)1; // all satisfied
            break;
        }

        // Mark as in simplex
        best_gp->m_lambda = -1.0f;

        // Validate cur_set
        iassert(cur_set >= 0 && cur_set < 15);

        // Compute new_index = popcount(cur_set), i.e. next free slot index (0-based)
        int new_index;
        if (cur_set & 1)
        {
            if (cur_set & 2)
                new_index = ((cur_set & 4) ? 1 : 0) + 2;
            else
                new_index = 1;
        }
        else
        {
            new_index = 0;
        }

        // max_index = number of planes currently in simplex before adding new one
        int max_index;
        if (cur_set & 8) max_index = 4;
        else if (cur_set & 4) max_index = 3;
        else if (cur_set & 2) max_index = 2;
        else                  max_index = (cur_set & 1) ? 1 : 0;

        // Store new plane's RHS term and record its pointer
        rhs[new_index] = best_gp->m_right_side
            - (point->x * best_gp->m_normal.x
                + point->y * best_gp->m_normal.y
                + point->z * best_gp->m_normal.z);
        cross[new_index][new_index] = 1.0f;  // diagonal sentinel (used in cross-product checks)
        list_gp[new_index] = best_gp;

        // Compute cross-dot terms between new plane and existing simplex planes
        for (int i = 0; i < max_index; ++i)
        {
            if (cur_set & (1 << i))
            {
                float dot_ij = list_gp[new_index]->m_normal.x * list_gp[i]->m_normal.x
                    + list_gp[new_index]->m_normal.y * list_gp[i]->m_normal.y
                    + list_gp[new_index]->m_normal.z * list_gp[i]->m_normal.z;
                cross[new_index][i] = dot_ij;
                cross[i][new_index] = dot_ij;
            }
        }

        // -------------------------------------------------------
        // Johnson distance algorithm: find best subset of
        // {cur_set | new_index} that minimizes squared distance
        // -------------------------------------------------------
        // Start with singleton {new_index}
        new_dist_sq = rhs[new_index];
        float new_dist_sq_sq = new_dist_sq * new_dist_sq;
        new_set = 1 << new_index;
        lambda[new_index] = rhs[new_index];

        // Test all 2-plane subsets {new_index, j}
        for (int j = 0; j < max_index; ++j)
        {
            if (!(cur_set & (1 << j)))
                continue;

            // det_new = rhs[new_index] - rhs[j] * cross[new_index][j]
            float det_new = rhs[new_index] - rhs[j] * cross[new_index][j];
            if (det_new < 0.0f)
                goto next_j;

            {
                // det_j = rhs[j] - rhs[new_index] * cross[new_index][j]
                float det_j = rhs[j] - rhs[new_index] * cross[new_index][j];
                if (det_j < 0.0f)
                    goto next_j;

                // cos_ij = cross[new_index][j]
                float cos_ij = cross[new_index][j];
                float sin_sq = 1.0f - cos_ij * cos_ij;

                if (sin_sq > 0.0000099999997f)
                {
                    // dist_sq for {new_index, j}
                    float d = (2.0f * det_j * cross[new_index][j] + det_new) * det_new
                        + det_j * det_j;
                    d /= (sin_sq * sin_sq);

                    if (d > new_dist_sq_sq)
                    {
                        lambda[new_index] = det_new / sin_sq;
                        lambda[j] = det_j / sin_sq;
                        new_dist_sq_sq = d;
                        new_set = (1 << j) | (1 << new_index);
                    }
                }

                // Test all 3-plane subsets {new_index, j, k}
                for (int k = j + 1; k < max_index; ++k)
                {
                    if (!(cur_set & (1 << k)))
                        continue;

                    // First set of cofactors (using cross table, row k)
                    float ck_new = cross[k][j] * rhs[k] - rhs[j] * cross[k][k];
                    float ck_j = -cross[k][new_index] * rhs[k] + rhs[new_index] * cross[k][k];
                    float ck_k = cross[k][new_index] * rhs[j] - rhs[new_index] * cross[k][j];

                    // Feasibility check 1: dot with row j of cross table
                    float check1 = ck_new * cross[j][new_index]
                        + ck_j * cross[j][j]
                        + ck_k * cross[j][k];
                    if (check1 < 0.0f)
                        continue;

                    // Feasibility check 2: negated dot with row new_index
                    float check2 = -(ck_new * cross[new_index][new_index]
                        + ck_j * cross[new_index][j]
                        + ck_k * cross[new_index][k]);
                    if (check2 < 0.0f)
                        continue;

                    // Second set of cofactors (using cross table differently)
                    float ck2_new = cross[new_index][j] * cross[j][k]
                        - cross[j][j] * cross[new_index][k];
                    float ck2_j = -cross[new_index][new_index] * cross[j][k]
                        + cross[j][new_index] * cross[new_index][k];
                    float ck2_k = cross[new_index][new_index] * cross[j][j]
                        - cross[j][new_index] * cross[new_index][j];

                    // check3: dot of ck2 with rhs
                    float check3 = ck2_new * rhs[new_index]
                        + ck2_j * rhs[j]
                        + ck2_k * rhs[k];
                    if (check3 < 0.0f)
                        continue;

                    // denom: dot of ck2 with cross row k
                    float denom = ck2_new * cross[k][new_index]
                        + ck2_j * cross[k][j]
                        + ck2_k * cross[k][k];
                    if (denom <= 0.0000099999997f)
                        continue;

                    // dist_sq for {new_index, j, k}
                    float d3 = check1 * check1 + check2 * check2 + check3 * check3
                        + 2.0f * ((check2 * cross[new_index][j]
                            + check3 * cross[new_index][k]) * check1
                            + check2 * check3 * cross[j][k]);
                    d3 /= (denom * denom);

                    if (d3 > new_dist_sq_sq)
                    {
                        lambda[new_index] = check1 / denom;
                        lambda[j] = check2 / denom;
                        lambda[k] = check3 / denom;
                        new_dist_sq_sq = d3;
                        new_set = (1 << k) | (1 << j) | (1 << new_index);
                    }
                }
            }

        next_j:;
        }

        // Check if distance improved
        if (cur_dist_sq > new_dist_sq_sq)
        {
            support_gp = (geom_plane *)2;
            break;
        }

        // Accept this update
        cur_dist_sq = new_dist_sq_sq;
        cur_set = new_set;

        // Recompute cur_result = point + sum(lambda[m] * normal_m) for m in cur_set
        cur_result.x = point->x;
        cur_result.y = point->y;
        cur_result.z = point->z;
        for (int m = 0; m < 4; ++m)
        {
            if (cur_set & (1 << m))
            {
                float lam = lambda[m];
                cur_result.x += lam * list_gp[m]->m_normal.x;
                cur_result.y += lam * list_gp[m]->m_normal.y;
                cur_result.z += lam * list_gp[m]->m_normal.z;
            }
        }
    }

    result->x = cur_result.x;
    result->y = cur_result.y;
    result->z = cur_result.z;
#endif
}

void __cdecl get_material_from_brush(const cbrush_t *brush, int *sflags)
{
    cplane_s *plane; // edx
    unsigned int j; // [esp+18h] [ebp-1Ch]
    float best_dot; // [esp+28h] [ebp-Ch]
    unsigned int i; // [esp+2Ch] [ebp-8h]
    unsigned int ia; // [esp+2Ch] [ebp-8h]
    unsigned int ib; // [esp+2Ch] [ebp-8h]

    if ( s_caulk_sflags == 0x7FFFFFFF )
    {
        s_caulk_sflags = 2147483646;
        for ( i = 0; i < cm.numMaterials; ++i )
        {
            if ( !strcmp(cm.materials[i].material, "caulk") )
            {
                s_caulk_sflags = cm.materials[i].surfaceFlags;
                break;
            }
        }
    }
    *sflags = brush->axial_sflags[1][2];
    if ( *sflags == s_caulk_sflags )
    {
        *sflags = 0x7FFFFFFF;
        best_dot = -10.0f;
        for ( ia = 0; ia < brush->numsides; ++ia )
        {
            if ( brush->sides[ia].sflags != s_caulk_sflags )
            {
                plane = brush->sides[ia].plane;
                if ( (float)((float)((float)(plane->normal[0] * 0.0) + (float)(plane->normal[1] * 0.0))
                                     + (float)(plane->normal[2] * 1.0)) > best_dot )
                {
                    best_dot = (float)((float)(plane->normal[0] * 0.0) + (float)(plane->normal[1] * 0.0))
                                     + (float)(plane->normal[2] * 1.0);
                    *sflags = brush->sides[ia].sflags;
                }
            }
        }
        if ( *sflags == 0x7FFFFFFF )
        {
            for ( ib = 0; ib < 2; ++ib )
            {
                for ( j = 0; j < 3; ++j )
                {
                    if ( brush->axial_sflags[ib][j] != s_caulk_sflags )
                    {
                        *sflags = brush->axial_sflags[ib][j];
                        return;
                    }
                }
            }
            *sflags = s_caulk_sflags;
        }
    }
}

void __cdecl fill_results(const gjk_trace_output_t &gto, bool is_walkable, trace_t *trace)
{
    const cbrush_t *v3; // eax
    unsigned __int16 number; // [esp+4h] [ebp-10h]
    unsigned __int16 v5; // [esp+6h] [ebp-Eh]
    dmaterial_t *materialInfo; // [esp+8h] [ebp-Ch]

    trace->fraction = gto.m_hit_time;
    trace->allsolid = 0;
    trace->startsolid = gto.m_hit_dist < 0.0;
    Phys_NitrousVecToVec3(gto.m_hit_normal,trace->normal.vec.v);
    trace->walkable = is_walkable;
    if ( gto.m_gi->m_ent_info )
    {
        if ( gto.m_gi->m_ent_info->m_ent_type )
        {
            if ( gto.m_gi->m_ent_info->m_ent_type == gjk_entity_info_t::ENTITY_TYPE::ET_CENT )
            {
                //number = gjk_entity_info_t::get_cent(gto->m_gi->m_ent_info)->nextState.number;
                number = gto.m_gi->m_ent_info->get_cent()->nextState.number;
                iassert(trace);
                trace->hitType = TRACE_HITTYPE_ENTITY;
                trace->hitId = number;
            }
            else if ( gto.m_gi->m_ent_info->m_ent_type == gjk_entity_info_t::ENTITY_TYPE::ET_GLASS )
            {
                iassert(trace);
                trace->hitType = TRACE_HITTYPE_ENTITY;
                trace->hitId = 1022;
            }
            else
            {
                iassert(gto.m_gi->m_ent_info->is_dent());
                iassert(trace);
                trace->hitType = TRACE_HITTYPE_ENTITY;
                trace->hitId = 1022;
            }
        }
        else
        {
            //v5 = gjk_entity_info_t::get_gent(gto->m_gi->m_ent_info)->s.number;
            v5 = gto.m_gi->m_ent_info->get_gent()->s.number;
            iassert(trace);
            trace->hitType = TRACE_HITTYPE_ENTITY;
            trace->hitId = v5;
        }
    }
    else
    {
        iassert(trace);
        trace->hitType = TRACE_HITTYPE_ENTITY;
        trace->hitId = 1022;
    }
    if ( gjk_get_cbrush(gto.m_gi->m_cg) )
    {
        v3 = gjk_get_cbrush(gto.m_gi->m_cg);
        trace->cflags = v3->contents;
        get_material_from_brush(v3, &trace->sflags);
    }
    else if ( gto.m_gi->m_cg->get_type() == GJK_PARTITION )
    {
        gjk_partition_t *partition = (gjk_partition_t *)gto.m_gi->m_cg;
        materialInfo = &cm.materials[partition->tree->materialIndex];
        trace->sflags = materialInfo->surfaceFlags;
        trace->cflags = materialInfo->contentFlags;
    }
    else
    {
        trace->cflags = 0;
        trace->sflags = 0;
    }
    trace->modelIndex = 0;
    trace->partName = 0;
    trace->partGroup = 0;
    trace->boneIndex = 254;
    trace->staticModel = 0;
}

void __cdecl fill_results_type_and_id(const gjk_trace_output_t *gto, trace_t *trace)
{
    unsigned __int16 index; // [esp+2h] [ebp-6h]
    unsigned __int16 number; // [esp+4h] [ebp-4h]
    unsigned __int16 v4; // [esp+6h] [ebp-2h]

    if ( gto->m_gi->m_ent_info )
    {
        if ( gto->m_gi->m_ent_info->m_ent_type )
        {
            if ( gto->m_gi->m_ent_info->m_ent_type == gjk_entity_info_t::ENTITY_TYPE::ET_CENT )
            {
                //number = gjk_entity_info_t::get_cent(gto->m_gi->m_ent_info)->nextState.number;
                number = gto->m_gi->m_ent_info->get_cent()->nextState.number;
                if ( !trace
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\physics\\../qcommon/cm_public.h",
                                175,
                                0,
                                "%s",
                                "trace") )
                {
                    __debugbreak();
                }
                trace->hitType = TRACE_HITTYPE_ENTITY;
                trace->hitId = number;
            }
            else if ( gto->m_gi->m_ent_info->m_ent_type == gjk_entity_info_t::ENTITY_TYPE::ET_GLASS )
            {
                //index = gjk_entity_info_t::get_glass(gto->m_gi->m_ent_info)->index;
                index = gto->m_gi->m_ent_info->get_glass()->index;
                if ( !trace
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\physics\\../qcommon/cm_public.h",
                                175,
                                0,
                                "%s",
                                "trace") )
                {
                    __debugbreak();
                }
                trace->hitType = TRACE_HITTYPE_GLASS;
                trace->hitId = index;
            }
            else
            {
                if ( gto->m_gi->m_ent_info->m_ent_type != gjk_entity_info_t::ENTITY_TYPE::ET_DENT
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_gjk_collision_detection.cpp",
                                1765,
                                0,
                                "%s",
                                "gto.m_gi->m_ent_info->is_dent()") )
                {
                    __debugbreak();
                }
                if ( !trace
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\physics\\../qcommon/cm_public.h",
                                175,
                                0,
                                "%s",
                                "trace") )
                {
                    __debugbreak();
                }
                trace->hitType = TRACE_HITTYPE_ENTITY;
                trace->hitId = 1022;
            }
        }
        else
        {
            //v4 = gjk_entity_info_t::get_gent(gto->m_gi->m_ent_info)->s.number;
            v4 = gto->m_gi->m_ent_info->get_gent()->s.number;
            if ( !trace
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\physics\\../qcommon/cm_public.h",
                            175,
                            0,
                            "%s",
                            "trace") )
            {
                __debugbreak();
            }
            trace->hitType = TRACE_HITTYPE_ENTITY;
            trace->hitId = v4;
        }
    }
    else
    {
        if ( !trace
            && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\physics\\../qcommon/cm_public.h", 175, 0, "%s", "trace") )
        {
            __debugbreak();
        }
        trace->hitType = TRACE_HITTYPE_ENTITY;
        trace->hitId = 1022;
    }
}

void __cdecl fill_results_no_hit(trace_t *trace)
{
    iassert(trace);

    trace->fraction = 1.0f;
    trace->allsolid = 0;
    trace->startsolid = 0;
    trace->normal.vec.u[0] = 0;
    trace->normal.vec.u[1] = 0;
    trace->normal.vec.u[2] = 0;
    trace->walkable = 0;
    trace->hitType = TRACE_HITTYPE_NONE;
    trace->hitId = 0;
    trace->cflags = 0;
    trace->sflags = 0;
    trace->modelIndex = 0;
    trace->partName = 0;
    trace->partGroup = 0;
    trace->boneIndex = 254;
    trace->staticModel = 0;
}

bool    is_walkable(const gjk_trace_output_t *gto)
{
    phys_vec3 up_loc; // [esp-Ch] [ebp-7Ch] BYREF
    phys_vec3 hit_loc; // [esp+24h] [ebp-4Ch] BYREF
    
    if (!gto->m_is_foot)
        return 0;

    if (gto->m_hit_normal.z >= 0.69999999)
        return true;

    if (gto->m_gi->m_ent_info)
    {
        hit_loc = phys_full_inv_multiply(gto->m_gi->m_ent_info->m_mat, gto->m_hit_point);
        up_loc = phys_inv_multiply(gto->m_gi->m_ent_info->m_mat, PHYS_Z_VEC);
    }
    else
    {
        hit_loc = gto->m_hit_point;
        up_loc = PHYS_Z_VEC;
    }
    return gjk_is_walkable(gto->m_gi->m_cg, &hit_loc, &up_loc);
}

void *gjk_physics_collision_visitor::allocate(
    const int size,
    const int alignment,
    const bool no_error)
{
    //return phys_transient_allocator::mt_allocate(
    return G_BPM->g_collision_memory_buffer.mt_allocate(
        size,
        alignment,
        no_error,
        "gjk terrain query out of memory.");
}