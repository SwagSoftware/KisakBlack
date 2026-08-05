#include "ui_gametype_custom_mp.h"
#include "ui_gametype_variants_mp.h"
#include <bgame/bg_unlockable_items.h>
#include <universal/q_shared.h>
#include <universal/com_shared.h>
#include <universal/com_memory.h>
#include <qcommon/common.h>
#include <game_mp/g_main_mp.h>
#include <universal/com_stringtable.h>
#include <server_mp/sv_main_pc_mp.h>
#include <demo/demo_playback.h>
#include <bgame/bg_perks.h>
#include <bgame/bg_misc.h>
#include <ddl/ddl_api.h>
#include <live/live_win.h>
#include <live/live_fileshare.h>
#include <live/live_counter.h>
#include <qcommon/com_clients.h>
#include <ctime>
#include <client_mp/cl_main_mp.h>

const int CustomKillstreakNums_0[22] =
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

const char *GAMEMODE_SPECIFIC_DVARS[6] =
{
  "timelimit",
  "scorelimit",
  "waverespawndelay",
  "numlives",
  "playerrespawndelay",
  "roundlimit"
};

const char *GAMEMODE_DVARS[94] =
{
  "scr_game_spectatetype",
  "scr_team_fftype",
  "scr_team_teamkillspawndelay",
  "scr_player_maxhealth",
  "scr_player_healthregentime",
  "scr_player_sprintTime",
  "scr_game_allowkillcam",
  "scr_game_forceradar",
  "scr_player_forcerespawn",
  "scr_game_onlyheadshots",
  "scr_game_perks",
  "scr_vehicles_enabled",
  "scr_hardcore",
  "scr_game_hardpoints",
  "scr_num_bots",
  "scr_num_bots_enemy",
  "scr_num_bots_friendly",
  "scr_bot_difficulty",
  "scr_rcbomb_notimeout",
  "scr_allowbattlechatter",
  "scr_allowannouncer",
  "custom_perks_allow_pro",
  "perk_weapreloadmultiplier",
  "player_laststandbleedouttime",
  "player_laststandbleedouttimenorevive",
  "revive_time_taken",
  "perk_weapspreadmultiplier",
  "perk_killstreakreduction",
  "perk_delayexplosivetime",
  "perk_disarmexplosivetime",
  "perk_extrabreath",
  "perk_sprintmultiplier",
  "perk_weapratemultiplier",
  "perk_bulletpenetrationmultiplier",
  "perk_weapadsmultiplier",
  "perk_weapswitchmultiplier",
  "perk_mantlereduction",
  "perk_weapmeleemultiplier",
  "perk_healthregenmultiplier",
  "perk_damagekickreduction",
  "perk_shellshockreduction",
  "perk_fallheightunits",
  "perk_interactspeedmultiplier",
  "perk_flakJacket",
  "perk_fireproof",
  "perk_armorpiercing",
  "perk_sprintrecoverymultiplier",
  "perk_speedmultiplier",
  "scr_dm_score_kill",
  "scr_dm_score_assist",
  "scr_dm_score_death",
  "scr_dm_score_suicide",
  "scr_dm_bonus_leader",
  "scr_dm_score_headshot",
  "scr_tdm_score_kill",
  "scr_custom_score_assist",
  "scr_tdm_score_death",
  "scr_tdm_score_suicide",
  "scr_tdm_bonus_leader",
  "scr_tdm_score_headshot",
  "scr_ctf_idleflagreturntime",
  "scr_ctf_enemycarriervisible",
  "scr_ctf_touchreturn",
  "scr_ctf_roundswitch",
  "koth_autodestroytime",
  "koth_spawntime",
  "koth_kothmode",
  "scr_sab_bombtimer",
  "scr_sab_planttime",
  "scr_sab_defusetime",
  "scr_sab_hotpotato",
  "scr_sab_roundswitch",
  "scr_sd_bombtimer",
  "scr_sd_planttime",
  "scr_sd_defusetime",
  "scr_sd_multibomb",
  "scr_sd_roundswitch",
  "scr_dem_bombtimer",
  "scr_dem_planttime",
  "scr_dem_defusetime",
  "scr_dem_extratime",
  "scr_dem_roundswitch",
  "scr_dom_flagcapturetime",
  "custom_class_mode",
  "custom_killstreak_mode",
  "custom_killstreak_1",
  "custom_killstreak_2",
  "custom_killstreak_3",
  "custom_killstreak_1_kills",
  "custom_killstreak_2_kills",
  "custom_killstreak_3_kills",
  "g_pregame_enabled",
  "party_minplayers",
  "demo_recordPrivateMatch"
};


const dvar_t *custom_class_name;
const dvar_t *custom_class_create_enabled;
const dvar_t *custom_class_team;
const dvar_t *custom_class_health;
const dvar_t *custom_class_health_regen;
const dvar_t *custom_class_health_vampirism;
const dvar_t *custom_class_speed;
const dvar_t *custom_class_sprint_speed;
const dvar_t *custom_class_damage;
const dvar_t *custom_class_explosive_damage;
const dvar_t *ui_items_no_cost;
const dvar_t *custom_killstreak_mode;
const dvar_t *custom_killstreak_numkills;
const dvar_t *ui_useCustomClassInfo;

void *s_fileShareBuffer;
int s_fileShareControllerIndex;

_CustomClassDescription g_customGameModeClassDescriptions[10];
CustomGameModeData g_customGameModeData;
bool s_isCustomGameModeDirty;

void __cdecl EnableMinimumCustomClasses()
{
    int c; // [esp+0h] [ebp-4h]

    for (c = 0; c < 10; ++c)
    {
        if (g_customGameModeClasses[c].isActive > 0)
            return;
    }
    g_customGameModeClasses[0].isActive = 1;
    g_customGameModeClasses[0].team = 1;
}

void __cdecl UI_Gametype_ShowErrorPopup(char *title, char *message)
{
    Dvar_SetStringByName("com_errorTitle", title);
    Dvar_SetStringByName("com_errorMessage", message);
    UI_OpenMenu(0, "popup_custom_game_error");
}

void __cdecl UI_Gametype_WriteDvarChunk(MemoryFile *memFile, char *dvarName)
{
    unsigned __int8 v2; // [esp+26h] [ebp-Ah]
    unsigned __int8 type; // [esp+27h] [ebp-9h]
    const dvar_s *dvar; // [esp+2Ch] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if ( dvar )
    {
        if ( (unsigned int)(memFile->cacheBufferUsed + 1) >= 0x7FF8 )
            MemFile_WriteDataFlushInternal(memFile);
        memFile->cacheBuffer[memFile->cacheBufferUsed++] = 0;
        MemFile_WriteCString(memFile, dvarName);
        type = dvar->type;
        if ( (unsigned int)(memFile->cacheBufferUsed + 1) >= 0x7FF8 )
            MemFile_WriteDataFlushInternal(memFile);
        memFile->cacheBuffer[memFile->cacheBufferUsed++] = type;
        switch ( dvar->type )
        {
            case DVAR_TYPE_BOOL:
                v2 = dvar->current.color[0];
                if ( (unsigned int)(memFile->cacheBufferUsed + 1) >= 0x7FF8 )
                    MemFile_WriteDataFlushInternal(memFile);
                memFile->cacheBuffer[memFile->cacheBufferUsed++] = v2;
                break;
            case DVAR_TYPE_FLOAT:
                MemFile_WriteFloat(memFile, dvar->current.value);
                break;
            case DVAR_TYPE_FLOAT_2:
                MemFile_WriteFloat(memFile, dvar->current.value);
                MemFile_WriteFloat(memFile, dvar->current.vector[1]);
                break;
            case DVAR_TYPE_FLOAT_3:
                MemFile_WriteFloat(memFile, dvar->current.value);
                MemFile_WriteFloat(memFile, dvar->current.vector[1]);
                MemFile_WriteFloat(memFile, dvar->current.vector[2]);
                break;
            case DVAR_TYPE_FLOAT_4:
                MemFile_WriteFloat(memFile, dvar->current.value);
                MemFile_WriteFloat(memFile, dvar->current.vector[1]);
                MemFile_WriteFloat(memFile, dvar->current.vector[2]);
                MemFile_WriteFloat(memFile, dvar->current.vector[3]);
                break;
            case DVAR_TYPE_INT:
            case DVAR_TYPE_ENUM:
                MemFile_WriteInt(memFile, dvar->current.integer);
                break;
            case DVAR_TYPE_STRING:
                MemFile_WriteCString(memFile, (char *)dvar->current.integer);
                break;
            case DVAR_TYPE_COLOR:
                MemFile_WriteByte(memFile, dvar->current.color[0]);
                MemFile_WriteByte(memFile, dvar->current.color[1]);
                MemFile_WriteByte(memFile, dvar->current.color[2]);
                MemFile_WriteByte(memFile, dvar->current.color[3]);
                break;
            default:
                return;
        }
    }
}

char __cdecl UI_Gametype_ReadDvarChunk(MemoryFile *memFile)
{
    char *CString; // eax
    unsigned __int8 v3; // [esp+14h] [ebp-47Ch] BYREF
    unsigned __int8 v4; // [esp+15h] [ebp-47Bh] BYREF
    unsigned __int8 v5; // [esp+16h] [ebp-47Ah] BYREF
    unsigned __int8 v6; // [esp+17h] [ebp-479h] BYREF
    int v7; // [esp+18h] [ebp-478h] BYREF
    unsigned int v8; // [esp+1Ch] [ebp-474h] BYREF
    unsigned int v9; // [esp+20h] [ebp-470h] BYREF
    unsigned int v10; // [esp+24h] [ebp-46Ch] BYREF
    unsigned int v11[4]; // [esp+28h] [ebp-468h] BYREF
    unsigned int v12; // [esp+38h] [ebp-458h] BYREF
    unsigned int v13; // [esp+3Ch] [ebp-454h] BYREF
    float v14; // [esp+40h] [ebp-450h] BYREF
    int value; // [esp+44h] [ebp-44Ch] BYREF
    unsigned __int8 v16; // [esp+4Ah] [ebp-446h] BYREF
    unsigned __int8 p; // [esp+4Bh] [ebp-445h] BYREF
    float r; // [esp+4Ch] [ebp-444h]
    float g; // [esp+50h] [ebp-440h]
    float b; // [esp+54h] [ebp-43Ch]
    float byteToFloatConversion; // [esp+58h] [ebp-438h]
    float a; // [esp+5Ch] [ebp-434h]
    unsigned int z; // [esp+60h] [ebp-430h]
    unsigned int v24; // [esp+64h] [ebp-42Ch]
    unsigned int v25; // [esp+68h] [ebp-428h]
    unsigned int w; // [esp+6Ch] [ebp-424h]
    float vec[3]; // [esp+70h] [ebp-420h] BYREF
    unsigned int x; // [esp+7Ch] [ebp-414h]
    unsigned int y; // [esp+80h] [ebp-410h]
    const char *dvarNameFromCache; // [esp+84h] [ebp-40Ch]
    char dvarName[1024]; // [esp+88h] [ebp-408h] BYREF
    unsigned __int8 type; // [esp+48Fh] [ebp-1h]

    dvarNameFromCache = MemFile_ReadCString(memFile);
    if ( !dvarNameFromCache || !*dvarNameFromCache )
        return 0;
    I_strncpyz(dvarName, dvarNameFromCache, 1024);
    MemFile_ReadData(memFile, 1, &p);
    type = p;
    switch ( p )
    {
        case 0u:
            MemFile_ReadData(memFile, 1, &v16);
            Dvar_SetBoolByName(dvarName, v16 != 0);
            break;
        case 1u:
            MemFile_ReadData(memFile, 4, (unsigned __int8 *)&v14);
            Dvar_SetFloatByName(dvarName, v14);
            break;
        case 2u:
            MemFile_ReadData(memFile, 4, (unsigned __int8 *)&v13);
            x = v13;
            MemFile_ReadData(memFile, 4, (unsigned __int8 *)&v12);
            y = v12;
            Dvar_SetVec2ByName(dvarName, x, v12);
            break;
        case 3u:
            MemFile_ReadVec3(memFile, vec);
            Dvar_SetVec3ByName(dvarName, vec[0], vec[1], vec[2]);
            break;
        case 4u:
            MemFile_ReadData(memFile, 4, (unsigned __int8 *)v11);
            v24 = v11[0];
            MemFile_ReadData(memFile, 4, (unsigned __int8 *)&v10);
            v25 = v10;
            MemFile_ReadData(memFile, 4, (unsigned __int8 *)&v9);
            z = v9;
            MemFile_ReadData(memFile, 4, (unsigned __int8 *)&v8);
            w = v8;
            Dvar_SetVec4ByName(dvarName, v24, v25, z, v8);
            break;
        case 5u:
            MemFile_ReadData(memFile, 4, (unsigned __int8 *)&value);
            Dvar_SetIntByName(dvarName, value);
            break;
        case 6u:
            MemFile_ReadData(memFile, 4, (unsigned __int8 *)&v7);
            Dvar_SetIntByName(dvarName, v7);
            break;
        case 7u:
            CString = MemFile_ReadCString(memFile);
            Dvar_SetStringByName(dvarName, CString);
            break;
        case 8u:
            byteToFloatConversion = 0.0039215689f;
            MemFile_ReadData(memFile, 1, &v6);
            r = (float)v6 * byteToFloatConversion;
            MemFile_ReadData(memFile, 1, &v5);
            g = (float)v5 * byteToFloatConversion;
            MemFile_ReadData(memFile, 1, &v4);
            b = (float)v4 * byteToFloatConversion;
            MemFile_ReadData(memFile, 1, &v3);
            a = (float)v3 * byteToFloatConversion;
            Dvar_SetColorByName(dvarName, r, g, b, a);
            break;
        default:
            return 1;
    }
    return 1;
}

void __cdecl MemFile_ReadVec3(MemoryFile *memFile, float *value)
{
    int v2; // [esp+0h] [ebp-Ch] BYREF
    int v3; // [esp+4h] [ebp-8h] BYREF
    int p; // [esp+8h] [ebp-4h] BYREF

    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&p);
    *(unsigned int *)value = p;
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&v3);
    *((unsigned int *)value + 1) = v3;
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&v2);
    *((unsigned int *)value + 2) = v2;
}

const dvar_s *__cdecl RegisterDvarIfUndefined(const char *name, int defaultValue)
{
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = Dvar_FindVar(name);
    if ( !dvar )
        return _Dvar_RegisterInt(name, defaultValue, 0, 1000, 0, "custom class dvar");
    return dvar;
}

void __cdecl RegisterCustomClassDvarsIfNeeded()
{
    custom_class_create_enabled = RegisterDvarIfUndefined("custom_class_create_enabled", 1);
    custom_class_team = RegisterDvarIfUndefined("custom_class_team", 1);
    custom_class_health = RegisterDvarIfUndefined("custom_class_health", 100);
    custom_class_health_regen = RegisterDvarIfUndefined("custom_class_health_regen", 4);
    custom_class_health_vampirism = RegisterDvarIfUndefined("custom_class_health_vampirism", 0);
    custom_class_speed = RegisterDvarIfUndefined("custom_class_speed", 3);
    custom_class_sprint_speed = RegisterDvarIfUndefined("custom_class_sprint_speed", 1);
    custom_class_damage = RegisterDvarIfUndefined("custom_class_damage", 2);
    custom_class_explosive_damage = RegisterDvarIfUndefined("custom_class_explosive_damage", 2);
}

void __cdecl GetClassLoadoutForClassName(const char *className, unsigned int customClassSlotNum)
{
    _CustomClassData *customClass; // [esp+0h] [ebp-4h]

    if ( customClassSlotNum >= 0xA
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_gametype_custom_mp.cpp",
                    451,
                    0,
                    "%s",
                    "customClassSlotNum >= 0 && customClassSlotNum < NUM_CUSTOM_CLASSES") )
    {
        __debugbreak();
    }
    customClass = &g_customGameModeClasses[customClassSlotNum];
    customClass->primary = BG_UnlockablesGetDefaultClassSlotIndexFromName(className, "primary");
    customClass->secondary = BG_UnlockablesGetDefaultClassSlotIndexFromName(className, "secondary");
    customClass->primarygrenade = BG_UnlockablesGetDefaultClassSlotIndexFromName(className, "primarygrenade");
    customClass->specialty1 = BG_UnlockablesGetDefaultClassSlotIndexFromName(className, "specialty1");
    customClass->specialty2 = BG_UnlockablesGetDefaultClassSlotIndexFromName(className, "specialty2");
    customClass->specialty3 = BG_UnlockablesGetDefaultClassSlotIndexFromName(className, "specialty3");
    customClass->equipment = BG_UnlockablesGetDefaultClassSlotIndexFromName(className, "equipment");
    customClass->specialgrenade = BG_UnlockablesGetDefaultClassSlotIndexFromName(className, "specialgrenade");
    customClass->body = BG_UnlockablesGetDefaultClassSlotIndexFromName(className, "body");
}

void __cdecl UI_Gametype_BuildDefaultCustomClasses()
{
    char *v0; // eax
    int defaultItemNum; // [esp+4h] [ebp-10h]
    unsigned int d; // [esp+8h] [ebp-Ch]
    signed int c; // [esp+10h] [ebp-4h]

    RegisterCustomClassDvarsIfNeeded();
    for ( c = 0; c < 10; ++c )
    {
        for ( d = 0; d < 0x27; ++d )
        {
            defaultItemNum = BG_UnlockablesGetActualDefaultItem(d);
            if (defaultItemNum == -1)
                defaultItemNum = 0;
            *(&g_customGameModeClasses[c].primary + d) = defaultItemNum;
        }
        //_CustomClassData::SaveClassDataFromDvars(&g_customGameModeClasses[c]);
        g_customGameModeClasses[c].SaveClassDataFromDvars();
        v0 = UI_SafeTranslateString("MPUI_CUSTOM_CLASS_NAME");
        Com_sprintf(g_customGameModeClassDescriptions[c].name, 0x10u, "%s %d", v0, c + 1);
        switch ( c % 5 )
        {
            case 0:
                GetClassLoadoutForClassName("CLASS_SMG", c);
                break;
            case 1:
                GetClassLoadoutForClassName("CLASS_CQB", c);
                break;
            case 2:
                GetClassLoadoutForClassName("CLASS_ASSAULT", c);
                break;
            case 3:
                GetClassLoadoutForClassName("CLASS_LMG", c);
                break;
            default:
                GetClassLoadoutForClassName("CLASS_SNIPER", c);
                break;
        }
    }
}

void __cdecl UI_Gametype_CustomClassesReset()
{
    int c; // [esp+0h] [ebp-4h]

    for ( c = 0; c < 10; ++c )
        g_customGameModeClasses[c].isActive = 0;
    UI_Gametype_BuildDefaultCustomClasses();
}

char __cdecl UI_Gametype_ReadCustomClassChunk(MemoryFile *memFile)
{
    int v2; // [esp+0h] [ebp-10h] BYREF
    int p; // [esp+4h] [ebp-Ch] BYREF
    int version; // [esp+8h] [ebp-8h]
    int size; // [esp+Ch] [ebp-4h]

    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&p);
    version = p;
    UI_Gametype_BuildDefaultCustomClasses();
    EnableMinimumCustomClasses();
    if ( version < 2 )
        return 0;
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&v2);
    size = v2;
    if ( version == 2 && size == 800 )
    {
        MemFile_ReadData(memFile, 800, (unsigned __int8 *)g_customGameModeClasses);
        return 1;
    }
    else
    {
        memFile->bytesUsed += size;
        if ( memFile->bytesUsed > memFile->bufferSize )
            memFile->bytesUsed = memFile->bufferSize;
        return 1;
    }
}

char __cdecl UI_Gametype_ReadCustomClassDescriptionChunk(MemoryFile *memFile)
{
    MemFile_ReadData(memFile, 160, (unsigned __int8 *)g_customGameModeClassDescriptions);
    return 1;
}

bool __cdecl UI_Gametype_HasDvarChanged(const dvar_s *dvar)
{
    bool result; // al
    bool v2; // [esp+0h] [ebp-28h]
    bool v3; // [esp+4h] [ebp-24h]
    bool v4; // [esp+8h] [ebp-20h]
    bool v5; // [esp+10h] [ebp-18h]

    switch ( dvar->type )
    {
        case DVAR_TYPE_BOOL:
            result = dvar->current.color[0] != dvar->reset.color[0];
            break;
        case DVAR_TYPE_FLOAT:
            result = dvar->current.value != dvar->reset.value;
            break;
        case DVAR_TYPE_FLOAT_2:
            v4 = dvar->current.value != dvar->reset.value || dvar->current.vector[1] != dvar->reset.vector[1];
            result = v4;
            break;
        case DVAR_TYPE_FLOAT_3:
            v3 = dvar->current.value != dvar->reset.value
                || dvar->current.vector[1] != dvar->reset.vector[1]
                || dvar->current.vector[2] != dvar->reset.vector[2];
            result = v3;
            break;
        case DVAR_TYPE_FLOAT_4:
            v2 = dvar->current.value == dvar->reset.value
                && dvar->current.vector[1] == dvar->reset.vector[1]
                && dvar->current.vector[2] == dvar->reset.vector[2]
                && dvar->current.vector[3] == dvar->reset.vector[3];
            result = v2;
            break;
        case DVAR_TYPE_INT:
        case DVAR_TYPE_ENUM:
        case DVAR_TYPE_COLOR:
            result = dvar->current.integer != dvar->reset.integer;
            break;
        case DVAR_TYPE_STRING:
            result = I_strcmp(dvar->current.string, dvar->reset.string) != 0;
            break;
        case DVAR_TYPE_INT64:
            v5 = dvar->current.integer != dvar->reset.integer || (dvar->current.vector[1]) != (dvar->reset.vector[1]);
            result = v5;
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

bool __cdecl UI_Gametype_HasDvarChanged(const char *dvarName)
{
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    return dvar && UI_Gametype_HasDvarChanged(dvar);
}

void __cdecl UI_Gametype_WriteDvarIfChanged(MemoryFile *memFile, char *dvarName)
{
    if ( UI_Gametype_HasDvarChanged(dvarName) )
        UI_Gametype_WriteDvarChunk(memFile, dvarName);
}

void __cdecl UI_Gametype_WriteDvars(MemoryFile *memFile)
{
    char *v1; // eax
    int j; // [esp+0h] [ebp-10h]
    int i; // [esp+4h] [ebp-Ch]
    const char *gametype; // [esp+8h] [ebp-8h]

    gametype = Dvar_GetString("ui_gametype");
    for ( i = 0; i < 6; ++i )
    {
        v1 = va("scr_%s_%s", gametype, GAMEMODE_SPECIFIC_DVARS[i]);
        UI_Gametype_WriteDvarIfChanged(memFile, v1);
    }
    for ( j = 0; j < 94; ++j )
        UI_Gametype_WriteDvarIfChanged(memFile, (char *)GAMEMODE_DVARS[j]);
}

char __cdecl HasClassDescriptionDataChanged()
{
    int c; // [esp+0h] [ebp-4h]

    for ( c = 0; c < 10; ++c )
    {
        if ( g_customGameModeClassDescriptions[c].name[0] )
            return 1;
    }
    return 0;
}

void __cdecl UI_Gametype_WriteCustomClasses(MemoryFile *memFile)
{
    if ( (unsigned int)(memFile->cacheBufferUsed + 1) >= 0x7FF8 )
        MemFile_WriteDataFlushInternal(memFile);
    memFile->cacheBuffer[memFile->cacheBufferUsed++] = 1;
    MemFile_WriteInt(memFile, 2);
    MemFile_WriteInt(memFile, 800);
    MemFile_WriteData(memFile, 0x320u, (unsigned __int8 *)g_customGameModeClasses);
    if ( HasClassDescriptionDataChanged() )
    {
        if ( (unsigned int)(memFile->cacheBufferUsed + 1) >= 0x7FF8 )
            MemFile_WriteDataFlushInternal(memFile);
        memFile->cacheBuffer[memFile->cacheBufferUsed++] = 3;
        MemFile_WriteData(memFile, 0xA0u, (unsigned __int8 *)g_customGameModeClassDescriptions);
    }
}

void __cdecl UI_Gametype_WriteHeader(MemoryFile *memFile, UIGametypeFileHeader *header)
{
    const char *String; // eax
    const char *v3; // eax
    const char *v4; // eax

    header->version = 4;
    String = Dvar_GetString("ui_customModeEditName");
    Com_sprintf(header->name, 0x20u, "%s", String);
    v3 = Dvar_GetString("ui_customModeEditDesc");
    Com_sprintf(header->description, 0x80u, "%s", v3);
    v4 = Dvar_GetString("ui_gametype");
    Com_sprintf(header->baseGameType, 0x20u, "%s", v4);
    header->lastModifiedDateTime = Com_RealTime(0, 1);
    MemFile_WriteInt(memFile, header->version);
    MemFile_WriteCString(memFile, header->baseGameType);
    MemFile_WriteCString(memFile, header->description);
    MemFile_WriteInt(memFile, header->lastModifiedDateTime);
    MemFile_WriteCString(memFile, header->name);
}

void __cdecl UI_Gametype_ReadHeader(MemoryFile *memFile, UIGametypeFileHeader *header)
{
    char *CString; // eax
    char *v3; // eax
    char *v4; // eax
    int v5; // [esp+0h] [ebp-8h] BYREF
    int p; // [esp+4h] [ebp-4h] BYREF

    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&p);
    header->version = p;
    CString = MemFile_ReadCString(memFile);
    I_strncpyz(header->baseGameType, CString, 32);
    v3 = MemFile_ReadCString(memFile);
    I_strncpyz(header->description, v3, 128);
    header->lastModifiedDateTime = 0;
    if ( header->version >= 2 )
    {
        MemFile_ReadData(memFile, 4, (unsigned __int8 *)&v5);
        header->lastModifiedDateTime = v5;
        if ( header->version >= 3 )
        {
            v4 = MemFile_ReadCString(memFile);
            I_strncpyz(header->name, v4, 32);
        }
    }
}

void __cdecl UI_Gametype_WriteToMemFile(MemoryFile *memFile, UIGametypeFileHeader *header)
{
    unsigned __int8 *v2; // eax

    v2 = (unsigned __int8 *)Z_VirtualAlloc(0x2000, "custom_gametype", 0);
    MemFile_InitForWriting(memFile, 0x2000, v2, 0, 0);
    UI_Gametype_WriteHeader(memFile, header);
    UI_Gametype_WriteDvars(memFile);
    UI_Gametype_WriteCustomClasses(memFile);
    UI_GameVariants_WriteToMemoryFile(memFile);
}

void __cdecl UI_Gametype_CopyDvarsToCustom()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    char *String; // [esp-4h] [ebp-14h]
    char *VariantString; // [esp-4h] [ebp-14h]
    int j; // [esp+0h] [ebp-10h]
    int i; // [esp+4h] [ebp-Ch]
    const char *gametype; // [esp+8h] [ebp-8h]

    gametype = Dvar_GetString("ui_gametype");
    for ( i = 0; i < 6; ++i )
    {
        v0 = va("scr_%s_%s", gametype, GAMEMODE_SPECIFIC_DVARS[i]);
        String = (char *)Dvar_GetString(v0);
        v1 = va("custom_%s", GAMEMODE_SPECIFIC_DVARS[i]);
        Dvar_SetStringByName(v1, String);
    }
    for ( j = 0; j < 94; ++j )
    {
        VariantString = (char *)Dvar_GetVariantString(GAMEMODE_DVARS[j]);
        v2 = va("custom_%s", GAMEMODE_DVARS[j]);
        Dvar_SetStringByName(v2, VariantString);
    }
}

void __cdecl UI_Gametype_CopyCustomToDvars()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    char *String; // [esp-4h] [ebp-14h]
    char *VariantString; // [esp-4h] [ebp-14h]
    int j; // [esp+0h] [ebp-10h]
    int i; // [esp+4h] [ebp-Ch]
    const char *gametype; // [esp+8h] [ebp-8h]

    gametype = Dvar_GetString("ui_gametype");
    for ( i = 0; i < 6; ++i )
    {
        v0 = va("custom_%s", GAMEMODE_SPECIFIC_DVARS[i]);
        String = (char *)Dvar_GetString(v0);
        v1 = va("scr_%s_%s", gametype, GAMEMODE_SPECIFIC_DVARS[i]);
        Dvar_SetStringByName(v1, String);
    }
    for ( j = 0; j < 94; ++j )
    {
        VariantString = (char *)Dvar_GetVariantString(GAMEMODE_DVARS[j]);
        v2 = va("custom_%s", GAMEMODE_DVARS[j]);
        Dvar_SetStringByName(v2, VariantString);
    }
}

char __cdecl UI_Gametype_HasAnyDvarsChanged(const char **dvarNameList, int dvarNameListSize, bool areGameSpecificDvars)
{
    const char *v3; // eax
    const char *customDvarValue; // [esp+0h] [ebp-14h]
    char *dvarName; // [esp+4h] [ebp-10h]
    const char *dvarValue; // [esp+8h] [ebp-Ch]
    int i; // [esp+Ch] [ebp-8h]
    const char *gametype; // [esp+10h] [ebp-4h]

    if ( !dvarNameList
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_gametype_custom_mp.cpp",
                    757,
                    0,
                    "%s",
                    "dvarNameList") )
    {
        __debugbreak();
    }
    gametype = Dvar_GetString("ui_gametype");
    for ( i = 0; i < dvarNameListSize; ++i )
    {
        v3 = va("custom_%s", dvarNameList[i]);
        customDvarValue = Dvar_GetVariantString(v3);
        dvarName = (char *)dvarNameList[i];
        if ( areGameSpecificDvars )
            dvarName = va("scr_%s_%s", gametype, dvarName);
        dvarValue = Dvar_GetVariantString(dvarName);
        if ( I_strcmp(customDvarValue, dvarValue) )
            return 1;
    }
    return 0;
}

void __cdecl UI_Gametype_CheckIfAnythingChanged_f()
{
    const char *v0; // eax
    char hasAnythingChanged; // [esp+7h] [ebp-1h]

    v0 = va("custom_%s", GAMEMODE_SPECIFIC_DVARS[0]);
    if ( Dvar_FindVar(v0) )
    {
        hasAnythingChanged = UI_Gametype_HasAnyDvarsChanged(GAMEMODE_SPECIFIC_DVARS, 6, 1);
        if ( !hasAnythingChanged )
            hasAnythingChanged = UI_Gametype_HasAnyDvarsChanged(GAMEMODE_DVARS, 94, 0);
        if ( !hasAnythingChanged )
            hasAnythingChanged = UI_Gametype_IsDirty();
        Dvar_SetBoolByName("ui_custom_haschanged", hasAnythingChanged);
    }
}

void __cdecl CheckForMinimumTeamClasses()
{
    _CustomClassData *customClass; // [esp+0h] [ebp-10h]
    int c; // [esp+4h] [ebp-Ch]
    int numAllies; // [esp+8h] [ebp-8h]
    int numAxis; // [esp+Ch] [ebp-4h]

    numAllies = 0;
    numAxis = 0;
    for ( c = 0; c < 10; ++c )
    {
        customClass = &g_customGameModeClasses[c];
        if ( customClass->isActive > 0 )
        {
            if ( customClass->team == 2 || customClass->team == 1 )
                ++numAllies;
            if ( customClass->team == 3 || customClass->team == 1 )
                ++numAxis;
        }
    }
    if ( (!numAllies || numAxis) && (numAllies || !numAxis) && (numAllies || numAxis) )
        Dvar_SetBoolByName("ui_custom_class_need_team_balance", 0);
    else
        Dvar_SetBoolByName("ui_custom_class_need_team_balance", 1);
}

void __cdecl UI_Gametype_UpdateCustomClassDvarsForClass_f()
{
    const char *v0; // eax
    int classNum; // [esp+10h] [ebp-4h]

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        classNum = atoi(v0) - 1;
        //_CustomClassData::PopulateCustomDvarsFromClassData(&g_customGameModeClasses[classNum]);
        g_customGameModeClasses[classNum].PopulateCustomDvarsFromClassData();
        custom_class_name = Dvar_FindVar("custom_class_name");
        if ( !custom_class_name )
            custom_class_name = _Dvar_RegisterString("custom_class_name", (char *)"", 0, "custom class name");
        Dvar_SetString((dvar_s *)custom_class_name, g_customGameModeClassDescriptions[classNum].name);
    }
    else
    {
        Com_Printf(0, "USAGE: updateClassDataFromDvars classnum \n");
    }
}

void __cdecl UI_Gametype_UpdateClassDataFromDvars_f()
{
    const char *v0; // eax
    int v1; // [esp+18h] [ebp-3DCh]
    _CustomClassDescription *j; // [esp+1Ch] [ebp-3D8h]
    int v3; // [esp+20h] [ebp-3D4h]
    _CustomClassData *i; // [esp+24h] [ebp-3D0h]
    int classNum; // [esp+28h] [ebp-3CCh]
    _CustomClassDescription customClassNameCopy[10]; // [esp+2Ch] [ebp-3C8h] BYREF
    _CustomClassData customClassCopy[10]; // [esp+D4h] [ebp-320h] BYREF

    v3 = 10;
    for ( i = customClassCopy; --v3 >= 0; ++i )
        i->isActive = -1;
    v1 = 10;
    for ( j = customClassNameCopy; --v1 >= 0; ++j )
    {
        *(unsigned int *)j->name = 0;
        *(unsigned int *)&j->name[4] = 0;
        *(unsigned int *)&j->name[8] = 0;
        *(unsigned int *)&j->name[12] = 0;
    }
    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        classNum = atoi(v0) - 1;
        memcpy(customClassCopy, g_customGameModeClasses, sizeof(customClassCopy));
        memcpy(customClassNameCopy, g_customGameModeClassDescriptions, sizeof(customClassNameCopy));
        //_CustomClassData::SaveClassDataFromDvars(&g_customGameModeClasses[classNum]);
        g_customGameModeClasses[classNum].SaveClassDataFromDvars();
        EnableMinimumCustomClasses();
        I_strncpyz(g_customGameModeClassDescriptions[classNum].name, custom_class_name->current.string, 16);
        if ( memcmp(customClassCopy, g_customGameModeClasses, 0x320u)
            || memcmp(customClassNameCopy, g_customGameModeClassDescriptions, 0xA0u) )
        {
            UI_Gametype_MarkDirty();
        }
        CheckForMinimumTeamClasses();
    }
    else
    {
        Com_Printf(0, "USAGE: updateClassDataFromDvars classnum \n");
    }
}

void __cdecl UI_Gametype_CreateDefaultCustomClassIfNeeded_f()
{
    int c; // [esp+0h] [ebp-8h]
    bool isDefaultClassNeeded; // [esp+7h] [ebp-1h]

    isDefaultClassNeeded = 1;
    for ( c = 0; c < 10; ++c )
    {
        if ( g_customGameModeClasses[c].isActive > 0 )
        {
            isDefaultClassNeeded = 0;
            break;
        }
    }
    if ( isDefaultClassNeeded )
    {
        g_customGameModeClasses[0].isActive = 1;
        g_customGameModeClasses[0].team = 1;
    }
}

void __cdecl UI_Gametype_ApplyCustomClassTeamSelections_f()
{
    const char *v0; // eax
    int team; // [esp+0h] [ebp-Ch]
    _CustomClassData *customClass; // [esp+4h] [ebp-8h]
    int c; // [esp+8h] [ebp-4h]

    for ( c = 0; c < 10; ++c )
    {
        customClass = &g_customGameModeClasses[c];
        v0 = va("custom_class_team_%d", c + 1);
        team = Dvar_GetInt(v0);
        customClass->isActive = team != 0;
        if ( team != customClass->team )
            UI_Gametype_MarkDirty();
        customClass->team = team;
    }
    CheckForMinimumTeamClasses();
}

void __cdecl UI_Gametype_UpdateCustomClassTeamDvars_f()
{
    const char *v0; // eax
    int value; // [esp+0h] [ebp-Ch]
    int c; // [esp+8h] [ebp-4h]

    for ( c = 0; c < 10; ++c )
    {
        if ( g_customGameModeClasses[c].isActive <= 0 )
            value = 0;
        else
            value = g_customGameModeClasses[c].team;
        v0 = va("custom_class_team_%d", c + 1);
        Dvar_SetIntByName(v0, value);
    }
    CheckForMinimumTeamClasses();
}

char __cdecl UI_Gametype_ReadChunk(MemoryFile *memFile)
{
    char result; // al
    unsigned __int8 p; // [esp+7h] [ebp-5h] BYREF
    int type; // [esp+8h] [ebp-4h]

    MemFile_ReadData(memFile, 1, &p);
    type = p;
    switch ( p )
    {
        case 0u:
            result = UI_Gametype_ReadDvarChunk(memFile);
            break;
        case 1u:
            result = UI_Gametype_ReadCustomClassChunk(memFile);
            break;
        case 2u:
            result = UI_GameVariants_ReadGameVariantRuleChunk(memFile);
            break;
        case 3u:
            result = UI_Gametype_ReadCustomClassDescriptionChunk(memFile);
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

char __cdecl UI_Gametype_ReadChunks(MemoryFile *memFile)
{
    while ( memFile->bytesUsed < memFile->bufferSize )
    {
        if ( !UI_Gametype_ReadChunk(memFile) )
        {
            Com_PrintError(14, "Error reading in game mode.\n");
            UI_Gametype_ShowErrorPopup((char*)"@MENU_ERROR", (char*)"@CUSTOM_LOAD_ERROR");
            return 0;
        }
    }
    return 1;
}

char __cdecl UI_Gametype_ReadFromMemFile(MemoryFile *memFile)
{
    UIGametypeFileHeader header; // [esp+0h] [ebp-D0h] BYREF

    UI_Gametype_ReadHeader(memFile, &header);
    Dvar_SetString((dvar_s *)g_gametype, header.baseGameType);
    return UI_Gametype_ReadChunks(memFile);
}

void __cdecl ResetCustomClassDvars()
{
    Dvar_Reset((dvar_s *)custom_class_create_enabled, DVAR_SOURCE_INTERNAL);
    Dvar_Reset((dvar_s *)custom_class_team, DVAR_SOURCE_INTERNAL);
    Dvar_Reset((dvar_s *)custom_class_health, DVAR_SOURCE_INTERNAL);
    Dvar_Reset((dvar_s *)custom_class_health_regen, DVAR_SOURCE_INTERNAL);
    Dvar_Reset((dvar_s *)custom_class_health_vampirism, DVAR_SOURCE_INTERNAL);
    Dvar_Reset((dvar_s *)custom_class_speed, DVAR_SOURCE_INTERNAL);
    Dvar_Reset((dvar_s *)custom_class_sprint_speed, DVAR_SOURCE_INTERNAL);
    Dvar_Reset((dvar_s *)custom_class_damage, DVAR_SOURCE_INTERNAL);
    Dvar_Reset((dvar_s *)custom_class_explosive_damage, DVAR_SOURCE_INTERNAL);
}

void __cdecl UI_Gametype_ResetDvars()
{
    const char *v0; // eax
    dvar_s *Var; // [esp+0h] [ebp-18h]
    int j; // [esp+4h] [ebp-14h]
    dvar_s *dvar; // [esp+8h] [ebp-10h]
    int i; // [esp+Ch] [ebp-Ch]
    const char *gametype; // [esp+10h] [ebp-8h]

    gametype = Dvar_GetString("ui_gametype");
    for ( i = 0; i < 6; ++i )
    {
        v0 = va("scr_%s_%s", gametype, GAMEMODE_SPECIFIC_DVARS[i]);
        dvar = (dvar_s *)Dvar_FindVar(v0);
        if ( dvar )
            Dvar_Reset(dvar, DVAR_SOURCE_INTERNAL);
    }
    for ( j = 0; j < 94; ++j )
    {
        Var = (dvar_s *)Dvar_FindVar(GAMEMODE_DVARS[j]);
        if ( Var )
            Dvar_Reset(Var, DVAR_SOURCE_INTERNAL);
    }
    ResetCustomClassDvars();
}

void __cdecl UI_Gametype_EndChanges()
{
    EnableMinimumCustomClasses();
    UI_Gametype_CopyCustomToDvars();
    UI_Gametype_DvarsToCustomGameModeData(&g_customGameModeData);
    s_isCustomGameModeDirty = 0;
}

void __cdecl UI_Gametype_EndChanges_f()
{
    UI_Gametype_EndChanges();
}

void __cdecl UI_Gametype_SetupCustomModeNameAndDesc()
{
    char *v0; // eax
    char *v1; // eax
    char *v2; // eax
    const char *gameTypeName; // [esp+0h] [ebp-Ch]
    const StringTable *gameTypeTable; // [esp+4h] [ebp-8h] BYREF
    const char *gameType; // [esp+8h] [ebp-4h]

    Dvar_SetString((dvar_s *)ui_customModeName, "");
    StringTable_GetAsset("mp/gametypesTable.csv", (XAssetHeader *)&gameTypeTable);
    if ( !gameTypeTable
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_gametype_custom_mp.cpp",
                    1052,
                    0,
                    "%s",
                    "gameTypeTable") )
    {
        __debugbreak();
    }
    gameType = Dvar_GetString("ui_gametype");
    gameTypeName = StringTable_Lookup(gameTypeTable, 0, gameType, 1);
    v0 = UI_SafeTranslateString(gameTypeName);
    Dvar_SetString((dvar_s *)ui_customModeEditName, v0);
    v1 = UI_SafeTranslateString("CUSTOM_CUSTOM_GAME_MODE_CAPS");
    Dvar_SetString((dvar_s *)ui_customModeDesc, v1);
    v2 = UI_SafeTranslateString("CUSTOM_CUSTOM_GAME_MODE_CAPS");
    Dvar_SetString((dvar_s *)ui_customModeEditDesc, v2);
}

void __cdecl UI_Gametype_BeginChanges_f()
{
    UI_Gametype_CopyDvarsToCustom();
    UI_Gametype_SetupCustomModeNameAndDesc();
    Dvar_SetBoolByName("ui_custom_haschanged", 0);
}

bool __cdecl UI_Gametype_CanUseCustom()
{
    int LicenseType; // eax

    LicenseType = SV_GetLicenseType();
    return !SV_IsServerRanked(LicenseType);
}

bool __cdecl UI_Gametype_IsUsingCustom()
{
    int LicenseType; // eax

    LicenseType = SV_GetLicenseType();
    if ( SV_IsServerRanked(LicenseType) )
        return 0;
    if ( Demo_IsPlaying() )
        return Demo_IsCustomGameMode();
    return customGameMode && customGameMode->current.integer;
}

void __cdecl UI_Gametype_ResetGamemode()
{
    UI_Gametype_ResetDvars();
    UI_GameVariants_Reset();
    UI_Gametype_CustomClassesReset();
    UI_Gametype_DvarsToCustomGameModeData(&g_customGameModeData);
    Dvar_SetInt((dvar_s *)customGameMode, 0);
}

void __cdecl UI_Gametype_Custom_Reset_f()
{
    UI_Gametype_ResetGamemode();
}

void __cdecl UI_Gametype_RevertChanges_f()
{
    UI_Gametype_ResetGamemode();
    UI_Gametype_BeginChanges_f();
    s_isCustomGameModeDirty = 0;
}

bool __cdecl isValidClassSlotNum(unsigned int slotNum)
{
    const char *v1; // eax

    if ( slotNum >= 0xA )
    {
        v1 = va("Invalid index %d passsed in to custom class function", slotNum);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_gametype_custom_mp.cpp",
                        1135,
                        0,
                        "%s\n\t%s",
                        "slotNum < NUM_CUSTOM_CLASSES && slotNum >= 0",
                        v1) )
            __debugbreak();
    }
    return slotNum <= 9;
}

unsigned __int8 __cdecl GetCustomClassLoadoutItemForSlot(
                int controllerIndex,
                unsigned int slotNum,
                const char *itemName)
{
    if ( isValidClassSlotNum(slotNum) )
        return g_customGameModeClasses[slotNum].GetLoadoutItem(itemName);
    else
        return 0;
}

unsigned __int8 __cdecl GetCustomClassLoadoutItemForClassName(
                int controllerIndex,
                const char *className,
                const char *itemName)
{
    unsigned int slotNum; // [esp+0h] [ebp-4h]

    slotNum = atoi(className) - 1;
    if ( isValidClassSlotNum(slotNum) )
        return g_customGameModeClasses[slotNum].GetLoadoutItem(itemName);
    else
        return 0;
}

int __cdecl GetCustomClassModifierForClass(int controllerIndex, unsigned int classNum, const char *itemName)
{
    if ( classNum >= 0xA
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_gametype_custom_mp.cpp",
                    1167,
                    0,
                    "%s",
                    "classNum < NUM_CUSTOM_CLASSES && classNum >= 0") )
    {
        __debugbreak();
    }
    if ( isValidClassSlotNum(classNum) )
        return g_customGameModeClasses[classNum].GetClassModifierValue(itemName);
    else
        return 0;
}

void __cdecl UI_Gametype_EquipCustomClassLoadoutItem(
                int controllerIndex,
                const char *customClassName,
                unsigned __int8 itemIndex,
                const char *loadoutName)
{
    unsigned int slotNum; // [esp+0h] [ebp-4h]

    slotNum = atoi(customClassName) - 1;
    if ( isValidClassSlotNum(slotNum) )
        g_customGameModeClasses[slotNum].SetLoadoutItem(loadoutName, itemIndex);
}

int __thiscall _CustomClassData::GetClassModifierValue(const char *modifierName)
{
    unsigned int c; // [esp+4h] [ebp-8h]

    for (c = 0; c < 9; ++c)
    {
        if (!I_stricmp(_CustomClassData::modifierIndex[c].modifierName, modifierName))
            return *(int *)((char *)&this->isActive + _CustomClassData::modifierIndex[c].modifierOffset);
    }
    Com_PrintError(13, "Unknown custom class modifier %s\n", modifierName);
    return -1;
}

unsigned __int8 __thiscall _CustomClassData::GetLoadoutItem(const char *itemName)
{
    unsigned int i; // [esp+8h] [ebp-8h]
    unsigned __int8 *currentItemPtr; // [esp+Ch] [ebp-4h]

    currentItemPtr = &this->primary;
    for ( i = 0; i < 0x27; ++i )
    {
        if ( !I_stricmp(_CustomClassData::itemNames[i], itemName) )
            return currentItemPtr[i];
    }
    Com_PrintError(13, "Unknown custom class item name %s\n", itemName);
    return -1;
}

void __thiscall _CustomClassData::SetLoadoutItem(
                const char *itemName,
                unsigned __int8 itemIndex)
{
    unsigned int i; // [esp+Ch] [ebp-8h]
    unsigned __int8 *currentItemPtr; // [esp+10h] [ebp-4h]

    currentItemPtr = &this->primary;
    for ( i = 0; ; ++i )
    {
        if ( i >= 0x27 )
        {
            Com_PrintError(13, "Unknown custom class item name %s\n", itemName);
            return;
        }
        if ( !I_stricmp(_CustomClassData::itemNames[i], itemName) )
            break;
    }
    if ( currentItemPtr[i] != itemIndex )
    {
        currentItemPtr[i] = itemIndex;
        UI_Gametype_MarkDirty();
    }
}

void __thiscall _CustomClassData::PopulateCustomDvarsFromClassData()
{
    RegisterCustomClassDvarsIfNeeded();
    if ( this->isActive == -1 )
    {
        ResetCustomClassDvars();
    }
    else
    {
        Dvar_SetInt((dvar_s *)custom_class_create_enabled, this->isActive);
        Dvar_SetInt((dvar_s *)custom_class_team, this->team);
        Dvar_SetInt((dvar_s *)custom_class_health, this->health);
        Dvar_SetInt((dvar_s *)custom_class_health_regen, this->healthRegeneration);
        Dvar_SetInt((dvar_s *)custom_class_health_vampirism, this->healthVampirism);
        Dvar_SetInt((dvar_s *)custom_class_speed, this->movementSpeed);
        Dvar_SetInt((dvar_s *)custom_class_sprint_speed, this->movementSprintSpeed);
        Dvar_SetInt((dvar_s *)custom_class_damage, this->damage);
        Dvar_SetInt((dvar_s *)custom_class_explosive_damage, this->damageExplosive);
    }
}

void __thiscall _CustomClassData::SaveClassDataFromDvars()
{
    this->isActive = custom_class_create_enabled->current.integer;
    this->team = custom_class_team->current.color[0];
    this->health = custom_class_health->current.integer;
    this->healthRegeneration = custom_class_health_regen->current.color[0];
    this->healthVampirism = custom_class_health_vampirism->current.color[0];
    this->movementSpeed = custom_class_speed->current.color[0];
    this->movementSprintSpeed = custom_class_sprint_speed->current.color[0];
    this->damage = custom_class_damage->current.color[0];
    this->damageExplosive = custom_class_explosive_damage->current.color[0];
}

void __thiscall _CustomClassData::WriteClassToClientData(msg_t *msg)
{
    if ( this->isActive )
    {
        MSG_WriteBit1(msg);
        MSG_WriteByte(msg, this->primary);
        MSG_WriteByte(msg, this->primaryattachment);
        MSG_WriteByte(msg, this->primaryattachmenttop);
        MSG_WriteByte(msg, this->primaryattachmentbottom);
        MSG_WriteByte(msg, this->primaryattachmenttrigger);
        MSG_WriteByte(msg, this->primaryattachmentmuzzle);
        MSG_WriteByte(msg, this->primarycamo);
        MSG_WriteByte(msg, this->primaryreticle);
        MSG_WriteByte(msg, this->primaryreticlecolor);
        MSG_WriteByte(msg, this->primarylens);
        MSG_WriteByte(msg, this->primaryemblem);
        MSG_WriteByte(msg, this->primarytag);
        MSG_WriteByte(msg, this->secondary);
        MSG_WriteByte(msg, this->secondaryattachment);
        MSG_WriteByte(msg, this->secondaryattachmenttop);
        MSG_WriteByte(msg, this->secondaryattachmentbottom);
        MSG_WriteByte(msg, this->secondaryattachmenttrigger);
        MSG_WriteByte(msg, this->secondaryattachmentmuzzle);
        MSG_WriteByte(msg, this->secondarycamo);
        MSG_WriteByte(msg, this->secondaryreticle);
        MSG_WriteByte(msg, this->secondaryreticlecolor);
        MSG_WriteByte(msg, this->secondarylens);
        MSG_WriteByte(msg, this->secondaryemblem);
        MSG_WriteByte(msg, this->secondarytag);
        MSG_WriteByte(msg, this->specialty1);
        MSG_WriteByte(msg, this->specialty2);
        MSG_WriteByte(msg, this->specialty3);
        MSG_WriteByte(msg, this->primarygrenade);
        MSG_WriteByte(msg, this->specialgrenade);
        MSG_WriteByte(msg, this->equipment);
        MSG_WriteByte(msg, this->team);
    }
    else
    {
        MSG_WriteBit0(msg);
    }
}

void __thiscall _CustomClassData::ReadClassFromClientData(msg_t *msg)
{
    this->isActive = MSG_ReadBit(msg);
    if ( this->isActive )
    {
        this->primary = MSG_ReadByte(msg);
        this->primaryattachment = MSG_ReadByte(msg);
        this->primaryattachmenttop = MSG_ReadByte(msg);
        this->primaryattachmentbottom = MSG_ReadByte(msg);
        this->primaryattachmenttrigger = MSG_ReadByte(msg);
        this->primaryattachmentmuzzle = MSG_ReadByte(msg);
        this->primarycamo = MSG_ReadByte(msg);
        this->primaryreticle = MSG_ReadByte(msg);
        this->primaryreticlecolor = MSG_ReadByte(msg);
        this->primarylens = MSG_ReadByte(msg);
        this->primaryemblem = MSG_ReadByte(msg);
        this->primarytag = MSG_ReadByte(msg);
        this->secondary = MSG_ReadByte(msg);
        this->secondaryattachment = MSG_ReadByte(msg);
        this->secondaryattachmenttop = MSG_ReadByte(msg);
        this->secondaryattachmentbottom = MSG_ReadByte(msg);
        this->secondaryattachmenttrigger = MSG_ReadByte(msg);
        this->secondaryattachmentmuzzle = MSG_ReadByte(msg);
        this->secondarycamo = MSG_ReadByte(msg);
        this->secondaryreticle = MSG_ReadByte(msg);
        this->secondaryreticlecolor = MSG_ReadByte(msg);
        this->secondarylens = MSG_ReadByte(msg);
        this->secondaryemblem = MSG_ReadByte(msg);
        this->secondarytag = MSG_ReadByte(msg);
        this->specialty1 = MSG_ReadByte(msg);
        this->specialty2 = MSG_ReadByte(msg);
        this->specialty3 = MSG_ReadByte(msg);
        this->primarygrenade = MSG_ReadByte(msg);
        this->specialgrenade = MSG_ReadByte(msg);
        this->equipment = MSG_ReadByte(msg);
        this->team = MSG_ReadByte(msg);
    }
}

void __cdecl UI_Gametype_Custom_WriteClassClientData(msg_t *msg)
{
    int c; // [esp+0h] [ebp-4h]

    if ( Dvar_GetInt(custom_class_mode) )
    {
        MSG_WriteBit1(msg);
        for ( c = 0; c < 10; ++c )
            g_customGameModeClasses[c].WriteClassToClientData(msg);
    }
    else
    {
        MSG_WriteBit0(msg);
    }
    if ( HasClassDescriptionDataChanged() )
    {
        MSG_WriteBit1(msg);
        MSG_WriteData(msg, (unsigned __int8 *)g_customGameModeClassDescriptions, 0xA0u);
    }
    else
    {
        MSG_WriteBit0(msg);
    }
}

void __cdecl UI_Gametype_Custom_ReadClassClientData(msg_t *msg)
{
    int c; // [esp+0h] [ebp-Ch]
    int isUsingCustomClasses; // [esp+8h] [ebp-4h]

    isUsingCustomClasses = MSG_ReadBit(msg);
    Dvar_SetInt((dvar_s *)custom_class_mode, isUsingCustomClasses != 0);
    if ( isUsingCustomClasses )
    {
        for ( c = 0; c < 10; ++c )
            g_customGameModeClasses[c].ReadClassFromClientData(msg);
    }
    if ( MSG_ReadBit(msg) )
        MSG_ReadData(msg, (unsigned __int8 *)g_customGameModeClassDescriptions, 160);
}

void __cdecl UI_Gametype_Custom_WriteKillstreakClientData(msg_t *msg)
{
    const dvar_s *Var; // eax
    unsigned __int8 v2; // al
    const dvar_s *v3; // eax
    unsigned __int8 v4; // al
    const dvar_s *v5; // eax
    unsigned __int8 v6; // al
    int customKillstreakMode; // [esp+0h] [ebp-4h]

    customKillstreakMode = Dvar_GetInt(custom_killstreak_mode);
    if ( customKillstreakMode == 1 || customKillstreakMode == 2 )
    {
        MSG_WriteBit1(msg);
        MSG_WriteByte(msg, customKillstreakMode);
        if ( customKillstreakMode == 2 )
        {
            Var = Dvar_FindVar("custom_killstreak_1");
            v2 = atoi(Var->current.string);
            MSG_WriteByte(msg, v2);
            v3 = Dvar_FindVar("custom_killstreak_2");
            v4 = atoi(v3->current.string);
            MSG_WriteByte(msg, v4);
            v5 = Dvar_FindVar("custom_killstreak_3");
            v6 = atoi(v5->current.string);
            MSG_WriteByte(msg, v6);
        }
    }
    else
    {
        MSG_WriteBit0(msg);
    }
}

void __cdecl UI_Gametype_Custom_ReadKillstreakClientData(msg_t *msg)
{
    int Byte; // eax
    char *v2; // eax
    int v3; // eax
    char *v4; // eax
    int v5; // eax
    char *v6; // eax
    int customKillstreakMode; // [esp+0h] [ebp-8h]

    if ( MSG_ReadBit(msg) )
    {
        customKillstreakMode = MSG_ReadByte(msg);
        Dvar_SetInt((dvar_s *)custom_killstreak_mode, customKillstreakMode);
        if ( customKillstreakMode == 2 )
        {
            Byte = MSG_ReadByte(msg);
            v2 = va("%d", Byte);
            Dvar_SetStringByName("custom_killstreak_1", v2);
            v3 = MSG_ReadByte(msg);
            v4 = va("%d", v3);
            Dvar_SetStringByName("custom_killstreak_2", v4);
            v5 = MSG_ReadByte(msg);
            v6 = va("%d", v5);
            Dvar_SetStringByName("custom_killstreak_3", v6);
        }
    }
}

void __cdecl UI_Gametype_Custom_WritePerkDvarClientData(msg_t *msg)
{
    UI_Gametype_DvarsToCustomGameModeData(&g_customGameModeData);
    MSG_WriteFloat(msg, SLODWORD(g_customGameModeData.perk_extraBreath));
    MSG_WriteFloat(msg, SLODWORD(g_customGameModeData.perk_bulletPenetrationMultiplier));
    MSG_WriteFloat(msg, SLODWORD(g_customGameModeData.perk_weapSpreadMultiplier));
    MSG_WriteFloat(msg, SLODWORD(g_customGameModeData.perk_weapReloadMultiplier));
    MSG_WriteFloat(msg, SLODWORD(g_customGameModeData.perk_weapMeleeMultiplier));
    MSG_WriteFloat(msg, SLODWORD(g_customGameModeData.perk_weapSwitchMultiplier));
    MSG_WriteFloat(msg, SLODWORD(g_customGameModeData.perk_weapAdsMultiplier));
    MSG_WriteFloat(msg, SLODWORD(g_customGameModeData.perk_sprintMultiplier));
    MSG_WriteFloat(msg, SLODWORD(g_customGameModeData.perk_sprintRecoveryMultiplier));
    MSG_WriteFloat(msg, SLODWORD(g_customGameModeData.player_lastStandBleedoutTime));
}

void __cdecl UI_Gametype_Custom_ReadPerkDvarClientData(msg_t *msg)
{
    g_customGameModeData.perk_extraBreath = MSG_ReadFloat(msg);
    g_customGameModeData.perk_bulletPenetrationMultiplier = MSG_ReadFloat(msg);
    g_customGameModeData.perk_weapSpreadMultiplier = MSG_ReadFloat(msg);
    g_customGameModeData.perk_weapReloadMultiplier = MSG_ReadFloat(msg);
    g_customGameModeData.perk_weapMeleeMultiplier = MSG_ReadFloat(msg);
    g_customGameModeData.perk_weapSwitchMultiplier = MSG_ReadFloat(msg);
    g_customGameModeData.perk_weapAdsMultiplier = MSG_ReadFloat(msg);
    g_customGameModeData.perk_sprintMultiplier = MSG_ReadFloat(msg);
    g_customGameModeData.perk_sprintRecoveryMultiplier = MSG_ReadFloat(msg);
    g_customGameModeData.player_lastStandBleedoutTime = MSG_ReadFloat(msg);
    UI_Gametype_CustomGameModeDataToDvars();
}

void __cdecl UI_Gametype_Custom_WriteClientData(msg_t *msg)
{
    if ( customGameMode->current.integer )
    {
        MSG_WriteBit1(msg);
        UI_Gametype_Custom_WriteClassClientData(msg);
        UI_Gametype_Custom_WriteKillstreakClientData(msg);
        UI_Gametype_Custom_WritePerkDvarClientData(msg);
    }
    else
    {
        MSG_WriteBit0(msg);
    }
}

char __cdecl UI_Gametype_Custom_ReadClientData(msg_t *msg)
{
    int Bit; // eax

    Bit = MSG_ReadBit(msg);
    Dvar_SetInt((dvar_s *)customGameMode, Bit);
    if ( !customGameMode->current.integer )
        return 0;
    UI_Gametype_Custom_ReadClassClientData(msg);
    UI_Gametype_Custom_ReadKillstreakClientData(msg);
    UI_Gametype_Custom_ReadPerkDvarClientData(msg);
    return 1;
}

bool __cdecl UI_Gametype_IsDirty()
{
    return s_isCustomGameModeDirty;
}

void __cdecl UI_Gametype_MarkDirty()
{
    s_isCustomGameModeDirty = 1;
}

bool __cdecl UI_Gametype_HasPerkChangedFromDefault(int itemIndex)
{
    bool result; // al
    bool v2; // [esp+0h] [ebp-20h]
    bool v3; // [esp+4h] [ebp-1Ch]
    bool v4; // [esp+8h] [ebp-18h]
    bool v5; // [esp+Ch] [ebp-14h]
    bool v6; // [esp+10h] [ebp-10h]
    bool v7; // [esp+14h] [ebp-Ch]

    if ( (unsigned int)(itemIndex - 150) >= 0x1E
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_gametype_custom_mp.cpp",
                    1591,
                    0,
                    "perkIndex doesn't index CUSTOM_PERKS_COUNT\n\t%i not in [0, %i)",
                    itemIndex - 150,
                    30) )
    {
        __debugbreak();
    }
    switch ( itemIndex )
    {
        case 150:
        case 151:
            result = UI_Gametype_HasDvarChanged(perk_speedMultiplier);
            break;
        case 156:
            result = UI_Gametype_HasDvarChanged(perk_flakJacket);
            break;
        case 157:
            v7 = UI_Gametype_HasDvarChanged(perk_flakJacket) || UI_Gametype_HasDvarChanged(perk_fireproof);
            result = v7;
            break;
        case 158:
        case 159:
            result = UI_Gametype_HasDvarChanged(perk_killstreakReduction);
            break;
        case 160:
            result = UI_Gametype_HasDvarChanged(perk_weapSpreadMultiplier);
            break;
        case 161:
            v6 = UI_Gametype_HasDvarChanged(perk_weapSpreadMultiplier)
                || UI_Gametype_HasDvarChanged(perk_sprintRecoveryMultiplier)
                || UI_Gametype_HasDvarChanged(perk_weapMeleeMultiplier);
            result = v6;
            break;
        case 162:
            result = UI_Gametype_HasDvarChanged(perk_extraBreath);
            break;
        case 163:
            v5 = UI_Gametype_HasDvarChanged(perk_extraBreath) || UI_Gametype_HasDvarChanged(perk_weapSwitchMultiplier);
            result = v5;
            break;
        case 164:
            result = UI_Gametype_HasDvarChanged(perk_bulletPenetrationMultiplier);
            break;
        case 165:
            v4 = UI_Gametype_HasDvarChanged(perk_bulletPenetrationMultiplier)
                || UI_Gametype_HasDvarChanged(perk_armorPiercing)
                || UI_Gametype_HasDvarChanged(perk_damageKickReduction);
            result = v4;
            break;
        case 166:
            result = UI_Gametype_HasDvarChanged(perk_weapReloadMultiplier);
            break;
        case 167:
            v3 = UI_Gametype_HasDvarChanged(perk_weapReloadMultiplier) || UI_Gametype_HasDvarChanged(perk_weapAdsMultiplier);
            result = v3;
            break;
        case 170:
            result = UI_Gametype_HasDvarChanged(perk_sprintMultiplier);
            break;
        case 174:
            result = UI_Gametype_HasDvarChanged(player_lastStandBleedoutTimeNoRevive);
            break;
        case 175:
            v2 = UI_Gametype_HasDvarChanged(player_lastStandBleedoutTime) || UI_Gametype_HasDvarChanged(revive_time_taken);
            result = v2;
            break;
        case 177:
            result = UI_Gametype_HasDvarChanged(perk_disarmExplosiveTime);
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

bool __cdecl UI_Gametype_Custom_IsPerkTweakable(int itemIndex)
{
    bool result; // al

    if ( (unsigned int)(itemIndex - 150) >= 0x1E
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_gametype_custom_mp.cpp",
                    1647,
                    0,
                    "perkIndex doesn't index CUSTOM_PERKS_COUNT\n\t%i not in [0, %i)",
                    itemIndex - 150,
                    30) )
    {
        __debugbreak();
    }
    switch ( itemIndex )
    {
        case 152:
        case 153:
        case 154:
        case 155:
        case 168:
        case 169:
        case 171:
        case 172:
        case 173:
        case 176:
        case 178:
        case 179:
            result = 0;
            break;
        default:
            result = 1;
            break;
    }
    return result;
}

void __cdecl UI_Gametype_Custom_ResolveNumKillsConflicts()
{
    const char *v0; // eax
    const char *v1; // eax
    int v2; // eax
    const char *v3; // eax
    int v4; // [esp+4h] [ebp-20h]
    char *dvarName; // [esp+8h] [ebp-1Ch]
    int Int; // [esp+Ch] [ebp-18h]
    int currentKillNum; // [esp+10h] [ebp-14h]
    int firstOtherKillNum; // [esp+18h] [ebp-Ch]
    int secondOtherKillNum; // [esp+1Ch] [ebp-8h]
    unsigned int arrayIndex; // [esp+20h] [ebp-4h]

    Int = Dvar_GetInt("custom_killstreak_edit_slot");
    dvarName = va("custom_killstreak_%d_kills", Int);
    currentKillNum = Dvar_GetInt(dvarName);
    if ( Int + 1 <= 3 )
        v4 = Int + 1;
    else
        v4 = Int - 2;
    v0 = va("custom_killstreak_%d_kills", v4);
    firstOtherKillNum = Dvar_GetInt(v0);
    if ( Int + 2 <= 3 )
        v1 = va("custom_killstreak_%d_kills", Int + 2);
    else
        v1 = va("custom_killstreak_%d_kills", Int - 1);
    secondOtherKillNum = Dvar_GetInt(v1);
    arrayIndex = 0;
    while ( currentKillNum == firstOtherKillNum || currentKillNum == secondOtherKillNum )
    {
        currentKillNum = CustomKillstreakNums_0[arrayIndex];
        arrayIndex = (arrayIndex + 1) % 0x16;
    }
    v2 = Dvar_GetInt("custom_killstreak_edit_slot");
    v3 = va("custom_killstreak_%d_kills", v2);
    Dvar_SetIntByName(v3, currentKillNum);
}

char gamerTag[32];
char metaData[255];
void __cdecl UI_Gametype_Custom_UploadToFileShareSuccess(int controllerIndex, unsigned __int64 fileID)
{
    char *ClientName; // eax
    unsigned __int64 v3; // rax
    const char *v4; // eax
    char *v5; // eax
    unsigned __int64 v6; // rax
    const char *v7; // eax
    bool v8; // al
    const char *String; // eax
    bool Bool; // al
    int LocalClientNum; // eax
    char *v12; // [esp-8h] [ebp-568h]
    int v13; // [esp+10h] [ebp-550h]
    bdTaskResult *j; // [esp+14h] [ebp-54Ch]
    int v15; // [esp+18h] [ebp-548h]
    bdTaskResult *k; // [esp+1Ch] [ebp-544h]
    int v17; // [esp+20h] [ebp-540h]
    bdTaskResult *m; // [esp+24h] [ebp-53Ch]
    int v19; // [esp+28h] [ebp-538h]
    bdTaskResult *n; // [esp+2Ch] [ebp-534h]
    int v21; // [esp+30h] [ebp-530h]
    bdTaskResult *ii; // [esp+34h] [ebp-52Ch]
    int v23; // [esp+38h] [ebp-528h]
    bdTaskResult *jj; // [esp+3Ch] [ebp-524h]
    unsigned int value; // [esp+40h] [ebp-520h]
    int v26; // [esp+48h] [ebp-518h]
    bdTaskResult *kk; // [esp+4Ch] [ebp-514h]
    int v28; // [esp+50h] [ebp-510h]
    bdTaskResult *mm; // [esp+54h] [ebp-50Ch]
    int v30; // [esp+58h] [ebp-508h]
    bdTaskResult *nn; // [esp+5Ch] [ebp-504h]
    //int v32; // [esp+60h] [ebp-500h]
    bdTag *i; // [esp+64h] [ebp-4FCh]
    char backup[260]; // [esp+68h] [ebp-4F8h] BYREF
    const char *result; // [esp+170h] [ebp-3F0h]
    int index; // [esp+174h] [ebp-3ECh]
    ddlState_t state; // [esp+178h] [ebp-3E8h] BYREF
    int baseGameType; // [esp+188h] [ebp-3D8h]
    const StringTable *gameTypesTable; // [esp+18Ch] [ebp-3D4h] BYREF
    int numTags; // [esp+190h] [ebp-3D0h] BYREF
    int metaDataSize; // [esp+194h] [ebp-3CCh]
    bdTag tags[40]; // [esp+198h] [ebp-3C8h] BYREF
    int v43; // [esp+558h] [ebp-8h] BYREF
    int bitSize; // [esp+55Ch] [ebp-4h]

    metaDataSize = 0;
    numTags = 0;
    //v32 = 40;
    //for ( i = tags; --v32 >= 0; ++i )
    //    bdTag::bdTag(i);
    ClientName = Live_ControllerIndex_GetClientName(controllerIndex);
    I_strncpyz(gamerTag, ClientName, 32);
    memset(metaData, 0, sizeof(metaData));
    LODWORD(v3) = Live_GetXuid(controllerIndex);
    Live_FileShare_AddTag(4u, v3, &numTags, tags, 40);
    Live_FileShare_AddTag(3u, 4u, &numTags, tags, 40);
    baseGameType = 0;
    StringTable_GetAsset("mp/gametypesTable.csv", (XAssetHeader *)&gameTypesTable);
    if ( !gameTypesTable
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_gametype_custom_mp.cpp",
                    1724,
                    0,
                    "%s",
                    "gameTypesTable") )
    {
        __debugbreak();
    }
    if ( gameTypesTable )
    {
        for ( index = 0; index < gameTypesTable->rowCount; ++index )
        {
            v4 = va("%d", index);
            result = StringTable_Lookup(gameTypesTable, 4, v4, 0);
            if ( result && *result && !I_stricmp(result, ui_gametype->current.string) )
            {
                baseGameType = index;
                break;
            }
        }
        if ( index == gameTypesTable->rowCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_gametype_custom_mp.cpp",
                        1741,
                        0,
                        "%s",
                        "index != gameTypesTable->rowCount") )
        {
            __debugbreak();
        }
        if ( index != gameTypesTable->rowCount )
            Live_FileShare_AddTag(1u, baseGameType, &numTags, tags, 40);
    }
    if ( DDL_AssociateBuffer(metaData, 255, g_fileshareDDL)
        || DDL_FixBufferVersion(metaData, g_fileshareDDL, "ddl_mp/file_share.ddl", backup, 255) )
    {
        bitSize = DDL_GetTotalBufferBitSize(g_fileshareDDL);
        metaDataSize = bitSize / 8;
        if ( bitSize % 8 )
            ++metaDataSize;
        if ( DDL_MoveToName(&g_fileShareRootState, &state, "authorName")
            && (v5 = Live_ControllerIndex_GetClientName(controllerIndex), DDL_SetString(&state, v5, metaData)) )
        {
            if ( DDL_MoveToName(&g_fileShareRootState, &state, "authorXuid")
                && (LODWORD(v6) = Live_GetXuid(controllerIndex), DDL_SetInt64(&state, v6, metaData)) )
            {
                if ( DDL_MoveToName(&g_fileShareRootState, &state, "createTime")
                    && (value = _time64(0), DDL_SetInt(&state, value, metaData)) )
                {
                    if ( DDL_MoveToName(&g_fileShareRootState, &state, "name")
                        && (v7 = Dvar_GetString("fsSelectedFileName"), DDL_SetString(&state, v7, metaData)) )
                    {
                        if ( DDL_MoveToName(&g_fileShareRootState, &state, "isModifiedName")
                            && (v8 = Dvar_GetBool("fsIsSelectedFileNameModified"), DDL_SetInt(&state, v8, metaData)) )
                        {
                            if ( DDL_MoveToName(&g_fileShareRootState, &state, "description")
                                && (String = Dvar_GetString("fsSelectedFileDescription"), DDL_SetString(&state, String, metaData)) )
                            {
                                if ( DDL_MoveToName(&g_fileShareRootState, &state, "isModifiedDescription")
                                    && (Bool = Dvar_GetBool("fsIsSelectedFileDescriptionModified"), DDL_SetInt(&state, Bool, metaData)) )
                                {
                                    LiveStorage_FileShare_WriteSummary(
                                        controllerIndex,
                                        fileID,
                                        FILESHARE_LOCATION_USERSTORAGE,
                                        gamerTag,
                                        strlen(gamerTag),
                                        metaData,
                                        metaDataSize,
                                        tags,
                                        numTags,
                                        0);
                                    LiveCounter_IncrementCounterValueByName("global_customgames_created", 1u);
                                    LiveCounter_IncrementCounterValueByName("global_fileshare_shared", 1u);
                                    v12 = UI_SafeTranslateString("MENU_SAVED_CAPS");
                                    LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
                                    UI_OpenToastPopup(LocalClientNum, "menu_mp_killstreak_select", "", v12, 2700);
                                    Z_VirtualFree(s_fileShareBuffer, 0);
                                    //v13 = 40;
                                    //for ( j = (bdTaskResult *)&v43; --v13 >= 0; bdTag::~bdTag(j) )
                                    //    j -= 6;
                                }
                                else
                                {
                                    Com_PrintError(16, "Could not set 'isModifiedDescription' in the file share ddl.\n");
                                    //v15 = 40;
                                    //for ( k = (bdTaskResult *)&v43; --v15 >= 0; bdTag::~bdTag(k) )
                                    //    k -= 6;
                                }
                            }
                            else
                            {
                                Com_PrintError(16, "Could not set 'description' in the file share ddl.\n");
                                //v17 = 40;
                                //for ( m = (bdTaskResult *)&v43; --v17 >= 0; bdTag::~bdTag(m) )
                                //    m -= 6;
                            }
                        }
                        else
                        {
                            Com_PrintError(16, "Could not set 'isModifiedName' in the file share ddl.\n");
                            //v19 = 40;
                            //for ( n = (bdTaskResult *)&v43; --v19 >= 0; bdTag::~bdTag(n) )
                            //    n -= 6;
                        }
                    }
                    else
                    {
                        Com_PrintError(16, "Could not set 'name' in the file share ddl.\n");
                        //v21 = 40;
                        //for ( ii = (bdTaskResult *)&v43; --v21 >= 0; bdTag::~bdTag(ii) )
                        //    ii -= 6;
                    }
                }
                else
                {
                    Com_PrintError(16, "Could not set 'createTime' in the file share ddl.\n");
                    //v23 = 40;
                    //for ( jj = (bdTaskResult *)&v43; --v23 >= 0; bdTag::~bdTag(jj) )
                    //    jj -= 6;
                }
            }
            else
            {
                Com_PrintError(16, "Could not set 'authorXuid' in the file share ddl.\n");
                //v26 = 40;
                //for ( kk = (bdTaskResult *)&v43; --v26 >= 0; bdTag::~bdTag(kk) )
                //    kk -= 6;
            }
        }
        else
        {
            Com_PrintError(16, "Could not set 'authorName' in the file share ddl.\n");
            //v28 = 40;
            //for ( mm = (bdTaskResult *)&v43; --v28 >= 0; bdTag::~bdTag(mm) )
            //    mm -= 6;
        }
    }
    else
    {
        Com_PrintError(16, "Could not read file share meta data buffer.\n");
        //v30 = 40;
        //for ( nn = (bdTaskResult *)&v43; --v30 >= 0; bdTag::~bdTag(nn) )
        //    nn -= 6;
    }
}

void __cdecl UI_Gametype_Custom_UploadToFileShareFailure()
{
    Z_VirtualFree(s_fileShareBuffer, 0);
}

void __cdecl UI_Gametype_Custom_UploadToFileShare(
                int controllerIndex,
                unsigned __int16 fileSlot,
                const char *fileName,
                MemoryFile *customGameMode)
{
    fileShareWriteFileInfo writeFileInfo; // [esp+0h] [ebp-34h] BYREF

    MemFile_StartSegment(customGameMode, -1);
    writeFileInfo.category = 4;
    writeFileInfo.fileData = customGameMode->buffer;
    writeFileInfo.fileSize = customGameMode->bufferSize;
    writeFileInfo.location = FILESHARE_LOCATION_USERSTORAGE;
    writeFileInfo.fileSlot = fileSlot;
    s_fileShareBuffer = writeFileInfo.fileData;
    writeFileInfo.tags = 0;
    writeFileInfo.fileName = fileName;
    memset(&writeFileInfo.thumbData, 0, 12);
    writeFileInfo.dataCallback = 0;
    writeFileInfo.successCallback = UI_Gametype_Custom_UploadToFileShareSuccess;
    writeFileInfo.failureCallback = (void (__cdecl *)(int))UI_Gametype_Custom_UploadToFileShareFailure;
    LiveStorage_FileShare_WriteFile(controllerIndex, &writeFileInfo);
}

void __cdecl UI_Gametype_UploadToFileShare_f()
{
    const char *v0; // eax
    unsigned __int16 v1; // ax
    UIGametypeFileHeader header; // [esp+8h] [ebp-80F8h] BYREF
    MemoryFile memFile; // [esp+D0h] [ebp-8030h] BYREF

    UI_Gametype_EndChanges();
    UI_Gametype_WriteToMemFile(&memFile, &header);
    v0 = Cmd_Argv(1);
    v1 = atoi(v0);
    UI_Gametype_Custom_UploadToFileShare(0, v1, header.name, &memFile);
}

void __cdecl UI_Gametype_FileShareDownloadComplete(dwFileShareReadFileTask *task)
{
    int localClientNum; // [esp+0h] [ebp-8034h]
    MemoryFile memFile; // [esp+4h] [ebp-8030h] BYREF

    MemFile_InitForReading(&memFile, task->fileSize, (unsigned __int8 *)task->buffer, 0);
    if ( UI_Gametype_ReadFromMemFile(&memFile) )
    {
        Dvar_SetInt((dvar_s *)customGameMode, 1);
        UI_Gametype_DvarsToCustomGameModeData(&g_customGameModeData);
    }
    localClientNum = Com_ControllerIndex_GetLocalClientNum(s_fileShareControllerIndex);
    if ( Dvar_GetInt("ui_custom_gobackonupload") )
    {
        UI_CloseMenu(localClientNum, "custom_game_create");
        UI_CloseMenu(localClientNum, "game_mode");
    }
    Z_VirtualFree(task->buffer, 0);
}

void __cdecl UI_Gametype_FileShareDownloadFailed(dwFileShareReadFileTask *task)
{
    int LocalClientNum; // eax

    Com_PrintWarning(14, "downloadCustomGametype: Failed to download from file share.\n");
    Z_VirtualFree(task->buffer, 0);
    LocalClientNum = Com_ControllerIndex_GetLocalClientNum(s_fileShareControllerIndex);
    UI_OpenMenu(LocalClientNum, "menu_fileshare_error");
}

void __cdecl UI_Gametype_DownloadFromFileShare_f()
{
    const char *v0; // eax
    const char *v1; // eax
    __int64 fileId; // [esp+20h] [ebp-38h]
    fileShareReadFileInfo fileInfo; // [esp+28h] [ebp-30h] BYREF

    if ( Cmd_Argc() >= 3 )
    {
        v0 = Cmd_Argv(1);
        fileId = I_atoi64(v0);
        v1 = Cmd_Argv(2);
        fileInfo.fileID = fileId;
        fileInfo.fileSize = atoi(v1);
        fileInfo.location = FILESHARE_LOCATION_USERSTORAGE;
        fileInfo.buffer = Z_VirtualAlloc(0x2000, "custom_gametype", 0);
        fileInfo.bufferSize = 0x2000;
        fileInfo.isStreamed = 0;
        fileInfo.cacheBuffer = fileInfo.buffer;
        fileInfo.successCallback = UI_Gametype_FileShareDownloadComplete;
        fileInfo.failureCallback = UI_Gametype_FileShareDownloadFailed;
        s_fileShareControllerIndex = 0;
        if ( !LiveStorage_FileShare_ReadFile(0, &fileInfo) )
            Com_Printf(14, "downloadCustomGametype: Task not started. Confirm that the game was found in the cache.\n");
    }
    else
    {
        Com_PrintWarning(14, "downloadCustomGametype: incorrect parameters.\n");
    }
}

void __cdecl SV_GameType_DownloadFromFileshare_f()
{
    int i; // [esp+10h] [ebp-4Ch]
    const char *gameName; // [esp+14h] [ebp-48h]
    fileSharePrivateData *fileShareData; // [esp+18h] [ebp-44h]
    unsigned int fileSize; // [esp+20h] [ebp-3Ch]
    unsigned __int64 fileId; // [esp+24h] [ebp-38h]
    fileShareReadFileInfo fileInfo; // [esp+2Ch] [ebp-30h] BYREF

    if ( Cmd_Argc() >= 2 )
    {
        fileShareData = LiveStorage_FileShare_GetFileShareData(FILESHARE_BUFFER_SECONDARY);
        if ( fileShareData->valid )
        {
            gameName = Cmd_Argv(1);
            fileId = 0;
            fileSize = 0;
            for ( i = 0; i < fileShareData->descriptorCount; ++i )
            {
                if ( !I_stricmp(gameName, fileShareData->descriptors[i].m_fileName) )
                {
                    fileId = fileShareData->descriptors[i].m_fileID;
                    fileSize = fileShareData->descriptors[i].m_fileSize;
                    break;
                }
            }
            if ( fileSize )
            {
                fileInfo.fileID = fileId;
                fileInfo.fileSize = fileSize;
                fileInfo.location = FILESHARE_LOCATION_USERSTORAGE;
                fileInfo.buffer = Z_VirtualAlloc(0x2000, "custom_gametype", 0);
                fileInfo.bufferSize = 0x2000;
                fileInfo.isStreamed = 0;
                fileInfo.cacheBuffer = fileInfo.buffer;
                fileInfo.successCallback = UI_Gametype_FileShareDownloadComplete;
                fileInfo.failureCallback = UI_Gametype_FileShareDownloadFailed;
                if ( !LiveStorage_FileShare_ReadFile(0, &fileInfo) )
                    Com_Printf(15, "downloadCustomGametype: Task not started. Confirm that the game was found in the cache.\n");
            }
            else
            {
                Com_PrintError(15, "Couldn't find any game called %s\n", gameName);
            }
        }
        else
        {
            Com_PrintWarning(15, "call listcustomgametypes first\n");
        }
    }
    else
    {
        Com_PrintWarning(15, "usage: loadusergametype filename\n");
    }
}

void __cdecl UI_Gametype_DisableCheats_f()
{
    Dvar_SetBoolByName("sv_cheats", 0);
}

void __cdecl UI_Gametype_CopyCustomClass_f()
{
    const char *v0; // eax
    const char *v1; // eax
    _CustomClassDescription *v2; // ecx
    _CustomClassDescription *v3; // edx
    int classIndexToCopy; // [esp+20h] [ebp-8h]
    int classIndexToReplace; // [esp+24h] [ebp-4h]

    if ( Cmd_Argc() >= 3 )
    {
        v0 = Cmd_Argv(1);
        classIndexToCopy = atoi(v0);
        v1 = Cmd_Argv(2);
        classIndexToReplace = atoi(v1);
        if ( classIndexToCopy < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_gametype_custom_mp.cpp",
                        2048,
                        0,
                        "%s",
                        "classIndexToCopy >= 0") )
        {
            __debugbreak();
        }
        if ( classIndexToReplace < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_gametype_custom_mp.cpp",
                        2049,
                        0,
                        "%s",
                        "classIndexToReplace >= 0") )
        {
            __debugbreak();
        }
        if ( classIndexToCopy >= 10
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_gametype_custom_mp.cpp",
                        2050,
                        0,
                        "%s",
                        "classIndexToCopy < NUM_CUSTOM_CLASSES") )
        {
            __debugbreak();
        }
        if ( classIndexToReplace >= 10
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_gametype_custom_mp.cpp",
                        2051,
                        0,
                        "%s",
                        "classIndexToReplace < NUM_CUSTOM_CLASSES") )
        {
            __debugbreak();
        }
        if ( classIndexToReplace >= classIndexToCopy )
            ++classIndexToReplace;
        memcpy(
            &g_customGameModeClasses[classIndexToReplace],
            &g_customGameModeClasses[classIndexToCopy],
            sizeof(_CustomClassData));
        v2 = &g_customGameModeClassDescriptions[classIndexToCopy];
        v3 = &g_customGameModeClassDescriptions[classIndexToReplace];
        *(unsigned int *)v3->name = *(unsigned int *)v2->name;
        *(unsigned int *)&v3->name[4] = *(unsigned int *)&v2->name[4];
        *(unsigned int *)&v3->name[8] = *(unsigned int *)&v2->name[8];
        *(unsigned int *)&v3->name[12] = *(unsigned int *)&v2->name[12];
        UI_Gametype_MarkDirty();
    }
    else
    {
        Com_PrintError(
            15,
            "copyCustomGametypeClass usage: copyCustomGametypeClass <classIndexToCopy> <classIndexToReplace>\n");
    }
}

cmd_function_s UI_Gametype_DisableCheats_f_VAR;
cmd_function_s UI_Gametype_Custom_Reset_f_VAR;
cmd_function_s UI_Gametype_EndChanges_f_VAR;
cmd_function_s UI_Gametype_BeginChanges_f_VAR;
cmd_function_s UI_Gametype_RevertChanges_f_VAR;
cmd_function_s UI_Gametype_CheckIfAnythingChanged_f_VAR;
cmd_function_s UI_Gametype_UpdateCustomClassDvarsForClass_f_VAR;
cmd_function_s UI_Gametype_UpdateClassDataFromDvars_f_VAR;
cmd_function_s UI_Gametype_CreateDefaultCustomClassIfNeeded_f_VAR;
cmd_function_s UI_Gametype_ApplyCustomClassTeamSelections_f_VAR;
cmd_function_s UI_Gametype_UpdateCustomClassTeamDvars_f_VAR;
cmd_function_s UI_Gametype_CopyCustomClass_f_VAR;
cmd_function_s UI_Gametype_UploadToFileShare_f_VAR;
cmd_function_s UI_Gametype_DownloadFromFileShare_f_VAR;
cmd_function_s SV_GameType_DownloadFromFileshare_f_VAR;

void __cdecl UI_Gametype_Custom_Init()
{
    Cmd_AddCommandInternal("disableCheats", UI_Gametype_DisableCheats_f, &UI_Gametype_DisableCheats_f_VAR);
    Cmd_AddCommandInternal("resetCustomGametype", UI_Gametype_Custom_Reset_f, &UI_Gametype_Custom_Reset_f_VAR);
    Cmd_AddCommandInternal("endCustomGametypeChanges", UI_Gametype_EndChanges_f, &UI_Gametype_EndChanges_f_VAR);
    Cmd_AddCommandInternal("beginCustomGametypeChanges", UI_Gametype_BeginChanges_f, &UI_Gametype_BeginChanges_f_VAR);
    Cmd_AddCommandInternal("revertCustomGametypeChanges", UI_Gametype_RevertChanges_f, &UI_Gametype_RevertChanges_f_VAR);
    Cmd_AddCommandInternal(
        "checkIfCustomGametypeChanged",
        UI_Gametype_CheckIfAnythingChanged_f,
        &UI_Gametype_CheckIfAnythingChanged_f_VAR);
    Cmd_AddCommandInternal(
        "updateCustomClassDvarsForClass",
        UI_Gametype_UpdateCustomClassDvarsForClass_f,
        &UI_Gametype_UpdateCustomClassDvarsForClass_f_VAR);
    Cmd_AddCommandInternal(
        "updateCustomClassDataFromDvars",
        UI_Gametype_UpdateClassDataFromDvars_f,
        &UI_Gametype_UpdateClassDataFromDvars_f_VAR);
    Cmd_AddCommandInternal(
        "createDefaultCustomClassIfNeeded",
        UI_Gametype_CreateDefaultCustomClassIfNeeded_f,
        &UI_Gametype_CreateDefaultCustomClassIfNeeded_f_VAR);
    Cmd_AddCommandInternal(
        "applyCustomClassTeamSelections",
        UI_Gametype_ApplyCustomClassTeamSelections_f,
        &UI_Gametype_ApplyCustomClassTeamSelections_f_VAR);
    Cmd_AddCommandInternal(
        "updateCustomClassTeamDvars",
        UI_Gametype_UpdateCustomClassTeamDvars_f,
        &UI_Gametype_UpdateCustomClassTeamDvars_f_VAR);
    Cmd_AddCommandInternal("copyCustomGametypeClass", UI_Gametype_CopyCustomClass_f, &UI_Gametype_CopyCustomClass_f_VAR);
    Cmd_AddCommandInternal("uploadCustomGametype", UI_Gametype_UploadToFileShare_f, &UI_Gametype_UploadToFileShare_f_VAR);
    Cmd_AddCommandInternal(
        "downloadCustomGametype",
        UI_Gametype_DownloadFromFileShare_f,
        &UI_Gametype_DownloadFromFileShare_f_VAR);
    Cmd_AddCommandInternal(
        "loadusergametype",
        SV_GameType_DownloadFromFileshare_f,
        &SV_GameType_DownloadFromFileshare_f_VAR);
    UI_InitGametypeVariants();
    UI_Gametype_DvarsToCustomGameModeData(&g_customGameModeData);
    ui_items_no_cost = _Dvar_RegisterInt("ui_items_no_cost", 0, 0, 1, 0x80u, "Set to 1 if you want items to have no cost");
    custom_class_mode = _Dvar_RegisterInt("custom_class_mode", 0, 0, 1, 0, "Set to 1 if you want to use custom classes");
    custom_killstreak_mode = _Dvar_RegisterInt(
                                                         "custom_killstreak_mode",
                                                         0,
                                                         0,
                                                         2,
                                                         0,
                                                         "Set to 1 for no killstreaks and 2 for custom");
    custom_killstreak_numkills = _Dvar_RegisterInt(
                                                                 "custom_killstreak_numkills",
                                                                 1,
                                                                 1,
                                                                 0x7FFFFFFF,
                                                                 0,
                                                                 "used to communicate killstreak num from feeders");
    ui_useCustomClassInfo = _Dvar_RegisterInt(
                                                        "ui_useCustomClassInfo",
                                                        0,
                                                        0,
                                                        4,
                                                        0,
                                                        "set to 1 to divert ui class functions to use custmo class data");
}

void __cdecl UI_Gametype_CustomGameModeDataToDvars()
{
    if ( UI_Gametype_IsUsingCustom() )
    {
        Dvar_SetFloat((dvar_s *)perk_extraBreath, g_customGameModeData.perk_extraBreath);
        Dvar_SetFloat((dvar_s *)perk_bulletPenetrationMultiplier, g_customGameModeData.perk_bulletPenetrationMultiplier);
        Dvar_SetFloat((dvar_s *)perk_weapSpreadMultiplier, g_customGameModeData.perk_weapSpreadMultiplier);
        Dvar_SetFloat((dvar_s *)perk_weapReloadMultiplier, g_customGameModeData.perk_weapReloadMultiplier);
        Dvar_SetFloat((dvar_s *)perk_weapMeleeMultiplier, g_customGameModeData.perk_weapMeleeMultiplier);
        Dvar_SetFloat((dvar_s *)perk_weapSwitchMultiplier, g_customGameModeData.perk_weapSwitchMultiplier);
        Dvar_SetFloat((dvar_s *)perk_weapAdsMultiplier, g_customGameModeData.perk_weapAdsMultiplier);
        Dvar_SetFloat((dvar_s *)perk_sprintMultiplier, g_customGameModeData.perk_sprintMultiplier);
        Dvar_SetFloat((dvar_s *)perk_sprintRecoveryMultiplier, g_customGameModeData.perk_sprintRecoveryMultiplier);
        Dvar_SetFloat((dvar_s *)player_lastStandBleedoutTime, g_customGameModeData.player_lastStandBleedoutTime);
        Dvar_SetFloat((dvar_s *)perk_speedMultiplier, g_customGameModeData.perk_speedMultiplier);
        Dvar_SetFloat((dvar_s *)perk_flakJacket, g_customGameModeData.perk_flakJacket);
        Dvar_SetFloat((dvar_s *)perk_fireproof, g_customGameModeData.perk_fireproof);
        Dvar_SetFloat((dvar_s *)perk_armorPiercing, g_customGameModeData.perk_armorPiercing);
        Dvar_SetFloat(
            (dvar_s *)player_lastStandBleedoutTimeNoRevive,
            g_customGameModeData.player_lastStandBleedoutTimeNoRevive);
        Dvar_SetFloat((dvar_s *)revive_time_taken, g_customGameModeData.revive_time_taken);
        Dvar_SetFloat((dvar_s *)perk_disarmExplosiveTime, g_customGameModeData.perk_disarmExplosiveTime);
        Dvar_SetFloat((dvar_s *)perk_damageKickReduction, g_customGameModeData.perk_damageKickReduction);
        Dvar_SetInt((dvar_s *)perk_killstreakReduction, g_customGameModeData.perk_killstreakReduction);
    }
}

void __cdecl UI_Gametype_DvarsToCustomGameModeData(CustomGameModeData *customGameModeData)
{
    customGameModeData->perk_extraBreath = perk_extraBreath->current.value;
    customGameModeData->perk_bulletPenetrationMultiplier = perk_bulletPenetrationMultiplier->current.value;
    customGameModeData->perk_weapSpreadMultiplier = perk_weapSpreadMultiplier->current.value;
    customGameModeData->perk_weapReloadMultiplier = perk_weapReloadMultiplier->current.value;
    customGameModeData->perk_weapMeleeMultiplier = perk_weapMeleeMultiplier->current.value;
    customGameModeData->perk_weapSwitchMultiplier = perk_weapSwitchMultiplier->current.value;
    customGameModeData->perk_weapAdsMultiplier = perk_weapAdsMultiplier->current.value;
    customGameModeData->perk_sprintMultiplier = perk_sprintMultiplier->current.value;
    customGameModeData->perk_sprintRecoveryMultiplier = perk_sprintRecoveryMultiplier->current.value;
    customGameModeData->player_lastStandBleedoutTime = player_lastStandBleedoutTime->current.value;
    customGameModeData->perk_speedMultiplier = perk_speedMultiplier->current.value;
    customGameModeData->perk_flakJacket = perk_flakJacket->current.value;
    customGameModeData->perk_fireproof = perk_fireproof->current.value;
    customGameModeData->perk_armorPiercing = perk_armorPiercing->current.value;
    customGameModeData->player_lastStandBleedoutTimeNoRevive = player_lastStandBleedoutTimeNoRevive->current.value;
    customGameModeData->revive_time_taken = revive_time_taken->current.value;
    customGameModeData->perk_disarmExplosiveTime = perk_disarmExplosiveTime->current.value;
    customGameModeData->perk_damageKickReduction = perk_damageKickReduction->current.value;
    customGameModeData->perk_killstreakReduction = perk_killstreakReduction->current.integer;
}

