#include "tl_support.h"
#include <universal/mem_userhunk.h>
#include "common.h"
#include <win32/win_common.h>
#include <win32/win_workercmds.h>
#include <tl/jobqueue/jobqueue_all.h>

HunkUser *s_tlHunkUser;

static tlJobFunc s_tlJobFunc;
static void *s_tlJobUserData;
static volatile long s_tlJobNextIndex;
static jqBatch s_tlJobBatch;

static int __cdecl TL_RunJobsBatch(jqBatch *batch)
{
    s_tlJobFunc(s_tlJobUserData, InterlockedIncrement(&s_tlJobNextIndex) - 1);
    return 0;
}

static jqModule s_tlJobModule =
{
    .Name = "tlRunJobs",
    .Type = JQ_WORKER_GENERIC,
    .Code = TL_RunJobsBatch,
};

// TreyBO's job dispatch (tlRunJobs): func(user_data, i) once for every i in [0, job_count) on the nuge_physics
// worker cmd, returning when all of them are done. This is what phys_task_manager_process + _flush did.
void __cdecl TL_RunJobs(tlJobFunc func, void *user_data, int job_count)
{
    s_tlJobFunc = func;
    s_tlJobUserData = user_data;
    s_tlJobNextIndex = 0;
    s_tlJobBatch.Module = &s_tlJobModule;
    s_tlJobBatch.Input = user_data;
    for ( int i = 0; i < job_count; ++i )
        IW_task_manager_add_batch(&s_tlJobBatch);
    IW_task_manager_flush();
}

void __cdecl Sys_SetupTLCallbacks(int hunkMemSize)
{
    tlSystemCallbacks callbacks; // [esp+0h] [ebp-20h] BYREF

    s_tlHunkUser = Hunk_UserCreate(hunkMemSize, HU_SCHEME_DEFAULT, 4u, 0, "TL_MemAlloc support", 37);
    callbacks.ReadFile = (bool (__cdecl *)(const char *, tlFileBuf *, unsigned int, unsigned int))TL_ReadFile;
    callbacks.ReleaseFile = (void (__cdecl *)(tlFileBuf *))TL_ReleaseFile;
    callbacks.CriticalError = TL_CriticalError;
    callbacks.Warning = TL_Warning;
    callbacks.DebugPrint = (void (__cdecl *)(const char *))TL_DebugPrint;
    callbacks.MemAlloc = (void *(__cdecl *)(unsigned int, unsigned int, unsigned int))TL_MemAlloc;
    callbacks.MemRealloc = (void *(__cdecl *)(void *, unsigned int, unsigned int, unsigned int))RETURN_ZERO32;
    callbacks.MemFree = TL_MemFree;
    callbacks.RunJobs = TL_RunJobs;
    tlSetSystemCallbacks(&callbacks);
}

void __cdecl TL_Warning(const char *Text)
{
    Com_PrintWarning(14, "TL Warning: %s\n", Text);
}

bool __cdecl TL_ReadFile()
{
    TL_Warning("TL_ReadFile: no implementation");
    return 0;
}

void __cdecl TL_ReleaseFile()
{
    TL_Warning("TL_ReleaseFile: no implementation");
}

void __cdecl TL_DebugPrint(char *Text)
{
    Com_Printf(14, Text);
}

void *__cdecl TL_MemAlloc(unsigned int Size, unsigned int Align)
{
    void *v3; // [esp+8h] [ebp-Ch]

    Sys_EnterCriticalSection(CRITSECT_TL_MEMALLOC);
    v3 = Hunk_UserAlloc(s_tlHunkUser, Size, Align, 0);
    Sys_LeaveCriticalSection(CRITSECT_TL_MEMALLOC);
    return v3;
}

void __cdecl TL_MemFree(void *Ptr)
{
    Hunk_UserFree(s_tlHunkUser, Ptr);
}

void __cdecl TL_CriticalError(const char *msg)
{
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\tl_support.cpp", 95, 0, msg) )
        __debugbreak();
}

