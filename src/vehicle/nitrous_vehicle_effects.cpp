#include "nitrous_vehicle_effects.h"
#include "nitrous_vehicle.h"
#include <client/cl_debugdata.h>
#include <bgame/bg_slidemove.h>
#include <physics/phys_render.h>
#include <game/g_debug.h>

float s_ableToSkidSpeed = 100.0;
float s_tireSkidSpeedMin = 100.0;
float s_tireSkidSpeedMax = 600.0;


void NitrousVehicle::_update_wheel_effect_states(float delta_t)
{
    double max_speed; // st7
    double v4; // st7
    float m_forward_vel; // [esp-10h] [ebp-1A8h]
    float v6; // [esp-8h] [ebp-1A0h]
    float skid_delta; // [esp+0h] [ebp-198h]
    float speed; // [esp+4h] [ebp-194h]
    float v9; // [esp+18h] [ebp-180h]
    float v10; // [esp+40h] [ebp-158h]
    float v11; // [esp+44h] [ebp-154h]
    phys_vec3 v12; // [esp+60h] [ebp-138h] BYREF
    float m_wheel_vel; // [esp+84h] [ebp-114h]
    float v14; // [esp+88h] [ebp-110h]
    phys_vec3 m_b1_wheel_axis_loc; // [esp+8Ch] [ebp-10Ch] BYREF
    phys_vec3 v16; // [esp+9Ch] [ebp-FCh] BYREF
    phys_vec3 p_vel; // [esp+ACh] [ebp-ECh] BYREF
    float v18; // [esp+BCh] [ebp-DCh]
    float v19[7]; // [esp+C0h] [ebp-D8h] BYREF
    phys_vec3 p; // [esp+DCh] [ebp-BCh] BYREF
    float v21; // [esp+F0h] [ebp-A8h]
    float v22; // [esp+F4h] [ebp-A4h]
    float v23; // [esp+F8h] [ebp-A0h]
    phys_vec3 m_b1_wheel_center_loc; // [esp+FCh] [ebp-9Ch]
    float v25; // [esp+10Ch] [ebp-8Ch]
    float v26; // [esp+110h] [ebp-88h]
    float v27; // [esp+114h] [ebp-84h]
    float v28; // [esp+124h] [ebp-74h]
    float v29; // [esp+128h] [ebp-70h]
    float v30; // [esp+12Ch] [ebp-6Ch]
    float v31; // [esp+130h] [ebp-68h]
    float m_wheel_displaced_center_dist; // [esp+134h] [ebp-64h]
    float m_wheel_radius; // [esp+138h] [ebp-60h]
    phys_vec3 m_b1_suspension_dir_loc; // [esp+13Ch] [ebp-5Ch]
    phys_vec3 a_vel_loc; // [esp+14Ch] [ebp-4Ch] BYREF
    phys_vec3 t_vel_loc; // [esp+15Ch] [ebp-3Ch] BYREF
    int v37; // [esp+170h] [ebp-28h]
    float v38; // [esp+174h] [ebp-24h]
    WheelEffectState oldState; // [esp+178h] [ebp-20h]
    rigid_body_constraint_wheel *rbc_wheel; // [esp+17Ch] [ebp-1Ch]
    int i; // [esp+180h] [ebp-18h]
    rigid_body *rb; // [esp+184h] [ebp-14h]
    NitrousVehicle *thisa; // [esp+188h] [ebp-10h]
    //_UNKNOWN *v44; // [esp+18Ch] [ebp-Ch]
    //float delta_ta; // [esp+190h] [ebp-8h]
    //int vars0; // [esp+198h] [ebp+0h]
    //
    //v44 = a2;
    //LODWORD(delta_ta) = vars0;
    thisa = this;
    if ((this->m_flags & 1) == 0)
    {
        rb = thisa->m_phys_user_data->body;
        for (i = 0; i < 4; ++i)
        {
            rbc_wheel = thisa->m_wheels[i];
            if (rbc_wheel)
            {
                oldState = thisa->m_wheel_state[i].m_last_state;
                thisa->m_wheel_state[i].m_rate = 0.0f;
                if ((rbc_wheel->m_wheel_flags & 1) != 0)
                {
                    if ((thisa->m_state_flags & 2) != 0
                        && (v38 = thisa->m_forward_vel,
                            v37 = fabs(v38),
                            (fabs(v38)) > s_ableToSkidSpeed))
                    {
                        thisa->m_wheel_state[i].m_state = WHEEL_STATE_SKIDDING;
                        thisa->m_wheel_state[i].m_rate = 1.0f;
                    }
                    else if ((rbc_wheel->m_wheel_flags & 4) != 0)
                    {
                        t_vel_loc = phys_inv_multiply(rb->m_mat, rb->m_t_vel);
                        a_vel_loc = phys_inv_multiply(rb->m_mat, rb->m_a_vel);
                        m_b1_suspension_dir_loc = rbc_wheel->m_b1_suspension_dir_loc;
                        m_wheel_radius = rbc_wheel->m_wheel_radius;
                        m_wheel_displaced_center_dist = rbc_wheel->m_wheel_displaced_center_dist;
                        v31 = m_wheel_radius - m_wheel_displaced_center_dist;
                        v30 = (float)(m_wheel_radius - m_wheel_displaced_center_dist) * m_b1_suspension_dir_loc.x;
                        v29 = (float)(m_wheel_radius - m_wheel_displaced_center_dist) * m_b1_suspension_dir_loc.y;
                        v28 = (float)(m_wheel_radius - m_wheel_displaced_center_dist) * m_b1_suspension_dir_loc.z;
                        v25 = v30;
                        v26 = v29;
                        v27 = v28;
                        m_b1_wheel_center_loc = rbc_wheel->m_b1_wheel_center_loc;
                        v23 = m_b1_wheel_center_loc.x + v30;
                        v22 = m_b1_wheel_center_loc.y + v29;
                        v21 = m_b1_wheel_center_loc.z + v28;
                        p.x = m_b1_wheel_center_loc.x + v30;
                        p.y = m_b1_wheel_center_loc.y + v29;
                        p.z = m_b1_wheel_center_loc.z + v28;
                        phys_vec3 tmp_cross;
                        //LODWORD(v19[2]) = phys_cross(&tmp_cross, &a_vel_loc, &p);
                        tmp_cross = phys_cross(a_vel_loc, p);
                        //v19[1] = t_vel_loc.x + *(float *)LODWORD(v19[2]);
                        //v19[0] = t_vel_loc.y + *(float *)(LODWORD(v19[2]) + 4);
                        //v18 = t_vel_loc.z + *(float *)(LODWORD(v19[2]) + 8);
                        v19[1] = t_vel_loc.x + tmp_cross.x;
                        v19[0] = t_vel_loc.y + tmp_cross.y;
                        v18 = t_vel_loc.z + tmp_cross.z;
                        p_vel.x = v19[1];
                        p_vel.y = v19[0];
                        p_vel.z = v18;
                        v16 = rbc_wheel->m_b1_suspension_dir_loc;
                        m_b1_wheel_axis_loc = rbc_wheel->m_b1_wheel_axis_loc;
                        v14 = rbc_wheel->m_wheel_radius;
                        m_wheel_vel = rbc_wheel->m_wheel_vel;
                        //LODWORD(v12.z) = phys_cross((phys_vec3 *)&v12.w, &m_b1_wheel_axis_loc, &v16);
                        tmp_cross = phys_cross(m_b1_wheel_axis_loc, v16);
                        //v10 = (float)(v14 * m_wheel_vel) * *(float *)(LODWORD(v12.z) + 4);
                        //v11 = (float)(v14 * m_wheel_vel) * *(float *)(LODWORD(v12.z) + 8);
                        //p_vel.x = p_vel.x + (float)((float)(v14 * m_wheel_vel) * *(float *)LODWORD(v12.z));

                        v10 = (float)(v14 * m_wheel_vel) * tmp_cross.y;
                        v11 = (float)(v14 * m_wheel_vel) * tmp_cross.z;
                        p_vel.x = p_vel.x + (float)((float)(v14 * m_wheel_vel) * tmp_cross.x);

                        p_vel.y = p_vel.y + v10;
                        p_vel.z = p_vel.z + v11;
                        v9 = (float)((float)(p_vel.x * rbc_wheel->m_b1_suspension_dir_loc.x)
                            + (float)(p_vel.y * rbc_wheel->m_b1_suspension_dir_loc.y))
                            + (float)(p_vel.z * rbc_wheel->m_b1_suspension_dir_loc.z);
                        skid_delta = v9 * rbc_wheel->m_b1_suspension_dir_loc.y;
                        speed = v9 * rbc_wheel->m_b1_suspension_dir_loc.z;
                        p_vel.x = p_vel.x - (float)(v9 * rbc_wheel->m_b1_suspension_dir_loc.x);
                        p_vel.y = p_vel.y - skid_delta;
                        p_vel.z = p_vel.z - speed;
                        v6 = Vec3Length(&p_vel.x);
                        if (v6 <= thisa->m_vehicle_info->skidSpeedMin)
                        {
                            thisa->m_wheel_state[i].m_state = WHEEL_STATE_ROLLING;
                            max_speed = NitrousVehicle::get_max_speed(0);
                            thisa->m_wheel_state[i].m_rate = v6 / max_speed;
                        }
                        else
                        {
                            thisa->m_wheel_state[i].m_state = WHEEL_STATE_SKIDDING;
                            thisa->m_wheel_state[i].m_rate = (float)(v6 - thisa->m_vehicle_info->skidSpeedMin)
                                / (float)(thisa->m_vehicle_info->skidSpeedMax
                                    - thisa->m_vehicle_info->skidSpeedMin);
                        }
                    }
                    else
                    {
                        thisa->m_wheel_state[i].m_state = WHEEL_STATE_ROLLING;
                        m_forward_vel = thisa->m_forward_vel;
                        v4 = NitrousVehicle::get_max_speed(0);
                        thisa->m_wheel_state[i].m_rate = m_forward_vel / v4;
                    }
                }
                else
                {
                    thisa->m_wheel_state[i].m_state = WHEEL_STATE_AIRBORN;
                    thisa->m_wheel_state[i].m_rate = 0.0f;
                }
                if (oldState == thisa->m_wheel_state[i].m_state)
                {
                    thisa->m_wheel_state[i].m_state_time = thisa->m_wheel_state[i].m_state_time + delta_t;
                }
                else
                {
                    thisa->m_wheel_state[i].m_last_state = oldState;
                    thisa->m_wheel_state[i].m_state_time = 0.0f;
                }
            }
        }
    }
}

void __cdecl GetWheelStateColor(WheelEffectState state, float *color, float rate)
{
    if ( state )
    {
        if ( state == WHEEL_STATE_AIRBORN )
        {
            *color = colorLtBlue[0];
            color[1] = 0.5;
            color[2] = 1.0;
            color[3] = 1.0;
        }
        else if ( state == WHEEL_STATE_SKIDDING )
        {
            Vec4Lerp(colorYellow, colorRed, rate, color);
        }
    }
    else
    {
        *color = colorGreen[0];
        color[1] = 1.0;
        color[2] = 0.0;
        color[3] = 1.0;
    }
}

double NitrousVehicle::debug_render_wheel_effects(int wheel)
{
    phys_vec3 *v4; // eax
    const phys_vec3 *v5; // eax
    float v6; // xmm0_4
    phys_vec3 *p_w; // [esp+10h] [ebp-290h]
    phys_vec3 end; // [esp+24h] [ebp-27Ch] BYREF
    float v9; // [esp+38h] [ebp-268h]
    float v10; // [esp+3Ch] [ebp-264h]
    float skidSpeedMax; // [esp+40h] [ebp-260h]
    phys_vec3 direction; // [esp+44h] [ebp-25Ch] BYREF
    float speed; // [esp+60h] [ebp-240h]
    phys_vec3 v14; // [esp+64h] [ebp-23Ch] BYREF
    float v15; // [esp+74h] [ebp-22Ch]
    float v16; // [esp+78h] [ebp-228h]
    float v17; // [esp+7Ch] [ebp-224h]
    float v18; // [esp+80h] [ebp-220h]
    phys_vec3 v19; // [esp+84h] [ebp-21Ch]
    phys_vec3 v20; // [esp+94h] [ebp-20Ch]
    float v21; // [esp+A4h] [ebp-1FCh]
    float v22; // [esp+A8h] [ebp-1F8h]
    float v23; // [esp+ACh] [ebp-1F4h]
    float v24; // [esp+C0h] [ebp-1E0h]
    float v25; // [esp+C4h] [ebp-1DCh]
    float v26; // [esp+C8h] [ebp-1D8h]
    float v27; // [esp+CCh] [ebp-1D4h]
    phys_vec3 *v28; // [esp+D0h] [ebp-1D0h]
    phys_vec3 v29; // [esp+D4h] [ebp-1CCh] BYREF
    float m_wheel_vel; // [esp+ECh] [ebp-1B4h]
    float v31; // [esp+F0h] [ebp-1B0h]
    phys_vec3 v32; // [esp+F4h] [ebp-1ACh] BYREF
    phys_vec3 v33; // [esp+104h] [ebp-19Ch] BYREF
    phys_vec3 p_vel; // [esp+114h] [ebp-18Ch] BYREF
    float v35; // [esp+124h] [ebp-17Ch]
    float v36[2]; // [esp+128h] [ebp-178h]
    phys_vec3 *v37; // [esp+130h] [ebp-170h]
    phys_vec3 v38; // [esp+134h] [ebp-16Ch] BYREF
    phys_vec3 p; // [esp+144h] [ebp-15Ch] BYREF
    float v40; // [esp+158h] [ebp-148h]
    float v41; // [esp+15Ch] [ebp-144h]
    float v42; // [esp+160h] [ebp-140h]
    phys_vec3 m_b1_wheel_center_loc; // [esp+164h] [ebp-13Ch]
    float v44; // [esp+174h] [ebp-12Ch]
    float v45; // [esp+178h] [ebp-128h]
    float v46; // [esp+17Ch] [ebp-124h]
    float v47; // [esp+18Ch] [ebp-114h]
    float v48; // [esp+190h] [ebp-110h]
    float v49; // [esp+194h] [ebp-10Ch]
    float v50; // [esp+198h] [ebp-108h]
    float m_wheel_displaced_center_dist; // [esp+19Ch] [ebp-104h]
    float m_wheel_radius; // [esp+1A0h] [ebp-100h]
    phys_vec3 m_b1_suspension_dir_loc; // [esp+1A4h] [ebp-FCh]
    phys_vec3 a_vel_loc; // [esp+1B4h] [ebp-ECh] BYREF
    phys_vec3 t_vel_loc; // [esp+1C4h] [ebp-DCh] BYREF
    rigid_body *rb; // [esp+1D8h] [ebp-C8h]
    float _pos[3]; // [esp+1DCh] [ebp-C4h] BYREF
    float _dir[3]; // [esp+1E8h] [ebp-B8h] BYREF
    phys_vec3 dir; // [esp+1F4h] [ebp-ACh] BYREF
    phys_vec3 m_b1_wheel_axis_loc; // [esp+204h] [ebp-9Ch] BYREF
    phys_vec3 v61; // [esp+214h] [ebp-8Ch] BYREF
    phys_vec3 v62; // [esp+224h] [ebp-7Ch] BYREF
    phys_mat44 *p_m_mat; // [esp+240h] [ebp-60h]
    phys_vec3 wheel_pos; // [esp+244h] [ebp-5Ch] BYREF
    phys_vec3 v65; // [esp+254h] [ebp-4Ch] BYREF
    float color[4]; // [esp+26Ch] [ebp-34h] BYREF
    rigid_body *b1; // [esp+27Ch] [ebp-24h]
    float radius; // [esp+280h] [ebp-20h]
    float rate; // [esp+284h] [ebp-1Ch]
    WheelEffectState state; // [esp+288h] [ebp-18h]
    rigid_body_constraint_wheel *rbc_wheel; // [esp+28Ch] [ebp-14h]
    NitrousVehicle *thisa; // [esp+290h] [ebp-10h]
    //_UNKNOWN *v73; // [esp+294h] [ebp-Ch]
    //int wheela; // [esp+298h] [ebp-8h]
    //int vars0; // [esp+2A0h] [ebp+0h]
    //
    //v73 = (_UNKNOWN *)a2;
    //wheela = vars0;
    thisa = this;
    rbc_wheel = this->m_wheels[wheel];
    if (!rbc_wheel)
        return 0.0;
    state = thisa->m_wheel_state[wheel].m_state;
    rate = thisa->m_wheel_state[wheel].m_rate;
    radius = rbc_wheel->m_wheel_radius;
    b1 = rbc_wheel->b1;
    GetWheelStateColor(state, color, rate);
    //v4 = operator*(&v65, rbc_wheel->m_wheel_displaced_center_dist, &rbc_wheel->m_b1_suspension_dir_loc);
    v65 = rbc_wheel->m_b1_suspension_dir_loc * rbc_wheel->m_wheel_displaced_center_dist;
    //operator-(&wheel_pos, &rbc_wheel->m_b1_wheel_center_loc, v4);
    wheel_pos = rbc_wheel->m_b1_wheel_center_loc - v65;
    p_m_mat = &b1->m_mat;
    p_w = &b1->m_mat.w;
    v62 = phys_multiply(b1->m_mat, wheel_pos);
    v5 = &v62;
    v61 = *v5 + *p_w;
    //operator+(&v61, v5, p_w);
    wheel_pos.x = v61.x;
    wheel_pos.y = v61.y;
    wheel_pos.z = v61.z;
    m_b1_wheel_axis_loc = rbc_wheel->m_b1_wheel_axis_loc;
    dir = phys_multiply(b1->m_mat, m_b1_wheel_axis_loc);
    Phys_NitrousVecToVec3(dir, _dir);
    Phys_NitrousVecToVec3(wheel_pos, _pos);
    G_DebugCircleEx(_pos, radius, _dir, color, 0, 0);
    rb = thisa->m_phys_user_data->body;
    t_vel_loc = phys_inv_multiply(rb->m_mat, rb->m_t_vel);
    a_vel_loc = phys_inv_multiply(rb->m_mat, rb->m_a_vel);
    m_b1_suspension_dir_loc = rbc_wheel->m_b1_suspension_dir_loc;
    m_wheel_radius = rbc_wheel->m_wheel_radius;
    m_wheel_displaced_center_dist = rbc_wheel->m_wheel_displaced_center_dist;
    v50 = m_wheel_radius - m_wheel_displaced_center_dist;
    v49 = (float)(m_wheel_radius - m_wheel_displaced_center_dist) * m_b1_suspension_dir_loc.x;
    v48 = (float)(m_wheel_radius - m_wheel_displaced_center_dist) * m_b1_suspension_dir_loc.y;
    v47 = (float)(m_wheel_radius - m_wheel_displaced_center_dist) * m_b1_suspension_dir_loc.z;
    v44 = v49;
    v45 = v48;
    v46 = v47;
    m_b1_wheel_center_loc = rbc_wheel->m_b1_wheel_center_loc;
    v42 = m_b1_wheel_center_loc.x + v49;
    v41 = m_b1_wheel_center_loc.y + v48;
    v40 = m_b1_wheel_center_loc.z + v47;
    p.x = m_b1_wheel_center_loc.x + v49;
    p.y = m_b1_wheel_center_loc.y + v48;
    p.z = m_b1_wheel_center_loc.z + v47;
    v38 = phys_cross(a_vel_loc, p);
    v37 = &v38;
    v36[1] = t_vel_loc.x + v37->x;
    v36[0] = t_vel_loc.y + v37->y;
    v35 = t_vel_loc.z + v37->z;
    p_vel.x = v36[1];
    p_vel.y = v36[0];
    p_vel.z = v35;
    v33 = rbc_wheel->m_b1_suspension_dir_loc;
    v32 = rbc_wheel->m_b1_wheel_axis_loc;
    v31 = rbc_wheel->m_wheel_radius;
    m_wheel_vel = rbc_wheel->m_wheel_vel;
    v29 = phys_cross(v32, v33);
    v28 = &v29;
    v27 = v31 * m_wheel_vel;
    v26 = (float)(v31 * m_wheel_vel) * v28->x;
    v25 = (float)(v31 * m_wheel_vel) * v28->y;
    v24 = (float)(v31 * m_wheel_vel) * v28->z;
    v21 = v26;
    v22 = v25;
    v23 = v24;
    p_vel.x = p_vel.x + v26;
    p_vel.y = p_vel.y + v25;
    p_vel.z = p_vel.z + v24;
    v20 = rbc_wheel->m_b1_suspension_dir_loc;
    v19 = rbc_wheel->m_b1_suspension_dir_loc;
    v18 = (float)((float)(p_vel.x * v19.x) + (float)(p_vel.y * v19.y)) + (float)(p_vel.z * v19.z);
    v17 = v18 * v20.x;
    v16 = v18 * v20.y;
    v15 = v18 * v20.z;
    v14.x = v18 * v20.x;
    v14.y = v18 * v20.y;
    v14.z = v18 * v20.z;
    //phys_vec3::operator-=(&p_vel, &v14);
    p_vel -= v14;
    speed = Vec3Length(&p_vel.x);
    //operator/(&direction, &p_vel, speed);
    direction = p_vel / speed;
    skidSpeedMax = thisa->m_vehicle_info->skidSpeedMax;
    if ((float)(speed - skidSpeedMax) < 0.0)
        v6 = speed;
    else
        v6 = skidSpeedMax;
    v10 = v6;
    if ((float)(0.0 - speed) < 0.0)
        v9 = v10;
    else
        v9 = 0.0f;
    //operator*(&end, &direction, v9);
    end = direction * v9;
    if (NitrousVehicle::is_peeling_out())
        Phys_DebugPoint(&wheel_pos, radius, colorMagenta);
    else
        Phys_DebugPoint(&wheel_pos, radius, color);
    return speed;
}

//phys_vec3 *__cdecl operator/(phys_vec3 *result, const phys_vec3 *a, float d)
//{
//    float v4; // [esp+4h] [ebp-Ch]
//    float v5; // [esp+8h] [ebp-8h]
//
//    v4 = a->y * (float)(1.0 / d);
//    v5 = a->z * (float)(1.0 / d);
//    result->x = a->x * (float)(1.0 / d);
//    result->y = v4;
//    result->z = v5;
//    return result;
//}

float scale_2 = 0.1f;
void    NitrousVehicle::debug_render_effects()
{
    double v2; // st7
    char buffer[508]; // [esp+28h] [ebp-244h] BYREF
    float textPos[3]; // [esp+224h] [ebp-48h] BYREF
    phys_vec3 pos; // [esp+230h] [ebp-3Ch] BYREF
    phys_mat44 *p_m_mat; // [esp+240h] [ebp-2Ch]
    float wheelSpeeds[4]; // [esp+244h] [ebp-28h]
    int i; // [esp+254h] [ebp-18h]
    NitrousVehicle *thisa; // [esp+258h] [ebp-14h]
    //_UNKNOWN *v10[2]; // [esp+260h] [ebp-Ch] BYREF
    //int vars0; // [esp+26Ch] [ebp+0h]
    //
    //v10[0] = a2;
    //v10[1] = (_UNKNOWN *)vars0;
    thisa = this;
    if (phys_drawNitrousVehicleEffects->current.enabled && thisa->m_phys_user_data)
    {
        for (i = 0; i < 4; ++i)
        {
            v2 = NitrousVehicle::debug_render_wheel_effects(i);
            wheelSpeeds[i] = v2;
        }
        p_m_mat = &thisa->m_phys_user_data->body->m_mat;
        pos = p_m_mat->w;
        Phys_NitrousVecToVec3(pos, textPos);
        textPos[2] = textPos[2] + 50.0;
        sprintf(buffer, "Wheel Speeds %.0f %.0f %.0f %.0f", wheelSpeeds[0], wheelSpeeds[1], wheelSpeeds[2], wheelSpeeds[3]);
        CL_AddDebugString(textPos, colorWhite, scale_2, buffer, 0);
    }
}

