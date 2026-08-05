#include "ui_gametype_variants_mp.h"
#include "ui_gametype_custom_mp.h"

#include <cstring>
#include <universal/q_shared.h>
#include <bgame/bg_unlockable_items.h>
#include <qcommon/common.h>
#include <ui/ui_utils.h>
#include <stdlib.h>
#include <universal/memfile.h>
#include <universal/com_memory.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/scr_const.h>
#include <ui/ui_main.h>

const dvar_t *ui_gv_rulecount;
const dvar_t *ui_gv_reloadSpeedModifier;
const dvar_s *custom_class_mode;

GVValue gvCondOpValues[6] =
{
    { "CUSTOM_EQUAL_TO",                 "==" },
    { "CUSTOM_NOT_EQUAL_TO",             "!=" },
    { "CUSTOM_LESS_THAN",                "<"  },
    { "CUSTOM_LESS_THAN_OR_EQUAL_TO",    "<=" },
    { "CUSTOM_GREATER_THAN",             ">"  },
    { "CUSTOM_GREATER_THAN_OR_EQUAL_TO", ">=" }
};

GVValue classParamValues[10] =
{
    { "", "" },
    { "", "" },
    { "", "" },
    { "", "" },
    { "", "" },
    { "", "" },
    { "", "" },
    { "", "" },
    { "", "" },
    { "", "" }
};

_CustomClassData g_customGameModeClasses[10];

GVGlob gvGlob;

GVValue gvCondRoundsPlayedRhsValues[10] =
{
    { "1",  "1"  },
    { "2",  "2"  },
    { "3",  "3"  },
    { "4",  "4"  },
    { "5",  "5"  },
    { "6",  "6"  },
    { "7",  "7"  },
    { "8",  "8"  },
    { "9",  "9"  },
    { "10", "10" }
};

GVValue gvCondPlayersLeftRhsValues[12] =
{
    { "1",  "1"  },
    { "2",  "2"  },
    { "3",  "3"  },
    { "4",  "4"  },
    { "5",  "5"  },
    { "6",  "6"  },
    { "7",  "7"  },
    { "8",  "8"  },
    { "9",  "9"  },
    { "10", "10" },
    { "11", "11" },
    { "12", "12" }
};

GVValue gvPlaceValues[3] =
{
    { "CUSTOM_FIRST_PLACE", "first"  },
    { "CUSTOM_TOP_THREE",   "top3"   },
    { "CUSTOM_LAST_PLACE",  "last"   }
};

GVValue gvCondHitByRhsValues[4] =
{
    { "CUSTOM_HIT_BY_KNIFE", "knife" },
    { "CUSTOM_HEADSHOT",     "headshot" },
    { "CUSTOM_EXPLOSION",    "explosion" },
    { "CUSTOM_BULLET",       "bullet" }
};

GVConditionalLhs gvCondLhsValues[9] =
{
    {
        "CUSTOM_PLAYERS_LEFT",
        "PlayersLeft",
        GV_COND_MASK_GENERAL,
        { gvCondPlayersLeftRhsValues, 12, GVTYPE_INT }
    },
    {
        "CUSTOM_ROUNDS_PLAYED",
        "RoundsPlayed",
        GV_COND_MASK_GENERAL,
        { gvCondRoundsPlayedRhsValues, 10, GVTYPE_INT }
    },
    {
        "CUSTOM_HIT_BY",
        "HitBy",
        GV_COND_MASK_HIT_BY,
        { gvCondHitByRhsValues, 4, GVTYPE_STRING }
    },
    {
        "CUSTOM_PLAYERS_CLASS",
        "PlayersClass",
        GV_COND_MASK_PLAYER_CLASS,
        { classParamValues, 10, GVTYPE_STRING }
    },
    {
        "CUSTOM_VICTIMS_CLASS",
        "VictimsClass",
        GV_COND_MASK_ATTACK_CLASS,
        { classParamValues, 10, GVTYPE_STRING }
    },
    {
        "CUSTOM_ATTACKERS_CLASS",
        "AttackersClass",
        GV_COND_MASK_ATTACK_CLASS,
        { classParamValues, 10, GVTYPE_STRING }
    },
    {
        "CUSTOM_PLAYERS_PLACE",
        "PlayersPlace",
        GV_COND_MASK_PLAYER,
        { gvPlaceValues, 3, GVTYPE_STRING }
    },
    {
        "CUSTOM_VICTIMS_PLACE",
        "VictimsPlace",
        GV_COND_MASK_ATTACK,
        { gvPlaceValues, 3, GVTYPE_STRING }
    },
    {
        "CUSTOM_ATTACKERS_PLACE",
        "AttackersPlace",
        GV_COND_MASK_ATTACK,
        { gvPlaceValues, 3, GVTYPE_STRING }
    }
};

GVEvent gvEvents[10] =
{
    { "CUSTOM_ROUND_BEGIN",         "OnRoundBegin",              1,  1  },
    { "CUSTOM_ROUND_END",           "OnRoundEnd",                1,  1  },
    { "CUSTOM_PLAYER_HIT",          "OnPlayerTakeDamage",       19, 27 },
    { "CUSTOM_PLAYER_KILLED",       "OnKill",                   51, 25 },
    { "CUSTOM_PLAYER_ELIMINATED",   "OnPlayerElimination",      13, 25 },
    { "CUSTOM_PLAYER_SPAWN",        "OnPlayerSpawn",            13, 37 },
    { "CUSTOM_PLAYER_TEAM_CHANGE",  "OnPlayerTeamChange",       13, 37 },
    { "CUSTOM_PLAYER_CLASS_CHANGE", "OnPlayerClassChange",      13, 37 },
    { "CUSTOM_PLAYER_KS_EARNED",    "OnPlayerKillstreakEarned", 13, 37 },
    { "CUSTOM_PLAYER_KS_ACTIVATED", "OnPlayerKillstreakActivated", 13, 37 }
};

int gvEventCount;

GVValue healthRegenParamValues[6] =
{
    { "CUSTOM_NONE",      "0.0"  },
    { "CUSTOM_VERY_SLOW", "0.01" },
    { "CUSTOM_SLOW",      "0.05" },
    { "CUSTOM_NORMAL",    "0.1"  },
    { "CUSTOM_FAST",      "0.2"  },
    { "CUSTOM_VERY_FAST", "0.4"  }
};

GVValue livesParamValues[10] =
{
    { "1",  "1"  },
    { "2",  "2"  },
    { "3",  "3"  },
    { "4",  "4"  },
    { "5",  "5"  },
    { "6",  "6"  },
    { "7",  "7"  },
    { "8",  "8"  },
    { "9",  "9"  },
    { "10", "10" }
};

GVValue maxHealthParamValues[17] =
{
    { "1",     "1"     },
    { "25",    "25"    },
    { "50",    "50"    },
    { "75",    "75"    },
    { "100",   "100"   },
    { "125",   "125"   },
    { "150",   "150"   },
    { "175",   "175"   },
    { "200",   "200"   },
    { "225",   "225"   },
    { "250",   "250"   },
    { "275",   "275"   },
    { "300",   "300"   },
    { "400",   "400"   },
    { "500",   "500"   },
    { "750",   "750"   },
    { "1000",  "1000"  }
};

GVValue moveSpeedParamValues[11] =
{
    { "0.1",  "0.1"  },
    { "0.25", "0.25" },
    { "0.5",  "0.5"  },
    { "0.75", "0.75" },
    { "0.9",  "0.9"  },
    { "1.0",  "1.0"  },
    { "1.1",  "1.1"  },
    { "1.25", "1.25" },
    { "1.5",  "1.5"  },
    { "1.75", "1.75" },
    { "2.0",  "2.0"  }
};

GVValue radarParamValues[2] =
{
    { "CUSTOM_ENABLE",  "enable"  },
    { "CUSTOM_DISABLE", "disable" }
};

GVValue reloadParamValues[12] =
{
    { "0.1",  "0.1"  },
    { "0.25", "0.25" },
    { "0.5",  "0.5"  },
    { "0.75", "0.75" },
    { "0.9",  "0.9"  },
    { "1.0",  "1.0"  },
    { "1.1",  "1.1"  },
    { "1.25", "1.25" },
    { "1.5",  "1.5"  },
    { "1.75", "1.75" },
    { "2.0",  "2.0"  },
    { "4.0",  "4.0"  }
};

GVValue scoreParamValues[25] =
{
    { "1",    "1"    },
    { "5",    "5"    },
    { "10",   "10"   },
    { "15",   "15"   },
    { "20",   "20"   },
    { "25",   "25"   },
    { "30",   "30"   },
    { "35",   "35"   },
    { "40",   "40"   },
    { "45",   "45"   },
    { "50",   "50"   },
    { "100",  "100"  },
    { "150",  "150"  },
    { "200",  "200"  },
    { "250",  "250"  },
    { "300",  "300"  },
    { "350",  "350"  },
    { "400",  "400"  },
    { "450",  "450"  },
    { "500",  "500"  },
    { "600",  "600"  },
    { "700",  "700"  },
    { "800",  "800"  },
    { "900",  "900"  },
    { "1000", "1000" }
};

GVValue teamParamValues[3] =
{
    { "CUSTOM_TOGGLE", "toggle" },
    { "CUSTOM_TEAM_A", "allies" },
    { "CUSTOM_TEAM_B", "axis"   }
};

GVValue ammoParamValues[6] =
{
    { "1", "1" },
    { "2", "2" },
    { "3", "3" },
    { "4", "4" },
    { "5", "5" },
    { "6", "6" }
};

GVValue damageModParamValues[8] =
{
    { "0.25", "0.25" },
    { "0.5",  "0.5"  },
    { "0.75", "0.75" },
    { "1.0",  "1.0"  },
    { "1.25", "1.25" },
    { "1.5",  "1.5"  },
    { "1.75", "1.75" },
    { "2.0",  "2.0"  }
};

GVAction gvActions[25] =
{
    GVAction("CUSTOM_GIVE_AMMO",         "GiveAmmo",           1, ammoParamValues,        6),
    GVAction("CUSTOM_REMOVE_AMMO",       "RemoveAmmo",         1, ammoParamValues,        6),
    GVAction("CUSTOM_GIVE_SCORE",        "GiveScore",          1, scoreParamValues,       25),
    GVAction("CUSTOM_REMOVE_SCORE",      "RemoveScore",        1, scoreParamValues,       25),
    GVAction("CUSTOM_ENABLE_UAV",        "EnableUAV",          0, nullptr,                0),
    GVAction("CUSTOM_SET_HEADER",        "SetHeader",          4, nullptr,                0),
    GVAction("CUSTOM_SET_SUBHEADER",     "SetSubHeader",       4, nullptr,                0),
    GVAction("CUSTOM_ANNOUNCEMENT",      "DisplayMessage",     4, nullptr,                0),
    GVAction("CUSTOM_SET_HEALTH_REGEN",  "SetHealthRegen",     1, healthRegenParamValues, 6),
    GVAction("CUSTOM_CHANGE_CLASS",      "ChangeClass",        5, classParamValues,       10),
    GVAction("CUSTOM_CHANGE_TEAM",       "ChangeTeam",         3, teamParamValues,        3),
    GVAction("CUSTOM_GIVE_HEALTH",       "GiveHealth",         1, maxHealthParamValues,   17),
    GVAction("CUSTOM_REMOVE_HEALTH",     "RemoveHealth",       1, maxHealthParamValues,   17),
    GVAction("CUSTOM_GIVE_LIVES",        "GiveLives",          1, livesParamValues,       10),
    GVAction("CUSTOM_REMOVE_LIVES",      "RemoveLives",        1, livesParamValues,       10),
    GVAction("CUSTOM_SET_DAMAGE_MOD",    "SetDamageModifier",  1, damageModParamValues,   8),
    GVAction("CUSTOM_GIVE_INVULN",       "GiveInvuln",         0, nullptr,                0),
    GVAction("CUSTOM_REMOVE_INVULN",     "RemoveInvuln",       0, nullptr,                0),
    GVAction("CUSTOM_GIVE_PERK",         "GivePerk",           6, nullptr,                0),
    GVAction("CUSTOM_REMOVE_PERK",       "RemovePerk",         6, nullptr,                0),
    GVAction("CUSTOM_GIVE_KILLSTREAK",   "GiveKillstreak",     7, nullptr,                0),
    GVAction("CUSTOM_REMOVE_KILLSTREAK", "RemoveKillstreak",   7, nullptr,                0),
    GVAction("CUSTOM_SCALE_SPEED",       "ScaleMoveSpeed",     2, moveSpeedParamValues,   11),
    GVAction("CUSTOM_SHOW_ON_RADAR",     "ShowOnRadar",        3, radarParamValues,       2),
    GVAction("CUSTOM_SCALE_RELOAD_SPEED","ScaleReloadSpeed",   2, reloadParamValues,      12)
};
int gvActionCount;

GVTarget gvTargets[19] =
{
    { (char *)"CUSTOM_EVERYONE",                   (char *)"Everyone",                        1  },
    { (char *)"CUSTOM_PLAYERS_REMAINING",         (char *)"PlayersLeft",                     1  },
    { (char *)"CUSTOM_PLAYERS_ELIMINATED",        (char *)"PlayersEliminated",               1  },
    { (char *)"CUSTOM_ATTACKER",                   (char *)"Attacker",                        2  },
    { (char *)"CUSTOM_VICTIM",                     (char *)"Victim",                          2  },
    { (char *)"CUSTOM_PLAYER",                     (char *)"Player",                          4  },
    { (char *)"CUSTOM_PLAYERS_TEAM",               (char *)"PlayersTeam",                     8  },
    { (char *)"CUSTOM_OTHER_TEAM",                 (char *)"OtherTeam",                        8  },
    { (char *)"CUSTOM_ATTACKERS_TEAM",             (char *)"AttackersTeam",                   16 },
    { (char *)"CUSTOM_VICTIMS_TEAM",              (char *)"VictimsTeam",                     16 },
    { (char *)"CUSTOM_PLAYERS_LEFT_PLAYERS_TEAM", (char *)"PlayersLeftOnPlayersTeam",        8  },
    { (char *)"CUSTOM_PLAYERS_LEFT_OTHER_TEAM",   (char *)"PlayersLeftOnOtherTeam",          8  },
    { (char *)"CUSTOM_PLAYERS_ELIM_PLAYERS_TEAM", (char *)"PlayersEliminatedOnPlayersTeam",  8  },
    { (char *)"CUSTOM_PLAYERS_ELIM_OTHER_TEAM",   (char *)"PlayersEliminatedOnOtherTeam",    8  },
    { (char *)"CUSTOM_PLAYERS_LEFT_VICTIMS_TEAM", (char *)"PlayersLeftOnVictimsTeam",       16 },
    { (char *)"CUSTOM_PLAYERS_LEFT_ATTACKERS_TEAM",(char *)"PlayersLeftOnAttackersTeam",     16 },
    { (char *)"CUSTOM_PLAYERS_ELIM_VICTIMS_TEAM", (char *)"PlayersEliminatedOnVictimsTeam", 16 },
    { (char *)"CUSTOM_PLAYERS_ELIM_ATTACKERS_TEAM",(char *)"PlayersEliminatedOnAttackersTeam",16 },
    { (char *)"CUSTOM_ASSISTING_PLAYERS",         (char *)"AssistingPlayers",               32 }
};

int s_popupStackSize;
bool s_isFeederSelectionFromPopupStart;
int gvTargetCount;

menuDef_t *s_popupStack[12];

const char aDm_0[] = "dm";

const char *CUSTOM_CLASS_VALUES[10] =
{
  "CLASS_CUSTOM1",
  "CLASS_CUSTOM2",
  "CLASS_CUSTOM3",
  "CLASS_CUSTOM4",
  "CLASS_CUSTOM5",
  "CLASS_CUSTOM6",
  "CLASS_CUSTOM7",
  "CLASS_CUSTOM8",
  "CLASS_CUSTOM9",
  "CLASS_CUSTOM10"
};

int __cdecl UI_GameVariants_GetClassParameters()
{
    int i; // [esp+0h] [ebp-Ch]
    unsigned int c; // [esp+4h] [ebp-8h]
    int numClasses; // [esp+8h] [ebp-4h]

    numClasses = 0;
    for (c = 0; c < 0xA; ++c)
    {
        classParamValues[c].m_displayValue = "";
        classParamValues[c].m_value = "";
    }
    if (UI_Gametype_IsUsingCustom() && Dvar_GetInt(custom_class_mode))
    {
        numClasses = 0;
        for (i = 0; i < 10; ++i)
        {
            if (g_customGameModeClasses[i].isActive == 1)
            {
                classParamValues[i].m_displayValue = g_customGameModeClassDescriptions[i].name;
                classParamValues[i].m_value = CUSTOM_CLASS_VALUES[i];
                ++numClasses;
            }
        }
    }
    return numClasses;
}

void __cdecl UI_GV_StartAddingEvent_f()
{
    int GVEventCount; // eax

    GVEventCount = UI_GetGVEventCount();
    UI_GV_StartEditingFeeder(0, 40, GVEventCount);
}

void __cdecl UI_GV_StartEditingFeeder(int localClientNum, int feederId, int feederCount)
{
    int selection; // [esp+8h] [ebp-8h]
    uiInfo_s *dc; // [esp+Ch] [ebp-4h]

    s_isFeederSelectionFromPopupStart = 1;
    dc = UI_GetInfo(0);
    selection = Menu_GetFeederSelection(&dc->uiDC, 0, feederId, 0);
    if ( selection < 0 || selection >= feederCount )
    {
        Menu_SetFeederSelection(localClientNum, &dc->uiDC, 0, feederId, 0, 0);
        selection = 0;
    }
    UI_FeederSelection_GameVariants(localClientNum, dc->uiDC.contextIndex, (float)feederId, selection);
    s_isFeederSelectionFromPopupStart = 0;
}

void __cdecl UI_GV_StartAddingAction_f()
{
    int GVActionCount; // eax

    GVActionCount = UI_GetGVActionCount();
    UI_GV_StartEditingFeeder(0, 41, GVActionCount);
}

void __cdecl UI_GV_StartAddingParam_f()
{
    int GVParamCount; // eax

    GVParamCount = UI_GetGVParamCount();
    UI_GV_StartEditingFeeder(0, 42, GVParamCount);
}

void __cdecl UI_GV_StartAddingTarget_f()
{
    int GVTargetCount; // eax

    GVTargetCount = UI_GetGVTargetCount();
    UI_GV_StartEditingFeeder(0, 43, GVTargetCount);
}

void __cdecl UI_GV_StartAddingConditional_f()
{
    int GVCondLhsCount; // eax
    int GVCondOpCount; // eax
    int GVCondRhsCount; // eax

    GVCondLhsCount = UI_GetGVCondLhsCount();
    UI_GV_StartEditingFeeder(0, 35, GVCondLhsCount);
    GVCondOpCount = UI_GetGVCondOpCount();
    UI_GV_StartEditingFeeder(0, 36, GVCondOpCount);
    GVCondRhsCount = UI_GetGVCondRhsCount();
    UI_GV_StartEditingFeeder(0, 37, GVCondRhsCount);
    SetNextPopupToAddOrEditConfirm();
}

void SetNextPopupToAddOrEditConfirm()
{
    const char *nextPopup; // [esp+0h] [ebp-4h]

    nextPopup = "popup_gv_edit_selected";
    if ( Dvar_GetInt("ui_gv_iscreatingnew") )
        nextPopup = "popup_gv_add_new";
    Dvar_SetStringByName("ui_gv_nextpopup", (char*)nextPopup);
}

void __cdecl UI_GV_EditSelectedRule_f()
{
    int v0; // ecx
    GVGlob *v1; // edi
    GVRule *p_oldRule; // esi
    bool v3; // zf
    GVGlob *rule; // [esp+8h] [ebp-11Ch]
    GVRule oldRule; // [esp+Ch] [ebp-118h] BYREF

    rule = GetSelectedRule(0);
    if ( rule )
    {
        memcpy(&oldRule, rule, sizeof(oldRule));
        UI_GV_UpdateRuleFromDvars(rule->rules);
        v0 = 133;
        v1 = rule;
        p_oldRule = &oldRule;
        v3 = 1;
        do
        {
            if ( !v0 )
                break;
            v3 = *(_WORD *)&p_oldRule->isValid == *(_WORD *)&v1->rules[0].isValid;
            p_oldRule = (GVRule *)((char *)p_oldRule + 2);
            v1 = (GVGlob *)((char *)v1 + 2);
            --v0;
        }
        while ( v3 );
        if ( !v3 )
            UI_Gametype_MarkDirty();
    }
}

GVGlob *__cdecl GetSelectedRule(int localClientNum)
{
    int ruleSelected; // [esp+0h] [ebp-8h]
    uiInfo_s *dc; // [esp+4h] [ebp-4h]

    dc = UI_GetInfo(localClientNum);
    ruleSelected = Menu_GetFeederSelection(&dc->uiDC, 0, 44, "gametype_variants");
    return GetRuleForFeederIndex(ruleSelected);
}

GVGlob *__cdecl GetRuleForFeederIndex(int feederIndex)
{
    int i; // [esp+4h] [ebp-8h]
    int validIndex; // [esp+8h] [ebp-4h]

    if ( feederIndex < 0 || feederIndex >= gvGlob.ruleCount )
        return 0;
    validIndex = 0;
    for ( i = 0; i < 64; ++i )
    {
        if ( gvGlob.rules[i].isValid )
        {
            if ( validIndex == feederIndex )
                return (GVGlob *)((char *)&gvGlob + 266 * i);
            ++validIndex;
        }
    }
    return 0;
}

void __cdecl UI_GV_UpdateRuleFromDvars(GVRule *rule)
{
    int Int; // eax
    int v2; // eax
    int v3; // eax
    GVConditional result; // [esp+0h] [ebp-4h] BYREF

    if ( rule )
    {
        rule->isValid = 1;
        Int = Dvar_GetInt("ui_gv_event_index");
        rule->eventID = GetEventIndexFromFeederIndex(Int);
        v2 = Dvar_GetInt("ui_gv_action_index");
        rule->actionID = GetActionIndexFromFeederIndex(v2);
        v3 = Dvar_GetInt("ui_gv_target_index");
        rule->targetID = GetTargetIDFromFeederIndex(v3);
        rule->m_conditional = *GetConditionalForDvarIndex(
                                                         &result,
                                                         "ui_gv_condlhs_index",
                                                         "ui_gv_condop_index",
                                                         "ui_gv_condrhs_index");
        UI_GV_SetRuleParameterFromDisplayDvars(rule);
        s_popupStackSize = 0;
    }
}

int __cdecl GetEventIndexFromFeederIndex(int feederIndex)
{
    int i; // [esp+0h] [ebp-8h]
    int index; // [esp+4h] [ebp-4h]

    index = 0;
    for ( i = 0; i < gvEventCount; ++i )
    {
        if ( IsEventValid(&gvEvents[i]) )
        {
            if ( index == feederIndex )
                return i;
            ++index;
        }
    }
    return 0;
}

bool __cdecl IsEventValid(GVEvent *ev)
{
    if ( IsFreeForAllMode() && !I_stricmp(ev->m_scriptName, "OnPlayerTeamChange") )
        return 0;
    return AreCustomClassesAvailable() || I_stricmp(ev->m_scriptName, "OnPlayerClassChange");
}

bool __cdecl IsFreeForAllMode()
{
    return I_stricmp(ui_gametype->current.string, aDm_0) == 0;
}

char __cdecl AreCustomClassesAvailable()
{
    int c; // [esp+0h] [ebp-4h]

    if ( UI_Gametype_IsUsingCustom() && !Dvar_GetInt(custom_class_mode) )
        return 0;
    for ( c = 0; c < 10; ++c )
    {
        if ( g_customGameModeClasses[c].isActive == 1 )
            return 1;
    }
    return 0;
}

int __cdecl GetActionIndexFromFeederIndex(int feederIndex)
{
    int i; // [esp+0h] [ebp-8h]
    int index; // [esp+4h] [ebp-4h]

    index = 0;
    for ( i = 0; i < gvActionCount; ++i )
    {
        if ( IsActionValid(&gvActions[i]) )
        {
            if ( index == feederIndex )
                return i;
            ++index;
        }
    }
    return 0;
}

bool __cdecl IsActionValid(GVAction *action)
{
    if ( IsFreeForAllMode() && action->m_parameters == teamParamValues )
        return 0;
    return AreCustomClassesAvailable() || action->m_parameterType != 5;
}

int __cdecl GetTargetIDFromFeederIndex(int feederIndex)
{
    int i; // [esp+0h] [ebp-10h]
    unsigned __int8 mask; // [esp+7h] [ebp-9h]
    GVEvent *ev; // [esp+8h] [ebp-8h]
    int count; // [esp+Ch] [ebp-4h]

    ev = GetEventByDvarIndex("ui_gv_event_index");
    if (!ev)
        return 0;
    mask = GetValidTargetMask(ev->m_targetMask);
    count = 0;
    for (i = 0; i < gvTargetCount; ++i)
    {
        if ((mask & gvTargets[i].m_targetMask) != 0)
        {
            if (count == feederIndex)
                return i;
            ++count;
        }
    }
    return 0;
}

GVEvent *__cdecl GetEventByDvarIndex(const char *dvarName)
{
    int index; // [esp+0h] [ebp-4h]

    index = Dvar_GetInt(dvarName);
    if ( index >= 0 && index < UI_GetGVEventCount() )
        return &gvEvents[GetEventIndexFromFeederIndex(index)];
    else
        return 0;
}

unsigned __int8 __cdecl GetValidTargetMask(unsigned __int8 targetMask)
{
    if ( IsFreeForAllMode() )
        targetMask &= 0xE7u;
    return targetMask;
}

GVConditional *__cdecl GetConditionalForDvarIndex(
                GVConditional *result,
                const char *lhsDvarIndexName,
                const char *opDvarIndexName,
                const char *rhsDvarIndexName)
{
    unsigned __int8 CondLhsIndexForFeederIndex; // al
    unsigned __int8 Int; // [esp+1h] [ebp-7h]
    unsigned __int8 v7; // [esp+2h] [ebp-6h]
    int lhsFeederIndex; // [esp+4h] [ebp-4h]

    lhsFeederIndex = Dvar_GetInt(lhsDvarIndexName);
    if ( lhsFeederIndex >= 0 )
    {
        Int = Dvar_GetInt(rhsDvarIndexName);
        v7 = Dvar_GetInt(opDvarIndexName);
        CondLhsIndexForFeederIndex = GetCondLhsIndexForFeederIndex(lhsFeederIndex);
        result->hasConditional = 1;
        result->m_lhsIndex = CondLhsIndexForFeederIndex;
        result->m_operatorIndex = v7;
        result->m_rhsIndex = Int;
    }
    else
    {
        result->hasConditional = 0;
        result->m_lhsIndex = 0;
        result->m_operatorIndex = 0;
        result->m_rhsIndex = 0;
    }
    return result;
}

unsigned int __cdecl GetCondLhsIndexForFeederIndex(int feederIndex)
{
    unsigned int i; // [esp+0h] [ebp-10h]
    unsigned __int8 mask; // [esp+7h] [ebp-9h]
    GVEvent *ev; // [esp+8h] [ebp-8h]
    int count; // [esp+Ch] [ebp-4h]

    ev = GetEventByDvarIndex("ui_gv_event_index");
    if (!ev)
        return 0;
    mask = GetValidCondMask(ev->m_condMask);
    count = 0;
    for (i = 0; i < 9; ++i)
    {
        if ((gvCondLhsValues[i].m_type & mask) != 0)
        {
            if (count == feederIndex)
                return i;
            ++count;
        }
    }
    return 0;
}

unsigned __int8 __cdecl GetValidCondMask(unsigned __int8 condMask)
{
    if ( !AreCustomClassesAvailable() )
        condMask &= 0xCFu;
    return condMask;
}

void __cdecl UI_GV_SetRuleParameterFromDisplayDvars(GVRule *rule)
{
    bool v1; // [esp+4h] [ebp-Ch]
    const char *parameter; // [esp+8h] [ebp-8h]

    rule->parameter.value[0] = 0;
    rule->secondParameter.value[0] = 0;
    rule->secondParameter.hasParameter = 0;
    parameter = Dvar_GetString("ui_gv_param");
    v1 = parameter && *parameter;
    rule->parameter.hasParameter = v1;
    if ( rule->parameter.hasParameter )
    {
        switch ( gvActions[rule->actionID].m_parameterType )
        {
            case 1u:
            case 2u:
            case 3u:
            case 5u:
                parameter = GetParamValueForDvarIndex(rule->actionID, "ui_gv_param_index");
                break;
            case 6u:
            case 7u:
                parameter = GetPerkParamValueFromDvarIndex(rule, "ui_gv_param_index");
                break;
            default:
                break;
        }
        if ( parameter )
        {
            if ( *parameter )
                I_strncpyz(rule->parameter.value, parameter, 128);
        }
    }
}

const char *__cdecl GetParamValueForDvarIndex(int actionIndex, const char *paramIndexDvarName)
{
    return gvActions[actionIndex].m_parameters[Dvar_GetInt(paramIndexDvarName)].m_value;
}

const char *__cdecl GetPerkParamValueFromDvarIndex(GVRule *rule, const char *paramDvarIndexName)
{
    int Int; // eax
    const char *ItemImage; // eax
    int itemIndex; // [esp+0h] [ebp-4h]

    Int = Dvar_GetInt(paramDvarIndexName);
    itemIndex = BG_UnlockablesGetItemIndexInSlot(Int);
    rule->secondParameter.hasParameter = 1;
    ItemImage = BG_UnlockablesGetItemImage(itemIndex);
    I_strncpyz(rule->secondParameter.value, ItemImage, 128);
    return BG_UnlockablesGetItemRef(itemIndex);
}

void __cdecl UI_GV_AddNewRule_f()
{
    uiInfo_s *dc; // [esp+0h] [ebp-8h]
    int ruleIndex; // [esp+4h] [ebp-4h]

    if ( gvGlob.ruleCount < 64 )
    {
        UI_Gametype_MarkDirty();
        ruleIndex = UI_GV_GetFirstInvalidRuleIndex();
        UI_GV_UpdateRuleFromDvars(&gvGlob.rules[ruleIndex]);
        Dvar_SetInt((dvar_s *)ui_gv_rulecount, ++gvGlob.ruleCount);
        dc = UI_GetInfo(0);
        Menu_SetFeederSelection(0, &dc->uiDC, 0, 44, ruleIndex, 0);
    }
    else
    {
        UI_Gametype_ShowErrorPopup((char*)"@MENU_ERROR", (char *)"@CUSTOM_MAX_RULES_REACHED");
    }
}

int __cdecl UI_GV_GetFirstInvalidRuleIndex()
{
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 64; ++i )
    {
        if ( !gvGlob.rules[i].isValid )
            return i;
    }
    return -1;
}

void __cdecl UI_GV_DeleteSelected_f()
{
    GVGlob *rule; // [esp+0h] [ebp-4h]

    rule = GetSelectedRule(0);
    if ( rule )
    {
        UI_Gametype_MarkDirty();
        rule->rules[0].isValid = 0;
        Dvar_SetInt((dvar_s *)ui_gv_rulecount, --gvGlob.ruleCount);
    }
}

void __cdecl UI_GV_NavForward_f()
{
    uiInfo_s *dc; // [esp+0h] [ebp-4h]

    if ( s_popupStackSize < 12 )
    {
        dc = UI_GetInfo(0);
        s_popupStack[s_popupStackSize++] = Menu_GetFocused(&dc->uiDC);
    }
    else
    {
        Com_PrintWarning(14, "Reached max popup stack size for adding/editing game variant rules.\n");
    }
}

void __cdecl UI_GV_NavBackward_f()
{
    uiInfo_s *dc; // [esp+4h] [ebp-4h]

    if ( s_popupStackSize > 0 )
    {
        dc = UI_GetInfo(0);
        Menus_Open(0, &dc->uiDC, s_popupStack[--s_popupStackSize]);
    }
}

void __cdecl UI_GV_UpdateSelectedRule_f()
{
    int ruleSelected; // [esp+8h] [ebp-8h]
    uiInfo_s *dc; // [esp+Ch] [ebp-4h]

    dc = UI_GetInfo(0);
    ruleSelected = Menu_GetFeederSelection(&dc->uiDC, 0, 44, "gametype_variants");
    UI_FeederSelection_GameVariants(0, dc->uiDC.contextIndex, 44.0, ruleSelected);
}

cmd_function_s UI_GV_NavForward_f_VAR;
cmd_function_s UI_GV_NavBackward_f_VAR;
cmd_function_s UI_GV_UpdateSelectedRule_f_VAR;
cmd_function_s UI_GV_StartAddingEvent_f_VAR;
cmd_function_s UI_GV_StartAddingAction_f_VAR;
cmd_function_s UI_GV_StartAddingTarget_f_VAR;
cmd_function_s UI_GV_StartAddingParam_f_VAR;
cmd_function_s UI_GV_StartAddingConditional_f_VAR;
cmd_function_s UI_GV_EditSelectedRule_f_VAR;
cmd_function_s UI_GV_AddNewRule_f_VAR;
cmd_function_s UI_GV_DeleteSelected_f_VAR;
cmd_function_s UI_GV_ResetFeeder_f_VAR;

void __cdecl UI_InitGametypeVariants()
{
    memset(&gvGlob, 0, sizeof(gvGlob));
    gvEventCount = 10;
    gvActionCount = 25;
    gvTargetCount = 19;
    gvGlob.ruleCount = 0;
    ui_gv_rulecount = _Dvar_RegisterInt(
                                            "ui_gv_rulecount",
                                            0,
                                            0,
                                            0x7FFFFFFF,
                                            0,
                                            "How many advanced game rules are created.");
    ui_gv_reloadSpeedModifier = _Dvar_RegisterFloat(
                                                                "ui_gv_reloadSpeedModifier",
                                                                0.0,
                                                                0.0,
                                                                4.0,
                                                                0,
                                                                "Adjusts the reload speed");
    Cmd_AddCommandInternal("ui_gv_navforward", UI_GV_NavForward_f, &UI_GV_NavForward_f_VAR);
    Cmd_AddCommandInternal("ui_gv_navbackward", UI_GV_NavBackward_f, &UI_GV_NavBackward_f_VAR);
    Cmd_AddCommandInternal("ui_gv_updateselectedrule", UI_GV_UpdateSelectedRule_f, &UI_GV_UpdateSelectedRule_f_VAR);
    Cmd_AddCommandInternal("ui_gv_startaddingevent", UI_GV_StartAddingEvent_f, &UI_GV_StartAddingEvent_f_VAR);
    Cmd_AddCommandInternal("ui_gv_startaddingaction", UI_GV_StartAddingAction_f, &UI_GV_StartAddingAction_f_VAR);
    Cmd_AddCommandInternal("ui_gv_startaddingtarget", UI_GV_StartAddingTarget_f, &UI_GV_StartAddingTarget_f_VAR);
    Cmd_AddCommandInternal("ui_gv_startaddingparam", UI_GV_StartAddingParam_f, &UI_GV_StartAddingParam_f_VAR);
    Cmd_AddCommandInternal("ui_gv_startaddingcond", UI_GV_StartAddingConditional_f, &UI_GV_StartAddingConditional_f_VAR);
    Cmd_AddCommandInternal("ui_gv_editselectedrule", UI_GV_EditSelectedRule_f, &UI_GV_EditSelectedRule_f_VAR);
    Cmd_AddCommandInternal("ui_gv_addnewrule", UI_GV_AddNewRule_f, &UI_GV_AddNewRule_f_VAR);
    Cmd_AddCommandInternal("ui_gv_deleteselected", UI_GV_DeleteSelected_f, &UI_GV_DeleteSelected_f_VAR);
    Cmd_AddCommandInternal("ui_gv_resetfeeder", UI_GV_ResetFeeder_f, &UI_GV_ResetFeeder_f_VAR);
}

void __cdecl UI_GV_ResetFeeder_f()
{
    const char *v0; // eax
    itemDef_s *item; // [esp+18h] [ebp-10h]
    listBoxDef_s *listPtr; // [esp+1Ch] [ebp-Ch]
    int index; // [esp+20h] [ebp-8h]
    uiInfo_s *dc; // [esp+24h] [ebp-4h]

    if ( Cmd_Argc() == 2 )
    {
        item = Cmd_ItemDef();
        if ( item )
        {
            listPtr = Item_GetListBoxDef(item);
            if ( listPtr )
            {
                v0 = Cmd_Argv(1);
                index = atoi(v0);
                dc = UI_GetInfo(0);
                Menu_SetFeederSelection(0, &dc->uiDC, 0, (int)listPtr->special, index, 0);
            }
        }
    }
    else
    {
        Com_Printf(13, "Usage: ui_gv_resetfeeder index\n");
    }
}

int __cdecl UI_GetGVRulesCount()
{
    return gvGlob.ruleCount;
}

int __cdecl UI_GetGVEventCount()
{
    return GetValidEventCount();
}

int __cdecl GetValidEventCount()
{
    int i; // [esp+0h] [ebp-8h]
    int count; // [esp+4h] [ebp-4h]

    count = 0;
    for ( i = 0; i < gvEventCount; ++i )
    {
        if ( IsEventValid(&gvEvents[i]) )
            ++count;
    }
    return count;
}

int __cdecl UI_GetGVActionCount()
{
    return GetValidActionCount();
}

int __cdecl GetValidActionCount()
{
    int i; // [esp+0h] [ebp-8h]
    int count; // [esp+4h] [ebp-4h]

    count = 0;
    for ( i = 0; i < gvActionCount; ++i )
    {
        if ( IsActionValid(&gvActions[i]) )
            ++count;
    }
    return count;
}

int __cdecl UI_GetGVTargetCount()
{
    GVEvent *ev; // [esp+0h] [ebp-4h]

    ev = GetEventByDvarIndex("ui_gv_event_index");
    if ( ev )
        return GetValidTargetCount(ev->m_targetMask);
    else
        return 0;
}

int __cdecl GetValidTargetCount(unsigned __int8 targetMask)
{
    int i; // [esp+0h] [ebp-Ch]
    unsigned __int8 mask; // [esp+7h] [ebp-5h]
    int count; // [esp+8h] [ebp-4h]

    mask = GetValidTargetMask(targetMask);
    count = 0;
    for (i = 0; i < gvTargetCount; ++i)
    {
        if ((mask & gvTargets[i].m_targetMask) != 0)
            ++count;
    }
    return count;
}

int __cdecl UI_GetGVParamCount()
{
    GVAction *action; // [esp+0h] [ebp-4h]

    action = GetActionByDvarIndex("ui_gv_action_index");
    if ( action )
        return action->m_parametersSize;
    else
        return 0;
}

GVAction *__cdecl GetActionByDvarIndex(const char *dvarName)
{
    int index; // [esp+0h] [ebp-4h]

    index = Dvar_GetInt(dvarName);
    if ( index >= 0 && index < UI_GetGVActionCount() )
        return &gvActions[GetActionIndexFromFeederIndex(index)];
    else
        return 0;
}

int __cdecl UI_GetGVCondLhsCount()
{
    GVEvent *ev; // [esp+0h] [ebp-4h]

    ev = GetEventByDvarIndex("ui_gv_event_index");
    if ( ev )
        return GetValidCondLhsCount(ev->m_condMask);
    else
        return 0;
}

int __cdecl GetValidCondLhsCount(unsigned __int8 condMask)
{
    unsigned int i; // [esp+0h] [ebp-Ch]
    unsigned __int8 mask; // [esp+7h] [ebp-5h]
    int count; // [esp+8h] [ebp-4h]

    mask = GetValidCondMask(condMask);
    count = 0;
    for (i = 0; i < 9; ++i)
    {
        if ((gvCondLhsValues[i].m_type & mask) != 0)
            ++count;
    }
    return count;
}

int __cdecl UI_GetGVCondOpCount()
{
    if ( GetCondLhsForDvarIndex("ui_gv_condlhs_index")->m_type <= GV_COND_MASK_GENERAL )
        return 6;
    else
        return 1;
}

GVConditionalLhs *__cdecl GetCondLhsForDvarIndex(const char *lhsDvarIndexName)
{
    int Int; // eax

    Int = Dvar_GetInt(lhsDvarIndexName);
    return GetCondLhsForFeederIndex(Int);
}

GVConditionalLhs *__cdecl GetCondLhsForFeederIndex(int feederIndex)
{
    return &gvCondLhsValues[GetCondLhsIndexForFeederIndex(feederIndex)];
}

int __cdecl UI_GetGVCondRhsCount()
{
    return GetCondLhsForDvarIndex("ui_gv_condlhs_index")->m_rhs.m_valuesSize;
}

int __cdecl UI_FeederCount_GameVariants(int contextIndex, float feederID)
{
    int result; // eax

    switch ( (int)feederID )
    {
        case '#':
            result = UI_GetGVCondLhsCount();
            break;
        case '$':
            result = UI_GetGVCondOpCount();
            break;
        case '%':
            result = UI_GetGVCondRhsCount();
            break;
        case '\'':
        case ',':
            result = UI_GetGVRulesCount();
            break;
        case '(':
            result = UI_GetGVEventCount();
            break;
        case ')':
            result = UI_GetGVActionCount();
            break;
        case '*':
            result = UI_GetGVParamCount();
            break;
        case '+':
            result = UI_GetGVTargetCount();
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

char *__cdecl GetItemGroupDisplayValue(itemGroup_t itemGroup, const char *value)
{
    const char *ItemRef; // eax
    const char *ItemName; // eax
    int itemIndex; // [esp+0h] [ebp-Ch]
    int item; // [esp+4h] [ebp-8h]
    int itemsInGroup; // [esp+8h] [ebp-4h]

    itemsInGroup = BG_UnlockablesBuildItemListForGroup(itemGroup);
    for ( item = 0; item < itemsInGroup; ++item )
    {
        itemIndex = BG_UnlockablesGetItemIndexInSlot(item);
        ItemRef = BG_UnlockablesGetItemRef(itemIndex);
        if ( !I_stricmp(ItemRef, value) )
        {
            ItemName = BG_UnlockablesGetItemName(itemIndex);
            return UI_SafeTranslateString(ItemName);
        }
    }
    return (char *)"";
}

char *__cdecl UI_FeederItemText_GameVariantRules(int index, int column)
{
    char *result; // eax
    char *displayValue; // [esp+4h] [ebp-8h]
    GVGlob *rule; // [esp+8h] [ebp-4h]

    rule = GetRuleForFeederIndex(index);
    if ( !rule )
        return (char *)"";
    switch ( column )
    {
        case 0:
            result = UI_SafeTranslateString(gvEvents[rule->rules[0].eventID].m_name);
            break;
        case 1:
            result = UI_SafeTranslateString(gvActions[rule->rules[0].actionID].m_name);
            break;
        case 2:
            displayValue = UI_GV_GetRuleParamDisplayValue(rule->rules);
            if ( displayValue && *displayValue )
                result = displayValue;
            else
                result = UI_SafeTranslateString("CUSTOM_GV_PARAM_EMPTY");
            break;
        case 3:
            result = UI_SafeTranslateString(gvTargets[rule->rules[0].targetID].m_name);
            break;
        case 4:
            if ( rule->rules[0].m_conditional.hasConditional )
                result = UI_SafeTranslateString("MPUI_YES");
            else
                result = UI_SafeTranslateString("MPUI_NONE");
            break;
        default:
            result = (char *)"";
            break;
    }
    return result;
}

char *__cdecl UI_GV_GetRuleParamDisplayValue(GVRule *rule)
{
    char *result; // eax
    int ParamIndexForValue; // eax
    GVAction *action; // [esp+4h] [ebp-4h]

    action = &gvActions[rule->actionID];
    switch ( action->m_parameterType )
    {
        case 0u:
            result = (char *)"";
            break;
        case 1u:
        case 2u:
        case 3u:
        case 5u:
            if ( rule->parameter.hasParameter )
            {
                ParamIndexForValue = GetParamIndexForValue(rule->actionID, rule->parameter.value);
                result = UI_SafeTranslateString(action->m_parameters[ParamIndexForValue].m_displayValue);
            }
            else
            {
                result = (char *)"";
            }
            break;
        case 4u:
            result = rule->parameter.value;
            break;
        case 6u:
            result = GetItemGroupDisplayValue(ITEMGROUP_SPECIALTY, rule->parameter.value);
            break;
        case 7u:
            result = GetItemGroupDisplayValue(ITEMGROUP_GLOBAL_ITEMS_START, rule->parameter.value);
            break;
        default:
            result = (char *)"";
            break;
    }
    return result;
}

int __cdecl GetParamIndexForValue(int actionIndex, const char *paramValue)
{
    int i; // [esp+0h] [ebp-8h]
    GVAction *action; // [esp+4h] [ebp-4h]

    action = &gvActions[actionIndex];
    if ( !action->m_parameters )
        return 0;
    for ( i = 0; i < action->m_parametersSize; ++i )
    {
        if ( !I_stricmp(action->m_parameters[i].m_value, paramValue) )
            return i;
    }
    return 0;
}

char *__cdecl UI_FeederItemText_GameVariantEvents(int index, int column)
{
    int EventIndexFromFeederIndex; // eax

    if ( index < 0 || index >= UI_GetGVEventCount() )
        return (char *)"";
    if ( column )
        return (char *)"";
    EventIndexFromFeederIndex = GetEventIndexFromFeederIndex(index);
    return UI_SafeTranslateString(gvEvents[EventIndexFromFeederIndex].m_name);
}

char *__cdecl UI_FeederItemText_GameVariantActions(int index, int column)
{
    int ActionIndexFromFeederIndex; // eax

    if ( index < 0 || index >= UI_GetGVActionCount() )
        return (char *)"";
    if ( column )
        return (char *)"";
    ActionIndexFromFeederIndex = GetActionIndexFromFeederIndex(index);
    return UI_SafeTranslateString(gvActions[ActionIndexFromFeederIndex].m_name);
}

char *__cdecl UI_FeederItemText_GameVariantTargets(int index, int column)
{
    GVTarget *target; // [esp+0h] [ebp-4h]

    if ( index < 0 || index >= UI_GetGVTargetCount() )
        return (char *)"";
    if ( column )
        return (char *)"";
    target = GetTargetAtFeederIndex(index);
    if ( target )
        return UI_SafeTranslateString(target->m_name);
    else
        return (char *)"";
}

GVTarget *__cdecl GetTargetAtFeederIndex(int feederIndex)
{
    return &gvTargets[GetTargetIDFromFeederIndex(feederIndex)];
}

char *__cdecl UI_FeederItemText_GameVariantParams(int index, int column)
{
    int ItemIndexInSlot; // eax
    const char *ItemName; // eax
    GVAction *action; // [esp+0h] [ebp-4h]

    if ( index < 0 || index >= UI_GetGVParamCount() )
        return (char *)"";
    if ( column )
        return (char *)"";
    action = GetActionByDvarIndex("ui_gv_action_index");
    if ( action->m_parameterType == 6 || action->m_parameterType == 7 )
    {
        ItemIndexInSlot = BG_UnlockablesGetItemIndexInSlot(index);
        ItemName = BG_UnlockablesGetItemName(ItemIndexInSlot);
        return UI_SafeTranslateString(ItemName);
    }
    else if ( action->m_parameters )
    {
        return UI_SafeTranslateString(action->m_parameters[index].m_displayValue);
    }
    else
    {
        return (char *)"";
    }
}

char *__cdecl UI_FeederItemText_GameVariantCondLhs(int index, int column)
{
    GVConditionalLhs *lhs; // [esp+0h] [ebp-4h]

    if ( index < 0 || index >= UI_GetGVCondLhsCount() )
        return (char *)"";
    if ( column )
        return (char *)"";
    lhs = GetCondLhsForFeederIndex(index);
    return UI_SafeTranslateString(lhs->m_displayName);
}

char *__cdecl UI_FeederItemText_GameVariantCondOp(int index, int column)
{
    if ( index < 0 || index >= UI_GetGVCondOpCount() )
        return (char *)"";
    if ( column )
        return (char *)"";
    return UI_SafeTranslateString(gvCondOpValues[index].m_displayValue);
}

char *__cdecl UI_FeederItemText_GameVariantCondRhs(int index, int column)
{
    GVConditionalLhs *lhs; // [esp+0h] [ebp-4h]

    lhs = GetCondLhsForDvarIndex("ui_gv_condlhs_index");
    if ( index < 0 || index >= lhs->m_rhs.m_valuesSize )
        return (char *)"";
    if ( column )
        return (char *)"";
    return UI_SafeTranslateString(lhs->m_rhs.m_values[index].m_displayValue);
}

char *__cdecl UI_FeederItemText_GameVariants(float feederID, int index, int column)
{
    char *result; // eax

    switch ( (int)feederID )
    {
        case '#':
            result = UI_FeederItemText_GameVariantCondLhs(index, column);
            break;
        case '$':
            result = UI_FeederItemText_GameVariantCondOp(index, column);
            break;
        case '%':
            result = UI_FeederItemText_GameVariantCondRhs(index, column);
            break;
        case '\'':
            result = UI_FeederItemText_GameVariantRules(index, column);
            break;
        case '(':
            result = UI_FeederItemText_GameVariantEvents(index, column);
            break;
        case ')':
            result = UI_FeederItemText_GameVariantActions(index, column);
            break;
        case '*':
            result = UI_FeederItemText_GameVariantParams(index, column);
            break;
        case '+':
            result = UI_FeederItemText_GameVariantTargets(index, column);
            break;
        case ',':
            result = UI_FeederItemText_GameVariantRules(index, 1);
            break;
        default:
            result = (char *)"";
            break;
    }
    return result;
}

void __cdecl UI_FeederSelection_GameVariantRules(int contextIndex, int index)
{
    int FeederIndexFromEventIndex; // eax
    int FeederIndexFromActionIndex; // eax
    int FeederIndexFromTargetId; // eax
    int ParamIndexForValue; // eax
    int FeederIndexForCondLhsIndex; // eax
    GVGlob *rule; // [esp+0h] [ebp-8h]
    uiInfo_s *dc; // [esp+4h] [ebp-4h]

    if ( index >= 0 && index < gvGlob.ruleCount )
    {
        rule = GetRuleForFeederIndex(index);
        s_isFeederSelectionFromPopupStart = 1;
        dc = UI_UIContext_GetInfo(contextIndex);
        FeederIndexFromEventIndex = GetFeederIndexFromEventIndex(rule->rules[0].eventID);
        Menu_SetFeederSelection(0, &dc->uiDC, 0, 40, FeederIndexFromEventIndex, "popup_gv_select_event");
        FeederIndexFromActionIndex = GetFeederIndexFromActionIndex(rule->rules[0].actionID);
        Menu_SetFeederSelection(0, &dc->uiDC, 0, 41, FeederIndexFromActionIndex, "popup_gv_select_action");
        FeederIndexFromTargetId = GetFeederIndexFromTargetId(
            rule->rules[0].targetID,
            gvEvents[rule->rules[0].eventID].m_targetMask);
        Menu_SetFeederSelection(0, &dc->uiDC, 0, 43, FeederIndexFromTargetId, "popup_gv_select_target");
        ParamIndexForValue = GetParamIndexForValue(rule->rules[0].actionID, rule->rules[0].parameter.value);
        Menu_SetFeederSelection(0, &dc->uiDC, 0, 42, ParamIndexForValue, "popup_gv_select_param");
        FeederIndexForCondLhsIndex = GetFeederIndexForCondLhsIndex(
            rule->rules[0].m_conditional.m_lhsIndex,
            gvEvents[rule->rules[0].eventID].m_condMask);
        Menu_SetFeederSelection(0, &dc->uiDC, 0, 35, FeederIndexForCondLhsIndex, "popup_gv_select_cond");
        Menu_SetFeederSelection(0, &dc->uiDC, 0, 36, rule->rules[0].m_conditional.m_operatorIndex, "popup_gv_select_cond");
        Menu_SetFeederSelection(0, &dc->uiDC, 0, 37, rule->rules[0].m_conditional.m_rhsIndex, "popup_gv_select_cond");
        if (!rule->rules[0].m_conditional.hasConditional)
        {
            Dvar_SetStringByName("ui_gv_condlhs", (char *)"");
            Dvar_SetStringByName("ui_gv_condop", (char *)"");
            Dvar_SetStringByName("ui_gv_condrhs", (char *)"");
        }
        s_isFeederSelectionFromPopupStart = 0;
    }
    else
    {
        Dvar_SetStringByName("ui_gv_event", (char *)"");
        Dvar_SetStringByName("ui_gv_action", (char *)"");
        Dvar_SetStringByName("ui_gv_target", (char *)"");
        Dvar_SetStringByName("ui_gv_param", (char *)"");
        Dvar_SetStringByName("ui_gv_condlhs", (char *)"");
        Dvar_SetStringByName("ui_gv_condop", (char *)"");
        Dvar_SetStringByName("ui_gv_condrhs", (char *)"");
        Dvar_SetIntByName("ui_gv_event_index", 0);
        Dvar_SetIntByName("ui_gv_action_index", 0);
        Dvar_SetIntByName("ui_gv_target_index", 0);
        Dvar_SetIntByName("ui_gv_param_index", 0);
        Dvar_SetIntByName("ui_gv_condlhs_index", 0);
        Dvar_SetIntByName("ui_gv_condop_index", 0);
        Dvar_SetIntByName("ui_gv_condrhs_index", 0);
    }
}

int __cdecl GetFeederIndexFromEventIndex(int eventIndex)
{
    int i; // [esp+0h] [ebp-8h]
    int index; // [esp+4h] [ebp-4h]

    index = 0;
    for ( i = 0; i < gvEventCount; ++i )
    {
        if ( IsEventValid(&gvEvents[i]) )
        {
            if ( i == eventIndex )
                return index;
            ++index;
        }
    }
    return 0;
}

int __cdecl GetFeederIndexFromActionIndex(int actionIndex)
{
    int i; // [esp+0h] [ebp-8h]
    int index; // [esp+4h] [ebp-4h]

    index = 0;
    for ( i = 0; i < gvActionCount; ++i )
    {
        if ( IsActionValid(&gvActions[i]) )
        {
            if ( i == actionIndex )
                return index;
            ++index;
        }
    }
    return 0;
}

int __cdecl GetFeederIndexFromTargetId(int targetId, unsigned __int8 targetMask)
{
    int i; // [esp+0h] [ebp-Ch]
    unsigned __int8 mask; // [esp+7h] [ebp-5h]
    int count; // [esp+8h] [ebp-4h]

    mask = GetValidTargetMask(targetMask);
    count = 0;
    for (i = 0; i < gvTargetCount; ++i)
    {
        if ((mask & gvTargets[i].m_targetMask) != 0)
        {
            if (i == targetId)
                return count;
            ++count;
        }
    }
    return 0;
}

int __cdecl GetFeederIndexForCondLhsIndex(int condLhsIndex, unsigned __int8 condMask)
{
    unsigned int i; // [esp+0h] [ebp-Ch]
    unsigned __int8 mask; // [esp+7h] [ebp-5h]
    int count; // [esp+8h] [ebp-4h]

    mask = GetValidCondMask(condMask);
    count = 0;
    for (i = 0; i < 9; ++i)
    {
        if ((gvCondLhsValues[i].m_type & mask) != 0)
        {
            if (i == condLhsIndex)
                return count;
            ++count;
        }
    }
    return 0;
}

void __cdecl UI_FeederSelection_GameVariantEvent(int contextIndex, int index)
{
    char *v2; // eax

    v2 = UI_FeederItemText_GameVariantEvents(index, 0);
    Dvar_SetStringByName("ui_gv_event", v2);
    Dvar_SetIntByName("ui_gv_event_index", index);
    Dvar_SetStringByName("ui_gv_nextpopup", (char*)"popup_gv_select_action");
}

void __cdecl UI_FeederSelection_GameVariantAction(int localClientNum, int contextIndex, int index)
{
    uiInfo_s *dc; // [esp+0h] [ebp-10h]
    char *actionName; // [esp+4h] [ebp-Ch]
    const char *nextPopup; // [esp+8h] [ebp-8h]
    GVAction *action; // [esp+Ch] [ebp-4h]

    if ( !s_isFeederSelectionFromPopupStart )
    {
        dc = UI_UIContext_GetInfo(contextIndex);
        Menu_SetFeederSelection(localClientNum, &dc->uiDC, 0, 42, 0, "popup_gv_select_param");
        Dvar_SetStringByName("ui_gv_param", (char *)"");
        Dvar_SetIntByName("ui_gv_param_index", 0);
    }
    actionName = UI_FeederItemText_GameVariantActions(index, 0);
    Dvar_SetStringByName("ui_gv_action", actionName);
    Dvar_SetIntByName("ui_gv_action_index", index);
    action = &gvActions[GetActionIndexFromFeederIndex(index)];
    nextPopup = "popup_gv_select_param";
    if ( action->m_parameterType == 4 )
    {
        Dvar_SetStringByName("ui_gv_nextpopup", (char *)"popup_gv_enter_param");
    }
    else
    {
        switch ( action->m_parameterType )
        {
            case 5u:
                action->m_parametersSize = UI_GameVariants_GetClassParameters();
                break;
            case 6u:
                action->m_parametersSize = BG_UnlockablesBuildItemListForGroup(ITEMGROUP_SPECIALTY);
                break;
            case 7u:
                action->m_parametersSize = BG_UnlockablesBuildItemListForGroup(ITEMGROUP_GLOBAL_ITEMS_START);
                break;
            default:
                if ( !action->m_parametersSize )
                    nextPopup = "popup_gv_select_target";
                break;
        }
        Dvar_SetStringByName("ui_gv_nextpopup", (char *)nextPopup);
    }
}

void __cdecl UI_FeederSelection_GameVariantTarget(int contextIndex, int index)
{
    char *v2; // eax

    v2 = UI_FeederItemText_GameVariantTargets(index, 0);
    Dvar_SetStringByName("ui_gv_target", v2);
    Dvar_SetIntByName("ui_gv_target_index", index);
    Dvar_SetStringByName("ui_gv_nextpopup", (char *)"popup_gv_select_cond");
}

void __cdecl UI_FeederSelection_GameVariantCondOp(int contextIndex, int index)
{
    char *v2; // eax

    v2 = UI_FeederItemText_GameVariantCondOp(index, 0);
    Dvar_SetStringByName("ui_gv_condop", v2);
    Dvar_SetIntByName("ui_gv_condop_index", index);
}

void __cdecl UI_FeederSelection_GameVariantCondRhs(int contextIndex, int index)
{
    char *v2; // eax

    v2 = UI_FeederItemText_GameVariantCondRhs(index, 0);
    Dvar_SetStringByName("ui_gv_condrhs", v2);
    Dvar_SetIntByName("ui_gv_condrhs_index", index);
}

void __cdecl UI_FeederSelection_GameVariants(int localClientNum, int contextIndex, float feederID, int index)
{
    switch ( (int)feederID )
    {
        case '#':
            UI_FeederSelection_GameVariantCondLhs(localClientNum, contextIndex, index);
            break;
        case '$':
            UI_FeederSelection_GameVariantCondOp(contextIndex, index);
            break;
        case '%':
            UI_FeederSelection_GameVariantCondRhs(contextIndex, index);
            break;
        case '\'':
        case ',':
            UI_FeederSelection_GameVariantRules(contextIndex, index);
            break;
        case '(':
            UI_FeederSelection_GameVariantEvent(contextIndex, index);
            break;
        case ')':
            UI_FeederSelection_GameVariantAction(localClientNum, contextIndex, index);
            break;
        case '*':
            UI_FeederSelection_GameVariantParam(localClientNum, contextIndex, index);
            break;
        case '+':
            UI_FeederSelection_GameVariantTarget(contextIndex, index);
            break;
        default:
            return;
    }
}

void __cdecl UI_FeederSelection_GameVariantParam(int localClientNum, int contextIndex, int index)
{
    bool v3; // [esp+0h] [ebp-14h]
    GVGlob *rule; // [esp+4h] [ebp-10h]
    char *paramText; // [esp+10h] [ebp-4h]

    paramText = UI_FeederItemText_GameVariantParams(index, 0);
    if ( GetActionByDvarIndex("ui_gv_action_index")->m_parameterType == 4 )
    {
        paramText = (char *)Dvar_GetString("ui_gv_param");
        v3 = paramText && *paramText;
        if ( !Dvar_GetInt("ui_gv_iscreatingnew") && !v3 )
        {
            rule = GetSelectedRule(localClientNum);
            if ( rule )
            {
                if ( rule->rules[0].parameter.hasParameter )
                    paramText = rule->rules[0].parameter.value;
            }
        }
    }
    Dvar_SetStringByName("ui_gv_param", paramText);
    Dvar_SetIntByName("ui_gv_param_index", index);
    Dvar_SetStringByName("ui_gv_nextpopup", (char *)"popup_gv_select_target");
}

void __cdecl UI_FeederSelection_GameVariantCondLhs(int localClientNum, int contextIndex, int index)
{
    char *v3; // eax
    GVConditionalLhs *lhs; // [esp+0h] [ebp-8h]
    uiInfo_s *dc; // [esp+4h] [ebp-4h]

    v3 = UI_FeederItemText_GameVariantCondLhs(index, 0);
    Dvar_SetStringByName("ui_gv_condlhs", v3);
    Dvar_SetIntByName("ui_gv_condlhs_index", index);
    lhs = GetCondLhsForFeederIndex(index);
    if ( lhs->m_rhs.m_values == classParamValues )
        lhs->m_rhs.m_valuesSize = UI_GameVariants_GetClassParameters();
    dc = UI_UIContext_GetInfo(contextIndex);
    Menu_SetFeederSelection(localClientNum, &dc->uiDC, 0, 37, 0, "popup_gv_select_cond");
}

void __cdecl UI_GameVariants_ArchiveRule(MemoryFile *memFile, GVRule *rule)
{
    unsigned __int8 hasConditional; // [esp+Fh] [ebp-1h] BYREF

    MemFile_ArchiveByte(memFile, &rule->actionID);
    MemFile_ArchiveByte(memFile, &rule->eventID);
    MemFile_ArchiveByte(memFile, &rule->targetID);
    UI_GameVariants_ArchiveParameter(memFile, &rule->parameter);
    UI_GameVariants_ArchiveParameter(memFile, &rule->secondParameter);
    hasConditional = rule->m_conditional.hasConditional;
    MemFile_ArchiveByte(memFile, &hasConditional);
    rule->m_conditional.hasConditional = hasConditional != 0;
    if ( rule->m_conditional.hasConditional )
    {
        MemFile_ArchiveByte(memFile, &rule->m_conditional.m_lhsIndex);
        MemFile_ArchiveByte(memFile, &rule->m_conditional.m_operatorIndex);
        MemFile_ArchiveByte(memFile, &rule->m_conditional.m_rhsIndex);
    }
}

void __cdecl MemFile_ArchiveByte(MemoryFile *memFile, unsigned __int8 *value)
{
    unsigned __int8 p; // [esp+2h] [ebp-2h] BYREF
    unsigned __int8 v3; // [esp+3h] [ebp-1h]

    if ( memFile->is_writing )
    {
        v3 = *value;
        if ( (unsigned int)(memFile->cacheBufferUsed + 1) >= 0x7FF8 )
            MemFile_WriteDataFlushInternal(memFile);
        memFile->cacheBuffer[memFile->cacheBufferUsed++] = v3;
    }
    else
    {
        MemFile_ReadData(memFile, 1, &p);
        *value = p;
    }
}

void __cdecl UI_GameVariants_ArchiveParameter(MemoryFile *memFile, GVParameter *param)
{
    char *paramValue; // [esp+18h] [ebp-8h]
    unsigned __int8 hasParameter; // [esp+1Fh] [ebp-1h] BYREF

    hasParameter = param->hasParameter;
    MemFile_ArchiveByte(memFile, &hasParameter);
    param->hasParameter = hasParameter != 0;
    if ( param->hasParameter )
    {
        if ( memFile->is_writing )
        {
            MemFile_WriteCString(memFile, param->value);
        }
        else
        {
            paramValue = MemFile_ReadCString(memFile);
            I_strncpyz(param->value, paramValue, strlen(paramValue) + 1);
        }
    }
    else
    {
        param->value[0] = 0;
    }
}

unsigned int __cdecl UI_GameVariants_GetRuleSize(const GVRule *rule)
{
    unsigned int size; // [esp+20h] [ebp-4h]
    unsigned int sizea; // [esp+20h] [ebp-4h]
    unsigned int sizeb; // [esp+20h] [ebp-4h]

    size = 4;
    if ( rule->parameter.hasParameter )
        size = strlen(rule->parameter.value) + 5;
    sizea = size + 1;
    if ( rule->secondParameter.hasParameter )
        sizea += strlen(rule->secondParameter.value) + 1;
    sizeb = sizea + 1;
    if ( rule->m_conditional.hasConditional )
        sizeb += 3;
    return sizeb;
}

void __cdecl UI_GameVariants_WriteRuleToMemoryFile(MemoryFile *memFile, GVRule *rule)
{
    unsigned int RuleSize; // eax

    if ( (unsigned int)(memFile->cacheBufferUsed + 1) >= 0x7FF8 )
        MemFile_WriteDataFlushInternal(memFile);
    memFile->cacheBuffer[memFile->cacheBufferUsed++] = 2;
    MemFile_WriteShort(memFile, 2);
    RuleSize = UI_GameVariants_GetRuleSize(rule);
    MemFile_WriteUInt(memFile, RuleSize);
    UI_GameVariants_ArchiveRule(memFile, rule);
}

void __cdecl MemFile_WriteUInt(MemoryFile *memFile, unsigned int value)
{
    if ( (unsigned int)(memFile->cacheBufferUsed + 4) >= 0x7FF8 )
        MemFile_WriteDataFlushInternal(memFile);
    *(unsigned int *)&memFile->cacheBuffer[memFile->cacheBufferUsed] = value;
    memFile->cacheBufferUsed += 4;
}

void __cdecl UI_GameVariants_WriteToMemoryFile(MemoryFile *memFile)
{
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < UI_GetGVRulesCount(); ++i )
    {
        if ( gvGlob.rules[i].isValid )
            UI_GameVariants_WriteRuleToMemoryFile(memFile, &gvGlob.rules[i]);
    }
}

void __cdecl UI_GameVariants_Reset()
{
    int i; // [esp+0h] [ebp-4h]

    gvGlob.ruleCount = 0;
    for ( i = 0; i < 64; ++i )
        gvGlob.rules[i].isValid = 0;
    Dvar_SetInt((dvar_s *)ui_gv_rulecount, gvGlob.ruleCount);
}

char __cdecl UI_GameVariants_ReadGameVariantRuleChunk(MemoryFile *memFile)
{
    unsigned int RuleSize; // eax
    unsigned int v3; // [esp+0h] [ebp-18h] BYREF
    unsigned __int8 p[4]; // [esp+4h] [ebp-14h] BYREF
    void *dataBuffer; // [esp+8h] [ebp-10h]
    GVRule *rule; // [esp+Ch] [ebp-Ch]
    int version; // [esp+10h] [ebp-8h]
    unsigned int size; // [esp+14h] [ebp-4h]

    MemFile_ReadData(memFile, 2, p);
    version = *(__int16 *)p;
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&v3);
    size = v3;
    if ( version == 2 )
    {
        rule = &gvGlob.rules[gvGlob.ruleCount++];
        Dvar_SetInt((dvar_s *)ui_gv_rulecount, gvGlob.ruleCount);
        rule->isValid = 1;
        UI_GameVariants_ArchiveRule(memFile, rule);
        RuleSize = UI_GameVariants_GetRuleSize(rule);
        if ( size != RuleSize )
            Com_Error(ERR_DROP, "PLATFORM_ERR_SAVEGAME_BAD");
        return 1;
    }
    else
    {
        Com_PrintWarning(14, "Invalid custom game mode rule chunk version.    Expected: %i Got: %i\n", 2, version);
        dataBuffer = Z_VirtualAlloc(size, "gv_rules", 0);
        MemFile_ReadData(memFile, size, (unsigned __int8 *)dataBuffer);
        Z_VirtualFree(dataBuffer, 0);
        return 1;
    }
}

void __cdecl Scr_GameVariants_AddConditional(GVRule *rule)
{
    int value; // eax
    float v2; // [esp+8h] [ebp-18h]
    bool v3; // [esp+Ch] [ebp-14h]
    GVConditionalLhs *lhs; // [esp+18h] [ebp-8h]
    char *rhsValue; // [esp+1Ch] [ebp-4h]

    if ( rule->m_conditional.hasConditional )
    {
        lhs = &gvCondLhsValues[rule->m_conditional.m_lhsIndex];
        Scr_AddString((char *)lhs->m_scriptName, SCRIPTINSTANCE_SERVER);
        Scr_AddArrayStringIndexed(scr_const.gv_condlhs, SCRIPTINSTANCE_SERVER);
        Scr_AddString((char *)gvCondOpValues[rule->m_conditional.m_operatorIndex].m_value, SCRIPTINSTANCE_SERVER);
        Scr_AddArrayStringIndexed(scr_const.gv_condop, SCRIPTINSTANCE_SERVER);
        rhsValue = (char *)lhs->m_rhs.m_values[rule->m_conditional.m_rhsIndex].m_value;
        switch ( lhs->m_rhs.m_type )
        {
            case GVTYPE_BOOL:
                v3 = !I_stricmp(rhsValue, "false") || !I_stricmp(rhsValue, "0");
                Scr_AddBool(!v3, SCRIPTINSTANCE_SERVER);
                break;
            case GVTYPE_INT:
                value = atoi(rhsValue);
                Scr_AddInt(value, SCRIPTINSTANCE_SERVER);
                break;
            case GVTYPE_FLOAT:
                v2 = atof(rhsValue);
                Scr_AddFloat(v2, SCRIPTINSTANCE_SERVER);
                break;
            case GVTYPE_STRING:
                Scr_AddString(rhsValue, SCRIPTINSTANCE_SERVER);
                break;
            default:
                break;
        }
        Scr_AddArrayStringIndexed(scr_const.gv_condrhs, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl Scr_GameVariants_AddParams(GVRule *rule)
{
    int value; // eax
    float v2; // [esp+8h] [ebp-Ch]
    const GVAction *action; // [esp+10h] [ebp-4h]

    action = &gvActions[rule->actionID];
    if ( action->m_parameterType )
    {
        Scr_MakeArray(SCRIPTINSTANCE_SERVER);
        switch ( action->m_parameterType )
        {
            case 1u:
                value = atoi(rule->parameter.value);
                Scr_AddInt(value, SCRIPTINSTANCE_SERVER);
                break;
            case 2u:
                v2 = atof(rule->parameter.value);
                Scr_AddFloat(v2, SCRIPTINSTANCE_SERVER);
                break;
            case 3u:
            case 4u:
            case 5u:
            case 7u:
                Scr_AddString(rule->parameter.value, SCRIPTINSTANCE_SERVER);
                break;
            case 6u:
                Scr_GameVariants_AddPerkString(rule->parameter.value);
                break;
            default:
                break;
        }
        Scr_AddArray(SCRIPTINSTANCE_SERVER);
        if ( rule->secondParameter.hasParameter )
        {
            Scr_AddString(rule->secondParameter.value, SCRIPTINSTANCE_SERVER);
            Scr_AddArray(SCRIPTINSTANCE_SERVER);
        }
        Scr_AddArrayStringIndexed(scr_const.gv_parameter, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl Scr_GameVariants_AddPerkString(char *paramValue)
{
    const char *v1; // eax
    const char *v2; // eax
    char specialty[132]; // [esp+0h] [ebp-90h] BYREF
    const char *cursor; // [esp+88h] [ebp-8h]
    const char *pipeDelim; // [esp+8Ch] [ebp-4h]

    cursor = paramValue;
    v1 = strstr(paramValue, "|");
    for ( pipeDelim = v1; pipeDelim; pipeDelim = v2 )
    {
        I_strncpyz(specialty, cursor, pipeDelim - cursor + 1);
        Scr_AddString(specialty, SCRIPTINSTANCE_SERVER);
        Scr_AddArray(SCRIPTINSTANCE_SERVER);
        cursor = pipeDelim + 1;
        v2 = strstr(pipeDelim + 1, "|");
    }
    Scr_AddString((char *)cursor, SCRIPTINSTANCE_SERVER);
}

void __cdecl Scr_GameVariants_GetRule()
{
    GVRule *rule; // [esp+0h] [ebp-8h]
    int ruleNum; // [esp+4h] [ebp-4h]

    ruleNum = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if (ruleNum >= 0 && ruleNum <= gvGlob.ruleCount)
    {
        rule = &gvGlob.rules[ruleNum];
        Scr_MakeArray(SCRIPTINSTANCE_SERVER);
        Scr_AddString(gvEvents[rule->eventID].m_scriptName, SCRIPTINSTANCE_SERVER);
        Scr_AddArrayStringIndexed(scr_const.gv_event, SCRIPTINSTANCE_SERVER);
        Scr_AddString(gvActions[rule->actionID].m_scriptName, SCRIPTINSTANCE_SERVER);
        Scr_AddArrayStringIndexed(scr_const.gv_action, SCRIPTINSTANCE_SERVER);
        Scr_AddString(gvTargets[rule->targetID].m_scriptName, SCRIPTINSTANCE_SERVER);
        Scr_AddArrayStringIndexed(scr_const.gv_target, SCRIPTINSTANCE_SERVER);
        Scr_GameVariants_AddConditional(rule);
        Scr_GameVariants_AddParams(rule);
    }
    else
    {
        Com_PrintError(24, "ERROR: Invalid Game Variant Rule Number!");
    }
}

GVGlob::GVGlob()
{
    int v2; // [esp+4h] [ebp-Ch]
    GVGlob *i; // [esp+8h] [ebp-8h]

    v2 = 64;
    for ( i = this; --v2 >= 0; i = (GVGlob *)((char *)i + 266) )
    {
        i->rules[0].m_conditional.hasConditional = 0;
        i->rules[0].m_conditional.m_lhsIndex = 0;
        i->rules[0].m_conditional.m_operatorIndex = 0;
        i->rules[0].m_conditional.m_rhsIndex = 0;
    }
}

GVAction::GVAction(
                const char *name,
                const char *scriptName,
                unsigned __int8 parameterType,
                GVValue *parameters,
                int parametersSize)
{
    this->m_name = name;
    this->m_scriptName = scriptName;
    this->m_parameterType = parameterType;
    this->m_parameters = parameters;
    this->m_parametersSize = parametersSize;
}

