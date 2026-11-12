#pragma once

#include <physics/phys_collision.h>

struct gentity_s;

void    path_constraint_update(rigid_body_constraint_custom_path *vpc, gentity_s *veh);
rigid_body_constraint_custom_path * path_constraint_create(gentity_s *veh);
void __cdecl path_constraint_destroy(rigid_body_constraint_custom_path *vpc);
