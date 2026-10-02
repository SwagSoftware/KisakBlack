#include "ui_server.h"
#include "ui_shared.h"
#include <client_mp/cl_ui_pc_mp.h>
#include "ui_main_pc.h"
#include <ui_mp/ui_main_mp.h>
#include <game_mp/ui_gameinfo_mp.h>
#include <qcommon/com_clients.h>
#include <universal/com_shared.h>
#include <live/live_win.h>
#include <win32/win_shared.h>

int ui_toolTipColumns[8] = { 0, 1, 3, 2, 4, 5, 6, 7 };
int ui_toolTipCount = 8;

int ui_serverFilterType;
int ui_showToolTipTime;
bool ui_toolAllowTimer;
char info_8[1024];

void __cdecl UI_ClearDisplayedServers()
{
    sharedUiInfo.serverStatus.numDisplayServers = 0;
    sharedUiInfo.serverStatus.numPlayersOnServers = 0;
    sharedUiInfo.serverStatus.serverCount = LAN_GetServerCount(ui_netSource->current.integer);
    memset(sharedUiInfo.serverStatus.displayServersFriendNameInd, 0, 0x100u);
}

int numclean;
void __cdecl UI_BuildServerDisplayList(int localClientNum, uiInfo_s *uiInfo, int force)
{
    const char *String; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    char *v11; // eax
    char *v12; // eax
    char *v13; // eax
    char *v14; // eax
    char *v15; // eax
    char *v16; // eax
    char *v17; // eax
    char *v18; // eax
    gameTypeInfo *v19; // [esp-4h] [ebp-460h]
    const char *v20; // [esp-4h] [ebp-460h]
    char v21; // [esp+13h] [ebp-449h]
    char *motd; // [esp+18h] [ebp-444h]
    char *v23; // [esp+1Ch] [ebp-440h]
    int filter; // [esp+30h] [ebp-42Ch] BYREF
    const char *mod; // [esp+38h] [ebp-424h]
    char info[1024]; // [esp+3Ch] [ebp-420h] BYREF
    int ping; // [esp+440h] [ebp-1Ch]
    int maxClients; // [esp+444h] [ebp-18h]
    int len; // [esp+448h] [ebp-14h]
    int dirty; // [esp+44Ch] [ebp-10h]
    int i; // [esp+450h] [ebp-Ch]
    int clients; // [esp+454h] [ebp-8h]
    int count; // [esp+458h] [ebp-4h]

    if (force || uiInfo->uiDC.realTime > sharedUiInfo.serverStatus.nextDisplayRefresh)
    {
        if (force == 2)
            force = 0;
        String = Dvar_GetString("cl_motdString");
        I_strncpyz(sharedUiInfo.serverStatus.motd, String, 1024);
        len = strlen(sharedUiInfo.serverStatus.motd);
        if (!len)
        {
            v5 = UI_SafeTranslateString("EXE_COD_MULTIPLAYER");
            v23 = va("%s - %s", v5, "1.0");
            motd = sharedUiInfo.serverStatus.motd;
            do
            {
                v21 = *v23;
                *motd++ = *v23++;
            } while (v21);
            len = strlen(sharedUiInfo.serverStatus.motd);
        }
        if (len != sharedUiInfo.serverStatus.motdLen)
        {
            sharedUiInfo.serverStatus.motdLen = len;
            sharedUiInfo.serverStatus.motdWidth = -1;
        }
        if (force)
        {
            numclean = 0;
            UI_ClearDisplayedServers();
            if (sharedUiInfo.serverStatus.currentServer >= 0)
                Menu_SetFeederSelection(localClientNum, &uiInfo->uiDC, 0, 2, 0, 0);
            LAN_MarkServerDirty(ui_netSource->current.integer, -1, 1u);
        }
        count = LAN_GetServerCount(ui_netSource->current.integer);
        if (LAN_WaitServerResponse())
        {
            UI_ClearDisplayedServers();
            sharedUiInfo.serverStatus.nextDisplayRefresh = uiInfo->uiDC.realTime + 500;
        }
        else
        {
            UI_ServersSort(sharedUiInfo.serverStatus.sortKey | 0x100000000LL);
            dirty = 0;
            for (i = 0; i < count; ++i)
            {
                if (LAN_ServerIsDirty(ui_netSource->current.integer, i))
                {
                    dirty = 1;
                    ping = LAN_GetServerPing(ui_netSource->current.integer, i);
                    if (ping > 0)
                    {
                        LAN_GetServerInfo(ui_netSource->current.integer, i, info, 1024);
                        v6 = Info_ValueForKey(info, "clients");
                        clients = atoi(v6);
                        sharedUiInfo.serverStatus.numPlayersOnServers += clients;
                        v7 = Info_ValueForKey(info, "addr");
                        if (!I_strnicmp(v7, "000.000.000.000", 15))
                            goto LABEL_64;
                        if (ping <= 0)
                            goto LABEL_65;
                        if (!ui_browserShowEmpty->current.enabled && !clients)
                            goto LABEL_64;
                        if (!ui_browserShowFull->current.enabled)
                        {
                            v8 = Info_ValueForKey(info, "com_maxclients");
                            maxClients = atoi(v8);
                            if (clients == maxClients)
                                goto LABEL_64;
                        }
                        if (ui_browserShowPassword->current.integer >= 0)
                        {
                            v9 = Info_ValueForKey(info, "pswrd");
                            if (atoi(v9) != ui_browserShowPassword->current.integer)
                                goto LABEL_64;
                        }
                        if (ui_browserShowPure->current.enabled)
                        {
                            v10 = Info_ValueForKey(info, "pure");
                            if (!atoi(v10))
                                goto LABEL_64;
                        }
                        if (ui_browserMod->current.integer > -2)
                        {
                            mod = Info_ValueForKey(info, "game");
                            if (ui_browserMod->current.integer == -1)
                            {
                                if (*mod)
                                    goto LABEL_64;
                            }
                            if (ui_browserMod->current.integer >= 0)
                            {
                                if (ui_browserMod->current.integer >= 0x40u
                                    && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_server.cpp",
                                        317,
                                        0,
                                        "ui_browserMod->current.integer doesn't index MAX_MODS\n\t%i not in [0, %i)",
                                        ui_browserMod->current.integer,
                                        64))
                                {
                                    __debugbreak();
                                }
                                if (I_stricmp(mod + 5, sharedUiInfo.modList[ui_browserMod->current.integer].modName))
                                    goto LABEL_64;
                            }
                        }
                        if (ui_browserFriendlyfire->current.integer >= 0)
                        {
                            v11 = Info_ValueForKey(info, "ff");
                            if (atoi(v11) != ui_browserFriendlyfire->current.integer)
                                goto LABEL_64;
                        }
                        if (ui_browserKillcam->current.integer >= 0)
                        {
                            v12 = Info_ValueForKey(info, "kc");
                            if (atoi(v12) != ui_browserKillcam->current.integer)
                                goto LABEL_64;
                        }
                        if (ui_browserHardcore->current.integer >= 0)
                        {
                            v13 = Info_ValueForKey(info, "hc");
                            if (atoi(v13) != ui_browserHardcore->current.integer)
                                goto LABEL_64;
                        }
                        if ((*(char*)ui_browserGameMode->current.integer == 48
                            || (v14 = Info_ValueForKey(info, "gametype"), !I_stricmp(ui_browserGameMode->current.string, v14)))
                            && (ui_browserMap->current.integer <= 0
                                || ui_browserMap->current.integer >= sharedUiInfo.mapCount + 1
                                || (v15 = Info_ValueForKey(info, "mapname"),
                                    !I_stricmp(
                                        &sharedUiInfo.joinGameTypes[29].gameTypeNameCaps[304 * ui_browserMap->current.integer + 4],
                                        v15)))
                            && (ui_browserVoiceChat->current.integer < 0
                                || (v16 = Info_ValueForKey(info, "voice"), atoi(v16) == ui_browserVoiceChat->current.integer))
                            && (!sharedUiInfo.joinGameTypes[ui_joinGameType->current.integer].gameTypeName[0]
                                || (v19 = &sharedUiInfo.joinGameTypes[ui_joinGameType->current.integer],
                                    v17 = Info_ValueForKey(info, "gametype"),
                                    !I_stricmp(v17, v19->gameType)))
                            && (ui_serverFilterType <= 0
                                || (v20 = UI_GetServerFilter((const char**)&filter, ui_serverFilterType)[1],
                                    v18 = Info_ValueForKey(info, "game"),
                                    !I_stricmp(v18, v20))))
                        {
                        LABEL_65:
                            if (ui_netSource->current.integer == 3)
                                UI_RemoveServerFromDisplayList(i);
                            UI_BinaryServerInsertion(i, 0);
                            if (ping > 0)
                            {
                                LAN_MarkServerDirty(ui_netSource->current.integer, i, 0);
                                ++numclean;
                            }
                        }
                        else
                        {
                        LABEL_64:
                            LAN_MarkServerDirty(ui_netSource->current.integer, i, 0);
                        }
                    }
                }
            }
            if (ui_netSource->current.integer == 4 && sharedUiInfo.serverStatus.sortKey == 11)
                UI_ServersSort(0x10000000BLL);
            sharedUiInfo.serverStatus.refreshtime = uiInfo->uiDC.realTime;
        }
    }
}

int __cdecl UI_BinaryServerInsertion(int num, int nameNum)
{
    int offset; // [esp+0h] [ebp-10h]
    int len; // [esp+4h] [ebp-Ch]
    int res; // [esp+8h] [ebp-8h]
    int mid; // [esp+Ch] [ebp-4h]

    len = sharedUiInfo.serverStatus.numDisplayServers;
    mid = sharedUiInfo.serverStatus.numDisplayServers;
    offset = 0;
    res = 0;
    while (mid > 0)
    {
        mid = len >> 1;
        if (sharedUiInfo.serverStatus.sortKey == 11)
            res = LAN_CompareServers(
                ui_netSource->current.integer,
                11,
                sharedUiInfo.serverStatus.sortDir,
                nameNum,
                sharedUiInfo.serverStatus.displayServersFriendNameInd[mid + offset]);
        else
            res = LAN_CompareServers(
                ui_netSource->current.integer,
                sharedUiInfo.serverStatus.sortKey,
                sharedUiInfo.serverStatus.sortDir,
                num,
                sharedUiInfo.serverStatus.displayServers[mid + offset]);
        if (sharedUiInfo.serverStatus.sortKey == 11)
        {
            if (res != -LAN_CompareServers(
                ui_netSource->current.integer,
                11,
                sharedUiInfo.serverStatus.sortDir,
                sharedUiInfo.serverStatus.displayServersFriendNameInd[mid + offset],
                nameNum)
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_server.cpp",
                    137,
                    0,
                    "%s",
                    "res == -LAN_CompareServers( ui_netSource->current.integer, sharedUiInfo.serverStatus.sortKey, sharedUiInfo"
                    ".serverStatus.sortDir, sharedUiInfo.serverStatus.displayServersFriendNameInd[offset + mid], nameNum )"))
            {
                __debugbreak();
            }
        }
        else if (res != -LAN_CompareServers(
            ui_netSource->current.integer,
            sharedUiInfo.serverStatus.sortKey,
            sharedUiInfo.serverStatus.sortDir,
            sharedUiInfo.serverStatus.displayServers[mid + offset],
            num)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_server.cpp",
                141,
                0,
                "%s",
                "res == -LAN_CompareServers( ui_netSource->current.integer, sharedUiInfo.serverStatus.sortKey, sharedUiI"
                "nfo.serverStatus.sortDir, sharedUiInfo.serverStatus.displayServers[offset + mid], num )"))
        {
            __debugbreak();
        }
        if (!res)
        {
            UI_InsertServerIntoDisplayList(num, mid + offset, nameNum);
            return mid + offset;
        }
        if (res > 0)
            offset += mid;
        len -= mid;
    }
    if (res > 0)
        ++offset;
    UI_InsertServerIntoDisplayList(num, offset, nameNum);
    return offset;
}

void __cdecl UI_InsertServerIntoDisplayList(int num, int position, int nameNum)
{
    int i; // [esp+0h] [ebp-8h]
    int res; // [esp+4h] [ebp-4h]
    int resa; // [esp+4h] [ebp-4h]
    int resb; // [esp+4h] [ebp-4h]
    int resc; // [esp+4h] [ebp-4h]

    if (position < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_server.cpp", 55, 0, "%s", "position >= 0"))
    {
        __debugbreak();
    }
    if (sharedUiInfo.serverStatus.sortKey == 11)
    {
        if (position < sharedUiInfo.serverStatus.numDisplayServers)
        {
            res = LAN_CompareServers(
                ui_netSource->current.integer,
                sharedUiInfo.serverStatus.sortKey,
                sharedUiInfo.serverStatus.sortDir,
                nameNum,
                sharedUiInfo.serverStatus.displayServersFriendNameInd[position]);
            if (res != -LAN_CompareServers(
                ui_netSource->current.integer,
                sharedUiInfo.serverStatus.sortKey,
                sharedUiInfo.serverStatus.sortDir,
                sharedUiInfo.serverStatus.displayServersFriendNameInd[position],
                nameNum)
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_server.cpp",
                    63,
                    0,
                    "%s",
                    "res == -LAN_CompareServers( ui_netSource->current.integer, sharedUiInfo.serverStatus.sortKey, sharedUiInfo"
                    ".serverStatus.sortDir, sharedUiInfo.serverStatus.displayServersFriendNameInd[position], nameNum )"))
            {
                __debugbreak();
            }
            if (res > 0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_server.cpp", 64, 0, "%s", "res <= 0"))
                __debugbreak();
        }
        if (position > 0)
        {
            resa = LAN_CompareServers(
                ui_netSource->current.integer,
                sharedUiInfo.serverStatus.sortKey,
                sharedUiInfo.serverStatus.sortDir,
                nameNum,
                sharedUiInfo.serverStatus.motd[position + 1023]);
            if (resa != -LAN_CompareServers(
                ui_netSource->current.integer,
                sharedUiInfo.serverStatus.sortKey,
                sharedUiInfo.serverStatus.sortDir,
                sharedUiInfo.serverStatus.motd[position + 1023],
                nameNum)
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_server.cpp",
                    69,
                    0,
                    "%s",
                    "res == -LAN_CompareServers( ui_netSource->current.integer, sharedUiInfo.serverStatus.sortKey, sharedUiInfo"
                    ".serverStatus.sortDir, sharedUiInfo.serverStatus.displayServersFriendNameInd[position - 1], nameNum )"))
            {
                __debugbreak();
            }
            if (resa < 0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_server.cpp", 70, 0, "%s", "res >= 0"))
            {
                __debugbreak();
            }
        }
    }
    else
    {
        if (position < sharedUiInfo.serverStatus.numDisplayServers)
        {
            resb = LAN_CompareServers(
                ui_netSource->current.integer,
                sharedUiInfo.serverStatus.sortKey,
                sharedUiInfo.serverStatus.sortDir,
                num,
                sharedUiInfo.serverStatus.displayServers[position]);
            if (resb != -LAN_CompareServers(
                ui_netSource->current.integer,
                sharedUiInfo.serverStatus.sortKey,
                sharedUiInfo.serverStatus.sortDir,
                sharedUiInfo.serverStatus.displayServers[position],
                num)
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_server.cpp",
                    79,
                    0,
                    "%s",
                    "res == -LAN_CompareServers( ui_netSource->current.integer, sharedUiInfo.serverStatus.sortKey, sharedUiInfo"
                    ".serverStatus.sortDir, sharedUiInfo.serverStatus.displayServers[position], num )"))
            {
                __debugbreak();
            }
            if (resb > 0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_server.cpp", 80, 0, "%s", "res <= 0"))
            {
                __debugbreak();
            }
        }
        if (position > 0)
        {
            resc = LAN_CompareServers(
                ui_netSource->current.integer,
                sharedUiInfo.serverStatus.sortKey,
                sharedUiInfo.serverStatus.sortDir,
                num,
                *(&sharedUiInfo.serverStatus.currentServer + position));
            if (resc != -LAN_CompareServers(
                ui_netSource->current.integer,
                sharedUiInfo.serverStatus.sortKey,
                sharedUiInfo.serverStatus.sortDir,
                *(&sharedUiInfo.serverStatus.currentServer + position),
                num)
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_server.cpp",
                    85,
                    0,
                    "%s",
                    "res == -LAN_CompareServers( ui_netSource->current.integer, sharedUiInfo.serverStatus.sortKey, sharedUiInfo"
                    ".serverStatus.sortDir, sharedUiInfo.serverStatus.displayServers[position - 1], num )"))
            {
                __debugbreak();
            }
            if (resc < 0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_server.cpp", 86, 0, "%s", "res >= 0"))
            {
                __debugbreak();
            }
        }
    }
    if (position >= 0 && position <= sharedUiInfo.serverStatus.numDisplayServers)
    {
        if (position <= sharedUiInfo.serverStatus.currentServer && sharedUiInfo.serverStatus.numDisplayServers)
            ++sharedUiInfo.serverStatus.currentServer;
        for (i = ++sharedUiInfo.serverStatus.numDisplayServers; i > position; --i)
            sharedUiInfo.serverStatus.displayServers[i] = *(&sharedUiInfo.serverStatus.currentServer + i);
        sharedUiInfo.serverStatus.displayServers[position] = num;
    }
}

void __cdecl UI_BuildServerStatus(int localClientNum, uiInfo_s *uiInfo, int force)
{
    if (!uiInfo->nextFindPlayerRefresh)
    {
        if (force)
        {
            Menu_SetFeederSelection(localClientNum, &uiInfo->uiDC, 0, 13, 0, 0);
            sharedUiInfo.serverStatusInfo.numLines = 0;// serverStatusInfo.numLines = 0 (0x244D8)
            LAN_GetServerStatus(0, 0, 0);
        }
        else if (!sharedUiInfo.nextServerStatusRefresh || sharedUiInfo.nextServerStatusRefresh > uiInfo->uiDC.realTime)
        {
            return;
        }
        UI_UpdateDisplayServers(localClientNum, uiInfo);
        if (sharedUiInfo.serverStatus.currentServer >= 0
            && sharedUiInfo.serverStatus.currentServer <= sharedUiInfo.serverStatus.numDisplayServers
            && sharedUiInfo.serverStatus.numDisplayServers)
        {
            if (UI_GetServerStatusInfo(
                //sharedUiInfo.serverStatusSecurityID,// .serverStatusAddress (0x23030) LWSS: i have no clue why `StatusSecurityID` is here, maybe a bug? Weird
                sharedUiInfo.serverStatusAddress,
                &sharedUiInfo.serverStatusInfo))  // .serverStatusInfo (0x23038)
            {
                sharedUiInfo.nextServerStatusRefresh = 0;
                UI_GetServerStatusInfo(sharedUiInfo.serverStatusAddress, 0);
            }
            else
            {
                sharedUiInfo.nextServerStatusRefresh = uiInfo->uiDC.realTime + 500;
            }
        }
    }
}

void __cdecl UI_BuildServerStatusScoreBoard(int localClientNum, uiInfo_s *uiInfo, int force)
{
#if 0 // KISAKTODO
    if (force)
    {
        Menu_SetFeederSelection(localClientNum, &uiInfo->uiDC, 0, 91, 0, 0);
        dword_98DA4D4 = 0;
        LAN_GetServerStatusScoreBoard(0, 0, 0);
    }
    else if (!dword_98DA4DC || dword_98DA4DC > uiInfo->uiDC.realTime)
    {
        return;
    }
    UI_UpdateDisplayServers(localClientNum, uiInfo);
    if (sharedUiInfo.serverStatus.currentServer >= 0
        && sharedUiInfo.serverStatus.currentServer <= sharedUiInfo.serverStatus.numDisplayServers
        && sharedUiInfo.serverStatus.numDisplayServers)
    {
        if (UI_GetServerStatusInfoScoreBoard(
            &sharedUiInfo.serverStatusInfoScoreBoard.lines[4][7],
            &sharedUiInfo.pendingServerStatus.server[1].name[12]))
        {
            tries = 5;
            dword_98DA4DC = 0;
            UI_GetServerStatusInfoScoreBoard(&sharedUiInfo.serverStatusInfoScoreBoard.lines[2][7], 0);
        }
        else if (tries <= 0)
        {
            tries = 5;
            dword_98DA4DC = 0;
        }
        else
        {
            dword_98DA4DC = uiInfo->uiDC.realTime + 500;
            --tries;
        }
    }
#endif
}

void __cdecl UI_DoServerRefresh(int localClientNum, uiInfo_s *uiInfo)
{
    BOOL wait; // [esp+0h] [ebp-4h]

    if (sharedUiInfo.serverStatus.refreshActive)
    {
        wait = LAN_WaitServerResponse() != 0;
        if (uiInfo->uiDC.realTime >= sharedUiInfo.serverStatus.refreshtime || !wait)
        {
            UI_UpdateDisplayServers(localClientNum, uiInfo);
            if (LAN_UpdateDirtyPings(0, ui_netSource->current.unsignedInt))
            {
                sharedUiInfo.serverStatus.refreshtime = uiInfo->uiDC.realTime + 2000;
                UI_BuildServerDisplayList(0, uiInfo, 2);
                UI_UpdateDisplayServers(0, uiInfo);
            }
            else if (!wait)
            {
                UI_BuildServerDisplayList(localClientNum, uiInfo, 2);
                UI_StopServerRefresh();
            }
            UI_BuildServerDisplayList(localClientNum, uiInfo, 0);
        }
    }
}

void __cdecl UI_StartServerRefresh(int localClientNum, int contextIndex, int full)
{
    const char *MonthAbbrev; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    int tm_mday; // [esp-10h] [ebp-58h]
    int v9; // [esp-Ch] [ebp-54h]
    int tm_hour; // [esp-8h] [ebp-50h]
    int tm_min; // [esp-4h] [ebp-4Ch]
    int controllerIndex; // [esp+0h] [ebp-48h]
    uiInfo_s *uiInfo; // [esp+4h] [ebp-44h]
    qtime_s q; // [esp+8h] [ebp-40h] BYREF
    char dvarName[24]; // [esp+2Ch] [ebp-1Ch] BYREF

    uiInfo = UI_UIContext_GetInfo(contextIndex);
    if (localClientNum == -1
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_server.cpp",
            619,
            0,
            "%s",
            "localClientNum != INVALID_CLIENT_NUMBER"))
    {
        __debugbreak();
    }
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if (controllerIndex == -1
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_server.cpp",
            621,
            0,
            "%s",
            "controllerIndex != INVALID_CONTROLLER_PORT"))
    {
        __debugbreak();
    }
    Com_RealTime(&q, 1);
    _snprintf(dvarName, 0x18u, "ui_lastServerRefresh_%i", ui_netSource->current.integer);
    tm_min = q.tm_min;
    tm_hour = q.tm_hour;
    v9 = q.tm_year + 1900;
    tm_mday = q.tm_mday;
    MonthAbbrev = UI_GetMonthAbbrev(q.tm_mon);
    v5 = UI_SafeTranslateString(MonthAbbrev);
    v6 = va("%s %i, %i   %i:%02i", v5, tm_mday, v9, tm_hour, tm_min);
    Dvar_SetStringByName(dvarName, v6);
    if (full)
    {
        *(_QWORD *)&sharedUiInfo.serverStatusInfo.lines[30][7] = 1;
        sharedUiInfo.serverStatus.nextDisplayRefresh = uiInfo->uiDC.realTime + 1000;
        UI_ClearDisplayedServers();
        LAN_MarkServerDirty(ui_netSource->current.integer, -1, 1u);
        LAN_ResetPings(ui_netSource->current.integer);
        sharedUiInfo.serverStatus.refreshtime = uiInfo->uiDC.realTime + 5000;
        cls.pingUpdateSource = ui_netSource->current.integer;
        v7 = va("findservers %i\n", ui_netSource->current.integer);
        Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, v7);
    }
    else
    {
        UI_UpdatePendingPings(uiInfo);
    }
}

void __cdecl UI_StopServerRefresh()
{
    int count; // [esp+0h] [ebp-4h]

    if (sharedUiInfo.serverStatus.refreshActive)
    {
        sharedUiInfo.serverStatus.refreshActive = 0;
        Com_Printf(
            13,
            "%d servers listed in browser with %d players.\n",
            sharedUiInfo.serverStatus.numDisplayServers,
            sharedUiInfo.serverStatus.numPlayersOnServers);
        count = LAN_GetServerCount(ui_netSource->current.integer);
        if (count - sharedUiInfo.serverStatus.numDisplayServers > 0)
            Com_Printf(
                13,
                "%d servers not listed (filtered out by game browser settings)\n",
                count - sharedUiInfo.serverStatus.numDisplayServers);
    }
}

void __cdecl UI_ServersSort(__int64 column)
{
    if (column != sharedUiInfo.serverStatus.sortKey)
    {
        sharedUiInfo.serverStatus.sortKey = column;
        qsort(
            &sharedUiInfo.serverStatusInfo.lines[31][1],
            sharedUiInfo.serverStatus.numDisplayServers,
            4u,
            (int(__cdecl *)(const void *, const void *))UI_ServersQsortCompare);
    }
}

int __cdecl UI_ServersQsortCompare(int *arg1, int *arg2)
{
    return LAN_CompareServers(
        ui_netSource->current.integer,
        sharedUiInfo.serverStatus.sortKey,
        sharedUiInfo.serverStatus.sortDir,
        *arg1,
        *arg2);
}

void __cdecl UI_AddServerToFavoritesList(const char *pszName, const char *pszAddress)
{
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    const char *v8; // eax
    int res; // [esp+20h] [ebp-4h]

    if ( strlen(pszName) )
    {
        if ( strlen(pszAddress) )
        {
            res = LAN_AddServer(3, pszName, pszAddress);
            if ( res )
            {
                if ( res == -1 )
                {
                    v5 = UI_SafeTranslateString("EXE_FAVORITELISTFULL");
                    Com_Printf(13, "%s\n", v5);
                    Dvar_SetStringByName("ui_favorite_message", (char*)"@EXE_FAVORITELISTFULL");
                }
                else if ( res == -2 )
                {
                    v6 = UI_SafeTranslateString("EXE_BADSERVERADDRESS");
                    Com_Printf(13, "%s\n", v6);
                    Dvar_SetStringByName("ui_favorite_message", (char *)"@EXE_BADSERVERADDRESS");
                }
                else
                {
                    v7 = UI_SafeTranslateString("EXE_FAVORITEADDED");
                    v8 = va("%s\n", v7);
                    Com_Printf(13, v8, pszAddress);
                    Dvar_SetStringByName("ui_favorite_message", (char *)"@EXE_FAVORITEADDED");
                }
            }
            else
            {
                v4 = UI_SafeTranslateString("EXE_FAVORITEINLIST");
                Com_Printf(13, "%s\n", v4);
                Dvar_SetStringByName("ui_favorite_message", (char *)"@EXE_FAVORITEINLIST");
            }
        }
        else
        {
            v3 = UI_SafeTranslateString("EXE_FAVORITEADDRESSEMPTY");
            Com_Printf(13, "%s\n", v3);
            Dvar_SetStringByName("ui_favorite_message", (char *)"@EXE_FAVORITEADDRESSEMPTY");
        }
    }
    else
    {
        v2 = UI_SafeTranslateString("EXE_FAVORITENAMEEMPTY");
        Com_Printf(13, "%s\n", v2);
        Dvar_SetStringByName("ui_favorite_message", (char *)"@EXE_FAVORITENAMEEMPTY");
    }
}

void __cdecl UI_AddServerToFavoritesList(unsigned __int64 uid)
{
    Live_AddFavourite(0, uid);
}

void __cdecl UI_RemoveServerFromFavoritesList()
{
    char *v0; // eax
    unsigned __int64 uid; // [esp+0h] [ebp-410h] BYREF
    char buff[1028]; // [esp+8h] [ebp-408h] BYREF

    if (sharedUiInfo.serverStatus.currentServer >= 0
        && sharedUiInfo.serverStatus.currentServer < sharedUiInfo.serverStatus.numDisplayServers)
    {
        LAN_GetServerInfo(
            ui_netSource->current.integer,
            sharedUiInfo.serverStatus.displayServers[sharedUiInfo.serverStatus.currentServer],
            buff,
            1024);
        v0 = Info_ValueForKey(buff, "dwuserid");
        StringToXUID(v0, &uid);
        Live_DeleteFavourite(uid);
    }
}

// local variable allocation has failed, the output may be wrong!
void __cdecl UI_DrawServerRefreshDate(
                int contextIndex,
                rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                int textStyle)
{
    double v7; // xmm0_8
    const char *v8; // eax
    char *v9; // eax
    const char *VariantString; // eax
    long double v11; // [esp+1Ch] [ebp-48Ch] OVERLAPPED BYREF
    int offset; // [esp+64h] [ebp-444h]
    char hackString[1028]; // [esp+68h] [ebp-440h] BYREF
    int replaceCounts[2]; // [esp+470h] [ebp-38h] BYREF
    const char *first; // [esp+478h] [ebp-30h]
    float lowLight[4]; // [esp+47Ch] [ebp-2Ch] BYREF
    float newColor[4]; // [esp+48Ch] [ebp-1Ch] BYREF
    uiInfo_s *uiInfo; // [esp+49Ch] [ebp-Ch]
    const char *string; // [esp+4A0h] [ebp-8h]
    int serverCount; // [esp+4A4h] [ebp-4h]

    if (sharedUiInfo.serverStatus.refreshActive)
    {
        lowLight[0] = 0.80000001 * *color;
        lowLight[1] = 0.80000001 * color[1];
        lowLight[2] = 0.80000001 * color[2];
        lowLight[3] = 0.80000001 * color[3];
        uiInfo = UI_UIContext_GetInfo(contextIndex);

        //*(float *)&v11 = (float)(uiInfo->uiDC.realTime / 75);
        //v7 = *(float *)&v11;
        //__libm_sse2_sin(v11);
        //*(float *)&v7 = v7;
        v7 = sin((float)(uiInfo->uiDC.realTime / 75));
        LerpColor(color, lowLight, newColor, (float)v7 * 0.5f + 0.5f);
        if (LAN_WaitServerResponse())
        {
            if (Dvar_GetBool("dw_loggedin"))
                string = UI_SafeTranslateString("EXE_WAITINGFORMASTERSERVERRESPONSE");
            else
                string = UI_SafeTranslateString("PLATFORM_NOT_ONLINE");
        }
        else
        {
            serverCount = LAN_GetServerCount(ui_netSource->current.integer);
            string = UI_SafeTranslateString("EXE_GETTINGINFOFORSERVERS");
            I_strncpyz(hackString, string, 1024);
            v8 = strstr(string, "&&1");
            first = v8;
            if (v8)
            {
                offset = first - string + 3;
                hackString[offset++] = '/';
                hackString[offset++] = '&';
                hackString[offset++] = '&';
                hackString[offset++] = '2';
                I_strncpyz(&hackString[offset], &string[offset - 4], 1024 - offset);
            }
            replaceCounts[0] = cls.pingedServerCount;
            replaceCounts[1] = serverCount;
            string = UI_ReplaceConversionInts(hackString, 2, replaceCounts);
        }
        UI_DrawText(
            &scrPlaceView[contextIndex],
            (char *)string,
            0x7FFFFFFF,
            font,
            rect->x,
            rect->y,
            rect->horzAlign,
            rect->vertAlign,
            scale,
            newColor,
            textStyle);
    }
    else
    {
        v9 = va("ui_lastServerRefresh_%i", ui_netSource->current.integer);
        VariantString = Dvar_GetVariantString(v9);
        I_strncpyz((char *)&v11 + 4, VariantString, 64);
        string = UI_SafeTranslateString("EXE_REFRESHTIME");
        string = UI_ReplaceConversionString((char *)string, (const char *)&v11 + 4);
        UI_DrawText(
            &scrPlaceView[contextIndex],
            (char *)string,
            0x7FFFFFFF,
            font,
            rect->x,
            rect->y,
            rect->horzAlign,
            rect->vertAlign,
            scale,
            color,
            textStyle);
    }
}

void __cdecl UI_UpdateServerToolTips(int localClientNum, listBoxDef_s *listPtr)
{
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    float deltaX; // [esp+20h] [ebp-50h]
    int iswager; // [esp+24h] [ebp-4Ch]
    float v13; // [esp+2Ch] [ebp-44h]
    uiInfo_s *uiInfo; // [esp+30h] [ebp-40h]
    itemDef_s *serversListItem; // [esp+34h] [ebp-3Ch]
    signed int time; // [esp+38h] [ebp-38h]
    bool showMap; // [esp+3Fh] [ebp-31h]
    char *toolTipText; // [esp+40h] [ebp-30h]
    int i; // [esp+44h] [ebp-2Ch]
    int ia; // [esp+44h] [ebp-2Ch]
    menuDef_t *serverBrowserMenu; // [esp+48h] [ebp-28h]
    rectDef_s *columnRect; // [esp+4Ch] [ebp-24h]
    rectDef_s rect; // [esp+54h] [ebp-1Ch] BYREF
    float dy; // [esp+6Ch] [ebp-4h]

    time = Sys_Milliseconds();
    serverBrowserMenu = 0;
    serversListItem = 0;
    uiInfo = UI_GetInfo(localClientNum);
    toolTipText = 0;
    showMap = 0;
    if (!uiInfo && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_server.cpp", 889, 0, "%s", "uiInfo"))
        __debugbreak();
    if (!listPtr && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_server.cpp", 890, 0, "%s", "listPtr"))
        __debugbreak();
    v13 = uiInfo->uiDC.cursor.x - uiInfo->uiDC.prevCursor.x;
    dy = uiInfo->uiDC.cursor.y - uiInfo->uiDC.prevCursor.y;
    if ((fabs(v13)) <= 0.001 && (fabs(dy)) <= 0.001)
    {
        if (ui_showToolTipTime)
        {
            if (time >= ui_showToolTipTime)
            {
                ui_toolAllowTimer = 0;
                ui_showToolTipTime = 0;
                if (listPtr->mousePos >= 0 && listPtr->mousePos < sharedUiInfo.serverStatus.numDisplayServers)
                {
                    for (i = uiInfo->uiDC.openMenuCount - 1; i >= 0 && !serversListItem; --i)
                    {
                        serverBrowserMenu = uiInfo->uiDC.menuStack[i].menu;
                        if (serverBrowserMenu)
                            serversListItem = Menu_FindItemByName(serverBrowserMenu, "browser");
                    }
                    if (serversListItem
                        && Rect_ContainsPoint(
                            uiInfo->uiDC.contextIndex,
                            &serversListItem->window.rect,
                            uiInfo->uiDC.cursor.x,
                            uiInfo->uiDC.cursor.y))
                    {
                        for (ia = 0; ia < ui_toolTipCount; ++ia)
                        {
                            columnRect = &listPtr->columnInfo[ui_toolTipColumns[ia]].rect;
                            rect.x = serversListItem->window.rect.x + columnRect->x;
                            rect.y = (float)((float)(listPtr->mousePos - listPtr->startPos[uiInfo->uiDC.contextIndex])
                                * listPtr->elementHeight)
                                + serversListItem->window.rect.y;
                            rect.w = columnRect->w;
                            rect.h = columnRect->h;
                            rect.horzAlign = serversListItem->window.rect.horzAlign;
                            rect.vertAlign = serversListItem->window.rect.vertAlign;
                            if (Rect_ContainsPoint(uiInfo->uiDC.contextIndex, &rect, uiInfo->uiDC.cursor.x, uiInfo->uiDC.cursor.y))
                            {
                                LAN_GetServerInfo(
                                    ui_netSource->current.integer,
                                    sharedUiInfo.serverStatus.displayServers[listPtr->mousePos],
                                    info_8,
                                    1024);
                                switch (ui_toolTipColumns[ia])
                                {
                                case 0:
                                    v2 = Info_ValueForKey(info_8, "wager");
                                    iswager = atoi(v2);
                                    v3 = Info_ValueForKey(info_8, "licensetype");
                                    switch (atoi(v3))
                                    {
                                    case 2:
                                        if (iswager)
                                            v5 = UI_SafeTranslateString("PLATFORM_WAGER_MATCH");
                                        else
                                            v5 = UI_SafeTranslateString("PLATFORM_RANKED_SERVER");
                                        toolTipText = v5;
                                        break;
                                    case 4:
                                        if (iswager)
                                            v4 = UI_SafeTranslateString("PLATFORM_OFFICIAL_WAGER_MATCH");
                                        else
                                            v4 = UI_SafeTranslateString("PLATFORM_OFFICIAL_RANKED_SERVER");
                                        toolTipText = v4;
                                        break;
                                    case 5:
                                        toolTipText = UI_SafeTranslateString("PLATFORM_LISTEN_SERVER");
                                        break;
                                    default:
                                        toolTipText = UI_SafeTranslateString("PLATFORM_UNRANKED_SERVER");
                                        break;
                                    }
                                    break;
                                case 1:
                                    v6 = Info_ValueForKey(info_8, "pb");
                                    if (atoi(v6))
                                        v7 = UI_SafeTranslateString("PLATFORM_VAC_SERVER");
                                    else
                                        v7 = UI_SafeTranslateString("PLATFORM_NON_VAC");
                                    toolTipText = v7;
                                    break;
                                case 2:
                                    v8 = Info_ValueForKey(info_8, "pswrd");
                                    if (atoi(v8))
                                        v9 = UI_SafeTranslateString("PLATFORM_PASSWORD_PROTECTED");
                                    else
                                        v9 = UI_SafeTranslateString("PLATFORM_PUBLIC");
                                    toolTipText = v9;
                                    break;
                                case 3:
                                case 5:
                                    if (ui_netSource->current.integer == 4)
                                    {
                                        toolTipText = Info_ValueForKey(info_8, "friendsOnServer");
                                        if (!toolTipText || !*toolTipText)
                                            toolTipText = (char *)"";
                                    }
                                    break;
                                case 4:
                                    showMap = 1;
                                    toolTipText = Info_ValueForKey(info_8, "mapname");
                                    break;
                                case 6:
                                    if (Info_ValueForKey(info_8, "gametype") && *Info_ValueForKey(info_8, "gametype"))
                                    {
                                        v10 = Info_ValueForKey(info_8, "gametype");
                                        toolTipText = UI_GetGameTypeDisplayName(v10);
                                    }
                                    break;
                                case 7:
                                    toolTipText = Info_ValueForKey(info_8, "geolocation");
                                    if (!toolTipText || !*toolTipText)
                                    {
                                        toolTipText = Info_ValueForKey(info_8, "countrycode");
                                        if (!toolTipText || !*toolTipText)
                                            toolTipText = UI_SafeTranslateString("PLATFORM_GEO_NOT_SPECIFIED");
                                    }
                                    break;
                                default:
                                    break;
                                }
                                if (showMap)
                                {
                                    UI_UpdateContextItem(
                                        &uiInfo->uiDC,
                                        serverBrowserMenu->window.name,
                                        "server_browser_maptip_back",
                                        10.0,
                                        10.0);
                                    UI_UpdateContextItem(
                                        &uiInfo->uiDC,
                                        serverBrowserMenu->window.name,
                                        "server_browser_maptip_map",
                                        10.0,
                                        10.0);
                                    Dvar_SetBoolIfChanged((dvar_s*)ui_browserShowMapTip, 1);
                                }
                                else
                                {
                                    deltaX = 2.6666667
                                        / (float)(scrPlaceView[uiInfo->uiDC.contextIndex].realViewportSize[0]
                                            / scrPlaceView[uiInfo->uiDC.contextIndex].realViewportSize[1]);
                                    UI_UpdateContextItem(
                                        &uiInfo->uiDC,
                                        serverBrowserMenu->window.name,
                                        "server_browser_tooltip_back",
                                        10.0,
                                        10.0);
                                    UI_UpdateContextItem(
                                        &uiInfo->uiDC,
                                        serverBrowserMenu->window.name,
                                        "server_browser_tooltip_content",
                                        deltaX + 10.0,
                                        10.0);
                                    if (toolTipText)
                                        Dvar_SetBoolIfChanged((dvar_s *)ui_browserShowToolTip, 1);
                                    else
                                        Dvar_SetBoolIfChanged((dvar_s *)ui_browserShowToolTip, 0);
                                }
                                if (toolTipText)
                                    Dvar_SetString((dvar_s *)ui_browserToolTip, toolTipText);
                                return;
                            }
                        }
                    }
                }
            }
        }
        else if (ui_toolAllowTimer)
        {
            ui_showToolTipTime = time + 500;
        }
    }
    else
    {
        Dvar_SetBoolIfChanged((dvar_s*)ui_browserShowToolTip, 0);
        Dvar_SetBoolIfChanged((dvar_s*)ui_browserShowMapTip, 0);
        ui_showToolTipTime = 0;
        ui_toolAllowTimer = 1;
    }
}

