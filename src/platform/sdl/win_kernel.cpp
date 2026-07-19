// win_kernel.cpp — POSIX/pthread implementations of the Win32 "kernel object" APIs
// the engine uses (declared in platform/winsdk/windows.h): files, threads, events,
// semaphores, mutexes, and virtual/global memory. All kernel handles are unified
// behind one tagged object so CloseHandle / WaitForSingleObject dispatch on kind.
//
// This is the Linux replacement for the Win32 kernel calls in src/win32. Nothing
// links against it yet (the engine is still being brought to compile), but the
// implementations are real so the eventual link behaves correctly.
#include <windows.h>

#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>   // mmap/munmap/madvise — page-aligned virtual memory
#include <glob.h>
#include <dirent.h>
#include <strings.h>   // strcasecmp (safe here — this TU includes no engine 'index' globals)
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <ctime>
#include <map>
#include <mutex>

// ---- Case-insensitive path resolution --------------------------------------
// The game ships Windows-cased asset paths (e.g. zone/english) on a case-sensitive
// filesystem (the dir is zone/English). Resolve each component case-insensitively;
// components that don't exist yet (new files being created) pass through unchanged.
namespace {
void ResolveCaseInsensitive(const char *inRaw, char *out, size_t outsz) {
    if (!inRaw) { if (outsz) out[0] = 0; return; }
    // Normalize Windows backslashes to '/' first (paths like base\zone\english\x.ff).
    char in[4096]; { size_t i = 0; for (; inRaw[i] && i < sizeof(in) - 1; ++i) in[i] = inRaw[i] == '\\' ? '/' : inRaw[i]; in[i] = 0; }
    struct stat st;
    if (stat(in, &st) == 0) { strncpy(out, in, outsz - 1); out[outsz - 1] = 0; return; }
    char resolved[4096]; size_t rlen = 0; resolved[0] = 0;
    const char *p = in;
    if (*p == '/') { resolved[rlen++] = '/'; resolved[rlen] = 0; ++p; }
    bool give_up = false;
    while (*p) {
        const char *slash = strchr(p, '/');
        size_t clen = slash ? (size_t)(slash - p) : strlen(p);
        char comp[256]; if (clen >= sizeof(comp)) clen = sizeof(comp) - 1;
        memcpy(comp, p, clen); comp[clen] = 0;
        const char *sep = (rlen && resolved[rlen - 1] != '/') ? "/" : "";
        char trial[4096]; snprintf(trial, sizeof(trial), "%s%s%s", resolved, sep, comp);
        if (give_up || stat(trial, &st) == 0) {
            strncpy(resolved, trial, sizeof(resolved) - 1);
        } else {
            DIR *d = opendir(rlen ? resolved : "."); bool found = false;
            if (d) { struct dirent *e;
                while ((e = readdir(d))) if (strcasecmp(e->d_name, comp) == 0) {
                    snprintf(trial, sizeof(trial), "%s%s%s", resolved, sep, e->d_name);
                    strncpy(resolved, trial, sizeof(resolved) - 1); found = true; break; }
                closedir(d); }
            if (!found) { strncpy(resolved, trial, sizeof(resolved) - 1); give_up = true; }  // new file: keep as-is
        }
        resolved[sizeof(resolved) - 1] = 0; rlen = strlen(resolved);
        p += clen; if (*p == '/') ++p;
    }
    strncpy(out, resolved, outsz - 1); out[outsz - 1] = 0;
}
} // namespace

namespace {

enum Kind { K_FILE, K_THREAD, K_EVENT, K_SEM, K_MUTEX };

struct KObject {
    Kind kind;
    int  fd = -1;                       // K_FILE
    pthread_t thread{};                 // K_THREAD
    LPTHREAD_START_ROUTINE start = nullptr;
    void *param = nullptr;
    pthread_mutex_t mtx{};              // K_EVENT / K_SEM / K_MUTEX / K_THREAD start-gate
    pthread_cond_t  cond{};             // K_EVENT / K_SEM / K_THREAD start-gate
    bool manualReset = false;          // K_EVENT
    bool signaled = false;             // K_EVENT
    long count = 0;                    // K_SEM
    bool suspended = false;            // K_THREAD: CREATE_SUSPENDED start-gate
};

inline KObject *obj(HANDLE h) { return reinterpret_cast<KObject *>(h); }
inline bool valid(HANDLE h)   { return h && h != INVALID_HANDLE_VALUE; }

// Absolute deadline `ms` from now, for pthread_cond_timedwait.
void deadline(struct timespec *ts, DWORD ms) {
    clock_gettime(CLOCK_REALTIME, ts);
    ts->tv_sec  += ms / 1000;
    ts->tv_nsec += (long)(ms % 1000) * 1000000L;
    if (ts->tv_nsec >= 1000000000L) { ts->tv_sec++; ts->tv_nsec -= 1000000000L; }
}

void *thread_thunk(void *arg) {
    KObject *k = static_cast<KObject *>(arg);
    // CREATE_SUSPENDED start-gate: block until ResumeThread() opens it. This is
    // load-bearing — Sys_CreateThread() creates every engine thread suspended,
    // records its id in the thread table, then resumes it. Running early would
    // race the id store (e.g. SV_ServerThread asserting Sys_IsServerThread()).
    if (k->suspended) {
        pthread_mutex_lock(&k->mtx);
        while (k->suspended) pthread_cond_wait(&k->cond, &k->mtx);
        pthread_mutex_unlock(&k->mtx);
    }
    if (k->start) k->start(k->param);
    return nullptr;
}

} // namespace

// ---- Threads ---------------------------------------------------------------
// CREATE_SUSPENDED is honored via a start-gate (see thread_thunk): the thread is
// created but parks before the user routine until ResumeThread() runs. The engine
// relies on this — it stores the new thread's id, then resumes — so the thread
// never observes its id table slot before it is written.
HANDLE CreateThread(void *, SIZE_T, LPTHREAD_START_ROUTINE start, void *param, DWORD flags, DWORD *threadId) {
    KObject *k = new KObject(); k->kind = K_THREAD; k->start = start; k->param = param;
    pthread_mutex_init(&k->mtx, nullptr); pthread_cond_init(&k->cond, nullptr);
    k->suspended = (flags & CREATE_SUSPENDED) != 0;
    if (pthread_create(&k->thread, nullptr, thread_thunk, k) != 0) {
        pthread_cond_destroy(&k->cond); pthread_mutex_destroy(&k->mtx); delete k; return nullptr;
    }
    // The engine stores this id in its thread table and later matches it against
    // GetCurrentThreadId() inside the new thread. GetCurrentThreadId() is
    // (DWORD)(uintptr_t)pthread_self(), and pthread_self() in the new thread equals
    // k->thread — so report that here, NOT 0.
    if (threadId) *threadId = (DWORD)(uintptr_t)k->thread;
    return static_cast<HANDLE>(k);
}
DWORD ResumeThread(HANDLE h) {
    if (!valid(h)) return 0;
    KObject *k = obj(h);
    pthread_mutex_lock(&k->mtx);
    bool was = k->suspended; k->suspended = false; pthread_cond_broadcast(&k->cond);
    pthread_mutex_unlock(&k->mtx);
    return was ? 1 : 0;   // prior suspend count
}
DWORD SuspendThread(HANDLE) { return 0; }

// ---- Events ----------------------------------------------------------------
HANDLE CreateEventA(void *, BOOL manualReset, BOOL initialState, const char *) {
    KObject *k = new KObject(); k->kind = K_EVENT;
    k->manualReset = manualReset; k->signaled = initialState;
    pthread_mutex_init(&k->mtx, nullptr); pthread_cond_init(&k->cond, nullptr);
    return static_cast<HANDLE>(k);
}
BOOL SetEvent(HANDLE h) {
    if (!valid(h)) return FALSE; KObject *k = obj(h);
    pthread_mutex_lock(&k->mtx); k->signaled = true;
    if (k->manualReset) pthread_cond_broadcast(&k->cond); else pthread_cond_signal(&k->cond);
    pthread_mutex_unlock(&k->mtx); return TRUE;
}
BOOL ResetEvent(HANDLE h) {
    if (!valid(h)) return FALSE; KObject *k = obj(h);
    pthread_mutex_lock(&k->mtx); k->signaled = false; pthread_mutex_unlock(&k->mtx); return TRUE;
}
BOOL PulseEvent(HANDLE h) {
    if (!valid(h)) return FALSE; KObject *k = obj(h);
    pthread_mutex_lock(&k->mtx); k->signaled = true; pthread_cond_broadcast(&k->cond);
    k->signaled = false; pthread_mutex_unlock(&k->mtx); return TRUE;
}

// ---- Semaphores ------------------------------------------------------------
HANDLE CreateSemaphoreA(void *, LONG initialCount, LONG, const char *) {
    KObject *k = new KObject(); k->kind = K_SEM; k->count = initialCount;
    pthread_mutex_init(&k->mtx, nullptr); pthread_cond_init(&k->cond, nullptr);
    return static_cast<HANDLE>(k);
}
BOOL ReleaseSemaphore(HANDLE h, LONG releaseCount, LONG *prevCount) {
    if (!valid(h)) return FALSE; KObject *k = obj(h);
    pthread_mutex_lock(&k->mtx);
    if (prevCount) *prevCount = (LONG)k->count;
    k->count += releaseCount; pthread_cond_signal(&k->cond);
    pthread_mutex_unlock(&k->mtx); return TRUE;
}

// ---- Mutexes (recursive, matching Win32 semantics) -------------------------
HANDLE CreateMutexA(void *, BOOL initialOwner, const char *) {
    KObject *k = new KObject(); k->kind = K_MUTEX;
    pthread_mutexattr_t a; pthread_mutexattr_init(&a);
    pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&k->mtx, &a); pthread_mutexattr_destroy(&a);
    if (initialOwner) pthread_mutex_lock(&k->mtx);
    return static_cast<HANDLE>(k);
}
BOOL ReleaseMutex(HANDLE h) {
    if (!valid(h)) return FALSE; pthread_mutex_unlock(&obj(h)->mtx); return TRUE;
}

// ---- Wait / close / duplicate ----------------------------------------------
DWORD WaitForSingleObject(HANDLE h, DWORD ms) {
    if (!valid(h)) return WAIT_TIMEOUT;
    KObject *k = obj(h);
    switch (k->kind) {
    case K_MUTEX: pthread_mutex_lock(&k->mtx); return WAIT_OBJECT_0;
    case K_THREAD: pthread_join(k->thread, nullptr); return WAIT_OBJECT_0;
    case K_EVENT: {
        pthread_mutex_lock(&k->mtx); int rc = 0;
        struct timespec ts; if (ms != INFINITE) deadline(&ts, ms);
        while (!k->signaled && rc == 0)
            rc = (ms == INFINITE) ? pthread_cond_wait(&k->cond, &k->mtx)
                                  : pthread_cond_timedwait(&k->cond, &k->mtx, &ts);
        DWORD r = WAIT_OBJECT_0;
        if (!k->signaled) r = WAIT_TIMEOUT; else if (!k->manualReset) k->signaled = false;
        pthread_mutex_unlock(&k->mtx); return r;
    }
    case K_SEM: {
        pthread_mutex_lock(&k->mtx); int rc = 0;
        struct timespec ts; if (ms != INFINITE) deadline(&ts, ms);
        while (k->count <= 0 && rc == 0)
            rc = (ms == INFINITE) ? pthread_cond_wait(&k->cond, &k->mtx)
                                  : pthread_cond_timedwait(&k->cond, &k->mtx, &ts);
        DWORD r = WAIT_OBJECT_0;
        if (k->count <= 0) r = WAIT_TIMEOUT; else k->count--;
        pthread_mutex_unlock(&k->mtx); return r;
    }
    default: return WAIT_OBJECT_0;
    }
}
BOOL CloseHandle(HANDLE h) {
    if (!valid(h)) return FALSE; KObject *k = obj(h);
    switch (k->kind) {
    case K_FILE:   if (k->fd >= 0) close(k->fd); break;
    case K_THREAD: pthread_detach(k->thread);
                   pthread_cond_destroy(&k->cond); pthread_mutex_destroy(&k->mtx); break;
    case K_EVENT:
    case K_SEM:    pthread_cond_destroy(&k->cond);  pthread_mutex_destroy(&k->mtx); break;
    case K_MUTEX:  pthread_mutex_destroy(&k->mtx); break;
    }
    delete k; return TRUE;
}
// Win32 duplicates create a second reference; the engine only uses the duplicate as
// an opaque handle, so alias the same object (close is the caller's responsibility).
BOOL DuplicateHandle(HANDLE, HANDLE src, HANDLE, HANDLE *dst, DWORD, BOOL, DWORD) {
    if (dst) *dst = src; return TRUE;
}

// ---- Files (HANDLE = K_FILE wrapping a POSIX fd) ---------------------------
HANDLE CreateFileA(const char *name, DWORD access, DWORD, void *, DWORD disp, DWORD, HANDLE) {
    int flags = 0;
    if ((access & GENERIC_READ) && (access & GENERIC_WRITE)) flags = O_RDWR;
    else if (access & GENERIC_WRITE)                         flags = O_WRONLY;
    else                                                     flags = O_RDONLY;
    switch (disp) {
    case CREATE_ALWAYS: flags |= O_CREAT | O_TRUNC; break;
    case OPEN_ALWAYS:   flags |= O_CREAT;           break;
    case OPEN_EXISTING: default:                    break;
    }
    char resolved[4096]; ResolveCaseInsensitive(name, resolved, sizeof(resolved));
    int fd = open(resolved, flags, 0644);
    if (fd < 0) return INVALID_HANDLE_VALUE;
    KObject *k = new KObject(); k->kind = K_FILE; k->fd = fd;
    return static_cast<HANDLE>(k);
}
BOOL ReadFile(HANDLE h, void *buf, DWORD n, DWORD *numRead, OVERLAPPED *) {
    if (!valid(h)) return FALSE; ssize_t r = read(obj(h)->fd, buf, n);
    if (numRead) *numRead = (r < 0) ? 0 : (DWORD)r; return r >= 0;
}
// Overlapped read: honour the OVERLAPPED offset (pread doesn't move the file
// position). The engine's DB loader does these "async" then waits via an alertable
// SleepEx — which our SleepEx returns from immediately, so the data is ready by then.
BOOL ReadFileEx(HANDLE h, void *buf, DWORD n, OVERLAPPED *ov, void *) {
    if (!valid(h)) return FALSE;
    off_t off = ov ? (off_t)(((unsigned long long)ov->OffsetHigh << 32) | ov->Offset) : 0;
    ssize_t r = ov ? pread(obj(h)->fd, buf, n, off) : read(obj(h)->fd, buf, n);
    return r >= 0;
}
BOOL WriteFile(HANDLE h, const void *buf, DWORD n, DWORD *written, OVERLAPPED *) {
    if (!valid(h)) return FALSE; ssize_t w = write(obj(h)->fd, buf, n);
    if (written) *written = (w < 0) ? 0 : (DWORD)w; return w >= 0;
}
DWORD GetFileSize(HANDLE h, DWORD *high) {
    if (!valid(h)) return (DWORD)-1; struct stat st;
    if (fstat(obj(h)->fd, &st) != 0) return (DWORD)-1;
    if (high) *high = (DWORD)((unsigned long long)st.st_size >> 32);
    return (DWORD)(st.st_size & 0xffffffffu);
}
DWORD SetFilePointer(HANDLE h, LONG dist, LONG *distHigh, DWORD method) {
    if (!valid(h)) return (DWORD)-1;
    off_t off = (distHigh ? ((off_t)*distHigh << 32) : 0) | (unsigned)dist;
    int whence = method == FILE_CURRENT ? SEEK_CUR : method == FILE_END ? SEEK_END : SEEK_SET;
    off_t r = lseek(obj(h)->fd, off, whence);
    if (r < 0) return (DWORD)-1;
    if (distHigh) *distHigh = (LONG)((unsigned long long)r >> 32);
    return (DWORD)(r & 0xffffffffu);
}
DWORD GetFileAttributesA(const char *name) {
    char resolved[4096]; ResolveCaseInsensitive(name, resolved, sizeof(resolved));
    struct stat st; if (stat(resolved, &st) != 0) return INVALID_FILE_ATTRIBUTES;
    return S_ISDIR(st.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
}
DWORD GetCurrentDirectoryA(DWORD len, char *buf) {
    if (!buf || !getcwd(buf, len)) return 0; return (DWORD)strlen(buf);
}
DWORD GetModuleFileNameA(HMODULE, char *buf, DWORD size) {
    if (!buf || size == 0) return 0;
    ssize_t n = readlink("/proc/self/exe", buf, size - 1);
    if (n < 0) { buf[0] = '\0'; return 0; }
    buf[n] = '\0'; return (DWORD)n;
}

// ---- Virtual / global memory (mmap-backed) ---------------------------------
// The engine's Z_Virtual* layer reserves an address range then commits sub-ranges,
// and Z_VirtualFree walks the committed regions via VirtualQuery before releasing.
// Win32 VirtualAlloc always returns PAGE-ALIGNED memory; the engine's hunk relies on
// that (it page-aligns sub-ranges with `& 0xFFFFF000` and asserts allocations are
// 32-byte aligned). calloc only guarantees 16-byte alignment (a malloc-chunk header
// sits between the page start and the returned pointer), which broke the hunk's
// alignment assert during map load. So we back the reservation with mmap(MAP_ANONYMOUS):
// page-aligned, demand-zero, lazily committed — matching Win32 reserve/commit semantics.
// We track each (base -> size) so MEM_RELEASE can munmap and VirtualQuery can report
// the owning allocation (its free-walk asserts AllocationBase==ptr, RegionSize>0).
namespace { std::map<char *, size_t> g_vallocs; std::mutex g_vmutex; }
void *VirtualAlloc(void *addr, SIZE_T size, DWORD, DWORD) {
    if (addr) return addr;                          // commit within an already-reserved range (lazy)
    size_t n = size ? size : 1;
    void *p = mmap(nullptr, n, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) return nullptr;
    { std::lock_guard<std::mutex> lk(g_vmutex); g_vallocs[(char *)p] = n; }
    return p;
}
BOOL VirtualFree(void *addr, SIZE_T size, DWORD freeType) {
    if (!addr) return TRUE;
    if (freeType & MEM_RELEASE) {
        // Release the whole reservation; addr is its tracked base.
        size_t n = 0;
        { std::lock_guard<std::mutex> lk(g_vmutex);
          auto it = g_vallocs.find((char *)addr);
          if (it != g_vallocs.end()) { n = it->second; g_vallocs.erase(it); } }
        if (n) munmap(addr, n);
    } else {
        // MEM_DECOMMIT: addr/size are a page-aligned SUB-REGION of a reservation.
        // madvise(MADV_DONTNEED) drops the physical pages and faults in fresh zero
        // pages on next access — exactly Win32's decommit / next-commit contract.
        // (Hunk_Clear -> Z_VirtualDecommit passes page-aligned ranges.)
        if (size) {
            if (((uintptr_t)addr & 0xFFF) == 0 && (size & 0xFFF) == 0)
                madvise(addr, size, MADV_DONTNEED);
            else
                memset(addr, 0, size);
        }
    }
    return TRUE;
}
SIZE_T VirtualQuery(const void *addr, MEMORY_BASIC_INFORMATION *info, SIZE_T len) {
    if (!info) return 0;
    memset(info, 0, sizeof(*info));
    char *a = (char *)addr;
    std::lock_guard<std::mutex> lk(g_vmutex);
    auto it = g_vallocs.upper_bound(a);             // first base strictly greater than a
    if (it != g_vallocs.begin()) {
        --it; char *base = it->first; size_t sz = it->second;
        if (a >= base && a < base + sz) {
            info->BaseAddress = a; info->AllocationBase = base;
            info->RegionSize = (SIZE_T)(base + sz - a);
            info->State = 0x1000; info->Protect = 0x04; info->Type = 0x20000;   // COMMIT / RW / PRIVATE
            return len;
        }
    }
    return 0;                                       // not a tracked address
}
HGLOBAL GlobalAlloc(UINT, SIZE_T size)  { return static_cast<HGLOBAL>(calloc(1, size ? size : 1)); }
void   *GlobalLock(HGLOBAL h)           { return h; }
BOOL    GlobalUnlock(HGLOBAL)           { return TRUE; }
HGLOBAL GlobalFree(HGLOBAL h)           { free(h); return nullptr; }

// ---- Directory enumeration (FindFirstFile* -> glob) ------------------------
// Kept here (not in windows.h) so <glob.h>'s `glob` symbol doesn't collide with the
// engine's `glob` variables. Find handles are a separate namespace from kernel
// objects above — the engine closes them with FindClose, never CloseHandle.
namespace {
struct FindState { glob_t g; size_t i; };
void FindFill(FindState *h, WIN32_FIND_DATAA *d) {
    const char *p = h->g.gl_pathv[h->i];
    const char *base = strrchr(p, '/'); base = base ? base + 1 : p;
    int n = 0; while (base[n] && n < MAX_PATH - 1) { d->cFileName[n] = base[n]; ++n; }
    d->cFileName[n] = '\0'; d->dwFileAttributes = 0;
}
} // namespace
HANDLE FindFirstFileA(const char *pattern, WIN32_FIND_DATAA *d) {
    FindState *h = new FindState(); h->i = 0;
    if (glob(pattern, 0, nullptr, &h->g) != 0 || h->g.gl_pathc == 0) { globfree(&h->g); delete h; return INVALID_HANDLE_VALUE; }
    FindFill(h, d); return static_cast<HANDLE>(h);
}
BOOL FindNextFileA(HANDLE handle, WIN32_FIND_DATAA *d) {
    FindState *h = static_cast<FindState *>(handle);
    if (++h->i >= h->g.gl_pathc) return FALSE;
    FindFill(h, d); return TRUE;
}
BOOL FindClose(HANDLE handle) {
    FindState *h = static_cast<FindState *>(handle);
    if (h && h != INVALID_HANDLE_VALUE) { globfree(&h->g); delete h; } return TRUE;
}
