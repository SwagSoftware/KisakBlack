// sdl_system.cpp — SDL implementations of the Sys_* platform utilities that
// src/win32/win_shared.cpp provides on Windows. The Linux build compiles this
// instead of the Win32 versions. Signatures match the engine's declarations
// (e.g. win_shared.h) so call sites link unchanged.
#include <SDL2/SDL.h>

// Milliseconds since startup. Matches `unsigned int __cdecl Sys_Milliseconds()`
// (win_shared.cpp); on x86-32 __cdecl is the default convention.
unsigned int Sys_Milliseconds() {
    if (SDL_WasInit(SDL_INIT_TIMER) == 0) SDL_InitSubSystem(SDL_INIT_TIMER);
    return (unsigned int)SDL_GetTicks();
}

// Yield the CPU for `msec` milliseconds (engine's Sys_Sleep / NET_Sleep helper).
void Sys_SleepMSec(int msec) {
    if (msec > 0) SDL_Delay((Uint32)msec);
}
