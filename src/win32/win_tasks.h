#pragma once

#include <universal/offline.h>

void __cdecl TaskManager_ClearOverlappedTasks(overlappedTask *overlappedTasks);
overlappedTask *__cdecl TaskManager_GetOpenTaskSlot(overlappedTask *overlappedTasks, int controllerIndex, int type);
void __cdecl TaskManager_ClearTask(overlappedTask *task);
char __cdecl TaskManager_AnyTaskInProgress(overlappedTask *overlappedTasks);
char __cdecl TaskManager_TaskIsInProgress(overlappedTask *overlappedTasks, int type);
void *__cdecl TaskManager_GetTaskData(overlappedTask *overlappedTask);
