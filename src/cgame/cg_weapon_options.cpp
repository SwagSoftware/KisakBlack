#include "cg_weapon_options.h"
#include <gfx_d3d/r_shader_constant_set.h>
#include <gfx_d3d/r_scene.h>
#include <gfx_d3d/r_model_lod.h>
#include <live/live_pcache.h>
#include <database/db_registry.h>

const dvar_t *weaponCamoLodDist;
const dvar_t *weaponEmblemLodDist;
const dvar_t *weaponClanTagLodDist;
const dvar_t *facepaintLodDist;

WeaponOptions weaponOptions;

const GfxImage *__cdecl WeaponOptions::GetImageFromTable(const StringTable *table, int row, int column)
{
    XAssetHeader v4; // [esp+0h] [ebp-1Ch]
    const char *name; // [esp+18h] [ebp-4h]

    name = StringTable_GetColumnValueForRow(table, row, column);
    if ( !name || !strlen(name) )
        return 0;
    v4.xmodelPieces = DB_FindXAssetHeader(ASSET_TYPE_IMAGE, (char*)name, 1, -1).xmodelPieces;
    return (const GfxImage *)v4.xmodelPieces;
}

void __thiscall WeaponOptions::InitWeaponOptionTextures(
                const char *tableName,
                const char *label,
                int labelCol,
                int textureCol,
                const GfxImage **overrides,
                unsigned int num)
{
    int i; // [esp+4h] [ebp-20Ch]
    const StringTable *tablePtr; // [esp+8h] [ebp-208h] BYREF
    int nRows; // [esp+Ch] [ebp-204h]
    int rows[128]; // [esp+10h] [ebp-200h] BYREF

    StringTable_GetAsset(tableName, (XAssetHeader *)&tablePtr);
    if ( num >= 0x80
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_weapon_options.cpp",
                    120,
                    0,
                    "%s",
                    "num < ARRAY_COUNT(rows)") )
    {
        __debugbreak();
    }
    nRows = StringTable_Find(tablePtr, rows, num, labelCol, label, -1, 0);
    for ( i = 0; i < nRows; ++i )
        overrides[i] = WeaponOptions::GetImageFromTable(tablePtr, rows[i], textureCol);
}

void __thiscall WeaponOptions::InitWeaponOptionMaterials(
                const char *tableName,
                const char *label,
                int labelCol,
                int materialCol,
                Material **overrides,
                unsigned int num)
{
    const char *materialName; // [esp+18h] [ebp-210h]
    int i; // [esp+1Ch] [ebp-20Ch]
    const StringTable *tablePtr; // [esp+20h] [ebp-208h] BYREF
    int nRows; // [esp+24h] [ebp-204h]
    int rows[128]; // [esp+28h] [ebp-200h] BYREF

    StringTable_GetAsset(tableName, (XAssetHeader *)&tablePtr);
    if ( num >= 0x80
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_weapon_options.cpp",
                    135,
                    0,
                    "%s",
                    "num < ARRAY_COUNT(rows)") )
    {
        __debugbreak();
    }
    nRows = StringTable_Find(tablePtr, rows, num, labelCol, label, -1, 0);
    for ( i = 0; i < nRows; ++i )
    {
        materialName = StringTable_GetColumnValueForRow(tablePtr, rows[i], materialCol);
        if ( materialName )
        {
            if ( strlen(materialName) )
            {
                overrides[i] = DB_FindXAssetHeader(ASSET_TYPE_MATERIAL, (char*)materialName, 1, -1).material;
                if ( overrides[i] )
                {
                    if ( this->facepaintMaterials[0]->textureCount != 1 )
                    {
                        Com_PrintWarning(0, "Weapon option material '%s' can have only 1 texture\n", materialName);
                        overrides[i] = 0;
                    }
                }
            }
        }
    }
}

void __thiscall WeaponOptions::InitWeaponOptionColors(
                const char *tableName,
                const char *label,
                int labelCol,
                int colorsCol,
                float (*colors)[4])
{
    float *v6; // [esp+14h] [ebp-60h]
    float *v7; // [esp+1Ch] [ebp-58h]
    const char *colorsStr; // [esp+20h] [ebp-54h]
    int j; // [esp+24h] [ebp-50h]
    unsigned int i; // [esp+28h] [ebp-4Ch]
    const StringTable *tablePtr; // [esp+2Ch] [ebp-48h] BYREF
    int nRows; // [esp+30h] [ebp-44h]
    int rows[16]; // [esp+34h] [ebp-40h] BYREF

    for ( i = 0; i < 0x10; ++i )
    {
        v7 = &(*colors)[4 * i];
        *v7 = -1.0f;
        v7[1] = -1.0f;
        v7[2] = -1.0f;
        v7[3] = -1.0f;
    }
    StringTable_GetAsset(tableName, (XAssetHeader *)&tablePtr);
    nRows = StringTable_Find(tablePtr, rows, 16, labelCol, label, -1, 0);
    for ( j = 0; j < nRows; ++j )
    {
        colorsStr = StringTable_GetColumnValueForRow(tablePtr, rows[j], colorsCol);
        if ( colorsStr )
        {
            if ( strlen(colorsStr) )
            {
                sscanf(colorsStr, "%f %f %f", &(*colors)[4 * j], &(*colors)[4 * j + 1], &(*colors)[4 * j + 2]);
                v6 = &(*colors)[4 * j];
                *v6 = 0.0039215689 * *v6;
                v6[1] = 0.0039215689 * v6[1];
                v6[2] = 0.0039215689 * v6[2];
                v6[3] = 0.0039215689 * v6[3];
            }
        }
    }
}

void __thiscall WeaponOptions::InitWeaponOptions()
{
    const char *v1; // eax
    unsigned int i; // [esp+10h] [ebp-10h]
    const StringTable *tablePtr; // [esp+14h] [ebp-Ch] BYREF
    int goldRow; // [esp+18h] [ebp-8h]
    int blackRow; // [esp+1Ch] [ebp-4h]

    memset(this, 0, sizeof(WeaponOptions));
    for ( i = 0; i < 0xA; ++i )
    {
        WeaponOptions::InitWeaponOptionTextures(
            "mp/weaponoptions.csv",
            "camo",
            1,
            i + 2,
            this->camoTextureOverrides[i],
            0x40u);
        WeaponOptions::InitWeaponOptionTextures(
            "mp/weaponoptions.csv",
            "lens",
            1,
            i + 3,
            this->lensTextureOverrides[i],
            0x10u);
        WeaponOptions::InitWeaponOptionTextures(
            "mp/weaponoptions.csv",
            "reticle",
            1,
            i + 2,
            this->reticleTextureOverrides[i],
            0x40u);
    }
    WeaponOptions::InitWeaponOptionColors("mp/weaponoptions.csv", "reticle_color", 1, 2, this->reticleColors);
    WeaponOptions::InitWeaponOptionColors("mp/weaponoptions.csv", "lens", 1, 2, this->lensColors);
    WeaponOptions::InitWeaponOptionMaterials(
        "mp/bodyHeadTable.csv",
        "pattern",
        1,
        3,
        this->facepaintMaterials,
        0x40u);
    WeaponOptions::InitWeaponOptionColors("mp/bodyHeadTable.csv", "pattern_color", 1, 3, this->facepaintColors);
    this->numWeaponOverrides = 0;
    StringTable_GetAsset("mp/weaponoptions.csv", (XAssetHeader *)&tablePtr);
    goldRow = StringTable_LookupRowNumForValue(tablePtr, 1, "gold");
    if ( goldRow != -1 )
    {
        this->gold.diffuseTexture = WeaponOptions::GetImageFromTable(tablePtr, goldRow, 2);
        this->gold.specularTexture = WeaponOptions::GetImageFromTable(tablePtr, goldRow, 3);
        this->gold.reflectionTexture = WeaponOptions::GetImageFromTable(tablePtr, goldRow, 4);
        this->gold.camoTexture = WeaponOptions::GetImageFromTable(tablePtr, goldRow, 5);
    }
    blackRow = StringTable_LookupRowNumForValue(tablePtr, 1, "black");
    if ( blackRow != -1 )
    {
        this->black.diffuseTexture = WeaponOptions::GetImageFromTable(tablePtr, blackRow, 2);
        this->black.specularTexture = WeaponOptions::GetImageFromTable(tablePtr, blackRow, 3);
        this->black.reflectionTexture = WeaponOptions::GetImageFromTable(tablePtr, blackRow, 4);
        this->black.camoTexture = WeaponOptions::GetImageFromTable(tablePtr, blackRow, 5);
    }
    v1 = StringTable_Lookup(tablePtr, 1, "camo", 2, "gold", 0);
    this->goldCamoIndex = atoi(v1);
    this->emblemClearTexture = DB_FindXAssetHeader(ASSET_TYPE_IMAGE, (char*)"emblem_clear", 1, -1).image;
    this->lightClanTagFont = DB_FindXAssetHeader(ASSET_TYPE_IMAGE, (char *)"clan_font_c", 1, -1).image;
    this->darkClanTagFont = DB_FindXAssetHeader(ASSET_TYPE_IMAGE, (char *)"clan_font_dark_c", 1, -1).image;
}

void __cdecl GC_InitWeaponOptionsDvars()
{
    weaponCamoLodDist = _Dvar_RegisterFloat(
                                                "weaponCamoLodDist",
                                                1250.0,
                                                10.0,
                                                100000.0,
                                                0,
                                                "Distance to stop drawing the weapon camo.");
    weaponEmblemLodDist = _Dvar_RegisterFloat(
                                                    "weaponEmblemLodDist",
                                                    300.0,
                                                    10.0,
                                                    100000.0,
                                                    0,
                                                    "Distance to stop drawing the weapon emblem.");
    weaponClanTagLodDist = _Dvar_RegisterFloat(
                                                     "weaponClanTagLodDist",
                                                     300.0,
                                                     10.0,
                                                     100000.0,
                                                     0,
                                                     "Distance to stop drawing the weapon clan tag.");
    facepaintLodDist = _Dvar_RegisterFloat(
                                             "facepaintLodDist",
                                             1250.0,
                                             10.0,
                                             100000.0,
                                             0,
                                             "Distance to stop drawing the player facepaint.");
}

unsigned int __thiscall WeaponOptions::FindCamoOverrideIndex(const GfxImage *base, unsigned int defaultIndex)
{
    unsigned int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 0xA; ++i )
    {
        if ( this->camoTextureOverrides[i][0] == base )
            return i;
    }
    return defaultIndex;
}

void __thiscall WeaponOptions::WeaponOverride::AddToGold(const char *matName, bool gold)
{
    if ( gold )
    {
        if ( this->numToGold >= 6
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_weapon_options.cpp",
                        262,
                        0,
                        "%s",
                        "numToGold < ARRAY_COUNT( toGold )") )
        {
            __debugbreak();
        }
        //if ( WeaponOptions::WeaponOverride::ToGold::Init(&this->toGold[this->numToGold], matName) )
        if ( this->toGold[this->numToGold].Init(matName) )
            ++this->numToGold;
    }
    else
    {
        if ( this->numToBlack >= 6
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_weapon_options.cpp",
                        268,
                        0,
                        "%s",
                        "numToBlack < ARRAY_COUNT( toBlack )") )
        {
            __debugbreak();
        }
        //if ( WeaponOptions::WeaponOverride::ToGold::Init(&this->toBlack[this->numToBlack], matName) )
        if ( this->toBlack[this->numToBlack].Init(matName) )
            ++this->numToBlack;
    }
}

void __thiscall WeaponOptions::WeaponOverride::Init(const WeaponVariantDef *weapVarDef)
{
    const char *ColumnValueForRow; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    const char *v13; // eax
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
    const char *v25; // eax
    const char *v26; // eax
    const char *v27; // eax
    const char *v28; // eax
    const char *v29; // eax
    const StringTable *tablePtr; // [esp+4h] [ebp-8h] BYREF
    int row; // [esp+8h] [ebp-4h]

    StringTable_GetAsset("mp/weaponoptions.csv", (XAssetHeader *)&tablePtr);
    this->lastFrame = com_frameNumber;
    this->weapon = weapVarDef;
    row = StringTable_LookupRowNumForValue(tablePtr, 1, "weapon", 2, weapVarDef->weapDef->parentWeaponName, -1, 0);
    if ( row == -1 )
    {
        Com_PrintWarning(
            14,
            "Couldn't find weapon parent '%s' for weapon '%s' in weaponOptions.csv\n",
            weapVarDef->weapDef->parentWeaponName,
            weapVarDef->szInternalName);
    }
    else
    {
        this->camoBaseTexture1 = WeaponOptions::GetImageFromTable(tablePtr, row, 3);
        this->camoBaseTexture2 = WeaponOptions::GetImageFromTable(tablePtr, row, 4);
        //this->camoOverrideIndex1 = WeaponOptions::FindCamoOverrideIndex(&weaponOptions, this->camoBaseTexture1, 0);
        this->camoOverrideIndex1 = weaponOptions.FindCamoOverrideIndex(this->camoBaseTexture1, 0);
        //this->camoOverrideIndex2 = WeaponOptions::FindCamoOverrideIndex(&weaponOptions, this->camoBaseTexture2, 1u);
        this->camoOverrideIndex2 = weaponOptions.FindCamoOverrideIndex(this->camoBaseTexture2, 1u);
        this->numToGold = 0;
        ColumnValueForRow = StringTable_GetColumnValueForRow(tablePtr, row, 5);
        //WeaponOptions::WeaponOverride::AddToGold(this, ColumnValueForRow, 1);
        this->AddToGold(ColumnValueForRow, true);
        v3 = StringTable_GetColumnValueForRow(tablePtr, row, 6);
        this->AddToGold(v3, 1);
        v4 = StringTable_GetColumnValueForRow(tablePtr, row, 7);
        this->AddToGold(v4, 1);
        v5 = StringTable_GetColumnValueForRow(tablePtr, row, 8);
        this->AddToGold(v5, 1);
        v6 = strstr(weapVarDef->szInternalName, "_acog");
        if ( v6 )
        {
            v7 = StringTable_GetColumnValueForRow(tablePtr, row, 13);
            this->AddToGold(v7, 1);
        }
        else
        {
            v8 = strstr(weapVarDef->szInternalName, "_elbit");
            if ( v8 )
            {
                v9 = StringTable_GetColumnValueForRow(tablePtr, row, 14);
                this->AddToGold(v9, 1);
            }
            else
            {
                v10 = strstr(weapVarDef->szInternalName, "_ir");
                if ( v10 )
                {
                    v11 = StringTable_GetColumnValueForRow(tablePtr, row, 15);
                    this->AddToGold(v11, 1);
                }
                else
                {
                    v12 = strstr(weapVarDef->szInternalName, "_reflex");
                    if ( v12 )
                    {
                        v13 = StringTable_GetColumnValueForRow(tablePtr, row, 16);
                        this->AddToGold(v13, 1);
                    }
                    else
                    {
                        v14 = strstr(weapVarDef->szInternalName, "_vzoom");
                        if ( v14 )
                        {
                            v15 = StringTable_GetColumnValueForRow(tablePtr, row, 17);
                            this->AddToGold(v15, 1);
                        }
                    }
                }
            }
        }
        this->numToBlack = 0;
        v16 = StringTable_GetColumnValueForRow(tablePtr, row, 9);
        this->AddToGold(v16, 0);
        v17 = StringTable_GetColumnValueForRow(tablePtr, row, 10);
        this->AddToGold(v17, 0);
        v18 = StringTable_GetColumnValueForRow(tablePtr, row, 11);
        this->AddToGold(v18, 0);
        v19 = StringTable_GetColumnValueForRow(tablePtr, row, 12);
        this->AddToGold(v19, 0);
        v20 = strstr(weapVarDef->szInternalName, "_acog");
        if ( v20 )
        {
            v21 = StringTable_GetColumnValueForRow(tablePtr, row, 18);
            this->AddToGold(v21, 0);
        }
        else
        {
            v22 = strstr(weapVarDef->szInternalName, "_elbit");
            if ( v22 )
            {
                v23 = StringTable_GetColumnValueForRow(tablePtr, row, 19);
                this->AddToGold(v23, 0);
            }
            else
            {
                v24 = strstr(weapVarDef->szInternalName, "_ir");
                if ( v24 )
                {
                    v25 = StringTable_GetColumnValueForRow(tablePtr, row, 20);
                    this->AddToGold(v25, 0);
                }
                else
                {
                    v26 = strstr(weapVarDef->szInternalName, "_reflex");
                    if ( v26 )
                    {
                        v27 = StringTable_GetColumnValueForRow(tablePtr, row, 21);
                        this->AddToGold(v27, 0);
                    }
                    else
                    {
                        v28 = strstr(weapVarDef->szInternalName, "_vzoom");
                        if ( v28 )
                        {
                            v29 = StringTable_GetColumnValueForRow(tablePtr, row, 22);
                            this->AddToGold(v29, 0);
                        }
                    }
                }
            }
        }
    }
}

bool __thiscall WeaponOptions::WeaponOverride::ToGold::Init(const char *matName)
{
    const char *v2; // eax
    XAssetHeader v4; // [esp+0h] [ebp-10h]
    int j; // [esp+Ch] [ebp-4h]

    if ( matName && *matName )
    {
        v2 = va("mc/%s", matName);
        v4.xmodelPieces = DB_FindXAssetHeader(ASSET_TYPE_MATERIAL, (char*)v2, 1, -1).xmodelPieces;
    }
    else
    {
        v4.xmodelPieces = 0;
    }
    this->material = v4.material;
    if ( this->material )
    {
        for ( j = 0; j < this->material->textureCount; ++j )
        {
            switch ( this->material->textureTable[j].semantic )
            {
                case 2u:
                    this->diffuseTexture = this->material->textureTable[j].u.image;
                    break;
                case 8u:
                    this->specularTexture = this->material->textureTable[j].u.image;
                    break;
                case 0x1Bu:
                    this->reflectionTexture = this->material->textureTable[j].u.image;
                    break;
                case 0x13u:
                    this->camoTexture = this->material->textureTable[j].u.image;
                    break;
            }
        }
    }
    return this->material != 0;
}

int __thiscall WeaponOptions::WeaponOverride::ToGold::Set(
                int textureOverrideIndex,
                unsigned __int16 modelMask,
                const WeaponOptions::WeaponOverride::ToGold *gold)
{
    int v4; // eax
    int v5; // eax
    int v6; // eax

    if ( !this->material
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_weapon_options.cpp", 364, 0, "%s", "material") )
    {
        __debugbreak();
    }
    v4 = R_AllocTextureOverride(
                 this->material,
                 modelMask,
                 this->diffuseTexture,
                 (GfxImage *)gold->diffuseTexture,
                 textureOverrideIndex);
    v5 = R_AllocTextureOverride(this->material, modelMask, this->specularTexture, (GfxImage *)gold->specularTexture, v4);
    v6 = R_AllocTextureOverride(
                 this->material,
                 modelMask,
                 this->reflectionTexture,
                 (GfxImage *)gold->reflectionTexture,
                 v5);
    return R_AllocTextureOverride(this->material, modelMask, this->camoTexture, (GfxImage *)gold->camoTexture, v6);
}

const WeaponOptions::WeaponOverride *__thiscall WeaponOptions::GetWeaponOverrides(const WeaponVariantDef *weapVarDef)
{
    int j; // [esp+4h] [ebp-10h]
    int minFrame; // [esp+8h] [ebp-Ch]
    int i; // [esp+Ch] [ebp-8h]
    WeaponOptions::WeaponOverride *ret; // [esp+10h] [ebp-4h]

    ret = 0;
    if ( weapVarDef )
    {
        for ( i = 0; i < this->numWeaponOverrides; ++i )
        {
            if ( this->weaponOverrides[i].weapon == weapVarDef )
            {
                this->weaponOverrides[i].lastFrame = com_frameNumber;
                return &this->weaponOverrides[i];
            }
        }
        if ( this->numWeaponOverrides == 100 )
        {
            minFrame = this->weaponOverrides[0].lastFrame;
            ret = this->weaponOverrides;
            for ( j = 0; j < this->numWeaponOverrides; ++j )
            {
                if ( this->weaponOverrides[j].lastFrame < minFrame )
                {
                    minFrame = this->weaponOverrides[j].lastFrame;
                    ret = &this->weaponOverrides[j];
                }
            }
        }
        else
        {
            ret = &this->weaponOverrides[this->numWeaponOverrides++];
        }
        //WeaponOptions::WeaponOverride::Init(ret, weapVarDef);
        ret->Init(weapVarDef);
    }
    return ret;
}

int __cdecl GetShaderConstant(ShaderConstantSet *constantSet, const char *name)
{
    signed int index; // [esp+0h] [ebp-4h]

    index = R_GetConstantIndex(constantSet, name);
    if ( index < 0 )
    {
        index = R_GetFreeConstantIndex(constantSet);
        if ( index >= 0 )
            R_MapShaderConstantSet(constantSet, index, name);
    }
    return index;
}

int __thiscall WeaponOptions::SetupWeaponOptionsRender(
                int localClientNum,
                const float *origin,
                const WeaponVariantDef *weapVarDef,
                renderOptions_s weaponOptions,
                unsigned __int16 modelMask,
                ShaderConstantSet *constantSet,
                const char *name,
                int textureOverrideIndex)
{
    int v9; // eax
    int v10; // edx
    unsigned int ShaderConstant; // eax
    unsigned int v12; // eax
    unsigned int v13; // eax
    unsigned int v14; // eax
    unsigned int v15; // eax
    unsigned int v16; // eax
    unsigned int v17; // eax
    unsigned int v18; // eax
    unsigned int v19; // eax
    unsigned int v20; // eax
    unsigned int v21; // eax
    float x; // [esp+0h] [ebp-B0h]
    float xa; // [esp+0h] [ebp-B0h]
    float xb; // [esp+0h] [ebp-B0h]
    float y; // [esp+4h] [ebp-ACh]
    float ya; // [esp+4h] [ebp-ACh]
    float yb; // [esp+4h] [ebp-ACh]
    float z; // [esp+8h] [ebp-A8h]
    float za; // [esp+8h] [ebp-A8h]
    float zb; // [esp+8h] [ebp-A8h]
    float z_4; // [esp+Ch] [ebp-A4h]
    float z_4a; // [esp+Ch] [ebp-A4h]
    char v34; // [esp+13h] [ebp-9Dh]
    float AdjustedLodDist; // [esp+14h] [ebp-9Ch]
    unsigned int fpPattern; // [esp+38h] [ebp-78h]
    int ii; // [esp+3Ch] [ebp-74h]
    const Glyph *glyphs[4]; // [esp+40h] [ebp-70h]
    const Glyph *glyphSize; // [esp+50h] [ebp-60h]
    GfxImage *emblem; // [esp+54h] [ebp-5Ch]
    int player; // [esp+58h] [ebp-58h]
    float shaderBase[4]; // [esp+5Ch] [ebp-54h] BYREF
    float color[4]; // [esp+6Ch] [ebp-44h] BYREF
    unsigned int n; // [esp+7Ch] [ebp-34h]
    unsigned int m; // [esp+80h] [ebp-30h]
    unsigned int lens; // [esp+84h] [ebp-2Ch]
    unsigned int reticle; // [esp+88h] [ebp-28h]
    unsigned int k; // [esp+8Ch] [ebp-24h]
    unsigned int j; // [esp+90h] [ebp-20h]
    unsigned int i; // [esp+94h] [ebp-1Ch]
    bool firstPerson; // [esp+9Bh] [ebp-15h]
    const cg_s *cgameGlob; // [esp+9Ch] [ebp-14h]
    float dist; // [esp+A0h] [ebp-10h]
    unsigned int camo; // [esp+A4h] [ebp-Ch]
    WeaponOptions::WeaponOverride *weaponOverride; // [esp+A8h] [ebp-8h]
    bool goldCamo; // [esp+AFh] [ebp-1h]

    if ( weaponOptions.i )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        firstPerson = origin == 0;
        if ( origin )
        {
            z = Vec3Distance(origin, cgameGlob->refdef.vieworg);
            AdjustedLodDist = R_GetAdjustedLodDist(z, XMODEL_LOD_RAMP_RIGID);
        }
        else
        {
            AdjustedLodDist = 0.0f;
        }
        dist = AdjustedLodDist;
        camo = weaponOptions.i & 0x3F;
        goldCamo = 0;
        weaponOverride = (WeaponOptions::WeaponOverride * )this->GetWeaponOverrides(weapVarDef);
        if ( (weaponOptions.i & 0x3F) != 0 && weaponCamoLodDist->current.value > dist )
        {
            if ( camo == this->goldCamoIndex )
            {
                if ( weaponOverride )
                {
                    goldCamo = 1;
                    for ( i = 0; i < weaponOverride->numToGold; ++i )
                        textureOverrideIndex = weaponOverride->toGold[i].Set( textureOverrideIndex, modelMask, &this->gold);

                    for ( j = 0; j < weaponOverride->numToBlack; ++j )
                        textureOverrideIndex = weaponOverride->toBlack[j].Set(textureOverrideIndex, modelMask, &this->black);
                }
            }
            else if ( weaponOverride )
            {
                v9 = R_AllocTextureOverride(
                             modelMask,
                             weaponOverride->camoBaseTexture2,
                             (GfxImage *)this->camoTextureOverrides[weaponOverride->camoOverrideIndex2][camo],
                             textureOverrideIndex);
                textureOverrideIndex = R_AllocTextureOverride(
                                                                 modelMask,
                                                                 weaponOverride->camoBaseTexture1,
                                                                 (GfxImage *)this->camoTextureOverrides[weaponOverride->camoOverrideIndex1][camo],
                                                                 v9);
                if ( weaponOverride->camoBaseTexture1 != this->camoTextureOverrides[0][0]
                    && weaponOverride->camoBaseTexture2 != this->camoTextureOverrides[0][0] )
                {
                    textureOverrideIndex = R_AllocTextureOverride(
                                                                     modelMask,
                                                                     this->camoTextureOverrides[0][0],
                                                                     (GfxImage *)this->camoTextureOverrides[0][camo],
                                                                     textureOverrideIndex);
                }
            }
            else
            {
                for ( k = 0; k < 0xA; ++k )
                    textureOverrideIndex = R_AllocTextureOverride(
                                                                     modelMask,
                                                                     this->camoTextureOverrides[k][0],
                                                                     (GfxImage *)this->camoTextureOverrides[k][camo],
                                                                     textureOverrideIndex);
            }
        }
        if ( firstPerson )
        {
            lens = (weaponOptions.i >> 6) & 0xF;
            reticle = LOWORD(weaponOptions.i) >> 10;
            if ( lens )
            {
                for ( m = 0; m < 0xA; ++m )
                    textureOverrideIndex = R_AllocTextureOverride(
                                                                     modelMask,
                                                                     this->lensTextureOverrides[m][0],
                                                                     (GfxImage *)this->lensTextureOverrides[m][lens],
                                                                     textureOverrideIndex);
            }
            if ( reticle )
            {
                for ( n = 0; n < 0xA; ++n )
                    textureOverrideIndex = R_AllocTextureOverride(
                                                                     modelMask,
                                                                     this->reticleTextureOverrides[n][0],
                                                                     (GfxImage *)this->reticleTextureOverrides[n][reticle],
                                                                     textureOverrideIndex);
            }
            if ( constantSet )
            {
                if ((weaponOptions.i & 0x70000) != 0)
                {
                    shaderBase[0] = 1.0f;
                    memset(&shaderBase[1], 0, 12);
                    v10 = HIWORD(weaponOptions) & 7;
                    color[0] = this->reticleColors[v10][0] - 1.0;
                    color[1] = this->reticleColors[v10][1] - 0.0;
                    color[2] = this->reticleColors[v10][2] - 0.0;
                    color[3] = this->reticleColors[v10][3] - 0.0;
                    ShaderConstant = GetShaderConstant(constantSet, "scriptVector3");
                    R_SetShaderConstantSetValue(constantSet, ShaderConstant, color);
                }
                else
                {
                    v12 = GetShaderConstant(constantSet, "scriptVector3");
                    R_SetShaderConstantSetValue(constantSet, v12, 0.0, 0.0, 0.0, 0.0);
                }
            }
        }
        if ( ((weaponOptions.i >> 21) & 0x1F) != 0
            && ((weaponOptions.i >> 19) & 1) != 0
            && weaponEmblemLodDist->current.value > dist )
        {
            player = ((weaponOptions.i >> 21) & 0x1F) - 1;
            emblem = PCache_GetPlayerEmblem(-1, cgameGlob->bgs.clientinfo[player].xuid);
            if ( emblem )
                textureOverrideIndex = R_AllocTextureOverride(modelMask, this->emblemClearTexture, emblem, textureOverrideIndex);
        }
        if ( constantSet && weaponClanTagLodDist->current.value > dist )
        {
            if ( !this->clanFont )
                this->clanFont = DB_FindXAssetHeader(ASSET_TYPE_FONT, (char*)"fonts/clanFont", 0, -1).font;
            if ( ((weaponOptions.i >> 21) & 0x1F) != 0 && ((weaponOptions.i >> 20) & 1) != 0 )
            {
                if ( !name )
                    name = (const char *)&cgameGlob->teamScores[370 * ((weaponOptions.i >> 21) & 0x1F)];
            }
            else
            {
                name = 0;
            }
            if ( this->clanFont && name )
            {
                glyphSize = R_GetCharacterGlyph(this->clanFont, 0x21u);
                for ( ii = 0; ii < 4; ++ii )
                {
                    if ( name[ii] )
                        v34 = name[ii];
                    else
                        v34 = 32;
                    glyphs[ii] = R_GetCharacterGlyph(this->clanFont, v34);
                }
                y = glyphSize->t1 - glyphSize->t0;
                x = glyphSize->s1 - glyphSize->s0;
                v13 = GetShaderConstant(constantSet, "scriptVector4");
                R_SetShaderConstantSetValue(constantSet, v13, x, y, 0.0, 0.0);
                z_4 = glyphs[1]->t0;
                za = glyphs[1]->s0;
                ya = glyphs[0]->t0;
                xa = glyphs[0]->s0;
                v14 = GetShaderConstant(constantSet, "scriptVector5");
                R_SetShaderConstantSetValue(constantSet, v14, xa, ya, za, z_4);
                z_4a = glyphs[3]->t0;
                zb = glyphs[3]->s0;
                yb = glyphs[2]->t0;
                xb = glyphs[2]->s0;
                v15 = GetShaderConstant(constantSet, "scriptVector6");
                R_SetShaderConstantSetValue(constantSet, v15, xb, yb, zb, z_4a);
                if ( goldCamo )
                    textureOverrideIndex = R_AllocTextureOverride(
                                                                     modelMask,
                                                                     this->lightClanTagFont,
                                                                     (GfxImage *)this->darkClanTagFont,
                                                                     textureOverrideIndex);
            }
            else
            {
                v16 = GetShaderConstant(constantSet, "scriptVector4");
                R_SetShaderConstantSetValue(constantSet, v16, 0.0, 0.0, 0.0, 0.0);
                v17 = GetShaderConstant(constantSet, "scriptVector5");
                R_SetShaderConstantSetValue(constantSet, v17, 0.0, 0.0, 0.0, 0.0);
                v18 = GetShaderConstant(constantSet, "scriptVector6");
                R_SetShaderConstantSetValue(constantSet, v18, 0.0, 0.0, 0.0, 0.0);
            }
        }
        if ( constantSet )
        {
            if ( !firstPerson && facepaintLodDist->current.value > dist )
            {
                fpPattern = weaponOptions.i >> 26;
                if ( weaponOptions.i >> 26 )
                {
                    if ( this->facepaintMaterials[fpPattern] )
                        return R_AllocTextureOverride(
                                         modelMask,
                                         this->facepaintMaterials[0]->textureTable->u.image,
                                         this->facepaintMaterials[fpPattern]->textureTable->u.image,
                                         textureOverrideIndex);
                }
            }
        }
    }
    else if ( constantSet )
    {
        v19 = GetShaderConstant(constantSet, "scriptVector4");
        R_SetShaderConstantSetValue(constantSet, v19, 0.0, 0.0, 0.0, 0.0);
        v20 = GetShaderConstant(constantSet, "scriptVector5");
        R_SetShaderConstantSetValue(constantSet, v20, 0.0, 0.0, 0.0, 0.0);
        v21 = GetShaderConstant(constantSet, "scriptVector6");
        R_SetShaderConstantSetValue(constantSet, v21, 0.0, 0.0, 0.0, 0.0);
    }
    return textureOverrideIndex;
}

//bool __thiscall WeaponOptions::IsValidRenderOption(renderOptions_s renderOptions)
//{
//    if ( !this->camoTextureOverrides[0][renderOptions.i & 0x3F] && (renderOptions.i & 0x3F) != this->goldCamoIndex )
//        return 0;
//    if ( !this->lensTextureOverrides[0][(renderOptions.i >> 6) & 0xF] )
//        return 0;
//    if ( !this->reticleTextureOverrides[0][LOWORD(renderOptions.i) >> 10] )
//        return 0;
//    if ( this->reticleColors[*((_BYTE *)&renderOptions.s + 2) & 7][0] >= 0.0 )
//        return this->facepaintMaterials[renderOptions.i >> 26] != 0;
//    return 0;
//}

bool __thiscall WeaponOptions::IsValidRenderOption(renderOptions_s renderOptions)
{
    if (!this->camoTextureOverrides[0][renderOptions.camo] && renderOptions.camo != this->goldCamoIndex)
    {
        return false;
    }

    if (!this->lensTextureOverrides[0][renderOptions.lens])
    {
        return false;
    }

    if (!this->reticleTextureOverrides[0][renderOptions.reticle])
    {
        return false;
    }

    if (this->reticleColors[renderOptions.reticleColor][0] >= 0.0f)
    {
        return this->facepaintMaterials[renderOptions.facepaintPattern] != nullptr;
    }

    return false;
}

char __thiscall WeaponOptions::GetWeaponOptionReticleColor(unsigned int reticleColorIndex, float *outColor)
{
    if ( reticleColorIndex >= 0x10
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_weapon_options.cpp",
                    616,
                    0,
                    "%s",
                    "( reticleColorIndex >= 0 ) && ( reticleColorIndex < ARRAY_COUNT( reticleColors ) )") )
    {
        __debugbreak();
    }
    if ( reticleColorIndex >= 0x10 )
        return 0;
    if ( this->reticleColors[reticleColorIndex][0] < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_weapon_options.cpp",
                    620,
                    0,
                    "%s",
                    "reticleColors[reticleColorIndex][0] >= 0.0f") )
    {
        __debugbreak();
    }
    if ( this->reticleColors[reticleColorIndex][0] < 0.0 )
        return 0;
    *outColor = this->reticleColors[reticleColorIndex][0];
    outColor[1] = this->reticleColors[reticleColorIndex][1];
    outColor[2] = this->reticleColors[reticleColorIndex][2];
    outColor[3] = this->reticleColors[reticleColorIndex][3];
    return 1;
}

char __thiscall WeaponOptions::GetWeaponOptionLensColor(
                unsigned int reticleColorIndex,
                float *outColor)
{
    if ( reticleColorIndex >= 0x10
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_weapon_options.cpp",
                    630,
                    0,
                    "%s",
                    "( reticleColorIndex >= 0 ) && ( reticleColorIndex < ARRAY_COUNT( lensColors ) )") )
    {
        __debugbreak();
    }
    if ( reticleColorIndex >= 0x10 )
        return 0;
    if ( this->lensColors[reticleColorIndex][0] < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_weapon_options.cpp",
                    634,
                    0,
                    "%s",
                    "lensColors[reticleColorIndex][0] >= 0.0f") )
    {
        __debugbreak();
    }
    if ( this->lensColors[reticleColorIndex][0] < 0.0 )
        return 0;
    *outColor = this->lensColors[reticleColorIndex][0];
    outColor[1] = this->lensColors[reticleColorIndex][1];
    outColor[2] = this->lensColors[reticleColorIndex][2];
    outColor[3] = this->lensColors[reticleColorIndex][3];
    return 1;
}

void __cdecl GC_InitWeaponOptions()
{
    //WeaponOptions::InitWeaponOptions(&weaponOptions);
    weaponOptions.InitWeaponOptions();
}

int __cdecl CG_SetupWeaponOptionsRender(
                int localClientNum,
                const float *origin,
                const WeaponVariantDef *weaponVarDef,
                renderOptions_s renderOptions,
                ShaderConstantSet *constantSet,
                const char *name)
{
    return weaponOptions.SetupWeaponOptionsRender(
                     localClientNum,
                     origin,
                     weaponVarDef,
                     renderOptions,
                     0xFFFFu,
                     constantSet,
                     name,
                     -1);
}

int __cdecl CG_SetupWeaponOptionsRender(
                int localClientNum,
                const float *origin,
                const WeaponVariantDef *wvd,
                renderOptions_s wo,
                unsigned __int16 modelMask,
                ShaderConstantSet *constantSet,
                const char *name,
                int textureOverrideIndex)
{
    return weaponOptions.SetupWeaponOptionsRender(
                     localClientNum,
                     origin,
                     wvd,
                     wo,
                     modelMask,
                     constantSet,
                     name,
                     textureOverrideIndex);
}

bool __cdecl CG_IsValidRenderOption(renderOptions_s renderOptions)
{
    //return WeaponOptions::IsValidRenderOption(&weaponOptions, renderOptions);
    return weaponOptions.IsValidRenderOption(renderOptions);
}

char __cdecl CG_GetWeaponOptionReticleColor(unsigned int reticleColorIndex, float *outColor)
{
    //return WeaponOptions::GetWeaponOptionReticleColor(&weaponOptions, reticleColorIndex, outColor);
    return weaponOptions.GetWeaponOptionReticleColor(reticleColorIndex, outColor);
}

char __cdecl CG_GetWeaponOptionLensColor(unsigned int reticleColorIndex, float *outColor)
{
    //return WeaponOptions::GetWeaponOptionLensColor(&weaponOptions, reticleColorIndex, outColor);
    return weaponOptions.GetWeaponOptionLensColor(reticleColorIndex, outColor);
}

