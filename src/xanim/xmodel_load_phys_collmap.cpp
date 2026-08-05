#include "xmodel_load_phys_collmap.h"
#include <universal/com_math.h>
#include <qcommon/common.h>
#include <gfx_d3d/r_material_load_obj.h>
#include <universal/com_files.h>
#include <universal/q_parse.h>
#include "dobj.h"
#include "xanim.h"
#include "xmodel.h"
#include <clientscript/cscr_stringlist.h>
#include <common/brush_edges.h>
#include <universal/com_math_anglevectors.h>

collision_material_t g_collision_materials[512];
int g_collision_materials_count;

bool __cdecl PlaneEqual(const float *p1, const float *p2)
{
    return VecNCompareCustomEpsilon(p1, p2, 0.001, 3) && fabs(p1[3] - p2[3]) < 0.001;
}

char __cdecl RemoveDuplicateBrushPlanes(
                float (*planes)[4],
                unsigned int *sideCount,
                const char *mapname,
                unsigned int brushnum)
{
    float *v5; // [esp+Ch] [ebp-2Ch]
    float *v6; // [esp+10h] [ebp-28h]
    float *v7; // [esp+14h] [ebp-24h]
    float *v8; // [esp+18h] [ebp-20h]
    unsigned int sideIndexK; // [esp+20h] [ebp-18h]
    unsigned int sideIndexKa; // [esp+20h] [ebp-18h]
    float negatedNormal[3]; // [esp+24h] [ebp-14h] BYREF
    unsigned int sideIndexJ; // [esp+30h] [ebp-8h]
    unsigned int sideIndexI; // [esp+34h] [ebp-4h]

    for ( sideIndexI = 1; sideIndexI < *sideCount; ++sideIndexI )
    {
        if ( Vec3IsNormalized(&(*planes)[4 * sideIndexI]) )
        {
            for ( sideIndexJ = 0; sideIndexJ < sideIndexI; ++sideIndexJ )
            {
                if ( PlaneEqual(&(*planes)[4 * sideIndexI], &(*planes)[4 * sideIndexJ]) )
                {
                    for ( sideIndexKa = sideIndexI + 1; sideIndexKa < *sideCount; ++sideIndexKa )
                    {
                        v5 = &(*planes)[4 * sideIndexKa - 4];
                        v6 = &(*planes)[4 * sideIndexKa];
                        *v5 = *v6;
                        v5[1] = v6[1];
                        v5[2] = v6[2];
                        v5[3] = v6[3];
                    }
                    --*sideCount;
                    --sideIndexI;
                    break;
                }
                (negatedNormal[0]) = -((*planes)[4 * sideIndexJ]);
                (negatedNormal[1]) = -((*planes)[4 * sideIndexJ + 1]);
                (negatedNormal[2]) = -((*planes)[4 * sideIndexJ + 2]);
                if ( VecNCompareCustomEpsilon(&(*planes)[4 * sideIndexI], negatedNormal, 0.001, 3) && (-((*planes)[4 * sideIndexJ + 3])) >= (*planes)[4 * sideIndexI + 3] )
                {
                    Com_PrintWarning(19, "Map %s, Brush %i: mirrored plane\n", mapname, brushnum);
                    return 0;
                }
            }
        }
        else
        {
            Com_PrintWarning(19, "Collmap for %s, Brush %i: degenerate plane\n", mapname, brushnum);
            for ( sideIndexK = sideIndexI + 1; sideIndexK < *sideCount; ++sideIndexK )
            {
                v7 = &(*planes)[4 * sideIndexK - 4];
                v8 = &(*planes)[4 * sideIndexK];
                *v7 = *v8;
                v7[1] = v8[1];
                v7[2] = v8[2];
                v7[3] = v8[3];
            }
            --*sideCount;
            --sideIndexI;
        }
    }
    return 1;
}

void __cdecl SnapPlane(float *plane)
{
    float v1; // [esp+4h] [ebp-4h]

    SnapAxialVector(plane);
    v1 = plane[3] - Q_rint(plane[3]);
    if ( fabs(v1) < 0.001 )
        plane[3] = Q_rint(plane[3]);
}

char __cdecl SnapAxialVector(float *normal)
{
    int axis; // [esp+8h] [ebp-4h]

    for ( axis = 0; axis < 3; ++axis )
    {
        if ( fabs(normal[axis] - 1.0) < 0.0000001 )
        {
            *normal = 0.0f;
            normal[1] = 0.0f;
            normal[2] = 0.0f;
            normal[axis] = 1.0f;
            return 1;
        }
        if ( fabs(normal[axis] - -1.0) < 0.0000001 )
        {
            *normal = 0.0f;
            normal[1] = 0.0f;
            normal[2] = 0.0f;
            normal[axis] = -1.0f;
            return 1;
        }
    }
    return 0;
}

void __cdecl GetCollisionMaterial(char *name, collision_material_t *cmat)
{
    char filename[1028]; // [esp+0h] [ebp-410h] BYREF
    const MaterialRaw *mtlRaw; // [esp+408h] [ebp-8h] BYREF
    int fileSize; // [esp+40Ch] [ebp-4h]

    sprintf(filename, "materials/%s", name);
    fileSize = FS_ReadFile(filename, (void **)&mtlRaw);
    if ( fileSize > 0 )
    {
        strncpy(cmat->name, name, 0x40u);
        cmat->cflags = mtlRaw->info.contents;
        cmat->sflags = mtlRaw->info.surfaceFlags;
        FS_FreeFile((void *)mtlRaw);
    }
}

int __cdecl IsAxisAligned(float *normal)
{
    float v[3]; // [esp+8h] [ebp-10h] BYREF
    int axis; // [esp+14h] [ebp-4h]

    for ( axis = 0; axis < 3; ++axis )
    {
        memset(v, 0, sizeof(v));
        v[axis] = 1.0f;
        if ( VecNCompareCustomEpsilon(normal, v, 0.001, 3) )
            return 1;
        v[axis] = -1.0f;
        if ( VecNCompareCustomEpsilon(normal, v, 0.001, 3) )
            return 1;
    }
    return 0;
}

char __cdecl SkipEpair(char *token, const char **file)
{
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    parseInfo_t *tokena; // [esp+28h] [ebp+8h]

    if ( token[strlen(token) - 2] == 92 )
    {
        Com_PrintError(19, "SkipEpair: key '%s' ends with a '\\'\n", token);
        return 0;
    }
    else
    {
        v3 = strchr(token, 0xAu);
        if ( v3 || (v4 = strchr(token, 0xDu), v4) )
        {
            Com_PrintError(19, "SkipEpair: key '%s' contains a newline character\n", token);
            return 0;
        }
        else
        {
            v5 = strchr(token, 0x22u);
            if ( v5 )
            {
                Com_PrintError(19, "SkipEpair: key '%s' contains a \" character, will cause parsing errors\n", token);
                return 0;
            }
            else
            {
                tokena = Com_ParseOnLine(file);
                if ( tokena->token[strlen(tokena->token) - 2] == 92 )
                {
                    Com_PrintError(19, "SkipEpair: value '%s' ends with a '\\'\n", tokena->token);
                    return 0;
                }
                else
                {
                    v6 = strchr(tokena->token, 0xAu);
                    if ( v6 || (v7 = strchr(tokena->token, 0xDu), v7) )
                    {
                        Com_PrintError(
                            19,
                            "SkipEpair: value '%s' contains a newline character (use of '\\' at end of value?)\n",
                            tokena->token);
                        return 0;
                    }
                    else
                    {
                        v8 = strchr(tokena->token, 0x22u);
                        if ( v8 )
                        {
                            Com_PrintError(
                                19,
                                "SkipEpair: value '%s' contains a \" character, will cause parsing errors\n",
                                tokena->token);
                            return 0;
                        }
                        else
                        {
                            return 1;
                        }
                    }
                }
            }
        }
    }
}

void __cdecl XModel_LoadCollMap(const char *name, void *(__cdecl *Alloc)(int), XModel *model, const char *xmodelName)
{
    unsigned int LowercaseString; // eax
    unsigned int v5; // eax
    char *v6; // eax
    unsigned int v7; // eax
    PhysGeomList *v8; // eax
    DObjAnimMat *mat; // [esp+0h] [ebp-6B4h]
    char filename[1028]; // [esp+4h] [ebp-6B0h] BYREF
    unsigned int boneIndex; // [esp+40Ch] [ebp-2A8h]
    const char *buf; // [esp+410h] [ebp-2A4h] BYREF
    unsigned int boneHash; // [esp+414h] [ebp-2A0h]
    unsigned __int8 *file; // [esp+418h] [ebp-29Ch] BYREF
    int fileSize; // [esp+41Ch] [ebp-298h]
    unsigned int geomCount; // [esp+420h] [ebp-294h]
    unsigned int boneHashes[161]; // [esp+424h] [ebp-290h] BYREF
    unsigned int collmapCount; // [esp+6A8h] [ebp-Ch]
    Collmap *collmap; // [esp+6ACh] [ebp-8h]
    unsigned int collIndex; // [esp+6B0h] [ebp-4h]

    if ( Com_sprintf(filename, 0x400u, "collmaps/%s", name) >= 0 )
    {
        fileSize = FS_ReadFile(filename, (void **)&file);
        if ( fileSize >= 0 )
        {
            if ( fileSize )
            {
                memset(boneHashes, 0xFFu, 0x280u);
                buf = (const char *)file;
                collmapCount = Xmodel_CountPhysicsCollMaps(&buf, filename, boneHashes, 160);
                if ( (!collmapCount || collmapCount > 0xFF)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_phys_collmap.cpp",
                                1124,
                                0,
                                "collmapCount not in [1, static_cast< byte >( ~0 )]\n\t%i not in [%i, %i]",
                                collmapCount,
                                1,
                                255) )
                {
                    __debugbreak();
                }
                model->numCollmaps = collmapCount;
                model->collmaps = (Collmap *)Alloc(4 * collmapCount);
                memset(model->collmaps, 0, 4 * collmapCount);
                for ( boneIndex = 0; boneIndex < model->numBones; ++boneIndex )
                    model->localBoneInfo[boneIndex].collmap = -1;
                for ( collIndex = 0; collIndex < collmapCount; ++collIndex )
                {
                    boneHash = boneHashes[collIndex];
                    if ( boneHash == -1
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_phys_collmap.cpp",
                                    1138,
                                    0,
                                    "%s",
                                    "boneHash != BONE_NAME_HASH_INVALID") )
                    {
                        __debugbreak();
                    }
                    LowercaseString = SL_FindLowercaseString("", SCRIPTINSTANCE_SERVER);
                    if ( boneHash == LowercaseString )
                        collmap = model->collmaps;
                    else
                        collmap = &model->collmaps[collIndex];
                    for ( boneIndex = 0; ; ++boneIndex )
                    {
                        v5 = SL_FindLowercaseString("", SCRIPTINSTANCE_SERVER);
                        if ( boneHash == v5 || boneIndex >= model->numBones )
                            break;
                        if ( boneHash == model->localBoneNames[boneIndex] )
                        {
                            model->localBoneInfo[boneIndex].collmap = collIndex;
                            break;
                        }
                    }
                    if ( boneIndex == model->numBones )
                    {
                        v6 = SL_ConvertToString(boneHash, SCRIPTINSTANCE_SERVER);
                        Com_PrintError(
                            19,
                            "ERROR: collmap %s has bone %s that doesn't exist in model %s\n",
                            filename,
                            v6,
                            xmodelName);
                    }
                    buf = (const char *)file;
                    geomCount = Xmodel_CountPhysicsCollMapGeoms(&buf, filename, boneHash);
                    if ( geomCount )
                    {
                        mat = 0;
                        v7 = SL_FindLowercaseString("", SCRIPTINSTANCE_SERVER);
                        if ( boneHash != v7 && boneIndex < model->numBones )
                        {
                            if ( boneIndex > model->numBones
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_phys_collmap.cpp",
                                            1168,
                                            0,
                                            "boneIndex not in [0, model->numBones]\n\t%i not in [%i, %i]",
                                            boneIndex,
                                            0,
                                            model->numBones) )
                            {
                                __debugbreak();
                            }
                            mat = &model->baseMat[boneIndex];
                        }
                        buf = (const char *)file;
                        v8 = Xmodel_ParsePhysicsCollMap(&buf, filename, geomCount, boneHash, mat, Alloc);
                        collmap->geomList = v8;
                    }
                }
                FS_FreeFile(file);
            }
            else
            {
                Com_PrintError(19, "ERROR: physics collmap '%s' has 0 length\n", name);
                FS_FreeFile(file);
            }
        }
        else if ( file )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_phys_collmap.cpp",
                            1107,
                            0,
                            "%s",
                            "!file") )
                __debugbreak();
        }
    }
    else
    {
        Com_PrintError(19, "ERROR: filename '%s' too long\n", filename);
    }
}

unsigned int __cdecl Xmodel_CountPhysicsCollMaps(
                const char **file,
                const char *name,
                unsigned int *boneHashes,
                int boneNameLen)
{
    int CurrentParseLine; // eax
    unsigned int collMapCount; // [esp+64h] [ebp-8h]
    parseInfo_t *token; // [esp+68h] [ebp-4h]
    parseInfo_t *tokena; // [esp+68h] [ebp-4h]
    parseInfo_t *tokenb; // [esp+68h] [ebp-4h]
    parseInfo_t *tokenc; // [esp+68h] [ebp-4h]

    if ( !strcmp(Com_Parse(file)->token, "iwmap") )
    {
        token = Com_Parse(file);
        if ( atoi(token->token) == 4 )
        {
            while ( 1 )
            {
                tokena = Com_Parse(file);
                if ( !tokena->token[0] )
                    return 0;
                if ( !strcmp(tokena->token, "{") )
                    break;
                Com_SkipRestOfLine(file);
            }
            if ( !strcmp(tokena->token, "{") )
            {
                AddBoneName(boneHashes, boneNameLen, "");
                collMapCount = 1;
                while ( 1 )
                {
                    tokenb = Com_Parse(file);
                    if ( !tokenb->token[0] )
                        break;
                    if ( !strcmp(tokenb->token, "}") )
                    {
                        if ( AddBoneName(boneHashes, boneNameLen, "") )
                            ++collMapCount;
                    }
                    else if ( !strcmp(tokenb->token, "collmap_bone") )
                    {
                        tokenc = Com_Parse(file);
                        if ( AddBoneName(boneHashes, boneNameLen, tokenc->token) )
                            ++collMapCount;
                        Com_SkipBracedSection(file, 1u, 0x7FFFFFFF);
                    }
                    else
                    {
                        Com_SkipRestOfLine(file);
                    }
                }
                return collMapCount;
            }
            else
            {
                CurrentParseLine = Com_GetCurrentParseLine();
                Com_PrintError(
                    19,
                    "ERROR: ParsePhysicsCollMap: { not found, found %s on line %d",
                    tokena->token,
                    CurrentParseLine);
                return 0;
            }
        }
        else
        {
            Com_PrintError(19, "ERROR: '%s' is version '%s'; should be version '%i'\n", name, token->token, 4);
            return 0;
        }
    }
    else
    {
        Com_PrintError(19, "ERROR: '%s' is missing 'iwmap' version specification\n", name);
        return 0;
    }
}

char __cdecl AddBoneName(unsigned int *boneNameArray, int boneNameLen, const char *boneName)
{
    unsigned int hash; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    hash = SL_GetLowercaseString(boneName, 0, SCRIPTINSTANCE_SERVER);
    for ( i = 0; boneNameArray[i] != -1 && i < boneNameLen; ++i )
    {
        if ( boneNameArray[i] == hash )
            return 0;
    }
    if ( i >= boneNameLen )
        return 0;
    boneNameArray[i] = hash;
    return 1;
}

unsigned int __cdecl Xmodel_CountPhysicsCollMapGeoms(const char **file, const char *name, unsigned int boneHash)
{
    int CurrentParseLine; // eax
    unsigned int brushCount; // [esp+DCh] [ebp-8h]
    parseInfo_t *token; // [esp+E0h] [ebp-4h]
    parseInfo_t *tokena; // [esp+E0h] [ebp-4h]
    parseInfo_t *tokenb; // [esp+E0h] [ebp-4h]
    parseInfo_t *tokenc; // [esp+E0h] [ebp-4h]
    parseInfo_t *tokene; // [esp+E0h] [ebp-4h]
    parseInfo_t *tokend; // [esp+E0h] [ebp-4h]

    if ( !strcmp(Com_Parse(file)->token, "iwmap") )
    {
        token = Com_Parse(file);
        if ( atoi(token->token) == 4 )
        {
            while ( 1 )
            {
                tokena = Com_Parse(file);
                if ( !tokena->token[0] )
                    return 0;
                if ( !strcmp(tokena->token, "{") )
                    break;
                Com_SkipRestOfLine(file);
            }
            if ( !strcmp(tokena->token, "{") )
            {
                brushCount = 0;
                while ( 1 )
                {
LABEL_12:
                    tokenb = Com_Parse(file);
                    if ( !tokenb->token[0] )
                    {
                        Com_PrintError(19, "ERROR: ParsePhysicsCollMap: EOF without closing brace");
                        return 0;
                    }
                    if ( !strcmp(tokenb->token, "}") )
                        return brushCount;
                    if ( !strcmp(tokenb->token, "{") )
                    {
                        while ( 1 )
                        {
                            tokenc = Com_Parse(file);
                            if ( !tokenc->token[0] )
                                break;
                            if ( !strcmp(tokenc->token, "}") )
                            {
                                if ( boneHash == SL_FindLowercaseString("", SCRIPTINSTANCE_SERVER) )
                                    ++brushCount;
                                goto LABEL_12;
                            }
                            if ( !strcmp(tokenc->token, "collmap_bone") )
                            {
                                tokene = Com_Parse(file);
                                if ( boneHash == SL_FindLowercaseString(tokene->token, SCRIPTINSTANCE_SERVER) )
                                    ++brushCount;
                                Com_SkipBracedSection(file, 1u, 0x7FFFFFFF);
                                goto LABEL_12;
                            }
                            if ( !strcmp(tokenc->token, "curve") )
                            {
                                Com_PrintError(19, "ERROR: cannot have curves in collision maps");
                                return 0;
                            }
                            if ( !strcmp(tokenc->token, "mesh") )
                            {
                                Com_PrintError(19, "ERROR: cannot have patch terrain in collision maps");
                                return 0;
                            }
                            if ( !strcmp(tokenc->token, "{") )
                            {
                                do
                                    tokend = Com_Parse(file);
                                while ( tokend->token[0] && strcmp(tokend->token, "}") );
                            }
                        }
                    }
                }
            }
            else
            {
                CurrentParseLine = Com_GetCurrentParseLine();
                Com_PrintError(
                    19,
                    "ERROR: ParsePhysicsCollMap: { not found, found %s on line %d",
                    tokena->token,
                    CurrentParseLine);
                return 0;
            }
        }
        else
        {
            Com_PrintError(19, "ERROR: '%s' is version '%s'; should be version '%i'\n", name, token->token, 4);
            return 0;
        }
    }
    else
    {
        Com_PrintError(19, "ERROR: '%s' is missing 'iwmap' version specification\n", name);
        return 0;
    }
}

PhysGeomList *__cdecl Xmodel_ParsePhysicsCollMap(
                const char **file,
                char *name,
                unsigned int geomCount,
                unsigned int boneHash,
                DObjAnimMat *boneMat,
                void *(__cdecl *Alloc)(int))
{
    int CurrentParseLine; // eax
    PhysGeomList *v8; // eax
    float v9; // [esp+0h] [ebp-120h]
    float v10; // [esp+4h] [ebp-11Ch]
    float v11; // [esp+8h] [ebp-118h]
    float v12; // [esp+Ch] [ebp-114h]
    float v13; // [esp+10h] [ebp-110h]
    float v14; // [esp+14h] [ebp-10Ch]
    unsigned int i; // [esp+E0h] [ebp-40h]
    float mins; // [esp+E4h] [ebp-3Ch]
    float mins_4; // [esp+E8h] [ebp-38h]
    float mins_8; // [esp+ECh] [ebp-34h]
    float geomMaxs[3]; // [esp+F0h] [ebp-30h] BYREF
    float maxs[3]; // [esp+FCh] [ebp-24h]
    PhysGeomList *geomList; // [esp+108h] [ebp-18h]
    float geomMins[3]; // [esp+10Ch] [ebp-14h] BYREF
    const char *token; // [esp+118h] [ebp-8h]
    unsigned int geomIndex; // [esp+11Ch] [ebp-4h]

    if ( !geomCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_phys_collmap.cpp",
                    929,
                    0,
                    "%s",
                    "geomCount > 0") )
    {
        __debugbreak();
    }
    if ( !strcmp(Com_Parse(file)->token, "iwmap") )
    {
        token = (const char *)Com_Parse(file);
        if ( atoi(token) == 4 )
        {
            while ( 1 )
            {
                token = (const char *)Com_Parse(file);
                if ( !*token )
                    return 0;
                if ( !strcmp(token, "{") )
                    break;
                Com_SkipRestOfLine(file);
            }
            if ( !strcmp(token, "{") )
            {
                v8 = (PhysGeomList *)Alloc(12);
                geomList = v8;
                v8->count = 0;
                v8->geoms = 0;
                v8->contents = 0;
                geomList->count = geomCount;
                geomList->geoms = (PhysGeomInfo *)Alloc(68 * geomCount);
                memset(geomList->geoms, 0, 68 * geomCount);
                geomIndex = 0;
                while ( geomIndex < geomCount )
                {
                    token = (const char *)Com_Parse(file);
                    if ( !*token )
                    {
                        Com_PrintError(19, "ERROR: ParsePhysicsCollMap: EOF without closing brace");
                        return 0;
                    }
                    if ( !strcmp(token, "}") )
                        break;
                    if ( !strcmp(token, "{") )
                    {
                        token = (const char *)Com_Parse(file);
                        if ( !*token )
                            break;
                        if ( !strcmp(token, "guid") )
                        {
                            Com_ParseRestOfLine(file);
                            token = (const char *)Com_Parse(file);
                        }
                        if ( !strcmp(token, "curve") )
                        {
                            Com_PrintError(19, "ERROR: cannot have curves in collision maps");
                            return 0;
                        }
                        if ( !strcmp(token, "mesh") )
                        {
                            Com_PrintError(19, "ERROR: cannot have patch terrain in collision maps");
                            return 0;
                        }
                        if ( !strcmp(token, "physics_cylinder") )
                        {
                            if ( !Xmodel_ParsePhysicsCylinder(file, &geomList->geoms[geomIndex]) )
                                return 0;
                            if ( !Com_MatchToken(file, "}", 0) )
                                return 0;
                            ++geomIndex;
                        }
                        else if ( !strcmp(token, "physics_box") )
                        {
                            if ( !Xmodel_ParsePhysicsBox(file, &geomList->geoms[geomIndex]) )
                                return 0;
                            if ( !Com_MatchToken(file, "}", 0) )
                                return 0;
                            ++geomIndex;
                        }
                        else
                        {
                            Com_UngetToken();
                            if ( Xmodel_ParsePhysicsBrush(
                                         (char **)file,
                                         name,
                                         geomIndex,
                                         &geomList->geoms[geomIndex],
                                         boneHash,
                                         boneMat,
                                         Alloc) )
                            {
                                if ( !geomList->geoms[geomIndex].brush
                                    && !Assert_MyHandler(
                                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_phys_collmap.cpp",
                                                1027,
                                                1,
                                                "%s",
                                                "geomList->geoms[geomIndex].brush") )
                                {
                                    __debugbreak();
                                }
                                ++geomIndex;
                            }
                            else
                            {
                                Com_SkipBracedSection(file, 1u, 0x7FFFFFFF);
                            }
                        }
                    }
                    else if ( !SkipEpair((char *)token, file) )
                    {
                        return 0;
                    }
                }
                if ( geomIndex != geomCount
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_phys_collmap.cpp",
                                1045,
                                0,
                                "geomIndex == geomCount\n\t%i, %i",
                                geomIndex,
                                geomCount) )
                {
                    __debugbreak();
                }
                mins = FLT_MAX;
                mins_4 = FLT_MAX;
                mins_8 = FLT_MAX;
                maxs[0] = -FLT_MAX;
                maxs[1] = -FLT_MAX;
                maxs[2] = -FLT_MAX;
                for ( geomIndex = 0; geomIndex < geomCount; ++geomIndex )
                {
                    GetGeomAABB(&geomList->geoms[geomIndex], geomMins, geomMaxs);
                    if ( (float)(maxs[0] - geomMaxs[0]) < 0.0 )
                        v14 = geomMaxs[0];
                    else
                        v14 = maxs[0];
                    maxs[0] = v14;
                    if ( (float)(maxs[1] - geomMaxs[1]) < 0.0 )
                        v13 = geomMaxs[1];
                    else
                        v13 = maxs[1];
                    maxs[1] = v13;
                    if ( (float)(maxs[2] - geomMaxs[2]) < 0.0 )
                        v12 = geomMaxs[2];
                    else
                        v12 = maxs[2];
                    maxs[2] = v12;
                    if ( (float)(geomMins[0] - mins) < 0.0 )
                        v11 = geomMins[0];
                    else
                        v11 = mins;
                    mins = v11;
                    if ( (float)(geomMins[1] - mins_4) < 0.0 )
                        v10 = geomMins[1];
                    else
                        v10 = mins_4;
                    mins_4 = v10;
                    if ( (float)(geomMins[2] - mins_8) < 0.0 )
                        v9 = geomMins[2];
                    else
                        v9 = mins_8;
                    mins_8 = v9;
                }
                if ( maxs[0] <= mins
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_phys_collmap.cpp",
                                1059,
                                0,
                                "%s",
                                "mins[0] < maxs[0]") )
                {
                    __debugbreak();
                }
                if ( maxs[1] <= mins_4
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_phys_collmap.cpp",
                                1060,
                                0,
                                "%s",
                                "mins[1] < maxs[1]") )
                {
                    __debugbreak();
                }
                if ( maxs[2] <= mins_8
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_phys_collmap.cpp",
                                1061,
                                0,
                                "%s",
                                "mins[2] < maxs[2]") )
                {
                    __debugbreak();
                }
                geomList->contents = 0;
                for ( i = 0; i < geomList->count; ++i )
                {
                    if ( !geomList->geoms
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_phys_collmap.cpp",
                                    1067,
                                    0,
                                    "%s",
                                    "geomList->geoms") )
                    {
                        __debugbreak();
                    }
                    if ( geomList->geoms[i].brush )
                    {
                        geomList->contents |= geomList->geoms[i].brush->contents;
                    }
                    else if ( geomList->geoms[i].type == 3 || geomList->geoms[i].type == 1 )
                    {
                        geomList->contents |= 0x400u;
                    }
                }
                return geomList;
            }
            else
            {
                CurrentParseLine = Com_GetCurrentParseLine();
                Com_PrintError(19, "ERROR: ParsePhysicsCollMap: { not found, found %s on line %d", token, CurrentParseLine);
                return 0;
            }
        }
        else
        {
            Com_PrintError(19, "ERROR: '%s' is version '%s'; should be version '%i'\n", name, token, 4);
            return 0;
        }
    }
    else
    {
        Com_PrintError(19, "ERROR: '%s' is missing 'iwmap' version specification\n", name);
        return 0;
    }
}

char __cdecl Xmodel_ParsePhysicsBrush(
    char **file,
    char *mapname,
    unsigned int brushCount,
    PhysGeomInfo *geom,
    unsigned int boneHash,
    DObjAnimMat *boneMat,
    void *(__cdecl *Alloc)(int))
{
    float *v8; // [esp+3Ch] [ebp-6ABCh]
    SimplePlaneIntersection *v9; // [esp+40h] [ebp-6AB8h]
    cplane_s *v10; // [esp+44h] [ebp-6AB4h]
    float *v11; // [esp+48h] [ebp-6AB0h]
    float *maxs; // [esp+4Ch] [ebp-6AACh]
    BrushWrapper *brush; // [esp+50h] [ebp-6AA8h]
    unsigned int jj; // [esp+F0h] [ebp-6A08h]
    int v15; // [esp+F4h] [ebp-6A04h]
    int ii; // [esp+F8h] [ebp-6A00h]
    collision_material_t *cmat; // [esp+FCh] [ebp-69FCh]
    collision_material_t *v18; // [esp+100h] [ebp-69F8h]
    int i; // [esp+104h] [ebp-69F4h]
    parseInfo_t *name; // [esp+108h] [ebp-69F0h]
    float m[3]; // [esp+10Ch] [ebp-69ECh] BYREF
    float in[3]; // [esp+118h] [ebp-69E0h] BYREF
    float out[3]; // [esp+124h] [ebp-69D4h] BYREF
    int v24; // [esp+130h] [ebp-69C8h]
    int v25; // [esp+134h] [ebp-69C4h]
    unsigned int sideCount; // [esp+138h] [ebp-69C0h] BYREF
    unsigned int n; // [esp+13Ch] [ebp-69BCh]
    _DWORD dst[33]; // [esp+140h] [ebp-69B8h] BYREF
    float v29; // [esp+1C4h] [ebp-6934h]
    float v30; // [esp+1C8h] [ebp-6930h]
    float v31; // [esp+1CCh] [ebp-692Ch]
    float plane[128]; // [esp+1D0h] [ebp-6928h] BYREF
    SimplePlaneIntersection OutPts[1024]; // [esp+3D0h] [ebp-6728h] BYREF
    int InPtCount; // [esp+63D0h] [ebp-728h]
    unsigned int j; // [esp+63D4h] [ebp-724h]
    _DWORD v36[32]; // [esp+63D8h] [ebp-720h] BYREF
    int basePlaneIndex; // [esp+6458h] [ebp-6A0h]
    float v38; // [esp+645Ch] [ebp-69Ch]
    float v39; // [esp+6460h] [ebp-698h]
    float v40; // [esp+6464h] [ebp-694h]
    adjacencyWinding_t optionalOutWinding[32]; // [esp+6468h] [ebp-690h] BYREF
    int v42; // [esp+6AE8h] [ebp-10h]
    char *str; // [esp+6AECh] [ebp-Ch]
    int v44; // [esp+6AF0h] [ebp-8h]
    unsigned int k; // [esp+6AF4h] [ebp-4h]

    v42 = 32;
    if (!geom
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_phys_collmap.cpp", 365, 0, "%s", "geom"))
    {
        __debugbreak();
    }
    sideCount = 0;
    Map_SkipOptionalArg((const char **)file, "layer");
    if (!Map_SkipNamedFlags((const char **)file, "contents"))
        return 0;
    if (!Map_SkipNamedFlags((const char **)file, "toolFlags"))
        return 0;
    str = (char *)Com_Parse((const char **)file);
    if (!strcmp(str, "collmap_bone"))
    {
        str = (char *)Com_Parse((const char **)file);
        if (boneHash != SL_FindLowercaseString(str, SCRIPTINSTANCE_SERVER))
            return 0;
        str = (char *)Com_Parse((const char **)file);
    }
    else if (boneHash != SL_FindLowercaseString("", SCRIPTINSTANCE_SERVER))
    {
        return 0;
    }
    Com_UngetToken();
    v24 = 0;
    memset(dst, 0, 0x80u);
    memset(v36, 0, sizeof(v36));
    while (1)
    {
        str = (char *)Com_Parse((const char **)file);
        if (!*str)
            return 0;
        if (!strcmp(str, "}"))
            break;
        Com_UngetToken();
        if (sideCount == 32)
        {
            Com_PrintError(19, "ERROR: MAX_BUILD_SIDES (%i) -- brush too many sides.  Simplify the collision geometry.\n", 32);
            return 0;
        }
        Com_Parse1DMatrix((const char **)file, 3, m);
        Com_Parse1DMatrix((const char **)file, 3, in);
        Com_Parse1DMatrix((const char **)file, 3, out);
        if (boneMat)
        {
            InvMatrixTransformVectorQuatTrans(m, boneMat, m);
            InvMatrixTransformVectorQuatTrans(in, boneMat, in);
            InvMatrixTransformVectorQuatTrans(out, boneMat, out);
        }
        PlaneFromPoints(&plane[4 * sideCount], m, in, out);
        SnapPlane(&plane[4 * sideCount]);
        name = Com_Parse((const char **)file);
        for (i = 0; i < g_collision_materials_count; ++i)
        {
            v18 = &g_collision_materials[i];
            if (!strcmp(name->token, v18->name))
            {
                dst[sideCount] = v18->cflags;
                v36[sideCount] = v18->sflags;
                break;
            }
        }
        if (i == g_collision_materials_count)
        {
            if (g_collision_materials_count >= 512)
            {
                Com_PrintError(19, "Max number of collision materials exceeded %d\n", 512);
            }
            else
            {
                cmat = &g_collision_materials[g_collision_materials_count++];
                GetCollisionMaterial(name->token, cmat);
                dst[sideCount] = cmat->cflags;
                v36[sideCount] = cmat->sflags;
            }
        }
        v24 |= dst[sideCount++];
        Com_SkipRestOfLine((const char **)file);
    }
    if (!RemoveDuplicateBrushPlanes((float (*)[4])plane, &sideCount, mapname, brushCount))
        return 0;
    if (!sideCount)
        return 0;
    InPtCount = GetPlaneIntersections((const float (*)[4])plane, sideCount, OutPts, 0x400u);
    v44 = 0;
    v29 = OutPts[0].xyz[0];
    v30 = OutPts[0].xyz[1];
    v31 = OutPts[0].xyz[2];
    v38 = OutPts[0].xyz[0];
    v39 = OutPts[0].xyz[1];
    v40 = OutPts[0].xyz[2];
    for (j = 1; j < InPtCount; ++j)
    {
        for (k = 0; k < 3; ++k)
        {
            if (*(&v29 + k) <= OutPts[j].xyz[k])
            {
                if (OutPts[j].xyz[k] > *(&v38 + k))
                    *(&v38 + k) = OutPts[j].xyz[k];
            }
            else
            {
                *(&v29 + k) = OutPts[j].xyz[k];
            }
        }
    }
    InPtCount = GetPlaneIntersections((const float (*)[4])plane, sideCount, OutPts, 0x400u);
    v44 = 0;
    for (basePlaneIndex = 0; basePlaneIndex < sideCount; ++basePlaneIndex)
    {
        if (BuildBrushdAdjacencyWindingForSide(
            mapname,
            &plane[4 * basePlaneIndex],
            basePlaneIndex,
            OutPts,
            InPtCount,
            &optionalOutWinding[basePlaneIndex],
            12))
        {
            if (optionalOutWinding[basePlaneIndex].numsides <= 0
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_phys_collmap.cpp",
                    498,
                    0,
                    "%s",
                    "windings[sideIndex].numsides > 0"))
            {
                __debugbreak();
            }
            v44 += optionalOutWinding[basePlaneIndex].numsides;
        }
        else
        {
            optionalOutWinding[basePlaneIndex].numsides = 0;
        }
    }
    if (v44)
    {
        geom->brush = (BrushWrapper *)Alloc(96);
        memset(geom->brush, 0, sizeof(BrushWrapper));
        geom->type = 2;
        brush = geom->brush;
        brush->mins[0] = v29;
        brush->mins[1] = v30;
        brush->mins[2] = v31;
        maxs = geom->brush->maxs;
        *maxs = v38;
        maxs[1] = v39;
        maxs[2] = v40;
        v25 = 0;
        for (k = 0; k < 3; ++k)
        {
            for (n = 0; n < 2; geom->brush->axial_sflags[n++][k] = v36[v25++])
                geom->brush->axial_cflags[n][k] = dst[v25];
        }
        geom->brush->numsides = sideCount;
        for (ii = 0; ii < 6; ++ii)
        {
            if (IsAxisAligned(&plane[4 * ii]))
                --geom->brush->numsides;
        }
        if (geom->brush->numsides)
        {
            geom->brush->sides = (cbrushside_t *)Alloc(12 * geom->brush->numsides);
            geom->brush->planes = (cplane_s *)Alloc(20 * geom->brush->numsides);
            v15 = 0;
            for (basePlaneIndex = 0; basePlaneIndex < sideCount; ++basePlaneIndex)
            {
                if ((unsigned int)basePlaneIndex >= 6 || !IsAxisAligned(&plane[4 * basePlaneIndex]))
                {
                    v10 = &geom->brush->planes[v15];
                    v11 = &plane[4 * basePlaneIndex];
                    v10->normal[0] = *v11;
                    v10->normal[1] = v11[1];
                    v10->normal[2] = v11[2];
                    geom->brush->planes[v15].dist = plane[4 * basePlaneIndex + 3];
                    geom->brush->sides[v15].plane = &geom->brush->planes[v15];
                    geom->brush->sides[v15].cflags = dst[basePlaneIndex];
                    geom->brush->sides[v15++].sflags = v36[basePlaneIndex];
                }
            }
        }
        geom->brush->numverts = InPtCount;
        geom->brush->verts = (float (*)[3])Alloc(12 * geom->brush->numverts);
        for (jj = 0; jj < geom->brush->numverts; ++jj)
        {
            v8 = geom->brush->verts[jj];
            v9 = &OutPts[jj];
            *v8 = v9->xyz[0];
            v8[1] = v9->xyz[1];
            v8[2] = v9->xyz[2];
        }
        geom->brush->contents = v24;
        return 1;
    }
    else
    {
        Com_UngetToken();
        return 0;
    }
}

unsigned int __cdecl GetPlaneIntersections(
                const float (*planes)[4],
                unsigned int planeCount,
                SimplePlaneIntersection *OutPts,
                unsigned int maxPtCount)
{
    const float *plane[3]; // [esp+8h] [ebp-34h] BYREF
    unsigned int ptCount; // [esp+14h] [ebp-28h]
    float xyz[3]; // [esp+18h] [ebp-24h] BYREF
    unsigned int sideIndex[3]; // [esp+24h] [ebp-18h]
    unsigned int planeIndex[3]; // [esp+30h] [ebp-Ch] BYREF

    ptCount = 0;
    for ( sideIndex[0] = 0; sideIndex[0] < planeCount - 2; ++sideIndex[0] )
    {
        planeIndex[0] = sideIndex[0];
        plane[0] = &(*planes)[4 * sideIndex[0]];
        for ( sideIndex[1] = sideIndex[0] + 1; sideIndex[1] < planeCount - 1; ++sideIndex[1] )
        {
            planeIndex[1] = sideIndex[1];
            plane[1] = &(*planes)[4 * sideIndex[1]];
            for ( sideIndex[2] = sideIndex[1] + 1; sideIndex[2] < planeCount; ++sideIndex[2] )
            {
                planeIndex[2] = sideIndex[2];
                plane[2] = &(*planes)[4 * sideIndex[2]];
                if ( IntersectPlanes(plane, xyz) )
                {
                    SnapPointToIntersectingPlanes(plane, xyz, 0.25, 0.0099999998);
                    ptCount = AddSimplePoint(planes, planeCount, planeIndex, xyz, OutPts, ptCount, maxPtCount);
                }
            }
        }
    }
    return ptCount;
}

int __cdecl AddSimplePoint(
                const float (*planes)[4],
                unsigned int planeCount,
                const unsigned int *planeIndex,
                const float *xyz,
                SimplePlaneIntersection *pts,
                int ptCount,
                int maxPtCount)
{
    SimplePlaneIntersection *v8; // [esp+0h] [ebp-10h]
    unsigned int sideIndex; // [esp+Ch] [ebp-4h]

    for ( sideIndex = 0; sideIndex < planeCount; ++sideIndex )
    {
        if ( sideIndex != *planeIndex
            && sideIndex != planeIndex[1]
            && sideIndex != planeIndex[2]
            && (float)((float)((float)((float)((float)(*planes)[4 * sideIndex] * *xyz)
                                                             + (float)((float)(*planes)[4 * sideIndex + 1] * xyz[1]))
                                             + (float)((float)(*planes)[4 * sideIndex + 2] * xyz[2]))
                             - (float)(*planes)[4 * sideIndex + 3]) > 0.0099999998 )
        {
            return ptCount;
        }
    }
    if ( ptCount < maxPtCount )
    {
        v8 = &pts[ptCount];
        v8->xyz[0] = *xyz;
        v8->xyz[1] = xyz[1];
        v8->xyz[2] = xyz[2];
        v8->planeIndex[0] = *planeIndex;
        v8->planeIndex[1] = planeIndex[1];
        v8->planeIndex[2] = planeIndex[2];
        return ptCount + 1;
    }
    else
    {
        Com_PrintError(
            19,
            "More than %i points from plane intersections on %i-sided brush.    Simplify the collision geometry.\n",
            ptCount,
            planeCount);
        return 0;
    }
}

void __cdecl Map_SkipOptionalArg(const char **file, const char *argName)
{
    if ( !strcmp(Com_Parse(file)->token, argName) )
        Com_ParseOnLine(file);
    else
        Com_UngetToken();
}

char __cdecl Map_SkipNamedFlags(const char **file, const char *key)
{
    parseInfo_t *token; // [esp+28h] [ebp-4h]

    if ( !strcmp(Com_Parse(file)->token, key) )
    {
        do
        {
            token = Com_ParseOnLine(file);
            if ( !token->token[0] )
            {
                Com_PrintError(19, "missing token for '%s' on collmap\n", key);
                return 0;
            }
        }
        while ( token->token[0] != 59 && token->token[strlen(token->token) - 1] != 59 );
        return 1;
    }
    else
    {
        Com_UngetToken();
        return 1;
    }
}

bool __cdecl Xmodel_ParsePhysicsBox(const char **file, PhysGeomInfo *geom)
{
    if ( !geom
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_phys_collmap.cpp", 572, 0, "%s", "geom") )
    {
        __debugbreak();
    }
    if ( Com_Parse(file)->token[0] == 123 )
    {
        geom->type = 1;
        geom->orientation[0][0] = Com_ParseFloat(file);
        geom->orientation[0][1] = Com_ParseFloatOnLine(file);
        geom->orientation[0][2] = Com_ParseFloatOnLine(file);
        geom->orientation[1][0] = Com_ParseFloat(file);
        geom->orientation[1][1] = Com_ParseFloatOnLine(file);
        geom->orientation[1][2] = Com_ParseFloatOnLine(file);
        geom->orientation[2][0] = Com_ParseFloat(file);
        geom->orientation[2][1] = Com_ParseFloatOnLine(file);
        geom->orientation[2][2] = Com_ParseFloatOnLine(file);
        geom->offset[0] = Com_ParseFloatOnLine(file);
        geom->offset[1] = Com_ParseFloatOnLine(file);
        geom->offset[2] = Com_ParseFloatOnLine(file);
        geom->halfLengths[0] = Com_ParseFloatOnLine(file);
        geom->halfLengths[1] = Com_ParseFloatOnLine(file);
        geom->halfLengths[2] = Com_ParseFloatOnLine(file);
        return Com_MatchToken(file, "}", 0) != 0;
    }
    else
    {
        Com_PrintError(19, "Expecting '{' while parsing physics box");
        return 0;
    }
}

char __cdecl Xmodel_ParsePhysicsCylinder(const char **file, PhysGeomInfo *geom)
{
    float angles[3]; // [esp+0h] [ebp-1Ch] BYREF
    const char *token; // [esp+Ch] [ebp-10h]
    float axis[3]; // [esp+10h] [ebp-Ch] BYREF

    if ( !geom
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_phys_collmap.cpp", 611, 0, "%s", "geom") )
    {
        __debugbreak();
    }
    token = (const char *)Com_Parse(file);
    if ( *token == 123 )
    {
        geom->type = 3;
        axis[0] = Com_ParseFloat(file);
        axis[1] = Com_ParseFloatOnLine(file);
        axis[2] = Com_ParseFloatOnLine(file);
        geom->offset[0] = Com_ParseFloatOnLine(file);
        geom->offset[1] = Com_ParseFloatOnLine(file);
        geom->offset[2] = Com_ParseFloatOnLine(file);
        geom->halfLengths[0] = Com_ParseFloatOnLine(file) * 0.5;
        geom->halfLengths[1] = Com_ParseFloatOnLine(file);
        if ( Com_MatchToken(file, "}", 0) )
        {
            vectoangles(axis, angles);
            AnglesToAxis(angles, geom->orientation);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_PrintError(19, "Expecting '{' while parsing physics cylinder");
        return 0;
    }
}

void __cdecl GetGeomAABB(const PhysGeomInfo *geom, float *mins, float *maxs)
{
    int type; // [esp+0h] [ebp-34h]
    float *v4; // [esp+18h] [ebp-1Ch]
    BrushWrapper *brush; // [esp+1Ch] [ebp-18h]
    float range; // [esp+20h] [ebp-14h]
    float range_4; // [esp+24h] [ebp-10h]
    float range_8; // [esp+28h] [ebp-Ch]
    unsigned int axisIndex; // [esp+2Ch] [ebp-8h]
    float axisRange; // [esp+30h] [ebp-4h]

    if ( geom->brush )
    {
        brush = geom->brush;
        *mins = geom->brush->mins[0];
        mins[1] = brush->mins[1];
        mins[2] = brush->mins[2];
        v4 = geom->brush->maxs;
        *maxs = *v4;
        maxs[1] = v4[1];
        maxs[2] = v4[2];
    }
    else
    {
        range = 0.0f;
        range_4 = 0.0f;
        range_8 = 0.0f;
        type = geom->type;
        if ( type == 1 )
        {
            range = geom->halfLengths[0];
            range_4 = geom->halfLengths[1];
            range_8 = geom->halfLengths[2];
        }
        else if ( type == 3 )
        {
            range = geom->halfLengths[0];
            range_4 = geom->halfLengths[1];
            range_8 = range_4;
        }
        else if ( !Assert_MyHandler(
                                 "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_phys_collmap.cpp",
                                 707,
                                 0,
                                 "unhandled geom type") )
        {
            __debugbreak();
        }
        for ( axisIndex = 0; axisIndex < 3; ++axisIndex )
        {
            axisRange = (float)(fabs(geom->orientation[0][axisIndex] * range)
                                                + fabs(geom->orientation[1][axisIndex] * range_4))
                                + fabs(geom->orientation[2][axisIndex] * range_8);
            mins[axisIndex] = geom->offset[axisIndex] - axisRange;
            maxs[axisIndex] = geom->offset[axisIndex] + axisRange;
        }
    }
}

