// linux_main.cpp — the Linux entry point + the remaining platform stubs that pull
// the whole engine together into a binary. main() mirrors win32/win_main.cpp's WinMain
// init sequence (critical sections -> main thread/TLS -> Com_Init -> Com_Frame loop).
// The gamepad, worker-thread/task-manager, streaming and crypto symbols are stubbed so
// the engine links and boots; these are progressively replaced as bring-up continues.
#include <qcommon/common.h>
#include <qcommon/threads.h>
#include <qcommon/tl_support.h>
#include <win32/win_main.h>
#include <win32/win_common.h>
#include <win32/win_gamepad.h>
#include <win32/win_workercmds.h>
#include <win32/win_tasks.h>
#include <win32/win_stream.h>
#include <demo/demo_common.h>
#include <physics/phys_broad_phase.h>

#include <cstring>
#include <cstdio>
#include <csignal>
#include <execinfo.h>
#include <cstdlib>

// Crash backtrace — the engine's asserts trap via __debugbreak (SIGILL); print where.
static void CrashHandler(int sig) {
    void *bt[40]; int n = backtrace(bt, 40);
    fprintf(stderr, "\n*** caught signal %d — backtrace (%d frames) ***\n", sig, n);
    backtrace_symbols_fd(bt, n, 2);
    fflush(stderr); _exit(128 + sig);
}

// ---- Gamepad: no controller (SDL game-controller support is future work) ----
const dvar_t *gpad_enabled = nullptr;
bool   GPad_IsActive(int) { return false; }
double GPad_GetButton(int, GamePadButton) { return 0.0; }
double GPad_GetStick(int, GamePadStick) { return 0.0; }
bool   GPad_IsButtonPressed(int, GamePadButton) { return false; }
bool   GPad_IsStickPressed(int, GamePadStick, GamePadStickDir) { return false; }
bool   GPad_IsStickReleased(int, GamePadStick, GamePadStickDir) { return false; }


// ---- Streaming / task manager (worker threads now in linux_workercmds.cpp) ---
// R_InitWorkerThreads, IW_task_manager_*, and the nuge_physics job module are the
// real job-queue bring-up, ported to src/platform/linux/linux_workercmds.cpp.
char Stream_Init() { return 1; }
bool PC_StartWithNoSounds() { return false; }
char TaskManager_AnyTaskInProgress(overlappedTask *) { return 0; }
void TaskManager_ClearOverlappedTasks(overlappedTask *) {}
void TaskManager_ClearTask(overlappedTask *) {}
void *TaskManager_GetTaskData(overlappedTask *) { return nullptr; }
char TaskManager_TaskIsInProgress(overlappedTask *, int) { return 0; }

// ---- vtable/typeinfo anchors for two decompiled polymorphic structs ---------
// Defining each class's key virtual out-of-line makes the compiler emit its vtable
// and typeinfo here (otherwise nothing in the build emits them).
unsigned int bdDownloadInterceptor::handleDownload(void *, unsigned int) { return 0; }
void broad_phase_terrain_query_callback::query(const broad_phase_environment_query_input *, broad_phase_environement_query_results *) {}

// ---- libtomcrypt: CBC + cipher-registry stubs -------------------------------
// TODO(bring-up): compile the real libtomcrypt for fast-file decryption. Stubbed to
// link (signatures match tomcrypt.h, which the engine headers pull in).
extern "C" {
int find_cipher(const char *) { return -1; }
int cbc_start(int, const unsigned char *, const unsigned char *, int, int, symmetric_CBC *) { return 0; }
int cbc_encrypt(const unsigned char *pt, unsigned char *ct, unsigned long len, symmetric_CBC *) { if (pt && ct && pt != ct) memcpy(ct, pt, len); return 0; }
int cbc_decrypt(const unsigned char *ct, unsigned char *pt, unsigned long len, symmetric_CBC *) { if (ct && pt && ct != pt) memcpy(pt, ct, len); return 0; }
int cbc_setiv(const unsigned char *, unsigned long, symmetric_CBC *) { return 0; }
const char *error_to_string(int) { return "ok"; }
}

// ---- Entry point -----------------------------------------------------------
int main(int argc, char **argv) {
    char cmdline[2048] = {0};
    for (int i = 1; i < argc; ++i) {
        size_t n = strlen(cmdline);
        snprintf(cmdline + n, sizeof(cmdline) - n, "%s%s", argv[i], i + 1 < argc ? " " : "");
    }
    fprintf(stderr, "[KisakBlack] boot: cmdline=\"%s\"\n", cmdline);
    signal(SIGILL, CrashHandler); signal(SIGSEGV, CrashHandler); signal(SIGABRT, CrashHandler);

    Sys_InitializeCriticalSections();
    Sys_InitMainThread();
    Sys_SetupTLCallbacks(0x900000);
    Com_Init(cmdline);

    for (;;) Com_Frame();
    return 0;
}
