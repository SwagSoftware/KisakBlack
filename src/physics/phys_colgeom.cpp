#include "phys_colgeom.h"

#include <xanim/xmodel_utils.h>

#include <new>
#include <cgame/cg_world.h>
#include <qcommon/cm_load.h>
#include <bgame/bg_slidemove.h>
#include <tl/tl_system.h>
#include <qcommon/dobj_management.h>
#include <xanim/dobj_utils.h>
#include <clientscript/scr_const.h>
#include <DynEntity/DynEntity_client.h>
#include "phys_contact_manifold.h"
#include "phys_main.h"
#include <glass/glass.h>

phys_simple_allocator<gjk_aabb_t> aabb_pool;
phys_simple_allocator<gjk_obb_t> obb_pool;
phys_simple_allocator<gjk_brush_t> brush_pool;
phys_simple_allocator<gjk_partition_t> partition_pool;
phys_simple_allocator<gjk_double_sphere_t> double_sphere_pool;
phys_simple_allocator<gjk_cylinder_t> cylinder_pool;
phys_simple_allocator<gjk_polygon_cylinder_t> polygon_cylinder_pool;

gjk_unique_id_database_t g_gjk_unique_id_database{ 1 };

static const phys_vec3 g_phys_vec3_box_sgn[8] =
{
    {  1.0f,  1.0f,  1.0f },
    {  1.0f,  1.0f, -1.0f },
    {  1.0f, -1.0f,  1.0f },
    {  1.0f, -1.0f, -1.0f },
    { -1.0f,  1.0f,  1.0f },
    { -1.0f,  1.0f, -1.0f },
    { -1.0f, -1.0f,  1.0f },
    { -1.0f, -1.0f, -1.0f }
};
const char *g_contact_manifold_error_msg = "contact_manifold memory overflow: INCREASE phys_contact_manifold_process::ALLOCATOR_MEMORY_SIZE";

create_gjk_geom_collision_visitor g_empty_collision_visitor;

void gjk_base_t::comp_aabb_loc()
{
    if ((this->m_flags & FLAG_AABB_LOC_VALID) == 0)
    {
        this->m_flags |= FLAG_AABB_LOC_VALID;
        this->calc_aabb(&PHYS_IDENTITY_MATRIX, &this->m_aabb_mn_loc, &this->m_aabb_mx_loc);
    }
}

const cbrush_t *gjk_base_t::get_brush() const
{
    return 0;
}

int gjk_base_t::get_contents()
{
    iassert(get_flag(FLAG_CONTENTS_VALID));
    return this->m_contents;
}

unsigned int gjk_base_t::get_geom_id() const
{
    iassert(get_flag(FLAG_GEOM_ID_VALID));
    return this->m_gjk_geom_id;
}

const phys_mat44 *__thiscall gjk_base_t::get_xform()
{
    iassert(get_flag(FLAG_XFORM_VALID));
    assert_mat44_initialized(*this->m_xform_);
    return this->m_xform_;
}

bool gjk_base_t::is_foot(const phys_vec3 *hit_point) const
{
    return 0;
}

bool gjk_base_t::is_walkable(const phys_vec3 *hit_point, const phys_vec3 *up)
{
    return 0;
}

void gjk_base_t::set_contents(int contents)
{
    this->m_flags |= FLAG_CONTENTS_VALID;
    this->m_contents = contents;
}

void gjk_base_t::set_geom_id_new(unsigned int geom_id)
{
    this->m_flags |= FLAG_GEOM_ID_VALID;
    this->m_gjk_geom_id = geom_id;
}

void gjk_base_t::set_xform(const phys_mat44 *xform)
{
    iassert(xform);

    assert_mat44_initialized(*xform);

    this->m_flags |= FLAG_XFORM_VALID;
    this->m_xform_ = xform;
}

PhysGeomList *__cdecl xmodel_get_geomlist(const XModel *model, int bone_index)
{
    iassert(model);

    if ( bone_index < 0 )
        return XModelGetCollmap(model, 0);
    else
        return XModelGetCollmapForBoneIndex(model, bone_index);
}

gjk_aabb_t *__cdecl gjk_aabb_t::create(
                const phys_vec3 *center,
                const phys_vec3 *dims,
                int stype,
                gjk_collision_visitor *allocator)
{
    unsigned int unique_id; // eax
    gjk_aabb_t *obj; // [esp+3Ch] [ebp-4h]

    iassert(allocator);

    if ( allocator->is_query() )
    {
        obj = (gjk_aabb_t *)allocator->allocate(sizeof(gjk_aabb_t), 16, 0);
        new (obj) gjk_aabb_t();
        obj->m_flags |= FLAG_TEMP_ALLOCATION;
    }
    else
    {
        //obj = phys_simple_allocator<gjk_aabb_t>::allocate(&aabb_pool);
        obj = aabb_pool.allocate();
        //unique_id = gjk_unique_id_database_t::get_unique_id(&g_gjk_unique_id_database);
        unique_id = g_gjk_unique_id_database.get_unique_id();
        //gjk_base_t::set_geom_id_new(obj, unique_id);
        obj->set_geom_id_new(unique_id);
    }

    iassert(obj);

    obj->m_center_local.x = center->x;
    obj->m_center_local.y = center->y;
    obj->m_center_local.z = center->z;
    obj->m_dims.x = dims->x;
    obj->m_dims.y = dims->y;
    obj->m_dims.z = dims->z;

    iassert(stype < SURF_TYPECOUNT);

    obj->stype = stype;
    return obj;
}

unsigned int gjk_unique_id_database_t::get_unique_id()
{
    unsigned int id; // [esp+8h] [ebp-4h]

    do
    {
        id = this->m_counter;

        iassert(id != 0);
        //if ( !this->m_counter
        //    && _tlAssert(
        //             "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_broad_phase.h",
        //             153,
        //             "id != 0",
        //             "gjk unique_id wrapped around.") )
        //{
        //    __debugbreak();
        //}
    }
    while ( _InterlockedCompareExchange(&this->m_counter, id + 1, id) != id );

    return id;
}

void gjk_aabb_t::support(
                const phys_vec3 &dir,
                phys_vec3 *support_vert,
                phys_vec3 *support_ind) const
{
    if ( dir.z >= 0.0 )
        support_ind->z = 1.0f;
    else
        support_ind->z = -1.0f;

    if ( dir.y >= 0.0 )
        support_ind->y = 1.0f;
    else
        support_ind->y = -1.0f;

    if ( dir.x >= 0.0 )
        support_ind->x = 1.0f;
    else
        support_ind->x = -1.0f;

    support_vert->x = (support_ind->x * this->m_dims.x) + this->m_center_local.x;
    support_vert->y = (support_ind->y * this->m_dims.y) + this->m_center_local.y;
    support_vert->z = (support_ind->z * this->m_dims.z) + this->m_center_local.z;
}

void gjk_aabb_t::get_simplex(
                const cached_simplex_info *cache_info,
                int index_count,
                phys_vec3 *simplex_verts,
                phys_vec3 *simplex_inds) const
{
    phys_vec3 *v7; // [esp-10h] [ebp-1Ch]
    const float *p_x; // [esp-Ch] [ebp-18h]

    for ( int i = 0; i < index_count; ++i )
    {
        p_x = &cache_info->m_indices[i].x;
        v7 = &simplex_inds[i];
        v7->x = p_x[0];
        v7->y = p_x[1];
        v7->z = p_x[2];
        simplex_verts[i].x = (p_x[0] * this->m_dims.x) + this->m_center_local.x;
        simplex_verts[i].y = (p_x[1] * this->m_dims.y) + this->m_center_local.y;
        simplex_verts[i].z = (p_x[2] * this->m_dims.z) + this->m_center_local.z;
    }
}

const phys_vec3 *gjk_aabb_t::get_center(phys_vec3 *result) const
{
    *result = this->m_center_local;
    return result;
}

void gjk_aabb_t::get_feature(phys_contact_manifold *cman) const
{
    phys_vec3 p; // [esp-50h] [ebp-5Ch] BYREF
    const phys_vec3 *p_sign; // [esp-Ch] [ebp-18h]

    p_sign = g_phys_vec3_box_sgn;

    static_assert(ARRAY_COUNT(g_phys_vec3_box_sgn) == 8);
    for ( int i = 0; i < ARRAY_COUNT(g_phys_vec3_box_sgn); i++ )
    {
        p.x = (p_sign->x * this->m_dims.x) + this->m_center_local.x;
        p.y = (p_sign->y * this->m_dims.y) + this->m_center_local.y;
        p.z = (p_sign->z * this->m_dims.z) + this->m_center_local.z;

        cman->add_feature_point(&p);

        p_sign++;
    }
}

void phys_contact_manifold::add_feature_point(const phys_vec3 *p)
{
    contact_manifold_mesh_point *mp; // [esp-8h] [ebp-58h]
    float v4; // [esp+4h] [ebp-4Ch]
    float v5; // [esp+8h] [ebp-48h]
    float v6; // [esp+Ch] [ebp-44h]
    float orth_dist_sq; // [esp+14h] [ebp-3Ch]
    float orth_dist; // [esp+18h] [ebp-38h]
    float v9; // [esp+1Ch] [ebp-34h]

    orth_dist_sq = p->x - this->m_feature_hitp.x;
    orth_dist = p->y - this->m_feature_hitp.y;
    v9 = p->z - this->m_feature_hitp.z;
    v6 = (float)((float)(orth_dist_sq * this->m_feature_hitn.x) + (float)(orth_dist * this->m_feature_hitn.y))
         + (float)(v9 * this->m_feature_hitn.z);
    v5 = v6 * v6;
    v4 = (float)((float)(orth_dist_sq * orth_dist_sq) + (float)(orth_dist * orth_dist)) + (float)(v9 * v9);
    if ( this->m_feature_distance_eps >= v6 || (float)(v4 * this->m_sin_feautre_angular_eps_sq) >= v5 )
    {
        mp = (contact_manifold_mesh_point *)this->m_allocator->fast_allocate(32, g_contact_manifold_error_msg);

        iassert((unsigned int)(mp) % PHYS_ALIGNOF(contact_manifold_mesh_point) == 0);
        iassert(mp == m_list_mesh_point + m_list_mesh_point_count);

        ++this->m_list_mesh_point_count;
        mp->m_p.x = orth_dist_sq;
        mp->m_p.y = orth_dist;
        mp->m_p.z = v9;
        mp->m_contact_p.y = v4;
        if ( v4 <= (float)(0.0033999998 * 0.0033999998) )
        {
            mp->m_contact_p.x = 2.0f;
            ++this->m_close_mesh_point_count;
        }
        else
        {
            mp->m_contact_p.x = 1.0 - (float)(v5 / v4);
        }
    }
}

char *phys_memory_heap::fast_allocate(int size, const char *error_msg)
{
    char *addr; // [esp+4h] [ebp-4h]

    addr = this->m_buffer_cur;
    this->m_buffer_cur = &addr[size];

    iassert(m_buffer_cur <= m_buffer_end);
    return addr;
}

void gjk_aabb_t::calc_aabb(
                const phys_mat44 *xform,
                phys_vec3 *aabb_min,
                phys_vec3 *aabb_max) const
{
    assert_mat44_initialized(*xform);
    phys_calc_world_aabb(&this->m_center_local, &this->m_dims, xform, aabb_min, aabb_max);
}

const cbrush_t *gjk_aabb_t::get_brush() const
{
    return this->m_brush;
}

void __cdecl gjk_aabb_t::destroy(gjk_aabb_t *geom)
{
    if ( geom )
    {
        iassert(!geom->get_flag(gjk_base_t::FLAG_TEMP_ALLOCATION));

        aabb_pool.free(geom);
        //phys_simple_allocator<gjk_polygon_cylinder_t>::free(
        //    (phys_simple_allocator<gjk_polygon_cylinder_t> *)&aabb_pool,
        //    (gjk_polygon_cylinder_t *)geom);
    }
}

gjk_obb_t *__cdecl gjk_obb_t::create(
                const phys_mat44 *xform,
                const phys_vec3 *dims,
                int stype,
                gjk_collision_visitor *allocator)
{
    unsigned int unique_id; // eax
    gjk_obb_t *obj; // [esp+50h] [ebp-4h]

    iassert(allocator);
    if ( allocator->is_query() )
    {
        obj = (gjk_obb_t *)allocator->allocate(sizeof(gjk_obb_t), 16, 0);
        new (obj) gjk_obb_t();
        obj->m_flags |= FLAG_TEMP_ALLOCATION;
    }
    else
    {
        //obj = phys_simple_allocator<gjk_obb_t>::allocate(&obb_pool);
        obj = obb_pool.allocate();
        //unique_id = gjk_unique_id_database_t::get_unique_id(&g_gjk_unique_id_database);
        unique_id = g_gjk_unique_id_database.get_unique_id();
        //gjk_base_t::set_geom_id_new(obj, unique_id);
        obj->set_geom_id_new(unique_id);
    }

    iassert(obj);

    obj->m_dims.x = dims->x;
    obj->m_dims.y = dims->y;
    obj->m_dims.z = dims->z;

    //phys_mat44::operator=(&obj->m_xform, xform);
    obj->m_xform = *xform;

    assert_mat44_initialized(*xform);

    iassert(stype < SURF_TYPECOUNT);
    obj->stype = stype;
    return obj;
}

void gjk_obb_t::support(
                const phys_vec3 &dir,
                phys_vec3 *support_vert,
                phys_vec3 *support_ind) const
{
    phys_vec3 v7; // [esp-4Ch] [ebp-6Ch] BYREF
    phys_vec3 local_v; // [esp-Ch] [ebp-2Ch] BYREF
    
    phys_inv_multiply(&local_v, &m_xform, &dir);
    if (local_v.z >= 0.0)
        support_ind->z = 1.0f;
    else
        support_ind->z = -1.0f;
    if (local_v.y >= 0.0)
        support_ind->y = 1.0f;
    else
        support_ind->y = -1.0f;
    if (local_v.x >= 0.0)
        support_ind->x = 1.0f;
    else
        support_ind->x = -1.0f;

    v7.x = support_ind->x * m_dims.x;
    v7.y = support_ind->y * m_dims.y;
    v7.z = support_ind->z * m_dims.z;
    phys_full_multiply(support_vert, &m_xform, &v7);
}

void gjk_obb_t::get_simplex(
                const cached_simplex_info *cache_info,
                int index_count,
                phys_vec3 *simplex_verts,
                phys_vec3 *simplex_inds) const
{
    phys_vec3 *v6; // [esp-58h] [ebp-64h]
    const phys_vec3 *v7; // [esp-54h] [ebp-60h]
    phys_vec3 v8; // [esp-50h] [ebp-5Ch] BYREF
    phys_vec3 v9; // [esp-40h] [ebp-4Ch] BYREF
    float v10; // [esp-24h] [ebp-30h]
    float v11; // [esp-20h] [ebp-2Ch]
    float v12; // [esp-1Ch] [ebp-28h]
    const float *v13; // [esp-18h] [ebp-24h]
    const phys_vec3 *p_m_dims; // [esp-14h] [ebp-20h]
    phys_vec3 *v15; // [esp-10h] [ebp-1Ch]
    const float *p_x; // [esp-Ch] [ebp-18h]
    int j; // [esp-8h] [ebp-14h]
    const gjk_obb_t *v18; // [esp-4h] [ebp-10h]
    //int v19; // [esp+0h] [ebp-Ch] BYREF
    //int i; // [esp+4h] [ebp-8h]
    //int retaddr; // [esp+Ch] [ebp+0h]
    //
    //v19 = a2;
    //i = retaddr;
    v18 = this;
    for ( j = 0; j < index_count; ++j )
    {
        p_x = &cache_info->m_indices[j].x;
        v15 = &simplex_inds[j];
        v15->x = *p_x;
        v15->y = p_x[1];
        v15->z = p_x[2];
        p_m_dims = &v18->m_dims;
        v13 = &cache_info->m_indices[j].x;
        v12 = *v13 * v18->m_dims.x;
        v11 = v13[1] * v18->m_dims.y;
        v10 = v13[2] * v18->m_dims.z;
        v9.x = v12;
        v9.y = v11;
        v9.z = v10;
        v7 = phys_full_multiply(&v8, &v18->m_xform, &v9);
        v6 = &simplex_verts[j];
        v6->x = v7->x;
        v6->y = v7->y;
        v6->z = v7->z;
    }
}

const phys_vec3 *gjk_obb_t::get_center(phys_vec3 *result) const
{
    *result = this->m_xform.w;
    return result;
}

void gjk_obb_t::get_feature(phys_contact_manifold *cman) const
{
    phys_vec3 pt; // [esp-40h] [ebp-4Ch] BYREF
    phys_vec3 v5; // [esp-30h] [ebp-3Ch] BYREF
    const phys_vec3 *p_sign; // [esp-Ch] [ebp-18h]

    p_sign = g_phys_vec3_box_sgn;
    static_assert(ARRAY_COUNT(g_phys_vec3_box_sgn) == 8);
    for (int i = 0; i < ARRAY_COUNT(g_phys_vec3_box_sgn); i++)
    {
        v5.x = p_sign->x * this->m_dims.x;
        v5.y = p_sign->y * this->m_dims.y;
        v5.z = p_sign->z * this->m_dims.z;

        phys_full_multiply(&pt, &this->m_xform, &v5);
        cman->add_feature_point(&pt);

        p_sign++;
    }
}

void gjk_obb_t::calc_aabb(
    const phys_mat44 *xform,
    phys_vec3 *aabb_min,
    phys_vec3 *aabb_max) const
{
    phys_vec3 v5; // [esp-30h] [ebp-BCh] BYREF
    const phys_vec3 *p_sign; // [esp-Ch] [ebp-98h]
    const phys_vec3 *v12; // [esp-4h] [ebp-90h]
    phys_vec3 i; // [esp+0h] [ebp-8Ch] BYREF
    phys_vec3 v14; // [esp+10h] [ebp-7Ch] BYREF
    phys_mat44 mat; // [esp+30h] [ebp-5Ch] BYREF

    assert_mat44_initialized(*xform);

    phys_full_multiply_mat(&mat, xform, &this->m_xform);
    v14.x = this->m_dims.x * g_phys_vec3_box_sgn[0].x;
    v14.y = this->m_dims.y * g_phys_vec3_box_sgn[0].y;
    v14.z = this->m_dims.z * g_phys_vec3_box_sgn[0].z;

    v12 = phys_full_multiply(&i, &mat, &v14);
    aabb_min->x = v12->x;
    aabb_min->y = v12->y;
    aabb_min->z = v12->z;
    aabb_max->x = aabb_min->x;
    aabb_max->y = aabb_min->y;
    aabb_max->z = aabb_min->z;

    p_sign = &g_phys_vec3_box_sgn[1];
    for (int zz = 0; zz < ARRAY_COUNT(g_phys_vec3_box_sgn) - 1; zz++)
    {
        v5.x = this->m_dims.x * p_sign->x;
        v5.y = this->m_dims.y * p_sign->y;
        v5.z = this->m_dims.z * p_sign->z;
        phys_aabb_add_point(&mat, &v5, aabb_min, aabb_max);

        p_sign++;
    }
}

void    phys_aabb_add_point(
                const phys_mat44 *xform,
                const phys_vec3 *point,
                phys_vec3 *aabb_min,
                phys_vec3 *aabb_max)
{
    const phys_vec3 *v5; // [esp-40h] [ebp-50h]
    phys_vec3 v6; // [esp-3Ch] [ebp-4Ch] BYREF
    const phys_vec3 *v7; // [esp-20h] [ebp-30h]
    phys_vec3 v8; // [esp-1Ch] [ebp-2Ch] BYREF
    phys_vec3 v9; // [esp-Ch] [ebp-1Ch] OVERLAPPED BYREF
    //_DWORD v10[3]; // [esp+4h] [ebp-Ch] BYREF
    //_UNKNOWN *retaddr; // [esp+10h] [ebp+0h]
    //
    //*(float *)v10 = a1;
    //v10[1] = retaddr;
    phys_full_multiply(&v9, xform, point);
    v7 = phys_min(&v8, aabb_min, &v9);
    aabb_min->x = v7->x;
    aabb_min->y = v7->y;
    aabb_min->z = v7->z;
    v5 = phys_max(&v6, aabb_max, &v9);
    aabb_max->x = v5->x;
    aabb_max->y = v5->y;
    aabb_max->z = v5->z;
}

void __cdecl gjk_obb_t::destroy(gjk_obb_t *geom)
{
    if ( geom )
    {
        iassert(!geom->get_flag(gjk_base_t::FLAG_TEMP_ALLOCATION));

        //phys_simple_allocator<gjk_cylinder_t>::free((phys_simple_allocator<gjk_cylinder_t> *)&obb_pool, geom);
        obb_pool.free(geom);
    }
}

gjk_brush_t * gjk_brush_t::create(
                const cbrush_t *brush,
                int stype,
                gjk_collision_visitor *allocator)
{
    unsigned int unique_id; // eax

    if (brush->numsides)
    {
        gjk_brush_t *obj; // [esp-Ch] [ebp-CCh]

        iassert(allocator);

        if (allocator->is_query())
        {
            obj = (gjk_brush_t *)allocator->allocate(sizeof(gjk_brush_t), 16, 0);
            new (obj) gjk_brush_t();
            obj->m_flags |= FLAG_TEMP_ALLOCATION;
        }
        else
        {
            obj = brush_pool.allocate();
            unique_id = g_gjk_unique_id_database.get_unique_id();
            obj->set_geom_id_new(unique_id);
        }

        iassert(obj);

        obj->verts = brush->verts;
        obj->nverts = brush->numverts;
        obj->brush = brush;

        iassert(stype < SURF_TYPECOUNT);
        obj->stype = stype;

        obj->set_contents(brush->contents);
        return obj;
    }
    else
    {
        phys_vec3 aabb_max; // [esp+94h] [ebp-2Ch] BYREF
        phys_vec3 aabb_min; // [esp+A4h] [ebp-1Ch] BYREF
        Phys_Vec3ToNitrousVec(brush->mins, &aabb_min);
        Phys_Vec3ToNitrousVec(brush->maxs, &aabb_max);

        phys_vec3 center; // [esp+54h] [ebp-6Ch] BYREF
        center.x = 0.5 * (aabb_min.x + aabb_max.x);
        center.y = 0.5 * (aabb_min.y + aabb_max.y);
        center.z = 0.5 * (aabb_min.z + aabb_max.z);

        phys_vec3 half_dims; // [esp+14h] [ebp-ACh] BYREF
        half_dims.x = 0.5 * (aabb_max.x - aabb_min.x);
        half_dims.y = 0.5 * (aabb_max.y - aabb_min.y);
        half_dims.z = 0.5 * (aabb_max.z - aabb_min.z);

        // if there are no brush->numsides, it seems to just create an aabb instead
        gjk_aabb_t *aabb = gjk_aabb_t::create(&center, &half_dims, stype, allocator);
        aabb->set_contents(brush->contents);
        aabb->m_brush = brush;
        return (gjk_brush_t *)aabb;
    }
}

void gjk_brush_t::support(
                const phys_vec3 &dir,
                phys_vec3 *support_vert,
                phys_vec3 *support_ind) const
{
    float vec3_v[3]; // [esp+18h] [ebp-10h] BYREF
    Phys_NitrousVecToVec3(&dir, vec3_v);

    int best_i = 0;
    float best_d = Vec3Dot(this->verts[0], vec3_v);

    for ( int i = 1; i < this->nverts; i++ )
    {
        float dot = Vec3Dot(this->verts[i], vec3_v);
        if (dot > best_d)
        {
            best_i = i;
            best_d = dot;
        }
    }

    Phys_Vec3ToNitrousVec(this->verts[best_i], support_vert);
    LODWORD(support_ind->x) = best_i;
}

void gjk_brush_t::get_simplex(
                const cached_simplex_info *cache_info,
                int index_count,
                phys_vec3 *simplex_verts,
                phys_vec3 *simplex_inds) const
{
    const float *p_x; // [esp+4h] [ebp-10h]
    phys_vec3 *v7; // [esp+8h] [ebp-Ch]
    int i; // [esp+10h] [ebp-4h]

    for ( i = 0; i < index_count; ++i )
    {
        p_x = &cache_info->m_indices[i].x;
        v7 = &simplex_inds[i];
        v7->x = *p_x;
        v7->y = p_x[1];
        v7->z = p_x[2];
        Phys_Vec3ToNitrousVec((float *)this->verts[*(unsigned int *)p_x], &simplex_verts[i]);
    }
}

void gjk_brush_t::get_feature(phys_contact_manifold *cman) const
{
    phys_vec3 vec;

    for (int i = 0; i < this->nverts; i++ )
    {
        Phys_Vec3ToNitrousVec((float *)this->verts[i], &vec);
        cman->add_feature_point(&vec);
    }
}

void gjk_brush_t::calc_aabb(
                const phys_mat44 *xform,
                phys_vec3 *aabb_min,
                phys_vec3 *aabb_max) const
{
    const phys_vec3 *v5; // eax
    int i; // [esp-Ch] [ebp-44h]
    phys_vec3 v7; // [esp-4h] [ebp-3Ch] OVERLAPPED BYREF
    phys_vec3 v8; // [esp+Ch] [ebp-2Ch] BYREF
    const gjk_brush_t *v9; // [esp+28h] [ebp-10h]
    //_DWORD v10[2]; // [esp+2Ch] [ebp-Ch] BYREF
    //_UNKNOWN *retaddr; // [esp+38h] [ebp+0h]
    //
    //v10[0] = a2;
    //v10[1] = retaddr;
    v9 = this;
    Phys_Vec3ToNitrousVec((float *)this->verts, &v8);
    v5 = phys_full_multiply(&v7, xform, &v8);
    aabb_min->x = v5->x;
    aabb_min->y = v5->y;
    aabb_min->z = v5->z;
    aabb_max->x = aabb_min->x;
    aabb_max->y = aabb_min->y;
    aabb_max->z = aabb_min->z;
    for (i = 1; i < v9->nverts; ++i)
    {
        Phys_Vec3ToNitrousVec((float *)v9->verts[i], &v8);
        phys_aabb_add_point(xform, &v8, aabb_min, aabb_max);
    }
}

const cbrush_t *gjk_brush_t::get_brush() const
{
    return this->brush;
}

void __cdecl gjk_brush_t::destroy(gjk_brush_t *geom)
{
    if ( geom )
    {
        iassert(!geom->get_flag(gjk_base_t::FLAG_TEMP_ALLOCATION));
        //phys_simple_allocator<gjk_brush_t>::free(&brush_pool, geom);
        brush_pool.free(geom);
    }
}

gjk_partition_t *__cdecl gjk_partition_t::create(const CollisionAabbTree *tree, gjk_collision_visitor *allocator)
{
    unsigned int unique_id; // eax
    int nuinds; // [esp+Ch] [ebp-40h]
    unsigned __int16 *v6; // [esp+10h] [ebp-3Ch]
    gjk_partition_t *obj; // [esp+3Ch] [ebp-10h]
    dmaterial_t *materialInfo; // [esp+40h] [ebp-Ch]
    const CollisionPartition *partition; // [esp+48h] [ebp-4h]

    iassert(allocator);

    if ( allocator->is_query() )
    {
        obj = (gjk_partition_t *)allocator->allocate(sizeof(gjk_partition_t), 16, 0);
        new (obj) gjk_partition_t();
        obj->m_flags |= FLAG_TEMP_ALLOCATION;
    }
    else
    {
        //obj = phys_simple_allocator<gjk_partition_t>::allocate(&partition_pool);
        obj = partition_pool.allocate();
        //unique_id = gjk_unique_id_database_t::get_unique_id(&g_gjk_unique_id_database);
        unique_id = g_gjk_unique_id_database.get_unique_id();
        //gjk_base_t::set_geom_id_new(obj, unique_id);
        obj->set_geom_id_new(unique_id);
    }
    iassert(obj);
    partition = &cm.partitions[tree->u.firstChildIndex];
    nuinds = partition->nuinds;
    v6 = &cm.uinds[partition->fuind];
    obj->verts = cm.verts;
    obj->inds = v6;
    obj->ninds = nuinds;
    obj->partition = partition;
    obj->tree = tree;
    materialInfo = &cm.materials[tree->materialIndex];
    obj->stype = (materialInfo->surfaceFlags & 0x3F00000) >> 20;
    iassert(obj->stype < SURF_TYPECOUNT);
    //gjk_base_t::set_contents(obj, materialInfo->contentFlags);
    obj->set_contents(materialInfo->contentFlags);
    return obj;
}

void gjk_partition_t::support(
                const phys_vec3 &dir,
                phys_vec3 *support_vert,
                phys_vec3 *support_ind) const
{
    int best_i; // [esp+14h] [ebp-14h]
    float dir_v[3]; // [esp+18h] [ebp-10h] BYREF
    float best_d; // [esp+24h] [ebp-4h]

    Phys_NitrousVecToVec3(&dir, dir_v);
    best_i = 0;
    best_d = Vec3Dot(this->verts[this->inds[0]], dir_v);

    for ( int i = 1; i < this->ninds; ++i )
    {
        float dot = Vec3Dot(this->verts[this->inds[i]], dir_v);
        if ( dot > best_d )
        {
            best_i = i;
            best_d = dot;
        }
    }

    Phys_Vec3ToNitrousVec(this->verts[this->inds[best_i]], support_vert);
    LODWORD(support_ind->x) = best_i;
}

void gjk_partition_t::get_simplex(
                const cached_simplex_info *cache_info,
                int index_count,
                phys_vec3 *simplex_verts,
                phys_vec3 *simplex_inds) const
{
    const float *p_x; // [esp+4h] [ebp-10h]
    phys_vec3 *v7; // [esp+8h] [ebp-Ch]
    int i; // [esp+10h] [ebp-4h]

    for ( i = 0; i < index_count; ++i )
    {
        p_x = &cache_info->m_indices[i].x;
        v7 = &simplex_inds[i];
        v7->x = *p_x;
        v7->y = p_x[1];
        v7->z = p_x[2];
        Phys_Vec3ToNitrousVec((float *)this->verts[this->inds[*(unsigned int *)p_x]], &simplex_verts[i]);
    }
}

const phys_vec3 *gjk_brush_t::get_center(phys_vec3 *result) const
{
    result->x = 0.0f;
    result->y = 0.0f;
    result->z = 0.0f;
    return result;
}

void gjk_partition_t::get_feature(phys_contact_manifold *cman) const
{
    phys_vec3 v; // [esp+0h] [ebp-20h]


    for (int i = 0; i < this->ninds; i++)
    {
        //Phys_Vec3ToNitrousVec((float *)v5->verts[v5->inds[LODWORD(v.w)]], (phys_vec3 *)v3);
        Phys_Vec3ToNitrousVec((float *)this->verts[this->inds[i]], &v);
        //phys_contact_manifold::add_feature_point(cman, (const phys_vec3 *)v3);
        cman->add_feature_point(&v);
    }
}

void gjk_partition_t::calc_aabb(
                const phys_mat44 *xform,
                phys_vec3 *aabb_min,
                phys_vec3 *aabb_max) const
{
    phys_vec3 v5; // [esp-Ch] [ebp-5Ch] BYREF
    const phys_vec3 *v6; // [esp+10h] [ebp-40h]
    phys_vec3 v7; // [esp+14h] [ebp-3Ch] BYREF
    phys_vec3 v; // [esp+24h] [ebp-2Ch] BYREF
    unsigned __int16 *last_inds_i; // [esp+38h] [ebp-18h]
    unsigned __int16 *inds_i; // [esp+3Ch] [ebp-14h]
    const gjk_partition_t *thisa; // [esp+40h] [ebp-10h]
    //_UNKNOWN *v12[2]; // [esp+44h] [ebp-Ch] BYREF
    //phys_vec3 *aabb_maxa; // [esp+50h] [ebp+0h]
    //
    //v12[0] = a2;
    //v12[1] = aabb_maxa;
    thisa = this;
    inds_i = this->inds;
    last_inds_i = &inds_i[this->ninds];
    Phys_Vec3ToNitrousVec((float *)this->verts[*inds_i], &v);
    v6 = phys_full_multiply(&v7, xform, &v);
    aabb_min->x = v6->x;
    aabb_min->y = v6->y;
    aabb_min->z = v6->z;
    aabb_max->x = aabb_min->x;
    aabb_max->y = aabb_min->y;
    aabb_max->z = aabb_min->z;
    for (++inds_i; inds_i < last_inds_i; ++inds_i)
    {
        Phys_Vec3ToNitrousVec((float *)thisa->verts[*inds_i], &v5);
        phys_aabb_add_point(xform, &v5, aabb_min, aabb_max);
    }
}

void __cdecl gjk_partition_t::destroy(gjk_partition_t *geom)
{
    if ( geom )
    {
        iassert(!geom->get_flag(gjk_base_t::FLAG_TEMP_ALLOCATION));
        //phys_simple_allocator<gjk_partition_t>::free(&partition_pool, geom);
        partition_pool.free(geom);
    }
}

void gjk_double_sphere_t::support(
                const phys_vec3 &dir,
                phys_vec3 *support_vert,
                phys_vec3 *support_ind) const
{
    float v4; // [esp-Ch] [ebp-7Ch]
    float v5; // [esp-8h] [ebp-78h]
    float v6; // [esp-4h] [ebp-74h]
    float v7; // [esp+30h] [ebp-40h]
    float nv; // [esp+58h] [ebp-18h]

    if ( this->m_count == 1 )
    {
        support_vert->x = this->m_list_center[0].x;
        support_vert->y = this->m_list_center[0].y;
        support_vert->z = this->m_list_center[0].z;
        support_ind->x = 0.0;
    }
    else
    {
        nv = Abs(dir);
        if ( nv <= 0.000001
            && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.h", 449, 0, "%s", "nv > 0.000001f") )
        {
            __debugbreak();
        }
        v7 = this->m_list_radius[1];
        v4 = this->m_list_center[1].x + (float)(v7 * (float)((float)(1.0 / nv) * dir.x));
        v5 = this->m_list_center[1].y + (float)(v7 * (float)((float)(1.0 / nv) * dir.y));
        v6 = this->m_list_center[1].z + (float)(v7 * (float)((float)(1.0 / nv) * dir.z));

        if ( (float)((float)((float)(dir.x * (float)(this->m_list_center[0].x - v4))
                                             + (float)(dir.y * (float)(this->m_list_center[0].y - v5)))
                             + (float)(dir.z * (float)(this->m_list_center[0].z - v6))) < 0.0 )
        {
            support_vert->x = v4;
            support_vert->y = v5;
            support_vert->z = v6;
            LODWORD(support_ind->x) = 1;
        }
        else
        {
            support_vert->x = this->m_list_center[0].x;
            support_vert->y = this->m_list_center[0].y;
            support_vert->z = this->m_list_center[0].z;
            support_ind->x = 0.0;
        }
    }
}

void gjk_double_sphere_t::get_simplex(
                const cached_simplex_info *cache_info,
                int index_count,
                phys_vec3 *simplex_verts,
                phys_vec3 *simplex_inds) const
{
    float v7; // [esp-10h] [ebp-20h]
    unsigned int ind; // [esp-Ch] [ebp-1Ch]

    unsigned int *cached = (unsigned int*)&cache_info->m_indices[1].x;
    for ( int i = 0; i < index_count; ++i )
    {
        //ind = *((unsigned int *)&cache_info->m_indices[1].x + i);
        ind = cached[i];
        iassert(ind >= 0 && ind < m_count);

        LODWORD(simplex_inds[i].x) = ind;
        v7 = this->m_list_radius[ind];

        simplex_verts[i].x = this->m_list_center[ind].x + (float)(v7 * cache_info->m_indices[0].x);
        simplex_verts[i].y = this->m_list_center[ind].y + (float)(v7 * cache_info->m_indices[0].y);
        simplex_verts[i].z = this->m_list_center[ind].z + (float)(v7 * cache_info->m_indices[0].z);
    }
}

void gjk_double_sphere_t::set_simplex(
                const phys_vec3 *simplex_inds,
                int w_set,
                const phys_vec3 *normal,
                cached_simplex_info *cache_info) const
{
    cache_info->m_indices[0].x = normal->x;
    cache_info->m_indices[0].y = normal->y;
    cache_info->m_indices[0].z = normal->z;

    unsigned int *inds = (unsigned int*)&cache_info->m_indices[1].x;
    for (int i = 0; i < 4; ++i)
    {
        if ((w_set & (1 << i)) != 0)
        {
            *inds++ = LODWORD(simplex_inds[i].x); // ptr incremented by 4 in asm
        }
    }
}

const phys_vec3 *gjk_double_sphere_t::get_center(phys_vec3 *result) const
{
    *result = this->m_center;
    return result;
}

void gjk_double_sphere_t::get_feature(phys_contact_manifold *cman) const
{
    phys_vec3 v3; // [esp-50h] [ebp-5Ch] BYREF
    float v4; // [esp-40h] [ebp-4Ch]
    float v5; // [esp-3Ch] [ebp-48h]
    float v6; // [esp-38h] [ebp-44h]
    const float *p_x; // [esp-34h] [ebp-40h]
    float v8; // [esp-30h] [ebp-3Ch]
    float v9; // [esp-2Ch] [ebp-38h]
    float v10; // [esp-28h] [ebp-34h]
    float v11; // [esp-1Ch] [ebp-28h]
    float v12; // [esp-18h] [ebp-24h]
    float v13; // [esp-14h] [ebp-20h]
    float v14; // [esp-10h] [ebp-1Ch]
    phys_vec3 *p_m_feature_hitn; // [esp-Ch] [ebp-18h]
    int j; // [esp-8h] [ebp-14h]
    const gjk_double_sphere_t *v17; // [esp-4h] [ebp-10h]
    //int v18; // [esp+0h] [ebp-Ch]
    //int i; // [esp+4h] [ebp-8h]
    //int retaddr; // [esp+Ch] [ebp+0h]

    //v18 = a2;
    //i = retaddr;
    v17 = this;
    for ( j = 0; j < v17->m_count; ++j )
    {
        p_m_feature_hitn = &cman->m_feature_hitn;
        v14 = v17->m_list_radius[j] + v17->m_geom_radius;
        v13 = v14 * cman->m_feature_hitn.x;
        v12 = v14 * cman->m_feature_hitn.y;
        v11 = v14 * cman->m_feature_hitn.z;
        v8 = v13;
        v9 = v12;
        v10 = v11;
        p_x = &v17->m_list_center[j].x;
        v6 = *p_x - v13;
        v5 = v17->m_list_center[j].y - v12;
        v4 = v17->m_list_center[j].z - v11;
        v3.x = v6;
        v3.y = v5;
        v3.z = v4;
        cman->add_feature_point(&v3);
        //phys_contact_manifold::add_feature_point(cman, &v3);
    }
}

void gjk_double_sphere_t::calc_aabb(
                const phys_mat44 *xform,
                phys_vec3 *aabb_min,
                phys_vec3 *aabb_max) const
{
    int i; // [esp+1A8h] [ebp-4h]

    phys_aabb_init_sphere(
        xform,
        this->m_list_center,
        this->m_list_radius[0] + this->m_geom_radius,
        aabb_min,
        aabb_max);

    for ( i = 1; i < this->m_count; ++i )
        phys_aabb_add_sphere(
            xform,
            &this->m_list_center[i],
            this->m_list_radius[i] + this->m_geom_radius,
            aabb_min,
            aabb_max);
}

void    phys_aabb_init_sphere(
                const phys_mat44 *xform,
                const phys_vec3 *center,
                float radius,
                phys_vec3 *aabb_min,
                phys_vec3 *aabb_max)
{
    phys_vec3 local_center; // [esp+4h] [ebp-1Ch] OVERLAPPED BYREF

    phys_full_multiply(&local_center, xform, center);
    aabb_min->x = local_center.x - radius;
    aabb_min->y = local_center.y - radius;
    aabb_min->z = local_center.z - radius;

    aabb_max->x = local_center.x + radius;
    aabb_max->y = local_center.y + radius;
    aabb_max->z = local_center.z + radius;
}

void    phys_aabb_add_sphere(
                const phys_mat44 *xform,
                const phys_vec3 *center,
                float radius,
                phys_vec3 *aabb_min,
                phys_vec3 *aabb_max)
{
    const phys_vec3 *v6; // eax
    const phys_vec3 *v7; // eax
    phys_vec3 v8; // [esp-6Ch] [ebp-9Ch] BYREF
    phys_vec3 v9; // [esp-5Ch] [ebp-8Ch] BYREF
    float v10; // [esp-4Ch] [ebp-7Ch]
    float v11; // [esp-48h] [ebp-78h]
    float v12; // [esp-44h] [ebp-74h]
    const phys_vec3 *v13; // [esp-40h] [ebp-70h]
    phys_vec3 v14; // [esp-3Ch] [ebp-6Ch] BYREF
    phys_vec3 v15; // [esp-2Ch] [ebp-5Ch] BYREF
    float v16; // [esp-18h] [ebp-48h]
    float v17; // [esp-14h] [ebp-44h]
    float v18; // [esp-10h] [ebp-40h]
    float v19; // [esp-Ch] [ebp-3Ch]
    float v20; // [esp-8h] [ebp-38h]
    float v21; // [esp-4h] [ebp-34h]
    phys_vec3 v22; // [esp+4h] [ebp-2Ch] OVERLAPPED BYREF
    phys_vec3 v23; // [esp+14h] [ebp-1Ch] BYREF
    //int v24; // [esp+24h] [ebp-Ch]
    //void *v25; // [esp+28h] [ebp-8h]
    //void *retaddr; // [esp+30h] [ebp+0h]
    //
    //v24 = a1;
    //v25 = retaddr;
    v6 = phys_multiply(&v23, xform, center);
    //operator+(&v22, v6, &xform->w);
    v22 = *v6 + xform->w;
    v19 = radius;
    v20 = radius;
    v21 = radius;
    v18 = v22.x - radius;
    v17 = v22.y - radius;
    v16 = v22.z - radius;
    v15.x = v22.x - radius;
    v15.y = v22.y - radius;
    v15.z = v22.z - radius;
    v13 = phys_min(&v14, aabb_min, &v15);
    aabb_min->x = v13->x;
    aabb_min->y = v13->y;
    aabb_min->z = v13->z;
    v12 = v22.x + v19;
    v11 = v22.y + v20;
    v10 = v22.z + v21;
    v9.x = v22.x + v19;
    v9.y = v22.y + v20;
    v9.z = v22.z + v21;
    v7 = phys_max(&v8, aabb_max, &v9);
    aabb_max->x = v7->x;
    aabb_max->y = v7->y;
    aabb_max->z = v7->z;
}

float gjk_double_sphere_t::get_geom_radius() const
{
    return this->m_geom_radius;
}

bool gjk_double_sphere_t::is_polyhedron()
{
    return this->m_count == 1 || this->m_list_radius[1] == 0.0;
}

gjk_double_sphere_t *__cdecl gjk_double_sphere_t::create(
                const phys_vec3 *c0,
                const phys_vec3 *c1,
                float r,
                int stype,
                gjk_collision_visitor *allocator)
{
    unsigned int unique_id; // eax
    gjk_double_sphere_t *obj; // [esp-Ch] [ebp-28h]
    gjk_double_sphere_t *v12; // [esp-4h] [ebp-20h]

    iassert(allocator);

    if ( allocator->is_query() )
    {
        obj = (gjk_double_sphere_t *)allocator->allocate(sizeof(gjk_double_sphere_t), 0, 0);
        new (obj) gjk_double_sphere_t();
        obj->m_flags |= FLAG_TEMP_ALLOCATION;
    }
    else
    {
        //obj = phys_simple_allocator<gjk_double_sphere_t>::allocate(&double_sphere_pool);
        obj = double_sphere_pool.allocate();
        //unique_id = gjk_unique_id_database_t::get_unique_id(&g_gjk_unique_id_database);
        unique_id = g_gjk_unique_id_database.get_unique_id();
        //gjk_base_t::set_geom_id_new(obj, unique_id);
        obj->set_geom_id_new(unique_id);
    }

    iassert(obj);

    obj->m_center.x = 0.5 * (float)(c0->x + c1->x);
    obj->m_center.y = 0.5 * (float)(c0->y + c1->y);
    obj->m_center.z = 0.5 * (float)(c0->z + c1->z);
    obj->m_count = 2;
    obj->m_list_center[0].x = c0->x;
    obj->m_list_center[0].y = c0->y;
    obj->m_list_center[0].z = c0->z;
    obj->m_list_center[1].x = c1->x;
    obj->m_list_center[1].y = c1->y;
    obj->m_list_center[1].z = c1->z;
    obj->m_list_radius[0] = 0.0f;
    obj->m_list_radius[1] = 0.0f;
    obj->m_geom_radius = r;

    iassert(stype < SURF_TYPECOUNT);
    obj->stype = stype;
    return obj;
}

void __cdecl gjk_double_sphere_t::destroy(gjk_double_sphere_t *geom)
{
    if ( geom )
    {
        iassert(!geom->get_flag(gjk_base_t::FLAG_TEMP_ALLOCATION));
        double_sphere_pool.free(geom);
    }
}

gjk_cylinder_t *__cdecl gjk_cylinder_t::create(
                int _direction,
                float _halfHeight,
                float _radius,
                const phys_mat44 *_xform,
                int stype,
                gjk_collision_visitor *allocator)
{
    unsigned int unique_id; // eax
    gjk_cylinder_t *obj;

    iassert(allocator);

    if ( allocator->is_query() )
    {
        obj = (gjk_cylinder_t *)allocator->allocate(sizeof(gjk_cylinder_t), 16, 0);
        new ((void *)obj) gjk_cylinder_t();
        obj->m_flags |= FLAG_TEMP_ALLOCATION;
    }
    else
    {
        //obj = phys_simple_allocator<gjk_cylinder_t>::allocate(&cylinder_pool);
        obj = cylinder_pool.allocate();
        //unique_id = gjk_unique_id_database_t::get_unique_id(&g_gjk_unique_id_database);
        unique_id = g_gjk_unique_id_database.get_unique_id();
        //gjk_base_t::set_geom_id_new(obj, unique_id);
        obj->set_geom_id_new(unique_id);
    }

    iassert(obj);
    obj->direction = _direction;
    obj->halfHeight = _halfHeight;
    obj->radius = _radius;
    obj->m_geom_radius = 0.0f;
    //phys_mat44::operator=(&obj->xform, _xform);
    obj->xform = *_xform;

    assert_mat44_initialized(*_xform);

    iassert(stype < SURF_TYPECOUNT);
    obj->stype = stype;
    return obj;
}

void gjk_cylinder_t::support(
                const phys_vec3 &dir,
                phys_vec3 *support_vert,
                phys_vec3 *support_ind) const
{
    const phys_vec3 *v5; // eax
    phys_vec3 v6; // [esp-5Ch] [ebp-8Ch] BYREF
    phys_vec3 v7; // [esp-4Ch] [ebp-7Ch] BYREF
    const phys_vec3 *dims; // [esp-30h] [ebp-60h]
    phys_vec3 v12; // [esp-2Ch] [ebp-5Ch] BYREF
    unsigned int v13; // [esp-18h] [ebp-48h]
    float v16; // [esp-Ch] [ebp-3Ch]
    float v18; // [esp-4h] [ebp-34h]
    int len; // [esp+0h] [ebp-30h]
    phys_vec3 dir_local; // [esp+4h] [ebp-2Ch] OVERLAPPED BYREF

    phys_inv_multiply(&dir_local, &this->xform, &dir);
    
    v18 = dir_local[this->direction];
    dir_local[this->direction] = 0.0f;

    v16 = Vec3Length(&dir_local.x);
    if (v16 <= 0.001)
    {
        dir_local.x = PHYS_ZERO_VEC.x;
        dir_local.y = PHYS_ZERO_VEC.y;
        dir_local.z = PHYS_ZERO_VEC.z;
    }
    else
    {
        dir_local.x = dir_local.x * (float)(1.0 / v16);
        dir_local.y = dir_local.y * (float)(1.0 / v16);
        dir_local.z = dir_local.z * (float)(1.0 / v16);
    }

    if (v18 < 0.0)
        dir_local[this->direction] = -1.0f;
    else
        dir_local[this->direction] = 1.0f;

    support_ind->x = dir_local.x;
    support_ind->y = dir_local.y;
    support_ind->z = dir_local.z;

    dims = this->get_dims(&v12);

    v7.x = dir_local.x * dims->x;
    v7.y = dir_local.y * dims->y;
    v7.z = dir_local.z * dims->z;

    //v5 = phys_full_multiply((int)v22, &v6, &v21->xform, &v7);
    v5 = phys_full_multiply(&v6, &this->xform, &v7);

    support_vert->x = v5->x;
    support_vert->y = v5->y;
    support_vert->z = v5->z;
}

const phys_vec3 *gjk_cylinder_t::get_dims(phys_vec3 *result) const
{
    phys_vec3 dims; // [esp-Ch] [ebp-2Ch]
    
    dims.x = this->radius;
    dims.y = this->radius;
    dims.z = this->radius;

    dims[this->direction] = this->halfHeight;

    *result = dims;
    return result;
}

void gjk_cylinder_t::get_simplex(
                const cached_simplex_info *cache_info,
                int index_count,
                phys_vec3 *simplex_verts,
                phys_vec3 *simplex_inds) const
{
    phys_vec3 *v6; // [esp-50h] [ebp-74h]
    const phys_vec3 *v7; // [esp-4Ch] [ebp-70h]
    phys_vec3 v8; // [esp-48h] [ebp-6Ch] BYREF
    phys_vec3 v9; // [esp-38h] [ebp-5Ch] BYREF
    float v10; // [esp-24h] [ebp-48h]
    float v11; // [esp-20h] [ebp-44h]
    float v12; // [esp-1Ch] [ebp-40h]
    const float *v13; // [esp-18h] [ebp-3Ch]
    phys_vec3 *v14; // [esp-14h] [ebp-38h]
    const float *p_x; // [esp-10h] [ebp-34h]
    int i; // [esp-Ch] [ebp-30h]
    phys_vec3 v17; // [esp-8h] [ebp-2Ch] OVERLAPPED BYREF
    const gjk_cylinder_t *v18; // [esp+14h] [ebp-10h]
    //_DWORD v19[2]; // [esp+18h] [ebp-Ch] BYREF
    //_UNKNOWN *retaddr; // [esp+24h] [ebp+0h]
    //
    //v19[0] = a2;
    //v19[1] = retaddr;
    v18 = this;
    gjk_cylinder_t::get_dims(&v17);
    for (i = 0; i < index_count; ++i)
    {
        p_x = &cache_info->m_indices[i].x;
        v14 = &simplex_inds[i];
        v14->x = *p_x;
        v14->y = p_x[1];
        v14->z = p_x[2];
        v13 = &cache_info->m_indices[i].x;
        v12 = *v13 * v17.x;
        v11 = v13[1] * v17.y;
        v10 = v13[2] * v17.z;
        v9.x = v12;
        v9.y = v11;
        v9.z = v10;
        v7 = phys_full_multiply(&v8, &v18->xform, &v9);
        v6 = &simplex_verts[i];
        v6->x = v7->x;
        v6->y = v7->y;
        v6->z = v7->z;
    }
}

const phys_vec3 *gjk_cylinder_t::get_center(phys_vec3 *result) const
{
    phys_vec3 v4; // [esp-20h] [ebp-2Ch] BYREF

    memset(&v4, 0, 12);
    phys_full_multiply(result, &this->xform, &v4);
    return result;
}

//float ? c@ ? BC@ ? ? get_feature@gjk_cylinder_t@@UBEXPAVphys_contact_manifold@@@Z@4PAMA[4] = { 1.0, 0.0, -1.0, 0.0 };
//float ? s@ ? BC@ ? ? get_feature@gjk_cylinder_t@@UBEXPAVphys_contact_manifold@@@Z@4PAMA[4] = { 0.0, 1.0, 0.0, -1.0 };
float gjk_cylinder_t_c[4] = { 1.0, 0.0, -1.0, 0.0 };
float gjk_cylinder_t_s[4] = { 0.0, 1.0, 0.0, -1.0 };


void gjk_cylinder_t::get_feature(phys_contact_manifold *cman) const
{
    const phys_vec3 *v3; // ecx
    const phys_vec3 *v4; // eax
    const phys_vec3 *v5; // eax
    float halfHeight_low; // xmm0_4
    phys_vec3 *v7; // eax
    const phys_vec3 *v8; // eax
    const phys_vec3 *v9; // eax
    phys_vec3 *v10; // [esp+8h] [ebp-204h]
    const phys_vec3 *p_w; // [esp+Ch] [ebp-200h]
    const phys_vec3 *v12; // [esp+Ch] [ebp-200h]
    phys_vec3 v13; // [esp+10h] [ebp-1FCh] BYREF
    phys_vec3 v14; // [esp+20h] [ebp-1ECh] OVERLAPPED BYREF
    phys_vec3 v15; // [esp+30h] [ebp-1DCh] BYREF
    phys_vec3 v16; // [esp+40h] [ebp-1CCh] BYREF
    phys_vec3 v17; // [esp+50h] [ebp-1BCh] BYREF
    int j; // [esp+60h] [ebp-1ACh]
    int i; // [esp+6Ch] [ebp-1A0h]
    phys_vec3 v22; // [esp+70h] [ebp-19Ch] OVERLAPPED BYREF
    phys_vec3 v23; // [esp+80h] [ebp-18Ch] BYREF
    phys_vec3 v27; // [esp+A0h] [ebp-16Ch] BYREF
    phys_vec3 v31; // [esp+C0h] [ebp-14Ch] BYREF
    phys_vec3 *v32; // [esp+DCh] [ebp-130h]
    phys_vec3 v33; // [esp+E0h] [ebp-12Ch] BYREF
    float v34; // [esp+FCh] [ebp-110h]
    phys_vec3 v35; // [esp+100h] [ebp-10Ch] BYREF
    phys_vec3 v36; // [esp+110h] [ebp-FCh] BYREF
    unsigned int v38; // [esp+12Ch] [ebp-E0h]
    phys_vec3 v39; // [esp+130h] [ebp-DCh] BYREF
    phys_vec3 v40; // [esp+140h] [ebp-CCh] BYREF
    unsigned int direction; // [esp+15Ch] [ebp-B0h]
    phys_vec3 v43; // [esp+160h] [ebp-ACh] BYREF
    float v48; // [esp+180h] [ebp-8Ch]
    float len_; // [esp+184h] [ebp-88h]
    float v50; // [esp+188h] [ebp-84h]
    float sr; // [esp+18Ch] [ebp-80h]
    phys_vec3 len; // [esp+190h] [ebp-7Ch] BYREF
    //float x; // [esp+1A0h] [ebp-6Ch] BYREF
    //float y; // [esp+1A4h] [ebp-68h]
    //float z; // [esp+1A8h] [ebp-64h]
    phys_vec3 xyz;
    const phys_vec3 *v56; // [esp+1BCh] [ebp-50h]
    phys_vec3 v57; // [esp+1C0h] [ebp-4Ch] BYREF
    const phys_vec3 *p_z; // [esp+1D8h] [ebp-34h]
    const phys_vec3 *p_y; // [esp+1DCh] [ebp-30h]
    phys_vec3 v60; // [esp+1E0h] [ebp-2Ch] BYREF
    const gjk_cylinder_t *thisa; // [esp+1FCh] [ebp-10h]

    thisa = this;

    iassert(m_geom_radius == 0.0f);

    if (thisa->direction)
    {
        if (thisa->direction == 1)
        {
            p_y = &PHYS_IDENTITY_MATRIX.y;
            v60.x = PHYS_IDENTITY_MATRIX.y.x;
            v60.y = PHYS_IDENTITY_MATRIX.y.y;
            v3 = &PHYS_IDENTITY_MATRIX.y;
        }
        else
        {
            p_z = &PHYS_IDENTITY_MATRIX.z;
            v60.x = PHYS_IDENTITY_MATRIX.z.x;
            v60.y = PHYS_IDENTITY_MATRIX.z.y;
            v3 = &PHYS_IDENTITY_MATRIX.z;
        }
        v60.z = v3->z;
    }
    else
    {
        v60.x = PHYS_IDENTITY_MATRIX.x.x;
        v60.y = PHYS_IDENTITY_MATRIX.x.y;
        v60.z = PHYS_IDENTITY_MATRIX.x.z;
    }

    v56 = phys_inv_multiply(&v57, &thisa->xform, &cman->m_feature_hitn);
    xyz.x = v56->x;
    xyz.y = v56->y;
    xyz.z = v56->z;
    len.x = xyz.x;
    len.y = xyz.y;
    len.z = xyz.z;

    len[this->direction] = 0.0f;

    v50 = (float)((float)(v60.x * xyz.x) + (float)(v60.y * xyz.y)) + (float)(v60.z * xyz.z);

    if (fabs(v50) >= 0.70709997)
    {
        if (fabs(v50) >= 0.99000001)
        {
            v27.x = v60[2];
            v27.y = v60[0];
            v27.z = v60[1];
            len = v27;
            //phys_vec3::operator=(&len, &v27);
            v23.x = v60[1];
            v23.y = v60[2];
            v23.z = v60[0];
            v31 = v23;
            //phys_vec3::operator=(&v31, &v23);
        }
        else
        {
            v34 = 1.0 / Vec3Length(&len.x);
            len.x = len.x * v34;
            len.y = len.y * v34;
            len.z = len.z * v34;
            v32 = phys_cross(&v33, &len, &v60);
            v31.x = v32->x;
            v31.y = v32->y;
            v31.z = v32->z;
        }
        memset(&v22, 0, 12);

        if (xyz[this->direction] <= 0.0f)
            halfHeight_low = this->halfHeight;
        else
            halfHeight_low = -this->halfHeight;

        i = halfHeight_low;
        v22[this->direction] = halfHeight_low;
        len.x = len.x * this->radius;
        len.y = len.y * this->radius;
        len.z = len.z * this->radius;

        v31.x = v31.x * this->radius;
        v31.y = v31.y * this->radius;
        v31.z = v31.z * this->radius;

        for (j = 0; j < 4; ++j)
        {
            v17 = v31 * gjk_cylinder_t_s[j];
            //v10 = operator*(&v17, `gjk_cylinder_t::get_feature'::`18'::s[j], &v31);
            //v7 = operator*(&v16, `gjk_cylinder_t::get_feature'::`18'::c[j], &len);
            v16 = len * gjk_cylinder_t_c[j];
            //v8 = operator+(&v15, v7, v10);
            v15 = v16 + v17;
            //operator+(&v14, v8, &v22);
            v14 = v15 + v22;
            v9 = phys_full_multiply(&v13, &thisa->xform, &v14);
            cman->add_feature_point(v9);
        }
    }
    else
    {
        len_ = Vec3Length(&len.x);
        if (len_ <= 0.0000099999997
            && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.h", 604, 0, "%s", "len > 0.00001f"))
        {
            __debugbreak();
        }

        v48 = thisa->radius / len_;
        v43.x = (-(v48)) * len.x;
        v43.y = (-(v48)) * len.y;
        v43.z = (-(v48)) * len.z;

        v43[this->direction] = this->halfHeight;

        p_w = &thisa->xform.w;
        v4 = phys_multiply(&v40, &thisa->xform, &v43);
        //operator+(&v39, v4, p_w);
        v39 = *v4 + *p_w;
        cman->add_feature_point(&v39);

        v43[this->direction] = -this->halfHeight;

        v12 = &thisa->xform.w;
        v5 = phys_multiply(&v36, &thisa->xform, &v43);
        //operator+(&v35, v5, v12);
        v35 = *v5 + *v12;
        cman->add_feature_point(&v35);
    }
}

void gjk_cylinder_t::calc_aabb(
                const phys_mat44 *xform_,
                phys_vec3 *aabb_min,
                phys_vec3 *aabb_max) const
{
    phys_vec3 worldBot; // [esp+Ch] [ebp-6Ch] BYREF
    phys_vec3 worldTop; // [esp+1Ch] [ebp-5Ch] BYREF
    phys_vec3 bot; // [esp+3Ch] [ebp-3Ch] OVERLAPPED BYREF
    phys_vec3 top; // [esp+4Ch] [ebp-2Ch] BYREF

    memset(&top, 0, 12);
    memset(&bot, 0, 12);

    top[this->direction] += this->halfHeight;
    bot[this->direction] -= this->halfHeight;

    phys_full_multiply(&worldTop, &this->xform, &top);
    phys_aabb_init_sphere(xform_, &worldTop, this->radius + this->m_geom_radius, aabb_min, aabb_max);
    phys_full_multiply(&worldBot, &this->xform, &bot);
    phys_aabb_add_sphere(xform_, &worldBot, this->radius + this->m_geom_radius, aabb_min, aabb_max);
}

float gjk_cylinder_t::get_geom_radius() const
{
    return this->m_geom_radius;
}

void __cdecl gjk_cylinder_t::destroy(gjk_cylinder_t *geom)
{
    if ( geom )
    {
        iassert(!geom->get_flag(gjk_base_t::FLAG_TEMP_ALLOCATION));
        cylinder_pool.free(geom);
    }
}

static const float foot_offset = 2.0;
static const float head_offset = 10.0;
static const int tad = 1;

void    setup_gjk_polygon_cylinder(
                float *mins,
                float *maxs,
                float radius_adjust,
                gjk_polygon_cylinder_t *gjk_cylinder)
{
    float radius; // xmm0_4
    float x; // [esp+4h] [ebp-F8h]
    phys_vec3 dim; // [esp+10h] [ebp-ECh]
    phys_vec3 adjusted_maxs; // [esp+40h] [ebp-BCh] BYREF
    phys_vec3 adjusted_mins; // [esp+70h] [ebp-8Ch] BYREF
    phys_vec3 save_pv_maxs; // [esp+90h] [ebp-6Ch] BYREF
    phys_vec3 save_pv_mins; // [esp+A0h] [ebp-5Ch] BYREF
    phys_vec3 dim_adjust_vec; // [esp+B0h] [ebp-4Ch]
    phys_vec3 pv_maxs; // [esp+D0h] [ebp-2Ch] BYREF
    phys_vec3 pv_mins; // [esp+E0h] [ebp-1Ch] BYREF

    Phys_Vec3ToNitrousVec(mins, &pv_mins);
    Phys_Vec3ToNitrousVec(maxs, &pv_maxs);

    iassert(radius_adjust >= SURFACE_CLIP_EPSILON);

    dim_adjust_vec.x = radius_adjust - SURFACE_CLIP_EPSILON;
    dim_adjust_vec.y = radius_adjust - SURFACE_CLIP_EPSILON;
    dim_adjust_vec.z = radius_adjust - SURFACE_CLIP_EPSILON;

    save_pv_mins = pv_mins;
    save_pv_maxs = pv_maxs;

    adjusted_mins.x = pv_mins.x + dim_adjust_vec.x;
    adjusted_mins.y = pv_mins.y + dim_adjust_vec.y;
    adjusted_mins.z = pv_mins.z + dim_adjust_vec.z;
    phys_min(&pv_mins, &adjusted_mins, &save_pv_maxs);

    adjusted_maxs.x = save_pv_maxs.x - dim_adjust_vec.x;
    adjusted_maxs.y = save_pv_maxs.y - dim_adjust_vec.y;
    adjusted_maxs.z = save_pv_maxs.z - dim_adjust_vec.z;
    phys_max(&pv_maxs, &adjusted_maxs, &save_pv_mins);

    dim.x = pv_maxs.x - pv_mins.x;
    dim.y = pv_maxs.y - pv_mins.y;
    dim.z = pv_maxs.z - pv_mins.z;

    iassert(dim.GetX() >= 0.0f);
    iassert(dim.GetY() >= 0.0f);
    iassert(dim.GetZ() >= 0.0f);

    if (dim.x <= dim.z)
        x = dim.x;
    else
        x = dim.z;

    if ((0.5 * x) >= 1.0)
        radius = 0.5 * x;
    else
        radius = 1.0f;

    if (radius == 1.0)
        tlWarning("degenerate capsule.");

    iassert(radius >= 0.0f);

    if ((0.5 * dim.z) >= 1.0)
        gjk_cylinder->m_half_height = 0.5 * dim.z;
    else
        gjk_cylinder->m_half_height = 1.0f;

    if (gjk_cylinder->m_half_height == 1.0)
        tlWarning("degenerate capsule1.");

    gjk_cylinder->m_capsule_radius = radius;

    if (tad)
    {
        static float radius_scale_factor = 1.0 / cosf(((6.2831855 / 12.0) / 2.0));
        gjk_cylinder->m_polygon_cylinder_radius = radius * radius_scale_factor;
    }
    else
    {
        gjk_cylinder->m_polygon_cylinder_radius = radius;
    }

    gjk_cylinder->m_geom_radius = radius_adjust;
    gjk_cylinder->m_center.x = 0.0f;
    gjk_cylinder->m_center.y = 0.0f;
    gjk_cylinder->m_center.z = ((pv_mins.z + pv_maxs.z) * 0.5) + phys_player_collision_adjust_height->current.value;

    if (gjk_cylinder->m_half_height <= head_offset)
    {
        gjk_cylinder->m_head_offset = gjk_cylinder->m_half_height;
    }
    else
    {
        gjk_cylinder->m_head_offset = head_offset;
    }

    if (gjk_cylinder->m_half_height <= foot_offset)
    {
        gjk_cylinder->m_foot_offset = gjk_cylinder->m_half_height;
    }
    else
    {
        gjk_cylinder->m_foot_offset = foot_offset;
    }

    gjk_cylinder->m_mode = 0;
}



gjk_polygon_cylinder_t::poly_verts gjk_polygon_cylinder_t::s_poly_verts;

gjk_polygon_cylinder_t *__cdecl gjk_polygon_cylinder_t::create(
                float (*mins)[3],
                float (*maxs)[3],
                float radius_adjust,
                int stype,
                gjk_collision_visitor *allocator)
{
    unsigned int unique_id; // eax
    gjk_polygon_cylinder_t *obj; // [esp+3Ch] [ebp-4h]

    iassert(allocator);

    if (allocator->is_query())
    {
        obj = (gjk_polygon_cylinder_t *)allocator->allocate(sizeof(gjk_polygon_cylinder_t), 16, 0);
        new (obj) gjk_polygon_cylinder_t();
        obj->m_flags |= FLAG_TEMP_ALLOCATION;
    }
    else
    {
        obj = polygon_cylinder_pool.allocate();
        unique_id = g_gjk_unique_id_database.get_unique_id();
        obj->set_geom_id_new(unique_id);
    }

    iassert(obj);
    setup_gjk_polygon_cylinder((float *)mins, (float *)maxs, radius_adjust, obj);
    obj->stype = stype;
    return obj;
}

bool gjk_polygon_cylinder_t::is_foot(const phys_vec3 *hit_point) const
{
    if (this->m_mode)
    {
        float foot_z = (this->m_center.z - this->m_half_height + this->m_capsule_radius) - 1.0f;
        return foot_z >= hit_point->z;
    }

    static float offset = this->m_geom_radius * sinf(DEG2RAD(30.0f)); // KISAKTODO: consider making this non-static? looks odd, does the radius never change?

    float foot_z = (this->m_center.z - this->m_half_height + this->m_foot_offset) - offset;
    return foot_z >= hit_point->z;
}

void __cdecl gjk_polygon_cylinder_t::destroy(gjk_polygon_cylinder_t *geom)
{
    if ( geom )
    {
        iassert(!geom->get_flag(gjk_base_t::FLAG_TEMP_ALLOCATION));
        polygon_cylinder_pool.free(geom);
    }
}

void __cdecl destroy_gjk_geom(gjk_base_t *geom)
{
    switch ( geom->get_type() )
    {
        case GJK_AABB:
            gjk_aabb_t::destroy((gjk_aabb_t *)geom);
            break;
        case GJK_BRUSH:
            gjk_brush_t::destroy((gjk_brush_t *)geom);
            break;
        case GJK_PARTITION:
            gjk_partition_t::destroy((gjk_partition_t *)geom);
            break;
        case GJK_DOUBLE_SPHERE:
            gjk_double_sphere_t::destroy((gjk_double_sphere_t *)geom);
            break;
        case GJK_CYLINDER:
            gjk_cylinder_t::destroy((gjk_cylinder_t *)geom);
            break;
        case GJK_OBB:
            gjk_obb_t::destroy((gjk_obb_t *)geom);
            break;
        case GJK_POLYGON_CYLINDER:
            gjk_polygon_cylinder_t::destroy((gjk_polygon_cylinder_t *)geom);
            break;
        default:
            if ( _tlAssert(
                         "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.cpp",
                         403,
                         "0",
                         "Trying to free up unkownn geom. Leak!!!") )
            {
                __debugbreak();
            }
            break;
    }
}

gjk_aabb_t * create_aabb_gjk_geom(
                const float *mn,
                const float *mx,
                int stype,
                gjk_collision_visitor *allocator)
{
    phys_vec3 center;
    phys_vec3 halfExtents;

    center.x = (mx[0] + mn[0]) * 0.5f;
    center.y = (mx[1] + mn[1]) * 0.5f;
    center.z = (mx[2] + mn[2]) * 0.5f;

    halfExtents.x = (mx[0] - mn[0]) * 0.5f;
    halfExtents.y = (mx[1] - mn[1]) * 0.5f;
    halfExtents.z = (mx[2] - mn[2]) * 0.5f;

    return gjk_aabb_t::create(&center, &halfExtents, stype, allocator);
}

gjk_obb_t * create_obb_gjk_geom(
                float (*orientation)[3],
                float *offset,
                float *halfLengths,
                int stype,
                gjk_collision_visitor *allocator)
{
    phys_vec3 dims; // [esp-Ch] [ebp-5Ch] OVERLAPPED BYREF
    phys_mat44 xform; // [esp+4h] [ebp-4Ch] BYREF

    Phys_Vec3ToNitrousVec(offset, &xform.w);
    Phys_AxisToNitrousMat(orientation, &xform);
    Phys_Vec3ToNitrousVec(halfLengths, &dims);
    return gjk_obb_t::create(&xform, &dims, stype, allocator);
}

gjk_brush_t *__cdecl create_brush_gjk_geom(const cbrush_t *brush, int stype, gjk_collision_visitor *allocator)
{
    return gjk_brush_t::create(brush, stype, allocator);
}

gjk_double_sphere_t * create_capsule_gjk_geom(
                float *center,
                float radius,
                float halfHeight,
                unsigned int direction,
                int stype,
                gjk_collision_visitor *allocator)
{
    phys_vec3 c1; // [esp+1Ch] [ebp-3Ch] BYREF
    phys_vec3 c0; // [esp+2Ch] [ebp-2Ch] BYREF
    phys_vec3 _center; // [esp+3Ch] [ebp-1Ch] BYREF

    Phys_Vec3ToNitrousVec(center, &_center);
    c0 = _center;
    c1 = _center;

    c0[direction] -= halfHeight;
    c1[direction] += halfHeight;

    return gjk_double_sphere_t::create(&c0, &c1, radius, stype, allocator);
}

gjk_cylinder_t * create_cylinder_gjk_geom(
                float (*rot)[3],
                float *trans,
                float radius,
                float halfHeight,
                int stype,
                gjk_collision_visitor *allocator)
{
    phys_mat44 xform; // [esp+24h] [ebp-5Ch] OVERLAPPED BYREF
    phys_vec3 trans_v; // [esp+64h] [ebp-1Ch] BYREF

    Phys_Vec3ToNitrousVec(trans, &trans_v);
    Phys_AxisToNitrousMat(rot, &xform);

    xform.w.x = trans_v.x;
    xform.w.y = trans_v.y;
    xform.w.z = trans_v.z;

    assert_mat44_initialized(xform);

    return gjk_cylinder_t::create(0, halfHeight, radius, &xform, stype, allocator);
}

void __cdecl create_brush_model_gjk_geom_r(
                cLeafBrushNode_s *node,
                int *index_base,
                gjk_collision_visitor *allocator,
                int contents_mask)
{
    gjk_brush_t *gjk_geom; // [esp+0h] [ebp-18h]
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
                    gjk_geom = gjk_brush_t::create(
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
    gjk_brush_t *gjk_geom; // [esp+4h] [ebp-Ch]
    int savedregs; // [esp+10h] [ebp+0h] BYREF

    if ( !this->m_allocator
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.cpp", 497, 0, "%s", "m_allocator") )
    {
        __debugbreak();
    }
    if ( this->m_allocator->query_create_prolog_1(brush->mins, brush->maxs, brush) )
    {
        gjk_geom = gjk_brush_t::create(brush, (brush->axial_sflags[0][0] & 0x3F00000) >> 20, this->m_allocator);
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
    gjk_aabb_t *v9; // [esp+68h] [ebp-94h]
    gjk_cylinder_t *cylinder_gjk_geom; // [esp+6Ch] [ebp-90h]
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
    gjk_aabb_t *gjk_geom; // [esp+0h] [ebp-24h]
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

gjk_polygon_cylinder_t::poly_verts::poly_verts()
{
    for (int i = 0; i < 4; ++i)
    {
        float theta = (float)((float)((float)i * 3.1415925) / 2.0) / 3.0;
        this->m_co[i] = cos(theta);
        this->m_si[i] = sin(theta);
    }
}

void gjk_polygon_cylinder_t::support(const phys_vec3 &dir, phys_vec3 *support_vert, phys_vec3 *support_ind) const
{
    if (this->m_mode)
    {
        float pt2[3]; // [esp-Ch] [ebp-11Ch]
        float p1[3]; // [esp+14h] [ebp-FCh]
        iassert(m_half_height >= m_capsule_radius);

        float capradius = m_half_height - m_capsule_radius;
        p1[0] = m_center.x + 0.0;
        p1[1] = m_center.y + 0.0;
        p1[2] = m_center.z + capradius;

        pt2[0] = m_center.x - 0.0;
        pt2[1] = m_center.y - 0.0;
        pt2[2] = m_center.z - capradius;

        if (((((p1[0] - pt2[0]) * dir.x) + ((p1[1] - pt2[1]) * dir.y)) + ((p1[2] - pt2[2]) * dir.z)) < 0.0)
        {
            LODWORD(support_ind->x) = 1;
            support_vert->x = pt2[0];
            support_vert->y = pt2[1];
            support_vert->z = pt2[2];
        }
        else
        {
            support_ind->x = 0.0;
            support_vert->x = p1[0];
            support_vert->y = p1[1];
            support_vert->z = p1[2];
        }
    }
    else
    {
        float foot_or_head_z;
        float height; // [esp+F4h] [ebp-1Ch]
        int foot_or_head_ind; // [esp+ECh] [ebp-24h]
        int best_i = gjk_polygon_cylinder_t::s_poly_verts.support(&dir);
        if (dir.z < 0.0)
        {
            LODWORD(support_ind->x) = best_i + 12;
            height = -this->m_half_height + this->m_foot_offset;
            foot_or_head_z = -this->m_half_height;
            foot_or_head_ind = 25;
        }
        else
        {
            LODWORD(support_ind->x) = best_i;
            height = this->m_half_height - this->m_head_offset;
            foot_or_head_z = this->m_half_height;
            foot_or_head_ind = 24;
        }

        float co_; // [esp+E4h] [ebp-2Ch] BYREF
        float si_; // [esp+E8h] [ebp-28h] BYREF
        gjk_polygon_cylinder_t::s_poly_verts.get_co_si(best_i, &co_, &si_);

        float point[3];
        point[0] = (float)(this->m_polygon_cylinder_radius * co_) + this->m_center.x;
        point[1] = (float)(this->m_polygon_cylinder_radius * si_) + this->m_center.y;
        point[2] = height + this->m_center.z;

        float foot_or_head[3];
        foot_or_head[0] = this->m_center.x + 0.0;
        foot_or_head[1] = this->m_center.y + 0.0;
        foot_or_head[2] = foot_or_head_z + this->m_center.z;

        if (((((point[0] - foot_or_head[0]) * dir.x) 
            + ((point[1] - foot_or_head[1]) * dir.y)) 
            + ((point[2] - foot_or_head[2]) * dir.z)) < 0.0)
        {
            LODWORD(support_ind->x) = foot_or_head_ind;
            support_vert->x = foot_or_head[0];
            support_vert->y = foot_or_head[1];
            support_vert->z = foot_or_head[2];
        }
        else
        {
            support_vert->x = point[0];
            support_vert->y = point[1];
            support_vert->z = point[2];
        }

        // lwss add
        iassert(best_i >= 0 && best_i < 12);
        iassert(fabsf(support_vert->x - this->m_center.x) <= this->m_polygon_cylinder_radius + 0.01f);
        iassert(fabsf(support_vert->y - this->m_center.y) <= this->m_polygon_cylinder_radius + 0.01f);
        float z_range = this->m_half_height + 0.01f;
        iassert(support_vert->z >= this->m_center.z - z_range && support_vert->z <= this->m_center.z + z_range);
        // lwss end
    }
}

float gjk_polygon_cylinder_t::get_geom_radius() const
{
    if (this->m_mode)
        return this->m_geom_radius + this->m_capsule_radius;
    else
        return this->m_geom_radius;
}

// aislop supervised
void gjk_polygon_cylinder_t::get_simplex(
    const cached_simplex_info *cache_info,
    int index_count,
    phys_vec3 *simplex_verts,
    phys_vec3 *simplex_inds) const
{
    if (this->m_mode)
    {
        iassert(m_half_height >= m_capsule_radius);
        float cap_offset = m_half_height - m_capsule_radius;

        for (int i = 0; i < index_count; ++i)
        {
            float x = cache_info->m_indices[i].x;
            iassert(LODWORD(x) == 0 || LODWORD(x) == 1);

            simplex_inds[i].x = x;

            if (x == 0.0f)
            {
                simplex_verts[i].x = m_center.x + 0.0f;
                simplex_verts[i].y = m_center.y + 0.0f;
                simplex_verts[i].z = m_center.z + cap_offset;
            }
            else
            {
                simplex_verts[i].x = m_center.x - 0.0f;
                simplex_verts[i].y = m_center.y - 0.0f;
                simplex_verts[i].z = m_center.z - cap_offset;
            }
        }
    }
    else
    {
        for (int i = 0; i < index_count; ++i)
        {
            float x = cache_info->m_indices[i].x;
            int ind = SLODWORD(x);
            iassert(ind >= 0);

            simplex_inds[i].x = x;

            if (ind < 12)
            {
                float co, si;
                s_poly_verts.get_co_si(ind, &co, &si);

                simplex_verts[i].x = (m_polygon_cylinder_radius * co) + m_center.x;
                simplex_verts[i].y = (m_polygon_cylinder_radius * si) + m_center.y;
                simplex_verts[i].z = (m_half_height - m_head_offset) + m_center.z;
            }
            else if (ind < 24)
            {
                float co, si;
                s_poly_verts.get_co_si(ind - 12, &co, &si);

                simplex_verts[i].x = (m_polygon_cylinder_radius * co) + m_center.x;
                simplex_verts[i].y = (m_polygon_cylinder_radius * si) + m_center.y;
                simplex_verts[i].z = (-m_half_height + m_foot_offset) + m_center.z;
            }
            else if (ind == 24)
            {
                simplex_verts[i].x = m_center.x + 0.0f;
                simplex_verts[i].y = m_center.y + 0.0f;
                simplex_verts[i].z = m_half_height + m_center.z;
            }
            else
            {
                iassert(ind == 2 * poly_verts::NUM_VERTS_ + 1);
                simplex_verts[i].x = m_center.x + 0.0f;
                simplex_verts[i].y = m_center.y + 0.0f;
                simplex_verts[i].z = -m_half_height + m_center.z;
            }
        }
    }
}


static inline void calc_disc_aabb(
    const phys_vec3 *axis,
    float radius,
    phys_vec3 *aabb_min,
    phys_vec3 *aabb_max)
{
    float v6; // [esp-20h] [ebp-40h]
    float v7; // [esp-14h] [ebp-34h]
    float v8; // [esp-10h] [ebp-30h]

    v8 = axis->x * axis->x;
    v7 = axis->y * axis->y;
    v6 = axis->z * axis->z;

    aabb_max->x = radius * sqrtf(v7 + v6);
    aabb_max->y = radius * sqrtf(v8 + v6);
    aabb_max->z = radius * sqrtf(v8 + v7);

    aabb_min->x = -aabb_max->x;
    aabb_min->y = -aabb_max->y;
    aabb_min->z = -aabb_max->z;
}

void gjk_polygon_cylinder_t::calc_aabb(
    const phys_mat44 *xform,
    phys_vec3 *aabb_min,
    phys_vec3 *aabb_max) const
{
    const phys_vec3 *v5; // eax
    float v6; // [esp+10h] [ebp-188h]
    float v7; // [esp+14h] [ebp-184h]
    float v8; // [esp+50h] [ebp-148h]
    float v9; // [esp+54h] [ebp-144h]
    float m_geom_radius; // [esp+8Ch] [ebp-10Ch]
    phys_vec3 v11; // [esp+ACh] [ebp-ECh] OVERLAPPED BYREF
    phys_vec3 center; // [esp+BCh] [ebp-DCh] BYREF
    float v13; // [esp+CCh] [ebp-CCh]
    float v14; // [esp+D0h] [ebp-C8h]
    float v15; // [esp+D4h] [ebp-C4h]
    float v16; // [esp+DCh] [ebp-BCh]
    float v17; // [esp+E0h] [ebp-B8h]
    float v18; // [esp+E4h] [ebp-B4h]
    const phys_vec3 *v19; // [esp+E8h] [ebp-B0h]
    phys_vec3 v20; // [esp+ECh] [ebp-ACh] BYREF
    float v21; // [esp+FCh] [ebp-9Ch]
    float v22; // [esp+100h] [ebp-98h]
    float v23; // [esp+104h] [ebp-94h]
    float v24; // [esp+10Ch] [ebp-8Ch]
    float v25; // [esp+110h] [ebp-88h]
    float v26; // [esp+114h] [ebp-84h]
    const phys_vec3 *v27; // [esp+118h] [ebp-80h]
    phys_vec3 v28; // [esp+11Ch] [ebp-7Ch] BYREF
    phys_vec3 bot; // [esp+12Ch] [ebp-6Ch] BYREF
    float v30; // [esp+148h] [ebp-50h]
    float v31; // [esp+14Ch] [ebp-4Ch]
    float v32; // [esp+150h] [ebp-48h]
    float negaHalfHeight; // [esp+154h] [ebp-44h]
    const phys_vec3 *v34; // [esp+158h] [ebp-40h]
    phys_vec3 v35; // [esp+15Ch] [ebp-3Ch] BYREF
    float top_12; // [esp+174h] [ebp-24h]
    float v37; // [esp+178h] [ebp-20h]
    float v38; // [esp+17Ch] [ebp-1Ch]
    float m_half_height; // [esp+180h] [ebp-18h]
    const phys_vec3 *p_z; // [esp+184h] [ebp-14h]
    const gjk_polygon_cylinder_t *thisa; // [esp+188h] [ebp-10h]
    //int v42; // [esp+18Ch] [ebp-Ch]
    //void *xforma; // [esp+190h] [ebp-8h]
    //void *aabb_maxa; // [esp+198h] [ebp+0h]
    //
    //v42 = a2;
    //xforma = aabb_maxa;

    assert_mat44_initialized(*xform);

    thisa = this;
    calc_disc_aabb(&xform->z, this->m_polygon_cylinder_radius, aabb_min, aabb_max);
    p_z = &xform->z;
    m_half_height = thisa->m_half_height;
    v38 = m_half_height * xform->z.x;
    v37 = m_half_height * xform->z.y;
    top_12 = m_half_height * xform->z.z;
    v35.x = v38;
    v35.y = v37;
    v35.z = top_12;
    v34 = &xform->z;
    negaHalfHeight = -thisa->m_half_height;
    v32 = negaHalfHeight * xform->z.x;
    v31 = negaHalfHeight * xform->z.y;
    v30 = negaHalfHeight * xform->z.z;
    bot.x = v32;
    bot.y = v31;
    bot.z = v30;
    v27 = phys_min(&v28, &v35, &bot);
    v26 = aabb_min->x + v27->x;
    v25 = aabb_min->y + v27->y;
    v24 = aabb_min->z + v27->z;
    v21 = v26;
    v22 = v25;
    v23 = v24;
    aabb_min->x = v26;
    aabb_min->y = v22;
    aabb_min->z = v23;
    v19 = phys_max(&v20, &v35, &bot);
    v18 = aabb_max->x + v19->x;
    v17 = aabb_max->y + v19->y;
    v16 = aabb_max->z + v19->z;
    v13 = v18;
    v14 = v17;
    v15 = v16;
    aabb_max->x = v18;
    aabb_max->y = v14;
    aabb_max->z = v15;
    if (thisa->m_half_height < thisa->m_head_offset
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.h",
            961,
            0,
            "%s",
            "m_head_offset <= m_half_height"))
    {
        __debugbreak();
    }
    if (thisa->m_half_height < thisa->m_foot_offset
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.h",
            962,
            0,
            "%s",
            "m_foot_offset <= m_half_height"))
    {
        __debugbreak();
    }
    if (thisa->m_polygon_cylinder_radius < thisa->m_capsule_radius
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.h",
            972,
            0,
            "%s",
            "m_capsule_radius <= m_polygon_cylinder_radius"))
    {
        __debugbreak();
    }
    v5 = phys_multiply(&center, xform, &thisa->m_center);
    //operator+(&v11, v5, &xform->w);
    v11 = xform->w + *v5;
    m_geom_radius = thisa->m_geom_radius;
    v8 = (float)(aabb_min->y + v11.y) - m_geom_radius;
    v9 = (float)(aabb_min->z + v11.z) - m_geom_radius;
    aabb_min->x = (float)(aabb_min->x + v11.x) - m_geom_radius;
    aabb_min->y = v8;
    aabb_min->z = v9;
    v6 = (float)(aabb_max->y + v11.y) + m_geom_radius;
    v7 = (float)(aabb_max->z + v11.z) + m_geom_radius;
    aabb_max->x = (float)(aabb_max->x + v11.x) + m_geom_radius;
    aabb_max->y = v6;
    aabb_max->z = v7;
}