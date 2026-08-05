#include "live_groups_dw.h"
#include <qcommon/cmd.h>

unsigned int s_groupCounts[1024];
GroupMembership s_groupMembership[1];
int s_groupSetCount;
unsigned int s_groupCount;
int s_groupUpdateTime;
struct bdTaskResult s_bdGroupCounts_0;

const char *s_onlineNames[4] =
{ "mp", "sp", "zombie", NULL };

GroupSet s_groupSets[3] =
{
  { "playlist", true, NULL, 64, 0u },
  { "online", true, s_onlineNames, 0, 0u },
  { NULL, false, NULL, 0, 0u }
};

cmd_function_s LiveGroups_JoinGroup_f_VAR;
cmd_function_s LiveGroups_LeaveGroup_f_VAR;
cmd_function_s LiveGroups_Dump_f_VAR;

void __cdecl LiveGroups_Init()
{
    int i; // [esp+0h] [ebp-8h]
    GroupSet *j; // [esp+4h] [ebp-4h]

    memset(s_groupMembership, 0, sizeof(s_groupMembership));
    s_groupSetCount = 0;
    s_groupCount = 0;
    s_groupUpdateTime = 0;
    for ( j = s_groupSets; j->name; ++j )
    {
        j->start = s_groupCount;
        if ( j->groupNames )
        {
            j->capacity = 0;
            for ( i = 0; j->groupNames[i]; ++i )
                ++j->capacity;
        }
        if ( j->capacity <= 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_groups_dw.cpp",
                        100,
                        0,
                        "%s",
                        "gs->capacity > 0") )
        {
            __debugbreak();
        }
        s_groupCount += j->capacity;
        ++s_groupSetCount;
    }
    Cmd_AddCommandInternal("joinLiveGroup", LiveGroups_JoinGroup_f, &LiveGroups_JoinGroup_f_VAR);
    Cmd_AddCommandInternal("leaveLiveGroup", LiveGroups_LeaveGroup_f, &LiveGroups_LeaveGroup_f_VAR);
    Cmd_AddCommandInternal("dumpLiveGroups", LiveGroups_Dump_f, &LiveGroups_Dump_f_VAR);
}

void __cdecl LiveGroups_SetGroupsComplete(TaskRecord *task)
{
    if ( task->controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_groups_dw.cpp",
                    113,
                    0,
                    "task->controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    task->controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( --s_groupMembership[task->controllerIndex].dirty < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_groups_dw.cpp",
                    117,
                    0,
                    "%s",
                    "s_groupMembership[task->controllerIndex].dirty >= 0") )
    {
        __debugbreak();
    }
}

void __cdecl LiveGroups_SetGroups(int localControllerIndex)
{
#ifdef KISAK_DEMON
    const bdReference<bdCommonAddr> *v1; // eax
    bdReference<bdCommonAddr> v2; // [esp+1Ch] [ebp-101Ch] BYREF
    unsigned int i; // [esp+20h] [ebp-1018h]
    TaskRecord *task; // [esp+24h] [ebp-1014h]
    unsigned int v5; // [esp+28h] [ebp-1010h]
    int v6; // [esp+2Ch] [ebp-100Ch]
    bdGroup *Group; // [esp+30h] [ebp-1008h]
    GroupMembership *v8; // [esp+34h] [ebp-1004h]
    unsigned int v9[1024]; // [esp+38h] [ebp-1000h] BYREF

    if ( localControllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_groups_dw.cpp",
                    122,
                    0,
                    "localControllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    localControllerIndex,
                    1) )
    {
        __debugbreak();
    }
    Group = dwGetGroup(localControllerIndex);
    v5 = 0;
    v8 = &s_groupMembership[localControllerIndex];
    v6 = _time64(0);
    if ( Group && v8->dirty > 0 && v6 - v8->updateTime > 60 )
    {
        task = TaskManager2_CreateTask(task_setGroups, localControllerIndex, 0, 0);
        v8->updateTime = v6;
        v8->dirty = 1;
        for ( i = 0; i < s_groupCount; ++i )
        {
            if ( (v8->bits[i >> 5] & (1 << (i & 0x1F))) != 0 )
                v9[v5++] = i;
        }
        v1 = (const bdReference<bdCommonAddr> *)bdGroup::setGroups(Group, (int)&v2, (int)v9, v5);
        bdReference<bdCommonAddr>::operator=((bdReference<bdCommonAddr> *)&task->remoteTask, v1);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v2);
        TaskManager2_StartTask(task);
    }
#endif
}

void __cdecl LiveGroups_GetGroupCountsComplete(TaskRecord *task)
{
#ifdef KISAK_DEMON
    unsigned int idx; // [esp+4h] [ebp-4h]

    memset(s_groupCounts, 0, sizeof(s_groupCounts));
    for ( idx = 0; idx < bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)task->remoteTask.m_ptr); ++idx )
        s_groupCounts[*((unsigned int *)&unk_A4E0D04 + 3 * idx)] = dword_A4E0D08[3 * idx];
#endif
}

void __cdecl LiveGroups_GetCounts(int localControllerIndex)
{
#ifdef KISAK_DEMON
    const bdReference<bdCommonAddr> *GroupCounts; // eax
    bdReference<bdCommonAddr> v2; // [esp+1Ch] [ebp-1014h] BYREF
    TaskRecord *task; // [esp+20h] [ebp-1010h]
    unsigned int i; // [esp+24h] [ebp-100Ch]
    int v5; // [esp+28h] [ebp-1008h]
    bdGroup *Group; // [esp+2Ch] [ebp-1004h]
    int v7[1024]; // [esp+30h] [ebp-1000h] BYREF

    Group = dwGetGroup(localControllerIndex);
    v5 = _time64(0);
    if ( Group && v5 - s_groupUpdateTime > 900 && !TaskManager2_CountTasksInProgress(task_getGroupCounts_0) )
    {
        task = TaskManager2_CreateTask(task_getGroupCounts_0, localControllerIndex, 0, 0);
        s_groupUpdateTime = v5;
        for ( i = 0; i < s_groupCount; ++i )
            v7[i] = i;
        v7[i++] = 490;
        v7[i++] = 491;
        v7[i++] = 492;
        GroupCounts = (const bdReference<bdCommonAddr> *)bdGroup::getGroupCounts(
                                                                                                             Group,
                                                                                                             (int)&v2,
                                                                                                             (int)v7,
                                                                                                             s_groupCount + 3,
                                                                                                             &s_bdGroupCounts_0,
                                                                                                             s_groupCount + 3);
        bdReference<bdCommonAddr>::operator=((bdReference<bdCommonAddr> *)&task->remoteTask, GroupCounts);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v2);
        TaskManager2_StartTask(task);
    }
#endif
}

char __cdecl LiveGroups_GetGroupID(char *path, int *offset, GroupSet **gs)
{
    char *v3; // eax
    int s; // [esp+10h] [ebp-14h]
    unsigned int setLen; // [esp+18h] [ebp-Ch]
    const char *groupName; // [esp+1Ch] [ebp-8h]
    int i; // [esp+20h] [ebp-4h]

    groupName = 0;
    v3 = strchr(path, 0x2Fu);
    if ( v3 )
    {
        setLen = (unsigned int)v3 - (unsigned int)path;
        groupName = (const char *)(v3 + 1);
        if ( !*(_BYTE *)(v3 + 1) )
            groupName = 0;
    }
    else
    {
        setLen = strlen(path);
    }
    for ( i = 0; ; ++i )
    {
        if ( i >= s_groupSetCount )
        {
            Com_PrintError(1, "Group '%s' not found\n", path);
            return 0;
        }
        if ( !I_strnicmp(path, s_groupSets[i].name, setLen) )
            break;
    }
    *gs = &s_groupSets[i];
    if ( groupName )
    {
        if ( (*gs)->groupNames )
        {
            for ( s = 0; s < (*gs)->capacity; ++s )
            {
                if ( !I_stricmp(groupName, (*gs)->groupNames[s]) )
                {
                    *offset = s;
                    return 1;
                }
            }
            Com_PrintError(1, "Group '%s' not found in set '%s'\n", groupName, (*gs)->name);
            return 0;
        }
        else
        {
            *offset = atoi(groupName);
            if ( *offset <= (*gs)->capacity )
            {
                return 1;
            }
            else
            {
                Com_PrintError(1, "Group ID '%d' out of bounds for set '%s'\n", *offset, (*gs)->name);
                return 0;
            }
        }
    }
    else
    {
        *offset = -1;
        return 1;
    }
}

void __cdecl LiveGroups_JoinGroup(int localControllerIndex, char *path)
{
    unsigned int id; // [esp+8h] [ebp-94h]
    GroupSet *v3; // [esp+Ch] [ebp-90h] BYREF
    int offset; // [esp+10h] [ebp-8Ch] BYREF
    unsigned int bits[33]; // [esp+14h] [ebp-88h] BYREF
    GroupMembership *membership; // [esp+98h] [ebp-4h]

    v3 = 0;
    if ( localControllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_groups_dw.cpp",
                    263,
                    0,
                    "localControllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    localControllerIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( LiveGroups_GetGroupID(path, &offset, &v3) )
    {
        membership = &s_groupMembership[localControllerIndex];
        memcpy(bits, membership, 0x80u);
        if ( v3->exclusive )
        {
            for ( id = v3->start; id < v3->capacity + v3->start; ++id )
                bits[id >> 5] &= ~(1 << (id & 0x1F));
        }
        if ( offset >= 0 )
            bits[(offset + v3->start) >> 5] |= 1 << ((offset + v3->start) & 0x1F);
        if ( memcmp(bits, membership, 0x80u) )
        {
            ++membership->dirty;
            memcpy(membership, bits, 0x80u);
        }
    }
    else
    {
        Com_PrintError(1, "Could not find group %s\n", path);
    }
}

void __cdecl LiveGroups_LeaveGroup(int localControllerIndex, char *path)
{
    unsigned int id; // [esp+8h] [ebp-94h]
    GroupSet *v3; // [esp+Ch] [ebp-90h] BYREF
    int offset; // [esp+10h] [ebp-8Ch] BYREF
    unsigned int bits[33]; // [esp+14h] [ebp-88h] BYREF
    GroupMembership *membership; // [esp+98h] [ebp-4h]

    v3 = 0;
    if ( localControllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_groups_dw.cpp",
                    302,
                    0,
                    "localControllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    localControllerIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( LiveGroups_GetGroupID(path, &offset, &v3) )
    {
        membership = &s_groupMembership[localControllerIndex];
        memcpy(bits, membership, 0x80u);
        if ( offset < 0 )
        {
            for ( id = v3->start; id < v3->capacity + v3->start; ++id )
                bits[id >> 5] &= ~(1 << (id & 0x1F));
        }
        else
        {
            bits[(offset + v3->start) >> 5] &= ~(1 << ((offset + v3->start) & 0x1F));
        }
        if ( memcmp(bits, membership, 0x80u) )
        {
            ++membership->dirty;
            memcpy(membership, bits, 0x80u);
        }
    }
    else
    {
        Com_PrintError(1, "Could not find group %s\n", path);
    }
}

unsigned int __cdecl LiveGroups_GetCount(char *path)
{
    GroupSet *v2; // [esp+0h] [ebp-Ch] BYREF
    unsigned int total; // [esp+4h] [ebp-8h]
    int offset; // [esp+8h] [ebp-4h] BYREF

    v2 = 0;
    if ( !LiveGroups_GetGroupID(path, &offset, &v2) )
        return 0;
    if ( offset >= 0 )
        return s_groupCounts[offset + v2->start];
    total = 0;
    for ( offset = 0; offset < v2->capacity; ++offset )
        total += s_groupCounts[offset + v2->start];
    return total;
}

unsigned int __cdecl LiveGroups_GetCountByType(servertype_t type)
{
    if ( (unsigned int)type >= 0x400 )
        return 0;
    else
        return s_groupCounts[type];
}

void __cdecl LiveGroups_RegisterPlayer(int localControllerIndex)
{
    if ( localControllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_groups_dw.cpp",
                    372,
                    0,
                    "localControllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    localControllerIndex,
                    1) )
    {
        __debugbreak();
    }
    memset(&s_groupMembership[localControllerIndex], 0, 0x80u);
    LiveGroups_JoinGroup(localControllerIndex, (char*)"online/mp");
}

void __cdecl LiveGroups_Update(int localControllerIndex)
{
    if ( localControllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_groups_dw.cpp",
                    384,
                    0,
                    "localControllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    localControllerIndex,
                    1) )
    {
        __debugbreak();
    }
    LiveGroups_SetGroups(localControllerIndex);
    LiveGroups_GetCounts(localControllerIndex);
}

void __cdecl LiveGroups_JoinGroup_f()
{
    char *v0; // eax

    v0 = (char *)Cmd_Argv(1);
    LiveGroups_JoinGroup(0, v0);
}

void __cdecl LiveGroups_LeaveGroup_f()
{
    char *v0; // eax

    v0 = (char *)Cmd_Argv(1);
    LiveGroups_LeaveGroup(0, v0);
}

void __cdecl LiveGroups_Dump_f()
{
    unsigned int Count; // eax
    int i; // [esp+0h] [ebp-8h]
    GroupSet *j; // [esp+4h] [ebp-4h]

    for ( j = s_groupSets; j->name; ++j )
    {
        Count = LiveGroups_GetCount((char *)j->name);
        Com_Printf(0, "%s: %u\n", j->name, Count);
        for ( i = 0; i < j->capacity; ++i )
        {
            if ( j->groupNames )
                Com_Printf(0, "        %s: %u\n", j->groupNames[i], s_groupCounts[i + j->start]);
            else
                Com_Printf(0, "        %d: %u\n", i, s_groupCounts[i + j->start]);
        }
    }
}

