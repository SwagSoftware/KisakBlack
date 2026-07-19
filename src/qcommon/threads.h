#pragma once

#include <Windows.h>

//enum $93E05B02D75CB8D5BD679C8A761DAE10 : __int32
enum ThreadContext_t : __int32 // not a real enum name
{
    THREAD_CONTEXT_MAIN = 0x0,
    THREAD_CONTEXT_BACKEND = 0x1,
    THREAD_CONTEXT_WORKER0 = 0x2,
    THREAD_CONTEXT_WORKER1 = 0x3,
    THREAD_CONTEXT_WORKER2 = 0x4,
    THREAD_CONTEXT_WORKER3 = 0x5,
    THREAD_CONTEXT_WORKER4 = 0x6,
    THREAD_CONTEXT_WORKER5 = 0x7,
    THREAD_CONTEXT_WORKER6 = 0x8,
    THREAD_CONTEXT_WORKER7 = 0x9,
    THREAD_CONTEXT_SERVER = 0xA,
    THREAD_CONTEXT_OCCLUSION = 0xB,
    THREAD_CONTEXT_TRACE_COUNT = 0xC,
    THREAD_CONTEXT_TRACE_LAST = 0xB,
    THREAD_CONTEXT_TITLE_SERVER = 0xC,
    THREAD_CONTEXT_DATABASE = 0xD,
    THREAD_CONTEXT_STREAM = 0xE,
    THREAD_CONTEXT_COUNT = 0xF,
    THREAD_CONTEXT_INVALID = -1,
};

enum ThreadOwner : __int32
{                                       // XREF: ?Sys_SuspendDiscReads@@YAXW4ThreadOwner@@@Z/r
    THREAD_OWNER_NONE       = 0x0,
    THREAD_OWNER_DATABASE   = 0x1,
    THREAD_OWNER_CINEMATICS = 0x2,
    THREAD_OWNER_SHUTDOWN   = 0x3,
};

#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType;     // Must be 0x1000.
    LPCSTR szName;    // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags;    // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)


unsigned int __cdecl Sys_GetDefaultWorkerThreadsCount();
unsigned int __cdecl Sys_GetCpuCount();
void __cdecl Sys_InitMainThread();
unsigned int __cdecl Sys_GetCurrentThreadId();
unsigned int Win_InitThreads();
void __cdecl Sys_InitThread(int threadContext);
char __cdecl Sys_SpawnRenderThread(void (__cdecl *function)(unsigned int));
void __cdecl Sys_CreateEvent(bool manualReset, bool initialState, void **event);
void __cdecl Sys_CreateThread(void (__cdecl *function)(unsigned int), unsigned int threadContext);
void __cdecl SetThreadName(unsigned int dwThreadID, const char *szThreadName);
DWORD WINAPI Sys_ThreadMain(LPVOID threadContext);
void __cdecl Sys_InitDemoStreamingEvent();
void __cdecl Sys_WaitForDemoStreamingEvent();
void __cdecl Sys_WaitForSingleObject(void **event);
void __cdecl Sys_SetDemoStreamingEvent();
void __cdecl Sys_SetEvent(void **event);
void __cdecl Sys_InitServerEvents();
void __cdecl Sys_ResetEvent(void **event);
char __cdecl Sys_SpawnServerThread(void (__cdecl *function)(unsigned int));
char __cdecl Sys_SpawnDatabaseThread(void (__cdecl *function)(unsigned int));
void __cdecl Sys_SuspendDiscReads(ThreadOwner owner);
void __cdecl Sys_ResumeDiscReads(ThreadOwner owner);
bool __cdecl Sys_HaveSuspendedDiscReads(ThreadOwner owner);
void __cdecl Sys_WaitDiscReads();
void __cdecl Sys_InitWorkerThreadContext();
void __cdecl Sys_ResumeThread(unsigned int threadContext);
int __cdecl Sys_RendererSleep();
bool __cdecl Sys_RendererReady();
void __cdecl Sys_RenderCompleted();
void __cdecl Sys_FrontEndSleep();
int __cdecl Sys_WaitRenderer();
bool __cdecl Sys_WaitForSingleObjectTimeout(void **event, unsigned intmsec);
void __cdecl Sys_WakeRenderer(void *data);
void __cdecl Sys_NotifyRenderer();
void __cdecl Sys_SleepServer();
bool __cdecl Sys_WaitServer(unsigned int timeout);
void __cdecl Sys_WakeServer();
void __cdecl Sys_ServerCompleted();
bool __cdecl Sys_WaitStartServer(unsigned int timeout);
bool __cdecl Sys_IsServerThread();
void __cdecl Sys_DatabaseCompleted();
void __cdecl Sys_WaitStartDatabase();
bool __cdecl Sys_IsDatabaseReady();
void __cdecl Sys_SyncDatabase();
void __cdecl Sys_WakeDatabase();
void __cdecl Sys_NotifyDatabase();
void __cdecl Sys_DatabaseCompleted2();
bool __cdecl Sys_IsDatabaseReady2();
void __cdecl Sys_WakeDatabase2();
void __cdecl Sys_StopRenderer();
void __cdecl Sys_StartRenderer();
bool __cdecl Sys_IsRenderThread();
bool __cdecl Sys_IsDatabaseThread();
bool __cdecl Sys_IsMainThread();
const char *__cdecl Sys_GetCurrentThreadName();
int __cdecl Sys_GetThreadContext();
void __cdecl Sys_SetValue(int valueIndex, void *data);
void *__cdecl Sys_GetValue(int valueIndex);
void __cdecl Sys_SetWorkerCmdEvent();
bool __cdecl Sys_WaitBackendEvent(unsigned int msec);
void __cdecl Sys_SetUpdateSpotLightEffectEvent();
void __cdecl Sys_ResetUpdateSpotLightEffectEvent();
void __cdecl Sys_WaitUpdateNonDependentEffectsCompleted();
void __cdecl Sys_SetUpdateNonDependentEffectsEvent();
void __cdecl Sys_ResetUpdateNonDependentEffectsEvent();
void __cdecl Sys_SuspendOtherThreads();
void __cdecl Sys_SetWin32QuitEvent();
bool __cdecl Sys_QueryWin32QuitEvent();
void __cdecl Sys_WaitD3DDeviceOKEvent();
void __cdecl Sys_ClearD3DDeviceOKEvent();
void __cdecl Sys_SetD3DDeviceOKEvent();
bool __cdecl Sys_QueryD3DDeviceOKEvent();
void __cdecl Sys_WaitResourcesFlushedEvent();
void __cdecl Sys_ClearResourcesFlushedEvent();
void __cdecl Sys_SetResourcesFlushedEvent();
void __cdecl Sys_WaitResourcesQueuedEvent(unsigned int msec);
void __cdecl Sys_ClearResourcesQueuedEvent();
void __cdecl Sys_SetResourcesQueuedEvent();
void __cdecl Sys_SetD3DDeviceStartEvent();
void __cdecl Sys_ClearD3DDeviceStartEvent();
void __cdecl Sys_WaitRGRegisteredEvent();
void __cdecl Sys_SetRGRegisteredEvent();
void __cdecl Sys_ClearRGRegisteredEvent();
bool __cdecl Sys_QueryRGRegisteredEvent();
bool __cdecl Sys_QueryRenderEvent();
void __cdecl Sys_SetD3DShutdownEvent();
void __cdecl Sys_ClearD3DShutdownEvent();
bool __cdecl Sys_QueryD3DShutdownEvent();
char __cdecl Sys_SpawnStreamThread(void (__cdecl *function)(unsigned int));
int __cdecl Sys_StreamSleep();
void __cdecl Sys_SetSndInitializedEvent();
void __cdecl Sys_WakeStream();
bool __cdecl Sys_IsStreamThread();
void __cdecl Sys_SpawnOcclusion(void (__cdecl *function)(unsigned int));
void __cdecl Sys_WaitForOcclusion();
void __cdecl Sys_SetOcclusion();
void __cdecl Sys_SetServerAllowNetworkEvent();
void __cdecl Sys_ResetServerAllowNetworkEvent();
void __cdecl Sys_SetServerNetworkCompletedEvent();
void __cdecl Sys_ResetServerNetworkCompletedEvent();
void __cdecl Sys_WaitAllowServerNetworkLoop();
void __cdecl Sys_WaitServerNetworkCompleted();
void __cdecl Win_LockThreadAffinity();
void __cdecl Win_UnlockThreadAffinity();
