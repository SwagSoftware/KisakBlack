#include "nitrous_vehicle_controller.h"
#include "nitrous_vehicle.h"
#include <bgame/bg_slidemove.h>
#include <client/cl_debugdata.h>
#include <game_mp/g_main_mp.h>
#include <bgame/bg_misc.h>
#include <cgame/cg_main.h>
#include <cgame/cg_drawtools.h>
#include <physics/phys_assert.h>
#include <physics/phys_main.h>
#include <cgame_mp/cg_local_mp.h>

float hand_brake_apply_delta = 30.0;
float steer_velocity = 400.0;
float speed_pref = 90.0;
float input_pref = 35.0;
float base_forward_pref = 100.0;
float throttle_min_scalar = 0.1;
float throttle_max_scalar = 1.0;
float stun_time_min = 0.25;
float stun_time_max = 0.5;
float min_brake = 1.0;
float max_brake = 1.0;
float brake_start_chance = 0.89999998;
float brake_soft_percent = 0.40000001;
float brake_soft_chance = 0.60000002;
float brake_fail_chance = 0.5;
float tweaker_0 = 11.0;
float breakHandbrakeThreshold = 0.69999999;
float required_stop_time = 0.2;
float steer_reduce_frac = 0.5;
float forward_accel_scale = -0.25;
float coast_steer_scaler = 10.0;
float analog_throttle_turn_factor = 2.5;


// local variable allocation has failed, the output may be wrong!
void NitrousVehicleController::UpdateScriptVehicleControl(
                NitrousVehicle *rbveh,
                float delta_t)
{
    float v4; // xmm0_4
    double max_speed; // st7
    float speed_at_cur_dist; // [esp+18h] [ebp-E0h]
    float frac_to_goal; // [esp+1Ch] [ebp-DCh]
    float current_speed; // [esp+2Ch] [ebp-CCh]
    float stuck_time; // [esp+34h] [ebp-C4h]
    phys_vec3 *p_m_stuck_position; // [esp+38h] [ebp-C0h]
    float angle_to_front; // [esp+60h] [ebp-98h]
    float angle_to_side; // [esp+68h] [ebp-90h]
    float end[3]; // [esp+7Ch] [ebp-7Ch] BYREF
    float start[3]; // [esp+88h] [ebp-70h] BYREF
    float goal_speed; // [esp+94h] [ebp-64h]
    float yaw; // [esp+98h] [ebp-60h]
    __int64 v17; // [esp+9Ch] [ebp-5Ch]
    float v18; // [esp+A4h] [ebp-54h]
    float to_goal_dist; // [esp+A8h] [ebp-50h]
    phys_vec3 to_goal; // [esp+ACh] [ebp-4Ch] BYREF
    float v21; // [esp+C0h] [ebp-38h]
    float v22; // [esp+C4h] [ebp-34h]
    float v23; // [esp+C8h] [ebp-30h]
    phys_vec3 currentOrigin; // [esp+CCh] [ebp-2Ch] BYREF
    phys_mat44 *p_m_mat; // [esp+E0h] [ebp-18h]
    PhysObjUserData *userData; // [esp+E4h] [ebp-14h]
    NitrousVehicleController *thisa; // [esp+E8h] [ebp-10h]
    //_UNKNOWN *v28[2]; // [esp+ECh] [ebp-Ch] BYREF
    //float delta_ta; // [esp+F4h] [ebp-4h] BYREF
    //int vars0; // [esp+F8h] [ebp+0h]
    //
    //v28[0] = a2;
    //v28[1] = (_UNKNOWN *)vars0;
    thisa = this;
    userData = rbveh->m_phys_user_data;
    p_m_mat = &userData->body->m_mat;
    currentOrigin = p_m_mat->w;
    v23 = this->m_script_goal_position.x - currentOrigin.x;
    v22 = this->m_script_goal_position.y - currentOrigin.y;
    v21 = this->m_script_goal_position.z - currentOrigin.z;
    to_goal.x = v23;
    to_goal.y = v22;
    to_goal.z = 0.0f;
    to_goal_dist = Vec3Length(&to_goal.x);
    if (to_goal_dist <= 0.0)
    {
        yaw = 0.0f;
    }
    else
    {
        v18 = 1.0 / to_goal_dist;
        to_goal.x = to_goal.x * (float)(1.0 / to_goal_dist);
        to_goal.y = to_goal.y * (float)(1.0 / to_goal_dist);
        to_goal.z = to_goal.z * (float)(1.0 / to_goal_dist);
        v17 = *(_QWORD *)&to_goal.x;
        //v4 = __libm_sse2_atan2(to_goal.y);
        v4 = atan2(to_goal.y, to_goal.x);
        yaw = RAD2DEG(v4);
        if (yaw < 0.0)
            yaw = yaw + 360.0;
    }
    goal_speed = thisa->m_script_goal_speed + 10.0;
    if ((rbveh->m_flags & 0x100) != 0)
        goal_speed = thisa->m_drive_path_speed;
    if (phys_drawNitrousVehicle->current.enabled)
    {
        Phys_NitrousVecToVec3(&currentOrigin, start);
        Phys_NitrousVecToVec3(&thisa->m_script_goal_position, end);
        CL_AddDebugLine(start, end, colorGreen, 0, 1);
    }
    (angle_to_side) = fabs((float)((float)(userData->body->m_mat.y.x * to_goal.x) + (float)(userData->body->m_mat.y.y * to_goal.y)) + (float)(userData->body->m_mat.y.z * to_goal.z));
    angle_to_front = (float)((float)(userData->body->m_mat.x.x * to_goal.x)
        + (float)(userData->body->m_mat.x.y * to_goal.y))
        + (float)(userData->body->m_mat.x.z * to_goal.z);
    if (rbveh->m_script_brake == 0.0
        && goal_speed > 0.0
        && ((float)((float)((float)((float)(currentOrigin.x - thisa->m_stuck_position.x)
            * (float)(currentOrigin.x - thisa->m_stuck_position.x))
            + (float)((float)(currentOrigin.y - thisa->m_stuck_position.y)
                * (float)(currentOrigin.y - thisa->m_stuck_position.y)))
            + (float)((float)(currentOrigin.z - thisa->m_stuck_position.z)
                * (float)(currentOrigin.z - thisa->m_stuck_position.z))) < 225.0
            || thisa->m_stuck_time < 0.0))
    {
        thisa->m_stuck_time = thisa->m_stuck_time + delta_t;
    }
    else
    {
        p_m_stuck_position = &thisa->m_stuck_position;
        thisa->m_stuck_position.x = currentOrigin.x;
        p_m_stuck_position->y = currentOrigin.y;
        p_m_stuck_position->z = currentOrigin.z;
        thisa->m_stuck_time = 0.0f;
    }
    stuck_time = 1.0f;
    if (rbveh->m_vehicle_info->type == 2)
        stuck_time = 4.0f;
    if (thisa->m_stuck_time > stuck_time)
        thisa->m_stuck_time = stuck_time * -0.75;
    (current_speed) = fabs(rbveh->m_forward_vel);
    if (rbveh->m_vehicle_info->type != 2 && angle_to_side > 0.80000001 && to_goal_dist < 400.0)
        goto LABEL_23;
    if (thisa->m_stuck_time >= 0.0 || angle_to_front >= -0.5)
    {
        if (thisa->m_stuck_time < 0.0)
        {
        LABEL_23:
            NitrousVehicleController::UpdateVehicleInputs(rbveh, yaw + 180.0, 1.0, -1.6);
            goto LABEL_36;
        }
        if (to_goal_dist > 600.0 || current_speed > 200.0 && angle_to_front < -0.80000001 || (rbveh->m_flags & 0x100) != 0)
            NitrousVehicleController::UpdateVehicleInputs(rbveh, yaw, 1.0, 4.0);
        else
            NitrousVehicleController::UpdateVehicleInputs(rbveh, yaw, 1.0, 0.0);
    }
    else if (to_goal_dist <= 600.0)
    {
        NitrousVehicleController::UpdateVehicleInputs(rbveh, yaw, 1.0, 1.0);
    }
    else
    {
        NitrousVehicleController::UpdateVehicleInputs(rbveh, yaw, 1.0, -1.6);
    }
LABEL_36:
    if ((rbveh->m_flags & 0x100) != 0)
    {
        if (current_speed > goal_speed)
        {
            rbveh->set_throttle(0.0f);
            //NitrousVehicle::set_throttle(rbveh, 0.0);
            rbveh->set_hand_brake(1.0f);
            //NitrousVehicle::set_hand_brake(rbveh, (VDCArgs *)&delta_ta, 1.0);
        }
    }
    else if ((float)(thisa->m_script_goal_radius - 10.0) <= to_goal_dist)
    {
        if (to_goal_dist >= 1000.0 || current_speed <= goal_speed)
        {
            if (current_speed > goal_speed)
            {
                //NitrousVehicle::set_throttle(rbveh, 0.0);
                rbveh->set_throttle(0.0f);
            }
        }
        else
        {
            frac_to_goal = to_goal_dist / 1000.0;
            //max_speed = NitrousVehicle::get_max_speed(rbveh, 1);
            max_speed = rbveh->get_max_speed(1);
            speed_at_cur_dist = (max_speed - goal_speed) * frac_to_goal + goal_speed;
            if (current_speed <= speed_at_cur_dist)
            {
                if (current_speed > goal_speed && current_speed > 300.0)
                {
                    //NitrousVehicle::set_throttle(rbveh, 0.0);
                    rbveh->set_throttle(0.0f);
                }
            }
            else
            {
                //NitrousVehicle::set_throttle(rbveh, 0.0);
                rbveh->set_throttle(0.0f);
                //NitrousVehicle::set_hand_brake(rbveh, (VDCArgs *)&delta_ta, 1.0);
                rbveh->set_hand_brake(1.0f);
            }
        }
    }
    else if (thisa->m_stop_at_goal)
    {
        //NitrousVehicle::set_throttle(rbveh, 0.0);
        rbveh->set_throttle(0.0f);
        //NitrousVehicle::set_hand_brake(rbveh, (VDCArgs *)&delta_ta, 1.0);
        rbveh->set_hand_brake(1.0f);
        if (current_speed <= 1.0)
            rbveh->m_flags &= ~4u;
    }
    else
    {
        rbveh->m_flags &= ~4u;
    }
}

void __thiscall NitrousVehicle::set_hand_brake(float braking)
{
    if ( (braking < 0.0 || braking > 1.0)
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle.h",
                    447,
                    0,
                    "%s\n\t%s",
                    "braking>= 0.0f && braking<= 1.0f",
                    "Out of range") )
    {
        __debugbreak();
    }
    this->m_hand_brake = braking;
}

void __thiscall NitrousVehicleController::UpdateControlsStrafeMode(
                NitrousVehicle *rbveh)
{
    float v2; // [esp+Ch] [ebp-78h]
    float v3; // [esp+10h] [ebp-74h]
    float v4; // [esp+14h] [ebp-70h]
    float v5; // [esp+18h] [ebp-6Ch]
    float rb_angles[3]; // [esp+48h] [ebp-3Ch] BYREF
    cg_s *cgameGlob; // [esp+54h] [ebp-30h]
    gentity_s *driver; // [esp+58h] [ebp-2Ch]
    int driver_occupant; // [esp+5Ch] [ebp-28h]
    float camera_yaw; // [esp+60h] [ebp-24h]
    float move[2]; // [esp+64h] [ebp-20h]
    float forwardPreference; // [esp+6Ch] [ebp-18h]
    float vehicleDeadZone; // [esp+70h] [ebp-14h]
    float input_yaw; // [esp+74h] [ebp-10h]
    float input; // [esp+78h] [ebp-Ch]
    float vec[2]; // [esp+7Ch] [ebp-8h] BYREF
    int savedregs; // [esp+84h] [ebp+0h] BYREF

    camera_yaw = 0.0f;
    if ( rbveh->m_owner )
    {
        if ( !rbveh->m_owner->scr_vehicle || !rbveh->m_owner->r.inuse )
            return;
        driver = 0;
        driver_occupant = VEH_GetSeatOccupantEntNum(rbveh->m_owner, 0);
        if ( driver_occupant != 1023 )
            driver = &g_entities[driver_occupant];
        if ( driver && driver->client )
            camera_yaw = driver->client->ps.viewangles[1];
    }
    else
    {
        cgameGlob = CG_GetLocalClientGlobals(this->m_cmd_local_client_num);
        camera_yaw = cgameGlob->predictedPlayerState.viewangles[1];
    }
    if ( player_topDownCamMode->current.integer )
        camera_yaw = player_topDownCamAngles->current.vector[1];
    move[0] = (float)this->m_cmd.rightmove;
    move[1] = (float)this->m_cmd.forwardmove * -1.0;
    if ( rbveh->m_vehicle_info->cameraMode == 2 && rbveh->m_vehicle_info->type != 2 )
        move[0] = 0.0f;
    vehicleDeadZone = 40.0f;
    if ( fabs(move[1]) >= 40.0 )
    {
        if ( move[1] < 0.0 )
            v5 = -1.0f;
        else
            v5 = 1.0f;
        vec[0] = (float)((float)(fabs(move[1]) - vehicleDeadZone)
                                     / (float)(128.0 - vehicleDeadZone))
                     * v5;
    }
    else
    {
        vec[0] = 0.0f;
    }
    if ( vehicleDeadZone <= fabs(move[0]) )
    {
        if ( move[0] < 0.0 )
            v4 = -1.0f;
        else
            v4 = 1.0f;
        vec[1] = (float)((float)(fabs(move[0]) - vehicleDeadZone)
                                     / (float)(128.0 - vehicleDeadZone))
                     * v4;
    }
    else
    {
        vec[1] = 0.0f;
    }
    input = Vec2Length(vec);
    if ( input <= 1.0 )
        v3 = input;
    else
        v3 = 1.0f;
    input = v3;
    input_yaw = vectoyaw(vec) + camera_yaw + 180.0;
    input_yaw = AngleNormalize180(input_yaw);
    if ( v3 == 0.0 )
    {
        if ( rbveh->m_vehicle_info->cameraMode == 2 )
        {
            input_yaw = AngleNormalize180(camera_yaw);
        }
        else
        {
            Axis4ToAngles((const float (*)[4])&rbveh->m_phys_user_data->body->m_mat, rb_angles);
            input_yaw = rb_angles[1];
        }
    }
    forwardPreference = 0.0f;
    if ( !player_topDownCamMode->current.integer )
    {
        if ( rbveh->m_vehicle_info->cameraMode == 2 )
        {
            if ( this->m_cmd.forwardmove < (char)(int)(-(vehicleDeadZone)) )
                v2 = -1.0f;
            else
                v2 = 1.0f;
            forwardPreference = v2;
        }
        else
        {
            forwardPreference = -vec[0];
        }
    }
    if ( rbveh->m_vehicle_info->type != 2 )
        forwardPreference = forwardPreference * 4.0;
    NitrousVehicleController::UpdateVehicleInputs(
        rbveh,
        input_yaw,
        input,
        forwardPreference);
}

// local variable allocation has failed, the output may be wrong!
void NitrousVehicleController::UpdateVehicleInputs(
                NitrousVehicle *rbveh,
                float target_yaw,
                float target_accel,
                float forward_preference)
{
    phys_vec3 v6; // [esp+20h] [ebp-F8h] BYREF
    int v7; // [esp+44h] [ebp-D4h]
    float hand_brake_min_speed; // [esp+48h] [ebp-D0h]
    bool apply_hand_brake; // [esp+4Fh] [ebp-C9h]
    float v10; // [esp+50h] [ebp-C8h]
    float v11; // [esp+54h] [ebp-C4h]
    float yaw_steer_range; // [esp+58h] [ebp-C0h]
    const VehicleParameter *v13; // [esp+5Ch] [ebp-BCh]
    float v14; // [esp+60h] [ebp-B8h]
    float steerFactor; // [esp+64h] [ebp-B4h]
    phys_vec3 *velocity; // [esp+68h] [ebp-B0h]
    phys_vec3 v17; // [esp+6Ch] [ebp-ACh] BYREF
    float v18; // [esp+84h] [ebp-94h]
    float v19; // [esp+88h] [ebp-90h]
    float m_steer_factor; // [esp+8Ch] [ebp-8Ch]
    float m_hand_brake; // [esp+90h] [ebp-88h]
    float fwd; // [esp+94h] [ebp-84h]
    float targetSpeed; // [esp+98h] [ebp-80h]
    float v24; // [esp+9Ch] [ebp-7Ch]
    float v25; // [esp+A0h] [ebp-78h]
    float anglePreference; // [esp+A4h] [ebp-74h]
    float m_forward_vel; // [esp+A8h] [ebp-70h]
    float total_input_pref; // [esp+ACh] [ebp-6Ch]
    float total_base_forward_pref; // [esp+B0h] [ebp-68h]
    bool reverse; // [esp+B7h] [ebp-61h]
    float v31; // [esp+B8h] [ebp-60h]
    const VehicleParameter *m_parameter; // [esp+BCh] [ebp-5Ch]
    float rb_angles[3]; // [esp+C0h] [ebp-58h] BYREF
    float _pt[4]; // [esp+CCh] [ebp-4Ch] BYREF
    phys_vec3 pt; // [esp+DCh] [ebp-3Ch] BYREF
    phys_mat44 *p_m_mat; // [esp+F0h] [ebp-28h]
    float input_fwd[3]; // [esp+F4h] [ebp-24h] BYREF
    const PhysObjUserData *userData; // [esp+100h] [ebp-18h]
    float delta_yaw; // [esp+104h] [ebp-14h]
    NitrousVehicleController *thisa; // [esp+108h] [ebp-10h]
    //_UNKNOWN *v41; // [esp+10Ch] [ebp-Ch]
    //NitrousVehicle *rbveha; // [esp+110h] [ebp-8h]
    //float target_yawa; // [esp+114h] [ebp-4h] BYREF
    //NitrousVehicle *target_accela; // [esp+118h] [ebp+0h]
    //
    //v41 = a2;
    //rbveha = target_accela;
    thisa = this;
    delta_yaw = 0.0f;
    if ((target_accel != 0.0 || rbveh->m_vehicle_info->type != 2) && (rbveh->m_flags & 1) == 0)
    {
        userData = rbveh->m_phys_user_data;
        if (phys_drawNitrousVehicle->current.enabled)
        {
            YawVectors(target_yaw, input_fwd, 0);
            p_m_mat = &userData->body->m_mat;
            pt = p_m_mat->w;
            _pt[3] = pt.z;
            pt.z = pt.z + 100.0;
            Phys_NitrousVecToVec3(&pt, _pt);
            input_fwd[0] = (float)(450.0 * input_fwd[0]) + _pt[0];
            input_fwd[1] = (float)(450.0 * input_fwd[1]) + _pt[1];
            input_fwd[2] = (float)(450.0 * input_fwd[2]) + _pt[2];
            CL_AddDebugLine(_pt, input_fwd, colorGreen, 0, 1);
        }
        Axis4ToAngles((const float (*)[4]) & userData->body->m_mat, rb_angles);
        delta_yaw = AngleDelta(target_yaw, rb_angles[1]);
    }
    if (rbveh->m_vehicle_info->type != 2)
    {
        m_parameter = rbveh->m_parameter;
        v31 = 3.0 * m_parameter->m_tire_damp_coast;
        rbveh->m_coasting_factor = v31 * rbveh->m_parameter->m_body_mass;
    }
    reverse = 0;
    total_base_forward_pref = base_forward_pref;
    total_input_pref = input_pref;
    if (rbveh->m_vehicle_info->type != 2)
    {
        total_input_pref = total_input_pref + 20.0;
        m_forward_vel = rbveh->m_forward_vel;
        if (m_forward_vel > speed_pref)
            total_base_forward_pref = total_base_forward_pref + 30.0;
    }
    anglePreference = (float)(total_input_pref * forward_preference) + total_base_forward_pref;
    if ((fabs(delta_yaw)) > anglePreference)
    {
        if (delta_yaw < 0.0)
            v25 = -1.0f;
        else
            v25 = 1.0f;
        delta_yaw = delta_yaw - (float)(180.0 * v25);
        reverse = 1;
    }
    if (reverse)
        v24 = -1.0f;
    else
        v24 = 1.0f;
    targetSpeed = v24 * target_accel;
    //NitrousVehicle::set_throttle(rbveh, v24 * target_accel);
    rbveh->set_throttle(v24 * target_accel);
    if (rbveh->m_vehicle_info->cameraMode == 2)
    {
        fwd = rbveh->m_forward_vel;
        if (target_accel != 0.0 && (fwd > 25.0 && reverse || fwd < -25.0 && !reverse))
            delta_yaw = delta_yaw * -1.0;
        m_hand_brake = rbveh->m_hand_brake;
        if (m_hand_brake > 0.1 && (fwd > 25.0 && reverse || fwd < -25.0 && !reverse))
        {
            m_steer_factor = rbveh->m_steer_factor;
            if (m_steer_factor < 0.5 || (v19 = rbveh->m_steer_factor, v19 > -0.5))
            {
                //NitrousVehicle::set_brake(rbveh, (VDCArgs *)&target_yawa, 1.0);
                rbveh->set_brake(1.0f);
                goto LABEL_46;
            }
        }
        goto LABEL_45;
    }
    v18 = rbveh->m_forward_vel;
    if (target_accel == 0.0 || (v18 <= 25.0 || !reverse) && (v18 >= -25.0 || reverse) || rbveh->m_vehicle_info->type == 4)
    {
    LABEL_45:
        rbveh->set_brake(0.0f);
        //NitrousVehicle::set_brake(rbveh, (VDCArgs *)&target_yawa, 0.0);
        goto LABEL_46;
    }
    //velocity = NitrousVehicle::get_velocity(rbveh, &v17);
    velocity = rbveh->get_velocity(&v17);
    if ((float)((float)((float)(velocity->x * velocity->x) + (float)(velocity->y * velocity->y)) + (float)(velocity->z * velocity->z)) > (float)(steer_velocity * steer_velocity))
        delta_yaw = 0.0f;
    delta_yaw = delta_yaw * -1.0;
    //NitrousVehicle::set_brake(rbveh, (VDCArgs *)&target_yawa, 1.0);
    rbveh->set_brake(1.0f);
LABEL_46:
    steerFactor = 0.0f;
    if (delta_yaw <= 0.0)
    {
        if (delta_yaw < 0.0)
            steerFactor = -1.0f;
    }
    else
    {
        steerFactor = 1.0f;
    }
    if (reverse)
        v14 = -1.0f;
    else
        v14 = 1.0f;
    steerFactor = steerFactor * v14;
    v13 = rbveh->m_parameter;
    yaw_steer_range = RAD2DEG(v13->m_steer_angle_max);
    if (reverse && rbveh->m_vehicle_info->type != 2)
        yaw_steer_range = yaw_steer_range * 2.0;
    if (yaw_steer_range > (fabs(delta_yaw)))
        steerFactor = (float)((fabs(delta_yaw)) / yaw_steer_range) * steerFactor;
    if ((float)(steerFactor - 1.0) < 0.0)
        v11 = steerFactor;
    else
        v11 = 1.0f;
    if ((float)(-1.0 - steerFactor) < 0.0)
        v10 = v11;
    else
        v10 = -1.0f;
    rbveh->m_steer_factor = v10;
    apply_hand_brake = 0;
    hand_brake_min_speed = rbveh->m_parameter->m_auto_hand_brake_min_speed;
    v7 = fabs(delta_yaw);

    phys_vec3 tmp_vel;
    rbveh->get_velocity(&tmp_vel);
    float velSq = Vec3LengthSq((float*)&tmp_vel);

    apply_hand_brake = (fabs(delta_yaw)) > hand_brake_apply_delta && velSq > (hand_brake_min_speed * hand_brake_min_speed);
        //&& (LODWORD(v6.z) = NitrousVehicle::get_velocity(rbveh, (phys_vec3 *)&v6.w),
        //    (float)((float)((float)(*(float *)LODWORD(v6.z) * *(float *)LODWORD(v6.z))
        //        + (float)(*(float *)(LODWORD(v6.z) + 4) * *(float *)(LODWORD(v6.z) + 4)))
        //        + (float)(*(float *)(LODWORD(v6.z) + 8) * *(float *)(LODWORD(v6.z) + 8))) > (float)(hand_brake_min_speed * hand_brake_min_speed));

    if (apply_hand_brake)
    {
        if (delta_yaw >= 0.0 || rbveh->m_phys_user_data->body->m_a_vel.z <= 0.0)
        {
            if (delta_yaw > 0.0 && rbveh->m_phys_user_data->body->m_a_vel.z < 0.0)
                apply_hand_brake = 0;
        }
        else
        {
            apply_hand_brake = 0;
        }
    }
    //if (bitarray<51>::testBit(&thisa->m_cmd.button_bits, 0x17u))
    if (thisa->m_cmd.button_bits.testBit(0x17u))
        apply_hand_brake = 1;
    if (apply_hand_brake)
    {
        //NitrousVehicle::set_hand_brake(rbveh, (VDCArgs *)&target_yawa, 1.0);
        rbveh->set_hand_brake(1.0f);
    }
    else
    {
        //NitrousVehicle::set_hand_brake(rbveh, (VDCArgs *)&target_yawa, 0.0);
        rbveh->set_hand_brake(0.0f);
    }
}

void __thiscall NitrousVehicle::set_brake(float braking)
{
    if ( (braking < 0.0 || braking > 1.0)
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle.h",
                    441,
                    0,
                    "%s\n\t%s",
                    "braking>= 0.0f && braking<= 1.0f",
                    "Out of range") )
    {
        __debugbreak();
    }
    this->m_brake = braking;
}

void __thiscall NitrousVehicleController::UpdateStun(
                NitrousVehicle *rbveh,
                float delta_t)
{
    float v4; // [esp+Ch] [ebp-14h]
    float desired_throttle; // [esp+18h] [ebp-8h]
    float desired_brake; // [esp+1Ch] [ebp-4h]

    if ( (rbveh->m_flags & 0x2000) != 0 )
    {
        desired_brake = rbveh->m_brake;
        desired_throttle = rbveh->m_throttle;
        if ( this->m_stun.m_stun_time > 0.0 )
        {
            this->m_stun.m_changed_this_frame = 0;
        }
        else
        {
            this->m_stun.m_stun_time = CG_flrand(stun_time_min, stun_time_max);
            this->m_stun.m_last_brake = StunRandomBrake(&this->m_cmd, desired_brake, this->m_stun.m_last_brake);
            if ( this->m_stun.m_last_brake <= 0.001 )
                v4 = (float)((float)((float)(throttle_max_scalar - throttle_min_scalar)
                                                     * (float)((float)(this->m_stun.m_stun_time - stun_time_min)
                                                                     / (float)(stun_time_max - stun_time_min)))
                                     + throttle_min_scalar)
                     * desired_throttle;
            else
                v4 = 0.0f;
            this->m_stun.m_last_throttle = v4;
            this->m_stun.m_changed_this_frame = 1;
        }
        //NitrousVehicle::set_brake(rbveh, this->m_stun.m_last_brake);
        rbveh->set_brake(this->m_stun.m_last_brake);
        //NitrousVehicle::set_throttle(rbveh, this->m_stun.m_last_throttle);
        rbveh->set_throttle(this->m_stun.m_last_throttle);
        this->m_stun.m_stun_time = this->m_stun.m_stun_time - delta_t;
    }
    else
    {
        this->m_stun.m_stun_time = 0.0f;
        this->m_stun.m_last_throttle = 0.0f;
        this->m_stun.m_last_brake = 0.0f;
        this->m_stun.m_changed_this_frame = 0;
    }
}

double __cdecl StunRandomBrake(const usercmd_s *cmd, float desired_brake, float last_brake)
{
    //if ( bitarray<51>::testBit(&cmd->button_bits, 0x16u) && desired_brake > 0.5 )
    if ( cmd->button_bits.testBit(0x16u) && desired_brake > 0.5 )
    {
        if ( brake_fail_chance > CG_flrand(0.0, 1.0) )
            return 0.0;
        if ( brake_soft_chance > CG_flrand(0.0, 1.0) )
            return CG_flrand(0.0, desired_brake * brake_soft_percent);
    }
    else
    {
        if ( last_brake > 0.001 )
            return 0.0;
        if ( brake_start_chance > CG_flrand(0.0, 1.0) )
            return CG_flrand(min_brake, max_brake);
    }
    return CG_flrand(0.0, desired_brake);
}

void NitrousVehicleController::UpdateControls(
                NitrousVehicle *rbveh,
                float delta_t)
{
    float v4; // [esp+24h] [ebp-7Ch]
    float m_forward_vel; // [esp+2Ch] [ebp-74h]
    float v6; // [esp+38h] [ebp-68h]
    float v7; // [esp+3Ch] [ebp-64h]
    scr_vehicle_s *scr_vehicle; // [esp+6Ch] [ebp-34h]
    float v9; // [esp+80h] [ebp-20h]
    float v10; // [esp+84h] [ebp-1Ch]
    float v11; // [esp+88h] [ebp-18h]
    //float steering[2]; // [esp+94h] [ebp-Ch] BYREF
    //float retaddr; // [esp+A0h] [ebp+0h]
    //
    //steering[0] = a2;
    //steering[1] = retaddr;
    if ( (rbveh->m_flags & 4) != 0 )
    {
        NitrousVehicleController::UpdateScriptVehicleControl(rbveh, delta_t);
        if ( rbveh->m_vehicle_info->type == 4 )
            NitrousVehicleController::UpdateApplyBoatAccel(rbveh, delta_t);
        return;
    }
    if ( !this->m_cmd_valid )
        return;
    if ( (rbveh->m_flags & 0x80) == 0 )
    {
        if ( (rbveh->m_flags & 0x100) != 0 && (rbveh->m_flags & 1) == 0 )
        {
            if ( rbveh->m_owner && rbveh->m_owner->r.inuse && rbveh->m_owner->scr_vehicle )
            {
                scr_vehicle = rbveh->m_owner->scr_vehicle;
                //if ( ((LODWORD(scr_vehicle->pathPos.origin[0]) & 0x7F800000) == 0x7F800000
                //     || (LODWORD(scr_vehicle->pathPos.origin[1]) & 0x7F800000) == 0x7F800000
                //     || (LODWORD(scr_vehicle->pathPos.origin[2]) & 0x7F800000) == 0x7F800000)
                //    && !Assert_MyHandler(
                //                "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle_controller.cpp",
                //                515,
                //                0,
                //                "%s",
                //                "!IS_NAN((vpp.origin)[0]) && !IS_NAN((vpp.origin)[1]) && !IS_NAN((vpp.origin)[2])") )
                //{
                //    __debugbreak();
                //}
                //if ( ((LODWORD(scr_vehicle->pathPos.lookPos[0]) & 0x7F800000) == 0x7F800000
                //     || (LODWORD(scr_vehicle->pathPos.lookPos[1]) & 0x7F800000) == 0x7F800000
                //     || (LODWORD(scr_vehicle->pathPos.lookPos[2]) & 0x7F800000) == 0x7F800000)
                //    && !Assert_MyHandler(
                //                "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle_controller.cpp",
                //                516,
                //                0,
                //                "%s",
                //                "!IS_NAN((vpp.lookPos)[0]) && !IS_NAN((vpp.lookPos)[1]) && !IS_NAN((vpp.lookPos)[2])") )
                //{
                //    __debugbreak();
                //}
                v6 = scr_vehicle->pathPos.lookPos[1];
                v7 = scr_vehicle->pathPos.lookPos[2];
                this->m_script_goal_position.x = scr_vehicle->pathPos.lookPos[0];
                this->m_script_goal_position.y = v6;
                this->m_script_goal_position.z = v7;
                this->m_script_goal_radius = 5.0f;
                if ( rbveh->m_owner->scr_vehicle->manualMode == 1 )
                    this->m_drive_path_speed = this->m_script_goal_speed;
                else
                    this->m_drive_path_speed = scr_vehicle->pathPos.speed;
                if ( phys_drawNitrousVehicle->current.enabled )
                {
                    CG_DebugSphere(scr_vehicle->pathPos.origin, 15.0, colorYellow, 10, 0, 1);
                    CG_DebugSphere(scr_vehicle->pathPos.lookPos, 15.0, colorGreen, 10, 0, 1);
                }
            }
            NitrousVehicleController::UpdateScriptVehicleControl(rbveh, delta_t);
            goto LABEL_60;
        }
        if ( rbveh->m_vehicle_info->type == 2 )
        {
            if ( rbveh->m_vehicle_info->cameraMode == 5 )
                NitrousVehicleController::UpdateControlsTank(rbveh);
            else
                NitrousVehicleController::UpdateControlsStrafeMode(rbveh);
            m_forward_vel = rbveh->m_forward_vel;
            //if ( m_forward_vel <= NitrousVehicle::get_max_speed(rbveh, 1) || rbveh->m_throttle <= 0.0 )
            if ( m_forward_vel <= rbveh->get_max_speed(1) || rbveh->m_throttle <= 0.0 )
            {
                v4 = rbveh->m_forward_vel;
                //if (-NitrousVehicle::get_max_speed(rbveh, 1) > v4 && rbveh->m_throttle < 0.0)
                if (-rbveh->get_max_speed(1) > v4 &&rbveh->m_throttle < 0.0)
                {
                    rbveh->set_throttle(0.0f);
                    //NitrousVehicle::set_throttle(rbveh, 0.0);
                }
            }
            else
            {
                //NitrousVehicle::set_throttle(rbveh, 0.0);
                rbveh->set_throttle(0.0f);
            }
            goto LABEL_59;
        }
        if ( rbveh->m_vehicle_info->cameraMode == 2 || rbveh->m_vehicle_info->cameraMode == 3 )
            goto LABEL_56;
        if ( rbveh->m_vehicle_info->cameraMode == 5 )
            goto LABEL_52;
        if ( rbveh->m_vehicle_info->cameraMode != 4 )
        {
            NitrousVehicleController::UpdateControlsDefault(rbveh, delta_t);
            goto LABEL_59;
        }
        if ( vehControlMode && vehControlMode->current.integer )
LABEL_56:
            NitrousVehicleController::UpdateControlsStrafeMode(rbveh);
        else
LABEL_52:
            NitrousVehicleController::UpdateControlsTank(rbveh);
LABEL_59:
        NitrousVehicleController::UpdateJump(rbveh, delta_t);
        goto LABEL_60;
    }
    if ( (rbveh->m_flags & 1) != 0 )
    {
        //NitrousVehicle::start_path(rbveh, 0);
        rbveh->start_path(0);
    }
    else
    {
        v11 = (float)(DEG2RAD(rbveh->m_owner->s.lerp.u.turret.gunAngles[0])) * tweaker_0;
        if ( (float)(v11 - 1.0) < 0.0 )
            v10 = (float)(DEG2RAD(rbveh->m_owner->s.lerp.u.turret.gunAngles[0])) * tweaker_0;
        else
            v10 = 1.0f;
        if ( (float)(-1.0 - v11) < 0.0 )
            v9 = v10;
        else
            v9 = -1.0f;
        rbveh->m_steer_factor = v9;
        //NitrousVehicle::set_throttle(rbveh, 0.0);
        rbveh->set_throttle(0.0f);
        //NitrousVehicle::set_hand_brake(rbveh, 0.0);
        rbveh->set_hand_brake(0.0f);
    }
LABEL_60:
    if ( rbveh->m_vehicle_info->type == 4 )
        NitrousVehicleController::UpdateApplyBoatAccel(rbveh, delta_t);
    NitrousVehicleController::UpdateStun(rbveh, delta_t);
}

double __cdecl RecalibrateInputCustom(float val, float threshold)
{
    float v3; // [esp+0h] [ebp-8h]

    if ( threshold <= fabs(val) )
    {
        if ( val <= 0.0 )
            v3 = -1.0f;
        else
            v3 = 1.0f;
        return (float)((float)((float)((float)(fabs(val) - threshold)
                                                                 / (float)(128.0 - threshold))
                                                 * 128.0)
                                 * v3);
    }
    else
    {
        return 0.0f;
    }
}

// local variable allocation has failed, the output may be wrong!
void NitrousVehicleController::UpdateControlsDefault(
                NitrousVehicle *rbveh,
                float delta_t)
{
    bool v4; // [esp+1Ch] [ebp-74h]
    phys_vec3 *velocity; // [esp+30h] [ebp-60h]
    int v6; // [esp+34h] [ebp-5Ch] BYREF
    float *v7; // [esp+4Ch] [ebp-44h]
    int p_m_mat; // [esp+50h] [ebp-40h]
    PhysObjUserData *m_phys_user_data; // [esp+54h] [ebp-3Ch]
    bool v10; // [esp+5Bh] [ebp-35h]
    float AnalogButtonValue; // [esp+5Ch] [ebp-34h]
    float v12; // [esp+60h] [ebp-30h]
    float v13; // [esp+64h] [ebp-2Ch] OVERLAPPED
    float gas; // [esp+68h] [ebp-28h]
    float fraction; // [esp+6Ch] [ebp-24h]
    float v16; // [esp+70h] [ebp-20h]
    float m_tire_damp_coast; // [esp+74h] [ebp-1Ch]
    const VehicleParameter *m_parameter; // [esp+78h] [ebp-18h]
    const vehicle_info_t *steering; // [esp+7Ch] [ebp-14h]
    NitrousVehicleController *v20; // [esp+80h] [ebp-10h]
    //int v21; // [esp+84h] [ebp-Ch]
    //const vehicle_info_t *info; // [esp+88h] [ebp-8h]
    //const vehicle_info_t *retaddr; // [esp+90h] [ebp+0h]
    //
    //v21 = a2;
    //info = retaddr;
    v20 = this;
    steering = rbveh->m_vehicle_info;
    m_parameter = rbveh->m_parameter;
    m_tire_damp_coast = m_parameter->m_tire_damp_coast;
    rbveh->m_coasting_factor = m_tire_damp_coast * rbveh->m_parameter->m_body_mass;
    v16 = (float)-v20->m_cmd.rightmove / 128.0;
    if ( steering->steerGraph )
    {
        fraction = fabs(v16);
        if ( v16 < 0.0 )
            gas = -1.0f;
        else
            gas = 1.0f;
        v16 = GraphFloat_GetValue(steering->steerGraph, fraction) * gas;
    }
    if ( (float)(v16 - 1.0) < 0.0 )
        v13 = v16;
    else
        v13 = 1.0f;
    if ( (float)(-1.0 - v16) < 0.0 )
        v12 = v13;
    else
        v12 = -1.0f;
    rbveh->m_steer_factor = v12;
    if ( rbveh->m_vehicle_info->cameraMode == 4 )
        AnalogButtonValue = (float)v20->m_cmd.forwardmove / 127.0;
    else
        AnalogButtonValue = CMD_GetAnalogButtonValue(&v20->m_cmd, 0x13u);
    v10 = rbveh->m_vehicle_info->type != 4;
    rbveh->set_hand_brake(0.0f);
    rbveh->set_throttle(0.0f);
    rbveh->set_brake(0.0f);
    //NitrousVehicle::set_hand_brake(rbveh, 0.0);
    //NitrousVehicle::set_throttle(rbveh, 0.0);
    //NitrousVehicle::set_brake(rbveh, 0.0);
    if ( AnalogButtonValue > 0.0 )
    {
        //NitrousVehicle::set_throttle(rbveh, AnalogButtonValue);
        rbveh->set_throttle(AnalogButtonValue);
        v20->m_stopped_time = 0.0f;
    }
    //if ( !bitarray<51>::testBit(&v20->m_cmd.button_bits, 0x16u) || rbveh->m_vehicle_info->cameraMode == 4 )
    if ( !v20->m_cmd.button_bits.testBit(0x16u) || rbveh->m_vehicle_info->cameraMode == 4 )
    {
        if (AnalogButtonValue < 0.0 && rbveh->m_vehicle_info->cameraMode == 4)
        {
            //NitrousVehicle::set_throttle(rbveh, AnalogButtonValue);
            rbveh->set_throttle(AnalogButtonValue);
        }
    }
    else
    {
        if ( v10 )
        {
            m_phys_user_data = rbveh->m_phys_user_data;
            p_m_mat = (int)&m_phys_user_data->body->m_mat;
            v7 = (float *)(p_m_mat + 16);
            //velocity = NitrousVehicle::get_velocity(rbveh, (phys_vec3 *)&v6);
            phys_vec3 tmp;
            //velocity = rbveh->get_velocity((phys_vec3 *)&v6);
            velocity = rbveh->get_velocity(&tmp);
            v4 = rbveh->m_forward_vel <= 1.0
                && (float)(fabs((float)((float)(velocity->x * *v7) + (float)(velocity->y * v7[1])) + (float)(velocity->z * v7[2]))
                                 - 2.0) > fabs(rbveh->m_forward_vel)
                && required_stop_time > v20->m_stopped_time;
            if ( rbveh->m_forward_vel > 1.0 || v4 )
            {
                v20->m_stopped_time = 0.0f;
                if ( v4 )
                    v20->m_stopped_time = -0.2f;
                if ( fabs(rbveh->m_steer_factor) > breakHandbrakeThreshold
                    || AnalogButtonValue > 0.0 )
                {
LABEL_36:
                    //NitrousVehicle::set_hand_brake(rbveh, 1.0);
                    rbveh->set_hand_brake(1.0f);
                    goto LABEL_42;
                }
            }
            else
            {
                v20->m_stopped_time = v20->m_stopped_time + delta_t;
                if ( v20->m_stopped_time > required_stop_time )
                {
                    //NitrousVehicle::set_throttle(rbveh, -1.0);
                    rbveh->set_throttle(-1.0f);
                    goto LABEL_42;
                }
                if ( AnalogButtonValue > 0.0 )
                    goto LABEL_36;
            }
            //NitrousVehicle::set_brake(rbveh, 1.0);
            rbveh->set_brake(1.0f);
            goto LABEL_42;
        }
        //NitrousVehicle::set_throttle(rbveh, -1.0);
        rbveh->set_throttle(-1.0f);
    }
LABEL_42:
    if ( !v10 )
    {
        //if ( bitarray<51>::testBit(&v20->m_cmd.button_bits, 0x17u) )
        if (v20->m_cmd.button_bits.testBit(0x17u))
        {
            //NitrousVehicle::set_hand_brake(rbveh, 1.0);
            rbveh->set_hand_brake(1.0f);
        }
    }
}

void __thiscall NitrousVehicleController::UpdateControlsTank(NitrousVehicle *rbveh)
{
    float v2; // [esp+8h] [ebp-50h]
    float v3; // [esp+Ch] [ebp-4Ch]
    float v4; // [esp+10h] [ebp-48h]
    float v5; // [esp+14h] [ebp-44h]
    float v6; // [esp+1Ch] [ebp-3Ch]
    float v7; // [esp+20h] [ebp-38h]
    float steering; // [esp+34h] [ebp-24h]
    float normalized_x; // [esp+3Ch] [ebp-1Ch]
    float input_dist; // [esp+40h] [ebp-18h]
    float xa; // [esp+4Ch] [ebp-Ch]
    float x; // [esp+4Ch] [ebp-Ch]
    float y; // [esp+50h] [ebp-8h]
    float throttle; // [esp+54h] [ebp-4h]
    float throttlea; // [esp+54h] [ebp-4h]

    rbveh->m_coasting_factor = rbveh->m_parameter->m_tire_damp_coast * rbveh->m_parameter->m_body_mass;
    xa = (float)this->m_cmd.rightmove;
    y = RecalibrateInputCustom((float)this->m_cmd.forwardmove * -1.0, 40.0);
    x = RecalibrateInputCustom(xa, 40.0);
    normalized_x = 0.0f;
    input_dist = sqrtf((float)(x * x) + (float)(y * y));
    if ( input_dist != 0.0 )
        normalized_x = x / input_dist;
    if ( normalized_x < 0.0 )
        v5 = -1.0f;
    else
        v5 = 1.0f;
    steering = (-(normalized_x * normalized_x)) * v5;
    if ( fabs(steering) < 0.98000002 )
        steering = steering * steer_reduce_frac;
    if ( rbveh->m_vehicle_info->cameraMode == 4 || rbveh->m_vehicle_info->type != 2 )
        v4 = 1.0f;
    else
        v4 = 0.25f;
    throttle = (float)((float)((float)((float)(normalized_x * x) / 129.0) * v4)
                                     * (float)((float)((float)(normalized_x * x) / 129.0) * v4))
                     + (float)((float)(y / 129.0) * (float)(y / 129.0));
    if ( throttle == 0.0 )
        throttlea = 0.0f;
    else
        throttlea = throttle / sqrtf(throttle);
    if ( y > 0.0 )
        throttlea = throttlea * -1.0;
    if ( rbveh->m_forward_vel >= 0.0 || throttlea <= 0.0 )
    {
        if (rbveh->m_forward_vel > 0.0 && throttlea < 0.0)
        {
            //LODWORD(steering) ^= _mask__NegFloat_;
            (steering) = -steering;
        }
    }
    else
    {
        //LODWORD(steering) ^= _mask__NegFloat_;
        steering = -steering;
    }
    if ( (float)(steering - 1.0) < 0.0 )
        v7 = steering;
    else
        v7 = 1.0f;
    if ( (float)(-1.0 - steering) < 0.0 )
        v3 = v7;
    else
        v3 = -1.0f;
    rbveh->m_steer_factor = v3;
    if ( (float)(throttlea - 1.0) < 0.0 )
        v6 = throttlea;
    else
        v6 = 1.0f;
    if ( (float)(-1.0 - throttlea) < 0.0 )
        v2 = v6;
    else
        v2 = -1.0f;

    rbveh->set_throttle(v2);
    rbveh->set_hand_brake(0.0f);
    rbveh->set_brake(0.0f);
    //NitrousVehicle::set_throttle(rbveh, v2);
    //NitrousVehicle::set_hand_brake(rbveh, 0.0);
    //NitrousVehicle::set_brake(rbveh, 0.0);
}

void __thiscall NitrousVehicleController::SetScriptTarget(
                NitrousVehicle *rbveh,
                phys_vec3 *goal_position,
                float goal_radius,
                float goal_speed,
                bool stopAtGoal)
{
    float v6; // [esp+4h] [ebp-Ch]
    float v7; // [esp+Ch] [ebp-4h]

    this->m_script_goal_position.x = goal_position->x;
    this->m_script_goal_position.y = goal_position->y;
    this->m_script_goal_position.z = goal_position->z;
    if ( (float)(goal_radius - 5000.0) < 0.0 )
        v7 = goal_radius;
    else
        v7 = 5000.0f;
    if ( (float)(5.0 - goal_radius) < 0.0 )
        v6 = v7;
    else
        v6 = 5.0f;
    this->m_script_goal_radius = v6;
    this->m_script_goal_speed = goal_speed;
    this->m_stop_at_goal = stopAtGoal;
    rbveh->m_flags |= 4u;
}

void NitrousVehicleController::UpdateJump(
                NitrousVehicle *rbveh,
                float delta_t)
{
    phys_vec3 v4; // [esp-Ch] [ebp-3Ch] BYREF
    float m_jump_force; // [esp+10h] [ebp-20h]
    const VehicleParameter *vehParams; // [esp+14h] [ebp-1Ch]
    rigid_body *body; // [esp+18h] [ebp-18h]
    int m_num_colliding_wheels; // [esp+1Ch] [ebp-14h]
    NitrousVehicleController *thisa; // [esp+20h] [ebp-10h]
    //_UNKNOWN *v10; // [esp+24h] [ebp-Ch]
    //NitrousVehicle *rbveha; // [esp+28h] [ebp-8h]
    //int vars0; // [esp+30h] [ebp+0h]
    //
    //v10 = a2;
    //rbveha = (NitrousVehicle *)vars0;
    thisa = this;
    if (this->m_cmd.button_bits.testBit(0x24u))
    {
        m_num_colliding_wheels = rbveh->m_num_colliding_wheels;
        if (m_num_colliding_wheels >= 2 && !thisa->m_jump_button_pressed)
        {
            body = rbveh->m_phys_user_data->body;
            vehParams = rbveh->m_parameter;
            m_jump_force = vehParams->m_jump_force;
            v4.x = 0.0f;
            v4.y = 0.0f;
            v4.z = m_jump_force;
            //rigid_body::add_force(body, &v4);
            body->add_force(&v4);
            thisa->m_jump_button_pressed = 1;
        }
    }
    else
    {
        thisa->m_jump_button_pressed = 0;
    }
}

//void __thiscall rigid_body::add_force(const phys_vec3 *force)
//{
//    this->m_force_sum.x = this->m_force_sum.x + force->x;
//    this->m_force_sum.y = this->m_force_sum.y + force->y;
//    this->m_force_sum.z = this->m_force_sum.z + force->z;
//    if ( fabs(this->m_force_sum.x) > 100000.0
//        || fabs(this->m_force_sum.y) > 100000.0
//        || fabs(this->m_force_sum.z) > 100000.0 )
//    {
//        phys_exec_debug_callback(this);
//    }
//}

// local variable allocation has failed, the output may be wrong!
void NitrousVehicleController::UpdateApplyBoatAccel(
                NitrousVehicle *rbveh,
                float delta_t)
{
    double v4; // st7
    float v5; // [esp+10h] [ebp-364h]
    float v6; // [esp+14h] [ebp-360h]
    phys_vec3 y; // [esp+18h] [ebp-35Ch] BYREF
    float v8; // [esp+28h] [ebp-34Ch]
    float v9; // [esp+2Ch] [ebp-348h]
    bool apply_ebrake; // [esp+33h] [ebp-341h]
    BOOL v11; // [esp+34h] [ebp-340h]
    float v12; // [esp+38h] [ebp-33Ch]
    float v13; // [esp+3Ch] [ebp-338h]
    float v14; // [esp+40h] [ebp-334h]
    float v15; // [esp+44h] [ebp-330h]
    float turn_throttle; // [esp+48h] [ebp-32Ch]
    phys_vec3 end; // [esp+4Ch] [ebp-328h] BYREF
    float v18; // [esp+5Ch] [ebp-318h]
    float v19; // [esp+60h] [ebp-314h]
    float v20; // [esp+6Ch] [ebp-308h]
    float v21; // [esp+70h] [ebp-304h]
    float v22; // [esp+74h] [ebp-300h]
    float v23; // [esp+78h] [ebp-2FCh]
    float v24; // [esp+7Ch] [ebp-2F8h]
    float v25; // [esp+80h] [ebp-2F4h]
    float v26; // [esp+8Ch] [ebp-2E8h]
    float v27; // [esp+90h] [ebp-2E4h]
    float v28; // [esp+94h] [ebp-2E0h]
    float start[3]; // [esp+98h] [ebp-2DCh] BYREF
    const phys_vec3 *v30; // [esp+A4h] [ebp-2D0h]
    phys_vec3 v31; // [esp+A8h] [ebp-2CCh] BYREF
    phys_mat44 rv; // [esp+B8h] [ebp-2BCh] BYREF
    float rot; // [esp+100h] [ebp-274h]
    float v34; // [esp+104h] [ebp-270h]
    phys_vec3 accel; // [esp+108h] [ebp-26Ch] BYREF
    phys_vec3 motor_position; // [esp+118h] [ebp-25Ch] BYREF
    float v37; // [esp+12Ch] [ebp-248h]
    float v38; // [esp+130h] [ebp-244h]
    float v39; // [esp+134h] [ebp-240h]
    float v40; // [esp+138h] [ebp-23Ch]
    float v41; // [esp+13Ch] [ebp-238h]
    float v42; // [esp+140h] [ebp-234h]
    float v43; // [esp+14Ch] [ebp-228h]
    float v44; // [esp+150h] [ebp-224h]
    float v45; // [esp+154h] [ebp-220h]
    float v46; // [esp+158h] [ebp-21Ch]
    float v47; // [esp+15Ch] [ebp-218h]
    float v48; // [esp+160h] [ebp-214h]
    float v49; // [esp+174h] [ebp-200h]
    float v50; // [esp+178h] [ebp-1FCh]
    float v51; // [esp+17Ch] [ebp-1F8h]
    phys_mat44 *p_m_mat; // [esp+180h] [ebp-1F4h]
    float v53; // [esp+184h] [ebp-1F0h]
    float x; // [esp+188h] [ebp-1ECh]
    float v55; // [esp+18Ch] [ebp-1E8h]
    float v56; // [esp+190h] [ebp-1E4h]
    float v57; // [esp+1A4h] [ebp-1D0h]
    float v58; // [esp+1A8h] [ebp-1CCh]
    phys_vec3 v59; // [esp+1ACh] [ebp-1C8h] BYREF
    float v60; // [esp+1BCh] [ebp-1B8h]
    float v61; // [esp+1C0h] [ebp-1B4h]
    phys_vec3 v62; // [esp+1CCh] [ebp-1A8h] BYREF
    float v63; // [esp+1DCh] [ebp-198h]
    float v64; // [esp+1E0h] [ebp-194h]
    float v65; // [esp+1ECh] [ebp-188h]
    float v66; // [esp+1F0h] [ebp-184h]
    float v67; // [esp+1F4h] [ebp-180h]
    phys_vec3 centerOffset; // [esp+1F8h] [ebp-17Ch]
    float v69; // [esp+214h] [ebp-160h]
    float v70; // [esp+218h] [ebp-15Ch]
    float v71; // [esp+21Ch] [ebp-158h]
    phys_vec3 *p_y; // [esp+220h] [ebp-154h]
    float v73; // [esp+224h] [ebp-150h]
    phys_vec3 leftForce; // [esp+228h] [ebp-14Ch] BYREF
    float v75; // [esp+23Ch] [ebp-138h]
    float v76; // [esp+240h] [ebp-134h]
    float v77; // [esp+244h] [ebp-130h]
    float v78; // [esp+248h] [ebp-12Ch]
    float v79; // [esp+24Ch] [ebp-128h]
    float v80; // [esp+250h] [ebp-124h]
    float v81; // [esp+25Ch] [ebp-118h]
    float v82; // [esp+260h] [ebp-114h]
    float v83; // [esp+264h] [ebp-110h]
    float v84; // [esp+268h] [ebp-10Ch]
    float v85; // [esp+26Ch] [ebp-108h]
    float v86; // [esp+270h] [ebp-104h]
    float v87; // [esp+27Ch] [ebp-F8h]
    float v88; // [esp+280h] [ebp-F4h]
    float v89; // [esp+284h] [ebp-F0h]
    float v90; // [esp+288h] [ebp-ECh]
    float v91; // [esp+28Ch] [ebp-E8h]
    float v92; // [esp+290h] [ebp-E4h]
    float v93; // [esp+2A4h] [ebp-D0h]
    float v94; // [esp+2A8h] [ebp-CCh]
    float v95; // [esp+2ACh] [ebp-C8h]
    float m_steer_factor; // [esp+2B0h] [ebp-C4h]
    float m_throttle; // [esp+2B4h] [ebp-C0h]
    phys_vec3 rightForce; // [esp+2B8h] [ebp-BCh] BYREF
    float v99; // [esp+2C8h] [ebp-ACh]
    float v100; // [esp+2CCh] [ebp-A8h]
    float v101; // [esp+2D0h] [ebp-A4h]
    float v102; // [esp+2DCh] [ebp-98h]
    float v103; // [esp+2E0h] [ebp-94h]
    float v104; // [esp+2E4h] [ebp-90h]
    float v105; // [esp+2E8h] [ebp-8Ch]
    float v106; // [esp+2ECh] [ebp-88h]
    float v107; // [esp+2F0h] [ebp-84h]
    float v108; // [esp+2F8h] [ebp-7Ch]
    float v109; // [esp+2FCh] [ebp-78h]
    float v110; // [esp+300h] [ebp-74h]
    float v111; // [esp+304h] [ebp-70h]
    float throttle; // [esp+308h] [ebp-6Ch]
    int v113; // [esp+30Ch] [ebp-68h]
    float rotSpeedFactor; // [esp+310h] [ebp-64h]
    float speedFactor; // [esp+314h] [ebp-60h]
    float turnProportion; // [esp+318h] [ebp-5Ch]
    float forwardProportion; // [esp+31Ch] [ebp-58h]
    int v118; // [esp+320h] [ebp-54h]
    float m_forward_vel; // [esp+324h] [ebp-50h]
    phys_vec3 forwardForce; // [esp+328h] [ebp-4Ch] BYREF
    float pos[3]; // [esp+33Ch] [ebp-38h] BYREF
    phys_vec3 center; // [esp+348h] [ebp-2Ch] BYREF
    const vehicle_info_t *info; // [esp+358h] [ebp-1Ch]
    const VehicleParameter *vehParams; // [esp+35Ch] [ebp-18h]
    rigid_body *body; // [esp+360h] [ebp-14h]
    NitrousVehicleController *thisa; // [esp+364h] [ebp-10h]
    //_UNKNOWN *v127[2]; // [esp+368h] [ebp-Ch] BYREF
    //int vars0; // [esp+374h] [ebp+0h]
    //
    //v127[0] = a2;
    //v127[1] = (_UNKNOWN *)vars0;
    thisa = this;
    if ((rbveh->m_flags & 0x400) != 0)
    {
        body = rbveh->m_phys_user_data->body;
        vehParams = rbveh->m_parameter;
        info = rbveh->m_vehicle_info;
        center = body->m_mat.w;
        Phys_NitrousVecToVec3(&center, pos);
        forwardForce = body->m_mat.x;
        m_forward_vel = rbveh->m_forward_vel;
        v118 = fabs(m_forward_vel);
        forwardProportion = (fabs(m_forward_vel))
            / (float)(vehParams->m_water_speed_max * rbveh->m_speed_scale);
        if (forwardProportion > 1.0)
            forwardProportion = 1.0f;
        v4 = Vec3Length(&body->m_a_vel.x);
        turnProportion = v4 / vehParams->m_water_turn_speed_max;
        if (turnProportion > 1.0)
            turnProportion = 1.0f;
        speedFactor = 1.0 - forwardProportion;
        rotSpeedFactor = (float)(1.0 - forwardProportion) * 0.5;
        if (info->accelGraph)
        {
            v113 = fabs(forwardProportion);
            speedFactor = GraphFloat_GetValue(info->accelGraph, (fabs(forwardProportion)));
        }
        speedFactor = (float)((float)((float)(delta_t * 60.0) * vehParams->m_water_accel_max) * rbveh->m_speed_scale)
            * speedFactor;
        rotSpeedFactor = (float)((float)((float)(delta_t * 60.0) * vehParams->m_water_turn_accel) * rbveh->m_drivepath_scale)
            * rotSpeedFactor;
        throttle = rbveh->m_throttle;
        if (throttle < 0.0)
        {
            v111 = rbveh->m_forward_vel;
            if (v111 < 1.0)
                speedFactor = speedFactor * vehParams->m_reverse_scale;
        }

        static bool dothisoldstuff = false;
        if (dothisoldstuff)
        {
            v110 = throttle * forwardForce.x;
            v109 = throttle * forwardForce.y;
            v108 = throttle * forwardForce.z;
            v105 = throttle * forwardForce.x;
            v106 = throttle * forwardForce.y;
            v107 = throttle * forwardForce.z;
            v104 = speedFactor * (float)(throttle * forwardForce.x);
            v103 = speedFactor * (float)(throttle * forwardForce.y);
            v102 = speedFactor * (float)(throttle * forwardForce.z);
            v99 = v104;
            v100 = v103;
            v101 = v102;
            forwardForce.x = v104;
            forwardForce.y = v103;
            forwardForce.z = v102;
            //rigid_body::add_force(body, &forwardForce);
            body->add_force(&forwardForce);
            rightForce = body->m_mat.x;
            m_throttle = rbveh->m_throttle;
            m_steer_factor = rbveh->m_steer_factor;
            v95 = m_steer_factor * rightForce.x;
            v94 = m_steer_factor * rightForce.y;
            v93 = m_steer_factor * rightForce.z;
            v90 = m_steer_factor * rightForce.x;
            v91 = m_steer_factor * rightForce.y;
            v92 = m_steer_factor * rightForce.z;
            v89 = rotSpeedFactor * (float)(m_steer_factor * rightForce.x);
            v88 = rotSpeedFactor * (float)(m_steer_factor * rightForce.y);
            v87 = rotSpeedFactor * (float)(m_steer_factor * rightForce.z);
            v84 = v89;
            v85 = v88;
            v86 = v87;
            v83 = m_throttle * v89;
            v82 = m_throttle * v88;
            v81 = m_throttle * v87;
            v78 = m_throttle * v89;
            v79 = m_throttle * v88;
            v80 = m_throttle * v87;
            rightForce.x = m_throttle * v89;
            rightForce.y = m_throttle * v88;
            rightForce.z = m_throttle * v87;
            (v77) = -(m_throttle * v89);
            (v76) = -(m_throttle * v88);
            (v75) = -(m_throttle * v87);
            leftForce.x = v77;
            leftForce.y = v76;
            leftForce.z = v75;
            v73 = (float)(vehParams->m_buoyancybox_max[1] - vehParams->m_buoyancybox_min[1]) / 2.0;
            p_y = &body->m_mat.y;
            v71 = v73 * body->m_mat.y.x;
            v70 = v73 * body->m_mat.y.y;
            v69 = v73 * body->m_mat.y.z;
            centerOffset.x = v71;
            centerOffset.y = v70;
            centerOffset.z = v69;
            v67 = center.x + v71;
            v66 = center.y + v70;
            v65 = center.z + v69;
            v62.w = center.x + v71;
            v63 = center.y + v70;
            v64 = center.z + v69;
            //rigid_body::add_force(body, &leftForce, (phys_vec3 *)&v62.w, 1.0);
            body->add_force(&leftForce, (phys_vec3 *)&v62.w, 1.0);
            v62.z = center.x - centerOffset.x;
            v62.y = center.y - centerOffset.y;
            v62.x = center.z - centerOffset.z;
            v59.w = center.x - centerOffset.x;
            v60 = center.y - centerOffset.y;
            v61 = center.z - centerOffset.z;
            //rigid_body::add_force(body, &rightForce, (phys_vec3 *)&v59.w, 1.0);
            body->add_force(&rightForce, (phys_vec3 *)&v59.w, 1.0);
        }
        else
        {
            v59.z = vehParams->m_boat_motor_offset[2];
            //LODWORD(v59.y) = &body->m_mat.z;
            v59.x = v59.z * body->m_mat.z.x;
            v58 = v59.z * body->m_mat.z.y;
            v57 = v59.z * body->m_mat.z.z;
            x = v59.x;
            v55 = v58;
            v56 = v57;
            v53 = vehParams->m_boat_motor_offset[0];
            p_m_mat = &body->m_mat;
            v51 = v53 * body->m_mat.x.x;
            v50 = v53 * body->m_mat.x.y;
            v49 = v53 * body->m_mat.x.z;
            v46 = v51;
            v47 = v50;
            v48 = v49;
            v45 = center.x + v51;
            v44 = center.y + v50;
            v43 = center.z + v49;
            v40 = center.x + v51;
            v41 = center.y + v50;
            v42 = center.z + v49;
            v39 = (float)(center.x + v51) + v59.x;
            v38 = (float)(center.y + v50) + v58;
            v37 = (float)(center.z + v49) + v57;
            motor_position.x = v39;
            motor_position.y = v38;
            motor_position.z = v37;
            accel = body->m_mat.x;
            v34 = rbveh->m_steer_factor;
            //rot = COERCE_FLOAT(LODWORD(v34) ^ _mask__NegFloat_) * vehParams->m_steer_angle_max;
            rot = (-(v34)) * vehParams->m_steer_angle_max;
            make_rotate(&rv, &body->m_mat.z, rot, 1000.0);
            v30 = phys_multiply(&v31, &rv, &accel);
            accel.x = v30->x;
            accel.y = v30->y;
            accel.z = v30->z;
            if (phys_drawNitrousVehicle->current.enabled)
            {
                Phys_NitrousVecToVec3(&motor_position, start);
                v28 = 50.0 * accel.x;
                v27 = 50.0 * accel.y;
                v26 = 50.0 * accel.z;
                v23 = 50.0 * accel.x;
                v24 = 50.0 * accel.y;
                v25 = 50.0 * accel.z;
                v22 = motor_position.x - (float)(50.0 * accel.x);
                v21 = motor_position.y - (float)(50.0 * accel.y);
                v20 = motor_position.z - (float)(50.0 * accel.z);
                end.w = v22;
                v18 = v21;
                v19 = v20;
                Phys_NitrousVecToVec3((phys_vec3 *)&end.w, &end.x);
                CL_AddDebugLine(start, &end.x, colorWhite, 0, 1);
            }
            turn_throttle = throttle;
            if (rbveh->m_vehicle_info->cameraMode == 1)
            {
                if (turn_throttle >= 1.0 || turn_throttle <= 0.0)
                {
                    if (turn_throttle == 0.0)
                    {
                        turn_throttle = forwardProportion * coast_steer_scaler;
                        v15 = rbveh->m_forward_vel;
                        if (v15 < 0.0)
                            turn_throttle = turn_throttle * -1.0;
                    }
                    else if (turn_throttle < 0.0)
                    {
                        v14 = rbveh->m_forward_vel;
                        if (v14 > 0.0)
                            turn_throttle = turn_throttle * -1.0;
                        v13 = rbveh->m_steer_factor;
                        v11 = (fabs(v13)) > 0.60000002
                            && (v12 = rbveh->m_forward_vel, v12 > 100.0);
                        apply_ebrake = v11;
                        if (v11)
                        {
                            rotSpeedFactor = rotSpeedFactor * vehParams->m_water_ebrake_power;
                            speedFactor = speedFactor * forward_accel_scale;
                            thisa->m_stopped_time = -0.6f;
                        }
                        else if (thisa->m_stopped_time < 0.0)
                        {
                            speedFactor = 0.0f;
                            rotSpeedFactor = 0.0f;
                        }
                        thisa->m_stopped_time = thisa->m_stopped_time + delta_t;
                    }
                }
                else
                {
                    turn_throttle = turn_throttle * analog_throttle_turn_factor;
                }
                if ((float)(turn_throttle - 1.0) < 0.0)
                    v9 = turn_throttle;
                else
                    v9 = 1.0f;
                if ((float)(-1.0 - turn_throttle) < 0.0)
                    v8 = v9;
                else
                    v8 = -1.0f;
                turn_throttle = v8;
            }
            y = body->m_mat.y;
            v6 = (float)((float)((float)((float)(y.x * accel.x) + (float)(y.y * accel.y)) + (float)(y.z * accel.z))
                * turn_throttle)
                * rotSpeedFactor;
            y.x = y.x * v6;
            y.y = y.y * v6;
            y.z = y.z * v6;
            //rigid_body::add_force(body, &y, &motor_position, 1.0);
            body->add_force(&y, &motor_position, 1.0);
            v5 = (float)((float)((float)((float)(forwardForce.x * accel.x) + (float)(forwardForce.y * accel.y))
                + (float)(forwardForce.z * accel.z))
                * throttle)
                * speedFactor;
            forwardForce.x = forwardForce.x * v5;
            forwardForce.y = forwardForce.y * v5;
            forwardForce.z = forwardForce.z * v5;
            //rigid_body::add_force(body, &forwardForce, &motor_position, 1.0);
            body->add_force(&forwardForce, &motor_position, 1.0);
        }
    }
}

//void __thiscall rigid_body::add_force(
//                const phys_vec3 *force,
//                const phys_vec3 *point,
//                float torque_mult)
//{
//    double v6; // st7
//    float v7; // [esp-30h] [ebp-3Ch]
//    float v8; // [esp-2Ch] [ebp-38h]
//    float v9; // [esp-28h] [ebp-34h]
//    float v10; // [esp-20h] [ebp-2Ch]
//    float v11; // [esp-20h] [ebp-2Ch]
//    float v12; // [esp-1Ch] [ebp-28h]
//    float v13; // [esp-1Ch] [ebp-28h]
//    float v14; // [esp-18h] [ebp-24h]
//    float v15; // [esp-18h] [ebp-24h]
//    float v16; // [esp-4h] [ebp-10h]
//    float v17; // [esp-4h] [ebp-10h]
//    float v18; // [esp-4h] [ebp-10h]
//    float v19; // [esp-4h] [ebp-10h]
//    float v20; // [esp-4h] [ebp-10h]
//    float v21; // [esp-4h] [ebp-10h]
//
//    this->m_force_sum.x = this->m_force_sum.x + force->x;
//    this->m_force_sum.y = this->m_force_sum.y + force->y;
//    this->m_force_sum.z = this->m_force_sum.z + force->z;
//    v10 = point->x - this->m_mat.w.x;
//    v12 = point->y - this->m_mat.w.y;
//    v14 = point->z - this->m_mat.w.z;
//    v7 = force->z * v12 - force->y * v14;
//    v8 = v14 * force->x - force->z * v10;
//    v9 = v10 * force->y - v12 * force->x;
//    v11 = v7 * torque_mult;
//    v13 = v8 * torque_mult;
//    v15 = torque_mult * v9;
//    this->m_torque_sum.x = this->m_torque_sum.x + v11;
//    this->m_torque_sum.y = v13 + this->m_torque_sum.y;
//    this->m_torque_sum.z = this->m_torque_sum.z + v15;
//    v6 = 100000.0;
//    v16 = fabs(this->m_force_sum.x);
//    if ( v16 > 100000.0
//        || (v17 = fabs(this->m_force_sum.y), v17 > 100000.0)
//        || (v18 = fabs(this->m_force_sum.z), v18 > 100000.0) )
//    {
//        phys_exec_debug_callback(this);
//        v6 = 100000.0;
//    }
//    v19 = fabs(this->m_torque_sum.x);
//    if ( v19 > v6 || (v20 = fabs(this->m_torque_sum.y), v20 > v6) || (v21 = fabs(this->m_torque_sum.z), v6 < v21) )
//        phys_exec_debug_callback(this);
//}
//
//