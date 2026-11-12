#pragma once

// The rigid-body library is TreyBO (submodule, <repo>/TreyBO). This header pulls it in for the game and
// holds the game-owned pieces that sit on top of it.

#include <universal/assertive.h>
#include <cstring>
#include <cmath>

#include <Windows.h>

#include "tl/base/include/tl_defs.h"
#include "tl/base/include/tl_system.h"
#include "tl/base/include/tl_thread.h"

#include "tl/physics/include/phys_base.h"
#include "tl/physics/include/phys_math.h"
#include "tl/physics/include/phys_assert.h"
#include "tl/physics/include/phys_mem.h"
#include "tl/physics/include/phys_mutex.h"
#include "tl/physics/include/phys_link_list.h"
#include "tl/physics/include/phys_avl_tree.h"
#include "tl/physics/include/phys_hash_table.h"
#include "tl/physics/include/phys_transient_allocator.h"
#include "tl/physics/include/phys_util.h"
#include "tl/physics/include/rigid_body.h"
#include "tl/physics/include/rigid_body_internal.h"
#include "tl/physics/include/physics_system.h"
#include "tl/physics/include/physics_system_internal.h"
#include "tl/physics/include/rb_vehicle_model.h"
#include "tl/physics/include/rbc_defs/rbc_def_base.h"
#include "tl/physics/include/rbc_defs/rbc_def_contact.h"
#include "tl/physics/include/rbc_defs/rbc_def_generic.h"
#include "tl/physics/include/rbc_defs/rbc_def_ragdoll.h"
#include "tl/physics/include/rbc_defs/rbc_def_vehicle.h"
#include "tl/physics/include/rbc_defs/rbc_def_custom.h"
#include "tl/physics/include/collision/phys_bounding_volume.h"
#include "tl/physics/include/collision/phys_environment.h"
#include "tl/physics/include/collision/phys_gjk.h"
#include "tl/physics/include/collision/phys_gjk_cache_system.h"
#include "tl/physics/include/collision/phys_gjk_geom.h"
#include "tl/physics/include/collision/phys_convex_hull.h"

static const float PHYS_PI = PHYS_PI_;

// ---------------------------------------------------------------------------------------------
// Game-owned types
// ---------------------------------------------------------------------------------------------

struct __declspec(align(16)) plane_lt // sizeof=0x20
{
    phys_vec3 n;
    float d;
};

struct chull_t // sizeof=0x20
{
    unsigned int key;
    int nverts;
    phys_vec3 *verts;
    int ninds;
    unsigned __int16 *inds;
    chull_t *next;
    int touched;
    chull_t *next_list;
};

struct __declspec(align(16)) cached_query_info_t // sizeof=0x30
{
    phys_vec3 m_query_aabb_min;
    phys_vec3 m_query_aabb_max;
    int m_query_contents;
    unsigned int m_query_flags;

    inline bool is_empty()
    {
        if (this->m_query_contents)
            return false;

        iassert(m_query_flags == 0);
        return true;
    }

    bool aabb_is_valid();
    void init_query(
        const phys_vec3 *query_aabb_min,
        const phys_vec3 *query_aabb_max,
        const phys_vec3 *extra,
        int query_contents,
        unsigned int query_flags);

    void add_query(
        const phys_vec3 *query_aabb_min,
        const phys_vec3 *query_aabb_max,
        const phys_vec3 *extra,
        int query_contents,
        unsigned int query_flags);
    bool is_subset(
        const phys_vec3 *query_aabb_min,
        const phys_vec3 *query_aabb_max,
        int query_contents,
        unsigned int query_flags);
    bool is_subset_aabb(
        const phys_vec3 *query_aabb_min,
        const phys_vec3 *query_aabb_max);
};

struct geom_plane // sizeof=0x30
{
    phys_vec3 m_normal;
    phys_vec3 m_arm;
    float m_d;
    float m_lambda;
    float m_right_side;
    bool m_active;
    bool m_walkable;
    bool m_no_push_out;
};

inline void assert_mat44_initialized(const phys_mat44 &m)
{
    const unsigned int UNINIT = 0xCCCCCCCC;
    const unsigned int *u = reinterpret_cast<const unsigned int *>(&m);
    iassert(u[0] != UNINIT); //x.x
    iassert(u[1] != UNINIT); //x.y
    iassert(u[2] != UNINIT); //x.z

    iassert(u[4] != UNINIT); //y.x
    iassert(u[5] != UNINIT); //y.y
    iassert(u[6] != UNINIT); //y.z

    iassert(u[8] != UNINIT); //z.x
    iassert(u[9] != UNINIT); //z.y
    iassert(u[10] != UNINIT);//z.z

    iassert(u[12] != UNINIT);//w.x
    iassert(u[13] != UNINIT);//w.y
    iassert(u[14] != UNINIT);//w.z
}

inline const phys_vec3 &get_mat_wrow(const phys_mat44 &mat)
{
    return mat.w;
}

inline bool CrazyFloat(float f) // lwss add
{
    return !(f < 50000.0f && f > -50000.0f);
}

