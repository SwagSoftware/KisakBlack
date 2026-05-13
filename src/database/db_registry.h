#pragma once
#include <win32/win_common.h>
#include <cstddef>
#include "database.h"
#include <bgame/bg_emblems.h>
#include <gfx_d3d/r_material.h>
#include <xanim/xmodel.h>

enum FF_DIR : __int32
{                                       // XREF: XZoneName/r
                                        // UI_LoadModsMap/r ...
    FFD_DEFAULT  = 0x0,
    FFD_MOD_DIR  = 0x1,
    FFD_USER_MAP = 0x2,
};

enum DBCloneMethod : __int32
{                                       // XREF: DB_DynamicCloneMenu/r
                                        // DB_CloneXAssetEntry/r ...
    DB_CLONE_NORMAL       = 0x0,
    DB_CLONE_FROM_DEFAULT = 0x1,
    DB_CLONE_SWAP         = 0x2,
};

struct RawFile // sizeof=0xC
{                                       // XREF: XAssetPoolEntry<RawFile>/r
    const char *name;
    int len;
    const char *buffer;
};

struct XZoneInfoInternal // sizeof=0x44
{                                       // XREF: .data:g_zoneInfo/r
    char name[64];
    int flags;                          // XREF: DB_LoadXZone+12D/w
};

struct XAsset // sizeof=0x8
{                                       // XREF: XAssetEntry/r
    XAssetType type;                    // XREF: DB_CreateDefaultEntry+7B/w
    XAssetHeader header;                // XREF: DB_CreateDefaultEntry+19/w
};

struct ScriptStringList // sizeof=0x8
{                                       // XREF: XAssetList/r
    int count;
    const char **strings;
};

struct XAssetList // sizeof=0x10
{                                       // XREF: .data:g_varXAssetList/r
    ScriptStringList stringList;
    int assetCount;
    XAsset *assets;
};

struct PhysConstraints;
struct PhysPreset;
struct XAnimParts;
struct DestructibleDef;
struct XModel;
struct GfxImage;
struct windowDef_t;

struct XAssetEntry // sizeof=0x10
{                                       // XREF: XAssetEntryPoolEntry/r
                                        // DB_AddXAsset/r
    XAsset asset;                       // XREF: DB_AddXAsset+9/w
                                        // DB_AddXAsset+F/w
    unsigned __int8 zoneIndex;
    bool inuse;
    unsigned __int16 nextHash;
    unsigned __int16 nextOverride;
    unsigned __int16 usageFrame;
};

struct DBReorderAssetEntry // sizeof=0x10
{                                       // XREF: $73C9AB3B505E60D629E435B147795978/r
    unsigned int sequence;
    int type;
    const char *typeString;
    const char *assetName;
};

struct XZoneInfo // sizeof=0xC
{                                       // XREF: DB_UpdateDebugZone(void)+1E/w
    const char *name;                   // XREF: DB_UpdateDebugZone(void)+15/w
    int allocFlags;                     // XREF: DB_UpdateDebugZone(void)+18/w
    int freeFlags;                      // XREF: DB_UpdateDebugZone(void)+1B/w
};

union XAssetEntryPoolEntry // sizeof=0x10
{                                       // XREF: DB_Init+AB/w
    XAssetEntry entry;
    XAssetEntryPoolEntry *next;

    XAssetEntryPoolEntry()
    {
    }
};

struct XBlock // sizeof=0x8
{                                       // XREF: XZone/r
    unsigned __int8 *data;
    unsigned int size;
};

struct XZone // sizeof=0x40
{                                       // XREF: .data:g_zones/r
    unsigned __int16 index;
    // padding byte
    // padding byte
    int flags;                          // XREF: DB_UnloadXAssetsMemoryForZone+31/r
    XBlock blocks[7];
};

struct __declspec(align(4)) XZoneName // sizeof=0x50
{                                       // XREF: .data:g_zoneNames/r
    char name[64];                      // XREF: DB_LinkXAssetEntry+236/r
                                        // DB_LinkXAssetEntry+A1E/r ...
    int flags;                          // XREF: DB_OverrideAsset+64/r
                                        // DB_OverrideAsset+82/r ...
    int fileSize;                       // XREF: DB_TryLoadXFileInternal+34D/w
                                        // DB_ReferencedFFChecksums(void)+B3/r
    FF_DIR dir;                         // XREF: DB_TryLoadXFileInternal+362/w
                                        // DB_ReferencedFFNameList(void)+A3/r ...
    bool loaded;                        // XREF: DB_IsZoneLoaded(char const *)+50/r
                                        // DB_IsZoneTypeLoaded(int)+44/r ...
    // padding byte
    // padding byte
    // padding byte
};

template <typename T>
union XAssetPoolEntry // sizeof=0x10
{                                       // ...
    XAssetPoolEntry()
    {
        next = NULL;
    }
    T entry;
    XAssetPoolEntry<T> *next;
};

template <typename T, int LEN>
struct XAssetPool
{
    XAssetPoolEntry<T> *freeHead;
    XAssetPoolEntry<T> entries[LEN];
};

struct Material;
struct SndBank;
struct SndPatch;
struct ComWorld;
struct GameWorldSp;
struct GameWorldMp;
struct MapEnts;
struct clipMap_t;

void __cdecl DB_InitSingleton(void *pool, int size);
void __cdecl Load_PhysPresetAsset(XAssetHeader *physPreset);
void __cdecl Mark_PhysPresetAsset(PhysPreset *physPreset);
void __cdecl Load_PhysConstraintsAsset(XAssetHeader *physConstraints);
void __cdecl Mark_PhysConstraintsAsset(PhysConstraints *physConstraints);
void __cdecl Load_DestructibleDefAsset(XAssetHeader *destructibleDef);
void __cdecl Mark_DestructibleDefAsset(DestructibleDef *destructibleDef);
void __cdecl Load_XAnimPartsAsset(XAssetHeader *parts);
void __cdecl Mark_XAnimPartsAsset(XAnimParts *parts);
void __cdecl Load_XModelAsset(XAssetHeader *model);
void __cdecl Mark_XModelAsset(XModel *model);
void __cdecl DB_RemoveXModel(XAssetHeader header);
void __cdecl Load_MaterialAsset(Material **material);
void __cdecl Mark_MaterialAsset(Material *material);
void __cdecl Load_MaterialTechniqueSetAsset(XAssetHeader *techniqueSet);
void __cdecl Mark_MaterialTechniqueSetAsset(MaterialTechniqueSet *techniqueSet);
void __cdecl Load_GfxImageAsset(XAssetHeader *image);
void __cdecl Mark_GfxImageAsset(GfxImage *image);
void __cdecl DB_RemoveImage(XAssetHeader header);
void __cdecl Load_SndBankAsset(XAssetHeader *sound);
void __cdecl Mark_SndBankAsset(SndBank *sound);
void __cdecl Load_SndPatchAsset(XAssetHeader *soundPatch);
void __cdecl Mark_SndPatchAsset(SndPatch *soundPatch);
void __cdecl DB_RemoveSound(XAssetHeader header);
void __cdecl DB_RemoveSoundPatch(XAssetHeader header);
void __cdecl Load_ClipMapAsset(XAssetHeader *clipMap);
void __cdecl Mark_ClipMapAsset(clipMap_t *clipMap);
void __cdecl DB_RemoveClipMap(XAssetHeader);
void __cdecl Load_ComWorldAsset(XAssetHeader *comWorld);
void __cdecl Mark_ComWorldAsset(ComWorld *comWorld);
void __cdecl DB_RemoveComWorld(XAssetHeader);
void __cdecl Load_GameWorldSpAsset(XAssetHeader *gameWorldSp);
void __cdecl Mark_GameWorldSpAsset(GameWorldSp *gameWorldSp);
void __cdecl Load_GameWorldMpAsset(XAssetHeader *gameWorldMp);
void __cdecl Mark_GameWorldMpAsset(GameWorldMp *gameWorldMp);
void __cdecl Load_MapEntsAsset(XAssetHeader *mapEnts);
void __cdecl Mark_MapEntsAsset(MapEnts *mapEnts);
void __cdecl Load_DDLAsset(XAssetHeader *ddlRoot);
void __cdecl Mark_DDLAsset(ddlRoot_t *ddlRoot);
void __cdecl Load_GfxWorldAsset(XAssetHeader *gfxWorld);
void __cdecl Mark_GfxWorldAsset(GfxWorld *gfxWorld);
void __cdecl DB_RemoveGfxWorld(XAssetHeader);
void __cdecl Load_LightDefAsset(XAssetHeader *lightDef);
void __cdecl Mark_LightDefAsset(GfxLightDef *lightDef);
void __cdecl Load_FontAsset(XAssetHeader *font);
void __cdecl Mark_FontAsset(Font_s *font);
void __cdecl Load_MenuListAsset(XAssetHeader *menuList);
void __cdecl Mark_MenuListAsset(MenuList *menuList);
void __cdecl Load_MenuAsset(XAssetHeader *menu);
void __cdecl Mark_MenuAsset(menuDef_t *menu);
void __cdecl DB_DynamicCloneMenu(const XAssetHeader from, XAssetHeader to, DBCloneMethod __formal);
void __cdecl DB_RemoveWindowFocus(windowDef_t *window);
void __cdecl Load_LocalizeEntryAsset(XAssetHeader *localize);
void __cdecl Mark_LocalizeEntryAsset(LocalizeEntry *localize);
void __cdecl Load_WeaponVariantDefAsset(XAssetHeader *weapon);
void __cdecl Mark_WeaponVariantDefAsset(WeaponVariantDef *weapon);
void __cdecl Load_SndDriverGlobalsAsset(SndDriverGlobals **sndDriverGlobals);
void __cdecl Mark_SndDriverGlobalsAsset(SndDriverGlobals *sndDriverGlobals);
void __cdecl Load_FxEffectDefAsset(XAssetHeader *fx);
void __cdecl Mark_FxEffectDefAsset(FxEffectDef *fx);
void __cdecl Load_FxEffectDefFromName(const char **name);
void __cdecl Load_FxImpactTableAsset(XAssetHeader *impactFx);
void __cdecl Mark_FxImpactTableAsset(FxImpactTable *impactFx);
void __cdecl Load_RawFileAsset(XAssetHeader *rawfile);
void __cdecl Mark_RawFileAsset(RawFile *rawfile);
void __cdecl Load_StringTableAsset(XAssetHeader *stringTable);
void __cdecl Mark_StringTableAsset(StringTable *stringTable);
void __cdecl Load_PackIndexAsset(XAssetHeader *packIndex);
void __cdecl Mark_PackIndexAsset(PackIndex *packIndex);
void __cdecl Load_XGlobalsAsset(XAssetHeader *xGlobals);
void __cdecl Mark_XGlobalsAsset(XGlobals *xGlobals);
void __cdecl Load_GlassesAsset(XAssetHeader *glasses);
void __cdecl Mark_GlassesAsset(Glasses *glasses);
void __cdecl Load_EmblemSetAsset(XAssetHeader *emblemSet);
void __cdecl Mark_EmblemSetAsset(EmblemSet *emblemSet);
XAssetHeader __cdecl DB_AllocMaterial(void *pool);
void __cdecl DB_FreeMaterial(XAssetPoolEntry<EmblemSet> **pool, XAssetHeader header);
void __cdecl DB_RemoveGfxImage(XAssetHeader header);
void __cdecl DB_FreeGfxImage(XAssetPoolEntry<EmblemSet> **pool, XAssetHeader header);
void __cdecl DB_FreeMenu(XAssetPoolEntry<EmblemSet> **pool, XAssetHeader header);
void __cdecl DB_FreeXGlobals(XAssetPoolEntry<EmblemSet> **pool, XAssetHeader header);
void __cdecl DB_MarkAssetTypeUsageDirty();
void __cdecl DB_GetAssetTypeUsageInfo(XAssetType assetType, const char **outName, int *outUsed, int *outPool);
void __cdecl DB_PrintXAssetsForType_FastFile(XAssetType type, void *inData, bool includeOverride);
void __cdecl DB_PrintXAssetsForType(
                XAssetType type,
                void (__cdecl *func)(XAssetHeader, void *),
                void *inData,
                bool includeOverride);
GfxImage *__cdecl DB_AllocTempImage();
XAssetHeader __cdecl DB_AllocXAssetHeader(XAssetType type);
void __cdecl DB_PrintAssetName(XAssetHeader header, int *data);
void __cdecl DB_FreeXAssetHeader(XAssetType type, XAssetHeader header);
unsigned int __cdecl DB_HashForName(const char *name, XAssetType type);
void __cdecl DB_LogMissingAsset(XAssetType type, const char *name);
void __cdecl DB_EnumXAssets_FastFile(
                XAssetType type,
                void (__cdecl *func)(XAssetHeader, void *),
                void *inData,
                bool includeOverride);
void __cdecl DB_LoadDelayedImages();
XAssetHeader __cdecl DB_FindXAssetDefaultHeaderInternal(XAssetType type);
const char *__cdecl DB_FindXAssetNameFromHash(XAssetType type, unsigned int hash);
XAssetHeader __cdecl DB_FindXAssetHeader(XAssetType type, char *name, bool errorIfMissing, int waitTime);
void __cdecl DB_RegisteredReorderAsset(int type, const char *assetName, XAssetEntry *assetEntry);
void __cdecl Sys_LockWrite(FastCriticalSection *critSect);
void __cdecl Sys_UnlockWrite(FastCriticalSection *critSect);
void DB_EndReorderZone();
char __cdecl DB_CompareReorderEntries(const DBReorderAssetEntry &e0, const DBReorderAssetEntry &e1);
void DB_SetReorderIncludeSequence();
char __cdecl DB_RegisterAllReorderAssetsOfType(int type, XAssetEntry *assetEntry);
void __cdecl DB_Sleep(unsigned int msec);
XAssetEntryPoolEntry *__cdecl DB_FindXAssetEntry(XAssetType type, const char *name);
XAssetEntry *__cdecl DB_CreateDefaultEntry(XAssetType type, const char *name);
XAssetEntryPoolEntry *__cdecl DB_AllocXAssetEntry(XAssetType type, unsigned __int8 zoneIndex);
void __cdecl DB_CloneXAssetInternal(const XAsset *from, XAsset *to);
void __cdecl PrintWaitedError(XAssetType type, char *name, int waitedMsec);
bool __cdecl IsConfigFile(char *name);
void __cdecl DB_Update();
void __cdecl DB_SetInitializing(bool inUse);
bool __cdecl DB_GetInitializing();
bool __cdecl DB_IsXAssetDefault(XAssetType type, const char *name);
int __cdecl DB_GetAllXAssetOfType_FastFile(XAssetType type, XAssetHeader *assets, int maxCount);
void __cdecl DB_BeginRecoverLostDevice();
void __cdecl DB_EndRecoverLostDevice();
int __cdecl DB_GetZonePriority(int zone);
XAssetHeader __cdecl DB_AddXAsset(XAssetType type, XAssetHeader header);
void DB_SyncLostDevice();
XAssetEntryPoolEntry *__cdecl DB_LinkXAssetEntry(XAssetEntry *newEntry, int allowOverride);
void __cdecl DB_FreeXAssetEntry(XAssetEntryPoolEntry *assetEntry);
void __cdecl DB_CloneXAssetEntry(const XAssetEntry *from, XAssetEntry *to, DBCloneMethod cloneMethod);
void __cdecl DB_CloneXAsset(const XAsset *from, XAsset *to, DBCloneMethod cloneMethod);
char __cdecl DB_DynamicCloneXAsset(XAssetHeader from, XAssetHeader to, XAssetType type, DBCloneMethod cloneMethod);
void __cdecl DB_SwapXAsset(XAsset *from, XAsset *to);
void __cdecl DB_DelayedCloneXAsset(XAssetEntry *newEntry);
bool __cdecl DB_OverrideAsset(unsigned int newZoneIndex, unsigned int existingZoneIndex);
void __cdecl DB_GetXAsset(XAssetType type, XAssetHeader header);
void __cdecl DB_BuildOSPath(const char *zoneName, const char *ext, unsigned int size, char *filename);
char __cdecl DB_IsUsingGermanPaths();
void __cdecl DB_BuildOSPath_Unlocalized(const char *zoneName, const char *ext, unsigned int size, char *filename);
void DB_PostLoadXZone();
void __cdecl DB_UpdateDebugZone();
void __cdecl DB_SyncXAssets();
bool __cdecl DB_IsZoneLoaded(const char *name);
char __cdecl DB_IsZoneTypeLoaded(int zoneType);
void __cdecl DB_LoadXAssets(XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
void DB_Init();
void __cdecl DB_InitPoolHeader(XAssetType type);
void __cdecl DB_ListDefaultEntries_f();
void __cdecl DB_ListAssetPool_f();
void __cdecl DB_ListAssetPool(XAssetType type);
void __cdecl DB_DumpMaterialList_f();
void __cdecl DB_LoadXZone(XZoneInfo *zoneInfo, unsigned int zoneCount);
void __cdecl DB_LoadZone_f();
void __cdecl DB_InitThread();
void __cdecl    DB_Thread(unsigned int threadContext);
void DB_TryLoadXFile();
int __cdecl DB_TryLoadXFileInternal(const char *zoneName, int zoneFlags);
void __cdecl DB_BeginReorderZone(const char *zoneName);
void __cdecl DB_AddReorderAsset(const char *typeString, const char *assetName);
char __cdecl DB_NextZoneCsvToken(const char **parse, char *token, unsigned int tokenSize, bool allowNewLine);
bool __cdecl DB_GetZoneAllocType(int zoneFlags);
void __cdecl DB_ModXFileHandle(const char *zoneName, void **zoneFile, FF_DIR *zoneDir);
void __cdecl DB_BuildOSPath_FromSource(const char *zoneName, FF_DIR source, unsigned int size, char *filename);
void __cdecl DB_BuildOSPath_ForUsermap(const char *zoneName, unsigned int size, char *filename);
void __cdecl DB_UnloadXZone(XZone *zone, bool createDefault);
void __cdecl DB_UnloadXZoneInternal(unsigned int zoneIndex, bool createDefault);
void __cdecl DB_RemoveXAsset(XAsset *asset);
void __cdecl DB_ReleaseXAssets();
void __cdecl DB_ShutdownXAssets();
void __cdecl DB_UnloadXZoneMemory(XZone *zone);
void DB_FreeDefaultEntries();
void __cdecl DB_UnloadXAssetsMemoryForZone(int zoneFreeFlags, int zoneFreeBit);
void __cdecl DB_UnloadXAssetsMemory(XZone *zone);
void __cdecl DB_ReplaceModel(const char *original, const char *replacement);
void __cdecl DB_ReplaceXAsset(XAssetType type, const char *original, const char *replacement);
void __cdecl DB_SyncExternalAssetsInternal();
void DB_SyncExternalAssets();
void DB_ArchiveAssets();
void DB_FreeUnusedResources();
void __cdecl DB_EnableInUseCache(unsigned __int16 *data);
bool DB_DisableInUseCache();
void DB_ExternalInitAssets();
void DB_StreamCompletePreload();
void DB_UnarchiveAssets();
void __cdecl DB_Cleanup();
int __cdecl DB_GetImageIndex(const GfxImage *image);
XAssetPoolEntry<GfxImage> *__cdecl DB_GetImageAtIndex(unsigned int index);
int __cdecl DB_GetMaterialIndex(const Material *material);
XAssetPoolEntry<Material> *__cdecl DB_GetMaterialAtIndex(unsigned int index);
int __cdecl DB_GetXModelIndex(const XModel *model);
XAssetPoolEntry<XModel> *__cdecl DB_GetXModelAtIndex(unsigned int index);
char *__cdecl DB_ReferencedFFChecksums();
char *__cdecl DB_ReferencedFFNameList();
int __cdecl DB_FileSize(const char *zoneName, FF_DIR source);
char __cdecl DB_FileExists(const char *zoneName, FF_DIR source);
char __cdecl DB_ModFileExists();
void __cdecl DB_AddUserMapDir(char *zoneName);
void __cdecl DB_LoadFastFilesForPC();
void __cdecl DB_LoadGraphicsAssetsForPC();


extern int skipLoadingMaterialsHack;
extern volatile unsigned int g_mainThreadBlocked;