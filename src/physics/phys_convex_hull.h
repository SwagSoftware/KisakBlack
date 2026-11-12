#pragma once

#include "phys_local.h"

struct cbrush_t;
struct CollisionAabbTree;
struct cLeafBrushNode_s;
struct PhysGeomList;

chull_t *__cdecl create_chull(phys_convex_hull *pch);
void __cdecl free_chull(chull_t *first);
void __cdecl chull_list_add(chull_t *chull, unsigned int key);
void __cdecl free_chull_lists();
chull_t *generate_brush_chull(const cbrush_t *brush);

chull_t *generate_partition_chull(const CollisionAabbTree *tree);
void __cdecl generate_brushmodel_chull_r(cLeafBrushNode_s *node, chull_t **hull);
chull_t *__cdecl generate_brushmodel_chull(unsigned int brushmodel);
chull_t *__cdecl generate_collmap_chull(PhysGeomList *geomList);
chull_t *__cdecl get_collmap_chull(PhysGeomList *geomList, unsigned int key);
chull_t *__cdecl get_brushmodel_chull(unsigned int brushmodel, unsigned int key);
chull_t *__cdecl get_brush_chull(const cbrush_t *brush);
chull_t *__cdecl get_partition_chull(const CollisionAabbTree *tree);