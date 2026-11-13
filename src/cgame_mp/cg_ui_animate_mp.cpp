#include "cg_ui_animate_mp.h"
#include <universal/q_shared.h>
#include <ui/ui_shared.h>
#include <ui/ui_main.h>
#include <ui/ui_localvars.h>
#include <client_mp/cl_main_mp.h>
#include "cg_main_mp.h"
#include <cgame/cg_sound.h>
#include <demo/demo_playback.h>
#include <client/cl_rank.h>
#include <cgame/cg_draw_names.h>
#include <universal/offline.h>
#include <client/cl_milestone.h>
#include <stringed/stringed_hooks.h>
#include <client_mp/cl_cgame_mp.h>
#include <client/cl_medal.h>

void __cdecl UI_AnimateLocalItem(
                int localClientNum,
                UiContext *dc,
                const char *menuName,
                const char *itemName,
                const char *newStateName,
                int milliseconds)
{
    char menuNameLocal[132]; // [esp+0h] [ebp-88h] BYREF

    Com_sprintf(menuNameLocal, 0x80u, "%s%i", menuName, localClientNum);
    UI_AnimateItem(localClientNum, dc, menuNameLocal, itemName, newStateName, milliseconds);
}

void __cdecl UI_SetLocalVarStringByName(char *inputName, char *inputValue, int localClientNum)
{
    UILocalVarContext *var; // [esp+0h] [ebp-8h]
    uiInfo_s *uiInfo; // [esp+4h] [ebp-4h]

    if ( localClientNum >= 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                    44,
                    0,
                    "%s",
                    "localClientNum < MAX_LOCAL_CLIENTS && MAX_LOCAL_CLIENTS >= 0") )
    {
        __debugbreak();
    }
    if ( !localClientNum )
    {
        uiInfo = UI_GetInfo(0);
        if ( !inputName
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                        51,
                        0,
                        "%s",
                        "inputName != NULL") )
        {
            __debugbreak();
        }
        if ( inputName )
        {
            if ( !inputValue
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                            55,
                            0,
                            "%s",
                            "inputValue != NULL") )
            {
                __debugbreak();
            }
            if ( inputValue )
            {
                var = UILocalVar_FindOrCreate(&uiInfo->uiDC.localVars, inputName);
                if ( !var
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp", 61, 0, "%s", "var") )
                {
                    __debugbreak();
                }
                UILocalVar_SetString(var->table, inputValue);
            }
        }
    }
}

void __cdecl UI_SetFakeLocalDvarStringByName(const char *inputName, char *inputValue, unsigned int localClientNum)
{
    char dvarName[132]; // [esp+0h] [ebp-88h] BYREF

    if ( localClientNum < 2 )
    {
        if ( !inputName
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                        70,
                        0,
                        "%s",
                        "inputName != NULL") )
        {
            __debugbreak();
        }
        if ( inputName )
        {
            if ( !inputValue
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                            74,
                            0,
                            "%s",
                            "inputValue != NULL") )
            {
                __debugbreak();
            }
            if ( inputValue )
            {
                Com_sprintf(dvarName, 0x80u, "%s%i", inputName, localClientNum);
                Dvar_SetStringByName(dvarName, inputValue);
            }
        }
    }
}

void __cdecl CG_ClearPopUpUI(int localClientNum)
{
    CG_ClearCenterPopUpUI(localClientNum);
    CG_ClearPlayerCardPopUpUI(localClientNum);
    CG_TeamMessagePopUpUI(localClientNum);
    CG_GameMessagePopUpUI(localClientNum);
}

void __cdecl CG_ClearCenterPopUpUI(int localClientNum)
{
    UI_AnimateLocalItem(localClientNum, &cgDC[localClientNum], "InGamePopUps", "KillstreakPopUp", "Hide", 0);
    UI_AnimateLocalItem(localClientNum, &cgDC[localClientNum], "InGamePopUps", "RankPopUp", "Hide", 0);
    UI_AnimateLocalItem(localClientNum, &cgDC[localClientNum], "InGamePopUps", "ChallengePopUp", "Hide", 0);
    UI_AnimateLocalItem(localClientNum, &cgDC[localClientNum], "InGamePopUps", "MedalPopUp", "Hide", 0);
    UI_AnimateLocalItem(localClientNum, &cgDC[localClientNum], "InGamePopUps", "ContractPopUp", "Hide", 0);
    UI_AnimateLocalItem(localClientNum, &cgDC[localClientNum], "InGamePopUps", "WagerPopUp", "Hide", 0);
}

void __cdecl CG_ClearPlayerCardPopUpUI(int localClientNum)
{
    UI_AnimateLocalItem(localClientNum, &cgDC[localClientNum], "playerKilled", "playerCardPopUp", "Hide", 0);
    UI_AnimateLocalItem(localClientNum, &cgDC[localClientNum], "playerCard", "playerCardPopUp", "Hide", 0);
}

void __cdecl CG_TeamMessagePopUpUI(int localClientNum)
{
    UI_AnimateLocalItem(localClientNum, &cgDC[localClientNum], "teamMessageSlideOut", "SlideOut", "Hide", 0);
}

void __cdecl CG_GameMessagePopUpUI(int localClientNum)
{
    UI_AnimateLocalItem(localClientNum, &cgDC[localClientNum], "GameMessageSlideOut", "SlideOut", "Hide", 0);
}

void __cdecl CG_DisplayCenterPopup(
                int localClientNum,
                const char *animationType,
                char *displayString1,
                char *displayString2,
                char *displayString3,
                char *displayMaterial,
                char *displayBacking)
{
    if ( (!displayString1 || !displayString2)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                    121,
                    0,
                    "%s",
                    "displayString1 != NULL && displayString2 != NULL") )
    {
        __debugbreak();
    }
    if ( displayString1 && displayString2 )
    {
        if ( !animationType
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                        125,
                        0,
                        "%s",
                        "animationType != NULL") )
        {
            __debugbreak();
        }
        if ( animationType )
        {
            UI_SetLocalVarStringByName("popUpCenterStringA", displayString1, localClientNum);
            UI_SetLocalVarStringByName("popUpCenterStringB", displayString2, localClientNum);
            if ( displayString3 )
                UI_SetLocalVarStringByName("popUpCenterStringC", displayString3, localClientNum);
            if ( displayMaterial )
                UI_SetLocalVarStringByName("popUpCenterMaterial", displayMaterial, localClientNum);
            if ( displayBacking )
                UI_SetLocalVarStringByName("popUpCenterMaterialBacking", displayBacking, localClientNum);
            CG_ClearCenterPopUpUI(localClientNum);
            UI_AnimateLocalItem(localClientNum, &cgDC[localClientNum], "InGamePopUps", animationType, "Start", 0);
        }
    }
}

void __cdecl CG_PlayPopUpSound(int localClientNum, char *sound)
{
    int ClientNumForLocalClient; // eax

    if ( !sound
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                    149,
                    0,
                    "%s",
                    "sound != NULL") )
    {
        __debugbreak();
    }
    if ( sound )
    {
        ClientNumForLocalClient = CG_GetClientNumForLocalClient(localClientNum);
        CG_PlaySound(localClientNum, ClientNumForLocalClient, 0, 0, 0, 1.0, sound);
    }
}

void __cdecl CG_DisplayRankUp(int localClientNum)
{
    const char *v1; // eax
    char *localizedPromoted; // [esp+18h] [ebp-24h]
    char *rankImage; // [esp+1Ch] [ebp-20h]
    int prestige; // [esp+20h] [ebp-1Ch]
    const char *rank; // [esp+24h] [ebp-18h]
    cg_s *cgameGlob; // [esp+28h] [ebp-14h]
    const char *cpEarned; // [esp+2Ch] [ebp-10h]
    char *sound; // [esp+30h] [ebp-Ch]
    int rankIndex; // [esp+34h] [ebp-8h]
    char *localizedCpEarned; // [esp+38h] [ebp-4h]
    char *localizedCpEarneda; // [esp+38h] [ebp-4h]

    localizedPromoted = UI_SafeTranslateString("RANK_PROMOTED_TO");
    localizedCpEarned = UI_SafeTranslateString("RANK_COD_POINTS");
    rank = Cmd_Argv(2);
    v1 = Cmd_Argv(3);
    prestige = atoi(v1);
    sound = (char *)Cmd_Argv(4);
    if ( !Demo_IsThirdPersonCamera() && !Demo_IsMovieCamera() )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        cgameGlob->popUpSwayStartTime = cgameGlob->time;
        cgameGlob->popupRotationAngle = 7.0f;
        rankIndex = atoi(rank) - 1;
        rankImage = (char *)CL_GetRankIconName(rankIndex, prestige);
        if ( !rankImage
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                        178,
                        0,
                        "%s",
                        "rankImage != NULL") )
        {
            __debugbreak();
        }
        if ( rankImage )
        {
            UI_SetFakeLocalDvarStringByName("rankUpPopUpIcon", rankImage, localClientNum);
            cpEarned = CL_GetRankData(rankIndex, MP_RANKTABLE_CP);
            if ( !cpEarned
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                            185,
                            0,
                            "%s",
                            "cpEarned != NULL") )
            {
                __debugbreak();
            }
            if ( cpEarned )
            {
                localizedCpEarneda = UI_ReplaceConversionString(localizedCpEarned, cpEarned);
                if ( atoi(rank) < 999 && atoi(rank) >= 0 )
                    localizedPromoted = UI_ReplaceConversionString(localizedPromoted, rank);
                CG_DisplayCenterPopup(localClientNum, "RankPopUp", localizedPromoted, localizedCpEarneda, 0, rankImage, 0);
                CG_PlayPopUpSound(localClientNum, sound);
            }
        }
    }
}

void __cdecl CG_DisplayMilestoneComplete(int localClientNum)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    int v4; // eax
    char nameLocalized; // [esp+3Eh] [ebp-45Eh]
    char descLocalized; // [esp+3Fh] [ebp-45Dh]
    const char *milestoneIndex; // [esp+40h] [ebp-45Ch]
    char localizedMilestoneXP[32]; // [esp+44h] [ebp-458h] BYREF
    char backingMaterialName[256]; // [esp+64h] [ebp-438h] BYREF
    statsMilestoneTypes_t milestoneType; // [esp+164h] [ebp-338h]
    const char *perkName; // [esp+168h] [ebp-334h]
    cg_s *cgameGlob; // [esp+16Ch] [ebp-330h]
    int statIndex; // [esp+170h] [ebp-32Ch]
    const char *statName; // [esp+174h] [ebp-328h]
    const char *targetValue; // [esp+178h] [ebp-324h]
    const char *weaponType; // [esp+17Ch] [ebp-320h]
    const char *sound; // [esp+180h] [ebp-31Ch]
    char materialName[256]; // [esp+184h] [ebp-318h] BYREF
    int milestoneTier; // [esp+284h] [ebp-218h]
    float xpScale; // [esp+288h] [ebp-214h]
    char milestoneLocalizedDesc[260]; // [esp+28Ch] [ebp-210h] BYREF
    const char *statType; // [esp+390h] [ebp-10Ch]
    char milestoneLocalizedName[260]; // [esp+394h] [ebp-108h] BYREF

    v1 = Cmd_Argv(2);
    milestoneTier = atoi(v1);
    milestoneIndex = Cmd_Argv(3);
    v2 = Cmd_Argv(4);
    xpScale = atof(v2);
    v3 = Cmd_Argv(5);
    statIndex = atoi(v3);
    weaponType = Cmd_Argv(6);
    sound = Cmd_Argv(7);
    if ( !Demo_IsThirdPersonCamera() && !Demo_IsMovieCamera() )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        cgameGlob->typewriterStartTime = cgameGlob->time;
        if ( !milestoneIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                        224,
                        0,
                        "%s",
                        "milestoneIndex != NULL") )
        {
            __debugbreak();
        }
        if ( milestoneIndex )
        {
            statName = CL_GetMilestoneData(milestoneIndex, MILESTONE_COLUMN_STATNAME);
            if ( !statName
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                            229,
                            0,
                            "%s",
                            "statName != NULL") )
            {
                __debugbreak();
            }
            if ( statName )
            {
                targetValue = CL_GetMilestoneData(milestoneIndex, MILESTONE_COLUMN_TARGETVALUE);
                if ( !targetValue
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                234,
                                0,
                                "%s",
                                "targetValue != NULL") )
                {
                    __debugbreak();
                }
                if ( targetValue )
                {
                    statType = CL_GetMilestoneData(milestoneIndex, MILESTONE_COLUMN_STATTYPE);
                    if ( !statType
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                    239,
                                    0,
                                    "%s",
                                    "statType != NULL") )
                    {
                        __debugbreak();
                    }
                    if ( statType )
                    {
                        perkName = CL_GetMilestoneData(milestoneIndex, MILESTONE_COLUMN_PERKNAME);
                        if ( !perkName
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                        244,
                                        0,
                                        "%s",
                                        "perkName != NULL") )
                        {
                            __debugbreak();
                        }
                        if ( perkName )
                        {
                            if ( !weaponType
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                            248,
                                            0,
                                            "%s",
                                            "weaponType != NULL") )
                            {
                                __debugbreak();
                            }
                            if ( weaponType )
                            {
                                if ( !sound
                                    && !Assert_MyHandler(
                                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                                252,
                                                0,
                                                "%s",
                                                "sound != NULL") )
                                {
                                    __debugbreak();
                                }
                                if ( sound )
                                {
                                    milestoneType = getMilestoneType(statType);
                                    if ( milestoneType == MILESTONE_INVALID
                                        && !Assert_MyHandler(
                                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                                    257,
                                                    0,
                                                    "%s",
                                                    "milestoneType != MILESTONE_INVALID") )
                                    {
                                        __debugbreak();
                                    }
                                    if ( milestoneType != MILESTONE_INVALID )
                                    {
                                        CL_GetMilestoneLocalizedXP(localizedMilestoneXP, 32, xpScale, milestoneTier, milestoneIndex);
                                        nameLocalized = CL_GetInGameMilestoneLocalizedName(
                                                                            localClientNum,
                                                                            milestoneLocalizedName,
                                                                            256,
                                                                            statName,
                                                                            milestoneType,
                                                                            (eAttachment)statIndex,
                                                                            milestoneTier,
                                                                            weaponType);
                                        v4 = atoi(targetValue);
                                        descLocalized = CL_GetInGameMilestoneLocalizedDesc(
                                                                            localClientNum,
                                                                            milestoneLocalizedDesc,
                                                                            256,
                                                                            statName,
                                                                            milestoneType,
                                                                            statIndex,
                                                                            milestoneTier,
                                                                            weaponType,
                                                                            v4);
                                        if ( !nameLocalized
                                            && !Assert_MyHandler(
                                                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                                        264,
                                                        0,
                                                        "%s",
                                                        "nameLocalized") )
                                        {
                                            __debugbreak();
                                        }
                                        if ( !descLocalized
                                            && !Assert_MyHandler(
                                                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                                        265,
                                                        0,
                                                        "%s",
                                                        "descLocalized") )
                                        {
                                            __debugbreak();
                                        }
                                        if ( nameLocalized )
                                        {
                                            if ( descLocalized )
                                            {
                                                CL_GetMilestoneMaterial(materialName, 256, statName, perkName);
                                                CL_GetMilestoneBackingMaterial(backingMaterialName, 256, statName, perkName);
                                                if ( backingMaterialName )
                                                {
                                                    if ( materialName )
                                                    {
                                                        CG_DisplayCenterPopup(
                                                            localClientNum,
                                                            "ChallengePopUp",
                                                            milestoneLocalizedName,
                                                            milestoneLocalizedDesc,
                                                            localizedMilestoneXP,
                                                            materialName,
                                                            backingMaterialName);
                                                        CG_PlayPopUpSound(localClientNum, (char *)sound);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void __cdecl CG_SetupEndGame(int localClientNum)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    UILocalVarContext *var; // [esp+20h] [ebp-18h]
    UILocalVarContext *vara; // [esp+20h] [ebp-18h]
    UILocalVarContext *varb; // [esp+20h] [ebp-18h]
    UILocalVarContext *varc; // [esp+20h] [ebp-18h]
    int contract2; // [esp+24h] [ebp-14h]
    uiInfo_s *uiInfo; // [esp+28h] [ebp-10h]
    int contract0; // [esp+2Ch] [ebp-Ch]
    int promoted; // [esp+30h] [ebp-8h]
    int contract1; // [esp+34h] [ebp-4h]

    uiInfo = UI_GetInfo(localClientNum);
    v1 = Cmd_Argv(2);
    promoted = atoi(v1);
    v2 = Cmd_Argv(3);
    contract0 = atoi(v2);
    v3 = Cmd_Argv(4);
    contract1 = atoi(v3);
    v4 = Cmd_Argv(5);
    contract2 = atoi(v4);
    var = UILocalVar_FindOrCreate(&uiInfo->uiDC.localVars, "promoted");
    if ( !var
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp", 296, 0, "%s", "var") )
    {
        __debugbreak();
    }
    UILocalVar_SetInt(var->table, promoted);
    vara = UILocalVar_FindOrCreate(&uiInfo->uiDC.localVars, "contract0");
    if ( !vara
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp", 299, 0, "%s", "var") )
    {
        __debugbreak();
    }
    UILocalVar_SetInt(vara->table, contract0);
    varb = UILocalVar_FindOrCreate(&uiInfo->uiDC.localVars, "contract1");
    if ( !varb
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp", 302, 0, "%s", "var") )
    {
        __debugbreak();
    }
    UILocalVar_SetInt(varb->table, contract1);
    varc = UILocalVar_FindOrCreate(&uiInfo->uiDC.localVars, "contract2");
    if ( !varc
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp", 305, 0, "%s", "var") )
    {
        __debugbreak();
    }
    UILocalVar_SetInt(varc->table, contract2);
}

void __cdecl CG_SetupMilestoneEndGame(int localClientNum)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    int v4; // eax
    char nameLocalized; // [esp+32h] [ebp-442h]
    char descLocalized; // [esp+33h] [ebp-441h]
    char localVarDescString[260]; // [esp+34h] [ebp-440h] BYREF
    UILocalVar *var; // [esp+138h] [ebp-33Ch]
    const char *milestoneIndex; // [esp+13Ch] [ebp-338h]
    statsMilestoneTypes_t milestoneType; // [esp+140h] [ebp-334h]
    uiInfo_s *uiInfo; // [esp+144h] [ebp-330h]
    int slot; // [esp+148h] [ebp-32Ch]
    int statIndex; // [esp+14Ch] [ebp-328h]
    const char *statName; // [esp+150h] [ebp-324h]
    const char *targetValue; // [esp+154h] [ebp-320h]
    const char *weaponType; // [esp+158h] [ebp-31Ch]
    char milestoneLocalizedDesc[260]; // [esp+15Ch] [ebp-318h] BYREF
    int milestoneTier; // [esp+260h] [ebp-214h]
    int argCount; // [esp+264h] [ebp-210h]
    const char *statType; // [esp+268h] [ebp-20Ch]
    char localVarNameString[256]; // [esp+26Ch] [ebp-208h] BYREF
    char milestoneLocalizedName[260]; // [esp+36Ch] [ebp-108h] BYREF

    uiInfo = UI_GetInfo(localClientNum);
    memset(localVarNameString, 0, sizeof(localVarNameString));
    memset(localVarDescString, 0, 256);
    memset(milestoneLocalizedName, 0, 256);
    memset(milestoneLocalizedDesc, 0, 256);
    v1 = Cmd_Argv(2);
    slot = atoi(v1);
    v2 = Cmd_Argv(3);
    milestoneTier = atoi(v2);
    milestoneIndex = Cmd_Argv(4);
    v3 = Cmd_Argv(5);
    statIndex = atoi(v3);
    weaponType = Cmd_Argv(6);
    argCount = Cmd_Argc();
    if ( argCount != 7
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                    330,
                    0,
                    "%s",
                    "argCount == 7") )
    {
        __debugbreak();
    }
    if ( argCount == 7 )
    {
        statName = CL_GetMilestoneData(milestoneIndex, MILESTONE_COLUMN_STATNAME);
        if ( !statName
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                        335,
                        0,
                        "%s",
                        "statName != NULL") )
        {
            __debugbreak();
        }
        if ( statName )
        {
            targetValue = CL_GetMilestoneData(milestoneIndex, MILESTONE_COLUMN_TARGETVALUE);
            if ( !targetValue
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                            340,
                            0,
                            "%s",
                            "targetValue != NULL") )
            {
                __debugbreak();
            }
            if ( targetValue )
            {
                statType = CL_GetMilestoneData(milestoneIndex, MILESTONE_COLUMN_STATTYPE);
                if ( !statType
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                345,
                                0,
                                "%s",
                                "statType != NULL") )
                {
                    __debugbreak();
                }
                if ( statType )
                {
                    if ( !weaponType
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                    349,
                                    0,
                                    "%s",
                                    "weaponType != NULL") )
                    {
                        __debugbreak();
                    }
                    if ( weaponType )
                    {
                        if ( (unsigned int)slot >= 4
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                        353,
                                        0,
                                        "%s",
                                        "slot < ENDGAME_MILESTONE_SLOT_COUNT && slot >=0") )
                        {
                            __debugbreak();
                        }
                        if ( (unsigned int)slot < 4 )
                        {
                            Com_sprintf(localVarNameString, 0x100u, "milestoneEndGameNameString%i", slot);
                            Com_sprintf(localVarDescString, 0x100u, "milestoneEndGameDescString%i", slot);
                            milestoneType = getMilestoneType(statType);
                            if ( milestoneType == MILESTONE_INVALID
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                            361,
                                            0,
                                            "%s",
                                            "milestoneType != MILESTONE_INVALID") )
                            {
                                __debugbreak();
                            }
                            if ( milestoneType != MILESTONE_INVALID )
                            {
                                nameLocalized = CL_GetInGameMilestoneLocalizedName(
                                                                    localClientNum,
                                                                    milestoneLocalizedName,
                                                                    256,
                                                                    statName,
                                                                    milestoneType,
                                                                    (eAttachment)statIndex,
                                                                    milestoneTier,
                                                                    weaponType);
                                v4 = atoi(targetValue);
                                descLocalized = CL_GetInGameMilestoneLocalizedDesc(
                                                                    localClientNum,
                                                                    milestoneLocalizedDesc,
                                                                    256,
                                                                    statName,
                                                                    milestoneType,
                                                                    statIndex,
                                                                    milestoneTier,
                                                                    weaponType,
                                                                    v4);
                                if ( !nameLocalized
                                    && !Assert_MyHandler(
                                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                                366,
                                                0,
                                                "%s",
                                                "nameLocalized") )
                                {
                                    __debugbreak();
                                }
                                if ( !descLocalized
                                    && !Assert_MyHandler(
                                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                                367,
                                                0,
                                                "%s",
                                                "descLocalized") )
                                {
                                    __debugbreak();
                                }
                                if ( nameLocalized && descLocalized )
                                {
                                    var = (UILocalVar *)UILocalVar_FindOrCreate(&uiInfo->uiDC.localVars, localVarNameString);
                                    if ( !var
                                        && !Assert_MyHandler(
                                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                                    372,
                                                    0,
                                                    "%s",
                                                    "var") )
                                    {
                                        __debugbreak();
                                    }
                                    if ( var )
                                    {
                                        UILocalVar_SetString(var, milestoneLocalizedName);
                                        var = (UILocalVar *)UILocalVar_FindOrCreate(&uiInfo->uiDC.localVars, localVarDescString);
                                        if ( !var
                                            && !Assert_MyHandler(
                                                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                                        378,
                                                        0,
                                                        "%s",
                                                        "var") )
                                        {
                                            __debugbreak();
                                        }
                                        if ( var )
                                            UILocalVar_SetString(var, milestoneLocalizedDesc);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void __cdecl CG_ClearEndGame(int localClientNum)
{
    UILocalVarContext *var; // [esp+0h] [ebp-8h]
    UILocalVarContext *vara; // [esp+0h] [ebp-8h]
    UILocalVarContext *varb; // [esp+0h] [ebp-8h]
    UILocalVarContext *varc; // [esp+0h] [ebp-8h]
    UILocalVarContext *vard; // [esp+0h] [ebp-8h]
    UILocalVarContext *vare; // [esp+0h] [ebp-8h]
    UILocalVarContext *varf; // [esp+0h] [ebp-8h]
    uiInfo_s *uiInfo; // [esp+4h] [ebp-4h]

    uiInfo = UI_GetInfo(localClientNum);
    var = UILocalVar_FindOrCreate(&uiInfo->uiDC.localVars, "promoted");
    if ( !var
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp", 392, 0, "%s", "var") )
    {
        __debugbreak();
    }
    UILocalVar_SetInt(var->table, 0);
    vara = UILocalVar_FindOrCreate(&uiInfo->uiDC.localVars, "contract0");
    if ( !vara
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp", 395, 0, "%s", "var") )
    {
        __debugbreak();
    }
    UILocalVar_SetInt(vara->table, -1);
    varb = UILocalVar_FindOrCreate(&uiInfo->uiDC.localVars, "contract1");
    if ( !varb
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp", 398, 0, "%s", "var") )
    {
        __debugbreak();
    }
    UILocalVar_SetInt(varb->table, -1);
    varc = UILocalVar_FindOrCreate(&uiInfo->uiDC.localVars, "contract2");
    if ( !varc
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp", 401, 0, "%s", "var") )
    {
        __debugbreak();
    }
    UILocalVar_SetInt(varc->table, -1);
    vard = UILocalVar_FindOrCreate(&uiInfo->uiDC.localVars, "milestoneEndGameString0");
    if ( !vard
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp", 404, 0, "%s", "var") )
    {
        __debugbreak();
    }
    UILocalVar_SetString(vard->table, (char *)"");
    vare = UILocalVar_FindOrCreate(&uiInfo->uiDC.localVars, "milestoneEndGameString1");
    if ( !vare
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp", 407, 0, "%s", "var") )
    {
        __debugbreak();
    }
    UILocalVar_SetString(vare->table, (char *)"");
    varf = UILocalVar_FindOrCreate(&uiInfo->uiDC.localVars, "milestoneEndGameString2");
    if ( !varf
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp", 410, 0, "%s", "var") )
    {
        __debugbreak();
    }
    UILocalVar_SetString(varf->table, (char *)"");
}

void __cdecl CG_DisplayGameModeMessage(int localClientNum)
{
    char *sound; // [esp+10h] [ebp-Ch]
    char *localizedGameModeMessage; // [esp+14h] [ebp-8h]
    const char *gameModeMessage; // [esp+18h] [ebp-4h]

    gameModeMessage = Cmd_Argv(2);
    sound = (char *)Cmd_Argv(3);
    localizedGameModeMessage = (char *)SEH_LocalizeTextMessage(gameModeMessage, "gameModeMessage", LOCMSG_SAFE);
    if ( !localizedGameModeMessage
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                    421,
                    0,
                    "%s",
                    "localizedGameModeMessage != NULL") )
    {
        __debugbreak();
    }
    if ( localizedGameModeMessage )
    {
        UI_SetLocalVarStringByName("gameModeMessage", localizedGameModeMessage, localClientNum);
        CG_PlayPopUpSound(localClientNum, sound);
        UI_AnimateLocalItem(localClientNum, &cgDC[localClientNum], "GameMessageSlideOut", "SlideOut", "Start", 0);
    }
}

void __cdecl CG_DisplayTeamMessage(unsigned int localClientNum)
{
    const char *v1; // eax
    const char *v2; // eax
    int configStringIndex; // [esp+18h] [ebp-460h]
    cg_s *cgameGlob; // [esp+1Ch] [ebp-45Ch]
    clientInfo_t *selfClientInfo; // [esp+20h] [ebp-458h]
    char *rank; // [esp+24h] [ebp-454h]
    char *sound; // [esp+28h] [ebp-450h]
    int playerNum; // [esp+2Ch] [ebp-44Ch]
    char clanTag[44]; // [esp+34h] [ebp-444h] BYREF
    const char *localizedGameModeMessage; // [esp+60h] [ebp-418h]
    const char *teamMessageRankIcon; // [esp+64h] [ebp-414h]
    char teamMessageXuid[1028]; // [esp+68h] [ebp-410h] BYREF
    bool friendly; // [esp+473h] [ebp-5h]
    const char *teamMessage; // [esp+474h] [ebp-4h]

    sound = (char *)Cmd_Argv(4);
    v1 = Cmd_Argv(3);
    playerNum = atoi(v1);
    v2 = Cmd_Argv(2);
    configStringIndex = atoi(v2);
    memset(clanTag, 0, 42);
    memset(teamMessageXuid, 0, 1024);
    friendly = 0;
    if ( localClientNum < 2 )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        selfClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->clientNum];
        if ( cgameGlob->bgs.clientinfo[playerNum].rank >= 999
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                        454,
                        0,
                        "%s",
                        "clientInfo->rank < 999") )
        {
            __debugbreak();
        }
        if ( cgameGlob->bgs.clientinfo[playerNum].rank < 999 )
        {
            teamMessage = CL_GetConfigString(configStringIndex + 515);
            localizedGameModeMessage = SEH_LocalizeTextMessage(teamMessage, "teamMessage", LOCMSG_SAFE);
            if ( !localizedGameModeMessage
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                            460,
                            0,
                            "%s",
                            "localizedGameModeMessage != NULL") )
            {
                __debugbreak();
            }
            if ( localizedGameModeMessage )
            {
                teamMessageRankIcon = CL_GetRankIconName(
                                                                cgameGlob->bgs.clientinfo[playerNum].rank,
                                                                cgameGlob->bgs.clientinfo[playerNum].prestige);
                UI_SetLocalVarStringByName("teamMessageRankIcon", (char *)teamMessageRankIcon, localClientNum);
                rank = (char *)CL_GetRankData(cgameGlob->bgs.clientinfo[playerNum].rank, MP_RANKTABLE_DISPLAYLEVEL);
                UI_SetLocalVarStringByName("teamMessage", (char *)localizedGameModeMessage, localClientNum);
                UI_SetLocalVarStringByName("teamMessageName", cgameGlob->bgs.clientinfo[playerNum].name, localClientNum);
                UI_SetLocalVarStringByName("teamMessageRank", rank, localClientNum);
                Com_sprintf(teamMessageXuid, 0x400u, "teamMessageXuid%d", localClientNum);
                Dvar_SetInt64ByName(teamMessageXuid, cgameGlob->bgs.clientinfo[playerNum].xuid);
                if ( cgameGlob->bgs.clientinfo[playerNum].clanAbbrev[0] )
                    Com_sprintf(clanTag, 0x2Au, "[%s]", cgameGlob->bgs.clientinfo[playerNum].clanAbbrev);
                UI_SetLocalVarStringByName("teamClanTagPopUp", clanTag, localClientNum);
                if ( playerNum == cgameGlob->clientNum
                    || cgameGlob->bgs.clientinfo[playerNum].oldteam
                    && selfClientInfo->oldteam == cgameGlob->bgs.clientinfo[playerNum].oldteam )
                {
                    friendly = 1;
                }
                CG_PlayPopUpSound(localClientNum, sound);
                UI_AnimateLocalItem(localClientNum, &cgDC[localClientNum], "teamMessageSlideOut", "SlideOut", "Start", 0);
            }
        }
    }
}

void __cdecl CG_DisplayWagerMedal(int localClientNum)
{
    const char *v1; // eax
    const char *v2; // eax
    char *localizedPopUpDesc; // [esp+18h] [ebp-24h]
    const char *medalPopUpName; // [esp+20h] [ebp-1Ch]
    char *sound; // [esp+24h] [ebp-18h]
    const char *medalPopUpDesc; // [esp+28h] [ebp-14h]
    char *localizedPopUpCp; // [esp+2Ch] [ebp-10h]
    bool teamBased; // [esp+33h] [ebp-9h]
    char *localizedPopUpName; // [esp+34h] [ebp-8h]
    int wagerMedalIndex; // [esp+38h] [ebp-4h]

    v1 = Cmd_Argv(2);
    wagerMedalIndex = atoi(v1);
    v2 = Cmd_Argv(3);
    teamBased = atoi(v2) != 0;
    sound = (char *)Cmd_Argv(4);
    if ( !Demo_IsThirdPersonCamera() && !Demo_IsMovieCamera() )
    {
        CG_GetLocalClientGlobals(localClientNum);
        medalPopUpName = CG_GetWagerMedalData(wagerMedalIndex, WAGER_MEDAL_COLUMN_NAME);
        if ( !medalPopUpName
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                        513,
                        0,
                        "%s",
                        "medalPopUpName != NULL") )
        {
            __debugbreak();
        }
        if ( medalPopUpName )
        {
            medalPopUpDesc = CG_GetWagerMedalData(wagerMedalIndex, WAGER_MEDAL_COLUMN_DESC);
            if ( !medalPopUpDesc
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                            517,
                            0,
                            "%s",
                            "medalPopUpDesc != NULL") )
            {
                __debugbreak();
            }
            if ( medalPopUpDesc )
            {
                localizedPopUpName = (char *)SEH_LocalizeTextMessage(medalPopUpName, "popUpName", LOCMSG_SAFE);
                if ( !localizedPopUpName
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                521,
                                0,
                                "%s",
                                "localizedPopUpName != NULL") )
                {
                    __debugbreak();
                }
                if ( localizedPopUpName )
                {
                    localizedPopUpDesc = (char *)SEH_LocalizeTextMessage(medalPopUpDesc, "popUpDesc", LOCMSG_SAFE);
                    if ( !localizedPopUpDesc
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                    525,
                                    0,
                                    "%s",
                                    "localizedPopUpDesc != NULL") )
                    {
                        __debugbreak();
                    }
                    if ( localizedPopUpDesc )
                    {
                        localizedPopUpCp = CG_GetWagerMedalLocalizedCP(wagerMedalIndex, teamBased);
                        CG_DisplayCenterPopup(
                            localClientNum,
                            "MedalPopUp",
                            localizedPopUpName,
                            localizedPopUpDesc,
                            localizedPopUpCp,
                            0,
                            0);
                        CG_PlayPopUpSound(localClientNum, sound);
                    }
                }
            }
        }
    }
}

void __cdecl CG_DisplayMedal(int localClientNum)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    char localizedPopUpDesc[132]; // [esp+2Ch] [ebp-D8h] BYREF
    const char *popUpXp; // [esp+B4h] [ebp-50h]
    cg_s *cgameGlob; // [esp+B8h] [ebp-4Ch]
    const char *medalPopUpName; // [esp+BCh] [ebp-48h]
    const char *sound; // [esp+C0h] [ebp-44h]
    const char *medalPopUpDesc; // [esp+C4h] [ebp-40h]
    float xpScale; // [esp+C8h] [ebp-3Ch]
    const char *localizedPopUpXp; // [esp+CCh] [ebp-38h]
    bool teamBased; // [esp+D3h] [ebp-31h]
    const char *localizedPopUpName; // [esp+D4h] [ebp-30h]
    ConversionArguments conversion; // [esp+D8h] [ebp-2Ch] BYREF
    int medalIndex; // [esp+100h] [ebp-4h]

    v1 = Cmd_Argv(2);
    medalIndex = atoi(v1);
    v2 = Cmd_Argv(3);
    teamBased = atoi(v2) != 0;
    v3 = Cmd_Argv(4);
    xpScale = atof(v3);
    sound = Cmd_Argv(5);
    if ( !Demo_IsThirdPersonCamera() && !Demo_IsMovieCamera() )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        medalPopUpName = CG_GetMedalData(medalIndex, MEDAL_COLUMN_NAME);
        if ( !medalPopUpName
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                        556,
                        0,
                        "%s",
                        "medalPopUpName != NULL") )
        {
            __debugbreak();
        }
        if ( medalPopUpName )
        {
            medalPopUpDesc = CG_GetMedalData(medalIndex, MEDAL_COLUMN_DESC);
            if ( !medalPopUpDesc
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                            560,
                            0,
                            "%s",
                            "medalPopUpDesc != NULL") )
            {
                __debugbreak();
            }
            if ( medalPopUpDesc )
            {
                localizedPopUpName = SEH_LocalizeTextMessage(medalPopUpName, "popUpName", LOCMSG_SAFE);
                if ( !localizedPopUpName
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                564,
                                0,
                                "%s",
                                "localizedPopUpName != NULL") )
                {
                    __debugbreak();
                }
                if ( localizedPopUpName )
                {
                    localizedPopUpXp = CG_GetMedalLocalizedXP(xpScale, medalIndex, teamBased);
                    if ( !localizedPopUpXp
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                    568,
                                    0,
                                    "%s",
                                    "localizedPopUpXp != NULL") )
                    {
                        __debugbreak();
                    }
                    if ( localizedPopUpXp )
                    {
                        conversion.argCount = 2;
                        conversion.args[0] = SEH_LocalizeTextMessage(medalPopUpDesc, "popUpDesc", LOCMSG_SAFE);
                        if ( !conversion.args[0]
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                        573,
                                        0,
                                        "%s",
                                        "conversion.args[0] != NULL") )
                        {
                            __debugbreak();
                        }
                        if ( conversion.args[0] )
                        {
                            conversion.args[1] = CG_GetMedalLocalizedXP(xpScale, medalIndex, teamBased);
                            popUpXp = SEH_LocalizeTextMessage("MEDAL_DESC_POPUP", "popUpName", LOCMSG_SAFE);
                            if ( !popUpXp
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                            579,
                                            0,
                                            "%s",
                                            "popUpXp != NULL") )
                            {
                                __debugbreak();
                            }
                            if ( popUpXp )
                            {
                                if ( Live_IsPublicOnlineMatch() || Live_IsCombatTrainingMatch() )
                                {
                                    UI_ReplaceConversions((char *)popUpXp, &conversion, localizedPopUpDesc, 128);
                                    CG_DisplayCenterPopup(
                                        localClientNum,
                                        "MedalPopUp",
                                        (char *)localizedPopUpName,
                                        localizedPopUpDesc,
                                        (char *)localizedPopUpXp,
                                        0,
                                        0);
                                }
                                else
                                {
                                    CG_DisplayCenterPopup(
                                        localClientNum,
                                        "MedalPopUp",
                                        (char *)localizedPopUpName,
                                        (char *)conversion.args[0],
                                        0,
                                        0,
                                        0);
                                }
                                CG_PlayPopUpSound(localClientNum, (char *)sound);
                            }
                        }
                    }
                }
            }
        }
    }
}

void __cdecl CG_DisplayContract(int localClientNum)
{
    const char *v1; // eax
    const char *v2; // eax
    char *v3; // eax
    int cpReward; // [esp+18h] [ebp-2Ch]
    char *localizedPopUpDesc; // [esp+1Ch] [ebp-28h]
    int contractIndex; // [esp+28h] [ebp-1Ch]
    int passed; // [esp+30h] [ebp-14h]
    char *sound; // [esp+34h] [ebp-10h]
    char *localizedCP; // [esp+38h] [ebp-Ch]
    char *localizedPopUpName; // [esp+40h] [ebp-4h]

    v1 = Cmd_Argv(2);
    contractIndex = atoi(v1);
    sound = (char *)Cmd_Argv(3);
    v2 = Cmd_Argv(4);
    passed = atoi(v2);
    if ( !Demo_IsThirdPersonCamera() && !Demo_IsMovieCamera() )
    {
        CG_GetLocalClientGlobals(localClientNum)->popUpAngleDelta = 0.5f;
        UI_SafeTranslateString("MP_CONTRACT_COMPLETE_DESC");
        localizedPopUpName = LiveContracts_GetContractName(contractIndex);
        if ( !localizedPopUpName
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                        619,
                        0,
                        "%s",
                        "localizedPopUpName != NULL") )
        {
            __debugbreak();
        }
        if ( localizedPopUpName )
        {
            localizedPopUpDesc = LiveContracts_GetContractDesc(contractIndex);
            if ( !localizedPopUpDesc
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                            623,
                            0,
                            "%s",
                            "localizedPopUpDesc != NULL") )
            {
                __debugbreak();
            }
            if ( localizedPopUpDesc )
            {
                if ( passed )
                {
                    cpReward = LiveContracts_GetContractRewardCP(contractIndex);
                    if ( cpReward <= 0 )
                    {
                        localizedCP = (char *)"";
                    }
                    else
                    {
                        v3 = UI_SafeTranslateString("MENU_POINTS");
                        localizedCP = UI_ReplaceConversionInt(v3, cpReward);
                    }
                    CG_DisplayCenterPopup(
                        localClientNum,
                        "ContractPopUp",
                        localizedPopUpName,
                        localizedPopUpDesc,
                        localizedCP,
                        (char*)"contract_completed",
                        (char*)"hud_medal_burst_green");
                }
                else
                {
                    CG_DisplayCenterPopup(
                        localClientNum,
                        "ContractPopUp",
                        localizedPopUpName,
                        localizedPopUpDesc,
                        (char *)"",
                        (char*)"contract_expired",
                        (char*)"hud_medal_burst_red");
                }
                CG_PlayPopUpSound(localClientNum, sound);
            }
        }
    }
}

void __cdecl CG_DisplayKillstreak(int localClientNum)
{
    const char *streakCount; // [esp+14h] [ebp-19Ch]
    char eventString[64]; // [esp+18h] [ebp-198h] BYREF
    cg_s *cgameGlob; // [esp+58h] [ebp-158h]
    const char *popUpDesc; // [esp+5Ch] [ebp-154h]
    const StringTable *tablePtr; // [esp+60h] [ebp-150h] BYREF
    const char *killstreakText; // [esp+64h] [ebp-14Ch]
    const char *killstreakTextUnlocalized; // [esp+68h] [ebp-148h]
    const char *killstreakIcon; // [esp+6Ch] [ebp-144h]
    const char *displayString; // [esp+70h] [ebp-140h]
    const char *killstreakTableNumber; // [esp+74h] [ebp-13Ch]
    char binding[260]; // [esp+78h] [ebp-138h] BYREF
    const char *popUpName; // [esp+180h] [ebp-30h]
    const char *localizedPopUpName; // [esp+184h] [ebp-2Ch]
    ConversionArguments conversion; // [esp+188h] [ebp-28h] BYREF

    streakCount = Cmd_Argv(2);
    killstreakTableNumber = Cmd_Argv(3);
    CG_GetLocalClientGlobals(localClientNum)->popUpAngleDelta = 0.0f;
    StringTable_GetAsset("mp/statstable.csv", (XAssetHeader *)&tablePtr);
    if ( !tablePtr
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                    676,
                    0,
                    "%s",
                    "tablePtr != NULL") )
    {
        __debugbreak();
    }
    if ( tablePtr )
    {
        killstreakIcon = StringTable_Lookup(tablePtr, 0, killstreakTableNumber, 6);
        if ( !killstreakIcon
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                        681,
                        0,
                        "%s",
                        "killstreakIcon != NULL") )
        {
            __debugbreak();
        }
        if ( killstreakIcon )
        {
            killstreakTextUnlocalized = StringTable_Lookup(tablePtr, 0, killstreakTableNumber, 3);
            if ( !killstreakTextUnlocalized
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                            685,
                            0,
                            "%s",
                            "killstreakTextUnlocalized != NULL") )
            {
                __debugbreak();
            }
            if ( killstreakTextUnlocalized )
            {
                killstreakText = UI_SafeTranslateString(killstreakTextUnlocalized);
                cgameGlob = CG_GetLocalClientGlobals(localClientNum);
                UI_GetKeyBindingLocalizedString(localClientNum, "+actionslot 4", binding, 0);
                displayString = SEH_LocalizeTextMessage("KILLSTREAK_PRESS_TO_KILLSTREAK", "Hint String", LOCMSG_SAFE);
                if ( !displayString
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ui_animate_mp.cpp",
                                695,
                                0,
                                "%s",
                                "displayString != NULL") )
                {
                    __debugbreak();
                }
                if ( displayString )
                {
                    conversion.argCount = 1;
                    conversion.args[0] = binding;
                    popUpName = UI_SafeTranslateString("KILLSTREAK_KILLCOUNT_DISPLAY");
                    popUpDesc = UI_SafeTranslateString(displayString);
                    localizedPopUpName = UI_ReplaceConversionString((char *)popUpName, streakCount);
                    UI_ReplaceConversions((char *)popUpDesc, &conversion, eventString, 64);
                    if ( *streakCount == 48 )
                        localizedPopUpName = killstreakText;
                    CG_DisplayCenterPopup(
                        localClientNum,
                        "KillstreakPopUp",
                        (char *)localizedPopUpName,
                        eventString,
                        0,
                        (char *)killstreakIcon,
                        0);
                }
            }
        }
    }
}

void __cdecl CG_DisplayWagerPopup(int localClientNum)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    char *ConfigString; // eax
    char *v5; // eax
    char *v6; // eax
    int messageStringIndex; // [esp+20h] [ebp-1Ch]
    char *subMessageString; // [esp+24h] [ebp-18h]
    int points; // [esp+28h] [ebp-14h]
    int subMessageStringIndex; // [esp+2Ch] [ebp-10h]
    char *localizedPoints; // [esp+34h] [ebp-8h]
    char *messageString; // [esp+38h] [ebp-4h]

    subMessageString = (char *)"";
    v1 = Cmd_Argv(2);
    messageStringIndex = atoi(v1);
    v2 = Cmd_Argv(3);
    points = atoi(v2);
    localizedPoints = 0;
    subMessageStringIndex = -1;
    if ( !Demo_IsThirdPersonCamera() && !Demo_IsMovieCamera() )
    {
        if ( Cmd_Argc() == 5 )
        {
            v3 = Cmd_Argv(4);
            subMessageStringIndex = atoi(v3);
        }
        CG_GetLocalClientGlobals(localClientNum);
        ConfigString = CL_GetConfigString(messageStringIndex + 515);
        messageString = UI_SafeTranslateString(ConfigString);
        if ( subMessageStringIndex >= 0 )
        {
            v5 = CL_GetConfigString(subMessageStringIndex + 515);
            subMessageString = UI_SafeTranslateString(v5);
        }
        if ( points > 0 )
        {
            v6 = UI_SafeTranslateString("MEDAL_XP_POPUP");
            localizedPoints = UI_ReplaceConversionInt(v6, points);
        }
        CG_DisplayCenterPopup(localClientNum, "WagerPopUp", messageString, subMessageString, localizedPoints, 0, 0);
        CG_PlayPopUpSound(localClientNum, (char*)"uin_challenge_repeatable");
    }
}

void __cdecl CG_DisplayHudAnim(int localClientNum)
{
    const char *menuName; // [esp+10h] [ebp-8h]
    const char *animName; // [esp+14h] [ebp-4h]

    animName = Cmd_Argv(3);
    menuName = Cmd_Argv(2);
    UI_AnimateItem(localClientNum, &cgDC[localClientNum], menuName, animName, "Start", 0);
}

