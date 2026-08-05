#include "g_utils_mp.h"
#include <qcommon/dobj_management.h>
#include <clientscript/cscr_stringlist.h>
#include <server_mp/sv_init_mp.h>
#include <clientscript/scr_const.h>
#include <clientscript/cscr_vm.h>
#include "g_main_mp.h"
#include <server/sv_game.h>
#include <xanim/xmodel_load_obj.h>
#include <universal/com_math_anglevectors.h>
#include <universal/com_memory.h>
#include <clientscript/cscr_memorytree.h>
#include <client_mp/g_client_mp.h>
#include <cgame/cg_drawtools.h>
#include "g_spawnsystem_mp.h"
#include <server/sv_world.h>
#include <game/actor_script_cmd.h>
#include <xanim/dobj_skel.h>
#include <game/g_missile.h>
#include <game/g_targets.h>
#include "actor_mp.h"
#include <game/actor_event_listeners.h>
#include <game/actor_corpse.h>
#include <game/turret.h>
#include <game/g_player_corpse.h>
#include "g_spawn_mp.h"
#include <bgame/bg_misc.h>
#include <sound/snd_utils.h>
#include <stringed/stringed_hooks.h>

const char *origErrorMsg = "localized string";

XModel *cached_models[512];

void __cdecl G_ClearCachedModels()
{
    memset(cached_models, 0, sizeof(cached_models));
}

int __cdecl G_FindConfigstringIndex(char *name, int start, int max, int create, const char *errormsg)
{
    const char *v6; // eax
    unsigned int ConfigstringConst; // eax
    char *v8; // eax
    const char *v9; // eax
    unsigned int String; // [esp+0h] [ebp-14h]
    unsigned int s; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    if ( !name || !*name )
        return 0;
    if ( start < 1547 )
        String = SL_FindString(name, SCRIPTINSTANCE_SERVER);
    else
        String = SL_FindLowercaseString(name, SCRIPTINSTANCE_SERVER);
    for ( i = 1; i < max; ++i )
    {
        s = SV_GetConfigstringConst(i + start);
        if ( s == scr_const._ )
            break;
        if ( s == String )
            return i;
    }
    if ( create )
    {
        if ( i == max )
        {
            Com_Printf(15, "G_FindConfigstringIndex: overflow...\n");
            Com_Printf(15, "Dumping these %i Config Strings:\n", max);
            for ( i = 1; i < max; ++i )
            {
                ConfigstringConst = SV_GetConfigstringConst(i + start);
                v8 = SL_ConvertToString(ConfigstringConst, SCRIPTINSTANCE_SERVER);
                Com_Printf(15, "%i: %s\n", i, v8);
            }
            v9 = va("G_FindConfigstringIndex: overflow (%d): %s", start, name);
            Com_Error(ERR_DROP, v9);
        }
        SV_SetConfigstring(i + start, name);
        return i;
    }
    else
    {
        if ( errormsg )
        {
            v6 = va("%s \"%s\" not precached", errormsg, name);
            Scr_Error(v6, 0);
        }
        return 0;
    }
}

int __cdecl G_LocalizedStringIndex(char *string)
{
    int v2; // eax
    int configStringIndex; // [esp+0h] [ebp-Ch]
    const char *errormsg; // [esp+4h] [ebp-8h]
    int allowCreate; // [esp+8h] [ebp-4h]

    if ( !string
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 147, 0, "%s", "string") )
    {
        __debugbreak();
    }
    if ( !*string )
        return 0;
    allowCreate = level.initializing;
    errormsg = origErrorMsg;
    if ( !level.initializing )
    {
        if ( !loc_warnings->current.enabled )
        {
            allowCreate = 1;
            v2 = G_FindConfigstringIndex(string, 515, 1023, 1, origErrorMsg);
            goto LABEL_12;
        }
        if ( !loc_warningsAsErrors->current.enabled )
            errormsg = 0;
    }
    v2 = G_FindConfigstringIndex(string, 515, 1023, level.initializing, errormsg);
LABEL_12:
    configStringIndex = v2;
    if ( !v2 && !allowCreate && loc_warnings->current.enabled && !loc_warningsAsErrors->current.enabled )
    {
        configStringIndex = G_FindConfigstringIndex(string, 515, 1023, 1, origErrorMsg);
        if ( configStringIndex )
            Com_PrintWarning(25, "WARNING: %s \"%s\" not precached\n", origErrorMsg, string);
    }
    return configStringIndex;
}

int __cdecl G_MaterialIndex(const char *name)
{
    char v2; // [esp+3h] [ebp-65h]
    char *v3; // [esp+8h] [ebp-60h]
    const char *v4; // [esp+Ch] [ebp-5Ch]
    char shaderName[68]; // [esp+20h] [ebp-48h] BYREF

    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 191, 0, "%s", "name") )
        __debugbreak();
    if ( !*name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 192, 0, "%s", "name[0]") )
    {
        __debugbreak();
    }
    if ( strlen(name) >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    193,
                    0,
                    "%s",
                    "strlen(name) < MAX_QPATH") )
    {
        __debugbreak();
    }
    v4 = name;
    v3 = shaderName;
    do
    {
        v2 = *v4;
        *v3++ = *v4++;
    }
    while ( v2 );
    I_strlwr(shaderName);
    return G_FindConfigstringIndex(shaderName, 2580, 256, level.initializing, "material");
}

int __cdecl G_ModelIndex(char *name)
{
    const char *v2; // eax
    unsigned int nameString; // [esp+0h] [ebp-Ch]
    unsigned int s; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    iassert(name);

    if (!name[0])
        return 0;

    nameString = SL_FindLowercaseString(name, SCRIPTINSTANCE_SERVER);

    for ( i = 1; i < MAX_MODELS; ++i )
    {
        s = SV_GetConfigstringConst(i + 1568);
        if ( s == scr_const._ )
            break;
        if ( s == nameString )
        {
            iassert(cached_models[i]);
            return i;
        }
    }
    if ( !level.initializing )
    {
        Scr_Error(va("model '%s' not precached", name), 0);
    }
    bcassert(i, MAX_MODELS);

    if ( i == MAX_MODELS)
        Com_Error(ERR_DROP, "G_ModelIndex: overflow");

    cached_models[i] = SV_XModelGet(name);
    SV_SetConfigstring(i + 1568, name);
    return i;
}

bool __cdecl G_GetModelBounds(int index, float *outMins, float *outMaxs)
{
    float identityBasis[3][3]; // [esp+0h] [ebp-28h] BYREF
    XModel *xmodel; // [esp+24h] [ebp-4h]

    if ( !outMins
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 368, 0, "%s", "outMins") )
    {
        __debugbreak();
    }
    if ( !outMaxs
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 369, 0, "%s", "outMaxs") )
    {
        __debugbreak();
    }
    xmodel = G_GetModel(index);
    if ( !xmodel
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 371, 0, "%s", "xmodel") )
    {
        __debugbreak();
    }
    AxisClear(identityBasis);
    return XModelGetStaticBounds(xmodel, outMins, outMaxs) != 0;
}

XModel *__cdecl G_GetModel(int index)
{
    if ( index <= 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 358, 0, "%s", "index > 0") )
    {
        __debugbreak();
    }
    if ( index >= 512
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    359,
                    0,
                    "%s",
                    "index < MAX_MODELS") )
    {
        __debugbreak();
    }
    return cached_models[index];
}

bool __cdecl G_XModelBad(int index)
{
    const XModel *Model; // eax

    if ( !index && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 383, 0, "%s", "index") )
        __debugbreak();
    Model = G_GetModel(index);
    return XModelBad(Model);
}

unsigned int __cdecl G_ModelName(unsigned int index)
{
    if ( index >= 0x200
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    390,
                    0,
                    "%s",
                    "(unsigned)index < MAX_MODELS") )
    {
        __debugbreak();
    }
    return SV_GetConfigstringConst(index + 1568);
}

void __cdecl G_EntityCentroid(const gentity_s *ent, float *centroid)
{
    float offset[3]; // [esp+Ch] [ebp-30h] BYREF
    float axis[3][3]; // [esp+18h] [ebp-24h] BYREF

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 397, 0, "%s", "ent") )
        __debugbreak();
    if ( !centroid
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 398, 0, "%s", "centroid") )
    {
        __debugbreak();
    }
    AnglesToAxis(ent->r.currentAngles, axis);
    offset[0] = ent->r.mins[0] + ent->r.maxs[0];
    offset[1] = ent->r.mins[1] + ent->r.maxs[1];
    offset[2] = ent->r.mins[2] + ent->r.maxs[2];
    offset[0] = 0.5 * offset[0];
    offset[1] = 0.5 * offset[1];
    offset[2] = 0.5 * offset[2];
    MatrixTransformVector(offset, axis, centroid);
    *centroid = ent->r.currentOrigin[0] + *centroid;
    centroid[1] = ent->r.currentOrigin[1] + centroid[1];
    centroid[2] = ent->r.currentOrigin[2] + centroid[2];
}

void __cdecl G_GetEntityBoundsPoint(
                const gentity_s *ent,
                float ratioFromCenterX,
                float ratioFromCenterY,
                float ratioFromCenterZ,
                float *result)
{
    float halfSize; // [esp+10h] [ebp-48h]
    float halfSize_4; // [esp+14h] [ebp-44h]
    float halfSize_8; // [esp+18h] [ebp-40h]
    float testPoint[3]; // [esp+1Ch] [ebp-3Ch] BYREF
    float midPoint[3]; // [esp+28h] [ebp-30h]
    float axis[3][3]; // [esp+34h] [ebp-24h] BYREF

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 418, 0, "%s", "ent") )
        __debugbreak();
    if ( !result
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 419, 0, "%s", "result") )
    {
        __debugbreak();
    }
    AnglesToAxis(ent->r.currentAngles, axis);
    halfSize = 0.5 * (float)(ent->r.maxs[0] - ent->r.mins[0]);
    halfSize_4 = 0.5 * (float)(ent->r.maxs[1] - ent->r.mins[1]);
    halfSize_8 = 0.5 * (float)(ent->r.maxs[2] - ent->r.mins[2]);
    midPoint[0] = ent->r.mins[0] + halfSize;
    midPoint[1] = ent->r.mins[1] + halfSize_4;
    midPoint[2] = ent->r.mins[2] + halfSize_8;
    testPoint[0] = (float)(halfSize * ratioFromCenterX) + midPoint[0];
    testPoint[1] = (float)(halfSize_4 * ratioFromCenterY) + midPoint[1];
    testPoint[2] = (float)(halfSize_8 * ratioFromCenterZ) + midPoint[2];
    MatrixTransformVector(testPoint, axis, result);
    *result = ent->r.currentOrigin[0] + *result;
    result[1] = ent->r.currentOrigin[1] + result[1];
    result[2] = ent->r.currentOrigin[2] + result[2];
}

int __cdecl G_TagIndex(char *name)
{
    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 440, 0, "%s", "name") )
        __debugbreak();
    return G_FindConfigstringIndex(name, 3115, 32, 1, 0);
}

int __cdecl G_EffectIndex(char *name)
{
    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 447, 0, "%s", "name") )
        __debugbreak();
    return G_FindConfigstringIndex(name, 2080, 196, level.initializing, "effect");
}

int __cdecl G_ShellShockIndex(char *name)
{
    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 454, 0, "%s", "name") )
        __debugbreak();
    return G_FindConfigstringIndex(name, 2532, 16, 1, 0);
}

XAnimTree_s *__cdecl G_GetEntAnimTree(gentity_s *ent)
{
    return ent->pAnimTree;
}

void __cdecl G_UpdateVehicleAttachedModels(gentity_s *ent)
{
    int v1; // eax
    char *tagName; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    int modelIndex; // [esp+8h] [ebp-4h]

    if ( ent->s.eType == 14 || ent->s.eType == 16 )
    {
        for ( i = 0; i < 2; ++i )
        {
            modelIndex = ent->attachModelNames[i];
            AssignToSmallerType<short>(&ent->s.vehicleState.attachModelIndex[i], modelIndex);
            ent->s.vehicleState.attachTagIndex[i] = 0;
            if ( modelIndex )
            {
                if ( !ent->attachTagNames[i]
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                                513,
                                0,
                                "%s",
                                "ent->attachTagNames[i]") )
                {
                    __debugbreak();
                }
                tagName = SL_ConvertToString(ent->attachTagNames[i], SCRIPTINSTANCE_SERVER);
                v1 = G_TagIndex(tagName);
                AssignToSmallerType<unsigned char>(&ent->s.vehicleState.attachTagIndex[i], v1);
            }
        }
    }
}

void __cdecl G_UpdateScriptMoverAttachedModels(gentity_s *ent)
{
    int v1; // eax
    char *tagName; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    int modelIndex; // [esp+8h] [ebp-4h]

    if ( ent->s.eType == 6 )
    {
        for ( i = 0; i < 4; ++i )
        {
            modelIndex = ent->attachModelNames[i];
            AssignToSmallerType<short>((__int16 *)ent->s.lerp.u.vehicle.gunnerAngles + i, modelIndex);
            ent->s.lerp.u.primaryLight.colorAndExp[i] = 0;
            if ( modelIndex )
            {
                if ( !ent->attachTagNames[i]
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                                537,
                                0,
                                "%s",
                                "ent->attachTagNames[i]") )
                {
                    __debugbreak();
                }
                tagName = SL_ConvertToString(ent->attachTagNames[i], SCRIPTINSTANCE_SERVER);
                v1 = G_TagIndex(tagName);
                AssignToSmallerType<unsigned char>((unsigned __int8 *)&ent->s.lerp.u + i, v1);
            }
        }
    }
}

void __cdecl G_DObjUpdate(gentity_s *ent)
{
    DObj *dobj; // [esp+0h] [ebp-118h]
    XAnimTree_s *tree; // [esp+4h] [ebp-114h]
    XModel *model; // [esp+8h] [ebp-110h]
    signed int numModels; // [esp+Ch] [ebp-10Ch]
    unsigned __int16 numModelsa; // [esp+Ch] [ebp-10Ch]
    int i; // [esp+10h] [ebp-108h]
    int modelIndex; // [esp+14h] [ebp-104h]
    int modelIndexa; // [esp+14h] [ebp-104h]
    DObjModel_s dobjModels[32]; // [esp+18h] [ebp-100h] BYREF

    if ( !ent->client )
    {
        G_SafeDObjFree(ent);
        modelIndex = ent->model;
        if ( ent->model )
        {
            tree = G_GetEntAnimTree(ent);
            model = G_GetModel(modelIndex);
            if ( !model
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 599, 0, "%s", "model") )
            {
                __debugbreak();
            }
            dobjModels[0].model = model;
            dobjModels[0].boneName = 0;
            dobjModels[0].ignoreCollision = 0;
            numModels = 1;
            if ( !ent->s.eType
                || ent->s.eType == 6
                || ent->s.eType == 13
                || ent->s.eType == 11
                || ent->s.eType == 17
                || ent->s.eType == 19 )
            {
                AssignToSmallerType<short>(&ent->s.index.brushmodel, modelIndex);
            }
            else if ( ent->s.eType == 14 || ent->s.eType == 12 )
            {
                ent->s.un3.item = modelIndex;
            }
            G_UpdateScriptMoverAttachedModels(ent);
            G_UpdateVehicleAttachedModels(ent);
            for ( i = 0; i < 19; ++i )
            {
                modelIndexa = ent->attachModelNames[i];
                if ( !ent->attachModelNames[i] )
                    break;
                if ( numModels >= 32
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                                627,
                                0,
                                "%s",
                                "numModels < DOBJ_MAX_SUBMODELS") )
                {
                    __debugbreak();
                }
                dobjModels[numModels].model = G_GetModel(modelIndexa);
                if ( !dobjModels[numModels].model
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                                629,
                                0,
                                "%s",
                                "dobjModels[numModels].model") )
                {
                    __debugbreak();
                }
                if ( !ent->attachTagNames[i]
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                                630,
                                0,
                                "%s",
                                "ent->attachTagNames[i]") )
                {
                    __debugbreak();
                }
                dobjModels[numModels].boneName = ent->attachTagNames[i];
                dobjModels[numModels++].ignoreCollision = (ent->attachIgnoreCollision & (1 << i)) != 0;
            }
            numModelsa = DestructibleUpdate(ent, dobjModels, numModels);
            dobj = Com_ServerDObjCreate(dobjModels, numModelsa, tree, ent->s.number);
            DObjSetHidePartBits(dobj, ent->s.partBits);
            if ( ent->scr_vehicle )
                G_UpdateVehicleTags(ent);
            G_UpdateTags(ent, 1);
        }
        else
        {
            G_UpdateTags(ent, 0);
        }
    }
}

void __cdecl G_SetModel(gentity_s *ent, char *modelName)
{
    int modelIndex; // [esp+0h] [ebp-4h]

    if ( *modelName )
    {
        modelIndex = G_ModelIndex(modelName);
        if ( modelIndex != (unsigned __int16)modelIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                        783,
                        0,
                        "%s",
                        "modelIndex == (modelNameIndex_t) modelIndex") )
        {
            __debugbreak();
        }
        ent->model = modelIndex;
    }
    else
    {
        ent->model = 0;
    }
}

void __cdecl G_SetModelIfLoaded(gentity_s *ent, const char *modelName)
{
    int i; // [esp+0h] [ebp-10h]
    unsigned int nameString; // [esp+4h] [ebp-Ch]
    int modelIndex; // [esp+Ch] [ebp-4h]

    if ( *modelName )
    {
        nameString = SL_FindLowercaseString(modelName, SCRIPTINSTANCE_SERVER);
        modelIndex = 0;
        for ( i = 1; i < 512; ++i )
        {
            if ( SV_GetConfigstringConst(i + 1568) == nameString )
            {
                if ( !cached_models[i]
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                                807,
                                0,
                                "%s",
                                "cached_models[i]") )
                {
                    __debugbreak();
                }
                modelIndex = i;
            }
        }
        if ( modelIndex != (unsigned __int16)modelIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                        812,
                        0,
                        "%s",
                        "modelIndex == (modelNameIndex_t) modelIndex") )
        {
            __debugbreak();
        }
        ent->model = modelIndex;
    }
    else
    {
        ent->model = 0;
    }
}

void __cdecl G_OverrideModel(unsigned int modelIndex, char *defaultModelName)
{
    unsigned int v2; // eax
    XModel *v3; // eax
    char *modelName; // [esp+8h] [ebp-4h]

    if ( !modelIndex
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 845, 0, "%s", "modelIndex") )
    {
        __debugbreak();
    }
    v2 = G_ModelName(modelIndex);
    modelName = SL_ConvertToString(v2, SCRIPTINSTANCE_SERVER);
    if ( !*modelName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 847, 0, "%s", "modelName[0]") )
    {
        __debugbreak();
    }
    if ( useFastFile->current.enabled )
    {
        DB_ReplaceModel(modelName, defaultModelName);
    }
    else
    {
        v3 = SV_XModelGet(defaultModelName);
        cached_models[modelIndex] = v3;
        Hunk_OverrideDataForFile(5, modelName, v3);
    }
}

int __cdecl G_EntAttach(gentity_s *ent, char *modelName, unsigned int tagName, int ignoreCollision)
{
    int i; // [esp+0h] [ebp-8h]
    int modelIndex; // [esp+4h] [ebp-4h]

    if ( !tagName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 869, 0, "%s", "tagName") )
    {
        __debugbreak();
    }
    if ( G_EntDetach(ent, modelName, tagName)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    872,
                    0,
                    "%s",
                    "!G_EntDetach( ent, modelName, tagName )") )
    {
        __debugbreak();
    }
    for ( i = 0; ; ++i )
    {
        if ( i >= 19 )
            return 0;
        if ( !ent->attachModelNames[i] )
            break;
    }
    modelIndex = G_ModelIndex(modelName);
    if ( !modelIndex )
        return 0;
    if ( modelIndex != (unsigned __int16)modelIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    883,
                    0,
                    "%s",
                    "modelIndex == (modelNameIndex_t) modelIndex") )
    {
        __debugbreak();
    }
    ent->attachModelNames[i] = modelIndex;
    if ( ent->attachTagNames[i]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    885,
                    0,
                    "%s",
                    "!ent->attachTagNames[i]") )
    {
        __debugbreak();
    }
    Scr_SetString(&ent->attachTagNames[i], tagName, SCRIPTINSTANCE_SERVER);
    if ( (ent->attachIgnoreCollision & (1 << i)) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    887,
                    0,
                    "%s",
                    "!(ent->attachIgnoreCollision & (1 << i))") )
    {
        __debugbreak();
    }
    if ( ignoreCollision )
        ent->attachIgnoreCollision |= 1 << i;
    G_DObjUpdate(ent);
    return 1;
}

int __cdecl G_EntDetach(gentity_s *ent, const char *modelName, unsigned int tagName)
{
    unsigned int v4; // edx
    unsigned int modelNameString; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    if ( !tagName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 908, 0, "%s", "tagName") )
    {
        __debugbreak();
    }
    modelNameString = SL_FindLowercaseString(modelName, SCRIPTINSTANCE_SERVER);
    if ( !modelNameString || modelNameString == scr_const._ )
        return 0;
    for ( i = 0; ; ++i )
    {
        if ( i >= 19 )
            return 0;
        if ( ent->attachTagNames[i] == tagName && G_ModelName(ent->attachModelNames[i]) == modelNameString )
            break;
    }
    if ( !ent->attachModelNames[i]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    922,
                    0,
                    "%s",
                    "ent->attachModelNames[i]") )
    {
        __debugbreak();
    }
    ent->attachModelNames[i] = 0;
    Scr_SetString(&ent->attachTagNames[i], 0, SCRIPTINSTANCE_SERVER);
    while ( i < 18 )
    {
        ent->attachModelNames[i] = ent->attachModelNames[i + 1];
        ent->attachTagNames[i] = ent->attachTagNames[i + 1];
        if ( (ent->attachIgnoreCollision & (1 << (i + 1))) != 0 )
            v4 = ent->attachIgnoreCollision | (1 << i);
        else
            v4 = ent->attachIgnoreCollision & ~(1 << i);
        ent->attachIgnoreCollision = v4;
        ++i;
    }
    ent->attachModelNames[i] = 0;
    ent->attachTagNames[i] = 0;
    ent->attachIgnoreCollision &= ~(1 << i);
    G_DObjUpdate(ent);
    return 1;
}

void __cdecl G_EntDetachAll(gentity_s *ent)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 19; ++i )
    {
        ent->attachModelNames[i] = 0;
        Scr_SetString(&ent->attachTagNames[i], 0, SCRIPTINSTANCE_SERVER);
    }
    ent->attachIgnoreCollision = 0;
    G_DObjUpdate(ent);
}

int __cdecl G_EntLinkTo(gentity_s *ent, gentity_s *parent, unsigned int tagName)
{
    if ( !G_EntLinkToInternal(ent, parent, tagName) )
        return 0;
    G_CalcTagAxis(ent, 0);
    return 1;
}

int __cdecl G_EntLinkToInternal(gentity_s *ent, gentity_s *parent, unsigned int tagName)
{
    char *v4; // eax
    int pm_type; // [esp+0h] [ebp-10h]
    char *tagInfo; // [esp+4h] [ebp-Ch]
    gentity_s *checkEnt; // [esp+8h] [ebp-8h]
    int index; // [esp+Ch] [ebp-4h]

    if ( !parent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 981, 0, "%s", "parent") )
    {
        __debugbreak();
    }
    if ( (ent->flags & 0x1000) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    982,
                    0,
                    "%s",
                    "ent->flags & FL_SUPPORTS_LINKTO") )
    {
        __debugbreak();
    }
    G_EntUnlink(ent);
    if ( ent->tagInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 986, 0, "%s", "!ent->tagInfo") )
    {
        __debugbreak();
    }
    if ( tagName )
    {
        if ( !SV_DObjExists(parent) )
            return 0;
        index = SV_DObjGetBoneIndex(parent, tagName);
        if ( index < 0 )
            return 0;
    }
    else
    {
        index = -1;
    }
    for ( checkEnt = parent; ; checkEnt = checkEnt->tagInfo->parent )
    {
        if ( !checkEnt
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 1005, 0, "%s", "checkEnt") )
        {
            __debugbreak();
        }
        if ( checkEnt == ent )
            return 0;
        if ( !checkEnt->tagInfo )
            break;
    }
    tagInfo = MT_Alloc(112, 17, SCRIPTINSTANCE_SERVER);
    *(unsigned int *)tagInfo = (unsigned int)parent;
    *((_WORD *)tagInfo + 4) = 0;
    if ( tagName )
    {
        if ( !SL_IsLowercaseString(tagName, SCRIPTINSTANCE_SERVER) )
        {
            v4 = SL_ConvertToString(tagName, SCRIPTINSTANCE_SERVER);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                            1017,
                            0,
                            "%s\n\t(SL_ConvertToString( tagName )) = %s",
                            "(!tagName || SL_IsLowercaseString( tagName ))",
                            v4) )
                __debugbreak();
        }
    }
    Scr_SetString((unsigned __int16 *)tagInfo + 4, tagName, SCRIPTINSTANCE_SERVER);
    *((unsigned int *)tagInfo + 1) = (unsigned int)parent->tagChildren;
    *((unsigned int *)tagInfo + 3) = index;
    memset(tagInfo + 16, 0, 0x30u);
    parent->tagChildren = ent;
    ent->tagInfo = (tagInfo_s *)tagInfo;
    memset(tagInfo + 64, 0, 0x30u);
    ent->s.clientLinkInfo.parentEnt = 0;
    ent->s.clientLinkInfo.tagIndex = 0;
    if ( ent->client )
    {
        pm_type = ent->client->ps.pm_type;
        if ( pm_type )
        {
            if ( pm_type == 9 )
                ent->client->ps.pm_type = 10;
        }
        else
        {
            ent->client->ps.pm_type = 1;
            if ( parent->scr_vehicle && ent->client->ps.vehicleAnimStage > 0 )
            {
                if ( index + 1 >= 256
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                                1037,
                                0,
                                "%s",
                                "(index+1) < (1<<MAX_BONE_BITS)") )
                {
                    __debugbreak();
                }
                ent->client->ps.vehicleAnimBoneIndex = index + 1;
            }
        }
    }
    G_UpdateClientLinkInfo(ent);
    return 1;
}

int __cdecl G_EntLinkToWithOffset(
                gentity_s *ent,
                gentity_s *parent,
                unsigned int tagName,
                const float *originOffset,
                const float *anglesOffset)
{
    tagInfo_s *tagInfo; // [esp+4h] [ebp-4h]

    if ( !G_EntLinkToInternal(ent, parent, tagName) )
        return 0;
    tagInfo = ent->tagInfo;
    AnglesToAxis(anglesOffset, tagInfo->axis);
    tagInfo->axis[3][0] = *originOffset;
    tagInfo->axis[3][1] = originOffset[1];
    tagInfo->axis[3][2] = originOffset[2];
    return 1;
}

void __cdecl G_EntUnlink(gentity_s *ent)
{
    gclient_s *client; // eax
    int pm_type; // [esp+0h] [ebp-24h]
    tagInfo_s *tagInfo; // [esp+8h] [ebp-1Ch]
    gentity_s *next; // [esp+Ch] [ebp-18h]
    gentity_s *parent; // [esp+10h] [ebp-14h]
    gentity_s *prev; // [esp+14h] [ebp-10h]
    float viewAngles[3]; // [esp+18h] [ebp-Ch] BYREF

    ent->s.clientLinkInfo.parentEnt = 0;
    ent->s.clientLinkInfo.tagIndex = 0;
    tagInfo = ent->tagInfo;
    if ( tagInfo )
    {
        ent->s.lerp.eFlags ^= 2u;
        G_SetOrigin(ent, ent->r.currentOrigin);
        G_SetAngle(ent, ent->r.currentAngles);
        parent = tagInfo->parent;
        if ( !tagInfo->parent
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 1116, 0, "%s", "parent") )
        {
            __debugbreak();
        }
        if ( ent->client )
        {
            client = ent->client;
            viewAngles[0] = client->ps.viewangles[0];
            viewAngles[1] = client->ps.viewangles[1];
            viewAngles[2] = 0.0f;
            SetClientViewAngle(ent, viewAngles);
            ent->r.currentAngles[0] = 0.0f;
        }
        prev = 0;
        next = parent->tagChildren;
        while ( next != ent )
        {
            if ( !next->tagInfo
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                            1131,
                            0,
                            "%s",
                            "next->tagInfo") )
            {
                __debugbreak();
            }
            prev = next;
            next = next->tagInfo->next;
            if ( !next
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 1134, 0, "%s", "next") )
            {
                __debugbreak();
            }
        }
        if ( prev )
            prev->tagInfo->next = tagInfo->next;
        else
            parent->tagChildren = tagInfo->next;
        ent->tagInfo = 0;
        if ( ent->client )
        {
            pm_type = ent->client->ps.pm_type;
            if ( pm_type == 1 )
            {
                ent->client->ps.pm_type = 0;
            }
            else if ( pm_type == 10 )
            {
                ent->client->ps.pm_type = 9;
            }
        }
        Scr_SetString(&tagInfo->name, 0, SCRIPTINSTANCE_SERVER);
        MT_Free((unsigned char*)tagInfo, 112, SCRIPTINSTANCE_SERVER);
        G_UpdateClientLinkInfo(ent);
    }
}

bool __cdecl G_EntIsLinkedTo(gentity_s *ent, gentity_s *parent)
{
    tagInfo_s *tagInfo; // [esp+4h] [ebp-4h]

    tagInfo = ent->tagInfo;
    return tagInfo && tagInfo->parent == parent;
}

void __cdecl G_UpdateTagInfo(gentity_s *ent, int bParentHasDObj)
{
    tagInfo_s *tagInfo; // [esp+0h] [ebp-4h]

    tagInfo = ent->tagInfo;
    if ( !tagInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 1196, 0, "%s", "tagInfo") )
    {
        __debugbreak();
    }
    if ( tagInfo->name )
    {
        if ( !bParentHasDObj
            || (tagInfo->index = SV_DObjGetBoneIndex(tagInfo->parent, tagInfo->name),
                    G_UpdateClientLinkInfo(ent),
                    tagInfo->index < 0) )
        {
            G_EntUnlink(ent);
        }
    }
    else if ( tagInfo->index != -1 )
    {
        tagInfo->index = -1;
        G_UpdateClientLinkInfo(ent);
    }
}

void __cdecl G_UpdateTags(gentity_s *ent, int bHasDObj)
{
    if ( ent->scr_vehicle )
        G_UpdateVehicleTags(ent);
    G_UpdateTagInfoOfChildren(ent, bHasDObj);
}

void __cdecl G_UpdateTagInfoOfChildren(gentity_s *parent, int bHasDObj)
{
    gentity_s *next; // [esp+0h] [ebp-8h]
    gentity_s *ent; // [esp+4h] [ebp-4h]

    for ( ent = parent->tagChildren; ent; ent = next )
    {
        next = ent->tagInfo->next;
        G_UpdateTagInfo(ent, bHasDObj);
    }
}

void __cdecl G_CalcTagParentAxis(gentity_s *ent, float (*parentAxis)[3])
{
    float *currentOrigin; // [esp+4h] [ebp-D0h]
    tagInfo_s *tagInfo; // [esp+70h] [ebp-64h]
    DObj *obj; // [esp+74h] [ebp-60h]
    float tempAxis[4][3]; // [esp+78h] [ebp-5Ch] BYREF
    gentity_s *parent; // [esp+A8h] [ebp-2Ch]
    DObjAnimMat *mat; // [esp+ACh] [ebp-28h]
    float axis[3][3]; // [esp+B0h] [ebp-24h] BYREF

    tagInfo = ent->tagInfo;
    if ( !tagInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 1273, 0, "%s", "tagInfo") )
    {
        __debugbreak();
    }
    parent = tagInfo->parent;
    if ( !parent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 1275, 0, "%s", "parent") )
    {
        __debugbreak();
    }
    if ( ((LODWORD(parent->r.currentOrigin[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(parent->r.currentOrigin[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(parent->r.currentOrigin[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    1277,
                    0,
                    "%s",
                    "!IS_NAN((parent->r.currentOrigin)[0]) && !IS_NAN((parent->r.currentOrigin)[1]) && !IS_NAN((parent->r.currentOrigin)[2])") )
    {
        __debugbreak();
    }
    if ( ((LODWORD(parent->r.currentAngles[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(parent->r.currentAngles[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(parent->r.currentAngles[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    1278,
                    0,
                    "%s",
                    "!IS_NAN((parent->r.currentAngles)[0]) && !IS_NAN((parent->r.currentAngles)[1]) && !IS_NAN((parent->r.currentAngles)[2])") )
    {
        __debugbreak();
    }
    obj = Com_GetServerDObj(parent->s.number);
    if ( tagInfo->index >= 0 && obj )
    {
        AnglesToAxis(parent->r.currentAngles, tempAxis);
        tempAxis[3][0] = parent->r.currentOrigin[0];
        tempAxis[3][1] = parent->r.currentOrigin[1];
        tempAxis[3][2] = parent->r.currentOrigin[2];
        G_DObjCalcBone(parent, tagInfo->index);
        mat = &SV_DObjGetMatrixArray(parent)[tagInfo->index];
        ConvertQuatToMat(mat, axis);
        MatrixMultiply(axis, tempAxis, parentAxis);
        MatrixTransformVector43(mat->trans, tempAxis, &(*parentAxis)[9]);
    }
    else
    {
        AnglesToAxis(parent->r.currentAngles, parentAxis);
        currentOrigin = parent->r.currentOrigin;
        (*parentAxis)[9] = parent->r.currentOrigin[0];
        (*parentAxis)[10] = currentOrigin[1];
        (*parentAxis)[11] = currentOrigin[2];
    }
}

void __cdecl G_CalcTagAxis(gentity_s *ent, int bAnglesOnly)
{
    tagInfo_s *tagInfo; // [esp+8h] [ebp-94h]
    float invParentAxis[4][3]; // [esp+Ch] [ebp-90h] BYREF
    float parentAxis[4][3]; // [esp+3Ch] [ebp-60h] BYREF
    float axis[4][3]; // [esp+6Ch] [ebp-30h] BYREF

    G_CalcTagParentAxis(ent, parentAxis);
    AnglesToAxis(ent->r.currentAngles, axis);
    tagInfo = ent->tagInfo;
    if ( !tagInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 1338, 0, "%s", "tagInfo") )
    {
        __debugbreak();
    }
    if ( bAnglesOnly )
    {
        MatrixTranspose(parentAxis, invParentAxis);
        MatrixMultiply(axis, invParentAxis, tagInfo->axis);
    }
    else
    {
        MatrixInverseOrthogonal43(parentAxis, invParentAxis);
        axis[3][0] = ent->r.currentOrigin[0];
        axis[3][1] = ent->r.currentOrigin[1];
        axis[3][2] = ent->r.currentOrigin[2];
        MatrixMultiply43(axis, invParentAxis, tagInfo->axis);
    }
}

void __cdecl G_SetFixedLink(gentity_s *ent, int eAngles)
{
    tagInfo_s *tagInfo; // [esp+1Ch] [ebp-64h]
    float parentAxis[4][3]; // [esp+20h] [ebp-60h] BYREF
    float axis[4][3]; // [esp+50h] [ebp-30h] BYREF

    G_CalcTagParentAxis(ent, parentAxis);
    tagInfo = ent->tagInfo;
    if ( !tagInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 1368, 0, "%s", "tagInfo") )
    {
        __debugbreak();
    }
    if ( eAngles )
    {
        if ( eAngles == 1 )
        {
            MatrixMultiply43(tagInfo->axis, parentAxis, axis);
            ent->r.currentOrigin[0] = axis[3][0];
            ent->r.currentOrigin[1] = axis[3][1];
            ent->r.currentOrigin[2] = axis[3][2];
            ent->r.currentAngles[1] = vectoyaw(axis[0]);
        }
        else if ( eAngles == 2 )
        {
            MatrixTransformVector43(tagInfo->axis[3], parentAxis, axis[3]);
            ent->r.currentOrigin[0] = axis[3][0];
            ent->r.currentOrigin[1] = axis[3][1];
            ent->r.currentOrigin[2] = axis[3][2];
        }
    }
    else
    {
        MatrixMultiply43(tagInfo->axis, parentAxis, axis);
        ent->r.currentOrigin[0] = axis[3][0];
        ent->r.currentOrigin[1] = axis[3][1];
        ent->r.currentOrigin[2] = axis[3][2];
        AxisToAngles(axis, ent->r.currentAngles);
    }
}

void __cdecl G_UpdateViewAngleClamp(gclient_s *client, const float *worldAnglesCenter)
{
    if ( !client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 1396, 0, "%s", "client") )
    {
        __debugbreak();
    }
    if ( !worldAnglesCenter
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    1397,
                    0,
                    "%s",
                    "worldAnglesCenter") )
    {
        __debugbreak();
    }
    client->ps.viewAngleClampRange[0] = (float)(client->linkAnglesMaxClamp[0] - client->linkAnglesMinClamp[0]) * 0.5;
    client->ps.viewAngleClampBase[0] = *worldAnglesCenter + client->linkAnglesMaxClamp[0];
    client->ps.viewAngleClampBase[0] = AngleNormalize360(client->ps.viewAngleClampBase[0] - client->ps.viewAngleClampRange[0]);
    client->ps.viewAngleClampRange[1] = (float)(client->linkAnglesMaxClamp[1] - client->linkAnglesMinClamp[1]) * 0.5;
    client->ps.viewAngleClampBase[1] = worldAnglesCenter[1] + client->linkAnglesMaxClamp[1];
    client->ps.viewAngleClampBase[1] = AngleNormalize360(client->ps.viewAngleClampBase[1] - client->ps.viewAngleClampRange[1]);
}

void __cdecl G_SetPlayerFixedLink(gentity_s *ent)
{
    float viewHeightCurrent; // [esp+10h] [ebp-164h]
    gclient_s *client; // [esp+38h] [ebp-13Ch]
    float viewQuat[4]; // [esp+3Ch] [ebp-138h] BYREF
    tagInfo_s *tagInfo; // [esp+4Ch] [ebp-128h]
    float worldQuat[4]; // [esp+50h] [ebp-124h] BYREF
    int angleIndex; // [esp+60h] [ebp-114h]
    float localViewOff[3]; // [esp+64h] [ebp-110h] BYREF
    float linkChangeQuat[4]; // [esp+70h] [ebp-104h] BYREF
    float viewMat[3][3]; // [esp+80h] [ebp-F4h] BYREF
    float identQuat[4]; // [esp+A4h] [ebp-D0h] BYREF
    float newViewQuat[4]; // [esp+B4h] [ebp-C0h] BYREF
    float newViewMat[3][3]; // [esp+C4h] [ebp-B0h] BYREF
    float NewViewangles[3]; // [esp+E8h] [ebp-8Ch] BYREF
    float worldAxis[4][3]; // [esp+F4h] [ebp-80h] BYREF
    float worldAngles[3]; // [esp+124h] [ebp-50h] BYREF
    playerState_s *ps; // [esp+130h] [ebp-44h]
    float axis[4][3]; // [esp+134h] [ebp-40h] BYREF
    float angleDiff; // [esp+164h] [ebp-10h]
    float worldViewOff[3]; // [esp+168h] [ebp-Ch] BYREF
    int savedregs; // [esp+174h] [ebp+0h] BYREF

    if ( !ent->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 1442, 0, "%s", "ent->client") )
    {
        __debugbreak();
    }
    client = ent->client;
    tagInfo = ent->tagInfo;
    if ( !tagInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 1446, 0, "%s", "tagInfo") )
    {
        __debugbreak();
    }
    ps = &client->ps;
    if ( (client->ps.eFlags & 0x4000) == 0 || !ps->vehiclePos )
    {
        G_CalcTagParentAxis(ent, worldAxis);
        if ( ent )
        {
            MatrixMultiply43(tagInfo->axis, worldAxis, axis);
            memcpy(worldAxis, axis, sizeof(worldAxis));
            AxisToQuat(worldAxis, worldQuat);
            AxisToAngles(worldAxis, worldAngles);
            if ( ps->vehicleType == 6 || tagInfo->parent && tagInfo->parent->s.eType == 13 )
                G_UpdateViewAngleClamp(client, worldAngles);
            if ( client->prevLinkAnglesSet )
            {
                QuatMultiply(client->prevLinkedInvQuat, worldQuat, linkChangeQuat);
                client->prevLinkedInvQuat[0] = -worldQuat[0];
                client->prevLinkedInvQuat[1] = -worldQuat[1];
                client->prevLinkedInvQuat[2] = -worldQuat[2];
                client->prevLinkedInvQuat[3] = worldQuat[3];
                memset(identQuat, 0, 12);
                identQuat[3] = 1.0f;
                QuatLerp(identQuat, linkChangeQuat, client->linkAnglesFrac, linkChangeQuat);
            }
            else
            {
                client->prevLinkAnglesSet = 1;
                client->prevLinkedInvQuat[0] = -worldQuat[0];
                client->prevLinkedInvQuat[1] = -worldQuat[1];
                client->prevLinkedInvQuat[2] = -worldQuat[2];
                client->prevLinkedInvQuat[3] = worldQuat[3];
                memset(linkChangeQuat, 0, 12);
                linkChangeQuat[3] = 1.0f;
            }
            AnglesToAxis(client->ps.viewangles, viewMat);
            AxisToQuat(viewMat, viewQuat);
            QuatMultiply(viewQuat, linkChangeQuat, newViewQuat);
            QuatToAxis(newViewQuat, newViewMat);
            AxisToAngles(newViewMat, NewViewangles);
            for ( angleIndex = 0; angleIndex < 2; ++angleIndex )
            {
                angleDiff = AngleNormalize180(NewViewangles[angleIndex] - ps->viewangles[angleIndex]);
                ps->delta_angles[angleIndex] = ps->delta_angles[angleIndex] + angleDiff;
                ps->viewangles[angleIndex] = ps->viewangles[angleIndex] + angleDiff;
            }
            client->ps.linkAngles[0] = worldAngles[0];
            client->ps.linkAngles[1] = worldAngles[1];
            client->ps.linkAngles[2] = worldAngles[2];
        }
        else
        {
            AxisToAngles(worldAxis, NewViewangles);
            for ( angleIndex = 0; angleIndex < 3; ++angleIndex )
            {
                angleDiff = AngleNormalize180(NewViewangles[angleIndex] - ps->viewangles[angleIndex]);
                ps->delta_angles[angleIndex] = ps->delta_angles[angleIndex] + angleDiff;
                ps->viewangles[angleIndex] = ps->viewangles[angleIndex] + angleDiff;
            }
        }
        if ( ent->client->link_rotationMovesEyePos )
        {
            viewHeightCurrent = ps->viewHeightCurrent;
            localViewOff[0] = 0.0f;
            localViewOff[1] = 0.0f;
            localViewOff[2] = viewHeightCurrent;
            MatrixTransformVector43(localViewOff, worldAxis, worldViewOff);
            worldViewOff[2] = worldViewOff[2] - ps->viewHeightCurrent;
            G_SetOrigin(ent, worldViewOff);
        }
        else
        {
            G_SetOrigin(ent, worldAxis[3]);
        }
        ent->s.lerp.pos.trType = 1;
        SV_LinkEntity(ent);
    }
}

void __cdecl G_GeneralLink(gentity_s *ent)
{
    int savedregs; // [esp+8h] [ebp+0h] BYREF

    if ( !ent->tagInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 1565, 0, "%s", "ent->tagInfo") )
    {
        __debugbreak();
    }
    G_SetFixedLink(ent, 0);
    G_SetOrigin(ent, ent->r.currentOrigin);
    G_SetAngle(ent, ent->r.currentAngles);
    ent->s.lerp.pos.trType = 1;
    ent->s.lerp.pos.trDelta[0] = 0.0f;
    ent->s.lerp.pos.trDelta[1] = 0.0f;
    ent->s.lerp.pos.trDelta[2] = 0.0f;
    ent->s.lerp.pos.trTime = 0;
    ent->s.lerp.pos.trDuration = 0;
    ent->s.lerp.apos.trType = 1;
    ent->s.lerp.apos.trDelta[0] = 0.0f;
    ent->s.lerp.apos.trDelta[1] = 0.0f;
    ent->s.lerp.apos.trDelta[2] = 0.0f;
    ent->s.lerp.apos.trTime = 0;
    ent->s.lerp.apos.trDuration = 0;
    SV_LinkEntity(ent);
}

void __cdecl G_SafeDObjFree(gentity_s *ent)
{
    Com_SafeServerDObjFree(ent->s.number);
}

void __cdecl G_DObjUpdateServerTime(gentity_s *ent, int bNotify, void (__cdecl *CallbackFunc)())
{
    int iLoop; // [esp+8h] [ebp-314h]
    XAnimServerNotify notifies[64]; // [esp+Ch] [ebp-310h] BYREF
    XAnimServerNotifyList theList; // [esp+310h] [ebp-Ch] BYREF

    if ( bNotify
        && !CallbackFunc
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    1604,
                    0,
                    "%s",
                    "!bNotify || (CallbackFunc != NULL)") )
    {
        __debugbreak();
    }
    theList.pList = notifies;
    theList.iCurrentElement = 0;
    theList.iNumElements = 64;
    DObjClearServerNotifies();
    DObjSetServerNotifies(&theList);
    SV_DObjUpdateServerTime(ent, 0.050000001, bNotify);
    for ( iLoop = 0; iLoop < theList.iCurrentElement; ++iLoop )
    {
        Scr_AddConstString(notifies[iLoop].notetrackName, SCRIPTINSTANCE_SERVER);
        Scr_NotifyNum(notifies[iLoop].entNum, 0, notifies[iLoop].notifyName, 1u);
        CallbackFunc();
    }
    DObjClearServerNotifies();
}

void __cdecl G_DObjCalcPose(gentity_s *ent, int *partBits)
{
    void (__cdecl *controller)(const gentity_s *, int *); // [esp+0h] [ebp-8h]
    DObj *obj; // [esp+4h] [ebp-4h]

    obj = Com_GetServerDObj(ent->s.number);
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 1639, 0, "%s", "obj") )
        __debugbreak();
    if ( !SV_DObjCreateSkelForBones(obj, partBits) )
    {
        controller = entityHandlers[ent->handler].controller;
        if (controller)
            controller(ent, partBits);
        DObjCalcSkel(obj, partBits);
    }
}

void __cdecl G_DObjCalcBone(const gentity_s *ent, int boneIndex)
{
    void (__cdecl *controller)(const gentity_s *, int *); // [esp+0h] [ebp-1Ch]
    DObj *obj; // [esp+4h] [ebp-18h]
    int partBits[5]; // [esp+8h] [ebp-14h] BYREF

    obj = Com_GetServerDObj(ent->s.number);
    if ( obj && !SV_DObjCreateSkelForBone(obj, boneIndex) )
    {
        DObjGetHierarchyBits(obj, boneIndex, partBits);
        controller = entityHandlers[ent->handler].controller;
        if (controller)
            controller(ent, partBits);
        DObjCalcSkel(obj, partBits);
    }
}

DObjAnimMat *__cdecl G_DObjGetLocalTagMatrix(const gentity_s *ent, unsigned int tagName)
{
    int boneIndex; // [esp+0h] [ebp-8h]

    boneIndex = SV_DObjGetBoneIndex(ent, tagName);
    if ( boneIndex < 0 )
        return 0;
    G_DObjCalcBone(ent, boneIndex);
    return &SV_DObjGetMatrixArray(ent)[boneIndex];
}

int __cdecl G_DObjGetWorldTagMatrix(const gentity_s *ent, unsigned int tagName, float (*tagMat)[3])
{
    float ent_axis[4][3]; // [esp+50h] [ebp-58h] BYREF
    DObjAnimMat *mat; // [esp+80h] [ebp-28h]
    float axis[3][3]; // [esp+84h] [ebp-24h] BYREF

    mat = G_DObjGetLocalTagMatrix(ent, tagName);
    if ( !mat )
        return 0;
    AnglesToAxis(ent->r.currentAngles, ent_axis);
    ent_axis[3][0] = ent->r.currentOrigin[0];
    ent_axis[3][1] = ent->r.currentOrigin[1];
    ent_axis[3][2] = ent->r.currentOrigin[2];
    ConvertQuatToMat(mat, axis);
    MatrixMultiply(axis, ent_axis, tagMat);
    MatrixTransformVector43(mat->trans, ent_axis, &(*tagMat)[9]);
    return 1;
}

int __cdecl G_DObjGetWorldTagPos(const gentity_s *ent, unsigned int tagName, float *pos)
{
    float ent_axis[4][3]; // [esp+8h] [ebp-34h] BYREF
    DObjAnimMat *mat; // [esp+38h] [ebp-4h]

    mat = G_DObjGetLocalTagMatrix(ent, tagName);
    if ( !mat )
        return 0;
    AnglesToAxis(ent->r.currentAngles, ent_axis);
    ent_axis[3][0] = ent->r.currentOrigin[0];
    ent_axis[3][1] = ent->r.currentOrigin[1];
    ent_axis[3][2] = ent->r.currentOrigin[2];
    MatrixTransformVector43(mat->trans, ent_axis, pos);
    return 1;
}

void __cdecl G_TraceBulletPathForVehTurret(gentity_s *ent, DObjTrace_s *trace, int gunnerIndex)
{
    float entAxis[4][3]; // [esp+10h] [ebp-B8h] BYREF
    bool dualBarrel; // [esp+43h] [ebp-85h]
    float localStart[3]; // [esp+44h] [ebp-84h] BYREF
    DObj *obj; // [esp+50h] [ebp-78h]
    scr_vehicle_s *veh; // [esp+54h] [ebp-74h]
    float start[3]; // [esp+58h] [ebp-70h] BYREF
    float end[3]; // [esp+64h] [ebp-64h] BYREF
    int flash; // [esp+70h] [ebp-58h]
    float localEnd[3]; // [esp+74h] [ebp-54h] BYREF
    int contentmask; // [esp+80h] [ebp-48h]
    int partBits[5]; // [esp+84h] [ebp-44h] BYREF
    float flashMtx[4][3]; // [esp+98h] [ebp-30h] BYREF

    veh = ent->scr_vehicle;
    dualBarrel = (veh->gunnerTurrets[gunnerIndex].flags & 2) != 0;
    flash = 0;
    if ( dualBarrel )
    {
        if ( veh->gunnerTurrets[gunnerIndex].fireBarrel )
            veh->gunnerTurrets[gunnerIndex].fireBarrel = 0;
        else
            veh->gunnerTurrets[gunnerIndex].fireBarrel = 1;
    }
    if ( veh->gunnerTurrets[gunnerIndex].fireBarrel )
        flash = veh->boneIndex.gunnerTags[gunnerIndex].flash2;
    else
        flash = veh->boneIndex.gunnerTags[gunnerIndex].flash;
    G_DObjGetWorldBoneIndexMatrix(ent, flash, flashMtx);
    *(_QWORD *)start = *(_QWORD *)&flashMtx[3][0];
    start[2] = flashMtx[3][2];
    end[0] = (float)(500.0 * flashMtx[0][0]) + flashMtx[3][0];
    end[1] = (float)(500.0 * flashMtx[0][1]) + flashMtx[3][1];
    end[2] = (float)(500.0 * flashMtx[0][2]) + flashMtx[3][2];
    AnglesToAxis(ent->r.currentAngles, entAxis);
    entAxis[3][0] = ent->r.currentOrigin[0];
    entAxis[3][1] = ent->r.currentOrigin[1];
    entAxis[3][2] = ent->r.currentOrigin[2];
    MatrixTransposeTransformVector43(start, entAxis, localStart);
    MatrixTransposeTransformVector43(end, entAxis, localEnd);
    contentmask = 0x280E833;
    obj = Com_GetServerDObj(ent->s.number);
    DObjGeomTracelinePartBits(obj, contentmask, partBits);
    G_DObjCalcPose(ent, partBits);
    trace->fraction = 1.0f;
    DObjGeomTraceline(obj, localStart, localEnd, contentmask, trace);
}

DObjAnimMat *__cdecl G_DObjGetLocalBoneIndexMatrix(const gentity_s *ent, int boneIndex)
{
    G_DObjCalcBone(ent, boneIndex);
    return &SV_DObjGetMatrixArray(ent)[boneIndex];
}

void __cdecl G_DObjGetWorldBoneIndexPos(const gentity_s *ent, int boneIndex, float *pos)
{
    float ent_axis[4][3]; // [esp+8h] [ebp-34h] BYREF
    DObjAnimMat *mat; // [esp+38h] [ebp-4h]

    mat = G_DObjGetLocalBoneIndexMatrix(ent, boneIndex);
    AnglesToAxis(ent->r.currentAngles, ent_axis);
    ent_axis[3][0] = ent->r.currentOrigin[0];
    ent_axis[3][1] = ent->r.currentOrigin[1];
    ent_axis[3][2] = ent->r.currentOrigin[2];
    MatrixTransformVector43(mat->trans, ent_axis, pos);
}

void __cdecl G_DObjGetWorldBoneIndexMatrix(const gentity_s *ent, int boneIndex, float (*tagMat)[3])
{
    float ent_axis[4][3]; // [esp+50h] [ebp-58h] BYREF
    DObjAnimMat *mat; // [esp+80h] [ebp-28h]
    float axis[3][3]; // [esp+84h] [ebp-24h] BYREF

    mat = G_DObjGetLocalBoneIndexMatrix(ent, boneIndex);
    AnglesToAxis(ent->r.currentAngles, ent_axis);
    ent_axis[3][0] = ent->r.currentOrigin[0];
    ent_axis[3][1] = ent->r.currentOrigin[1];
    ent_axis[3][2] = ent->r.currentOrigin[2];
    ConvertQuatToMat(mat, axis);
    MatrixMultiply(axis, ent_axis, tagMat);
    MatrixTransformVector43(mat->trans, ent_axis, &(*tagMat)[9]);
}

gentity_s *__cdecl G_Find(gentity_s *from, int fieldofs, unsigned __int16 match)
{
    unsigned __int16 s; // [esp+0h] [ebp-4h]
    gentity_s *froma; // [esp+Ch] [ebp+8h]

    if ( from )
        froma = from + 1;
    else
        froma = g_entities;
    while ( froma < &g_entities[level.num_entities] )
    {
        if ( froma->r.inuse )
        {
            s = *(_WORD *)((char *)&froma->s.number + fieldofs);
            if ( s )
            {
                if ( s == match )
                    return froma;
            }
        }
        ++froma;
    }
    return 0;
}

void __cdecl G_InitGentity(gentity_s *e)
{
    e->nextFree = 0;
    e->r.inuse = 1;
    Scr_SetString(&e->classname, scr_const.noclass, SCRIPTINSTANCE_SERVER);
    e->s.number = e - g_entities;

    iassert(!e->r.ownerNum.isDefined());

    e->eventTime = 0;
    e->birthTime = level.time;
    e->freeAfterEvent = 0;
}

void __cdecl G_PrintEntities()
{
    char *v0; // [esp+18h] [ebp-8h]
    int entityIndex; // [esp+1Ch] [ebp-4h]

    for ( entityIndex = 0; entityIndex < level.num_entities; ++entityIndex )
    {
        if ( g_entities[entityIndex].classname )
            v0 = SL_ConvertToString(g_entities[entityIndex].classname, SCRIPTINSTANCE_SERVER);
        else
            v0 = (char *)"";
        Com_Printf(
            15,
            "%4i: '%s', origin: %f %f %f\n",
            entityIndex,
            v0,
            g_entities[entityIndex].r.currentOrigin[0],
            g_entities[entityIndex].r.currentOrigin[1],
            g_entities[entityIndex].r.currentOrigin[2]);
    }
}

gentity_s *__cdecl G_Spawn()
{
    gentity_s *e; // [esp+0h] [ebp-4h]

    e = level.firstFreeEnt;
    if ( G_MaySpawnEntity(level.firstFreeEnt) )
    {
        level.firstFreeEnt = level.firstFreeEnt->nextFree;
        if ( !level.firstFreeEnt )
            level.lastFreeEnt = 0;
        e->nextFree = 0;
    }
    else
    {
        if ( level.num_entities == 1022 )
        {
            G_PrintEntities();
            Com_Error(ERR_DROP, "G_Spawn: no free entities");
        }
        e = &level.gentities[level.num_entities++];
        SV_LocateGameData(level.gentities, level.num_entities, 760, &level.clients->ps, 10720);
    }
    G_InitGentity(e);
    return e;
}

bool __cdecl G_MaySpawnEntity(gentity_s *e)
{
    if ( !e )
        return 0;
    return level.time - e->eventTime >= 500 || level.num_entities >= 1022;
}

gentity_s *__cdecl G_SpawnPlayerClone()
{
    gentity_s *e; // [esp+0h] [ebp-8h]
    int flags; // [esp+4h] [ebp-4h]

    e = &level.gentities[level.currentPlayerClone + 32];
    level.currentPlayerClone = (level.currentPlayerClone + 1) % 4;
    flags = e->s.lerp.eFlags & 2 ^ 2;
    if ( e->r.inuse )
        G_FreeEntity(e);
    G_InitGentity(e);
    AssignToSmallerType<int>(&e->s.lerp.useCount, e->useCount + 1);
    e->s.lerp.eFlags = flags;
    return e;
}

gentity_s *__cdecl G_SpawnActorClone()
{
    gentity_s *e; // [esp+0h] [ebp-8h]
    int flags; // [esp+4h] [ebp-4h]

    e = &level.gentities[level.currentActorClone + 36];
    level.currentActorClone = (level.currentActorClone + 1) % 8;
    flags = e->s.lerp.eFlags & 2 ^ 2;
    if ( e->r.inuse )
        G_FreeEntity(e);
    G_InitGentity(e);
    e->s.lerp.eFlags = flags;
    return e;
}

void __cdecl G_ClearGroundEntity(gentity_s *ent)
{
    if ( (ent->s.eType == 4 || ent->s.eType == 6) && !ent->s.lerp.pos.trType && ent->s.groundEntityNum != 1022 )
    {
        ent->s.lerp.pos.trType = 6;
        ent->s.lerp.pos.trTime = level.time;
        ent->s.lerp.pos.trDuration = 0;
        ent->s.lerp.pos.trBase[0] = ent->r.currentOrigin[0];
        ent->s.lerp.pos.trBase[1] = ent->r.currentOrigin[1];
        ent->s.lerp.pos.trBase[2] = ent->r.currentOrigin[2];
        ent->s.lerp.pos.trDelta[0] = 0.0f;
        ent->s.lerp.pos.trDelta[1] = 0.0f;
        ent->s.lerp.pos.trDelta[2] = 0.0f;
    }
    ent->s.groundEntityNum = 1023;
}

void __cdecl G_ClearGroundEntityRefs(gentity_s *ed)
{
    int i; // [esp+0h] [ebp-8h]
    gentity_s *other; // [esp+4h] [ebp-4h]

    if ( (ed->flags & 0x100000) != 0 )
    {
        for ( i = 0; i < level.num_entities; ++i )
        {
            other = &g_entities[i];
            if ( other->r.inuse )
            {
                if ( other->s.groundEntityNum == ed->s.number )
                    G_ClearGroundEntity(other);
            }
        }
    }
}

void __cdecl G_FreeEntityRefs(gentity_s *ed)
{
    gentity_s *other; // [esp+0h] [ebp-10h]
    gclient_s *pClient; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    int ia; // [esp+8h] [ebp-8h]
    int entnum; // [esp+Ch] [ebp-4h]

    entnum = ed->s.number;
    if ( (ed->flags & 0x400000) != 0 )
    {
        for ( i = 0; i < level.num_entities; ++i )
        {
            other = &g_entities[i];
            if ( other->r.inuse
                && other->r.ownerNum.isDefined()
                && other->r.ownerNum.entnum() == entnum
                && other->s.eType == 11 )
            {
                other->active = 0;
                G_FreeEntityAfterEvent(other);
                break;
            }
        }
    }
    G_ClearGroundEntityRefs(ed);
    if ( (ed->flags & 0x200000) != 0 )
    {
        for ( ia = 0; ia < com_maxclients->current.integer; ++ia )
        {
            if ( g_entities[ia].r.inuse )
            {
                pClient = g_entities[ia].client;
                if ( !pClient
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 2137, 0, "%s", "pClient") )
                {
                    __debugbreak();
                }
                if ( pClient->ps.cursorHintEntIndex == entnum )
                    pClient->ps.cursorHintEntIndex = 1023;
            }
        }
    }
    if ( (0x800000 & ed->flags) != 0 )
        Missile_FreeAttractorRefs(ed);
    if ((ed->flags & 0x1000000) != 0)
        Targ_Remove(ed);
}

void __cdecl G_FreeEntity(gentity_s *ed)
{
    XAnimTree_s *tree; // [esp+0h] [ebp-8h]
    int useCount; // [esp+4h] [ebp-4h]

    G_EntUnlink(ed);
    while ( ed->tagChildren )
        G_EntUnlink(ed->tagChildren);
    SV_UnlinkEntity(ed);
    tree = SV_DObjGetTree(ed);
    if ( tree )
        XAnimClearTree(tree);
    if ( ed->actor )
    {
        Actor_Free(ed->actor);
        if ( ed->actor )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                            2179,
                            0,
                            "%s",
                            "ed->actor == NULL") )
                __debugbreak();
        }
    }
    Com_SafeServerDObjFree(ed->s.number);
    if ( Path_IsDynamicBlockingEntity(ed) )
        Path_ConnectPathsForEntity(ed);
    if ( ed->disconnectedLinks
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    2192,
                    0,
                    "%s",
                    "!ed->disconnectedLinks") )
    {
        __debugbreak();
    }
    if ( ed->sentient )
    {
        Sentient_Free(ed->sentient);
        if ( ed->sentient
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                        2197,
                        0,
                        "%s",
                        "ed->sentient == NULL") )
        {
            __debugbreak();
        }
    }
    else
    {
        G_FreeEntityRefs(ed);
    }
    Actor_EventListener_RemoveEntity(ed->s.number);
    if ( ed->s.eType == 19 )
        ActorCorpse_Free(ed);
    if ( ed->pTurretInfo )
    {
        if ( !ed->pTurretInfo->inuse
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                        2220,
                        0,
                        "%s",
                        "ed->pTurretInfo->inuse") )
        {
            __debugbreak();
        }
        G_FreeTurret(ed);
        if ( ed->pTurretInfo
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                        2222,
                        0,
                        "%s",
                        "ed->pTurretInfo == NULL") )
        {
            __debugbreak();
        }
    }
    if ( ed->scr_vehicle )
    {
        G_FreeVehicle(ed);
        if ( ed->scr_vehicle )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                            2228,
                            0,
                            "%s",
                            "ed->scr_vehicle == NULL") )
                __debugbreak();
        }
    }
    if ( ed->physObjId && ed->physObjId != -1 )
    {
        Sys_EnterCriticalSection(CRITSECT_PHYSICS_UPDATE);
        Phys_ObjDestroy(0, ed->physObjId);
        ed->physObjId = 0;
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS_UPDATE);
    }
    if ( ed->destructible )
        Destructible_FreeDestructible(ed->destructible);
    ed->destructible = 0;
    if ( ed->s.eType == 2 )
        PlayerCorpse_Free(ed);
    SpawnSystem_ClearEntityInfluencers(ed);
    EntHandleDissociate(ed);
    if ( ed->client )
        Com_Printf(14, "G_FreeEntity: Player %i is being freed.\n", ed->s.number);
    ed->r.ownerNum.setEnt(0);
    ed->parent.setEnt(0);
    ed->missileTargetEnt.setEnt(0);
    if ( !ed->r.inuse
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 2258, 0, "%s", "ed->r.inuse") )
    {
        __debugbreak();
    }
    Scr_FreeEntity(ed);
    if ( ed->classname
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    2261,
                    0,
                    "%s",
                    "ed->classname == 0") )
    {
        __debugbreak();
    }
    useCount = ed->useCount;
    memset(ed, 0, sizeof(gentity_s));
    ed->eventTime = level.time;
    if ( ed - level.gentities >= 44 )
    {
        if ( level.lastFreeEnt )
            level.lastFreeEnt->nextFree = ed;
        else
            level.firstFreeEnt = ed;
        level.lastFreeEnt = ed;
        ed->nextFree = 0;
    }
    ed->useCount = useCount + 1;
    if ( ed->r.inuse )
    {
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 2277, 0, "%s", "!ed->r.inuse") )
            __debugbreak();
    }
}

void __cdecl G_FreeEntityDelay(gentity_s *ed)
{
    unsigned __int16 hThread; // [esp+0h] [ebp-4h]

    if ( !g_scr_data.delete_
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    2291,
                    0,
                    "%s",
                    "g_scr_data.delete_") )
    {
        __debugbreak();
    }
    hThread = Scr_ExecEntThread(ed, g_scr_data.delete_, 0);
    Scr_FreeThread(hThread, SCRIPTINSTANCE_SERVER);
}

void __cdecl G_BroadcastEntity(gentity_s *ent)
{
    ent->r.svFlags = 8;
}

void __cdecl G_FreeEntityAfterEvent(gentity_s *ent)
{
    ent->freeAfterEvent = 1;
}

gentity_s *__cdecl G_TempEntity(const float *origin, entity_event_t event)
{
    float snapped[3]; // [esp+0h] [ebp-10h] BYREF
    gentity_s *e; // [esp+Ch] [ebp-4h]
    int savedregs; // [esp+10h] [ebp+0h] BYREF

    e = G_Spawn();
    AssignToSmallerType<entityType_t>(&e->s.eType, ET_EVENTS + event);
    Scr_SetString(&e->classname, scr_const.tempEntity, SCRIPTINSTANCE_SERVER);
    e->eventTime = level.time;
    e->r.eventTime = level.time;
    e->freeAfterEvent = 1;
    snapped[0] = *origin;
    snapped[1] = origin[1];
    snapped[2] = origin[2];
    snapped[0] = (float)(int)snapped[0]; // KISAKTODO: SnapVector()
    snapped[1] = (float)(int)snapped[1];
    snapped[2] = (float)(int)snapped[2];
    G_SetOrigin(e, snapped);
    SV_LinkEntity(e);
    return e;
}

void __cdecl G_AddPredictableEvent(gentity_s *ent, unsigned int event, unsigned int eventParm)
{
    if ( ent->client )
        BG_AddPredictableEventToPlayerstate(event, eventParm, &ent->client->ps);
}

void __cdecl G_AddEvent(gentity_s *ent, unsigned int event, unsigned int eventParm)
{
    if ( !event && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp", 2434, 0, "%s", "event") )
        __debugbreak();
    if ( event >= 0x100
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    2435,
                    0,
                    "event doesn't index 256\n\t%i not in [0, %i)",
                    event,
                    256) )
    {
        __debugbreak();
    }
    if ( eventParm >= 0x7FF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    2436,
                    0,
                    "%s",
                    "eventParm < EVENT_PARM_MAX") )
    {
        __debugbreak();
    }
    if ( ent->s.eType >= 21
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    2437,
                    0,
                    "%s",
                    "ent->s.eType < ET_EVENTS") )
    {
        __debugbreak();
    }
    if ( ent->client )
        BG_AddUnpredictableEventToPlayerstate(event, eventParm, &ent->client->ps);
    else
        BG_AddEntityStateEvent(event, eventParm, &ent->s, level.time);
    ent->eventTime = level.time;
    ent->r.eventTime = level.time;
}

gentity_s *__cdecl G_PlaySoundAliasAtPoint(const float *origin, unsigned int alias)
{
    gentity_s *tmp; // [esp+0h] [ebp-4h]

    tmp = 0;
    if ( alias )
    {
        tmp = G_TempEntity(origin, EV_SOUND_ALIAS);
        tmp->r.svFlags |= 8u;
        tmp->s.loopSoundId = alias;
        tmp->s.otherEntityNum = 1022;
    }
    return tmp;
}

gentity_s *__cdecl G_PlaySoundAlias(gentity_s *ent, unsigned int alias, unsigned int notifyString, unsigned int bone)
{
    gentity_s *tmp; // [esp+0h] [ebp-4h]

    tmp = 0;
    if ( notifyString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    2481,
                    0,
                    "%s",
                    "notifyString == 0") )
    {
        __debugbreak();
    }
    if ( alias )
    {
        tmp = G_TempEntity(ent->r.currentOrigin, EV_SOUND_ALIAS);
        tmp->r.svFlags |= 8u;
        tmp->s.loopSoundId = alias;
        AssignToSmallerType<short>(&tmp->s.otherEntityNum, ent->s.number);
        tmp->s.un3.item = bone;
    }
    return tmp;
}

void __cdecl G_AnimScriptSound(int client, snd_alias_list_t *aliasList)
{
    unsigned int v2; // eax

    v2 = SND_HashAlias(aliasList);
    G_PlaySoundAlias(&g_entities[client], v2, 0, 0);
}

void __cdecl G_SetOrigin(gentity_s *ent, const float *origin)
{
    ent->s.lerp.pos.trBase[0] = *origin;
    ent->s.lerp.pos.trBase[1] = origin[1];
    ent->s.lerp.pos.trBase[2] = origin[2];
    ent->s.lerp.pos.trType = 0;
    ent->s.lerp.pos.trTime = 0;
    ent->s.lerp.pos.trDuration = 0;
    ent->s.lerp.pos.trDelta[0] = 0.0f;
    ent->s.lerp.pos.trDelta[1] = 0.0f;
    ent->s.lerp.pos.trDelta[2] = 0.0f;
    ent->r.currentOrigin[0] = *origin;
    ent->r.currentOrigin[1] = origin[1];
    ent->r.currentOrigin[2] = origin[2];
}

void __cdecl G_SetAngle(gentity_s *ent, const float *angle)
{
    ent->s.lerp.apos.trBase[0] = *angle;
    ent->s.lerp.apos.trBase[1] = angle[1];
    ent->s.lerp.apos.trBase[2] = angle[2];
    ent->s.lerp.apos.trType = 0;
    ent->s.lerp.apos.trTime = 0;
    ent->s.lerp.apos.trDuration = 0;
    ent->s.lerp.apos.trDelta[0] = 0.0f;
    ent->s.lerp.apos.trDelta[1] = 0.0f;
    ent->s.lerp.apos.trDelta[2] = 0.0f;
    ent->r.currentAngles[0] = *angle;
    ent->r.currentAngles[1] = angle[1];
    ent->r.currentAngles[2] = angle[2];
}

void __cdecl G_SetConstString(unsigned __int16 *to, char *from)
{
    Scr_SetString(to, 0, SCRIPTINSTANCE_SERVER);
    *to = SL_GetString(from, 0, SCRIPTINSTANCE_SERVER);
}

const char *__cdecl G_GetEntityTypeName(const gentity_s *ent)
{
    if ( (unsigned int)ent->s.eType >= 0x15
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_utils_mp.cpp",
                    2584,
                    0,
                    "%s",
                    "(unsigned)ent->s.eType < ET_EVENTS") )
    {
        __debugbreak();
    }
    return entityTypeNames[ent->s.eType];
}

unsigned int holdrand = 2309737967u;


void __cdecl G_srand(unsigned int seed)
{
    holdrand = seed;
}

int __cdecl G_rand()
{
    holdrand = 214013 * holdrand + 2531011;
    return holdrand >> 17;
}

double __cdecl G_flrand(float min, float max)
{
    return (double)G_rand() * (max - min) / 32768.0 + min;
}

int __cdecl G_irand(int min, int max)
{
    return ((G_rand() * (__int64)(max - min)) >> 15) + min;
}

float __cdecl G_random()
{
    return (double)G_rand() / 32768.0;
}

double __cdecl G_crandom()
{
    return G_random() * 2.0 - 1.0;
}

