#pragma once
#include <universal/offline.h>

enum sv_apstate_t : __int32
{
    AP_START       = 0x0,
    AP_GETTINGDATA = 0x1,
    AP_GETONE      = 0x2,
    AP_CHOOSE      = 0x3,
    AP_SETTINGDATA = 0x4,
    AP_SLEEPING    = 0x5,               // XREF: .data:s_apstate/s
    AP_ERROR       = 0x6,
};
inline sv_apstate_t &operator++(sv_apstate_t &t)
{
    t = static_cast<sv_apstate_t>((static_cast<int>(t) + 1));
    return t;
}
inline sv_apstate_t operator++(sv_apstate_t &t, int)
{
    sv_apstate_t old = t;
    t = static_cast<sv_apstate_t>((static_cast<int>(t) + 1));
    return old;
}

void __cdecl SV_AP_DumpTable();
bool __cdecl SV_AP_ParseControlFile(unsigned __int8 *controlFile);
int __cdecl SV_AP_PlaylistFromDistribution();
int __cdecl comparePlaylists(unsigned int *p1, unsigned int *p2);
void __cdecl SV_AP_GetControlFileComplete();
int __cdecl SV_AP_GetControlFileFailure();
TaskRecord *__cdecl SV_AP_GetControlFile();
void __cdecl SV_AP_GetControlFileName(char *buf, int buflen);
void __cdecl SV_SetGroupCountsComplete();
void __cdecl SV_GetGroupCountsComplete();
void __cdecl SV_GroupsFailure(TaskRecord *task);
void SV_GroupError(const char *fmt, ...);
TaskRecord *__cdecl SV_GetGroupCounts();
void __cdecl SV_Groups_SetGroupMembership(bool full);
void __cdecl SV_Groups_ParseGeos(const char *geoblob);
void __cdecl SV_AP_Start();
void __cdecl SV_AP_Frame();
bool __cdecl SV_AP_ServerIsFull();
