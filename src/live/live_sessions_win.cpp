#include "live_sessions_win.h"
#include <win32/win_tasks.h>
#include <DW/dwMatchMaking.h>
#include <client_mp/cl_main_pc_mp.h>
#include <gfx_d3d/r_rendercmds.h>
#include "live_storage_win.h"
#include "live_win.h"
#include <DW/dwUtils_pc.h>
#include "live_stats.h"
#include <game_mp/g_main_mp.h>
#include <server_mp/sv_init_mp.h>
#include "live_leaderboard.h"
#include <cgame/cg_compass.h>

SessionData_s g_serverSession;

overlappedTask overlappedTasks_2[32];
const dvar_t *matchmaking_debug;
SessionJoinData sessionJoinData[32];
SessionCreateData sessionCreateData[32];

SessionGraveYard sessionGraveYard[6];

void __cdecl Session_ClearDWOverlappedTasks()
{
    TaskManager_ClearOverlappedTasks(overlappedTasks_2);
}

void __cdecl Live_FindSessionsStart(bool reset, int servertype)
{
#ifdef KISAK_LIVE
    dwFindSessionsPaged(1044, servertype, 0, 1);
#endif
}

void __cdecl Live_FindSessionsPump()
{
    CL_QuickMatch_Frame();
}

void __cdecl Session_QoSListenStart(SessionData_s *session)
{
#ifdef KISAK_LIVE_STUBS
    unsigned __int8 *QosPayloadBuffer; // eax
    unsigned int v2; // [esp-4h] [ebp-8h]
    bdQoSProbe *qosProbe; // [esp+0h] [ebp-4h]

    if ( session->qosListenEnabled
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    201,
                    0,
                    "%s",
                    "!session->qosListenEnabled") )
    {
        __debugbreak();
    }
    Com_Printf(16, "calling XNetQosListen( ENABLE ) for session '%s'\n", session->sessionName);
    Session_BuildQoSPayload();
    qosProbe = dwGetQoSProbe();
    if ( qosProbe
        && (v2 = XAnimTreeSize(),
                QosPayloadBuffer = Session_GetQosPayloadBuffer(),
                bdQoSProbe::listen(qosProbe, &session->sessionInfo.sessionID, QosPayloadBuffer, v2)) )
    {
        session->qosListenEnabled = 1;
    }
    else
    {
        Com_Error(ERR_DROP, "Error when trying to listen for QoS queries for %s\n", session->sessionName);
    }
    UI_CloseMenu(0, "popup_gettingdata");
#endif
}

void __cdecl Session_QoSListenStop(SessionData_s *session)
{
#ifdef KISAK_LIVE_STUBS
    bdQoSProbe *qosProbe; // [esp+0h] [ebp-4h]

    if ( !session->qosListenEnabled
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    221,
                    0,
                    "%s",
                    "session->qosListenEnabled") )
    {
        __debugbreak();
    }
    Com_Printf(16, "calling XNetQosListen( DISABLE ) for session '%s'\n", session->sessionName);
    qosProbe = dwGetQoSProbe();
    if ( qosProbe )
        bdQoSProbe::disableListener(qosProbe);
    else
        Com_PrintError(16, "Error from dwGetQoSProbe()\n");
    session->qosListenEnabled = 0;
#endif
}

int __cdecl Session_FindRegisteredUser(const SessionData_s *session, unsigned __int64 xuid)
{
    int slot; // [esp+8h] [ebp-4h]

    for ( slot = 0; slot < 32; ++slot )
    {
        if ( session->registeredUsers[slot].active && session->registeredUsers[slot].xuid == xuid )
            return slot;
    }
    return -1;
}

void __cdecl Session_EndGameSession(SessionData_s *session)
{
    if ( session->sessionStartCalled )
    {
        if ( session->sessionHandle )
        {
            R_BeginRemoteScreenUpdate();
            Session_EndOngoingSessionTasks(session);
            LiveStorage_UploadStats();
            R_EndRemoteScreenUpdate(0);
        }
        session->sessionStartCalled = 0;
    }
}

void __cdecl Session_DeleteSession(SessionData_s *session)
{
#ifdef KISAK_LIVE_STUBS
    int FreeSessionGraveYardSlot; // eax

    Session_EndOngoingSessionTasks(session);
    if ( session->qosListenEnabled )
        Session_QoSListenStop(session);
    if ( session->sessionHandle )
    {
        if ( session_nonblocking->current.enabled )
        {
            Session_UnregisterAllUsersFromVoice(session);
            FreeSessionGraveYardSlot = Session_GetFreeSessionGraveYardSlot();
            sessionGraveYard[FreeSessionGraveYardSlot].active = 1;
            memcpy((char *)&unk_A53DA40 + 2432 * FreeSessionGraveYardSlot, session, 0x978u);
            session->sessionHandle = 0;
            memset(session->registeredUsers, 0, sizeof(session->registeredUsers));
        }
        else
        {
            Com_Printf(16, "Deleting '%s' session\n", session->sessionName);
            Session_EndGameSession(session);
            Session_EveryoneLeaveSessionAsync(0, session);
            Session_EndOngoingSessionTasks(session);
            Session_DeleteHandle(&session->sessionHandle);
            session->sessionHandle = 0;
        }
    }
    //BLOPS_NULLSUB();
    memset(session->registeredUsers, 0, sizeof(session->registeredUsers));
    session->flags = 0;
    session->privateSlots = 0;
    session->publicSlots = 0;
    session->keysGenerated = 0;
    //BLOPS_NULLSUB();
#endif
}

void __cdecl Session_DeleteHandle(bool *sessionHandle)
{
#ifdef KISAK_LIVE_STUBS
    overlappedTask *sessionDeleteOverlappedIO; // [esp+0h] [ebp-14h]
    bdSecurityID secID; // [esp+4h] [ebp-10h] BYREF
    int localControllerIndex; // [esp+10h] [ebp-4h]

    if ( (!sessionHandle || !*sessionHandle)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    319,
                    0,
                    "%s",
                    "sessionHandle && *sessionHandle") )
    {
        __debugbreak();
    }
    *sessionHandle = 0;
    if ( g_matchmakingInfo->m_active )
    {
        localControllerIndex = 0;
        sessionDeleteOverlappedIO = TaskManager_GetOpenTaskSlot(overlappedTasks_2, 0, 3);
        g_matchmakingInfo->m_active = 0;
        bdSecurityID::bdSecurityID(&secID, &g_matchmakingInfo->m_sessionID);
        dwDeleteSession(sessionDeleteOverlappedIO->controllerIndex, sessionDeleteOverlappedIO);
        TaskManager_ClearTask(sessionDeleteOverlappedIO);
    }
    LiveStorage_UploadStats();
#endif
}

void __cdecl Session_UnregisterAllUsersFromVoice(SessionData_s *session)
{
    int slot; // [esp+0h] [ebp-4h]

    if ( session->registerUsersWithVoice )
    {
        for ( slot = 0; slot < 32; ++slot )
            ;
    }
}

int __cdecl Session_GetFreeSessionGraveYardSlot()
{
    int slot; // [esp+0h] [ebp-4h]

    for ( slot = 0; slot < 6; ++slot )
    {
        if ( !sessionGraveYard[slot].active )
            return slot;
    }
    if ( !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    364,
                    0,
                    "All session graveyard slots are full") )
        __debugbreak();
    return 0;
}

void __cdecl Session_StartHost(SessionData_s *session, int sessionFlags, int numPrivateSlots, int numPublicSlots)
{
#ifdef KISAK_LIVE_STUBS
    int SignInState; // eax
    bdTrulyRandomImpl *Instance; // eax
    bdSecurityKey secKey; // [esp+64h] [ebp-24h] BYREF
    int requiredSigninState; // [esp+78h] [ebp-10h]
    unsigned int sessionCreateControllerIndex; // [esp+7Ch] [ebp-Ch]
    overlappedTask *sessionCreateOverlappedIO; // [esp+80h] [ebp-8h]
    int sessionCreateSlot; // [esp+84h] [ebp-4h]
    int sessionFlagsa; // [esp+94h] [ebp+Ch]

    if ( !session
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp", 755, 0, "%s", "session") )
    {
        __debugbreak();
    }
    if ( !session->sessionName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    756,
                    0,
                    "%s",
                    "session->sessionName") )
    {
        __debugbreak();
    }
    if ( session->sessionHandle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    757,
                    0,
                    "%s",
                    "!session->sessionHandle") )
    {
        __debugbreak();
    }
    sessionFlagsa = sessionFlags | 8;
    if ( (sessionFlagsa & 8) != 0 || (sessionFlagsa & 2) != 0 )
        requiredSigninState = 2;
    else
        requiredSigninState = 1;
    sessionCreateControllerIndex = 0;
    if ( CL_ControllerIndex_GetSignInState(0) < requiredSigninState )
        Com_Error(ERR_DROP, "XBOXLIVE_SIGNEDOUTOFLIVE");
    if ( CL_ControllerIndex_GetSignInState(sessionCreateControllerIndex) < requiredSigninState
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    775,
                    0,
                    "%s",
                    "CL_ControllerIndex_GetSignInState( sessionCreateControllerIndex ) >= requiredSigninState") )
    {
        __debugbreak();
    }
    sessionCreateOverlappedIO = TaskManager_GetOpenTaskSlot(overlappedTasks_2, sessionCreateControllerIndex, 2);
    sessionCreateSlot = Session_GetFreeCreateSessionSlot();
    sessionCreateData[sessionCreateSlot].active = 1;
    sessionCreateData[sessionCreateSlot].session = session;
    sessionCreateOverlappedIO->data = (void *)(8 * sessionCreateSlot + 173258800);
    //BLOPS_NULLSUB();
    if ( sessionCreateControllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    793,
                    0,
                    "%s\n\t(sessionCreateControllerIndex) = %i",
                    "(sessionCreateControllerIndex >= 0 && sessionCreateControllerIndex < 1)",
                    sessionCreateControllerIndex) )
    {
        __debugbreak();
    }
    if ( CL_ControllerIndex_GetSignInState(sessionCreateControllerIndex) <= 0 )
    {
        SignInState = CL_ControllerIndex_GetSignInState(sessionCreateControllerIndex);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                        794,
                        0,
                        "%s\n\t(CL_ControllerIndex_GetSignInState( sessionCreateControllerIndex )) = %i",
                        "(CL_ControllerIndex_GetSignInState( sessionCreateControllerIndex ) > 0)",
                        SignInState) )
            __debugbreak();
    }
    if ( (sessionFlagsa & 8) != 0 )
    {
        if ( numPrivateSlots <= 0
            && numPublicSlots <= 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                        799,
                        0,
                        "%s",
                        "(numPrivateSlots > 0) || (numPublicSlots > 0)") )
        {
            __debugbreak();
        }
        Com_Printf(
            16,
            "Calling dwCreateSession (%s [%p]) with controller %i as the host (flags are %i), %i public and %i private slots\n",
            session->sessionName,
            (const void *)session->sessionHandle,
            sessionCreateControllerIndex,
            sessionFlagsa,
            numPublicSlots,
            numPrivateSlots);
        g_matchmakingInfo->m_memberGAME_TYPE = xblive_privatematch->current.color[0] == 1;
        g_matchmakingInfo->m_memberNETCODE_VERSION = 1044;
        g_matchmakingInfo->m_maxPlayers = numPublicSlots;
        g_matchmakingInfo->m_memberDDL_VERSION = LiveStats_GetStatsDDL()->version;
        g_matchmakingInfo->m_memberLICENSE = 5;
        if ( ui_gametype )
            g_matchmakingInfo->m_memberGAME_TYPE = Com_GametypeToInt(ui_gametype->current.string);
        I_strncpyz(g_matchmakingInfo->m_membermapname, sv_mapname->current.string, 32);
        I_strncpyz(g_matchmakingInfo->m_memberservername, sv_hostname->current.string, 64);
        g_matchmakingInfo->m_memberserverType = 0;
        dwGetOnlineUserID(0, &g_matchmakingInfo->m_memberdemonwareID);
        g_matchmakingInfo->m_memberTEAM_SIZE_MAX = numPublicSlots;
        sessionCreateOverlappedIO->data = (void *)(8 * sessionCreateSlot + 173258800);
        bdSecurityKey::bdSecurityKey(&secKey);
        Instance = bdSingleton<bdTrulyRandomImpl>::getInstance();
        bdTrulyRandomImpl::getRandomUByte8(Instance, secKey.ab, 0x10u);
        MatchMakingInfo::setSessionKey(g_matchmakingInfo, &secKey);
        dwCreateSession((bdReference<bdCommonAddr> *)sessionCreateOverlappedIO, g_matchmakingInfo);
    }
    session->flags = sessionFlagsa;
    session->publicSlots = numPublicSlots;
    session->privateSlots = numPrivateSlots;
    session->sessionHandle = 1;
    session->sessionCreateController = sessionCreateControllerIndex;
    if ( !session_nonblocking->current.enabled )
        Session_EndOngoingSessionTasks(session);
    dwCreateLanSession();
#endif
}

int __cdecl Session_GetFreeCreateSessionSlot()
{
    int slot; // [esp+0h] [ebp-4h]

    for ( slot = 0; slot < 32; ++slot )
    {
        if ( !sessionCreateData[slot].active )
            return slot;
    }
    if ( !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    742,
                    0,
                    "All create session slots are full") )
        __debugbreak();
    return 0;
}

void __cdecl Session_Modify(
                int localControllerIndex,
                SessionData_s *session,
                int flags,
                int publicSlots,
                int privateSlots)
{
#ifdef KISAK_LIVE
    overlappedTask *sessionModifyOverlappedIO; // [esp+0h] [ebp-8h]

    if ( session->sessionHandle && g_matchmakingInfo->m_active )
    {
        sessionModifyOverlappedIO = TaskManager_GetOpenTaskSlot(overlappedTasks_2, localControllerIndex, 6);
        Com_Printf(16, "Modifying session\n");
        sessionModifyOverlappedIO->data = session;
        dwUpdateSession(sessionModifyOverlappedIO, g_matchmakingInfo);
        if ( (unsigned int)dwUpdateSessionComplete(sessionModifyOverlappedIO) <= TASK_COMPLETE )
        {
            session->privateSlots = privateSlots;
            session->publicSlots = publicSlots;
            session->flags = flags;
        }
    }
#endif
}

taskCompleteResults __cdecl Session_ModifyComplete(int slot)
{
#ifdef KISAK_LIVE
    taskCompleteResults res; // [esp+0h] [ebp-8h]
    overlappedTask *sessionModifyOverlappedIO; // [esp+4h] [ebp-4h]

    if ( slot >= 32
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    1010,
                    0,
                    "%s",
                    "slot < static_cast<int>( sizeof(overlappedTasks) / sizeof(overlappedTasks[0]) )") )
    {
        __debugbreak();
    }
    sessionModifyOverlappedIO = &overlappedTasks_2[slot];
    if ( !overlappedTasks_2[slot].active
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    1014,
                    0,
                    "%s",
                    "overlappedTasks[slot].active") )
    {
        __debugbreak();
    }
    if ( overlappedTasks_2[slot].type != 6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    1015,
                    0,
                    "%s",
                    "overlappedTasks[slot].type == TASK_MODIFYINGSESSION") )
    {
        __debugbreak();
    }
    res = dwUpdateSessionComplete(sessionModifyOverlappedIO);
    if ( res == TASK_COMPLETE )
        Com_Printf(16, "Session modify complete\n");
    if ( res )
        TaskManager_ClearTask(sessionModifyOverlappedIO);
    if ( res == TASK_ERROR )
        Com_PrintError(16, "EXE_ERROR_MODIFYING_SESSION");
    return res;
#else
    return TASK_COMPLETE;
#endif
}

void __cdecl Session_EveryoneLeaveSessionAsync(int localControllerIndex, SessionData_s *session)
{
    int v2; // ecx
    int registeredUserSlot; // [esp+Ch] [ebp-114h]
    unsigned __int64 players[32]; // [esp+10h] [ebp-110h]
    int numPlayers; // [esp+114h] [ebp-Ch]
    unsigned __int64 player; // [esp+118h] [ebp-8h]

    if ( session->sessionHandle )
    {
        Session_EndOngoingSessionTasks(session);
        numPlayers = 0;
        for ( registeredUserSlot = 0; registeredUserSlot < 32; ++registeredUserSlot )
        {
            if ( registeredUserSlot < 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                            1198,
                            0,
                            "%s\n\t(registeredUserSlot) = %i",
                            "(registeredUserSlot >= 0 && registeredUserSlot < 32)",
                            registeredUserSlot) )
            {
                __debugbreak();
            }
            if ( session->registeredUsers[registeredUserSlot].active )
            {
                if ( session->remoteTalkers[registeredUserSlot].xuid )
                {
                    if ( Session_JoinInProgress() )
                        Live_FinishOngoingSessionJoinTasksForXUID(session->remoteTalkers[registeredUserSlot].xuid);
                }
                player = session->registeredUsers[registeredUserSlot].xuid;
                v2 = numPlayers;
                LODWORD(players[numPlayers]) = player;
                HIDWORD(players[v2]) = HIDWORD(player);
                ++numPlayers;
                Com_Printf(
                    16,
                    "Calling XSessionLeaveRemote for slot %i in session '%s'\n",
                    registeredUserSlot,
                    session->sessionName);
                session->registeredUsers[registeredUserSlot].active = 0;
            }
        }
    }
}

char __cdecl Session_JoinInProgress()
{
    return TaskManager_TaskIsInProgress(overlappedTasks_2, 1);
}

bool __cdecl Session_SessionTasksInProgress(SessionData_s *session)
{
    bool result; // al
    const char *v2; // eax
    int tasknum; // [esp+Ch] [ebp-4h]

    for ( tasknum = 0; ; ++tasknum )
    {
        if ( tasknum >= 32 )
            return 0;
        if ( overlappedTasks_2[tasknum].active )
            break;
LABEL_2:
        ;
    }
    switch ( overlappedTasks_2[tasknum].type )
    {
        case 1:
            if ( *((SessionData_s **)TaskManager_GetTaskData(&overlappedTasks_2[tasknum]) + 6) != session )
                goto LABEL_2;
            result = 1;
            break;
        case 2:
            if ( *((SessionData_s **)TaskManager_GetTaskData(&overlappedTasks_2[tasknum]) + 1) != session )
                goto LABEL_2;
            result = 1;
            break;
        case 4:
            if ( session != &g_serverSession )
                goto LABEL_2;
            result = 1;
            break;
        case 5:
            if ( session != &g_serverSession )
                goto LABEL_2;
            result = 1;
            break;
        case 6:
            if ( session != TaskManager_GetTaskData(&overlappedTasks_2[tasknum]) )
                goto LABEL_2;
            result = 1;
            break;
        case 7:
            if ( session != TaskManager_GetTaskData(&overlappedTasks_2[tasknum]) )
                goto LABEL_2;
            result = 1;
            break;
        default:
            v2 = va("Unknown session task type %i\n", overlappedTasks_2[tasknum].type);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp", 1782, 0, v2) )
                __debugbreak();
            goto LABEL_2;
    }
    return result;
}

int __cdecl Session_StartSessionComplete(int slot)
{
    if ( slot >= 32
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    1102,
                    0,
                    "%s",
                    "slot < static_cast<int>( sizeof(overlappedTasks) / sizeof(overlappedTasks[0]) )") )
    {
        __debugbreak();
    }
    if ( !overlappedTasks_2[slot].active
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    1106,
                    0,
                    "%s",
                    "overlappedTasks[slot].active") )
    {
        __debugbreak();
    }
    if ( overlappedTasks_2[slot].type != 5
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    1107,
                    0,
                    "%s",
                    "overlappedTasks[slot].type == TASK_STARTINGGAMESESSION") )
    {
        __debugbreak();
    }
    TaskManager_ClearTask(&overlappedTasks_2[slot]);
    return 1;
}

int __cdecl Session_JoinSessionComplete(int slot)
{
    if ( slot >= 32
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    1142,
                    0,
                    "%s",
                    "slot < static_cast<int>( sizeof(overlappedTasks) / sizeof(overlappedTasks[0]) )") )
    {
        __debugbreak();
    }
    if ( !overlappedTasks_2[slot].active
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    1146,
                    0,
                    "%s",
                    "overlappedTasks[slot].active") )
    {
        __debugbreak();
    }
    if ( overlappedTasks_2[slot].type != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    1147,
                    0,
                    "%s",
                    "overlappedTasks[slot].type == TASK_JOININGSESSION") )
    {
        __debugbreak();
    }
    TaskManager_ClearTask(&overlappedTasks_2[slot]);
    return 1;
}

void __cdecl Session_EndOngoingSessionTasks(SessionData_s *session)
{
    const char *v1; // eax
    taskCompleteResults started; // [esp+10h] [ebp-18h]
    bool done; // [esp+1Bh] [ebp-Dh]
    int tasknum; // [esp+24h] [ebp-4h]

    for ( tasknum = 0; tasknum < 32; ++tasknum )
    {
        if ( overlappedTasks_2[tasknum].active )
        {
            switch ( overlappedTasks_2[tasknum].type )
            {
                case 1:
                    if ( *((SessionData_s **)TaskManager_GetTaskData(&overlappedTasks_2[tasknum]) + 6) == session )
                    {
                        while ( overlappedTasks_2[tasknum].active )
                        {
                            if ( (unsigned int)Session_JoinSessionComplete(tasknum) >= 2 )
                                Com_PrintError(16, "EXE_ERROR_JOINING_SESSION");
                        }
                    }
                    break;
                case 2:
                    if ( *((SessionData_s **)TaskManager_GetTaskData(&overlappedTasks_2[tasknum]) + 1) == session )
                    {
                        done = 0;
                        while ( !done && overlappedTasks_2[tasknum].active )
                        {
                            started = Session_StartHostComplete(tasknum);
                            if ( started )
                            {
                                if ( started != TASK_COMPLETE )
                                    Com_PrintError(16, "EXE_ERROR_CREATING_SESSION");
                                done = 1;
                            }
                        }
                    }
                    break;
                case 5:
                    if ( session == &g_serverSession )
                    {
                        while ( overlappedTasks_2[tasknum].active )
                        {
                            if ( (unsigned int)Session_StartSessionComplete(tasknum) >= 2 )
                                Com_PrintError(16, "EXE_ERROR_STARTING_SESSION");
                        }
                    }
                    break;
                case 6:
                    if ( session == TaskManager_GetTaskData(&overlappedTasks_2[tasknum]) )
                    {
                        while ( overlappedTasks_2[tasknum].active )
                        {
                            if ( (unsigned int)Session_ModifyComplete(tasknum) >= TASK_ERROR )
                                Com_PrintError(16, "EXE_ERROR_MODIFYING_SESSION");
                        }
                    }
                    break;
                case 7:
                    while ( overlappedTasks_2[tasknum].active )
                    {
                        if ( (unsigned int)Session_EveryoneLeaveSessionComplete(tasknum) > 1 )
                            Com_PrintError(16, "EXE_ERROR_LEAVING_SESSION");
                    }
                    break;
                default:
                    v1 = va("Unknown session task type %i\n", overlappedTasks_2[tasknum].type);
                    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp", 1976, 0, v1) )
                        __debugbreak();
                    break;
            }
        }
    }
    LB_EndOngoingTasks();
}

taskCompleteResults __cdecl Session_StartHostComplete(int slot)
{
#ifdef KISAK_LIVE_STUBS
    bdTrulyRandomImpl *Instance; // eax
    XNADDR *p_hostAddress; // esi
    XNADDR *v4; // edi
    bdReference<bdCommonAddr> v5; // [esp+44h] [ebp-20h] BYREF
    XSESSION_INFO *sessionInfo; // [esp+48h] [ebp-1Ch]
    bdReference<bdCommonAddr> localAddr; // [esp+4Ch] [ebp-18h] BYREF
    taskCompleteResults result; // [esp+50h] [ebp-14h]
    int sessionCreateControllerIndex; // [esp+54h] [ebp-10h]
    SessionCreateData *createData; // [esp+58h] [ebp-Ch]
    overlappedTask *sessionCreateOverlappedIO; // [esp+5Ch] [ebp-8h]
    SessionData_s *session; // [esp+60h] [ebp-4h]

    session = 0;
    if ( slot >= 32
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    626,
                    0,
                    "%s",
                    "slot < static_cast<int>( sizeof(overlappedTasks) / sizeof(overlappedTasks[0]) )") )
    {
        __debugbreak();
    }
    if ( !overlappedTasks_2[slot].active
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    627,
                    0,
                    "%s",
                    "overlappedTasks[slot].active") )
    {
        __debugbreak();
    }
    if ( overlappedTasks_2[slot].type != 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    628,
                    0,
                    "%s",
                    "overlappedTasks[slot].type == TASK_CREATINGHOSTSESSION") )
    {
        __debugbreak();
    }
    sessionCreateOverlappedIO = &overlappedTasks_2[slot];
    if ( sessionCreateOverlappedIO->overlappedIO.m_ptr )
    {
        session = &g_serverSession;
        result = dwCreateSessionComplete(sessionCreateOverlappedIO);
        if ( result == TASK_COMPLETE )
        {
            session->sessionInfo.sessionID = *dwGetSessionSecID();
            session->sessionInfo.keyExchangeKey = g_securityKey;
            dwRegisterSecIDAndKey(&session->sessionInfo.sessionID, &session->sessionInfo.keyExchangeKey);
        }
    }
    else
    {
        if ( g_dwNetStatus != DW_NET_STARTED_LAN && g_dwNetStatus != DW_NET_STARTED_ONLINE )
        {
            dwNetPump();
            NET_Sleep(0xAu);
            if ( (float)((float)(int)Sys_Milliseconds() - sessionCreateOverlappedIO->startMS) <= 5000.0 )
                return 0;
            else
                return 2;
        }
        result = TASK_COMPLETE;
        createData = (SessionCreateData *)sessionCreateOverlappedIO->data;
        session = createData->session;
        dwGetLocalCommonAddr(&localAddr);
        if ( localAddr.m_ptr )
            bdCommonAddr::serialize(localAddr.m_ptr, createData->session->sessionInfo.hostAddress.addrBuff);
        Instance = bdSingleton<bdTrulyRandomImpl>::getInstance();
        bdTrulyRandomImpl::getRandomUByte8(Instance, g_securityKey.ab, 0x10u);
        createData->session->sessionInfo.keyExchangeKey = g_securityKey;
        createData->session->sessionInfo.sessionID = *dwGetSessionSecID();
        dwCreateSecIDAndKey(&createData->session->sessionInfo.sessionID, &createData->session->sessionInfo.keyExchangeKey);
        if ( !dwRegisterSecIDAndKey(
                        &createData->session->sessionInfo.sessionID,
                        &createData->session->sessionInfo.keyExchangeKey) )
            result = TASK_ERROR;
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&localAddr);
    }
    if ( result == TASK_COMPLETE )
    {
        sessionInfo = (XSESSION_INFO *)sessionCreateOverlappedIO->data;
        sessionCreateControllerIndex = overlappedTasks_2[slot].controllerIndex;
        dwGetLocalCommonAddr(&v5);
        if ( v5.m_ptr )
        {
            bdCommonAddr::serialize(v5.m_ptr, sessionInfo->hostAddress.addrBuff);
            p_hostAddress = &sessionInfo->hostAddress;
            v4 = &session->sessionInfo.hostAddress;
            memcpy(&session->sessionInfo.hostAddress, &sessionInfo->hostAddress, 0x18u);
            v4->addrBuff[24] = p_hostAddress->addrBuff[24];
        }
        g_matchmakingInfo->m_active = 1;
        TaskManager_ClearTask(sessionCreateOverlappedIO);
        LiveStorage_UploadStats();
        session->keysGenerated = 1;
        session->sessionStartCalled = 0;
        if ( !xblive_basictraining->current.enabled )
            Session_QoSListenStart(session);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v5);
    }
    else if ( result )
    {
        TaskManager_ClearTask(sessionCreateOverlappedIO);
    }
    if ( result == TASK_ERROR )
        Com_Error(ERR_DROP, "EXE_ERROR_CREATING_SESSION");
    return result;
#else
    return TASK_NOTCOMPLETE;
#endif
}

int __cdecl Session_EveryoneLeaveSessionComplete(int slot)
{
    if ( slot >= 32
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    1164,
                    0,
                    "%s",
                    "slot < static_cast<int>( sizeof(overlappedTasks) / sizeof(overlappedTasks[0]) )") )
    {
        __debugbreak();
    }
    if ( !overlappedTasks_2[slot].active
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    1168,
                    0,
                    "%s",
                    "overlappedTasks[slot].active") )
    {
        __debugbreak();
    }
    if ( overlappedTasks_2[slot].type != 7
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp",
                    1169,
                    0,
                    "%s",
                    "overlappedTasks[slot].type == TASK_LEAVINGSESSION") )
    {
        __debugbreak();
    }
    TaskManager_ClearTask(&overlappedTasks_2[slot]);
    return 1;
}

void __cdecl Live_FinishOngoingSessionJoinTasksForXUID(unsigned __int64 player)
{
#ifdef KISAK_LIVE_SERVICE
    unsigned int v1; // edx
    int Xuid; // [esp+Ch] [ebp-18h]
    int type; // [esp+14h] [ebp-10h]
    taskCompleteResults result; // [esp+18h] [ebp-Ch]
    int tasknum; // [esp+20h] [ebp-4h]

    for ( tasknum = 0; tasknum < 32; ++tasknum )
    {
        if ( overlappedTasks_2[tasknum].active )
        {
            type = overlappedTasks_2[tasknum].type;
            if ( type == 1 )
            {
                if ( *((_QWORD *)TaskManager_GetTaskData(&overlappedTasks_2[tasknum]) + 2) == player )
                {
                    do
                    {
                        result = (taskCompleteResults)Session_JoinSessionComplete(tasknum);
                        if ( (unsigned int)result >= TASK_ERROR )
                            Com_PrintError(16, "EXE_ERROR_JOINING_SESSION");
                    }
                    while ( result == TASK_NOTCOMPLETE );
                }
            }
            else if ( type == 2 )
            {
                Xuid = Live_GetXuid(overlappedTasks_2[tasknum].controllerIndex);
                if ( __PAIR64__(v1, Xuid) == player )
                {
                    while ( overlappedTasks_2[tasknum].active )
                    {
                        if ( (unsigned int)Session_StartHostComplete(tasknum) >= TASK_ERROR )
                            Com_PrintError(16, "EXE_ERROR_CREATING_SESSION");
                    }
                }
            }
        }
    }
#endif
}

void __cdecl Live_CheckOngoingSessionTasks()
{
    const char *v0; // eax
    int tasknum; // [esp+18h] [ebp-4h]

    for ( tasknum = 0; tasknum < 32; ++tasknum )
    {
        if ( overlappedTasks_2[tasknum].active )
        {
            switch ( overlappedTasks_2[tasknum].type )
            {
                case 1:
                    if ( (unsigned int)Session_JoinSessionComplete(tasknum) >= 2 )
                        Com_PrintError(16, "EXE_ERROR_JOINING_SESSION");
                    break;
                case 2:
                    if ( (unsigned int)Session_StartHostComplete(tasknum) >= TASK_ERROR )
                        Com_PrintError(16, "EXE_ERROR_CREATING_SESSION");
                    break;
                case 4:
                    continue;
                case 5:
                    if ( (unsigned int)Session_StartSessionComplete(tasknum) >= 2 )
                        Com_PrintError(16, "EXE_ERROR_STARTING_SESSION");
                    break;
                case 6:
                    if ( (unsigned int)Session_ModifyComplete(tasknum) >= TASK_ERROR )
                        Com_PrintError(16, "EXE_ERROR_MODIFYING_SESSION");
                    break;
                case 7:
                    if ( (unsigned int)Session_EveryoneLeaveSessionComplete(tasknum) > 1 )
                        Com_PrintError(16, "EXE_ERROR_LEAVING_SESSION");
                    break;
                default:
                    v0 = va("Unknown session task type %i\n", overlappedTasks_2[tasknum].type);
                    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_sessions_win.cpp", 2155, 0, v0) )
                        __debugbreak();
                    break;
            }
        }
    }
    Session_ManageGraveYard();
}

void Session_ManageGraveYard()
{
    int slot; // [esp+0h] [ebp-Ch]
    SessionData_s *session; // [esp+8h] [ebp-4h]

    if (!CG_IsShowingZombieMap())
    {
        for (slot = 0; slot < 6; ++slot)
        {
            if (sessionGraveYard[slot].active)
            {
                session = &sessionGraveYard[slot].sessionData;
                if (!Session_SessionTasksInProgress(session))
                {
                    Session_EndGameSession(session);
                    if (!Session_SessionTasksInProgress(session) && !Session_SessionTasksInProgress(session))
                    {
                        Session_DeleteHandle(&session->sessionHandle);
                        session->sessionHandle = 0;
                        sessionGraveYard[slot].active = 0;
                    }
                }
            }
        }
    }
}

void __cdecl Session_Init()
{
    memset(sessionJoinData, 0, sizeof(sessionJoinData));
    memset(sessionCreateData, 0, sizeof(sessionCreateData));
    TaskManager_ClearOverlappedTasks(overlappedTasks_2);
    g_serverSession.sessionName = (char*)"gameSession";
    g_serverSession.registerUsersWithVoice = 1;
    matchmaking_debug = _Dvar_RegisterBool("matchmaking_debug", 1, 0, "Enable matchmaking debugging information");
}

