#include "snd_stream.h"
#include <qcommon/threads.h>
#include <universal/physicalmemory.h>
#include <tl/tl_system.h>

#include <Windows.h>
#include <qcommon/common.h>
#include <universal/com_files.h>

char *g_snd_stream_buffer;
snd_stream *g_snd_streams;
snd_buffer *g_snd_buffers;
volatile unsigned int g_snd_stream_time;
snd_stream_file g_snd_stream_files[10];
snd_pack_file g_snd_pack_files[8];

void __cdecl Snd_StreamBufferInit(snd_buffer *buffer, char *data)
{
    memset((unsigned __int8 *)buffer, 0, sizeof(snd_buffer));
    buffer->filename[0] = 0;
    buffer->data = data;
    buffer->reference_count = 0;
    buffer->file_size = 0;
}

void __cdecl Snd_StreamReset(snd_stream *stream)
{
    int Target[5]; // [esp+4h] [ebp-18h] BYREF

    //tlAtomicMutex::Lock(&stream->mutex);
    stream->mutex.Lock();

    stream->filename[0] = 0;
    for ( int i = 0; i < 3; ++i )
        stream->window_return[i] = 0;
    stream->head = 2096;
    stream->read = 0;
    stream->buffers[0] = 0;
    stream->buffers[1] = 0;
    stream->buffer_references[0] = 0;
    stream->buffer_references[1] = 0;
    stream->windows_in_use = 0;
    stream->looping = 0;
    stream->error = 0;
    stream->head_at_eof = 0;
    stream->in_use = 0;
    stream->have_header = 0;
    stream->stream_length = 0;

    stream->mutex.Unlock();
    //if ( !--stream->mutex.LockCount )
    //{
    //    Target[0] = 0;
    //    InterlockedExchange(Target, 0);
    //    stream->mutex.ThreadId = 0;
    //}
}

void __cdecl Snd_StreamInit()
{
    tlAtomicMutex *p_mutex; // [esp+0h] [ebp-Ch]

    iassert(Sys_IsMainThread());

    if (!g_snd_stream_buffer)
    {
        g_snd_stream_buffer = (char *)_PMem_Alloc(
            0xA3C000u,
            0x1000u,
            4u,
            1u,
            TRACK_SOUND_GLOBALS,
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
            247);
        g_snd_streams = (snd_stream *)_PMem_Alloc(
            0xFA0u,
            0x80u,
            4u,
            1u,
            TRACK_SOUND_GLOBALS,
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
            254);
        g_snd_buffers = (snd_buffer *)_PMem_Alloc(
            0x15E0u,
            0x80u,
            4u,
            1u,
            TRACK_SOUND_GLOBALS,
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
            261);
        *(_DWORD *)g_snd_streams->filename = 0;
    }

    for ( int i = 0; i < 10; ++i )
    {
        p_mutex = &g_snd_streams[i].mutex;
        g_snd_streams[i].mutex.ThisPtr = p_mutex;
        p_mutex->ThreadId = 0;
        p_mutex->LockCount = 0;
        Snd_StreamReset(&g_snd_streams[i]);
    }

    for ( int j = 0; j < 20; ++j )
        Snd_StreamBufferInit(&g_snd_buffers[j], &g_snd_stream_buffer[536576 * j]);

    g_snd_stream_time = 0;
    memset((unsigned __int8 *)g_snd_stream_files, 0, sizeof(g_snd_stream_files));
    memset((unsigned __int8 *)g_snd_pack_files, 0, sizeof(g_snd_pack_files));
}

void __cdecl Snd_StreamFini()
{
    tlAtomicMutex *p_mutex; // [esp+0h] [ebp-10h]
    unsigned int k; // [esp+4h] [ebp-Ch]
    unsigned int j; // [esp+8h] [ebp-8h]
    unsigned int i; // [esp+Ch] [ebp-4h]

    if ( g_snd_streams )
    {
        for ( i = 0; i < 10; ++i )
        {
            iassert(!g_snd_streams[i].in_use);
            p_mutex = &g_snd_streams[i].mutex;
            p_mutex->ThreadId = 0;
            p_mutex->ThisPtr = 0;
        }
        for ( j = 0; j < 20; ++j )
        {
            iassert(g_snd_buffers[i].reference_count == 0);
        }
        for ( k = 0; k < 0xA; ++k )
        {
            if ( g_snd_stream_files[k].handle )
            {
                Snd_FileClose(&g_snd_stream_files[k]);
                g_snd_stream_files[k].handle = 0;
            }
        }
    }
}

void __cdecl SND_StreamCloseFiles()
{
    tlAtomicMutex *p_mutex; // [esp+0h] [ebp-24h]
    int Target[5]; // [esp+4h] [ebp-20h] BYREF
    unsigned int k; // [esp+18h] [ebp-Ch]
    unsigned int j; // [esp+1Ch] [ebp-8h]
    unsigned int i; // [esp+20h] [ebp-4h]

    if ( SND_Active() )
    {
        for (i = 0; i < 10; ++i)
        {
            //tlAtomicMutex::Lock(&g_snd_streams[i].mutex);
            g_snd_streams[i].mutex.Lock();
        }
        for ( j = 0; j < 10; ++j )
        {
            if ( g_snd_stream_files[j].handle )
                Snd_FileClose(&g_snd_stream_files[j]);
        }
        for ( k = 0; k < 10; ++k )
        {
            p_mutex = &g_snd_streams[k].mutex;

            g_snd_streams[k].mutex.Unlock();
            //--g_snd_streams[k].mutex.LockCount;
            //if ( !p_mutex->LockCount )
            //{
            //    Target[0] = 0;
            //    InterlockedExchange(Target, 0);
            //    LODWORD(p_mutex->ThreadId) = 0;
            //    HIDWORD(p_mutex->ThreadId) = 0;
            //}
        }
    }
}

void __cdecl Snd_StreamOpen(
                unsigned int index,
                char *filename,
                bool looping,
                unsigned int prime_size,
                char *prime_data)
{
    tlAtomicMutex *p_mutex; // [esp+Ch] [ebp-38h]
    int Target[5]; // [esp+10h] [ebp-34h] BYREF
    scoped_performance_error pe; // [esp+24h] [ebp-20h] BYREF
    snd_stream *s; // [esp+40h] [ebp-4h]

    pe.threshold = 0.0003f;
    pe.what = "Snd_StreamOpen";
    pe.start = tlPcGetTick().QuadPart;
    if ( !g_snd_streams
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp", 349, 0, "%s", "g_snd_streams") )
    {
        __debugbreak();
    }
    if ( index >= 0xA
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                    351,
                    0,
                    "%s",
                    "index < SND_MAX_STREAM_VOICES") )
    {
        __debugbreak();
    }
    s = &g_snd_streams[index];
    if ( s->in_use
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp", 356, 0, "%s", "!s->in_use") )
    {
        __debugbreak();
    }
    if ( s->head != 2096
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                    357,
                    0,
                    "%s",
                    "s->head == SND_STREAM_HEADER_SIZE") )
    {
        __debugbreak();
    }
    if ( s->buffers[0]
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp", 358, 0, "%s", "s->buffers[0] == 0") )
    {
        __debugbreak();
    }
    if ( s->buffers[1]
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp", 359, 0, "%s", "s->buffers[1] == 0") )
    {
        __debugbreak();
    }
    if ( s->error
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp", 360, 0, "%s", "s->error == false") )
    {
        __debugbreak();
    }
    if ( (!prime_size || !prime_data)
        && (prime_data || prime_size)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                    361,
                    0,
                    "%s",
                    "(prime_size && prime_data) || !(prime_data || prime_size)") )
    {
        __debugbreak();
    }
    //tlAtomicMutex::Lock(&s->mutex);
    s->mutex.Lock();
    strncpy(s->filename, filename, 0x104u);
    s->prime_size = prime_size;
    s->looping = looping;
    s->in_use = 1;
    s->prime_data = prime_data;
    if ( s->prime_size )
    {
        if ( s->prime_size <= 0x830
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                        383,
                        0,
                        "%s",
                        "s->prime_size > SND_STREAM_HEADER_SIZE") )
        {
            __debugbreak();
        }
        Snd_StreamLoadHeader(s, s->prime_data, filename, 0);
        if ( s->error )
        {
            s->error = 0;
            s->have_header = 0;
            s->prime_data = 0;
            s->prime_size = 0;
        }
    }
    Sys_WakeStream();
    p_mutex = &s->mutex;
    s->mutex.Unlock();
    //--s->mutex.LockCount;
    //if ( !p_mutex->LockCount )
    //{
    //    Target[0] = 0;
    //    InterlockedExchange(Target, 0);
    //    LODWORD(p_mutex->ThreadId) = 0;
    //    HIDWORD(p_mutex->ThreadId) = 0;
    //}
    //scoped_performance_error::test(&pe, "");
    pe.test("");
}

void scoped_performance_error::test(const char *__formal)
{
    tlPcGetTick();
}

void __cdecl Snd_StreamClose(unsigned int index)
{
    tlAtomicMutex *p_mutex; // [esp+Ch] [ebp-40h]
    int Target; // [esp+10h] [ebp-3Ch] BYREF
    volatile unsigned __int32 *v3; // [esp+14h] [ebp-38h]
    volatile unsigned __int32 *p_reference_count; // [esp+18h] [ebp-34h]
    scoped_performance_error pe; // [esp+2Ch] [ebp-20h] BYREF
    snd_stream *s; // [esp+48h] [ebp-4h]

    pe.threshold = 0.0003f;
    pe.what = "Snd_StreamClose";
    pe.start = tlPcGetTick().QuadPart;
    if ( index >= 0xA
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                    412,
                    0,
                    "%s",
                    "index < SND_STREAM_COUNT") )
    {
        __debugbreak();
    }
    s = &g_snd_streams[index];
    if ( !s->in_use
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp", 416, 0, "%s", "s->in_use") )
    {
        __debugbreak();
    }
    //tlAtomicMutex::Lock(&s->mutex);
    s->mutex.Lock();
    Snd_StreamReleaseProcess(index);
    if ( s->windows_in_use
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                    422,
                    0,
                    "%s",
                    "s->windows_in_use == 0") )
    {
        __debugbreak();
    }
    if ( s->buffers[0] )
    {
        p_reference_count = (volatile unsigned int*)&s->buffers[0]->reference_count;
        _InterlockedExchangeAdd(p_reference_count, 0xFFFFFFFF);
        if ( s->buffers[0]->reference_count > 0xAu
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                        427,
                        0,
                        "%s",
                        "s->buffers[0]->reference_count >= 0 && s->buffers[0]->reference_count <= SND_STREAM_COUNT") )
        {
            __debugbreak();
        }
        s->buffers[0] = 0;
    }
    if ( s->buffers[1] )
    {
        v3 = (volatile unsigned int *)&s->buffers[1]->reference_count;
        _InterlockedExchangeAdd(v3, 0xFFFFFFFF);
        if ( s->buffers[1]->reference_count > 0xAu
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                        434,
                        0,
                        "%s",
                        "s->buffers[1]->reference_count >= 0 && s->buffers[1]->reference_count <= SND_STREAM_COUNT") )
        {
            __debugbreak();
        }
        s->buffers[1] = 0;
    }
    Snd_StreamReset(s);
    p_mutex = &s->mutex;
    //--s->mutex.LockCount;
    //if ( !p_mutex->LockCount )
    //{
    //    Target = 0;
    //    InterlockedExchange(&Target, 0);
    //    LODWORD(p_mutex->ThreadId) = 0;
    //    HIDWORD(p_mutex->ThreadId) = 0;
    //}
    s->mutex.Unlock();
    //scoped_performance_error::test(&pe, "");
    pe.test("");
}

snd_stream_status __cdecl Snd_StreamStatus(unsigned int index)
{
    tlAtomicMutex *p_mutex; // [esp+Ch] [ebp-3Ch]
    int Target[5]; // [esp+10h] [ebp-38h] BYREF
    snd_stream_status v4; // [esp+24h] [ebp-24h]
    scoped_performance_error pe; // [esp+28h] [ebp-20h] BYREF
    snd_stream_status status; // [esp+40h] [ebp-8h]
    snd_stream *s; // [esp+44h] [ebp-4h]

    pe.threshold = 0.0003f;
    pe.what = "Snd_StreamStatus";
    pe.start = tlPcGetTick().QuadPart;
    if ( index >= 0xA
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                    451,
                    0,
                    "%s",
                    "index < SND_STREAM_COUNT") )
    {
        __debugbreak();
    }
    s = &g_snd_streams[index];
    status = SND_STREAM_OK;
    //tlAtomicMutex::Lock(&s->mutex);
    s->mutex.Lock();
    if ( s->in_use )
    {
        if ( s->error )
        {
            status = SND_STREAM_ERROR;
        }
        else if ( s->head_at_eof )
        {
            status = SND_STREAM_EOF;
        }
        else if ( !s->buffers[0] && !s->buffers[1] && (!s->prime_data || s->head >= s->prime_size) )
        {
            status = SND_STREAM_STARVING;
        }
    }
    else
    {
        status = SND_STREAM_UNUSED;
    }
    p_mutex = &s->mutex;
    s->mutex.Unlock();
    //--s->mutex.LockCount;
    //if ( !p_mutex->LockCount )
    //{
    //    Target[0] = 0;
    //    InterlockedExchange(Target, 0);
    //    LODWORD(p_mutex->ThreadId) = 0;
    //    HIDWORD(p_mutex->ThreadId) = 0;
    //}
    v4 = status;
    //scoped_performance_error::test(&pe, "");
    pe.test("");
    return v4;
}

const snd_asset *__cdecl Snd_StreamGetHeader(unsigned int index)
{
    if ( index >= 0xA
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                    506,
                    0,
                    "%s",
                    "index < SND_STREAM_COUNT") )
    {
        __debugbreak();
    }
    if ( !g_snd_streams[index].in_use
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                    507,
                    0,
                    "%s",
                    "g_snd_streams[index].in_use") )
    {
        __debugbreak();
    }
    if ( g_snd_streams[index].have_header )
        return &g_snd_streams[index].header;
    else
        return 0;
}

void __cdecl Snd_StarvationError()
{
    Sys_WakeStream();
}

unsigned int __cdecl Snd_StreamGetFreeWindows(unsigned int index)
{
    tlAtomicMutex *p_mutex; // [esp+Ch] [ebp-3Ch]
    int Target[5]; // [esp+10h] [ebp-38h] BYREF
    unsigned int v4; // [esp+24h] [ebp-24h]
    scoped_performance_error pe; // [esp+28h] [ebp-20h] BYREF
    snd_stream *s; // [esp+40h] [ebp-8h]
    unsigned int count; // [esp+44h] [ebp-4h]

    pe.threshold = 0.00009999999747378752;
    pe.what = "Snd_StreamGetFreeWindows";
    pe.start = tlPcGetTick().QuadPart;
    if ( index >= 0xA
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                    589,
                    0,
                    "%s",
                    "index < SND_STREAM_COUNT") )
    {
        __debugbreak();
    }
    s = &g_snd_streams[index];
    if ( !s->in_use
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp", 593, 0, "%s", "s->in_use") )
    {
        __debugbreak();
    }

    //while ( !tlAtomicMutex::TryLock(&s->mutex) );
    while ( !s->mutex.TryLock() );

    Snd_StreamReleaseProcess(index);
    count = 3 - s->windows_in_use;
    p_mutex = &s->mutex;
    s->mutex.Unlock();
    //--s->mutex.LockCount;
    //if ( !p_mutex->LockCount )
    //{
    //    Target[0] = 0;
    //    InterlockedExchange(Target, 0);
    //    LODWORD(p_mutex->ThreadId) = 0;
    //    HIDWORD(p_mutex->ThreadId) = 0;
    //}
    v4 = count;
    //scoped_performance_error::test(&pe, "");
    pe.test("");
    return v4;
}

snd_stream_status __cdecl Snd_StreamAcquireWindow(
                unsigned int index,
                unsigned int *size,
                unsigned int *position,
                char **data)
{
    unsigned int v5; // [esp+0h] [ebp-7Ch]
    tlAtomicMutex *p_mutex; // [esp+1Ch] [ebp-60h]
    int Target[4]; // [esp+20h] [ebp-5Ch] BYREF
    signed int v8; // [esp+30h] [ebp-4Ch]
    snd_stream_status v9; // [esp+50h] [ebp-2Ch]
    unsigned int tail_size; // [esp+54h] [ebp-28h]
    unsigned int base_size; // [esp+58h] [ebp-24h]
    scoped_performance_error pe; // [esp+5Ch] [ebp-20h] BYREF
    snd_stream_status status; // [esp+74h] [ebp-8h]
    snd_stream *s; // [esp+78h] [ebp-4h]

    pe.threshold = 0.00009999999747378752;
    pe.what = "Snd_StreamAcquireWindow";
    pe.start = tlPcGetTick().QuadPart;
    if ( index >= 0xA
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                    615,
                    0,
                    "%s",
                    "index < SND_STREAM_COUNT") )
    {
        __debugbreak();
    }
    s = &g_snd_streams[index];
    if ( !s->in_use
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp", 619, 0, "%s", "s->in_use") )
    {
        __debugbreak();
    }
    status = SND_STREAM_OK;

    while ( !s->mutex.TryLock() );

    //scoped_performance_error::test(&pe, "lock");
    pe.test("lock");
    Snd_StreamReleaseProcess(index);
    *size = 0;
    *data = 0;
    *position = 0;
    if ( s->error )
    {
        status = SND_STREAM_ERROR;
    }
    else if ( s->head_at_eof )
    {
        status = SND_STREAM_EOF;
    }
    else if ( s->buffers[0] && Snd_StreamIsInBufferStrict(s->buffers[0], s->head) )
    {
        *position = s->head - 2096;
        Snd_StreamGetWindow(s->buffers[0], s->head, s->stream_length, s->looping, size, data);
        _InterlockedExchangeAdd(s->buffer_references, 1u);
        _InterlockedExchangeAdd(&s->windows_in_use, 1u);
        if ( s->windows_in_use >= 4u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                        648,
                        0,
                        "%s\n\t(s->windows_in_use) = %i",
                        "(s->windows_in_use <= 3 && s->windows_in_use >= 0)",
                        s->windows_in_use) )
        {
            __debugbreak();
        }
    }
    else if ( s->buffers[1] && Snd_StreamIsInBufferStrict(s->buffers[1], s->head) )
    {
        *position = s->head - 2096;
        Snd_StreamGetWindow(s->buffers[1], s->head, s->stream_length, s->looping, size, data);
        _InterlockedExchangeAdd(&s->buffer_references[1], 1u);
        _InterlockedExchangeAdd(&s->windows_in_use, 1u);
        if ( s->windows_in_use >= 4u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                        656,
                        0,
                        "%s\n\t(s->windows_in_use) = %i",
                        "(s->windows_in_use <= 3 && s->windows_in_use >= 0)",
                        s->windows_in_use) )
        {
            __debugbreak();
        }
    }
    else if ( s->buffers[0] && Snd_StreamIsInBuffer(s->buffers[0], s->head, s->stream_length, s->looping) )
    {
        *position = s->head - 2096;
        Snd_StreamGetWindow(s->buffers[0], s->head, s->stream_length, s->looping, size, data);
        _InterlockedExchangeAdd(s->buffer_references, 1u);
        _InterlockedExchangeAdd(&s->windows_in_use, 1u);
        if ( s->windows_in_use >= 4u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                        664,
                        0,
                        "%s\n\t(s->windows_in_use) = %i",
                        "(s->windows_in_use <= 3 && s->windows_in_use >= 0)",
                        s->windows_in_use) )
        {
            __debugbreak();
        }
    }
    else if ( s->buffers[1] && Snd_StreamIsInBuffer(s->buffers[1], s->head, s->stream_length, s->looping) )
    {
        *position = s->head - 2096;
        Snd_StreamGetWindow(s->buffers[1], s->head, s->stream_length, s->looping, size, data);
        _InterlockedExchangeAdd(&s->buffer_references[1], 1u);
        _InterlockedExchangeAdd(&s->windows_in_use, 1u);
        if ( s->windows_in_use >= 4u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                        672,
                        0,
                        "%s\n\t(s->windows_in_use) = %i",
                        "(s->windows_in_use <= 3 && s->windows_in_use >= 0)",
                        s->windows_in_use) )
        {
            __debugbreak();
        }
    }
    else if ( s->prime_data && s->head < s->prime_size )
    {
        v8 = s->prime_size - s->head;
        if ( v8 > 67072 )
            v5 = 67072;
        else
            v5 = v8;
        *size = v5;
        *data = &s->prime_data[s->head];
        _InterlockedExchangeAdd(&s->windows_in_use, 1u);
    }
    else
    {
        Snd_StarvationError();
        status = SND_STREAM_STARVING;
    }
    if ( !s->have_header
        && (s->buffers[0] || s->buffers[1])
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                    688,
                    0,
                    "%s",
                    "s->have_header || (s->buffers[0] == 0 && s->buffers[1] == 0)") )
    {
        __debugbreak();
    }
    if ( status == SND_STREAM_OK
        && !s->have_header
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp", 692, 0, "%s", "s->have_header") )
    {
        __debugbreak();
    }
    if ( s->have_header && s->buffers[0] )
    {
        if ( _stricmp(s->filename, s->buffers[0]->filename)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                        697,
                        0,
                        "%s",
                        "!stricmp(s->filename, s->buffers[0]->filename)") )
        {
            __debugbreak();
        }
        if ( s->buffers[0]
            && _stricmp(s->filename, s->buffers[0]->filename)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                        699,
                        0,
                        "%s",
                        "!s->buffers[0] || !stricmp(s->filename, s->buffers[0]->filename)") )
        {
            __debugbreak();
        }
        if ( s->looping )
        {
            if ( s->header.data_size + 2096 > 0x106000 )
            {
                if ( s->buffers[0]->file_size % 0x83000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                                711,
                                0,
                                "%s",
                                "s->buffers[0]->file_size % SND_STREAM_BUFFER_SIZE == 0") )
                {
                    __debugbreak();
                }
                base_size = s->header.data_size + 2096;
                tail_size = (536576 - base_size % 0x83000) % 0x83000;
                if ( tail_size + base_size != s->buffers[0]->file_size
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                                714,
                                0,
                                "%s",
                                "base_size+tail_size == s->buffers[0]->file_size") )
                {
                    __debugbreak();
                }
            }
            else if ( s->header.data_size + 2096 != s->buffers[0]->file_size
                         && !Assert_MyHandler(
                                     "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                                     707,
                                     0,
                                     "%s",
                                     "s->header.data_size+SND_STREAM_HEADER_SIZE == s->buffers[0]->file_size") )
            {
                __debugbreak();
            }
        }
        else if ( s->header.data_size + 2096 != s->buffers[0]->file_size
                     && !Assert_MyHandler(
                                 "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                                 703,
                                 0,
                                 "%s",
                                 "s->header.data_size+SND_STREAM_HEADER_SIZE == s->buffers[0]->file_size") )
        {
            __debugbreak();
        }
    }
    if ( *size )
    {
        s->head += *size;
        if ( s->head >= s->stream_length )
        {
            if ( s->looping )
            {
                if ( s->head > s->stream_length )
                {
                    *size -= s->head - s->stream_length;
                    s->head = s->stream_length;
                }
                s->head = s->head + 2096 - s->stream_length;
                Sys_WakeStream();
            }
            else
            {
                s->head_at_eof = 1;
            }
        }
        if ( s->head > s->stream_length
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                        748,
                        0,
                        "%s",
                        "s->head <= s->stream_length") )
        {
            __debugbreak();
        }
    }
    //scoped_performance_error::test(&pe, "data");
    pe.test("data");
    p_mutex = &s->mutex;
    //--s->mutex.LockCount;
    //if ( !p_mutex->LockCount )
    //{
    //    Target[0] = 0;
    //    InterlockedExchange(Target, 0);
    //    LODWORD(p_mutex->ThreadId) = 0;
    //    HIDWORD(p_mutex->ThreadId) = 0;
    //}
    p_mutex->Unlock();
    //scoped_performance_error::test(&pe, "unlock");
    pe.test("unlock");
    v9 = status;
    //scoped_performance_error::test(&pe, "");
    pe.test("");
    return v9;
}

bool __cdecl Snd_StreamIsInBufferStrict(snd_buffer *buffer, unsigned int offset_in_file)
{
    return buffer->offset_in_file <= offset_in_file && buffer->data_size + buffer->offset_in_file > offset_in_file;
}

bool __cdecl Snd_StreamIsInBuffer(
                snd_buffer *buffer,
                unsigned int offset_in_file,
                unsigned int stream_length,
                bool looping)
{
    unsigned int eos; // [esp+0h] [ebp-8h]
    bool result; // [esp+7h] [ebp-1h]

    result = Snd_StreamIsInBufferStrict(buffer, offset_in_file);
    if ( looping && !result && stream_length > 0x106000 )
    {
        eos = stream_length - buffer->offset_in_file;
        if ( eos < 0x83000 )
            return offset_in_file < 536576 - eos;
    }
    return result;
}

void __cdecl Snd_StreamGetWindow(
                snd_buffer *buffer,
                unsigned int offset_in_file,
                unsigned int stream_length,
                bool looping,
                unsigned int *size,
                char **data)
{
    unsigned int offset_in_buffer; // [esp+4h] [ebp-4h]

    if ( !Snd_StreamIsInBuffer(buffer, offset_in_file, stream_length, looping)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                    556,
                    0,
                    "%s",
                    "Snd_StreamIsInBuffer(buffer, offset_in_file, stream_length, looping)") )
    {
        __debugbreak();
    }
    if ( !size && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp", 557, 0, "%s", "size") )
        __debugbreak();
    if ( !data && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp", 558, 0, "%s", "data") )
        __debugbreak();
    if ( Snd_StreamIsInBufferStrict(buffer, offset_in_file) )
    {
        offset_in_buffer = offset_in_file - buffer->offset_in_file;
        *data = &buffer->data[offset_in_buffer];
        *size = buffer->data_size - offset_in_buffer;
    }
    else
    {
        if ( !looping
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp", 571, 0, "%s", "looping") )
        {
            __debugbreak();
        }
        *data = &buffer->data[stream_length - buffer->offset_in_file - 2096 + offset_in_file];
        if ( *data >= buffer->data + 536576
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                        574,
                        0,
                        "%s",
                        "*data < buffer->data +SND_STREAM_BUFFER_SIZE") )
        {
            __debugbreak();
        }
        *size = buffer->data + 536576 - *data;
    }
    if ( *size > 0x10600 )
        *size = 67072;
}

void __cdecl Snd_StreamReleaseWindow(unsigned int index, char *data)
{
    unsigned int i; // [esp+4h] [ebp-4h]

    if ( index >= 0xA
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                    768,
                    0,
                    "%s",
                    "index < SND_STREAM_COUNT") )
    {
        __debugbreak();
    }
    for ( i = 0; i < 3; ++i )
    {
        if ( !_InterlockedCompareExchange(
                        (volatile unsigned __int32 *)&g_snd_streams[index].window_return[i],
                        (signed __int32)data,
                        0) )
        {
            Sys_WakeStream();
            return;
        }
    }
    Com_PrintError(9, "ran out of window return buffers\n");
}

void __cdecl Snd_StreamReleaseWindowWork(unsigned int index, char *data)
{
    unsigned int i; // [esp+4h] [ebp-Ch]
    bool valid; // [esp+Bh] [ebp-5h]
    snd_stream *s; // [esp+Ch] [ebp-4h]

    if ( index >= 0xA
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                    785,
                    0,
                    "%s",
                    "index < SND_STREAM_COUNT") )
    {
        __debugbreak();
    }
    s = &g_snd_streams[index];
    if ( s->in_use )
    {
        if ( s->buffer_references[0] >= 4u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                        793,
                        0,
                        "%s\n\t(s->buffer_references[0]) = %i",
                        "(s->buffer_references[0] <= 3 && s->buffer_references[0] >= 0)",
                        s->buffer_references[0]) )
        {
            __debugbreak();
        }
        if ( s->buffer_references[1] >= 4u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                        794,
                        0,
                        "%s\n\t(s->buffer_references[1]) = %i",
                        "(s->buffer_references[1] <= 3 && s->buffer_references[1] >= 0)",
                        s->buffer_references[1]) )
        {
            __debugbreak();
        }
        valid = 0;
        if ( data <= s->prime_data || data >= &s->prime_data[s->prime_size] )
        {
            for ( i = 0; i < 2; ++i )
            {
                if ( s->buffers[i] && data >= s->buffers[i]->data && data - s->buffers[i]->data < 536576 )
                {
                    _InterlockedExchangeAdd(&s->buffer_references[i], 0xFFFFFFFF);
                    _InterlockedExchangeAdd(&s->windows_in_use, 0xFFFFFFFF);
                    if ( s->windows_in_use >= 4u
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                                    819,
                                    0,
                                    "%s\n\t(s->windows_in_use) = %i",
                                    "(s->windows_in_use <= 3 && s->windows_in_use >= 0)",
                                    s->windows_in_use) )
                    {
                        __debugbreak();
                    }
                    if ( !s->buffer_references[i]
                        && s->stream_length > 0x106000
                        && !Snd_StreamIsInBufferStrict(s->buffers[i], s->head) )
                    {
                        _InterlockedExchangeAdd(&s->buffers[i]->reference_count, 0xFFFFFFFF);
                        if ( s->buffers[i]->reference_count > 0xAu
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                                        826,
                                        0,
                                        "%s",
                                        "s->buffers[i]->reference_count >= 0 && s->buffers[i]->reference_count <= SND_STREAM_COUNT") )
                        {
                            __debugbreak();
                        }
                        s->buffers[i] = 0;
                        Sys_WakeStream();
                    }
                    valid = 1;
                    break;
                }
            }
            if ( s->buffer_references[0] >= 4u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                            836,
                            0,
                            "%s\n\t(s->buffer_references[0]) = %i",
                            "(s->buffer_references[0] <= 3 && s->buffer_references[0] >= 0)",
                            s->buffer_references[0]) )
            {
                __debugbreak();
            }
            if ( s->buffer_references[1] >= 4u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                            837,
                            0,
                            "%s\n\t(s->buffer_references[1]) = %i",
                            "(s->buffer_references[1] <= 3 && s->buffer_references[1] >= 0)",
                            s->buffer_references[1]) )
            {
                __debugbreak();
            }
            if ( !valid
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp", 839, 0, "%s", "valid") )
            {
                __debugbreak();
            }
        }
        else
        {
            _InterlockedExchangeAdd(&s->windows_in_use, 0xFFFFFFFF);
            Sys_WakeStream();
        }
    }
}

void __cdecl Snd_StreamReleaseProcess(unsigned int index)
{
    char *data; // [esp+0h] [ebp-8h]
    unsigned int i; // [esp+4h] [ebp-4h]

    if ( index >= 0xA
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                    846,
                    0,
                    "%s",
                    "index < SND_STREAM_COUNT") )
    {
        __debugbreak();
    }
    for ( i = 0; i < 3; ++i )
    {
        data = g_snd_streams[index].window_return[i];
        if ( data )
        {
            g_snd_streams[index].window_return[i] = 0;
            Snd_StreamReleaseWindowWork(index, data);
        }
    }
}

void __cdecl Snd_StreamLoadHeader(snd_stream *s, char *data, const char *filename, unsigned int file_size)
{
    unsigned int size; // [esp+8h] [ebp-4h]

    if ( s->have_header
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp", 957, 0, "%s", "!s->have_header") )
    {
        __debugbreak();
    }
    memcpy(&s->header, data, sizeof(s->header));
    s->header.seek_table = (unsigned int *)(data + 56);
    s->have_header = 1;
    if ( s->looping == ((s->header.flags & 1) != 0) )
    {
        if ( s->header.version == 1 )
        {
            if ( s->header.header_size != 2096
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                            977,
                            0,
                            "%s",
                            "SND_STREAM_HEADER_SIZE == s->header.header_size") )
            {
                __debugbreak();
            }
            size = s->header.data_size + 2096;
            if ( s->looping && s->header.data_size > 0x106000 )
                size += (536576 - size % 0x83000) % 0x83000;
            if ( !file_size || size == file_size || s->error )
            {
                s->stream_length = s->header.data_size + 2096;
            }
            else
            {
                Com_PrintError(9, "### Invalid file (incorrect length) %s\n", filename);
                s->error = 1;
            }
        }
        else
        {
            Com_PrintError(9, "### Invalid file (bad header) %s\n", filename);
            s->error = 1;
        }
    }
    else
    {
        Com_PrintError(9, "### Invalid file (incorrect loop) %s\n", filename);
        s->error = 1;
    }
}

void __cdecl Snd_StreamSetError(snd_stream *s, snd_stream_request *r)
{
    unsigned int read; // [esp+0h] [ebp-20h]
    volatile unsigned int Target[5]; // [esp+8h] [ebp-18h] BYREF
    bool same_offset; // [esp+1Eh] [ebp-2h]
    bool same_file; // [esp+1Fh] [ebp-1h]

    //tlAtomicMutex::Lock(&s->mutex);
    s->mutex.Lock();
    if ( r->buffer
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp", 1090, 0, "%s", "!r->buffer") )
    {
        __debugbreak();
    }
    same_file = _stricmp(r->filename, s->filename) == 0;
    if ( s->read < s->stream_length )
        read = s->read;
    else
        read = 0;
    same_offset = r->start_of_read == read;
    if ( same_file && same_offset )
        s->error = 1;

    s->mutex.Unlock();
    //if ( !--s->mutex.LockCount )
    //{
    //    Target[0] = 0;
    //    InterlockedExchange(Target, 0);
    //    s->mutex.ThreadId = 0;
    //}
}

char __cdecl Snd_LoadBuffer(unsigned int streamIndex, char *filename, unsigned int offset)
{
    snd_buffer *b; // [esp+8h] [ebp-10h]
    unsigned int i; // [esp+Ch] [ebp-Ch]
    snd_buffer *free_buffer; // [esp+10h] [ebp-8h]
    unsigned int largest_file_size; // [esp+14h] [ebp-4h]

    free_buffer = 0;
    largest_file_size = 0;
    for ( i = 0; i < 0x14; ++i )
    {
        b = &g_snd_buffers[i];
        if ( !b->reference_count )
        {
            if ( !b->filename[0] )
            {
                free_buffer = &g_snd_buffers[i];
                largest_file_size = -1;
            }
            if ( largest_file_size < b->file_size )
            {
                free_buffer = &g_snd_buffers[i];
                largest_file_size = b->file_size;
            }
        }
    }
    if ( free_buffer )
        return Snd_ReadBuffer(filename, offset, 0x83000u, free_buffer);
    Com_PrintError(9, "### Could not find a free streaming buffer\n");
    return 0;
}

void __cdecl Snd_StreamUpdate()
{
    unsigned int j; // [esp+4h] [ebp-AC4h]
    snd_stream *s; // [esp+8h] [ebp-AC0h]
    unsigned int i; // [esp+Ch] [ebp-ABCh]
    signed int greatest_need_index; // [esp+10h] [ebp-AB8h]
    int request_count; // [esp+14h] [ebp-AB4h]
    int greatest_need; // [esp+18h] [ebp-AB0h]
    unsigned int have_work; // [esp+1Ch] [ebp-AACh]
    snd_stream_request requests[10]; // [esp+20h] [ebp-AA8h] BYREF

    while ( g_snd.init )
    {
        have_work = 0;
        memset((unsigned __int8 *)requests, 0, sizeof(requests));
        for ( i = 0; i < 0xA; ++i )
        {
            s = &g_snd_streams[i];
            Snd_StreamGetRequest(s, &requests[i]);
            if ( requests[i].filename[0] )
            {
                requests[i].buffer = Snd_FindBuffer(requests[i].filename, requests[i].start_of_read);
                if ( requests[i].buffer )
                {
                    if ( !Snd_StreamSetRequest(s, &requests[i]) )
                    {
                        _InterlockedExchangeAdd(&requests[i].buffer->reference_count, 0xFFFFFFFF);
                        if ( requests[i].buffer->reference_count > 0xAu
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_stream.cpp",
                                        1200,
                                        0,
                                        "%s",
                                        "requests[i].buffer->reference_count >= 0 && requests[i].buffer->reference_count <= SND_STREAM_COUNT") )
                        {
                            __debugbreak();
                        }
                    }
                    memset((unsigned __int8 *)&requests[i], 0, sizeof(snd_stream_request));
                }
                else
                {
                    ++have_work;
                }
            }
        }
        if ( !have_work )
            break;
        greatest_need = -1;
        greatest_need_index = -1;
        request_count = 0;
        for ( j = 0; j < 0xA; ++j )
        {
            if ( requests[j].filename[0] )
            {
                ++request_count;
                if ( requests[j].need > greatest_need )
                {
                    greatest_need = requests[j].need;
                    greatest_need_index = j;
                }
            }
        }

        iassert(greatest_need_index >= 0);

        if ( !Snd_LoadBuffer(
                        greatest_need_index,
                        requests[greatest_need_index].filename,
                        requests[greatest_need_index].start_of_read) )
            Snd_StreamSetError(&g_snd_streams[greatest_need_index], &requests[greatest_need_index]);
    }
}

void __cdecl Snd_StreamGetRequest(snd_stream *s, snd_stream_request *r)
{
    unsigned int v2; // [esp+0h] [ebp-70h]
    bool v3; // [esp+4h] [ebp-6Ch]
    bool v4; // [esp+8h] [ebp-68h]
    unsigned int read; // [esp+Ch] [ebp-64h]
    bool v6; // [esp+10h] [ebp-60h]
    bool v7; // [esp+14h] [ebp-5Ch]
    bool v8; // [esp+18h] [ebp-58h]
    int Target[8]; // [esp+20h] [ebp-50h] BYREF
    scoped_performance_error pe; // [esp+40h] [ebp-30h] BYREF
    int b1need; // [esp+5Ch] [ebp-14h]
    int b0need; // [esp+60h] [ebp-10h]
    bool need; // [esp+65h] [ebp-Bh]
    bool loaded1; // [esp+66h] [ebp-Ah]
    bool valid; // [esp+67h] [ebp-9h]
    unsigned int offset; // [esp+68h] [ebp-8h]
    bool active; // [esp+6Eh] [ebp-2h]
    bool loaded0; // [esp+6Fh] [ebp-1h]

    //tlAtomicMutex::Lock(&s->mutex);
    s->mutex.Lock();
    Snd_StreamReleaseProcess(s - g_snd_streams);
    v8 = s->in_use && !s->error;
    active = v8;
    v7 = !s->buffers[0] || !s->buffers[1];
    need = v7;
    v6 = s->read < s->stream_length || !s->stream_length || s->looping;
    valid = v6;
    if ( s->read < s->stream_length )
        read = s->read;
    else
        read = 0;
    offset = read;
    v4 = s->buffers[0] && s->buffers[0]->offset_in_file == offset;
    loaded0 = v4;
    v3 = s->buffers[1] && s->buffers[1]->offset_in_file == offset;
    loaded1 = v3;
    if ( active && need && valid && !loaded0 && !loaded1 )
    {
        pe.threshold = 0.0003f;
        pe.what = "Snd_StreamSetRequest_if1";
        pe.start = tlPcGetTick().QuadPart;
        strncpy((char *)r, (char *)s, 0x104u);
        if ( s->read < s->stream_length )
            v2 = s->read;
        else
            v2 = 0;
        r->start_of_read = v2;
        r->buffer = 0;
        r->need = 1;
        b0need = 1;
        if ( s->buffers[0] && s->head > s->buffers[0]->offset_in_file )
            b0need = s->head - s->buffers[0]->offset_in_file;
        if ( b0need > r->need )
            r->need = b0need;
        b1need = 1;
        if ( s->buffers[1] )
            b1need = s->head - s->buffers[1]->offset_in_file;
        if ( b1need > r->need )
            r->need = b1need;
        if ( !s->buffers[0] && !s->buffers[1] && s->prime_data )
            r->need = 1073152;
        //scoped_performance_error::test(&pe, "");
        pe.test("");
    }
    //if ( !--s->mutex.LockCount )
    //{
    //    Target[0] = 0;
    //    InterlockedExchange(Target, 0);
    //    s->mutex.ThreadId = 0;
    //}
    s->mutex.Unlock();
}

bool __cdecl Snd_StreamSetRequest(snd_stream *s, snd_stream_request *r)
{
    unsigned int read; // [esp+0h] [ebp-70h]
    int Target[8]; // [esp+14h] [ebp-5Ch] BYREF
    bool v5; // [esp+37h] [ebp-39h]
    scoped_performance_error v6; // [esp+38h] [ebp-38h] BYREF
    scoped_performance_error pe; // [esp+50h] [ebp-20h] BYREF
    bool same_offset; // [esp+6Dh] [ebp-3h]
    bool used_buffer; // [esp+6Eh] [ebp-2h]
    bool same_file; // [esp+6Fh] [ebp-1h]

    pe.threshold = 0.0003f;
    pe.what = "Snd_StreamSetRequest";
    pe.start = tlPcGetTick().QuadPart;
    //tlAtomicMutex::Lock(&s->mutex);
    s->mutex.Lock();

    iassert(r->buffer);

    used_buffer = 0;
    same_file = _stricmp(r->buffer->filename, s->filename) == 0;
    if ( s->read < s->stream_length )
        read = s->read;
    else
        read = 0;
    same_offset = r->start_of_read == read;
    if ( same_file && same_offset )
    {
        used_buffer = 1;
        v6.threshold = 0.0003f;
        v6.what = "Snd_StreamSetRequest_if1";
        v6.start = tlPcGetTick().QuadPart;

        iassert(s->in_use);
        iassert(!s->buffers[0] || !s->buffers[1]);

        if ( s->buffers[0] )
            s->buffers[1] = r->buffer;
        else
            s->buffers[0] = r->buffer;
        s->read = r->buffer->data_size + r->buffer->offset_in_file;
        if ( !s->have_header && !s->prime_size )
            Snd_StreamLoadHeader(s, r->buffer->data, r->buffer->filename, r->buffer->file_size);
        //scoped_performance_error::test(&v6, "");
        v6.test("");
    }
    //if ( !--s->mutex.LockCount )
    //{
    //    Target[0] = 0;
    //    InterlockedExchange(Target, 0);
    //    s->mutex.ThreadId = 0;
    //}
    s->mutex.Unlock();
    v5 = used_buffer;
    //scoped_performance_error::test(&pe, "");
    pe.test("");
    return v5;
}

snd_buffer *__cdecl Snd_FindBuffer(const char *filename, unsigned int offset)
{
    snd_buffer *buffer; // [esp+20h] [ebp-20h]
    unsigned int i; // [esp+24h] [ebp-1Ch]
    scoped_performance_error pe; // [esp+28h] [ebp-18h] BYREF

    pe.threshold = 0.0003f;
    pe.what = "Snd_FindBuffer";
    pe.start = tlPcGetTick().QuadPart;
    for ( int i = 0; ; ++i )
    {
        if ( i >= 0x14 )
        {
            //scoped_performance_error::test(&pe, "");
            pe.test("");
            return 0;
        }
        buffer = &g_snd_buffers[i];
        if ( !_stricmp(filename, buffer->filename) && buffer->offset_in_file == offset )
            break;
    }

    iassert(buffer->file_size > 0);
    _InterlockedExchangeAdd(&buffer->reference_count, 1u);
    iassert(buffer->reference_count >= 0 && buffer->reference_count <= SND_STREAM_COUNT);

    //scoped_performance_error::test(&pe, "");
    pe.test("");
    return buffer;
}

char __cdecl Snd_FileClose(snd_stream_file *file)
{
    if ( !file->shared )
        FS_FCloseFile(file->handle);
    file->handle = 0;
    file->filename[0] = 0;
    file->age = 0;
    file->shared = 0;
    file->base_offset = 0;
    return 1;
}

char __cdecl Snd_ReadBuffer(char *filename, unsigned int start_offset, unsigned int size, snd_buffer *buffer)
{
    signed __int32 v5; // eax
    unsigned int v6; // [esp+0h] [ebp-138h]
    int j; // [esp+10h] [ebp-128h]
    snd_stream_file *c; // [esp+14h] [ebp-124h]
    char os_filename[260]; // [esp+18h] [ebp-120h] BYREF
    unsigned int i; // [esp+120h] [ebp-18h]
    unsigned int read_size; // [esp+124h] [ebp-14h]
    int oldest_age; // [esp+128h] [ebp-10h]
    snd_stream_file *file; // [esp+12Ch] [ebp-Ch]
    int oldest; // [esp+130h] [ebp-8h]
    snd_stream_file *free_file; // [esp+134h] [ebp-4h]

    free_file = 0;
    file = 0;
    oldest = 0;
    oldest_age = g_snd_stream_files[0].age;
    for ( i = 0; i < 0xA; ++i )
    {
        if ( g_snd_stream_files[i].handle )
        {
            if ( !_stricmp(g_snd_stream_files[i].filename, filename) )
            {
                file = &g_snd_stream_files[i];
                break;
            }
        }
        else
        {
            free_file = &g_snd_stream_files[i];
        }
        if ( g_snd_stream_files[i].age < oldest_age )
        {
            oldest = i;
            oldest_age = g_snd_stream_files[i].age;
        }
    }
    if ( !file && !free_file )
    {
        free_file = &g_snd_stream_files[oldest];
        Snd_FileClose(free_file);
    }
    if ( file )
        goto LABEL_31;
    I_strncpyz(free_file->filename, filename, 260);
    for ( c = free_file; c->filename[0]; c = (snd_stream_file *)((char *)c + 1) )
        c->filename[0] = tolower(c->filename[0]);
    for ( j = 0; !file && j < 2; ++j )
    {
        if ( !j )
        {
            j = 1;
LABEL_25:
            Com_sprintf(os_filename, 0x100u, "%s", filename);
            goto LABEL_26;
        }
        if ( j == 1 )
            goto LABEL_25;
LABEL_26:
        if ( Snd_FileOpen(os_filename, free_file) )
            file = free_file;
    }
    if ( !file )
    {
        Com_PrintError(9, "### Could not open streaming sound %s\n", os_filename);
        return 0;
    }
LABEL_31:
    if ( (int)(file->size - start_offset) <= 536576 )
        v6 = file->size - start_offset;
    else
        v6 = 536576;
    read_size = v6;
    iassert(read_size);
    if ( !Snd_FileRead(file, start_offset, read_size, (unsigned __int8 *)buffer->data) )
        return 0;
    v5 = _InterlockedExchangeAdd(&g_snd_stream_time, 1u);
    file->age = v5 + 1;
    strncpy((char*)buffer->filename, (char *)filename, 0x104u);
    buffer->offset_in_file = start_offset;
    buffer->data_size = read_size;
    buffer->file_size = file->size;
    return 1;
}

char __cdecl Snd_FileOpen(const char *filename, snd_stream_file *file)
{
    file->shared = 0;
    file->base_offset = 0;
    file->handle = 0;
    FS_FOpenFileRead(filename, &file->handle);
    if ( !file->handle )
        return 0;
    file->size = FS_filelength(file->handle);
    return 1;
}

char __cdecl Snd_FileRead(snd_stream_file *file, unsigned int offset, unsigned int size, unsigned __int8 *data)
{
    DWORD LastError; // eax

    FS_Seek(file->handle, offset + LODWORD(file->base_offset), 2);
    if (FS_Read(data, size, file->handle) == size)
        return 1;
    LastError = GetLastError();
    Com_PrintError(9, "### could not read file %s, error: %08.8X\n", file->filename, LastError);
    return 0;
}

