#include "cl_milestone.h"
#include <universal/assertive.h>
#include <universal/q_shared.h>
#include <universal/com_stringtable.h>
#include <live/live_stats.h>
#include <ui/ui_main.h>
#include <bgame/bg_weapons_attachment.h>
#include <cstring>
#include <bgame/bg_unlockable_items.h>
#include <qcommon/common.h>
#include <stringed/stringed_hooks.h>
#include <cgame_mp/cg_draw_mp.h>

int __cdecl CL_GetMilestoneRowNum(
                XAssetHeader *stringTable,
                int comparisonColumn1,
                const char *value1,
                int comparisonColumn2,
                const char *value2,
                int comparisonColumn3,
                const char *value3)
{
    int tableTier; // [esp+0h] [ebp-2Ch]
    char tableName[32]; // [esp+4h] [ebp-28h] BYREF
    int milestoneTableRow; // [esp+28h] [ebp-4h]

    milestoneTableRow = -1;
    if ( !value1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    33,
                    0,
                    "%s",
                    "value1 != NULL") )
    {
        __debugbreak();
    }
    if ( !value1 )
        return milestoneTableRow;
    for ( tableTier = 1; tableTier <= 4; ++tableTier )
    {
        Com_sprintf(tableName, 0x20u, "mp/statsmilestones%i.csv", tableTier);
        StringTable_GetAsset(tableName, stringTable);
        if ( stringTable )
        {
            milestoneTableRow = StringTable_LookupRowNumForValue(
                                                        stringTable->stringTable,
                                                        comparisonColumn1,
                                                        value1,
                                                        comparisonColumn2,
                                                        value2,
                                                        comparisonColumn3,
                                                        value3);
            if ( milestoneTableRow != -1 )
                return milestoneTableRow;
        }
    }
    return milestoneTableRow;
}

const char *__cdecl CL_GetMilestoneData(const char *indexName, milestoneTableColumns_t column)
{
    const StringTable *milestoneTable; // [esp+0h] [ebp-Ch] BYREF
    int milestoneTableRow; // [esp+4h] [ebp-8h]
    const char *milestoneName; // [esp+8h] [ebp-4h]

    milestoneTable = 0;
    milestoneName = 0;
    if ( !indexName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    58,
                    0,
                    "%s",
                    "indexName != NULL") )
    {
        __debugbreak();
    }
    if ( !indexName )
        return 0;
    milestoneTableRow = CL_GetMilestoneRowNum((XAssetHeader *)&milestoneTable, 0, indexName, -1, 0, -1, 0);
    if ( milestoneTable )
    {
        if ( milestoneTableRow != -1 )
            return StringTable_GetColumnValueForRow(milestoneTable, milestoneTableRow, column);
    }
    return milestoneName;
}

void __cdecl CL_GetFrontEndMilestoneLocalizedName(
                char *outputString,
                int stringLength,
                unsigned int index,
                statsMilestoneTypes_t milestoneType,
                const char *challengeType)
{
    const char *String; // eax
    char statName[260]; // [esp+0h] [ebp-110h] BYREF
    challengeInfo_t *challenge; // [esp+108h] [ebp-8h] BYREF
    const char *localizedString; // [esp+10Ch] [ebp-4h]

    localizedString = 0;
    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    78,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( outputString && LiveStats_GetChallengeInfo(&challenge, index, milestoneType) )
    {
        if ( challengeType && *challengeType && !I_stricmp(challengeType, "lifetime_") )
            Com_sprintf(statName, 0x100u, "%s%s", "lifetime_", challenge->name);
        else
            Com_sprintf(statName, 0x100u, "%s", challenge->name);
        if ( challenge->type == MILESTONE_GROUP )
        {
            String = Dvar_GetString("weapon_group_name");
            localizedString = UI_SafeTranslateString(String);
        }
        CL_GetMilestoneLocalizedName(
            outputString,
            stringLength,
            statName,
            challenge->type,
            (eAttachment)challenge->index,
            challenge->milestone,
            (challenge->flags & 4) != 0,
            localizedString);
    }
}

char __cdecl CL_GetInGameMilestoneLocalizedName(
                int localClientNum,
                char *outputString,
                int stringLength,
                const char *statName,
                statsMilestoneTypes_t milestoneType,
                eAttachment statsTableIndex,
                int milestoneTier,
                const char *weaponType)
{
    const char *v9; // eax
    char *v10; // eax
    const char *weaponName; // [esp+0h] [ebp-110h]
    char *weaponGroupName; // [esp+4h] [ebp-10Ch]
    char localizedNameString[260]; // [esp+8h] [ebp-108h] BYREF

    memset(localizedNameString, 0, 256);
    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    106,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( !outputString )
        return 0;
    if ( !statName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    110,
                    0,
                    "%s",
                    "statName != NULL") )
    {
        __debugbreak();
    }
    if ( !statName )
        return 0;
    if ( milestoneType == MILESTONE_GROUP )
    {
        if ( !weaponType
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                        116,
                        0,
                        "%s",
                        "weaponType != NULL") )
        {
            __debugbreak();
        }
        if ( !weaponType )
            return 0;
        v9 = va("MPUI_%s", weaponType);
        weaponGroupName = UI_SafeTranslateString(v9);
        Com_sprintf(localizedNameString, 0x100u, "%s", weaponGroupName);
    }
    else if ( milestoneType == MILESTONE_WEAPON )
    {
        weaponName = BG_UnlockablesGetItemName(statsTableIndex);
        if ( !weaponName
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                        125,
                        0,
                        "%s",
                        "weaponName != NULL") )
        {
            __debugbreak();
        }
        if ( !weaponName )
            return 0;
        v10 = UI_SafeTranslateString(weaponName);
        Com_sprintf(localizedNameString, 0x100u, "%s", v10);
    }
    return CL_GetMilestoneLocalizedName(
                     outputString,
                     stringLength,
                     statName,
                     milestoneType,
                     statsTableIndex,
                     milestoneTier,
                     0,
                     localizedNameString);
}

char __cdecl CL_GetMilestoneLocalizedName(
                char *outputString,
                int stringLength,
                const char *statName,
                statsMilestoneTypes_t milestoneType,
                eAttachment statsTableIndex,
                int milestoneTier,
                bool isItemClassified,
                const char *localizedString)
{
    const char *v9; // eax
    char couldLocalize; // [esp+7h] [ebp-5h]
    const StringTable *stringTable; // [esp+8h] [ebp-4h] BYREF

    couldLocalize = 0;
    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    138,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( !outputString )
        return 0;
    if ( stringLength < 256
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    142,
                    0,
                    "%s",
                    "stringLength >= MAX_MILESTONE_STRING_SIZE") )
    {
        __debugbreak();
    }
    if ( (unsigned int)milestoneType >= MILESTONE_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    143,
                    0,
                    "%s",
                    "milestoneType >= MILESTONE_GLOBAL && milestoneType < MILESTONE_COUNT") )
    {
        __debugbreak();
    }
    memset(outputString, 0, stringLength);
    if ( !statName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    146,
                    0,
                    "%s",
                    "statName != NULL") )
    {
        __debugbreak();
    }
    if ( !statName )
        return 0;
    v9 = va("%d", milestoneTier);
    if ( CL_GetMilestoneRowNum((XAssetHeader *)&stringTable, 4, statName, 1, v9, -1, 0) == -1 )
        --milestoneTier;
    switch ( milestoneType )
    {
        case MILESTONE_GLOBAL:
            couldLocalize = PrepareGlobalChallengeName(statName, outputString, stringLength, milestoneTier);
            break;
        case MILESTONE_WEAPON:
            couldLocalize = PrepareWeaponChallengeName(
                                                statName,
                                                outputString,
                                                stringLength,
                                                statsTableIndex,
                                                milestoneTier,
                                                isItemClassified);
            break;
        case MILESTONE_MAP:
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp", 180, 0, "%s", "false") )
                __debugbreak();
            break;
        case MILESTONE_GAMEMODE:
            couldLocalize = PrepareGameModeChallengeName(
                                                statName,
                                                outputString,
                                                stringLength,
                                                statsTableIndex,
                                                milestoneTier,
                                                localizedString);
            break;
        case MILESTONE_GROUP:
            couldLocalize = PrepareGroupChallengeName(
                                                statName,
                                                outputString,
                                                stringLength,
                                                statsTableIndex,
                                                milestoneTier,
                                                localizedString);
            break;
        case MILESTONE_ATTACHMENTS:
            couldLocalize = PrepareAttachmentChallengeName(
                                                statName,
                                                outputString,
                                                stringLength,
                                                statsTableIndex,
                                                milestoneTier);
            break;
        case MILESTONE_KILLSTREAKS:
            couldLocalize = PrepareKillstreakChallengeName(
                                                statName,
                                                outputString,
                                                stringLength,
                                                statsTableIndex,
                                                milestoneTier);
            break;
        case MILESTONE_GRENADES:
            couldLocalize = PrepareGrenadeChallengeName(statName, outputString, stringLength, statsTableIndex, milestoneTier);
            break;
        default:
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp", 183, 0, "%s", "false") )
                __debugbreak();
            break;
    }
    return couldLocalize;
}

char __cdecl FormatStatsMilestoneName(
                char *outputString,
                int stringLength,
                int milestoneTier,
                const char *statsName,
                const char *itemName)
{
    char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    char milestoneTierRoman[8]; // [esp+0h] [ebp-34h] BYREF
    ConversionArguments convArgs; // [esp+Ch] [ebp-28h] BYREF

    memset(&convArgs, 0, sizeof(convArgs));
    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    197,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( !outputString )
        return 0;
    if ( !itemName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    201,
                    0,
                    "%s",
                    "itemName != NULL") )
    {
        __debugbreak();
    }
    if ( !itemName )
        return 0;
    if ( !statsName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    205,
                    0,
                    "%s",
                    "statsName != NULL") )
    {
        __debugbreak();
    }
    if ( !statsName )
        return 0;
    CL_MilestoneTier_DecToRoman(milestoneTier, milestoneTierRoman, 5);
    v6 = UI_SafeTranslateString(itemName);
    convArgs.args[convArgs.argCount++] = v6;
    v7 = UI_SafeTranslateString(statsName);
    convArgs.args[convArgs.argCount++] = v7;
    convArgs.args[convArgs.argCount++] = milestoneTierRoman;
    v8 = UI_SafeTranslateString("STATS_STATSMILESTONES");
    UI_ReplaceConversions(v8, &convArgs, outputString, stringLength);
    return 1;
}

char __cdecl PrepareGlobalChallengeName(const char *name, char *outputString, int stringLength, int milestoneTier)
{
    const char *v5; // eax
    char *v6; // eax
    char *tempChallengeName; // [esp+0h] [ebp-14h]
    char milestoneTierRoman[8]; // [esp+4h] [ebp-10h] BYREF
    const StringTable *stringTable; // [esp+10h] [ebp-4h] BYREF

    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    219,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( !outputString )
        return 0;
    if ( !name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp", 223, 0, "%s", "name != NULL") )
    {
        __debugbreak();
    }
    if ( !name )
        return 0;
    if ( StringBeginsWith("gm_", name)
        || StringBeginsWith("kills_", name)
        || StringBeginsWith("basic_", name)
        || StringBeginsWith("medal_", name)
        || StringBeginsWith("lifetime_", name)
        || StringBeginsWith("final_", name) )
    {
        v5 = va("CH_%s", name);
        tempChallengeName = UI_SafeTranslateString(v5);
        Com_sprintf(outputString, stringLength, "%s", tempChallengeName);
    }
    else
    {
        v6 = UI_SafeTranslateString(name);
        Com_sprintf(outputString, stringLength, "%s", v6);
    }
    if ( CL_GetMilestoneRowNum((XAssetHeader *)&stringTable, 4, name, 1, "1", -1, 0) != -1 )
    {
        CL_MilestoneTier_DecToRoman(milestoneTier, milestoneTierRoman, 5);
        Com_sprintf(outputString, stringLength, "%s %s", outputString, milestoneTierRoman);
    }
    return 1;
}

char __cdecl PrepareKillstreakChallengeName(
                const char *name,
                char *outputString,
                int stringLength,
                int index,
                int tierId)
{
    const char *itemName; // [esp+0h] [ebp-8h]
    const char *statsName; // [esp+4h] [ebp-4h]

    statsName = "";
    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    258,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( !outputString )
        return 0;
    if ( !name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp", 262, 0, "%s", "name != NULL") )
    {
        __debugbreak();
    }
    if ( !name )
        return 0;
    if ( !I_stricmp(name, "kills") )
        statsName = "STATS_MILESTONE_STATNAME_KILLS";
    if ( !I_stricmp(name, "used") )
        statsName = "STATS_MILESTONE_STATNAME_USED";
    if ( !I_stricmp(name, "destroyed") )
        statsName = "STATS_MILESTONE_STATNAME_DESTROYED";
    itemName = BG_UnlockablesGetItemName(index);
    if ( !itemName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    274,
                    0,
                    "%s",
                    "itemName != NULL") )
    {
        __debugbreak();
    }
    if ( !itemName )
        return 0;
    if ( !statsName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    278,
                    0,
                    "%s",
                    "statsName != NULL") )
    {
        __debugbreak();
    }
    if ( statsName )
        return FormatStatsMilestoneName(outputString, stringLength, tierId, statsName, itemName);
    else
        return 0;
}

char __cdecl PrepareGrenadeChallengeName(const char *name, char *outputString, int stringLength, int index, int tierId)
{
    const char *itemName; // [esp+0h] [ebp-8h]
    const char *statsName; // [esp+4h] [ebp-4h]

    statsName = "";
    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    290,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( !outputString )
        return 0;
    if ( !name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp", 294, 0, "%s", "name != NULL") )
    {
        __debugbreak();
    }
    if ( !name )
        return 0;
    if ( !I_stricmp(name, "kills") )
        statsName = "STATS_MILESTONE_STATNAME_KILLS";
    itemName = BG_UnlockablesGetItemName(index);
    if ( !itemName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    305,
                    0,
                    "%s",
                    "itemName != NULL") )
    {
        __debugbreak();
    }
    if ( !itemName )
        return 0;
    if ( !statsName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    309,
                    0,
                    "%s",
                    "statsName != NULL") )
    {
        __debugbreak();
    }
    if ( statsName )
        return FormatStatsMilestoneName(outputString, stringLength, tierId, statsName, itemName);
    else
        return 0;
}

char __cdecl PrepareAttachmentChallengeName(
                const char *name,
                char *outputString,
                int stringLength,
                eAttachment index,
                int tierId)
{
    const char *itemName; // [esp+0h] [ebp-8h]
    const char *statsName; // [esp+4h] [ebp-4h]

    statsName = "";
    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    321,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( !outputString )
        return 0;
    if ( !name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp", 325, 0, "%s", "name != NULL") )
    {
        __debugbreak();
    }
    if ( !name )
        return 0;
    if ( !I_stricmp(name, "kills") )
        statsName = "STATS_MILESTONE_STATNAME_MARKSMAN";
    if ( index >= ATTACHMENT_COUNT )
        itemName = BG_UnlockablesGetItemName(index);
    else
        itemName = BG_GetAttachmentDisplayName(index);
    if ( !itemName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    343,
                    0,
                    "%s",
                    "itemName != NULL") )
    {
        __debugbreak();
    }
    if ( !itemName )
        return 0;
    if ( !statsName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    347,
                    0,
                    "%s",
                    "statsName != NULL") )
    {
        __debugbreak();
    }
    if ( statsName )
        return FormatStatsMilestoneName(outputString, stringLength, tierId, statsName, itemName);
    else
        return 0;
}

char __cdecl PrepareWeaponChallengeName(
                const char *name,
                char *outputString,
                int stringLength,
                int index,
                int tierId,
                bool isItemClassified)
{
    char *v7; // eax
    const char *itemName; // [esp+0h] [ebp-34h]
    const char *statsName; // [esp+8h] [ebp-2Ch]

    statsName = "";
    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    364,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( !outputString )
        return 0;
    if ( !name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp", 368, 0, "%s", "name != NULL") )
    {
        __debugbreak();
    }
    if ( !name )
        return 0;
    if ( isItemClassified )
    {
        v7 = UI_SafeTranslateString("MENU_CLASSIFIED");
        Com_sprintf(outputString, stringLength, "%s", v7);
        return 1;
    }
    else
    {
        if ( !I_stricmp(name, "kills") )
            statsName = "STATS_MILESTONE_STATNAME_MARKSMAN";
        if ( !I_stricmp(name, "headshots") )
            statsName = "STATS_MILESTONE_STATNAME_EXPERT";
        itemName = BG_UnlockablesGetItemName(index);
        if ( !itemName
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                        386,
                        0,
                        "%s",
                        "itemName != NULL") )
        {
            __debugbreak();
        }
        if ( !itemName )
            return 0;
        if ( !I_stricmp(itemName, "WEAPON_KNIFE") )
            statsName = "STATS_MILESTONE_STATNAME_KNIFE";
        if ( !statsName
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                        393,
                        0,
                        "%s",
                        "statsName != NULL") )
        {
            __debugbreak();
        }
        if ( !statsName )
            return 0;
        return FormatStatsMilestoneName(outputString, stringLength, tierId, statsName, itemName);
    }
}

char __cdecl PrepareGroupChallengeName(
                const char *name,
                char *outputString,
                int stringLength,
                int index,
                int tierId,
                const char *weaponNameLocalizedString)
{
    char *v7; // eax
    char *v8; // eax
    char milestoneTierRoman[8]; // [esp+0h] [ebp-38h] BYREF
    const char *statsName; // [esp+Ch] [ebp-2Ch]
    ConversionArguments convArgs; // [esp+10h] [ebp-28h] BYREF

    memset(&convArgs, 0, sizeof(convArgs));
    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    411,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( !outputString )
        return 0;
    if ( !name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp", 415, 0, "%s", "name != NULL") )
    {
        __debugbreak();
    }
    if ( !name )
        return 0;
    statsName = va("STATS_MILESTONE_STATNAME_%s", name);
    if ( !statsName
        && !weaponNameLocalizedString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    422,
                    0,
                    "%s",
                    "statsName != NULL || weaponNameLocalizedString != NULL") )
    {
        __debugbreak();
    }
    if ( !statsName || !weaponNameLocalizedString )
        return 0;
    CL_MilestoneTier_DecToRoman(tierId, milestoneTierRoman, 5);
    convArgs.args[convArgs.argCount++] = weaponNameLocalizedString;
    v7 = UI_SafeTranslateString(statsName);
    convArgs.args[convArgs.argCount++] = v7;
    convArgs.args[convArgs.argCount++] = milestoneTierRoman;
    v8 = UI_SafeTranslateString("STATS_STATSGROUPMILESTONES");
    UI_ReplaceConversions(v8, &convArgs, outputString, stringLength);
    return 1;
}

char __cdecl PrepareGameModeChallengeName(
                const char *name,
                char *outputString,
                int stringLength,
                int index,
                int tierId,
                const char *gameModeLocalizedString)
{
    const char *v7; // eax
    char *v8; // eax
    char milestoneTierRoman[8]; // [esp+0h] [ebp-110h] BYREF
    char challengeName[256]; // [esp+8h] [ebp-108h] BYREF
    const StringTable *stringTable; // [esp+10Ch] [ebp-4h] BYREF

    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    440,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( !outputString )
        return 0;
    if ( !name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp", 444, 0, "%s", "name != NULL") )
    {
        __debugbreak();
    }
    if ( !name )
        return 0;
    v7 = va("CH_GM_%s", name);
    v8 = UI_SafeTranslateString(v7);
    Com_sprintf(challengeName, 0x100u, "%s", v8);
    if ( gameModeLocalizedString )
        Com_sprintf(outputString, stringLength, "%s %s", gameModeLocalizedString, challengeName);
    else
        Com_sprintf(outputString, stringLength, "%s", challengeName);
    if ( CL_GetMilestoneRowNum((XAssetHeader *)&stringTable, 4, name, 1, "1", -1, 0) != -1 )
    {
        CL_MilestoneTier_DecToRoman(tierId, milestoneTierRoman, 5);
        Com_sprintf(outputString, stringLength, "%s %s", outputString, milestoneTierRoman);
    }
    return 1;
}

void __cdecl CL_GetFrontEndMilestoneLocalizedDesc(
                char *outputString,
                int stringLength,
                unsigned int index,
                statsMilestoneTypes_t milestoneType,
                const char *challengeType)
{
    const char *v5; // eax
    const char *String; // eax
    const char *v7; // eax
    const char *v8; // eax
    char *itemName; // [esp+0h] [ebp-10Ch]
    char *itemNamea; // [esp+0h] [ebp-10Ch]
    char *itemNameb; // [esp+0h] [ebp-10Ch]
    char statName[256]; // [esp+4h] [ebp-108h] BYREF
    challengeInfo_t *challenge; // [esp+108h] [ebp-4h] BYREF

    itemName = 0;
    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    473,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( outputString && LiveStats_GetChallengeInfo(&challenge, index, milestoneType) )
    {
        if ( challengeType && *challengeType && !I_stricmp(challengeType, "lifetime_") )
            Com_sprintf(statName, 0x100u, "%s%s", "lifetime_", challenge->name);
        else
            Com_sprintf(statName, 0x100u, "%s", challenge->name);
        switch ( challenge->type )
        {
            case MILESTONE_GAMEMODE:
                v5 = va("MPUI_%s", ui_challengeGameMode->current.string);
                itemNamea = UI_SafeTranslateString(v5);
                CL_GetMilestoneLocalizedDesc(
                    outputString,
                    stringLength,
                    statName,
                    challenge->type,
                    challenge->index,
                    challenge->milestone,
                    challenge->targetValue,
                    itemNamea);
                break;
            case MILESTONE_GROUP:
                String = Dvar_GetString("weapon_group_name");
                v7 = va("%s_WITH_S", String);
                itemNameb = UI_SafeTranslateString(v7);
                CL_GetMilestoneLocalizedDesc(
                    outputString,
                    stringLength,
                    statName,
                    challenge->type,
                    challenge->index,
                    challenge->milestone,
                    challenge->targetValue,
                    itemNameb);
                break;
            case MILESTONE_WEAPON:
                v8 = BG_UnlockablesGetItemName(challenge->index);
                itemName = UI_SafeTranslateString(v8);
                if ( !itemName
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                                501,
                                0,
                                "%s",
                                "itemName != NULL") )
                {
                    __debugbreak();
                }
                if ( itemName )
LABEL_20:
                    CL_GetMilestoneLocalizedDesc(
                        outputString,
                        stringLength,
                        statName,
                        challenge->type,
                        challenge->index,
                        challenge->milestone,
                        challenge->targetValue,
                        itemName);
                break;
            default:
                goto LABEL_20;
        }
    }
}

char __cdecl CL_GetInGameMilestoneLocalizedDesc(
                int localClientNum,
                char *outputString,
                int stringLength,
                const char *statName,
                statsMilestoneTypes_t milestoneType,
                int statsTableIndex,
                int milestoneTier,
                const char *weaponType,
                int targetValue)
{
    const char *String; // eax
    const char *v11; // eax
    const char *v12; // eax
    char *v13; // eax
    const char *v14; // eax
    char *v15; // eax
    char *v16; // eax
    const char *weaponName; // [esp+0h] [ebp-20Ch]
    char gameType[256]; // [esp+4h] [ebp-208h] BYREF
    char localizedDescString[260]; // [esp+104h] [ebp-108h] BYREF

    memset(localizedDescString, 0, 256);
    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    515,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( !outputString )
        return 0;
    if ( !statName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    519,
                    0,
                    "%s",
                    "statName != NULL") )
    {
        __debugbreak();
    }
    if ( !statName )
        return 0;
    switch ( milestoneType )
    {
        case MILESTONE_GAMEMODE:
            memset(gameType, 0, sizeof(gameType));
            if ( IsHardcoreMode(localClientNum) )
            {
                String = Dvar_GetString("g_gametype");
                Com_sprintf(gameType, 0x100u, "HC%s", String);
            }
            else
            {
                v11 = Dvar_GetString("g_gametype");
                Com_sprintf(gameType, 0x100u, "%s", v11);
            }
            v12 = va("MPUI_%s", gameType);
            v13 = UI_SafeTranslateString(v12);
            Com_sprintf(localizedDescString, 0x100u, "%s", v13);
            break;
        case MILESTONE_GROUP:
            if ( !weaponType
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                            540,
                            0,
                            "%s",
                            "weaponType != NULL") )
            {
                __debugbreak();
            }
            if ( !weaponType )
                return 0;
            v14 = va("MPUI_%s_WITH_S", weaponType);
            v15 = UI_SafeTranslateString(v14);
            Com_sprintf(localizedDescString, 0x100u, "%s", v15);
            break;
        case MILESTONE_WEAPON:
            weaponName = BG_UnlockablesGetItemName(statsTableIndex);
            if ( !weaponName
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                            548,
                            0,
                            "%s",
                            "weaponName != NULL") )
            {
                __debugbreak();
            }
            if ( !weaponName )
                return 0;
            v16 = UI_SafeTranslateString(weaponName);
            Com_sprintf(localizedDescString, 0x100u, "%s", v16);
            break;
    }
    return CL_GetMilestoneLocalizedDesc(
                     outputString,
                     stringLength,
                     statName,
                     milestoneType,
                     statsTableIndex,
                     milestoneTier,
                     targetValue,
                     localizedDescString);
}

char __cdecl CL_GetMilestoneLocalizedDesc(
                char *outputString,
                int stringLength,
                const char *statName,
                statsMilestoneTypes_t milestoneType,
                int statsTableIndex,
                int milestoneTier,
                int targetValue,
                const char *localizedString)
{
    char couldLocalize; // [esp+7h] [ebp-1h]

    if ( stringLength < 256
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    560,
                    0,
                    "%s",
                    "stringLength >= MAX_MILESTONE_STRING_SIZE") )
    {
        __debugbreak();
    }
    if ( (unsigned int)milestoneType >= MILESTONE_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    561,
                    0,
                    "%s",
                    "milestoneType >= MILESTONE_GLOBAL && milestoneType < MILESTONE_COUNT") )
    {
        __debugbreak();
    }
    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    563,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( !outputString )
        return 0;
    if ( !statName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    567,
                    0,
                    "%s",
                    "statName != NULL") )
    {
        __debugbreak();
    }
    if ( !statName )
        return 0;
    memset(outputString, 0, stringLength);
    couldLocalize = 0;
    switch ( milestoneType )
    {
        case MILESTONE_GLOBAL:
            couldLocalize = GetGlobalChallengeDescription(statName, outputString, stringLength, targetValue);
            break;
        case MILESTONE_WEAPON:
        case MILESTONE_GROUP:
            couldLocalize = PrepareWeaponDescription(
                                                statName,
                                                outputString,
                                                stringLength,
                                                targetValue,
                                                milestoneType,
                                                localizedString);
            break;
        case MILESTONE_MAP:
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp", 596, 0, "%s", "false") )
                __debugbreak();
            break;
        case MILESTONE_GAMEMODE:
            couldLocalize = PrepareGameModeDescription(statName, outputString, stringLength, targetValue, localizedString);
            break;
        case MILESTONE_ATTACHMENTS:
            couldLocalize = PrepareAttachmentDescription(statName, outputString, stringLength, statsTableIndex, targetValue);
            break;
        case MILESTONE_KILLSTREAKS:
            couldLocalize = PrepareKillstreakDescription(statName, outputString, stringLength, statsTableIndex, targetValue);
            break;
        case MILESTONE_GRENADES:
            couldLocalize = PrepareGrenadeDescription(statName, outputString, stringLength, statsTableIndex, targetValue);
            break;
        default:
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp", 599, 0, "%s", "false") )
                __debugbreak();
            break;
    }
    return couldLocalize;
}

char __cdecl GetGlobalChallengeDescription(const char *name, char *outputString, int stringLength, int targetValue)
{
    const char *v5; // eax
    char *v6; // eax
    const char *v7; // eax
    char *v8; // eax
    ConversionArguments convArgs; // [esp+0h] [ebp-28h] BYREF

    memset(&convArgs, 0, sizeof(convArgs));
    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    611,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( !outputString )
        return 0;
    if ( !name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp", 615, 0, "%s", "name != NULL") )
    {
        __debugbreak();
    }
    if ( !name )
        return 0;
    if ( StringBeginsWith("gm_", name)
        || StringBeginsWith("kills_", name)
        || StringBeginsWith("basic_", name)
        || StringBeginsWith("medal_", name)
        || StringBeginsWith("lifetime_", name)
        || StringBeginsWith("final_", name) )
    {
        if ( I_stricmp(name, "lifetime_time_played_total") )
            convArgs.args[convArgs.argCount] = va("%d", targetValue);
        else
            convArgs.args[convArgs.argCount] = va("%d", targetValue / 3600);
        ++convArgs.argCount;
        v5 = va("CH_%s_DESC", name);
        v6 = UI_SafeTranslateString(v5);
        UI_ReplaceConversions(v6, &convArgs, outputString, stringLength);
    }
    else
    {
        convArgs.args[convArgs.argCount++] = va("%d", targetValue);
        v7 = va("%s_DESC", name);
        v8 = UI_SafeTranslateString(v7);
        UI_ReplaceConversions(v8, &convArgs, outputString, stringLength);
    }
    return 1;
}

char __cdecl PrepareAttachmentDescription(
                const char *name,
                char *outputString,
                int stringLength,
                int index,
                int targetValue)
{
    char *v6; // eax
    char *v7; // eax
    ConversionArguments convArgs; // [esp+0h] [ebp-28h] BYREF

    memset(&convArgs, 0, sizeof(convArgs));
    if ( !name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp", 651, 0, "%s", "name != NULL") )
    {
        __debugbreak();
    }
    if ( !name )
        return 0;
    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    655,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( !outputString )
        return 0;
    if ( !I_stricmp(name, "kills") )
    {
        if ( index == 1 || index == 2 || index == 3 || index == 4 || index == 5 || index == 6 || index == 21 )
        {
            convArgs.args[convArgs.argCount++] = va("%d", targetValue);
            v6 = UI_SafeTranslateString("WEAPON_ATTACHMENT_SIGHT_KILLS_CHALLENGE_DESC");
            UI_ReplaceConversions(v6, &convArgs, outputString, stringLength);
        }
        else
        {
            convArgs.args[convArgs.argCount++] = va("%d", targetValue);
            v7 = UI_SafeTranslateString("WEAPON_ATTACHMENT_KILLS_CHALLENGE_DESC");
            UI_ReplaceConversions(v7, &convArgs, outputString, stringLength);
        }
    }
    return 1;
}

char __cdecl PrepareKillstreakDescription(
                const char *name,
                char *outputString,
                int stringLength,
                int index,
                int targetValue)
{
    const char *v6; // eax
    const char *v7; // eax
    char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    char *v11; // eax
    const char *v12; // eax
    char *v13; // eax
    const char *v14; // eax
    const char *v15; // eax
    const char *v16; // eax
    char *v17; // eax
    const char *v18; // eax
    char *v19; // eax
    const char *v20; // eax
    const char *itemName; // [esp+0h] [ebp-30h]
    bool couldLocalize; // [esp+7h] [ebp-29h]
    ConversionArguments convArgs; // [esp+8h] [ebp-28h] BYREF

    memset(&convArgs, 0, sizeof(convArgs));
    couldLocalize = 0;
    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    684,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( !outputString )
        return 0;
    if ( !name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp", 688, 0, "%s", "name != NULL") )
    {
        __debugbreak();
    }
    if ( !name )
        return 0;
    itemName = BG_UnlockablesGetItemName(index);
    if ( !itemName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    694,
                    0,
                    "%s",
                    "itemName != NULL") )
    {
        __debugbreak();
    }
    if ( !itemName )
        return 0;
    if ( *itemName )
    {
        if ( !I_stricmp(name, "kills") )
        {
            v6 = va("%s_KILLS_CHALLENGE_DESC", itemName);
            if ( SEH_StringEd_GetString((char*)v6) )
            {
                convArgs.args[convArgs.argCount++] = va("%d", targetValue);
                v7 = va("%s_KILLS_CHALLENGE_DESC", itemName);
                v8 = UI_SafeTranslateString(v7);
            }
            else
            {
                convArgs.args[convArgs.argCount++] = va("%d", targetValue);
                v9 = BG_UnlockablesGetItemName(index);
                convArgs.args[convArgs.argCount++] = UI_SafeTranslateString(v9);
                v8 = UI_SafeTranslateString("KILLSTREAK_KILLS_CHALLENGE_DESC");
            }
            UI_ReplaceConversions(v8, &convArgs, outputString, stringLength);
            couldLocalize = 1;
        }
        if ( !I_stricmp(name, "used") )
        {
            v10 = BG_UnlockablesGetItemName(index);
            v11 = UI_SafeTranslateString(v10);
            convArgs.args[convArgs.argCount++] = v11;
            v12 = va("%d", targetValue);
            convArgs.args[convArgs.argCount++] = v12;
            v13 = UI_SafeTranslateString("KILLSTREAK_EARNED_CHALLENGE_DESC");
            UI_ReplaceConversions(v13, &convArgs, outputString, stringLength);
            couldLocalize = 1;
        }
        if ( !I_stricmp(name, "destroyed") )
        {
            v14 = va("%s_DESTROYED_CHALLENGE_DESC", itemName);
            if ( SEH_StringEd_GetString((char*)v14) )
            {
                v15 = va("%d", targetValue);
                convArgs.args[convArgs.argCount++] = v15;
                v16 = va("%s_DESTROYED_CHALLENGE_DESC", itemName);
                v17 = UI_SafeTranslateString(v16);
            }
            else
            {
                v18 = BG_UnlockablesGetItemName(index);
                v19 = UI_SafeTranslateString(v18);
                convArgs.args[convArgs.argCount++] = v19;
                v20 = va("%d", targetValue);
                convArgs.args[convArgs.argCount++] = v20;
                v17 = UI_SafeTranslateString("KILLSTREAK_DESTROYED_CHALLENGE_DESC");
            }
            UI_ReplaceConversions(v17, &convArgs, outputString, stringLength);
            return 1;
        }
    }
    return couldLocalize;
}

char __cdecl PrepareGrenadeDescription(
                const char *name,
                char *outputString,
                int stringLength,
                int index,
                int targetValue)
{
    char *v6; // eax
    const char *itemName; // [esp+0h] [ebp-2Ch]
    ConversionArguments convArgs; // [esp+4h] [ebp-28h] BYREF

    memset(&convArgs, 0, sizeof(convArgs));
    itemName = BG_UnlockablesGetItemName(index);
    if ( !name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp", 748, 0, "%s", "name != NULL") )
    {
        __debugbreak();
    }
    if ( !name )
        return 0;
    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    752,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( !outputString )
        return 0;
    if ( !itemName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    756,
                    0,
                    "%s",
                    "itemName != NULL") )
    {
        __debugbreak();
    }
    if ( !itemName )
        return 0;
    convArgs.args[convArgs.argCount++] = va("%d", targetValue);
    convArgs.args[convArgs.argCount++] = UI_SafeTranslateString(itemName);
    v6 = UI_SafeTranslateString("WEAPON_GRENADE_KILLS_CHALLENGE_DESC");
    UI_ReplaceConversions(v6, &convArgs, outputString, stringLength);
    return 1;
}

char __cdecl PrepareWeaponDescription(
                const char *name,
                char *outputString,
                int stringLength,
                int targetValue,
                int milestoneType,
                const char *weaponLocalizedString)
{
    const char *v7; // eax
    char *v8; // eax
    char *v9; // eax
    bool couldLocalize; // [esp+3h] [ebp-29h]
    ConversionArguments convArgs; // [esp+4h] [ebp-28h] BYREF

    memset(&convArgs, 0, sizeof(convArgs));
    couldLocalize = 0;
    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    774,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( !outputString )
        return 0;
    if ( !name
        && !weaponLocalizedString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    778,
                    0,
                    "%s",
                    "name != NULL || weaponLocalizedString != NULL") )
    {
        __debugbreak();
    }
    if ( !name || !weaponLocalizedString )
        return 0;
    convArgs.args[convArgs.argCount++] = va("%d", targetValue);
    v7 = va("STATS_MILESTONE_STATNAME_%s_SMALL", name);
    convArgs.args[convArgs.argCount++] = UI_SafeTranslateString(v7);
    convArgs.args[convArgs.argCount++] = weaponLocalizedString;
    if ( milestoneType == 1 )
    {
        v8 = UI_SafeTranslateString("STATS_WEAPONITEM_DESC");
        UI_ReplaceConversions(v8, &convArgs, outputString, stringLength);
        return 1;
    }
    else if ( milestoneType == 4 )
    {
        v9 = UI_SafeTranslateString("STATS_WEAPONGROUP_DESC");
        UI_ReplaceConversions(v9, &convArgs, outputString, stringLength);
        return 1;
    }
    return couldLocalize;
}

char __cdecl PrepareGameModeDescription(
                const char *name,
                char *outputString,
                int stringLength,
                int targetValue,
                const char *gameModeLocalizedString)
{
    const char *v6; // eax
    char *v7; // eax
    ConversionArguments convArgs; // [esp+0h] [ebp-28h] BYREF

    memset(&convArgs, 0, sizeof(convArgs));
    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    805,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( !outputString )
        return 0;
    if ( !name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp", 809, 0, "%s", "name != NULL") )
    {
        __debugbreak();
    }
    if ( !name )
        return 0;
    convArgs.args[convArgs.argCount++] = va("%d", targetValue);
    if ( gameModeLocalizedString )
        convArgs.args[convArgs.argCount++] = gameModeLocalizedString;
    v6 = va("CH_GM_%s_DESC", name);
    v7 = UI_SafeTranslateString(v6);
    UI_ReplaceConversions(v7, &convArgs, outputString, stringLength);
    return 1;
}

char __cdecl CL_GetMilestoneLocalizedXP(
                char *outputString,
                int stringLength,
                float xpScale,
                int tier,
                const char *indexName)
{
    char *localizedMilestoneXP; // [esp+0h] [ebp-10h]
    char *milestoneXP; // [esp+4h] [ebp-Ch]
    int xpAmount; // [esp+8h] [ebp-8h]
    const char *xpStringAmount; // [esp+Ch] [ebp-4h]

    if ( stringLength < 32
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    825,
                    0,
                    "%s",
                    "stringLength >= MAX_MILESTONE_XP_STRING_SIZE") )
    {
        __debugbreak();
    }
    if ( !outputString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    826,
                    0,
                    "%s",
                    "outputString != NULL") )
    {
        __debugbreak();
    }
    if ( !outputString )
        return 0;
    if ( !indexName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    830,
                    0,
                    "%s",
                    "indexName != NULL") )
    {
        __debugbreak();
    }
    if ( !indexName )
        return 0;
    memset(outputString, 0, stringLength);
    xpStringAmount = CL_GetMilestoneData(indexName, MILESTONE_COLUMN_XPEARNED);
    if ( !xpStringAmount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    841,
                    0,
                    "%s",
                    "xpStringAmount != NULL") )
    {
        __debugbreak();
    }
    if ( !xpStringAmount )
        return 0;
    xpAmount = (int)(float)((float)atoi(xpStringAmount) * xpScale);
    if ( !xpAmount )
        return 0;
    milestoneXP = (char *)SEH_LocalizeTextMessage("RANK_POPUP_XP", "popUpDesc", LOCMSG_SAFE);
    if ( !milestoneXP
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    851,
                    0,
                    "%s",
                    "milestoneXP != NULL") )
    {
        __debugbreak();
    }
    if ( !milestoneXP )
        return 0;
    localizedMilestoneXP = UI_ReplaceConversionInt(milestoneXP, xpAmount);
    Com_sprintf(outputString, stringLength, "%s", localizedMilestoneXP);
    return 1;
}

void __cdecl CL_GetMilestoneBackingMaterial(
                char *backingMaterialName,
                int stringLength,
                const char *statName,
                const char *perkName)
{
    const StringTable *statsTable; // [esp+0h] [ebp-Ch] BYREF
    int row; // [esp+4h] [ebp-8h]
    const char *loadoutSlot; // [esp+8h] [ebp-4h]

    if ( !backingMaterialName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    861,
                    0,
                    "%s",
                    "backingMaterialName != NULL") )
    {
        __debugbreak();
    }
    if ( backingMaterialName )
    {
        if ( !statName
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                        865,
                        0,
                        "%s",
                        "statName != NULL") )
        {
            __debugbreak();
        }
        if ( statName )
        {
            if ( stringLength < 256
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                            869,
                            0,
                            "%s",
                            "stringLength >= MAX_MILESTONE_MATERIAL_STRING_SIZE") )
            {
                __debugbreak();
            }
            memset(backingMaterialName, 0, stringLength);
            Com_sprintf(backingMaterialName, stringLength, "%s", "hud_medal_burst");
            if ( perkName && StringBeginsWith("perks_", statName) )
            {
                StringTable_GetAsset("mp/statstable.csv", (XAssetHeader *)&statsTable);
                if ( !statsTable
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                                878,
                                0,
                                "%s",
                                "statsTable") )
                {
                    __debugbreak();
                }
                if ( statsTable )
                {
                    row = StringTable_LookupRowNumForValue(statsTable, 3, perkName);
                    loadoutSlot = StringTable_GetColumnValueForRow(statsTable, row, 13);
                    if ( loadoutSlot )
                    {
                        if ( I_stricmp(loadoutSlot, "specialty2") )
                        {
                            if ( !I_stricmp(loadoutSlot, "specialty3") )
                                Com_sprintf(backingMaterialName, stringLength, "%s", "hud_medal_burst_green");
                        }
                        else
                        {
                            Com_sprintf(backingMaterialName, stringLength, "%s", "hud_medal_burst_orange");
                        }
                    }
                }
            }
        }
    }
}

void __cdecl CL_GetMilestoneMaterial(char *materialName, int stringLength, const char *statName, const char *perkName)
{
    const StringTable *statsTable; // [esp+0h] [ebp-Ch] BYREF
    int row; // [esp+4h] [ebp-8h]
    const char *imageName; // [esp+8h] [ebp-4h]

    if ( !materialName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    904,
                    0,
                    "%s",
                    "materialName != NULL") )
    {
        __debugbreak();
    }
    if ( materialName )
    {
        if ( !statName
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                        908,
                        0,
                        "%s",
                        "statName != NULL") )
        {
            __debugbreak();
        }
        if ( statName )
        {
            if ( stringLength < 256
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                            912,
                            0,
                            "%s",
                            "stringLength >= MAX_MILESTONE_MATERIAL_STRING_SIZE") )
            {
                __debugbreak();
            }
            memset(materialName, 0, stringLength);
            Com_sprintf(materialName, stringLength, "%s", "menu_mp_lobby_aar_award_challenge");
            if ( perkName && StringBeginsWith("perks_", statName) )
            {
                StringTable_GetAsset("mp/statstable.csv", (XAssetHeader *)&statsTable);
                if ( !statsTable
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                                921,
                                0,
                                "%s",
                                "statsTable") )
                {
                    __debugbreak();
                }
                if ( statsTable )
                {
                    row = StringTable_LookupRowNumForValue(statsTable, 3, perkName);
                    imageName = StringTable_GetColumnValueForRow(statsTable, row, 6);
                    if ( imageName )
                        Com_sprintf(materialName, stringLength, "%s", imageName);
                }
            }
        }
    }
}

statsMilestoneTypes_t __cdecl getMilestoneType(const char *statsType)
{
    if ( !statsType
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    978,
                    0,
                    "%s",
                    "statsType != NULL") )
    {
        __debugbreak();
    }
    if ( !statsType )
        return MILESTONE_INVALID;
    if ( !strcmp(statsType, "global") )
        return MILESTONE_GLOBAL;
    if ( !strcmp(statsType, "killstreak") )
        return MILESTONE_KILLSTREAKS;
    if ( !strcmp(statsType, "group") )
        return MILESTONE_GROUP;
    if ( !strcmp(statsType, "attachment") )
        return MILESTONE_ATTACHMENTS;
    if ( !strcmp(statsType, "gamemode") )
        return MILESTONE_GAMEMODE;
    if ( !strcmp(statsType, "weapon_grenade") )
        return MILESTONE_GRENADES;
    return MILESTONE_WEAPON;
}

void __cdecl CL_MilestoneTier_DecToRoman(int milestoneTierId, char *milestoneTierRoman, int stringLength)
{
    if ( !milestoneTierRoman
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                    1020,
                    0,
                    "%s",
                    "milestoneTierRoman != NULL") )
    {
        __debugbreak();
    }
    if ( milestoneTierRoman )
    {
        memset(milestoneTierRoman, 0, stringLength);
        if ( stringLength < 5
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_milestone.cpp",
                        1026,
                        0,
                        "%s",
                        "MAX_MILESTONE_ROMAN_STRING_SIZE <= stringLength") )
        {
            __debugbreak();
        }
        if ( stringLength <= 5 )
        {
            switch ( milestoneTierId )
            {
                case 0:
                    I_strncpyz(milestoneTierRoman, "I", stringLength);
                    break;
                case 1:
                    I_strncpyz(milestoneTierRoman, "II", stringLength);
                    break;
                case 2:
                    I_strncpyz(milestoneTierRoman, "III", stringLength);
                    break;
                case 3:
                    I_strncpyz(milestoneTierRoman, "IV", stringLength);
                    break;
                case 4:
                    I_strncpyz(milestoneTierRoman, "V", stringLength);
                    break;
                case 5:
                    I_strncpyz(milestoneTierRoman, "VI", stringLength);
                    break;
                case 6:
                    I_strncpyz(milestoneTierRoman, "VII", stringLength);
                    break;
                case 7:
                    I_strncpyz(milestoneTierRoman, "VIII", stringLength);
                    break;
                case 8:
                    I_strncpyz(milestoneTierRoman, "IX", stringLength);
                    break;
                case 9:
                    I_strncpyz(milestoneTierRoman, "X", stringLength);
                    break;
                default:
                    Com_Printf(16, "Conversion form Milestone Tier ID to roman numeral failed.");
                    break;
            }
        }
    }
}

