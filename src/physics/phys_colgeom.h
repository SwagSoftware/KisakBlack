#pragma once

#include "phys_local.h"
#include <universal/q_shared.h>
#include <qcommon/cm_trace.h>
#include <qcommon/statmonitor.h>
#include <tl/tl_system.h>

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

struct __declspec(align(16)) contact_manifold_mesh_point // sizeof=0x20
{
        phys_vec3 m_p;
        phys_vec2 m_contact_p;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
};

struct phys_contact_manifold;

struct phys_gjk_geom // sizeof=0x4
{
    // Find the Furthest point in a shape along the given direction
    virtual void support(const phys_vec3 &dir, phys_vec3 *, phys_vec3 *) const = 0;
    virtual void get_simplex(const struct cached_simplex_info *, const int, phys_vec3 *, phys_vec3 *) const = 0;
    virtual void set_simplex(const phys_vec3 *, const int, const phys_vec3 *, cached_simplex_info *) const;
    virtual const phys_vec3 *get_center(phys_vec3 *result) const = 0;
    virtual void get_feature(struct phys_contact_manifold *) const = 0;
    virtual float get_geom_radius() const
    {
        return 0.0f;
    }

    virtual void calc_aabb(const phys_mat44 *, phys_vec3 *, phys_vec3 *) const = 0;
    // ray_cast() seems to be a weird leftover (unused)
    virtual bool ray_cast(const phys_vec3 *, const phys_vec3 *, const float, float *, phys_vec3 *) const;
    // false for rounded shapes (cylinder, capsule)
    virtual bool is_polyhedron() = 0;

    const phys_vec3 *support_only(const phys_vec3 *result, const phys_mat44 *xform, const phys_vec3 *v) const;
};

//enum $2DE106F947F3758108E2812F8595AD24 : __int32
enum gjk_type_t : __int32 // LWSS: not a real enum name!
{
    GJK_BASE             = 0x0,
    GJK_AABB             = 0x1,
    GJK_BRUSH            = 0x2,
    GJK_PARTITION        = 0x3,
    GJK_DOUBLE_SPHERE    = 0x4,
    GJK_CYLINDER         = 0x5,
    GJK_OBB              = 0x6,
    GJK_POLYGON_CYLINDER = 0x7,
};

// LWSS ADD
inline const char *GjkTypeToString(gjk_type_t type)
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
// LWSS END

struct gjk_base_t : public phys_gjk_geom // sizeof=0x50
{                                                                             // XREF: gjk_brush_t/r
public:
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
        phys_vec3 m_aabb_mn_loc;
        phys_vec3 m_aabb_mx_loc;
        unsigned int m_gjk_geom_id;
        const phys_mat44 *m_xform_;
        gjk_base_t *m_next_geom;
        unsigned int m_flags;
        int stype;
        int m_contents;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte

        static const int FLAG_TEMP_ALLOCATION = 1;
        static const int FLAG_GEOM_ID_VALID = 4;
        static const int FLAG_XFORM_VALID = 8;
        static const int FLAG_CONTENTS_VALID = 16;

        inline int get_flag(int flag) const
        {
            return m_flags & flag;
        }

        // inferred from asserts
        inline void check_aabb_valid()
        {
            iassert(get_flag(FLAG_AABB_LOC_VALID));
        }

        virtual ~gjk_base_t()
        {
            iassert(!get_flag(FLAG_TEMP_ALLOCATION));
        }

        gjk_base_t() : phys_gjk_geom()
        {
            this->m_flags = 0;
        }

        const phys_vec3 *get_aabb_mx() const
        {
            iassert(get_flag(FLAG_AABB_LOC_VALID));
            return &this->m_aabb_mx_loc;
        }
        const phys_vec3 *get_aabb_mn() const
        {
            iassert(get_flag(FLAG_AABB_LOC_VALID));
            return &this->m_aabb_mn_loc;
        }

        unsigned int get_geom_id() const;
        const phys_mat44 *get_xform();

        int get_contents();
        void set_contents(int contents);

        void set_geom_id_new(unsigned int geom_id);
        void set_xform(const phys_mat44 *xform);

        // support() - PURE
        // get_simplex() - PURE
        // set_simplex() - phys_gjk_geom
        // get_center() - PURE
        // get_feature() - PURE
        // get_geom_radius() - phys_gjk_geom
        // calc_aabb() - PURE
        // ray_cast() - phys_gjk_geom
        // is_polyhedron() - PURE
        ////////////////////////
        virtual void comp_aabb_loc();
        virtual gjk_type_t get_type() const
        {
            iassert(0);
            return GJK_BASE;
        }
        virtual bool is_foot(const phys_vec3 *hit_point) const;
        virtual bool is_walkable(const phys_vec3 *hit_point, const phys_vec3 *up);
        virtual const cbrush_t *get_brush() const;
};


struct __declspec(align(16)) gjk_aabb_t : gjk_base_t // sizeof=0x80
{
        phys_vec3 m_center_local;
        phys_vec3 m_dims;
        const struct cbrush_t *m_brush;
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

        static gjk_aabb_t *__cdecl create(
                const phys_vec3 *center,
                const phys_vec3 *dims,
                int stype,
                gjk_collision_visitor *allocator);
        static void __cdecl destroy(gjk_aabb_t *geom);

        virtual void support(const phys_vec3 &dir, phys_vec3 *support_vert, phys_vec3 *support_ind) const override;
        virtual void get_simplex(const cached_simplex_info *cache_info, int index_count, phys_vec3 *simplex_verts, phys_vec3 *simplex_inds) const override;
        // set_simplex() - phys_gjk_geom
        virtual const phys_vec3 * get_center(phys_vec3 *result) const override;
        virtual void get_feature(phys_contact_manifold *cman) const override;
        // get_geom_radius() - phys_gjk_geom
        virtual void calc_aabb(const phys_mat44 *xform, phys_vec3 *aabb_min, phys_vec3 *aabb_max) const override;
        // ray_cast() - phys_gjk_geom
        virtual bool is_polyhedron() override // technically re-used gjk_obb_t::is_polyhedron()
        {
            return true;
        }
        // comp_aabb_loc() - gjk_base_t
        virtual gjk_type_t get_type() const override
        {
            return GJK_AABB;
        }
        // is_foot() - gjk_base_t
        virtual bool is_walkable(const phys_vec3 *hit_point, const phys_vec3 *up) override;
        virtual const cbrush_t *get_brush() const override;
};
static_assert(sizeof(gjk_aabb_t) == 128);

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

struct gjk_obb_t : gjk_base_t // sizeof=0xA0
{
        phys_vec3 m_dims;
        phys_mat44 m_xform;

        static gjk_obb_t *__cdecl create(
            const phys_mat44 *xform,
            const phys_vec3 *dims,
            int stype,
            gjk_collision_visitor *allocator);
        static void __cdecl destroy(gjk_obb_t *geom);

        // support() - PURE
        // get_simplex() - PURE
        // set_simplex() - phys_gjk_geom
        // get_center() - PURE
        // get_feature() - PURE
        // get_geom_radius() - phys_gjk_geom
        // calc_aabb() - PURE
        // ray_cast() - phys_gjk_geom
        // is_polyhedron() - PURE
        ////////////////////////
        //virtual void comp_aabb_loc();
        //virtual gjk_type_t get_type() const
        //{
        //    iassert(0);
        //    return GJK_BASE;
        //}
        //virtual bool is_foot(const phys_vec3 *hit_point) const;
        //virtual bool is_walkable(const phys_vec3 *hit_point, const phys_vec3 *up);
        //virtual const cbrush_t *get_brush() const;

        virtual void support(const phys_vec3 &dir, phys_vec3 *support_vert, phys_vec3 *support_ind) const override;
        virtual void get_simplex(const cached_simplex_info *cache_info, int index_count, phys_vec3 *simplex_verts, phys_vec3 *simplex_inds) const override;
        // set_simplex - phys_gjk_geom
        virtual const phys_vec3 *get_center(phys_vec3 *result) const override;
        virtual void get_feature(phys_contact_manifold *cman) const override;
        // get_geom_radius() - phys_gjk_geom
        virtual void calc_aabb(const phys_mat44 *xform, phys_vec3 *aabb_min, phys_vec3 *aabb_max) const override;
        // ray_cast() - phys_gjk_geom
        virtual bool is_polyhedron() override
        {
            return true;
        }
        // comp_aabb_loc() - gjk_base_t
        virtual gjk_type_t get_type() const override
        {
            return GJK_OBB;
        }
        // is_foot() - gjk_base_t
        // is_walkable() - gjk_base_t
        // get_brush() - gjk_base_t
};
static_assert(sizeof(gjk_obb_t) == 160);

struct __declspec(align(8)) gjk_brush_t : gjk_base_t // sizeof=0x60
{
        int nverts;
        const float (*verts)[3];
        const cbrush_t *brush;
        // padding byte
        // padding byte
        // padding byte
        // padding byte

        static gjk_brush_t *create(
            const cbrush_t *brush,
            int stype,
            gjk_collision_visitor *allocator);
        static void __cdecl destroy(gjk_brush_t *geom);

        // support() - PURE
        // get_simplex() - PURE
        // set_simplex() - phys_gjk_geom
        // get_center() - PURE
        // get_feature() - PURE
        // get_geom_radius() - phys_gjk_geom
        // calc_aabb() - PURE
        // ray_cast() - phys_gjk_geom
        // is_polyhedron() - PURE
        ////////////////////////
        //virtual void comp_aabb_loc();
        //virtual gjk_type_t get_type() const
        //{
        //    iassert(0);
        //    return GJK_BASE;
        //}
        //virtual bool is_foot(const phys_vec3 *hit_point) const;
        //virtual bool is_walkable(const phys_vec3 *hit_point, const phys_vec3 *up);
        //virtual const cbrush_t *get_brush() const;

        virtual void support(const phys_vec3 &dir, phys_vec3 *support_vert, phys_vec3 *support_ind) const override;
        virtual void get_simplex(const cached_simplex_info *cache_info, int index_count, phys_vec3 *simplex_verts, phys_vec3 *simplex_inds) const override;
        // set_simplex() - phys_gjk_geom
        virtual const phys_vec3 *get_center(phys_vec3 *result) const override;
        virtual void get_feature(phys_contact_manifold *cman) const override;
        // get_geom_radius() - phys_gjk_geom
        virtual void calc_aabb(const phys_mat44 *xform, phys_vec3 *aabb_min, phys_vec3 *aabb_max) const override;
        // ray_cast() - phys_gjk_geom
        virtual bool is_polyhedron() override
        {
            return true;
        }
        // comp_aabb_loc() - gjk_base_t
        virtual gjk_type_t get_type() const override
        {
            return GJK_BRUSH;
        }
        // is_foot() - gjk_base_t
        virtual bool is_walkable(const phys_vec3 *hit_point, const phys_vec3 *up) override;
        virtual const cbrush_t *get_brush() const override;
};
static_assert(sizeof(gjk_brush_t) == 96);

struct CollisionBorder // sizeof=0x1C
{
        float distEq[3];
        float zBase;
        float zSlope;
        float start;
        float length;
};

struct CollisionPartition // sizeof=0x14
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

struct __declspec(align(16)) gjk_partition_t : gjk_base_t // sizeof=0x70
{
        int ninds;
        const float (*verts)[3];
        unsigned __int16 *inds;
        const CollisionPartition *partition;
        const CollisionAabbTree *tree;
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

        static gjk_partition_t *__cdecl create(const CollisionAabbTree *tree, gjk_collision_visitor *allocator);
        static void __cdecl destroy(gjk_partition_t *geom);

        // support() - PURE
        // get_simplex() - PURE
        // set_simplex() - phys_gjk_geom
        // get_center() - PURE
        // get_feature() - PURE
        // get_geom_radius() - phys_gjk_geom
        // calc_aabb() - PURE
        // ray_cast() - phys_gjk_geom
        // is_polyhedron() - PURE
        ////////////////////////
        //virtual void comp_aabb_loc();
        //virtual gjk_type_t get_type() const
        //{
        //    iassert(0);
        //    return GJK_BASE;
        //}
        //virtual bool is_foot(const phys_vec3 *hit_point) const;
        //virtual bool is_walkable(const phys_vec3 *hit_point, const phys_vec3 *up);
        //virtual const cbrush_t *get_brush() const;

        virtual void support(const phys_vec3 &dir, phys_vec3 *support_vert, phys_vec3 *support_ind) const override;
        virtual void get_simplex(const cached_simplex_info *cache_info, int index_count, phys_vec3 *simplex_verts, phys_vec3 *simplex_inds) const override;
        // set_simplex() - phys_gjk_geom
        virtual const phys_vec3 *get_center(phys_vec3 *result) const override // This is technically "gjk_brush_t::get_center()"
        {
            result->x = 0.0f;
            result->y = 0.0f;
            result->z = 0.0f;
            return result;
        }
        virtual void get_feature(phys_contact_manifold *cman) const override;
        // get_geom_radius() - phys_gjk_geom
        virtual void calc_aabb(const phys_mat44 *xform, phys_vec3 *aabb_min, phys_vec3 *aabb_max) const override;
        // ray_cast() - phys_gjk_geom
        virtual bool is_polyhedron() override
        {
            return true;
        }
        // comp_aabb_loc() - gjk_base_t
        virtual gjk_type_t get_type() const override
        {
            return GJK_PARTITION;
        }
        // is_foot() - gjk_base_t
        virtual bool is_walkable(const phys_vec3 *hit_point, const phys_vec3 *up) override;
        // get_brush() - gjk_base_t
};
// Itanium (GCC/Clang) reuses the polymorphic base gjk_base_t's tail padding for
// this derived class, so it is 96 bytes here vs 112 under MSVC. gjk_partition_t is
// a runtime-only physics object (never serialized from a fastfile), so its exact
// size only has to match the original binary under MSVC.
#ifdef _MSC_VER
static_assert(sizeof(gjk_partition_t) == 112);
#endif

struct gjk_double_sphere_t : gjk_base_t // sizeof=0x90
{
        phys_vec3 m_list_center[2];
        phys_vec3 m_center;
        float m_list_radius[2];
        float m_geom_radius;
        int m_count;

        gjk_double_sphere_t() = default;

        static gjk_double_sphere_t *__cdecl create(
            const phys_vec3 *c0,
            const phys_vec3 *c1,
            float r,
            int stype,
            gjk_collision_visitor *allocator);
        static void __cdecl destroy(gjk_double_sphere_t *geom);

        // support() - PURE
        // get_simplex() - PURE
        // set_simplex() - phys_gjk_geom
        // get_center() - PURE
        // get_feature() - PURE
        // get_geom_radius() - phys_gjk_geom
        // calc_aabb() - PURE
        // ray_cast() - phys_gjk_geom
        // is_polyhedron() - PURE
        ////////////////////////
        //virtual void comp_aabb_loc();
        //virtual gjk_type_t get_type() const
        //{
        //    iassert(0);
        //    return GJK_BASE;
        //}
        //virtual bool is_foot(const phys_vec3 *hit_point) const;
        //virtual bool is_walkable(const phys_vec3 *hit_point, const phys_vec3 *up);
        //virtual const cbrush_t *get_brush() const;

        virtual void support(const phys_vec3 &dir, phys_vec3 *support_vert, phys_vec3 *support_ind) const override;
        virtual void get_simplex(const cached_simplex_info *cache_info, int index_count, phys_vec3 *simplex_verts, phys_vec3 *simplex_inds) const override;
        virtual void set_simplex(const phys_vec3 *simplex_inds, int w_set, const phys_vec3 *normal, cached_simplex_info *cache_info) const override;
        virtual const phys_vec3 *get_center(phys_vec3 *result) const override;
        virtual void get_feature(phys_contact_manifold *cman) const override;
        virtual float get_geom_radius() const override;
        virtual void calc_aabb(const phys_mat44 *xform, phys_vec3 *aabb_min, phys_vec3 *aabb_max) const override;
        // ray_cast() - phys_gjk_geom
        virtual bool is_polyhedron() override;
        // comp_aabb_loc() - gjk_base_t
        virtual gjk_type_t get_type() const override
        {
            return GJK_DOUBLE_SPHERE;
        }
        virtual bool is_foot(const phys_vec3 *hit_point) const override;
        // is_walkable() - gjk_base_t
        // get_brush() - gjk_base_t
};
static_assert(sizeof(gjk_double_sphere_t) == 144);

struct gjk_cylinder_t : gjk_base_t // sizeof=0xA0
{
        int direction;
        float halfHeight;
        float radius;
        float m_geom_radius;
        phys_mat44 xform;

        static gjk_cylinder_t *__cdecl create(
            int _direction,
            float _halfHeight,
            float _radius,
            const phys_mat44 *_xform,
            int stype,
            gjk_collision_visitor *allocator);
        static void __cdecl destroy(gjk_cylinder_t *geom);

        // support() - PURE
        // get_simplex() - PURE
        // set_simplex() - phys_gjk_geom
        // get_center() - PURE
        // get_feature() - PURE
        // get_geom_radius() - phys_gjk_geom
        // calc_aabb() - PURE
        // ray_cast() - phys_gjk_geom
        // is_polyhedron() - PURE
        ////////////////////////
        //virtual void comp_aabb_loc();
        //virtual gjk_type_t get_type() const
        //{
        //    iassert(0);
        //    return GJK_BASE;
        //}
        //virtual bool is_foot(const phys_vec3 *hit_point) const;
        //virtual bool is_walkable(const phys_vec3 *hit_point, const phys_vec3 *up);
        //virtual const cbrush_t *get_brush() const;

        virtual void support(const phys_vec3 &dir, phys_vec3 *support_vert, phys_vec3 *support_ind) const override;
        virtual void get_simplex(const cached_simplex_info *cache_info, int index_count, phys_vec3 *simplex_verts, phys_vec3 *simplex_inds) const override;
        // set_simplex() - phys_gjk_geom
        virtual const phys_vec3 *get_center(phys_vec3 *result) const override;
        virtual void get_feature(phys_contact_manifold *cman) const override;
        virtual float get_geom_radius() const override;
        virtual void calc_aabb(const phys_mat44 *xform_, phys_vec3 *aabb_min, phys_vec3 *aabb_max) const override;
        // ray_cast() - phys_gjk_geom
        virtual bool is_polyhedron() override
        {
            return false;
        }
        // comp_aabb_loc() - gjk_base_t
        virtual gjk_type_t get_type() const override
        {
            return GJK_CYLINDER;
        }
        virtual bool is_foot(const phys_vec3 *hit_point) const override;
        // is_walkable() - gjk_base_t
        // get_brush() - gjk_base_t


        const phys_vec3 * get_dims(phys_vec3 *result) const;
};
static_assert(sizeof(gjk_cylinder_t) == 160);

struct __declspec(align(8)) gjk_polygon_cylinder_t : gjk_base_t // sizeof=0x80
{                                                                             // XREF: ?render_gjkcc_collision@@YAXAAY02$$CBM00@Z/r
        phys_vec3 m_center;
        float m_polygon_cylinder_radius;
        float m_capsule_radius;
        float m_half_height;
        float m_geom_radius;
        float m_head_offset;
        float m_foot_offset;
        int m_mode; // mode specifies capsule or cylinder mode. Capsule has 2 spheres like it would                                                 // XREF: render_gjkcc_collision(float const (&)[3],float const (&)[3],float const (&)[3])+10/w
        // padding byte
        // padding byte
        // padding byte
        // padding byte

        struct poly_verts // sizeof=0x20
        {                                                                             // XREF: .data:poly_verts s_poly_verts/r
                float m_co[4];
                float m_si[4];

                poly_verts();

                static constexpr int NUM_VERTS_ = 12;
                static constexpr int VERTS_PER_FACE = 3;

                inline void get_co_si(int i, float *co_, float *si_)
                {
                    iassert(i >= 0 && i < NUM_VERTS_);

                    // The 12 vertices are split into 4 quadrants of 3 verts each.
                    // Each quadrant has a sign flip for co and si, and a base offset
                    // into the m_co/m_si arrays with alternating step direction.
                    static const int   a[] = { 0,  6, -6, 12 };
                    static const int   b[] = { 1, -1,  1, -1 };
                    static const float cos_[] = { 1.0f, -1.0f, -1.0f,  1.0f };
                    static const float sin_[] = { 1.0f,  1.0f, -1.0f, -1.0f };

                    int quadrant = i / VERTS_PER_FACE;
                    int ii = a[quadrant] + i * b[quadrant];

                    *co_ = cos_[quadrant] * this->m_co[ii];
                    *si_ = sin_[quadrant] * this->m_si[ii];
                }

                inline int support(const phys_vec3 *v)
                {
                    float at; // [esp+Ch] [ebp-10h]

                    at = atan2(v->y, v->x);
                    if (at < 0.0)
                        at += (float)(2.0 * 3.1415927);

                    int best_i = (int)(float)((float)((float)(12.0 * at) / (float)(2.0 * 3.1415927)) + 0.5) % 12;

                    iassert(best_i >= 0 && best_i < NUM_VERTS_);
                    return best_i;
                }
        };

        static poly_verts s_poly_verts;

        static gjk_polygon_cylinder_t *__cdecl create(
                float (*mins)[3],
                float (*maxs)[3],
                float radius_adjust,
                int stype,
                gjk_collision_visitor *allocator);
        static void __cdecl destroy(gjk_polygon_cylinder_t *geom);


        // support() - PURE
        // get_simplex() - PURE
        // set_simplex() - phys_gjk_geom
        // get_center() - PURE
        // get_feature() - PURE
        // get_geom_radius() - phys_gjk_geom
        // calc_aabb() - PURE
        // ray_cast() - phys_gjk_geom
        // is_polyhedron() - PURE
        ////////////////////////
        //virtual void comp_aabb_loc();
        //virtual gjk_type_t get_type() const
        //{
        //    iassert(0);
        //    return GJK_BASE;
        //}
        //virtual bool is_foot(const phys_vec3 *hit_point) const;
        //virtual bool is_walkable(const phys_vec3 *hit_point, const phys_vec3 *up);
        //virtual const cbrush_t *get_brush() const;

        virtual void support(const phys_vec3 &dir, phys_vec3 *support_vert, phys_vec3 *support_ind) const override;
        virtual void get_simplex(const cached_simplex_info *cache_info, int index_count, phys_vec3 *simplex_verts, phys_vec3 *simplex_inds) const override;
        //set_simplex() - phys_gjk_geom
        virtual const phys_vec3 *get_center(phys_vec3 *result) const override// This is "gjk_aabb_t::get_center()" in the decomp,
        {
            *result = this->m_center;
            return result;
        }
        virtual void get_feature(phys_contact_manifold *) const override 
        {
            iassert(0); // should be empty func, just curious if this gets called
        }
        virtual float get_geom_radius() const override;
        virtual void calc_aabb(const phys_mat44 *xform, phys_vec3 *aabb_min, phys_vec3 *aabb_max) const override;
        // ray_cast() - phys_gjk_geom
        virtual bool is_polyhedron() override
        {
            return true;
        }
        // comp_aabb_loc() - gjk_base_t
        virtual gjk_type_t get_type() const override
        {
            return GJK_POLYGON_CYLINDER;
        }
        virtual bool is_foot(const phys_vec3 *hit_point) const override;
        // is_walkable() - gjk_base_t
        // get_brush() - gjk_base_t

        //gjk_polygon_cylinder_t::poly_verts gjk_polygon_cylinder_t::s_poly_verts
};
static_assert(sizeof(gjk_polygon_cylinder_t) == 128);

struct gjk_geom_list_t // sizeof=0x8
{
    gjk_base_t *m_first_geom;
    int m_geom_count;
public:
    inline int get_geom_count()
    {
        if (this->m_geom_count < 0
            && _tlAssert(
                "c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.h",
                1036,
                "m_geom_count >= 0",
                ""))
        {
            __debugbreak();
        }
        return this->m_geom_count;
    }

    inline void add_geom(gjk_base_t *geom)
    {
        if (!geom && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.h", 1029, 0, "%s", "geom"))
            __debugbreak();
        geom->m_next_geom = this->m_first_geom;
        this->m_first_geom = geom;
        ++this->m_geom_count;
    }

    inline void comp_aabb_loc(
        phys_vec3 *aabb_mn_loc,
        phys_vec3 *aabb_mx_loc)
    {
        {
            iassert(m_first_geom);
            this->m_first_geom->comp_aabb_loc();

            m_first_geom->check_aabb_valid();

            *aabb_mn_loc = m_first_geom->m_aabb_mn_loc;

            this->m_first_geom->check_aabb_valid();

            *aabb_mx_loc = this->m_first_geom->m_aabb_mx_loc;

            for (gjk_base_t *i = this->m_first_geom->m_next_geom; i; i = i->m_next_geom)
            {
                i->comp_aabb_loc();

                i->check_aabb_valid();

                phys_vec3 min; // [esp-38h] [ebp-7Ch] BYREF
                phys_min(&min, aabb_mn_loc, &i->m_aabb_mn_loc);
                aabb_mn_loc->x = min.x;
                aabb_mn_loc->y = min.y;
                aabb_mn_loc->z = min.z;

                i->check_aabb_valid();

                phys_vec3 max; // [esp-68h] [ebp-ACh] BYREF
                phys_max(&max, aabb_mx_loc, &i->m_aabb_mx_loc);
                aabb_mx_loc->x = max.x;
                aabb_mx_loc->y = max.y;
                aabb_mx_loc->z = max.z;
            }
        }
    }
};


PhysGeomList *__cdecl xmodel_get_geomlist(const XModel *model, int bone_index);

void    phys_aabb_add_point(
        const phys_mat44 *xform,
        const phys_vec3 *point,
        phys_vec3 *aabb_min,
        phys_vec3 *aabb_max);

void    phys_aabb_init_sphere(
                const phys_mat44 *xform,
                const phys_vec3 *center,
                float radius,
                phys_vec3 *aabb_min,
                phys_vec3 *aabb_max);
void    phys_aabb_add_sphere(
                const phys_mat44 *xform,
                const phys_vec3 *center,
                float radius,
                phys_vec3 *aabb_min,
                phys_vec3 *aabb_max);


void    setup_gjk_polygon_cylinder(
                float *mins,
                float *maxs,
                float radius_adjust,
                gjk_polygon_cylinder_t *gjk_cylinder);

gjk_aabb_t * create_aabb_gjk_geom(
                const float *mn,
                const float *mx,
                int stype,
                gjk_collision_visitor *allocator);
gjk_obb_t * create_obb_gjk_geom(
                float (*orientation)[3],
                float *offset,
                float *halfLengths,
                int stype,
                gjk_collision_visitor *allocator);
gjk_brush_t *__cdecl create_brush_gjk_geom(const cbrush_t *brush, int stype, gjk_collision_visitor *allocator);
gjk_double_sphere_t * create_capsule_gjk_geom(
                float *center,
                float radius,
                float halfHeight,
                unsigned int direction,
                int stype,
                gjk_collision_visitor *allocator);
gjk_cylinder_t * create_cylinder_gjk_geom(
                float (*rot)[3],
                float *trans,
                float radius,
                float halfHeight,
                int stype,
                gjk_collision_visitor *allocator);
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

void __cdecl destroy_gjk_geom(gjk_geom_list_t *gjk_geom_list);


//extern create_gjk_geom_collision_visitor g_empty_collision_visitor;
//extern gjk_unique_id_database_t g_gjk_unique_id_database;