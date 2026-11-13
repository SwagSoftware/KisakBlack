#pragma once
#include "q_shared.h"

struct TaskRecord;
struct RemoteTask;
struct TaskDefinition;

enum TaskState : __int32;

void __cdecl TaskManager2_Init();
void __cdecl TaskManager2_DeferTaskToMainThread(RemoteTask *remoteTask, const TaskDefinition *taskDef, void *payload);
void __cdecl TaskManager2_PickUpDeferredTasks();
void __cdecl TaskManager2_ProcessXOverlappedTask(TaskRecord *task);
void __cdecl TaskManager2_CancelEndlessTasks(int localControllerIndex);
void __cdecl Task_Deallocate(_BYTE *ptr, int bytes);
void __cdecl ChunkFree(int index, int blocks);
int __cdecl ChunkNext(int index);
void __cdecl TaskManager2_ComErrorCleanup();
void __cdecl TaskManager2_FreeDeadTasks(int localControllerIndex);
void __cdecl TaskManager2_FreeAllPendingTasksForController(int localControllerIndex);
void __cdecl TaskManager2_FreeAllPendingTasksInternal(int localControllerIndex);
void __cdecl TaskManager2_HandleTimedOutTask(TaskRecord *TimedOutTask);
void __cdecl TaskManager2_ProcessRemoteTask(TaskRecord *task);
void __cdecl TaskManager2_ProcessNestedTask(TaskRecord *task);
void __cdecl TaskManager2_ProcessTasks(int localControllerIndex);
void TaskManager2_CreateDeferredTasks();
TaskRecord *__cdecl TaskManager2_CreateTaskFromServerThread(
                const TaskDefinition *definition,
                int controllerIndex,
                TaskRecord *nestTask,
                int timeout);
int __cdecl Task_Allocate(int bytes);
int __cdecl ChunkAllocate(int index, int blocks);
bool __cdecl TaskManager2_IsValidServerTask(const TaskDefinition *definition);
TaskRecord *__cdecl TaskManager2_CreateTask(
                const TaskDefinition *definition,
                int controllerIndex,
                TaskRecord *nestTask,
                int timeout);
void __cdecl TaskManager2_StartTask(TaskRecord *task);
char __cdecl TaskManger2_TaskGetInProgressForControllerByName(const char *taskName, int controllerIndex);
TaskRecord *__cdecl TaskManager2_TaskGetInProgressForController(const TaskDefinition *definition, int controllerIndex);
TaskRecord *__cdecl TaskManager2_TaskGetInProgress(const TaskDefinition *definition);
bool __cdecl TaskManager2_TaskIsInProgressForController(const TaskDefinition *definition, int controllerIndex);
bool __cdecl TaskManager2_TaskIsInProgress(const TaskDefinition *definition);
int __cdecl TaskManager2_CountTasksInProgress(const TaskDefinition *definition);
int __cdecl TaskManager2_CountTasksInProgressForController(int controllerindex);
bool __cdecl TaskManager2_TaskIsPending(const TaskRecord *task);
bool __cdecl TaskManager2_TaskIsTimedOut(const TaskRecord *task);
void __cdecl TaskManager2_DumpTasks();
void __cdecl TaskManager2_StateToString(TaskState state, char *string, unsigned int stringsize);
