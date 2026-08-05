#ifdef KISAK_DW

#include "dwStats.h"
#include "dwUtils.h"

void __cdecl dwWriteStats(overlappedTask *const task, bdStatsInfo **statsInfo, unsigned int numStats)
{
#ifdef KISAK_DEMON
    bdReference<bdCommonAddr> v3; // [esp+20h] [ebp-Ch] BYREF
    bdRemoteTask *remoteTask; // [esp+24h] [ebp-8h]
    bdStats *stats; // [esp+28h] [ebp-4h]

    stats = dwGetStats(task->controllerIndex);
    if ( stats && task )
    {
        remoteTask = *(bdRemoteTask **)bdStats::writeStats(stats, (int)&v3, (int)statsInfo, numStats);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v3);
        if ( remoteTask )
        {
            bdReference<bdCommonAddr>::operator=(&task->overlappedIO, remoteTask);
        }
        else
        {
            TaskManager_ClearTask(task);
            dwConnectionErrorHandler(6);
        }
    }
#endif
}

taskCompleteResults __cdecl dwWriteStatsComplete(overlappedTask *const task)
{
#ifdef KISAK_DEMON
    char errorStr[64]; // [esp+Ch] [ebp-58h] BYREF
    bdLobbyErrorCode errorCode; // [esp+50h] [ebp-14h]
    bdRemoteTask::bdStatus taskStatus; // [esp+54h] [ebp-10h]
    taskCompleteResults status; // [esp+58h] [ebp-Ch]
    bdStats *stats; // [esp+5Ch] [ebp-8h]
    int localControllerIndex; // [esp+60h] [ebp-4h]

    status = TASK_ERROR;
    localControllerIndex = task->controllerIndex;
    stats = dwGetStats(localControllerIndex);
    if ( stats && task && task->overlappedIO.m_ptr )
    {
        dwLobbyPump(localControllerIndex);
        taskStatus = bdRemoteTask::getStatus(task->overlappedIO.m_ptr);
        errorCode = BD_NO_ERROR;
        if ( taskStatus == BD )
        {
            errorCode = bdRemoteTask::getErrorCode(task->overlappedIO.m_ptr);
            if ( errorCode )
            {
                TaskManager_ClearTask(task);
                dwLobbyErrorCodeToString(errorCode, errorStr, 0x40u);
                Com_PrintError(16, "DW write stats error %i - '%s'\n", errorCode, errorStr);
            }
        }
        status = dwTaskStatusConvert(taskStatus, errorCode);
        if ( taskStatus != BD_PENDING )
            dwCloseRemoteTask(&task->overlappedIO);
    }
    else
    {
        Com_Printf(16, "Failed to retreive stats write status\n");
    }
    return status;
#else
    return TASK_COMPLETE;
#endif
}

void __cdecl dwReadStatsByUser(
                overlappedTask *const task,
                unsigned int leaderBoardID,
                unsigned __int64 entityID,
                bdStatsInfo *statsInfo)
{
    dwReadStatsForUsers(task, leaderBoardID, &entityID, 1u, statsInfo);
}

void __cdecl dwReadStatsByRank(
                overlappedTask *const task,
                unsigned int leaderBoardID,
                unsigned int topRank,
                unsigned int maxResults,
                bdStatsInfo *statsInfo)
{
#ifdef KISAK_DEMON
    bdReference<bdCommonAddr> v5; // [esp+20h] [ebp-Ch] BYREF
    bdRemoteTask *remoteTask; // [esp+24h] [ebp-8h]
    bdStats *stats; // [esp+28h] [ebp-4h]

    stats = dwGetStats(task->controllerIndex);
    if ( stats && task )
    {
        remoteTask = *(bdRemoteTask **)bdStats::readStatsByRank(
                                                                         stats,
                                                                         (int)&v5,
                                                                         leaderBoardID,
                                                                         topRank,
                                                                         statsInfo,
                                                                         maxResults);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v5);
        if ( remoteTask )
            bdReference<bdCommonAddr>::operator=(&task->overlappedIO, remoteTask);
    }
#endif
}

void __cdecl dwReadStatsByPivot(
                overlappedTask *const task,
                unsigned int leaderBoardID,
                unsigned __int64 entityID,
                unsigned int maxResults,
                bdStatsInfo *statsInfo)
{
#ifdef KISAK_DEMON
    bdReference<bdCommonAddr> v5; // [esp+20h] [ebp-Ch] BYREF
    bdRemoteTask *remoteTask; // [esp+24h] [ebp-8h]
    bdStats *stats; // [esp+28h] [ebp-4h]

    stats = dwGetStats(task->controllerIndex);
    if ( stats && task )
    {
        remoteTask = *(bdRemoteTask **)bdStats::readStatsByPivot(
                                                                         stats,
                                                                         (int)&v5,
                                                                         leaderBoardID,
                                                                         entityID,
                                                                         statsInfo,
                                                                         maxResults);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v5);
        if ( remoteTask )
            bdReference<bdCommonAddr>::operator=(&task->overlappedIO, remoteTask);
    }
#endif
}

void __cdecl dwReadStatsForUsers(
                overlappedTask *const task,
                unsigned int leaderBoardID,
                const unsigned __int64 *entityIDs,
                unsigned int numEntityIDs,
                bdStatsInfo *statsInfo)
{
#ifdef KISAK_DEMON
    bdReference<bdCommonAddr> v5; // [esp+20h] [ebp-Ch] BYREF
    bdRemoteTask *remoteTask; // [esp+24h] [ebp-8h]
    bdStats *stats; // [esp+28h] [ebp-4h]

    stats = dwGetStats(task->controllerIndex);
    if ( stats && task )
    {
        remoteTask = *(bdRemoteTask **)bdStats::readStatsByEntityID(
                                                                         stats,
                                                                         (int)&v5,
                                                                         leaderBoardID,
                                                                         (int)entityIDs,
                                                                         numEntityIDs,
                                                                         statsInfo);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v5);
        if ( remoteTask )
        {
            bdReference<bdCommonAddr>::operator=(&task->overlappedIO, remoteTask);
        }
        else
        {
            TaskManager_ClearTask(task);
            dwConnectionErrorHandler(8);
        }
    }
#endif
}

taskCompleteResults __cdecl dwReadStatsComplete(overlappedTask *const task, int *numResults, int *totalResults)
{
#ifdef KISAK_DEMON
    char errorStr[68]; // [esp+14h] [ebp-58h] BYREF
    bdLobbyErrorCode errorCode; // [esp+5Ch] [ebp-10h]
    bdRemoteTask::bdStatus taskStatus; // [esp+60h] [ebp-Ch]
    taskCompleteResults status; // [esp+64h] [ebp-8h]
    bdStats *stats; // [esp+68h] [ebp-4h]

    if ( !numResults
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStats.cpp", 235, 0, "%s", "numResults") )
    {
        __debugbreak();
    }
    if ( !totalResults
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStats.cpp", 236, 0, "%s", "totalResults") )
    {
        __debugbreak();
    }
    status = TASK_ERROR;
    stats = dwGetStats(task->controllerIndex);
    if ( stats && task && task->overlappedIO.m_ptr && numResults && totalResults )
    {
        dwLobbyPump(task->controllerIndex);
        taskStatus = bdRemoteTask::getStatus(task->overlappedIO.m_ptr);
        errorCode = BD_NO_ERROR;
        if ( taskStatus == BD )
        {
            errorCode = bdRemoteTask::getErrorCode(task->overlappedIO.m_ptr);
            if ( errorCode )
            {
                TaskManager_ClearTask(task);
                dwLobbyErrorCodeToString(errorCode, errorStr, 0x40u);
                Com_Error(ERR_DROP, "DW read stats error %i - '%s'\n", errorCode, errorStr);
            }
            *numResults = bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)task->overlappedIO.m_ptr);
            *totalResults = bdRemoteTask::getTotalNumResults(task->overlappedIO.m_ptr);
        }
        status = dwTaskStatusConvert(taskStatus, errorCode);
        if ( taskStatus != BD_PENDING )
            dwCloseRemoteTask(task);
    }
    else
    {
        Com_Printf(16, "Failed to retreive stats read status\n");
    }
    return status;
#else
    return TASK_COMPLETE;
#endif
}

taskCompleteResults __cdecl dwFetchPerformanceValuesComplete(
                overlappedTask *const task,
                PlayerRank *playerRanks,
                int *numPlayerRanks)
{
#ifdef KISAK_DEMON
    int v3; // ecx
    int v4; // edx
    __int64 v6; // [esp-10h] [ebp-98h]
    __int64 v7; // [esp-8h] [ebp-90h]
    void (__thiscall ***data)(void *, int); // [esp+20h] [ebp-68h]
    char errorStr[64]; // [esp+28h] [ebp-60h] BYREF
    int i; // [esp+6Ch] [ebp-1Ch]
    int numResults; // [esp+70h] [ebp-18h]
    int numRankSlots; // [esp+74h] [ebp-14h]
    bdLobbyErrorCode errorCode; // [esp+78h] [ebp-10h]
    bdRemoteTask::bdStatus taskStatus; // [esp+7Ch] [ebp-Ch]
    taskCompleteResults status; // [esp+80h] [ebp-8h]
    bdMatchMaking *matchmaking; // [esp+84h] [ebp-4h]

    status = TASK_ERROR;
    matchmaking = dwGetMatchmaking(task->controllerIndex);
    if ( matchmaking && task && task->overlappedIO.m_ptr )
    {
        dwLobbyPump(task->controllerIndex);
        taskStatus = bdRemoteTask::getStatus(task->overlappedIO.m_ptr);
        errorCode = BD_NO_ERROR;
        if ( taskStatus == BD )
        {
            numRankSlots = *numPlayerRanks;
            numResults = bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)task->overlappedIO.m_ptr);
            if ( numResults > numRankSlots
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStats.cpp",
                            382,
                            0,
                            "%s",
                            "numResults <= numRankSlots") )
            {
                __debugbreak();
            }
            memset(playerRanks, 0, 16 * numRankSlots);
            *numPlayerRanks = 0;
            for ( i = 0; i < numResults; ++i )
            {
                if ( i < numRankSlots )
                {
                    playerRanks[i].rank = s_performanceValue[i].m_performanceValue;
                    v3 = 24 * i;
                    v4 = i;
                    LODWORD(playerRanks[v4].xuid) = s_performanceValue[i].m_entityID;
                    HIDWORD(playerRanks[v4].xuid) = *(unsigned int *)(v3 + 53928740);
                    ++*numPlayerRanks;
                }
                HIDWORD(v7) = HIDWORD(s_performanceValue[i].m_entityID);
                LODWORD(v7) = s_performanceValue[i].m_entityID;
                HIDWORD(v6) = HIDWORD(s_performanceValue[i].m_performanceValue);
                LODWORD(v6) = s_performanceValue[i].m_performanceValue;
                Com_Printf(16, "Fetched performance value %llx for %llx \n", v6, v7);
            }
            errorCode = bdRemoteTask::getErrorCode(task->overlappedIO.m_ptr);
            if ( errorCode )
            {
                TaskManager_ClearTask(task);
                dwLobbyErrorCodeToString(errorCode, errorStr, 0x40u);
                Com_Error(ERR_DROP, "DW fetch matchmaking values error %i - '%s'\n", errorCode, errorStr);
            }
        }
        status = dwTaskStatusConvert(taskStatus, errorCode);
        if ( taskStatus != BD_PENDING )
        {
            data = (void (__thiscall ***)(void *, int))task->data;
            if ( data )
            {
                if ( *(data - 1) )
                    (**data)(data, 3);
                else
                    operator delete[](data - 1);
            }
            dwCloseRemoteTask(&task->overlappedIO);
        }
    }
    else
    {
        Com_Printf(16, "Failed to retrieve performance values fetch status\n");
    }
    return status;
#else
    return TASK_COMPLETE;
#endif
}

taskCompleteResults __cdecl dwWritePerformanceStatsComplete(int controllerIndex, overlappedTask *const task)
{
#ifdef KISAK_DEMON
    char errorStr[68]; // [esp+Ch] [ebp-58h] BYREF
    bdLobbyErrorCode errorCode; // [esp+54h] [ebp-10h]
    bdRemoteTask::bdStatus taskStatus; // [esp+58h] [ebp-Ch]
    taskCompleteResults status; // [esp+5Ch] [ebp-8h]
    bdMatchMaking *performance; // [esp+60h] [ebp-4h]

    status = TASK_ERROR;
    performance = dwGetMatchmaking(controllerIndex);
    if ( performance && task && task->overlappedIO.m_ptr )
    {
        taskStatus = bdRemoteTask::getStatus(task->overlappedIO.m_ptr);
        errorCode = BD_NO_ERROR;
        if ( taskStatus == BD )
        {
            errorCode = bdRemoteTask::getErrorCode(task->overlappedIO.m_ptr);
            if ( errorCode )
            {
                TaskManager_ClearTask(task);
                dwLobbyErrorCodeToString(errorCode, errorStr, 0x40u);
                Com_Error(ERR_DROP, "DW write performance values error %i - '%s'\n", errorCode, errorStr);
            }
        }
        status = dwTaskStatusConvert(taskStatus, errorCode);
        if ( taskStatus != BD_PENDING )
            dwCloseRemoteTask(&task->overlappedIO);
    }
    else
    {
        Com_Printf(16, "Failed to retreive performance value write status\n");
    }
    return status;
#else
    return TASK_COMPLETE;
#endif
}


#endif