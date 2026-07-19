// sdl_events.h — SDL → engine input translation (Linux/macOS message pump).
//
// The Linux/SDL replacement for the Win32 message pump in src/win32: it polls the
// SDL event queue and feeds the engine's Sys_QueEvent() the same way win_wndproc /
// win_input do on Windows. The pure key mapping is exposed so it can be unit-tested
// without a window.
#ifndef KISAK_SDL_EVENTS_H
#define KISAK_SDL_EVENTS_H

// Map an SDL_Keycode (passed as int to keep SDL out of this header) to the engine's
// keyNum_t value, or K_NONE (0) if unmapped.
int Sys_SDLKeyToKeyNum(int sdlKeycode);

// Drain the SDL event queue, dispatching key/char/mouse events via Sys_QueEvent.
void Sys_PumpSDLEvents(unsigned timeMs);

#endif // KISAK_SDL_EVENTS_H
