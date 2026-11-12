#pragma once
#include "phys_local.h"
#include "phys_gjk.h"

enum $067C566A97EEC6E7D3DB7B726B5C1A66 : __int32
{
    COL_PARTITION = 0x0,
    COL_BRUSH     = 0x1,
};

struct centity_s;
struct gentity_s;
struct cbrush_t;
struct trace_t;
struct DynEntityDef;
struct gjk_query_input;
struct gjk_query_output;
struct gjk_entity_info_t;
struct gjk_trace_output_t;

struct destructible_ent_aa : phys_auto_activate_callback // sizeof=0xC
{
    bool m_has_auto_activated;
    // padding byte
    // padding byte
    // padding byte
    centity_s *m_cent;


    bool has_auto_activated()
    {
        return m_has_auto_activated;
    }
    void auto_activate(broad_phase_info *bpi_impactor);
};
static_assert(sizeof(destructible_ent_aa) == 12);

struct dynamic_ent_aa : phys_auto_activate_callback // sizeof=0xC
{
    bool m_has_auto_activated;
    // padding byte
    // padding byte
    // padding byte
    DynEntityDef *m_dynEntDef;

    bool has_auto_activated()
    {
        return m_has_auto_activated;
    }
    void auto_activate(broad_phase_info *bpi_impactor);
};

struct __declspec(align(8)) gjk_entity_info_t // sizeof=0x50
{
    enum ENTITY_TYPE : __int32
    {                                       // XREF: gjk_entity_info_t/r
        ET_GENT = 0x0,
        ET_CENT = 0x1,
        ET_DENT = 0x2,
        ET_GLASS = 0x3,
        ET_NONE = 0x4,
    };
    phys_mat44 m_mat;
    gjk_entity_info_t::ENTITY_TYPE m_ent_type;
    const void *m_ent;
    int m_query_visitor_count;
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    const Glass *get_glass()
    {
        iassert(m_ent_type == ET_GLASS && m_ent != NULL);
        return (const Glass *)this->m_ent;
    }

    const void *get_ent()
    {
        iassert(m_ent_type != ET_NONE && m_ent != NULL);
        return this->m_ent;
    }

    inline const gentity_s *get_gent()
    {
        iassert(m_ent_type == ET_GENT && m_ent != NULL);
        return (const gentity_s *)this->m_ent;
    }

    inline const centity_s *get_cent()
    {
        iassert(m_ent_type == ET_CENT && m_ent != NULL);
        return (const centity_s *)this->m_ent;
    }

    const DynEntityDef *get_dent() const
    {
        iassert(m_ent_type == ET_DENT && m_ent != NULL);
        return (const DynEntityDef *)this->m_ent;
    }

    bool is_dent() const
    {
        return this->m_ent_type == ET_DENT;
    }

    bool is_gent() const
    {
        return this->m_ent_type == ET_GENT;
    }
};

void __cdecl gjk_collision_epilog(bool is_server_thread);


bool query_should_pass_gentity(const gjk_query_input &input, const gentity_s *gent);

void gjk_query_prims(const gjk_query_input &input, gjk_query_output *output);
void gjk_query_terrain(const gjk_query_input &input, gjk_query_output *output);
void gjk_query_gents(const gjk_query_input &input, gjk_query_output *output);
void gjk_query_cents(const gjk_query_input &input, gjk_query_output *output);
void gjk_query_glass(const gjk_query_input &input, gjk_query_output *output);
void gjk_query_dents(const gjk_query_input &input, gjk_query_output *output);
void gjk_query(const gjk_query_input &input, gjk_query_output *output);
void gjk_query_cached(const gjk_query_input &input, gjk_query_output *output);


char __cdecl gjk_trace_aabb(
                const phys_vec3 *p0,
                const phys_vec3 *dir,
                const phys_vec3 *aabb_min,
                const phys_vec3 *aabb_max,
                float *t_);
char __cdecl query_should_pass_entity(const gjk_query_input &input, gjk_entity_info_t *ent_info);
void    setup_gjk_capsule(
                float *mins,
                float *maxs,
                float radius_adjust,
                struct gjk_double_sphere_t *gjk_capsule);
void    setup_gjk_cylinder(
                float *mins,
                float *maxs,
                float radius_adjust,
                struct gjk_cylinder_t *gjk_cylinder);

bool    is_walkable(
                const cbrush_t *brush,
                const phys_vec3 *hit_point_loc,
                const phys_vec3 *up_loc);
bool    is_walkable(
                const struct CollisionPartition *partition,
                const phys_vec3 *hit_point_loc,
                const phys_vec3 *up_loc);

void    project(
                const phys_vec3 *point,
                phys_static_array<geom_plane,128> *list_geom_plane,
                phys_vec3 *result);
void __cdecl get_material_from_brush(const cbrush_t *brush, int *sflags);
void __cdecl fill_results(const gjk_trace_output_t &gto, bool is_walkable, trace_t *results);
void __cdecl fill_results_type_and_id(const gjk_trace_output_t *gto, trace_t *results);
void __cdecl fill_results_no_hit(trace_t *results);
bool    is_walkable(const gjk_trace_output_t *gto);

// Free-function replacements for the old gjk_base_t::is_walkable / is_foot virtuals (dispatch on get_type()).
bool gjk_is_walkable(const gjk_base_t *geom, const phys_vec3 *hit_point, const phys_vec3 *up);
bool gjk_is_foot(const gjk_base_t *geom, const phys_vec3 *hit_point);

//phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal *__thiscall phys_simple_allocator<phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal>::allocate(
//                phys_simple_allocator<phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal> *this);
