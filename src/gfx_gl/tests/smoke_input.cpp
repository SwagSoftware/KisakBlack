// smoke_input.cpp — SDL → engine event translation (Linux input pump).
//
// Unit-checks the pure key mapping, then pushes synthetic SDL events through
// Sys_PumpSDLEvents and confirms they arrive as the right Sys_QueEvent calls. A
// mock Sys_QueEvent (matching the engine's signature) captures the dispatched
// events in place of the real engine queue.
#include <SDL2/SDL.h>
#include <ui/keycodes.h>
#include <win32/win_local.h>
#include <platform/sdl/sdl_events.h>
#include <cstdio>
#include <vector>

// From src/platform/sdl/sdl_system.cpp.
unsigned int Sys_Milliseconds();

struct Ev { sysEventType_t type; int value; int value2; };
static std::vector<Ev> g_events;

// Mock of the engine's Sys_QueEvent (src/win32/win_main.cpp).
void Sys_QueEvent(unsigned int, sysEventType_t type, int value, int value2, int, void *) {
    g_events.push_back({type, value, value2});
}

static void pushKey(SDL_Keycode sym, bool down) {
    SDL_Event e{};
    e.type = down ? SDL_KEYDOWN : SDL_KEYUP;
    e.key.keysym.sym = sym;
    SDL_PushEvent(&e);
}
static void pushMouse(Uint8 button, bool down) {
    SDL_Event e{};
    e.type = down ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
    e.button.button = button;
    SDL_PushEvent(&e);
}

int main() {
    int fails = 0;
    auto check = [&](const char *what, bool ok) { if (!ok) { printf("  FAIL: %s\n", what); ++fails; } };

    // 1) Pure key mapping.
    check("'a' -> ascii",      Sys_SDLKeyToKeyNum('a') == 'a');
    check("ESCAPE -> K_ESCAPE", Sys_SDLKeyToKeyNum(SDLK_ESCAPE) == K_ESCAPE);
    check("RETURN -> K_ENTER",  Sys_SDLKeyToKeyNum(SDLK_RETURN) == K_ENTER);
    check("UP -> K_UPARROW",    Sys_SDLKeyToKeyNum(SDLK_UP) == K_UPARROW);
    check("F1 -> K_F1",         Sys_SDLKeyToKeyNum(SDLK_F1) == K_F1);
    check("F12 -> K_F12",       Sys_SDLKeyToKeyNum(SDLK_F12) == K_F12);
    check("SHIFT -> K_SHIFT",   Sys_SDLKeyToKeyNum(SDLK_LSHIFT) == K_SHIFT);
    check("unmapped -> K_NONE", Sys_SDLKeyToKeyNum(SDLK_SCROLLLOCK) == K_NONE);

    // 2) Event pump dispatch.
    if (SDL_Init(SDL_INIT_VIDEO) != 0) { printf("FAIL: SDL_Init: %s\n", SDL_GetError()); return 1; }
    SDL_PumpEvents(); SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
    g_events.clear();

    pushKey(SDLK_ESCAPE, true);
    pushKey(SDLK_ESCAPE, false);
    pushMouse(SDL_BUTTON_LEFT, true);
    Sys_PumpSDLEvents(123);

    check("3 events dispatched", g_events.size() == 3);
    if (g_events.size() == 3) {
        check("esc down", g_events[0].type == SE_KEY && g_events[0].value == K_ESCAPE && g_events[0].value2 == 1);
        check("esc up",   g_events[1].type == SE_KEY && g_events[1].value == K_ESCAPE && g_events[1].value2 == 0);
        check("mouse1",   g_events[2].type == SE_KEY && g_events[2].value == K_MOUSE1 && g_events[2].value2 == 1);
    }
    // 3) Sys_Milliseconds (SDL timer) advances.
    unsigned t0 = Sys_Milliseconds();
    SDL_Delay(5);
    unsigned t1 = Sys_Milliseconds();
    check("Sys_Milliseconds advances", t1 >= t0 && (t1 - t0) < 1000);

    SDL_Quit();

    printf(fails == 0 ? "INPUT: PASS\n" : "INPUT: FAIL (%d)\n", fails);
    return fails == 0 ? 0 : 1;
}
