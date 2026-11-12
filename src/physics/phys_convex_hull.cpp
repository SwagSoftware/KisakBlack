#include "phys_convex_hull.h"
#include "phys_gjk_collision_detection.h"
#include "phys_collision.h"

chull_t *chull_list;
phys_convex_hull g_hull;
phys_inplace_avl_tree<unsigned int, generic_avl_map_node_t, generic_avl_map_node_t> entities_map;

chull_t *__cdecl create_chull(phys_convex_hull *pch)
{
    chull_t *hull = (chull_t *)PMM_ALLOC(sizeof(chull_t), 4u);
    hull->nverts = pch->m_convex_hull_vert_list.get_count();
    hull->verts = (phys_vec3 *)PMM_ALLOC(sizeof(phys_vec3) * hull->nverts, 16);
    hull->ninds = 3 * pch->m_convex_hull_triangle_list.get_count();
    hull->inds = (unsigned __int16 *)PMM_ALLOC((2 * hull->ninds + 3) & 0xFFFFFFFC, 4u);
    hull->next = 0;

    // copy vertex positions
    int vert_i = 0;
    phys_vec3 **vi = &*pch->m_convex_hull_vert_list.begin();
    phys_vec3 **vi_end = vi + pch->m_convex_hull_vert_list.get_count();

    while (vi_end != vi)
    {
        phys_vec3 *src = *vi;
        phys_vec3 *dst = &hull->verts[vert_i];

        dst->x = src->x;
        dst->y = src->y;
        dst->z = src->z;

        vi++;
        vert_i++;
    }

    int ind_i = 0;
    phys_convex_hull::ch_triangle *tri = &*pch->m_convex_hull_triangle_list.begin();
    phys_convex_hull::ch_triangle *tri_end = tri + pch->m_convex_hull_triangle_list.get_count();

    while (tri_end != tri)
    {
        unsigned __int16 slot_i = 0;
        phys_vec3 **slot = &*pch->m_convex_hull_vert_list.begin();
        phys_vec3 **slot_end = slot + pch->m_convex_hull_vert_list.get_count();

        iassert((slot_end - slot) < 100);

        while (slot_end != slot)
        {
            if (tri->m_verts[0] == *slot)
            {
                hull->inds[ind_i] = slot_i;
            }
            if (tri->m_verts[1] == *slot)
            {
                hull->inds[ind_i + 1] = slot_i;
            }
            if (tri->m_verts[2] == *slot)
            {
                hull->inds[ind_i + 2] = slot_i;
            }
            slot_i++;
            slot++;

            iassert(slot_i < 100);
        }

        ind_i += 3;
        tri++;
    }

    return hull;
}

void __cdecl free_chull(chull_t *first)
{
    chull_t *next; // [esp+Ch] [ebp-4h]

    while (first)
    {
        next = first->next;
        PMM_FREE((unsigned __int8 *)first->verts, 16 * first->nverts, 0x10u);
        PMM_FREE((unsigned __int8 *)first->inds, (2 * first->ninds + 3) & 0xFFFFFFFC, 4u);
        PMM_FREE((unsigned __int8 *)first, 0x20u, 4u);
        first = next;
    }
}

void __cdecl chull_list_add(chull_t *chull, unsigned int key)
{
    generic_avl_map_add(&entities_map, chull, key);
    chull->key = key;
    chull->next_list = chull_list;
    chull_list = chull;
}

void __cdecl free_chull_lists()
{
    chull_t *next_list; // [esp+0h] [ebp-Ch]
    chull_t *first_list; // [esp+4h] [ebp-8h]
    chull_t **prev_next; // [esp+8h] [ebp-4h]

    first_list = chull_list;
    prev_next = &chull_list;
    while (first_list)
    {
        next_list = first_list->next_list;
        if (first_list->touched == 1)
        {
            first_list->touched = 0;
            prev_next = &first_list->next_list;
        }
        else
        {
            *prev_next = next_list;
            generic_avl_map_destroy(&entities_map, first_list->key);
            free_chull(first_list);
        }
        first_list = next_list;
    }
}

chull_t *generate_brush_chull(const cbrush_t *brush)
{
    phys_vec3 *v3; // eax
    phys_vec3 v4; // [esp+14h] [ebp-2Ch] BYREF
    unsigned int vi; // [esp+2Ch] [ebp-14h]
    int i; // [esp+30h] [ebp-10h]
    //int v7; // [esp+34h] [ebp-Ch] BYREF
    //unsigned int vi; // [esp+38h] [ebp-8h]
    //unsigned int retaddr; // [esp+40h] [ebp+0h]
    //
    //v7 = a1;
    //vi = retaddr;
    if (!brush)
        return 0;

    g_hull.m_vertex_buffer.remove_all();

    for (vi = 0; vi < brush->numverts; ++vi)
    {
        Phys_Vec3ToNitrousVec(brush->verts[vi], v4);
        //v3 = phys_static_array<phys_vec3, 6144>::add(&g_hull.m_vertex_buffer, 0, "phys array add overflow.");
        v3 = g_hull.m_vertex_buffer.add(0, "phys array add overflow.");
        v3->x = v4.x;
        v3->y = v4.y;
        v3->z = v4.z;
    }
    //phys_convex_hull::compute_convex_hull(&g_hull, (int)&v7, 128, 0.0);
    g_hull.compute_convex_hull(128, 0.0);
    return create_chull(&g_hull);
}

chull_t *generate_partition_chull(const CollisionAabbTree *tree)
{
    phys_vec3 *v3; // eax
    phys_vec3 v; // [esp+14h] [ebp-3Ch] BYREF
    int ind_i; // [esp+2Ch] [ebp-24h]
    int ninds; // [esp+30h] [ebp-20h]
    unsigned __int16 *inds; // [esp+34h] [ebp-1Ch]
    const float (*verts)[3]; // [esp+38h] [ebp-18h]
    const CollisionPartition *partition; // [esp+3Ch] [ebp-14h]
    int i; // [esp+40h] [ebp-10h]

    if (!tree)
        return 0;

    g_hull.m_vertex_buffer.remove_all();

    partition = &cm.partitions[tree->u.firstChildIndex];
    verts = cm.verts;
    inds = &cm.uinds[partition->fuind];
    ninds = partition->nuinds;
    for (ind_i = 0; ind_i < ninds; ++ind_i)
    {
        Phys_Vec3ToNitrousVec((float *)verts[inds[ind_i]], v);
        //v3 = phys_static_array<phys_vec3, 6144>::add(&g_hull.m_vertex_buffer, 0, "phys array add overflow.");
        v3 = g_hull.m_vertex_buffer.add(0, "phys array add overflow.");
        v3->x = v.x;
        v3->y = v.y;
        v3->z = v.z;
    }
    //phys_convex_hull::compute_convex_hull(&g_hull, (int)v11, 128, 0.0);
    g_hull.compute_convex_hull(128, 0.0f);
    return create_chull(&g_hull);
}

void __cdecl generate_brushmodel_chull_r(cLeafBrushNode_s *node, chull_t **hull)
{
    chull_t *brush_chull; // eax
    int i; // [esp+Ch] [ebp-4h]
    int savedregs; // [esp+10h] [ebp+0h] BYREF

    if (node->leafBrushCount)
    {
        if (node->leafBrushCount > 0)
        {
            for (i = 0; i < node->leafBrushCount; ++i)
            {
                brush_chull = generate_brush_chull(&cm.brushes[node->data.leaf.brushes[i]]);
                brush_chull->next = *hull;
                *hull = brush_chull;
            }
            return;
        }
        generate_brushmodel_chull_r(node + 1, hull);
    }
    if (node->data.children.childOffset[0] || node->data.children.childOffset[1])
    {
        generate_brushmodel_chull_r(&node[node->data.children.childOffset[0]], hull);
        generate_brushmodel_chull_r(&node[node->data.children.childOffset[1]], hull);
    }
}

chull_t *__cdecl generate_brushmodel_chull(unsigned int brushmodel)
{
    const cmodel_t *model; // [esp+4h] [ebp-8h]
    chull_t *hull; // [esp+8h] [ebp-4h] BYREF

    hull = 0;
    model = CM_ClipHandleToModel(brushmodel);
    generate_brushmodel_chull_r(&cm.leafbrushNodes[model->leaf.leafBrushNode], &hull);
    return hull;
}

chull_t *__cdecl generate_collmap_chull(PhysGeomList *geomList)
{
    chull_t *brush_chull; // eax
    PhysGeomInfo *geom; // [esp+8h] [ebp-Ch]
    chull_t *first; // [esp+Ch] [ebp-8h]
    unsigned int gi; // [esp+10h] [ebp-4h]
    int savedregs; // [esp+14h] [ebp+0h] BYREF

    first = 0;
    gi = 0;
    geom = geomList->geoms;
    while (gi < geomList->count)
    {
        if (geom->brush)
        {
            brush_chull = generate_brush_chull((const cbrush_t *)geom->brush);
            brush_chull->next = first;
            first = brush_chull;
        }
        ++geom;
        ++gi;
    }
    return first;
}

chull_t *__cdecl get_collmap_chull(PhysGeomList *geomList, unsigned int key)
{
    generic_avl_map_node_t *m_tree_root; // [esp+0h] [ebp-Ch]
    chull_t *chull; // [esp+8h] [ebp-4h]

    m_tree_root = entities_map.m_tree_root;
    while (m_tree_root && key != m_tree_root->m_avl_key)
    {
        if (key >= m_tree_root->m_avl_key)
            m_tree_root = m_tree_root->m_avl_tree_node.m_right;
        else
            m_tree_root = m_tree_root->m_avl_tree_node.m_left;
    }
    if (m_tree_root)
    {
        chull = (chull_t *)m_tree_root->m_data;
    }
    else
    {
        chull = generate_collmap_chull(geomList);
        if (!chull)
            return 0;
        chull_list_add(chull, key);
    }
    if (!chull && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp", 1789, 0, "%s", "chull"))
        __debugbreak();
    chull->touched = 1;
    return chull;
}

chull_t *__cdecl get_brushmodel_chull(unsigned int brushmodel, unsigned int key)
{
    generic_avl_map_node_t *m_tree_root; // [esp+0h] [ebp-Ch]
    chull_t *chull; // [esp+8h] [ebp-4h]

    m_tree_root = entities_map.m_tree_root;
    while (m_tree_root && key != m_tree_root->m_avl_key)
    {
        if (key >= m_tree_root->m_avl_key)
            m_tree_root = m_tree_root->m_avl_tree_node.m_right;
        else
            m_tree_root = m_tree_root->m_avl_tree_node.m_left;
    }
    if (m_tree_root)
    {
        chull = (chull_t *)m_tree_root->m_data;
    }
    else
    {
        chull = generate_brushmodel_chull(brushmodel);
        if (!chull)
            return 0;
        chull_list_add(chull, key);
    }
    if (!chull && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp", 1810, 0, "%s", "chull"))
        __debugbreak();
    chull->touched = 1;
    return chull;
}

chull_t *__cdecl get_brush_chull(const cbrush_t *brush)
{
    generic_avl_map_node_t *m_tree_root; // [esp+0h] [ebp-10h]
    chull_t *chull; // [esp+8h] [ebp-8h]
    int savedregs; // [esp+10h] [ebp+0h] BYREF

    m_tree_root = entities_map.m_tree_root;
    while (m_tree_root && brush != (const cbrush_t *)m_tree_root->m_avl_key)
    {
        if ((unsigned int)brush >= m_tree_root->m_avl_key)
            m_tree_root = m_tree_root->m_avl_tree_node.m_right;
        else
            m_tree_root = m_tree_root->m_avl_tree_node.m_left;
    }
    if (m_tree_root)
    {
        chull = (chull_t *)m_tree_root->m_data;
    }
    else
    {
        chull = generate_brush_chull(brush);
        if (!chull)
            return 0;
        chull_list_add(chull, (unsigned int)brush);
    }
    if (!chull && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp", 1832, 0, "%s", "chull"))
        __debugbreak();
    chull->touched = 1;
    return chull;
}

chull_t *__cdecl get_partition_chull(const CollisionAabbTree *tree)
{
    generic_avl_map_node_t *m_tree_root; // [esp+0h] [ebp-10h]
    chull_t *chull; // [esp+8h] [ebp-8h]
    int savedregs; // [esp+10h] [ebp+0h] BYREF

    m_tree_root = entities_map.m_tree_root;
    while (m_tree_root && tree != (const CollisionAabbTree *)m_tree_root->m_avl_key)
    {
        if ((unsigned int)tree >= m_tree_root->m_avl_key)
            m_tree_root = m_tree_root->m_avl_tree_node.m_right;
        else
            m_tree_root = m_tree_root->m_avl_tree_node.m_left;
    }
    if (m_tree_root)
    {
        chull = (chull_t *)m_tree_root->m_data;
    }
    else
    {
        chull = generate_partition_chull(tree);
        if (!chull)
            return 0;
        chull_list_add(chull, (unsigned int)tree);
    }
    if (!chull && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp", 1854, 0, "%s", "chull"))
        __debugbreak();
    chull->touched = 1;
    return chull;
}