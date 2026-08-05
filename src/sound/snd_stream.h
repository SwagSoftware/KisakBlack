#pragma once
#include "snd.h"
#include <database/db_load.h>
#include <tl/jobqueue/jobqueue_all.h>
#include "snd_driver_xaudio2.h"

#define SND_STREAM_COUNT 10

struct snd_buffer // sizeof=0x118
{
    char *data;
    volatile unsigned int reference_count;
    char filename[260];
    unsigned int offset_in_file;
    unsigned int data_size;
    unsigned int file_size;
};

struct snd_stream_file // sizeof=0x120
{                                       // XREF: .data:snd_stream_file * g_snd_stream_files/r
                                        // snd_pack_file/r
    char filename[260];
    int handle;                         // XREF: Snd_StreamFini(void)+14B/r
                                        // Snd_StreamFini(void)+174/w ...
    int age;                            // XREF: Snd_ReadBuffer+28/r
                                        // Snd_ReadBuffer+AE/r ...
    unsigned int size;
    bool shared;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 base_offset;
};

struct __declspec(align(8)) snd_pack_file // sizeof=0x140
{                                       // XREF: .data:snd_pack_file * g_snd_pack_files/r
    snd_stream_file file;
    PackIndex index;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(8)) snd_stream // sizeof=0x190
{
    char filename[260];
    snd_asset header;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    tlAtomicMutex mutex;
    char *window_return[3];
    unsigned int head;
    unsigned int read;
    unsigned int stream_length;
    snd_buffer *buffers[2];
    //int buffer_references[2];
    volatile unsigned int buffer_references[2];
    unsigned int prime_size;
    char *prime_data;
    //int windows_in_use;
    volatile unsigned int windows_in_use;
    bool looping;
    bool error;
    bool in_use;
    bool head_at_eof;
    bool have_header;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct snd_stream_request // sizeof=0x110
{                                       // XREF: ?Snd_StreamUpdate@@YAXXZ/r
    char filename[260];                 // XREF: Snd_StreamUpdate(void)+CC/r
                                        // Snd_StreamUpdate(void)+2DD/r ...
    unsigned int start_of_read;         // XREF: Snd_StreamUpdate(void)+E8/r
                                        // Snd_StreamUpdate(void)+15D/r ...
    int need;                           // XREF: Snd_StreamUpdate(void)+31C/r
                                        // Snd_StreamUpdate(void)+337/r
    snd_buffer *buffer;                 // XREF: Snd_StreamUpdate(void)+118/w
};

struct __declspec(align(8)) scoped_performance_error // sizeof=0x18
{                                       // XREF: ?Snd_StreamOpen@@YAXIPBD_NIPAD@Z/r
                                        // ?Snd_StreamClose@@YAXI@Z/r ...
    long double threshold;              // XREF: Snd_StreamOpen(uint,char const *,bool,uint,char *)+E/w
                                        // Snd_StreamClose(uint)+E/w ...
    unsigned __int64 start;             // XREF: Snd_StreamOpen(uint,char const *,bool,uint,char *)+1F/w
                                        // Snd_StreamOpen(uint,char const *,bool,uint,char *)+22/w ...
    const char *what;                   // XREF: Snd_StreamOpen(uint,char const *,bool,uint,char *)+13/w
                                        // Snd_StreamClose(uint)+13/w ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    void test(const char *__formal);
};

void __cdecl Snd_StreamBufferInit(snd_buffer *buffer, char *data);
void __cdecl Snd_StreamReset(snd_stream *stream);
void __cdecl Snd_StreamInit();
void __cdecl Snd_StreamFini();
void __cdecl SND_StreamCloseFiles();
void __cdecl Snd_StreamOpen(
                unsigned int index,
                char *filename,
                bool looping,
                unsigned int prime_size,
                char *prime_data);
void __cdecl Snd_StreamClose(unsigned int index);
snd_stream_status __cdecl Snd_StreamStatus(unsigned int index);
const snd_asset *__cdecl Snd_StreamGetHeader(unsigned int index);
void __cdecl Snd_StarvationError();
unsigned int __cdecl Snd_StreamGetFreeWindows(unsigned int index);
snd_stream_status __cdecl Snd_StreamAcquireWindow(
                unsigned int index,
                unsigned int *size,
                unsigned int *position,
                char **data);
bool __cdecl Snd_StreamIsInBufferStrict(snd_buffer *buffer, unsigned int offset_in_file);
bool __cdecl Snd_StreamIsInBuffer(
                snd_buffer *buffer,
                unsigned int offset_in_file,
                unsigned int stream_length,
                bool looping);
void __cdecl Snd_StreamGetWindow(
                snd_buffer *buffer,
                unsigned int offset_in_file,
                unsigned int stream_length,
                bool looping,
                unsigned int *size,
                char **data);
void __cdecl Snd_StreamReleaseWindow(unsigned int index, char *data);
void __cdecl Snd_StreamReleaseWindowWork(unsigned int index, char *data);
void __cdecl Snd_StreamReleaseProcess(unsigned int index);
void __cdecl Snd_StreamLoadHeader(snd_stream *s, char *data, const char *filename, unsigned int file_size);
void __cdecl Snd_StreamSetError(snd_stream *s, snd_stream_request *r);
char __cdecl Snd_LoadBuffer(unsigned int streamIndex, char *filename, unsigned int offset);
void __cdecl Snd_StreamUpdate();
void __cdecl Snd_StreamGetRequest(snd_stream *s, snd_stream_request *r);
bool __cdecl Snd_StreamSetRequest(snd_stream *s, snd_stream_request *r);
snd_buffer *__cdecl Snd_FindBuffer(const char *filename, unsigned int offset);
char __cdecl Snd_FileClose(snd_stream_file *file);
char __cdecl Snd_ReadBuffer(char *filename, unsigned int start_offset, unsigned int size, snd_buffer *buffer);
char __cdecl Snd_FileOpen(const char *filename, snd_stream_file *file);
char __cdecl Snd_FileRead(snd_stream_file *file, unsigned int offset, unsigned int size, unsigned __int8 *data);


