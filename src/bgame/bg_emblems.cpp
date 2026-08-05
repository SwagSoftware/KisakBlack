#include "bg_emblems.h"
#include <live/live_stats.h>
#include <live/live_storage.h>
#include <ddl/ddl_api.h>
#include <qcommon/common.h>
#include <live/live_counter.h>
#include <ui/ui_main.h>
#include <client/cl_compositing.h>
#include <universal/com_encode.h>
#include <game_mp/g_main_mp.h>

const dvar_s *allEmblemsUnlocked;
const dvar_s *allEmblemsPurchased;
const dvar_s *bg_overridePlayerEmblemColor;
const dvar_s *bg_overridePlayerEmblemIcon;

Material *s_missingEmblemMaterial;
EmblemSet *s_emblemSet;
BGDefaultEmblem s_defaultEmblems[24];

BackgroundPurchasedCount s_backgroundPurchasedCount[1];
ResultCache s_resultCache[4];
unsigned int s_flushCount;
unsigned int s_useCount;

const char *bg_iconNames[1024];

int __cdecl BG_EmblemsGetColorCount()
{
    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 59, 0, "%s", "s_emblemSet != NULL") )
    {
        __debugbreak();
    }
    return s_emblemSet->colorCount;
}

int __cdecl BG_EmblemsGetUnlockedLayerCount(int controllerIndex)
{
    int prestige; // [esp+4h] [ebp-Ch]
    int layer; // [esp+8h] [ebp-8h]
    int rank; // [esp+Ch] [ebp-4h]

    rank = LiveStats_GetRank(controllerIndex);
    prestige = LiveStats_GetPrestige(controllerIndex);
    if ( allEmblemsUnlocked && allEmblemsUnlocked->current.enabled )
        return s_emblemSet->layerCount;
    if ( prestige > 0 )
        return s_emblemSet->layerCount;
    for ( layer = 0; layer < s_emblemSet->layerCount && s_emblemSet->layers[layer].unlockLevel <= rank; ++layer )
        ;
    return layer;
}

int __cdecl BG_EmblemsGetLayerCost(int layer)
{
    if ( layer >= 0 && layer < s_emblemSet->layerCount )
        return s_emblemSet->layers[layer].cost;
    else
        return 0;
}

int __cdecl BG_EmblemsGetLayerUnlockLevel(int layer)
{
    if ( layer >= 0 && layer < s_emblemSet->layerCount )
        return s_emblemSet->layers[layer].unlockLevel;
    else
        return 0;
}

int __cdecl BG_EmblemsGetPurchasedLayerCount(int controllerIndex)
{
    const ddlState_t *RootDDLState; // eax
    persistentStats *buffer; // [esp+4h] [ebp-14h]
    ddlState_t searchState; // [esp+8h] [ebp-10h] BYREF

    if ( allEmblemsPurchased && allEmblemsPurchased->current.enabled )
        return BG_EmblemsGetUnlockedLayerCount(controllerIndex);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 2, "emblemStats", "layersPurchased") )
    {
        buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        return DDL_GetInt(&searchState, (char *)buffer);
    }
    else
    {
        DDL_PrintError("DDL: Error could not find layersPurchased\n");
        return 0;
    }
}

char __cdecl BG_EmblemsPurchaseLayer(int controllerIndex)
{
    const ddlState_t *RootDDLState; // eax
    persistentStats *buffer; // [esp+0h] [ebp-24h]
    int purchasedCount; // [esp+4h] [ebp-20h]
    EmblemLayer *layer; // [esp+8h] [ebp-1Ch]
    int currentCP; // [esp+10h] [ebp-14h]
    ddlState_t searchState; // [esp+14h] [ebp-10h] BYREF

    purchasedCount = BG_EmblemsGetPurchasedLayerCount(controllerIndex);
    if ( purchasedCount == BG_EmblemsGetUnlockedLayerCount(controllerIndex) )
    {
        Com_PrintError(15, "No layers available for purchase.\n");
        return 0;
    }
    else
    {
        layer = &s_emblemSet->layers[purchasedCount];
        currentCP = LiveStats_GetCurrency(controllerIndex);
        if ( currentCP >= layer->cost )
        {
            RootDDLState = LiveStats_GetRootDDLState();
            if ( DDL_MoveTo(RootDDLState, &searchState, 2, "emblemStats", "layersPurchased") )
            {
                buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
                if ( DDL_SetInt(&searchState, purchasedCount + 1, (char *)buffer) )
                {
                    LiveCounter_IncrementCounterValueByName("global_emblem_layerspurchased", 1u);
                    LiveStats_SpendCurrency(controllerIndex, layer->cost, POINTSSPENT_EMBLEM_LAYER, 0);
                    BG_EmblemsFlushResults();
                    return 1;
                }
                else
                {
                    DDL_PrintError("DDL: Error could not set purchased count to %d for layers\n", purchasedCount + 1);
                    return 0;
                }
            }
            else
            {
                DDL_PrintError("DDL: Error could not find layersPurchased\n");
                return 0;
            }
        }
        else
        {
            Com_PrintError(15, "Layer costs %d and you only have %d\n", layer->cost, currentCP);
            return 0;
        }
    }
}

void __cdecl BG_EmblemsFlushResults()
{
    ++s_flushCount;
}

ResultCache *__cdecl BG_EmblemsGetResults(int controllerIndex, unsigned int include, unsigned int exclude)
{
    EmblemIcon *icon; // [esp+0h] [ebp-1Ch]
    ResultCache *best; // [esp+4h] [ebp-18h]
    unsigned int category; // [esp+8h] [ebp-14h]
    int currency; // [esp+Ch] [ebp-10h]
    unsigned int i; // [esp+10h] [ebp-Ch]
    int ia; // [esp+10h] [ebp-Ch]
    ResultCache *results; // [esp+14h] [ebp-8h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp",
                    229,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 230, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    currency = LiveStats_GetCurrency(controllerIndex);
    best = s_resultCache;
    for ( i = 0; i < 4; ++i )
    {
        results = &s_resultCache[i];
        if ( results->include == include
            && results->exclude == exclude
            && results->controllerIndex == controllerIndex
            && results->flush == s_flushCount )
        {
            results->use = s_useCount++;
            return &s_resultCache[i];
        }
        if ( results->use < best->use )
            best = &s_resultCache[i];
    }
    memset(best, 0, sizeof(ResultCache));
    for ( ia = 0; ia < s_emblemSet->iconCount; ++ia )
    {
        icon = &s_emblemSet->icons[ia];
        category = icon->category;
        if ( !category )
            category = 0x80000000;
        if ( (category & 1) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp",
                        263,
                        0,
                        "%s",
                        "( category & BG_EMBLEM_MASK( LOCKED ) ) == 0") )
        {
            __debugbreak();
        }
        if ( (category & 2) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp",
                        264,
                        0,
                        "%s",
                        "( category & BG_EMBLEM_MASK( PURCHASED ) ) == 0") )
        {
            __debugbreak();
        }
        if ( icon->image )
        {
            if ( BG_EmblemsIsIconLocked(controllerIndex, ia) || BG_EmblemsIsIconLocked(controllerIndex, ia) )
                category |= 1u;
            if ( BG_EmblemsIsIconPurchased(controllerIndex, ia) )
                category |= 2u;
            if ( BG_EmblemsGetIconCost(ia) <= currency )
                category |= 4u;
            if ( (category & include) == include && (category & exclude) == 0 )
                best->ids[best->count++] = ia;
        }
    }
    qsort(best->ids, best->count, 2u, (int (__cdecl *)(const void *, const void *))ResultSort);
    best->controllerIndex = controllerIndex;
    best->use = s_useCount;
    best->flush = s_flushCount;
    best->include = include;
    best->exclude = exclude;
    ++s_useCount;
    return best;
}

int __cdecl ResultSort(__int16 *a, __int16 *b)
{
    return s_emblemSet->icons[*a].sortKey - s_emblemSet->icons[*b].sortKey;
}

int __cdecl BG_EmblemsGetIconCount(int controllerIndex, unsigned int include, unsigned int exclude)
{
    return BG_EmblemsGetResults(controllerIndex, include, exclude)->count;
}

int __cdecl BG_EmblemsGetIconCount()
{
    return s_emblemSet->iconCount;
}

__int16 __cdecl BG_EmblemsGetIconID(int controllerIndex, int index, unsigned int include, unsigned int exclude)
{
    ResultCache *results; // [esp+0h] [ebp-4h]

    results = BG_EmblemsGetResults(controllerIndex, include, exclude);
    if ( results->count > 0 )
        return results->ids[(results->count + index % results->count) % results->count];
    else
        return -1;
}

int __cdecl BG_EmblemsGetPurchasedIconCount(int controllerIndex)
{
    return BG_EmblemsGetIconCount(controllerIndex, 2u, 0);
}

const char *__cdecl BG_EmblemsGetIconDesc(int controllerIndex, __int16 id)
{
    int unclassifyAt; // [esp+4h] [ebp-4h]

    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 349, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    if ( id < 0 || id >= s_emblemSet->iconCount )
        return 0;
    if ( controllerIndex == -1 )
        return s_emblemSet->icons[id].description;
    unclassifyAt = BG_EmblemsGetIconUnclassifyAt(id);
    if ( BG_EmblemsGetPurchasedIconCount(controllerIndex) >= unclassifyAt )
        return s_emblemSet->icons[id].description;
    return "@MENU_CLASSIFIED";
}

const char *__cdecl BG_EmblemsGetIconUnlockDesc(int controllerIndex, __int16 id)
{
    char *v2; // eax
    char *v4; // eax
    char *v5; // eax
    int prestige; // [esp+4h] [ebp-18h]
    int purchasedCount; // [esp+8h] [ebp-14h]
    int unclassifyAt; // [esp+Ch] [ebp-10h]
    int rank; // [esp+10h] [ebp-Ch]
    int unlockPLevel; // [esp+14h] [ebp-8h]
    int unlockLevel; // [esp+18h] [ebp-4h]

    unlockLevel = BG_EmblemsGetIconUnlockLevel(id);
    unlockPLevel = BG_EmblemsGetIconUnlockPLevel(id);
    unclassifyAt = BG_EmblemsGetIconUnclassifyAt(id);
    purchasedCount = BG_EmblemsGetPurchasedIconCount(controllerIndex);
    rank = LiveStats_GetRank(controllerIndex);
    prestige = LiveStats_GetPrestige(controllerIndex);
    if ( purchasedCount >= unclassifyAt )
    {
        if ( unlockPLevel <= prestige )
        {
            if ( prestige <= 0 )
            {
                if ( unlockLevel <= rank )
                {
                    return "";
                }
                else
                {
                    v5 = UI_SafeTranslateString("PERKS_UNLOCKED_AT_LV");
                    return UI_ReplaceConversionInt(v5, unlockLevel + 1);
                }
            }
            else
            {
                return "";
            }
        }
        else
        {
            v4 = UI_SafeTranslateString("CLASS_PRESTIGE_UNLOCK_DESC");
            return UI_ReplaceConversionInt(v4, unlockPLevel);
        }
    }
    else if ( unclassifyAt - purchasedCount <= 1 )
    {
        return "@MENU_EMBLEM_ICON_CLASSIFIED_TOOLTIP_SING";
    }
    else
    {
        v2 = UI_SafeTranslateString("MENU_EMBLEM_ICON_CLASSIFIED_TOOLTIP");
        return UI_ReplaceConversionInt(v2, unclassifyAt - purchasedCount);
    }
}

int __cdecl BG_EmblemsGetIconCost(__int16 id)
{
    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 403, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    if ( id >= 0 && id < s_emblemSet->iconCount )
        return s_emblemSet->icons[id].cost;
    else
        return -1;
}

int __cdecl BG_EmblemsGetIconUnlockLevel(__int16 id)
{
    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 413, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    if ( id >= 0 && id < s_emblemSet->iconCount )
        return s_emblemSet->icons[id].unlockLevel;
    else
        return -1;
}

int __cdecl BG_EmblemsGetIconUnlockPLevel(__int16 id)
{
    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 423, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    if ( id >= 0 && id < s_emblemSet->iconCount )
        return s_emblemSet->icons[id].unlockPLevel;
    else
        return -1;
}

int __cdecl BG_EmblemsGetIconUnclassifyAt(__int16 id)
{
    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 433, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    if ( id >= 0 && id < s_emblemSet->iconCount )
        return s_emblemSet->icons[id].unclassifyAt;
    else
        return -1;
}

int __cdecl BG_EmblemsGetIconDefaultColor(__int16 id)
{
    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 444, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    if ( id >= 0 && id < s_emblemSet->iconCount )
        return s_emblemSet->icons[id].defaultColor;
    else
        return -1;
}

GfxImage *__cdecl BG_EmblemsGetIconImage(__int16 id)
{
    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 455, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    if ( id >= 0 && id < s_emblemSet->iconCount )
        return s_emblemSet->icons[id].image;
    else
        return 0;
}

double __cdecl BG_EmblemsGetIconOutlineSize(__int16 id)
{
    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 465, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    if ( id >= 0 && id < s_emblemSet->iconCount )
        return s_emblemSet->icons[id].outlineSize;
    else
        return 0.0;
}

bool __cdecl BG_EmblemsIsIconLocked(int controllerIndex, __int16 id)
{
    int prestige; // [esp+4h] [ebp-Ch]
    int unlockPLevel; // [esp+8h] [ebp-8h]
    int unlockLevel; // [esp+Ch] [ebp-4h]

    if ( allEmblemsUnlocked && allEmblemsUnlocked->current.enabled )
        return 0;
    unlockLevel = BG_EmblemsGetIconUnlockLevel(id);
    unlockPLevel = BG_EmblemsGetIconUnlockPLevel(id);
    prestige = LiveStats_GetPrestige(controllerIndex);
    if ( unlockPLevel )
        return unlockPLevel > prestige;
    if ( prestige <= 0 )
        return unlockLevel > LiveStats_GetRank(controllerIndex);
    return 0;
}

bool __cdecl BG_EmblemsIsIconClassified(int controllerIndex, __int16 id)
{
    int unclassifyAt; // [esp+4h] [ebp-8h]
    int unlockPLevel; // [esp+8h] [ebp-4h]

    if ( allEmblemsUnlocked && allEmblemsUnlocked->current.enabled )
        return 0;
    unlockPLevel = BG_EmblemsGetIconUnlockPLevel(id);
    if ( unlockPLevel && unlockPLevel > LiveStats_GetPrestige(controllerIndex) )
        return 1;
    unclassifyAt = BG_EmblemsGetIconUnclassifyAt(id);
    return unclassifyAt > BG_EmblemsGetPurchasedIconCount(controllerIndex);
}

bool __cdecl BG_EmblemsIsIconPurchased(int controllerIndex, __int16 id)
{
    const ddlState_t *RootDDLState; // eax
    char *v4; // [esp-8h] [ebp-20h]
    persistentStats *buffer; // [esp+4h] [ebp-14h]
    ddlState_t searchState; // [esp+8h] [ebp-10h] BYREF

    if ( allEmblemsPurchased && allEmblemsPurchased->current.enabled )
        return 1;
    if ( BG_EmblemsIsIconLocked(controllerIndex, id) )
        return 0;
    if ( !BG_EmblemsGetIconCost(id) )
        return 1;
    v4 = va("%d", id);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 4, "emblemStats", "icons", v4) )
    {
        buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        return DDL_GetInt(&searchState, (char *)buffer) != 0;
    }
    else
    {
        DDL_PrintError("DDL: Error could not find emblem icon %d\n", id);
        return 0;
    }
}

char __cdecl BG_EmblemsPurchaseIcon(int controllerIndex, __int16 id)
{
    const ddlState_t *RootDDLState; // eax
    char *v4; // [esp-8h] [ebp-24h]
    persistentStats *buffer; // [esp+0h] [ebp-1Ch]
    int currentCP; // [esp+4h] [ebp-18h]
    int cost; // [esp+8h] [ebp-14h]
    ddlState_t searchState; // [esp+Ch] [ebp-10h] BYREF

    if ( BG_EmblemsIsIconLocked(controllerIndex, id) )
    {
        Com_PrintError(15, "emblem icon %d is locked and cannot be purchased.\n", id);
        return 0;
    }
    else if ( BG_EmblemsIsIconPurchased(controllerIndex, id) )
    {
        Com_PrintError(15, "emblem icon %d is already purchased.\n", id);
        return 0;
    }
    else
    {
        cost = BG_EmblemsGetIconCost(id);
        currentCP = LiveStats_GetCurrency(controllerIndex);
        if ( currentCP >= cost )
        {
            v4 = va("%d", id);
            RootDDLState = LiveStats_GetRootDDLState();
            if ( DDL_MoveTo(RootDDLState, &searchState, 4, "emblemStats", "icons", v4) )
            {
                buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
                if ( DDL_SetInt(&searchState, 1u, (char *)buffer) )
                {
                    LiveCounter_IncrementCounterValueByName("global_emblem_imagespurchased", 1u);
                    LiveStats_SpendCurrency(controllerIndex, cost, POINTSSPENT_EMBLEM_ICON, 0);
                    BG_EmblemsFlushResults();
                    return 1;
                }
                else
                {
                    DDL_PrintError("DDL: Error could not purchase emblem icon %d\n", id);
                    return 0;
                }
            }
            else
            {
                DDL_PrintError("DDL: Error could not find emblem icon %d\n", id);
                return 0;
            }
        }
        else
        {
            Com_PrintError(15, "emblem icon %d costs %d and you only have %d\n", id, cost, currentCP);
            return 0;
        }
    }
}

int __cdecl BG_EmblemsGetCategoryCount()
{
    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 615, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    return s_emblemSet->categoryCount;
}

const char *__cdecl BG_EmblemsGetCategoryDesc(int index)
{
    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 659, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    if ( index >= 0 && index <= s_emblemSet->categoryCount )
        return s_emblemSet->categories[index].description;
    else
        return "";
}

int __cdecl BG_EmblemsGetBackgroundCount()
{
    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 669, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    return s_emblemSet->backgroundLookupCount;
}

int __cdecl BG_EmblemsGetTotalBackgroundCount()
{
    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 676, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    return s_emblemSet->backgroundCount;
}

int __cdecl BG_EmblemsGetPurchasedBackgroundCount(int controllerIndex)
{
    int total; // [esp+4h] [ebp-Ch]
    int idx; // [esp+8h] [ebp-8h]
    __int16 id; // [esp+Ch] [ebp-4h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp",
                    697,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( s_backgroundPurchasedCount[controllerIndex].flush == s_flushCount )
        return s_backgroundPurchasedCount[controllerIndex].count;
    s_backgroundPurchasedCount[controllerIndex].count = 0;
    total = BG_EmblemsGetBackgroundCount();
    for ( idx = 0; idx < total; ++idx )
    {
        id = BG_EmblemsGetBackgroundID(idx);
        if ( BG_EmblemsIsBackgroundPurchased(controllerIndex, id) )
            ++s_backgroundPurchasedCount[controllerIndex].count;
    }
    s_backgroundPurchasedCount[controllerIndex].flush = s_flushCount;
    return s_backgroundPurchasedCount[controllerIndex].count;
}

__int16 __cdecl BG_EmblemsGetBackgroundID(int index)
{
    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 722, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    if ( index < 0 || index >= s_emblemSet->backgroundLookupCount )
        return -1;
    else
        return s_emblemSet->backgroundLookup[index];
}

int __cdecl BG_EmblemsGetBackgroundIndex(__int16 id)
{
    int index; // [esp+0h] [ebp-4h]

    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 734, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    for ( index = 0; index < s_emblemSet->backgroundLookupCount; ++index )
    {
        if ( s_emblemSet->backgroundLookup[index] == id )
            return index;
    }
    return -1;
}

const char *__cdecl BG_EmblemsGetBackgroundDesc(int controllerIndex, __int16 id)
{
    int unclassifyAt; // [esp+4h] [ebp-4h]

    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 747, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    if ( id < 0 || id >= s_emblemSet->backgroundCount )
        return "";
    unclassifyAt = BG_EmblemsGetBackgroundUnclassifyAt(id);
    if ( BG_EmblemsGetPurchasedBackgroundCount(controllerIndex) >= unclassifyAt )
        return s_emblemSet->backgrounds[id].description;
    else
        return "MENU_CLASSIFIED";
}

const char *__cdecl BG_EmblemsGetBackgroundUnlockDesc(int controllerIndex, __int16 id)
{
    char *v2; // eax
    char *v4; // eax
    char *v5; // eax
    int prestige; // [esp+4h] [ebp-18h]
    int purchasedCount; // [esp+8h] [ebp-14h]
    int unclassifyAt; // [esp+Ch] [ebp-10h]
    int rank; // [esp+10h] [ebp-Ch]
    int unlockPLevel; // [esp+14h] [ebp-8h]
    int unlockLevel; // [esp+18h] [ebp-4h]

    unlockLevel = BG_EmblemsGetBackgroundUnlockLevel(id);
    unlockPLevel = BG_EmblemsGetBackgroundUnlockPLevel(id);
    unclassifyAt = BG_EmblemsGetBackgroundUnclassifyAt(id);
    purchasedCount = BG_EmblemsGetPurchasedBackgroundCount(controllerIndex);
    rank = LiveStats_GetRank(controllerIndex);
    prestige = LiveStats_GetPrestige(controllerIndex);
    if ( purchasedCount >= unclassifyAt )
    {
        if ( unlockPLevel <= prestige )
        {
            if ( prestige <= 0 )
            {
                if ( unlockLevel <= rank )
                {
                    return "";
                }
                else
                {
                    v5 = UI_SafeTranslateString("PERKS_UNLOCKED_AT_LV");
                    return UI_ReplaceConversionInt(v5, unlockLevel + 1);
                }
            }
            else
            {
                return "";
            }
        }
        else
        {
            v4 = UI_SafeTranslateString("CLASS_PRESTIGE_UNLOCK_DESC");
            return UI_ReplaceConversionInt(v4, unlockPLevel);
        }
    }
    else if ( unclassifyAt - purchasedCount <= 1 )
    {
        return "@MENU_EMBLEM_BACKGROUND_CLASSIFIED_TOOLTIP_SING";
    }
    else
    {
        v2 = UI_SafeTranslateString("MENU_EMBLEM_BACKGROUND_CLASSIFIED_TOOLTIP");
        return UI_ReplaceConversionInt(v2, unclassifyAt - purchasedCount);
    }
}

int __cdecl BG_EmblemsGetBackgroundCost(__int16 id)
{
    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 799, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    if ( id >= 0 && id < s_emblemSet->backgroundCount )
        return s_emblemSet->backgrounds[id].cost;
    else
        return -1;
}

int __cdecl BG_EmblemsGetBackgroundUnlockLevel(__int16 id)
{
    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 809, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    if ( id >= 0 && id < s_emblemSet->backgroundCount )
        return s_emblemSet->backgrounds[id].unlockLevel;
    else
        return -1;
}

int __cdecl BG_EmblemsGetBackgroundUnlockPLevel(__int16 id)
{
    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 819, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    if ( id >= 0 && id < s_emblemSet->backgroundCount )
        return s_emblemSet->backgrounds[id].unlockPLevel;
    else
        return -1;
}

int __cdecl BG_EmblemsGetBackgroundUnclassifyAt(__int16 id)
{
    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 829, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    if ( id >= 0 && id < s_emblemSet->backgroundCount )
        return s_emblemSet->backgrounds[id].unclassifyAt;
    else
        return 0;
}

Material *__cdecl BG_EmblemsGetBackgroundMaterial(__int16 id)
{
    if ( !s_emblemSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 839, 0, "%s", "s_emblemSet") )
    {
        __debugbreak();
    }
    if ( id >= 0 && id < s_emblemSet->backgroundCount )
        return s_emblemSet->backgrounds[id].material;
    else
        return s_missingEmblemMaterial;
}

char __cdecl BG_EmblemsIsBackgroundLocked(int controllerIndex, __int16 id)
{
    int prestige; // [esp+4h] [ebp-Ch]
    int unlockPLevel; // [esp+8h] [ebp-8h]
    int unlockLevel; // [esp+Ch] [ebp-4h]

    if ( allEmblemsUnlocked && allEmblemsUnlocked->current.enabled )
        return 0;
    if ( BG_EmblemsIsBackgroundClassified(controllerIndex, id) )
        return 1;
    unlockLevel = BG_EmblemsGetBackgroundUnlockLevel(id);
    unlockPLevel = BG_EmblemsGetBackgroundUnlockPLevel(id);
    prestige = LiveStats_GetPrestige(controllerIndex);
    if ( unlockPLevel )
        return unlockPLevel > prestige;
    if ( prestige <= 0 )
        return unlockLevel > LiveStats_GetRank(controllerIndex);
    return 0;
}

bool __cdecl BG_EmblemsIsBackgroundClassified(int controllerIndex, __int16 id)
{
    int unclassifyAt; // [esp+4h] [ebp-8h]
    int unlockPLevel; // [esp+8h] [ebp-4h]

    if ( allEmblemsUnlocked && allEmblemsUnlocked->current.enabled )
        return 0;
    unlockPLevel = BG_EmblemsGetBackgroundUnlockPLevel(id);
    if ( unlockPLevel && unlockPLevel > LiveStats_GetPrestige(controllerIndex) )
        return 1;
    unclassifyAt = BG_EmblemsGetBackgroundUnclassifyAt(id);
    return unclassifyAt > BG_EmblemsGetPurchasedBackgroundCount(controllerIndex);
}

bool __cdecl BG_EmblemsIsBackgroundPurchased(int controllerIndex, __int16 id)
{
    const ddlState_t *RootDDLState; // eax
    char *v4; // [esp-8h] [ebp-20h]
    persistentStats *buffer; // [esp+4h] [ebp-14h]
    ddlState_t searchState; // [esp+8h] [ebp-10h] BYREF

    if ( allEmblemsPurchased && allEmblemsPurchased->current.enabled )
        return 1;
    if ( !BG_EmblemsGetBackgroundCost(id) )
        return 1;
    v4 = va("%d", id);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 4, "emblemStats", "backgrounds", v4) )
    {
        buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        return DDL_GetInt(&searchState, (char *)buffer) != 0;
    }
    else
    {
        DDL_PrintError("DDL: Error could not find emblem background %d\n", id);
        return 0;
    }
}

char __cdecl BG_EmblemsPurchaseBackground(int controllerIndex, __int16 id)
{
    const ddlState_t *RootDDLState; // eax
    char *v4; // [esp-8h] [ebp-24h]
    persistentStats *buffer; // [esp+0h] [ebp-1Ch]
    int currentCP; // [esp+4h] [ebp-18h]
    int cost; // [esp+8h] [ebp-14h]
    ddlState_t searchState; // [esp+Ch] [ebp-10h] BYREF

    if ( BG_EmblemsIsBackgroundLocked(controllerIndex, id) )
    {
        Com_PrintError(15, "emblem background %d is locked and cannot be purchased.\n", id);
        return 0;
    }
    else if ( BG_EmblemsIsBackgroundPurchased(controllerIndex, id) )
    {
        Com_PrintError(15, "emblem background %d is already purchased.\n", id);
        return 0;
    }
    else
    {
        cost = BG_EmblemsGetBackgroundCost(id);
        currentCP = LiveStats_GetCurrency(controllerIndex);
        if ( currentCP >= cost )
        {
            v4 = va("%d", id);
            RootDDLState = LiveStats_GetRootDDLState();
            if ( DDL_MoveTo(RootDDLState, &searchState, 4, "emblemStats", "backgrounds", v4) )
            {
                buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
                if ( DDL_SetInt(&searchState, 1u, (char *)buffer) )
                {
                    LiveStats_SpendCurrency(controllerIndex, cost, POINTSSPENT_IGNORE, 0);
                    BG_EmblemsFlushResults();
                    return 1;
                }
                else
                {
                    DDL_PrintError("DDL: Error could not purchase emblem background %d\n", id);
                    return 0;
                }
            }
            else
            {
                DDL_PrintError("DDL: Error could not find emblem background %d\n", id);
                return 0;
            }
        }
        else
        {
            Com_PrintError(15, "emblem background %d costs %d and you only have %d\n", id, cost, currentCP);
            return 0;
        }
    }
}

void __cdecl BG_EmblemsWriteMsg(msg_t *msg, __int16 background, CompositeEmblemLayer *layers, int layerCount)
{
    CompositeEmblemLayer *layer; // [esp+10h] [ebp-8h]
    int i; // [esp+14h] [ebp-4h]

    if ( background >= 512
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 996, 0, "%s", "background < 512") )
    {
        __debugbreak();
    }
    MSG_WriteBits(msg, background, 9u);
    for ( i = 0; i < layerCount; ++i )
    {
        layer = &layers[i];
        if ( layer->icon == -1 )
        {
            MSG_WriteBit0(msg);
        }
        else
        {
            MSG_WriteBit1(msg);
            MSG_WriteBits(msg, layer->icon, 0xAu);
            MSG_WriteBits(msg, layer->colorIdx, 8u);
            MSG_WriteBits(msg, layer->outline, 1u);
            MSG_WriteBits(msg, layer->flip, 1u);
            MSG_WriteRangedFloatBits(msg, layer->pos[0], -1.0, 1.0, 8u);
            MSG_WriteRangedFloatBits(msg, layer->pos[1], -1.0, 1.0, 8u);
            MSG_WriteRangedFloatBits(msg, layer->scale[0], -4.0, 4.0, 8u);
            MSG_WriteRangedFloatBits(msg, layer->scale[1], -4.0, 4.0, 8u);
            MSG_WriteRangedFloatBits(msg, layer->angle, 0.0, 360.0, 9u);
        }
    }
}

void __cdecl BG_EmblemsReadMsg(msg_t *msg, __int16 *background, CompositeEmblemLayer *layers, int layerCount)
{
    CompositeEmblemLayer *layer; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    *background = MSG_ReadBits(msg, 9u);
    for ( i = 0; i < layerCount; ++i )
    {
        layer = &layers[i];
        CL_CompositeClearLayer(layer);
        if ( MSG_ReadBit(msg) )
        {
            if ( !msg->overflowed )
            {
                layer->icon = MSG_ReadBits(msg, 0xAu);
                layer->colorIdx = MSG_ReadBits(msg, 8u);
                layer->outline = MSG_ReadBit(msg) > 0;
                layer->flip = MSG_ReadBit(msg) > 0;
                layer->pos[0] = MSG_ReadRangedFloatBits(msg, -1.0, 1.0, 8u);
                layer->pos[1] = MSG_ReadRangedFloatBits(msg, -1.0, 1.0, 8u);
                layer->scale[0] = MSG_ReadRangedFloatBits(msg, -4.0, 4.0, 8u);
                layer->scale[1] = MSG_ReadRangedFloatBits(msg, -4.0, 4.0, 8u);
                layer->angle = MSG_ReadRangedFloatBits(msg, 0.0, 360.0, 9u);
            }
        }
    }
}

char *__cdecl BG_EmblemsWriteString(__int16 background, CompositeEmblemLayer *layers, int layerCount)
{
    static char result[256];
    unsigned __int8 msgbuf[132]; // [esp+0h] [ebp-B8h] BYREF
    msg_t msg; // [esp+88h] [ebp-30h] BYREF

    MSG_Init(&msg, msgbuf, 128);
    BG_EmblemsWriteMsg(&msg, background, layers, layerCount);
    if ( msg.overflowed
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 1061, 0, "%s", "!msg.overflowed") )
    {
        __debugbreak();
    }
    if ( msg.overflowed )
        return 0;
    Com_EncodeYEnc(msg.data, msg.cursize, result, 256);
    return result;
}

bool __cdecl BG_EmblemsReadString(const char *str, __int16 *background, CompositeEmblemLayer *layers, int layerCount)
{
    unsigned __int8 msgbuf[132]; // [esp+0h] [ebp-B8h] BYREF
    msg_t msg; // [esp+88h] [ebp-30h] BYREF

    MSG_Init(&msg, msgbuf, 128);
    msg.cursize = Com_DecodeYEnc(str, msgbuf, 128);
    BG_EmblemsReadMsg(&msg, background, layers, layerCount);
    return msg.overflowed == 0;
}

cmd_function_s BG_EmblemsPurchaseLayer_f_VAR;
cmd_function_s BG_EmblemsPurchaseIcon_f_VAR;
cmd_function_s BG_EmblemsPurchaseBackgroundByID_f_VAR;
cmd_function_s BG_EmblemsPurchaseBackgroundByIndex_f_VAR;
cmd_function_s BG_EmblemsFlushResults_VAR;
void __cdecl BG_EmblemsInit()
{
    int i; // [esp+4h] [ebp-4h]

    if ( !G_ExitAfterToolComplete() )
    {
        if (!IsDedicatedServer())
        {
            s_missingEmblemMaterial = Material_RegisterHandle("emblem_bg_nocod", 7);
        }

        s_emblemSet = DB_FindXAssetHeader(ASSET_TYPE_EMBLEMSET, (char*)"emblemset", 1, -1).emblemSet;
        memset(s_resultCache, 0, sizeof(s_resultCache));
        allEmblemsUnlocked = _Dvar_RegisterBool("allEmblemsUnlocked", 0, 0x80u, "Unlock all emblem icons and layers");
        allEmblemsPurchased = _Dvar_RegisterBool(
                                                        "allEmblemsPurchased",
                                                        0,
                                                        0x80u,
                                                        "Allows all emblem icons and layers to be purchased");
        for ( i = 0; i < s_emblemSet->iconCount; ++i )
            bg_iconNames[i] = s_emblemSet->icons[i].description;
        bg_iconNames[i] = 0;
        bg_overridePlayerEmblemColor = _Dvar_RegisterInt(
                                                                         "bg_overridePlayerEmblemColor",
                                                                         0,
                                                                         0,
                                                                         s_emblemSet->colorCount - 1,
                                                                         0,
                                                                         "Set to >=0 to override player emblem, value selects color");
        bg_overridePlayerEmblemIcon = _Dvar_RegisterEnum(
                                                                        "bg_overridePlayerEmblemIcon",
                                                                        bg_iconNames,
                                                                        0,
                                                                        0,
                                                                        "Override player emblem with a particular icon");
        Cmd_AddCommandInternal("emblemPurchaseLayer", BG_EmblemsPurchaseLayer_f, &BG_EmblemsPurchaseLayer_f_VAR);
        Cmd_AddCommandInternal("emblemPurchaseIcon", BG_EmblemsPurchaseIcon_f, &BG_EmblemsPurchaseIcon_f_VAR);
        Cmd_AddCommandInternal(
            "emblemPurchaseBackgroundByID",
            BG_EmblemsPurchaseBackgroundByID_f,
            &BG_EmblemsPurchaseBackgroundByID_f_VAR);
        Cmd_AddCommandInternal(
            "emblemPurchaseBackgroundByIndex",
            BG_EmblemsPurchaseBackgroundByIndex_f,
            &BG_EmblemsPurchaseBackgroundByIndex_f_VAR);
        Cmd_AddCommandInternal("emblemFlushResults", BG_EmblemsFlushResults, &BG_EmblemsFlushResults_VAR);
        BG_EmblemsClearDefaults();
        Cmd_ExecuteSingleCommand(0, 0, (char*)"exec mp/default_emblems.cfg");
    }
}

void __cdecl BG_EmblemsPurchaseLayer_f()
{
    BG_EmblemsPurchaseLayer(0);
}

void __cdecl BG_EmblemsPurchaseIcon_f()
{
    const char *v0; // eax
    __int16 v1; // ax

    v0 = Cmd_Argv(1);
    v1 = atoi(v0);
    BG_EmblemsPurchaseIcon(0, v1);
}

void __cdecl BG_EmblemsPurchaseBackgroundByID_f()
{
    const char *v0; // eax
    __int16 v1; // ax

    v0 = Cmd_Argv(1);
    v1 = atoi(v0);
    BG_EmblemsPurchaseBackground(0, v1);
}

void __cdecl BG_EmblemsPurchaseBackgroundByIndex_f()
{
    const char *v0; // eax
    int v1; // eax
    __int16 BackgroundID; // ax

    v0 = Cmd_Argv(1);
    v1 = atoi(v0);
    BackgroundID = BG_EmblemsGetBackgroundID(v1);
    BG_EmblemsPurchaseBackground(0, BackgroundID);
}

void __cdecl BG_EmblemsClearDefaults()
{
    int def; // [esp+0h] [ebp-4h]

    for ( def = 0; def < 24; ++def )
    {
        s_defaultEmblems[def].maxlevel = -1;
        s_defaultEmblems[def].minlevel = -1;
    }
}

void __cdecl BG_EmblemsSetDefaultForLevel(int minlevel, int maxlevel, CompositeEmblemLayer *s_emblem, int layerCount)
{
    int def; // [esp+0h] [ebp-4h]

    if ( minlevel < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 1152, 0, "%s", "minlevel >= 0") )
    {
        __debugbreak();
    }
    if ( maxlevel < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp", 1153, 0, "%s", "maxlevel >= 0") )
    {
        __debugbreak();
    }
    if ( minlevel > maxlevel
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp",
                    1154,
                    0,
                    "%s",
                    "minlevel <= maxlevel") )
    {
        __debugbreak();
    }
    for ( def = 0; ; ++def )
    {
        if ( def >= 24 )
        {
            Com_PrintError(16, "Ran out of default emblem slots\n");
            return;
        }
        if ( s_defaultEmblems[def].minlevel == -1 )
            break;
    }
    if ( s_defaultEmblems[def].maxlevel != -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_emblems.cpp",
                    1160,
                    0,
                    "%s",
                    "s_defaultEmblems[def].maxlevel == -1") )
    {
        __debugbreak();
    }
    s_defaultEmblems[def].minlevel = minlevel;
    s_defaultEmblems[def].maxlevel = maxlevel;
    s_defaultEmblems[def].layerCount = layerCount;
    memcpy((unsigned __int8 *)s_defaultEmblems[def].layers, (unsigned __int8 *)s_emblem, 32 * layerCount);
}

void __cdecl BG_EmblemsGenerateDefault(int controllerIndex, int level, CompositeEmblemLayer *layers, int layerCount)
{
    int v4; // [esp+0h] [ebp-14h]
    int def; // [esp+8h] [ebp-Ch]
    int layer; // [esp+Ch] [ebp-8h]

    for ( layer = 0; layer < layerCount; ++layer )
        CL_CompositeClearLayer(&layers[layer]);
    for ( def = 0; def < 24; ++def )
    {
        if ( s_defaultEmblems[def].minlevel <= level && s_defaultEmblems[def].maxlevel >= level )
        {
            if ( s_defaultEmblems[def].layerCount < layerCount )
                v4 = s_defaultEmblems[def].layerCount;
            else
                v4 = layerCount;
            memcpy((unsigned __int8 *)layers, (unsigned __int8 *)s_defaultEmblems[def].layers, 32 * v4);
        }
    }
}

unsigned int __cdecl SV_CACValidate_GetPurchasedEmblemLayers(unsigned __int8 *buffer)
{
    const ddlState_t *RootDDLState; // eax
    ddlState_t searchState; // [esp+0h] [ebp-10h] BYREF

    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 2, "emblemStats", "layersPurchased") )
        return DDL_GetInt(&searchState, (char *)buffer);
    else
        return 0;
}

bool __cdecl SV_CACValidate_IsBackgroundPurchased(unsigned __int8 *buffer, int rank, __int16 id)
{
    const ddlState_t *RootDDLState; // eax
    char *v5; // [esp-8h] [ebp-18h]
    ddlState_t searchState; // [esp+0h] [ebp-10h] BYREF

    if ( SV_CACValidate_IsBackgroundLockedForRank(buffer, rank, id) )
        return 0;
    if ( !BG_EmblemsGetBackgroundCost(id) )
        return 1;
    v5 = va("%d", id);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 4, "emblemStats", "backgrounds", v5) )
        return DDL_GetInt(&searchState, (char *)buffer) != 0;
    DDL_PrintError("DDL: Error could not find emblem background %d\n", id);
    return 0;
}

bool __cdecl SV_CACValidate_IsBackgroundLockedForRank(unsigned __int8 *buffer, int rank, __int16 id)
{
    return BG_EmblemsGetBackgroundUnlockLevel(id) > rank;
}

bool __cdecl SV_CACValidate_IsIconPurchased(unsigned __int8 *buffer, int rank, __int16 id)
{
    const ddlState_t *RootDDLState; // eax
    char *v5; // [esp-8h] [ebp-18h]
    ddlState_t searchState; // [esp+0h] [ebp-10h] BYREF

    if ( SV_CACValidate_IsIconLockedForRank(buffer, rank, id) )
        return 0;
    if ( !BG_EmblemsGetIconCost(id) )
        return 1;
    v5 = va("%d", id);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 4, "emblemStats", "icons", v5) )
        return DDL_GetInt(&searchState, (char *)buffer) != 0;
    DDL_PrintError("DDL: Error could not find emblem icon %d\n", id);
    return 0;
}

bool __cdecl SV_CACValidate_IsIconLockedForRank(unsigned __int8 *buffer, int rank, __int16 id)
{
    return BG_EmblemsGetIconUnlockLevel(id) > rank;
}

