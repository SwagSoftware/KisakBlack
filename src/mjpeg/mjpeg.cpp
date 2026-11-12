#include "mjpeg.h"

#include <Windows.h>
#include <qcommon/threads.h>
#include "avi.h"
#include "yuv.h"
#include <qcommon/common.h>
#include "tl/base/include/tl_system.h"
#include <gfx_d3d/r_dvars.h>

struct mjpeg_render // sizeof=0x0
{
};

mjpeg_render *mjpeg;
void(__cdecl *mjpeg_current_callback)(unsigned __int8 *, unsigned __int8 *, unsigned __int8 *);
HANDLE mjpeg_hEvent;
bool mjpeg_run_encoder;
bool mjpeg_quit_encoder;
bool mjpeg_inited;



void mjpeg_initonce()
{
    DWORD threadId; // [esp+0h] [ebp-4h] BYREF

    mjpeg_hEvent = CreateEventA(0, 0, 0, 0);
    CreateThread(0, 0x10000u, (LPTHREAD_START_ROUTINE)mjpeg_thread, 0, 0, &threadId);
    SetThreadName(threadId, "MJPEG Thread");
    mjpeg_inited = 1;
}

void __cdecl mjpeg_set_callback(void (__cdecl *callback)(unsigned __int8 *, unsigned __int8 *, unsigned __int8 *))
{
    mjpeg_current_callback = callback;
}

void __cdecl mjpeg_init()
{
    mjpeg_set_callback(mjpeg_add_frame);
}

bool __cdecl mjpeg_is_encoding()
{
    return mjpeg_run_encoder;
}

void __stdcall    mjpeg_thread(void *context)
{
    float diff; // [esp+30h] [ebp-34h]
    unsigned __int8 *u; // [esp+40h] [ebp-24h] BYREF
    unsigned __int8 *y; // [esp+44h] [ebp-20h] BYREF
    unsigned __int8 *v; // [esp+48h] [ebp-1Ch] BYREF
    int frames; // [esp+4Ch] [ebp-18h]
    int mjpeg_fps; // [esp+50h] [ebp-14h]
    unsigned __int64 tick; // [esp+54h] [ebp-10h]
    float fpsms; // [esp+60h] [ebp-4h]

    while ( 1 )
    {
        while ( !mjpeg_run_encoder )
            WaitForSingleObject(mjpeg_hEvent, 0xFFFFFFFF);
        tick = tlPcGetTick();
        mjpeg_fps = r_clipFPS->current.integer;
        fpsms = 1000.0 / (float)mjpeg_fps;
        for ( frames = 0; frames < 0x2000 && !mjpeg_quit_encoder; ++frames )
        {
            //diff = (double)(tlPcGetTick() - tick) / tlPcTicksPerMS;
            diff = (double)(tlPcGetTick() - tick) / tlPcTicksPerMS;
            if ( fpsms <= diff )
                Com_PrintWarning(16, "WARNING: Movie encoding took to %g ms limit is %g ms\n", diff, fpsms);
            else
                Sleep((__int64)(fpsms - diff));
            tick = tlPcGetTick();
            WaitForSingleObject(mjpeg_hEvent, 0xFFFFFFFF);
            yuv_get_buffers(&y, &u, &v);
            mjpeg_current_callback(y, u, v);
        }
        mjpeg_run_encoder = 0;
        mjpeg_close_int();
        mjpeg_quit_encoder = 0;
    }
}

void __cdecl mjpeg_close()
{
    if ( mjpeg_run_encoder )
    {
        mjpeg_quit_encoder = 1;
        while ( mjpeg_quit_encoder )
        {
            PulseEvent(mjpeg_hEvent);
            Sleep(0xAu);
        }
    }
}

void __cdecl mjpeg_draw()
{
    if ( mjpeg_run_encoder )
    {
        if ( yuv_encode_frame() )
            PulseEvent(mjpeg_hEvent);
    }
}

