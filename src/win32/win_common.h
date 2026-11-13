#pragma once
#include <universal/mem_userhunk.h>

enum CriticalSection : __int32
{                                                                             // XREF: Smp_CriticalSectionBlock/r
    CRITSECT_FX_UNIQUE_HANDLE                = 0x0,
    CRITSEC_SOUND_OCCLUSION                    = 0x1,
    CRITSECT_SOUND_COMMAND_ALLOC         = 0x2,
    CRITSECT_SOUND_COMMAND_PUSH            = 0x3,
    CRITSECT_SOUND_NOTIFY_ALLOC            = 0x4,
    CRITSECT_SOUND_NOTIFY_PUSH             = 0x5,
    CRITSECT_SOUND_PLAYBACK_ALLOC        = 0x6,
    CRITSECT_SOUND_BANK                            = 0x7,
    CRITSECT_SOUND_REMOTE                        = 0x8,
    CRITSECT_SOUND_LOOKUP_CACHE            = 0x9,
    CRITSECT_CONSOLE                                 = 0xA,
    CRITSECT_DEBUG_SOCKET                        = 0xB,
    CRITSECT_COM_ERROR                             = 0xC,
    CRITSECT_STATMON                                 = 0xD,
    CRITSECT_DEBUG_LINE                            = 0xE,
    CRTISECT_DEBUG_BRUSHES_AND_PATCHES = 0xF,
    CRITSECT_ALLOC_MARK                            = 0x10,
    CRITSECT_FREE_MARK                             = 0x11,
    CRITSECT_FX_PRIORITY_SEARCH            = 0x12,
    CRITSECT_FAKELAG                                 = 0x13,
    CRITSECT_CLIENT_MESSAGE                    = 0x14,
    CRITSECT_CLIENT_CMD                            = 0x15,
    CRITSECT_DOBJ_ALLOC                            = 0x16,
    CRITSECT_XANIM_ALLOC                         = 0x17,
    CRITSECT_KEY_BINDINGS                        = 0x18,
    CRITSECT_FX_VIS                                    = 0x19,
    CRITSECT_SERVER_MESSAGE                    = 0x1A,
    CRITSECT_SCRIPT_STRING                     = 0x1B,
    CRITSECT_MEMORY_TREE                         = 0x1C,
    CRITSECT_ASSERT                                    = 0x1D,
    CRITSECT_RD_BUFFER                             = 0x1E,
    CRITSECT_SYS_EVENT_QUEUE                 = 0x1F,
    CRITSECT_GPU_FENCE                             = 0x20,
    CRITSECT_FATAL_ERROR                         = 0x21,
    CRITSECT_DXDEVICE                                = 0x22,
    CRITSECT_DXDEVICE_GLOB                     = 0x23,
    CRITSECT_SCRIPT_DEBUGGER_ALLOC     = 0x24,
    CRITSECT_SCRIPT_DEBUGGER_SORT        = 0x25,
    CRITSECT_MISSING_ASSET                     = 0x26,
    CRITSECT_PHYSICS                                 = 0x27,
    CRITSECT_PHYSICS_UPDATE                    = 0x28,
    CRITSECT_PHYSICS_DESTRUCTIBLE_HIT = 0x29,
    CRITSECT_LIVE                                        = 0x2A,
    CRITSECT_AUDIO_PHYSICS                     = 0x2B,
    CRITSECT_CINEMATIC                             = 0x2C,
    CRITSECT_CINEMATIC_TARGET_CHANGE = 0x2D,
    CRITSECT_RB_TRANSFER                         = 0x2E,
    CRITSECT_STREAM_ASYNC_COMMAND        = 0x2F,
    CRITSECT_STREAM_FORCE_LOAD_COMMAND = 0x30,
    CRITSECT_STREAM_SYNC_COMMAND         = 0x31,
    CRITSECT_NETTHREAD_OVERRIDE            = 0x32,
    CRITSECT_ONLINE                             = 0x33,
    CRITSECT_DEFERRED_ONLINE                         = 0x34,
    CRITSECT_IK                                            = 0x35,
    CRITSECT_TL_MEMALLOC                         = 0x36,
    CRITSECT_VA_ALLOC                                = 0x37,
    CRITSECT_MEMTRACK                                = 0x38,
    CRITSECT_CBUF                                        = 0x39,
    CRITSECT_CURVEALLOC                            = 0x3A,
    CRITSEC_XAUDIO2_RELEASE_STREAM_BUFFER = 0x3B,
    CRITSECT_ZLIB                                        = 0x3C,
    CRITSECT_BLACKBOX                                = 0x3D,
    CRITSECT_BLACKBOX_NET                        = 0x3E,
    CRITSECT_GDT_COMMAND                         = 0x3F,
    CRITSECT_STRINGED_COMMAND                = 0x40,
    CRITSECT_RADIANT_SERVER_COMMAND    = 0x41,
    CRITSECT_RADIANT_CLIENT_COMMAND    = 0x42,
    CRITSECT_RECORDER                                = 0x43,
    CRITSECT_SERVERDEMO                            = 0x44,
    CRITSECT_TENSION                                 = 0x45,
    CRITSECT_IO_SCHEDULER                        = 0x46,
    CRITSECT_MEMFIRSTFIT                         = 0x47,
    CRITSECT_FXBEAM                                    = 0x48,
    CRITSECT_GLASS_ACTIONS                     = 0x49,
    CRITSECT_DBHASH                                    = 0x4A,
    CRITSECT_COUNT                                     = 0x4B,
};



struct FastCriticalSection // sizeof=0x8
{                                                                             // XREF: .data:g_dvarCritSect/r
        volatile long readCount;
        volatile long writeCount;                        // XREF: Dvar_FindMalleableVar_0:loc_7BD2E2/r
};

void __cdecl Sys_Mkdir(const char *path);
char *__cdecl Sys_Cwd();
const char *__cdecl Sys_DefaultCDPath();
char *__cdecl Sys_DefaultInstallPath();
HunkUser **__cdecl Sys_ListFiles(char *directory, char *extension, char *filter, int *numfiles, int wantsubs);
void __cdecl Sys_ListFilteredFiles(
                HunkUser *user,
                const char *basedir,
                const char *subdirs,
                const char *filter,
                char **list,
                int *numfiles);
bool __cdecl HasFileExtension(const char *name, const char *extension);
int __cdecl Sys_DirectoryHasContents(const char *directory);
void __cdecl Sys_InitializeCriticalSections();
void __cdecl Sys_EnterCriticalSection(CriticalSection critSect);
bool __cdecl Sys_TryEnterCriticalSection(CriticalSection critSect);
void __cdecl Sys_LeaveCriticalSection(CriticalSection critSect);
