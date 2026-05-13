#include "db_registry.h"
#include <clientscript/cscr_parsetree.h>
#include <gfx_d3d/rb_resource.h>
#include <qcommon/cm_load.h>
#include <gfx_d3d/r_stream.h>
#include "db_assetnames.h"
#include <universal/com_memory.h>
#include <universal/com_files.h>
#include <qcommon/threads.h>
#include <qcommon/com_profilemapload.h>
#include <win32/win_shared.h>
#include "db_file_load.h"
#include <gfx_d3d/r_singlethreaded_device_pc.h>
#include <win32/win_net.h>
#include <algorithm>
#include <win32/win_local.h>
#include <win32/win_main.h>
#include <clientscript/cscr_stringlist.h>
#include <win32/win_localize.h>
#include <client/splitscreen.h>
#include <live/live_win_common.h>
#include <universal/physicalmemory.h>
#include "db_memory.h"
#include <gfx_d3d/rb_shade.h>
#include <gfx_d3d/r_staticmodelcache.h>
#include <qcommon/dobj_management.h>
#include <EffectsCore/fx_update.h>
#include <gfx_d3d/r_extracam.h>
#include <sound/snd_bank.h>
#include <physics/physconstraints_load_obj.h>
#include "db_load.h"
#include <qcommon/cm_load_obj.h>
#include <stringed/stringed_hooks.h>
#include <glass/glass.h>
#include <cgame/cg_effects_load_obj.h>
#include <gfx_d3d/r_image.h>
#include <game/g_bsp.h>
#include <qcommon/com_bsp.h>
#include <client/cl_keys.h>
#include <win32/win_input.h>
#include <client_mp/cl_scrn_mp.h>


#define POOLSIZE_XMODELPIECES      64
#define POOLSIZE_PHYSPRESET        64
#define POOLSIZE_PHYSCONSTRAINTS   64
#define POOLSIZE_DESTRUCTIBLEDEF   64
#define POOLSIZE_XANIM             5100
#define POOLSIZE_XMODEL            1000
#define POOLSIZE_MATERIAL          4096
#define POOLSIZE_TECHSET           2048
//#define POOLSIZE_IMAGE             4224
#define POOLSIZE_IMAGE             4608
#define POOLSIZE_SOUND             32
#define POOLSIZE_SOUND_PATCH       16
#define POOLSIZE_COL_MAP_SP        1
#define POOLSIZE_COL_MAP_MP        1
#define POOLSIZE_COM_MAP           1
#define POOLSIZE_GAME_MAP_SP       1
#define POOLSIZE_GAME_MAP_MP       1
#define POOLSIZE_MAP_ENTS          2
#define POOLSIZE_GFX_MAP           1
#define POOLSIZE_LIGHTDEF          32
#define POOLSIZE_UI_MAP            0
#define POOLSIZE_FONT              16
//#define POOLSIZE_MENUFILE          128
#define POOLSIZE_MENUFILE          164
//#define POOLSIZE_MENU              850
#define POOLSIZE_MENU              1024
#define POOLSIZE_LOCALIZE          10240
#define POOLSIZE_WEAPON            2048
#define POOLSIZE_WEAPONDEF         0
#define POOLSIZE_WEAPONVARIANT     0
#define POOLSIZE_SNDDRIVERGLOBALS  1
//#define POOLSIZE_FX                450
#define POOLSIZE_FX                500
#define POOLSIZE_IMPACTFX          4
#define POOLSIZE_AITYPE            0
#define POOLSIZE_MPTYPE            0
#define POOLSIZE_MPBODY            0
#define POOLSIZE_MPHEAD            0
#define POOLSIZE_CHARACTER         0
#define POOLSIZE_XMODELALIAS       0
#define POOLSIZE_RAWFILE           1024
//#define POOLSIZE_STRINGTABLE       256
#define POOLSIZE_STRINGTABLE       576
#define POOLSIZE_PACKINDEX         16
#define POOLSIZE_XGLOBALS          1
#define POOLSIZE_DDL               24
#define POOLSIZE_GLASSES           1
//#define POOLSIZE_EMBLEMSET         2
#define POOLSIZE_EMBLEMSET         4


static const int g_poolSize[43] =
{
    POOLSIZE_XMODELPIECES,
    POOLSIZE_PHYSPRESET,
    POOLSIZE_PHYSCONSTRAINTS,
    POOLSIZE_DESTRUCTIBLEDEF,
    POOLSIZE_XANIM,
    POOLSIZE_XMODEL,
    POOLSIZE_MATERIAL,
    POOLSIZE_TECHSET,
    POOLSIZE_IMAGE,
    POOLSIZE_SOUND,
    POOLSIZE_SOUND_PATCH,
    POOLSIZE_COL_MAP_SP,
    POOLSIZE_COL_MAP_MP,
    POOLSIZE_COM_MAP,
    POOLSIZE_GAME_MAP_SP,
    POOLSIZE_GAME_MAP_MP,
    POOLSIZE_MAP_ENTS,
    POOLSIZE_GFX_MAP,
    POOLSIZE_LIGHTDEF,
    POOLSIZE_UI_MAP,
    POOLSIZE_FONT,
    POOLSIZE_MENUFILE,
    POOLSIZE_MENU,
    POOLSIZE_LOCALIZE,
    POOLSIZE_WEAPON,
    POOLSIZE_WEAPONDEF,
    POOLSIZE_WEAPONVARIANT,
    POOLSIZE_SNDDRIVERGLOBALS,
    POOLSIZE_FX,
    POOLSIZE_IMPACTFX,
    POOLSIZE_AITYPE,
    POOLSIZE_MPTYPE,
    POOLSIZE_MPBODY,
    POOLSIZE_MPHEAD,
    POOLSIZE_CHARACTER,
    POOLSIZE_XMODELALIAS,
    POOLSIZE_RAWFILE,
    POOLSIZE_STRINGTABLE,
    POOLSIZE_PACKINDEX,
    POOLSIZE_XGLOBALS,
    POOLSIZE_DDL,
    POOLSIZE_GLASSES,
    POOLSIZE_EMBLEMSET,
};

void __cdecl DB_FreeXAssetHeader_EmblemSet_(XAssetPoolEntry<EmblemSet> **pool, XAssetHeader header);

template <typename T>
XAssetHeader DB_AllocXAsset(void *pool)
{
    XAssetHeader header;
    XAssetPool<T, 1> *castPool = (XAssetPool<T, 1> *) pool;
    XAssetPoolEntry<T> *freeHead = castPool->freeHead;

    if (freeHead)
    {
        header.data = &freeHead->entry;
        castPool->freeHead = freeHead->next;
    }
    else
    {
        header.data = NULL;
    }

    return header;
}

XAssetHeader XASSET_SINGLETON(void *pool)
{
    return (XAssetHeader)pool;
}

XAssetHeader(__cdecl *DB_AllocXAssetHeaderHandler[43])(void *) =
{
  DB_AllocXAsset<XModelPieces>,
  DB_AllocXAsset<PhysPreset>,
  DB_AllocXAsset<PhysConstraints>,
  DB_AllocXAsset<DestructibleDef>,
  DB_AllocXAsset<XAnimParts>,
  DB_AllocXAsset<XModel>,
  DB_AllocMaterial,
  DB_AllocXAsset<MaterialTechniqueSet>,
  DB_AllocXAsset<GfxImage>,
  DB_AllocXAsset<SndBank>,
  DB_AllocXAsset<snd_alias_list_t>, //SOUND_PATCH
  XASSET_SINGLETON, // COL_MAP_SP
  XASSET_SINGLETON, // COL_MAP_MP
  XASSET_SINGLETON, // COM_MAP
  XASSET_SINGLETON, // GAME_MAP_SP
  XASSET_SINGLETON, // GAME_MAP_MP
  DB_AllocXAsset<MapEnts>,
  XASSET_SINGLETON, // GFX_MAP
  DB_AllocXAsset<GfxLightDef>,
  NULL,             // UI_MAP
  DB_AllocXAsset<Font_s>,
  DB_AllocXAsset<MenuList>,
  DB_AllocXAsset<menuDef_t>,
  DB_AllocXAsset<LocalizeEntry>,
  DB_AllocXAsset<WeaponVariantDef>,
  NULL,             // WEAPONDEF
  NULL,             // WEAPONVARIANT
  DB_AllocXAsset<SndDriverGlobals>, // should be a singleton?
  DB_AllocXAsset<FxEffectDef>,
  DB_AllocXAsset<FxImpactTable>,
  NULL,             // AITYPE
  NULL,             // MPTYPE
  NULL,             // MPBODY
  NULL,             // MPHEAD
  NULL,             // CHARACTER
  NULL,             // XMODELALIAS
  DB_AllocXAsset<RawFile>,
  DB_AllocXAsset<StringTable>,
  DB_AllocXAsset<PackIndex>,
  DB_AllocXAsset<XGlobals>,
  DB_AllocXAsset<ddlDef_t>,
  DB_AllocXAsset<Glasses>,
  DB_AllocXAsset<EmblemSet>
};


XAssetPool<XModelPieces, POOLSIZE_XMODELPIECES>         g_XModelPiecesPool;
XAssetPool<PhysPreset, POOLSIZE_PHYSPRESET>             g_PhysPresetPool;
XAssetPool<PhysConstraints, POOLSIZE_PHYSCONSTRAINTS>   g_PhysConstraintsPool;
XAssetPool<DestructibleDef, POOLSIZE_DESTRUCTIBLEDEF>   g_DestructibleDefPool;
XAssetPool<XAnimParts, POOLSIZE_XANIM>                  g_XAnimPartsPool;
XAssetPool<XModel, POOLSIZE_XMODEL>                     g_XModelPool;
XAssetPool<Material, POOLSIZE_MATERIAL>                 g_MaterialPool;
XAssetPool<MaterialTechniqueSet, POOLSIZE_TECHSET>      g_MaterialTechniqueSetPool;
XAssetPool<GfxImage, POOLSIZE_IMAGE>                    g_GfxImagePool;
XAssetPool<SndBank, POOLSIZE_SOUND>                     g_SoundPool;
XAssetPool<snd_alias_list_t, POOLSIZE_SOUND_PATCH>      g_SoundPatchPool;
XAssetPool<MapEnts, POOLSIZE_MAP_ENTS>                  g_MapEntsPool;
XAssetPool<GfxLightDef, POOLSIZE_LIGHTDEF>              g_GfxLightDefPool;
XAssetPool<Font_s, POOLSIZE_FONT>                       g_FontPool;
XAssetPool<MenuList, POOLSIZE_MENUFILE>                 g_MenuListPool;
XAssetPool<menuDef_t, POOLSIZE_MENU>                    g_MenuPool;
XAssetPool<LocalizeEntry, POOLSIZE_LOCALIZE>            g_LocalizeEntryPool;
XAssetPool<WeaponVariantDef, POOLSIZE_WEAPON>           g_WeaponVariantDefPool;
XAssetPool<SndDriverGlobals, POOLSIZE_SNDDRIVERGLOBALS> g_SndDriverGlobalsPool;
XAssetPool<FxEffectDef, POOLSIZE_FX>                    g_FxEffectDefPool;
XAssetPool<FxImpactTable, POOLSIZE_IMPACTFX>            g_FxImpactTablePool;
XAssetPool<RawFile, POOLSIZE_RAWFILE>                   g_RawFilePool;
XAssetPool<StringTable, POOLSIZE_STRINGTABLE>           g_StringTablePool;
XAssetPool<PackIndex, POOLSIZE_PACKINDEX>               g_PackIndexPool;
XAssetPool<XGlobals, POOLSIZE_XGLOBALS>                 g_XGlobalsPool;
XAssetPool<ddlDef_t, POOLSIZE_DDL>                      g_DDLPool;
XAssetPool<Glasses, POOLSIZE_GLASSES>                   g_GlassesPool;
XAssetPool<EmblemSet, POOLSIZE_EMBLEMSET>               g_EmblemSetPool;

void *DB_XAssetPool[43] =
{
  &g_XModelPiecesPool,
  &g_PhysPresetPool,
  &g_PhysConstraintsPool,
  &g_DestructibleDefPool,
  &g_XAnimPartsPool,
  &g_XModelPool,
  &g_MaterialPool,
  &g_MaterialTechniqueSetPool,
  &g_GfxImagePool,
  &g_SoundPool,
  &g_SoundPatchPool,
  &cm,
  &cm,
  &comWorld,
  NULL,
  &gameWorldMp,
  &g_MapEntsPool,
  &s_world,
  &g_GfxLightDefPool,
  NULL,
  &g_FontPool,
  &g_MenuListPool,
  &g_MenuPool,
  &g_LocalizeEntryPool,
  &g_WeaponVariantDefPool,
  NULL,
  NULL,
  &g_SndDriverGlobalsPool,
  &g_FxEffectDefPool,
  &g_FxImpactTablePool,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  &g_RawFilePool,
  &g_StringTablePool,
  &g_PackIndexPool,
  &g_XGlobalsPool,
  &g_DDLPool,
  &g_GlassesPool,
  &g_EmblemSetPool
};


static void FREE_NULLSUB(void *, XAssetHeader)
{

}

//template <typename T>
//void __cdecl DB_FreeXAssetHeader(XAssetPoolEntry<T> **pool, XAssetHeader header)
//{
//    XAssetPoolEntry<T> *oldFreeHead; // [esp+8h] [ebp-4h]
//
//    oldFreeHead = *pool;
//    *pool = (XAssetPoolEntry<T> *)header.xmodelPieces;
//    header.xmodelPieces->name = (const char *)oldFreeHead;
//}

// casting all cuz it's easier
void(__cdecl *DB_FreeXAssetHeaderHandler[43])(void *, XAssetHeader) =
{
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))DB_FreeMaterial,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))DB_FreeGfxImage,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))FREE_NULLSUB,
  (void(*)(void*, XAssetHeader))FREE_NULLSUB,
  (void(*)(void*, XAssetHeader))FREE_NULLSUB,
  (void(*)(void*, XAssetHeader))FREE_NULLSUB,
  (void(*)(void*, XAssetHeader))FREE_NULLSUB,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))FREE_NULLSUB,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))NULL,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))DB_FreeMenu,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))NULL,
  (void(*)(void*, XAssetHeader))NULL,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))NULL,
  (void(*)(void*, XAssetHeader))NULL,
  (void(*)(void*, XAssetHeader))NULL,
  (void(*)(void*, XAssetHeader))NULL,
  (void(*)(void*, XAssetHeader))NULL,
  (void(*)(void*, XAssetHeader))NULL,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))DB_FreeXGlobals,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_,
  (void(*)(void*, XAssetHeader))DB_FreeXAssetHeader_EmblemSet_
};

const char *g_defaultAssetName[43] =
{
  "",
  "default",
  "default",
  "",
  "void",
  "void",
  "$default",
  "default",
  "$white",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "light_dynamic",
  "",
  "fonts/consolefont",
  "ui/default.menu",
  "default_menu",
  "CGAME_UNKNOWN",
  "defaultweapon_mp",
  "",
  "",
  "",
  "misc/missing_fx",
  "default",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "mp/defaultStringTable.csv",
  "",
  "",
  "",
  "",
  ""
};

static void CLONE_NULLSUB(const XAssetHeader, XAssetHeader, DBCloneMethod)
{

}

void(__cdecl *DB_DynamicCloneXAssetHandler[43])(const XAssetHeader, XAssetHeader, DBCloneMethod) =
{
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  CLONE_NULLSUB,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  DB_DynamicCloneMenu,
  NULL,
  CLONE_NULLSUB,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};



struct// $73C9AB3B505E60D629E435B147795978 // sizeof=0x8011C
{                                       // XREF: .data:s_dbReorder/r
    FastCriticalSection critSect;
    char zoneName[256];                 // XREF: DB_EndReorderZone+28/o
                                        // DB_EndReorderZone+21B/o ...
    bool alreadyFinished;               // XREF: DB_RegisteredReorderAsset+17E/r
                                        // DB_EndReorderZone:loc_58EB71/w ...
    bool loadedSound;                   // XREF: DB_RegisteredReorderAsset+4C/r
                                        // DB_RegisteredReorderAsset+6C/w ...
    bool loadedLocalization;            // XREF: DB_RegisteredReorderAsset+1A/r
                                        // DB_RegisteredReorderAsset+3A/w ...
    // padding byte
    DBReorderAssetEntry *lastEntry;     // XREF: DB_RegisteredReorderAsset:loc_58E988/r
                                        // DB_RegisteredReorderAsset+81/r ...
    unsigned int sequence;              // XREF: DB_RegisteredReorderAsset+12D/r
                                        // DB_RegisteredReorderAsset+162/r ...
    unsigned int sequenceForIncludes;   // XREF: DB_RegisteredReorderAsset+16B/w
                                        // DB_SetReorderIncludeSequence+58/r ...
    unsigned int entryCount;            // XREF: DB_RegisteredReorderAsset+6/r
                                        // DB_RegisteredReorderAsset+D8/r ...
    DBReorderAssetEntry entries[32768]; // XREF: DB_RegisteredReorderAsset+EA/o
                                        // DB_EndReorderZone+CD/o ...
} s_dbReorder;

unsigned __int8 g_fileBuf[524288];

unsigned __int16 db_hashTable[0x8000/*32768*/];
XAssetEntryPoolEntry g_assetEntryPool[32768];
XZoneName g_zoneNames[33];

XAssetEntry *g_copyInfo[3072];
unsigned int g_copyInfoCount;

unsigned int g_zoneAllocType;
bool g_zoneInited;
bool g_mayRecoverLostAssets;
int g_defaultAssetCount;
int skipLoadingMaterialsHack;
int g_missingAssetFile;
int com_missingAssetOpenFailed;
XAssetEntryPoolEntry *g_freeAssetEntryHead;
int XAssetEntryHighWaterMark;
int XAssetEntryHighCount;

char g_zoneNameList[2080];
volatile bool g_loadingZone;
bool g_isRecoveringLostDevice;
bool g_initializing;
bool s_usageUpToDate[43];
int s_usageCounts[43];
unsigned int g_zoneIndex;
int g_sync;
bool s_inuseCacheEnabled;
unsigned __int16 *s_inuseCache;
unsigned int s_inuseCacheHits;
unsigned int s_inuseCacheMisses;
volatile unsigned int g_zoneInfoCount;
bool g_archiveBuf;
XZoneInfoInternal g_zoneInfo[32];

volatile unsigned int g_mainThreadBlocked;
char g_debugZoneName[64];

int g_zoneCount;
XZone g_zones[32];

const dvar_t *db_xassetdebug;
const dvar_t *db_xassetdebugtype;

void __cdecl DB_InitSingleton(void *pool, int size)
{
    if ( size != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    658,
                    0,
                    "%s\n\t(size) = %i",
                    "(size == 1)",
                    size) )
    {
        __debugbreak();
    }
}

void __cdecl Load_PhysPresetAsset(XAssetHeader *physPreset)
{
    physPreset->xmodelPieces = DB_AddXAsset(ASSET_TYPE_PHYSPRESET, (XAssetHeader)physPreset->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_PhysPresetAsset(PhysPreset *physPreset)
{
    DB_GetXAsset(ASSET_TYPE_PHYSPRESET, (XAssetHeader)physPreset);
}

void __cdecl Load_PhysConstraintsAsset(XAssetHeader *physConstraints)
{
    physConstraints->xmodelPieces = DB_AddXAsset(ASSET_TYPE_PHYSCONSTRAINTS, (XAssetHeader)physConstraints->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_PhysConstraintsAsset(PhysConstraints *physConstraints)
{
    DB_GetXAsset(ASSET_TYPE_PHYSCONSTRAINTS, (XAssetHeader)physConstraints);
}

void __cdecl Load_DestructibleDefAsset(XAssetHeader *destructibleDef)
{
    destructibleDef->xmodelPieces = DB_AddXAsset(ASSET_TYPE_DESTRUCTIBLEDEF, (XAssetHeader)destructibleDef->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_DestructibleDefAsset(DestructibleDef *destructibleDef)
{
    DB_GetXAsset(ASSET_TYPE_DESTRUCTIBLEDEF, (XAssetHeader)destructibleDef);
}

void __cdecl Load_XAnimPartsAsset(XAssetHeader *parts)
{
    parts->xmodelPieces = DB_AddXAsset(ASSET_TYPE_XANIMPARTS, (XAssetHeader)parts->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_XAnimPartsAsset(XAnimParts *parts)
{
    DB_GetXAsset(ASSET_TYPE_XANIMPARTS, (XAssetHeader)parts);
}

void __cdecl Load_XModelAsset(XAssetHeader *model)
{
    model->xmodelPieces = DB_AddXAsset(ASSET_TYPE_XMODEL, (XAssetHeader)model->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_XModelAsset(XModel *model)
{
    DB_GetXAsset(ASSET_TYPE_XMODEL, (XAssetHeader)model);
}

void __cdecl DB_RemoveXModel(XAssetHeader header)
{
    XModelReleaseResources(header.model);
}

void __cdecl Load_MaterialAsset(Material **material)
{
    XAssetHeader asset;
    asset.material = *material;
    *material = DB_AddXAsset(ASSET_TYPE_MATERIAL, asset).material;
}

void __cdecl Mark_MaterialAsset(Material *material)
{
    DB_GetXAsset(ASSET_TYPE_MATERIAL, (XAssetHeader)material);
}

void __cdecl Load_MaterialTechniqueSetAsset(XAssetHeader *techniqueSet)
{
    techniqueSet->xmodelPieces = DB_AddXAsset(ASSET_TYPE_TECHNIQUE_SET, (XAssetHeader)techniqueSet->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_MaterialTechniqueSetAsset(MaterialTechniqueSet *techniqueSet)
{
    DB_GetXAsset(ASSET_TYPE_TECHNIQUE_SET, (XAssetHeader)techniqueSet);
}

void __cdecl Load_GfxImageAsset(XAssetHeader *image)
{
    RB_Resource_Flush();
    image->xmodelPieces = DB_AddXAsset(ASSET_TYPE_IMAGE, (XAssetHeader)image->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_GfxImageAsset(GfxImage *image)
{
    DB_GetXAsset(ASSET_TYPE_IMAGE, (XAssetHeader)image);
}

void __cdecl DB_RemoveImage(XAssetHeader header)
{
    DB_RemoveGfxImage(header);
    Image_Release(header.image);
}

void __cdecl Load_SndBankAsset(XAssetHeader *sound)
{
    sound->xmodelPieces = DB_AddXAsset(ASSET_TYPE_SOUND, (XAssetHeader)sound->xmodelPieces).xmodelPieces;
    SND_AddBank(sound->sound);
}

void __cdecl Mark_SndBankAsset(SndBank *sound)
{
    DB_GetXAsset(ASSET_TYPE_SOUND, (XAssetHeader)sound);
}

void __cdecl Load_SndPatchAsset(XAssetHeader *soundPatch)
{
    soundPatch->xmodelPieces = DB_AddXAsset(ASSET_TYPE_SOUND_PATCH, (XAssetHeader)soundPatch->xmodelPieces).xmodelPieces;
    SND_AddPatch(soundPatch->soundPatch);
}

void __cdecl Mark_SndPatchAsset(SndPatch *soundPatch)
{
    DB_GetXAsset(ASSET_TYPE_SOUND_PATCH, (XAssetHeader)soundPatch);
}

void __cdecl DB_RemoveSound(XAssetHeader header)
{
    SND_RemoveBank(header.sound);
}

void __cdecl DB_RemoveSoundPatch(XAssetHeader header)
{
    SND_RemovePatch(header.soundPatch);
}

void __cdecl Load_ClipMapAsset(XAssetHeader *clipMap)
{
    clipMap->xmodelPieces = DB_AddXAsset(ASSET_TYPE_CLIPMAP_PVS, (XAssetHeader)clipMap->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_ClipMapAsset(clipMap_t *clipMap)
{
    DB_GetXAsset(ASSET_TYPE_CLIPMAP_PVS, (XAssetHeader)clipMap);
}

void __cdecl DB_RemoveClipMap(XAssetHeader)
{
    CM_Unload();
}

void __cdecl Load_ComWorldAsset(XAssetHeader *comWorld)
{
    comWorld->xmodelPieces = DB_AddXAsset(ASSET_TYPE_COMWORLD, (XAssetHeader)comWorld->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_ComWorldAsset(ComWorld *comWorld)
{
    DB_GetXAsset(ASSET_TYPE_COMWORLD, (XAssetHeader)comWorld);
}

void __cdecl DB_RemoveComWorld(XAssetHeader)
{
    Com_UnloadWorld();
}

void __cdecl Load_GameWorldSpAsset(XAssetHeader *gameWorldSp)
{
    gameWorldSp->xmodelPieces = DB_AddXAsset(ASSET_TYPE_GAMEWORLD_SP, (XAssetHeader)gameWorldSp->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_GameWorldSpAsset(GameWorldSp *gameWorldSp)
{
    DB_GetXAsset(ASSET_TYPE_GAMEWORLD_SP, (XAssetHeader)gameWorldSp);
}

void __cdecl Load_GameWorldMpAsset(XAssetHeader *gameWorldMp)
{
    gameWorldMp->xmodelPieces = DB_AddXAsset(ASSET_TYPE_GAMEWORLD_MP, (XAssetHeader)gameWorldMp->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_GameWorldMpAsset(GameWorldMp *gameWorldMp)
{
    DB_GetXAsset(ASSET_TYPE_GAMEWORLD_MP, (XAssetHeader)gameWorldMp);
}

void __cdecl Load_MapEntsAsset(XAssetHeader *mapEnts)
{
    mapEnts->xmodelPieces = DB_AddXAsset(ASSET_TYPE_MAP_ENTS, (XAssetHeader)mapEnts->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_MapEntsAsset(MapEnts *mapEnts)
{
    DB_GetXAsset(ASSET_TYPE_MAP_ENTS, (XAssetHeader)mapEnts);
}

void __cdecl Load_DDLAsset(XAssetHeader *ddlRoot)
{
    ddlRoot->xmodelPieces = DB_AddXAsset(ASSET_TYPE_DDL, (XAssetHeader)ddlRoot->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_DDLAsset(ddlRoot_t *ddlRoot)
{
    DB_GetXAsset(ASSET_TYPE_DDL, (XAssetHeader)ddlRoot);
}

void __cdecl Load_GfxWorldAsset(XAssetHeader *gfxWorld)
{
    gfxWorld->xmodelPieces = DB_AddXAsset(ASSET_TYPE_GFXWORLD, (XAssetHeader)gfxWorld->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_GfxWorldAsset(GfxWorld *gfxWorld)
{
    DB_GetXAsset(ASSET_TYPE_GFXWORLD, (XAssetHeader)gfxWorld);
}

void __cdecl DB_RemoveGfxWorld(XAssetHeader)
{
    R_UnloadWorld();
}

void __cdecl Load_LightDefAsset(XAssetHeader *lightDef)
{
    lightDef->xmodelPieces = DB_AddXAsset(ASSET_TYPE_LIGHT_DEF, (XAssetHeader)lightDef->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_LightDefAsset(GfxLightDef *lightDef)
{
    DB_GetXAsset(ASSET_TYPE_LIGHT_DEF, (XAssetHeader)lightDef);
}

void __cdecl Load_FontAsset(XAssetHeader *font)
{
    font->xmodelPieces = DB_AddXAsset(ASSET_TYPE_FONT, (XAssetHeader)font->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_FontAsset(Font_s *font)
{
    DB_GetXAsset(ASSET_TYPE_FONT, (XAssetHeader)font);
}

void __cdecl Load_MenuListAsset(XAssetHeader *menuList)
{
    menuList->xmodelPieces = DB_AddXAsset(ASSET_TYPE_MENULIST, (XAssetHeader)menuList->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_MenuListAsset(MenuList *menuList)
{
    DB_GetXAsset(ASSET_TYPE_MENULIST, (XAssetHeader)menuList);
}

void __cdecl Load_MenuAsset(XAssetHeader *menu)
{
    XAssetHeader header; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    //header.xmodelPieces = menu->xmodelPieces;
    //menu->xmodelPieces = DB_AddXAsset(ASSET_TYPE_MENU, (XAssetHeader)menu->xmodelPieces).xmodelPieces;

    //for ( i = 0; i < (int)header.xmodelPieces[14].pieces; ++i )
    //    *(XAssetHeader *)(*((unsigned int *)&header.xmodelPieces[32].pieces->model + i) + 196) = (XAssetHeader)menu->xmodelPieces;

    header.menu = menu->menu;
    menu->menu = DB_AddXAsset(ASSET_TYPE_MENU, *menu).menu;

    for (i = 0; i < header.menu->itemCount; ++i)
        header.menu->items[i]->parent = menu->menu;

}

void __cdecl Mark_MenuAsset(menuDef_t *menu)
{
    DB_GetXAsset(ASSET_TYPE_MENU, (XAssetHeader)menu);
}

void __cdecl DB_DynamicCloneMenu(const XAssetHeader from, XAssetHeader to, DBCloneMethod _formal)
{
    windowDef_t *toWindow; // [esp+14h] [ebp-18h]
    int toIndex; // [esp+18h] [ebp-14h]
    int fromIndex; // [esp+1Ch] [ebp-10h]
    windowDef_t *fromWindow; // [esp+24h] [ebp-8h]

    from.xmodelPieces[7].name = to.xmodelPieces[7].name;
    for ( toIndex = 0; toIndex < (int)from.xmodelPieces[14].pieces; ++toIndex )
    {
        toWindow = (windowDef_t *)*((unsigned int *)&from.xmodelPieces[32].pieces->model + toIndex);
        if ( toWindow->name )
        {
            for ( fromIndex = 0; fromIndex < (int)to.xmodelPieces[14].pieces; ++fromIndex )
            {
                fromWindow = (windowDef_t *)*((unsigned int *)&to.xmodelPieces[32].pieces->model + fromIndex);
                if ( fromWindow->name && !strcmp(fromWindow->name, toWindow->name) )
                {
                    toWindow->dynamicFlags[0] = fromWindow->dynamicFlags[0];
                    break;
                }
            }
        }
        DB_RemoveWindowFocus(toWindow);
    }
}

void __cdecl DB_RemoveWindowFocus(windowDef_t *window)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for ( i = 0; !i; i = 1 )
        window->dynamicFlags[0] &= ~2u;
}

void __cdecl Load_LocalizeEntryAsset(XAssetHeader *localize)
{
    localize->xmodelPieces = DB_AddXAsset(ASSET_TYPE_LOCALIZE_ENTRY, (XAssetHeader)localize->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_LocalizeEntryAsset(LocalizeEntry *localize)
{
    DB_GetXAsset(ASSET_TYPE_LOCALIZE_ENTRY, (XAssetHeader)localize);
}

void __cdecl Load_WeaponVariantDefAsset(XAssetHeader *weapon)
{
    weapon->xmodelPieces = DB_AddXAsset(ASSET_TYPE_WEAPON, (XAssetHeader)weapon->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_WeaponVariantDefAsset(WeaponVariantDef *weapon)
{
    DB_GetXAsset(ASSET_TYPE_WEAPON, (XAssetHeader)weapon);
}

void __cdecl Load_SndDriverGlobalsAsset(SndDriverGlobals **sndDriverGlobals)
{
    XAssetHeader header;
    header.sndDriverGlobals = *sndDriverGlobals;
    *sndDriverGlobals = DB_AddXAsset(ASSET_TYPE_SNDDRIVER_GLOBALS, header).sndDriverGlobals;
}

void __cdecl Mark_SndDriverGlobalsAsset(SndDriverGlobals *sndDriverGlobals)
{
    DB_GetXAsset(ASSET_TYPE_SNDDRIVER_GLOBALS, (XAssetHeader)sndDriverGlobals);
}

void __cdecl Load_FxEffectDefAsset(XAssetHeader *fx)
{
    fx->xmodelPieces = DB_AddXAsset(ASSET_TYPE_FX, (XAssetHeader)fx->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_FxEffectDefAsset(FxEffectDef *fx)
{
    DB_GetXAsset(ASSET_TYPE_FX, (XAssetHeader)fx);
}

void __cdecl Load_FxEffectDefFromName(const char **name)
{
    if ( *name )
        *(XAssetHeader *)name = DB_FindXAssetHeader(ASSET_TYPE_FX, (char *)*name, 1, -1);
}

void __cdecl Load_FxImpactTableAsset(XAssetHeader *impactFx)
{
    impactFx->xmodelPieces = DB_AddXAsset(ASSET_TYPE_IMPACT_FX, (XAssetHeader)impactFx->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_FxImpactTableAsset(FxImpactTable *impactFx)
{
    DB_GetXAsset(ASSET_TYPE_IMPACT_FX, (XAssetHeader)impactFx);
}

void __cdecl Load_RawFileAsset(XAssetHeader *rawfile)
{
    rawfile->xmodelPieces = DB_AddXAsset(ASSET_TYPE_RAWFILE, (XAssetHeader)rawfile->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_RawFileAsset(RawFile *rawfile)
{
    DB_GetXAsset(ASSET_TYPE_RAWFILE, (XAssetHeader)rawfile);
}

void __cdecl Load_StringTableAsset(XAssetHeader *stringTable)
{
    stringTable->xmodelPieces = DB_AddXAsset(ASSET_TYPE_STRINGTABLE, (XAssetHeader)stringTable->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_StringTableAsset(StringTable *stringTable)
{
    DB_GetXAsset(ASSET_TYPE_STRINGTABLE, (XAssetHeader)stringTable);
}

void __cdecl Load_PackIndexAsset(XAssetHeader *packIndex)
{
    packIndex->xmodelPieces = DB_AddXAsset(ASSET_TYPE_PACK_INDEX, (XAssetHeader)packIndex->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_PackIndexAsset(PackIndex *packIndex)
{
    DB_GetXAsset(ASSET_TYPE_PACK_INDEX, (XAssetHeader)packIndex);
}

void __cdecl Load_XGlobalsAsset(XAssetHeader *xGlobals)
{
    xGlobals->xmodelPieces = DB_AddXAsset(ASSET_TYPE_XGLOBALS, (XAssetHeader)xGlobals->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_XGlobalsAsset(XGlobals *xGlobals)
{
    DB_GetXAsset(ASSET_TYPE_XGLOBALS, (XAssetHeader)xGlobals);
}

void __cdecl Load_GlassesAsset(XAssetHeader *glasses)
{
    glasses->xmodelPieces = DB_AddXAsset(ASSET_TYPE_GLASSES, (XAssetHeader)glasses->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_GlassesAsset(Glasses *glasses)
{
    DB_GetXAsset(ASSET_TYPE_GLASSES, (XAssetHeader)glasses);
}

void __cdecl Load_EmblemSetAsset(XAssetHeader *emblemSet)
{
    emblemSet->xmodelPieces = DB_AddXAsset(ASSET_TYPE_EMBLEMSET, (XAssetHeader)emblemSet->xmodelPieces).xmodelPieces;
}

void __cdecl Mark_EmblemSetAsset(EmblemSet *emblemSet)
{
    DB_GetXAsset(ASSET_TYPE_EMBLEMSET, (XAssetHeader)emblemSet);
}

void __cdecl DB_FreeXAssetHeader_EmblemSet_(XAssetPoolEntry<EmblemSet> **pool, XAssetHeader header)
{
    XAssetPoolEntry<EmblemSet> *oldFreeHead; // [esp+8h] [ebp-4h]

    oldFreeHead = *pool;
    *pool = (XAssetPoolEntry<EmblemSet> *)header.xmodelPieces;
    header.xmodelPieces->name = (const char *)oldFreeHead;
}

XAssetHeader __cdecl DB_AllocMaterial(void *arg)
{
    XAssetHeader *pool = (XAssetHeader *)arg;
    Material_DirtySort();
    return DB_AllocXAsset<Material>(pool);
}

void __cdecl DB_FreeMaterial(XAssetPoolEntry<EmblemSet> **pool, XAssetHeader header)
{
    Material_DirtySort();
    DB_FreeXAssetHeader_EmblemSet_(pool, header);
}

void __cdecl DB_RemoveGfxImage(XAssetHeader header)
{
    R_Stream_ReleaseImage(header.image, 1, 0);
    //HIBYTE(header.xmodelPieces[2].name) = 0;
    header.image->streaming = 0; // 0x1b
}

void __cdecl DB_FreeGfxImage(XAssetPoolEntry<EmblemSet> **pool, XAssetHeader header)
{
    DB_RemoveGfxImage(header);
    DB_FreeXAssetHeader_EmblemSet_(pool, header);
}

void __cdecl DB_FreeMenu(XAssetPoolEntry<EmblemSet> **pool, XAssetHeader header)
{
    Menus_RemoveMenu(header);
    DB_FreeXAssetHeader_EmblemSet_(pool, header);
}

void __cdecl DB_FreeXGlobals(XAssetPoolEntry<EmblemSet> **pool, XAssetHeader header)
{
    DB_FreeXAssetHeader_EmblemSet_(pool, header);
}

void __cdecl DB_MarkAssetTypeUsageDirty()
{
    memset((unsigned __int8 *)s_usageUpToDate, 0, sizeof(s_usageUpToDate));
}

void __cdecl DB_GetAssetTypeUsageInfo(XAssetType assetType, const char **outName, int *outUsed, int *outPool)
{
    if ( !s_usageUpToDate[assetType] )
    {
        s_usageUpToDate[assetType] = 1;
        s_usageCounts[assetType] = DB_GetAllXAssetOfType(assetType, 0, 0);
    }
    *outUsed = s_usageCounts[assetType];
    *outPool = g_poolSize[assetType];
    *outName = g_assetNames[assetType];
}

void __cdecl DB_PrintXAssetsForType_FastFile(XAssetType type, void *inData, bool includeOverride)
{
    const char *XAssetHeaderName; // eax
    const char *v4; // eax
    XZoneName *v5; // [esp-4h] [ebp-18h]
    XZoneName *v6; // [esp-4h] [ebp-18h]
    unsigned int hash; // [esp+0h] [ebp-14h]
    unsigned int assetEntryIndex; // [esp+4h] [ebp-10h]
    XAssetEntryPoolEntry *assetEntry; // [esp+Ch] [ebp-8h]
    unsigned int overrideAssetEntryIndex; // [esp+10h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_DBHASH);
    for ( hash = 0; hash < 0x8000; ++hash )
    {
        for ( assetEntryIndex = db_hashTable[hash]; assetEntryIndex; assetEntryIndex = assetEntry->entry.nextHash )
        {
            assetEntry = &g_assetEntryPool[assetEntryIndex];
            if ( assetEntry->entry.asset.type == type )
            {
                v5 = &g_zoneNames[assetEntry->entry.zoneIndex];
                XAssetHeaderName = DB_GetXAssetHeaderName(type, &assetEntry->entry.asset.header);
                Com_Printf(0, "'%s','%s'\n", XAssetHeaderName, v5->name);
                if ( includeOverride )
                {
                    for ( overrideAssetEntryIndex = assetEntry->entry.nextOverride;
                                overrideAssetEntryIndex;
                                overrideAssetEntryIndex = g_assetEntryPool[overrideAssetEntryIndex].entry.nextOverride )
                    {
                        v6 = &g_zoneNames[g_assetEntryPool[overrideAssetEntryIndex].entry.zoneIndex];
                        v4 = DB_GetXAssetHeaderName(type, &g_assetEntryPool[overrideAssetEntryIndex].entry.asset.header);
                        Com_Printf(0, "'%s','%s'\n", v4, v6->name);
                    }
                }
            }
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_DBHASH);
}

void __cdecl DB_PrintXAssetsForType(
                XAssetType type,
                void (__cdecl *func)(XAssetHeader, void *),
                void *inData,
                bool includeOverride)
{
    if ( useFastFile->current.enabled )
        DB_PrintXAssetsForType_FastFile(type, &inData, includeOverride);
    else
        DB_EnumXAssets(type, func, &inData, includeOverride);
}

GfxImage *__cdecl DB_AllocTempImage()
{
    XAssetHeader header; // [esp+4h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_DBHASH);
    header.xmodelPieces = DB_AllocXAssetHeader(ASSET_TYPE_IMAGE).xmodelPieces;
    Sys_LeaveCriticalSection(CRITSECT_DBHASH);
    return (GfxImage *)header.xmodelPieces;
}

XAssetHeader __cdecl DB_AllocXAssetHeader(XAssetType type)
{
    XAssetHeader header; // [esp+4h] [ebp-4h]

    header.xmodelPieces = DB_AllocXAssetHeaderHandler[type](DB_XAssetPool[type]).xmodelPieces;
    if ( !header.xmodelPieces )
    {
        Sys_LeaveCriticalSection(CRITSECT_DBHASH);
        Com_PrintError(1, "Exceeded limit of %d '%s' assets.\n", g_poolSize[type], g_assetNames[type]);
        DB_PrintXAssetsForType(type, (void (__cdecl *)(XAssetHeader, void *))DB_PrintAssetName, &type, 1);
        Com_Error(ERR_DROP, "Exceeded limit of %d '%s' assets.\n", g_poolSize[type], g_assetNames[type]);
    }
    return header;
}

void __cdecl DB_PrintAssetName(XAssetHeader header, int *data)
{
    const char *XAssetHeaderName; // eax

    XAssetHeaderName = DB_GetXAssetHeaderName(*data, &header);
    Com_Printf(0, "%s\n", XAssetHeaderName);
}

void __cdecl DB_FreeXAssetHeader(XAssetType type, XAssetHeader header)
{
    DB_FreeXAssetHeaderHandler[type](DB_XAssetPool[type], header);
}

unsigned int __cdecl DB_HashForName(const char *name, XAssetType type)
{
    unsigned int hash = type;

    while (*name)
    {
        int c = tolower(*name++);

        if (c == '\\')
            c = '/';

        hash = (hash << 16) + (hash << 6) + c - hash;
    }

    return hash;
}

void __cdecl DB_LogMissingAsset(XAssetType type, const char *name)
{
    char msg[1028]; // [esp+14h] [ebp-408h] BYREF

    switch ( type )
    {
        case ASSET_TYPE_SOUND:
        case ASSET_TYPE_MENU:
        case ASSET_TYPE_LOCALIZE_ENTRY:
        case ASSET_TYPE_SNDDRIVER_GLOBALS:
        case ASSET_TYPE_XGLOBALS:
            return;
        case ASSET_TYPE_WEAPON:
            Com_sprintf(msg, 0x400u, "%s,mp/%s\n", g_assetNames[type], name);
            goto LABEL_4;
        default:
            Com_sprintf(msg, 0x400u, "%s,%s\n", g_assetNames[type], name);
LABEL_4:
            Sys_EnterCriticalSection(CRITSECT_MISSING_ASSET);
            if ( g_missingAssetFile )
                g_missingAssetFile = FS_FOpenFileAppend((char*)"missingasset.csv");
            else
                g_missingAssetFile = FS_FOpenTextFileWrite((char *)"missingasset.csv");
            if ( g_missingAssetFile )
            {
                FS_Write(msg, &msg[strlen(msg) + 1] - &msg[1], g_missingAssetFile);
                FS_FCloseFile(g_missingAssetFile);
            }
            else
            {
                com_missingAssetOpenFailed = 1;
            }
            Sys_LeaveCriticalSection(CRITSECT_MISSING_ASSET);
            break;
    }
}

void __cdecl DB_EnumXAssets_FastFile(
                XAssetType type,
                void (__cdecl *func)(XAssetHeader, void *),
                void *inData,
                bool includeOverride)
{
    unsigned int hash; // [esp+0h] [ebp-14h]
    unsigned int assetEntryIndex; // [esp+4h] [ebp-10h]
    XAssetEntryPoolEntry *assetEntry; // [esp+Ch] [ebp-8h]
    unsigned int overrideAssetEntryIndex; // [esp+10h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_DBHASH);
    for ( hash = 0; hash < 0x8000; ++hash )
    {
        for ( assetEntryIndex = db_hashTable[hash]; assetEntryIndex; assetEntryIndex = assetEntry->entry.nextHash )
        {
            assetEntry = &g_assetEntryPool[assetEntryIndex];
            if ( assetEntry->entry.asset.type == type )
            {
                //((void (__cdecl *)(unsigned int, unsigned int))func)((XAssetHeader)assetEntry->entry.asset.header.xmodelPieces, inData);
                func(assetEntry->entry.asset.header, inData);
                if ( includeOverride )
                {
                    for ( overrideAssetEntryIndex = assetEntry->entry.nextOverride;
                                overrideAssetEntryIndex;
                                overrideAssetEntryIndex = g_assetEntryPool[overrideAssetEntryIndex].entry.nextOverride )
                    {
                        //((void (__cdecl *)(unsigned int, unsigned int))func)((XAssetHeader)g_assetEntryPool[overrideAssetEntryIndex].entry.asset.header.xmodelPieces, inData);
                        func(g_assetEntryPool[overrideAssetEntryIndex].entry.asset.header, inData);
                    }
                }
            }
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_DBHASH);
}

void __cdecl DB_LoadDelayedImages()
{
    unsigned int copyIter; // [esp+0h] [ebp-4h]

    DB_EnumXAssets(ASSET_TYPE_IMAGE, R_DelayLoadImage, 0, 0);
    for ( copyIter = 0; copyIter < g_copyInfoCount; ++copyIter )
    {
        if ( g_copyInfo[copyIter]->asset.type == ASSET_TYPE_IMAGE )
            R_DelayLoadImage(g_copyInfo[copyIter]->asset.header, 0);
    }
}

XAssetHeader __cdecl DB_FindXAssetDefaultHeaderInternal(XAssetType type)
{
    const char *XAssetName; // eax
    unsigned int assetEntryIndex; // [esp+8h] [ebp-Ch]
    const char *name; // [esp+Ch] [ebp-8h]
    XAssetEntryPoolEntry *assetEntry; // [esp+10h] [ebp-4h]

    name = g_defaultAssetName[type];
    for ( assetEntryIndex = db_hashTable[DB_HashForName(name, type) % 0x8000]; ; assetEntryIndex = assetEntry->entry.nextHash )
    {
        if ( !assetEntryIndex )
            return 0;
        assetEntry = &g_assetEntryPool[assetEntryIndex];
        if ( assetEntry->entry.asset.type == type )
        {
            XAssetName = DB_GetXAssetName(&assetEntry->entry.asset);
            if ( !I_stricmp(XAssetName, name) )
                break;
        }
    }
    while ( assetEntry->entry.nextOverride )
        assetEntry = &g_assetEntryPool[assetEntry->entry.nextOverride];
    return assetEntry->entry.asset.header;
}

const char *__cdecl DB_FindXAssetNameFromHash(XAssetType type, unsigned int hash)
{
    //unsigned int assetEntryIndex; // [esp+0h] [ebp-Ch]
    const char *name; // [esp+4h] [ebp-8h]
    XAssetEntryPoolEntry *assetEntry; // [esp+8h] [ebp-4h]

    uint16_t assetEntryIndex = db_hashTable[hash % 0x8000];

    Sys_EnterCriticalSection(CRITSECT_DBHASH);

    while (assetEntryIndex)
    {
        assetEntry = &g_assetEntryPool[assetEntryIndex];

        if (assetEntry->entry.asset.type == type)
        {
            name = DB_GetXAssetName(&assetEntry->entry.asset);
            if (DB_HashForName(name, type) == hash)
            {
                Sys_LeaveCriticalSection(CRITSECT_DBHASH);
                return name;
            }
        }

        assetEntryIndex = assetEntry->entry.nextHash;
    }
        
    Sys_LeaveCriticalSection(CRITSECT_DBHASH);

    return 0;
}

XAssetHeader __cdecl DB_FindXAssetHeader(XAssetType type, char *name, bool errorIfMissing, int waitTime)
{
    XAssetEntryPoolEntry *assEntry; // eax
    DWORD v6; // eax
    XAssetHeader result; // eax
    XAssetEntryPoolEntry *v8; // eax
    DWORD v9; // eax
    const char *String; // eax
    const char *XAssetTypeName; // eax
    XAssetEntry *DefaultEntry; // eax
    const char *v13; // [esp-4h] [ebp-F0h]
    char *v14; // [esp+8h] [ebp-E4h]
    char *v15; // [esp+14h] [ebp-D8h]
    signed int v16; // [esp+40h] [ebp-ACh]
    signed int v17; // [esp+50h] [ebp-9Ch]
    BOOL suspendedThread; // [esp+78h] [ebp-74h]
    int semaphore; // [esp+7Ch] [ebp-70h]
    const char *basename; // [esp+80h] [ebp-6Ch]
    DWORD start; // [esp+90h] [ebp-5Ch]
    char so_name[64]; // [esp+94h] [ebp-58h] BYREF
    const char *so_prefix; // [esp+D8h] [ebp-14h]
    int use_so_name; // [esp+DCh] [ebp-10h]
    XAssetEntry *assetEntry; // [esp+E0h] [ebp-Ch]
    XAssetEntry *newEntry; // [esp+E4h] [ebp-8h]
    const char *bspext; // [esp+E8h] [ebp-4h]

    if (!useFastFile->current.enabled
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
            3435,
            0,
            "%s",
            "IsFastFileLoad()"))
    {
        __debugbreak();
    }
    start = 0;
    so_prefix = "maps/so_";
    bspext = ".d3dbsp";
    use_so_name = 0;
    if (!I_strncmp("maps/so_", name, strlen("maps/so_")))
    {
        v17 = strlen(name);
        v16 = strlen(bspext);
        if (v17 > v16 && !I_strncmp(bspext, &name[v17 - v16], strlen(bspext)))
        {
            for (basename = &name[strlen(so_prefix)]; *basename && *basename != 95; ++basename)
                ;
            if (!*basename)
                Com_PrintError(1, "Bad specop level name\n");
            if (I_strncmp("_mp_", basename, 4))
                Com_sprintf(so_name, 0x40u, "maps/%s", basename + 1);
            else
                Com_sprintf(so_name, 0x40u, "maps/mp/%s", basename + 1);
            use_so_name = 1;
        }
    }
    do
    {
        while (1)
        {
            Sys_EnterCriticalSection(CRITSECT_DBHASH);
            if (use_so_name)
                assEntry = DB_FindXAssetEntry(type, so_name);
            else
                assEntry = DB_FindXAssetEntry(type, name);
            assetEntry = &assEntry->entry;
            Sys_LeaveCriticalSection(CRITSECT_DBHASH);
            if (use_so_name)
                DB_RegisteredReorderAsset(type, so_name, assetEntry);
            else
                DB_RegisteredReorderAsset(type, name, assetEntry);
            if (assetEntry && (assetEntry->zoneIndex || Sys_IsDatabaseReady2()))
                goto returnAsset;
            if (Sys_IsDatabaseThread())
                goto LABEL_62;
            if (start)
                break;
            ProfLoad_Begin("Wait for fastfile asset");
            start = Sys_Milliseconds();
            if (!Sys_IsDatabaseReady2())
            {
                //if (Sys_IsMainThread())
                //    BG_EvalVehicleName();
                break;
            }
        }
        if (Sys_IsDatabaseReady2() || DB_IsMinimumFastFileLoaded() && DB_GetInitializing())
            break;
        if (Sys_IsDatabaseReady()
            && (Sys_IsMainThread() || Sys_IsRenderThread() && R_IsInRemoteScreenUpdate() && Sys_QueryRGRegisteredEvent()))
        {
            DB_PostLoadXZone();
        }
        else
        {
            //if (Sys_IsMainThread())
            //    BG_EvalVehicleName();
            if (Sys_IsRenderThread())
                RB_Resource_Update(5);
            suspendedThread = Sys_HaveSuspendedDiscReads(THREAD_OWNER_DATABASE);
            if (suspendedThread)
                Sys_ResumeDiscReads(THREAD_OWNER_DATABASE);
            semaphore = R_ReleaseDXDeviceOwnership();
            DB_Sleep(0);
            if (semaphore)
                R_AcquireDXDeviceOwnership(0);
            if (suspendedThread)
                Sys_SuspendDiscReads(THREAD_OWNER_DATABASE);
        }
    } while (waitTime == -1 || (int)(Sys_Milliseconds() - start) < waitTime);
    ProfLoad_End();
LABEL_62:
    if (assetEntry)
    {
    returnAsset:
        if (!assetEntry->asset.header.xmodelPieces
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                3485,
                0,
                "%s",
                "assetEntry->asset.header.data"))
        {
            __debugbreak();
        }
        assetEntry->inuse = 1;
        if (start)
        {
            if (use_so_name)
                v15 = so_name;
            else
                v15 = name;
            v13 = g_assetNames[type];
            v6 = Sys_Milliseconds();
            Com_Printf(10, "Waited %i msec for asset '%s' of type '%s'.\n", v6 - start, v15, v13);
            ProfLoad_End();
        }
        return assetEntry->asset.header;
    }
    Sys_EnterCriticalSection(CRITSECT_DBHASH);
    if (use_so_name)
        v8 = DB_FindXAssetEntry(type, so_name);
    else
        v8 = DB_FindXAssetEntry(type, name);
    assetEntry = &v8->entry;
    if (v8)
    {
        if (!assetEntry->asset.header.xmodelPieces
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                3581,
                0,
                "%s",
                "assetEntry->asset.header.data"))
        {
            __debugbreak();
        }
        Sys_LeaveCriticalSection(CRITSECT_DBHASH);
        goto returnAsset;
    }
    if (errorIfMissing && type != ASSET_TYPE_XANIMPARTS && com_developer->current.enabled)
    {
        if (use_so_name)
            DB_LogMissingAsset(type, so_name);
        else
            DB_LogMissingAsset(type, name);
    }
    if (start && errorIfMissing)
    {
        if (use_so_name)
            v14 = so_name;
        else
            v14 = name;
        v9 = Sys_Milliseconds();
        PrintWaitedError(type, v14, v9 - start);
    }
    switch (type)
    {
    case ASSET_TYPE_LOCALIZE_ENTRY:
    case ASSET_TYPE_RAWFILE:
    case ASSET_TYPE_XGLOBALS:
    case ASSET_TYPE_GLASSES:
        Sys_LeaveCriticalSection(CRITSECT_DBHASH);
        result.xmodelPieces = 0;
        break;
    default:
        if (db_xassetdebug)
        {
            if (db_xassetdebug->current.enabled)
            {
                if (db_xassetdebugtype->current.integer == -1
                    || db_xassetdebugtype->current.integer == type
                    || *Dvar_GetString("db_xassetdebugname")
                    && (String = Dvar_GetString("db_xassetdebugname"), !I_stricmp(name, String)))
                {
                    XAssetTypeName = DB_GetXAssetTypeName(type);
                    Com_Printf(
                        16,
                        "***db_xassetdebug:***\nDB_FindXAssetHeader: missing asset: '%s','%s'\n",
                        XAssetTypeName,
                        name);
                }
            }
        }
        if (use_so_name)
            DefaultEntry = DB_CreateDefaultEntry(type, so_name);
        else
            DefaultEntry = DB_CreateDefaultEntry(type, name);
        newEntry = DefaultEntry;
        Sys_LeaveCriticalSection(CRITSECT_DBHASH);
        //result.xmodelPieces = (XModelPieces *)newEntry->asset.header;
        result.xmodelPieces = newEntry->asset.header.xmodelPieces;
        break;
    }
    return result;
}

void __cdecl DB_RegisteredReorderAsset(int type, const char *assetName, XAssetEntry *assetEntry)
{
    DBReorderAssetEntry *entry; // [esp+0h] [ebp-Ch]
    const char *extension; // [esp+4h] [ebp-8h]
    unsigned int entryIter; // [esp+8h] [ebp-4h]

    if ( s_dbReorder.entryCount )
    {
        if ( type == 23 )
        {
            if ( !s_dbReorder.loadedLocalization )
                s_dbReorder.loadedLocalization = DB_RegisterAllReorderAssetsOfType(23, assetEntry) != 0;
        }
        else if ( type == 9 )
        {
            if ( !s_dbReorder.loadedSound )
                s_dbReorder.loadedSound = DB_RegisterAllReorderAssetsOfType(9, assetEntry) != 0;
        }
        else if ( !s_dbReorder.lastEntry
                     || s_dbReorder.lastEntry->type != type
                     || _stricmp(s_dbReorder.lastEntry->assetName, assetName) )
        {
            Sys_LockWrite(&s_dbReorder.critSect);
            s_dbReorder.lastEntry = 0;
            for ( entryIter = 0; entryIter < s_dbReorder.entryCount; ++entryIter )
            {
                entry = &s_dbReorder.entries[entryIter];
                if ( entry->type == type && !_stricmp(entry->assetName, assetName) )
                {
                    s_dbReorder.lastEntry = &s_dbReorder.entries[entryIter];
                    if ( entry->sequence == -1 )
                    {
                        entry->sequence = s_dbReorder.sequence;
                        if ( entry->type == 36 )
                        {
                            extension = Com_GetExtensionSubString(assetName);
                            if ( !I_stricmp(extension, ".gsc") )
                                s_dbReorder.sequenceForIncludes = s_dbReorder.sequence + 1;
                        }
                        s_dbReorder.sequence += 2;
                        if ( s_dbReorder.alreadyFinished )
                            DB_EndReorderZone();
                    }
                    break;
                }
            }
            Sys_UnlockWrite(&s_dbReorder.critSect);
        }
    }
}

void __cdecl Sys_LockWrite(FastCriticalSection *critSect)
{
    while ( 1 )
    {
        if ( critSect->readCount )
            goto LABEL_5;
        if ( _InterlockedIncrement(&critSect->writeCount) == 1 && !critSect->readCount )
            break;
        _InterlockedExchangeAdd(&critSect->writeCount, 0xFFFFFFFF);
LABEL_5:
        NET_Sleep(0);
    }
}

void __cdecl Sys_UnlockWrite(FastCriticalSection *critSect)
{
    if ( critSect->writeCount <= 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\universal\\../qcommon/threads_interlock.h",
                    126,
                    0,
                    "%s",
                    "critSect->writeCount > 0") )
    {
        __debugbreak();
    }
    _InterlockedExchangeAdd(&critSect->writeCount, 0xFFFFFFFF);
}

void DB_EndReorderZone()
{
    DBReorderAssetEntry *entry; // [esp+180h] [ebp-41Ch]
    bool wroteBlank; // [esp+187h] [ebp-415h]
    unsigned int bytesa; // [esp+188h] [ebp-414h]
    unsigned int bytes; // [esp+188h] [ebp-414h]
    HANDLE file; // [esp+18Ch] [ebp-410h]
    DWORD written; // [esp+190h] [ebp-40Ch] BYREF
    char csvName[256]; // [esp+194h] [ebp-408h] BYREF
    char line[512]; // [esp+294h] [ebp-308h] BYREF
    char bakName[256]; // [esp+494h] [ebp-108h] BYREF
    unsigned int entryIter; // [esp+598h] [ebp-4h]

    if ( s_dbReorder.entryCount )
    {
        s_dbReorder.alreadyFinished = 1;
        Com_sprintf(csvName, 0x100u, "..\\share\\zone_source\\%s.csv", s_dbReorder.zoneName);
        Com_sprintf(bakName, 0x100u, "%s.bak", csvName);
        DeleteFileA(bakName);
        rename(csvName, bakName);
        file = CreateFileA(csvName, 0x40000000u, 0, 0, 2u, 0, 0);
        if ( file != (HANDLE)-1 )
        {
            wroteBlank = 0;
            DB_SetReorderIncludeSequence();

            std::sort(&s_dbReorder.entries[0], &s_dbReorder.entries[s_dbReorder.entryCount], DB_CompareReorderEntries);
            //std::_Sort<DBReorderAssetEntry *,int,bool (__cdecl *)(DBReorderAssetEntry const &,DBReorderAssetEntry const &)>(
            //    (GfxSModelSurfStats *)s_dbReorder.entries,
            //    (GfxSModelSurfStats *)&s_dbReorder.entries[s_dbReorder.entryCount],
            //    (signed int)(16 * s_dbReorder.entryCount) >> 4,
            //    (bool (__cdecl *)(GfxSModelSurfStats *, GfxSModelSurfStats *))DB_CompareReorderEntries);

            for ( entryIter = 0; entryIter < s_dbReorder.entryCount; ++entryIter )
            {
                entry = &s_dbReorder.entries[entryIter];
                if ( !wroteBlank && entry->sequence == -1 )
                {
                    switch ( entry->type )
                    {
                        case 9:
                        case 0xB:
                        case 0xC:
                        case 0x17:
                        case 0x2B:
                            break;
                        default:
                            wroteBlank = 1;
                            WriteFile(file, "\r\n", 2u, &written, 0);
                            break;
                    }
                }
                if ( entry->type == 24 )
                {
                    bytesa = Com_sprintf(line, 0x200u, "%s,%s%s\r\n", entry->typeString, "mp/", entry->assetName);
                    WriteFile(file, line, bytesa, &written, 0);
                }
                else
                {
                    if ( entry->type == 9 )
                        bytes = Com_sprintf(
                                            line,
                                            0x200u,
                                            "%s,%s,%s,%s\r\n",
                                            entry->typeString,
                                            entry->assetName,
                                            s_dbReorder.zoneName,
                                            "all_mp");
                    else
                        bytes = Com_sprintf(line, 0x200u, "%s,%s\r\n", entry->typeString, entry->assetName);
                    WriteFile(file, line, bytes, &written, 0);
                }
            }
            CloseHandle(file);
        }
    }
}

char __cdecl DB_CompareReorderEntries(const DBReorderAssetEntry &e0, const DBReorderAssetEntry &e1)
{
    int comparison;

    if (e0.sequence != e1.sequence)
        return e0.sequence < e1.sequence;

    if (e0.type == 43)
    {
        if (e1.type != 43)
            return true;

        comparison = _stricmp(e0.typeString, e1.typeString);
        if (comparison)
            return comparison < 0;

        return _stricmp(e0.assetName, e1.assetName) < 0;
    }

    if (e1.type == 43)
        return false;

    if (e0.type == e1.type)
        return _stricmp(e0.assetName, e1.assetName) < 0;

    if (e0.sequence != -1)
        return e0.type < e1.type;

    if (e0.type == 9)
        return true;

    if (e1.type == 9)
        return false;

    if (e0.type == 23)
        return true;

    return e1.type != 23 && e0.type < e1.type;
}

void DB_SetReorderIncludeSequence()
{
    DBReorderAssetEntry *entry; // [esp+0h] [ebp-8h]
    unsigned int entryIter; // [esp+4h] [ebp-4h]

    for ( entryIter = 0; entryIter < s_dbReorder.entryCount; ++entryIter )
    {
        entry = &s_dbReorder.entries[entryIter];
        if ( entry->type == 43 && !I_strnicmp(entry->typeString, "include", 7) )
            entry->sequence = s_dbReorder.sequenceForIncludes;
    }
}

char __cdecl DB_RegisterAllReorderAssetsOfType(int type, XAssetEntry *assetEntry)
{
    DBReorderAssetEntry *entry; // [esp+0h] [ebp-8h]
    unsigned int entryIter; // [esp+4h] [ebp-4h]

    if ( assetEntry && _stricmp(g_zoneNames[assetEntry->zoneIndex].name, s_dbReorder.zoneName) )
        return 0;
    if ( Sys_IsDatabaseThread() )
        return 0;
    Sys_LockWrite(&s_dbReorder.critSect);
    for ( entryIter = 0; entryIter < s_dbReorder.entryCount; ++entryIter )
    {
        entry = &s_dbReorder.entries[entryIter];
        if ( entry->type == type && entry->sequence == -1 )
            entry->sequence = s_dbReorder.sequence;
    }
    s_dbReorder.sequence += 2;
    if ( s_dbReorder.alreadyFinished )
        DB_EndReorderZone();
    Sys_UnlockWrite(&s_dbReorder.critSect);
    return 1;
}

void __cdecl DB_Sleep(unsigned int msec)
{
    sysEvent_t result; // [esp+0h] [ebp-38h] BYREF
    sysEvent_t v2; // [esp+1Ch] [ebp-1Ch]

    R_BeginRemoteScreenUpdate();
    v2 = *Sys_GetEvent(&result);

    if (!IsDedicatedServer() && Sys_IsMainThread())
    {
        bool active = Key_IsCatcherActive(0, 16);

        Key_AddCatcher(0, 16);
        IN_Frame();

        if (!active)
            Key_RemoveCatcher(0, -17);

        if (!r_glob.remoteScreenUpdateNesting)
            SCR_UpdateScreen();

        Sys_WaitRenderer();
    }

    NET_Sleep(msec);
    R_EndRemoteScreenUpdate(0);
}

XAssetEntryPoolEntry *__cdecl DB_FindXAssetEntry(XAssetType type, const char *name)
{
    const char *XAssetName; // eax
    unsigned int assetEntryIndex; // [esp+4h] [ebp-8h]
    XAssetEntryPoolEntry *assetEntry; // [esp+8h] [ebp-4h]

    for ( assetEntryIndex = db_hashTable[DB_HashForName(name, type) % 0x8000u];
                assetEntryIndex;
                assetEntryIndex = assetEntry->entry.nextHash )
    {
        assetEntry = &g_assetEntryPool[assetEntryIndex];
        if ( assetEntry->entry.asset.type == type )
        {
            XAssetName = DB_GetXAssetName(&assetEntry->entry.asset);
            if ( !I_stricmp(XAssetName, name) )
                return &g_assetEntryPool[assetEntryIndex];
        }
    }
    return 0;
}

XAssetEntry *__cdecl DB_CreateDefaultEntry(XAssetType type, const char *name)
{
    const char *XAssetTypeName; // eax
    unsigned int String; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *XAssetName; // [esp-8h] [ebp-20h]
    unsigned int hash; // [esp+8h] [ebp-10h]
    XAsset asset; // [esp+Ch] [ebp-Ch] BYREF
    XAssetEntry *newEntry; // [esp+14h] [ebp-4h]

    asset.header = DB_FindXAssetDefaultHeaderInternal(type);
    if ( !asset.header.xmodelPieces )
    {
        Sys_LeaveCriticalSection(CRITSECT_DBHASH);
        if ( type == ASSET_TYPE_CLIPMAP || type == ASSET_TYPE_CLIPMAP_PVS )
        {
            Com_Error(
                ERR_DROP,
                "Couldn't find the bsp for this map.    Please build the fast file associated with %s and try again.",
                name);
        }
        else
        {
            XAssetTypeName = DB_GetXAssetTypeName(type);
            Com_Error(
                ERR_DROP,
                "Could not load default asset '%s' for asset type '%s'.\nTried to load asset '%s'.",
                g_defaultAssetName[type],
                XAssetTypeName,
                name);
        }
    }
    asset.type = type;
    ++g_defaultAssetCount;
    newEntry = &DB_AllocXAssetEntry(type, 0)->entry;
    DB_CloneXAssetInternal(&asset, &newEntry->asset);
    hash = DB_HashForName(name, type);
    newEntry->nextHash = db_hashTable[hash % 0x8000u];
    db_hashTable[hash % 0x8000u] = ((char *)newEntry - (char *)g_assetEntryPool) >> 4;
    String = SL_GetString((char*)name, 4u, SCRIPTINSTANCE_SERVER);
    v4 = SL_ConvertToString(String, SCRIPTINSTANCE_SERVER);
    DB_SetXAssetName(&newEntry->asset, v4);
    newEntry->inuse = 1;
    if ( db_xassetdebug->current.enabled )
    {
        if ( db_xassetdebugtype->current.integer == -1
            || db_xassetdebugtype->current.integer == type
            || *Dvar_GetString("db_xassetdebugname") && (v5 = Dvar_GetString("db_xassetdebugname"), !I_stricmp(name, v5)) )
        {
            XAssetName = DB_GetXAssetName(&asset);
            v6 = DB_GetXAssetTypeName(asset.type);
            Com_Printf(
                16,
                "DB_CreateDefaultEntry: used default asset: '%s','%s' for asset name: '%s'\n",
                v6,
                XAssetName,
                name);
        }
    }
    return newEntry;
}

XAssetEntryPoolEntry *__cdecl DB_AllocXAssetEntry(XAssetType type, unsigned __int8 zoneIndex)
{
    XAssetEntryPoolEntry *freeHead; // [esp+4h] [ebp-8h]

    freeHead = g_freeAssetEntryHead;
    if ( !g_freeAssetEntryHead )
    {
        Sys_LeaveCriticalSection(CRITSECT_DBHASH);
        Com_Error(ERR_DROP, "Could not allocate asset - increase XASSET_ENTRY_POOL_SIZE");
    }
    if ( ++XAssetEntryHighCount > XAssetEntryHighWaterMark )
        XAssetEntryHighWaterMark = XAssetEntryHighCount;
    g_freeAssetEntryHead = (XAssetEntryPoolEntry *)freeHead->entry.asset.type;
    freeHead->entry.asset.type = type;
    freeHead->entry.asset.header = DB_AllocXAssetHeader(type);
    freeHead->entry.zoneIndex = zoneIndex;
    freeHead->entry.inuse = 0;
    freeHead->entry.nextHash = 0;
    freeHead->entry.nextOverride = 0;
    return freeHead;
}

void __cdecl DB_CloneXAssetInternal(const XAsset *from, XAsset *to)
{
    int size; // [esp+0h] [ebp-4h]

    if ( from->type != to->type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    3048,
                    0,
                    "%s",
                    "from->type == to->type") )
    {
        __debugbreak();
    }
    size = DB_GetXAssetTypeSize(from->type);
    memcpy((unsigned __int8 *)to->header.xmodelPieces, (unsigned __int8 *)from->header.xmodelPieces, size);
}

void __cdecl PrintWaitedError(XAssetType type, char *name, int waitedMsec)
{
    if ( waitedMsec > 100 )
    {
        if ( type == ASSET_TYPE_SOUND )
            Com_Printf(10, "Waited %i msec for missing asset \"%s\".\n", waitedMsec, name);
        else
            Com_PrintError(1, "Waited %i msec for missing asset \"%s\".\n", waitedMsec, name);
    }
    switch ( type )
    {
        case ASSET_TYPE_XANIMPARTS:
            Com_PrintWarning(10, "Could not load %s \"%s\".\n", g_assetNames[type], name);
            break;
        case ASSET_TYPE_SOUND:
            return;
        case ASSET_TYPE_LOCALIZE_ENTRY:
            if ( loc_warnings->current.enabled )
            {
                if ( loc_warningsAsErrors->current.enabled )
                    goto LABEL_12;
                Com_PrintWarning(10, "Could not load %s \"%s\".\n", g_assetNames[type], name);
            }
            break;
        case ASSET_TYPE_RAWFILE:
            if ( !IsConfigFile(name) )
                goto LABEL_12;
            break;
        default:
LABEL_12:
            Com_PrintError(1, "Could not load %s \"%s\".\n", g_assetNames[type], name);
            break;
    }
}

bool __cdecl IsConfigFile(char *name)
{
    char *v1; // eax

    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp", 3354, 0, "%s", "name") )
        __debugbreak();
    v1 = strstr(name, ".cfg");
    return v1 != 0;
}

void __cdecl DB_Update()
{
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    3651,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !Sys_IsDatabaseReady2() )
    {
        if ( Sys_IsDatabaseReady() )
            DB_PostLoadXZone();
    }
}

void __cdecl DB_SetInitializing(bool inUse)
{
    g_initializing = inUse;
}

bool __cdecl DB_GetInitializing()
{
    return g_initializing;
}

bool __cdecl DB_IsXAssetDefault(XAssetType type, const char *name)
{
    const char *XAssetName; // eax
    unsigned int hash; // [esp+0h] [ebp-Ch]
    unsigned int assetEntryIndex; // [esp+4h] [ebp-8h]
    XAssetEntryPoolEntry *assetEntry; // [esp+8h] [ebp-4h]

    hash = DB_HashForName(name, type);
    Sys_EnterCriticalSection(CRITSECT_DBHASH);
    for ( assetEntryIndex = db_hashTable[hash % 0x8000]; assetEntryIndex; assetEntryIndex = assetEntry->entry.nextHash )
    {
        assetEntry = &g_assetEntryPool[assetEntryIndex];
        if ( assetEntry->entry.asset.type == type )
        {
            XAssetName = DB_GetXAssetName(&assetEntry->entry.asset);
            if ( !I_stricmp(XAssetName, name) )
            {
                Sys_LeaveCriticalSection(CRITSECT_DBHASH);
                return assetEntry->entry.zoneIndex == 0;
            }
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_DBHASH);
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp", 3697, 0, "unreachable") )
        __debugbreak();
    return 1;
}

int __cdecl DB_GetAllXAssetOfType_FastFile(XAssetType type, XAssetHeader *assets, int maxCount)
{
    unsigned int hash; // [esp+0h] [ebp-10h]
    unsigned int assetEntryIndex; // [esp+4h] [ebp-Ch]
    int assetCount; // [esp+8h] [ebp-8h]
    XAssetEntryPoolEntry *assetEntry; // [esp+Ch] [ebp-4h]

    assetCount = 0;
    Sys_EnterCriticalSection(CRITSECT_DBHASH);
    for ( hash = 0; hash < 0x8000; ++hash )
    {
        for ( assetEntryIndex = db_hashTable[hash]; assetEntryIndex; assetEntryIndex = assetEntry->entry.nextHash )
        {
            assetEntry = &g_assetEntryPool[assetEntryIndex];
            if ( assetEntry->entry.asset.type == type )
            {
                if ( assets )
                {
                    if ( assetCount >= maxCount
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                                    3726,
                                    0,
                                    "%s",
                                    "assetCount < maxCount") )
                    {
                        __debugbreak();
                    }
                    assets[assetCount] = assetEntry->entry.asset.header;
                }
                ++assetCount;
            }
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_DBHASH);
    return assetCount;
}

void __cdecl DB_BeginRecoverLostDevice()
{
    int semaphore; // [esp+0h] [ebp-4h]

    if ( !Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    3745,
                    0,
                    "%s",
                    "Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    if ( g_isRecoveringLostDevice
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    3750,
                    0,
                    "%s",
                    "!g_isRecoveringLostDevice") )
    {
        __debugbreak();
    }
    semaphore = R_ReleaseDXDeviceOwnership();
    g_isRecoveringLostDevice = 1;
    while ( !g_mayRecoverLostAssets )
        NET_Sleep(0);
    if ( semaphore )
        R_AcquireDXDeviceOwnership(0);
}

void __cdecl DB_EndRecoverLostDevice()
{
    if ( !Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    3765,
                    0,
                    "%s",
                    "Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    if ( !g_isRecoveringLostDevice
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    3769,
                    0,
                    "%s",
                    "g_isRecoveringLostDevice") )
    {
        __debugbreak();
    }
    if ( !g_mayRecoverLostAssets
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    3770,
                    0,
                    "%s",
                    "g_mayRecoverLostAssets") )
    {
        __debugbreak();
    }
    g_mayRecoverLostAssets = !g_loadingZone;
    g_isRecoveringLostDevice = 0;
}

int __cdecl DB_GetZonePriority(int zone)
{
    int result; // eax

    if (zone > 0x2000)
    {
        if (zone > 0x200000)
        {
            if (zone > 0x2000000)
            {
                if (zone == 0x4000000)
                    return 58;
                if (zone == 0x10000000)
                    return 50;
            }
            else
            {
                switch (zone)
                {
                case 0x2000000:
                    return 8;
                case 0x400000:
                    return 62;
                case 0x800000:
                    return 3;
                case 0x1000000:
                    return 53;
                }
            }
        }
        else
        {
            if (zone == 0x200000)
                return 12;
            if (zone > 0x20000)
            {
                if (zone == 0x40000)
                    return 57;
                if (zone == 0x80000)
                    return 7;
            }
            else
            {
                switch (zone)
                {
                case 0x20000:
                    return 6;
                case 0x4000:
                    return 55;
                case 0x8000:
                    return 5;
                case 0x10000:
                    return 56;
                }
            }
        }
    LABEL_62:
        if (!Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp", 3857, 0, "%s", "0"))
            __debugbreak();
        return 0;
    }
    if (zone == 0x2000)
        return 4;
    if (zone > 64)
    {
        if (zone > 1024)
        {
            if (zone == 2048)
                return 10;
            if (zone == 4096)
                return 54;
        }
        else
        {
            switch (zone)
            {
            case 1024:
                return 60;
            case 128:
                return 11;
            case 256:
                return 52;
            case 512:
                return 2;
            }
        }
        goto LABEL_62;
    }
    if (zone == 64)
        return 61;
    switch (zone)
    {
    case 1:
        result = 51;
        break;
    case 2:
        result = 1;
        break;
    case 4:
        result = 63;
        break;
    case 8:
        result = 13;
        break;
    case 16:
        result = 59;
        break;
    case 32:
        result = 9;
        break;
    default:
        goto LABEL_62;
    }
    return result;
}

XAssetHeader __cdecl DB_AddXAsset(XAssetType type, XAssetHeader header)
{
    XAssetEntryPoolEntry *existingEntry; // [esp+0h] [ebp-14h]
    XAssetEntry newEntry; // [esp+4h] [ebp-10h] BYREF

    newEntry.asset.type = type;
    newEntry.asset.header = header;
    Sys_EnterCriticalSection(CRITSECT_DBHASH);
    existingEntry = DB_LinkXAssetEntry(&newEntry, 0);
    Sys_LeaveCriticalSection(CRITSECT_DBHASH);
    DB_SyncLostDevice();
    return existingEntry->entry.asset.header;
}

void DB_SyncLostDevice()
{
    if ( g_isRecoveringLostDevice )
    {
        if ( g_mayRecoverLostAssets
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                        3783,
                        0,
                        "%s",
                        "!g_mayRecoverLostAssets") )
        {
            __debugbreak();
        }
        g_mayRecoverLostAssets = 1;
        do
            NET_Sleep(0x19u);
        while ( g_isRecoveringLostDevice );
        if ( g_mayRecoverLostAssets
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                        3789,
                        0,
                        "%s",
                        "!g_mayRecoverLostAssets") )
        {
            __debugbreak();
        }
    }
}

XAssetEntryPoolEntry *__cdecl DB_LinkXAssetEntry(XAssetEntry *newEntry, int allowOverride)
{
    const char *String; // eax
    const char *XAssetTypeName; // eax
    int v4; // ecx
    const char *XAssetName; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    XAssetHeader v13; // edx
    const char *v14; // eax
    const char *v15; // eax
    const char *v16; // eax
    const char *v17; // eax
    const char *v18; // eax
    const char *v19; // eax
    const char *v20; // eax
    const char *v21; // eax
    const char *v22; // eax
    const char *v23; // eax
    const char *v24; // eax
    const char *v25; // [esp-8h] [ebp-40h]
    const char *v26; // [esp-8h] [ebp-40h]
    const char *v27; // [esp-8h] [ebp-40h]
    const char *v28; // [esp-4h] [ebp-3Ch]
    XZoneName *v29; // [esp-4h] [ebp-3Ch]
    const char *v30; // [esp-4h] [ebp-3Ch]
    XZoneName *v31; // [esp-4h] [ebp-3Ch]
    XZoneName *v32; // [esp+4h] [ebp-34h]
    XZoneName *v33; // [esp+8h] [ebp-30h]
    XAssetEntryPoolEntry *existingEntry; // [esp+Ch] [ebp-2Ch]
    XAssetEntryPoolEntry *existingEntrya; // [esp+Ch] [ebp-2Ch]
    unsigned int hash; // [esp+10h] [ebp-28h]
    unsigned int existingEntryIndex; // [esp+14h] [ebp-24h]
    unsigned int existingEntryIndexa; // [esp+14h] [ebp-24h]
    XAssetEntryPoolEntry *overrideAssetEntry; // [esp+18h] [ebp-20h]
    XAsset asset; // [esp+1Ch] [ebp-1Ch] BYREF
    int isStubAsset; // [esp+24h] [ebp-14h]
    const char *name; // [esp+28h] [ebp-10h]
    unsigned __int8 zoneIndex; // [esp+2Fh] [ebp-9h]
    XAssetType type; // [esp+30h] [ebp-8h]
    unsigned __int16 *pOverrideAssetEntryIndex; // [esp+34h] [ebp-4h]

    name = DB_GetXAssetName(&newEntry->asset);
    if ( db_xassetdebug->current.enabled )
    {
        if ( db_xassetdebugtype->current.integer == -1
            || db_xassetdebugtype->current.integer == newEntry->asset.type
            || *Dvar_GetString("db_xassetdebugname")
            && (String = Dvar_GetString("db_xassetdebugname"), !I_stricmp(name, String)) )
        {
            XAssetTypeName = DB_GetXAssetTypeName(newEntry->asset.type);
            Com_Printf(16, "***db_xassetdebug:***\nDB_LinkXAssetEntry: link asset: '%s','%s'\n", XAssetTypeName, name);
        }
    }
    v4 = *name;
    isStubAsset = v4 == 44;
    if ( v4 == 44 )
        ++name;
    type = newEntry->asset.type;
    hash = DB_HashForName(name, type);
    for ( existingEntryIndex = db_hashTable[hash % 0x8000];
                existingEntryIndex;
                existingEntryIndex = existingEntry->entry.nextHash )
    {
        existingEntry = &g_assetEntryPool[existingEntryIndex];
        if ( existingEntry->entry.asset.type == type )
        {
            XAssetName = DB_GetXAssetName(&existingEntry->entry.asset);
            if ( hash == DB_HashForName(XAssetName, type) )
            {
                v6 = DB_GetXAssetName(&existingEntry->entry.asset);
                if ( I_stricmp(v6, name) )
                    break;
            }
        }
    }
    existingEntrya = 0;
    for ( existingEntryIndexa = db_hashTable[hash % 0x8000];
                existingEntryIndexa;
                existingEntryIndexa = existingEntrya->entry.nextHash )
    {
        existingEntrya = &g_assetEntryPool[existingEntryIndexa];
        if ( existingEntrya->entry.asset.type == type )
        {
            v7 = DB_GetXAssetName(&existingEntrya->entry.asset);
            if ( !I_stricmp(v7, name) )
            {
                if ( db_xassetdebug->current.enabled )
                {
                    if ( db_xassetdebugtype->current.integer == -1
                        || db_xassetdebugtype->current.integer == newEntry->asset.type
                        || *Dvar_GetString("db_xassetdebugname")
                        && (v8 = Dvar_GetString("db_xassetdebugname"), !I_stricmp(name, v8)) )
                    {
                        if ( g_zoneNames[existingEntrya->entry.zoneIndex].name[0] )
                            v33 = &g_zoneNames[existingEntrya->entry.zoneIndex];
                        else
                            v33 = (XZoneName *)"default asset pool";
                        v9 = DB_GetXAssetTypeName(newEntry->asset.type);
                        Com_Printf(
                            16,
                            "DB_LinkXAssetEntry: existing asset: '%s','%s' loaded from fastfile: '%s'\n",
                            v9,
                            name,
                            v33->name);
                    }
                }
                break;
            }
        }
    }
    if ( allowOverride )
    {
        if ( isStubAsset
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                        3960,
                        0,
                        "%s",
                        "!isStubAsset") )
        {
            __debugbreak();
        }
    }
    else
    {
        if ( isStubAsset )
        {
            if ( !existingEntryIndexa )
                return (XAssetEntryPoolEntry *)DB_CreateDefaultEntry(type, name);
            if ( !existingEntrya
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                            3940,
                            0,
                            "%s",
                            "existingEntry") )
            {
                __debugbreak();
            }
            if ( db_xassetdebug->current.enabled )
            {
                if ( db_xassetdebugtype->current.integer == -1
                    || db_xassetdebugtype->current.integer == newEntry->asset.type
                    || *Dvar_GetString("db_xassetdebugname")
                    && (v10 = Dvar_GetString("db_xassetdebugname"), !I_stricmp(name, v10)) )
                {
                    v11 = DB_GetXAssetTypeName(newEntry->asset.type);
                    Com_Printf(16, "DB_LinkXAssetEntry: stub asset: '%s','%s' already exists. Using existingEntry\n", v11, name);
                }
            }
            return existingEntrya;
        }
        //v13.xmodelPieces = (XModelPieces *)newEntry->asset.header;
        v13.xmodelPieces = (XModelPieces *)newEntry->asset.header.xmodelPieces;
        asset.type = newEntry->asset.type;
        asset.header = v13;
        newEntry = &DB_AllocXAssetEntry(asset.type, g_zoneIndex)->entry;
        DB_CloneXAssetInternal(&asset, &newEntry->asset);
    }
    if ( !existingEntryIndexa )
    {
        if ( db_xassetdebug->current.enabled )
        {
            if ( db_xassetdebugtype->current.integer == -1
                || db_xassetdebugtype->current.integer == newEntry->asset.type
                || *Dvar_GetString("db_xassetdebugname") && (v14 = Dvar_GetString("db_xassetdebugname"), !I_stricmp(name, v14)) )
            {
                v28 = name;
                v15 = DB_GetXAssetTypeName(newEntry->asset.type);
                Com_Printf(16, "DB_LinkXAssetEntry: created new asset: '%s','%s'\n", v15, v28);
            }
        }
        newEntry->nextHash = db_hashTable[hash % 0x8000];
        db_hashTable[hash % 0x8000] = ((char *)newEntry - (char *)g_assetEntryPool) >> 4;
        if ( db_xassetdebug->current.enabled )
        {
            if ( db_xassetdebugtype->current.integer == -1
                || db_xassetdebugtype->current.integer == newEntry->asset.type
                || *Dvar_GetString("db_xassetdebugname") && (v16 = Dvar_GetString("db_xassetdebugname"), !I_stricmp(name, v16)) )
            {
                v29 = &g_zoneNames[newEntry->zoneIndex];
                v25 = name;
                v17 = DB_GetXAssetTypeName(newEntry->asset.type);
                Com_Printf(
                    16,
                    "DB_LinkXAssetEntry: return new asset: '%s','%s' loaded from fastfile: '%s'\n",
                    v17,
                    v25,
                    v29->name);
            }
        }
        return (XAssetEntryPoolEntry *)newEntry;
    }
    if ( !existingEntrya
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp", 3981, 0, "%s", "existingEntry") )
    {
        __debugbreak();
    }
    if ( existingEntrya->entry.zoneIndex )
    {
        if ( existingEntrya->entry.zoneIndex == newEntry->zoneIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                        3986,
                        0,
                        "%s",
                        "existingEntry->zoneIndex != newEntry->zoneIndex") )
        {
            __debugbreak();
        }
        if ( !*g_defaultAssetName[type] && type != ASSET_TYPE_RAWFILE && type != ASSET_TYPE_MAP_ENTS )
        {
            Sys_LeaveCriticalSection(CRITSECT_DBHASH);
            Com_Error(
                ERR_DROP,
                "Attempting to override asset '%s' from zone '%s' with zone '%s'",
                name,
                g_zoneNames[existingEntrya->entry.zoneIndex].name,
                g_zoneNames[newEntry->zoneIndex].name);
        }
        if ( !DB_OverrideAsset(newEntry->zoneIndex, existingEntrya->entry.zoneIndex) )
        {
            for ( pOverrideAssetEntryIndex = &existingEntrya->entry.nextOverride;
                        *pOverrideAssetEntryIndex;
                        pOverrideAssetEntryIndex = &overrideAssetEntry->entry.nextOverride )
            {
                overrideAssetEntry = &g_assetEntryPool[*pOverrideAssetEntryIndex];
                if ( DB_OverrideAsset(newEntry->zoneIndex, overrideAssetEntry->entry.zoneIndex) )
                    break;
            }
            if ( db_xassetdebug->current.enabled )
            {
                if ( db_xassetdebugtype->current.integer == -1
                    || db_xassetdebugtype->current.integer == newEntry->asset.type
                    || *Dvar_GetString("db_xassetdebugname")
                    && (v18 = Dvar_GetString("db_xassetdebugname"), !I_stricmp(name, v18)) )
                {
                    Com_Printf(16, "DB_LinkXAssetEntry: keep existing asset, and put new in its override position\n");
                }
            }
            newEntry->nextOverride = *pOverrideAssetEntryIndex;
            *pOverrideAssetEntryIndex = ((char *)newEntry - (char *)g_assetEntryPool) >> 4;
            return existingEntrya;
        }
        goto LABEL_106;
    }
    if ( !*g_defaultAssetName[type]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    4033,
                    0,
                    "%s",
                    "g_defaultAssetName[type][0]") )
    {
        __debugbreak();
    }
    if ( existingEntrya->entry.nextOverride
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    4034,
                    0,
                    "%s",
                    "!existingEntry->nextOverride") )
    {
        __debugbreak();
    }
    if ( !g_defaultAssetCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    4035,
                    0,
                    "%s",
                    "g_defaultAssetCount") )
    {
        __debugbreak();
    }
    if ( !allowOverride )
    {
LABEL_106:
        if ( allowOverride )
        {
            if ( !existingEntrya->entry.zoneIndex
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                            4061,
                            0,
                            "%s",
                            "existingEntry->zoneIndex") )
            {
                __debugbreak();
            }
            if ( existingEntrya->entry.inuse )
            {
                varXAsset = &existingEntrya->entry.asset;
                Mark_XAsset();
            }
            if ( db_xassetdebug->current.enabled )
            {
                if ( db_xassetdebugtype->current.integer == -1
                    || db_xassetdebugtype->current.integer == newEntry->asset.type
                    || *Dvar_GetString("db_xassetdebugname")
                    && (v21 = Dvar_GetString("db_xassetdebugname"), !I_stricmp(name, v21)) )
                {
                    v31 = &g_zoneNames[newEntry->zoneIndex];
                    v26 = name;
                    v22 = DB_GetXAssetTypeName(newEntry->asset.type);
                    Com_Printf(
                        16,
                        "DB_LinkXAssetEntry: swapping existing asset: '%s','%s' with new asset loaded from fastfile: '%s'\n",
                        v22,
                        v26,
                        v31->name);
                }
            }
            newEntry->nextOverride = existingEntrya->entry.nextOverride;
            existingEntrya->entry.nextOverride = ((char *)newEntry - (char *)g_assetEntryPool) >> 4;
            DB_SwapXAsset(&newEntry->asset, &existingEntrya->entry.asset);
            zoneIndex = existingEntrya->entry.zoneIndex;
            existingEntrya->entry.zoneIndex = newEntry->zoneIndex;
            newEntry->zoneIndex = zoneIndex;
        }
        else
        {
            DB_DelayedCloneXAsset(newEntry);
        }
        if ( db_xassetdebug->current.enabled )
        {
            if ( db_xassetdebugtype->current.integer == -1
                || db_xassetdebugtype->current.integer == existingEntrya->entry.asset.type
                || *Dvar_GetString("db_xassetdebugname") && (v23 = Dvar_GetString("db_xassetdebugname"), !I_stricmp(name, v23)) )
            {
                if ( g_zoneNames[existingEntrya->entry.zoneIndex].name[0] )
                    v32 = &g_zoneNames[existingEntrya->entry.zoneIndex];
                else
                    v32 = (XZoneName *)"default asset pool";
                v27 = name;
                v24 = DB_GetXAssetTypeName(existingEntrya->entry.asset.type);
                Com_Printf(16, "DB_LinkXAssetEntry: return asset: '%s','%s' loaded from fastfile: '%s'\n", v24, v27, v32->name);
            }
        }
        return existingEntrya;
    }
    if ( db_xassetdebug->current.enabled )
    {
        if ( db_xassetdebugtype->current.integer == -1
            || db_xassetdebugtype->current.integer == newEntry->asset.type
            || *Dvar_GetString("db_xassetdebugname") && (v19 = Dvar_GetString("db_xassetdebugname"), !I_stricmp(name, v19)) )
        {
            v30 = name;
            v20 = DB_GetXAssetTypeName(newEntry->asset.type);
            Com_Printf(16, "DB_LinkXAssetEntry: replacing default asset in asset: '%s','%s'\n", v20, v30);
        }
    }
    --g_defaultAssetCount;
    if ( existingEntrya->entry.inuse )
    {
        varXAsset = &existingEntrya->entry.asset;
        Mark_XAsset();
    }
    DB_CloneXAssetEntry(newEntry, &existingEntrya->entry, DB_CLONE_FROM_DEFAULT);
    DB_FreeXAssetEntry((XAssetEntryPoolEntry*)newEntry);
    return existingEntrya;
}

void __cdecl DB_FreeXAssetEntry(XAssetEntryPoolEntry *assetEntry)
{
    DB_FreeXAssetHeader(assetEntry->entry.asset.type, assetEntry->entry.asset.header);

    --XAssetEntryHighCount;

    XAssetEntryPoolEntry *oldFreeHead = g_freeAssetEntryHead;
    g_freeAssetEntryHead = assetEntry;
    assetEntry->next = oldFreeHead;
}

void __cdecl DB_CloneXAssetEntry(const XAssetEntry *from, XAssetEntry *to, DBCloneMethod cloneMethod)
{
    DB_CloneXAsset(&from->asset, &to->asset, cloneMethod);
    to->zoneIndex = from->zoneIndex;
}

void __cdecl DB_CloneXAsset(const XAsset *from, XAsset *to, DBCloneMethod cloneMethod)
{
    if ( from->type != to->type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    3069,
                    0,
                    "%s",
                    "from->type == to->type") )
    {
        __debugbreak();
    }
    DB_DynamicCloneXAsset(from->header, to->header, to->type, cloneMethod);
    DB_CloneXAssetInternal(from, to);
}

char __cdecl DB_DynamicCloneXAsset(XAssetHeader from, XAssetHeader to, XAssetType type, DBCloneMethod cloneMethod)
{
    if ( !DB_DynamicCloneXAssetHandler[type] )
        return 0;
    //((void (__cdecl *)(unsigned int, unsigned int, unsigned int))DB_DynamicCloneXAssetHandler[type])((const XAssetHeader)from.xmodelPieces, (XAssetHeader)to.xmodelPieces, cloneMethod);
    DB_DynamicCloneXAssetHandler[type]((const XAssetHeader)from.xmodelPieces, (XAssetHeader)to.xmodelPieces, cloneMethod);
    return 1;
}

void __cdecl DB_SwapXAsset(XAsset *from, XAsset *to)
{
    XAsset asset; // [esp+8h] [ebp-Ch] BYREF

    iassert(from->type == to->type);
    DB_DynamicCloneXAsset(from->header, to->header, to->type, DB_CLONE_SWAP);
    unsigned int size = DB_GetXAssetTypeSize(from->type);
    asset.header.data = alloca(size);
    asset.type = from->type;
    DB_CloneXAssetInternal(to, &asset);
    DB_CloneXAssetInternal(from, to);
    DB_CloneXAssetInternal(&asset, from);
}

void __cdecl DB_DelayedCloneXAsset(XAssetEntry *newEntry)
{
    const char *v1; // eax
    const char *XAssetTypeName; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // [esp-8h] [ebp-Ch]
    const char *String; // [esp-4h] [ebp-8h]
    const char *XAssetName; // [esp-4h] [ebp-8h]
    const char *v9; // [esp-4h] [ebp-8h]
    const char *v10; // [esp-4h] [ebp-8h]
    XZoneName *v11; // [esp-4h] [ebp-8h]
    unsigned int i; // [esp+0h] [ebp-4h]

    if ( g_sync )
    {
        if ( db_xassetdebug->current.enabled )
        {
            if ( db_xassetdebugtype->current.integer == -1
                || db_xassetdebugtype->current.integer == newEntry->asset.type
                || *Dvar_GetString("db_xassetdebugname")
                && (String = Dvar_GetString("db_xassetdebugname"),
                        v1 = DB_GetXAssetName(&newEntry->asset),
                        !I_stricmp(v1, String)) )
            {
                XAssetName = DB_GetXAssetName(&newEntry->asset);
                XAssetTypeName = DB_GetXAssetTypeName(newEntry->asset.type);
                Com_Printf(16, "DB_DelayedCloneXAsset: g_sync forced load asset: '%s','%s'\n", XAssetTypeName, XAssetName);
            }
        }
        DB_LinkXAssetEntry(newEntry, 1);
    }
    else
    {
        if ( g_copyInfoCount >= 0xC00 )
        {
            Com_Printf(0, "g_copyInfo exceeded: too many asset overrides in one call to DB_LoadXAssets.\n");
            for ( i = 0; i < 0xC00; ++i )
            {
                v9 = DB_GetXAssetName(&g_copyInfo[i]->asset);
                v3 = DB_GetXAssetTypeName(g_copyInfo[i]->asset.type);
                Com_Printf(0, "'%s','%s'\n", v3, v9);
            }
            Sys_Error((char*)"g_copyInfo exceeded");
        }
        if ( db_xassetdebug->current.enabled )
        {
            if ( db_xassetdebugtype->current.integer == -1
                || db_xassetdebugtype->current.integer == newEntry->asset.type
                || *Dvar_GetString("db_xassetdebugname")
                && (v10 = Dvar_GetString("db_xassetdebugname"), v4 = DB_GetXAssetName(&newEntry->asset), !I_stricmp(v4, v10)) )
            {
                v11 = &g_zoneNames[newEntry->zoneIndex];
                v6 = DB_GetXAssetName(&newEntry->asset);
                v5 = DB_GetXAssetTypeName(newEntry->asset.type);
                Com_Printf(16, "DB_DelayedCloneXAsset: postponed load asset: '%s','%s' from fastfile %s\n", v5, v6, v11->name);
            }
        }
        g_copyInfo[g_copyInfoCount++] = newEntry;
    }
}

bool __cdecl DB_OverrideAsset(unsigned int newZoneIndex, unsigned int existingZoneIndex)
{
    int newPriority; // [esp+0h] [ebp-8h]

    if ( !newZoneIndex
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp", 3865, 0, "%s", "newZoneIndex") )
    {
        __debugbreak();
    }
    if ( !existingZoneIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    3866,
                    0,
                    "%s",
                    "existingZoneIndex") )
    {
        __debugbreak();
    }
    newPriority = DB_GetZonePriority(g_zoneNames[newZoneIndex].flags & 0x17FFFFFF);
    return newPriority >= DB_GetZonePriority(g_zoneNames[existingZoneIndex].flags & 0x17FFFFFF);
}

void __cdecl DB_GetXAsset(XAssetType type, XAssetHeader header)
{
    unsigned int assetEntryIndex; // [esp+4h] [ebp-14h]
    XAsset asset; // [esp+8h] [ebp-10h] BYREF
    const char *name; // [esp+10h] [ebp-8h]
    XAssetEntry *assetEntry; // [esp+14h] [ebp-4h]

    if ( s_inuseCacheEnabled
        && s_inuseCache[((unsigned int)header.xmodelPieces >> 2) & 0x1FFF] == HIWORD(header.physPreset) )
    {
        ++s_inuseCacheHits;
    }
    else
    {
        ++s_inuseCacheMisses;
        asset.type = type;
        asset.header = header;
        name = DB_GetXAssetName(&asset);
        for ( assetEntryIndex = db_hashTable[DB_HashForName(name, type) % 0x8000u];
                    assetEntryIndex;
                    assetEntryIndex = assetEntry->nextHash )
        {
            assetEntry = &g_assetEntryPool[assetEntryIndex].entry;
            if ( assetEntry->asset.type == type && assetEntry->asset.header.xmodelPieces == header.xmodelPieces )
            {
                assetEntry->inuse = 1;
                if ( s_inuseCacheEnabled )
                    s_inuseCache[((unsigned int)header.xmodelPieces >> 2) & 0x1FFF] = HIWORD(header.physPreset);
                return;
            }
        }
    }
}

void __cdecl DB_BuildOSPath(const char *zoneName, const char *ext, unsigned int size, char *filename)
{
    unsigned int CurrentLanguage; // eax
    char *v5; // eax
    const char *Language; // [esp+0h] [ebp-Ch]
    const char *languageName; // [esp+8h] [ebp-4h]

    if ( loc_language )
    {
        CurrentLanguage = SEH_GetCurrentLanguage();
        Language = SEH_GetLanguageName(CurrentLanguage);
    }
    else
    {
        Language = Win_GetLanguage();
    }
    languageName = Language;
    if ( DB_IsUsingGermanPaths() )
        languageName = SEH_GetLanguageName(3u);
    v5 = Sys_DefaultInstallPath();
    Com_sprintf(filename, size, "%s\\zone\\%s\\%s%s", v5, languageName, zoneName, ext);
}

char __cdecl DB_IsUsingGermanPaths()
{
    const char *LanguageName; // eax
    const char *v2; // eax
    char *lang; // [esp+0h] [ebp-4h]

    if ( loc_language )
    {
        if ( loc_language->current.integer == 4 || loc_language->current.integer == 3 )
            return 1;
    }
    else
    {
        lang = Win_GetLanguage();
        LanguageName = SEH_GetLanguageName(4u);
        if ( !I_stricmp(lang, LanguageName) )
            return 1;
        v2 = SEH_GetLanguageName(3u);
        if ( !I_stricmp(lang, v2) )
            return 1;
    }
    return 0;
}

void __cdecl DB_BuildOSPath_Unlocalized(const char *zoneName, const char *ext, unsigned int size, char *filename)
{
    char *v4; // eax

    v4 = Sys_DefaultInstallPath();
    Com_sprintf(filename, size, "%s\\zone\\Common\\%s%s", v4, zoneName, ext);
}

void DB_PostLoadXZone()
{
    //jpeg_decompress_struct *v0; // ecx
    const char *XAssetTypeName; // eax
    const char *XAssetName; // [esp-4h] [ebp-Ch]
    unsigned int i; // [esp+0h] [ebp-8h]
    int remoteScreenUpdateNesting; // [esp+4h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    4457,
                    0,
                    "%s",
                    "Sys_IsMainThread() || Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    if ( g_loadingZone
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    4464,
                    0,
                    "%s",
                    "!g_loadingZone") )
    {
        __debugbreak();
    }
    if ( g_zoneInfoCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    4465,
                    0,
                    "%s",
                    "!g_zoneInfoCount") )
    {
        __debugbreak();
    }
    if ( !Sys_IsDatabaseReady2() )
    {
        if ( g_copyInfoCount )
        {
            remoteScreenUpdateNesting = 0;
            if ( !Sys_IsMainThread()
                || (++g_mainThreadBlocked,
                        remoteScreenUpdateNesting = R_PopRemoteScreenUpdate(),
                        --g_mainThreadBlocked,
                        g_copyInfoCount) )
            {
                DB_ArchiveAssets();
                Sys_EnterCriticalSection(CRITSECT_DBHASH);
                for ( i = 0; i < g_copyInfoCount; ++i )
                {
                    XAssetName = DB_GetXAssetName(&g_copyInfo[i]->asset);
                    XAssetTypeName = DB_GetXAssetTypeName(g_copyInfo[i]->asset.type);
                    Com_Printf(16, "Redundant asset: '%s','%s'\n", XAssetTypeName, XAssetName);
                    DB_LinkXAssetEntry(g_copyInfo[i], 1);
                }
                g_copyInfoCount = 0;
                Sys_LeaveCriticalSection(CRITSECT_DBHASH);
                DB_StreamCompletePreload();
                DB_UnarchiveAssets();
                if ( Sys_IsMainThread() )
                    R_PushRemoteScreenUpdate(remoteScreenUpdateNesting);
                Sys_DatabaseCompleted2();
            }
            else
            {
                R_PushRemoteScreenUpdate(remoteScreenUpdateNesting);
            }
        }
        else
        {
            DB_StreamCompletePreload();
            DB_ExternalInitAssets();
            Sys_DatabaseCompleted2();
        }
    }
}

void __cdecl DB_UpdateDebugZone()
{
    XZoneInfo zoneInfo[2]; // [esp+0h] [ebp-18h] BYREF

    if ( g_debugZoneName[0] )
    {
        zoneInfo[0].name = 0;
        zoneInfo[1].name = g_debugZoneName;
        Com_SyncThreads();
        zoneInfo[0].allocFlags = 0;
        zoneInfo[0].freeFlags = 0x400000;
        zoneInfo[1].allocFlags = 0x400000;
        zoneInfo[1].freeFlags = 0;
        DB_LoadXAssets(zoneInfo, 2u, 1);
        CG_VisionSetMyChanges();
        if ( !CL_AllLocalClientsDisconnected() )
            CG_WeaponMyChanges();
        G_ReloadScrVehicleInfo();
    }
}

void __cdecl DB_SyncXAssets()
{
    if ( !Sys_IsMainThread()
        && !Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    4574,
                    0,
                    "%s",
                    "Sys_IsMainThread() || Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    R_BeginRemoteScreenUpdate();
    Sys_SyncDatabase();
    R_EndRemoteScreenUpdate(0);
    SocketRouter_EmergencyFrame();
    DB_PostLoadXZone();
}

bool __cdecl DB_IsZoneLoaded(const char *name)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for ( i = 1; i < 0x21; ++i )
    {
        if ( g_zoneNames[i].name[0] && !I_stricmp(g_zoneNames[i].name, name) )
            return g_zoneNames[i].loaded;
    }
    return 0;
}

char __cdecl DB_IsZoneTypeLoaded(int zoneType)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for ( i = 1; i < 0x21; ++i )
    {
        if ( g_zoneNames[i].name[0] && g_zoneNames[i].flags == zoneType && g_zoneNames[i].loaded )
            return 1;
    }
    return 0;
}


cmd_function_s DB_LoadZone_f_VAR;
cmd_function_s DB_ListDefaultEntries_f_VAR;
cmd_function_s DB_ListAssetPool_f_VAR;
cmd_function_s DB_DumpMaterialList_f_VAR;
void __cdecl DB_LoadXAssets(XZoneInfo *zoneInfo, unsigned int zoneCount, int sync)
{
    unsigned int v3; // ecx
    int v4; // edx
    int v5; // eax
    //jpeg_decompress_struct *v6; // ecx
    char *v7; // eax
    const char *LanguageNameAbbr; // eax
    XZoneInfo *v9; // eax
    XZoneInfo *v10; // edx
    const char *name; // [esp-4h] [ebp-32Ch]
    int allocFlags; // [esp+0h] [ebp-328h]
    int language; // [esp+4h] [ebp-324h] BYREF
    unsigned int k; // [esp+8h] [ebp-320h]
    int maxZones; // [esp+Ch] [ebp-31Ch]
    XZoneInfo locZoneInfo[10]; // [esp+10h] [ebp-318h] BYREF
    char zoneName[10][64]; // [esp+88h] [ebp-2A0h] BYREF
    unsigned int count; // [esp+310h] [ebp-18h]
    XZone *zone; // [esp+314h] [ebp-14h]
    unsigned int j; // [esp+318h] [ebp-10h]
    bool unloadedZone; // [esp+31Fh] [ebp-9h]
    int i; // [esp+320h] [ebp-8h]
    int zoneFreeFlags; // [esp+324h] [ebp-4h]

    iassert(Sys_IsMainThread() || Sys_IsRenderThread());
    iassert(zoneCount);

    if ( !g_zoneInited )
    {
        g_zoneInited = 1;
        DB_Init();
        Cmd_AddCommandInternal("loadzone", DB_LoadZone_f, &DB_LoadZone_f_VAR);
        Cmd_AddCommandInternal("listdefaultassets", DB_ListDefaultEntries_f, &DB_ListDefaultEntries_f_VAR);
        Cmd_AddCommandInternal("listassetpool", DB_ListAssetPool_f, &DB_ListAssetPool_f_VAR);
        Cmd_AddCommandInternal("dumpmateriallist", DB_DumpMaterialList_f, &DB_DumpMaterialList_f_VAR);
    }

    if ( sync )
        R_StreamPushSyncDisable();

    unloadedZone = 0;
    DB_SyncXAssets();
    v3 = g_archiveBuf;

    iassert(!g_archiveBuf);

    for ( j = 0; j < zoneCount; ++j )
    {
        if ( (zoneInfo[j].allocFlags & 0x40000000) == 0 )
        {
            if ( zoneInfo[j].allocFlags && zoneInfo[j].freeFlags >= 0 )
            {
                if ( zoneInfo[j].allocFlags <= 0x100000 )
                    v4 = zoneInfo[j].allocFlags | ~(zoneInfo[j].allocFlags - 1) & 0xFFFFF;
                else
                    v4 = zoneInfo[j].allocFlags | (zoneInfo[j].allocFlags - 1) & 0x7E00000;
                zoneInfo[j].freeFlags = v4;
            }
            else if ( !zoneInfo[j].allocFlags && zoneInfo[j].freeFlags >= 0 )
            {
                if ( zoneInfo[j].freeFlags <= 0x100000 )
                    v5 = zoneInfo[j].freeFlags | ~(zoneInfo[j].freeFlags - 1) & 0xFFFFF;
                else
                    v5 = zoneInfo[j].freeFlags | (zoneInfo[j].freeFlags - 1) & 0x7E00000;
                zoneInfo[j].freeFlags = v5;
            }
            zoneFreeFlags = zoneInfo[j].freeFlags & 0x7FFFFFFF;
            for ( i = g_zoneCount - 1; i >= 0; --i )
            {
                zone = &g_zones[i];
                if ( (zoneFreeFlags & zone->flags) != 0 )
                {
                    if ( !unloadedZone )
                    {
                        unloadedZone = 1;
                        DB_SyncExternalAssets();
                        DB_ArchiveAssets();
                        Sys_EnterCriticalSection(CRITSECT_DBHASH);
                    }
                    DB_UnloadXZone(zone, 1);
                }
            }
        }
        v3 = j + 1;
    }
    if ( unloadedZone )
    {
        DB_FreeUnusedResources();
        for ( j = 0; j < zoneCount; ++j )
        {
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 0x40000);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 0x80000);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 0x10000);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 0x20000);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 0x4000);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 0x8000);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 4096);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 0x2000);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 256);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 512);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 1024);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 2048);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 64);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 128);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 16);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 32);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 4);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 8);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 2);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 0x400000);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 0x200000);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 0x1000000);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 0x800000);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 0x4000000);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 0x2000000);
            DB_UnloadXAssetsMemoryForZone(zoneInfo[j].freeFlags, 0x10000000);
        }
        Sys_LeaveCriticalSection(CRITSECT_DBHASH);
        DB_UnarchiveAssets();
    }

    if ( sync )
        DB_ArchiveAssets();

    maxZones = 10;
    if ( 2 * zoneCount >= 0xA
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    4783,
                    0,
                    "%s",
                    "zoneCount*2<maxZones") )
    {
        __debugbreak();
    }
    count = 0;
    for ( k = 0; k < zoneCount; ++k )
    {
        allocFlags = zoneInfo[k].allocFlags;
        if ( allocFlags > 4096 )
        {
            if ( allocFlags > 0x400000 )
            {
                if (allocFlags == 0x1000000)
                {
                    locZoneInfo[count].allocFlags = 0x800000;
                    goto LABEL_85;
                }
                if ( allocFlags == 0x4000000 )
                {
                    locZoneInfo[count].allocFlags = 0x2000000;
                    goto LABEL_85;
                }
            }
            else
            {
                switch ( allocFlags )
                {
                    case 0x400000:
                        locZoneInfo[count].allocFlags = 0x200000;
                        goto LABEL_85;
                    case 0x4000:
                        locZoneInfo[count].allocFlags = 0x8000;
                        goto LABEL_85;
                    case 0x10000:
                        locZoneInfo[count].allocFlags = 0x20000;
                        goto LABEL_85;
                    case 0x40000:
                        locZoneInfo[count].allocFlags = 0x80000;
                        goto LABEL_85;
                }
            }
        }
        else
        {
            if ( allocFlags == 4096 )
            {
                locZoneInfo[count].allocFlags = 0x2000;
                goto LABEL_85;
            }
            if ( allocFlags > 64 )
            {
                if ( allocFlags == 256 )
                {
                    locZoneInfo[count].allocFlags = 512;
                    goto LABEL_85;
                }
                if ( allocFlags == 1024 )
                {
                    locZoneInfo[count].allocFlags = 2048;
                    goto LABEL_85;
                }
            }
            else
            {
                switch ( allocFlags )
                {
                    case 64:
                        locZoneInfo[count].allocFlags = 128;
                        goto LABEL_85;
                    case 1:
                        locZoneInfo[count].allocFlags = 2;
LABEL_85:
                        v7 = Win_GetLanguage();
                        SEH_GetLanguageIndexForName(v7, &language);
                        name = zoneInfo[k].name;
                        LanguageNameAbbr = SEH_GetLanguageNameAbbr(language);
                        Com_sprintf(zoneName[count], 0x40u, "%s%s", LanguageNameAbbr, name);
                        locZoneInfo[count].name = zoneName[count];
                        locZoneInfo[count++].freeFlags = 0;
                        v9 = &zoneInfo[k];
                        v10 = &locZoneInfo[count];
                        v10->name = v9->name;
                        v10->allocFlags = v9->allocFlags;
                        v10->freeFlags = v9->freeFlags;
                        ++count;
                        continue;
                    case 4:
                        locZoneInfo[count].allocFlags = 8;
                        goto LABEL_85;
                    case 16:
                        locZoneInfo[count].allocFlags = 32;
                        goto LABEL_85;
                }
            }
        }
    }
    DB_LoadXZone(locZoneInfo, count);
    if ( sync )
    {
        if ( g_copyInfoCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                        4858,
                        0,
                        "%s",
                        "!g_copyInfoCount") )
        {
            __debugbreak();
        }
        Sys_SyncDatabase();
        DB_UnarchiveAssets();
    }
    if ( sync )
        R_StreamPopSyncDisable();
}

template <typename T>
void __cdecl DB_InitPool(void *arg, int32_t size)
{
    //XAssetPool<RawFile, POOLSIZE_RAWFILE> *pool = (XAssetPool<RawFile, POOLSIZE_RAWFILE>*)arg;
    T *pool = (T *)arg;
    pool->freeHead = &pool->entries[0];
    for (int32_t i = 0; i < size - 1; i++)
    {
        pool->entries[i].next = &pool->entries[i + 1];
    }
    pool->entries[size - 1].next = NULL;
}

void(__cdecl *DB_InitPoolHeaderHandler[43])(void *, int) =
{
  DB_InitPool<XAssetPool<XModelPieces, POOLSIZE_XMODELPIECES>>,
  DB_InitPool<XAssetPool<PhysPreset, POOLSIZE_PHYSPRESET>>,
  DB_InitPool<XAssetPool<PhysConstraints, POOLSIZE_PHYSCONSTRAINTS>>,
  DB_InitPool<XAssetPool<DestructibleDef, POOLSIZE_DESTRUCTIBLEDEF>>,
  DB_InitPool<XAssetPool<XAnimParts, POOLSIZE_XANIM>>,
  DB_InitPool<XAssetPool<XModel, POOLSIZE_XMODEL>>,
  DB_InitPool<XAssetPool<Material, POOLSIZE_MATERIAL>>,
  DB_InitPool<XAssetPool<MaterialTechniqueSet, POOLSIZE_TECHSET>>,
  DB_InitPool<XAssetPool<GfxImage, POOLSIZE_IMAGE>>, // image
  DB_InitPool<XAssetPool<SndBank, POOLSIZE_SOUND>>, // sound // KISAKTODO: HACK 
  DB_InitPool<XAssetPool<snd_alias_list_t, POOLSIZE_SOUND_PATCH>>, // sound_patch
  DB_InitSingleton, // col_map_sp
  DB_InitSingleton,
  DB_InitSingleton,
  DB_InitSingleton,
  DB_InitSingleton,
  DB_InitPool<XAssetPool<MapEnts, POOLSIZE_MAP_ENTS>>, // map_ents
  DB_InitSingleton, // gfx_map
  DB_InitPool<XAssetPool<GfxLightDef, POOLSIZE_LIGHTDEF>>, // lightdef
  NULL, // ui_map
  DB_InitPool<XAssetPool<Font_s, POOLSIZE_FONT>>, // font
  DB_InitPool<XAssetPool<MenuList, POOLSIZE_MENUFILE>>, // menufile
  DB_InitPool<XAssetPool<menuDef_t, POOLSIZE_MENU>>, // menu
  DB_InitPool<XAssetPool<LocalizeEntry, POOLSIZE_LOCALIZE>>, // localize
  DB_InitPool<XAssetPool<WeaponVariantDef, POOLSIZE_WEAPON>>, // weapon
  NULL, // weapondef
  NULL, // weaponvariant
  DB_InitPool<XAssetPool<SndDriverGlobals, POOLSIZE_SNDDRIVERGLOBALS>>, // snddriverglobals
  DB_InitPool<XAssetPool<FxEffectDef, POOLSIZE_FX>>, // fx
  DB_InitPool<XAssetPool<FxImpactTable, POOLSIZE_IMPACTFX>>, // impactfx
  NULL, // aitype
  NULL, // mptype
  NULL, // mpbody
  NULL, // mphead
  NULL, // character
  NULL, // xmodelalias
  DB_InitPool<XAssetPool<RawFile, POOLSIZE_RAWFILE>>, // rawfile
  DB_InitPool<XAssetPool<StringTable, POOLSIZE_STRINGTABLE>>, // stringtable
  DB_InitPool<XAssetPool<PackIndex, POOLSIZE_PACKINDEX>>, // packindex
  DB_InitPool<XAssetPool<XGlobals, POOLSIZE_XGLOBALS>>, // xglobals
  DB_InitPool<XAssetPool<ddlDef_t, POOLSIZE_DDL>>, // ddl
  DB_InitPool<XAssetPool<Glasses, POOLSIZE_GLASSES>>, // glasses
  DB_InitPool<XAssetPool<EmblemSet, POOLSIZE_EMBLEMSET>>//emblemset
};



void DB_Init()
{
    XAssetType type; // [esp+0h] [ebp-8h]
    unsigned int i; // [esp+4h] [ebp-4h]

    db_xassetdebug = _Dvar_RegisterBool("db_xassetdebug", 0, 0, "debug assets loading");
    db_xassetdebugtype = _Dvar_RegisterInt(
        "db_xassetdebugtype",
        -1,
        -2,
        43,
        0,
        "debug assets loading type: -1 all; indexes start at 0");

    for (type = ASSET_TYPE_XMODELPIECES; type < ASSET_TYPE_COUNT; ++type)
        DB_InitPoolHeader(type);

    g_freeAssetEntryHead = g_assetEntryPool + 16;

    for (i = 1; i < 0x7FFF; ++i)
    {
        g_assetEntryPool[i].next = &g_assetEntryPool[i + 1];
        //g_assetEntryPool[i].entry.asset.type = g_assetEntryPool[i + 1].entry.asset.type;
    }

    g_assetEntryPool[0x7FFF].next = NULL;
    //g_assetEntryPool[0x7FFF].entry.asset.type = ASSET_TYPE_XMODELPIECES;
}

void __cdecl DB_InitPoolHeader(XAssetType type)
{
    if ( DB_XAssetPool[type] )
        DB_InitPoolHeaderHandler[type](DB_XAssetPool[type], g_poolSize[type]);
}

void __cdecl DB_ListDefaultEntries_f()
{
    const char *XAssetTypeName; // eax
    const char *XAssetName; // [esp-4h] [ebp-1Ch]
    unsigned int nextAssetEntryIndex; // [esp+0h] [ebp-18h]
    unsigned int hash; // [esp+4h] [ebp-14h]
    unsigned int assetEntryIndex; // [esp+8h] [ebp-10h]
    XAssetEntryPoolEntry *assetEntry; // [esp+Ch] [ebp-Ch]
    unsigned int defaultAssetCount; // [esp+10h] [ebp-8h]
    unsigned int defaultAssetsSize; // [esp+14h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_DBHASH);
    defaultAssetCount = g_defaultAssetCount;
    defaultAssetsSize = 0;
    for ( hash = 0; hash < 0x8000; ++hash )
    {
        for ( assetEntryIndex = db_hashTable[hash]; assetEntryIndex; assetEntryIndex = nextAssetEntryIndex )
        {
            assetEntry = &g_assetEntryPool[assetEntryIndex];
            nextAssetEntryIndex = assetEntry->entry.nextHash;
            if ( !assetEntry->entry.zoneIndex )
            {
                --defaultAssetCount;
                XAssetName = DB_GetXAssetName(&assetEntry->entry.asset);
                XAssetTypeName = DB_GetXAssetTypeName(assetEntry->entry.asset.type);
                Com_Printf(16, "%s,%s\n", XAssetTypeName, XAssetName);
                defaultAssetsSize += DB_GetXAssetTypeSize(assetEntry->entry.asset.type);
            }
        }
    }
    Com_Printf(16, "Total default assets %d, size %d\n", g_defaultAssetCount, defaultAssetsSize);
    if ( defaultAssetCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    3271,
                    0,
                    "%s",
                    "!defaultAssetCount") )
    {
        __debugbreak();
    }
    Sys_LeaveCriticalSection(CRITSECT_DBHASH);
}

void __cdecl DB_ListAssetPool_f()
{
    const char *XAssetTypeName; // eax
    const char *v1; // eax
    signed int i; // [esp+10h] [ebp-Ch]
    XAssetType type; // [esp+14h] [ebp-8h]

    if ( Cmd_Argc() >= 2 )
    {
        v1 = Cmd_Argv(1);
        type = (XAssetType)atoi(v1);
        DB_ListAssetPool(type);
    }
    else
    {
        Com_Printf(0, "listassetpool <poolnumber>: lists all the assets in the specified pool\n");
        for ( i = 0; i < 43; ++i )
        {
            XAssetTypeName = DB_GetXAssetTypeName(i);
            Com_Printf(0, "%d %s\n", i, XAssetTypeName);
        }
    }
}

void __cdecl DB_ListAssetPool(XAssetType type)
{
    const char *XAssetTypeName; // eax
    const char *XAssetName; // eax
    const char *v3; // eax
    const char *v4; // eax
    XZoneName *v5; // [esp-4h] [ebp-24h]
    XZoneName *v6; // [esp-4h] [ebp-24h]
    unsigned int nextAssetEntryIndex; // [esp+0h] [ebp-20h]
    unsigned int hash; // [esp+4h] [ebp-1Ch]
    unsigned int assetPoolSize; // [esp+8h] [ebp-18h]
    unsigned int assetEntryIndex; // [esp+Ch] [ebp-14h]
    XAssetEntryPoolEntry *overrideAssetEntry; // [esp+10h] [ebp-10h]
    unsigned int assetPoolCount; // [esp+14h] [ebp-Ch]
    XAssetEntryPoolEntry *assetEntry; // [esp+18h] [ebp-8h]
    unsigned int overrideAssetEntryIndex; // [esp+1Ch] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_DBHASH);
    assetPoolCount = 0;
    assetPoolSize = 0;
    XAssetTypeName = DB_GetXAssetTypeName(type);
    Com_Printf(16, "Listing assets in %s pool.\n", XAssetTypeName);
    for ( hash = 0; hash < 0x8000; ++hash )
    {
        for ( assetEntryIndex = db_hashTable[hash]; assetEntryIndex; assetEntryIndex = nextAssetEntryIndex )
        {
            assetEntry = &g_assetEntryPool[assetEntryIndex];
            nextAssetEntryIndex = assetEntry->entry.nextHash;
            if ( assetEntry->entry.asset.type == type )
            {
                ++assetPoolCount;
                v5 = &g_zoneNames[assetEntry->entry.zoneIndex];
                XAssetName = DB_GetXAssetName(&assetEntry->entry.asset);
                Com_Printf(16, "%s,%s\n", XAssetName, v5->name);
                assetPoolSize += DB_GetXAssetTypeSize(assetEntry->entry.asset.type);
                for ( overrideAssetEntryIndex = assetEntry->entry.nextOverride;
                            overrideAssetEntryIndex;
                            overrideAssetEntryIndex = overrideAssetEntry->entry.nextOverride )
                {
                    overrideAssetEntry = &g_assetEntryPool[overrideAssetEntryIndex];
                    ++assetPoolCount;
                    v6 = &g_zoneNames[overrideAssetEntry->entry.zoneIndex];
                    v3 = DB_GetXAssetName(&overrideAssetEntry->entry.asset);
                    Com_Printf(16, "%s,%s\n", v3, v6->name);
                    assetPoolSize += DB_GetXAssetTypeSize(overrideAssetEntry->entry.asset.type);
                }
            }
        }
    }
    v4 = DB_GetXAssetTypeName(type);
    Com_Printf(16, "Total of %d assets in %s pool, size %d\n", assetPoolCount, v4, assetPoolSize);
    Sys_LeaveCriticalSection(CRITSECT_DBHASH);
}

void __cdecl DB_DumpMaterialList_f()
{
    XAssetType type; // [esp+0h] [ebp-4h] BYREF

    type = ASSET_TYPE_MATERIAL;
    DB_PrintXAssetsForType(ASSET_TYPE_MATERIAL, (void (__cdecl *)(XAssetHeader, void *))DB_PrintAssetName, &type, 1);
}

void __cdecl DB_LoadXZone(XZoneInfo *zoneInfo, unsigned int zoneCount)
{
    unsigned int j; // [esp+0h] [ebp-Ch]
    const char *zoneName; // [esp+4h] [ebp-8h]
    unsigned int zoneInfoCount; // [esp+8h] [ebp-4h]

    if ( g_zoneCount == 32 )
        Com_Error(ERR_DROP, "Max zone count exceeded");
    if ( g_zoneInfoCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    4417,
                    0,
                    "%s",
                    "!g_zoneInfoCount") )
    {
        __debugbreak();
    }
    if ( g_loadingAssets
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    4418,
                    0,
                    "%s",
                    "!g_loadingAssets") )
    {
        __debugbreak();
    }
    zoneInfoCount = 0;
    for ( j = 0; j < zoneCount; ++j )
    {
        zoneName = zoneInfo[j].name;
        if ( zoneName )
        {
            if ( zoneInfoCount >= 0x20
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                            4426,
                            0,
                            "%s",
                            "zoneInfoCount < ARRAY_COUNT( g_zoneInfo )") )
            {
                __debugbreak();
            }
            I_strncpyz(g_zoneInfo[zoneInfoCount].name, zoneName, 64);
            Com_Printf(16, "Adding fastfile '%s' to queue\n", g_zoneInfo[zoneInfoCount].name);
            g_zoneInfo[zoneInfoCount++].flags = zoneInfo[j].allocFlags;
        }
    }
    if ( zoneInfoCount )
    {
        g_loadingAssets = zoneInfoCount;
        Sys_WakeDatabase2();
        Sys_WakeDatabase();
        g_zoneInfoCount = zoneInfoCount;
        Sys_NotifyDatabase();
    }
}

void __cdecl DB_LoadZone_f()
{
    const char *v0; // eax

    v0 = Cmd_Argv(1);
    I_strncpyz(g_debugZoneName, v0, 64);
    DB_UpdateDebugZone();
}

void __cdecl DB_InitThread()
{
    if ( !Sys_SpawnDatabaseThread((void (__cdecl *)(unsigned int))DB_Thread) )
        Sys_Error((char*)"Failed to create database thread");
}

void __cdecl    DB_Thread(unsigned int threadContext)
{
    int *Value; // eax

    if ( threadContext != 13
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    5383,
                    0,
                    "threadContext == THREAD_CONTEXT_DATABASE\n\t%i, %i",
                    threadContext,
                    13) )
    {
        __debugbreak();
    }
    Value = (int*)Sys_GetValue(2);
    //if ( _setjmp3(Value, 0) )
    if ( _setjmp(Value) )
        Com_ErrorAbort();
    R_ReleaseDXDeviceOwnership();
    while ( 1 )
    {
        Sys_WaitStartDatabase();
        DB_TryLoadXFile();
    }
}

void DB_TryLoadXFile()
{
    char *v0; // eax
    unsigned int j; // [esp+0h] [ebp-Ch]
    unsigned int zoneInfoCount; // [esp+8h] [ebp-4h]

    if ( g_zoneInfoCount )
    {
        zoneInfoCount = g_zoneInfoCount;
        g_zoneInfoCount = 0;
        if ( g_loadingZone
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                        5330,
                        0,
                        "%s",
                        "!g_loadingZone") )
        {
            __debugbreak();
        }
        for ( j = 0; j < zoneInfoCount; ++j )
        {
            if ( !DB_TryLoadXFileInternal(g_zoneInfo[j].name, g_zoneInfo[j].flags) )
            {
                v0 = strstr(g_zoneInfo[j].name, "_xpatch");
                if ( v0 )
                {
                    sprintf(g_zoneInfo[j].name, "default_mp");
                    if ( !DB_TryLoadXFileInternal(g_zoneInfo[j].name, g_zoneInfo[j].flags) )
                        --g_loadingAssets;
                }
                else
                {
                    --g_loadingAssets;
                }
            }
        }
        if ( g_loadingZone
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                        5356,
                        0,
                        "%s",
                        "!g_loadingZone") )
        {
            __debugbreak();
        }
        if ( g_loadingAssets
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                        5357,
                        0,
                        "%s",
                        "!g_loadingAssets") )
        {
            __debugbreak();
        }
        Sys_LockWrite(&s_dbReorder.critSect);
        DB_EndReorderZone();
        Sys_UnlockWrite(&s_dbReorder.critSect);
        Sys_DatabaseCompleted();
    }
    else if ( g_loadingAssets )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                        5325,
                        0,
                        "%s",
                        "!g_loadingAssets") )
            __debugbreak();
    }
}

int __cdecl DB_TryLoadXFileInternal(const char *zoneName, int zoneFlags)
{
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    const char *LanguageNameAbbr; // eax
    unsigned int v7; // eax
    unsigned int startWaitingTime; // [esp+2Ch] [ebp-124h]
    XZone *zone; // [esp+30h] [ebp-120h]
    FF_DIR zoneDir; // [esp+34h] [ebp-11Ch] BYREF
    char filename[260]; // [esp+38h] [ebp-118h] BYREF
    const char *fixedZoneName; // [esp+140h] [ebp-10h]
    bool succeeded; // [esp+147h] [ebp-9h]
    unsigned int i; // [esp+148h] [ebp-8h]
    void *zoneFile; // [esp+14Ch] [ebp-4h] BYREF

    zoneDir = FFD_DEFAULT;
    zone = &g_zones[g_zoneCount];
    memset((unsigned __int8 *)zone, 0, sizeof(XZone));
    if ( g_zoneInfoCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    5083,
                    0,
                    "%s",
                    "!g_zoneInfoCount") )
    {
        __debugbreak();
    }
    zoneFile = (void *)-1;
    if ((zoneFlags & 0x2AAAAAA) != 0
        || (DB_BuildOSPath_Unlocalized(zoneName, ".ff", 0x100u, filename), GetFileAttributesA(filename) == -1) )
    {
        DB_BuildOSPath(zoneName, ".ff", 0x100u, filename);
    }
    zoneFile = CreateFileA(filename, 0x80000000, 1u, 0, 3u, 0x60000000u, 0);
    if ( zoneFile == (void *)-1 )
        DB_ModXFileHandle(zoneName, &zoneFile, &zoneDir);
    if ( zoneFile == (void *)-1 )
    {
        v2 = strstr(filename, "_load");
        if ( v2
            || (v3 = strstr(filename, "_patch"), v3)
            || (v4 = strstr(filename, "default"), v4)
            || loc_language
            && (LanguageNameAbbr = SEH_GetLanguageNameAbbr(loc_language->current.unsignedInt),
                    I_strnicmp(filename, LanguageNameAbbr, 3)) )
        {
            Com_PrintWarning(10, "WARNING: Could not find zone '%s'\n", filename);
            return 0;
        }
        else
        {
            Sys_DatabaseCompleted();
            Com_Error(ERR_DROP, "ERROR: Could not find zone '%s'\n", filename);
            return 0;
        }
    }
    else
    {
        g_zoneIndex = 0;
        for ( i = 1; i < 0x21; ++i )
        {
            if ( !g_zoneNames[i].name[0] )
            {
                g_zoneIndex = i;
                break;
            }
        }
        if ( !g_zoneIndex
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp", 5198, 0, "%s", "g_zoneIndex") )
        {
            __debugbreak();
        }
        if ( !*zoneName
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp", 5199, 0, "%s", "zoneName[0]") )
        {
            __debugbreak();
        }
        fixedZoneName = g_zoneNames[g_zoneIndex].name;
        if ( *fixedZoneName
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                        5202,
                        0,
                        "%s",
                        "!fixedZoneName[0]") )
        {
            __debugbreak();
        }
        g_zoneNames[g_zoneIndex].loaded = 0;
        I_strncpyz((char *)fixedZoneName, zoneName, 80);
        g_zoneNames[g_zoneIndex].flags = zoneFlags;
        g_zoneNames[g_zoneIndex].fileSize = GetFileSize(zoneFile, 0);
        g_zoneNames[g_zoneIndex].dir = zoneDir;
        zone->index = g_zoneIndex;
        zone->flags = zoneFlags;
        memset((unsigned __int8 *)zone->blocks, 0, sizeof(zone->blocks));
        if ( g_loadingZone
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                        5217,
                        0,
                        "%s",
                        "!g_loadingZone") )
        {
            __debugbreak();
        }

        static const dvar_t *zone_reorder = _Dvar_RegisterString(
            "zone_reorder",
            (char *)"",
            0,
            "Set to the name of the fast file you want to reorder");

        //if ( (_S1_4 & 1) == 0 )
        //{
        //    _S1_4 |= 1u;
        //    zone_reorder = _Dvar_RegisterString(
        //                                     "zone_reorder",
        //                                     (char *)"",
        //                                     0,
        //                                     "Set to the name of the fast file you want to reorder");
        //}

        if ( !_stricmp(zone_reorder->current.string, zoneName) )
            DB_BeginReorderZone(zoneName);
        ++g_zoneCount;
        g_loadingZone = 1;
        while ( g_isRecoveringLostDevice )
            NET_Sleep(0x19u);
        g_mayRecoverLostAssets = 0;
        g_zoneAllocType = DB_GetZoneAllocType(zoneFlags);
        if ( g_zoneAllocType == 1 && g_initializing )
        {
            startWaitingTime = Sys_Milliseconds();
            Com_Printf(0, "Waiting for $init to finish.    There may be assets missing from code_post_gfx.\n");
            while ( g_initializing )
                DB_Sleep(1u);
            v7 = Sys_Milliseconds();
            Com_Printf(16, "Waited %d ms for $init to finish.\n", v7 - startWaitingTime);
        }
        if ( (zoneFlags & 0x40000000) == 0 )
            PMem_BeginAlloc(fixedZoneName, g_zoneAllocType, TRACK_FASTFILE);
        if ( (zoneFlags & 0x8000) == 0 )
            DB_ResetZoneSize((zoneFlags & 0x4000) != 0);
        if ( !zone
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp", 5260, 0, "%s", "zone") )
        {
            __debugbreak();
        }
        skipLoadingMaterialsHack = -1;
        if ( dx.vendorId == 4098
            && dx.d3d9
            && dx.d3d9->CheckDeviceFormat(
                     dx.adapterIndex,
                     D3DDEVTYPE_HAL,
                     D3DFMT_X8R8G8B8,
                     0x100000u,
                     D3DRTYPE_TEXTURE,
                     D3DFMT_R32F) )
        {
            Com_Printf(8, "Detected ATI card that doesn't support vertex textures.");
            if ( !strcmp(g_zoneNames[g_zoneIndex].name, "fullahead") )
            {
                skipLoadingMaterialsHack = 1205;
            }
            else if ( !strcmp(g_zoneNames[g_zoneIndex].name, "rebirth") )
            {
                skipLoadingMaterialsHack = 542;
            }
        }
        succeeded = DB_LoadXFile(
                                    filename,
                                    zoneFile,
                                    g_zoneNames[g_zoneIndex].name,
                                    zone->blocks,
                                    0,
                                    g_fileBuf,
                                    g_zoneAllocType,
                                    zoneFlags);
        if ( (zoneFlags & 0x40000000) == 0 )
            PMem_EndAlloc(g_zoneNames[g_zoneIndex].name, g_zoneAllocType);
        if ( succeeded )
            g_zoneNames[g_zoneIndex].loaded = 1;
        if ( !g_loadingZone
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                        5290,
                        0,
                        "%s",
                        "g_loadingZone") )
        {
            __debugbreak();
        }
        g_loadingZone = 0;
        g_mayRecoverLostAssets = 1;
        return 1;
    }
}

void __cdecl DB_BeginReorderZone(const char *zoneName)
{
    char v1; // [esp+3h] [ebp-259h]
    char *v2; // [esp+8h] [ebp-254h]
    const char *v3; // [esp+Ch] [ebp-250h]
    char *from; // [esp+10h] [ebp-24Ch]
    DBReorderAssetEntry *entry; // [esp+14h] [ebp-248h]
    char assetType[32]; // [esp+18h] [ebp-244h] BYREF
    unsigned int size; // [esp+38h] [ebp-224h]
    void *file; // [esp+3Ch] [ebp-220h]
    int success; // [esp+40h] [ebp-21Ch]
    char assetName[256]; // [esp+44h] [ebp-218h] BYREF
    char csvName[256]; // [esp+144h] [ebp-118h] BYREF
    const char *parse; // [esp+248h] [ebp-14h] BYREF
    char *csv; // [esp+24Ch] [ebp-10h]
    char *to; // [esp+250h] [ebp-Ch]
    DWORD read; // [esp+254h] [ebp-8h] BYREF
    unsigned int entryIter; // [esp+258h] [ebp-4h]

    for ( entryIter = 0; entryIter < s_dbReorder.entryCount; ++entryIter )
    {
        entry = &s_dbReorder.entries[entryIter];
        free((void *)entry->assetName);
        if ( entry->type == 43 )
            free((void *)entry->typeString);
    }
    s_dbReorder.entryCount = 0;
    s_dbReorder.sequence = 2;
    s_dbReorder.sequenceForIncludes = 3;
    s_dbReorder.alreadyFinished = 0;
    s_dbReorder.loadedSound = 0;
    s_dbReorder.loadedLocalization = 0;
    s_dbReorder.lastEntry = 0;
    v3 = zoneName;
    v2 = s_dbReorder.zoneName;
    do
    {
        v1 = *v3;
        *v2++ = *v3++;
    }
    while ( v1 );
    Sys_LockWrite(&s_dbReorder.critSect);
    Com_sprintf(csvName, 0x100u, "..\\share\\zone_source\\%s.csv", zoneName);
    file = CreateFileA(csvName, 0x80000000, 0, 0, 3u, 0, 0);
    if ( file == (void *)-1 )
    {
        Sys_UnlockWrite(&s_dbReorder.critSect);
    }
    else
    {
        size = GetFileSize(file, 0);
        csv = (char *)malloc(size + 1);
        success = ReadFile(file, csv, size, &read, 0);
        CloseHandle(file);
        if ( success && read == size )
        {
            csv[size] = 0;
            to = csv;
            for ( from = csv; *from; ++from )
            {
                if ( *from != 13 )
                    *to++ = *from;
            }
            *to = 0;
            for ( parse = csv; DB_NextZoneCsvToken(&parse, assetType, 0x20u, 1); ++parse )
            {
                if ( DB_NextZoneCsvToken(&parse, assetName, 0x100u, 0) && assetType[0] && assetName[0] )
                    DB_AddReorderAsset(assetType, assetName);
                while ( *parse && *parse != 10 )
                    ++parse;
                if ( !*parse )
                    break;
            }
            free(csv);
            Sys_UnlockWrite(&s_dbReorder.critSect);
        }
        else
        {
            free(csv);
            Sys_UnlockWrite(&s_dbReorder.critSect);
        }
    }
}

void __cdecl DB_AddReorderAsset(const char *typeString, const char *assetName)
{
    char *v2; // [esp+0h] [ebp-10h]
    DBReorderAssetEntry *entry; // [esp+4h] [ebp-Ch]
    DBReorderAssetEntry *entrya; // [esp+4h] [ebp-Ch]
    int type; // [esp+8h] [ebp-8h]
    unsigned int entryIter; // [esp+Ch] [ebp-4h]

    for ( type = 0; type < 43 && _stricmp(typeString, g_assetNames[type]); ++type )
        ;
    if ( type == 24 )
    {
        if ( I_strnicmp(assetName, "mp/", 3u)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                        2350,
                        0,
                        "%s",
                        "!strnicmp( assetName, REORDER_WEAPON_PREFIX, REORDER_WEAPON_PREFIX_LEN )") )
        {
            __debugbreak();
        }
        assetName += 3;
    }
    for ( entryIter = 0; entryIter < s_dbReorder.entryCount; ++entryIter )
    {
        entry = &s_dbReorder.entries[entryIter];
        if ( entry->type == type && !_stricmp(entry->assetName, assetName) && !_stricmp(entry->typeString, typeString) )
            return;
    }
    entrya = &s_dbReorder.entries[s_dbReorder.entryCount++];
    entrya->type = type;
    if ( type >= 43 )
        v2 = _strdup(typeString);
    else
        v2 = (char *)g_assetNames[type];
    entrya->typeString = v2;
    entrya->assetName = _strdup(assetName);
    if ( entrya->type != 43 || I_stricmp(typeString, "ignore") )
    {
        if ( entrya->type == 11 || entrya->type == 12 )
            entrya->sequence = 1;
        else
            entrya->sequence = -1;
    }
    else
    {
        entrya->sequence = 0;
    }
}

char __cdecl DB_NextZoneCsvToken(const char **parse, char *token, unsigned int tokenSize, bool allowNewLine)
{
    unsigned int used; // [esp+0h] [ebp-Ch]
    bool isSkippingLeadingSpaces; // [esp+7h] [ebp-5h]
    const char *scan; // [esp+8h] [ebp-4h]

    scan = *parse;
    if ( **parse == 10 )
    {
        if ( !allowNewLine )
            return 0;
        while ( *scan == 10 )
            ++scan;
    }
    if ( *scan )
    {
        if ( *scan == 44 )
            ++scan;
        used = 0;
        isSkippingLeadingSpaces = 1;
        while ( *scan && *scan != 10 && *scan != 44 )
        {
            if ( !isSkippingLeadingSpaces || !isspace(*scan) )
            {
                isSkippingLeadingSpaces = 0;
                if ( used < tokenSize - 1 )
                    token[used++] = *scan;
            }
            ++scan;
        }
        while ( used && isspace(token[used - 1]) )
            --used;
        token[used] = 0;
        *parse = scan;
        return 1;
    }
    else
    {
        *parse = scan;
        return 0;
    }
}

bool __cdecl DB_GetZoneAllocType(int zoneFlags)
{
    return zoneFlags >= 0x100000;
}

void __cdecl DB_ModXFileHandle(const char *zoneName, void **zoneFile, FF_DIR *zoneDir)
{
    const char *String; // eax
    const char *v4; // eax
    bool v6; // [esp+4h] [ebp-10Ch]
    char filename[260]; // [esp+8h] [ebp-108h] BYREF

    v6 = fs_gameDirVar && *(_BYTE *)fs_gameDirVar->current.integer;
    if ( !v6 || I_stricmp(zoneName, "mod") )
    {
        if ( fs_usermapDir && *(_BYTE *)fs_usermapDir->current.integer )
        {
            String = Dvar_GetString("fs_usermapDir");
            v4 = va("%s\\%s", String, zoneName);
            DB_BuildOSPath_ForUsermap(v4, 0x100u, filename);
            *zoneFile = CreateFileA(filename, 0x80000000, 1u, 0, 3u, 0x60000000u, 0);
            if ( *zoneFile != (void *)-1 )
                *zoneDir = FFD_USER_MAP;
        }
    }
    else
    {
        DB_BuildOSPath_FromSource(zoneName, FFD_MOD_DIR, 0x100u, filename);
        *zoneFile = CreateFileA(filename, 0x80000000, 1u, 0, 3u, 0x60000000u, 0);
        if ( *zoneFile != (void *)-1 )
            *zoneDir = FFD_MOD_DIR;
    }
}

void __cdecl DB_BuildOSPath_FromSource(const char *zoneName, FF_DIR source, unsigned int size, char *filename)
{
    bool v4; // [esp+0h] [ebp-10h]

    if ( source )
    {
        if ( source == FFD_MOD_DIR )
        {
            v4 = fs_gameDirVar && *(_BYTE *)fs_gameDirVar->current.integer;
            if ( !v4
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                            4370,
                            0,
                            "%s",
                            "IsUsingMods()") )
            {
                __debugbreak();
            }
            Com_sprintf(filename, size, "%s\\%s\\%s.ff", fs_homepath->current.string, fs_gameDirVar->current.string, zoneName);
        }
        else if ( source == FFD_USER_MAP )
        {
            Com_sprintf(filename, size, "%s\\%s\\%s\\%s.ff", fs_homepath->current.string, "usermaps", zoneName, zoneName);
        }
        else if ( !Assert_MyHandler(
                                 "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                                 4385,
                                 1,
                                 "inconceivable") )
        {
            __debugbreak();
        }
    }
    else
    {
        DB_BuildOSPath_Unlocalized(zoneName, ".ff", size, filename);
        if ( GetFileAttributesA(filename) == -1 )
            DB_BuildOSPath(zoneName, ".ff", size, filename);
    }
}

void __cdecl DB_BuildOSPath_ForUsermap(const char *zoneName, unsigned int size, char *filename)
{
    Com_sprintf(filename, size, "%s\\%s\\%s.ff", fs_homepath->current.string, "usermaps", zoneName);
}

void __cdecl DB_UnloadXZone(XZone *zone, bool createDefault)
{
    DB_UnloadXZoneInternal(zone->index, createDefault);
}

void __cdecl DB_UnloadXZoneInternal(unsigned int zoneIndex, bool createDefault)
{
    const char *v2; // eax
    const char *XAssetTypeName; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // [esp-8h] [ebp-34h]
    const char *String; // [esp-4h] [ebp-30h]
    const char *XAssetName; // [esp-4h] [ebp-30h]
    const char *v11; // [esp-4h] [ebp-30h]
    const char *v12; // [esp-4h] [ebp-30h]
    const char *v13; // [esp-4h] [ebp-30h]
    XZoneName *v14; // [esp-4h] [ebp-30h]
    unsigned int hash; // [esp+4h] [ebp-28h]
    unsigned __int16 *pAssetEntryIndex; // [esp+8h] [ebp-24h]
    XAssetEntryPoolEntry *overrideAssetEntrya; // [esp+Ch] [ebp-20h]
    XAssetEntryPoolEntry *overrideAssetEntry; // [esp+Ch] [ebp-20h]
    XAsset asset; // [esp+14h] [ebp-18h] BYREF
    const char *name; // [esp+1Ch] [ebp-10h]
    XAssetEntry *assetEntry; // [esp+20h] [ebp-Ch]
    unsigned __int16 *pOverrideAssetEntryIndex; // [esp+24h] [ebp-8h]
    unsigned int overrideAssetEntryIndex; // [esp+28h] [ebp-4h]

    if ( !zoneIndex
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp", 5436, 0, "%s", "zoneIndex") )
    {
        __debugbreak();
    }
    R_StreamPushSyncDisable();
    Com_Printf(16, "Unloading assets from fastfile '%s' ", g_zoneNames[zoneIndex].name);
    if ( createDefault )
        Com_Printf(16, "and creating default assets stubs\n");
    else
        Com_Printf(16, "and deleting all assets\n");
    hash = 0;
LABEL_8:
    if ( hash < 0x8000 )
    {
        pAssetEntryIndex = &db_hashTable[hash];
        while ( 1 )
        {
            while ( 1 )
            {
                if ( !*pAssetEntryIndex )
                {
                    ++hash;
                    goto LABEL_8;
                }
                assetEntry = &g_assetEntryPool[*pAssetEntryIndex].entry;
                if ( assetEntry->zoneIndex == zoneIndex )
                    break;
LABEL24:
                pOverrideAssetEntryIndex = &assetEntry->nextOverride;
                while ( *pOverrideAssetEntryIndex )
                {
                    overrideAssetEntry = &g_assetEntryPool[*pOverrideAssetEntryIndex];
                    if ( overrideAssetEntry->entry.inuse
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                                    5530,
                                    0,
                                    "%s",
                                    "!overrideAssetEntry->inuse") )
                    {
                        __debugbreak();
                    }
                    if ( overrideAssetEntry->entry.zoneIndex == zoneIndex )
                    {
                        DB_RemoveXAsset(&overrideAssetEntry->entry.asset);
                        *pOverrideAssetEntryIndex = overrideAssetEntry->entry.nextOverride;
                        DB_FreeXAssetEntry((XAssetEntryPoolEntry *)&overrideAssetEntry->entry);
                    }
                    else
                    {
                        pOverrideAssetEntryIndex = &overrideAssetEntry->entry.nextOverride;
                    }
                }
                pAssetEntryIndex = &assetEntry->nextHash;
            }
            if ( assetEntry->inuse && createDefault )
            {
                varXAsset = &assetEntry->asset;
                Mark_XAsset();
            }
            if ( db_xassetdebug->current.enabled )
            {
                if ( db_xassetdebugtype->current.integer == -1
                    || db_xassetdebugtype->current.integer == assetEntry->asset.type
                    || *Dvar_GetString("db_xassetdebugname")
                    && (String = Dvar_GetString("db_xassetdebugname"),
                            v2 = DB_GetXAssetName(&assetEntry->asset),
                            !I_stricmp(v2, String)) )
                {
                    XAssetName = DB_GetXAssetName(&assetEntry->asset);
                    XAssetTypeName = DB_GetXAssetTypeName(assetEntry->asset.type);
                    Com_Printf(16, "DB_UnloadXZoneInternal: removing asset: '%s','%s'\n", XAssetTypeName, XAssetName);
                }
            }
            DB_RemoveXAsset(&assetEntry->asset);
            overrideAssetEntryIndex = assetEntry->nextOverride;
            if ( overrideAssetEntryIndex )
            {
                overrideAssetEntrya = &g_assetEntryPool[overrideAssetEntryIndex];
                DB_CloneXAssetEntry(&overrideAssetEntrya->entry, assetEntry, DB_CLONE_NORMAL);
                assetEntry->nextOverride = overrideAssetEntrya->entry.nextOverride;
                DB_FreeXAssetEntry((XAssetEntryPoolEntry *)&overrideAssetEntrya->entry);
                if ( db_xassetdebug->current.enabled )
                {
                    if ( db_xassetdebugtype->current.integer == -1
                        || db_xassetdebugtype->current.integer == assetEntry->asset.type
                        || *Dvar_GetString("db_xassetdebugname")
                        && (v13 = Dvar_GetString("db_xassetdebugname"),
                                v6 = DB_GetXAssetName(&assetEntry->asset),
                                !I_stricmp(v6, v13)) )
                    {
                        v14 = &g_zoneNames[assetEntry->zoneIndex];
                        v8 = DB_GetXAssetName(&assetEntry->asset);
                        v7 = DB_GetXAssetTypeName(assetEntry->asset.type);
                        Com_Printf(16, "DB_UnloadXZoneInternal: reverted to asset: '%s','%s' from %s\n", v7, v8, v14->name);
                    }
                }
                goto LABEL24;
            }
            if ( createDefault )
            {
                asset.type = assetEntry->asset.type;
                asset.header = DB_FindXAssetDefaultHeaderInternal(asset.type);
                if ( asset.header.xmodelPieces )
                {
                    ++g_defaultAssetCount;
                    assetEntry->zoneIndex = 0;
                    name = DB_GetXAssetName(&assetEntry->asset);
                    DB_CloneXAssetInternal(&asset, &assetEntry->asset);
                    DB_SetXAssetName(&assetEntry->asset, name);
                    if ( db_xassetdebug->current.enabled )
                    {
                        if ( db_xassetdebugtype->current.integer == -1
                            || db_xassetdebugtype->current.integer == assetEntry->asset.type
                            || *Dvar_GetString("db_xassetdebugname")
                            && (v11 = Dvar_GetString("db_xassetdebugname"),
                                    v4 = DB_GetXAssetName(&assetEntry->asset),
                                    !I_stricmp(v4, v11)) )
                        {
                            v12 = DB_GetXAssetName(&assetEntry->asset);
                            v5 = DB_GetXAssetTypeName(assetEntry->asset.type);
                            Com_Printf(16, "DB_UnloadXZoneInternal: using default for asset: '%s','%s'\n", v5, v12);
                        }
                    }
                    goto LABEL24;
                }
                if ( assetEntry->nextOverride
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                                5486,
                                0,
                                "%s",
                                "!assetEntry->nextOverride") )
                {
                    __debugbreak();
                }
                *pAssetEntryIndex = assetEntry->nextHash;
                DB_FreeXAssetEntry((XAssetEntryPoolEntry *)assetEntry);
                if ( *g_defaultAssetName[asset.type] )
                {
                    Sys_LeaveCriticalSection(CRITSECT_DBHASH);
                    Sys_Error((char*)"Could not load default asset for asset type '%s'", g_assetNames[asset.type]);
                }
            }
            else
            {
                if ( assetEntry->nextOverride )
                {
                    if ( !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                                    5475,
                                    0,
                                    "%s",
                                    "!assetEntry->nextOverride") )
                        __debugbreak();
                }
                *pAssetEntryIndex = assetEntry->nextHash;
                DB_FreeXAssetEntry((XAssetEntryPoolEntry *)assetEntry);
            }
        }
    }
    R_StreamPopSyncDisable();
}

static void REMOVE_NULLSUB(XAssetHeader)
{

}

void(__cdecl *DB_RemoveXAssetHandler[43])(XAssetHeader) =
{
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  &DB_RemoveXModel,
  NULL,
  &Material_FreeTechniqueSet,
  &DB_RemoveImage,
  &DB_RemoveSound,
  &DB_RemoveSoundPatch,
  &DB_RemoveClipMap,
  &DB_RemoveClipMap,
  &DB_RemoveComWorld,
  NULL,
  NULL,
  NULL,
  &DB_RemoveGfxWorld,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  &REMOVE_NULLSUB,
  NULL,
  NULL,
  NULL,
  NULL
};



void __cdecl DB_RemoveXAsset(XAsset *asset)
{
    if (DB_RemoveXAssetHandler[asset->type])
    {
        //((void(__cdecl *)(unsigned int))DB_RemoveXAssetHandler[asset->type])((XAssetHeader)asset->header.xmodelPieces);
        DB_RemoveXAssetHandler[asset->type]((XAssetHeader)asset->header.xmodelPieces);
    }
}

void __cdecl DB_ReleaseXAssets()
{
    unsigned int hash; // [esp+0h] [ebp-Ch]
    unsigned int assetEntryIndex; // [esp+4h] [ebp-8h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    5611,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    Sys_SyncDatabase();
    for ( hash = 0; hash < 0x8000; ++hash )
    {
        for ( assetEntryIndex = db_hashTable[hash];
                    assetEntryIndex;
                    assetEntryIndex = g_assetEntryPool[assetEntryIndex].entry.nextHash )
        {
            g_assetEntryPool[assetEntryIndex].entry.inuse = 0;
        }
    }
}

void __cdecl DB_ShutdownXAssets()
{
    int i; // [esp+0h] [ebp-4h]
    int ia; // [esp+0h] [ebp-4h]

    DB_SyncXAssets();
    DB_SyncExternalAssets();
    Sys_EnterCriticalSection(CRITSECT_DBHASH);
    for ( i = g_zoneCount - 1; i >= 0; --i )
        DB_UnloadXZone(&g_zones[i], 0);
    DB_FreeDefaultEntries();
    DB_FreeUnusedResources();
    for ( ia = g_zoneCount - 1; ia >= 0; --ia )
        DB_UnloadXZoneMemory(&g_zones[ia]);
    g_zoneCount = 0;
    DB_ResetMinimumFastFileLoaded();
    Sys_LeaveCriticalSection(CRITSECT_DBHASH);
}

void __cdecl DB_UnloadXZoneMemory(XZone *zone)
{
    DB_FreeXBlocks(zone->blocks, zone->flags);
    Com_Printf(16, "Unloaded fastfile '%s'\n", g_zoneNames[zone->index].name);
    if ( (zone->flags & 0x40000000) == 0 )
        PMem_Free(g_zoneNames[zone->index].name);
    g_zoneNames[zone->index].name[0] = 0;
}

void DB_FreeDefaultEntries()
{
    unsigned int nextAssetEntryIndex; // [esp+0h] [ebp-10h]
    unsigned int hash; // [esp+4h] [ebp-Ch]
    unsigned int assetEntryIndex; // [esp+8h] [ebp-8h]
    XAssetEntryPoolEntry *assetEntry; // [esp+Ch] [ebp-4h]

    for ( hash = 0; hash < 0x8000; ++hash )
    {
        for ( assetEntryIndex = db_hashTable[hash]; assetEntryIndex; assetEntryIndex = nextAssetEntryIndex )
        {
            assetEntry = &g_assetEntryPool[assetEntryIndex];
            nextAssetEntryIndex = assetEntry->entry.nextHash;
            if ( assetEntry->entry.zoneIndex
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                            5592,
                            0,
                            "%s",
                            "!assetEntry->zoneIndex") )
            {
                __debugbreak();
            }
            if ( assetEntry->entry.nextOverride
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                            5593,
                            0,
                            "%s",
                            "!assetEntry->nextOverride") )
            {
                __debugbreak();
            }
            if ( !g_defaultAssetCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                            5594,
                            0,
                            "%s",
                            "g_defaultAssetCount") )
            {
                __debugbreak();
            }
            --g_defaultAssetCount;
            DB_FreeXAssetEntry(assetEntry);
        }
        db_hashTable[hash] = 0;
    }
    if ( g_defaultAssetCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    5601,
                    0,
                    "%s",
                    "!g_defaultAssetCount") )
    {
        __debugbreak();
    }
}

void __cdecl DB_UnloadXAssetsMemoryForZone(int zoneFreeFlags, int zoneFreeBit)
{
    int i; // [esp+0h] [ebp-4h]

    if ( (zoneFreeBit & zoneFreeFlags) != 0 )
    {
        for ( i = g_zoneCount - 1; i >= 0; --i )
        {
            if ( (zoneFreeBit & g_zones[i].flags) != 0 )
                DB_UnloadXAssetsMemory(&g_zones[i]);
        }
    }
}

void __cdecl DB_UnloadXAssetsMemory(XZone *zone)
{
    int zoneIter; // [esp+8h] [ebp-4h]

    for ( zoneIter = 0; zone != &g_zones[zoneIter]; ++zoneIter )
    {
        if ( zoneIter >= g_zoneCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                        5661,
                        0,
                        "%s",
                        "zoneIter < g_zoneCount") )
        {
            __debugbreak();
        }
    }
    DB_UnloadXZoneMemory(zone);
    --g_zoneCount;
    while ( zoneIter < g_zoneCount )
    {
        memcpy(&g_zones[zoneIter], &g_zones[zoneIter + 1], sizeof(XZone));
        ++zoneIter;
    }
}

void __cdecl DB_ReplaceModel(const char *original, const char *replacement)
{
    DB_ReplaceXAsset(ASSET_TYPE_XMODEL, original, replacement);
}

void __cdecl DB_ReplaceXAsset(XAssetType type, const char *original, const char *replacement)
{
    const char *originalName; // [esp+8h] [ebp-14h]
    XAsset replacementAsset; // [esp+Ch] [ebp-10h] BYREF
    XAsset originalAsset; // [esp+14h] [ebp-8h] BYREF

    originalAsset.type = type;
    originalAsset.header = DB_FindXAssetHeader(type, (char*)original, 1, -1);
    originalName = DB_GetXAssetName(&originalAsset);
    replacementAsset.type = type;
    replacementAsset.header = DB_FindXAssetHeader(type, (char *)replacement, 1, -1);
    DB_CloneXAsset(&replacementAsset, &originalAsset, DB_CLONE_NORMAL);
    DB_SetXAssetName(&originalAsset, originalName);
}

void __cdecl DB_SyncExternalAssetsInternal()
{
    int semaphore; // [esp+0h] [ebp-4h]

    semaphore = R_AcquireDXDeviceOwnership(0);
    RB_UnbindAllImages();
    R_ShutdownStreams();
    RB_ClearPixelShader();
    RB_ClearVertexShader();
    RB_ClearVertexDecl();
    if ( semaphore )
        R_ReleaseDXDeviceOwnership();
}

void DB_SyncExternalAssets() // inlined in retail
{
    int depth; // [esp+0h] [ebp-4h]

    depth = 0;

    if ( Sys_IsMainThread() )
        depth = R_PopRemoteScreenUpdate();

    R_SyncRenderThread();

    if (!Sys_IsRenderThread() && !IsDedicatedServer())
    {
        RB_Resource_Callback(DB_SyncExternalAssetsInternal);
        RB_Resource_Flush();
    }
    else
    {
        DB_SyncExternalAssetsInternal();
    }


    if ( Sys_IsMainThread() )
        R_PushRemoteScreenUpdate(depth);
}

void DB_ArchiveAssets()
{
    int depth; // [esp+0h] [ebp-4h]

    if ( !g_archiveBuf )
    {
        g_archiveBuf = 1;
        depth = 0;
        if ( Sys_IsMainThread() )
            depth = R_PopRemoteScreenUpdate();
        R_ClearAllStaticModelCacheRefs();
        //BLOPS_NULLSUB();
        DB_SaveDObjs();
        DB_RestartEffects_Begin();
        if ( Sys_IsMainThread() )
            R_PushRemoteScreenUpdate(depth);
    }
}

void DB_FreeUnusedResources()
{
    unsigned int String; // eax
    const char *v1; // eax
    const char *XAssetTypeName; // eax
    const char *v3; // [esp-4h] [ebp-8024h]
    const char *XAssetName; // [esp-4h] [ebp-8024h]
    unsigned __int16 data[16386]; // [esp+0h] [ebp-8020h] BYREF
    unsigned int i; // [esp+8008h] [ebp-18h]
    _WORD *v7; // [esp+800Ch] [ebp-14h]
    int j; // [esp+8010h] [ebp-10h]
    char *name; // [esp+8014h] [ebp-Ch]
    char *str; // [esp+8018h] [ebp-8h]
    XAsset *asset; // [esp+801Ch] [ebp-4h]

    SL_TransferSystem(4u, 8u, SCRIPTINSTANCE_SERVER);
    DB_EnableInUseCache(data);
    for ( i = 0; i < 0x8000; ++i )
    {
        for ( j = db_hashTable[i]; j; j = HIWORD(asset[1].type) )
        {
            asset = &g_assetEntryPool[j].entry.asset;
            if ( LOBYTE(asset[1].type) )
            {
                varXAsset = asset;
                Mark_XAsset();
            }
        }
    }
    DB_DisableInUseCache();
    for ( i = 0; i < 0x8000; ++i )
    {
        v7 = &db_hashTable[i];
        while ( *v7 )
        {
            j = (unsigned __int16)*v7;
            asset = &g_assetEntryPool[j].entry.asset;
            if ( LOBYTE(asset[1].type) )
            {
                v7 = (_WORD *)&asset[1].type + 1;
            }
            else if ( BYTE1(asset[1].type) )
            {
                str = (char *)DB_GetXAssetName(asset);
                String = SL_GetString(str, 4u, SCRIPTINSTANCE_SERVER);
                name = SL_ConvertToString(String, SCRIPTINSTANCE_SERVER);
                DB_SetXAssetName(asset, name);
                v7 = (_WORD *)&asset[1].type + 1;
            }
            else
            {
                if ( LOWORD(asset[1].header.xmodelPieces)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                                5847,
                                0,
                                "%s",
                                "!assetEntry->nextOverride") )
                {
                    __debugbreak();
                }
                *v7 = HIWORD(asset[1].type);
                if ( !g_defaultAssetCount
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                                5849,
                                0,
                                "%s",
                                "g_defaultAssetCount") )
                {
                    __debugbreak();
                }
                --g_defaultAssetCount;
                if ( db_xassetdebug->current.enabled )
                {
                    if ( db_xassetdebugtype->current.integer == -1
                        || db_xassetdebugtype->current.integer == asset->type
                        || *Dvar_GetString("db_xassetdebugname")
                        && (v3 = Dvar_GetString("db_xassetdebugname"), v1 = DB_GetXAssetName(asset), !I_stricmp(v1, v3)) )
                    {
                        XAssetName = DB_GetXAssetName(asset);
                        XAssetTypeName = DB_GetXAssetTypeName(asset->type);
                        Com_Printf(16, "DB_FreeUnusedResources: freed asset: '%s','%s'\n", XAssetTypeName, XAssetName);
                    }
                }
                DB_FreeXAssetEntry((XAssetEntryPoolEntry *)asset);
            }
        }
    }
    SL_ShutdownSystem(SCRIPTINSTANCE_SERVER, 8u);
}

void __cdecl DB_EnableInUseCache(unsigned __int16 *data)
{
    if ( s_inuseCacheEnabled
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    4142,
                    0,
                    "%s",
                    "s_inuseCacheEnabled == false") )
    {
        __debugbreak();
    }
    s_inuseCacheEnabled = 1;
    s_inuseCacheHits = 0;
    s_inuseCacheMisses = 0;
    s_inuseCache = data;
    *(unsigned int *)data = -1;
}

bool DB_DisableInUseCache()
{
    bool result; // al

    result = s_inuseCacheEnabled;
    if ( !s_inuseCacheEnabled )
    {
        result = Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                             4154,
                             0,
                             "%s",
                             "s_inuseCacheEnabled == true");
        if ( !result )
            __debugbreak();
    }
    s_inuseCache = 0;
    s_inuseCacheEnabled = 0;
    return result;
}

void DB_ExternalInitAssets()
{
    BG_FillInAllWeaponItems();
}

// this is a hack, the real one is ifdef'd out
static void r_PumpDemonware()
{

}
//extern void __cdecl r_PumpDemonware(); // KISAKTODO: remove
void DB_StreamCompletePreload()
{
    R_StreamUpdate_SetupInitialImageList();
    if ( DB_IsZoneTypeLoaded(0x4000) )
        R_StreamUpdate_CompletePreload(r_PumpDemonware);
}

void DB_UnarchiveAssets()
{
    if ( !g_archiveBuf
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp", 5905, 0, "%s", "g_archiveBuf") )
    {
        __debugbreak();
    }
    g_archiveBuf = 0;
    //BLOPS_NULLSUB();
    DB_LoadDObjs();
    DB_RestartEffects_Finish();
    DB_ExternalInitAssets();
    if ( !Sys_IsMainThread()
        && !Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    5918,
                    0,
                    "%s",
                    "Sys_IsMainThread() || Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    //if ( Sys_IsMainThread() )
        //BLOPS_NULLSUB();
}

void __cdecl DB_Cleanup()
{
    Sys_SyncDatabase();
    if ( g_archiveBuf
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp", 5934, 0, "%s", "!g_archiveBuf") )
    {
        __debugbreak();
    }
}

int __cdecl DB_GetImageIndex(const GfxImage *image)
{
    unsigned int index = ((char *)image - (char *)g_GfxImagePool.entries) / sizeof(GfxImage);
    bcassert(index, ARRAY_COUNT(g_GfxImagePool.entries));
    return index;
}

XAssetPoolEntry<GfxImage> *__cdecl DB_GetImageAtIndex(unsigned int index)
{
    bcassert(index, ARRAY_COUNT(g_GfxImagePool.entries));
    return &g_GfxImagePool.entries[index];
}

int __cdecl DB_GetMaterialIndex(const Material *material)
{
    unsigned int index = ((char *)material - (char *)g_MaterialPool.entries) / sizeof(Material);
    bcassert(index, ARRAY_COUNT(g_MaterialPool.entries));
    return index;
}

XAssetPoolEntry<Material> *__cdecl DB_GetMaterialAtIndex(unsigned int index)
{
    bcassert(index, ARRAY_COUNT(g_MaterialPool.entries));
    return &g_MaterialPool.entries[index];
}

int __cdecl DB_GetXModelIndex(const XModel *model)
{
    unsigned int index = ((char *)model - (char *)g_XModelPool.entries) / sizeof(XModel);
    bcassert(index, ARRAY_COUNT(g_XModelPool.entries));
    return index;
}

XAssetPoolEntry<XModel> *__cdecl DB_GetXModelAtIndex(unsigned int index)
{
    if ( index >= 0x3E8
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    5997,
                    0,
                    "index doesn't index ARRAY_COUNT( g_XModelPool.entries )\n\t%i not in [0, %i)",
                    index,
                    1000) )
    {
        __debugbreak();
    }
    return &g_XModelPool.entries[index];
}

char *__cdecl DB_ReferencedFFChecksums()
{
    const char *LanguageNameAbbr; // eax
    int i; // [esp+4h] [ebp-1Ch]
    char zoneSizeStr[16]; // [esp+Ch] [ebp-14h] BYREF

    g_zoneNameList[0] = 0;
    for ( i = 0; i < 32; ++i )
    {
        if ( g_zoneNames[i].name[0] )
        {
            LanguageNameAbbr = SEH_GetLanguageNameAbbr(loc_language->current.unsignedInt);
            if ( I_strncmp(g_zoneNames[i].name, LanguageNameAbbr, 3) )
            {
                if ( g_zoneNameList[0] )
                    I_strncat(g_zoneNameList, 2080, " ");
                //itoa(g_zoneNames[i].fileSize, zoneSizeStr, 0xAu);
                _itoa(g_zoneNames[i].fileSize, zoneSizeStr, 10);
                I_strncat(g_zoneNameList, 2080, zoneSizeStr);
            }
        }
    }
    return g_zoneNameList;
}

char *__cdecl DB_ReferencedFFNameList()
{
    const char *LanguageNameAbbr; // eax
    const char *String; // eax
    int i; // [esp+4h] [ebp-8h]

    g_zoneNameList[0] = 0;
    for ( i = 0; i < 32; ++i )
    {
        if ( g_zoneNames[i].name[0] )
        {
            LanguageNameAbbr = SEH_GetLanguageNameAbbr(loc_language->current.integer);
            if ( I_strncmp(g_zoneNames[i].name, LanguageNameAbbr, 3) )
            {
                if ( g_zoneNameList[0] )
                    I_strncat(g_zoneNameList, 2080, " ");
                if ( g_zoneNames[i].dir == FFD_MOD_DIR )
                {
                    I_strncat(g_zoneNameList, 2080, fs_gameDirVar->current.string);
                    I_strncat(g_zoneNameList, 2080, "/");
                }
                else if ( g_zoneNames[i].dir == FFD_USER_MAP )
                {
                    I_strncat(g_zoneNameList, 2080, "usermaps");
                    I_strncat(g_zoneNameList, 2080, "/");
                    String = Dvar_GetString("fs_usermapdir");
                    I_strncat(g_zoneNameList, 2080, String);
                    I_strncat(g_zoneNameList, 2080, "/");
                }
                I_strncat(g_zoneNameList, 2080, g_zoneNames[i].name);
            }
        }
    }
    return g_zoneNameList;
}

int __cdecl DB_FileSize(const char *zoneName, FF_DIR source)
{
    char filename[260]; // [esp+0h] [ebp-110h] BYREF
    int size; // [esp+108h] [ebp-8h]
    void *zoneFile; // [esp+10Ch] [ebp-4h]

    if ( source == FFD_USER_MAP )
        DB_BuildOSPath_ForUsermap(zoneName, 0x100u, filename);
    else
        DB_BuildOSPath_FromSource(zoneName, source, 0x100u, filename);
    zoneFile = CreateFileA(filename, 0x80000000, 1u, 0, 3u, 0x60000000u, 0);
    if ( zoneFile == (void *)-1 )
        return 0;
    size = GetFileSize(zoneFile, 0);
    CloseHandle(zoneFile);
    return size;
}

char __cdecl DB_FileExists(const char *zoneName, FF_DIR source)
{
    char filename[256]; // [esp+0h] [ebp-108h] BYREF
    void *zoneFile; // [esp+104h] [ebp-4h]

    DB_BuildOSPath_FromSource(zoneName, source, 0x100u, filename);
    zoneFile = CreateFileA(filename, 0x80000000, 1u, 0, 3u, 0x60000000u, 0);
    if ( zoneFile == (void *)-1 )
        return 0;
    CloseHandle(zoneFile);
    return 1;
}

char __cdecl DB_ModFileExists()
{
    char filename[256]; // [esp+4h] [ebp-108h] BYREF
    void *zoneFile; // [esp+108h] [ebp-4h]

    if ( !fs_gameDirVar || !*(_BYTE *)fs_gameDirVar->current.integer )
        return 0;
    DB_BuildOSPath_FromSource("mod", FFD_MOD_DIR, 0x100u, filename);
    zoneFile = CreateFileA(filename, 0x80000000, 1u, 0, 3u, 0x60000000u, 0);
    if ( zoneFile == (void *)-1 )
        return 0;
    CloseHandle(zoneFile);
    return 1;
}

void __cdecl DB_AddUserMapDir(char *zoneName)
{
    char *v1; // eax

    if ( DB_FileExists(zoneName, FFD_DEFAULT) || !DB_FileExists(zoneName, FFD_USER_MAP) )
    {
        Dvar_SetStringByName("fs_usermapdir", (char *)"");
    }
    else
    {
        Dvar_SetStringByName("fs_usermapdir", zoneName);
        v1 = va("%s/%s", "usermaps", zoneName);
        FS_AddUserMapDirIWDs(v1);
    }
}

void __cdecl DB_LoadFastFilesForPC()
{
    XZoneInfo zoneInfo[6]; // [esp+0h] [ebp-50h] BYREF

    int zone = 0;

    if (!IsDedicatedServer())
    {
        //zoneInfo[zone].name = "patch_ui_mp";
        //zoneInfo[zone].allocFlags = 0x4000000;
        //zoneInfo[zone].freeFlags = 0;
        //zone++;

        zoneInfo[zone].name = "ui_mp";
        zoneInfo[zone].allocFlags = 0x4000000;//  0x2000000; (KISAKTODO: flag fix, this flag doesn't load for some reason)
        zoneInfo[zone].freeFlags = 0;
        zone++;

        DB_LoadXAssets(zoneInfo, zone, 0);
    }

    zone = 0;

    zoneInfo[zone].name = "common_mp";
#ifdef KISAK_DEDICATED
    zoneInfo[zone].allocFlags = 256;
#else
    zoneInfo[zone].allocFlags = 64;
#endif
    zoneInfo[zone].freeFlags = 0;
    zone++;

    if (!IsDedicatedServer())
    {
        zoneInfo[zone].name = "ui_viewer_mp";
        zoneInfo[zone].allocFlags = 2048;
        zoneInfo[zone].freeFlags = 0;
        zone++;
    }

    DB_LoadXAssets(zoneInfo, zone, 0);
}

void __cdecl DB_LoadGraphicsAssetsForPC()
{
    XZoneInfo zoneInfo[6]; // [esp+0h] [ebp-50h] BYREF
    unsigned int zoneCount; // [esp+4Ch] [ebp-4h]

    zoneInfo[0].name = "code_post_gfx_mp";
    zoneInfo[0].allocFlags = 1;
    zoneInfo[0].freeFlags = 0x80000000;
    zoneCount = 1;
    DB_LoadXAssets(zoneInfo, zoneCount, 0);
    DB_SyncXAssets();
    zoneInfo[0].name = "dev_mp";
    zoneInfo[0].allocFlags = 4;
    zoneInfo[0].freeFlags = 0;
    zoneInfo[1].name = "patch_mp";
    zoneInfo[1].allocFlags = 16;
    zoneInfo[1].freeFlags = 0;
    zoneCount = 2;
    if ( DB_ModFileExists() )
    {
        zoneInfo[zoneCount].name = "mod";
        zoneInfo[zoneCount].allocFlags = 64;
        zoneInfo[zoneCount++].freeFlags = 0;
    }
    if ( zoneCount > 6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_registry.cpp",
                    6304,
                    0,
                    "%s",
                    "zoneCount <= MAX_ZONE_INFO") )
    {
        __debugbreak();
    }
    DB_LoadXAssets(zoneInfo, zoneCount, 0);
}

