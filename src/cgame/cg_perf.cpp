#include "cg_perf.h"

#include <cstring>
#include <win32/win_shared.h>
#include <clientscript/cscr_vm.h>
#include <gfx_d3d/rb_backend.h>
#include <universal/timing.h>

CG_PerfInfo cg_perfInfo;

unsigned __int64 gRunFrameTicks;

void __cdecl CG_PerfInit()
{
    if ( !cg_perfInfo.initialized )
    {
        memset(&cg_perfInfo, 0, sizeof(cg_perfInfo));
        cg_perfInfo.frame.count = 32;
        cg_perfInfo.script.count = 10;
        cg_perfInfo.cscript.count = 10;
        cg_perfInfo.server.count = 20;
        cg_perfInfo.renderExec.count = 32;
        cg_perfInfo.renderSwap.count = 32;
        cg_perfInfo.initialized = 1;
    }
}

int previousMS;
void __cdecl CG_PerfUpdate()
{
    int frameMS; // [esp+10h] [ebp-8h]
    unsigned int currentMS; // [esp+14h] [ebp-4h]

    CG_PerfInit();
    currentMS = Sys_Milliseconds();
    frameMS = currentMS - previousMS;
    previousMS = currentMS;
    UpdateData(&cg_perfInfo.frame, frameMS);
    UpdateData(&cg_perfInfo.server, (int)((double)gRunFrameTicks * msecPerRawTimerTick));
    UpdateData(&cg_perfInfo.script, gScrExecuteTime[0]);
    UpdateData(&cg_perfInfo.cscript, gScrExecuteTime[1]);
    UpdateData(&cg_perfInfo.renderExec, rb_execCmdsMS);
    UpdateData(&cg_perfInfo.renderSwap, rb_swapMS);
}

void __cdecl UpdateData(CG_PerfData *data, int value)
{
    int v2; // eax
    int i; // [esp+10h] [ebp-4h]
    int ia; // [esp+10h] [ebp-4h]

    data->history[data->index % 32] = value;
    data->instant = value;
    data->min = 0x7FFFFFFF;
    data->max = 0;
    data->average = 0.0f;
    data->variance = 0.0f;
    data->total = 0;
    for ( i = 0; i < data->count; ++i )
    {
        v2 = (data->index - i) % 32;
        if ( v2 < 0 )
            break;
        data->total += data->history[v2];
        if ( data->min > data->history[v2] )
            data->min = data->history[v2];
        if ( data->max < data->history[v2] )
            data->max = data->history[v2];
    }
    data->average = (float)data->total / (float)data->count;
    for ( ia = 0; ia < data->count && (data->index - ia) % 32 >= 0; ++ia )
        data->variance = data->variance
                                     + fabs((float)data->history[(data->index - ia) % 32] - data->average);
    data->variance = data->variance / (float)data->count;
    ++data->index;
}

