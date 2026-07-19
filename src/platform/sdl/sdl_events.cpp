// sdl_events.cpp — SDL → engine input translation.
#include "sdl_events.h"

#include <SDL2/SDL.h>
#include <ui/keycodes.h>      // keyNum_t (the engine's key codes)
#include <win32/win_local.h>  // sysEventType_t (SE_KEY / SE_CHAR / ...)

// Provided by the engine (src/win32/win_main.cpp). On x86-32 __cdecl is the default
// convention, so this declaration resolves to the same symbol.
void Sys_QueEvent(unsigned int time, sysEventType_t type, int value, int value2,
                  int ptrLength, void *ptr);

int Sys_SDLKeyToKeyNum(int sym) {
    // Printable ASCII maps straight through (Quake convention: letter/number/symbol
    // keys use their ASCII value). SDL reports unshifted lowercase here.
    if (sym >= 0x20 && sym < 0x7F) return sym;

    switch (sym) {
        case SDLK_RETURN: case SDLK_KP_ENTER: return K_ENTER;
        case SDLK_ESCAPE:    return K_ESCAPE;
        case SDLK_BACKSPACE: return K_BACKSPACE;
        case SDLK_TAB:       return K_TAB;
        case SDLK_UP:        return K_UPARROW;
        case SDLK_DOWN:      return K_DOWNARROW;
        case SDLK_LEFT:      return K_LEFTARROW;
        case SDLK_RIGHT:     return K_RIGHTARROW;
        case SDLK_LALT:   case SDLK_RALT:   return K_ALT;
        case SDLK_LCTRL:  case SDLK_RCTRL:  return K_CTRL;
        case SDLK_LSHIFT: case SDLK_RSHIFT: return K_SHIFT;
        case SDLK_INSERT:    return K_INS;
        case SDLK_DELETE:    return K_DEL;
        case SDLK_PAGEDOWN:  return K_PGDN;
        case SDLK_PAGEUP:    return K_PGUP;
        case SDLK_HOME:      return K_HOME;
        case SDLK_END:       return K_END;
        case SDLK_CAPSLOCK:  return K_CAPSLOCK;
        default: break;
    }
    if (sym >= SDLK_F1 && sym <= SDLK_F12) return K_F1 + (sym - SDLK_F1);
    return K_NONE;
}

void Sys_PumpSDLEvents(unsigned t) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                int k = Sys_SDLKeyToKeyNum((int)e.key.keysym.sym);
                if (k != K_NONE)
                    Sys_QueEvent(t, SE_KEY, k, e.type == SDL_KEYDOWN ? 1 : 0, 0, nullptr);
                break;
            }
            case SDL_TEXTINPUT:
                for (const char *p = e.text.text; *p; ++p)
                    Sys_QueEvent(t, SE_CHAR, (unsigned char)*p, 0, 0, nullptr);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                int btn = e.button.button - SDL_BUTTON_LEFT;  // 0-based -> K_MOUSE1..
                Sys_QueEvent(t, SE_KEY, K_MOUSE1 + btn, e.type == SDL_MOUSEBUTTONDOWN ? 1 : 0, 0, nullptr);
                break;
            }
            case SDL_MOUSEWHEEL: {
                int k = e.wheel.y > 0 ? K_MWHEELUP : K_MWHEELDOWN;
                Sys_QueEvent(t, SE_KEY, k, 1, 0, nullptr);  // wheel = press + release
                Sys_QueEvent(t, SE_KEY, k, 0, 0, nullptr);
                break;
            }
            default: break;
        }
    }
}
