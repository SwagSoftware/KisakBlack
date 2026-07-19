// sdl_window.cpp — SDL implementations of the Win32 window/monitor APIs the
// renderer calls (declared in platform/winsdk/windows.h). The Linux build compiles
// this instead of the Win32 window code in src/win32.
//
// Window ownership: the GL backend (src/gfx_gl) creates and owns the single SDL
// window in CreateDevice. So the renderer's window-*manipulation* calls here are
// sentinels / no-ops — the SDL window is authoritative and there is nothing for the
// renderer to position or restyle. The *display* queries (screen metrics, monitor
// rectangle, monitor enumeration) return real SDL data so the renderer's resolution
// and fullscreen-rect logic gets correct numbers. The HWND/HMONITOR values the
// renderer threads around are opaque non-null sentinels onto display 0.
#include <SDL2/SDL.h>
#include <windows.h>

namespace {
// Non-null opaque sentinels. The renderer only ever tests these for non-null and
// hands them straight back to APIs below — it never dereferences them.
HWND     kSentinelWindow  = (HWND)(intptr_t)1;
HMONITOR kSentinelMonitor = (HMONITOR)(intptr_t)1;
HMODULE  kSentinelModule  = (HMODULE)(intptr_t)1;

void EnsureVideo() {
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0) SDL_InitSubSystem(SDL_INIT_VIDEO);
}

// Bounds of display 0 (the GL window's display), with a 1080p fallback if SDL video
// is unavailable (e.g. headless) so callers still get a sane rectangle.
void Display0Bounds(RECT *out) {
    SDL_Rect r;
    EnsureVideo();
    if (SDL_WasInit(SDL_INIT_VIDEO) && SDL_GetDisplayBounds(0, &r) == 0) {
        out->left = r.x; out->top = r.y; out->right = r.x + r.w; out->bottom = r.y + r.h;
    } else {
        out->left = 0; out->top = 0; out->right = 1920; out->bottom = 1080;
    }
}
} // namespace

// ---- Display metrics / monitors --------------------------------------------
int GetSystemMetrics(int nIndex) {
    RECT r; Display0Bounds(&r);
    if (nIndex == 0) return r.right - r.left;   // SM_CXSCREEN
    if (nIndex == 1) return r.bottom - r.top;   // SM_CYSCREEN
    return 0;
}

HMONITOR MonitorFromPoint(POINT, DWORD)  { return kSentinelMonitor; }
HMONITOR MonitorFromWindow(HWND, DWORD)  { return kSentinelMonitor; }

BOOL GetMonitorInfoA(HMONITOR, LPMONITORINFO lpmi) {
    if (!lpmi) return FALSE;
    Display0Bounds(&lpmi->rcMonitor);
    lpmi->rcWork = lpmi->rcMonitor;             // no taskbar inset modeled
    lpmi->dwFlags = 1;                          // MONITORINFOF_PRIMARY
    return TRUE;
}

BOOL EnumDisplayMonitors(HDC, LPRECT, MONITORENUMPROC lpfnEnum, LPARAM dwData) {
    if (!lpfnEnum) return FALSE;
    RECT r; Display0Bounds(&r);
    return lpfnEnum(kSentinelMonitor, (HDC)0, &r, dwData);  // single (primary) monitor
}

BOOL ClientToScreen(HWND, LPPOINT) { return TRUE; }  // SDL window client == screen origin here

// ---- Window manipulation (SDL window is authoritative -> no-ops) ------------
BOOL AdjustWindowRectEx(LPRECT, DWORD, BOOL, DWORD) { return TRUE; }  // SDL owns decorations
HWND CreateWindowExA(DWORD, const char *, const char *, DWORD, int, int, int, int,
                     HWND, HMENU, HINSTANCE, void *) { return kSentinelWindow; }
BOOL DestroyWindow(HWND)              { return TRUE; }
BOOL IsWindow(HWND hWnd)             { return hWnd == kSentinelWindow; }
BOOL ShowWindow(HWND, int)            { return TRUE; }
BOOL SetWindowPos(HWND, HWND, int, int, int, int, UINT) { return TRUE; }
LONG SetWindowLongA(HWND, int, LONG)  { return 0; }
HWND SetFocus(HWND)                   { return kSentinelWindow; }
HMODULE GetModuleHandleA(const char *) { return kSentinelModule; }
