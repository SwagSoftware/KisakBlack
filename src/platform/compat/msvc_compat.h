// msvc_compat.h — MSVC/Hex-Rays keyword compatibility for GCC/Clang builds.
//
// The decompiled sources use a handful of MSVC builtins that GCC/Clang don't
// provide as keywords. This header supplies them on non-MSVC. It is force-included
// (compiler `-include`) ahead of every translation unit on non-Windows so it
// applies before q_shared.h (which already maps __int64/_DWORD/etc. for __GNUC__).
//
// Calling-convention keywords: GCC/Clang do NOT accept the bare __cdecl /
// __stdcall / __fastcall / __thiscall on a non-Windows target (they parse as
// identifiers -> "expected initializer"). The build is 32-bit x86 and internally
// self-consistent (no external symbol relies on a specific convention), so map
// them all to nothing (see below).
#ifndef KISAK_MSVC_COMPAT_H
#define KISAK_MSVC_COMPAT_H

#include "msvc_intrin.h"  // _Interlocked*/MemoryBarrier -> GCC builtins

#if !defined(_MSC_VER)

// MSVC makes the C string/float-limits functions broadly visible; the decompiled
// code uses memset/memcpy/FLT_MAX/etc. without always including the header. Pull
// them in build-wide (this header is force-included) to match. (Not <cstdlib> —
// its POSIX random()/etc. clash with the engine's own declarations.)
#include <cstring>
#include <cfloat>
#include <cstdio>
#include <ctime>
#include <cctype>
#include <csetjmp>
// NOT <strings.h>: it declares index()/rindex(), which collide with the engine's
// `index` variables. Declare the two case-insensitive compares we need directly.
extern "C" int strcasecmp(const char *, const char *) noexcept;
extern "C" int strncasecmp(const char *, const char *, size_t) noexcept;

#ifndef __debugbreak
#define __debugbreak() __builtin_trap()
#endif

// More MSVC-isms: another calling convention, and the _-prefixed CRT aliases.
#ifndef __pascal
#define __pascal
#endif
#ifndef _snprintf
#define _snprintf  snprintf
#define _vsnprintf vsnprintf
#define _stricmp   strcasecmp
#define _strnicmp  strncasecmp
#define _strdup    strdup
#endif
#ifndef sprintf_s
#define sprintf_s(buf, size, ...) snprintf((buf), (size), __VA_ARGS__)
#endif

// MSVC's _-prefixed sized-int CRT conversions -> POSIX/C equivalents. Declared
// directly (NOT via <cstdlib>) so we don't pull in POSIX random()/srandom(), which
// collide with the engine's own random() declaration (only scope-renamed in a few
// files). These libc symbols are otherwise standard.
extern "C" long long          atoll(const char *) noexcept;
extern "C" long long          strtoll(const char *, char **, int) noexcept;
extern "C" unsigned long long strtoull(const char *, char **, int) noexcept;
extern "C" int                putenv(char *) noexcept;
static inline long long          _atoi64(const char *s)                    { return atoll(s); }
static inline long long          _strtoi64(const char *s, char **e, int b)  { return strtoll(s, e, b); }
static inline unsigned long long _strtoui64(const char *s, char **e, int b) { return strtoull(s, e, b); }
static inline char *_strlwr(char *s) { for (char *p = s; *p; ++p) *p = (char)tolower((unsigned char)*p); return s; }
static inline char *_strupr(char *s) { for (char *p = s; *p; ++p) *p = (char)toupper((unsigned char)*p); return s; }
#ifndef _putenv
#define _putenv putenv
#endif
#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif
typedef FILE _iobuf;  // MSVC's FILE struct tag, used bare in the decompiled code

// MSVC's 64-bit time CRT (rb_logfile.cpp) and the SEH-era setjmp spelling
// (rb_backend.cpp). __time64_t is just a 64-bit time_t; map onto the POSIX CRT.
typedef long long __time64_t;
static inline __time64_t _time64(__time64_t *t) { time_t r = time(nullptr); if (t) *t = (__time64_t)r; return (__time64_t)r; }
static inline struct tm *_localtime64(const __time64_t *t) { time_t v = (time_t)*t; return localtime(&v); }
static inline struct tm *_gmtime64(const __time64_t *t)    { time_t v = (time_t)*t; return gmtime(&v); }
static inline char       *_ctime64(const __time64_t *t)    { time_t v = (time_t)*t; return ctime(&v); }
#include <cerrno>
#ifndef _errno
#define _errno() (&errno)   // MSVC: errno is (*_errno())
#endif
// _ui64toa(value, buffer, radix) — unsigned 64-bit integer-to-string.
static inline char *_ui64toa(unsigned long long value, char *str, int radix) {
    char tmp[65]; int i = 0;
    do { unsigned d = (unsigned)(value % (unsigned)radix); tmp[i++] = (char)(d < 10 ? '0' + d : 'a' + d - 10); value /= (unsigned)radix; } while (value);
    char *p = str; while (i > 0) *p++ = tmp[--i]; *p = '\0'; return str;
}
// MSVC spells setjmp `_setjmp`, and the decompiled code passes a raw int*/void*
// buffer (not a typed jmp_buf). Bridge straight to glibc's __sigsetjmp primitive
// (savemask 0): this both casts the buffer and avoids the glibc setjmp<->_setjmp
// macro recursion that `#define _setjmp setjmp` would trigger.
#ifndef _setjmp
#define _setjmp(buf) __sigsetjmp((struct __jmp_buf_tag *)(buf), 0)
#endif
// longjmp's matching restore — the decompiled code passes the same raw int*/void*
// buffer. _longjmp is glibc's non-sigmask variant (pairs with __sigsetjmp(...,0))
// and isn't a macro, so this doesn't recurse.
#define longjmp(buf, val) _longjmp((struct __jmp_buf_tag *)(buf), (val))

// Structured Exception Handling -> C++ try/catch. GCC has no SEH; the filter
// expression is dropped (catch-all). This compiles the crash-guard scaffolding;
// actual hardware-fault interception is a separate (deferred) concern.
#ifndef __try
#define __try            try
#define __except(filter) catch (...)
#define __finally
#endif

// Our 32-bit build uses the x87 FPU (no -mfpmath=sse), which is what the few
// precision-sensitive files assert via MSVC's _M_IX86_FP (0 == x87, no SSE).
#ifndef _M_IX86_FP
#define _M_IX86_FP 0
#endif

// MSVC's qsort/bsearch comparator typedef (used in casts, e.g. rb_imagetouch.cpp).
typedef int (*_CoreCrtNonSecureSearchSortCompareFunction)(const void *, const void *);

#ifndef __forceinline
#define __forceinline inline __attribute__((always_inline))
#endif

// Calling conventions -> nothing (see header note above).
#ifndef __cdecl
#define __cdecl
#endif
#ifndef __stdcall
#define __stdcall
#endif
#ifndef __fastcall
#define __fastcall
#endif
#ifndef __thiscall
#define __thiscall
#endif

// MSVC's <windows.h> provides 2-arg min/max as macros; the decompiled engine calls
// them bare. We can't use macros (they poison the C++ standard library — e.g.
// numeric_limits::max), so provide global function templates instead. Safe because
// the engine never does `using namespace std` (verified), so bare min/max resolve
// here with no ambiguity. Two type params + a decltype'd return mirror the macro's
// type-agnostic behaviour (mixed int/float operands).
#if defined(__cplusplus)
template <class A, class B> inline auto min(A a, B b) -> decltype(a < b ? a : b) { return a < b ? a : b; }
template <class A, class B> inline auto max(A a, B b) -> decltype(a > b ? a : b) { return a > b ? a : b; }
#endif

// Sized integer keywords (also defined identically in q_shared.h under __GNUC__;
// benign redefinition). Lets standalone headers like ui/keycodes.h — which use
// `enum X : __int32` — compile without pulling in q_shared.h.
#ifndef __int8
#define __int8  char
#endif
#ifndef __int16
#define __int16 short
#endif
#ifndef __int32
#define __int32 int
#endif
#ifndef __int64
#define __int64 long long
#endif

// MSVC __declspec(...) -> GCC/Clang. GCC does not parse `__declspec(align(N))`
// natively (the decompiled code uses it on hundreds of structs). Map it via token
// paste: __declspec(align(8)) -> __KISAK_DS_align(8) -> __attribute__((aligned(8))).
// Coexists with -fms-extensions (verified).
#define __declspec(x)          __KISAK_DS_##x
#define _declspec(x)           __KISAK_DS_##x   // deprecated single-underscore spelling (r_stream.h)
#define __KISAK_DS_align(n)    __attribute__((aligned(n)))
#define __KISAK_DS_noinline    __attribute__((noinline))
#define __KISAK_DS_noreturn    __attribute__((noreturn))
#define __KISAK_DS_nothrow     __attribute__((nothrow))
#define __KISAK_DS_deprecated  __attribute__((deprecated))
#define __KISAK_DS_selectany   __attribute__((weak))
#define __KISAK_DS_naked
#define __KISAK_DS_dllimport
#define __KISAK_DS_dllexport
#define __KISAK_DS_import
#define __KISAK_DS_export

#endif // !_MSC_VER
#endif // KISAK_MSVC_COMPAT_H
