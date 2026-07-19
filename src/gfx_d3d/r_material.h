#pragma once

#include <d3d9.h>
#include <database/database.h>
#include <universal/fft.h>
#include "r_gfx.h"

#define STREAM_SRC_COUNT 10

enum surfaceType_t : __int32
{                                       // XREF: ?R_AddXModelSurfacesCamera@@YAXPIAUXModelDrawInfo@@PIBUXModel@@QAMIIIHHQAPIATGfxDrawSurf@@3III@Z/r
                                        // R_GetXSurface/r ...
    SF_TRIANGLES            = 0x0,
    SF_TRIANGLES_PRETESS    = 0x1,
    SF_BEGIN_STATICMODEL    = 0x2,
    SF_STATICMODEL_RIGID    = 0x2,
    SF_STATICMODEL_PRETESS  = 0x3,
    SF_STATICMODEL_CACHED   = 0x4,
    SF_STATICMODEL_SKINNED  = 0x5,
    SF_END_STATICMODEL      = 0x6,
    SF_BMODEL               = 0x6,
    SF_BEGIN_XMODEL         = 0x7,
    SF_XMODEL_RIGID         = 0x7,
    SF_XMODEL_RIGID_SKINNED = 0x8,
    SF_XMODEL_SKINNED       = 0x9,
    SF_XMODEL_WATER         = 0xA,
    SF_END_XMODEL           = 0xB,
    SF_BEGIN_FX             = 0xB,
    SF_CODE_MESH            = 0xB,
    SF_MARK_MESH            = 0xC,
    SF_PARTICLE_CLOUD       = 0xD,
    SF_ROPE_MESH            = 0xE,
    SF_GLASS_MESH           = 0xF,
    SF_END_FX               = 0x10,
    SF_NUM_SURFACE_TYPES    = 0x10,
    SF_FORCE_32_BITS        = -1,
};

enum GfxDepthRangeType : __int32
{                                       // XREF: GfxCmdBufState/r
                                        // ?R_ChangeDepthRange@@YAXPAUGfxCmdBufState@@W4GfxDepthRangeType@@@Z/r ...
    GFX_DEPTH_RANGE_SCENE     = 0x0,
    GFX_DEPTH_RANGE_VIEWMODEL = 0x2,
    GFX_DEPTH_RANGE_FULL      = -1,
};

enum MaterialVertexDeclType : __int32
{                                       // XREF: GfxCmdBufPrimState/r
                                        // materialCommands_t/r ...
    VERTDECL_GENERIC          = 0x0,
    VERTDECL_PACKED           = 0x1,
    VERTDECL_WORLD            = 0x2,
    VERTDECL_WORLD_T1N0       = 0x3,
    VERTDECL_WORLD_T1N1       = 0x4,
    VERTDECL_WORLD_T2N0       = 0x5,
    VERTDECL_WORLD_T2N1       = 0x6,
    VERTDECL_WORLD_T2N2       = 0x7,
    VERTDECL_WORLD_T3N0       = 0x8,
    VERTDECL_WORLD_T3N1       = 0x9,
    VERTDECL_WORLD_T3N2       = 0xA,
    VERTDECL_WORLD_T4N0       = 0xB,
    VERTDECL_WORLD_T4N1       = 0xC,
    VERTDECL_WORLD_T4N2       = 0xD,
    VERTDECL_POS_TEX          = 0xE,
    VERTDECL_STATICMODELCACHE = 0xF,
    VERTDECL_WATER            = 0x10,
    VERTDECL_PARTICLECLOUD    = 0x11,
    VERTDECL_COUNT            = 0x12,
};

struct dmaterial_t // sizeof=0x48
{
    char material[64];
    int surfaceFlags;
    int contentFlags;
};

struct __declspec(align(8)) MaterialInfo // sizeof=0x28
{                                                                             // XREF: Material/r
                                                                                // FX_ConvertAtlas/r ...
        const char *name;                                     // XREF: FX_ValidateAtlasSettings+92/r
                                                                                // FX_ValidateAtlasSettings+DD/r ...
        unsigned int gameFlags;
        unsigned __int8 pad;
        unsigned __int8 sortKey;
        unsigned __int8 textureAtlasRowCount;
                                                                                // XREF: FX_Beam_GenerateVerts(FxGenerateVertsCmd *)+8CA/r
                                                                                // FX_Beam_GenerateVerts(FxGenerateVertsCmd *):loc_5C10FA/r ...
        unsigned __int8 textureAtlasColumnCount;
                                                                                // XREF: FX_Beam_GenerateVerts(FxGenerateVertsCmd *)+8BE/r
                                                                                // FX_Beam_GenerateVerts(FxGenerateVertsCmd *)+8E1/r ...
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        GfxDrawSurf drawSurf;
        unsigned int surfaceTypeBits;
        unsigned int layeredSurfaceTypes;
        unsigned __int16 hashIndex;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
};

struct MaterialStreamRouting // sizeof=0x2
{                                                                             // XREF: MaterialVertexStreamRouting/r
                                                                                // ?Load_BuildVertexDecl@@YAXPAPAUMaterialVertexDeclaration@@@Z/r ...
        unsigned __int8 source;                         // XREF: Material_LoadPassVertexDecl+26A/w
                                                                                // Material_LoadPassVertexDecl+277/w
        unsigned __int8 dest;                             // XREF: Material_LoadPassVertexDecl+281/w
};

struct MaterialVertexStreamRouting // sizeof=0x68
{                                                                             // XREF: MaterialVertexDeclaration/r
        MaterialStreamRouting data[16];
        IDirect3DVertexDeclaration9 *decl[18];
};

struct MaterialVertexDeclaration // sizeof=0x6C
{                                                                             // XREF: $54435CF730F84DB67694F69169881761/r
        unsigned __int8 streamCount;
        bool hasOptionalSource;
        bool isLoaded;
        // padding byte
        MaterialVertexStreamRouting routing;
};

struct __declspec(align(4)) GfxVertexShaderLoadDef // sizeof=0x8
{                                                                             // XREF: MaterialVertexShaderProgram/r
        DWORD *program;
        unsigned __int16 programSize;
        // padding byte
        // padding byte
};

struct MaterialVertexShaderProgram // sizeof=0xC
{                                                                             // XREF: MaterialVertexShader/r
        IDirect3DVertexShader9 *vs;
        GfxVertexShaderLoadDef loadDef;
};

struct MaterialVertexShader // sizeof=0x10
{
        const char *name;
        MaterialVertexShaderProgram prog;
};

struct __declspec(align(4)) GfxPixelShaderLoadDef // sizeof=0x8
{                                                                             // XREF: MaterialPixelShaderProgram/r
        DWORD *program;
        unsigned __int16 programSize;
        // padding byte
        // padding byte
};

struct MaterialPixelShaderProgram // sizeof=0xC
{                                                                             // XREF: MaterialPixelShader/r
        IDirect3DPixelShader9 *ps;
        GfxPixelShaderLoadDef loadDef;
};

struct MaterialPixelShader // sizeof=0x10
{
        const char *name;
        MaterialPixelShaderProgram prog;
};

struct MaterialArgumentCodeConst // sizeof=0x4
{                                                                             // XREF: MaterialArgumentDef/r
                                                                                // GfxShaderConstantState::<unnamed_type_fields>/r
        unsigned __int16 index;
        unsigned __int8 firstRow;
        unsigned __int8 rowCount;
};

union MaterialArgumentDef // sizeof=0x4
{                                                                             // XREF: MaterialShaderArgument/r
        const float *literalConst;
        MaterialArgumentCodeConst codeConst;
        unsigned int codeSampler;
        unsigned int nameHash;
};

struct MaterialShaderArgument // sizeof=0x8
{                                                                             // XREF: Material_LoadPass/r
                                                                                // Material_ParseShaderArguments/r
        unsigned __int16 type;
        unsigned __int16 dest;
        MaterialArgumentDef u;
};

struct MaterialPass // sizeof=0x14
{                                                                             // XREF: MaterialTechnique/r
                                                                                // Material_LoadTechnique/r
        MaterialVertexDeclaration *vertexDecl;
                                                                                // XREF: Material_LoadTechnique+3A6/r
        MaterialVertexShader *vertexShader;
        //$DC376F3ECA4A0877231069DAD65BD3EF ___u2;
        union //$DC376F3ECA4A0877231069DAD65BD3EF // sizeof=0x4
        {                                                                             // XREF: MaterialPass/r
            MaterialPixelShader *pixelShader;
            MaterialPixelShader *localPixelShader;
        };
        unsigned __int8 perPrimArgCount;
        unsigned __int8 perObjArgCount;
        unsigned __int8 stableArgCount;
        unsigned __int8 customSamplerFlags;
        //$852269A6FD668DA4A9FB8DA442445F23 ___u7;
        union //$852269A6FD668DA4A9FB8DA442445F23 // sizeof=0x4
        {                                                                             // XREF: MaterialPass/r
            MaterialShaderArgument *localArgs;
            MaterialShaderArgument *args;
        };
};

struct MaterialTechnique // sizeof=0x1C
{
        const char *name;
        unsigned __int16 flags;
        unsigned __int16 passCount;
        MaterialPass passArray[1];
};

struct MaterialTechniqueSet // sizeof=0x210
{                                                                             // XREF: XAssetPoolEntry<MaterialTechniqueSet>/r
        const char *name;
        unsigned __int8 worldVertFormat;
        unsigned __int8 unused[1];
        unsigned __int16 techsetFlags;
        MaterialTechnique *techniques[130];
};
static_assert(sizeof(MaterialTechniqueSet) == 528);

struct __declspec(align(4)) GfxImageLoadDef // sizeof=0x10
{
        unsigned __int8 levelCount;
        unsigned __int8 flags;
        // padding byte
        // padding byte
        int format;
        int resourceSize;
        unsigned __int8 data[1];
        // padding byte
        // padding byte
        // padding byte
};

union GfxTexture // sizeof=0x4
{                                                                             // XREF: GfxImage/r
        IDirect3DBaseTexture9 *basemap;
        IDirect3DTexture9 *map;
        IDirect3DVolumeTexture9 *volmap;
        IDirect3DCubeTexture9 *cubemap;
        GfxImageLoadDef *loadDef;
};

struct Picmip // sizeof=0x2
{                                                                             // XREF: GfxImage/r
                                                                                // ?Image_UpdatePicmip@@YAXPAUGfxImage@@@Z/r
        unsigned __int8 platform[2];                // XREF: Image_UpdatePicmip(GfxImage *)+60/r
};

struct CardMemory // sizeof=0x8
{                                                                             // XREF: GfxImage/r
                                                                                // $240FF632105687D15E57E43F247C9BAA/r
        int platform[2];                                        // XREF: Image_TrackTotalMemory(GfxImage *,int,int)+2A/r
                                                                                // Image_TrackTotalMemory(GfxImage *,int,int)+34/w ...
};

struct GfxImageFileHeader // sizeof=0x30
{                                       // XREF: ?Image_LoadToBuffer@@YA_NPAUGfxImage@@_NPAPAEPAH@Z/r
                                        // _Image_LoadFromFileWithReader/r
    char tag[3];
    unsigned __int8 version;
    unsigned __int8 format;
    unsigned __int8 flags;              // XREF: Image_LoadToBuffer(GfxImage *,bool,uchar * *,int *):loc_B2A604/r
    // _Image_LoadFromFileWithReader:loc_B2AADC/r
    __int16 dimensions[3];              // XREF: Image_LoadToBuffer(GfxImage *,bool,uchar * *,int *)+1A0/r
    // Image_LoadToBuffer(GfxImage *,bool,uchar * *,int *)+1AA/r ...
    float gamma;
    int fileSizeForPicmip[8];           // XREF: Image_LoadToBuffer(GfxImage *,bool,uchar * *,int *)+202/r
    // Image_LoadToBuffer(GfxImage *,bool,uchar * *,int *)+20E/r ...
};

struct GfxImage // sizeof=0x34
{                                                                             // XREF: .data:g_imageProgs/r
                                                                                // XAssetPoolEntry<GfxImage>/r ...
        GfxTexture texture;
        unsigned __int8 mapType;                        // XREF: R_ImageList_Output(void)+88/r
        unsigned __int8 semantic;
        unsigned __int8 category;
        bool delayLoadPixels;
        Picmip picmip;
        bool noPicmip;
        unsigned __int8 track;
        CardMemory cardMemory;
        unsigned __int16 width;
        unsigned __int16 height;
        unsigned __int16 depth;
        unsigned __int8 levelCount;
        unsigned __int8 streaming;
        unsigned int baseSize;
        unsigned __int8 *pixels;
        unsigned int loadedSize;
        unsigned __int8 skippedMipLevels;
        // padding byte
        // padding byte
        // padding byte
        const char *name;
        unsigned int hash;
};

struct WaterWritable // sizeof=0x4
{                                                                             // XREF: water_t/r
        float floatTime;
};

struct water_t // sizeof=0x44
{                                                                             // XREF: .data:sceneWaterMapSetups/r
                                                                                // Material_RegisterWaterImage/r
        WaterWritable writable;
        complex_s *H0;
        float *wTerm;
        int M;                                                            // XREF: Material_RegisterWaterImage+17/w
                                                                                // Material_RegisterWaterImage+1A/r ...
        int N;                                                            // XREF: Material_RegisterWaterImage+1D/w
                                                                                // Material_RegisterWaterImage+B4/r ...
        float Lx;                                                     // XREF: Material_RegisterWaterImage+28/w
        float Lz;                                                     // XREF: Material_RegisterWaterImage+35/w
        float gravity;                                            // XREF: Material_RegisterWaterImage+42/w
        float windvel;                                            // XREF: Material_RegisterWaterImage+4F/w
        float winddir[2];                                     // XREF: Material_RegisterWaterImage+5C/w
                                                                                // Material_RegisterWaterImage+69/w
        float amplitude;                                        // XREF: Material_RegisterWaterImage+76/w
        float codeConstant[4];
        GfxImage *image;                                        // XREF: Material_RegisterWaterImage+7B/w
};

union MaterialTextureDefInfo // sizeof=0x4
{                                                                             // XREF: MaterialTextureDef/r
        GfxImage *image;
        water_t *water;
};

struct MaterialTextureDef // sizeof=0x10
{
        unsigned int nameHash;
        char nameStart;
        char nameEnd;
        unsigned __int8 samplerState;
        unsigned __int8 semantic;
        unsigned __int8 isMatureContent;
        unsigned __int8 pad[3];
        MaterialTextureDefInfo u;
};

struct MaterialConstantDef // sizeof=0x20
{                                                                             // XREF: .data:MaterialParameterConstantEntry_save/r
        unsigned int nameHash;                            // XREF: R_MaterialParameterTweakUpdate(void)+16/r
        char name[12];
        float literal[4];
};

struct GfxStateBits // sizeof=0x8
{
        unsigned int loadBits[2];
};

struct Material // sizeof=0xC0
{                                                                             // XREF: XAssetPoolEntry<Material>/r
        MaterialInfo info;
        unsigned __int8 stateBitsEntry[130];
        unsigned __int8 textureCount;
        unsigned __int8 constantCount;
        unsigned __int8 stateBitsCount;
        unsigned __int8 stateFlags;
        unsigned __int8 cameraRegion;
        unsigned __int8 maxStreamedMips;
        //$090A22A23743445EB0E711FA011CE2F0 ___u8;
        union //$090A22A23743445EB0E711FA011CE2F0 // sizeof=0x4
        {                                                                             // XREF: Material/r
            MaterialTechniqueSet *localTechniqueSet;
            MaterialTechniqueSet *techniqueSet;
        };
        MaterialTextureDef *textureTable;
        //$A4FF57C4AF6A63DB9C0EEC1063D439DF ___u10;
        union //$A4FF57C4AF6A63DB9C0EEC1063D439DF // sizeof=0x4
        {                                                                             // XREF: Material/r
            MaterialConstantDef *localConstantTable;
            MaterialConstantDef *constantTable;
        };
        GfxStateBits *stateBitsTable;
};
static_assert(sizeof(Material) == 192);


struct stream_source_info_t // sizeof=0x3
{                                                                             // XREF: .rdata:s_streamSourceInfo/r
        unsigned __int8 Stream;
        unsigned __int8 Offset;
        unsigned __int8 Type;
};

enum MtlTechSetNotFoundBehavior : __int32
{                                                                             // XREF: ?Material_FindTechniqueSet@@YAPAUMaterialTechniqueSet@@PBDW4MtlTechSetNotFoundBehavior@@@Z/r
                                                                                // Material_FindTechniqueSet_LoadObj/r ...
        MTL_TECHSET_NOT_FOUND_RETURN_NULL = 0x0,
        MTL_TECHSET_NOT_FOUND_RETURN_DEFAULT = 0x1,
};

struct MaterialMemory // sizeof=0x8
{                                                                             // XREF: MaterialList/r
        Material *material;                                 // XREF: std::_Sort_heap<MaterialMemory *,bool (*)(MaterialMemory const &,MaterialMemory const &)>(MaterialMemory *,MaterialMemory *,bool (*)(MaterialMemory const &,MaterialMemory const &))+4E/w
        int memory;                                                 // XREF: std::_Sort_heap<MaterialMemory *,bool (*)(MaterialMemory const &,MaterialMemory const &)>(MaterialMemory *,MaterialMemory *,bool (*)(MaterialMemory const &,MaterialMemory const &))+51/w
};

struct PerMapMaterialTable // sizeof=0x8
{                                                                             // XREF: .rdata:s_permapMaterials/r
        const char *name;
        Material **material;
};

struct BuiltInMaterialTable // sizeof=0x8
{                                                                             // XREF: .rdata:s_builtInMaterials/r
        const char *name;
        Material **material;
};

struct stream_dest_info_t // sizeof=0x2
{                                       // XREF: .rdata:s_streamDestInfo/r
    unsigned __int8 Usage;
    unsigned __int8 UsageIndex;
};

void __cdecl Material_FreeTechniqueSet(XAssetHeader header);
unsigned __int8 *__cdecl Material_Alloc(unsigned int size);
void __cdecl Load_CreateMaterialPixelShader(GfxPixelShaderLoadDef *loadDef, MaterialPixelShader *mtlShader);
void __cdecl Load_CreateMaterialVertexShader(GfxVertexShaderLoadDef *loadDef, MaterialVertexShader *mtlShader);
void __cdecl Load_BuildVertexDecl(MaterialVertexDeclaration **mtlVertDecl);
IDirect3DVertexDeclaration9 *__cdecl Material_BuildVertexDecl(
                const MaterialStreamRouting *routingData,
                int streamCount,
                const stream_source_info_t *sourceTable);
MaterialTechniqueSet *__cdecl Material_FindTechniqueSet(const char *name, MtlTechSetNotFoundBehavior notFoundBehavior);
MaterialTechniqueSet *__cdecl Material_FindTechniqueSet_LoadObj(
                const char *name,
                MtlTechSetNotFoundBehavior notFoundBehavior);
char __cdecl MaterialTechniqueSet_FindHashLocation(const char *name, int *foundHashIndex);
MaterialTechniqueSet *__cdecl Material_FindTechniqueSet_FastFile(
                const char *name,
                MtlTechSetNotFoundBehavior notFoundBehavior);
void __cdecl Material_SetAlwaysUseDefaultMaterial(bool alwaysUseDefaultMaterial);
void __cdecl Material_PreventOverrideTechniqueGeneration();
MaterialTechniqueSet *__cdecl Material_RegisterTechniqueSet(char *name);
void __cdecl Material_SetTechniqueSet(const char *name, MaterialTechniqueSet *techniqueSet);
void __cdecl Material_ReleaseTechniqueSetResources(MaterialTechniqueSet *techniqueSet);
void __cdecl Material_ReleasePassResources(MaterialPass *pass);
void __cdecl Material_FreeAllTechniqueSets();
void __cdecl Material_DirtySort();
Material *__cdecl Material_Duplicate(Material *mtlCopy, char *name);
void __cdecl Material_Add(Material *material, unsigned __int16 hashIndex);
bool __cdecl Material_IsDefault(const Material *material);
void __cdecl Material_GetHashIndex(const char *name, unsigned __int16 *hashIndex, bool *exists);
Material *__cdecl Material_Register(char *name, int imageTrack);
Material *__cdecl Material_Register_LoadObj(char *name, int imageTrack);
Material *__cdecl Material_MakeDefault(char *name);
Material *__cdecl Material_Register_FastFile(const char *name);
Material *__cdecl Material_RegisterHandle(char *name, int imageTrack);
inline Material *Material_RegisterHandle(const char *name, int imgTrack)
{
    return Material_RegisterHandle((char *)name, imgTrack);
}
void __cdecl R_MaterialList_f();
void __cdecl R_GetMaterialList(XAssetHeader header, char *data);
int __cdecl R_GetMaterialMemory(Material *material);
bool __cdecl R_MaterialCompare(const MaterialMemory &material0, const MaterialMemory &material1);
void __cdecl R_MaterialEdit_f();
void __cdecl R_MaterialParameterEdit_f();
void __cdecl R_MaterialParameterTweak_f();
void __cdecl R_MaterialParameterTweakUpdate();
void __cdecl R_EnumTechniqueSets(void (__cdecl *func)(MaterialTechniqueSet *, void *), void *data);
void __cdecl R_EnumMaterials(void (__cdecl *func)(Material *, void *), void *data);
const char *__cdecl Material_GetName(Material *handle);
void __cdecl R_InitAssets_PostMapFastfileLoad();
void __cdecl Material_LoadPerMap(const PerMapMaterialTable *mtlTable, int mtlTableCount);
void __cdecl Material_Init();
void __cdecl Material_LoadBuiltIn(const BuiltInMaterialTable *mtlTable, int mtlTableCount);
void __cdecl Material_Shutdown();
void __cdecl Material_UpdatePicmipAll();
void __cdecl Material_UpdatePicmipSingle(XAssetHeader header);
void __cdecl Material_UpdatePicmipForTexdef(const MaterialTextureDef *texdef);
void __cdecl R_Cmd_ReloadMaterialTextures();
Material *__cdecl Material_Find(const char *name);
void __cdecl Material_ReloadTextures(const Material *material);
unsigned int __cdecl Material_LoadFile(const char *filename, int *file);
bool __cdecl IsValidMaterialHandle(Material *const handle);
int __cdecl GetCodeConst_GenericParam0();
int __cdecl GetCodeConst_GenericParam1();
int __cdecl GetCodeConst_GenericParam2();
int __cdecl GetCodeConst_GenericParam3();
int __cdecl GetCodeConst_GenericParamA();
int __cdecl GetCodeConst_GenericParamB();
int __cdecl GetCodeConst_GameTimeParam();
int __cdecl Material_GetTextureCount(Material *handle);
GfxImage *__cdecl Material_GetTexture(Material *handle, int index);


extern bool skipShaderCreationHack;
