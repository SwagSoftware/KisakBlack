#include "live_combatrecord.h"
#include <ui/ui_main.h>
#include <universal/com_stringtable.h>
#include <cstring>
#include <bgame/bg_unlockable_items.h>
#include <ddl/ddl_api.h>
#include "live_stats.h"
#include "live_storage.h"
#include <qcommon/com_gamemodes.h>

const int sortedStatLoadoutSlots[9] =
{ 0, 12, 30, 31, 32, 33, 29, 24, 25 };

const char *lbWagerGameModeEnum_5[5] =
{ "oic", "hlnd", "gun", "shrp", NULL };


const char *lbTypeEnum_5[17] =
{
  "tdm",
  "dm",
  "ctf",
  "dom",
  "sab",
  "sd",
  "koth",
  "dem",
  "hctdm",
  "hcdm",
  "hcctf",
  "hcdom",
  "hcsab",
  "hcsd",
  "hckoth",
  "hcdem",
  NULL
};

const char *CombatRecordViews[23] =
{
  "SUMMARY_CAPS",
  "RECENT_PERFORMANCE_CAPS",
  "PERSONAL_BESTS_CAPS",
  "COMBAT_SUMMARY_CAPS",
  "HIT_LOCATION_HEATMAP_CAPS",
  "KILL_DEATH_RATIO_CAPS",
  "ACCURACY_CAPS",
  "TOTAL_KILLS_CAPS",
  "TOTAL_DEATHS_CAPS",
  "HEADSHOT_KILLS_CAPS",
  "TIMES_KILLED_BY_CAPS",
  "EQUIPMENT_KILLS_CAPS",
  "EQUIPMENT_KILLED_BY_CAPS",
  "EQUIPMENT_USAGE_CAPS",
  "MATCH_PERFORMANCE_CAPS",
  "HEAT_MAP_CAPS",
  "WAGER_EARNINGS_CAPS",
  "CONTRACTS_CAPS",
  "KILLSTREAKS_FAVORITES_CAPS",
  "KILLSTREAKS_CALLED_CAPS",
  "KILLSTREAKS_KILLS_CAPS",
  "KILLSTREAKS_KILLED_BY_CAPS",
  NULL
};

const char *hitLocationImages[19] =
{
  "NULL",
  "menu_mp_heatmap_1",
  "menu_mp_heatmap_2",
  "menu_mp_heatmap_3",
  "menu_mp_heatmap_4",
  "menu_mp_heatmap_5",
  "menu_mp_heatmap_6",
  "menu_mp_heatmap_7",
  "menu_mp_heatmap_8",
  "menu_mp_heatmap_9",
  "menu_mp_heatmap_10",
  "menu_mp_heatmap_11",
  "menu_mp_heatmap_12",
  "menu_mp_heatmap_13",
  "menu_mp_heatmap_14",
  "menu_mp_heatmap_15",
  "menu_mp_heatmap_16",
  "menu_mp_heatmap_17",
  "NULL"
};

const char *combatRecordEquipmentUsageStrings[14][2] =
{
  { "WEAPON_FRAGGRENADE", "TOSSED" },
  { "WEAPON_STICKY_GRENADE", "TOSSED" },
  { "WEAPON_CLAYMORE", "DEPLOYED" },
  { "WEAPON_SATCHEL_CHARGE", "DEPLOYED" },
  { "WEAPON_HATCHET", "THROWN" },
  { "WEAPON_WILLY_PETE", "TOSSED" },
  { "WEAPON_TABUN_GRENADE", "POISONED" },
  { "WEAPON_FLASHBANG", "FLASHED" },
  { "WEAPON_STUN_GRENADE", "STUNNED" },
  { "WEAPON_NIGHTINGALE", "USED" },
  { "WEAPON_SCRAMBLER", "DEPLOYED" },
  { "WEAPON_TACTICAL_INSERTION", "SPAWNS" },
  { "WEAPON_CAMERA_SPIKE", "DEPLOYED" },
  { "WEAPON_ACOUSTIC_SENSOR", "DEPLOYED" }
};

const char *combatRecordPersonalBestStats[5] =
{ "KILLS", "DEATHS", "SCORE", "ASSISTS", "REVIVES" };

sortedItemsData_t s_otherPlayerSortedItemList[256];
sortedItemsData_t s_sortedItemList[256];

const dvar_t *ui_showFriendsCombatRecord;
const dvar_t *ui_combatRecordOpen;
const dvar_t *ui_combatCurrView;
const dvar_t *ui_combatCurrViewNum;
const dvar_t *ui_combatComparisonModeOn;
const dvar_t *ui_combatOtherPlayerStatsAvailable;
const dvar_t *ui_combatHistogramCurrGametype;

void __cdecl LiveCombatRecord_OpenFriendsCombatRecord(int localClientNum)
{
    if ( Dvar_GetBool("ui_showFriendsCombatRecord") )
    {
        if ( Dvar_GetBool("ui_playercardOpen") )
            UI_CloseMenu(localClientNum, "menu_playercard");
        UI_OpenMenu(localClientNum, "menu_cr_career_summary");
    }
}

double __cdecl LiveCombatRecord_GetSortedItemData(int index, bool forFriend, sortedItemParams_t param)
{
    double result; // st7
    sortedItemsData_t *v4; // [esp+4h] [ebp-8h]

    if ( index >= 0 && index < sharedUiInfo.clanTagFeature )
    {
        if ( (unsigned int)param <= SORT_KEY )
        {
            if ( forFriend )
                v4 = &s_otherPlayerSortedItemList[index];
            else
                v4 = &s_sortedItemList[index];
            switch ( param )
            {
                case ITEM_INDEX:
                    result = (double)v4->itemIndex;
                    break;
                case PARAM1:
                    result = (double)v4->param1;
                    break;
                case PARAM2:
                    result = (double)v4->param2;
                    break;
                case PARAM3:
                    result = (double)v4->param3;
                    break;
                case SORT_KEY:
                    result = v4->sortKey;
                    break;
                default:
                    result = 0.0;
                    break;
            }
        }
        else
        {
            Com_PrintError(15, "Invalid param for sorted item.");
            return 0.0;
        }
    }
    else
    {
        Com_PrintError(15, "Invalid index for sorted items array.");
        return 0.0;
    }
    return result;
}

int __cdecl LiveCombatRecord_GetSortedItemPositionByItemIndex(int itemIndex, bool forFriend)
{
    sortedItemsData_t *v3; // [esp+0h] [ebp-Ch]
    int currPos; // [esp+4h] [ebp-8h]

    if ( forFriend )
        v3 = s_otherPlayerSortedItemList;
    else
        v3 = s_sortedItemList;
    for ( currPos = 0; currPos < sharedUiInfo.clanTagFeature; ++currPos )
    {
        if ( v3[currPos].itemIndex == itemIndex )
            return currPos;
    }
    return 0;
}

const char *__cdecl LiveCombatRecord_GetEquipmentUsageStringSuffix(const char *equipmentName)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 0xE; ++i )
    {
        if ( !I_stricmp(equipmentName, combatRecordEquipmentUsageStrings[i][0]) )
            return combatRecordEquipmentUsageStrings[i][1];
    }
    return 0;
}

const char *__cdecl LiveCombatRecord_GetPersonalBestStatNameByIndex(int index)
{
    return combatRecordPersonalBestStats[index];
}

const char *gameTypeNames[16];
const char *__cdecl LiveCombatRecord_GetGameTypeName(unsigned int index)
{
    if ( index < 0x10 )
        return gameTypeNames[index];
    else
        return "";
}

int gameTypeLbIndex[16];
int __cdecl LiveCombatRecord_GetGameTypeLbIndex(unsigned int index)
{
    if ( index < 0x10 )
        return gameTypeLbIndex[index];
    else
        return 0;
}

int totalGametypeCount;
int __cdecl LiveCombatRecord_GetGameTypeCount()
{
    return totalGametypeCount;
}

const char *__cdecl LiveCombatRecord_GetHitLocationImageRef(hitLocation_t hitLocation)
{
    if ( (hitLocation <= HITLOC_NONE || hitLocation >= HITLOC_NUM)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_combatrecord.cpp",
                    1020,
                    0,
                    "%s",
                    "hitLocation > 0 && hitLocation < HITLOC_NUM") )
    {
        __debugbreak();
    }
    if ( (unsigned int)hitLocation <= HITLOC_GUN )
        return hitLocationImages[hitLocation];
    else
        return 0;
}

cmd_function_s LiveCombatRecord_CombatRecordViewUpdateCmd_VAR;
void __cdecl LiveCombatRecord_Init()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    int currGameType; // [esp+0h] [ebp-10h]
    const StringTable *gameTypesTable; // [esp+4h] [ebp-Ch] BYREF
    int currGameTypeLbIndex; // [esp+8h] [ebp-8h]
    const char *currGameTypeName; // [esp+Ch] [ebp-4h]

    StringTable_GetAsset("mp/gametypesTable.csv", (XAssetHeader *)&gameTypesTable);
    if ( !gameTypesTable
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_combatrecord.cpp",
                    1038,
                    0,
                    "%s",
                    "gameTypesTable") )
    {
        __debugbreak();
    }
    totalGametypeCount = 0;
    currGameTypeLbIndex = 0;
    for ( currGameType = 0; currGameType < 16; ++currGameType )
    {
        v0 = va("%d", currGameType);
        currGameTypeName = StringTable_Lookup(gameTypesTable, 0, v0, 3);
        v1 = va("%d", currGameType);
        v2 = StringTable_Lookup(gameTypesTable, 0, v1, 5);
        currGameTypeLbIndex = atoi(v2);
        if ( currGameTypeName )
        {
            if ( *currGameTypeName )
            {
                gameTypeNames[totalGametypeCount] = currGameTypeName;
                gameTypeLbIndex[totalGametypeCount++] = currGameTypeLbIndex;
            }
        }
    }
    Cmd_AddCommandInternal(
        "updateCombatRecordView",
        LiveCombatRecord_CombatRecordViewUpdateCmd,
        &LiveCombatRecord_CombatRecordViewUpdateCmd_VAR);
    ui_showFriendsCombatRecord = _Dvar_RegisterBool(
                                                                 "ui_friendsCombatRecordOpen",
                                                                 0,
                                                                 0,
                                                                 "True when friend's or clan member's combat record is open.");
    ui_combatRecordOpen = _Dvar_RegisterBool("ui_combatRecordOpen", 0, 0, "True when Combat Record is Open.");
    ui_combatCurrView = _Dvar_RegisterEnum(
                                                "ui_combatCurrView",
                                                CombatRecordViews,
                                                0,
                                                0,
                                                "Current View in the combat record weapons tab.");
    ui_combatCurrViewNum = _Dvar_RegisterInt(
                                                     "ui_combatCurrViewNum",
                                                     0,
                                                     0,
                                                     21,
                                                     0,
                                                     "Current View number in the combat record weapons tab.");
    ui_combatComparisonModeOn = _Dvar_RegisterBool(
                                                                "ui_combatComparisonModeOn",
                                                                0,
                                                                0,
                                                                "True when comparing stats with other player in combat record");
    ui_combatOtherPlayerStatsAvailable = _Dvar_RegisterBool(
                                                                                 "ui_combatOtherPlayerStatsAvailable",
                                                                                 1,
                                                                                 0,
                                                                                 "False when compared player has not played any public matches, or stats are corr"
                                                                                 "upted, or could not be converted to current DDL.");
    ui_combatHistogramCurrGametype = _Dvar_RegisterInt(
                                                                         "ui_combatHistogramCurrGametype",
                                                                         0,
                                                                         0x80000000,
                                                                         0x7FFFFFFF,
                                                                         0,
                                                                         "Current scroll bar Position for combat record");
}

const int CombatRecordStatParams[22][2] =
{
  { -1, -1 },
  { 2, -1 },
  { -1, -1 },
  { 26, -1 },
  { -1, -1 },
  { 16, 18 },
  { 25, 24 },
  { 16, -1 },
  { 18, -1 },
  { 19, -1 },
  { 17, -1 },
  { 16, -1 },
  { 17, -1 },
  { 27, -1 },
  { -1, -1 },
  { -1, -1 },
  { 2, -1 },
  { -1, -1 },
  { 26, -1 },
  { 27, -1 },
  { 16, -1 },
  { 17, -1 }
};

void __cdecl LiveCombatRecord_CombatRecordViewUpdateCmd()
{
    playerStatsKeyIndex_t param1; // [esp+8h] [ebp-Ch]
    itemSortingCriteria_t criterion; // [esp+Ch] [ebp-8h]
    playerStatsKeyIndex_t param2; // [esp+10h] [ebp-4h]

    criterion = CRITERION_NULL;
    param1 = (playerStatsKeyIndex_t)CombatRecordStatParams[ui_combatCurrView->current.integer][0];
    param2 = (playerStatsKeyIndex_t)CombatRecordStatParams[ui_combatCurrView->current.integer][1];
    switch ( ui_combatCurrView->current.integer )
    {
        case 1:
            LiveCombatRecord_BuildSortedItemListByStats(0, RECENT_PERFORMANCE, param1, param2);
            break;
        case 2:
            LiveCombatRecord_BuildSortedItemListByStats(0, PERSONAL_BESTS, param1, param2);
            break;
        case 3:
            LiveCombatRecord_BuildSortedItemListByStats(0, FAVORITE_WEAPON, param1, param2);
            break;
        case 4:
            LiveCombatRecord_BuildSortedItemListByStats(0, WEAPON_HITLOCATION_HEATMAP, param1, param2);
            break;
        case 5:
        case 7:
        case 8:
        case 0xA:
            LiveCombatRecord_BuildSortedItemListByStats(0, WEAPON_STATS, param1, param2);
            break;
        case 6:
            LiveCombatRecord_BuildSortedItemListByStats(0, WEAPON_ACCURACY, param1, param2);
            break;
        case 9:
            LiveCombatRecord_BuildSortedItemListByStats(0, WEAPON_HEADSHOTS, param1, param2);
            break;
        case 0xB:
        case 0xC:
            LiveCombatRecord_BuildSortedItemListByStats(0, EQUIPMENT_KILLS, param1, param2);
            break;
        case 0xD:
            LiveCombatRecord_BuildSortedItemListByStats(0, EQUIPMENT_USAGE, param1, param2);
            break;
        case 0xE:
            LiveCombatRecord_BuildSortedItemListByStats(0, MATCH_PERFORMANCE, param1, param2);
            break;
        case 0x10:
            LiveCombatRecord_BuildSortedItemListByStats(0, RECENT_EARNINGS, param1, param2);
            break;
        case 0x12:
            LiveCombatRecord_BuildSortedItemListByStats(0, KILLSTREAKS_FAVORITES, param1, param2);
            break;
        case 0x13:
            LiveCombatRecord_BuildSortedItemListByStats(0, KILLSTREAKS_CALLED, param1, param2);
            break;
        case 0x14:
        case 0x15:
            criterion = KILLSTREAKS_KILLS;
            goto LABEL_16;
        default:
LABEL_16:
            LiveCombatRecord_BuildSortedItemListByStats(0, criterion, param1, param2);
            break;
    }
    Dvar_SetInt((dvar_s *)ui_combatCurrViewNum, ui_combatCurrView->current.integer);
}

void __cdecl LiveCombatRecord_BuildSortedItemListByStats(
                int controllerIndex,
                itemSortingCriteria_t criterion,
                playerStatsKeyIndex_t param,
                playerStatsKeyIndex_t divisionParam)
{
    sortedItemsData_t *v4; // edx
    sortedItemsData_t *v5; // eax
    sortedItemsData_t *v6; // eax
    sortedItemsData_t *v7; // edx
    sortedItemsData_t *v8; // eax
    sortedItemsData_t *v9; // edx
    sortedItemsData_t *v10; // eax
    sortedItemsData_t *v11; // ecx
    sortedItemsData_t *v12; // [esp+4h] [ebp-F0h]
    int currEarnings; // [esp+8h] [ebp-ECh]
    int minEarnings; // [esp+Ch] [ebp-E8h]
    int maxEarnings; // [esp+10h] [ebp-E4h]
    int minScore; // [esp+14h] [ebp-E0h]
    int currScore; // [esp+18h] [ebp-DCh]
    int maxScore; // [esp+1Ch] [ebp-D8h]
    int itemNumber; // [esp+20h] [ebp-D4h]
    int itemNumbera; // [esp+20h] [ebp-D4h]
    int itemNumberb; // [esp+20h] [ebp-D4h]
    int itemNumberc; // [esp+20h] [ebp-D4h]
    int itemNumberd; // [esp+20h] [ebp-D4h]
    int itemNumbere; // [esp+20h] [ebp-D4h]
    int itemNumberf; // [esp+20h] [ebp-D4h]
    int itemNumberg; // [esp+20h] [ebp-D4h]
    int itemNumberh; // [esp+20h] [ebp-D4h]
    sortedItemsData_t sortedItemsBuffer[9]; // [esp+24h] [ebp-D0h] BYREF
    bool comparisonModeOn; // [esp+E3h] [ebp-11h]
    int index; // [esp+E4h] [ebp-10h]
    int currLoadout; // [esp+E8h] [ebp-Ch]
    sortedItemsData_t *sortedItemList; // [esp+ECh] [ebp-8h]
    bool forOtherPlayer; // [esp+F3h] [ebp-1h]

    if ( criterion != CRITERION_NULL )
    {
        forOtherPlayer = Dvar_GetBool("ui_showFriendsCombatRecord");
        comparisonModeOn = Dvar_GetBool("ui_combatComparisonModeOn");
        memset(s_sortedItemList, 0, sizeof(s_sortedItemList));
        memset(s_otherPlayerSortedItemList, 0, sizeof(s_otherPlayerSortedItemList));
        if ( forOtherPlayer )
            v12 = s_otherPlayerSortedItemList;
        else
            v12 = s_sortedItemList;
        sortedItemList = v12;
        switch ( criterion )
        {
            case PERSONAL_BESTS:
                itemNumber = 0;
                index = 0;
                while ( itemNumber < 5 )
                {
                    LiveCombatRecord_BuildSortedItem(controllerIndex, criterion, 0, index, itemNumber, param, divisionParam);
                    if ( forOtherPlayer || comparisonModeOn )
                        LiveCombatRecord_BuildSortedItem(controllerIndex, criterion, 1, index, itemNumber, param, divisionParam);
                    ++index;
                    ++itemNumber;
                }
                sharedUiInfo.sortedItemPivot = 2;
                sharedUiInfo.clanTagFeature = index;
                break;
            case RECENT_PERFORMANCE:
                itemNumbera = 0;
                index = 0;
                while ( itemNumbera < 10 )
                {
                    LiveCombatRecord_BuildSortedItem(controllerIndex, criterion, 0, index, itemNumbera, param, divisionParam);
                    if ( forOtherPlayer || comparisonModeOn )
                        LiveCombatRecord_BuildSortedItem(controllerIndex, criterion, 1, index, itemNumbera, param, divisionParam);
                    ++index;
                    ++itemNumbera;
                }
                minScore = sortedItemList->param1;
                maxScore = minScore;
                for ( index = 0; index < 10 && sortedItemList[index].itemIndex; ++index )
                {
                    currScore = sortedItemList[index].param1;
                    if ( currScore > maxScore )
                        maxScore = sortedItemList[index].param1;
                    if ( currScore < minScore )
                        minScore = sortedItemList[index].param1;
                }
                sortedItemList[10].itemIndex = index;
                sortedItemList[10].param1 = minScore;
                sortedItemList[10].param2 = maxScore;
                sharedUiInfo.clanTagFeature = 11;
                break;
            case FAVORITE_WEAPON:
            case TIME_SPENT_USING_ITEM:
                for ( currLoadout = 0; currLoadout < 9; ++currLoadout )
                {
                    itemNumberc = 0;
                    index = 0;
                    while ( itemNumberc < 256 )
                    {
                        if ( LiveCombatRecord_FilterCurrentSortedItem(
                                     controllerIndex,
                                     TIME_SPENT_USING_ITEM,
                                     itemNumberc,
                                     (sortedStatLoadoutSlots_t)currLoadout) )
                        {
                            LiveCombatRecord_BuildSortedItem(
                                controllerIndex,
                                TIME_SPENT_USING_ITEM,
                                forOtherPlayer,
                                index++,
                                itemNumberc,
                                param,
                                divisionParam);
                        }
                        ++itemNumberc;
                    }
                    qsort(
                        sortedItemList,
                        index,
                        0x14u,
                        (int (__cdecl *)(const void *, const void *))LiveCombatRecord_CompareItemsByStats);
                    sortedItemsBuffer[currLoadout] = *sortedItemList;
                }
                for ( currLoadout = 0; currLoadout < 9; ++currLoadout )
                {
                    v4 = &sortedItemsBuffer[currLoadout];
                    v5 = &sortedItemList[currLoadout];
                    v5->itemIndex = v4->itemIndex;
                    v5->sortKey = v4->sortKey;
                    v5->param1 = v4->param1;
                    v5->param2 = v4->param2;
                    v5->param3 = v4->param3;
                }
                sharedUiInfo.clanTagFeature = 9;
                if ( criterion == FAVORITE_WEAPON )
                {
                    if ( sortedItemList[1].sortKey > sortedItemList->sortKey )
                        *sortedItemList = sortedItemsBuffer[1];
                    sortedItemList[10].itemIndex = 0;
                    itemNumberd = 0;
                    index = 1;
                    while ( itemNumberd < 5 )
                    {
                        LiveCombatRecord_BuildSortedItem(
                            controllerIndex,
                            FAVORITE_WEAPON,
                            0,
                            index,
                            itemNumberd,
                            param,
                            divisionParam);
                        if ( forOtherPlayer || comparisonModeOn )
                            LiveCombatRecord_BuildSortedItem(
                                controllerIndex,
                                FAVORITE_WEAPON,
                                1,
                                index,
                                itemNumberd,
                                param,
                                divisionParam);
                        ++index;
                        ++itemNumberd;
                    }
                    sharedUiInfo.clanTagFeature = 11;
                }
                break;
            case WEAPON_STATS:
            case WEAPON_ACCURACY:
            case WEAPON_HEADSHOTS:
            case EQUIPMENT_KILLS:
            case EQUIPMENT_USAGE:
            case KILLSTREAKS_FAVORITES:
            case KILLSTREAKS_CALLED:
            case KILLSTREAKS_KILLS:
                itemNumberb = 0;
                index = 0;
                while ( itemNumberb < 256 )
                {
                    if ( LiveCombatRecord_FilterCurrentSortedItem(controllerIndex, criterion, itemNumberb, SLOT_NULL) )
                    {
                        LiveCombatRecord_BuildSortedItem(controllerIndex, criterion, 0, index, itemNumberb, param, divisionParam);
                        if ( forOtherPlayer || comparisonModeOn )
                            LiveCombatRecord_BuildSortedItem(controllerIndex, criterion, 1, index, itemNumberb, param, divisionParam);
                        ++index;
                    }
                    ++itemNumberb;
                }
                sharedUiInfo.sortedItemPivot = 2;
                sharedUiInfo.clanTagFeature = index;
                qsort(
                    s_sortedItemList,
                    index,
                    0x14u,
                    (int (__cdecl *)(const void *, const void *))LiveCombatRecord_CompareItemsByStats);
                qsort(
                    s_otherPlayerSortedItemList,
                    sharedUiInfo.clanTagFeature,
                    0x14u,
                    (int (__cdecl *)(const void *, const void *))LiveCombatRecord_CompareItemsByStats);
                break;
            case WEAPON_HITLOCATION_HEATMAP:
                for ( index = 0; index < 19; ++index )
                {
                    LiveCombatRecord_BuildSortedItem(controllerIndex, criterion, 0, index, 0, param, divisionParam);
                    if ( forOtherPlayer || comparisonModeOn )
                        LiveCombatRecord_BuildSortedItem(controllerIndex, criterion, 1, index, 0, param, divisionParam);
                }
                sharedUiInfo.clanTagFeature = 19;
                break;
            case MATCH_TYPES:
                itemNumbere = 0;
                index = 0;
                while ( itemNumbere < 16 )
                    LiveCombatRecord_BuildSortedItem(
                        controllerIndex,
                        MATCH_TYPES,
                        forOtherPlayer,
                        index++,
                        itemNumbere++,
                        param,
                        divisionParam);
                qsort(
                    sortedItemList,
                    index,
                    0x14u,
                    (int (__cdecl *)(const void *, const void *))LiveCombatRecord_CompareMatchCount);
                for ( index = 0; index < 3; ++index )
                {
                    v6 = &sortedItemList[index];
                    v7 = &sortedItemsBuffer[index];
                    v7->itemIndex = v6->itemIndex;
                    v7->sortKey = v6->sortKey;
                    v7->param1 = v6->param1;
                    v7->param2 = v6->param2;
                    v7->param3 = v6->param3;
                }
                itemNumberf = 0;
                index = 0;
                while ( itemNumberf < 4 )
                    LiveCombatRecord_BuildSortedItem(
                        controllerIndex,
                        WAGER_MATCH_TYPES,
                        forOtherPlayer,
                        index++,
                        itemNumberf++,
                        param,
                        divisionParam);
                qsort(
                    sortedItemList,
                    index,
                    0x14u,
                    (int (__cdecl *)(const void *, const void *))LiveCombatRecord_CompareMatchCount);
                for ( index = 0; index < 3; ++index )
                {
                    v8 = &sortedItemList[index];
                    v9 = &sortedItemsBuffer[index + 3];
                    v9->itemIndex = v8->itemIndex;
                    v9->sortKey = v8->sortKey;
                    v9->param1 = v8->param1;
                    v9->param2 = v8->param2;
                    v9->param3 = v8->param3;
                }
                for ( index = 0; index < 6; ++index )
                {
                    v10 = &sortedItemsBuffer[index];
                    v11 = &sortedItemList[index];
                    v11->itemIndex = v10->itemIndex;
                    v11->sortKey = v10->sortKey;
                    v11->param1 = v10->param1;
                    v11->param2 = v10->param2;
                    v11->param3 = v10->param3;
                }
                sharedUiInfo.clanTagFeature = 6;
                break;
            case MATCH_PERFORMANCE:
                for ( itemNumberg = 0; itemNumberg < 16; ++itemNumberg )
                    LiveCombatRecord_BuildSortedItem(
                        controllerIndex,
                        MATCH_PERFORMANCE,
                        forOtherPlayer,
                        0,
                        itemNumberg,
                        param,
                        divisionParam);
                sharedUiInfo.clanTagFeature = 160;
                break;
            case RECENT_EARNINGS:
                for ( itemNumberh = 0; itemNumberh < 10; ++itemNumberh )
                    LiveCombatRecord_BuildSortedItem(
                        controllerIndex,
                        RECENT_EARNINGS,
                        forOtherPlayer,
                        0,
                        itemNumberh,
                        param,
                        divisionParam);
                minEarnings = sortedItemList->param1;
                maxEarnings = minEarnings;
                for ( index = 0; index < 10 && sortedItemList[index].itemIndex; ++index )
                {
                    currEarnings = sortedItemList[index].param1;
                    if ( currEarnings > maxEarnings )
                        maxEarnings = sortedItemList[index].param1;
                    if ( currEarnings < minEarnings )
                        minEarnings = sortedItemList[index].param1;
                }
                sortedItemList[10].param1 = minEarnings;
                sortedItemList[10].param2 = maxEarnings;
                sharedUiInfo.clanTagFeature = 11;
                break;
            default:
                return;
        }
    }
}

int __cdecl LiveCombatRecord_CompareItemsByStats(float *arg0, float *arg1)
{
    loadoutSlot_t loadoutSlot1; // [esp+0h] [ebp-24h]
    loadoutSlot_t loadoutSlot0; // [esp+4h] [ebp-20h]
    int count0; // [esp+8h] [ebp-1Ch]
    int count1; // [esp+Ch] [ebp-18h]
    itemGroup_t group0; // [esp+10h] [ebp-14h]
    itemGroup_t group1; // [esp+14h] [ebp-10h]

    if ( arg0[1] == 2147483600.0 && arg1[1] == 2147483600.0 )
    {
        if ( *((unsigned int *)arg0 + 2) == *((unsigned int *)arg1 + 2) )
            return *(unsigned int *)arg0 - *(unsigned int *)arg1;
        else
            return *((unsigned int *)arg1 + 2) - *((unsigned int *)arg0 + 2);
    }
    else if ( arg1[1] <= (float)(arg0[1] - 0.0000099999997) || (float)(arg0[1] + 0.0000099999997) <= arg1[1] )
    {
        if ( arg1[1] <= arg0[1] )
            return -1;
        else
            return 1;
    }
    else
    {
        group0 = (itemGroup_t)BG_UnlockablesGetItemGroupEnum(*(unsigned int *)arg0);
        group1 = (itemGroup_t)BG_UnlockablesGetItemGroupEnum(*(unsigned int *)arg1);
        count0 = BG_UnlockablesGetItemCount(*(unsigned int *)arg0);
        count1 = BG_UnlockablesGetItemCount(*(unsigned int *)arg1);
        if ( group0 == ITEMGROUP_GLOBAL_ITEMS_START && group1 == ITEMGROUP_GLOBAL_ITEMS_START )
        {
            return count0 - count1;
        }
        else if ( *((unsigned int *)arg0 + 3) || *((unsigned int *)arg1 + 3) )
        {
            return *((unsigned int *)arg1 + 3) - *((unsigned int *)arg0 + 3);
        }
        else if ( group0 == ITEMGROUP_MISCWEAPON || group1 == ITEMGROUP_MISCWEAPON )
        {
            if ( group0 == group1 )
                return *(unsigned int *)arg0 - *(unsigned int *)arg1;
            else
                return group0 - group1;
        }
        else if ( group0 == ITEMGROUP_GRENADE
                     || group0 == ITEMGROUP_EXPLOSIVE
                     || group1 == ITEMGROUP_GRENADE
                     || group1 == ITEMGROUP_EXPLOSIVE )
        {
            loadoutSlot0 = (loadoutSlot_t)BG_UnlockablesGetItemLoadoutSlot(*(unsigned int *)arg0);
            loadoutSlot1 = (loadoutSlot_t)BG_UnlockablesGetItemLoadoutSlot(*(unsigned int *)arg1);
            if ( loadoutSlot0 == loadoutSlot1 )
                return *(unsigned int *)arg0 - *(unsigned int *)arg1;
            else
                return loadoutSlot0 - loadoutSlot1;
        }
        else if ( group0 == group1 )
        {
            return *(unsigned int *)arg0 - *(unsigned int *)arg1;
        }
        else
        {
            return group0 - group1;
        }
    }
}

int __cdecl LiveCombatRecord_CompareMatchCount(float *arg0, float *arg1)
{
    if ( arg0[1] == 2147483600.0 && arg1[1] == 2147483600.0 )
    {
        if ( *((unsigned int *)arg0 + 2) == *((unsigned int *)arg1 + 2) )
            return *(unsigned int *)arg0 - *(unsigned int *)arg1;
        else
            return *((unsigned int *)arg1 + 2) - *((unsigned int *)arg0 + 2);
    }
    else if ( arg1[1] <= (float)(arg0[1] - 0.0000099999997) || (float)(arg0[1] + 0.0000099999997) <= arg1[1] )
    {
        if ( arg1[1] <= arg0[1] )
            return -1;
        else
            return 1;
    }
    else if ( *((unsigned int *)arg0 + 3) || *((unsigned int *)arg1 + 3) )
    {
        return *((unsigned int *)arg1 + 3) - *((unsigned int *)arg0 + 3);
    }
    else
    {
        return *(unsigned int *)arg0 - *(unsigned int *)arg1;
    }
}

void __cdecl LiveCombatRecord_BuildSortedItem(
                int controllerIndex,
                itemSortingCriteria_t criterion,
                bool forOtherPlayer,
                int index,
                int itemNumber,
                playerStatsKeyIndex_t param,
                playerStatsKeyIndex_t divisionParam)
{
    ddlDef_t *StatsDDL; // eax
    ddlDef_t *v8; // eax
    ddlDef_t *v9; // eax
    ddlDef_t *v10; // eax
    const ddlState_t *RootDDLState; // eax
    unsigned int Int; // eax
    const ddlState_t *v13; // eax
    unsigned int v14; // eax
    const ddlState_t *v15; // eax
    unsigned int v16; // eax
    const ddlState_t *v17; // eax
    unsigned int v18; // eax
    const ddlState_t *v19; // eax
    const ddlState_t *v20; // eax
    const ddlState_t *v21; // eax
    unsigned int v22; // eax
    const ddlState_t *v23; // eax
    unsigned int v24; // eax
    const ddlState_t *v25; // eax
    unsigned int v26; // eax
    const ddlState_t *v27; // eax
    const ddlState_t *v28; // eax
    unsigned int v29; // eax
    const ddlState_t *v30; // eax
    unsigned int v31; // eax
    const ddlState_t *v32; // eax
    const ddlState_t *v33; // eax
    const ddlState_t *v34; // eax
    const ddlState_t *v35; // eax
    const ddlState_t *v36; // eax
    const ddlState_t *v37; // eax
    unsigned int v38; // eax
    const ddlState_t *v39; // eax
    unsigned int v40; // eax
    const ddlState_t *v41; // eax
    unsigned int v42; // eax
    char *v43; // [esp-10h] [ebp-9DC4h]
    char *v44; // [esp-10h] [ebp-9DC4h]
    char *v45; // [esp-10h] [ebp-9DC4h]
    const char *v46; // [esp-10h] [ebp-9DC4h]
    const char *v47; // [esp-10h] [ebp-9DC4h]
    char *v48; // [esp-Ch] [ebp-9DC0h]
    char *v49; // [esp-Ch] [ebp-9DC0h]
    char *v50; // [esp-8h] [ebp-9DBCh]
    char *v51; // [esp-8h] [ebp-9DBCh]
    char *v52; // [esp-8h] [ebp-9DBCh]
    char *v53; // [esp-8h] [ebp-9DBCh]
    char *v54; // [esp-8h] [ebp-9DBCh]
    char *v55; // [esp-8h] [ebp-9DBCh]
    const char *v56; // [esp-8h] [ebp-9DBCh]
    const char *v57; // [esp-8h] [ebp-9DBCh]
    const char *v58; // [esp-8h] [ebp-9DBCh]
    const char *v59; // [esp-8h] [ebp-9DBCh]
    char *v60; // [esp-8h] [ebp-9DBCh]
    char *v61; // [esp-8h] [ebp-9DBCh]
    char *v62; // [esp-8h] [ebp-9DBCh]
    char *v63; // [esp-8h] [ebp-9DBCh]
    const char *v64; // [esp-4h] [ebp-9DB8h]
    const char *v65; // [esp-4h] [ebp-9DB8h]
    const char *v66; // [esp-4h] [ebp-9DB8h]
    const char *v67; // [esp-4h] [ebp-9DB8h]
    const char *v68; // [esp-4h] [ebp-9DB8h]
    __int64 v69; // [esp+4h] [ebp-9DB0h]
    __int64 v70; // [esp+Ch] [ebp-9DA8h]
    __int64 v71; // [esp+14h] [ebp-9DA0h]
    __int64 v72; // [esp+1Ch] [ebp-9D98h]
    __int64 v73; // [esp+24h] [ebp-9D90h]
    float sortKey; // [esp+2Ch] [ebp-9D88h]
    int v75; // [esp+30h] [ebp-9D84h]
    __int64 v76; // [esp+34h] [ebp-9D80h]
    bool v77; // [esp+43h] [ebp-9D71h]
    bool v78; // [esp+4Bh] [ebp-9D69h]
    bool v79; // [esp+53h] [ebp-9D61h]
    bool v80; // [esp+5Bh] [ebp-9D59h]
    bool v81; // [esp+63h] [ebp-9D51h]
    bool v82; // [esp+6Fh] [ebp-9D45h]
    bool v83; // [esp+77h] [ebp-9D3Dh]
    bool v84; // [esp+7Fh] [ebp-9D35h]
    int v85; // [esp+84h] [ebp-9D30h]
    int j; // [esp+88h] [ebp-9D2Ch]
    int v87; // [esp+8Ch] [ebp-9D28h]
    int i; // [esp+90h] [ebp-9D24h]
    char backupBuffer[40172]; // [esp+94h] [ebp-9D20h] BYREF
    int HeaderVersion; // [esp+9D84h] [ebp-30h]
    const char *PlayerStatStringByKey; // [esp+9D88h] [ebp-2Ch]
    char *buffer; // [esp+9D8Ch] [ebp-28h]
    int v93; // [esp+9D90h] [ebp-24h]
    const char *v94; // [esp+9D94h] [ebp-20h]
    ddlState_t resultState; // [esp+9D98h] [ebp-1Ch] BYREF
    sortedItemsData_t *v96; // [esp+9DA8h] [ebp-Ch]
    int v97; // [esp+9DACh] [ebp-8h]
    char v98; // [esp+9DB3h] [ebp-1h]
    int v99; // [esp+9DC8h] [ebp+14h]

    v98 = 0;
    PlayerStatStringByKey = 0;
    v94 = 0;
    v93 = 0;
    v97 = 0;
    if ( index >= 256
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_combatrecord.cpp",
                    282,
                    0,
                    "%s",
                    "index < MAX_UNLOCKABLE_ITEMS") )
    {
        __debugbreak();
    }
    if ( index < 256 )
    {
        if ( param != -1 )
            PlayerStatStringByKey = LiveStats_GetPlayerStatStringByKey(param);
        if ( divisionParam != -1 )
            v94 = LiveStats_GetPlayerStatStringByKey(divisionParam);
        if ( forOtherPlayer )
        {
            buffer = (char *)LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_OTHERPLAYER, 1);
            v96 = s_otherPlayerSortedItemList;
            HeaderVersion = DDL_GetHeaderVersion(buffer);
            if ( LiveStats_GetStatsDDL() && LiveStats_GetStatsDDL()->version != HeaderVersion )
            {
                StatsDDL = LiveStats_GetStatsDDL();
                if ( !DDL_FixBufferVersion(buffer, StatsDDL, "ddl_mp/stats_archive.ddl", backupBuffer, 40168) )
                {
                    v8 = LiveStats_GetStatsDDL();
                    if ( !DDL_FixBufferVersion(buffer, v8, "ddl_mp/stats.ddl", backupBuffer, 40168) )
                    {
                        v9 = LiveStats_GetStatsDDL();
                        Com_PrintError(
                            16,
                            "DDL: Could not convert other player stats from version %d to version %d.\n",
                            HeaderVersion,
                            v9->version);
                        v98 = 1;
                    }
                }
                if ( !v98 )
                {
                    v10 = LiveStats_GetStatsDDL();
                    Com_PrintWarning(
                        16,
                        "DDL: Converted other player stats from version %d to version %d.\n",
                        HeaderVersion,
                        v10->version);
                }
            }
        }
        else
        {
            buffer = (char *)LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
            v96 = s_sortedItemList;
        }
        switch ( criterion )
        {
            case PERSONAL_BESTS:
                if ( !v98 )
                {
                    v64 = combatRecordPersonalBestStats[itemNumber];
                    RootDDLState = LiveStats_GetRootDDLState();
                    if ( itemNumber >= 3 )
                        DDL_MoveTo(RootDDLState, &resultState, 2, "PlayerStatsList", v64);
                    else
                        DDL_MoveTo(RootDDLState, &resultState, 2, "PersonalBests", v64);
                    if ( !forOtherPlayer )
                    {
                        v84 = xblive_basictraining && xblive_basictraining->current.enabled
                             || LiveStats_GetBasicTrainingState((unsigned __int8 *)buffer);
                        if ( !v84 && resultState.member->permission == 2 )
                            buffer = (char *)LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_GLOBAL, 1);
                    }
                    v96[index].itemIndex = itemNumber;
                    Int = DDL_GetInt(&resultState, buffer);
                    v96[index].param1 = Int;
                    v76 = DDL_GetInt(&resultState, buffer);
                    v96[index].sortKey = (float)v76;
                }
                break;
            case RECENT_PERFORMANCE:
            case FAVORITE_WEAPON:
                if ( !v98 )
                {
                    LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_GAMETYPE);
                    v50 = va("%d", itemNumber);
                    v13 = LiveStats_GetRootDDLState();
                    DDL_MoveTo(v13, &resultState, 3, "RecentScores", v50);
                    if ( !forOtherPlayer )
                    {
                        v83 = xblive_basictraining && xblive_basictraining->current.enabled
                             || LiveStats_GetBasicTrainingState((unsigned __int8 *)buffer);
                        if ( !v83 && resultState.member->permission == 2 )
                            buffer = (char *)LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_GLOBAL, 1);
                    }
                    v14 = DDL_GetInt(&resultState, buffer);
                    v96[index].param2 = v14;
                    if ( criterion == RECENT_PERFORMANCE )
                    {
                        v51 = va("%d", itemNumber);
                        v15 = LiveStats_GetRootDDLState();
                        DDL_MoveTo(v15, &resultState, 3, "RecentScores", v51);
                        v16 = DDL_GetInt(&resultState, buffer);
                        v96[index].param1 = v16;
                    }
                    v52 = va("%d", itemNumber);
                    v17 = LiveStats_GetRootDDLState();
                    DDL_MoveTo(v17, &resultState, 3, "RecentScores", v52);
                    v18 = DDL_GetInt(&resultState, buffer);
                    v96[index].itemIndex = v18;
                    LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_KILLS);
                    v53 = va("%d", itemNumber);
                    v19 = LiveStats_GetRootDDLState();
                    DDL_MoveTo(v19, &resultState, 3, "RecentScores", v53);
                    v93 = DDL_GetInt(&resultState, buffer);
                    LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_DEATHS);
                    v54 = va("%d", itemNumber);
                    v20 = LiveStats_GetRootDDLState();
                    DDL_MoveTo(v20, &resultState, 3, "RecentScores", v54);
                    v97 = DDL_GetInt(&resultState, buffer);
                    if ( v93 || v97 )
                    {
                        if ( v97 > 1 )
                            v75 = v97;
                        else
                            v75 = 1;
                        v96[index].sortKey = (float)v93 / (float)v75;
                    }
                    else
                    {
                        v96[index].sortKey = 1.0f;
                    }
                    if ( v96[index].itemIndex )
                        ++v96[10].itemIndex;
                }
                break;
            case WEAPON_STATS:
            case WEAPON_ACCURACY:
            case WEAPON_HEADSHOTS:
            case EQUIPMENT_KILLS:
            case EQUIPMENT_USAGE:
            case TIME_SPENT_USING_ITEM:
            case KILLSTREAKS_FAVORITES:
            case KILLSTREAKS_CALLED:
            case KILLSTREAKS_KILLS:
                v96[index].itemIndex = itemNumber;
                if ( !v98 )
                {
                    v43 = va("%d", itemNumber);
                    v21 = LiveStats_GetRootDDLState();
                    DDL_MoveTo(v21, &resultState, 5, "ItemStats", v43);
                    if ( !forOtherPlayer )
                    {
                        v82 = xblive_basictraining && xblive_basictraining->current.enabled
                             || LiveStats_GetBasicTrainingState((unsigned __int8 *)buffer);
                        if ( !v82 && resultState.member->permission == 2 )
                            buffer = (char *)LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_GLOBAL, 1);
                    }
                    v22 = DDL_GetInt(&resultState, buffer);
                    v96[index].param1 = v22;
                    v44 = va("%d", itemNumber);
                    v23 = LiveStats_GetRootDDLState();
                    DDL_MoveTo(v23, &resultState, 5, "ItemStats", v44);
                    v24 = DDL_GetInt(&resultState, buffer);
                    v96[index].param3 = v24;
                    v96[index].sortKey = (float)v96[index].param1;
                    if ( v94 )
                    {
                        v45 = va("%d", itemNumber);
                        v25 = LiveStats_GetRootDDLState();
                        DDL_MoveTo(v25, &resultState, 5, "ItemStats", v45);
                        v26 = DDL_GetInt(&resultState, buffer);
                        v96[index].param2 = v26;
                        if ( v96[index].param2 )
                        {
                            v96[index].sortKey = (float)v96[index].param1 / (float)v96[index].param2;
                        }
                        else if ( v96[index].param1 )
                        {
                            v96[index].sortKey = 2.1474836e9;
                        }
                        else
                        {
                            v96[index].sortKey = 0.0f;
                        }
                    }
                    if ( criterion == WEAPON_ACCURACY )
                    {
                        if ( (float)(v96[index].sortKey - 1.0) < 0.0 )
                            sortKey = v96[index].sortKey;
                        else
                            sortKey = 1.0f;
                        v96[index].sortKey = sortKey;
                    }
                }
                break;
            case WEAPON_HITLOCATION_HEATMAP:
                for ( i = 0; i < 10; ++i )
                {
                    v55 = va("%d", i);
                    v27 = LiveStats_GetRootDDLState();
                    DDL_MoveTo(v27, &resultState, 4, "RecentHitLocCounts", v55);
                    if ( !forOtherPlayer )
                    {
                        v81 = xblive_basictraining && xblive_basictraining->current.enabled
                             || LiveStats_GetBasicTrainingState((unsigned __int8 *)buffer);
                        if ( !v81 && resultState.member->permission == 2 )
                            buffer = (char *)LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_GLOBAL, 1);
                    }
                    if ( !DDL_GetInt(&resultState, buffer) )
                        break;
                    va("%d", index);
                    v48 = va("%d", i);
                    v28 = LiveStats_GetRootDDLState();
                    DDL_MoveTo(v28, &resultState, 4, "RecentHitLocCounts", v48);
                    v29 = DDL_GetInt(&resultState, buffer);
                    v96[index].param1 += v29;
                    va("%d", index);
                    v49 = va("%d", i);
                    v30 = LiveStats_GetRootDDLState();
                    DDL_MoveTo(v30, &resultState, 4, "RecentHitLocCounts", v49);
                    v31 = DDL_GetInt(&resultState, buffer);
                    v96[index].param2 += v31;
                }
                break;
            case MATCH_TYPES:
                v96[index].itemIndex = itemNumber;
                if ( !v98 )
                {
                    v65 = LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_WINS);
                    v56 = lbTypeEnum_5[itemNumber];
                    v32 = LiveStats_GetRootDDLState();
                    DDL_MoveTo(v32, &resultState, 3, "PlayerStatsByGameMode", v56, v65);
                    if ( !forOtherPlayer )
                    {
                        v80 = xblive_basictraining && xblive_basictraining->current.enabled
                             || LiveStats_GetBasicTrainingState((unsigned __int8 *)buffer);
                        if ( !v80 && resultState.member->permission == 2 )
                            buffer = (char *)LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_GLOBAL, 1);
                    }
                    v73 = DDL_GetInt(&resultState, buffer);
                    v96[index].sortKey = (float)v73;
                    v66 = LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_LOSSES);
                    v57 = lbTypeEnum_5[itemNumber];
                    v33 = LiveStats_GetRootDDLState();
                    DDL_MoveTo(v33, &resultState, 3, "PlayerStatsByGameMode", v57, v66);
                    v72 = DDL_GetInt(&resultState, buffer);
                    v96[index].sortKey = (double)v72 + v96[index].sortKey;
                }
                break;
            case WAGER_MATCH_TYPES:
                v96[index].itemIndex = itemNumber;
                if ( !v98 )
                {
                    v67 = LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_WINS);
                    v58 = lbWagerGameModeEnum_5[itemNumber];
                    v34 = LiveStats_GetRootDDLState();
                    DDL_MoveTo(v34, &resultState, 3, "PlayerStatsByGameMode", v58, v67);
                    if ( !forOtherPlayer )
                    {
                        v79 = xblive_basictraining && xblive_basictraining->current.enabled
                             || LiveStats_GetBasicTrainingState((unsigned __int8 *)buffer);
                        if ( !v79 && resultState.member->permission == 2 )
                            buffer = (char *)LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_GLOBAL, 1);
                    }
                    v71 = DDL_GetInt(&resultState, buffer);
                    v96[index].sortKey = (float)v71;
                    v68 = LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_LOSSES);
                    v59 = lbWagerGameModeEnum_5[itemNumber];
                    v35 = LiveStats_GetRootDDLState();
                    DDL_MoveTo(v35, &resultState, 3, "PlayerStatsByGameMode", v59, v68);
                    v70 = DDL_GetInt(&resultState, buffer);
                    v96[index].sortKey = (double)v70 + v96[index].sortKey;
                }
                break;
            case MATCH_PERFORMANCE:
                if ( !v98 )
                {
                    v87 = 0;
                    v85 = 0;
                    for ( j = 0; j < 10; ++j )
                    {
                        v99 = j + 10 * itemNumber;
                        if ( v99 >= 256
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_combatrecord.cpp",
                                        513,
                                        0,
                                        "%s",
                                        "index < MAX_UNLOCKABLE_ITEMS") )
                        {
                            __debugbreak();
                        }
                        LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_SCORE);
                        v60 = va("%d", j);
                        v46 = lbTypeEnum_5[itemNumber];
                        v36 = LiveStats_GetRootDDLState();
                        DDL_MoveTo(v36, &resultState, 5, "PlayerStatsByGameMode", v46, "prevScores", v60);
                        if ( !forOtherPlayer )
                        {
                            v78 = xblive_basictraining && xblive_basictraining->current.enabled
                                 || LiveStats_GetBasicTrainingState((unsigned __int8 *)buffer);
                            if ( !v78 && resultState.member->permission == 2 )
                                buffer = (char *)LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_GLOBAL, 1);
                        }
                        v69 = DDL_GetInt(&resultState, buffer);
                        v96[v99].sortKey = (float)v69;
                        v61 = va("%d", j);
                        v47 = lbTypeEnum_5[itemNumber];
                        v37 = LiveStats_GetRootDDLState();
                        DDL_MoveTo(v37, &resultState, 5, "PlayerStatsByGameMode", v47, "prevScores", v61);
                        v38 = DDL_GetInt(&resultState, buffer);
                        v96[v99].itemIndex = v38;
                        if ( !v96[v99].itemIndex )
                            break;
                        if ( !j )
                        {
                            v87 = (int)v96[v99].sortKey;
                            v85 = v87;
                        }
                        if ( v96[v99].sortKey > (float)v85 )
                            v85 = (int)v96[v99].sortKey;
                        if ( (float)v87 > v96[v99].sortKey )
                            v87 = (int)v96[v99].sortKey;
                    }
                    v96[10 * itemNumber].itemIndex = j;
                    v96[10 * itemNumber].param1 = v87;
                    v96[10 * itemNumber].param2 = v85;
                }
                break;
            case RECENT_EARNINGS:
                LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_SCORE);
                v62 = va("%d", itemNumber);
                v39 = LiveStats_GetRootDDLState();
                DDL_MoveTo(v39, &resultState, 3, "RecentEarnings", v62);
                if ( !forOtherPlayer )
                {
                    v77 = xblive_basictraining && xblive_basictraining->current.enabled
                         || LiveStats_GetBasicTrainingState((unsigned __int8 *)buffer);
                    if ( !v77 && resultState.member->permission == 2 )
                        buffer = (char *)LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_GLOBAL, 1);
                }
                v40 = DDL_GetInt(&resultState, buffer);
                v96[itemNumber].param1 = v40;
                v63 = va("%d", itemNumber);
                v41 = LiveStats_GetRootDDLState();
                DDL_MoveTo(v41, &resultState, 3, "RecentEarnings", v63);
                v42 = DDL_GetInt(&resultState, buffer);
                v96[itemNumber].itemIndex = v42;
                break;
            default:
                return;
        }
    }
    else
    {
        Com_PrintError(16, "LiveStats_BuildSortedItem: Sorted Item Buffer Overflow.");
    }
}

bool __cdecl LiveCombatRecord_FilterCurrentSortedItem(
                int controllerIndex,
                itemSortingCriteria_t criterion,
                int itemNumber,
                sortedStatLoadoutSlots_t currLoadout)
{
    bool v5; // [esp+0h] [ebp-40h]
    bool v6; // [esp+4h] [ebp-3Ch]
    bool v7; // [esp+8h] [ebp-38h]
    bool v8; // [esp+Ch] [ebp-34h]
    bool v9; // [esp+10h] [ebp-30h]
    bool v10; // [esp+14h] [ebp-2Ch]
    bool v11; // [esp+18h] [ebp-28h]
    bool v12; // [esp+1Ch] [ebp-24h]
    bool v13; // [esp+20h] [ebp-20h]
    bool v14; // [esp+24h] [ebp-1Ch]
    bool v15; // [esp+28h] [ebp-18h]
    loadoutSlot_t loadoutSlot; // [esp+30h] [ebp-10h]
    int cost; // [esp+34h] [ebp-Ch]
    itemGroup_t group; // [esp+38h] [ebp-8h]
    bool isItemValid; // [esp+3Eh] [ebp-2h]
    bool isPassive; // [esp+3Fh] [ebp-1h]

    if ( !BG_UnlockablesIsItemValidNotNull(itemNumber) )
        return 0;
    if ( BG_UnlockablesIsItemClassifiedGeneric(controllerIndex, itemNumber) )
        return 0;
    isItemValid = 0;
    loadoutSlot = (loadoutSlot_t)BG_UnlockablesGetItemLoadoutSlot(itemNumber);
    group = (itemGroup_t)BG_UnlockablesGetItemGroupEnum(itemNumber);
    isPassive = BG_UnlockablesIsItemPassive(itemNumber);
    cost = BG_UnlockablesGetItemCost(itemNumber);
    switch ( criterion )
    {
        case WEAPON_STATS:
            v15 = loadoutSlot == LOADOUTSLOT_FIRST
                 || loadoutSlot == LOADOUTSLOT_SECONDARY_WEAPON
                 || group == ITEMGROUP_MASTERKEY
                 || group == ITEMGROUP_GRENADELAUNCHER
                 || group == ITEMGROUP_FLAMETHROWER;
            v14 = v15 && cost != -1 && !isPassive;
            isItemValid = v14;
            break;
        case WEAPON_ACCURACY:
        case WEAPON_HEADSHOTS:
            v13 = (loadoutSlot == LOADOUTSLOT_FIRST || loadoutSlot == LOADOUTSLOT_SECONDARY_WEAPON)
                 && group != ITEMGROUP_LAUNCHER
                 && group != ITEMGROUP_SPECIAL;
            v12 = v13 && cost != -1;
            isItemValid = v12;
            break;
        case EQUIPMENT_KILLS:
            v11 = (loadoutSlot == LOADOUTSLOT_PRIMARY_GRENADE
                    || loadoutSlot == LOADOUTSLOT_SPECIAL_GRENADE
                    || group == ITEMGROUP_EXPLOSIVE
                    || group == ITEMGROUP_MISCWEAPON)
                 && !isPassive;
            v10 = v11 && cost != -1;
            isItemValid = v10;
            break;
        case EQUIPMENT_USAGE:
            v9 = loadoutSlot == LOADOUTSLOT_PRIMARY_GRENADE
                || loadoutSlot == LOADOUTSLOT_SPECIAL_GRENADE
                || loadoutSlot == LOADOUTSLOT_EQUIPMENT;
            v8 = v9 && cost != -1;
            isItemValid = v8;
            break;
        case TIME_SPENT_USING_ITEM:
            v5 = loadoutSlot == sortedStatLoadoutSlots[currLoadout] && cost != -1;
            isItemValid = v5;
            break;
        case KILLSTREAKS_FAVORITES:
        case KILLSTREAKS_CALLED:
            v7 = group == ITEMGROUP_GLOBAL_ITEMS_START && cost != -1;
            isItemValid = v7;
            break;
        case KILLSTREAKS_KILLS:
            v6 = group == ITEMGROUP_GLOBAL_ITEMS_START && !isPassive && cost != -1
                || group == ITEMGROUP_GLOBAL_ITEMS_START && isPassive && cost == -1;
            isItemValid = v6;
            break;
        default:
            return isItemValid;
    }
    return isItemValid;
}

