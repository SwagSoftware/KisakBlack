#pragma once

#include "phys_local.h"
#include "phys_traverse.h"
#include "phys_colgeom.h"
#include <qcommon/common.h>
#include <qcommon/cm_load.h>
#include <qcommon/cm_test.h>
#include <qcommon/cm_trace.h>

struct pmove_t;
struct centity_s;
struct gentity_s;
struct gjk_base_t;
struct Glass;
struct DynEntityDef;
struct CollisionPartition;

struct __declspec(align(16)) gjk_geom_info_t // sizeof=0x40
{
    phys_vec3 m_aabb_min;
    phys_vec3 m_aabb_max;
    gjk_base_t *m_cg;
    struct gjk_entity_info_t *m_ent_info;
    int m_query_visitor_count;
    float m_hit_time;
    gjk_geom_info_t *m_next_link;
    gjk_geom_info_t *m_total_next_link;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    void calc_aabb();
    struct gjk_entity_info_t *get_xform();
};

const struct __declspec(align(16)) gjk_query_input // sizeof=0x80
{                                       // XREF: gjk_trace_input_t/r
                                        // ?cached_query_resize@gjk_query_output@@QAEX_NPAV?$colgeom_visitor_inlined_t@$0MI@@@H@Z/r
    phys_vec3 m_cg_aabb_min;
    phys_vec3 m_cg_aabb_max;
    phys_vec3 m_cg_position;
    phys_vec3 m_cg_translation;
    phys_vec3 m_ac_eps_vec;
    int m_contents;
    int m_pass_entity_num;
    int m_pass_owner_num;
    bool m_is_server_thread;
    // padding byte
    // padding byte
    // padding byte
    colgeom_visitor_inlined_t<200> *m_proximity_data;
    int m_proximity_mask;
    unsigned int m_gjk_query_flags; // 1 = terrain/prims; 2 = gents/cents/glass; 4 = dents(dynents)
    phys_link_list<gjk_geom_info_t> m_geom_skip_list;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    gjk_query_input()
    {
        //this->m_geom_skip_list.m_first = 0;
        //this->m_geom_skip_list.m_last_next_ptr = &this->m_geom_skip_list.m_first;
        //this->m_geom_skip_list.m_alloc_count = 0;
    }

    void visit_skip_list(int query_visitor_count) const;
    bool is_in_skip_list(const gjk_geom_info_t *gi_) const;
};

struct gjk_collision_visitor : gjk_geom_allocator
{                                                                             // XREF: create_gjk_geom_collision_visitor/r

    //struct /*VFT*/ gjk_collision_visitor_vtbl // sizeof=0x1C
    //{
    //    void *(* allocate)(gjk_collision_visitor * this, const int, const int, const bool);
    //    bool(* is_query)(gjk_collision_visitor * this);
    //    void(*get_local_query_aabb)(gjk_collision_visitor *this, float *, float *);
    //    bool(* query_create_prolog)(gjk_collision_visitor * this, const void *);
    //    void(* query_create_epilog)(gjk_collision_visitor * this, gjk_base_t *);
    //    bool(* query_create_prolog_1)(gjk_collision_visitor * this, const float *, const float *, const void *);
    //    void(*query_create_epilog_1)(gjk_collision_visitor *this, gjk_base_t *);
    //
    // };
    virtual void *allocate(const int, const int, const bool) = 0;
    virtual const bool is_query() override
    {
        return false;
    }
    virtual const phys_gjk_geom_id get_unique_id() override
    {
        return g_gjk_unique_id_database.get_unique_id();
    }
    virtual void get_local_query_aabb(float *, float *)
    {
        //if (!Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.h", 35, 0, "%s", "0"))
        //    __debugbreak();
        iassert(0);
    }
    virtual bool query_create_prolog(const void *)
    {
        return true;
    }
    virtual void query_create_epilog(gjk_base_t *gjk_geom)
    {
        ;
    }

    virtual bool query_create_prolog_1(const float *, const float *, const void *)
    {
        return true;
    }

    virtual void query_create_epilog_1(gjk_base_t *)
    {
        ;
    }
};

struct gjk_physics_collision_visitor : gjk_collision_visitor // sizeof=0x80
{
    bpei_database_id m_local_database_id;
    broad_phase_environment_info *m_local_bpei;
    const void *m_local_entity;
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
    phys_vec3 m_local_query_trace_aabb_min;
    phys_vec3 m_local_query_trace_aabb_max;
    phys_vec3 m_local_query_trace_translation;
    const centity_s *cent;
    const DynEntityDef *dynEntDef;
    const Glass *glass;
    rigid_body *rb;
    const phys_mat44 *rb_to_world_xform;
    const phys_mat44 *cg_to_world_xform;
    const phys_mat44 *cg_to_rb_xform;
    unsigned int env_collision_flags;
    const broad_phase_environment_query_input *bpeqi;
    broad_phase_environement_query_results *bpeqr;
    phys_auto_activate_callback *auto_activate_callback;
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    void set_local_query_info(const void *entity);

    //struct /*VFT*/ gjk_collision_visitor_vtbl // sizeof=0x1C
    //{
    //    void *(*allocate)(gjk_collision_visitor *this, const int, const int, const bool);
    //    bool (*is_query)(gjk_collision_visitor *this);
    //    void (*get_local_query_aabb)(gjk_collision_visitor *this, float *, float *);
    //    bool (*query_create_prolog)(gjk_collision_visitor *this, const void *);
    //    void (*query_create_epilog)(gjk_collision_visitor *this, gjk_base_t *);
    //    bool (*query_create_prolog_1)(gjk_collision_visitor *this, const float *, const float *, const void *);
    //    void (*query_create_epilog_1)(gjk_collision_visitor *this, gjk_base_t *);
    //};

    virtual void *allocate(const int, const int, const bool) override;
    virtual const bool is_query() override // LWSS: re-used as gjk_obb_t::is_polyhedron() in binary
    {
        return true;
    }
    virtual void get_local_query_aabb(float *, float *) override;
    virtual bool query_create_prolog(const void *) override;
    virtual void query_create_epilog(gjk_base_t *gjk_geom) override;
    virtual bool query_create_prolog_1(const float *local_aabb_min, const float *local_aabb_max, const void *geom) override;
    virtual void query_create_epilog_1(gjk_base_t *gjk_geom) override;
};

struct create_gjk_geom_collision_visitor : gjk_collision_visitor // sizeof=0x8
{                                       // XREF: .data:create_gjk_geom_collision_visitor g_empty_collision_visitor/r
                                        // XDoll_CreatePhysObj/r ...
    gjk_geom_list_t *gjk_geom_list;     // XREF: DynEntCl_CreatePhysObj(DynEntityDef const *,DynEntityClient *,GfxPlacement const *)+E8/w
    // FX_SpawnModelPhysics+5D6/w ...

    //struct /*VFT*/ gjk_collision_visitor_vtbl // sizeof=0x1C
    //{
    //    void *(*allocate)(gjk_collision_visitor *this, const int, const int, const bool);
    //    bool (*is_query)(gjk_collision_visitor *this);
    //    void (*get_local_query_aabb)(gjk_collision_visitor *this, float *, float *);
    //    bool (*query_create_prolog)(gjk_collision_visitor *this, const void *);
    //    void (*query_create_epilog)(gjk_collision_visitor *this, gjk_base_t *);
    //    bool (*query_create_prolog_1)(gjk_collision_visitor *this, const float *, const float *, const void *);
    //    void (*query_create_epilog_1)(gjk_collision_visitor *this, gjk_base_t *);
    //};

    virtual void *allocate(int size, int alignment, bool no_error) override
    {
        iassert(0);
        return 0;
    }
    // is_query() = default
    // get_local_query_aabb() = default

    virtual bool query_create_prolog(const void *geom) override
    {
        return true;
    }

    virtual void query_create_epilog(gjk_base_t *gjk_geom) override // exact same function as epilog_1()
    {
        this->gjk_geom_list->add_geom(gjk_geom);
    }

    //prolog_1() = default

    virtual void query_create_epilog_1(gjk_base_t *gjk_geom) override
    {
        this->gjk_geom_list->add_geom(gjk_geom);
    }
};

struct gjk_query_output : gjk_collision_visitor // sizeof=0x150
{                                       // XREF: gjkcc_info/r
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
    phys_vec3 m_query_aabb_min;
    phys_vec3 m_query_aabb_max;
    phys_vec3 m_local_query_aabb_min;
    phys_vec3 m_local_query_aabb_max;
    const gjk_query_input *m_local_query_input;
    bpei_database_id m_local_database_id;
    broad_phase_environment_info *m_local_bpei;
    gjk_entity_info_t *m_local_ent_info;
    bpei_database_t m_bpei_database;
    phys_transient_allocator m_allocator;
    phys_transient_allocator::allocator_state m_allocator_state;
    int m_ent_count;
    int m_geom_count;
    gjk_geom_info_t *m_total_list_geom_info;
    phys_link_list<gjk_geom_info_t> m_list_geom_info;
    int m_query_visitor_count;
    int m_gent_query_visitor_count;
    int m_cent_query_visitor_count;
    int m_dent_query_visitor_count;
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
    cached_query_info_t m_cached_query_info;
    phys_vec3 m_accum_start_origin;
    cached_query_info_t m_accum_query_info;
    int m_total_query_count;
    int m_total_cached_query_count;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    gjk_query_output()
    {
        //this->__vftable = (gjk_query_output_vtbl *)&gjk_collision_visitor::`vftable';
        //this->__vftable = (gjk_query_output_vtbl *)&gjk_query_output::`vftable';
        
        //this->m_bpei_database.m_bpei_map.m_tree_root = 0;
        //this->m_bpei_database.m_bpei_allocator.m_count = 0;
        //this->m_bpei_database.m_mutex.m_count = 1;
        //this->m_bpei_database.m_bpei_list = 0;

        this->m_allocator.m_first_block = 0;
        this->m_allocator.m_cur = 0;
        this->m_allocator.m_end = 0;
        this->m_allocator.m_total_memory_allocated = 0;
        this->m_allocator.m_mutex.m_count = 1;
        this->m_allocator.m_slot_pool = 0;
        //this->m_list_geom_info.m_first = 0;
        //this->m_list_geom_info.m_last_next_ptr = &this->m_list_geom_info.m_first;
        //this->m_list_geom_info.m_alloc_count = 0;
        this->m_total_query_count = 0;
        this->m_total_cached_query_count = 0;
        gjk_query_output::reset_cache();
        gjk_query_output::accum_query_reset(&PHYS_ZERO_VEC);
        //return this;
    }

    ~gjk_query_output()
    {
        if (this->m_allocator.m_first_block
            && _tlAssert(
                "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_transient_allocator.h",
                69,
                "m_first_block == NULL",
                ""))
        {
            __debugbreak();
        }

        this->m_bpei_database.purge_database();
        //bpei_database_t::purge_database(&this->m_bpei_database);
    }

    //// VIRTUALS ////
    virtual void *allocate(int size, int alignment, bool no_error);
    virtual const bool is_query() override // LWSS: re-used in binary as "gjk_obb_t::is_polyhedron()"  ..i.,
    {
        return true;
    }
    virtual void get_local_query_aabb(
        float *local_query_aabb_min,
        float *local_query_aabb_max) override;

    virtual bool query_create_prolog(const void *geom) override;
    virtual bool query_create_prolog_1(const float *local_aabb_min, const float *local_aabb_max, const void *geom) override;
    virtual void query_create_epilog(gjk_base_t *gjk_geom) override;
    virtual void query_create_epilog_1(gjk_base_t *gjk_geom) override;
    //////////////////

    inline void verify_empty()
    {
        iassert(m_ent_count == 0);
        iassert(m_geom_count == 0);
        iassert(m_query_visitor_count == 0);
        iassert(m_gent_query_visitor_count == 0);
        iassert(m_cent_query_visitor_count == 0);
        iassert(m_dent_query_visitor_count == 0);
        //iassert(m_list_geom_info.get_first() == NULL);
        //iassert(m_allocator.is_empty());
        iassert(m_total_list_geom_info == NULL);
        iassert(m_cached_query_info.is_empty());
    }

    void reset_cache();
    void query_prolog(const gjk_query_input *input);
    void calc_query_aabb(const gjk_query_input *input);
    void query_epilog();

    broad_phase_environment_info *get_ent_info(unsigned int ent_id);
    void set_local_query_info(
        const gjk_query_input &input,
        gjk_entity_info_t *ent_info);
    gjk_geom_info_t *create_geom_info(
        gjk_base_t *cg,
        gjk_entity_info_t *ent_info,
        float *aabb_min,
        float *aabb_max);
    gjk_entity_info_t *create_entity_info();
    void add(const gjk_query_input &input, const CollisionPartition *partition, const CollisionAabbTree *tree);
    void add(const gjk_query_input &input, const cbrush_t *brush, const float *query_mins, const float *query_maxs);
    void add(const gjk_query_input &input, gentity_s *gent);
    void add(const gjk_query_input &input, centity_s *cent);
    void add(const gjk_query_input &input, const Glass *glass);
    void add(const gjk_query_input &input, const DynEntityDef *dent);
    void cached_query_resize(
        bool is_server_thread,
        colgeom_visitor_inlined_t<200> *proximity_data,
        int proximity_mask);
    void cached_query_prolog(
        bool is_server_thread,
        colgeom_visitor_inlined_t<200> *proximity_data,
        int proximity_mask,
        const phys_vec3 *start_origin);
    void cached_query_epilog();
    void accum_query_reset(const phys_vec3 *start_origin);
};

struct __declspec(align(16)) gjkcc_info // sizeof=0x200
{
    phys_mat44 m_cg_to_world_xform;
    phys_vec3 m_cg_aabb_min;
    phys_vec3 m_cg_aabb_max;
    float m_mins[3];
    float m_maxs[3];
    phys_heap_gjk_cache_system_avl_tree m_gjk_cache;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    gjk_query_output m_gjk_query_output;
    volatile unsigned int m_active;
    bool m_is_server_thread;
    // padding byte
    // padding byte
    // padding byte
    gjk_base_t *m_cg_;
    float m_last_origin[3];
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    void update_cg(float *mins, float *maxs, bool force);
    void Lock();
    void Unlock();
};

struct __declspec(align(16)) gjk_trace_input_t // sizeof=0xB0
{                                       // XREF: gjk_player_trace(gjkcc_input_t const &,trace_t *,float const * const,float const * const,float const * const,float const * const,int,int)+10/w
                                        // ?gjk_player_trace@@YAXABUgjkcc_input_t@@PAUtrace_t@@QBM222HH@Z/r ...
    const gjk_base_t *m_cg;
    float m_gjk_ac_eps;
    bool m_keep_all_collisions;
    bool m_exit_on_penetration;
    // padding byte
    // padding byte
    float m_extra_time;
    phys_heap_gjk_cache_system_avl_tree *m_gjk_cache;
    gjk_query_output *m_query_output;
    gjkcc_info *m_gcci;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    gjk_query_input m_query_input;
    phys_transient_allocator *m_allocator;
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

    const phys_mat44 &get_cg_mat() const
    {
        return m_gcci->m_cg_to_world_xform;
    }

    void set_cg_position(const phys_vec3 *position);
};

struct generic_avl_map_node_t // sizeof=0x14
{
    void *m_data;
    unsigned int m_avl_key;
    phys_inplace_avl_tree_node<generic_avl_map_node_t> m_avl_tree_node;

    static unsigned int &get_avl_key(generic_avl_map_node_t *n) { return n->m_avl_key; }
    static const unsigned int &get_avl_key(const generic_avl_map_node_t *n) { return n->m_avl_key; }
    static phys_inplace_avl_tree_node<generic_avl_map_node_t> *get_avl_node(generic_avl_map_node_t *n) { return &n->m_avl_tree_node; }
    static const phys_inplace_avl_tree_node<generic_avl_map_node_t> *get_avl_node(const generic_avl_map_node_t *n) { return &n->m_avl_tree_node; }
};

struct __declspec(align(4)) gjkcc_info_database_t // sizeof=0x10
{                                       // XREF: .data:gjkcc_info_database_t g_gjkcc_info_client_database/r
                                        // .data:gjkcc_info_database_t g_gjkcc_info_server_database/r
    phys_inplace_avl_tree<unsigned int,generic_avl_map_node_t,generic_avl_map_node_t> m_map;
                                        // XREF: _dynamic_initializer_for__g_gjkcc_info_client_database__+3/w
                                        // _dynamic_initializer_for__g_gjkcc_info_server_database__+3/w
    volatile unsigned int m_token;                        // XREF: _dynamic_initializer_for__g_gjkcc_info_client_database__+D/w
                                        // _dynamic_initializer_for__g_gjkcc_info_server_database__+D/w
    int m_gcci_count;                   // XREF: _dynamic_initializer_for__g_gjkcc_info_client_database__+17/w
                                        // _dynamic_initializer_for__g_gjkcc_info_server_database__+17/w
    bool m_is_server_thread;            // XREF: _dynamic_initializer_for__g_gjkcc_info_client_database__+21/w
                                        // _dynamic_initializer_for__g_gjkcc_info_server_database__+21/w
    // padding byte
    // padding byte
    // padding byte

    gjkcc_info_database_t(unsigned int token, int gcci_count, bool is_server_thread)
    {
        m_token = token;
        m_gcci_count = gcci_count;
        m_is_server_thread = is_server_thread;
    }

    gjkcc_info *gjkcc_info_find_or_create(
        unsigned int gjkcc_id,
        bool is_server_thread,
        const float *origin);
    void Lock();
    void Unlock();
    void verify_internal(bool is_server_thread);
    gjkcc_info *gjkcc_info_find(
        unsigned int gjkcc_id,
        bool is_server_thread);
    void gjkcc_info_destroy(
        unsigned int gjkcc_id,
        bool is_server_thread);
};

struct gjkcc_input_t // sizeof=0x20
{                                       // XREF: ?Actor_Physics_z@@YAHPAUactor_physics_t@@@Z/r
                                        // Actor_CallThink/r ...
    unsigned int gjkcc_id;              // XREF: ai_physics_trace(trace_t *,float const * const,float const * const,float const * const,float const * const,int,int,actor_physics_t *):loc_912994/w
    bool is_server_thread;              // XREF: ai_physics_trace(trace_t *,float const * const,float const * const,float const * const,float const * const,int,int,actor_physics_t *)+5B/w
    // padding byte
    // padding byte
    // padding byte
    colgeom_visitor_inlined_t<200> *proximity_data;
    // XREF: ai_physics_trace(trace_t *,float const * const,float const * const,float const * const,float const * const,int,int,actor_physics_t *)+5F/w
    int proximity_mask;                 // XREF: ai_physics_trace(trace_t *,float const * const,float const * const,float const * const,float const * const,int,int,actor_physics_t *)+66/w
    int m_ent_num;                      // XREF: ai_physics_trace(trace_t *,float const * const,float const * const,float const * const,float const * const,int,int,actor_physics_t *)+6D/w
    unsigned int m_gjk_query_flags;     // XREF: ai_physics_trace(trace_t *,float const * const,float const * const,float const * const,float const * const,int,int,actor_physics_t *)+74/w
    const gjk_base_t *m_gjk_cg;         // XREF: ai_physics_trace(trace_t *,float const * const,float const * const,float const * const,float const * const,int,int,actor_physics_t *)+7B/w
    const phys_mat44 *m_mat;            // XREF: ai_physics_trace(trace_t *,float const * const,float const * const,float const * const,float const * const,int,int,actor_physics_t *)+82/w
};

struct __declspec(align(16)) gjk_trace_output_t // sizeof=0x50
{
    phys_vec3 m_hit_normal;
    phys_vec3 m_hit_point;
    phys_vec3 m_arm;
    float m_hit_time;
    float m_hit_dist;
    bool m_is_foot;
    // padding byte
    // padding byte
    // padding byte
    gjk_geom_info_t *m_gi;
    gjk_trace_output_t *m_next_link;
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
static_assert(sizeof(gjk_trace_output_t) == 80);

struct __declspec(align(4)) gjk_slide_move_input_t // sizeof=0x2C
{                                       // XREF: ai_gjk_slide_move_input_t/r
                                        // player_gjk_slide_move_input_t/r ...
    //gjk_slide_move_input_t_vtbl *__vftable; // XREF: PM_SlideMove+22/w
                                        // PM_SlideMove+2C/w ...
    const float (*position)[3];
    const float (*velocity)[3];
    int gravity;
    int has_gravity;
    const float (*mins)[3];
    const float (*maxs)[3];
    int tracemask;
    int clientnum;
    float frametime;
    bool do_step_down;
    // padding byte
    // padding byte
    // padding byte

    virtual void custom_process(gjk_trace_output_t *gto) = 0;
};

struct ai_gjk_slide_move_input_t : gjk_slide_move_input_t // sizeof=0x30
{                                       // XREF: AIPhys_SlideMove/r
    struct actor_physics_t *m_pPhys;

    virtual void custom_process(gjk_trace_output_t *gto) override;
};

struct __declspec(align(4)) gjk_slide_move_output_t // sizeof=0x1C
{                                       // XREF: AIPhys_SlideMove/r
                                        // PM_SlideMove/r
    float new_position[3];              // XREF: PM_SlideMove+97/r
                                        // PM_SlideMove+A9/r ...
    float new_velocity[3];              // XREF: PM_SlideMove+DD/r
                                        // PM_SlideMove+EF/r ...
    bool expensive_push_out;
    bool expensive_push_out_failed;
    // padding byte
    // padding byte
};

struct player_gjk_slide_move_input_t : gjk_slide_move_input_t // sizeof=0x30
{                                       // XREF: PM_SlideMove/r
    pmove_t *pm;

    virtual void custom_process(gjk_trace_output_t *gto) override;
};

struct player_push_slide_move_input_t : gjk_slide_move_input_t // sizeof=0x38
{
    float velocity_[3];

    void custom_process(gjk_trace_output_t *gto) override
    {

    }
};

struct list_gjk_trace_output // sizeof=0x10
{                                       // XREF: ?gjk_player_trace@@YAXABUgjkcc_input_t@@PAUtrace_t@@QBM222HH@Z/r
    phys_link_list<gjk_trace_output_t> m_list;
    gjk_trace_output_t *m_first_hit;

    list_gjk_trace_output() // inlined
    {
        //this->m_list.m_first = 0;
        //this->m_list.m_last_next_ptr = (gjk_trace_output_t **)this;
        //this->m_list.m_alloc_count = 0;
        this->m_first_hit = 0;
    }

    ~list_gjk_trace_output() // inlined
    {
        //this->m_list.m_first = 0;
        //this->m_list.m_last_next_ptr = (gjk_trace_output_t **)this;
        //this->m_list.m_alloc_count = 0;
        this->m_list.remove_all();
        this->m_first_hit = 0;
    }
};

phys_gjk_cache_info *get_gjk_cache_info(
    phys_heap_gjk_cache_system_avl_tree *gjk_cache,
    const gjk_base_t *cg1,
    const gjk_base_t *cg2);

extern create_gjk_geom_collision_visitor g_empty_collision_visitor;
