#pragma once
#include "phys_collision.h"
#include "phys_gjk_collision_detection.h"

struct centity_s;

struct auto_rigid_body // sizeof=0x1C
{
    // for avl tree access
    static const centity_s *&get_avl_key(auto_rigid_body *n) { return n->cent; }
    static const centity_s *const &get_avl_key(const auto_rigid_body *n) { return n->cent; }
    static phys_inplace_avl_tree_node<auto_rigid_body> *get_avl_node(auto_rigid_body *n) { return &n->m_avl_tree_node; }
    static const phys_inplace_avl_tree_node<auto_rigid_body> *get_avl_node(const auto_rigid_body *n) { return &n->m_avl_tree_node; }

    user_rigid_body *rb;
    const centity_s *cent;
    int frame_count;
    auto_rigid_body *next;
    phys_inplace_avl_tree_node<auto_rigid_body> m_avl_tree_node;

    static void add(const centity_s *cent, gjk_physics_collision_visitor *collision_visitor, int mask);
    static void remove_ent(const centity_s *cent);
    static user_rigid_body *ent_has_auto_rigid_body(const centity_s *cent);
    static void update();
};

void __cdecl fixup_wheel_constraints_arb(auto_rigid_body *arb);