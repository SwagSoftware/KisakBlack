#pragma once

#include <universal/dvar.h>
#include <universal/mem_userhunk.h>

enum //$284BF4CF6978AE12DFEEB2363B7EFFE8 : __int32
{                                       // XREF: $4D82FFD23A232AE1E043C123231C9F85/r
    CS_GAME_VERSION            = 0x2,
    CS_SERVERID                = 0x3,
    CS_MESSAGE                 = 0x4,
    CS_SCORES1                 = 0x5,
    CS_SCORES2                 = 0x6,
    CS_CULLDIST                = 0x7,
    CS_SUNLIGHT                = 0x8,
    CS_SUNDIR                  = 0x9,
    CS_FOGVARS                 = 0xA,
    CS_MOTD                    = 0xB,
    CS_GAMEENDTIME             = 0xC,
    CS_MAPCENTER               = 0xD,
    CS_TIMESCALE               = 0xE,
    CS_VOTE_TIME               = 0xF,
    CS_VOTE_STRING             = 0x10,
    CS_VOTE_YES                = 0x11,
    CS_VOTE_NO                 = 0x12,
    CS_VOTE_MAPNAME            = 0x13,
    CS_VOTE_GAMETYPE           = 0x14,
    CS_MULTI_MAPWINNER         = 0x15,
    CS_ENEMY_CROSSHAIR         = 0x16,
    CS_CODINFO                 = 0x17,
    CS_CODINFO_LAST            = 0xAC,
    CS_CODINFO_VALUE           = 0xAD,
    CS_CODINFO_VALUE_LAST      = 0x142,
    CS_PLAYERINFOS             = 0x143,
    CS_PLAYERINFOS_LAST        = 0x162,
    CS_PLAYEREMBLEMS           = 0x163,
    CS_PLAYEREMBLEMS_LAST      = 0x182,
    CS_TARGETS                 = 0x183,
    CS_TARGETS_LAST            = 0x1A2,
    CS_USE_TRIG_STRINGS        = 0x1A3,
    CS_USE_TRIG_STRINGS_LAST   = 0x202,
    CS_LOCALIZED_STRINGS       = 0x203,
    CS_LOCALIZED_STRINGS_LAST  = 0x601,
    CS_CLIENTSYSTEM_NAMES      = 0x602,
    CS_CLIENTSYSTEM_NAMES_LAST = 0x60A,
    CS_CASE_INSENSITIVE_BEGIN  = 0x60B,
    CS_AMBIENT                 = 0x60B,
    CS_NORTHYAW                = 0x60C,
    CS_MINIMAP                 = 0x60D,
    CS_VISIONSET_NAKED         = 0x60E,
    CS_VISIONSET_NIGHT         = 0x60F,
    CS_NIGHTVISION             = 0x610,
    CS_LOC_SEL_MTLS            = 0x611,
    CS_LOC_SEL_MTLS_LAST       = 0x61F,
    CS_MODELS                  = 0x620,
    CS_MODELS_LAST             = 0x81F,
    CS_EFFECT_NAMES            = 0x820,
    CS_EFFECT_NAMES_LAST       = 0x8E3,
    CS_EFFECT_TAGS             = 0x8E4,
    CS_EFFECT_TAGS_LAST        = 0x9E3,
    CS_SHELLSHOCKS             = 0x9E4,
    CS_SHELLSHOCKS_LAST        = 0x9F3,
    CS_SCRIPT_MENUS            = 0x9F4,
    CS_SCRIPT_MENUS_LAST       = 0xA13,
    CS_SERVER_MATERIALS        = 0xA14,
    CS_SERVER_MATERIALS_LAST   = 0xB13,
    CS_WEAPONFILES             = 0xB14,
    CS_WEAPONFILES_LAST        = 0xC13,
    CS_STATUS_ICONS            = 0xC14,
    CS_STATUS_ICONS_LAST       = 0xC1B,
    CS_HEAD_ICONS              = 0xC1C,
    CS_HEAD_ICONS_LAST         = 0xC2A,
    CS_TAGS                    = 0xC2B,
    CS_TAGS_LAST               = 0xC4A,
    CS_ITEMS                   = 0xC4B,
    CS_VEHICLE_TYPES           = 0xC4C,
    CS_VEHICLE_TYPES_LAST      = 0xC6B,
    CS_DESTRUCTIBLES           = 0xC6C,
    CS_DESTRUCTIBLES_LAST      = 0xCAB,
    CS_ANIMTREES               = 0xCAC,
    CS_ANIMTREES_LAST          = 0xCBB,
    MAX_CONFIGSTRINGS          = 0xCBC,
};

void __cdecl SV_SetConfigstring(int index, char *val);
void __cdecl SV_GetConfigstring(unsigned int index, char *buffer, int bufferSize);
unsigned int __cdecl SV_GetConfigstringConst(unsigned int index);
void __cdecl SV_SetConfigValueForKey(int start, int max, char *key, char *value);
void __cdecl SV_SetUserinfo(int index, char *val);
void __cdecl SV_GetUserinfo(int index, char *buffer, int bufferSize);
void __cdecl SV_CreateBaseline();
void __cdecl SV_SetXUIDConfigStrings();
void __cdecl SV_Startup(int controllerIndex);
void __cdecl SV_SetExpectedHunkUsage(char *mapname);
void __cdecl SV_ClearServer();
void __cdecl SV_InitArchivedSnapshot();
void __cdecl SV_SetSystemInfoConfig();
void __cdecl SV_SaveSystemInfo();
void __cdecl SV_SetServerDvarsBeforeScriptsInit();
void __cdecl    SV_SpawnServer(int controllerIndex, char *server, int mapIsPreloaded, int savegame);
char *__cdecl SV_AllocateClientMemory_SizeRequired(int maxLocalClients, int maxClients);
void __cdecl SV_AllocateClientMemory(HunkUser *hunk, int maxLocalClients, int maxClients);
void __cdecl SV_FreeClientMemory(HunkUser *hunk);
bool __cdecl SV_Loaded();
void __cdecl SV_Init();
void __cdecl SV_DropAllClients();
void __cdecl SV_Shutdown(const char *finalmsg);
void __cdecl SV_FinalMessage(const char *message);
void __cdecl SV_ClearServerThreadOwnsGame();
void __cdecl SV_IncServerThreadOwnsGame();
void __cdecl SV_DecServerThreadOwnsGame();
int __cdecl SV_GetServerThreadOwnsGame();
void __cdecl SV_CheckThread();


extern const dvar_t *sv_gametype;
extern const dvar_t *sv_privateClients;
extern const dvar_t *sv_hostname;
extern const dvar_t *sv_noname;
extern const dvar_t *sv_geolocation;
extern const dvar_t *sv_maxgrouperrors;
extern const dvar_t *sv_ownerid;
extern const dvar_t *sv_numreservedslots;
extern const dvar_t *sv_clientSideBullets;
extern const dvar_t *sv_clientSideVehicles;
extern const dvar_t *sv_penetrationCount;
extern const dvar_t *sv_axis_penetrationCount;
extern const dvar_t *sv_allies_penetrationCount;
extern const dvar_t *sv_bullet_range;
extern const dvar_t *sv_hitFXFrustumCutoff;
extern const dvar_t *sv_punkbuster;
extern const dvar_t *sv_security;
extern const dvar_t *sv_ranked;
extern const dvar_t *ui_ranked;
extern const dvar_t *sv_dedicatedmaxclients;
extern const dvar_t *sv_maxclients;
extern const dvar_t *sv_maxRate;
extern const dvar_t *sv_minPing;
extern const dvar_t *sv_maxPing;
extern const dvar_t *sv_timeout;
extern const dvar_t *sv_connectTimeout;
extern const dvar_t *sv_floodProtect;
extern const dvar_t *sv_showCommands;
extern const dvar_t *sv_writeConfigStrings;
extern const dvar_t *scr_writeConfigStrings;
extern const dvar_t *sv_lsgerror;
extern const dvar_t *sv_allowAnonymous;
extern const dvar_t *sv_disableClientConsole;
extern const dvar_t *sv_privatePassword;
extern const dvar_t *sv_allowDownload;
extern const dvar_t *sv_iwds;
extern const dvar_t *sv_iwdNames;
extern const dvar_t *sv_referencedIwds;
extern const dvar_t *sv_referencedIwdNames;
extern const dvar_t *sv_FFCheckSums;
extern const dvar_t *sv_FFNames;
extern const dvar_t *sv_referencedFFCheckSums;
extern const dvar_t *sv_referencedFFNames;
extern const dvar_t *sv_authenticating;
extern const dvar_t *sv_voice;
extern const dvar_t *sv_voiceQuality;
extern const dvar_t *sv_cheats;
extern const dvar_t *sv_pure;
extern const dvar_t *rcon_password;
extern const dvar_t *sv_fps;
extern const dvar_t *sv_showPingSpam;
extern const dvar_t *sv_zombietime;
extern const dvar_t *sv_reconnectlimit;
extern const dvar_t *sv_padPackets;
extern const dvar_t *sv_allowedClan1;
extern const dvar_t *sv_allowedClan2;
extern const dvar_t *sv_packet_info;
extern const dvar_t *sv_showAverageBPS;
extern const dvar_t *sv_kickBanTime;
extern const dvar_t *sv_debugMessageKey;
extern const dvar_t *sv_debugPacketContents;
extern const dvar_t *sv_debugPacketContentsForClientThisFrame;
extern const dvar_t *sv_showHuffmanData;
extern const dvar_t *sv_debugConstantConfigStrings;
extern const dvar_t *sv_loadMyChanges;
extern const dvar_t *sv_debugPlayerstate;
extern const dvar_t *sv_debugPacketContentsQuick;
extern const dvar_t *sv_printMessageSize;
extern const dvar_t *sv_mapRotation;
extern const dvar_t *sv_mapRotationCurrent;
extern const dvar_t *sv_debugRate;
extern const dvar_t *sv_debugReliableCmds;
extern const dvar_t *nextmap;
extern const dvar_t *com_movieIsPlaying;
extern const dvar_t *sv_wwwDownload;
extern const dvar_t *sv_wwwBaseURL;
extern const dvar_t *sv_wwwDlDisconnected;
extern const dvar_t *sv_smp;
extern const dvar_t *sv_network_fps;
extern const dvar_t *sv_assistWorkers;
extern const dvar_t *sv_clientArchive;