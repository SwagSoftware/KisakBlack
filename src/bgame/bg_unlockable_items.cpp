#include "bg_unlockable_items.h"
#include <qcommon/com_gamemodes.h>
#include <ddl/ddl_api.h>
#include <live/live_storage.h>
#include <live/live_stats.h>
#include <ui_mp/ui_gametype_custom_mp.h>
#include <universal/com_stringtable_obj.h>
#include <universal/q_parse.h>
#include <live/live_counter.h>
#include "bg_weapons_def.h"
#include <ui_mp/ui_gametype_variants_mp.h>
#include <client_mp/cl_main_mp.h>

const char *connectionString_4[11] =
{
  "CA_DISCONNECTED",
  "CA_CINEMATIC",
  "CA_UICINEMATIC",
  "CA_LOGO",
  "CA_CONNECTING",
  "CA_CHALLENGING",
  "CA_CONNECTED",
  "CA_SENDINGSTATS",
  "CA_LOADING",
  "CA_PRIMED",
  "CA_ACTIVE"
};

const char *s_itemGroupNames[24] =
{
  "weapon_smg",
  "weapon_assault",
  "weapon_cqb",
  "weapon_lmg",
  "weapon_sniper",
  "weapon_pistol",
  "weapon_launcher",
  "weapon_special",
  "weapon_shotgun",
  "weapon_explosive",
  "weapon_grenade",
  "weapon_masterkey",
  "weapon_grenadelauncher",
  "weapon_flamethrower",
  "specialty",
  "specialgrenade",
  "inventory",
  "head",
  "headgear",
  "body",
  "miscweapon",
  "feature",
  "deathstreak",
  "killstreak"
};

const char *s_customClassNames[10] =
{
  "customClass1",
  "customClass2",
  "customClass3",
  "customClass4",
  "customClass5",
  "prestigeClass1",
  "prestigeClass2",
  "prestigeClass3",
  "prestigeClass4",
  "prestigeClass5"
};

const char *s_classNames[20] =
{
  "CLASS_SMG",
  "CLASS_CQB",
  "CLASS_ASSAULT",
  "CLASS_LMG",
  "CLASS_SNIPER",
  "CLASS_CUSTOM_SMG",
  "CLASS_CUSTOM_CQB",
  "CLASS_CUSTOM_ASSAULT",
  "CLASS_CUSTOM_LMG",
  "CLASS_CUSTOM_SNIPER",
  "CLASS_WAGER1",
  "CLASS_WAGER2",
  "CLASS_WAGER3",
  "CLASS_WAGER4",
  "CLASS_WAGER5",
  "CLASS_WAGER6",
  "CLASS_WAGER7",
  "CLASS_WAGER8",
  "CLASS_WAGER9",
  "CLASS_WAGER10"
};

const char *s_loadoutNames[40] =
{
  "primary",
  "primaryattachment",
  "primaryattachmenttop",
  "primaryattachmentbottom",
  "primaryattachmenttrigger",
  "primaryattachmentmuzzle",
  "primarycamo",
  "primaryreticle",
  "primaryreticlecolor",
  "primarylens",
  "primaryemblem",
  "primarytag",
  "secondary",
  "secondaryattachment",
  "secondaryattachmenttop",
  "secondaryattachmentbottom",
  "secondaryattachmenttrigger",
  "secondaryattachmentmuzzle",
  "secondarycamo",
  "secondaryreticle",
  "secondaryreticlecolor",
  "secondarylens",
  "secondaryemblem",
  "secondarytag",
  "specialty1",
  "specialty2",
  "specialty3",
  "classbonus",
  "head",
  "body",
  "primarygrenade",
  "specialgrenade",
  "equipment",
  "facepaintpattern",
  "facepaintcolor",
  "killstreak1",
  "killstreak2",
  "killstreak3",
  "deathstreak",
  NULL
};

int lastItem =
-1;

const char *_bad_alloc_Message_9 =
"bad allocation";

static const char *impactTypeNames[15] =
{
  "bullet_small",
  "bullet_large",
  "bullet_ap",
  "bullet_xtreme",
  "shotgun",
  "grenade_bounce",
  "grenade_explode",
  "rifle_grenade",
  "rocket_explode",
  "rocket_explode_xtreme",
  "projectile_dud",
  "mortar_shell",
  "tank_shell",
  "bolt",
  "blade"
};

// *WARNING* One or more selections were skipped as they could not be interpreted as c data

const dvar_t *attachmentFilter;
const dvar_t *maxAttachmentsPerWeapon;
const dvar_t *allItemsUnlocked;
const dvar_t *allItemsPurchased;
const dvar_t *itemSellFactor;
const dvar_t *ui_currentWeaponOptionGroup;
const dvar_t *bodyTypeFromGun;

unlockableItemsData s_unlockableItems;

const char *__cdecl BG_UnlockablesGetCustomClassNameForInt(customClass_t customClassNum)
{
    return BG_UnlockablesGetCustomClassName(customClassNum);
}

const char *__cdecl BG_UnlockablesGetCustomClassName(customClass_t customClassNum)
{
    if ( (unsigned int)customClassNum >= CUSTOM_CLASS_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    271,
                    0,
                    "%s",
                    "customClassNum > CUSTOM_CLASS_INVALID && customClassNum < CUSTOM_CLASS_COUNT") )
    {
        __debugbreak();
    }
    if ( (unsigned int)customClassNum >= CUSTOM_CLASS_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    272,
                    0,
                    "%s",
                    "customClassNum < ARRAY_COUNT( s_customClassNames )") )
    {
        __debugbreak();
    }
    if ( (unsigned int)customClassNum > PRESTIGE_CLASS_5 )
        return 0;
    else
        return s_customClassNames[customClassNum];
}

int __cdecl BG_UnlockablesGetFaceCamoItemIndex(int faceCamoNum)
{
    if ( faceCamoNum >= s_unlockableItems.numFaceCamoPatterns )
    {
        Com_PrintError(
            13,
            "Only %d face camo patterns loaded - asking for camo %d\n",
            s_unlockableItems.numFaceCamoPatterns,
            faceCamoNum);
        faceCamoNum = 0;
    }
    return s_unlockableItems.faceCamoPatterns[faceCamoNum];
}

int __cdecl BG_UnlockablesGetItemGroupFromName(const char *groupName)
{
    return FindEntryInList(groupName, s_itemGroupNames, 24);
}

int __cdecl FindEntryInList(const char *stringEntry, const char **textArray, int numEntries)
{
    int entryNum; // [esp+0h] [ebp-4h]

    if ( !textArray
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp", 306, 0, "%s", "textArray") )
    {
        __debugbreak();
    }
    if ( !numEntries
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    307,
                    0,
                    "%s",
                    "numEntries") )
    {
        __debugbreak();
    }
    if ( !stringEntry
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    308,
                    0,
                    "%s",
                    "stringEntry") )
    {
        __debugbreak();
    }
    if ( !textArray || !stringEntry )
        return -1;
    for ( entryNum = 0; entryNum < numEntries; ++entryNum )
    {
        if ( textArray[entryNum] && !I_stricmp(stringEntry, textArray[entryNum]) )
            return entryNum;
    }
    return -1;
}

const char *__cdecl BG_UnlockablesGetItemGroupName(itemGroup_t groupIndex)
{
    if ( (unsigned int)groupIndex >= ITEMGROUP_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    679,
                    0,
                    "%s",
                    "groupIndex >= 0 && groupIndex < ITEMGROUP_COUNT") )
    {
        __debugbreak();
    }
    if ( (unsigned int)groupIndex >= ITEMGROUP_COUNT )
        return 0;
    else
        return s_itemGroupNames[groupIndex];
}

const char *__cdecl BG_UnlockablesGetLoadoutSlotName(loadoutSlot_t loadoutSlot)
{
    if ( (unsigned int)loadoutSlot > LOADOUTSLOT_DEATHSTREAK
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    689,
                    0,
                    "%s",
                    "loadoutSlot >= LOADOUTSLOT_FIRST && loadoutSlot < LOADOUTSLOT_COUNT") )
    {
        __debugbreak();
    }
    if ( (unsigned int)loadoutSlot > LOADOUTSLOT_DEATHSTREAK )
        return 0;
    else
        return s_loadoutNames[loadoutSlot];
}

bool __cdecl BG_UnlockablesIsItemInDefaultClass(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    return itemInfo && itemInfo->defaultClass[0] != CLASS_NONE;
}

itemInfo_t *__cdecl BG_UnlockablesGetItemInfo(int itemIndex)
{
    const char *v2; // eax

    if ( itemIndex >= 256
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    533,
                    0,
                    "%s",
                    "itemIndex < MAX_UNLOCKABLE_ITEMS") )
    {
        __debugbreak();
    }
    if ( itemIndex >= 256 || itemIndex == -1 )
    {
        Com_PrintError(13, "Invalid unlockable item %d returning NULL\n", itemIndex);
        return 0;
    }
    else
    {
        if ( !s_unlockableItems.itemTable[itemIndex].isValid )
        {
            v2 = va("Invalid unlockable item %d\n", itemIndex);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                            542,
                            0,
                            "%s\n\t%s",
                            "s_unlockableItems.itemTable[ itemIndex ].isValid",
                            v2) )
                __debugbreak();
        }
        if ( s_unlockableItems.itemTable[itemIndex].isValid )
        {
            return &s_unlockableItems.itemTable[itemIndex];
        }
        else
        {
            Com_PrintError(0, "Awoogah! returning NULL for itemindex %i!\n", itemIndex);
            return 0;
        }
    }
}

char __cdecl BG_UnlockablesAllItemsUnlocked()
{
    if ( Com_GameMode_IsPublicOnlineGame() || Com_GameMode_IsGameMode(GAMEMODE_BASIC_TRAINING) )
        return Dvar_GetInt(allItemsUnlocked) > 0;
    if ( Com_GameMode_IsGameMode(GAMEMODE_LOCAL_SPLITSCREEN) || Com_GameMode_IsGameMode(GAMEMODE_SYSTEMLINK) )
        return 1;
    if ( Com_GameMode_IsPrivateOnlineGame() )
        return Dvar_GetInt(ui_items_no_cost) > 0;
    return 0;
}

unsigned int __cdecl BG_UnlockablesGetEquippedClanTagFeatureIndex(int controllerIndex)
{
    const ddlState_t *RootDDLState; // eax
    persistentStats *buffer; // [esp+0h] [ebp-14h]
    ddlState_t searchState; // [esp+4h] [ebp-10h] BYREF

    if ( !LiveStorage_DoWeHaveAllStats(controllerIndex) )
        return 0;
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 2, "clanTagStats", "selectedFeature") )
    {
        buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        return DDL_GetInt(&searchState, (char *)buffer);
    }
    else
    {
        DDL_PrintError("DDL: Error could not find clantag feature index\n");
        return -1;
    }
}

int __cdecl BG_UnlockablesGetClanTagFeatureCount()
{
    return s_unlockableItems.numClanTagFeatures;
}

const char *__cdecl BG_UnlockablesGetClanTagFeature(unsigned int index, clanTagFeatureColumn_t column)
{
    const StringTable *clanTagFeatures; // [esp+4h] [ebp-4h] BYREF

    if ( index >= 0x100 || s_unlockableItems.clanTagFeatureRows[index] == -1 )
        return "";
    if ( !useFastFile->current.enabled )
        return "";
    StringTable_GetAsset("mp/clantagfeatures.csv", (XAssetHeader *)&clanTagFeatures);
    if ( !clanTagFeatures
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    1084,
                    0,
                    "%s",
                    "clanTagFeatures") )
    {
        __debugbreak();
    }
    if ( clanTagFeatures )
        return StringTable_GetColumnValueForRow(clanTagFeatures, s_unlockableItems.clanTagFeatureRows[index], column);
    else
        return "";
}

int __cdecl BG_UnlockablesGetClanTagFeaturePLvlUnlock(unsigned int index)
{
    const char *ClanTagFeature; // eax

    ClanTagFeature = BG_UnlockablesGetClanTagFeature(index, CLANTAG_COL_UNLOCKPLVL);
    return atoi(ClanTagFeature);
}

int __cdecl BG_UnlockablesGetClanTagFeatureLvlUnlock(unsigned int index)
{
    const char *ClanTagFeature; // eax

    ClanTagFeature = BG_UnlockablesGetClanTagFeature(index, CLANTAG_COL_UNLOCKLVL);
    return atoi(ClanTagFeature);
}

bool __cdecl BG_UnlockablesIsClanTagFeatureLockedForRank(int PLevel, int rank, unsigned int index)
{
    int featureRank; // [esp+4h] [ebp-8h]
    int featurePLevel; // [esp+8h] [ebp-4h]

    if ( BG_UnlockablesAllItemsUnlocked() )
        return 0;
    featureRank = BG_UnlockablesGetClanTagFeatureLvlUnlock(index);
    featurePLevel = BG_UnlockablesGetClanTagFeaturePLvlUnlock(index);
    return featureRank > rank || featurePLevel > PLevel;
}

bool __cdecl BG_UnlockablesIsClanTagFeatureLocked(int controllerIndex, unsigned int index)
{
    int Prestige; // eax
    int Rank; // [esp-8h] [ebp-8h]

    Rank = LiveStats_GetRank(controllerIndex);
    Prestige = LiveStats_GetPrestige(controllerIndex);
    return BG_UnlockablesIsClanTagFeatureLockedForRank(Prestige, Rank, index);
}

bool __cdecl BG_UnlockablesIsClanTagFeaturePurchased(int controllerIndex, unsigned int index)
{
    const char *ClanTagFeature; // eax
    const ddlState_t *RootDDLState; // eax
    const char *v5; // [esp-8h] [ebp-24h]
    persistentStats *buffer; // [esp+0h] [ebp-1Ch]
    ddlState_t searchState; // [esp+4h] [ebp-18h] BYREF
    const char *featureName; // [esp+14h] [ebp-8h]

    if ( BG_UnlockablesAllItemsFree() )
        return 1;
    if ( BG_UnlockablesIsClanTagFeatureLocked(controllerIndex, index) )
        return 0;
    ClanTagFeature = BG_UnlockablesGetClanTagFeature(index, CLANTAG_COL_COST);
    if ( !atoi(ClanTagFeature) )
        return 1;
    featureName = BG_UnlockablesGetClanTagFeature(index, CLANTAG_COL_NAME);
    if ( !featureName || !*featureName )
        return 0;
    v5 = featureName;
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 4, "clanTagStats", "features", v5, "purchased") )
    {
        buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        return DDL_GetInt(&searchState, (char *)buffer) != 0;
    }
    else
    {
        DDL_PrintError("DDL: Error could not find clan tag feature %s\n", featureName);
        return 0;
    }
}

char __cdecl BG_UnlockablesAllItemsFree()
{
    if ( Com_GameMode_IsPublicOnlineGame() || Com_GameMode_IsGameMode(GAMEMODE_BASIC_TRAINING) )
        return Dvar_GetInt(allItemsPurchased) > 0;
    if ( Com_GameMode_IsGameMode(GAMEMODE_LOCAL_SPLITSCREEN) || Com_GameMode_IsGameMode(GAMEMODE_SYSTEMLINK) )
        return 1;
    if ( Com_GameMode_IsPrivateOnlineGame() )
        return Dvar_GetInt(ui_items_no_cost) > 0;
    return 0;
}

void __cdecl BG_UnlockablesSetClanTagFeatureNew(int controllerIndex, unsigned int index)
{
    const ddlState_t *RootDDLState; // eax
    const char *v3; // [esp-8h] [ebp-20h]
    persistentStats *buffer; // [esp+0h] [ebp-18h]
    ddlState_t searchState; // [esp+4h] [ebp-14h] BYREF
    const char *featureName; // [esp+14h] [ebp-4h]

    if ( !BG_UnlockablesIsClanTagFeatureLocked(controllerIndex, index) )
    {
        featureName = BG_UnlockablesGetClanTagFeature(index, CLANTAG_COL_NAME);
        if ( featureName )
        {
            if ( *featureName )
            {
                v3 = featureName;
                RootDDLState = LiveStats_GetRootDDLState();
                if ( DDL_MoveTo(RootDDLState, &searchState, 4, "clanTagStats", "features", v3, "new") )
                {
                    buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
                    if ( DDL_SetInt(&searchState, 1u, (char *)buffer) )
                        DDL_PrintError("DDL: Error could not set new flag for clan tag feature %s\n", featureName);
                }
                else
                {
                    DDL_PrintError("DDL: Error could not find clan tag feature %s\n", featureName);
                }
            }
        }
    }
}

void __cdecl BG_UnlockablesPurchaseClanTagFeature(int controllerIndex, unsigned int index)
{
    const char *ClanTagFeature; // eax
    const ddlState_t *RootDDLState; // eax
    const char *v4; // [esp-8h] [ebp-28h]
    persistentStats *buffer; // [esp+0h] [ebp-20h]
    ddlState_t searchState; // [esp+4h] [ebp-1Ch] BYREF
    const char *featureName; // [esp+14h] [ebp-Ch]
    int cost; // [esp+18h] [ebp-8h]
    int currentCodPoints; // [esp+1Ch] [ebp-4h]

    if ( !BG_UnlockablesIsClanTagFeaturePurchased(controllerIndex, index) )
    {
        ClanTagFeature = BG_UnlockablesGetClanTagFeature(index, CLANTAG_COL_COST);
        cost = atoi(ClanTagFeature);
        currentCodPoints = LiveStats_GetCurrency(controllerIndex);
        featureName = BG_UnlockablesGetClanTagFeature(index, CLANTAG_COL_NAME);
        if ( currentCodPoints >= cost )
        {
            if ( featureName && *featureName )
            {
                v4 = featureName;
                RootDDLState = LiveStats_GetRootDDLState();
                if ( DDL_MoveTo(RootDDLState, &searchState, 4, "clanTagStats", "features", v4, "purchased") )
                {
                    buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
                    if ( DDL_SetInt(&searchState, 1u, (char *)buffer) )
                    {
                        currentCodPoints -= cost;
                        LiveStats_SpendCurrency(controllerIndex, cost, POINTSSPENT_UNLOCKABLE_CLANTAG, index);
                    }
                    else
                    {
                        DDL_PrintError("DDL: Error could not set purchased flag for clan tag feature %s\n", featureName);
                    }
                }
                else
                {
                    DDL_PrintError("DDL: Error could not find clan tag feature %s\n", featureName);
                }
            }
        }
        else
        {
            Com_PrintError(15, "Item %s costs %d and you only have %d\n", featureName, cost, currentCodPoints);
        }
    }
}

int __cdecl BG_UnlockablesGetProfessionalIndex()
{
    return s_unlockableItems.professionalPerkIndex;
}

void __cdecl BG_InitUnlockables()
{
    int numUnlockables; // [esp+0h] [ebp-4h]

    memset(&s_unlockableItems, 0, sizeof(s_unlockableItems));
    for ( numUnlockables = 0; numUnlockables < 256; ++numUnlockables )
    {
        if ( BG_UnlockablesBuildItemInfo(numUnlockables, &s_unlockableItems.itemTable[numUnlockables]) )
        {
            ++s_unlockableItems.numItems;
            s_unlockableItems.maxItem = numUnlockables;
        }
    }
    BG_UnlockablesBuildDefaultClasses();
    BG_UnlockablesBuildDefaultItems();
    BG_UnlockablesBuildActualDefaultItems();
    BG_UnlockablesBuildClanTagFeatureRows();
    BG_UnlockablesSetProfessionalIndex();
}

void BG_UnlockablesBuildDefaultClasses()
{
    const char *v0; // eax
    const char *v1; // eax
    int killStreakCount; // [esp+0h] [ebp-38h]
    int currentEntry; // [esp+4h] [ebp-34h]
    const char *loadoutName; // [esp+8h] [ebp-30h]
    defaultClass_t defaultClass; // [esp+Ch] [ebp-2Ch]
    int i; // [esp+10h] [ebp-28h]
    itemInfo_t *itemInfo; // [esp+14h] [ebp-24h]
    int itemNumber; // [esp+18h] [ebp-20h]
    int loadoutSlot; // [esp+1Ch] [ebp-1Ch]
    int defaultClassNum; // [esp+20h] [ebp-18h]
    signed int defaultGlobalItemNum; // [esp+24h] [ebp-14h]
    itemInfo_t *killStreakItems[4]; // [esp+28h] [ebp-10h] BYREF

    for ( defaultClassNum = 0; defaultClassNum < 20; ++defaultClassNum )
    {
        for ( loadoutSlot = 0; loadoutSlot < 35; ++loadoutSlot )
            s_unlockableItems.defaultClassLoadouts[defaultClassNum][loadoutSlot] = -1;
    }
    defaultGlobalItemNum = 0;
    for ( itemNumber = 0; itemNumber < 256; ++itemNumber )
    {
        itemInfo = &s_unlockableItems.itemTable[itemNumber];
        if ( itemInfo->isValid )
        {
            if ( itemInfo->group < ITEMGROUP_GLOBAL_ITEMS_START )
            {
                for ( i = 0; i < 20; ++i )
                {
                    defaultClass = itemInfo->defaultClass[i];
                    if ( defaultClass != CLASS_NONE && itemInfo->loadoutSlot != LOADOUTSLOT_INVALID )
                    {
                        loadoutName = BG_UnlockablesGetLoadoutName(itemInfo->loadoutSlot);
                        if ( loadoutName )
                        {
                            currentEntry = s_unlockableItems.defaultClassLoadouts[defaultClass][itemInfo->loadoutSlot];
                            if ( currentEntry != -1 )
                            {
                                v1 = va(
                                             "Default Class %s already has equipment in slot %s has no room for %s",
                                             s_classNames[defaultClass],
                                             loadoutName,
                                             itemInfo->name);
                                if ( !Assert_MyHandler(
                                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                                                404,
                                                0,
                                                "%s\n\t%s",
                                                "currentEntry == ENTRY_NOT_FOUND",
                                                v1) )
                                    __debugbreak();
                            }
                            if ( currentEntry == -1 )
                                s_unlockableItems.defaultClassLoadouts[defaultClass][itemInfo->loadoutSlot] = itemInfo->index;
                        }
                    }
                }
            }
            else if ( itemInfo->isDefaultGlobalItem )
            {
                if ( defaultGlobalItemNum >= 4 )
                {
                    v0 = va("Too many default global items at %s", itemInfo->name);
                    if ( !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                                    376,
                                    0,
                                    "%s\n\t%s",
                                    "defaultGlobalItemNum < LOADOUTSLOT_COUNT - LOADOUTSLOT_GLOBAL_ITEMS_START",
                                    v0) )
                        __debugbreak();
                }
                if ( defaultGlobalItemNum < 4 )
                    killStreakItems[defaultGlobalItemNum++] = itemInfo;
            }
        }
    }
    if ( defaultGlobalItemNum )
    {
        qsort(killStreakItems, defaultGlobalItemNum, 4u, BG_UnlockablesCompareItemsBySortKey);
        for ( killStreakCount = 0; killStreakCount < defaultGlobalItemNum; ++killStreakCount )
            s_unlockableItems.defaultGlobalItems[killStreakCount] = killStreakItems[killStreakCount]->index;
    }
}

const char *__cdecl BG_UnlockablesGetLoadoutName(loadoutSlot_t loadoutSlot)
{
    if ( (unsigned int)loadoutSlot > LOADOUTSLOT_DEATHSTREAK
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    291,
                    0,
                    "%s",
                    "loadoutSlot > LOADOUTSLOT_INVALID && loadoutSlot < LOADOUTSLOT_COUNT") )
    {
        __debugbreak();
    }
    if ( (unsigned int)loadoutSlot >= (LOADOUTSLOT_EQUIPMENT|LOADOUTSLOT_PRIMARY_RETICLE_COLOR)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    292,
                    0,
                    "%s",
                    "loadoutSlot < ARRAY_COUNT( s_loadoutNames )") )
    {
        __debugbreak();
    }
    if ( (unsigned int)loadoutSlot > LOADOUTSLOT_DEATHSTREAK )
        return 0;
    else
        return s_loadoutNames[loadoutSlot];
}

int __cdecl BG_UnlockablesCompareItemsBySortKey(const void *arg0, const void *arg1)
{
    int returnValue; // [esp+0h] [ebp-Ch]

    returnValue = *(unsigned int *)(*(unsigned int *)arg0 + 264) - *(unsigned int *)(*(unsigned int *)arg1 + 264);
    if ( !returnValue )
        return **(unsigned int **)arg0 - **(unsigned int **)arg1;
    return returnValue;
}

char __cdecl BG_UnlockablesBuildItemInfo(int itemIndex, itemInfo_t *itemInfo)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *ColumnValueForRow; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    const char *v13; // eax
    const char *v14; // eax
    const char *v15; // eax
    const char *v16; // eax
    const char *name; // [esp+24h] [ebp-10h]
    const StringTable *statsTable; // [esp+28h] [ebp-Ch] BYREF
    int row; // [esp+2Ch] [ebp-8h]
    const char *reference; // [esp+30h] [ebp-4h]

    if ( !itemInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp", 757, 0, "%s", "itemInfo") )
    {
        __debugbreak();
    }
    if ( !useFastFile->current.enabled )
        return 0;
    if ( !itemInfo )
        return 0;
    if ( itemInfo->isValid )
    {
        v3 = va("Item number %d already contains an item %s\n", itemIndex, itemInfo->name);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                        769,
                        0,
                        "%s\n\t%s",
                        "!itemInfo->isValid",
                        v3) )
            __debugbreak();
    }
    if ( itemInfo->isValid )
        return 0;
    StringTable_GetAsset("mp/statstable.csv", (XAssetHeader *)&statsTable);
    if ( !statsTable
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    778,
                    0,
                    "%s",
                    "statsTable") )
    {
        __debugbreak();
    }
    if ( !statsTable )
        return 0;
    if ( itemIndex >= 256
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    785,
                    0,
                    "%s",
                    "itemIndex < MAX_UNLOCKABLE_ITEMS") )
    {
        __debugbreak();
    }
    if ( itemIndex >= 256 )
        return 0;
    v4 = va("%d", itemIndex);
    row = StringTable_LookupRowNumForValue(statsTable, 0, v4);
    if ( row == -1 )
        return 0;
    memset(itemInfo, 0, sizeof(itemInfo_t));
    name = StringTable_GetColumnValueForRow(statsTable, row, 3);
    reference = StringTable_GetColumnValueForRow(statsTable, row, 4);
    if ( name && reference && strlen(name) && strlen(reference) )
    {
        itemInfo->name = name;
        itemInfo->reference = reference;
        if ( I_stristr(reference, "null") )
            itemInfo->isNullItem = 1;
        itemInfo->index = itemIndex;
        ColumnValueForRow = StringTable_GetColumnValueForRow(statsTable, row, 1);
        itemInfo->isPassive = I_stricmp(ColumnValueForRow, "passive") == 0;
        v6 = StringTable_GetColumnValueForRow(statsTable, row, 2);
        itemInfo->group = (itemGroup_t)BG_UnlockablesGetItemGroupFromName(v6);
        v7 = StringTable_GetColumnValueForRow(statsTable, row, 5);
        itemInfo->count = atoi(v7);
        itemInfo->imageRef = StringTable_GetColumnValueForRow(statsTable, row, 6);
        itemInfo->desc = StringTable_GetColumnValueForRow(statsTable, row, 7);
        v8 = StringTable_GetColumnValueForRow(statsTable, row, 9);
        itemInfo->sortKey = atoi(v8);
        v9 = StringTable_GetColumnValueForRow(statsTable, row, 10);
        itemInfo->unlockLevel = atoi(v9);
        v10 = StringTable_GetColumnValueForRow(statsTable, row, 14);
        itemInfo->unlockPLevel = atoi(v10);
        v11 = StringTable_GetColumnValueForRow(statsTable, row, 8);
        BG_UnlockablesParseAttachmentsForItem(itemInfo, v11);
        if ( itemInfo->group >= ITEMGROUP_GLOBAL_ITEMS_START )
        {
            v13 = StringTable_GetColumnValueForRow(statsTable, row, 11);
            itemInfo->isDefaultGlobalItem = I_stricmp(v13, "all") == 0;
        }
        else
        {
            v12 = StringTable_GetColumnValueForRow(statsTable, row, 11);
            BG_UnlockablesParseDefaultClassesForItem(itemInfo, v12);
            itemInfo->isDefaultGlobalItem = 0;
        }
        v14 = StringTable_GetColumnValueForRow(statsTable, row, 12);
        itemInfo->cost = atoi(v14);
        v15 = StringTable_GetColumnValueForRow(statsTable, row, 13);
        itemInfo->loadoutSlot = BG_UnlockablesGetLoadoutSlotFromString(v15);
        v16 = StringTable_GetColumnValueForRow(statsTable, row, 15);
        itemInfo->unclassifyAt = atoi(v16);
        itemInfo->nameHash = StringTable_HashString(itemInfo->name);
        itemInfo->refHash = StringTable_HashString(itemInfo->reference);
        itemInfo->isValid = 1;
        BG_UnlockablesParseDualWieldInfo(itemInfo);
        BG_UnlockablesParseFaceCamoInfo(itemInfo);
        BG_UnlockablesParsePerkProChallenges(itemInfo);
    }
    return 1;
}

int __cdecl BG_UnlockablesParseDefaultClassesForItem(itemInfo_t *itemInfo, const char *defaultClassList)
{
    const char *v2; // eax
    defaultClass_t EntryInList; // [esp+0h] [ebp-14h]
    parseInfo_t *defaultClassName; // [esp+4h] [ebp-10h]
    int defaultClassCount; // [esp+8h] [ebp-Ch]
    const char *parseLocation; // [esp+Ch] [ebp-8h] BYREF
    int defaultClassNum; // [esp+10h] [ebp-4h]

    if ( !itemInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp", 433, 0, "%s", "itemInfo") )
    {
        __debugbreak();
    }
    if ( !defaultClassList
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    434,
                    0,
                    "%s",
                    "defaultClassList") )
    {
        __debugbreak();
    }
    for ( defaultClassNum = 0; defaultClassNum < 20; ++defaultClassNum )
        itemInfo->defaultClass[defaultClassNum] = CLASS_NONE;
    if ( itemInfo && defaultClassList )
    {
        defaultClassCount = 0;
        Com_BeginParseSession("defaultClasses");
        Com_SetSpaceDelimited(0);
        parseLocation = defaultClassList;
        while ( 1 )
        {
            defaultClassName = Com_ParseOnLine(&parseLocation);
            if ( !parseLocation || !defaultClassName )
                break;
            EntryInList = (defaultClass_t)FindEntryInList(defaultClassName->token, s_classNames, 20);
            if ( EntryInList == CLASS_NONE )
            {
                v2 = va("Default Class %s not found", defaultClassName->token);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                                463,
                                0,
                                "%s\n\t%s",
                                "defaultClassNum != ENTRY_NOT_FOUND",
                                v2) )
                    __debugbreak();
            }
            if ( EntryInList != CLASS_NONE )
                itemInfo->defaultClass[defaultClassCount++] = EntryInList;
        }
        Com_EndParseSession();
    }
    return -1;
}

void __cdecl BG_UnlockablesParseAttachmentsForItem(itemInfo_t *itemInfo, const char *attachmentList)
{
    const char **AttachmentNames; // eax
    const char *v3; // eax
    int *v4; // [esp+0h] [ebp-1Ch]
    eAttachment EntryInList; // [esp+8h] [ebp-14h]
    parseInfo_t *attachmentName; // [esp+Ch] [ebp-10h]
    const char *parseLocation; // [esp+10h] [ebp-Ch] BYREF
    int attachPoint; // [esp+14h] [ebp-8h]
    int attachmentNum; // [esp+18h] [ebp-4h]

    if ( !itemInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp", 481, 0, "%s", "itemInfo") )
    {
        __debugbreak();
    }
    if ( !attachmentList
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    482,
                    0,
                    "%s",
                    "attachmentList") )
    {
        __debugbreak();
    }
    for ( attachmentNum = 0; attachmentNum < 24; ++attachmentNum )
        itemInfo->attachments[attachmentNum] = ATTACHMENT_NONE;
    for ( attachPoint = 0; attachPoint < 5; ++attachPoint )
        itemInfo->numAttachments[attachPoint] = 1;
    if ( itemInfo && attachmentList )
    {
        Com_BeginParseSession("attachments");
        Com_SetSpaceDelimited(0);
        parseLocation = attachmentList;
        while ( 1 )
        {
            attachmentName = Com_ParseOnLine(&parseLocation);
            if ( !parseLocation || !attachmentName )
                break;
            AttachmentNames = BG_GetAttachmentNames();
            EntryInList = (eAttachment)FindEntryInList(attachmentName->token, AttachmentNames, 24);
            if ( EntryInList == -1 )
            {
                v3 = va("Attachment %s not found", attachmentName->token);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                                516,
                                0,
                                "%s\n\t%s",
                                "attachmentNum != ENTRY_NOT_FOUND",
                                v3) )
                    __debugbreak();
            }
            if ( (unsigned int)EntryInList >= ATTACHMENT_COUNT
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                            517,
                            0,
                            "attachmentNum doesn't index ATTACHMENT_COUNT\n\t%i not in [0, %i)",
                            EntryInList,
                            24) )
            {
                __debugbreak();
            }
            if ( EntryInList != -1 )
            {
                itemInfo->attachments[itemInfo->numAttachments[0]++] = EntryInList;
                v4 = &itemInfo->numAttachments[BG_GetAttachmentPointIndexFromAttachment(EntryInList)];
                ++*v4;
            }
        }
        Com_EndParseSession();
    }
}

void __cdecl BG_UnlockablesParseDualWieldInfo(itemInfo_t *itemInfo)
{
    itemInfo_t *originalItem; // [esp+14h] [ebp-30h]
    int originalItemIndex; // [esp+18h] [ebp-2Ch]
    char originalItemName[32]; // [esp+1Ch] [ebp-28h] BYREF
    int nameLength; // [esp+40h] [ebp-4h]

    if ( !itemInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp", 602, 0, "%s", "itemInfo") )
    {
        __debugbreak();
    }
    if ( itemInfo )
    {
        if ( I_stricmpwild("*_DW", itemInfo->name) )
        {
            itemInfo->dwIndex = -1;
            itemInfo->dwBaseIndex = -1;
            itemInfo->dwCost = -1;
        }
        else
        {
            nameLength = strlen(itemInfo->name);
            I_strncpyz(originalItemName, itemInfo->name, nameLength - 2);
            originalItemIndex = BG_UnlockablesGetItemIndexFromName(originalItemName);
            originalItem = BG_UnlockablesGetItemInfo(originalItemIndex);
            itemInfo->dwIndex = -1;
            itemInfo->dwBaseIndex = -1;
            itemInfo->dwCost = -1;
            if ( originalItem )
            {
                originalItem->dwIndex = itemInfo->index;
                originalItem->dwCost = itemInfo->cost;
                itemInfo->dwBaseIndex = originalItem->index;
            }
            else
            {
                Com_PrintWarning(1, "WARNING: Dual Wield ONLY weapon: %s was added to the unlockable items.\n", itemInfo->name);
            }
            itemInfo->cost = -1;
        }
    }
}

void __cdecl BG_UnlockablesParseFaceCamoInfo(itemInfo_t *itemInfo)
{
    if ( !itemInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp", 645, 0, "%s", "itemInfo") )
    {
        __debugbreak();
    }
    if ( itemInfo && itemInfo->loadoutSlot == LOADOUTSLOT_FACEPAINT_PATTERN )
    {
        if ( s_unlockableItems.numFaceCamoPatterns >= 64 )
            Com_PrintError(13, "Only %d face camo patterns supported cannot add %s\n", 64, itemInfo->name);
        else
            s_unlockableItems.faceCamoPatterns[s_unlockableItems.numFaceCamoPatterns++] = itemInfo->index;
    }
    else if ( itemInfo && itemInfo->loadoutSlot == LOADOUTSLOT_FACEPAINT_COLOR )
    {
        if ( s_unlockableItems.numFaceCamoColors >= 64 )
            Com_PrintError(13, "Only %d face camo colors supported cannot add %s\n", 64, itemInfo->name);
        else
            s_unlockableItems.faceCamoColors[s_unlockableItems.numFaceCamoColors++] = itemInfo->index;
    }
}

void __cdecl BG_UnlockablesParsePerkProChallenges(itemInfo_t *itemInfo)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *perkName; // [esp+10h] [ebp-14h]
    int row; // [esp+14h] [ebp-10h]
    int milestone; // [esp+18h] [ebp-Ch]
    const StringTable *statsMilestoneTable; // [esp+1Ch] [ebp-8h] BYREF
    int currentProChallenge; // [esp+20h] [ebp-4h]

    if ( !itemInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp", 701, 0, "%s", "itemInfo") )
    {
        __debugbreak();
    }
    if ( itemInfo
        && itemInfo->group == ITEMGROUP_SPECIALTY
        && itemInfo->name
        && *itemInfo->name
        && !BG_UnlockablesIsProItem(itemInfo->index)
        && BG_UnlockablesIsItemValidNotNull(itemInfo->index) )
    {
        statsMilestoneTable = 0;
        currentProChallenge = 0;
        for ( milestone = 0; milestone < 1000; ++milestone )
        {
            v1 = va("%d", milestone);
            row = CL_GetMilestoneRowNum((XAssetHeader *)&statsMilestoneTable, 0, v1, -1, 0, -1, 0);
            if ( statsMilestoneTable )
            {
                if ( row != -1 )
                {
                    perkName = StringTable_GetColumnValueForRow(statsMilestoneTable, row, 10);
                    if ( strlen(perkName) )
                    {
                        if ( !I_stricmp(itemInfo->name, perkName) )
                        {
                            itemInfo->challengeIndices[currentProChallenge++] = milestone;
                            if ( currentProChallenge == 3 )
                                return;
                        }
                    }
                }
            }
        }
        v2 = va(
                     "Not enough challenges for perkPro for perk %s only %d challenges found",
                     itemInfo->name,
                     currentProChallenge);
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp", 738, 0, v2) )
            __debugbreak();
    }
}

void BG_UnlockablesBuildActualDefaultItems()
{
    const char *loadoutName; // [esp+4h] [ebp-10h]
    int defaultEquipment; // [esp+8h] [ebp-Ch]
    loadoutSlot_t loadoutSlot; // [esp+Ch] [ebp-8h]

    for ( loadoutSlot = LOADOUTSLOT_FIRST; loadoutSlot < LOADOUTSLOT_COUNT; ++loadoutSlot )
    {
        defaultEquipment = BG_UnlockablesGetItemIndexFromName("weapon_null");
        loadoutName = BG_UnlockablesGetLoadoutName(loadoutSlot);
        if ( loadoutName )
        {
            switch ( loadoutSlot )
            {
                case LOADOUTSLOT_FIRST:
                case LOADOUTSLOT_PRIMARY_ATTACHMENT:
                case LOADOUTSLOT_PRIMARY_CAMO:
                case LOADOUTSLOT_SECONDARY_WEAPON:
                case LOADOUTSLOT_SECONDARY_ATTACHMENT:
                case LOADOUTSLOT_SECONDARY_CAMO:
                case LOADOUTSLOT_PRIMARY_GRENADE:
                case LOADOUTSLOT_SPECIAL_GRENADE:
                    defaultEquipment = s_unlockableItems.defaultClassLoadouts[0][loadoutSlot];
                    break;
                case LOADOUTSLOT_FIRST_SPECIALTY:
                case LOADOUTSLOT_SPECIALTY2:
                case LOADOUTSLOT_SPECIALTY3:
                case LOADOUTSLOT_CLASSBONUS:
                case LOADOUTSLOT_DEATHSTREAK:
                    defaultEquipment = BG_UnlockablesGetItemIndexFromName("perks_none");
                    break;
                case LOADOUTSLOT_HEAD:
                    defaultEquipment = BG_UnlockablesGetItemIndexFromName("armor_null");
                    break;
                case LOADOUTSLOT_BODY:
                    defaultEquipment = s_unlockableItems.defaultClassLoadouts[0][loadoutSlot];
                    break;
                case LOADOUTSLOT_EQUIPMENT:
                    break;
                case LOADOUTSLOT_GLOBAL_ITEMS_START:
                case LOADOUTSLOT_KILLSTREAK2:
                case LOADOUTSLOT_KILLSTREAK3:
                    defaultEquipment = BG_UnlockablesGetItemIndexFromName("killstreak_null");
                    break;
                default:
                    Com_Printf(
                        13,
                        "No default item defined for loadout slot '%s'. Returning default item: '%s'\n",
                        loadoutName,
                        "weapon_null");
                    break;
            }
            s_unlockableItems.actualDefaultItems[loadoutSlot] = defaultEquipment;
        }
    }
}

void BG_UnlockablesBuildDefaultItems()
{
    const char *loadoutName; // [esp+4h] [ebp-10h]
    int defaultEquipment; // [esp+8h] [ebp-Ch]
    loadoutSlot_t loadoutSlot; // [esp+Ch] [ebp-8h]

    for ( loadoutSlot = LOADOUTSLOT_FIRST; loadoutSlot < LOADOUTSLOT_COUNT; ++loadoutSlot )
    {
        defaultEquipment = BG_UnlockablesGetItemIndexFromName("weapon_null");
        loadoutName = BG_UnlockablesGetLoadoutName(loadoutSlot);
        if ( loadoutName )
        {
            switch ( loadoutSlot )
            {
                case LOADOUTSLOT_FIRST:
                case LOADOUTSLOT_PRIMARY_ATTACHMENT:
                case LOADOUTSLOT_PRIMARY_CAMO:
                case LOADOUTSLOT_SECONDARY_WEAPON:
                case LOADOUTSLOT_SECONDARY_ATTACHMENT:
                case LOADOUTSLOT_SECONDARY_CAMO:
                case LOADOUTSLOT_PRIMARY_GRENADE:
                case LOADOUTSLOT_SPECIAL_GRENADE:
                case LOADOUTSLOT_EQUIPMENT:
                    break;
                case LOADOUTSLOT_FIRST_SPECIALTY:
                case LOADOUTSLOT_SPECIALTY2:
                case LOADOUTSLOT_SPECIALTY3:
                case LOADOUTSLOT_CLASSBONUS:
                case LOADOUTSLOT_DEATHSTREAK:
                    defaultEquipment = BG_UnlockablesGetItemIndexFromName("perks_none");
                    break;
                case LOADOUTSLOT_HEAD:
                case LOADOUTSLOT_BODY:
                    defaultEquipment = BG_UnlockablesGetItemIndexFromName("armor_null");
                    break;
                case LOADOUTSLOT_GLOBAL_ITEMS_START:
                case LOADOUTSLOT_KILLSTREAK2:
                case LOADOUTSLOT_KILLSTREAK3:
                    defaultEquipment = BG_UnlockablesGetItemIndexFromName("killstreak_null");
                    break;
                default:
                    Com_Printf(
                        13,
                        "No default item defined for loadout slot '%s'. Returning default item: '%s'\n",
                        loadoutName,
                        "weapon_null");
                    break;
            }
            s_unlockableItems.defaultItems[loadoutSlot] = defaultEquipment;
        }
    }
}

void BG_UnlockablesBuildClanTagFeatureRows()
{
    const char *v0; // eax
    int numFeatures; // [esp+4h] [ebp-8h]
    const StringTable *clanTagFeatures; // [esp+8h] [ebp-4h] BYREF

    if ( useFastFile->current.enabled )
    {
        StringTable_GetAsset("mp/clantagfeatures.csv", (XAssetHeader *)&clanTagFeatures);
        if ( !clanTagFeatures
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                        1028,
                        0,
                        "%s",
                        "clanTagFeatures") )
        {
            __debugbreak();
        }
        if ( clanTagFeatures )
        {
            for ( numFeatures = 0; numFeatures < 256; ++numFeatures )
            {
                v0 = va("%d", numFeatures);
                s_unlockableItems.clanTagFeatureRows[numFeatures] = StringTable_LookupRowNumForValue(clanTagFeatures, 0, v0);
                if ( s_unlockableItems.clanTagFeatureRows[numFeatures] != -1 )
                    s_unlockableItems.numClanTagFeatures = numFeatures;
            }
        }
    }
}

int BG_UnlockablesSetProfessionalIndex()
{
    int result; // eax

    result = BG_UnlockablesGetItemIndexFromName("PERKS_PROFESSIONAL");
    s_unlockableItems.professionalPerkIndex = result;
    return result;
}

int __cdecl BG_UnlockablesGetItemAttachmentNumWithAttachPoint(int itemIndex, int attachmentNum, int attachmentPoint)
{
    int currentAttachment; // [esp+0h] [ebp-8h]
    itemInfo_t *itemInfo; // [esp+4h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( !itemInfo || !attachmentNum )
        return 0;
    if ( !attachmentPoint && attachmentNum < itemInfo->numAttachments[0] )
        return attachmentNum;
    for ( currentAttachment = 1; currentAttachment < itemInfo->numAttachments[0]; ++currentAttachment )
    {
        if ( BG_GetAttachmentPointIndexFromAttachment(itemInfo->attachments[currentAttachment]) == attachmentPoint
            && --attachmentNum <= 0 )
        {
            return currentAttachment;
        }
    }
    return 0;
}

eAttachment __cdecl BG_UnlockablesGetItemAttachment(int itemIndex, int attachmentNum)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( !itemInfo || !attachmentNum )
        return ATTACHMENT_NONE;
    if ( attachmentNum < itemInfo->numAttachments[0] )
        return itemInfo->attachments[attachmentNum];
    Com_PrintWarning(
        17,
        "Item %s does not have attachment number %d it only has %d attachments\n",
        itemInfo->name,
        attachmentNum,
        itemInfo->numAttachments[0]);
    return ATTACHMENT_NONE;
}

int __cdecl BG_UnlockablesGetItemAttachmentIndexForAttachmentPoint(
                int itemIndex,
                eAttachment attachment,
                eAttachmentPoint point)
{
    int index; // [esp+0h] [ebp-Ch]
    itemInfo_t *itemInfo; // [esp+4h] [ebp-8h]
    int count; // [esp+8h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( !itemInfo || attachment == ATTACHMENT_NONE )
        return 0;
    count = 0;
    for ( index = 0; index < itemInfo->numAttachments[0]; ++index )
    {
        if ( point == BG_GetAttachmentPointIndexFromAttachment(itemInfo->attachments[index]) )
            ++count;
        if ( itemInfo->attachments[index] == attachment )
            return count;
    }
    return 0;
}

const char *__cdecl BG_UnlockablesGetItemName(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo )
        return itemInfo->name;
    else
        return 0;
}

const char *__cdecl BG_UnlockablesGetItemRef(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo )
        return itemInfo->reference;
    else
        return 0;
}

int __cdecl BG_UnlockablesGetItemCount(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo )
        return itemInfo->count;
    else
        return -1;
}

const char *__cdecl BG_UnlockablesGetItemImage(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo )
        return itemInfo->imageRef;
    else
        return 0;
}

int __cdecl BG_UnlockablesGetItemUnlockLevel(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo )
        return itemInfo->unlockLevel;
    else
        return 0;
}

int __cdecl BG_UnlockablesGetItemUnlockPLevel(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo )
        return itemInfo->unlockPLevel;
    else
        return 0;
}

int __cdecl BG_UnlockablesGetItemCost(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo && !BG_UnlockablesAllItemsFree() )
        return itemInfo->cost;
    else
        return 0;
}

int __cdecl BG_UnlockablesGetItemDualWieldCost(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo && !BG_UnlockablesAllItemsFree() )
        return itemInfo->dwCost;
    else
        return 0;
}

int __cdecl BG_UnlockablesGetItemDualWieldIndex(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo )
        return itemInfo->dwIndex;
    else
        return -1;
}

int __cdecl BG_UnlockablesGetItemDualWieldBaseIndex(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo )
        return itemInfo->dwBaseIndex;
    else
        return -1;
}

int __cdecl BG_UnlockablesGetItemSellPrice(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( !itemInfo )
        return 0;
    if ( itemSellFactor && itemSellFactor->current.enabled )
        return (int)(float)((float)itemInfo->cost * itemSellFactor->current.value);
    return 0;
}

const char *__cdecl BG_UnlockablesGetItemDesc(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo )
        return itemInfo->desc;
    else
        return 0;
}

int __cdecl BG_UnlockablesGetNumItemAttachmentsWithAttachPoint(int itemIndex, int attachPoint)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo )
        return itemInfo->numAttachments[attachPoint];
    else
        return 0;
}

int __cdecl BG_UnlockablesGetNumItemAttachments(int itemIndex)
{
    return BG_UnlockablesGetNumItemAttachmentsWithAttachPoint(itemIndex, 0);
}

int __cdecl BG_UnlockablesGetNumItemAttachmentsAtPoint(int itemIndex, eAttachmentPoint point)
{
    int attachment; // [esp+0h] [ebp-Ch]
    int count; // [esp+4h] [ebp-8h]
    itemInfo_t *itemInfo; // [esp+8h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( !itemInfo )
        return 0;
    count = 0;
    for ( attachment = 0; attachment < itemInfo->numAttachments[0]; ++attachment )
    {
        if ( BG_GetAttachmentPointIndexFromAttachment(itemInfo->attachments[attachment]) == point )
            ++count;
    }
    return count;
}

eAttachment __cdecl BG_UnlockablesGetItemAttachmentAtPointByIndex(
                int itemIndex,
                eAttachmentPoint point,
                int attachmentIndex)
{
    int attachment; // [esp+0h] [ebp-Ch]
    int count; // [esp+4h] [ebp-8h]
    itemInfo_t *itemInfo; // [esp+8h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo )
    {
        count = -1;
        for ( attachment = 0; attachment < itemInfo->numAttachments[0]; ++attachment )
        {
            if ( BG_GetAttachmentPointIndexFromAttachment(itemInfo->attachments[attachment]) == point )
                ++count;
            if ( count == attachmentIndex )
                return itemInfo->attachments[attachment];
        }
    }
    return ATTACHMENT_NONE;
}

const char *__cdecl BG_UnlockablesGetItemGroup(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo )
        return BG_UnlockablesGetItemGroupName(itemInfo->group);
    else
        return 0;
}

int __cdecl BG_UnlockablesGetItemGroupEnum(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo )
        return itemInfo->group;
    else
        return -1;
}

loadoutSlot_t __cdecl BG_UnlockablesGetLoadoutSlotFromString(const char *slotName)
{
    if ( !slotName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp", 1615, 0, "%s", "slotName") )
    {
        __debugbreak();
    }
    if ( slotName && *slotName )
        return (loadoutSlot_t)FindEntryInList(slotName, s_loadoutNames, 39);
    else
        return LOADOUTSLOT_INVALID;
}

int __cdecl BG_UnlockablesGetItemLoadoutSlot(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo )
        return itemInfo->loadoutSlot;
    else
        return -1;
}

bool __cdecl BG_UnlockablesIsItemLockedForRank(int pLevel, int rank, int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+4h] [ebp-4h]

    if ( BG_UnlockablesAllItemsUnlocked() )
        return 0;
    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( !itemInfo )
        return 1;
    return itemInfo->unlockLevel > rank || itemInfo->unlockPLevel > pLevel;
}

bool __cdecl BG_UnlockablesIsItemLocked(int controllerIndex, int itemIndex)
{
    int Prestige; // eax
    int Rank; // [esp-8h] [ebp-8h]

    Rank = LiveStats_GetRank(controllerIndex);
    Prestige = LiveStats_GetPrestige(controllerIndex);
    return BG_UnlockablesIsItemLockedForRank(Prestige, Rank, itemIndex);
}

bool __cdecl BG_UnlockablesIsItemDualWieldLocked(int controllerIndex, int itemIndex)
{
    int Prestige; // eax
    int Rank; // [esp-8h] [ebp-Ch]
    int dwIndex; // [esp-4h] [ebp-8h]
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( !itemInfo )
        return 1;
    if ( itemInfo->dwIndex == -1 )
        return 1;
    dwIndex = itemInfo->dwIndex;
    Rank = LiveStats_GetRank(controllerIndex);
    Prestige = LiveStats_GetPrestige(controllerIndex);
    return BG_UnlockablesIsItemLockedForRank(Prestige, Rank, dwIndex);
}

bool __cdecl BG_UnlockablesIsItemPassive(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    return itemInfo && itemInfo->isPassive;
}

bool __cdecl BG_UnlockablesItemHasDualWield(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    return itemInfo && itemInfo->dwIndex != -1;
}

bool __cdecl BG_UnlockablesItemIsDualWield(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    return itemInfo && itemInfo->dwBaseIndex != -1;
}

bool __cdecl BG_UnlockablesIsItemValidNotNull(int itemIndex)
{
    return s_unlockableItems.itemTable[itemIndex].isValid && !s_unlockableItems.itemTable[itemIndex].isNullItem;
}

void __cdecl BG_UnlockablesSetItemNew(int controllerIndex, int itemIndex)
{
    const ddlState_t *RootDDLState; // eax
    char *v3; // [esp-8h] [ebp-20h]
    persistentStats *buffer; // [esp+0h] [ebp-18h]
    ddlState_t searchState; // [esp+8h] [ebp-10h] BYREF

    if ( !BG_UnlockablesIsItemLocked(controllerIndex, itemIndex) && BG_UnlockablesGetItemInfo(itemIndex) )
    {
        v3 = va("%d", itemIndex);
        RootDDLState = LiveStats_GetRootDDLState();
        if ( DDL_MoveTo(RootDDLState, &searchState, 3, "itemStats", v3) )
        {
            buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
            DDL_SetInt(&searchState, 1u, (char *)buffer);
        }
        else
        {
            DDL_PrintError("DDL: Error could not find item %d\n", itemIndex);
        }
    }
}

void __cdecl SV_SetItemNewForClient(char *clientBuffer, int itemIndex, int clientRank, int clientPLevel)
{
    const ddlState_t *RootDDLState; // eax
    char *v5; // [esp-8h] [ebp-1Ch]
    ddlState_t searchState; // [esp+4h] [ebp-10h] BYREF

    if ( !clientBuffer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    1759,
                    0,
                    "%s",
                    "clientBuffer") )
    {
        __debugbreak();
    }
    if ( !BG_UnlockablesIsItemLockedForRank(clientPLevel, clientRank, itemIndex) && BG_UnlockablesGetItemInfo(itemIndex) )
    {
        v5 = va("%d", itemIndex);
        RootDDLState = LiveStats_GetRootDDLState();
        if ( DDL_MoveTo(RootDDLState, &searchState, 3, "itemStats", v5) )
            DDL_SetInt(&searchState, 1u, clientBuffer);
        else
            DDL_PrintError("DDL: Error could not find item %d\n", itemIndex);
    }
}

bool __cdecl BG_UnlockablesIsItemNew(int controllerIndex, int itemIndex)
{
    const ddlState_t *RootDDLState; // eax
    char *v4; // [esp-8h] [ebp-20h]
    persistentStats *buffer; // [esp+0h] [ebp-18h]
    ddlState_t searchState; // [esp+8h] [ebp-10h] BYREF

    if ( BG_UnlockablesAllItemsUnlocked() )
        return 0;
    if ( BG_UnlockablesIsItemLocked(controllerIndex, itemIndex) )
        return 0;
    if ( !BG_UnlockablesGetItemInfo(itemIndex) )
        return 0;
    v4 = va("%d", itemIndex);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 3, "itemStats", v4) )
    {
        buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        return DDL_GetInt(&searchState, (char *)buffer) != 0;
    }
    else
    {
        DDL_PrintError("DDL: Error could not find item %d\n", itemIndex);
        return 0;
    }
}

char __cdecl BG_UnlockablesIsItemGroupNew(int controllerIndex, const char *groupName)
{
    int itemIndex; // [esp+4h] [ebp-8h]
    int group; // [esp+8h] [ebp-4h]

    if ( !groupName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    1825,
                    0,
                    "%s",
                    "groupName") )
    {
        __debugbreak();
    }
    if ( !*groupName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    1826,
                    0,
                    "%s",
                    "groupName[ 0 ]") )
    {
        __debugbreak();
    }
    if ( BG_UnlockablesAllItemsUnlocked() )
        return 0;
    if ( groupName && *groupName )
    {
        group = FindEntryInList(groupName, s_itemGroupNames, 24);
        if ( group == -1 )
        {
            Com_PrintError(15, "Item Group %s not found\n", groupName);
            return 0;
        }
        for ( itemIndex = 0; itemIndex <= s_unlockableItems.maxItem; ++itemIndex )
        {
            if ( s_unlockableItems.itemTable[itemIndex].isValid
                && s_unlockableItems.itemTable[itemIndex].group == group
                && BG_UnlockablesIsItemNew(controllerIndex, itemIndex) )
            {
                return 1;
            }
        }
    }
    return 0;
}

void __cdecl BG_UnlockablesPurchaseItem(int controllerIndex, int itemIndex, int cost)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-8h]
    int currentCodPoints; // [esp+4h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( BG_UnlockablesIsItemPurchased(controllerIndex, itemIndex) )
    {
        Com_PrintError(15, "Item %s is already purchased.\n", itemInfo->name);
    }
    else if ( itemInfo )
    {
        if ( BG_UnlockablesIsItemLocked(controllerIndex, itemIndex) )
        {
            Com_PrintError(15, "Item %s is locked for the current level\n", itemInfo->name);
        }
        else
        {
            currentCodPoints = LiveStats_GetCurrency(controllerIndex);
            if ( currentCodPoints >= cost )
            {
                if ( itemInfo->group == ITEMGROUP_HEAD )
                    LiveCounter_IncrementCounterValueByName("global_purchasedfacepaint", 1u);
                if ( BG_UnlockablesSetItemPurchased(controllerIndex, itemIndex, 1) )
                    LiveStats_SpendCurrency(controllerIndex, cost, POINTSSPENT_UNLOCKABLE_ITEM, itemIndex);
            }
            else
            {
                Com_PrintError(15, "Item %s costs %d and you only have %d\n", itemInfo->name, cost, currentCodPoints);
            }
        }
    }
}

char __cdecl BG_UnlockablesSetItemPurchased(int controllerIndex, int itemIndex, bool isPurchased)
{
    const ddlState_t *RootDDLState; // eax
    char *v5; // [esp-8h] [ebp-20h]
    persistentStats *buffer; // [esp+0h] [ebp-18h]
    ddlState_t searchState; // [esp+8h] [ebp-10h] BYREF

    v5 = va("%d", itemIndex);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 3, "itemStats", v5) )
    {
        buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        if ( DDL_SetInt(&searchState, isPurchased, (char *)buffer) )
        {
            return 1;
        }
        else
        {
            DDL_PrintError("DDL: Error could not set purchased flag for item %d\n", itemIndex);
            return 0;
        }
    }
    else
    {
        DDL_PrintError("DDL: Error could not find item %d\n", itemIndex);
        return 0;
    }
}

void __cdecl BG_UnlockablesSellItem(int controllerIndex, int itemIndex)
{
    const ddlState_t *RootDDLState; // eax
    int ActualDefaultItem; // eax
    char *v4; // [esp-8h] [ebp-34h]
    const char *customClassName; // [esp+0h] [ebp-2Ch]
    const char *loadoutName; // [esp+4h] [ebp-28h]
    loadoutSlot_t loadoutSlot; // [esp+8h] [ebp-24h]
    customClass_t classIndex; // [esp+Ch] [ebp-20h]
    int sellBackAmount; // [esp+10h] [ebp-1Ch]
    persistentStats *buffer; // [esp+14h] [ebp-18h]
    itemInfo_t *itemInfo; // [esp+18h] [ebp-14h]
    ddlState_t searchState; // [esp+1Ch] [ebp-10h] BYREF

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( BG_UnlockablesIsItemPurchased(controllerIndex, itemIndex) )
    {
        if ( itemInfo )
        {
            if ( BG_UnlockablesIsItemLocked(controllerIndex, itemIndex) )
            {
                Com_PrintError(15, "Item %s is locked for the current level\n", itemInfo->name);
            }
            else
            {
                buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
                v4 = va("%d", itemIndex);
                RootDDLState = LiveStats_GetRootDDLState();
                if ( DDL_MoveTo(RootDDLState, &searchState, 3, "itemStats", v4) )
                {
                    if ( DDL_SetInt(&searchState, 0, (char *)buffer) )
                    {
                        sellBackAmount = 0;
                        if ( itemSellFactor && itemSellFactor->current.enabled )
                            sellBackAmount = (int)(float)((float)itemInfo->cost * itemSellFactor->current.value);
                        LiveStats_SpendCurrency(controllerIndex, -sellBackAmount, POINTSSPENT_UNLOCKABLE_SELL, itemIndex);
                        for ( classIndex = CUSTOM_CLASS_1; classIndex < CUSTOM_CLASS_COUNT; ++classIndex )
                        {
                            for ( loadoutSlot = LOADOUTSLOT_FIRST; loadoutSlot < LOADOUTSLOT_GLOBAL_ITEMS_START; ++loadoutSlot )
                            {
                                customClassName = BG_UnlockablesGetCustomClassName(classIndex);
                                if ( customClassName )
                                {
                                    loadoutName = BG_UnlockablesGetLoadoutName(loadoutSlot);
                                    if ( loadoutName )
                                    {
                                        if ( itemIndex == BG_UnlockablesGetEquippedItemInSlot(controllerIndex, customClassName, loadoutName) )
                                        {
                                            ActualDefaultItem = BG_UnlockablesGetActualDefaultItem(loadoutSlot);
                                            BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, ActualDefaultItem, loadoutName);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        DDL_PrintError("DDL: Error could not clear purchased flag for item %d\n", itemIndex);
                    }
                }
                else
                {
                    DDL_PrintError("DDL: Error could not find item %d\n", itemIndex);
                }
            }
        }
    }
    else
    {
        Com_PrintError(15, "Item %s has not been purchased.\n", itemInfo->name);
    }
}

void __cdecl BG_UnlockablesSetPurchasedBits(unsigned __int8 *statsBuffer, unsigned __int8 *purchasedItems)
{
    int itemIndex; // [esp+8h] [ebp-4h]

    for ( itemIndex = 0; itemIndex < 256; ++itemIndex )
    {
        if ( BG_UnlockablesIsItemValidNotNull(itemIndex) && BG_UnlockablesIsItemPurchasedFromBuffer(statsBuffer, itemIndex) )
            purchasedItems[itemIndex >> 3] |= 1 << (itemIndex & 7);
    }
}

bool __cdecl BG_UnlockablesIsItemPurchasedFromBuffer(unsigned __int8 *statsBuffer, int itemIndex)
{
    const ddlState_t *RootDDLState; // eax
    char *v4; // [esp-8h] [ebp-20h]
    itemInfo_t *itemInfo; // [esp+4h] [ebp-14h]
    ddlState_t searchState; // [esp+8h] [ebp-10h] BYREF

    if ( BG_UnlockablesAllItemsFree() )
        return 1;
    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( !itemInfo )
        return 0;
    if ( !itemInfo->cost )
        return 1;
    v4 = va("%d", itemIndex);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 3, "itemStats", v4) )
        return DDL_GetInt(&searchState, (char *)statsBuffer) != 0;
    DDL_PrintError("DDL: Error could not find item %d\n", itemIndex);
    return 0;
}

bool __cdecl BG_UnlockablesIsItemPurchased(int controllerIndex, int itemIndex)
{
    persistentStats *StatsBuffer; // eax

    if ( BG_UnlockablesAllItemsFree() )
        return 1;
    if ( BG_UnlockablesIsItemLocked(controllerIndex, itemIndex) )
        return 0;
    StatsBuffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
    return BG_UnlockablesIsItemPurchasedFromBuffer(StatsBuffer->statsBuffer, itemIndex);
}

bool __cdecl BG_UnlockablesIsItemDualWieldPurchased(int controllerIndex, int itemIndex)
{
    const ddlState_t *RootDDLState; // eax
    char *v4; // [esp-8h] [ebp-20h]
    persistentStats *buffer; // [esp+0h] [ebp-18h]
    itemInfo_t *itemInfo; // [esp+4h] [ebp-14h]
    ddlState_t searchState; // [esp+8h] [ebp-10h] BYREF

    if ( BG_UnlockablesAllItemsFree() )
        return 1;
    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( !itemInfo )
        return 0;
    if ( itemInfo->dwIndex == -1 )
        return 0;
    if ( BG_UnlockablesIsItemLocked(controllerIndex, itemInfo->dwIndex) )
        return 0;
    if ( !itemInfo->dwCost )
        return 1;
    v4 = va("%d", itemInfo->dwIndex);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 3, "itemStats", v4) )
    {
        buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        return DDL_GetInt(&searchState, (char *)buffer) != 0;
    }
    else
    {
        DDL_PrintError("DDL: Error could not find item %d\n", itemInfo->dwIndex);
        return 0;
    }
}

const char *__cdecl BG_UnlockablesGetItemAttachmentDisplayName(int itemIndex, int attachmentNum)
{
    eAttachment attachment; // [esp+0h] [ebp-4h]

    attachment = BG_UnlockablesGetItemAttachment(itemIndex, attachmentNum);
    return BG_GetAttachmentDisplayName(attachment);
}

const char *__cdecl BG_UnlockablesGetItemAttachmentDesc(int itemIndex, int attachmentNum)
{
    eAttachment attachment; // [esp+0h] [ebp-4h]

    attachment = BG_UnlockablesGetItemAttachment(itemIndex, attachmentNum);
    return BG_GetAttachmentDesc(attachment);
}

int __cdecl BG_UnlockablesGetItemAttachmentCost(int itemIndex, int attachmentNum)
{
    eAttachment attachment; // [esp+0h] [ebp-4h]

    attachment = BG_UnlockablesGetItemAttachment(itemIndex, attachmentNum);
    return BG_GetAttachmentCost(attachment);
}

const char *__cdecl BG_UnlockablesGetItemAttachmentRef(int itemIndex, int attachmentNum)
{
    eAttachment attachment; // [esp+0h] [ebp-4h]

    attachment = BG_UnlockablesGetItemAttachment(itemIndex, attachmentNum);
    return BG_GetAttachmentReference(attachment);
}

bool __cdecl BG_UnlockablesGetAttachmentPurchased(int controllerIndex, const char *attachmentRef)
{
    const ddlState_t *RootDDLState; // eax
    persistentStats *buffer; // [esp+0h] [ebp-14h]
    ddlState_t searchState; // [esp+4h] [ebp-10h] BYREF

    if ( !attachmentRef
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    2213,
                    0,
                    "%s",
                    "attachmentRef") )
    {
        __debugbreak();
    }
    if ( !attachmentRef )
        return 0;
    RootDDLState = LiveStats_GetRootDDLState();
    if ( !DDL_MoveTo(RootDDLState, &searchState, 2, "PurchasedAttachments", attachmentRef) )
    {
        DDL_PrintError("DDL: Error could not find attachment %s\n", attachmentRef);
        return 0;
    }
    buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
    if ( !buffer
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp", 2227, 0, "%s", "buffer") )
    {
        __debugbreak();
    }
    return buffer && DDL_GetInt(&searchState, (char *)buffer) == 1;
}

void __cdecl BG_UnlockablesPurchaseItemAttachmentPoint(int controllerIndex, int itemIndex, const char *attachmentPoint)
{
    const ddlState_t *RootDDLState; // eax
    char *v4; // [esp-Ch] [ebp-30h]
    persistentStats *buffer; // [esp+0h] [ebp-24h]
    ddlState_t searchState; // [esp+4h] [ebp-20h] BYREF
    const AttachmentPointTableEntry *attachmentPointInfo; // [esp+14h] [ebp-10h]
    itemInfo_t *itemInfo; // [esp+18h] [ebp-Ch]
    eAttachmentPoint attachmentPoinNum; // [esp+1Ch] [ebp-8h]
    int currentCodPoints; // [esp+20h] [ebp-4h]

    if ( !BG_UnlockablesIsItemLocked(controllerIndex, itemIndex)
        && BG_UnlockablesIsItemPurchased(controllerIndex, itemIndex) )
    {
        itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
        if ( itemInfo )
        {
            if ( BG_UnlockablesIsItemAttachmentPointPurchased(controllerIndex, itemIndex, attachmentPoint) )
            {
                Com_PrintError(15, "Attachment point %s is already purchased for item %s\n", attachmentPoint, itemInfo->name);
            }
            else
            {
                currentCodPoints = LiveStats_GetCurrency(controllerIndex);
                attachmentPoinNum = BG_GetAttachmentPointIndex(attachmentPoint);
                attachmentPointInfo = BG_GetAttachmentPointTableEntry(attachmentPoinNum);
                if ( !attachmentPointInfo
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                                2306,
                                0,
                                "%s",
                                "attachmentPointInfo") )
                {
                    __debugbreak();
                }
                if ( attachmentPointInfo )
                {
                    if ( currentCodPoints >= attachmentPointInfo->values.cost )
                    {
                        currentCodPoints -= attachmentPointInfo->values.cost;
                        v4 = va("%d", itemIndex);
                        RootDDLState = LiveStats_GetRootDDLState();
                        if ( DDL_MoveTo(RootDDLState, &searchState, 4, "weaponStats", v4) )
                        {
                            buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
                            if ( DDL_SetInt(&searchState, 1u, (char *)buffer) )
                                LiveStats_SpendCurrency(
                                    controllerIndex,
                                    attachmentPointInfo->values.cost,
                                    POINTSSPENT_UNLOCKABLE_ATTACHMENT_POINT,
                                    itemIndex);
                            else
                                DDL_PrintError(
                                    "DDL: Error could not set purchased flag for item %d attachment point %s\n",
                                    itemIndex,
                                    attachmentPoint);
                        }
                        else
                        {
                            DDL_PrintError("DDL: Error could not find item %d attachment point %s\n", itemIndex, attachmentPoint);
                        }
                    }
                    else
                    {
                        Com_PrintError(
                            15,
                            "Item attachment point %s costs %d and you only have %d\n",
                            attachmentPoint,
                            attachmentPointInfo->values.cost,
                            currentCodPoints);
                    }
                }
            }
        }
    }
}

int __cdecl BG_UnlockablesIsItemAttachmentValid(itemInfo_t *itemInfo, const char *attachmentString)
{
    const char **AttachmentNames; // eax
    int numAttachments; // [esp+0h] [ebp-8h]
    eAttachment attachment; // [esp+4h] [ebp-4h]

    AttachmentNames = BG_GetAttachmentNames();
    attachment = (eAttachment)FindEntryInList(attachmentString, AttachmentNames, 24);
    if ( attachment != -1 )
    {
        for ( numAttachments = 0; numAttachments < itemInfo->numAttachments[0]; ++numAttachments )
        {
            if ( itemInfo->attachments[numAttachments] == attachment )
                return numAttachments;
        }
    }
    return -1;
}

void __cdecl BG_UnlockablesPurchaseItemAttachment(int controllerIndex, int itemIndex, eAttachment attachmentNum)
{
    const char *AttachmentName; // eax
    int cost; // [esp-8h] [ebp-18h]
    const AttachmentTableEntry *attachmentInfo; // [esp+0h] [ebp-10h]
    eAttachment attachment; // [esp+4h] [ebp-Ch]
    itemInfo_t *itemInfo; // [esp+8h] [ebp-8h]
    int currentCodPoints; // [esp+Ch] [ebp-4h]

    if ( !BG_UnlockablesIsItemLocked(controllerIndex, itemIndex)
        && BG_UnlockablesIsItemPurchased(controllerIndex, itemIndex) )
    {
        itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
        if ( itemInfo )
        {
            if ( BG_UnlockablesIsItemAttachmentPurchased(controllerIndex, itemIndex, attachmentNum) )
            {
                Com_PrintError(15, "Attachment num %d is already purchased for item %s\n", attachmentNum, itemInfo->name);
            }
            else
            {
                currentCodPoints = LiveStats_GetCurrency(controllerIndex);
                attachment = BG_UnlockablesGetItemAttachment(itemIndex, attachmentNum);
                attachmentInfo = BG_GetAttachmentTableEntry(attachment);
                if ( !attachmentInfo
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                                2262,
                                0,
                                "%s",
                                "attachmentInfo") )
                {
                    __debugbreak();
                }
                if ( attachmentInfo )
                {
                    if ( currentCodPoints >= attachmentInfo->values.cost )
                    {
                        if ( BG_UnlockablesSetItemAttachmentPurchased(controllerIndex, itemIndex, attachmentNum, 1) )
                        {
                            LiveStats_SpendCurrency(
                                controllerIndex,
                                attachmentInfo->values.cost,
                                POINTSSPENT_UNLOCKABLE_ATTACHMENT,
                                attachmentNum);
                            BG_UnlockablesSetAttachmentPurchased(controllerIndex, attachmentInfo->values.referenceName, 1);
                        }
                    }
                    else
                    {
                        cost = attachmentInfo->values.cost;
                        AttachmentName = BG_GetAttachmentName(attachmentNum);
                        Com_PrintError(
                            15,
                            "Item attachment %s costs %d and you only have %d\n",
                            AttachmentName,
                            cost,
                            currentCodPoints);
                    }
                }
            }
        }
    }
}

char __cdecl BG_UnlockablesSetItemAttachmentPurchased(
                int controllerIndex,
                int itemIndex,
                int attachmentNum,
                bool isPurchased)
{
    const ddlState_t *RootDDLState; // eax
    char *v6; // [esp-Ch] [ebp-24h]
    persistentStats *buffer; // [esp+0h] [ebp-18h]
    ddlState_t searchState; // [esp+8h] [ebp-10h] BYREF

    va("%d", attachmentNum);
    v6 = va("%d", itemIndex);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 4, "weaponStats", v6) )
    {
        buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        if ( DDL_SetInt(&searchState, isPurchased, (char *)buffer) )
        {
            return 1;
        }
        else
        {
            DDL_PrintError("DDL: Error could not set purchased flag for item %d attachment %d\n", itemIndex, attachmentNum);
            return 0;
        }
    }
    else
    {
        DDL_PrintError("DDL: Error could not find item %d attachment %d\n", itemIndex, attachmentNum);
        return 0;
    }
}

char __cdecl BG_UnlockablesSetAttachmentPurchased(int controllerIndex, const char *attachmentRef, bool isPurchased)
{
    const ddlState_t *RootDDLState; // eax
    persistentStats *buffer; // [esp+4h] [ebp-14h]
    ddlState_t searchState; // [esp+8h] [ebp-10h] BYREF

    if ( !attachmentRef
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    2178,
                    0,
                    "%s",
                    "attachmentRef") )
    {
        __debugbreak();
    }
    if ( !attachmentRef )
        return 0;
    RootDDLState = LiveStats_GetRootDDLState();
    if ( !DDL_MoveTo(RootDDLState, &searchState, 2, "PurchasedAttachments", attachmentRef) )
    {
        DDL_PrintError("DDL: Error could not find attachment %s\n", attachmentRef);
        return 0;
    }
    buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
    if ( !buffer
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp", 2192, 0, "%s", "buffer") )
    {
        __debugbreak();
    }
    if ( !buffer )
        return 0;
    if ( DDL_SetInt(&searchState, isPurchased, (char *)buffer) )
        return 1;
    DDL_PrintError("DDL: Error could not set purchased flag for attachment %s\n", attachmentRef);
    return 0;
}

bool __cdecl BG_UnlockablesIsItemAttachmentPointPurchased(
                int controllerIndex,
                int itemIndex,
                const char *attachmentPoint)
{
    eAttachmentPoint AttachmentPointIndex; // eax
    const ddlState_t *RootDDLState; // eax
    char *v6; // [esp-Ch] [ebp-24h]
    persistentStats *buffer; // [esp+0h] [ebp-18h]
    ddlState_t searchState; // [esp+4h] [ebp-14h] BYREF

    if ( BG_UnlockablesAllItemsFree() )
        return 1;
    if ( BG_UnlockablesIsItemLocked(controllerIndex, itemIndex)
        || !BG_UnlockablesIsItemPurchased(controllerIndex, itemIndex) )
    {
        return 0;
    }
    if ( !BG_UnlockablesGetItemInfo(itemIndex) )
        return 0;
    if ( !I_stricmp(attachmentPoint, "") )
        return 0;
    AttachmentPointIndex = BG_GetAttachmentPointIndex(attachmentPoint);
    if ( !BG_GetAttachmentPointCost(AttachmentPointIndex) )
        return 1;
    v6 = va("%d", itemIndex);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 4, "weaponStats", v6) )
    {
        buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        return DDL_GetInt(&searchState, (char *)buffer) != 0;
    }
    else
    {
        DDL_PrintError("DDL: Error could not find item %d attachment point %s\n", itemIndex, attachmentPoint);
        return 0;
    }
}

bool __cdecl BG_UnlockablesIsItemAttachmentPurchased(int controllerIndex, int itemIndex, int attachmentNum)
{
    eAttachment ItemAttachment; // eax
    const ddlState_t *RootDDLState; // eax
    char *v6; // [esp-Ch] [ebp-24h]
    persistentStats *buffer; // [esp+0h] [ebp-18h]
    ddlState_t searchState; // [esp+4h] [ebp-14h] BYREF
    itemInfo_t *itemInfo; // [esp+14h] [ebp-4h]

    if ( BG_UnlockablesAllItemsFree() )
        return 1;
    if ( BG_UnlockablesIsItemLocked(controllerIndex, itemIndex)
        || !BG_UnlockablesIsItemPurchased(controllerIndex, itemIndex) )
    {
        return 0;
    }
    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( !itemInfo )
        return 0;
    if ( attachmentNum >= itemInfo->numAttachments[0] )
        return 0;
    ItemAttachment = BG_UnlockablesGetItemAttachment(itemIndex, attachmentNum);
    if ( !BG_GetAttachmentCost(ItemAttachment) )
        return 1;
    va("%d", attachmentNum);
    v6 = va("%d", itemIndex);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 4, "weaponStats", v6) )
    {
        buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        return DDL_GetInt(&searchState, (char *)buffer) != 0;
    }
    else
    {
        DDL_PrintError("DDL: Error could not find item %d attachment %d\n", itemIndex, attachmentNum);
        return 0;
    }
}

int __cdecl BG_UnlockablesGetWeaponOptionFromName(const char *optionName)
{
    return BG_GetWeaponOptionFromName(optionName);
}

bool __cdecl BG_UnlockablesIsItemOptionPurchased(int controllerIndex, int itemIndex, int optionIndex)
{
    const ddlState_t *RootDDLState; // eax
    char *v5; // [esp-Ch] [ebp-24h]
    persistentStats *buffer; // [esp+0h] [ebp-18h]
    ddlState_t searchState; // [esp+4h] [ebp-14h] BYREF

    if ( BG_UnlockablesAllItemsFree() )
        return 1;
    if ( !optionIndex )
        return 1;
    if ( optionIndex >= BG_GetWeaponOptionCount()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    2669,
                    0,
                    "%s",
                    "optionIndex < BG_GetWeaponOptionCount()") )
    {
        __debugbreak();
    }
    if ( optionIndex >= BG_GetWeaponOptionCount() )
        return 0;
    if ( BG_UnlockablesIsItemLocked(controllerIndex, itemIndex)
        || !BG_UnlockablesIsItemPurchased(controllerIndex, itemIndex) )
    {
        return 0;
    }
    if ( !BG_UnlockablesGetItemInfo(itemIndex) )
        return 0;
    if ( !BG_GetWeaponOptionCost(optionIndex) )
        return 1;
    va("%d", optionIndex);
    v5 = va("%d", itemIndex);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 4, "weaponStats", v5) )
    {
        buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        return DDL_GetInt(&searchState, (char *)buffer) != 0;
    }
    else
    {
        DDL_PrintError("DDL: Error could not find item %d camo %d\n", itemIndex, optionIndex);
        return 0;
    }
}

int __cdecl BG_UnlockablesGetItemIndexFromName(const char *itemName)
{
    int v2; // [esp+20h] [ebp-10h]
    int itemIndex; // [esp+24h] [ebp-Ch]
    int searchHash; // [esp+28h] [ebp-8h]

    PROF_SCOPED("BG_UnlockablesGetItemIndexFromName");

    if ( !s_unlockableItems.maxItem
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    2711,
                    0,
                    "%s",
                    "s_unlockableItems.maxItem") )
    {
        __debugbreak();
    }
    if ( !itemName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp", 2712, 0, "%s", "itemName") )
    {
        __debugbreak();
    }
    if ( itemName && *itemName )
    {
        searchHash = StringTable_HashString(itemName);
        if ( lastItem != -1
            && searchHash == s_unlockableItems.itemTable[lastItem].nameHash
            && s_unlockableItems.itemTable[lastItem].name
            && !I_stricmp(s_unlockableItems.itemTable[lastItem].name, itemName) )
        {
            v2 = lastItem;
            return v2;
        }
        for ( itemIndex = 0; itemIndex <= s_unlockableItems.maxItem; ++itemIndex )
        {
            if ( s_unlockableItems.itemTable[itemIndex].isValid )
            {
                if ( !s_unlockableItems.itemTable[itemIndex].name
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                                2731,
                                0,
                                "%s",
                                "s_unlockableItems.itemTable[ itemIndex ].name") )
                {
                    __debugbreak();
                }
                if ( s_unlockableItems.itemTable[itemIndex].name
                    && searchHash == s_unlockableItems.itemTable[itemIndex].nameHash
                    && !I_stricmp(s_unlockableItems.itemTable[itemIndex].name, itemName) )
                {
                    lastItem = itemIndex;
                    return itemIndex;
                }
            }
        }
    }
    return -1;
}

int __cdecl BG_UnlockablesGetItemIndexFromRef(const char *itemName)
{
    int itemIndex; // [esp+0h] [ebp-8h]
    int searchHash; // [esp+4h] [ebp-4h]

    if ( !s_unlockableItems.maxItem
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    2749,
                    0,
                    "%s",
                    "s_unlockableItems.maxItem") )
    {
        __debugbreak();
    }
    if ( !itemName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp", 2750, 0, "%s", "itemName") )
    {
        __debugbreak();
    }
    if ( itemName && *itemName )
    {
        searchHash = StringTable_HashString(itemName);
        for ( itemIndex = 0; itemIndex <= s_unlockableItems.maxItem; ++itemIndex )
        {
            if ( s_unlockableItems.itemTable[itemIndex].isValid )
            {
                if ( !s_unlockableItems.itemTable[itemIndex].reference
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                                2759,
                                0,
                                "%s",
                                "s_unlockableItems.itemTable[ itemIndex ].reference") )
                {
                    __debugbreak();
                }
                if ( s_unlockableItems.itemTable[itemIndex].reference
                    && searchHash == s_unlockableItems.itemTable[itemIndex].refHash
                    && !I_stricmp(s_unlockableItems.itemTable[itemIndex].reference, itemName) )
                {
                    return itemIndex;
                }
            }
        }
    }
    return -1;
}

int __cdecl BG_UnlockablesGetItemIndexforWeapon(unsigned int weaponIndex)
{
    int itemIndex; // [esp+14h] [ebp-Ch]
    const char *weaponReference; // [esp+18h] [ebp-8h]
    const WeaponVariantDef *weaponVariantDef; // [esp+1Ch] [ebp-4h]

    if ( !s_unlockableItems.maxItem
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    2776,
                    0,
                    "%s",
                    "s_unlockableItems.maxItem") )
    {
        __debugbreak();
    }
    if ( !weaponIndex )
        return -1;
    weaponVariantDef = BG_GetWeaponVariantDef(weaponIndex);
    if ( weaponVariantDef )
    {
        weaponReference = weaponVariantDef->szInternalName;
        for ( itemIndex = 0; itemIndex <= s_unlockableItems.maxItem; ++itemIndex )
        {
            if ( s_unlockableItems.itemTable[itemIndex].isValid )
            {
                if ( !s_unlockableItems.itemTable[itemIndex].reference
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                                2791,
                                0,
                                "%s",
                                "s_unlockableItems.itemTable[ itemIndex ].reference") )
                {
                    __debugbreak();
                }
                if ( s_unlockableItems.itemTable[itemIndex].reference
                    && *s_unlockableItems.itemTable[itemIndex].reference
                    && !I_strnicmp(
                                s_unlockableItems.itemTable[itemIndex].reference,
                                weaponReference,
                                strlen(s_unlockableItems.itemTable[itemIndex].reference)) )
                {
                    return itemIndex;
                }
            }
        }
    }
    return -1;
}

void __cdecl BG_UnlockablesClearItemNew(int controllerIndex, int itemIndex)
{
    const ddlState_t *RootDDLState; // eax
    char *v3; // [esp-8h] [ebp-20h]
    persistentStats *buffer; // [esp+0h] [ebp-18h]
    ddlState_t searchState; // [esp+4h] [ebp-14h] BYREF

    if ( BG_UnlockablesIsItemNew(controllerIndex, itemIndex) )
    {
        if ( BG_UnlockablesGetItemInfo(itemIndex) )
        {
            v3 = va("%d", itemIndex);
            RootDDLState = LiveStats_GetRootDDLState();
            if ( DDL_MoveTo(RootDDLState, &searchState, 3, "itemStats", v3) )
            {
                buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
                if ( !DDL_SetInt(&searchState, 0, (char *)buffer) )
                    DDL_PrintError("DDL: Error could not set purchased flag for item %d\n", itemIndex);
            }
            else
            {
                DDL_PrintError("DDL: Error could not find item %d\n", itemIndex);
            }
        }
    }
}

void __cdecl BG_UnlockablesClearAllItemsNew(int controllerIndex)
{
    int index; // [esp+0h] [ebp-4h]

    for ( index = 0; index < 256; ++index )
    {
        if ( BG_UnlockablesIsItemValidNotNull(index) )
            BG_UnlockablesClearItemNew(controllerIndex, index);
    }
}

int __cdecl BG_UnlockablesGetAssociatedBody(int itemIndex)
{
    if ( bodyTypeFromGun && bodyTypeFromGun->current.enabled )
        return BG_UnlockablesGetAssociatedBodyWithPrimary(itemIndex);
    else
        return BG_UnlockablesGetAssociatedBodyWithPerk(itemIndex);
}

int __cdecl BG_UnlockablesGetAssociatedBodyWithPrimary(int itemIndex)
{
    itemInfo_t *itemInfo2; // [esp+0h] [ebp-10h]
    int itemNum; // [esp+4h] [ebp-Ch]
    const char *groupName; // [esp+8h] [ebp-8h]
    itemInfo_t *itemInfo; // [esp+Ch] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo )
    {
        if ( itemInfo->loadoutSlot == LOADOUTSLOT_FIRST )
        {
            groupName = BG_UnlockablesGetItemGroupName(itemInfo->group);
            if ( groupName )
            {
                if ( *groupName )
                {
                    for ( itemNum = 0; itemNum < 256; ++itemNum )
                    {
                        if ( BG_UnlockablesIsItemValidNotNull(itemNum) )
                        {
                            itemInfo2 = BG_UnlockablesGetItemInfo(itemNum);
                            if ( itemInfo2->loadoutSlot == LOADOUTSLOT_BODY
                                && itemInfo2->desc
                                && *itemInfo2->desc
                                && !I_stricmp(itemInfo2->desc, groupName) )
                            {
                                return itemNum;
                            }
                        }
                    }
                }
            }
        }
    }
    return -1;
}

int __cdecl BG_UnlockablesGetAssociatedBodyWithPerk(int itemIndex)
{
    itemInfo_t *itemInfo2; // [esp+10h] [ebp-Ch]
    int itemNum; // [esp+14h] [ebp-8h]
    itemInfo_t *itemInfo; // [esp+18h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo && itemInfo->loadoutSlot == LOADOUTSLOT_FIRST_SPECIALTY && itemInfo->name && *itemInfo->name )
    {
        for ( itemNum = 0; itemNum < 256; ++itemNum )
        {
            if ( BG_UnlockablesIsItemValidNotNull(itemNum) )
            {
                itemInfo2 = BG_UnlockablesGetItemInfo(itemNum);
                if ( itemInfo2->loadoutSlot == LOADOUTSLOT_BODY
                    && itemInfo2->imageRef
                    && *itemInfo2->imageRef
                    && !I_strnicmp(itemInfo2->imageRef, itemInfo->name, strlen(itemInfo2->imageRef)) )
                {
                    return itemNum;
                }
            }
        }
    }
    return -1;
}

void __cdecl BG_UnlockablesEquipClassToSlot(
                int controllerIndex,
                const char *customClassName,
                int itemIndex,
                const char *loadoutName)
{
    const ddlState_t *CacDDLState; // eax
    const ddlState_t *v5; // eax
    itemInfo_t *ItemInfo; // eax
    persistentStats *buffer; // [esp+0h] [ebp-18h]
    ddlState_t searchState; // [esp+4h] [ebp-14h] BYREF
    unsigned int statValue; // [esp+14h] [ebp-4h]

    if ( Dvar_GetInt(ui_useCustomClassInfo) )
    {
        if ( GetCustomClassLoadoutItemForClassName(controllerIndex, customClassName, loadoutName) != itemIndex )
        {
            UI_Gametype_EquipCustomClassLoadoutItem(controllerIndex, customClassName, itemIndex, loadoutName);
            BG_UnlockablesEquipClassSpecialPropertiesToSlot(controllerIndex, customClassName, itemIndex, loadoutName);
        }
    }
    else
    {
        if ( customClassName )
        {
            CacDDLState = LiveStats_GetCacDDLState();
            if ( !DDL_MoveTo(CacDDLState, &searchState, 2, customClassName, loadoutName) )
            {
                DDL_PrintError("DDL: Error could not find loadout slot %s\n", loadoutName);
                return;
            }
        }
        else
        {
            v5 = LiveStats_GetCacDDLState();
            if ( !DDL_MoveTo(v5, &searchState, 1, loadoutName) )
            {
                DDL_PrintError("DDL: Error could not find global slot %s\n", loadoutName);
                return;
            }
        }
        buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        statValue = DDL_GetInt(&searchState, (char *)buffer);
        if ( statValue != itemIndex )
        {
            BG_UnlockablesEquipClassSpecialPropertiesToSlot(controllerIndex, customClassName, itemIndex, loadoutName);
            if ( !DDL_SetInt(&searchState, itemIndex, (char *)buffer) )
            {
                ItemInfo = BG_UnlockablesGetItemInfo(itemIndex);
                DDL_PrintError("DDL: Error equipping class %s with item %s\n", customClassName, ItemInfo->name);
            }
        }
    }
}

void __cdecl BG_UnlockablesEquipClassSpecialPropertiesToSlot(
                int controllerIndex,
                const char *customClassName,
                int itemIndex,
                const char *loadoutName)
{
    const char *AttachmentPointName; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    eAttachmentPoint attachPoint; // [esp+0h] [ebp-4h]

    if ( I_stricmp(loadoutName, "primary") && I_stricmp(loadoutName, "secondary") )
    {
        if ( !I_strnicmp(loadoutName, "specialty", 9) )
            BG_UnlockablesEquipAssociatedBody(controllerIndex, customClassName, itemIndex);
    }
    else
    {
        BG_UnlockablesEquipAssociatedBody(controllerIndex, customClassName, itemIndex);
        for ( attachPoint = ATTACHMENT_POINT_TOP; attachPoint < ATTACHMENT_POINT_COUNT; ++attachPoint )
        {
            AttachmentPointName = BG_GetAttachmentPointName(attachPoint);
            v5 = va("%sattachment%s", loadoutName, AttachmentPointName);
            BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, 0, v5);
        }
        v6 = va("%scamo", loadoutName);
        BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, 0, v6);
        v7 = va("%sreticle", loadoutName);
        BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, 0, v7);
        v8 = va("%sreticlecolor", loadoutName);
        BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, 0, v8);
        v9 = va("%slens", loadoutName);
        BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, 0, v9);
        v10 = va("%stag", loadoutName);
        BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, 0, v10);
        v11 = va("%semblem", loadoutName);
        BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, 0, v11);
    }
}

void __cdecl BG_UnlockablesEquipAssociatedBody(int controllerIndex, const char *customClassName, int itemIndex)
{
    int bodyIndex; // [esp+0h] [ebp-4h]

    bodyIndex = BG_UnlockablesGetAssociatedBody(itemIndex);
    if ( bodyIndex != -1 )
        BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, bodyIndex, "body");
}

unsigned int __cdecl BG_UnlockablesGetEquippedItemInSlot(
                int controllerIndex,
                const char *customClassName,
                const char *slotName)
{
    const ddlState_t *CacDDLState; // eax
    const ddlState_t *v5; // eax
    persistentStats *buffer; // [esp+0h] [ebp-14h]
    ddlState_t searchState; // [esp+4h] [ebp-10h] BYREF

    if ( Dvar_GetInt(ui_useCustomClassInfo) )
        return GetCustomClassLoadoutItemForClassName(controllerIndex, customClassName, slotName);
    if ( customClassName )
    {
        CacDDLState = LiveStats_GetCacDDLState();
        if ( !DDL_MoveTo(CacDDLState, &searchState, 2, customClassName, slotName) )
        {
LABEL_5:
            DDL_PrintError("DDL: Error could not find loadout slot %s\n", slotName);
            return -1;
        }
    }
    else
    {
        v5 = LiveStats_GetCacDDLState();
        if ( !DDL_MoveTo(v5, &searchState, 1, slotName) )
            goto LABEL_5;
    }
    buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
    return DDL_GetInt(&searchState, (char *)buffer);
}

bool __cdecl BG_UnlockablesIsItemEquipped(int controllerIndex, const char *customClassName, int itemIndex)
{
    const char *v3; // eax
    const char *loadoutName; // [esp+0h] [ebp-8h]
    itemInfo_t *itemInfo; // [esp+4h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo->loadoutSlot == LOADOUTSLOT_INVALID )
    {
        v3 = va("Item %s has an invalid loadout slot\n", itemInfo->name);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                        3090,
                        0,
                        "%s\n\t%s",
                        "itemInfo->loadoutSlot != LOADOUTSLOT_INVALID",
                        v3) )
            __debugbreak();
    }
    return itemInfo->loadoutSlot != LOADOUTSLOT_INVALID
            && (loadoutName = BG_UnlockablesGetLoadoutName(itemInfo->loadoutSlot)) != 0
            && itemIndex == BG_UnlockablesGetEquippedItemInSlot(controllerIndex, customClassName, loadoutName);
}

bool __cdecl BG_UnlockablesIsItemEquippedInAnyCustomClass(int controllerIndex, int itemIndex)
{
    bool *v4; // [esp+Ch] [ebp-434h]
    bool *p_isEquipped; // [esp+14h] [ebp-42Ch]
    const char *String; // [esp+24h] [ebp-41Ch]
    unsigned int v8; // [esp+28h] [ebp-418h]
    const char *customClassName; // [esp+2Ch] [ebp-414h]
    customClass_t classIndex; // [esp+30h] [ebp-410h]
    bool isEquipped; // [esp+37h] [ebp-409h] BYREF
    char tempString[1028]; // [esp+38h] [ebp-408h] BYREF

    tempString[0] = 0;
    isEquipped = 0;
    for ( classIndex = CUSTOM_CLASS_1; classIndex < CUSTOM_CLASS_COUNT; ++classIndex )
    {
        customClassName = BG_UnlockablesGetCustomClassName(classIndex);
        if ( customClassName && BG_UnlockablesIsItemEquipped(controllerIndex, customClassName, itemIndex) )
        {
            String = Dvar_GetString(customClassName);
            v8 = (unsigned int)&String[strlen(String) + 1];
            p_isEquipped = &isEquipped;
            while ( *++p_isEquipped )
                ;
            memcpy(p_isEquipped, String, v8 - (unsigned int)String);
            v4 = &isEquipped;
            while ( *++v4 )
                ;
            strcpy((char *)v4, "\n");
            isEquipped = 1;
        }
    }
    Dvar_SetStringByName("ui_classesCurrentItemEquippedIn", tempString);
    return isEquipped;
}

int __cdecl BG_UnlockablesGetActualDefaultItem(unsigned int loadoutSlot)
{
    if ( loadoutSlot > 0x26
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    3516,
                    0,
                    "%s\n\t(loadoutSlot) = %i",
                    "(loadoutSlot >= LOADOUTSLOT_FIRST && loadoutSlot < LOADOUTSLOT_COUNT)",
                    loadoutSlot) )
    {
        __debugbreak();
    }
    if ( loadoutSlot > 0x26 )
        return 0;
    else
        return s_unlockableItems.actualDefaultItems[loadoutSlot];
}

int __cdecl BG_UnlockablesGetDefaultItem(unsigned int loadoutSlot)
{
    if ( loadoutSlot > 0x26
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    3528,
                    0,
                    "%s\n\t(loadoutSlot) = %i",
                    "(loadoutSlot >= LOADOUTSLOT_FIRST && loadoutSlot < LOADOUTSLOT_COUNT)",
                    loadoutSlot) )
    {
        __debugbreak();
    }
    if ( loadoutSlot > 0x26 )
        return 0;
    else
        return s_unlockableItems.defaultItems[loadoutSlot];
}

int __cdecl BG_UnlockablesGetDefaultWeaponItem()
{
    return BG_UnlockablesGetDefaultItem(0);
}

int __cdecl BG_UnlockablesGetDefaultClassSlotIndexFromName(const char *className, const char *slotName)
{
    const char *v2; // eax
    const char *v3; // eax
    int itemIndex; // [esp+0h] [ebp-Ch]
    signed int loadoutSlot; // [esp+4h] [ebp-8h]
    int defaultClassNum; // [esp+8h] [ebp-4h]

    defaultClassNum = FindEntryInList(className, s_classNames, 20);
    if ( defaultClassNum == -1 )
    {
        v2 = va("Default class %s not found\n", className);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                        3621,
                        0,
                        "%s\n\t%s",
                        "defaultClassNum != ENTRY_NOT_FOUND",
                        v2) )
            __debugbreak();
    }
    loadoutSlot = BG_UnlockablesGetLoadoutSlotFromString(slotName);
    if ( loadoutSlot == -1 )
    {
        v3 = va("Loadout slot %s not found\n", slotName);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                        3625,
                        0,
                        "%s\n\t%s",
                        "loadoutSlot != ENTRY_NOT_FOUND",
                        v3) )
            __debugbreak();
    }
    if ( defaultClassNum == -1 || loadoutSlot == -1 )
        return 0;
    if ( loadoutSlot >= 35 )
        itemIndex = s_unlockableItems.defaultGlobalItems[loadoutSlot - 35];
    else
        itemIndex = s_unlockableItems.defaultClassLoadouts[defaultClassNum][loadoutSlot];
    if ( itemIndex == -1 )
        return BG_UnlockablesGetDefaultItem(loadoutSlot);
    return itemIndex;
}

const char *__cdecl BG_UnlockablesGetDefaultClassSlotFromName(const char *className, const char *slotName)
{
    int DefaultClassSlotIndexFromName; // eax

    DefaultClassSlotIndexFromName = BG_UnlockablesGetDefaultClassSlotIndexFromName(className, slotName);
    return BG_UnlockablesGetItemRef(DefaultClassSlotIndexFromName);
}

bool __cdecl BG_UnlockablesIsProItemUnlocked(int controllerIndex, int itemIndex)
{
    int proChallenges; // [esp+0h] [ebp-18h]
    persistentStats *buffer; // [esp+4h] [ebp-14h]
    ddlState_t searchState; // [esp+8h] [ebp-10h] BYREF

    if ( Dvar_GetInt(ui_items_no_cost) )
        return Dvar_GetBool("custom_perks_allow_pro");
    if ( BG_UnlockablesAllItemsUnlocked() )
        return 1;
    if ( !BG_UnlockablesIsItemPurchased(controllerIndex, itemIndex) )
        return 0;
    buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_GLOBAL, 1);
    for ( proChallenges = 0; proChallenges < 3; ++proChallenges )
    {
        if ( !LiveStats_MoveToProItemVersion(&searchState, itemIndex, proChallenges) )
            return 0;
        if ( !DDL_GetInt(&searchState, (char *)buffer) )
            return 0;
    }
    return 1;
}

bool __cdecl BG_UnlockablesIsProItem(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    return itemInfo && itemInfo->group == ITEMGROUP_SPECIALTY && itemInfo->count;
}

int __cdecl BG_UnlockablesGetProItem(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+4h] [ebp-4h]

    if ( BG_UnlockablesIsProItem(itemIndex) )
        return itemIndex;
    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo && itemInfo->group == ITEMGROUP_SPECIALTY && BG_UnlockablesIsProItem(itemIndex + 1) )
        return itemIndex + 1;
    else
        return -1;
}

int __cdecl BG_UnlockablesBuildItemListForGroup(itemGroup_t group)
{
    int itemNum; // [esp+0h] [ebp-4h]

    s_unlockableItems.numItemsInSlot = 0;
    if ( group != ITEMGROUP_COUNT )
    {
        for ( itemNum = 0; itemNum <= 256; ++itemNum )
        {
            if ( s_unlockableItems.itemTable[itemNum].isValid
                && s_unlockableItems.itemTable[itemNum].group == group
                && s_unlockableItems.itemTable[itemNum].cost >= 0 )
            {
                s_unlockableItems.itemsInSlot[s_unlockableItems.numItemsInSlot++] = &s_unlockableItems.itemTable[itemNum];
            }
        }
    }
    qsort(s_unlockableItems.itemsInSlot, s_unlockableItems.numItemsInSlot, 4u, BG_UnlockablesCompareItemsBySortKey);
    return s_unlockableItems.numItemsInSlot;
}

int __cdecl BG_UnlockablesBuildItemListForGroupForWeaponTable(itemGroup_t group)
{
    int ItemDualWieldBaseIndex; // eax
    int itemNum; // [esp+0h] [ebp-4h]

    s_unlockableItems.numItemsInSlot = 0;
    if ( group != ITEMGROUP_COUNT )
    {
        for ( itemNum = 0; itemNum <= 256; ++itemNum )
        {
            if ( s_unlockableItems.itemTable[itemNum].isValid && s_unlockableItems.itemTable[itemNum].group == group )
            {
                if ( BG_UnlockablesItemIsDualWield(s_unlockableItems.itemTable[itemNum].index) )
                {
                    ItemDualWieldBaseIndex = BG_UnlockablesGetItemDualWieldBaseIndex(s_unlockableItems.itemTable[itemNum].index);
                    if ( BG_UnlockablesGetItemDualWieldCost(ItemDualWieldBaseIndex) < 0 )
                        break;
                }
                else if ( s_unlockableItems.itemTable[itemNum].cost < 0 )
                {
                    break;
                }
                s_unlockableItems.itemsInSlot[s_unlockableItems.numItemsInSlot++] = &s_unlockableItems.itemTable[itemNum];
            }
        }
    }
    qsort(s_unlockableItems.itemsInSlot, s_unlockableItems.numItemsInSlot, 4u, BG_UnlockablesCompareItemsBySortKey);
    return s_unlockableItems.numItemsInSlot;
}

int __cdecl BG_UnlockablesGetGroupItemCount()
{
    return s_unlockableItems.numItemsInSlot;
}

bool __cdecl BG_UnlockablesIsItemClassified(int itemIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    return itemInfo && itemInfo->unclassifyAt > s_unlockableItems.numPurchasedItemsInSlot;
}

bool __cdecl BG_UnlockablesIsItemClassifiedGeneric(int controllerIndex, int itemIndex)
{
    int currItem; // [esp+0h] [ebp-10h]
    int numPurchasedItemsInItemGroup; // [esp+4h] [ebp-Ch]
    itemInfo_t *itemInfo; // [esp+8h] [ebp-8h]
    itemInfo_t *currItemInfo; // [esp+Ch] [ebp-4h]

    if ( !BG_UnlockablesIsItemValidNotNull(itemIndex) )
        return 0;
    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    numPurchasedItemsInItemGroup = 0;
    if ( !itemInfo || itemInfo->cost == -1 )
        return 0;
    if ( !itemInfo->unclassifyAt )
        return 0;
    for ( currItem = 0; currItem < 256; ++currItem )
    {
        if ( BG_UnlockablesIsItemValidNotNull(currItem) )
        {
            currItemInfo = BG_UnlockablesGetItemInfo(currItem);
            if ( currItemInfo )
            {
                if ( currItemInfo->isValid
                    && !currItemInfo->isNullItem
                    && currItemInfo->cost != -1
                    && itemInfo->loadoutSlot == currItemInfo->loadoutSlot
                    && itemInfo->group == currItemInfo->group
                    && BG_UnlockablesIsItemPurchased(controllerIndex, currItem) )
                {
                    ++numPurchasedItemsInItemGroup;
                }
            }
        }
    }
    return itemInfo->unclassifyAt > numPurchasedItemsInItemGroup;
}

int __cdecl BG_UnlockablesGetNumPurchasesBeforeDeclassified(int itemIndex)
{
    int numPurchasesBeforeDeclassified; // [esp+0h] [ebp-8h]
    itemInfo_t *itemInfo; // [esp+4h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( !itemInfo )
        return 0;
    numPurchasesBeforeDeclassified = itemInfo->unclassifyAt - s_unlockableItems.numPurchasedItemsInSlot;
    if ( numPurchasesBeforeDeclassified < 0 )
        return 0;
    return numPurchasesBeforeDeclassified;
}

int __cdecl BG_UnlockablesGetItemIndexInSlot(int itemNum)
{
    if ( itemNum >= s_unlockableItems.numItemsInSlot
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    4258,
                    0,
                    "%s",
                    "itemNum < s_unlockableItems.numItemsInSlot") )
    {
        __debugbreak();
    }
    if ( !s_unlockableItems.itemsInSlot[itemNum]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    4259,
                    0,
                    "%s",
                    "s_unlockableItems.itemsInSlot[ itemNum ]") )
    {
        __debugbreak();
    }
    if ( itemNum < s_unlockableItems.numItemsInSlot && s_unlockableItems.itemsInSlot[itemNum] )
        return s_unlockableItems.itemsInSlot[itemNum]->index;
    else
        return -1;
}

void __cdecl BG_UnlockablesClearWeaponInfo(int controllerIndex, int itemIndex)
{
    int optionNum; // [esp+0h] [ebp-8h]
    int attachmentNum; // [esp+4h] [ebp-4h]

    if ( itemIndex < 64 )
    {
        for ( attachmentNum = 0; attachmentNum < 24; ++attachmentNum )
            BG_UnlockablesSetItemAttachmentPurchased(controllerIndex, itemIndex, attachmentNum, 0);
        for ( optionNum = 0; optionNum < 32; ++optionNum )
            BG_UnlockablesSetItemOptionPurchased(controllerIndex, itemIndex, optionNum, 0);
    }
    BG_UnlockablesSetItemPurchased(controllerIndex, itemIndex, 0);
    BG_UnlockablesSetProItemUnlocked(controllerIndex, itemIndex, 0);
}

char __cdecl BG_UnlockablesSetItemOptionPurchased(
                int controllerIndex,
                int itemIndex,
                int optionIndex,
                bool isPurchased)
{
    const ddlState_t *RootDDLState; // eax
    char *v6; // [esp-Ch] [ebp-24h]
    persistentStats *buffer; // [esp+0h] [ebp-18h]
    ddlState_t searchState; // [esp+8h] [ebp-10h] BYREF

    va("%d", optionIndex);
    v6 = va("%d", itemIndex);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 4, "weaponStats", v6) )
    {
        buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        if ( DDL_SetInt(&searchState, isPurchased, (char *)buffer) )
        {
            return 1;
        }
        else
        {
            DDL_PrintError("DDL: Error could not set purchased flag for item %d option %d\n", itemIndex, optionIndex);
            return 0;
        }
    }
    else
    {
        DDL_PrintError("DDL: Error could not find item %d option %d\n", itemIndex, optionIndex);
        return 0;
    }
}

char __cdecl BG_UnlockablesSetProItemUnlocked(int controllerIndex, int itemIndex, bool isUnlocked)
{
    int proChallenges; // [esp+0h] [ebp-1Ch]
    persistentStats *buffer; // [esp+4h] [ebp-18h]
    ddlState_t searchState; // [esp+Ch] [ebp-10h] BYREF

    buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_GLOBAL, 1);
    for ( proChallenges = 0; proChallenges < 3; ++proChallenges )
    {
        if ( !LiveStats_MoveToProItemVersion(&searchState, itemIndex, proChallenges) )
            return 0;
        if ( !DDL_SetInt(&searchState, isUnlocked, (char *)buffer) )
            return 0;
    }
    return 1;
}

bool __cdecl BG_UnlockablesIsWeaponOptionLocked(int controllerIndex, int weaponOptionIndex)
{
    bool result; // al
    int Rank; // esi
    int Prestige; // esi

    if ( BG_UnlockablesAllItemsUnlocked() )
        return 0;
    Rank = LiveStats_GetRank(controllerIndex);
    result = 1;
    if ( Rank >= BG_GetWeaponOptionUnlockLvl(weaponOptionIndex) )
    {
        Prestige = LiveStats_GetPrestige(controllerIndex);
        if ( Prestige >= BG_GetWeaponOptionUnlockPLevel(weaponOptionIndex) )
            return 0;
    }
    return result;
}

bool __cdecl SV_CacValidate_IsItemPurchased(unsigned __int8 *cacBuffer, int itemIndex, int pLevel, int rank)
{
    const ddlState_t *RootDDLState; // eax
    char *v6; // [esp-8h] [ebp-20h]
    itemInfo_t *itemInfo; // [esp+4h] [ebp-14h]
    ddlState_t searchState; // [esp+8h] [ebp-10h] BYREF

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( !itemInfo )
        return 0;
    if ( rank < 0 )
    {
        if ( itemInfo->cost <= 0 )
            return 1;
    }
    else if ( itemInfo->cost <= 0 && !BG_UnlockablesIsItemLockedForRank(pLevel, rank, itemIndex) )
    {
        return 1;
    }
    v6 = va("%d", itemIndex);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 3, "itemStats", v6) )
        return DDL_GetInt(&searchState, (char *)cacBuffer) != 0;
    DDL_PrintError("DDL: Error could not find item %d\n", itemIndex);
    return 0;
}

bool __cdecl SV_CACValidate_IsItemAttachmentPurchased(
                unsigned __int8 *cacbuffer,
                int itemIndex,
                int attachmentNum,
                int pLevel,
                int rank)
{
    eAttachment ItemAttachment; // eax
    const ddlState_t *RootDDLState; // eax
    char *v8; // [esp-Ch] [ebp-24h]
    ddlState_t searchState; // [esp+4h] [ebp-14h] BYREF
    itemInfo_t *itemInfo; // [esp+14h] [ebp-4h]

    if ( BG_UnlockablesAllItemsFree() )
        return 1;
    if ( !SV_CacValidate_IsItemPurchased(cacbuffer, itemIndex, pLevel, rank) )
        return 0;
    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( !itemInfo )
        return 0;
    if ( attachmentNum >= itemInfo->numAttachments[0] )
        return 0;
    ItemAttachment = BG_UnlockablesGetItemAttachment(itemIndex, attachmentNum);
    if ( !BG_GetAttachmentCost(ItemAttachment) )
        return 1;
    va("%d", attachmentNum);
    v8 = va("%d", itemIndex);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 4, "weaponStats", v8) )
        return DDL_GetInt(&searchState, (char *)cacbuffer) != 0;
    DDL_PrintError("DDL: Error could not find item %d attachment %d\n", itemIndex, attachmentNum);
    return 0;
}

bool __cdecl SV_CACValidate_IsItemOptionPurchased(
                unsigned __int8 *cacbuffer,
                int itemIndex,
                int optionIndex,
                int pLevel,
                int rank)
{
    const ddlState_t *RootDDLState; // eax
    char *v7; // [esp-Ch] [ebp-24h]
    ddlState_t searchState; // [esp+4h] [ebp-14h] BYREF

    if ( BG_UnlockablesAllItemsFree() )
        return 1;
    if ( !optionIndex )
        return 1;
    if ( optionIndex >= BG_GetWeaponOptionCount()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    4749,
                    0,
                    "%s",
                    "optionIndex < BG_GetWeaponOptionCount()") )
    {
        __debugbreak();
    }
    if ( optionIndex >= BG_GetWeaponOptionCount() )
        return 0;
    if ( !SV_CacValidate_IsItemPurchased(cacbuffer, itemIndex, pLevel, rank) )
        return 0;
    if ( !BG_UnlockablesGetItemInfo(itemIndex) )
        return 0;
    if ( !BG_GetWeaponOptionCost(optionIndex) )
        return 1;
    va("%d", optionIndex);
    v7 = va("%d", itemIndex);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 4, "weaponStats", v7) )
        return DDL_GetInt(&searchState, (char *)cacbuffer) != 0;
    DDL_PrintError("DDL: Error could not find item %d camo %d\n", itemIndex, optionIndex);
    return 0;
}

bool __cdecl SV_CACValidate_IsClanTagFeaturePurchased(
                unsigned __int8 *cacbuffer,
                unsigned int index,
                int plevel,
                int rank)
{
    const char *ClanTagFeature; // eax
    const ddlState_t *RootDDLState; // eax
    const char *v7; // [esp-8h] [ebp-24h]
    ddlState_t searchState; // [esp+4h] [ebp-18h] BYREF
    const char *featureName; // [esp+14h] [ebp-8h]

    if ( SV_CACValidate_IsClanTagFeatureLocked(rank, plevel, index) )
        return 0;
    ClanTagFeature = BG_UnlockablesGetClanTagFeature(index, CLANTAG_COL_COST);
    if ( !atoi(ClanTagFeature) )
        return 1;
    featureName = BG_UnlockablesGetClanTagFeature(index, CLANTAG_COL_NAME);
    if ( !featureName || !*featureName )
        return 0;
    v7 = featureName;
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 4, "clanTagStats", "features", v7, "purchased") )
        return DDL_GetInt(&searchState, (char *)cacbuffer) != 0;
    DDL_PrintError("DDL: Error could not find clan tag feature %s\n", featureName);
    return 0;
}

bool __cdecl SV_CACValidate_IsClanTagFeatureLocked(int rank, int plevel, unsigned int index)
{
    return BG_UnlockablesIsClanTagFeatureLockedForRank(plevel, rank, index);
}

void __cdecl SV_CACValidate_ClearWeaponInfo(char *buffer, int itemIndex)
{
    int optionNum; // [esp+0h] [ebp-8h]
    int attachmentNum; // [esp+4h] [ebp-4h]

    if ( itemIndex < 64 )
    {
        for ( attachmentNum = 0; attachmentNum < 24; ++attachmentNum )
            SV_CACValidate_SetItemAttachmentPurchased(buffer, itemIndex, attachmentNum, 0);
        for ( optionNum = 0; optionNum < 32; ++optionNum )
            SV_CACValidate_SetItemOptionPurchased(buffer, itemIndex, optionNum, 0);
    }
    SV_CACValidate_SetItemPurchased(buffer, itemIndex, 0);
    SV_CACValidate_SetProItemUnlocked(buffer, itemIndex, 0);
}

char __cdecl SV_CACValidate_SetProItemUnlocked(char *buffer, int itemIndex, bool isUnlocked)
{
    int proChallenges; // [esp+0h] [ebp-18h]
    ddlState_t searchState; // [esp+8h] [ebp-10h] BYREF

    for ( proChallenges = 0; proChallenges < 3; ++proChallenges )
    {
        if ( !LiveStats_MoveToProItemVersion(&searchState, itemIndex, proChallenges) )
            return 0;
        if ( !DDL_SetInt(&searchState, isUnlocked, buffer) )
            return 0;
    }
    return 1;
}

char __cdecl SV_CACValidate_SetItemPurchased(char *buffer, int itemIndex, bool isPurchased)
{
    const ddlState_t *RootDDLState; // eax
    char *v5; // [esp-8h] [ebp-1Ch]
    ddlState_t searchState; // [esp+4h] [ebp-10h] BYREF

    v5 = va("%d", itemIndex);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 3, "itemStats", v5) )
    {
        if ( DDL_SetInt(&searchState, isPurchased, buffer) )
        {
            return 1;
        }
        else
        {
            DDL_PrintError("DDL: Error could not set purchased flag for item %d\n", itemIndex);
            return 0;
        }
    }
    else
    {
        DDL_PrintError("DDL: Error could not find item %d\n", itemIndex);
        return 0;
    }
}

char __cdecl SV_CACValidate_SetItemAttachmentPurchased(
                char *buffer,
                int itemIndex,
                int attachmentNum,
                bool isPurchased)
{
    const ddlState_t *RootDDLState; // eax
    char *v6; // [esp-Ch] [ebp-20h]
    ddlState_t searchState; // [esp+4h] [ebp-10h] BYREF

    va("%d", attachmentNum);
    v6 = va("%d", itemIndex);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 4, "weaponStats", v6) )
    {
        if ( DDL_SetInt(&searchState, isPurchased, buffer) )
        {
            return 1;
        }
        else
        {
            DDL_PrintError("DDL: Error could not set purchased flag for item %d attachment %d\n", itemIndex, attachmentNum);
            return 0;
        }
    }
    else
    {
        DDL_PrintError("DDL: Error could not find item %d attachment %d\n", itemIndex, attachmentNum);
        return 0;
    }
}

char __cdecl SV_CACValidate_SetItemOptionPurchased(char *buffer, int itemIndex, int optionIndex, bool isPurchased)
{
    const ddlState_t *RootDDLState; // eax
    char *v6; // [esp-Ch] [ebp-20h]
    ddlState_t searchState; // [esp+4h] [ebp-10h] BYREF

    va("%d", optionIndex);
    v6 = va("%d", itemIndex);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 4, "weaponStats", v6) )
    {
        if ( DDL_SetInt(&searchState, isPurchased, buffer) )
        {
            return 1;
        }
        else
        {
            DDL_PrintError("DDL: Error could not set purchased flag for item %d option %d\n", itemIndex, optionIndex);
            return 0;
        }
    }
    else
    {
        DDL_PrintError("DDL: Error could not find item %d option %d\n", itemIndex, optionIndex);
        return 0;
    }
}

bool __cdecl SV_CACValidate_IsItemDualWieldPurchased(unsigned __int8 *cacbuffer, int itemIndex, int pLevel, int rank)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( !itemInfo )
        return 0;
    if ( itemInfo->dwIndex == -1 )
        return 0;
    return SV_CacValidate_IsItemPurchased(cacbuffer, itemInfo->dwIndex, pLevel, rank);
}

char __cdecl SV_CACValidate_CheckIfEquippedItemsArePurchased(unsigned __int8 *cacBuffer, int pLevel, int rank)
{
    const ddlState_t *CacDDLState; // eax
    const char *AttachmentName; // eax
    const char *WeaponOptionReference; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    ddlDef_t *StatsDDL; // eax
    const ddlState_t *v12; // eax
    const char *v13; // [esp-4h] [ebp-4Ch]
    const char *v14; // [esp+0h] [ebp-48h]
    int WeaponOptionNumFromIndexAndGroup; // [esp+4h] [ebp-44h]
    int weaponOption; // [esp+8h] [ebp-40h]
    int optionIndex; // [esp+Ch] [ebp-3Ch]
    int actualoptionidx; // [esp+10h] [ebp-38h]
    eAttachment attachment; // [esp+14h] [ebp-34h]
    itemInfo_t *itemInfo; // [esp+18h] [ebp-30h]
    itemInfo_t *itemInfoa; // [esp+18h] [ebp-30h]
    const char *customclassname; // [esp+20h] [ebp-28h]
    int j; // [esp+24h] [ebp-24h]
    ddlState_t searchState; // [esp+2Ch] [ebp-1Ch] BYREF
    int itemIndex; // [esp+3Ch] [ebp-Ch]
    int currentItemIndex; // [esp+40h] [ebp-8h]
    int i; // [esp+44h] [ebp-4h]

    currentItemIndex = 0;
    itemIndex = 0;
    i = 0;
LABEL_2:
    if ( (unsigned int)i < 0xA )
    {
        customclassname = s_customClassNames[i];
        for ( j = 0; ; ++j )
        {
            if ( j >= 35 )
            {
                ++i;
                goto LABEL_2;
            }
            v13 = s_loadoutNames[j];
            CacDDLState = LiveStats_GetCacDDLState();
            if ( DDL_MoveTo(CacDDLState, &searchState, 2, customclassname, v13) )
            {
                itemIndex = DDL_GetInt(&searchState, (char *)cacBuffer);
                if ( itemIndex > 0 )
                {
                    if ( j && j != 12 )
                    {
                        if ( (j < 1 || j > 5) && (j < 13 || j > 17) )
                        {
                            if ( j == 7 || j == 19 )
                            {
                                actualoptionidx = BG_GetWeaponOptionNumFromIndexAndGroup(itemIndex, WEAPONOPTION_GROUP_RETICLE);
                                if ( !SV_CACValidate_IsItemOptionPurchased(cacBuffer, currentItemIndex, actualoptionidx, pLevel, rank) )
                                {
                                    WeaponOptionReference = BG_GetWeaponOptionReference(actualoptionidx);
                                    Com_DPrintf(
                                        15,
                                        "TREASON UNCLOAKED: User claims %s option, but not purchased!\n",
                                        WeaponOptionReference);
                                    return 0;
                                }
                            }
                            else if ( j == 9 || j == 21 )
                            {
                                optionIndex = BG_GetWeaponOptionNumFromIndexAndGroup(itemIndex, WEAPONOPTION_GROUP_LENS);
                                if ( !SV_CACValidate_IsItemOptionPurchased(cacBuffer, currentItemIndex, optionIndex, pLevel, rank) )
                                {
                                    v7 = BG_GetWeaponOptionReference(optionIndex);
                                    Com_DPrintf(15, "TREASON UNCLOAKED: User claims %s option, but not purchased!\n", v7);
                                    return 0;
                                }
                            }
                            else if ( j == 6 || j == 18 )
                            {
                                if ( !SV_CACValidate_IsItemOptionPurchased(cacBuffer, currentItemIndex, itemIndex, pLevel, rank) )
                                {
                                    v8 = BG_GetWeaponOptionReference(itemIndex);
                                    Com_DPrintf(15, "TREASON UNCLOAKED: User claims %s option, but not purchased!\n", v8);
                                    return 0;
                                }
                            }
                            else if ( j == 8 || j == 20 )
                            {
                                weaponOption = BG_GetWeaponOptionNumFromIndexAndGroup(itemIndex, WEAPONOPTION_GROUP_RETICLE_COLOR);
                                if ( !SV_CACValidate_IsItemOptionPurchased(cacBuffer, currentItemIndex, weaponOption, pLevel, rank) )
                                {
                                    v9 = BG_GetWeaponOptionReference(weaponOption);
                                    Com_DPrintf(15, "TREASON UNCLOAKED: User claims %s option, but not purchased!\n", v9);
                                    return 0;
                                }
                            }
                            else if ( j == 10 || j == 22 )
                            {
                                WeaponOptionNumFromIndexAndGroup = BG_GetWeaponOptionNumFromIndexAndGroup(
                                                                                                         itemIndex,
                                                                                                         WEAPONOPTION_GROUP_EMBLEM);
                                if ( !SV_CACValidate_IsItemOptionPurchased(
                                                cacBuffer,
                                                currentItemIndex,
                                                WeaponOptionNumFromIndexAndGroup,
                                                pLevel,
                                                rank) )
                                {
                                    v10 = BG_GetWeaponOptionReference(WeaponOptionNumFromIndexAndGroup);
                                    Com_DPrintf(15, "TREASON UNCLOAKED: User claims %s option, but not purchased!\n", v10);
                                    return 0;
                                }
                            }
                            else
                            {
                                if ( j == 33 )
                                    itemIndex = BG_UnlockablesGetFaceCamoItemIndex(itemIndex);
                                itemInfoa = BG_UnlockablesGetItemInfo(itemIndex);
                                if ( itemInfoa->loadoutSlot != LOADOUTSLOT_INVALID
                                    && !SV_CacValidate_IsItemPurchased(cacBuffer, itemIndex, pLevel, rank) )
                                {
                                    if ( itemInfoa )
                                        Com_Printf(0, "User claims item %s, but it's not purchased!\n", itemInfoa->name);
                                    else
                                        Com_PrintError(0, "SV_CACValidate_CheckIfEquippedItemsArePurchased(): iteminfo is NULL!\n");
                                    return 0;
                                }
                            }
                        }
                        else
                        {
                            attachment = BG_UnlockablesGetItemAttachment(currentItemIndex, itemIndex);
                            if ( attachment )
                            {
                                if ( attachment == ATTACHMENT_DW )
                                {
                                    if ( !SV_CACValidate_IsItemDualWieldPurchased(cacBuffer, currentItemIndex, pLevel, rank) )
                                        goto LABEL_23;
                                }
                                else if ( !SV_CACValidate_IsItemAttachmentPurchased(
                                                         cacBuffer,
                                                         currentItemIndex,
                                                         itemIndex,
                                                         pLevel,
                                                         rank) )
                                {
LABEL_23:
                                    AttachmentName = BG_GetAttachmentName(attachment);
                                    Com_DPrintf(
                                        15,
                                        "TREASON UNCLOAKED! User claims to have attachment %s equipped, but it's not purchased!\n",
                                        AttachmentName);
                                    return 0;
                                }
                            }
                        }
                    }
                    else
                    {
                        currentItemIndex = itemIndex;
                        if ( !SV_CacValidate_IsItemPurchased(cacBuffer, itemIndex, pLevel, rank) )
                        {
                            itemInfo = BG_UnlockablesGetItemInfo(currentItemIndex);
                            if ( itemInfo )
                                Com_Printf(0, "User claims item %s, but it's not purchased!\n", itemInfo->name);
                            else
                                Com_PrintError(0, "SV_CACValidate_CheckIfEquippedItemsArePurchased(): iteminfo is NULL!\n");
                            return 0;
                        }
                    }
                }
            }
        }
    }
    for ( i = 35; i < 39; ++i )
    {
        v14 = s_loadoutNames[i];
        StatsDDL = LiveStats_GetStatsDDL();
        DDL_Reset(&searchState, StatsDDL);
        v12 = LiveStats_GetCacDDLState();
        if ( DDL_MoveTo(v12, &searchState, 1, v14) )
        {
            itemIndex = DDL_GetInt(&searchState, (char *)cacBuffer);
            if ( itemIndex > 0 && !SV_CacValidate_IsItemPurchased(cacBuffer, itemIndex, pLevel, rank) )
                return 0;
        }
    }
    return 1;
}

void __cdecl SV_SetClanTagFeatureNewForClient(char *buffer, int clientRank, int clientPrestige, unsigned int index)
{
    const ddlState_t *RootDDLState; // eax
    const char *v5; // [esp-8h] [ebp-1Ch]
    ddlState_t searchState; // [esp+0h] [ebp-14h] BYREF
    const char *featureName; // [esp+10h] [ebp-4h]

    if ( !BG_UnlockablesIsClanTagFeatureLockedForRank(clientPrestige, clientRank, index) )
    {
        featureName = BG_UnlockablesGetClanTagFeature(index, CLANTAG_COL_NAME);
        if ( featureName )
        {
            if ( *featureName )
            {
                v5 = featureName;
                RootDDLState = LiveStats_GetRootDDLState();
                if ( DDL_MoveTo(RootDDLState, &searchState, 4, "clanTagStats", "features", v5, "new") )
                {
                    if ( DDL_SetInt(&searchState, 1u, buffer) )
                        DDL_PrintError("DDL: Error could not set new flag for clan tag feature %s\n", featureName);
                }
                else
                {
                    DDL_PrintError("DDL: Error could not find clan tag feature %s\n", featureName);
                }
            }
        }
    }
}

cmd_function_s BG_UnlockablesEquipClassCmd_VAR;
cmd_function_s BG_UnlockablesEquipClassAttachmentCmd_VAR;
cmd_function_s BG_UnlockablesEquipClassCurrentAttachmentCmd_VAR;
cmd_function_s BG_UnlockablesEquipClassCurrentItemCmd_VAR;
cmd_function_s BG_UnlockablesEquipClassCurrentGlobalItemCmd_VAR;
cmd_function_s BG_UnlockablesEquipDefaultClassCmd_VAR;
cmd_function_s BG_UnlockablesEquipDefaultItemToSlotCmd_VAR;
cmd_function_s BG_UnlockablesEquipDefaultGlobalItemToSlotCmd_VAR;
cmd_function_s BG_UnlockablesEquipDefaultGlobalItemsCmd_VAR;
cmd_function_s BG_UnlockablesClearItemNewCmd_VAR;
cmd_function_s BG_UnlockablesClearAllItemsNewCmd_VAR;
cmd_function_s BG_UnlockablesPurchaseCurrentItemCmd_VAR;
cmd_function_s BG_UnlockablesPurchaseCurrentProItemCmd_VAR;
cmd_function_s BG_UnlockablesSellCurrentItemCmd_VAR;
cmd_function_s BG_UnlockablesPurchaseCurrentItemAttachmentCmd_VAR;
cmd_function_s BG_UnlockablesPurchaseItemAttachmentPointCmd_VAR;
cmd_function_s OpenAttachmentMenuForCurrentItemCmd_VAR;
cmd_function_s BG_ClearAllAttachmentsForItemCmd_VAR;
cmd_function_s BG_ClearAttachmentForItemAtPointCmd_VAR;
cmd_function_s BG_UnlockablesEquipItemInSlotSortedCmd_VAR;
cmd_function_s BG_UnlockablesPurchaseCurrentItemOptionCmd_VAR;
cmd_function_s BG_UnlockablesEquipClassCurrentOptionCmd_VAR;
cmd_function_s BG_UnlockablesEquipGlobalItemInSlotSortedCmd_VAR;
cmd_function_s BG_UnlockablesSetCurrentItemIndexCmd_VAR;
cmd_function_s BG_UnlockablesEquipClassItemCmd_VAR;
cmd_function_s BG_UnlockablesPurchaseItemCmd_VAR;
cmd_function_s BG_UnlockablesBuildItemListForSlotNameCmd_VAR;
cmd_function_s BG_UnlockablesBuildItemListForChallengesPerksCmd_VAR;
cmd_function_s BG_UnlockablesBuildItemListForGroupCmd_VAR;
cmd_function_s BG_UnlockablesBuildItemListForGroupForWeaponTableCmd_VAR;
cmd_function_s BG_UnlockablesBuildItemListForGroupNameCmd_VAR;
cmd_function_s BG_UnlockablesBuildItemListForSlotNameAndGroupCmd_VAR;
cmd_function_s BG_UnlockablesPurchaseClanTagFeatureCmd_VAR;
cmd_function_s BG_UnlockablesToggleWeaponOptionCmd_VAR;
cmd_function_s BG_UnlockablesPurchaseWeaponOptionCmd_VAR;
cmd_function_s BG_UnlockablesCopyClassCmd_VAR;
cmd_function_s BG_UnlockablesSetClanTagCmd_VAR;

void __cdecl BG_UnlockableItemsInit()
{
    int numPurchasedItemsInSlot; // [esp+1Ch] [ebp-40Ch]
    itemInfo_t *itemsInSlot[256]; // [esp+20h] [ebp-408h] BYREF
    int numItemsInSlot; // [esp+424h] [ebp-4h]

    Cmd_AddCommandInternal("equipClass", BG_UnlockablesEquipClassCmd, &BG_UnlockablesEquipClassCmd_VAR);
    Cmd_AddCommandInternal(
        "equipClassAttachment",
        BG_UnlockablesEquipClassAttachmentCmd,
        &BG_UnlockablesEquipClassAttachmentCmd_VAR);
    Cmd_AddCommandInternal(
        "equipClassCurrentAttachment",
        BG_UnlockablesEquipClassCurrentAttachmentCmd,
        &BG_UnlockablesEquipClassCurrentAttachmentCmd_VAR);
    Cmd_AddCommandInternal(
        "equipClassCurrentItem",
        BG_UnlockablesEquipClassCurrentItemCmd,
        &BG_UnlockablesEquipClassCurrentItemCmd_VAR);
    Cmd_AddCommandInternal(
        "equipClassCurrentGlobalItem",
        BG_UnlockablesEquipClassCurrentGlobalItemCmd,
        &BG_UnlockablesEquipClassCurrentGlobalItemCmd_VAR);
    Cmd_AddCommandInternal(
        "equipDefaultClass",
        BG_UnlockablesEquipDefaultClassCmd,
        &BG_UnlockablesEquipDefaultClassCmd_VAR);
    Cmd_AddCommandInternal(
        "equipDefaultItemToSlot",
        BG_UnlockablesEquipDefaultItemToSlotCmd,
        &BG_UnlockablesEquipDefaultItemToSlotCmd_VAR);
    Cmd_AddCommandInternal(
        "equipDefaultGlobalItemToSlot",
        BG_UnlockablesEquipDefaultGlobalItemToSlotCmd,
        &BG_UnlockablesEquipDefaultGlobalItemToSlotCmd_VAR);
    Cmd_AddCommandInternal(
        "equipDefaultGlobalItems",
        BG_UnlockablesEquipDefaultGlobalItemsCmd,
        &BG_UnlockablesEquipDefaultGlobalItemsCmd_VAR);
    Cmd_AddCommandInternal("clearItemNew", BG_UnlockablesClearItemNewCmd, &BG_UnlockablesClearItemNewCmd_VAR);
    Cmd_AddCommandInternal("clearAllItemsNew", BG_UnlockablesClearAllItemsNewCmd, &BG_UnlockablesClearAllItemsNewCmd_VAR);
    Cmd_AddCommandInternal(
        "purchasecurrentitem",
        BG_UnlockablesPurchaseCurrentItemCmd,
        &BG_UnlockablesPurchaseCurrentItemCmd_VAR);
    Cmd_AddCommandInternal(
        "purchasecurrentproitem",
        BG_UnlockablesPurchaseCurrentProItemCmd,
        &BG_UnlockablesPurchaseCurrentProItemCmd_VAR);
    Cmd_AddCommandInternal("sellcurrentitem", BG_UnlockablesSellCurrentItemCmd, &BG_UnlockablesSellCurrentItemCmd_VAR);
    Cmd_AddCommandInternal(
        "purchasecurrentitemattachment",
        BG_UnlockablesPurchaseCurrentItemAttachmentCmd,
        &BG_UnlockablesPurchaseCurrentItemAttachmentCmd_VAR);
    Cmd_AddCommandInternal(
        "purchaseitemattachmentpoint",
        BG_UnlockablesPurchaseItemAttachmentPointCmd,
        &BG_UnlockablesPurchaseItemAttachmentPointCmd_VAR);
    Cmd_AddCommandInternal(
        "openAttachmentMenuForCurrentItem",
        OpenAttachmentMenuForCurrentItemCmd,
        &OpenAttachmentMenuForCurrentItemCmd_VAR);
    Cmd_AddCommandInternal(
        "clearAllAttachmentsForItem",
        BG_ClearAllAttachmentsForItemCmd,
        &BG_ClearAllAttachmentsForItemCmd_VAR);
    Cmd_AddCommandInternal(
        "clearAttachmentForItemAtPoint",
        BG_ClearAttachmentForItemAtPointCmd,
        &BG_ClearAttachmentForItemAtPointCmd_VAR);
    Cmd_AddCommandInternal(
        "equipItemInSlotSorted",
        BG_UnlockablesEquipItemInSlotSortedCmd,
        &BG_UnlockablesEquipItemInSlotSortedCmd_VAR);
    Cmd_AddCommandInternal(
        "purchasecurrentitemoption",
        BG_UnlockablesPurchaseCurrentItemOptionCmd,
        &BG_UnlockablesPurchaseCurrentItemOptionCmd_VAR);
    Cmd_AddCommandInternal(
        "equipClassCurrentOption",
        BG_UnlockablesEquipClassCurrentOptionCmd,
        &BG_UnlockablesEquipClassCurrentOptionCmd_VAR);
    Cmd_AddCommandInternal(
        "equipGlobalItemInSlotSorted",
        BG_UnlockablesEquipGlobalItemInSlotSortedCmd,
        &BG_UnlockablesEquipGlobalItemInSlotSortedCmd_VAR);
    Cmd_AddCommandInternal(
        "setCurrentItemIndex",
        BG_UnlockablesSetCurrentItemIndexCmd,
        &BG_UnlockablesSetCurrentItemIndexCmd_VAR);
    Cmd_AddCommandInternal("equipClassItem", BG_UnlockablesEquipClassItemCmd, &BG_UnlockablesEquipClassItemCmd_VAR);
    Cmd_AddCommandInternal("purchaseItem", BG_UnlockablesPurchaseItemCmd, &BG_UnlockablesPurchaseItemCmd_VAR);
    Cmd_AddCommandInternal(
        "buildItemListForSlotName",
        BG_UnlockablesBuildItemListForSlotNameCmd,
        &BG_UnlockablesBuildItemListForSlotNameCmd_VAR);
    Cmd_AddCommandInternal(
        "buildItemListForChallengesPerks",
        BG_UnlockablesBuildItemListForChallengesPerksCmd,
        &BG_UnlockablesBuildItemListForChallengesPerksCmd_VAR);
    Cmd_AddCommandInternal(
        "buildItemListForGroup",
        BG_UnlockablesBuildItemListForGroupCmd,
        &BG_UnlockablesBuildItemListForGroupCmd_VAR);
    Cmd_AddCommandInternal(
        "buildItemListForGroupForWeaponTable",
        BG_UnlockablesBuildItemListForGroupForWeaponTableCmd,
        &BG_UnlockablesBuildItemListForGroupForWeaponTableCmd_VAR);
    Cmd_AddCommandInternal(
        "buildItemListForGroupName",
        BG_UnlockablesBuildItemListForGroupNameCmd,
        &BG_UnlockablesBuildItemListForGroupNameCmd_VAR);
    Cmd_AddCommandInternal(
        "buildItemListForSlotNameAndGroup",
        BG_UnlockablesBuildItemListForSlotNameAndGroupCmd,
        &BG_UnlockablesBuildItemListForSlotNameAndGroupCmd_VAR);
    Cmd_AddCommandInternal(
        "PurchaseClanTagFeature",
        BG_UnlockablesPurchaseClanTagFeatureCmd,
        &BG_UnlockablesPurchaseClanTagFeatureCmd_VAR);
    Cmd_AddCommandInternal(
        "ToggleWeaponOption",
        BG_UnlockablesToggleWeaponOptionCmd,
        &BG_UnlockablesToggleWeaponOptionCmd_VAR);
    Cmd_AddCommandInternal(
        "PurchaseWeaponOption",
        BG_UnlockablesPurchaseWeaponOptionCmd,
        &BG_UnlockablesPurchaseWeaponOptionCmd_VAR);
    Cmd_AddCommandInternal("copyclass", BG_UnlockablesCopyClassCmd, &BG_UnlockablesCopyClassCmd_VAR);
    Cmd_AddCommandInternal("setClanTag", BG_UnlockablesSetClanTagCmd, &BG_UnlockablesSetClanTagCmd_VAR);
    attachmentFilter = _Dvar_RegisterInt("attachmentFilter", 0, 0, 255, 0, "attachmentFilter");
    maxAttachmentsPerWeapon = _Dvar_RegisterInt(
                                                            "maxAttachmentsPerWeapon",
                                                            2,
                                                            0,
                                                            255,
                                                            0,
                                                            "Max attachments allowed for a weapon");
    allItemsUnlocked = _Dvar_RegisterBool(
                                             "allItemsUnlocked",
                                             0,
                                             0x80u,
                                             "Allows all items to be purchased in the Black Market");
    allItemsPurchased = _Dvar_RegisterBool(
                                                "allItemsPurchased",
                                                0,
                                                0x80u,
                                                "Allows all items to be equiped in Create-a-Class");
    itemSellFactor = _Dvar_RegisterFloat(
                                         "itemSellFactor",
                                         0.5,
                                         0.0,
                                         1.0,
                                         0,
                                         "Factor by which to multiply the item cost to get the item sell price.");
    ui_currentWeaponOptionGroup = _Dvar_RegisterInt(
                                                                    "ui_currentWeaponOptionGroup",
                                                                    0,
                                                                    0,
                                                                    5,
                                                                    0,
                                                                    "Current weapon option group for the UI");
    bodyTypeFromGun = _Dvar_RegisterBool(
                                            "bodyTypeFromGun",
                                            0,
                                            0,
                                            "Associate the body type with the primary weapon (perk1 if false)");
    BG_LoadWeaponAttachmentTable();
    memcpy(itemsInSlot, s_unlockableItems.itemsInSlot, sizeof(itemsInSlot));
    numItemsInSlot = s_unlockableItems.numItemsInSlot;
    numPurchasedItemsInSlot = s_unlockableItems.numPurchasedItemsInSlot;
    BG_InitUnlockables();
    memcpy(s_unlockableItems.itemsInSlot, itemsInSlot, sizeof(s_unlockableItems.itemsInSlot));
    s_unlockableItems.numItemsInSlot = numItemsInSlot;
    s_unlockableItems.numPurchasedItemsInSlot = numPurchasedItemsInSlot;
}

void __cdecl BG_UnlockablesPurchaseClanTagFeatureCmd()
{
    BG_UnlockablesPurchaseClanTagFeature(0, sharedUiInfo.clanTagFeature);
}

void __cdecl BG_UnlockablesPurchaseCurrentItemAttachmentCmd()
{
    BG_UnlockablesPurchaseItemAttachment(0, sharedUiInfo.itemIndex, (eAttachment)sharedUiInfo.attachmentNum);
}

void __cdecl BG_UnlockablesPurchaseItemAttachmentPointCmd()
{
    const char *v0; // eax
    itemInfo_t *itemInfo; // [esp+1Ch] [ebp-10h]
    const char *attachmentPoint; // [esp+20h] [ebp-Ch]
    int itemIndex; // [esp+28h] [ebp-4h]

    if ( Cmd_Argc() >= 2 )
    {
        v0 = Cmd_Argv(1);
        itemIndex = atoi(v0);
        attachmentPoint = Cmd_Argv(2);
        itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
        if ( itemInfo )
        {
            if ( BG_GetAttachmentPointIndex(attachmentPoint) == 5 )
                Com_PrintError(15, "Item attachment point %s not found for item %s\n", attachmentPoint, itemInfo->name);
            else
                BG_UnlockablesPurchaseItemAttachmentPoint(0, itemIndex, attachmentPoint);
        }
    }
    else
    {
        Com_PrintError(15, "purchase item attachment point usage: statset <statname> <value>\n");
    }
}


void __cdecl BG_UnlockablesPurchaseCurrentItemCmd()
{
    int ItemCost; // eax

    ItemCost = BG_UnlockablesGetItemCost(sharedUiInfo.itemIndex);
    BG_UnlockablesPurchaseItem(0, sharedUiInfo.itemIndex, ItemCost);
}

void __cdecl BG_UnlockablesPurchaseItemCmd()
{
    const char *v0; // eax
    int ItemDualWieldBaseIndex; // eax
    int ItemDualWieldCost; // eax
    int itemIndex; // [esp+10h] [ebp-4h]

    if ( Cmd_Argc() >= 2 )
    {
        v0 = Cmd_Argv(1);
        itemIndex = atoi(v0);
        if ( itemIndex != -1 )
        {
            if ( BG_UnlockablesItemIsDualWield(itemIndex) )
            {
                ItemDualWieldBaseIndex = BG_UnlockablesGetItemDualWieldBaseIndex(itemIndex);
                ItemDualWieldCost = BG_UnlockablesGetItemDualWieldCost(ItemDualWieldBaseIndex);
            }
            else
            {
                ItemDualWieldCost = BG_UnlockablesGetItemCost(itemIndex);
            }
            BG_UnlockablesPurchaseItem(0, itemIndex, ItemDualWieldCost);
        }
    }
    else
    {
        Com_PrintError(15, "purchaseitem usage: purchaseitem <itemindex>\n");
    }
}

void __cdecl BG_UnlockablesSellCurrentItemCmd()
{
    BG_UnlockablesSellItem(0, sharedUiInfo.itemIndex);
}

void __cdecl BG_UnlockablesPurchaseWeaponOptionCmd()
{
    const char *v0; // eax
    const char *v1; // eax
    int optionNum; // [esp+18h] [ebp-4h]

    if ( Cmd_Argc() >= 2 )
    {
        v0 = Cmd_Argv(1);
        optionNum = BG_UnlockablesGetWeaponOptionFromName(v0);
        if ( optionNum == -1 )
        {
            v1 = Cmd_Argv(1);
            Com_PrintError(15, "weapon option %s not found\n", v1);
        }
        else
        {
            BG_UnlockablesPurchaseItemOption(0, sharedUiInfo.itemIndex, optionNum);
        }
    }
    else
    {
        Com_PrintError(15, "purchaseWeaponOption usage: purchaseWeaponOption <option>\n");
    }
}

void __cdecl BG_UnlockablesPurchaseItemOption(int controllerIndex, int itemIndex, int optionIndex)
{
    const char *WeaponOptionReference; // eax
    eWeaponOptionGroup WeaponOptionGroup; // [esp+0h] [ebp-14h]
    int cost; // [esp+8h] [ebp-Ch]
    int currentCodPoints; // [esp+Ch] [ebp-8h]
    itemInfo_t *itemInfo; // [esp+10h] [ebp-4h]

    if ( optionIndex )
    {
        if ( !BG_UnlockablesIsItemLocked(controllerIndex, itemIndex)
            && BG_UnlockablesIsItemPurchased(controllerIndex, itemIndex) )
        {
            itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
            if ( itemInfo )
            {
                if ( BG_UnlockablesIsItemOptionPurchased(controllerIndex, itemIndex, optionIndex) )
                {
                    Com_PrintError(15, "Option Index %d is arleady purchased for item %s\n", optionIndex, itemInfo->name);
                    return;
                }
                if ( optionIndex < BG_GetWeaponOptionCount() )
                {
                    currentCodPoints = LiveStats_GetCurrency(controllerIndex);
                    cost = BG_GetWeaponOptionCost(optionIndex);
                    if ( currentCodPoints < cost )
                    {
                        WeaponOptionReference = BG_GetWeaponOptionReference(optionIndex);
                        Com_PrintError(
                            15,
                            "Item camo %s costs %d and you only have %d\n",
                            WeaponOptionReference,
                            cost,
                            currentCodPoints);
                        return;
                    }
                    if ( BG_UnlockablesSetItemOptionPurchased(controllerIndex, itemIndex, optionIndex, 1) )
                    {
                        WeaponOptionGroup = BG_GetWeaponOptionGroup(optionIndex);
                        if ( WeaponOptionGroup )
                        {
                            if ( WeaponOptionGroup <= WEAPONOPTION_GROUP_FIRST || WeaponOptionGroup > WEAPONOPTION_GROUP_RETICLE_COLOR )
                                goto LABEL_18;
                        }
                        else
                        {
                            LiveCounter_IncrementCounterValueByName("global_camopurchased", 1u);
                        }
                        LiveCounter_IncrementCounterValueByName("global_purchasedguncustomization", 1u);
LABEL_18:
                        LiveStats_SpendCurrency(controllerIndex, cost, POINTSSPENT_UNLOCKABLE_ATTACHMENT_OPTION, itemIndex);
                    }
                }
            }
        }
    }
}

void __cdecl BG_UnlockablesPurchaseCurrentItemOptionCmd()
{
    int optionNum; // [esp+0h] [ebp-4h]

    optionNum = BG_GetWeaponOptionNumFromIndexAndGroup(
                                sharedUiInfo.sortedItemPivot,
                                (eWeaponOptionGroup)ui_currentWeaponOptionGroup->current.integer);
    BG_UnlockablesPurchaseItemOption(0, sharedUiInfo.itemIndex, optionNum);
}

void __cdecl BG_UnlockablesEquipClassCmd()
{
    const char *v0; // eax
    const char *customClassName; // [esp+18h] [ebp-10h]
    const char *itemName; // [esp+20h] [ebp-8h]
    int itemIndex; // [esp+24h] [ebp-4h]

    if ( Cmd_Argc() >= 3 )
    {
        customClassName = Cmd_Argv(1);
        itemName = Cmd_Argv(2);
        itemIndex = BG_UnlockablesGetItemIndexFromName(itemName);
        if ( itemIndex == -1 )
        {
            v0 = va("Item %s not found", itemName);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                            3142,
                            0,
                            "%s\n\t%s",
                            "itemIndex != ENTRY_NOT_FOUND",
                            v0) )
                __debugbreak();
        }
        if ( itemIndex != -1 && BG_UnlockablesIsItemPurchased(0, itemIndex) )
            BG_UnlockablesEquipClass(0, customClassName, itemIndex);
    }
    else
    {
        Com_PrintError(15, "equipclass usage: equipclass <customclassname> <item>\n");
    }
}

void __cdecl BG_UnlockablesEquipClass(int controllerIndex, const char *customClassName, int itemIndex)
{
    const char *loadoutName; // [esp+0h] [ebp-8h]
    itemInfo_t *itemInfo; // [esp+4h] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    if ( itemInfo )
    {
        if ( itemInfo->loadoutSlot == LOADOUTSLOT_INVALID )
        {
            Com_Printf(1, "Item %s has an invalid loadout slot\n", itemInfo->name);
        }
        else
        {
            loadoutName = BG_UnlockablesGetLoadoutName(itemInfo->loadoutSlot);
            if ( loadoutName )
                BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, itemIndex, loadoutName);
        }
    }
    else
    {
        Com_Printf(1, "Item %d is invalid\n", itemIndex);
    }
}

void __cdecl BG_UnlockablesEquipDefaultItemToSlotCmd()
{
    const char *v0; // eax
    int DefaultItem; // eax
    const char *customClassName; // [esp+18h] [ebp-10h]
    const char *loadoutName; // [esp+20h] [ebp-8h]
    loadoutSlot_t loadoutSlot; // [esp+24h] [ebp-4h]

    if ( Cmd_Argc() >= 3 )
    {
        customClassName = Cmd_Argv(1);
        loadoutName = Cmd_Argv(2);
        loadoutSlot = BG_UnlockablesGetLoadoutSlotFromString(loadoutName);
        if ( loadoutSlot == LOADOUTSLOT_INVALID )
        {
            v0 = va("Loadout slot %s not found\n", loadoutName);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                            3166,
                            0,
                            "%s\n\t%s",
                            "loadoutSlot != ENTRY_NOT_FOUND",
                            v0) )
                __debugbreak();
        }
        if ( loadoutSlot != LOADOUTSLOT_INVALID )
        {
            DefaultItem = BG_UnlockablesGetDefaultItem(loadoutSlot);
            BG_UnlockablesEquipClassToSlot(0, customClassName, DefaultItem, loadoutName);
        }
    }
    else
    {
        Com_PrintError(15, "equipdefaultitemtoslot usage: equipdefaultitemtoslot <customclassname> <slotname>\n");
    }
}

void __cdecl BG_UnlockablesEquipDefaultGlobalItemToSlotCmd()
{
    const char *v0; // eax
    int DefaultItem; // eax
    const char *loadoutName; // [esp+14h] [ebp-8h]
    loadoutSlot_t loadoutSlot; // [esp+18h] [ebp-4h]

    if ( Cmd_Argc() >= 2 )
    {
        loadoutName = Cmd_Argv(1);
        loadoutSlot = BG_UnlockablesGetLoadoutSlotFromString(loadoutName);
        if ( loadoutSlot == LOADOUTSLOT_INVALID )
        {
            v0 = va("Loadout slot %s not found\n", loadoutName);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                            3189,
                            0,
                            "%s\n\t%s",
                            "loadoutSlot != ENTRY_NOT_FOUND",
                            v0) )
                __debugbreak();
        }
        if ( loadoutSlot != LOADOUTSLOT_INVALID )
        {
            DefaultItem = BG_UnlockablesGetDefaultItem(loadoutSlot);
            BG_UnlockablesEquipClassToSlot(0, 0, DefaultItem, loadoutName);
        }
    }
    else
    {
        Com_PrintError(15, "equipdefaultglobalitemtoslot usage: equipdefaultglobalitemtoslot <slotname>\n");
    }
}

void __cdecl BG_UnlockablesEquipClassAttachmentCmd()
{
    const char *customClassName; // [esp+28h] [ebp-14h]
    const char *itemName; // [esp+30h] [ebp-Ch]
    const char *attachmentName; // [esp+34h] [ebp-8h]
    const char *attachPointName; // [esp+38h] [ebp-4h]

    if ( Cmd_Argc() >= 4 )
    {
        customClassName = Cmd_Argv(1);
        itemName = Cmd_Argv(2);
        attachmentName = Cmd_Argv(3);
        attachPointName = Cmd_Argv(4);
        BG_UnlockablesEquipClassAttachment(0, customClassName, itemName, attachmentName, attachPointName);
    }
    else
    {
        Com_PrintError(15, "equipclassattachment usage: equipclassattachment <customclassname> <item> <attachment>\n");
    }
}

void __cdecl BG_UnlockablesEquipClassAttachment(
                int controllerIndex,
                const char *customClassName,
                const char *itemName,
                const char *attachmentName,
                const char *attachmentPoint)
{
    const char *v5; // eax
    const char *AttachmentPointName; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    eAttachmentPoint currAttachPoint; // [esp+0h] [ebp-28h]
    int numAttachmentsEquipped; // [esp+4h] [ebp-24h]
    const char *loadoutName; // [esp+8h] [ebp-20h]
    unsigned int equippedAttachmentNum; // [esp+Ch] [ebp-1Ch]
    eAttachmentPoint attachPoint; // [esp+10h] [ebp-18h]
    eAttachment equippedAttachmentIndex; // [esp+14h] [ebp-14h]
    eAttachment attachmentIndex; // [esp+18h] [ebp-10h]
    int attachmentNum; // [esp+1Ch] [ebp-Ch]
    itemInfo_t *itemInfo; // [esp+20h] [ebp-8h]
    int itemIndex; // [esp+24h] [ebp-4h]

    itemIndex = BG_UnlockablesGetItemIndexFromName(itemName);
    if ( itemIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    3201,
                    0,
                    "%s",
                    "itemIndex != ENTRY_NOT_FOUND") )
    {
        __debugbreak();
    }
    if ( itemIndex != -1 && BG_UnlockablesIsItemPurchased(controllerIndex, itemIndex) )
    {
        itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
        if ( itemInfo->loadoutSlot == LOADOUTSLOT_INVALID
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                        3206,
                        0,
                        "%s",
                        "itemInfo->loadoutSlot != LOADOUTSLOT_INVALID") )
        {
            __debugbreak();
        }
        if ( itemInfo->loadoutSlot != LOADOUTSLOT_INVALID )
        {
            attachmentNum = BG_UnlockablesIsItemAttachmentValid(itemInfo, attachmentName);
            if ( attachmentNum == -1 )
            {
                v5 = va("attachment %s is not valid for item %s\n", attachmentName, itemInfo->name);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                                3211,
                                0,
                                "%s\n\t%s",
                                "attachmentNum != ENTRY_NOT_FOUND",
                                v5) )
                    __debugbreak();
            }
            if ( attachmentNum != -1 && BG_UnlockablesIsItemAttachmentPurchased(controllerIndex, itemIndex, attachmentNum) )
            {
                loadoutName = BG_UnlockablesGetLoadoutName(itemInfo->loadoutSlot);
                if ( loadoutName )
                {
                    BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, itemIndex, loadoutName);
                    numAttachmentsEquipped = 0;
                    attachmentIndex = itemInfo->attachments[attachmentNum];
                    attachPoint = BG_GetAttachmentPointIndexFromAttachment(attachmentIndex);
                    for ( currAttachPoint = ATTACHMENT_POINT_TOP; currAttachPoint < ATTACHMENT_POINT_COUNT; ++currAttachPoint )
                    {
                        AttachmentPointName = BG_GetAttachmentPointName(currAttachPoint);
                        v7 = va("%sattachment%s", loadoutName, AttachmentPointName);
                        equippedAttachmentNum = BG_UnlockablesGetEquippedItemInSlot(controllerIndex, customClassName, v7);
                        equippedAttachmentIndex = itemInfo->attachments[equippedAttachmentNum];
                        if ( equippedAttachmentNum == attachmentNum )
                        {
                            v8 = BG_GetAttachmentPointName(currAttachPoint);
                            v9 = va("%sattachment%s", loadoutName, v8);
                            BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, 0, v9);
                            return;
                        }
                        if ( equippedAttachmentIndex )
                        {
                            ++numAttachmentsEquipped;
                            if ( currAttachPoint == attachPoint )
                                return;
                        }
                        if ( !BG_AreAttachmentsCompatible(attachmentIndex, equippedAttachmentIndex) )
                            return;
                    }
                    if ( attachmentNum && numAttachmentsEquipped < maxAttachmentsPerWeapon->current.integer )
                    {
                        v10 = va("%sattachment%s", loadoutName, attachmentPoint);
                        BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, attachmentNum, v10);
                    }
                    else
                    {
                        v11 = BG_GetAttachmentPointName((eAttachmentPoint)attachmentFilter->current.integer);
                        v12 = va("%sattachment%s", loadoutName, v11);
                        BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, attachmentNum, v12);
                    }
                }
            }
        }
    }
}

void __cdecl BG_UnlockablesEquipClassCurrentAttachmentCmd()
{
    eAttachment ItemAttachment; // eax
    eAttachmentPoint AttachmentPointIndexFromAttachment; // eax
    const char *AttachmentPointName; // eax
    const char *attachmentRef; // [esp+10h] [ebp-10h]
    const char *customClassName; // [esp+14h] [ebp-Ch]
    itemInfo_t *itemInfo; // [esp+1Ch] [ebp-4h]

    if ( Cmd_Argc() >= 2 )
    {
        customClassName = Cmd_Argv(1);
        itemInfo = BG_UnlockablesGetItemInfo(sharedUiInfo.itemIndex);
        if ( !itemInfo
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                        3313,
                        0,
                        "%s",
                        "itemInfo") )
        {
            __debugbreak();
        }
        if ( itemInfo && sharedUiInfo.attachmentNum < itemInfo->numAttachments[0] )
        {
            attachmentRef = BG_UnlockablesGetItemAttachmentRef(sharedUiInfo.itemIndex, sharedUiInfo.attachmentNum);
            if ( (!attachmentRef || !*attachmentRef)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                            3318,
                            0,
                            "%s",
                            "attachmentRef && attachmentRef[ 0 ]") )
            {
                __debugbreak();
            }
            if ( (!itemInfo->name || !*itemInfo->name)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                            3319,
                            0,
                            "%s",
                            "itemInfo->name && itemInfo->name[ 0 ]") )
            {
                __debugbreak();
            }
            if ( attachmentRef && *attachmentRef && itemInfo->name )
            {
                if ( *itemInfo->name )
                {
                    ItemAttachment = BG_UnlockablesGetItemAttachment(sharedUiInfo.itemIndex, sharedUiInfo.attachmentNum);
                    AttachmentPointIndexFromAttachment = BG_GetAttachmentPointIndexFromAttachment(ItemAttachment);
                    AttachmentPointName = BG_GetAttachmentPointName(AttachmentPointIndexFromAttachment);
                    BG_UnlockablesEquipClassAttachment(0, customClassName, itemInfo->name, attachmentRef, AttachmentPointName);
                }
            }
        }
    }
    else
    {
        Com_PrintError(15, "equipclasscurrentattachment usage: equipclasscurrentattachment <customclassname>\n");
    }
}

void __cdecl BG_UnlockablesEquipClassCurrentOptionCmd()
{
    const char *v0; // eax
    const char *loadoutName; // [esp+18h] [ebp-18h]
    const char *customClassName; // [esp+1Ch] [ebp-14h]
    itemInfo_t *itemInfo; // [esp+24h] [ebp-Ch]
    int optionNum; // [esp+28h] [ebp-8h]
    const char *optionName; // [esp+2Ch] [ebp-4h]

    if ( Cmd_Argc() >= 3 )
    {
        customClassName = Cmd_Argv(1);
        optionName = Cmd_Argv(2);
        itemInfo = BG_UnlockablesGetItemInfo(sharedUiInfo.itemIndex);
        if ( !itemInfo
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                        3345,
                        0,
                        "%s",
                        "itemInfo") )
        {
            __debugbreak();
        }
        optionNum = BG_GetWeaponOptionNumFromIndexAndGroup(
                                    sharedUiInfo.sortedItemPivot,
                                    (eWeaponOptionGroup)ui_currentWeaponOptionGroup->current.integer);
        if ( itemInfo
            && BG_UnlockablesIsItemPurchased(0, sharedUiInfo.itemIndex)
            && BG_UnlockablesIsItemOptionPurchased(0, sharedUiInfo.itemIndex, optionNum) )
        {
            loadoutName = BG_UnlockablesGetLoadoutName(itemInfo->loadoutSlot);
            if ( loadoutName )
            {
                BG_UnlockablesEquipClassToSlot(0, customClassName, sharedUiInfo.itemIndex, loadoutName);
                v0 = va("%s%s", loadoutName, optionName);
                BG_UnlockablesEquipClassToSlot(0, customClassName, sharedUiInfo.sortedItemPivot, v0);
            }
        }
    }
    else
    {
        Com_PrintError(15, "equipclasscurrentoption usage: equipclasscurrentoption <customclassname> <optionName>\n");
    }
}

void __cdecl BG_UnlockablesToggleWeaponOptionCmd()
{
    const char *v0; // eax
    const char *v1; // eax
    unsigned int isEquipped; // [esp+18h] [ebp-1Ch]
    const char *loadoutName; // [esp+1Ch] [ebp-18h]
    const char *customClassName; // [esp+20h] [ebp-14h]
    itemInfo_t *itemInfo; // [esp+28h] [ebp-Ch]
    int optionNum; // [esp+2Ch] [ebp-8h]
    const char *optionName; // [esp+30h] [ebp-4h]

    if ( Cmd_Argc() >= 3 )
    {
        customClassName = Cmd_Argv(1);
        optionName = Cmd_Argv(2);
        itemInfo = BG_UnlockablesGetItemInfo(sharedUiInfo.itemIndex);
        if ( !itemInfo
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                        3379,
                        0,
                        "%s",
                        "itemInfo") )
        {
            __debugbreak();
        }
        optionNum = BG_GetWeaponOptionNumFromIndexAndGroup(
                                    0,
                                    (eWeaponOptionGroup)ui_currentWeaponOptionGroup->current.integer);
        if ( itemInfo
            && BG_UnlockablesIsItemPurchased(0, sharedUiInfo.itemIndex)
            && BG_UnlockablesIsItemOptionPurchased(0, sharedUiInfo.itemIndex, optionNum) )
        {
            loadoutName = BG_UnlockablesGetLoadoutName(itemInfo->loadoutSlot);
            if ( loadoutName )
            {
                BG_UnlockablesEquipClassToSlot(0, customClassName, sharedUiInfo.itemIndex, loadoutName);
                v0 = va("%s%s", loadoutName, optionName);
                isEquipped = BG_UnlockablesGetEquippedItemInSlot(0, customClassName, v0);
                v1 = va("%s%s", loadoutName, optionName);
                BG_UnlockablesEquipClassToSlot(0, customClassName, isEquipped ^ 1, v1);
            }
        }
    }
    else
    {
        Com_PrintError(15, "toggleWeaponOption usage: toggleWeaponOption <customclassname> <optionName>\n");
    }
}

void __cdecl BG_UnlockablesEquipClassCurrentItemCmd()
{
    BG_UnlockablesEquipClassCurrentItemCmdInternal(0);
}

void __cdecl BG_UnlockablesEquipClassCurrentItemCmdInternal(bool isGlobal)
{
    int FaceCamoNumFromItemIndex; // eax
    const char *loadoutName; // [esp+10h] [ebp-18h]
    loadoutSlot_t loadoutSlot; // [esp+14h] [ebp-14h]
    itemInfo_t *itemInfo; // [esp+18h] [ebp-10h]
    const char *customClassName; // [esp+1Ch] [ebp-Ch]
    int itemIndex; // [esp+24h] [ebp-4h]

    if ( isGlobal || Cmd_Argc() >= 2 )
    {
        customClassName = 0;
        if ( !isGlobal )
            customClassName = Cmd_Argv(1);
        itemIndex = sharedUiInfo.itemIndex;
        if ( sharedUiInfo.itemIndex == -1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                        3420,
                        0,
                        "%s",
                        "itemIndex != ENTRY_NOT_FOUND") )
        {
            __debugbreak();
        }
        if ( itemIndex != -1 && BG_UnlockablesIsItemPurchased(0, itemIndex) )
        {
            itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
            if ( !itemInfo
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                            3427,
                            0,
                            "%s",
                            "itemInfo") )
            {
                __debugbreak();
            }
            if ( itemInfo )
            {
                loadoutSlot = itemInfo->loadoutSlot;
                if ( loadoutSlot == LOADOUTSLOT_FACEPAINT_PATTERN || loadoutSlot == LOADOUTSLOT_FACEPAINT_COLOR )
                {
                    loadoutName = BG_UnlockablesGetLoadoutName(loadoutSlot);
                    if ( loadoutName )
                    {
                        FaceCamoNumFromItemIndex = BG_UnlockablesGetFaceCamoNumFromItemIndex(itemIndex);
                        BG_UnlockablesEquipClassToSlot(0, customClassName, FaceCamoNumFromItemIndex, loadoutName);
                    }
                }
                else if ( (loadoutSlot == LOADOUTSLOT_FIRST || loadoutSlot == LOADOUTSLOT_SECONDARY_WEAPON)
                             && itemInfo->numAttachments[0] == 1 )
                {
                    BG_UnlockablesEquipClassAttachment(0, customClassName, itemInfo->name, "none", "");
                }
                else
                {
                    BG_UnlockablesEquipClass(0, customClassName, itemIndex);
                }
            }
        }
    }
    else
    {
        Com_PrintError(15, "equipclasscurrentitem usage: equipclasscurrentitem <customclassname>\n");
    }
}

int __cdecl BG_UnlockablesGetFaceCamoNumFromItemIndex(int itemIndex)
{
    int faceCamoNum; // [esp+0h] [ebp-10h]
    itemInfo_t *itemInfo; // [esp+4h] [ebp-Ch]
    int numCamos; // [esp+8h] [ebp-8h]
    int *itemIndices; // [esp+Ch] [ebp-4h]

    itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
    itemIndices = 0;
    numCamos = 0;
    if ( itemInfo )
    {
        if ( itemInfo->loadoutSlot == LOADOUTSLOT_FACEPAINT_PATTERN )
        {
            itemIndices = s_unlockableItems.faceCamoPatterns;
            numCamos = s_unlockableItems.numFaceCamoPatterns;
        }
        else if ( itemInfo->loadoutSlot == LOADOUTSLOT_FACEPAINT_COLOR )
        {
            itemIndices = s_unlockableItems.faceCamoColors;
            numCamos = s_unlockableItems.numFaceCamoColors;
        }
        if ( itemIndices && numCamos )
        {
            for ( faceCamoNum = 0; faceCamoNum < numCamos; ++faceCamoNum )
            {
                if ( itemIndices[faceCamoNum] == itemIndex )
                    return faceCamoNum;
            }
        }
    }
    Com_PrintError(13, "Face camo item %d not found\n", itemIndex);
    return 0;
}

void __cdecl BG_UnlockablesEquipClassCurrentGlobalItemCmd()
{
    BG_UnlockablesEquipClassCurrentItemCmdInternal(1);
}

void __cdecl BG_UnlockablesEquipClassItemCmd()
{
    const char *v0; // eax
    itemInfo_t *itemInfo; // [esp+18h] [ebp-10h]
    const char *customClassName; // [esp+1Ch] [ebp-Ch]
    int itemIndex; // [esp+24h] [ebp-4h]

    if ( Cmd_Argc() >= 3 )
    {
        customClassName = Cmd_Argv(1);
        v0 = Cmd_Argv(2);
        itemIndex = atoi(v0);
        if ( itemIndex == -1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                        3488,
                        0,
                        "%s",
                        "itemIndex != ENTRY_NOT_FOUND") )
        {
            __debugbreak();
        }
        if ( itemIndex != -1 && BG_UnlockablesIsItemPurchased(0, itemIndex) )
        {
            itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
            if ( !itemInfo
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                            3495,
                            0,
                            "%s",
                            "itemInfo") )
            {
                __debugbreak();
            }
            if ( itemInfo )
            {
                if ( (itemInfo->loadoutSlot == LOADOUTSLOT_FIRST || itemInfo->loadoutSlot == LOADOUTSLOT_SECONDARY_WEAPON)
                    && itemInfo->numAttachments[0] == 1 )
                {
                    BG_UnlockablesEquipClassAttachment(0, customClassName, itemInfo->name, "none", "");
                }
                else
                {
                    BG_UnlockablesEquipClass(0, customClassName, itemIndex);
                }
            }
        }
    }
    else
    {
        Com_PrintError(15, "equipclassitem usage: equipclassitem <customclassname> <itemindex>\n");
    }
}

void __cdecl BG_UnlockablesEquipDefaultClassCmd()
{
    const char *v0; // eax
    const char *customClassName; // [esp+18h] [ebp-10h]
    const char *defaultClassName; // [esp+20h] [ebp-8h]
    defaultClass_t defaultClassNum; // [esp+24h] [ebp-4h]

    if ( Cmd_Argc() >= 3 )
    {
        customClassName = Cmd_Argv(1);
        defaultClassName = Cmd_Argv(2);
        defaultClassNum = (defaultClass_t)FindEntryInList(defaultClassName, s_classNames, 20);
        if ( defaultClassNum == CLASS_NONE )
        {
            v0 = va("Default class %s not found\n", defaultClassName);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                            3583,
                            0,
                            "%s\n\t%s",
                            "defaultClassNum != ENTRY_NOT_FOUND",
                            v0) )
                __debugbreak();
        }
        if ( defaultClassNum != CLASS_NONE )
            BG_UnlockablesEquipDefaultClass(0, customClassName, defaultClassNum);
    }
    else
    {
        Com_PrintError(15, "equipdefaultclass usage: equipdefaultclass <customclassname> <defaultClass>\n");
    }
}

void __cdecl BG_UnlockablesEquipDefaultClass(
                int controllerIndex,
                const char *customClassName,
                defaultClass_t defaultClass)
{
    const char *loadoutName; // [esp+0h] [ebp-Ch]
    int itemIndex; // [esp+4h] [ebp-8h]
    loadoutSlot_t loadoutSlot; // [esp+8h] [ebp-4h]

    for ( loadoutSlot = LOADOUTSLOT_FIRST; loadoutSlot < LOADOUTSLOT_GLOBAL_ITEMS_START; ++loadoutSlot )
    {
        itemIndex = s_unlockableItems.defaultClassLoadouts[defaultClass][loadoutSlot];
        if ( itemIndex == -1 )
            itemIndex = BG_UnlockablesGetDefaultItem(loadoutSlot);
        loadoutName = BG_UnlockablesGetLoadoutName(loadoutSlot);
        if ( loadoutName )
            BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, itemIndex, loadoutName);
    }
    BG_ReplaceItemsWithPurchasedProItem(controllerIndex);
}

void __cdecl BG_UnlockablesEquipDefaultGlobalItemsCmd()
{
    const char *loadoutName; // [esp+0h] [ebp-10h]
    int itemIndex; // [esp+4h] [ebp-Ch]
    loadoutSlot_t loadoutSlot; // [esp+8h] [ebp-8h]

    for ( loadoutSlot = LOADOUTSLOT_GLOBAL_ITEMS_START; loadoutSlot < LOADOUTSLOT_COUNT; ++loadoutSlot )
    {
        itemIndex = s_unlockableItems.defaultGlobalItems[loadoutSlot - 35];
        if ( itemIndex == -1 )
            itemIndex = BG_UnlockablesGetDefaultItem(loadoutSlot);
        loadoutName = BG_UnlockablesGetLoadoutName(loadoutSlot);
        if ( loadoutName )
            BG_UnlockablesEquipClassToSlot(0, 0, itemIndex, loadoutName);
    }
}

void __cdecl BG_UnlockablesClearItemNewCmd()
{
    const char *itemName; // [esp+14h] [ebp-8h]
    int itemIndex; // [esp+18h] [ebp-4h]

    if ( Cmd_Argc() >= 2 )
    {
        itemName = Cmd_Argv(1);
        itemIndex = BG_UnlockablesGetItemIndexFromName(itemName);
        if ( itemIndex == -1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                        3673,
                        0,
                        "%s",
                        "itemIndex != ENTRY_NOT_FOUND") )
        {
            __debugbreak();
        }
        if ( itemIndex != -1 )
            BG_UnlockablesClearItemNew(0, itemIndex);
    }
    else
    {
        Com_PrintError(15, "clearitemnew usage: clearitemnew <itemname>\n");
    }
}

void __cdecl BG_UnlockablesClearAllItemsNewCmd()
{
    BG_UnlockablesClearAllItemsNew(0);
}

void __cdecl BG_ReplaceItemsWithPurchasedProItem(int controllerIndex)
{
    int proItemIndex; // [esp+0h] [ebp-18h]
    unsigned int itemIndex; // [esp+4h] [ebp-14h]
    const char *loadoutName; // [esp+8h] [ebp-10h]
    loadoutSlot_t loadoutSlot; // [esp+Ch] [ebp-Ch]
    char *customClassName; // [esp+10h] [ebp-8h]
    customClass_t classIndex; // [esp+14h] [ebp-4h]

    for ( classIndex = CUSTOM_CLASS_1; classIndex < CUSTOM_CLASS_COUNT; ++classIndex )
    {
        customClassName = (char *)BG_UnlockablesGetCustomClassName(classIndex);
        if ( Dvar_GetInt(custom_class_mode) )
            customClassName = va("%d", classIndex + 1);
        if ( customClassName )
        {
            for ( loadoutSlot = LOADOUTSLOT_FIRST_SPECIALTY; loadoutSlot <= LOADOUTSLOT_SPECIALTY3; ++loadoutSlot )
            {
                loadoutName = BG_UnlockablesGetLoadoutName(loadoutSlot);
                if ( loadoutName )
                {
                    itemIndex = BG_UnlockablesGetEquippedItemInSlot(controllerIndex, customClassName, loadoutName);
                    if ( BG_UnlockablesIsItemValidNotNull(itemIndex) )
                    {
                        proItemIndex = BG_UnlockablesGetProItemIfPurchased(controllerIndex, itemIndex);
                        if ( itemIndex != proItemIndex )
                            BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, proItemIndex, loadoutName);
                    }
                }
            }
        }
    }
}

int __cdecl BG_UnlockablesGetProItemIfPurchased(int controllerIndex, int itemIndex)
{
    int proItemIndex; // [esp+0h] [ebp-4h]

    if ( BG_UnlockablesIsProItemUnlocked(controllerIndex, itemIndex)
        && (proItemIndex = BG_UnlockablesGetProItem(itemIndex), proItemIndex != -1)
        && BG_UnlockablesIsItemPurchased(controllerIndex, proItemIndex) )
    {
        return proItemIndex;
    }
    else
    {
        return itemIndex;
    }
}

void __cdecl BG_UnlockablesPurchaseCurrentProItemCmd()
{
    int ItemCost; // eax
    int proItemIndex; // [esp+0h] [ebp-4h]

    proItemIndex = BG_UnlockablesGetProItem(sharedUiInfo.itemIndex);
    if ( proItemIndex != -1 )
    {
        ItemCost = BG_UnlockablesGetItemCost(proItemIndex);
        BG_UnlockablesPurchaseItem(0, proItemIndex, ItemCost);
        BG_ReplaceItemsWithPurchasedProItem(0);
    }
}

void __cdecl BG_UnlockablesBuildItemListForSlotNameCmd()
{
    const char *slotName; // [esp+14h] [ebp-4h]

    if ( Cmd_Argc() >= 2 )
    {
        slotName = Cmd_Argv(1);
        BG_UnlockablesBuildItemListForSlotName(0, slotName);
    }
    else
    {
        Com_PrintError(15, "buildItemListForSlotName usage: buildItemListForSlotName <loadout slot> \n");
    }
}

int __cdecl BG_UnlockablesBuildItemListForSlotName(int controllerIndex, const char *slotName)
{
    const char *v2; // eax
    int proItem; // [esp+0h] [ebp-10h]
    itemInfo_t *itemInfo; // [esp+4h] [ebp-Ch]
    int itemNum; // [esp+8h] [ebp-8h]
    int loadoutSlot; // [esp+Ch] [ebp-4h]

    if ( !slotName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp", 3886, 0, "%s", "slotName") )
    {
        __debugbreak();
    }
    s_unlockableItems.numItemsInSlot = 0;
    if ( slotName )
    {
        loadoutSlot = BG_UnlockablesGetLoadoutSlotFromString(slotName);
        if ( loadoutSlot == -1 )
        {
            v2 = va("Loadout Slot %s not found", slotName);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                            3893,
                            0,
                            "%s\n\t%s",
                            "loadoutSlot != ENTRY_NOT_FOUND",
                            v2) )
                __debugbreak();
        }
        if ( loadoutSlot != -1 )
        {
            for ( itemNum = 0; itemNum <= 256; ++itemNum )
            {
                itemInfo = &s_unlockableItems.itemTable[itemNum];
                if ( itemInfo->isValid && itemInfo->loadoutSlot == loadoutSlot && itemInfo->cost >= 0 )
                {
                    proItem = itemNum;
                    if ( itemInfo->group == ITEMGROUP_SPECIALTY )
                    {
                        if ( BG_UnlockablesIsProItem(itemNum) )
                            continue;
                        proItem = BG_UnlockablesGetProItemIfPurchased(controllerIndex, itemNum);
                    }
                    s_unlockableItems.itemsInSlot[s_unlockableItems.numItemsInSlot++] = &s_unlockableItems.itemTable[proItem];
                }
            }
        }
    }
    s_unlockableItems.numPurchasedItemsInSlot = BG_UnlockablesGetNumPurchasedItemsInCurrentList(controllerIndex);
    qsort(s_unlockableItems.itemsInSlot, s_unlockableItems.numItemsInSlot, 4u, BG_UnlockablesCompareItemsBySortKey);
    return s_unlockableItems.numItemsInSlot;
}

int __cdecl BG_UnlockablesGetNumPurchasedItemsInCurrentList(int controllerIndex)
{
    itemInfo_t *itemInfo; // [esp+0h] [ebp-Ch]
    int itemNum; // [esp+4h] [ebp-8h]
    int numPurchasedItems; // [esp+8h] [ebp-4h]

    numPurchasedItems = 0;
    for ( itemNum = 0; itemNum < s_unlockableItems.numItemsInSlot; ++itemNum )
    {
        itemInfo = s_unlockableItems.itemsInSlot[itemNum];
        if ( itemInfo && itemInfo->isValid && BG_UnlockablesIsItemPurchased(controllerIndex, itemInfo->index) )
            ++numPurchasedItems;
    }
    return numPurchasedItems;
}

void __cdecl BG_UnlockablesBuildItemListForChallengesPerksCmd()
{
    const char *slotName; // [esp+14h] [ebp-4h]

    if ( Cmd_Argc() >= 2 )
    {
        slotName = Cmd_Argv(1);
        BG_UnlockablesBuildItemListForChallengesPerks(0, slotName);
    }
    else
    {
        Com_PrintError(15, "buildItemListForChallengesPerks usage: buildItemListForChallengesPerks <loadout slot> \n");
    }
}

int __cdecl BG_UnlockablesBuildItemListForChallengesPerks(int controllerIndex, const char *slotName)
{
    const char *v2; // eax
    itemInfo_t *itemInfo; // [esp+0h] [ebp-Ch]
    int itemNum; // [esp+4h] [ebp-8h]
    int loadoutSlot; // [esp+8h] [ebp-4h]

    if ( !slotName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp", 3931, 0, "%s", "slotName") )
    {
        __debugbreak();
    }
    s_unlockableItems.numItemsInSlot = 0;
    if ( slotName )
    {
        loadoutSlot = BG_UnlockablesGetLoadoutSlotFromString(slotName);
        if ( loadoutSlot == -1 )
        {
            v2 = va("Loadout Slot %s not found", slotName);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                            3938,
                            0,
                            "%s\n\t%s",
                            "loadoutSlot != ENTRY_NOT_FOUND",
                            v2) )
                __debugbreak();
        }
        if ( loadoutSlot != -1 )
        {
            for ( itemNum = 0; itemNum <= 256; ++itemNum )
            {
                itemInfo = &s_unlockableItems.itemTable[itemNum];
                if ( itemInfo->isValid
                    && itemInfo->loadoutSlot == loadoutSlot
                    && itemInfo->cost >= 0
                    && itemInfo->group == ITEMGROUP_SPECIALTY
                    && !BG_UnlockablesIsProItem(itemNum) )
                {
                    s_unlockableItems.itemsInSlot[s_unlockableItems.numItemsInSlot++] = &s_unlockableItems.itemTable[itemNum];
                }
            }
        }
    }
    s_unlockableItems.numPurchasedItemsInSlot = BG_UnlockablesGetNumPurchasedItemsInCurrentList(controllerIndex);
    qsort(s_unlockableItems.itemsInSlot, s_unlockableItems.numItemsInSlot, 4u, BG_UnlockablesCompareItemsBySortKey);
    return s_unlockableItems.numItemsInSlot;
}

void __cdecl BG_UnlockablesBuildItemListForGroupCmd()
{
    const char *v0; // eax
    itemGroup_t group; // [esp+10h] [ebp-4h]

    if ( Cmd_Argc() >= 2 )
    {
        v0 = Cmd_Argv(1);
        group = (itemGroup_t)atoi(v0);
        BG_UnlockablesBuildItemListForGroup(group);
    }
    else
    {
        Com_PrintError(15, "buildItemListForGroup usage: buildItemListForGroup <group> \n");
    }
}

void __cdecl BG_UnlockablesBuildItemListForGroupForWeaponTableCmd()
{
    const char *v0; // eax
    itemGroup_t group; // [esp+10h] [ebp-4h]

    if ( Cmd_Argc() >= 2 )
    {
        v0 = Cmd_Argv(1);
        group = (itemGroup_t)atoi(v0);
        BG_UnlockablesBuildItemListForGroupForWeaponTable(group);
    }
    else
    {
        Com_PrintError(15, "buildItemListForGroupForWeaponTable usage: buildItemListForGroupForWeaponTable <group> \n");
    }
}

void __cdecl BG_UnlockablesBuildItemListForGroupNameCmd()
{
    const char *groupName; // [esp+14h] [ebp-4h]

    if ( Cmd_Argc() >= 2 )
    {
        groupName = Cmd_Argv(1);
        BG_UnlockablesBuildItemListForGroupName(0, groupName);
    }
    else
    {
        Com_PrintError(15, "buildItemListForGroupName usage: buildItemListForGroupName <group name> \n");
    }
}

int __cdecl BG_UnlockablesBuildItemListForGroupName(int controllerIndex, const char *groupName)
{
    const char *v2; // eax
    itemGroup_t group; // [esp+0h] [ebp-4h]

    if ( !groupName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    4088,
                    0,
                    "%s",
                    "groupName") )
    {
        __debugbreak();
    }
    s_unlockableItems.numItemsInSlot = 0;
    if ( groupName )
    {
        group = (itemGroup_t)FindEntryInList(groupName, s_itemGroupNames, 24);
        if ( group == ITEMGROUP_INVALID )
        {
            v2 = va("Item Group %s not found", groupName);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                            4095,
                            0,
                            "%s\n\t%s",
                            "group != ENTRY_NOT_FOUND",
                            v2) )
                __debugbreak();
        }
        if ( group != ITEMGROUP_INVALID )
            s_unlockableItems.numItemsInSlot = BG_UnlockablesBuildItemListForGroup(group);
    }
    s_unlockableItems.numPurchasedItemsInSlot = BG_UnlockablesGetNumPurchasedItemsInCurrentList(controllerIndex);
    qsort(s_unlockableItems.itemsInSlot, s_unlockableItems.numItemsInSlot, 4u, BG_UnlockablesCompareItemsBySortKey);
    return s_unlockableItems.numItemsInSlot;
}

void __cdecl BG_UnlockablesBuildItemListForSlotNameAndGroupCmd()
{
    const char *groupName; // [esp+1Ch] [ebp-8h]
    const char *slotName; // [esp+20h] [ebp-4h]

    if ( Cmd_Argc() >= 3 )
    {
        slotName = Cmd_Argv(1);
        groupName = Cmd_Argv(2);
        BG_UnlockablesBuildItemListForSlotNameAndGroup(0, slotName, groupName);
    }
    else
    {
        Com_PrintError(
            15,
            "buildItemListForSlotNameAndGroup usage: buildItemListForSlotNameAndGroup <slot name> <group name> \n");
    }
}

int __cdecl BG_UnlockablesBuildItemListForSlotNameAndGroup(
                int controllerIndex,
                const char *slotName,
                const char *groupName)
{
    const char *v3; // eax
    const char *v4; // eax
    int itemNum; // [esp+0h] [ebp-Ch]
    int loadoutSlot; // [esp+4h] [ebp-8h]
    int group; // [esp+8h] [ebp-4h]

    if ( !slotName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp", 4135, 0, "%s", "slotName") )
    {
        __debugbreak();
    }
    if ( !groupName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    4136,
                    0,
                    "%s",
                    "groupName") )
    {
        __debugbreak();
    }
    s_unlockableItems.numItemsInSlot = 0;
    if ( slotName && groupName )
    {
        loadoutSlot = BG_UnlockablesGetLoadoutSlotFromString(slotName);
        if ( loadoutSlot == -1 )
        {
            v3 = va("Loadout Slot %s not found", slotName);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                            4143,
                            0,
                            "%s\n\t%s",
                            "loadoutSlot != ENTRY_NOT_FOUND",
                            v3) )
                __debugbreak();
        }
        group = FindEntryInList(groupName, s_itemGroupNames, 24);
        if ( group == -1 )
        {
            v4 = va("Item Group %s not found", groupName);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                            4145,
                            0,
                            "%s\n\t%s",
                            "group != ENTRY_NOT_FOUND",
                            v4) )
                __debugbreak();
        }
        if ( loadoutSlot != -1 && group != -1 )
        {
            for ( itemNum = 0; itemNum <= 256; ++itemNum )
            {
                if ( s_unlockableItems.itemTable[itemNum].isValid
                    && s_unlockableItems.itemTable[itemNum].loadoutSlot == loadoutSlot
                    && s_unlockableItems.itemTable[itemNum].group == group
                    && s_unlockableItems.itemTable[itemNum].cost >= 0 )
                {
                    s_unlockableItems.itemsInSlot[s_unlockableItems.numItemsInSlot++] = &s_unlockableItems.itemTable[itemNum];
                }
            }
        }
    }
    s_unlockableItems.numPurchasedItemsInSlot = BG_UnlockablesGetNumPurchasedItemsInCurrentList(controllerIndex);
    qsort(s_unlockableItems.itemsInSlot, s_unlockableItems.numItemsInSlot, 4u, BG_UnlockablesCompareItemsBySortKey);
    return s_unlockableItems.numItemsInSlot;
}

void __cdecl OpenAttachmentMenuForCurrentItemCmd()
{
    const char *v0; // eax
    const char *ItemName; // eax
    char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    int numAttachments; // [esp+30h] [ebp-14h]
    uiInfo_s *uiInfo; // [esp+34h] [ebp-10h]
    const char *menuName; // [esp+3Ch] [ebp-8h]
    bool equipIfNone; // [esp+43h] [ebp-1h]

    if ( Cmd_Argc() >= 2 )
    {
        menuName = Cmd_Argv(1);
        equipIfNone = 0;
        if ( Cmd_Argc() == 3 )
        {
            v0 = Cmd_Argv(2);
            equipIfNone = I_stricmp(v0, "equipifnone") == 0;
        }
        if ( !menuName
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                        4289,
                        0,
                        "%s",
                        "menuName") )
        {
            __debugbreak();
        }
        if ( sharedUiInfo.itemIndex == -1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                        4290,
                        0,
                        "%s",
                        "sharedUiInfo.itemIndex != ENTRY_NOT_FOUND") )
        {
            __debugbreak();
        }
        if ( menuName && sharedUiInfo.itemIndex != -1 && BG_UnlockablesIsItemPurchased(0, sharedUiInfo.itemIndex) )
        {
            uiInfo = UI_GetInfo(0);
            if ( !uiInfo
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                            4299,
                            0,
                            "%s",
                            "uiInfo") )
            {
                __debugbreak();
            }
            numAttachments = BG_UnlockablesGetNumItemAttachments(sharedUiInfo.itemIndex);
            if ( !numAttachments
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                            4302,
                            0,
                            "%s",
                            "numAttachments") )
            {
                __debugbreak();
            }
            if ( numAttachments == 1 )
            {
                ItemName = BG_UnlockablesGetItemName(sharedUiInfo.itemIndex);
                if ( equipIfNone )
                {
                    v2 = va("equipclassattachment ( dvarString( ui_custom_name ) ) %s none", ItemName);
                    Cmd_ExecuteSingleCommand(0, 0, v2);
                }
                else
                {
                    Com_PrintWarning(13, "No attachments on item '%s'\n", ItemName);
                }
            }
            else if ( uiInfo )
            {
                v3 = Cmd_Argv(1);
                if ( !Menus_OpenByName(0, &uiInfo->uiDC, v3) )
                {
                    v4 = Cmd_Argv(1);
                    Com_PrintWarning(13, "Could not find menu '%s'\n", v4);
                }
            }
        }
    }
    else
    {
        Com_PrintError(
            15,
            "OpenAttachmentMenuForCurrentItem usage: OpenAttachmentMenuForCurrentItem <attachment_menu> <optional    - equipifnone>\n");
    }
}

void __cdecl BG_UnlockablesEquipItemInSlotSortedCmd()
{
    BG_UnlockablesEquipItemInSlotSortedCmdInternal(0);
}

void __cdecl BG_UnlockablesEquipItemInSlotSortedCmdInternal(bool isGlobalItem)
{
    const char *v1; // eax
    loadoutSlot_t LoadoutSlotFromString; // eax
    const char *v3; // eax
    const char *v4; // eax
    int DefaultItem; // eax
    const char *v6; // eax
    unsigned int EquippedItemInSlot; // eax
    itemInfo_t *ItemInfo; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    const char *v13; // eax
    const char *v14; // eax
    unsigned int v15; // eax
    const char *v16; // eax
    const char *v17; // [esp-4h] [ebp-304h]
    char *v18; // [esp+0h] [ebp-300h]
    char *nptr; // [esp+Ch] [ebp-2F4h]
    unsigned int itemLoadoutSlots[41]; // [esp+48h] [ebp-2B8h]
    int selectedItemAttribute; // [esp+ECh] [ebp-214h]
    int itemAttribute[40]; // [esp+F0h] [ebp-210h]
    int shiftIndex; // [esp+190h] [ebp-170h]
    const char *customClassName; // [esp+194h] [ebp-16Ch]
    int numItemsOfSameAttributeAllowed; // [esp+198h] [ebp-168h]
    int controllerIndex; // [esp+19Ch] [ebp-164h]
    const char *groupName; // [esp+1A0h] [ebp-160h]
    int totalItemsEquipped; // [esp+1A4h] [ebp-15Ch]
    int currSlot; // [esp+1A8h] [ebp-158h]
    int currentArg; // [esp+1ACh] [ebp-154h]
    int itemIndices[41]; // [esp+1B0h] [ebp-150h]
    int groupItemCount; // [esp+254h] [ebp-ACh]
    int defaultItemIndices[41]; // [esp+258h] [ebp-A8h]
    int itemsOfSameAttributeEquipped; // [esp+2FCh] [ebp-4h]

    controllerIndex = 0;
    if ( isGlobalItem )
    {
        if ( Cmd_Argc() < 4 )
        {
            Com_PrintError(
                15,
                "equipGlobalItemInSlotSorted usage: equipItemInSlotSorted <groupItemCount> <groupName> <numItemsOfSameAttributeAllowed>\n");
            return;
        }
    }
    else if ( Cmd_Argc() < 5 )
    {
        Com_PrintError(
            15,
            "equipItemInSlotSorted usage: equipItemInSlotSorted <customclassname> <groupItemCount> <groupName> <numItemsOfSameA"
            "ttributeAllowed>\n");
        return;
    }
    currentArg = 1;
    customClassName = 0;
    if ( !isGlobalItem )
        customClassName = Cmd_Argv(currentArg++);
    nptr = (char *)Cmd_Argv(currentArg);
    groupItemCount = atoi(nptr);
    groupName = Cmd_Argv(currentArg + 1);
    v18 = (char *)Cmd_Argv(currentArg + 2);
    numItemsOfSameAttributeAllowed = atoi(v18);
    currentArg += 3;
    totalItemsEquipped = 0;
    itemsOfSameAttributeEquipped = 0;
    if ( groupItemCount > 39
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                    4361,
                    0,
                    "%s",
                    "groupItemCount <= LOADOUTSLOT_COUNT") )
    {
        __debugbreak();
    }
    if ( groupItemCount <= 39 )
    {
        selectedItemAttribute = BG_UnlockablesGetItemInfo(sharedUiInfo.itemIndex)->count;
        for ( currSlot = 0; currSlot < groupItemCount; ++currSlot )
        {
            v1 = va("%s%d", groupName, currSlot + 1);
            LoadoutSlotFromString = BG_UnlockablesGetLoadoutSlotFromString(v1);
            itemLoadoutSlots[currSlot] = LoadoutSlotFromString;
            if ( itemLoadoutSlots[currSlot] == -1 )
            {
                v3 = va("%s%d", groupName, currSlot + 1);
                v4 = va("Loadout Slot %s not found", v3);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                                4371,
                                0,
                                "%s\n\t%s",
                                "itemLoadoutSlots[ currSlot ] != ENTRY_NOT_FOUND",
                                v4) )
                    __debugbreak();
            }
            if ( itemLoadoutSlots[currSlot] == -1 )
                return;
            DefaultItem = BG_UnlockablesGetDefaultItem(itemLoadoutSlots[currSlot]);
            defaultItemIndices[currSlot] = DefaultItem;
            v6 = va("%s%d", groupName, currSlot + 1);
            EquippedItemInSlot = BG_UnlockablesGetEquippedItemInSlot(controllerIndex, customClassName, v6);
            itemIndices[currSlot] = EquippedItemInSlot;
            ItemInfo = BG_UnlockablesGetItemInfo(itemIndices[currSlot]);
            itemAttribute[currSlot] = ItemInfo->count;
            if ( itemIndices[currSlot] != defaultItemIndices[currSlot] )
            {
                ++totalItemsEquipped;
                if ( itemAttribute[currSlot] == selectedItemAttribute )
                    ++itemsOfSameAttributeEquipped;
            }
        }
        for ( currSlot = 0; currSlot < groupItemCount; ++currSlot )
        {
            if ( sharedUiInfo.itemIndex == itemIndices[currSlot] )
            {
                for ( shiftIndex = currSlot; shiftIndex <= totalItemsEquipped - 2; ++shiftIndex )
                {
                    v9 = va("%s%d", groupName, shiftIndex + 1);
                    BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, itemIndices[shiftIndex + 1], v9);
                }
                v10 = va("%s%d", groupName, shiftIndex + 1);
                BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, defaultItemIndices[shiftIndex], v10);
                return;
            }
        }
        if ( BG_UnlockablesIsItemPurchased(controllerIndex, sharedUiInfo.itemIndex) && totalItemsEquipped < groupItemCount )
        {
            if ( !totalItemsEquipped )
            {
                v11 = va("%s1", groupName);
                BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, sharedUiInfo.itemIndex, v11);
                return;
            }
            for ( currSlot = 0; currSlot < totalItemsEquipped; ++currSlot )
            {
                if ( selectedItemAttribute <= itemAttribute[currSlot]
                    && itemsOfSameAttributeEquipped < numItemsOfSameAttributeAllowed )
                {
                    for ( shiftIndex = totalItemsEquipped - 1; shiftIndex >= currSlot; --shiftIndex )
                    {
                        v12 = va("%s%d", groupName, shiftIndex + 2);
                        BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, itemIndices[shiftIndex], v12);
                    }
                    if ( selectedItemAttribute == itemAttribute[currSlot] )
                    {
                        if ( sharedUiInfo.itemIndex >= itemIndices[currSlot] )
                        {
                            v17 = va("%s%d", groupName, currSlot + 1);
                            v14 = va("%s%d", groupName, currSlot + 1);
                            v15 = BG_UnlockablesGetEquippedItemInSlot(controllerIndex, customClassName, v14);
                            BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, v15, v17);
                            v13 = va("%s%d", groupName, currSlot + 2);
                        }
                        else
                        {
                            v13 = va("%s%d", groupName, currSlot + 1);
                        }
                        BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, sharedUiInfo.itemIndex, v13);
                        return;
                    }
LABEL_51:
                    v16 = va("%s%d", groupName, currSlot + 1);
                    BG_UnlockablesEquipClassToSlot(controllerIndex, customClassName, sharedUiInfo.itemIndex, v16);
                    return;
                }
            }
            if ( itemsOfSameAttributeEquipped >= numItemsOfSameAttributeAllowed )
                return;
            goto LABEL_51;
        }
    }
}

void __cdecl BG_UnlockablesEquipGlobalItemInSlotSortedCmd()
{
    BG_UnlockablesEquipItemInSlotSortedCmdInternal(1);
}

void __cdecl BG_ClearAllAttachmentsForItemCmd()
{
    const char *v0; // eax
    const char *AttachmentPointName; // eax
    const char *v2; // eax
    eAttachmentPoint attachPoint; // [esp+18h] [ebp-18h]
    const char *customClassName; // [esp+1Ch] [ebp-14h]
    const char *loadoutName; // [esp+24h] [ebp-Ch]
    itemInfo_t *itemInfo; // [esp+28h] [ebp-8h]
    int itemIndex; // [esp+2Ch] [ebp-4h]

    if ( Cmd_Argc() >= 3 )
    {
        customClassName = Cmd_Argv(1);
        v0 = Cmd_Argv(2);
        itemIndex = atoi(v0);
        itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
        if ( itemInfo->loadoutSlot == LOADOUTSLOT_INVALID
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                        4481,
                        0,
                        "%s",
                        "itemInfo->loadoutSlot != LOADOUTSLOT_INVALID") )
        {
            __debugbreak();
        }
        if ( itemInfo )
        {
            loadoutName = BG_UnlockablesGetLoadoutName(itemInfo->loadoutSlot);
            if ( loadoutName )
            {
                for ( attachPoint = ATTACHMENT_POINT_TOP; attachPoint < ATTACHMENT_POINT_COUNT; ++attachPoint )
                {
                    AttachmentPointName = BG_GetAttachmentPointName(attachPoint);
                    v2 = va("%sattachment%s", loadoutName, AttachmentPointName);
                    BG_UnlockablesEquipClassToSlot(0, customClassName, 0, v2);
                }
            }
        }
    }
    else
    {
        Com_PrintError(15, "clearAllAttachmentsForItem usage: clearAllAttachmentsForItem <customClassName> <itemIndex>\n");
    }
}

void __cdecl BG_ClearAttachmentForItemAtPointCmd()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *AttachmentPointName; // eax
    const char *v3; // eax
    const char *customClassName; // [esp+20h] [ebp-18h]
    const char *loadoutName; // [esp+28h] [ebp-10h]
    itemInfo_t *itemInfo; // [esp+2Ch] [ebp-Ch]
    eAttachmentPoint attachPoint; // [esp+30h] [ebp-8h]
    int itemIndex; // [esp+34h] [ebp-4h]

    if ( Cmd_Argc() >= 4 )
    {
        customClassName = Cmd_Argv(1);
        v0 = Cmd_Argv(2);
        itemIndex = atoi(v0);
        v1 = Cmd_Argv(3);
        attachPoint = (eAttachmentPoint)atoi(v1);
        itemInfo = BG_UnlockablesGetItemInfo(itemIndex);
        if ( itemInfo->loadoutSlot == LOADOUTSLOT_INVALID
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_unlockable_items.cpp",
                        4516,
                        0,
                        "%s",
                        "itemInfo->loadoutSlot != LOADOUTSLOT_INVALID") )
        {
            __debugbreak();
        }
        if ( itemInfo )
        {
            loadoutName = BG_UnlockablesGetLoadoutName(itemInfo->loadoutSlot);
            if ( loadoutName )
            {
                if ( *loadoutName )
                {
                    AttachmentPointName = BG_GetAttachmentPointName(attachPoint);
                    v3 = va("%sattachment%s", loadoutName, AttachmentPointName);
                    BG_UnlockablesEquipClassToSlot(0, customClassName, 0, v3);
                }
            }
        }
    }
    else
    {
        Com_PrintError(
            15,
            "clearAttachmentForItemAtPoint usage: clearAttachmentForItemAtPoint <customClassName> <itemIndex> <attachmentPoint> \n");
    }
}

void __cdecl BG_UnlockablesSetCurrentItemIndexCmd()
{
    const char *v0; // eax

    if ( Cmd_Argc() >= 2 )
    {
        v0 = Cmd_Argv(1);
        sharedUiInfo.itemIndex = atoi(v0);
    }
    else
    {
        Com_PrintError(15, "SetCurrentItemIndex usage: SetCurrentItemIndex <itemIndex>\n");
    }
}

void __cdecl BG_UnlockablesCopyClassCmd()
{
    const char *v0; // eax
    const char *v1; // eax
    char *String; // eax
    const char *loadoutName; // [esp+18h] [ebp-24h]
    loadoutSlot_t currLoadout; // [esp+1Ch] [ebp-20h]
    const char *classToReplace; // [esp+20h] [ebp-1Ch]
    int classIndex; // [esp+24h] [ebp-18h]
    int classIndexToCopy; // [esp+2Ch] [ebp-10h]
    unsigned int itemIndex; // [esp+34h] [ebp-8h]
    const char *classToCopy; // [esp+38h] [ebp-4h]

    if ( Cmd_Argc() >= 3 )
    {
        v0 = Cmd_Argv(1);
        classIndexToCopy = atoi(v0);
        v1 = Cmd_Argv(2);
        classIndex = atoi(v1);
        if ( classIndex >= classIndexToCopy )
            ++classIndex;
        classToReplace = customClassDvars[classIndex];
        classToCopy = customClassDvars[classIndexToCopy];
        if ( classToCopy && classToReplace && *classToCopy && *classToReplace )
        {
            for ( currLoadout = LOADOUTSLOT_FIRST; currLoadout < LOADOUTSLOT_COUNT; ++currLoadout )
            {
                loadoutName = BG_UnlockablesGetLoadoutName(currLoadout);
                if ( loadoutName )
                {
                    itemIndex = BG_UnlockablesGetEquippedItemInSlot(0, classToCopy, loadoutName);
                    BG_UnlockablesEquipClassToSlot(0, classToReplace, itemIndex, loadoutName);
                }
            }
            BG_ReplaceItemsWithPurchasedProItem(0);
        }
        String = (char *)Dvar_GetString(classToCopy);
        Dvar_SetStringByName(classToReplace, String);
    }
    else
    {
        Com_PrintError(15, "copyclass usage: copyclass <classIndexToCopy> <classIndexToReplace>\n");
    }
}

void __cdecl BG_UnlockablesSetClanTagCmd()
{
    const char *v0; // eax
    const ddlState_t *RootDDLState; // eax
    persistentStats *buffer; // [esp+10h] [ebp-20h]
    int currentClanTagFeature; // [esp+14h] [ebp-1Ch]
    int clanTagFeatureIndex; // [esp+1Ch] [ebp-14h]
    ddlState_t searchState; // [esp+20h] [ebp-10h] BYREF

    if ( Cmd_Argc() >= 2 )
    {
        v0 = Cmd_Argv(1);
        clanTagFeatureIndex = atoi(v0);
        RootDDLState = LiveStats_GetRootDDLState();
        if ( DDL_MoveTo(RootDDLState, &searchState, 2, "clanTagStats", "selectedFeature") )
        {
            buffer = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_NORMAL, 1);
            currentClanTagFeature = DDL_GetInt(&searchState, (char *)buffer);
            DDL_SetInt(&searchState, currentClanTagFeature != clanTagFeatureIndex ? clanTagFeatureIndex : 0, (char *)buffer);
        }
        else
        {
            DDL_PrintError("DDL: Error could not find clantag feature index\n");
        }
    }
    else
    {
        Com_PrintError(15, "setClanTag usage: setClanTag <clantagFeatureIndex> \n");
    }
}

int __cdecl SV_GetMaxAttachCount()
{
    return 24;
}

