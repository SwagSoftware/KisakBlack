#include "ui_main_pc.h"
#include <universal/q_parse.h>
#include <server_mp/sv_init_mp.h>
#include <client_mp/cl_ui_mp.h>
#include <client_mp/cl_cgame_mp.h>
#include <client_mp/cl_ui_pc_mp.h>
#include <client_mp/cl_main_pc_mp.h>
#include "ui_server.h"
#include "ui_atoms.h"
#include <universal/com_files.h>
#include "ui_utils.h"
#include <live/live_win.h>
#include <stringed/stringed_hooks.h>
#include <database/db_registry.h>

const dvar_t *ui_joinGameType;
const dvar_t *ui_netGameTypeName;
const dvar_t *ui_dedicated;
const dvar_t *ui_currentNetMap;
const dvar_t *ui_browserShowFull;
const dvar_t *ui_browserShowEmpty;
const dvar_t *ui_browserShowPassword;
const dvar_t *ui_browserShowPure;
const dvar_t *ui_browserMod;
const dvar_t *ui_browserShowDedicated;
const dvar_t *ui_browserFriendlyfire;
const dvar_t *ui_browserKillcam;
const dvar_t *ui_browserVoiceChat;
const dvar_t *ui_browserGameMode;
const dvar_t *ui_friendNameNew;
const dvar_t *ui_friendSelectedInd;
const dvar_t *ui_friendPendingSelectedInd;
const dvar_t *ui_inviteSelectedInd;
const dvar_t *ui_inviteScreen;
const dvar_t *ui_browserPlayerCount;
const dvar_t *ui_browserDedicatedServerCount;
const dvar_t *ui_browserShowToolTip;
const dvar_t *ui_browserShowMapTip;
const dvar_t *ui_browserToolTip;
const dvar_t *ui_browserShowInfo;
const dvar_t *ui_browserShowPunkBuster;
const dvar_t *ui_allowConsole;
const dvar_t *ui_allowFov;

parseInfo_t *__cdecl UI_GetMapRotationToken()
{
    parseInfo_t *token; // [esp+0h] [ebp-8h]
    const char *value; // [esp+4h] [ebp-4h] BYREF

    value = sv_mapRotationCurrent->current.string;
    token = Com_Parse(&value);
    if ( value )
    {
        Dvar_SetString((dvar_s *)sv_mapRotationCurrent, value);
        return token;
    }
    else
    {
        Dvar_SetString((dvar_s *)sv_mapRotationCurrent, "");
        return 0;
    }
}

void __cdecl UI_RegisterDvars_PC()
{
    ui_joinGameType = _Dvar_RegisterInt("ui_joinGametype", 0, 0, 0x7FFFFFFF, 1u, "Game join type");
    ui_netGameTypeName = _Dvar_RegisterString(
                                                 "ui_netGametypeName",
                                                 (char *)"",
                                                 0,
                                                 "Displayed game type name");
    ui_dedicated = _Dvar_RegisterInt("ui_dedicated", 0, 0, 2, 1u, "True if this is a dedicated server");
    ui_currentNetMap = _Dvar_RegisterInt("ui_currentNetMap", 0, 0, 0x7FFFFFFF, 1u, "Currently running map");
    ui_browserShowFull = _Dvar_RegisterBool("ui_browserShowFull", 1, 1u, "Show full servers");
    ui_browserShowEmpty = _Dvar_RegisterBool("ui_browserShowEmpty", 1, 1u, "Show empty servers");
    ui_browserShowPassword = _Dvar_RegisterInt(
                                                         "ui_browserShowPassword",
                                                         -1,
                                                         -1,
                                                         1,
                                                         1u,
                                                         "Show servers that are password protected");
    ui_browserShowPure = _Dvar_RegisterBool("ui_browserShowPure", 0, 1u, "Show pure servers only");
    ui_browserMod = _Dvar_RegisterInt("ui_browserMod", -2, -2, 64, 1u, "UI Mod value");
    ui_browserShowDedicated = _Dvar_RegisterBool("ui_browserShowDedicated", 0, 1u, "Show dedicated servers only");
    ui_browserFriendlyfire = _Dvar_RegisterInt(
                                                         "ui_browserFriendlyfire",
                                                         -1,
                                                         0x80000000,
                                                         0x7FFFFFFF,
                                                         1u,
                                                         "Friendly fire is active");
    ui_browserKillcam = _Dvar_RegisterInt("ui_browserKillcam", -1, 0x80000000, 0x7FFFFFFF, 1u, "Kill cam is active");
    ui_browserVoiceChat = _Dvar_RegisterInt("ui_browserVoiceChat", -1, 0x80000000, 0x7FFFFFFF, 1u, "Voice chat is active");
    ui_browserGameMode = _Dvar_RegisterString(
                                                 "ui_browserGameMode",
                                                 "0",
                                                 1u,
                                                 "Game mode preferred in server browser filter.");
    ui_friendNameNew = _Dvar_RegisterString("ui_friendNameNew", (char *)"", 0, "New friend name");
    ui_friendSelectedInd = _Dvar_RegisterInt(
                                                     "ui_friendSelectedInd",
                                                     0,
                                                     0x80000000,
                                                     0x7FFFFFFF,
                                                     0x40u,
                                                     "Currently selected friend index");
    ui_friendPendingSelectedInd = _Dvar_RegisterInt(
                                                                    "ui_friendPendingSelectedInd",
                                                                    0,
                                                                    0x80000000,
                                                                    0x7FFFFFFF,
                                                                    0x40u,
                                                                    "Currently selected pending friend index");
    ui_inviteSelectedInd = _Dvar_RegisterInt(
                                                     "ui_inviteSelectedInd",
                                                     0,
                                                     0x80000000,
                                                     0x7FFFFFFF,
                                                     0x40u,
                                                     "Currently selected invite index");
    ui_inviteScreen = _Dvar_RegisterBool(
                                            "ui_inviteScreen",
                                            0,
                                            0,
                                            "Distinguishs between invite friend and manage friends screens in the UI.");
    ui_browserPlayerCount = _Dvar_RegisterString(
                                                        "ui_browserPlayerCount",
                                                        (char *)"",
                                                        0,
                                                        "Number of players currently online");
    ui_browserDedicatedServerCount = _Dvar_RegisterString(
                                                                         "ui_browserDedicatedServerCount",
                                                                         (char *)"",
                                                                         0,
                                                                         "Number of live dedicated servers");
    ui_browserShowToolTip = _Dvar_RegisterBool(
                                                        "ui_browserShowToolTip",
                                                        0,
                                                        0,
                                                        "Whether or not to show the mouseover tooltips for the server browser");
    ui_browserShowMapTip = _Dvar_RegisterBool(
                                                     "ui_browserShowMapTip",
                                                     0,
                                                     0,
                                                     "Whether or not to show the mouseover map tip for the server browser");
    ui_browserToolTip = _Dvar_RegisterString(
                                                "ui_browserToolTip",
                                                (char *)"",
                                                0,
                                                "The content of the server browser toolip");
    ui_browserShowInfo = _Dvar_RegisterBool("ui_browserShowInfo", 0, 0, "Whether or not to show the server info");
    ui_browserShowPunkBuster = _Dvar_RegisterInt(
                                                             "ui_browserShowPunkBuster",
                                                             -1,
                                                             0x80000000,
                                                             0x7FFFFFFF,
                                                             1u,
                                                             "Only show PunkBuster servers?");
    ui_allowConsole = _Dvar_RegisterBool("ui_allowConsole", 1, 0x10u, "Show the 'Enable Console' menu item?");
    ui_allowFov = _Dvar_RegisterBool("ui_allowFov", 0, 1u, "Show the 'FOV' graphics setting?");
}

const char *netSources[4] = { "EXE_LAN", "EXE_INTERNET", "EXE_FAVORITES", "EXE_FRIENDS" };

const char *__cdecl UI_GetNetSources(unsigned int source)
{
    if ( source >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main_pc.cpp",
                    206,
                    0,
                    "source doesn't index ARRAY_COUNT(netSources)\n\t%i not in [0, %i)",
                    source,
                    4) )
    {
        __debugbreak();
    }
    return netSources[source];
}

void __cdecl UI_SelectCurrentGameType()
{
    const char *currType; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    currType = Dvar_GetString("g_gametype");
    for (i = 0; i < sharedUiInfo.numGameTypes; ++i)
    {
        if (!I_stricmp(currType, sharedUiInfo.gameTypes[i].gameType))
        {
            Dvar_SetInt((dvar_s*)ui_netGameType, i);
            Dvar_SetString((dvar_s *)ui_netGameTypeName, sharedUiInfo.gameTypes[i].gameType);
            return;
        }
    }
}

int __cdecl UI_GetListIndexFromMapIndex(int testMapIndex)
{
    int listIndex; // [esp+0h] [ebp-8h]
    int mapIndex; // [esp+4h] [ebp-4h]

    if ((testMapIndex < 0 || testMapIndex >= sharedUiInfo.mapCount)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main_pc.cpp",
            266,
            0,
            "%s\n\t(testMapIndex) = %i",
            "(testMapIndex >= 0 && testMapIndex < sharedUiInfo.mapCount)",
            testMapIndex))
    {
        __debugbreak();
    }
    listIndex = 0;
    for (mapIndex = 0; mapIndex < sharedUiInfo.mapCount; ++mapIndex)
    {
        if (sharedUiInfo.mapList[mapIndex].active)
        {
            if (mapIndex == testMapIndex)
                return listIndex;
            ++listIndex;
        }
    }
    return 0;
}

int __cdecl UI_MapCountByGameType()
{
    int c; // [esp+0h] [ebp-Ch]
    int game; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    game = ui_netGameType->current.integer;
    c = 0;
    for (i = 0; i < sharedUiInfo.mapCount; ++i)
    {
        sharedUiInfo.mapList[i].active = 0;
        if ((sharedUiInfo.mapList[i].typeBits & (1 << game)) != 0)
        {
            ++c;
            sharedUiInfo.mapList[i].active = 1;
        }
    }
    return c;
}

void __cdecl UI_SelectCurrentMap(int localClientNum)
{
    char *v1; // eax
    int iCount; // [esp+0h] [ebp-860h]
    char *info; // [esp+4h] [ebp-85Ch]
    char szMap[68]; // [esp+8h] [ebp-858h] BYREF
    int i; // [esp+4Ch] [ebp-814h]
    uiClientState_s cstate; // [esp+50h] [ebp-810h] BYREF

    CL_GetClientState(localClientNum, &cstate);
    if (cstate.connState == CA_ACTIVE)
    {
        info = CL_GetConfigString(0);
        if (*info)
        {
            v1 = Info_ValueForKey(info, "mapname");
            I_strncpyz(szMap, v1, 64);
            iCount = 0;
            for (i = 0; i < sharedUiInfo.mapCount; ++i)
            {
                if (sharedUiInfo.mapList[i].active)
                {
                    if (!I_stricmp(szMap, sharedUiInfo.mapList[i].mapName))
                    {
                        Menu_SetFeederSelection(0, &uiInfoArray->uiDC, 0, 4, iCount, "pc_gamesetup_coop");
                        return;
                    }
                    ++iCount;
                }
            }
        }
    }
}

void __cdecl UI_UseAltColorPalette(char *buffer)
{
    int i; // [esp+14h] [ebp-14h]
    const char *glyph; // [esp+18h] [ebp-10h] BYREF
    int len; // [esp+1Ch] [ebp-Ch]
    unsigned int letter; // [esp+20h] [ebp-8h]
    unsigned __int8 colorCode; // [esp+27h] [ebp-1h]

    len = strlen(buffer) - 2;
    for ( i = 0; i < len; ++i )
    {
        glyph = &buffer[i];
        letter = SEH_ReadCharFromString(&glyph, 0);
        if ( buffer[i] != 94 || &buffer[i] == (char *)-1 || buffer[i + 1] == 94 || buffer[i + 1] < 48 || buffer[i + 1] > 64 )
        {
            if ( buffer[i] == 94 && &buffer[i] != (char *)-1 && buffer[i + 1] == 70 )
            {
                buffer[i + 1] = 55;
            }
            else if ( ValidGamePadButtonIcon(letter) )
            {
                buffer[i] = 32;
            }
        }
        else
        {
            switch ( buffer[i + 1] )
            {
                case '1':
                    colorCode = 58;
                    goto LABEL_17;
                case '2':
                    colorCode = 59;
                    goto LABEL_17;
                case '3':
                    colorCode = 60;
                    goto LABEL_17;
                case '4':
                    colorCode = 61;
                    goto LABEL_17;
                case '5':
                    colorCode = 62;
                    goto LABEL_17;
                case '6':
                    colorCode = 63;
LABEL_17:
                    buffer[++i] = colorCode;
                    break;
                default:
                    ++i;
                    break;
            }
        }
    }
}

bool __cdecl ValidGamePadButtonIcon(unsigned int letter)
{
    if ( letter && letter <= 6 )
        return 1;
    if ( letter >= 0xE && letter <= 0x19 )
        return 1;
    if ( letter >= 0x1C && letter <= 0x1F )
        return 1;
    if ( letter == 188 )
        return 1;
    return letter == 189;
}

void __cdecl UI_RemoveServerFromDisplayList(int num)
{
    int j; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    for (i = 0; i < sharedUiInfo.serverStatus.numDisplayServers; ++i)
    {
        if (sharedUiInfo.serverStatus.displayServers[i] == num)
        {
            --sharedUiInfo.serverStatus.numDisplayServers;
            for (j = i; j < sharedUiInfo.serverStatus.numDisplayServers; ++j)
            {
                sharedUiInfo.serverStatus.displayServers[j] = sharedUiInfo.serverStatus.displayServers[j + 1];
                if (ui_netSource->current.integer == 4 && sharedUiInfo.serverStatus.numDisplayServers < 256)
                    sharedUiInfo.serverStatus.displayServersFriendNameInd[j] = sharedUiInfo.serverStatus.displayServersFriendNameInd[j + 1];
            }
            return;
        }
    }
}

serverStatusDvar_t serverStatusDvars[5] =
{
  { "sv_maxclients", "@EXE_SV_INFO_MAXCLIENTS", SSC_STRING },
  { "scr_team_fftype", "@EXE_SV_INFO_FRIENDLY_FIRE", SSC_YESNO },
  { "sv_voice", "@EXE_SV_INFO_VOICE", SSC_YESNO },
  { "geolocation", NULL, (sscType_t)4 },
  { NULL, NULL, SSC_STRING }
};


char __cdecl UI_IsServerStatusDvar(const char *dvarname)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; serverStatusDvars[i].name; ++i )
    {
        if ( !I_stricmp(serverStatusDvars[i].name, dvarname) )
            return 1;
    }
    return 0;
}

int __cdecl UI_GetServerStatusInfo(char *serverAddress, serverStatusInfo_s *info)
{
    char *v3; // eax
    char *v4; // eax
    char *hostname; // [esp+0h] [ebp-10h]
    char *servermessage; // [esp+4h] [ebp-Ch]
    char *mapname; // [esp+8h] [ebp-8h]
    char *p; // [esp+Ch] [ebp-4h]
    char *pa; // [esp+Ch] [ebp-4h]

    hostname = 0;
    servermessage = 0;
    mapname = 0;
    if ( info )
    {
        memset(info, 0, sizeof(serverStatusInfo_s));
        if ( LAN_GetServerStatus(serverAddress, info->text, 1024) )
        {
            Dvar_SetBool((dvar_s *)ui_browserShowInfo, 1);
            UI_UseAltColorPalette(info->text);
            I_strncpyz(info->address, serverAddress, 64);
            p = (char*)info->text;
            info->numLines = 0;
            do
            {
                if ( !p )
                    break;
                if ( !*p )
                    break;
                v3 = strchr(p, 0x5Cu);
                if ( !v3 )
                    break;
                *v3 = 0;
                pa = v3 + 1;
                if ( v3[1] == 92 )
                    break;
                info->lines[info->numLines][0] = pa;
                v4 = strchr(pa, 0x5Cu);
                if ( !v4 )
                    break;
                *v4 = 0;
                p = v4 + 1;
                info->lines[info->numLines][1] = v4 + 1;
                if ( !I_strcmp(info->lines[info->numLines][0], "ui_serverinfomessage") )
                    servermessage = (char *)info->lines[info->numLines][1];
                if ( !I_strcmp(info->lines[info->numLines][0], "sv_hostname") )
                    hostname = (char *)info->lines[info->numLines][1];
                if ( !I_strcmp(info->lines[info->numLines][0], "mapname") )
                    mapname = (char *)info->lines[info->numLines][1];
                if ( UI_IsServerStatusDvar(info->lines[info->numLines][0]) )
                    ++info->numLines;
            }
            while ( info->numLines < 128 );
            if ( servermessage && *servermessage )
                Dvar_SetStringByName("ui_serverinfomessage", servermessage);
            if ( hostname && *hostname )
                Dvar_SetStringByName("ui_serverinfomessagehostname", hostname);
            if ( mapname )
            {
                if ( *mapname )
                    Dvar_SetStringByName("ui_mapname", mapname);
            }
            UI_SortServerStatusInfo(info);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        LAN_GetServerStatus(serverAddress, 0, 0);
        return 0;
    }
}

void __cdecl UI_SortServerStatusInfo(serverStatusInfo_s *info)
{
    char *v2; // eax
    char *longname; // [esp+18h] [ebp-18h]
    int j; // [esp+1Ch] [ebp-14h]
    const char *tmp1; // [esp+20h] [ebp-10h]
    const char *tmp2; // [esp+24h] [ebp-Ch]
    int index; // [esp+28h] [ebp-8h]
    int i; // [esp+2Ch] [ebp-4h]

    index = 0;
    for (i = 0; serverStatusDvars[i].name; ++i)
    {
        for (j = 0; j < info->numLines; ++j)
        {
            if (info->lines[j][1] && *info->lines[j][1] && !I_stricmp(serverStatusDvars[i].name, info->lines[j][0]))
            {
                tmp1 = info->lines[index][0];
                tmp2 = info->lines[index][1];
                info->lines[index][0] = info->lines[j][0];
                info->lines[index][1] = info->lines[j][1];
                info->lines[j][0] = tmp1;
                info->lines[j][1] = tmp2;
                if (serverStatusDvars[i].altName && strlen(serverStatusDvars[i].altName))
                    info->lines[index][0] = serverStatusDvars[i].altName;
                switch (serverStatusDvars[i].type)
                {
                case 0:
                    break;
                case 1:
                    if (atoi(info->lines[index][1]))
                        info->lines[index][1] = "@EXE_YES";
                    else
                        info->lines[index][1] = "@EXE_NO";
                    break;
                case 2:
                    info->lines[index][1] = UI_GetGameTypeDisplayName(info->lines[index][1]);
                    break;
                case 3:
                    info->lines[index][1] = UI_GetMapDisplayName(info->lines[index][1]);
                    break;
                case 4:
                    if (info->lines[index][1] && *info->lines[index][1])
                    {
                        longname = CL_LongNameForShortName(info->lines[index][1]);
                        if (longname)
                            info->lines[index][1] = longname;
                    }
                    else
                    {
                        info->lines[index][1] = UI_SafeTranslateString("PLATFORM_GEO_NOT_SPECIFIED");
                    }
                    info->lines[index][0] = info->lines[index][1];
                    info->lines[index][1] = 0;
                    break;
                default:
                    v2 = va("unknown server status dvar type: %i", serverStatusDvars[i].type);
                    if (!Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main_pc.cpp", 549, 0, v2))
                        __debugbreak();
                    break;
                }
                ++index;
            }
        }
    }
}

int __cdecl UI_GetServerStatusInfoScoreBoard(char *serverAddress, serverStatusInfo_s *info)
{
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    char *kills; // [esp+0h] [ebp-28h]
    char *rank; // [esp+4h] [ebp-24h]
    char *assists; // [esp+8h] [ebp-20h]
    char *ping; // [esp+Ch] [ebp-1Ch]
    char *deaths; // [esp+18h] [ebp-10h]
    int i; // [esp+1Ch] [ebp-Ch]
    char *p; // [esp+20h] [ebp-8h]
    char *score; // [esp+24h] [ebp-4h]

    if ( info )
    {
        memset(info, 0, sizeof(serverStatusInfo_s));
        if ( LAN_GetServerStatusScoreBoard(serverAddress, info->text, 1024) )
        {
            I_strncpyz(info->address, serverAddress, 64);
            p = info->text;
            info->numLines = 0;
            if ( info->numLines < 125 )
            {
                info->lines[info->numLines][0] = "@EXE_SV_INFO_RANK";
                info->lines[info->numLines][1] = "@EXE_SV_INFO_NAME";
                info->lines[info->numLines][2] = "@EXE_SV_INFO_SCORE";
                info->lines[info->numLines][3] = "@EXE_SV_INFO_KILLS";
                info->lines[info->numLines][4] = "@EXE_SV_INFO_ASSISTS";
                info->lines[info->numLines][5] = "@EXE_SV_INFO_DEATHS";
                info->lines[info->numLines++][6] = "@EXE_SV_INFO_PING";
                for ( i = 0; p && *p; ++i )
                {
                    if ( *p == 92 )
                        *p++ = 0;
                    if ( !p )
                        break;
                    v3 = strchr(p, 0x20u);
                    if ( !v3 )
                        break;
                    *v3 = 0;
                    rank = v3 + 1;
                    v4 = strchr(v3 + 1, 0x20u);
                    if ( !v4 )
                        break;
                    *v4 = 0;
                    score = v4 + 1;
                    v5 = strchr(v4 + 1, 0x20u);
                    if ( !v5 )
                        break;
                    *v5 = 0;
                    kills = v5 + 1;
                    v6 = strchr(v5 + 1, 0x20u);
                    if ( !v6 )
                        break;
                    *v6 = 0;
                    assists = v6 + 1;
                    v7 = strchr(v6 + 1, 0x20u);
                    if ( !v7 )
                        break;
                    *v7 = 0;
                    deaths = v7 + 1;
                    v8 = strchr(v7 + 1, 0x20u);
                    if ( !v8 )
                        break;
                    *v8 = 0;
                    ping = v8 + 1;
                    v9 = strchr(v8 + 1, 0x20u);
                    if ( !v9 )
                        break;
                    *(_BYTE *)v9 = 0;
                    info->lines[info->numLines][0] = rank;
                    info->lines[info->numLines][1] = (const char *)(v9 + 1);
                    info->lines[info->numLines][2] = score;
                    info->lines[info->numLines][3] = kills;
                    info->lines[info->numLines][4] = assists;
                    info->lines[info->numLines][5] = deaths;
                    info->lines[info->numLines][6] = ping;
                    if ( ++info->numLines >= 128 )
                        break;
                    v10 = strchr((v9 + 1), 0x5Cu);
                    if ( !v10 )
                        break;
                    *v10 = 0;
                    p = v10 + 1;
                }
            }
            UI_SortServerStatusScoreBoardInfo(info);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        LAN_GetServerStatusScoreBoard(serverAddress, 0, 0);
        return 0;
    }
}

void __cdecl UI_SortServerStatusScoreBoardInfo(serverStatusInfo_s *info)
{
    qsort(info->lines[1], info->numLines - 1, 0x20u, (int (__cdecl *)(const void *, const void *))ScoreBar_CompareScores);
}

int __cdecl ScoreBar_CompareScores(const char **sv1, const char **sv2)
{
    int v2; // esi

    v2 = atoi(sv2[3]);
    return v2 - atoi(sv1[3]);
}

void __cdecl UI_RefreshServers()
{
    UI_UpdateDisplayServers(0, uiInfoArray);
}

void __cdecl UI_BuildFindPlayerList()
{
#if 0 // KISAKTODO
    char *v0; // eax
    int j; // [esp+4h] [ebp-18E4h]
    serverStatusInfo_s info; // [esp+8h] [ebp-18E0h] BYREF
    uiInfo_s *uiInfo; // [esp+14B4h] [ebp-434h]
    char dest[36]; // [esp+14B8h] [ebp-430h] BYREF
    int i; // [esp+14DCh] [ebp-40Ch]
    char buf[1028]; // [esp+14E0h] [ebp-408h] BYREF

    uiInfo = uiInfoArray;
    if ( unk_98B4B40 && uiInfo->nextFindPlayerRefresh <= uiInfo->uiDC.realTime )
    {
        UI_UpdateDisplayServers(0, uiInfo);
        for ( i = 0; i < 16; ++i )
        {
            if ( *(unsigned int *)&sharedUiInfo.pendingServerStatus.server[i + 2].adrstr[8]
                && UI_GetServerStatusInfo(&sharedUiInfo.pendingServerStatus.server[i + 1].adrstr[12], &info) )
            {
                ++numFound;
                for ( j = 0; j < info.numLines; ++j )
                {
                    if ( *(unsigned int *)&info.text[32 * j - 4064] )
                    {
                        if ( **(_BYTE **)&info.text[32 * j - 4064] )
                        {
                            I_strncpyz(dest, *(const char **)&info.text[32 * j - 4084], 34);
                            I_CleanStr(dest);
                            if ( stristr(dest, uiInfo->findPlayerName) )
                            {
                                if ( uiInfo->numFoundPlayerServers >= 15 )
                                {
                                    *(unsigned int *)&sharedUiInfo.pendingServerStatus.server[1].adrstr[8] = *(unsigned int *)&sharedUiInfo.gap0[81128];
                                }
                                else
                                {
                                    I_strncpyz(
                                        &uiInfo->findPlayerName[64 * uiInfo->numFoundPlayerServers + 960],
                                        &sharedUiInfo.pendingServerStatus.server[i + 1].adrstr[12],
                                        64);
                                    I_strncpyz(
                                        uiInfo->foundPlayerServerAddresses[uiInfo->numFoundPlayerServers + 15],
                                        &sharedUiInfo.pendingServerStatus.server[i + 1].name[12],
                                        64);
                                    ++uiInfo->numFoundPlayerServers;
                                }
                            }
                        }
                    }
                }
                Com_sprintf(
                    uiInfo->foundPlayerServerAddresses[uiInfo->numFoundPlayerServers + 15],
                    0x40u,
                    "searching %d/%d...",
                    *(unsigned int *)&sharedUiInfo.pendingServerStatus.server[1].adrstr[8],
                    numFound);
                *(unsigned int *)&sharedUiInfo.pendingServerStatus.server[i + 2].adrstr[8] = 0;
            }
            if ( !*(unsigned int *)&sharedUiInfo.pendingServerStatus.server[i + 2].adrstr[8]
                || *(unsigned int *)sharedUiInfo.pendingServerStatus.server[i + 2].adrstr < uiInfo->uiDC.realTime
                                                                                                                                                        - ui_serverStatusTimeOut->current.integer )
            {
                if ( *(unsigned int *)&sharedUiInfo.pendingServerStatus.server[i + 2].adrstr[8] )
                    ++numTimeOuts;
                UI_GetServerStatusInfo(&sharedUiInfo.pendingServerStatus.server[i + 1].adrstr[12], 0);
                *(unsigned int *)&sharedUiInfo.pendingServerStatus.server[i + 2].adrstr[8] = 0;
                UI_UpdateDisplayServers(0, uiInfo);
                if ( *(int *)&sharedUiInfo.pendingServerStatus.server[1].adrstr[8] < *(int *)&sharedUiInfo.gap0[81128] )
                {
                    *(unsigned int *)sharedUiInfo.pendingServerStatus.server[i + 2].adrstr = uiInfo->uiDC.realTime;
                    LAN_GetServerAddressString(
                        ui_netSource->current.integer,
                        *(unsigned int *)&sharedUiInfo.gap0[4 * *(unsigned int *)&sharedUiInfo.pendingServerStatus.server[1].adrstr[8] + 1128],
                        &sharedUiInfo.pendingServerStatus.server[i + 1].adrstr[12],
                        64);
                    LAN_GetServerInfo(
                        ui_netSource->current.integer,
                        *(unsigned int *)&sharedUiInfo.gap0[4 * *(unsigned int *)&sharedUiInfo.pendingServerStatus.server[1].adrstr[8] + 1128],
                        buf,
                        1024);
                    v0 = Info_ValueForKey(buf, "hostname");
                    I_strncpyz(&sharedUiInfo.pendingServerStatus.server[i + 1].name[12], v0, 64);
                    *(unsigned int *)&sharedUiInfo.pendingServerStatus.server[i + 2].adrstr[8] = 1;
                    Com_sprintf(
                        uiInfo->foundPlayerServerAddresses[uiInfo->numFoundPlayerServers + 15],
                        0x40u,
                        "searching %d/%d...",
                        ++*(unsigned int *)&sharedUiInfo.pendingServerStatus.server[1].adrstr[8],
                        numFound);
                }
            }
        }
        for ( i = 0; i < 16 && !*(unsigned int *)&sharedUiInfo.pendingServerStatus.server[i + 2].adrstr[8]; ++i )
            ;
        if ( i >= 16 )
        {
            if ( uiInfo->numFoundPlayerServers )
            {
                if ( uiInfo->numFoundPlayerServers == 2 )
                    Com_sprintf(
                        uiInfo->foundPlayerServerAddresses[uiInfo->numFoundPlayerServers + 15],
                        0x40u,
                        "%d server%s found with player %s",
                        uiInfo->numFoundPlayerServers - 1,
                        "",
                        uiInfo->findPlayerName);
                else
                    Com_sprintf(
                        uiInfo->foundPlayerServerAddresses[uiInfo->numFoundPlayerServers + 15],
                        0x40u,
                        "%d server%s found with player %s",
                        uiInfo->numFoundPlayerServers - 1,
                        "s",
                        uiInfo->findPlayerName);
            }
            else
            {
                Com_sprintf(uiInfo->foundPlayerServerAddresses[uiInfo->numFoundPlayerServers + 15], 0x40u, "no servers found");
            }
            uiInfo->nextFindPlayerRefresh = 0;
        }
        else
        {
            uiInfo->nextFindPlayerRefresh = uiInfo->uiDC.realTime + 25;
        }
    }
#endif
}

char *__cdecl stristr(char *str, char *charset)
{
    int v2; // esi
    int i; // [esp+4h] [ebp-4h]

    while ( *str )
    {
        for ( i = 0; charset[i]; ++i )
        {
            if ( !str[i] )
                break;
            v2 = toupper(charset[i]);
            if ( v2 != toupper(str[i]) )
                break;
        }
        if ( !charset[i] )
            return str;
        ++str;
    }
    return 0;
}

char *__cdecl UI_SelectedMap(int index, int *actual)
{
    int c; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    c = 0;
    *actual = 0;
    for (i = 0; i < sharedUiInfo.mapCount; ++i)
    {
        if (sharedUiInfo.mapList[i].active)
        {
            if (c == index)
            {
                *actual = i;
                return UI_SafeTranslateString(sharedUiInfo.mapList[i].mapName);
            }
            ++c;
        }
    }
    return (char *)"";
}

int __cdecl UI_GameType_HandleKey(int flags, int key, int resetMap)
{
    int oldCount; // [esp+0h] [ebp-8h]
    int nextGameType; // [esp+4h] [ebp-4h]

    if (key != 200 && key != 201 && key != 13 && key != 191)
        return 0;
    oldCount = UI_MapCountByGameType();
    if (key != 201)
    {
        nextGameType = ui_netGameTypeName->current.integer + 1;
        if (nextGameType >= sharedUiInfo.numGameTypes)
            goto LABEL_7;
        if (ui_netGameTypeName->current.integer == 1)
            nextGameType = 3;
    LABEL_14:
        Dvar_SetInt((dvar_s*)ui_netGameTypeName, nextGameType);
        goto LABEL_15;
    }
    nextGameType = ui_netGameTypeName->current.integer - 1;
    if (ui_netGameTypeName->current.integer != 3)
    {
        if (nextGameType < 2)
            nextGameType = sharedUiInfo.numGameTypes - 1;
        goto LABEL_14;
    }
LABEL_7:
    Dvar_SetInt((dvar_s *)ui_netGameTypeName, 1);
LABEL_15:
    if (resetMap)
    {
        if (oldCount != UI_MapCountByGameType())
            Dvar_SetInt((dvar_s *)ui_currentMap, 0);
    }
    return 1;
}

int __cdecl UI_NetGameType_HandleKey(int flags, int key)
{
    int integer; // [esp+0h] [ebp-8h]
    int nextNetGameType; // [esp+4h] [ebp-4h]

    if (key != 200 && key != 201 && key != 13 && key != 191)
        return 0;
    if (key == 201)
    {
        if (ui_netGameType->current.integer)
            integer = ui_netGameType->current.integer;
        else
            integer = sharedUiInfo.numGameTypes;
        Dvar_SetInt((dvar_s*)ui_netGameType, integer - 1);
    }
    else
    {
        nextNetGameType = ui_netGameType->current.integer + 1;
        if (nextNetGameType == sharedUiInfo.numGameTypes)
            nextNetGameType = 0;
        Dvar_SetInt((dvar_s *)ui_netGameType, nextNetGameType);
    }
    Dvar_SetString((dvar_s *)ui_netGameTypeName, sharedUiInfo.gameTypes[ui_netGameType->current.integer].gameType);
    UI_MapCountByGameType();
    if (UI_IsMapActive(ui_currentNetMap->current.integer))
        UI_SelectListIndexForMapIndex(ui_currentNetMap->current.integer);
    else
        UI_SelectFirstActiveMap();
    UI_SelectCurrentMap(0);
    return 1;
}

bool __cdecl UI_IsMapActive(int mapIndex)
{
    if ((mapIndex < 0 || mapIndex >= sharedUiInfo.mapCount)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main_pc.cpp",
            238,
            0,
            "%s\n\t(mapIndex) = %i",
            "(mapIndex >= 0 && mapIndex < sharedUiInfo.mapCount)",
            mapIndex))
    {
        __debugbreak();
    }
    return sharedUiInfo.mapList[mapIndex].active != 0;
}

int UI_SelectFirstActiveMap()
{
    int result; // eax
    int mapIndex; // [esp+4h] [ebp-4h]

    for ( mapIndex = 0; mapIndex < sharedUiInfo.mapCount; ++mapIndex )
    {
        if ( sharedUiInfo.mapList[mapIndex].levelShot )
        {
            Menu_SetFeederSelection(0, &uiInfoArray->uiDC, 0, 4, 0, "createserver");
            Dvar_SetInt((dvar_s *)ui_currentNetMap, mapIndex);
            return 1;
        }
        result = mapIndex + 1;
    }
    return result;
}

void __cdecl UI_SelectListIndexForMapIndex(int mapIndex)
{
    int listIndex; // [esp+0h] [ebp-4h]

    listIndex = UI_GetListIndexFromMapIndex(mapIndex);
    Menu_SetFeederSelection(0, &uiInfoArray->uiDC, 0, 4, listIndex, "createserver");
}

int __cdecl UI_JoinGameType_HandleKey(int flags, int key)
{
    int integer; // [esp+0h] [ebp-8h]
    int nextJoinGameType; // [esp+4h] [ebp-4h]

    if ( key != 200 && key != 201 && key != 13 && key != 191 )
        return 0;
    if ( key == 201 )
    {
        if ( ui_joinGameType->current.integer )
            integer = ui_joinGameType->current.integer;
        else
            integer = sharedUiInfo.gameTypeMapCount[31];
        Dvar_SetInt((dvar_s *)ui_joinGameType, integer - 1);
    }
    else
    {
        nextJoinGameType = ui_joinGameType->current.integer + 1;
        if ( nextJoinGameType == sharedUiInfo.gameTypeMapCount[31] )
            nextJoinGameType = 0;
        Dvar_SetInt((dvar_s *)ui_joinGameType, nextJoinGameType);
    }
    UI_BuildServerDisplayList(0, uiInfoArray, 1);
    return 1;
}

int __cdecl UI_JoinMod_HandleKey(int flags, int key)
{
    const char *nextJoinMod; // [esp+0h] [ebp-4h]
    int nextJoinModa; // [esp+0h] [ebp-4h]

    if ( key != 200 && key != 201 && key != 13 && key != 191 )
        return 0;
    if ( key == 201 )
    {
        if ( ui_browserMod->current.integer - 1 >= -2 )
            nextJoinMod = ui_browserMod->current.string;
        else
            nextJoinMod = sharedUiInfo.modList[63].modDescr;
        nextJoinModa = (int)(nextJoinMod - 1);
    }
    else
    {
        nextJoinModa = ui_browserMod->current.integer + 1;
        if ( (const char *)nextJoinModa == sharedUiInfo.modList[63].modDescr )
            nextJoinModa = -2;
    }
    if ( (nextJoinModa < -2 || nextJoinModa > 64)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main_pc.cpp",
                    1106,
                    0,
                    "nextJoinMod not in [ALLMODS, MAX_MODS]\n\t%i not in [%i, %i]",
                    nextJoinModa,
                    -2,
                    64) )
    {
        __debugbreak();
    }
    Dvar_SetInt((dvar_s *)ui_browserMod, nextJoinModa);
    UI_BuildServerDisplayList(0, uiInfoArray, 1);
    return 1;
}

void __cdecl UI_NetSource_UpdateDisplayList(int source)
{
    Dvar_SetInt((dvar_s *)ui_netSource, source);
    UI_BuildServerDisplayList(0, uiInfoArray, 1);
}

int __cdecl UI_NetSource_HandleKey(int flags, int key)
{
    int integer; // [esp+0h] [ebp-8h]
    int nextNetSource; // [esp+4h] [ebp-4h]

    if ( key != 200 && key != 201 && key != 13 && key != 191 )
        return 0;
    if ( key == 201 )
    {
        if ( ui_netSource->current.integer )
            integer = ui_netSource->current.integer;
        else
            integer = 4;
        UI_NetSource_UpdateDisplayList(integer - 1);
    }
    else
    {
        nextNetSource = ui_netSource->current.integer + 1;
        if ( ui_netSource->current.integer == 3 )
            nextNetSource = 0;
        UI_NetSource_UpdateDisplayList(nextNetSource);
    }
    return 1;
}

const int numServerFilters = 1;
int __cdecl UI_NetFilter_HandleKey(int flags, int key)
{
    if ( key != 200 && key != 201 && key != 13 && key != 191 )
        return 0;
    if ( key == 201 )
        --ui_serverFilterType;
    else
        ++ui_serverFilterType;
    if ( ui_serverFilterType < numServerFilters )
    {
        if ( ui_serverFilterType < 0 )
            ui_serverFilterType = numServerFilters - 1;
    }
    else
    {
        ui_serverFilterType = 0;
    }
    UI_BuildServerDisplayList(0, uiInfoArray, 1);
    return 1;
}

void __cdecl UI_DrawGameType(
                int contextIndex,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                const float *color,
                int textStyle)
{
    char *v6; // eax

    if ( sharedUiInfo.gameTypes[ui_netGameTypeName->current.integer].gameType[8] )
        v6 = UI_SafeTranslateString(&sharedUiInfo.gameTypes[ui_netGameTypeName->current.integer].gameType[8]);
    else
        v6 = UI_SafeTranslateString("EXE_ALL");
    UI_DrawText(
        &scrPlaceView[contextIndex],
        v6,
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

void __cdecl UI_DrawNetGameType(
                int contextIndex,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                const float *color,
                int textStyle)
{
    char *v6; // eax

    if (ui_netGameType->current.integer > sharedUiInfo.numGameTypes)
    {
        Dvar_SetInt((dvar_s*)ui_netGameType, 0);
        Dvar_SetString((dvar_s *)ui_netGameTypeName, sharedUiInfo.gameTypes[0].gameType);
    }

    if (sharedUiInfo.gameTypes[ui_netGameType->current.integer].gameTypeName[0])
        v6 = UI_SafeTranslateString(sharedUiInfo.gameTypes[ui_netGameType->current.integer].gameTypeName);
    else
        v6 = UI_SafeTranslateString("EXE_ALL");

    UI_DrawText(
        &scrPlaceView[contextIndex],
        v6,
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

void __cdecl UI_DrawJoinGameType(
                int contextIndex,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                const float *color,
                int textStyle)
{
    char *v6; // eax

    if (ui_joinGameType->current.integer > sharedUiInfo.numJoinGameTypes)
        Dvar_SetInt((dvar_s*)ui_joinGameType, 0);
    if (sharedUiInfo.joinGameTypes[ui_joinGameType->current.integer].gameTypeName[0])
        v6 = UI_SafeTranslateString(sharedUiInfo.joinGameTypes[ui_joinGameType->current.integer].gameTypeName);
    else
        v6 = UI_SafeTranslateString("EXE_ALL");
    UI_DrawText(
        &scrPlaceView[contextIndex],
        v6,
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

void __cdecl UI_DrawJoinMod(
                int contextIndex,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                const float *color,
                int textStyle)
{
    char *v6; // eax

    if (ui_browserMod->current.integer > sharedUiInfo.modCount)
        Dvar_SetInt((dvar_s*)ui_browserMod, -2);
    if (ui_browserMod->current.integer == -2)
    {
        v6 = UI_SafeTranslateString("EXE_ALL");
    }
    else if (ui_browserMod->current.integer == -1)
    {
        v6 = UI_SafeTranslateString("EXE_NO");
    }
    else
    {
        if (ui_browserMod->current.integer >= 0x40u
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main_pc.cpp",
                1247,
                0,
                "ui_browserMod->current.integer doesn't index MAX_MODS\n\t%i not in [0, %i)",
                ui_browserMod->current.integer,
                64))
        {
            __debugbreak();
        }
        v6 = UI_SafeTranslateString(sharedUiInfo.modList[ui_browserMod->current.integer].modName);
    }
    UI_DrawText(
        &scrPlaceView[contextIndex],
        v6,
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

void __cdecl UI_DrawNetFilter(
                int contextIndex,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                const float *color,
                int textStyle)
{
    const char **ServerFilter; // eax
    const char *v7; // eax
    int filter; // [esp+1Ch] [ebp-Ch] BYREF
    const char *pszTeanslation; // [esp+24h] [ebp-4h]

    if ( ui_serverFilterType < 0 || ui_serverFilterType > numServerFilters )
        ui_serverFilterType = 0;
    ServerFilter = UI_GetServerFilter((const char **)&filter, ui_serverFilterType);
    v7 = va("EXE_SERVERFILTER %s", *ServerFilter);
    pszTeanslation = SEH_LocalizeTextMessage(v7, "server filter", LOCMSG_SAFE);
    UI_DrawText(
        &scrPlaceView[contextIndex],
        (char *)pszTeanslation,
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

void __cdecl UI_DrawNetSource(
                int contextIndex,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                const float *color,
                int textStyle)
{
    const char *v6; // eax
    char *translation; // [esp+1Ch] [ebp-4h]

    v6 = va("EXE_NETSOURCE %s", netSources[ui_netSource->current.integer]);
    translation = (char *)SEH_LocalizeTextMessage(v6, "net source", LOCMSG_SAFE);
    UI_DrawText(
        &scrPlaceView[contextIndex],
        translation,
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

Material *__cdecl UI_GetLevelShot(int index)
{
    if ( index < 0 || index >= sharedUiInfo.mapCount )
        index = 0;
    if ( !sharedUiInfo.mapList[index].timeToBeat[31] )
        sharedUiInfo.mapList[index].timeToBeat[31] = (int)Material_RegisterHandle(
                                                                                                                (char *)&sharedUiInfo.mapList[index].mapPackTypeIndex,
                                                                                                                3);
    return (Material *)sharedUiInfo.mapList[index].timeToBeat[31];
}

void __cdecl UI_DrawMapPreview(int contextIndex, const rectDef_s *rect, const float *color)
{
    Material *mtl; // [esp+20h] [ebp-4h]

    mtl = UI_GetLevelShot(ui_currentNetMap->current.integer);
    UI_DrawHandlePic(
        &scrPlaceView[contextIndex],
        rect->x,
        rect->y,
        rect->w,
        rect->h,
        rect->horzAlign,
        rect->vertAlign,
        color,
        mtl);
}

bool __cdecl UI_LodMods_ShouldSkipMap(const char *dir)
{
    return I_strnicmp(dir, "mp_", 3) != 0;
}

char dirlist[8192];
void __cdecl UI_LoadMods()
{
    unsigned int v0; // [esp+10h] [ebp-44h]
    unsigned int v1; // [esp+30h] [ebp-24h]
    int numdirs; // [esp+40h] [ebp-14h]
    char *dirptr; // [esp+44h] [ebp-10h]
    int i; // [esp+4Ch] [ebp-8h]

    sharedUiInfo.modCount = 0;
    sharedUiInfo.modIndex = 0;
    numdirs = FS_GetFileList("$modlist", (char*)"", FS_LIST_ALL, dirlist, 0x2000);
    dirptr = dirlist;
    for (i = 0; i < numdirs; ++i)
    {
        if (UI_LodMods_ShouldSkipMap(dirptr))
        {
            v1 = strlen(dirptr);
            dirptr += v1 + strlen(&dirptr[v1 + 1]) + 2;
        }
        else
        {
            v0 = strlen(dirptr);
            sharedUiInfo.modList[sharedUiInfo.modCount].modName = String_Alloc(dirptr);
            sharedUiInfo.modList[sharedUiInfo.modCount].modDescr = String_Alloc(&dirptr[v0 + 1]);
            dirptr += v0 + strlen(&dirptr[v0 + 1]) + 2;
            if (++sharedUiInfo.modCount >= 64)
                return;
        }
    }
}

void __cdecl UI_AcceptFriend()
{
    if ( !ui_friendPendingSelectedInd
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main_pc.cpp",
                    1859,
                    0,
                    "%s",
                    "ui_friendPendingSelectedInd") )
    {
        __debugbreak();
    }
    //BLOPS_NULLSUB();
}

void __cdecl UI_DeclineFriend()
{
    if ( !ui_friendPendingSelectedInd
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main_pc.cpp",
                    1901,
                    0,
                    "%s",
                    "ui_friendPendingSelectedInd") )
    {
        __debugbreak();
    }
    //BLOPS_NULLSUB();
}

void __cdecl UI_AcceptInvite()
{
    unsigned __int64 v0; // rax

    LODWORD(v0) = Live_GetInviteFriend(ui_inviteSelectedInd->current.integer);
    Live_AcceptInvite(v0);
}

void __cdecl UI_ClearMods()
{
    dvar_s *Var; // eax

    if ( useFastFile->current.enabled )
        DB_SyncXAssets();
    Var = (dvar_s *)Dvar_FindVar("fs_game");
    Dvar_Reset(Var, DVAR_SOURCE_INTERNAL);
    Cbuf_AddText(0, "vid_restart\n");
}

