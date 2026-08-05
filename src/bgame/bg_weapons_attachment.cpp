#include "bg_weapons_attachment.h"
#include <universal/assertive.h>
#include <universal/q_shared.h>
#include <qcommon/common.h>
#include "bg_unlockable_items.h"
#include <game_mp/g_main_mp.h>
#include <universal/q_parse.h>

const char *s_attachmentPointNames[5] =
{ "none", "top", "bottom", "trigger", "muzzle" };

const char *s_attachmentGroupNames[3] =
{ "attachment", "weaponoption", "point" };

const char *s_attachmentNames[24] =
{
  "none",
  "reflex",
  "elbit",
  "acog",
  "lps",
  "vzoom",
  "ir",
  "gl",
  "mk",
  "silencer",
  "grip",
  "extclip",
  "dualclip",
  "saddle",
  "fmj",
  "hp",
  "rf",
  "ft",
  "bayonet",
  "auto",
  "speed",
  "upgradesight",
  "snub",
  "dw"
};

const char *s_weaponOptionGroupNames[6] =
{ "camo", "reticle", "lens", "tag", "emblem", "reticle_color" };

int s_numweaponTableAttachmentPoints;
int s_weaponOptionListForGroup[6][96];
int s_numweaponTableWeaponOptions;
WeaponOptionTableEntry s_weaponTableWeaponOptions[96];
int s_numweaponTableWeaponOptionsForGroup[6];
AttachmentPointTableEntry s_weaponTableAttachmentPoints[5];
AttachmentTableEntry s_weaponTableAttachments[24];
int s_numWeaponTableAttachments;


const char **__cdecl BG_GetAttachmentNames()
{
    return s_attachmentNames;
}

const char *__cdecl BG_GetAttachmentName(eAttachment index)
{
    if ( (unsigned int)index >= (ATTACHMENT_INVALID|ATTACHMENT_REFLEX)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_attachment.cpp",
                    89,
                    0,
                    "index doesn't index ATTACHMENT_COUNT + 1\n\t%i not in [0, %i)",
                    index,
                    25) )
    {
        __debugbreak();
    }
    if ( index == ATTACHMENT_COUNT )
        return 0;
    else
        return s_attachmentNames[index];
}

eAttachment __cdecl BG_GetAttachmentIndex(const char *name)
{
    int index; // [esp+0h] [ebp-4h]

    if ( !name || !*name )
        return ATTACHMENT_INVALID;
    for ( index = 0; index < 24; ++index )
    {
        if ( !I_strcmp(name, s_attachmentNames[index]) )
            return (eAttachment)index;
    }
    return ATTACHMENT_INVALID;
}

const char *__cdecl BG_GetAttachmentPointName(eAttachmentPoint index)
{
    if ( (unsigned int)index >= (ATTACHMENT_POINT_MUZZLE|ATTACHMENT_POINT_BOTTOM)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_attachment.cpp",
                    112,
                    0,
                    "index doesn't index ATTACHMENT_POINT_COUNT + 1\n\t%i not in [0, %i)",
                    index,
                    6) )
    {
        __debugbreak();
    }
    if ( index == ATTACHMENT_POINT_COUNT )
        return 0;
    else
        return s_attachmentPointNames[index];
}

eAttachmentPoint __cdecl BG_GetAttachmentPointIndex(const char *name)
{
    int index; // [esp+0h] [ebp-4h]

    if ( !name || !*name )
        return ATTACHMENT_POINT_INVALID;
    for ( index = 0; index < 5; ++index )
    {
        if ( !I_strcmp(name, s_attachmentPointNames[index]) )
            return (eAttachmentPoint)index;
    }
    return ATTACHMENT_POINT_INVALID;
}

int __cdecl BG_GetWeaponOptionGroup(const char *name)
{
    int index; // [esp+0h] [ebp-4h]

    if ( !name || !*name )
        return 6;
    for ( index = 0; index < 6; ++index )
    {
        if ( !I_stricmp(name, s_weaponOptionGroupNames[index]) )
            return index;
    }
    return 6;
}

const char *__cdecl BG_GetAttachmentDisplayName(eAttachment attachment)
{
    int attachmentArrayIndex; // [esp+0h] [ebp-4h]

    attachmentArrayIndex = BG_GetAttachmentArrayIndex(attachment);
    if ( attachmentArrayIndex == 24 )
        return 0;
    if ( s_weaponTableAttachments[attachmentArrayIndex].isValid )
        return s_weaponTableAttachments[attachmentArrayIndex].values.displayName;
    return 0;
}

int __cdecl BG_GetAttachmentArrayIndex(eAttachment attachment)
{
    int attachmentIndex; // [esp+0h] [ebp-4h]

    for ( attachmentIndex = 0; attachmentIndex < s_numWeaponTableAttachments; ++attachmentIndex )
    {
        if ( s_weaponTableAttachments[attachmentIndex].isValid
            && s_weaponTableAttachments[attachmentIndex].attachmentIndex == attachment )
        {
            return attachmentIndex;
        }
    }
    return 24;
}

const char *__cdecl BG_GetAttachmentReference(eAttachment attachment)
{
    int attachmentArrayIndex; // [esp+0h] [ebp-4h]

    attachmentArrayIndex = BG_GetAttachmentArrayIndex(attachment);
    if ( attachmentArrayIndex == 24 )
        return 0;
    if ( s_weaponTableAttachments[attachmentArrayIndex].isValid )
        return s_weaponTableAttachments[attachmentArrayIndex].values.referenceName;
    return 0;
}

const char *__cdecl BG_GetAttachmentImage(eAttachment attachment)
{
    int attachmentArrayIndex; // [esp+0h] [ebp-4h]

    attachmentArrayIndex = BG_GetAttachmentArrayIndex(attachment);
    if ( attachmentArrayIndex == 24 )
        return 0;
    if ( s_weaponTableAttachments[attachmentArrayIndex].isValid )
        return s_weaponTableAttachments[attachmentArrayIndex].values.imageName;
    return 0;
}

const char *__cdecl BG_GetAttachmentDesc(eAttachment attachment)
{
    int attachmentArrayIndex; // [esp+0h] [ebp-4h]

    attachmentArrayIndex = BG_GetAttachmentArrayIndex(attachment);
    if ( attachmentArrayIndex == 24 )
        return 0;
    if ( s_weaponTableAttachments[attachmentArrayIndex].isValid )
        return s_weaponTableAttachments[attachmentArrayIndex].values.description;
    return 0;
}

int __cdecl BG_GetAttachmentCost(eAttachment attachment)
{
    int attachmentArrayIndex; // [esp+0h] [ebp-4h]

    attachmentArrayIndex = BG_GetAttachmentArrayIndex(attachment);
    if ( attachmentArrayIndex == 24 )
        return 0;
    if ( s_weaponTableAttachments[attachmentArrayIndex].isValid )
        return s_weaponTableAttachments[attachmentArrayIndex].values.cost;
    return 0;
}

int __cdecl BG_GetAttachmentPointCost(eAttachmentPoint attachmentPoint)
{
    if ( attachmentPoint >= ATTACHMENT_POINT_COUNT )
        return 0;
    if ( s_weaponTableAttachmentPoints[attachmentPoint].isValid )
        return s_weaponTableAttachmentPoints[attachmentPoint].values.cost;
    return 0;
}

eAttachmentPoint __cdecl BG_GetAttachmentPointIndexFromAttachment(eAttachment attachment)
{
    int attachmentIndex; // [esp+0h] [ebp-4h]

    for ( attachmentIndex = 0; attachmentIndex < s_numWeaponTableAttachments; ++attachmentIndex )
    {
        if ( s_weaponTableAttachments[attachmentIndex].isValid
            && s_weaponTableAttachments[attachmentIndex].attachmentIndex == attachment )
        {
            return s_weaponTableAttachments[attachmentIndex].attachmentPointIndex;
        }
    }
    return ATTACHMENT_POINT_INVALID;
}

bool __cdecl BG_AreAttachmentsCompatible(eAttachment attachment1, eAttachment attachment2)
{
    int currAttachment; // [esp+0h] [ebp-Ch]
    int attachment1Index; // [esp+4h] [ebp-8h]
    int attachment2Index; // [esp+8h] [ebp-4h]

    if ( attachment1 == ATTACHMENT_COUNT || attachment2 == ATTACHMENT_COUNT )
        return 0;
    if ( attachment1 == ATTACHMENT_NONE || attachment2 == ATTACHMENT_NONE )
        return 1;
    attachment1Index = -1;
    attachment2Index = -1;
    for ( currAttachment = 0; currAttachment < s_numWeaponTableAttachments; ++currAttachment )
    {
        if ( s_weaponTableAttachments[currAttachment].attachmentIndex == attachment1 )
            attachment1Index = currAttachment;
        if ( s_weaponTableAttachments[currAttachment].attachmentIndex == attachment2 )
            attachment2Index = currAttachment;
    }
    if ( attachment1Index == -1 || attachment2Index == -1 )
        return 0;
    return s_weaponTableAttachments[attachment1Index].compatibleAttachments[attachment2]
            && s_weaponTableAttachments[attachment2Index].compatibleAttachments[attachment1];
}

const AttachmentTableEntry *__cdecl BG_GetAttachmentTableEntry(unsigned int index)
{
    if ( index >= 0x18
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_attachment.cpp",
                    346,
                    0,
                    "index doesn't index ATTACHMENT_COUNT\n\t%i not in [0, %i)",
                    index,
                    24) )
    {
        __debugbreak();
    }
    return &s_weaponTableAttachments[index];
}

const AttachmentPointTableEntry *__cdecl BG_GetAttachmentPointTableEntry(unsigned int index)
{
    if ( index >= 5
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_attachment.cpp",
                    353,
                    0,
                    "index doesn't index ATTACHMENT_POINT_COUNT\n\t%i not in [0, %i)",
                    index,
                    5) )
    {
        __debugbreak();
    }
    return &s_weaponTableAttachmentPoints[index];
}

int __cdecl BG_GetWeaponAttachmentCount()
{
    return s_numWeaponTableAttachments;
}

int __cdecl BG_GetWeaponOptionCountForGroup(eWeaponOptionGroup optionGroup)
{
    if ( (unsigned int)optionGroup <= WEAPONOPTION_GROUP_RETICLE_COLOR )
        return s_numweaponTableWeaponOptionsForGroup[optionGroup];
    Com_PrintWarning(17, "Unrecognized weapongroup %d\n", optionGroup);
    return 0;
}

int __cdecl BG_GetWeaponOptionCount()
{
    int optionGroup; // [esp+0h] [ebp-8h]
    int weaponOptionCount; // [esp+4h] [ebp-4h]

    weaponOptionCount = 0;
    for ( optionGroup = 0; optionGroup < 6; ++optionGroup )
        weaponOptionCount += s_numweaponTableWeaponOptionsForGroup[optionGroup];
    return weaponOptionCount;
}

const char *__cdecl BG_GetWeaponOptionDisplayName(int weaponOption)
{
    int weaponOptionArrayIndex; // [esp+0h] [ebp-4h]

    weaponOptionArrayIndex = BG_GetWeaponOptionArrayIndex(weaponOption);
    if ( weaponOptionArrayIndex == -1 )
        return 0;
    if ( s_weaponTableWeaponOptions[weaponOptionArrayIndex].isValid )
        return s_weaponTableWeaponOptions[weaponOptionArrayIndex].values.displayName;
    return 0;
}

int __cdecl BG_GetWeaponOptionArrayIndex(int weaponOption)
{
    int optionIndex; // [esp+0h] [ebp-4h]

    if ( weaponOption > BG_GetWeaponOptionCount() )
        return -1;
    for ( optionIndex = 0; optionIndex < s_numweaponTableWeaponOptions; ++optionIndex )
    {
        if ( s_weaponTableWeaponOptions[optionIndex].isValid
            && s_weaponTableWeaponOptions[optionIndex].weaponOptionIndex == weaponOption )
        {
            return optionIndex;
        }
    }
    return -1;
}

const char *__cdecl BG_GetWeaponOptionReference(int weaponOption)
{
    int weaponOptionArrayIndex; // [esp+0h] [ebp-4h]

    weaponOptionArrayIndex = BG_GetWeaponOptionArrayIndex(weaponOption);
    if ( weaponOptionArrayIndex == -1 )
        return 0;
    if ( s_weaponTableWeaponOptions[weaponOptionArrayIndex].isValid )
        return s_weaponTableWeaponOptions[weaponOptionArrayIndex].values.referenceName;
    return 0;
}

const char *__cdecl BG_GetWeaponOptionImage(int weaponOption)
{
    int weaponOptionArrayIndex; // [esp+0h] [ebp-4h]

    weaponOptionArrayIndex = BG_GetWeaponOptionArrayIndex(weaponOption);
    if ( weaponOptionArrayIndex == -1 )
        return 0;
    if ( s_weaponTableWeaponOptions[weaponOptionArrayIndex].isValid )
        return s_weaponTableWeaponOptions[weaponOptionArrayIndex].values.imageName;
    return 0;
}

int __cdecl BG_GetWeaponOptionCost(int weaponOption)
{
    int weaponOptionArrayIndex; // [esp+0h] [ebp-4h]

    weaponOptionArrayIndex = BG_GetWeaponOptionArrayIndex(weaponOption);
    if ( weaponOptionArrayIndex == -1 )
        return 0;
    if ( s_weaponTableWeaponOptions[weaponOptionArrayIndex].isValid )
        return s_weaponTableWeaponOptions[weaponOptionArrayIndex].values.cost;
    return 0;
}

eWeaponOptionGroup __cdecl BG_GetWeaponOptionGroup(int weaponOption)
{
    int weaponOptionArrayIndex; // [esp+0h] [ebp-4h]

    weaponOptionArrayIndex = BG_GetWeaponOptionArrayIndex(weaponOption);
    if ( weaponOptionArrayIndex == -1 )
        return WEAPONOPTION_GROUP_INVALID;
    if ( s_weaponTableWeaponOptions[weaponOptionArrayIndex].isValid )
        return s_weaponTableWeaponOptions[weaponOptionArrayIndex].weaponOptionGroup;
    return WEAPONOPTION_GROUP_INVALID;
}

int __cdecl BG_GetWeaponOptionUnlockLvl(int weaponOption)
{
    int weaponOptionArrayIndex; // [esp+0h] [ebp-4h]

    if ( BG_UnlockablesAllItemsUnlocked() )
        return 0;
    weaponOptionArrayIndex = BG_GetWeaponOptionArrayIndex(weaponOption);
    if ( weaponOptionArrayIndex == -1 )
        return 0;
    if ( s_weaponTableWeaponOptions[weaponOptionArrayIndex].isValid )
        return s_weaponTableWeaponOptions[weaponOptionArrayIndex].values.unlockLvl;
    return 0;
}

int __cdecl BG_GetWeaponOptionUnlockPLevel(int weaponOption)
{
    int weaponOptionArrayIndex; // [esp+0h] [ebp-4h]

    if ( BG_UnlockablesAllItemsUnlocked() )
        return 0;
    weaponOptionArrayIndex = BG_GetWeaponOptionArrayIndex(weaponOption);
    if ( weaponOptionArrayIndex == -1 )
        return 0;
    if ( s_weaponTableWeaponOptions[weaponOptionArrayIndex].isValid )
        return s_weaponTableWeaponOptions[weaponOptionArrayIndex].values.unlockPLevel;
    return 0;
}

int __cdecl BG_GetWeaponOptionNumFromIndexAndGroup(int index, eWeaponOptionGroup weaponOptionGroup)
{
    if ( index >= BG_GetWeaponOptionCountForGroup(weaponOptionGroup) )
        return -1;
    else
        return s_weaponOptionListForGroup[weaponOptionGroup][index];
}

int __cdecl BG_GetWeaponOptionFromName(const char *optionName)
{
    int optionIndex; // [esp+0h] [ebp-4h]

    for ( optionIndex = 0; optionIndex < s_numweaponTableWeaponOptions; ++optionIndex )
    {
        if ( s_weaponTableWeaponOptions[optionIndex].isValid
            && !I_stricmp(s_weaponTableWeaponOptions[optionIndex].values.referenceName, optionName) )
        {
            return s_weaponTableWeaponOptions[optionIndex].weaponOptionIndex;
        }
    }
    return -1;
}

void __cdecl BG_LoadWeaponAttachmentTable()
{
    const char *v0; // eax
    const char *v1; // eax
    int v2; // [esp+0h] [ebp-10h]
    int row; // [esp+4h] [ebp-Ch]
    const StringTable *attachmentTable; // [esp+8h] [ebp-8h] BYREF
    int count; // [esp+Ch] [ebp-4h] BYREF

    s_numWeaponTableAttachments = 0;
    s_numweaponTableWeaponOptions = 0;
    s_numweaponTableAttachmentPoints = 0;
    if ( !G_ExitAfterToolComplete() )
    {
        attachmentTable = 0;
        StringTable_GetAsset("mp/attachmenttable.csv", (XAssetHeader *)&attachmentTable);
        if ( !attachmentTable->columnCount || !attachmentTable->rowCount )
            Com_Error(ERR_DROP, "Couldn't load file or file is invalid '%s'", "mp/attachmenttable.csv");
        count = 0;
        while ( 1 )
        {
            v0 = va("%d", count);
            row = StringTable_LookupRowNumForValue(attachmentTable, 2, s_attachmentGroupNames[0], 0, v0, -1, 0);
            if ( row == -1 )
                break;
            if ( count >= 24
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_attachment.cpp",
                            723,
                            0,
                            "%s",
                            "ATTACHMENT_COUNT > count") )
            {
                __debugbreak();
            }
            if ( count >= 24 )
                break;
            BG_LoadAttachmentRow(attachmentTable, row, &s_weaponTableAttachments[s_numWeaponTableAttachments], &count);
            if ( s_weaponTableAttachments[s_numWeaponTableAttachments].attachmentIndex != ATTACHMENT_COUNT )
                ++s_numWeaponTableAttachments;
        }
        BG_LoadWeaponOptions(attachmentTable);
        count = 0;
        while ( 1 )
        {
            v1 = va("%d", count);
            v2 = StringTable_LookupRowNumForValue(attachmentTable, 2, s_attachmentGroupNames[2], 0, v1, -1, 0);
            if ( v2 == -1 || count >= 5 )
                break;
            BG_LoadAttachmentPointRow(
                attachmentTable,
                v2,
                &s_weaponTableAttachmentPoints[s_numweaponTableAttachmentPoints],
                &count);
            if ( s_weaponTableAttachmentPoints[s_numweaponTableAttachmentPoints].attachmentPointIndex != ATTACHMENT_POINT_COUNT )
                ++s_numweaponTableAttachmentPoints;
        }
    }
}

void __cdecl BG_LoadAttachmentRow(const StringTable *attachmentTable, int row, AttachmentTableEntry *entry, int *count)
{
    eAttachment attachmentNum; // [esp+0h] [ebp-10h]
    parseInfo_t *attachmentName; // [esp+4h] [ebp-Ch]
    const char *parseLocation; // [esp+8h] [ebp-8h] BYREF
    const char *compatibleAttachmentList; // [esp+Ch] [ebp-4h]

    if ( !entry
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_attachment.cpp", 558, 0, "%s", "entry") )
    {
        __debugbreak();
    }
    if ( !count
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_attachment.cpp", 559, 0, "%s", "count") )
    {
        __debugbreak();
    }
    BG_LoadWeaponAttachmentTableRow(attachmentTable, row, &entry->values);
    entry->attachmentIndex = (eAttachment)BG_GetAttachmentIndex(entry->values.referenceName);
    entry->attachmentPointIndex = (eAttachmentPoint)BG_GetAttachmentPointIndex(entry->values.pointName);
    entry->values.referenceName = BG_GetAttachmentName(entry->attachmentIndex);
    entry->values.pointName = BG_GetAttachmentPointName(entry->attachmentPointIndex);
    compatibleAttachmentList = StringTable_GetColumnValueForRow(attachmentTable, row, 11);
    if ( compatibleAttachmentList )
    {
        Com_BeginParseSession("compatibleAttachments");
        Com_SetSpaceDelimited(0);
        parseLocation = compatibleAttachmentList;
        while ( 1 )
        {
            attachmentName = Com_ParseOnLine(&parseLocation);
            if ( !parseLocation || !attachmentName )
                break;
            attachmentNum = (eAttachment)BG_GetAttachmentIndex(attachmentName->token);
            if ( attachmentNum == ATTACHMENT_COUNT )
                Com_PrintError(
                    17,
                    "Invalid attachment name '%s' in table '%s'",
                    attachmentName->token,
                    "mp/attachmenttable.csv");
            entry->compatibleAttachments[attachmentNum] = 1;
        }
        Com_EndParseSession();
    }
    entry->isValid = 1;
    if ( entry->attachmentPointIndex == ATTACHMENT_POINT_COUNT )
        Com_PrintWarning(
            17,
            "Unrecognized attachpoint name '%s' in %s row %i.\n",
            entry->values.pointName,
            "mp/attachmenttable.csv",
            row);
    ++*count;
}

void __cdecl BG_LoadWeaponAttachmentTableRow(
                const StringTable *attachmentTable,
                int row,
                WeaponAttachmentTableEntry *entry)
{
    const char *ColumnValueForRow; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax

    entry->referenceName = StringTable_GetColumnValueForRow(attachmentTable, row, 4);
    entry->pointName = StringTable_GetColumnValueForRow(attachmentTable, row, 1);
    entry->displayName = StringTable_GetColumnValueForRow(attachmentTable, row, 3);
    entry->imageName = StringTable_GetColumnValueForRow(attachmentTable, row, 6);
    entry->description = StringTable_GetColumnValueForRow(attachmentTable, row, 7);
    ColumnValueForRow = StringTable_GetColumnValueForRow(attachmentTable, row, 12);
    entry->cost = atoi(ColumnValueForRow);
    v4 = StringTable_GetColumnValueForRow(attachmentTable, row, 5);
    entry->count = atoi(v4);
    v5 = StringTable_GetColumnValueForRow(attachmentTable, row, 13);
    entry->unlockLvl = atoi(v5);
    v6 = StringTable_GetColumnValueForRow(attachmentTable, row, 14);
    entry->unlockPLevel = atoi(v6);
}

void __cdecl BG_LoadAttachmentPointRow(
                const StringTable *attachmentTable,
                int row,
                AttachmentPointTableEntry *entry,
                int *count)
{
    if ( !entry
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_attachment.cpp", 628, 0, "%s", "entry") )
    {
        __debugbreak();
    }
    if ( !count
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_attachment.cpp", 629, 0, "%s", "count") )
    {
        __debugbreak();
    }
    BG_LoadWeaponAttachmentTableRow(attachmentTable, row, &entry->values);
    entry->attachmentPointIndex = (eAttachmentPoint)BG_GetAttachmentPointIndex(entry->values.referenceName);
    entry->isValid = 1;
    ++*count;
}

void __cdecl BG_LoadWeaponOptions(const StringTable *attachmentTable)
{
    const char *v1; // eax
    int weaponOptionNum; // [esp+4h] [ebp-14h]
    int currentOptionCount; // [esp+8h] [ebp-10h]
    int weaponOptionGroup; // [esp+Ch] [ebp-Ch]
    int row; // [esp+10h] [ebp-8h]
    int count; // [esp+14h] [ebp-4h] BYREF

    if ( !attachmentTable
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_attachment.cpp",
                    641,
                    0,
                    "%s",
                    "attachmentTable") )
    {
        __debugbreak();
    }
    if ( attachmentTable )
    {
        count = 0;
        while ( 1 )
        {
            v1 = va("%d", count);
            row = StringTable_LookupRowNumForValue(attachmentTable, 2, s_attachmentGroupNames[1], 0, v1, -1, 0);
            if ( row == -1 )
                break;
            if ( count >= 96
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_attachment.cpp",
                            658,
                            0,
                            "%s",
                            "MAX_WEAPON_OPTIONS > count") )
            {
                __debugbreak();
            }
            if ( count >= 96 )
                break;
            BG_LoadWeaponOptionRow(attachmentTable, row, &s_weaponTableWeaponOptions[s_numweaponTableWeaponOptions], &count);
            ++s_numweaponTableWeaponOptions;
        }
        memset(s_weaponOptionListForGroup, 0, sizeof(s_weaponOptionListForGroup));
        s_numweaponTableWeaponOptionsForGroup[0] = 0;
        s_numweaponTableWeaponOptionsForGroup[1] = 0;
        s_numweaponTableWeaponOptionsForGroup[2] = 0;
        s_numweaponTableWeaponOptionsForGroup[3] = 0;
        s_numweaponTableWeaponOptionsForGroup[4] = 0;
        s_numweaponTableWeaponOptionsForGroup[5] = 0;
        for ( weaponOptionGroup = 0; weaponOptionGroup < 6; ++weaponOptionGroup )
        {
            currentOptionCount = 0;
            for ( weaponOptionNum = 0; weaponOptionNum < s_numweaponTableWeaponOptions; ++weaponOptionNum )
            {
                if ( s_weaponTableWeaponOptions[weaponOptionNum].isValid
                    && s_weaponTableWeaponOptions[weaponOptionNum].weaponOptionGroup == weaponOptionGroup )
                {
                    s_weaponOptionListForGroup[weaponOptionGroup][currentOptionCount++] = weaponOptionNum;
                }
            }
            s_numweaponTableWeaponOptionsForGroup[weaponOptionGroup] = currentOptionCount;
        }
    }
}

void __cdecl BG_LoadWeaponOptionRow(
                const StringTable *attachmentTable,
                int row,
                WeaponOptionTableEntry *entry,
                int *count)
{
    if ( !entry
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_attachment.cpp", 607, 0, "%s", "entry") )
    {
        __debugbreak();
    }
    if ( !count
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_attachment.cpp", 608, 0, "%s", "count") )
    {
        __debugbreak();
    }
    BG_LoadWeaponAttachmentTableRow(attachmentTable, row, &entry->values);
    entry->weaponOptionIndex = *count;
    entry->weaponOptionGroup = (eWeaponOptionGroup)BG_GetWeaponOptionGroup(entry->values.pointName);
    if ( entry->weaponOptionGroup == WEAPONOPTION_GROUP_COUNT )
        Com_PrintWarning(
            17,
            "Unrecognized weaponoption name '%s' in %s row %i.\n",
            entry->values.pointName,
            "mp/attachmenttable.csv",
            row);
    else
        entry->isValid = 1;
    ++*count;
}

