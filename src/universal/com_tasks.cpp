#include "com_tasks.h"
#include <live/live_storage.h>
#include <qcommon/cmd.h>
#include <qcommon/threads.h>

// LWSS: this appears to be some more demonware networking crap

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
#ifdef KISAK_DW_TASK
    int idx; // [esp+0h] [ebp-4h]

    s_taskFreeHead.next = s_taskRecords;
    s_taskActiveHead.next = 0;
    for ( idx = 0; idx < 255; ++idx )
        s_taskRecords[idx].next = &s_taskRecords[idx + 1];
    s_taskRecords[255].next = 0;
    memset(s_taskMemoryPool, 0, sizeof(s_taskMemoryPool));
    s_taskMemoryPoolIndex = 0;
    s_taskMemoryPool[0] = 0x100001FFFLL;
    s_taskPollCount = 0;
    Cmd_AddCommandInternal("dumpTasks", TaskManager2_DumpTasks, &TaskManager2_DumpTasks_VAR);
    Com_Printf(32, "Task manager initialized.\n");
#endif
}

void __cdecl TaskManager2_DeferTaskToMainThread(bdRemoteTask *dwTask, const TaskDefinition *taskDef, void *payload)
{
#ifdef KISAK_DW_TASK
    TaskRecord *newTask; // [esp+10h] [ebp-4h]

    dwEnterDeferredCritsec();
    newTask = &g_DeferredTasks[g_taskHead++];
    Com_DPrintf(32, "g_taskHead is now %i\n", g_taskHead);
    newTask->definition = taskDef;
    newTask->controllerIndex = 0;
    bdReference<bdCommonAddr>::operator=(&newTask->remoteTask, dwTask);
    newTask->ownerThread = 1;
    newTask->payload = payload;
    dwLeaveDeferredCritsec();
#endif
}

void __cdecl TaskManager2_PickUpDeferredTasks()
{
#ifdef KISAK_DW_TASK
    int i; // [esp+8h] [ebp-4h]

    if ( !Sys_IsServerThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    277,
                    0,
                    "%s\n\t%s",
                    "Sys_IsServerThread()",
                    "Tried to pickup deferred tasks from not server thread!!\n") )
    {
        __debugbreak();
    }
    dwEnterDeferredCritsec();
    for ( i = 63; i > g_taskEnd; --i )
    {
        if ( bdRemoteTask::getStatus(g_DeferredTasks[i].remoteTask.m_ptr) == BD_PENDING
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                        282,
                        0,
                        "%s",
                        "bdRemoteTask::BD_PENDING != g_DeferredTasks[i].remoteTask->getStatus()") )
        {
            __debugbreak();
        }
        if ( !g_DeferredTasks[i].ownerThread
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                        283,
                        0,
                        "%s\n\t%s",
                        "0 != g_DeferredTasks[i].ownerThread",
                        "Task in g_deferredTasks with no ownerthread\n") )
        {
            __debugbreak();
        }
        if ( bdRemoteTask::getStatus(g_DeferredTasks[i].remoteTask.m_ptr) == BD
            && g_DeferredTasks[i].definition->completed_callback )
        {
            g_DeferredTasks[i].definition->completed_callback(&g_DeferredTasks[i]);
        }
        else if ( g_DeferredTasks[i].definition->failure_callback )
        {
            g_DeferredTasks[i].definition->failure_callback(&g_DeferredTasks[i]);
        }
    }
    g_taskEnd = 63;
    dwLeaveDeferredCritsec();
#endif
}

void __cdecl TaskManager2_ProcessXOverlappedTask(TaskRecord *task)
{
#ifdef KISAK_DW_TASK
    if ( !task && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp", 500, 0, "%s", "task") )
        __debugbreak();
    if ( !task->definition
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    501,
                    0,
                    "%s",
                    "task->definition") )
    {
        __debugbreak();
    }
    Com_Error(ERR_FATAL, "XOverlapped task (%s) created on unsupported platform\n", task->definition->name);
#endif
}

void __cdecl TaskManager2_CancelEndlessTasks(int localControllerIndex)
{
#ifdef KISAK_DW_TASK
    bool cancelTask; // [esp+17h] [ebp-9h]
    TaskRecord *next; // [esp+18h] [ebp-8h]
    TaskRecord *current; // [esp+1Ch] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    514,
                    0,
                    "%s\n\t%s",
                    "Sys_IsMainThread()",
                    "TaskManager2 is not threadsafe.\n") )
    {
        __debugbreak();
    }
    current = &s_taskActiveHead;
    while ( current->next )
    {
        next = current->next->next;
        if ( !current->next->definition
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                        523,
                        0,
                        "%s",
                        "(*current)->definition") )
        {
            __debugbreak();
        }
        cancelTask = 0;
        if ( localControllerIndex == -1 )
        {
            if ( TaskManager2_TaskIsPending(current->next) && (current->next->definition->category & 2) != 0 )
                cancelTask = current->next->timeOut == -1;
        }
        else if ( current->next->controllerIndex == localControllerIndex
                     && TaskManager2_TaskIsPending(current->next)
                     && (current->next->definition->category & 2) != 0 )
        {
            cancelTask = current->next->timeOut == -1;
        }
        if ( cancelTask )
        {
            Com_DPrintf(
                32,
                "Cancelling task %s for controlleridx %i\n",
                current->next->definition->name,
                current->next->controllerIndex);
            bdRemoteTask::cancelTask(current->next->remoteTask.m_ptr);
            current->next->state = TASK_STATE_CHILDFAILED;
            if ( current->next->definition->failure_callback )
                current->next->definition->failure_callback(current->next);
            if ( current->next->definition->payloadSize && current->next->payload )
            {
                Task_Deallocate((_BYTE *)current->next->payload, current->next->definition->payloadSize);
            }
            else if ( current->next->definition->payloadSize )
            {
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                                560,
                                0,
                                "%s",
                                "(*current)->definition->payloadSize == 0") )
                    __debugbreak();
            }
            current->next->payload = 0;
            current->next->definition = 0;
            current->next->next = s_taskFreeHead.next;
            s_taskFreeHead.next = current->next;
            current->next = next;
        }
        else
        {
            current = current->next;
        }
    }
#endif
}

void __cdecl Task_Deallocate(_BYTE *ptr, int bytes)
{
#ifdef KISAK_DW_TASK
    int index; // [esp+Ch] [ebp-4h]

    index = ((ptr - (_BYTE *)s_taskMemoryPool) >> 3) - 1;
    if ( index < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp", 125, 0, "%s", "index >= 0") )
    {
        __debugbreak();
    }
    ChunkFree(index, (bytes + 7) / 8);
#endif
}

void __cdecl ChunkFree(int index, int blocks)
{
#ifdef KISAK_DW_TASK
    int next; // [esp+1Ch] [ebp-4h]

    if ( LODWORD(s_taskMemoryPool[index]) != blocks
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    85,
                    0,
                    "%s",
                    "ChunkSize( index ) == blocks") )
    {
        __debugbreak();
    }
    if ( (HIDWORD(s_taskMemoryPool[index]) & 1) == 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    86,
                    0,
                    "%s",
                    "!ChunkIsFree( index )") )
    {
        __debugbreak();
    }
    next = ChunkNext(index);
    if ( next > index && (HIDWORD(s_taskMemoryPool[next]) & 1) == 1 )
    {
        blocks += LODWORD(s_taskMemoryPool[next]) + 1;
        LODWORD(s_taskMemoryPool[next]) = -26804617;
        HIDWORD(s_taskMemoryPool[next]) = -29032875;
        if ( s_taskMemoryPoolIndex == next )
            s_taskMemoryPoolIndex = index;
    }
    LODWORD(s_taskMemoryPool[index]) = blocks;
    HIDWORD(s_taskMemoryPool[index]) = (blocks >> 31) | 1;
#endif
}

int __cdecl ChunkNext(int index)
{
#ifdef KISAK_DW_TASK
    int size; // [esp+0h] [ebp-4h]

    size = LODWORD(s_taskMemoryPool[index]) + 1;
    if ( size + index > 0x2000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    55,
                    0,
                    "%s",
                    "index + size <= TASK_MEMORY_POOL_SIZE") )
    {
        __debugbreak();
    }
    if ( size + index == 0x2000 )
        return 0;
    else
        return size + index;
#else
    return 0;
#endif
}

void __cdecl TaskManager2_ComErrorCleanup()
{
#ifdef KISAK_DW_TASK
    int i; // [esp+0h] [ebp-4h]

    if ( Sys_IsMainThread() )
    {
        for ( i = 0; i < 1; ++i )
        {
            TaskManager2_FreeDeadTasks(i);
            TaskManager2_CancelEndlessTasks(i);
            TaskManager2_ProcessTasks(i);
        }
    }
    else
    {
        Com_PrintWarning(0, "Skipped TaskManager2_ComErrorCleanup because I'm not the main thread.\n");
    }
#endif
}

void __cdecl TaskManager2_FreeDeadTasks(int localControllerIndex)
{
#ifdef KISAK_DW_TASK
    int v1; // eax
    TaskRecord *next; // [esp+0h] [ebp-8h]
    TaskRecord *current; // [esp+4h] [ebp-4h]

    if ( dwGetLogOnStatus(localControllerIndex) == 2 )
    {
        if ( TaskManager2_CountTasksInProgressForController(localControllerIndex) > 0 )
        {
            v1 = TaskManager2_CountTasksInProgressForController(localControllerIndex);
            Com_Printf(32, "TASK: %d total tasks while disconnected from DW for controlleridx %i\n", v1, localControllerIndex);
        }
        TaskManager2_FreeAllPendingTasksForController(localControllerIndex);
    }
    current = &s_taskActiveHead;
    while ( current->next )
    {
        next = current->next->next;
        if ( !current->next->definition
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                        412,
                        0,
                        "%s",
                        "(*current)->definition") )
        {
            __debugbreak();
        }
        if ( TaskManager2_TaskIsPending(current->next) || current->next->controllerIndex != localControllerIndex )
        {
            if ( TaskManager2_TaskIsTimedOut(current->next) )
                TaskManager2_HandleTimedOutTask(current->next);
            else
                current = current->next;
        }
        else
        {
            Com_DPrintf(32, "Freeing task %s(%d)\n", current->next->definition->name, current->next->startMS);
            if ( current->next->payload && current->next->definition->payloadSize > 0 )
            {
                Task_Deallocate((_BYTE *)current->next->payload, current->next->definition->payloadSize);
            }
            else if ( current->next->definition->payloadSize )
            {
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                                424,
                                0,
                                "%s",
                                "(*current)->definition->payloadSize == 0") )
                    __debugbreak();
            }
            current->next->payload = 0;
            current->next->definition = 0;
            current->next->ownerThread = 0;
            current->next->next = s_taskFreeHead.next;
            s_taskFreeHead.next = current->next;
            current->next = next;
        }
    }
#endif
}

void __cdecl TaskManager2_FreeAllPendingTasksForController(int localControllerIndex)
{
#ifdef KISAK_DW_TASK
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    369,
                    0,
                    "%s\n\t%s",
                    "Sys_IsMainThread()",
                    "TaskManager2 is not threadsafe.\n") )
    {
        __debugbreak();
    }
    TaskManager2_FreeAllPendingTasksInternal(localControllerIndex);
#endif
}

void __cdecl TaskManager2_FreeAllPendingTasksInternal(int localControllerIndex)
{
#ifdef KISAK_DW_TASK
    TaskRecord *next; // [esp+20h] [ebp-Ch]
    bool freeTask; // [esp+27h] [ebp-5h]
    TaskRecord *current; // [esp+28h] [ebp-4h]

    current = &s_taskActiveHead;
    while ( current->next )
    {
        next = current->next->next;
        if ( !current->next->definition
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                        322,
                        0,
                        "%s",
                        "(*current)->definition") )
        {
            __debugbreak();
        }
        freeTask = 0;
        if ( localControllerIndex == -1 )
        {
            if ( TaskManager2_TaskIsPending(current->next)
                && ((current->next->definition->category & 2) != 0 || (current->next->definition->category & 8) != 0) )
            {
                freeTask = 1;
            }
        }
        else if ( current->next->controllerIndex == localControllerIndex
                     && TaskManager2_TaskIsPending(current->next)
                     && ((current->next->definition->category & 2) != 0 || (current->next->definition->category & 8) != 0) )
        {
            freeTask = 1;
        }
        if ( freeTask )
        {
            if ( current->next->definition->failure_callback )
                current->next->definition->failure_callback(current->next);
            if ( current->next->definition->payloadSize && current->next->payload )
            {
                Task_Deallocate((_BYTE *)current->next->payload, current->next->definition->payloadSize);
            }
            else if ( current->next->definition->payloadSize )
            {
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                                352,
                                0,
                                "%s",
                                "(*current)->definition->payloadSize == 0") )
                    __debugbreak();
            }
            current->next->payload = 0;
            current->next->definition = 0;
            current->next->next = s_taskFreeHead.next;
            s_taskFreeHead.next = current->next;
            current->next = next;
        }
        else
        {
            current = current->next;
        }
    }
#endif
}

void __cdecl TaskManager2_HandleTimedOutTask(TaskRecord *TimedOutTask)
{
#ifdef KISAK_DW_TASK
    unsigned intv1; // eax
    bdReference<bdRemoteTask> dwtask; // [esp+24h] [ebp-4h] BYREF

    v1 = Sys_Milliseconds();
    Com_Printf(32, "Task %s timed out after %dms\n", TimedOutTask->definition->name, v1 - TimedOutTask->startMS);
    if ( TimedOutTask->definition->failure_callback )
        TimedOutTask->definition->failure_callback(TimedOutTask);
    if ( (TimedOutTask->definition->category & 2) != 0 )
    {
        if ( !TimedOutTask->remoteTask.m_ptr
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                        384,
                        0,
                        "%s",
                        "TimedOutTask->remoteTask") )
        {
            __debugbreak();
        }
        dwtask.m_ptr = TimedOutTask->remoteTask.m_ptr;
        if ( dwtask.m_ptr )
            InterlockedIncrement(&dwtask.m_ptr->m_refCount);
        bdRemoteTask::cancelTask(dwtask.m_ptr);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&dwtask);
    }
    TimedOutTask->state = TASK_STATE_FAILED;
#endif
}

void __cdecl TaskManager2_ProcessDemonwareTask(TaskRecord *task)
{
#ifdef KISAK_DW_TASK
    enum bdLobbyErrorCode v1; // eax
    const char *errorStr; // [esp+84h] [ebp-10h]
    enum bdLobbyErrorCode errorCode; // [esp+88h] [ebp-Ch]
    const TaskDefinition *def; // [esp+8Ch] [ebp-8h]
    int ms; // [esp+90h] [ebp-4h]

    if ( !task && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp", 599, 0, "%s", "task") )
        __debugbreak();
    if ( !task->definition
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    600,
                    0,
                    "%s",
                    "task->definition") )
    {
        __debugbreak();
    }
    if ( task->state == TASK_STATE_INPROGRESS )
    {
        if ( !task->remoteTask.m_ptr
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                        609,
                        0,
                        "%s",
                        "task->remoteTask != NULL") )
        {
            __debugbreak();
        }
        def = task->definition;
        ms = Sys_Milliseconds() - task->startMS;
        switch ( bdRemoteTask::getStatus(task->remoteTask.m_ptr) )
        {
            case BD_PENDING:
                return;
            case BD:
                Com_Printf(
                    32,
                    "Demonware task '%s'(%d) for controller %d completed in %dms\n",
                    def->name,
                    task->startMS,
                    task->controllerIndex,
                    ms);
                task->state = task->isChildTask ? TASK_STATE_CHILDCOMPLETE : TASK_STATE_COMPLETED;
                if ( task->state == TASK_STATE_CHILDCOMPLETE )
                    Com_DPrintf(
                        32,
                        "Task %s's (startms %d) state set to %s\n",
                        task->definition->name,
                        task->startMS,
                        "TASK_STATE_CHILDCOMPLETE");
                else
                    Com_DPrintf(
                        32,
                        "Task %s's (startms %d) state set to %s\n",
                        task->definition->name,
                        task->startMS,
                        "TASK_STATE_COMPLETED");
                if ( task->ownerThread )
                {
                    bdReference<bdCommonAddr>::operator=(
                        (bdReference<bdCommonAddr> *)&g_DeferredTasks[g_taskEnd].remoteTask,
                        (const bdReference<bdCommonAddr> *)&task->remoteTask);
                    g_DeferredTasks[g_taskEnd].definition = task->definition;
                    g_DeferredTasks[g_taskEnd].payload = task->payload;
                    g_DeferredTasks[g_taskEnd--].ownerThread = task->ownerThread;
                    Com_DPrintf(32, "g_taskEnd is now %i\n", g_taskEnd);
                }
                else if ( def->completed_callback )
                {
                    def->completed_callback(task);
                }
                if ( !task->isChildTask )
                    bdReference<bdCommonAddr>::operator=(&task->remoteTask, 0);
                break;
            case BD_FAILED:
                errorCode = bdRemoteTask::getErrorCode(task->remoteTask.m_ptr);
                v1 = bdRemoteTask::getErrorCode(task->remoteTask.m_ptr);
                errorStr = dwLobbyErrorCodeToString(v1);
                Com_PrintError(
                    32,
                    "Demonware task '%s'(%d) for controller %d failed [%s (%d)] after %dms\n",
                    def->name,
                    task->startMS,
                    task->controllerIndex,
                    errorStr,
                    errorCode,
                    ms);
                task->state = task->isChildTask ? TASK_STATE_CHILDFAILED : TASK_STATE_FAILED;
                if ( task->state == TASK_STATE_CHILDFAILED )
                    Com_DPrintf(
                        32,
                        "Task %s(%d) state set to %s\n",
                        task->definition->name,
                        task->startMS,
                        "TASK_STATE_CHILDFAILED");
                else
                    Com_DPrintf(32, "Task %s(%d) state set to %s\n", task->definition->name, task->startMS, "TASK_STATE_FAILED");
                if ( task->ownerThread )
                {
                    bdReference<bdCommonAddr>::operator=(
                        (bdReference<bdCommonAddr> *)&g_DeferredTasks[g_taskEnd].remoteTask,
                        (const bdReference<bdCommonAddr> *)&task->remoteTask);
                    g_DeferredTasks[g_taskEnd].definition = task->definition;
                    g_DeferredTasks[g_taskEnd].payload = task->payload;
                    g_DeferredTasks[g_taskEnd--].ownerThread = task->ownerThread;
                    Com_DPrintf(32, "g_taskEnd is now %i\n", g_taskEnd);
                }
                else if ( def->failure_callback )
                {
                    def->failure_callback(task);
                }
                if ( !task->isChildTask )
                    bdReference<bdCommonAddr>::operator=(&task->remoteTask, 0);
                break;
            case BD_TIMED_OUT:
                Com_PrintError(
                    32,
                    "Demonware task '%s' for controller %d timed out after %dms\n",
                    def->name,
                    task->controllerIndex,
                    ms);
                task->state = task->isChildTask ? TASK_STATE_CHILDFAILED : TASK_STATE_FAILED;
                if ( task->state == TASK_STATE_CHILDFAILED )
                    Com_DPrintf(32, "Task %s's state set to %s\n", task->definition->name, "TASK_STATE_CHILDFAILED");
                else
                    Com_DPrintf(32, "Task %s's state set to %s\n", task->definition->name, "TASK_STATE_FAILED");
                if ( def->failure_callback )
                    def->failure_callback(task);
                if ( !task->isChildTask )
                    bdReference<bdCommonAddr>::operator=(&task->remoteTask, 0);
                break;
            case BD_CANCELLED:
                Com_Printf(
                    32,
                    "Demonware task '%s' for controller %d cancelled after %dms\n",
                    def->name,
                    task->controllerIndex,
                    ms);
                task->state = task->isChildTask ? TASK_STATE_CHILDFAILED : TASK_STATE_FAILED;
                if ( task->state == TASK_STATE_CHILDFAILED )
                    Com_DPrintf(32, "Task %s's state set to %s\n", task->definition->name, "TASK_STATE_CHILDFAILED");
                else
                    Com_DPrintf(32, "Task %s's state set to %s\n", task->definition->name, "TASK_STATE_FAILED");
                if ( def->failure_callback )
                    def->failure_callback(task);
                if ( !task->isChildTask )
                    bdReference<bdCommonAddr>::operator=(&task->remoteTask, 0);
                break;
            default:
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                                703,
                                0,
                                "unknown remote task status") )
                    __debugbreak();
                break;
        }
    }
    else if ( !task->isChildTask
                 && !Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                             604,
                             0,
                             "%s",
                             "task->isChildTask") )
    {
        __debugbreak();
    }
#endif
}

void __cdecl TaskManager2_ProcessNestedTask(TaskRecord *task)
{
#ifdef KISAK_DW_TASK
    const TaskDefinition *def; // [esp+1Ch] [ebp-8h]
    int ms; // [esp+20h] [ebp-4h]

    if ( !task && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp", 722, 0, "%s", "task") )
        __debugbreak();
    if ( !task->definition
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    723,
                    0,
                    "%s",
                    "task->definition") )
    {
        __debugbreak();
    }
    if ( task->state == TASK_STATE_INPROGRESS )
    {
        def = task->definition;
        ms = Sys_Milliseconds() - task->startMS;
        if ( task->nestedTask && task->nestedTask->state )
            task->state = task->nestedTask->state;
        else
            task->state = TASK_STATE_FAILED;
        switch ( task->state )
        {
            case TASK_STATE_INVALID:
                if ( task->definition )
                    Com_Printf(32, "Invalid task made it into process task - %s\n", task->definition->name);
                else
                    Com_Printf(32, "Invalid task made it into process task - %s\n", "null definition");
                break;
            case TASK_STATE_INIT:
            case TASK_STATE_INPROGRESS:
                return;
            case TASK_STATE_CHILDCOMPLETE:
            case TASK_STATE_COMPLETED:
                Com_Printf(
                    32,
                    "Nested task '%s'(startms %d) for controller %d completed in %dms\n",
                    def->name,
                    task->startMS,
                    task->controllerIndex,
                    ms);
                if ( def->completed_callback )
                    def->completed_callback(task);
                if ( task->isChildTask )
                {
                    Com_DPrintf(
                        32,
                        "Task %s (%d) has a parent, setting state to TASK_STATE_CHILDCOMPLETE\n",
                        task->definition->name,
                        task->startMS);
                    task->nestedTask->state = TASK_STATE_COMPLETED;
                    task->state = TASK_STATE_CHILDCOMPLETE;
                }
                else
                {
                    Com_DPrintf(
                        32,
                        "Task %s(%d) is an orphan, setting state to TASK_STATE_COMPLETED\n",
                        task->definition->name,
                        task->startMS);
                    task->nestedTask->state = TASK_STATE_COMPLETED;
                    task->state = TASK_STATE_COMPLETED;
                }
                if ( task->nestedTask->remoteTask.m_ptr )
                    bdReference<bdCommonAddr>::operator=(&task->nestedTask->remoteTask, 0);
                break;
            case TASK_STATE_CHILDFAILED:
            case TASK_STATE_FAILED:
                Com_Printf(
                    32,
                    "Nested task '%s'(%d) for controller %d failed in %dms\n",
                    def->name,
                    task->startMS,
                    task->controllerIndex,
                    ms);
                if ( def->failure_callback )
                    def->failure_callback(task);
                if ( task->isChildTask )
                {
                    Com_DPrintf(
                        32,
                        "Task %s(%d) has a parent, setting state to TASK_STATE_CHILDFAILED\n",
                        task->definition->name,
                        task->startMS);
                    if ( task->nestedTask && task->nestedTask->state )
                        task->nestedTask->state = TASK_STATE_FAILED;
                    task->state = TASK_STATE_CHILDFAILED;
                }
                else
                {
                    Com_DPrintf(
                        32,
                        "Task %s(%d) is an orphan, setting state to TASK_STATE_FAILED\n",
                        task->definition->name,
                        task->startMS);
                    if ( task->nestedTask && task->nestedTask->state )
                        task->nestedTask->state = TASK_STATE_FAILED;
                    task->state = TASK_STATE_FAILED;
                }
                break;
            default:
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                                805,
                                0,
                                "unknown nested remote task status") )
                    __debugbreak();
                break;
        }
    }
    else if ( !task->isChildTask
                 && !Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                             727,
                             0,
                             "%s",
                             "task->isChildTask") )
    {
        __debugbreak();
    }
#endif
}

void __cdecl TaskManager2_ProcessTasks(int localControllerIndex)
{
#ifdef KISAK_DW_TASK
    TaskRecord *task; // [esp+18h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    813,
                    0,
                    "%s\n\t%s",
                    "Sys_IsMainThread()",
                    "TaskManager2 is not threadsafe.\n") )
    {
        __debugbreak();
    }
    if ( localControllerIndex != -1 )
    {
        TaskManager2_CreateDeferredTasks();
        task = s_taskActiveHead.next;
        ++s_taskPollCount;
        while ( task )
        {
            if ( !task->definition
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                            829,
                            0,
                            "%s",
                            "task->definition != NULL") )
            {
                __debugbreak();
            }
            if ( task->lastPoll != s_taskPollCount && task->controllerIndex == localControllerIndex )
            {
                if ( (task->state < TASK_STATE_INPROGRESS || task->state > TASK_STATE_FAILED)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                                835,
                                0,
                                "%s",
                                "task->state >= TASK_STATE_INPROGRESS && task->state <= TASK_STATE_FAILED") )
                {
                    __debugbreak();
                }
                if ( (task->definition->category & 2) != 0 )
                    TaskManager2_ProcessDemonwareTask(task);
                if ( (task->definition->category & 4) != 0 )
                    TaskManager2_ProcessXOverlappedTask(task);
                if ( (task->definition->category & 8) != 0 )
                    TaskManager2_ProcessNestedTask(task);
                task->lastPoll = s_taskPollCount;
                task->lastPollMS = Sys_Milliseconds();
            }
            task = task->next;
        }
        TaskManager2_FreeDeadTasks(localControllerIndex);
    }
#endif
}

void TaskManager2_CreateDeferredTasks()
{
#ifdef KISAK_DW_TASK
    TaskRecord *newTask; // [esp+10h] [ebp-8h]
    int i; // [esp+14h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    244,
                    0,
                    "%s\n\t%s",
                    "Sys_IsMainThread()",
                    "This should only be called from the main thread!\n") )
    {
        __debugbreak();
    }
    if ( g_taskHead )
    {
        dwEnterDeferredCritsec();
        Com_DPrintf(32, "Starting %i deferred tasks\n", g_taskHead);
        for ( i = 0; i < g_taskHead; ++i )
        {
            newTask = TaskManager2_CreateTaskFromServerThread(g_DeferredTasks[i].definition, 0, 0, 0);
            bdReference<bdCommonAddr>::operator=(
                (bdReference<bdCommonAddr> *)&newTask->remoteTask,
                (const bdReference<bdCommonAddr> *)&g_DeferredTasks[i].remoteTask);
            newTask->payload = g_DeferredTasks[i].payload;
            TaskManager2_StartTask(newTask);
        }
        g_taskHead = 0;
        dwLeaveDeferredCritsec();
    }
#endif
}

TaskRecord *__cdecl TaskManager2_CreateTaskFromServerThread(
                const TaskDefinition *definition,
                int controllerIndex,
                TaskRecord *nestTask,
                int timeout)
{
#ifdef KISAK_DW_TASK
    int v5; // [esp+0h] [ebp-28h]
    TaskRecord *task; // [esp+24h] [ebp-4h]

    if ( !definition
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    184,
                    0,
                    "%s",
                    "definition != NULL") )
    {
        __debugbreak();
    }
    if ( !s_taskFreeHead.next
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    185,
                    0,
                    "%s",
                    "s_taskFreeHead.next != NULL") )
    {
        __debugbreak();
    }
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    186,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !TaskManager2_IsValidServerTask(definition)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    187,
                    0,
                    "%s",
                    "TaskManager2_IsValidServerTask(definition)") )
    {
        __debugbreak();
    }
    if ( (definition->category & 2) == 0 || dwGetLogOnStatus(controllerIndex) == 4 )
    {
        task = s_taskFreeHead.next;
        s_taskFreeHead.next = s_taskFreeHead.next->next;
        task->next = 0;
        task->definition = definition;
        task->startMS = Sys_Milliseconds();
        task->controllerIndex = controllerIndex;
        if ( definition->payloadSize <= 0 )
            task->payload = 0;
        else
            task->payload = (void *)Task_Allocate(definition->payloadSize);
        task->lastPoll = s_taskPollCount;
        task->lastPollMS = Sys_Milliseconds();
        task->state = TASK_STATE_INIT;
        task->isChildTask = 0;
        bdReference<bdCommonAddr>::operator=(&task->remoteTask, 0);
        if ( (definition->category & 2) != 0 )
        {
            if ( timeout == -1 )
            {
                task->timeOut = -1;
            }
            else
            {
                if ( timeout )
                    v5 = timeout;
                else
                    v5 = 300000;
                task->timeOut = v5;
            }
        }
        task->ownerThread = 1;
        Com_Printf(
            32,
            "Task '%s'(%d) created for local controller %d with timeout of %dms\n",
            task->definition->name,
            task->startMS,
            task->controllerIndex,
            task->timeOut);
        return task;
    }
    else
    {
        Com_PrintWarning(32, "Not starting task %s, not connected to DW\n", definition->name);
        return 0;
    }
#else
    return 0;
#endif
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
#ifdef KISAK_DW_TASK
    __int64 v2; // rax
    int remainingSize; // [esp+14h] [ebp-4h]
    int indexa; // [esp+20h] [ebp+8h]

    if ( SLODWORD(s_taskMemoryPool[index]) < blocks
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    65,
                    0,
                    "%s",
                    "ChunkSize( index ) >= blocks") )
    {
        __debugbreak();
    }
    if ( (HIDWORD(s_taskMemoryPool[index]) & 1) != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    66,
                    0,
                    "%s",
                    "ChunkIsFree( index )") )
    {
        __debugbreak();
    }
    remainingSize = LODWORD(s_taskMemoryPool[index]) - blocks;
    LODWORD(s_taskMemoryPool[index]) = blocks;
    HIDWORD(s_taskMemoryPool[index]) = blocks >> 31;
    indexa = ChunkNext(index);
    if ( remainingSize > 0 )
    {
        v2 = remainingSize - 1;
        LODWORD(s_taskMemoryPool[indexa]) = v2;
        HIDWORD(s_taskMemoryPool[indexa]) = HIDWORD(v2) | 1;
    }
    return indexa;
#else
    return 0;
#endif
}

bool __cdecl TaskManager2_IsValidServerTask(const TaskDefinition *definition)
{
#ifdef KISAK_DW_TASK
    unsigned int i; // [esp+0h] [ebp-18h]
    char *validservertasks[4]; // [esp+4h] [ebp-14h]
    bool retval; // [esp+17h] [ebp-1h]

    validservertasks[0] = "sendMessage";
    validservertasks[1] = "SV_WriteClientLeaderboards";
    validservertasks[2] = "SV_WriteClientFile";
    validservertasks[3] = "SV_ReadClientFile";
    retval = 0;
    for ( i = 0; i < 4; ++i )
    {
        if ( !I_strcmp(definition->name, validservertasks[i]) )
            return 1;
    }
    return retval;
#else
    return false;
#endif
}

TaskRecord *__cdecl TaskManager2_CreateTask(
                const TaskDefinition *definition,
                int controllerIndex,
                TaskRecord *nestTask,
                int timeout)
{
#ifdef KISAK_DW_TASK
    int v5; // [esp+8h] [ebp-34h]
    int v6; // [esp+14h] [ebp-28h]
    TaskRecord *task; // [esp+38h] [ebp-4h]

    if ( !definition
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    865,
                    0,
                    "%s",
                    "definition != NULL") )
    {
        __debugbreak();
    }
    if ( !s_taskFreeHead.next
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    866,
                    0,
                    "%s",
                    "s_taskFreeHead.next != NULL") )
    {
        __debugbreak();
    }
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    867,
                    0,
                    "%s\n\t%s",
                    "Sys_IsMainThread()",
                    "TaskManager2 is not threadsafe.\n") )
    {
        __debugbreak();
    }
    if ( (definition->category & 2) == 0 || dwGetLogOnStatus(controllerIndex) == 4 )
    {
        task = s_taskFreeHead.next;
        s_taskFreeHead.next = s_taskFreeHead.next->next;
        task->next = 0;
        task->definition = definition;
        task->startMS = Sys_Milliseconds();
        task->controllerIndex = controllerIndex;
        if ( definition->payloadSize <= 0 )
            task->payload = 0;
        else
            task->payload = (void *)Task_Allocate(definition->payloadSize);
        task->lastPoll = s_taskPollCount;
        task->lastPollMS = Sys_Milliseconds();
        task->state = TASK_STATE_INIT;
        task->isChildTask = 0;
        bdReference<bdCommonAddr>::operator=(&task->remoteTask, 0);
        if ( (definition->category & 2) != 0 )
        {
            if ( timeout == -1 )
            {
                task->timeOut = -1;
            }
            else
            {
                if ( timeout )
                    v6 = timeout;
                else
                    v6 = 300000;
                task->timeOut = v6;
            }
        }
        if ( (definition->category & 4) != 0 )
        {
            if ( timeout )
                v5 = timeout;
            else
                v5 = 300000;
            task->timeOut = v5;
        }
        if ( (definition->category & 8) != 0 )
        {
            task->nestedTask = nestTask;
            if ( task->nestedTask )
            {
                task->nestedTask->isChildTask = 1;
                Com_DPrintf(
                    32,
                    "Child task of %s(%d) set to %s(%d)\n",
                    task->definition->name,
                    task->startMS,
                    task->nestedTask->definition->name,
                    task->nestedTask->startMS);
            }
            Com_Printf(
                32,
                "NESTED Task '%s'(%d) created for local controller %d\n",
                task->definition->name,
                task->startMS,
                task->controllerIndex);
        }
        else
        {
            Com_Printf(
                32,
                "Task '%s'(%d) created for local controller %d with timeout of %dms\n",
                task->definition->name,
                task->startMS,
                task->controllerIndex,
                task->timeOut);
        }
        return task;
    }
    else
    {
        Com_PrintWarning(32, "Not starting task %s, not connected to DW\n", definition->name);
        return 0;
    }
#else
    return 0;
#endif
}

void __cdecl TaskManager2_StartTask(TaskRecord *task)
{
#ifdef KISAK_DW_TASK
    int v1; // eax

    if ( !task
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp", 943, 0, "%s", "task != NULL") )
    {
        __debugbreak();
    }
    if ( !task->definition
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    944,
                    0,
                    "%s",
                    "task->definition != NULL") )
    {
        __debugbreak();
    }
    if ( task->next
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    945,
                    0,
                    "%s",
                    "task->next == NULL") )
    {
        __debugbreak();
    }
    if ( task->state != TASK_STATE_INIT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    946,
                    0,
                    "%s",
                    "task->state == TASK_STATE_INIT") )
    {
        __debugbreak();
    }
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    947,
                    0,
                    "%s\n\t%s",
                    "Sys_IsMainThread()",
                    "TaskManager2 is not threadsafe.\n") )
    {
        __debugbreak();
    }
    task->state = TASK_STATE_INPROGRESS;
    task->next = s_taskActiveHead.next;
    s_taskActiveHead.next = task;
    v1 = TaskManager2_CountTasksInProgress(0);
    Com_Printf(
        32,
        "Task '%s' started for local controller %d (%d total tasks)\n",
        task->definition->name,
        task->controllerIndex,
        v1);
#endif
}

char __cdecl TaskManger2_TaskGetInProgressForControllerByName(const char *taskName, int controllerIndex)
{
#ifdef KISAK_DW_TASK
    TaskRecord *task; // [esp+0h] [ebp-4h]

    if ( !taskName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp", 1011, 0, "%s", "taskName") )
    {
        __debugbreak();
    }
    if ( !taskName || !*taskName )
        return 0;
    for ( task = s_taskActiveHead.next; task; task = task->next )
    {
        if ( !I_stricmp(task->definition->name, taskName) && task->controllerIndex == controllerIndex )
            return 1;
    }
    return 0;
#else
    return 0;
#endif
}

TaskRecord *__cdecl TaskManager2_TaskGetInProgressForController(const TaskDefinition *definition, int controllerIndex)
{
#ifdef KISAK_DW_TASK
    TaskRecord *task; // [esp+0h] [ebp-4h]

    if ( !definition
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    1035,
                    0,
                    "%s",
                    "definition != NULL") )
    {
        __debugbreak();
    }
    for ( task = s_taskActiveHead.next; task; task = task->next )
    {
        if ( TaskManager2_TaskIsPending(task) && task->definition == definition && task->controllerIndex == controllerIndex )
            return task;
    }
    return 0;
#else
    return 0;
#endif
}

TaskRecord *__cdecl TaskManager2_TaskGetInProgress(const TaskDefinition *definition)
{
#ifdef KISAK_DW_TASK
    TaskRecord *task; // [esp+0h] [ebp-4h]

    if ( !definition
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                    1054,
                    0,
                    "%s",
                    "definition != NULL") )
    {
        __debugbreak();
    }
    for ( task = s_taskActiveHead.next; task; task = task->next )
    {
        if ( TaskManager2_TaskIsPending(task) && task->definition == definition )
            return task;
    }
    return 0;
#else
    return 0;
#endif
}

bool __cdecl TaskManager2_TaskIsInProgressForController(const TaskDefinition *definition, int controllerIndex)
{
#ifdef KISAK_DW_TASK
    return TaskManager2_TaskGetInProgressForController(definition, controllerIndex) != 0;
#else
    return false;
#endif
}

bool __cdecl TaskManager2_TaskIsInProgress(const TaskDefinition *definition)
{
#ifdef KISAK_DW_TASK
    return TaskManager2_TaskGetInProgress(definition) != 0;
#else
    return false;
#endif
}

int __cdecl TaskManager2_CountTasksInProgress(const TaskDefinition *definition)
{
#ifdef KISAK_DW_TASK
    TaskRecord *task; // [esp+0h] [ebp-8h]
    int count; // [esp+4h] [ebp-4h]

    count = 0;
    for ( task = s_taskActiveHead.next; task; task = task->next )
    {
        if ( TaskManager2_TaskIsPending(task) && (task->definition == definition || !definition) )
            ++count;
    }
    return count;
#else
    return 0;
#endif
}

int __cdecl TaskManager2_CountTasksInProgressForController(int controllerindex)
{
#ifdef KISAK_DW_TASK
    TaskRecord *task; // [esp+0h] [ebp-8h]
    int count; // [esp+4h] [ebp-4h]

    count = 0;
    for ( task = s_taskActiveHead.next; task; task = task->next )
    {
        if ( TaskManager2_TaskIsPending(task) && controllerindex == task->controllerIndex )
            ++count;
    }
    return count;
#else
    return 0;
#endif
}

bool __cdecl TaskManager2_TaskIsPending(const TaskRecord *task)
{
#ifdef KISAK_DW_TASK
    return task->state == TASK_STATE_INPROGRESS
            || task->state == TASK_STATE_CHILDCOMPLETE
            || task->state == TASK_STATE_CHILDFAILED;
#else
    return false;
#endif
}

bool __cdecl TaskManager2_TaskIsTimedOut(const TaskRecord *task)
{
#ifdef KISAK_DW_TASK
    bool v3; // [esp+4h] [ebp-18h]
    bool v4; // [esp+8h] [ebp-14h]

    v4 = (task->definition->category & 8) == 0 && task->timeOut != -1;
    v3 = v4 && Sys_Milliseconds() - task->lastPollMS < 0x2710;
    return v3 && (signed int)(Sys_Milliseconds() - task->startMS) > task->timeOut;
#else
    return false;
#endif
}

void __cdecl TaskManager2_DumpTasks()
{
#ifdef KISAK_DW_TASK
    TaskRecord *task; // [esp+0h] [ebp-4Ch]
    char taskState[68]; // [esp+4h] [ebp-48h] BYREF

    memset(taskState, 0, 64);
    task = s_taskActiveHead.next;
    Com_Printf(32, "g_TaskEnd: %i\t g_taskHead: %i\n", g_taskEnd, g_taskHead);
    Com_Printf(32, "Task name\tState\n");
    while ( task )
    {
        if ( task->definition )
        {
            TaskManager2_StateToString(task->state, taskState, 0x40u);
            Com_Printf(32, "%s(%d)\t%s\n", task->definition->name, task->startMS, taskState);
        }
        task = task->next;
    }
#endif
}

void __cdecl TaskManager2_StateToString(TaskState state, char *string, unsigned int stringsize)
{
#ifdef KISAK_DW_TASK
    if ( string )
    {
        memset(string, 0, stringsize);
        switch ( state )
        {
            case TASK_STATE_INVALID:
                Com_sprintf(string, stringsize, "TASK_STATE_INVALID");
                break;
            case TASK_STATE_INIT:
                Com_sprintf(string, stringsize, "TASK_STATE_INIT");
                break;
            case TASK_STATE_INPROGRESS:
                Com_sprintf(string, stringsize, "TASK_STATE_INPROGRESS");
                break;
            case TASK_STATE_CHILDCOMPLETE:
                Com_sprintf(string, stringsize, "TASK_STATE_CHILDCOMPLETE");
                break;
            case TASK_STATE_CHILDFAILED:
                Com_sprintf(string, stringsize, "TASK_STATE_CHILDFAILED");
                break;
            case TASK_STATE_COMPLETED:
                Com_sprintf(string, stringsize, "TASK_STATE_COMPLETED");
                break;
            case TASK_STATE_FAILED:
                Com_sprintf(string, stringsize, "TASK_STATE_FAILED");
                break;
            default:
                Com_sprintf(string, stringsize, "Unknown state!!");
                break;
        }
    }
    else if ( !Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_tasks.cpp",
                             1160,
                             0,
                             "You passed in a null string pointer to TaskManager2_StateToString.\n") )
    {
        __debugbreak();
    }
#endif
}

