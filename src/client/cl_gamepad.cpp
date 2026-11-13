#include "cl_gamepad.h"
#include <win32/win_shared.h>
#include <qcommon/cmd.h>
#include <universal/dvar.h>
#include <universal/q_shared.h>
#include <qcommon/common.h>
#include <universal/com_files.h>
#include "cl_main.h"
#include "client.h"
#include "cl_keys.h"
#include <cgame_mp/cg_main_mp.h>
#include <server_mp/sv_init_mp.h>
#include <win32/win_gamepad.h>
#include <devgui/devgui.h>
#include <cgame_mp/cg_newDraw_mp.h>
#include <ui/keycodes.h>
#include <demo/demo_playback.h>
#include <universal/offline.h>
#include <bgame/bg_unlockable_items.h>
#include <qcommon/com_clients.h>
#include <cgame/cg_compass.h>
#include <cgame_mp/cg_scoreboard_mp.h>
#include <cgame/cg_gamepad.h>

int lastGamepadEventTimeGlobal;
GpadAxesGlob gaGlobs[1];

int keyMask[32];

GamepadPhysicalAxis axisSameStick[6] =
{
  GPAD_PHYSAXIS_RSTICK_Y,
  GPAD_PHYSAXIS_RSTICK_X,
  GPAD_PHYSAXIS_LSTICK_Y,
  GPAD_PHYSAXIS_LSTICK_X,
  GPAD_PHYSAXIS_NONE,
  GPAD_PHYSAXIS_NONE
};

keyNum_t menuScrollButtonList[8] =
{
  K_DPAD_UP,
  K_DPAD_DOWN,
  K_DPAD_LEFT,
  K_DPAD_RIGHT,
  K_FIRSTGAMEPADBUTTON_RANGE_3,
  K_APAD_DOWN,
  K_APAD_LEFT,
  K_APAD_RIGHT
};

const char *physicalAxisNames[6] =
{
  "A_RSTICK_X",
  "A_RSTICK_Y",
  "A_LSTICK_X",
  "A_LSTICK_Y",
  "A_RTRIGGER",
  "A_LTRIGGER"
};

const char *inputTypeNames[2] =
{ "MAP_LINEAR", "MAP_SQUARED" };

const char *virtualAxisNames[6] =
{ "VA_SIDE", "VA_FORWARD", "VA_UP", "VA_YAW", "VA_PITCH", "VA_ATTACK" };

void __cdecl CL_ResetLastGamePadEventTime()
{
    lastGamepadEventTimeGlobal = Sys_Milliseconds();
}


cmd_function_s Axis_Bind_f_VAR;
cmd_function_s Axis_Unbindall_f_VAR;
cmd_function_s Bind_GP_SticksConfigs_f_VAR;
cmd_function_s Bind_GP_ButtonsConfigs_f_VAR;
void __cdecl CL_InitGamepadCommands()
{
    Cmd_AddCommandInternal("bindaxis", Axis_Bind_f, &Axis_Bind_f_VAR);
    Cmd_AddCommandInternal("unbindallaxis", Axis_Unbindall_f, &Axis_Unbindall_f_VAR);
    Cmd_AddCommandInternal("bindgpsticksconfigs", Bind_GP_SticksConfigs_f, &Bind_GP_SticksConfigs_f_VAR);
    Cmd_AddCommandInternal("bindgpbuttonsconfigs", Bind_GP_ButtonsConfigs_f, &Bind_GP_ButtonsConfigs_f_VAR);
}

void __cdecl Bind_GP_SticksConfigs_f()
{
    const char *String; // eax
    char *v1; // eax

    if ( *Dvar_GetString("gpad_sticksConfig") )
    {
        String = Dvar_GetString("gpad_sticksConfig");
        v1 = va("exec %s\n", String);
        Cmd_ExecuteSingleCommand(0, 0, v1);
    }
}

void __cdecl Bind_GP_ButtonsConfigs_f()
{
    const char *String; // eax
    char *v1; // eax

    if ( *Dvar_GetString("gpad_buttonsConfig") )
    {
        String = Dvar_GetString("gpad_buttonsConfig");
        v1 = va("exec %s\n", String);
        Cmd_ExecuteSingleCommand(0, 0, v1);
    }
}

void __cdecl CL_InitGamepadAxisBindings()
{
    int localClientNum; // [esp+0h] [ebp-8h]
    int axisIndex; // [esp+4h] [ebp-4h]

    for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
    {
        for ( axisIndex = 0; axisIndex < 6; ++axisIndex )
        {
            gaGlobs[localClientNum].virtualAxes[axisIndex].physicalAxis = GPAD_PHYSAXIS_NONE;
            gaGlobs[localClientNum].virtualAxes[axisIndex].mapType = GPAD_MAP_SQUARED;
            gaGlobs[localClientNum].axesValues[axisIndex] = 0;
        }
    }
}

void __cdecl Axis_Bind_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    char buffer[1024]; // [esp+38h] [ebp-418h] BYREF
    int c; // [esp+43Ch] [ebp-14h]
    GamepadMapping inputType; // [esp+440h] [ebp-10h]
    GamepadPhysicalAxis physicalAxis; // [esp+444h] [ebp-Ch]
    GamepadVirtualAxis virtualAxis; // [esp+448h] [ebp-8h]
    char *cmd; // [esp+44Ch] [ebp-4h]

    c = Cmd_Argc();
    if ( c < 2 )
    {
        Com_Printf(14, "bindaxis <real axis> [virtual axis] : setup pad axis\n");
        return;
    }
    v0 = Cmd_Argv(1);
    I_strncpyz(buffer, v0, 1024);
    I_strupr(buffer);
    cmd = buffer;
    physicalAxis = Gamepad_StringToPhysicalAxis(buffer);
    if ( physicalAxis == GPAD_PHYSAXIS_NONE )
    {
        v1 = Cmd_Argv(1);
        Com_Printf(14, "\"%s\" isn't a valid axis\n", v1);
        return;
    }
    if ( c <= 2 )
    {
        virtualAxis = GPAD_VIRTAXIS_NONE;
    }
    else
    {
        v2 = Cmd_Argv(2);
        I_strncpyz(buffer, v2, 1024);
        I_strupr(buffer);
        virtualAxis = Axis_StringToVirtualAxis(buffer);
        if ( virtualAxis == GPAD_VIRTAXIS_NONE )
        {
            v3 = Cmd_Argv(2);
            Com_Printf(14, "\"%s\" isn't a valid axis\n", v3);
            return;
        }
    }
    if ( c <= 3 )
    {
        inputType = GPAD_MAP_SQUARED;
    }
    else
    {
        v4 = Cmd_Argv(3);
        I_strncpyz(buffer, v4, 1024);
        I_strupr(buffer);
        inputType = Gamepad_InputTypeStringToId(buffer);
        if ( inputType == GPAD_MAP_NONE )
        {
            v5 = Cmd_Argv(3);
            Com_Printf(14, "\"%s\" isn't a valid input type\n", v5);
            return;
        }
    }
    Gamepad_BindAxis(gaGlobs, physicalAxis, virtualAxis, inputType);
}

GamepadPhysicalAxis __cdecl Gamepad_StringToPhysicalAxis(const char *axisName)
{
    unsigned int axis; // [esp+0h] [ebp-4h]

    if ( !axisName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_gamepad.cpp", 256, 0, "%s", "axisName") )
    {
        __debugbreak();
    }
    for ( axis = 0; axis < 6; ++axis )
    {
        if ( !I_stricmp(axisName, physicalAxisNames[axis]) )
            return (GamepadPhysicalAxis)axis;
    }
    return (GamepadPhysicalAxis)-1;
}

GamepadVirtualAxis __cdecl Axis_StringToVirtualAxis(const char *axisName)
{
    unsigned int axis; // [esp+0h] [ebp-4h]

    if ( !axisName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_gamepad.cpp", 270, 0, "%s", "axisName") )
    {
        __debugbreak();
    }
    for ( axis = 0; axis < 6; ++axis )
    {
        if ( !I_stricmp(axisName, virtualAxisNames[axis]) )
            return (GamepadVirtualAxis)axis;
    }
    return (GamepadVirtualAxis)-1;
}

GamepadMapping __cdecl Gamepad_InputTypeStringToId(const char *name)
{
    unsigned int type; // [esp+0h] [ebp-4h]

    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_gamepad.cpp", 298, 0, "%s", "name") )
        __debugbreak();
    for ( type = 0; type < 2; ++type )
    {
        if ( !I_stricmp(name, inputTypeNames[type]) )
            return (GamepadMapping)type;
    }
    return (GamepadMapping)-1;
}

void __cdecl Gamepad_BindAxis(
                GpadAxesGlob *gaGlob,
                GamepadPhysicalAxis realIndex,
                GamepadVirtualAxis axisIndex,
                GamepadMapping mapType)
{
    if ( realIndex != GPAD_PHYSAXIS_NONE
        && (unsigned int)realIndex >= GPAD_PHYSAXIS_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_gamepad.cpp",
                    317,
                    0,
                    "%s\n\t(realIndex) = %i",
                    "(realIndex == GPAD_PHYSAXIS_NONE || (realIndex >= 0 && realIndex < GPAD_PHYSAXIS_COUNT))",
                    realIndex) )
    {
        __debugbreak();
    }
    if ( axisIndex >= GPAD_VIRTAXIS_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_gamepad.cpp",
                    318,
                    0,
                    "%s",
                    "axisIndex < GPAD_VIRTAXIS_COUNT") )
    {
        __debugbreak();
    }
    if ( mapType >= GPAD_MAP_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_gamepad.cpp",
                    319,
                    0,
                    "%s",
                    "mapType < GPAD_MAP_COUNT") )
    {
        __debugbreak();
    }
    gaGlob->virtualAxes[axisIndex].physicalAxis = realIndex;
    gaGlob->virtualAxes[axisIndex].mapType = mapType;
    dvar_modifiedFlags |= 1u;
}

void __cdecl Axis_Unbindall_f()
{
    int axis; // [esp+4h] [ebp-4h]

    for ( axis = 0; axis < 6; ++axis )
        gaGlobs[0].virtualAxes[axis].physicalAxis = GPAD_PHYSAXIS_NONE;
}

void __cdecl Gamepad_WriteBindings(int localClientNum, int f)
{
    GpadAxesGlob *gaGlob; // [esp+0h] [ebp-14h]
    const char *inputTypeName; // [esp+4h] [ebp-10h]
    const char *virtualAxisName; // [esp+8h] [ebp-Ch]
    const char *realAxisName; // [esp+Ch] [ebp-8h]
    GamepadVirtualAxis axis; // [esp+10h] [ebp-4h]

    gaGlob = &gaGlobs[localClientNum];
    FS_Printf(f, (char*)"unbindallaxis\n");
    for ( axis = GPAD_VIRTAXIS_SIDE; axis < GPAD_VIRTAXIS_COUNT; ++axis )
    {
        if ( gaGlob->virtualAxes[axis].physicalAxis != GPAD_PHYSAXIS_NONE )
        {
            virtualAxisName = Gamepad_VirtualAxisName(axis);
            realAxisName = Axis_NumToAxisString(gaGlob->virtualAxes[axis].physicalAxis);
            inputTypeName = Gamepad_InputTypeName(gaGlob->virtualAxes[axis].mapType);
            if ( !realAxisName
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_gamepad.cpp", 424, 0, "%s", "realAxisName") )
            {
                __debugbreak();
            }
            if ( !virtualAxisName
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_gamepad.cpp",
                            425,
                            0,
                            "%s",
                            "virtualAxisName") )
            {
                __debugbreak();
            }
            if ( !inputTypeName
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_gamepad.cpp", 426, 0, "%s", "inputTypeName") )
            {
                __debugbreak();
            }
            FS_Printf(f, (char*)"bindaxis %s %s %s\n", realAxisName, virtualAxisName, inputTypeName);
        }
    }
}

const char *__cdecl Axis_NumToAxisString(GamepadPhysicalAxis axis)
{
    if ( (unsigned int)axis >= GPAD_PHYSAXIS_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_gamepad.cpp",
                    282,
                    0,
                    "%s\n\t(axis) = %i",
                    "(axis >= 0 && axis < GPAD_PHYSAXIS_COUNT)",
                    axis) )
    {
        __debugbreak();
    }
    return physicalAxisNames[axis];
}

const char *__cdecl Gamepad_VirtualAxisName(GamepadVirtualAxis axis)
{
    if ( (unsigned int)axis >= GPAD_VIRTAXIS_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_gamepad.cpp",
                    289,
                    0,
                    "%s\n\t(axis) = %i",
                    "(axis >= 0 && axis < GPAD_VIRTAXIS_COUNT)",
                    axis) )
    {
        __debugbreak();
    }
    return virtualAxisNames[axis];
}

const char *__cdecl Gamepad_InputTypeName(GamepadMapping mapType)
{
    if ( (unsigned int)mapType >= GPAD_MAP_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_gamepad.cpp",
                    310,
                    0,
                    "%s\n\t(mapType) = %i",
                    "(mapType >= 0 && mapType < GPAD_MAP_COUNT)",
                    mapType) )
    {
        __debugbreak();
    }
    return inputTypeNames[mapType];
}

void __cdecl CL_GamepadEvent(int portIndex, unsigned int physicalAxis, int value)
{
    int v3; // [esp+0h] [ebp-14h]
    connstate_t clcState; // [esp+Ch] [ebp-8h]
    bool maska; // [esp+10h] [ebp-4h]
    int maskb; // [esp+10h] [ebp-4h]
    int maskc; // [esp+10h] [ebp-4h]
    int maskd; // [esp+10h] [ebp-4h]
    int maske; // [esp+10h] [ebp-4h]
    int mask; // [esp+10h] [ebp-4h]

    if ( physicalAxis >= 6 )
        Com_Error(ERR_DROP, "CL_GamepadEvent: bad axis %i", physicalAxis);
    clcState = CL_GetLocalClientConnectionState(0);
    if ( (clcState == CA_CINEMATIC || clcState == CA_LOGO || clcState == CA_DISCONNECTED)
        && gaGlobs[0].axesValues[physicalAxis] != value )
    {
        CL_ResetLastGamePadEventTime();
    }
    CL_GamepadGenerateAPad(0, portIndex, physicalAxis);
    gaGlobs[0].axesValues[physicalAxis] = value;
    maska = Key_IsDown(0, 16) != 0;
    maskb = maska | (Key_IsDown(0, 17) != 0 ? 2 : 0);
    maskc = maskb | (Key_IsDown(0, 18) != 0 ? 4 : 0);
    maskd = maskc | (Key_IsDown(0, 19) != 0 ? 8 : 0);
    maske = maskd | (Key_IsDown(0, 20) != 0 ? 0x10 : 0);
    mask = maske | (Key_IsDown(0, 21) != 0 ? 0x20 : 0);
    if ( keyMask[0] != 18 || mask == 18 )
    {
        if ( keyMask[0] == 19 && mask != 19 && sv_debugPacketContentsQuick )
        {
            v3 = sv_debugPacketContentsQuick->current.integer + 1;
            if ( v3 > sv_debugPacketContentsQuick->domain.integer.max )
                v3 = 0;
            Dvar_SetInt((dvar_s *)sv_debugPacketContentsQuick, v3);
            Com_Printf(16, "Network logging: %d\n", sv_debugPacketContentsQuick->current.integer);
        }
    }
    else
    {
        Dvar_SetBool((dvar_s *)cg_wadefps, !cg_wadefps->current.enabled);
    }
    keyMask[0] = mask;
}

void __cdecl CL_GamepadGenerateAPad(int localClientNum, int portIndex, unsigned int physicalAxis)
{
    GamePadStick stick; // [esp+0h] [ebp-1Ch]
    GamePadStick stickForAxis[6]; // [esp+4h] [ebp-18h]

    stickForAxis[0] = GPAD_RX;
    stickForAxis[1] = GPAD_RY;
    stickForAxis[2] = GPAD_LX;
    stickForAxis[3] = GPAD_LY;
    stickForAxis[4] = GPAD_INVALID;
    stickForAxis[5] = GPAD_INVALID;
    if ( physicalAxis >= 6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_gamepad.cpp",
                    480,
                    0,
                    "%s",
                    "physicalAxis >= 0 && physicalAxis < GPAD_PHYSAXIS_COUNT") )
    {
        __debugbreak();
    }
    stick = stickForAxis[physicalAxis];
    if ( stick )
    {
        if (GPad_IsStickPressed(portIndex, stick, GPAD_STICK_POS) || GPad_IsStickPressed(portIndex, stick, GPAD_STICK_NEG))
        {
            //BLOPS_NULLSUB();
        }
        if ( GPad_IsStickReleased(portIndex, stick, GPAD_STICK_POS) || GPad_IsStickReleased(portIndex, stick, GPAD_STICK_NEG) )
        {
            //BLOPS_NULLSUB();
        }
    }
}

double __cdecl CL_GamepadAxisValue(int localClientNum, unsigned int virtualAxis)
{
    const GpadAxesGlob *gaGlob; // [esp+0h] [ebp-14h]
    float axisDeflection; // [esp+4h] [ebp-10h]
    GamepadPhysicalAxis otherSameStickAxis; // [esp+8h] [ebp-Ch]
    float otherAxisDeflection; // [esp+Ch] [ebp-8h]
    GamepadPhysicalAxis physicalAxis; // [esp+10h] [ebp-4h]

    if ( virtualAxis >= 6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_gamepad.cpp",
                    713,
                    0,
                    "%s\n\t(virtualAxis) = %i",
                    "(virtualAxis >= 0 && virtualAxis < GPAD_VIRTAXIS_COUNT)",
                    virtualAxis) )
    {
        __debugbreak();
    }
    gaGlob = &gaGlobs[localClientNum];
    physicalAxis = gaGlob->virtualAxes[virtualAxis].physicalAxis;
    if ( DevGui_IsActive() )
        return 0.0;
    if ( physicalAxis == GPAD_PHYSAXIS_NONE )
    {
        axisDeflection = 0.0f;
    }
    else
    {
        axisDeflection = (float)gaGlob->axesValues[physicalAxis] * 0.000015259022;
        if ( gaGlob->virtualAxes[virtualAxis].mapType == GPAD_MAP_SQUARED )
        {
            otherSameStickAxis = axisSameStick[physicalAxis];
            if ( otherSameStickAxis == GPAD_PHYSAXIS_NONE )
                otherAxisDeflection = 0.0f;
            else
                otherAxisDeflection = (float)gaGlob->axesValues[otherSameStickAxis] * 0.000015259022;
            axisDeflection = sqrtf((float)(axisDeflection * axisDeflection) + (float)(otherAxisDeflection * otherAxisDeflection))
                                         * axisDeflection;
        }
    }
    if ( fabs(axisDeflection) < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_gamepad.cpp",
                    743,
                    1,
                    "%s",
                    "I_fabs( axisDeflection ) >= 0.0f") )
    {
        __debugbreak();
    }
    if ( fabs(axisDeflection) > 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_gamepad.cpp",
                    744,
                    1,
                    "%s",
                    "I_fabs( axisDeflection ) <= 1.0f") )
    {
        __debugbreak();
    }
    return axisDeflection;
}

void __cdecl CL_GamepadResetMenuScrollTime(int localClientNum, int key, int down, unsigned int time)
{
    int buttonIndex; // [esp+8h] [ebp-8h]
    int scrollDelayFirst; // [esp+Ch] [ebp-4h]

    if ( down )
    {
        scrollDelayFirst = Dvar_GetInt("gpad_menu_scroll_delay_first");
        for ( buttonIndex = 0; buttonIndex < 8; ++buttonIndex )
        {
            if ( key == menuScrollButtonList[buttonIndex] )
            {
                CL_GetLocalClientUIGlobals(localClientNum)->nextScrollTime = scrollDelayFirst + time;
                return;
            }
        }
    }
}

void __cdecl CL_GamepadButtonEvent(
                int localClientNum,
                int controllerIndex,
                int key,
                int buttonEvent,
                unsigned int time,
                GamePadButton gamePadButton)
{
    const char *v6; // eax
    int v7; // eax
    int v8; // eax
    const char *v9; // eax
    int v10; // eax
    uiMenuCommand_t ActiveMenu; // [esp+Ch] [ebp-4D0h]
    bool v12; // [esp+74h] [ebp-468h]
    bool v13; // [esp+78h] [ebp-464h]
    bool v14; // [esp+7Ch] [ebp-460h]
    bool v15; // [esp+80h] [ebp-45Ch]
    float Button; // [esp+98h] [ebp-444h]
    int tempControllerIndex; // [esp+9Ch] [ebp-440h]
    float keyDown; // [esp+A0h] [ebp-43Ch]
    int i; // [esp+A4h] [ebp-438h]
    clientUIActive_t *LocalClientUIGlobals; // [esp+B0h] [ebp-42Ch]
    KeyState *keys; // [esp+B4h] [ebp-428h]
    const char *kb; // [esp+B8h] [ebp-424h]
    const char *kba; // [esp+B8h] [ebp-424h]
    connstate_t clcState; // [esp+C0h] [ebp-41Ch]
    LocSelInputState *locSelInputState; // [esp+C8h] [ebp-414h]
    bool pressed; // [esp+CCh] [ebp-410h]
    char cmd[1024]; // [esp+D4h] [ebp-408h] BYREF
    bool devguiToggle; // [esp+4DBh] [ebp-1h]

    pressed = buttonEvent == 1;
    v15 = buttonEvent == 1 || buttonEvent == 2;
    LocalClientUIGlobals = CL_GetLocalClientUIGlobals(localClientNum);
    if ( (LocalClientUIGlobals->keyCatchers & 0x100) == 0 )
    {
        if ( !IsGamepadKey(key) )
        {
            CL_KeyEvent(localClientNum, key, pressed, time);
            return;
        }
        keys = playerKeys[localClientNum].keys;
        keys[key].down = v15;
        clcState = CL_GetLocalClientConnectionState(localClientNum);
        if ( time
            && (clcState == CA_CINEMATIC || clcState == CA_LOGO || clcState == CA_DISCONNECTED || clcState == CA_UICINEMATIC) )
        {
            CL_ResetLastGamePadEventTime();
        }
        if ( buttonEvent == 1 )
        {
            if ( ++keys[key].repeats == 1 )
                ++playerKeys[localClientNum].anyKeyDown;
        }
        else
        {
            keys[key].repeats = 0;
            if ( --playerKeys[localClientNum].anyKeyDown < 0 )
                playerKeys[localClientNum].anyKeyDown = 0;
        }
        kb = keys[key].binding;
        v14 = kb && !I_stricmp(kb, "devgui") && buttonEvent == 1 && keys[key].repeats == 1;
        v13 = keys[15].down && keys[14].down;
        devguiToggle = v13 || v14;
        if ( zombiemode->current.enabled )
        {
            v12 = keys[15].down && keys[14].down;
            devguiToggle = v12;
        }
        if ( devguiToggle )
        {
            if ( DevGui_IsActive() )
            {
                DevGui_Toggle();
            }
            else
            {
                Cbuf_AddText(localClientNum, "devgui\n");
                Cbuf_AddText(localClientNum, "-scores\n");
            }
            return;
        }
        if ( DevGui_IsActive() )
            return;
        if ( !Com_IsMenuLevel(0)
            && v14
            && zombiemode->current.enabled
            && !Menu_IsMenuOpenAndVisible(localClientNum, "menu_friends") )
        {
            Cbuf_AddText(localClientNum, "togglescores\n");
            return;
        }
        locSelInputState = &playerKeys[localClientNum].locSelInputState;
        if ( (LocalClientUIGlobals->keyCatchers & 8) != 0 && v15 )
        {
            if ( key == 2 || keys[key].binding && !strcmp(keys[key].binding, "+actionslot 4") )
            {
                *locSelInputState = LOC_SEL_INPUT_CANCEL;
            }
            else if ( key == 1 || keys[key].binding && !strcmp(keys[key].binding, "+attack") )
            {
                *locSelInputState = LOC_SEL_INPUT_CONFIRM;
            }
            else if ( keys[key].binding && !strcmp(keys[key].binding, "weapnext") )
            {
                *locSelInputState = LOC_SEL_INPUT_REGROUP;
            }
            else if ( keys[key].binding && !strcmp(keys[key].binding, "+usereload") )
            {
                *locSelInputState = LOC_SEL_INPUT_DEFEND;
            }
            else if ( keys[key].binding && !strcmp(keys[key].binding, "togglescores") )
            {
                *locSelInputState = LOC_SEL_INPUT_SQUAD_CANCEL;
            }
        }
        else
        {
            *locSelInputState = LOC_SEL_INPUT_NONE;
            if ( (LocalClientUIGlobals->keyCatchers & 0x10) != 0 )
                ActiveMenu = UI_GetActiveMenu(localClientNum);
            else
                ActiveMenu = UIMENU_NONE;
            if ( clcState == CA_CINEMATIC || clcState == CA_LOGO )
            {
                if ( buttonEvent == 1
                    && (key == 1 || key == 14)
                    && ((LocalClientUIGlobals->keyCatchers & 0x10) == 0 || ActiveMenu == UIMENU_NONE) )
                {
                    CL_StopLogoOrCinematic(localClientNum);
                }
            }
            else if ( !v15 || !CL_CheckForIgnoreDueToRepeat(localClientNum, key, keys[key].repeats, time) || buttonEvent == 2 )
            {
                if ( kb && !I_stricmp(kb, "mainmenu") && buttonEvent == 1 )
                {
                    if ( (LocalClientUIGlobals->keyCatchers & 0x10) != 0 || clcState != CA_ACTIVE )
                        UI_KeyEvent(localClientNum, 15, pressed);
                }
                else
                {
                    Demo_HandleInput(localClientNum, controllerIndex, key, pressed);
                    if ( keys[16].down
                        && keys[17].down
                        && keys[19].down
                        && keys[18].down
                        && controllerIndex != -1
                        && Live_GetTier(controllerIndex) == 1
                        && (Menu_IsMenuOpenAndVisible(localClientNum, "menu_xboxlive")
                         || Menu_IsMenuOpenAndVisible(localClientNum, "menu_xboxlive_lobby")) )
                    {
                        BG_UnlockablesClearAllItemsNew(controllerIndex);
                    }
                    if ( keys[16].down && !keys[17].down )
                    {
                        if ( key == 20 )
                        {
                            if ( buttonEvent == 1 )
                            {
                                v6 = va("exec screenshot.cfg\n");
                                Cbuf_AddText(localClientNum, v6);
                            }
                        }
                        else if ( key == 21 )
                        {
                            if ( buttonEvent == 1 )
                            {
                                v7 = Com_LocalClient_GetControllerIndex(localClientNum);
                                Cmd_ExecuteSingleCommand(localClientNum, v7, (char *)"noclip");
                            }
                        }
                        else if ( key == 23 && buttonEvent == 1 )
                        {
                            v8 = Com_LocalClient_GetControllerIndex(localClientNum);
                            Cmd_ExecuteSingleCommand(localClientNum, v8, (char*)"notarget");
                        }
                    }
                    if ( !net_showprofile->current.integer || !com_sv_running->current.enabled || !CG_IsShowingZombieMap() )
                    {
                        if ( ActiveMenu == UIMENU_SCOREBOARD )
                        {
                            if ( buttonEvent == 1 && Scoreboard_HandleInput(localClientNum, key) )
                                return;
                            if ( CG_GetLocalClientGlobals(localClientNum)->nextSnap->ps.pm_type == 5 )
                            {
                                UI_KeyEvent(localClientNum, key, v15);
                                return;
                            }
                        }
                        if ( (ActiveMenu != UIMENU_GAMERCARD || buttonEvent != 1 || !MuteError_HandleInput(localClientNum))
                            && (ActiveMenu != UIMENU_MUTEERROR || buttonEvent != 1 || !MuteError_HandleInput(localClientNum)) )
                        {
                            for ( i = 0; i < 2; ++i )
                            {
                                if ( i )
                                    kba = keys[key].binding2;
                                else
                                    kba = keys[key].binding;
                                if ( v15 )
                                {
                                    if ( (LocalClientUIGlobals->keyCatchers & 0x50) == 0 || ActiveMenu == UIMENU_SCOREBOARD )
                                    {
                                        if ( clcState && clcState != CA_UICINEMATIC )
                                        {
                                            if ( kba )
                                            {
                                                if ( CG_TranslateGamepadButton(localClientNum, key, kba) )
                                                {
                                                    if ( i == 1 )
                                                        return;
                                                }
                                                else if ( *kba == 43 )
                                                {
                                                    Button = 0.0f;
                                                    if ( gamePadButton )
                                                    {
                                                        v10 = Com_LocalClient_GetControllerIndex(localClientNum);
                                                        Button = GPad_GetButton(v10, gamePadButton);
                                                    }
                                                    Com_sprintf(cmd, 0x400u, "%s %i %i %0.3f\n", kba, key, time, Button);
                                                    Cbuf_AddText(localClientNum, cmd);
                                                }
                                                else
                                                {
                                                    I_stricmp(kba, "devgui");
                                                    Cbuf_AddText(localClientNum, kba);
                                                    Cbuf_AddText(localClientNum, "\n");
                                                }
                                            }
                                            else if ( key >= 200 )
                                            {
                                                v9 = Key_KeynumToString(key, 0);
                                                Com_Printf(14, "%s is unbound, use controls menu to set.\n", v9);
                                            }
                                        }
                                    }
                                    else if ( cls.uiStarted && !i )
                                    {
                                        //BLOPS_NULLSUB();
                                        UI_KeyEvent(localClientNum, key, v15);
                                    }
                                }
                                else
                                {
                                    if ( kba && *kba == 43 )
                                    {
                                        keyDown = 0.0f;
                                        if ( gamePadButton )
                                        {
                                            tempControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
                                            if ( tempControllerIndex == -1 )
                                                tempControllerIndex = controllerIndex;
                                            keyDown = GPad_GetButton(tempControllerIndex, gamePadButton);
                                        }
                                        Com_sprintf(cmd, 0x400u, "-%s %i %i %0.3f\n", kba + 1, key, time, keyDown);
                                        Cbuf_AddText(localClientNum, cmd);
                                    }
                                    if ( (LocalClientUIGlobals->keyCatchers & 0x10) != 0 && cls.uiStarted && !i )
                                        UI_KeyEvent(localClientNum, key, 0);
                                    if ( i == 1 )
                                        return;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

bool __cdecl CL_CheckForIgnoreDueToRepeat(int localClientNum, int key, int repeatCount, int time)
{
    clientUIActive_t *LocalClientUIGlobals; // [esp+0h] [ebp-14h]
    int scrollDelayRest; // [esp+4h] [ebp-10h]
    int buttonIndex; // [esp+Ch] [ebp-8h]
    int scrollDelayFirst; // [esp+10h] [ebp-4h]

    LocalClientUIGlobals = CL_GetLocalClientUIGlobals(localClientNum);
    if ( (LocalClientUIGlobals->keyCatchers & 0x10) != 0 )
    {
        if ( (LocalClientUIGlobals->keyCatchers & 0x80) != 0 )
        {
            scrollDelayFirst = Dvar_GetInt("emblem_scroll_delay_first");
            scrollDelayRest = Dvar_GetInt("emblem_scroll_delay_rest");
        }
        else
        {
            scrollDelayFirst = Dvar_GetInt("gpad_menu_scroll_delay_first");
            scrollDelayRest = Dvar_GetInt("gpad_menu_scroll_delay_rest");
        }
        for ( buttonIndex = 0; buttonIndex < 8; ++buttonIndex )
        {
            if ( key == menuScrollButtonList[buttonIndex] )
            {
                if ( repeatCount == 1 )
                {
                    LocalClientUIGlobals->nextScrollTime = scrollDelayFirst + time;
                    return 0;
                }
                if ( time > LocalClientUIGlobals->nextScrollTime )
                {
                    LocalClientUIGlobals->nextScrollTime = scrollDelayRest + time;
                    return 0;
                }
            }
        }
    }
    return repeatCount > 1;
}

char __cdecl IsGamepadKey(int key)
{
    switch ( key )
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 14:
        case 15:
            return 1;
        case 5:
        case 6:
        case 16:
        case 17:
        case 18:
        case 19:
            return 1;
        case 20:
        case 21:
        case 22:
        case 23:
            return 1;
        case 28:
        case 29:
        case 30:
        case 31:
            return 1;
    }
    return 0;
}

void __cdecl CL_GamepadButtonEventForPort(
                int portIndex,
                int key,
                int buttonEvent,
                unsigned int time,
                GamePadButton button)
{
    int localClientNum; // [esp+0h] [ebp-4h]

    localClientNum = Com_ControllerIndex_GetLocalClientNum(portIndex);
    if ( localClientNum == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_gamepad.cpp",
                    1478,
                    0,
                    "%s",
                    "localClientNum != INVALID_CLIENT_NUMBER") )
    {
        __debugbreak();
    }
    if ( Key_IsCatcherActive(localClientNum, 16) )
        CL_GamepadResetMenuScrollTime(localClientNum, key, buttonEvent == 1, time);
    CL_GamepadButtonEvent(localClientNum, portIndex, key, buttonEvent, time, button);
}

