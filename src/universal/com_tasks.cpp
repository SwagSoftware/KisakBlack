#include "com_tasks.h"
#include <universal/offline.h>
#include <qcommon/cmd.h>
#include <qcommon/threads.h>

// LWSS: online task manager (stubbed)

TaskRecord s_taskRecords[256];
TaskRecord s_taskFreeHead;
TaskRecord s_taskActiveHead;

unsigned __int64 s_taskMemoryPool[8192];
int s_taskMemoryPoolIndex;
unsigned int s_taskPollCount;
volatile int g_taskHead;



cmd_function_s TaskManager2_DumpTasks_VAR;
void __cdecl TaskManager2_Init()
{
}

void __cdecl TaskManager2_DeferTaskToMainThread(RemoteTask *remoteTask, const TaskDefinition *taskDef, void *payload)
{
}

void __cdecl TaskManager2_PickUpDeferredTasks()
{
}

void __cdecl TaskManager2_ProcessXOverlappedTask(TaskRecord *task)
{
}

void __cdecl TaskManager2_CancelEndlessTasks(int localControllerIndex)
{
}

void __cdecl Task_Deallocate(_BYTE *ptr, int bytes)
{
}

void __cdecl ChunkFree(int index, int blocks)
{
}

int __cdecl ChunkNext(int index)
{
    return 0;
}

void __cdecl TaskManager2_ComErrorCleanup()
{
}

void __cdecl TaskManager2_FreeDeadTasks(int localControllerIndex)
{
}

void __cdecl TaskManager2_FreeAllPendingTasksForController(int localControllerIndex)
{
}

void __cdecl TaskManager2_FreeAllPendingTasksInternal(int localControllerIndex)
{
}

void __cdecl TaskManager2_HandleTimedOutTask(TaskRecord *TimedOutTask)
{
}

void __cdecl TaskManager2_ProcessRemoteTask(TaskRecord *task)
{
}

void __cdecl TaskManager2_ProcessNestedTask(TaskRecord *task)
{
}

void __cdecl TaskManager2_ProcessTasks(int localControllerIndex)
{
}

void TaskManager2_CreateDeferredTasks()
{
}

TaskRecord *__cdecl TaskManager2_CreateTaskFromServerThread(
                const TaskDefinition *definition,
                int controllerIndex,
                TaskRecord *nestTask,
                int timeout)
{
    return 0;
}

int __cdecl Task_Allocate(int bytes)
{
    int blocks; // [esp+20h] [ebp-8h]
    int index; // [esp+24h] [ebp-4h]

    blocks = (bytes + 7) / 8;
    for ( index = s_taskMemoryPoolIndex;
                SLODWORD(s_taskMemoryPool[index]) < blocks || (HIDWORD(s_taskMemoryPool[index]) & 1) != 1;
                index = ChunkNext(index) )
    {
        if ( ChunkNext(index) == s_taskMemoryPoolIndex )
        {
            Com_Error(ERR_DROP, "Could not allocate %d bytes from task memory pool\n", bytes);
            return 0;
        }
    }
    s_taskMemoryPoolIndex = ChunkAllocate(index, blocks);
    return 8 * index + 161336912;
}

int __cdecl ChunkAllocate(int index, int blocks)
{
    return 0;
}

bool __cdecl TaskManager2_IsValidServerTask(const TaskDefinition *definition)
{
    return false;
}

TaskRecord *__cdecl TaskManager2_CreateTask(
                const TaskDefinition *definition,
                int controllerIndex,
                TaskRecord *nestTask,
                int timeout)
{
    return 0;
}

void __cdecl TaskManager2_StartTask(TaskRecord *task)
{
}

char __cdecl TaskManger2_TaskGetInProgressForControllerByName(const char *taskName, int controllerIndex)
{
    return 0;
}

TaskRecord *__cdecl TaskManager2_TaskGetInProgressForController(const TaskDefinition *definition, int controllerIndex)
{
    return 0;
}

TaskRecord *__cdecl TaskManager2_TaskGetInProgress(const TaskDefinition *definition)
{
    return 0;
}

bool __cdecl TaskManager2_TaskIsInProgressForController(const TaskDefinition *definition, int controllerIndex)
{
    return false;
}

bool __cdecl TaskManager2_TaskIsInProgress(const TaskDefinition *definition)
{
    return false;
}

int __cdecl TaskManager2_CountTasksInProgress(const TaskDefinition *definition)
{
    return 0;
}

int __cdecl TaskManager2_CountTasksInProgressForController(int controllerindex)
{
    return 0;
}

bool __cdecl TaskManager2_TaskIsPending(const TaskRecord *task)
{
    return false;
}

bool __cdecl TaskManager2_TaskIsTimedOut(const TaskRecord *task)
{
    return false;
}

void __cdecl TaskManager2_DumpTasks()
{
}

void __cdecl TaskManager2_StateToString(TaskState state, char *string, unsigned int stringsize)
{
}

