#include <Windows.h>
#include "devgui_input.h"
#include "devgui.h"
#include <qcommon/cmd.h>
#include <win32/win_gamepad.h>
#include <client_mp/cl_input_mp.h>
#include <client/cl_keys.h>
#include <qcommon/com_clients.h>

#include <cmath>
#include <universal/assertive.h>
#include <client/cl_gamepad.h>
#include <qcommon/common.h>

DevGuiInput s_input;

int s_butMapsKey[13] =
{ 154, 155, 156, 157, 13, 27, 9, 32, 161, 162, 171, 173, 174 };

GamePadButton s_butMapsGamepad[13] =
{
  GPAD_UP,
  GPAD_DOWN,
  GPAD_LEFT,
  GPAD_RIGHT,
  GPAD_A,
  GPAD_B,
  GPAD_X,
  GPAD_Y,
  GPAD_L_SHLDR,
  GPAD_R_SHLDR,
  GPAD_START,
  GPAD_R_SHLDR,
  GPAD_Y
};

cmd_function_s DevGui_Toggle_VAR;
void __cdecl DevGui_InputInit()
{
    Cmd_AddCommandInternal("devgui", DevGui_Toggle, &DevGui_Toggle_VAR);
}

void __cdecl DevGui_InputShutdown()
{
    Cmd_RemoveCommand("devgui");
}

void __cdecl DevGui_SelectGamepad(int gamePadIndex)
{
    s_input.selectedGamePadIndex = gamePadIndex;
}

char __cdecl DevGui_InputUpdate(int localClientNum, float deltaTime)
{
    int butIndex; // [esp+14h] [ebp-8h]

    DevGui_InputUpdateGamepad();
    DevGui_InputUpdateMouse();
    for ( butIndex = 0; butIndex < 13; ++butIndex )
    {
        s_input.prevButtonDown[butIndex] = s_input.buttonDown[butIndex];
        if ( s_input.gamePadIndex >= 0 )
            s_input.buttonDown[butIndex] = GPad_GetButton(s_input.gamePadIndex, s_butMapsGamepad[butIndex]) > 0.0;
        s_input.buttonDown[butIndex] = Key_IsDown(localClientNum, s_butMapsKey[butIndex]) != 0;
    }
    DevGui_UpdateScrollInputs(localClientNum);
    DevGui_UpdateScrollStates(deltaTime, s_input.digitalStates, s_input.digitalAxis, s_input.digitalTimes);
    if ( s_input.gamePadIndex >= 0 )
        DevGui_UpdateScrollStates(deltaTime, s_input.analogStates, s_input.analogAxis, s_input.analogTimes);
    DevGui_UpdateMenuScroll(deltaTime);
    CL_ClearKeys(localClientNum);
    return 1;
}

void __cdecl DevGui_UpdateScrollInputs(int localClientNum)
{
    int ControllerIndex; // eax
    float v2; // [esp+0h] [ebp-40h]
    float v3; // [esp+4h] [ebp-3Ch]
    float v4; // [esp+8h] [ebp-38h]
    float v5; // [esp+Ch] [ebp-34h]
    float v6; // [esp+10h] [ebp-30h]
    float v7; // [esp+14h] [ebp-2Ch]
    float right; // [esp+1Ch] [ebp-24h]
    float left; // [esp+20h] [ebp-20h]
    float lx; // [esp+24h] [ebp-1Ch]
    float lxa; // [esp+24h] [ebp-1Ch]
    float up; // [esp+2Ch] [ebp-14h]
    float down; // [esp+30h] [ebp-10h]
    float ltrig; // [esp+34h] [ebp-Ch]
    float ly; // [esp+38h] [ebp-8h]
    float lya; // [esp+38h] [ebp-8h]
    float rtrig; // [esp+3Ch] [ebp-4h]

    lx = 0.0f;
    ly = 0.0f;
    rtrig = 0.0f;
    ltrig = 0.0f;
    if ( DevGui_IsButtonDown(INPUT_RIGHT) )
        v7 = 1.0f;
    else
        v7 = 0.0f;
    right = v7;
    if ( DevGui_IsButtonDown(INPUT_LEFT) )
        v6 = 1.0f;
    else
        v6 = 0.0f;
    left = v6;
    if ( DevGui_IsButtonDown(INPUT_UP) )
        v5 = 1.0f;
    else
        v5 = 0.0f;
    up = v5;
    if ( DevGui_IsButtonDown(INPUT_DOWN) )
        v4 = 1.0f;
    else
        v4 = 0.0f;
    down = v4;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( GPad_IsActive(ControllerIndex) )
    {
        lxa = GPad_GetStick(s_input.gamePadIndex, GPAD_LX);
        if ( lxa < 0.0 )
            v3 = -1.0f;
        else
            v3 = 1.0f;
        lx = (float)(lxa * lxa) * v3;
        lya = GPad_GetStick(s_input.gamePadIndex, GPAD_LY);
        if ( lya < 0.0 )
            v2 = -1.0f;
        else
            v2 = 1.0f;
        ly = (float)(lya * lya) * v2;
        rtrig = GPad_GetButton(s_input.gamePadIndex, GPAD_R_TRIG);
        ltrig = GPad_GetButton(s_input.gamePadIndex, GPAD_L_TRIG);
    }
    if ( s_input.gamePadIndex < 0 && Key_IsDown(localClientNum, 200) )
    {
        lx = s_input.mousePos[0];
        //ly = -s_input.mousePos[1];
        ly = -s_input.mousePos[1];
    }
    s_input.mousePos[0] = 0.0f;
    s_input.mousePos[1] = 0.0f;
    if ( (v7 != 0.0 || v6 != 0.0) && (v5 != 0.0 || v4 != 0.0) )
    {
        right = 0.0f;
        left = 0.0f;
        up = 0.0f;
        down = 0.0f;
    }
    s_input.scrollScale = (float)((float)(ltrig * 1.0) + 1.0) * (float)((float)(rtrig * 4.0) + 1.0);
    s_input.digitalAxis[0] = right - left;
    s_input.digitalAxis[1] = up - down;
    s_input.analogAxis[0] = lx;
    s_input.analogAxis[1] = ly;
}

void __cdecl DevGui_UpdateScrollStates(float deltaTime, DevGuiInputState *states, float *axis, float *times)
{
    DevGuiInputState v4; // [esp+0h] [ebp-Ch]
    int axisIndex; // [esp+8h] [ebp-4h]

    for ( axisIndex = 0; axisIndex < 2; ++axisIndex )
    {
        if ( axis[axisIndex] == 0.0 )
        {
            states[axisIndex] = SCROLL_NONE;
            times[axisIndex] = 0.0f;
        }
        else
        {
            v4 = states[axisIndex];
            if ( v4 )
            {
                if ( v4 == SCROLL_PRESSED )
                {
                    states[axisIndex] = SCROLL_STALLED;
                    times[axisIndex] = 0.0f;
                }
                else if ( v4 == SCROLL_STALLED )
                {
                    times[axisIndex] = times[axisIndex] + deltaTime;
                    if ( times[axisIndex] > 0.25 )
                    {
                        states[axisIndex] = SCROLL_HELD;
                        times[axisIndex] = 0.0f;
                    }
                }
                else
                {
                    times[axisIndex] = times[axisIndex] + deltaTime;
                }
            }
            else if ( fabs(axis[axisIndex]) > 0.40000001 )
            {
                states[axisIndex] = SCROLL_PRESSED;
            }
        }
    }
}

void __cdecl DevGui_UpdateMenuScroll(float deltaTime)
{
    float v1; // [esp+0h] [ebp-34h]
    float v2; // [esp+4h] [ebp-30h]
    bool v3; // [esp+8h] [ebp-2Ch]
    bool v4; // [esp+Ch] [ebp-28h]
    float adjustedAnalogAxis[2]; // [esp+20h] [ebp-14h]
    int axisIndex; // [esp+28h] [ebp-Ch]
    bool pressed; // [esp+2Eh] [ebp-6h]
    bool held; // [esp+2Fh] [ebp-5h]
    float axis; // [esp+30h] [ebp-4h]

    if ( fabs(s_input.analogAxis[0]) <= fabs(s_input.analogAxis[1]) )
    {
        adjustedAnalogAxis[0] = 0.0f;
        adjustedAnalogAxis[1] = s_input.analogAxis[1];
    }
    else
    {
        adjustedAnalogAxis[0] = s_input.analogAxis[0];
        adjustedAnalogAxis[1] = 0.0f;
    }
    for ( axisIndex = 0; axisIndex < 2; ++axisIndex )
    {
        s_input.menuScroll[axisIndex] = 0;
        v4 = s_input.digitalStates[axisIndex] == SCROLL_HELD || s_input.analogStates[axisIndex] == SCROLL_HELD;
        held = v4;
        v3 = s_input.digitalStates[axisIndex] == SCROLL_PRESSED || s_input.analogStates[axisIndex] == SCROLL_PRESSED;
        pressed = v3;
        axis = (float)(s_input.digitalAxis[axisIndex] + adjustedAnalogAxis[axisIndex]) * s_input.scrollScale;
        if ( held )
        {
            for ( s_input.menuScrollTime[axisIndex] = (float)(deltaTime * fabs(axis))
                                                                                            + s_input.menuScrollTime[axisIndex];
                        s_input.menuScrollTime[axisIndex] > 0.1;
                        s_input.menuScrollTime[axisIndex] = s_input.menuScrollTime[axisIndex] - 0.1 )
            {
                if ( axis < 0.0 )
                    v2 = -1.0f;
                else
                    v2 = 1.0f;
                s_input.menuScroll[axisIndex] += (int)v2;
            }
        }
        else if ( pressed )
        {
            if ( axis < 0.0 )
                v1 = -1.0f;
            else
                v1 = 1.0f;
            s_input.menuScroll[axisIndex] += (int)v1;
            s_input.menuScrollTime[axisIndex] = 0.0f;
        }
    }
}

char __cdecl DevGui_InputUpdateGamepad()
{
    s_input.gamePadIndex = s_input.selectedGamePadIndex;
    if ( s_input.selectedGamePadIndex >= 0 && GPad_IsActive(s_input.gamePadIndex) )
    {
        s_input.sliderScrollTime = 10.0f;
        s_input.sliderScrollMaxTimeStep = 0.1f;
        return 1;
    }
    else
    {
        s_input.gamePadIndex = -1;
        return 0;
    }
}

void DevGui_InputUpdateMouse()
{
    s_input.sliderScrollTime = 100.0f;
    s_input.sliderScrollMaxTimeStep = 0.3f;
}

void __cdecl DevGui_MouseEvent(int dx, int dy)
{
    s_input.mousePos[0] = (float)dx;
    s_input.mousePos[1] = (float)dy;
}

__int16 __cdecl DevGui_GetMenuScroll(DevGuiInputAxis axis)
{
    return s_input.menuScroll[axis];
}

__int64 __cdecl DevGui_UpdateIntScroll(float deltaTime, __int64 value, __int64 min, __int64 max, DevGuiInputAxis axis)
{
    float v6; // [esp+0h] [ebp-48h]
    __int64 range; // [esp+38h] [ebp-10h]
    float stepTime; // [esp+40h] [ebp-8h]
    float stepTimea; // [esp+40h] [ebp-8h]
    int scroll; // [esp+44h] [ebp-4h]
    __int64 valuea; // [esp+54h] [ebp+Ch]

    range = max - min;
    if ( (((unsigned __int64)(max - min) >> 32) & 0x80000000) != 0LL
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui_input.cpp", 486, 0, "%s", "range >= 0") )
    {
        __debugbreak();
    }
    if ( (unsigned int)axis >= SCROLL_AXIS_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui_input.cpp",
                    487,
                    0,
                    "%s",
                    "axis >= SCROLL_XAXIS && axis < SCROLL_AXIS_COUNT") )
    {
        __debugbreak();
    }
    if ( !range )
        return 0;
    scroll = 0;
    if ( s_input.digitalStates[axis] == SCROLL_HELD )
    {
        stepTime = 0.1 / s_input.scrollScale;
        if ( (float)(0.1 / s_input.scrollScale) > s_input.sliderScrollMaxTimeStep )
            stepTime = s_input.sliderScrollMaxTimeStep;
        for ( s_input.digitalSliderTime = s_input.digitalSliderTime + deltaTime;
                    s_input.digitalSliderTime > stepTime;
                    s_input.digitalSliderTime = s_input.digitalSliderTime - stepTime )
        {
            scroll += SnapFloatToInt(s_input.digitalAxis[axis]);
        }
    }
    else if ( s_input.digitalStates[axis] == SCROLL_PRESSED )
    {
        scroll = SnapFloatToInt(s_input.digitalAxis[axis]);
    }
    else
    {
        s_input.digitalSliderTime = 0.0f;
    }
    if ( s_input.analogAxis[axis] == 0.0 )
    {
        s_input.analogSliderTime = 0.0f;
    }
    else
    {
        stepTimea = s_input.sliderScrollTime
                            / (double)range
                            / fabs(s_input.analogAxis[axis] * s_input.scrollScale);
        if ( stepTimea > s_input.sliderScrollMaxTimeStep )
            stepTimea = s_input.sliderScrollMaxTimeStep;
        s_input.analogSliderTime = s_input.analogSliderTime + deltaTime;
        while ( s_input.analogSliderTime >= stepTimea )
        {
            if ( s_input.analogAxis[axis] < 0.0 )
                v6 = -1.0f;
            else
                v6 = 1.0f;
            scroll += (int)v6;
            s_input.analogSliderTime = s_input.analogSliderTime - stepTimea;
            if ( s_input.gamePadIndex < 0 && fabs(s_input.analogAxis[axis]) <= 2.0 )
            {
                s_input.analogSliderTime = 0.0f;
                break;
            }
        }
    }
    valuea = value + scroll;
    if ( valuea <= min )
        return min;
    if ( valuea < max )
        return valuea;
    return max;
}

double __cdecl DevGui_UpdateFloatScroll(
                float deltaTime,
                float value,
                float min,
                float max,
                float step,
                DevGuiInputAxis axis)
{
    float stepTime; // [esp+8h] [ebp-8h]
    float scroll; // [esp+Ch] [ebp-4h]
    float valuea; // [esp+1Ch] [ebp+Ch]

    if ( (float)(max - min) < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui_input.cpp", 579, 0, "%s", "range >= 0") )
    {
        __debugbreak();
    }
    if ( step == 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui_input.cpp", 580, 0, "%s", "step") )
    {
        __debugbreak();
    }
    if ( (unsigned int)axis >= SCROLL_AXIS_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui_input.cpp",
                    581,
                    0,
                    "%s",
                    "axis >= SCROLL_XAXIS && axis < SCROLL_AXIS_COUNT") )
    {
        __debugbreak();
    }
    scroll = (float)((float)((float)(s_input.analogAxis[axis] * s_input.scrollScale) / s_input.sliderScrollTime)
                                 * (float)(max - min))
                 * deltaTime;
    if ( s_input.digitalStates[axis] == SCROLL_HELD )
    {
        stepTime = 0.1 / s_input.scrollScale;
        if ( (float)(0.1 / s_input.scrollScale) > s_input.sliderScrollMaxTimeStep )
            stepTime = s_input.sliderScrollMaxTimeStep;
        for ( s_input.digitalSliderTime = s_input.digitalSliderTime + deltaTime;
                    s_input.digitalSliderTime > stepTime;
                    s_input.digitalSliderTime = s_input.digitalSliderTime - stepTime )
        {
            scroll = (float)(s_input.digitalAxis[axis] * step) + scroll;
        }
    }
    else if ( s_input.digitalStates[axis] == SCROLL_PRESSED )
    {
        scroll = (float)(s_input.digitalAxis[axis] * step) + scroll;
    }
    valuea = value + scroll;
    if ( min >= valuea )
        return min;
    if ( valuea < max )
        return valuea;
    return max;
}

bool __cdecl DevGui_IsButtonDown(DevGuiInputButton button)
{
    return s_input.buttonDown[button];
}

bool __cdecl DevGui_IsButtonPressed(DevGuiInputButton button)
{
    return s_input.buttonDown[button] && !s_input.prevButtonDown[button];
}

bool __cdecl DevGui_IsButtonReleased(DevGuiInputButton button)
{
    return !s_input.buttonDown[button] && s_input.prevButtonDown[button];
}

