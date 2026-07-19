// windows.h — minimal portable <windows.h> for non-Windows builds.
//
// Placed on the include path only on non-Windows, so the game's `#include
// <windows.h>` resolves here. Provides the base Win32 types the engine uses; grown
// as real translation units surface more of the surface. On Windows the real SDK
// header is used and this file is never seen.
#ifndef KISAK_WINDOWS_H
#define KISAK_WINDOWS_H

#include "_kisak_wintypes.h"

// Message-pump / windowing scalar types (the actual pump lives in the platform
// layer; these let renderer/headers that mention them compile).
typedef UINT_PTR  WPARAM;
typedef LONG_PTR  LPARAM;
typedef LONG_PTR  LRESULT;
typedef DWORD     COLORREF;
typedef WORD      ATOM;

KISAK_DECLARE_HANDLE(HMENU);
KISAK_DECLARE_HANDLE(HICON);
KISAK_DECLARE_HANDLE(HCURSOR);
KISAK_DECLARE_HANDLE(HBRUSH);
KISAK_DECLARE_HANDLE(HBITMAP);
KISAK_DECLARE_HANDLE(HGLRC);

// NOTE: deliberately NOT defining max/min macros here. MSVC's <windows.h> does
// (unless NOMINMAX), but those 2-arg macros poison the C++ standard library
// (std::max, numeric_limits::max, iterator traits). The handful of bare max()/min()
// call sites in the decompiled renderer are spelled std::max/std::min instead.

#ifndef MAKEFOURCC
#define MAKEFOURCC(a, b, c, d) \
    ((DWORD)(BYTE)(a) | ((DWORD)(BYTE)(b) << 8) | \
     ((DWORD)(BYTE)(c) << 16) | ((DWORD)(BYTE)(d) << 24))
#endif

// ---- Win32 thread / timing / atomic APIs the engine calls ------------------
#include "../compat/msvc_intrin.h"  // _Interlocked*/MemoryBarrier
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <ctime>
#include <cerrno>

static inline DWORD GetCurrentThreadId()  { return (DWORD)(uintptr_t)pthread_self(); }
static inline DWORD GetCurrentProcessId() { return (DWORD)getpid(); }
static inline void  Sleep(DWORD ms)       { if (ms) usleep((useconds_t)ms * 1000u); }
static inline BOOL  SwitchToThread()      { return sched_yield() == 0; }

typedef struct _SYSTEMTIME {
    WORD wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMilliseconds;
} SYSTEMTIME, *LPSYSTEMTIME;
static inline void GetLocalTime(SYSTEMTIME *st) {
    time_t t = time(nullptr); struct tm tmv; localtime_r(&t, &tmv);
    st->wYear = (WORD)(tmv.tm_year + 1900); st->wMonth = (WORD)(tmv.tm_mon + 1);
    st->wDayOfWeek = (WORD)tmv.tm_wday;      st->wDay = (WORD)tmv.tm_mday;
    st->wHour = (WORD)tmv.tm_hour; st->wMinute = (WORD)tmv.tm_min;
    st->wSecond = (WORD)tmv.tm_sec; st->wMilliseconds = 0;
}
static inline void GetSystemTime(SYSTEMTIME *st) { GetLocalTime(st); }

// MSVC CRT _itoa(value, buffer, radix) — minimal integer-to-string.
static inline char *_itoa(int value, char *str, int radix) {
    char tmp[35]; int i = 0;
    bool neg = (radix == 10 && value < 0);
    unsigned int v = neg ? (unsigned int)(-value) : (unsigned int)value;
    do { unsigned d = v % (unsigned)radix; tmp[i++] = (char)(d < 10 ? '0' + d : 'a' + d - 10); v /= (unsigned)radix; } while (v);
    char *p = str;
    if (neg) *p++ = '-';
    while (i > 0) *p++ = tmp[--i];
    *p = '\0';
    return str;
}

// The non-underscore Interlocked* Win32 functions; macros (type-generic, like the
// _Interlocked* intrinsics) so they accept the engine's various pointer types.
#define InterlockedExchange(p, v)          __sync_lock_test_and_set((p), (v))
#define InterlockedExchangeAdd(p, v)       __sync_fetch_and_add((p), (v))
#define InterlockedIncrement(p)            __sync_add_and_fetch((p), 1)
#define InterlockedDecrement(p)            __sync_sub_and_fetch((p), 1)
#define InterlockedCompareExchange(p, e, c) __sync_val_compare_and_swap((p), (c), (e))
#define InterlockedExchange64(p, v)        __sync_lock_test_and_set((p), (v))
#define InterlockedExchangeAdd64(p, v)     __sync_fetch_and_add((p), (v))
#define InterlockedIncrement64(p)          __sync_add_and_fetch((p), 1)
#define InterlockedDecrement64(p)          __sync_sub_and_fetch((p), 1)
#define InterlockedCompareExchange64(p, e, c) __sync_val_compare_and_swap((p), (c), (e))

// ---- System info -----------------------------------------------------------
typedef struct _SYSTEM_INFO {
    union { DWORD dwOemId; struct { WORD wProcessorArchitecture, wReserved; }; };
    DWORD     dwPageSize;
    void     *lpMinimumApplicationAddress, *lpMaximumApplicationAddress;
    DWORD_PTR dwActiveProcessorMask;
    DWORD     dwNumberOfProcessors, dwProcessorType, dwAllocationGranularity;
    WORD      wProcessorLevel, wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO;
static inline void GetSystemInfo(SYSTEM_INFO *si) {
    if (!si) return; memset(si, 0, sizeof(*si));
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    si->dwNumberOfProcessors = n > 0 ? (DWORD)n : 1;
    si->dwPageSize = 4096; si->dwAllocationGranularity = 65536; si->wProcessorArchitecture = 0;
}

// ---- COM init + wide/narrow string conversion ------------------------------
// COM doesn't exist on Linux; the audio init calls become no-ops. The string
// converters do a plain UTF-16<->8-bit truncation (the engine only converts ASCII
// device names).
static inline HRESULT CoInitializeEx(void *, DWORD) { return 0; }   // S_OK
static inline HRESULT CoInitialize(void *)          { return 0; }
static inline void    CoUninitialize()              {}
static inline HRESULT CLSIDFromString(const wchar_t *, GUID *clsid) { if (clsid) { DWORD *p = (DWORD *)clsid; p[0]=p[1]=p[2]=p[3]=0; } return 0; }
static inline int WideCharToMultiByte(UINT, DWORD, const wchar_t *wstr, int cch, char *out, int cb, const char *, int *) {
    if (!wstr) return 0;
    int n = 0; int limit = (cb > 0) ? cb : 0x7fffffff;
    while ((cch < 0 || n < cch) && n < limit) { wchar_t w = wstr[n]; if (cch < 0 && w == 0) { if (out) out[n] = 0; n++; break; } if (out) out[n] = (char)w; if (w == 0) { n++; break; } n++; }
    return n;
}
static inline int MultiByteToWideChar(UINT, DWORD, const char *str, int cch, wchar_t *out, int cw) {
    if (!str) return 0;
    int n = 0; int limit = (cw > 0) ? cw : 0x7fffffff;
    while ((cch < 0 || n < cch) && n < limit) { char c = str[n]; if (out) out[n] = (wchar_t)(unsigned char)c; if (c == 0) { n++; break; } n++; }
    return n;
}

// ---- Dynamic library loading -----------------------------------------------
// The engine LoadLibraryA's a few Windows-only DLLs (ddraw.dll, ...) and degrades
// gracefully when they are missing. Returning null here takes that fallback path —
// the matching POSIX route would be dlopen, but none of these DLLs exist on Linux.
static inline HMODULE LoadLibraryA(const char *) { return nullptr; }
static inline void   *GetProcAddress(HMODULE, const char *) { return nullptr; }
static inline BOOL    FreeLibrary(HMODULE) { return TRUE; }

// ---- Window / monitor management -------------------------------------------
// The GL backend owns the single SDL window (it creates it in CreateDevice). The
// renderer's Win32 window calls are bridged onto that window by the SDL platform
// layer (src/platform/sdl/sdl_window.cpp): window-manipulation calls are no-ops /
// sentinels (the SDL window is authoritative), while the monitor/metrics queries
// return real SDL display data so the renderer picks a correct resolution. These
// are plain declarations (not inline) to keep SDL out of this header.
typedef struct tagMONITORINFO {
    DWORD cbSize;
    RECT  rcMonitor;
    RECT  rcWork;
    DWORD dwFlags;
} MONITORINFO, *LPMONITORINFO;
typedef BOOL (*MONITORENUMPROC)(HMONITOR, HDC, LPRECT, LPARAM);

int      GetSystemMetrics(int nIndex);                 // 0=SM_CXSCREEN, 1=SM_CYSCREEN
HMONITOR MonitorFromPoint(POINT pt, DWORD dwFlags);
HMONITOR MonitorFromWindow(HWND hWnd, DWORD dwFlags);
BOOL     GetMonitorInfoA(HMONITOR hMonitor, LPMONITORINFO lpmi);
BOOL     EnumDisplayMonitors(HDC hdc, LPRECT lprcClip, MONITORENUMPROC lpfnEnum, LPARAM dwData);
BOOL     ClientToScreen(HWND hWnd, LPPOINT lpPoint);
BOOL     AdjustWindowRectEx(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle);
HWND     CreateWindowExA(DWORD exStyle, const char *cls, const char *name, DWORD style,
                         int x, int y, int w, int h, HWND parent, HMENU menu,
                         HINSTANCE inst, void *param);
BOOL     DestroyWindow(HWND hWnd);
BOOL     IsWindow(HWND hWnd);
BOOL     ShowWindow(HWND hWnd, int nCmdShow);
BOOL     SetWindowPos(HWND hWnd, HWND hWndAfter, int x, int y, int cx, int cy, UINT flags);
LONG     SetWindowLongA(HWND hWnd, int nIndex, LONG dwNewLong);
HWND     SetFocus(HWND hWnd);
HMODULE  GetModuleHandleA(const char *lpModuleName);

// ---- Misc Win32 ------------------------------------------------------------
typedef struct _FILETIME { DWORD dwLowDateTime, dwHighDateTime; } FILETIME, *LPFILETIME;
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
static inline DWORD GetLastError() { return (DWORD)errno; }
static inline void  SetLastError(DWORD e) { errno = (int)e; }
static inline void  OutputDebugStringA(const char *s) { if (s) fputs(s, stderr); }
// Win32 ShowCursor maintains a per-process display counter: +1 on show, -1 on
// hide, returning the new value (cursor visible when >= 0). The engine's swap
// loop spins ShowCursor() until the returned counter reaches a target, so a
// constant return value would loop forever (it hung the render thread in
// RB_SwapBuffers). Track the counter so the loop converges.
static inline int   ShowCursor(BOOL bShow) {
    static int s_cursorDisplayCount = 0;
    s_cursorDisplayCount += bShow ? 1 : -1;
    return s_cursorDisplayCount;
}
static inline BOOL  SystemTimeToFileTime(const SYSTEMTIME *, FILETIME *ft) {
    if (ft) { ft->dwLowDateTime = 0; ft->dwHighDateTime = 0; } return TRUE;
}

// ---- A couple more file helpers --------------------------------------------
static inline BOOL SetFileAttributesA(const char *, DWORD) { return TRUE; }  // attrs are a Win32 concept

// ---- Critical sections (no-op; the engine's real locking goes through the
// Sys_*CriticalSection layer — these satisfy the Win32-typed declarations) -----
typedef struct _RTL_CRITICAL_SECTION { void *opaque[8]; } CRITICAL_SECTION, *LPCRITICAL_SECTION;
static inline void InitializeCriticalSection(LPCRITICAL_SECTION) {}
static inline void DeleteCriticalSection(LPCRITICAL_SECTION) {}
static inline void EnterCriticalSection(LPCRITICAL_SECTION) {}
static inline void LeaveCriticalSection(LPCRITICAL_SECTION) {}
static inline BOOL TryEnterCriticalSection(LPCRITICAL_SECTION) { return TRUE; }

// ---- Window message pump (stubbed; real input runs through SDL) -------------
typedef struct tagMSG {
    HWND hwnd; UINT message; WPARAM wParam; LPARAM lParam; DWORD time; POINT pt;
} MSG, *LPMSG;
static inline BOOL    PeekMessageA(MSG *, HWND, UINT, UINT, UINT) { return FALSE; }
static inline BOOL    GetMessageA(MSG *, HWND, UINT, UINT) { return FALSE; }
static inline BOOL    TranslateMessage(const MSG *) { return FALSE; }
static inline LRESULT DispatchMessageA(const MSG *) { return 0; }

// ---- High-resolution timer (QPC -> CLOCK_MONOTONIC) ------------------------
// (LARGE_INTEGER lives in _kisak_wintypes.h — also needed by d3d9 headers.)
static inline BOOL QueryPerformanceCounter(LARGE_INTEGER *c) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    if (c) c->QuadPart = (LONGLONG)ts.tv_sec * 1000000000LL + ts.tv_nsec; return TRUE;
}
static inline BOOL QueryPerformanceFrequency(LARGE_INTEGER *f) {
    if (f) f->QuadPart = 1000000000LL; return TRUE;   // QPC counts nanoseconds
}

// ---- Async-overlap / SEH placeholders --------------------------------------
// OVERLAPPED is an opaque member of the async file-load state (db_file_load.h);
// the real async I/O runs through the platform layer. _EXCEPTION_POINTERS only
// needs to name a type for the unhandled-filter prototype (win_main.h); the body
// lives in the Windows-only win32/ source.
typedef struct _OVERLAPPED {
    ULONG_PTR Internal, InternalHigh;
    DWORD Offset, OffsetHigh;
    HANDLE hEvent;
} OVERLAPPED, *LPOVERLAPPED;
struct _EXCEPTION_POINTERS;

// ---- Kernel objects: file / thread / event / semaphore / mutex -------------
// All Win32 kernel handles are unified behind one tagged HANDLE, implemented over
// POSIX/pthreads in src/platform/sdl/win_kernel.cpp; CloseHandle and
// WaitForSingleObject dispatch on the object kind. Declared (not inline) so the
// handle table has a single owner.
#define WAIT_OBJECT_0    0x0
#define WAIT_TIMEOUT     0x102
#define INFINITE         0xFFFFFFFF
#define CREATE_SUSPENDED 0x4

typedef DWORD (*LPTHREAD_START_ROUTINE)(void *);
typedef struct _SECURITY_ATTRIBUTES { DWORD nLength; void *lpSecurityDescriptor; BOOL bInheritHandle; } SECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;
static inline BOOL TerminateThread(HANDLE, DWORD) { return TRUE; }   // best-effort no-op (pthreads can't force-kill safely)
#define STILL_ACTIVE 259
static inline BOOL GetExitCodeThread(HANDLE, DWORD *code) { if (code) *code = 0; return TRUE; }   // thread already finished
typedef struct _MEMORYSTATUS {
    DWORD  dwLength, dwMemoryLoad;
    SIZE_T dwTotalPhys, dwAvailPhys, dwTotalPageFile, dwAvailPageFile, dwTotalVirtual, dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;
static inline void GlobalMemoryStatus(MEMORYSTATUS *s) {
    if (!s) return; memset(s, 0, sizeof(*s)); s->dwLength = sizeof(*s);
    long pages = sysconf(_SC_PHYS_PAGES), avail = sysconf(_SC_AVPHYS_PAGES), psz = sysconf(_SC_PAGE_SIZE);
    if (pages > 0 && psz > 0) s->dwTotalPhys = (SIZE_T)pages * psz;
    if (avail > 0 && psz > 0) s->dwAvailPhys = (SIZE_T)avail * psz;
    s->dwTotalVirtual = s->dwTotalPhys; s->dwAvailVirtual = s->dwAvailPhys;
}
static inline DWORD GetCurrentThreadId();   // (defined earlier)

HANDLE CreateThread(void *attrs, SIZE_T stack, LPTHREAD_START_ROUTINE start, void *param, DWORD flags, DWORD *threadId);
DWORD  ResumeThread(HANDLE thread);
DWORD  SuspendThread(HANDLE thread);
HANDLE CreateEventA(void *attrs, BOOL manualReset, BOOL initialState, const char *name);
BOOL   SetEvent(HANDLE ev);
BOOL   ResetEvent(HANDLE ev);
BOOL   PulseEvent(HANDLE ev);
HANDLE CreateSemaphoreA(void *attrs, LONG initialCount, LONG maxCount, const char *name);
BOOL   ReleaseSemaphore(HANDLE sem, LONG releaseCount, LONG *prevCount);
HANDLE CreateMutexA(void *attrs, BOOL initialOwner, const char *name);
BOOL   ReleaseMutex(HANDLE mtx);
DWORD  WaitForSingleObject(HANDLE h, DWORD ms);
BOOL   CloseHandle(HANDLE h);
BOOL   DuplicateHandle(HANDLE srcProc, HANDLE src, HANDLE dstProc, HANDLE *dst,
                       DWORD access, BOOL inherit, DWORD options);

// ---- File I/O (HANDLE = fd; impl in win_kernel.cpp) ------------------------
#ifndef GENERIC_READ
#define GENERIC_READ          0x80000000u
#define GENERIC_WRITE         0x40000000u
#define FILE_SHARE_READ       0x1
#define FILE_SHARE_WRITE      0x2
#define CREATE_ALWAYS         2
#define OPEN_EXISTING         3
#define OPEN_ALWAYS           4
#define FILE_ATTRIBUTE_NORMAL    0x80u
#define FILE_ATTRIBUTE_DIRECTORY 0x10u
#define INVALID_FILE_ATTRIBUTES  ((DWORD)-1)
#define FILE_BEGIN   0
#define FILE_CURRENT 1
#define FILE_END     2
#endif
HANDLE CreateFileA(const char *name, DWORD access, DWORD share, void *sec, DWORD disp, DWORD flags, HANDLE tmpl);
BOOL   ReadFile(HANDLE h, void *buf, DWORD n, DWORD *numRead, OVERLAPPED *ov);
BOOL   ReadFileEx(HANDLE h, void *buf, DWORD n, OVERLAPPED *ov, void *completion);
BOOL   WriteFile(HANDLE h, const void *buf, DWORD n, DWORD *written, OVERLAPPED *ov);
DWORD  GetFileSize(HANDLE h, DWORD *high);
DWORD  SetFilePointer(HANDLE h, LONG dist, LONG *distHigh, DWORD method);
DWORD  GetFileAttributesA(const char *name);
DWORD  GetCurrentDirectoryA(DWORD len, char *buf);
DWORD  GetModuleFileNameA(HMODULE mod, char *buf, DWORD size);

// ---- Virtual / global memory (malloc/mmap-backed; impl in win_kernel.cpp) --
typedef struct _MEMORY_BASIC_INFORMATION {
    void  *BaseAddress, *AllocationBase;
    DWORD  AllocationProtect;
    SIZE_T RegionSize;
    DWORD  State, Protect, Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;
#ifndef MEM_COMMIT
#define MEM_COMMIT   0x1000
#define MEM_RESERVE  0x2000
#define MEM_DECOMMIT 0x4000
#define MEM_RELEASE  0x8000
#define PAGE_READWRITE 0x04
#endif
typedef HANDLE HGLOBAL;
void  *VirtualAlloc(void *addr, SIZE_T size, DWORD type, DWORD protect);
BOOL   VirtualFree(void *addr, SIZE_T size, DWORD type);
SIZE_T VirtualQuery(const void *addr, MEMORY_BASIC_INFORMATION *info, SIZE_T len);
HGLOBAL GlobalAlloc(UINT flags, SIZE_T size);
void   *GlobalLock(HGLOBAL h);
BOOL    GlobalUnlock(HGLOBAL h);
HGLOBAL GlobalFree(HGLOBAL h);

// ---- Process / debug / exceptions (inline, self-contained) -----------------
#ifndef EXCEPTION_EXECUTE_HANDLER
#define EXCEPTION_EXECUTE_HANDLER     1
#define EXCEPTION_CONTINUE_SEARCH     0
#define EXCEPTION_CONTINUE_EXECUTION (-1)
#endif
typedef struct _TOKEN_PRIVILEGES { DWORD PrivilegeCount; struct { struct { DWORD LowPart; LONG HighPart; } Luid; DWORD Attributes; } Privileges[1]; } TOKEN_PRIVILEGES, *PTOKEN_PRIVILEGES;
static inline void ExitProcess(UINT code)            { _exit((int)code); }
// No-op: the common caller is SetThreadName's magic SEH "name this thread" exception,
// which is meant to be swallowed when no debugger is attached. Genuine fatal paths go
// through Sys_Error / assert, not RaiseException.
static inline void RaiseException(DWORD, DWORD, DWORD, const void *) {}
static inline BOOL IsDebuggerPresent()               { return FALSE; }
static inline void GetSystemTimeAsFileTime(FILETIME *ft) { if (ft) { ft->dwLowDateTime = 0; ft->dwHighDateTime = 0; } }
// SleepEx: an alertable wait (bAlertable=TRUE) is how the engine's async DB loader
// waits for an I/O completion APC. Our overlapped I/O is synchronous (the data is
// already read by the time we get here), so return WAIT_IO_COMPLETION immediately
// rather than blocking forever on the APC that will never come.
static inline DWORD SleepEx(DWORD ms, BOOL alertable) {
    if (alertable) return 0x000000C0;                              // WAIT_IO_COMPLETION
    if (ms && ms != INFINITE) usleep((useconds_t)ms * 1000u);
    return 0;
}
static inline void *InterlockedExchangePointer(void **target, void *value) { return __sync_lock_test_and_set(target, value); }

// ---- Thread scheduling (best-effort / no-op on Linux) ----------------------
static inline DWORD_PTR SetThreadAffinityMask(HANDLE, DWORD_PTR mask)        { return mask; }
static inline DWORD     SetThreadIdealProcessor(HANDLE, DWORD proc)          { return proc; }
static inline BOOL      SetThreadPriority(HANDLE, int)                       { return TRUE; }
// Report one affinity bit per online CPU. The engine derives its CPU count
// (and thus sys_smp_allowed) by popcount-ing this mask, so returning a single
// bit would force single-threaded rendering — which is wrong for the GL
// backend, whose context is bound to the render thread and must receive all
// draw work via the SMP hand-off rather than inline on the main thread.
static inline BOOL      GetProcessAffinityMask(HANDLE, DWORD_PTR *p, DWORD_PTR *s) {
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    if (n < 1)  n = 1;
    if (n > 32) n = 32;  // the mask is 32-bit; the engine caps the count anyway
    DWORD_PTR mask = (n >= 32) ? (DWORD_PTR)~0u : (((DWORD_PTR)1 << n) - 1);
    if (p) *p = mask; if (s) *s = mask; return TRUE;
}
static inline HANDLE    GetCurrentThread()                                   { return (HANDLE)(intptr_t)-2; }
static inline HANDLE    GetCurrentProcess()                                  { return (HANDLE)(intptr_t)-1; }

// ---- Window / GDI / clipboard / shell (no-op on the SDL/Linux build) -------
typedef BOOL (*WNDENUMPROC)(HWND, LPARAM);
#ifndef IDC_ARROW
#define IDC_ARROW   ((const char *)32512)
#define IDC_WAIT    ((const char *)32514)
#define IDC_APPSTARTING ((const char *)32650)
#endif
static inline HCURSOR LoadCursor(HINSTANCE, const char *) { return (HCURSOR)0; }
static inline HCURSOR SetCursor(HCURSOR)                  { return (HCURSOR)0; }
static inline HANDLE  OpenProcess(DWORD, BOOL, DWORD)     { return (HANDLE)0; }
static inline DWORD   GetCurrentProcessId();   // (defined earlier)
static inline HWND  GetActiveWindow()                       { return (HWND)0; }
static inline HWND  GetDesktopWindow()                      { return (HWND)0; }
static inline HDC   GetDC(HWND)                             { return (HDC)0; }
static inline int   ReleaseDC(HWND, HDC)                    { return 1; }
static inline LONG  GetWindowLongA(HWND, int)              { return 0; }
static inline int   GetWindowTextA(HWND, char *buf, int n)  { if (buf && n) buf[0] = '\0'; return 0; }
static inline BOOL  EnumThreadWindows(DWORD, WNDENUMPROC, LPARAM) { return TRUE; }
static inline LONG  ChangeDisplaySettingsA(void *, DWORD)   { return 0; }   // DISP_CHANGE_SUCCESSFUL
static inline BOOL  SetDeviceGammaRamp(HDC, void *)         { return TRUE; }
#ifndef MB_OK
#define MB_OK 0x0
#define MB_OKCANCEL 0x1
#define MB_YESNO 0x4
#define MB_ICONERROR 0x10
#define MB_ICONWARNING 0x30
#define MB_ICONINFORMATION 0x40
#define IDOK 1
#define IDCANCEL 2
#define IDYES 6
#define IDNO 7
#endif
static inline int   MessageBoxA(HWND, const char *, const char *, UINT) { return 1; }   // IDOK
static inline int   MessageBoxW(HWND, const wchar_t *, const wchar_t *, UINT) { return 1; }
static inline HINSTANCE ShellExecuteA(HWND, const char *, const char *, const char *, const char *, int) { return (HINSTANCE)33; }
static inline BOOL  OpenClipboard(HWND)                     { return FALSE; }
static inline BOOL  CloseClipboard()                       { return TRUE; }
static inline BOOL  EmptyClipboard()                       { return TRUE; }
static inline HANDLE SetClipboardData(UINT, HANDLE h)       { return h; }

// ---- Path / directory / file-time helpers ----------------------------------
static inline BOOL RemoveDirectoryA(const char *path) { return rmdir(path) == 0; }
// GetFullPathNameA normalizes lexically (the target need not exist) -> getcwd join.
static inline DWORD GetFullPathNameA(const char *name, DWORD len, char *buf, char **filePart) {
    char abs[MAX_PATH * 4];
    if (name[0] == '/') { abs[0] = '\0'; strncat(abs, name, sizeof(abs) - 1); }
    else { if (!getcwd(abs, sizeof(abs))) return 0; size_t n = strlen(abs);
           if (n && abs[n - 1] != '/') { abs[n++] = '/'; abs[n] = '\0'; } strncat(abs, name, sizeof(abs) - n - 1); }
    DWORD wrote = (DWORD)strlen(abs);
    if (buf && len) { strncpy(buf, abs, len - 1); buf[len - 1] = '\0';
        if (filePart) { char *s = strrchr(buf, '/'); *filePart = s ? s + 1 : buf; } }
    return wrote;
}
static inline LONG CompareFileTime(const FILETIME *a, const FILETIME *b) {
    unsigned long long x = ((unsigned long long)a->dwHighDateTime << 32) | a->dwLowDateTime;
    unsigned long long y = ((unsigned long long)b->dwHighDateTime << 32) | b->dwLowDateTime;
    return x < y ? -1 : (x > y ? 1 : 0);
}

// ---- File enumeration / deletion (impl in win_kernel.cpp) ------------------
// FindFirstFile* are backed by glob() in win_kernel.cpp — NOT here, because
// <glob.h> pollutes the global namespace with `glob`, which the engine uses as an
// ordinary variable name (e.g. SentientGlobals glob). Same reasoning as keeping
// <cstdlib>'s random() out of this header.
typedef struct _WIN32_FIND_DATAA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime, ftLastAccessTime, ftLastWriteTime;
    DWORD nFileSizeHigh, nFileSizeLow, dwReserved0, dwReserved1;
    char cFileName[MAX_PATH];
    char cAlternateFileName[14];
} WIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;

HANDLE FindFirstFileA(const char *pattern, WIN32_FIND_DATAA *d);
BOOL   FindNextFileA(HANDLE handle, WIN32_FIND_DATAA *d);
BOOL   FindClose(HANDLE handle);
static inline BOOL DeleteFileA(const char *path) { return unlink(path) == 0; }

#endif // KISAK_WINDOWS_H
