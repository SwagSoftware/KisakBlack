#include "statmonitor.h"
#include <gfx_d3d/r_dvars.h>
#include <game_mp/g_main_mp.h>
#include <win32/win_common.h>
#include <win32/win_shared.h>
#include <client_mp/cl_main_mp.h>

statmonitor_s stats[14];
int statCount;

void __cdecl StatMon_Warning(int type, int duration, char *materialName)
{
    if ( com_statmon->current.enabled && !G_ExitAfterToolComplete() )
    {
        if ( (unsigned int)type >= 0xE
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\statmonitor.cpp",
                        42,
                        0,
                        "type doesn't index ARRAY_COUNT( stats )\n\t%i not in [0, %i)",
                        type,
                        14) )
        {
            __debugbreak();
        }
        Sys_EnterCriticalSection(CRITSECT_STATMON);
        stats[type].endtime = duration + Sys_Milliseconds();
        if ( !stats[type].material && cls.rendererStarted )
            stats[type].material = Material_RegisterHandle(materialName, 1);
        if ( type >= statCount )
            statCount = type + 1;
        Sys_LeaveCriticalSection(CRITSECT_STATMON);
    }
}

void __cdecl StatMon_GetStatsArray(const statmonitor_s **array, int *count)
{
    *array = stats;
    *count = statCount;
}

void __cdecl StatMon_Reset()
{
    memset(stats, 0, sizeof(stats));
    statCount = 0;
}

