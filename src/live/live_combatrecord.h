#pragma once
#include <qcommon/common.h>

enum sortedItemParams_t : __int32
{                                       // XREF: ?LiveCombatRecord_GetSortedItemData@@YAMH_NW4sortedItemParams_t@@@Z/r
    ITEM_INDEX               = 0x0,
    PARAM1                   = 0x1,
    PARAM2                   = 0x2,
    PARAM3                   = 0x3,
    SORT_KEY                 = 0x4,
    SORTEDITEM_PARAM_COUNT   = 0x5,
    SORTEDITEM_PARAM_INVALID = -1,
};

enum itemSortingCriteria_t : __int32
{                                       // XREF: LiveCombatRecord_CombatRecordViewUpdateCmd/r
                                        // LiveCombatRecord_BuildSortedItemListByStats/r ...
    CRITERION_NULL             = -1,
    PERSONAL_BESTS             = 0x0,
    RECENT_PERFORMANCE         = 0x1,
    FAVORITE_WEAPON            = 0x2,
    WEAPON_STATS               = 0x3,
    WEAPON_ACCURACY            = 0x4,
    WEAPON_HEADSHOTS           = 0x5,
    WEAPON_HITLOCATION_HEATMAP = 0x6,
    EQUIPMENT_KILLS            = 0x7,
    EQUIPMENT_USAGE            = 0x8,
    TIME_SPENT_USING_ITEM      = 0x9,
    MATCH_TYPES                = 0xA,
    WAGER_MATCH_TYPES          = 0xB,
    MATCH_PERFORMANCE          = 0xC,
    RECENT_EARNINGS            = 0xD,
    KILLSTREAKS_FAVORITES      = 0xE,
    KILLSTREAKS_CALLED         = 0xF,
    KILLSTREAKS_KILLS          = 0x10,
};

enum playerStatsKeyIndex_t : __int32;

enum sortedStatLoadoutSlots_t : __int32
{                                       // XREF: LiveCombatRecord_FilterCurrentSortedItem/r
    SLOT_NULL                    = -1,
    PRIMARY_WEAPON               = 0x0,
    SECONDARY_WEAPON             = 0x1,
    PRIMARY_GRENADE              = 0x2,
    SPECIAL_GRENADE              = 0x3,
    EQUIPMENT                    = 0x4,
    FACEPAINT_PATTERN            = 0x5,
    BODY                         = 0x6,
    SPECIALTY1                   = 0x7,
    SPECIALTY2                   = 0x8,
    SORTEDSTAT_LOADOUTSLOT_COUNT = 0x9,
};

enum loadoutSlot_t : __int32
{                                       // XREF: itemInfo_t/r
                                        // LiveCombatRecord_CompareItemsByStats/r ...
    LOADOUTSLOT_INVALID              = -1,
    LOADOUTSLOT_FIRST                = 0x0,
    LOADOUTSLOT_PRIMARY_WEAPON       = 0x0,
    LOADOUTSLOT_PRIMARY_ATTACHMENT   = 0x1,
    LOADOUTSLOT_PRIMARY_ATTACHMENT_TOP = 0x2,
    LOADOUTSLOT_PRIMARY_ATTACHMENT_BOTTOM = 0x3,
    LOADOUTSLOT_PRIMARY_ATTACHMENT_TRIGGER = 0x4,
    LOADOUTSLOT_PRIMARY_ATTACHMENT_MUZZLE = 0x5,
    LOADOUTSLOT_PRIMARY_CAMO         = 0x6,
    LOADOUTSLOT_PRIMARY_RETICLE      = 0x7,
    LOADOUTSLOT_PRIMARY_RETICLE_COLOR = 0x8,
    LOADOUTSLOT_PRIMARY_LENS         = 0x9,
    LOADOUTSLOT_PRIMARY_EMBLEM       = 0xA,
    LOADOUTSLOT_PRIMARY_TAG          = 0xB,
    LOADOUTSLOT_SECONDARY_WEAPON     = 0xC,
    LOADOUTSLOT_SECONDARY_ATTACHMENT = 0xD,
    LOADOUTSLOT_SECONDARY_ATTACHMENT_TOP = 0xE,
    LOADOUTSLOT_SECONDARY_ATTACHMENT_BOTTOM = 0xF,
    LOADOUTSLOT_SECONDARY_ATTACHMENT_TRIGGER = 0x10,
    LOADOUTSLOT_SECONDARY_ATTACHMENT_MUZZLE = 0x11,
    LOADOUTSLOT_SECONDARY_CAMO       = 0x12,
    LOADOUTSLOT_SECONDARY_RETICLE    = 0x13,
    LOADOUTSLOT_SECONDARY_RETICLE_COLOR = 0x14,
    LOADOUTSLOT_SECONDARY_LENS       = 0x15,
    LOADOUTSLOT_SECONDARY_EMBLEM     = 0x16,
    LOADOUTSLOT_SECONDARY_TAG        = 0x17,
    LOADOUTSLOT_FIRST_SPECIALTY      = 0x18,
    LOADOUTSLOT_SPECIALTY1           = 0x18,
    LOADOUTSLOT_SPECIALTY2           = 0x19,
    LOADOUTSLOT_SPECIALTY3           = 0x1A,
    LOADOUTSLOT_LAST_SPECIALTY       = 0x1A,
    LOADOUTSLOT_CLASSBONUS           = 0x1B,
    LOADOUTSLOT_HEAD                 = 0x1C,
    LOADOUTSLOT_BODY                 = 0x1D,
    LOADOUTSLOT_PRIMARY_GRENADE      = 0x1E,
    LOADOUTSLOT_SPECIAL_GRENADE      = 0x1F,
    LOADOUTSLOT_EQUIPMENT            = 0x20,
    LOADOUTSLOT_FACEPAINT_PATTERN    = 0x21,
    LOADOUTSLOT_FACEPAINT_COLOR      = 0x22,
    LOADOUTSLOT_GLOBAL_ITEMS_START   = 0x23,
    LOADOUTSLOT_KILLSTREAK1          = 0x23,
    LOADOUTSLOT_KILLSTREAK2          = 0x24,
    LOADOUTSLOT_KILLSTREAK3          = 0x25,
    LOADOUTSLOT_DEATHSTREAK          = 0x26,
    LOADOUTSLOT_COUNT                = 0x27,
};
inline loadoutSlot_t &operator++(loadoutSlot_t &t)
{
    t = static_cast<loadoutSlot_t>((static_cast<int>(t) + 1));
    return t;
}
inline loadoutSlot_t operator++(loadoutSlot_t &t, int)
{
    loadoutSlot_t old = t;
    t = static_cast<loadoutSlot_t>((static_cast<int>(t) + 1));
    return old;
}

struct sortedItemsData_t // sizeof=0x14
{                                       // XREF: LiveCombatRecord_BuildSortedItemListByStats+50C/o
                                        // .data:s_otherPlayerSortedItemList/r ...
    int itemIndex;
    float sortKey;
    int param1;
    int param2;
    int param3;
};

void __cdecl LiveCombatRecord_OpenFriendsCombatRecord(int localClientNum);
double __cdecl LiveCombatRecord_GetSortedItemData(int index, bool forFriend, sortedItemParams_t param);
int __cdecl LiveCombatRecord_GetSortedItemPositionByItemIndex(int itemIndex, bool forFriend);
const char *__cdecl LiveCombatRecord_GetEquipmentUsageStringSuffix(const char *equipmentName);
const char *__cdecl LiveCombatRecord_GetPersonalBestStatNameByIndex(int index);
const char *__cdecl LiveCombatRecord_GetGameTypeName(unsigned int index);
int __cdecl LiveCombatRecord_GetGameTypeLbIndex(unsigned int index);
int __cdecl LiveCombatRecord_GetGameTypeCount();
const char *__cdecl LiveCombatRecord_GetHitLocationImageRef(hitLocation_t hitLocation);
void __cdecl LiveCombatRecord_Init();
void __cdecl LiveCombatRecord_CombatRecordViewUpdateCmd();
void __cdecl LiveCombatRecord_BuildSortedItemListByStats(
                int controllerIndex,
                itemSortingCriteria_t criterion,
                playerStatsKeyIndex_t param,
                playerStatsKeyIndex_t divisionParam);
int __cdecl LiveCombatRecord_CompareItemsByStats(float *arg0, float *arg1);
int __cdecl LiveCombatRecord_CompareMatchCount(float *arg0, float *arg1);
void __cdecl LiveCombatRecord_BuildSortedItem(
                int controllerIndex,
                itemSortingCriteria_t criterion,
                bool forOtherPlayer,
                int index,
                int itemNumber,
                playerStatsKeyIndex_t param,
                playerStatsKeyIndex_t divisionParam);
bool __cdecl LiveCombatRecord_FilterCurrentSortedItem(
                int controllerIndex,
                itemSortingCriteria_t criterion,
                int itemNumber,
                sortedStatLoadoutSlots_t currLoadout);

extern const dvar_t *ui_showFriendsCombatRecord;
extern const dvar_t *ui_combatRecordOpen;
extern const dvar_t *ui_combatCurrView;
extern const dvar_t *ui_combatCurrViewNum;
extern const dvar_t *ui_combatComparisonModeOn;
extern const dvar_t *ui_combatOtherPlayerStatsAvailable;
extern const dvar_t *ui_combatHistogramCurrGametype;