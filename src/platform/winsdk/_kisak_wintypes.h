// _kisak_wintypes.h — minimal Win32 base types + COM plumbing for non-Windows builds.
//
// This dir (src/platform/winsdk) is placed on the include path ONLY on non-Windows,
// so the game's `#include <windows.h>` / `<d3d9.h>` resolve to our portable headers.
// On Windows the real Platform SDK is used and nothing here is compiled.
//
// We only declare what the KisakBlack renderer actually touches — not the whole Win32
// API. The goal is source-compatibility (same type/method names + signatures), NOT ABI
// compatibility with a real DLL: our D3D9 interfaces are backed by our own GL code, so
// vtable layout only has to be self-consistent.
#ifndef KISAK_WINTYPES_H
#define KISAK_WINTYPES_H

#if defined(_WIN32)
#error "_kisak_wintypes.h is a non-Windows shim; it must not be on the include path on Windows."
#endif

#include <stdint.h>
#include <stddef.h>

// ---- Integer / scalar typedefs --------------------------------------------
typedef unsigned char       BYTE,  *LPBYTE;
typedef unsigned char       UCHAR;
typedef unsigned short      WORD,  *LPWORD;
typedef unsigned short      USHORT;
typedef short               SHORT;
typedef unsigned int        UINT,  *LPUINT;
typedef int                 INT,   *LPINT, WINBOOL, BOOL;
typedef unsigned int        DWORD, *LPDWORD;
typedef int                 LONG,  *LPLONG;
typedef unsigned int        ULONG, *PULONG;
typedef float               FLOAT, *LPFLOAT;
typedef char                CHAR,  *PCHAR;
typedef wchar_t             WCHAR, *LPWSTR;
typedef int64_t             LONGLONG, INT64, LONG64;
typedef uint64_t            ULONGLONG, UINT64, DWORDLONG, ULONG64;
typedef union _LARGE_INTEGER {
    struct { DWORD LowPart; LONG HighPart; };
    LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;
typedef int8_t              INT8;
typedef uint8_t             UINT8;
typedef int16_t             INT16;
typedef int32_t             INT32;
typedef uint32_t            UINT32;

// Pointer-width integers (correct for both 32- and 64-bit builds).
typedef intptr_t            INT_PTR,  LONG_PTR;
typedef uintptr_t           UINT_PTR, ULONG_PTR, DWORD_PTR;
typedef size_t              SIZE_T;

typedef void                VOID;
typedef void               *LPVOID, *PVOID, *HANDLE;
typedef const void         *LPCVOID;
typedef char               *LPSTR;
typedef const char         *LPCSTR, *PCSTR;
typedef const wchar_t      *LPCWSTR;

#ifndef CONST
#define CONST const
#endif
#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

// ---- Calling-convention / interface macros --------------------------------
// We are not matching the real DirectX ABI, so these become no-ops; methods use
// the platform default convention, which is fine because both caller and callee
// share these headers.
#define WINAPI
#define APIENTRY
#define CALLBACK
#define STDMETHODCALLTYPE
#define STDAPICALLTYPE
#define FAR
#define WINAPIV

// COM interface-declaration macros (from <objbase.h>): the decompiled COM-style
// interfaces (e.g. the XAudio2 voice callback) are written with these.
#define STDMETHOD(method)        virtual HRESULT method
#define STDMETHOD_(type, method) virtual type method
#define THIS_
#define THIS                     void
#ifndef PURE
#define PURE                     = 0
#endif
#ifndef __cdecl
#define __cdecl
#endif
#ifndef __stdcall
#define __stdcall
#endif

// ---- HRESULT --------------------------------------------------------------
typedef LONG HRESULT;
#define S_OK            ((HRESULT)0L)
#define S_FALSE         ((HRESULT)1L)
#define E_FAIL          ((HRESULT)0x80004005L)
#define E_NOINTERFACE   ((HRESULT)0x80004002L)
#define E_OUTOFMEMORY   ((HRESULT)0x8007000EL)
#define E_INVALIDARG    ((HRESULT)0x80070057L)
#define E_NOTIMPL       ((HRESULT)0x80004001L)
#define E_POINTER       ((HRESULT)0x80004003L)
#define SUCCEEDED(hr)   (((HRESULT)(hr)) >= 0)
#define FAILED(hr)      (((HRESULT)(hr)) <  0)

// ---- Opaque handle types --------------------------------------------------
#define KISAK_DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
KISAK_DECLARE_HANDLE(HWND);
KISAK_DECLARE_HANDLE(HMONITOR);
KISAK_DECLARE_HANDLE(HDC);
KISAK_DECLARE_HANDLE(HFONT);
KISAK_DECLARE_HANDLE(HINSTANCE);
typedef HINSTANCE HMODULE;

// ---- Simple geometry structs ----------------------------------------------
typedef struct tagRECT  { LONG left, top, right, bottom; } RECT, *LPRECT;
typedef struct tagPOINT { LONG x, y; }                     POINT, *LPPOINT;
typedef struct tagSIZE  { LONG cx, cy; }                   SIZE;

// ---- GUID / COM -----------------------------------------------------------
typedef struct _GUID {
    DWORD Data1;
    WORD  Data2;
    WORD  Data3;
    BYTE  Data4[8];
} GUID, IID, CLSID;
typedef const GUID &REFGUID;
typedef const IID  &REFIID;
typedef const CLSID &REFCLSID;

// The engine compares GUIDs/IIDs with == (e.g. in QueryInterface). MSVC provides
// operator== for GUID via a header; supply it here (byte-compare).
#ifdef __cplusplus
inline bool operator==(const GUID &a, const GUID &b) {
    return a.Data1 == b.Data1 && a.Data2 == b.Data2 && a.Data3 == b.Data3 &&
           a.Data4[0] == b.Data4[0] && a.Data4[1] == b.Data4[1] &&
           a.Data4[2] == b.Data4[2] && a.Data4[3] == b.Data4[3] &&
           a.Data4[4] == b.Data4[4] && a.Data4[5] == b.Data4[5] &&
           a.Data4[6] == b.Data4[6] && a.Data4[7] == b.Data4[7];
}
inline bool operator!=(const GUID &a, const GUID &b) { return !(a == b); }
#endif

// Abstract COM root. All D3D9 interfaces derive from this.
struct IUnknown {
    virtual HRESULT WINAPI QueryInterface(REFIID riid, void **ppv) = 0;
    virtual ULONG   WINAPI AddRef() = 0;
    virtual ULONG   WINAPI Release() = 0;
};
typedef IUnknown *LPUNKNOWN;

// ---- FourCC ---------------------------------------------------------------
#ifndef MAKEFOURCC
#define MAKEFOURCC(a, b, c, d) \
    ((DWORD)(BYTE)(a) | ((DWORD)(BYTE)(b) << 8) | \
     ((DWORD)(BYTE)(c) << 16) | ((DWORD)(BYTE)(d) << 24))
#endif

#endif // KISAK_WINTYPES_H
