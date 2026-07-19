#pragma once

enum GamepadPhysicalAxis : __int32
{                                       // XREF: GamepadVirtualAxisMapping/r
                                        // StickToCodeMap/r ...
    GPAD_PHYSAXIS_RSTICK_X = 0x0,       // XREF: .data:axisSameStick/s
                                        // .data:StickToCodeMap * menuLeftyScrollAnalogList/s
    GPAD_PHYSAXIS_RSTICK_Y = 0x1,       // XREF: .data:axisSameStick/s
                                        // .data:StickToCodeMap * menuLeftyScrollAnalogList/s
    GPAD_PHYSAXIS_LSTICK_X = 0x2,       // XREF: .data:axisSameStick/s
                                        // .data:StickToCodeMap * menuScrollAnalogList/s
    GPAD_PHYSAXIS_LSTICK_Y = 0x3,       // XREF: .data:axisSameStick/s
                                        // .data:StickToCodeMap * menuScrollAnalogList/s
    GPAD_PHYSAXIS_RTRIGGER = 0x4,
    GPAD_PHYSAXIS_LTRIGGER = 0x5,
    GPAD_PHYSAXIS_COUNT    = 0x6,
    GPAD_PHYSAXIS_NONE     = -1, // XREF: .data:axisSameStick/s
};

enum GamePadStickDir : __int32
{                                       // XREF: StickToCodeMap_t/r
                                        // ?GPad_IsStickPressed@@YA_NHW4GamePadStick@@W4GamePadStickDir@@@Z/r ...
    GPAD_STICK_POS      = 0x0,          // XREF: .data:StickToCodeMap_t * analogStickList/s
    GPAD_STICK_NEG      = 0x1,          // XREF: .data:StickToCodeMap_t * analogStickList/s
    GPAD_STICK_DIRCOUNT = 0x2,
};

enum GamepadVirtualAxis : __int32
{                                       // XREF: Axis_Bind_f/r
                                        // Gamepad_BindAxis/r ...
    GPAD_VIRTAXIS_SIDE    = 0x0,
    GPAD_VIRTAXIS_FORWARD = 0x1,
    GPAD_VIRTAXIS_UP      = 0x2,
    GPAD_VIRTAXIS_YAW     = 0x3,
    GPAD_VIRTAXIS_PITCH   = 0x4,
    GPAD_VIRTAXIS_ATTACK  = 0x5,
    GPAD_VIRTAXIS_COUNT   = 0x6,
    GPAD_VIRTAXIS_NONE    = -1,
};
inline GamepadVirtualAxis &operator++(GamepadVirtualAxis &t)
{
    t = static_cast<GamepadVirtualAxis>((static_cast<int>(t) + 1));
    return t;
}
inline GamepadVirtualAxis operator++(GamepadVirtualAxis &t, int)
{
    GamepadVirtualAxis old = t;
    t = static_cast<GamepadVirtualAxis>((static_cast<int>(t) + 1));
    return old;
}

enum GamepadMapping : __int32
{                                       // XREF: GamepadVirtualAxisMapping/r
                                        // Axis_Bind_f/r ...
    GPAD_MAP_LINEAR  = 0x0,
    GPAD_MAP_SQUARED = 0x1,
    GPAD_MAP_COUNT   = 0x2,
    GPAD_MAP_NONE    = -1,
};


enum GamePadButton : __int32
{                                       // XREF: ButtonToCodeMap_t/r
    GPAD_NONE    = 0x0,
    GPAD_UP      = 0x10000001,          // XREF: .data:s_butMapsGamepad/s
    GPAD_DOWN    = 0x10000002,          // XREF: .data:s_butMapsGamepad/s
    GPAD_LEFT    = 0x10000004,          // XREF: .data:s_butMapsGamepad/s
    GPAD_RIGHT   = 0x10000008,          // XREF: .data:s_butMapsGamepad/s
    GPAD_START   = 0x10000010,          // XREF: .data:s_butMapsGamepad/s
    GPAD_BACK    = 0x10000020,
    GPAD_L3      = 0x10000040,
    GPAD_R3      = 0x10000080,
    GPAD_A       = 0x10001000,          // XREF: .data:s_butMapsGamepad/s
    GPAD_B       = 0x10002000,          // XREF: .data:s_butMapsGamepad/s
    GPAD_X       = 0x10004000,          // XREF: .data:s_butMapsGamepad/s
                                        // .data:ButtonToCodeMap_t * buttonList/s
    GPAD_Y       = 0x10008000,          // XREF: .data:s_butMapsGamepad/s
    GPAD_L_SHLDR = 0x10000100,          // XREF: .data:s_butMapsGamepad/s
    GPAD_R_SHLDR = 0x10000200,          // XREF: .data:s_butMapsGamepad/s
    GPAD_L_TRIG  = 0x20000000,
    GPAD_R_TRIG  = 0x20000001,
};

enum GamePadStick : __int32
{                                       // XREF: StickToCodeMap_t/r
                                        // ?GPad_GetStick@@YAMHW4GamePadStick@@@Z/r ...
    GPAD_INVALID = 0x0,
    GPAD_LX      = 0x40000000,          // XREF: .data:StickToCodeMap_t * analogStickList/s
    GPAD_LY      = 0x40000001,          // XREF: .data:StickToCodeMap_t * analogStickList/s
    GPAD_RX      = 0x40000002,
    GPAD_RY      = 0x40000003,
};

struct GamepadVirtualAxisMapping // sizeof=0x8
{                                       // XREF: GpadAxesGlob/r
    GamepadPhysicalAxis physicalAxis;   // XREF: CL_InitGamepadAxisBindings(void)+3F/w
                                        // Axis_Unbindall_f+2E/w
    GamepadMapping mapType;
};

struct GpadAxesGlob // sizeof=0x48
{                                       // XREF: .data:gaGlobs/r
    int axesValues[6];                  // XREF: CL_InitGamepadAxisBindings(void)+67/w
    GamepadVirtualAxisMapping virtualAxes[6];
                                        // XREF: CL_InitGamepadAxisBindings(void)+3F/w
                                        // Axis_Unbindall_f+2E/w
};

void __cdecl CL_ResetLastGamePadEventTime();
void __cdecl CL_InitGamepadCommands();
void __cdecl Bind_GP_SticksConfigs_f();
void __cdecl Bind_GP_ButtonsConfigs_f();
void __cdecl CL_InitGamepadAxisBindings();
void __cdecl Axis_Bind_f();
GamepadPhysicalAxis __cdecl Gamepad_StringToPhysicalAxis(const char *axisName);
GamepadVirtualAxis __cdecl Axis_StringToVirtualAxis(const char *axisName);
GamepadMapping __cdecl Gamepad_InputTypeStringToId(const char *name);
void __cdecl Gamepad_BindAxis(
                GpadAxesGlob *gaGlob,
                GamepadPhysicalAxis realIndex,
                GamepadVirtualAxis axisIndex,
                GamepadMapping mapType);
void __cdecl Axis_Unbindall_f();
void __cdecl Gamepad_WriteBindings(int localClientNum, int f);
const char *__cdecl Axis_NumToAxisString(GamepadPhysicalAxis axis);
const char *__cdecl Gamepad_VirtualAxisName(GamepadVirtualAxis axis);
const char *__cdecl Gamepad_InputTypeName(GamepadMapping mapType);
void __cdecl CL_GamepadEvent(int portIndex, unsigned int physicalAxis, int value);
void __cdecl CL_GamepadGenerateAPad(int localClientNum, int portIndex, unsigned int physicalAxis);
double __cdecl CL_GamepadAxisValue(int localClientNum, unsigned int virtualAxis);
void __cdecl CL_GamepadResetMenuScrollTime(int localClientNum, int key, int down, unsigned int time);
void __cdecl CL_GamepadButtonEvent(
                int localClientNum,
                int controllerIndex,
                int key,
                int buttonEvent,
                unsigned int time,
                GamePadButton gamePadButton);
bool __cdecl CL_CheckForIgnoreDueToRepeat(int localClientNum, int key, int repeatCount, int time);
char __cdecl IsGamepadKey(int key);
void __cdecl CL_GamepadButtonEventForPort(
                int portIndex,
                int key,
                int buttonEvent,
                unsigned int time,
                GamePadButton button);
