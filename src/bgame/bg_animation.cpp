#include "bg_animation.h"
#include <cstdarg>
#include <ctype.h>
#include <qcommon/common.h>
#include <universal/q_parse.h>
#include "bg_local.h"
#include "bg_public.h"
#include <clientscript/cscr_animtree.h>
#include "bg_weapons_load_obj.h"
#include <clientscript/cscr_stringlist.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "bg_misc.h"
#include <cgame/cg_bolt.h>
#include "bg_weapons.h"
#include "bg_weapons_def.h"
#include <demo/demo_common.h>
#include <demo/demo_playback.h>
#include <game_mp/g_main_mp.h>
#include "bg_animconditions.h"
#include <xanim/xanim.h>
#include <ik/ik_import.h>
#include <xanim/xmodel.h>
#include <xanim/dobj_utils.h>
#include <universal/com_memory.h>
#include <universal/com_loadutils.h>
#include <qcommon/ent.h>
#include <cgame_mp/cg_predict_mp.h>

animStringItem_t defineStr[23][16];
char defineStrings[10000];
unsigned int defineBits[23][16];
int numDefines[23];
unsigned int defineStringsOffset;

char debugString[16];

const char *globalFilename = "mp/playeranim.script";

animStringItem_t weaponStrings[2048];

thread_local bgs_t *bgs; // tls+8
bgs_t level_bgs;

loadAnim_t *g_pLoadAnims;
unsigned int *g_piNumLoadAnims;

bool parseVehicleLoop;
int parseVehicleNameIndex;

void __cdecl BG_NULLSUB(pmove_t *pm)
{
    ;
}
void(__cdecl *localClientConditionUpdateFunc[23])(pmove_t *) =
{
  &BG_LocalEvalPlayerAnimType,
  &BG_LocalEvalWeaponClass,
  &BG_LocalEvalNextPlayerAnimType,
  &BG_LocalEvalNextWeaponClass,
  &BG_LocalEvalMounted,
  &BG_LocalEvalMoveStatus,
  &BG_LocalEvalDirection,
  &BG_LocalEvalDmgDirection,
  &BG_LocalEvalDmgType,
  &BG_LocalEvalStance,
  &BG_LocalEvalUnderhand,
  &BG_LocalEvalFiring,
  &BG_LocalEvalWeaponPosition,
  &BG_LocalEvalSlope,
  &BG_LocalEvalPerk,
  &BG_LocalEvalAttachment,
  &BG_LocalEvalVehicleName,
  &BG_LocalEvalVehicleSeatTo,
  &BG_NULLSUB,
  &BG_NULLSUB,
  &BG_NULLSUB,
  &BG_LocalEvalEvent,
  &BG_NULLSUB
};

animStringItem_t animParseModesStr[7] =
{
  { "defines", -1 },
  { "animations", -1 },
  { "canned_animations", -1 },
  { "statechanges", -1 },
  { "events", -1 },
  { "forceload", -1 },
  { NULL, -1 }
};

animStringItem_t animStateStr[2] =
{ { "COMBAT", -1 }, { NULL, -1 } };

animStringItem_t animMoveTypesStr[28] =
{
  { "** UNUSED **", -1 },
  { "IDLE", -1 },
  { "SHUFFLE", -1 },
  { "WALK", -1 },
  { "RUN", -1 },
  { "CLIMBUP", -1 },
  { "CLIMBDOWN", -1 },
  { "SPRINT", -1 },
  { "MANTLE_ROOT", -1 },
  { "MANTLE_UP_57", -1 },
  { "MANTLE_UP_51", -1 },
  { "MANTLE_UP_45", -1 },
  { "MANTLE_UP_39", -1 },
  { "MANTLE_UP_33", -1 },
  { "MANTLE_UP_27", -1 },
  { "MANTLE_UP_21", -1 },
  { "MANTLE_OVER_HIGH", -1 },
  { "MANTLE_OVER_MID", -1 },
  { "MANTLE_OVER_LOW", -1 },
  { "TURNRIGHT", -1 },
  { "TURNLEFT", -1 },
  { "STUMBLE", -1 },
  { "STUMBLE_WALK", -1 },
  { "STUMBLE_SPRINT", -1 },
  { "SWIM", -1 },
  { "DTP_MOVE", -1 },
  { "SLIDE_MOVE", -1 },
  { NULL, -1 }
};

animStringItem_t animVehicleAnimTypeStr[3] =
{ { "DEFAULT", -1 }, { "TANK", -1 }, { NULL, -1 } };

animStringItem_t animConditionsStr[24] =
{
  { "PLAYERANIMTYPE", -1 },
  { "WEAPONCLASS", -1 },
  { "NEXTPLAYERANIMTYPE", -1 },
  { "NEXTWEAPONCLASS", -1 },
  { "MOUNTED", -1 },
  { "MOVESTATUS", -1 },
  { "DIRECTION", -1 },
  { "DMGDIRECTION", -1 },
  { "DMGTYPE", -1 },
  { "STANCE", -1 },
  { "UNDERHAND", -1 },
  { "FIRING", -1 },
  { "WEAPON_POSITION", -1 },
  { "SLOPE", -1 },
  { "PERK", -1 },
  { "ATTACHMENT", -1 },
  { "VEHICLE_NAME", -1 },
  { "VEHICLE_SEAT_TO", -1 },
  { "VEHICLE_SEAT_FROM", -1 },
  { "VEHICLE_ANIM_STAGE", -1 },
  { "VEHICLE_ENTRY_POS", -1 },
  { "EVENT", -1 },
  { "CAC", -1 },
  { NULL, -1 }
};

animStringItem_t animEventTypesStr[35] =
{
  { "** UNUSED **", -1 },
  { "DEATH", -1 },
  { "FIREWEAPON", -1 },
  { "JUMP", -1 },
  { "LAND", -1 },
  { "DROPWEAPON", -1 },
  { "RAISEWEAPON", -1 },
  { "RELOAD", -1 },
  { "CROUCH_TO_PRONE", -1 },
  { "PRONE_TO_CROUCH", -1 },
  { "STAND_TO_CROUCH", -1 },
  { "CROUCH_TO_STAND", -1 },
  { "PRONE_TO_STAND", -1 },
  { "PRONE_TO_SPRINT", -1 },
  { "MELEEATTACK", -1 },
  { "KNIFE_MELEE", -1 },
  { "KNIFE_MELEE_CHARGE", -1 },
  { "FLINCH", -1 },
  { "SHELLSHOCK", -1 },
  { "VEHICLE_ENTRY", -1 },
  { "VEHICLE_CHANGE_SEAT", -1 },
  { "VEHICLE_EXIT", -1 },
  { "SCRIPTEVENT", -1 },
  { "DTP_TAKEOFF", -1 },
  { "DTP_LAND", -1 },
  { "STAND_TO_LASTSTAND", -1 },
  { "CROUCH_TO_LASTSTAND", -1 },
  { "PRONE_TO_LASTSTAND", -1 },
  { "LASTSTAND_TO_STAND", -1 },
  { "LASTSTAND_TO_CROUCH", -1 },
  { "LASTSTAND_TO_PRONE", -1 },
  { "FLARED", -1 },
  { "LASTSTAND_SUICIDE", -1 },
  { "PRIME_GRENADE", -1 },
  { NULL, -1 }
};

animStringItem_t animConditionMountedStr[3] = { { "** UNUSED **", -1 }, { "MG42", -1 }, { NULL, -1 } };

animStringItem_t animWeaponPositionStr[3] = { { "HIP", -1 }, { "ADS", -1 }, { NULL, -1 } };

animStringItem_t animPerkStateStr[4] =
{
  { "** UNUSED **", -1 },
  { "LASTSTAND", -1 },
  { "GRENADEDEATH", -1 },
  { NULL, -1 }
};

animStringItem_t animAttachmentStateStr[3] = { { "** UNUSED **", -1 }, { "BAYONET", -1 }, { NULL, -1 } };

animStringItem_t animVehicleSeatStr[12] =
{
  { "DRIVER", -1 },
  { "GUNNER1", -1 },
  { "GUNNER2", -1 },
  { "GUNNER3", -1 },
  { "GUNNER4", -1 },
  { "PASSENGER1", -1 },
  { "PASSENGER2", -1 },
  { "PASSENGER3", -1 },
  { "PASSENGER4", -1 },
  { "PASSENGER5", -1 },
  { "PASSENGER6", -1 },
  { NULL, -1 }
};

animStringItem_t animVehicleEntryPosStr[4] = { { "SIDE_LEFT", -1 }, { "SIDE_RIGHT", -1 }, { "REAR", -1 }, { NULL, -1 } };


animStringItem_t animVehicleNameStr[16] =
{
  { NULL, -1 },
  { NULL, 0 },
  { NULL, 0 },
  { NULL, 0 },
  { NULL, 0 },
  { NULL, 0 },
  { NULL, 0 },
  { NULL, 0 },
  { NULL, 0 },
  { NULL, 0 },
  { NULL, 0 },
  { NULL, 0 },
  { NULL, 0 },
  { NULL, 0 },
  { NULL, 0 },
  { NULL, 0 }
};

animStringItem_t animStanceStr[4] =
{ 
    { "STAND", -1 }, 
    { "CROUCH", -1 }, 
    { "PRONE", -1 }, 
    { NULL, -1 } 
};

animStringItem_t animBodyPartsStr[5] =
{
  { "** UNUSED **", -1 },
  { "LEGS", -1 },
  { "TORSO", -1 },
  { "BOTH", -1 },
  { NULL, -1 }
};

animStringItem_t animMoveStatusStr[4] =
{ 
    { "STATIONARY", -1 }, 
    { "WALK", -1 }, 
    { "RUN", -1 }, 
    { NULL, -1 } 
};

animStringItem_t animDirectionStr[6] =
{
  { "NONE", -1 },
  { "FORWARD", -1 },
  { "BACKWARD", -1 },
  { "LEFT", -1 },
  { "RIGHT", -1 },
  { NULL, -1 }
};

animStringItem_t animDmgDirectionStr[6] =
{
  { "NONE", -1 },
  { "FRONT", -1 },
  { "BACK", -1 },
  { "LEFT", -1 },
  { "RIGHT", -1 },
  { NULL, -1 }
};

animStringItem_t animDmgTypeStr[8] =
{
  { "NORMAL", -1 },
  { "EXPLOSIVE", -1 },
  { "MELEE", -1 },
  { "HEADSHOT", -1 },
  { "FIRE", -1 },
  { "SUICIDE", -1 },
  { "GAS", -1 },
  { NULL, -1 }
};

animStringItem_t animSlopeStateStr[4] =
{ { "NONE", -1 }, { "UP", -1 }, { "DOWN", -1 }, { NULL, -1 } };

animStringItem_t animWeaponClassStr[14] =
{
  { "RIFLE", -1 },
  { "MG", -1 },
  { "SMG", -1 },
  { "SPREAD", -1 },
  { "PISTOL", -1 },
  { "GRENADE", -1 },
  { "ROCKETLAUNCHER", -1 },
  { "TURRET", -1 },
  { "NON-PLAYER", -1 },
  { "GAS", -1 },
  { "ITEM", -1 },
  { "MELEE", -1 },
  { "KILLSTREAK_ALT_STORED_WEAPON", -1 },
  { NULL, -1 }
};

void BG_NULLSUB2(const entityState_s *, const clientInfo_t *)
{
    ;
}

void(__cdecl * clientConditionUpdateFunc[23])(const entityState_s *, const clientInfo_t *) =
{
  BG_EvalPlayerAnimType,
  BG_EvalWeaponClass,
  BG_NULLSUB2,
  BG_NULLSUB2,
  BG_EvalMounted,
  BG_EvalMoveStatus,
  BG_EvalDirection,
  BG_EvalDmgDirection,
  BG_NULLSUB2,
  BG_EvalStance,
  BG_EvalUnderhand,
  BG_EvalFiring,
  BG_EvalWeaponPosition,
  BG_EvalSlope,
  BG_NULLSUB2,
  BG_NULLSUB2,
  BG_NULLSUB2,
  BG_NULLSUB2,
  BG_NULLSUB2,
  BG_NULLSUB2,
  BG_NULLSUB2,
  BG_NULLSUB2,
  BG_NULLSUB2
};

animConditionTable_t animConditionsTable[23] =
{
  { ANIM_CONDTYPE_BITFLAGS, weaponStrings, 0, 0 },
  { ANIM_CONDTYPE_BITFLAGS, animWeaponClassStr, 0, 0 },
  { ANIM_CONDTYPE_BITFLAGS, weaponStrings, 0, 0 },
  { ANIM_CONDTYPE_BITFLAGS, animWeaponClassStr, 0, 0 },
  { ANIM_CONDTYPE_VALUE, animConditionMountedStr, 0, 0 },
  { ANIM_CONDTYPE_BITFLAGS, animMoveStatusStr, 0, 0 },
  { ANIM_CONDTYPE_VALUE, animDirectionStr, 0, 0 },
  { ANIM_CONDTYPE_VALUE, animDmgDirectionStr, 0, 0 },
  { ANIM_CONDTYPE_VALUE, animDmgTypeStr, 0, 0 },
  { ANIM_CONDTYPE_VALUE, animStanceStr, 0, 0 },
  { ANIM_CONDTYPE_ENABLED, NULL, 0, 0 },
  { ANIM_CONDTYPE_ENABLED, NULL, 0, 0 },
  { ANIM_CONDTYPE_VALUE, animWeaponPositionStr, 0, 0 },
  { ANIM_CONDTYPE_VALUE, animSlopeStateStr, 0, 0 },
  { ANIM_CONDTYPE_BITFLAGS, animPerkStateStr, 0, 0 },
  { ANIM_CONDTYPE_VALUE, animAttachmentStateStr, 0, 0 },
  { ANIM_CONDTYPE_VALUE, animVehicleNameStr, 0, 0 },
  { ANIM_CONDTYPE_BITFLAGS, animVehicleSeatStr, 0, 0 },
  { ANIM_CONDTYPE_BITFLAGS, animVehicleSeatStr, 0, 0 },
  { ANIM_CONDTYPE_VALUE, NULL, 1, 7 },
  { ANIM_CONDTYPE_VALUE, animVehicleEntryPosStr, 0, 0 },
  { ANIM_CONDTYPE_STRINGHASH, NULL, 0, 0 },
  { ANIM_CONDTYPE_ENABLED, NULL, 0, 0 }
};



char animVehicleNameStrings[24 * 16 /*384*/];
int animVehicleTypes[16];
animVehicleSeats_t animVehicleSeats[16];

scriptAnimMoveTypes_t parseMovetype;
scriptAnimStances_t parseStance;
scriptAnimMoveStatusStates_t parseMovestatus;
int parseEvent;



int __cdecl BG_StringHashValue(const char *fname)
{
    int hash; // [esp+0h] [ebp-Ch]
    int i; // [esp+8h] [ebp-4h]

    hash = 0;
    for ( i = 0; fname[i]; ++i )
        hash += (i + 119) * (char)tolower(fname[i]);
    if ( hash == -1 )
        return 0;
    return hash;
}

void BG_AnimParseError(const char *msg, ...)
{
    int CurrentParseLine; // eax
    char text[1028]; // [esp+4h] [ebp-408h] BYREF
    va_list va; // [esp+418h] [ebp+Ch] BYREF

    va_start(va, msg);
    _vsnprintf(text, 0x400u, msg, va);
    if ( globalFilename )
    {
        CurrentParseLine = Com_GetCurrentParseLine();
        Com_Error(ERR_DROP, "%s: (%s, line %i)", text, globalFilename, CurrentParseLine + 1);
    }
    else
    {
        Com_Error(ERR_DROP, "%s", text);
    }
}

unsigned int __cdecl BG_AnimationIndexForString(const char *string)
{
    char v2; // [esp+3h] [ebp-1Dh]
    char *szAnimName; // [esp+8h] [ebp-18h]
    const char *v4; // [esp+Ch] [ebp-14h]
    int hash; // [esp+10h] [ebp-10h]
    int hasha; // [esp+10h] [ebp-10h]
    loadAnim_t *loadAnim; // [esp+14h] [ebp-Ch]
    loadAnim_t *loadAnima; // [esp+14h] [ebp-Ch]
    bgsAnim_s *anim; // [esp+18h] [ebp-8h]
    unsigned int i; // [esp+1Ch] [ebp-4h]
    unsigned int ia; // [esp+1Ch] [ebp-4h]

    BG_CheckThread();

    iassert(bgs);

    if (g_pLoadAnims)
    {
        hasha = BG_StringHashValue(string);
        ia = 0;
        loadAnim = g_pLoadAnims;
        while (ia < *g_piNumLoadAnims)
        {
            if (hasha == loadAnim->iNameHash && !I_stricmp(string, loadAnim->szAnimName))
                return ia;
            ++ia;
            ++loadAnim;
        }
        loadAnima = &g_pLoadAnims[*g_piNumLoadAnims];
        Scr_FindAnim(
            SCRIPTINSTANCE_SERVER,
            "multiplayer",
            string,
            &loadAnima->anim,
            bgs->anim_user);
        v4 = string;
        szAnimName = loadAnima->szAnimName;
        do
        {
            v2 = *v4;
            *szAnimName++ = *v4++;
        } while (v2);
        loadAnima->iNameHash = hasha;
        return (*g_piNumLoadAnims)++;
    }
    else
    {
        hash = BG_StringHashValue(string);
        i = 0;
        anim = bgs->animData;
        while (i < bgs->animData->animScriptData.numAnimations)
        {
            if (hash == anim->animScriptData.animations[0].nameHash && !I_stricmp(string, (const char *)anim))
                return i;
            ++i;
            anim = (bgsAnim_s *)((char *)anim + 120);
        }
        BG_AnimParseError("BG_AnimationIndexForString: unknown player animation '%s'", string);
        return -1;
    }
}

int __cdecl BG_IndexForString(const char *token, animStringItem_t *strings, int allowFail)
{
    int hash; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    hash = BG_StringHashValue(token);
    i = 0;
    while ( strings->string )
    {
        if ( strings->hash == -1 )
            strings->hash = BG_StringHashValue(strings->string);
        if ( hash == strings->hash && !I_stricmp(token, strings->string) )
            return i;
        ++strings;
        ++i;
    }
    if ( !allowFail )
        BG_AnimParseError("BG_IndexForString: unknown token '%s'", token);
    return -1;
}

const char *__cdecl BG_StringForIndex(int index, animStringItem_t *strings, int allowFail)
{
    int i; // [esp+4h] [ebp-4h]

    i = 0;
    while ( strings->string )
    {
        if ( i == index )
            return strings->string;
        ++strings;
        ++i;
    }
    if ( !allowFail )
        BG_AnimParseError("BG_StringForIndex: index out of range (%i, max is %i)", index, i - 1);
    return 0;
}

void __cdecl BG_InitWeaponString(int index, const char *name)
{
    weaponStrings[index].string = name;
    weaponStrings[index].hash = BG_StringHashValue(name);
}

void __cdecl BG_InitWeaponStrings()
{
    memset(weaponStrings, 0, sizeof(weaponStrings));
    BG_LoadWeaponStrings();
}

void __cdecl BG_AddVehicleName(const char *vehName, int vehType, int drivers, int gunners, int passengers)
{
    int j; // [esp+0h] [ebp-10h]
    int ja; // [esp+0h] [ebp-10h]
    int pos; // [esp+4h] [ebp-Ch]
    int posa; // [esp+4h] [ebp-Ch]
    animStringItem_t *strav; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    i = 0;
    for ( strav = animVehicleNameStr; strav->string && i < 16; ++strav )
    {
        if ( !I_stricmp(vehName, strav->string) )
            return;
        ++i;
    }
    if ( i + 1 >= 16
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                    839,
                    0,
                    "%s",
                    "i+1 < BG_ANIM_MAX_VEHICLE_TYPES") )
    {
        __debugbreak();
    }
    I_strncpyz(&animVehicleNameStrings[24 * i], vehName, 24);
    animVehicleNameStr[i].string = &animVehicleNameStrings[24 * i];
    animVehicleNameStr[i].hash = BG_StringHashValue(vehName);
    animVehicleTypes[i] = vehType;
    animVehicleSeats[i].seats = 0;
    if ( drivers > 0 )
        animVehicleSeats[i].seats |= 1u;
    j = 0;
    pos = 1;
    while ( j < gunners )
    {
        animVehicleSeats[i].seats |= 1 << pos;
        ++j;
        ++pos;
    }
    ja = 0;
    posa = 5;
    while ( ja < passengers )
    {
        animVehicleSeats[i].seats |= 1 << posa;
        ++ja;
        ++posa;
    }
    animVehicleNameStr[i + 1].string = 0;
    animVehicleNameStr[i + 1].hash = -1;
}

unsigned int __cdecl BG_AnimGetConditionValue(animScriptItem_t *scriptItem, int conditionIndex)
{
    animScriptCondition_t *scriptCond; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    i = 0;
    scriptCond = scriptItem->conditions;
    while ( i < scriptItem->numConditions )
    {
        if ( scriptCond->index == conditionIndex )
            return scriptCond->value;
        ++i;
        ++scriptCond;
    }
    return -1;
}

void __cdecl BG_InitVehicles()
{
    memset(animVehicleNameStr, 0, sizeof(animVehicleNameStr));
    memset(animVehicleTypes, 0, sizeof(animVehicleTypes));
}

int __cdecl BG_GetVehicleAnimSetIndex(unsigned __int16 animSetStr)
{
    char *v1; // eax

    v1 = SL_ConvertToString(animSetStr, SCRIPTINSTANCE_SERVER);
    return BG_IndexForString(v1, animVehicleNameStr, 1);
}

void __cdecl BG_ParseCommands(const char **input, animScriptItem_t *scriptItem, animScriptData_t *scriptData)
{
    __int16 v4; // ax
    unsigned __int16 String; // ax
    int v10; // eax
    const char *v19; // [esp+4Ch] [ebp-88h]
    unsigned int v20; // [esp+50h] [ebp-84h]
    parseInfo_t *v21; // [esp+74h] [ebp-60h]
    int partIndex; // [esp+78h] [ebp-5Ch] BYREF
    char animName[64]; // [esp+7Ch] [ebp-58h] BYREF
    int numAnimTokens; // [esp+C0h] [ebp-14h]
    const char *token; // [esp+C4h] [ebp-10h]
    animScriptCommand_t *command; // [esp+C8h] [ebp-Ch]
    int i; // [esp+CCh] [ebp-8h]
    int bCommandFound; // [esp+D0h] [ebp-4h]

    command = 0;
    partIndex = 0;
    while ( 1 )
    {
        v21 = partIndex >= 1 ? Com_ParseOnLine(input) : Com_Parse(input);
        token = (const char *)v21;
        if ( !v21 || !*token )
            break;

        if ( !I_stricmp(token, "}") )
        {
            *input -= strlen(token);
            return;
        }

        if ( !partIndex )
        {
            if ( scriptItem->numCommands >= 8 )
                BG_AnimParseError("BG_ParseCommands: exceeded maximum number of animations (%i)", 8);
            command = &scriptItem->commands[scriptItem->numCommands++];
            *(unsigned int *)command->bodyPart = 0;
        }

        command->bodyPart[partIndex] = BG_IndexForString(token, animBodyPartsStr, 1);
        if ( command->bodyPart[partIndex] <= 0 )
        {
            *input -= strlen(token);
            goto LABEL_198;
        }

        animName[0] = 0;
        numAnimTokens = 1;
        while ( numAnimTokens > 0 )
        {
            token = Com_ParseOnLine(input)->token;
            if ( token && *token )
            {
                --numAnimTokens;
                if ( I_stricmp(token, "%vehicle_name") )
                {
                    strcat(animName, token);
                }
                else
                {
                    if ( parseVehicleNameIndex < 0
                        || !BG_StringForIndex(parseVehicleNameIndex, animVehicleNameStr, 0)
                        || !parseVehicleLoop )
                    {
                        BG_AnimParseError("BG_ParseCommands: %vehicle_name variable used outside #FOR_ALL_VEHICLES loop");
                    }

                    strcat(animName, BG_StringForIndex(parseVehicleNameIndex, animVehicleNameStr, 0));
                }
                token = Com_ParseOnLine(input)->token;
                if ( token )
                {
                    if ( I_stricmp(token, "+") )
                        Com_UngetToken();
                    else
                        numAnimTokens = 1;
                }
            }
            else
            {
                if ( &animName[strlen(animName) + 1] != &animName[1] )
                    break;
                BG_AnimParseError("BG_ParseCommands: expected animation");
            }
        }
        v4 = BG_AnimationIndexForString(animName);
        command->animIndex[partIndex] = v4;
        command->animDuration[partIndex] = scriptData->animations[command->animIndex[partIndex]].duration;
        if ( command->animDuration[partIndex] != scriptData->animations[command->animIndex[partIndex]].duration )
            scriptData->animations[command->animIndex[partIndex]].flags |= 0x1000u;


        if ( !g_pLoadAnims )
        {
            if ( (parseMovetype || parseEvent) && command->bodyPart[partIndex] != 2 )
            {
                scriptData->animations[command->animIndex[partIndex]].stance |= 1 << parseStance;
                scriptData->animations[command->animIndex[partIndex]].movetype |= 1LL << parseMovetype;
                if ( parseMovestatus == ANIM_MOVESTATUS_RUN )
                {
                    scriptData->animations[command->animIndex[partIndex]].flags |= 0x4000000u;
                }
                else if ( parseMovestatus == ANIM_MOVESTATUS_WALK )
                {
                    scriptData->animations[command->animIndex[partIndex]].flags |= 0x2000000u;
                }
                if ( (parseMovetype == ANIM_MT_CLIMBUP || parseMovetype == ANIM_MT_CLIMBDOWN)
                    && scriptData->animations[command->animIndex[partIndex]].moveSpeed != 0.0 )
                {
                    scriptData->animations[command->animIndex[partIndex]].flags |= 2u;
                }
                for ( i = 0; i < scriptItem->numConditions; ++i )
                {
                    switch ( scriptItem->conditions[i].index )
                    {
                        case 6:
                            switch ( scriptItem->conditions[i].value )
                            {
                                case 3u:
                                    scriptData->animations[command->animIndex[partIndex]].flags |= 0x10u;
                                    break;
                                case 4u:
                                    scriptData->animations[command->animIndex[partIndex]].flags |= 0x20u;
                                    break;
                                case 2u:
                                    scriptData->animations[command->animIndex[partIndex]].flags |= 0x40u;
                                    break;
                                case 1u:
                                    scriptData->animations[command->animIndex[partIndex]].flags |= 0x80u;
                                    break;
                            }
                            break;
                        case 0xD:
                            if ( scriptItem->conditions[i].value == 1 )
                            {
                                scriptData->animations[command->animIndex[partIndex]].flags |= 0x20000u;
                            }
                            else if ( scriptItem->conditions[i].value == 2 )
                            {
                                scriptData->animations[command->animIndex[partIndex]].flags |= 0x40000u;
                            }
                            break;
                        case 7:
                            switch ( scriptItem->conditions[i].value )
                            {
                                case 3u:
                                    scriptData->animations[command->animIndex[partIndex]].flags |= 0x80000u;
                                    break;
                                case 4u:
                                    scriptData->animations[command->animIndex[partIndex]].flags |= 0x100000u;
                                    break;
                                case 2u:
                                    scriptData->animations[command->animIndex[partIndex]].flags |= 0x200000u;
                                    break;
                                case 1u:
                                    scriptData->animations[command->animIndex[partIndex]].flags |= 0x400000u;
                                    break;
                            }
                            break;
                        case 5:
                            switch ( scriptItem->conditions[i].value )
                            {
                                case 2u:
                                    scriptData->animations[command->animIndex[partIndex]].flags |= 0x4000000u;
                                    break;
                                case 1u:
                                    scriptData->animations[command->animIndex[partIndex]].flags |= 0x2000000u;
                                    break;
                                case 0u:
                                    scriptData->animations[command->animIndex[partIndex]].flags = 0;
                                    break;
                            }
                            break;
                        case 9:
                            if ( scriptItem->conditions[i].value == 1 )
                            {
                                scriptData->animations[command->animIndex[partIndex]].flags |= 0x800000u;
                            }
                            else if ( scriptItem->conditions[i].value == 2 )
                            {
                                scriptData->animations[command->animIndex[partIndex]].flags |= 0x1000000u;
                            }
                            else
                            {
                                scriptData->animations[command->animIndex[partIndex]].flags = 0;
                            }
                            break;
                    }
                }
            }
            if ( parseMovetype == ANIM_MT_IDLE || parseEvent == 22 )
                scriptData->animations[command->animIndex[partIndex]].moveSpeed = 0.0f;
            if ( parseMovetype == ANIM_MT_RUN )
                scriptData->animations[command->animIndex[partIndex]].flags |= 0x4000000u;
            if ( parseMovetype == ANIM_MT_WALK )
                scriptData->animations[command->animIndex[partIndex]].flags |= 0x2000000u;
            switch ( parseEvent )
            {
                case 2:
                    scriptData->animations[command->animIndex[partIndex]].flags |= 8u;
                    scriptData->animations[command->animIndex[partIndex]].initialLerp = 30;
                    break;
                case 15:
                case 16:
                    scriptData->animations[command->animIndex[partIndex]].flags |= 0x400u;
                    break;
                case 1:
                    scriptData->animations[command->animIndex[partIndex]].moveSpeed = 0.0f;
                    scriptData->animations[command->animIndex[partIndex]].rotSpeed = 0.0f;
                    scriptData->animations[command->animIndex[partIndex]].flags |= 0x100u;
                    break;
                case 7:
                    scriptData->animations[command->animIndex[partIndex]].moveSpeed = 0.0f;
                    break;
                default:
                    if ( parseMovetype < ANIM_MT_MANTLE_UP_57 || parseMovetype > ANIM_MT_MANTLE_OVER_LOW )
                    {
                        if ( parseEvent < 19 || parseEvent > 21 )
                        {
                            if ( parseEvent == 22 )
                                scriptData->animations[command->animIndex[partIndex]].flags |= 0x800u;
                        }
                        else
                        {
                            scriptData->animations[command->animIndex[partIndex]].moveSpeed = 0.0f;
                            scriptData->animations[command->animIndex[partIndex]].rotSpeed = 0.0f;
                        }
                    }
                    else
                    {
                        scriptData->animations[command->animIndex[partIndex]].moveSpeed = 0.0f;
                        scriptData->animations[command->animIndex[partIndex]].rotSpeed = 0.0f;
                    }
                    break;
            }
        }
        do
        {
            bCommandFound = 0;
            token = Com_ParseOnLine(input)->token;
            if ( !token || !*token )
            {
LABEL_192:
                Com_UngetToken();
                continue;
            }
            if ( I_stricmp(token, "duration") )
            {
                if ( I_stricmp(token, "turretanim") )
                {
                    if ( I_stricmp(token, "blendtime") )
                    {
                        if ( I_stricmp(token, "blendouttime") )
                        {
                            if ( I_stricmp(token, "attach") )
                            {
                                if ( !I_stricmp(token, "turn_to_turret") )
                                {
                                    bCommandFound = 1;
                                    command->flags |= 2u;
                                    continue;
                                }
                                if ( !I_stricmp(token, "throw_grenade") )
                                {
                                    bCommandFound = 1;
                                    command->flags |= 1u;
                                    continue;
                                }
                                if ( I_stricmp(token, "grenadeAnim") )
                                {
                                    if ( I_stricmp(token, "allowMovementInterrupt") )
                                    {
                                        if ( I_stricmp(token, "weaponTimeScale") )
                                        {
                                            if ( I_stricmp(token, "weaponDelayScale") )
                                            {
                                                if ( I_stricmp(token, "animrate") )
                                                    goto LABEL_192;
                                                bCommandFound = 1;
                                                if ( (scriptData->animations[command->animIndex[partIndex]].flags & 0xC000) != 0
                                                    && !Assert_MyHandler(
                                                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                                                                1842,
                                                                0,
                                                                "%s\n\t%s",
                                                                "( scriptData->animations[command->animIndex[partIndex]].flags & (ANIMFL_WEAPONDELAY_SCAL"
                                                                "E | ANIMFL_WEAPONTIME_SCALE) ) == 0",
                                                                "ERROR: You cannot set animRate along with either weaponDelayScale or weaponTimeScale\n") )
                                                {
                                                    __debugbreak();
                                                }
                                                token = Com_ParseOnLine(input)->token;
                                                if ( !token || !*token )
                                                    BG_AnimParseError("BG_ParseCommands: expected animrate value");
                                                if ( !g_pLoadAnims )
                                                {
                                                    scriptData->animations[command->animIndex[partIndex]].forceAnimRate = atof(token);
                                                }
                                            }
                                            else
                                            {
                                                bCommandFound = 1;
                                                if ( ((scriptData->animations[command->animIndex[partIndex]].flags & 0x4000) != 0
                                                     || scriptData->animations[command->animIndex[partIndex]].forceAnimRate > 0.0)
                                                    && !Assert_MyHandler(
                                                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                                                                1834,
                                                                0,
                                                                "%s\n\t%s",
                                                                "( scriptData->animations[command->animIndex[partIndex]].flags & ANIMFL_WEAPONTIME_SCALE "
                                                                ") == 0 && scriptData->animations[command->animIndex[partIndex]].forceAnimRate <= 0.f",
                                                                "ERROR: You cannot set weaponDelayScale along with either animRate or weaponTimeScale\n") )
                                                {
                                                    __debugbreak();
                                                }
                                                if ( !g_pLoadAnims )
                                                    scriptData->animations[command->animIndex[partIndex]].flags |= 0x8000u;
                                            }
                                        }
                                        else
                                        {
                                            bCommandFound = 1;
                                            if ( ((scriptData->animations[command->animIndex[partIndex]].flags & 0x8000) != 0
                                                 || scriptData->animations[command->animIndex[partIndex]].forceAnimRate > 0.0)
                                                && !Assert_MyHandler(
                                                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                                                            1826,
                                                            0,
                                                            "%s\n\t%s",
                                                            "( scriptData->animations[command->animIndex[partIndex]].flags & ANIMFL_WEAPONDELAY_SCALE )"
                                                            " == 0 && scriptData->animations[command->animIndex[partIndex]].forceAnimRate <= 0.f",
                                                            "ERROR: You cannot set weaponTimeScale along with either animRate or weaponDelayScale\n") )
                                            {
                                                __debugbreak();
                                            }
                                            if ( !g_pLoadAnims )
                                                scriptData->animations[command->animIndex[partIndex]].flags |= 0x4000u;
                                        }
                                    }
                                    else
                                    {
                                        bCommandFound = 1;
                                        if ( !g_pLoadAnims )
                                            scriptData->animations[command->animIndex[partIndex]].flags |= 0x8000000u;
                                    }
                                }
                                else
                                {
                                    bCommandFound = 1;
                                    if ( !g_pLoadAnims )
                                        scriptData->animations[command->animIndex[partIndex]].flags |= 0x10000u;
                                }
                            }
                            else
                            {
                                bCommandFound = 1;
                                token = Com_ParseOnLine(input)->token;
                                if ( !token || !*token )
                                    BG_AnimParseError("BG_ParseCommands: expected attach tag name");
                                String = SL_FindString(token, SCRIPTINSTANCE_SERVER);
                                command->tagName = String;
                                if ( !command->tagName )
                                {
                                    command->tagName = SL_GetString_(SCRIPTINSTANCE_SERVER, (char *)token, 0, 17);
                                }
                            }
                        }
                        else
                        {
                            bCommandFound = 1;
                            token = Com_ParseOnLine(input)->token;
                            if ( !token || !*token )
                                BG_AnimParseError("BG_ParseCommands: expected blendtime value");
                            if ( !g_pLoadAnims )
                            {
                                scriptData->animations[command->animIndex[partIndex]].finalLerp = atoi(token);
                            }
                        }
                    }
                    else
                    {
                        bCommandFound = 1;
                        token = Com_ParseOnLine(input)->token;
                        if ( !token || !*token )
                            BG_AnimParseError("BG_ParseCommands: expected blendtime value");
                        if ( !g_pLoadAnims )
                        {
                            scriptData->animations[command->animIndex[partIndex]].initialLerp = atoi(token);
                        }
                    }
                }
                else
                {
                    bCommandFound = 1;
                    if ( !g_pLoadAnims )
                        scriptData->animations[command->animIndex[partIndex]].flags |= 4u;
                    if ( command->bodyPart[partIndex] != 3 )
                        BG_AnimParseError("BG_ParseCommands: Turret animations can only be played on the 'both' body part");
                }
            }
            else
            {
                bCommandFound = 1;
                token = Com_ParseOnLine(input)->token;
                if ( !token || !*token )
                    BG_AnimParseError("BG_ParseCommands: expected duration value");
                command->animDuration[partIndex] = atoi(token);
            }
        }
        while ( bCommandFound );
        if ( command->bodyPart[partIndex] != 3 )
        {
            v10 = partIndex++;
            if ( v10 < 1 )
                continue;
        }
LABEL_198:
        while ( 1 )
        {
            token = Com_ParseOnLine(input)->token;
            if ( !token || !*token )
                break;
            if ( I_stricmp(token, "sound") )
            {
                BG_AnimParseError("BG_ParseCommands: unknown parameter '%s'", token);
            }
            else
            {
                token = Com_ParseOnLine(input)->token;
                if ( !token || !*token )
                    BG_AnimParseError("BG_ParseCommands: expected sound");
                if (strstr(token, ".wav"))
                    BG_AnimParseError("BG_ParseCommands: wav files not supported, only sound scripts");
                command->soundAlias = bgs->animData->animScriptData.soundAlias(token);
            }
        }
        partIndex = 0;
    }
}

int __cdecl GetValueForBitfield(unsigned int bitfield)
{
    int i; // [esp+0h] [ebp-8h]

    for ( i = 0; i < 32; ++i )
    {
        if ( Com_BitCheckAssert(&bitfield, i, 0xFFFFFFF) )
            return i;
    }
    return 0;
}

bool __cdecl Com_BitCheckAssert(const unsigned int *array, int bitNum, int size)
{
    if ( !array
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../universal/q_shared.h", 870, 0, "%s", "array") )
    {
        __debugbreak();
    }
    if ( bitNum >= (unsigned int)(8 * size)
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../universal/q_shared.h",
                    871,
                    0,
                    "bitNum doesn't index size * 8\n\t%i not in [0, %i)",
                    bitNum,
                    8 * size) )
    {
        __debugbreak();
    }
    return (array[bitNum >> 5] & (1 << (bitNum & 0x1F))) != 0;
}

int __cdecl BG_PlayAnim(
                playerState_s *ps,
                int animNum,
                animBodyPart_t bodyPart,
                int forceDuration,
                int setTimer,
                int isContinue,
                int force)
{
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    const char *v13; // eax
    const char *v14; // eax
    const char *v15; // eax
    const char *CurrentMoveStatus; // eax
    const char *CurrentWeaponName; // eax
    const char *CurrentWeaponClass; // eax
    char *CurrentPlayerAnimType; // eax
    const char *CurrentStance; // eax
    const char *CurrentDirection; // eax
    const char *CurrentDmgDirection; // eax
    const char *CurrentDmgType; // eax
    const char *CurrentSlope; // eax
    const char *v26; // [esp+0h] [ebp-2Ch]
    const char *v27; // [esp+8h] [ebp-24h]
    const char *v28; // [esp+Ch] [ebp-20h]
    const char *v29; // [esp+14h] [ebp-18h]
    float forceAnimRate; // [esp+1Ch] [ebp-10h]
    int wasSet; // [esp+24h] [ebp-8h]
    int duration; // [esp+28h] [ebp-4h]

    wasSet = 0;
    if (bgs->animData->animScriptData.animations[animNum].forceAnimRate <= 0.0)
        forceAnimRate = 1.0f;
    else
        forceAnimRate = bgs->animData->animScriptData.animations[animNum].forceAnimRate;

    if ( bg_forceDurationOverride->current.integer > 0 )
        forceDuration = bg_forceDurationOverride->current.integer;

    if (forceDuration)
        duration = (int)(float)((float)forceDuration / forceAnimRate);
    else
        duration = (int)(float)((float)bgs->animData->animScriptData.animations[animNum].duration / forceAnimRate) + 50;
    if ( bodyPart != ANIM_BP_LEGS )
    {
        if ( bodyPart == ANIM_BP_TORSO )
        {
LABEL_53:
            if ( ps->torsoTimer < 50 || force )
            {
                if ( isContinue && (ps->torsoAnim & 0xFFFFFBFF) == animNum )
                {
                    if ( setTimer
                        && (bgs->animData->animScriptData.animations[animNum].flags & 0x200) != 0 )
                    {
                        ps->torsoTimer = duration;
                    }
                }
                else
                {
                    AssignToSmallerType<short>(&ps->torsoAnim, animNum | ps->torsoAnim & 0x400 ^ 0x400);
                    if ( setTimer )
                        ps->torsoTimer = duration;
                    ps->torsoAnimDuration = duration;
                    if ( xanim_debug->current.enabled )
                    {
                        if ( bodyPart == ANIM_BP_BOTH )
                            v27 = "body";
                        else
                            v27 = "torso";

                        if ( bgs == &level_bgs )
                            v26 = "SV";
                        else
                            v26 = "CL";

                        Com_Printf(
                            19,
                            "\n[%s]%i: Playing (client %i) %s on %s\n",
                            v26,
                            ps->commandTime,
                            ps->clientNum,
                            bgs->animData->animScriptData.animations[animNum].name,
                            v27);
                        CurrentMoveStatus = BG_GetCurrentMoveStatus(ps);
                        Com_Printf(19, "         MoveStatus: %s\n", CurrentMoveStatus);
                        CurrentWeaponName = BG_GetCurrentWeaponName(ps);
                        Com_Printf(19, "         WeaponName: %s\n", CurrentWeaponName);
                        CurrentWeaponClass = BG_GetCurrentWeaponClass(ps);
                        Com_Printf(19, "         WeaponClass: %s\n", CurrentWeaponClass);
                        CurrentPlayerAnimType = BG_GetCurrentPlayerAnimType(ps);
                        Com_Printf(19, "         PlayerAnimType: %s\n", CurrentPlayerAnimType);
                        CurrentStance = BG_GetCurrentStance(ps);
                        Com_Printf(19, "         Stance: %s\n", CurrentStance);
                        CurrentDirection = BG_GetCurrentDirection(ps);
                        Com_Printf(19, "         Direction: %s\n", CurrentDirection);
                        CurrentDmgDirection = BG_GetCurrentDmgDirection(ps);
                        Com_Printf(19, "         DmgDirection: %s\n", CurrentDmgDirection);
                        CurrentDmgType = BG_GetCurrentDmgType(ps);
                        Com_Printf(19, "         DmgType: %s\n", CurrentDmgType);
                        CurrentSlope = BG_GetCurrentSlope(ps);
                        Com_Printf(19, "         Slope: %s\n", CurrentSlope);
                    }
                }
            }
            goto LABEL_71;
        }
        if ( bodyPart != ANIM_BP_BOTH )
            goto LABEL_71;
    }
    if ( !force && (ps->legsAnim & 0xFFFFFBFF) != animNum )
    {
        iassert((ps->legsAnim & ~ANIM_TOGGLEBIT) >= 0);
        iassert((uint)(ps->legsAnim & ~ANIM_TOGGLEBIT) < bgs->animData->animScriptData.numAnimations);

        if (bgs->animData->animScriptData.animations[ps->legsAnim & 0xFFFFFBFF].flags != 0
            && (bgs->animData->animScriptData.animations[animNum].moveSpeed != 0.0
             || bgs->animData->animScriptData.animations[animNum].rotSpeed != 0.0) )
        {
            force = 1;
        }
    }
    if ( ps->legsTimer < 50 || force )
    {
        if ( isContinue && (ps->legsAnim & 0xFFFFFBFF) == animNum )
        {
            if ( setTimer
                && (bgs->animData->animScriptData.animations[animNum].flags & 0x200) != 0 )
            {
                ps->legsTimer = duration;
            }
            else if ( xanim_debug->current.enabled && (ps->legsAnim & 0xFFFFFBFF) != animNum )
            {
                Com_Printf(19, "anim failed because");
                if ( (ps->legsAnim & 0xFFFFFBFF) == animNum )
                {
                    Com_Printf(19, ", isContinue is true");
                    Com_Printf(
                        19,
                        ", legsAnim is %s, asking to play %s",
                        bgs->animData->animScriptData.animations[ps->legsAnim].name,
                        bgs->animData->animScriptData.animations[animNum].name);
                }
                if ( setTimer )
                {
                    if ((bgs->animData->animScriptData.animations[animNum].flags & 0x200) == 0 )
                        Com_Printf(19, ", on a non-looped anim");
                }
                else
                {
                    Com_Printf(19, ", setTimer is false");
                }
                Com_Printf(19, "\n");
            }
        }
        else
        {
            wasSet = 1;
            ps->legsAnimDuration = duration;
            AssignToSmallerType<short>(&ps->legsAnim, animNum | ps->legsAnim & 0x400 ^ 0x400);
            if ( setTimer )
                ps->legsTimer = duration;
            if ( xanim_debug->current.enabled )
            {
                if ( bodyPart == ANIM_BP_BOTH )
                    v29 = "body";
                else
                    v29 = "legs";
                if (bgs == &level_bgs )
                    v28 = "SV";
                else
                    v28 = "CL";
                Com_Printf(
                    19,
                    "\n[%s]%i: Playing (client %i) %s on %s\n",
                    v28,
                    ps->commandTime,
                    ps->clientNum,
                    bgs->animData->animScriptData.animations[animNum].name,
                    v29);
                v7 = BG_GetCurrentMoveStatus(ps);
                Com_Printf(19, "         MoveStatus: %s\n", v7);
                v8 = BG_GetCurrentWeaponName(ps);
                Com_Printf(19, "         WeaponName: %s\n", v8);
                v9 = BG_GetCurrentWeaponClass(ps);
                Com_Printf(19, "         WeaponClass: %s\n", v9);
                v10 = BG_GetCurrentPlayerAnimType(ps);
                Com_Printf(19, "         PlayerAnimType: %s\n", v10);
                v11 = BG_GetCurrentStance(ps);
                Com_Printf(19, "         Stance: %s\n", v11);
                v12 = BG_GetCurrentDirection(ps);
                Com_Printf(19, "         Direction: %s\n", v12);
                v13 = BG_GetCurrentDmgDirection(ps);
                Com_Printf(19, "         DmgDirection: %s\n", v13);
                v14 = BG_GetCurrentDmgType(ps);
                Com_Printf(19, "         DmgType: %s\n", v14);
                v15 = BG_GetCurrentSlope(ps);
                Com_Printf(19, "         Slope: %s\n", v15);
            }
        }
    }
    if ( bodyPart == ANIM_BP_BOTH )
    {
        animNum = 0;
        goto LABEL_53;
    }
LABEL_71:
    if ( wasSet )
        return duration;
    else
        return -1;
}

const char *__cdecl BG_GetCurrentStance(playerState_s *ps)
{
    return animStanceStr[BG_GetConditionValue(&bgs->clientinfo[ps->clientNum],9)].string;
}

const char *__cdecl BG_GetCurrentMoveStatus(playerState_s *ps)
{
    int moveStatus; // [esp+0h] [ebp-4h]

    moveStatus = BG_GetConditionValue(&bgs->clientinfo[ps->clientNum], 5u);
    return animMoveStatusStr[GetValueForBitfield(moveStatus)].string;
}

const char *__cdecl BG_GetCurrentDirection(playerState_s *ps)
{
    return animDirectionStr[BG_GetConditionValue(&bgs->clientinfo[ps->clientNum], 6u)].string;
}

const char *__cdecl BG_GetCurrentDmgDirection(playerState_s *ps)
{
    return animDmgDirectionStr[BG_GetConditionValue(&bgs->clientinfo[ps->clientNum], 7u)].string;
}

const char *__cdecl BG_GetCurrentDmgType(playerState_s *ps)
{
    return animDmgTypeStr[BG_GetConditionValue(&bgs->clientinfo[ps->clientNum], 8u)].string;
}

const char *__cdecl BG_GetCurrentSlope(playerState_s *ps)
{
    return animSlopeStateStr[BG_GetConditionValue(&bgs->clientinfo[ps->clientNum], 0xDu)].string;
}

const char *__cdecl BG_GetCurrentWeaponName(playerState_s *ps)
{
    unsigned int ViewmodelWeaponIndex; // eax

    ViewmodelWeaponIndex = BG_GetViewmodelWeaponIndex(ps);
    return BG_GetWeaponVariantDef(ViewmodelWeaponIndex)->szInternalName;
}

const char *__cdecl BG_GetCurrentWeaponClass(playerState_s *ps)
{
    unsigned int ViewmodelWeaponIndex; // eax

    ViewmodelWeaponIndex = BG_GetViewmodelWeaponIndex(ps);
    return animWeaponClassStr[BG_GetWeaponDef(ViewmodelWeaponIndex)->weapClass].string;
}

char *__cdecl BG_GetCurrentPlayerAnimType(playerState_s *ps)
{
    unsigned int ViewmodelWeaponIndex; // eax
    const WeaponDef *WeaponDef; // eax

    ViewmodelWeaponIndex = BG_GetViewmodelWeaponIndex(ps);
    WeaponDef = BG_GetWeaponDef(ViewmodelWeaponIndex);
    return BG_GetPlayerAnimTypeName(WeaponDef->playerAnimType);
}

int __cdecl BG_ExecuteCommand(
    playerState_s *ps,
    animScriptCommand_t *scriptCommand,
    int setTimer,
    int isContinue,
    int force)
{
    int duration; // [esp+0h] [ebp-8h]
    bool playedLegsAnim; // [esp+4h] [ebp-4h]

    duration = -1;
    playedLegsAnim = 0;
    if (Demo_IsPlaying())
        return -1;
    if (scriptCommand->bodyPart[0])
    {
        if ((bgs->animData->animScriptData.animations[scriptCommand->animIndex[0]].flags & 0x1000) != 0)
            duration = bgs->animData->animScriptData.animations[scriptCommand->animIndex[0]].duration + 50;
        else
            duration = scriptCommand->animDuration[0] + 50;
        if (scriptCommand->bodyPart[0] == 1 || scriptCommand->bodyPart[0] == 3)
            playedLegsAnim = BG_PlayAnim(
                ps,
                scriptCommand->animIndex[0],
                (animBodyPart_t)scriptCommand->bodyPart[0],
                duration,
                setTimer,
                isContinue,
                force) > -1;
        else
            BG_PlayAnim(
                ps,
                scriptCommand->animIndex[0],
                (animBodyPart_t)scriptCommand->bodyPart[0],
                duration,
                setTimer,
                isContinue,
                force);
    }
    if (scriptCommand->bodyPart[1])
    {
        if ((bgs->animData->animScriptData.animations[scriptCommand->animIndex[0]].flags
            & 0x1000) != 0)
            duration = bgs->animData->animScriptData.animations[scriptCommand->animIndex[0]].duration + 50;
        else
            duration = scriptCommand->animDuration[0] + 50;
        if (scriptCommand->bodyPart[0] == 1 || scriptCommand->bodyPart[0] == 3)
            playedLegsAnim = BG_PlayAnim(
                ps,
                scriptCommand->animIndex[1],
                (animBodyPart_t)scriptCommand->bodyPart[1],
                duration,
                setTimer,
                isContinue,
                force) > -1;
        else
            BG_PlayAnim(
                ps,
                scriptCommand->animIndex[1],
                (animBodyPart_t)scriptCommand->bodyPart[1],
                duration,
                setTimer,
                isContinue,
                force);
    }
    if (scriptCommand->soundAlias)
        bgs->animData->animScriptData.playSoundAlias(
            ps->clientNum,
            scriptCommand->soundAlias);
    if (playedLegsAnim)
        return duration;
    else
        return -1;
}

int __cdecl BG_AnimScriptAnimation(pmove_t *pm, aistateEnum_t state, scriptAnimMoveTypes_t movetype, int force)
{
    animScript_t *script; // [esp+8h] [ebp-Ch]
    animScriptItem_t *scriptItem; // [esp+Ch] [ebp-8h]
    playerState_s *ps; // [esp+10h] [ebp-4h]

    scriptItem = 0;
    ps = pm->ps;

    BG_CheckThread();
    iassert(bgs);
    if (movetype < ANIM_MT_UNUSED
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp", 3269, 0, "%s", "movetype >= 0"))
    {
        __debugbreak();
    }
    if (movetype >= NUM_ANIM_MOVETYPES
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
            3270,
            0,
            "%s",
            "movetype < NUM_ANIM_MOVETYPES"))
    {
        __debugbreak();
    }
    if (ps->clientNum >= 0x20u
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
            3272,
            0,
            "ps->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            ps->clientNum,
            32))
    {
        __debugbreak();
    }
    BG_AnimUpdatePlayerStateConditions(pm);
    if (ps->pm_type >= 9)
        return -1;
    while (!scriptItem && state >= AISTATE_COMBAT)
    {
        script = &bgs->animData->animScriptData.scriptAnims[state][movetype];
        if (script->numItems)
        {
            scriptItem = BG_FirstValidItem(ps->clientNum, script);
            if (!scriptItem)
                --state;
        }
        else
        {
            --state;
        }
    }
    if (scriptItem)
    {
        if (scriptItem->numCommands)
        {
            ps->moveType = movetype;
            if (ps->clientNum >= 0x20u
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                    3319,
                    0,
                    "ps->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    ps->clientNum,
                    32))
            {
                __debugbreak();
            }
            if (!scriptItem->numCommands
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                    3322,
                    0,
                    "%s",
                    "scriptItem->numCommands"))
            {
                __debugbreak();
            }
            return BG_ExecuteCommand(ps, &scriptItem->commands[ps->clientNum % scriptItem->numCommands], 0, 1, force) != -1;
        }
        else
        {
            if (xanim_debug->current.enabled)
                Com_Printf(19, "Animation has no commands associated, finding new animation\n");
            return -1;
        }
    }
    else
    {
        if (xanim_debug->current.enabled)
            Com_Printf(19, "Failed playing animation, finding new animation\n");
        return -1;
    }
}

animScriptItem_t *__cdecl BG_FirstValidItem(unsigned int client, animScript_t *script)
{
    const char *BodyPart; // eax
    int command; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    animScriptItem_t **ppScriptItem; // [esp+8h] [ebp-4h]

    BG_CheckThread();
    iassert(bgs);
    i = 0;
    ppScriptItem = script->items;
    while (i < script->numItems)
    {
        if (animscript_debug->current.enabled)
        {
            Com_Printf(19, "Evaluating whether to play: ");
            for (command = 0; command < (*ppScriptItem)->numCommands; ++command)
            {
                BodyPart = GetBodyPart((*ppScriptItem)->commands[command].bodyPart[0]);
                Com_Printf(
                    19,
                    "%s on %s",
                    bgs->animData->animScriptData.animations[(*ppScriptItem)->commands[command].animIndex[0]].name,
                    BodyPart);
                if (command > 0)
                    Com_Printf(19, ", ");
            }
            Com_Printf(19, "\n");
        }
        if (client >= 0x20
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                2947,
                0,
                "client doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                client,
                32))
        {
            __debugbreak();
        }
        if (BG_EvaluateConditions(&bgs->clientinfo[client], *ppScriptItem))
        {
            return *ppScriptItem;
        }
        ++i;
        ++ppScriptItem;
    }
    return 0;
}

int __cdecl BG_EvaluateConditions(clientInfo_t *ci, animScriptItem_t *scriptItem)
{
    char *PlayerAnimTypeName; // eax
    int ValueForBitfield; // eax
    const char *WeaponTypeName; // eax
    char *ConditionString; // eax
    int v6; // eax
    char *v7; // eax
    char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    animScriptConditionTypes_t type; // [esp+0h] [ebp-18h]
    animScriptCondition_t *cond; // [esp+8h] [ebp-10h]
    int index; // [esp+Ch] [ebp-Ch]
    int i; // [esp+10h] [ebp-8h]

    i = 0;
    cond = scriptItem->conditions;
    while ( i < scriptItem->numConditions )
    {
        if ( animscript_debug->current.enabled )
        {
            switch ( scriptItem->conditions[i].index )
            {
                case 0:
                    index = GetValueForBitfield(scriptItem->conditions[i].value);
                    PlayerAnimTypeName = BG_GetPlayerAnimTypeName(index);
                    Com_Printf(19, "Checking to see if weapon animtype is %s...\n", PlayerAnimTypeName);
                    break;
                case 1:
                    ValueForBitfield = GetValueForBitfield(scriptItem->conditions[i].value);
                    WeaponTypeName = GetWeaponTypeName(ValueForBitfield);
                    Com_Printf(19, "Checking to see if weapon type is %s...\n", WeaponTypeName);
                    break;
                case 4:
                    ConditionString = BG_GetConditionString(scriptItem->conditions[i].index, scriptItem->conditions[i].value);
                    Com_Printf(19, "Checking to see if player is mounted status is %s...\n", ConditionString);
                    break;
                case 5:
                    v6 = GetValueForBitfield(scriptItem->conditions[i].value);
                    v7 = BG_GetConditionString(scriptItem->conditions[i].index, v6);
                    Com_Printf(19, "Checking to see if player movestatus is %s...\n", v7);
                    break;
                case 6:
                    v8 = BG_GetConditionString(scriptItem->conditions[i].index, scriptItem->conditions[i].value);
                    Com_Printf(19, "Checking to see if player direction is %s...\n", v8);
                    break;
                case 7:
                    v9 = BG_GetConditionString(scriptItem->conditions[i].index, scriptItem->conditions[i].value);
                    Com_Printf(19, "Checking to see if player damage direction is %s...\n", v9);
                    break;
                case 9:
                    v10 = BG_GetConditionString(scriptItem->conditions[i].index, scriptItem->conditions[i].value);
                    Com_Printf(19, "Checking to see if player stance is %s...\n", v10);
                    break;
                case 0xA:
                    Com_Printf(19, "Checking to see if player using underhand...\n");
                    break;
                case 0xB:
                    Com_Printf(19, "Checking to see if player is firing...\n");
                    break;
                case 0xC:
                    Com_Printf(19, "Checking weapon position...\n");
                    break;
                case 0x10:
                    Com_Printf(19, "Checking for vehicle name...\n");
                    break;
                case 0x11:
                    Com_Printf(19, "Checking for vehicle seat to...\n");
                    break;
                case 0x12:
                    Com_Printf(19, "Checking for vehicle seat from...\n");
                    break;
                case 0x15:
                    Com_Printf(19, "Checking for script event...\n");
                    break;
                case 0x16:
                    Com_Printf(19, "Checking if Crate a Class...\n");
                    break;
                default:
                    break;
            }
        }
        type = animConditionsTable[cond->index].type;
        if ( type )
        {
            if ( type > ANIM_CONDTYPE_BITFLAGS
                && type <= ANIM_CONDTYPE_STRINGHASH
                && ci->clientConditions[cond->index] != cond->value )
            {
                if ( animscript_debug->current.enabled )
                    Com_Printf(19, "failed\n");
                return 0;
            }
        }
        else if ( (cond->value & ci->clientConditions[cond->index]) == 0 )
        {
            if ( animscript_debug->current.enabled )
                Com_Printf(19, "failed\n");
            return 0;
        }
        ++i;
        ++cond;
    }
    if ( animscript_debug->current.enabled )
        Com_Printf(19, "Success\n");
    return 1;
}

const char *__cdecl GetWeaponTypeName(int type)
{
    const char *result; // eax

    switch ( type )
    {
        case 0:
            result = "Rifle";
            break;
        case 1:
            result = "Machine Gun";
            break;
        case 2:
            result = "Submachine Gun";
            break;
        case 4:
            result = "Pistol";
            break;
        case 5:
            result = "Grenade";
            break;
        case 6:
            result = "Rocket Launcher";
            break;
        case 7:
            result = "Turret";
            break;
        case 8:
            result = "Weapon that's not meant for use by the player";
            break;
        case 9:
            result = "Flamethrower";
            break;
        case 10:
            result = "Item";
            break;
        default:
            result = "Unknown";
            break;
    }
    return result;
}

char *__cdecl BG_GetConditionString(int condition, int value)
{
    char *result; // eax
    const char *v3; // eax
    const char *v4; // [esp+0h] [ebp-8h]

    BG_CheckThread();
    switch ( condition )
    {
        case 0:
        case 2:
            result = BG_GetPlayerAnimTypeName(value);
            break;
        case 1:
        case 3:
            result = (char *)animWeaponClassStr[value].string;
            break;
        case 4:
            result = (char *)animConditionMountedStr[value].string;
            break;
        case 5:
            result = (char *)animMoveStatusStr[value].string;
            break;
        case 6:
            result = (char *)animDirectionStr[value].string;
            break;
        case 7:
            result = (char *)animDmgDirectionStr[value].string;
            break;
        case 9:
            result = (char *)animStanceStr[value].string;
            break;
        case 10:
        case 11:
        case 22:
            if ( value )
                v4 = "true";
            else
                v4 = "false";
            result = (char *)v4;
            break;
        case 12:
            result = (char *)animWeaponPositionStr[value].string;
            break;
        case 13:
            result = (char *)animSlopeStateStr[value].string;
            break;
        case 21:
            sprintf(debugString, "0x%08x", value);
            result = debugString;
            break;
        default:
            v3 = va("BG_GetConditionString: unhandled case: %d", condition);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp", 2757, 0, v3) )
                __debugbreak();
            result = 0;
            break;
    }
    return result;
}

const char *__cdecl GetBodyPart(int bodypart)
{
    switch ( bodypart )
    {
        case 1:
            return "legs";
        case 2:
            return "torso";
        case 3:
            return "fullbody";
    }
    return "unknown body part";
}

animScriptCommand_t *__cdecl BG_AnimScriptEventGetCommand(playerState_s *ps, scriptAnimEventTypes_t event)
{
    animScript_t *script; // [esp+4h] [ebp-8h]
    animScriptItem_t *scriptItem; // [esp+8h] [ebp-4h]

    if (event != ANIM_ET_DEATH && ps->pm_type >= 9)
        return 0;
    if (G_IsServerGameSystem(ps->clientNum))
        Com_Printf(19, "event: %s\n", animEventTypesStr[event].string);
    script = &bgs->animData->animScriptData.scriptEvents[event];
    if (!script->numItems)
        return 0;
    scriptItem = BG_FirstValidItem(ps->clientNum, script);
    if (!scriptItem)
        return 0;
    if (scriptItem->numCommands)
        return &scriptItem->commands[bgs->Rand() % scriptItem->numCommands];
    return 0;
}

int __cdecl BG_AnimScriptEvent(pmove_t *pm, scriptAnimEventTypes_t event, int isContinue, int force)
{
    float v5; // [esp+0h] [ebp-20h]
    float v6; // [esp+4h] [ebp-1Ch]
    int i; // [esp+10h] [ebp-10h]
    animScriptCommand_t *scriptCommand; // [esp+14h] [ebp-Ch]
    float forcedAnimRate; // [esp+18h] [ebp-8h]
    float forcedAnimRatea; // [esp+18h] [ebp-8h]
    playerState_s *ps; // [esp+1Ch] [ebp-4h]

    ps = pm->ps;
    BG_AnimUpdatePlayerStateConditions(pm);
    scriptCommand = BG_AnimScriptEventGetCommand(ps, event);
    for (i = 0; i < 2; ++i)
    {
        if (scriptCommand
            && (bgs->animData->animScriptData.animations[scriptCommand->animIndex[i]].flags & 0x4000) != 0)
        {
            forcedAnimRate = (float)bgs->animData->animScriptData.animations[scriptCommand->animIndex[i]].duration / (float)ps->weaponTime;
            if (fabs(forcedAnimRate - 1.0) <= bg_maxWeaponAnimScale->current.value)
            {
                bgs->animData->animScriptData.animations[scriptCommand->animIndex[i]].forceAnimRate = forcedAnimRate;
            }
            else
            {
                if ((float)(forcedAnimRate - 1.0) <= 0.0)
                    v6 = 1.0 - bg_maxWeaponAnimScale->current.value;
                else
                    v6 = bg_maxWeaponAnimScale->current.value + 1.0;
                bgs->animData->animScriptData.animations[scriptCommand->animIndex[i]].forceAnimRate = v6;
            }
        }
        else if (scriptCommand
            && (bgs->animData->animScriptData.animations[scriptCommand->animIndex[i]].flags & 0x8000) != 0)
        {
            forcedAnimRatea = (float)bgs->animData->animScriptData.animations[scriptCommand->animIndex[i]].duration / (float)ps->weaponDelay;
            if (fabs(forcedAnimRatea - 1.0) <= bg_maxWeaponAnimScale->current.value)
            {
                bgs->animData->animScriptData.animations[scriptCommand->animIndex[i]].forceAnimRate = forcedAnimRatea;
            }
            else
            {
                if ((float)(forcedAnimRatea - 1.0) <= 0.0)
                    v5 = 1.0 - bg_maxWeaponAnimScale->current.value;
                else
                    v5 = bg_maxWeaponAnimScale->current.value + 1.0;
                bgs->animData->animScriptData.animations[scriptCommand->animIndex[i]].forceAnimRate = v5;
            }
        }
    }
    if (scriptCommand)
        return BG_ExecuteCommand(ps, scriptCommand, 1, isContinue, force);
    else
        return -1;
}

void __cdecl BG_SetConditionValue(unsigned int client, unsigned int condition, unsigned __int64 value)
{
    unsigned int ConditionBit; // eax
    char *ConditionString; // eax

    BG_CheckThread();
    iassert(bgs);
    if (condition > 0x16
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
            3450,
            0,
            "%s\n\t(condition) = %i",
            "(condition < NUM_ANIM_CONDITIONS && condition >= 0)",
            condition))
    {
        __debugbreak();
    }
    if (client >= 0x20
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
            3452,
            0,
            "client doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            client,
            32))
    {
        __debugbreak();
    }
    bgs->clientinfo[client].clientConditions[condition] = value;
    if (bgs->clientinfo[client].clientConditions[condition] != (_DWORD)value
        && G_IsServerGameSystem(client))
    {
        if (client >= 0x20
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                3461,
                0,
                "client doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                client,
                32))
        {
            __debugbreak();
        }
        ConditionBit = BG_GetConditionBit(&bgs->clientinfo[client], condition);
        ConditionString = BG_GetConditionString(condition, ConditionBit);
        Com_Printf(19, "condition: %s: %s\n", animConditionsStr[condition].string, ConditionString);
    }
}

void __cdecl BG_SetConditionBit(unsigned int client, int condition, int value)
{
    char *ConditionString; // eax

    BG_CheckThread();
    iassert(bgs);
    if (animConditionsTable[condition].type
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
            3481,
            0,
            "%s",
            "animConditionsTable[condition].type == ANIM_CONDTYPE_BITFLAGS"))
    {
        __debugbreak();
    }
    if (client >= 0x20
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
            3483,
            0,
            "client doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            client,
            32))
    {
        __debugbreak();
    }
    if (value >= 32
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
            3484,
            0,
            "%s",
            "value < MAX_PLAYERANIMTYPE_NAMES"))
    {
        __debugbreak();
    }
    if (client >= 0x20
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
            3487,
            0,
            "client doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            client,
            32))
    {
        __debugbreak();
    }
    if (!Com_BitCheckAssert(
        &bgs->clientinfo[client].clientConditions[condition],
        value,
        0xFFFFFFF)
        && G_IsServerGameSystem(client))
    {
        ConditionString = BG_GetConditionString(condition, value);
        Com_Printf(19, "condition: %s: %s\n", animConditionsStr[condition].string, ConditionString);
    }
    if (client >= 0x20
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
            3498,
            0,
            "client doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            client,
            32))
    {
        __debugbreak();
    }
    bgs->clientinfo[client].clientConditions[condition] = 0;
    Com_BitSetAssert(
        &bgs->clientinfo[client].clientConditions[condition],
        value,
        0xFFFFFFF);
}

void __cdecl Com_BitSetAssert(unsigned int *array, int bitNum, int size)
{
    if ( !array
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../universal/q_shared.h", 877, 0, "%s", "array") )
    {
        __debugbreak();
    }
    if ( bitNum >= (unsigned int)(8 * size)
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../universal/q_shared.h",
                    878,
                    0,
                    "bitNum doesn't index size * 8\n\t%i not in [0, %i)",
                    bitNum,
                    8 * size) )
    {
        __debugbreak();
    }
    array[bitNum >> 5] |= 1 << (bitNum & 0x1F);
}

unsigned int __cdecl BG_GetConditionValue(clientInfo_t *ci, unsigned int condition)
{
    if ( condition > 0x16
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                    3514,
                    0,
                    "%s\n\t(condition) = %i",
                    "(condition < NUM_ANIM_CONDITIONS && condition >= 0)",
                    condition) )
    {
        __debugbreak();
    }
    return ci->clientConditions[condition];
}

unsigned int __cdecl BG_GetConditionBit(const clientInfo_t *ci, unsigned int condition)
{
    unsigned int i; // [esp+0h] [ebp-8h]

    if ( condition > 0x16
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                    3529,
                    0,
                    "%s\n\t(condition) = %i",
                    "(condition < NUM_ANIM_CONDITIONS && condition >= 0)",
                    condition) )
    {
        __debugbreak();
    }
    if ( animConditionsTable[condition].type )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                        3530,
                        0,
                        "%s\n\t(animConditionsTable[condition].type) = %i",
                        "(animConditionsTable[condition].type == ANIM_CONDTYPE_BITFLAGS)",
                        animConditionsTable[condition].type) )
            __debugbreak();
    }
    for ( i = 0; i < 0x20; ++i )
    {
        if ( Com_BitCheckAssert(&ci->clientConditions[condition], i, 0xFFFFFFF) )
            return i;
    }
    return 0;
}

scriptAnimMoveTypes_t __cdecl BG_GetAnimMoveType(const clientInfo_t *ci)
{
    return (scriptAnimMoveTypes_t)ci->moveType; // KISAKTODO: odd
}

bgsAnim_s *__cdecl BG_GetAnimationForIndex(int client, unsigned int index)
{
    if (index >= bgs->animData->animScriptData.numAnimations)
        Com_Error(ERR_DROP, "BG_GetAnimationForIndex: index out of bounds");
    return (bgsAnim_s *)((char *)bgs->animData + 120 * index);
}

int __cdecl BG_GetPlayerStateWeaponIndex(const pmove_t *pm)
{
    int weapIndex; // [esp+0h] [ebp-4h]

    if ( (pm->ps->weapFlags & 2) == 0 )
        return pm->ps->weapon;
    weapIndex = pm->ps->offHandIndex;
    if ( !weapIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                    3578,
                    0,
                    "%s",
                    "weapIndex != WP_NONE") )
    {
        __debugbreak();
    }
    return weapIndex;
}

void __cdecl BG_AnimUpdatePlayerStateConditions(pmove_t *pmove)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 23; ++i )
        localClientConditionUpdateFunc[i](pmove);
}

bool __cdecl BG_IsCrouchingAnim(const clientInfo_t *ci, int animNum)
{
    return (BG_GetAnimationForIndex(ci->clientNum, animNum & 0xFFFFFBFF)->animScriptData.animations[0].stance & 2) != 0;
}

bool __cdecl BG_IsAds(const clientInfo_t *ci, int animNum)
{
    return (BG_GetAnimationForIndex(ci->clientNum, animNum & 0xFFFFFBFF)->animScriptData.animations[0].movestatus & 2) != 0;
}

bool __cdecl BG_IsProneAnim(const clientInfo_t *ci, int animNum)
{
    return (BG_GetAnimationForIndex(ci->clientNum, animNum & 0xFFFFFBFF)->animScriptData.animations[0].stance & 4) != 0;
}

bool __cdecl BG_IsKnifeMeleeAnim(const clientInfo_t *ci, int animNum)
{
    return (BG_GetAnimationForIndex(ci->clientNum, animNum & 0xFFFFFBFF)->animScriptData.animations[0].flags & 0x400) != 0;
}

bool __cdecl BG_IsGrenadeAnim(const clientInfo_t *ci, int animNum)
{
    bgsAnim_s *anim; // [esp+0h] [ebp-4h]

    anim = BG_GetAnimationForIndex(ci->clientNum, animNum & 0xFFFFFBFF);
    return bg_drawGrenadeInHand->current.enabled && (anim->animScriptData.animations[0].flags & 0x10000) != 0;
}

bool __cdecl BG_IsSliding(const clientInfo_t *ci)
{
    return BG_GetAnimMoveType(ci) == ANIM_MT_SLIDE;
}

void __cdecl BG_LerpOffset(float *offset_goal, float maxOffsetChange, float *offset)
{
    float diff; // [esp+18h] [ebp-10h]
    float diff_4; // [esp+1Ch] [ebp-Ch]
    float diff_8; // [esp+20h] [ebp-8h]
    float error; // [esp+24h] [ebp-4h]

    diff = *offset_goal - *offset;
    diff_4 = offset_goal[1] - offset[1];
    diff_8 = offset_goal[2] - offset[2];

    error = (float)((float)(diff * diff) + (float)(diff_4 * diff_4)) + (float)(diff_8 * diff_8);
    if ( error != 0.0 )
    {
        error = I_rsqrt(error) * maxOffsetChange;
        if ( error >= 1.0 )
        {
            *offset = *offset_goal;
            offset[1] = offset_goal[1];
            offset[2] = offset_goal[2];
        }
        else
        {
            *offset = (float)(error * diff) + *offset;
            offset[1] = (float)(error * diff_4) + offset[1];
            offset[2] = (float)(error * diff_8) + offset[2];
        }
    }
}

void __cdecl BG_Player_DoControllersInternal(const entityState_s *es, const clientInfo_t *ci, controller_info_t *info)
{
    float v3; // xmm0_4
    long double v4; // st7
    float *v5; // [esp+1Ch] [ebp-F8h]
    float *v6; // [esp+20h] [ebp-F4h]
    float v7; // [esp+30h] [ebp-E4h]
    float v8; // [esp+40h] [ebp-D4h]
    bool v9; // [esp+74h] [ebp-A0h]
    int lean_direction; // [esp+78h] [ebp-9Ch]
    float tag_origin_offset; // [esp+80h] [ebp-94h]
    float tag_origin_offset_4; // [esp+84h] [ebp-90h]
    float c; // [esp+8Ch] [ebp-88h]
    float vHeadAngles[3]; // [esp+90h] [ebp-84h] BYREF
    float vTorsoAngles[3]; // [esp+9Ch] [ebp-78h] BYREF
    float tag_origin_angles[3]; // [esp+A8h] [ebp-6Ch] BYREF
    float angles[6][3]; // [esp+B4h] [ebp-60h] BYREF
    __int64 moveType; // [esp+FCh] [ebp-18h]
    float s; // [esp+104h] [ebp-10h]
    float fLeanFrac; // [esp+108h] [ebp-Ch]
    int i; // [esp+10Ch] [ebp-8h]
    int clientNum; // [esp+110h] [ebp-4h]

    if ( (es->lerp.eFlags & 0x300) != 0
        || (es->lerp.eFlags & 0x4000) != 0 && (!es->lerp.u.player.vehicleSeat || es->lerp.u.player.vehicleSeat <= 4u) )
    {
        goto LABEL_7;
    }
    clientNum = es->clientNum;
    memset(tag_origin_angles, 0, sizeof(tag_origin_angles));
    memset(vTorsoAngles, 0, sizeof(vTorsoAngles));
    vHeadAngles[0] = ci->playerAngles[0];
    vHeadAngles[1] = ci->playerAngles[1];
    vHeadAngles[2] = ci->playerAngles[2];
    tag_origin_angles[1] = ci->legs.yawAngle;
    vTorsoAngles[1] = ci->torso.yawAngle;
    moveType = ci->moveType;
    if ( moveType == 24 )
    {
LABEL_7:
        memset(info, 0, sizeof(controller_info_t));
    }
    else
    {
        if ( (LODWORD(ci->moveType) != 5 || HIDWORD(ci->moveType)) && (LODWORD(ci->moveType) != 6 || HIDWORD(ci->moveType)) )
        {
            vTorsoAngles[0] = ci->torso.pitchAngle;
            if ( (es->lerp.eFlags & 8) != 0 )
            {
                vTorsoAngles[0] = AngleNormalize180(vTorsoAngles[0]);
                if ( vTorsoAngles[0] >= 0.0 )
                    vTorsoAngles[0] = vTorsoAngles[0] * 0.0;
                else
                    vTorsoAngles[0] = vTorsoAngles[0] * 0.5;
            }
        }
        AnglesSubtract(vHeadAngles, vTorsoAngles, vHeadAngles);
        AnglesSubtract(vTorsoAngles, tag_origin_angles, vTorsoAngles);
        tag_origin_offset = 0.0f;
        tag_origin_offset_4 = 0.0f;
        fLeanFrac = GetLeanFraction(ci->lerpLean);
        vTorsoAngles[2] = (float)(fLeanFrac * 50.0) * 0.92500001;
        vHeadAngles[2] = vTorsoAngles[2];
        if ( fLeanFrac != 0.0 )
        {
            lean_direction = fLeanFrac > 0.0;
            if ( (es->lerp.eFlags & 4) != 0 )
                v3 = (float)((-fLeanFrac) * player_lean_shift_crouch->current.vector[lean_direction]) + 0.0;
            else
                v3 = (float)((-fLeanFrac) * player_lean_shift->current.vector[lean_direction]) + 0.0;
            tag_origin_offset_4 = v3;
        }
        if ( (es->lerp.eFlags & 0x40000) == 0 )
            tag_origin_angles[1] = AngleNormalize180(tag_origin_angles[1] - ci->playerAngles[1]);
        memset(angles[5], 0, sizeof(float[3]));
        if ( (es->lerp.eFlags & 8) != 0 )
        {
            if ( fLeanFrac != 0.0 )
                vHeadAngles[2] = vHeadAngles[2] * 0.5;
            tag_origin_angles[0] = tag_origin_angles[0] + es->animState.fAimLeftRight;
            v4 = (float)(vTorsoAngles[1] * 0.017453292);
            c = cos(v4);
            s = sin(v4);
            tag_origin_offset = (float)((float)(1.0 - c) * 0.0) + 0.0;
            tag_origin_offset_4 = (float)(s * 0.0) + tag_origin_offset_4;
            if ( (float)(fLeanFrac * s) > 0.0 )
                tag_origin_offset_4 = (float)((float)((-fLeanFrac) * (float)(1.0 - c)) * 16.0) + tag_origin_offset_4;
            angles[0][0] = 0.0f;
            angles[0][1] = (float)(vTorsoAngles[1] * 0.30000001) + (float)(vTorsoAngles[2] * -1.2);
            angles[0][2] = vTorsoAngles[2] * 0.30000001;
            if ( es->animState.fAimUpDown != 0.0 || es->animState.fAimLeftRight != 0.0 )
            {
                v8 = AngleNormalize180(es->animState.fAimUpDown - es->animState.fAimLeftRight);
                angles[0][0] = angles[0][0] + v8;
            }
            angles[1][0] = 0.0f;
            angles[1][1] = (float)(vTorsoAngles[1] * 0.30000001) - (float)(vTorsoAngles[2] * 0.2);
            angles[1][2] = vTorsoAngles[2] * 0.2;
            angles[2][0] = 0.0f;
            angles[2][1] = (float)(vTorsoAngles[1] * 0.30000001) - (float)(vTorsoAngles[2] * -1.0);
            angles[2][2] = vTorsoAngles[2] * -0.2;
        }
        else
        {
            if ( fLeanFrac != 0.0 )
            {
                v9 = fLeanFrac > 0.0;
                if ( (es->lerp.eFlags & 4) != 0 )
                {
                    vTorsoAngles[2] = vTorsoAngles[2] * player_lean_rotate_crouch->current.vector[v9];
                    vHeadAngles[2] = vHeadAngles[2] * player_lean_rotate_crouch->current.vector[v9];
                }
                else
                {
                    vTorsoAngles[2] = vTorsoAngles[2] * player_lean_rotate->current.vector[v9];
                    vHeadAngles[2] = vHeadAngles[2] * player_lean_rotate->current.vector[v9];
                }
            }
            tag_origin_angles[2] = (float)((float)(fLeanFrac * 50.0) * 0.075000003) + tag_origin_angles[2];
            angles[0][0] = vTorsoAngles[0] * player_AimBlend_Back_Low->current.value;
            angles[0][1] = vTorsoAngles[1] * player_AimBlend_Back_Low->current.vector[1];
            angles[0][2] = vTorsoAngles[2] * player_AimBlend_Back_Low->current.vector[2];
            if ( es->animState.fAimUpDown != 0.0 || es->animState.fAimLeftRight != 0.0 )
            {
                v7 = AngleNormalize180(es->animState.fAimUpDown - es->animState.fAimLeftRight);
                angles[0][0] = angles[0][0] + v7;
            }
            angles[1][0] = vTorsoAngles[0] * player_AimBlend_Back_Mid->current.value;
            angles[1][1] = vTorsoAngles[1] * player_AimBlend_Back_Mid->current.vector[1];
            angles[1][2] = vTorsoAngles[2] * player_AimBlend_Back_Mid->current.vector[2];
            angles[2][0] = vTorsoAngles[0] * player_AimBlend_Back_Up->current.value;
            angles[2][1] = vTorsoAngles[1] * player_AimBlend_Back_Up->current.vector[1];
            angles[2][2] = vTorsoAngles[2] * player_AimBlend_Back_Up->current.vector[2];
            angles[5][0] = vTorsoAngles[0] * player_AimBlend_Pelvis->current.value;
            angles[5][1] = vTorsoAngles[1] * player_AimBlend_Pelvis->current.vector[1];
            angles[5][2] = vTorsoAngles[2] * player_AimBlend_Pelvis->current.vector[2];
        }
        angles[3][0] = vHeadAngles[0] * player_AimBlend_Neck->current.value;
        angles[3][1] = vHeadAngles[1] * player_AimBlend_Neck->current.vector[1];
        angles[3][2] = vHeadAngles[2] * player_AimBlend_Neck->current.vector[2];
        angles[4][0] = vHeadAngles[0] * player_AimBlend_Head->current.value;
        angles[4][1] = vHeadAngles[1] * player_AimBlend_Head->current.vector[1];
        angles[4][2] = vHeadAngles[2] * player_AimBlend_Head->current.vector[2];
        if ( es->animState.fAimLeftRight != 0.0 || es->animState.fAimUpDown != 0.0 )
            angles[5][0] = AngleNormalize180(es->animState.fAimLeftRight - es->animState.fAimUpDown);
        for ( i = 0; i < 6; ++i )
        {
            v5 = info->angles[i];
            v6 = angles[i];
            *v5 = *v6;
            v5[1] = v6[1];
            v5[2] = v6[2];
        }
        info->tag_origin_angles[0] = tag_origin_angles[0];
        info->tag_origin_angles[1] = tag_origin_angles[1];
        info->tag_origin_angles[2] = tag_origin_angles[2];
        info->tag_origin_offset[0] = tag_origin_offset;
        info->tag_origin_offset[1] = tag_origin_offset_4;
        info->tag_origin_offset[2] = 0.0f;
    }
}

void __cdecl BG_Player_DoControllersSetup(const entityState_s *es, clientInfo_t *ci, int frametime)
{
    int v3; // [esp+8h] [ebp-74h]
    controller_info_t info; // [esp+Ch] [ebp-70h] BYREF
    float maxAngleChange; // [esp+70h] [ebp-Ch]
    int time; // [esp+74h] [ebp-8h]
    int i; // [esp+78h] [ebp-4h]

    if ((es->lerp.eFlags & 8) != 0)
    {
        time = bgs->proneTime[es->number];
        if (time >= 500)
            v3 = 500;
        else
            v3 = time;
        bgs->proneTime[es->number] = v3;
    }
    else
    {
        time = bgs->proneTime[es->number] - frametime;
        bgs->proneTime[es->number] = time <= 0 ? 0 : time;
    }
    BG_Player_DoControllersInternal(es, (const clientInfo_t *)ci, &info);
    maxAngleChange = (float)frametime * 0.30000001;
    for (i = 0; i < 6; ++i)
        BG_LerpAngles(info.angles[i], maxAngleChange, ci->control.angles[i]);
    BG_LerpAngles(info.tag_origin_angles, maxAngleChange, ci->control.tag_origin_angles);
    BG_LerpOffset(info.tag_origin_offset, (float)frametime * 0.1, ci->control.tag_origin_offset);
}

void __cdecl BG_LerpAngles(float *angles_goal, float maxAngleChange, float *angles)
{
    float diff; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    for ( i = 0; i < 3; ++i )
    {
        diff = angles_goal[i] - angles[i];
        if ( diff <= maxAngleChange )
        {
            if ( (-maxAngleChange) <= diff )
                angles[i] = angles_goal[i];
            else
                angles[i] = angles[i] - maxAngleChange;
        }
        else
        {
            angles[i] = angles[i] + maxAngleChange;
        }
    }
}

void __cdecl BG_PlayerAnimation(int localClientNum, const entityState_s *es, clientInfo_t *ci)
{
    BG_PlayerAngles(es, ci);
    BG_AnimPlayerConditions(es, ci);
    BG_PlayerAnimation_VerifyAnim(ci->pXAnimTree, &ci->legs);
    BG_PlayerAnimation_VerifyAnim(ci->pXAnimTree, &ci->torso);
    if ( ci->leftHandGun && (ci->torso.animationNumber & 0xFFFFFBFF) == 0 )
    {
        ci->leftHandGun = 0;
        ci->dobjDirty = 1;
    }
    BG_RunLerpFrameRate(localClientNum, ci, &ci->legs, es->animState.state, es);
    BG_RunLerpFrameRate(localClientNum, ci, &ci->torso, es->anim.torsoAnim, es);
}

void __cdecl BG_RunLerpFrameRate(
                int localClientNum,
                clientInfo_t *ci,
                lerpFrame_t *lf,
                int newAnimation,
                const entityState_s *es)
{
    bool v5; // [esp+4h] [ebp-68h]
    bool v6; // [esp+8h] [ebp-64h]
    float v7; // [esp+1Ch] [ebp-50h]
    float v[2]; // [esp+30h] [ebp-3Ch] BYREF
    float v9; // [esp+38h] [ebp-34h]
    float *oldFramePos; // [esp+3Ch] [ebp-30h]
    const float *trBase; // [esp+40h] [ebp-2Ch]
    float fScaleMax; // [esp+44h] [ebp-28h]
    int bNewAnim; // [esp+48h] [ebp-24h]
    float moveSpeed; // [esp+4Ch] [ebp-20h]
    int isScriptEventAnim; // [esp+50h] [ebp-1Ch]
    float rotSpeed; // [esp+54h] [ebp-18h]
    animation_s *anim; // [esp+58h] [ebp-14h]
    animation_s *oldAnim; // [esp+5Ch] [ebp-10h]
    int isLadderAnim; // [esp+60h] [ebp-Ch]
    XAnimTree_s *pAnimTree; // [esp+64h] [ebp-8h]
    XAnim_s *pXAnims; // [esp+68h] [ebp-4h]

    bNewAnim = 0;
    BG_CheckThread();
    iassert(bgs);
    v6 = lf->animation && (lf->animation->flags & 2) != 0;
    isLadderAnim = v6;
    v5 = lf->animation && (lf->animation->flags & 0x800) != 0;
    isScriptEventAnim = v5;
    oldAnim = lf->animation;
    pAnimTree = ci->pXAnimTree;
    pXAnims = bgs->animData->animScriptData.animTree.anims;
    if ( newAnimation != lf->animationNumber || !lf->animation && (newAnimation & 0xFFFFFBFF) != 0 )
    {
        BG_SetNewAnimation(localClientNum, ci, lf, newAnimation, es);
        bNewAnim = 1;
    }
    if ( (newAnimation & 0xFFFFFBFF) != 0 && lf->animation )
    {
        anim = lf->animation;
        if ( anim->forceAnimRate <= 0.0 )
        {
            if ( !anim || anim->rotSpeed == 0.0 && anim->moveSpeed == 0.0 || !lf->oldFrameSnapshotTime )
            {
                lf->animSpeedScale = 1.0f;
                lf->oldFrameSnapshotTime = bgs->latestSnapshotTime;
                lf->oldFramePos[0] = es->lerp.pos.trBase[0];
                lf->oldFramePos[1] = es->lerp.pos.trBase[1];
                lf->oldFramePos[2] = es->lerp.pos.trBase[2];
                lf->oldFrameYaw = es->lerp.apos.trBase[1];
            }
            else if (bgs->latestSnapshotTime != lf->oldFrameSnapshotTime)
            {
                if ( isLadderAnim )
                {
                    v9 = lf->oldFramePos[2] - es->lerp.pos.trBase[2];
                    moveSpeed = fabs(v9);
                }
                else
                {
                    v[0] = es->lerp.pos.trBase[0] - lf->oldFramePos[0];
                    v[1] = es->lerp.pos.trBase[1] - lf->oldFramePos[1];
                    moveSpeed = Vec2Length(v);
                }
                moveSpeed = moveSpeed / (float)((float)(bgs->latestSnapshotTime - lf->oldFrameSnapshotTime) * 0.001);
                v7 = AngleNormalize180(lf->oldFrameYaw - es->lerp.apos.trBase[1]);
                rotSpeed = fabs(v7);
                rotSpeed = rotSpeed
                    / (float)((float)(bgs->latestSnapshotTime
                        - lf->oldFrameSnapshotTime)
                        * 0.001);
                if ( anim->moveSpeed == 0.0
                    && anim->rotSpeed == 0.0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                                4040,
                                0,
                                "%s",
                                "(anim->moveSpeed || anim->rotSpeed)") )
                {
                    __debugbreak();
                }
                if ( anim->moveSpeed == 0.0 )
                {
                    lf->animSpeedScale = rotSpeed / anim->rotSpeed;
                    if ( lf->animSpeedScale <= 1.0 )
                    {
                        if ( lf->animSpeedScale < 0.40000001 )
                            lf->animSpeedScale = 0.4f;
                    }
                    else
                    {
                        lf->animSpeedScale = 1.0f;
                    }
                }
                else
                {
                    lf->animSpeedScale = moveSpeed / anim->moveSpeed;
                }
                lf->oldFrameSnapshotTime = bgs->latestSnapshotTime;
                if ( isScriptEventAnim )
                    lf->animSpeedScale = 1.0f;
                lf->oldFramePos[0] = es->lerp.pos.trBase[0];
                lf->oldFramePos[1] = es->lerp.pos.trBase[1];
                lf->oldFramePos[2] = es->lerp.pos.trBase[2];
                lf->oldFrameYaw = es->lerp.apos.trBase[1];
                if ( lf->animSpeedScale >= 0.1 )
                {
                    if ( lf->animSpeedScale > 2.0 )
                    {
                        if ( (anim->flags & 2) != 0 )
                        {
                            if ( lf->animSpeedScale > 4.0 )
                                lf->animSpeedScale = 4.0f;
                        }
                        else if ( anim->moveSpeed <= 150.0 )
                        {
                            if ( anim->moveSpeed >= 20.0 )
                            {
                                fScaleMax = 3.0 - (float)((float)((float)(anim->moveSpeed - 20.0) * 1.0) / 130.0);
                                if ( lf->animSpeedScale > fScaleMax )
                                    lf->animSpeedScale = fScaleMax;
                            }
                            else if ( lf->animSpeedScale > 3.0 )
                            {
                                lf->animSpeedScale = 3.0f;
                            }
                        }
                        else
                        {
                            lf->animSpeedScale = 2.0f;
                        }
                    }
                }
                else if ( lf->animSpeedScale < 0.0099999998 && isLadderAnim )
                {
                    lf->animSpeedScale = 0.0f;
                }
                else
                {
                    lf->animSpeedScale = 0.1f;
                }
            }
        }
        else
        {
            lf->animSpeedScale = anim->forceAnimRate;
            oldFramePos = lf->oldFramePos;
            trBase = es->lerp.pos.trBase;
            lf->oldFramePos[0] = es->lerp.pos.trBase[0];
            oldFramePos[1] = trBase[1];
            oldFramePos[2] = trBase[2];
            lf->oldFrameYaw = es->lerp.apos.trBase[1];
            lf->oldFrameSnapshotTime = bgs->latestSnapshotTime;
        }
        if ( lf->animationNumber )
            XAnimSetAnimRate(pAnimTree, lf->animationNumber & 0xFFFFFBFF, lf->animSpeedScale);
    }
}

void __cdecl BG_SetNewAnimation(
    int localClientNum,
    clientInfo_t *ci,
    lerpFrame_t *lf,
    int newAnimation,
    const entityState_s *es)
{
    BOOL IsCrouchingAnim; // esi
    BOOL IsProneAnim; // esi
    BOOL v7; // [esp+20h] [ebp-50h]
    animation_s *oldanim; // [esp+28h] [ebp-48h]
    int firstAnim; // [esp+2Ch] [ebp-44h]
    int transitionMin; // [esp+30h] [ebp-40h]
    DObj *obj; // [esp+34h] [ebp-3Ch]
    int oldAnimNum; // [esp+38h] [ebp-38h]
    float fStartTime; // [esp+3Ch] [ebp-34h]
    float fStartTimea; // [esp+3Ch] [ebp-34h]
    int cycleLen; // [esp+40h] [ebp-30h]
    unsigned int animIndex; // [esp+44h] [ebp-2Ch]
    BOOL crouchMatch; // [esp+48h] [ebp-28h]
    int stanceTransitionTime; // [esp+4Ch] [ebp-24h]
    animation_s *anim; // [esp+5Ch] [ebp-14h]
    XAnimTree_s *pAnimTree; // [esp+60h] [ebp-10h]
    XAnim_s *pXAnims; // [esp+64h] [ebp-Ch]
    BOOL proneMatch; // [esp+68h] [ebp-8h]
    BOOL bNew; // [esp+6Ch] [ebp-4h]
    unsigned int newAnimationa; // [esp+84h] [ebp+14h]

    transitionMin = -1;
    firstAnim = 0;
    fStartTime = 0.0f;
    BG_CheckThread();
    iassert(bgs);
    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        bNew = (es->lerp.eFlags & 0x10) != 0;
        oldanim = lf->animation;
        oldAnimNum = lf->animationNumber;
        if (!oldanim)
            firstAnim = 1;
        lf->animationNumber = newAnimation;
        newAnimationa = newAnimation & 0xFFFFFBFF;
        if (newAnimationa >= bgs->animData->animScriptData.numAnimations)
            Com_Error(
                ERR_DROP,
                "Player animation index out of range (%i): %i",
                bgs->animData->animScriptData.numAnimations,
                newAnimationa);
        pAnimTree = ci->pXAnimTree;
        pXAnims = bgs->animData->animScriptData.animTree.anims;
        if (newAnimationa)
        {
            anim = &bgs->animData->animScriptData.animations[newAnimationa];
            lf->animation = anim;
            if (anim->initialLerp >= 0
                || bgs->animData->animScriptData.animations[newAnimationa].finalLerp < 0)
            {
                lf->animationTime = anim->initialLerp;
            }
            else
            {
                lf->animationTime = bgs->animData->animScriptData.animations[newAnimationa].finalLerp;
            }
            if (bg_blendTimeOverride->current.integer >= 0)
                lf->animationTime = bg_blendTimeOverride->current.integer;
            IsCrouchingAnim = BG_IsCrouchingAnim(ci, oldAnimNum);
            crouchMatch = BG_IsCrouchingAnim(ci, newAnimationa) == IsCrouchingAnim;
            IsProneAnim = BG_IsProneAnim(ci, oldAnimNum);
            proneMatch = BG_IsProneAnim(ci, newAnimationa) == IsProneAnim;
            if (lf == &ci->legs && (!crouchMatch || !proneMatch))
                ci->stanceTransitionTime = bgs->time + 400;
        }
        else
        {
            anim = 0;
            lf->animation = 0;
            lf->animationTime = 200;
        }
        if (firstAnim && lf == &ci->legs || Demo_IsPaused())
        {
            lf->animationTime = 0;
        }
        else
        {
            if (!anim || lf->animationTime < 0)
            {
                if (anim && (anim->flags & 4) != 0)
                {
                    transitionMin = 0;
                }
                else if (oldanim
                    && (oldanim->flags & 0x8000000) != 0
                    && anim
                    && (anim->moveSpeed != 0.0 || anim->rotSpeed != 0.0))
                {
                    transitionMin = 267;
                }
                else if (!anim || anim->moveSpeed == 0.0)
                {
                    if (!oldanim || oldanim->moveSpeed == 0.0)
                        transitionMin = 167;
                    else
                        transitionMin = 267;
                }
                else
                {
                    transitionMin = 133;
                }
            }
            stanceTransitionTime = ci->stanceTransitionTime - bgs->time;
            if (stanceTransitionTime > transitionMin)
                transitionMin = stanceTransitionTime;
            if (lf->animationTime < transitionMin)
                lf->animationTime = transitionMin;
        }
        if (anim && anim->moveSpeed != 0.0 && XAnimIsLooped(pXAnims, newAnimationa))
        {
            animIndex = oldAnimNum & 0xFFFFFBFF;
            if (oldanim && oldanim->moveSpeed != 0.0 && XAnimIsLooped(pXAnims, animIndex))
            {
                fStartTime = XAnimGetTime(pAnimTree, animIndex);
            }
            else if (firstAnim)
            {
                if (XAnimIsPrimitive(pXAnims, animIndex))
                    cycleLen = XAnimGetLengthMsec(pXAnims, animIndex) + 200;
                else
                    cycleLen = 1000;
                fStartTimea = (float)((float)(bgs->time % cycleLen)
                    / (float)cycleLen)
                    + (float)((float)ci->clientNum * 0.36000001);
                fStartTime = fStartTimea - (float)(int)fStartTimea;
            }
        }
        if (oldanim)
            XAnimClearTreeGoalWeights(pAnimTree, oldAnimNum & 0xFFFFFBFF, (float)lf->animationTime * 0.001, -1);
        if (newAnimationa)
        {
            if (lf != &ci->legs && ci->leftHandGun)
            {
                ci->leftHandGun = 0;
                ci->dobjDirty = 1;
            }
            if ((anim->flags & 0x100) != 0)
            {
                if (XAnimIsLooped(pXAnims, newAnimationa))
                    Com_Error(ERR_DROP, "death animation '%s' is looping", anim->name);
                if (bNew)
                {
                    XAnimSetCompleteGoalWeight(obj, newAnimationa, 1.0, (float)lf->animationTime * 0.001, 1.0, 0, 0, 0, -1);
                }
                else
                {
                    XAnimSetGoalWeightKnobAll(obj, newAnimationa, 0, 1.0, 0.0, 1.0, 0, 0, 0, -1);
                    XAnimSetTime(pAnimTree, newAnimationa, 1.0, 0xFFFFu);
                }
            }
            else
            {
                v7 = anim->moveSpeed != 0.0 && XAnimGetWeight(pAnimTree, newAnimationa) == 0.0 || firstAnim;
                if (localClientNum >= 0)
                    XAnimSetCompleteGoalWeight(
                        obj,
                        newAnimationa,
                        1.0,
                        (float)lf->animationTime * 0.001,
                        1.0,
                        0,
                        anim->noteType | 2,
                        lf != &ci->legs,
                        -1);
                else
                    XAnimSetCompleteGoalWeight(
                        obj,
                        newAnimationa,
                        1.0,
                        (float)lf->animationTime * 0.001,
                        1.0,
                        0,
                        anim->noteType,
                        lf != &ci->legs,
                        -1);
                if (Demo_IsPaused() && (anim->flags & 0x400) != 0)
                {
                    XAnimSetTime(pAnimTree, newAnimationa, 0.1, 0xFFFFu);
                }
                else if (v7)
                {
                    XAnimSetTime(pAnimTree, newAnimationa, fStartTime, 0xFFFFu);
                }
            }
            if (lf != &ci->legs)
            {
                XAnimSetCompleteGoalWeight(
                    obj,
                    bgs->animData->animScriptData.torsoAnim,
                    1.0,
                    (float)lf->animationTime * 0.001,
                    1.0,
                    0,
                    anim->noteType,
                    0,
                    -1);
                XAnimSetCompleteGoalWeight(
                    obj,
                    bgs->animData->animScriptData.legsAnim,
                    0.0099999998,
                    (float)lf->animationTime * 0.001,
                    1.0,
                    0,
                    anim->noteType,
                    0,
                    -1);
            }
        }
        else if (lf != &ci->legs)
        {
            XAnimSetCompleteGoalWeight(
                obj,
                bgs->animData->animScriptData.torsoAnim,
                0.0,
                (float)lf->animationTime * 0.001,
                1.0,
                0,
                0,
                0,
                -1);
            XAnimSetCompleteGoalWeight(
                obj,
                bgs->animData->animScriptData.legsAnim,
                1.0,
                (float)lf->animationTime * 0.001,
                1.0,
                0,
                0,
                0,
                -1);
        }
    }
}

void __cdecl BG_PlayerAnimation_VerifyAnim(XAnimTree_s *pAnimTree, lerpFrame_t *lf)
{
    BG_CheckThread();
    iassert(bgs);
    if ( lf->animationNumber )
    {
        if ( XAnimGetWeight(pAnimTree, lf->animationNumber & 0xFFFFFBFF) == 0.0 )
        {
            lf->animationNumber = 0;
            lf->animation = 0;
            lf->animationTime = 167;
        }
    }
    else if ( lf->animation
                 && !Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                             4157,
                             0,
                             "%s",
                             "!lf->animation") )
    {
        __debugbreak();
    }
}

void __cdecl BG_PlayerAngles(const entityState_s *es, clientInfo_t *ci)
{
    bool bTeleportBit; // [esp+1Fh] [ebp-3Dh]
    float vHeadAngles; // [esp+20h] [ebp-3Ch]
    float vHeadAngles_4; // [esp+24h] [ebp-38h]
    float swingSpeed; // [esp+2Ch] [ebp-30h]
    float vTorsoAngles_4; // [esp+34h] [ebp-28h]
    float clampTolerance; // [esp+3Ch] [ebp-20h]
    float vLegsAngles_4; // [esp+44h] [ebp-18h]
    float MAX_PITCH_FRACTION; // [esp+50h] [ebp-Ch]
    float moveDir; // [esp+58h] [ebp-4h]

    BG_CheckThread();
    iassert(bgs);
    GetLeanFraction(ci->lerpLean);
    moveDir = ci->lerpMoveDir;
    if ( bg_teleportAlignTime->current.integer > 0 )
    {
        bTeleportBit = (es->lerp.eFlags & 2) != 0;
        if ( bTeleportBit != ci->teleportBit )
        {
            ci->teleportTime = bgs->time;
            ci->teleportBit = bTeleportBit;
        }
    }
    vHeadAngles = ci->playerAngles[0];
    vHeadAngles_4 = AngleNormalize360(ci->playerAngles[1]);
    if ( player_topDownCamMode->current.integer <= 0 )
    {
        if ( (es->lerp.eFlags & 8) != 0 )
            swingSpeed = bg_proneSwingSpeed->current.value;
        else
            swingSpeed = bg_swingSpeed->current.value;
    }
    else
    {
        swingSpeed = 1.0f;
    }
    if ( (es->lerp.eFlags & 0x300) != 0 )
    {
        ci->torso.yawing = 1;
        ci->torso.pitching = 1;
        ci->legs.yawing = 1;
        goto LABEL_25;
    }
    if ( es->lerp.u.actor.team == 5 || es->lerp.u.actor.team == 6 )
        goto LABEL_20;
    if ( (es->lerp.eFlags & 0x8000) != 0 )
        goto LABEL_18;
    if ( es->lerp.u.actor.team != 1 )
    {
LABEL_20:
        ci->torso.yawing = 1;
        ci->torso.pitching = 1;
        ci->legs.yawing = 1;
        goto LABEL_25;
    }
    if (bg_teleportAlignTime->current.integer > 0 && ci->teleportTime > bgs->time - bg_teleportAlignTime->current.integer)
    {
LABEL_18:
        ci->torso.yawing = 1;
        ci->torso.pitching = 1;
        ci->legs.yawing = 1;
        goto LABEL_25;
    }
    if ( BG_GetConditionValue(ci, 0xBu) )
    {
        ci->torso.yawing = 1;
        ci->torso.pitching = 1;
    }
LABEL_25:
    vLegsAngles_4 = vHeadAngles_4 + moveDir;
    if ( (es->lerp.eFlags & 0x40000) != 0 )
    {
        vLegsAngles_4 = vHeadAngles_4;
        vTorsoAngles_4 = vHeadAngles_4;
        clampTolerance = 90.0f;
    }
    else if ( es->lerp.u.actor.team == 5 || es->lerp.u.actor.team == 6 )
    {
        vTorsoAngles_4 = vHeadAngles_4 + moveDir;
        clampTolerance = 0.0f;
    }
    else if ( (es->lerp.eFlags & 0x8000) != 0 )
    {
        vTorsoAngles_4 = vHeadAngles_4;
        vLegsAngles_4 = vHeadAngles_4;
        clampTolerance = 90.0f;
    }
    else if ( ci->needsRevive )
    {
        vLegsAngles_4 = vHeadAngles_4;
        vTorsoAngles_4 = vHeadAngles_4;
        clampTolerance = 45.0f;
    }
    else if ( (es->lerp.eFlags & 8) != 0 )
    {
        vTorsoAngles_4 = vHeadAngles_4;
        clampTolerance = 90.0f;
    }
    else if ( (es->lerp.eFlags & 0x40) != 0 )
    {
        vTorsoAngles_4 = vHeadAngles_4;
        clampTolerance = 45.0f;
    }
    else
    {
        if ( (es->lerp.eFlags & 0x80000) != 0 )
            vTorsoAngles_4 = vHeadAngles_4;
        else
            vTorsoAngles_4 = (float)(player_move_factor_on_torso->current.value * moveDir) + vHeadAngles_4;
        clampTolerance = 90.0f;
    }
    BG_SwingAngles(vTorsoAngles_4, 0.0, clampTolerance, swingSpeed, &ci->torso.yawAngle, &ci->torso.yawing);

    iassert(bgs->animData->animScriptData.animations);
    
    if ( (es->lerp.eFlags & 0x40000) != 0 )
        goto LABEL_46;

    if ((bgs->animData->animScriptData.animations[es->animState.state & 0xFFFFFBFF].flags
        & 0x30) != 0)
    {
        ci->legs.yawing = 0;
        BG_SwingAngles(vHeadAngles_4, 0.0, 150.0, swingSpeed, &ci->legs.yawAngle, &ci->legs.yawing);
    }
    else
    {
        if ( ci->legs.yawing )
        {
LABEL_46:
            BG_SwingAngles(vLegsAngles_4, 0.0, 150.0, swingSpeed, &ci->legs.yawAngle, &ci->legs.yawing);
            goto LABEL_55;
        }
        if ( (es->lerp.eFlags & 8) != 0 )
        {
            BG_SwingAngles(
                vLegsAngles_4,
                bg_legYawProneTolerance->current.value,
                150.0,
                swingSpeed,
                &ci->legs.yawAngle,
                &ci->legs.yawing);
        }
        else if ( (es->lerp.eFlags & 4) != 0 )
        {
            BG_SwingAngles(
                vLegsAngles_4,
                bg_legYawCrouchTolerance->current.value,
                150.0,
                swingSpeed,
                &ci->legs.yawAngle,
                &ci->legs.yawing);
        }
        else
        {
            BG_SwingAngles(
                vLegsAngles_4,
                bg_legYawTolerance->current.value,
                150.0,
                swingSpeed,
                &ci->legs.yawAngle,
                &ci->legs.yawing);
        }
    }
LABEL_55:
    if ( (es->lerp.eFlags & 0x300) != 0 )
    {
        ci->torso.yawAngle = vHeadAngles_4;
        ci->legs.yawAngle = vHeadAngles_4;
    }
    else if ( es->lerp.u.actor.team == 5 || es->lerp.u.actor.team == 6 )
    {
        ci->torso.yawAngle = vHeadAngles_4 + moveDir;
        ci->legs.yawAngle = vHeadAngles_4 + moveDir;
    }
    MAX_PITCH_FRACTION = 2.0f;

    static const float IK_MAX_PITCH_FRACTION = 1.0f;

    if ( IKImport_GetVar_IK_Enable() )
        MAX_PITCH_FRACTION = IK_MAX_PITCH_FRACTION;

    if ( (es->lerp.eFlags & 0x40000) != 0
        || (es->lerp.eFlags & 0x300) != 0
        || es->lerp.u.actor.team == 5
        || es->lerp.u.actor.team == 6
        || es->lerp.eFlags == 0x8000 )
    {
        BG_SwingAngles(0.0, 0.0, 45.0, 0.15000001, &ci->torso.pitchAngle, &ci->torso.pitching);
    }
    else if ( vHeadAngles <= 180.0 )
    {
        BG_SwingAngles(vHeadAngles * MAX_PITCH_FRACTION, 0.0, 45.0, 0.15000001, &ci->torso.pitchAngle, &ci->torso.pitching);
    }
    else
    {
        BG_SwingAngles(
            (float)(vHeadAngles + -360.0) * MAX_PITCH_FRACTION,
            0.0,
            45.0,
            0.15000001,
            &ci->torso.pitchAngle,
            &ci->torso.pitching);
    }
}

void __cdecl BG_SwingAngles(
                float destination,
                float swingTolerance,
                float clampTolerance,
                float speed,
                float *angle,
                int *swinging)
{
    float move; // [esp+1Ch] [ebp-Ch]
    float swing; // [esp+20h] [ebp-8h]
    float swinga; // [esp+20h] [ebp-8h]
    float swingb; // [esp+20h] [ebp-8h]
    float scale; // [esp+24h] [ebp-4h]

    BG_CheckThread();
    iassert(bgs);
    
    if ( !*swinging )
    {
        swing = AngleNormalize180(*angle - destination);
        if ( swing > swingTolerance || -swingTolerance > swing )
            *swinging = 1;
    }
    if ( *swinging )
    {
        swinga = AngleNormalize180(destination - *angle);
        scale = fabs(swinga) * 0.050000001;
        if ( scale < 0.5 )
            scale = 0.5f;
        if ( swinga < 0.0 )
        {
            move = (float)((float)bgs->frametime * scale) * (-speed);
            if ( swinga < move )
            {
                *swinging = 1;
            }
            else
            {
                move = swinga;
                *swinging = 0;
            }
        }
        else
        {
            move = (float)((float)bgs->frametime * scale) * speed;
            if ( move < swinga )
            {
                *swinging = 1;
            }
            else
            {
                move = swinga;
                *swinging = 0;
            }
        }
        *angle = AngleNormalize360(*angle + move);
        swingb = AngleNormalize180(destination - *angle);
        if ( swingb <= clampTolerance )
        {
            if ( -clampTolerance > swingb)
                *angle = AngleNormalize360(destination + clampTolerance);
        }
        else
        {
            *angle = AngleNormalize360(destination - clampTolerance);
        }
    }
}

void __cdecl BG_AnimPlayerConditions(const entityState_s *es, const clientInfo_t *ci)
{
    for ( int i = 0; i < ARRAY_COUNT(clientConditionUpdateFunc); ++i )
        clientConditionUpdateFunc[i](es, ci);
}

void __cdecl BG_UpdatePlayerDObj(
                int localClientNum,
                DObj *pDObj,
                entityState_s *es,
                clientInfo_t *ci,
                int attachIgnoreCollision)
{
    const WeaponVariantDef *weapVariantDefStowed; // [esp+4h] [ebp-15Ch]
    const WeaponDef *weapDefDW; // [esp+8h] [ebp-158h]
    const WeaponVariantDef *weapVariantDefDW; // [esp+Ch] [ebp-154h]
    unsigned __int8 index; // [esp+13h] [ebp-14Dh] BYREF
    int b; // [esp+14h] [ebp-14Ch]
    const XModel *model; // [esp+18h] [ebp-148h]
    int m; // [esp+1Ch] [ebp-144h]
    int iStowedWeaponModelIndex; // [esp+20h] [ebp-140h]
    unsigned __int8 boneIndex; // [esp+27h] [ebp-139h] BYREF
    int iNumModels; // [esp+28h] [ebp-138h]
    DObj *dobj; // [esp+2Ch] [ebp-134h]
    int ikActive; // [esp+30h] [ebp-130h]
    int tagIndex; // [esp+34h] [ebp-12Ch]
    unsigned int iClientOffhandWeapon; // [esp+38h] [ebp-128h]
    int lastWeaponModel; // [esp+3Ch] [ebp-124h]
    const WeaponDef *weapDefStowed; // [esp+40h] [ebp-120h]
    const WeaponVariantDef *weapVariantDef; // [esp+44h] [ebp-11Ch]
    unsigned int iClientWeapon; // [esp+48h] [ebp-118h]
    int firstWeaponModel; // [esp+4Ch] [ebp-114h]
    int i; // [esp+50h] [ebp-110h]
    XAnimTree_s *pAnimTree; // [esp+54h] [ebp-10Ch]
    int objExists; // [esp+58h] [ebp-108h]
    const WeaponDef *weapDef; // [esp+5Ch] [ebp-104h]
    DObjModel_s dobjModels[32]; // [esp+60h] [ebp-100h] BYREF

    ikActive = 0;
    BG_CheckThread();
    iassert(bgs);
    objExists = pDObj != 0;
    iClientWeapon = BG_GetPlayerWeaponForDObj(localClientNum, es);
    iClientOffhandWeapon = BG_GetPlayerOffhandWeaponForDObj(localClientNum, es);
    pAnimTree = ci->pXAnimTree;
    iStowedWeaponModelIndex = 0;
    weapDefStowed = BG_GetWeaponDef(es->lerp.u.player.stowedWeapon);
    if ( pAnimTree && (!ci->infoValid || !ci->model[0]) )
    {
        XAnimClearTree(pAnimTree);
        bgs->SafeDObjFree(es->number, localClientNum);
        return;
    }
    if ( objExists )
    {
        if ( ci->iDObjWeapon == iClientWeapon
            && ci->weaponModel == es->weaponModel
            && ci->iDObjMeleeWeapon == es->lerp.u.player.meleeWeapon
            && ci->meleeWeaponModel == es->lerp.u.player.meleeWeaponModel
            && !ci->dobjDirty )
        {
            if ( pAnimTree != DObjGetTree(pDObj)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                            5061,
                            0,
                            "%s",
                            "pAnimTree == DObjGetTree( pDObj )") )
            {
                __debugbreak();
            }
            return;
        }
        ikActive = DObjHasIKActiveFlag(pDObj);
        bgs->SafeDObjFree(es->number, localClientNum);
    }
    ci->iDObjMeleeWeapon = es->lerp.u.player.meleeWeapon;
    ci->meleeWeaponModel = es->lerp.u.player.meleeWeaponModel;
    ci->iDObjOffhandWeapon = es->lerp.u.player.offhandWeapon;
    ci->offhandWeaponModel = es->lerp.u.player.offhandWeaponModel;
    iNumModels = 0;
    dobjModels[0].model = bgs->GetXModel(ci->model);
    if ( !dobjModels[0].model
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                    5084,
                    0,
                    "%s",
                    "dobjModels[iNumModels].model") )
    {
        __debugbreak();
    }
    dobjModels[iNumModels].boneName = 0;
    dobjModels[iNumModels++].ignoreCollision = 0;
    ci->iDObjWeapon = iClientWeapon;
    ci->weaponModel = es->weaponModel;
    firstWeaponModel = iNumModels;
    if (bgs->AttachWeapon)
        iNumModels = bgs->AttachWeapon(dobjModels, iNumModels, ci);
    lastWeaponModel = iNumModels;
    for ( i = 0; i < 6 && ci->attachModelNames[i][0]; ++i )
    {
        if ( iNumModels >= 32
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                        5105,
                        0,
                        "%s",
                        "iNumModels < DOBJ_MAX_SUBMODELS") )
        {
            __debugbreak();
        }
        dobjModels[iNumModels].model = bgs->GetXModel(ci->attachModelNames[i]);
        if ( !dobjModels[iNumModels].model
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                        5107,
                        0,
                        "%s",
                        "dobjModels[iNumModels].model") )
        {
            __debugbreak();
        }
        dobjModels[iNumModels].boneName = SL_FindString(ci->attachTagNames[i], SCRIPTINSTANCE_SERVER);
        dobjModels[iNumModels].ignoreCollision = (attachIgnoreCollision & (1 << i)) != 0;
        if ( es->lerp.u.player.stowedWeapon
            && !I_strcmp(ci->attachModelNames[i], **(const char ***)weapDefStowed->worldModel) )
        {
            iStowedWeaponModelIndex = iNumModels;
        }
        ++iNumModels;
    }
    if (pAnimTree)
    {
        bgs->CreateDObj(
            dobjModels,
            iNumModels,
            pAnimTree,
            es->number,
            localClientNum,
            ci);
        ci->dobjDirty = 0;
        dobj = bgs->GetDObj(es->number, localClientNum);
        es->partBits[0] = 0;
        es->partBits[1] = 0;
        es->partBits[2] = 0;
        es->partBits[3] = 0;
        es->partBits[4] = 0;
        weapVariantDef = BG_GetWeaponVariantDef(es->weapon);
        weapDef = BG_GetWeaponDef(es->weapon);
        if (weapDef->inventoryType == WEAPINVENTORY_ALTMODE)
            weapVariantDef = BG_GetWeaponVariantDef(es->lerp.u.player.primaryWeapon);
        if (ci->hideWeapon)
        {
            for (m = firstWeaponModel; m < lastWeaponModel; ++m)
            {
                model = dobjModels[m].model;
                for (b = 0; b < model->numBones; ++b)
                {
                    index = -2;
                    if (DObjGetBoneIndex(dobj, model->localBoneNames[b], &index, -1))
                        es->partBits[(int)index >> 5] |= 0x80000000 >> (index & 0x1F);
                }
            }
            Com_Memcpy((unsigned __int8 *)ci->weaponHideBits, (unsigned __int8 *)es->partBits, 0x14u);
        }
        else
        {
            for (tagIndex = 0; tagIndex < 32 && weapVariantDef->hideTags[tagIndex]; ++tagIndex)
            {
                boneIndex = -2;
                if (DObjGetBoneIndex(dobj, weapVariantDef->hideTags[tagIndex], &boneIndex, -1))
                    es->partBits[(int)boneIndex >> 5] |= 0x80000000 >> (boneIndex & 0x1F);
            }
            if (weapDef->bDualWield && !ci->usingKnife)
            {
                weapVariantDefDW = BG_GetWeaponVariantDef(weapDef->dualWieldWeaponIndex);
                weapDefDW = BG_GetWeaponDef(weapDef->dualWieldWeaponIndex);
                for (tagIndex = 0; tagIndex < 32 && weapVariantDefDW->hideTags[tagIndex]; ++tagIndex)
                {
                    boneIndex = -2;
                    if (DObjGetModelBoneIndex(
                        dobj,
                        **(const char ***)weapDefDW->worldModel,
                        weapVariantDefDW->hideTags[tagIndex],
                        &boneIndex))
                    {
                        es->partBits[(int)boneIndex >> 5] |= 0x80000000 >> (boneIndex & 0x1F);
                    }
                }
            }
        }
        if (es->lerp.u.player.stowedWeapon)
        {
            weapVariantDefStowed = BG_GetWeaponVariantDef(es->lerp.u.player.stowedWeapon);
            for (tagIndex = 0; tagIndex < 32 && weapVariantDefStowed->hideTags[tagIndex]; ++tagIndex)
            {
                boneIndex = -2;
                if (DObjGetModelBoneIndex(dobj, iStowedWeaponModelIndex, weapVariantDefStowed->hideTags[tagIndex], &boneIndex))
                    es->partBits[(int)boneIndex >> 5] |= 0x80000000 >> (boneIndex & 0x1F);
            }
        }
        DObjSetHidePartBits(dobj, es->partBits);
        if (bg_enableIKActiveFix->current.enabled)
            DObjSetIKActiveFlag(dobj, ikActive);
    }
}

int __cdecl BG_GetPlayerWeaponForDObj(int localClientNum, entityState_s *es)
{
    int iClientWeapon; // [esp+0h] [ebp-4h]

    iClientWeapon = es->lerp.u.player.primaryWeapon;
    if ( (es->lerp.eFlags & 0x300) != 0 )
        iClientWeapon = 0;
    if ( (es->lerp.eFlags & 0x4000) != 0
        && (es->lerp.eFlags2 & 0x10000000) == 0
        && (es->lerp.u.player.vehicleSeat < 5u || es->lerp.u.player.vehicleSeat > 0xAu) )
    {
        iClientWeapon = 0;
    }
    if ( es->eType == 2 )
        return 0;
    return iClientWeapon;
}

int __cdecl BG_GetPlayerOffhandWeaponForDObj(int localClientNum, entityState_s *es)
{
    int iClientWeapon; // [esp+0h] [ebp-4h]

    iClientWeapon = es->lerp.u.player.offhandWeapon;
    if ( (es->lerp.eFlags & 0x300) != 0 )
        iClientWeapon = 0;
    if ( (es->lerp.eFlags & 0x4000) != 0
        && (es->lerp.eFlags2 & 0x10000000) == 0
        && (es->lerp.u.player.vehicleSeat < 5u || es->lerp.u.player.vehicleSeat > 0xAu) )
    {
        return 0;
    }
    return iClientWeapon;
}

void __cdecl BG_FindAnimTrees()
{
    BG_CheckThread();
    if (!bgs
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp", 5299, 0, "%s", "bgs"))
    {
        __debugbreak();
    }
    bgs->animData->generic_human.tree = BG_FindAnimTree("multiplayer", 1);
    bgs->animData->animScriptData.animTree.anims = bgs->animData->generic_human.tree.anims;
    bgs->animData->animScriptData.mainAnim = bgs->animData->generic_human.main.index;
    bgs->animData->animScriptData.torsoAnim = bgs->animData->generic_human.torso.index;
    bgs->animData->animScriptData.legsAnim = bgs->animData->generic_human.legs.index;
}

scr_animtree_t __cdecl BG_FindAnimTree(const char *filename, int bEnforceExists)
{
    scr_animtree_t tree; // [esp+4h] [ebp-4h]

    tree.anims = Scr_FindAnimTree(SCRIPTINSTANCE_SERVER, filename).anims;
    if ( !tree.anims && bEnforceExists )
        Com_Error(ERR_DROP, "Could not find animation tree '%s'", filename);
    return tree;
}

unsigned int iNumPlayerAnims;
void __cdecl BG_LoadAnim(const char *levelName)
{
    LargeLocal playerAnims_large_local(sizeof(loadAnim_t) * 1024 /*73728*/); // [esp+0h] [ebp-Ch] BYREF
    //loadAnim_t (*playerAnims)[1024]; // [esp+8h] [ebp-4h]

    //LargeLocal::LargeLocal(&playerAnims_large_local, 73728);
    //playerAnims = (loadAnim_t (*)[1024])LargeLocal::GetBuf(&playerAnims_large_local);
    loadAnim_t *playerAnims = (loadAnim_t *)playerAnims_large_local.GetBuf();
    BG_CheckThread();
    iassert(bgs);

    iNumPlayerAnims = 0;

    BG_FindAnims();
    BG_AnimParseAnimScript(
        &bgs->animData->animScriptData,
        playerAnims,
        &iNumPlayerAnims,
        levelName);
    Scr_PrecacheAnimTrees(
        SCRIPTINSTANCE_SERVER,
        (void*(*)(int))bgs->AllocXAnim,
        bgs->anim_user,
        bgs == &level_bgs);
    BG_FindAnimTrees();

    //LargeLocal::~LargeLocal(&playerAnims_large_local);
}

void __cdecl BG_AnimParseAnimScript(
                animScriptData_t *scriptData,
                loadAnim_t *pLoadAnims,
                unsigned int *piNumAnims,
                const char *levelName)
{
    int v4; // eax
    const char *v5; // eax
    int v6; // eax
    const char *v7; // eax
    int v8; // eax
    int v9; // eax
    int v10; // eax
    int v11; // eax
    int v12; // eax
    int v13; // eax
    int v14; // eax
    bool inBrace; // [esp+9Eh] [ebp-15Ah]
    bool conditionSeatsPresent; // [esp+9Fh] [ebp-159h]
    signed int seat; // [esp+A0h] [ebp-158h]
    signed int seata; // [esp+A0h] [ebp-158h]
    char vehName[24]; // [esp+A4h] [ebp-154h] BYREF
    int gunners; // [esp+BCh] [ebp-13Ch] BYREF
    int passengers; // [esp+C0h] [ebp-138h] BYREF
    int drivers; // [esp+C4h] [ebp-134h] BYREF
    int vehType; // [esp+C8h] [ebp-130h]
    int *pInt; // [esp+CCh] [ebp-12Ch]
    const char *levelStr; // [esp+D0h] [ebp-128h]
    bool matched; // [esp+D7h] [ebp-121h]
    const char *vehTypeStr; // [esp+D8h] [ebp-120h]
    int newParseMode; // [esp+DCh] [ebp-11Ch]
    animScriptItem_t *currentScriptItem; // [esp+E0h] [ebp-118h]
    int oldState; // [esp+E4h] [ebp-114h]
    const char *input; // [esp+E8h] [ebp-110h]
    const char *text_p; // [esp+ECh] [ebp-10Ch] BYREF
    animScript_t *currentScript; // [esp+F0h] [ebp-108h]
    int inCommentLevel; // [esp+F4h] [ebp-104h]
    int indentLevel; // [esp+F8h] [ebp-100h]
    com_parse_mark_t forAllVehiclesMark; // [esp+FCh] [ebp-FCh] BYREF
    animScriptItem_t tempScriptItem; // [esp+110h] [ebp-E8h] BYREF
    const char *token; // [esp+1DCh] [ebp-1Ch]
    animScriptParseMode_t parseMode; // [esp+1E0h] [ebp-18h]
    int indexes[3]; // [esp+1E4h] [ebp-14h]
    int i; // [esp+1F0h] [ebp-8h]
    int defineType; // [esp+1F4h] [ebp-4h]

    currentScriptItem = 0;
    input = Com_LoadRawTextFile(globalFilename);
    if ( !input )
        Com_Error(ERR_DROP, "Couldn't load player animation script %s", globalFilename);
    g_pLoadAnims = pLoadAnims;
    g_piNumLoadAnims = piNumAnims;
    parseMode = PARSEMODE_DEFINES;
    BG_InitWeaponStrings();
    BG_InitVehicles();
    memset(defineStr, 0, sizeof(defineStr));
    memset(defineStrings, 0, sizeof(defineStrings));
    memset(numDefines, 0, sizeof(numDefines));
    defineStringsOffset = 0;
    parseVehicleLoop = 0;
    parseVehicleNameIndex = -1;
    inCommentLevel = 0;
    for ( i = 0; i < 3; ++i )
        indexes[i] = -1;
    indentLevel = 0;
    currentScript = 0;
    text_p = input;
    Com_BeginParseSession("BG_AnimParseAnimScript");
    for ( i = 0; i < 34; ++i )
        memset(&scriptData->scriptEvents[i], 0, sizeof(scriptData->scriptEvents[i]));
    while ( 1 )
    {
        token = (const char *)Com_Parse(&text_p);
        if ( !token || !*token )
            break;
        if ( inCommentLevel )
        {
            if ( !I_stricmp(token, "*/") && --inCommentLevel < 0 )
                inCommentLevel = 0;
        }
        else if ( I_stricmp(token, "/*") )
        {
            v4 = BG_IndexForString(token, animParseModesStr, 1);
            newParseMode = v4;
            if ( v4 < 0 )
            {
                switch ( parseMode )
                {
                    case PARSEMODE_DEFINES:
                        if ( I_stricmp(token, "set") )
                        {
                            if ( I_stricmp(token, "#IF") )
                            {
                                if ( I_stricmp(token, "#END_IF") && !I_stricmp(token, "#ADD_VEHICLE") )
                                {
                                    token = (const char *)Com_ParseOnLine(&text_p);
                                    if ( !token )
                                        BG_AnimParseError("BG_AnimParseAnimScript: expected #ADD_VEHICLE <vehicle_name> <default/tank>");
                                    if ( strlen(token) >= 0x18
                                        && !Assert_MyHandler(
                                                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                                                    2200,
                                                    0,
                                                    "%s",
                                                    "strlen( token ) < sizeof(vehName)") )
                                    {
                                        __debugbreak();
                                    }
                                    I_strncpyz(vehName, token, 24);
                                    token = (const char *)Com_ParseOnLine(&text_p);
                                    if ( !token )
                                        BG_AnimParseError("BG_AnimParseAnimScript: expected #ADD_VEHICLE <vehicle_name> <default/tank>");
                                    vehType = BG_IndexForString(token, animVehicleAnimTypeStr, 0);
                                    drivers = 0;
                                    gunners = 0;
                                    passengers = 0;
                                    for ( token = (const char *)Com_ParseOnLine(&text_p);
                                                token && *token;
                                                token = (const char *)Com_ParseOnLine(&text_p) )
                                    {
                                        pInt = 0;
                                        if ( I_stricmp(token, "drivers") )
                                        {
                                            if ( I_stricmp(token, "gunners") )
                                            {
                                                if ( !I_stricmp(token, "passengers") )
                                                    pInt = &passengers;
                                            }
                                            else
                                            {
                                                pInt = &gunners;
                                            }
                                        }
                                        else
                                        {
                                            pInt = &drivers;
                                        }
                                        if ( pInt )
                                        {
                                            token = (const char *)Com_ParseOnLine(&text_p);
                                            if ( token && *token )
                                            {
                                                v8 = atoi(token);
                                                *pInt = v8;
                                            }
                                            else
                                            {
                                                BG_AnimParseError("BG_AnimParseAnimScript: missing index for seat type");
                                            }
                                        }
                                        else
                                        {
                                            BG_AnimParseError("BG_AnimParseAnimScript: unknown token (%s) in #ADD_VEHICLE command", token);
                                        }
                                    }
                                    BG_AddVehicleName(vehName, vehType, drivers, gunners, passengers);
                                }
                            }
                            else
                            {
                                token = (const char *)Com_ParseOnLine(&text_p);
                                if ( !token || !*token )
                                    BG_AnimParseError("BG_AnimParseAnimScript: expected #IF <conditional> <value>");
                                if ( I_stricmp(token, "vehicle_anim_type") )
                                {
                                    if ( I_stricmp(token, "level_name") )
                                    {
                                        BG_AnimParseError("BG_AnimParseAnimScript: unknown pre-processor conditional (%s)", token);
                                    }
                                    else
                                    {
                                        matched = 0;
                                        token = (const char *)Com_ParseOnLine(&text_p);
                                        if ( !token || !*token )
                                            BG_AnimParseError("BG_AnimParseAnimScript: expected #IF <conditional> <value>");
                                        while ( token && *token )
                                        {
                                            if ( !matched )
                                            {
                                                levelStr = levelName;
                                                v7 = I_stristr(levelName, "mp/");
                                                if ( v7 == levelStr )
                                                    levelStr = levelName + 3;
                                                if ( !I_stricmp(token, levelStr) )
                                                    matched = 1;
                                            }
                                            token = (const char *)Com_ParseOnLine(&text_p);
                                        }
                                        if ( !matched )
                                        {
                                            token = (const char *)Com_Parse(&text_p);
                                            while ( I_stricmp(token, "#END_IF") )
                                            {
                                                token = (const char *)Com_Parse(&text_p);
                                                if ( !token || !*token )
                                                    BG_AnimParseError("BG_AnimParseAnimScript: expected #END_IF, reached end of file");
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    token = (const char *)Com_ParseOnLine(&text_p);
                                    if ( !token || !*token )
                                        BG_AnimParseError("BG_AnimParseAnimScript: expected #IF <conditional> <value>");
                                    vehTypeStr = BG_StringForIndex(animVehicleTypes[parseVehicleNameIndex], animVehicleAnimTypeStr, 1);
                                    if ( !vehTypeStr )
                                        BG_AnimParseError(
                                            "BG_AnimParseAnimScript: unknown vehicle type for %s",
                                            animVehicleNameStr[parseVehicleNameIndex].string);
                                    if ( I_stricmp(vehTypeStr, token) )
                                    {
                                        while ( I_stricmp(token, "#END_IF") )
                                        {
                                            token = (const char *)Com_Parse(&text_p);
                                            if ( !token || !*token )
                                                BG_AnimParseError("BG_AnimParseAnimScript: expected #END_IF, reached end of file");
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            token = (const char *)Com_ParseOnLine(&text_p);
                            if ( !token || !*token )
                                BG_AnimParseError("BG_AnimParseAnimScript: expected condition type string");
                            defineType = BG_IndexForString(token, animConditionsStr, 0);
                            if ( animConditionsTable[defineType].type )
                                BG_AnimParseError("BG_AnimParseAnimScript: can not make a define of type '%s'", token);
                            token = (const char *)Com_ParseOnLine(&text_p);
                            if ( !token || !*token )
                                BG_AnimParseError("BG_AnimParseAnimScript: expected condition define string");
                            v5 = BG_CopyStringIntoBuffer(token, defineStrings, 0x2710u, &defineStringsOffset);
                            defineStr[defineType][numDefines[defineType]].string = v5;
                            v6 = BG_StringHashValue(defineStr[defineType][numDefines[defineType]].string);
                            defineStr[defineType][numDefines[defineType]].hash = v6;
                            token = (const char *)Com_ParseOnLine(&text_p);
                            if ( !token )
                                BG_AnimParseError("BG_AnimParseAnimScript: expected '=', found end of line");
                            if ( I_stricmp(token, "=") )
                                BG_AnimParseError("BG_AnimParseAnimScript: expected '=', found '%s'", token);
                            BG_ParseConditionBits(
                                &text_p,
                                animConditionsTable[defineType].values,
                                defineType,
                                &defineBits[defineType][numDefines[defineType]]);
                            ++numDefines[defineType];
                        }
                        break;
                    case PARSEMODE_ANIMATION:
                    case PARSEMODE_CANNED_ANIMATIONS:
                        if ( I_stricmp(token, "{") )
                        {
                            if ( I_stricmp(token, "}") )
                            {
                                if ( indentLevel || indexes[0] >= 0 )
                                {
                                    if ( indentLevel == 1 && indexes[1] < 0 )
                                    {
                                        v10 = BG_IndexForString(token, animMoveTypesStr, 0);
                                        indexes[indentLevel] = v10;
                                        if ( parseMode == PARSEMODE_ANIMATION )
                                        {
                                            currentScript = &scriptData->scriptAnims[indexes[0]][indexes[1]];
                                            parseMovetype = (scriptAnimMoveTypes_t)indexes[1];
                                        }
                                        else if ( parseMode == PARSEMODE_CANNED_ANIMATIONS )
                                        {
                                            currentScript = &scriptData->scriptCannedAnims[indexes[0]][indexes[1]];
                                        }
                                        memset(currentScript, 0, sizeof(animScript_t));
                                    }
                                    else
                                    {
                                        if ( indentLevel != 2 || indexes[2] >= 0 )
                                        {
                                            if ( indentLevel != 3 )
                                                goto LABEL_181;
                                            text_p -= strlen(token);
                                            if ( I_strncmp(text_p, token, strlen(token)) )
                                                BG_AnimParseError("BG_AnimParseAnimScript: internal error");
                                            goto LABEL_180;
                                        }
                                        text_p -= strlen(token);
                                        if ( I_strncmp(text_p, token, strlen(token)) )
                                            BG_AnimParseError("BG_AnimParseAnimScript: internal error");
                                        if ( I_stricmp(token, "#FOR_ALL_VEHICLES") )
                                        {
                                            if ( I_stricmp(token, "#END_FOR_ALL_VEHICLES") )
                                            {
                                                memset(&tempScriptItem, 0, sizeof(tempScriptItem));
                                                v11 = BG_ParseConditions(&text_p, &tempScriptItem);
                                                indexes[indentLevel] = v11;
                                                if ( currentScript->numItems >= 128 )
                                                    BG_AnimParseError("BG_AnimParseAnimScript: exceeded maximum items per script (%i)", 128);
                                                if ( scriptData->numScriptItems >= 2048 )
                                                    BG_AnimParseError("BG_AnimParseAnimScript: exceeded maximum global items (%i)", 2048);
                                                goto LABEL_176;
                                            }
                                            if ( BG_StringForIndex(++parseVehicleNameIndex, animVehicleNameStr, 1) )
                                                goto LABEL_171;
                                            token = (const char *)Com_Parse(&text_p);
                                            if ( !token || !*token )
                                                BG_AnimParseError("BG_AnimParseAnimScript: internal error");
                                            parseVehicleLoop = 0;
                                            parseVehicleNameIndex = 0;
                                        }
                                        else
                                        {
                                            token = (const char *)Com_Parse(&text_p);
                                            if ( !token || !*token )
                                                BG_AnimParseError("BG_AnimParseAnimScript: internal error");
                                            Com_ParseSetMark(&text_p, &forAllVehiclesMark);
                                            parseVehicleLoop = 1;
                                            parseVehicleNameIndex = 0;
                                            if ( !BG_StringForIndex(0, animVehicleNameStr, 1) )
                                            {
                                                parseVehicleLoop = 0;
                                                parseVehicleNameIndex = 0;
                                                while ( I_stricmp(token, "#END_FOR_ALL_VEHICLES") )
                                                {
                                                    token = (const char *)Com_Parse(&text_p);
                                                    if ( !token || !*token )
                                                        BG_AnimParseError("BG_AnimParseAnimScript: expected #END_FOR_ALL_VEHICLES, reached end of file");
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if ( I_stricmp(token, "state") )
                                        BG_AnimParseError("BG_AnimParseAnimScript: expected 'state'");
                                    token = (const char *)Com_ParseOnLine(&text_p);
                                    if ( !token )
                                        BG_AnimParseError("BG_AnimParseAnimScript: expected state type");
                                    v9 = BG_IndexForString(token, animStateStr, 0);
                                    indexes[indentLevel] = v9;
                                    token = (const char *)Com_Parse(&text_p);
                                    if ( !token || I_stricmp(token, "{") )
                                        BG_AnimParseError("BG_AnimParseAnimScript: expected '{'");
                                    ++indentLevel;
                                }
                            }
                            else
                            {
                                if ( --indentLevel < 0 )
                                    BG_AnimParseError("BG_AnimParseAnimScript: unexpected '%s'", token);
                                if ( indentLevel == 1 )
                                    currentScript = 0;
                                indexes[indentLevel] = -1;
                            }
                        }
                        else
                        {
                            if ( indentLevel >= 3 )
                                BG_AnimParseError("BG_AnimParseAnimScript: unexpected '%s'", token);
                            if ( indexes[indentLevel] < 0 )
                                BG_AnimParseError("BG_AnimParseAnimScript: unexpected '%s'", token);
                            ++indentLevel;
                        }
                        break;
                    case PARSEMODE_STATECHANGES:
                    case PARSEMODE_EVENTS:
                        if ( I_stricmp(token, "{") )
                        {
                            if ( I_stricmp(token, "}") )
                            {
                                if ( indentLevel || indexes[0] >= 0 )
                                {
                                    if ( indentLevel == 1 && indexes[1] < 0 )
                                    {
                                        text_p -= strlen(token);
                                        if ( I_strncmp(text_p, token, strlen(token)) )
                                            BG_AnimParseError("BG_AnimParseAnimScript: internal error");
                                        memset(&tempScriptItem, 0, sizeof(tempScriptItem));
                                        v14 = BG_ParseConditions(&text_p, &tempScriptItem);
                                        indexes[indentLevel] = v14;
                                        if ( currentScript->numItems >= 128 )
                                            BG_AnimParseError("BG_AnimParseAnimScript: exceeded maximum items per script (%i)", 128);
                                        if ( scriptData->numScriptItems >= 2048 )
                                            BG_AnimParseError("BG_AnimParseAnimScript: exceeded maximum global items (%i)", 2048);
                                        if ( !parseVehicleLoop )
                                            goto LABEL_176;
                                        conditionSeatsPresent = 1;
                                        seat = BG_AnimGetConditionValue(&tempScriptItem, 18);
                                        if ( seat <= 0 || (seat & animVehicleSeats[parseVehicleNameIndex].seats) != 0 )
                                        {
                                            seata = BG_AnimGetConditionValue(&tempScriptItem, 17);
                                            if ( seata > 0 && (seata & animVehicleSeats[parseVehicleNameIndex].seats) == 0 )
                                                conditionSeatsPresent = 0;
                                        }
                                        else
                                        {
                                            conditionSeatsPresent = 0;
                                        }
                                        if ( conditionSeatsPresent )
                                        {
LABEL_176:
                                            currentScript->items[currentScript->numItems] = &scriptData->scriptItems[scriptData->numScriptItems++];
                                            currentScriptItem = currentScript->items[currentScript->numItems++];
                                            memcpy(currentScriptItem, &tempScriptItem, sizeof(animScriptItem_t));
                                        }
                                        else
                                        {
                                            inBrace = 0;
                                            for ( token = (const char *)Com_Parse(&text_p);
                                                        token && *token;
                                                        token = (const char *)Com_Parse(&text_p) )
                                            {
                                                if ( inBrace )
                                                {
                                                    if ( !I_stricmp(token, "}") )
                                                        break;
                                                }
                                                else
                                                {
                                                    inBrace = I_stricmp(token, "{") == 0;
                                                }
                                            }
                                            indexes[indentLevel] = -1;
                                        }
                                    }
                                    else if ( indentLevel == 2 )
                                    {
                                        text_p -= strlen(token);
                                        if ( I_strncmp(text_p, token, strlen(token)) )
                                            BG_AnimParseError("BG_AnimParseAnimScript: internal error");
LABEL_180:
                                        BG_ParseCommands(&text_p, currentScriptItem, scriptData);
                                    }
                                    else
                                    {
LABEL_181:
                                        BG_AnimParseError("BG_AnimParseAnimScript: unexpected '%s'", token);
                                    }
                                }
                                else if ( parseMode == PARSEMODE_STATECHANGES )
                                {
                                    if ( I_stricmp(token, "statechange") )
                                        BG_AnimParseError("BG_AnimParseAnimScript: expected 'statechange', got '%s'", token);
                                    token = (const char *)Com_ParseOnLine(&text_p);
                                    if ( !token )
                                        BG_AnimParseError("BG_AnimParseAnimScript: expected <state type>");
                                    oldState = BG_IndexForString(token, animStateStr, 0);
                                    token = (const char *)Com_ParseOnLine(&text_p);
                                    if ( !token )
                                        BG_AnimParseError("BG_AnimParseAnimScript: expected <state type>");
                                    v12 = BG_IndexForString(token, animStateStr, 0);
                                    indexes[indentLevel] = v12;
                                    currentScript = &scriptData->scriptStateChange[oldState][indexes[indentLevel]];
                                    token = (const char *)Com_Parse(&text_p);
                                    if ( !token || I_stricmp(token, "{") )
                                        BG_AnimParseError("BG_AnimParseAnimScript: expected '{'");
                                    ++indentLevel;
                                    memset(currentScript, 0, sizeof(animScript_t));
                                }
                                else if ( I_stricmp(token, "#FOR_ALL_VEHICLES") )
                                {
                                    if ( I_stricmp(token, "#END_FOR_ALL_VEHICLES") )
                                    {
                                        v13 = BG_IndexForString(token, animEventTypesStr, 0);
                                        indexes[indentLevel] = v13;
                                        currentScript = &scriptData->scriptEvents[indexes[indentLevel]];
                                        parseEvent = indexes[indentLevel];
                                    }
                                    else if ( BG_StringForIndex(++parseVehicleNameIndex, animVehicleNameStr, 1) )
                                    {
LABEL_171:
                                        Com_ParseReturnToMark(&text_p, &forAllVehiclesMark);
                                    }
                                    else
                                    {
                                        parseVehicleLoop = 0;
                                        parseVehicleNameIndex = 0;
                                    }
                                }
                                else
                                {
                                    Com_ParseSetMark(&text_p, &forAllVehiclesMark);
                                    parseVehicleLoop = 1;
                                    parseVehicleNameIndex = 0;
                                    if ( !BG_StringForIndex(0, animVehicleNameStr, 1) )
                                    {
                                        parseVehicleLoop = 0;
                                        parseVehicleNameIndex = 0;
                                        while ( I_stricmp(token, "#END_FOR_ALL_VEHICLES") )
                                        {
                                            token = (const char *)Com_Parse(&text_p);
                                            if ( !token || !*token )
                                                BG_AnimParseError("BG_AnimParseAnimScript: expected #END_FOR_ALL_VEHICLES, reached end of file");
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if ( --indentLevel < 0 )
                                    BG_AnimParseError("BG_AnimParseAnimScript: unexpected '%s'", token);
                                if ( !indentLevel )
                                    currentScript = 0;
                                indexes[indentLevel] = -1;
                            }
                        }
                        else
                        {
                            if ( indentLevel >= 3 )
                                BG_AnimParseError("BG_AnimParseAnimScript: unexpected '%s'", token);
                            if ( indexes[indentLevel] < 0 )
                                BG_AnimParseError("BG_AnimParseAnimScript: unexpected '%s'", token);
                            ++indentLevel;
                        }
                        break;
                    case PARSEMODE_FORCELOAD:
                        if ( I_stricmp(token, "{") )
                        {
                            if ( I_stricmp(token, "}") )
                            {
                                if ( token && *token )
                                    BG_AnimationIndexForString(token);
                                else
                                    BG_AnimParseError("BG_AnimParseAnimScript: expected animation");
                            }
                            else
                            {
                                if ( --indentLevel < 0 )
                                    BG_AnimParseError("BG_AnimParseAnimScript: unexpected '%s'", token);
                                if ( indentLevel == 1 )
                                    currentScript = 0;
                            }
                        }
                        else
                        {
                            if ( indentLevel >= 3 )
                                BG_AnimParseError("BG_AnimParseAnimScript: unexpected '%s'", token);
                            ++indentLevel;
                        }
                        break;
                    default:
                        continue;
                }
            }
            else
            {
                if ( indentLevel )
                    BG_AnimParseError("BG_AnimParseAnimScript: unexpected '%s'", token);
                parseMode = (animScriptParseMode_t)newParseMode;
                parseMovetype = ANIM_MT_UNUSED;
                parseEvent = -1;
            }
        }
        else
        {
            ++inCommentLevel;
        }
    }
    if ( indentLevel )
        BG_AnimParseError("BG_AnimParseAnimScript: unexpected end of file: %s", token);
    Com_EndParseSession();
    Com_UnloadRawTextFile((char *)input);
}

char *__cdecl BG_CopyStringIntoBuffer(const char *string, char *buffer, unsigned int bufSize, unsigned int *offset)
{
    char v5; // [esp+13h] [ebp-21h]
    char *v6; // [esp+18h] [ebp-1Ch]
    const char *v7; // [esp+1Ch] [ebp-18h]
    char *pch; // [esp+30h] [ebp-4h]

    if ( *offset + strlen(string) + 1 >= bufSize )
        BG_AnimParseError("BG_CopyStringIntoBuffer: out of buffer space");
    pch = &buffer[*offset];
    v7 = string;
    v6 = pch;
    do
    {
        v5 = *v7;
        *v6++ = *v7++;
    }
    while ( v5 );
    *offset += strlen(string) + 1;
    return pch;
}

void __cdecl BG_ParseConditionBits(
                const char **text_pp,
                animStringItem_t *stringTable,
                int condIndex,
                unsigned int *result)
{
    unsigned int tempBits; // [esp+50h] [ebp-5Ch] BYREF
    char currentString[68]; // [esp+54h] [ebp-58h] BYREF
    int minus; // [esp+9Ch] [ebp-10h]
    char *token; // [esp+A0h] [ebp-Ch]
    int endFlag; // [esp+A4h] [ebp-8h]
    int indexFound; // [esp+A8h] [ebp-4h]

    endFlag = 0;
    minus = 0;
    currentString[0] = 0;
    *result = 0;
    *result = 0;
    tempBits = 0;
    while ( !endFlag )
    {
        token = Com_ParseOnLine(text_pp)->token;
        if ( !token || !*token )
        {
            Com_UngetToken();
            endFlag = 1;
            if ( &currentString[strlen(currentString) + 1] == &currentString[1] )
                break;
        }
        if ( !I_stricmp(token, ",") )
            endFlag = 1;
        if ( I_stricmp(token, "none") )
        {
            if ( I_stricmp(token, "none,") )
            {
                if ( !I_stricmp(token, "NOT") )
                    token = (char*)"MINUS";
                if ( !endFlag && I_stricmp(token, "AND") && I_stricmp(token, "MINUS") )
                {
                    if ( token[strlen(token) - 1] == 44 )
                    {
                        endFlag = 1;
                        token[strlen(token) - 1] = 0;
                    }
                    if ( &currentString[strlen(currentString) + 1] != &currentString[1] )
                        I_strncat(currentString, 64, " ");
                    I_strncat(currentString, 64, token);
                }
                if ( !I_stricmp(token, "AND") || !I_stricmp(token, "MINUS") || endFlag )
                {
                    if ( &currentString[strlen(currentString) + 1] != &currentString[1] )
                        goto LABEL_31;
                    if ( endFlag )
                    {
                        BG_AnimParseError("BG_ParseConditionBits: unexpected end of condition");
                        goto LABEL_31;
                    }
                    if ( I_stricmp(token, "MINUS") )
                    {
                        BG_AnimParseError("BG_ParseConditionBits: unexpected '%s'", token);
LABEL_31:
                        if ( I_stricmp(currentString, "all") )
                        {
                            indexFound = BG_IndexForString(currentString, defineStr[condIndex], 1);
                            if ( indexFound < 0 )
                            {
                                tempBits = 0;
                                indexFound = BG_IndexForString(currentString, stringTable, 0);
                                Com_BitSetAssert(&tempBits, indexFound, 0xFFFFFFF);
                            }
                            else
                            {
                                tempBits = defineBits[condIndex][indexFound];
                            }
                        }
                        else
                        {
                            tempBits = -1;
                        }
                        if ( minus )
                        {
                            if ( !*result )
                                *result = -1;
                            *result &= ~tempBits;
                        }
                        else
                        {
                            *result |= tempBits;
                        }
                        currentString[0] = 0;
                        if ( !I_stricmp(token, "MINUS") )
                            minus = 1;
                    }
                    else
                    {
                        minus = 1;
                    }
                }
            }
            else
            {
                Com_BitSetAssert(result, 0, 0xFFFFFFF);
                endFlag = 1;
            }
        }
        else
        {
            Com_BitSetAssert(result, 0, 0xFFFFFFF);
        }
    }
    BG_SetAnimConditionFlags(condIndex, *result);
}

void __cdecl BG_SetAnimConditionFlags(int condIndex, unsigned int result)
{
    if ( condIndex == 5 )
    {
        if ( Com_BitCheckAssert(&result, 2, 0xFFFFFFF) )
            parseMovestatus = ANIM_MOVESTATUS_RUN;
        else
            parseMovestatus = (scriptAnimMoveStatusStates_t )Com_BitCheckAssert(&result, 1, 0xFFFFFFF);
    }
    else if ( condIndex == 9 )
    {
        if ( Com_BitCheckAssert(&result, 2, 0xFFFFFFF) )
            parseStance = ANIM_STANCE_PRONE;
        else
            parseStance = (scriptAnimStances_t)Com_BitCheckAssert(&result, 1, 0xFFFFFFF);
    }
}

int __cdecl BG_ParseConditions(const char **text_pp, animScriptItem_t *scriptItem)
{
    int conditionIndex; // [esp+44h] [ebp-14h]
    unsigned int conditionValue; // [esp+50h] [ebp-8h] BYREF
    char *token; // [esp+54h] [ebp-4h]

    conditionValue = 0;
    parseStance = ANIM_STANCE_STAND;
    parseMovestatus = ANIM_MOVESTATUS_STATIONARY;
    while ( 1 )
    {
        token = Com_ParseOnLine(text_pp)->token;
        if ( !token || !*token )
            break;
        if ( !I_stricmp(token, "default") )
            return 1;
        conditionIndex = BG_IndexForString(token, animConditionsStr, 0);
        switch ( animConditionsTable[conditionIndex].type )
        {
            case ANIM_CONDTYPE_BITFLAGS:
                BG_ParseConditionBits(text_pp, animConditionsTable[conditionIndex].values, conditionIndex, &conditionValue);
                break;
            case ANIM_CONDTYPE_VALUE:
                token = Com_ParseOnLine(text_pp)->token;
                if ( !token || !*token )
                    BG_AnimParseError("BG_ParseConditions: expected condition value, found end of line");
                if ( token[strlen(token) - 1] == 44 )
                    token[strlen(token) - 1] = 0;
                if ( animConditionsTable[conditionIndex].values )
                {
                    if ( I_stricmp(token, "%vehicle_name") )
                    {
                        conditionValue = BG_IndexForString(token, animConditionsTable[conditionIndex].values, 0);
                    }
                    else
                    {
                        if ( parseVehicleNameIndex < 0 )
                            BG_AnimParseError("BG_ParseConditions: %vehicle_name found outside of #FOR_ALL_VEHICLES loop");
                        conditionValue = parseVehicleNameIndex;
                    }
                }
                else
                {
                    conditionValue = atoi(token);
                }
                break;
            case ANIM_CONDTYPE_ENABLED:
                conditionValue = 1;
                break;
            case ANIM_CONDTYPE_STRINGHASH:
                token = Com_ParseOnLine(text_pp)->token;
                if ( !token || !*token )
                    BG_AnimParseError("BG_ParseConditions: expected condition value, found end of line");
                if ( token[strlen(token) - 1] == 44 )
                    token[strlen(token) - 1] = 0;
                conditionValue = BG_StringHashValue(token);
                break;
            default:
                break;
        }
        scriptItem->conditions[scriptItem->numConditions].index = conditionIndex;
        scriptItem->conditions[scriptItem->numConditions++].value = conditionValue;
    }
    if ( !scriptItem->numConditions )
        BG_AnimParseError("BG_ParseConditions: no conditions found");
    return 1;
}

void BG_FindAnims()
{
    BG_CheckThread();
    iassert(bgs);
    Scr_FindAnim(
        SCRIPTINSTANCE_SERVER,
        "multiplayer",
        "main",
        &bgs->animData->generic_human.main,
        bgs->anim_user);
    Scr_FindAnim(
        SCRIPTINSTANCE_SERVER,
        "multiplayer",
        "torso",
        &bgs->animData->generic_human.torso,
        bgs->anim_user);
    Scr_FindAnim(
        SCRIPTINSTANCE_SERVER,
        "multiplayer",
        "legs",
        &bgs->animData->generic_human.legs,
        bgs->anim_user);
}

void __cdecl BG_PostLoadAnim(const char *levelName)
{
    Scr_EndLoadAnimTrees(SCRIPTINSTANCE_SERVER);
    BG_FinalizePlayerAnims(levelName);
}

void __cdecl BG_FinalizePlayerAnims(const char *levelName)
{
    const char *AnimName; // eax
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    double v6; // st7
    double v7; // st7
    double moveSpeed; // [esp+Ch] [ebp-70h]
    double v9; // [esp+14h] [ebp-68h]
    float duration; // [esp+3Ch] [ebp-40h]
    float fullspeed; // [esp+40h] [ebp-3Ch]
    float fullspeeda; // [esp+40h] [ebp-3Ch]
    loadAnim_t *pLoadAnim; // [esp+44h] [ebp-38h]
    animation_s *pCurrAnima; // [esp+48h] [ebp-34h]
    animation_s *pCurrAnim; // [esp+48h] [ebp-34h]
    const char *moveType; // [esp+4Ch] [ebp-30h]
    float vRot[2]; // [esp+50h] [ebp-2Ch] BYREF
    int devNumNamedAnims; // [esp+58h] [ebp-24h]
    int i; // [esp+5Ch] [ebp-20h]
    XAnim_s *pXAnims; // [esp+60h] [ebp-1Ch]
    float rotate; // [esp+64h] [ebp-18h] BYREF
    int iNumAnims; // [esp+68h] [ebp-14h]
    animation_s *pAnims; // [esp+6Ch] [ebp-10h]
    float vDelta[3]; // [esp+70h] [ebp-Ch] BYREF

    if ( !g_pLoadAnims
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp", 1017, 0, "%s", "g_pLoadAnims") )
    {
        __debugbreak();
    }
    if ( !g_piNumLoadAnims
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                    1018,
                    0,
                    "%s",
                    "g_piNumLoadAnims") )
    {
        __debugbreak();
    }
    pAnims = (animation_s *)bgs->animData;
    pXAnims = bgs->animData->animScriptData.animTree.anims;
    iNumAnims = XAnimGetAnimTreeSize(pXAnims);
    if ( iNumAnims >= 1024
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_animation.cpp",
                    1024,
                    0,
                    "%s",
                    "iNumAnims < MAX_MODEL_ANIMATIONS") )
    {
        __debugbreak();
    }
    bgs->animData->animScriptData.numAnimations = iNumAnims;
    pCurrAnima = pAnims;
    pAnims->flags |= 0x2001u;
    I_strncpyz(pCurrAnima->name, "root", 64);
    pCurrAnima->nameHash = 0;
    pCurrAnim = pCurrAnima + 1;
    devNumNamedAnims = 0;
    if ( animScript_listAnims->current.enabled )
    {
        Com_Printf(19, "\n===========================================================\n");
        Com_Printf(19, "Listing playeranim.script animations...\n\n");
    }
    i = 1;
    while ( i < iNumAnims )
    {
        pLoadAnim = BG_LoadAnimForAnimIndex(i);
        if ( pLoadAnim )
        {
            if ( XAnimIsPrimitive(pXAnims, i) )
            {
                AnimName = XAnimGetAnimName(pXAnims, i);
                I_strncpyz(pCurrAnim->name, AnimName, 64);
                pCurrAnim->nameHash = BG_StringHashValue(pCurrAnim->name);
                if ( !pCurrAnim->initialLerp )
                    pCurrAnim->initialLerp = -1;
                pCurrAnim->finalLerp = -1;
                pCurrAnim->forceAnimRate = -1.0f;
                duration = XAnimGetLength(pXAnims, i);
                if ( duration == 0.0 )
                {
                    pCurrAnim->duration = 500;
                    pCurrAnim->moveSpeed = 0.0f;
                }
                else
                {
                    pCurrAnim->duration = (int)(float)(duration * 1000.0);
                    XAnimGetRelDelta(pXAnims, i, vRot, vDelta, 0.0, 1.0);
                    if ( (float)(vDelta[0] + vDelta[1]) <= (float)(vDelta[2] * 0.80000001) )
                        pCurrAnim->moveSpeed = Vec3Length(vDelta) / duration;
                    else
                        pCurrAnim->moveSpeed = Vec2Length(vDelta) / duration;
                    if ( XAnimGetParamValue(pXAnims, i, "turn", &rotate) )
                        pCurrAnim->rotSpeed = rotate / duration;
                    else
                        pCurrAnim->rotSpeed = 0.0f;
                    if ( anim_debugSpeeds->current.enabled )
                    {
                        if ( pCurrAnim->moveSpeed <= 1.0 )
                        {
                            v7 = Vec3Length(vDelta);
                            Com_Printf(19, "Anim '%s' moves %f units over %fms\n", pCurrAnim->name, v7, duration);
                        }
                        else
                        {
                            if ( g_speed )
                                fullspeed = (float)g_speed->current.integer;
                            else
                                fullspeed = 190.0f;
                            v2 = strstr(pCurrAnim->name, "crouch");
                            if ( v2 )
                            {
                                fullspeeda = fullspeed * 0.64999998;
                                moveType = "crouch";
                            }
                            else
                            {
                                v3 = strstr(pCurrAnim->name, "prone");
                                if ( v3 )
                                {
                                    fullspeeda = fullspeed * 0.15000001;
                                    moveType = "prone";
                                }
                                else
                                {
                                    v4 = strstr(pCurrAnim->name, "walk");
                                    if ( v4 )
                                    {
                                        fullspeeda = fullspeed * 0.40000001;
                                        moveType = "walk";
                                    }
                                    else
                                    {
                                        v5 = strstr(pCurrAnim->name, "fast");
                                        if ( v5 )
                                        {
                                            fullspeeda = fullspeed * player_sprintSpeedScale->current.value;
                                            moveType = "sprint";
                                        }
                                        else
                                        {
                                            fullspeeda = fullspeed * 1.0;
                                            moveType = "run";
                                        }
                                    }
                                }
                            }
                            v9 = (float)((float)(fullspeeda * 100.0) / pCurrAnim->moveSpeed);
                            moveSpeed = pCurrAnim->moveSpeed;
                            v6 = Vec3Length(vDelta);
                            Com_Printf(
                                19,
                                "Anim '%s' moves %f units over %ims (%f units/s), will play back at %.1f%% speed when player moves at ful"
                                "l %s speed (%.0f units/sec)\n",
                                pCurrAnim->name,
                                v6,
                                (int)(float)(duration * 1000.0),
                                moveSpeed,
                                v9,
                                moveType,
                                fullspeeda);
                        }
                    }
                    if ( pCurrAnim->duration < 500 )
                        pCurrAnim->duration = 500;
                }
                if ( XAnimIsLooped(pXAnims, i) )
                    pCurrAnim->flags |= 0x200u;
                if ( animScript_listAnims->current.enabled )
                {
                    ++devNumNamedAnims;
                    Com_Printf(19, "%s\n", pCurrAnim->name);
                }
            }
            else
            {
                pCurrAnim->flags |= 1u;
                I_strncpyz(pCurrAnim->name, pLoadAnim->szAnimName, 64);
                pCurrAnim->nameHash = pLoadAnim->iNameHash;
                if ( !pCurrAnim->initialLerp )
                    pCurrAnim->initialLerp = -1;
                pCurrAnim->duration = 0;
                pCurrAnim->moveSpeed = 0.0f;
                pCurrAnim->rotSpeed = 0.0f;
                pCurrAnim->finalLerp = -1;
                pCurrAnim->forceAnimRate = -1.0f;
            }
        }
        else
        {
            pCurrAnim->flags |= 0x2000u;
            I_strncpyz(pCurrAnim->name, "unused", 64);
            pCurrAnim->nameHash = 0;
        }
        ++i;
        ++pCurrAnim;
    }
    if ( animScript_listAnims->current.enabled )
    {
        Com_Printf(19, "\n%i total player anims.\n", devNumNamedAnims);
        Com_Printf(19, "===========================================================\n");
    }
    BG_AnimParseAnimScript(
        (animScriptData_t *)bgs->animData,
        0,
        0,
        levelName);
    BG_SetupAnimNoteTypes((animScriptData_t *)bgs->animData); 
}

loadAnim_t *__cdecl BG_LoadAnimForAnimIndex(unsigned int iAnimIndex)
{
    unsigned int i; // [esp+0h] [ebp-8h]
    loadAnim_t *pAnim; // [esp+4h] [ebp-4h]

    if ( iAnimIndex >= bgs->animData->animScriptData.numAnimations)
        Com_Error(
            ERR_DROP,
            "Player animation index %i out of 0 to %i range",
            iAnimIndex,
            bgs->animData->animScriptData.numAnimations);
    i = 0;
    pAnim = g_pLoadAnims;
    while ( i < *g_piNumLoadAnims )
    {
        if ( iAnimIndex == pAnim->anim.index )
            return pAnim;
        ++i;
        ++pAnim;
    }
    return 0;
}

void __cdecl BG_SetupAnimNoteTypes(animScriptData_t *scriptData)
{
    int cmdIndex; // [esp+4h] [ebp-14h]
    animScript_t *script; // [esp+8h] [ebp-10h]
    animScriptItem_t *scriptItem; // [esp+Ch] [ebp-Ch]
    unsigned int animIndex; // [esp+10h] [ebp-8h]
    int itemIndex; // [esp+14h] [ebp-4h]

    BG_CheckThread();
    iassert(bgs);
    for (animIndex = 0; animIndex < scriptData->numAnimations; ++animIndex)
        scriptData->animations[animIndex].noteType = 0;
    if (!bgs->anim_user)
    {
        script = &scriptData->scriptEvents[7];
        for (itemIndex = 0; itemIndex < script->numItems; ++itemIndex)
        {
            scriptItem = script->items[itemIndex];
            for (cmdIndex = 0; cmdIndex < scriptItem->numCommands; ++cmdIndex)
            {
                if (scriptItem->commands[cmdIndex].bodyPart[0])
                    scriptData->animations[scriptItem->commands[cmdIndex].animIndex[0]].noteType = 1;
                if (scriptItem->commands[cmdIndex].bodyPart[1])
                    scriptData->animations[scriptItem->commands[cmdIndex].animIndex[1]].noteType = 1;
            }
        }
    }
}
