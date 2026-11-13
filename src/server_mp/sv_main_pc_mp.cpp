#include "sv_main_pc_mp.h"
#include <qcommon/common.h>
#include "sv_main_mp.h"
#include <client_mp/sv_client_mp.h>
#include "sv_init_mp.h"
#include <game_mp/g_main_mp.h>
#include <universal/offline.h>
#include <game_mp/pregame.h>
#include <ui/ui_playlists.h>
#include <win32/win_main.h>
#include <cstring>
#include <win32/win_shared.h>
#include <game_mp/g_cmds_mp.h>
#include <client/cl_console.h>
#include <client_mp/cl_main_mp.h>

int s_region = 1;
int s_licensetype = -1;


int s_LSGTime;
bool sessionCreated;
bool sessionCreateFinished;
int s_numslotpasses;
reservedslot_t s_reservedSlots[6];

void __cdecl SV_ResetOnlineState()
{
    Com_Printf(0, "resetting state..\n");
    sessionCreated = 0;
    sessionCreateFinished = 0;
}

int __cdecl SV_GetRegion()
{
    return s_region;
}

void __cdecl SV_SetRegion(int region)
{
    s_region = region;
}

void __cdecl SV_SetTime(int time)
{
    s_LSGTime = time;
}

int __cdecl SV_GetTime()
{
    if ( !s_LSGTime
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_pc_mp.cpp",
                    199,
                    0,
                    "%s",
                    "0 != s_LSGTime") )
    {
        __debugbreak();
    }
    return s_LSGTime;
}

void __cdecl SV_OnlineUserDisconnected(unsigned __int64 userID)
{
    int i; // [esp+8h] [ebp-4h]

    if ( svs.clients )
    {
        for ( i = 0; i < com_maxclients->current.integer; ++i )
        {
            if ( svs.clients[i].header.state >= CS_CONNECTED
                && !svs.clients[i].bIsTestClient
                && !svs.clients[i].bIsDemoClient
                && svs.clients[i].onlineUserID == userID )
            {
                Com_Printf(
                    0,
                    "Treason uncloaked! User %s is still connected, but the LSG says they're gone!\n",
                    svs.clients[i].name);
                SV_DropClient(&svs.clients[i], "EXE_DISCONNECTED", 1, 1);
                return;
            }
        }
    }
    else
    {
        Com_DPrintf(15, "Got user disconnected notification, but svs.clients is NULL.\n");
    }
}

int __cdecl SV_GetSlotForPasswordIfFree(const char *password)
{
    char *v1; // eax
    int i; // [esp+0h] [ebp-8h]
    int slot; // [esp+4h] [ebp-4h]

    slot = -1;
    for ( i = 0; i < s_numslotpasses; ++i )
    {
        if ( !I_strcmp(password, s_reservedSlots[i].pass) )
        {
            if ( !s_reservedSlots[i].inUse
                || (v1 = Info_ValueForKey(svs.clients[i].userinfo, "password"), I_strcmp(v1, s_reservedSlots[i].pass)) )
            {
                s_reservedSlots[i].inUse = 1;
                return i;
            }
            else
            {
                Com_Printf(15, "Reserved slot %d in use by %s.\n", i, svs.clients[i].name);
            }
            return slot;
        }
    }
    return slot;
}

void __cdecl SV_FreeReservedSlot(int slot)
{
    if ( (slot < 0 || slot > s_numslotpasses)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_pc_mp.cpp",
                    284,
                    0,
                    "%s",
                    "slot >= 0 && slot <= s_numslotpasses") )
    {
        __debugbreak();
    }
    if ( !s_reservedSlots[slot].inUse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_pc_mp.cpp",
                    285,
                    0,
                    "%s",
                    "s_reservedSlots[slot].inUse") )
    {
        __debugbreak();
    }
    if ( slot >= 0 && slot <= s_numslotpasses )
        s_reservedSlots[slot].inUse = 0;
}

int __cdecl SV_DropClientForReservedSlot(const char *password)
{
    int slot; // [esp+0h] [ebp-4h]

    slot = SV_GetSlotForPasswordIfFree(password);
    if ( !Demo_IsEnabled() || slot )
    {
        if ( slot >= 0 )
        {
            if ( (unsigned int)slot >= com_maxclients->current.integer
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_pc_mp.cpp",
                            309,
                            0,
                            "slot doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                            slot,
                            com_maxclients->current.integer) )
            {
                __debugbreak();
            }
            SV_DropClient(&svs.clients[slot], "EXE_SERVERISFULL", 1, 1);
        }
        return slot;
    }
    else
    {
        Com_DPrintf(15, "Reserved slot 0 can not be used when film recording is enabled.\n");
        return -1;
    }
}

void __cdecl SV_SetLicenseType(int licensetype)
{
    if ( licensetype == 5 )
        Com_Error(ERR_FATAL, "Incorrect licensetype for server!\n");
    else
        s_licensetype = licensetype;
}

int __cdecl SV_GetLicenseType()
{
    if ( Dvar_GetInt("sv_forcelicensetype") )
        return Dvar_GetInt("sv_forcelicensetype");
    else
        return s_licensetype;
}

bool __cdecl SV_IsWagerServer()
{
    return xblive_wagermatch && xblive_wagermatch->current.enabled;
}

const char *__cdecl SV_GetMapName()
{
    return sv_mapname->current.string;
}

const char *__cdecl SV_GetHostName()
{
    return sv_hostname->current.string;
}

unsigned int __cdecl SV_GetClientCount()
{
    int i; // [esp+0h] [ebp-8h]
    unsigned int clientCount; // [esp+4h] [ebp-4h]

    clientCount = 0;
    if ( com_sv_running && com_sv_running->current.enabled )
    {
        if ( xblive_wagermatch && xblive_wagermatch->current.enabled && Pregame_GetState() == PREGAME_GAMESTARTED )
        {
            return sv_maxclients->current.unsignedInt;
        }
        else
        {
            for ( i = 0; i < com_maxclients->current.integer; ++i )
            {
                if ( svs.clients[i].header.state >= CS_CONNECTED )
                    ++clientCount;
            }
        }
    }
    return clientCount;
}

bool __cdecl SV_HasInfoChanged()
{
    return false;
}

bool __cdecl SV_IsServerRanked(int licensetype)
{
    if ( Dvar_GetBool("sv_forceunranked") )
        return 0;
    return licensetype == 4 || licensetype == 2;
}

void __cdecl SV_MasterHeartbeat(int controllerIndex)
{
}

void SV_ReadWhiteListfile()
{
    char *v0; // eax
    _iobuf *fp; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    i = 0;
    fp = fopen("reservedslots.txt", "r");
    if ( fp )
    {
        while ( !feof(fp) && i < sv_numreservedslots->current.integer && i < com_maxclients->current.integer )
        {
            fgets(s_reservedSlots[i].pass, 24, fp);
            v0 = strchr(s_reservedSlots[i].pass, 0xAu);
            if ( v0 )
                *v0 = 0;
            ++i;
        }
        s_numslotpasses = i;
    }
    Com_DPrintf(15, "Parsed %i reserved slot passwords\n", s_numslotpasses);
}

int s_onlineBackOff = 2000;
int s_lastOnlineGoodUpdate;
int SV_QuitIfNeeded()
{
    int result; // eax

    if (!s_lastOnlineGoodUpdate || (int)(Sys_Milliseconds() - s_lastOnlineGoodUpdate) > 10800000)
    {
        Com_PrintWarning(0, "Session creation failed. Killing server\n");
        Sys_Quit();
    }
    result = 2 * s_onlineBackOff;
    s_onlineBackOff *= 2;
    return result;
}

void __cdecl SV_FlushRedirect(char *outputbuf)
{
    char c; // [esp+13h] [ebp-49Dh]
    char buf[1168]; // [esp+18h] [ebp-498h] BYREF
    int len; // [esp+4ACh] [ebp-4h]

    len = strlen(outputbuf);
    while ( len > 1156 )
    {
        c = outputbuf[1156];
        outputbuf[1156] = 0;
        buf[0] = 1;
        Com_sprintf(&buf[1], 0x48Bu, "print\n%s", outputbuf);
        NET_OutOfBandPrint(NS_SERVER, svs.redirectAddress, buf);
        len -= 1156;
        outputbuf += 1156;
        *outputbuf = c;
    }
    buf[0] = 1;
    Com_sprintf(&buf[1], 0x48Bu, "print\n%s", outputbuf);
    NET_OutOfBandPrint(NS_SERVER, svs.redirectAddress, buf);
}

static int lasttime;
void __cdecl SVC_RemoteCommand(netadr_t from)
{
    char *v1; // eax
    char *v2; // eax
    const char *v3; // eax
    int v4; // [esp-Ch] [ebp-C2Ch]
    const char *v5; // [esp-4h] [ebp-C24h]
    const char *v6; // [esp-4h] [ebp-C24h]
    char remaining[1024]; // [esp+18h] [ebp-C08h] BYREF
    char sv_outputbuf[2032]; // [esp+418h] [ebp-808h] BYREF
    int valid; // [esp+C08h] [ebp-18h]
    int len; // [esp+C0Ch] [ebp-14h]
    const char *password; // [esp+C10h] [ebp-10h]
    int time; // [esp+C14h] [ebp-Ch]
    int i; // [esp+C18h] [ebp-8h]

    time = Sys_Milliseconds();
    if ( !lasttime || time - lasttime >= 500 )
    {
        lasttime = time;
        password = SV_Cmd_Argv(1);
        if ( *(_BYTE *)rcon_password->current.integer && !strcmp(password, rcon_password->current.string) )
        {
            valid = 1;
            v6 = SV_Cmd_Argv(2);
            v2 = NET_AdrToStringOnline(from);
            Com_Printf(15, "Rcon from %s:\n%s\n", v2, v6);
        }
        else
        {
            valid = 0;
            v5 = SV_Cmd_Argv(2);
            v1 = NET_AdrToStringOnline(from);
            Com_Printf(15, "Bad rcon from %s:\n%s\n", v1, v5);
        }
        svs.redirectAddress = from;
        Com_BeginRedirect(sv_outputbuf, 0x7F0u, SV_FlushRedirect);
        if ( *(_BYTE *)rcon_password->current.integer )
        {
            if ( valid )
            {
                len = 0;
                for ( i = 2; i < SV_Cmd_Argc(); ++i )
                {
                    v4 = len;
                    v3 = SV_Cmd_Argv(i);
                    len = Com_AddToString(v3, remaining, v4, 1024, 1);
                    len = Com_AddToString(" ", remaining, len, 1024, 0);
                }
                if ( len < 1024 )
                {
                    remaining[len] = 0;
                    Con_Restricted_ExecuteBuf(0, 0, remaining);
                }
            }
            else if ( *password )
            {
                Com_Printf(15, "Invalid password.\n");
            }
            else
            {
                Com_Printf(15, "You must log in with 'rcon login <password>' before using 'rcon'.\n");
            }
        }
        else
        {
            Com_Printf(15, "The server must set 'rcon_password' for clients to use 'rcon'.\n");
        }
        Com_EndRedirect();
    }
}

void __cdecl SV_MatchEnd()
{
    void *v0; // ecx

    if ( onlinegame )
    {
        if ( onlinegame->current.enabled )
        {
            Com_DPrintf(0, "\n*******SERVER: SV_MatchEnd called, uploading leaderboards.\n");
            SV_CommitClientLeaderboards();
        }
    }
}

void __cdecl SV_SysLog_LogMessage(int severity, const char *msg)
{
}

void __cdecl SV_SysLog_LogMessage_f()
{
    const char *v0; // eax
    const char *message; // [esp+18h] [ebp-8h]
    int severity; // [esp+1Ch] [ebp-4h]

    if ( Cmd_Argc() == 3 )
    {
        message = Cmd_Argv(2);
        v0 = Cmd_Argv(1);
        severity = atoi(v0);
        SV_SysLog_LogMessage(severity, message);
    }
    else
    {
        Com_Printf(0, "Usage: logmessage severity message\n");
    }
}

unsigned __int64 __cdecl SV_GetOwnerID()
{
    unsigned __int64 retval; // [esp+0h] [ebp-8h] BYREF

    retval = 0;
    if ( sv_ownerid && *(_BYTE *)sv_ownerid->current.integer )
        StringToXUID(sv_ownerid->current.string, &retval);
    return retval;
}

bool __cdecl SV_CanLoadCustomGameType()
{
    int LicenseType; // eax

    LicenseType = SV_GetLicenseType();
    return !SV_IsServerRanked(LicenseType) && !playlist_enabled->current.enabled;
}

void __cdecl SV_RegisterRconKey_f()
{
}

