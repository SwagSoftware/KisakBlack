#include "g_spawn_mp.h"
#include <game/g_load_utils.h>
#include <clientscript/cscr_vm.h>
#include "g_utils_mp.h"
#include <clientscript/cscr_stringlist.h>
#include <server_mp/sv_init_mp.h>
#include <game/pathnode_load_obj.h>
#include "actor_mp.h"
#include <clientscript/scr_const.h>
#include <game/g_mover.h>
#include "g_trigger_mp.h"
#include "g_misc_mp.h"
#include "game/turret.h"
#include <game/g_scr_mover.h>
#include <bgame/bg_misc.h>
#include <game/g_weapon.h>
#include <game/g_client_fields.h>
#include <cgame/cg_hudelem.h>
#include <bgame/bg_weapons_def.h>

struct ent_field_t // sizeof=0x14
{                                       // XREF: .rdata:fields_1/r
    const char *name;
    int ofs;
    int size[1];
    fieldtype_t type;
    void (__cdecl *callback)(gentity_s *, int);
};

const ent_field_t fields_1[16] =
{
  { "classname", 356, { 2 }, F_STRING, Scr_ReadOnlyField },
  { "origin", 292, { 12 }, F_VECTOR, Scr_SetOrigin },
  { "model", 348, { 2 }, F_MODEL, Scr_ReadOnlyField },
  { "spawnflags", 368, { 4 }, F_INT, Scr_ReadOnlyField },
  { "target", 358, { 2 }, F_STRING, NULL },
  { "targetname", 360, { 2 }, F_STRING, NULL },
  { "script_noteworthy", 362, { 2 }, F_STRING, NULL },
  { "count", 532, { 4 }, F_INT, NULL },
  { "health", 404, { 4 }, F_INT, Scr_SetHealth },
  { "dmg", 412, { 4 }, F_INT, NULL },
  { "angles", 304, { 12 }, F_VECTOR, Scr_SetAngles },
  { "birthtime", 748, { 4 }, F_INT, Scr_ReadOnlyField },
  { "index", 560, { 4 }, F_INT, Scr_SetExposureIndex },
  { "lerp_to_lighter", 564, { 4 }, F_FLOAT, Scr_SetExposureLerpToLighter },
  { "lerp_to_darker", 568, { 4 }, F_FLOAT, Scr_SetExposureLerpToDarker },
  { NULL, 0, { 0 }, F_INT, NULL }
};

const SpawnFuncEntry s_bspOnlySpawns[15] =
{
  { "trigger_use", trigger_use_touch },
  { "trigger_use_touch", trigger_use_touch },
  { "trigger_multiple", SP_trigger_multiple },
  { "trigger_disk", SP_trigger_disk },
  { "trigger_hurt", SP_trigger_hurt },
  { "trigger_once", SP_trigger_once },
  { "trigger_damage", SP_trigger_damage },
  { "trigger_lookat", SP_trigger_lookat },
  { "trigger_ik_playerclip_terrain", SP_trigger_ik_playerclip_terrain },
  { "light", SP_light },
  { "misc_mg42", SP_turret },
  { "misc_turret", SP_turret },
  { "script_brushmodel", SP_script_brushmodel },
  { "script_struct", G_FreeEntityWrapper },
  { "script_vehicle", SP_script_vehicle }
};

const SpawnFuncEntry s_bspOrDynamicSpawns[7] =
{
  { "info_notnull", SP_info_notnull },
  { "info_notnull_big", SP_info_notnull },
  { "trigger_radius", SP_trigger_radius },
  { "trigger_radius_use", SP_trigger_radius_use },
  { "script_model", SP_script_model },
  { "script_origin", SP_script_origin },
  { "script_vehicle_collmap", SP_script_vehicle_collmap }
};




int __cdecl G_SpawnFloat(const SpawnVar *spawnVar, const char *key, const char *defaultString, float *out)
{
    int present; // [esp+0h] [ebp-8h]
    const char *s; // [esp+4h] [ebp-4h] BYREF

    present = G_SpawnString(spawnVar, key, defaultString, &s);
    *out = atof(s);
    return present;
}

int __cdecl G_SpawnInt(const SpawnVar *spawnVar, const char *key, const char *defaultString, int *out)
{
    int present; // [esp+0h] [ebp-8h]
    const char *s; // [esp+4h] [ebp-4h] BYREF

    present = G_SpawnString(spawnVar, key, defaultString, &s);
    *out = atoi(s);
    return present;
}

void __cdecl Scr_ReadOnlyField(gentity_s *ent, int)
{
    Scr_Error("Tried to set a read only entity field", 0);
}

void __cdecl G_FreeEntityWrapper(gentity_s *ent, SpawnVar *v)
{
    G_FreeEntity(ent);
}

unsigned int __cdecl G_SetEntityScriptVariableInternal(const char *key, char *value)
{
    int v3; // eax
    const char *v4; // eax
    float v5; // [esp+8h] [ebp-1Ch]
    unsigned int index; // [esp+10h] [ebp-14h]
    int type; // [esp+14h] [ebp-10h] BYREF
    float vec[3]; // [esp+18h] [ebp-Ch] BYREF

    index = Scr_FindField(key, &type, SCRIPTINSTANCE_SERVER);
    if ( !index )
        return 0;
    switch ( type )
    {
        case 2:
            Scr_AddString(value, SCRIPTINSTANCE_SERVER);
            break;
        case 4:
            memset(vec, 0, sizeof(vec));
            sscanf(value, "%f %f %f", vec, &vec[1], &vec[2]);
            Scr_AddVector(vec, SCRIPTINSTANCE_SERVER);
            break;
        case 5:
            v5 = atof(value);
            Scr_AddFloat(v5, SCRIPTINSTANCE_SERVER);
            break;
        case 6:
            v3 = atoi(value);
            Scr_AddInt(v3, SCRIPTINSTANCE_SERVER);
            break;
        default:
            v4 = va("G_SetEntityScriptVariableInternal: bad case %d", type);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 210, 0, v4) )
                __debugbreak();
            break;
    }
    return index;
}

void __cdecl G_ParseEntityFields(const SpawnVar *spawnVar, gentity_s *ent, int radiant_update)
{
    int i; // [esp+0h] [ebp-4h]

    if ( !spawnVar->spawnVarsValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    316,
                    0,
                    "%s",
                    "spawnVar.spawnVarsValid") )
    {
        __debugbreak();
    }
    for ( i = 0; i < spawnVar->numSpawnVars; ++i )
        G_ParseEntityField(spawnVar->spawnVars[i][0], spawnVar->spawnVars[i][1], ent, radiant_update);
    G_SetOrigin(ent, ent->r.currentOrigin);
    G_SetAngle(ent, ent->r.currentAngles);
}

void __cdecl G_ParseEntityField(const char *key, char *value, gentity_s *ent, int radiant_update)
{
    float v4; // [esp+0h] [ebp-20h]
    const ent_field_t *f; // [esp+Ch] [ebp-14h]
    float vec[3]; // [esp+10h] [ebp-10h] BYREF
    int modelIndex; // [esp+1Ch] [ebp-4h]

    for ( f = fields_1; ; ++f )
    {
        if ( !f->name )
        {
            G_SetEntityScriptVariable(key, value, ent);
            return;
        }
        if ( !I_stricmp(f->name, key) )
            break;
    }
    switch ( f->type )
    {
        case F_INT:
            *(int *)((char *)&ent->s.number + f->ofs) = atoi(value);
            break;
        case F_FLOAT:
            v4 = atof(value);
            *(float *)((char *)&ent->s.number + f->ofs) = v4;
            break;
        case F_STRING:
            Scr_SetStringLiveUpdateSafe((unsigned __int16 *)((char *)ent + f->ofs), value, SCRIPTINSTANCE_SERVER);
            break;
        case F_VECTOR:
            memset(vec, 0, sizeof(vec));
            sscanf(value, "%f %f %f", vec, &vec[1], &vec[2]);
            *(float *)((char *)&ent->s.number + f->ofs) = vec[0];
            *(float *)((char *)&ent->s.lerp.eFlags + f->ofs) = vec[1];
            *(float *)((char *)&ent->s.lerp.eFlags2 + f->ofs) = vec[2];
            break;
        case F_MODEL:
            if ( *value == 42 )
            {
                modelIndex = atoi(value + 1);
                if ( modelIndex != (unsigned __int16)modelIndex
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                                282,
                                0,
                                "%s",
                                "modelIndex == (modelNameIndex_t)modelIndex") )
                {
                    __debugbreak();
                }
                ent->s.index.brushmodel = modelIndex;
            }
            else
            {
                if ( Com_IsLegacyXModelName(value) )
                    value += 7;
                if ( radiant_update )
                    G_SetModelIfLoaded(ent, value);
                else
                    G_SetModel(ent, value);
            }
            break;
        default:
            return;
    }
}

void __cdecl G_SetEntityScriptVariable(const char *key, char *value, gentity_s *ent)
{
    unsigned int index; // [esp+0h] [ebp-4h]

    index = G_SetEntityScriptVariableInternal(key, value);
    if ( index )
        GScr_SetDynamicEntityField(ent, index);
}

unsigned int __cdecl G_SpawnStruct(SpawnVar *spawnVar)
{
    unsigned int index; // [esp+0h] [ebp-10h]
    int i; // [esp+4h] [ebp-Ch]
    const char *scriptLabel; // [esp+8h] [ebp-8h] BYREF
    unsigned int structId; // [esp+Ch] [ebp-4h]

    if ( !spawnVar->spawnVarsValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    339,
                    0,
                    "%s",
                    "spawnVar.spawnVarsValid") )
    {
        __debugbreak();
    }
    G_SpawnString(spawnVar, "script_label", "", &scriptLabel);
    if ( !I_stricmp(scriptLabel, "random") || !I_stricmp(scriptLabel, "looper") || !I_stricmp(scriptLabel, "line_emitter") )
        return 0;
    if ( !g_scr_data.createstruct
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    348,
                    0,
                    "%s",
                    "g_scr_data.createstruct") )
    {
        __debugbreak();
    }
    Scr_AddExecThread(SCRIPTINSTANCE_SERVER, g_scr_data.createstruct, 0);
    structId = Scr_GetObject(0, SCRIPTINSTANCE_SERVER);
    for ( i = 0; i < spawnVar->numSpawnVars; ++i )
    {
        index = G_SetEntityScriptVariableInternal(spawnVar->spawnVars[i][0], spawnVar->spawnVars[i][1]);
        if ( index )
            Scr_SetStructField(structId, index, SCRIPTINSTANCE_SERVER);
    }
    return structId;
}

void __cdecl G_DuplicateEntityFields(gentity_s *dest, const gentity_s *source)
{
    float *v2; // [esp+8h] [ebp-14h]
    float *v3; // [esp+Ch] [ebp-10h]
    const ent_field_t *f; // [esp+14h] [ebp-8h]

    for ( f = fields_1; f->name; ++f )
    {
        switch ( f->type )
        {
            case F_INT:
                *(int *)((char *)&dest->s.number + f->ofs) = *(int *)((char *)&source->s.number + f->ofs);
                break;
            case F_FLOAT:
                *(int *)((char *)&dest->s.number + f->ofs) = *(int *)((char *)&source->s.number + f->ofs);
                break;
            case F_STRING:
                Scr_SetString(
                    (unsigned __int16 *)((char *)dest + f->ofs),
                    *(unsigned __int16 *)((char *)&source->s.number + f->ofs),
                    SCRIPTINSTANCE_SERVER);
                break;
            case F_VECTOR:
                v2 = (float *)((char *)&dest->s.number + f->ofs);
                v3 = (float *)((char *)&source->s.number + f->ofs);
                *v2 = *v3;
                v2[1] = v3[1];
                v2[2] = v3[2];
                break;
            case F_MODEL:
                *(_WORD *)((char *)&dest->s.number + f->ofs) = *(_WORD *)((char *)&source->s.number + f->ofs);
                break;
            default:
                continue;
        }
    }
}

void __cdecl G_DuplicateScriptFields(gentity_s *dest, const gentity_s *source)
{
    if ( dest->s.number != dest - g_entities
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    412,
                    0,
                    "%s",
                    "dest->s.number == dest - g_entities") )
    {
        __debugbreak();
    }
    if ( source->s.number != source - g_entities
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    413,
                    0,
                    "%s",
                    "source->s.number == source - g_entities") )
    {
        __debugbreak();
    }
    Scr_CopyEntityNum(source->s.number, dest->s.number, 0, SCRIPTINSTANCE_SERVER);
}

bool __cdecl IsClientOnlyEntity(const SpawnVar *spawnVar)
{
    bool result; // al
    char *out; // [esp+28h] [ebp-24h] BYREF
    const char *modelName; // [esp+2Ch] [ebp-20h] BYREF
    const char *clientSide; // [esp+30h] [ebp-1Ch] BYREF
    DestructibleDef *ddef; // [esp+34h] [ebp-18h]
    const char *destructibledefname; // [esp+38h] [ebp-14h] BYREF
    bool bClientSide; // [esp+3Fh] [ebp-Dh]
    const char *classname; // [esp+40h] [ebp-Ch] BYREF
    unsigned int flags; // [esp+44h] [ebp-8h]
    const char *spawnFlags; // [esp+48h] [ebp-4h] BYREF

    G_SpawnString(spawnVar, "classname", "", &classname);
    G_SpawnString(spawnVar, "spawnflags", "", &spawnFlags);
    flags = atoi(spawnFlags);
    bClientSide = (flags & 2) != 0;
    if ( strcmp("script_model", classname) )
        goto LABEL_15;
    G_SpawnString(spawnVar, "destructibledef", 0, &destructibledefname);
    if ( destructibledefname )
    {
        ddef = DestructibleDefPrecache(destructibledefname);
        if ( ddef && ddef->clientOnly )
            return 1;
        goto LABEL_15;
    }
    if ( (clientSide = 0, G_SpawnString(spawnVar, "clientside", 0, &clientSide), !bClientSide) && !clientSide
        || !bClientSide && atof(clientSide) != 1.0 )
    {
LABEL_15:
        result = !strcmp("script_origin", classname)
                    && ((out = 0, G_SpawnString(spawnVar, "clientside", 0, (const char **)&out), bClientSide) || out)
                    && (bClientSide || atof(out) == 1.0);
        return result;
    }
    G_SpawnString(spawnVar, "model", 0, &modelName);
    if ( modelName && *modelName )
        G_ModelIndex((char *)modelName);
    return 1;
}

void __cdecl G_SetupDestructible(gentity_s *ent, char *destructibleName)
{
    char szConfigString[1024]; // [esp+0h] [ebp-408h] BYREF
    int iDestID; // [esp+404h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 533, 0, "%s", "ent") )
        __debugbreak();
    if ( !destructibleName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 534, 0, "%s", "destructibleName") )
    {
        __debugbreak();
    }
    for ( iDestID = 0; iDestID < 64; ++iDestID )
    {
        SV_GetConfigstring(iDestID + 3180, szConfigString, 1024);
        if ( !szConfigString[0] )
        {
            SV_SetConfigstring(iDestID + 3180, destructibleName);
            break;
        }
        if ( !I_stricmp(szConfigString, destructibleName) )
            break;
    }
    if ( iDestID >= 64
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    552,
                    0,
                    "%s",
                    "iDestID < MAX_DESTRUCTIBLE_DEFS") )
    {
        __debugbreak();
    }
    AssignToSmallerType<unsigned char>(&ent->s.un1.scale, iDestID);
    ent->s.lerp.eFlags |= 0x20000u;
}

void __cdecl G_SetupDestructible(gentity_s *ent, SpawnVar *spawnVar)
{
    const char *destructibleName; // [esp+0h] [ebp-4h] BYREF

    destructibleName = 0;
    G_SpawnString(spawnVar, "destructibledef", 0, &destructibleName);
    if ( destructibleName )
        G_SetupDestructible(ent, (char *)destructibleName);
}

void __cdecl G_CallSpawn(SpawnVar *spawnVar)
{
    const gitem_s *item; // [esp+8Ch] [ebp-14h]
    void (__cdecl *spawnFunc)(gentity_s *, SpawnVar *); // [esp+90h] [ebp-10h]
    const char *classname; // [esp+94h] [ebp-Ch] BYREF
    gentity_s *ent; // [esp+98h] [ebp-8h]
    int radiantLiveUpdate; // [esp+9Ch] [ebp-4h]

    if ( !spawnVar->spawnVarsValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    584,
                    1,
                    "%s",
                    "spawnVar->spawnVarsValid") )
    {
        __debugbreak();
    }
    radiantLiveUpdate = !level.spawnVar.spawnVarsValid;
    G_SpawnString(spawnVar, "classname", "", &classname);
    if ( classname )
    {
        if ( strncmp(classname, "dyn_", 4u) )
        {
            if ( !strncmp(classname, "node_", 5u) )
            {
                if ( !useFastFile->current.enabled )
                    G_SpawnPathnodeStatic(spawnVar, classname);
                G_SpawnPathnodeDynamic(spawnVar);
            }
            else if ( !strncmp(classname, "actor_", 6u) )
            {
                ent = G_Spawn();
                G_ParseEntityFields(spawnVar, ent, radiantLiveUpdate);
                SP_actor(ent, spawnVar);
            }
            else if ( !strcmp("info_vehicle_node", classname) )
            {
                SP_info_vehicle_node(spawnVar, 0, 0);
            }
            else if ( !strcmp("info_vehicle_node_rotate", classname) )
            {
                SP_info_vehicle_node(spawnVar, 1, 0);
            }
            else
            {
                item = G_GetItemForClassname(classname);
                if ( item )
                {
                    ent = G_Spawn();
                    G_ParseEntityFields(spawnVar, ent, radiantLiveUpdate);
                    G_SpawnItem(ent, item);
                }
                else if ( !IsClientOnlyEntity(spawnVar)
                             && strcmp("occlusion_override", classname)
                             && strcmp("undertow_volume", classname)
                             && strcmp("rope", classname) )
                {
                    if ( !strcmp("heli_height_lock", classname) )
                    {
                        G_SpawnHeliHeightLock(spawnVar);
                    }
                    else if ( strcmp("glass", classname) )
                    {
                        spawnFunc = G_FindSpawnFunc(classname, s_bspOrDynamicSpawns, 7);
                        if ( !spawnFunc && level.spawnVar.spawnVarsValid )
                            spawnFunc = G_FindSpawnFunc(classname, s_bspOnlySpawns, 15);
                        if ( (char *)spawnFunc != (char *)G_FreeEntityWrapper )
                        {
                            ent = G_Spawn();
                            G_ParseEntityFields(spawnVar, ent, radiantLiveUpdate);
                            G_SetupDestructible(ent, spawnVar);
                            if ( spawnFunc )
                                spawnFunc(ent, spawnVar);
                        }
                    }
                }
            }
        }
    }
    else
    {
        Com_Printf(15, "G_CallSpawn: NULL classname\n");
    }
}

const gitem_s *__cdecl G_GetItemForClassname(const char *classname)
{
    unsigned int weapIndex; // [esp+0h] [ebp-8h]

    if ( g_connectpaths->current.integer )
        return 0;
    if ( strncmp(classname, "weapon_", 7u) )
        return 0;
    weapIndex = G_GetWeaponIndexForName((char *)classname + 7);
    if ( !weapIndex )
        return 0;
    BG_GetWeaponDef(weapIndex);
    return BG_FindItemForWeapon(weapIndex);
}

void (__cdecl *__cdecl G_FindSpawnFunc(
                const char *classname,
                const SpawnFuncEntry *spawnFuncArray,
                int spawnFuncCount))(gentity_s *, SpawnVar *)
{
    int spawnFuncIter; // [esp+14h] [ebp-4h]

    for ( spawnFuncIter = 0; spawnFuncIter < spawnFuncCount; ++spawnFuncIter )
    {
        if ( !strcmp(classname, spawnFuncArray[spawnFuncIter].classname) )
            return spawnFuncArray[spawnFuncIter].callback;
    }
    return 0;
}

int __cdecl G_CallSpawnEntity(gentity_s *ent)
{
    const gitem_s *item; // [esp+0h] [ebp-Ch]
    void (__cdecl *spawnFunc)(gentity_s *); // [esp+4h] [ebp-8h]
    char *classname; // [esp+8h] [ebp-4h]

    if ( level.spawnVar.spawnVarsValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    705,
                    1,
                    "%s",
                    "!level.spawnVar.spawnVarsValid") )
    {
        __debugbreak();
    }
    if ( ent->classname )
    {
        classname = SL_ConvertToString(ent->classname, SCRIPTINSTANCE_SERVER);
        item = G_GetItemForClassname(classname);
        if ( item )
        {
            G_SpawnItem(ent, item);
            return 1;
        }
        else
        {
            spawnFunc = (void (__cdecl *)(gentity_s *))G_FindSpawnFunc(classname, s_bspOrDynamicSpawns, 7);
            if ( spawnFunc )
            {
                //if ( spawnFunc == G_FreeEntityWrapper
                //    && !Assert_MyHandler(
                //                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                //                729,
                //                0,
                //                "%s",
                //                "spawnFunc != G_FreeEntityWrapper") )
                //{
                //    __debugbreak();
                //}
                ((void (__cdecl *)(gentity_s *, unsigned int))spawnFunc)(ent, 0);
                if ( !ent->r.inuse
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                                731,
                                0,
                                "%s",
                                "ent->r.inuse") )
                {
                    __debugbreak();
                }
                return 1;
            }
            else
            {
                Com_Printf(15, "%s cannot be spawned dynamically\n", classname);
                return 0;
            }
        }
    }
    else
    {
        Com_Printf(15, "G_CallSpawnEntity: NULL classname\n");
        return 0;
    }
}

void __cdecl GScr_AddFieldsForEntity()
{
    const ent_field_t *f; // [esp+4h] [ebp-4h]

    for ( f = fields_1; f->name; ++f )
    {
        if ( ((f - fields_1) & 0xC000) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                        747,
                        0,
                        "%s",
                        "((f - fields) & ENTFIELD_MASK) == ENTFIELD_ENTITY") )
        {
            __debugbreak();
        }
        if ( f - fields_1 != (unsigned __int16)(f - fields_1)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                        748,
                        0,
                        "%s",
                        "(f - fields) == (unsigned short)( f - fields )") )
        {
            __debugbreak();
        }
        Scr_AddClassField(0, (char *)f->name, (unsigned __int16)(f - fields_1), SCRIPTINSTANCE_SERVER);
    }
    GScr_AddFieldsForActor();
    GScr_AddFieldsForSentient();
    GScr_AddFieldsForClient();
}

void __cdecl GScr_AddFieldsForRadiant()
{
    Scr_AddFields("radiant", (char*)"txt", SCRIPTINSTANCE_SERVER);
}

void __cdecl GScr_SetGenericField(unsigned __int8 *b, fieldtype_t type, int ofs, unsigned int whichbits)
{
    gentity_s *ent; // [esp+8h] [ebp-4h]
    gentity_s *enta; // [esp+8h] [ebp-4h]
    gentity_s *entb; // [esp+8h] [ebp-4h]

    switch ( type )
    {
        case F_ACTOR:
            ent = Scr_GetEntityAllowNull(0, SCRIPTINSTANCE_SERVER);
            if ( ent )
                *(unsigned int *)&b[ofs] = (unsigned int)ent->actor;
            else
                *(unsigned int *)&b[ofs] = 0;
            break;
        case F_SENTIENT:
            enta = Scr_GetEntityAllowNull(0, SCRIPTINSTANCE_SERVER);
            if ( enta )
                *(unsigned int *)&b[ofs] = (unsigned int)enta->sentient;
            else
                *(unsigned int *)&b[ofs] = 0;
            break;
        case F_SENTIENTHANDLE:
            entb = Scr_GetEntityAllowNull(0, SCRIPTINSTANCE_SERVER);
            if ( entb )
                ((SentientHandle *)&b[ofs])->setSentient(entb->sentient);
            else
                ((SentientHandle *)&b[ofs])->setSentient(0);
            break;
        case F_PATHNODE:
            *(unsigned int *)&b[ofs] = (unsigned int)Scr_GetPathnode(0, SCRIPTINSTANCE_SERVER);
            break;
        default:
            Scr_SetGenericField(b, type, ofs, SCRIPTINSTANCE_SERVER, whichbits);
            break;
    }
}

int __cdecl Scr_SetObjectField(unsigned int classnum, unsigned int entnum, unsigned int offset)
{
    const char *v4; // eax
    const char *v5; // eax

    switch ( classnum )
    {
        case 0u:
            return Scr_SetEntityField(entnum, offset);
        case 1u:
            Scr_SetHudElemField(entnum, offset);
            return 1;
        case 2u:
            Scr_SetPathnodeField(entnum, offset);
            return 1;
        case 3u:
            v4 = va("vehicle node is read-only");
            Scr_Error(v4, 0);
            return 1;
        default:
            v5 = va("bad class num %u", classnum);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 894, 0, v5) )
                __debugbreak();
            return 1;
    }
}

int __cdecl Scr_SetEntityField(unsigned int entnum, unsigned int offset)
{
    const ent_field_t *f; // [esp+0h] [ebp-8h]
    gentity_s *ent; // [esp+4h] [ebp-4h]

    if ( entnum >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    781,
                    0,
                    "%s",
                    "(unsigned)entnum < MAX_GENTITIES") )
    {
        __debugbreak();
    }
    ent = &g_entities[entnum];
    if ( !ent->r.inuse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    784,
                    0,
                    "%s\n\t(ent->s.number) = %i",
                    "(ent->r.inuse)",
                    ent->s.number) )
    {
        __debugbreak();
    }
    switch ( offset & 0xC000 )
    {
        case 0x8000u:
            if ( ent->actor )
            {
                Scr_SetActorField(ent->actor, offset & 0xFFFF3FFF);
                return 1;
            }
            else
            {
                return 0;
            }
        case 0x4000u:
            if ( ent->sentient )
            {
                Scr_SetSentientField(ent->sentient, offset & 0xFFFF3FFF);
                return 1;
            }
            else
            {
                return 0;
            }
        case 0xC000u:
            if ( ent->client )
            {
                Scr_SetClientField(ent->client, offset & 0xFFFF3FFF);
                return 1;
            }
            else
            {
                return 0;
            }
        default:
            if ( offset >= 0xF
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                            815,
                            0,
                            "%s",
                            "(unsigned)offset < ARRAY_COUNT( fields ) - 1") )
            {
                __debugbreak();
            }
            f = &fields_1[offset];
            if ( f->callback )
                f->callback(ent, offset);
            else
                GScr_SetGenericField((unsigned __int8 *)ent, f->type, f->ofs, 0);
            return 1;
    }
}

void __cdecl Scr_GetEntityField(unsigned int entnum, unsigned int offset)
{
    gentity_s *ent; // [esp+4h] [ebp-4h]

    if ( entnum >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    911,
                    0,
                    "%s",
                    "(unsigned)entnum < MAX_GENTITIES") )
    {
        __debugbreak();
    }
    ent = &g_entities[entnum];
    if ( !ent->r.inuse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    914,
                    0,
                    "%s\n\t(ent->s.number) = %i",
                    "(ent->r.inuse)",
                    ent->s.number) )
    {
        __debugbreak();
    }
    switch ( offset & 0xC000 )
    {
        case 0x8000u:
            if ( ent->actor )
                Scr_GetActorField(ent->actor, offset & 0xFFFF3FFF);
            break;
        case 0x4000u:
            if ( ent->sentient )
                Scr_GetSentientField(ent->sentient, offset & 0xFFFF3FFF);
            break;
        case 0xC000u:
            if ( ent->client )
                Scr_GetClientField(ent->client, offset & 0xFFFF3FFF);
            break;
        default:
            if ( offset >= 0xF
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                            942,
                            0,
                            "%s",
                            "(unsigned)offset < ARRAY_COUNT( fields ) - 1") )
            {
                __debugbreak();
            }
            GScr_GetGenericField((unsigned __int8 *)ent, fields_1[offset].type, fields_1[offset].ofs, 0);
            break;
    }
}

void __cdecl GScr_GetGenericField(unsigned __int8 *b, fieldtype_t type, int ofs, unsigned int whichbits)
{
    gentity_s *v4; // eax
    sentient_s *v5; // eax
    unsigned int v6; // eax
    unsigned int id; // [esp+4h] [ebp-4h]

    switch ( type )
    {
        case F_ENTITY:
            if ( *(unsigned int *)&b[ofs] )
                Scr_AddEntity(*(gentity_s **)&b[ofs], SCRIPTINSTANCE_SERVER);
            break;
        case F_ENTHANDLE:
            if ( ((EntHandle *)&b[ofs])->isDefined() )
            {
                v4 = ((EntHandle *)&b[ofs])->ent();
                Scr_AddEntity(v4, SCRIPTINSTANCE_SERVER);
            }
            break;
        case F_ACTOR:
            if ( *(unsigned int *)&b[ofs] )
                Scr_AddEntity(**(gentity_s ***)&b[ofs], SCRIPTINSTANCE_SERVER);
            break;
        case F_SENTIENT:
            if ( *(unsigned int *)&b[ofs] )
                Scr_AddEntity(**(gentity_s ***)&b[ofs], SCRIPTINSTANCE_SERVER);
            break;
        case F_SENTIENTHANDLE:
            if ( ((SentientHandle *)&b[ofs])->isDefined() )
            {
                v5 = ((SentientHandle *)&b[ofs])->sentient();
                Scr_AddEntity(v5->ent, SCRIPTINSTANCE_SERVER);
            }
            break;
        case F_PATHNODE:
            if ( *(unsigned int *)&b[ofs] )
                Scr_AddPathnode(*(pathnode_t **)&b[ofs]);
            break;
        case F_OBJECT:
            id = *(unsigned int *)&b[ofs];
            if ( id )
                Scr_AddObject(id, SCRIPTINSTANCE_SERVER);
            break;
        case F_MODEL:
            v6 = G_ModelName(b[ofs]);
            Scr_AddConstString(v6, SCRIPTINSTANCE_SERVER);
            break;
        default:
            Scr_GetGenericField(b, type, ofs, SCRIPTINSTANCE_SERVER, whichbits);
            break;
    }
}

void __cdecl Scr_GetObjectField(unsigned int classnum, unsigned int entnum, unsigned int offset)
{
    const char *v3; // eax

    switch ( classnum )
    {
        case 0u:
            Scr_GetEntityField(entnum, offset);
            break;
        case 1u:
            Scr_GetHudElemField(entnum, offset);
            break;
        case 2u:
            Scr_GetPathnodeField(entnum, offset);
            break;
        case 3u:
            GScr_GetVehicleNodeField(entnum, offset);
            break;
        default:
            v3 = va("bad class num %u", classnum);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 1023, 0, v3) )
                __debugbreak();
            break;
    }
}

void __cdecl Scr_FreeEntityConstStrings(gentity_s *pEnt)
{
    const ent_field_t *f; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    for ( f = fields_1; f->name; ++f )
    {
        if ( f->type == F_STRING )
            Scr_SetString((unsigned __int16 *)((char *)pEnt + f->ofs), 0, SCRIPTINSTANCE_SERVER);
    }
    for ( i = 0; i < 19; ++i )
    {
        pEnt->attachModelNames[i] = 0;
        Scr_SetString(&pEnt->attachTagNames[i], 0, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl Scr_FreeEntity(gentity_s *ent)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 1065, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->s.number != ent - g_entities
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1066,
                    0,
                    "ent->s.number == ent - g_entities\n\t%i, %i",
                    ent->s.number,
                    ent - g_entities) )
    {
        __debugbreak();
    }
    if ( !ent->r.inuse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1067,
                    0,
                    "%s\n\t(ent->s.number) = %i",
                    "(ent->r.inuse)",
                    ent->s.number) )
    {
        __debugbreak();
    }
    Scr_FreeEntityConstStrings(ent);
    Scr_FreeEntityNum(ent->s.number, 0, SCRIPTINSTANCE_SERVER);
}

void __cdecl Scr_AddEntity(gentity_s *ent, scriptInstance_t inst)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 1076, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->s.number != ent - g_entities
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1077,
                    0,
                    "ent->s.number == ent - g_entities\n\t%i, %i",
                    ent->s.number,
                    ent - g_entities) )
    {
        __debugbreak();
    }
    if ( !ent->r.inuse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1078,
                    0,
                    "%s\n\t(ent->s.number) = %i",
                    "(ent->r.inuse)",
                    ent->s.number) )
    {
        __debugbreak();
    }
    Scr_AddEntityNum(ent->s.number, 0, inst, 0);
}

gentity_s *__cdecl Scr_GetEntityAllowNull(unsigned int index, scriptInstance_t inst)
{
    scr_entref_t v3; // [esp+0h] [ebp-1Ch] BYREF
    scr_entref_t v4; // [esp+Ah] [ebp-12h]
    scr_entref_t entref; // [esp+10h] [ebp-Ch]

    if ( !Scr_GetType(index, inst) )
        return 0;
    //v4 = *Scr_GetEntityRef(&v3, index, inst);
    v4 = Scr_GetEntityRef(index, inst);
    entref = v4;
    if ( v4.classnum )
        return 0;
    if ( entref.entnum >= 0x400u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1101,
                    0,
                    "%s",
                    "entref.entnum < MAX_GENTITIES") )
    {
        __debugbreak();
    }
    return &g_entities[entref.entnum];
}

gentity_s *__cdecl Scr_GetEntity(unsigned int index)
{
    scr_entref_t v2; // [esp+0h] [ebp-18h] BYREF
    scr_entref_t v3; // [esp+Ah] [ebp-Eh]
    scr_entref_t entref; // [esp+10h] [ebp-8h]

    //v3 = *Scr_GetEntityRef(&v2, index, SCRIPTINSTANCE_SERVER);
    v3 = Scr_GetEntityRef(index, SCRIPTINSTANCE_SERVER);
    entref = v3;
    if ( v3.classnum )
    {
        Scr_ParamError(index, "not an entity", SCRIPTINSTANCE_SERVER);
        return 0;
    }
    else
    {
        if ( entref.entnum >= 0x400u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                        1120,
                        0,
                        "%s",
                        "entref.entnum < MAX_GENTITIES") )
        {
            __debugbreak();
        }
        return &g_entities[entref.entnum];
    }
}

void __cdecl Scr_FreeHudElem(game_hudelem_s *hud)
{
    if ( !hud && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 1137, 0, "%s", "hud") )
        __debugbreak();
    if ( (unsigned int)(hud - g_hudelems) >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1138,
                    0,
                    "hud - g_hudelems doesn't index MAX_HUDELEMS_TOTAL\n\t%i not in [0, %i)",
                    hud - g_hudelems,
                    1024) )
    {
        __debugbreak();
    }
    if ( !hud->elem.type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1139,
                    0,
                    "%s",
                    "hud->elem.type != HE_TYPE_FREE") )
    {
        __debugbreak();
    }
    Scr_NotifyNum(hud - g_hudelems, 1u, scr_const.death, 0);
    Scr_FreeHudElemConstStrings(hud);
    Scr_FreeEntityNum(hud - g_hudelems, 1u, SCRIPTINSTANCE_SERVER);
}

void __cdecl Scr_AddHudElem(game_hudelem_s *hud)
{
    if ( !hud && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 1158, 0, "%s", "hud") )
        __debugbreak();
    if ( (unsigned int)(hud - g_hudelems) >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1159,
                    0,
                    "hud - g_hudelems doesn't index MAX_HUDELEMS_TOTAL\n\t%i not in [0, %i)",
                    hud - g_hudelems,
                    1024) )
    {
        __debugbreak();
    }
    if ( !hud->elem.type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1160,
                    0,
                    "%s",
                    "hud->elem.type != HE_TYPE_FREE") )
    {
        __debugbreak();
    }
    Scr_AddEntityNum(hud - g_hudelems, 1u, SCRIPTINSTANCE_SERVER, 0);
}

unsigned __int16 __cdecl Scr_ExecEntThread(gentity_s *ent, int handle, unsigned int paramcount)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 1193, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->s.number != ent - g_entities
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1194,
                    0,
                    "ent->s.number == ent - g_entities\n\t%i, %i",
                    ent->s.number,
                    ent - g_entities) )
    {
        __debugbreak();
    }
    if ( !ent->r.inuse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1195,
                    0,
                    "%s\n\t(ent->s.number) = %i",
                    "(ent->r.inuse)",
                    ent->s.number) )
    {
        __debugbreak();
    }
    return Scr_ExecEntThreadNum(SCRIPTINSTANCE_SERVER, ent->s.number, 0, handle, paramcount, 0);
}

void __cdecl Scr_Notify(gentity_s *ent, unsigned __int16 stringValue, unsigned int paramcount)
{
    char *v3; // eax
    const char *v4; // eax

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 1223, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->s.number != ent - g_entities )
    {
        v3 = SL_ConvertToString(stringValue, SCRIPTINSTANCE_SERVER);
        v4 = va("s.number = %i, index = %i, stringValue = %s", ent->s.number, ent - g_entities, v3);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                        1224,
                        0,
                        "%s\n\t%s",
                        "ent->s.number == ent - g_entities",
                        v4) )
            __debugbreak();
    }
    if ( !ent->r.inuse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1225,
                    0,
                    "%s\n\t(ent->s.number) = %i",
                    "(ent->r.inuse)",
                    ent->s.number) )
    {
        __debugbreak();
    }
    Scr_NotifyNum(ent->s.number, 0, stringValue, paramcount);
}

void __cdecl Scr_GetEnt()
{
    gentity_s *result; // [esp+0h] [ebp-24h]
    unsigned __int16 name; // [esp+8h] [ebp-1Ch]
    int offset; // [esp+Ch] [ebp-18h]
    char *key; // [esp+10h] [ebp-14h]
    gentity_s *ent; // [esp+18h] [ebp-Ch]
    int i; // [esp+1Ch] [ebp-8h]
    unsigned __int16 value; // [esp+20h] [ebp-4h]

    name = (unsigned __int16)Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    key = Scr_GetString(1u, SCRIPTINSTANCE_SERVER);
    offset = Scr_GetOffset(0, key, SCRIPTINSTANCE_SERVER);
    if ( offset >= 0 )
    {
        if ( offset >= 15
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                        1255,
                        0,
                        "%s",
                        "offset >= 0 && offset < static_cast<int>( ARRAY_COUNT( fields ) - 1 )") )
        {
            __debugbreak();
        }
        if ( fields_1[offset].type == F_STRING )
        {
            result = 0;
            i = 0;
            ent = g_entities;
            while ( i < level.num_entities )
            {
                if ( ent->r.inuse )
                {
                    value = *(_WORD *)((char *)&ent->s.number + fields_1[offset].ofs);
                    if ( value )
                    {
                        if ( value == name )
                        {
                            if ( result )
                                Scr_Error("getent used with more than one entity", 0);
                            result = ent;
                        }
                    }
                }
                ++i;
                ++ent;
            }
            if ( result )
                Scr_AddEntity(result, SCRIPTINSTANCE_SERVER);
        }
    }
}

void __cdecl Scr_GetEntArray()
{
    unsigned __int16 name; // [esp+4h] [ebp-1Ch]
    int offset; // [esp+8h] [ebp-18h]
    char *key; // [esp+Ch] [ebp-14h]
    gentity_s *ent; // [esp+14h] [ebp-Ch]
    gentity_s *enta; // [esp+14h] [ebp-Ch]
    int i; // [esp+18h] [ebp-8h]
    int ia; // [esp+18h] [ebp-8h]
    unsigned __int16 value; // [esp+1Ch] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) )
    {
        name = (unsigned __int16)Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
        key = Scr_GetString(1u, SCRIPTINSTANCE_SERVER);
        offset = Scr_GetOffset(0, key, SCRIPTINSTANCE_SERVER);
        if ( offset >= 0 )
        {
            if ( offset >= 15
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                            1321,
                            0,
                            "%s",
                            "offset >= 0 && offset < static_cast<int>( ARRAY_COUNT( fields ) - 1 )") )
            {
                __debugbreak();
            }
            if ( fields_1[offset].type == F_STRING )
            {
                Scr_MakeArray(SCRIPTINSTANCE_SERVER);
                ia = 0;
                enta = g_entities;
                while ( ia < level.num_entities )
                {
                    if ( enta->r.inuse )
                    {
                        value = *(_WORD *)((char *)&enta->s.number + fields_1[offset].ofs);
                        if ( value )
                        {
                            if ( value == name )
                            {
                                Scr_AddEntity(enta, SCRIPTINSTANCE_SERVER);
                                Scr_AddArray(SCRIPTINSTANCE_SERVER);
                            }
                        }
                    }
                    ++ia;
                    ++enta;
                }
            }
        }
    }
    else
    {
        Scr_MakeArray(SCRIPTINSTANCE_SERVER);
        i = 0;
        ent = g_entities;
        while ( i < level.num_entities )
        {
            if ( ent->r.inuse )
            {
                Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
                Scr_AddArray(SCRIPTINSTANCE_SERVER);
            }
            ++i;
            ++ent;
        }
    }
}

void __cdecl GScr_SetDynamicEntityField(gentity_s *ent, unsigned int index)
{
    Scr_SetDynamicEntityField(ent->s.number, 0, index, SCRIPTINSTANCE_SERVER, 0);
}

void __cdecl SP_worldspawn(SpawnVar *spawnVar)
{
    char *v1; // eax
    float v2; // [esp+4h] [ebp-14h]
    float value; // [esp+8h] [ebp-10h]
    float v4; // [esp+10h] [ebp-8h]
    const char *s; // [esp+14h] [ebp-4h] BYREF

    G_SpawnString(spawnVar, "classname", "", &s);
    if ( I_stricmp(s, "worldspawn") )
        Com_Error(ERR_DROP, "SP_worldspawn: The first entity isn't 'worldspawn'");
    SV_SetConfigstring(2, (char*)"cod");
    G_SpawnString(spawnVar, "ambienttrack", "", &s);
    if ( *s )
    {
        v1 = va("n\\%s", s);
        SV_SetConfigstring(1547, v1);
    }
    else
    {
        SV_SetConfigstring(1547, (char *)"");
    }
    G_SpawnString(spawnVar, "message", "", &s);
    SV_SetConfigstring(4, (char *)s);
    SV_SetConfigstring(11, (char *)g_motd->current.integer);
    G_SpawnString(spawnVar, "gravity", "800", &s);
    if ( !bg_gravity
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 1381, 0, "%s", "bg_gravity") )
    {
        __debugbreak();
    }
    value = atof(s);
    Dvar_SetFloat((dvar_s *)bg_gravity, value);
    G_SpawnString(spawnVar, "northyaw", "", &s);
    if ( *s )
    {
        SV_SetConfigstring(1548, (char *)s);
        v2 = atof(s);
        v4 = DEG2RAD(v2);
        level.compassNorth[0] = cos(v4);
        level.compassNorth[1] = sin(v4);
    }
    else
    {
        SV_SetConfigstring(1548, (char*)"0");
        level.compassNorth[0] = 1.0f;
        level.compassNorth[1] = 0.0f;
    }
    G_SpawnString(spawnVar, "spawnflags", "0", &s);
    g_entities[1022].spawnflags = atoi(s);
    g_entities[1022].s.number = 1022;
    Scr_SetString(&g_entities[1022].classname, scr_const.worldspawn, SCRIPTINSTANCE_SERVER);
    g_entities[1022].r.inuse = 1;
    if ( g_entities[1022].r.ownerNum.isDefined() )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                        1403,
                        0,
                        "%s",
                        "!g_entities[ENTITYNUM_WORLD].r.ownerNum.isDefined()") )
            __debugbreak();
    }
}

void __cdecl G_SpawnEntitiesFromString()
{
    if ( !G_ParseSpawnVars(&level.spawnVar) )
        Com_Error(ERR_DROP, "SpawnEntities: no entities");
    SP_worldspawn(&level.spawnVar);
    while ( G_ParseSpawnVars(&level.spawnVar) )
        G_CallSpawn(&level.spawnVar);
    G_ResetEntityParsePoint();
}

void __cdecl G_LoadStructs()
{
    unsigned __int16 hThread; // [esp+14h] [ebp-8h]
    const char *classname; // [esp+18h] [ebp-4h] BYREF

    if ( !g_scr_data.initstructs
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1443,
                    0,
                    "%s",
                    "g_scr_data.initstructs") )
    {
        __debugbreak();
    }
    hThread = Scr_ExecThread(SCRIPTINSTANCE_SERVER, g_scr_data.initstructs, 0);
    Scr_FreeThread(hThread, SCRIPTINSTANCE_SERVER);
    while ( G_ParseSpawnVars(&level.spawnVar) )
    {
        G_SpawnString(&level.spawnVar, "classname", "", &classname);
        if ( !strcmp("script_struct", classname) )
            G_SpawnStruct(&level.spawnVar);
    }
    G_ResetEntityParsePoint();
}

