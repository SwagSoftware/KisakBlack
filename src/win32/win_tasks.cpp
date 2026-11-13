#include "win_tasks.h"

void __cdecl TaskManager_ClearOverlappedTasks(overlappedTask *overlappedTasks)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 32; ++i )
        TaskManager_ClearTask(&overlappedTasks[i]);
}

overlappedTask *__cdecl TaskManager_GetOpenTaskSlot(overlappedTask *overlappedTasks, int controllerIndex, int type)
{
    int i; // [esp+20h] [ebp-4h]
    int ia; // [esp+20h] [ebp-4h]

    for ( i = 0; i < 32; ++i )
    {
        if ( !overlappedTasks[i].active )
        {
            overlappedTasks[i].overlappedIO = 0;
            overlappedTasks[i].type = type;
            if ( controllerIndex )
            {
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_tasks.cpp",
                                55,
                                0,
                                "%s\n\t(controllerIndex) = %i",
                                "(controllerIndex >= 0 && controllerIndex < 1)",
                                controllerIndex) )
                    __debugbreak();
            }
            overlappedTasks[i].controllerIndex = (unsigned __int8)controllerIndex;
            overlappedTasks[i].active = 1;
            overlappedTasks[i].finalStatus = RemoteTask::TaskStatus::TASK_STATUS_EMPTY;
            return &overlappedTasks[i];
        }
    }
    for ( ia = 0; ia < 32; ++ia )
    {
        if ( !overlappedTasks[ia].active )
        {
            overlappedTasks[ia].overlappedIO = 0;
            overlappedTasks[ia].type = type;
            if ( controllerIndex
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_tasks.cpp",
                            75,
                            0,
                            "%s\n\t(controllerIndex) = %i",
                            "(controllerIndex >= 0 && controllerIndex < 1)",
                            controllerIndex) )
            {
                __debugbreak();
            }
            overlappedTasks[ia].controllerIndex = (unsigned __int8)controllerIndex;
            overlappedTasks[ia].active = 1;
            overlappedTasks[ia].finalStatus = RemoteTask::TaskStatus::TASK_STATUS_EMPTY;
            return &overlappedTasks[ia];
        }
    }
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_tasks.cpp", 84, 0, "Ran out of task slots!") )
        __debugbreak();
    return 0;
}

void __cdecl TaskManager_ClearTask(overlappedTask *task)
{
    if ( task )
    {
        task->overlappedIO = 0;
        task->finalStatus = RemoteTask::TaskStatus::TASK_STATUS_EMPTY;
        task->errorCode = LOBBY_NO_ERROR;
        task->type = 0;
        *(unsigned int *)&task->active = 0;
        task->controllerIndex = 0;
        task->data = 0;
        task->miscData = 0;
        task->startMS = 0.0;
    }
}

char __cdecl TaskManager_AnyTaskInProgress(overlappedTask *overlappedTasks)
{
    int taskIdx; // [esp+0h] [ebp-4h]

    for ( taskIdx = 0; taskIdx < 32; ++taskIdx )
    {
        if ( overlappedTasks[taskIdx].active )
            return 1;
    }
    return 0;
}

char __cdecl TaskManager_TaskIsInProgress(overlappedTask *overlappedTasks, int type)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 32; ++i )
    {
        if ( overlappedTasks[i].active && overlappedTasks[i].type == type )
            return 1;
    }
    return 0;
}

void *__cdecl TaskManager_GetTaskData(overlappedTask *overlappedTask)
{
    if ( !overlappedTask
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_tasks.cpp", 173, 0, "%s", "overlappedTask") )
    {
        __debugbreak();
    }
    if ( !overlappedTask->active
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_tasks.cpp",
                    174,
                    0,
                    "%s",
                    "overlappedTask->active") )
    {
        __debugbreak();
    }
    return overlappedTask->data;
}

