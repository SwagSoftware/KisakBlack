// wtypes.h — portable stand-in for the Win32 base-type header.
//
// On Windows <wtypes.h> defines the fundamental COM/Win32 scalar types (DWORD,
// HRESULT, WCHAR, BSTR, ...). Everything the tree references is already provided by
// our portable base-types header, so forward to it.
#ifndef KISAK_WTYPES_H
#define KISAK_WTYPES_H

#include "_kisak_wintypes.h"

#endif // KISAK_WTYPES_H
