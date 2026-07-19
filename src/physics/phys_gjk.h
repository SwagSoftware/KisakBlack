#pragma once

#include "phys_local.h"
#include "phys_traverse.h"
#include "phys_colgeom.h"
#include <tl/tl_system.h>
#include "phys_transient_allocator.h"
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


struct phys_gjk_geom_id_pair_key // sizeof=0x8
{                                       // XREF: phys_gjk_cache_info/r
    unsigned int m_id1;                 // XREF: phys_heap_gjk_cache_system_avl_tree::get_gjk_cache_info(uint,uint,bool)+8B/w
    unsigned int m_id2;                 // XREF: phys_heap_gjk_cache_system_avl_tree::get_gjk_cache_info(uint,uint,bool)+91/w
};

struct broad_phase_environment_info // sizeof=0x24
{
    struct avl_tree_accessor
    {
        using key_type = bpei_database_id;
        using node_type = broad_phase_environment_info;

        static bool less(const key_type &key, const node_type *node)
        {
            if (key.m_id1 < node->m_database_id.m_id1)
                return true;
            if (key.m_id1 > node->m_database_id.m_id1)
                return false;

            return key.m_id2 < node->m_database_id.m_id2;
        }

        static bool less(const node_type *node, const key_type &key)
        {
            if (node->m_database_id.m_id1 < key.m_id1)
                return true;
            if (node->m_database_id.m_id1 > key.m_id1)
                return false;

            return node->m_database_id.m_id2 < key.m_id2;
        }

        static bool equals(const node_type *node, const key_type &key)
        {
            return node->m_database_id.m_id1 == key.m_id1 &&
                node->m_database_id.m_id2 == key.m_id2;
        }

        // Hacky aislop shit
        static void set_key(node_type *node, const key_type &key)
        {
            node->m_database_id = key;  // uses m_database_id
        }
    };

    void *m_data;
    minspec_mutex m_mutex;
    unsigned int m_gjk_geom_id;
    broad_phase_environment_info *m_next_bpei;
    phys_inplace_avl_tree_node<broad_phase_environment_info> m_avl_tree_node;
    bpei_database_id m_database_id;
};

struct bpei_database_t // sizeof=0x10
{                                       // XREF: broad_phase_memory/r
    phys_inplace_avl_tree<bpei_database_id, broad_phase_environment_info, broad_phase_environment_info::avl_tree_accessor> m_bpei_map;
    phys_simple_allocator<broad_phase_environment_info> m_bpei_allocator;
    broad_phase_environment_info *m_bpei_list;
    minspec_read_write_mutex m_mutex;

    bpei_database_t()
    {
        m_bpei_list = NULL;
    }

    ~bpei_database_t()
    {
        purge_database();
    }

    void update_database();
    void purge_database();

    broad_phase_environment_info *get_bpei_mt(bpei_database_id database_id);

    broad_phase_environment_info *get_bpei(bpei_database_id database_id);
    broad_phase_environment_info *create_bpei(bpei_database_id database_id);
};


struct cached_simplex_info // sizeof=0x30
{                                                                             // XREF: phys_gjk_cache_info/r
    phys_vec3 m_indices[3];

    //cached_simplex_info& operator=(const cached_simplex_info *__that);
    cached_simplex_info &operator=(const cached_simplex_info &) = default; // using default compiler operator
};

struct phys_gjk_cache_info // sizeof=0x80
{
//private:
    phys_vec3 m_support_dir;
    cached_simplex_info m_support_a;
    cached_simplex_info m_support_b;
    int m_support_count;
    phys_gjk_geom_id_pair_key m_key;
    unsigned int m_flags;
public:
    enum phys_gjk_cache_info_e
    {
        FLAG_WAS_TOUCHED = (1 << 0),          // 1
        FLAG_IS_SWAPPED = (1 << 1),           // 2
        FLAG_IS_SUPPORT_DIR_VALID = (1 << 2), // 4
        FLAG_IS_SIMPLEX_VALID = (1 << 3),     // 8
    };

    phys_gjk_cache_info() = default;

    inline void update_swapped(const bool swapped)
    {
        if (swapped != (get_flag(FLAG_IS_SWAPPED) ? true : false))
        {
            set_flag(FLAG_IS_SWAPPED, swapped);

            if (get_flag(FLAG_IS_SUPPORT_DIR_VALID))
                m_support_dir = -m_support_dir;

            if (get_flag(FLAG_IS_SIMPLEX_VALID))
            {
                cached_simplex_info temp = m_support_a;
                m_support_a = m_support_b;
                m_support_b = temp;
            }
        }
    }

    const phys_vec3 &get_support_dir() const
    {
        iassert(get_flag(FLAG_IS_SUPPORT_DIR_VALID));
        return m_support_dir;
    }

    void get_simplex(cached_simplex_info *support_a, cached_simplex_info *support_b, int *count)
    {
        iassert(get_flag(FLAG_IS_SIMPLEX_VALID));
        *support_a = m_support_a;
        *support_b = m_support_b;
        *count = m_support_count;
    }

    void set_flag_was_touched()
    {
        set_flag(FLAG_WAS_TOUCHED, true);
    }

    const int get_flag_was_touched()
    {
        return get_flag(FLAG_WAS_TOUCHED);
    }

    void set_support_dir(const phys_vec3 &support_dir)
    {
        set_flag(FLAG_IS_SUPPORT_DIR_VALID, true);
        m_support_dir = support_dir;
    }

    const uint get_flag(const uint f) const
    {
        return m_flags & f;
    }

    void set_flag(const uint f, const bool b)
    {
        b ? m_flags |= f : m_flags &= ~f;
    }

    const int is_support_dir_valid() const
    {
        return get_flag(FLAG_IS_SUPPORT_DIR_VALID);
    }

    const int is_simplex_valid() const
    {
        return get_flag(FLAG_IS_SIMPLEX_VALID);
    }
};

struct phys_heap_gjk_cache_system_avl_tree // sizeof=0x10
{                                       // XREF: broad_phase_memory/r
                                        // gjkcc_info/r
    struct phys_gjk_cache_info_internal : phys_gjk_cache_info // sizeof=0x90
    {
        struct avl_tree_accessor
        {
            using key_type = phys_gjk_geom_id_pair_key;
            using node_type =
                phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal;

            // Lexicographic comparison: first ID1, then ID2
            static bool less(const key_type &key, const node_type *node)
            {
                return (key.m_id1 < node->m_key.m_id1) ||
                    (key.m_id1 == node->m_key.m_id1 && key.m_id2 < node->m_key.m_id2);
            }

            static bool less(const node_type *node, const key_type &key)
            {
                return (node->m_key.m_id1 < key.m_id1) ||
                    (node->m_key.m_id1 == key.m_id1 && node->m_key.m_id2 < key.m_id2);
            }

            static bool equals(const node_type *node, const key_type &key)
            {
                return node->m_key.m_id1 == key.m_id1 &&
                    node->m_key.m_id2 == key.m_id2;
            }

            // Hacky aislop shit
            static void set_key(node_type *node, const key_type &key)
            {
                node->m_key = key;  // uses m_key, not m_avl_key
            }
        };

        phys_inplace_avl_tree_node<phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal> m_avl_tree_node;
        phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal *m_next_gjk_ci;
    };
    static_assert(sizeof(phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal) == 0x90);

    phys_simple_allocator<phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal> m_list_phys_gjk_cache_info_internal;
    phys_inplace_avl_tree<phys_gjk_geom_id_pair_key, phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal, phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal::avl_tree_accessor> m_search_tree;
    int m_max_num_gjk_ci;
    phys_gjk_cache_info_internal *m_list_head;


    ~phys_heap_gjk_cache_system_avl_tree()
    {
        shutdown();
    }

    phys_gjk_cache_info_internal *get_gjk_cache_info(
        unsigned int id1,
        unsigned int id2,
        bool __formal);

    phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal *get_gjk_cache_info_mutex(
        unsigned int id1,
        unsigned int id2,
        struct tlAtomicReadWriteMutex *query_mutex,
        bool __formal);

    // phys_heap_gjk_cache_system_avl_tree::shutdown()
    inline void shutdown()
    {
        phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal *next; // [esp+154h] [ebp-4h]

        while (this->m_list_head)
        {
            next = this->m_list_head->m_next_gjk_ci;

            this->m_search_tree.remove(this->m_list_head->m_key);
            //phys_inplace_avl_tree<phys_gjk_geom_id_pair_key,phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal,phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal::avl_tree_accessor>::remove(
            //    &this->m_search_tree,
            //    &this->m_list_head->m_key);
            this->m_list_phys_gjk_cache_info_internal.free(this->m_list_head);
            //phys_simple_allocator<phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal>::free(
            //    &this->m_list_phys_gjk_cache_info_internal,
            //    this->m_list_head);
            this->m_list_head = next;
        }
        if (this->m_list_phys_gjk_cache_info_internal.m_count)
        {
            if (_tlAssert(
                "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_gjk_cache_system.h",
                260,
                "m_list_phys_gjk_cache_info_internal.get_count() == 0",
                ""))
            {
                __debugbreak();
            }
        }
    }
    void update_cache();
};

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

struct __declspec(align(16)) gjk_query_input // sizeof=0x80
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

struct gjk_collision_visitor // sizeof=0x4
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
    virtual bool is_query()
    {
        return false;
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
    struct rigid_body *rb;
    const phys_mat44 *rb_to_world_xform;
    const phys_mat44 *cg_to_world_xform;
    const phys_mat44 *cg_to_rb_xform;
    unsigned int env_collision_flags;
    const struct broad_phase_environment_query_input *bpeqi;
    struct broad_phase_environement_query_results *bpeqr;
    struct phys_auto_activate_callback *auto_activate_callback;
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
    virtual bool is_query() override // LWSS: re-used as gjk_obb_t::is_polyhedron() in binary
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
    struct broad_phase_environment_info *m_local_bpei;
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
    virtual bool is_query() override // LWSS: re-used in binary as "gjk_obb_t::is_polyhedron()"  ..i.,
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

    static inline bool less(unsigned int key, const generic_avl_map_node_t *node)
    {
        return key < node->m_avl_key;
    }

    static inline bool less(const generic_avl_map_node_t *node, unsigned int key)
    {
        return node->m_avl_key < key;
    }

    static inline bool less(const generic_avl_map_node_t *a, const generic_avl_map_node_t *b)
    {
        return a->m_avl_key < b->m_avl_key;
    }

    // aislop hacky shit
    static inline bool equals(const generic_avl_map_node_t *node, unsigned int key)
    {
        return node->m_avl_key == key;
    }

    static inline void set_key(generic_avl_map_node_t *node, unsigned int key)
    {
        node->m_avl_key = key;
    }
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
        this->m_list.clear();
        this->m_first_hit = 0;
    }
};

struct phys_gjk_geom;

struct phys_gjk_collision_info // sizeof=0x30
{                                       // XREF: phys_gjk_info/r
    phys_vec3 m_p1;
    phys_vec3 m_p2;
    phys_vec3 m_n;
};

struct __declspec(align(8)) phys_gjk_input // sizeof=0x50
{                                       // XREF: ?gjk_trace@@YAXABUgjk_trace_input_t@@PAUlist_gjk_trace_output@@@Z/r
                                        // ?do_initial_tunnel_test@@YAXPAVbroad_phase_group@@ABVbroad_phase_environement_query_results@@@Z/r ...
    phys_vec3 m_cg1_translation;
    phys_vec3 m_cg2_translation;
    float m_start_time;
    float m_end_time;
    //const phys_gjk_geom *gjk_cg1;
    //const phys_gjk_geom *gjk_cg2;
    const gjk_base_t *gjk_cg1; // lwss: I am changing this to the sub-class so I can call more methods.
    const gjk_base_t *gjk_cg2;
    const phys_mat44 *cg1_to_world_xform;
    const phys_mat44 *cg2_to_world_xform;
    phys_gjk_cache_info *gjk_ci;
    float cg1_radius;
    float cg2_radius;
    float m_sep_thresh;
    bool m_intersection_test_only;      // XREF: phys_collide_do_gjk_collide_and_contact_manifold(phys_collision_pair *,phys_gjk_info *,phys_contact_manifold_process *)+10/w
    bool m_continuous_collision;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct phys_gjk_info // sizeof=0x3A0
{                                       // XREF: ?gjk_trace@@YAXABUgjk_trace_input_t@@PAUlist_gjk_trace_output@@@Z/r
    enum gjk_retval_e : __int32
    {                                       // XREF: ?phys_collide_do_gjk_collide@phys_gjk_info@@QAE?B_NPBVphys_gjk_input@@@Z/r
        GJK_SEPARATED   = 0x0,
        GJK_VALID       = 0x1,
        GJK_PENETRATING = 0x2,
        GJK_INVALID     = 0x3,
    };
    struct phys_gjk_set_info // sizeof=0x14
    {                                       // XREF: phys_gjk_info/r
        float m_lamda[4];
        int m_candidate;
    };

    phys_gjk_info()
    {
        memset(this, 0, sizeof(*this)); // LWSS ADD
    }

    gjk_retval_e gjk(const phys_gjk_input *d, const phys_vec3 *initial_support_dir, bool in_separation_loop);

    int seed_simplex(int cached_vert_count);
    int gjk_subalgorithm(int w_set, int new_index);
    gjk_retval_e gjk_ray_cast(
        const phys_gjk_input *d,
        const phys_vec3 *initial_support_dir,
        bool in_separation_loop);
    gjk_retval_e collide(const phys_gjk_input *d);
    char phys_collide_do_gjk_collide(const phys_gjk_input *d);
    int compress_verts(char w_set);
    void comp_closest_points(int w_set, phys_vec3 *a, phys_vec3 *b);
    void gjk_cache_update_invalid(const phys_gjk_input *d);
    const phys_vec3 *get_initial_support_dir(phys_vec3 *result, const phys_gjk_input *d);
    void gjk_cache_update_test_only_valid(const phys_gjk_input *d);
    void gjk_cache_update_colliding(const phys_gjk_input *d);
    void gjk_cache_update_test_only_penetrating(const phys_gjk_input *d);
    void comp_v(int w_set, phys_vec3 *v) const;
    int init_gjk(
        const phys_gjk_input *d,
        const phys_vec3 *initial_support_dir,
        bool in_separation_loop);

    phys_mat44 cg2_to_cg1_xform;
    phys_vec3 m_cg1_relative_translation_loc;
    float m_continuous_collision_lambda;
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
    phys_gjk_collision_info cg1_cinfo_loc;
    phys_vec3 m_gjk_origin;
    phys_vec3 m_w_verts[4];
    phys_vec3 m_a_verts[4];
    phys_vec3 m_b_verts[4];
    phys_vec3 m_a_inds[4];
    phys_vec3 m_b_inds[4];
    phys_vec3 m_support_dir;
    float m_geom_radii_sum;
    int m_cc_reset_iter;
    int m_flags; // 8 = use_ray_cast?
    int m_w_set;
    int m_last_w_set;
    int m_gjk_iter;
    float m_gjk_sep_thresh;
    float m_gjk_pen_thresh_sq;
    float m_upper_dist_sq;
    float m_lower_dist_sq;
    float m_dot_ij[4][4]; // unused
    phys_gjk_info::phys_gjk_set_info m_set_list[16];
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct gjk_unique_id_database_t // sizeof=0x4
{                                                                             // XREF: .data:gjk_unique_id_database_t g_gjk_unique_id_database/r
    volatile unsigned int m_counter;
    unsigned int get_unique_id();
};

// local variable allocation has failed, the output may be wrong!
void    phys_full_inv_multiply_mat(
                phys_mat44 *dest,
                const phys_mat44 *left,
                const phys_mat44 *right);
void __cdecl get_simplex(
                const phys_gjk_geom *cg1,
                const phys_gjk_geom *cg2,
                phys_gjk_cache_info *gjk_ci,
                phys_vec3 *a_verts,
                phys_vec3 *a_inds,
                phys_vec3 *b_verts,
                phys_vec3 *b_inds,
                int *vert_count);
void __cdecl set_simplex(
                const phys_gjk_geom *cg1,
                const phys_gjk_geom *cg2,
                phys_gjk_cache_info *gjk_ci,
                const phys_vec3 *a_normal,
                const phys_vec3 *b_normal,
                const phys_vec3 *a_inds,
                const phys_vec3 *b_inds,
                int w_set);
int    backup1(phys_gjk_info *gjk_info, int new_index, bool seed_simplex);

void __cdecl setup_gjk_input_from_pcp(phys_gjk_input *pgi, struct phys_collision_pair *pcp);

phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal *get_gjk_cache_info(
    phys_heap_gjk_cache_system_avl_tree *gjk_cache,
    const gjk_base_t *cg1,
    const gjk_base_t *cg2);

extern create_gjk_geom_collision_visitor g_empty_collision_visitor;
extern gjk_unique_id_database_t g_gjk_unique_id_database;