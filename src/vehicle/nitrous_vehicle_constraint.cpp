#include "nitrous_vehicle_constraint.h"
#include <cgame/cg_drawtools.h>
#include <universal/com_math_anglevectors.h>
#include <game/g_scr_vehicle.h>
#include <physics/phys_main.h>

void    path_constraint_update(rigid_body_constraint_custom_path *vpc, gentity_s *veh)
{
    float v3[9]; // [esp+58h] [ebp-38h] BYREF
    phys_mat44 *p_m_path_mat; // [esp+7Ch] [ebp-14h]
    scr_vehicle_s *scr_vehicle; // [esp+80h] [ebp-10h]
    //int v6; // [esp+84h] [ebp-Ch]
    //void *v7; // [esp+88h] [ebp-8h]
    //void *retaddr; // [esp+90h] [ebp+0h]
    //
    //v6 = a1;
    //v7 = retaddr;
    if ( vpc && veh && veh->scr_vehicle && veh->scr_vehicle->nitrousVehicle )
    {
        scr_vehicle = veh->scr_vehicle;
        p_m_path_mat = &vpc->m_path_mat;
        AnglesToAxis(scr_vehicle->pathPos.angles, (float (*)[3])v3);
        Phys_AxisToNitrousMat((float (*)[3])v3, *p_m_path_mat);
        Phys_Vec3ToNitrousVec(scr_vehicle->pathPos.origin, p_m_path_mat->w);
        if ( ((LODWORD(scr_vehicle->pathPos.origin[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(scr_vehicle->pathPos.origin[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(scr_vehicle->pathPos.origin[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle_constraint.cpp",
                        22,
                        0,
                        "%s",
                        "!IS_NAN((vpp.origin)[0]) && !IS_NAN((vpp.origin)[1]) && !IS_NAN((vpp.origin)[2])") )
        {
            __debugbreak();
        }
        if ( ((LODWORD(scr_vehicle->pathPos.lookPos[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(scr_vehicle->pathPos.lookPos[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(scr_vehicle->pathPos.lookPos[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle_constraint.cpp",
                        23,
                        0,
                        "%s",
                        "!IS_NAN((vpp.lookPos)[0]) && !IS_NAN((vpp.lookPos)[1]) && !IS_NAN((vpp.lookPos)[2])") )
        {
            __debugbreak();
        }
        if ( phys_drawNitrousVehicle->current.enabled )
            CG_DebugSphere(scr_vehicle->pathPos.origin, 15.0, colorGreen, 10, 0, 1);
        vpc->b1_r_loc.x = 0.0f;
        vpc->b1_r_loc.y = 0.0f;
        vpc->b1_r_loc.z = 0.0f;
        if ( veh->scr_vehicle->nitrousVehicle->m_num_colliding_wheels >= 3
            || veh->scr_vehicle->nitrousVehicle->m_vehicle_info->type == 4 )
        {
            vpc->b1_r_loc.z = -6.0f;
        }
    }
}

rigid_body_constraint_custom_path * path_constraint_create(gentity_s *veh)
{
    float v3[9]; // [esp-50h] [ebp-68h] BYREF
    phys_mat44 *p_m_path_mat; // [esp-2Ch] [ebp-44h]
    phys_vec3 *p_b1_r_loc; // [esp-28h] [ebp-40h]
    int v6; // [esp-24h] [ebp-3Ch]
    float v7; // [esp-20h] [ebp-38h]
    float v8; // [esp-1Ch] [ebp-34h]
    rigid_body_constraint_custom_path *v9; // [esp-Ch] [ebp-24h]
    user_rigid_body *user_rb; // [esp-8h] [ebp-20h]
    environment_rigid_body *b1; // [esp-4h] [ebp-1Ch]
    rigid_body_constraint_custom_path *rbc_custom_path; // [esp+0h] [ebp-18h]
    user_rigid_body *urb; // [esp+4h] [ebp-14h]
    rigid_body *rb; // [esp+8h] [ebp-10h]
    //int v15; // [esp+Ch] [ebp-Ch]
    //void *v16; // [esp+10h] [ebp-8h]
    //void *retaddr; // [esp+18h] [ebp+0h]
    //
    //v15 = a1;
    //v16 = retaddr;
    if ( !veh
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle_constraint.cpp",
                 38,
                 "veh",
                 "") )
    {
        __debugbreak();
    }
    if ( !veh->scr_vehicle
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle_constraint.cpp",
                 39,
                 "veh->scr_vehicle",
                 "") )
    {
        __debugbreak();
    }
    if ( !veh->scr_vehicle->nitrousVehicle
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle_constraint.cpp",
                 40,
                 "veh->scr_vehicle->nitrousVehicle",
                 "") )
    {
        __debugbreak();
    }
    rb = (rigid_body *)veh->scr_vehicle->nitrousVehicle->m_phys_user_data;
    if ( !rb
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle_constraint.cpp",
                 41,
                 "veh->scr_vehicle->nitrousVehicle->get_phys_user_data()",
                 "") )
    {
        __debugbreak();
    }
    urb = (user_rigid_body *)veh->scr_vehicle->nitrousVehicle->m_phys_user_data;
    if ( !LODWORD(urb->m_last_position.x)
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle_constraint.cpp",
                 42,
                 "veh->scr_vehicle->nitrousVehicle->get_phys_user_data()->body",
                 "") )
    {
        __debugbreak();
    }
    rbc_custom_path = (rigid_body_constraint_custom_path *)veh->scr_vehicle->nitrousVehicle->m_phys_user_data;
    b1 = (environment_rigid_body *)rbc_custom_path->b1;
    user_rb = phys_sys::create_user_rigid_body(0);
    //user_rigid_body::set(user_rb, 0);
    user_rb->set(0);
    v9 = phys_sys::create_rbc_custom_path(b1, (environment_rigid_body *)user_rb, 0);
    if ( !v9
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle_constraint.cpp",
                 49,
                 "rbc_custom_path",
                 "") )
    {
        __debugbreak();
    }
    v6 = 0;
    v7 = 0.0f;
    v8 = 0.0f;
    p_b1_r_loc = &v9->b1_r_loc;
    v9->b1_r_loc.x = 0.0f;
    p_b1_r_loc->y = v7;
    p_b1_r_loc->z = v8;
    p_m_path_mat = &v9->m_path_mat;
    AnglesToAxis(veh->r.currentAngles, (float (*)[3])v3);
    Phys_AxisToNitrousMat((float (*)[3])v3, *p_m_path_mat);
    Phys_Vec3ToNitrousVec(veh->r.currentOrigin, p_m_path_mat->w);
    v9->m_urb = user_rb;
    //user_rb::set(v9->m_urb, &v9->m_path_mat);
    v9->m_urb->set(&v9->m_path_mat);
    return v9;
}

void __cdecl path_constraint_destroy(rigid_body_constraint_custom_path *vpc)
{
    if ( !vpc->m_urb
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle_constraint.cpp",
                 60,
                 "vpc->m_urb",
                 "") )
    {
        __debugbreak();
    }
    if ( vpc->b2 != vpc->m_urb
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle_constraint.cpp",
                 61,
                 "vpc->get_b2() == vpc->m_urb",
                 "") )
    {
        __debugbreak();
    }
    phys_sys::destroy(vpc->m_urb);
}

