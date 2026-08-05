#ifdef KISAK_DW

#include "MatchRecorder.h"
#include <win32/win_tasks.h>
#include <live/live_combatrecord.h>
#include <universal/mem_largelocal.h>
#include <live/live_stats.h>
#include <bgame/bg_unlockable_items.h>
#include <game_mp/g_main_mp.h>
#include <DemonWare/bdLobbyService.h>
#include "dwUtils.h"
#include <ui/ui_playlists.h>
#include <live/live_fileshare.h>
#include <live/live_win.h>
#include <client_mp/sv_client_mp.h>
#include <server/sv_live_stats.h>
#include "dwUtils_pc.h"
#include <server_mp/sv_main_mp.h>
#include <game/g_weapon.h>
#include <bgame/bg_weapons_def.h>
#include "dwRecordEvent.h"
#include <qcommon/com_gamemodes.h>

overlappedTask overlappedTasks_0[32];
char s_matchRecorderBuffer[66560];
ddlDef_t *g_matchRecorderDDL;
unsigned __int8 s_itemIndexes[85];

ddlState_t g_RootState;
ddlState_t g_HeaderState;
ddlState_t g_playerBufferState;
ddlState_t g_lifeBufState;
ddlState_t g_UploadBandwitdh;
ddlState_t g_ConnectionID;
ddlState_t g_TimePlayedTotal;
ddlState_t g_Rank;
ddlState_t g_LastConsoleSave;
ddlState_t g_ConsoleInfo;

void __cdecl MatchRecorder_ClearDWOverlappedTasks()
{
    TaskManager_ClearOverlappedTasks(overlappedTasks_0);
}

void __cdecl MatchRecord_InitMatchData()
{
    MatchRecord_InitMatchDataInternal(0, 0);
}

int __cdecl MatchRecord_InitMatchDataInternal(char *inputBuffer, int buffSize)
{
    const ddlState_t *RootDDLState; // eax
    const ddlState_t *v4; // eax
    const ddlState_t *v5; // eax
    const ddlState_t *v6; // eax
    const ddlState_t *v7; // eax
    const ddlState_t *v8; // eax
    loadoutSlot_t loadoutSlot; // [esp+4h] [ebp-10h]
    int i; // [esp+8h] [ebp-Ch]
    int itemIndex; // [esp+Ch] [ebp-8h]
    int res; // [esp+10h] [ebp-4h]

    res = 0;
    if ( inputBuffer && buffSize )
    {
        if ( buffSize < 66560 )
        {
            Com_PrintError(16, "Match data is %d bytes and supplied buffer is only %d bytes\n", 66560, buffSize);
            return 0;
        }
        memcpy((unsigned __int8 *)s_matchRecorderBuffer, (unsigned __int8 *)inputBuffer, sizeof(s_matchRecorderBuffer));
    }
    else
    {
        Com_Printf(16, "Received NULL inputBuffer in MatchRecord_InitMatchData setting static buffer to 0\n");
        memset(s_matchRecorderBuffer, 0, sizeof(s_matchRecorderBuffer));
    }
    if ( !useFastFile->current.enabled )
        return 0;
    g_matchRecorderDDL = DDL_LoadAsset("ddl_mp/match_record.ddl");
    if ( g_matchRecorderDDL )
    {
        res = DDL_AssociateBuffer(s_matchRecorderBuffer, 66560, g_matchRecorderDDL);
        DDL_Reset(&g_RootState, g_matchRecorderDDL);
        DDL_Reset(&g_HeaderState, g_matchRecorderDDL);
        DDL_Reset(&g_playerBufferState, g_matchRecorderDDL);
        DDL_Reset(&g_lifeBufState, g_matchRecorderDDL);
        DDL_MoveToName(&g_RootState, &g_RootState, "s_matchRecordData");
        DDL_MoveToName(&g_RootState, &g_HeaderState, "header");
        DDL_MoveToName(&g_RootState, &g_playerBufferState, "playerBuffer");
        DDL_MoveToName(&g_RootState, &g_lifeBufState, "lifeBuffer");
        RootDDLState = LiveStats_GetRootDDLState();
        DDL_MoveToName(RootDDLState, &g_ConnectionID, "connectionID");
        v4 = LiveStats_GetRootDDLState();
        DDL_MoveToName(v4, &g_UploadBandwitdh, "uploadBandwidth");
        v5 = LiveStats_GetRootDDLState();
        DDL_MoveToName(v5, &g_TimePlayedTotal, "PlayerStatsList");
        DDL_MoveToName(&g_TimePlayedTotal, &g_TimePlayedTotal, "TIME_PLAYED_TOTAL");
        v6 = LiveStats_GetRootDDLState();
        DDL_MoveToName(v6, &g_Rank, "PlayerStatsList");
        DDL_MoveToName(&g_Rank, &g_Rank, "RANK");
        v7 = LiveStats_GetRootDDLState();
        DDL_MoveToName(v7, &g_LastConsoleSave, "LastConsoleSave");
        v8 = LiveStats_GetRootDDLState();
        DDL_MoveToName(v8, &g_ConsoleInfo, "ConsoleInfo");
    }
    if ( !inputBuffer || !buffSize )
    {
        MatchRecorderDDLSetInt(&g_HeaderState, "titleID", 0);
        itemIndex = 0;
        for ( i = 0; i < 256 && itemIndex < 85; ++i )
        {
            if ( BG_UnlockablesIsItemValidNotNull(i) )
            {
                loadoutSlot = (loadoutSlot_t)BG_UnlockablesGetItemLoadoutSlot(i);
                if ( loadoutSlot == LOADOUTSLOT_FIRST
                    || loadoutSlot == LOADOUTSLOT_SECONDARY_WEAPON
                    || loadoutSlot == LOADOUTSLOT_GLOBAL_ITEMS_START
                    || loadoutSlot == LOADOUTSLOT_KILLSTREAK2
                    || loadoutSlot == LOADOUTSLOT_KILLSTREAK3
                    || loadoutSlot == LOADOUTSLOT_PRIMARY_GRENADE
                    || loadoutSlot == LOADOUTSLOT_SPECIAL_GRENADE )
                {
                    s_itemIndexes[itemIndex++] = i;
                }
            }
        }
    }
    return res;
}

void __cdecl MatchRecorderDDLSetInt(ddlState_t *searchState, const char *name, unsigned int value)
{
    ddlState_t tempState; // [esp+0h] [ebp-10h] BYREF

    if ( !searchState
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DW\\MatchRecorder.cpp",
                    211,
                    0,
                    "%s",
                    "searchState != NULL") )
    {
        __debugbreak();
    }
    if ( searchState )
    {
        if ( DDL_MoveToName(searchState, &tempState, name) )
            DDL_SetInt(&tempState, value, s_matchRecorderBuffer);
        else
            Com_PrintError(16, "Unable to navigate to correct leaf in matchrecorder");
    }
    else
    {
        Com_PrintError(16, "received NULL search state");
    }
}

bool __cdecl MatchRecord_IsHeatMapDataGenerated()
{
    ddlState_t tempState; // [esp+4h] [ebp-10h] BYREF

    if ( !g_matchRecorderDDL )
        return 0;
    DDL_MoveToName(&g_RootState, &tempState, "heatMapDataGenerated");
    return DDL_GetInt(&tempState, s_matchRecorderBuffer) == 1;
}

unsigned int __cdecl MatchRecord_GetMapIndex()
{
    ddlState_t tempState; // [esp+0h] [ebp-10h] BYREF

    DDL_MoveToName(&g_HeaderState, &tempState, "mapID");
    return DDL_GetInt(&tempState, s_matchRecorderBuffer);
}

unsigned int __cdecl MatchRecord_GetXUIDForPlayerSlot(unsigned int playerSlot)
{
    ddlState_t tempState; // [esp+0h] [ebp-10h] BYREF

    if ( MatchRecord_GetPlayerInSlot(playerSlot, &tempState) )
        return MatchRecorderDDLGetInt64(&tempState, "xuid");
    else
        return 0;
}

unsigned int __cdecl MatchRecorderDDLGetInt64(ddlState_t *searchState, const char *name)
{
    ddlState_t tempState; // [esp+0h] [ebp-10h] BYREF

    if ( !searchState
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DW\\MatchRecorder.cpp",
                    270,
                    0,
                    "%s",
                    "searchState != NULL") )
    {
        __debugbreak();
    }
    if ( searchState )
    {
        if ( DDL_MoveToName(searchState, &tempState, name) )
        {
            return DDL_GetInt64(&tempState, s_matchRecorderBuffer);
        }
        else
        {
            Com_PrintError(16, "Unable to navigate to correct leaf in matchrecorder");
            return 0;
        }
    }
    else
    {
        Com_PrintError(16, "received NULL search state");
        return 0;
    }
}

char __cdecl MatchRecord_GetPlayerInSlot(unsigned int playerSlot, ddlState_t *playerSlotState)
{
    if ( (int)playerSlot >= 50
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DW\\MatchRecorder.cpp",
                    608,
                    0,
                    "%s",
                    "playerSlot < MAX_RECORDED_PLAYERS") )
    {
        __debugbreak();
    }
    if ( playerSlot > 0x31 )
        return 0;
    DDL_MoveToIndex(&g_playerBufferState, playerSlotState, playerSlot, 1);
    return 1;
}

void __cdecl SaveTeamScores()
{
    if ( g_matchRecorderDDL )
    {
        MatchRecorderDDLSetInt(&g_HeaderState, "alliesScore", level.teamScores[2]);
        MatchRecorderDDLSetInt(&g_HeaderState, "axisScore", level.teamScores[1]);
    }
}

void __cdecl GenerateRecordedMatchData(int controllerIndex, char **matchRecordBuffer, int *matchRecordBufferSize)
{
    int compressedSize; // [esp+4h] [ebp-18h] BYREF
    int bufferSize; // [esp+8h] [ebp-14h]
    char (*tempBuffer)[66560]; // [esp+Ch] [ebp-10h]
    bdLobbyService *lobby; // [esp+10h] [ebp-Ch]

    if ( g_matchRecorderDDL )
    {
        *matchRecordBuffer = 0;
        *matchRecordBufferSize = 0;

        LargeLocal tempBuffer_large_local(66560); // [esp+14h] [ebp-8h] BYREF

        //LargeLocal::LargeLocal(&tempBuffer_large_local, 66560);
        tempBuffer = (char (*)[66560])tempBuffer_large_local.GetBuf();// LargeLocal::GetBuf(&tempBuffer_large_local);
        Com_Printf(16, "Generating COD7 match data.\n");
        lobby = dwGetLobby(controllerIndex);
        GenerateBinaryMatchSummary(lobby->m_titleID);
        GenerateBinaryBreadcrumbData();
        SaveTeamScores();
        compressedSize = 66560;
        if ( !CompressBuffer((char *)tempBuffer, (unsigned int *)&compressedSize, s_matchRecorderBuffer, 0x10400u) )
            Com_Printf(16, "Compression of COD7 xml match data failed.\n");
        *matchRecordBuffer = LiveStorage_GetMatchRecordBuffer();
        bufferSize = LiveStorage_GetMatchRecordBufferSize();
        if ( *matchRecordBuffer )
        {
            if ( bufferSize >= compressedSize )
            {
                memcpy((unsigned __int8 *)*matchRecordBuffer, (unsigned __int8 *)tempBuffer, compressedSize);
                *matchRecordBufferSize = compressedSize;
            }
            else
            {
                Com_PrintError(
                    16,
                    "Compressed Match Data is %d and buffer is %d - not enough space\n",
                    compressedSize,
                    bufferSize);
            }
        }
        else
        {
            Com_PrintError(16, "Content Server already in use - cannot upload match data\n");
        }
        ClearTeamScores();
        //LargeLocal::~LargeLocal(&tempBuffer_large_local);
    }
}

bool __cdecl CompressBuffer(
                char *destBuffer,
                unsigned int *destBufferLen,
                char *sourceBuffer,
                unsigned int sourceBufferLen)
{
    *destBufferLen = MSG_CompressWithZLib(
                                         (unsigned __int8 *)sourceBuffer,
                                         sourceBufferLen,
                                         (unsigned __int8 *)destBuffer,
                                         *destBufferLen);
    return *destBufferLen != 0;
}

void __cdecl GenerateBinaryMatchSummary(unsigned int titleID)
{
    unsigned int CurrentMapIndex; // eax
    unsigned int CurrentPlaylistIndex; // eax
    signed int playerSlot; // [esp+0h] [ebp-18h]
    unsigned int start; // [esp+4h] [ebp-14h]
    unsigned int end; // [esp+8h] [ebp-10h]
    __int16 playerCount; // [esp+10h] [ebp-8h]
    unsigned int joined; // [esp+14h] [ebp-4h]

    MatchRecorderDDLSetInt(&g_HeaderState, "titleID", titleID);
    CurrentMapIndex = GetCurrentMapIndex();
    MatchRecorderDDLSetInt(&g_HeaderState, "mapID", CurrentMapIndex);
    MatchRecorderDDLSetInt(&g_HeaderState, "mapDuration", level.g_msgTime / 1000);
    CurrentPlaylistIndex = GetCurrentPlaylistIndex();
    MatchRecorderDDLSetInt(&g_HeaderState, "playlistID", CurrentPlaylistIndex);
    start = MatchRecorderDDLGetInt(&g_HeaderState, "playerCountStart");
    joined = MatchRecorderDDLGetInt(&g_HeaderState, "playerCountJoined");
    end = joined + start - MatchRecorderDDLGetInt(&g_HeaderState, "playerCountLeft");
    MatchRecorderDDLSetInt(&g_HeaderState, "playerCountEnd", end);
    SaveTeamScores();
    playerCount = MatchRecord_GetPlayerCount();
    for ( playerSlot = 0; playerSlot < playerCount; ++playerSlot )
    {
        FindBitchForPlayer(playerSlot);
        FindNemesisForPlayer(playerSlot);
    }
}

unsigned int __cdecl MatchRecorderDDLGetInt(ddlState_t *searchState, const char *name)
{
    ddlState_t tempState; // [esp+0h] [ebp-10h] BYREF

    if ( !searchState
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DW\\MatchRecorder.cpp",
                    231,
                    0,
                    "%s",
                    "searchState != NULL") )
    {
        __debugbreak();
    }
    if ( searchState )
    {
        if ( DDL_MoveToName(searchState, &tempState, name) )
        {
            return DDL_GetInt(&tempState, s_matchRecorderBuffer);
        }
        else
        {
            Com_PrintError(16, "Unable to navigate to correct leaf in matchrecorder");
            return 0;
        }
    }
    else
    {
        Com_PrintError(16, "received NULL search state");
        return 0;
    }
}

unsigned int __cdecl MatchRecord_GetPlayerCount()
{
    ddlState_t tempState; // [esp+0h] [ebp-10h] BYREF

    DDL_MoveToName(&g_HeaderState, &tempState, "playerCount");
    return DDL_GetInt(&tempState, s_matchRecorderBuffer);
}

int __cdecl GetCurrentMapIndex()
{
    const char *ColumnValueForRow; // eax
    const StringTable *mapTable; // [esp+0h] [ebp-Ch] BYREF
    int row; // [esp+4h] [ebp-8h]
    const char *mapname; // [esp+8h] [ebp-4h]

    StringTable_GetAsset("mp/mapsTable.csv", (XAssetHeader *)&mapTable);
    if ( !mapTable
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DW\\MatchRecorder.cpp", 726, 0, "%s", "mapTable") )
    {
        __debugbreak();
    }
    mapname = Dvar_GetString("mapname");
    row = StringTable_LookupRowNumForValue(mapTable, 0, mapname);
    if ( row < 0 )
        return -1;
    ColumnValueForRow = StringTable_GetColumnValueForRow(mapTable, row, 5);
    return atoi(ColumnValueForRow);
}

int __cdecl GetCurrentPlaylistIndex()
{
    return Playlist_GetPlaylistIdForNum(0, playlist->current.integer);
}

void __cdecl FindBitchForPlayer(unsigned int attackingPlayerSlot)
{
    int i; // [esp+0h] [ebp-10Ch]
    __int16 playerFromState; // [esp+4h] [ebp-108h]
    ddlState_t lifeState; // [esp+8h] [ebp-104h] BYREF
    char attackerFromState; // [esp+1Bh] [ebp-F1h]
    int lifeNum; // [esp+1Ch] [ebp-F0h]
    int playerNum; // [esp+20h] [ebp-ECh]
    int playerKills[50]; // [esp+24h] [ebp-E8h] BYREF
    int maxPlayerKills; // [esp+ECh] [ebp-20h]
    __int16 playerCount; // [esp+F0h] [ebp-1Ch]
    int playerBitch; // [esp+F4h] [ebp-18h]
    __int16 lifeCount; // [esp+F8h] [ebp-14h]
    ddlState_t playerState; // [esp+FCh] [ebp-10h] BYREF

    memset(playerKills, 0, sizeof(playerKills));
    playerCount = MatchRecord_GetPlayerCount();
    lifeCount = MatchRecord_GetLifeCount();
    for ( playerNum = 0; playerNum < playerCount; ++playerNum )
    {
        if ( playerNum != attackingPlayerSlot )
        {
            for ( lifeNum = 0; lifeNum < lifeCount; ++lifeNum )
            {
                MatchRecord_GetLifeInSlot(lifeNum, &lifeState);
                playerFromState = MatchRecorderDDLGetInt(&lifeState, "player");
                attackerFromState = MatchRecorderDDLGetInt(&lifeState, "attacker");
                if ( playerFromState == playerNum && attackerFromState == attackingPlayerSlot )
                    ++playerKills[playerNum];
            }
        }
    }
    playerBitch = -1;
    maxPlayerKills = 0;
    for ( i = 0; i < (__int16)MatchRecord_GetPlayerCount(); ++i )
    {
        if ( playerKills[i] > maxPlayerKills )
        {
            playerBitch = i;
            maxPlayerKills = playerKills[i];
        }
    }
    if ( MatchRecord_GetPlayerInSlot(attackingPlayerSlot, &playerState) && playerBitch != -1 )
    {
        MatchRecorderDDLSetInt(&playerState, "bitch", playerBitch);
        MatchRecorderDDLSetInt(&playerState, "bitchKills", maxPlayerKills);
    }
}

unsigned int __cdecl MatchRecord_GetLifeCount()
{
    ddlState_t tempState; // [esp+0h] [ebp-10h] BYREF

    DDL_MoveToName(&g_HeaderState, &tempState, "lifeCount");
    return DDL_GetInt(&tempState, s_matchRecorderBuffer);
}

char __cdecl MatchRecord_GetLifeInSlot(unsigned int lifeSlot, ddlState_t *lifeSlotState)
{
    if ( (int)lifeSlot >= 1000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DW\\MatchRecorder.cpp",
                    648,
                    0,
                    "%s",
                    "lifeSlot < MAX_RECORDED_LIVES") )
    {
        __debugbreak();
    }
    if ( lifeSlot >= 0x3E8 )
        return 0;
    DDL_MoveToIndex(&g_lifeBufState, lifeSlotState, lifeSlot, 1);
    return 1;
}

void __cdecl FindNemesisForPlayer(unsigned int attackingPlayerSlot)
{
    int i; // [esp+0h] [ebp-2B0h]
    __int16 playerFromState; // [esp+4h] [ebp-2ACh]
    ddlState_t lifeState; // [esp+8h] [ebp-2A8h] BYREF
    char attackerFromState; // [esp+1Bh] [ebp-295h]
    int lifeNum; // [esp+1Ch] [ebp-294h]
    int playerNum; // [esp+20h] [ebp-290h]
    float maxNemesisScore; // [esp+24h] [ebp-28Ch]
    int nemesisDeaths[50]; // [esp+28h] [ebp-288h] BYREF
    int nemesisKills[50]; // [esp+F0h] [ebp-1C0h] BYREF
    ddlState_t nemesisState; // [esp+1BCh] [ebp-F4h] BYREF
    __int16 playerCount; // [esp+1CCh] [ebp-E4h]
    float nemesisScore[50]; // [esp+1D0h] [ebp-E0h] BYREF
    __int16 lifeCount; // [esp+298h] [ebp-18h]
    int playerNemesis; // [esp+29Ch] [ebp-14h]
    ddlState_t playerState; // [esp+2A0h] [ebp-10h] BYREF

    memset(nemesisScore, 0, sizeof(nemesisScore));
    memset(nemesisDeaths, 0, sizeof(nemesisDeaths));
    memset(nemesisKills, 0, sizeof(nemesisKills));
    playerCount = MatchRecord_GetPlayerCount();
    lifeCount = MatchRecord_GetLifeCount();
    for ( playerNum = 0; playerNum < playerCount; ++playerNum )
    {
        for ( lifeNum = 0; lifeNum < lifeCount; ++lifeNum )
        {
            MatchRecord_GetLifeInSlot(lifeNum, &lifeState);
            if ( MatchRecord_GetLifeInSlot(lifeNum, &lifeState) )
            {
                playerFromState = MatchRecorderDDLGetInt(&lifeState, "player");
                attackerFromState = MatchRecorderDDLGetInt(&lifeState, "attacker");
                if ( playerFromState == attackingPlayerSlot && attackerFromState == playerNum )
                {
                    nemesisScore[playerNum] = nemesisScore[playerNum] + 1.5;
                    ++nemesisDeaths[playerNum];
                }
                if ( playerFromState == playerNum && attackerFromState == attackingPlayerSlot )
                {
                    nemesisScore[playerNum] = nemesisScore[playerNum] + 1.0;
                    ++nemesisKills[playerNum];
                }
            }
        }
    }
    playerNemesis = -1;
    maxNemesisScore = 0.0f;
    for ( i = 0; i < playerCount; ++i )
    {
        if ( i != attackingPlayerSlot && nemesisScore[i] > maxNemesisScore )
        {
            playerNemesis = i;
            maxNemesisScore = nemesisScore[i];
        }
    }
    if ( MatchRecord_GetPlayerInSlot(attackingPlayerSlot, &playerState) )
        MatchRecorderDDLSetInt(&playerState, "nemesis", playerNemesis);
    if ( MatchRecord_GetPlayerInSlot(playerNemesis, &nemesisState) && playerNemesis != -1 )
    {
        MatchRecorderDDLSetInt(&nemesisState, "nemesisDeaths", nemesisDeaths[playerNemesis]);
        MatchRecorderDDLSetInt(&nemesisState, "nemesisKills", nemesisKills[playerNemesis]);
    }
}

void GenerateBinaryBreadcrumbData()
{
    ddlState_t breadCrumbHeaderState; // [esp+0h] [ebp-14h] BYREF
    int breadcrumbDataLength; // [esp+10h] [ebp-4h]

    DDL_MoveToName(&g_RootState, &breadCrumbHeaderState, "breadCrumbHeader");
    MatchRecorderDDLSetInt(&breadCrumbHeaderState, "frequency", 0xB4u);
    breadcrumbDataLength = (level.g_msgTime / 1000 / 18) << 6;
    MatchRecorderDDLSetInt(&breadCrumbHeaderState, "dataLength", breadcrumbDataLength);
}

void ClearTeamScores()
{
    MatchRecorderDDLSetInt(&g_HeaderState, "alliesScore", 0);
    MatchRecorderDDLSetInt(&g_HeaderState, "axisScore", 0);
}

void __cdecl MatchRecord_NormaliseHeatMap(
                int width,
                int height,
                unsigned __int8 *heatMap,
                unsigned __int8 *buffer,
                float maxVal)
{
    unsigned __int8 value; // [esp+3h] [ebp-Dh]
    int xPos; // [esp+4h] [ebp-Ch]
    int yOffset; // [esp+8h] [ebp-8h]
    int yPos; // [esp+Ch] [ebp-4h]

    for ( yPos = 0; yPos < height; ++yPos )
    {
        yOffset = width * yPos;
        for ( xPos = 0; xPos < width; ++xPos )
        {
            value = (int)(float)((float)((float)heatMap[xPos + yOffset] / maxVal) * 255.0);
            if ( value )
                buffer[xPos + yOffset] = value;
        }
    }
}

unsigned __int8 __cdecl MatchRecord_ReadHeatMapPixel(
                int xPos,
                int yPos,
                int width,
                int height,
                unsigned __int8 *buffer)
{
    if ( xPos < width && xPos >= 0 && yPos < height && yPos >= 0 )
        return buffer[xPos + width * yPos];
    else
        return 0;
}

void __cdecl MatchRecord_WriteHeatMapPixel(
                int xPos,
                int yPos,
                int width,
                int height,
                unsigned __int8 *buffer,
                unsigned __int8 pixelVal)
{
    if ( pixelVal && xPos < width && xPos >= 0 && yPos < height && yPos >= 0 )
        buffer[xPos + width * yPos] = pixelVal;
}

void __cdecl MatchRecord_GenerateHeatMapData(
                unsigned __int8 *buffer,
                int buffSize,
                int width,
                int height,
                unsigned __int64 xuid)
{
    int v5; // edx
    float maxVal; // xmm0_4
    float v7; // [esp+10h] [ebp-5Ch]
    __int64 v8; // [esp+14h] [ebp-58h]
    int numBlurs; // [esp+24h] [ebp-48h]
    int j; // [esp+28h] [ebp-44h]
    unsigned __int8 victimY; // [esp+2Eh] [ebp-3Eh]
    unsigned __int8 victimX; // [esp+2Fh] [ebp-3Dh]
    unsigned int player; // [esp+30h] [ebp-3Ch]
    ddlState_t lifeState; // [esp+34h] [ebp-38h] BYREF
    int i; // [esp+44h] [ebp-28h]
    float xScale; // [esp+48h] [ebp-24h]
    float yScale; // [esp+4Ch] [ebp-20h]
    float maxKills; // [esp+58h] [ebp-14h]
    int newY; // [esp+5Ch] [ebp-10h]
    int arrayIndex; // [esp+60h] [ebp-Ch]
    int newX; // [esp+64h] [ebp-8h]
    unsigned __int8 (*heatMap)[65536]; // [esp+68h] [ebp-4h]

    if ( g_matchRecorderDDL )
    {
        LargeLocal heatMap_large_local(0x10000); // [esp+50h] [ebp-1Ch] BYREF

        //LargeLocal::LargeLocal(&heatMap_large_local, 0x10000);
        heatMap = (unsigned __int8 (*)[65536])heatMap_large_local.GetBuf();// LargeLocal::GetBuf(&heatMap_large_local);
        memset(heatMap, 0, sizeof(unsigned __int8[65536]));
        if ( buffSize < height * width
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\DW\\MatchRecorder.cpp",
                        1233,
                        0,
                        "%s",
                        "buffSize >= width * height") )
        {
            __debugbreak();
        }
        memset(buffer, 0, buffSize);
        maxKills = 0.0f;
        arrayIndex = 0;
        newX = 0;
        newY = 0;
        if ( (__int16)MatchRecord_GetLifeCount() > 0 )
        {
            xScale = 256.0 / (float)width;
            yScale = 256.0 / (float)height;
            for ( i = 0; i < (__int16)MatchRecord_GetLifeCount(); ++i )
            {
                if ( MatchRecord_GetLifeInSlot(i, &lifeState) )
                {
                    victimX = MatchRecorderDDLGetInt(&lifeState, "victimX");
                    victimY = MatchRecorderDDLGetInt(&lifeState, "victimY");
                    player = MatchRecorderDDLGetInt(&lifeState, "player");
                    if ( victimY != 255 && victimX != 255 )
                    {
                        if ( !xuid || (LODWORD(v8) = MatchRecord_GetXUIDForPlayerSlot(player), HIDWORD(v8) = v5, xuid == v8) )
                        {
                            if ( xuid )
                            {
                                for ( j = -1; j <= 1; ++j )
                                {
                                    MatchRecord_SetPlayerDeathPosition(buffer, width, height, xScale, yScale, victimX, j + victimY);
                                    MatchRecord_SetPlayerDeathPosition(buffer, width, height, xScale, yScale, j + victimX, victimY);
                                }
                            }
                            else
                            {
                                newX = (int)(float)((float)victimX / xScale);
                                newY = (int)(float)((float)victimY / yScale);
                                arrayIndex = newX + width * newY;
                                if ( (float)(maxKills - (float)++(*heatMap)[arrayIndex]) < 0.0 )
                                    v7 = (float)(*heatMap)[arrayIndex];
                                else
                                    v7 = maxKills;
                                maxKills = v7;
                            }
                        }
                    }
                }
            }
            if ( !xuid )
            {
                MatchRecord_NormaliseHeatMap(width, height, (unsigned __int8 *)heatMap, buffer, maxKills);
                for ( numBlurs = 0; numBlurs < 3; ++numBlurs )
                {
                    maxVal = (float)MatchRecord_BlurHeatMap(width, height, buffer, buffSize);
                    MatchRecord_NormaliseHeatMap(width, height, buffer, buffer, maxVal);
                }
            }
        }
        MatchRecorderDDLSetInt(&g_RootState, "heatMapDataGenerated", 1u);
        //LargeLocal::~LargeLocal(&heatMap_large_local);
    }
}

unsigned __int8 __cdecl MatchRecord_BlurHeatMap(
                int width,
                int height,
                unsigned __int8 *buffer,
                unsigned int bufferSize)
{
    unsigned __int8 v5; // [esp+0h] [ebp-4Ch]
    int v6; // [esp+4h] [ebp-48h]
    unsigned __int8 v7; // [esp+8h] [ebp-44h]
    int v8; // [esp+Ch] [ebp-40h]
    unsigned __int8 v9; // [esp+1Bh] [ebp-31h]
    int yOffset; // [esp+1Ch] [ebp-30h]
    int v11; // [esp+20h] [ebp-2Ch]
    int v12; // [esp+20h] [ebp-2Ch]
    int j; // [esp+24h] [ebp-28h]
    int i; // [esp+28h] [ebp-24h]
    int xOffset; // [esp+2Ch] [ebp-20h]
    int pixelVal; // [esp+30h] [ebp-1Ch]
    int pixelVala; // [esp+30h] [ebp-1Ch]
    int xPos; // [esp+34h] [ebp-18h]
    int yPos; // [esp+38h] [ebp-14h]
    unsigned __int8 *tempHeatMap; // [esp+3Ch] [ebp-10h]
    unsigned __int8 maxVal; // [esp+4Bh] [ebp-1h]

    LargeLocal tempHeatMap_large_local(0x10000); // [esp+40h] [ebp-Ch] BYREF
    //LargeLocal::LargeLocal(&tempHeatMap_large_local, 0x10000);
    tempHeatMap = tempHeatMap_large_local.GetBuf();// LargeLocal::GetBuf(&tempHeatMap_large_local);
    maxVal = 0;
    memset(tempHeatMap, 0, 0x10000u);
    for ( yPos = 0; yPos < height; ++yPos )
    {
        for ( xPos = 0; xPos < width; ++xPos )
        {
            pixelVal = 0;
            for ( xOffset = -1; xOffset <= 1; ++xOffset )
                pixelVal += MatchRecord_ReadHeatMapPixel(xOffset + xPos, yPos, width, height, buffer);
            pixelVala = pixelVal / 2;
            if ( pixelVala < 255 )
                v8 = pixelVala;
            else
                v8 = 255;
            if ( maxVal < v8 )
                v7 = v8;
            else
                v7 = maxVal;
            maxVal = v7;
            MatchRecord_WriteHeatMapPixel(xPos, yPos, width, height, tempHeatMap, v8);
        }
    }
    memcpy(buffer, tempHeatMap, bufferSize);
    for ( i = 0; i < width; ++i )
    {
        for ( j = 0; j < height; ++j )
        {
            v11 = 0;
            for ( yOffset = -1; yOffset <= 1; ++yOffset )
                v11 += MatchRecord_ReadHeatMapPixel(i, yOffset + j, width, height, buffer);
            v12 = v11 / 2;
            if ( v12 < 255 )
                v6 = v12;
            else
                v6 = 255;
            if ( maxVal < v6 )
                v5 = v6;
            else
                v5 = maxVal;
            maxVal = v5;
            MatchRecord_WriteHeatMapPixel(i, j, width, height, tempHeatMap, v6);
        }
    }
    memcpy(buffer, tempHeatMap, bufferSize);
    v9 = maxVal;
    //LargeLocal::~LargeLocal(&tempHeatMap_large_local);
    return v9;
}

void __cdecl MatchRecord_SetPlayerDeathPosition(
                unsigned __int8 *buffer,
                int width,
                int height,
                float xScale,
                float yScale,
                int xPos,
                int yPos)
{
    int arrayIndex; // [esp+8h] [ebp-4h]

    arrayIndex = (int)(float)((float)xPos / xScale) + width * (int)(float)((float)yPos / yScale);
    if ( arrayIndex >= 0 || arrayIndex <= height * width )
        buffer[arrayIndex] = -1;
}

void __cdecl MatchRecord_GeneratePooledFileDetails(int controllerIndex)
{
    unsigned __int64 v1; // rax
    __int64 v2; // rax
    int Int; // eax
    int v4; // eax
    int v5; // eax
    int v6; // eax
    int v7; // eax
    unsigned int v8; // eax
    ddlState_t playerState; // [esp+8h] [ebp-48h] BYREF
    ddlState_t playerItemState; // [esp+18h] [ebp-38h] BYREF
    int i; // [esp+28h] [ebp-28h]
    fileSharePooledFileContext_t context; // [esp+2Ch] [ebp-24h]
    fileSharePooledDetails_t *fileDetails; // [esp+30h] [ebp-20h]
    ddlState_t headerItemState; // [esp+34h] [ebp-1Ch] BYREF
    int numPlayers; // [esp+44h] [ebp-Ch]
    unsigned __int64 xuid; // [esp+48h] [ebp-8h]

    fileDetails = Live_FileShare_GetFilePooledDetailsBuffer();
    xuid = 0;
    if ( fileDetails->context == FILESHARE_POOLEDFILEDETAILS_SELF )
    {
        LODWORD(v1) = Live_GetXuid(controllerIndex);
        xuid = v1;
    }
    else if ( fileDetails->context == FILESHARE_POOLEDFILEDETAILS_FRIEND
                 && selectedPlayerXuid
                 && selectedPlayerXuid->current.integer
                 && *(_BYTE *)selectedPlayerXuid->current.integer )
    {
        xuid = I_atoi64(selectedPlayerXuid->current.string);
    }
    if ( xuid )
    {
        context = fileDetails->context;
        memset(fileDetails, 0, sizeof(fileSharePooledDetails_t));
        fileDetails->context = context;
        numPlayers = 0;
        if ( DDL_MoveToName(&g_HeaderState, &headerItemState, "mapID") )
            fileDetails->mapID = DDL_GetInt(&headerItemState, s_matchRecorderBuffer);
        if ( DDL_MoveToName(&g_HeaderState, &headerItemState, "playerCount") )
            numPlayers = DDL_GetInt(&headerItemState, s_matchRecorderBuffer);
        if ( DDL_MoveToName(&g_HeaderState, &headerItemState, "alliesScore") )
            fileDetails->alliesScore = DDL_GetInt(&headerItemState, s_matchRecorderBuffer);
        if ( DDL_MoveToName(&g_HeaderState, &headerItemState, "axisScore") )
            fileDetails->axisScore = DDL_GetInt(&headerItemState, s_matchRecorderBuffer);
        if ( DDL_MoveToName(&g_HeaderState, &headerItemState, "isWager") )
            fileDetails->isWager = DDL_GetInt(&headerItemState, s_matchRecorderBuffer);
        for ( i = 0; i < numPlayers; ++i )
        {
            DDL_MoveToName(&g_RootState, &playerState, "playerBuffer");
            DDL_MoveToIndex(&playerState, &playerState, i, 1);
            if ( DDL_MoveToName(&playerState, &playerItemState, "xuid") )
            {
                LODWORD(v2) = DDL_GetInt64(&playerItemState, s_matchRecorderBuffer);
                if ( xuid != v2 )
                    continue;
            }
            if ( DDL_MoveToName(&playerState, &playerItemState, "team") )
            {
                Int = DDL_GetInt(&playerItemState, s_matchRecorderBuffer);
                fileDetails->team = Int;
            }
            if ( DDL_MoveToName(&playerState, &playerItemState, "kills") )
            {
                v4 = DDL_GetInt(&playerItemState, s_matchRecorderBuffer);
                fileDetails->kills = v4;
            }
            if ( DDL_MoveToName(&playerState, &playerItemState, "deaths") )
            {
                v5 = DDL_GetInt(&playerItemState, s_matchRecorderBuffer);
                fileDetails->deaths = v5;
            }
            if ( DDL_MoveToName(&playerState, &playerItemState, "wagerPayout") )
            {
                v6 = DDL_GetInt(&playerItemState, s_matchRecorderBuffer);
                fileDetails->wagerPayout = v6;
            }
            if ( DDL_MoveToName(&playerState, &playerItemState, "score") )
            {
                v7 = DDL_GetInt(&playerItemState, s_matchRecorderBuffer);
                fileDetails->score = v7;
            }
            if ( DDL_MoveToName(&playerState, &playerItemState, "position") )
            {
                v8 = DDL_GetInt(&playerItemState, s_matchRecorderBuffer);
                fileDetails->position = v8 + 1;
            }
            fileDetails->isValid = 1;
        }
    }
    else
    {
        Com_PrintError(1, "Invalid XUID for generating pooled file details.\n");
    }
}

int __cdecl MatchRecord_ParseDownloadedMatchData(int controllerIndex, char *inputBuffer, int buffsize)
{
    if ( buffsize == 66560 )
    {
        if ( MatchRecord_InitMatchDataInternal(inputBuffer, 66560) )
        {
            MatchRecord_GeneratePooledFileDetails(controllerIndex);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_PrintError(
            16,
            "size of uncompressed buffer does not match MATCH_RECORDER_SIZE - expecting %d found %d\n",
            66560,
            buffsize);
        return 0;
    }
}

void __cdecl MatchRecordSpawn(gclient_s *client)
{
    unsigned __int64 v1; // rax
    unsigned int UTC; // eax
    unsigned int playerCountJoined; // [esp+18h] [ebp-54h]
    unsigned int upload; // [esp+1Ch] [ebp-50h]
    unsigned int rank; // [esp+2Ch] [ebp-40h]
    unsigned int playtime; // [esp+30h] [ebp-3Ch]
    unsigned int playerSlot; // [esp+38h] [ebp-34h]
    unsigned __int8 spawnZ; // [esp+3Eh] [ebp-2Eh] BYREF
    unsigned __int8 spawnX; // [esp+3Fh] [ebp-2Dh] BYREF
    ddlState_t lifeState; // [esp+40h] [ebp-2Ch] BYREF
    __int16 lifeCount; // [esp+50h] [ebp-1Ch]
    ddlState_t playerState; // [esp+54h] [ebp-18h] BYREF
    int numLives; // [esp+64h] [ebp-8h]
    unsigned __int8 spawnY; // [esp+6Bh] [ebp-1h] BYREF

    if ( g_matchRecorderDDL )
    {
        lifeCount = MatchRecord_GetLifeCount();
        if ( (__int16)MatchRecord_GetPlayerCount() < 50 && lifeCount < 1000 )
        {
            playerSlot = MatchRecord_SearchCreateForPlayer(client, &playerState);
            if ( playerSlot != -1 )
            {
                MatchRecorderDDLSetInt(&playerState, "team", LOBYTE(client->sess.cs.team));
                numLives = MatchRecorderDDLGetInt(&playerState, "num_lives");
                if ( !numLives )
                {
                    MatchRecordCopyExternalIP(playerState, client->ps.clientNum);
                    LODWORD(v1) = SV_GetClientDInt64Stat(client->ps.clientNum, &g_ConnectionID);
                    MatchRecorderDDLSetInt64(&playerState, "connectionid", v1);
                    upload = (SV_GetClientDIntStat(client->ps.clientNum, &g_UploadBandwitdh) >> 3) / 0x412;
                    MatchRecorderDDLSetInt(&playerState, "upload", upload);
                    UTC = LiveStorage_GetUTC();
                    MatchRecorderDDLSetInt(&playerState, "UTCStartTime", UTC);
                    playtime = SV_GetClientDIntStat(client->ps.clientNum, &g_TimePlayedTotal);
                    MatchRecorderDDLSetInt(&playerState, "totalPlaytime", playtime);
                    if ( MatchRecorderDDLGetInt(&g_HeaderState, "playerCountStart") )
                    {
                        playerCountJoined = MatchRecorderDDLGetInt(&g_HeaderState, "playerCountJoined") + 1;
                        MatchRecorderDDLSetInt(&g_HeaderState, "playerCountJoined", playerCountJoined);
                    }
                    rank = SV_GetClientDIntStat(client->ps.clientNum, &g_Rank);
                    MatchRecorderDDLSetInt(&playerState, "rankAtStart", rank);
                    MatchRecorderDDLSetInt(&playerState, "natType", 0);
                    MatchRecordWriteItemStatsStart(playerState, client->ps.clientNum);
                }
                MatchRecorderDDLSetInt(&playerState, "num_lives", ++numLives);
                MatchRecorderDDLSetInt(&playerState, "currentLife", lifeCount);
                if ( MatchRecord_GetLifeInSlot(lifeCount, &lifeState) )
                {
                    MatchRecord_SetLifeCount(++lifeCount);
                    MatchRecorderDDLSetInt(&lifeState, "player", (__int16)playerSlot);
                    MatchRecorderDDLSetInt(&lifeState, "spawn_time", (__int16)(level.g_msgTime / 1000));
                    GetWorldLocation(client->ps.origin[0], client->ps.origin[1], client->ps.origin[2], &spawnX, &spawnY, &spawnZ);
                    MatchRecorderDDLSetInt(&lifeState, "spawnX", spawnX);
                    MatchRecorderDDLSetInt(&lifeState, "spawnY", spawnY);
                    MatchRecorderDDLSetInt(&lifeState, "spawnZ", spawnZ);
                    MatchRecorderDDLSetInt(&lifeState, "weapon", 0);
                    MatchRecorderDDLSetInt(&lifeState, "attacker", 0xFFu);
                    MatchRecorderDDLSetInt(&lifeState, "death_time", 0xFFFFu);
                    MatchRecorderDDLSetInt(&lifeState, "victimX", 0xFFu);
                    MatchRecorderDDLSetInt(&lifeState, "victimY", 0xFFu);
                    MatchRecorderDDLSetInt(&lifeState, "victimZ", 0xFFu);
                }
            }
        }
        else
        {
            Com_PrintError(16, "MatchRecord: maxed out lives buffer\n");
        }
    }
}

void __cdecl MatchRecorderDDLSetInt64(ddlState_t *searchState, const char *name, unsigned __int64 value)
{
    ddlState_t tempState; // [esp+0h] [ebp-10h] BYREF

    if ( !searchState
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DW\\MatchRecorder.cpp",
                    250,
                    0,
                    "%s",
                    "searchState != NULL") )
    {
        __debugbreak();
    }
    if ( searchState )
    {
        if ( DDL_MoveToName(searchState, &tempState, name) )
            DDL_SetInt64(&tempState, value, s_matchRecorderBuffer);
        else
            Com_PrintError(16, "Unable to navigate to correct leaf in matchrecorder");
    }
    else
    {
        Com_PrintError(16, "received NULL search state");
    }
}

void __cdecl MatchRecordCopyExternalIP(ddlState_t playerState, unsigned int clientNum)
{
    unsigned int lastConsoleSave; // [esp+0h] [ebp-34h]
    ddlState_t tempState; // [esp+4h] [ebp-30h] BYREF
    ddlState_t tempIPState; // [esp+14h] [ebp-20h] BYREF
    unsigned int externalIP[4]; // [esp+24h] [ebp-10h]

    lastConsoleSave = SV_GetClientDIntStat(clientNum, &g_LastConsoleSave);
    DDL_MoveToIndex(&g_ConsoleInfo, &tempState, lastConsoleSave, 1);
    DDL_MoveToName(&tempState, &tempState, "ConsoleIPExternal");
    DDL_MoveToName(&tempState, &tempIPState, "a");
    externalIP[0] = SV_GetClientDIntStat(clientNum, &tempIPState);
    DDL_MoveToName(&tempState, &tempIPState, "b");
    externalIP[1] = SV_GetClientDIntStat(clientNum, &tempIPState);
    DDL_MoveToName(&tempState, &tempIPState, "c");
    externalIP[2] = SV_GetClientDIntStat(clientNum, &tempIPState);
    DDL_MoveToName(&tempState, &tempIPState, "d");
    externalIP[3] = SV_GetClientDIntStat(clientNum, &tempIPState);
    DDL_MoveToName(&playerState, &tempState, "externalIP");
    MatchRecorderDDLSetInt(&tempState, "a", externalIP[0]);
    MatchRecorderDDLSetInt(&tempState, "b", externalIP[1]);
    MatchRecorderDDLSetInt(&tempState, "c", externalIP[2]);
    MatchRecorderDDLSetInt(&tempState, "d", externalIP[3]);
    Com_Printf(
        16,
        "MatchRecord: recording ip for %i - %u.%u.%u.%u\n",
        clientNum,
        externalIP[0],
        externalIP[1],
        externalIP[2],
        externalIP[3]);
}

void __cdecl MatchRecordWriteItemStatsStart(ddlState_t playerState, unsigned int clientNum)
{
    const ddlState_t *RootDDLState; // eax
    int statsItemIndex; // [esp+0h] [ebp-58h]
    unsigned int used; // [esp+4h] [ebp-54h]
    unsigned int kills; // [esp+8h] [ebp-50h]
    unsigned int shots; // [esp+Ch] [ebp-4Ch]
    unsigned int hits; // [esp+10h] [ebp-48h]
    unsigned int itemIndex; // [esp+14h] [ebp-44h]
    ddlState_t tempStatsState; // [esp+18h] [ebp-40h] BYREF
    ddlState_t tempState; // [esp+28h] [ebp-30h] BYREF
    ddlState_t tempWeaponState; // [esp+38h] [ebp-20h] BYREF
    ddlState_t tempItemState; // [esp+48h] [ebp-10h] BYREF

    RootDDLState = LiveStats_GetRootDDLState();
    DDL_MoveToName(RootDDLState, &tempStatsState, "ItemStats");
    for ( itemIndex = 0; itemIndex < 0x55; ++itemIndex )
    {
        statsItemIndex = s_itemIndexes[itemIndex];
        DDL_MoveToIndex(&tempStatsState, &tempWeaponState, statsItemIndex, 1);
        DDL_MoveToName(&tempWeaponState, &tempWeaponState, "stats");
        DDL_MoveToName(&tempWeaponState, &tempItemState, "kills");
        DDL_MoveToName(&tempItemState, &tempState, "statValue");
        kills = SV_GetClientDIntStat(clientNum, &tempState);
        DDL_MoveToName(&tempWeaponState, &tempItemState, "shots");
        DDL_MoveToName(&tempItemState, &tempState, "statValue");
        shots = SV_GetClientDIntStat(clientNum, &tempState);
        DDL_MoveToName(&tempWeaponState, &tempItemState, "hits");
        DDL_MoveToName(&tempItemState, &tempState, "statValue");
        hits = SV_GetClientDIntStat(clientNum, &tempState);
        DDL_MoveToName(&tempWeaponState, &tempItemState, "used");
        DDL_MoveToName(&tempItemState, &tempState, "statValue");
        used = SV_GetClientDIntStat(clientNum, &tempState);
        if ( kills || shots || hits || used )
        {
            DDL_MoveToName(&playerState, &tempState, "itemBuffer");
            DDL_MoveToIndex(&tempState, &tempWeaponState, itemIndex, 1);
            MatchRecorderDDLSetInt(&tempWeaponState, "index", statsItemIndex);
            MatchRecorderDDLSetInt(&tempWeaponState, "kills", kills);
            MatchRecorderDDLSetInt(&tempWeaponState, "shots", shots);
            MatchRecorderDDLSetInt(&tempWeaponState, "hits", hits);
            MatchRecorderDDLSetInt(&tempWeaponState, "used", used);
        }
    }
}

void __cdecl MatchRecord_SetLifeCount(__int16 currentLives)
{
    ddlState_t tempState; // [esp+0h] [ebp-10h] BYREF

    if ( currentLives <= 1000 )
    {
        DDL_MoveToName(&g_HeaderState, &tempState, "lifeCount");
        DDL_SetInt(&tempState, currentLives, s_matchRecorderBuffer);
    }
}

unsigned int __cdecl MatchRecord_SearchCreateForPlayer(gclient_s *client, ddlState_t *playerState)
{
    int i; // [esp+0h] [ebp-24h]
    int controllerIndex; // [esp+4h] [ebp-20h]
    unsigned int playerSlot; // [esp+8h] [ebp-1Ch]
    unsigned __int64 dwid; // [esp+Ch] [ebp-18h] BYREF
    int connectedBits; // [esp+14h] [ebp-10h]
    __int16 playerCount; // [esp+18h] [ebp-Ch]
    unsigned __int64 xuid; // [esp+1Ch] [ebp-8h]

    xuid = SV_GetPlayerXuid(client->ps.clientNum);
    connectedBits = 0;
    playerSlot = MatchRecord_SearchForPlayerSlotByXUID(xuid);
    if ( playerSlot != -1 && MatchRecord_GetPlayerInSlot(playerSlot, playerState) )
        return playerSlot;
    playerCount = MatchRecord_GetPlayerCount();
    if ( playerCount >= 50 )
        return -1;
    MatchRecord_SetPlayerCount(++playerCount);
    Com_Printf(
        16,
        "MatchRecord: Creating player. xuid = %lli clientnum = %i numplayers = %i\n",
        xuid,
        client->ps.clientNum,
        playerCount);
    if ( !MatchRecord_GetPlayerInSlot(playerCount - 1, playerState) )
        return -1;
    controllerIndex = Live_GetControllerFromXUID(xuid);
    dwGetOnlineUserID(controllerIndex, &dwid);
    MatchRecorderDDLSetInt64(playerState, "xuid", xuid);
    MatchRecorderDDLSetInt64(playerState, "dwid", dwid);
    if ( client->sess.cs.name[0] )
        MatchRecorderDDLSetString(playerState, "gamertag", client->sess.cs.name);
    for ( i = 0; i < com_maxclients->current.integer; ++i )
    {
        if ( svs.clients[i].header.state >= CS_CONNECTED )
            connectedBits |= 1 << i;
    }
    MatchRecorderDDLSetInt(playerState, "connectedBits", connectedBits);
    return playerSlot;
}

void __cdecl MatchRecorderDDLSetString(ddlState_t *searchState, const char *name, const char *value)
{
    ddlState_t tempState; // [esp+0h] [ebp-10h] BYREF

    if ( !searchState
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DW\\MatchRecorder.cpp",
                    291,
                    0,
                    "%s",
                    "searchState != NULL") )
    {
        __debugbreak();
    }
    if ( searchState )
    {
        if ( DDL_MoveToName(searchState, &tempState, name) )
            DDL_SetString(&tempState, value, s_matchRecorderBuffer);
        else
            Com_PrintError(16, "Unable to navigate to correct leaf in matchrecorder");
    }
    else
    {
        Com_PrintError(16, "received NULL search state");
    }
}

void __cdecl MatchRecord_SetPlayerCount(__int16 currentPlayers)
{
    ddlState_t tempState; // [esp+0h] [ebp-10h] BYREF

    if ( currentPlayers <= 50 )
    {
        DDL_MoveToName(&g_HeaderState, &tempState, "playerCount");
        DDL_SetInt(&tempState, currentPlayers, s_matchRecorderBuffer);
    }
}

int __cdecl MatchRecord_SearchForPlayerSlotByXUID(unsigned __int64 xuid)
{
    int v1; // edx
    ddlState_t tempState; // [esp+Ch] [ebp-14h] BYREF
    int playerSlot; // [esp+1Ch] [ebp-4h]

    for ( playerSlot = 0; playerSlot < (__int16)MatchRecord_GetPlayerCount(); ++playerSlot )
    {
        if ( MatchRecord_GetPlayerInSlot(playerSlot, &tempState)
            && MatchRecorderDDLGetInt64(&tempState, "xuid") == (unsigned int)xuid
            && v1 == HIDWORD(xuid) )
        {
            return playerSlot;
        }
    }
    return -1;
}

void __cdecl GetWorldLocation(
                float xPos,
                float yPos,
                float zPos,
                unsigned __int8 *xWorld,
                unsigned __int8 *yWorld,
                unsigned __int8 *zWorld)
{
    float v6; // [esp+0h] [ebp-30h]
    float v7; // [esp+4h] [ebp-2Ch]
    float v8; // [esp+8h] [ebp-28h]
    float outTemp_4; // [esp+28h] [ebp-8h]
    float outTemp_8; // [esp+2Ch] [ebp-4h]

    outTemp_4 = (float)((float)((float)((float)((float)((-(level.compassNorth[1]))
                                                                                                        * (float)(yPos - level.compassMapUpperLeft[1]))
                                                                                        - (float)(level.compassNorth[0]
                                                                                                        * (float)(xPos - level.compassMapUpperLeft[0])))
                                                                        / level.compassMapWorldSize[1])
                                                        - 0.5)
                                        * 255.0)
                        + 127.0;
    outTemp_8 = (float)((float)((float)((float)((float)(zPos - level.compassMapWorldSize[0]) + 10000.0) / 20000.0) - 0.5)
                                        * 255.0)
                        + 127.0;
    if ( (float)((float)((float)((float)((float)((float)(level.compassNorth[1]
                                                                                                         * (float)(xPos - level.compassMapUpperLeft[0]))
                                                                                         - (float)(level.compassNorth[0]
                                                                                                         * (float)(yPos - level.compassMapUpperLeft[1])))
                                                                         / level.compassMapWorldSize[0])
                                                         - 0.5)
                                         * 255.0)
                         + 127.0) <= 255.0 )
        v8 = (float)((float)((float)((float)((float)(level.compassNorth[1] * (float)(xPos - level.compassMapUpperLeft[0]))
                                                                             - (float)(level.compassNorth[0] * (float)(yPos - level.compassMapUpperLeft[1])))
                                                             / level.compassMapWorldSize[0])
                                             - 0.5)
                             * 255.0)
             + 127.0;
    else
        v8 = 255.0f;
    *xWorld = (int)v8;
    if ( outTemp_4 <= 255.0 )
        v7 = outTemp_4;
    else
        v7 = 255.0f;
    *yWorld = (int)v7;
    if ( outTemp_8 <= 255.0 )
        v6 = outTemp_8;
    else
        v6 = 255.0f;
    *zWorld = (int)v6;
}

void __cdecl MatchRecordHit(gclient_s *attackerClient, hitLocation_t hitLocation)
{
    ddlState_t playerState; // [esp+0h] [ebp-10h] BYREF

    if ( attackerClient )
    {
        if ( MatchRecord_SearchCreateForPlayer(attackerClient, &playerState) != -1 )
            MatchRecordSetHitStat(playerState, attackerClient, hitLocation, "hitLocations");
    }
}

void __cdecl MatchRecordSetHitStat(
                ddlState_t playerState,
                gclient_s *attackerClient,
                hitLocation_t hitLocation,
                const char *hitLocStatName)
{
    const ddlState_t *RootDDLState; // eax
    char *v5; // [esp-Ch] [ebp-30h]
    ddlState_t hitLocState; // [esp+0h] [ebp-24h] BYREF
    unsigned __int8 hitLocationValue; // [esp+13h] [ebp-11h]
    ddlState_t tempState; // [esp+14h] [ebp-10h] BYREF

    DDL_MoveToName(&playerState, &tempState, hitLocStatName);
    DDL_MoveToIndex(&tempState, &tempState, hitLocation, 1);
    hitLocationValue = DDL_GetInt(&tempState, s_matchRecorderBuffer);
    if ( hitLocationValue != 255 )
    {
        DDL_SetInt(&tempState, ++hitLocationValue, s_matchRecorderBuffer);
        va("%d", hitLocation);
        v5 = va("%d", 0);
        RootDDLState = LiveStats_GetRootDDLState();
        DDL_MoveTo(RootDDLState, &hitLocState, 4, "RecentHitLocCounts", v5);
        SV_SetClientDIntStat(attackerClient->ps.clientNum, &hitLocState, hitLocationValue);
    }
}

void __cdecl MatchRecordDeath(
                gclient_s *client,
                gclient_s *attackerClient,
                char *weaponName,
                hitLocation_t hitLocation)
{
    unsigned int WeaponTableItemIndex; // eax
    const WeaponVariantDef *WeaponVariantDef; // [esp+1Ch] [ebp-50h]
    int attacker; // [esp+20h] [ebp-4Ch]
    unsigned __int8 killerY; // [esp+25h] [ebp-47h] BYREF
    unsigned __int8 killerX; // [esp+26h] [ebp-46h] BYREF
    unsigned __int8 killerZ; // [esp+27h] [ebp-45h] BYREF
    ddlState_t playerState2; // [esp+28h] [ebp-44h] BYREF
    unsigned int iWeaponIndex; // [esp+38h] [ebp-34h]
    unsigned __int8 victimY; // [esp+3Eh] [ebp-2Eh] BYREF
    unsigned __int8 victimX; // [esp+3Fh] [ebp-2Dh] BYREF
    ddlState_t lifeState; // [esp+40h] [ebp-2Ch] BYREF
    unsigned __int8 victimZ; // [esp+53h] [ebp-19h] BYREF
    unsigned int baseWeaponIndex; // [esp+54h] [ebp-18h]
    __int16 currentLife; // [esp+58h] [ebp-14h]
    ddlState_t playerState; // [esp+5Ch] [ebp-10h] BYREF

    if ( g_matchRecorderDDL )
    {
        if ( (__int16)MatchRecord_GetPlayerCount() < 49
            && (__int16)MatchRecord_GetLifeCount() < 999
            && MatchRecord_SearchCreateForPlayer(client, &playerState) != -1 )
        {
            currentLife = MatchRecorderDDLGetInt(&playerState, "currentLife");
            if ( MatchRecord_GetLifeInSlot(currentLife, &lifeState) )
            {
                MatchRecorderDDLSetInt(&lifeState, "death_time", (__int16)(level.g_msgTime / 1000));
                iWeaponIndex = G_GetWeaponIndexForName(weaponName);
                baseWeaponIndex = iWeaponIndex;
                if ( iWeaponIndex )
                {
                    WeaponVariantDef = BG_GetWeaponVariantDef(iWeaponIndex);
                    if ( WeaponVariantDef->iVariantCount >= 0 )
                        WeaponTableItemIndex = BG_GetWeaponTableItemIndex(iWeaponIndex);
                    else
                        WeaponTableItemIndex = BG_GetWeaponTableItemIndex(-WeaponVariantDef->iVariantCount);
                    baseWeaponIndex = WeaponTableItemIndex;
                }
                MatchRecorderDDLSetInt(&lifeState, "weapon", (__int16)baseWeaponIndex);
                if ( attackerClient )
                {
                    attacker = MatchRecord_SearchCreateForPlayer(attackerClient, &playerState2);
                    if ( attacker != -1 )
                    {
                        MatchRecordSetHitStat(playerState2, attackerClient, hitLocation, "criticalHitLocations");
                        MatchRecorderDDLSetInt(&lifeState, "attacker", attacker);
                    }
                    GetWorldLocation(
                        attackerClient->ps.origin[0],
                        attackerClient->ps.origin[1],
                        attackerClient->ps.origin[2],
                        &killerX,
                        &killerY,
                        &killerZ);
                    MatchRecorderDDLSetInt(&lifeState, "killerX", killerX);
                    MatchRecorderDDLSetInt(&lifeState, "killerY", killerY);
                    MatchRecorderDDLSetInt(&lifeState, "killerZ", killerZ);
                }
                GetWorldLocation(client->ps.origin[0], client->ps.origin[1], client->ps.origin[2], &victimX, &victimY, &victimZ);
                MatchRecorderDDLSetInt(&lifeState, "victimX", victimX);
                MatchRecorderDDLSetInt(&lifeState, "victimY", victimY);
                MatchRecorderDDLSetInt(&lifeState, "victimZ", victimZ);
            }
        }
    }
}

void __cdecl MatchRecordPointsSpent(int controllerIndex, int amount, pointsSpent_t reasonType, int reasonIndex)
{
    unsigned __int64 v4; // rax
    overlappedTask *overlappedIO; // [esp+0h] [ebp-28h]
    char buffer[20]; // [esp+4h] [ebp-24h] BYREF
    int offset; // [esp+1Ch] [ebp-Ch]
    unsigned __int64 xuid; // [esp+20h] [ebp-8h]

    offset = 0;
    if ( reasonType != POINTSSPENT_GUEST_SIGNIN )
    {
        LODWORD(v4) = Live_GetXuid(controllerIndex);
        xuid = v4;
        *(unsigned int *)&buffer[4] = HIDWORD(v4);
        offset += 8;
        *(unsigned int *)buffer = amount;
        offset += 4;
        *(unsigned int *)&buffer[offset] = reasonType;
        offset += 4;
        *(unsigned int *)&buffer[offset] = reasonIndex;
        offset += 4;
        if ( offset <= 20 )
        {
            overlappedIO = TaskManager_GetOpenTaskSlot(overlappedTasks_0, controllerIndex, 1);
            dwRecordEvent(overlappedIO, buffer, offset, 3u);
        }
    }
}

void __cdecl MatchRecordEnd(gclient_s *client)
{
    unsigned int UTC; // eax
    unsigned int rank; // [esp+0h] [ebp-14h]
    ddlState_t playerState; // [esp+4h] [ebp-10h] BYREF

    if ( MatchRecord_SearchCreateForPlayer(client, &playerState) != -1 )
    {
        UTC = LiveStorage_GetUTC();
        MatchRecorderDDLSetInt(&playerState, "UTCLeaveTime", UTC);
        rank = SV_GetClientDIntStat(client->ps.clientNum, &g_Rank);
        MatchRecorderDDLSetInt(&playerState, "rankAtEnd", rank);
        MatchRecordWriteItemStatsEnd(playerState, client->ps.clientNum);
    }
}

void __cdecl MatchRecordWriteItemStatsEnd(ddlState_t playerState, unsigned int clientNum)
{
    const ddlState_t *RootDDLState; // eax
    unsigned int oldShots; // [esp+0h] [ebp-70h]
    unsigned int oldKills; // [esp+4h] [ebp-6Ch]
    unsigned int oldUsed; // [esp+8h] [ebp-68h]
    unsigned int oldHits; // [esp+Ch] [ebp-64h]
    int statsItemIndex; // [esp+10h] [ebp-60h]
    unsigned int used; // [esp+14h] [ebp-5Ch]
    unsigned int kills; // [esp+18h] [ebp-58h]
    unsigned int shots; // [esp+1Ch] [ebp-54h]
    unsigned int hits; // [esp+20h] [ebp-50h]
    unsigned int itemIndex; // [esp+24h] [ebp-4Ch]
    ddlState_t tempStatsState; // [esp+28h] [ebp-48h] BYREF
    ddlState_t tempState; // [esp+38h] [ebp-38h] BYREF
    ddlState_t tempWeaponState; // [esp+48h] [ebp-28h] BYREF
    ddlState_t tempItemState; // [esp+58h] [ebp-18h] BYREF
    unsigned __int64 xuid; // [esp+68h] [ebp-8h]

    xuid = SV_GetPlayerXuid(clientNum);
    Com_Printf(16, "MatchRecord: Writing final stats for %llu\n", xuid);
    RootDDLState = LiveStats_GetRootDDLState();
    DDL_MoveToName(RootDDLState, &tempStatsState, "ItemStats");
    for ( itemIndex = 0; itemIndex < 0x55; ++itemIndex )
    {
        statsItemIndex = s_itemIndexes[itemIndex];
        DDL_MoveToIndex(&tempStatsState, &tempWeaponState, statsItemIndex, 1);
        DDL_MoveToName(&tempWeaponState, &tempWeaponState, "stats");
        DDL_MoveToName(&tempWeaponState, &tempItemState, "kills");
        DDL_MoveToName(&tempItemState, &tempState, "statValue");
        kills = SV_GetClientDIntStat(clientNum, &tempState);
        DDL_MoveToName(&tempWeaponState, &tempItemState, "shots");
        DDL_MoveToName(&tempItemState, &tempState, "statValue");
        shots = SV_GetClientDIntStat(clientNum, &tempState);
        DDL_MoveToName(&tempWeaponState, &tempItemState, "hits");
        DDL_MoveToName(&tempItemState, &tempState, "statValue");
        hits = SV_GetClientDIntStat(clientNum, &tempState);
        DDL_MoveToName(&tempWeaponState, &tempItemState, "used");
        DDL_MoveToName(&tempItemState, &tempState, "statValue");
        used = SV_GetClientDIntStat(clientNum, &tempState);
        if ( kills || shots || hits || used )
        {
            DDL_MoveToName(&playerState, &tempState, "itemBuffer");
            DDL_MoveToIndex(&tempState, &tempWeaponState, itemIndex, 1);
            oldKills = MatchRecorderDDLGetInt(&tempWeaponState, "kills");
            if ( kills >= oldKills )
                kills -= oldKills;
            MatchRecorderDDLSetInt(&tempWeaponState, "kills", kills);
            Com_Printf(16, "MatchRecord: \tWriting %i kills for %i\n", kills, statsItemIndex);
            oldShots = MatchRecorderDDLGetInt(&tempWeaponState, "shots");
            if ( shots >= oldShots )
                shots -= oldShots;
            MatchRecorderDDLSetInt(&tempWeaponState, "shots", shots);
            oldHits = MatchRecorderDDLGetInt(&tempWeaponState, "hits");
            if ( hits >= oldHits )
                hits -= oldHits;
            MatchRecorderDDLSetInt(&tempWeaponState, "hits", hits);
            oldUsed = MatchRecorderDDLGetInt(&tempWeaponState, "used");
            if ( used >= oldUsed )
                used -= oldUsed;
            MatchRecorderDDLSetInt(&tempWeaponState, "used", used);
        }
    }
}

void __cdecl MatchRecordBegin()
{
    int UTCOffset; // eax
    unsigned int playersAtStart; // [esp+0h] [ebp-8h]
    unsigned int utcStartTime; // [esp+4h] [ebp-4h]

    if ( !MatchRecorderDDLGetInt(&g_HeaderState, "playerCountStart") )
    {
        playersAtStart = MatchRecorderDDLGetInt(&g_HeaderState, "playerCount");
        MatchRecorderDDLSetInt(&g_HeaderState, "playerCountStart", playersAtStart);
        Com_Printf(16, "MatchRecord: %i players at start of match\n", playersAtStart);
    }
    utcStartTime = LiveStorage_GetUTC();
    MatchRecorderDDLSetInt(&g_HeaderState, "UTCStartTime", utcStartTime);
    UTCOffset = LiveStorage_GetUTCOffset();
    MatchRecorderDDLSetInt(&g_HeaderState, "utcOffset", UTCOffset + 12);
    if ( Com_GameMode_IsGameMode(GAMEMODE_WAGER_MATCH) )
        MatchRecorderDDLSetInt(&g_HeaderState, "isWager", 1u);
    else
        MatchRecorderDDLSetInt(&g_HeaderState, "isWager", 0);
}

void __cdecl MatchRecordPlayerDetails(gclient_s *client, const char *reason)
{
    unsigned int playerCountLeft; // [esp+0h] [ebp-34h]
    char shortReason[16]; // [esp+Ch] [ebp-28h] BYREF
    ddlState_t playerState; // [esp+24h] [ebp-10h] BYREF

    if ( g_matchRecorderDDL && !MatchRecorderDDLGetInt(&g_HeaderState, "titleID") )
    {
        if ( (__int16)MatchRecord_GetPlayerCount() < 49 && (__int16)MatchRecord_GetLifeCount() < 999 )
        {
            if ( MatchRecord_SearchCreateForPlayer(client, &playerState) != -1 )
            {
                if ( client->sess.cs.score.deaths > (signed int)MatchRecorderDDLGetInt(&playerState, "deaths") )
                    MatchRecorderDDLSetInt(&playerState, "deaths", client->sess.cs.score.deaths);
                if ( client->sess.cs.score.kills > (signed int)MatchRecorderDDLGetInt(&playerState, "kills") )
                    MatchRecorderDDLSetInt(&playerState, "kills", client->sess.cs.score.kills);
                if ( client->sess.cs.score.score > (signed int)MatchRecorderDDLGetInt(&playerState, "score") )
                    MatchRecorderDDLSetInt(&playerState, "score", client->sess.cs.score.score);
                MatchRecordEnd(client);
                if ( !MatchRecorderDDLGetInt(&g_HeaderState, "playerCountEnd") )
                {
                    playerCountLeft = MatchRecorderDDLGetInt(&g_HeaderState, "playerCountLeft") + 1;
                    MatchRecorderDDLSetInt(&g_HeaderState, "playerCountLeft", playerCountLeft);
                }
                I_strncpyz(shortReason, reason, 16);
                MatchRecorderDDLSetString(&playerState, "gamertag", client->sess.cs.name);
                MatchRecorderDDLSetString(&playerState, "dropReason", shortReason);
            }
        }
        else
        {
            Com_PrintError(16, "MatchRecord: maxed out players buffer\n");
        }
    }
}

char __cdecl MatchRecordSetPlayerStat(gclient_s *client, const char *statName, unsigned int statValue)
{
    char *v4; // eax
    char *v5; // eax
    ddlState_t tempState; // [esp+0h] [ebp-20h] BYREF
    ddlState_t playerState; // [esp+10h] [ebp-10h] BYREF

    if ( !g_matchRecorderDDL )
        return 0;
    if ( MatchRecord_SearchCreateForPlayer(client, &playerState) == -1 )
        return 0;
    if ( DDL_MoveToName(&playerState, &tempState, statName) )
    {
        DDL_SetInt(&tempState, statValue, s_matchRecorderBuffer);
        v4 = va("MatchRecord: clientNum %i statName %s statValue %i\n", client->ps.clientNum, statName, statValue);
        Com_Printf(16, v4);
        return 1;
    }
    else
    {
        v5 = va("Unhandled stat - clientNum %i statName %s statValue %i\n", client->ps.clientNum, statName, statValue);
        Com_Printf(16, v5);
        return 0;
    }
}

void __cdecl MatchRecordMovement()
{
    ddlState_t movementBufferState; // [esp+18h] [ebp-38h] BYREF
    gclient_s *client; // [esp+28h] [ebp-28h]
    unsigned int offset; // [esp+2Ch] [ebp-24h]
    ddlState_t playerState; // [esp+30h] [ebp-20h] BYREF
    int clientNum; // [esp+40h] [ebp-10h]
    int i; // [esp+44h] [ebp-Ch]
    unsigned __int8 origin[4]; // [esp+48h] [ebp-8h] BYREF
    gentity_s *ent; // [esp+4Ch] [ebp-4h]

    if ( g_matchRecorderDDL && level.g_msgTime < 900000 && !(level.g_msgTime % 18000) )
    {
        ent = g_entities;
        i = 0;
        while ( i < level.maxclients )
        {
            clientNum = ent->client->ps.clientNum;
            if ( (unsigned int)clientNum >= com_maxclients->current.integer
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\DW\\MatchRecorder.cpp",
                            1863,
                            0,
                            "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                            clientNum,
                            com_maxclients->current.integer) )
            {
                __debugbreak();
            }
            if ( ent->r.inuse )
            {
                client = ent->client;
                if ( MatchRecord_SearchCreateForPlayer(client, &playerState) != -1
                    && (!svs.clients || !svs.clients[clientNum].bIsDemoClient) )
                {
                    GetWorldLocation(
                        level.clients[clientNum].ps.origin[0],
                        level.clients[clientNum].ps.origin[1],
                        level.clients[clientNum].ps.origin[2],
                        origin,
                        &origin[1],
                        &origin[2]);
                    offset = clientNum + 2 * level.maxclients * (level.g_msgTime / 18000);
                    if ( offset < 0xC80 )
                    {
                        DDL_MoveToName(&g_RootState, &movementBufferState, "movementBuffer");
                        DDL_MoveToIndex(&movementBufferState, &movementBufferState, offset, 1);
                        DDL_SetInt(&movementBufferState, origin[0], s_matchRecorderBuffer);
                        DDL_IterateNext(&movementBufferState, &movementBufferState);
                        DDL_SetInt(&movementBufferState, origin[1], s_matchRecorderBuffer);
                    }
                }
            }
            ++i;
            ++ent;
        }
    }
}

void __cdecl Live_CheckOngoingMatchRecordingTasks()
{
    taskCompleteResults matched; // [esp+0h] [ebp-8h]
    int tasknum; // [esp+4h] [ebp-4h]

    for ( tasknum = 0; tasknum < 32; ++tasknum )
    {
        if ( overlappedTasks_0[tasknum].active )
        {
            matched = MatchRecordEventComplete(tasknum);
            if ( matched )
            {
                if ( matched == TASK_COMPLETE )
                    Com_Printf(16, "Upload of COD7 xml match data was a success.\n");
                else
                    Com_Printf(16, "Upload of COD7 xml match data failed.\n");
            }
        }
    }
}

taskCompleteResults __cdecl MatchRecordEventComplete(int slot)
{
    taskCompleteResults result; // [esp+0h] [ebp-Ch]
    overlappedTask *fetchedOverlappedIO; // [esp+8h] [ebp-4h]

    fetchedOverlappedIO = &overlappedTasks_0[slot];
    if ( !overlappedTasks_0[slot].active
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DW\\MatchRecorder.cpp",
                    1916,
                    0,
                    "%s",
                    "overlappedTasks[slot].active") )
    {
        __debugbreak();
    }
    if ( overlappedTasks_0[slot].type != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DW\\MatchRecorder.cpp",
                    1917,
                    0,
                    "%s",
                    "overlappedTasks[slot].type == TASK_RECORD_GAME") )
    {
        __debugbreak();
    }
    result = dwRecordEventComplete(fetchedOverlappedIO);
    if ( result )
        TaskManager_ClearTask(fetchedOverlappedIO);
    return result;
}


#endif