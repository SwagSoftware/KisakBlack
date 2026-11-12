#pragma once

#include <universal/q_shared.h>
#include <universal/com_math.h>

#include "phys_local.h"
#include "phys_gjk_collision_detection.h"

void __cdecl set_cpi_params(contact_point_info *cpi, phys_collision_pair *pcp);
void __cdecl set_bp_standard_query();
void __cdecl set_debug_callback();

struct gjk_physics_collision_visitor;

struct standard_query : broad_phase_terrain_query_callback // sizeof=0x4
{                                       // XREF: .data:standard_query g_standard_query/r
    void query(const broad_phase_environment_query_input &bpeqi, broad_phase_environement_query_results *bpeqr) override;
};

struct entity_bpi_header // sizeof=0x8
{
    phys_mat44 *m_mat;
    phys_auto_activate_callback *m_aac;
};

//void __thiscall contact_point_info::get_closest_psc(
//                contact_point_info *this,
//                const phys_vec3 *normal,
//                const phys_vec3 *b1_r_loc,
//                const phys_vec3 *b2_r_loc,
//                float *closest_error,
//                const contact_point_info::pulse_sum_cache_info **closest_psc);
//void __userpurge phys_contact_manifold::xform_and_translate_mesh_points(
//                phys_contact_manifold *this@<ecx>,
//                int a2@<ebp>,
//                const phys_mat44 *xform,
//                const phys_vec3 *translation);
//double __cdecl phys_contact_manifold::get_STD_COMP_FEATURE_NORMAL_DISTANCE_EPS(float penetration_t);
//double __cdecl phys_contact_manifold::get_STD_COMP_FEATURE_NORMAL_SIN_ANGULAR_EPS_SQ(float penetration_t);
//double __cdecl phys_contact_manifold::get_STD_GET_FEATURE_DISTANCE_EPS(float penetration_t);
//double __cdecl phys_contact_manifold::get_STD_GET_FEATURE_SIN_ANGULAR_EPS_SQ(float penetration_t);
//void __thiscall phys_contact_manifold_process::comp_contact_mat(
//                phys_contact_manifold_process *this,
//                const phys_vec3 *contact_normal);
//rigid_body_constraint_contact *__cdecl avl_tree_find<rigid_body_pair_key,rigid_body_constraint_contact,rigid_body_constraint_contact::avl_tree_accessor>(
//                rigid_body_constraint_contact *tree_root,
//                const rigid_body_pair_key *key);
//contact_point_info *__cdecl contact_point_info::create_cpi(
//                int point_pair_count,
//                bool no_error,
//                phys_transient_allocator *allocator);
//void __thiscall phys_contact_manifold::set_get_feature_params(
//                phys_contact_manifold *this,
//                const phys_vec3 *hitp,
//                const phys_vec3 *hitn,
//                float feature_distance_eps,
//                float sin_feautre_angular_eps_sq);
//void __cdecl contact_point_info::set_closest_cached_psc(
//                contact_point_info *cached_cpi,
//                const phys_vec3 *normal,
//                const phys_vec3 *b1_r_loc,
//                const phys_vec3 *b2_r_loc,
//                contact_point_info::pulse_sum_cache_info *psc);
//void __thiscall contact_point_info::set_closest_cached_psc(contact_point_info *this, contact_point_info *cached_cpi);
//// bad sp value at call has been detected, the output may be wrong!
//void __userpurge phys_contact_manifold_process::process(
//                phys_contact_manifold_process *this@<ecx>,
//                float a2@<ebp>,
//                int a3@<edi>,
//                int a4@<esi>,
//                phys_collision_pair *pcp,
//                phys_gjk_info *gjk_info);

void create_entity_bpi(gjk_physics_collision_visitor *collision_visitor, int mask);

struct generic_avl_map_node_t;
generic_avl_map_node_t *__cdecl generic_avl_map_add(
    phys_inplace_avl_tree<unsigned int, generic_avl_map_node_t, generic_avl_map_node_t> *gam,
    void *data,
    unsigned int avl_key);

void *__cdecl generic_avl_map_destroy(
    phys_inplace_avl_tree<unsigned int, generic_avl_map_node_t, generic_avl_map_node_t> *gam,
    unsigned int avl_key);

struct PhysObjUserData *__cdecl Phys_GetUserData(int id);