#include "cg_spawn.h"
#include <game/g_load_utils.h>
#include "cg_scr_main.h"
#include <cgame_mp/cg_local_mp.h>
#include <cgame_mp/cg_main_mp.h>
#include <xanim/xmodel.h>
#include <cgame_mp/cg_ents_mp.h>
#include <gfx_d3d/r_shader_constant_set.h>
#include <cgame_mp/cg_snapshot_mp.h>
#include <cgame_mp/cg_animtree_mp.h>
#include <EffectsCore/fx_system.h>
#include <clientscript/cscr_stringlist.h>
#include <clientscript/cscr_vm.h>
#include <qcommon/cm_load.h>
#include <client/splitscreen.h>

const cent_field_t client_spawn_fields[5] =
{
  { "origin", 48, F_VECTOR },
  { "angles", 60, F_VECTOR },
  { "model", 682, F_MODEL },
  { "targetname", 694, F_STRING },
  { NULL, 0, F_INT }
};


int __cdecl CG_SpawnFloat(const char *key, const char *defaultString, float *out)
{
    int present; // [esp+0h] [ebp-8h]
    const char *s; // [esp+4h] [ebp-4h] BYREF

    present = G_SpawnString(&cg_level.spawnVar, key, defaultString, &s);
    *out = atof(s);
    return present;
}

const char *__cdecl CG_GetEntityTypeName(centity_s *cent)
{
    const char *result; // eax

    switch ( cent->nextState.eType )
    {
        case 6:
            result = "SCRIPTMOVER";
            break;
        case 8:
        case 9:
            result = "FX";
            break;
        case 0xA:
            result = "LIGHT";
            break;
        default:
            if ( cent->nextState.loopSoundId )
            {
                result = "SOUND";
            }
            else if ( cent->destructible )
            {
                result = "DESTRUCTABLE";
            }
            else if ( ((*((unsigned int *)cent + 201) >> 15) & 1) != 0 )
            {
                result = "TRIGGER";
            }
            else
            {
                result = "MISC";
            }
            break;
    }
    return result;
}

void __cdecl CG_PrintEntities(unsigned int localClientNum)
{
    const char *EntityTypeName; // eax
    double v2; // [esp+0h] [ebp-38h]
    double v3; // [esp+8h] [ebp-30h]
    double v4; // [esp+10h] [ebp-28h]
    XModel *model; // [esp+28h] [ebp-10h]
    const cgs_t *cgs; // [esp+2Ch] [ebp-Ch]
    fake_centity_s *cent; // [esp+30h] [ebp-8h]
    unsigned int i; // [esp+34h] [ebp-4h]

    if ( localClientNum >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_spawn.cpp",
                    99,
                    0,
                    "localClientNum not in [0, MAX_LOCAL_CLIENTS]\n\t%i not in [%i, %i]",
                    localClientNum,
                    0,
                    1) )
    {
        __debugbreak();
    }
    for ( i = 0; i < 0x200; ++i )
    {
        cent = &cg_fakeEntitiesArray[512 * localClientNum + i];
        if ( cg_fakeEntitiesInuseArray[512 * localClientNum + cent - &cg_fakeEntitiesArray[512 * localClientNum]] )
        {
            v4 = cent->cent.pose.origin[2];
            v3 = cent->cent.pose.origin[1];
            v2 = cent->cent.pose.origin[0];
            EntityTypeName = CG_GetEntityTypeName(&cent->cent);
            Com_Printf(14, "%4i: Type '%s', Org: %6.1f %6.1f %6.1f :", i, EntityTypeName, v2, v3, v4);
            if ( cent->cent.nextState.eType == 6 )
            {
                if ( (cent->cent.nextState.lerp.eFlags & 0x20) != 0 )
                {
                    Com_Printf(14, "\tNo Draw, i.e. ScriptOrigin\n");
                }
                else if (cent->cent.nextState.solid == 0xFFFFFF)
                {
                    Com_Printf(14, "\tSolid BModel");
                }
                else
                {
                    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
                    if ( cent->cent.destructible )
                        model = Destructible_GetDDef(&cent->cent)->model;
                    else
                        model = cgs->gameModels[cent->cent.nextState.index.brushmodel];
                    if ( model )
                        Com_Printf(14, "\t%s", model->name);
                    else
                        Com_Printf(14, "\t%s", "NONE");
                }
            }
            Com_Printf(14, "\n");
        }
        else
        {
            Com_Printf(14, "%4i: Unused.\n", i);
        }
    }
}

centity_s *__cdecl CG_Spawn(unsigned int localClientNum)
{
    fake_centity_s *cent; // [esp+8h] [ebp-8h]
    unsigned int i; // [esp+Ch] [ebp-4h]

    if ( localClientNum >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_spawn.cpp",
                    154,
                    0,
                    "localClientNum not in [0, MAX_LOCAL_CLIENTS]\n\t%i not in [%i, %i]",
                    localClientNum,
                    0,
                    1) )
    {
        __debugbreak();
    }
    for ( i = 0; i < 0x200; ++i )
    {
        cent = &cg_fakeEntitiesArray[512 * localClientNum + i];
        cent->cent.pose.physObjId = 0;
        if ( !cg_fakeEntitiesInuseArray[512 * localClientNum + cent - &cg_fakeEntitiesArray[512 * localClientNum]] )
        {
            CG_SetFakeEntInUse(localClientNum, cent);
            *((unsigned int *)&cent->cent + 201) |= 2u;
            cent->cent.nextState.lerp.useCount = 1;
            cent->cent.nextState.number = i + 1024;
            AssignToSmallerType<unsigned char>(&cent->cent.nextState.clientNum, localClientNum);
            AssignToSmallerType<unsigned char>(&cent->cent.pose.localClientNum, localClientNum);
            CG_UpdatePoseUnion(localClientNum, &cent->cent);
            R_InitShaderConstantSet(&cent->cent.pose.constantSet);
            return &cent->cent;
        }
    }
    CG_PrintEntities(localClientNum);
    if ( !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_spawn.cpp",
                    180,
                    0,
                    "Ran out of fake local centities") )
        __debugbreak();
    return 0;
}

void __cdecl CG_Free(int localClientNum, int entNum)
{
    int eType; // [esp+4h] [ebp-18h]
    centity_s *cent; // [esp+14h] [ebp-8h]
    fake_centity_s *fcent; // [esp+18h] [ebp-4h]

    cent = CG_GetEntity(localClientNum, entNum);
    eType = cent->nextState.eType;
    if ( eType >= 8 && eType <= 9 )
    {
        FX_ThroughWithEffect(localClientNum, cent->pose.fx.effect, 1);
        cent->pose.fx.effect = 0;
    }
    ++cent->currentState.useCount;
    CG_ShutdownEntity(localClientNum, cent, 1);
    CG_SafeDObjFree(localClientNum, entNum);
    fcent = &cg_fakeEntitiesArray[512 * localClientNum - 1024 + entNum];
    if ( !cg_fakeEntitiesInuseArray[512 * localClientNum + fcent - &cg_fakeEntitiesArray[512 * localClientNum]]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_spawn.cpp",
                    206,
                    0,
                    "%s",
                    "CG_FakeEntInUse(localClientNum,fcent)") )
    {
        __debugbreak();
    }
    if ( &fcent->cent != cent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_spawn.cpp", 207, 0, "%s", "&fcent->cent == cent") )
    {
        __debugbreak();
    }
    fcent->flags = 0;
    CG_ClearFakeEntInUse(localClientNum, fcent);
    memset(&fcent->cent, 0, sizeof(fcent->cent));
}

void __cdecl CG_ClearFakeEntInUse(int localClientNum, fake_centity_s *ent)
{
    if ( cg_fakeEntitiesInuseArray[512 * localClientNum + ent - &cg_fakeEntitiesArray[512 * localClientNum]]
        && --cg_fakeEntitiesInuseCount[localClientNum] < 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                    417,
                    0,
                    "%s",
                    "cg_fakeEntitiesInuseCount[localClientNum] >= 0") )
    {
        __debugbreak();
    }
    cg_fakeEntitiesInuseArray[512 * localClientNum + ent - &cg_fakeEntitiesArray[512 * localClientNum]] = 0;
}

void __cdecl CG_ShutdownFakeEntities(int localClientNum)
{
    bool v1; // [esp+4h] [ebp-10h]
    fake_centity_s *cent; // [esp+Ch] [ebp-8h]
    int num; // [esp+10h] [ebp-4h]

    for ( num = 0; num < 512; ++num )
    {
        cent = &cg_fakeEntitiesArray[512 * localClientNum + num];
        v1 = !cg_fakeEntitiesInuseArray[512 * localClientNum + cent - &cg_fakeEntitiesArray[512 * localClientNum]]
            || cent->cent.nextState.number >= 1024;
        if ( !v1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_spawn.cpp",
                        225,
                        0,
                        "%s",
                        "CG_FakeEntInUse(localClientNum,cent) ? cent->cent.nextState.number >= MAX_GENTITIES : true") )
        {
            __debugbreak();
        }
        if ( cg_fakeEntitiesInuseArray[512 * localClientNum + cent - &cg_fakeEntitiesArray[512 * localClientNum]] )
            CG_Free(localClientNum, cent->cent.nextState.number);
    }
    memset(&cg_fakeEntitiesInuseArray[512 * localClientNum], 0, 0x200u);
    cg_fakeEntitiesInuseCount[localClientNum] = 0;
    cg_fakeEntitiesInuseCountFromMap = 0;
    cg_fakeEntitiesInuseCountFromLoadScript = 0;
}

void __cdecl CG_InitFakeEntities(int localClientNum, bool spawnEntsFromMap)
{
    CG_ShutdownFakeEntities(localClientNum);
    if ( spawnEntsFromMap )
    {
        CG_SpawnEntitiesFromString(localClientNum);
        cg_fakeEntitiesInuseCountFromMap = cg_fakeEntitiesInuseCount[localClientNum];
    }
}

int __cdecl CG_GetModelIndex(const char *name, int localClientNum)
{
    int i; // [esp+0h] [ebp-8h]
    const cgs_t *cgs; // [esp+4h] [ebp-4h]

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    for ( i = 0; i < 512; ++i )
    {
        if ( cgs->gameModels[i] && !_stricmp(name, cgs->gameModels[i]->name) )
            return i;
    }
    return -1;
}

void __cdecl CScr_FreeFields(const cent_field_t *fields, unsigned __int8 *base)
{
    while ( fields->name )
    {
        if ( fields->type == F_STRING )
            Scr_SetString((unsigned __int16 *)&base[fields->ofs], 0, SCRIPTINSTANCE_CLIENT);
        ++fields;
    }
}

void __cdecl CScr_FreeEntityFields(centity_s *cent)
{
    if ( cent->nextState.number >= 1024 )
        CScr_FreeFields(client_spawn_fields, (unsigned __int8 *)cent);
}

void __cdecl CScr_SetDynamicEntityField(unsigned __int16 clientNum, centity_s *ent, unsigned int index)
{
    Scr_SetDynamicEntityField(ent->nextState.number, 0, index, SCRIPTINSTANCE_CLIENT, clientNum);
}

void __cdecl CG_InitScriptMover(centity_s *cent)
{
    cent->nextState.lerp.eFlags |= 0x10u;
    cent->nextState.eType = ET_SCRIPTMOVER;
    cent->pose.eType = 6;
    cent->nextState.lerp.pos.trBase[0] = cent->pose.origin[0];
    cent->nextState.lerp.pos.trBase[1] = cent->pose.origin[1];
    cent->nextState.lerp.pos.trBase[2] = cent->pose.origin[2];
    cent->currentState.pos.trBase[0] = cent->pose.origin[0];
    cent->currentState.pos.trBase[1] = cent->pose.origin[1];
    cent->currentState.pos.trBase[2] = cent->pose.origin[2];
    cent->nextState.lerp.pos.trType = 0;
    cent->nextState.lerp.apos.trBase[0] = cent->pose.angles[0];
    cent->nextState.lerp.apos.trBase[1] = cent->pose.angles[1];
    cent->nextState.lerp.apos.trBase[2] = cent->pose.angles[2];
    cent->currentState.apos.trBase[0] = cent->pose.angles[0];
    cent->currentState.apos.trBase[1] = cent->pose.angles[1];
    cent->currentState.apos.trBase[2] = cent->pose.angles[2];
    cent->nextState.lerp.apos.trType = 0;
    *(_QWORD *)cent->nextState.lerp.u.turret.gunAngles = 0;
    *(_QWORD *)cent->nextState.lerp.u.scriptMover.attachModelIndex = 0;
    *((_QWORD *)&cent->nextState.lerp.u.createDynEnt + 2) = 0;
    cent->nextState.lerp.u.player.meleeWeapon = 1023;
}

// local variable allocation has failed, the output may be wrong!
char __cdecl CG_SetTriggerBrushModel(int localClientNum, centity_s *ent)
{
    _BYTE mins[24]; // [esp+8h] [ebp-18h] OVERLAPPED BYREF

    if ( ((*((unsigned int *)ent + 201) >> 1) & 1) == 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_spawn.cpp", 520, 0, "%s", "ent->nextValid") )
    {
        __debugbreak();
    }
    if ( !CM_ClipHandleIsValid(ent->nextState.index.brushmodel) )
        return 0;
    CM_ModelBounds(ent->nextState.index.brushmodel, (float *)mins, (float *)&mins[12]);
    *(_QWORD *)ent->pose.general.mins = *(_QWORD *)mins;
    *(_QWORD *)&ent->pose.general.mins[2] = *(_QWORD *)&mins[8];
    *(_QWORD *)&ent->pose.general.maxs[1] = *(_QWORD *)&mins[16];
    ent->pose.fx.triggerTime = CM_ContentsOfModel(ent->nextState.index.brushmodel);
    ent->nextState.solid = 0xFFFFFF;
    return 1;
}

char __cdecl CG_InitTrigger(int localClientNum, centity_s *self)
{
    if ( CG_SetTriggerBrushModel(localClientNum, self) )
    {
        self->pose.fx.triggerTime = 1079771144;
        self->nextState.lerp.eFlags |= 1u;
        self->currentState.eFlags |= 1u;
        self->currentState.eFlags |= 0x20u;
        self->nextState.lerp.eFlags |= 0x20u;
        return 1;
    }
    else
    {
        Com_PrintError(
            1,
            "Killing trigger at (%f %f %f) because the brush model is invalid.\n",
            self->nextState.lerp.pos.trBase[0],
            self->nextState.lerp.pos.trBase[1],
            self->nextState.lerp.pos.trBase[2]);
        CG_Free(localClientNum, self->nextState.number);
        return 0;
    }
}

void __cdecl CG_InitSentientTrigger(centity_s *self, char spawnflags)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_spawn.cpp", 580, 0, "%s", "self") )
        __debugbreak();
    self->pose.fx.triggerTime = 0;
    if ( (spawnflags & 8) == 0 )
        self->pose.fx.triggerTime |= 0x40000000u;
    if ( (spawnflags & 1) != 0 )
        self->pose.fx.triggerTime |= 0x40000u;
    if ( (spawnflags & 2) != 0 )
        self->pose.fx.triggerTime |= 0x80000u;
    if ( (spawnflags & 4) != 0 )
        self->pose.fx.triggerTime |= 0x100000u;
    if ( (spawnflags & 0x10) != 0 )
        self->pose.fx.triggerTime |= 8u;
    if ( (spawnflags & 0x40) != 0 )
        *((unsigned int *)self + 201) |= 0x20000u;
}

void __cdecl CG_SP_trigger_multiple(int localClientNum, centity_s *ent, char spawnFlags)
{
    CG_InitTriggerWait(localClientNum, ent, (int *)&spawnFlags, 64);
    if ( CG_InitTrigger(localClientNum, ent) )
        CG_InitSentientTrigger(ent, spawnFlags);
}

void __cdecl CG_InitTriggerWait(int localClientNum, centity_s *ent, int *spawnflags, int waitSpawnFlag)
{
    float wait; // [esp+0h] [ebp-4h] BYREF

    if ( cg_level.spawnVar.spawnVarsValid && CG_SpawnFloat("wait", "", &wait) && wait <= 0.0 )
    {
        if ( (waitSpawnFlag & 0x40) != 0 )
            *((unsigned int *)ent + 201) |= 0x20000u;
        else
            *spawnflags |= waitSpawnFlag;
    }
}

void __cdecl CG_SP_trigger_once(int localClientNum, centity_s *ent, char spawnFlags)
{
    char spawnFlagsa; // [esp+10h] [ebp+10h]

    spawnFlagsa = spawnFlags | 0x40;
    if ( CG_InitTrigger(localClientNum, ent) )
        CG_InitSentientTrigger(ent, spawnFlagsa);
}

void __cdecl CG_SP_trigger_radius(int localClientNum, centity_s *ent, char spawnFlags)
{
    const char *v3; // eax
    const char *v4; // eax
    float height; // [esp+18h] [ebp-8h] BYREF
    float radius; // [esp+1Ch] [ebp-4h] BYREF

    if ( !CG_SpawnFloat("radius", "", &radius) )
    {
        v3 = va(
                     "radius not specified for trigger_radius at (%g %g %g)",
                     ent->currentState.pos.trBase[0],
                     ent->currentState.pos.trBase[1],
                     ent->currentState.pos.trBase[2]);
        Com_Error(ERR_DROP, v3);
    }
    if ( !CG_SpawnFloat("height", "", &height) )
    {
        v4 = va(
                     "height not specified for trigger_radius at (%g %g %g)",
                     ent->pose.origin[0],
                     ent->pose.origin[1],
                     ent->pose.origin[2]);
        Com_Error(ERR_DROP, v4);
    }
    if ( CG_InitTrigger(localClientNum, ent) )
    {
        ent->pose.turret.barrelPitch = -radius;
        ent->pose.player.waterHeight = -radius;
        ent->pose.player.nextWaterHeightCheck = 0;
        ent->pose.turret.firingOffset = radius;
        ent->pose.general.maxs[1] = radius;
        ent->pose.general.maxs[2] = height;
        CG_InitTriggerWait(localClientNum, ent, (int *)&spawnFlags, 64);
        CG_InitSentientTrigger(ent, spawnFlags);
    }
}

void __cdecl CG_CallSpawn(int localClientNum, SpawnVar *spawnVar)
{
    float *v2; // [esp+B4h] [ebp-9Ch]
    float v3; // [esp+B8h] [ebp-98h]
    unsigned int *v4; // [esp+C0h] [ebp-90h]
    float *v5; // [esp+C4h] [ebp-8Ch]
    float *v6; // [esp+C8h] [ebp-88h]
    char *out[2]; // [esp+CCh] [ebp-84h] BYREF
    int v8; // [esp+D4h] [ebp-7Ch] BYREF
    int v9; // [esp+D8h] [ebp-78h] BYREF
    int v10; // [esp+DCh] [ebp-74h] BYREF
    float force[3]; // [esp+E0h] [ebp-70h] BYREF
    float fmag; // [esp+ECh] [ebp-64h]
    char *v13; // [esp+F0h] [ebp-60h] BYREF
    const char *forceString; // [esp+F4h] [ebp-5Ch] BYREF
    int v15; // [esp+F8h] [ebp-58h]
    const char *originString; // [esp+FCh] [ebp-54h] BYREF
    int brushmodel; // [esp+100h] [ebp-50h]
    float origin[3]; // [esp+104h] [ebp-4Ch] BYREF
    float mins[3]; // [esp+110h] [ebp-40h] BYREF
    float maxs[3]; // [esp+11Ch] [ebp-34h] BYREF
    const char *modelString; // [esp+128h] [ebp-28h] BYREF
    int modelIndex; // [esp+12Ch] [ebp-24h]
    const char *clientSide; // [esp+130h] [ebp-20h] BYREF
    const char *destructibledefname; // [esp+134h] [ebp-1Ch] BYREF
    bool bClientSide; // [esp+13Bh] [ebp-15h]
    const char *classname; // [esp+13Ch] [ebp-14h] BYREF
    DestructibleDef *ddef; // [esp+140h] [ebp-10h]
    centity_s *ent; // [esp+144h] [ebp-Ch]
    unsigned int flags; // [esp+148h] [ebp-8h]
    const char *spawnFlags; // [esp+14Ch] [ebp-4h] BYREF

    if ( (!spawnVar || !spawnVar->spawnVarsValid)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_spawn.cpp",
                    661,
                    1,
                    "%s",
                    "spawnVar && spawnVar->spawnVarsValid") )
    {
        __debugbreak();
    }
    G_SpawnString(spawnVar, "classname", "", &classname);
    if ( !classname )
    {
        Com_Printf(15, "CG_CallSpawn: NULL classname\n");
        return;
    }
    G_SpawnString(spawnVar, "spawnflags", "", &spawnFlags);
    flags = atoi(spawnFlags);
    bClientSide = (flags & 2) != 0;
    if ( !strcmp("script_model", classname) || !strcmp("script_origin", classname) )
    {
        G_SpawnString(spawnVar, "destructibledef", 0, &destructibledefname);
        if ( destructibledefname )
        {
            ddef = DestructibleDefPrecache(destructibledefname);
            if ( ddef && ddef->clientOnly )
            {
                ent = CG_Spawn(localClientNum);
                CG_ParseEntityFields(localClientNum, ent, spawnVar, 1);
                CG_DestructibleClientOnlyCreate(ent, ddef);
                return;
            }
        }
        else
        {
            G_SpawnString(spawnVar, "clientside", 0, &clientSide);
            if ( bClientSide || clientSide && atof(clientSide) == 1.0 )
            {
                ent = CG_Spawn(localClientNum);
                CG_ParseEntityFields(localClientNum, ent, spawnVar, 0);
                CG_InitScriptMover(ent);
                return;
            }
        }
    }
    if ( strcmp("glass", classname) )
    {
        if ( !CL_LocalClient_IsFirstActive(localClientNum) )
            goto LABEL_55;
        bClientSide = 0;
        if ( (flags & 0x80) == 0 )
            goto LABEL_55;
        if ( !strcmp("trigger_multiple", classname) )
        {
            ent = CG_Spawn(localClientNum);
            CG_ParseEntityFields(localClientNum, ent, spawnVar, 0);
            CG_SP_trigger_multiple(localClientNum, ent, flags);
            *((unsigned int *)ent + 201) |= 0x8000u;
            return;
        }
        if ( !strcmp("trigger_radius", classname) )
        {
            ent = CG_Spawn(localClientNum);
            CG_ParseEntityFields(localClientNum, ent, spawnVar, 0);
            CG_SP_trigger_radius(localClientNum, ent, flags);
            *((unsigned int *)ent + 201) |= 0x8000u;
            *((unsigned int *)ent + 201) |= 0x10000u;
            return;
        }
        if ( !strcmp("trigger_once", classname) )
        {
            ent = CG_Spawn(localClientNum);
            CG_ParseEntityFields(localClientNum, ent, spawnVar, 0);
            CG_SP_trigger_once(localClientNum, ent, flags);
            *((unsigned int *)ent + 201) |= 0x8000u;
        }
        else
        {
LABEL_55:
            if ( !strcmp("heli_height_lock", classname) )
            {
                G_SpawnHeliHeightLock(spawnVar);
            }
            else
            {
                if ( num_occlusion_overrides < 32 && !strcmp("occlusion_override", classname) )
                {
                    G_SpawnString(spawnVar, "origin", 0, &originString);
                    G_SpawnString(spawnVar, "model", 0, &modelString);
                    if ( *modelString != 42
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_spawn.cpp",
                                    778,
                                    0,
                                    "%s",
                                    "modelString[0] == '*'") )
                    {
                        __debugbreak();
                    }
                    modelIndex = atoi(modelString + 1);
                    if ( modelIndex != (unsigned __int16)modelIndex
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_spawn.cpp",
                                    780,
                                    0,
                                    "%s",
                                    "modelIndex == (modelNameIndex_t)modelIndex") )
                    {
                        __debugbreak();
                    }
                    brushmodel = (unsigned __int16)modelIndex;
                    memset(origin, 0, sizeof(origin));
                    sscanf(originString, "%f %f %f", origin, &origin[1], &origin[2]);
                    CM_ModelBounds(brushmodel, mins, maxs);
                    v6 = occlusion_overrides[num_occlusion_overrides][0];
                    *v6 = origin[0] + mins[0];
                    v6[1] = origin[1] + mins[1];
                    v6[2] = origin[2] + mins[2];
                    v5 = occlusion_overrides[num_occlusion_overrides][1];
                    *v5 = origin[0] + maxs[0];
                    v5[1] = origin[1] + maxs[1];
                    v5[2] = origin[2] + maxs[2];
                    ++num_occlusion_overrides;
                }
                if ( !strcmp("undertow_volume", classname) )
                {
                    if ( num_undertow_volumes >= 32
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_spawn.cpp",
                                    797,
                                    0,
                                    "%s",
                                    "num_undertow_volumes < MAX_UNDERTOW_VOLUMES") )
                    {
                        __debugbreak();
                    }
                    if ( num_undertow_volumes < 32 )
                    {
                        G_SpawnString(spawnVar, "origin", 0, (const char **)out);
                        G_SpawnString(spawnVar, "model", 0, (const char **)&v13);
                        G_SpawnString(spawnVar, "force", 0, &forceString);
                        if ( *v13 != 42
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_spawn.cpp",
                                        806,
                                        0,
                                        "%s",
                                        "modelString[0] == '*'") )
                        {
                            __debugbreak();
                        }
                        v15 = atoi(v13 + 1);
                        if ( v15 != (unsigned __int16)v15
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_spawn.cpp",
                                        808,
                                        0,
                                        "%s",
                                        "modelIndex == (modelNameIndex_t)modelIndex") )
                        {
                            __debugbreak();
                        }
                        out[1] = (char *)(unsigned __int16)v15;
                        undertow_volumes[num_undertow_volumes] = (unsigned __int16)v15;
                        v8 = 0;
                        v9 = 0;
                        v10 = 0;
                        sscanf(out[0], "%f %f %f", &v8, &v9, &v10);
                        v4 = (unsigned int*)undertow_origins[num_undertow_volumes];
                        *v4 = v8;
                        v4[1] = v9;
                        v4[2] = v10;
                        force[0] = 0.0f;
                        force[1] = 0.0f;
                        force[2] = -200.0f;
                        if ( forceString )
                        {
                            if ( *forceString )
                                sscanf(forceString, "%f %f %f", force, &force[1], &force[2]);
                        }
                        fmag = Vec3Length(force);
                        v2 = undertow_forces[num_undertow_volumes];
                        v3 = 1.0 / fmag;
                        *v2 = (float)(1.0 / fmag) * force[0];
                        v2[1] = v3 * force[1];
                        v2[2] = v3 * force[2];
                        undertow_forces[num_undertow_volumes++][3] = fmag;
                    }
                }
            }
        }
    }
}

void __cdecl CG_ParseEntityFields(int localClientNum, centity_s *ent, SpawnVar *spawnVar, int ignoreModel)
{
    int i; // [esp+0h] [ebp-4h]

    if ( !spawnVar->spawnVarsValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_spawn.cpp",
                    482,
                    0,
                    "%s",
                    "spawnVar.spawnVarsValid") )
    {
        __debugbreak();
    }
    for ( i = 0; i < spawnVar->numSpawnVars; ++i )
        CG_ParseEntityField(localClientNum, spawnVar->spawnVars[i][0], spawnVar->spawnVars[i][1], ent, ignoreModel);
}

void __cdecl CG_ParseEntityField(int localClientNum, const char *key, char *value, centity_s *ent, int ignoreModel)
{
    float v5; // [esp+0h] [ebp-20h]
    int modelIndex; // [esp+8h] [ebp-18h]
    int modelIndexa; // [esp+8h] [ebp-18h]
    const cent_field_t *f; // [esp+10h] [ebp-10h]
    float vec[3]; // [esp+14h] [ebp-Ch] BYREF

    for ( f = client_spawn_fields; ; ++f )
    {
        if ( !f->name )
        {
            CG_SetEntityScriptVariable(localClientNum, key, value, ent);
            return;
        }
        if ( !I_stricmp(f->name, key) )
            break;
    }
    switch ( f->type )
    {
        case F_INT:
            *(unsigned int *)((char *)&ent->pose.lightingHandle + f->ofs) = atoi(value);
            break;
        case F_SHORT:
            *(unsigned __int16 *)((char *)&ent->pose.lightingHandle + f->ofs) = atoi(value);
            break;
        case F_BYTE:
            *((_BYTE *)&ent->pose.lightingHandle + f->ofs) = atoi(value);
            break;
        case F_FLOAT:
            v5 = atof(value);
            *(float *)((char *)&ent->pose.lightingHandle + f->ofs) = v5;
            break;
        case F_STRING:
            Scr_SetStringLiveUpdateSafe(
                (unsigned __int16 *)((char *)&ent->pose.lightingHandle + f->ofs),
                value,
                SCRIPTINSTANCE_CLIENT);
            break;
        case F_VECTOR:
            memset(vec, 0, sizeof(vec));
            sscanf(value, "%f %f %f", vec, &vec[1], &vec[2]);
            *(float *)((char *)&ent->pose.lightingHandle + f->ofs) = vec[0];
            *(float *)(&ent->pose.localClientNum + f->ofs) = vec[1];
            *(float *)((char *)&ent->pose.cullIn + f->ofs) = vec[2];
            break;
        case F_MODEL:
            if ( !ignoreModel )
            {
                if ( *value == 42 )
                {
                    modelIndex = atoi(value + 1);
                    if ( modelIndex != (unsigned __int16)modelIndex
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_spawn.cpp",
                                    452,
                                    0,
                                    "%s",
                                    "modelIndex == (modelNameIndex_t)modelIndex") )
                    {
                        __debugbreak();
                    }
                    ent->nextState.index.brushmodel = modelIndex;
                }
                else
                {
                    modelIndexa = CG_GetModelIndex(value, localClientNum);
                    if ( modelIndexa < 0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_spawn.cpp",
                                    459,
                                    0,
                                    "%s",
                                    "modelIndex >= 0") )
                    {
                        __debugbreak();
                    }
                    if ( modelIndexa != (unsigned __int16)modelIndexa
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_spawn.cpp",
                                    460,
                                    0,
                                    "%s",
                                    "modelIndex == (modelNameIndex_t)modelIndex") )
                    {
                        __debugbreak();
                    }
                    ent->nextState.index.brushmodel = modelIndexa;
                    *((unsigned int *)ent + 201) |= 2u;
                }
            }
            break;
        default:
            return;
    }
}

void __cdecl CG_SetEntityScriptVariable(unsigned __int16 clientNum, const char *key, char *value, centity_s *ent)
{
    unsigned int index; // [esp+0h] [ebp-4h]

    index = CG_SetEntityScriptVariableInternal(key, value);
    if ( index )
        CScr_SetDynamicEntityField(clientNum, ent, index);
}

void __cdecl CG_SpawnEntitiesFromString(int localClientNum)
{
    num_occlusion_overrides = 0;
    num_undertow_volumes = 0;
    G_ResetEntityParsePoint();
    if ( G_ParseSpawnVars(&cg_level.spawnVar) )
    {
        while ( G_ParseSpawnVars(&cg_level.spawnVar) )
            CG_CallSpawn(localClientNum, &cg_level.spawnVar);
        G_ResetEntityParsePoint();
    }
}

