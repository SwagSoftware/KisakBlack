#include "cg_draw_mp.h"
#include <cgame/cg_hudelem.h>
#include "cg_local_mp.h"
#include <client/cl_rank.h>
#include "cg_ui_animate_mp.h"
#include <cgame/cg_drawtools.h>
#include "cg_main_mp.h"
#include <cgame/cg_draw_names.h>
#include <stringed/stringed_hooks.h>
#include <demo/demo_playback.h>
#include "cg_newDraw_mp.h"
#include <ui_mp/ui_main_mp.h>
#include "cg_scoreboard_mp.h"
#include <cgame/cg_draw_reticles.h>
#include <cgame/cg_draw_indicators.h>
#include "cg_draw_net_mp.h"
#include <cgame/cg_draw_debug.h>
#include <client_mp/cl_cgame_mp.h>
#include <ui/ui_atoms.h>
#include "cg_servercmds_mp.h"
#include <client/splitscreen.h>
#include <bgame/bg_misc.h>
#include <bgame/bg_pmove.h>
#include <client/cl_console.h>
#include <client/cl_keys.h>
#include <xanim/xmodel.h>
#include <cgame/cg_localents.h>
#include <gfx_d3d/r_rope_render.h>
#include "cg_players_mp.h"
#include <bgame/bg_fire.h>
#include <flame/flame_class_stream.h>
#include <cgame/cg_local.h>
#include <qcommon/common.h>

const char *WeaponStateNames[50] =
{
  "WEAPON_READY",
  "WEAPON_RAISING",
  "WEAPON_RAISING_ALTSWITCH",
  "WEAPON_DROPPING",
  "WEAPON_DROPPING_QUICK",
  "WEAPON_DROPPING_ALTSWITCH",
  "WEAPON_FIRING",
  "WEAPON_RECHAMBERING",
  "WEAPON_RELOADING_RIGHT",
  "WEAPON_RELOADING_LEFT",
  "WEAPON_RELOADING",
  "WEAPON_RELOADING_INTERUPT",
  "WEAPON_RELOAD_START",
  "WEAPON_RELOAD_START_INTERUPT",
  "WEAPON_RELOAD_END",
  "WEAPON_RELOAD_QUICK",
  "WEAPON_RELOAD_QUICK_EMPTY",
  "WEAPON_MELEE_INIT",
  "WEAPON_MELEE_FIRE",
  "WEAPON_MELEE_END",
  "WEAPON_OFFHAND_INIT",
  "WEAPON_OFFHAND_PREPARE",
  "WEAPON_OFFHAND_HOLD",
  "WEAPON_OFFHAND_START",
  "WEAPON_OFFHAND",
  "WEAPON_OFFHAND_END",
  "WEAPON_DETONATING",
  "WEAPON_SPRINT_RAISE",
  "WEAPON_SPRINT_LOOP",
  "WEAPON_SPRINT_DROP",
  "WEAPON_CONT_FIRE_IN",
  "WEAPON_CONT_FIRE_LOOP",
  "WEAPON_CONT_FIRE_OUT",
  "WEAPON_NIGHTVISION_WEAR",
  "WEAPON_NIGHTVISION_REMOVE",
  "WEAPON_DEPLOYING",
  "WEAPON_DEPLOYED",
  "WEAPON_BREAKING_DOWN",
  "WEAPON_SWIM_IN",
  "WEAPON_SWIM_OUT",
  "WEAPON_DTP_IN",
  "WEAPON_DTP_LOOP",
  "WEAPON_DTP_OUT",
  "WEAPON_SLIDE_IN",
  "WEAPON_FIRING_LEFT",
  "WEAPON_FIRING_BOTH",
  "WEAPON_JAMMED",
  "WEAPON_LOWREADY_RAISE",
  "WEAPON_LOWREADY_LOOP",
  "WEAPON_LOWREADY_DROP"
};

CenterPrint s_centerPrint[1];
ScreenBurn s_screenBurn[1];
ScreenBurn s_screenElectrified[1];

void __cdecl CG_PriorityCenterPrint(int localClientNum, const char *str, int priority)
{
    CenterPrint *centerPrint; // [esp+4h] [ebp-10Ch]
    char hudElemString[260]; // [esp+8h] [ebp-108h] BYREF

    centerPrint = &s_centerPrint[localClientNum];
    if ( !centerPrint->time || priority >= centerPrint->priority )
    {
        CG_TranslateHudElemMessage(localClientNum, str, "Center Print", hudElemString);
        I_strncpyz(centerPrint->text, hudElemString, 256);
        centerPrint->priority = priority;
        centerPrint->time = CG_GetLocalClientGlobals(localClientNum)->time;
    }
}

void __cdecl CG_DisplayPlayerCard(
                int localClientNum,
                const clientInfo_t *attackerCI,
                const clientInfo_t *victimCI,
                bool wasKilled)
{
    char *rankIconName; // [esp+0h] [ebp-44Ch]
    char *rank; // [esp+8h] [ebp-444h]
    char playerCardXuid[1024]; // [esp+Ch] [ebp-440h] BYREF
    const char *localizedKillString; // [esp+40Ch] [ebp-40h]
    char clanTag[44]; // [esp+410h] [ebp-3Ch] BYREF
    bool friendly; // [esp+443h] [ebp-9h]
    const clientInfo_t *displayCI; // [esp+444h] [ebp-8h]
    const char *killString; // [esp+448h] [ebp-4h]

    memset(playerCardXuid, 0, sizeof(playerCardXuid));
    memset(clanTag, 0, 42);
    friendly = 0;
    displayCI = 0;
    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_draw_mp.cpp",
                    297,
                    0,
                    "%s",
                    "localClientNum < MAX_LOCAL_CLIENTS && localClientNum >= 0") )
    {
        __debugbreak();
    }
    if ( localClientNum != 1 && localClientNum >= 0 )
    {
        if ( wasKilled )
        {
            killString = "CGAME_KILLEDBY";
            displayCI = attackerCI;
        }
        else
        {
            killString = "CGAME_KILLED";
            displayCI = victimCI;
        }
        if ( !displayCI
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_draw_mp.cpp", 312, 0, "%s", "displayCI") )
        {
            __debugbreak();
        }
        if ( displayCI )
        {
            if ( displayCI->rank >= 999
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_draw_mp.cpp",
                            316,
                            0,
                            "%s",
                            "displayCI->rank < 999") )
            {
                __debugbreak();
            }
            if ( attackerCI->oldteam && victimCI->oldteam == attackerCI->oldteam )
            {
                friendly = 1;
                if ( wasKilled )
                    killString = "CGAME_TEAMKILLEDBY";
                else
                    killString = "CGAME_TEAMKILLED";
            }
            Com_sprintf(playerCardXuid, 0x400u, "playerCardXuid%d", localClientNum);
            Dvar_SetInt64ByName(playerCardXuid, displayCI->xuid);
            if ( displayCI->rank < 999 )
            {
                rankIconName = (char *)CL_GetRankIconName(displayCI->rank, displayCI->prestige);
                if ( !rankIconName
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_draw_mp.cpp",
                                336,
                                0,
                                "%s",
                                "rankIconName") )
                {
                    __debugbreak();
                }
                if ( !rankIconName )
                    return;
                UI_SetLocalVarStringByName("rankIconPopUp", rankIconName, localClientNum);
                rank = (char *)CL_GetRankData(displayCI->rank, MP_RANKTABLE_DISPLAYLEVEL);
                if ( !rank
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_draw_mp.cpp", 341, 0, "%s", "rank") )
                {
                    __debugbreak();
                }
                if ( !rank )
                    return;
                UI_SetLocalVarStringByName("rankPopUp", rank, localClientNum);
            }
        }
        else
        {
            UI_SetLocalVarStringByName("rankIconPopUp", 0, localClientNum);
            UI_SetLocalVarStringByName("rankPopUp", 0, localClientNum);
        }
        localizedKillString = SEH_LocalizeTextMessage(killString, "popUpDesc", LOCMSG_SAFE);
        UI_SetLocalVarStringByName("killStringPopUp", (char *)localizedKillString, localClientNum);
        UI_SetLocalVarStringByName("namePopUp", (char*)displayCI->name, localClientNum);
        if ( displayCI->clanAbbrev[0] )
            Com_sprintf(clanTag, 0x2Au, "[%s]", displayCI->clanAbbrev);
        UI_SetLocalVarStringByName("clanTagPopUp", clanTag, localClientNum);
        UI_AnimateLocalItem(localClientNum, &cgDC[localClientNum], "playerCard", "playerCardPopUp", "Start", 0);
        UI_AnimateLocalItem(localClientNum, &cgDC[localClientNum], "playerKilled", "playerCardPopUp", "Start", 0);
    }
}

void __cdecl CG_ClearCenterPrint(int localClientNum)
{
    s_centerPrint[localClientNum].time = 0;
}

void __cdecl CG_DrawCenterString(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float fontscale,
                float *color,
                int textStyle)
{
    CenterPrint *centerPrint; // [esp+30h] [ebp-10h]
    float *fadeColor; // [esp+34h] [ebp-Ch]
    int time; // [esp+38h] [ebp-8h]
    float x; // [esp+3Ch] [ebp-4h]

    time = CG_GetLocalClientGlobals(localClientNum)->time;
    centerPrint = &s_centerPrint[localClientNum];
    if ( centerPrint->time > time )
        centerPrint->time = 0;
    if ( centerPrint->time )
    {
        fadeColor = CG_FadeColor(time, centerPrint->time, (int)(float)(1000.0 * cg_centertime->current.value), 100);
        if ( fadeColor )
        {
            *color = *color * *fadeColor;
            color[1] = color[1] * fadeColor[1];
            color[2] = color[2] * fadeColor[2];
            color[3] = color[3] * fadeColor[3];
            x = rect->x
                - (float)SnapFloatToInt((float)((float)UI_TextWidth(centerPrint->text, 0, font, fontscale) * 0.5));
            UI_DrawText(
                &scrPlaceView[localClientNum],
                centerPrint->text,
                0x7FFFFFFF,
                font,
                x,
                rect->y,
                rect->horzAlign,
                rect->vertAlign,
                fontscale,
                color,
                textStyle);
        }
        else
        {
            centerPrint->time = 0;
            centerPrint->priority = 0;
        }
    }
}

void __cdecl CG_Electrified(int localClientNum, int durationMS)
{
    const cg_s *cgameGlob; // [esp+0h] [ebp-8h]
    ScreenBurn *se; // [esp+4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    se = &s_screenElectrified[localClientNum];
    se->startTime = cgameGlob->time;
    se->endTime = durationMS + cgameGlob->time;
    se->localClientNum = localClientNum;
    CG_SetupElectrifiedFX(localClientNum, 1);
    Com_Printf(21, "Player %d electrified for %d (ms) at time: %d\n", localClientNum, durationMS, se->startTime);
}

void __cdecl CG_ClearElectrified(int localClientNum)
{
    ScreenBurn *se; // [esp+0h] [ebp-4h]

    se = &s_screenElectrified[localClientNum];
    se->startTime = 0;
    se->endTime = 0;
    se->localClientNum = 0;
    CG_SetupElectrifiedFX(localClientNum, 0);
    Com_Printf(21, "Player %d no longer electrified at time: %d\n", localClientNum, se->startTime);
}

void __cdecl CG_Burn(int localClientNum, int durationMS)
{
    const cg_s *cgameGlob; // [esp+0h] [ebp-8h]
    ScreenBurn *scrBurn; // [esp+4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    scrBurn = &s_screenBurn[localClientNum];
    scrBurn->startTime = cgameGlob->time;
    scrBurn->endTime = durationMS + cgameGlob->time;
    scrBurn->localClientNum = localClientNum;
    CG_SetupFlameFX(localClientNum, 1);
    Com_Printf(21, "Player %d burning for %d (ms) at time: %d\n", localClientNum, durationMS, scrBurn->startTime);
}

void __cdecl CG_ClearBurn(int localClientNum)
{
    ScreenBurn *scrBurn; // [esp+0h] [ebp-4h]

    scrBurn = &s_screenBurn[localClientNum];
    scrBurn->startTime = 0;
    scrBurn->endTime = 0;
    scrBurn->localClientNum = 0;
    CG_SetupFlameFX(localClientNum, 0);
    Com_Printf(21, "Player %d no longer burning at time: %d\n", localClientNum, scrBurn->startTime);
}

bool __cdecl IsHardcoreMode(int localClientNum)
{
    return (CG_GetLocalClientGlobals(localClientNum)->matchUIVisibilityFlags & 0x200) != 0;
}

bool __cdecl CG_AreAllPlayerNamesVisible()
{
    if ( !Demo_IsPlaying() )
        return CG_IsVehicleMayhemGameType();
    if ( Demo_IsGameHudHidden() )
        return 0;
    return Demo_IsMovieCamera();
}

void __cdecl CG_Draw2D(int localClientNum)
{
    CG_Draw2DInternal(localClientNum);
}

void __cdecl CG_Draw2DInternal(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-14h]
    bool drawHud; // [esp+7h] [ebp-Dh]
    int isScoreboardVisible; // [esp+8h] [ebp-Ch]
    bool chatOverScoreboard; // [esp+Ch] [ebp-8h]
    const playerState_s *ps; // [esp+10h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->cubemapShot == CUBEMAPSHOT_NONE && cg_draw2D->current.enabled )
    {
        CG_DrawGenericOverlay(0, localClientNum);
        if ( debugOverlay->current.integer == 1 )
        {
            DrawViewmodelInfo(localClientNum);
        }
        else if ( !net_showprofile->current.integer )
        {
            drawHud = CG_ShouldDrawHud(localClientNum);
            ps = &cgameGlob->nextSnap->ps;
            if ( cgameGlob->nextSnap->ps.pm_type == 5 )
            {
                DrawIntermission(localClientNum);
                CG_DrawSay(localClientNum);
            }
            else
            {
                if ( ui_showEndOfGame->current.enabled )
                    Dvar_SetBool((dvar_s *)ui_showEndOfGame, 0);
                CG_CompassUpdateActors(localClientNum);
                chatOverScoreboard = CG_IsScoreboardDisplayed(localClientNum);
                if ( ps->pm_type == 4 )
                {
                    if ( drawHud )
                    {
                        CG_ScanForCrosshairEntity(localClientNum);
                        CG_UpdatePlayerNames(localClientNum);
                        if ( !chatOverScoreboard )
                            CG_DrawChatMessages(localClientNum);
                        CG_Draw2dHudElems(localClientNum, 0);
                        Menu_PaintAll(localClientNum, &cgDC[localClientNum]);
                        CG_Draw2dHudElems(localClientNum, 1);
                    }
                }
                else
                {
                    CG_DrawNightVisionOverlay(localClientNum);
                    CG_ScanForCrosshairEntity(localClientNum);
                    CG_DrawCrosshair(localClientNum);
                    if ( drawHud )
                    {
                        if ( ps->pm_type < 9 || Demo_IsMovieCamera() )
                            CG_UpdatePlayerNames(localClientNum);
                        if ( !chatOverScoreboard )
                            CG_DrawChatMessages(localClientNum);
                    }
                    CG_CheckTimedMenus(localClientNum);
                    if ( drawHud )
                    {
                        CG_Draw2dHudElems(localClientNum, 0);
                        Menu_PaintAll(localClientNum, &cgDC[localClientNum]);
                    }
                    CG_Draw2dHudElems(localClientNum, 1);
                }
                CG_DrawGenericOverlay(1, localClientNum);
                isScoreboardVisible = CG_DrawScoreboard(localClientNum);
                CG_DrawSay(localClientNum);
                if ( drawHud )
                {
                    if ( chatOverScoreboard )
                        CG_DrawChatMessages(localClientNum);
                    if ( (ps->eFlags & 0x4000) == 0 || ps->vehicleType != 6 )
                    {
                        if ( !CG_KillCamEntityEnabled(localClientNum) )
                        {
                            CG_DrawFlashDamage(cgameGlob);
                            CG_DrawDamageDirectionIndicators(localClientNum);
                        }
                        CG_DrawGrenadeIndicators(localClientNum);
                        CG_ScreenBurn(localClientNum);
                        CG_ScreenElectrified(localClientNum);
                    }
                    if ( !isScoreboardVisible && (cgameGlob->matchUIVisibilityFlags & 0x800) != 0 )
                        CG_DrawSpectatorMessage(localClientNum);
                    CG_DrawVote(localClientNum);
                    CG_DrawLagometer(localClientNum);
                    CG_DrawEntityCount(localClientNum);
                    CG_DrawServerBandwidth(localClientNum);
                    CG_DrawServerKillcamData(localClientNum);
                    CG_DrawDebugOverlays(localClientNum);
                    Demo_DrawTransitionScreen(localClientNum);
                    Demo_DrawMetaInformationScreen(localClientNum);
                    if ( !isScoreboardVisible )
                    {
                        CG_DrawMiniConsole(localClientNum);
                        CG_DrawErrorMessages(localClientNum);
                    }
                }
            }
        }
    }
}

void __cdecl CG_DrawChatMessages(int localClientNum)
{
    int v1; // edx
    const dvar_s *v2; // [esp+20h] [ebp-58h]
    Font_s *font; // [esp+24h] [ebp-54h]
    ScreenPlacement *scrPlace; // [esp+28h] [ebp-50h]
    int cgameTimeNow; // [esp+30h] [ebp-48h]
    float alphapercent; // [esp+38h] [ebp-40h]
    float alphapercenta; // [esp+38h] [ebp-40h]
    const char *msg; // [esp+3Ch] [ebp-3Ch]
    int hudChatY; // [esp+40h] [ebp-38h]
    cgs_t *cgs; // [esp+44h] [ebp-34h]
    int hudChatX; // [esp+48h] [ebp-30h]
    int i; // [esp+4Ch] [ebp-2Ch]
    float fontHeight; // [esp+50h] [ebp-28h]
    float y; // [esp+58h] [ebp-20h]
    float ya; // [esp+58h] [ebp-20h]
    float color[4]; // [esp+5Ch] [ebp-1Ch] BYREF
    float fontScale; // [esp+6Ch] [ebp-Ch]
    int chatHeight; // [esp+70h] [ebp-8h]
    float w; // [esp+74h] [ebp-4h]

    if ( !cg_hudChatPosition
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_draw_mp.cpp",
                    181,
                    0,
                    "%s",
                    "cg_hudChatPosition") )
    {
        __debugbreak();
    }
    chatHeight = cg_chatHeight->current.integer;
    if ( chatHeight )
    {
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        if ( !cg_hudChatIntermissionPosition
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_draw_mp.cpp",
                        190,
                        0,
                        "%s",
                        "cg_hudChatIntermissionPosition") )
        {
            __debugbreak();
        }
        v2 = CG_IsScoreboardDisplayed(localClientNum) ? cg_hudChatIntermissionPosition : cg_hudChatPosition;
        fontHeight = cgs->viewHeight <= 600 ? 16.0f : 10.0f;
        fontScale = fontHeight / 48.0;
        hudChatX = (int)v2->current.value;
        hudChatY = (int)v2->current.vector[1];
        if ( cgs->teamLastChatPos != cgs->teamChatPos )
        {
            cgameTimeNow = CG_GetLocalClientGlobals(localClientNum)->time;
            if ( cgameTimeNow - cgs->teamChatMsgTimes[cgs->teamLastChatPos % chatHeight] > cg_chatTime->current.integer )
                ++cgs->teamLastChatPos;
            scrPlace = &scrPlaceView[localClientNum];
            font = UI_GetFontHandle(scrPlace, 0, fontScale);
            for ( i = cgs->teamChatPos - 1; ; --i )
            {
                if ( i < cgs->teamLastChatPos )
                    return;
                alphapercent = (float)cg_chatTime->current.integer
                                         - (float)(cgameTimeNow - cgs->teamChatMsgTimes[i % chatHeight]);
                if ( alphapercent <= 200.0 )
                {
                    alphapercenta = alphapercent / 200.0;
                    if ( alphapercenta <= 0.0 )
                        continue;
                }
                else
                {
                    alphapercenta = 1.0f;
                }
                v1 = i % chatHeight;
                msg = cgs->teamChatMsgs[v1];
                if ( (cgs_t *)((char *)cgs + v1 * 241) == (cgs_t *)-4128
                    || *msg != '^'
                    || !cgs->teamChatMsgs[v1][1]
                    || cgs->teamChatMsgs[v1][1] == '^'
                    || cgs->teamChatMsgs[v1][1] < '0'
                    || cgs->teamChatMsgs[v1][1] > 64 )
                {
                    color[0] = 1.0f;
                    color[1] = 1.0f;
                    color[2] = 1.0f;
                }
                else
                {
                    CL_LookupColor(cgs->teamChatMsgs[v1][1], color);
                }
                color[0] = 0.25 * color[0];
                color[1] = 0.25 * color[1];
                color[2] = 0.25 * color[2];
                color[3] = 0.60000002 * alphapercenta;
                y = (float)hudChatY - (float)((float)(cgs->teamChatPos - i) * fontHeight);
                w = (float)UI_TextWidth(msg, 0, font, fontScale) + (float)(8.0 * 3.0);
                UI_DrawHandlePic(scrPlace, 0.0, y, w, fontHeight, 1, 1, color, cgMedia.teamStatusBar);
                color[3] = alphapercenta;
                ya = (float)((float)hudChatY - (float)((float)(cgs->teamChatPos - i) * fontHeight)) + (float)(fontHeight - 1.0);
                color[0] = 1.0f;
                color[1] = 1.0f;
                color[2] = 1.0f;
                UI_DrawText(scrPlace, (char*)msg, 0x7FFFFFFF, font, (float)hudChatX, ya, 1, 1, fontScale, color, 3);
            }
        }
    }
}

void __cdecl CG_CheckTimedMenus(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->voiceTime && cgameGlob->time - cgameGlob->voiceTime > 2500 )
    {
        Menus_CloseByName(localClientNum, &cgDC[localClientNum], "voiceMenu");
        cgameGlob->voiceTime = 0;
    }
    CG_CheckForPlayerInput(localClientNum);
    CG_CheckHudHealthDisplay(localClientNum);
    CG_CheckHudAmmoDisplay(localClientNum);
    CG_CheckHudCompassDisplay(localClientNum);
    CG_CheckHudStanceDisplay(localClientNum);
    CG_CheckHudOffHandDisplay(localClientNum);
    CG_CheckHudObjectiveDisplay(localClientNum);
    CG_CheckHudSprintDisplay(localClientNum);
    CG_CheckVehicleControlsDisplay(localClientNum);
}

void __cdecl CG_CheckForPlayerInput(int localClientNum)
{
    usercmd_s v1; // [esp-68h] [ebp-10Ch] BYREF
    usercmd_s v2; // [esp-34h] [ebp-D8h] BYREF
    int n; // [esp+8h] [ebp-9Ch]
    int m; // [esp+Ch] [ebp-98h]
    int k; // [esp+10h] [ebp-94h]
    bitarray<51> *v6; // [esp+14h] [ebp-90h]
    int j; // [esp+18h] [ebp-8Ch]
    bitarray<51> *p_button_bits; // [esp+1Ch] [ebp-88h]
    int i; // [esp+20h] [ebp-84h]
    usercmd_s oldCmd; // [esp+24h] [ebp-80h] BYREF
    int oldCmdIndex; // [esp+58h] [ebp-4Ch]
    usercmd_s newCmd; // [esp+5Ch] [ebp-48h] BYREF
    bitarray<51> changedButton_bits; // [esp+94h] [ebp-10h] BYREF
    int newInput; // [esp+9Ch] [ebp-8h]
    int newCmdIndex; // [esp+A0h] [ebp-4h]

    p_button_bits = &oldCmd.button_bits;
    for ( i = 0; i < 2; ++i )
        p_button_bits->array[i] = 0;
    v6 = &newCmd.button_bits;
    for ( j = 0; j < 2; ++j )
        v6->array[j] = 0;
    for ( k = 0; k < 2; ++k )
        changedButton_bits.array[k] = 0;
    newCmdIndex = CL_GetCurrentCmdNumber(localClientNum);
    if ( newCmdIndex > 1 )
    {
        oldCmdIndex = newCmdIndex - 1;
        CL_GetUserCmd(localClientNum, newCmdIndex - 1, &oldCmd);
        CL_GetUserCmd(localClientNum, newCmdIndex, &newCmd);
        for ( m = 0; m < 2; ++m )
            changedButton_bits.array[m] = newCmd.button_bits.array[m];
        for ( n = 0; n < 2; ++n )
            changedButton_bits.array[n] ^= oldCmd.button_bits.array[n];
        memcpy(&v2, &newCmd, sizeof(v2));
        memcpy(&v1, &oldCmd, sizeof(v1));
        newInput = CG_CheckPlayerMovement(v1, v2);
        if ( CG_CheckPlayerWeaponUsage(localClientNum, &newCmd.button_bits) )
            newInput = 1;
        if ( CG_CheckPlayerOffHandUsage(localClientNum, &newCmd.button_bits) )
            newInput = 1;
        if ( CG_CheckPlayerStanceChange(localClientNum, &newCmd.button_bits, &changedButton_bits) )
            newInput = 1;
        if ( !newInput )
            newInput = CG_CheckPlayerMiscInput(&changedButton_bits) != 0;
        if ( newInput )
            CG_MenuShowNotify(localClientNum, 3);
    }
}

bool __cdecl CG_CheckPlayerMovement(usercmd_s oldCmd, usercmd_s newCmd)
{
    if ( memcmp(oldCmd.angles, newCmd.angles, 0xCu) )
        return 1;
    return newCmd.forwardmove || newCmd.rightmove;
}

int __cdecl CG_CheckPlayerStanceChange(
                int localClientNum,
                bitarray<51> *newButton_bits,
                bitarray<51> *changedButton_bits)
{
    if ( changedButton_bits->testBit(9u)
        || changedButton_bits->testBit(8u)
        || changedButton_bits->testBit(12) )
    {
        CG_MenuShowNotify(localClientNum, 4);
        return 1;
    }
    else
    {
        if ( newButton_bits->testBit(9u)
            || newButton_bits->testBit(8u)
            || newButton_bits->testBit(0xCu) )
        {
            CG_MenuShowNotify(localClientNum, 4);
        }
        return 0;
    }
}

int __cdecl CG_CheckPlayerWeaponUsage(int localClientNum, bitarray<51> *button_bits)
{
    if ( !CG_CheckPlayerFireNonTurret(localClientNum, button_bits)
        && !CG_CheckPlayerTryReload(localClientNum, button_bits) )
    {
        return 0;
    }
    CG_MenuShowNotify(localClientNum, 1);
    return 1;
}

bool __cdecl CG_CheckPlayerTryReload(int localClientNum, bitarray<51> *button_bits)
{
    bool result; // eax
    const cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    result = 0;
    if ( button_bits->testBit(4) || button_bits->testBit(5) )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        if ( (cgameGlob->predictedPlayerState.pm_flags & 4) == 0 && (cgameGlob->predictedPlayerState.eFlags & 0x300) == 0 )
            return 1;
    }
    return result;
}

bool __cdecl CG_CheckPlayerFireNonTurret(int localClientNum, bitarray<51> *button_bits)
{
    return button_bits->testBit(0)
            && (CG_GetLocalClientGlobals(localClientNum)->predictedPlayerState.eFlags & 0x300) == 0;
}

int __cdecl CG_CheckPlayerOffHandUsage(int localClientNum, bitarray<51> *button_bits)
{
    if ( !button_bits->testBit(0xEu) && !button_bits->testBit(0xFu) )
        return 0;
    CG_MenuShowNotify(localClientNum, 5);
    return 1;
}

int __cdecl CG_CheckPlayerMiscInput(bitarray<51> *button_bits)
{
    int k; // [esp+4h] [ebp-14h]
    int j; // [esp+8h] [ebp-10h]
    int i; // [esp+Ch] [ebp-Ch]
    bitarray<51> mask_bits(12, 8, 9, -1); // [esp+10h] [ebp-8h] BYREF

    //bitarray<51>::bitarray<51>(&mask_bits, 0xCu, 8, 9, -1);
    for ( i = 0; i < 2; ++i )
        mask_bits.array[i] = ~mask_bits.array[i];
    for ( j = 0; j < 2; ++j )
        mask_bits.array[j] &= button_bits->array[j];
    for ( k = 0; k < 2; ++k )
    {
        if ( mask_bits.array[k] )
            return 1;
    }
    return 0;
}

void __cdecl CG_CheckHudHealthDisplay(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( hud_health_startpulse_injured->current.value <= CG_CalcPlayerHealth(&cgameGlob->nextSnap->ps) )
    {
        if ( hud_fade_healthbar->current.value != 0.0
            && cgameGlob->healthFadeTime
            && (float)(cgameGlob->time - cgameGlob->healthFadeTime) > (float)(hud_fade_healthbar->current.value * 1000.0) )
        {
            if ( CL_LocalClient_GetActiveCount() == 1 )
                Menus_HideByName(&cgDC[localClientNum], "Health");
            else
                Menus_HideByName(&cgDC[localClientNum], "Health_mp");
            cgameGlob->healthFadeTime = 0;
        }
    }
    else
    {
        CG_MenuShowNotify(localClientNum, 0);
    }
}

void __cdecl CG_CheckHudAmmoDisplay(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( CG_CheckPlayerForLowAmmo(cgameGlob) || CG_CheckPlayerForLowClip(cgameGlob) )
        CG_MenuShowNotify(localClientNum, 1);
    if ( hud_fade_ammodisplay->current.value != 0.0
        && cgameGlob->ammoFadeTime
        && (float)(cgameGlob->time - cgameGlob->ammoFadeTime) > (float)(hud_fade_ammodisplay->current.value * 1000.0) )
    {
        if ( CL_LocalClient_GetActiveCount() == 1 )
        {
            Menus_HideByName(&cgDC[localClientNum], "weaponinfo");
            Menus_HideByName(&cgDC[localClientNum], "weaponinfo_lowdef");
        }
        else
        {
            Menus_HideByName(&cgDC[localClientNum], "weaponinfo_mp");
        }
        cgameGlob->ammoFadeTime = 0;
    }
}

void __cdecl CG_CheckHudCompassDisplay(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( hud_fade_compass->current.value != 0.0
        && cgameGlob->compassFadeTime
        && (float)(cgameGlob->time - cgameGlob->compassFadeTime) > (float)(hud_fade_compass->current.value * 1000.0) )
    {
        if ( CL_LocalClient_GetActiveCount() == 1 )
            Menus_HideByName(&cgDC[localClientNum], "Compass");
        else
            Menus_HideByName(&cgDC[localClientNum], "Compass_mp");
        cgameGlob->compassFadeTime = 0;
    }
}

void __cdecl CG_CheckHudStanceDisplay(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-Ch]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( (cgameGlob->nextSnap->ps.eFlags & 8) != 0 && (cgameGlob->nextSnap->ps.eFlags & 0x100) != 0
        || (cgameGlob->nextSnap->ps.eFlags & 4) != 0 && (cgameGlob->nextSnap->ps.eFlags & 0x200) != 0 )
    {
        CG_MenuShowNotify(localClientNum, 4);
    }
    if ( hud_fade_stance->current.value != 0.0
        && cgameGlob->stanceFadeTime
        && (float)(cgameGlob->time - cgameGlob->stanceFadeTime) > (float)(hud_fade_stance->current.value * 1000.0) )
    {
        if ( CL_LocalClient_GetActiveCount() == 1 )
            Menus_HideByName(&cgDC[localClientNum], "stance");
        else
            Menus_HideByName(&cgDC[localClientNum], "stance_mp");
        cgameGlob->stanceFadeTime = 0;
    }
}

void __cdecl CG_CheckHudSprintDisplay(int localClientNum)
{
    cg_s *cgameGlob; // [esp+4h] [ebp-14h]
    int maxSprintTime; // [esp+Ch] [ebp-Ch]
    const playerState_s *ps; // [esp+10h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->nextSnap->ps;
    if ( cgameGlob->nextSnap->ps.pm_type != 9 )
    {
        maxSprintTime = BG_GetMaxSprintTime(ps);
        if ( PM_GetSprintLeft(ps, cgameGlob->time) < maxSprintTime )
            CG_MenuShowNotify(localClientNum, 7);
    }
    if ( ps->pm_type != 9
        && cgameGlob->predictedPlayerState.sprintState.lastSprintStart > cgameGlob->predictedPlayerState.sprintState.lastSprintEnd )
    {
        CG_MenuShowNotify(localClientNum, 7);
    }
    if ( hud_fade_sprint->current.value != 0.0
        && cgameGlob->sprintFadeTime
        && (float)(cgameGlob->time - cgameGlob->sprintFadeTime) > (float)(hud_fade_stance->current.value * 1000.0) )
    {
        if ( CL_LocalClient_GetActiveCount() == 1 )
            Menus_HideByName(&cgDC[localClientNum], "sprintMeter");
        else
            Menus_HideByName(&cgDC[localClientNum], "sprintMeter_mp");
        cgameGlob->sprintFadeTime = 0;
    }
}

void __cdecl CG_CheckHudOffHandDisplay(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( hud_fade_offhand->current.value != 0.0
        && cgameGlob->offhandFadeTime
        && (float)(cgameGlob->time - cgameGlob->offhandFadeTime) > (float)(hud_fade_offhand->current.value * 1000.0) )
    {
        if ( CL_LocalClient_GetActiveCount() == 1 )
            Menus_HideByName(&cgDC[localClientNum], "offhandinfo");
        else
            Menus_HideByName(&cgDC[localClientNum], "offhandinfo_mp");
        cgameGlob->offhandFadeTime = 0;
    }
}

void __cdecl CG_CheckHudObjectiveDisplay(int localClientNum)
{
    cg_s *LocalClientGlobals; // edx

    if ( CG_IsScoreboardDisplayed(localClientNum) )
    {
        LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
        if ( LocalClientGlobals->time - LocalClientGlobals->scoreFadeTime > 100 )
            Menus_HideByName(&cgDC[localClientNum], "objectiveinfo");
    }
}

void __cdecl CG_CheckVehicleControlsDisplay(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-Ch]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( hud_fade_vehiclecontrols->current.value != 0.0
        && cgameGlob->vehicleControlsFadeTime
        && (float)(cgameGlob->time - cgameGlob->vehicleControlsFadeTime) > (float)(hud_fade_vehiclecontrols->current.value
                                                                                                                                                         * 1000.0) )
    {
        Menus_HideByName(&cgDC[localClientNum], "RCBombHints");
        cgameGlob->vehicleControlsFadeTime = 0;
    }
}

void __cdecl CG_ScreenBurn(int localClientNum)
{
    if ( s_screenBurn[localClientNum].endTime )
    {
        if ( s_screenBurn[localClientNum].endTime < CG_GetLocalClientGlobals(localClientNum)->time )
            CG_ClearBurn(localClientNum);
    }
}

void __cdecl CG_ScreenElectrified(int localClientNum)
{
    if ( s_screenElectrified[localClientNum].endTime )
    {
        if ( s_screenElectrified[localClientNum].endTime < CG_GetLocalClientGlobals(localClientNum)->time )
            CG_ClearElectrified(localClientNum);
    }
}

void __cdecl CG_DrawMiniConsole(int localClientNum)
{
    if ( cg_minicon->current.enabled )
        Con_DrawMiniConsole(localClientNum, 2, 4, 1.0);
}

void __cdecl CG_DrawErrorMessages(int localClientNum)
{
    Con_DrawErrors(localClientNum, 2, 300, 1.0);
}

void __cdecl CG_DrawSay(int localClientNum)
{
    if ( !cg_hudSayPosition
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_draw_mp.cpp",
                    901,
                    0,
                    "%s",
                    "cg_hudSayPosition") )
    {
        __debugbreak();
    }
    Con_DrawSay(localClientNum, (int)cg_hudSayPosition->current.value, (int)cg_hudSayPosition->current.vector[1] + 24);
}

void __cdecl CG_DrawVote(int localClientNum)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    int v5; // [esp+Ch] [ebp-350h]
    int voteYes; // [esp+10h] [ebp-34Ch]
    const char *v7; // [esp+10h] [ebp-34Ch]
    int v8; // [esp+14h] [ebp-348h]
    const char *v9; // [esp+14h] [ebp-348h]
    char *scale; // [esp+18h] [ebp-344h]
    int scalea; // [esp+18h] [ebp-344h]
    int scaleb; // [esp+18h] [ebp-344h]
    Font_s *font; // [esp+20h] [ebp-33Ch]
    char szVoteYes[260]; // [esp+24h] [ebp-338h] BYREF
    const ScreenPlacement *scrPlace; // [esp+128h] [ebp-234h]
    const cg_s *cgameGlob; // [esp+12Ch] [ebp-230h]
    int iNumKeys; // [esp+130h] [ebp-22Ch]
    const cgs_t *cgs; // [esp+134h] [ebp-228h]
    int halign; // [esp+138h] [ebp-224h]
    int sec; // [esp+13Ch] [ebp-220h]
    const char *s; // [esp+140h] [ebp-21Ch]
    char szVoteNo[256]; // [esp+144h] [ebp-218h] BYREF
    char binding[260]; // [esp+244h] [ebp-118h] BYREF
    float fontHeight; // [esp+34Ch] [ebp-10h]
    float x; // [esp+350h] [ebp-Ch]
    float y; // [esp+354h] [ebp-8h]
    float fontScale; // [esp+358h] [ebp-4h]

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    if ( cgs->voteTime )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        scrPlace = &scrPlaceView[localClientNum];
        if ( !cg_hudVotePosition
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_draw_mp.cpp",
                        933,
                        0,
                        "%s",
                        "cg_hudVotePosition") )
        {
            __debugbreak();
        }
        if ( cgs->viewHeight <= 600 )
            fontHeight = 16.0f;
        else
            fontHeight = 10.0f;
        fontScale = fontHeight / 48.0;
        halign = 1;
        font = UI_GetFontHandle(scrPlace, 0, fontHeight / 48.0);
        x = cg_hudVotePosition->current.value;
        y = cg_hudVotePosition->current.vector[1] + fontHeight;
        iNumKeys = UI_GetKeyBindingLocalizedString(localClientNum, "vote yes", binding, 0);
        if ( iNumKeys )
            I_strncpyz(szVoteYes, binding, 256);
        else
            I_strncpyz(szVoteYes, "vote yes", 256);
        iNumKeys = UI_GetKeyBindingLocalizedString(localClientNum, "vote no", binding, 0);
        if ( iNumKeys )
            I_strncpyz(szVoteNo, binding, 256);
        else
            I_strncpyz(szVoteNo, "vote no", 256);
        sec = (cgs->voteTime - cgameGlob->time) / 1000;
        if ( sec < 0 )
            sec = 0;
        scale = (char*)cgs->voteString;
        v8 = sec;
        if ( (cgameGlob->nextSnap->ps.eFlags & 0x100000) != 0 )
        {
            v1 = UI_SafeTranslateString("CGAME_VOTE");
            s = va("%s(%i):%s", v1, v8, scale);
            UI_DrawText(scrPlace, (char *)s, 0x7FFFFFFF, font, x, y, 1, 1, fontScale, colorYellow, 3);
            y = y + fontHeight;
            scalea = cgs->voteNo;
            v9 = UI_SafeTranslateString("CGAME_NO");
            voteYes = cgs->voteYes;
            v2 = UI_SafeTranslateString("CGAME_YES");
            s = va("%s:%i, %s:%i", v2, voteYes, v9, scalea);
        }
        else
        {
            v3 = UI_SafeTranslateString("CGAME_VOTE");
            s = va("%s(%i):%s", v3, v8, scale);
            UI_DrawText(scrPlace, (char *)s, 0x7FFFFFFF, font, x, y, 1, 1, fontScale, colorYellow, 3);
            y = y + fontHeight;
            scaleb = cgs->voteNo;
            v7 = UI_SafeTranslateString("CGAME_NO");
            v5 = cgs->voteYes;
            v4 = UI_SafeTranslateString("CGAME_YES");
            s = va("%s(%s):%i, %s(%s):%i", v4, szVoteYes, v5, v7, szVoteNo, scaleb);
        }
        UI_DrawText(scrPlace, (char *)s, 0x7FFFFFFF, font, x, y, 1, 1, fontScale, colorYellow, 3);
    }
}

char __cdecl CG_DrawGenericOverlay(int SortIndex, int localClientNum)
{
    unsigned __int16 CodeConst_GenericParamA; // ax
    unsigned __int16 CodeConst_GenericParamB; // ax
    unsigned __int16 CodeConst_GenericParam0; // ax
    unsigned __int16 CodeConst_GenericParam1; // ax
    unsigned __int16 v6; // ax
    unsigned __int16 v7; // ax
    unsigned __int16 v8; // ax
    unsigned __int16 v9; // ax
    unsigned __int16 CodeConst_GenericParam2; // ax
    unsigned __int16 CodeConst_GenericParam3; // ax
    unsigned __int16 v12; // ax
    unsigned __int16 v13; // ax
    float fOverlayAlphaScale; // [esp+30h] [ebp-120h]
    float v16[4]; // [esp+38h] [ebp-118h] BYREF
    float v17[4]; // [esp+48h] [ebp-108h] BYREF
    float v18[4]; // [esp+58h] [ebp-F8h] BYREF
    float v19[4]; // [esp+68h] [ebp-E8h] BYREF
    float c3[4]; // [esp+78h] [ebp-D8h] BYREF
    float v21; // [esp+88h] [ebp-C8h] BYREF
    float v22; // [esp+8Ch] [ebp-C4h] BYREF
    float v23; // [esp+90h] [ebp-C0h] BYREF
    float c0[4]; // [esp+94h] [ebp-BCh] BYREF
    float v25; // [esp+A4h] [ebp-ACh] BYREF
    int iOverlay; // [esp+A8h] [ebp-A8h]
    float v27[4]; // [esp+ACh] [ebp-A4h] BYREF
    float vec[4]; // [esp+BCh] [ebp-94h] BYREF
    float c2[4]; // [esp+CCh] [ebp-84h] BYREF
    float c1[4]; // [esp+DCh] [ebp-74h] BYREF
    float v31; // [esp+ECh] [ebp-64h] BYREF
    float v32; // [esp+F0h] [ebp-60h] BYREF
    float v33; // [esp+F4h] [ebp-5Ch] BYREF
    float v34; // [esp+F8h] [ebp-58h] BYREF
    float cb[4]; // [esp+FCh] [ebp-54h] BYREF
    float ca[4]; // [esp+10Ch] [ebp-44h] BYREF
    const WeaponVariantDef *weapVariantDef; // [esp+11Ch] [ebp-34h]
    int weapIndex; // [esp+120h] [ebp-30h]
    float x; // [esp+124h] [ebp-2Ch] BYREF
    float y; // [esp+128h] [ebp-28h] BYREF
    float color[4]; // [esp+12Ch] [ebp-24h] BYREF
    float h; // [esp+13Ch] [ebp-14h] BYREF
    float zoomFrac; // [esp+140h] [ebp-10h] BYREF
    float alphaScale; // [esp+144h] [ebp-Ch]
    float w; // [esp+148h] [ebp-8h] BYREF
    cg_s *cgameGlob; // [esp+14Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if (CG_IsTvguided(localClientNum, 1) && cgameGlob->genericMaterialMap[2] && !SortIndex)
    {
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            0.0,
            0.0,
            640.0,
            480.0,
            4,
            4,
            0.0,
            0.0,
            1.0,
            1.0,
            0,
            cgameGlob->genericMaterialMap[2]);
    }
    else if (CG_IsInfrared(localClientNum) && cgameGlob->genericMaterialMap[1] && !SortIndex)
    {
        x = cgameGlob->genericOverlayParamADS[0];
        y = cgameGlob->genericOverlayParamADS[1];
        w = cgameGlob->genericOverlayParamADS[2];
        h = cgameGlob->genericOverlayParamADS[3];
        ScrPlace_ApplyRect(&scrPlaceView[localClientNum], &x, &y, &w, &h, 2, 2);
        ca[0] = scrPlaceView[localClientNum].realViewportSize[0];
        ca[1] = scrPlaceView[localClientNum].realViewportSize[1];
        ca[2] = 0.0f;
        ca[3] = 0.0f;
        cb[0] = x / scrPlaceView[localClientNum].realViewportSize[0];
        cb[1] = y / scrPlaceView[localClientNum].realViewportSize[1];
        cb[2] = (float)(x + w) / scrPlaceView[localClientNum].realViewportSize[0];
        cb[3] = (float)(y + h) / scrPlaceView[localClientNum].realViewportSize[1];
        CodeConst_GenericParamA = GetCodeConst_GenericParamA();
        CL_SetCustomConstant(CodeConst_GenericParamA, ca);
        CodeConst_GenericParamB = GetCodeConst_GenericParamB();
        CL_SetCustomConstant(CodeConst_GenericParamB, cb);
        weapIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
        weapVariantDef = BG_GetWeaponVariantDef(weapIndex);
        CG_GetWeapReticleZoom(cgameGlob, &zoomFrac);
        if (weapVariantDef->fOverlayAlphaScale >= 1.0)
            fOverlayAlphaScale = weapVariantDef->fOverlayAlphaScale;
        else
            fOverlayAlphaScale = 1.0f;
        alphaScale = fOverlayAlphaScale;
        color[0] = 1.0f;
        color[1] = 1.0f;
        color[2] = 1.0f;
        color[3] = (float)(zoomFrac / fOverlayAlphaScale) * (float)(zoomFrac / fOverlayAlphaScale);
        CL_DrawStretchPicPhysical(x, y, w, h, 0.0, 0.0, 1.0, 1.0, color, cgameGlob->genericMaterialMap[1]);
    }
    else if (cgameGlob->genericOverlayEnableADS && cgameGlob->genericMaterialMap[0] && !SortIndex)
    {
        v31 = cgameGlob->genericOverlayParamADS[0];
        v32 = cgameGlob->genericOverlayParamADS[1];
        v34 = cgameGlob->genericOverlayParamADS[2];
        v33 = cgameGlob->genericOverlayParamADS[3];
        ScrPlace_ApplyRect(&scrPlaceView[localClientNum], &v31, &v32, &v34, &v33, 2, 2);
        memset(c1, 0, sizeof(c1));
        c2[0] = cgameGlob->genericOverlayParamADS[4];
        c2[1] = cgameGlob->genericOverlayParamADS[5];
        c2[2] = cgameGlob->genericOverlayParamADS[6];
        c2[3] = cgameGlob->genericOverlayParamADS[7];
        vec[0] = scrPlaceView[localClientNum].realViewportSize[0];
        vec[1] = scrPlaceView[localClientNum].realViewportSize[1];
        vec[2] = 0.0f;
        vec[3] = 0.0f;
        v27[0] = v31 / scrPlaceView[localClientNum].realViewportSize[0];
        v27[1] = v32 / scrPlaceView[localClientNum].realViewportSize[1];
        v27[2] = (float)(v31 + v34) / scrPlaceView[localClientNum].realViewportSize[0];
        v27[3] = (float)(v32 + v33) / scrPlaceView[localClientNum].realViewportSize[1];
        CodeConst_GenericParam0 = GetCodeConst_GenericParam0();
        CL_SetCustomConstant(CodeConst_GenericParam0, c1);
        CodeConst_GenericParam1 = GetCodeConst_GenericParam1();
        CL_SetCustomConstant(CodeConst_GenericParam1, c2);
        v6 = GetCodeConst_GenericParamA();
        CL_SetCustomConstant(v6, vec);
        v7 = GetCodeConst_GenericParamB();
        CL_SetCustomConstant(v7, v27);
        CL_DrawStretchPicPhysical(v31, v32, v34, v33, 0.0, 0.0, 1.0, 1.0, 0, cgameGlob->genericMaterialMap[0]);
        cgameGlob->genericOverlayEnableADS = 0;
    }
    for (iOverlay = 0; iOverlay < 3; ++iOverlay)
    {
        if (cgameGlob->genericOverlayEnabled[iOverlay]
            && cgameGlob->genericOverlayMaterial[iOverlay]
            && SortIndex == cgameGlob->genericOverlaySortIndex[iOverlay])
        {
            v21 = 0.0f;
            v22 = 0.0f;
            v25 = 640.0f;
            v23 = 480.0f;
            ScrPlace_ApplyRect(&scrPlaceView[localClientNum], &v21, &v22, &v25, &v23, 4, 4);
            c0[0] = cgameGlob->genericOverlayParam[iOverlay][0];
            c0[1] = cgameGlob->genericOverlayParam[iOverlay][1];
            c0[2] = cgameGlob->genericOverlayParam[iOverlay][2];
            c0[3] = cgameGlob->genericOverlayParam[iOverlay][3];
            v19[0] = cgameGlob->genericOverlayParam[iOverlay][4];
            v19[1] = cgameGlob->genericOverlayParam[iOverlay][5];
            v19[2] = cgameGlob->genericOverlayParam[iOverlay][6];
            v19[3] = cgameGlob->genericOverlayParam[iOverlay][7];
            v18[0] = cgameGlob->genericOverlayParam[iOverlay][8];
            v18[1] = cgameGlob->genericOverlayParam[iOverlay][9];
            v18[2] = cgameGlob->genericOverlayParam[iOverlay][10];
            v18[3] = cgameGlob->genericOverlayParam[iOverlay][11];
            c3[0] = cgameGlob->genericOverlayParam[iOverlay][12];
            c3[1] = cgameGlob->genericOverlayParam[iOverlay][13];
            c3[2] = cgameGlob->genericOverlayParam[iOverlay][14];
            c3[3] = cgameGlob->genericOverlayParam[iOverlay][15];
            v17[0] = scrPlaceView[localClientNum].realViewportSize[0];
            v17[1] = scrPlaceView[localClientNum].realViewportSize[1];
            v17[2] = 0.0f;
            v17[3] = 0.0f;
            v16[0] = v21 / scrPlaceView[localClientNum].realViewportSize[0];
            v16[1] = v22 / scrPlaceView[localClientNum].realViewportSize[1];
            v16[2] = (float)(v21 + v25) / scrPlaceView[localClientNum].realViewportSize[0];
            v16[3] = (float)(v22 + v23) / scrPlaceView[localClientNum].realViewportSize[1];
            v8 = GetCodeConst_GenericParam0();
            CL_SetCustomConstant(v8, c0);
            v9 = GetCodeConst_GenericParam1();
            CL_SetCustomConstant(v9, v19);
            CodeConst_GenericParam2 = GetCodeConst_GenericParam2();
            CL_SetCustomConstant(CodeConst_GenericParam2, v18);
            CodeConst_GenericParam3 = GetCodeConst_GenericParam3();
            CL_SetCustomConstant(CodeConst_GenericParam3, c3);
            v12 = GetCodeConst_GenericParamA();
            CL_SetCustomConstant(v12, v17);
            v13 = GetCodeConst_GenericParamB();
            CL_SetCustomConstant(v13, v16);
            CL_DrawStretchPicPhysical(v21, v22, v25, v23, 0.0, 0.0, 1.0, 1.0, 0, cgameGlob->genericOverlayMaterial[iOverlay]);
        }
    }
    return 1;
}

void __cdecl DrawIntermission(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    if ( UI_GetActiveMenu(localClientNum) == UIMENU_SCRIPT_POPUP && UI_GetTopActiveMenuName(localClientNum) )
    {
        CG_DrawScoreboard(localClientNum);
        CG_DrawChatMessages(localClientNum);
    }
    else
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        if ( ui_showEndOfGame->current.enabled )
        {
            if ( UI_GetActiveMenu(localClientNum) != UIMENU_ENDOFGAME )
                UI_SetActiveMenu(localClientNum, UIMENU_ENDOFGAME);
        }
        else
        {
            cgameGlob->showScores = 1;
            cgameGlob->scoreFadeTime = cgameGlob->time;
            if ( UI_GetActiveMenu(localClientNum) != UIMENU_SCOREBOARD
                && UI_GetActiveMenu(localClientNum) != UIMENU_GAMERCARD
                && UI_GetActiveMenu(localClientNum) != UIMENU_MUTEERROR
                && !ui_showEndOfGame->current.enabled )
            {
                UI_SetActiveMenu(localClientNum, UIMENU_SCOREBOARD);
            }
            if ( UI_GetActiveMenu(localClientNum) == UIMENU_SCOREBOARD )
                CG_DrawScoreboard(localClientNum);
        }
        CG_DrawChatMessages(localClientNum);
    }
}

void __cdecl CG_DrawSpectatorMessage(int localClientNum)
{
    const char *BoundSpectatorCommand; // eax
    const char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    const char *v5; // [esp+1Ch] [ebp-194h]
    char *str; // [esp+20h] [ebp-190h]
    Font_s *font; // [esp+24h] [ebp-18Ch]
    float lineHeight; // [esp+2Ch] [ebp-184h]
    const char *followPrev[4]; // [esp+34h] [ebp-17Ch] BYREF
    const cg_s *cgameGlob; // [esp+44h] [ebp-16Ch]
    int vertAlign; // [esp+48h] [ebp-168h]
    const char *followStop[2]; // [esp+4Ch] [ebp-164h] BYREF
    int lineNum; // [esp+54h] [ebp-15Ch]
    const char *commands[7]; // [esp+58h] [ebp-158h]
    int horzAlign; // [esp+74h] [ebp-13Ch]
    const char *messages[7]; // [esp+78h] [ebp-138h]
    int i; // [esp+94h] [ebp-11Ch]
    char binding[260]; // [esp+98h] [ebp-118h] BYREF
    float x; // [esp+1A0h] [ebp-10h]
    float y; // [esp+1A4h] [ebp-Ch]
    float fontScale; // [esp+1A8h] [ebp-8h]
    const playerState_s *ps; // [esp+1ACh] [ebp-4h]

    followPrev[0] = "+toggleads_throw";
    followPrev[1] = "+speed_throw";
    followPrev[2] = "+speed";
    followPrev[3] = "toggleads";
    followStop[0] = "+melee";
    followStop[1] = "+melee_breath";
    if ( cg_descriptiveText->current.enabled && !Key_IsCatcherActive(localClientNum, 16) )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        if ( !cgameGlob->nextSnap
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_draw_mp.cpp",
                        1200,
                        0,
                        "%s",
                        "cgameGlob->nextSnap") )
        {
            __debugbreak();
        }
        ps = &cgameGlob->nextSnap->ps;
        if ( (ps->otherFlags & 0x18) != 0 )
        {
            fontScale = 0.20833333f;
            font = UI_GetFontHandle(&scrPlaceView[localClientNum], 0, 0.20833333);
            lineNum = 0;
            lineHeight = (float)UI_TextHeight(font, 0.20833333);
            horzAlign = 0;
            vertAlign = 0;
            if ( (ps->otherFlags & 2) != 0 )
            {
                if ( Dvar_GetBool("gpad_enabled") )
                    commands[lineNum] = "weapnext";
                else
                    commands[lineNum] = "toggleView";
                if ( cgameGlob->renderingThirdPerson == TP_FOR_MODEL )
                    v5 = "PLATFORM_FIRSTPERSON_PC";
                else
                    v5 = "PLATFORM_THIRDPERSON_PC";
                messages[lineNum++] = v5;
            }
            if ( (ps->otherFlags & 8) != 0 )
            {
                commands[lineNum] = "+attack";
                messages[lineNum++] = "PLATFORM_FOLLOWNEXTPLAYER";
                BoundSpectatorCommand = CG_GetBoundSpectatorCommand(localClientNum, followPrev, 4);
                commands[lineNum] = BoundSpectatorCommand;
                messages[lineNum++] = "PLATFORM_FOLLOWPREVIOUSPLAYER";
            }
            if ( (ps->otherFlags & 0x10) != 0 )
            {
                v2 = CG_GetBoundSpectatorCommand(localClientNum, followStop, 2);
                commands[lineNum] = v2;
                messages[lineNum++] = "PLATFORM_FOLLOWSTOP";
            }
            x = 240.0f;
            y = 470.0 - (float)((float)lineNum * lineHeight);
            if ( lineNum > 7
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_draw_mp.cpp",
                            1344,
                            0,
                            "%s",
                            "lineNum <= MAX_SPECTATOR_MSG_LINES") )
            {
                __debugbreak();
            }
            for ( i = 0; i < lineNum; ++i )
            {
                if ( !UI_GetKeyBindingLocalizedString(localClientNum, commands[i], binding, 0) )
                {
                    v3 = UI_SafeTranslateString("KEY_UNBOUND");
                    I_strncpyz(binding, v3, 256);
                }
                v4 = UI_SafeTranslateString(messages[i]);
                str = UI_ReplaceConversionString(v4, binding);
                UI_DrawText(
                    &scrPlaceView[localClientNum],
                    str,
                    0x7FFFFFFF,
                    font,
                    x,
                    y,
                    horzAlign,
                    vertAlign,
                    fontScale,
                    colorWhite,
                    3);
                y = y + lineHeight;
            }
        }
    }
}

const char *__cdecl CG_GetBoundSpectatorCommand(int localClientNum, const char **choices, int choiceCount)
{
    int i; // [esp+0h] [ebp-10Ch]
    char binding[260]; // [esp+4h] [ebp-108h] BYREF

    for ( i = 0; i < choiceCount; ++i )
    {
        if ( !choices[i]
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_draw_mp.cpp", 1154, 0, "%s", "choices[i]") )
        {
            __debugbreak();
        }
        if ( UI_GetKeyBindingLocalizedString(localClientNum, choices[i], binding, 0) )
            return choices[i];
    }
    if ( choiceCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_draw_mp.cpp",
                    1159,
                    0,
                    "%s",
                    "choiceCount > 0") )
    {
        __debugbreak();
    }
    return *choices;
}

void __cdecl CG_UpdatePlayerNames(int localClientNum)
{
    CG_UpdatePlayerNamesInternal(localClientNum);
}

void __cdecl CG_UpdatePlayerNamesInternal(int localClientNum)
{
    PROF_SCOPED("CG_UpdatePlayerNames");

    if ( CG_AreAllPlayerNamesVisible() )
    {
        CG_DrawVisibleNames(localClientNum);
    }
    else
    {
        CG_DrawCrosshairNames(localClientNum);
        CG_DrawFriendlyNames(localClientNum);
        CG_DrawNames(localClientNum);
    }
}

void __cdecl DrawViewmodelInfo(int localClientNum)
{
    const char *v1; // eax
    const char *v2; // eax
    double fWeaponPosFrac; // [esp+Ch] [ebp-884h]
    double v4; // [esp+Ch] [ebp-884h]
    const char *v5; // [esp+2Ch] [ebp-864h]
    const char *v6; // [esp+30h] [ebp-860h]
    const char *v7; // [esp+34h] [ebp-85Ch]
    const char *v8; // [esp+38h] [ebp-858h]
    const char *v9; // [esp+3Ch] [ebp-854h]
    const char *v10; // [esp+40h] [ebp-850h]
    const char *v11; // [esp+44h] [ebp-84Ch]
    const char *v12; // [esp+48h] [ebp-848h]
    const char *name; // [esp+4Ch] [ebp-844h]
    weaponInfo_s *weapInfo; // [esp+5Ch] [ebp-834h]
    char buffer[2048]; // [esp+60h] [ebp-830h] BYREF
    Font_s *font; // [esp+864h] [ebp-2Ch]
    const ScreenPlacement *scrPlace; // [esp+868h] [ebp-28h]
    XModel *weaponMdl; // [esp+86Ch] [ebp-24h]
    const cg_s *cgameGlob; // [esp+870h] [ebp-20h]
    const char *weaponMdlName; // [esp+874h] [ebp-1Ch]
    int len; // [esp+878h] [ebp-18h]
    ViewModelInfo *viewModelInfo; // [esp+87Ch] [ebp-14h]
    int weaponIndex; // [esp+880h] [ebp-10h]
    float fov; // [esp+884h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+888h] [ebp-8h]
    const playerState_s *ps; // [esp+88Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    weaponIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
    scrPlace = &scrPlaceView[localClientNum];
    font = UI_GetFontHandle(scrPlace, 1, 0.25);
    if ( weaponIndex > 0 )
    {
        viewModelInfo = CG_GetLocalClientViewModelInfo(localClientNum);
        weapInfo = CG_GetLocalClientWeaponInfo(localClientNum, weaponIndex);
        weapDef = BG_GetWeaponDef(weaponIndex);
        weaponMdl = weapDef->gunXModel[BG_GetPlayerWeaponModel(ps, weaponIndex)];
        if ( weaponMdl )
            name = weaponMdl->name;
        else
            name = 0;
        weaponMdlName = name;
        fov = CG_GetViewFov(localClientNum);
        if ( weapDef->bDualWield )
        {
            if ( weapInfo->rocketModel )
                v12 = weapInfo->rocketModel->name;
            else
                v12 = "none";
            if ( weapInfo->gogglesModel )
                v11 = weapInfo->gogglesModel->name;
            else
                v11 = "none";
            if ( weapInfo->handModel )
                v10 = weapInfo->handModel->name;
            else
                v10 = "none";
            if ( weaponMdlName )
                v9 = weaponMdlName;
            else
                v9 = "none";
            fWeaponPosFrac = ps->fWeaponPosFrac;
            v1 = BG_WeaponName(weaponIndex);
            Com_sprintf(
                buffer,
                0x800u,
                "^7Left Hand:    ^6%s\n"
                "^7Right Hand: ^6%s\n"
                "^7Weapon: ^2%s^7 - ^5%s\n"
                "^7Hands: ^5%s\n"
                "^7Goggles: ^5%s\n"
                "^7Rocket: ^5%s\n"
                "^7ADS: ^5%.2f ^7-^5 %.0f^7fov\n"
                "^7---Anims---\n"
                "^3",
                WeaponStateNames[ps->weaponstateLeft],
                WeaponStateNames[ps->weaponstate],
                v1,
                v9,
                v10,
                v11,
                v12,
                fWeaponPosFrac,
                fov);
        }
        else
        {
            if ( weapInfo->rocketModel )
                v8 = weapInfo->rocketModel->name;
            else
                v8 = "none";
            if ( weapInfo->gogglesModel )
                v7 = weapInfo->gogglesModel->name;
            else
                v7 = "none";
            if ( weapInfo->handModel )
                v6 = weapInfo->handModel->name;
            else
                v6 = "none";
            if ( weaponMdlName )
                v5 = weaponMdlName;
            else
                v5 = "none";
            v4 = ps->fWeaponPosFrac;
            v2 = BG_WeaponName(weaponIndex);
            Com_sprintf(
                buffer,
                0x800u,
                "^6%s\n"
                "^7Weapon: ^2%s^7 - ^5%s\n"
                "^7Hands: ^5%s\n"
                "^7Goggles: ^5%s\n"
                "^7Rocket: ^5%s\n"
                "^7ADS: ^5%.2f ^7-^5 %.0f^7fov\n"
                "^7---Anims---\n"
                "^3",
                WeaponStateNames[ps->weaponstate],
                v2,
                v5,
                v6,
                v7,
                v8,
                v4,
                fov);
        }
        len = &buffer[strlen(buffer) + 1] - &buffer[1];
        DObjDisplayAnimToBuffer(viewModelInfo->viewModelDObj, "", &buffer[len], 2048 - len);
        UI_DrawText(scrPlace, buffer, 2048, font, 0.0, 20.0, 1, 1, 0.25, colorWhite, 3);
    }
}

//weaponInfo_s *__cdecl CG_GetLocalClientWeaponInfo(int localClientNum, int weaponIndex)
//{
//    if ( localClientNum
//        && !Assert_MyHandler(
//                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_local.h",
//                    224,
//                    0,
//                    "%s\n\t(localClientNum) = %i",
//                    "(localClientNum == 0)",
//                    localClientNum) )
//    {
//        __debugbreak();
//    }
//    return &cg_weaponsArray[0][weaponIndex];
//}

void __cdecl CG_DrawActive(int localClientNum)
{
    int LastWeaponForAlt; // eax
    int i; // [esp+4h] [ebp-20h]
    cg_s *cgameGlob; // [esp+10h] [ebp-14h]
    float angles[3]; // [esp+14h] [ebp-10h] BYREF
    float fovSensitivityScale; // [esp+20h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    fovSensitivityScale = cgameGlob->zoomSensitivity;
    if ( cgameGlob->shellshock.sensitivity != 0.0 )
        fovSensitivityScale = fovSensitivityScale * cgameGlob->shellshock.sensitivity;
    CL_SetFOVSensitivityScale(localClientNum, fovSensitivityScale);
    angles[0] = cgameGlob->kickAngles[0] + cgameGlob->offsetAngles[0];
    angles[1] = cgameGlob->kickAngles[1] + cgameGlob->offsetAngles[1];
    angles[2] = cgameGlob->kickAngles[2] + cgameGlob->offsetAngles[2];
    CL_SetUserCmdAimValues(localClientNum, angles);
    BG_AssertOffhandIndexOrNone(cgameGlob->equippedOffHand);
    LastWeaponForAlt = CG_GetLastWeaponForAlt(cgameGlob, &cgameGlob->predictedPlayerState, cgameGlob->weaponSelect);
    CL_SetUserCmdWeapons(localClientNum, cgameGlob->weaponSelect, cgameGlob->equippedOffHand, LastWeaponForAlt);
    CL_SetExtraButtons(localClientNum, &cgameGlob->extraButton_bits);
    for ( i = 0; i < 2; ++i )
        cgameGlob->extraButton_bits.array[i] = 0;
    cgameGlob->refdef.hideMatureContent = cg_blood->current.color[0] == 0;
    cgameGlob->refdef.splitscreen = 0;
    cgameGlob->refdef.playerTeleported = cgameGlob->playerTeleported;
    CL_RenderScene(&cgameGlob->refdef, cgameGlob->frametime);
}

void __cdecl CG_DrawMissileCam(int localClientNum)
{
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgameGlob->refdef.hideMatureContent = cg_blood->current.color[0] == 0;
    cgameGlob->refdef.splitscreen = 0;
    cgameGlob->refdef.playerTeleported = cgameGlob->playerTeleported;
    cgameGlob->refdef.preExtraCamVieworg[0] = cgameGlob->refdef.vieworg[0];
    cgameGlob->refdef.preExtraCamVieworg[1] = cgameGlob->refdef.vieworg[1];
    cgameGlob->refdef.preExtraCamVieworg[2] = cgameGlob->refdef.vieworg[2];
    AxisCopy(cgameGlob->refdef.viewaxis, cgameGlob->refdef.preExtraCamViewaxis);
    cgameGlob->refdef.preExtraCamTanHalfFovX = cgameGlob->refdef.tanHalfFovX;
    cgameGlob->refdef.preExtraCamTanHalfFovY = cgameGlob->refdef.tanHalfFovY;
    cgameGlob->refdef.extraCamLargeFrame = 0;
    CL_RenderMissileCam(&cgameGlob->refdef, cgameGlob->frametime);
}

void __cdecl CG_AddSceneTracerBeams(int localClientNum)
{
    PROF_SCOPED("CG_AddSceneTracerBeams");
    CG_AddLocalEntityTracerBeams(localClientNum);
}

void __cdecl CG_GenerateSceneCodeMeshes(int localClientNum)
{
    RopeGenerateVertsCmd ropeVertsCmd; // [esp+14h] [ebp-48h] BYREF
    cg_s *clientGlobals; // [esp+54h] [ebp-8h]

    PROF_SCOPED("CG_GenerateSceneCodeMeshes");

    {
        PROF_SCOPED("Rope generate verts");

        ropeVertsCmd.localClientNum = localClientNum;
        ropeVertsCmd.ropeInfo = R_Rope_GetInfo();
        clientGlobals = CG_GetLocalClientGlobals(localClientNum);
        ropeVertsCmd.vieworg[0] = clientGlobals->refdef.vieworg[0];
        ropeVertsCmd.vieworg[1] = clientGlobals->refdef.vieworg[1];
        ropeVertsCmd.vieworg[2] = clientGlobals->refdef.vieworg[2];
        AxisCopy(clientGlobals->refdef.viewaxis, ropeVertsCmd.viewaxis);
        ropeVertsCmd.refdef_tanHalfFovX = clientGlobals->refdef.tanHalfFovX;
        ropeVertsCmd.refdef_tanHalfFovY = clientGlobals->refdef.tanHalfFovY;
        R_Rope_GenerateVerts_Camera(&ropeVertsCmd);
        //BLOPS_NULLSUB();
    }

    Flame_Generate_Verts(localClientNum);
}

void __cdecl CG_GenerateSceneVerts(int localClientNum)
{
    PROF_SCOPED("CG_GenerateSceneVerts");

    CG_AddAllPlayerSpriteDrawSurfs(localClientNum);
    CG_AddDrawSurfsFor3dHudElems(localClientNum);
    CG_RenderFire();
    CG_Flame_Render();
    CG_GenerateSceneCodeMeshes(localClientNum);
}

