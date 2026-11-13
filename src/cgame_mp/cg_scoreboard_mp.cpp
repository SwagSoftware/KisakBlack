#include "cg_scoreboard_mp.h"
#include <universal/assertive.h>
#include "cg_local_mp.h"
#include <client_mp/cl_ui_pc_mp.h>
#include <universal/offline.h>
#include <client_mp/cl_main_pc_mp.h>
#include <client/cl_rank.h>
#include <cgame/cg_draw_names.h>
#include <game/g_client_fields.h>
#include <client/cl_voice.h>
#include <stringed/stringed_hooks.h>
#include "cg_main_mp.h"
#include <cgame/cg_drawtools.h>
#include <demo/demo_playback.h>
#include <cgame/cg_compass.h>
#include <ui/ui_main_pc.h>
#include <ui/ui_atoms.h>
#include <bgame/bg_misc.h>
#include <ui_mp/ui_main_mp.h>
#include <client/client.h>

const char *sbColumnNames[18] =
{
  "",
  "",
  "MPUI_KILLS",
  "MPUI_DEATHS",
  "MPUI_ASSISTS",
  "MPUI_DEFENDS",
  "MPUI_PLANTS",
  "MPUI_DEFUSES",
  "MPUI_RETURNS",
  "MPUI_CAPTURES",
  "MPUI_DESTRUCTIONS",
  "MPUI_RATIO",
  "MPUI_SURVIVED",
  "MPUI_STABS",
  "MPUI_TOMAHAWKS",
  "MPUI_HUMILIATED",
  "MPUI_X2SCORE",
  "MPUI_HEADSHOTS"
};

const dvar_t *cg_ScoresPing_MaxBars;
const dvar_t *cg_ScoresPing_Interval;
const dvar_t *cg_ScoresPing_HighColor;
const dvar_t *cg_ScoresPing_MedColor;
const dvar_t *cg_ScoresPing_LowColor;
const dvar_t *cg_ScoresPing_BgColor;
const dvar_t *cg_scoreboardScrollStep;
const dvar_t *cg_scoreboardBannerHeight;
const dvar_t *cg_scoreboardItemHeight;
const dvar_t *cg_scoreboardPingWidth;
const dvar_t *cg_scoreboardPingHeight;
const dvar_t *cg_scoreboardWidth;
const dvar_t *cg_scoreboardSplitscreenWidth;
const dvar_t *cg_scoreboardQuarterscreenWidth;
const dvar_t *cg_scoreboardHeight;
const dvar_t *cg_scoreboardMyColor;
const dvar_t *cg_scoreboardRankFontScale;
const dvar_t *cg_scoreboardTextOffset;
const dvar_t *cg_scoreboardFont;
const dvar_t *cg_scoreboardHeaderFontScale;
const dvar_t *cg_scoreboardPingText;
const dvar_t *cg_scoreboardPingGraph;


matchScoreBoardData_t matchScoreBoardData[1];

const char *__cdecl CG_GetNameForScoreboardColumn(int localClientNum, unsigned int columnNumber)
{
    if ( columnNumber >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scoreboard_mp.cpp",
                    406,
                    0,
                    "%s",
                    "columnNumber >= 0 && columnNumber < MAX_SB_COLUMNS") )
    {
        __debugbreak();
    }
    if ( columnNumber < 4 )
        return sbColumnNames[matchScoreBoardData[localClientNum].scoreboardColumnTypes[columnNumber]];
    else
        return "";
}

void __cdecl CG_UpdateMatchScoreboard(int localClientNum)
{
    int v1; // eax
    int v2; // ecx
    const char *NameForScoreboardColumn; // eax
    scoreboardColumnType_t v4; // [esp+Ch] [ebp-30h]
    signed int columnNumber; // [esp+18h] [ebp-24h]
    int j; // [esp+1Ch] [ebp-20h]
    unsigned int clientNum; // [esp+20h] [ebp-1Ch]
    int i; // [esp+24h] [ebp-18h]
    int column; // [esp+28h] [ebp-14h]
    cg_s *cgameGlob; // [esp+2Ch] [ebp-10h]
    float currentScoreboardWidthUsed; // [esp+30h] [ebp-Ch]
    float currentScoreboardWidthUseda; // [esp+30h] [ebp-Ch]
    int numDynamicSBColumns; // [esp+38h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    numDynamicSBColumns = 0;
    matchScoreBoardData[localClientNum].numAllies = 0;
    matchScoreBoardData[localClientNum].numClients = 0;
    matchScoreBoardData[localClientNum].numInGameScoreboardColumns = 0;
    for (column = 0; column < 4; ++column)
        matchScoreBoardData[localClientNum].scoreboardColumnTypes[column] = cgameGlob->scoreboardColumnTypes[column];
    for (i = 0; i < 32; ++i)
    {
        clientNum = cgameGlob->scoreOrder[i];
        if (clientNum >= 0x20)
            break;
        if (cgameGlob->bgs.clientinfo[clientNum].infoValid)
        {
            matchScoreBoardData[localClientNum].matchClientScoreData[i].clientNum = clientNum;
            v1 = localClientNum;
            v2 = i;
            LODWORD(matchScoreBoardData[v1].matchClientScoreData[v2].xuid) = cgameGlob->bgs.clientinfo[clientNum].xuid;
            HIDWORD(matchScoreBoardData[v1].matchClientScoreData[v2].xuid) = HIDWORD(cgameGlob->bgs.clientinfo[clientNum].xuid);
            matchScoreBoardData[localClientNum].matchClientScoreData[i].rank = cgameGlob->bgs.clientinfo[clientNum].rank;
            matchScoreBoardData[localClientNum].matchClientScoreData[i].hRankIcon = cgameGlob->bgs.clientinfo[clientNum].hRankIcon;
            memcpy(
                &matchScoreBoardData[localClientNum].matchClientScoreData[i].score,
                &cgameGlob->bgs.clientinfo[clientNum].score,
                sizeof(matchScoreBoardData[localClientNum].matchClientScoreData[i].score));
            matchScoreBoardData[localClientNum].matchClientScoreData[i].team = cgameGlob->bgs.clientinfo[clientNum].team;
            I_strncpyz(
                matchScoreBoardData[localClientNum].matchClientScoreData[i].name,
                cgameGlob->bgs.clientinfo[clientNum].name,
                32);
            I_strncpyz(
                matchScoreBoardData[localClientNum].matchClientScoreData[i].clanAbbrev,
                cgameGlob->bgs.clientinfo[clientNum].clanAbbrev,
                8);
            if (matchScoreBoardData[localClientNum].matchClientScoreData[i].team == TEAM_ALLIES)
                ++matchScoreBoardData[localClientNum].numAllies;
            ++matchScoreBoardData[localClientNum].numClients;
        }
    }
    if (Flame_GetLocalClientSourceRange())
    {
        CG_AddSBColumnToMatchScoreBoard(localClientNum, LCT_RANK_ICON, 0.07f, "", 1, SB_TYPE_INVALID);
        CG_AddSBColumnToMatchScoreBoard(localClientNum, LCT_STATUS_ICON, 0.05f, "", 1, SB_TYPE_INVALID);
        CG_AddSBColumnToMatchScoreBoard(localClientNum, LCT_NAME, 0.2f, "", 0, SB_TYPE_INVALID);
        CG_AddSBColumnToMatchScoreBoard(localClientNum, LCT_TALKING_ICON, 0.05f, "", 0, SB_TYPE_INVALID);
        currentScoreboardWidthUsed = (float)((float)(0.07f + 0.05f) + 0.2f) + 0.05f;
    }
    else
    {
        CG_AddSBColumnToMatchScoreBoard(localClientNum, LCT_NAME, 0.45f, "", 0, SB_TYPE_INVALID);
        currentScoreboardWidthUsed = 0.45f;
    }
    CG_AddSBColumnToMatchScoreBoard(localClientNum, LCT_SCORE, 0.1f, "CGAME_SB_SCORE", 2, SB_TYPE_INVALID);
    currentScoreboardWidthUseda = currentScoreboardWidthUsed + 0.1f;
    if (cg_scoreboardPingText->current.enabled)
        currentScoreboardWidthUseda = currentScoreboardWidthUseda + 0.1f;

    for (j = 0; j < 4; ++j)
    {
        matchScoreBoardData[localClientNum].scoreboardColumnTypes[j] = cgameGlob->scoreboardColumnTypes[j];
        if (matchScoreBoardData[localClientNum].scoreboardColumnTypes[j] > SB_TYPE_NONE
            && matchScoreBoardData[localClientNum].scoreboardColumnTypes[j] < NUM_SB_TYPES)
        {
            ++numDynamicSBColumns;
        }
    }
    if (currentScoreboardWidthUseda >= 1.0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scoreboard_mp.cpp",
            544,
            0,
            "%s",
            "currentScoreboardWidthUsed < 1.0f"))
    {
        __debugbreak();
    }
    if (currentScoreboardWidthUseda > 1.0)
        currentScoreboardWidthUseda = 1.0f;
    for (columnNumber = 0; columnNumber < 4; ++columnNumber)
    {
        if (matchScoreBoardData[localClientNum].scoreboardColumnTypes[columnNumber] > SB_TYPE_NONE
            && matchScoreBoardData[localClientNum].scoreboardColumnTypes[columnNumber] < NUM_SB_TYPES)
        {
            v4 = matchScoreBoardData[localClientNum].scoreboardColumnTypes[columnNumber];
            NameForScoreboardColumn = CG_GetNameForScoreboardColumn(localClientNum, columnNumber);
            CG_AddSBColumnToMatchScoreBoard(
                localClientNum,
                LCT_SB_COLUMN,
                (float)(1.0 - currentScoreboardWidthUseda) / (float)numDynamicSBColumns,
                NameForScoreboardColumn,
                2,
                v4);
        }
    }
    if (cg_scoreboardPingText->current.enabled)
        CG_AddSBColumnToMatchScoreBoard(localClientNum, LCT_PING, 0.1f, "CGAME_SB_PING", 2, SB_TYPE_INVALID);
}

void __cdecl CG_AddSBColumnToMatchScoreBoard(
    int localClientNum,
    listColumnTypes_t columnType,
    float columnWidth,
    const char *columnName,
    int alignment,
    scoreboardColumnType_t sbColumnType)
{
    matchScoreBoardData[localClientNum].inGameScoreboardColumnInfo[matchScoreBoardData[localClientNum].numInGameScoreboardColumns].type = columnType;
    matchScoreBoardData[localClientNum].inGameScoreboardColumnInfo[matchScoreBoardData[localClientNum].numInGameScoreboardColumns].fWidth = columnWidth;
    matchScoreBoardData[localClientNum].inGameScoreboardColumnInfo[matchScoreBoardData[localClientNum].numInGameScoreboardColumns].pszName = columnName;
    matchScoreBoardData[localClientNum].inGameScoreboardColumnInfo[matchScoreBoardData[localClientNum].numInGameScoreboardColumns].iAlignment = alignment;
    matchScoreBoardData[localClientNum].inGameScoreboardColumnInfo[matchScoreBoardData[localClientNum].numInGameScoreboardColumns++].sbColumnType = sbColumnType;
}

int __cdecl CG_GetMatchScoreboardClientCount(int localClientNum, team_t team)
{
    int i; // [esp+0h] [ebp-8h]
    int clientCount; // [esp+4h] [ebp-4h]

    clientCount = 0;
    if (team == TEAM_NUM_TEAMS)
        return matchScoreBoardData[localClientNum].numClients;
    for (i = 0; i < matchScoreBoardData[localClientNum].numClients; ++i)
    {
        if (matchScoreBoardData[localClientNum].matchClientScoreData[i].team == team)
            ++clientCount;
    }
    return clientCount;
}

team_t __cdecl CG_GetMatchscoreboardTeam(int localClientNum, int index)
{
    int currClient; // [esp+0h] [ebp-4h]

    currClient = CG_GetMatchScoreboardIndexForTeam(localClientNum, index, TEAM_NUM_TEAMS);
    if ((currClient < 0 || currClient >= matchScoreBoardData[localClientNum].numClients)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scoreboard_mp.cpp",
            606,
            0,
            "%s",
            "currClient >= 0 && currClient < matchScoreBoardData[localClientNum].numClients"))
    {
        __debugbreak();
    }
    return matchScoreBoardData[localClientNum].matchClientScoreData[currClient].team;
}

int __cdecl CG_GetMatchScoreboardIndexForTeam(int localClientNum, int index, team_t team)
{
    int currClient; // [esp+0h] [ebp-4h]
    int indexa; // [esp+10h] [ebp+Ch]

    if (team == TEAM_NUM_TEAMS)
    {
        if (index >= matchScoreBoardData[localClientNum].numAllies)
            index -= matchScoreBoardData[localClientNum].numAllies;
        else
            team = TEAM_ALLIES;
    }
    indexa = index + 1;
    for (currClient = 0; currClient < matchScoreBoardData[localClientNum].numClients; ++currClient)
    {
        if (matchScoreBoardData[localClientNum].matchClientScoreData[currClient].team == team
            || team == TEAM_NUM_TEAMS
            && matchScoreBoardData[localClientNum].matchClientScoreData[currClient].team != TEAM_ALLIES)
        {
            --indexa;
        }
        if (!indexa)
            break;
    }
    return currClient;
}

void __cdecl CG_GetInGamePlayerListIcon(int localClientNum, int column, int index, team_t team, Material **handle)
{
    unsigned __int64 v5; // [esp-Ch] [ebp-CCh]
    FriendInfo finfo; // [esp+0h] [ebp-C0h] BYREF
    int currClient; // [esp+BCh] [ebp-4h]

    if (index >= 0 && index < matchScoreBoardData[localClientNum].numClients)
    {
        currClient = CG_GetMatchScoreboardIndexForTeam(localClientNum, index, team);
        if (column == 8)
        {
            HIDWORD(v5) = HIDWORD(matchScoreBoardData[localClientNum].matchClientScoreData[currClient].xuid);
            LODWORD(v5) = matchScoreBoardData[localClientNum].matchClientScoreData[currClient].xuid;
            if (Friends_GetByID(0, v5, &finfo))
                *handle = Material_RegisterHandle("pc_friend", 7);
        }
        else if (column == 9
            && CL_IsPlayerMuted(
                localClientNum,
                matchScoreBoardData[localClientNum].matchClientScoreData[currClient].clientNum))
        {
            *handle = Material_RegisterHandle("voice_off", 7);
        }
    }
}

char *__cdecl CG_GetMatchScoreboardInfo(int localClientNum, int column, int index, team_t team)
{
    char *result; // eax
    __int64 v5; // [esp-8h] [ebp-10h]
    int currClient; // [esp+4h] [ebp-4h]

    if (index < 0 || index >= matchScoreBoardData[localClientNum].numClients)
        return (char *)"";
    currClient = CG_GetMatchScoreboardIndexForTeam(localClientNum, index, team);
    switch (column)
    {
    case 0:
        HIDWORD(v5) = HIDWORD(matchScoreBoardData[localClientNum].matchClientScoreData[currClient].xuid);
        LODWORD(v5) = matchScoreBoardData[localClientNum].matchClientScoreData[currClient].xuid;
        result = va("%llu", v5);
        break;
    case 2:
        result = (char *)CL_GetRankData(
            matchScoreBoardData[localClientNum].matchClientScoreData[currClient].rank,
            MP_RANKTABLE_DISPLAYLEVEL);
        break;
    case 4:
        if (matchScoreBoardData[localClientNum].matchClientScoreData[currClient].clanAbbrev[0])
            result = va(
                "[%s]%s",
                matchScoreBoardData[localClientNum].matchClientScoreData[currClient].clanAbbrev,
                matchScoreBoardData[localClientNum].matchClientScoreData[currClient].name);
        else
            result = matchScoreBoardData[localClientNum].matchClientScoreData[currClient].name;
        break;
    case 5:
        result = va("%d", matchScoreBoardData[localClientNum].matchClientScoreData[currClient].score.score);
        break;
    case 6:
        result = (char *)CG_GetColumnValueString(
            localClientNum,
            &matchScoreBoardData[localClientNum].matchClientScoreData[currClient].score,
            matchScoreBoardData[localClientNum].scoreboardColumnTypes[0]);
        break;
    case 7:
        result = (char *)CG_GetColumnValueString(
            localClientNum,
            &matchScoreBoardData[localClientNum].matchClientScoreData[currClient].score,
            matchScoreBoardData[localClientNum].scoreboardColumnTypes[1]);
        break;
    case 8:
        result = (char *)CG_GetColumnValueString(
            localClientNum,
            &matchScoreBoardData[localClientNum].matchClientScoreData[currClient].score,
            matchScoreBoardData[localClientNum].scoreboardColumnTypes[2]);
        break;
    case 9:
        result = (char *)CG_GetColumnValueString(
            localClientNum,
            &matchScoreBoardData[localClientNum].matchClientScoreData[currClient].score,
            matchScoreBoardData[localClientNum].scoreboardColumnTypes[3]);
        break;
    default:
        result = (char *)"";
        break;
    }
    return result;
}

char *__cdecl CG_GetColumnValueString(int localClientNum, const score_s *score, scoreboardColumnType_t columnType)
{
    double v3; // xmm0_8
    int ColumnValue; // eax
    float deaths; // [esp+8h] [ebp-4h]

    if ( columnType == SB_TYPE_KDRATIO )
    {
        deaths = (float)CG_GetDeaths(localClientNum, score) * 1.0;
        if ( deaths == 0.0 )
            deaths = 1.0f;
        v3 = (float)((float)CG_GetKills(localClientNum, score) / deaths);
        return va("%.2f", v3);
    }
    else
    {
        ColumnValue = CG_GetColumnValue(localClientNum, score, columnType);
        return va("%i", ColumnValue);
    }
}

int __cdecl CG_GetColumnValue(int localClientNum, const score_s *score, scoreboardColumnType_t columnType)
{
    const char *ColumnNameByType; // eax
    int i; // [esp+0h] [ebp-4h]

    if (!score
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scoreboard_mp.cpp", 416, 0, "%s", "score"))
    {
        __debugbreak();
    }
    for (i = 0; ; ++i)
    {
        if (i >= 4)
        {
            ColumnNameByType = CScr_GetColumnNameByType(columnType);
            Com_PrintWarning(
                1,
                "Scoreboard column type %s was accessed but is not valid for current gametype.\n",
                ColumnNameByType);
            return -1;
        }
        if (matchScoreBoardData[localClientNum].scoreboardColumnTypes[i] == columnType)
            break;
    }
    if (columnType == SB_TYPE_X2SCORE)
        return 10 * score->scoreboardColumns[i];
    else
        return score->scoreboardColumns[i];
}

char *__cdecl CG_GetMatchInGamePlayerListInfo(
    int localClientNum,
    int column,
    int index,
    team_t team,
    Material **handle)
{
    char *result; // eax
    int currClient; // [esp+4h] [ebp-4h]
    int indexa; // [esp+18h] [ebp+10h]

    if (index < 0 || index >= matchScoreBoardData[localClientNum].numClients)
        return (char *)"";
    indexa = index + 1;
    for (currClient = 0; currClient < matchScoreBoardData[localClientNum].numClients; ++currClient)
    {
        if (matchScoreBoardData[localClientNum].matchClientScoreData[currClient].team == team)
            --indexa;
        if (!indexa)
            break;
    }
    switch (column)
    {
    case 0:
        result = (char *)CG_IsInGamePlayerListPlayerTalking(
            localClientNum,
            matchScoreBoardData[localClientNum].matchClientScoreData[currClient].clientNum,
            handle);
        break;
    case 4:
        result = (char *)CL_GetRankData(
            matchScoreBoardData[localClientNum].matchClientScoreData[currClient].rank,
            MP_RANKTABLE_DISPLAYLEVEL);
        break;
    case 6:
        if (matchScoreBoardData[localClientNum].matchClientScoreData[currClient].clanAbbrev[0])
            result = va(
                "[%s]%s",
                matchScoreBoardData[localClientNum].matchClientScoreData[currClient].clanAbbrev,
                matchScoreBoardData[localClientNum].matchClientScoreData[currClient].name);
        else
            result = matchScoreBoardData[localClientNum].matchClientScoreData[currClient].name;
        break;
    case 7:
        result = va("%d", matchScoreBoardData[localClientNum].matchClientScoreData[currClient].score.score);
        break;
    default:
        result = (char *)"";
        break;
    }
    return result;
}

const char *__cdecl CG_IsInGamePlayerListPlayerTalking(int localClientNum, unsigned int clientNum, Material **handle)
{
    if ( CL_IsPlayerMuted(localClientNum, clientNum) )
    {
        *handle = Material_RegisterHandle("voice_off", 7);
    }
    else if ( CL_IsPlayerTalking(localClientNum, clientNum) )
    {
        *handle = Material_RegisterHandle("voice_on", 7);
    }
    else
    {
        *handle = 0;
    }
    return "";
}

char *__cdecl CG_GetMatchInGamePlayersInfo(
    int localClientNum,
    int column,
    unsigned int index,
    int feederType,
    Material **handle)
{
    char *result; // eax
    int a_count; // [esp+18h] [ebp-14h]
    cg_s *cgameGlob; // [esp+24h] [ebp-8h]
    int clientIndex; // [esp+28h] [ebp-4h]
    int indexa; // [esp+3Ch] [ebp+10h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if (!cgameGlob || index >= 0x20)
        return (char *)"";
    indexa = index + 1;
    switch (feederType)
    {
    case 'i':
    case 'j':
        for (clientIndex = 0; clientIndex < 32; ++clientIndex)
        {
            if (cgameGlob->bgs.clientinfo[clientIndex].infoValid)
            {
                if (cgameGlob->bgs.clientinfo[clientIndex].team == (feederType == 105) + 1 && !--indexa)
                    break;
            }
        }
        break;
    case 'k':
    case 'l':
        a_count = (int)((float)((float)matchScoreBoardData[localClientNum].numClients * 0.5) + 0.4999999990686774);
        for (clientIndex = 0; clientIndex < 32; ++clientIndex)
        {
            if (cgameGlob->bgs.clientinfo[clientIndex].infoValid
                && cgameGlob->bgs.clientinfo[clientIndex].team == TEAM_FREE
                && (feederType == 107 && clientIndex < a_count || feederType == 108 && clientIndex >= a_count)
                && !--indexa)
            {
                break;
            }
        }
        break;
    default:
        break;
    }
    if (clientIndex >= 32)
        return (char *)"";
    switch (column)
    {
    case 3:
        result = (char *)CL_GetRankData(cgameGlob->bgs.clientinfo[clientIndex].rank, MP_RANKTABLE_DISPLAYLEVEL);
        break;
    case 4:
        *handle = cgameGlob->bgs.clientinfo[clientIndex].hRankIcon;
        result = (char *)"";
        break;
    case 5:
        if (cgameGlob->bgs.clientinfo[clientIndex].clanAbbrev[0])
            result = va(
                "[%s]%s",
                cgameGlob->bgs.clientinfo[clientIndex].clanAbbrev,
                cgameGlob->bgs.clientinfo[clientIndex].name);
        else
            result = cgameGlob->bgs.clientinfo[clientIndex].name;
        break;
    case 6:
        result = (char *)CG_IsInGamePlayerListPlayerTalking(localClientNum, clientIndex, handle);
        break;
    case 7:
        result = (char *)"";
        break;
    default:
        result = (char *)"";
        break;
    }
    return result;
}

const char *__cdecl CG_GetMatchInGamePlayerXuid(int localClientNum, int index, team_t team)
{
    __int64 v4; // [esp-8h] [ebp-Ch]
    int currClient; // [esp+0h] [ebp-4h]
    int indexa; // [esp+10h] [ebp+Ch]

    if (index < 0 || index >= matchScoreBoardData[localClientNum].numClients)
        return "";
    indexa = index + 1;
    for (currClient = 0; currClient < matchScoreBoardData[localClientNum].numClients; ++currClient)
    {
        if (matchScoreBoardData[localClientNum].matchClientScoreData[currClient].team == team)
            --indexa;
        if (!indexa)
            break;
    }
    HIDWORD(v4) = HIDWORD(matchScoreBoardData[localClientNum].matchClientScoreData[currClient].xuid);
    LODWORD(v4) = matchScoreBoardData[localClientNum].matchClientScoreData[currClient].xuid;
    return va("%llu", v4);
}

char __cdecl CG_GetMatchInGamePlayersXUID(
    int localClientNum,
    unsigned int index,
    int feederType,
    unsigned __int64 *retXUID)
{
    int a_count; // [esp+18h] [ebp-14h]
    cg_s *cgameGlob; // [esp+24h] [ebp-8h]
    int clientIndex; // [esp+28h] [ebp-4h]
    int indexa; // [esp+38h] [ebp+Ch]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if (!cgameGlob || index >= 0x20)
        return 0;
    indexa = index + 1;
    switch (feederType)
    {
    case 'i':
    case 'j':
        for (clientIndex = 0; clientIndex < 32; ++clientIndex)
        {
            if (cgameGlob->bgs.clientinfo[clientIndex].infoValid)
            {
                if (cgameGlob->bgs.clientinfo[clientIndex].team == (feederType == 105) + 1 && !--indexa)
                    break;
            }
        }
        break;
    case 'k':
    case 'l':
        a_count = (int)((float)((float)matchScoreBoardData[localClientNum].numClients * 0.5) + 0.4999999990686774);
        for (clientIndex = 0; clientIndex < 32; ++clientIndex)
        {
            if (cgameGlob->bgs.clientinfo[clientIndex].infoValid
                && cgameGlob->bgs.clientinfo[clientIndex].team == TEAM_FREE
                && (feederType == 107 && clientIndex < a_count || feederType == 108 && clientIndex >= a_count)
                && !--indexa)
            {
                break;
            }
        }
        break;
    default:
        break;
    }
    if (clientIndex >= 32)
        return 0;
    *retXUID = cgameGlob->bgs.clientinfo[clientIndex].xuid;
    return 1;
}

void __cdecl CG_GetMatchScoreboardRankIcon(int localClientNum, int index, team_t team, Material **handle)
{
    if (index >= 0 && index < matchScoreBoardData[localClientNum].numClients)
        *handle = matchScoreBoardData[localClientNum].matchClientScoreData[CG_GetMatchScoreboardIndexForTeam(
            localClientNum,
            index,
            team)].hRankIcon;
}

int __cdecl CG_GetIndexIntoMatchScoreboard(int localClientNum, int controllerIndex)
{
    int i; // [esp+18h] [ebp-4h]

    for (i = 0; i < matchScoreBoardData[localClientNum].numClients; ++i)
    {
        if (__PAIR64__(
            HIDWORD(matchScoreBoardData[localClientNum].matchClientScoreData[i].xuid),
            matchScoreBoardData[localClientNum].matchClientScoreData[i].xuid) == Live_GetXuid(controllerIndex))
            return i;
    }
    Com_PrintWarning(16, "Couldn't find local player in match scoreboard!\n");
    return -1;
}

int __cdecl CG_GetWagerPlaceForMatchScoreboard(int localClientNum, int matchScoreboardIndex)
{
    int i; // [esp+0h] [ebp-Ch]
    int wagerPlace; // [esp+4h] [ebp-8h]

    if (matchScoreboardIndex >= 0 && matchScoreboardIndex < matchScoreBoardData[localClientNum].numClients)
    {
        wagerPlace = matchScoreboardIndex + 1;
        for (i = matchScoreboardIndex - 1;
            i >= 0
            && matchScoreBoardData[localClientNum].matchClientScoreData[i].score.score == matchScoreBoardData[localClientNum].matchClientScoreData[matchScoreboardIndex].score.score;
            --i)
        {
            --wagerPlace;
        }
        return wagerPlace;
    }
    else
    {
        Com_Printf(16, "Tried to access invalid location in match score board!\n");
        return 1;
    }
}

const score_s *__cdecl GetClientScore(int localClientNum, int clientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->bgs.clientinfo[clientNum].infoValid )
        return &cgameGlob->bgs.clientinfo[clientNum].score;
    else
        return 0;
}

const score_s *__cdecl UI_GetOurClientScore(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    return GetClientScore(localClientNum, cgameGlob->clientNum);
}

const score_s *__cdecl UI_GetScoreAtRank(int localClientNum, int rank)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]
    unsigned int clientNum; // [esp+4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( rank < 1 || rank > 32 )
        return 0;
    clientNum = cgameGlob->teamPlayers[rank + 3];
    if ( clientNum >= 0x20 )
        return 0;
    if ( cgameGlob->bgs.clientinfo[clientNum].infoValid )
        return &cgameGlob->bgs.clientinfo[clientNum].score;
    return 0;
}

char *__cdecl CG_GetGametypeDescription(int localClientNum)
{
    const cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    return SEH_LocalizeTextMessage(cgameGlob->objectiveText, "game objective display", LOCMSG_SAFE);
}

char __cdecl CG_DrawScoreboard_GetTeamColorIndex(int team, int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scoreboard_mp.cpp",
                    1267,
                    0,
                    "cgameGlob->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cgameGlob->clientNum,
                    32) )
    {
        __debugbreak();
    }
    if ( team != 2 && team != 1 )
        return 55;
    if ( cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team != TEAM_ALLIES
        && cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team != TEAM_AXIS )
    {
        return 55;
    }
    if ( cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team == team )
        return 56;
    return 57;
}

int __cdecl CG_DrawScoreboard(int localClientNum)
{
    int menuCount; // [esp+4h] [ebp-24h]
    const char *menuNames[4]; // [esp+8h] [ebp-20h]
    bool clearErrors; // [esp+1Bh] [ebp-Dh]
    const float *fadeColor; // [esp+1Ch] [ebp-Ch]
    float fade; // [esp+20h] [ebp-8h]
    cg_s *cgameGlob; // [esp+24h] [ebp-4h]

    if ( cg_paused->current.integer )
        return 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( CG_IsScoreboardDisplayed(localClientNum) )
    {
        fade = 1.0f;
        fadeColor = colorWhite;
        menuNames[0] = "error_popmenu_party";
        menuNames[1] = "error_popmenu_lobby";
        menuNames[2] = "error_popmenu";
        menuNames[3] = 0;
        menuCount = 0;
        clearErrors = 0;
        while ( menuNames[menuCount] )
        {
            if ( Menu_IsMenuOpenAndVisible(localClientNum, menuNames[menuCount]) )
            {
                UI_CloseMenu(localClientNum, menuNames[menuCount]);
                clearErrors = 1;
                Com_Printf(14, "Clearing Error %s\n", menuNames[menuCount]);
            }
            ++menuCount;
        }
        if ( clearErrors )
            UI_ClearErrors();
    }
    else
    {
        fadeColor = CG_FadeColor(cgameGlob->time, cgameGlob->scoreFadeTime, 100, 100);
        if ( !fadeColor || Demo_IsPaused() )
            return 0;
        fade = *fadeColor;
    }
    if ( cgameGlob->scoresTop <= 0 )
        CenterViewOnClient(localClientNum);
    if ( cgameGlob->scoresTop <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scoreboard_mp.cpp",
                    2248,
                    0,
                    "%s",
                    "cgameGlob->scoresTop > 0") )
    {
        __debugbreak();
    }
    CG_DrawScoreboard_Backdrop(localClientNum, fade);
    CG_DrawScoreboard_ScoresList(localClientNum, fade);
    return 1;
}

void __cdecl CG_DrawScoreboard_Backdrop(int localClientNum, float alpha)
{
    CG_BackdropLeft(localClientNum);
    CG_BackdropTop();
    CG_BackdropWidth();
    CG_DrawBackdropServerInfo(localClientNum, alpha);
}

double __cdecl CG_BackdropWidth()
{
    if ( !CG_IsShowingZombieMap() )
        return cg_scoreboardWidth->current.value * 1.0;
    if ( Flame_GetLocalClientSourceRange() )
        return cg_scoreboardSplitscreenWidth->current.value * 1.0;
    return cg_scoreboardQuarterscreenWidth->current.value * 1.0;
}

double __cdecl CG_BackdropLeft(int localClientNum)
{
    float v3; // [esp+4h] [ebp-10h]
    float v4; // [esp+Ch] [ebp-8h]

    v4 = scrPlaceView[localClientNum].virtualViewableMax[0] - scrPlaceView[localClientNum].virtualViewableMin[0];
    v3 = (v4 - CG_BackdropWidth()) / 2.0;
    if ( (float)(0.0 - v3) < 0.0 )
        return v3;
    else
        return 0.0f;
}

double __cdecl CG_BackdropTop()
{
    if ( (float)(0.0 - (float)((float)((float)(480.0 - cg_scoreboardHeight->current.value) / 2.0) - 15.0)) < 0.0 )
        return (float)((float)((float)(480.0 - cg_scoreboardHeight->current.value) / 2.0) - 15.0);
    else
        return 0.0f;
}

void __cdecl CG_DrawBackdropServerInfo(int localClientNum, float alpha)
{
    int v2; // esi
    int v3; // eax
    float v4; // [esp+20h] [ebp-158h]
    float v5; // [esp+28h] [ebp-150h]
    float v6; // [esp+2Ch] [ebp-14Ch]
    float value; // [esp+38h] [ebp-140h]
    Font_s *footerFont; // [esp+40h] [ebp-138h]
    ScreenPlacement *scrPlace; // [esp+44h] [ebp-134h]
    const cgs_t *cgs; // [esp+4Ch] [ebp-12Ch]
    char serverName[256]; // [esp+50h] [ebp-128h] BYREF
    const char *serverIP; // [esp+154h] [ebp-24h]
    int horzAlign; // [esp+158h] [ebp-20h]
    float x; // [esp+15Ch] [ebp-1Ch]
    float y; // [esp+160h] [ebp-18h]
    float color[4]; // [esp+164h] [ebp-14h] BYREF
    float fontScale; // [esp+174h] [ebp-4h]

    horzAlign = 1;
    color[0] = colorWhite[0];
    color[1] = 1.0;
    color[2] = 1.0;
    color[3] = alpha;
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    I_strncpyz(serverName, cgs->szHostName, 256);
    UI_UseAltColorPalette(serverName);
    serverIP = CL_GetServerIPAddress();
    if ( !I_stricmp(serverIP, "0.0.0.0:0") )
        serverIP = "";
    fontScale = 0.2f;
    scrPlace = &scrPlaceView[localClientNum];
    do
    {
        footerFont = UI_GetFontHandle(scrPlace, cg_scoreboardFont->current.integer, fontScale);
        v2 = UI_TextWidth(serverName, 0, footerFont, fontScale);
        v6 = (float)(v2 + UI_TextWidth(serverIP, 0, footerFont, fontScale) + 4);
        if ( CG_BackdropWidth() - 6.0 - 8.0 >= v6 )
            break;
        fontScale = fontScale - 0.0099999998;
    }
    while ( fontScale > 0.075000003 );
    value = cg_scoreboardHeight->current.value;
    v5 = CG_BackdropTop() + value - 3.0 - 2.0 - 14.0 + 14.0;
    y = v5 - (double)(14 - UI_TextHeight(footerFont, fontScale)) * 0.5;
    x = CG_BackdropLeft(localClientNum) + 3.0 + 2.0 + 4.0;
    UI_DrawText(scrPlace, serverName, 0x7FFFFFFF, footerFont, x, y, horzAlign, 0, fontScale, color, 3);
    v4 = CG_BackdropLeft(localClientNum);
    x = CG_BackdropWidth() + v4 - 3.0 - 2.0 - 4.0;
    v3 = UI_TextWidth(serverIP, 0, footerFont, fontScale);
    UI_DrawText(
        scrPlace,
        (char *)serverIP,
        0x7FFFFFFF,
        footerFont,
        x - (float)(v3 + 4),
        y,
        horzAlign,
        0,
        fontScale,
        color,
        3);
}

void __cdecl CG_DrawScoreboard_ScoresList(int localClientNum, float alpha)
{
    double v2; // st7
    float scrollbarTop; // [esp+14h] [ebp-2Ch]
    int team; // [esp+18h] [ebp-28h]
    cg_s *cgameGlob; // [esp+1Ch] [ebp-24h]
    float listWidth; // [esp+20h] [ebp-20h]
    float yb; // [esp+24h] [ebp-1Ch]
    float y; // [esp+24h] [ebp-1Ch]
    float ya; // [esp+24h] [ebp-1Ch]
    float yc; // [esp+24h] [ebp-1Ch]
    float yd; // [esp+24h] [ebp-1Ch]
    float color[4]; // [esp+28h] [ebp-18h] BYREF
    float realListTop; // [esp+38h] [ebp-8h]
    int drawLine; // [esp+3Ch] [ebp-4h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->numScores )
    {
        if ( cgameGlob->scoresTop > CG_ScoreboardTotalLines(localClientNum)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scoreboard_mp.cpp",
                        2076,
                        0,
                        "%s\n\t(cgameGlob->scoresTop) = %i",
                        "(cgameGlob->scoresTop <= CG_ScoreboardTotalLines( localClientNum ))",
                        cgameGlob->scoresTop) )
        {
            __debugbreak();
        }
        cgameGlob->scoresOffBottom = 0;
        yb = CG_BackdropTop() + 3.0 + 2.0 + 36.0 + 1.0;
        color[0] = 1.0f;
        color[1] = 1.0f;
        color[2] = 1.0f;
        color[3] = alpha;
        listWidth = CG_BackdropWidth() - 6.0 - 4.0 - 8.0;
        y = yb + 15.900001;
        scrollbarTop = (float)cg_scoreboardBannerHeight->current.integer + y;
        if ( cgameGlob->scoresTop <= 1 )
            CG_DrawScoreboard_ListColumnHeaders(
                localClientNum,
                color,
                y,
                (float)cg_scoreboardBannerHeight->current.integer,
                listWidth);
        else
            y = CG_DrawScoreboard_ListColumnHeaders(
                        localClientNum,
                        color,
                        y,
                        (float)cg_scoreboardBannerHeight->current.integer,
                        listWidth);
        realListTop = y;
        drawLine = 1;
        if ( cgameGlob->teamPlayers[1] || cgameGlob->teamPlayers[2] )
        {
            team = cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team;
            if ( team != 1 && team != 2 )
                team = 2;
            ya = CG_DrawTeamOfClientScore(localClientNum, color, y, team, listWidth, &drawLine);
            if ( team == 1 )
                v2 = CG_DrawTeamOfClientScore(localClientNum, color, ya + 4.0, 2, listWidth, &drawLine);
            else
                v2 = CG_DrawTeamOfClientScore(localClientNum, color, ya + 4.0, 1, listWidth, &drawLine);
            yc = v2;
            y = yc + 4.0;
        }
        if ( cgameGlob->teamPlayers[0] )
        {
            yd = CG_DrawTeamOfClientScore(localClientNum, color, y, 0, listWidth, &drawLine);
            y = yd + 4.0;
        }
        if ( cgameGlob->teamPlayers[3] )
            CG_DrawTeamOfClientScore(localClientNum, color, y, 3, listWidth, &drawLine);
        cgameGlob->scoresBottom = drawLine - 1;
        CG_DrawScrollbar(localClientNum, scrollbarTop);
    }
}

double __cdecl CG_DrawScoreboard_ListColumnHeaders(
                int localClientNum,
                const float *color,
                float y,
                float h,
                float listWidth)
{
    float v5; // xmm0_4
    float v7; // [esp+1Ch] [ebp-28h]
    char *translation; // [esp+20h] [ebp-24h]
    const listColumnInfo_t *info; // [esp+24h] [ebp-20h] BYREF
    Font_s *font; // [esp+28h] [ebp-1Ch]
    const ScreenPlacement *scrPlace; // [esp+2Ch] [ebp-18h]
    float xAdj; // [esp+30h] [ebp-14h]
    int i; // [esp+34h] [ebp-10h]
    float scale; // [esp+38h] [ebp-Ch]
    float x; // [esp+3Ch] [ebp-8h]
    int fieldCount; // [esp+40h] [ebp-4h] BYREF

    scale = 0.315f;
    if ( CG_IsShowingZombieMap() )
    {
        v5 = scale * 1.5;
        scale = v5;
    }
    scrPlace = &scrPlaceView[localClientNum];
    font = UI_GetFontHandle(scrPlace, cg_scoreboardFont->current.integer, scale);
    x = CG_BackdropLeft(localClientNum) + 3.0 + 2.0 + 4.0;
    CG_GetScoreboardInfo(localClientNum, &info, &fieldCount);
    for ( i = 0; i < fieldCount; ++i )
    {
        if ( *info[i].pszName )
        {
            translation = UI_SafeTranslateString(info[i].pszName);
            v7 = info[i].fWidth * listWidth;
            xAdj = (float)(v7 - (float)UI_TextWidth(translation, 0, font, scale)) * 0.5;
            UI_DrawText(scrPlace, translation, 0x7FFFFFFF, font, x + xAdj, y + h, 1, 0, scale, color, 3);
        }
        x = (float)(info[i].fWidth * listWidth) + x;
    }
    return y + h + 4.0;
}

void __cdecl CG_GetScoreboardInfo(int localClientNum, const listColumnInfo_t **colInfo, int *numFields)
{
    Demo_IsPlaying();
    *colInfo = matchScoreBoardData[localClientNum].inGameScoreboardColumnInfo;
    *numFields = matchScoreBoardData[localClientNum].numInGameScoreboardColumns;
}

int __cdecl CG_ScoreboardTotalLines(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]
    int total; // [esp+4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    total = cgameGlob->numScores;
    if ( cgameGlob->teamPlayers[0] )
        ++total;
    if ( cgameGlob->teamPlayers[1] )
        ++total;
    if ( cgameGlob->teamPlayers[2] )
        ++total;
    if ( cgameGlob->teamPlayers[3] )
    {
        ++total;
        if ( cgameGlob->teamPlayers[1] || !cgameGlob->teamPlayers[2] )
        {
            if ( !cgameGlob->teamPlayers[2] && cgameGlob->teamPlayers[1] )
                ++total;
        }
        else
        {
            ++total;
        }
    }
    return total;
}

double __cdecl CG_DrawTeamOfClientScore(
                int localClientNum,
                const float *color,
                float y,
                int team,
                float listWidth,
                int *drawLine)
{
    unsigned int clientNum; // [esp+14h] [ebp-24h]
    cg_s *cgameGlob; // [esp+18h] [ebp-20h]
    float teamColor[4]; // [esp+20h] [ebp-18h] BYREF
    int i; // [esp+30h] [ebp-8h]
    const score_s *score; // [esp+34h] [ebp-4h]
    float ya; // [esp+48h] [ebp+10h]
    float yb; // [esp+48h] [ebp+10h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ya = CG_DrawScoreboard_ListBanner(
                 localClientNum,
                 color,
                 y,
                 listWidth,
                 (float)cg_scoreboardBannerHeight->current.integer,
                 team,
                 drawLine);
    for ( i = 0; i < cgameGlob->numScores; ++i )
    {
        clientNum = cgameGlob->scoreOrder[i];
        if ( clientNum >= 0x20
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scoreboard_mp.cpp",
                        1770,
                        0,
                        "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        clientNum,
                        32) )
        {
            __debugbreak();
        }
        if ( cgameGlob->bgs.clientinfo[clientNum].infoValid )
        {
            score = &cgameGlob->bgs.clientinfo[clientNum].score;
            if ( cgameGlob->bgs.clientinfo[clientNum].team == team )
            {
                if ( CG_CheckDrawScoreboardLine(localClientNum, drawLine, ya, 12.0) )
                {
                    CG_ScoreboardTeamColor(localClientNum, team, teamColor);
                    teamColor[3] = color[3];
                    yb = CG_DrawClientScore(localClientNum, clientNum, teamColor, ya, score, listWidth);
                    ya = yb + 4.0;
                }
            }
        }
    }
    return ya;
}

int __cdecl CG_CheckDrawScoreboardLine(int localClientNum, int *drawLine, float y, float lineHeight)
{
    float v5; // [esp+0h] [ebp-Ch]
    float value; // [esp+4h] [ebp-8h]
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->scoresOffBottom )
        return 0;
    if ( *drawLine >= cgameGlob->scoresTop )
    {
        value = cg_scoreboardHeight->current.value;
        v5 = y + lineHeight;
        if ( v5 <= CG_BackdropTop() + value - 3.0 - 2.0 - 14.0 - 1.0 )
        {
            ++*drawLine;
            return 1;
        }
        else
        {
            cgameGlob->scoresOffBottom = 1;
            return 0;
        }
    }
    else
    {
        ++*drawLine;
        return 0;
    }
}

double __cdecl CG_DrawScoreboard_ListBanner(
                int localClientNum,
                const float *color,
                float y,
                float w,
                float h,
                int team,
                int *piDrawLine)
{
    float v8; // xmm0_4
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    float v12; // xmm0_4
    const char *String; // eax
    const char *v14; // eax
    float v15; // [esp+0h] [ebp-68h]
    float v16; // [esp+4h] [ebp-64h]
    Font_s *bannerFont; // [esp+2Ch] [ebp-3Ch]
    const char *teamName; // [esp+30h] [ebp-38h]
    const char *teamNamea; // [esp+30h] [ebp-38h]
    Font_s *bannerScoreFont; // [esp+34h] [ebp-34h]
    ScreenPlacement *scrPlace; // [esp+38h] [ebp-30h]
    float bannerFontScale; // [esp+3Ch] [ebp-2Ch]
    cg_s *cgameGlob; // [esp+40h] [ebp-28h]
    Material *material; // [esp+44h] [ebp-24h]
    char *shaderName; // [esp+48h] [ebp-20h]
    char *displayString; // [esp+4Ch] [ebp-1Ch]
    char *displayStringa; // [esp+4Ch] [ebp-1Ch]
    char *displayStringb; // [esp+4Ch] [ebp-1Ch]
    int livePlayers; // [esp+50h] [ebp-18h]
    float x; // [esp+54h] [ebp-14h]
    float xa; // [esp+54h] [ebp-14h]
    float xb; // [esp+54h] [ebp-14h]
    char teamScore[12]; // [esp+58h] [ebp-10h] BYREF

    memset(teamScore, 0, sizeof(teamScore));
    bannerFontScale = 0.32f;
    if ( !CG_CheckDrawScoreboardLine(localClientNum, piDrawLine, y, h) )
        return y;
    if ( CG_IsShowingZombieMap() )
    {
        v8 = 0.31999999 * 1.5;
        bannerFontScale = v8;
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    scrPlace = &scrPlaceView[localClientNum];
    bannerFont = UI_GetFontHandle(scrPlace, cg_scoreboardFont->current.integer, bannerFontScale);
    bannerScoreFont = UI_GetFontHandle(scrPlace, 6, bannerFontScale);
    if ( team )
    {
        if ( team == 1 )
        {
            shaderName = (char *)Dvar_GetString("g_TeamIcon_Axis");
            teamName = Dvar_GetString("g_TeamName_Axis");
            Com_sprintf(teamScore, 0xCu, "%i", cgameGlob->teamScores[1]);
            v9 = SEH_LocalizeTextMessage(teamName, "scoreboard team name", LOCMSG_SAFE);
            displayString = va("%s", v9);
        }
        else if ( team == 2 )
        {
            shaderName = (char *)Dvar_GetString("g_TeamIcon_Allies");
            teamNamea = Dvar_GetString("g_TeamName_Allies");
            Com_sprintf(teamScore, 0xCu, "%i", cgameGlob->teamScores[2]);
            v10 = SEH_LocalizeTextMessage(teamNamea, "scoreboard team name", LOCMSG_SAFE);
            displayString = va("%s", v10);
        }
        else
        {
            if ( team != 3
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scoreboard_mp.cpp",
                            1358,
                            0,
                            "%s\n\t(team) = %i",
                            "(team == TEAM_SPECTATOR)",
                            team) )
            {
                __debugbreak();
            }
            shaderName = (char *)Dvar_GetString("g_TeamIcon_Spectator");
            v11 = SEH_LocalizeTextMessage("CGAME_SPECTATORS", "scoreboard team name", LOCMSG_SAFE);
            displayString = va("%s", v11);
        }
    }
    else
    {
        shaderName = (char *)Dvar_GetString("g_TeamIcon_Free");
        Com_sprintf(teamScore, 0xCu, "%i", cgameGlob->teamScores[0]);
        displayString = (char *)"";
    }
    x = CG_BackdropLeft(localClientNum) + 3.0 + 2.0 + 4.0;
    material = Material_RegisterHandle(shaderName, 7);
    if ( Flame_GetLocalClientSourceRange() && !Material_IsDefault(material) && cgameGlob->teamPlayers[team] > 0 )
    {
        v12 = h * 1.15;
        v15 = CG_BackdropLeft(localClientNum) + 3.0 + 2.0 + 4.0 - v12 - 8.0;
        UI_DrawHandlePic(
            scrPlace,
            v15,
            (float)(8.0 - (float)(v12 / 2.0)) + (float)((float)(y + h) - 3.0),
            v12,
            v12,
            1,
            0,
            color,
            material);
    }
    if ( team != 3 )
    {
        v16 = CG_BackdropLeft(localClientNum) + 3.0 + 2.0 + 4.0;
        UI_DrawText(scrPlace, teamScore, 0x7FFFFFFF, bannerScoreFont, v16, (float)(y + h) + 6.0, 1, 0, 0.64999998, color, 3);
        x = (float)(UI_TextWidth(teamScore, 0x7FFFFFFF, bannerScoreFont, 0.64999998) + 8) + x;
    }
    UI_DrawText(scrPlace, displayString, 0x7FFFFFFF, bannerFont, x, y + h, 1, 0, bannerFontScale, color, 3);
    xa = (float)(UI_TextWidth(displayString, 0x7FFFFFFF, bannerFont, bannerFontScale) + 8) + x;
    displayStringa = va("( %i )", cgameGlob->teamPlayers[team]);
    UI_DrawText(scrPlace, displayStringa, 0x7FFFFFFF, bannerFont, xa, y + h, 1, 0, bannerFontScale, color, 3);
    String = Dvar_GetString("g_gametype");
    if ( !I_strcmp(String, "sd") && team != 3 )
    {
        if ( Flame_GetLocalClientSourceRange() )
        {
            xb = (float)UI_TextWidth(displayStringa, 0x7FFFFFFF, bannerFont, bannerFontScale) + xa;
            livePlayers = CG_GetLivePlayersOnTeam(localClientNum, team);
            v14 = SEH_LocalizeTextMessage("MPUI_ALIVE", "scoreboard header", LOCMSG_SAFE);
            displayStringb = va("        %i %s", livePlayers, v14);
            UI_DrawText(scrPlace, displayStringb, 0x7FFFFFFF, bannerFont, xb, y + h, 1, 0, bannerFontScale, color, 3);
        }
    }
    return y + h + 4.0;
}

int __cdecl CG_GetLivePlayersOnTeam(int localClientNum, int team)
{
    int clientNum; // [esp+0h] [ebp-14h]
    int i; // [esp+4h] [ebp-10h]
    cg_s *cgameGlob; // [esp+8h] [ebp-Ch]
    int livePlayers; // [esp+Ch] [ebp-8h]

    livePlayers = 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    for ( i = 0; i < 32; ++i )
    {
        clientNum = cgameGlob->scoreOrder[i];
        if ( clientNum == 32 )
            break;
        if ( cgameGlob->bgs.clientinfo[clientNum].infoValid
            && cgameGlob->bgs.clientinfo[clientNum].team == team
            && !cgameGlob->bgs.clientinfo[clientNum].hStatusIcon )
        {
            ++livePlayers;
        }
    }
    return livePlayers;
}

double __cdecl CG_DrawClientScore(
                int localClientNum,
                unsigned int clientNum,
                const float *color,
                float y,
                const score_s *score,
                float listWidth)
{
    int v7; // eax
    int iAlignment; // [esp+Ch] [ebp-80h]
    Font_s *v9; // [esp+10h] [ebp-7Ch]
    const listColumnInfo_t *info; // [esp+28h] [ebp-64h] BYREF
    const ScreenPlacement *scrPlace; // [esp+2Ch] [ebp-60h]
    cg_s *cgameGlob; // [esp+30h] [ebp-5Ch]
    const char *string; // [esp+34h] [ebp-58h]
    Material *material; // [esp+38h] [ebp-54h]
    float xAdj; // [esp+3Ch] [ebp-50h]
    float textColor[4]; // [esp+40h] [ebp-4Ch] BYREF
    float backing[4]; // [esp+50h] [ebp-3Ch] BYREF
    Font_s *listFont; // [esp+60h] [ebp-2Ch]
    float backColor[4]; // [esp+64h] [ebp-28h] BYREF
    clientInfo_t *ci; // [esp+74h] [ebp-18h]
    int i; // [esp+78h] [ebp-14h]
    float x; // [esp+7Ch] [ebp-10h]
    float h; // [esp+80h] [ebp-Ch]
    int fieldCount; // [esp+84h] [ebp-8h] BYREF
    float w; // [esp+88h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( clientNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scoreboard_mp.cpp",
                    1518,
                    0,
                    "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    clientNum,
                    32) )
    {
        __debugbreak();
    }
    ci = &cgameGlob->bgs.clientinfo[clientNum];
    if ( !ci->infoValid )
        return y;
    scrPlace = &scrPlaceView[localClientNum];
    CG_GetScoreboardInfo(localClientNum, &info, &fieldCount);
    x = CG_BackdropLeft(localClientNum) + 3.0 + 2.0 + 4.0;
    h = 12.0f;
    material = Material_RegisterHandle("white", 7);
    backColor[0] = *color;
    backColor[1] = color[1];
    backColor[2] = color[2];
    backColor[3] = 0.5 * color[3];
    backing[0] = 0.5f;
    backing[1] = 0.5f;
    backing[2] = 0.5f;
    backing[3] = 0.5f;
    UI_DrawHandlePic(scrPlace, x, y, listWidth, h, 1, 0, backing, material);
    UI_DrawHandlePic(scrPlace, x, y, listWidth, h, 1, 0, backColor, material);
    listFont = UI_GetFontHandle(scrPlace, cg_scoreboardFont->current.integer, 0.22);
    x = CG_BackdropLeft(localClientNum) + 3.0 + 2.0 + 4.0;
    if ( clientNum == cgameGlob->clientNum )
    {
        Dvar_GetUnpackedColor(cg_scoreboardMyColor, textColor);
    }
    else
    {
        textColor[0] = 1.0f;
        textColor[1] = 1.0f;
        textColor[2] = 1.0f;
    }
    textColor[3] = color[3];
    for ( i = 0; i < fieldCount; ++i )
    {
        w = info[i].fWidth * listWidth;
        switch ( info[i].type )
        {
            case LCT_RANK_ICON:
                if ( ci->hRankIcon )
                {
                    string = CL_GetRankData(ci->rank, MP_RANKTABLE_DISPLAYLEVEL);
                    v9 = listFont;
                    iAlignment = info[i].iAlignment;
                    v7 = R_TextHeight(listFont);
                    DrawListString(
                        localClientNum,
                        (char*)string,
                        x,
                        (float)((float)v7 * 0.055) + y,
                        13.0,
                        iAlignment,
                        v9,
                        0.22,
                        3,
                        colorWhite);
                    backColor[0] = 1.0f;
                    backColor[1] = 1.0f;
                    backColor[2] = 1.0f;
                    backColor[3] = color[3];
                    UI_DrawHandlePic(scrPlace, x + 13.0, y, 13.0, 13.0, 1, 0, backColor, ci->hRankIcon);
                }
                break;
            case LCT_STATUS_ICON:
                if ( ci->hStatusIcon )
                {
                    xAdj = CalcXAdj(info[i].iAlignment, w, 13.0);
                    backColor[0] = 1.0f;
                    backColor[1] = 1.0f;
                    backColor[2] = 1.0f;
                    backColor[3] = color[3];
                    UI_DrawHandlePic(scrPlace, x + xAdj, y, 13.0 - 0.0, 13.0 - 0.0, 1, 0, backColor, ci->hStatusIcon);
                }
                break;
            case LCT_NAME:
                if ( Flame_GetLocalClientSourceRange() )
                    string = BG_DisplayName(ci, 3);
                else
                    string = BG_DisplayName(ci, 1);
                DrawListString(localClientNum, (char*)string, x, y, w, info[i].iAlignment, listFont, 0.22, 3, textColor);
                break;
            case LCT_TALKING_ICON:
                if ( CL_IsPlayerMuted(localClientNum, clientNum) )
                {
                    material = Material_RegisterHandle("voice_off", 7);
                }
                else if ( CL_IsPlayerTalking(localClientNum, clientNum) )
                {
                    material = Material_RegisterHandle("voice_on", 7);
                }
                else
                {
                    material = 0;
                }
                if ( Demo_IsPlaying() )
                    material = 0;
                if ( material )
                {
                    backColor[0] = 1.0f;
                    backColor[1] = 1.0f;
                    backColor[2] = 1.0f;
                    backColor[3] = color[3];
                    UI_DrawHandlePic(scrPlace, x, y, 13.0, 13.0, 1, 0, backColor, material);
                }
                break;
            case LCT_SCORE:
                if ( ci->team != TEAM_SPECTATOR )
                {
                    string = va("%i", score->score);
                    DrawListString(localClientNum, (char*)string, x, y, w, info[i].iAlignment, listFont, 0.22, 3, textColor);
                }
                break;
            case LCT_SB_COLUMN:
                if ( ci->team != TEAM_SPECTATOR )
                {
                    string = CG_GetColumnValueString(localClientNum, score, info[i].sbColumnType);
                    DrawListString(localClientNum, (char *)string, x, y, w, info[i].iAlignment, listFont, 0.22, 3, textColor);
                }
                break;
            case LCT_PING:
                string = va("%i", score->ping);
                DrawListString(localClientNum, (char *)string, x, y, w, info[i].iAlignment, listFont, 0.22, 3, textColor);
                break;
            default:
                break;
        }
        x = x + w;
    }
    if ( cg_scoreboardPingGraph->current.enabled )
        CG_DrawClientPing(localClientNum, score->ping, x, y, listWidth * cg_scoreboardPingWidth->current.value, 12.0);
    return y + 12.0;
}

double __cdecl CalcXAdj(int align, float maxw, float w)
{
    if ( align == 1 )
        return (maxw - w) / 2.0;
    if ( align == 2 )
        return maxw - w;
    if ( align )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scoreboard_mp.cpp",
                        1409,
                        0,
                        "%s",
                        "align == UI_LEFT") )
            __debugbreak();
    }
    return 0.0;
}

void __cdecl DrawListString(
                int localClientNum,
                char *string,
                float x,
                float y,
                float width,
                int alignment,
                Font_s *font,
                float scale,
                int style,
                const float *color)
{
    int v10; // eax
    int v11; // eax
    float value; // [esp+1Ch] [ebp-Ch]
    float xAdj; // [esp+24h] [ebp-4h]

    if ( string )
    {
        while ( (float)UI_TextWidth(string, 0x7FFFFFFF, font, scale) > width )
            scale = scale - 0.02;
        iassert(scale > 0);
        if ( scale < 0.16 )
            style = 0;
        v10 = UI_TextWidth(string, 0x7FFFFFFF, font, scale);
        xAdj = CalcXAdj(alignment, width, (float)v10);
        if ( alignment == 2 )
            xAdj = xAdj - 4.0;
        value = cg_scoreboardTextOffset->current.value;
        v11 = R_TextHeight(font);
        UI_DrawText(
            &scrPlaceView[localClientNum],
            string,
            0x7FFFFFFF,
            font,
            x + xAdj,
            (float)(value * (float)((float)((float)v11 * scale) + 12.0)) + y,
            1,
            0,
            scale,
            color,
            style);
    }
}

void __cdecl CG_DrawClientPing(int localClientNum, int ping, float x, float y, float maxWidth, float maxHeight)
{
    int v6; // [esp+20h] [ebp-5Ch]
    int maxBars; // [esp+2Ch] [ebp-50h]
    ScreenPlacement *scrPlace; // [esp+30h] [ebp-4Ch]
    Material *materiala; // [esp+34h] [ebp-48h]
    Material *material; // [esp+34h] [ebp-48h]
    int bar; // [esp+38h] [ebp-44h]
    float lerp; // [esp+3Ch] [ebp-40h]
    float endColor[4]; // [esp+40h] [ebp-3Ch] BYREF
    int interval; // [esp+50h] [ebp-2Ch]
    float startColor[4]; // [esp+54h] [ebp-28h] BYREF
    float color[4]; // [esp+64h] [ebp-18h] BYREF
    float h; // [esp+74h] [ebp-8h]
    float w; // [esp+78h] [ebp-4h]
    float xa; // [esp+8Ch] [ebp+10h]

    scrPlace = &scrPlaceView[localClientNum];
    materiala = Material_RegisterHandle("white", 7);
    Dvar_GetUnpackedColor(cg_ScoresPing_BgColor, color);
    UI_DrawHandlePic(scrPlace, x, y, maxWidth + 4.0, maxHeight, 1, 0, color, materiala);
    maxBars = Dvar_GetInt(cg_ScoresPing_MaxBars);
    interval = Dvar_GetInt(cg_ScoresPing_Interval);
    if ( interval <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scoreboard_mp.cpp",
                    1468,
                    0,
                    "%s\n\t(interval) = %i",
                    "(interval > 0)",
                    interval) )
    {
        __debugbreak();
    }
    material = Material_RegisterHandle("white", 7);
    if ( maxBars - ping / interval < 1 )
        v6 = 1;
    else
        v6 = maxBars - ping / interval;
    if ( v6 >= maxBars / 2 )
    {
        Dvar_GetUnpackedColor(cg_ScoresPing_MedColor, startColor);
        Dvar_GetUnpackedColor(cg_ScoresPing_LowColor, endColor);
        lerp = (float)(v6 - maxBars / 2) / (float)(maxBars / 2);
    }
    else
    {
        Dvar_GetUnpackedColor(cg_ScoresPing_HighColor, startColor);
        Dvar_GetUnpackedColor(cg_ScoresPing_MedColor, endColor);
        lerp = (float)v6 / (float)(maxBars / 2);
    }
    Vec4Lerp(startColor, endColor, lerp, color);
    xa = x + 2.0;
    w = (float)(maxWidth - (float)(maxBars - 1)) / (float)maxBars;
    if ( w < 1.0 )
        w = 1.0f;
    for ( bar = 1; bar <= v6; ++bar )
    {
        h = (float)((float)(maxHeight * cg_scoreboardPingHeight->current.value) * (float)bar) / (float)maxBars;
        UI_DrawHandlePic(scrPlace, xa, (float)((float)(y + maxHeight) - h) - 2.0, w, h, 1, 0, color, material);
        xa = (float)(w + 1.0) + xa;
    }
}

void __cdecl CG_DrawScrollbar(int localClientNum, float top)
{
    Material *v2; // eax
    Material *v3; // eax
    Material *v4; // eax
    Material *v5; // eax
    float v6; // [esp+24h] [ebp-70h]
    float value; // [esp+2Ch] [ebp-68h]
    float scrollbarHeight; // [esp+30h] [ebp-64h]
    ScreenPlacement *scrPlace; // [esp+34h] [ebp-60h]
    const cg_s *cgameGlob; // [esp+38h] [ebp-5Ch]
    Material *material; // [esp+3Ch] [ebp-58h]
    Material *materiala; // [esp+3Ch] [ebp-58h]
    int totalLines; // [esp+40h] [ebp-54h]
    float barColor[4]; // [esp+44h] [ebp-50h] BYREF
    float x; // [esp+54h] [ebp-40h]
    float y; // [esp+58h] [ebp-3Ch]
    float color[4]; // [esp+5Ch] [ebp-38h] BYREF
    float h; // [esp+6Ch] [ebp-28h]
    float handleColor[4]; // [esp+70h] [ebp-24h]
    float arrowColor[4]; // [esp+80h] [ebp-14h]
    float w; // [esp+90h] [ebp-4h]

    memset(color, 0, 12);
    color[3] = 1.0f;
    handleColor[0] = 1.0f;
    handleColor[1] = 1.0f;
    handleColor[2] = 1.0f;
    handleColor[3] = 1.0f;
    arrowColor[0] = 0.97f;
    arrowColor[1] = 0.57999998;
    arrowColor[2] = 0.11;
    arrowColor[3] = 1.0f;
    value = cg_scoreboardHeight->current.value;
    scrollbarHeight = CG_BackdropTop() + value - 3.0 - 2.0 - 14.0 - 1.0 - top - 0.0 - 15.0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->scoresTop > 1 || cgameGlob->scoresOffBottom )
    {
        totalLines = CG_ScoreboardTotalLines(localClientNum);
        scrPlace = &scrPlaceView[localClientNum];
        barColor[0] = color[0];
        barColor[1] = color[1];
        barColor[2] = color[2];
        material = Material_RegisterHandle("white", 7);
        barColor[3] = 0.5 * color[3];
        v6 = CG_BackdropLeft(localClientNum) + 3.0 + 2.0 + 4.0;
        x = (CG_BackdropWidth() - 6.0 - 4.0 - 8.0) * (cg_scoreboardPingWidth->current.value + 1.0) + v6 + 2.0 + 4.0 + 2.0;
        y = top;
        w = 4.0f;
        h = scrollbarHeight;
        UI_DrawHandlePic(scrPlace, x, top, 4.0, scrollbarHeight, 1, 0, barColor, material);
        x = x + 0.0;
        w = w - 0.0;
        y = y + 0.0;
        h = h - 0.0;
        if ( totalLines )
        {
            y = (float)((float)((float)(cgameGlob->scoresTop - 1) / (float)totalLines) * h) + y;
            h = (float)((float)(cgameGlob->scoresBottom - cgameGlob->scoresTop + 1) / (float)totalLines) * h;
        }
        v2 = Material_RegisterHandle("white", 7);
        barColor[0] = handleColor[0];
        barColor[1] = handleColor[1];
        barColor[2] = handleColor[2];
        barColor[3] = 0.25 * handleColor[3];
        UI_DrawHandlePic(scrPlace, x, y, w, h, 1, 0, barColor, v2);
        barColor[0] = arrowColor[0];
        barColor[1] = arrowColor[1];
        barColor[2] = arrowColor[2];
        barColor[3] = arrowColor[3];
        v3 = Material_RegisterHandle("ui_arrow_right", 7);
        x = (float)(x - 7.0) + 1.0;
        y = (float)(top - 15.0) + 2.0;
        w = 15.0f;
        h = 15.0f;
        UI_DrawHandlePicRotated(scrPlace, x, y, 15.0, 15.0, 1, 0, barColor, 270.0, v3);
        v4 = Material_RegisterHandle("hudscoreboardscroll_upkey", 7);
        x = x - 0.0;
        y = y + 17.0;
        w = 16.0f;
        h = 16.0f;
        UI_DrawHandlePic(scrPlace, x, y, 16.0, 16.0, 1, 0, barColor, v4);
        v5 = Material_RegisterHandle("ui_arrow_right", 7);
        y = (float)(top + scrollbarHeight) - 2.0;
        w = 15.0f;
        h = 15.0f;
        UI_DrawHandlePicRotated(scrPlace, x, y, 15.0, 15.0, 1, 0, barColor, 90.0, v5);
        materiala = Material_RegisterHandle("hudscoreboardscroll_downkey", 7);
        x = x - 0.0;
        y = y - 18.0;
        w = 16.0f;
        h = 16.0f;
        UI_DrawHandlePic(scrPlace, x, y, 16.0, 16.0, 1, 0, barColor, materiala);
    }
}

void __cdecl CenterViewOnClient(int localClientNum)
{
    float v1; // [esp+0h] [ebp-28h]
    float value; // [esp+4h] [ebp-24h]
    unsigned int otherClientNum; // [esp+8h] [ebp-20h]
    int clientLine; // [esp+10h] [ebp-18h]
    int team; // [esp+14h] [ebp-14h]
    cg_s *cgameGlob; // [esp+18h] [ebp-10h]
    int viewmax; // [esp+1Ch] [ebp-Ch]
    int i; // [esp+20h] [ebp-8h]
    int clientNum; // [esp+24h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    value = cg_scoreboardHeight->current.value;
    v1 = CG_BackdropTop() + value - 3.0 - 2.0 - 14.0;
    viewmax = (int)((v1
                                 - (CG_BackdropTop()
                                    + 3.0
                                    + 2.0
                                    + 36.0
                                    + 1.0)
                                 - 1.0
                                 - (double)cg_scoreboardBannerHeight->current.integer
                                 - 11.900001
                                 - 4.0)
                                / 16.0);
    clientNum = cgameGlob->clientNum;
    team = cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team;
    clientLine = 1;
    if ( team == 3 )
    {
        cgameGlob->scoresTop = 1;
    }
    else
    {
        for ( i = 0; i < cgameGlob->numScores; ++i )
        {
            otherClientNum = cgameGlob->scoreOrder[i];
            if ( otherClientNum >= 0x20
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scoreboard_mp.cpp",
                            2167,
                            0,
                            "otherClientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                            otherClientNum,
                            32) )
            {
                __debugbreak();
            }
            if ( cgameGlob->bgs.clientinfo[otherClientNum].infoValid && cgameGlob->bgs.clientinfo[otherClientNum].team == team )
            {
                if ( otherClientNum == clientNum )
                    break;
                ++clientLine;
            }
        }
        if ( clientLine > viewmax )
            cgameGlob->scoresTop = clientLine - viewmax / 2 + 1;
        else
            cgameGlob->scoresTop = 1;
    }
}

bool __cdecl CG_IsScoreboardDisplayed(int localClientNum)
{
    return UI_GetActiveMenu(localClientNum) == UIMENU_SCOREBOARD;
}

bool __cdecl CG_IsIntermission(int localClientNum)
{
    return CG_GetLocalClientGlobals(localClientNum)->nextSnap->ps.pm_type == 5;
}

void __cdecl CG_ScrollScoreboardUp(cg_s *cgameGlob)
{
    if ( cgameGlob->scoresTop > 1 )
    {
        cgameGlob->scoresTop -= cg_scoreboardScrollStep->current.integer;
        if ( cgameGlob->scoresTop < 2 )
            cgameGlob->scoresTop = 1;
    }
}

void __cdecl CG_ScrollScoreboardDown(cg_s *cgameGlob)
{
    int totalLines; // [esp+0h] [ebp-4h]

    if ( cgameGlob->scoresOffBottom )
    {
        totalLines = CG_ScoreboardTotalLines(cgameGlob->localClientNum);
        cgameGlob->scoresTop += cg_scoreboardScrollStep->current.integer;
        if ( cgameGlob->scoresTop > totalLines )
            cgameGlob->scoresTop = totalLines;
    }
}

cmd_function_s CG_SetFocusScoreboardCmd_VAR;
void __cdecl CG_InitScoreboard()
{
    Cmd_AddCommandInternal("setFocusScoreboard", CG_SetFocusScoreboardCmd, &CG_SetFocusScoreboardCmd_VAR);
}

void __cdecl CG_SetFocusScoreboardCmd()
{
    UI_SetActiveMenu(0, UIMENU_SCOREBOARD);
}

void __cdecl CG_RegisterScoreboardDvars()
{
    cg_ScoresPing_MaxBars = _Dvar_RegisterInt(
                                                        "cg_ScoresPing_MaxBars",
                                                        4,
                                                        1,
                                                        10,
                                                        1u,
                                                        "Number of bars to show in ping graph");
    cg_ScoresPing_Interval = _Dvar_RegisterInt(
                                                         "cg_ScoresPing_Interval",
                                                         1,
                                                         1,
                                                         1,
                                                         0x40u,
                                                         "Number of milliseconds each bar represents");
    cg_ScoresPing_HighColor = _Dvar_RegisterColor(
                                                            "cg_ScoresPing_HighColor",
                                                            0.80000001,
                                                            0.0,
                                                            0.0,
                                                            1.0,
                                                            1u,
                                                            "Color for high ping");
    cg_ScoresPing_MedColor = _Dvar_RegisterColor(
                                                         "cg_ScoresPing_MedColor",
                                                         0.80000001,
                                                         0.80000001,
                                                         0.0,
                                                         1.0,
                                                         1u,
                                                         "Color for medium ping");
    cg_ScoresPing_LowColor = _Dvar_RegisterColor("cg_ScoresPing_LowColor", 0.0, 0.75, 0.0, 1.0, 1u, "Color for low ping");
    cg_ScoresPing_BgColor = _Dvar_RegisterColor(
                                                        "cg_ScoresPing_BgColor",
                                                        0.25,
                                                        0.25,
                                                        0.25,
                                                        0.5,
                                                        1u,
                                                        "Background color of ping");
    cg_scoreboardScrollStep = _Dvar_RegisterInt(
                                                            "cg_scoreboardScrollStep",
                                                            3,
                                                            1,
                                                            8,
                                                            0,
                                                            "Scroll step amount for the scoreboard");
    cg_scoreboardBannerHeight = _Dvar_RegisterInt(
                                                                "cg_scoreboardBannerHeight",
                                                                35,
                                                                1,
                                                                100,
                                                                0,
                                                                "Banner height of the scoreboard");
    cg_scoreboardItemHeight = _Dvar_RegisterInt("cg_scoreboardItemHeight", 18, 1, 1000, 0, "Item height of each item");
    cg_scoreboardPingWidth = _Dvar_RegisterFloat(
                                                         "cg_scoreboardPingWidth",
                                                         0.035999998,
                                                         0.0,
                                                         1.0,
                                                         0,
                                                         "Width of the ping graph as a % of the scoreboard");
    cg_scoreboardPingHeight = _Dvar_RegisterFloat(
                                                            "cg_scoreboardPingHeight",
                                                            0.69999999,
                                                            0.0,
                                                            1.0,
                                                            0,
                                                            "Height of the ping graph as a % of the scoreboard row height");
    cg_scoreboardWidth = _Dvar_RegisterFloat("cg_scoreboardWidth", 500.0, 0.0, 3.4028235e38, 0, "Width of the scoreboard");
    cg_scoreboardSplitscreenWidth = _Dvar_RegisterFloat(
                                                                        "cg_scoreboardSplitscreenWidth",
                                                                        700.0,
                                                                        0.0,
                                                                        3.4028235e38,
                                                                        0,
                                                                        "Width of the scoreboard in splitscreen");
    cg_scoreboardQuarterscreenWidth = _Dvar_RegisterFloat(
                                                                            "cg_scoreboardQuarterscreenWidth",
                                                                            395.0,
                                                                            0.0,
                                                                            3.4028235e38,
                                                                            0,
                                                                            "Width of the scoreboard for a quarter of the screen splitscreen");
    cg_scoreboardHeight = _Dvar_RegisterFloat(
                                                    "cg_scoreboardHeight",
                                                    435.0,
                                                    0.0,
                                                    3.4028235e38,
                                                    0,
                                                    "Height of the scoreboard");
    cg_scoreboardMyColor = _Dvar_RegisterColor(
                                                     "cg_scoreboardMyColor",
                                                     1.0,
                                                     0.80000001,
                                                     0.40000001,
                                                     1.0,
                                                     0,
                                                     "The local player's font color when shown in scoreboard");
    cg_scoreboardRankFontScale = _Dvar_RegisterFloat(
                                                                 "cg_scoreboardRankFontScale",
                                                                 0.25999999,
                                                                 0.0,
                                                                 3.4028235e38,
                                                                 0,
                                                                 "Scale of rank font");
    cg_scoreboardTextOffset = _Dvar_RegisterFloat(
                                                            "cg_scoreboardTextOffset",
                                                            0.60000002,
                                                            0.0,
                                                            3.4028235e38,
                                                            0,
                                                            "Scoreboard text offset");
    cg_scoreboardFont = _Dvar_RegisterInt(
                                                "cg_scoreboardFont",
                                                3,
                                                0,
                                                6,
                                                0,
                                                "Scoreboard font enum ( see menudefinition.h )");
    cg_scoreboardHeaderFontScale = _Dvar_RegisterFloat(
                                                                     "cg_scoreboardHeaderFontScale",
                                                                     0.315,
                                                                     0.0,
                                                                     3.4028235e38,
                                                                     0,
                                                                     "Scoreboard header font scale");
    cg_scoreboardPingText = _Dvar_RegisterBool("cg_scoreboardPingText", 1, 0, "Whether to show numeric ping value");
    cg_scoreboardPingGraph = _Dvar_RegisterBool("cg_scoreboardPingGraph", 0, 0, "Whether to show graphical ping");
}

void __cdecl CG_RegisterScoreboardGraphics()
{
    Material_RegisterHandle("white", 7);
    Material_RegisterHandle("white", 7);
    Material_RegisterHandle("white", 7);
    Material_RegisterHandle("white", 7);
    Material_RegisterHandle("white", 7);
    Material_RegisterHandle("black", 7);
    Material_RegisterHandle("hudscoreboardscroll_uparrow", 7);
    Material_RegisterHandle("hudscoreboardscroll_upkey", 7);
    Material_RegisterHandle("hudscoreboardscroll_downarrow", 7);
    Material_RegisterHandle("hudscoreboardscroll_downkey", 7);
    Material_RegisterHandle("voice_on", 7);
    Material_RegisterHandle("voice_off", 7);
}

bool __cdecl Scoreboard_HandleInput(int localClientNum, int key)
{
    bool result; // al
    cg_s *cgameGlob; // [esp+4h] [ebp-4h]

    if ( CL_GetLocalClientConnectionState(localClientNum) < 10 )
        return 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    switch ( key )
    {
        case 20:
        case 164:
        case 184:
        case 206:
            CG_ScrollScoreboardUp(cgameGlob);
            result = 1;
            break;
        case 21:
        case 163:
        case 190:
        case 205:
            CG_ScrollScoreboardDown(cgameGlob);
            result = 1;
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

bool __cdecl MuteError_HandleInput(int localClientNum)
{
    CL_GetLocalClientConnectionState(localClientNum);
    return 0;
}

int __cdecl CG_GetPlaceWithTiesForScore(int localClientNum, int score)
{
    unsigned int clientNum; // [esp+0h] [ebp-10h]
    int i; // [esp+4h] [ebp-Ch]
    cg_s *cgameGlob; // [esp+8h] [ebp-8h]
    int place; // [esp+Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    place = 1;
    for ( i = 0; i < cgameGlob->numScores; ++i )
    {
        clientNum = cgameGlob->scoreOrder[i];
        if ( clientNum <= 0x20 && cgameGlob->bgs.clientinfo[clientNum].infoValid )
        {
            if ( cgameGlob->bgs.clientinfo[clientNum].score.score <= score )
                return place;
            ++place;
        }
    }
    return place;
}

int __cdecl CG_GetKills(int localClientNum, const score_s *score)
{
    return CG_GetColumnValue(localClientNum, score, SB_TYPE_KILLS);
}

int __cdecl CG_GetDeaths(int localClientNum, const score_s *score)
{
    return CG_GetColumnValue(localClientNum, score, SB_TYPE_DEATHS);
}

