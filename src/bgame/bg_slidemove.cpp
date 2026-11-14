#include "bg_slidemove.h"
#include "bg_pmove.h"
#include <cgame_mp/cg_predict_mp.h>
#include "tl/base/include/tl_system.h"
#include <physics/phys_local.h>
#include <server/sv_world.h>
#include <cgame_mp/cg_vehicles_mp.h>
#include <cgame/cg_drawtools.h>
#include "bg_misc.h"
#include <physics/phys_render.h>
#include <physics/phys_gjk_collision_detection.h>
#include "bg_jump.h"
#include <qcommon/cm_load.h>
#include <qcommon/common.h>

bool do_push; // this supposed to have a value? (KISAKTODO)
bool do_step_down = true;
float WALKABLE_DIST_THRESH = 0.1f;


const float PT_AC_EPS = 0.125f;
phys_vec3 PT_AC_EPS_VEC = { PT_AC_EPS , PT_AC_EPS , PT_AC_EPS };


phys_simple_allocator<gjkcc_info> g_gjkcc_info_allocator;
gjkcc_info_database_t g_gjkcc_info_client_database(0, 0, false);
gjkcc_info_database_t g_gjkcc_info_server_database(0, 0, true);

gjk_polygon_cylinder_t *__cdecl create_gjkcc_gjk_geom(
                float (*mins)[3],
                float (*maxs)[3],
                int stype,
                gjk_collision_visitor *allocator)
{
    gjk_polygon_cylinder_t *gjk_geom; // [esp+Ch] [ebp-4h]

    gjk_geom = gjk_polygon_cylinder_t::create(*mins, *maxs, 0.5, stype, allocator);
    setup_gjk_polygon_cylinder((float *)mins, (float *)maxs, 0.5, gjk_geom);
    return gjk_geom;
}

gjkcc_info *__cdecl create_gjkcc_info(unsigned int gjkcc_id, bool is_server_thread, const float *origin)
{
    gjkcc_info *gcci; // [esp+58h] [ebp-8h]

    //gcci = phys_simple_allocator<gjkcc_info>::allocate(&g_gjkcc_info_allocator);
    gcci = g_gjkcc_info_allocator.allocate();
    if ( gjkcc_id )
        gcci->m_gjk_cache.m_max_num_gjk_ci = 256;
    else
        gcci->m_gjk_cache.m_max_num_gjk_ci = 0;
    gcci->m_gjk_cache.m_list_head = 0;
    gcci->m_active = 0;
    gcci->m_is_server_thread = is_server_thread;
    //phys_mat44::operator=(&gcci->m_cg_to_world_xform, &PHYS_IDENTITY_MATRIX);
    gcci->m_cg_to_world_xform = PHYS_IDENTITY_MATRIX;
    gcci->m_mins[0] = -15.0f;
    gcci->m_mins[1] = -15.0f;
    gcci->m_mins[2] = 0.0f;
    gcci->m_maxs[0] = 15.0f;
    gcci->m_maxs[1] = 15.0f;
    gcci->m_maxs[2] = 70.0f;
    gcci->m_cg_ = gjk_polygon_cylinder_t::create(
                                    gcci->m_mins,
                                    gcci->m_maxs,
                                    0.5,
                                    0,
                                    &g_empty_collision_visitor);
    //gjkcc_info::update_cg(gcci, gcci->m_mins, gcci->m_maxs, 1);
    gcci->update_cg(gcci->m_mins, gcci->m_maxs, true);
    gcci->m_last_origin[0] = *origin;
    gcci->m_last_origin[1] = origin[1];
    gcci->m_last_origin[2] = origin[2];
    return gcci;
}

void gjkcc_info::update_cg(float *mins, float *maxs, bool force)
{
    if (   mins[0] != this->m_mins[0]
        || mins[1] != this->m_mins[1]
        || mins[2] != this->m_mins[2]
        || maxs[0] != this->m_maxs[0]
        || maxs[1] != this->m_maxs[1]
        || maxs[2] != this->m_maxs[2]
        || force )
    {
        this->m_mins[0] = mins[0];
        this->m_mins[1] = mins[1];
        this->m_mins[2] = mins[2];

        this->m_maxs[0] = maxs[0];
        this->m_maxs[1] = maxs[1];
        this->m_maxs[2] = maxs[2];

        if ( this->m_cg_->get_type() == GJK_DOUBLE_SPHERE )
        {
            setup_gjk_capsule(mins, maxs, 0.5, (gjk_double_sphere_t *)this->m_cg_);
        }
        else if ( this->m_cg_->get_type() == GJK_POLYGON_CYLINDER )
        {
            setup_gjk_polygon_cylinder(mins, maxs, 0.5, (gjk_polygon_cylinder_t *)this->m_cg_);
        }
        else if ( this->m_cg_->get_type() == GJK_CYLINDER )
        {
            setup_gjk_cylinder(mins, maxs, 0.5, (gjk_cylinder_t *)this->m_cg_);
        }

        this->m_cg_->calc_aabb(PHYS_IDENTITY_MATRIX, &this->m_cg_aabb_min, &this->m_cg_aabb_max);
        this->m_cg_aabb_min.x = this->m_cg_aabb_min.x + PT_AC_EPS_VEC.x;
        this->m_cg_aabb_min.y = this->m_cg_aabb_min.y + PT_AC_EPS_VEC.y;
        this->m_cg_aabb_min.z = this->m_cg_aabb_min.z + PT_AC_EPS_VEC.z;
        this->m_cg_aabb_max.x = this->m_cg_aabb_max.x - PT_AC_EPS_VEC.x;
        this->m_cg_aabb_max.y = this->m_cg_aabb_max.y - PT_AC_EPS_VEC.y;
        this->m_cg_aabb_max.z = this->m_cg_aabb_max.z - PT_AC_EPS_VEC.z;
    }
}

void __cdecl destroy_gjkcc_info(gjkcc_info *gcci_)
{
    if (!gcci_ && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp", 108, 0, "%s", "gcci_"))
        __debugbreak();
    if (_InterlockedCompareExchange(&gcci_->m_active, 1, 1) == 1
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp",
            110,
            0,
            "%s",
            "gcci->IsLocked() == false"))
    {
        __debugbreak();
    }
    gjk_polygon_cylinder_t::destroy((gjk_polygon_cylinder_t *)gcci_->m_cg_);
    //gjk_query_output::verify_empty(&gcci_->m_gjk_query_output);
    gcci_->m_gjk_query_output.verify_empty();
    //phys_heap_gjk_cache_system_avl_tree::shutdown(&gcci_->m_gjk_cache);
    gcci_->m_gjk_cache.shutdown();
    //phys_simple_allocator<gjkcc_info>::free(&g_gjkcc_info_allocator, gcci_);
    g_gjkcc_info_allocator.free(gcci_);
}

gjkcc_info *__thiscall gjkcc_info_database_t::gjkcc_info_find_or_create(
                unsigned int gjkcc_id,
                bool is_server_thread,
                const float *origin)
{
    generic_avl_map_node_t *m_tree_root; // [esp+8h] [ebp-14h]
    gjkcc_info *data; // [esp+10h] [ebp-Ch]
    generic_avl_map_node_t *gamn; // [esp+14h] [ebp-8h]
    gjkcc_info *gcci; // [esp+18h] [ebp-4h]

    gjkcc_info_database_t::verify_internal(is_server_thread);
    gjkcc_info_database_t::Lock();
    m_tree_root = this->m_map.m_tree_root;
    while ( m_tree_root && gjkcc_id != m_tree_root->m_avl_key )
    {
        if ( gjkcc_id >= m_tree_root->m_avl_key )
            m_tree_root = m_tree_root->m_avl_tree_node.m_right;
        else
            m_tree_root = m_tree_root->m_avl_tree_node.m_left;
    }
    gamn = m_tree_root;
    if ( !m_tree_root )
    {
        data = create_gjkcc_info(gjkcc_id, is_server_thread, origin);
        gamn = generic_avl_map_add(&this->m_map, data, gjkcc_id);
        ++this->m_gcci_count;
    }
    if ( !gamn && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp", 156, 0, "%s", "gamn") )
        __debugbreak();
    gcci = (gjkcc_info *)gamn->m_data;
    if ( !gamn->m_data
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp", 158, 0, "%s", "gcci") )
    {
        __debugbreak();
    }
    if ( _InterlockedCompareExchange(&gcci->m_active, 1, 1) == 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp",
                    159,
                    0,
                    "%s",
                    "gcci->IsLocked() == false") )
    {
        __debugbreak();
    }
    if ( gcci->m_is_server_thread != is_server_thread
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp",
                    160,
                    0,
                    "%s",
                    "gcci->m_is_server_thread == is_server_thread") )
    {
        __debugbreak();
    }
    gjkcc_info_database_t::Unlock();
    return gcci;
}

void __thiscall gjkcc_info_database_t::Lock()
{
    if ( _InterlockedCompareExchange(&this->m_token, 1, 0) )
    {
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp", 135, 0, "%s", "retv") )
            __debugbreak();
    }
}

void __thiscall gjkcc_info_database_t::Unlock()
{
    if ( _InterlockedCompareExchange(&this->m_token, 0, 1) != 1
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp", 136, 0, "%s", "retv") )
    {
        __debugbreak();
    }
}

void __thiscall gjkcc_info_database_t::verify_internal(bool is_server_thread)
{
    if ( this->m_is_server_thread != is_server_thread
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp",
                    137,
                    0,
                    "%s",
                    "m_is_server_thread == is_server_thread") )
    {
        __debugbreak();
    }
}

gjkcc_info *__thiscall gjkcc_info_database_t::gjkcc_info_find(
                unsigned int gjkcc_id,
                bool is_server_thread)
{
    generic_avl_map_node_t *m_tree_root; // [esp+8h] [ebp-10h]
    gjkcc_info *gcci; // [esp+14h] [ebp-4h]

    gjkcc_info_database_t::verify_internal(is_server_thread);
    gjkcc_info_database_t::Lock();
    m_tree_root = this->m_map.m_tree_root;
    while ( m_tree_root && gjkcc_id != m_tree_root->m_avl_key )
    {
        if ( gjkcc_id >= m_tree_root->m_avl_key )
            m_tree_root = m_tree_root->m_avl_tree_node.m_right;
        else
            m_tree_root = m_tree_root->m_avl_tree_node.m_left;
    }
    if ( !m_tree_root
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp", 173, 0, "%s", "gamn") )
    {
        __debugbreak();
    }
    gcci = (gjkcc_info *)m_tree_root->m_data;
    if ( !m_tree_root->m_data
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp", 175, 0, "%s", "gcci") )
    {
        __debugbreak();
    }
    if ( _InterlockedCompareExchange(&gcci->m_active, 1, 1) != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp",
                    176,
                    0,
                    "%s",
                    "gcci->IsLocked() == true") )
    {
        __debugbreak();
    }
    if ( gcci->m_is_server_thread != is_server_thread
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp",
                    177,
                    0,
                    "%s",
                    "gcci->m_is_server_thread == is_server_thread") )
    {
        __debugbreak();
    }
    gjkcc_info_database_t::Unlock();
    return gcci;
}

void __thiscall gjkcc_info_database_t::gjkcc_info_destroy(
                unsigned int gjkcc_id,
                bool is_server_thread)
{
    char *gcci_; // [esp+8h] [ebp-4h]

    gjkcc_info_database_t::verify_internal(is_server_thread);
    gjkcc_info_database_t::Lock();
    gcci_ = (char *)generic_avl_map_destroy(&this->m_map, gjkcc_id);
    if ( gcci_ )
    {
        destroy_gjkcc_info((gjkcc_info *)gcci_);
        --this->m_gcci_count;
    }
    gjkcc_info_database_t::Unlock();
}

void __cdecl destroy_client_gjkcc_info(int localClientNum)
{
    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp",
                    220,
                    0,
                    "%s",
                    "localClientNum >= 0 && localClientNum < MAX_LOCAL_CLIENTS") )
    {
        __debugbreak();
    }
    //gjkcc_info_database_t::gjkcc_info_destroy(&g_gjkcc_info_client_database, (unsigned int)&cg_pmove[localClientNum], 0);
    g_gjkcc_info_client_database.gjkcc_info_destroy((unsigned int)&cg_pmove[localClientNum], 0);

}

void    gjkcc_prolog(const gjkcc_input_t *gjkcc_in, float *origin)
{
    phys_vec3 v4; // [esp-20h] [ebp-2Ch] BYREF
    gjkcc_info *v5; // [esp-4h] [ebp-10h]
    //unsigned int v6[2]; // [esp+0h] [ebp-Ch] BYREF
    //_UNKNOWN *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v6[0] = a1;
    //v6[1] = retaddr;
    if (gjkcc_in->is_server_thread)
    {
        //v5 = gjkcc_info_database_t::gjkcc_info_find_or_create(&g_gjkcc_info_server_database, gjkcc_in->gjkcc_id, 1, origin);
        v5 = g_gjkcc_info_server_database.gjkcc_info_find_or_create(gjkcc_in->gjkcc_id, 1, origin);
    }
    else
    {
        //v5 = gjkcc_info_database_t::gjkcc_info_find_or_create(&g_gjkcc_info_client_database, gjkcc_in->gjkcc_id, 0, origin);
        v5 = g_gjkcc_info_client_database.gjkcc_info_find_or_create(gjkcc_in->gjkcc_id, 0, origin);
    }
    //gjkcc_info::Lock(v5);
    v5->Lock();
    Phys_Vec3ToNitrousVec(origin, v4);
    //gjk_query_output::cached_query_prolog(
        //&v5->m_gjk_query_output,
    v5->m_gjk_query_output.cached_query_prolog(
        gjkcc_in->is_server_thread,
        gjkcc_in->proximity_data,
        gjkcc_in->proximity_mask,
        &v4);
}

void __thiscall gjkcc_info::Lock()
{
    if ( _InterlockedCompareExchange(&this->m_active, 1, 0) )
    {
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp", 78, 0, "%s", "retv") )
            __debugbreak();
    }
}

void __cdecl gjkcc_epilog(const gjkcc_input_t *gjkcc_in, const float *origin)
{
    gjkcc_info *gcci; // [esp+20h] [ebp-4h]

    gcci = find_gjkcc_info(gjkcc_in->gjkcc_id, gjkcc_in->is_server_thread);
    //phys_heap_gjk_cache_system_avl_tree::update_cache(&gcci->m_gjk_cache);
    gcci->m_gjk_cache.update_cache();
    //gjk_query_output::cached_query_epilog(&gcci->m_gjk_query_output);
    gcci->m_gjk_query_output.cached_query_epilog();
    gcci->m_last_origin[0] = *origin;
    gcci->m_last_origin[1] = origin[1];
    gcci->m_last_origin[2] = origin[2];
    //gjkcc_info::Unlock(gcci);
    gcci->Unlock();
}

void __thiscall gjkcc_info::Unlock()
{
    if ( _InterlockedCompareExchange(&this->m_active, 0, 1) != 1
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp", 79, 0, "%s", "retv") )
    {
        __debugbreak();
    }
}

gjkcc_info *__cdecl find_gjkcc_info(unsigned int gjkcc_id, bool is_server_thread)
{
    if (is_server_thread)
    {
        return g_gjkcc_info_server_database.gjkcc_info_find(gjkcc_id, true);
    }
    else
    {
        return g_gjkcc_info_client_database.gjkcc_info_find(gjkcc_id, false);
    }
}

void __cdecl calc_time(phys_gjk_info *gjk_info, bool keep_all_collisions, float *time, float *dist)
{
    float v4; // [esp+20h] [ebp-18h]

    v4 = (float)((float)(gjk_info->m_cg1_relative_translation_loc.x * gjk_info->cg1_cinfo_loc.m_n.x)
                         + (float)(gjk_info->m_cg1_relative_translation_loc.y * gjk_info->cg1_cinfo_loc.m_n.y))
         + (float)(gjk_info->m_cg1_relative_translation_loc.z * gjk_info->cg1_cinfo_loc.m_n.z);
    *dist = (float)((float)(gjk_info->m_continuous_collision_lambda * v4)
                                + (float)((float)((float)((float)(gjk_info->cg1_cinfo_loc.m_p1.x - gjk_info->cg1_cinfo_loc.m_p2.x)
                                                                                * gjk_info->cg1_cinfo_loc.m_n.x)
                                                                + (float)((float)(gjk_info->cg1_cinfo_loc.m_p1.y - gjk_info->cg1_cinfo_loc.m_p2.y)
                                                                                * gjk_info->cg1_cinfo_loc.m_n.y))
                                                + (float)((float)(gjk_info->cg1_cinfo_loc.m_p1.z - gjk_info->cg1_cinfo_loc.m_p2.z)
                                                                * gjk_info->cg1_cinfo_loc.m_n.z)))
                + 0.125;
    if ( v4 >= 0.0 )
    {
        if ( *dist <= (float)(0.5 * 0.125) || keep_all_collisions )
            *time = gjk_info->m_continuous_collision_lambda;
        else
            *time = 1.0f;
    }
    else
    {
        *time = gjk_info->m_continuous_collision_lambda;
    }
}

static void POLYGON_CYLINDER_HACK(phys_gjk_input *pgi, const gjk_base_t *cg1, const gjk_base_t *cg2)
{
    if ( cg1->get_type() == GJK_POLYGON_CYLINDER ) // CYLINDER
    {
        gjk_polygon_cylinder_t *cylinder1 = (gjk_polygon_cylinder_t *)cg1;

        if (cg2->get_brush())
            cylinder1->m_mode = 0;
        else
            cylinder1->m_mode = 1;
        pgi->cg1_radius = cylinder1->get_geom_radius();
    }
}

bool gjk_collide(
                phys_gjk_info *gjk_info,
                phys_gjk_input *pgi,
                gjk_trace_output_t *gto,
                const gjk_trace_input_t &gti,
                gjk_geom_info_t *gi)
{
    float hit_time; // [esp+150h] [ebp-1Ch] BYREF
    float hit_dist; // [esp+154h] [ebp-18h] BYREF

    POLYGON_CYLINDER_HACK(pgi, gti.m_cg, gi->m_cg); // de-const cast

    if (!gjk_info->phys_collide_do_gjk_collide(pgi))
    {
        return false;
    }

    calc_time(gjk_info, gti.m_keep_all_collisions, &hit_time, &hit_dist);

    if (pgi->m_end_time <= hit_time && hit_dist >= 0.0)
    {
        return false;
    }

    iassert(gto);

    gto->m_arm.x = gjk_info->cg1_cinfo_loc.m_p1.x;
    gto->m_arm.y = gjk_info->cg1_cinfo_loc.m_p1.y;
    gto->m_arm.z = gjk_info->cg1_cinfo_loc.m_p1.z;

    gto->m_hit_point.x = gjk_info->cg1_cinfo_loc.m_p2.x + gti.m_gcci->m_cg_to_world_xform.w.x;
    gto->m_hit_point.y = gjk_info->cg1_cinfo_loc.m_p2.y + gti.m_gcci->m_cg_to_world_xform.w.y;
    gto->m_hit_point.z = gjk_info->cg1_cinfo_loc.m_p2.z + gti.m_gcci->m_cg_to_world_xform.w.z;

    gto->m_hit_normal.x = gjk_info->cg1_cinfo_loc.m_n.x;
    gto->m_hit_normal.y = gjk_info->cg1_cinfo_loc.m_n.y;
    gto->m_hit_normal.z = gjk_info->cg1_cinfo_loc.m_n.z;

    gto->m_hit_time = hit_time;
    gto->m_hit_dist = hit_dist;

    if (hit_dist < 0.0 && (hit_dist > 0.0 || hit_time != 0.0))
    {
        char buf[256]; // [esp+18h] [ebp-154h] BYREF
        sprintf(buf, "bad hit_time, hit_dist:%f, %f\n", hit_time, hit_dist);
        tlWarning(buf);
        Com_PrintError(1, buf);
    }

    gto->m_is_foot = gjk_is_foot(gti.m_cg, &gto->m_arm);
    gto->m_gi = gi;

    return true;
}

void __cdecl sort_gi_list(gjk_geom_info_t **list, int list_count)
{
    gjk_geom_info_t *v2; // [esp+4h] [ebp-28h]
    gjk_geom_info_t *v3; // [esp+8h] [ebp-24h]
    gjk_geom_info_t *v4; // [esp+10h] [ebp-1Ch]
    gjk_geom_info_t **cur; // [esp+14h] [ebp-18h]
    gjk_geom_info_t **prev_cur; // [esp+18h] [ebp-14h]
    gjk_geom_info_t **middle; // [esp+1Ch] [ebp-10h]
    gjk_geom_info_t **prev_middle; // [esp+20h] [ebp-Ch]

    if ( list_count >= 3 )
    {
        sort_gi_list(list, list_count / 2);
        sort_gi_list(&list[list_count / 2], list_count - list_count / 2);
        middle = &list[list_count / 2];
        for ( prev_middle = middle - 1;
                    middle < &list[list_count] && (*prev_middle)->m_hit_time > (*middle)->m_hit_time;
                    prev_middle = middle++ )
        {
            v3 = *prev_middle;
            *prev_middle = *middle;
            *middle = v3;
            cur = prev_middle;
            for ( prev_cur = prev_middle - 1; cur > list && (*prev_cur)->m_hit_time > (*cur)->m_hit_time; --prev_cur )
            {
                v2 = *prev_cur;
                *prev_cur = *cur;
                *cur = v2;
                cur = prev_cur;
            }
        }
    }
    else if ( list_count == 2 && (*list)->m_hit_time > list[1]->m_hit_time )
    {
        v4 = *list;
        *list = list[1];
        list[1] = v4;
    }
}

void    gjk_trace(const gjk_trace_input_t &gti, list_gjk_trace_output *list)
{
    gjk_geom_info_t *gi; // [esp-Ch] [ebp-4A0h]
    gjk_trace_output_t *gto; // [esp-4h] [ebp-498h]
    gjk_geom_info_t **gi_list; // [esp+44h] [ebp-450h]
    phys_gjk_input input; // [esp+58h] [ebp-43Ch] BYREF
    gjkcc_info *m_gcci; // [esp+480h] [ebp-14h]

    PROF_SCOPED("gjk_trace_query");

    iassert(gti.m_query_input.m_cg_position.GetX() == get_mat_wrow(gti.get_cg_mat()).GetX());
    iassert(gti.m_query_input.m_cg_position.GetY() == get_mat_wrow(gti.get_cg_mat()).GetY());
    iassert(gti.m_query_input.m_cg_position.GetZ() == get_mat_wrow(gti.get_cg_mat()).GetZ());

    gti.m_query_output->query_prolog(&gti.m_query_input);
    gjk_query_cached(gti.m_query_input, gti.m_query_output);
    gti.m_query_output->query_epilog();

    //phys_gjk_info::phys_gjk_info(&info);
    phys_gjk_info info; // [esp+B8h] [ebp-3DCh] BYREF

    list->m_list.remove_all();

    list->m_first_hit = 0;
    if (gti.m_query_output->m_list_geom_info.m_alloc_count)
    {
        init_pgi(&input, gti);

        gi_list = (gjk_geom_info_t **)gti.m_allocator->allocate(sizeof(gjk_geom_info_t *) * gti.m_query_output->m_list_geom_info.m_alloc_count, 4, 0, "phys_transient_allocator out of memory.");

        int gi_i = 0;
        for (gjk_geom_info_t *i = gti.m_query_output->m_list_geom_info.m_first; i; i = i->m_next_link)
            gi_list[gi_i++] = i;
        sort_gi_list(gi_list, gi_i);

        PROF_SCOPED("gjk_trace_collide");

        gto = (gjk_trace_output_t *)gti.m_allocator->allocate(sizeof(gjk_trace_output_t), 16, 0, "phys_transient_allocator out of memory.");
        for (int j = 0; j < gi_i && input.m_end_time >= gi_list[j]->m_hit_time; ++j)
        {
            gi = gi_list[j];
            input.m_start_time = gi->m_hit_time;
            set_pgi_cg2(&input, gti, gi);
            if (gjk_collide(&info, &input, gto, gti, gi))
            {
                //phys_link_list<gjk_trace_output_t>::add(&list->m_list, v8);
                list->m_list.add(gto);
                if (input.m_end_time > gto->m_hit_time)
                {
                    if ((float)(gto->m_hit_time + gti.m_extra_time) <= 1.0)
                    {
                        input.m_end_time = gto->m_hit_time + gti.m_extra_time;
                    }
                    else
                    {
                        input.m_end_time = 1.0f;
                    }
                }
                if (list->m_first_hit)
                {
                    if (list->m_first_hit->m_hit_time <= gto->m_hit_time)
                    {
                        if (gto->m_hit_time == 0.0 && list->m_first_hit->m_hit_dist > gto->m_hit_dist)
                        {
                            iassert(list->m_first_hit->m_hit_time == 0.0f);
                            list->m_first_hit = gto;
                        }
                    }
                    else
                    {
                        list->m_first_hit = gto;
                    }
                }
                else
                {
                    list->m_first_hit = gto;
                }

                gto = (gjk_trace_output_t *)gti.m_allocator->allocate(sizeof(gjk_trace_output_t), 16, 0, "phys_transient_allocator out of memory.");

                if (gti.m_exit_on_penetration && list->m_first_hit->m_hit_dist < 0.0)
                    break;
            }
        }
    }
}

void __cdecl init_pgi(phys_gjk_input *pgi, const gjk_trace_input_t &gti)
{
    pgi->m_cg1_translation.x = gti.m_query_input.m_cg_translation.x;
    pgi->m_cg1_translation.y = gti.m_query_input.m_cg_translation.y;
    pgi->m_cg1_translation.z = gti.m_query_input.m_cg_translation.z;
    pgi->m_cg2_translation.x = PHYS_ZERO_VEC.x;
    pgi->m_cg2_translation.y = PHYS_ZERO_VEC.y;
    pgi->m_cg2_translation.z = PHYS_ZERO_VEC.z;
    pgi->m_start_time = 0.0f;
    pgi->m_end_time = 1.0f;
    pgi->gjk_cg1 = gti.m_cg;
    pgi->gjk_cg2 = 0;
    pgi->cg1_to_world_xform = &gti.m_gcci->m_cg_to_world_xform;
    pgi->cg2_to_world_xform = 0;
    pgi->cg1_radius = gti.m_cg->get_geom_radius();
    pgi->cg2_radius = 0.0f;
    pgi->gjk_ci = 0;
    pgi->m_sep_thresh = gti.m_gjk_ac_eps;
    pgi->m_intersection_test_only = 0;
    pgi->m_continuous_collision = 1;
}

void __cdecl set_pgi_cg2(phys_gjk_input *pgi, const gjk_trace_input_t &gti, gjk_geom_info_t *gi)
{
    pgi->gjk_cg2 = gi->m_cg;
    pgi->cg2_to_world_xform = &gi->get_xform()->m_mat;
    pgi->cg2_radius = gi->m_cg->get_geom_radius();
    pgi->gjk_ci = get_gjk_cache_info(gti.m_gjk_cache, gti.m_cg, gi->m_cg);
}

gjk_entity_info_t *__thiscall gjk_geom_info_t::get_xform()
{
    if ((this->m_cg->m_flags & 8) != 0)
    {
        //return (gjk_entity_info_t *)gjk_base_t::get_xform(this->m_cg);
        return (gjk_entity_info_t*)this->m_cg->get_xform();
    }
    if ( this->m_ent_info )
        return this->m_ent_info;
    else
        return (gjk_entity_info_t *)&PHYS_IDENTITY_MATRIX;
}

void __cdecl setup_query_input(
                const gjkcc_input_t *gjkcc_in,
                const phys_vec3 *aabb_min,
                const phys_vec3 *aabb_max,
                const phys_vec3 *position,
                const phys_vec3 *translation,
                int passEntityNum,
                int contentMask,
                gjk_query_input *query_input)
{
    IgnoreEntParams ignoreEntParams; // [esp+14h] [ebp-Ch] BYREF

    query_input->m_cg_aabb_min.x = aabb_min->x;
    query_input->m_cg_aabb_min.y = aabb_min->y;
    query_input->m_cg_aabb_min.z = aabb_min->z;
    query_input->m_cg_aabb_max.x = aabb_max->x;
    query_input->m_cg_aabb_max.y = aabb_max->y;
    query_input->m_cg_aabb_max.z = aabb_max->z;
    query_input->m_cg_position.x = position->x;
    query_input->m_cg_position.y = position->y;
    query_input->m_cg_position.z = position->z;
    query_input->m_cg_translation.x = translation->x;
    query_input->m_cg_translation.y = translation->y;
    query_input->m_cg_translation.z = translation->z;
    query_input->m_ac_eps_vec.x = PT_AC_EPS_VEC.x;
    query_input->m_ac_eps_vec.y = PT_AC_EPS_VEC.y;
    query_input->m_ac_eps_vec.z = PT_AC_EPS_VEC.z;
    query_input->m_contents = contentMask;
    if ( gjkcc_in->is_server_thread )
    {
        SV_SetupIgnoreEntParams(&ignoreEntParams, passEntityNum);
        query_input->m_pass_entity_num = ignoreEntParams.baseEntity;
        query_input->m_pass_owner_num = ignoreEntParams.parentEntity;
    }
    else
    {
        query_input->m_pass_entity_num = passEntityNum;
        query_input->m_pass_owner_num = -1;
    }
    query_input->m_is_server_thread = gjkcc_in->is_server_thread;
    query_input->m_proximity_data = gjkcc_in->proximity_data;
    query_input->m_proximity_mask = gjkcc_in->proximity_mask;
    query_input->m_gjk_query_flags = gjkcc_in->m_gjk_query_flags;

    query_input->m_geom_skip_list.remove_all();
}

void    setup_trace_info(
                const gjkcc_input_t *gjkcc_in,
                float *start,
                float *mins,
                float *maxs,
                float *end,
                int passEntityNum,
                int contentMask,
                gjk_trace_input_t *gti)
{
    gjk_query_input *query_input; // [esp+0h] [ebp-58h]
    phys_vec3 translation; // [esp+8h] [ebp-50h]
    phys_vec3 pv_end; // [esp+1Ch] [ebp-3Ch] BYREF
    phys_vec3 pv_start; // [esp+2Ch] [ebp-2Ch] BYREF
    gjkcc_info *gcci; // [esp+48h] [ebp-10h]

    gcci = find_gjkcc_info(gjkcc_in->gjkcc_id, gjkcc_in->is_server_thread);
    gti->m_gcci = gcci;
    gti->m_gcci->update_cg(mins, maxs, false);
    gti->m_cg = gcci->m_cg_;
    gti->m_gjk_cache = &gcci->m_gjk_cache;
    gti->m_query_output = &gcci->m_gjk_query_output;

    Phys_Vec3ToNitrousVec(start, pv_start);
    Phys_Vec3ToNitrousVec(end, pv_end);

    translation.x = pv_end.x - pv_start.x;
    translation.y = pv_end.y - pv_start.y;
    translation.z = pv_end.z - pv_start.z;

    gti->set_cg_position(&pv_start);
    gti->m_query_input.m_cg_translation.x = translation.x;
    gti->m_query_input.m_cg_translation.y = translation.y;
    gti->m_query_input.m_cg_translation.z = translation.z;

    gti->m_gjk_ac_eps = PT_AC_EPS;
    gti->m_keep_all_collisions = 0;
    gti->m_exit_on_penetration = 1;
    gti->m_extra_time = 0.0f;
    gti->m_allocator = &gti->m_query_output->m_allocator;

    setup_query_input(
        gjkcc_in,
        &gcci->m_cg_aabb_min,
        &gcci->m_cg_aabb_max,
        &pv_start,
        &translation,
        passEntityNum,
        contentMask,
        &gti->m_query_input);

    if (!gjkcc_in->gjkcc_id)
    {
        if (gjkcc_in->m_gjk_cg)
        {
            gti->m_cg = gjkcc_in->m_gjk_cg;
            gti->m_gcci->m_cg_to_world_xform = *gjkcc_in->m_mat;
            gti->set_cg_position(&pv_start);
            gti->m_cg->calc_aabb(
                PHYS_IDENTITY_MATRIX,
                &gti->m_query_input.m_cg_aabb_min,
                &gti->m_query_input.m_cg_aabb_max);
        }
        else
        {
            gti->m_gcci->m_cg_to_world_xform = PHYS_IDENTITY_MATRIX;
            gti->set_cg_position(&pv_start);
        }
    }
}

void __thiscall gjk_trace_input_t::set_cg_position(const phys_vec3 *position)
{
    phys_vec3 *p_w; // [esp+Ch] [ebp-4h]

    p_w = &this->m_gcci->m_cg_to_world_xform.w;
    p_w->x = position->x;
    p_w->y = position->y;
    p_w->z = position->z;
    this->m_query_input.m_cg_position.x = position->x;
    this->m_query_input.m_cg_position.y = position->y;
    this->m_query_input.m_cg_position.z = position->z;
}

void    gjk_player_trace(
                const gjkcc_input_t *gjkcc_in,
                trace_t *results,
                float *start,
                float *mins,
                float *maxs,
                float *end,
                int passEntityNum,
                int contentMask)
{
    gjk_trace_output_t *gto; // [esp-Ch] [ebp-D8h]
    list_gjk_trace_output traceOut; // [esp-8h] [ebp-D4h] BYREF
    gjk_trace_input_t gti; // [esp+10h] [ebp-BCh] BYREF

    setup_trace_info(gjkcc_in, start, mins, maxs, end, passEntityNum, contentMask, &gti);
    gjk_trace(gti, &traceOut);

    if (traceOut.m_first_hit && traceOut.m_first_hit->m_hit_dist >= 0.0)
    {
        for (gto = traceOut.m_list.m_first; gto; gto = gto->m_next_link)
        {
            iassert(gto->m_hit_dist >= 0.0f);
        }
    }
    if (traceOut.m_first_hit)
    {
        fill_results(*traceOut.m_first_hit, is_walkable(traceOut.m_first_hit), results);
    }
    else
    {
        fill_results_no_hit(results);
    }
}

char __cdecl no_push_out(const gjk_trace_output_t *gto)
{
    const centity_s *cent; // eax
    const gentity_s *gent; // eax
    const vehicle_info_t *info; // [esp+0h] [ebp-8h]
    int etype; // [esp+4h] [ebp-4h]

    if ( !gto->m_gi->m_ent_info )
        return 0;
    switch ( gto->m_gi->m_ent_info->m_ent_type )
    {
        case gjk_entity_info_t::ENTITY_TYPE::ET_CENT:
            //etype = gjk_entity_info_t::get_cent(gto->m_gi->m_ent_info)->nextState.eType;
            etype = gto->m_gi->m_ent_info->get_cent()->nextState.eType;
LABEL_12:
            if ( etype == 1 || etype == 17 )
                return 1;
            if ( etype == 14 )
            {
                if ( gto->m_gi->m_ent_info->m_ent_type == gjk_entity_info_t::ENTITY_TYPE::ET_CENT )
                {
                    //cent = gjk_entity_info_t::get_cent(gto->m_gi->m_ent_info);
                    cent = gto->m_gi->m_ent_info->get_cent();
                    info = CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
                }
                else
                {
                    if ( gto->m_gi->m_ent_info->m_ent_type
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp",
                                    987,
                                    0,
                                    "%s",
                                    "gto->m_gi->m_ent_info->is_gent()") )
                    {
                        __debugbreak();
                    }
                    //gent = gjk_entity_info_t::get_gent(gto->m_gi->m_ent_info);
                    gent = gto->m_gi->m_ent_info->get_gent();
                    info = CG_GetVehicleInfo(gent->s.vehicleState.vehicleInfoIndex);
                }
                if ( info && info->remoteControl )
                    return 1;
            }
            return 0;
        case gjk_entity_info_t::ENTITY_TYPE::ET_GENT:
            //etype = gjk_entity_info_t::get_gent(gto->m_gi->m_ent_info)->s.eType;
            etype = gto->m_gi->m_ent_info->get_gent()->s.eType;
            goto LABEL_12;
        case gjk_entity_info_t::ENTITY_TYPE::ET_GLASS:
            return 0;
    }
    if ( gto->m_gi->m_ent_info->m_ent_type != gjk_entity_info_t::ENTITY_TYPE::ET_DENT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp",
                    971,
                    0,
                    "%s",
                    "gto->m_gi->m_ent_info->is_dent()") )
    {
        __debugbreak();
    }
    return 0;
}

void __cdecl set_hit_info(geom_plane *gp, const gjk_trace_output_t *gto)
{
    gp->m_normal.x = gto->m_hit_normal.x;
    gp->m_normal.y = gto->m_hit_normal.y;
    gp->m_normal.z = gto->m_hit_normal.z;
    gp->m_arm.x = gto->m_arm.x;
    gp->m_arm.y = gto->m_arm.y;
    gp->m_arm.z = gto->m_arm.z;
    gp->m_d = (gp->m_normal.x * gto->m_hit_point.x) + (gp->m_normal.y * gto->m_hit_point.y) + (gp->m_normal.z * gto->m_hit_point.z);
    gp->m_active = 1;
    gp->m_walkable = is_walkable(gto);
    gp->m_no_push_out = no_push_out(gto);
}

void __cdecl add_hit_info(
                const gjk_trace_output_t *gto,
                phys_static_array<geom_plane,128> *list_geom_plane,
                phys_link_list<gjk_geom_info_t> *geom_skip_list)
{
    set_hit_info(list_geom_plane->add(0, "phys array add overflow."), gto);
    geom_skip_list->add(gto->m_gi);
}

bool __cdecl project_succeeded(
                phys_static_array<geom_plane,128> *list_geom_plane,
                const phys_vec3 *new_position,
                float PROJECT_FAIL_THRESH)
{
    geom_plane *gp_i; // [esp+14h] [ebp-4h]

    for ( int i = 0; i < list_geom_plane->get_count(); ++i )
    {
        gp_i = &(*list_geom_plane)[i];
        if ((-(PROJECT_FAIL_THRESH)) >
            (float)((float)((float)((float)(new_position->x * gp_i->m_normal.x)
                + (float)(new_position->y * gp_i->m_normal.y))
                + (float)(new_position->z * gp_i->m_normal.z)) - gp_i->m_right_side)
            )
        {
            return 0;
        }
    }
    return 1;
}

bool gjk_push_out(
                const gjkcc_input_t *gjkcc_in,
                gjk_slide_move_input_t *input,
                gjk_slide_move_output_t *output)
{
    // aislop hybrid
    // 
    // --- Stack-allocated geometry plane array ---
    phys_static_array<geom_plane, 128> list_geom_plane;

    // --- Store geom_info pointers parallel to plane slots ---
    // slot 128 is used as a temp to snapshot m_alloc_count before each add
    gjk_geom_info_t *gi_list[129];

    // --- Convert input position to physics vector ---
    // cur_pos = the goal/target we're trying to reach (stays fixed as goal)
    // pos     = current solved position (iteratively updated)
    phys_vec3 cur_pos, pos;
    Phys_Vec3ToNitrousVec((const float *)input->position, cur_pos);
    pos = cur_pos;

    // --- Set up trace (start == end == position, zero translation initially) ---
    gjk_trace_input_t gti;

    setup_trace_info(
        gjkcc_in,
        (float *)input->position,
        (float *)input->mins,
        (float *)input->maxs,
        (float *)input->position, // end == start (zero translation)
        input->clientnum,
        input->tracemask,
        &gti);
    gti.m_keep_all_collisions = 1;
    gti.m_exit_on_penetration = 0;
    gti.m_extra_time = 1.0f;

    // --- Trace output list ---
    list_gjk_trace_output traceOut;

    const int MAX_OUTER_BUMPS = 5;
    int bumpcount = 0;

    // =========================================================
    // Outer bump loop — retry up to 5 times if push converges
    // =========================================================
    for (;;)
    {
        if (bumpcount >= MAX_OUTER_BUMPS)
            goto write_output; // exhausted retries, accept current pos

        // Set current solved position as query origin,
        // translation = goal - current (try to move toward goal)
        gti.m_gcci->m_cg_to_world_xform.w = pos;
        gti.m_query_input.m_cg_position = pos;

        phys_vec3 translation;
        translation.x = cur_pos.x - pos.x;
        translation.y = cur_pos.y - pos.y;
        translation.z = cur_pos.z - pos.z;
        gti.m_query_input.m_cg_translation = translation;

        gjk_trace(gti, &traceOut);

        if (!traceOut.m_first_hit)
        {
            // No collision — we can reach the goal, advance pos to goal
            pos.x = pos.x + gti.m_query_input.m_cg_translation.x;
            pos.y = pos.y + gti.m_query_input.m_cg_translation.y;
            pos.z = pos.z + gti.m_query_input.m_cg_translation.z;
            goto write_output;
        }

        // --- Collect all hit planes ---
        int plane_count = list_geom_plane.get_count();
        for (gjk_trace_output_t *gto = traceOut.m_list.m_first; gto; gto = gto->m_next_link)
        {
            gi_list[128] = (gjk_geom_info_t *)(uintptr_t)list_geom_plane.get_count(); // snapshot count
            gi_list[list_geom_plane.get_count()] = gto->m_gi;
            add_hit_info(gto, &list_geom_plane, &gti.m_query_input.m_geom_skip_list);
            input->custom_process(gto);
        }

        iassert(list_geom_plane.get_count() > plane_count);

        // --- Compute right-hand sides at current pos ---
        for (int pi = 0; pi < list_geom_plane.get_count(); ++pi)
        {
            geom_plane *gp = &list_geom_plane[pi];
            gp->m_lambda = 0.0f;
            phys_vec3 contact_pt;
            contact_pt.x = pos.x + gp->m_arm.x;
            contact_pt.y = pos.y + gp->m_arm.y;
            contact_pt.z = pos.z + gp->m_arm.z;
            gp->m_right_side = gp->m_d
                - (contact_pt.x * gp->m_normal.x
                    + contact_pt.y * gp->m_normal.y
                    + contact_pt.z * gp->m_normal.z);
            if (gp->m_no_push_out && gp->m_right_side > 0.0f)
                gp->m_right_side = 0.0f;
        }

        // --- Project zero vector through planes to get push-out offset ---
        phys_vec3 push_vec;
        project(&PHYS_ZERO_VEC, &list_geom_plane, &push_vec);

        if (!project_succeeded(&list_geom_plane, &push_vec, 5.0f))
        {
            // Projection failed — cannot push out
            return false;
        }

        // --- Check if push is negligibly small ---
        float push_mag = Abs(push_vec);
        if (push_mag <= 0.001f)
        {
            // Already at rest, no meaningful push needed
            iassert(bumpcount == 0);
            goto write_output;
        }

        // --- Apply push to get new candidate position ---
        cur_pos.x = pos.x + push_vec.x;
        cur_pos.y = pos.y + push_vec.y;
        cur_pos.z = pos.z + push_vec.z;

        // --- Iterative GJK refinement loop (up to 5 inner iterations) ---
        {
            phys_gjk_info gjk_info;
            phys_gjk_input pgi;

            // Zero out translation for static overlap query
            gti.m_query_input.m_cg_translation.x = PHYS_ZERO_VEC.x;
            gti.m_query_input.m_cg_translation.y = PHYS_ZERO_VEC.y;
            gti.m_query_input.m_cg_translation.z = PHYS_ZERO_VEC.z;
            init_pgi(&pgi, gti);
            pgi.m_sep_thresh = push_mag + 1000000000.0f;

            const int MAX_INNER_ITERS = 5;
            const float CONVERGENCE_THRESH_SQ = 0.1f * 0.1f;

            // Inner refinement loop. Three exit paths (matches ASM at gjk_push_out):
            //   - max iters reached      -> ++bumpcount; continue outer  (retry)
            //   - convergence reached    -> ++bumpcount; continue outer  (retry to verify)
            //   - project_succeeded fail -> return false                 (give up)
            bool projection_failed = false;
            for (int iter = 0; iter < MAX_INNER_ITERS; ++iter)
            {
                // Move query position to current candidate
                gti.set_cg_position(&cur_pos);

                // Re-run GJK against each stored geom to update plane hit info
                gjk_trace_output_t gto_temp;
                for (int pi = 0; pi < list_geom_plane.get_count(); ++pi)
                {
                    geom_plane *gp = &list_geom_plane[pi];
                    gjk_geom_info_t *gi = gi_list[pi];

                    set_pgi_cg2(&pgi, gti, gi);

                    iassert(pgi.m_cg1_translation.GetX() == 0.0f
                         && pgi.m_cg1_translation.GetY() == 0.0f
                         && pgi.m_cg1_translation.GetZ() == 0.0f);

                    if (!gjk_collide(&gjk_info, &pgi, &gto_temp, gti, gi))
                    {
                        iassert(0); // "retv"
                    }

                    set_hit_info(gp, &gto_temp);
                }

                // Recompute right-hand sides at original pos with updated hit info
                for (int pi = 0; pi < list_geom_plane.get_count(); ++pi)
                {
                    geom_plane *gp = &list_geom_plane[pi];
                    gp->m_lambda = 0.0f;
                    phys_vec3 contact_pt;
                    contact_pt.x = pos.x + gp->m_arm.x;
                    contact_pt.y = pos.y + gp->m_arm.y;
                    contact_pt.z = pos.z + gp->m_arm.z;
                    gp->m_right_side = gp->m_d
                        - (contact_pt.x * gp->m_normal.x
                            + contact_pt.y * gp->m_normal.y
                            + contact_pt.z * gp->m_normal.z);
                    if (gp->m_no_push_out && gp->m_right_side > 0.0f)
                        gp->m_right_side = 0.0f;
                }

                // Save old push vec for convergence check
                phys_vec3 old_push_vec = push_vec;

                // Re-project to get refined push
                project(&PHYS_ZERO_VEC, &list_geom_plane, &push_vec);

                if (!project_succeeded(&list_geom_plane, &push_vec, 5.0f))
                {
                    projection_failed = true;
                    break;
                }

                // Update candidate position
                cur_pos.x = pos.x + push_vec.x;
                cur_pos.y = pos.y + push_vec.y;
                cur_pos.z = pos.z + push_vec.z;
                push_mag = Abs(push_vec);

                // Convergence: if push vector moved less than threshold, refinement done.
                // Falls through to outer retry (matches ASM goto LABEL_53).
                float delta_x = push_vec.x - old_push_vec.x;
                float delta_y = push_vec.y - old_push_vec.y;
                float delta_z = push_vec.z - old_push_vec.z;
                float delta_sq = delta_x * delta_x + delta_y * delta_y + delta_z * delta_z;
                if (CONVERGENCE_THRESH_SQ >= delta_sq)
                    break;
            }

            // Only the project_succeeded-failed path is fatal
            if (projection_failed)
                return false;

            // Convergence or max-iters: retry outer bump (re-trace from new pos)
            ++bumpcount;
            continue;
        }
    }

write_output:
    Phys_NitrousVecToVec3(cur_pos, output->new_position);
    // Velocity is unchanged — copy directly from input
    output->new_velocity[0] = ((const float *)input->velocity)[0];
    output->new_velocity[1] = ((const float *)input->velocity)[1];
    output->new_velocity[2] = ((const float *)input->velocity)[2];

    return true;
}

bool    gjk_slide_move1(
                const gjkcc_input_t *gjkcc_in,
                gjk_slide_move_input_t *input,
                const bool no_push_out,
                gjk_slide_move_output_t *output,
                bool *needs_push_out)
{
    phys_vec3 pos;
    phys_vec3 vel;
    phys_vec3 vel_end;
    phys_vec3 start_pos;
    phys_vec3 target_pos;
    phys_vec3 remaining_disp;
    phys_vec3 cur_translation;
    phys_vec3 vel_translation;
    phys_vec3 full_disp;
    phys_vec3 half_disp;

    float    end[3];
    float    half_dt;
    float    frametime;
    float    init_dist_sq;
    float    max_disp_sq;
    float    elapsed;
    float    remaining_time;
    float    hit_real_time;
    float    advanced_time;
    int      plane_count;
    int      max_bumps;

    phys_static_array<geom_plane, 128> list_geom_plane;

    const float MIN_SUBSTEP = 0.016666668f;

    // ----------------------------------------------------------------
    // Setup: convert input pos/vel, apply gravity to vel_end
    // ----------------------------------------------------------------
    Phys_Vec3ToNitrousVec((const float *)input->position, pos);
    Phys_Vec3ToNitrousVec((const float *)input->velocity, vel);

    vel_end = vel;

    if (input->has_gravity)
    {
        vel_end.z -= (float)input->gravity * input->frametime;
    }

    gjk_trace_input_t gti;

    // target_pos = pos + 0.5 * (vel + vel_end) * dt   (trapezoidal integration)
    half_dt = input->frametime * 0.5f;
    half_disp.x = half_dt * (vel.x + vel_end.x);
    half_disp.y = half_dt * (vel.y + vel_end.y);
    half_disp.z = half_dt * (vel.z + vel_end.z);

    target_pos.x = pos.x + half_disp.x;
    target_pos.y = pos.y + half_disp.y;
    target_pos.z = pos.z + half_disp.z;

    Phys_NitrousVecToVec3(target_pos, end);
    setup_trace_info(
        gjkcc_in,
        (float *)input->position,
        (float *)input->mins,
        (float *)input->maxs,
        end,
        input->clientnum,
        input->tracemask,
        &gti);
    gti.m_exit_on_penetration = 0;

    remaining_disp.x = target_pos.x - pos.x;
    remaining_disp.y = target_pos.y - pos.y;
    remaining_disp.z = target_pos.z - pos.z;

    frametime = input->frametime;
    full_disp.x = frametime * vel_end.x;
    full_disp.y = frametime * vel_end.y;
    full_disp.z = frametime * vel_end.z;

    cur_translation = remaining_disp;
    vel_translation = full_disp;

    list_gjk_trace_output traceOut;
    *needs_push_out = 0;

    // max_disp_sq = max( (4*|disp|)^2 , 20^2 )
    init_dist_sq = (4.0f * 4.0f) * ((remaining_disp.x * remaining_disp.x)
                                  + (remaining_disp.y * remaining_disp.y)
                                  + (remaining_disp.z * remaining_disp.z));
    if (init_dist_sq <= (20.0f * 20.0f))
        max_disp_sq = (20.0f * 20.0f);
    else
        max_disp_sq = init_dist_sq;

    start_pos = pos;
    elapsed = 0.0f;
    max_bumps = 128 - list_geom_plane.get_count();

    if (g_bDebugRenderPlayerCollision->current.enabled)
    {
        Sys_EnterCriticalSection(CRTISECT_DEBUG_BRUSHES_AND_PATCHES);
        clear_debug_brushes_and_patches();
    }

    // ----------------------------------------------------------------
    // Bump loop: trace -> collect planes -> project remaining motion
    // ----------------------------------------------------------------
    for (int bump = 0; bump < max_bumps; ++bump)
    {
        gti.m_gcci->m_cg_to_world_xform.w = pos;
        gti.m_query_input.m_cg_position = pos;
        gti.m_query_input.m_cg_translation = cur_translation;

        remaining_time = input->frametime - elapsed;

        if (MIN_SUBSTEP <= remaining_time)
            gti.m_extra_time = MIN_SUBSTEP / remaining_time;
        else
            gti.m_extra_time = 1.0f;

        gjk_trace(gti, &traceOut);

        if (!traceOut.m_first_hit)
        {
            // Clean step - advance to end of cg_translation, done
            pos.x += gti.m_query_input.m_cg_translation.x;
            pos.y += gti.m_query_input.m_cg_translation.y;
            pos.z += gti.m_query_input.m_cg_translation.z;
            break;
        }

        //if (g_bDebugRenderPlayerCollision->current.enabled)
        //{
        //    for (gjk_trace_output_t *gto = traceOut.m_list.m_first; gto; gto = gto->m_next_link)
        //    {
        //        if (gto->m_gi->m_cg->get_brush())
        //        {
        //            const cbrush_t *brush = gto->m_gi->m_cg->get_brush();
        //            gjk_entity_info_t *ei = gto->m_gi->m_ent_info ? gto->m_gi->m_ent_info : 0;
        //            add_debug_brush(brush, &ei->m_mat);
        //        }
        //        else if (gto->m_gi->m_cg->get_type() == 3)
        //        {
        //            gjk_base_t *cg = gto->m_gi->m_cg;
        //            add_debug_patch((const CollisionAabbTree *)LODWORD(cg[1].m_aabb_mn_loc.x));
        //        }
        //    }
        //}

        // ---- Advance pos to first-hit time, scale remaining_disp ----
        hit_real_time = traceOut.m_first_hit->m_hit_time * remaining_time;
        advanced_time = 0.0f;
        if (hit_real_time > MIN_SUBSTEP)
        {
            const float ht = traceOut.m_first_hit->m_hit_time;
            pos.x += (ht * gti.m_query_input.m_cg_translation.x);
            pos.y += (ht * gti.m_query_input.m_cg_translation.y);
            pos.z += (ht * gti.m_query_input.m_cg_translation.z);

            const float remaining_frac = 1.0f - ht;
            remaining_disp.x = remaining_frac * cur_translation.x;
            remaining_disp.y = remaining_frac * cur_translation.y;
            remaining_disp.z = remaining_frac * cur_translation.z;

            full_disp.x = vel_translation.x;
            full_disp.y = vel_translation.y;
            full_disp.z = vel_translation.z;

            elapsed += hit_real_time;
            advanced_time = hit_real_time;
        }

        plane_count = list_geom_plane.get_count();

        // ---- Iterate trace hits and add ones within MIN_SUBSTEP of advanced_time ----
        for (gjk_trace_output_t *gto = traceOut.m_list.m_first; gto; gto = gto->m_next_link)
        {
            if (MIN_SUBSTEP >= (gto->m_hit_time * remaining_time) - advanced_time)
            {
                // ORIGINAL CODE: dead block - the v124/v125 vec3's are written but never read.
                // Kept here (commented) only because the compiled binary does execute these calls.
                //if (gjkcc_in->m_ent_num != -1)
                //{
                //    phys_vec3 v126;
                //    v126.x = gto->m_hit_point.x + (20.0f * gto->m_hit_normal.x);
                //    v126.y = gto->m_hit_point.y + (20.0f * gto->m_hit_normal.y);
                //    v126.z = gto->m_hit_point.z + (20.0f * gto->m_hit_normal.z);
                //    float v124[3], v125[3];
                //    Phys_NitrousVecToVec3(&gto->m_hit_point, v125);
                //    Phys_NitrousVecToVec3(&v126, v124);
                //}

                if (no_push_out && gto->m_hit_dist < 0.0f)
                {
                    const float dist = gto->m_hit_dist;
                    gto->m_hit_point.x += dist * gto->m_hit_normal.x;
                    gto->m_hit_point.y += dist * gto->m_hit_normal.y;
                    gto->m_hit_point.z += dist * gto->m_hit_normal.z;
                    gto->m_hit_dist = 0.0f;
                }

                add_hit_info(gto, &list_geom_plane, &gti.m_query_input.m_geom_skip_list);
                input->custom_process(gto);
            }
        }

        // At least one plane should have been added
        iassert(list_geom_plane.get_count() > plane_count);

        // ---- Compute right_side for every plane at the new pos ----
        for (int pi = 0; pi < list_geom_plane.get_count(); ++pi)
        {
            geom_plane *gp = &list_geom_plane[pi];
            gp->m_lambda = 0.0f;

            const float cpx = pos.x + gp->m_arm.x;
            const float cpy = pos.y + gp->m_arm.y;
            const float cpz = pos.z + gp->m_arm.z;

            gp->m_right_side = gp->m_d
                             - ((cpx * gp->m_normal.x)
                              + (cpy * gp->m_normal.y)
                              + (cpz * gp->m_normal.z));

            if ((no_push_out || gp->m_no_push_out) && gp->m_right_side > 0.0f)
                gp->m_right_side = 0.0f;
        }

        // Project remaining_disp onto the plane set -> cur_translation
        project(&remaining_disp, &list_geom_plane, &cur_translation);

        // ---- Bail to expensive push-out if total displacement got out of hand ----
        const float tdx = (pos.x + cur_translation.x) - start_pos.x;
        const float tdy = (pos.y + cur_translation.y) - start_pos.y;
        const float tdz = (pos.z + cur_translation.z) - start_pos.z;
        const float total_disp_sq = (tdx * tdx) + (tdy * tdy) + (tdz * tdz);

        if (total_disp_sq > max_disp_sq)
        {
            *needs_push_out = 1;
            return false;
        }

        // Clamp positive (outward-pushing) right_sides to 0 before re-projecting velocity
        for (int pi = 0; pi < list_geom_plane.get_count(); ++pi)
        {
            geom_plane *gp = &list_geom_plane[pi];
            if (gp->m_right_side > 0.0f)
                gp->m_right_side = 0.0f;
        }

        // Project full_disp -> vel_translation
        project(&full_disp, &list_geom_plane, &vel_translation);

        // Early-out if remaining motion is below 0.01 units
        const float disp_sq = (cur_translation.x * cur_translation.x)
                            + (cur_translation.y * cur_translation.y)
                            + (cur_translation.z * cur_translation.z);

        if ((0.0099999998f * 0.0099999998f) > disp_sq)
            break;
    } // bump loop

    if (g_bDebugRenderPlayerCollision->current.enabled)
        Sys_LeaveCriticalSection(CRTISECT_DEBUG_BRUSHES_AND_PATCHES);

    // ----------------------------------------------------------------
    // Recover velocity from accumulated vel_translation
    // ----------------------------------------------------------------
    const float inv_dt = 1.0f / input->frametime;
    vel.x = inv_dt * vel_translation.x;
    vel.y = inv_dt * vel_translation.y;
    vel.z = inv_dt * vel_translation.z;

    // ----------------------------------------------------------------
    // Step-down pass (find ground after a slide)
    // ----------------------------------------------------------------
    if (do_step_down && input->do_step_down)
    {
        // Find min walkable distance below us; reset all m_active flags
        float min_walkable_dist = 10000.0f;
        int   non_walkable_count = 0;

        for (int pi = 0; pi < list_geom_plane.get_count(); ++pi)
        {
            geom_plane *gp = &list_geom_plane[pi];
            gp->m_active = 0;

            if (gp->m_walkable)
            {
                const float cpx = pos.x + gp->m_arm.x;
                const float cpy = pos.y + gp->m_arm.y;
                const float cpz = pos.z + gp->m_arm.z;
                const float dist = ((cpx * gp->m_normal.x)
                                  + (cpy * gp->m_normal.y)
                                  + (cpz * gp->m_normal.z))
                                 - gp->m_d;
                if (min_walkable_dist > dist)
                    min_walkable_dist = dist;
            }
            else
            {
                ++non_walkable_count;
            }
        }

        // Only step down if currently above any walkable surface by more than threshold
        if (min_walkable_dist > WALKABLE_DIST_THRESH && min_walkable_dist != 10000.0f)
        {
            phys_vec3 step_dir;
            phys_vec3 step_cur;
            step_dir.x = 0.0f;
            step_dir.y = 0.0f;
            step_dir.z = -30.0f;
            step_cur.x = 0.0f;
            step_cur.y = 0.0f;
            step_cur.z = -30.0f;
            step_cur.w = step_dir.w; // matches original (uninitialised w copy)

            for (int j = 0; j < list_geom_plane.get_count(); ++j)
            {
                // ---- Find non-active plane hit earliest along step_cur ----
                float       best_t = 1.0f;
                geom_plane *potential_landing_plane = nullptr;

                for (int pi = 0; pi < list_geom_plane.get_count(); ++pi)
                {
                    geom_plane *gp = &list_geom_plane[pi];
                    if (gp->m_active)
                        continue;

                    const float cpx = pos.x + gp->m_arm.x;
                    const float cpy = pos.y + gp->m_arm.y;
                    const float cpz = pos.z + gp->m_arm.z;

                    const float dist_to_plane = ((cpx * gp->m_normal.x)
                                               + (cpy * gp->m_normal.y)
                                               + (cpz * gp->m_normal.z))
                                              - gp->m_d;
                    const float dot_vel = (step_cur.x * gp->m_normal.x)
                                        + (step_cur.y * gp->m_normal.y)
                                        + (step_cur.z * gp->m_normal.z);

                    if (fabs(dot_vel) > 0.001f)
                    {
                        const float t = (-dist_to_plane) / dot_vel;
                        if (best_t > t)
                        {
                            potential_landing_plane = gp;
                            best_t = t;
                        }
                    }
                }

                if (!potential_landing_plane)
                    break;

                potential_landing_plane->m_active = 1;

                // Step pos along step_cur by best_t
                const float step_pos_x = pos.x + best_t * step_cur.x;
                const float step_pos_y = pos.y + best_t * step_cur.y;
                const float step_pos_z = pos.z + best_t * step_cur.z;

                // ---- Resolve landing plane ----
                geom_plane *landing_plane = nullptr;

                if (potential_landing_plane->m_walkable)
                {
                    landing_plane = potential_landing_plane;
                }
                else
                {
                    landing_plane = nullptr;
                    float best_landing_dist = WALKABLE_DIST_THRESH;

                    for (int pi = 0; pi < list_geom_plane.get_count(); ++pi)
                    {
                        geom_plane *gp = &list_geom_plane[pi];
                        if (!gp->m_walkable)
                            continue;

                        if (gp->m_active)
                        {
                            if (!Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp",
                                    1482, 0, "%s", "gp->m_active == false"))
                                __debugbreak();
                        }

                        const float cpx = step_pos_x + gp->m_arm.x;
                        const float cpy = step_pos_y + gp->m_arm.y;
                        const float cpz = step_pos_z + gp->m_arm.z;
                        const float d = ((cpx * gp->m_normal.x)
                                       + (cpy * gp->m_normal.y)
                                       + (cpz * gp->m_normal.z))
                                      - gp->m_d;
                        if (best_landing_dist >= d)
                        {
                            landing_plane = gp;
                            best_landing_dist = d;
                        }
                    }
                }

                if (landing_plane)
                {
                    if (best_t > 0.0f)
                    {
                        pos.x = step_pos_x;
                        pos.y = step_pos_y;
                        pos.z = step_pos_z;

                        // Project velocity onto the landing plane normal
                        float landing_plane_normal[3];
                        float vel3[3];
                        Phys_NitrousVecToVec3(landing_plane->m_normal, landing_plane_normal);
                        Phys_NitrousVecToVec3(vel, vel3);
                        PM_ProjectVelocity(vel3, landing_plane_normal, vel3);
                        Phys_Vec3ToNitrousVec(vel3, vel);
                    }
                    break;
                }

                // ---- No landing plane found: re-compute right_side for ALL planes ----
                // (Compiled binary checks potential_landing_plane->m_active here, which
                //  was just set to 1 above - so the gate is unconditionally true and
                //  every plane in the list gets its right_side recomputed.)
                for (int pi = 0; pi < list_geom_plane.get_count(); ++pi)
                {
                    geom_plane *gp = &list_geom_plane[pi];
                    if (potential_landing_plane->m_active)
                    {
                        const float cpx = pos.x + gp->m_arm.x;
                        const float cpy = pos.y + gp->m_arm.y;
                        const float cpz = pos.z + gp->m_arm.z;
                        gp->m_right_side = gp->m_d
                                         - ((cpx * gp->m_normal.x)
                                          + (cpy * gp->m_normal.y)
                                          + (cpz * gp->m_normal.z));
                        if (gp->m_right_side > 0.0f)
                            gp->m_right_side = 0.0f;
                    }
                }

                project(&step_dir, &list_geom_plane, &step_cur);
            }
        }
    }

    Phys_NitrousVecToVec3(pos, output->new_position);
    Phys_NitrousVecToVec3(vel, output->new_velocity);

    return list_geom_plane.get_count() > 0;
}

int __cdecl gjk_slide_move(
    const gjkcc_input_t *gjkcc_in,
    gjk_slide_move_input_t *input,
    gjk_slide_move_output_t *output)
{
    const float *velocity; // [esp+0h] [ebp-28h]
    const float *position; // [esp+4h] [ebp-24h]
    float p1[3]; // [esp+8h] [ebp-20h] BYREF
    float p0[3]; // [esp+14h] [ebp-14h] BYREF
    int retv; // [esp+20h] [ebp-8h]
    bool needs_push_out; // [esp+27h] [ebp-1h] BYREF
    int savedregs; // [esp+28h] [ebp+0h] BYREF

    output->expensive_push_out = 0;
    output->expensive_push_out_failed = 0;
    retv = gjk_slide_move1(gjkcc_in, input, 0, output, &needs_push_out);
    if (!needs_push_out)
    {
        iassert(!CrazyFloat(output->new_position[0]));
        iassert(!CrazyFloat(output->new_position[1]));
        iassert(!CrazyFloat(output->new_position[2]));
        return retv;
    }

    if (phys_debugExpensivePushout->current.enabled)
    {
        position = (const float *)input->position;
        p0[0] = *position;
        p0[1] = position[1];
        p0[2] = position[2];
        velocity = (const float *)input->velocity;
        p1[0] = *velocity;
        p1[1] = velocity[1];
        p1[2] = velocity[2];
        CG_DebugLine(p0, p1, colorRed, 1, 1000);
        p1[0] = p0[0];
        p1[1] = p0[1];
        p1[2] = p0[2] + 10000.0;
        CG_DebugLine(p0, p1, colorBlue, 1, 1000);
    }
    output->expensive_push_out = 1;
    if (!gjk_push_out(gjkcc_in, input, output))
    {
        output->expensive_push_out_failed = 1;
        gjk_slide_move1(gjkcc_in, input, 1, output, &needs_push_out);
    }

    iassert(!CrazyFloat(output->new_position[0]));
    iassert(!CrazyFloat(output->new_position[1]));
    iassert(!CrazyFloat(output->new_position[2]));
    return 1;
}

void    setup_player_push_slide_move_input(
                player_push_slide_move_input_t *input,
                pmove_t *pm,
                pml_t *pml,
                const phys_vec3 *push_movement)
{
    phys_vec3 vel; // [esp-30h] [ebp-3Ch] BYREF
    float inv_frametime; // [esp-8h] [ebp-14h]
    float frametime; // [esp-4h] [ebp-10h]

    frametime = 0.033333335;
    inv_frametime = 1.0 / 0.033333335;
    vel.x = inv_frametime * push_movement->x;
    vel.y = inv_frametime * push_movement->y;
    vel.z = inv_frametime * push_movement->z;
    Phys_NitrousVecToVec3(vel, input->velocity_);
    input->clientnum = pm->ps->clientNum;
    input->do_step_down = 1;
    input->frametime = frametime;
    input->gravity = 0;
    input->has_gravity = 0;
    input->maxs = &pm->maxs;
    input->mins = &pm->mins;
    input->position = &pm->ps->origin;
    input->tracemask = pm->tracemask;
    input->velocity = &input->velocity_;
}

void    gjk_sentient_push(
                pmove_t *pm,
                pml_t *pml,
                float *origin,
                float *mins,
                float *maxs,
                int contentMask,
                int passEntityNum,
                const gjkcc_input_t *gjkcc_in,
                const float pushRadius,
                float frameTime,
                float *velocity_,
                float *origin_)
{
    //void *v13; // esp
    int i; // [esp-1AC8h] [ebp-1AD4h]
    //const phys_vec3 *v15; // [esp-1AC4h] [ebp-1AD0h]
    phys_vec3 v16; // [esp-1AC0h] [ebp-1ACCh] BYREF
    geom_plane *v17; // [esp-1AA4h] [ebp-1AB0h]
    geom_plane *v18; // [esp-1AA0h] [ebp-1AACh]
    geom_plane *v19; // [esp-1A9Ch] [ebp-1AA8h]
    geom_plane *v20; // [esp-1A98h] [ebp-1AA4h]
    geom_plane *m_slot_array; // [esp-1A94h] [ebp-1AA0h]
    gjk_slide_move_output_t v22; // [esp-1A90h] [ebp-1A9Ch] BYREF
    float v24; // [esp-1A3Ch] [ebp-1A48h]
    float v25; // [esp-1A38h] [ebp-1A44h]
    float v26; // [esp-1A34h] [ebp-1A40h]
    phys_vec3 v27; // [esp-1A30h] [ebp-1A3Ch] BYREF
    phys_vec3 *p_m_arm; // [esp-1A14h] [ebp-1A20h]
    float v29; // [esp-1A10h] [ebp-1A1Ch]
    float v30; // [esp-1A0Ch] [ebp-1A18h]
    float v31; // [esp-1A08h] [ebp-1A14h]
    float v32; // [esp-19F4h] [ebp-1A00h]
    float v33; // [esp-19F0h] [ebp-19FCh]
    float v34; // [esp-19ECh] [ebp-19F8h]
    float v35; // [esp-19E8h] [ebp-19F4h]
    geom_plane *v36; // [esp-19E4h] [ebp-19F0h]
    float v37; // [esp-19E0h] [ebp-19ECh]
    float v38; // [esp-19DCh] [ebp-19E8h]
    float v39; // [esp-19D8h] [ebp-19E4h]
    float v40; // [esp-19D0h] [ebp-19DCh]
    float v41; // [esp-19CCh] [ebp-19D8h]
    float v42; // [esp-19C8h] [ebp-19D4h]
    float v43; // [esp-19C4h] [ebp-19D0h]
    float v44; // [esp-19C0h] [ebp-19CCh]
    float v45; // [esp-19BCh] [ebp-19C8h]
    float v46; // [esp-19B8h] [ebp-19C4h]
    float v47; // [esp-19B4h] [ebp-19C0h]
    float v48; // [esp-19B0h] [ebp-19BCh]
    float v49; // [esp-19ACh] [ebp-19B8h]
    float v50; // [esp-19A8h] [ebp-19B4h]
    float v51; // [esp-199Ch] [ebp-19A8h]
    float v52; // [esp-1998h] [ebp-19A4h]
    float v53; // [esp-1994h] [ebp-19A0h]
    float *p_x; // [esp-1990h] [ebp-199Ch]
    gjk_entity_info_t *xform; // [esp-198Ch] [ebp-1998h]
    float v56; // [esp-1988h] [ebp-1994h]
    gjk_base_t *m_cg; // [esp-1984h] [ebp-1990h]
    float v58; // [esp-1980h] [ebp-198Ch]
    bool v59; // [esp-1979h] [ebp-1985h]
    gjk_geom_info_t *v60; // [esp-1978h] [ebp-1984h]
    gjk_geom_info_t *m_next_link; // [esp-1974h] [ebp-1980h]
    gjk_geom_info_t *v62; // [esp-1970h] [ebp-197Ch]
    gjk_geom_info_t *v63; // [esp-196Ch] [ebp-1978h]
    gjk_geom_info_t *m_first; // [esp-1968h] [ebp-1974h]
    char v65; // [esp-1961h] [ebp-196Dh]
    gjk_trace_input_t v67; // [esp-150h] [ebp-15Ch] BYREF
    phys_vec3 v68; // [esp-A0h] [ebp-ACh] BYREF
    float v69; // [esp-90h] [ebp-9Ch]
    float v70; // [esp-8Ch] [ebp-98h]
    float v71; // [esp-88h] [ebp-94h]
    float v72; // [esp-80h] [ebp-8Ch]
    float v73; // [esp-7Ch] [ebp-88h]
    float v74; // [esp-78h] [ebp-84h]
    //const phys_vec3 *v75; // [esp-74h] [ebp-80h]
    phys_vec3 v76; // [esp-70h] [ebp-7Ch] BYREF
    float v77; // [esp-60h] [ebp-6Ch]
    float v78; // [esp-5Ch] [ebp-68h]
    float v79; // [esp-58h] [ebp-64h]
    float v80; // [esp-50h] [ebp-5Ch]
    float v81; // [esp-4Ch] [ebp-58h]
    float v82; // [esp-48h] [ebp-54h]
    //const phys_vec3 *v83; // [esp-44h] [ebp-50h]
    phys_vec3 v84; // [esp-40h] [ebp-4Ch] BYREF
    float v85; // [esp-30h] [ebp-3Ch]
    float v86; // [esp-2Ch] [ebp-38h]
    float v87; // [esp-28h] [ebp-34h]
    float v88; // [esp-14h] [ebp-20h]
    int v89; // [esp-10h] [ebp-1Ch]
    int v90; // [esp-Ch] [ebp-18h]
    int v91; // [esp-8h] [ebp-14h]
    //unsigned int v92[3]; // [esp+0h] [ebp-Ch] BYREF
    //_UNKNOWN *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v92[0] = a1;
    //v92[1] = retaddr;
    //v13 = alloca(6864);
    if ( frameTime <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp",
                    1603,
                    0,
                    "%s",
                    "frameTime > 0.0f") )
    {
        __debugbreak();
    }
    v89 = 0;
    v90 = 0;
    v91 = 0;
    v88 = 2.0f;

    v85 = pushRadius + 2.0;
    v86 = pushRadius + 2.0;
    v87 = 0.0f;

    Phys_Vec3ToNitrousVec(mins, v84);
    v82 = v84.x - v85;
    v81 = v84.y - v86;
    v80 = v84.z - v87;
    v77 = v82;
    v78 = v81;
    v79 = v80;
    Phys_Vec3ToNitrousVec(maxs, v76);
    v74 = v76.x + v85;
    v73 = v76.y + v86;
    v72 = v76.z + v87;
    v69 = v74;
    v70 = v73;
    v71 = v72;
    Phys_Vec3ToNitrousVec(origin, v68);
    //gjk_query_input::gjk_query_input(&v67.m_query_input);
    setup_trace_info(gjkcc_in, origin, mins, maxs, origin, passEntityNum, contentMask, &v67);
    v67.m_query_input.m_cg_aabb_min.x = v77;
    v67.m_query_input.m_cg_aabb_min.y = v78;
    v67.m_query_input.m_cg_aabb_min.z = v79;
    v67.m_query_input.m_cg_aabb_max.x = v69;
    v67.m_query_input.m_cg_aabb_max.y = v70;
    v67.m_query_input.m_cg_aabb_max.z = v71;
    v67.m_query_input.m_gjk_query_flags = 2;
    //gjk_query_output::query_prolog(v67.m_query_output, &v67.m_query_input);
    v67.m_query_output->query_prolog(&v67.m_query_input);
    gjk_query_cached(v67.m_query_input, v67.m_query_output);
    //gjk_query_output::query_epilog(v67.m_query_output);
    v67.m_query_output->query_epilog();

    phys_static_array<geom_plane, 128> v66; // [esp-1960h] [ebp-196Ch] BYREF
    //v66.m_slot_array = (geom_plane *const)&v66;
    //v66.m_alloc_count = 0;
    v65 = 0;
    m_first = v67.m_query_output->m_list_geom_info.m_first;
    v63 = m_first;
    v62 = 0;
    while ( v62 != v63 )
    {
        v60 = v63;
        if ( v63->m_ent_info )
        {
            v59 = 0;
            if ( v60->m_ent_info->m_ent_type )
            {
                if ( v60->m_ent_info->m_ent_type == gjk_entity_info_t::ENTITY_TYPE::ET_CENT
                    && (v60->m_ent_info->get_cent()->nextState.eType == 17
                     || v60->m_ent_info->get_cent()->nextState.eType == 1))
                {
                    v59 = 1;
                }
            }
            else
            {
                v59 = v60->m_ent_info->get_gent()->sentient != 0;
            }
            if ( v59 && v60->m_cg->get_type() == 7 )
            {
                v58 = pushRadius + 15.0;
                m_cg = v60->m_cg;
                v56 = m_cg[1].m_aabb_mn_loc.w + m_cg[1].m_aabb_mn_loc.y;
                //xform = gjk_geom_info_t::get_xform(v60);
                xform = v60->get_xform();
                p_x = &xform->m_mat.w.x;
                v53 = v68.x - xform->m_mat.w.x;
                v52 = v68.y - xform->m_mat.w.y;
                v51 = v68.z - xform->m_mat.w.z;
                v48 = v53;
                v49 = v52;
                v50 = 0.0f;
                v47 = (float)((float)(v48 * v48) + (float)(v49 * v49)) + (float)(0.0 * 0.0);
                v46 = (float)((float)(pushRadius + 15.0) + v56) + v88;
                v45 = v46 * v46;
                if ( (float)(v46 * v46) > v47 && v47 > 0.001 )
                {
                    v44 = sqrtf(v47);
                    v43 = 1.0 / v44;
                    v42 = (float)(1.0 / v44) * v48;
                    v41 = (float)(1.0 / v44) * v49;
                    v40 = (float)(1.0 / v44) * v50;
                    v37 = v42;
                    v38 = v41;
                    v39 = v40;
                    //v36 = phys_static_array<geom_plane,128>::add(&v66, 0, "phys array add overflow.");
                    v36 = v66.add(0, "phys array add overflow.");
                    v36->m_normal.x = v42;
                    v36->m_normal.y = v38;
                    v36->m_normal.z = v39;
                    v35 = -(v58);
                    v34 = (-(v58)) * v37;
                    v33 = (-(v58)) * v38;
                    v32 = (-(v58)) * v39;
                    v29 = v34;
                    v30 = v33;
                    v31 = v32;
                    p_m_arm = &v36->m_arm;
                    v36->m_arm.x = v34;
                    p_m_arm->y = v30;
                    p_m_arm->z = v31;
                    v36->m_d = (float)((float)((float)(v36->m_normal.x * *p_x) + (float)(v36->m_normal.y * p_x[1]))
                                                     + (float)(v36->m_normal.z * p_x[2]))
                                     + v56;
                    v36->m_lambda = 0.0f;
                    v36->m_right_side = (float)(v36->m_d
                                                                        - (float)((float)((float)(v68.x * v36->m_normal.x) + (float)(v68.y * v36->m_normal.y))
                                                                                        + (float)(v68.z * v36->m_normal.z)))
                                                        + v58;
                    v36->m_active = 1;
                    v36->m_walkable = 0;
                    v36->m_no_push_out = 0;
                    v65 = 1;
                }
            }
        }
        m_next_link = v63->m_next_link;
        v63 = m_next_link;
    }
    if ( v65 )
    {
        project(&PHYS_ZERO_VEC, &v66, &v27);
        v26 = (float)((float)(v27.x * v27.x) + (float)(v27.y * v27.y)) + (float)(v27.z * v27.z);
        if ( v26 > (float)(pushRadius * pushRadius) )
        {
            v25 = sqrtf(v26);
            v24 = pushRadius / v25;
            v27.x = v27.x * (float)(pushRadius / v25);
            v27.y = v27.y * (float)(pushRadius / v25);
            v27.z = v27.z * (float)(pushRadius / v25);
        }
        player_push_slide_move_input_t v23; // [esp-1A74h] [ebp-1A80h] BYREF
        //v23.__vftable = (player_push_slide_move_input_t_vtbl *)&player_push_slide_move_input_t::`vftable';
        setup_player_push_slide_move_input(&v23, pm, pml, &v27);
        gjk_slide_move(gjkcc_in, &v23, &v22);
        *origin_ = v22.new_position[0];
        origin_[1] = v22.new_position[1];
        origin_[2] = v22.new_position[2];
        m_slot_array = v66.get_list_head();
        v20 = v66.get_list_head();
        v19 = &v66.get_list_head()[v66.get_count()];
        v18 = v19;
        while ( v18 != v20 )
        {
            v17 = v20;
            if ( v20->m_right_side > 0.0 )
                v17->m_right_side = 0.0f;
            ++v20;
        }
        Phys_Vec3ToNitrousVec(pm->ps->velocity, v16);
        v27.x = v16.x;
        v27.y = v16.y;
        v27.z = v16.z;
        project(&v27, &v66, &v27);
        Phys_NitrousVecToVec3(v27, pm->ps->velocity);
    }
}

bool render_bounding_box = true;
void render_gjkcc_collision(float (*mins)[3], float (*maxs)[3], float (*origin)[3])
{
    const phys_vec3 *v4; // eax
    const phys_vec3 *v5; // eax
    const phys_vec3 *v6; // eax
    const phys_vec3 *v7; // eax
    const phys_vec3 *v8; // eax
    const phys_vec3 *v9; // eax
    const phys_vec3 *v10; // eax
    const phys_vec3 *v11; // eax
    const phys_vec3 *v12; // eax
    const phys_vec3 *v13; // eax
    const phys_vec3 *v14; // eax
    const phys_vec3 *v15; // [esp-8h] [ebp-39Ch]
    const phys_vec3 *v16; // [esp-8h] [ebp-39Ch]
    phys_vec3 v17; // [esp+18h] [ebp-37Ch] BYREF
    phys_vec3 v18; // [esp+28h] [ebp-36Ch] BYREF
    phys_vec3 v19; // [esp+38h] [ebp-35Ch] BYREF
    phys_vec3 v20; // [esp+48h] [ebp-34Ch] BYREF
    phys_vec3 v21; // [esp+58h] [ebp-33Ch] BYREF
    phys_vec3 v22; // [esp+68h] [ebp-32Ch] BYREF
    phys_vec3 v23; // [esp+78h] [ebp-31Ch] BYREF
    phys_vec3 v24; // [esp+88h] [ebp-30Ch] BYREF
    phys_vec3 v25; // [esp+98h] [ebp-2FCh] BYREF
    phys_vec3 v26; // [esp+A8h] [ebp-2ECh] BYREF
    phys_vec3 v27; // [esp+B8h] [ebp-2DCh] BYREF
    phys_vec3 v28; // [esp+C8h] [ebp-2CCh] BYREF
    phys_vec3 v29; // [esp+D8h] [ebp-2BCh] BYREF
    phys_vec3 v30; // [esp+E8h] [ebp-2ACh] BYREF
    float v31; // [esp+FCh] [ebp-298h]
    float v32; // [esp+100h] [ebp-294h]
    float v33; // [esp+104h] [ebp-290h]
    phys_vec3 v34; // [esp+108h] [ebp-28Ch] BYREF
    float v35; // [esp+11Ch] [ebp-278h]
    float v36; // [esp+120h] [ebp-274h]
    float v37; // [esp+124h] [ebp-270h]
    float v38; // [esp+128h] [ebp-26Ch]
    float v39; // [esp+12Ch] [ebp-268h]
    float v40; // [esp+130h] [ebp-264h]
    float v41; // [esp+13Ch] [ebp-258h]
    float v42; // [esp+140h] [ebp-254h]
    float v43; // [esp+144h] [ebp-250h]
    phys_vec3 v44; // [esp+148h] [ebp-24Ch] BYREF
    float rvec2_12; // [esp+160h] [ebp-234h]
    float v46; // [esp+164h] [ebp-230h]
    phys_vec3 v47; // [esp+168h] [ebp-22Ch] BYREF
    float rvec1_4; // [esp+178h] [ebp-21Ch]
    float rvec1_8; // [esp+17Ch] [ebp-218h]
    float rvec1_12; // [esp+180h] [ebp-214h] BYREF
    float v51; // [esp+184h] [ebp-210h] BYREF
    int v52; // [esp+188h] [ebp-20Ch]
    float co2; // [esp+18Ch] [ebp-208h] BYREF
    float si2; // [esp+190h] [ebp-204h] BYREF
    int next_i; // [esp+194h] [ebp-200h]
    phys_vec3 co1; // [esp+198h] [ebp-1FCh] BYREF
    float v57; // [esp+1B4h] [ebp-1E0h]
    phys_vec3 v58; // [esp+1B8h] [ebp-1DCh] BYREF
    float v59; // [esp+1D4h] [ebp-1C0h]
    phys_vec3 v60; // [esp+1D8h] [ebp-1BCh] BYREF
    float foot_pos_8; // [esp+1ECh] [ebp-1A8h]
    float foot_pos_12; // [esp+1F0h] [ebp-1A4h]
    float v63; // [esp+1F4h] [ebp-1A0h]
    int v64; // [esp+1F8h] [ebp-19Ch]
    int v65; // [esp+1FCh] [ebp-198h]
    float v66; // [esp+200h] [ebp-194h]
    float v67; // [esp+214h] [ebp-180h]
    phys_vec3 v68; // [esp+218h] [ebp-17Ch] BYREF
    float head_pos_8; // [esp+22Ch] [ebp-168h]
    float head_pos_12; // [esp+230h] [ebp-164h]
    float v71; // [esp+234h] [ebp-160h]
    int v72; // [esp+238h] [ebp-15Ch]
    int v73; // [esp+23Ch] [ebp-158h]
    float v74; // [esp+240h] [ebp-154h]
    float m_half_height; // [esp+250h] [ebp-144h]
    float v76; // [esp+254h] [ebp-140h]
    phys_vec3 v77; // [esp+258h] [ebp-13Ch] OVERLAPPED BYREF
    float pos_8; // [esp+26Ch] [ebp-128h]
    float pos_12; // [esp+270h] [ebp-124h]
    float v80; // [esp+274h] [ebp-120h]
    phys_vec3 *p_m_center; // [esp+278h] [ebp-11Ch]
    const gjk_polygon_cylinder_t *pc; // [esp+27Ch] [ebp-118h]
    float mx[3]; // [esp+280h] [ebp-114h] BYREF
    float mn[3]; // [esp+28Ch] [ebp-108h] BYREF
    phys_vec3 aabb_min; // [esp+298h] [ebp-FCh] BYREF
    phys_vec3 aabb_max; // [esp+2A8h] [ebp-ECh] BYREF
    phys_mat44 xform; // [esp+2B8h] [ebp-DCh] BYREF
    phys_vec3 position; // [esp+2F8h] [ebp-9Ch] BYREF
    gjk_polygon_cylinder_t gjk_geom; // [esp+308h] [ebp-8Ch] BYREF

    setup_gjk_polygon_cylinder((float *)mins, (float *)maxs, 0.5, &gjk_geom);
    Phys_Vec3ToNitrousVec((float *)origin, position);
    if (render_bounding_box)
    {
        memcpy(&xform, &PHYS_IDENTITY_MATRIX, sizeof(xform));
        //phys_vec3::operator=(&xform.w, &position);
        xform.w = position;
        //gjk_polygon_cylinder_t::calc_aabb(&gjk_geom, (int)v90, &xform, &aabb_min, &aabb_max);
        gjk_geom.calc_aabb(xform, &aabb_min, &aabb_max);
        Phys_NitrousVecToVec3(aabb_min, mn);
        Phys_NitrousVecToVec3(aabb_max, mx);
        render_box(mn, mx, colorRed, 0);
    }
    pc = &gjk_geom;
    p_m_center = &gjk_geom.m_center;
    v80 = gjk_geom.m_center.x + position.x;
    pos_12 = gjk_geom.m_center.y + position.y;
    pos_8 = gjk_geom.m_center.z + position.z;
    v77.x = gjk_geom.m_center.x + position.x;
    v77.y = gjk_geom.m_center.y + position.y;
    v77.z = gjk_geom.m_center.z + position.z;
    v76 = gjk_geom.m_polygon_cylinder_radius + gjk_geom.m_geom_radius;
    m_half_height = gjk_geom.m_half_height;
    v72 = 0;
    v73 = 0;
    v74 = gjk_geom.m_half_height;
    v71 = (float)(gjk_geom.m_center.x + position.x) + 0.0;
    head_pos_12 = (float)(gjk_geom.m_center.y + position.y) + 0.0;
    head_pos_8 = (float)(gjk_geom.m_center.z + position.z) + gjk_geom.m_half_height;
    v68.x = v71;
    v68.y = head_pos_12;
    v68.z = head_pos_8;
    v67 = gjk_geom.m_half_height;
    v64 = 0;
    v65 = 0;
    v66 = gjk_geom.m_half_height;
    v63 = (float)(gjk_geom.m_center.x + position.x) - 0.0;
    foot_pos_12 = (float)(gjk_geom.m_center.y + position.y) - 0.0;
    foot_pos_8 = (float)(gjk_geom.m_center.z + position.z) - gjk_geom.m_half_height;
    v60.x = v63;
    v60.y = foot_pos_12;
    v60.z = foot_pos_8;
    v59 = gjk_geom.m_half_height - gjk_geom.m_head_offset;
    v58.x = 0.0f;
    v58.y = 0.0f;
    v58.z = gjk_geom.m_half_height - gjk_geom.m_head_offset;
    //v57 = COERCE_FLOAT(LODWORD(gjk_geom.m_half_height) ^ _mask__NegFloat_) + gjk_geom.m_foot_offset;
    v57 = -gjk_geom.m_half_height + gjk_geom.m_foot_offset;
    co1.x = 0.0f;
    co1.y = 0.0f;
    co1.z = v57;
    for (next_i = 0; next_i < 12; ++next_i)
    {
        //gjk_polygon_cylinder_t::poly_verts::get_co_si(&gjk_polygon_cylinder_t::s_poly_verts, next_i, &co2, &si2);
        gjk_polygon_cylinder_t::s_poly_verts.get_co_si(next_i, &co2, &si2);
        v52 = (next_i + 1) % 12;
        //gjk_polygon_cylinder_t::poly_verts::get_co_si(&gjk_polygon_cylinder_t::s_poly_verts, v52, &rvec1_12, &v51);
        gjk_polygon_cylinder_t::s_poly_verts.get_co_si(v52, &rvec1_12, &v51);
        rvec1_8 = v76 * co2;
        rvec1_4 = v76 * si2;
        v47.x = v76 * co2;
        v47.y = v76 * si2;
        v47.z = 0.0f;
        v46 = v76 * rvec1_12;
        rvec2_12 = v76 * v51;
        v44.x = v76 * rvec1_12;
        v44.y = v76 * v51;
        v44.z = 0.0f;
        v43 = v77.x + co1.x;
        v42 = v77.y + co1.y;
        v41 = v77.z + co1.z;
        v38 = v77.x + co1.x;
        v39 = v77.y + co1.y;
        v40 = v77.z + co1.z;
        v37 = (float)(v77.x + co1.x) + (float)(v76 * co2);
        v36 = (float)(v77.y + co1.y) + (float)(v76 * si2);
        v35 = (float)(v77.z + co1.z) + 0.0;
        v34.x = v37;
        v34.y = v36;
        v34.z = v35;
        v33 = v77.x + v58.x;
        v32 = v77.y + v58.y;
        v31 = v77.z + v58.z;
        v30.x = v77.x + v58.x;
        v30.y = v77.y + v58.y;
        v30.z = v77.z + v58.z;
        v29 = v30 + v47;
        //v4 = operator+(&v29, &v30, &v47);
        v29 = v30 + v47;
        //render_line(v4, &v34, colorBlue, 0, 0);
        render_line(&v29, &v34, colorBlue, 0, 0);
        //v5 = operator+(&v28, &v77, &v58);
        v28 = v77 + v58;
        //v15 = operator+(&v27, v5, &v44);
        v27 = v28 + v44;
        //v6 = operator+(&v26, &v77, &v58);
        v26 = v77 + v58;
        //v7 = operator+(&v25, v6, &v47);
        v25 = v26 + v47;
        //render_line(v7, v15, colorBlue, 0, 0);
        render_line(&v25, &v27, colorBlue, 0, 0);
        //v8 = operator+(&v24, &v77, &co1);
        v24 = v77 + co1;
        //v16 = operator+(&v23, v8, &v44);
        v23 = v24 + v44;
        //v9 = operator+(&v22, &v77, &co1);
        v22 = v77 + co1;
        //v10 = operator+(&v21, v9, &v47);
        v21 = v22 + v47;
        //render_line(v10, v16, colorBlue, 0, 0);
        render_line(&v21, &v23, colorBlue, 0, 0);
        //v11 = operator+(&v20, &v77, &v58);
        v20 = v77 + v58;
        //v12 = operator+(&v19, v11, &v47);
        v19 = v20 + v47;
        //render_line(v12, &v68, colorBlue, 0, 0);
        render_line(&v19, &v68, colorBlue, 0, 0);
        //v13 = operator+(&v18, &v77, &co1);
        v18 = v77 + co1;
        //v14 = operator+(&v17, v13, &v47);
        v17 = v18 + v47;
        //render_line(v14, &v60, colorBlue, 0, 0);
        render_line(&v17, &v60, colorBlue, 0, 0);
    }
    //gjk_base_t::~gjk_base_t(&gjk_geom);
}

void __cdecl render_gjkcc_collision(pmove_t *pm)
{
    if ( render_player_collision->current.integer == 1 )
        render_gjkcc_collision(&pm->mins, &pm->maxs, &pm->ps->origin);
}

void    PM_gjk_ground_trace(
                const gjkcc_input_t *gjkcc_in,
                trace_t *results,
                float *start,
                float *mins,
                float *maxs,
                float *end,
                int passEntityNum,
                int contentMask,
                float *first_hit_time)
{
    bool walkable; // [esp-Bh] [ebp-109h]
    float hit_time; // [esp-Ah] [ebp-108h]
    const gjk_trace_output_t *i; // [esp-2h] [ebp-100h]
    bool first_gto_walkable; // [esp+5h] [ebp-F9h]
    float first_hit_timea; // [esp+6h] [ebp-F8h]
    const gjk_trace_output_t *first_gto; // [esp+Ah] [ebp-F4h]
    float first_depth; // [esp+Eh] [ebp-F0h]
    float depth; // [esp+26h] [ebp-D8h]
    float search_depth; // [esp+EEh] [ebp-10h]

    search_depth = 0.5f;

    gjk_trace_input_t gti;

    setup_trace_info(gjkcc_in, start, mins, maxs, end, passEntityNum, contentMask, &gti);
    gti.m_exit_on_penetration = 0;
    depth = start[2] - end[2];
    
    iassert(depth >= 0.0f);
    if (search_depth <= depth)
        gti.m_extra_time = search_depth / depth;
    else
        gti.m_extra_time = 1.0f;

    list_gjk_trace_output list;
    gjk_trace(gti, &list);

    if (list.m_first_hit)
    {
        if (first_hit_time)
            *first_hit_time = list.m_first_hit->m_hit_time;
        first_depth = ((end[2] - start[2]) * list.m_first_hit->m_hit_time) + start[2];
        first_gto = list.m_list.m_first;
        first_hit_timea = list.m_list.m_first->m_hit_time;
        first_gto_walkable = is_walkable(list.m_list.m_first);
        for (i = first_gto->m_next_link; i; i = i->m_next_link)
        {
            depth = ((end[2] - start[2]) * i->m_hit_time) + start[2];
            iassert(depth <= first_depth);
            hit_time = i->m_hit_time;

            if (i->m_gi->m_cg->get_type() == GJK_PARTITION)
                hit_time -= 0.001;

            if (search_depth >= (first_depth - depth))
            {
                if (first_gto_walkable)
                {
                    if (first_hit_timea > hit_time && is_walkable(i))
                    {
                        first_hit_timea = hit_time;
                        first_gto = i;
                    }
                }
                else
                {
                    walkable = is_walkable(i);
                    if (first_hit_timea > hit_time || walkable)
                    {
                        first_hit_timea = hit_time;
                        first_gto = i;
                        first_gto_walkable = walkable;
                    }
                }
            }
        }
        fill_results(*first_gto, first_gto_walkable, results);
    }
    else
    {
        fill_results_no_hit(results);
    }
}

double __cdecl PM_PermuteRestrictiveClipPlanes(
                const float *velocity,
                int planeCount,
                const float (*planes)[3],
                int *permutation)
{
    int permutedIndex; // [esp+8h] [ebp-28h]
    float parallel[8]; // [esp+Ch] [ebp-24h]
    int planeIndex; // [esp+2Ch] [ebp-4h]

    iassert(velocity);
    iassert(planeCount > 0 && planeCount <= 8);
    iassert(planes);
    iassert(permutation);

    for ( planeIndex = 0; planeIndex < planeCount; ++planeIndex )
    {
        parallel[planeIndex] = Vec3Dot(velocity, &(*planes)[3 * planeIndex]);

        for ( permutedIndex = planeIndex;
                    permutedIndex && parallel[planeIndex] <= parallel[permutation[permutedIndex - 1]];
                    --permutedIndex )
        {
            permutation[permutedIndex] = permutation[permutedIndex - 1];
        }
        permutation[permutedIndex] = planeIndex;
    }
    return parallel[*permutation];
}

void __cdecl setup_player_gjk_slide_move_input(
                player_gjk_slide_move_input_t *input,
                pmove_t *pm,
                pml_t *pml,
                int gravity)
{
    input->clientnum = pm->ps->clientNum;
    input->do_step_down = !gravity && pml->groundPlane && (pm->ps->pm_flags & 8) == 0;
    input->frametime = pml->frametime;
    input->gravity = pm->ps->gravity;
    input->has_gravity = gravity;
    input->maxs = &pm->maxs;
    input->mins = &pm->mins;
    input->position = &pm->ps->origin;
    input->tracemask = pm->tracemask;
    input->velocity = &pm->ps->velocity;
    input->pm = pm;
}

void __cdecl PM_StepSlideMove(pmove_t *pm, pml_t *pml, int gravity)
{
    float *v3; // [esp+10h] [ebp-150h]
    float *v4; // [esp+14h] [ebp-14Ch]
    float *v5; // [esp+30h] [ebp-130h]
    float *v6; // [esp+38h] [ebp-128h]
    float *v7; // [esp+3Ch] [ebp-124h]
    float *v8; // [esp+48h] [ebp-118h]
    float *v9; // [esp+4Ch] [ebp-114h]
    float *v10; // [esp+50h] [ebp-110h]
    float *v11; // [esp+54h] [ebp-10Ch]
    float *v12; // [esp+58h] [ebp-108h]
    float *v13; // [esp+5Ch] [ebp-104h]
    float *velocity; // [esp+64h] [ebp-FCh]
    float *origin; // [esp+68h] [ebp-F8h]
    float v16; // [esp+70h] [ebp-F0h]
    float v17; // [esp+74h] [ebp-ECh]
    int old; // [esp+88h] [ebp-D8h]
    float fSpeedScale; // [esp+94h] [ebp-CCh]
    int iDelta; // [esp+9Ch] [ebp-C4h]
    int iDeltaa; // [esp+9Ch] [ebp-C4h]
    float flatDelta; // [esp+A4h] [ebp-BCh]
    float flatDelta_4; // [esp+A8h] [ebp-B8h]
    float down_v; // [esp+B4h] [ebp-ACh]
    float down_v_4; // [esp+B8h] [ebp-A8h]
    float down_v_8; // [esp+BCh] [ebp-A4h]
    int jumping; // [esp+C0h] [ebp-A0h]
    int iBumps; // [esp+C8h] [ebp-98h]
    float start_o[3]; // [esp+CCh] [ebp-94h] BYREF
    trace_t trace; // [esp+D8h] [ebp-88h] BYREF
    float endpos[3]; // [esp+114h] [ebp-4Ch] BYREF
    float start_v[3]; // [esp+120h] [ebp-40h] BYREF
    float down_o[3]; // [esp+12Ch] [ebp-34h] BYREF
    float up[3]; // [esp+138h] [ebp-28h] BYREF
    int bHadGround; // [esp+144h] [ebp-1Ch]
    float down[3]; // [esp+148h] [ebp-18h] BYREF
    playerState_s *ps; // [esp+154h] [ebp-Ch]
    float fStepSize; // [esp+158h] [ebp-8h] BYREF
    float fStepAmount; // [esp+15Ch] [ebp-4h]
    int savedregs; // [esp+160h] [ebp+0h] BYREF

    fStepAmount = 0.0f;
    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp", 2239, 0, "%s", "ps") )
        __debugbreak();
    jumping = 0;
    if ( (ps->pm_flags & 8) != 0 )
    {
        bHadGround = 0;
        Jump_ClearState(ps);
    }
    else if ( pml->groundPlane )
    {
        bHadGround = 1;
    }
    else
    {
        bHadGround = 0;
        if ( (ps->pm_flags & 0x4000) != 0 && ps->pm_time )
            Jump_ClearState(ps);
    }
    start_o[0] = ps->origin[0];
    start_o[1] = ps->origin[1];
    start_o[2] = ps->origin[2];
    start_v[0] = ps->velocity[0];
    start_v[1] = ps->velocity[1];
    start_v[2] = ps->velocity[2];
    iBumps = PM_SlideMove(pm, pml, gravity);
    if ( (ps->pm_flags & 1) != 0 )
        fStepSize = 10.0f;
    else
        fStepSize = 18.0f;
    if ( ps->groundEntityNum != 1023 )
        goto LABEL_27;
    if ( (ps->pm_flags & 0x4000) != 0 && ps->pm_time )
        Jump_ClearState(ps);
    if ( iBumps && (ps->pm_flags & 0x4000) != 0 && Jump_GetStepHeight(ps, start_o, &fStepSize) )
    {
        if ( fStepSize < 1.0 )
            return;
        jumping = 1;
    }
    if ( jumping || (ps->pm_flags & 8) != 0 && ps->velocity[2] > 0.0 )
    {
LABEL_27:
        down_o[0] = ps->origin[0];
        down_o[1] = ps->origin[1];
        down_o[2] = ps->origin[2];
        down_v = ps->velocity[0];
        down_v_4 = ps->velocity[1];
        down_v_8 = ps->velocity[2];
        flatDelta = down_o[0] - start_o[0];
        flatDelta_4 = down_o[1] - start_o[1];
        if ( iBumps || pml->groundPlane && pml->groundTrace.normal.vec.v[2] < 0.89999998 )
        {
            up[0] = start_o[0];
            up[1] = start_o[1];
            up[2] = (float)(fStepSize + 1.0) + start_o[2];
            PM_playerTrace(pm, &trace, start_o, pm->mins, pm->maxs, up, ps->clientNum, pm->tracemask);
            fStepAmount = (float)((float)(fStepSize + 1.0) * trace.fraction) - 1.0;
            if ( fStepAmount >= 1.0 )
            {
                origin = ps->origin;
                v16 = up[1];
                v17 = start_o[2] + fStepAmount;
                ps->origin[0] = up[0];
                origin[1] = v16;
                origin[2] = v17;
                velocity = ps->velocity;
                ps->velocity[0] = start_v[0];
                velocity[1] = start_v[1];
                velocity[2] = start_v[2];
                PM_SlideMove(pm, pml, gravity);
            }
            else
            {
                fStepAmount = 0.0f;
            }
        }
        if ( bHadGround || fStepAmount != 0.0 )
        {
            down[0] = ps->origin[0];
            down[1] = ps->origin[1];
            down[2] = ps->origin[2];
            down[2] = down[2] - fStepAmount;
            if ( bHadGround )
                down[2] = down[2] - 9.0;
            if ( phys_player_collision_mode->current.integer == 1 )
                PM_gjk_ground_trace(
                    pm->m_gjkcc_input,
                    &trace,
                    ps->origin,
                    pm->mins,
                    pm->maxs,
                    down,
                    ps->clientNum,
                    pm->tracemask,
                    0);
            else
                PM_playerTrace(pm, &trace, ps->origin, pm->mins, pm->maxs, down, ps->clientNum, pm->tracemask);
            if ( Trace_GetEntityHitId(&trace) < com_maxclients->current.integer )
            {
                v13 = ps->origin;
                ps->origin[0] = down_o[0];
                v13[1] = down_o[1];
                v13[2] = down_o[2];
                v12 = ps->velocity;
                ps->velocity[0] = down_v;
                v12[1] = down_v_4;
                v12[2] = down_v_8;
                return;
            }
            if ( trace.fraction >= 1.0 )
            {
                if ( fStepAmount != 0.0 )
                    ps->origin[2] = ps->origin[2] - fStepAmount;
            }
            else
            {
                if ( phys_player_collision_mode->current.integer == 1 )
                {
                    if ( !trace.walkable )
                    {
                        v11 = ps->origin;
                        ps->origin[0] = down_o[0];
                        v11[1] = down_o[1];
                        v11[2] = down_o[2];
                        v10 = ps->velocity;
                        ps->velocity[0] = down_v;
                        v10[1] = down_v_4;
                        v10[2] = down_v_8;
                        return;
                    }
                }
                else if ( !trace.walkable && trace.normal.vec.v[2] < 0.30000001 )
                {
                    v9 = ps->origin;
                    ps->origin[0] = down_o[0];
                    v9[1] = down_o[1];
                    v9[2] = down_o[2];
                    v8 = ps->velocity;
                    ps->velocity[0] = down_v;
                    v8[1] = down_v_4;
                    v8[2] = down_v_8;
                    return;
                }
                Vec3Lerp(ps->origin, down, trace.fraction, ps->origin);
                PM_ProjectVelocity(ps->velocity, trace.normal.vec.v, ps->velocity);
            }
        }
        if ( (float)((float)((float)(flatDelta * start_v[0]) + (float)(flatDelta_4 * start_v[1])) + 0.001) >= (float)((float)((float)(ps->origin[0] - start_o[0]) * start_v[0]) + (float)((float)(ps->origin[1] - start_o[1]) * start_v[1]))
            || jumping && Jump_IsPlayerAboveMax(ps) )
        {
            v7 = ps->origin;
            ps->origin[0] = down_o[0];
            v7[1] = down_o[1];
            v7[2] = down_o[2];
            v6 = ps->velocity;
            ps->velocity[0] = down_v;
            v6[1] = down_v_4;
            v6[2] = down_v_8;
            fStepAmount = 0.0f;
            if ( bHadGround )
            {
                down[0] = ps->origin[0];
                down[1] = ps->origin[1];
                down[2] = ps->origin[2];
                down[2] = down[2] - 9.0;
                PM_playerTrace(pm, &trace, ps->origin, pm->mins, pm->maxs, down, ps->clientNum, pm->tracemask);
                if ( trace.fraction < 1.0 )
                {
                    Vec3Lerp(ps->origin, down, trace.fraction, endpos);
                    fStepAmount = endpos[2] - ps->origin[2];
                    v5 = ps->origin;
                    ps->origin[0] = endpos[0];
                    v5[1] = endpos[1];
                    v5[2] = endpos[2];
                    if ( phys_player_collision_mode->current.integer == 1 )
                        PM_ProjectVelocity(ps->velocity, trace.normal.vec.v, ps->velocity);
                    else
                        PM_ClipVelocity(ps->velocity, trace.normal.vec.v, ps->velocity);
                }
            }
        }
        if ( jumping )
            Jump_ClampVelocity(ps, down_o);
        if ( bHadGround )
        {
            if ( ps->pm_type < 9 )
            {
                if ( PM_VerifyPronePosition(pm, start_o, start_v) )
                {
                    if ( fabs(ps->origin[2] - down_o[2]) > 0.5 )
                    {
                        iDelta = SnapFloatToInt((float)(ps->origin[2] - down_o[2]));
                        if ( iDelta )
                        {
                            if ( pm->viewChangeTime < ps->commandTime || pm->viewChangeTime > ps->commandTime + 1000 )
                            {
                                pm->viewChange = (float)(ps->origin[2] - down_o[2]) + pm->viewChange;
                                pm->viewChangeTime = ps->commandTime;
                            }
                            fSpeedScale = (float)(1.0 - 0.80000001)
                                                    + (float)((float)(1.0
                                                                                    - (float)(fabs(ps->origin[2] - start_o[2])
                                                                                                    / fStepSize))
                                                                    * 0.80000001);
                            v3 = ps->velocity;
                            v4 = ps->velocity;
                            ps->velocity[0] = fSpeedScale * ps->velocity[0];
                            v3[1] = fSpeedScale * v4[1];
                            v3[2] = fSpeedScale * v4[2];
                            pm->xyspeed = Vec2Length(ps->velocity);
                            if ( (int)abs(iDelta) > 3 && ps->groundEntityNum != 1023 && PM_ShouldMakeFootsteps(pm) )
                            {
                                iDeltaa = (int)abs(iDelta) / 2;
                                if ( iDeltaa > 4 )
                                    iDeltaa = 4;
                                old = ps->bobCycle;
                                ps->bobCycle = (unsigned __int8)(int)(float)((float)old + (float)((float)((float)iDeltaa * 1.25) + 7.0));
                                PM_FootstepEvent(pm, pml, old, ps->bobCycle, 1);
                            }
                        }
                    }
                }
            }
        }
    }
}

static bool return_true; // supposed to have value? (KISAKTODO)
int __cdecl PM_VerifyPronePosition(pmove_t *pm, float *vFallbackOrg, float *vFallbackVel)
{
    unsigned __int8 v4; // al
    float save_v; // [esp+44h] [ebp-20h]
    float save_v_4; // [esp+48h] [ebp-1Ch]
    float save_v_8; // [esp+4Ch] [ebp-18h]
    float save_o; // [esp+50h] [ebp-14h]
    float save_o_4; // [esp+54h] [ebp-10h]
    float save_o_8; // [esp+58h] [ebp-Ch]
    int bProneOK; // [esp+5Ch] [ebp-8h]
    playerState_s *ps; // [esp+60h] [ebp-4h]
    int savedregs; // [esp+64h] [ebp+0h] BYREF

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp", 1871, 0, "%s", "ps") )
        __debugbreak();
    if ( return_true )
        return 1;
    if ( (ps->pm_flags & 1) == 0 )
        return 1;
    v4 = BG_CheckProne(
                 ps,
                 ps->clientNum,
                 ps->origin,
                 15.0,
                 30.0,
                 ps->proneDirection,
                 &ps->fTorsoPitch,
                 &ps->fWaistPitch,
                 1,
                 1,
                 1,
                 pm->handler,
                 PCT_CLIENT,
                 50.0);
    bProneOK = v4;
    if ( !v4 )
    {
        save_o = ps->origin[0];
        save_o_4 = ps->origin[1];
        save_o_8 = ps->origin[2];
        save_v = ps->velocity[0];
        save_v_4 = ps->velocity[1];
        save_v_8 = ps->velocity[2];
        ps->origin[0] = *vFallbackOrg;
        ps->origin[1] = vFallbackOrg[1];
        ps->origin[2] = vFallbackOrg[2];
        ps->velocity[0] = *vFallbackVel;
        ps->velocity[1] = vFallbackVel[1];
        ps->velocity[2] = vFallbackVel[2];
        if ( BG_CheckProne(
                     ps,
                     ps->clientNum,
                     ps->origin,
                     15.0,
                     30.0,
                     ps->proneDirection,
                     &ps->fTorsoPitch,
                     &ps->fWaistPitch,
                     1,
                     1,
                     1,
                     pm->handler,
                     PCT_CLIENT,
                     50.0) )
        {
            return 0;
        }
        else
        {
            ps->origin[0] = save_o;
            ps->origin[1] = save_o_4;
            ps->origin[2] = save_o_8;
            ps->velocity[0] = save_v;
            ps->velocity[1] = save_v_4;
            ps->velocity[2] = save_v_8;
            return 1;
        }
    }
    return bProneOK;
}

int __cdecl PM_SlideMove(pmove_t *pm, pml_t *pml, int gravity)
{
    float *velocity; // edx
    unsigned __int16 EntityHitId; // ax
    float *v6; // [esp+8h] [ebp-1F4h]
    float *v7; // [esp+Ch] [ebp-1F0h]
    float *v8; // [esp+10h] [ebp-1ECh]
    float *v9; // [esp+14h] [ebp-1E8h]
    float *v10; // [esp+18h] [ebp-1E4h]
    float *v11; // [esp+30h] [ebp-1CCh]
    float *v12; // [esp+34h] [ebp-1C8h]
    float *v13; // [esp+38h] [ebp-1C4h]
    float *v14; // [esp+3Ch] [ebp-1C0h]
    float *v15; // [esp+44h] [ebp-1B8h]
    float *v16; // [esp+48h] [ebp-1B4h]
    float *v17; // [esp+4Ch] [ebp-1B0h]
    float *origin; // [esp+74h] [ebp-188h]
    // [esp+78h] [ebp-184h] BYREF
    int retv; // [esp+A8h] [ebp-154h]
    gjk_slide_move_output_t output; // [esp+ACh] [ebp-150h] BYREF
    const gjkcc_input_t *gjkcc_in; // [esp+C8h] [ebp-134h]
    int j; // [esp+CCh] [ebp-130h]
    float dir[3]; // [esp+D0h] [ebp-12Ch] BYREF
    float d; // [esp+DCh] [ebp-120h]
    int numbumps; // [esp+E0h] [ebp-11Ch]
    float endClipVelocity[3]; // [esp+E4h] [ebp-118h] BYREF
    int k; // [esp+F0h] [ebp-10Ch]
    float planes[8][3]; // [esp+F4h] [ebp-108h] BYREF
    int permutation[8]; // [esp+158h] [ebp-A4h] BYREF
    float time_left; // [esp+178h] [ebp-84h]
    float end[3]; // [esp+17Ch] [ebp-80h] BYREF
    int numplanes; // [esp+188h] [ebp-74h]
    int bumpcount; // [esp+18Ch] [ebp-70h]
    float primal_velocity[3]; // [esp+190h] [ebp-6Ch]
    float endVelocity[3]; // [esp+1D8h] [ebp-24h] BYREF
    float clipVelocity[3]; // [esp+1E4h] [ebp-18h] BYREF
    int i; // [esp+1F0h] [ebp-Ch]
    playerState_s *ps; // [esp+1F4h] [ebp-8h]
    float into; // [esp+1F8h] [ebp-4h]

    if ( phys_player_collision_mode->current.integer == 1 )
    {
        player_gjk_slide_move_input_t input;
        setup_player_gjk_slide_move_input(&input, pm, pml, gravity);
        gjkcc_in = pm->m_gjkcc_input;
        retv = gjk_slide_move(gjkcc_in, &input, &output);
        origin = pm->ps->origin;

        *origin = output.new_position[0];
        origin[1] = output.new_position[1];
        origin[2] = output.new_position[2];
        velocity = pm->ps->velocity;
        *velocity = output.new_velocity[0];
        velocity[1] = output.new_velocity[1];
        velocity[2] = output.new_velocity[2];
        return retv;
    }
    else
    {
        trace_t trace; // [esp+19Ch] [ebp-60h] BYREF
        ps = pm->ps;
        if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_slidemove.cpp", 2031, 0, "%s", "ps") )
            __debugbreak();
        numbumps = 4;
        primal_velocity[0] = ps->velocity[0];
        primal_velocity[1] = ps->velocity[1];
        primal_velocity[2] = ps->velocity[2];
        endVelocity[0] = ps->velocity[0];
        endVelocity[1] = ps->velocity[1];
        endVelocity[2] = ps->velocity[2];
        if ( gravity )
        {
            endVelocity[2] = endVelocity[2] - (float)((float)ps->gravity * pml->frametime);
            ps->velocity[2] = (float)(ps->velocity[2] + endVelocity[2]) * 0.5;
            primal_velocity[2] = endVelocity[2];
            if ( pml->groundPlane )
                PM_ClipVelocity(ps->velocity, pml->groundTrace.normal.vec.v, ps->velocity);
        }
        time_left = pml->frametime;
        if ( pml->groundPlane )
        {
            *(_QWORD *)&planes[0][0] = *(_QWORD *)pml->groundTrace.normal.vec.v;
            LODWORD(planes[0][2]) = pml->groundTrace.normal.vec.u[2];
            numplanes = 1;
        }
        else
        {
            numplanes = 0;
        }
        Vec3NormalizeTo(ps->velocity, planes[numplanes++]);
        for ( bumpcount = 0; bumpcount < numbumps; ++bumpcount )
        {
            end[0] = (float)(time_left * ps->velocity[0]) + ps->origin[0];
            end[1] = (float)(time_left * ps->velocity[1]) + ps->origin[1];
            end[2] = (float)(time_left * ps->velocity[2]) + ps->origin[2];
            PM_playerTrace(pm, &trace, ps->origin, pm->mins, pm->maxs, end, ps->clientNum, pm->tracemask);
            if ( trace.allsolid )
            {
                ps->velocity[2] = 0.0f;
                return 1;
            }
            if ( trace.startsolid && do_push )
            {
                v16 = ps->origin;
                v17 = ps->origin;
                ps->origin[0] = ps->origin[0] + trace.normal.vec.v[0];
                v16[1] = v17[1] + trace.normal.vec.v[1];
                v16[2] = v17[2] + trace.normal.vec.v[2];
            }
            if ( trace.fraction > 0.0 )
                Vec3Lerp(ps->origin, end, trace.fraction, ps->origin);
            if ( trace.fraction == 1.0 )
                break;
            EntityHitId = Trace_GetEntityHitId(&trace);
            PM_AddTouchEnt(pm, EntityHitId);
            time_left = time_left - (float)(time_left * trace.fraction);
            if ( numplanes >= 8 )
            {
                v15 = ps->velocity;
                ps->velocity[0] = 0.0f;
                v15[1] = 0.0f;
                v15[2] = 0.0f;
                return 1;
            }
            for ( i = 0; i < numplanes; ++i )
            {
                if ( (float)((float)((float)(trace.normal.vec.v[0] * planes[i][0])
                                                     + (float)(trace.normal.vec.v[1] * planes[i][1]))
                                     + (float)(trace.normal.vec.v[2] * planes[i][2])) > 0.99900001 )
                {
                    PM_ClipVelocity(ps->velocity, trace.normal.vec.v, ps->velocity);
                    v13 = ps->velocity;
                    v14 = ps->velocity;
                    ps->velocity[0] = trace.normal.vec.v[0] + ps->velocity[0];
                    v13[1] = trace.normal.vec.v[1] + v14[1];
                    v13[2] = trace.normal.vec.v[2] + v14[2];
                    break;
                }
            }
            if ( i >= numplanes )
            {
                v12 = planes[numplanes];
                *(_QWORD *)v12 = *(_QWORD *)trace.normal.vec.v;
                v12[2] = trace.normal.vec.v[2];
                into = PM_PermuteRestrictiveClipPlanes(ps->velocity, ++numplanes, planes, permutation);
                if ( into < 0.1 )
                {
                    if ( (-(into)) > pml->impactSpeed )
                        pml->impactSpeed = -into;
                    PM_ClipVelocity(ps->velocity, planes[permutation[0]], clipVelocity);
                    PM_ClipVelocity(endVelocity, planes[permutation[0]], endClipVelocity);
                    for ( j = 1; j < numplanes; ++j )
                    {
                        v11 = planes[permutation[j]];
                        if ( (float)((float)((float)(clipVelocity[0] * *v11) + (float)(clipVelocity[1] * v11[1]))
                                             + (float)(clipVelocity[2] * v11[2])) < 0.1 )
                        {
                            PM_ClipVelocity(clipVelocity, planes[permutation[j]], clipVelocity);
                            PM_ClipVelocity(endClipVelocity, planes[permutation[j]], endClipVelocity);
                            if ( (float)((float)((float)(clipVelocity[0] * planes[permutation[0]][0])
                                                                 + (float)(clipVelocity[1] * planes[permutation[0]][1]))
                                                 + (float)(clipVelocity[2] * planes[permutation[0]][2])) < 0.0 )
                            {
                                Vec3Cross(planes[permutation[0]], planes[permutation[j]], dir);
                                Vec3Normalize(dir);
                                d = (float)((float)(dir[0] * ps->velocity[0]) + (float)(dir[1] * ps->velocity[1]))
                                    + (float)(dir[2] * ps->velocity[2]);
                                clipVelocity[0] = d * dir[0];
                                clipVelocity[1] = d * dir[1];
                                clipVelocity[2] = d * dir[2];
                                d = (float)((float)(dir[0] * endVelocity[0]) + (float)(dir[1] * endVelocity[1]))
                                    + (float)(dir[2] * endVelocity[2]);
                                endClipVelocity[0] = d * dir[0];
                                endClipVelocity[1] = d * dir[1];
                                endClipVelocity[2] = d * dir[2];
                                for ( k = 1; k < numplanes; ++k )
                                {
                                    if ( k != j )
                                    {
                                        v10 = planes[permutation[k]];
                                        if ( (float)((float)((float)(clipVelocity[0] * *v10) + (float)(clipVelocity[1] * v10[1]))
                                                             + (float)(clipVelocity[2] * v10[2])) < 0.1 )
                                        {
                                            v9 = ps->velocity;
                                            ps->velocity[0] = 0.0f;
                                            v9[1] = 0.0f;
                                            v9[2] = 0.0f;
                                            return 1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    v8 = ps->velocity;
                    ps->velocity[0] = clipVelocity[0];
                    v8[1] = clipVelocity[1];
                    v8[2] = clipVelocity[2];
                    endVelocity[0] = endClipVelocity[0];
                    endVelocity[1] = endClipVelocity[1];
                    endVelocity[2] = endClipVelocity[2];
                }
            }
        }
        if ( gravity )
        {
            v7 = ps->velocity;
            ps->velocity[0] = endVelocity[0];
            v7[1] = endVelocity[1];
            v7[2] = endVelocity[2];
        }
        if ( ps->pm_time )
        {
            v6 = ps->velocity;
            ps->velocity[0] = primal_velocity[0];
            v6[1] = primal_velocity[1];
            v6[2] = primal_velocity[2];
        }
        return bumpcount != 0;
    }
}

void __thiscall player_gjk_slide_move_input_t::custom_process(gjk_trace_output_t *gto)
{
    trace_t trace; // [esp+4h] [ebp-3Ch] BYREF

    fill_results_type_and_id(gto, &trace);
    PM_AddTouchEnt(this->pm, Trace_GetEntityHitId(&trace));
    PM_AddTouchGlass(this->pm, Trace_GetGlassHitId(&trace));
}
