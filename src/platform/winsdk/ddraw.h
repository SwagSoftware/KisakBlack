// ddraw.h — minimal portable DirectDraw surface for non-Windows builds.
//
// The renderer uses DirectDraw for exactly one thing: querying available video
// memory via IDirectDraw7::GetAvailableVidMem (r_texturemem.cpp). That code loads
// ddraw.dll dynamically (LoadLibraryA) and degrades gracefully when it is absent —
// on Linux LoadLibraryA("ddraw.dll") returns null, so the query is skipped and the
// renderer falls back to its default VRAM estimate. Only the referenced surface is
// declared here.
#ifndef KISAK_DDRAW_H
#define KISAK_DDRAW_H

#include "windows.h"

typedef struct _DDSCAPS2 {
    DWORD dwCaps, dwCaps2, dwCaps3, dwCaps4;
} DDSCAPS2, *LPDDSCAPS2;

struct IDirectDraw7 : public IUnknown {
    virtual HRESULT WINAPI SetCooperativeLevel(HWND hWnd, DWORD dwFlags) = 0;
    virtual HRESULT WINAPI GetAvailableVidMem(DDSCAPS2 *lpDDSCaps2, DWORD *lpdwTotal, DWORD *lpdwFree) = 0;
};
typedef IDirectDraw7 *LPDIRECTDRAW7;

static const GUID IID_IDirectDraw7 = { 0x15e65ec0, 0x3b9c, 0x11d2, { 0xb9, 0x2f, 0x00, 0x60, 0x97, 0x97, 0xea, 0x5b } };

#endif // KISAK_DDRAW_H
