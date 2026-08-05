#include "xmodel_load_obj.h"
#include "xmodel.h"

#include <universal/assertive.h>
#include <physics/physpreset_load_obj.h>
#include <physics/physconstraints_load_obj.h>
#include <cfloat>
#include "xmodel_utils.h"
#include <universal/com_memory.h>
#include <gfx_d3d/r_xsurface.h>
#include <qcommon/common.h>
#include <universal/com_files.h>
#include "xmodel_load_phys_collmap.h"
#include <clientscript/cscr_stringlist.h>
#include <game_mp/g_spawnsystem_mp.h>
#include <cgame_mp/cg_pose_mp.h>
#include <gfx_d3d/r_stream_util.h>
#include <gfx_d3d/r_model.h>

PhysPreset *__cdecl XModel_PhysPresetPrecache(const char *name, void *(__cdecl *Alloc)(int))
{
    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp", 42, 0, "%s", "name") )
        __debugbreak();
    return PhysPresetPrecache(name, Alloc);
}

PhysConstraints *__cdecl XModel_PhysConstraintsPrecache(const char *name, void *(__cdecl *Alloc)(int))
{
    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp", 49, 0, "%s", "name") )
        __debugbreak();
    return PhysConstraintsPrecache(name, Alloc);
}

__int64 __cdecl FS_GetModTime()
{
    return 0;
}

void __cdecl XModelCopyXModelParts(const XModelPartsLoad *modelParts, XModel *model)
{
    model->numBones = modelParts->numBones;
    model->numRootBones = modelParts->numRootBones;
    model->localBoneNames = modelParts->boneNames;
    model->localParentList = modelParts->parentList;
    model->localQuats = modelParts->quats;
    model->localTrans = modelParts->trans;
    model->partClassification = modelParts->partClassification;
    model->baseMat = modelParts->baseMat;
}

void __cdecl R_GetXModelBounds(XModel *model, const float (*axes)[3], float *mins, float *maxs)
{
    float coord; // [esp+4h] [ebp-24h]
    int surfaceCount; // [esp+8h] [ebp-20h]
    int axisIndex; // [esp+Ch] [ebp-1Ch]
    XSurface *surfaces; // [esp+10h] [ebp-18h] BYREF
    int index; // [esp+14h] [ebp-14h]
    XSurface *xsurf; // [esp+18h] [ebp-10h]
    int vertCount; // [esp+1Ch] [ebp-Ch]
    int vertIndex; // [esp+20h] [ebp-8h]
    float (*vert)[3]; // [esp+24h] [ebp-4h]

    *mins = FLT_MAX;
    mins[1] = FLT_MAX;
    mins[2] = FLT_MAX;

    *maxs = -FLT_MAX;
    maxs[1] = -FLT_MAX;
    maxs[2] = -FLT_MAX;

    surfaceCount = XModelGetSurfaces(model, &surfaces, 0);
    if ( !surfaces
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp", 1230, 0, "%s", "surfaces") )
    {
        __debugbreak();
    }
    vert = (float (*)[3])Hunk_AllocateTempMemory(393216, "R_GetXModelBounds");
    if ( surfaceCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                    1234,
                    0,
                    "%s",
                    "surfaceCount > 0") )
    {
        __debugbreak();
    }
    for ( index = 0; index < surfaceCount; ++index )
    {
        xsurf = &surfaces[index];
        vertCount = XSurfaceGetNumVerts(xsurf);
        XSurfaceGetVerts(xsurf, (float *)vert, 0, 0);
        if ( vertCount <= 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                        1241,
                        0,
                        "%s",
                        "vertCount > 0") )
        {
            __debugbreak();
        }
        for ( vertIndex = 0; vertIndex < vertCount; ++vertIndex )
        {
            for ( axisIndex = 0; axisIndex < 3; ++axisIndex )
            {
                coord = (float)((float)(vert[vertIndex][0] * (float)(*axes)[axisIndex])
                                            + (float)(vert[vertIndex][1] * (float)(*axes)[axisIndex + 3]))
                            + (float)(vert[vertIndex][2] * (float)(*axes)[axisIndex + 6]);
                if ( mins[axisIndex] > coord )
                    mins[axisIndex] = coord;
                if ( coord > maxs[axisIndex] )
                    maxs[axisIndex] = coord;
            }
        }
    }
    Hunk_FreeTempMemory((char *)vert);
}

XModel *__cdecl XModelLoad(char *name, void *(__cdecl *Alloc)(int), void *(__cdecl *AllocColl)(int))
{
    __int64 xmodelModifiedTime; // [esp+0h] [ebp-10h] BYREF
    XModel *model; // [esp+Ch] [ebp-4h]

    model = XModelLoadFile(name, Alloc, AllocColl, &xmodelModifiedTime);
    if ( !model )
        return 0;
    XModelGenerateHighMipVolume(model);
    return model;
}

XModel *__cdecl XModelLoadFile(
                char *name,
                void *(__cdecl *Alloc)(int),
                void *(__cdecl *AllocColl)(int),
                __int64 *xmodelModifiedTime)
{
    int *partBits; // eax
    unsigned __int8 v6; // al
    Material *v7; // eax
    PhysPreset *v8; // eax
    PhysConstraints *v9; // eax
    float v10; // [esp+8h] [ebp-20B0h]
    float v11; // [esp+Ch] [ebp-20ACh]
    float v12; // [esp+48h] [ebp-2070h]
    int v13; // [esp+6Ch] [ebp-204Ch]
    int v14; // [esp+70h] [ebp-2048h]
    int v15; // [esp+74h] [ebp-2044h]
    int v16; // [esp+78h] [ebp-2040h]
    int v17; // [esp+7Ch] [ebp-203Ch]
    int v18; // [esp+80h] [ebp-2038h]
    unsigned __int16 v19; // [esp+84h] [ebp-2034h]
    char v20[512]; // [esp+88h] [ebp-2030h] BYREF
    float v21; // [esp+288h] [ebp-1E30h]
    float v22; // [esp+28Ch] [ebp-1E2Ch]
    float v23; // [esp+290h] [ebp-1E28h]
    float v24; // [esp+294h] [ebp-1E24h]
    float v25; // [esp+298h] [ebp-1E20h]
    unsigned __int8 *pos; // [esp+29Ch] [ebp-1E1Ch] BYREF
    int j; // [esp+2A0h] [ebp-1E18h]
    int numBones; // [esp+2A4h] [ebp-1E14h]
    char dest[68]; // [esp+2A8h] [ebp-1E10h] BYREF
    float v30; // [esp+2ECh] [ebp-1DCCh]
    float v31; // [esp+2F0h] [ebp-1DC8h]
    float v32; // [esp+2F4h] [ebp-1DC4h]
    float axes[9]; // [esp+2F8h] [ebp-1DC0h] BYREF
    int v34; // [esp+31Ch] [ebp-1D9Ch]
    XModel *model; // [esp+320h] [ebp-1D98h]
    float *v36; // [esp+324h] [ebp-1D94h]
    void *buffer; // [esp+328h] [ebp-1D90h] BYREF
    XModelLodInfo *v38; // [esp+32Ch] [ebp-1D8Ch]
    float *offset; // [esp+330h] [ebp-1D88h]
    int v40; // [esp+334h] [ebp-1D84h]
    const char *v41; // [esp+338h] [ebp-1D80h]
    int v42; // [esp+33Ch] [ebp-1D7Ch]
    char v43[260]; // [esp+340h] [ebp-1D78h] BYREF
    float *v44; // [esp+444h] [ebp-1C74h]
    XModelSurfs outModelSurfs; // [esp+448h] [ebp-1C70h] BYREF
    XBoneInfo *v46; // [esp+460h] [ebp-1C58h]
    int i; // [esp+464h] [ebp-1C54h]
    unsigned __int8 *v48; // [esp+468h] [ebp-1C50h]
    int v49; // [esp+46Ch] [ebp-1C4Ch]
    XModelConfig config; // [esp+470h] [ebp-1C48h] BYREF
    XModelPartsLoad *modelParts; // [esp+20B0h] [ebp-8h]
    const char *v52; // [esp+20B4h] [ebp-4h]

    if ( Com_IsLegacyXModelName(name) )
    {
        Com_PrintError(19, "ERROR: Remove xmodel prefix from model name '%s'\n", name);
        return 0;
    }
    else if ( Com_sprintf(dest, 0x40u, "xmodel/%s", name) >= 0 )
    {
        v42 = FS_ReadFile(dest, &buffer);
        *xmodelModifiedTime = FS_GetModTime();
        if ( v42 >= 0 )
        {
            if ( v42 )
            {
                pos = (unsigned __int8 *)buffer;
                if ( !XModelLoadConfigFile(name, (float **)&pos, &config) )
                    goto LABEL_28;
                v40 = 252;
                model = (XModel *)Alloc(252);
                model->memUsage = v40;
                model->flags = config.flags;
                XModelLoadCollData((const unsigned __int8 **)&pos, model, AllocColl, name);

                model->numLods = 0;
                v48 = pos;
                v49 = 0;
                for ( i = 0; i < 4; ++i )
                {
                    v38 = &model->lodInfo[i];
                    if ( config.entries[i].filename[0] )
                    {
                        if ( i != model->numLods
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                                        561,
                                        0,
                                        "%s",
                                        "i == model->numLods") )
                        {
                            __debugbreak();
                        }
                        ++model->numLods;
                        v19 = *(_WORD *)pos;
                        pos += 2;
                        v38->numsurfs = v19;
                        v49 += v38->numsurfs;
                        for ( j = 0; j < v38->numsurfs; ++j )
                        {
                            v52 = (const char *)pos;
                            pos += strlen((const char *)pos) + 1;
                        }
                    }
                    v38->dist = config.mins[257 * i - 772];
                }
                if ( !model->numLods
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                                576,
                                0,
                                "%s",
                                "model->numLods") )
                {
                    __debugbreak();
                }
                model->flags &= ~0x80000u;

                modelParts = XModelPartsPrecache(model, (const char *)&config, Alloc, xmodelModifiedTime);
                if ( modelParts )
                {
                    XModelCopyXModelParts(modelParts, model);
                    numBones = model->numBones;
                    v40 = 44 * numBones;
                    v46 = (XBoneInfo *)Alloc(44 * numBones);
                    model->memUsage += v40;
                    for ( i = 0; i < numBones; ++i )
                    {
                        v36 = (float *)&v46[i];
                        v18 = *(unsigned int *)pos;
                        pos += 4;
                        *(unsigned int *)v36 = v18;
                        v17 = *(unsigned int *)pos;
                        pos += 4;
                        *((unsigned int *)v36 + 1) = v17;
                        v16 = *(unsigned int *)pos;
                        pos += 4;
                        *((unsigned int *)v36 + 2) = v16;
                        v44 = v46[i].bounds[1];
                        v15 = *(unsigned int *)pos;
                        pos += 4;
                        *(unsigned int *)v44 = v15;
                        v14 = *(unsigned int *)pos;
                        pos += 4;
                        *((unsigned int *)v44 + 1) = v14;
                        v13 = *(unsigned int *)pos;
                        pos += 4;
                        *((unsigned int *)v44 + 2) = v13;
                        offset = v46[i].offset;
                        *offset = (float)(*v36 + *v44) * 0.5;
                        offset[1] = (float)(v36[1] + v44[1]) * 0.5;
                        offset[2] = (float)(v36[2] + v44[2]) * 0.5;
                        v30 = *v44 - *offset;
                        v31 = v44[1] - offset[1];
                        v32 = v44[2] - offset[2];
                        v46[i].radiusSquared = (float)((float)(v30 * v30) + (float)(v31 * v31)) + (float)(v32 * v32);
                    }
                    model->localBoneInfo = v46;
                    model->lodRampType = 0;
                    if ( (unsigned __int8)XModelAllowLoadMesh() )
                    {
                        pos = v48;
                        if ( !config.entries[0].filename[0]
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                                        623,
                                        0,
                                        "%s",
                                        "config.entries[0].filename[0]") )
                        {
                            __debugbreak();
                        }
                        model->numsurfs = v49;
                        if ( model->numsurfs != v49
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                                        626,
                                        0,
                                        "%s",
                                        "model->numsurfs == numsurfs") )
                        {
                            __debugbreak();
                        }
                        model->surfs = (XSurface *)Alloc(68 * v49);
                        model->materialHandles = (Material **)Alloc(4 * v49);
                        v34 = 0;
                        for ( i = 0; i < 4; ++i )
                        {
                            v38 = &model->lodInfo[i];
                            if ( config.entries[i].filename[0] )
                            {
                                pos += 2;
                                if ( !XModelSurfsPrecache(
                                                model,
                                                config.entries[i].filename,
                                                Alloc,
                                                v38->numsurfs,
                                                name,
                                                &outModelSurfs,
                                                xmodelModifiedTime) )
                                    goto LABEL_28;
                                partBits = v38->partBits;
                                v38->partBits[0] = outModelSurfs.partBits[0];
                                partBits[1] = outModelSurfs.partBits[1];
                                partBits[2] = outModelSurfs.partBits[2];
                                partBits[3] = outModelSurfs.partBits[3];
                                partBits[4] = outModelSurfs.partBits[4];
                                v38->surfIndex = v34;
                                if ( v38->surfIndex != v34
                                    && !Assert_MyHandler(
                                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                                                655,
                                                0,
                                                "%s",
                                                "modelLodInfo->surfIndex == surfIndex") )
                                {
                                    __debugbreak();
                                }
                                v6 = truncate_cast<unsigned char>(i);
                                v38->lod = v6;
                                v38->smcIndexPlusOne = 0;
                                for ( j = 0; j < v38->numsurfs; ++j )
                                {
                                    v52 = (const char *)pos;
                                    pos += strlen((const char *)pos) + 1;
                                    if ( !strcmp(v52, "$default") )
                                        v52 = "$default3d";
                                    v41 = "mc/";
                                    Com_sprintf(v43, 0x100u, "%s%s", "mc/", v52);
                                    v7 = Material_RegisterHandle(v43, 8);
                                    model->materialHandles[v34] = v7;
                                    if ( (outModelSurfs.surfs[j].flags & 0x80) != 0 )
                                        model->lodRampType = 1;
                                    memcpy(&model->surfs[v34++], &outModelSurfs.surfs[j], sizeof(model->surfs[v34++]));
                                }
                            }
                        }
                        if ( v34 != v49
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                                        730,
                                        0,
                                        "%s",
                                        "surfIndex == numsurfs") )
                        {
                            __debugbreak();
                        }
                        axes[0] = 1.0f;
                        memset(&axes[1], 0, 12);
                        axes[4] = 1.0f;
                        memset(&axes[5], 0, 12);
                        axes[8] = 1.0f;
                        R_GetXModelBounds(model, (const float (*)[3])axes, model->mins, model->maxs);
                    }
                    for ( i = 0; i < model->numLods; ++i )
                    {
                        v38 = &model->lodInfo[i];
                        v23 = model->maxs[0] - model->mins[0];
                        v24 = model->maxs[1] - model->mins[1];
                        v25 = model->maxs[2] - model->mins[2];
                        if ( (float)((float)(v23 * v24) - (float)(v24 * v25)) < 0.0 )
                            v12 = v24 * v25;
                        else
                            v12 = v23 * v24;
                        if ( (float)(v12 - (float)(v23 * v25)) < 0.0 )
                            v11 = v23 * v25;
                        else
                            v11 = v12;
                        v21 = v11;
                        if ( v11 < 0.0
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                                        746,
                                        0,
                                        "%s",
                                        "area >= 0") )
                        {
                            __debugbreak();
                        }
                        v22 = (float)(sqrtf(v21) * 128.0) * 1.5700001;
                        if ( v38->dist == 0.0 )
                        {
                            model->flags |= 0x80000u;
                            if ( i )
                            {
                                if ( (float)(v22 - *((float *)&model->localBoneNames + 8 * i)) < 0.0 )
                                    v10 = *((float *)&model->localBoneNames + 8 * i);
                                else
                                    v10 = v22;
                                v38->dist = v10;
                            }
                            else
                            {
                                v38->dist = v22;
                            }
                        }
                    }
                    FS_FreeFile(buffer);
                    if ( config.maxs[0] < 0.0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                                    788,
                                    0,
                                    "%s",
                                    "config.maxs[0] >= 0.0f") )
                    {
                        __debugbreak();
                    }
                    model->radius = config.maxs[0];
                    model->collLod = config.collLod;
                    if ( model->collLod >= model->numLods
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                                    793,
                                    0,
                                    "%s",
                                    "model->collLod < model->numLods") )
                    {
                        __debugbreak();
                    }
                    if ( config.physicsPresetFilename[0] )
                    {
                        v8 = XModel_PhysPresetPrecache(config.physicsPresetFilename, Alloc);
                        model->physPreset = v8;
                    }
                    if ( config.physicsConstraintsFilename[0] )
                    {
                        v9 = XModel_PhysConstraintsPrecache(config.physicsConstraintsFilename, Alloc);
                        model->physConstraints = v9;
                    }
                    if ( config.collMapFilename[0] )
                    {
                        XModel_LoadCollMap(config.collMapFilename, Alloc, model, name);
                    }
                    else if ( Com_sprintf(v20, 0x200u, "%s.map", name) > 0 )
                    {
                        XModel_LoadCollMap(v20, Alloc, model, name);
                    }
                    return model;
                }
                else
                {
LABEL_28:
                    FS_FreeFile(buffer);
                    return 0;
                }
            }
            else
            {
                Com_PrintError(19, "ERROR: xmodel '%s' has 0 length\n", name);
                FS_FreeFile(buffer);
                return 0;
            }
        }
        else
        {
            if ( buffer )
            {
                if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp", 515, 0, "%s", "!buf") )
                    __debugbreak();
            }
            Com_PrintError(19, "ERROR: xmodel '%s' not found\n", name);
            return 0;
        }
    }
    else
    {
        Com_PrintError(19, "ERROR: filename '%s' too long\n", dest);
        return 0;
    }
}

char __cdecl XModelLoadConfigFile(const char *name, float **pos, XModelConfig *config)
{
    char v4; // cl
    char v5; // cl
    char v6; // cl
    char v7; // dl
    XModelConfigEntry *v8; // [esp+18h] [ebp-9Ch]
    char *v9; // [esp+1Ch] [ebp-98h]
    char *physicsConstraintsFilename; // [esp+38h] [ebp-7Ch]
    char *v11; // [esp+3Ch] [ebp-78h]
    char *physicsPresetFilename; // [esp+58h] [ebp-5Ch]
    char *v13; // [esp+5Ch] [ebp-58h]
    char *collMapFilename; // [esp+78h] [ebp-3Ch]
    char *v15; // [esp+7Ch] [ebp-38h]
    int v16; // [esp+80h] [ebp-34h]
    float v17; // [esp+84h] [ebp-30h]
    float v18; // [esp+88h] [ebp-2Ch]
    float v19; // [esp+8Ch] [ebp-28h]
    float v20; // [esp+90h] [ebp-24h]
    float v21; // [esp+94h] [ebp-20h]
    float v22; // [esp+98h] [ebp-1Ch]
    float v23; // [esp+9Ch] [ebp-18h]
    float v24; // [esp+A0h] [ebp-14h]
    unsigned int v25; // [esp+A4h] [ebp-10h]
    __int16 v26; // [esp+A8h] [ebp-Ch]
    int i; // [esp+B0h] [ebp-4h]

    v26 = *(_WORD *)*pos;
    *pos = (float *)((char *)*pos + 2);
    if ( v26 == 62 )
    {
        v25 = *(unsigned int *)(*pos)++;
        config->flags = v25;
        v24 = *(*pos)++;
        config->mins[0] = v24;
        v23 = *(*pos)++;
        config->mins[1] = v23;
        v22 = *(*pos)++;
        config->mins[2] = v22;
        v21 = *(*pos)++;
        config->maxs[0] = v21;
        v20 = *(*pos)++;
        config->maxs[1] = v20;
        v19 = *(*pos)++;
        config->maxs[2] = v19;

        v15 = (char *)*pos;
        collMapFilename = config->collMapFilename;
        do
        {
            v4 = *v15;
            *collMapFilename++ = *v15++;
        }
        while ( v4 );
        *pos = (float *)((char *)*pos + strlen((const char *)*pos) + 1);
        v13 = (char *)*pos;
        physicsPresetFilename = config->physicsPresetFilename;
        do
        {
            v5 = *v13;
            *physicsPresetFilename++ = *v13++;
        }
        while ( v5 );
        *pos = (float *)((char *)*pos + strlen((const char *)*pos) + 1);

        v11 = (char *)*pos;
        physicsConstraintsFilename = config->physicsConstraintsFilename;
        do
        {
            v6 = *v11;
            *physicsConstraintsFilename++ = *v11++;
        }
        while ( v6 );
        *pos = (float *)((char *)*pos + strlen((const char *)*pos) + 1);
        v18 = *(*pos)++;
        config->bakeLightmapScale = v18;
        for ( i = 0; i < 4; ++i )
        {
            v17 = *(*pos)++;
            config->entries[i].dist = v17;
            v9 = (char *)*pos;
            v8 = &config->entries[i];
            do
            {
                v7 = *v9;
                v8->filename[0] = *v9++;
                v8 = (XModelConfigEntry *)((char *)v8 + 1);
            }
            while ( v7 );
            *pos = (float *)((char *)*pos + strlen((const char *)*pos) + 1);
        }
        v16 = *(unsigned int *)(*pos)++;
        config->collLod = v16;
        return 1;
    }
    else
    {
        Com_PrintError(19, "ERROR: xmodel '%s' out of date (version %d, expecting %d).\n", name, v26, 62);
        return 0;
    }
}

void __cdecl XModelLoadCollData(
                const unsigned __int8 **pos,
                XModel *model,
                void *(__cdecl *AllocColl)(int),
                const char *name)
{
    int v4; // [esp+0h] [ebp-B0h]
    int v5; // [esp+4h] [ebp-ACh]
    int v6; // [esp+8h] [ebp-A8h]
    float v7; // [esp+Ch] [ebp-A4h]
    float v8; // [esp+10h] [ebp-A0h]
    float v9; // [esp+14h] [ebp-9Ch]
    float v10; // [esp+18h] [ebp-98h]
    float v11; // [esp+1Ch] [ebp-94h]
    float v12; // [esp+20h] [ebp-90h]
    float *v13; // [esp+24h] [ebp-8Ch]
    float *svec; // [esp+28h] [ebp-88h]
    float v15; // [esp+2Ch] [ebp-84h]
    float v16; // [esp+30h] [ebp-80h]
    float v17; // [esp+34h] [ebp-7Ch]
    float v18; // [esp+38h] [ebp-78h]
    int v19; // [esp+3Ch] [ebp-74h]
    int v20; // [esp+40h] [ebp-70h]
    int v21; // [esp+44h] [ebp-6Ch]
    float v22; // [esp+48h] [ebp-68h]
    float v23; // [esp+4Ch] [ebp-64h]
    float v24; // [esp+54h] [ebp-5Ch]
    float v25; // [esp+58h] [ebp-58h]
    float v26; // [esp+5Ch] [ebp-54h]
    float v27; // [esp+60h] [ebp-50h]
    int v28; // [esp+64h] [ebp-4Ch]
    int v29; // [esp+68h] [ebp-48h]
    int j; // [esp+7Ch] [ebp-34h]
    float plane[4]; // [esp+80h] [ebp-30h] BYREF
    XModelCollTri_s *tri; // [esp+90h] [ebp-20h]
    XModelCollSurf_s *surf; // [esp+94h] [ebp-1Ch]
    float tvec[4]; // [esp+98h] [ebp-18h]
    int i; // [esp+A8h] [ebp-8h]
    int numCollTris; // [esp+ACh] [ebp-4h]

    if ( model->contents
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                    349,
                    0,
                    "%s",
                    "!model->contents") )
    {
        __debugbreak();
    }
    v29 = *(unsigned int *)*pos;
    *pos += 4;
    model->numCollSurfs = v29;
    if ( model->numCollSurfs )
    {
        model->collSurfs = (XModelCollSurf_s *)AllocColl(44 * model->numCollSurfs);
        for ( i = 0; i < model->numCollSurfs; ++i )
        {
            surf = &model->collSurfs[i];
            v28 = *(unsigned int *)*pos;
            *pos += 4;
            numCollTris = v28;
            if ( !v28
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                            364,
                            0,
                            "%s",
                            "numCollTris") )
            {
                __debugbreak();
            }
            surf->numCollTris = numCollTris;
            surf->collTris = (XModelCollTri_s *)AllocColl(48 * surf->numCollTris);
            for ( j = 0; j < numCollTris; ++j )
            {
                v27 = *(float *)*pos;
                *pos += 4;
                plane[0] = v27;
                v26 = *(float *)*pos;
                *pos += 4;
                plane[1] = v26;
                v25 = *(float *)*pos;
                *pos += 4;
                plane[2] = v25;
                v24 = *(float *)*pos;
                *pos += 4;
                plane[3] = v24;
                v23 = Vec3Length(plane) - 1.0;
                if ( fabs(v23) >= 0.0099999998
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                                378,
                                0,
                                "%s\n\t(name) = %s",
                                "(I_fabs( Vec3Length( plane ) - 1.0f ) < 0.01f)",
                                name) )
                {
                    __debugbreak();
                }
                v22 = *(float *)*pos;
                *pos += 4;
                v21 = *(unsigned int *)*pos;
                *pos += 4;
                v20 = *(unsigned int *)*pos;
                *pos += 4;
                v19 = *(unsigned int *)*pos;
                *pos += 4;
                v18 = *(float *)*pos;
                *pos += 4;
                tvec[0] = v18;
                v17 = *(float *)*pos;
                *pos += 4;
                tvec[1] = v17;
                v16 = *(float *)*pos;
                *pos += 4;
                tvec[2] = v16;
                v15 = *(float *)*pos;
                *pos += 4;
                tvec[3] = v15;
                tri = &surf->collTris[j];
                tri->plane[0] = plane[0];
                tri->plane[1] = plane[1];
                tri->plane[2] = plane[2];
                tri->plane[3] = plane[3];
                svec = tri->svec;
                tri->svec[0] = v22;
                *((unsigned int *)svec + 1) = v21;
                *((unsigned int *)svec + 2) = v20;
                *((unsigned int *)svec + 3) = v19;
                v13 = tri->tvec;
                tri->tvec[0] = tvec[0];
                v13[1] = tvec[1];
                v13[2] = tvec[2];
                v13[3] = tvec[3];
            }
            v12 = *(float *)*pos;
            *pos += 4;
            surf->mins[0] = v12 - 0.001;
            v11 = *(float *)*pos;
            *pos += 4;
            surf->mins[1] = v11 - 0.001;
            v10 = *(float *)*pos;
            *pos += 4;
            surf->mins[2] = v10 - 0.001;
            v9 = *(float *)*pos;
            *pos += 4;
            surf->maxs[0] = v9 + 0.001;
            v8 = *(float *)*pos;
            *pos += 4;
            surf->maxs[1] = v8 + 0.001;
            v7 = *(float *)*pos;
            *pos += 4;
            surf->maxs[2] = v7 + 0.001;
            v6 = *(unsigned int *)*pos;
            *pos += 4;
            surf->boneIdx = v6;
            v5 = *(unsigned int *)*pos;
            *pos += 4;
            surf->contents = v5 & 0xDFFFFFFB;
            if ( surf->contents
                && surf->boneIdx < 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                            409,
                            0,
                            "%s",
                            "!surf->contents || (surf->boneIdx >= 0)") )
            {
                __debugbreak();
            }
            v4 = *(unsigned int *)*pos;
            *pos += 4;
            surf->surfFlags = v4;
            model->contents |= surf->contents;
        }
    }
    else if ( model->collSurfs )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                        353,
                        0,
                        "%s",
                        "!model->collSurfs") )
            __debugbreak();
    }
}

XModelPartsLoad *__cdecl XModelPartsPrecache(
                XModel *model,
                const char *name,
                void *(__cdecl *Alloc)(int),
                __int64 *xmodelModifiedTime)
{
    XModelPartsLoad *modelParts; // [esp+0h] [ebp-4h]
    XModelPartsLoad *modelPartsa; // [esp+0h] [ebp-4h]

    modelParts = XModelPartsFindData(name);
    if ( modelParts )
        return modelParts;
    modelPartsa = XModelPartsLoadFile(model, name, Alloc, xmodelModifiedTime);
    if ( modelPartsa )
    {
        XModelPartsSetData(name, modelPartsa, Alloc);
        return modelPartsa;
    }
    else
    {
        Com_PrintError(19, "ERROR: Cannot find xmodelparts '%s'.\n", name);
        return 0;
    }
}

XModelPartsLoad *__cdecl XModelPartsLoadFile(
                XModel *model,
                const char *name,
                void *(__cdecl *Alloc)(int),
                __int64 *xmodelModifiedTime)
{
    unsigned __int8 *v5; // eax
    unsigned __int16 StringOfSize; // ax
    unsigned __int8 *v7; // [esp+10h] [ebp-B4h]
    int v8; // [esp+1Ch] [ebp-A8h]
    int v9; // [esp+20h] [ebp-A4h]
    int v10; // [esp+24h] [ebp-A0h]
    __int16 v11; // [esp+28h] [ebp-9Ch]
    __int16 v12; // [esp+2Ch] [ebp-98h]
    __int16 v13; // [esp+30h] [ebp-94h]
    const unsigned __int8 *pos; // [esp+34h] [ebp-90h] BYREF
    int numBones; // [esp+38h] [ebp-8Ch]
    char filename[64]; // [esp+3Ch] [ebp-88h] BYREF
    int numRootBones; // [esp+80h] [ebp-44h]
    __int16 numChildBones; // [esp+84h] [ebp-40h]
    unsigned __int8 *buf; // [esp+88h] [ebp-3Ch] BYREF
    float *trans; // [esp+8Ch] [ebp-38h]
    __int16 version; // [esp+90h] [ebp-34h]
    int len; // [esp+94h] [ebp-30h]
    int size; // [esp+98h] [ebp-2Ch]
    __int16 *quats; // [esp+9Ch] [ebp-28h]
    int fileSize; // [esp+A0h] [ebp-24h]
    unsigned __int8 *parentList; // [esp+A4h] [ebp-20h]
    int index; // [esp+A8h] [ebp-1Ch]
    __int64 xmodelPartsModifyTime; // [esp+ACh] [ebp-18h]
    int i; // [esp+B4h] [ebp-10h]
    XModelPartsLoad *modelParts; // [esp+B8h] [ebp-Ch]
    bool useBones; // [esp+BFh] [ebp-5h]
    unsigned __int16 *boneNames; // [esp+C0h] [ebp-4h]

    if ( Com_sprintf(filename, 0x40u, "xmodelparts/%s", name) >= 0 )
    {
        fileSize = FS_ReadFile(filename, (void **)&buf);
        xmodelPartsModifyTime = FS_GetModTime();
        if ( xmodelPartsModifyTime > *xmodelModifiedTime )
            *xmodelModifiedTime = xmodelPartsModifyTime;
        if ( fileSize >= 0 )
        {
            if ( fileSize )
            {
                if ( !buf
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp", 175, 0, "%s", "buf") )
                {
                    __debugbreak();
                }
                pos = buf;
                v13 = *(_WORD *)buf;
                pos = buf + 2;
                version = v13;
                if ( v13 == 62 )
                {
                    v12 = *(_WORD *)pos;
                    pos += 2;
                    numChildBones = v12;
                    v11 = *(_WORD *)pos;
                    pos += 2;
                    numRootBones = v11;
                    numBones = v11 + v12;
                    size = 2 * numBones;
                    boneNames = (unsigned __int16 *)Alloc(2 * numBones);
                    model->memUsage += size;
                    if ( numBones < 160 )
                    {
                        size = numChildBones;
                        if ( numChildBones )
                            v7 = (unsigned __int8 *)Alloc(size);
                        else
                            v7 = 0;
                        parentList = v7;
                        model->memUsage += size;
                        size = 28;
                        modelParts = (XModelPartsLoad *)Alloc(28);
                        model->memUsage += size;
                        modelParts->parentList = parentList;
                        modelParts->boneNames = boneNames;
                        size = 32 * numBones;
                        modelParts->baseMat = (DObjAnimMat *)Alloc(32 * numBones);
                        model->memUsage += size;
                        if ( numChildBones )
                        {
                            size = 8 * numChildBones;
                            modelParts->quats = (__int16 *)Alloc(size);
                            model->memUsage += size;
                            size = 16 * numChildBones;
                            modelParts->trans = (float *)Alloc(size);
                            model->memUsage += size;
                        }
                        else
                        {
                            modelParts->quats = 0;
                            modelParts->trans = 0;
                        }
                        size = numBones;
                        v5 = (unsigned __int8 *)Alloc(numBones);
                        modelParts->partClassification = v5;
                        model->memUsage += size;
                        modelParts->numBones = numBones;
                        if ( modelParts->numBones != numBones
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                                        238,
                                        0,
                                        "%s",
                                        "modelParts->numBones == numBones") )
                        {
                            __debugbreak();
                        }
                        modelParts->numRootBones = numRootBones;
                        if ( modelParts->numRootBones != numRootBones
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                                        241,
                                        0,
                                        "%s",
                                        "modelParts->numRootBones == numRootBones") )
                        {
                            __debugbreak();
                        }
                        quats = modelParts->quats;
                        trans = modelParts->trans;
                        i = numRootBones;
                        while ( i < numBones )
                        {
                            index = *pos++;
                            if ( index >= i
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                                            252,
                                            0,
                                            "%s",
                                            "index < i") )
                            {
                                __debugbreak();
                            }
                            *parentList = i - index;
                            if ( i - index != *parentList
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                                            254,
                                            0,
                                            "%s",
                                            "i - index == *parentList") )
                            {
                                __debugbreak();
                            }
                            v10 = *(unsigned int *)pos;
                            pos += 4;
                            *(unsigned int *)trans = v10;
                            v9 = *(unsigned int *)pos;
                            pos += 4;
                            *((unsigned int *)trans + 1) = v9;
                            v8 = *(unsigned int *)pos;
                            pos += 4;
                            *((unsigned int *)trans + 2) = v8;
                            ConsumeQuatNoSwap(&pos, quats);
                            ++i;
                            quats += 4;
                            trans += 3;
                            ++parentList;
                        }
                        for ( i = 0; i < numBones; ++i )
                        {
                            len = strlen((const char *)pos) + 1;
                            StringOfSize = SL_GetStringOfSize(SCRIPTINSTANCE_SERVER, (char *)pos, 0, len, 10);
                            boneNames[i] = StringOfSize;
                            pos += len;
                        }
                        memcpy(modelParts->partClassification, (unsigned __int8 *)pos, numBones);
                        pos += numBones;
                        useBones = *pos++ != 0;
                        FS_FreeFile(buf);
                        XModelCalcBasePose(modelParts);
                        if ( !useBones )
                            memset(modelParts->trans, 0, 16 * numChildBones);
                        return modelParts;
                    }
                    else
                    {
                        FS_FreeFile(buf);
                        Com_PrintError(19, "ERROR: xmodel '%s' has more than %d bones\n", name, 159);
                        return 0;
                    }
                }
                else
                {
                    FS_FreeFile(buf);
                    Com_PrintError(19, "ERROR: xmodelparts '%s' out of date (version %d, expecting %d).\n", name, version, 62);
                    return 0;
                }
            }
            else
            {
                Com_PrintError(19, "ERROR: xmodelparts '%s' has 0 length\n", name);
                FS_FreeFile(buf);
                return 0;
            }
        }
        else
        {
            if ( buf )
            {
                if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp", 163, 0, "%s", "!buf") )
                    __debugbreak();
            }
            Com_PrintError(19, "ERROR: xmodelparts '%s' not found\n", name);
            return 0;
        }
    }
    else
    {
        Com_PrintError(19, "ERROR: filename '%s' too long\n", filename);
        return 0;
    }
}

void __cdecl ConsumeQuatNoSwap(const unsigned __int8 **pos, __int16 *out)
{
    int v2; // [esp+8h] [ebp-2Ch]
    float v3; // [esp+Ch] [ebp-28h]
    __int16 v4; // [esp+14h] [ebp-20h]
    __int16 v5; // [esp+18h] [ebp-1Ch]
    __int16 v6; // [esp+1Ch] [ebp-18h]
    int temp; // [esp+30h] [ebp-4h]

    v6 = *(_WORD *)*pos;
    *pos += 2;
    *out = v6;
    v5 = *(_WORD *)*pos;
    *pos += 2;
    out[1] = v5;
    v4 = *(_WORD *)*pos;
    *pos += 2;
    out[2] = v4;
    temp = 1073676289 - (out[2] * out[2] + out[1] * out[1] + *out * *out);
    if ( temp <= 0 )
    {
        v2 = 0;
    }
    else
    {
        v3 = floor(sqrtf((float)temp) + 0.5);
        v2 = (int)v3;
    }
    if ( v2 != (__int16)v2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                    71,
                    0,
                    "%s",
                    "iQ[3] == (short)iQ[3]") )
    {
        __debugbreak();
    }
    out[3] = v2;
}

void __cdecl XModelCalcBasePose(XModelPartsLoad *modelParts)
{
    float v1; // [esp+6Ch] [ebp-30h]
    float *v2; // [esp+70h] [ebp-2Ch]
    int numBones; // [esp+74h] [ebp-28h]
    const float *trans; // [esp+78h] [ebp-24h]
    __int16 *quats; // [esp+7Ch] [ebp-20h]
    unsigned __int8 *parentList; // [esp+80h] [ebp-1Ch]
    int i; // [esp+84h] [ebp-18h]
    int ia; // [esp+84h] [ebp-18h]
    float tempQuat[4]; // [esp+88h] [ebp-14h] BYREF
    DObjAnimMat *quatTrans; // [esp+98h] [ebp-4h]

    parentList = modelParts->parentList;
    numBones = modelParts->numBones;
    quats = modelParts->quats;
    trans = modelParts->trans;
    quatTrans = modelParts->baseMat;
    for ( i = modelParts->numRootBones; i; --i )
    {
        quatTrans->quat[0] = 0.0f;
        quatTrans->quat[1] = 0.0f;
        quatTrans->quat[2] = 0.0f;
        quatTrans->quat[3] = 1.0f;
        v2 = quatTrans->trans;
        quatTrans->trans[0] = 0.0f;
        v2[1] = 0.0f;
        v2[2] = 0.0f;
        quatTrans->transWeight = 2.0f;
        ++quatTrans;
    }
    ia = numBones - modelParts->numRootBones;
    while ( ia )
    {
        tempQuat[0] = (float)*quats * 0.000030518509;
        tempQuat[1] = (float)quats[1] * 0.000030518509;
        tempQuat[2] = (float)quats[2] * 0.000030518509;
        tempQuat[3] = (float)quats[3] * 0.000030518509;
        QuatMultiply(tempQuat, quatTrans[-*parentList].quat, quatTrans->quat);
        v1 = Vec4LengthSq(quatTrans->quat);
        if ( v1 == 0.0 )
        {
            quatTrans->quat[3] = 1.0f;
            quatTrans->transWeight = 2.0f;
        }
        else
        {
            quatTrans->transWeight = 2.0 / v1;
        }
        MatrixTransformVectorQuatTrans(trans, &quatTrans[-*parentList], quatTrans->trans);
        --ia;
        quats += 4;
        trans += 3;
        ++quatTrans;
        ++parentList;
    }
}

int __cdecl XModelAllowLoadMesh()
{
    return 0;
}

void __cdecl XModelGenerateHighMipVolume(XModel *model)
{
    double v1; // st7
    float var14C; // [esp+8h] [ebp-14Ch]
    float v3; // [esp+Ch] [ebp-148h]
    unsigned int v4; // [esp+10h] [ebp-144h]
    float newRadius; // [esp+40h] [ebp-114h]
    float radiusJ; // [esp+44h] [ebp-110h]
    float dist; // [esp+48h] [ebp-10Ch]
    XModelHighMipBounds *boundsJ; // [esp+4Ch] [ebp-108h]
    float radius; // [esp+50h] [ebp-104h]
    XModelHighMipBounds *v2; // [esp+54h] [ebp-100h]
    int surfJ; // [esp+58h] [ebp-FCh]
    int surfI; // [esp+5Ch] [ebp-F8h]
    int vertIndex; // [esp+60h] [ebp-F4h]
    float coverage; // [esp+64h] [ebp-F0h]
    float area; // [esp+68h] [ebp-ECh]
    int triCorner; // [esp+6Ch] [ebp-E8h]
    float fakeTexCoord[3][2]; // [esp+70h] [ebp-E4h] BYREF
    float triPos[3][3]; // [esp+88h] [ebp-CCh] BYREF
    unsigned int width; // [esp+ACh] [ebp-A8h]
    float fakePos[3][3]; // [esp+B0h] [ebp-A4h] BYREF
    float radiusForTexture; // [esp+D4h] [ebp-80h] BYREF
    float cumulativeCoverage; // [esp+D8h] [ebp-7Ch]
    float triTexCoord[3][2]; // [esp+DCh] [ebp-78h] BYREF
    float cumulativeArea; // [esp+F4h] [ebp-60h]
    unsigned int uvmax; // [esp+F8h] [ebp-5Ch]
    int triangleIndex; // [esp+FCh] [ebp-58h]
    unsigned int maxFilterState; // [esp+100h] [ebp-54h]
    int texIter; // [esp+104h] [ebp-50h]
    Material *material; // [esp+108h] [ebp-4Ch]
    unsigned int filterState; // [esp+10Ch] [ebp-48h] BYREF
    float surfBoundBoxMins[3]; // [esp+110h] [ebp-44h] BYREF
    float radiusForModel; // [esp+11Ch] [ebp-38h]
    XModelHighMipBounds *bounds; // [esp+120h] [ebp-34h]
    float surfBoundBoxMaxs[3]; // [esp+124h] [ebp-30h] BYREF
    XSurface *surface; // [esp+130h] [ebp-24h]
    float texSize[2]; // [esp+134h] [ebp-20h] BYREF
    int lod; // [esp+13Ch] [ebp-18h]
    int surfCount; // [esp+140h] [ebp-14h]
    Material *const *materials; // [esp+144h] [ebp-10h]
    XSurface *surfaces; // [esp+148h] [ebp-Ch] BYREF
    int surfIter; // [esp+14Ch] [ebp-8h]
    int surfCountPrevLods; // [esp+150h] [ebp-4h]

    surfCount = 0;
    surfCountPrevLods = 0;
    model->streamInfo.highMipBounds = (XModelHighMipBounds *)Hunk_Alloc(
                                                                                                                         16 * model->numsurfs,
                                                                                                                         "XModelGenerateHighMipVolume",
                                                                                                                         22);
    for ( lod = 0; lod < 4; ++lod )
    {
        surfCount = XModelGetSurfaces(model, &surfaces, lod);
        materials = XModelGetSkins(model, lod);
        for ( surfIter = 0; surfIter != surfCount; ++surfIter )
        {
            maxFilterState = 1;
            if ( surfIter + surfCountPrevLods >= (unsigned int)model->numsurfs
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                            992,
                            0,
                            "surfCountPrevLods+surfIter doesn't index model->numsurfs\n\t%i not in [0, %i)",
                            surfIter + surfCountPrevLods,
                            model->numsurfs) )
            {
                __debugbreak();
            }
            bounds = &model->streamInfo.highMipBounds[surfIter + surfCountPrevLods];
            surface = &surfaces[surfIter];
            if ( !surface
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp", 998, 0, "%s", "surface") )
            {
                __debugbreak();
            }
            material = materials[surfIter];
            if ( !material
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp", 1001, 0, "%s", "material") )
            {
                __debugbreak();
            }
            ClearBounds(surfBoundBoxMins, surfBoundBoxMaxs);
            radiusForModel = 0.0f;
            for ( texIter = 0; texIter != material->textureCount; ++texIter )
            {
                cumulativeCoverage = 0.0f;
                cumulativeArea = 0.0f;
                radiusForTexture = 0.0f;
                if ( R_StreamGetMaterialTextureSize(material, texIter, texSize, &filterState) )
                {
                    v4 = (int)maxFilterState < (int)filterState ? filterState : maxFilterState;
                    maxFilterState = v4;
                    for ( triangleIndex = 0; triangleIndex < surface->triCount; ++triangleIndex )
                    {
                        for ( triCorner = 0; triCorner != 3; ++triCorner )
                        {
                            vertIndex = surface->triIndices[3 * triangleIndex + triCorner];
                            XModelGetSurfaceVertexPos(surface, vertIndex, triPos[triCorner]);
                            XModelGetSurfaceTexCoords(surface, vertIndex, triTexCoord[triCorner]);
                        }
                        coverage = R_CalculateTriangleArea2D(triTexCoord);
                        area = R_CalculateTriangleArea3D(triPos);
                        if ( coverage > 0.0 && area > 0.0 )
                        {
                            AddPointToBounds(triPos[0], surfBoundBoxMins, surfBoundBoxMaxs);
                            AddPointToBounds(triPos[1], surfBoundBoxMins, surfBoundBoxMaxs);
                            AddPointToBounds(triPos[2], surfBoundBoxMins, surfBoundBoxMaxs);
                            cumulativeCoverage = cumulativeCoverage + coverage;
                            cumulativeArea = cumulativeArea + area;
                        }
                        ++triangleIndex;
                    }
                    uvmax = sqrtf(cumulativeCoverage);
                    width = sqrtf(cumulativeArea);
                    memset(fakePos, 0, 12);
                    *(_QWORD *)&fakePos[1][0] = __PAIR64__(width, 0);
                    fakePos[1][2] = 0.0f;
                    *(_QWORD *)&fakePos[2][0] = __PAIR64__(0, width);
                    fakePos[2][2] = 0.0f;
                    fakeTexCoord[0][0] = 0.0f;
                    fakeTexCoord[0][1] = 0.0f;
                    *(_QWORD *)&fakeTexCoord[1][0] = __PAIR64__(uvmax, 0);
                    *(_QWORD *)&fakeTexCoord[2][0] = __PAIR64__(0, uvmax);
                    if ( R_CalculateTriangleTopMipAabb(fakePos, fakeTexCoord, texSize, 2, 0, 0, &radiusForTexture) )
                    {
                        if ( (float)(radiusForModel - radiusForTexture) < 0.0 )
                            v3 = radiusForTexture;
                        else
                            v3 = radiusForModel;
                        radiusForModel = v3;
                    }
                }
            }
            v1 = BoxMaxDimension(surfBoundBoxMins, surfBoundBoxMaxs);
            radiusForModel = v1 * 0.5 + radiusForModel;
            bounds->himipRadiusSq = radiusForModel * radiusForModel;
            Vec3Lerp(surfBoundBoxMins, surfBoundBoxMaxs, 0.5, bounds->center);
        }
        surfCountPrevLods += surfCount;
    }
    for ( surfI = 1; surfI < model->numsurfs; ++surfI )
    {
        for ( surfJ = 0; surfJ < surfI; ++surfJ )
        {
            v2 = &model->streamInfo.highMipBounds[surfI];
            if ( Material_FirstIncludesSecond(model->materialHandles[surfJ], model->materialHandles[surfI]) )
            {
                boundsJ = &model->streamInfo.highMipBounds[surfJ];
                dist = Vec3Distance(boundsJ->center, v2->center);
                radius = sqrtf(v2->himipRadiusSq);
                radiusJ = sqrtf(boundsJ->himipRadiusSq);
                if ( radiusJ >= (float)(dist + radius) )
                    goto LABEL_54;
                if ( radius >= (float)(dist + radiusJ) )
                {
                    boundsJ->himipRadiusSq = v2->himipRadiusSq;
                    boundsJ->center[0] = v2->center[0];
                    boundsJ->center[1] = v2->center[1];
                    boundsJ->center[2] = v2->center[2];
LABEL_54:
                    v2->himipRadiusSq = 0.0f;
                    break;
                }
                newRadius = (float)((float)(radius + dist) + radiusJ) / 2.0;
                if ( (float)(radius - radiusJ) < 0.0 )
                    var14C = radiusJ;
                else
                    var14C = radius;
                if ( (float)(newRadius / var14C) <= 1.05 )
                {
                    boundsJ->himipRadiusSq = newRadius * newRadius;
                    if ( !VecNCompareCustomEpsilon(v2->center, boundsJ->center, 0.001, 3) )
                        Vec3Lerp(
                            v2->center,
                            boundsJ->center,
                            (float)(newRadius - radius) / (float)((float)((float)(newRadius - radius) + newRadius) - radiusJ),
                            boundsJ->center);
                    goto LABEL_54;
                }
            }
        }
    }
}

double __cdecl BoxMaxDimension(const float *mins, const float *maxs)
{
    float v4; // [esp+4h] [ebp-10h]

    if ( (float)((float)(*maxs - *mins) - (float)(maxs[1] - mins[1])) < 0.0 )
        v4 = maxs[1] - mins[1];
    else
        v4 = *maxs - *mins;
    if ( (float)(v4 - (float)(maxs[2] - mins[2])) < 0.0 )
        return (float)(maxs[2] - mins[2]);
    else
        return v4;
}

char __cdecl Material_FirstIncludesSecond(Material *mat1, Material *mat2)
{
    int i; // [esp+0h] [ebp-4h]

    if ( mat1 == mat2 )
        return 1;
    for ( i = 0; i < mat2->textureCount; ++i )
    {
        if ( !Material_IncludesImage(mat1, mat2->textureTable[i].u.image) )
            return 0;
    }
    return 1;
}

char __cdecl Material_IncludesImage(Material *mat, GfxImage *image)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < mat->textureCount; ++i )
    {
        if ( mat->textureTable[i].u.image == image )
            return 1;
    }
    return 0;
}

void __cdecl XModelGetSurfaceVertexPos(const XSurface *surface, int vertIndex, float *out)
{
    GfxPackedVertex *v3; // [esp+0h] [ebp-4h]

    v3 = &surface->verts0[vertIndex];
    *out = v3->xyz[0];
    out[1] = v3->xyz[1];
    out[2] = v3->xyz[2];
}

void __cdecl XModelGetSurfaceTexCoords(const XSurface *surface, int vertIndex, float *out)
{
    Vec2UnpackTexCoords(surface->verts0[vertIndex].texCoord, out);
}

int __cdecl XModelSurfsPrecache(
                XModel *model,
                const char *name,
                void *(__cdecl *Alloc)(int),
                __int16 modelNumsurfs,
                const char *modelName,
                XModelSurfs *outModelSurfs,
                __int64 *xmodelModifiedTime)
{
    XModelSurfs *modelSurfs; // [esp+0h] [ebp-4h]
    XModelSurfs *modelSurfsa; // [esp+0h] [ebp-4h]

    modelSurfs = XModelSurfsFindData(name);
    if ( modelSurfs )
    {
        *outModelSurfs = *modelSurfs;
        return 1;
    }
    else
    {
        modelSurfsa = R_XModelSurfsLoadFile(model, name, Alloc, modelNumsurfs, modelName, xmodelModifiedTime);
        if ( modelSurfsa )
        {
            XModelSurfsSetData(name, modelSurfsa, Alloc);
            *outModelSurfs = *modelSurfsa;
            return 1;
        }
        else
        {
            Com_PrintError(19, "ERROR: Cannot find 'xmodelsurfs '%s'.\n", name);
            return 0;
        }
    }
}

XModelSurfs *__cdecl R_XModelSurfsLoadFile(
                XModel *model,
                const char *name,
                void *(__cdecl *Alloc)(int),
                __int16 modelNumsurfs,
                const char *modelName,
                __int64 *xmodelModifiedTime)
{
    __int16 v7; // [esp+4h] [ebp-74h]
    __int16 v8; // [esp+8h] [ebp-70h]
    const unsigned __int8 *pos; // [esp+Ch] [ebp-6Ch] BYREF
    char filename[64]; // [esp+10h] [ebp-68h] BYREF
    unsigned __int8 *buf; // [esp+54h] [ebp-24h] BYREF
    __int16 version; // [esp+58h] [ebp-20h]
    XModelSurfs *modelSurfs; // [esp+5Ch] [ebp-1Ch]
    int size; // [esp+60h] [ebp-18h]
    int fileSize; // [esp+64h] [ebp-14h]
    __int64 xmodelSurfsModifyTime; // [esp+68h] [ebp-10h]
    __int16 numsurfs; // [esp+74h] [ebp-4h]

    if ( Com_sprintf(filename, 0x40u, "xmodelsurfs/%s", name) >= 0 )
    {
        fileSize = FS_ReadFile(filename, (void **)&buf);
        xmodelSurfsModifyTime = FS_GetModTime();
        if ( xmodelSurfsModifyTime > *xmodelModifiedTime )
            *xmodelModifiedTime = xmodelSurfsModifyTime;
        if ( fileSize >= 0 )
        {
            if ( fileSize )
            {
                if ( !buf
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp", 1157, 0, "%s", "buf") )
                {
                    __debugbreak();
                }
                pos = buf;
                v8 = *(_WORD *)buf;
                pos = buf + 2;
                version = v8;
                if ( v8 == 62 )
                {
                    v7 = *(_WORD *)pos;
                    pos += 2;
                    numsurfs = v7;
                    if ( v7 == modelNumsurfs )
                    {
                        size = 68 * modelNumsurfs + 24;
                        modelSurfs = (XModelSurfs *)Alloc(size);
                        model->memUsage += size;
                        modelSurfs->surfs = (XSurface *)&modelSurfs[1];
                        XModelReadSurfaces(
                            model,
                            name,
                            modelSurfs,
                            modelSurfs->partBits,
                            modelNumsurfs,
                            (unsigned __int8 **)&pos,
                            Alloc);
                        FS_FreeFile(buf);
                        if ( !modelSurfs
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                                        1186,
                                        0,
                                        "%s",
                                        "modelSurfs") )
                        {
                            __debugbreak();
                        }
                        return modelSurfs;
                    }
                    else
                    {
                        FS_FreeFile(buf);
                        Com_PrintError(
                            19,
                            "ERROR: File conflict (between non-iwd and iwd file) on xmodelsurfs '%s' for xmodel '%s'.\n"
                            "Rename the export file to fix.\n",
                            name,
                            modelName);
                        return 0;
                    }
                }
                else
                {
                    FS_FreeFile(buf);
                    Com_PrintError(19, "ERROR: xmodelsurfs '%s' out of date (version %d, expecting %d).\n", name, version, 62);
                    return 0;
                }
            }
            else
            {
                Com_PrintError(19, "ERROR: xmodelsurf '%s' has 0 length\n", name);
                FS_FreeFile(buf);
                return 0;
            }
        }
        else
        {
            if ( buf )
            {
                if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp", 1145, 0, "%s", "!buf") )
                    __debugbreak();
            }
            Com_PrintError(19, "ERROR: xmodelsurf '%s' not found\n", name);
            return 0;
        }
    }
    else
    {
        Com_PrintError(19, "ERROR: filename '%s' too long\n", filename);
        return 0;
    }
}

void __cdecl XModelReadSurfaces(
                XModel *model,
                const char *name,
                XModelSurfs *modelSurfs,
                int *modelPartBits,
                int surfCount,
                unsigned __int8 **pos,
                void *(__cdecl *AllocMesh)(int))
{
    int j; // [esp+4h] [ebp-18h]
    int surfIndex; // [esp+8h] [ebp-14h]
    int baseVertIndex; // [esp+Ch] [ebp-10h]
    XSurface *xsurf; // [esp+10h] [ebp-Ch]
    int baseTriIndex; // [esp+14h] [ebp-8h]
    XSurface *surfs; // [esp+18h] [ebp-4h]

    surfs = modelSurfs->surfs;
    if ( !modelSurfs->surfs
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp", 831, 0, "%s", "surfs") )
    {
        __debugbreak();
    }
    if ( !modelPartBits
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp", 832, 0, "%s", "modelPartBits") )
    {
        __debugbreak();
    }
    if ( surfCount <= 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp", 833, 0, "%s", "surfCount > 0") )
    {
        __debugbreak();
    }
    if ( !pos && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp", 834, 0, "%s", "pos") )
        __debugbreak();
    if ( !*pos && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp", 835, 0, "%s", "*pos") )
        __debugbreak();
    baseTriIndex = 0;
    baseVertIndex = 0;
    for ( surfIndex = 0; surfIndex < surfCount; ++surfIndex )
    {
        xsurf = &surfs[surfIndex];
        XModelReadSurface(model, pos, AllocMesh, xsurf);
        for ( j = 0; j < 5; ++j )
            modelPartBits[j] |= xsurf->partBits[j];
        xsurf->baseTriIndex = baseTriIndex;
        xsurf->baseVertIndex = baseVertIndex;
        baseTriIndex += xsurf->triCount;
        baseVertIndex += xsurf->vertCount;
    }
    if ( baseTriIndex > 0xFFFF )
        Com_PrintError(
            19,
            "%d indicies in xmodel '%s' exceeds maximum of %d, model will not render correctly on all platforms\n",
            baseTriIndex,
            name,
            0xFFFF);
    if ( baseVertIndex > 0xFFFF )
        Com_PrintError(
            19,
            "%d vertices in xmodel '%s' exceeds maximum of %d, model will not render correctly on all platforms\n",
            baseVertIndex,
            name,
            0xFFFF);
}

int __cdecl XModelGetFlags(const XModel *model)
{
    return model->flags;
}

XModelPieces *__cdecl XModelPiecesLoadFile(const char *name, void *(__cdecl *Alloc)(int))
{
    char v3; // [esp+3h] [ebp-D9h]
    char *v4; // [esp+8h] [ebp-D4h]
    char *v5; // [esp+Ch] [ebp-D0h]
    float v6; // [esp+20h] [ebp-BCh]
    float v7; // [esp+24h] [ebp-B8h]
    float v8; // [esp+28h] [ebp-B4h]
    __int16 v9; // [esp+30h] [ebp-ACh]
    XModelPiece *piece; // [esp+38h] [ebp-A4h]
    int pieceIndex; // [esp+3Ch] [ebp-A0h]
    const unsigned __int8 *pos; // [esp+40h] [ebp-9Ch]
    const unsigned __int8 *posa; // [esp+40h] [ebp-9Ch]
    const unsigned __int8 *posb; // [esp+40h] [ebp-9Ch]
    const unsigned __int8 *posc; // [esp+40h] [ebp-9Ch]
    char filename[64]; // [esp+44h] [ebp-98h] BYREF
    char piecename[64]; // [esp+84h] [ebp-58h] BYREF
    unsigned __int8 *buf; // [esp+C8h] [ebp-14h] BYREF
    int version; // [esp+CCh] [ebp-10h]
    XModelPieces *xmodelPieces; // [esp+D0h] [ebp-Ch]
    int len; // [esp+D4h] [ebp-8h]
    int fileSize; // [esp+D8h] [ebp-4h]

    if ( Com_sprintf(filename, 0x40u, "xmodelpieces/%s", name) >= 0 )
    {
        fileSize = FS_ReadFile(filename, (void **)&buf);
        if ( fileSize >= 0 )
        {
            if ( fileSize )
            {
                if ( !buf
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp", 1439, 0, "%s", "buf") )
                {
                    __debugbreak();
                }
                pos = buf + 2;
                version = *(__int16 *)buf;
                if ( version == 1 )
                {
                    xmodelPieces = (XModelPieces *)Alloc(12);
                    if ( !xmodelPieces
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                                    1451,
                                    0,
                                    "%s",
                                    "xmodelPieces") )
                    {
                        __debugbreak();
                    }
                    v9 = *(_WORD *)pos;
                    posa = pos + 2;
                    xmodelPieces->numpieces = v9;
                    xmodelPieces->pieces = (XModelPiece *)Alloc(16 * xmodelPieces->numpieces);
                    for ( pieceIndex = 0; pieceIndex < xmodelPieces->numpieces; ++pieceIndex )
                    {
                        piece = &xmodelPieces->pieces[pieceIndex];
                        len = strlen((const char *)posa) + 1;
                        if ( len > 64 )
                        {
                            Com_PrintError(19, "ERROR: piecename '%s' too long\n", (const char *)posa);
                            FS_FreeFile(buf);
                            return 0;
                        }
                        v5 = (char *)posa;
                        v4 = piecename;
                        do
                        {
                            v3 = *v5;
                            *v4++ = *v5++;
                        }
                        while ( v3 );
                        posb = &posa[len];
                        piece->model = R_RegisterModel(piecename);
                        if ( !piece->model )
                        {
                            Com_PrintError(1, "ERROR: xmodel piece '%s' missing from pieces model 's%'\n", piecename, filename);
                            FS_FreeFile(buf);
                            return 0;
                        }
                        v8 = *(float *)posb;
                        posc = posb + 4;
                        piece->offset[0] = v8;
                        v7 = *(float *)posc;
                        posc += 4;
                        piece->offset[1] = v7;
                        v6 = *(float *)posc;
                        posa = posc + 4;
                        piece->offset[2] = v6;
                    }
                    FS_FreeFile(buf);
                    return xmodelPieces;
                }
                else
                {
                    FS_FreeFile(buf);
                    Com_PrintError(19, "ERROR: xmodelpieces '%s' out of date (version %d, expecting %d).\n", name, version, 1);
                    return 0;
                }
            }
            else
            {
                Com_PrintError(19, "ERROR: xmodelpieces '%s' has 0 length\n", name);
                FS_FreeFile(buf);
                return 0;
            }
        }
        else
        {
            if ( buf )
            {
                if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp", 1427, 0, "%s", "!buf") )
                    __debugbreak();
            }
            Com_PrintError(19, "ERROR: xmodelpieces '%s' not found\n", name);
            return 0;
        }
    }
    else
    {
        Com_PrintError(19, "ERROR: filename '%s' too long\n", filename);
        return 0;
    }
}

XModelPieces *__cdecl XModelPiecesPrecache(const char *name, void *(__cdecl *Alloc)(int))
{
    XModelPieces *xmodelPieces; // [esp+0h] [ebp-4h]
    XModelPieces *xmodelPiecesa; // [esp+0h] [ebp-4h]

    if ( !name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp", 1497, 0, "%s", "name") )
    {
        __debugbreak();
    }
    if ( !*name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp", 1498, 0, "%s", "name[0]") )
    {
        __debugbreak();
    }
    xmodelPieces = (XModelPieces *)Hunk_FindDataForFile(11, name);
    if ( xmodelPieces )
        return xmodelPieces;
    xmodelPiecesa = XModelPiecesLoadFile(name, Alloc);
    if ( xmodelPiecesa )
    {
        xmodelPiecesa->name = Hunk_SetDataForFile(11, name, xmodelPiecesa, Alloc);
        return xmodelPiecesa;
    }
    else
    {
        Com_PrintError(20, "ERROR: Cannot find xmodel pieces '%s'.\n", name);
        return 0;
    }
}

int __cdecl XModelGetStaticBounds(const XModel *model, float (*axis)[3], float *mins, float *maxs)
{
    float v5; // [esp+0h] [ebp-34h]
    float v6; // [esp+4h] [ebp-30h]
    float v7; // [esp+8h] [ebp-2Ch]
    int j; // [esp+Ch] [ebp-28h]
    int k; // [esp+10h] [ebp-24h]
    const XModelCollSurf_s *csurf; // [esp+14h] [ebp-20h]
    float rotated[3]; // [esp+18h] [ebp-1Ch] BYREF
    int i; // [esp+24h] [ebp-10h]
    float corner[3]; // [esp+28h] [ebp-Ch] BYREF

    if ( model->numCollSurfs )
    {
        *mins = FLT_MAX;
        mins[1] = FLT_MAX;
        mins[2] = FLT_MAX;
        *maxs = -FLT_MAX;
        maxs[1] = -FLT_MAX;
        maxs[2] = -FLT_MAX;
        for ( i = 0; i < model->numCollSurfs; ++i )
        {
            csurf = &model->collSurfs[i];
            for ( k = 0; k < 8; ++k )
            {
                if ( (k & 1) != 0 )
                    v7 = csurf->mins[0];
                else
                    v7 = csurf->maxs[0];
                corner[0] = v7;
                if ( (k & 2) != 0 )
                    v6 = csurf->mins[1];
                else
                    v6 = csurf->maxs[1];
                corner[1] = v6;
                if ( (k & 4) != 0 )
                    v5 = csurf->mins[2];
                else
                    v5 = csurf->maxs[2];
                corner[2] = v5;
                MatrixTransformVector(corner, axis, rotated);
                for ( j = 0; j < 3; ++j )
                {
                    if ( mins[j] > rotated[j] )
                        mins[j] = rotated[j];
                    if ( rotated[j] > maxs[j] )
                        maxs[j] = rotated[j];
                }
            }
        }
        return 1;
    }
    else
    {
        if ( model->contents )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                            1538,
                            0,
                            "%s",
                            "!model->contents") )
                __debugbreak();
        }
        return 0;
    }
}

int __cdecl XModelGetStaticBoundsExact(const XModel *model, float (*axis)[3], float *mins, float *maxs)
{
    int j; // [esp+0h] [ebp-3Ch]
    float rotated[3]; // [esp+4h] [ebp-38h] BYREF
    int vi; // [esp+10h] [ebp-2Ch]
    int ti; // [esp+14h] [ebp-28h]
    unsigned __int16 *triIndices; // [esp+18h] [ebp-24h]
    int triCount; // [esp+1Ch] [ebp-20h]
    XRigidVertList *vertList; // [esp+20h] [ebp-1Ch]
    int vli; // [esp+24h] [ebp-18h]
    int vertListCount; // [esp+28h] [ebp-14h]
    XSurface *surf; // [esp+2Ch] [ebp-10h]
    int i; // [esp+30h] [ebp-Ch]
    int nsurfs; // [esp+34h] [ebp-8h]
    XSurface *surfaces; // [esp+38h] [ebp-4h] BYREF

    if ( model->numCollSurfs )
    {
        *mins = FLT_MAX;
        mins[1] = FLT_MAX;
        mins[2] = FLT_MAX;
        *maxs = -FLT_MAX;
        maxs[1] = -FLT_MAX;
        maxs[2] = -FLT_MAX;
        nsurfs = XModelGetSurfaces(model, &surfaces, model->collLod);
        for ( i = 0; i < nsurfs; ++i )
        {
            surf = &surfaces[i];
            vertListCount = surf->vertListCount;
            for ( vli = 0; vli < vertListCount; ++vli )
            {
                vertList = &surf->vertList[vli];
                triCount = vertList->triCount;
                triIndices = &surf->triIndices[3 * vertList->triOffset];
                ti = 0;
                while ( ti < triCount )
                {
                    for ( vi = 0; vi < 3; ++vi )
                    {
                        MatrixTransformVector(surf->verts0[triIndices[vi]].xyz, axis, rotated);
                        for ( j = 0; j < 3; ++j )
                        {
                            if ( mins[j] > rotated[j] )
                                mins[j] = rotated[j];
                            if ( rotated[j] > maxs[j] )
                                maxs[j] = rotated[j];
                        }
                    }
                    ++ti;
                    triIndices += 3;
                }
            }
        }
        return 1;
    }
    else
    {
        if ( model->contents )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                            1575,
                            0,
                            "%s",
                            "!model->contents") )
                __debugbreak();
        }
        return 0;
    }
}

int __cdecl XModelGetStaticBounds(const XModel *model, float *mins, float *maxs)
{
    float *v4; // [esp+0h] [ebp-Ch]
    float *v5; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    if ( model->numCollSurfs )
    {
        v5 = model->collSurfs->mins;
        *mins = *v5;
        mins[1] = v5[1];
        mins[2] = v5[2];
        v4 = model->collSurfs->maxs;
        *maxs = *v4;
        maxs[1] = v4[1];
        maxs[2] = v4[2];
        for ( i = 1; i < model->numCollSurfs; ++i )
        {
            Vec3Min(mins, model->collSurfs[i].mins, mins);
            Vec3Max(maxs, model->collSurfs[i].maxs, maxs);
        }
        return 1;
    }
    else
    {
        if ( model->contents )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_load_obj.cpp",
                            1663,
                            0,
                            "%s",
                            "!model->contents") )
                __debugbreak();
        }
        return 0;
    }
}

