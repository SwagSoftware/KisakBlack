// XInput.h — minimal portable stand-in for the Xbox controller API.
//
// The gamepad code includes <XInput.h> but the decompiled call sites reference very
// little of it directly. Declare the standard structs/constants and have the query
// functions report "no controller", so the input layer compiles and degrades
// gracefully; real gamepad support will come through SDL's game-controller API.
#ifndef KISAK_XINPUT_H
#define KISAK_XINPUT_H

#include "windows.h"

typedef struct _XINPUT_GAMEPAD {
    WORD  wButtons;
    BYTE  bLeftTrigger, bRightTrigger;
    SHORT sThumbLX, sThumbLY, sThumbRX, sThumbRY;
} XINPUT_GAMEPAD, *PXINPUT_GAMEPAD;

typedef struct _XINPUT_STATE {
    DWORD          dwPacketNumber;
    XINPUT_GAMEPAD Gamepad;
} XINPUT_STATE, *PXINPUT_STATE;

typedef struct _XINPUT_VIBRATION {
    WORD wLeftMotorSpeed, wRightMotorSpeed;
} XINPUT_VIBRATION, *PXINPUT_VIBRATION;

typedef struct _XINPUT_CAPABILITIES {
    BYTE Type, SubType;
    WORD Flags;
    XINPUT_GAMEPAD   Gamepad;
    XINPUT_VIBRATION Vibration;
} XINPUT_CAPABILITIES, *PXINPUT_CAPABILITIES;

#ifndef ERROR_SUCCESS
#define ERROR_SUCCESS 0
#endif
#ifndef ERROR_DEVICE_NOT_CONNECTED
#define ERROR_DEVICE_NOT_CONNECTED 1167
#endif

#define XINPUT_GAMEPAD_DPAD_UP        0x0001
#define XINPUT_GAMEPAD_DPAD_DOWN      0x0002
#define XINPUT_GAMEPAD_DPAD_LEFT      0x0004
#define XINPUT_GAMEPAD_DPAD_RIGHT     0x0008
#define XINPUT_GAMEPAD_START          0x0010
#define XINPUT_GAMEPAD_BACK           0x0020
#define XINPUT_GAMEPAD_LEFT_THUMB     0x0040
#define XINPUT_GAMEPAD_RIGHT_THUMB    0x0080
#define XINPUT_GAMEPAD_LEFT_SHOULDER  0x0100
#define XINPUT_GAMEPAD_RIGHT_SHOULDER 0x0200
#define XINPUT_GAMEPAD_A              0x1000
#define XINPUT_GAMEPAD_B              0x2000
#define XINPUT_GAMEPAD_X              0x4000
#define XINPUT_GAMEPAD_Y              0x8000

static inline DWORD XInputGetState(DWORD, XINPUT_STATE *)              { return ERROR_DEVICE_NOT_CONNECTED; }
static inline DWORD XInputSetState(DWORD, XINPUT_VIBRATION *)          { return ERROR_DEVICE_NOT_CONNECTED; }
static inline DWORD XInputGetCapabilities(DWORD, DWORD, XINPUT_CAPABILITIES *) { return ERROR_DEVICE_NOT_CONNECTED; }
static inline void  XInputEnable(BOOL) {}

#endif // KISAK_XINPUT_H
