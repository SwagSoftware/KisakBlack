#pragma once
#include <database/db_registry.h>
#include <bgame/bg_weapons_attachment.h>

enum milestoneTableColumns_t : __int32
{                                       // XREF: ?GetStatsMilestoneData@@YAPBDW4milestoneTableColumns_t@@PBD01010@Z/r
                                        // ?GetStatsMilestoneData@@YAPBDW4milestoneTableColumns_t@@PBD01010@Z/r ...
    MILESTONE_COLUMN_INDEX = 0x0,
    MILESTONE_COLUMN_TIERID = 0x1,
    MILESTONE_COLUMN_TARGETVALUE = 0x2,
    MILESTONE_COLUMN_STATTYPE = 0x3,
    MILESTONE_COLUMN_STATNAME = 0x4,
    MILESTONE_COLUMN_STRING = 0x5,
    MILESTONE_COLUMN_XPEARNED = 0x6,
    MILESTONE_COLUMN_CPEARNED = 0x7,
    MILESTONE_COLUMN_EXCLUDE = 0x8,
    MILESTONE_COLUMN_UNLOCKITEM = 0x9,
    MILESTONE_COLUMN_PERKNAME = 0xA,
};

enum statsMilestoneTypes_t : __int32
{                                       // XREF: statsMilestone_t/r
                                        // challengeInfo_t/r ...
    MILESTONE_GLOBAL = 0x0,
    MILESTONE_WEAPON = 0x1,
    MILESTONE_MAP = 0x2,
    MILESTONE_GAMEMODE = 0x3,
    MILESTONE_GROUP = 0x4,
    MILESTONE_ATTACHMENTS = 0x5,
    MILESTONE_KILLSTREAKS = 0x6,
    MILESTONE_GRENADES = 0x7,
    MILESTONE_COUNT = 0x8,
    MILESTONE_INVALID = -1,
};

int __cdecl CL_GetMilestoneRowNum(
                XAssetHeader *stringTable,
                int comparisonColumn1,
                const char *value1,
                int comparisonColumn2,
                const char *value2,
                int comparisonColumn3,
                const char *value3);
const char *__cdecl CL_GetMilestoneData(const char *indexName, milestoneTableColumns_t column);
void __cdecl CL_GetFrontEndMilestoneLocalizedName(
                char *outputString,
                int stringLength,
                unsigned int index,
                statsMilestoneTypes_t milestoneType,
                const char *challengeType);
char __cdecl CL_GetInGameMilestoneLocalizedName(
                int localClientNum,
                char *outputString,
                int stringLength,
                const char *statName,
                statsMilestoneTypes_t milestoneType,
                eAttachment statsTableIndex,
                int milestoneTier,
                const char *weaponType);
char __cdecl CL_GetMilestoneLocalizedName(
                char *outputString,
                int stringLength,
                const char *statName,
                statsMilestoneTypes_t milestoneType,
                eAttachment statsTableIndex,
                int milestoneTier,
                bool isItemClassified,
                const char *localizedString);
char __cdecl FormatStatsMilestoneName(
                char *outputString,
                int stringLength,
                int milestoneTier,
                const char *statsName,
                const char *itemName);
char __cdecl PrepareGlobalChallengeName(const char *name, char *outputString, int stringLength, int milestoneTier);
char __cdecl PrepareKillstreakChallengeName(
                const char *name,
                char *outputString,
                int stringLength,
                int index,
                int tierId);
char __cdecl PrepareGrenadeChallengeName(const char *name, char *outputString, int stringLength, int index, int tierId);
char __cdecl PrepareAttachmentChallengeName(
                const char *name,
                char *outputString,
                int stringLength,
                eAttachment index,
                int tierId);
char __cdecl PrepareWeaponChallengeName(
                const char *name,
                char *outputString,
                int stringLength,
                int index,
                int tierId,
                bool isItemClassified);
char __cdecl PrepareGroupChallengeName(
                const char *name,
                char *outputString,
                int stringLength,
                int index,
                int tierId,
                const char *weaponNameLocalizedString);
char __cdecl PrepareGameModeChallengeName(
                const char *name,
                char *outputString,
                int stringLength,
                int index,
                int tierId,
                const char *gameModeLocalizedString);
void __cdecl CL_GetFrontEndMilestoneLocalizedDesc(
                char *outputString,
                int stringLength,
                unsigned int index,
                statsMilestoneTypes_t milestoneType,
                const char *challengeType);
char __cdecl CL_GetInGameMilestoneLocalizedDesc(
                int localClientNum,
                char *outputString,
                int stringLength,
                const char *statName,
                statsMilestoneTypes_t milestoneType,
                int statsTableIndex,
                int milestoneTier,
                const char *weaponType,
                int targetValue);
char __cdecl CL_GetMilestoneLocalizedDesc(
                char *outputString,
                int stringLength,
                const char *statName,
                statsMilestoneTypes_t milestoneType,
                int statsTableIndex,
                int milestoneTier,
                int targetValue,
                const char *localizedString);
char __cdecl GetGlobalChallengeDescription(const char *name, char *outputString, int stringLength, int targetValue);
char __cdecl PrepareAttachmentDescription(
                const char *name,
                char *outputString,
                int stringLength,
                int index,
                int targetValue);
char __cdecl PrepareKillstreakDescription(
                const char *name,
                char *outputString,
                int stringLength,
                int index,
                int targetValue);
char __cdecl PrepareGrenadeDescription(
                const char *name,
                char *outputString,
                int stringLength,
                int index,
                int targetValue);
char __cdecl PrepareWeaponDescription(
                const char *name,
                char *outputString,
                int stringLength,
                int targetValue,
                int milestoneType,
                const char *weaponLocalizedString);
char __cdecl PrepareGameModeDescription(
                const char *name,
                char *outputString,
                int stringLength,
                int targetValue,
                const char *gameModeLocalizedString);
char __cdecl CL_GetMilestoneLocalizedXP(
                char *outputString,
                int stringLength,
                float xpScale,
                int tier,
                const char *indexName);
void __cdecl CL_GetMilestoneBackingMaterial(
                char *backingMaterialName,
                int stringLength,
                const char *statName,
                const char *perkName);
void __cdecl CL_GetMilestoneMaterial(char *materialName, int stringLength, const char *statName, const char *perkName);
statsMilestoneTypes_t __cdecl getMilestoneType(const char *statsType);
void __cdecl CL_MilestoneTier_DecToRoman(int milestoneTierId, char *milestoneTierRoman, int stringLength);
