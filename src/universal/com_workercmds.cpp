#include "com_workercmds.h"

#include <tl/jobqueue/jobqueue_all.h>
#include "assertive.h"
#include <cstring>
#include <gfx_d3d/r_foliage.h>
#include "tl/base/include/tl_system.h"
#include <qcommon/threads.h>
#include <Windows.h>
#include <gfx_d3d/r_workercmds.h>

bool g_WorkerCmdInit;
unsigned __int64 gMainWaitWorker;

void __cdecl Sys_WorkerCmdInit()
{
    g_WorkerCmdInit = 1;
}

unsigned __int8 *__cdecl jqCommitMemory(jqBatch *batch, unsigned __int8 *input, unsigned int dataSize)
{
    unsigned __int8 *dest; // [esp+0h] [ebp-4h]

    if ( !dataSize )
        return input;
    if ( dataSize > 0x54 )
    {
        dest = jqAllocBatchData(dataSize);
        if ( !dest
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_workercmds.cpp", 66, 0, "%s", "dest") )
        {
            __debugbreak();
        }
        memcpy(dest, input, dataSize);
    }
    else
    {
        dest = (unsigned __int8 *)&batch->ParamData[2];
        memcpy((unsigned __int8 *)&batch->ParamData[2], input, dataSize);
    }
    return dest;
}

void __cdecl jqSafeFlush(jqBatchGroup *group, unsigned __int64 batchCount)
{
    jqFlush(group, batchCount);
}

jqWorkerCmd *jqGetWorkercmdParam(jqBatch *batch)
{
    return *reinterpret_cast<jqWorkerCmd **> (batch->ParamData);
}

void *__cdecl jqLockData(jqBatch *batch)
{
    jqWorkerCmd *workerCmd = jqGetWorkercmdParam(batch);
    if (workerCmd->dataSize <= 84)
    {
        return &batch->ParamData[2];
    }
    iassert((batch->Input));
    return batch->Input;
}

void __cdecl jqUnlockData(jqBatch *batch)
{
    jqWorkerCmd *workerCmd = jqGetWorkercmdParam(batch);
    if (workerCmd->dataSize > 84)
    {
        jqFreeBatchData((jqAtomicHeap::LevelInfo*)batch->Input);
    }
}

void __cdecl Sys_AddWorkerCmdInternal(jqWorkerCmd *name, unsigned __int8 *data, WorkerCmdConditional *cond)
{
    jqBatch batch; // [esp+0h] [ebp-88h] BYREF

    batch.Input = jqCommitMemory(&batch, data, name->dataSize);
    batch.Module = name->module;
    batch.GroupID = 0;
    if (cond)
    {
        batch.ConditionalAddress = cond->address;
        batch.ConditionalValue = cond->value;
    }
    batch.ParamData[0] = (unsigned int)name;
    batch.ParamData[1] = _InterlockedExchangeAdd(&name->ppu_fence, 1u);
    jqAddBatch(&batch, name->queue);
}

jqBatch::jqBatch()
{
    this->p3x_info = 0;
    this->Input = 0;
    this->Output = 0;
    this->Module = 0;
    this->ConditionalAddress = 0;
    this->ConditionalValue = 0;
    this->GroupID = 0;
    //return this;
}

void __cdecl Sys_WaitWorkerCmdInternal(jqWorkerCmd *name)
{
    unsigned __int64 startTime; // [esp+8h] [ebp-8h]

    startTime = tlPcGetTick();
    jqSafeFlush(&name->module->Group, 0);
    if ( Sys_IsMainThread() )
        gMainWaitWorker += tlPcGetTick() - startTime;
}

void __cdecl Sys_AssistAndWaitWorkerCmdInternal(jqWorkerCmd *name)
{
    jqBatchGroup *GroupID; // [esp+4h] [ebp-4h]

    if ( name )
        GroupID = &name->module->Group;
    else
        GroupID = 0;
    if ( name )
        jqAssistWithBatches(
            (bool (__cdecl *)(void *))Sys_AssistAndWaitWorkerCmdInternalCallback,
            &name->module->Group,
            GroupID);
    else
        jqAssistWithBatches((bool (__cdecl *)(void *))Sys_AssistAndWaitWorkerCmdInternalCallback, 0, GroupID);
}

bool __cdecl Sys_AssistAndWaitWorkerCmdInternalCallback(jqBatchGroup *context)
{
    return !jqPoll(context);
}

void __cdecl Sys_AssistSingle()
{
    int count; // [esp+0h] [ebp-4h] BYREF

    count = 1;
    jqAssistWithBatches((bool (__cdecl *)(void *))Sys_AssistSingleCallback, &count, 0);
}

bool __cdecl Sys_AssistSingleCallback(unsigned int *context)
{
    int v1; // ecx

    v1 = (*context)--;
    return v1 == 0;
}

bool __cdecl Sys_AssistNeeded()
{
    return jqAreJobsQueued(0);
}

bool __cdecl R_FinishedFrontendWorkerCmds()
{
    if ( jqPoll(&fx_update_spotModule.Group) )
        return 0;
    if ( jqPoll(&fx_updateModule.Group) )
        return 0;
    if ( jqPoll(&fx_update_remainingModule.Group) )
        return 0;
    if ( jqPoll(&r_dpvs_staticModule.Group) )
        return 0;
    if ( jqPoll(&r_dpvs_sceneentModule.Group) )
        return 0;
    if ( jqPoll(&r_dpvs_dynmodelModule.Group) )
        return 0;
    if ( jqPoll(&r_dpvs_dynbrushModule.Group) )
        return 0;
    if ( jqPoll(&r_dpvs_entityModule.Group) )
        return 0;
    if ( jqPoll(&r_add_sceneentModule.Group) )
        return 0;
    if ( jqPoll(&r_spot_shadow_entModule.Group) )
        return 0;
    if ( jqPoll(&dobj_skelModule.Group) )
        return 0;
    if ( jqPoll(&dobj_skinModule.Group) )
        return 0;
    if ( jqPoll(&fx_drawModule.Group) )
        return 0;
    if ( jqPoll(&fx_marks_drawModule.Group) )
        return 0;
    if ( jqPoll(&r_model_skinModule.Group) )
        return 0;
    if ( jqPoll(&nuge_physicsModule.Group) )
        return 0;
    return !jqPoll(&r_skin_cached_staticmodelModule.Group);
}

void __cdecl R_WaitFrontendWorkerCmds()
{
    Sys_WaitWorkerCmdInternal(&fx_update_spotWorkerCmd);
    Sys_WaitWorkerCmdInternal(&fx_updateWorkerCmd);
    Sys_WaitWorkerCmdInternal(&fx_update_remainingWorkerCmd);
    Sys_WaitWorkerCmdInternal(&r_dpvs_staticWorkerCmd);
    Sys_WaitWorkerCmdInternal(&r_dpvs_sceneentWorkerCmd);
    Sys_WaitWorkerCmdInternal(&r_dpvs_dynmodelWorkerCmd);
    Sys_WaitWorkerCmdInternal(&r_dpvs_dynbrushWorkerCmd);
    Sys_WaitWorkerCmdInternal(&r_dpvs_entityWorkerCmd);
    Sys_WaitWorkerCmdInternal(&r_add_sceneentWorkerCmd);
    Sys_WaitWorkerCmdInternal(&r_spot_shadow_entWorkerCmd);
    Sys_WaitWorkerCmdInternal(&dobj_skelWorkerCmd);
    Sys_WaitWorkerCmdInternal(&dobj_skinWorkerCmd);
    Sys_WaitWorkerCmdInternal(&fx_drawWorkerCmd);
    Sys_WaitWorkerCmdInternal(&fx_marks_drawWorkerCmd);
    Sys_WaitWorkerCmdInternal(&r_model_skinWorkerCmd);
    Sys_WaitWorkerCmdInternal(&nuge_physicsWorkerCmd);
    Sys_WaitWorkerCmdInternal(&r_skin_cached_staticmodelWorkerCmd);
    R_DynSModelWaitWorker();
}

void __cdecl R_WaitWorkerCmds()
{
    jqSafeFlush(0, 0);
}

