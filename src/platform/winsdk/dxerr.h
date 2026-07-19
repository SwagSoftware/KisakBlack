// dxerr.h — portable stand-in for the legacy DirectX error-string helper.
//
// The renderer uses only DXGetErrorDescriptionA, to turn a failing HRESULT into a
// human-readable string for a log line (r_init.cpp). A generic description keeps
// the diagnostic readable without pulling in the deprecated DXERR SDK component.
#ifndef KISAK_DXERR_H
#define KISAK_DXERR_H

#include "_kisak_wintypes.h"

static inline const char *DXGetErrorStringA(HRESULT hr)      { (void)hr; return "DXERR"; }
static inline const char *DXGetErrorDescriptionA(HRESULT hr) { (void)hr; return "DirectX error"; }

#endif // KISAK_DXERR_H
