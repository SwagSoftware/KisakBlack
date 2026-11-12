#include "phys_auto_rigid_body.h"

#include <bgame/bg_local.h>
#include <cgame_mp/cg_main_mp.h>
#include <qcommon/dobj_management.h>
#include <xanim/dobj_utils.h>
#include <xanim/xmodel_utils.h>
#include <universal/com_math_anglevectors.h>
#include <cgame_mp/cg_ents_mp.h>

auto_rigid_body *g_auto_rigid_body_list;

tlAtomicReadWriteMutex g_auto_rigid_body_map_mutex;
phys_inplace_avl_tree<centity_s const *, auto_rigid_body, auto_rigid_body> g_auto_rigid_body_map;
phys_simple_allocator<auto_rigid_body> g_auto_rigid_body_allocator;

void __cdecl auto_rigid_body::add(const centity_s *cent, gjk_physics_collision_visitor *collision_visitor, int mask)
{
    phys_mat44 *p_m_mat; // [esp+Ch] [ebp-BCh]
    phys_mat44 *outMat; // [esp+28h] [ebp-A0h]
    float *inVector; // [esp+2Ch] [ebp-9Ch]
    float axis[13]; // [esp+34h] [ebp-94h] BYREF
    auto_rigid_body *m_right; // [esp+68h] [ebp-60h]
    auto_rigid_body *m_tree_root; // [esp+94h] [ebp-34h]
    DObj *obj; // [esp+B8h] [ebp-10h]
    XModel *model; // [esp+BCh] [ebp-Ch]
    unsigned int collflag; // [esp+C0h] [ebp-8h]
    auto_rigid_body *arb; // [esp+C4h] [ebp-4h]

    if ( !cent
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_auto_rigid_body.cpp",
                    33,
                    0,
                    "%s",
                    "cent != NULL") )
    {
        __debugbreak();
    }
    if ( cent->currentState.pos.trType != 10
        && cent->nextState.lerp.pos.trType != 10
        && !CG_IsRagdollTrajectory(&cent->currentState.pos)
        && !CG_IsRagdollTrajectory(&cent->nextState.lerp.pos) )
    {
        if (cent->nextState.solid == 0xFFFFFF
            && cent->nextState.eType != 14
            && cent->nextState.eType != 16
            || (obj = Com_GetClientDObj(cent->nextState.number, cent->pose.localClientNum)) != 0
            && ((model = DObjGetModel(obj, 0), XModelHasCollmap(model)) || cent->nextState.eType == 17) )
        {
            //tlAtomicReadWriteMutex::ReadLock(&g_auto_rigid_body_map_mutex);
            g_auto_rigid_body_map_mutex.ReadLock();
            m_tree_root = g_auto_rigid_body_map.m_tree_root;
            while ( m_tree_root && cent != m_tree_root->cent )
            {
                if ( cent >= m_tree_root->cent )
                    m_tree_root = m_tree_root->m_avl_tree_node.m_right;
                else
                    m_tree_root = m_tree_root->m_avl_tree_node.m_left;
            }
            arb = m_tree_root;
            g_auto_rigid_body_map_mutex.ReadUnlock();
            if ( !m_tree_root )
            {
                //tlAtomicReadWriteMutex::WriteLock(&g_auto_rigid_body_map_mutex);
                g_auto_rigid_body_map_mutex.WriteLock();
                m_right = g_auto_rigid_body_map.m_tree_root;
                while ( m_right && cent != m_right->cent )
                {
                    if ( cent >= m_right->cent )
                        m_right = m_right->m_avl_tree_node.m_right;
                    else
                        m_right = m_right->m_avl_tree_node.m_left;
                }
                arb = m_right;
                if ( !m_right )
                {
                    //arb = phys_simple_allocator<auto_rigid_body>::allocate(&g_auto_rigid_body_allocator);
                    arb = g_auto_rigid_body_allocator.allocate();
                    arb->cent = cent;
                    arb->frame_count = 3;
                    arb->rb = phys_sys::create_user_rigid_body(0);
                    outMat = &arb->rb->m_dictator_mat;
                    inVector = (float*)cent->pose.origin;
                    AnglesToAxis(cent->pose.angles, (float (*)[3])axis);
                    Phys_AxisToNitrousMat((float (*)[3])axis, *outMat);
                    Phys_Vec3ToNitrousVec(inVector, outMat->w);
                    //user_rigid_body::set(arb->rb, &arb->rb->m_dictator_mat);
                    arb->rb->set(&arb->rb->m_dictator_mat);
                    //rigid_body::set_flag(arb->rb, 0x40u, 1);
                    arb->rb->set_flag(0x40u, 1);
                    arb->next = g_auto_rigid_body_list;
                    g_auto_rigid_body_list = arb;
                    //phys_inplace_avl_tree<centity_s const *,auto_rigid_body,auto_rigid_body>::add(&g_auto_rigid_body_map, &cent, arb);
                    g_auto_rigid_body_map.add(cent, arb);
                }
                //tlAtomicReadWriteMutex::WriteUnlock(&g_auto_rigid_body_map_mutex);
                g_auto_rigid_body_map_mutex.WriteUnlock();
            }
            collision_visitor->cent = cent;
            collision_visitor->dynEntDef = 0;
            collision_visitor->glass = 0;
            collflag = 2;
            if ( cent->nextState.eType == 17 )
                collflag = 128;
            if ( arb )
            {
                arb->frame_count = 3;
                p_m_mat = &arb->rb->m_mat;
                collision_visitor->rb = arb->rb;
                collision_visitor->rb_to_world_xform = p_m_mat;
                collision_visitor->cg_to_world_xform = p_m_mat;
                collision_visitor->cg_to_rb_xform = &PHYS_IDENTITY_MATRIX;
            }
            else
            {
                collision_visitor->rb = phys_sys::get_environment_rigid_body();
                collision_visitor->rb_to_world_xform = &PHYS_IDENTITY_MATRIX;
                collision_visitor->cg_to_world_xform = 0;
                collision_visitor->cg_to_rb_xform = 0;
            }
            collision_visitor->env_collision_flags = collflag;
            create_entity_bpi(collision_visitor, mask);
        }
    }
}

void    auto_rigid_body::update()
{
    phys_mat44 dictator; // [esp-Ch] [ebp-8Ch] OVERLAPPED BYREF
    float axis[3][3]; // [esp+3Ch] [ebp-44h] BYREF
    auto_rigid_body *iter_next; // [esp+68h] [ebp-18h]
    auto_rigid_body **prev_next; // [esp+6Ch] [ebp-14h]
    auto_rigid_body *iter; // [esp+70h] [ebp-10h]
    //_UNKNOWN *v8; // [esp+74h] [ebp-Ch]
    //int v9; // [esp+78h] [ebp-8h]
    //int vars0; // [esp+80h] [ebp+0h]
    //
    //v8 = a1;
    //v9 = vars0;
    iter = g_auto_rigid_body_list;
    prev_next = &g_auto_rigid_body_list;
    while (iter)
    {
        if (--iter->frame_count <= 0
            || CG_IsRagdollTrajectory(&iter->cent->nextState.lerp.pos)
            || iter->cent->nextState.lerp.pos.trType == 10)
        {
            iter_next = iter->next;
            //phys_inplace_avl_tree<centity_s const *, auto_rigid_body, auto_rigid_body>::remove(&g_auto_rigid_body_map,&iter->cent);
            g_auto_rigid_body_map.remove(iter->cent);
            phys_sys::destroy(iter->rb);
            //phys_simple_allocator<auto_rigid_body>::free(&g_auto_rigid_body_allocator, iter);
            g_auto_rigid_body_allocator.free(iter);
            *prev_next = iter_next;
            iter = iter_next;
        }
        else
        {
            const float *origin = iter->cent->pose.origin;
            const float *angles = iter->cent->pose.angles;
            AnglesToAxis(angles, axis);
            Phys_AxisToNitrousMat(axis, dictator);
            Phys_Vec3ToNitrousVec(origin, dictator.w);
            //user_rigid_body::setPosition(iter->rb, &dictator);
            iter->rb->setPosition(&dictator);
            prev_next = &iter->next;
            iter = iter->next;
        }
    }
}

void __cdecl fixup_wheel_constraints_arb(auto_rigid_body *arb)
{
    //phys_free_list<rigid_body_constraint_wheel>::T_internal_base *wci; // [esp+Ch] [ebp-8h]
    //phys_free_list<rigid_body_constraint_wheel> *wci_end; // [esp+10h] [ebp-4h]
    //
    //wci = g_physics_system->m_list_rbc_wheel.m_dummy_head.m_next_T_internal;
    //wci_end = &g_physics_system->m_list_rbc_wheel;
    //while ( wci_end != (phys_free_list<rigid_body_constraint_wheel> *)wci )
    //{
    //    if ( wci != (phys_free_list<rigid_body_constraint_wheel>::T_internal_base *)-16
    //        && wci[2].m_next_T_internal == (phys_free_list<rigid_body_constraint_wheel>::T_internal_base *)arb->rb )
    //    {
    //        //rigid_body_constraint_wheel::set_no_collision((rigid_body_constraint_wheel *)&wci[2]);
    //        ((rigid_body_constraint_wheel *)&wci[2])->set_no_collision();
    //    }
    //    wci = wci->m_next_T_internal;
    //}

    for (rigid_body_constraint_wheel &wci : g_physics_system->m_list_rbc_wheel)
    {
        if (wci.b2 == arb->rb)
        {
            wci.set_no_collision();
        }
    }
}

void __cdecl auto_rigid_body::remove_ent(const centity_s *cent)
{
    auto_rigid_body *m_tree_root; // [esp+144h] [ebp-Ch]
    auto_rigid_body **prev_next; // [esp+148h] [ebp-8h]

    m_tree_root = g_auto_rigid_body_map.m_tree_root;
    while ( m_tree_root && cent != m_tree_root->cent )
    {
        if ( cent >= m_tree_root->cent )
            m_tree_root = m_tree_root->m_avl_tree_node.m_right;
        else
            m_tree_root = m_tree_root->m_avl_tree_node.m_left;
    }
    if ( m_tree_root )
    {
        prev_next = &g_auto_rigid_body_list;
        while ( *prev_next != m_tree_root )
        {
            prev_next = &(*prev_next)->next;
            if ( !*prev_next
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_auto_rigid_body.cpp",
                            159,
                            0,
                            "%s",
                            "*prev_next") )
            {
                __debugbreak();
            }
        }
        *prev_next = m_tree_root->next;
        //phys_inplace_avl_tree<centity_s const *,auto_rigid_body,auto_rigid_body>::remove(&g_auto_rigid_body_map, &m_tree_root->cent);
        g_auto_rigid_body_map.remove(m_tree_root->cent);
        fixup_wheel_constraints_arb(m_tree_root);
        phys_sys::destroy(m_tree_root->rb);
        //phys_simple_allocator<auto_rigid_body>::free(&g_auto_rigid_body_allocator, m_tree_root);
        g_auto_rigid_body_allocator.free(m_tree_root);
    }
}

user_rigid_body *__cdecl auto_rigid_body::ent_has_auto_rigid_body(const centity_s *cent)
{
    auto_rigid_body *m_tree_root; // [esp+0h] [ebp-8h]

    m_tree_root = g_auto_rigid_body_map.m_tree_root;
    while ( m_tree_root && cent != m_tree_root->cent )
    {
        if ( cent >= m_tree_root->cent )
            m_tree_root = m_tree_root->m_avl_tree_node.m_right;
        else
            m_tree_root = m_tree_root->m_avl_tree_node.m_left;
    }
    if ( !m_tree_root )
        return 0;
    m_tree_root->frame_count = 3;
    return m_tree_root->rb;
}