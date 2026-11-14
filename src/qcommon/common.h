#pragma once
#include "msg_mp.h"
#include "net_chan_mp.h"

enum hitLocation_t : __int32
{                                       // XREF: RagdollBody/r
    HITLOC_NONE      = 0x0,
    HITLOC_HELMET    = 0x1,
    HITLOC_HEAD      = 0x2,
    HITLOC_NECK      = 0x3,
    HITLOC_TORSO_UPR = 0x4,
    HITLOC_TORSO_LWR = 0x5,
    HITLOC_R_ARM_UPR = 0x6,
    HITLOC_L_ARM_UPR = 0x7,
    HITLOC_R_ARM_LWR = 0x8,
    HITLOC_L_ARM_LWR = 0x9,
    HITLOC_R_HAND    = 0xA,
    HITLOC_L_HAND    = 0xB,
    HITLOC_R_LEG_UPR = 0xC,
    HITLOC_L_LEG_UPR = 0xD,
    HITLOC_R_LEG_LWR = 0xE,
    HITLOC_L_LEG_LWR = 0xF,
    HITLOC_R_FOOT    = 0x10,
    HITLOC_L_FOOT    = 0x11,
    HITLOC_GUN       = 0x12,
    HITLOC_NUM       = 0x13,
};
enum errorParm_t : __int32
{                                                                             // XREF: ?Com_Error@@YAXW4errorParm_t@@PBDZZ/r
        ERR_FATAL                        = 0x0,
        ERR_DROP                         = 0x1,
        ERR_SERVERDISCONNECT = 0x2,
        ERR_DISCONNECT             = 0x3,
        ERR_SCRIPT                     = 0x4,
        ERR_SCRIPT_DROP            = 0x5,
        ERR_LOCALIZATION         = 0x6,
};

struct SpawnVar // sizeof=0xA0C
{                                       // XREF: level_locals_t/r
                                        // cg_level_locals_t/r ...
    bool spawnVarsValid;                // XREF: CG_SpawnNode+6/r
                                        // CG_InitTriggerWait+4/r ...
    // padding byte
    // padding byte
    // padding byte
    int numSpawnVars;                   // XREF: CScr_LoadStructs(void)+1FC/r
                                        // CG_SpawnNode+7F/r ...
    char *spawnVars[64][2];             // XREF: CScr_LoadStructs(void)+20C/r
                                        // CScr_LoadStructs(void)+223/r ...
    int numSpawnVarChars;
    char spawnVarChars[2048];
};

struct __declspec(align(8)) SysInfo // sizeof=0x260
{                                                                             // XREF: .data:sys_info/r
                                                                                // ?Com_SetRecommended@@YAXHH@Z/r
        long double cpuGHz;                                 // XREF: Sys_RegisterInfoDvars+DA/r
                                                                                // Sys_Init(void)+1B7/r ...
        long double configureGHz;                     // XREF: Com_SetRecommended(int,int)+31/r
                                                                                // Com_SetRecommended(int,int)+41/w ...
        int logicalCpuCount;                                // XREF: Sys_Init(void)+13F/r
                                                                                // Sys_Init(void)+165/r ...
        int physicalCpuCount;                             // XREF: Sys_Init(void)+17B/r
                                                                                // Sys_Init(void)+1A1/r
        int sysMB;                                                    // XREF: Com_SetRecommended(int,int)+49/r
                                                                                // Com_SetRecommended(int,int)+55/w ...
        char gpuDescription[512];                     // XREF: Com_SetRecommended(int,int)+125/o
                                                                                // Sys_HasInfoChanged(void):loc_7D9644/o ...
        bool SSE;                                                     // XREF: Sys_RegisterInfoDvars+A5/r
                                                                                // Sys_Init(void)+211/r ...
        char cpuVendor[13];                                 // XREF: Sys_Init(void):loc_7DA2D7/o
                                                                                // Sys_FindInfo+90/o
        char cpuName[49];                                     // XREF: Sys_RegisterInfoDvars+FC/o
                                                                                // Sys_Init(void)+12B/o ...
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
};

struct field_t // sizeof=0x118
{                                                                             // XREF: .data:field_t g_consoleField/r
    int cursor;                                                 // XREF: Con_Echo_f+50/w
                                                                            // Con_CommitToAutoComplete(void)+C5/w ...
    int scroll;
    int drawWidth;                                            // XREF: Con_CommitToAutoComplete(void)+FF/w
    int widthInPixels;                                    // XREF: Con_ToggleConsole(void)+20/w
                                                                            // Con_Init(void)+67/w ...
    float charHeight;                                     // XREF: Con_ToggleConsole(void)+2D/w
                                                                            // Con_Init(void)+74/w ...
    int fixedSize;                                            // XREF: Con_ToggleConsole(void)+35/w
                                                                            // Con_Init(void)+7C/w ...
    char buffer[256];                                     // XREF: Con_Echo_f+15/o
};

struct XAnim_s;
struct clientState_s;
struct XAnimTree_s;

void *__cdecl Com_LiveAllocate(unsigned int size);
void __cdecl Com_LiveDeallocate(void *data);
unsigned __int8 *__cdecl Com_LiveRealloc(unsigned __int8 *ptr, unsigned int size);
unsigned __int8 *__cdecl Com_LiveCalloc(unsigned int num, unsigned int size);
bool __cdecl Com_IsRunningMenuLevel(const char *name);
bool __cdecl Com_IsMenuLevel(const char *name);
void __cdecl Com_BeginRedirect(char *buffer, unsigned int buffersize, void (__cdecl *flush)(char *));
void __cdecl Com_EndRedirect();
void __cdecl Com_PrintMessage(int channel, char *msg, int error);
void __cdecl Com_LogPrintMessage(int channel, char *msg);
void Com_OpenLogFile();
void Com_Printf(int channel, const char *fmt, ...);
void Com_DPrintf(int channel, const char *fmt, ...);
void Com_PrintError(int channel, const char *fmt, ...);
void Com_PrintWarning(int channel, const char *fmt, ...);
void __cdecl Com_Shutdown(const char *finalmsg);
void __cdecl Com_ShutdownInternal(const char *finalmsg);
void __cdecl Com_InitDynamicRender();
void __cdecl Com_InitDynamicMemorySystems();
void __cdecl Com_ShutdownDynamicMemorySystems();
double __cdecl Com_GetTimeScale();
void __cdecl Com_SetTimeScale(float timescale);
void __cdecl Com_SetLocalizedErrorMessage(const char *localizedErrorMessage, const char *titleToken);
void __cdecl Com_SetErrorMessage(char *errorMessage);
char __cdecl Com_ErrorIsNotice(const char *errorMessage);
void __cdecl Com_PrintStackTrace();
void __cdecl    Com_ErrorAbort();
void Com_Error(errorParm_t code, const char *fmt, ...);
void __cdecl Com_CheckError();
void __cdecl    Com_Quit_f();
void Com_ClearTempMemory();
int __cdecl Com_SafeMode();
void __cdecl Com_ForceSafeMode();
void __cdecl Com_StartupVariable(const char *match);
bool __cdecl Com_StartupProcessSetCommand(int lineIndex, const char *match);
void __cdecl Info_Print(const char *s);
unsigned int *__cdecl Com_AllocEvent(int size);
void __cdecl Com_FreeEvent(char *ptr);
void __cdecl Com_ServerPacketEvent();
void __cdecl Com_EventLoop();
void Com_ClientPacketEvent();
void __cdecl Com_PacketEventLoop(int localClientNum, msg_t *netmsg);
void __cdecl Com_DispatchClientPacketEvent(netadr_t adr, msg_t *netmsg);
void __cdecl Com_ReadCDKey();
int Com_ClearCDKey();
void __cdecl CL_ConvertRegKeytoOnlineKey(char *key, unsigned int size);
void __cdecl Com_SetRecommended(int localClientNum, int restart);
int __cdecl Com_ConfigureChecksum(const char *csv, int filesize);
char __cdecl Com_SetRecommendedCpu(int localClientNum, const SysInfo *info, char **text);
int __cdecl Com_GetConfigureDvarNames(const char **text, char (*dvarNames)[32]);
void __cdecl Com_GetConfigureDvarValues(int dvarCount, const char **text, char (*dvarValues)[32]);
void __cdecl Com_SetConfigureDvars(int dvarCount, const char (*dvarNames)[32], const char (*dvarValues)[32]);
char __cdecl Com_SetRecommendedGpu(const SysInfo *info, char **text);
bool __cdecl Com_DoesGpuStringMatch(const char *find, const char *ref);
int __cdecl Com_GpuStringCompare(const char *wild, const char *s);
void __cdecl Com_CheckSetRecommended(int localClientNum);
bool __cdecl Com_HasConfigureFileChanged();
void __cdecl Com_SetScriptSettings();
void __cdecl Com_RunAutoExec(int localClientNum, int controllerIndex);
void __cdecl Com_ExecStartupConfigs(int localClientNum, const char *configFile);
void __cdecl Com_InitUI3DCallback();
void __cdecl Com_InitUIAndCommonXAssets();
unsigned int __cdecl Com_CalculateStreamBuffer();
void __cdecl Com_Init(char *commandLine);
void Com_ErrorCleanup();
void Com_AddStartupCommands();
void __cdecl Com_Init_Try_Block_Function(char *commandLine);
void __cdecl Com_ParseCommandLine(char *commandLine);
void __cdecl Com_Error_f();
void __cdecl Com_Freeze_f();
void __cdecl Com_Crash_f();
void __cdecl Com_Assert_f();
void COM_PlayIntroMovies();
void Com_InitDvars();
void __cdecl Com_StartupConfigs(int localClientNum);
void __cdecl Com_InitCodeXAssets();
void __cdecl Com_WriteDefaultsToFile(char *filename);
void __cdecl Com_WriteConfig_f();
void __cdecl Com_WriteConfigToFile(int localClientNum, char *filename);
void __cdecl Com_WriteKeyConfig_f();
void __cdecl Com_WriteKeyConfigToFile(int localClientNum, char *filename);
void __cdecl Com_SaveKeys_f();
void __cdecl Com_RestoreKeys_f();
void __cdecl Com_WriteDefaults_f();
double __cdecl Com_GetTimescaleForSnd();
void Com_LoadUiFastFile();
void __cdecl Com_LoadMapLoadingScreenFastFile(const char *mapName);
void __cdecl Com_UnloadLevelFastFiles();
void __cdecl Com_LoadLevelFastFiles(char *mapName);
char *__cdecl Com_GetLevelSharedFastFile(char *mapName);
void Com_LoadCommonFastFile();
void __cdecl Com_LoadFrontEnd();
void __cdecl Com_UnloadFrontEnd();
void __cdecl Com_AssetLoadUI();
void __cdecl Com_ResetFrametime();
void __cdecl Com_CheckSyncFrame();
void __cdecl Com_Frame();
unsigned int Com_Frame_Try_Block_Function();
void __cdecl Com_WriteConfiguration(int localClientNum);
int __cdecl Com_ModifyMsec(int msec);
void Com_Statmon();
void __cdecl Com_AdjustMaxFPS(int *maxFPS);
char Com_UpdateMenu();
void Com_StartHunkUsers();
//void __thiscall cdl_proftimer::reset(cdl_proftimer *this);
void __cdecl Com_CloseLogfiles();
bool __cdecl Com_LogFileOpen();
void __cdecl Com_Close();
void __cdecl Field_Clear(field_t *edit);
void __cdecl Com_Restart();
XAnimTree_s *__cdecl Com_XAnimCreateSmallTree(XAnim_s *anims);
void __cdecl Com_XAnimFreeSmallTree(XAnimTree_s *animtree);
void __cdecl Com_SetWeaponInfoMemory(int source);
void __cdecl Com_FreeWeaponInfoMemory(int source);
int __cdecl Com_AddToString(const char *add, char *msg, int len, int maxlen, int mayAddQuotes);
char __cdecl Com_GetDecimalDelimiter();
void __cdecl Com_LocalizedFloatToString(float f, char *buffer, unsigned int maxlen, unsigned int numDecimalPlaces);
void __cdecl Com_SyncThreads();
const char *__cdecl Com_DisplayName(const char *name, const char *clanAbbrev, int type);
const char *__cdecl CS_DisplayName(const clientState_s *cs, int type);
int __cdecl Com_GetPrivateClients();
void Com_Printf_NoFilter(const char *fmt, ...);

#include <xmmintrin.h>  // SSE
#include <emmintrin.h>  // SSE2

// (https://github.com/SwagSoftware/KisakCOD/issues/52)
// The default x87 rounding mode for floating point to integer is Round to nearest Even (Banker's rounding)
// However if you use a modern compiler and regular casts, it will round down.
// This is because it seems to use the `cvttss2si` instruction (SSE) which is always truncated
// This probably was not the case back then, but should be emulated because the float rounding *highly* effects the movement amongst other code
// Anywhere you see "(float)(int)" should be investigated for this
// You are basically looking for `fistp`, which unloads the FPU into an integer (and does rounding in the process)
// Also any call to "ftol" or "ftol_sse" will be doing this ^^ and returning the result in eax
inline float SnapFloat(float x)
{
#if defined(KISAK_PURE) && defined(_WIN32)
    int i;
    __asm fld x;
    __asm fistp i;

    return static_cast<float>(i);
#endif

    // We can use `cvtss2si` instead (1 t). By default this is set to Nearest even and matches the other methods in my testbed
    // This is a bit more portable since other platforms dont have __asm or even 32bit support anymore
    int i;
    i = _mm_cvtss_si32(_mm_set_ss(x));
    return  static_cast<float>(i);
}

// A simpler version of the above that can be used to save a cast afterwards
// (basically ftol())
inline int SnapFloatToInt(float x)
{
#if defined(KISAK_PURE) && defined(_WIN32)
    int i;
    __asm fld x;
    __asm fistp i;

    return i;
#endif

    int i;
    i = _mm_cvtss_si32(_mm_set_ss(x));
    return i;
}

// Double precision version of the above, for the `fld; fadd qword ptr [2^-30]; fistp` idiom.
// The decompiler shows it as `(int)(x + 9.313225746154785e-10)`, but a C cast truncates toward zero,
// so an exact negative integer such as -72.0 becomes -71.999999999 and then -71 instead of -72.
inline int SnapDoubleToInt(double x)
{
    return _mm_cvtsd_si32(_mm_set_sd(x));
}


extern const dvar_t *collectors;
extern const dvar_t *primaryWeaponOffset;
extern const dvar_t *scr_xpcollectorsscale;
extern const dvar_t *scr_xpscale;
extern const dvar_t *scr_xpzmscale;
extern const dvar_t *scr_codpointsxpscale;
extern const dvar_t *scr_codpointsmatchscale;
extern const dvar_t *scr_codpointsperchallenge;
extern const dvar_t *scr_rankXpCap;
extern const dvar_t *scr_codPointsCap;
extern const dvar_t *version;
extern const dvar_t *shortversion;
extern const dvar_s *com_recommendedSet;

extern const dvar_s *useFastFile;
extern const dvar_s *sys_smp_allowed;
extern const dvar_t *com_maxclients;
extern const dvar_t *com_freemoveScale;
extern const dvar_t *disconnected_ctrls;
extern const dvar_t *com_first_time;
extern const dvar_t *com_first_time_pc;
extern const dvar_t *dedicated;
extern const dvar_t *com_maxfps;
extern const dvar_t *arcademode;
extern const dvar_t *zombiemode;
extern const dvar_t *legacy_zombiemode;
extern const dvar_t *zombieStopSplitScreen;
extern const dvar_t *zombietron;
extern const dvar_t *zombietron_discovered;
extern const dvar_t *zombiefive_discovered;
extern const dvar_t *zombiefive_norandomchar;
extern const dvar_t *blackopsmode;
extern const dvar_t *spmode;
extern const dvar_t *onlinegame;
extern const dvar_t *xblive_rankedmatch;
extern const dvar_t *xblive_privatematch;
extern const dvar_t *useFastFile;
extern const dvar_t *sys_lockThreads;
extern const dvar_t *sys_smp_allowed;
extern const dvar_t *com_developer;
extern const dvar_t *com_developer_script;
extern const dvar_t *com_script_debugger_smoke_test;
extern const dvar_t *com_logfile;
extern const dvar_t *com_statmon;
extern const dvar_t *com_timescale;
extern const dvar_t *dev_timescale;
extern const dvar_t *com_fixedtime;
extern const dvar_t *com_maxFrameTime;
extern const dvar_t *long_blocking_call;
extern const dvar_t *sv_network_warning;
extern const dvar_t *cl_network_warning;
extern const dvar_t *sv_paused;
extern const dvar_t *cl_paused;
extern const dvar_t *cl_paused_simple;
extern const dvar_t *com_sv_running;
extern const dvar_t *com_show_tty_timestamps;
extern const dvar_t *com_voip_resume_time;
extern const dvar_t *com_voip_bandwidth_restricted;
extern const dvar_t *com_voip_disable_threshold;
extern const dvar_t *com_filter_output;
extern const dvar_t *com_introPlayed;
extern const dvar_t *com_startupIntroPlayed;
extern const dvar_t *com_desiredMenu;
extern const dvar_t *com_skipMovies;
extern const dvar_t *com_animCheck;
extern const dvar_t *com_hiDef;
extern const dvar_t *com_wideScreen;
extern const dvar_t *doublesided_raycasts;
extern const dvar_t *log_append;
extern const dvar_t *com_waitForStreamer;
extern const dvar_t *dec20_Enabled;
extern const dvar_t *band_demosystem;
extern const dvar_t *band_2players;
extern const dvar_t *band_4players;
extern const dvar_t *band_6players;
extern const dvar_t *band_8players;
extern const dvar_t *band_12players;
extern const dvar_t *band_18players;
extern const dvar_t *band_lotsplayers;
extern const dvar_t *band_dedicated;

extern int com_frameNumber;
extern int com_errorEntered;
extern int com_expectedHunkUsage;
extern float com_timescaleValue;
extern int com_frameTime;
extern int com_inServerFrame;
extern int com_fixedConsolePosition;
extern unsigned int com_errorPrintsCount;