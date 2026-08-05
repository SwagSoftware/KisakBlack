#include "live_pcache_profile.h"
#include <universal/UserInfo.h>
#include "live_stats.h"
#include "live_win.h"

ddlDef_t *g_playerDDL;
ddlState_t s_rankState;
ddlState_t s_prestigeState;
ddlState_t s_emblemState;
ddlState_t s_codpointsState;

void __cdecl PCache_ProfileInit()
{
    g_playerDDL = DDL_LoadAsset("ddl_mp/player.ddl");
    DDL_Reset(&s_rankState, g_playerDDL);
    DDL_MoveToName(&s_rankState, &s_rankState, "rank");
    DDL_Reset(&s_prestigeState, g_playerDDL);
    DDL_MoveToName(&s_prestigeState, &s_prestigeState, "prestige");
    DDL_Reset(&s_emblemState, g_playerDDL);
    DDL_MoveToName(&s_emblemState, &s_emblemState, "emblem");
    DDL_Reset(&s_codpointsState, g_playerDDL);
    DDL_MoveToName(&s_codpointsState, &s_codpointsState, "codpoints");
}

ddlDef_t *__cdecl PCache_GetPublicProfileDDL()
{
    return g_playerDDL;
}

void __cdecl PCache_GetPublicProfilesCompleted(TaskRecord *task)
{
#ifdef KISAK_LIVE_STUBS
    unsigned int HeaderSize; // eax
    unsigned __int64 v2; // [esp-Ch] [ebp-154h]
    int retrievedProfileIndex; // [esp+20h] [ebp-128h]
    int controllerIndex; // [esp+24h] [ebp-124h]
    int batchCount; // [esp+2Ch] [ebp-11Ch]
    PCachePublicProfile *profile; // [esp+30h] [ebp-118h]
    PCachePublicProfile *profilea; // [esp+30h] [ebp-118h]
    int batchedProfileIndex; // [esp+34h] [ebp-114h]
    char backup[256]; // [esp+38h] [ebp-110h] BYREF
    PublicProfileInfo *dwProfile; // [esp+13Ch] [ebp-Ch]
    int totalNumberOfProfilesRetrieved; // [esp+140h] [ebp-8h]
    unsigned int index; // [esp+144h] [ebp-4h]

    batchCount = *(unsigned int *)task->payload;
    totalNumberOfProfilesRetrieved = bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)task->remoteTask.m_ptr);
    controllerIndex = task->controllerIndex;
    PCache_Lock();
    for ( index = 0; ; ++index )
    {
        HeaderSize = bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)task->remoteTask.m_ptr);
        if ( index >= HeaderSize )
            break;
        dwProfile = &s_taskData[task->controllerIndex][index];
        if ( dwProfile->m_memberVERSION == 2 )
        {
            profile = (PCachePublicProfile *)PCache_GetComponent(task->controllerIndex, dwProfile->m_entityID, 0);
            if ( profile )
            {
                if ( DDL_AssociateBuffer((char *)dwProfile->m_memberddl, 256, g_playerDDL) )
                {
                    memcpy(profile->ddl, dwProfile->m_memberddl, sizeof(profile->ddl));
                }
                else if ( DDL_FixBufferVersion((char *)dwProfile->m_memberddl, g_playerDDL, "ddl_mp/player.ddl", backup, 256) )
                {
                    memcpy(profile->ddl, backup, sizeof(profile->ddl));
                }
                else
                {
                    memset(profile->ddl, 0, sizeof(profile->ddl));
                    DDL_AssociateBuffer(profile->ddl, 256, g_playerDDL);
                }
                profile->c.state |= 2u;
            }
        }
    }
    if ( batchCount != totalNumberOfProfilesRetrieved )
    {
        for ( batchedProfileIndex = 0; batchedProfileIndex < batchCount; ++batchedProfileIndex )
        {
            for ( retrievedProfileIndex = 0;
                        retrievedProfileIndex < totalNumberOfProfilesRetrieved
                 && (LODWORD(s_batchedProfileXuids[controllerIndex][batchedProfileIndex]) != *((unsigned int *)&s_taskData[0][0].m_entityID
                                                                                                                                                                         + 1120 * task->controllerIndex
                                                                                                                                                                         + 70 * retrievedProfileIndex)
                    || HIDWORD(s_batchedProfileXuids[controllerIndex][batchedProfileIndex]) != *((unsigned int *)&s_taskData[0][0].m_entityID
                                                                                                                                                                         + 1120 * task->controllerIndex
                                                                                                                                                                         + 70 * retrievedProfileIndex
                                                                                                                                                                         + 1));
                        ++retrievedProfileIndex )
            {
                ;
            }
            if ( retrievedProfileIndex == totalNumberOfProfilesRetrieved )
            {
                HIDWORD(v2) = HIDWORD(s_batchedProfileXuids[controllerIndex][batchedProfileIndex]);
                LODWORD(v2) = s_batchedProfileXuids[controllerIndex][batchedProfileIndex];
                profilea = (PCachePublicProfile *)PCache_GetComponent(task->controllerIndex, v2, 0);
                memset(profilea->ddl, 0, sizeof(profilea->ddl));
                profilea->c.state |= 0x20u;
            }
        }
    }
    PCache_Unlock();
#endif
}

void __cdecl PCache_GetPublicProfilesFailed(TaskRecord *task)
{
#ifdef KISAK_LIVE_STUBS
    unsigned __int64 v1; // [esp-Ch] [ebp-20h]
    int controllerIndex; // [esp+0h] [ebp-14h]
    int batchCount; // [esp+8h] [ebp-Ch]
    PCachePublicProfile *profile; // [esp+Ch] [ebp-8h]
    int index; // [esp+10h] [ebp-4h]

    batchCount = *(unsigned int *)task->payload;
    controllerIndex = task->controllerIndex;
    PCache_Lock();
    for ( index = 0; index < batchCount; ++index )
    {
        HIDWORD(v1) = HIDWORD(s_batchedProfileXuids[controllerIndex][index]);
        LODWORD(v1) = s_batchedProfileXuids[controllerIndex][index];
        profile = (PCachePublicProfile *)PCache_GetComponent(task->controllerIndex, v1, 0);
        memset(profile->ddl, 0, sizeof(profile->ddl));
        profile->c.state |= 0x20u;
    }
    PCache_Unlock();
#endif
}

void __cdecl PCache_SetPublicProfileCompleted(TaskRecord *task)
{
    PCache_Lock();
    *((unsigned int *)task->payload + 6) &= ~1u;
    PCache_Unlock();
}

void __cdecl PCache_SetPublicProfileFailed(TaskRecord *task)
{
    PCache_Lock();
    *((unsigned int *)task->payload + 4) = PCache_Time() + 300;
    PCache_Unlock();
}

void __cdecl PCache_GetPublicProfileCompleted(TaskRecord *task)
{
#ifdef KISAK_LIVE_STUBS
    PCachePublicProfile *profile; // [esp+18h] [ebp-10Ch]
    char backup[256]; // [esp+1Ch] [ebp-108h] BYREF
    PublicProfileInfo *dwProfile; // [esp+120h] [ebp-4h]

    if ( bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)task->remoteTask.m_ptr) != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp",
                    162,
                    0,
                    "%s",
                    "task->remoteTask->getNumResults() == 1") )
    {
        __debugbreak();
    }
    PCache_Lock();
    dwProfile = &s_userData[task->controllerIndex];
    profile = (PCachePublicProfile *)task->payload;
    if ( !profile
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp", 169, 0, "%s", "profile") )
    {
        __debugbreak();
    }
    if ( (profile->c.state & 8) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp",
                    170,
                    0,
                    "%s",
                    "profile->c.state & PCACHE_STATE_LOCKED") )
    {
        __debugbreak();
    }
    if ( (LODWORD(profile->c.xuid) != LODWORD(dwProfile->m_entityID)
         || HIDWORD(profile->c.xuid) != HIDWORD(dwProfile->m_entityID))
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp",
                    171,
                    0,
                    "%s",
                    "profile->c.xuid == dwProfile->m_entityID") )
    {
        __debugbreak();
    }
    if ( profile->c.controllerIndex != task->controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp",
                    172,
                    0,
                    "%s",
                    "profile->c.controllerIndex == task->controllerIndex") )
    {
        __debugbreak();
    }
    if ( dwProfile->m_memberVERSION != 2 )
    {
        memset(dwProfile->m_memberddl, 0, 0x100u);
        profile->c.state |= 0x41u;
    }
    if ( DDL_AssociateBuffer((char *)dwProfile->m_memberddl, 256, g_playerDDL) )
    {
        memcpy(profile->ddl, dwProfile->m_memberddl, sizeof(profile->ddl));
    }
    else
    {
        if ( DDL_FixBufferVersion((char *)dwProfile->m_memberddl, g_playerDDL, "ddl_mp/player.ddl", backup, 256) )
        {
            memcpy(profile->ddl, backup, sizeof(profile->ddl));
        }
        else
        {
            memset(profile->ddl, 0, sizeof(profile->ddl));
            DDL_AssociateBuffer(profile->ddl, 256, g_playerDDL);
        }
        profile->c.state |= 0x41u;
    }
    profile->c.state |= 2u;
    PCache_Unlock();
#endif
}

void __cdecl PCache_GetPublicProfileFailed(TaskRecord *task)
{
#ifdef KISAK_LIVE_STUBS
    PCachePublicProfile *profile; // [esp+8h] [ebp-4h]

    PCache_Lock();
    profile = (PCachePublicProfile *)task->payload;
    if ( !profile
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp", 206, 0, "%s", "profile") )
    {
        __debugbreak();
    }
    if ( (profile->c.state & 8) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp",
                    207,
                    0,
                    "%s",
                    "profile->c.state & PCACHE_STATE_LOCKED") )
    {
        __debugbreak();
    }
    if ( profile->c.controllerIndex != task->controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp",
                    208,
                    0,
                    "%s",
                    "profile->c.controllerIndex == task->controllerIndex") )
    {
        __debugbreak();
    }
    if ( bdRemoteTask::getErrorCode(task->remoteTask.m_ptr) == BD_NO_PROFILE_INFO_EXISTS )
    {
        memset(profile->ddl, 0, sizeof(profile->ddl));
        DDL_AssociateBuffer(profile->ddl, 256, g_playerDDL);
        profile->c.state |= 2u;
    }
    PCache_Unlock();
#endif
}

void __cdecl PCache_NukeProfile(int controlleridx)
{
#ifdef KISAK_LIVE_STUBS
    const bdReference<bdCommonAddr> *v1; // eax
    bdReference<bdCommonAddr> v2; // [esp+1Ch] [ebp-10h] BYREF
    TaskRecord *task; // [esp+20h] [ebp-Ch]
    bdProfiles *profileService; // [esp+24h] [ebp-8h]
    bdLobbyService *lobbyService; // [esp+28h] [ebp-4h]

    lobbyService = dwGetLobby(controlleridx);
    if ( lobbyService )
    {
        profileService = bdLobbyService::getProfiles(lobbyService);
        if ( profileService )
        {
            task = TaskManager2_CreateTask(task_pcacheNukeProfile, controlleridx, 0, 0);
            v1 = (const bdReference<bdCommonAddr> *)bdProfiles::deleteProfile(profileService, (int)&v2);
            bdReference<bdCommonAddr>::operator=((bdReference<bdCommonAddr> *)&task->remoteTask, v1);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v2);
            TaskManager2_StartTask(task);
        }
    }
#endif
}

void __cdecl PCache_BatchUpdatePublicProfiles(int controllerIndex, PCachePublicProfile *profiles, int count)
{
#ifdef KISAK_LIVE_STUBS
    unsigned __int64 v3; // rax
    const bdReference<bdCommonAddr> *v4; // eax
    const bdReference<bdCommonAddr> *v5; // eax
    int v6; // edx
    int v7; // eax
    PCachePublicProfile *v8; // ecx
    const bdReference<bdCommonAddr> *PublicInfos; // eax
    bdReference<bdCommonAddr> v10; // [esp+64h] [ebp-34h] BYREF
    bdReference<bdCommonAddr> v11; // [esp+68h] [ebp-30h] BYREF
    bdReference<bdCommonAddr> v12; // [esp+6Ch] [ebp-2Ch] BYREF
    bdProfiles *profileService; // [esp+74h] [ebp-24h]
    bdLobbyService *lobbyService; // [esp+78h] [ebp-20h]
    TaskRecord *task; // [esp+7Ch] [ebp-1Ch]
    int batchCount; // [esp+80h] [ebp-18h]
    PCachePublicProfile *profile; // [esp+84h] [ebp-14h]
    int index; // [esp+88h] [ebp-10h]
    unsigned int updateDelta; // [esp+8Ch] [ebp-Ch]
    unsigned __int64 xuid; // [esp+90h] [ebp-8h] BYREF

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp",
                    249,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    lobbyService = dwGetLobby(controllerIndex);
    if ( lobbyService )
    {
        if ( bdLobbyService::getStatus(lobbyService) == BD_NOT_CONNECTED )
        {
            profileService = bdLobbyService::getProfiles(lobbyService);
            if ( profileService )
            {
                if ( !TaskManager2_TaskIsInProgressForController(task_pcacheGetPublicProfiles, controllerIndex)
                    && !TaskManager2_TaskIsInProgressForController(task_pcacheGetPublicProfile, controllerIndex)
                    && !TaskManager2_TaskIsInProgressForController(task_pcacheSetPublicProfile, controllerIndex) )
                {
                    PCache_Lock();
                    memset(s_batchedProfileXuids[controllerIndex], 0, sizeof(unsigned __int64[16]));
                    LODWORD(v3) = Live_GetXuid(controllerIndex);
                    xuid = v3;
                    profile = (PCachePublicProfile *)PCache_GetComponent(controllerIndex, v3, 0);
                    profile->c.state |= 8u;
                    updateDelta = PCache_Time() - profile->c.updateTime;
                    if ( (profile->c.state & 2) != 0 || updateDelta < 0xF )
                    {
                        if ( (profile->c.state & 1) != 0 && (updateDelta >= 0x3C || (profile->c.state & 0x40) != 0) )
                        {
                            profile->c.updateTime = PCache_Time();
                            profile->c.state &= ~0x40u;
                            s_userData[controllerIndex].m_memberVERSION = 2;
                            memcpy(s_userData[controllerIndex].m_memberddl, profile->ddl, 0x100u);
                            s_userData[controllerIndex].m_memberddl[256] = 0;
                            task = TaskManager2_CreateTask(task_pcacheSetPublicProfile, controllerIndex, 0, 0);
                            v5 = (const bdReference<bdCommonAddr> *)bdProfiles::setPublicInfo(
                                                                                                                profileService,
                                                                                                                (int)&v11,
                                                                                                                (int)&s_userData[controllerIndex]);
                            bdReference<bdCommonAddr>::operator=((bdReference<bdCommonAddr> *)&task->remoteTask, v5);
                            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v11);
                            task->payload = profile;
                            TaskManager2_StartTask(task);
                            PCache_Unlock();
                        }
                        else
                        {
                            if ( (profile->c.state & 2) != 0 )
                                PCache_UpdateProfileData(controllerIndex, profile);
                            batchCount = 0;
                            for ( index = 0; index < count && batchCount < 16; ++index )
                            {
                                profile = &profiles[index];
                                if ( profile->c.controllerIndex == controllerIndex )
                                {
                                    if ( (profile->c.state & 8) != 0 && profile->c.xuid != xuid )
                                        profile->c.state &= ~8u;
                                    if ( (profile->c.state & 4) != 0 && (profile->c.state & 1) == 0 )
                                    {
                                        profile->c.state &= ~4u;
                                        profile->c.updateTime = PCache_Time();
                                        if ( !profile->c.xuid
                                            && !Assert_MyHandler(
                                                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp",
                                                        331,
                                                        0,
                                                        "%s",
                                                        "profile->c.xuid") )
                                        {
                                            __debugbreak();
                                        }
                                        v6 = controllerIndex << 7;
                                        v7 = batchCount;
                                        v8 = profile;
                                        *(unsigned int *)((char *)&s_batchedProfileXuids[0][batchCount] + v6) = profile->c.xuid;
                                        *(unsigned int *)(v6 + 8 * v7 + 173252844) = HIDWORD(v8->c.xuid);
                                        ++batchCount;
                                    }
                                }
                            }
                            if ( batchCount > 0 )
                            {
                                task = TaskManager2_CreateTask(task_pcacheGetPublicProfiles, controllerIndex, 0, 0);
                                *(unsigned int *)task->payload = batchCount;
                                PublicInfos = (const bdReference<bdCommonAddr> *)bdProfiles::getPublicInfos(
                                                                                                                                     profileService,
                                                                                                                                     (int)&v10,
                                                                                                                                     (int)s_batchedProfileXuids[controllerIndex],
                                                                                                                                     &s_taskData[controllerIndex][0],
                                                                                                                                     batchCount);
                                bdReference<bdCommonAddr>::operator=((bdReference<bdCommonAddr> *)&task->remoteTask, PublicInfos);
                                bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v10);
                                TaskManager2_StartTask(task);
                            }
                            PCache_Unlock();
                        }
                    }
                    else
                    {
                        if ( (profile->c.state & 1) != 0
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp",
                                        285,
                                        0,
                                        "%s",
                                        "!( profile->c.state & PCACHE_STATE_DIRTY )") )
                        {
                            __debugbreak();
                        }
                        profile->c.updateTime = PCache_Time();
                        task = TaskManager2_CreateTask(task_pcacheGetPublicProfile, controllerIndex, 0, 0);
                        v4 = (const bdReference<bdCommonAddr> *)bdProfiles::getPublicInfos(
                                                                                                            profileService,
                                                                                                            (int)&v12,
                                                                                                            (int)&xuid,
                                                                                                            &s_userData[controllerIndex],
                                                                                                            1u);
                        bdReference<bdCommonAddr>::operator=((bdReference<bdCommonAddr> *)&task->remoteTask, v4);
                        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v12);
                        task->payload = profile;
                        TaskManager2_StartTask(task);
                        PCache_Unlock();
                    }
                }
            }
        }
    }
#endif
}

char __cdecl PCache_GetRankInternal(PCachePublicProfile *profile, int *rank, int *prestige)
{
    if ( !PCache_IsLocked()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp",
                    348,
                    0,
                    "%s",
                    "PCache_IsLocked()") )
    {
        __debugbreak();
    }
    if ( !profile
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp", 350, 0, "%s", "profile") )
    {
        __debugbreak();
    }
    if ( !PCache_TouchComponent(&profile->c) )
        return 0;
    if ( rank )
        *rank = DDL_GetInt(&s_rankState, profile->ddl);
    if ( prestige )
        *prestige = DDL_GetInt(&s_prestigeState, profile->ddl);
    return 1;
}

char __cdecl PCache_GetRank(int controllerIndex, unsigned __int64 xuid, int *rank, int *prestige)
{
    char RankInternal; // [esp+2h] [ebp-6h]
    PCachePublicProfile *publicProfile; // [esp+4h] [ebp-4h]

    *rank = 0;
    PCache_Lock();
    publicProfile = (PCachePublicProfile *)PCache_GetComponent(controllerIndex, xuid, 0);
    if ( !publicProfile
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp",
                    373,
                    0,
                    "%s",
                    "publicProfile") )
    {
        __debugbreak();
    }
    RankInternal = PCache_GetRankInternal(publicProfile, rank, prestige);
    PCache_Unlock();
    return RankInternal;
}

char __cdecl PCache_GetCodpoints(int controllerIndex, unsigned __int64 xuid, int *codpoints)
{
    PCachePublicProfile *publicProfile; // [esp+4h] [ebp-4h]

    *codpoints = 0;
    PCache_Lock();
    publicProfile = (PCachePublicProfile *)PCache_GetComponent(controllerIndex, xuid, 0);
    if ( !publicProfile
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp",
                    387,
                    0,
                    "%s",
                    "publicProfile") )
    {
        __debugbreak();
    }
    if ( PCache_TouchComponent(&publicProfile->c) )
    {
        if ( codpoints )
            *codpoints = DDL_GetInt(&s_codpointsState, publicProfile->ddl);
        PCache_Unlock();
        return 1;
    }
    else
    {
        PCache_Unlock();
        return 0;
    }
}

void __cdecl PCache_UpdateProfileData(int controllerIndex, PCachePublicProfile *profile)
{
    persistentStats *StatsBuffer; // eax
    PCacheComponent *emblem; // [esp+0h] [ebp-1Ch]
    int currentRank; // [esp+4h] [ebp-18h]
    int currentCodpoints; // [esp+8h] [ebp-14h]
    int currentPrestige; // [esp+Ch] [ebp-10h]
    unsigned int newRank; // [esp+10h] [ebp-Ch]
    unsigned int newPrestige; // [esp+14h] [ebp-8h]
    unsigned int newCodpoints; // [esp+18h] [ebp-4h]

    if ( LiveStorage_DoWeHaveAllStats(controllerIndex) )
    {
        StatsBuffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        if ( !LiveStats_GetBasicTrainingState(StatsBuffer->statsBuffer) )
        {
            if ( !PCache_IsLocked()
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp",
                            414,
                            0,
                            "%s",
                            "PCache_IsLocked()") )
            {
                __debugbreak();
            }
            if ( !profile
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp",
                            416,
                            0,
                            "%s",
                            "profile") )
            {
                __debugbreak();
            }
            if ( profile->c.controllerIndex != controllerIndex
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp",
                            417,
                            0,
                            "%s",
                            "profile->c.controllerIndex == controllerIndex") )
            {
                __debugbreak();
            }
            currentRank = DDL_GetInt(&s_rankState, profile->ddl);
            currentPrestige = DDL_GetInt(&s_prestigeState, profile->ddl);
            currentCodpoints = DDL_GetInt(&s_codpointsState, profile->ddl);
            newRank = LiveStats_GetRank(controllerIndex);
            newPrestige = LiveStats_GetPrestige(controllerIndex);
            newCodpoints = LiveStats_GetCurrency(controllerIndex);
            if ( currentRank != newRank || currentPrestige != newPrestige || currentCodpoints != newCodpoints )
            {
                DDL_SetInt(&s_rankState, newRank, profile->ddl);
                DDL_SetInt(&s_prestigeState, newPrestige, profile->ddl);
                DDL_SetInt(&s_codpointsState, newCodpoints, profile->ddl);
                profile->c.state |= 1u;
                if ( currentRank != newRank )
                {
                    profile->c.state |= 0x40u;
                    profile->c.updateTime = PCache_Time();
                    emblem = PCache_GetComponent(controllerIndex, profile->c.xuid, 1u);
                    if ( !emblem
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp",
                                    442,
                                    0,
                                    "%s",
                                    "emblem") )
                    {
                        __debugbreak();
                    }
                    PCache_InvalidateComponent(emblem);
                }
            }
        }
    }
}

void __cdecl PCache_GetProfileEmblem(
                PCachePublicProfile *profile,
                CompositeEmblemLayer *layers,
                int layerCount,
                __int16 *backgroundID)
{
    __int16 Int; // ax
    int v5; // esi
    double ItemFloat; // st7
    double v7; // st7
    double v8; // st7
    double v9; // st7
    double v10; // st7
    signed int v11; // eax
    signed int v12; // eax
    ddlState_t v13; // [esp+10h] [ebp-F0h] BYREF
    ddlState_t v14; // [esp+20h] [ebp-E0h] BYREF
    ddlState_t v15; // [esp+30h] [ebp-D0h] BYREF
    ddlState_t searchState; // [esp+CCh] [ebp-34h] BYREF
    ddlState_t resultState; // [esp+DCh] [ebp-24h] BYREF
    int layer; // [esp+ECh] [ebp-14h]
    ddlState_t layerState; // [esp+F0h] [ebp-10h] BYREF

    if ( !profile
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp", 492, 0, "%s", "profile") )
    {
        __debugbreak();
    }
    if ( !layers
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp", 493, 0, "%s", "layers") )
    {
        __debugbreak();
    }
    if ( !PCache_IsLocked()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp",
                    495,
                    0,
                    "%s",
                    "PCache_IsLocked()") )
    {
        __debugbreak();
    }
    if ( (profile->c.state & 2) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp",
                    497,
                    0,
                    "%s",
                    "profile->c.state & PCACHE_STATE_DATA") )
    {
        __debugbreak();
    }
    memset(layers, 0, 32 * layerCount);
    DDL_MoveToName(&s_emblemState, &layerState, "layer");
    DDL_MoveToIndex(&layerState, &layerState, 0, 1);
    for ( layer = 0; layer < layerCount; ++layer )
    {
        DDL_MoveToName(&layerState, &resultState, "icon");
        Int = DDL_GetInt(&resultState, profile->ddl);
        layers[layer].icon = Int - 1;
        DDL_MoveToName(&layerState, &searchState, "color");
        v5 = layer;
        layers[v5].colorIdx = DDL_GetInt(&searchState, profile->ddl);
        ItemFloat = GetItemFloat(&layerState, profile->ddl, "posx", -1.0, 1.0, 256);
        layers[layer].pos[0] = ItemFloat;
        v7 = GetItemFloat(&layerState, profile->ddl, "posy", -1.0, 1.0, 256);
        layers[layer].pos[1] = v7;
        v8 = GetItemFloat(&layerState, profile->ddl, "scalex", -4.0, 4.0, 256);
        layers[layer].scale[0] = v8;
        v9 = GetItemFloat(&layerState, profile->ddl, "scaley", -4.0, 4.0, 256);
        layers[layer].scale[1] = v9;
        v10 = GetItemFloat(&layerState, profile->ddl, "angle", 0.0, 360.0, 512);
        layers[layer].angle = v10;
        DDL_MoveToName(&layerState, &v15, "outline");
        v11 = DDL_GetInt(&v15, profile->ddl);
        layers[layer].outline = v11 > 0;
        DDL_MoveToName(&layerState, &v14, "flip");
        v12 = DDL_GetInt(&v14, profile->ddl);
        layers[layer].flip = v12 > 0;
        if ( !DDL_IterateNext(&layerState, &layerState) )
            break;
    }
    DDL_MoveToName(&s_emblemState, &v13, "background");
    *backgroundID = DDL_GetInt(&v13, profile->ddl);
}

double __cdecl GetItemFloat(const ddlState_t *state, char *buffer, const char *item, float min, float max, int range)
{
    float v8; // [esp+4h] [ebp-1Ch]
    ddlState_t resultState; // [esp+8h] [ebp-18h] BYREF
    int out; // [esp+18h] [ebp-8h]
    float value; // [esp+1Ch] [ebp-4h]

    DDL_MoveToName(state, &resultState, item);
    out = DDL_GetInt(&resultState, buffer);
    value = (float)out / (float)(range - 1);
    value = (float)(max - min) * value;
    value = value + min;
    if ( (float)(value - max) < 0.0 )
        v8 = value;
    else
        v8 = max;
    if ( (float)(min - value) < 0.0 )
        return v8;
    else
        return min;
}

void __cdecl PCache_SetProfileEmblem(
                int controllerIndex,
                const CompositeEmblemLayer *layers,
                int layerCount,
                __int16 backgroundID)
{
    unsigned __int64 v4; // rax
    PCacheComponent *Component; // eax
    ddlState_t v6; // [esp+10h] [ebp-FCh] BYREF
    unsigned int flip; // [esp+20h] [ebp-ECh]
    ddlState_t v8; // [esp+24h] [ebp-E8h] BYREF
    unsigned int outline; // [esp+34h] [ebp-D8h]
    ddlState_t v10; // [esp+38h] [ebp-D4h] BYREF
    unsigned int colorIdx; // [esp+C0h] [ebp-4Ch]
    ddlState_t searchState; // [esp+C4h] [ebp-48h] BYREF
    unsigned int value; // [esp+D4h] [ebp-38h]
    ddlState_t resultState; // [esp+D8h] [ebp-34h] BYREF
    int layer; // [esp+E8h] [ebp-24h]
    ddlState_t layerState; // [esp+F0h] [ebp-1Ch] BYREF
    PCachePublicProfile *profile; // [esp+100h] [ebp-Ch]
    unsigned __int64 xuid; // [esp+104h] [ebp-8h]

    if ( !layers
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp", 530, 0, "%s", "layers") )
    {
        __debugbreak();
    }
    PCache_Lock();
    LODWORD(v4) = Live_GetXuid(controllerIndex);
    xuid = v4;
    profile = (PCachePublicProfile *)PCache_GetComponent(controllerIndex, v4, 0);
    if ( (profile->c.state & 2) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache_profile.cpp",
                    537,
                    0,
                    "%s",
                    "profile->c.state & PCACHE_STATE_DATA") )
    {
        __debugbreak();
    }
    DDL_MoveToName(&s_emblemState, &layerState, "layer");
    DDL_MoveToIndex(&layerState, &layerState, 0, 1);
    for ( layer = 0; layer < layerCount; ++layer )
    {
        value = layers[layer].icon + 1;
        DDL_MoveToName(&layerState, &resultState, "icon");
        DDL_SetInt(&resultState, value, profile->ddl);
        colorIdx = layers[layer].colorIdx;
        DDL_MoveToName(&layerState, &searchState, "color");
        DDL_SetInt(&searchState, colorIdx, profile->ddl);
        SetItemFloat(&layerState, profile->ddl, "posx", layers[layer].pos[0], -1.0, 1.0, 256);
        SetItemFloat(&layerState, profile->ddl, "posy", layers[layer].pos[1], -1.0, 1.0, 256);
        SetItemFloat(&layerState, profile->ddl, "scalex", layers[layer].scale[0], -4.0, 4.0, 256);
        SetItemFloat(&layerState, profile->ddl, "scaley", layers[layer].scale[1], -4.0, 4.0, 256);
        SetItemFloat(&layerState, profile->ddl, "angle", layers[layer].angle, 0.0, 360.0, 512);
        outline = layers[layer].outline;
        DDL_MoveToName(&layerState, &v10, "outline");
        DDL_SetInt(&v10, outline, profile->ddl);
        flip = layers[layer].flip;
        DDL_MoveToName(&layerState, &v8, "flip");
        DDL_SetInt(&v8, flip, profile->ddl);
        if ( !DDL_IterateNext(&layerState, &layerState) )
            break;
    }
    DDL_MoveToName(&s_emblemState, &v6, "background");
    DDL_SetInt(&v6, backgroundID, profile->ddl);
    profile->c.state |= 0x41u;
    profile->c.updateTime = PCache_Time();
    Component = PCache_GetComponent(controllerIndex, xuid, 1u);
    PCache_InvalidateComponent(Component);
    PCache_Unlock();
}

void __cdecl SetItemFloat(
                const ddlState_t *state,
                char *buffer,
                const char *item,
                float value,
                float min,
                float max,
                int range)
{
    float v7; // [esp+8h] [ebp-24h]
    float v8; // [esp+10h] [ebp-1Ch]
    ddlState_t resultState; // [esp+18h] [ebp-14h] BYREF
    float v10; // [esp+28h] [ebp-4h]

    if ( (float)(value - max) < 0.0 )
        v10 = value;
    else
        v10 = max;
    if ( (float)(min - value) < 0.0 )
        v8 = v10;
    else
        v8 = min;
    v7 = floor((double)(range - 1) * (float)((float)(v8 - min) / (float)(max - min)));
    DDL_MoveToName(state, &resultState, item);
    DDL_SetInt(&resultState, (int)v7, buffer);
}

