#pragma once

#include <universal/dvar.h>
#include <universal/memfile.h>

#include "ui_gametype_variants_mp.h"

struct FileShareReadFileTask;
struct MemoryFile;

struct UIGametypeFileHeader // sizeof=0xC8
{                                       // XREF: ?UI_Gametype_ReadFromMemFile@@YA_NAAUMemoryFile@@@Z/r
                                        // ?UI_Gametype_UploadToFileShare_f@@YAXXZ/r
    int version;
    char name[32];                      // XREF: UI_Gametype_UploadToFileShare_f(void)+39/o
    char description[128];
    char baseGameType[32];              // XREF: UI_Gametype_ReadFromMemFile(MemoryFile &)+2F/o
                                        // UI_Gametype_ReadFromMemFile(MemoryFile &):loc_73E854/o
    int lastModifiedDateTime;
};

struct CustomGameModeData // sizeof=0x4C
{                                       // XREF: .data:CustomGameModeData g_customGameModeData/r
    float perk_extraBreath;             // XREF: UI_Gametype_Custom_WritePerkDvarClientData(msg_t *)+D/r
                                        // UI_Gametype_Custom_ReadPerkDvarClientData(msg_t *)+F/w ...
    float perk_bulletPenetrationMultiplier;
                                        // XREF: UI_Gametype_Custom_WritePerkDvarClientData(msg_t *)+22/r
                                        // UI_Gametype_Custom_ReadPerkDvarClientData(msg_t *)+21/w ...
    float perk_weapSpreadMultiplier;    // XREF: UI_Gametype_Custom_WritePerkDvarClientData(msg_t *)+37/r
                                        // UI_Gametype_Custom_ReadPerkDvarClientData(msg_t *)+33/w ...
    float perk_weapReloadMultiplier;    // XREF: UI_Gametype_Custom_WritePerkDvarClientData(msg_t *)+4C/r
                                        // UI_Gametype_Custom_ReadPerkDvarClientData(msg_t *)+45/w ...
    float perk_weapMeleeMultiplier;     // XREF: UI_Gametype_Custom_WritePerkDvarClientData(msg_t *)+61/r
                                        // UI_Gametype_Custom_ReadPerkDvarClientData(msg_t *)+57/w ...
    float perk_weapSwitchMultiplier;    // XREF: UI_Gametype_Custom_WritePerkDvarClientData(msg_t *)+76/r
                                        // UI_Gametype_Custom_ReadPerkDvarClientData(msg_t *)+69/w ...
    float perk_weapAdsMultiplier;       // XREF: UI_Gametype_Custom_WritePerkDvarClientData(msg_t *)+8B/r
                                        // UI_Gametype_Custom_ReadPerkDvarClientData(msg_t *)+7B/w ...
    float perk_sprintMultiplier;        // XREF: UI_Gametype_Custom_WritePerkDvarClientData(msg_t *)+A0/r
                                        // UI_Gametype_Custom_ReadPerkDvarClientData(msg_t *)+8D/w ...
    float perk_sprintRecoveryMultiplier;
                                        // XREF: UI_Gametype_Custom_WritePerkDvarClientData(msg_t *)+B5/r
                                        // UI_Gametype_Custom_ReadPerkDvarClientData(msg_t *)+9F/w ...
    float player_lastStandBleedoutTime; // XREF: UI_Gametype_Custom_WritePerkDvarClientData(msg_t *)+CA/r
                                        // UI_Gametype_Custom_ReadPerkDvarClientData(msg_t *)+B1/w ...
    float perk_speedMultiplier;         // XREF: UI_Gametype_CustomGameModeDataToDvars(void)+102/r
    float perk_flakJacket;              // XREF: UI_Gametype_CustomGameModeDataToDvars(void)+11A/r
    float perk_fireproof;               // XREF: UI_Gametype_CustomGameModeDataToDvars(void)+131/r
    float perk_armorPiercing;           // XREF: UI_Gametype_CustomGameModeDataToDvars(void)+149/r
    float player_lastStandBleedoutTimeNoRevive;
                                        // XREF: UI_Gametype_CustomGameModeDataToDvars(void)+161/r
    float revive_time_taken;            // XREF: UI_Gametype_CustomGameModeDataToDvars(void)+178/r
    float perk_disarmExplosiveTime;     // XREF: UI_Gametype_CustomGameModeDataToDvars(void)+190/r
    float perk_damageKickReduction;     // XREF: UI_Gametype_CustomGameModeDataToDvars(void)+1A8/r
    int perk_killstreakReduction;       // XREF: UI_Gametype_CustomGameModeDataToDvars(void)+1BF/r
};

void __cdecl EnableMinimumCustomClasses();
void __cdecl UI_Gametype_ShowErrorPopup(char *title, char *message);
void __cdecl UI_Gametype_WriteDvarChunk(MemoryFile *memFile, char *dvarName);
char __cdecl UI_Gametype_ReadDvarChunk(MemoryFile *memFile);
void __cdecl MemFile_ReadVec3(MemoryFile *memFile, float *value);
const dvar_s *__cdecl RegisterDvarIfUndefined(const char *name, int defaultValue);
void __cdecl RegisterCustomClassDvarsIfNeeded();
void __cdecl GetClassLoadoutForClassName(const char *className, unsigned int customClassSlotNum);
void __cdecl UI_Gametype_BuildDefaultCustomClasses();
void __cdecl UI_Gametype_CustomClassesReset();
char __cdecl UI_Gametype_ReadCustomClassChunk(MemoryFile *memFile);
char __cdecl UI_Gametype_ReadCustomClassDescriptionChunk(MemoryFile *memFile);
bool __cdecl UI_Gametype_HasDvarChanged(const dvar_s *dvar);
bool __cdecl UI_Gametype_HasDvarChanged(const char *dvarName);
void __cdecl UI_Gametype_WriteDvarIfChanged(MemoryFile *memFile, char *dvarName);
void __cdecl UI_Gametype_WriteDvars(MemoryFile *memFile);
char __cdecl HasClassDescriptionDataChanged();
void __cdecl UI_Gametype_WriteCustomClasses(MemoryFile *memFile);
void __cdecl UI_Gametype_WriteHeader(MemoryFile *memFile, UIGametypeFileHeader *header);
void __cdecl UI_Gametype_ReadHeader(MemoryFile *memFile, UIGametypeFileHeader *header);
void __cdecl UI_Gametype_WriteToMemFile(MemoryFile *memFile, UIGametypeFileHeader *header);
void __cdecl UI_Gametype_CopyDvarsToCustom();
void __cdecl UI_Gametype_CopyCustomToDvars();
char __cdecl UI_Gametype_HasAnyDvarsChanged(const char **dvarNameList, int dvarNameListSize, bool areGameSpecificDvars);
void __cdecl UI_Gametype_CheckIfAnythingChanged_f();
void __cdecl CheckForMinimumTeamClasses();
void __cdecl UI_Gametype_UpdateCustomClassDvarsForClass_f();
void __cdecl UI_Gametype_UpdateClassDataFromDvars_f();
void __cdecl UI_Gametype_CreateDefaultCustomClassIfNeeded_f();
void __cdecl UI_Gametype_ApplyCustomClassTeamSelections_f();
void __cdecl UI_Gametype_UpdateCustomClassTeamDvars_f();
char __cdecl UI_Gametype_ReadChunk(MemoryFile *memFile);
char __cdecl UI_Gametype_ReadChunks(MemoryFile *memFile);
char __cdecl UI_Gametype_ReadFromMemFile(MemoryFile *memFile);
void __cdecl ResetCustomClassDvars();
void __cdecl UI_Gametype_ResetDvars();
void __cdecl UI_Gametype_EndChanges();
void __cdecl UI_Gametype_EndChanges_f();
void __cdecl UI_Gametype_SetupCustomModeNameAndDesc();
void __cdecl UI_Gametype_BeginChanges_f();
bool __cdecl UI_Gametype_CanUseCustom();
bool __cdecl UI_Gametype_IsUsingCustom();
void __cdecl UI_Gametype_ResetGamemode();
void __cdecl UI_Gametype_Custom_Reset_f();
void __cdecl UI_Gametype_RevertChanges_f();
bool __cdecl isValidClassSlotNum(unsigned int slotNum);
unsigned __int8 __cdecl GetCustomClassLoadoutItemForSlot(
                int controllerIndex,
                unsigned int slotNum,
                const char *itemName);
unsigned __int8 __cdecl GetCustomClassLoadoutItemForClassName(
                int controllerIndex,
                const char *className,
                const char *itemName);
int __cdecl GetCustomClassModifierForClass(int controllerIndex, unsigned int classNum, const char *itemName);
void __cdecl UI_Gametype_EquipCustomClassLoadoutItem(
                int controllerIndex,
                const char *customClassName,
                unsigned __int8 itemIndex,
                const char *loadoutName);

void __cdecl UI_Gametype_Custom_WriteClassClientData(msg_t *msg);
void __cdecl UI_Gametype_Custom_ReadClassClientData(msg_t *msg);
void __cdecl UI_Gametype_Custom_WriteKillstreakClientData(msg_t *msg);
void __cdecl UI_Gametype_Custom_ReadKillstreakClientData(msg_t *msg);
void __cdecl UI_Gametype_Custom_WritePerkDvarClientData(msg_t *msg);
void __cdecl UI_Gametype_Custom_ReadPerkDvarClientData(msg_t *msg);
void __cdecl UI_Gametype_Custom_WriteClientData(msg_t *msg);
char __cdecl UI_Gametype_Custom_ReadClientData(msg_t *msg);
bool __cdecl UI_Gametype_IsDirty();
void __cdecl UI_Gametype_MarkDirty();
bool __cdecl UI_Gametype_HasPerkChangedFromDefault(int itemIndex);
bool __cdecl UI_Gametype_Custom_IsPerkTweakable(int itemIndex);
void __cdecl UI_Gametype_Custom_ResolveNumKillsConflicts();
void __cdecl UI_Gametype_Custom_UploadToFileShareSuccess(int controllerIndex, unsigned __int64 fileID);
void __cdecl UI_Gametype_Custom_UploadToFileShareFailure();
void __cdecl UI_Gametype_Custom_UploadToFileShare(
                int controllerIndex,
                unsigned __int16 fileSlot,
                const char *fileName,
                MemoryFile *customGameMode);
void __cdecl UI_Gametype_UploadToFileShare_f();
void __cdecl UI_Gametype_FileShareDownloadComplete(FileShareReadFileTask *task);
void __cdecl UI_Gametype_FileShareDownloadFailed(FileShareReadFileTask *task);
void __cdecl UI_Gametype_DownloadFromFileShare_f();
void __cdecl SV_GameType_DownloadFromFileshare_f();
void __cdecl UI_Gametype_DisableCheats_f();
void __cdecl UI_Gametype_CopyCustomClass_f();
void __cdecl UI_Gametype_Custom_Init();
void __cdecl UI_Gametype_CustomGameModeDataToDvars();
void __cdecl UI_Gametype_DvarsToCustomGameModeData(CustomGameModeData *customGameModeData);

extern const dvar_t *custom_class_name;
extern const dvar_t *custom_class_create_enabled;
extern const dvar_t *custom_class_team;
extern const dvar_t *custom_class_health;
extern const dvar_t *custom_class_health_regen;
extern const dvar_t *custom_class_health_vampirism;
extern const dvar_t *custom_class_speed;
extern const dvar_t *custom_class_sprint_speed;
extern const dvar_t *custom_class_damage;
extern const dvar_t *custom_class_explosive_damage;
extern const dvar_t *ui_items_no_cost;
extern const dvar_t *custom_killstreak_mode;
extern const dvar_t *custom_killstreak_numkills;
extern const dvar_t *ui_useCustomClassInfo;

extern _CustomClassDescription g_customGameModeClassDescriptions[10];
