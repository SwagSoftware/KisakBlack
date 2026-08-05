#include "ui_feeders.h"
#include <qcommon/com_clients.h>
#include "ui_server.h"
#include "ui_main_pc.h"
#include <qcommon/com_gamemodes.h>
#include <ui_mp/ui_feeders_mp.h>
#include "ui_utils.h"
#include "ui_playlists.h"
#include <live/live_leaderboard.h>
#include <live/live_friends_pc.h>
#include <live/live_win.h>
#include <live/live_meetplayer.h>
#include "ui_friends.h"
#include <live/live_fileshare.h>
#include <client_mp/cl_ui_mp.h>
#include <client_mp/cl_cgame_mp.h>
#include <live/live_pcache.h>
#include <live/live_pcache_profile.h>
#include <client/cl_rank.h>
#include <client_mp/cl_scrn_mp.h>
#include <client_mp/cl_main_pc_mp.h>
#include "ui_emblem.h"
#include <live/live_fileshare_search.h>
#include <cgame/cg_compass.h>


XuidInfo recentPlayerInfo;
FriendInfo friendInfo;

int __cdecl UI_FeederCount(int localClientNum, int contextIndex, float feederID, listBoxDef_s *listPtr)
{
    int result; // eax
    int CategoryIdForNum; // eax
    int leaderboardFeederCount; // [esp+Ch] [ebp-14h]
    signed int noOfMetPlayers; // [esp+10h] [ebp-10h]
    int friendCount; // [esp+14h] [ebp-Ch]
    int controllerIndex; // [esp+18h] [ebp-8h]
    uiInfo_s *uiInfo; // [esp+1Ch] [ebp-4h]

    if (localClientNum == -1
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp",
            284,
            0,
            "%s",
            "localClientNum != INVALID_CLIENT_NUMBER"))
    {
        __debugbreak();
    }
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if (controllerIndex == -1
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp",
            288,
            0,
            "%s",
            "controllerIndex != INVALID_CONTROLLER_PORT"))
    {
        __debugbreak();
    }
    uiInfo = UI_UIContext_GetInfo(contextIndex);
    switch ((int)feederID)
    {
    case 2:
        UI_UpdateDisplayServers(localClientNum, uiInfo);
        UI_UpdateServerToolTips(localClientNum, listPtr);
        result = sharedUiInfo.serverStatus.numDisplayServers;
        break;
    case 4:
        result = UI_MapCountByGameType();
        break;
    case 7:
    case 19:
        if (uiInfo->uiDC.realTime > uiInfo->playerRefresh)
        {
            uiInfo->playerRefresh = uiInfo->uiDC.realTime + 3000;
            UI_BuildPlayerList(localClientNum);
        }
        result = sharedUiInfo.playerCount;
        break;
    case 9:
        result = sharedUiInfo.modCount;
        break;
    case 13:
        result = sharedUiInfo.serverStatusInfo.numLines;
        break;
    case 26:
    case 77:
        if (xblive_wagermatch->current.enabled)
            CategoryIdForNum = Playlist_GetCategoryIdForNum(wagerCategory->current.integer);
        else
            CategoryIdForNum = Playlist_GetCategoryIdForNum(category->current.integer);
        result = Playlist_GetPlaylistCount(CategoryIdForNum);
        break;
    case 27:
        result = Playlist_GetGametypeCount();
        break;
    case 28:
    case 76:
        result = Playlist_GetCategoryCount();
        break;
    case 29:
        leaderboardFeederCount = LB_FeederCount(localClientNum);
        Dvar_SetBool((dvar_s*)ui_leaderboardFeederCountNotZero, leaderboardFeederCount > 0);
        result = leaderboardFeederCount;
        break;
    case 32:
        friendCount = Friends_GetCount(controllerIndex, 0);
        Dvar_SetBoolIfChanged((dvar_s *)ui_friendCountNotZero, friendCount > 0);
        result = friendCount;
        break;
    case 45:
    case 46:
        result = 0;
        break;
    case 47:
        result = Live_GetInvitesCount();
        break;
    case 50:
        noOfMetPlayers = LiveMeetPlayer_GetNoOfMetPlayers(controllerIndex);
        LiveMeetPlayer_SortMetPlayers(controllerIndex, noOfMetPlayers);
        Dvar_SetBool((dvar_s *)ui_recentPlayerCountNotZero, noOfMetPlayers > 0);
        result = noOfMetPlayers;
        break;
    case 54:
        result = listPtr->rowCount;
        break;
    case 74:
    case 84:
        result = Live_FileShare_GetSearchResultsCount();
        break;
    case 79:
        result = Live_FileShare_GetOtherPrivateSlotsCount();
        break;
    case 80:
        result = Live_FileShare_GetMyPrivateSlotsCount();
        break;
    case 86:
        result = 3 * Live_FileShare_GetMyPrivateSlotsCount();
        break;
    case 91:
        result = sharedUiInfo.serverStatusInfoScoreBoard.numLines;
        break;
    default:
        result = UI_Project_FeederCount(localClientNum, contextIndex, feederID, listPtr);
        break;
    }
    return result;
}

void __cdecl UI_BuildPlayerList(int localClientNum)
{
    char *v1; // eax
    char *info; // [esp+0h] [ebp-844h]
    uiClientState_s state; // [esp+4h] [ebp-840h] BYREF
    char szName[40]; // [esp+810h] [ebp-34h] BYREF
    int n; // [esp+83Ch] [ebp-8h]
    int count; // [esp+840h] [ebp-4h]

    CL_GetClientState(localClientNum, &state);
    info = CL_GetConfigString(0);
    v1 = Info_ValueForKey(info, "com_maxclients");
    count = atoi(v1);
    memset(sharedUiInfo.playerClientNums, 0xFFu, sizeof(sharedUiInfo.playerClientNums));
    sharedUiInfo.playerCount = 0;
    for (n = 0; n < count; ++n)
    {
        if (CL_GetClientName(localClientNum, n, szName, 38, 1))
        {
            sharedUiInfo.playerClientNums[sharedUiInfo.playerCount] = n;
            I_strncpyz(sharedUiInfo.playerNames[sharedUiInfo.playerCount], szName, 32);
            I_CleanStr(sharedUiInfo.playerNames[sharedUiInfo.playerCount]);
            ++sharedUiInfo.playerCount;
        }
    }
}

_CustomClassDescription *__cdecl UI_FeederItemText(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                float feederID,
                itemGroup_t index,
                int column,
                Material **handle,
                float *angle,
                bool *useOwnerDraw)
{
    unsigned int CategoryIdForNum; // eax
    int actual; // [esp+214h] [ebp-Ch] BYREF
    int controllerIndex; // [esp+218h] [ebp-8h]
    listBoxDef_s *listPtr; // [esp+21Ch] [ebp-4h]

    listPtr = Item_GetListBoxDef(item);
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( controllerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp",
                    2124,
                    0,
                    "%s",
                    "controllerIndex != INVALID_CONTROLLER_PORT") )
    {
        __debugbreak();
    }
    *handle = 0;
    switch ( (int)feederID )
    {
        case 1:
            return (_CustomClassDescription *)UI_FeederItemText_Maps(controllerIndex, item, index, column, handle);
        case 2:
            return (_CustomClassDescription *)UI_FeederItemText_Servers(localClientNum, contextIndex, index, column, handle);
        case 4:
            return (_CustomClassDescription *)UI_SelectedMap(index, &actual);
        case 7:
            if (index < 0 || index >= sharedUiInfo.playerCount)
                return (_CustomClassDescription *)"";
            if ( column )
            {
                if ( column == 1 )
                    return (_CustomClassDescription*)sharedUiInfo.playerNames[index];
            }
            else
            {
                *handle = sharedUiInfo.assets.whiteMaterial;
            }
            return (_CustomClassDescription *)"";
        case 9:
            return (_CustomClassDescription *)UI_FeederItemText_Mods(index);
        case 13:
            return (_CustomClassDescription *)UI_FeederItemText_ServerStatus(index, column);
        case 19:
            return (_CustomClassDescription *)UI_FeederItemText_MuteList(localClientNum, contextIndex, index, column);
        case 26:
            return (_CustomClassDescription *)UI_FeederItemText_Playlists(controllerIndex, index);
        case 27:
            if ( index < ITEMGROUP_SMG || index >= Playlist_GetGametypeCount() )
                return (_CustomClassDescription *)"";
            else
                return (_CustomClassDescription *)Playlist_GetGametypeLocalizedName(index);
        case 28:
            if ( index < ITEMGROUP_SMG || index >= Playlist_GetCategoryCount() )
                return (_CustomClassDescription *)"";
            CategoryIdForNum = Playlist_GetCategoryIdForNum(index);
            return (_CustomClassDescription *)Playlist_GetCategoryLocalizedName(controllerIndex, CategoryIdForNum);
        case 29:
            return (_CustomClassDescription *)LB_FeederItemText(localClientNum, index, column, handle);
        case 32:
        case 50:
        case 101:
            return (_CustomClassDescription *)UI_FeederItemText_CommmonPlayerListHandler(
                                                                                    controllerIndex,
                                                                                    (int)feederID,
                                                                                    index,
                                                                                    column,
                                                                                    handle,
                                                                                    useOwnerDraw);
        case 38:
            return (_CustomClassDescription *)UI_FeederItemText_CustomGametypes(index);
        case 45:
            return (_CustomClassDescription *)"";
        case 46:
            return (_CustomClassDescription *)"";
        case 47:
            return (_CustomClassDescription *)UI_FeederItemText_Invites(
                                                                                    controllerIndex,
                                                                                    (int)feederID,
                                                                                    index,
                                                                                    column,
                                                                                    handle,
                                                                                    useOwnerDraw);
        case 54:
            return (_CustomClassDescription *)UI_FeederItemText_DynamicMenu(contextIndex, index, column, listPtr, handle);
        case 70:
            return (_CustomClassDescription *)"";
        case 74:
            return (_CustomClassDescription *)Live_FileShare_SearchResultsItemText(
                                                                                    controllerIndex,
                                                                                    listPtr->startPos[contextIndex],
                                                                                    listPtr->cursorPos[contextIndex],
                                                                                    index,
                                                                                    useOwnerDraw);
        case 76:
            return (_CustomClassDescription *)UI_FeederItemText_NewCategories(controllerIndex, item, index, column, handle);
        case 77:
            return (_CustomClassDescription *)UI_FeederItemText_NewPlaylists(controllerIndex, item, index, column, handle);
        case 79:
            return (_CustomClassDescription *)Live_FileShare_PrivateSlotsText(controllerIndex, 79, index, column, handle);
        case 80:
            return (_CustomClassDescription *)Live_FileShare_PrivateSlotsText(controllerIndex, 80, index, column, handle);
        case 84:
            return (_CustomClassDescription *)"";
        case 86:
            return (_CustomClassDescription *)Live_FileShare_PrivateSlotsIngameText(controllerIndex, index, column, handle);
        case 91:
            return (_CustomClassDescription *)UI_FeederItemText_ServerStatusScoreboard(index, column);
        default:
            return UI_Project_FeederItemText(
                             localClientNum,
                             contextIndex,
                             item,
                             feederID,
                             index,
                             column,
                             handle,
                             angle,
                             useOwnerDraw);
    }
}

int lastColumn;
int lastTime_0;
char info[1024];
char clientBuff[38];
char *__cdecl UI_FeederItemText_Servers(int localClientNum, int contextIndex, int index, int column, Material **handle)
{
    char *v5; // eax
    char *v6; // eax
    const char *result; // eax
    char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    char *v11; // eax
    Material *v12; // eax
    Material *v13; // eax
    char *v14; // eax
    Material *v15; // eax
    char *v16; // eax
    char *v17; // eax
    char *v18; // eax
    char *v19; // eax
    char *v20; // eax
    char *v21; // eax
    char *v22; // [esp-4h] [ebp-40h]
    char *GameTypeDisplayName; // [esp+0h] [ebp-3Ch]
    char *tempstr; // [esp+18h] [ebp-24h]
    char *region; // [esp+20h] [ebp-1Ch]
    char *mod; // [esp+24h] [ebp-18h]
    int iswager; // [esp+28h] [ebp-14h]
    int ping; // [esp+34h] [ebp-8h]
    uiInfo_s *uiInfo; // [esp+38h] [ebp-4h]

    uiInfo = UI_UIContext_GetInfo(contextIndex);
    UI_UpdateDisplayServers(localClientNum, uiInfo);
    if (index < 0 || index >= sharedUiInfo.serverStatus.numDisplayServers)
        return (char *)"";
    if (lastColumn != column || lastTime_0 > uiInfo->uiDC.realTime + 5000)
    {
        LAN_GetServerInfo(
            ui_netSource->current.integer,
            *(_DWORD *)&sharedUiInfo.serverStatusAddress[4 * index - 81328],
            info,
            1024);
        lastColumn = column;
        lastTime_0 = uiInfo->uiDC.realTime;
    }
    v5 = Info_ValueForKey(info, "ping");
    ping = atoi(v5);
    switch (column)
    {
    case 0:
        v10 = Info_ValueForKey(info, "wager");
        iswager = atoi(v10);
        v11 = Info_ValueForKey(info, "licensetype");
        switch (atoi(v11))
        {
        case 2:
            if (iswager)
                v13 = Material_RegisterHandle("wager_green", 7);
            else
                v13 = Material_RegisterHandle("os", 7);
            *handle = v13;
            break;
        case 3:
            *handle = Material_RegisterHandle("no_ors", 7);
            break;
        case 4:
            if (iswager)
                v12 = Material_RegisterHandle("wager_yellow", 7);
            else
                v12 = Material_RegisterHandle("ors", 7);
            *handle = v12;
            break;
        case 5:
            *handle = Material_RegisterHandle("listen_server", 7);
            break;
        default:
            break;
        }
        result = (char *)"";
        break;
    case 1:
        v9 = Info_ValueForKey(info, "pb");
        if (atoi(v9))
            *handle = Material_RegisterHandle("vac", 7);
        result = (char *)"";
        break;
    case 2:
        v6 = Info_ValueForKey(info, "pswrd");
        if (atoi(v6))
            *handle = Material_RegisterHandle("lock", 7);
        result = (char *)"";
        break;
    case 3:
        if (ping > 0)
        {
            v17 = Info_ValueForKey(info, "hostname");
            I_strncpyz(clientBuff, v17, 38);
            UI_UseAltColorPalette(clientBuff);
            result = clientBuff;
        }
        else
        {
            result = Info_ValueForKey(info, "addr");
        }
        break;
    case 4:
        v18 = Info_ValueForKey(info, "mapname");
        result = UI_GetMapDisplayName(v18);
        break;
    case 5:
        v22 = Info_ValueForKey(info, "com_maxclients");
        v19 = Info_ValueForKey(info, "clients");
        Com_sprintf(clientBuff, 0x26u, "%s (%s)", v19, v22);
        result = clientBuff;
        break;
    case 6:
        if (Info_ValueForKey(info, "gametype") && *Info_ValueForKey(info, "gametype"))
        {
            v20 = Info_ValueForKey(info, "gametype");
            tempstr = UI_GetGameTypeDisplayShortName(v20);
            if (*tempstr)
            {
                GameTypeDisplayName = tempstr;
            }
            else
            {
                v21 = Info_ValueForKey(info, "gametype");
                GameTypeDisplayName = UI_GetGameTypeDisplayName(v21);
            }
            result = GameTypeDisplayName;
        }
        else
        {
            result = "?";
        }
        break;
    case 7:
        v14 = Info_ValueForKey(info, "licensetype");
        if (atoi(v14) != 5)
        {
            v16 = Info_ValueForKey(info, "region");
            switch (atoi(v16))
            {
            case 1:
                goto LABEL_34;
            case 2:
                goto LABEL_36;
            case 3:
                goto LABEL_38;
            case 4:
                goto LABEL_40;
            case 5:
                goto LABEL_42;
            case 6:
                goto LABEL_44;
            case 7:
                goto LABEL_46;
            case 8:
                goto LABEL_48;
            case 9:
                goto LABEL_50;
            case 10:
                goto LABEL_52;
            case 11:
                goto LABEL_54;
            case 12:
                goto LABEL_56;
            case 13:
                goto LABEL_58;
            case 14:
                goto LABEL_60;
            case 15:
                goto LABEL_62;
            case 16:
                goto LABEL_64;
            case 17:
                goto LABEL_66;
            case 18:
                goto LABEL_68;
            case 19:
                goto LABEL_73;
            default:
                goto LABEL_72;
            }
        }
        region = Info_ValueForKey(info, "countrycode");
        if (I_stricmp(region, "US"))
        {
            if (I_stricmp(region, "CA"))
            {
                if (I_stricmp(region, "NL"))
                {
                    if (I_stricmp(region, "GB"))
                    {
                        if (I_stricmp(region, "DK"))
                        {
                            if (I_stricmp(region, "PL"))
                            {
                                if (I_stricmp(region, "DE"))
                                {
                                    if (I_stricmp(region, "AU"))
                                    {
                                        if (I_stricmp(region, "JP"))
                                        {
                                            if (I_stricmp(region, "RU"))
                                            {
                                                if (I_stricmp(region, "ES"))
                                                {
                                                    if (I_stricmp(region, "FR"))
                                                    {
                                                        if (I_stricmp(region, "IT"))
                                                        {
                                                            if (I_stricmp(region, "GR"))
                                                            {
                                                                if (I_stricmp(region, "IS"))
                                                                {
                                                                    if (I_stricmp(region, "ZA"))
                                                                    {
                                                                        if (I_stricmp(region, "SA"))
                                                                        {
                                                                            if (I_stricmp(region, "KR"))
                                                                            {
                                                                                if (I_stricmp(region, "AE"))
                                                                                    LABEL_72:
                                                                                v15 = Material_RegisterHandle("sb_globe", 7);
                                                                                else
                                                                                    LABEL_73:
                                                                                v15 = Material_RegisterHandle("flag_dubai", 7);
                                                                            }
                                                                            else
                                                                            {
                                                                            LABEL_68:
                                                                                v15 = Material_RegisterHandle("flag_skorea", 7);
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                        LABEL_66:
                                                                            v15 = Material_RegisterHandle("flag_saudi", 7);
                                                                        }
                                                                    }
                                                                    else
                                                                    {
                                                                    LABEL_64:
                                                                        v15 = Material_RegisterHandle("flag_safrica", 7);
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                LABEL_62:
                                                                    v15 = Material_RegisterHandle("flag_israel", 7);
                                                                }
                                                            }
                                                            else
                                                            {
                                                            LABEL_60:
                                                                v15 = Material_RegisterHandle("flag_greece", 7);
                                                            }
                                                        }
                                                        else
                                                        {
                                                        LABEL_58:
                                                            v15 = Material_RegisterHandle("flag_italy", 7);
                                                        }
                                                    }
                                                    else
                                                    {
                                                    LABEL_56:
                                                        v15 = Material_RegisterHandle("flag_france", 7);
                                                    }
                                                }
                                                else
                                                {
                                                LABEL_54:
                                                    v15 = Material_RegisterHandle("flag_spain", 7);
                                                }
                                            }
                                            else
                                            {
                                            LABEL_52:
                                                v15 = Material_RegisterHandle("flag_russia", 7);
                                            }
                                        }
                                        else
                                        {
                                        LABEL_50:
                                            v15 = Material_RegisterHandle("flag_japan", 7);
                                        }
                                    }
                                    else
                                    {
                                    LABEL_48:
                                        v15 = Material_RegisterHandle("flag_australia", 7);
                                    }
                                }
                                else
                                {
                                LABEL_46:
                                    v15 = Material_RegisterHandle("flag_germany", 7);
                                }
                            }
                            else
                            {
                            LABEL_44:
                                v15 = Material_RegisterHandle("flag_poland", 7);
                            }
                        }
                        else
                        {
                        LABEL_42:
                            v15 = Material_RegisterHandle("flag_denmark", 7);
                        }
                    }
                    else
                    {
                    LABEL_40:
                        v15 = Material_RegisterHandle("flag_brit", 7);
                    }
                }
                else
                {
                LABEL_38:
                    v15 = Material_RegisterHandle("flag_neth", 7);
                }
            }
            else
            {
            LABEL_36:
                v15 = Material_RegisterHandle("flag_canada", 7);
            }
        }
        else
        {
        LABEL_34:
            v15 = Material_RegisterHandle("flag_usa", 7);
        }
        *handle = v15;
        return (char *)"";
    case 8:
        if (ping > 0)
            result = Info_ValueForKey(info, "ping");
        else
            result = "...";
        break;
    case 9:
        mod = Info_ValueForKey(info, "game");
        if (mod && I_stristr(mod, "mods/"))
            result = mod + 5;
        else
            result = (char *)"";
        break;
    case 10:
        v8 = Info_ValueForKey(info, "voice");
        if (atoi(v8))
            result = "X";
        else
            result = (char *)"";
        break;
    default:
        result = "Dt";
        break;
    }
    return (char*)result;
}

char *__cdecl UI_FeederItemText_ServerStatus(int index, unsigned int column)
{
    if ( index < 0 || index >= (int)sharedUiInfo.serverStatusInfoScoreBoard.lines[2][5] || column > 1 )
        return (char *)"";
    if ( sharedUiInfo.serverStatusInfo.lines[index + 4][column + 6]
        && *sharedUiInfo.serverStatusInfo.lines[index + 4][column + 6] == 64 )
    {
        return UI_SafeTranslateString(sharedUiInfo.serverStatusInfo.lines[index + 4][column + 6] + 1);
    }
    return (char *)sharedUiInfo.serverStatusInfo.lines[index + 4][column + 6];
}

char *__cdecl UI_FeederItemText_ServerStatusScoreboard(int index, unsigned int column)
{
    if ( index < 0 || index >= sharedUiInfo.pendingServerStatus.server[0].valid || column > 6 )
        return (char *)"";
    if ( *sharedUiInfo.serverStatusInfoScoreBoard.lines[index + 4][column + 6] == 64 )
        return UI_SafeTranslateString(sharedUiInfo.serverStatusInfoScoreBoard.lines[index + 4][column + 6] + 1);
    return (char *)sharedUiInfo.serverStatusInfoScoreBoard.lines[index + 4][column + 6];
}

const char *__cdecl UI_FeederItemText_Mods(int index)
{
    if ( index < 0 || index >= (int)sharedUiInfo.modList[63].modDescr )
        return "";
    if ( sharedUiInfo.modList[index].modName && *sharedUiInfo.modList[index].modName )
        return sharedUiInfo.modList[index].modName;
    return (const char *)sharedUiInfo.serverHardwareIconList[2 * index + 9];
}

const char *__cdecl UI_FeederItemText_CommmonPlayerListHandler(
                int controllerIndex,
                int feederId,
                int index,
                int column,
                Material **handle,
                bool *useOwnerDraw)
{
    __int16 PlayerEmblemBackgroundID; // ax
    char *v8; // eax
    char *v9; // eax
    unsigned int LastMetTime; // eax
    int v11; // [esp-4h] [ebp-9Ch]
    XuidInfo result; // [esp+10h] [ebp-88h] BYREF
    _BYTE v13[48]; // [esp+40h] [ebp-58h] BYREF
    int prestige; // [esp+74h] [ebp-24h] BYREF
    const char *emblemBackingName; // [esp+78h] [ebp-20h]
    int rank; // [esp+7Ch] [ebp-1Ch] BYREF
    int codpoints; // [esp+80h] [ebp-18h] BYREF
    const char *presenceString; // [esp+84h] [ebp-14h]
    Material *emblemBackingMaterial; // [esp+88h] [ebp-10h]
    const char *name; // [esp+8Ch] [ebp-Ch]
    unsigned __int64 xuid; // [esp+90h] [ebp-8h]

    rank = 0;
    prestige = 0;
    codpoints = 0;
    xuid = 0;
    emblemBackingName = "";
    name = "";
    presenceString = "";
    if ( feederId == 32 )
    {
        if ( Friends_GetByIndex(controllerIndex, 0, index, &friendInfo) )
        {
            xuid = friendInfo.dwid;
            name = friendInfo.name;
        }
    }
    else if ( feederId == 50 )
    {
        memcpy(v13, LiveMeetPlayer_GetMetFriendInfo(&result, controllerIndex, index), sizeof(v13));
        memcpy(&recentPlayerInfo, v13, sizeof(recentPlayerInfo));
        xuid = recentPlayerInfo.playerXuids;
        name = recentPlayerInfo.gamertag;
    }
    switch ( column )
    {
        case 0:
            *handle = sharedUiInfo.assets.whiteMaterial;
            return "";
        case 1:
            emblemBackingName = "emblem_bg_nocod";
            PlayerEmblemBackgroundID = PCache_GetPlayerEmblemBackgroundID(controllerIndex, xuid);
            emblemBackingMaterial = BG_EmblemsGetBackgroundMaterial(PlayerEmblemBackgroundID);
            if ( emblemBackingMaterial )
                emblemBackingName = emblemBackingMaterial->info.name;
            *handle = Material_RegisterHandle((char *)emblemBackingName, 7);
            return "";
        case 2:
            *useOwnerDraw = 1;
            return "";
        case 3:
            if ( PCache_GetRank(controllerIndex, xuid, &rank, &prestige) )
                return CL_GetRankData(rank, MP_RANKTABLE_DISPLAYLEVEL);
            else
                return "";
        case 4:
            if ( PCache_GetRank(controllerIndex, xuid, &rank, &prestige) )
                CL_GetRankIcon(rank, prestige, handle);
            return "";
        case 5:
            return name;
        case 6:
            if ( !PCache_GetCodpoints(controllerIndex, xuid, &codpoints) )
                return "";
            v11 = codpoints;
            v8 = UI_SafeTranslateString("MENU_POINTS");
            v9 = UI_ReplaceConversionInt(v8, v11);
            return va("%s", v9);
        case 7:
            if ( feederId == 32 )
            {
                if ( (friendInfo.status & 0x10) != 0 || (friendInfo.status & 0x20) != 0 )
                {
                    if ( (friendInfo.status & 0x10) != 0 )
                        presenceString = "MENU_FRIEND_REQUEST_PENDING";
                    if ( (friendInfo.status & 0x20) != 0 )
                        presenceString = "MENU_FRIEND_REQUEST_RECEIVED";
                }
                else if ( (friendInfo.status & 2) != 0 )
                {
                    if ( !RETURN_ZERO32() )
                        return friendInfo.presence;
                    presenceString = "MENU_PLAYING_COD";
                }
                else if ( (friendInfo.status & 4) != 0 )
                {
                    presenceString = "MENU_ONLINE";
                }
                else if ( (friendInfo.status & 8) != 0 )
                {
                    presenceString = "MENU_OFFLINE";
                }
                return UI_SafeTranslateString(presenceString);
            }
            else if ( feederId == 50 )
            {
                LastMetTime = LiveMeetPlayer_GetLastMetTime(controllerIndex, index);
                return GetRecentPlayerInfoString(LastMetTime / 0x3C);
            }
            else
            {
                return "";
            }
        default:
            return "";
    }
}

char *__cdecl GetRecentPlayerInfoString(int minutes)
{
    char *v1; // eax
    char *v2; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    int hours; // [esp+0h] [ebp-Ch]

    hours = minutes / 60;
    if ( minutes / 1440 < 1 )
    {
        if ( hours < 1 )
        {
            if ( minutes < 1 )
            {
                return UI_SafeTranslateString("MPUI_LAST_MET_LESS_THAN_A_MIN_AGO");
            }
            else
            {
                v6 = UI_SafeTranslateString("MPUI_LAST_MET_N_MINS_AGO");
                v7 = UI_ReplaceConversionInt(v6, minutes);
                return va("%s", v7);
            }
        }
        else
        {
            v4 = UI_SafeTranslateString("MPUI_LAST_MET_N_HOURS_AGO");
            v5 = UI_ReplaceConversionInt(v4, hours);
            return va("%s", v5);
        }
    }
    else
    {
        v1 = UI_SafeTranslateString("MPUI_LAST_MET_N_DAYS_AGO");
        v2 = UI_ReplaceConversionInt(v1, minutes / 1440);
        return va("%s", v2);
    }
}

const char *__cdecl UI_FeederItemText_Invites(
                int controllerIndex,
                int feederId,
                int index,
                int column,
                Material **handle,
                bool *useOwnerDraw)
{
    unsigned __int64 v6; // rax
    XuidInfo result; // [esp+8Ch] [ebp-168h] BYREF
    _BYTE v9[48]; // [esp+BCh] [ebp-138h] BYREF
    XuidInfo metinfo; // [esp+ECh] [ebp-108h] BYREF
    int j; // [esp+120h] [ebp-D4h]
    FriendInfo finfo; // [esp+124h] [ebp-D0h] BYREF
    int i; // [esp+1E0h] [ebp-14h]
    unsigned __int64 friendInviteId; // [esp+1E4h] [ebp-10h]
    int count; // [esp+1ECh] [ebp-8h]
    int invitesCount; // [esp+1F0h] [ebp-4h]

    invitesCount = Live_GetInvitesCount();
    if ( index >= 0 && index < invitesCount )
    {
        LODWORD(v6) = Live_GetInviteFriend(index);
        friendInviteId = v6;
        count = Friends_GetCount(controllerIndex, 0);
        for ( i = 0; i < count; ++i )
        {
            Friends_GetByIndex(controllerIndex, 0, i, &finfo);
            if ( finfo.dwid == friendInviteId )
                return UI_FeederItemText_CommmonPlayerListHandler(controllerIndex, 32, i, column, handle, useOwnerDraw);
        }
        count = LiveMeetPlayer_GetNoOfMetPlayers(0);
        for ( j = 0; j < count; ++j )
        {
            memcpy(v9, LiveMeetPlayer_GetMetFriendInfo(&result, 0, j), sizeof(v9));
            memcpy(&metinfo, v9, sizeof(metinfo));
            if ( metinfo.playerXuids == friendInviteId )
                return UI_FeederItemText_CommmonPlayerListHandler(controllerIndex, 50, j, column, handle, useOwnerDraw);
        }
    }
    return "";
}

char *__cdecl UI_FeederItemText_Playlists(int controllerIndex, int index)
{
    int CategoryIdForNum; // eax
    unsigned int PlaylistIdForNum; // eax
    const char *PlaylistName; // eax
    int currentCategory; // [esp+0h] [ebp-4h]

    if ( xblive_wagermatch->current.enabled )
        currentCategory = wagerCategory->current.integer;
    else
        currentCategory = category->current.integer;
    if ( index < 0 )
        return (char *)"";
    CategoryIdForNum = Playlist_GetCategoryIdForNum(currentCategory);
    if ( index >= Playlist_GetPlaylistCount(CategoryIdForNum) )
        return (char *)"";
    PlaylistIdForNum = Playlist_GetPlaylistIdForNum(controllerIndex, index, currentCategory);
    PlaylistName = Playlist_GetPlaylistName(controllerIndex, PlaylistIdForNum);
    return UI_ToUpper(PlaylistName);
}

const char *__cdecl UI_FeederItemText_NewCategories(
                int controllerIndex,
                itemDef_s *item,
                int index,
                int column,
                Material **handle)
{
    unsigned int CategoryIdForNum; // eax
    listBoxDef_s *listboxPtr; // [esp+4h] [ebp-8h]
    int contextIndex; // [esp+8h] [ebp-4h]

    contextIndex = Com_ControllerIndex_GetUIContextIndex(controllerIndex);
    listboxPtr = Item_GetListBoxDef(item);
    if ( !column )
    {
        *handle = sharedUiInfo.assets.whiteMaterial;
        return "";
    }
    if ( column != 1 )
    {
        if (column == 2 && listboxPtr->cursorPos[contextIndex] == index && Window_HasFocus(contextIndex, &item->window))
        {
            //BLOPS_NULLSUB();
        }
        return "";
    }
    if ( index < 0 || index >= Playlist_GetCategoryCount() )
        return "";
    CategoryIdForNum = Playlist_GetCategoryIdForNum(index);
    return Playlist_GetCategoryLocalizedName(controllerIndex, CategoryIdForNum);
}

bool __cdecl Window_HasFocus(int contextIndex, const windowDef_t *w)
{
    if ( !w && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\ui\\../ui/ui_utils.h", 95, 0, "%s", "w") )
        __debugbreak();
    if ( Window_IsVisible(contextIndex, w) )
        return (Window_GetDynamicFlags(contextIndex, w) & 2) != 0;
    if ( (Window_GetDynamicFlags(contextIndex, w) & 2) != 0
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\ui\\../ui/ui_utils.h", 99, 0, "Hidden window has focus!") )
    {
        __debugbreak();
    }
    return 0;
}

bool __cdecl Window_IsVisible(int contextIndex, const windowDef_t *w)
{
    if ( !w && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\ui\\../ui/ui_utils.h", 85, 0, "%s", "w") )
        __debugbreak();
    return (Window_GetDynamicFlags(contextIndex, w) & 4) != 0;
}

char *__cdecl UI_FeederItemText_NewPlaylists(
                int controllerIndex,
                itemDef_s *item,
                int index,
                int column,
                Material **handle)
{
    listBoxDef_s *listboxPtr; // [esp+4h] [ebp-8h]
    int contextIndex; // [esp+8h] [ebp-4h]

    contextIndex = Com_ControllerIndex_GetUIContextIndex(controllerIndex);
    listboxPtr = Item_GetListBoxDef(item);
    if ( column )
    {
        if ( column == 1 )
            return UI_FeederItemText_Playlists(controllerIndex, index);
        //if ( column == 2 && listboxPtr->cursorPos[contextIndex] == index && Window_HasFocus(contextIndex, &item->window) )
            //BLOPS_NULLSUB();
    }
    else
    {
        *handle = sharedUiInfo.assets.whiteMaterial;
    }
    return (char *)"";
}

char *__cdecl UI_FeederItemText_MuteList(int localClientNum, int contextIndex, int index, int column)
{
    unsigned int ClientNumForPlayerListNum; // eax

    if (index < 0 || index >= sharedUiInfo.playerCount)
        return (char *)"";
    if (column == 1)
        return sharedUiInfo.playerNames[index];
    ClientNumForPlayerListNum = UI_GetClientNumForPlayerListNum(index);
    if (CL_IsPlayerMuted(localClientNum, ClientNumForPlayerListNum))
        return UI_SafeTranslateString("MP_MUTED");
    else
        return (char *)"";
}

char *__cdecl UI_FeederItemText_DynamicMenu(
                int contextIndex,
                int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle)
{
    char *result =  NULL; // eax

    if ( column )
    {
        if ( column == 1 )
        {
            if ( listPtr->cursorPos[contextIndex] == index )
                *handle = sharedUiInfo.assets.whiteMaterial;
            return (char *)"";
        }
        else if ( column == 2 )
        {
            if (listPtr->cursorPos[contextIndex] == index)
            {
                //BLOPS_NULLSUB();
            }
            return (char *)"";
        }
        else if ( listPtr->rows[index].status == 2 )
        {
            return (char *)"";
        }
        else
        {
            switch ( listPtr->rows[index].cells[column].type )
            {
                case 0:
                case 1:
                    result = listPtr->rows[index].cells[column].stringValue;
                    break;
                case 2:
                    *handle = Material_RegisterHandle(listPtr->rows[index].cells[column].stringValue, 7);
                    result = (char *)"";
                    break;
                case 3:
                case 4:
                    if ( listPtr->cursorPos[contextIndex] == index )
                        result = listPtr->rows[index].cells[column].stringValue;
                    else
                        result = (char *)"";
                    break;
                case 5:
                    if ( listPtr->cursorPos[contextIndex] == index )
                        *handle = Material_RegisterHandle(listPtr->rows[index].cells[column].stringValue, 7);
                    result = (char *)"";
                    break;
                default:
                    result = (char *)"";
                    break;
            }
        }
    }
    else
    {
        *handle = sharedUiInfo.assets.whiteMaterial;
        return (char *)"";
    }
    return result;
}

void __cdecl UI_FeederItemOwnerDraw(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                float feederID,
                const rectDef_s *rect,
                int index,
                int col,
                const float *color)
{
    int controllerIndex; // [esp+18h] [ebp-4h]

    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( controllerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp",
                    2314,
                    0,
                    "%s",
                    "controllerIndex != INVALID_CONTROLLER_PORT") )
    {
        __debugbreak();
    }
    switch ( (int)feederID )
    {
        case 18:
            return;
        case 32:
            UI_FeederItemOwnerDraw_OnlineFriends(controllerIndex, contextIndex, item, rect, index, col, color);
            break;
        case 47:
            UI_FeederItemOwnerDraw_Invites(controllerIndex, contextIndex, item, rect, index, col, color);
            break;
        case 50:
            UI_FeederItemOwnerDraw_RecentPlayers(controllerIndex, contextIndex, item, rect, index, col, color);
            break;
        case 74:
            UI_FeederItemOwnerDraw_FileShareSearchResults(controllerIndex, contextIndex, item, rect, index, col, color);
            break;
        default:
            UI_Project_FeederItemOwnerDraw(localClientNum, contextIndex, feederID, rect, index);
            break;
    }
}

void __cdecl UI_FeederItemOwnerDraw_Invites(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                const rectDef_s *rect,
                int index,
                int col,
                const float *color)
{
    unsigned __int64 v7; // rax
    int LocalClientNum; // eax
    unsigned __int64 dwid; // [esp-8h] [ebp-D8h]
    FriendInfo info; // [esp+0h] [ebp-D0h] BYREF
    int i; // [esp+BCh] [ebp-14h]
    unsigned __int64 friendInviteId; // [esp+C0h] [ebp-10h]
    int friendsCount; // [esp+C8h] [ebp-8h]
    int invitesCount; // [esp+CCh] [ebp-4h]

    invitesCount = Live_GetInvitesCount();
    if ( index >= 0 && index < invitesCount )
    {
        LODWORD(v7) = Live_GetInviteFriend(index);
        friendInviteId = v7;
        friendsCount = Friends_GetCount(controllerIndex, 0);
        for ( i = 0; i < friendsCount; ++i )
        {
            Friends_GetByIndex(controllerIndex, 0, i, &info);
            if ( info.dwid == friendInviteId )
            {
                dwid = info.dwid;
                LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
                UI_DrawPlayerEmblemByXuid(LocalClientNum, contextIndex, item, rect, color, dwid);
            }
        }
    }
}

FriendInfo info_0;
void __cdecl UI_FeederItemOwnerDraw_OnlineFriends(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                const rectDef_s *rect,
                int index,
                int col,
                const float *color)
{
    int LocalClientNum; // eax
    unsigned __int64 dwid; // [esp-8h] [ebp-8h]

    if ( Friends_GetByIndex(controllerIndex, 0, index, &info_0) )
    {
        dwid = info_0.dwid;
        LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
        UI_DrawPlayerEmblemByXuid(LocalClientNum, contextIndex, item, rect, color, dwid);
    }
}

void __cdecl UI_FeederItemOwnerDraw_RecentPlayers(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                const rectDef_s *rect,
                int index,
                int col,
                const float *color)
{
    int LocalClientNum; // eax
    unsigned __int64 playerXuids; // [esp-8h] [ebp-70h]
    XuidInfo result; // [esp+8h] [ebp-60h] BYREF
    _BYTE v10[48]; // [esp+38h] [ebp-30h] BYREF

    memcpy(v10, LiveMeetPlayer_GetMetFriendInfo(&result, controllerIndex, index), sizeof(v10));
    memcpy(&recentPlayerInfo, v10, sizeof(recentPlayerInfo));
    playerXuids = recentPlayerInfo.playerXuids;
    LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
    UI_DrawPlayerEmblemByXuid(LocalClientNum, contextIndex, item, rect, color, playerXuids);
}

void __cdecl UI_FeederItemOwnerDraw_FileShareSearchResults(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                const rectDef_s *rect,
                int index,
                int col,
                const float *color)
{
    CL_DrawSpinner(
        &scrPlaceView[contextIndex],
        rect->x - 75.0,
        rect->y + 6.0,
        rect->w - 10.0,
        rect->h - 10.0,
        rect->horzAlign,
        rect->vertAlign,
        color);
}

void __cdecl UI_FeederItemColor(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                float feederID,
                int index,
                int column,
                DvarValue *color,
                bool onFocus)
{
    float v8; // xmm0_4
    bool v9; // [esp+3Dh] [ebp-47h]
    bool v10; // [esp+3Eh] [ebp-46h]
    int j; // [esp+40h] [ebp-44h]
    int localClientNum; // [esp+48h] [ebp-3Ch]
    listBoxDef_s *listPtr; // [esp+4Ch] [ebp-38h]
    uiInfo_s *dc; // [esp+70h] [ebp-14h]
    float newColor[4]; // [esp+74h] [ebp-10h] BYREF

    localClientNum = Com_LocalClient_GetControllerIndex(controllerIndex);
    dc = UI_UIContext_GetInfo(contextIndex);
    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp", 2360, 0, "%s", "listPtr") )
        __debugbreak();
    if ( feederID == 32.0 || feederID == 50.0 || feederID == 101.0 || feederID == 47.0 )
    {
        if ( column )
        {
            if ( column == 1 )
            {
                color->value = colorWhite[0];
                color->vector[1] = 1.0;
                color->vector[2] = 1.0;
                color->vector[3] = 1.0;
                return;
            }
LABEL_117:
            if ( (item->window.dynamicFlags[contextIndex] & 2) != 0 )
            {
                if ( UI_FeederItemEnabled(localClientNum, contextIndex, feederID, index) )
                {
                    if ( onFocus )
                    {
                        newColor[0] = color->value;
                        newColor[1] = color->vector[1];
                        newColor[2] = color->vector[2];
                        newColor[3] = color->vector[3];
                        Item_TextColor(&dc->uiDC, item, (float (*)[4])newColor);
                        color->value = newColor[0];
                        color->vector[1] = newColor[1];
                        color->vector[2] = newColor[2];
                        color->vector[3] = newColor[3];
                    }
                    else
                    {
                        color->value = item->window.foreColor[0];
                        color->vector[1] = item->window.foreColor[1];
                        color->vector[2] = item->window.foreColor[2];
                        color->vector[3] = item->window.foreColor[3];
                    }
                }
                else
                {
                    color->value = listPtr->disableColor[0];
                    color->vector[1] = listPtr->disableColor[1];
                    color->vector[2] = listPtr->disableColor[2];
                    color->vector[3] = listPtr->disableColor[3];
                }
            }
            else if ( onFocus && listPtr->special == 28.0 )
            {
                color->value = item->window.foreColor[0];
                color->vector[1] = item->window.foreColor[1];
                color->vector[2] = item->window.foreColor[2];
                color->vector[3] = item->window.foreColor[3];
            }
            else
            {
                color->value = listPtr->disableColor[0];
                color->vector[1] = listPtr->disableColor[1];
                color->vector[2] = listPtr->disableColor[2];
                color->vector[3] = listPtr->disableColor[3];
            }
            if ( feederID == 74.0 )
                Live_FileShareSearch_FeederColor(listPtr, item, contextIndex, index, column, color);
            return;
        }
        if ( onFocus )
        {
            color->vector[0] = newUIfeederHighlight[0];
            color->vector[1] = newUIfeederHighlight[1];
            color->vector[2] = newUIfeederHighlight[2];
            color->vector[3] = newUIfeederHighlight[3];
            return;
        }
    LABEL_100:
        color->vector[0] = newUIFeederBackgroundColor[0];
        color->vector[1] = newUIFeederBackgroundColor[1];
        color->vector[2] = newUIFeederBackgroundColor[2];
        color->vector[3] = newUIFeederBackgroundColor[3];
        return;
    }
    if ( feederID == 21.0 )
    {
        switch ( column )
        {
            case 2:
                v8 = color->vector[3] * 0.5;
                color->vector[3] = v8;
                break;
            case 3:
            case 4:
                color->integer = 0;
                color->vector[1] = 0.0f;
                color->vector[2] = 0.0f;
                color->vector[3] = 0.15000001f;
                break;
            case 7:
            case 8:
                if ( index >= Com_LocalClients_GetUsedControllerCount() )
                {
                    color->vector[0] = 0.69f;
                    color->vector[1] = 0.69f;
                    color->vector[2] = 0.69f;
                    color->vector[3] = 1.0f;
                }
                goto LABEL_117;
            default:
                goto LABEL_117;
        }
    }
    else
    {
        if ( feederID == 70.0 )
        {
            if ( column == 1 )
            {
                color->vector[3] = 0.6f;
                return;
            }
            goto LABEL_117;
        }
        if ( feederID == 73.0 )
        {
            if ( column == 1 )
            {
                color->vector[3] = 0.6f;
                return;
            }
            if ( column == 5 )
            {
                color->vector[0] = 0.15000001f;
                color->vector[1] = 0.61f;
                color->vector[2] = 0.0f;
                color->vector[3] = 0.8f;
                return;
            }
            goto LABEL_117;
        }
        if ( feederID == 54.0 )
        {
            if ( column )
            {
                if ( column > 0 && column <= 2 )
                {
                    if ( onFocus )
                    {
                        color->value = colorWhite[0];
                        color->vector[1] = 1.0;
                        color->vector[2] = 1.0;
                        color->vector[3] = 1.0;
                    }
                    else
                    {
                        color->value = colorBlackBlank[0];
                        color->vector[1] = 0.0;
                        color->vector[2] = 0.0;
                        color->vector[3] = 0.0;
                    }
                }
                else if ( listPtr->rows[index].status == 1 )
                {
                    static float newUIDisableColor[4] = { 0.40000001, 0.40000001, 0.40000001, 1.0 };
                    color->vector[0] = newUIDisableColor[0];
                    color->vector[1] = newUIDisableColor[1];
                    color->vector[2] = newUIDisableColor[2];
                    color->vector[3] = newUIDisableColor[3];
                }
                else
                {
                    if ( onFocus )
                    {
                        color->value = colorBlack[0];
                        color->vector[1] = 0.0;
                        color->vector[2] = 0.0;
                    }
                    else
                    {
                        color->value = colorWhite[0];
                        color->vector[1] = 1.0;
                        color->vector[2] = 1.0;
                    }
                    color->vector[3] = 1.0;
                }
            }
            else
            {
                if ( listPtr->backgroundItemListbox )
                {
                    for ( j = 3; j < listPtr->numColumns; ++j )
                    {
                        if ( listPtr->rows[index].cells[j].stringValue && *listPtr->rows[index].cells[j].stringValue )
                            goto LABEL_100;
                    }
                }
                color->value = colorBlackBlank[0];
                color->vector[1] = 0.0;
                color->vector[2] = 0.0;
                color->vector[3] = 0.0;
            }
        }
        else if ( feederID == 1.0 || feederID == 7.0 || feederID == 104.0 )
        {
            color->value = 1.0f;
            color->vector[1] = 1.0f;
            color->vector[2] = 1.0f;
            color->vector[3] = 1.0f;
            if ( column )
            {
                if ( column == 1 && listPtr->cursorPos[contextIndex] == index )
                {
                    color->integer = 0;
                    color->vector[1] = 0.0f;
                    color->vector[2] = 0.0f;
                    color->vector[3] = 1.0f;
                }
            }
            else if ( listPtr->cursorPos[contextIndex] == index )
            {
                color->value = 1.0f;
                color->vector[1] = 1.0f;
                color->vector[2] = 1.0f;
                color->vector[3] = 1.0f;
            }
            else
            {
                color->integer = 0;
                color->vector[1] = 0.0f;
                color->vector[2] = 0.0f;
                color->vector[3] = 0.2f;
            }
        }
        else if ( feederID == 48.0 )
        {
            color->value = 1.0f;
            color->vector[1] = 1.0f;
            color->vector[2] = 1.0f;
            color->vector[3] = 1.0f;
            v10 = listPtr->cursorPos[contextIndex] == index;
            if ( !Window_HasFocus(contextIndex, &item->parent->window) )
                v10 = 0;
            if ( column )
            {
                if ( column == 1 )
                {
                    if ( UI_FeederItemEnabled(localClientNum, contextIndex, feederID, index) )
                    {
                        if ( v10 && Window_HasFocus(contextIndex, &item->window) )
                        {
                            color->integer = 0;
                            color->vector[1] = 0.0f;
                            color->vector[2] = 0.0f;
                            color->vector[3] = 1.0f;
                        }
                        else if ( !v10 && !Window_HasFocus(contextIndex, &item->window) )
                        {
                            color->vector[0] = newUIGrey[0];
                            color->vector[1] = newUIGrey[1];
                            color->vector[2] = newUIGrey[2];
                            color->vector[3] = newUIGrey[3];
                        }
                    }
                    else
                    {
                        color->vector[0] = newUIGrey[0];
                        color->vector[1] = newUIGrey[1];
                        color->vector[2] = newUIGrey[2];
                        color->vector[3] = newUIGrey[3];
                    }
                }
            }
            else if ( Window_HasFocus(contextIndex, &item->window) )
            {
                if ( !v10 )
                {
                    color->vector[0] = newUIFeederBackgroundColor[0];
                    color->vector[1] = newUIFeederBackgroundColor[1];
                    color->vector[2] = newUIFeederBackgroundColor[2];
                    color->vector[3] = newUIFeederBackgroundColor[3];
                }
            }
            else if ( v10 )
            {
                color->vector[0] = newUIFeederBackgroundColor[0];
                color->vector[1] = newUIFeederBackgroundColor[1];
                color->vector[2] = newUIFeederBackgroundColor[2];
                color->vector[3] = newUIFeederBackgroundColor[3];
            }
            else
            {
                color->vector[3] = 0.0f;
            }
        }
        else if ( feederID == 76.0 || feederID == 77.0 )
        {
            color->value = 1.0f;
            color->vector[1] = 1.0f;
            color->vector[2] = 1.0f;
            color->vector[3] = 1.0f;
            v9 = listPtr->cursorPos[contextIndex] == index;
            if ( feederID != 76.0 || Window_HasFocus(contextIndex, &item->parent->window) )
            {
                if ( feederID == 77.0 && !Window_HasFocus(contextIndex, &item->window) )
                    v9 = 0;
            }
            else
            {
                v9 = 0;
            }
            if ( column )
            {
                if ( column == 1 )
                {
                    if ( Window_HasFocus(contextIndex, &item->window) )
                    {
                        if ( UI_FeederItemEnabled(localClientNum, contextIndex, feederID, index) )
                        {
                            if ( v9 )
                            {
                                color->integer = 0;
                                color->vector[1] = 0.0f;
                                color->vector[2] = 0.0f;
                                color->vector[3] = 1.0f;
                            }
                        }
                        else
                        {
                            color->vector[0] = newUIGrey[0];
                            color->vector[1] = newUIGrey[1];
                            color->vector[2] = newUIGrey[2];
                            color->vector[3] = newUIGrey[3];
                        }
                    }
                    else if ( !v9 )
                    {
                        color->vector[0] = 0;
                        color->vector[1] = 0.0f;
                        color->vector[2] = 0.0f;
                        color->vector[3] = 0.44999999f;
                    }
                }
            }
            else
            {
                if ( !Window_HasFocus(contextIndex, &item->window) )
                {
                    if ( !v9 )
                    {
                        color->vector[3] = 0.0f;
                        return;
                    }
                    goto LABEL_100;
                }
                if ( !v9 )
                {
                    color->vector[0] = newUIFeederBackgroundColor[0];
                    color->vector[1] = newUIFeederBackgroundColor[1];
                    color->vector[2] = newUIFeederBackgroundColor[2];
                    color->vector[3] = newUIFeederBackgroundColor[3];
                }
            }
        }
        else
        {
            if ( feederID == 80.0 )
            {
                Live_FileShare_FeederColor(listPtr, item, contextIndex, index, column, FILESHARE_BUFFER_PRIMARY, color);
                return;
            }
            if ( feederID == 79.0 )
            {
                Live_FileShare_FeederColor(listPtr, item, contextIndex, index, column, FILESHARE_BUFFER_SECONDARY, color);
                return;
            }
            if ( !UI_Project_FeederItemColor(
                            controllerIndex,
                            contextIndex,
                            item,
                            feederID,
                            index,
                            column,
                            &color->value,
                            onFocus) )
                goto LABEL_117;
        }
    }
}

void __cdecl UI_GetListDetails(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                char *feederName,
                itemDef_s **itemToFocus,
                int *lastIndex,
                int *viewMax)
{
    listBoxDef_s *listToFocus; // [esp+8h] [ebp-4h]

    *itemToFocus = Menu_GetMatchingItemByNumber(item->parent, 0, feederName);
    listToFocus = Item_GetListBoxDef(*itemToFocus);
    *lastIndex = (int)(UI_FeederCount(localClientNum, contextIndex, (float)(int)listToFocus->special, listToFocus) - 1);
    *viewMax = Item_ListBox_Viewmax(localClientNum, contextIndex, *itemToFocus);
}

bool __cdecl UI_OverrideKeyPress(int localClientNum, int contextIndex, itemDef_s *item, int key)
{
    uiInfo_s *v5; // eax
    uiInfo_s *v6; // eax
    uiInfo_s *v7; // eax
    uiInfo_s *v8; // eax
    uiInfo_s *v9; // eax
    uiInfo_s *v10; // eax
    uiInfo_s *v11; // eax
    uiInfo_s *v12; // eax
    uiInfo_s *v13; // eax
    uiInfo_s *v14; // eax
    int v15; // eax
    int v16; // [esp+10h] [ebp-194h]
    int v17; // [esp+14h] [ebp-190h]
    int v18; // [esp+18h] [ebp-18Ch]
    int v19; // [esp+1Ch] [ebp-188h]
    int v20; // [esp+20h] [ebp-184h]
    int v21; // [esp+24h] [ebp-180h]
    XuidInfo result; // [esp+2Ch] [ebp-178h] BYREF
    _BYTE v23[48]; // [esp+5Ch] [ebp-148h] BYREF
    int newCursorPos; // [esp+90h] [ebp-114h]
    int index; // [esp+94h] [ebp-110h]
    int newIndex; // [esp+98h] [ebp-10Ch]
    FriendInfo info; // [esp+9Ch] [ebp-108h] BYREF
    bool jumpToIndex; // [esp+15Bh] [ebp-49h]
    UiContext *dc; // [esp+15Ch] [ebp-48h]
    rectDef_s rect; // [esp+160h] [ebp-44h] BYREF
    int j; // [esp+178h] [ebp-2Ch]
    int controllerIndex; // [esp+17Ch] [ebp-28h]
    listBoxDef_s *listPtr; // [esp+180h] [ebp-24h]
    int lastIndex1; // [esp+184h] [ebp-20h] BYREF
    int currIndex; // [esp+188h] [ebp-1Ch]
    int viewMax; // [esp+18Ch] [ebp-18h] BYREF
    itemDef_s *itemToFocus; // [esp+190h] [ebp-14h] BYREF
    int viewMax1; // [esp+194h] [ebp-10h] BYREF
    itemDef_s *itemToFocus1; // [esp+198h] [ebp-Ch] BYREF
    int i; // [esp+19Ch] [ebp-8h]
    int lastIndex; // [esp+1A0h] [ebp-4h] BYREF

    itemToFocus = 0;
    itemToFocus1 = 0;
    listPtr = Item_GetListBoxDef(item);
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( !UI_FeederCount(localClientNum, contextIndex, (float)(int)listPtr->special, listPtr) )
        return 0;
    switch ( (int)listPtr->special )
    {
        case ' ':
            currIndex = listPtr->cursorPos[contextIndex];
            lastIndex = (int)(UI_FeederCount(localClientNum, contextIndex, (float)(int)listPtr->special, listPtr) - 1);
            viewMax = Item_ListBox_Viewmax(localClientNum, contextIndex, item);
            if ( key != 28 && key != 20 && key != 29 && key != 21 && key != 154 && key != 155 )
                return 0;
            newIndex = currIndex;
            if ( key == 28 || key == 20 || key == 154 )
            {
                if ( currIndex <= 0 )
                    v21 = lastIndex;
                else
                    v21 = currIndex - 1;
                newIndex = v21;
            }
            else
            {
                if ( currIndex >= lastIndex )
                    v20 = 0;
                else
                    v20 = currIndex + 1;
                newIndex = v20;
            }
            v15 = Com_LocalClient_GetControllerIndex(localClientNum);
            if ( Friends_GetByIndex(v15, 0, newIndex, &info) == 1 )
            {
                s_selectedPlayerXUID = info.dwid;
                s_selectedMetPlayerXUID = 0;
            }
            else
            {
                s_selectedPlayerXUID = 0;
            }
            Item_ListBox_SetCursorPos(localClientNum, contextIndex, item, lastIndex, viewMax, newIndex, 1);
            return 1;
        case '/':
            currIndex = listPtr->cursorPos[contextIndex];
            lastIndex = (int)(UI_FeederCount(localClientNum, contextIndex, (float)(int)listPtr->special, listPtr) - 1);
            viewMax = Item_ListBox_Viewmax(localClientNum, contextIndex, item);
            if ( key != 28 && key != 20 && key != 29 && key != 21 && key != 154 && key != 155 )
                return 0;
            newCursorPos = currIndex;
            if ( key == 28 || key == 20 || key == 154 )
            {
                if ( currIndex <= 0 )
                    v17 = lastIndex;
                else
                    v17 = currIndex - 1;
                newCursorPos = v17;
            }
            else
            {
                if ( currIndex >= lastIndex )
                    v16 = 0;
                else
                    v16 = currIndex + 1;
                newCursorPos = v16;
            }
            Item_ListBox_SetCursorPos(localClientNum, contextIndex, item, lastIndex, viewMax, newCursorPos, 1);
            return 1;
        case '2':
            currIndex = listPtr->cursorPos[contextIndex];
            lastIndex = (int)(UI_FeederCount(localClientNum, contextIndex, (float)(int)listPtr->special, listPtr) - 1);
            viewMax = Item_ListBox_Viewmax(localClientNum, contextIndex, item);
            if ( key != 28 && key != 20 && key != 29 && key != 21 && key != 154 && key != 155 )
                return 0;
            index = currIndex;
            if ( key == 28 || key == 20 || key == 154 )
            {
                if ( currIndex <= 0 )
                    v19 = lastIndex;
                else
                    v19 = currIndex - 1;
                index = v19;
            }
            else
            {
                if ( currIndex >= lastIndex )
                    v18 = 0;
                else
                    v18 = currIndex + 1;
                index = v18;
            }
            memcpy(v23, LiveMeetPlayer_GetMetFriendInfo(&result, controllerIndex, index), sizeof(v23));
            memcpy(&recentPlayerInfo, v23, sizeof(recentPlayerInfo));
            s_selectedMetPlayerXUID = recentPlayerInfo.playerXuids;
            Item_ListBox_SetCursorPos(localClientNum, contextIndex, item, lastIndex, viewMax, index, 1);
            return 1;
        case '6':
            jumpToIndex = 0;
            i = 0;
            currIndex = listPtr->cursorPos[contextIndex];
            lastIndex = (int)(UI_FeederCount(localClientNum, contextIndex, 54.0, listPtr) - 1);
            viewMax = Item_ListBox_Viewmax(localClientNum, contextIndex, item);
            if ( key == 21 || key == 29 || key == 155 || key == 189 )
            {
                if ( currIndex == lastIndex )
                {
                    Item_ListBox_SetCursorPos(localClientNum, contextIndex, item, lastIndex + 1, viewMax, 0, 1);
                    return 1;
                }
                for ( i = currIndex + 1; ; ++i )
                {
                    if ( i > lastIndex )
                        goto LABEL_207;
                    for ( j = 3; ; ++j )
                    {
                        if ( j >= listPtr->numColumns )
                            goto LABEL_188;
                        if ( listPtr->rows[i].cells[j].stringValue && *listPtr->rows[i].cells[j].stringValue )
                            break;
                    }
                    jumpToIndex = 1;
LABEL_188:
                    if ( jumpToIndex )
                        goto LABEL_207;
                }
            }
            if ( key != 20 && key != 28 && key != 154 && key != 183 )
                goto LABEL_207;
            if ( !currIndex )
            {
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, item, lastIndex + 1, viewMax, lastIndex, 1);
                return 1;
            }
            i = currIndex - 1;
            break;
        case 'J':
            dc = (UiContext *)UI_UIContext_GetInfo(contextIndex);
            rect = *Window_GetRect(&item->window);
            rect.w = rect.w - 14.0;
            return (key == 13 || key == 200 || key == 1)
                    && Rect_ContainsPoint(dc->contextIndex, &rect, dc->cursor.x, dc->cursor.y)
                    && !Live_FileShareSearch_FeederItemEnabled(listPtr->cursorPos[contextIndex]);
        case 'O':
            if ( key == 31 || key == 23 )
            {
                if ( fsOtherUserPrivateSlotCol->current.integer == 2 )
                    Dvar_SetInt((dvar_s *)fsOtherUserPrivateSlotCol, 0);
                else
                    Dvar_SetInt((dvar_s *)fsOtherUserPrivateSlotCol, fsOtherUserPrivateSlotCol->current.integer + 1);
            }
            else if ( key == 30 || key == 22 )
            {
                if ( fsOtherUserPrivateSlotCol->current.integer )
                    Dvar_SetInt((dvar_s *)fsOtherUserPrivateSlotCol, fsOtherUserPrivateSlotCol->current.integer - 1);
                else
                    Dvar_SetInt((dvar_s *)fsOtherUserPrivateSlotCol, 2);
            }
            return 0;
        case 'P':
            if ( key == 31 || key == 23 || key == 157 )
            {
                if ( fsPrivateSlotCol->current.integer == 2 )
                    Dvar_SetInt((dvar_s *)fsPrivateSlotCol, 0);
                else
                    Dvar_SetInt((dvar_s *)fsPrivateSlotCol, fsPrivateSlotCol->current.integer + 1);
            }
            else if ( key == 30 || key == 22 || key == 156 )
            {
                if ( fsPrivateSlotCol->current.integer )
                    Dvar_SetInt((dvar_s *)fsPrivateSlotCol, fsPrivateSlotCol->current.integer - 1);
                else
                    Dvar_SetInt((dvar_s *)fsPrivateSlotCol, 2);
            }
            Com_LocalClient_GetControllerIndex(localClientNum);
            if ( !CG_IsShowingZombieMap() )
            {
                currIndex = listPtr->cursorPos[contextIndex];
                if ( currIndex == UI_FeederCount(localClientNum, contextIndex, (float)(int)listPtr->special, listPtr) - 1 && (key == 29 || key == 21) )
                {
                    listPtr->cursorPos[contextIndex] = 0;
                    v13 = UI_UIContext_GetInfo(contextIndex);
                    Menus_SetFocusToItem(localClientNum, &v13->uiDC, "menu_fileshare_myshare", "new_frame_button_1");
                }
                if ( !currIndex && (key == 28 || key == 20) )
                {
                    listPtr->cursorPos[contextIndex] = 0;
                    v14 = UI_UIContext_GetInfo(contextIndex);
                    Menus_SetFocusToItem(localClientNum, &v14->uiDC, "menu_fileshare_myshare", "new_frame_button_1");
                }
            }
            return 0;
        case 'Z':
            UI_GetListDetails(localClientNum, contextIndex, item, (char*)"clan_tag_list_edit", &itemToFocus, &lastIndex, &viewMax);
            UI_GetListDetails(localClientNum, contextIndex, item, (char*)"clan_tag_list", &itemToFocus1, &lastIndex1, &viewMax1);
            if ( lastIndex == -1 || lastIndex1 == -1 )
                return 0;
            if ( listPtr->cursorPos[contextIndex] == listPtr->endPos[contextIndex] - 1
                && (key == 29 || key == 21 || key == 155) )
            {
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus1, lastIndex1, viewMax1, 0, 1);
                v11 = UI_UIContext_GetInfo(contextIndex);
                Menus_SetFocusToItem(localClientNum, &v11->uiDC, "menu_playercards_clantag", "clan_tag_list_edit");
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus, lastIndex, viewMax, 0, 1);
                return 1;
            }
            if ( !listPtr->cursorPos[contextIndex] && (key == 28 || key == 20 || key == 154) )
            {
                v12 = UI_UIContext_GetInfo(contextIndex);
                Menus_SetFocusToItem(localClientNum, &v12->uiDC, "menu_playercards_clantag", "clan_tag_list_edit");
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus, lastIndex, viewMax, lastIndex, 1);
                return 1;
            }
            if ( key == 2 )
            {
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus1, lastIndex1, viewMax1, 0, 1);
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus, lastIndex, viewMax, 0, 1);
            }
            return 0;
        case '\\':
            UI_GetListDetails(
                localClientNum,
                contextIndex,
                item,
                (char *)"stats_milestones_weapongrouplist",
                &itemToFocus,
                &lastIndex,
                &viewMax);
            UI_GetListDetails(
                localClientNum,
                contextIndex,
                item,
                (char *)"stats_milestones_list",
                &itemToFocus1,
                &lastIndex1,
                &viewMax1);
            if ( lastIndex == -1 || lastIndex1 == -1 )
                return 0;
            if ( listPtr->cursorPos[contextIndex] == listPtr->endPos[contextIndex] - 1 && (key == 29 || key == 21) )
            {
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus1, lastIndex1, viewMax1, 0, 1);
                v7 = UI_UIContext_GetInfo(contextIndex);
                Menus_SetFocusToItem(
                    localClientNum,
                    &v7->uiDC,
                    "menu_stats_milestones_weapons",
                    "stats_milestones_weapongrouplist");
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus, lastIndex, viewMax, 0, 1);
                return 1;
            }
            if ( !listPtr->cursorPos[contextIndex] && (key == 28 || key == 20) )
            {
                v8 = UI_UIContext_GetInfo(contextIndex);
                Menus_SetFocusToItem(
                    localClientNum,
                    &v8->uiDC,
                    "menu_stats_milestones_weapons",
                    "stats_milestones_weapongrouplist");
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus, lastIndex, viewMax, lastIndex, 1);
                return 1;
            }
            if ( key == 2 )
            {
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus1, lastIndex1, viewMax1, 0, 1);
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus, lastIndex, viewMax, 0, 1);
            }
            return 0;
        case '^':
            UI_GetListDetails(localClientNum, contextIndex, item, (char *)"stats_milestones_list", &itemToFocus, &lastIndex, &viewMax);
            if ( lastIndex == -1 )
                return 0;
            if ( listPtr->cursorPos[contextIndex] == listPtr->endPos[contextIndex] - 1 && (key == 29 || key == 21) )
            {
                v5 = UI_UIContext_GetInfo(contextIndex);
                Menus_SetFocusToItem(localClientNum, &v5->uiDC, "menu_stats_milestones_weapons", "stats_milestones_list");
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus, lastIndex, viewMax, 0, 1);
                return 1;
            }
            if ( !listPtr->cursorPos[contextIndex] && (key == 28 || key == 20) )
            {
                v6 = UI_UIContext_GetInfo(contextIndex);
                Menus_SetFocusToItem(localClientNum, &v6->uiDC, "menu_stats_milestones_weapons", "stats_milestones_list");
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus, lastIndex, viewMax, lastIndex, 1);
                return 1;
            }
            if ( key == 2 )
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus, lastIndex, viewMax, 0, 1);
            return 0;
        case '_':
            UI_GetListDetails(
                localClientNum,
                contextIndex,
                item,
                (char *)"global_challenges_list",
                &itemToFocus,
                &lastIndex,
                &viewMax);
            if ( key == 2 )
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus, lastIndex, viewMax, 0, 1);
            return 0;
        case '`':
            UI_GetListDetails(
                localClientNum,
                contextIndex,
                item,
                (char *)"stats_milestones_attachments",
                &itemToFocus,
                &lastIndex,
                &viewMax);
            if ( key == 2 )
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus, lastIndex, viewMax, 0, 1);
            return 0;
        case 'a':
            UI_GetListDetails(localClientNum, contextIndex, item, (char *)"perks_type_list", &itemToFocus, &lastIndex, &viewMax);
            if ( key == 2 )
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus, lastIndex, viewMax, 0, 1);
            return 0;
        case 'b':
            UI_GetListDetails(
                localClientNum,
                contextIndex,
                item,
                (char *)"killstreak_challenges_list",
                &itemToFocus,
                &lastIndex,
                &viewMax);
            if ( key == 2 )
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus, lastIndex, viewMax, 0, 1);
            return 0;
        case 'c':
            UI_GetListDetails(
                localClientNum,
                contextIndex,
                item,
                (char *)"stats_milestones_grenades",
                &itemToFocus,
                &lastIndex,
                &viewMax);
            if ( key == 2 )
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus, lastIndex, viewMax, 0, 1);
            return 0;
        case 'd':
            UI_GetListDetails(localClientNum, contextIndex, item, (char *)"killstreaks", &itemToFocus, &lastIndex, &viewMax);
            if ( key == 2 )
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus, lastIndex, viewMax, 0, 1);
            return 0;
        case 'f':
            UI_GetListDetails(localClientNum, contextIndex, item, (char *)"clan_tag_list", &itemToFocus, &lastIndex, &viewMax);
            if ( lastIndex == -1 )
                return 0;
            if ( key == 29 || key == 21 || key == 155 )
            {
                v9 = UI_UIContext_GetInfo(contextIndex);
                Menus_SetFocusToItem(localClientNum, &v9->uiDC, "menu_playercards_clantag", "clan_tag_list");
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus, lastIndex, viewMax, 0, 1);
                return 1;
            }
            else if ( key == 28 || key == 20 || key == 154 )
            {
                v10 = UI_UIContext_GetInfo(contextIndex);
                Menus_SetFocusToItem(localClientNum, &v10->uiDC, "menu_playercards_clantag", "clan_tag_list");
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus, lastIndex, viewMax, lastIndex, 1);
                return 1;
            }
            else
            {
                if ( key == 2 )
                    Item_ListBox_SetCursorPos(localClientNum, contextIndex, itemToFocus, lastIndex, viewMax, 0, 1);
                return 0;
            }
        default:
            return 0;
    }
    while ( i >= 0 )
    {
        for ( j = 3; j < listPtr->numColumns; ++j )
        {
            if ( listPtr->rows[i].cells[j].stringValue && I_stricmp(listPtr->rows[i].cells[j].stringValue, "") )
            {
                jumpToIndex = 1;
                break;
            }
        }
        if ( jumpToIndex )
            break;
        --i;
    }
LABEL_207:
    if ( !jumpToIndex )
        return 0;
    Item_ListBox_SetCursorPos(localClientNum, contextIndex, item, lastIndex + 1, viewMax, i, 1);
    return 1;
}

void __cdecl UI_OverrideCursorPos(int localClientNum, int contextIndex, itemDef_s *item)
{
    int controllerIndex; // [esp+184h] [ebp-Ch]
    float feederID; // [esp+188h] [ebp-8h]
    listBoxDef_s *listPtr; // [esp+18Ch] [ebp-4h]

    listPtr = Item_GetListBoxDef(item);
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    feederID = listPtr->special;
    switch ( (int)feederID )
    {
        case 1:
            UI_OverrideCursorPos_Maps(contextIndex, listPtr);
            break;
        case 2:
            UI_OverrideCursorPos_Servers(localClientNum, contextIndex, item, listPtr);
            break;
        case 4:
            listPtr->cursorPos[contextIndex] = UI_GetListIndexFromMapIndex(ui_currentNetMap->current.integer);
            break;
        case 26:
        case 77:
            UI_OverrideCursorPos_Playlists(contextIndex, listPtr);
            break;
        case 27:
            UI_OverrideCursorPos_Gamemodes(contextIndex, listPtr);
            break;
        case 28:
        case 76:
            UI_OverrideCursorPos_Categories(contextIndex, listPtr);
            break;
        case 29:
            UI_OverrideCursorPos_Leaderboards(localClientNum, contextIndex, item, listPtr);
            break;
        case 32:
            UI_OverrideCursorPos_OnlineFriends(controllerIndex, contextIndex, item, listPtr);
            break;
        case 38:
            UI_OverrideCursorPos_CustomGametypes(contextIndex, listPtr);
            break;
        case 45:
        case 46:
        case 70:
            return;
        case 47:
            UI_OverrideCursorPos_Invites(contextIndex, listPtr);
            break;
        case 50:
            UI_OverrideCursorPos_RecentPlayers(controllerIndex, contextIndex, item, listPtr);
            break;
        case 54:
            UI_OverrideCursorPos_DynamicMenu(contextIndex, listPtr);
            break;
        case 74:
            UI_OverrideCursorPos_FileShareSearchResults(localClientNum, contextIndex, listPtr);
            break;
        case 80:
            UI_OverrideCursorPos_FileShareMySlots(localClientNum, contextIndex, listPtr);
            break;
        default:
            if ( !UI_Project_OverrideCursorPos(localClientNum, contextIndex, item) )
                UI_OverrideCursorPos_Default(localClientNum, contextIndex, feederID, listPtr);
            break;
    }
}

void __cdecl UI_OverrideCursorPos_RecentPlayers(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                listBoxDef_s *listPtr)
{
    XuidInfo result; // [esp+8h] [ebp-A0h] BYREF
    _BYTE v5[48]; // [esp+38h] [ebp-70h] BYREF
    XuidInfo info; // [esp+68h] [ebp-40h] BYREF
    int currentIndex; // [esp+A0h] [ebp-8h]
    int count; // [esp+A4h] [ebp-4h]

    if ( controllerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp",
                    2788,
                    0,
                    "%s",
                    "controllerIndex != INVALID_CONTROLLER_PORT") )
    {
        __debugbreak();
    }
    count = Friends_GetCount(controllerIndex, 1);
    if ( listPtr->cursorPos[contextIndex] >= count || listPtr->cursorPos[contextIndex] < 0 )
    {
        listPtr->cursorPos[contextIndex] = 0;
        listPtr->startPos[contextIndex] = 0;
    }
    currentIndex = listPtr->cursorPos[contextIndex];
    memcpy(v5, LiveMeetPlayer_GetMetFriendInfo(&result, controllerIndex, currentIndex), sizeof(v5));
    memcpy(&info, v5, sizeof(info));
    s_selectedMetPlayerXUID = info.playerXuids;
    s_selectedPlayerXUID = 0;
}

void __cdecl UI_OverrideCursorPos_OnlineFriends(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                listBoxDef_s *listPtr)
{
    FriendInfo info; // [esp+0h] [ebp-C8h] BYREF
    int currentIndex; // [esp+C0h] [ebp-8h]
    int count; // [esp+C4h] [ebp-4h]

    if ( controllerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp",
                    2807,
                    0,
                    "%s",
                    "controllerIndex != INVALID_CONTROLLER_PORT") )
    {
        __debugbreak();
    }
    count = Friends_GetCount(controllerIndex, 0);
    if ( listPtr->cursorPos[contextIndex] >= count || listPtr->cursorPos[contextIndex] < 0 )
    {
        listPtr->cursorPos[contextIndex] = 0;
        listPtr->startPos[contextIndex] = 0;
    }
    currentIndex = listPtr->cursorPos[contextIndex];
    if ( Friends_GetByIndex(controllerIndex, 0, currentIndex, &info) == 1 )
    {
        s_selectedPlayerXUID = info.dwid;
        s_selectedMetPlayerXUID = 0;
    }
    else
    {
        s_selectedPlayerXUID = 0;
    }
}

void __cdecl UI_OverrideCursorPos_FileShareSearchResults(int localClientNum, int contextIndex, listBoxDef_s *listPtr)
{
    int numResults; // [esp+0h] [ebp-4h]

    Com_LocalClient_GetControllerIndex(localClientNum);
    numResults = Live_FileShare_GetSearchResultsCount();
    if ( numResults )
    {
        if ( listPtr->cursorPos[contextIndex] < numResults )
        {
            if ( listPtr->cursorPos[contextIndex] < 0 )
                listPtr->cursorPos[contextIndex] = 0;
        }
        else
        {
            listPtr->cursorPos[contextIndex] = numResults - 1;
        }
    }
    else
    {
        listPtr->cursorPos[contextIndex] = 0;
        listPtr->startPos[contextIndex] = 0;
        listPtr->endPos[contextIndex] = 0;
    }
}

void __cdecl UI_OverrideCursorPos_FileShareMySlots(int localClientNum, int contextIndex, listBoxDef_s *listPtr)
{
    int numResults; // [esp+8h] [ebp-4h]

    Com_LocalClient_GetControllerIndex(localClientNum);
    if ( !CG_IsShowingZombieMap() )
    {
        if ( fshSelectLastSlotRow && fshSelectLastSlotRow->current.integer == 1 )
        {
            listPtr->cursorPos[contextIndex] = UI_FeederCount(
                                                                                     localClientNum,
                                                                                     contextIndex,
                                                                                     (float)(int)listPtr->special,
                                                                                     listPtr)
                                                                             - 1;
            Dvar_SetInt((dvar_s *)fshSelectLastSlotRow, 0);
        }
        else if ( fshSelectFirstSlotRow && fshSelectFirstSlotRow->current.integer == 1 )
        {
            listPtr->cursorPos[contextIndex] = 0;
            Dvar_SetInt((dvar_s *)fshSelectFirstSlotRow, 0);
        }
    }
    numResults = UI_FeederCount(localClientNum, contextIndex, (float)(int)listPtr->special, listPtr);
    if ( listPtr->cursorPos[contextIndex] >= numResults )
        listPtr->cursorPos[contextIndex] = numResults - 1;
}

void __cdecl UI_OverrideCursorPos_Invites(int contextIndex, listBoxDef_s *listPtr)
{
    int inviteIndex; // [esp+0h] [ebp-8h]
    int numInvites; // [esp+4h] [ebp-4h]

    numInvites = Live_GetInvitesCount();
    if ( numInvites )
    {
        if ( listPtr->cursorPos[contextIndex] >= numInvites )
            listPtr->cursorPos[contextIndex] = numInvites - 1;
    }
    else
    {
        listPtr->cursorPos[contextIndex] = 0;
    }
    inviteIndex = ui_inviteSelectedInd->current.integer;
    if ( inviteIndex <= 0 )
        listPtr->cursorPos[contextIndex] = 0;
    else
        listPtr->cursorPos[contextIndex] = inviteIndex;
}

void __cdecl UI_OverrideCursorPos_Playlists(int contextIndex, listBoxDef_s *listPtr)
{
    int CategoryIdForNum; // eax
    int v3; // eax
    int playlistNum; // [esp+0h] [ebp-8h]
    int playlistCount; // [esp+4h] [ebp-4h]

    if ( xblive_wagermatch->current.enabled )
    {
        CategoryIdForNum = Playlist_GetCategoryIdForNum(wagerCategory->current.integer);
        playlistCount = Playlist_GetPlaylistCount(CategoryIdForNum);
        playlistNum = wagerCategoryPlaylist->current.integer;
    }
    else
    {
        v3 = Playlist_GetCategoryIdForNum(category->current.integer);
        playlistCount = Playlist_GetPlaylistCount(v3);
        playlistNum = categoryPlaylist->current.integer;
    }
    if ( playlistCount < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp", 3041, 0, "%s", "playlistCount >= 0") )
    {
        __debugbreak();
    }
    if ( playlistCount && listPtr->cursorPos[contextIndex] >= 0 )
        listPtr->cursorPos[contextIndex] = playlistNum;
    else
        listPtr->cursorPos[contextIndex] = 0;
    if ( listPtr->cursorPos[contextIndex] >= playlistCount )
        listPtr->cursorPos[contextIndex] = playlistCount - 1;
}

void __cdecl UI_OverrideCursorPos_Categories(int contextIndex, listBoxDef_s *listPtr)
{
    int integer; // edx
    int categoryCount; // [esp+0h] [ebp-4h]

    categoryCount = Playlist_GetCategoryCount();
    if ( categoryCount < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp", 3063, 0, "%s", "categoryCount >= 0") )
    {
        __debugbreak();
    }
    if ( categoryCount )
    {
        if ( xblive_wagermatch->current.enabled )
            integer = wagerCategory->current.integer;
        else
            integer = category->current.integer;
        listPtr->cursorPos[contextIndex] = integer;
    }
    else
    {
        listPtr->cursorPos[contextIndex] = 0;
    }
    if ( listPtr->cursorPos[contextIndex] >= categoryCount )
        listPtr->cursorPos[contextIndex] = categoryCount - 1;
}

void __cdecl UI_OverrideCursorPos_Gamemodes(int contextIndex, listBoxDef_s *listPtr)
{
    if ( Playlist_GetGametypeCount() < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp",
                    3089,
                    0,
                    "%s",
                    "Playlist_GetGametypeCount() >= 0") )
    {
        __debugbreak();
    }
    if ( Playlist_GetGametypeCount() )
    {
        if ( listPtr->cursorPos[contextIndex] >= Playlist_GetGametypeCount() )
            listPtr->cursorPos[contextIndex] = Playlist_GetGametypeCount() - 1;
    }
    else
    {
        listPtr->cursorPos[contextIndex] = 0;
    }
}

void __cdecl UI_OverrideCursorPos_Servers(int localClientNum, int contextIndex, itemDef_s *item, listBoxDef_s *listPtr)
{
    int maxScroll; // [esp+4h] [ebp-4h]

    if ( listPtr->endPos[contextIndex] )
    {
        if (sharedUiInfo.serverStatus.currentServer >= 0)
        {
            maxScroll = (int)Item_ListBox_MaxScroll(localClientNum, contextIndex, item);
            if ( listPtr->startPos[contextIndex] > maxScroll )
                listPtr->startPos[contextIndex] = maxScroll;
            if ( listPtr->cursorPos[contextIndex] >= listPtr->startPos[contextIndex]
                && listPtr->cursorPos[contextIndex] <= listPtr->endPos[contextIndex] )
            {
                sharedUiInfo.serverStatus.currentServer = listPtr->cursorPos[contextIndex];
            }
        }
    }
    else
    {
        listPtr->cursorPos[contextIndex] = 0;
    }
}

void __cdecl UI_OverrideCursorPos_Leaderboards(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                listBoxDef_s *listPtr)
{
    int viewmax; // [esp+0h] [ebp-4h]

    viewmax = Item_ListBox_Viewmax(localClientNum, contextIndex, item);
    LB_OverrideCursorPos(viewmax, &listPtr->cursorPos[contextIndex], &listPtr->startPos[contextIndex]);
}

void __cdecl UI_OverrideCursorPos_DynamicMenu(int contextIndex, listBoxDef_s *listPtr)
{
    int j; // [esp+0h] [ebp-4h]

    if ( listPtr->cursorPos[contextIndex] < listPtr->rowCount && listPtr->cursorPos[contextIndex] >= 0 )
    {
        for ( j = 3; j < listPtr->numColumns; ++j )
        {
            if ( listPtr->rows[listPtr->cursorPos[contextIndex]].cells[j].stringValue
                && *listPtr->rows[listPtr->cursorPos[contextIndex]].cells[j].stringValue )
            {
                return;
            }
        }
        listPtr->cursorPos[contextIndex] = 0;
    }
    else
    {
        listPtr->cursorPos[contextIndex] = 0;
    }
}

void __cdecl UI_OverrideCursorPos_Default(int localClientNum, int contextIndex, float feederID, listBoxDef_s *listPtr)
{
    int itemCount; // [esp+8h] [ebp-4h]

    itemCount = UI_FeederCount(localClientNum, contextIndex, feederID, listPtr);
    if ( itemCount < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp", 3586, 0, "%s", "itemCount >= 0") )
    {
        __debugbreak();
    }
    if ( listPtr->cursorPos[contextIndex] >= itemCount || listPtr->cursorPos[contextIndex] < 0 )
    {
        listPtr->cursorPos[contextIndex] = 0;
        listPtr->startPos[contextIndex] = 0;
    }
}

void __cdecl UI_FeederSelection(int localClientNum, int contextIndex, float feederID, int index)
{
    int ControllerIndex; // eax
    char *GametypeInternalName; // eax
    char *GametypeLocalizedName; // eax
    XuidInfo result; // [esp+28h] [ebp-6Ch] BYREF
    _BYTE v8[48]; // [esp+58h] [ebp-3Ch] BYREF
    uiInfo_s *uiInfo; // [esp+90h] [ebp-4h]

    if ( index < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp", 3920, 0, "%s", "index >= 0") )
    {
        __debugbreak();
    }
    uiInfo = UI_UIContext_GetInfo(contextIndex);
    switch ( (int)feederID )
    {
        case 1:
            UI_FeederSelection_Maps(contextIndex, feederID, index);
            break;
        case 2:
            UI_FeederSelection_Servers(contextIndex, feederID, index);
            break;
        case 4:
            UI_FeederSelection_AllMaps(contextIndex, feederID, index);
            break;
        case 7:
            uiInfo->playerIndex = index;
            break;
        case 9:
            sharedUiInfo.modCount = index;
            break;
        case 19:
            uiInfo->playerIndex = index;
            break;
        case 26:
        case 77:
            UI_FeederSelection_Playlists(localClientNum, contextIndex, feederID, index);
            break;
        case 27:
            if ( index >= 0 && index < Playlist_GetGametypeCount() )
            {
                GametypeInternalName = (char *)Playlist_GetGametypeInternalName(index);
                Dvar_SetStringByName("ui_gametype", GametypeInternalName);
                GametypeLocalizedName = (char *)Playlist_GetGametypeLocalizedName(index);
                Dvar_SetStringByName("ui_gametype_text", GametypeLocalizedName);
            }
            break;
        case 28:
        case 76:
            UI_FeederSelection_Categories(contextIndex, feederID, index);
            break;
        case 29:
            LB_FeederSelection(index);
            break;
        case 32:
        case 45:
        case 46:
        case 70:
            return;
        case 38:
            UI_FeederSelection_CustomGametypes(contextIndex, feederID, index);
            break;
        case 47:
            UI_FeederSelection_Invites(contextIndex, feederID, index);
            break;
        case 50:
            ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
            memcpy(v8, LiveMeetPlayer_GetMetFriendInfo(&result, ControllerIndex, index), sizeof(v8));
            memcpy(&recentPlayerInfo, v8, sizeof(recentPlayerInfo));
            break;
        case 54:
            Dvar_SetInt((dvar_s *)selectedMenuItemIndex, index);
            break;
        case 60:
            sharedUiInfo.sortedItemPivot = index;
            break;
        case 90:
            sharedUiInfo.clanTagFeature = index + 1;
            break;
        default:
            UI_Project_FeederSelection(localClientNum, contextIndex, feederID, index);
            break;
    }
}

void __cdecl UI_FeederSelection_AllMaps(int contextIndex, float feederID, int index)
{
    int actual; // [esp+8h] [ebp-4h] BYREF

    UI_SelectedMap(index, &actual);
    if ( feederID == 4.0 )
        Dvar_SetInt((dvar_s *)ui_currentMap, actual);
    Dvar_SetInt((dvar_s *)ui_currentNetMap, actual);
}

char info_1[1024];
void __cdecl UI_FeederSelection_Servers(int contextIndex, float feederID, int index)
{
    char *v4; // eax
    char *pos; // [esp+0h] [ebp-8h]

    if (sharedUiInfo.serverStatus.numDisplayServers > 0)
        sharedUiInfo.serverStatus.currentServer = index;
    LAN_GetServerInfo(
        ui_netSource->current.integer,
        *(_DWORD *)&sharedUiInfo.serverStatusAddress[4 * index - 81328],
        info_1,
        1024);
    v4 = Info_ValueForKey(info_1, "mapname");
    for (pos = va("levelshots/%s", v4); *pos; ++pos)
        *pos = tolower(*pos);
}

void __cdecl UI_FeederSelection_Invites(int contextIndex, float feederID, int index)
{
    if ( index < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp", 3820, 0, "%s", "index >= 0") )
    {
        __debugbreak();
    }
    if ( index < Live_GetInvitesCount() )
        Dvar_SetInt((dvar_s *)ui_inviteSelectedInd, index);
    else
        Dvar_SetInt((dvar_s *)ui_inviteSelectedInd, 0);
}

void __cdecl UI_FeederSelection_Playlists(int localClientNum, int contextIndex, float feederID, int index)
{
    int CategoryIdForNum; // eax
    int PlaylistNumFromId; // eax
    int v6; // eax
    int controllerIndex; // [esp+4h] [ebp-8h]
    int playlistId; // [esp+8h] [ebp-4h]
    int playlistIda; // [esp+8h] [ebp-4h]

    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( controllerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp",
                    3847,
                    0,
                    "%s",
                    "controllerIndex != INVALID_CONTROLLER_PORT") )
    {
        __debugbreak();
    }
    if ( index < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp",
                    3848,
                    0,
                    "%s\n\t(index) = %i",
                    "(index >= 0)",
                    index) )
    {
        __debugbreak();
    }
    if ( xblive_wagermatch->current.enabled )
    {
        CategoryIdForNum = Playlist_GetCategoryIdForNum(wagerCategory->current.integer);
        if ( index < Playlist_GetPlaylistCount(CategoryIdForNum) )
            Dvar_SetInt((dvar_s *)wagerCategoryPlaylist, index);
        else
            Dvar_SetInt((dvar_s *)wagerCategoryPlaylist, 0);
        Playlist_SetPrevPlaylist();
        playlistId = Playlist_GetPlaylistIdForNum(
                                     controllerIndex,
                                     wagerCategoryPlaylist->current.unsignedInt,
                                     wagerCategory->current.integer);
        PlaylistNumFromId = Playlist_GetPlaylistNumFromId(controllerIndex, playlistId);
    }
    else
    {
        v6 = Playlist_GetCategoryIdForNum(category->current.integer);
        if ( index < Playlist_GetPlaylistCount(v6) )
            Dvar_SetInt((dvar_s *)categoryPlaylist, index);
        else
            Dvar_SetInt((dvar_s *)categoryPlaylist, 0);
        Playlist_SetPrevPlaylist();
        playlistIda = Playlist_GetPlaylistIdForNum(
                                        controllerIndex,
                                        categoryPlaylist->current.unsignedInt,
                                        category->current.integer);
        PlaylistNumFromId = Playlist_GetPlaylistNumFromId(controllerIndex, playlistIda);
    }
    Dvar_SetInt((dvar_s *)playlist, PlaylistNumFromId);
}

void __cdecl UI_FeederSelection_Categories(int contextIndex, float feederID, int index)
{
    if ( index < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp",
                    3886,
                    0,
                    "%s\n\t(index) = %i",
                    "(index >= 0)",
                    index) )
    {
        __debugbreak();
    }
    if ( xblive_wagermatch->current.enabled )
    {
        if ( index < Playlist_GetCategoryCount() )
            Dvar_SetInt((dvar_s *)wagerCategory, index);
        else
            Dvar_SetInt((dvar_s *)wagerCategory, 0);
    }
    else if ( index < Playlist_GetCategoryCount() )
    {
        Dvar_SetInt((dvar_s *)category, index);
    }
    else
    {
        Dvar_SetInt((dvar_s *)category, 0);
    }
}

LBSelectedPlayerInfo info_2;
char __cdecl UI_FeederDataExtended(
                int localClientNum,
                itemDef_s *item,
                const char *field,
                char **stringResult,
                float *floatResult)
{
    char result; // al
    int v6; // eax
    char v7; // [esp+6h] [ebp-1Ah]
    char v8; // [esp+7h] [ebp-19h]
    int controllerIndex; // [esp+Ch] [ebp-14h]
    float feederID; // [esp+10h] [ebp-10h]
    int index; // [esp+14h] [ebp-Ch]
    int contextIndex; // [esp+18h] [ebp-8h]
    listBoxDef_s *listBox; // [esp+1Ch] [ebp-4h]

    listBox = Item_GetListBoxDef(item);
    if ( !listBox )
        return 0;
    contextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
    if ( contextIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp",
                    4254,
                    0,
                    "%s",
                    "contextIndex != INVALID_UI_CONTEXT") )
    {
        __debugbreak();
    }
    index = listBox->cursorPos[contextIndex];
    feederID = listBox->special;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( controllerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp",
                    4261,
                    0,
                    "%s",
                    "controllerIndex != INVALID_CONTROLLER_PORT") )
    {
        __debugbreak();
    }
    switch ( (int)feederID )
    {
        case 29:
            v6 = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
            LB_GetSelectedPlayerInfo(v6, &info_2);
            if ( I_stricmp(field, "xuid") )
            {
                if ( I_stricmp(field, "name") )
                {
                    v7 = 0;
                }
                else
                {
                    *stringResult = info_2.gamerTag;
                    v7 = 1;
                }
            }
            else
            {
                *stringResult = va("%lld", info_2.playerXuid);
                v7 = 1;
            }
            result = v7;
            break;
        case 32:
            result = UI_FeederDataExtended_OnlineFriends(controllerIndex, index, field, stringResult);
            break;
        case 50:
            result = UI_FeederDataExtended_RecentPlayers(controllerIndex, index, field, stringResult);
            break;
        case 54:
            result = UI_FeederDataExtended_DynamicMenu(index, listBox, field, stringResult);
            break;
        case 74:
            result = Live_FileShare_GetFeederData(controllerIndex, 74, index, field, (const char **)stringResult, floatResult);
            break;
        case 79:
            result = Live_FileShare_GetFeederData(controllerIndex, 79, index, field, (const char **)stringResult, floatResult);
            break;
        case 80:
            result = Live_FileShare_GetFeederData(controllerIndex, 80, index, field, (const char **)stringResult, floatResult);
            break;
        case 84:
            if ( I_stricmp(field, "index") )
            {
                v8 = 0;
            }
            else
            {
                *floatResult = (float)index;
                v8 = 1;
            }
            result = v8;
            break;
        case 86:
            result = Live_FileShare_GetFeederData(controllerIndex, 86, index, field, (const char **)stringResult, floatResult);
            break;
        default:
            result = UI_Project_FeederDataExtended(controllerIndex, contextIndex, (int)feederID, listBox, field, stringResult);
            break;
    }
    return result;
}

FriendInfo info_3;
FriendInfo info_4;
char __cdecl UI_FeederDataExtended_OnlineFriends(
                int controllerIndex,
                int index,
                const char *field,
                char **stringResult)
{
    if ( I_stricmp(field, "xuid") )
    {
        if ( I_stricmp(field, "name") )
        {
            return 0;
        }
        else
        {
            if ( Friends_GetByIndex(controllerIndex, 0, index, &info_3) )
                *stringResult = info_3.name;
            else
                *stringResult = 0;
            return 1;
        }
    }
    else
    {
        if ( Friends_GetByIndex(controllerIndex, 0, index, &info_4) )
            *stringResult = va("%lld", info_4.dwid);
        else
            *stringResult = 0;
        return 1;
    }
}

char __cdecl UI_FeederDataExtended_RecentPlayers(
                int controllerIndex,
                int index,
                const char *field,
                char **stringResult)
{
    XuidInfo v5; // [esp+8h] [ebp-C0h] BYREF
    _BYTE v6[48]; // [esp+38h] [ebp-90h] BYREF
    XuidInfo result; // [esp+68h] [ebp-60h] BYREF
    _BYTE v8[48]; // [esp+98h] [ebp-30h] BYREF

    if ( I_stricmp(field, "xuid") )
    {
        if ( I_stricmp(field, "name") )
        {
            return 0;
        }
        else
        {
            memcpy(v6, LiveMeetPlayer_GetMetFriendInfo(&v5, controllerIndex, index), sizeof(v6));
            memcpy(&recentPlayerInfo, v6, sizeof(recentPlayerInfo));
            if ( recentPlayerInfo.playerXuids )
                *stringResult = recentPlayerInfo.gamertag;
            else
                *stringResult = 0;
            return 1;
        }
    }
    else
    {
        memcpy(v8, LiveMeetPlayer_GetMetFriendInfo(&result, controllerIndex, index), sizeof(v8));
        memcpy(&recentPlayerInfo, v8, sizeof(recentPlayerInfo));
        if ( recentPlayerInfo.playerXuids )
            *stringResult = va("%lld", recentPlayerInfo.playerXuids);
        else
            *stringResult = 0;
        return 1;
    }
}

char __cdecl UI_FeederDataExtended_DynamicMenu(
                int index,
                listBoxDef_s *listBox,
                const char *field,
                char **stringResult)
{
    int col; // [esp+0h] [ebp-4h]

    if ( I_stricmp(field, "action") )
    {
        col = atoi(field);
        if ( col )
        {
            *stringResult = listBox->rows[index].cells[col - 1].stringValue;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        *stringResult = listBox->rows[index].eventName;
        return 1;
    }
}

void __cdecl UI_AddToFeeder(itemDef_s *item, const char *eventName, int argCount, const char **args)
{
    char *v4; // eax
    int maxChars; // [esp-4h] [ebp-14h]
    int j; // [esp+0h] [ebp-10h]
    listBoxDef_s *listbox; // [esp+4h] [ebp-Ch]
    itemDef_s *actualItem; // [esp+8h] [ebp-8h]
    int argStart; // [esp+Ch] [ebp-4h]

    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp", 4310, 0, "%s", "item") )
        __debugbreak();
    actualItem = Menu_GetMatchingItemByNumber(item->parent, 0, (char*)"dynamic_menu_feeder");
    if ( !actualItem
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp", 4312, 0, "%s", "actualItem") )
    {
        __debugbreak();
    }
    listbox = Item_GetListBoxDef(actualItem);
    if ( listbox->rowCount == listbox->maxRows )
    {
        Com_Printf(13, "UI: Feeder only has %d rows - not enough for item %s\n", listbox->maxRows, *args);
    }
    else
    {
        I_strncpyz(listbox->rows[listbox->rowCount].eventName, eventName, 32);
        listbox->rows[listbox->rowCount].name = listbox->rowCount;
        listbox->rows[listbox->rowCount].status = 0;
        j = 3;
        for ( argStart = 0; j < listbox->numColumns && argStart < argCount; ++argStart )
        {
            if ( listbox->rows[listbox->rowCount].cells[j].type == 1
                || listbox->rows[listbox->rowCount].cells[j].type == 4
                || listbox->rows[listbox->rowCount].cells[j].type == 7 )
            {
                maxChars = listbox->rows[listbox->rowCount].cells[j].maxChars;
                v4 = UI_SafeTranslateString(args[argStart]);
                I_strncpyz(listbox->rows[listbox->rowCount].cells[j].stringValue, v4, maxChars);
            }
            else
            {
                I_strncpyz(
                    listbox->rows[listbox->rowCount].cells[j].stringValue,
                    args[argStart],
                    listbox->rows[listbox->rowCount].cells[j].maxChars);
            }
            ++j;
        }
        ++listbox->rowCount;
    }
}

void __cdecl UI_AddToFeederExtended(
                itemDef_s *item,
                const char *onActionEvent,
                const char *onFocusEvent,
                bool disableCondition,
                int argCount,
                const char **args)
{
    char *v6; // eax
    int maxChars; // [esp-4h] [ebp-14h]
    int j; // [esp+0h] [ebp-10h]
    listBoxDef_s *listbox; // [esp+4h] [ebp-Ch]
    itemDef_s *actualItem; // [esp+8h] [ebp-8h]
    int argStart; // [esp+Ch] [ebp-4h]

    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp", 4349, 0, "%s", "item") )
        __debugbreak();
    actualItem = Menu_GetMatchingItemByNumber(item->parent, 0, (char*)"dynamic_menu_feeder");
    if ( !actualItem
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp", 4351, 0, "%s", "actualItem") )
    {
        __debugbreak();
    }
    listbox = Item_GetListBoxDef(actualItem);
    if ( listbox->rowCount == listbox->maxRows )
    {
        Com_Printf(13, "UI: Feeder only has %d rows - not enough for item %s\n", listbox->maxRows, *args);
    }
    else
    {
        I_strncpyz(listbox->rows[listbox->rowCount].eventName, onActionEvent, 32);
        I_strncpyz(listbox->rows[listbox->rowCount].onFocusEventName, onFocusEvent, 32);
        listbox->rows[listbox->rowCount].name = listbox->rowCount;
        listbox->rows[listbox->rowCount].status = disableCondition;
        j = 3;
        for ( argStart = 0; j < listbox->numColumns && argStart < argCount; ++argStart )
        {
            if ( listbox->rows[listbox->rowCount].cells[j].type == 1
                || listbox->rows[listbox->rowCount].cells[j].type == 4
                || listbox->rows[listbox->rowCount].cells[j].type == 7 )
            {
                maxChars = listbox->rows[listbox->rowCount].cells[j].maxChars;
                v6 = UI_SafeTranslateString(args[argStart]);
                I_strncpyz(listbox->rows[listbox->rowCount].cells[j].stringValue, v6, maxChars);
            }
            else
            {
                I_strncpyz(
                    listbox->rows[listbox->rowCount].cells[j].stringValue,
                    args[argStart],
                    listbox->rows[listbox->rowCount].cells[j].maxChars);
            }
            ++j;
        }
        ++listbox->rowCount;
    }
}

void __cdecl UI_ChangeRowStatus(itemDef_s *item, int rowToChange, int newStatus)
{
    int i; // [esp+0h] [ebp-Ch]
    listBoxDef_s *listbox; // [esp+4h] [ebp-8h]
    itemDef_s *actualItem; // [esp+8h] [ebp-4h]

    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp", 4392, 0, "%s", "item") )
        __debugbreak();
    actualItem = Menu_GetMatchingItemByNumber(item->parent, 0, (char*)"dynamic_menu_feeder");
    if ( !actualItem
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp", 4394, 0, "%s", "actualItem") )
    {
        __debugbreak();
    }
    listbox = Item_GetListBoxDef(actualItem);
    for ( i = 0; i < listbox->rowCount; ++i )
    {
        if ( listbox->rows[i].name == rowToChange )
        {
            listbox->rows[i].status = newStatus;
            return;
        }
    }
}

void __cdecl UI_ClearFeeder(int contextIndex, itemDef_s *item, bool resetCursorPos)
{
    int j; // [esp+0h] [ebp-Ch]
    listBoxDef_s *listbox; // [esp+4h] [ebp-8h]
    itemDef_s *actualItem; // [esp+8h] [ebp-4h]

    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp", 4414, 0, "%s", "item") )
        __debugbreak();
    actualItem = Menu_GetMatchingItemByNumber(item->parent, 0, (char *)"dynamic_menu_feeder");
    if ( !actualItem
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp", 4416, 0, "%s", "actualItem") )
    {
        __debugbreak();
    }
    listbox = Item_GetListBoxDef(actualItem);
    if ( resetCursorPos )
        listbox->cursorPos[contextIndex] = 0;
    while ( listbox->rowCount > 0 )
    {
        for ( j = 0; j < listbox->numColumns; ++j )
            I_strncpyz(
                listbox->rows[listbox->rowCount - 1].cells[j].stringValue,
                "",
                listbox->rows[listbox->rowCount - 1].cells[j].maxChars);
        listbox->rows[listbox->rowCount - 1].name = 0;
        listbox->rows[--listbox->rowCount].status = 0;
    }
}

void __cdecl UI_RaiseFeederEvent(int localClientNum, itemDef_s *item, const char *eventName)
{
    uiInfo_s *v3; // eax
    uiInfo_s *Info; // eax
    itemDef_s *v5; // [esp-8h] [ebp-18h]
    const char *v6; // [esp-4h] [ebp-14h]
    const char *stringResult; // [esp+0h] [ebp-10h] BYREF
    float floatResult; // [esp+4h] [ebp-Ch] BYREF
    itemDef_s *actualItem; // [esp+8h] [ebp-8h]
    int contextIndex; // [esp+Ch] [ebp-4h]

    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp", 4445, 0, "%s", "item") )
        __debugbreak();
    actualItem = Menu_GetMatchingItemByNumber(item->parent, 0, (char *)"dynamic_menu_feeder");
    if ( !actualItem
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp", 4447, 0, "%s", "actualItem") )
    {
        __debugbreak();
    }
    contextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
    if ( contextIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp",
                    4450,
                    0,
                    "%s",
                    "contextIndex != INVALID_UI_CONTEXT") )
    {
        __debugbreak();
    }
    if ( *eventName )
    {
        Info = UI_UIContext_GetInfo(contextIndex);
        Item_RunEventScript(localClientNum, &Info->uiDC, actualItem, eventName);
    }
    else
    {
        UI_FeederData(localClientNum, actualItem, "action", (char **)&stringResult, &floatResult);
        v6 = stringResult;
        v5 = actualItem;
        v3 = UI_UIContext_GetInfo(contextIndex);
        Item_RunEventScript(localClientNum, &v3->uiDC, v5, v6);
    }
}

char __cdecl UI_FeederData(
                int localClientNum,
                itemDef_s *item,
                const char *field,
                char **stringResult,
                float *numberResult)
{
    float feederID; // [esp+8h] [ebp-10h]
    int index; // [esp+Ch] [ebp-Ch]
    int contextIndex; // [esp+10h] [ebp-8h]
    listBoxDef_s *listBox; // [esp+14h] [ebp-4h]

    listBox = Item_GetListBoxDef(item);
    if ( !listBox )
        return 0;
    contextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
    if ( contextIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_feeders.cpp",
                    4472,
                    0,
                    "%s",
                    "contextIndex != INVALID_UI_CONTEXT") )
    {
        __debugbreak();
    }
    index = listBox->cursorPos[contextIndex];
    feederID = listBox->special;
    *stringResult = 0;
    if ( I_stricmp(field, "count") )
    {
        if ( I_stricmp(field, "selection") )
        {
            return UI_FeederDataExtended(localClientNum, item, field, stringResult, numberResult);
        }
        else
        {
            *numberResult = (float)index;
            return 1;
        }
    }
    else
    {
        *numberResult = (float)(int)UI_FeederCount(localClientNum, contextIndex, feederID, listBox);
        return 1;
    }
}

