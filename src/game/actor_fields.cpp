#include "actor_fields.h"
#include <clientscript/cscr_vm.h>
#include <game_mp/g_utils_mp.h>
#include <clientscript/cscr_stringlist.h>
#include <clientscript/scr_const.h>
#include <game_mp/actor_mp.h>
#include "g_weapon.h"
#include <universal/surfaceflags.h>
#include <game_mp/g_cmds_mp.h>
#include <game_mp/g_main_mp.h>
#include <game_mp/g_spawn_mp.h>
#include <flame/flame_system.h>
#include <bgame/bg_weapons_def.h>
#include <qcommon/common.h>

void __cdecl AIFIELD_NULLSUB(actor_s *pSelf, const actor_fields_s *pField)
{

}

const actor_fields_s sentientfields[12] =
{
  { "threatbias", 12, { 4 }, F_INT, NULL, NULL },
  { "node", 96, { 4 }, F_PATHNODE, &ActorScr_ReadOnly, NULL },
  { "prevnode", 100, { 4 }, F_PATHNODE, &ActorScr_ReadOnly, NULL },
  { "enemy", 56, { 4 }, F_ENTHANDLE, &ActorScr_ReadOnly, NULL },
  { "scriptenemy", 60, { 4 }, F_ENTHANDLE, &ActorScr_ReadOnly, NULL },
  { "scriptenemytag", 64, { 2 }, F_STRING, &ActorScr_ReadOnly, NULL },
  { "syncedmeleetarget", 52, { 4 }, F_ENTHANDLE, NULL, NULL },
  { "ignoreme", 20, { 1 }, F_BYTE, NULL, NULL },
  { "ignoreall", 21, { 1 }, F_BYTE, NULL, NULL },
  { "maxvisibledist", 36, { 4 }, F_FLOAT, NULL, NULL },
  { "ignoreforfriendlyfire", 22, { 1 }, F_BYTE, NULL, NULL },
  { NULL, 0, { 0 }, F_INT, NULL, NULL }
};

const actor_fields_s entfields[7] =
{
  { "health", 404, { 4 }, F_INT, NULL, NULL },
  { "maxhealth", 408, { 4 }, F_INT, NULL, NULL },
  { "targetname", 360, { 2 }, F_STRING, NULL, NULL },
  { "classname", 356, { 2 }, F_STRING, &ActorScr_ReadOnly, NULL },
  { "spawnflags", 368, { 4 }, F_INT, NULL, NULL },
  { "model", 348, { 2 }, F_MODEL, &ActorScr_ReadOnly, NULL },
  { NULL, 0, { 0 }, F_INT, NULL, NULL }
};

actor_fields_s aifield_delete =
{ NULL, 0, { 0 }, F_INT, NULL, NULL };

const actor_fields_s aifields[80] =
{
  { "type", 8, { 4 }, F_INT, ActorScr_SetSpecies, ActorScr_GetSpecies },
  { "isdog", 8, { 4 }, F_INT, ActorScr_ReadOnly, ActorScr_GetIsDog },
  { "accuracy", 468, { 4 }, F_FLOAT, ActorScr_Clamp_0_Positive, NULL },
  { "lookforward", 248, { 12 }, F_VECTOR, ActorScr_ReadOnly, NULL },
  { "lookright", 260, { 12 }, F_VECTOR, ActorScr_ReadOnly, NULL },
  { "lookup", 272, { 12 }, F_VECTOR, ActorScr_ReadOnly, NULL },
  { "fovcosine", 3860, { 4 }, F_FLOAT, ActorScr_Clamp_0_1, NULL },
  { "maxsightdistsqrd", 3864, { 4 }, F_FLOAT, NULL, NULL },
  { "ignoreclosefoliage", 3868, { 4 }, F_INT, NULL, NULL },
  { "interval", 3576, { 4 }, F_FLOAT, NULL, NULL },
  { "damagetaken", 496, { 4 }, F_INT, ActorScr_ReadOnly, NULL },
  { "damagedir", 504, { 12 }, F_VECTOR, ActorScr_ReadOnly, NULL },
  { "damageyaw", 500, { 4 }, F_INT, ActorScr_ReadOnly, NULL },
  { "damagelocation", 516, { 2 }, F_STRING, ActorScr_ReadOnly, NULL },
  { "damageweapon", 518, { 2 }, F_STRING, ActorScr_ReadOnly, NULL },
  { "damagemod", 520, { 2 }, F_STRING, ActorScr_ReadOnly, NULL },
  { "proneok", 332, { 4 }, F_INT, ActorScr_ReadOnly, NULL },
  { "walkdist", 3564, { 4 }, F_FLOAT, NULL, NULL },
  { "desiredangle", 316, { 4 }, F_FLOAT, ActorScr_ReadOnly, NULL },
  { "pacifist", 3768, { 4 }, F_INT, NULL, NULL },
  { "pacifistwait", 3772, { 4 }, F_INT, ActorScr_SetTime, ActorScr_GetTime },
  { "ignoresuppression", 5840, { 4 }, F_INT, NULL, NULL },
  { "suppressionwait", 5844, { 4 }, F_INT, NULL, NULL },
  { "suppressionduration", 5848, { 4 }, F_INT, NULL, NULL },
  { "suppressionstarttime", 5852, { 4 }, F_INT, ActorScr_ReadOnly, NULL },
  { "suppressionmeter", 5856, { 4 }, F_FLOAT, ActorScr_ReadOnly, NULL },
  { "aiweapon", 236, { 2 }, F_STRING, NULL, NULL },
  { "dontavoidplayer", 3600, { 4 }, F_INT, NULL, NULL },
  { "grenadeawareness", 5880, { 4 }, F_FLOAT, ActorScr_Clamp_0_1, NULL },
  { "grenade", 5888, { 4 }, F_ENTHANDLE, ActorScr_ReadOnly, NULL },
  {
    "grenadeweapon",
    5892,
    { 4 },
    F_INT,
    ActorScr_SetWeapon,
    ActorScr_GetWeapon
  },
  { "grenadeammo", 5908, { 4 }, F_INT, NULL, NULL },
  { "grenadethrowback", 5884, { 4 }, F_INT, NULL, NULL },
  { "favoriteenemy", 5792, { 4 }, F_SENTIENTHANDLE, NULL, NULL },
  { "allowpain", 228, { 1 }, F_BYTE, NULL, NULL },
  { "allowdeath", 229, { 1 }, F_BYTE, NULL, NULL },
  { "delayeddeath", 230, { 1 }, F_BYTE, NULL, NULL },
  { "providecoveringfire", 231, { 1 }, F_BYTE, NULL, NULL },
  { "ignoretriggers", 5960, { 1 }, F_BYTE, NULL, NULL },
  { "pushable", 5961, { 1 }, F_BYTE, NULL, NULL },
  { "dropweapon", 5948, { 4 }, F_INT, NULL, NULL },
  { "drawoncompass", 5952, { 4 }, F_INT, NULL, NULL },
  { "activatecrosshair", 5956, { 4 }, F_INT, NULL, NULL },
  {
    "groundtype",
    604,
    { 4 },
    F_STRING,
    ActorScr_ReadOnly,
    ActorScr_GetGroundType
  },
  { "scriptstate", 5992, { 2 }, F_STRING, ActorScr_ReadOnly, NULL },
  { "lastscriptstate", 5994, { 2 }, F_STRING, ActorScr_ReadOnly, NULL },
  { "statechangereason", 5996, { 2 }, F_STRING, ActorScr_ReadOnly, NULL },
  { "goalradius", 3716, { 4 }, F_FLOAT, ActorScr_SetGoalRadius, NULL },
  { "goalheight", 3720, { 4 }, F_FLOAT, ActorScr_SetGoalHeight, NULL },
  { "goalpos", 3648, { 12 }, F_VECTOR, ActorScr_ReadOnly, NULL },
  { "ignoreforfixednodesafecheck", 3752, { 1 }, F_BYTE, NULL, NULL },
  { "fixednode", 3753, { 1 }, F_BYTE, ActorScr_SetFixedNode, NULL },
  {
    "fixednodesaferadius",
    3756,
    { 4 },
    F_FLOAT,
    ActorScr_Clamp_0_Positive,
    NULL
  },
  {
    "pathgoalpos",
    3472,
    { 12 },
    F_VECTOR,
    ActorScr_ReadOnly,
    ActorScr_GetPathGoalPos
  },
  { "stopanimdistsq", 3552, { 4 }, F_FLOAT, NULL, NULL },
  {
    "lastenemysightpos",
    5800,
    { 12 },
    F_VECTOR,
    ActorScr_SetLastEnemySightPos,
    ActorScr_GetLastEnemySightPos
  },
  { "pathenemylookahead", 3736, { 4 }, F_FLOAT, NULL, NULL },
  { "pathenemyfightdist", 3740, { 4 }, F_FLOAT, NULL, NULL },
  { "meleeattackdist", 3744, { 4 }, F_FLOAT, NULL, NULL },
  {
    "movemode",
    548,
    { 1 },
    F_STRING,
    ActorScr_ReadOnly,
    ActorScr_GetMoveMode
  },
  { "safetochangescript", 549, { 1 }, F_BYTE, NULL, NULL },
  { "keepclaimednode", 3608, { 1 }, F_BYTE, NULL, NULL },
  { "keepclaimednodeingoal", 3609, { 1 }, F_BYTE, NULL, NULL },
  { "nododgemove", 3610, { 1 }, F_BYTE, NULL, NULL },
  { "leanamount", 3624, { 4 }, F_FLOAT, NULL, NULL },
  { "isfacingmotion", 3628, { 1 }, F_BYTE, ActorScr_ReadOnly, NULL },
  { "badplaceawareness", 5972, { 4 }, F_FLOAT, ActorScr_Clamp_0_1, NULL },
  { "goodshootpos", 5976, { 12 }, F_VECTOR, NULL, NULL },
  { "goodshootposvalid", 5988, { 4 }, F_INT, NULL, NULL },
  { "flashbangimmunity", 6100, { 4 }, F_INT, NULL, NULL },
  { "lookaheaddir", 3484, { 12 }, F_VECTOR, ActorScr_ReadOnly, NULL },
  { "lookaheaddist", 3504, { 4 }, F_FLOAT, ActorScr_ReadOnly, NULL },
  { "exposedduration", 3644, { 4 }, F_INT, NULL, NULL },
  { "requestarrivalnotify", 3828, { 4 }, F_INT, NULL, NULL },
  { "finalaccuracy", 484, { 4 }, F_FLOAT, ActorScr_ReadOnly, NULL },
  { "weaponaccuracy", 488, { 4 }, F_FLOAT, ActorScr_ReadOnly, NULL },
  { "goalangle", 3704, { 12 }, F_VECTOR, ActorScr_ReadOnly, NULL },
  { "ikpriority", 10108, { 4 }, F_INT, AIFIELD_NULLSUB, NULL },
  { "animtranslationscale", 560, { 4 }, F_FLOAT, NULL, NULL },
  { NULL, 0, { 0 }, F_INT, NULL, NULL }
};

actor_fields_s aifield_list =
{ NULL, 0, { 0 }, F_INT, NULL, NULL };

const unsigned __int16 *g_AISpeciesNames[1] = { (const unsigned __int16 *)&scr_const.dog };

void __cdecl ActorScr_SetSpecies(actor_s *pSelf, const actor_fields_s *pField)
{
    char *v1; // eax
    const char *v2; // eax
    AISpecies i; // [esp+0h] [ebp-8h]
    unsigned int type; // [esp+4h] [ebp-4h]

    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 304, 0, "%s", "pSelf") )
        __debugbreak();
    type = Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    for ( i = AI_SPECIES_DOG; i < MAX_AI_SPECIES; ++i )
    {
        if ( type == *g_AISpeciesNames[i] )
        {
            pSelf->species = i;
            pSelf->ent->s.lerp.u.loopFx.period = (unsigned __int8)i;
            G_DObjUpdate(pSelf->ent);
            return;
        }
    }
    v1 = SL_ConvertToString(type, SCRIPTINSTANCE_SERVER);
    v2 = va("unknown type '%s', should be human, dog, zombie, zombie_dog\n", v1);
    Scr_Error(v2, 0);
}

void __cdecl ActorScr_GetSpecies(actor_s *pSelf, const actor_fields_s *pField)
{
    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 330, 0, "%s", "pSelf") )
        __debugbreak();
    if ( pSelf->species
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    331,
                    0,
                    "pSelf->species doesn't index MAX_AI_SPECIES\n\t%i not in [0, %i)",
                    pSelf->species,
                    1) )
    {
        __debugbreak();
    }
    Scr_AddConstString(*g_AISpeciesNames[pSelf->species], SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorScr_GetIsDog(actor_s *pSelf, const actor_fields_s *pField)
{
    unsigned __int8 LocalClientSourceRange; // al

    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 343, 0, "%s", "pSelf") )
        __debugbreak();
    if ( pSelf->species
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    344,
                    0,
                    "pSelf->species doesn't index MAX_AI_SPECIES\n\t%i not in [0, %i)",
                    pSelf->species,
                    1) )
    {
        __debugbreak();
    }
    LocalClientSourceRange = Flame_GetLocalClientSourceRange();
    Scr_AddBool(LocalClientSourceRange, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorScr_Clamp_0_1(actor_s *pSelf, const actor_fields_s *pField)
{
    const char *v2; // eax
    const char *v3; // eax
    float fValue; // [esp+Ch] [ebp-4h]

    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 370, 0, "%s", "pSelf") )
        __debugbreak();
    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 375, 0, "%s", "pField") )
        __debugbreak();
    fValue = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    if ( fValue <= 1.0 )
    {
        if ( fValue < 0.0 )
        {
            v3 = va("actor field %s clamped from %g to 0\n", pField->name, fValue);
            Scr_Error(v3, 0);
            fValue = 0.0f;
        }
    }
    else
    {
        v2 = va("actor field %s clamped from %g to 1\n", pField->name, fValue);
        Scr_Error(v2, 0);
        fValue = 1.0f;
    }
    *(float *)((char *)&pSelf->ent + pField->ofs) = fValue;
}

void __cdecl ActorScr_Clamp_0_Positive(actor_s *pSelf, const actor_fields_s *pField)
{
    const char *v2; // eax
    float value; // [esp+Ch] [ebp-4h]

    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 403, 0, "%s", "pSelf") )
        __debugbreak();
    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 408, 0, "%s", "pField") )
        __debugbreak();
    value = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    if ( value < 0.0 )
    {
        v2 = va("actor field %s clamped from %g to 0\n", pField->name, value);
        Scr_Error(v2, 0);
        value = 0.0f;
    }
    *(float *)((char *)&pSelf->ent + pField->ofs) = value;
}

void __cdecl ActorScr_ReadOnly(actor_s *pSelf, const actor_fields_s *pField)
{
    const char *v2; // eax

    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 431, 0, "%s", "pSelf") )
        __debugbreak();
    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 434, 0, "%s", "pField") )
        __debugbreak();
    v2 = va("actor field %s is read-only", pField->name);
    Scr_Error(v2, 0);
}

void __cdecl ActorScr_SetGoalRadius(actor_s *pSelf, const actor_fields_s *pField)
{
    float radius; // [esp+4h] [ebp-4h]

    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 451, 0, "%s", "pSelf") )
        __debugbreak();
    radius = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    if ( radius < 0.0 )
        Scr_ParamError(0, "radius must be >= 0", SCRIPTINSTANCE_SERVER);
    Actor_SetGoalRadius(&pSelf->scriptGoal, radius);
}

void __cdecl ActorScr_SetGoalHeight(actor_s *pSelf, const actor_fields_s *pField)
{
    float height; // [esp+0h] [ebp-4h]

    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 469, 0, "%s", "pSelf") )
        __debugbreak();
    height = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    Actor_SetGoalHeight(&pSelf->scriptGoal, height);
}

void __cdecl ActorScr_SetTime(actor_s *pSelf, const actor_fields_s *pField)
{
    float v2; // [esp+4h] [ebp-10h]

    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 483, 0, "%s", "pSelf") )
        __debugbreak();
    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 486, 0, "%s", "pField") )
        __debugbreak();
    if ( pField->type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    488,
                    0,
                    "%s",
                    "pField->type == F_INT") )
    {
        __debugbreak();
    }
    v2 = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER) * 1000.0;
    *(gentity_s **)((char *)&pSelf->ent + pField->ofs) = (gentity_s *)SnapFloatToInt(v2);
}

void __cdecl ActorScr_GetTime(actor_s *pSelf, const actor_fields_s *pField)
{
    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 501, 0, "%s", "pField") )
        __debugbreak();
    if ( pField->type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    502,
                    0,
                    "%s",
                    "pField->type == F_INT") )
    {
        __debugbreak();
    }
    Scr_AddFloat((float)*(int *)((char *)&pSelf->ent + pField->ofs) * 0.001, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorScr_SetWeapon(actor_s *pSelf, const actor_fields_s *pField)
{
    const char *v2; // eax
    char *String; // eax
    char *weapName; // [esp+0h] [ebp-8h]

    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 517, 0, "%s", "pSelf") )
        __debugbreak();
    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 520, 0, "%s", "pField") )
        __debugbreak();
    if ( pField->type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    522,
                    0,
                    "%s",
                    "pField->type == F_INT") )
    {
        __debugbreak();
    }
    weapName = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    if ( !G_GetWeaponIndexForName(weapName) )
    {
        v2 = va("Can't find weapon [%s].    It probably needs to be precached.", weapName);
        Scr_ParamError(0, v2, SCRIPTINSTANCE_SERVER);
    }
    String = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    *(gentity_s **)((char *)&pSelf->ent + pField->ofs) = (gentity_s *)G_GetWeaponIndexForName(String);
}

void __cdecl ActorScr_GetWeapon(actor_s *pSelf, const actor_fields_s *pField)
{
    const WeaponVariantDef *weapVariantDef; // [esp+0h] [ebp-4h]

    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 541, 0, "%s", "pField") )
        __debugbreak();
    if ( pField->type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    542,
                    0,
                    "%s",
                    "pField->type == F_INT") )
    {
        __debugbreak();
    }
    weapVariantDef = BG_GetWeaponVariantDef(*(unsigned int *)((char *)&pSelf->ent + pField->ofs));
    if ( weapVariantDef )
        Scr_AddString((char *)weapVariantDef->szInternalName, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorScr_GetGroundType(actor_s *pSelf, const actor_fields_s *pField)
{
    char *v2; // eax

    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 557, 0, "%s", "pSelf") )
        __debugbreak();
    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 558, 0, "%s", "pField") )
        __debugbreak();
    if ( pField->type != F_STRING
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    559,
                    0,
                    "%s",
                    "pField->type == F_STRING") )
    {
        __debugbreak();
    }
    if ( pField->ofs != 604
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    560,
                    0,
                    "%s",
                    "pField->ofs == AFOFS( Physics.iSurfaceType )") )
    {
        __debugbreak();
    }
    if ( pSelf->Physics.iSurfaceType )
    {
        v2 = (char *)Com_SurfaceTypeToName(pSelf->Physics.iSurfaceType);
        Scr_AddString(v2, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl ActorScr_SetLastEnemySightPos(actor_s *pSelf, const actor_fields_s *pField)
{
    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 608, 0, "%s", "pSelf") )
        __debugbreak();
    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 609, 0, "%s", "pField") )
        __debugbreak();
    if ( pField->type != F_VECTOR
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    610,
                    0,
                    "%s",
                    "pField->type == F_VECTOR") )
    {
        __debugbreak();
    }
    if ( pField->ofs != 5800
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    611,
                    0,
                    "%s",
                    "pField->ofs == AFOFS( lastEnemySightPos )") )
    {
        __debugbreak();
    }
    if ( Scr_GetType(0, SCRIPTINSTANCE_SERVER) )
    {
        Scr_GetVector(0, pSelf->lastEnemySightPos, SCRIPTINSTANCE_SERVER);
        pSelf->lastEnemySightPosValid = 1;
    }
    else
    {
        pSelf->lastEnemySightPosValid = 0;
    }
}

void __cdecl ActorScr_GetLastEnemySightPos(actor_s *pSelf, const actor_fields_s *pField)
{
    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 631, 0, "%s", "pSelf") )
        __debugbreak();
    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 632, 0, "%s", "pField") )
        __debugbreak();
    if ( pField->type != F_VECTOR
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    633,
                    0,
                    "%s",
                    "pField->type == F_VECTOR") )
    {
        __debugbreak();
    }
    if ( pField->ofs != 5800
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    634,
                    0,
                    "%s",
                    "pField->ofs == AFOFS( lastEnemySightPos )") )
    {
        __debugbreak();
    }
    if ( pSelf->lastEnemySightPosValid )
        Scr_AddVector(pSelf->lastEnemySightPos, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorScr_GetPathGoalPos(actor_s *self, const actor_fields_s *field)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 649, 0, "%s", "self") )
        __debugbreak();
    if ( !field && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 650, 0, "%s", "field") )
        __debugbreak();
    if ( field->type != F_VECTOR
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    651,
                    0,
                    "%s",
                    "field->type == F_VECTOR") )
    {
        __debugbreak();
    }
    if ( field->ofs != 3472
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    652,
                    0,
                    "%s",
                    "field->ofs == AFOFS( Path.vFinalGoal )") )
    {
        __debugbreak();
    }
    if ( Actor_HasPath(self) )
        Scr_AddVector(self->Path.vFinalGoal, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorScr_SetFixedNode(actor_s *self, const actor_fields_s *field)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 667, 0, "%s", "self") )
        __debugbreak();
    if ( !field && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 668, 0, "%s", "field") )
        __debugbreak();
    if ( field->type != F_BYTE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    669,
                    0,
                    "%s",
                    "field->type == F_BYTE") )
    {
        __debugbreak();
    }
    self->fixedNode = Scr_GetInt(0, SCRIPTINSTANCE_SERVER) != 0;
    self->exposedStartTime = 0x80000000;
}

void __cdecl ActorScr_GetMoveMode(actor_s *pSelf, const actor_fields_s *pField)
{
    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 683, 0, "%s", "pSelf") )
        __debugbreak();
    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 684, 0, "%s", "pField") )
        __debugbreak();
    if ( pField->type != F_STRING
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    685,
                    0,
                    "%s",
                    "pField->type == F_STRING") )
    {
        __debugbreak();
    }
    if ( pField->ofs != 548
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    686,
                    0,
                    "%s",
                    "pField->ofs == AFOFS( moveMode )") )
    {
        __debugbreak();
    }
    switch ( pSelf->moveMode )
    {
        case AI_MOVE_STOP:
            Scr_AddConstString(scr_const.stop, SCRIPTINSTANCE_SERVER);
            break;
        case AI_MOVE_STOP_SOON:
            Scr_AddConstString(scr_const.stop_soon, SCRIPTINSTANCE_SERVER);
            break;
        case AI_MOVE_WALK:
            Scr_AddConstString(scr_const.walk, SCRIPTINSTANCE_SERVER);
            break;
        case AI_MOVE_RUN:
            Scr_AddConstString(scr_const.run, SCRIPTINSTANCE_SERVER);
            break;
        default:
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 707, 0, "unhandled") )
                __debugbreak();
            break;
    }
}

void __cdecl Cmd_AI_f()
{
    int bInvertSelection; // [esp+0h] [ebp-21Ch]
    char *pszEntity; // [esp+4h] [ebp-218h]
    const actor_fields_s *fields; // [esp+8h] [ebp-214h]
    char szEntity[256]; // [esp+Ch] [ebp-210h] BYREF
    char szField[260]; // [esp+10Ch] [ebp-110h] BYREF
    const actor_fields_s *pField; // [esp+214h] [ebp-8h]
    int argc; // [esp+218h] [ebp-4h]

    bInvertSelection = 0;
    argc = SV_Cmd_Argc();
    if ( argc < 3 )
    {
        Cmd_AI_PrintUsage();
        return;
    }
    SV_Cmd_ArgvBuffer(2, szField, 256);
    if ( I_stricmp(szField, "list") )
    {
        if ( I_stricmp(szField, "delete") )
        {
            fields = aifields;
            pField = FindFieldForName(aifields, szField);
            if ( !pField )
            {
                fields = sentientfields;
                pField = FindFieldForName(sentientfields, szField);
                if ( !pField )
                {
                    fields = entfields;
                    pField = FindFieldForName(entfields, szField);
                    if ( !pField )
                    {
                        Cmd_AI_PrintUsage();
                        Com_PrintError(0, "%s is not an actor or entity field\n", szField);
                        return;
                    }
                }
            }
        }
        else
        {
            pField = &aifield_delete;
            fields = 0;
        }
    }
    else
    {
        pField = &aifield_list;
        fields = 0;
    }
    if ( !pField
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 1296, 0, "%s", "pField") )
    {
        __debugbreak();
    }
    SV_Cmd_ArgvBuffer(1, szEntity, 256);
    pszEntity = szEntity;
    if ( szEntity[0] == 33 )
    {
        bInvertSelection = 1;
        pszEntity = &szEntity[1];
    }
    if ( isdigit(*pszEntity) )
    {
        Cmd_AI_EntityNumber(argc, fields, pField, pszEntity, bInvertSelection);
    }
    else if ( I_stricmp(pszEntity, "all") )
    {
        if ( I_stricmp(pszEntity, "axis") )
        {
            if ( I_stricmp(pszEntity, "allies") )
                Cmd_AI_Name(argc, fields, pField, pszEntity, bInvertSelection);
            else
                Cmd_AI_Team(argc, fields, pField, 4, bInvertSelection);
        }
        else
        {
            Cmd_AI_Team(argc, fields, pField, 2, bInvertSelection);
        }
    }
    else
    {
        Cmd_AI_Team(argc, fields, pField, -1, bInvertSelection);
    }
}

const actor_fields_s *__cdecl FindFieldForName(const actor_fields_s *fields, const char *pszFieldName)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; fields[i].name; ++i )
    {
        if ( !I_stricmp(pszFieldName, fields[i].name) )
            return &fields[i];
    }
    return 0;
}

void Cmd_AI_PrintUsage()
{
    Com_Printf(0, "^5USAGE: ai (!)target field (value), or ai (!) target [list/delete]\n");
    Com_Printf(
        0,
        "^5target can be an entity number, a targetname, an entity classname,\n        'all', 'axis', 'allies', or 'neutral'\n");
    Com_Printf(0, "^5if ! immediately precedes target, it uses AI that don't match target\n");
    Com_Printf(0, "^5field can be one of:\n");
    PrintFieldUsage(aifields);
    PrintFieldUsage(sentientfields);
    PrintFieldUsage(entfields);
}

void __cdecl PrintFieldUsage(const actor_fields_s *fields)
{
    const char *v1; // eax
    int i; // [esp+8h] [ebp-4h]

    for ( i = 0; fields[i].name; ++i )
    {
        switch ( fields[i].type )
        {
            case F_INT:
                Com_Printf(0, "^5    %-20s: %s\n", fields[i].name, "int");
                break;
            case F_SHORT:
                Com_Printf(0, "^5    %-20s: %s\n", fields[i].name, "short");
                break;
            case F_BYTE:
                Com_Printf(0, "^5    %-20s: %s\n", fields[i].name, "byte");
                break;
            case F_FLOAT:
                Com_Printf(0, "^5    %-20s: %s\n", fields[i].name, "float");
                break;
            case F_STRING:
            case F_MODEL:
                Com_Printf(0, "^5    %-20s: %s\n", fields[i].name, "string");
                break;
            case F_VECTOR:
                Com_Printf(0, "^5    %-20s: %s\n", fields[i].name, "vector");
                break;
            case F_ENTITY:
            case F_ENTHANDLE:
                Com_Printf(0, "^5    %-20s: %s\n", fields[i].name, "entnum");
                break;
            case F_ACTOR:
                Com_Printf(0, "^5    %-20s: %s\n", fields[i].name, "actor");
                break;
            case F_SENTIENT:
            case F_SENTIENTHANDLE:
                Com_Printf(0, "^5    %-20s: %s\n", fields[i].name, "sentient");
                break;
            case F_CLIENT:
                Com_Printf(0, "^5    %-20s: %s\n", fields[i].name, "clientnum");
                break;
            case F_PATHNODE:
                Com_Printf(0, "^5    %-20s: %s\n", fields[i].name, "pathnode");
                break;
            case F_ACTORGROUP:
                Com_Printf(0, "^5    %-20s: %s\n", fields[i].name, "actorgroup");
                break;
            default:
                v1 = va("Cmd_AI_f: unhandled field type %i\n", fields[i].type);
                if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 770, 0, v1) )
                    __debugbreak();
                break;
        }
    }
}

void __cdecl Cmd_AI_EntityNumber(
                int argc,
                const actor_fields_s *fields,
                const actor_fields_s *pField,
                const char *szNum,
                int bInvertSelection)
{
    actor_s *actor; // [esp+0h] [ebp-8h]
    unsigned int entnum; // [esp+4h] [ebp-4h]

    entnum = atoi(szNum);
    if ( bInvertSelection )
    {
        for ( actor = Actor_FirstActor(-1); actor; actor = Actor_NextActor(actor, -1) )
        {
            if ( actor->ent->s.number != entnum )
                Cmd_AI_Dispatch(argc, actor, fields, pField);
        }
    }
    else if ( entnum < 0x400 )
    {
        if ( level.gentities[entnum].actor )
        {
            Cmd_AI_Dispatch(argc, level.gentities[entnum].actor, fields, pField);
        }
        else
        {
            Cmd_AI_PrintUsage();
            Com_PrintError(0, "entity number %i is not an actor\n", entnum);
        }
    }
    else
    {
        Cmd_AI_PrintUsage();
        Com_PrintError(0, "%i is not a valid entity number\n", entnum);
    }
}

void __cdecl Cmd_AI_Dispatch(int argc, actor_s *pSelf, const actor_fields_s *fields, const actor_fields_s *pField)
{
    unsigned __int8 *v4; // eax
    unsigned __int8 *v5; // eax

    if ( argc < 3
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 1120, 0, "%s", "argc >= 3") )
    {
        __debugbreak();
    }
    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 1121, 0, "%s", "pSelf") )
        __debugbreak();
    if ( !pSelf->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 1122, 0, "%s", "pSelf->ent") )
    {
        __debugbreak();
    }
    if ( !pField
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 1126, 0, "%s", "pField") )
    {
        __debugbreak();
    }
    if ( pField == &aifield_list )
    {
        if ( fields )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                            1131,
                            0,
                            "%s",
                            "fields == NULL") )
                __debugbreak();
        }
        Cmd_AI_DisplayInfo(pSelf);
    }
    else if ( pField == &aifield_delete )
    {
        if ( fields
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 1136, 0, "%s", "fields == NULL") )
        {
            __debugbreak();
        }
        Cmd_AI_Delete(pSelf);
    }
    else if ( argc == 3 )
    {
        if ( !fields
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 1141, 0, "%s", "fields != NULL") )
        {
            __debugbreak();
        }
        v4 = (unsigned __int8 *)BaseForFields(pSelf, fields);
        Cmd_AI_DisplayValue(pSelf, v4, pField);
    }
    else
    {
        if ( !fields
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 1146, 0, "%s", "fields != NULL") )
        {
            __debugbreak();
        }
        v5 = (unsigned __int8 *)BaseForFields(pSelf, fields);
        Cmd_AI_SetValue(pSelf, argc, v5, pField);
    }
}

gentity_s *__cdecl BaseForFields(actor_s *actor, const actor_fields_s *fields)
{
    if ( fields == aifields )
        return (gentity_s *)actor;
    if ( fields == sentientfields )
        return (gentity_s *)actor->sentient;
    if ( fields == entfields )
        return actor->ent;
    if ( !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    270,
                    0,
                    "BaseForFields: invalid fields[]") )
        __debugbreak();
    Com_Error(ERR_DROP, "BaseForFields: invalid fields[]");
    return 0;
}

void __cdecl Cmd_AI_DisplayInfo(actor_s *actor)
{
    const char *v1; // eax
    char *v2; // eax
    char *v3; // [esp-4h] [ebp-4h]

    if ( !actor && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 805, 0, "%s", "actor") )
        __debugbreak();
    if ( !actor->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 806, 0, "%s", "actor->ent") )
    {
        __debugbreak();
    }
    if ( !actor->ent->classname
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    807,
                    0,
                    "%s",
                    "actor->ent->classname") )
    {
        __debugbreak();
    }
    if ( !actor->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 808, 0, "%s", "actor->sentient") )
    {
        __debugbreak();
    }
    v3 = SL_ConvertToString(actor->ent->classname, SCRIPTINSTANCE_SERVER);
    v1 = Sentient_NameForTeam(actor->sentient->eTeam);
    Com_Printf(0, "ent %i (%-7s) %-24s", actor->ent->s.number, v1, v3);
    if ( actor->ent->targetname )
    {
        v2 = SL_ConvertToString(actor->ent->targetname, SCRIPTINSTANCE_SERVER);
        Com_Printf(0, " targetname %s", v2);
    }
    Com_Printf(0, "\n");
}

void __cdecl Cmd_AI_Delete(actor_s *actor)
{
    if ( !actor && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 827, 0, "%s", "actor") )
        __debugbreak();
    if ( !actor->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 828, 0, "%s", "actor->ent") )
    {
        __debugbreak();
    }
    G_FreeEntityDelay(actor->ent);
}

void __cdecl Cmd_AI_DisplayValue(actor_s *pSelf, unsigned __int8 *pBase, const actor_fields_s *pField)
{
    unsigned int v3; // eax
    unsigned int v4; // eax
    char *v5; // eax
    const char *v6; // eax
    char *v7; // [esp+18h] [ebp-2Ch]
    char *v8; // [esp+1Ch] [ebp-28h]
    char *v9; // [esp+20h] [ebp-24h]
    char *v10; // [esp+24h] [ebp-20h]
    char *v11; // [esp+28h] [ebp-1Ch]
    char *v12; // [esp+2Ch] [ebp-18h]
    unsigned __int16 str; // [esp+34h] [ebp-10h]
    pathnode_t *pNode; // [esp+38h] [ebp-Ch]
    unsigned int i; // [esp+3Ch] [ebp-8h]
    int entnum; // [esp+40h] [ebp-4h]

    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 848, 0, "%s", "pField") )
        __debugbreak();
    entnum = pSelf->ent->s.number;
    if ( pField->getter == ActorScr_GetTime )
    {
        if ( pField->type )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                            854,
                            0,
                            "%s",
                            "pField->type == F_INT") )
                __debugbreak();
        }
        Com_Printf(0, "ent %i: %s = %g\n", entnum, pField->name, (float)((float)*(int *)&pBase[pField->ofs] * 0.001));
    }
    else
    {
        switch ( pField->type )
        {
            case F_INT:
                Com_Printf(0, "ent %i: %s = %i\n", entnum, pField->name, *(unsigned int *)&pBase[pField->ofs]);
                break;
            case F_SHORT:
                Com_Printf(0, "ent %i: %s = %i\n", entnum, pField->name, *(__int16 *)&pBase[pField->ofs]);
                break;
            case F_BYTE:
                Com_Printf(0, "ent %i: %s = %i\n", entnum, pField->name, pBase[pField->ofs]);
                break;
            case F_FLOAT:
                Com_Printf(0, "ent %i: %s = %g\n", entnum, pField->name, *(float *)&pBase[pField->ofs]);
                break;
            case F_STRING:
                str = *(_WORD *)&pBase[pField->ofs];
                if ( str )
                {
                    v12 = SL_ConvertToString(str, SCRIPTINSTANCE_SERVER);
                    Com_Printf(0, "ent %i: %s = %s\n", entnum, pField->name, v12);
                }
                else
                {
                    Com_Printf(0, "ent %i: %s = %s\n", entnum, pField->name, "<undefined>");
                }
                break;
            case F_VECTOR:
                Com_Printf(
                    0,
                    "ent %i: %s = %g %g %g\n",
                    entnum,
                    pField->name,
                    *(float *)&pBase[pField->ofs],
                    *(float *)&pBase[pField->ofs + 4],
                    *(float *)&pBase[pField->ofs + 8]);
                break;
            case F_ENTITY:
                if ( !*(unsigned int *)&pBase[pField->ofs] )
                    goto LABEL_43;
                i = (signed int)(*(unsigned int *)&pBase[pField->ofs] - (unsigned int)level.gentities) / 760;
                if ( i >= 0x400
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                                894,
                                0,
                                "%s",
                                "i >= 0 && i < MAX_GENTITIES") )
                {
                    __debugbreak();
                }
                if ( !level.gentities[i].targetname )
                    goto LABEL_46;
                v11 = SL_ConvertToString(level.gentities[i].targetname, SCRIPTINSTANCE_SERVER);
                Com_Printf(0, "ent %i: %s = %i (targetname %s)\n", entnum, pField->name, i, v11);
                break;
            case F_ENTHANDLE:
                //if ( !EntHandle::isDefined((EntHandle *)&pBase[pField->ofs]) )
                if ( !((EntHandle *)&pBase[pField->ofs])->isDefined() )
                    goto LABEL_43;
                //i = EntHandle::entnum((EntHandle *)&pBase[pField->ofs]);
                i = ((EntHandle *)&pBase[pField->ofs])->entnum();
                if ( i >= 0x400
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                                908,
                                0,
                                "%s",
                                "i >= 0 && i < MAX_GENTITIES") )
                {
                    __debugbreak();
                }
                if ( !level.gentities[i].targetname )
                    goto LABEL_46;
                v10 = SL_ConvertToString(level.gentities[i].targetname, SCRIPTINSTANCE_SERVER);
                Com_Printf(0, "ent %i: %s = %i (targetname %s)\n", entnum, pField->name, i, v10);
                break;
            case F_ACTOR:
                if ( !*(unsigned int *)&pBase[pField->ofs] )
                    goto LABEL_43;
                i = ***(unsigned int ***)&pBase[pField->ofs];
                if ( !level.gentities[i].targetname )
                    goto LABEL_46;
                v9 = SL_ConvertToString(level.gentities[i].targetname, SCRIPTINSTANCE_SERVER);
                Com_Printf(0, "ent %i: %s = %i (targetname %s)\n", entnum, pField->name, i, v9);
                break;
            case F_SENTIENT:
                if ( !*(unsigned int *)&pBase[pField->ofs] )
                    goto LABEL_43;
                i = ***(unsigned int ***)&pBase[pField->ofs];
                if ( !level.gentities[i].targetname )
                    goto LABEL_46;
                v8 = SL_ConvertToString(level.gentities[i].targetname, SCRIPTINSTANCE_SERVER);
                Com_Printf(0, "ent %i: %s = %i (targetname %s)\n", entnum, pField->name, i, v8);
                break;
            case F_SENTIENTHANDLE:
                //if ( !SentientHandle::isDefined((SentientHandle *)&pBase[pField->ofs]) )
                if ( !((SentientHandle *)&pBase[pField->ofs])->isDefined() )
                    goto LABEL_43;
                //i = SentientHandle::sentient((SentientHandle *)&pBase[pField->ofs])->ent->s.number;
                i = ((SentientHandle *)&pBase[pField->ofs])->sentient()->ent->s.number;
                if ( level.gentities[i].targetname )
                {
                    v7 = SL_ConvertToString(level.gentities[i].targetname, SCRIPTINSTANCE_SERVER);
                    Com_Printf(0, "ent %i: %s = %i (targetname %s)\n", entnum, pField->name, i, v7);
                }
                else
                {
LABEL_46:
                    Com_Printf(0, "ent %i: %s = %i (targetname %s)\n", entnum, pField->name, i, "<undefined>");
                }
                break;
            case F_CLIENT:
                Com_Printf(
                    0,
                    "ent %i: %s = client %i\n",
                    entnum,
                    pField->name,
                    (signed int)(*(unsigned int *)&pBase[pField->ofs] - (unsigned int)level.clients) / 10720);
                break;
            case F_PATHNODE:
                pNode = *(pathnode_t **)&pBase[pField->ofs];
                if ( pNode )
                {
                    v3 = Path_ConvertNodeToIndex(pNode);
                    Com_Printf(0, "ent %i: %s = node %i\n", entnum, pField->name, v3);
                }
                else
                {
LABEL_43:
                    Com_Printf(0, "ent %i: %s = (null)\n", entnum, pField->name);
                }
                break;
            case F_ACTORGROUP:
                return;
            case F_MODEL:
                v4 = G_ModelName(pBase[pField->ofs]);
                v5 = SL_ConvertToString(v4, SCRIPTINSTANCE_SERVER);
                Com_Printf(0, "ent %i: %s = %s\n", entnum, pField->name, v5);
                break;
            default:
                v6 = va("Cmd_AI_f: unhandled field type %i for %s\n", pField->type, pField->name);
                if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 973, 0, v6) )
                    __debugbreak();
                break;
        }
    }
}

void __cdecl Cmd_AI_SetValue(actor_s *pSelf, int argc, unsigned __int8 *pBase, const actor_fields_s *pField)
{
    const char *v4; // eax
    float v5; // [esp+8h] [ebp-130h]
    float v6; // [esp+10h] [ebp-128h]
    float f; // [esp+20h] [ebp-118h]
    float fa; // [esp+20h] [ebp-118h]
    int i; // [esp+24h] [ebp-114h]
    char arg[256]; // [esp+28h] [ebp-110h] BYREF
    float v[3]; // [esp+12Ch] [ebp-Ch]

    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 993, 0, "%s", "pField") )
        __debugbreak();
    if ( pField->setter == ActorScr_ReadOnly )
    {
        Com_PrintError(0, "%s is read-only\n", pField->name);
        return;
    }
    if ( pField->setter == ActorScr_SetTime )
    {
        if ( pField->type )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                            1003,
                            0,
                            "%s",
                            "pField->type == F_INT") )
                __debugbreak();
        }
        if ( argc == 4 )
        {
            SV_Cmd_ArgvBuffer(3, arg, 256);
            v5 = atof(arg);
            v6 = v5 * 1000.0;
            *(unsigned int *)&pBase[pField->ofs] = SnapFloatToInt(v6);
            return;
        }
LABEL_28:
        Cmd_AI_PrintUsage();
        return;
    }
    if ( pField->getter == ActorScr_SetGoalRadius )
    {
        if ( argc == 4 )
        {
            SV_Cmd_ArgvBuffer(3, arg, 256);
            f = atof(arg);
            Actor_SetGoalRadius(&pSelf->scriptGoal, f);
            return;
        }
        goto LABEL_28;
    }
    switch ( pField->type )
    {
        case F_INT:
            if ( argc != 4 )
                goto LABEL_28;
            SV_Cmd_ArgvBuffer(3, arg, 256);
            *(unsigned int *)&pBase[pField->ofs] = atoi(arg);
            break;
        case F_SHORT:
            if ( argc != 4 )
                goto LABEL_28;
            SV_Cmd_ArgvBuffer(3, arg, 256);
            *(_WORD *)&pBase[pField->ofs] = atoi(arg);
            break;
        case F_BYTE:
            if ( argc != 4 )
                goto LABEL_28;
            SV_Cmd_ArgvBuffer(3, arg, 256);
            pBase[pField->ofs] = atoi(arg);
            break;
        case F_FLOAT:
            if ( argc != 4 )
                goto LABEL_28;
            SV_Cmd_ArgvBuffer(3, arg, 256);
            fa = atof(arg);
            *(float *)&pBase[pField->ofs] = fa;
            break;
        case F_STRING:
        case F_ENTITY:
        case F_ENTHANDLE:
        case F_ACTOR:
        case F_SENTIENT:
        case F_SENTIENTHANDLE:
        case F_CLIENT:
        case F_PATHNODE:
        case F_ACTORGROUP:
        case F_MODEL:
            Com_Printf(0, "cannot set from console\n");
            break;
        case F_VECTOR:
            if ( argc != 6 )
                goto LABEL_28;
            for ( i = 0; i < 3; ++i )
            {
                SV_Cmd_ArgvBuffer(i + 3, arg, 256);
                v[i] = atof(arg);
                *(float *)&pBase[4 * i + pField->ofs] = v[i];
            }
            break;
        default:
            v4 = va("Cmd_AI_f: unhandled field type %i\n", pField->type);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 1107, 0, v4) )
                __debugbreak();
            break;
    }
}

void __cdecl Cmd_AI_Team(
                int argc,
                const actor_fields_s *fields,
                const actor_fields_s *pField,
                int iTeamFlags,
                int bInvertSelection)
{
    actor_s *actor; // [esp+0h] [ebp-4h]

    if ( bInvertSelection )
        iTeamFlags = ~iTeamFlags;
    for ( actor = Actor_FirstActor(iTeamFlags); actor; actor = Actor_NextActor(actor, iTeamFlags) )
        Cmd_AI_Dispatch(argc, actor, fields, pField);
}

void __cdecl Cmd_AI_Name(
                int argc,
                const actor_fields_s *fields,
                const actor_fields_s *pField,
                char *szName,
                int bInvertSelection)
{
    int offset; // [esp+4h] [ebp-Ch]
    unsigned __int16 name; // [esp+8h] [ebp-8h] BYREF
    actor_s *actor; // [esp+Ch] [ebp-4h]

    if ( I_strnicmp(szName, "actor_", 6) )
        offset = 360;
    else
        offset = 356;
    name = SL_GetString(szName, 0, SCRIPTINSTANCE_SERVER);
    for ( actor = Actor_FirstActor(-1); actor; actor = Actor_NextActor(actor, -1) )
    {
        if ( (*(unsigned __int16 *)((char *)&actor->ent->s.number + offset) == name) == (bInvertSelection == 0) )
            Cmd_AI_Dispatch(argc, actor, fields, pField);
    }
    Scr_SetString(&name, 0, SCRIPTINSTANCE_SERVER);
}

void __cdecl GScr_AddFieldsForActor()
{
    const actor_fields_s *f; // [esp+4h] [ebp-4h]

    for ( f = aifields; f->name; ++f )
    {
        if ( ((f - aifields) & 0xC000) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                        1354,
                        0,
                        "%s",
                        "!((f - aifields) & ENTFIELD_MASK)") )
        {
            __debugbreak();
        }
        if ( f - aifields != (unsigned __int16)(f - aifields)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                        1355,
                        0,
                        "%s",
                        "(f - aifields) == (unsigned short)( f - aifields )") )
        {
            __debugbreak();
        }
        Scr_AddClassField(0, (char *)f->name, (unsigned __int16)(f - aifields) | 0x8000, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl Scr_SetActorField(actor_s *actor, unsigned int offset)
{
    const actor_fields_s *f; // [esp+0h] [ebp-4h]

    if ( !actor && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 1370, 0, "%s", "actor") )
        __debugbreak();
    if ( offset >= 0x4F
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    1371,
                    0,
                    "%s",
                    "(unsigned)offset < ARRAY_COUNT( aifields ) - 1") )
    {
        __debugbreak();
    }
    f = &aifields[offset];
    if ( f->setter )
        f->setter(actor, f);
    else
        GScr_SetGenericField((unsigned __int8 *)actor, f->type, f->ofs, 0);
}

void __cdecl Scr_GetActorField(actor_s *actor, unsigned int offset)
{
    const actor_fields_s *f; // [esp+0h] [ebp-4h]

    if ( !actor && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp", 1394, 0, "%s", "actor") )
        __debugbreak();
    if ( offset >= 0x4F
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_fields.cpp",
                    1395,
                    0,
                    "%s",
                    "(unsigned)offset < ARRAY_COUNT( aifields ) - 1") )
    {
        __debugbreak();
    }
    f = &aifields[offset];
    if ( f->getter )
        f->getter(actor, f);
    else
        GScr_GetGenericField((unsigned __int8 *)actor, f->type, f->ofs, 0);
}

