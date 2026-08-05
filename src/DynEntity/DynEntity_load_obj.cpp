#include "DynEntity_load_obj.h"
#include "DynEntity_client.h"
#include <qcommon/cm_load.h>
#include <physics/physpreset_load_obj.h>
#include <universal/com_memory.h>
#include <xanim/xmodel_load_obj.h>
#include "DynEntity_coll.h"
#include <qcommon/com_bsp_load_obj.h>
#include <qcommon/common.h>
#include <physics/rope.h>
#include <clientscript/cscr_stringlist.h>
#include <universal/com_math_anglevectors.h>
#include <xanim/xmodel_utils.h>
#include <EffectsCore/fx_load_obj.h>
#include <sound/snd_utils.h>

int num_ents;
int num_ropes;
int num_infos;
int num_dents;
node_params_t ents_params[8192];
node_params_t rope_params[8192];
node_params_t info_params[8192];
node_params_t dents_params[8192];

int numAutoPhysPresets;
PhysPreset *autoPhysPresets[512];

const DynEntityProps dynEntProps[3] =
{
  { "invalid", false, false, false, false },
  { "clutter", false, false, true, false },
  { "destruct", false, false, false, true }
};


DynEntityClient *__cdecl DynEnt_GetClientEntity(unsigned __int16 dynEntId, DynEntityDrawType drawType)
{
    if ( dynEntId >= (unsigned int)cm.dynEntCount[drawType]
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\dynentity\\DynEntity_load_obj.h",
                    67,
                    0,
                    "dynEntId doesn't index cm.dynEntCount[DynEntGetClientCollType( drawType )]\n\t%i not in [0, %i)",
                    dynEntId,
                    cm.dynEntCount[drawType]) )
    {
        __debugbreak();
    }
    return &cm.dynEntClientList[drawType][dynEntId];
}

PhysPreset *__cdecl DynEnt_PhysPresetPrecache(const char *name)
{
    if ( !name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp", 599, 0, "%s", "name") )
    {
        __debugbreak();
    }
    return PhysPresetPrecache(name, (void *(__cdecl *)(int))DynEnt_AllocPhysPreset);
}

unsigned __int8 *__cdecl DynEnt_AllocPhysPreset(int size)
{
    if ( size <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    587,
                    0,
                    "%s",
                    "size > 0") )
    {
        __debugbreak();
    }
    return Hunk_Alloc(size, "DynEnt_AllocPhysPreset", 22);
}

XModelPieces *__cdecl DynEnt_XModelPiecesPrecache(const char *name)
{
    if ( !name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp", 605, 0, "%s", "name") )
    {
        __debugbreak();
    }
    return XModelPiecesPrecache(name, (void *(__cdecl *)(int))DynEnt_AllocXModelPieces);
}

unsigned __int8 *__cdecl DynEnt_AllocXModelPieces(int size)
{
    if ( size <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    593,
                    0,
                    "%s",
                    "size > 0") )
    {
        __debugbreak();
    }
    return Hunk_Alloc(size, "DynEnt_AllocXModelPieces", 22);
}

void __cdecl DynEnt_LoadEntities()
{
    char *v0; // eax
    char v1; // [esp+3h] [ebp-31AEDh]
    char *target; // [esp+8h] [ebp-31AE8h]
    char *v3; // [esp+Ch] [ebp-31AE4h]
    char v4; // [esp+13h] [ebp-31ADDh]
    char *targetname; // [esp+18h] [ebp-31AD8h]
    char *v6; // [esp+1Ch] [ebp-31AD4h]
    char v7; // [esp+23h] [ebp-31ACDh]
    char *physPresetFile; // [esp+28h] [ebp-31AC8h]
    char *v9; // [esp+2Ch] [ebp-31AC4h]
    char v10; // [esp+33h] [ebp-31ABDh]
    char *destroySound; // [esp+38h] [ebp-31AB8h]
    char *v12; // [esp+3Ch] [ebp-31AB4h]
    char v13; // [esp+43h] [ebp-31AADh]
    char *v14; // [esp+48h] [ebp-31AA8h]
    char *v15; // [esp+4Ch] [ebp-31AA4h]
    char v16; // [esp+53h] [ebp-31A9Dh]
    char *physModelName; // [esp+58h] [ebp-31A98h]
    unsigned char *v18; // [esp+5Ch] [ebp-31A94h]
    char v19; // [esp+63h] [ebp-31A8Dh]
    char *v20; // [esp+68h] [ebp-31A88h]
    char *v21; // [esp+6Ch] [ebp-31A84h]
    char v22; // [esp+73h] [ebp-31A7Dh]
    char *destroyedModelName; // [esp+78h] [ebp-31A78h]
    unsigned char *v24; // [esp+7Ch] [ebp-31A74h]
    char v25; // [esp+83h] [ebp-31A6Dh]
    char *v26; // [esp+88h] [ebp-31A68h]
    char *v27; // [esp+8Ch] [ebp-31A64h]
    char v28; // [esp+93h] [ebp-31A5Dh]
    char *modelName; // [esp+98h] [ebp-31A58h]
    unsigned char *v30; // [esp+9Ch] [ebp-31A54h]
    char v31; // [esp+A3h] [ebp-31A4Dh]
    DynEntityCreateParams *p_dest; // [esp+A8h] [ebp-31A48h]
    char *v33; // [esp+ACh] [ebp-31A44h]
    float v34; // [esp+BCh] [ebp-31A34h]
    float p1[3]; // [esp+C0h] [ebp-31A30h] BYREF
    float p0[3]; // [esp+CCh] [ebp-31A24h] BYREF
    PhysConstraint *constraint; // [esp+D8h] [ebp-31A18h]
    int jj; // [esp+DCh] [ebp-31A14h]
    int ii; // [esp+E0h] [ebp-31A10h]
    PhysConstraint *v44; // [esp+E4h] [ebp-31A0Ch]
    int n; // [esp+E8h] [ebp-31A08h]
    const DynEntityDef *v46; // [esp+ECh] [ebp-31A04h]
    unsigned __int16 m; // [esp+F0h] [ebp-31A00h]
    int k; // [esp+F4h] [ebp-319FCh]
    DynEntityDef *EntityDef; // [esp+F8h] [ebp-319F8h]
    int j; // [esp+FCh] [ebp-319F4h]
    int v51; // [esp+100h] [ebp-319F0h]
    DynEntityDrawType drawType; // [esp+104h] [ebp-319ECh]
    DynEntityCollType collType; // [esp+108h] [ebp-319E8h]
    signed int i; // [esp+10Ch] [ebp-319E4h]
    void *base; // [esp+110h] [ebp-319E0h]
    int entCount; // [esp+114h] [ebp-319DCh] BYREF
    DynEntityConstraintCreateParams constraintParams; // [esp+118h] [ebp-319D8h] BYREF
    char *v58; // [esp+31790h] [ebp-360h]
    unsigned int num; // [esp+31794h] [ebp-35Ch]
    char s0[64]; // [esp+31798h] [ebp-358h] BYREF
    DynEntityCreateParams dest; // [esp+317D8h] [ebp-318h] BYREF
    char *data_p; // [esp+31A9Ch] [ebp-54h] BYREF
    char IsValidClassName; // [esp+31AA3h] [ebp-4Dh]
    char *src; // [esp+31AA4h] [ebp-4Ch]
    char className[7]; // [esp+31AA8h] [ebp-48h] BYREF
    _BYTE v66[61]; // [esp+31AAFh] [ebp-41h] BYREF

    LoadRopeParams();
    if ( !Com_EntityString(0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    1074,
                    0,
                    "%s",
                    "Com_EntityString( NULL )") )
    {
        __debugbreak();
    }
    cm.dynEntDefList[0] = 0;
    cm.dynEntDefList[1] = 0;
    cm.dynEntPoseList[0] = 0;
    cm.dynEntPoseList[1] = 0;
    cm.dynEntClientList[0] = 0;
    cm.dynEntClientList[1] = 0;
    cm.dynEntServerList[0] = 0;
    cm.dynEntServerList[1] = 0;
    cm.dynEntCollList[0] = 0;
    cm.dynEntCollList[1] = 0;
    cm.dynEntCollList[2] = 0;
    cm.dynEntCollList[3] = 0;
    *(unsigned int *)cm.dynEntCount = 0;
    *(unsigned int *)&cm.dynEntCount[2] = 0;
    cm.constraints = 0;
    v0 = Com_EntityString(0);
    DynEnt_GetEntityCountFromString(v0, &entCount, &cm.num_constraints);
    if ( cm.num_constraints >= 511 )
        Com_Error(ERR_DROP, "Max number of contraints exceeded [%d]\n", 511);
    if ( entCount >= 4096 )
        Com_Error(ERR_DROP, "Found [%d] Dyn Entities, Max is [%d]\n", entCount, 4095);
    entCount += 256;
    base = Hunk_Alloc(84 * entCount, "DynEnt_LoadDefs", 10);
    num = 0;
    if ( cm.num_constraints )
    {
        cm.constraints = (PhysConstraint *)DynEnt_Alloc(cm.num_constraints, 168);
        DynEnt_GetEntityConstraintParams(&constraintParams, cm.constraints, cm.num_constraints);
    }
    data_p = Com_EntityString(0);
    while ( 1 )
    {
        src = (char *)Com_Parse((const char **)&data_p);
        if ( !data_p || *src != 123 )
            break;
        IsValidClassName = 0;
        Com_Memset((unsigned int *)&dest, 0, 700);
        while ( 1 )
        {
            src = (char *)Com_Parse((const char **)&data_p);
            if ( !data_p )
                break;
            if ( *src == 125 )
                break;
            I_strncpyz(s0, src, 64);
            src = (char *)Com_Parse((const char **)&data_p);
            if ( !data_p )
                break;
            I_strncpyz(className, src, 64);
            if ( I_stricmp(s0, "classname") )
            {
                if ( I_stricmp(s0, "type") )
                {
                    if ( I_stricmp(s0, "model") )
                    {
                        if ( I_stricmp(s0, "destroyedModel") )
                        {
                            if ( I_stricmp(s0, "physicsmodel") )
                            {
                                if ( I_stricmp(s0, "destroyEfx") )
                                {
                                    if ( I_stricmp(s0, "destroySound") )
                                    {
                                        if ( I_stricmp(s0, "destroyPieces") )
                                        {
                                            if ( I_stricmp(s0, "origin") )
                                            {
                                                if ( I_stricmp(s0, "angles") )
                                                {
                                                    if ( I_stricmp(s0, "health") )
                                                    {
                                                        if ( I_stricmp(s0, "physPreset") )
                                                        {
                                                            if ( I_stricmp(s0, "centerofmass") )
                                                            {
                                                                if ( I_stricmp(s0, "targetname") )
                                                                {
                                                                    if ( I_stricmp(s0, "target") )
                                                                    {
                                                                        if ( I_stricmp(s0, "mass") )
                                                                        {
                                                                            if ( I_stricmp(s0, "bounch") )
                                                                            {
                                                                                if ( I_stricmp(s0, "friction") )
                                                                                {
                                                                                    if ( !I_stricmp(s0, "spawnflags") )
                                                                                    {
                                                                                        v51 = atoi(className);
                                                                                        if ( (v51 & 1) != 0 )
                                                                                            dest.startInPhysics = 1;
                                                                                        if ( (v51 & 2) != 0 )
                                                                                            dest.autoActivate = 1;
                                                                                        if ( (v51 & 4) != 0 )
                                                                                            dest.noBulletDamage = 1;
                                                                                        if ( (v51 & 8) != 0 )
                                                                                            dest.noExplosiveDamage = 1;
                                                                                        if ( (v51 & 0x10) != 0 )
                                                                                            dest.alwaysFloats = 1;
                                                                                    }
                                                                                }
                                                                                else
                                                                                {
                                                                                    dest.friction = atof(className);
                                                                                }
                                                                            }
                                                                            else
                                                                            {
                                                                                dest.bounce = atof(className);
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                            dest.mass = atof(className);
                                                                        }
                                                                    }
                                                                    else
                                                                    {
                                                                        v3 = className;
                                                                        target = dest.target;
                                                                        do
                                                                        {
                                                                            v1 = *v3;
                                                                            *target++ = *v3++;
                                                                        }
                                                                        while ( v1 );
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    v6 = className;
                                                                    targetname = dest.targetname;
                                                                    do
                                                                    {
                                                                        v4 = *v6;
                                                                        *targetname++ = *v6++;
                                                                    }
                                                                    while ( v4 );
                                                                }
                                                            }
                                                            else
                                                            {
                                                                sscanf(
                                                                    className,
                                                                    "%f %f %f",
                                                                    dest.centerOfMass,
                                                                    &dest.centerOfMass[1],
                                                                    &dest.centerOfMass[2]);
                                                            }
                                                        }
                                                        else
                                                        {
                                                            v9 = className;
                                                            physPresetFile = dest.physPresetFile;
                                                            do
                                                            {
                                                                v7 = *v9;
                                                                *physPresetFile++ = *v9++;
                                                            }
                                                            while ( v7 );
                                                        }
                                                    }
                                                    else
                                                    {
                                                        dest.health = atoi(className);
                                                    }
                                                }
                                                else
                                                {
                                                    sscanf(className, "%f %f %f", dest.angles, &dest.angles[1], &dest.angles[2]);
                                                }
                                            }
                                            else
                                            {
                                                sscanf(className, "%f %f %f", dest.origin, &dest.origin[1], &dest.origin[2]);
                                            }
                                        }
                                        else
                                        {
                                            Com_StripExtension(className, dest.destroyPiecesFile);
                                        }
                                    }
                                    else
                                    {
                                        v12 = className;
                                        destroySound = dest.destroySound;
                                        do
                                        {
                                            v10 = *v12;
                                            *destroySound++ = *v12++;
                                        }
                                        while ( v10 );
                                    }
                                }
                                else
                                {
                                    Com_StripExtension(className, dest.destroyFxFile);
                                }
                            }
                            else if ( Com_IsLegacyXModelName(className) )
                            {
                                v18 = v66;
                                physModelName = dest.physModelName;
                                do
                                {
                                    v16 = *v18;
                                    *physModelName++ = *v18++;
                                }
                                while ( v16 );
                            }
                            else
                            {
                                v15 = className;
                                v14 = dest.physModelName;
                                do
                                {
                                    v13 = *v15;
                                    *v14++ = *v15++;
                                }
                                while ( v13 );
                            }
                        }
                        else if ( Com_IsLegacyXModelName(className) )
                        {
                            v24 = v66;
                            destroyedModelName = dest.destroyedModelName;
                            do
                            {
                                v22 = *v24;
                                *destroyedModelName++ = *v24++;
                            }
                            while ( v22 );
                        }
                        else
                        {
                            v21 = className;
                            v20 = dest.destroyedModelName;
                            do
                            {
                                v19 = *v21;
                                *v20++ = *v21++;
                            }
                            while ( v19 );
                        }
                    }
                    else if ( Com_IsLegacyXModelName(className) )
                    {
                        v30 = v66;
                        modelName = dest.modelName;
                        do
                        {
                            v28 = *v30;
                            *modelName++ = *v30++;
                        }
                        while ( v28 );
                    }
                    else
                    {
                        v27 = className;
                        v26 = dest.modelName;
                        do
                        {
                            v25 = *v27;
                            *v26++ = *v27++;
                        }
                        while ( v25 );
                    }
                }
                else
                {
                    v33 = className;
                    p_dest = &dest;
                    do
                    {
                        v31 = *v33;
                        p_dest->typeName[0] = *v33++;
                        p_dest = (DynEntityCreateParams *)((char *)p_dest + 1);
                    }
                    while ( v31 );
                }
            }
            else
            {
                IsValidClassName = DynEnt_IsValidClassName(className);
            }
        }
        if ( IsValidClassName )
        {
            if ( (int)num >= entCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                            1233,
                            0,
                            "%s",
                            "dynEntCount < dynEntStringCount") )
            {
                __debugbreak();
            }
            if ( DynEnt_Create((DynEntityDef *)base + num, &dest, &constraintParams, cm.num_constraints) )
                ++num;
        }
    }
    qsort(base, num, 0x54u, (int (__cdecl *)(const void *, const void *))DynEnt_CompareEntities);
    for ( i = 0; i < (int)num; ++i )
    {
        v58 = (char *)base + 84 * i;
        if ( *((unsigned int *)v58 + 8) )
        {
            drawType = DYNENT_DRAW_MODEL;
            *((unsigned int *)v58 + 18) |= 0x200u;
        }
        else
        {
            if ( !*((_WORD *)v58 + 20)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                            1252,
                            0,
                            "%s",
                            "dynEntDef->brushModel") )
            {
                __debugbreak();
            }
            drawType = DYNENT_DRAW_BRUSH;
            *((unsigned int *)v58 + 18) |= 0x400u;
        }
        if ( !cm.dynEntPoseList[drawType - 2] )
            cm.dynEntPoseList[drawType - 2] = (DynEntityPose *)v58;
        collType = (DynEntityCollType)drawType;
        ++cm.dynEntCount[drawType];
        collType = (DynEntityCollType)(drawType + 2);
        ++cm.dynEntCount[drawType + 2];
    }
    if ( !cm.dynEntDefList[0] )
        cm.dynEntDefList[0] = (DynEntityDef *)((char *)base + 84 * num);
    if ( cm.dynEntCount[1] + cm.dynEntCount[0] != num
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    1272,
                    0,
                    "%s",
                    "cm.dynEntCount[DYNENT_COLL_CLIENT_MODEL] + cm.dynEntCount[DYNENT_COLL_CLIENT_BRUSH] == dynEntCount") )
    {
        __debugbreak();
    }
    cm.originalDynEntCount = cm.dynEntCount[0];
    cm.dynEntCount[0] += 256;
    for ( j = cm.originalDynEntCount; j < cm.originalDynEntCount + 256; ++j )
    {
        EntityDef = (DynEntityDef*)DynEnt_GetEntityDef(j, DYNENT_DRAW_MODEL);
        for ( k = 0; k < 4; ++k )
            EntityDef->physConstraints[k] = 511;
    }
    for ( drawType = DYNENT_DRAW_MODEL; (unsigned int)drawType < DYNENT_DRAW_COUNT; ++drawType )
    {
        collType = (DynEntityCollType)drawType;
        num = DynEnt_GetEntityCount((DynEntityCollType)drawType);
        for ( m = 0; m < (int)num; ++m )
        {
            v46 = DynEnt_GetEntityDef(m, drawType);
            for ( n = 0; n < 4; ++n )
            {
                if ( v46->physConstraints[n] != 511 )
                {
                    v44 = &cm.constraints[v46->physConstraints[n]];
                    if ( v44->attach_point_type1 == ATTACH_POINT_DYNENT && v44->target_ent1 == v46->targetname )
                        v44->target_index1 = DynEnt_GetClientAbsId(m, drawType);
                    if ( v44->attach_point_type2 == ATTACH_POINT_DYNENT && v44->target_ent2 == v46->targetname )
                        v44->target_index2 = DynEnt_GetClientAbsId(m, drawType);
                }
            }
        }
    }
    for ( ii = 0; ii < num_ents; ++ii )
        FindConstraints(
            ents_params[ii].targetname,
            ents_params[ii].origin,
            ents_params[ii].angles,
            ATTACH_POINT_ENT,
            &constraintParams,
            cm.num_constraints,
            0);
    cm.max_ropes = 0;
    for ( jj = 0; jj < cm.num_constraints; ++jj )
    {
        constraint = &cm.constraints[jj];
        if ( constraint->type == CONSTRAINT_ROPE )
        {
            if ( constraint->distance == 0.0 )
            {
                GetAttachPointsInWorldSpace(constraint, p0, p1);
                //v34 = (float)((float)((float)(p0 - p1) * (float)(p0 - p1)) + (float)((float)(v39 - v36) * (float)(v39 - v36))) + (float)((float)(v40 - v37) * (float)(v40 - v37));
                v34 = (float)((float)((float)(p0[0] - p1[0]) * (float)(p0[0] - p1[0]))
                    + (float)((float)(p0[1] - p1[1]) * (float)(p0[1] - p1[1])))
                    + (float)((float)(p0[2] - p1[2]) * (float)(p0[2] - p1[2]));
                if ( v34 <= 0.0099999998 )
                    constraint->distance = 100.0f;
                else
                    constraint->distance = sqrtf(v34);
            }
            if ( constraint->scale[1] != 0.0 )
                constraint->distance = constraint->distance * constraint->scale[1];
            ++cm.max_ropes;
        }
    }
    cm.max_ropes += 32;
    if ( cm.max_ropes > 300 )
    {
        Com_Error(ERR_DROP, "Max ropes count exceeded! %d.\n", 300);
        cm.max_ropes = 300;
    }
    cm.ropes = (rope_t *)Hunk_Alloc(3188 * cm.max_ropes, "ropes", 10);
    for ( drawType = DYNENT_DRAW_MODEL; (unsigned int)drawType < DYNENT_DRAW_COUNT; ++drawType )
    {
        collType = (DynEntityCollType)drawType;
        if ( cm.dynEntCount[drawType] )
        {
            cm.dynEntPoseList[drawType] = (DynEntityPose *)DynEnt_Alloc(cm.dynEntCount[collType], 32);
            cm.dynEntClientList[drawType] = (DynEntityClient *)DynEnt_Alloc(cm.dynEntCount[collType], 20);
            cm.dynEntCollList[collType] = (DynEntityColl *)DynEnt_Alloc(cm.dynEntCount[collType], 32);
        }
        collType = (DynEntityCollType)(drawType + 2);
        if ( cm.dynEntCount[drawType + 2] )
        {
            cm.dynEntServerList[drawType] = (DynEntityServer *)DynEnt_Alloc(cm.dynEntCount[collType], 8);
            cm.dynEntCollList[collType] = (DynEntityColl *)DynEnt_Alloc(cm.dynEntCount[collType], 32);
        }
    }
}

const char *dynEntClassNames[2] = { "dyn_brushmodel", "dyn_model" };
char __cdecl DynEnt_IsValidClassName(const char *className)
{
    unsigned int classIndex; // [esp+0h] [ebp-4h]

    if ( !className
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    157,
                    0,
                    "%s",
                    "className") )
    {
        __debugbreak();
    }
    for ( classIndex = 0; classIndex < 2; ++classIndex )
    {
        if ( !I_stricmp(className, dynEntClassNames[classIndex]) )
            return 1;
    }
    return 0;
}

void __cdecl DynEnt_GetEntityCountFromString(const char *entityString, int *entCount, int *constraintCount)
{
    char key[64]; // [esp+0h] [ebp-90h] BYREF
    const char *ptr; // [esp+40h] [ebp-50h] BYREF
    const char *token; // [esp+44h] [ebp-4Ch]
    char value[68]; // [esp+48h] [ebp-48h] BYREF

    if ( !entityString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    217,
                    0,
                    "%s",
                    "entityString") )
    {
        __debugbreak();
    }
    ptr = entityString;
    *entCount = 0;
    *constraintCount = 0;
    while ( 1 )
    {
        token = (const char *)Com_Parse(&ptr);
        if ( !ptr || *token != 123 )
            break;
        while ( 1 )
        {
            token = (const char *)Com_Parse(&ptr);
            if ( !ptr )
                break;
            if ( *token == 125 )
                break;
            I_strncpyz(key, token, 64);
            token = (const char *)Com_Parse(&ptr);
            if ( !ptr )
                break;
            I_strncpyz(value, token, 64);
            if ( !I_stricmp(key, "classname") )
            {
                if ( DynEnt_IsValidClassName(value) )
                {
                    ++*entCount;
                }
                else if ( DynEnt_IsValidConstraintName(value) )
                {
                    ++*constraintCount;
                }
            }
        }
    }
}

const char *dynEntConstraintClassNames[4] = { "dyn_pointconstraint", "dyn_hingeconstraint", "dyn_lightconstraint", "rope" };

char __cdecl DynEnt_IsValidConstraintName(const char *className)
{
    unsigned int classIndex; // [esp+0h] [ebp-4h]

    if ( !className
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    184,
                    0,
                    "%s",
                    "className") )
    {
        __debugbreak();
    }
    for ( classIndex = 0; classIndex < 4; ++classIndex )
    {
        if ( !I_stricmp(className, dynEntConstraintClassNames[classIndex]) )
            return 1;
    }
    return 0;
}

parseInfo_t *LoadRopeParams()
{
    parseInfo_t *result; // eax
    char v1; // [esp+Bh] [ebp-1E9h]
    char *target2; // [esp+10h] [ebp-1E4h]
    char *v3; // [esp+14h] [ebp-1E0h]
    char v4; // [esp+1Bh] [ebp-1D9h]
    char *target; // [esp+20h] [ebp-1D4h]
    char *v6; // [esp+24h] [ebp-1D0h]
    char v7; // [esp+2Bh] [ebp-1C9h]
    char *targetname; // [esp+30h] [ebp-1C4h]
    char *v9; // [esp+34h] [ebp-1C0h]
    bool is_rope; // [esp+3Ah] [ebp-1BAh]
    bool is_dent; // [esp+3Bh] [ebp-1B9h]
    node_params_t params; // [esp+3Ch] [ebp-1B8h] BYREF
    bool is_ent; // [esp+162h] [ebp-92h]
    bool is_info; // [esp+163h] [ebp-91h]
    char key[64]; // [esp+164h] [ebp-90h] BYREF
    const char *ptr; // [esp+1A4h] [ebp-50h] BYREF
    const char *token; // [esp+1A8h] [ebp-4Ch]
    char value[68]; // [esp+1ACh] [ebp-48h] BYREF

    ptr = Com_EntityString(0);
    num_dents = 0;
    num_ents = 0;
    num_infos = 0;
    num_ropes = 0;
    while ( 1 )
    {
        result = Com_Parse(&ptr);
        token = (const char *)result;
        if ( !ptr || *token != 123 )
            break;
        Com_Memset((unsigned int *)&params, 0, 288);
        is_rope = 0;
        is_info = 0;
        is_ent = 0;
        is_dent = 0;
        while ( 1 )
        {
            token = (const char *)Com_Parse(&ptr);
            if ( !ptr )
                break;
            if ( *token == 125 )
                break;
            I_strncpyz(key, token, 64);
            token = (const char *)Com_Parse(&ptr);
            if ( !ptr )
                break;
            I_strncpyz(value, token, 64);
            if ( I_stricmp(key, "classname") )
            {
                if ( I_stricmp(key, "origin") )
                {
                    if ( I_stricmp(key, "angles") )
                    {
                        if ( I_stricmp(key, "targetname") )
                        {
                            if ( I_stricmp(key, "target") )
                            {
                                if ( !I_stricmp(key, "target2") )
                                {
                                    v3 = value;
                                    target2 = params.target2;
                                    do
                                    {
                                        v1 = *v3;
                                        *target2++ = *v3++;
                                    }
                                    while ( v1 );
                                }
                            }
                            else
                            {
                                v6 = value;
                                target = params.target;
                                do
                                {
                                    v4 = *v6;
                                    *target++ = *v6++;
                                }
                                while ( v4 );
                            }
                        }
                        else
                        {
                            v9 = value;
                            targetname = params.targetname;
                            do
                            {
                                v7 = *v9;
                                *targetname++ = *v9++;
                            }
                            while ( v7 );
                            params.targetname_hash = SL_GetLowercaseString(params.targetname, 0, SCRIPTINSTANCE_SERVER);
                        }
                    }
                    else
                    {
                        sscanf(value, "%f %f %f", params.angles, &params.angles[1], &params.angles[2]);
                    }
                }
                else
                {
                    sscanf(value, "%f %f %f", params.origin, &params.origin[1], &params.origin[2]);
                }
            }
            else if ( I_stricmp(value, "rope") )
            {
                if ( I_stricmp(value, "info_null") )
                {
                    if ( I_stricmp(value, "script_model") )
                    {
                        if ( I_stricmp(value, "script_brushmodel") )
                        {
                            if ( I_stricmp(value, "dyn_model") )
                            {
                                if ( !I_stricmp(value, "dyn_brushmodel") )
                                    is_dent = 1;
                            }
                            else
                            {
                                is_dent = 1;
                            }
                        }
                        else
                        {
                            is_ent = 1;
                        }
                    }
                    else
                    {
                        is_ent = 1;
                    }
                }
                else
                {
                    is_info = 1;
                }
            }
            else
            {
                is_rope = 1;
            }
        }
        if ( is_rope )
            memcpy(&rope_params[num_ropes++], &params, sizeof(node_params_t));
        if ( is_info )
            memcpy(&info_params[num_infos++], &params, sizeof(node_params_t));
        if ( is_ent )
            memcpy(&ents_params[num_ents++], &params, sizeof(node_params_t));
        if ( is_dent )
            memcpy(&dents_params[num_dents++], &params, sizeof(node_params_t));
    }
    return result;
}

void __cdecl DynEnt_GetEntityConstraintParams(
                DynEntityConstraintCreateParams *constraintParams,
                PhysConstraint *constraints,
                int dynEntConstraintCount)
{
    char v3; // [esp+3h] [ebp-EDh]
    char *materialname; // [esp+8h] [ebp-E8h]
    char *v5; // [esp+Ch] [ebp-E4h]
    char v6; // [esp+13h] [ebp-DDh]
    char *target2; // [esp+18h] [ebp-D8h]
    char *v8; // [esp+1Ch] [ebp-D4h]
    char v9; // [esp+23h] [ebp-CDh]
    char *target; // [esp+28h] [ebp-C8h]
    char *v11; // [esp+2Ch] [ebp-C4h]
    char v12; // [esp+33h] [ebp-BDh]
    char *targetname; // [esp+38h] [ebp-B8h]
    char *v14; // [esp+3Ch] [ebp-B4h]
    char v15; // [esp+43h] [ebp-ADh]
    DynEntityConstraintCreateParams *v16; // [esp+48h] [ebp-A8h]
    char *v17; // [esp+4Ch] [ebp-A4h]
    char isDynEntConstraint; // [esp+53h] [ebp-9Dh]
    DynEntityConstraintCreateParams *params; // [esp+54h] [ebp-9Ch]
    char key[68]; // [esp+58h] [ebp-98h] BYREF
    const char *ptr; // [esp+9Ch] [ebp-54h] BYREF
    const char *token; // [esp+A0h] [ebp-50h]
    int count; // [esp+A4h] [ebp-4Ch]
    char value[68]; // [esp+A8h] [ebp-48h] BYREF

    ptr = Com_EntityString(0);
    if ( !ptr
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp", 442, 0, "%s", "ptr") )
    {
        __debugbreak();
    }
    count = 0;
LABEL_5:
    token = (const char *)Com_Parse(&ptr);
    if ( ptr && *token == 123 )
    {
        isDynEntConstraint = 0;
        if ( count >= 511 )
        {
            Com_Error(ERR_DROP, "Max number of contraints exceeded (%d)\n", 511);
            return;
        }
        params = &constraintParams[count];
        Com_Memset((unsigned int *)params, 0, 396);
        while ( 1 )
        {
            while ( 1 )
            {
                while ( 1 )
                {
                    while ( 1 )
                    {
                        while ( 1 )
                        {
                            while ( 1 )
                            {
                                while ( 1 )
                                {
                                    while ( 1 )
                                    {
                                        while ( 1 )
                                        {
                                            while ( 1 )
                                            {
                                                token = (const char *)Com_Parse(&ptr);
                                                if ( !ptr
                                                    || *token == 125
                                                    || (I_strncpyz(key, token, 64), token = (const char *)Com_Parse(&ptr), !ptr) )
                                                {
                                                    if ( isDynEntConstraint )
                                                    {
                                                        if ( count >= dynEntConstraintCount
                                                            && !Assert_MyHandler(
                                                                        "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                                                                        556,
                                                                        0,
                                                                        "%s",
                                                                        "count < dynEntConstraintCount") )
                                                        {
                                                            __debugbreak();
                                                        }
                                                        if ( DynEnt_CreateConstraint(&constraints[count], params) )
                                                            ++count;
                                                    }
                                                    goto LABEL_5;
                                                }
                                                I_strncpyz(value, token, 64);
                                                if ( I_stricmp(key, "classname") )
                                                    break;
                                                isDynEntConstraint = DynEnt_IsValidConstraintName(value);
                                                if ( isDynEntConstraint )
                                                {
                                                    v17 = value;
                                                    v16 = params;
                                                    do
                                                    {
                                                        v15 = *v17;
                                                        v16->classname[0] = *v17++;
                                                        v16 = (DynEntityConstraintCreateParams *)((char *)v16 + 1);
                                                    }
                                                    while ( v15 );
                                                }
                                            }
                                            if ( I_stricmp(key, "origin") )
                                                break;
                                            sscanf(value, "%f %f %f", params->origin, &params->origin[1], &params->origin[2]);
                                        }
                                        if ( I_stricmp(key, "angles") )
                                            break;
                                        sscanf(value, "%f %f %f", params->angles, &params->angles[1], &params->angles[2]);
                                    }
                                    if ( I_stricmp(key, "health") )
                                        break;
                                    params->health = atoi(value);
                                }
                                if ( I_stricmp(key, "targetname") )
                                    break;
                                v14 = value;
                                targetname = params->targetname;
                                do
                                {
                                    v12 = *v14;
                                    *targetname++ = *v14++;
                                }
                                while ( v12 );
                            }
                            if ( I_stricmp(key, "target") )
                                break;
                            v11 = value;
                            target = params->target;
                            do
                            {
                                v9 = *v11;
                                *target++ = *v11++;
                            }
                            while ( v9 );
                        }
                        if ( I_stricmp(key, "target2") )
                            break;
                        v8 = value;
                        target2 = params->target2;
                        do
                        {
                            v6 = *v8;
                            *target2++ = *v8++;
                        }
                        while ( v6 );
                    }
                    if ( I_stricmp(key, "min_angle") )
                        break;
                    params->minAngle = atof(value);
                }
                if ( I_stricmp(key, "max_angle") )
                    break;
                params->maxAngle = atof(value);
            }
            if ( I_stricmp(key, "length") )
            {
                if ( I_stricmp(key, "length_scale") )
                {
                    if ( !I_stricmp(key, "distance") )
                        goto LABEL_43;
                    if ( I_stricmp(key, "width") )
                    {
                        if ( I_stricmp(key, "force_scale") )
                        {
                            if ( I_stricmp(key, "damp") )
                            {
                                if ( I_stricmp(key, "material") )
                                {
                                    if ( I_stricmp(key, "spawnflags") )
                                    {
                                        if ( !I_stricmp(key, "timeout") )
                                            params->timeout = atof(value);
                                    }
                                    else
                                    {
                                        params->spawnflags = atoi(value);
                                    }
                                }
                                else
                                {
                                    v5 = value;
                                    materialname = params->materialname;
                                    do
                                    {
                                        v3 = *v5;
                                        *materialname++ = *v5++;
                                    }
                                    while ( v3 );
                                }
                            }
                            else
                            {
                                params->damp = atof(value);
                            }
                        }
                        else
                        {
                            params->force_scale = atof(value);
                        }
                    }
                    else
                    {
                        params->width = atof(value);
                    }
                }
                else
                {
                    params->scale = atof(value);
                }
            }
            else
            {
LABEL_43:
                params->distance = atof(value);
            }
        }
    }
    if ( count != dynEntConstraintCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    562,
                    0,
                    "%s",
                    "count == dynEntConstraintCount") )
    {
        __debugbreak();
    }
}

char __cdecl DynEnt_CreateConstraint(PhysConstraint *constraint, DynEntityConstraintCreateParams *params)
{
    node_params_t *info_node; // [esp+38h] [ebp-8h]
    int j; // [esp+3Ch] [ebp-4h]

    constraint->attach_point_type1 = ATTACH_POINT_WORLD;
    constraint->attach_point_type2 = ATTACH_POINT_WORLD;
    constraint->target_index1 = -1;
    constraint->target_index2 = -1;
    constraint->constraintHandle = 0;
    constraint->rope_index = -1;
    constraint->type = CONSTRAINT_NONE;
    if ( I_stricmp("dyn_hingeconstraint", params->classname) )
    {
        if ( I_stricmp("dyn_pointconstraint", params->classname) )
        {
            if ( I_stricmp("rope", params->classname) )
            {
                if ( !I_stricmp("dyn_lightconstraint", params->classname) )
                {
                    constraint->type = CONSTRAINT_LIGHT;
                    constraint->target_index2 = atoi(params->target2);
                    constraint->centity_num[0] = -1;
                }
            }
            else
            {
                constraint->type = CONSTRAINT_ROPE;
                constraint->minAngle = params->width;
                if ( params->materialname[0] )
                    constraint->material = Material_RegisterHandle(params->materialname, 8);
                if ( (params->spawnflags & 1) != 0 )
                    constraint->flags |= 1u;
            }
        }
        else
        {
            constraint->type = CONSTRAINT_POINT;
        }
    }
    else
    {
        constraint->type = CONSTRAINT_HINGE;
        constraint->minAngle = params->minAngle;
        constraint->maxAngle = params->maxAngle;
    }
    if ( constraint->type == CONSTRAINT_NONE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    397,
                    0,
                    "%s",
                    "constraint.type != CONSTRAINT_NONE") )
    {
        __debugbreak();
    }
    constraint->targetname = SL_GetLowercaseString(params->targetname, 0, SCRIPTINSTANCE_SERVER);
    constraint->damp = params->damp;
    constraint->pos[0] = params->origin[0];
    constraint->pos[1] = params->origin[1];
    constraint->pos[2] = params->origin[2];
    constraint->pos2[0] = params->origin[0];
    constraint->pos2[1] = params->origin[1];
    constraint->pos2[2] = params->origin[2];
    constraint->dir[0] = params->angles[0];
    constraint->dir[1] = params->angles[1];
    constraint->dir[2] = params->angles[2];
    constraint->min_health = params->health;
    constraint->distance = params->distance;
    constraint->scale[0] = params->force_scale;
    constraint->scale[1] = params->scale;
    constraint->timeout = (int)(float)(params->timeout * 1000.0);
    if ( constraint->type == CONSTRAINT_ROPE )
    {
        for ( j = 0; j < num_infos; ++j )
        {
            info_node = &info_params[j];
            if ( &info_node->targetname[strlen(info_node->targetname) + 1] != &info_node->targetname[1] )
            {
                if ( I_stricmp(info_node->targetname, params->target) )
                {
                    if ( !I_stricmp(info_node->targetname, params->target2) )
                    {
                        constraint->pos2[0] = info_node->origin[0];
                        constraint->pos2[1] = info_node->origin[1];
                        constraint->pos2[2] = info_node->origin[2];
                    }
                }
                else
                {
                    constraint->pos[0] = info_node->origin[0];
                    constraint->pos[1] = info_node->origin[1];
                    constraint->pos[2] = info_node->origin[2];
                }
            }
        }
    }
    return 1;
}

bool __cdecl FindConstraints(
                const char *targetname,
                const float *origin,
                const float *angles,
                AttachPointType attach_type,
                DynEntityConstraintCreateParams *constraintParams,
                int numConstraints,
                int *outConstraints)
{
    node_params_t *info_node; // [esp+4h] [ebp-48h]
    int j; // [esp+8h] [ebp-44h]
    PhysConstraint *constraint; // [esp+Ch] [ebp-40h]
    int i; // [esp+10h] [ebp-3Ch]
    float xform[4][3]; // [esp+14h] [ebp-38h] BYREF
    unsigned __int16 targetname_hash; // [esp+44h] [ebp-8h]
    int count; // [esp+48h] [ebp-4h]

    if ( !*targetname )
        return 0;
    AnglesToAxis(angles, xform);
    xform[3][0] = *origin;
    xform[3][1] = origin[1];
    xform[3][2] = origin[2];
    count = 0;
    targetname_hash = SL_GetLowercaseString(targetname, 0, SCRIPTINSTANCE_SERVER);
    for ( i = 0; i < numConstraints && count < 4; ++i )
    {
        constraint = &cm.constraints[i];
        if ( !I_stricmp(targetname, constraintParams[i].target2) )
        {
            constraint->attach_point_type2 = attach_type;
            constraint->target_ent2 = targetname_hash;
            MatrixTransposeTransformVector43(constraintParams[i].origin, xform, constraint->pos2);
            if ( count >= 4
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                            636,
                            0,
                            "%s",
                            "count < DYNENT_MAX_CONSTRAINTS") )
            {
                __debugbreak();
            }
            if ( outConstraints )
                outConstraints[count++] = i;
        }
        if ( !I_stricmp(targetname, constraintParams[i].target) )
        {
            constraint->attach_point_type1 = attach_type;
            constraint->target_ent1 = targetname_hash;
            MatrixTransposeTransformVector43(constraintParams[i].origin, xform, constraint->pos);
            if ( count >= 4
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                            644,
                            0,
                            "%s",
                            "count < DYNENT_MAX_CONSTRAINTS") )
            {
                __debugbreak();
            }
            if ( outConstraints )
                outConstraints[count++] = i;
        }
        for ( j = 0; j < num_infos && count < 4; ++j )
        {
            info_node = &info_params[j];
            if ( !I_stricmp(info_node->target, targetname) )
            {
                if ( !I_stricmp(info_node->targetname, constraintParams[i].target2) )
                {
                    constraint->attach_point_type2 = attach_type;
                    constraint->target_ent2 = targetname_hash;
                    MatrixTransposeTransformVector43(info_node->origin, xform, constraint->pos2);
                    if ( count >= 4
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                                    662,
                                    0,
                                    "%s",
                                    "count < DYNENT_MAX_CONSTRAINTS") )
                    {
                        __debugbreak();
                    }
                    if ( outConstraints )
                        outConstraints[count++] = i;
                }
                if ( !I_stricmp(info_node->targetname, constraintParams[i].target) )
                {
                    constraint->attach_point_type1 = attach_type;
                    constraint->target_ent1 = targetname_hash;
                    MatrixTransposeTransformVector43(info_node->origin, xform, constraint->pos);
                    if ( count >= 4
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                                    670,
                                    0,
                                    "%s",
                                    "count < DYNENT_MAX_CONSTRAINTS") )
                    {
                        __debugbreak();
                    }
                    if ( outConstraints )
                        outConstraints[count++] = i;
                }
            }
        }
    }
    return count > 0;
}

char __cdecl DynEnt_Create(
                DynEntityDef *dynEntDef,
                const DynEntityCreateParams *params,
                DynEntityConstraintCreateParams *constraintParams,
                int numConstraints)
{
    int j; // [esp+20h] [ebp-28h]
    int i; // [esp+24h] [ebp-24h]
    PhysPreset *preset; // [esp+28h] [ebp-20h]
    unsigned int brushModel; // [esp+34h] [ebp-14h]
    int brushModela; // [esp+34h] [ebp-14h]
    int constraints[4]; // [esp+38h] [ebp-10h] BYREF

    if ( !dynEntDef
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    746,
                    0,
                    "%s",
                    "dynEntDef") )
    {
        __debugbreak();
    }
    if ( !params
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp", 747, 0, "%s", "params") )
    {
        __debugbreak();
    }
    if ( ((LODWORD(params->origin[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(params->origin[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(params->origin[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    749,
                    0,
                    "%s",
                    "!IS_NAN((params->origin)[0]) && !IS_NAN((params->origin)[1]) && !IS_NAN((params->origin)[2])") )
    {
        __debugbreak();
    }
    if ( ((LODWORD(params->angles[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(params->angles[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(params->angles[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    750,
                    0,
                    "%s",
                    "!IS_NAN((params->angles)[0]) && !IS_NAN((params->angles)[1]) && !IS_NAN((params->angles)[2])") )
    {
        __debugbreak();
    }
    Com_Memset((unsigned int *)dynEntDef, 0, 84);
    if ( params->typeName[0] )
    {
        dynEntDef->type = (DynEntityType)DynEnt_GetType(params->typeName);
        if ( dynEntDef->type == DYNENT_TYPE_INVALID )
        {
            Com_Error(ERR_DROP, "Invalid Dyn Entity type [%s]\n", params->typeName);
            return 0;
        }
    }
    else
    {
        dynEntDef->type = DYNENT_TYPE_CLUTTER;
    }
    if ( params->modelName[0] == 42 )
    {
        brushModel = atoi(&params->modelName[1]);
        dynEntDef->brushModel = brushModel;
        if ( dynEntDef->brushModel != brushModel
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                        777,
                        0,
                        "%s",
                        "dynEntDef->brushModel == brushModel") )
        {
            __debugbreak();
        }
        if ( brushModel >= cm.numSubModels
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                        779,
                        0,
                        "brushModel doesn't index cm.numSubModels\n\t%i not in [0, %i)",
                        brushModel,
                        cm.numSubModels) )
        {
            __debugbreak();
        }
        dynEntDef->contents = cm.cmodels[brushModel].leaf.terrainContents | cm.cmodels[brushModel].leaf.brushContents;
    }
    else
    {
        dynEntDef->xModel = DynEnt_XModelPrecache((char*)params->modelName);
        if ( !dynEntDef->xModel )
        {
            Com_Error(ERR_DROP, "Couldn't find xmodel [%s] for Dyn Entity.\n", params->modelName);
            return 0;
        }
        dynEntDef->contents = XModelGetContents(dynEntDef->xModel);
        if ( dynEntDef->xModel->collLod < 0 )
        {
            Com_Error(ERR_DROP, "Dyn Entity xmodel [%s] has no collision LOD set.\n", params->modelName);
            return 0;
        }
    }
    if ( params->physModelName[0] == 42 )
    {
        brushModela = atoi(&params->physModelName[1]);
        dynEntDef->physicsBrushModel = brushModela;
        if ( dynEntDef->physicsBrushModel != brushModela
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                        807,
                        0,
                        "%s",
                        "dynEntDef->physicsBrushModel == brushModel") )
        {
            __debugbreak();
        }
    }
    if ( params->destroyFxFile[0] )
    {
        dynEntDef->destroyFx = FX_Register((char*)params->destroyFxFile);
        if ( !dynEntDef->destroyFx )
        {
            Com_Error(ERR_DROP, "Couldn't find fx [%s] for Dyn Entity.\n", params->destroyFxFile);
            return 0;
        }
    }
    if ( params->destroySound[0] )
    {
        dynEntDef->destroySound = SND_HashName(params->destroySound);
        if ( !dynEntDef->destroySound )
        {
            Com_Error(ERR_DROP, "Couldn't find destroy sound [%s] for Dyn Entity.\n", params->destroySound);
            return 0;
        }
    }
    if ( !params->destroyedModelName[0] )
        goto LABEL_89;
    dynEntDef->destroyedxModel = DynEnt_XModelPrecache((char*)params->destroyedModelName);
    if ( !dynEntDef->destroyedxModel )
    {
        Com_Error(ERR_DROP, "Couldn't find destroyed xmodel [%s] for Dyn Entity.\n", params->destroyedModelName);
        return 0;
    }
    dynEntDef->contents = XModelGetContents(dynEntDef->destroyedxModel);
    if ( dynEntDef->destroyedxModel->collLod < 0 )
    {
        Com_Error(ERR_DROP, "Dyn Entity destroyed xmodel [%s] has no collision LOD set.\n", params->destroyedModelName);
        return 0;
    }
    else
    {
LABEL_89:
        if ( !params->destroyPiecesFile[0]
            || (dynEntDef->destroyPieces = DynEnt_XModelPiecesPrecache(params->destroyPiecesFile)) != 0 )
        {
            dynEntDef->physPreset = 0;
            if ( DynEnt_GetEntityProps(dynEntDef->type)->usePhysics )
            {
                if ( dynEntDef->xModel && dynEntDef->xModel->physPreset )
                    dynEntDef->physPreset = dynEntDef->xModel->physPreset;
                if ( dynEntDef->destroyedxModel && dynEntDef->destroyedxModel->physPreset )
                    dynEntDef->physPreset = dynEntDef->destroyedxModel->physPreset;
                if ( params->physPresetFile[0] )
                    dynEntDef->physPreset = DynEnt_PhysPresetPrecache(params->physPresetFile);
                if ( !dynEntDef->physPreset )
                {
                    preset = DynEnt_AutoCreatePhysPreset(dynEntDef, params);
                    if ( !preset
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                                    877,
                                    0,
                                    "%s",
                                    "preset") )
                    {
                        __debugbreak();
                    }
                    if ( preset )
                        dynEntDef->physPreset = preset;
                }
            }
            dynEntDef->targetname = SL_GetLowercaseString(params->targetname, 0, SCRIPTINSTANCE_SERVER);
            dynEntDef->target = SL_GetLowercaseString(params->target, 0, SCRIPTINSTANCE_SERVER);
            dynEntDef->pose.origin[0] = params->origin[0];
            dynEntDef->pose.origin[1] = params->origin[1];
            dynEntDef->pose.origin[2] = params->origin[2];
            AnglesToQuat(params->angles, dynEntDef->pose.quat);
            dynEntDef->health = params->health;
            if ( dynEntDef->destroyedxModel && !dynEntDef->health )
                dynEntDef->health = 1;
            if ( params->startInPhysics )
                dynEntDef->flags |= 1u;
            if ( params->autoActivate )
                dynEntDef->flags |= 2u;
            if ( params->noBulletDamage )
                dynEntDef->flags |= 4u;
            if ( params->noExplosiveDamage )
                dynEntDef->flags |= 8u;
            if ( params->alwaysFloats )
                dynEntDef->flags |= 0x10u;
            for ( i = 0; i < 4; ++i )
                constraints[i] = 511;
            FindConstraints(
                params->targetname,
                params->origin,
                params->angles,
                ATTACH_POINT_DYNENT,
                constraintParams,
                numConstraints,
                constraints);
            for ( j = 0; j < 4; ++j )
                dynEntDef->physConstraints[j] = constraints[j];
            return 1;
        }
        else
        {
            Com_Error(ERR_DROP, "Couldn't find pieces [%s] for Dyn Entity.\n", params->destroyPiecesFile);
            return 0;
        }
    }
}

int __cdecl DynEnt_GetType(const char *typeName)
{
    int type; // [esp+0h] [ebp-4h]

    if ( !typeName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    199,
                    0,
                    "%s",
                    "typeName") )
    {
        __debugbreak();
    }
    for ( type = 0; type < 3; ++type )
    {
        if ( !I_stricmp(typeName, dynEntProps[type].name) )
            return type;
    }
    return 0;
}

XModel *__cdecl DynEnt_XModelPrecache(char *modelName)
{
    if ( !modelName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    581,
                    0,
                    "%s",
                    "modelName") )
    {
        __debugbreak();
    }
    return XModelPrecache(
                     modelName,
                     (void *(__cdecl *)(int))DynEnt_AllocXModel,
                     (void *(__cdecl *)(int))DynEnt_AllocXModelColl);
}

unsigned __int8 *__cdecl DynEnt_AllocXModel(int size)
{
    if ( size <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    569,
                    0,
                    "%s",
                    "size > 0") )
    {
        __debugbreak();
    }
    return Hunk_Alloc(size, "DynEnt_AllocXModel", 22);
}

unsigned __int8 *__cdecl DynEnt_AllocXModelColl(int size)
{
    if ( size <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    575,
                    0,
                    "%s",
                    "size > 0") )
    {
        __debugbreak();
    }
    return Hunk_Alloc(size, "DynEnt_AllocXModelColl", 28);
}

PhysPreset *__cdecl DynEnt_AutoCreatePhysPreset(const DynEntityDef *dynEntDef, const DynEntityCreateParams *params)
{
    char name[32]; // [esp+0h] [ebp-2Ch] BYREF
    int i; // [esp+24h] [ebp-8h]
    PhysPreset *physPreset; // [esp+28h] [ebp-4h]

    if ( dynEntDef->xModel )
    {
        for ( i = 0; i < numAutoPhysPresets; ++i )
        {
            physPreset = autoPhysPresets[i];
            if ( physPreset->name && !I_stricmp(physPreset->name, dynEntDef->xModel->name) )
                return physPreset;
        }
    }
    physPreset = (PhysPreset *)Hunk_Alloc(0x54u, "DynEnt_AllocPhysPreset", 22);
    memset(physPreset, 0, sizeof(PhysPreset));
    if ( dynEntDef->xModel )
    {
        DynEnt_Strcpy((char **)physPreset, dynEntDef->xModel->name);
    }
    else
    {
        sprintf(name, "%d", numAutoPhysPresets + 1);
        DynEnt_Strcpy((char **)physPreset, name);
    }
    if ( params->mass > 0.0 )
        physPreset->mass = params->mass;
    if ( params->bounce > 0.0 )
        physPreset->bounce = params->bounce;
    if ( params->friction > 0.0 )
        physPreset->friction = params->friction;
    physPreset->flags |= 1u;
    if ( numAutoPhysPresets < 512 )
        autoPhysPresets[numAutoPhysPresets++] = physPreset;
    return physPreset;
}

void __cdecl DynEnt_Strcpy(char **memberString, const char *keyValue)
{
    char v2; // [esp+3h] [ebp-21h]
    char *v3; // [esp+8h] [ebp-1Ch]
    const char *v4; // [esp+Ch] [ebp-18h]
    char *buf; // [esp+20h] [ebp-4h]

    buf = (char *)Hunk_AllocLowAlign(strlen(keyValue) + 1, 1, "DynEnt_AllocPhysPreset", 0);
    v4 = keyValue;
    v3 = buf;
    do
    {
        v2 = *v4;
        *v3++ = *v4++;
    }
    while ( v2 );
    *memberString = buf;
}

int __cdecl DynEnt_CompareEntities(unsigned int *arg0, unsigned int *arg1)
{
    int hasModel0; // [esp+8h] [ebp-14h]
    int value1; // [esp+10h] [ebp-Ch]
    int hasModel1; // [esp+14h] [ebp-8h]
    int value0; // [esp+18h] [ebp-4h]

    if ( !arg0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp", 928, 0, "%s", "arg0") )
    {
        __debugbreak();
    }
    if ( !arg1
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp", 929, 0, "%s", "arg1") )
    {
        __debugbreak();
    }
    if ( *arg0 >= 3u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    934,
                    0,
                    "dynEntDef0->type doesn't index DYNENT_TYPE_COUNT\n\t%i not in [0, %i)",
                    *arg0,
                    3) )
    {
        __debugbreak();
    }
    if ( *arg1 >= 3u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    935,
                    0,
                    "dynEntDef1->type doesn't index DYNENT_TYPE_COUNT\n\t%i not in [0, %i)",
                    *arg1,
                    3) )
    {
        __debugbreak();
    }
    hasModel0 = arg0[8] != 0;
    hasModel1 = arg1[8] != 0;
    if ( hasModel0 != hasModel1 )
        return hasModel0 - hasModel1;
    value0 = dynEntProps[*arg0].clientOnly;
    value1 = dynEntProps[*arg1].clientOnly;
    if ( value0 != value1 )
        return value0 - value1;
    if ( arg0[8] )
    {
        if ( !arg1[8]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                        954,
                        0,
                        "%s",
                        "dynEntDef1->xModel") )
        {
            __debugbreak();
        }
        return arg0[8] < arg1[8] ? -1 : 1;
    }
    else
    {
        if ( arg1[8]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                        959,
                        0,
                        "%s",
                        "!dynEntDef1->xModel") )
        {
            __debugbreak();
        }
        return *((unsigned __int16 *)arg0 + 20) - *((unsigned __int16 *)arg1 + 20);
    }
}

unsigned __int8 *__cdecl DynEnt_Alloc(int count, int size)
{
    unsigned __int8 *buf; // [esp+0h] [ebp-4h]

    if ( count <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    967,
                    0,
                    "%s",
                    "count > 0") )
    {
        __debugbreak();
    }
    if ( size <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    968,
                    0,
                    "%s",
                    "size > 0") )
    {
        __debugbreak();
    }
    buf = Hunk_Alloc(size * count, "DynEnt_LoadEntities", 10);
    Com_Memset((unsigned int *)buf, 0, size * count);
    return buf;
}

void __cdecl GetAttachPointsInWorldSpace(PhysConstraint *constraint, float *p0, float *p1)
{
    AttachPointType attach_point_type2; // [esp+0h] [ebp-70h]
    AttachPointType attach_point_type1; // [esp+4h] [ebp-6Ch]
    float *v5; // [esp+Ch] [ebp-64h]
    float *v6; // [esp+14h] [ebp-5Ch]
    float *origin; // [esp+20h] [ebp-50h]
    float *v8; // [esp+28h] [ebp-48h]
    int k; // [esp+30h] [ebp-40h]
    int m; // [esp+34h] [ebp-3Ch]
    int j; // [esp+38h] [ebp-38h]
    int i; // [esp+3Ch] [ebp-34h]
    float xform[4][3]; // [esp+40h] [ebp-30h] BYREF

    attach_point_type1 = constraint->attach_point_type1;
    if ( attach_point_type1 )
    {
        if ( attach_point_type1 == ATTACH_POINT_DYNENT )
        {
            for ( j = 0; j < num_dents; ++j )
            {
                if ( constraint->target_ent1 == dents_params[j].targetname_hash )
                {
                    AnglesToAxis(dents_params[j].angles, xform);
                    origin = dents_params[j].origin;
                    xform[3][0] = *origin;
                    xform[3][1] = origin[1];
                    xform[3][2] = origin[2];
                    MatrixTransformVector43(constraint->pos, xform, p0);
                }
            }
        }
        else if ( attach_point_type1 == ATTACH_POINT_ENT )
        {
            for ( i = 0; i < num_ents; ++i )
            {
                if ( constraint->target_ent1 == ents_params[i].targetname_hash )
                {
                    AnglesToAxis(ents_params[i].angles, xform);
                    v8 = ents_params[i].origin;
                    xform[3][0] = *v8;
                    xform[3][1] = v8[1];
                    xform[3][2] = v8[2];
                    MatrixTransformVector43(constraint->pos, xform, p0);
                }
            }
        }
    }
    else
    {
        *p0 = constraint->pos[0];
        p0[1] = constraint->pos[1];
        p0[2] = constraint->pos[2];
    }
    attach_point_type2 = constraint->attach_point_type2;
    if ( attach_point_type2 )
    {
        if ( attach_point_type2 == ATTACH_POINT_DYNENT )
        {
            for ( k = 0; k < num_dents; ++k )
            {
                if ( constraint->target_ent2 == dents_params[k].targetname_hash )
                {
                    AnglesToAxis(dents_params[k].angles, xform);
                    v5 = dents_params[k].origin;
                    xform[3][0] = *v5;
                    xform[3][1] = v5[1];
                    xform[3][2] = v5[2];
                    MatrixTransformVector43(constraint->pos2, xform, p1);
                }
            }
        }
        else if ( attach_point_type2 == ATTACH_POINT_ENT )
        {
            for ( m = 0; m < num_ents; ++m )
            {
                if ( constraint->target_ent2 == ents_params[m].targetname_hash )
                {
                    AnglesToAxis(ents_params[m].angles, xform);
                    v6 = ents_params[m].origin;
                    xform[3][0] = *v6;
                    xform[3][1] = v6[1];
                    xform[3][2] = v6[2];
                    MatrixTransformVector43(constraint->pos2, xform, p1);
                }
            }
        }
    }
    else
    {
        *p1 = constraint->pos2[0];
        p1[1] = constraint->pos2[1];
        p1[2] = constraint->pos2[2];
    }
}

const DynEntityProps *__cdecl DynEnt_GetEntityProps(DynEntityType dynEntType)
{
    if ( dynEntType == DYNENT_TYPE_INVALID
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    1380,
                    0,
                    "%s",
                    "dynEntType != DYNENT_TYPE_INVALID") )
    {
        __debugbreak();
    }
    if ( (unsigned int)dynEntType >= DYNENT_TYPE_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    1381,
                    0,
                    "dynEntType doesn't index DYNENT_TYPE_COUNT\n\t%i not in [0, %i)",
                    dynEntType,
                    3) )
    {
        __debugbreak();
    }
    //return (const DynEntityProps *)(8 * dynEntType + 13277220);
    return &dynEntProps[dynEntType];
}

unsigned __int16 __cdecl DynEnt_GetId(const DynEntityDef *dynEntDef, DynEntityDrawType drawType)
{
    unsigned __int16 dynEntId; // [esp+0h] [ebp-4h]

    iassert(dynEntDef);

    dynEntId = dynEntDef - cm.dynEntDefList[drawType];

    bcassert(dynEntId, (unsigned int)cm.dynEntCount[drawType]);
    //if ( dynEntId >= (unsigned int)cm.dynEntCount[drawType]
    //    && !Assert_MyHandler(
    //                "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
    //                1394,
    //                1,
    //                "dynEntId doesn't index cm.dynEntCount[DynEntGetClientCollType( drawType )]\n\t%i not in [0, %i)",
    //                dynEntId,
    //                cm.dynEntCount[drawType]) )
    //{
    //    __debugbreak();
    //}
    return dynEntId;
}

unsigned __int16 __cdecl DynEnt_GetAbsId(const DynEntityDef *dynEntDef)
{
    unsigned __int16 absId; // [esp+0h] [ebp-4h]

    if ( !dynEntDef
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    1403,
                    0,
                    "%s",
                    "dynEntDef") )
    {
        __debugbreak();
    }
    absId = dynEntDef - cm.dynEntDefList[0];
    if ( absId >= 0x1000u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    1407,
                    1,
                    "absId doesn't index DYNENT_MAX_COUNT\n\t%i not in [0, %i)",
                    absId,
                    4096) )
    {
        __debugbreak();
    }
    return absId;
}

bool __cdecl DynEnt_Valid(unsigned __int16 absDynEntId)
{
    return absDynEntId >= cm.originalDynEntCount + 256 || absDynEntId < (int)cm.dynEntCount[0];
}

unsigned __int16 __cdecl DynEnt_GetEntityCount(DynEntityCollType collType)
{
    return cm.dynEntCount[collType];
}

const DynEntityDef *__cdecl DynEnt_GetEntityDef(unsigned __int16 absDynEntId)
{
    DynEntityDrawType drawType; // [esp+0h] [ebp-8h] BYREF
    unsigned __int16 id; // [esp+4h] [ebp-4h] BYREF

    DynEnt_GetClientIdDrawType(absDynEntId, &id, &drawType);
    return DynEnt_GetEntityDef(id, drawType);
}

const DynEntityPose *__cdecl DynEnt_GetClientPose(unsigned __int16 absDynEntId)
{
    DynEntityDrawType drawType; // [esp+0h] [ebp-8h] BYREF
    unsigned __int16 id; // [esp+4h] [ebp-4h] BYREF

    DynEnt_GetClientIdDrawType(absDynEntId, &id, &drawType);
    return DynEnt_GetClientPose(id, drawType);
}

DynEntityPose *__cdecl DynEnt_GetClientModelPoseList()
{
    return cm.dynEntPoseList[0];
}

DynEntityPose *__cdecl DynEnt_GetClientPose(unsigned __int16 dynEntId, DynEntityDrawType drawType)
{
    if ( dynEntId >= (unsigned int)cm.dynEntCount[drawType]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    1465,
                    0,
                    "dynEntId doesn't index cm.dynEntCount[DynEntGetClientCollType( drawType )]\n\t%i not in [0, %i)",
                    dynEntId,
                    cm.dynEntCount[drawType]) )
    {
        __debugbreak();
    }
    return &cm.dynEntPoseList[drawType][dynEntId];
}

DynEntityServer *__cdecl DynEnt_GetServerEntity(unsigned __int16 dynEntId, DynEntityDrawType drawType)
{
    if ( dynEntId >= (unsigned int)cm.dynEntCount[drawType + 2]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    1473,
                    0,
                    "dynEntId doesn't index cm.dynEntCount[DynEntGetServerCollType( drawType )]\n\t%i not in [0, %i)",
                    dynEntId,
                    cm.dynEntCount[drawType + 2]) )
    {
        __debugbreak();
    }
    return &cm.dynEntServerList[drawType][dynEntId];
}

void __cdecl DynEnt_GetClientIdDrawType(unsigned __int16 absId, unsigned __int16 *outId, DynEntityDrawType *drawType)
{
    unsigned __int16 brushid; // [esp+0h] [ebp-4h]

    if ( absId >= cm.originalDynEntCount + 256 )
    {
        brushid = absId - cm.originalDynEntCount - 256;
        if ( &cm.dynEntClientList[0][absId] != &cm.dynEntClientList[1][brushid]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                        1488,
                        0,
                        "%s",
                        "&cm.dynEntClientList[DYNENT_DRAW_MODEL][absId] == &cm.dynEntClientList[DYNENT_DRAW_BRUSH][brushid]") )
        {
            __debugbreak();
        }
        *outId = brushid;
        *drawType = DYNENT_DRAW_BRUSH;
    }
    else
    {
        *outId = absId;
        *drawType = DYNENT_DRAW_MODEL;
    }
}

DynEntityClient *__cdecl DynEnt_GetClientEntity(unsigned __int16 id)
{
    unsigned __int16 brushid; // [esp+0h] [ebp-4h]

    if ( id < cm.originalDynEntCount + 256 )
        return &cm.dynEntClientList[0][id];
    brushid = id - cm.originalDynEntCount - 256;
    if ( &cm.dynEntClientList[0][id] != &cm.dynEntClientList[1][brushid]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    1503,
                    0,
                    "%s",
                    "&cm.dynEntClientList[DYNENT_DRAW_MODEL][id] == &cm.dynEntClientList[DYNENT_DRAW_BRUSH][brushid]") )
    {
        __debugbreak();
    }
    return &cm.dynEntClientList[1][brushid];
}

unsigned __int16 __cdecl DynEnt_GetClientAbsId(unsigned __int16 dynEntId, DynEntityDrawType drawType)
{
    if ( drawType )
        return cm.originalDynEntCount + dynEntId + 256;
    else
        return dynEntId;
}

int __cdecl DynEnt_GetXModelUsageCount(const XModel *xModel)
{
    unsigned int drawType; // [esp+0h] [ebp-Ch]
    unsigned __int16 dynEntId; // [esp+4h] [ebp-8h]
    int count; // [esp+8h] [ebp-4h]

    if ( !xModel
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_load_obj.cpp",
                    1893,
                    0,
                    "%s",
                    "xModel") )
    {
        __debugbreak();
    }
    count = 0;
    for ( drawType = 0; drawType < 2; ++drawType )
    {
        for ( dynEntId = 0; dynEntId < (int)cm.dynEntCount[drawType]; ++dynEntId )
        {
            if (cm.dynEntDefList[drawType][dynEntId].xModel == xModel)
                ++count;
        }
    }
    return count;
}

