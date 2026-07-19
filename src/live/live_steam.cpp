#include "live_steam.h"
#include <win32/win_splash.h>
#include <qcommon/common.h>
#include <win32/win_localize.h>
#include <Windows.h>
#ifdef _WIN32
#include <shellapi.h>
#endif
#include <win32/win_main.h>
#include <steam/steamclientpublic.h>
#include <steam/steam_api.h>
#include <win32/win_net.h>
#include "live_friends_pc.h"
#include "live_steam_server.h"
#include "live_steam_achievements.h"
#include "live_win.h"
#include "live_steamp2p.h"
#include "live_steam_client.h"

const dvar_t *livesteam_sv_vac;

bool g_liveSteamInitialized;
LiveSteamP2P s_SteamP2P;
LiveSteamClient *g_liveLocalSteamClient;

void __cdecl SteamAPIDebugTextHook(int severity, const char *pchDebugText)
{
    if ( severity != 1 || I_strcmp(pchDebugText, "[S_API WARN] Invalid game server IP passed to InitiateGameConnection\n") )
        Com_PrintWarning(0, "Steam Warning sev %d: %s", severity, pchDebugText);
}

void __cdecl    LiveSteam_Quit(bool showErrorMessage)
{
    char *v1; // eax
    char *v2; // [esp-8h] [ebp-8h]

    Sys_HideSplashWindow();
    if ( showErrorMessage )
    {
        v2 = Win_LocalizeRef("WIN_ERROR");
        v1 = Win_LocalizeRef("WIN_NO_STEAM");
        MessageBoxA(0, v1, v2, 0x10u);
    }
    Sys_Quit();
}

void __cdecl LiveSteam_CheckAccess()
{
    ;
}

bool __cdecl LiveSteam_IsInitialized()
{
    return g_liveSteamInitialized;
}

char __cdecl LiveSteam_IsClientSignedInLocally()
{
    if (SteamUser())
    {
        return SteamUser()->GetSteamID().IsValid();
    }

    return false;
}

bool __cdecl LiveSteam_IsClientSignedInOnline()
{
    if ( !LiveSteam_IsClientSignedInLocally() )
        return 0;
    //v1 = SteamUser();
    //return (*(int (__thiscall **)(int, int))(*(unsigned int *)v1 + 4))(v1, v1);
    return SteamUser()->BLoggedOn();
}

unsigned __int64 __cdecl LiveSteam_GetClientIDAsXUID()
{
    CSteamID *v1; // eax
    ISteamUser *v2; // eax
    ISteamUser *v3; // [esp+4h] [ebp-20h]
    _BYTE v4[8]; // [esp+14h] [ebp-10h] BYREF
    _DWORD v5[2]; // [esp+1Ch] [ebp-8h] BYREF

    if (LiveSteam_IsClientSignedInLocally())
    {
        if (!SteamUser()
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_steam.cpp", 141, 0, "%s", "SteamUser()"))
        {
            __debugbreak();
        }

        iassert(SteamUser()->GetSteamID().IsValid());
        //v3 = SteamUser();
        //v1 = v3->GetSteamID(v5);
        //if (!CSteamID::IsValid(v1)
        //    && !Assert_MyHandler(
        //        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_steam.cpp",
        //        142,
        //        0,
        //        "%s",
        //        "SteamUser()->GetSteamID().IsValid()"))
        //{
        //    __debugbreak();
        //}
        //v2 = SteamUser();
        //return v2->GetSteamID(v2, v4)->m_steamid;
        return SteamUser()->GetSteamID().GetAccountID(); // KISAKTODO: prob wrong
    }
    else
    {
        return 0;
    }
}

const char *__cdecl LiveSteam_GetCountryCode()
{
    ISteamUtils *v1; // eax
    const char *loc; // [esp+4h] [ebp-4h]

    loc = Dvar_GetString("cl_forceloc");
    if (loc && *loc)
        return loc;

    return SteamUtils()->GetIPCountry();
    //v1 = SteamUtils();
    //return (v1->GetIPCountry)(v1, v1);
}

unsigned __int64 __cdecl LiveSteam_GetUid()
{
    ISteamUser *v1; // [esp+0h] [ebp-18h]
    _BYTE v2[8]; // [esp+10h] [ebp-8h] BYREF

    if (SteamUser())
    {
        //v1 = SteamUser();
        //return v1->GetSteamID(v1, v2)->m_steamid;
        return SteamUser()->GetSteamID().GetAccountID(); // KISAKTODO: prob wrong
    }
    else
    {
        return 0;
    }
}

int g_ShellExecuteErr;
volatile unsigned int g_ShellExecuteInProgress;
unsigned int __stdcall ShellExecuteThredProc(const char *lpParam)
{
    char currentDirectory[268]; // [esp+0h] [ebp-110h] BYREF

    GetCurrentDirectoryA(0x104u, currentDirectory);
    g_ShellExecuteErr = (int)ShellExecuteA(0, "open", lpParam, 0, 0, 1);
    _InterlockedExchange(&g_ShellExecuteInProgress, 0);
    return 0;
}

bool __cdecl LiveSteam_LaunchOtherApp(const char *cmd)
{
    const char *v1; // eax
    const char *url; // [esp+0h] [ebp-Ch]
    int result; // [esp+4h] [ebp-8h]
    DWORD dwThreadID; // [esp+8h] [ebp-4h] BYREF

    if ( !I_stricmp(cmd, "CoDSP_rd.exe") )
    {
        url = "steam://run/42700";
        goto LABEL_8;
    }
    if ( !I_stricmp(cmd, "CoDMP_rd.exe") )
    {
        url = "steam://run/42710";
LABEL_8:
        g_ShellExecuteInProgress = 1;
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ShellExecuteThredProc, (LPVOID)url, 0, &dwThreadID);
        while ( g_ShellExecuteInProgress )
            NET_Sleep(10);
        result = g_ShellExecuteErr;
        if ( g_ShellExecuteErr <= 32 )
            Com_Printf_NoFilter((char*)"Steam_LaunchOtherApp: ShellExecute return less than 32, '%d'\n", g_ShellExecuteErr);
        return result > 32;
    }
    Com_Printf_NoFilter((char *)"Steam_LaunchOtherApp: Don't know how to launch '%s'\n", cmd);
    v1 = va("Unknown exe '%s' to launch through Steam.\n", cmd);
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_steam.cpp", 195, 0, v1) )
        __debugbreak();
    return 0;
}

void __cdecl LiveSteam_ShowStore()
{
#if 0
    ISteamUtils *v0; // eax
    int v1; // eax
    ISteamFriends *steamFriends; // [esp+4h] [ebp-4h]

    if (g_liveSteamInitialized)
    {
        steamFriends = SteamFriends();
        if (steamFriends)
        {
            //v0 = SteamUtils();
            //v1 = (v0->GetAppID)(v0, v0);
            steamFriends->ActivateGameOverlayToStore(SteamUtils()->GetAppID());
        }
    }
#endif
}

void __cdecl LiveSteam_PopOverlayForSteamID(unsigned __int64 steamID)
{
    ISteamFriends *v1; // [esp+0h] [ebp-Ch]
    CSteamID sID; // [esp+4h] [ebp-8h] BYREF

    //CSteamID::CSteamID(&sID);
    //sID.m_steamid.m_comp = steamID;
    sID.SetFromUint64(steamID);
    //v1 = SteamFriends();
    //(v1->ActivateGameOverlayToUser)(v1, "steamid", *&sID.m_steamid.m_comp, *(&sID.m_steamid.m_comp + 1));

    SteamFriends()->ActivateGameOverlayToUser("steamid", sID);
}

void __cdecl LiveSteam_Init()
{
    LiveSteam_CreateSteamAppIdFile();
    g_liveSteamInitialized = 1;
    livesteam_sv_vac = _Dvar_RegisterBool("sv_vac", 1, 1u, "Enable VAC on this server");
}

void __cdecl LiveSteam_TestFriends_f()
{
#if 0 // KISAKTODO who cares
    int i; // [esp+Ch] [ebp-54h]
    unsigned __int64 dwID; // [esp+10h] [ebp-50h]
    char buf[64]; // [esp+18h] [ebp-48h] BYREF
    int n; // [esp+5Ch] [ebp-4h]

    Com_Printf_NoFilter("STEAM: Firends Functionality Testing\n");
    n = LiveSteam_GetNumFriends();
    Com_Printf_NoFilter("STEAM: LiveSteam_GetNumFriends(): %d\n", n);
    while ( n-- )
    {
        LiveSteam_GetFriendPersonaName(n, buf, 64);
        Com_Printf_NoFilter("STEAM: LiveSteam_GetFriendPersonaName( %d ): %s\n", n, buf);
        i = LiveSteam_GetFriendPersonaStateByIndex(n);
        Com_Printf_NoFilter(
            "STEAM: LiveSteam_GetFriendPersonaStateByIndex( %d ): %s\n",
            n,
            &aOffline[20 * (i == 3) + 15 * (i == 2) + 8 * (i == 1)]);
        dwID = LiveSteam_GetFriendXuid(n);
        Com_Printf_NoFilter("STEAM: bdOnlineUserID is: %I64x\n", dwID);
    }
    n = LiveSteam_GetNumFriendsOnTheServer();
    Com_Printf_NoFilter("STEAM: LiveSteam_GetNumFriendsOnTheServer(): %d\n", n);
    while ( n-- )
    {
        LiveSteam_GetFriendOnTheServerPersonaName(n, buf, 64);
        Com_Printf_NoFilter("STEAM: LiveSteam_GetFriendOnTheServerPersonaName( %d ): %s\n", n, buf);
    }
#endif
}

_iobuf *LiveSteam_CreateSteamAppIdFile()
{
    _iobuf *result; // eax
    _iobuf *f; // [esp+0h] [ebp-4h]

    DeleteFileA("steam_appid.txt");
    result = fopen("noship_create_steam_appid.txt", "r");
    if ( result )
    {
        fclose(result);
        result = fopen("steam_appid.txt", "w");
        f = result;
        if ( result )
        {
            fprintf(result, "42710");
            return (_iobuf *)fclose(f);
        }
    }
    return result;
}

void __cdecl LiveSteam_Shutdown()
{
    if ( g_liveSteamInitialized )
    {
        LiveSteam_Server_Shutdown();
        LiveSteam_Achievements_Shutdown();
    }
    g_liveSteamInitialized = 0;
}

void __cdecl LiveSteam_CheckForP2PMessages()
{
    int v0; // edx
    unsigned __int8 messageBuffer[68]; // [esp+8h] [ebp-58h] BYREF
    CSteamID remoteID; // [esp+50h] [ebp-10h] BYREF
    int numMessages; // [esp+58h] [ebp-8h]
    unsigned int messagesize; // [esp+5Ch] [ebp-4h] BYREF

    numMessages = 0;
    messagesize = 0;
    //CSteamID::CSteamID(&remoteID);

    while (s_SteamP2P.IsDataAvailable(&messagesize))
    {
        v0 = numMessages++;
        if ( v0 >= 10 )
            break;
        //LiveSteamP2P::ReadPacket(&s_SteamP2P, messageBuffer, 0x40u, &messagesize, &remoteID);
        s_SteamP2P.ReadPacket(messageBuffer, 64, &messagesize, &remoteID);
        Live_DispatchP2PMessage(messageBuffer, messagesize, remoteID.ConvertToUint64());
    }
}

void __cdecl LiveSteam_SendP2PMessage(unsigned __int64 uid, unsigned __int8 *payload, unsigned int payloadsize)
{
    //LiveSteamP2P::SendPacketToSteamID(&s_SteamP2P, (CSteamID)uid, payload, payloadsize);
    s_SteamP2P.SendPacketToSteamID((CSteamID)uid, payload, payloadsize);
}

void __cdecl LiveSteam_Frame()
{
    LiveSteam_Achievements_Frame();
    LiveSteam_Server_Frame();
    Friends_Update();
    SteamAPI_RunCallbacks();
}

void __cdecl LiveSteam_RunCallbacks()
{
    SteamAPI_RunCallbacks();
}

char shorName[64];
const char *__cdecl LiveSteam_GetClientPersonaName(bool shortName)
{
    const char *s; // [esp+4h] [ebp-4h]

    if (!SteamFriends())
        return "";

    s = SteamFriends()->GetPersonaName();
    if (!shortName)
        return s;

    Com_sprintf(shorName, 0x40u, "%.16s", s);
    return shorName;
}

char __cdecl LiveSteam_DWUserNameFromSteamID(char *dwUserName)
{
    ISteamUser *v2; // [esp+0h] [ebp-18h]
    _BYTE v3[8]; // [esp+8h] [ebp-10h] BYREF
    unsigned __int64 steamid; // [esp+10h] [ebp-8h]

    if (g_liveSteamInitialized)
    {
        //v2 = SteamUser();
        //steamid = v2->GetSteamID(v2, v3)->m_steamid.m_unAll64Bits;
        sprintf_s(dwUserName, 0x40u, "%s%I64x", "steam", SteamUser()->GetSteamID().ConvertToUint64());
        return 1;
    }
    else
    {
        *dwUserName = 0;
        return 0;
    }
}

char __cdecl LiveSteam_AuthRequestTicket(const void *authBlob, unsigned int authBlobSize)
{
    if ( !g_liveSteamInitialized )
        return 0;
    if ( (!authBlob || !authBlobSize)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_steam.cpp",
                    573,
                    0,
                    "%s",
                    "authBlob && authBlobSize") )
    {
        __debugbreak();
    }
    //LiveSteamClient::RequestEncryptedAppTicket(g_liveLocalSteamClient, authBlob, authBlobSize);
    g_liveLocalSteamClient->RequestEncryptedAppTicket(authBlob, authBlobSize);
    return 1;
}

char __cdecl LiveSteam_AuthGetRequestedTicket(void *ticketBuf, unsigned int ticketBufSize, unsigned int *ticketSize)
{
    if (g_liveLocalSteamClient->resultOnRequestEncryptedAppTicket == k_EResultOK)
    {
        //return LiveSteamClient::GetRetrievedEncryptedAppTicket(g_liveLocalSteamClient, ticketBuf, ticketBufSize, ticketSize);
        return g_liveLocalSteamClient->GetRetrievedEncryptedAppTicket(ticketBuf, ticketBufSize, ticketSize);
    }
    else
        return 0;
}

int __cdecl LiveSteam_Client_ConnectToSteamServer(unsigned __int64 serverID, void *authBlob, unsigned int bufferSize)
{
#if 0
    int v4; // [esp-14h] [ebp-24h] BYREF
    ISteamUser *v5; // [esp+4h] [ebp-Ch]
    int *v6; // [esp+8h] [ebp-8h]

    Com_DPrintf(14, "Steam CL_ConnectToSteamServer\n");
    v5 = SteamUser();
    v6 = &v4;
    //return (v5->InitiateGameConnection)(v5, authBlob, bufferSize, serverID, HIDWORD(serverID), 0, 0, 1);
    return SteamUser()->InitiateGameConnection(authBlob, bufferSize, serverID, 0, 0, true);
#else
    return 0;
#endif
}

void LiveSteam_Client_SteamDisconnect()
{
#if 0
    //SteamUser(this);
    //(*(void (__thiscall **)(unsigned int, unsigned int, unsigned int))(MEMORY[0] + 16))(0, 0, 0);
    SteamUser()->TerminateGameConnection(0, 0);
#endif
}

