#include "ui_feeders_mp.h"
#include <qcommon/com_clients.h>
#include "ui_gametype_variants_mp.h"
#include <qcommon/com_gamemodes.h>
#include <bgame/bg_unlockable_items.h>
#include <live/live_friends_pc.h>
#include <ui/ui_friends.h>
#include <live/live_contracts.h>
#include <live/live_stats.h>
#include <live/live_presence_win.h>
#include <ui/ui_feeders.h>
#include <live/live_pcache_profile.h>
#include <client/cl_rank.h>
#include <live/live_clans.h>
#include "ui_gametype_custom_mp.h"
#include <live/live_win.h>
#include "ui_main_mp.h"
#include <ui/ui_playlists.h>
#include <ui/ui_main_pc.h>
#include <cgame_mp/cg_local_mp.h>

float playerColor[4] = { 0.94999999, 0.72000003, 0.20999999, 1.0 };
float alliesColor[4] = { 0.80000001, 0.80000001, 0.89999998, 0.5 };
float axisColor[4] = { 0.89999998, 0.80000001, 0.80000001, 0.5 };
float fadedBlack[4] = { 0.0, 0.0, 0.0, 0.15000001 };
float white[4] = { 1.0, 1.0, 1.0, 1.0 };
float darkRowMultiplier = 0.75;

FriendInfo friendInfo_0;
FriendInfo info_5;

int __cdecl UI_GetMilestoneType(float feederID)
{
    int result; // eax

    switch ( (int)feederID )
    {
        case '\\':
            result = 1;
            break;
        case '^':
            result = 4;
            break;
        case '_':
            result = 0;
            break;
        case '`':
            result = 5;
            break;
        case 'b':
            result = 6;
            break;
        case 'c':
            result = 7;
            break;
        default:
            result = -1;
            break;
    }
    return result;
}

int __cdecl UI_Project_FeederCount(int localClientNum, int contextIndex, float feederID, listBoxDef_s *listPtr)
{
    int result; // eax
    int v5; // [esp+4h] [ebp-4Ch]
    int integer; // [esp+Ch] [ebp-44h]
    signed int NumPersonalBests; // [esp+10h] [ebp-40h]
    int friendCount; // [esp+3Ch] [ebp-14h]
    int milestoneType; // [esp+40h] [ebp-10h]
    int numWeaponOptions; // [esp+48h] [ebp-8h]
    int controllerIndex; // [esp+4Ch] [ebp-4h]

    if ( localClientNum == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp",
                    294,
                    0,
                    "%s",
                    "localClientNum != INVALID_CLIENT_NUMBER") )
    {
        __debugbreak();
    }
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( controllerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp",
                    299,
                    0,
                    "%s",
                    "controllerIndex != INVALID_CONTROLLER_PORT") )
    {
        __debugbreak();
    }
    switch ( (int)feederID )
    {
        case 1:
            result = sharedUiInfo.mapCount;
            break;
        case 34:
            result = 0;
            break;
        case 35:
        case 36:
        case 37:
        case 39:
        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
            result = UI_FeederCount_GameVariants(contextIndex, feederID);
            break;
        case 38:
            result = sharedUiInfo.numCustomMatchGameTypes;
            break;
        case 48:
            if ( Com_GameMode_IsGameMode(GAMEMODE_BASIC_TRAINING) )
                goto LABEL_11;
            if ( Com_GameMode_IsGameMode(GAMEMODE_FIRST_PRIVATE_ONLINE_GAMEMODE) )
                result = sharedUiInfo.numCustomMatchGameTypes + 1;
            else
                result = sharedUiInfo.numCustomMatchGameTypes;
            break;
        case 49:
        case 58:
        case 97:
            sharedUiInfo.numAttachments = BG_UnlockablesGetGroupItemCount();
            if ( listPtr->cursorPos[contextIndex] >= sharedUiInfo.numAttachments
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp",
                            377,
                            0,
                            "%s",
                            "listPtr->cursorPos[contextIndex] < sharedUiInfo.sortedItemPivot") )
            {
                __debugbreak();
            }
            sharedUiInfo.itemIndex = BG_UnlockablesGetItemIndexInSlot(listPtr->cursorPos[contextIndex]);
            sharedUiInfo.itemIndex = listPtr->cursorPos[contextIndex];
            result = sharedUiInfo.numAttachments;
            break;
        case 55:
            friendCount = Friends_GetCount(controllerIndex, 0);
            Dvar_SetBoolIfChanged((dvar_s *)ui_friendCountNotZero, friendCount > 0);
            result = friendCount;
            break;
        case 57:
            if ( sharedUiInfo.itemIndex == -1 )
                goto LABEL_55;
            sharedUiInfo.attachmentNum = BG_UnlockablesGetNumItemAttachmentsWithAttachPoint(
                                                                         sharedUiInfo.itemIndex,
                                                                         attachmentFilter->current.integer);
            if ( sharedUiInfo.attachmentNum == 1 )
                result = 1;
            else
                result = sharedUiInfo.attachmentNum - 1;
            break;
        case 59:
        case 65:
        case 66:
        case 67:
        case 100:
            sharedUiInfo.numAttachments = BG_UnlockablesGetGroupItemCount();
            result = sharedUiInfo.numAttachments;
            break;
        case 60:
            numWeaponOptions = 0;
            if ( ui_currentWeaponOptionGroup )
                numWeaponOptions = BG_GetWeaponOptionCountForGroup((eWeaponOptionGroup)ui_currentWeaponOptionGroup->current.integer);
            result = numWeaponOptions;
            break;
        case 61:
            sharedUiInfo.numAttachments = BG_UnlockablesGetGroupItemCount() + 1;
            result = sharedUiInfo.numAttachments;
            break;
        case 68:
            result = LiveContracts_GetContractsCountForList();
            break;
        case 71:
        case 81:
            result = LiveCombatRecord_GetGameTypeCount();
            break;
        case 72:
            integer = ui_numPersonalBests->current.integer;
            NumPersonalBests = LiveStats_GetNumPersonalBests(controllerIndex);
            if ( integer < NumPersonalBests )
                v5 = integer;
            else
                v5 = NumPersonalBests;
            result = v5;
            break;
        case 78:
            result = UI_FeederCount_WeaponGroups();
            break;
        case 82:
        case 83:
            result = CG_GetMatchScoreboardClientCount(localClientNum, TEAM_NUM_TEAMS);
            break;
        case 87:
        case 105:
            result = CG_GetMatchScoreboardClientCount(localClientNum, TEAM_ALLIES);
            break;
        case 88:
        case 106:
            result = CG_GetMatchScoreboardClientCount(localClientNum, TEAM_AXIS);
            break;
        case 89:
            result = CG_GetMatchScoreboardClientCount(localClientNum, TEAM_FREE);
            break;
        case 90:
            result = BG_UnlockablesGetClanTagFeatureCount();
            break;
        case 92:
        case 94:
        case 95:
        case 96:
        case 98:
        case 99:
            milestoneType = UI_GetMilestoneType(feederID);
            if ( milestoneType == -1 )
                result = 0;
            else
                result = LiveStats_GetChallengeCount(milestoneType);
            break;
        case 93:
            result = UI_FeederCount_CustomClasses(localClientNum);
            break;
        case 102:
            result = 1;
            break;
        case 103:
            result = UI_FeederCount_KillstreakNumKills();
            break;
        case 104:
            if ( xblive_basictraining->current.enabled )
LABEL_11:
                result = UI_GetBasicTrainingGameTypeCount();
            else
                result = sharedUiInfo.numCustomMatchGameTypes;
            break;
        case 107:
            result = (int)((float)((float)CG_GetMatchScoreboardClientCount(localClientNum, TEAM_FREE) * 0.5)
                                     + 0.4999999990686774);
            break;
        case 108:
            result = (int)((float)((float)CG_GetMatchScoreboardClientCount(localClientNum, TEAM_FREE) * 0.5)
                                     - 0.4999999990686774);
            break;
        case 109:
            result = Live_GetMapCount();
            break;
        default:
LABEL_55:
            result = 0;
            break;
    }
    return result;
}

int __cdecl UI_GetBasicTrainingGameTypeCount()
{
    int index; // [esp+0h] [ebp-8h]
    int count; // [esp+4h] [ebp-4h]

    count = 0;
    for ( index = 0; index < sharedUiInfo.numCustomMatchGameTypes; ++index )
    {
        if ( sharedUiInfo.gameTypeMapCount[29 * index - 901] )
            ++count;
    }
    return count;
}

int __cdecl UI_FeederCount_WeaponGroups()
{
    const char *loadoutName; // [esp+0h] [ebp-4h]

    loadoutName = Dvar_GetString("selected_loadout_slot");
    if ( !I_stricmp(loadoutName, "primary") )
        return 5;
    if ( I_stricmp(loadoutName, "secondary") )
        return 0;
    return 3;
}

int __cdecl UI_FeederCount_CustomClasses(int localClientNum)
{
    int ControllerIndex; // eax
    int v3; // [esp-4h] [ebp-20h]
    int i; // [esp+0h] [ebp-1Ch]
    int customClassCount; // [esp+4h] [ebp-18h]
    int prestigeClassIndices[5]; // [esp+8h] [ebp-14h]

    customClassCount = 4;
    prestigeClassIndices[0] = BG_UnlockablesGetItemIndexFromName("FEATURE_PRESTIGE_CLASS_1");
    prestigeClassIndices[1] = BG_UnlockablesGetItemIndexFromName("FEATURE_PRESTIGE_CLASS_2");
    prestigeClassIndices[2] = BG_UnlockablesGetItemIndexFromName("FEATURE_PRESTIGE_CLASS_3");
    prestigeClassIndices[3] = BG_UnlockablesGetItemIndexFromName("FEATURE_PRESTIGE_CLASS_4");
    prestigeClassIndices[4] = BG_UnlockablesGetItemIndexFromName("FEATURE_PRESTIGE_CLASS_5");
    for ( i = 0; i < 5; ++i )
    {
        v3 = prestigeClassIndices[i];
        ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        if ( !BG_UnlockablesIsItemLocked(ControllerIndex, v3) )
            ++customClassCount;
    }
    return customClassCount;
}

int __cdecl UI_FeederCount_KillstreakNumKills()
{
    return 22;
}

char *__cdecl UI_FeederItemText_Maps(int controllerIndex, itemDef_s *item, int index, int column, Material **handle)
{
    const char *v5; // eax
    listBoxDef_s *listboxPtr; // [esp+4h] [ebp-Ch]
    int contextIndex; // [esp+Ch] [ebp-4h]

    contextIndex = Com_ControllerIndex_GetUIContextIndex(controllerIndex);
    listboxPtr = Item_GetListBoxDef(item);
    if ( column )
    {
        if ( column == 1 )
        {
            if ( index < 0 || index >= sharedUiInfo.mapCount )
            {
                return (char *)"";
            }
            else
            {
                v5 = va("%s_CAPS", sharedUiInfo.mapList[index].mapName);
                return UI_SafeTranslateString(v5);
            }
        }
        else if ( column == 2 )
        {
            if (listboxPtr->cursorPos[contextIndex] == index && Window_HasFocus(contextIndex, &item->window))
            {
                //BLOPS_NULLSUB();
            }
            return (char *)"";
        }
        else
        {
            return (char *)"";
        }
    }
    else
    {
        *handle = sharedUiInfo.assets.whiteMaterial;
        return (char *)"";
    }
}

char *__cdecl UI_FeederItemText_CustomGametypes(int index)
{
    if ( index < 0 || index >= sharedUiInfo.numCustomMatchGameTypes )
        return (char *)"";
    else
        return UI_SafeTranslateString((const char *)&sharedUiInfo.gameTypeMapCount[29 * index - 926]);
}

const char *__cdecl UI_FeederItemText_GametypesBase(
                int contextIndex,
                int controllerIndex,
                int index,
                int column,
                itemDef_s *item,
                listBoxDef_s *listPtr,
                Material **handle)
{
    const char *result = NULL; // eax
    int LocalClientNum; // eax
    float feederID; // [esp+0h] [ebp-14h]
    int realIndex; // [esp+10h] [ebp-4h]
    int indexa; // [esp+24h] [ebp+10h]

    realIndex = index;
    switch ( column )
    {
        case 0:
            *handle = sharedUiInfo.assets.whiteMaterial;
            result = "";
            break;
        case 1:
            if ( xblive_basictraining->current.enabled )
            {
                indexa = UI_GetBasicTrainingGametypeIdForNum(index);
                if ( indexa >= 0 && indexa < sharedUiInfo.numCustomMatchGameTypes )
                    result = UI_SafeTranslateString((const char *)&sharedUiInfo.gameTypeMapCount[29 * indexa - 926]);
                else
                    result = "";
            }
            else
            {
                if ( index < 0 )
                    return "";
                feederID = listPtr->special;
                LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
                if ( index >= (int)UI_FeederCount(LocalClientNum, contextIndex, feederID, listPtr) )
                    return "";
                if ( Com_GameMode_IsGameMode(GAMEMODE_FIRST_PRIVATE_ONLINE_GAMEMODE) )
                {
                    if ( !index )
                        return UI_SafeTranslateString("CUSTOM_CUSTOM_GAME_MODE_CAPS");
                    realIndex = index - 1;
                }
                result = UI_SafeTranslateString((const char *)&sharedUiInfo.gameTypeMapCount[29 * realIndex - 926]);
            }
            break;
        case 2:
            if (listPtr->cursorPos[contextIndex] == index && Window_HasFocus(contextIndex, &item->window))
            {
                //BLOPS_NULLSUB();
            }
            result = "";
            break;
        case 3:
            if ( Com_GameMode_IsGameMode(GAMEMODE_FIRST_PRIVATE_ONLINE_GAMEMODE) )
                realIndex = index - 1;
            if ( !I_strcmp(&sharedUiInfo.customGameTypes[32].gameType[116 * realIndex], ui_gametype->current.string)
                && customGameMode->current.integer )
            {
                *handle = Material_RegisterHandle("ui_host", 3);
            }
            result = "";
            break;
        default:
            result = "";
            break;
    }
    return result;
}

int __cdecl UI_GetBasicTrainingGametypeIdForNum(int num)
{
    int index; // [esp+0h] [ebp-8h]
    int count; // [esp+4h] [ebp-4h]

    count = 0;
    for ( index = 0; index < sharedUiInfo.numCustomMatchGameTypes; ++index )
    {
        if ( sharedUiInfo.gameTypeMapCount[29 * index - 901] )
        {
            if ( count == num )
                return index;
            ++count;
        }
    }
    return 0;
}

char *__cdecl UI_FeederItemText_GametypesInGame(
                int contextIndex,
                int controllerIndex,
                int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle)
{
    int LocalClientNum; // eax
    float feederID; // [esp+0h] [ebp-Ch]

    if ( xblive_basictraining->current.enabled )
    {
        index = UI_GetBasicTrainingGametypeIdForNum(index);
        if ( index < 0 || index >= sharedUiInfo.numCustomMatchGameTypes )
            return (char *)"";
        if ( column )
        {
            if ( column == 1 )
                return UI_SafeTranslateString((const char *)&sharedUiInfo.gameTypeMapCount[29 * index - 926]);
            else
                return (char *)"";
        }
        *handle = sharedUiInfo.assets.whiteMaterial;
    }
    if ( index < 0 )
        return (char *)"";
    feederID = listPtr->special;
    LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
    if ( index >= (int)UI_FeederCount(LocalClientNum, contextIndex, feederID, listPtr) )
        return (char *)"";
    if ( column )
    {
        if ( column == 1 )
            return UI_SafeTranslateString((const char *)&sharedUiInfo.gameTypeMapCount[29 * index - 926]);
    }
    else
    {
        *handle = sharedUiInfo.assets.whiteMaterial;
    }
    return (char *)"";
}

char *__cdecl UI_FeederItemText_SystemLinkLobbyMembers(int localClientNum)
{
    clientInfo_t *ci; // [esp+4h] [ebp-4h]

    ci = CG_GetLocalClientGlobals(localClientNum)->bgs.clientinfo;
    if ( ci->infoValid )
        return ci->name;
    else
        return (char *)"";
}

char *__cdecl UI_FeederItemText_WeaponOptions(
                int localClientNum,
                int contextIndex,
                int controllerIndex,
                float feederID,
                int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle)
{
    int WeaponOptionNumFromIndexAndGroup; // eax
    const char *itemName; // [esp+8h] [ebp-20h]
    int weaponOption; // [esp+Ch] [ebp-1Ch]
    eWeaponOptionGroup currentWeaponGroup; // [esp+10h] [ebp-18h]
    int cost; // [esp+18h] [ebp-10h]
    int itemIndex; // [esp+1Ch] [ebp-Ch]
    int numWeaponOptions; // [esp+20h] [ebp-8h]
    bool itemLocked; // [esp+26h] [ebp-2h]
    bool isSelectedItem; // [esp+27h] [ebp-1h]

    currentWeaponGroup = WEAPONOPTION_GROUP_FIRST;
    if ( ui_currentWeaponOptionGroup )
        currentWeaponGroup = (eWeaponOptionGroup)ui_currentWeaponOptionGroup->current.integer;
    numWeaponOptions = BG_GetWeaponOptionCountForGroup(currentWeaponGroup);
    if ( listPtr->cursorPos[contextIndex] >= numWeaponOptions && numWeaponOptions )
    {
        listPtr->startPos[contextIndex] = 0;
        listPtr->cursorPos[contextIndex] = numWeaponOptions - 1;
        UI_FeederSelection(localClientNum, contextIndex, feederID, numWeaponOptions - 1);
    }
    itemIndex = sharedUiInfo.itemIndex;
    weaponOption = BG_GetWeaponOptionNumFromIndexAndGroup(index, currentWeaponGroup);
    cost = BG_GetWeaponOptionCost(weaponOption);
    WeaponOptionNumFromIndexAndGroup = BG_GetWeaponOptionNumFromIndexAndGroup(
                                                                             index,
                                                                             (eWeaponOptionGroup)ui_currentWeaponOptionGroup->current.integer);
    if ( BG_UnlockablesIsItemOptionPurchased(controllerIndex, itemIndex, WeaponOptionNumFromIndexAndGroup) )
        cost = 0;
    itemLocked = BG_UnlockablesIsWeaponOptionLocked(controllerIndex, weaponOption);
    isSelectedItem = listPtr->cursorPos[contextIndex] == index;
    if ( itemLocked )
        return UI_Feeder_HandleItemList(
                         controllerIndex,
                         itemIndex,
                         column,
                         isSelectedItem,
                         itemLocked,
                         0,
                         "MENU_CLASSIFIED",
                         handle);
    itemName = BG_GetWeaponOptionDisplayName(weaponOption);
    return UI_Feeder_HandleItemList(controllerIndex, itemIndex, column, isSelectedItem, 0, cost, itemName, handle);
}

char *__cdecl UI_Feeder_HandleItemList(
                int controllerIndex,
                int itemIndex,
                int column,
                bool isSelectedItem,
                bool itemLocked,
                int cost,
                const char *itemName,
                Material **handle)
{
    char *result = NULL; // eax
    char *v9; // eax

    if ( itemIndex == -1 )
        return (char *)"";
    switch ( column )
    {
        case 0:
            *handle = sharedUiInfo.assets.whiteMaterial;
            result = (char *)"";
            break;
        case 1:
            result = UI_SafeTranslateString(itemName);
            break;
        case 3:
            if ( !cost )
                return (char *)"";
            v9 = UI_SafeTranslateString("MENU_POINTS");
            result = UI_ReplaceConversionInt(v9, cost);
            break;
        case 4:
            if ( itemLocked )
                *handle = Material_RegisterHandle("menu_mp_lobby_locked", 3);
            result = (char *)"";
            break;
        case 6:
            if (isSelectedItem)
            {
                //BLOPS_NULLSUB();
            }
            result = (char *)"";
            break;
        default:
            return (char *)"";
    }
    return result;
}

void __cdecl UI_FeederItemText_CommonListColorHandler(int controllerIndex, bool isSelectedItem, bool isGrey, int cost)
{
    int codPoints; // [esp+0h] [ebp-4h]

    codPoints = LiveStats_GetCurrency(controllerIndex);
    sharedUiInfo.itemColor[0] = colorWhite[0];
    sharedUiInfo.itemColor[1] = 1.0;
    sharedUiInfo.itemColor[2] = 1.0;
    sharedUiInfo.itemColor[3] = 1.0;
    if (isSelectedItem)
    {
        sharedUiInfo.itemColor[0] = colorBlack[0];
        sharedUiInfo.itemColor[1] = 0.0;
        sharedUiInfo.itemColor[2] = 0.0;
        sharedUiInfo.itemColor[3] = 1.0;
    }
    if (cost)
    {
        if (cost <= codPoints)
        {
            sharedUiInfo.itemColor[0] = newUIGreen[0];
            sharedUiInfo.itemColor[1] = newUIGreen[1];
            sharedUiInfo.itemColor[2] = newUIGreen[2];
            sharedUiInfo.itemColor[3] = newUIGreen[3];
        }
        else
        {
            sharedUiInfo.itemColor[0] = newUIRed[0];
            sharedUiInfo.itemColor[1] = newUIRed[1];
            sharedUiInfo.itemColor[2] = newUIRed[2];
            sharedUiInfo.itemColor[3] = newUIRed[3];
        }
    }
    if (isGrey)
    {
        sharedUiInfo.itemColor[0] = newUIGrey[0];
        sharedUiInfo.itemColor[1] = newUIGrey[1];
        sharedUiInfo.itemColor[2] = newUIGrey[2];
        sharedUiInfo.itemColor[3] = newUIGrey[3];
    }
}

char *__cdecl UI_FeederItemText_Attachments(
                int contextIndex,
                int controllerIndex,
                float feederID,
                int index,
                int column,
                itemDef_s *item,
                Material **handle)
{
    int LocalClientNum; // eax
    eAttachment ItemAttachment; // eax
    const char *AttachmentPointName; // eax
    const char *v10; // eax
    char *result; // eax
    char *v12; // eax
    int v13; // [esp+4h] [ebp-50h]
    eAttachmentPoint currAttachPoint; // [esp+Ch] [ebp-48h]
    int numAttachmentsEquipped; // [esp+10h] [ebp-44h]
    eAttachment currEquippedAttachment; // [esp+14h] [ebp-40h]
    int perkProIndex; // [esp+18h] [ebp-3Ch]
    eAttachment attachment; // [esp+1Ch] [ebp-38h]
    const char *customClassName; // [esp+20h] [ebp-34h]
    unsigned int currEquippedAttachmentNum; // [esp+24h] [ebp-30h]
    const char *itemName; // [esp+28h] [ebp-2Ch]
    const char *loadoutName; // [esp+2Ch] [ebp-28h]
    bool primaryWeaponSelected; // [esp+33h] [ebp-21h]
    listBoxDef_s *listPtr; // [esp+34h] [ebp-20h]
    eAttachmentPoint attachPoint; // [esp+38h] [ebp-1Ch]
    int attachmentNum; // [esp+3Ch] [ebp-18h]
    bool isSecondAttachmentOptionEnabled; // [esp+43h] [ebp-11h]
    int perkIndex; // [esp+44h] [ebp-10h]
    int cost; // [esp+48h] [ebp-Ch]
    int itemIndex; // [esp+4Ch] [ebp-8h]
    bool isCurrentItemEquipped; // [esp+51h] [ebp-3h]
    bool isItemGreyedOut; // [esp+52h] [ebp-2h]
    bool isSelectedItem; // [esp+53h] [ebp-1h]

    listPtr = Item_GetListBoxDef(item);
    if ( listPtr->cursorPos[contextIndex] >= sharedUiInfo.attachmentNum && sharedUiInfo.attachmentNum )
    {
        listPtr->startPos[contextIndex] = 0;
        listPtr->cursorPos[contextIndex] = sharedUiInfo.attachmentNum - 1;
        v13 = sharedUiInfo.attachmentNum - 1;
        LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
        UI_FeederSelection(LocalClientNum, contextIndex, feederID, v13);
    }
    numAttachmentsEquipped = 0;
    isSecondAttachmentOptionEnabled = 0;
    isCurrentItemEquipped = 0;
    isSelectedItem = 0;
    isItemGreyedOut = 0;
    itemIndex = sharedUiInfo.itemIndex;
    if ( sharedUiInfo.attachmentNum == 1 )
        attachmentNum = BG_UnlockablesGetItemAttachmentNumWithAttachPoint(
                                            sharedUiInfo.itemIndex,
                                            index,
                                            attachmentFilter->current.integer);
    else
        attachmentNum = BG_UnlockablesGetItemAttachmentNumWithAttachPoint(
                                            sharedUiInfo.itemIndex,
                                            index + 1,
                                            attachmentFilter->current.integer);
    ItemAttachment = BG_UnlockablesGetItemAttachment(itemIndex, attachmentNum);
    attachPoint = BG_GetAttachmentPointIndexFromAttachment(ItemAttachment);
    attachment = BG_UnlockablesGetItemAttachment(itemIndex, attachmentNum);
    itemName = BG_UnlockablesGetItemAttachmentDisplayName(itemIndex, attachmentNum);
    customClassName = Dvar_GetString("ui_custom_name");
    loadoutName = Dvar_GetString("selected_loadout_slot");
    primaryWeaponSelected = I_stricmp(loadoutName, "primary") == 0;
    for ( currAttachPoint = ATTACHMENT_POINT_TOP; currAttachPoint < ATTACHMENT_POINT_COUNT; ++currAttachPoint )
    {
        AttachmentPointName = BG_GetAttachmentPointName(currAttachPoint);
        v10 = va("%sattachment%s", loadoutName, AttachmentPointName);
        currEquippedAttachmentNum = BG_UnlockablesGetEquippedItemInSlot(controllerIndex, customClassName, v10);
        currEquippedAttachment = BG_UnlockablesGetItemAttachment(itemIndex, currEquippedAttachmentNum);
        if ( !BG_AreAttachmentsCompatible(attachment, currEquippedAttachment) )
            isItemGreyedOut = 1;
        if ( currEquippedAttachment )
        {
            ++numAttachmentsEquipped;
            if ( attachment == currEquippedAttachment )
                isCurrentItemEquipped = 1;
            if ( currAttachPoint == attachPoint )
                isItemGreyedOut = 1;
        }
    }
    perkIndex = BG_UnlockablesGetProfessionalIndex();
    perkProIndex = BG_UnlockablesGetProItem(perkIndex);
    if ( BG_UnlockablesIsItemEquipped(controllerIndex, customClassName, perkIndex)
        || BG_UnlockablesIsItemEquipped(controllerIndex, customClassName, perkProIndex) )
    {
        isSecondAttachmentOptionEnabled = 1;
    }
    if ( numAttachmentsEquipped >= maxAttachmentsPerWeapon->current.integer )
        isItemGreyedOut = 1;
    if ( numAttachmentsEquipped == 1 && !isSecondAttachmentOptionEnabled
        || numAttachmentsEquipped == 1 && !primaryWeaponSelected )
    {
        isItemGreyedOut = 1;
    }
    cost = BG_UnlockablesGetItemAttachmentCost(itemIndex, attachmentNum);
    if ( BG_UnlockablesIsItemAttachmentPurchased(controllerIndex, itemIndex, attachmentNum) )
        cost = 0;
    if ( listPtr->cursorPos[contextIndex] == index )
    {
        sharedUiInfo.attachmentNum = attachmentNum;
        if ( Window_HasFocus(contextIndex, &item->window) )
            isSelectedItem = 1;
    }
    UI_FeederItemText_CommonListColorHandler(controllerIndex, isSelectedItem, isItemGreyedOut, cost);
    if ( itemIndex == -1 )
        return (char *)"";
    switch ( column )
    {
        case 0:
            result = UI_SafeTranslateString(itemName);
            break;
        case 1:
            if ( !isCurrentItemEquipped || attachment == ATTACHMENT_NONE )
                return (char *)"";
            *handle = Material_RegisterHandle("menu_mp_killstreak_select", 7);
            result = (char *)"";
            break;
        case 2:
            if ( !cost )
                return (char *)"";
            v12 = UI_SafeTranslateString("MENU_POINTS");
            result = UI_ReplaceConversionInt(v12, cost);
            break;
        case 3:
            result = (char *)"";
            break;
        default:
            return (char *)"";
    }
    return result;
}

char *__cdecl UI_ChallengeCompletionStatus(int column, int currentValue, int targetValue, bool *useOwnerDraw)
{
    if ( column == 1 )
    {
        if ( currentValue != targetValue )
        {
            *useOwnerDraw = 1;
            return (char *)"";
        }
    }
    else if ( column == 2 && currentValue == targetValue )
    {
        return UI_SafeTranslateString("STATS_CHALLENGE_COMPLETED");
    }
    return (char *)"";
}

char *__cdecl UI_PrepareStatsMilestoneName(
                int challengeIndex,
                int milestone,
                const char *statName,
                const char *itemName)
{
    char *v4; // eax
    char *v5; // eax
    const char *v6; // eax
    char *v7; // eax
    char milestoneTierRoman[8]; // [esp+0h] [ebp-438h] BYREF
    char outputString[1028]; // [esp+8h] [ebp-430h] BYREF
    ConversionArguments convArgs; // [esp+410h] [ebp-28h] BYREF

    memset(&convArgs, 0, sizeof(convArgs));
    CL_MilestoneTier_DecToRoman(milestone, milestoneTierRoman, 5);
    if ( !itemName || !statName )
        return (char *)"";
    v4 = UI_SafeTranslateString(itemName);
    convArgs.args[convArgs.argCount++] = v4;
    v5 = UI_SafeTranslateString(statName);
    convArgs.args[convArgs.argCount++] = v5;
    v6 = va("%s", milestoneTierRoman);
    convArgs.args[convArgs.argCount++] = v6;
    v7 = UI_SafeTranslateString("STATS_STATSMILESTONES");
    UI_ReplaceConversions(v7, &convArgs, outputString, 1024);
    return va("%s", outputString);
}

_CustomClassDescription *__cdecl UI_Project_FeederItemText(
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
    int v10; // eax
    const char *GameTypeName; // eax
    unsigned __int64 dwid; // [esp+4h] [ebp-90h]
    int prestige; // [esp+80h] [ebp-14h] BYREF
    int rank; // [esp+84h] [ebp-10h] BYREF
    int milestoneType; // [esp+88h] [ebp-Ch]
    int controllerIndex; // [esp+8Ch] [ebp-8h]
    listBoxDef_s *listPtr; // [esp+90h] [ebp-4h]

    listPtr = Item_GetListBoxDef(item);
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( controllerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp",
                    2392,
                    0,
                    "%s",
                    "controllerIndex != INVALID_CONTROLLER_PORT") )
    {
        __debugbreak();
    }
    switch ( (int)feederID )
    {
        case '"':
            return (_CustomClassDescription *)UI_FeederItemText_SystemLinkLobbyMembers(localClientNum);
        case '#':
        case '$':
        case '%':
        case '\'':
        case '(':
        case ')':
        case '*':
        case '+':
        case ',':
            return (_CustomClassDescription *)UI_FeederItemText_GameVariants(feederID, index, column);
        case '0':
            return (_CustomClassDescription *)UI_FeederItemText_GametypesBase(
                                                                                    contextIndex,
                                                                                    controllerIndex,
                                                                                    index,
                                                                                    column,
                                                                                    item,
                                                                                    listPtr,
                                                                                    handle);
        case '1':
            return (_CustomClassDescription *)UI_FeederItemText_CustomPerksInSlot(
                                                                                    contextIndex,
                                                                                    controllerIndex,
                                                                                    feederID,
                                                                                    index,
                                                                                    column,
                                                                                    item,
                                                                                    handle);
        case '3':
            return (_CustomClassDescription *)"";
        case '7':
            if ( !Friends_GetByIndex(controllerIndex, 0, index, &friendInfo_0) )
                return (_CustomClassDescription *)"";
            if ( column )
            {
                if ( column == 1 )
                    return (_CustomClassDescription *)friendInfo_0.name;
                else
                    return (_CustomClassDescription *)"";
            }
            else
            {
                dwid = friendInfo_0.dwid;
                v10 = Com_LocalClient_GetControllerIndex(contextIndex);
                if ( PCache_GetRank(v10, dwid, &rank, &prestige) )
                    CL_GetRankIcon(rank, prestige, handle);
                return (_CustomClassDescription *)"";
            }
        case '8':
            return (_CustomClassDescription *)"";
        case '9':
            return (_CustomClassDescription *)UI_FeederItemText_Attachments(
                                                                                    contextIndex,
                                                                                    controllerIndex,
                                                                                    feederID,
                                                                                    index,
                                                                                    column,
                                                                                    item,
                                                                                    handle);
        case ':':
        case ';':
            return (_CustomClassDescription *)UI_FeederItemText_ItemInSlot(
                                                                                    contextIndex,
                                                                                    controllerIndex,
                                                                                    feederID,
                                                                                    index,
                                                                                    column,
                                                                                    item,
                                                                                    handle);
        case '<':
            return (_CustomClassDescription *)UI_FeederItemText_WeaponOptions(
                                                                                    localClientNum,
                                                                                    contextIndex,
                                                                                    controllerIndex,
                                                                                    feederID,
                                                                                    index,
                                                                                    column,
                                                                                    listPtr,
                                                                                    handle);
        case '=':
            return (_CustomClassDescription *)UI_FeederItemText_CustomKillstreaks(
                                                                                    contextIndex,
                                                                                    controllerIndex,
                                                                                    item,
                                                                                    feederID,
                                                                                    index,
                                                                                    column,
                                                                                    listPtr);
        case '?':
            return (_CustomClassDescription *)"";
        case 'A':
        case 'B':
        case 'C':
            return (_CustomClassDescription *)UI_FeederItemText_SortedItems(
                                                                                    contextIndex,
                                                                                    controllerIndex,
                                                                                    item,
                                                                                    feederID,
                                                                                    index,
                                                                                    column,
                                                                                    listPtr,
                                                                                    handle);
        case 'D':
            return (_CustomClassDescription *)UI_FeederItemText_Contracts(
                                                                                    contextIndex,
                                                                                    controllerIndex,
                                                                                    index,
                                                                                    column,
                                                                                    item,
                                                                                    listPtr,
                                                                                    handle);
        case 'G':
        case 'Q':
            GameTypeName = LiveCombatRecord_GetGameTypeName(index);
            return (_CustomClassDescription *)UI_SafeTranslateString(GameTypeName);
        case 'H':
            return (_CustomClassDescription *)UI_FeederItemText_PersonalBests(contextIndex, controllerIndex, index, column);
        case 'N':
            return (_CustomClassDescription *)UI_FeederItemText_WeaponGroups(
                                                                                    controllerIndex,
                                                                                    contextIndex,
                                                                                    item,
                                                                                    index,
                                                                                    column,
                                                                                    handle);
        case 'R':
        case 'S':
            return (_CustomClassDescription *)UI_FeederItemText_AARScoreboard(
                                                                                    contextIndex,
                                                                                    controllerIndex,
                                                                                    index,
                                                                                    column,
                                                                                    listPtr,
                                                                                    handle,
                                                                                    feederID);
        case 'W':
            return (_CustomClassDescription *)UI_FeederItemText_InGamePlayerList(
                                                                                    contextIndex,
                                                                                    controllerIndex,
                                                                                    index,
                                                                                    column,
                                                                                    handle,
                                                                                    TEAM_ALLIES);
        case 'X':
            return (_CustomClassDescription *)UI_FeederItemText_InGamePlayerList(
                                                                                    contextIndex,
                                                                                    controllerIndex,
                                                                                    index,
                                                                                    column,
                                                                                    handle,
                                                                                    TEAM_AXIS);
        case 'Y':
            return (_CustomClassDescription *)UI_FeederItemText_InGamePlayerList(
                                                                                    contextIndex,
                                                                                    controllerIndex,
                                                                                    index,
                                                                                    column,
                                                                                    handle,
                                                                                    TEAM_FREE);
        case 'Z':
            return (_CustomClassDescription *)UI_FeederItemText_ClanTagFeatures(
                                                                                    controllerIndex,
                                                                                    contextIndex,
                                                                                    item,
                                                                                    index,
                                                                                    column,
                                                                                    handle);
        case '\\':
        case 'b':
        case 'c':
            milestoneType = UI_GetMilestoneType(feederID);
            if ( milestoneType == -1 )
                return (_CustomClassDescription *)"";
            else
                return (_CustomClassDescription *)UI_FeederItemText_StatsMilestones(
                                                                                        contextIndex,
                                                                                        controllerIndex,
                                                                                        index,
                                                                                        column,
                                                                                        listPtr,
                                                                                        handle,
                                                                                        useOwnerDraw,
                                                                                        (statsMilestoneTypes_t)milestoneType);
        case ']':
            return (_CustomClassDescription *)UI_FeederItemText_CustomClasses(controllerIndex, contextIndex, item, index);
        case '^':
            return (_CustomClassDescription *)UI_FeederItemText_WeaponGroupMilestones(
                                                                                    contextIndex,
                                                                                    controllerIndex,
                                                                                    index,
                                                                                    column,
                                                                                    listPtr,
                                                                                    handle,
                                                                                    useOwnerDraw);
        case '_':
            return (_CustomClassDescription *)UI_FeederItemText_GlobalChallenges(
                                                                                    contextIndex,
                                                                                    controllerIndex,
                                                                                    index,
                                                                                    column,
                                                                                    listPtr,
                                                                                    handle,
                                                                                    useOwnerDraw);
        case '`':
            return (_CustomClassDescription *)UI_FeederItemText_ChallengesAttachments(
                                                                                    contextIndex,
                                                                                    controllerIndex,
                                                                                    index,
                                                                                    column,
                                                                                    listPtr,
                                                                                    handle,
                                                                                    useOwnerDraw);
        case 'a':
            return (_CustomClassDescription *)UI_FeederItemText_ChallengesPerks(
                                                                                    contextIndex,
                                                                                    controllerIndex,
                                                                                    feederID,
                                                                                    index,
                                                                                    column,
                                                                                    item,
                                                                                    handle);
        case 'd':
            return (_CustomClassDescription *)UI_FeederItemText_ChallengesKillstreaks(
                                                                                    contextIndex,
                                                                                    controllerIndex,
                                                                                    item,
                                                                                    feederID,
                                                                                    index,
                                                                                    column,
                                                                                    listPtr,
                                                                                    handle);
        case 'f':
            return (_CustomClassDescription *)UI_FeederItemText_ClanTagFeaturesEdit(controllerIndex);
        case 'g':
            return (_CustomClassDescription *)UI_FeederItemText_KillstreakNumKills(
                                                                                    contextIndex,
                                                                                    controllerIndex,
                                                                                    index,
                                                                                    column,
                                                                                    listPtr,
                                                                                    handle);
        case 'h':
            return (_CustomClassDescription *)UI_FeederItemText_GametypesInGame(
                                                                                    contextIndex,
                                                                                    controllerIndex,
                                                                                    index,
                                                                                    column,
                                                                                    listPtr,
                                                                                    handle);
        case 'i':
        case 'j':
        case 'k':
        case 'l':
            return (_CustomClassDescription *)UI_FeederItemText_InGamePlayers(
                                                                                    contextIndex,
                                                                                    controllerIndex,
                                                                                    index,
                                                                                    column,
                                                                                    handle,
                                                                                    (int)feederID,
                                                                                    item);
        case 'm':
            return UI_FeederItemText_CustomGametypeClasses(controllerIndex, contextIndex, item, index);
        default:
            return (_CustomClassDescription *)"";
    }
}

char *__cdecl UI_FeederItemText_WeaponGroups(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                itemGroup_t index,
                int column,
                Material **handle)
{
    const char *ItemGroupName; // eax
    const char *v7; // eax
    const char *v8; // eax
    bool v10; // [esp+0h] [ebp-18h]
    const char *loadoutName; // [esp+4h] [ebp-14h]
    const char *itemName; // [esp+8h] [ebp-10h]
    itemGroup_t arrayOffset; // [esp+Ch] [ebp-Ch]

    v10 = Item_GetListBoxDef(item)->cursorPos[contextIndex] == index && Window_HasFocus(contextIndex, &item->window);
    loadoutName = Dvar_GetString("selected_loadout_slot");
    itemName = "";
    arrayOffset = ITEMGROUP_SMG;
    if ( !I_stricmp(loadoutName, "primary") )
    {
        arrayOffset = index;
        if ( (unsigned int)index > ITEMGROUP_SNIPER
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp",
                        977,
                        0,
                        "%s",
                        "arrayOffset >= WEAPONGROUP_PRIMARY_FIRST && arrayOffset <= WEAPONGROUP_PRIMARY_LAST") )
        {
            __debugbreak();
        }
        ItemGroupName = BG_UnlockablesGetItemGroupName(index);
        itemName = va("%s%s%s", "MPUI_", ItemGroupName, "_CAPS");
    }
    if ( !I_stricmp(loadoutName, "secondary") )
    {
        arrayOffset = (itemGroup_t)(index + 5);
        if ( (index + 5 < 5 || arrayOffset > ITEMGROUP_SPECIAL)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp",
                        984,
                        0,
                        "%s",
                        "arrayOffset >= WEAPONGROUP_SECONDARY_FIRST && arrayOffset <= WEAPONGROUP_SECONDARY_LAST") )
        {
            __debugbreak();
        }
        v7 = BG_UnlockablesGetItemGroupName(arrayOffset);
        itemName = va("%s%s%s", "MPUI_", v7, "_CAPS");
    }
    if ( column == 7 )
    {
        if ( BG_UnlockablesGetItemGroupName(arrayOffset) )
        {
            v8 = BG_UnlockablesGetItemGroupName(arrayOffset);
            if ( BG_UnlockablesIsItemGroupNew(controllerIndex, v8) )
                *handle = Material_RegisterHandle("menu_mp_lobby_new", 3);
        }
    }
    return UI_Feeder_HandleItemList(controllerIndex, 0, column, v10, 0, 0, itemName, handle);
}

const char *__cdecl UI_FeederItemText_ClanTagFeatures(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                int index,
                int column,
                Material **handle)
{
    const char *ClanTagFeature; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *result; // eax
    char *v10; // eax
    bool v11; // [esp+4h] [ebp-18h]
    int cost; // [esp+Ch] [ebp-10h]
    bool isLocked; // [esp+13h] [ebp-9h]
    unsigned int currClanTagFeature; // [esp+14h] [ebp-8h]

    cost = 0;
    currClanTagFeature = index + 1;
    v11 = Item_GetListBoxDef(item)->cursorPos[contextIndex] == index && Window_HasFocus(contextIndex, &item->window);
    if ( !BG_UnlockablesIsClanTagFeaturePurchased(controllerIndex, currClanTagFeature) )
    {
        ClanTagFeature = BG_UnlockablesGetClanTagFeature(currClanTagFeature, CLANTAG_COL_COST);
        cost = atoi(ClanTagFeature);
    }
    isLocked = BG_UnlockablesIsClanTagFeatureLocked(controllerIndex, currClanTagFeature);
    UI_FeederItemText_CommonListColorHandler(controllerIndex, v11, isLocked, cost);
    switch ( column )
    {
        case 0:
            v7 = BG_UnlockablesGetClanTagFeature(currClanTagFeature, CLANTAG_COL_NAME);
            v8 = va("MPUI_CLANTAG_%s", v7);
            return UI_SafeTranslateString(v8);
        case 1:
            if ( currClanTagFeature == BG_UnlockablesGetEquippedClanTagFeatureIndex(controllerIndex) )
                *handle = Material_RegisterHandle("menu_mp_killstreak_select", 7);
            goto LABEL_18;
        case 2:
            if ( isLocked || !cost )
                goto LABEL_18;
            v10 = UI_SafeTranslateString("MENU_POINTS");
            result = UI_ReplaceConversionInt(v10, cost);
            break;
        case 3:
            if ( isLocked )
                *handle = Material_RegisterHandle("menu_mp_lobby_locked", 3);
            goto LABEL_18;
        default:
LABEL_18:
            result = "";
            break;
    }
    return result;
}

char *__cdecl UI_FeederItemText_ClanTagFeaturesEdit(int controllerIndex)
{
    char *Name; // eax
    int v3; // [esp+0h] [ebp-10h]
    unsigned int currEquippedClanTagFeatureIndex; // [esp+8h] [ebp-8h]
    char *clanName; // [esp+Ch] [ebp-4h]

    clanName = Clan_GetName(controllerIndex);
    currEquippedClanTagFeatureIndex = BG_UnlockablesGetEquippedClanTagFeatureIndex(controllerIndex);
    if ( currEquippedClanTagFeatureIndex )
        v3 = currEquippedClanTagFeatureIndex;
    else
        v3 = 7;
    if ( !clanName || !*clanName )
        return UI_SafeTranslateString("MPUI_CREATE_CLAN_TAG_CAPS");
    Name = Clan_GetName(controllerIndex);
    return va("^%i%s", v3, Name);
}

const char *__cdecl UI_FeederItemText_CustomClasses(int controllerIndex, int contextIndex, itemDef_s *item, int index)
{
    if ( index >= Dvar_GetInt("ui_custom_class_highlighted") )
        ++index;
    return Dvar_GetString(customClassDvars[index]);
}

_CustomClassDescription *__cdecl UI_FeederItemText_CustomGametypeClasses(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                int index)
{
    if ( index >= Dvar_GetInt("ui_custom_gametype_class_highlighted") )
        ++index;
    return &g_customGameModeClassDescriptions[index];
}

char *__cdecl UI_FeederItemText_ChallengesPerks(
                int contextIndex,
                int controllerIndex,
                float feederID,
                int index,
                int column,
                itemDef_s *item,
                Material **handle)
{
    int LocalClientNum; // eax
    int v9; // [esp+4h] [ebp-18h]
    const char *itemName; // [esp+Ch] [ebp-10h]
    listBoxDef_s *listPtr; // [esp+10h] [ebp-Ch]
    int itemIndex; // [esp+14h] [ebp-8h]
    bool itemPurchased; // [esp+1Bh] [ebp-1h]

    listPtr = Item_GetListBoxDef(item);
    if ( listPtr->cursorPos[contextIndex] >= sharedUiInfo.numAttachments && sharedUiInfo.numAttachments )
    {
        listPtr->startPos[contextIndex] = 0;
        listPtr->cursorPos[contextIndex] = sharedUiInfo.numAttachments - 1;
        v9 = sharedUiInfo.numAttachments - 1;
        LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
        UI_FeederSelection(LocalClientNum, contextIndex, feederID, v9);
    }
    itemIndex = BG_UnlockablesGetItemIndexInSlot(index);
    itemName = BG_UnlockablesGetItemName(itemIndex);
    itemPurchased = BG_UnlockablesIsItemPurchased(controllerIndex, itemIndex);
    if ( itemIndex == -1 )
        return (char *)"";
    if ( !column )
        return UI_SafeTranslateString(itemName);
    if ( column != 1 )
        return (char *)"";
    if ( !itemPurchased )
        *handle = Material_RegisterHandle("menu_mp_lobby_locked", 3);
    return (char *)"";
}

char *__cdecl UI_FeederItemText_ItemInSlot(
                int contextIndex,
                int controllerIndex,
                float feederID,
                int index,
                int column,
                itemDef_s *item,
                Material **handle)
{
    int LocalClientNum; // eax
    int v9; // [esp+4h] [ebp-20h]
    bool v10; // [esp+8h] [ebp-1Ch]
    const char *itemName; // [esp+Ch] [ebp-18h]
    listBoxDef_s *listPtr; // [esp+10h] [ebp-14h]
    int cost; // [esp+14h] [ebp-10h]
    bool itemClassified; // [esp+1Bh] [ebp-9h]
    int itemIndex; // [esp+1Ch] [ebp-8h]
    bool itemLocked; // [esp+22h] [ebp-2h]
    bool isSelectedItem; // [esp+23h] [ebp-1h]

    listPtr = Item_GetListBoxDef(item);
    if ( listPtr->cursorPos[contextIndex] >= sharedUiInfo.numAttachments && sharedUiInfo.numAttachments )
    {
        listPtr->startPos[contextIndex] = 0;
        listPtr->cursorPos[contextIndex] = sharedUiInfo.numAttachments - 1;
        v9 = sharedUiInfo.numAttachments - 1;
        LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
        UI_FeederSelection(LocalClientNum, contextIndex, feederID, v9);
    }
    itemIndex = BG_UnlockablesGetItemIndexInSlot(index);
    isSelectedItem = 0;
    if ( listPtr->cursorPos[contextIndex] == index && !item->animInfo->animating )
    {
        sharedUiInfo.itemIndex = itemIndex;
        sharedUiInfo.itemIndex = index;
        isSelectedItem = Window_HasFocus(contextIndex, &item->window);
    }
    cost = BG_UnlockablesGetItemCost(itemIndex);
    v10 = BG_UnlockablesIsItemLocked(controllerIndex, itemIndex) && !Dvar_GetInt(ui_items_no_cost);
    itemLocked = v10;
    if ( BG_UnlockablesIsItemPurchased(controllerIndex, itemIndex) || v10 )
        cost = 0;
    itemName = BG_UnlockablesGetItemName(itemIndex);
    itemClassified = BG_UnlockablesIsItemClassified(itemIndex);
    if ( BG_UnlockablesGetItemGroupEnum(itemIndex) == 17 && v10 )
        itemClassified = 1;
    if ( itemClassified )
    {
        itemLocked = 1;
        cost = 0;
        itemName = "MENU_CLASSIFIED";
    }
    if ( column == 7 && BG_UnlockablesIsItemNew(controllerIndex, itemIndex) )
        *handle = Material_RegisterHandle("menu_mp_lobby_new", 3);
    return UI_Feeder_HandleItemList(
                     controllerIndex,
                     itemIndex,
                     column,
                     isSelectedItem,
                     itemLocked,
                     cost,
                     itemName,
                     handle);
}

char *__cdecl UI_FeederItemText_CustomPerksInSlot(
                int contextIndex,
                int controllerIndex,
                float feederID,
                int index,
                int column,
                itemDef_s *item,
                Material **handle)
{
    const char *itemName; // [esp+8h] [ebp-10h]
    int itemIndex; // [esp+10h] [ebp-8h]
    bool isSelectedItem; // [esp+16h] [ebp-2h]
    bool isTweakable; // [esp+17h] [ebp-1h]

    itemIndex = BG_UnlockablesGetItemIndexInSlot(index);
    itemName = BG_UnlockablesGetItemName(itemIndex);
    isSelectedItem = 0;
    if ( Item_GetListBoxDef(item)->cursorPos[contextIndex] == index )
    {
        sharedUiInfo.itemIndex = itemIndex;
        sharedUiInfo.itemIndex = index;
        isSelectedItem = Window_HasFocus(contextIndex, &item->window);
    }
    isTweakable = UI_Gametype_Custom_IsPerkTweakable(itemIndex);
    if ( isSelectedItem && !column )
    {
        if ( isTweakable )
            Dvar_SetStringByName("ui_hint_desc", (char *)"");
        else
            Dvar_SetStringByName("ui_hint_desc", (char*)"@CUSTOM_PERK_NO_EDIT_DESC");
    }
    if ( isSelectedItem && !isTweakable && column == 6 )
        isSelectedItem = 0;
    if ( !isTweakable || column != 2 )
        return UI_Feeder_HandleItemList(controllerIndex, itemIndex, column, isSelectedItem, 0, 0, itemName, handle);
    if ( UI_Gametype_HasPerkChangedFromDefault(itemIndex) )
    {
        Dvar_SetStringByName("ui_custom_perk_has_changed", (char *)"1");
        *handle = Material_RegisterHandle("ui_host", 3);
    }
    return (char *)"";
}

char *__cdecl UI_FeederItemText_ChallengesKillstreaks(
                int contextIndex,
                int controllerIndex,
                itemDef_s *item,
                float feederID,
                int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle)
{
    int LocalClientNum; // eax
    const char *ItemName; // eax
    char *v10; // eax
    int v12; // [esp+4h] [ebp-Ch]
    int itemIndex; // [esp+Ch] [ebp-4h]

    if ( listPtr->cursorPos[contextIndex] >= sharedUiInfo.numAttachments && sharedUiInfo.numAttachments )
    {
        listPtr->startPos[contextIndex] = 0;
        listPtr->cursorPos[contextIndex] = sharedUiInfo.numAttachments - 1;
        v12 = sharedUiInfo.numAttachments - 1;
        LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
        UI_FeederSelection(LocalClientNum, contextIndex, feederID, v12);
    }
    itemIndex = BG_UnlockablesGetItemIndexInSlot(index);
    if ( listPtr->cursorPos[contextIndex] == index )
        sharedUiInfo.itemIndex = itemIndex;
    if ( itemIndex == -1 )
        return (char *)"";
    if ( !column )
    {
        ItemName = BG_UnlockablesGetItemName(itemIndex);
        v10 = UI_SafeTranslateString(ItemName);
        return UI_ToUpper(v10);
    }
    if ( column != 1 || BG_UnlockablesIsItemPurchased(controllerIndex, itemIndex) )
        return (char *)"";
    *handle = Material_RegisterHandle("menu_mp_lobby_locked", 7);
    return (char *)"";
}

char *__cdecl UI_FeederItemText_SortedItems(
                int contextIndex,
                int controllerIndex,
                itemDef_s *item,
                float feederID,
                int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle)
{
    int LocalClientNum; // eax
    char *result; // eax
    const char *v10; // eax
    int LoadoutSlotFromString; // eax
    const char *v12; // eax
    const char *v13; // eax
    const char *v14; // eax
    unsigned int EquippedItemInSlot; // eax
    int ItemCount; // eax
    int DefaultItem; // eax
    int v18; // eax
    const char *ItemName; // eax
    char *v20; // eax
    char *v21; // eax
    int v22; // eax
    int v23; // [esp+4h] [ebp-1034h]
    int v24; // [esp+4h] [ebp-1034h]
    const char *String; // [esp+Ch] [ebp-102Ch]
    unsigned int v26[256]; // [esp+10h] [ebp-1028h]
    bool v27; // [esp+413h] [ebp-C25h]
    int Int; // [esp+414h] [ebp-C24h]
    int v29[513]; // [esp+418h] [ebp-C20h]
    int cost; // [esp+C1Ch] [ebp-41Ch]
    int v31; // [esp+C20h] [ebp-418h]
    int i; // [esp+C24h] [ebp-414h]
    int itemIndex; // [esp+C28h] [ebp-410h]
    char v34; // [esp+C2Fh] [ebp-409h]
    unsigned int v35[257]; // [esp+C30h] [ebp-408h]
    bool v36; // [esp+1037h] [ebp-1h]

    if ( listPtr->cursorPos[contextIndex] >= sharedUiInfo.numAttachments && sharedUiInfo.numAttachments )
    {
        listPtr->startPos[contextIndex] = 0;
        listPtr->cursorPos[contextIndex] = sharedUiInfo.numAttachments - 1;
        v23 = sharedUiInfo.numAttachments - 1;
        LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
        UI_FeederSelection(LocalClientNum, contextIndex, feederID, v23);
    }
    itemIndex = BG_UnlockablesGetItemIndexInSlot(index);
    cost = BG_UnlockablesGetItemCost(itemIndex);
    if ( BG_UnlockablesIsItemPurchased(controllerIndex, itemIndex) || Dvar_GetInt(ui_items_no_cost) )
        cost = 0;
    if ( listPtr->cursorPos[contextIndex] == index )
        sharedUiInfo.itemIndex = itemIndex;
    v31 = 0;
    i = 0;
    Int = 0;
    v34 = 0;
    v27 = 0;
    v36 = 0;
    if ( feederID == 67.0 )
    {
        Int = Dvar_GetInt("ui_slot_count");
        String = Dvar_GetString("selected_group");
        if ( Int > 256
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp",
                        1292,
                        0,
                        "%s",
                        "slotCount <= MAX_LOADOUT_SLOTS") )
        {
            __debugbreak();
        }
        if ( Int > 256 )
            return (char *)"";
        for ( i = 0; i < Int; ++i )
        {
            v10 = va("%s%d", String, i + 1);
            LoadoutSlotFromString = BG_UnlockablesGetLoadoutSlotFromString(v10);
            v29[i + 256] = LoadoutSlotFromString;
            if ( v29[i + 256] == -1 )
            {
                v12 = va("%s%d", String, i + 1);
                v13 = va("Loadout Slot %s not found", v12);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp",
                                1299,
                                0,
                                "%s\n\t%s",
                                "itemSlots[ currSlot ] != ENTRY_NOT_FOUND",
                                v13) )
                    __debugbreak();
            }
            if ( v29[i + 256] == -1 )
                return (char *)"";
            v14 = va("%s%d", String, i + 1);
            EquippedItemInSlot = BG_UnlockablesGetEquippedItemInSlot(controllerIndex, 0, v14);
            v29[i] = EquippedItemInSlot;
            if ( v29[i] != -1 )
            {
                ItemCount = BG_UnlockablesGetItemCount(v29[i]);
                v26[i] = ItemCount;
            }
            DefaultItem = BG_UnlockablesGetDefaultItem(v29[i + 256]);
            v35[i] = DefaultItem;
            if ( v29[i] != v35[i] )
            {
                if ( itemIndex == v29[i] )
                    v34 = 1;
                ++v31;
            }
            v18 = BG_UnlockablesGetItemCount(itemIndex);
            if ( v18 == v26[i] )
                v27 = 1;
        }
    }
    if ( v31 == Int )
        v27 = 1;
    if ( listPtr->cursorPos[contextIndex] == index && Window_HasFocus(contextIndex, &item->window) )
        v36 = 1;
    UI_FeederItemText_CommonListColorHandler(controllerIndex, v36, v27, cost);
    if ( itemIndex == -1 )
        return (char *)"";
    switch ( column )
    {
        case 0:
            ItemName = BG_UnlockablesGetItemName(itemIndex);
            v20 = UI_SafeTranslateString(ItemName);
            result = UI_ToUpper(v20);
            break;
        case 1:
            if ( v34 )
                *handle = Material_RegisterHandle("menu_mp_killstreak_select", 7);
            result = (char *)"";
            break;
        case 2:
            if ( cost )
            {
                if ( Dvar_GetInt(ui_items_no_cost) )
                {
                    result = (char *)"";
                }
                else
                {
                    v24 = cost;
                    v21 = UI_SafeTranslateString("MENU_POINTS");
                    result = UI_ReplaceConversionInt(v21, v24);
                }
            }
            else
            {
                result = (char *)"";
            }
            break;
        case 3:
            if ( !BG_UnlockablesIsItemLocked(controllerIndex, itemIndex) )
                return (char *)"";
            *handle = Material_RegisterHandle("menu_mp_lobby_locked", 7);
            result = (char *)"";
            break;
        case 4:
            v22 = BG_UnlockablesGetItemCount(itemIndex);
            result = va("%d", v22);
            break;
        default:
            return (char *)"";
    }
    return result;
}

const char *__cdecl UI_FeederItemText_CustomKillstreaks(
                int contextIndex,
                int controllerIndex,
                itemDef_s *item,
                float feederID,
                int index,
                int column,
                listBoxDef_s *listPtr)
{
    int LocalClientNum; // eax
    const char *ItemName; // eax
    char *v10; // eax
    int v11; // [esp+4h] [ebp-14h]
    bool isGrey; // [esp+Fh] [ebp-9h]
    int itemIndex; // [esp+10h] [ebp-8h]
    bool isSelectedItem; // [esp+17h] [ebp-1h]

    if ( listPtr->cursorPos[contextIndex] >= sharedUiInfo.numAttachments && sharedUiInfo.numAttachments )
    {
        listPtr->startPos[contextIndex] = 0;
        listPtr->cursorPos[contextIndex] = sharedUiInfo.numAttachments - 1;
        v11 = sharedUiInfo.numAttachments - 1;
        LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
        UI_FeederSelection(LocalClientNum, contextIndex, feederID, v11);
    }
    itemIndex = 0;
    if ( index )
        itemIndex = BG_UnlockablesGetItemIndexInSlot(index - 1);
    isGrey = !Window_HasFocus(contextIndex, &item->window);
    isSelectedItem = 0;
    if ( listPtr->cursorPos[contextIndex] == index )
    {
        sharedUiInfo.itemIndex = itemIndex;
        isSelectedItem = Window_HasFocus(contextIndex, &item->window);
    }
    UI_FeederItemText_CommonListColorHandler(controllerIndex, isSelectedItem, isGrey, 0);
    if ( itemIndex == -1 || column )
        return "";
    if ( !index )
        return UI_SafeTranslateString("CUSTOM_NO_KILLSTREAK");
    ItemName = BG_UnlockablesGetItemName(itemIndex);
    v10 = UI_SafeTranslateString(ItemName);
    return UI_ToUpper(v10);
}

const char *__cdecl UI_FeederItemText_AARScoreboard(
                int contextIndex,
                int controllerIndex,
                int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle,
                float feederID)
{
    const char *result; // eax
    char *MatchScoreboardInfo; // eax
    __int64 v9; // rax
    char *v10; // eax
    char *v11; // eax
    int WagerPlaceForMatchScoreboard; // eax
    const char *v13; // eax
    unsigned int DIntStat; // [esp-4h] [ebp-34h]
    int localClientNum; // [esp+Ch] [ebp-24h]
    unsigned __int64 nemesisXuid; // [esp+10h] [ebp-20h] BYREF
    unsigned __int64 currClientXuid; // [esp+18h] [ebp-18h]
    ddlState_t ddlState; // [esp+20h] [ebp-10h] BYREF

    localClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
    nemesisXuid = 0;
    currClientXuid = 0;
    switch ( column )
    {
        case 0:
            if ( feederID == 83.0 )
            {
                if ( !(index % 2) )
                    *handle = sharedUiInfo.assets.whiteMaterial;
                return "";
            }
            else
            {
                LiveStats_GetNemesisXuid(controllerIndex, &nemesisXuid);
                MatchScoreboardInfo = CG_GetMatchScoreboardInfo(localClientNum, 0, index, TEAM_NUM_TEAMS);
                currClientXuid = I_atoi64(MatchScoreboardInfo);
                if ( nemesisXuid && nemesisXuid == currClientXuid )
                {
                    *handle = Material_RegisterHandle("menu_mp_lobby_bar_nem", 7);
                    return "";
                }
                else
                {
                    LODWORD(v9) = Live_GetXuid(controllerIndex);
                    if ( currClientXuid == v9 )
                        *handle = Material_RegisterHandle("menu_mp_lobby_bar_party", 7);
                    else
                        *handle = sharedUiInfo.assets.whiteMaterial;
                    return "";
                }
            }
        case 1:
            if ( feederID == 83.0 )
                return "";
            if ( !listPtr || listPtr->cursorPos[contextIndex] != index )
                *handle = sharedUiInfo.assets.whiteMaterial;
            return "";
        case 2:
            if ( feederID != 83.0 )
                goto LABEL_33;
            WagerPlaceForMatchScoreboard = CG_GetWagerPlaceForMatchScoreboard(localClientNum, index);
            v13 = va("MPUI_ORDINAL_CAPS_%d", WagerPlaceForMatchScoreboard);
            return UI_SafeTranslateString(v13);
        case 3:
            if ( feederID != 83.0 )
                CG_GetMatchScoreboardRankIcon(localClientNum, index, TEAM_NUM_TEAMS, handle);
            return "";
        case 4:
            goto LABEL_33;
        case 5:
            if ( feederID != 83.0 )
                goto LABEL_33;
            ddlState = *LiveStats_GetRootDDLState();
            v10 = va("%d", index);
            DDL_MoveTo(&ddlState, &ddlState, 3, "AfterActionReportStats", "payouts", v10);
            DIntStat = LiveStats_GetDIntStat(controllerIndex, &ddlState);
            v11 = UI_SafeTranslateString("MENU_POINTS");
            result = UI_ReplaceConversionInt(v11, DIntStat);
            break;
        case 6:
        case 7:
        case 8:
        case 9:
            if ( Dvar_GetBool("ui_ingamePlayersOpen") && (column == 8 || column == 9) )
            {
                CG_GetInGamePlayerListIcon(localClientNum, column, index, TEAM_NUM_TEAMS, handle);
                result = "";
            }
            else if ( feederID == 83.0 )
            {
LABEL_34:
                result = "";
            }
            else
            {
LABEL_33:
                result = CG_GetMatchScoreboardInfo(localClientNum, column, index, TEAM_NUM_TEAMS);
            }
            break;
        default:
            goto LABEL_34;
    }
    return result;
}

const char *__cdecl UI_FeederItemText_InGamePlayerList(
                int contextIndex,
                int controllerIndex,
                int index,
                int column,
                Material **handle,
                team_t team)
{
    const char *result; // eax
    int localClientNum; // [esp+4h] [ebp-4h]

    localClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
    switch ( column )
    {
        case 0:
        case 4:
        case 6:
        case 7:
            result = CG_GetMatchInGamePlayerListInfo(localClientNum, column, index, team, handle);
            break;
        case 1:
            *handle = Material_RegisterHandle("menu_mp_lobby_bar_name", 7);
            result = "";
            break;
        case 2:
            if ( !(index % 2) )
                *handle = Material_RegisterHandle("white", 7);
            result = "";
            break;
        case 3:
            *handle = Material_RegisterHandle("white", 7);
            result = "";
            break;
        case 5:
            CG_GetMatchScoreboardRankIcon(localClientNum, index, team, handle);
            result = "";
            break;
        default:
            result = "";
            break;
    }
    return result;
}

char *__cdecl UI_FeederItemText_InGamePlayers(
                int contextIndex,
                int controllerIndex,
                int index,
                int column,
                Material **handle,
                int feederID,
                itemDef_s *item)
{
    char *result; // eax
    int localClientNum; // [esp+4h] [ebp-Ch]
    listBoxDef_s *listPtr; // [esp+8h] [ebp-8h]
    bool isRowHighlighted; // [esp+Fh] [ebp-1h]

    localClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp", 2023, 0, "%s", "listPtr") )
    {
        __debugbreak();
    }
    isRowHighlighted = listPtr->cursorPos[contextIndex] == index && Window_HasFocus(contextIndex, &item->window);
    switch ( column )
    {
        case 0:
            *handle = Material_RegisterHandle("menu_mp_lobby_bar_name", 7);
            result = (char *)"";
            break;
        case 1:
            if ( !(index % 2) || isRowHighlighted )
                *handle = Material_RegisterHandle("white", 7);
            result = (char *)"";
            break;
        case 2:
            *handle = Material_RegisterHandle("white", 7);
            result = (char *)"";
            break;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            result = CG_GetMatchInGamePlayersInfo(localClientNum, column, index, feederID, handle);
            break;
        default:
            result = (char *)"";
            break;
    }
    return result;
}

char *__cdecl UI_FeederItemText_Contracts(
                int contextIndex,
                int controllerIndex,
                int index,
                int column,
                itemDef_s *item,
                listBoxDef_s *listPtr,
                Material **handle)
{
    char *result = NULL; // eax
    char *v8; // eax
    int ContractCost; // [esp-4h] [ebp-Ch]
    int contractIndex; // [esp+4h] [ebp-4h]

    contractIndex = LiveContracts_GetContractIndexFromListIndex(controllerIndex, index);
    switch ( column )
    {
        case 0:
            *handle = sharedUiInfo.assets.whiteMaterial;
            result = (char *)"";
            break;
        case 1:
            result = LiveContracts_GetContractName(contractIndex);
            break;
        case 2:
            if ( LiveContracts_IsContractLocked(controllerIndex, contractIndex) )
            {
                result = (char *)"";
            }
            else
            {
                ContractCost = LiveContracts_GetContractCost(contractIndex);
                v8 = UI_SafeTranslateString("MENU_POINTS");
                result = UI_ReplaceConversionInt(v8, ContractCost);
            }
            break;
        case 3:
            if ( LiveContracts_IsContractLocked(controllerIndex, contractIndex) )
                *handle = Material_RegisterHandle("menu_mp_lobby_locked", 3);
            result = (char *)"";
            break;
        case 4:
            if (listPtr->cursorPos[contextIndex] == index && Window_HasFocus(contextIndex, &item->window))
            {
                //BLOPS_NULLSUB();
            }
            result = (char *)"";
            break;
        default:
            result = (char *)"";
            break;
    }
    return result;
}

char *__cdecl UI_FeederItemText_PersonalBests(int contextIndex, int controllerIndex, unsigned int index, int column)
{
    char *PersonalBestName; // eax
    const char *v5; // eax
    int PersonalBestValue; // eax
    int PersonalBestDelta; // eax

    if ( !LiveStats_GetPersonalBestRating(controllerIndex, index) )
        return (char *)"";
    if ( !column )
    {
        PersonalBestName = LiveStats_GetPersonalBestName(controllerIndex, index);
        v5 = va("STATS_PERSONALBEST_%s", PersonalBestName);
        return UI_SafeTranslateString(v5);
    }
    if ( column == 1 )
    {
        PersonalBestValue = LiveStats_GetPersonalBestValue(controllerIndex, index);
        return va("%d", PersonalBestValue);
    }
    if ( column != 2 )
        return (char *)"";
    PersonalBestDelta = LiveStats_GetPersonalBestDelta(controllerIndex, index);
    return va("%d", PersonalBestDelta);
}

char *__cdecl UI_FeederItemText_WeaponGroupMilestones(
                int contextIndex,
                int controllerIndex,
                unsigned int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle,
                bool *useOwnerDraw)
{
    const char *String; // eax
    char *v9; // eax
    char *v10; // eax
    const char *v11; // eax
    char *v12; // eax
    char milestoneTierRoman[8]; // [esp+8h] [ebp-43Ch] BYREF
    const char *statName; // [esp+10h] [ebp-434h]
    char outputString[1024]; // [esp+14h] [ebp-430h] BYREF
    challengeInfo_t *challenge; // [esp+418h] [ebp-2Ch] BYREF
    ConversionArguments convArgs; // [esp+41Ch] [ebp-28h] BYREF

    statName = 0;
    memset(&convArgs, 0, sizeof(convArgs));
    if ( !LiveStats_GetChallengeInfo(&challenge, index, 4) )
        return (char *)"";
    if ( column )
    {
        if ( column > 0 && column <= 2 )
            return UI_ChallengeCompletionStatus(column, challenge->currentValue, challenge->targetValue, useOwnerDraw);
        else
            return (char *)"";
    }
    else if ( challenge->type == MILESTONE_GROUP
                 && (statName = va("STATS_MILESTONE_STATNAME_%s", challenge->name),
                         CL_MilestoneTier_DecToRoman(challenge->milestone, milestoneTierRoman, 5),
                         statName) )
    {
        String = Dvar_GetString("weapon_group_name");
        v9 = UI_SafeTranslateString(String);
        convArgs.args[convArgs.argCount++] = v9;
        v10 = UI_SafeTranslateString(statName);
        convArgs.args[convArgs.argCount++] = v10;
        v11 = va("%s", milestoneTierRoman);
        convArgs.args[convArgs.argCount++] = v11;
        v12 = UI_SafeTranslateString("STATS_STATSGROUPMILESTONES");
        UI_ReplaceConversions(v12, &convArgs, outputString, 1024);
        return va("%s", outputString);
    }
    else
    {
        return (char *)"";
    }
}

char *__cdecl UI_FeederItemText_StatsMilestones(
                int contextIndex,
                int controllerIndex,
                unsigned int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle,
                bool *useOwnerDraw,
                statsMilestoneTypes_t milestoneType)
{
    char *result; // eax
    char outputString[260]; // [esp+4h] [ebp-110h] BYREF
    challengeInfo_t *challenge; // [esp+10Ch] [ebp-8h] BYREF
    bool isItemPurchased; // [esp+113h] [ebp-1h]

    memset(outputString, 0, 0x100u);
    if ( !LiveStats_GetChallengeInfo(&challenge, index, milestoneType) )
        return (char *)"";
    isItemPurchased = (challenge->flags & 1) == 0;
    switch ( column )
    {
        case 0:
            CL_GetFrontEndMilestoneLocalizedName(outputString, 256, index, milestoneType, 0);
            result = va("%s", outputString);
            break;
        case 1:
        case 2:
            if ( !isItemPurchased || (challenge->flags & 4) != 0 )
                result = (char *)"";
            else
                result = UI_ChallengeCompletionStatus(column, challenge->currentValue, challenge->targetValue, useOwnerDraw);
            break;
        case 3:
            if ( !isItemPurchased || (challenge->flags & 4) != 0 )
                *handle = Material_RegisterHandle("menu_mp_lobby_locked", 3);
            result = (char *)"";
            break;
        default:
            result = (char *)"";
            break;
    }
    return result;
}

char *__cdecl UI_FeederItemText_GlobalChallenges(
                int contextIndex,
                int controllerIndex,
                unsigned int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle,
                bool *useOwnerDraw)
{
    const char *String; // eax
    char outputString[1024]; // [esp+4h] [ebp-408h] BYREF
    challengeInfo_t *challenge; // [esp+408h] [ebp-4h] BYREF

    memset(outputString, 0, sizeof(outputString));
    if ( !LiveStats_GetChallengeInfo(&challenge, index, 0) )
        return (char *)"";
    if ( column )
    {
        if ( column > 0 && column <= 2 )
            return UI_ChallengeCompletionStatus(column, challenge->currentValue, challenge->targetValue, useOwnerDraw);
        else
            return (char *)"";
    }
    else
    {
        String = Dvar_GetString("globalChallengeType");
        CL_GetFrontEndMilestoneLocalizedName(outputString, 1024, index, MILESTONE_GLOBAL, String);
        return va("%s", outputString);
    }
}

char *__cdecl UI_FeederItemText_ChallengesAttachments(
                int contextIndex,
                int controllerIndex,
                unsigned int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle,
                bool *useOwnerDraw)
{
    char *result; // eax
    const char *itemName; // [esp+8h] [ebp-10h]
    char *statName; // [esp+Ch] [ebp-Ch]
    challengeInfo_t *challenge; // [esp+10h] [ebp-8h] BYREF
    bool attachmentPurchased; // [esp+17h] [ebp-1h]

    statName = 0;
    challenge = 0;
    if ( !LiveStats_GetChallengeInfo(&challenge, index, 5) )
        return (char *)"";
    if ( !challenge
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp", 2274, 0, "%s", "challenge") )
    {
        __debugbreak();
    }
    if ( !challenge )
        return (char *)"";
    attachmentPurchased = (challenge->flags & 1) == 0;
    switch ( column )
    {
        case 0:
            if ( challenge->type == MILESTONE_ATTACHMENTS )
            {
                if ( !I_stricmp(challenge->name, "kills") )
                    statName = va("STATS_MILESTONE_STATNAME_MARKSMAN");
                if ( !I_stricmp(challenge->name, "headshots") )
                    statName = va("STATS_MILESTONE_STATNAME_EXPERT");
            }
            if ( challenge->index >= 0x18u )
                itemName = BG_UnlockablesGetItemName(challenge->index);
            else
                itemName = BG_GetAttachmentDisplayName((eAttachment)challenge->index);
            result = UI_PrepareStatsMilestoneName(challenge->index, challenge->milestone, statName, itemName);
            break;
        case 1:
        case 2:
            if ( !attachmentPurchased )
                goto LABEL_24;
            result = UI_ChallengeCompletionStatus(column, challenge->currentValue, challenge->targetValue, useOwnerDraw);
            break;
        case 3:
            if ( !attachmentPurchased )
                *handle = Material_RegisterHandle("menu_mp_lobby_locked", 3);
            result = (char *)"";
            break;
        default:
LABEL_24:
            result = (char *)"";
            break;
    }
    return result;
}

const int CustomKillstreakNums[22] =
{
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  20,
  25,
  30,
  35,
  40,
  45,
  50
};


const char *__cdecl UI_FeederItemText_KillstreakNumKills(
                int contextIndex,
                int controllerIndex,
                int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle)
{
    int Int; // eax
    const char *v7; // eax
    const char *result = NULL; // eax
    char *v9; // eax
    char *v10; // eax
    int killNum; // [esp+4h] [ebp-8h]
    bool hasKillNumBeenSelected; // [esp+Ah] [ebp-2h]
    bool isCurrentSelectedKillstreakNum; // [esp+Bh] [ebp-1h]

    hasKillNumBeenSelected = 0;
    isCurrentSelectedKillstreakNum = 0;
    killNum = CustomKillstreakNums[index];
    if ( killNum == Dvar_GetInt("custom_killstreak_1_kills") && Dvar_GetInt("custom_killstreak_1")
        || killNum == Dvar_GetInt("custom_killstreak_2_kills") && Dvar_GetInt("custom_killstreak_2")
        || killNum == Dvar_GetInt("custom_killstreak_3_kills") && Dvar_GetInt("custom_killstreak_3") )
    {
        hasKillNumBeenSelected = 1;
    }
    Int = Dvar_GetInt("custom_killstreak_edit_slot");
    v7 = va("custom_killstreak_%d_kills", Int);
    if ( killNum == Dvar_GetInt(v7) )
        isCurrentSelectedKillstreakNum = 1;
    switch ( column )
    {
        case 0:
            *handle = sharedUiInfo.assets.whiteMaterial;
            result = "";
            break;
        case 1:
            if ( CustomKillstreakNums[index] == 1 )
            {
                v9 = UI_SafeTranslateString("MP_KILL");
                v10 = UI_ToUpper(v9);
            }
            else
            {
                v10 = UI_SafeTranslateString("MPUI_KILLS_CAPS");
            }
            result = va("%d %s", CustomKillstreakNums[index], v10);
            break;
        case 2:
            if ( isCurrentSelectedKillstreakNum )
                *handle = Material_RegisterHandle("menu_mp_killstreak_select", 7);
            result = "";
            break;
        case 6:
            if (listPtr->cursorPos[contextIndex] == index && !hasKillNumBeenSelected)
            {
                //BLOPS_NULLSUB();
            }
            result = "";
            break;
        default:
            result = "";
            break;
    }
    return result;
}

void __cdecl UI_Project_FeederItemOwnerDraw(
                int localClientNum,
                int contextIndex,
                float feederID,
                const rectDef_s *rect,
                unsigned int index)
{
    int milestoneType; // [esp+4h] [ebp-8h]
    int controllerIndex; // [esp+8h] [ebp-4h]

    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( controllerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp",
                    3084,
                    0,
                    "%s",
                    "controllerIndex != INVALID_CONTROLLER_PORT") )
    {
        __debugbreak();
    }
    milestoneType = UI_GetMilestoneType(feederID);
    if ( milestoneType != -1 )
        UI_DrawStatsMilestonesFeederProgressBar(controllerIndex, contextIndex, rect, index, milestoneType);
}

char __cdecl UI_Project_FeederItemColor(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                float feederID,
                int index,
                int column,
                float *color,
                bool onFocus)
{
    char result; // al
    char v9; // [esp+37h] [ebp-3Dh]
    int itemIndex; // [esp+38h] [ebp-3Ch]
    int milestoneType; // [esp+70h] [ebp-4h]

    switch ( (int)feederID )
    {
        case 3:
            switch ( column )
            {
                case 1:
                    Vec4Set(color, 0.15000001, 0.61000001, 0.0, 0.80000001);
                    break;
                case 6:
                    color[3] = 0.6f;
                    break;
                case 7:
                    Vec4Set(color, 0.81, 0.15000001, 0.039999999, 0.80000001);
                    break;
            }
            result = 1;
            break;
        case 49:
            itemIndex = BG_UnlockablesGetItemIndexInSlot(index);
            if ( column != 1 || UI_Gametype_Custom_IsPerkTweakable(itemIndex) )
            {
                v9 = UI_FeederItemColor_ItemList(controllerIndex, contextIndex, item, index, column, 0, 1, 0, 0, color);
            }
            else
            {
                Vec4Copy(newUIGrey, color);
                v9 = 1;
            }
            result = v9;
            break;
        case 57:
            if ( column && column != 2 )
                Vec4Copy(colorWhite, color);
            else
                Vec4Copy((const float *)&sharedUiInfo.itemNum, color);
            result = 1;
            break;
        case 58:
        case 59:
            result = UI_FeederItemColor_ItemsInSlot(controllerIndex, contextIndex, item, index, column, color);
            break;
        case 60:
            result = UI_FeederItemColor_WeaponOptions(controllerIndex, contextIndex, item, index, column, color);
            break;
        case 61:
        case 65:
        case 66:
        case 67:
            if ( !column || column == 2 || column == 4 )
                Vec4Copy((const float *)&sharedUiInfo.itemNum, color);
            result = 1;
            break;
        case 68:
            result = UI_FeederItemColor_Contracts(controllerIndex, contextIndex, item, index, column, color);
            break;
        case 78:
            result = UI_FeederItemColor_ItemList(controllerIndex, contextIndex, item, index, column, 0, 1, 0, 0, color);
            break;
        case 82:
        case 83:
            result = UI_FeederItemColor_AARScoreboard(controllerIndex, contextIndex, feederID, item, index, column, color);
            break;
        case 87:
            result = UI_FeederItemColor_InGamePlayerList(
                                 controllerIndex,
                                 contextIndex,
                                 item,
                                 index,
                                 column,
                                 color,
                                 onFocus,
                                 TEAM_ALLIES);
            break;
        case 88:
            result = UI_FeederItemColor_InGamePlayerList(
                                 controllerIndex,
                                 contextIndex,
                                 item,
                                 index,
                                 column,
                                 color,
                                 onFocus,
                                 TEAM_AXIS);
            break;
        case 89:
            result = UI_FeederItemColor_InGamePlayerList(
                                 controllerIndex,
                                 contextIndex,
                                 item,
                                 index,
                                 column,
                                 color,
                                 onFocus,
                                 TEAM_FREE);
            break;
        case 90:
            if ( column && column != 2 )
                Vec4Copy(colorWhite, color);
            else
                Vec4Copy((const float *)&sharedUiInfo.itemNum, color);
            result = 1;
            break;
        case 92:
        case 96:
        case 99:
            milestoneType = UI_GetMilestoneType(feederID);
            if ( milestoneType == -1 )
                goto $LN7_73;
            result = UI_FeederItemColor_StatsMilestones(
                                 controllerIndex,
                                 contextIndex,
                                 item,
                                 index,
                                 column,
                                 color,
                                 onFocus,
                                 milestoneType);
            break;
        case 94:
        case 95:
        case 98:
            result = UI_FeederItemColor_GlobalMilestones(controllerIndex, contextIndex, item, index, column, color);
            break;
        case 97:
        case 100:
$LN7_73:
            result = UI_FeederItemColor_PerksMilestones(controllerIndex, contextIndex, item, index, column, color);
            break;
        case 103:
            result = UI_FeederItemColor_KillstreakNumKills(controllerIndex, contextIndex, item, index, column, color);
            break;
        case 105:
        case 106:
        case 107:
        case 108:
            result = UI_FeederItemColor_InGamePlayers(
                                 controllerIndex,
                                 contextIndex,
                                 item,
                                 index,
                                 column,
                                 color,
                                 onFocus,
                                 (int)feederID);
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

char __cdecl UI_FeederItemColor_Contracts(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                int index,
                int column,
                float *color)
{
    int TimesPurchasable; // esi
    int codPoints; // [esp+8h] [ebp-14h]
    int contractIndex; // [esp+Ch] [ebp-10h]
    listBoxDef_s *listPtr; // [esp+10h] [ebp-Ch]
    bool isRowHighlighted; // [esp+17h] [ebp-5h]

    listPtr = Item_GetListBoxDef(item);
    if (!listPtr
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp", 2517, 0, "%s", "listPtr"))
    {
        __debugbreak();
    }
    contractIndex = LiveContracts_GetContractIndexFromListIndex(controllerIndex, index);
    LiveContracts_GetActiveContractIndex(controllerIndex, contractIndex);
    isRowHighlighted = listPtr->cursorPos[contextIndex] == index && Window_HasFocus(contextIndex, &item->window);
    *color =   1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    color[3] = 1.0f;
    if (column == -1)
        goto LABEL_9;
    if (!column && !isRowHighlighted)
    {
        *color = newUIFeederBackgroundColor[0];
        color[1] = newUIFeederBackgroundColor[1];
        color[2] = newUIFeederBackgroundColor[2];
        color[3] = newUIFeederBackgroundColor[3];
        return 1;
    }
    if (column == 1 || column == 2)
    {
        if (LiveContracts_IsContractLocked(controllerIndex, contractIndex)
            || LiveContracts_GetTimeUntilCooledDown(controllerIndex, contractIndex) > 0
            || LiveContracts_GetTimesPurchasable(contractIndex) > 0
            && (TimesPurchasable = LiveContracts_GetTimesPurchasable(contractIndex),
                (TimesPurchasable - LiveContracts_GetTimesPurchased(controllerIndex, contractIndex)) <= 0))
        {
        LABEL_9:
            *color = newUIGrey[0];
            color[1] = newUIGrey[1];
            color[2] = newUIGrey[2];
            color[3] = newUIGrey[3];
            return 1;
        }
        codPoints = LiveStats_GetCurrency(controllerIndex);
        if (LiveContracts_GetContractCost(contractIndex) <= codPoints)
        {
            *color = newUIGreen[0];
            color[1] = newUIGreen[1];
            color[2] = newUIGreen[2];
            color[3] = newUIGreen[3];
        }
        else
        {
            *color = newUIRed[0];
            color[1] = newUIRed[1];
            color[2] = newUIRed[2];
            color[3] = newUIRed[3];
        }
    }
    return 1;
}

char __cdecl UI_FeederItemColor_ItemList(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                int index,
                int column,
                int cost,
                bool itemPurchased,
                bool itemLocked,
                bool itemClassified,
                float *color)
{
    listBoxDef_s *listPtr; // [esp+4h] [ebp-8h]
    bool isRowHighlighted; // [esp+Bh] [ebp-1h]

    listPtr = Item_GetListBoxDef(item);
    if (!listPtr
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp", 2583, 0, "%s", "listPtr"))
    {
        __debugbreak();
    }
    isRowHighlighted = listPtr->cursorPos[contextIndex] == index && Window_HasFocus(contextIndex, &item->window);
    *color = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    color[3] = 1.0f;
    if (column)
    {
        if (column == 1 || column == 3)
        {
            if (itemLocked || itemClassified)
            {
                *color = newUIGrey[0];
                color[1] = newUIGrey[1];
                color[2] = newUIGrey[2];
                color[3] = newUIGrey[3];
            }
            else if (itemPurchased)
            {
                if (isRowHighlighted)
                {
                    *color = 0.0f;
                    color[1] = 0.0f;
                    color[2] = 0.0f;
                    color[3] = 1.0f;
                }
            }
            else if (cost <= LiveStats_GetCurrency(controllerIndex))
            {
                *color = newUIGreen[0];
                color[1] = newUIGreen[1];
                color[2] = newUIGreen[2];
                color[3] = newUIGreen[3];
            }
            else
            {
                *color = newUIRed[0];
                color[1] = newUIRed[1];
                color[2] = newUIRed[2];
                color[3] = newUIRed[3];
            }
        }
    }
    else if (!isRowHighlighted)
    {
        *color = newUIFeederBackgroundColor[0];
        color[1] = newUIFeederBackgroundColor[1];
        color[2] = newUIFeederBackgroundColor[2];
        color[3] = newUIFeederBackgroundColor[3];
    }
    return 1;
}

char __cdecl UI_FeederItemColor_ItemsInSlot(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                int index,
                int column,
                float *color)
{
    bool v7; // [esp+0h] [ebp-20h]
    int cost; // [esp+10h] [ebp-10h]
    bool itemClassified; // [esp+17h] [ebp-9h]
    int itemIndex; // [esp+18h] [ebp-8h]
    bool itemPurchased; // [esp+1Eh] [ebp-2h]

    itemIndex = BG_UnlockablesGetItemIndexInSlot(index);
    cost = BG_UnlockablesGetItemCost(itemIndex);
    itemPurchased = BG_UnlockablesIsItemPurchased(controllerIndex, itemIndex);
    v7 = BG_UnlockablesIsItemLocked(controllerIndex, itemIndex) && !Dvar_GetInt(ui_items_no_cost);
    itemClassified = BG_UnlockablesIsItemClassified(itemIndex);
    return UI_FeederItemColor_ItemList(
                     controllerIndex,
                     contextIndex,
                     item,
                     index,
                     column,
                     cost,
                     itemPurchased,
                     v7,
                     itemClassified,
                     color);
}

char __cdecl UI_FeederItemColor_WeaponOptions(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                int index,
                int column,
                float *color)
{
    int WeaponOptionNumFromIndexAndGroup; // eax
    int weaponOption; // [esp+Ch] [ebp-1Ch]
    eWeaponOptionGroup currentWeaponGroup; // [esp+10h] [ebp-18h]
    int cost; // [esp+18h] [ebp-10h]
    int itemIndex; // [esp+20h] [ebp-8h]
    char itemPurchased; // [esp+26h] [ebp-2h]
    bool itemLocked; // [esp+27h] [ebp-1h]

    currentWeaponGroup = WEAPONOPTION_GROUP_FIRST;
    if ( ui_currentWeaponOptionGroup )
        currentWeaponGroup = (eWeaponOptionGroup)ui_currentWeaponOptionGroup->current.integer;
    itemIndex = sharedUiInfo.itemIndex;
    weaponOption = BG_GetWeaponOptionNumFromIndexAndGroup(index, currentWeaponGroup);
    cost = BG_GetWeaponOptionCost(weaponOption);
    WeaponOptionNumFromIndexAndGroup = BG_GetWeaponOptionNumFromIndexAndGroup(
                                                                             index,
                                                                             (eWeaponOptionGroup)ui_currentWeaponOptionGroup->current.integer);
    itemPurchased = BG_UnlockablesIsItemOptionPurchased(controllerIndex, itemIndex, WeaponOptionNumFromIndexAndGroup);
    if ( !cost )
        itemPurchased = 1;
    if ( itemPurchased )
        cost = 0;
    itemLocked = BG_UnlockablesIsWeaponOptionLocked(controllerIndex, weaponOption);
    return UI_FeederItemColor_ItemList(
                     controllerIndex,
                     contextIndex,
                     item,
                     index,
                     column,
                     cost,
                     itemPurchased,
                     itemLocked,
                     0,
                     color);
}

bool __cdecl UI_FeederItemColor_InGamePlayerList(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                int index,
                int column,
                float *color,
                bool onFocus,
                team_t team)
{
    bool result; // al
    int LocalClientNum; // eax
    const char *MatchInGamePlayerXuid; // eax
    //int v11; // edx
    //__int64 v12; // [esp+0h] [ebp-14h]
    unsigned __int64 currClientXuid; // [esp+Ch] [ebp-8h]

    switch (column)
    {
    case 0:
    case 4:
    case 5:
        *color = white[0];
        color[1] = white[1];
        color[2] = white[2];
        color[3] = white[3];
        result = 1;
        break;
    case 1:
        if (team == TEAM_ALLIES)
        {
            *color = alliesColor[0];
            color[1] = alliesColor[1];
            color[2] = alliesColor[2];
            color[3] = alliesColor[3];
        }
        else if (team == TEAM_AXIS)
        {
            *color = axisColor[0];
            color[1] = axisColor[1];
            color[2] = axisColor[2];
            color[3] = axisColor[3];
        }
        result = 1;
        break;
    case 2:
    case 3:
        *color = fadedBlack[0];
        color[1] = fadedBlack[1];
        color[2] = fadedBlack[2];
        color[3] = fadedBlack[3];
        result = 1;
        break;
    case 6:
    case 7:
        LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
        MatchInGamePlayerXuid = CG_GetMatchInGamePlayerXuid(LocalClientNum, index, team);
        currClientXuid = I_atoi64(MatchInGamePlayerXuid);
        //LODWORD(v12) = Live_GetXuid(controllerIndex);
        //HIDWORD(v12) = v11;
        //if (v12 == currClientXuid)
        if (Live_GetXuid(controllerIndex) == currClientXuid)
        {
            *color = playerColor[0];
            color[1] = playerColor[1];
            color[2] = playerColor[2];
            color[3] = playerColor[3];
        }
        else
        {
            *color = white[0];
            color[1] = white[1];
            color[2] = white[2];
            color[3] = white[3];
        }
        result = 1;
        break;
    default:
        result = 1;
        break;
    }
    return result;
}

bool __cdecl UI_FeederItemColor_InGamePlayers(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                unsigned int index,
                int column,
                float *color,
                bool onFocus,
                int feederType)
{
    bool result; // al
    __int64 v9; // rax
    unsigned __int64 currClientXUID; // [esp+Ch] [ebp-18h] BYREF
    int localClientNum; // [esp+18h] [ebp-Ch]
    listBoxDef_s *listPtr; // [esp+1Ch] [ebp-8h]
    bool isRowHighlighted; // [esp+23h] [ebp-1h]

    localClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
    CG_GetMatchInGamePlayersXUID(localClientNum, index, feederType, &currClientXUID);
    listPtr = Item_GetListBoxDef(item);
    if (!listPtr
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp", 2771, 0, "%s", "listPtr"))
    {
        __debugbreak();
    }
    isRowHighlighted = listPtr->cursorPos[contextIndex] == index && Window_HasFocus(contextIndex, &item->window);
    switch (column)
    {
    case 0:
        if (isRowHighlighted)
        {
            *color = 1.0f;
            color[1] = 1.0f;
            color[2] = 1.0f;
            color[3] = 1.0f;
        }
        else if (feederType == 105)
        {
            *color = alliesColor[0];
            color[1] = alliesColor[1];
            color[2] = alliesColor[2];
            color[3] = alliesColor[3];
        }
        else if (feederType == 106)
        {
            *color = axisColor[0];
            color[1] = axisColor[1];
            color[2] = axisColor[2];
            color[3] = axisColor[3];
        }
        result = 1;
        break;
    case 1:
    case 2:
        if (isRowHighlighted)
        {
            *color = 1.0f;
            color[1] = 1.0f;
            color[2] = 1.0f;
            color[3] = 1.0f;
        }
        else
        {
            *color = fadedBlack[0];
            color[1] = fadedBlack[1];
            color[2] = fadedBlack[2];
            color[3] = fadedBlack[3];
        }
        result = 1;
        break;
    case 3:
    case 4:
    case 6:
    case 7:
        *color = white[0];
        color[1] = white[1];
        color[2] = white[2];
        color[3] = white[3];
        result = 1;
        break;
    case 5:
        LODWORD(v9) = Live_GetXuid(controllerIndex);
        if (v9 == currClientXUID)
        {
            *color = playerColor[0];
            color[1] = playerColor[1];
            color[2] = playerColor[2];
            color[3] = playerColor[3];
        }
        else
        {
            *color = white[0];
            color[1] = white[1];
            color[2] = white[2];
            color[3] = white[3];
        }
        result = 1;
        break;
    default:
        result = 1;
        break;
    }
    return result;
}

char __cdecl UI_FeederItemColor_GlobalMilestones(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                int index,
                int column,
                float *color)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-8h]
    bool isRowHighlighted; // [esp+7h] [ebp-1h]

    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp", 2858, 0, "%s", "listPtr") )
    {
        __debugbreak();
    }
    isRowHighlighted = listPtr->cursorPos[contextIndex] == index && Window_HasFocus(contextIndex, &item->window);
    *color = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    color[3] = 1.0f;
    if ( (!column || column == 2) && isRowHighlighted )
    {
        *color = 0.0f;
        color[1] = 0.0f;
        color[2] = 0.0f;
        color[3] = 1.0f;
    }
    return 1;
}

char __cdecl UI_FeederItemColor_PerksMilestones(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                int index,
                int column,
                float *color)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-10h]
    bool isRowHighlighted; // [esp+7h] [ebp-9h]
    int itemIndex; // [esp+8h] [ebp-8h]
    bool isItemPurchased; // [esp+Fh] [ebp-1h]

    listPtr = Item_GetListBoxDef(item);
    if (!listPtr
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp", 2884, 0, "%s", "listPtr"))
    {
        __debugbreak();
    }
    isRowHighlighted = listPtr->cursorPos[contextIndex] == index && Window_HasFocus(contextIndex, &item->window);
    itemIndex = BG_UnlockablesGetItemIndexInSlot(index);
    isItemPurchased = BG_UnlockablesIsItemPurchased(controllerIndex, itemIndex);
    *color = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    color[3] = 1.0f;
    if (!column)
    {
        if (isItemPurchased)
        {
            if (isRowHighlighted)
            {
                *color = 0.0f;
                color[1] = 0.0f;
                color[2] = 0.0f;
                color[3] = 1.0f;
            }
        }
        else
        {
            *color = newUIGrey[0];
            color[1] = newUIGrey[1];
            color[2] = newUIGrey[2];
            color[3] = newUIGrey[3];
        }
    }
    return 1;
}

char __cdecl UI_FeederItemColor_StatsMilestones(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                unsigned int index,
                int column,
                float *color,
                bool onFocus,
                int milestoneType)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-Ch]
    challengeInfo_t *challenge; // [esp+4h] [ebp-8h] BYREF
    bool isRowHighlighted; // [esp+Ah] [ebp-2h]
    bool isItemPurchased; // [esp+Bh] [ebp-1h]

    listPtr = Item_GetListBoxDef(item);
    if (!listPtr
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp", 2917, 0, "%s", "listPtr"))
    {
        __debugbreak();
    }
    isRowHighlighted = listPtr->cursorPos[contextIndex] == index && Window_HasFocus(contextIndex, &item->window);
    if (!LiveStats_GetChallengeInfo(&challenge, index, milestoneType))
        return 0;
    isItemPurchased = (challenge->flags & 1) == 0;
    *color = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    color[3] = 1.0f;
    if (!column || column == 2)
    {
        if (isItemPurchased && (challenge->flags & 4) == 0)
        {
            if (isRowHighlighted)
            {
                *color = 0.0f;
                color[1] = 0.0f;
                color[2] = 0.0f;
                color[3] = 1.0f;
            }
        }
        else
        {
            *color = newUIGrey[0];
            color[1] = newUIGrey[1];
            color[2] = newUIGrey[2];
            color[3] = newUIGrey[3];
        }
    }
    return 1;
}

bool __cdecl UI_FeederItemColor_AARScoreboard(
                int controllerIndex,
                int contextIndex,
                float feederID,
                itemDef_s *item,
                int index,
                int column,
                float *color)
{
    bool result; // al
    char *MatchScoreboardInfo; // eax
    __int64 v10; // rax
    char *v11; // eax
    char *v12; // eax
    //int v13; // edx
    //__int64 v14; // [esp+0h] [ebp-54h]
    float *focusColor; // [esp+14h] [ebp-40h]
    signed int payout; // [esp+24h] [ebp-30h]
    int localClientNum; // [esp+28h] [ebp-2Ch]
    unsigned __int64 nemesisXuid; // [esp+2Ch] [ebp-28h] BYREF
    unsigned __int64 currClientXuid; // [esp+34h] [ebp-20h]
    listBoxDef_s *listBox; // [esp+40h] [ebp-14h]
    ddlState_t ddlState; // [esp+44h] [ebp-10h] BYREF

    nemesisXuid = 0;
    currClientXuid = 0;
    localClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
    listBox = Item_GetListBoxDef(item);
    switch (column)
    {
    case 0:
        if (feederID == 83.0)
        {
            *color = item->window.foreColor[0];
            color[1] = item->window.foreColor[1];
            color[2] = item->window.foreColor[2];
            color[3] = item->window.foreColor[3];
            result = 1;
        }
        else
        {
            LiveStats_GetNemesisXuid(controllerIndex, &nemesisXuid);
            MatchScoreboardInfo = CG_GetMatchScoreboardInfo(localClientNum, 0, index, TEAM_NUM_TEAMS);
            currClientXuid = I_atoi64(MatchScoreboardInfo);
            if (nemesisXuid && nemesisXuid == currClientXuid
                || (LODWORD(v10) = Live_GetXuid(controllerIndex), currClientXuid == v10))
            {
                *color = item->window.foreColor[0];
                color[1] = item->window.foreColor[1];
                color[2] = item->window.foreColor[2];
                color[3] = item->window.foreColor[3];
                result = 1;
            }
            else
            {
                if (CG_GetMatchscoreboardTeam(localClientNum, index) == TEAM_ALLIES)
                {
                    *color = alliesColor[0];
                    color[1] = alliesColor[1];
                    color[2] = alliesColor[2];
                    color[3] = alliesColor[3];
                }
                else
                {
                    *color = axisColor[0];
                    color[1] = axisColor[1];
                    color[2] = axisColor[2];
                    color[3] = axisColor[3];
                }
                if (!(index % 2))
                {
                    *color = *color * darkRowMultiplier;
                    color[1] = color[1] * darkRowMultiplier;
                    color[2] = color[2] * darkRowMultiplier;
                }
                if (color[3] > 1.0)
                    color[3] = 1.0f;
                result = 1;
            }
        }
        break;
    case 1:
        if (feederID == 83.0)
        {
            result = 0;
        }
        else
        {
            *color = 0.0f;
            color[1] = 0.0f;
            color[2] = 0.0f;
            color[3] = 0.1f;
            result = 1;
        }
        break;
    case 2:
    case 4:
    case 5:
        if (feederID == 83.0)
        {
            ddlState = *LiveStats_GetRootDDLState();
            v11 = va("%d", index);
            DDL_MoveTo(&ddlState, &ddlState, 3, "AfterActionReportStats", "payouts", v11);
            payout = LiveStats_GetDIntStat(controllerIndex, &ddlState);
            v12 = CG_GetMatchScoreboardInfo(localClientNum, 0, index, TEAM_FREE);
            currClientXuid = I_atoi64(v12);
            //LODWORD(v14) = Live_GetXuid(controllerIndex);
            //HIDWORD(v14) = v13;
            //if (currClientXuid == v14)
            if (currClientXuid == Live_GetXuid(controllerIndex))
            {
                focusColor = listBox->focusColor;
                *color = listBox->focusColor[0];
                color[1] = focusColor[1];
                color[2] = focusColor[2];
                color[3] = focusColor[3];
                result = 1;
            }
            else
            {
                if (payout > 0)
                    goto LABEL_25;
                *color = newUIGrey[0];
                color[1] = newUIGrey[1];
                color[2] = newUIGrey[2];
                color[3] = newUIGrey[3];
                result = 1;
            }
        }
        else
        {
            result = 0;
        }
        break;
    default:
    LABEL_25:
        result = 0;
        break;
    }
    return result;
}

char __cdecl UI_FeederItemColor_KillstreakNumKills(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                int index,
                int column,
                float *color)
{
    listBoxDef_s *listPtr; // [esp+Ch] [ebp-8h]
    int killNum; // [esp+10h] [ebp-4h]

    if (column != 1)
        return UI_FeederItemColor_ItemList(controllerIndex, contextIndex, item, index, column, 0, 1, 0, 0, color);
    *color = colorWhite[0];
    color[1] = 1.0;
    color[2] = 1.0;
    color[3] = 1.0;
    killNum = CustomKillstreakNums[index];
    listPtr = Item_GetListBoxDef(item);
    if (!listPtr
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp", 3065, 0, "%s", "listPtr"))
    {
        __debugbreak();
    }
    if (listPtr->cursorPos[contextIndex] == index && Window_HasFocus(contextIndex, &item->window))
    {
        *color = 0.0f;
        color[1] = 0.0f;
        color[2] = 0.0f;
        color[3] = 1.0f;
    }
    if (killNum == Dvar_GetInt("custom_killstreak_1_kills") && Dvar_GetInt("custom_killstreak_1")
        || killNum == Dvar_GetInt("custom_killstreak_2_kills") && Dvar_GetInt("custom_killstreak_2")
        || killNum == Dvar_GetInt("custom_killstreak_3_kills") && Dvar_GetInt("custom_killstreak_3"))
    {
        *color = newUIGrey[0];
        color[1] = newUIGrey[1];
        color[2] = newUIGrey[2];
        color[3] = newUIGrey[3];
    }
    return 1;
}

int __cdecl UI_FeederDoubleClick(int localClientNum, int contextIndex, float feederID, itemGroup_t index)
{
    unsigned int PlaylistIdForNum; // eax
    char *ItemGroupName; // eax
    char *v7; // eax
    itemGroup_t groupIndex; // [esp+0h] [ebp-14h]
    const char *loadoutName; // [esp+8h] [ebp-Ch]
    int controllerIndex; // [esp+Ch] [ebp-8h]
    int currentCategory; // [esp+10h] [ebp-4h]

    if ( xblive_wagermatch->current.enabled )
        currentCategory = wagerCategory->current.integer;
    else
        currentCategory = category->current.integer;
    if ( feederID != 26.0 )
        goto LABEL_10;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( controllerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp",
                    3176,
                    0,
                    "%s",
                    "controllerIndex != INVALID_CONTROLLER_PORT") )
    {
        __debugbreak();
    }
    PlaylistIdForNum = Playlist_GetPlaylistIdForNum(controllerIndex, index, currentCategory);
    if ( Playlist_DoWeHaveRequiredDLC(controllerIndex, PlaylistIdForNum) )
    {
LABEL_10:
        if ( feederID == 78.0 )
        {
            loadoutName = Dvar_GetString("selected_loadout_slot");
            if ( !I_stricmp(loadoutName, "primary") )
            {
                if ( (unsigned int)index > ITEMGROUP_SNIPER
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp",
                                3198,
                                0,
                                "%s",
                                "arrayOffset >= WEAPONGROUP_PRIMARY_FIRST && arrayOffset <= WEAPONGROUP_PRIMARY_LAST") )
                {
                    __debugbreak();
                }
                ItemGroupName = (char *)BG_UnlockablesGetItemGroupName(index);
                Dvar_SetStringByName("selected_group", ItemGroupName);
            }
            if ( !I_stricmp(loadoutName, "secondary") )
            {
                groupIndex = (itemGroup_t)(index + 5);
                if ( (index + 5 < 5 || groupIndex > ITEMGROUP_SPECIAL)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp",
                                3205,
                                0,
                                "%s",
                                "arrayOffset >= WEAPONGROUP_SECONDARY_FIRST && arrayOffset <= WEAPONGROUP_SECONDARY_LAST") )
                {
                    __debugbreak();
                }
                v7 = (char *)BG_UnlockablesGetItemGroupName(groupIndex);
                Dvar_SetStringByName("selected_group", v7);
            }
        }
        return 0;
    }
    else
    {
        Live_ShowMarketplaceUI();
        return 1;
    }
}

void __cdecl UI_FeederDoubleClickDlcMaps(int localClientNum, int contextIndex, int index)
{
    if ( *(unsigned int *)&sharedUiInfo.mapList[index].mapPackType[12] )
        Dvar_GetBool((const char *)&sharedUiInfo.mapList[index].splitscreen);
}

bool __cdecl UI_FeederItemEnabled(int localClientNum, int contextIndex, float feederID, int index)
{
    unsigned int PlaylistIdForNum; // eax
    unsigned int CategoryIdForNum; // eax
    unsigned int Xp; // [esp-10h] [ebp-20h]
    unsigned int v8; // [esp-10h] [ebp-20h]
    int Prestige; // [esp-Ch] [ebp-1Ch]
    int v10; // [esp-Ch] [ebp-1Ch]
    int controllerIndex; // [esp+0h] [ebp-10h]
    int currentCategory; // [esp+4h] [ebp-Ch]

    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( controllerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp",
                    3257,
                    0,
                    "%s",
                    "controllerIndex != INVALID_CONTROLLER_PORT") )
    {
        __debugbreak();
    }
    if ( xblive_wagermatch->current.enabled )
        currentCategory = wagerCategory->current.integer;
    else
        currentCategory = category->current.integer;
    if ( feederID == 26.0 || feederID == 77.0 )
    {
        Prestige = LiveStats_GetPrestige(controllerIndex);
        Xp = LiveStats_GetXp(controllerIndex);
        PlaylistIdForNum = Playlist_GetPlaylistIdForNum(controllerIndex, index, currentCategory);
        return Playlist_IsLocked(controllerIndex, PlaylistIdForNum, Xp, Prestige, 0, 1) == 0;
    }
    else if ( feederID == 28.0 || feederID == 76.0 )
    {
        v10 = LiveStats_GetPrestige(controllerIndex);
        v8 = LiveStats_GetXp(controllerIndex);
        CategoryIdForNum = Playlist_GetCategoryIdForNum(index);
        return Playlist_CategoryIsLocked(controllerIndex, CategoryIdForNum, v8, v10) == 0;
    }
    else
    {
        return 1;
    }
}

Material *__cdecl UI_FeederItemImage(float feederID, int index)
{
    int actual; // [esp+0h] [ebp-4h] BYREF

    if ( feederID != 4.0 )
        return 0;
    UI_SelectedMap(index, &actual);
    return UI_GetLevelShot(actual);
}

char *__cdecl UI_GetMapLoadNameForCurrentIndex(int index)
{
    int i; // [esp+0h] [ebp-8h]
    int count; // [esp+4h] [ebp-4h]

    count = 0;
    for ( i = 0; i < sharedUiInfo.mapCount; ++i )
    {
        if ( index == count )
            return sharedUiInfo.mapList[i].mapLoadName;
        ++count;
    }
    Com_Printf(
        16,
        "Warning: Invalid Map load name Index found. Setting it to the first map of the current selected map pack type\n");
    return &sharedUiInfo.mapList[0].mapName[28];
}

void __cdecl UI_OverrideCursorPos_Maps(int contextIndex, listBoxDef_s *listPtr)
{
    const char *String; // eax
    char *v3; // eax
    const char *MapLoadNameForCurrentIndex; // [esp-4h] [ebp-8h]
    int mapCount; // [esp+0h] [ebp-4h]

    mapCount = sharedUiInfo.mapCount;
    if ( listPtr->cursorPos[contextIndex] >= sharedUiInfo.mapCount )
        listPtr->cursorPos[contextIndex] = 0;
    if ( mapCount )
    {
        if ( Dvar_GetBool("ui_mapPackChanged") )
        {
            listPtr->cursorPos[contextIndex] = 0;
            Dvar_SetBoolByName("ui_mapPackChanged", 0);
        }
        else
        {
            MapLoadNameForCurrentIndex = UI_GetMapLoadNameForCurrentIndex(listPtr->cursorPos[contextIndex]);
            String = Dvar_GetString("ui_preview");
            if ( I_strcmp(String, MapLoadNameForCurrentIndex) )
            {
                v3 = UI_GetMapLoadNameForCurrentIndex(listPtr->cursorPos[contextIndex]);
                Dvar_SetStringByName("ui_preview", v3);
            }
        }
    }
    else
    {
        listPtr->cursorPos[contextIndex] = 0;
        Dvar_SetStringByName("ui_preview", (char *)"");
    }
}

void __cdecl UI_OverrideCursorPos_CustomGametypes(int contextIndex, listBoxDef_s *listPtr)
{
    int index; // [esp+0h] [ebp-Ch]
    int typeCount; // [esp+4h] [ebp-8h]
    int typeCursorPos; // [esp+8h] [ebp-4h]

    typeCount = sharedUiInfo.gameTypes[31].basictraining + sharedUiInfo.numCustomMatchGameTypes;
    typeCursorPos = listPtr->cursorPos[contextIndex];
    if ( typeCursorPos >= sharedUiInfo.gameTypes[31].basictraining + sharedUiInfo.numCustomMatchGameTypes )
    {
        if ( xblive_basictraining->current.enabled )
            listPtr->cursorPos[contextIndex] = typeCursorPos;
        else
            listPtr->cursorPos[contextIndex] = 0;
    }
    if ( typeCount )
    {
        if ( UI_CustomGametypes_IsInvalidCursorPos(typeCursorPos) )
        {
            index = UI_CustomGametypes_GetSelectedGameTypeIndex();
            if ( xblive_basictraining->current.enabled )
                listPtr->cursorPos[contextIndex] = UI_GetBasicTrainingGametypeNumFromId(index);
            else
                listPtr->cursorPos[contextIndex] = index;
        }
    }
    else
    {
        listPtr->cursorPos[contextIndex] = 0;
        Dvar_SetString((dvar_s *)ui_gametype, "");
        Dvar_SetString((dvar_s *)ui_customModeName, "");
        Dvar_SetInt((dvar_s *)ui_customModeIndex, -1);
    }
}

int __cdecl UI_GetBasicTrainingGametypeNumFromId(int id)
{
    int index; // [esp+0h] [ebp-8h]
    int count; // [esp+4h] [ebp-4h]

    count = 0;
    for ( index = 0; index < sharedUiInfo.numCustomMatchGameTypes; ++index )
    {
        if ( sharedUiInfo.gameTypeMapCount[29 * index - 901] )
        {
            if ( index == id )
                return count;
            ++count;
        }
    }
    return 0;
}

int __cdecl UI_CustomGametypes_GetSelectedGameTypeIndex()
{
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < sharedUiInfo.numCustomMatchGameTypes; ++i )
    {
        if ( !I_stricmp(ui_gametype->current.string, sharedUiInfo.customMatchGameTypes[i].gameType) )
            return i;
    }
    return 0;
}

bool __cdecl UI_CustomGametypes_IsInvalidCursorPos(int cursorPos)
{
    return cursorPos < 0
            || cursorPos >= sharedUiInfo.numCustomMatchGameTypes
            || I_strcmp(ui_gametype->current.string, &sharedUiInfo.customGameTypes[32].gameType[116 * cursorPos]) != 0;
}

void __cdecl UI_OverrideCursorPos_Gametypes(
                int localClientNum,
                int contextIndex,
                listBoxDef_s *listPtr,
                float feederID)
{
    int itemCount; // [esp+8h] [ebp-4h]

    itemCount = UI_FeederCount(localClientNum, contextIndex, feederID, listPtr);
    if ( itemCount < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp", 3588, 0, "%s", "itemCount >= 0") )
    {
        __debugbreak();
    }
    if ( listPtr->cursorPos[contextIndex] >= itemCount || listPtr->cursorPos[contextIndex] < 0 )
    {
        listPtr->cursorPos[contextIndex] = 0;
        listPtr->startPos[contextIndex] = 0;
    }
}

bool __cdecl UI_Project_OverrideCursorPos(int localClientNum, int contextIndex, itemDef_s *item)
{
    bool result; // al
    listBoxDef_s *listPtr; // [esp+20h] [ebp-4h]

    listPtr = Item_GetListBoxDef(item);
    switch ( (int)listPtr->special )
    {
        case '#':
        case '$':
        case '%':
        case '\'':
        case '(':
        case ')':
        case '*':
        case '+':
        case ',':
            UI_OverrideCursorPos_GameVariants(localClientNum, contextIndex, listPtr, (int)listPtr->special);
            result = 1;
            break;
        case '0':
        case 'h':
            UI_OverrideCursorPos_Gametypes(localClientNum, contextIndex, listPtr, listPtr->special);
            result = 1;
            break;
        case '1':
        case ':':
        case ';':
        case '=':
        case 'A':
        case 'B':
        case 'C':
        case 'N':
        case 'a':
        case 'd':
            UI_OverrideCursorPos_ItemsInSlot(localClientNum, contextIndex, listPtr, (int)listPtr->special);
            result = 1;
            break;
        case '3':
            result = 1;
            break;
        case '7':
            UI_OverrideCursorPos_FriendsStatsCompare(localClientNum, contextIndex, listPtr);
            result = 1;
            break;
        case '?':
            result = 1;
            break;
        case 'D':
            UI_OverrideCursorPos_Contracts(localClientNum, contextIndex, item, listPtr);
            result = 1;
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

void __cdecl UI_OverrideCursorPos_FriendsStatsCompare(int localClientNum, int contextIndex, listBoxDef_s *listPtr)
{
    int controllerIndex; // [esp+0h] [ebp-8h]
    int count; // [esp+4h] [ebp-4h]

    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( controllerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp",
                    3349,
                    0,
                    "%s",
                    "controllerIndex != INVALID_CONTROLLER_PORT") )
    {
        __debugbreak();
    }
    count = Friends_GetCount(controllerIndex, 0);
    if ( count )
    {
        if ( listPtr->cursorPos[contextIndex] >= count )
            listPtr->cursorPos[contextIndex] = count - 1;
    }
    else
    {
        listPtr->cursorPos[contextIndex] = 0;
    }
}

void __cdecl UI_OverrideCursorPos_GameVariants(
                int localClientNum,
                int contextIndex,
                listBoxDef_s *listPtr,
                int feederID)
{
    int itemCount; // [esp+8h] [ebp-4h]

    itemCount = UI_FeederCount(localClientNum, contextIndex, (float)feederID, listPtr);
    if ( itemCount < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp", 3609, 0, "%s", "itemCount >= 0") )
    {
        __debugbreak();
    }
    if ( listPtr->cursorPos[contextIndex] >= itemCount || listPtr->cursorPos[contextIndex] < 0 )
    {
        listPtr->cursorPos[contextIndex] = 0;
        listPtr->startPos[contextIndex] = 0;
    }
}

void __cdecl UI_OverrideCursorPos_ItemsInSlot(
                int localClientNum,
                int contextIndex,
                listBoxDef_s *listPtr,
                int feederID)
{
    int itemCount; // [esp+8h] [ebp-4h]

    itemCount = UI_FeederCount(localClientNum, contextIndex, (float)feederID, listPtr);
    if ( itemCount < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp", 3621, 0, "%s", "itemCount >= 0") )
    {
        __debugbreak();
    }
    if ( listPtr->cursorPos[contextIndex] >= itemCount || listPtr->cursorPos[contextIndex] < 0 )
    {
        listPtr->cursorPos[contextIndex] = 0;
        listPtr->startPos[contextIndex] = 0;
    }
}

void __cdecl UI_OverrideCursorPos_Contracts(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                listBoxDef_s *listPtr)
{
    int ControllerIndex; // eax
    int v5; // [esp+4h] [ebp-Ch]
    uiInfo_s *uiInfo; // [esp+8h] [ebp-8h]
    int itemCount; // [esp+Ch] [ebp-4h]

    itemCount = UI_FeederCount(localClientNum, contextIndex, 68.0, listPtr);
    if ( itemCount < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp", 3633, 0, "%s", "itemCount >= 0") )
    {
        __debugbreak();
    }
    if ( listPtr->cursorPos[contextIndex] >= itemCount || listPtr->cursorPos[contextIndex] < 0 )
    {
        listPtr->cursorPos[contextIndex] = 0;
        listPtr->startPos[contextIndex] = 0;
    }
    if ( Window_HasFocus(contextIndex, &item->window) )
    {
        uiInfo = UI_UIContext_GetInfo(contextIndex);
        if ( !uiInfo
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_feeders_mp.cpp", 3645, 0, "%s", "uiInfo") )
        {
            __debugbreak();
        }
        v5 = listPtr->cursorPos[contextIndex];
        ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        uiInfo->contractIndex = LiveContracts_GetContractIndexFromListIndex(ControllerIndex, v5);
    }
}

void __cdecl UI_FeederSelection_Maps(int contextIndex, float feederID, int index)
{
    char *mapLoadName; // [esp+0h] [ebp-4h]

    mapLoadName = UI_GetMapLoadNameForCurrentIndex(index);
    if ( index >= 0 && index < sharedUiInfo.mapCount )
        Dvar_SetStringByName("ui_preview", mapLoadName);
}

void __cdecl UI_FeederSelection_CustomGametypes(int contextIndex, float feederID, int index)
{
    if ( index >= 0 && index < sharedUiInfo.numCustomMatchGameTypes )
        Dvar_SetStringByName("ui_gametype", &sharedUiInfo.customGameTypes[32].gameType[116 * index]);
}

void __cdecl UI_FeederSelection_GametypesBase(int localClientNum, int contextIndex, float feederID, int index)
{
    int v4; // [esp+8h] [ebp-8h]

    if ( index >= 0 && index < UI_FeederCount(localClientNum, contextIndex, feederID, 0) )
    {
        if ( xblive_basictraining->current.enabled )
            index = UI_GetBasicTrainingGametypeIdForNum(index);
        if ( Com_GameMode_IsGameMode(GAMEMODE_FIRST_PRIVATE_ONLINE_GAMEMODE) )
            v4 = index - 1;
        else
            v4 = index;
        if ( v4 >= 0 )
            Dvar_SetStringByName("ui_preview", &sharedUiInfo.customGameTypes[32].gameType[116 * v4]);
    }
}

void __cdecl UI_FeederSelection_GametypesInGame(int localClientNum, int contextIndex, float feederID, int index)
{
    if ( index >= 0 && index < UI_FeederCount(localClientNum, contextIndex, feederID, 0) )
    {
        if ( xblive_basictraining->current.enabled )
            index = UI_GetBasicTrainingGametypeIdForNum(index) + 1;
        Dvar_SetIntByName("ui_preview_gt_idx", 1);
        Dvar_SetStringByName("ui_preview_gt", &sharedUiInfo.customGameTypes[32].gameType[116 * index]);
    }
}

void __cdecl UI_Project_FeederSelection(int localClientNum, int contextIndex, float feederID, int index)
{
    int GameTypeLbIndex; // eax
    uiInfo_s *dc; // [esp+Ch] [ebp-8h]
    int ItemIndexInSlot; // [esp+10h] [ebp-4h]

    switch ( (int)feederID )
    {
        case '#':
        case '$':
        case '%':
        case '\'':
        case '(':
        case ')':
        case '*':
        case '+':
        case ',':
            UI_FeederSelection_GameVariants(localClientNum, contextIndex, feederID, index);
            break;
        case '0':
            goto $LN16_40;
        case '1':
        case ':':
        case ';':
        case 'A':
        case 'B':
        case 'C':
        case 'a':
        case 'd':
            sharedUiInfo.itemIndex = BG_UnlockablesGetItemIndexInSlot(index);
            sharedUiInfo.itemIndex = index;
            break;
        case '9':
            sharedUiInfo.attachmentNum = BG_UnlockablesGetItemAttachmentNumWithAttachPoint(
                                                                                         sharedUiInfo.itemIndex,
                                                                                         index,
                                                                                         attachmentFilter->current.integer);
            break;
        case '=':
            ItemIndexInSlot = 0;
            if ( index )
                ItemIndexInSlot = BG_UnlockablesGetItemIndexInSlot(index - 1);
            sharedUiInfo.itemIndex = ItemIndexInSlot;
            UI_Gametype_Custom_ResolveNumKillsConflicts();
            break;
        case 'G':
            dc = UI_UIContext_GetInfo(contextIndex);
            UI_AnimateItem(localClientNum, &dc->uiDC, "match_performance_histogram", "histogram_component", "change", 200);
            GameTypeLbIndex = LiveCombatRecord_GetGameTypeLbIndex(index);
            Dvar_SetInt((dvar_s *)ui_combatHistogramCurrGametype, GameTypeLbIndex);
$LN16_40:
            UI_FeederSelection_GametypesBase(localClientNum, contextIndex, feederID, index);
            break;
        case 'g':
            Dvar_SetInt((dvar_s *)custom_killstreak_numkills, CustomKillstreakNums[index]);
            break;
        case 'h':
            UI_FeederSelection_GametypesInGame(localClientNum, contextIndex, feederID, index);
            break;
        default:
            return;
    }
}

char __cdecl UI_Project_FeederDataExtended(
                int controllerIndex,
                int contextIndex,
                int feederID,
                listBoxDef_s *listBox,
                const char *field,
                char **stringResult)
{
    char result; // al
    bool v7; // [esp+38h] [ebp-8h]
    bool v8; // [esp+39h] [ebp-7h]
    bool v9; // [esp+3Ah] [ebp-6h]
    bool v10; // [esp+3Bh] [ebp-5h]
    int index; // [esp+3Ch] [ebp-4h]

    index = listBox->cursorPos[contextIndex];
    switch ( feederID )
    {
        case 3:
            if ( I_stricmp(field, "xuid") )
                v10 = I_stricmp(field, "name") == 0;
            else
                v10 = 1;
            result = v10;
            break;
        case 51:
            if ( I_stricmp(field, "xuid") )
                v9 = I_stricmp(field, "name") == 0;
            else
                v9 = 1;
            result = v9;
            break;
        case 55:
            result = UI_FeederDataExtended_FriendsStatsCompare(controllerIndex, index, field, stringResult);
            break;
        case 56:
            if ( I_stricmp(field, "xuid") )
                v7 = I_stricmp(field, "name") == 0;
            else
                v7 = 1;
            result = v7;
            break;
        case 63:
            if ( I_stricmp(field, "xuid") )
                v8 = I_stricmp(field, "name") == 0;
            else
                v8 = 1;
            result = v8;
            break;
        case 82:
        case 83:
            result = UI_FeederDataExtended_AARScoreboard(controllerIndex, index, feederID, field, stringResult);
            break;
        case 105:
        case 106:
        case 107:
        case 108:
            result = UI_FeederDataExtended_InGamePlayers(controllerIndex, index, feederID, field, stringResult);
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

char __cdecl UI_FeederDataExtended_FriendsStatsCompare(
                int controllerIndex,
                int index,
                const char *field,
                char **stringResult)
{
    *stringResult = 0;
    if ( I_stricmp(field, "xuid") || !Friends_GetByIndex(controllerIndex, 0, index, &info_5) )
    {
        if ( I_stricmp(field, "name") || !Friends_GetByIndex(controllerIndex, 0, index, &info_5) )
        {
            return 0;
        }
        else
        {
            *stringResult = info_5.name;
            return 1;
        }
    }
    else
    {
        *stringResult = va("%lld", info_5.dwid);
        return 1;
    }
}

char __cdecl UI_FeederDataExtended_InGamePlayers(
                int controllerIndex,
                unsigned int index,
                int feederType,
                const char *field,
                char **stringResult)
{
    int countIndex; // [esp+28h] [ebp-20h]
    int a_count; // [esp+2Ch] [ebp-1Ch]
    int numClients; // [esp+34h] [ebp-14h]
    cg_s *cgameGlob; // [esp+3Ch] [ebp-Ch]
    int localClientNum; // [esp+40h] [ebp-8h]
    int clientIndex; // [esp+44h] [ebp-4h]
    int indexa; // [esp+54h] [ebp+Ch]

    localClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob || index >= 0x20 )
        return 0;
    indexa = index + 1;
    switch ( feederType )
    {
        case 'i':
        case 'j':
            for ( clientIndex = 0; clientIndex < 32; ++clientIndex )
            {
                if ( cgameGlob->bgs.clientinfo[clientIndex].infoValid )
                {
                    if ( cgameGlob->bgs.clientinfo[clientIndex].team == (feederType == 105) + 1 && !--indexa )
                        break;
                }
            }
            break;
        case 'k':
        case 'l':
            numClients = 0;
            for ( countIndex = 0; countIndex < 32; ++countIndex )
            {
                if ( cgameGlob->bgs.clientinfo[countIndex].infoValid )
                    ++numClients;
            }
            a_count = (int)((float)((float)numClients * 0.5) + 0.4999999990686774);
            for ( clientIndex = 0; clientIndex < 32; ++clientIndex )
            {
                if ( cgameGlob->bgs.clientinfo[clientIndex].infoValid
                    && cgameGlob->bgs.clientinfo[clientIndex].team == TEAM_FREE
                    && (feederType == 107 && clientIndex < a_count || feederType == 108 && clientIndex >= a_count)
                    && !--indexa )
                {
                    break;
                }
            }
            break;
        default:
            break;
    }
    if ( clientIndex >= 32 )
        return 0;
    if ( I_stricmp(field, "xuid") )
    {
        if ( I_stricmp(field, "name") )
        {
            return 0;
        }
        else
        {
            *stringResult = cgameGlob->bgs.clientinfo[clientIndex].name;
            return 1;
        }
    }
    else
    {
        if ( !cgameGlob->bgs.clientinfo[clientIndex].xuid
            || (cgameGlob->bgs.clientinfo[clientIndex].xuid & 0xFFFF0000) == 0xFFFF0000
            && HIDWORD(cgameGlob->bgs.clientinfo[clientIndex].xuid) == -1 )
        {
            *stringResult = va("0");
        }
        else
        {
            *stringResult = va("%lld", cgameGlob->bgs.clientinfo[clientIndex].xuid);
        }
        return 1;
    }
}

char __cdecl UI_FeederDataExtended_AARScoreboard(
                int controllerIndex,
                int index,
                int feederType,
                const char *field,
                char **stringResult)
{
    int localClientNum; // [esp+0h] [ebp-4h]

    localClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
    if ( I_stricmp(field, "xuid") )
    {
        if ( I_stricmp(field, "name") )
        {
            return 0;
        }
        else
        {
            *stringResult = CG_GetMatchScoreboardInfo(localClientNum, 4, index, TEAM_NUM_TEAMS);
            return 1;
        }
    }
    else
    {
        *stringResult = CG_GetMatchScoreboardInfo(localClientNum, 0, index, TEAM_NUM_TEAMS);
        return 1;
    }
}

void __cdecl UI_FeederBackColor(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                float feederID,
                int index,
                float *color)
{
    *color = item->window.backColor[0];
    color[1] = item->window.backColor[1];
    color[2] = item->window.backColor[2];
    color[3] = item->window.backColor[3];
}

