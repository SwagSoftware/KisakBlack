#include "ui_main.h"
#include <universal/assertive.h>
#include <qcommon/com_clients.h>
#include <universal/com_files.h>
#include <qcommon/common.h>
#include <database/db_registry.h>
#include <gfx_d3d/r_font.h>
#include <universal/com_buildinfo.h>
#include <client_mp/cl_cgame_mp.h>
#include "ui_shared.h"
#include <gfx_d3d/r_rendercmds.h>
#include <client_mp/cl_main_mp.h>
#include <ui_mp/ui_main_mp.h>
#include "ui_server.h"
#include <win32/win_shared.h>
#include <client_mp/cl_scrn_mp.h>
#include "ui_utils.h"
#include "ui_main_pc.h"
#include "ui_atoms.h"
#include <cgame/cg_compass.h>
#include <client/cl_keys.h>
#include "ui_friends.h"
#include <client/cl_main.h>
#include <cgame/cg_camera.h>
#include <client/client.h>
#include <client/splitscreen.h>
#include "ui_localvars.h"
#include "ui_shared_obj.h"
#include <client_mp/cl_ui_pc_mp.h>
#include <live/live_leaderboard.h>
#include <client_mp/cl_main_pc_mp.h>
#include "ui_feeders.h"
#include <live/live_groups_dw.h>
#include <game_mp/ui_gameinfo_mp.h>
#include <client_mp/cl_input_mp.h>
#include <win32/win_voice.h>
#include <client/cl_voice.h>
#include <cgame_mp/cg_consolecmds_mp.h>
#include <cgame_mp/cg_newDraw_mp.h>
#include <qcommon/com_profilemapload.h>
#include "ui_emblem.h"
#include "ui_playlists.h"
#include <qcommon/com_gamemodes.h>
#include <client_mp/cl_ui_mp.h>
#include <live/live_clans.h>
#include <live/live_stats.h>
#include <client/cl_rank.h>
#include <live/live_ticker.h>
#include <database/db_file_load.h>
#include <sound/snd_driver_xaudio2.h>
#include <game_mp/g_main_mp.h>
#include <live/live_win.h>
#include <stringed/stringed_hooks.h>

const dvar_t *ui_netGameType;
const dvar_t *uiscript_debug;
const dvar_t *ui_animSpeedScale;
const dvar_t *ui_customModeIndex;
const dvar_t *ui_customModeName;
const dvar_t *ui_customModeEditName;
const dvar_t *ui_customModeEditDesc;
const dvar_t *ui_customModeDesc;
const dvar_t *ui_customClassName;
const dvar_t *g_allowvote;
//const dvar_t *cg_brass;
//const dvar_t *fx_marks;
const dvar_t *server1;
const dvar_t *server2;
const dvar_t *server3;
const dvar_t *server4;
const dvar_t *server5;
const dvar_t *server6;
const dvar_t *server7;
const dvar_t *server8;
const dvar_t *server9;
const dvar_t *server10;
const dvar_t *server11;
const dvar_t *server12;
const dvar_t *server13;
const dvar_t *server14;
const dvar_t *server15;
const dvar_t *server16;
const dvar_t *ui_netSource;
const dvar_t *ui_smallFont;
const dvar_t *ui_bigFont;
const dvar_t *ui_extraBigFont;
const dvar_t *ui_currentMap;
const dvar_t *ui_gametype;
const dvar_t *ui_mapname;
const dvar_t *ui_demoname;
const dvar_t *ui_currentRecentPlayer;
const dvar_t *ui_serverStatusTimeOut;
const dvar_t *ui_buildLocation;
const dvar_t *ui_buildSize;
const dvar_t *ui_showList;
const dvar_t *ui_showMenuOnly;
const dvar_t *ui_menuLvlNotify;
const dvar_t *selectedFriendName;
const dvar_t *selectedFriendClanTag;
const dvar_t *selectedPlayerClanName;
const dvar_t *selectedFriendIndex;
const dvar_t *selectedStoreItemIndex;
const dvar_t *selectedGroupIndex;
const dvar_t *selectedPlayerXuid;
const dvar_t *selectedMenuItemIndex;
const dvar_t *highlightedPlayerXuid;
const dvar_t *highlightedPlayerName;
const dvar_t *highlightedPlayerClanTag;
const dvar_t *splitscreen_lobbyPlayerCount;
const dvar_t *splitscreen_partyPlayerCount;
const dvar_t *tickerHeaderWidth;
const dvar_t *ui_borderLowLightScale;
const dvar_t *ui_cinematicsTimestamp;
const dvar_t *ui_connectScreenTextGlowColor;
const dvar_t *ui_drawCrosshair;
const dvar_t *ui_onCloseArgs;
const dvar_t *notice_onEscArg;
const dvar_t *statusinfo_onEscArg;
const dvar_t *ui_ethernetLinkActive;
const dvar_t *ui_listboxIndex;
const dvar_t *ui_readingSaveDevice;
const dvar_t *ui_signedInToProfile;
const dvar_t *emblem_scroll_delay_first;
const dvar_t *emblem_scroll_delay_rest;

uiInfo_s uiInfoArray[1];
const serverFilter_s serverFilters[1];

bool g_ingameMenusLoaded[1];

const char *MonthAbbrev[12] =
{
  "EXE_MONTH_ABV_JANUARY",
  "EXE_MONTH_ABV_FEBRUARY",
  "EXE_MONTH_ABV_MARCH",
  "EXE_MONTH_ABV_APRIL",
  "EXE_MONTH_ABV_MAY",
  "EXE_MONTH_ABV_JUN",
  "EXE_MONTH_ABV_JULY",
  "EXE_MONTH_ABV_AUGUST",
  "EXE_MONTH_ABV_SEPTEMBER",
  "EXE_MONTH_ABV_OCTOBER",
  "EXE_MONTH_ABV_NOVEMBER",
  "EXE_MONTH_ABV_DECEMBER"
};


uiInfo_s *__cdecl UI_UIContext_GetInfo(int contextIndex)
{
    if ( contextIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                    343,
                    0,
                    "%s\n\t(contextIndex) = %i",
                    "(contextIndex == 0)",
                    contextIndex) )
    {
        __debugbreak();
    }
    return &uiInfoArray[contextIndex];
}

uiInfo_s *__cdecl UI_GetInfo(int localClientNum)
{
    int contextIndex; // [esp+0h] [ebp-4h]

    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                    350,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    contextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
    return UI_UIContext_GetInfo(contextIndex);
}

UILocalVarContext *__cdecl UI_UIContext_GetLocalVarsContext(int contextIndex)
{
    return &UI_UIContext_GetInfo(contextIndex)->uiDC.localVars;
}

const char *__cdecl UI_GetMonthAbbrev(unsigned int month)
{
    if ( month >= 0xC
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                    362,
                    0,
                    "month doesn't index ARRAY_COUNT( MonthAbbrev )\n\t%i not in [0, %i)",
                    month,
                    12) )
    {
        __debugbreak();
    }
    return MonthAbbrev[month];
}

const char **__cdecl UI_GetServerFilter(const char **filter, int filtera)
{
    const char *basedir; // edx

    if (filtera
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
            369,
            0,
            "filter doesn't index numServerFilters\n\t%i not in [0, %i)",
            filtera,
            1))
    {
        __debugbreak();
    }
    basedir = serverFilters[filtera].basedir;
    *filter = serverFilters[filtera].description;
    filter[1] = basedir;
    return filter;
}

bool __cdecl UI_KeysBypassMenu(int localClientNum)
{
    return cl_bypassMouseInput && cl_bypassMouseInput->current.enabled
            || UI_GetActiveMenu(localClientNum) == UIMENU_SCOREBOARD;
}

char *__cdecl UI_GetMenuBuffer(char *filename)
{
    if ( useFastFile->current.enabled )
        return (char *)GetMenuBuffer_FastFile(filename);
    else
        return GetMenuBuffer_LoadObj(filename);
}

char menuBuf2[32768];
char *__cdecl GetMenuBuffer_LoadObj(char *filename)
{
    int len; // [esp+0h] [ebp-8h]
    int f; // [esp+4h] [ebp-4h] BYREF

    len = FS_FOpenFileByMode(filename, &f, FS_READ);
    if ( f )
    {
        if ( len < 0x8000 )
        {
            FS_Read((unsigned __int8 *)menuBuf2, len, f);
            menuBuf2[len] = 0;
            FS_FCloseFile(f);
            return menuBuf2;
        }
        else
        {
            Com_PrintError(13, "menu file too large: %s is %i, max allowed is %i", filename, len, 0x8000);
            FS_FCloseFile(f);
            return 0;
        }
    }
    else
    {
        Com_PrintError(13, "menu file not found: %s, using default\n", filename);
        return 0;
    }
}

char *__cdecl GetMenuBuffer_FastFile(const char *filename)
{
    RawFile *rawfile; // [esp+4h] [ebp-4h]

    rawfile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, (char*)filename, 1, -1).rawfile;
    if ( rawfile )
        return (char*)rawfile->buffer;
    Com_PrintError(13, "menu file not found: %s, using default\n", filename);
    return 0;
}

void __cdecl UI_DrawBuildNumber(int contextIndex)
{
    const char *BuildName; // eax
    int BuildNumber; // [esp+14h] [ebp-14h]
    int scale; // [esp+18h] [ebp-10h]
    Font_s *fh; // [esp+1Ch] [ebp-Ch]
    int width; // [esp+20h] [ebp-8h]
    char *text; // [esp+24h] [ebp-4h]

    fh = UI_GetFontHandle(&scrPlaceView[contextIndex], 0, ui_buildSize->current.value);
    scale = Com_GetChangelist();
    BuildNumber = Com_GetBuildNumber();
    BuildName = Com_GetBuildName();
    text = va("%s.%i [CL:%d]", BuildName, BuildNumber, scale);
    width = UI_TextWidth(text, 64, fh, ui_buildSize->current.value);
    UI_DrawText(
        &scrPlaceView[contextIndex],
        text,
        64,
        fh,
        ui_buildLocation->current.value - (float)width,
        ui_buildLocation->current.vector[1],
        3,
        1,
        ui_buildSize->current.value,
        colorWhite,
        0);
}

void __cdecl UI_DrawSides(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                float size,
                const float *color)
{
    CL_DrawStretchPic(
        scrPlace,
        x,
        y,
        size,
        h,
        horzAlign,
        vertAlign,
        0.0,
        0.0,
        0.0,
        0.0,
        color,
        sharedUiInfo.assets.whiteMaterial);
    CL_DrawStretchPic(
        scrPlace,
        (float)(x + w) - size,
        y,
        size,
        h,
        horzAlign,
        vertAlign,
        0.0,
        0.0,
        0.0,
        0.0,
        color,
        sharedUiInfo.assets.whiteMaterial);
}

void __cdecl UI_DrawTopBottom(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                float size,
                const float *color)
{
    CL_DrawStretchPic(
        scrPlace,
        x,
        y,
        w,
        size,
        horzAlign,
        vertAlign,
        0.0,
        0.0,
        0.0,
        0.0,
        color,
        sharedUiInfo.assets.whiteMaterial);
    CL_DrawStretchPic(
        scrPlace,
        x,
        (float)(y + h) - size,
        w,
        size,
        horzAlign,
        vertAlign,
        0.0,
        0.0,
        0.0,
        0.0,
        color,
        sharedUiInfo.assets.whiteMaterial);
}

void __cdecl UI_DrawRect(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float width,
                float height,
                int horzAlign,
                int vertAlign,
                float size,
                const float *color)
{
    UI_DrawTopBottom(scrPlace, x, y, width, height, horzAlign, vertAlign, size, color);
    UI_DrawSides(scrPlace, x, y + size, width, height - (float)(size * 2.0), horzAlign, vertAlign, size, color);
}

void __cdecl UI_DrawHighlightRect(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                float size,
                const float *hiColor,
                const float *loColor)
{
    float v10; // [esp+B0h] [ebp-30h] BYREF
    float dummyY; // [esp+B4h] [ebp-2Ch] BYREF
    float verts[4][2]; // [esp+B8h] [ebp-28h] BYREF
    float dummyX; // [esp+D8h] [ebp-8h] BYREF
    float dy; // [esp+DCh] [ebp-4h] BYREF

    ScrPlace_ApplyRect(scrPlace, &x, &y, &w, &h, horzAlign, vertAlign);
    dummyX = 0.0f;
    dummyY = 0.0f;
    v10 = size;
    dy = size;
    ScrPlace_ApplyRect(scrPlace, &dummyX, &dummyY, &v10, &dy, horzAlign, vertAlign);
    *(_QWORD *)&verts[0][0] = __PAIR64__(LODWORD(y), LODWORD(x));
    verts[1][0] = x + w;
    verts[1][1] = y;
    verts[2][0] = (float)(x + w) - v10;
    verts[2][1] = y + dy;
    verts[3][0] = x + v10;
    verts[3][1] = y + dy;
    R_AddCmdDrawQuadPic(verts, hiColor, sharedUiInfo.assets.whiteMaterial);
    *(_QWORD *)&verts[0][0] = __PAIR64__(LODWORD(y), LODWORD(x));
    verts[1][0] = x + v10;
    verts[1][1] = y + dy;
    verts[2][0] = x + v10;
    verts[2][1] = (float)(y + h) - dy;
    verts[3][0] = x;
    verts[3][1] = y + h;
    R_AddCmdDrawQuadPic(verts, hiColor, sharedUiInfo.assets.whiteMaterial);
    verts[0][0] = x;
    verts[0][1] = y + h;
    verts[1][0] = x + v10;
    verts[1][1] = (float)(y + h) - dy;
    verts[2][0] = (float)(x + w) - v10;
    verts[2][1] = verts[1][1];
    verts[3][0] = x + w;
    verts[3][1] = y + h;
    R_AddCmdDrawQuadPic(verts, loColor, sharedUiInfo.assets.whiteMaterial);
    verts[0][0] = x + w;
    verts[0][1] = y;
    verts[1][0] = x + w;
    verts[1][1] = y + h;
    verts[2][0] = (float)(x + w) - v10;
    verts[2][1] = (float)(y + h) - dy;
    verts[3][0] = verts[2][0];
    verts[3][1] = y + dy;
    R_AddCmdDrawQuadPic(verts, loColor, sharedUiInfo.assets.whiteMaterial);
}

int __cdecl UI_TextWidth(const char *text, int maxChars, Font_s *font, float scale)
{
    float v4; // xmm0_4
    float actualScale; // [esp+10h] [ebp-4h]

    actualScale = R_NormalizedTextScale(font, scale);
    v4 = (float)((float)R_TextWidth(text, maxChars, font) * actualScale) + 0.5;
    return (int)(float)floor(v4);
}

int __cdecl UI_TextHeight(Font_s *font, float scale)
{
    float v2; // xmm0_4
    float actualScale; // [esp+10h] [ebp-4h]

    actualScale = R_NormalizedTextScale(font, scale);
    v2 = (float)((float)R_TextHeight(font) * actualScale) + 0.5;
    return (int)(float)floor(v2);
}

void __cdecl UI_DrawText(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                int horzAlign,
                int vertAlign,
                float scale,
                const float *color,
                int style)
{
    float v11; // [esp+1Ch] [ebp-18h]
    float v12; // [esp+20h] [ebp-14h]
    float xScale; // [esp+2Ch] [ebp-8h] BYREF
    float yScale; // [esp+30h] [ebp-4h] BYREF

    xScale = R_NormalizedTextScale(font, scale);
    yScale = xScale;
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    v12 = floor((float)(x + 0.5));
    x = v12;
    v11 = floor((float)(y + 0.5));
    y = v11;
    CL_DrawTextPhysical(text, maxChars, font, x, v11, 1.0, xScale, yScale, color, style);
}

void __cdecl UI_DrawTextRotated(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                int horzAlign,
                int vertAlign,
                float scale,
                const float *color,
                int style,
                float rotation)
{
    float xScale; // [esp+24h] [ebp-8h]

    xScale = R_NormalizedTextScale(font, scale);
    CL_DrawTextRotate(scrPlace, text, maxChars, font, x, y, rotation, horzAlign, vertAlign, xScale, xScale, color, style);
}

void __cdecl UI_DrawTextWithGlowW(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float w,
                int horzAlign,
                int vertAlign,
                float scale,
                const float *color,
                int style,
                const float *glowColor,
                bool subtitle,
                bool cinematic)
{
    float v15; // [esp+38h] [ebp-18h]
    float v16; // [esp+3Ch] [ebp-14h]
    float xScale; // [esp+48h] [ebp-8h] BYREF
    float yScale; // [esp+4Ch] [ebp-4h] BYREF

    xScale = R_NormalizedTextScale(font, scale);
    yScale = xScale;
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    v16 = floor((float)(x + 0.5));
    x = v16;
    v15 = floor((float)(y + 0.5));
    y = v15;
    if ( subtitle )
        R_AddCmdDrawTextSubtitle(text, maxChars, font, x, y, xScale, yScale, 0.0, color, style, glowColor, cinematic);
    else
        CL_DrawTextPhysicalWithEffects(
            text,
            maxChars,
            font,
            x,
            y,
            w,
            xScale,
            yScale,
            color,
            style,
            glowColor,
            0,
            0,
            0,
            0,
            0,
            0);
}

void __cdecl UI_DrawTextWithGlow(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                int horzAlign,
                int vertAlign,
                float scale,
                const float *color,
                int style,
                const float *glowColor,
                bool subtitle,
                bool cinematic)
{
    UI_DrawTextWithGlowW(
        scrPlace,
        text,
        maxChars,
        font,
        x,
        y,
        1.0,
        horzAlign,
        vertAlign,
        scale,
        color,
        style,
        glowColor,
        subtitle,
        cinematic);
}

void __cdecl UI_DrawTextNoSnap(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                int horzAlign,
                int vertAlign,
                float scale,
                const float *color,
                int style)
{
    float xScale; // [esp+1Ch] [ebp-8h] BYREF
    float yScale; // [esp+20h] [ebp-4h] BYREF

    xScale = R_NormalizedTextScale(font, scale);
    yScale = xScale;
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    CL_DrawTextPhysical(text, maxChars, font, x, y, 1.0, xScale, yScale, color, style);
}

void __cdecl UI_DrawTextWithCursor(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                int horzAlign,
                int vertAlign,
                float scale,
                const float *color,
                int style,
                int cursorPos,
                char cursor)
{
    float v13; // [esp+20h] [ebp-18h]
    float v14; // [esp+24h] [ebp-14h]
    float xScale; // [esp+30h] [ebp-8h] BYREF
    float yScale; // [esp+34h] [ebp-4h] BYREF

    xScale = R_NormalizedTextScale(font, scale);
    yScale = xScale;
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    v14 = floor((float)(x + 0.5));
    x = v14;
    v13 = floor((float)(y + 0.5));
    y = v13;
    CL_DrawTextPhysicalWithCursor(text, maxChars, font, x, v13, xScale, yScale, color, style, cursorPos, cursor);
}

Font_s *__cdecl UI_GetFontHandleDefault(float scale)
{
    if ( ui_smallFont && ui_smallFont->current.value >= scale )
        return sharedUiInfo.assets.smallFont;
    if ( ui_extraBigFont && scale >= ui_extraBigFont->current.value )
        return sharedUiInfo.assets.extraBigFont;
    if ( ui_bigFont && scale >= ui_bigFont->current.value )
        return sharedUiInfo.assets.bigFont;
    return sharedUiInfo.assets.textFont;
}

Font_s *__cdecl UI_GetFontHandle(const ScreenPlacement *scrPlace, int fontEnum, float scale)
{
    float v4; // [esp+4h] [ebp-4h]
    float scalea; // [esp+18h] [ebp+10h]

    switch ( fontEnum )
    {
        case 2:
            return sharedUiInfo.assets.bigFont;
        case 3:
            return sharedUiInfo.assets.smallFont;
        case 1:
            return sharedUiInfo.assets.textFont;
        case 5:
            return sharedUiInfo.assets.consoleFont;
        case 6:
            return sharedUiInfo.assets.extraBigFont;
    }
    v4 = ScrPlace_HiResGetScale() * scale;
    scalea = v4 * scrPlace->scaleVirtualToReal[1];
    if ( fontEnum != 4 )
        return UI_GetFontHandleDefault(scalea);
    if ( ui_smallFont && ui_smallFont->current.value >= scalea )
        return sharedUiInfo.assets.smallFont;
    if ( ui_bigFont && scalea >= ui_bigFont->current.value )
        return sharedUiInfo.assets.boldFont;
    return sharedUiInfo.assets.textFont;
}

void __cdecl UI_UpdateTime(int localClientNum, int realtime)
{
    int frameTimeIndex; // [esp+0h] [ebp-Ch]
    uiInfo_s *uiInfo; // [esp+4h] [ebp-8h]
    int frameTimeTotal; // [esp+8h] [ebp-4h]

    uiInfo = UI_GetInfo(localClientNum);
    uiInfo->uiDC.frameTime = realtime - uiInfo->uiDC.realTime;
    uiInfo->uiDC.realTime = realtime;
    uiInfo->previousTimes[uiInfo->timeIndex++ % 4] = uiInfo->uiDC.frameTime;
    if ( uiInfo->timeIndex > 4 )
    {
        frameTimeTotal = 0;
        for ( frameTimeIndex = 0; frameTimeIndex < 4; ++frameTimeIndex )
            frameTimeTotal += uiInfo->previousTimes[frameTimeIndex];
        if ( !frameTimeTotal )
            frameTimeTotal = 1;
        uiInfo->uiDC.FPS = (float)(4000 / frameTimeTotal);
    }
}

void __cdecl UI_Refresh(int localClientNum)
{
    int closeSlideSpeed; // [esp+20h] [ebp-2Ch]
    int closeFadingTime; // [esp+24h] [ebp-28h]
    uiInfo_s *uiInfo; // [esp+38h] [ebp-14h]
    menuDef_t *toastPopupMenu; // [esp+3Ch] [ebp-10h]
    int contextIndex; // [esp+40h] [ebp-Ch]
    int savedregs; // [esp+4Ch] [ebp+0h] BYREF

    UI_Project_Refresh(localClientNum);
    uiInfo = UI_GetInfo(localClientNum);
    contextIndex = uiInfo->uiDC.contextIndex;
    if ( Menu_Count(&uiInfo->uiDC) > 0 )
    {
        Menu_PaintAll(localClientNum, &uiInfo->uiDC);
        UI_DoServerRefresh(localClientNum, uiInfo);
        UI_BuildServerStatus(localClientNum, uiInfo, 0);
        UI_BuildServerStatusScoreBoard(localClientNum, uiInfo, 0);
        if ( uiInfo->toastPopupOpened )
        {
            toastPopupMenu = Menus_FindByName(&uiInfo->uiDC, "menu_toast_popup");
            if ( (signed int)(Sys_Milliseconds() - uiInfo->toastPopupTimeCounter) >= uiInfo->toastPopupDuration )
            {
                if ( (Window_GetDynamicFlags(contextIndex, &toastPopupMenu->window) & 0x42000) != 0 )
                {
                    if ( (Window_GetDynamicFlags(contextIndex, &toastPopupMenu->window) & 0x42000) != 0 )
                    {
                        Menu_Paint(
                            localClientNum,
                            &uiInfo->uiDC,
                            0,
                            toastPopupMenu,
                            (const ScreenPlacement *)0xFFFFFFFF);
                        if ( (Window_GetDynamicFlags(contextIndex, &toastPopupMenu->window) & 0x42000) == 0 )
                        {
                            Window_RemoveDynamicFlags(contextIndex, &toastPopupMenu->window, 4);
                            uiInfo->toastPopupTimeCounter = -1;
                            uiInfo->toastPopupOpened = 0;
                        }
                    }
                }
                else
                {
                    Menu_Paint(
                        localClientNum,
                        &uiInfo->uiDC,
                        0,
                        toastPopupMenu,
                        (const ScreenPlacement *)0xFFFFFFFF);
                    if ( toastPopupMenu->closeSlideSpeed <= 0 || toastPopupMenu->closeSlideDirection >= 4u )
                    {
                        if ( toastPopupMenu->closeFadingTime > 0 )
                        {
                            Window_AddDynamicFlags(contextIndex, &toastPopupMenu->window, 0x2000);
                            toastPopupMenu->fadeTimeCounter = Sys_Milliseconds();
                        }
                    }
                    else
                    {
                        Window_AddDynamicFlags(contextIndex, &toastPopupMenu->window, 0x40000);
                        toastPopupMenu->slideTimeCounter = Sys_Milliseconds();
                        if ( toastPopupMenu->closeFadingTime > 0 )
                        {
                            Window_AddDynamicFlags(contextIndex, &toastPopupMenu->window, 0x2000);
                            if ( toastPopupMenu->closeSlideSpeed < toastPopupMenu->closeFadingTime )
                                closeFadingTime = toastPopupMenu->closeFadingTime;
                            else
                                closeFadingTime = toastPopupMenu->closeSlideSpeed;
                            toastPopupMenu->closeSlideSpeed = closeFadingTime;
                            if ( toastPopupMenu->closeSlideSpeed < toastPopupMenu->closeFadingTime )
                                closeSlideSpeed = toastPopupMenu->closeFadingTime;
                            else
                                closeSlideSpeed = toastPopupMenu->closeSlideSpeed;
                            toastPopupMenu->closeFadingTime = closeSlideSpeed;
                            toastPopupMenu->fadeTimeCounter = Sys_Milliseconds();
                        }
                    }
                }
            }
            else
            {
                Window_AddDynamicFlags(contextIndex, &toastPopupMenu->window, 4);
                Menu_Paint(
                    localClientNum,
                    &uiInfo->uiDC,
                    0,
                    toastPopupMenu,
                    (const ScreenPlacement *)0xFFFFFFFF);
            }
        }
        UI_BuildFindPlayerList();
        if ( UI_ShouldDrawBuildNumber(localClientNum) )
            UI_DrawBuildNumber(contextIndex);
        if ( uiInfo->uiDC.isCursorVisible && !UI_KeysBypassMenu(localClientNum) && !CG_IsShowingZombieMap() )
            UI_DrawHandlePic(
                &scrPlaceView[contextIndex],
                uiInfo->uiDC.cursor.x
            - (float)((float)((float)(32.0 * scrPlaceFull.scaleVirtualToReal[0]) / scrPlaceFull.scaleVirtualToFull[0]) * 0.5),
                uiInfo->uiDC.cursor.y
            - (float)((float)((float)(32.0 * scrPlaceFull.scaleVirtualToReal[1]) / scrPlaceFull.scaleVirtualToFull[1]) * 0.5),
                (float)(32.0 * scrPlaceFull.scaleVirtualToReal[0]) / scrPlaceFull.scaleVirtualToFull[0],
                (float)(32.0 * scrPlaceFull.scaleVirtualToReal[1]) / scrPlaceFull.scaleVirtualToFull[1],
                4,
                4,
                0,
                sharedUiInfo.assets.cursor);
    }
    if ( !Menu_GetFocused(&uiInfo->uiDC) && Key_IsCatcherActive(localClientNum, 16) )
        Key_RemoveCatcher(localClientNum, -17);
    if ( UI_ShouldUpdateFriends() )
    {
        Com_LocalClient_GetControllerIndex(localClientNum);
        UI_FriendsUpdate();
    }
}

bool __cdecl UI_ShouldDrawBuildNumber(int localClientNum)
{
    connstate_t connectionState; // [esp+0h] [ebp-4h]

    connectionState = CL_GetLocalClientConnectionState(localClientNum);
    return CL_AllLocalClientsDisconnected() && connectionState == CA_DISCONNECTED;
}

bool __cdecl UI_ShouldUpdateFriends()
{
    return Dvar_GetBool("ui_friendsListOpen") || Dvar_GetBool("ui_playerListOpen");
}

void __cdecl UI_Shutdown(int localClientNum)
{
    uiInfo_s *uiInfo; // [esp+0h] [ebp-4h]

    uiInfo = UI_GetInfo(localClientNum);
    Menus_CloseAll(localClientNum, &uiInfo->uiDC);
    sharedUiInfo.assets.whiteMaterial = 0;
    UILocalVar_Shutdown(&uiInfo->uiDC.localVars);
    if ( !useFastFile->current.enabled )
        Menus_FreeAllMemory(&uiInfo->uiDC);
    UI_Project_Shutdown(localClientNum);
}

int __cdecl Load_ScriptMenu(int localClientNum, const char *pszMenu, int imageTrack)
{
    uiInfo_s *uiInfo; // [esp+0h] [ebp-8h]
    MenuList *menuList; // [esp+4h] [ebp-4h]

    menuList = Load_ScriptMenuInternal(pszMenu, imageTrack);
    if ( !menuList )
        return 0;
    uiInfo = UI_GetInfo(localClientNum);
    UI_AddMenuList(localClientNum, &uiInfo->uiDC, menuList, 1);
    return 1;
}

MenuList *__cdecl Load_ScriptMenuInternal(const char *pszMenu, int imageTrack)
{
    char szMenuFile[260]; // [esp+0h] [ebp-108h] BYREF

    strcpy(szMenuFile, "ui_mp/scriptmenus/");
    I_strncat(szMenuFile, 256, pszMenu);
    I_strncat(szMenuFile, 256, ".menu");
    return UI_LoadMenu(szMenuFile, imageTrack);
}

char *__cdecl UI_GetMapDisplayName(const char *pszMap)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < sharedUiInfo.mapCount; ++i )
    {
        if ( !I_stricmp(pszMap, sharedUiInfo.mapList[i].mapLoadName) )
            return UI_SafeTranslateString(&sharedUiInfo.joinGameTypes[32].gameType[304 * i]);
    }
    return (char *)pszMap;
}

char *__cdecl UI_GetMapDisplayNameFromPartialLoadNameMatch(const char *mapName, int *mapLoadNameLen)
{
    int i; // [esp+10h] [ebp-4h]

    if ( !mapLoadNameLen
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp", 1173, 0, "%s", "mapLoadNameLen") )
    {
        __debugbreak();
    }
    for ( i = 0; i < sharedUiInfo.mapCount; ++i )
    {
        *mapLoadNameLen = strlen(sharedUiInfo.mapList[i].mapLoadName);
        if ( !I_strnicmp(mapName, sharedUiInfo.mapList[i].mapLoadName, *mapLoadNameLen) )
            return UI_SafeTranslateString(&sharedUiInfo.joinGameTypes[32].gameType[304 * i]);
    }
    return 0;
}

char *__cdecl UI_GetGameTypeDisplayName(const char *pszGameType)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < sharedUiInfo.numGameTypes; ++i)
    {
        if (!I_stricmp(pszGameType, sharedUiInfo.gameTypes[i].gameType))
            return UI_SafeTranslateString(sharedUiInfo.gameTypes[i].gameTypeName);
    }
    return (char *)pszGameType;
}

char *__cdecl UI_GetGameTypeDisplayShortName(const char *pszGameType)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < sharedUiInfo.numGameTypes; ++i)
    {
        if (!I_stricmp(pszGameType, sharedUiInfo.gameTypes[i].gameType))
            return UI_SafeTranslateString(sharedUiInfo.gameTypes[i].gameTypeShortName);
    }
    return (char *)pszGameType;
}

void __cdecl UI_Update(const char *name)
{
    char *value; // eax
    char *VariantString; // eax
    int rate; // [esp+4h] [ebp-4h]

    if ( I_stricmp(name, "ui_SetName") )
    {
        if ( I_stricmp(name, "ui_GetName") )
        {
            if ( I_stricmp(name, "ui_setRate") )
            {
                if ( !I_stricmp(name, "ui_mousePitch") )
                {
                    if ( Dvar_GetBool(name) )
                        Dvar_SetFloatByName("m_pitch", -0.022);
                    else
                        Dvar_SetFloatByName("m_pitch", 0.022);
                }
            }
            else
            {
                rate = Dvar_GetInt("rate");
                if ( rate < 5000 )
                {
                    Dvar_SetIntByName("cl_maxpackets", 15);
                    if ( rate < 4000 )
                        Dvar_SetIntByName("cl_packetdup", 1);
                    else
                        Dvar_SetIntByName("cl_packetdup", 2);
                }
                else
                {
                    Dvar_SetIntByName("cl_maxpackets", 30);
                    Dvar_SetIntByName("cl_packetdup", 1);
                }
            }
        }
        else
        {
            VariantString = (char *)Dvar_GetVariantString("name");
            Dvar_SetStringByName("ui_Name", VariantString);
        }
    }
    else
    {
        value = (char *)Dvar_GetVariantString("ui_Name");
        Dvar_SetStringByName("name", value);
    }
}

void __cdecl UI_VerifyLanguage()
{
    int newLanguage; // [esp+0h] [ebp-Ch]
    int verifiedLanguage; // [esp+4h] [ebp-8h]
    int oldLanguage; // [esp+8h] [ebp-4h]

    oldLanguage = Dvar_GetInt("loc_language");
    newLanguage = Dvar_GetInt("ui_language");
    verifiedLanguage = SEH_VerifyLanguageSelection(newLanguage);
    if ( verifiedLanguage != newLanguage )
        Dvar_SetIntByName("ui_language", verifiedLanguage);
    if ( newLanguage == oldLanguage )
        Dvar_SetBoolByName("ui_languagechanged", 0);
    else
        Dvar_SetBoolByName("ui_languagechanged", 1);
}

void __cdecl UI_UpdateDisplayServers(int localClientNum, uiInfo_s *uiInfo)
{
    int serverCount; // [esp+0h] [ebp-4h]

    serverCount = LAN_GetServerCount(ui_netSource->current.integer);
    if (sharedUiInfo.serverStatus.serverCount != serverCount)
    {
        sharedUiInfo.serverStatus.serverCount = serverCount;
        if (sharedUiInfo.serverStatus.numDisplayServers)
        {
            sharedUiInfo.serverStatus.currentServer = -1;
            UI_BuildServerDisplayList(localClientNum, uiInfo, 1);
        }
    }
}

int __cdecl UI_GetClientNumForPlayerListNum(int playerListIndex)
{
    if ( sharedUiInfo.playerClientNums[playerListIndex - 1] >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                    1420,
                    0,
                    "%s\n\t(sharedUiInfo.playerClientNums[playerListIndex]) = %i",
                    "(sharedUiInfo.playerClientNums[playerListIndex] >= 0 && sharedUiInfo.playerClientNums[playerListIndex] < 32)",
                    sharedUiInfo.playerClientNums[playerListIndex - 1]) )
    {
        __debugbreak();
    }
    return sharedUiInfo.playerClientNums[playerListIndex - 1];
}

void __cdecl UI_RunMenuScript(int localClientNum, int contextIndex, __int64 args)
{
    int ControllerIndex; // eax
    char *v5; // eax
    const char *String; // eax
    char *v7; // eax
    char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    char *v11; // eax
    unsigned int CountByType; // esi
    unsigned int v13; // esi
    char *v14; // eax
    char *v15; // eax
    char *v16; // eax
    char *v17; // eax
    char *v18; // eax
    int v19; // eax
    int Int; // eax
    unsigned int ClientNumForPlayerListNum; // eax
    const char *feederID_4; // [esp+4h] [ebp-1D44h]
    unsigned int feederID_4a; // [esp+4h] [ebp-1D44h]
    int feederID_4b; // [esp+4h] [ebp-1D44h]
    const char *v25; // [esp+8h] [ebp-1D40h]
    char v26[128]; // [esp+10h] [ebp-1D38h] BYREF
    char v27[256]; // [esp+90h] [ebp-1CB8h] BYREF
    char v28[128]; // [esp+190h] [ebp-1BB8h] BYREF
    char menuName[128]; // [esp+210h] [ebp-1B38h] BYREF
    char testValue[256]; // [esp+290h] [ebp-1AB8h] BYREF
    char v31[132]; // [esp+390h] [ebp-19B8h] BYREF
    int status; // [esp+414h] [ebp-1934h] BYREF
    char name[1024]; // [esp+418h] [ebp-1930h] BYREF
    unsigned __int64 xuid; // [esp+818h] [ebp-1530h] BYREF
    char buf[1024]; // [esp+820h] [ebp-1528h] BYREF
    int column; // [esp+C20h] [ebp-1128h] BYREF
    int i; // [esp+C24h] [ebp-1124h] BYREF
    char dest[268]; // [esp+C28h] [ebp-1120h] BYREF
    char dvarName[1024]; // [esp+D38h] [ebp-1010h] BYREF
    char v40[1024]; // [esp+1138h] [ebp-C10h] BYREF
    char v41[1028]; // [esp+1538h] [ebp-810h] BYREF
    UiContext *dc; // [esp+193Ch] [ebp-40Ch]
    char out[1028]; // [esp+1940h] [ebp-408h] BYREF

    if (String_Parse((const char **)args, out, 1024))
    {
        dc = (UiContext *)UI_UIContext_GetInfo(contextIndex);
        if (!I_stricmp(out, "Quit"))
        {
            ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
            Cmd_ExecuteSingleCommand(localClientNum, ControllerIndex, (char*)"quit");
            return;
        }
        if (I_stricmp(out, "openurl"))
        {
            if (!I_stricmp(out, "iviteOrDeleteFriend")
                || !I_stricmp(out, "sendRequestFriend")
                || !I_stricmp(out, "deleteFriendPopup")
                || !I_stricmp(out, "deleteFriend"))
            {
                goto LABEL_25;
            }
            if (!I_stricmp(out, "acceptFriend"))
            {
                UI_AcceptFriend();
                return;
            }
            if (!I_stricmp(out, "declineFriend"))
            {
                UI_DeclineFriend();
                return;
            }
            if (!I_stricmp(out, "acceptAllFriend") || !I_stricmp(out, "declineAllFriend") || !I_stricmp(out, "inviteFriend"))
                goto LABEL_25;
            if (!I_stricmp(out, "acceptInvite"))
            {
                UI_AcceptInvite();
                return;
            }
            if (I_stricmp(out, "clearInvites"))
            {
                if (I_stricmp(out, "loadArenas"))
                {
                    if (I_stricmp(out, "LoadMods"))
                    {
                        if (I_stricmp(out, "voteTypeMap"))
                        {
                            if (I_stricmp(out, "voteTypeMapDVar"))
                            {
                                if (I_stricmp(out, "voteMap"))
                                {
                                    if (I_stricmp(out, "voteGame"))
                                    {
                                        if (I_stricmp(out, "RefreshServers"))
                                        {
                                            if (I_stricmp(out, "RefreshFilter"))
                                            {
                                                if (I_stricmp(out, "RunMod"))
                                                {
                                                    if (I_stricmp(out, "UpdateNetSource"))
                                                    {
                                                        if (I_stricmp(out, "closeJoin"))
                                                        {
                                                            if (I_stricmp(out, "StopRefresh"))
                                                            {
                                                                if (I_stricmp(out, "ServerStatus"))
                                                                {
                                                                    if (I_stricmp(out, "ServerStatusScoreBoard"))
                                                                    {
                                                                        if (I_stricmp(out, "UpdateFilter"))
                                                                        {
                                                                            if (I_stricmp(out, "Controls"))
                                                                            {
                                                                                if (I_stricmp(out, "Leave"))
                                                                                {
                                                                                    if (I_stricmp(out, "ServerSort"))
                                                                                    {
                                                                                        if (I_stricmp(out, "closeingame"))
                                                                                        {
                                                                                            if (I_stricmp(out, "voteKick"))
                                                                                            {
                                                                                                if (I_stricmp(out, "voteTempBan"))
                                                                                                {
                                                                                                    if (I_stricmp(out, "addFavorite"))
                                                                                                    {
                                                                                                        if (I_stricmp(out, "deleteFavorite"))
                                                                                                        {
                                                                                                            if (I_stricmp(out, "update"))
                                                                                                            {
                                                                                                                if (I_stricmp(out, "setPbClStatus"))
                                                                                                                {
                                                                                                                    if (I_stricmp(out, "startSingleplayer"))
                                                                                                                    {
                                                                                                                        if (I_stricmp(out, "getLanguage"))
                                                                                                                        {
                                                                                                                            if (I_stricmp(out, "verifyLanguage"))
                                                                                                                            {
                                                                                                                                if (I_stricmp(out, "updateLanguage"))
                                                                                                                                {
                                                                                                                                    if (I_stricmp(out, "mutePlayer"))
                                                                                                                                    {
                                                                                                                                        if (I_stricmp(out, "openMenuOnDvar")
                                                                                                                                            && I_stricmp(out, "openMenuOnDvarNot"))
                                                                                                                                        {
                                                                                                                                            if (I_stricmp(out, "closeMenuOnDvar")
                                                                                                                                                && I_stricmp(out, "closeMenuOnDvarNot"))
                                                                                                                                            {
                                                                                                                                                if (I_stricmp(out, "setRecommended"))
                                                                                                                                                {
                                                                                                                                                    if (I_stricmp(out, "clearLoadErrorsSummary"))
                                                                                                                                                    {
                                                                                                                                                        if (I_stricmp(out, "RefreshLeaderboards"))
                                                                                                                                                        {
                                                                                                                                                            if (I_stricmp(out, "ViewGamerCard"))
                                                                                                                                                                UI_Project_RunMenuScript(
                                                                                                                                                                    localClientNum,
                                                                                                                                                                    contextIndex,
                                                                                                                                                                    out,
                                                                                                                                                                    (const char **)args,
                                                                                                                                                                    (const char *)HIDWORD(args));
                                                                                                                                                            else
                                                                                                                                                                LB_OnSelect(localClientNum);
                                                                                                                                                        }
                                                                                                                                                        else
                                                                                                                                                        {
                                                                                                                                                            LB_ForceRefresh();
                                                                                                                                                        }
                                                                                                                                                    }
                                                                                                                                                    else
                                                                                                                                                    {
                                                                                                                                                        Menus_CloseAll(localClientNum, dc);
                                                                                                                                                    }
                                                                                                                                                }
                                                                                                                                                else
                                                                                                                                                {
                                                                                                                                                    Com_SetRecommended(localClientNum, 1);
                                                                                                                                                }
                                                                                                                                            }
                                                                                                                                            else if (UI_GetOpenOrCloseMenuOnDvarArgs(
                                                                                                                                                (const char **)args,
                                                                                                                                                out,
                                                                                                                                                v28,
                                                                                                                                                128,
                                                                                                                                                v27,
                                                                                                                                                256,
                                                                                                                                                v26,
                                                                                                                                                128))
                                                                                                                                            {
                                                                                                                                                UI_CloseMenuOnDvar(
                                                                                                                                                    localClientNum,
                                                                                                                                                    (uiInfo_s *)dc,
                                                                                                                                                    out,
                                                                                                                                                    v26,
                                                                                                                                                    v28,
                                                                                                                                                    v27);
                                                                                                                                            }
                                                                                                                                        }
                                                                                                                                        else if (UI_GetOpenOrCloseMenuOnDvarArgs(
                                                                                                                                            (const char **)args,
                                                                                                                                            out,
                                                                                                                                            v31,
                                                                                                                                            128,
                                                                                                                                            testValue,
                                                                                                                                            256,
                                                                                                                                            menuName,
                                                                                                                                            128))
                                                                                                                                        {
                                                                                                                                            UI_OpenMenuOnDvar(
                                                                                                                                                localClientNum,
                                                                                                                                                (uiInfo_s *)dc,
                                                                                                                                                out,
                                                                                                                                                menuName,
                                                                                                                                                v31,
                                                                                                                                                testValue);
                                                                                                                                        }
                                                                                                                                    }
                                                                                                                                    else if (dc[1].bias >= 0.0
                                                                                                                                        && SLODWORD(dc[1].bias) < sharedUiInfo.playerCount)
                                                                                                                                    {
                                                                                                                                        ClientNumForPlayerListNum = UI_GetClientNumForPlayerListNum(LODWORD(dc[1].bias));
                                                                                                                                        CL_MutePlayer(
                                                                                                                                            localClientNum,
                                                                                                                                            ClientNumForPlayerListNum);
                                                                                                                                    }
                                                                                                                                }
                                                                                                                                else
                                                                                                                                {
                                                                                                                                    Int = Dvar_GetInt("ui_language");
                                                                                                                                    Dvar_SetIntByName("loc_language", Int);
                                                                                                                                    UI_VerifyLanguage();
                                                                                                                                    Cbuf_AddText(localClientNum, "vid_restart\n");
                                                                                                                                }
                                                                                                                            }
                                                                                                                            else
                                                                                                                            {
                                                                                                                                UI_VerifyLanguage();
                                                                                                                            }
                                                                                                                        }
                                                                                                                        else
                                                                                                                        {
                                                                                                                            v19 = Dvar_GetInt("loc_language");
                                                                                                                            Dvar_SetIntByName("ui_language", v19);
                                                                                                                            UI_VerifyLanguage();
                                                                                                                        }
                                                                                                                    }
                                                                                                                    else
                                                                                                                    {
                                                                                                                        Cbuf_AddText(localClientNum, "startSingleplayer\n");
                                                                                                                    }
                                                                                                                }
                                                                                                                else if (Int_Parse((const char **)args, &status))
                                                                                                                {
                                                                                                                    CLUI_SetPbClStatus(status);
                                                                                                                }
                                                                                                            }
                                                                                                            else if (String_Parse((const char **)args, name, 1024))
                                                                                                            {
                                                                                                                UI_Update(name);
                                                                                                            }
                                                                                                        }
                                                                                                        else if (ui_netSource->current.integer == 3
                                                                                                            && sharedUiInfo.serverStatus.currentServer >= 0
                                                                                                            && sharedUiInfo.serverStatus.currentServer < sharedUiInfo.serverStatus.numDisplayServers)
                                                                                                        {
                                                                                                            UI_UpdateDisplayServers(localClientNum, (uiInfo_s *)dc);
                                                                                                            UI_RemoveServerFromFavoritesList();
                                                                                                        }
                                                                                                    }
                                                                                                    else if (ui_netSource->current.integer != 3
                                                                                                        && sharedUiInfo.serverStatus.currentServer >= 0
                                                                                                        && sharedUiInfo.serverStatus.currentServer < sharedUiInfo.serverStatus.numDisplayServers)
                                                                                                    {
                                                                                                        LAN_GetServerInfo(
                                                                                                            ui_netSource->current.integer,
                                                                                                            *(_DWORD *)&sharedUiInfo.serverStatusAddress[4
                                                                                                            * sharedUiInfo.serverStatus.currentServer
                                                                                                            - 81328],
                                                                                                            buf,
                                                                                                            1024);
                                                                                                        v18 = Info_ValueForKey(buf, "dwuserid");
                                                                                                        StringToXUID(v18, &xuid);
                                                                                                        UI_AddServerToFavoritesList(xuid);
                                                                                                    }
                                                                                                }
                                                                                                else if (dc[1].bias >= 0.0
                                                                                                    && SLODWORD(dc[1].bias) < sharedUiInfo.playerCount)
                                                                                                {
                                                                                                    v17 = va(
                                                                                                        "callvote tempBanUser \"%s\"\n",
                                                                                                        sharedUiInfo.playerNames[LODWORD(dc[1].bias)]);
                                                                                                    Cbuf_AddText(localClientNum, v17);
                                                                                                }
                                                                                            }
                                                                                            else if (dc[1].bias >= 0.0
                                                                                                && SLODWORD(dc[1].bias) < sharedUiInfo.playerCount)
                                                                                            {
                                                                                                v16 = va(
                                                                                                    "callvote kick \"%s\"\n",
                                                                                                    sharedUiInfo.playerNames[LODWORD(dc[1].bias)]);
                                                                                                Cbuf_AddText(localClientNum, v16);
                                                                                            }
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            Key_RemoveCatcher(localClientNum, -17);
                                                                                            Key_ClearStates(localClientNum);
                                                                                            CL_PauseGame(0);
                                                                                            Menus_CloseAll(localClientNum, dc);
                                                                                        }
                                                                                    }
                                                                                    else if (Int_Parse((const char **)args, &column))
                                                                                    {
                                                                                        if (column == sharedUiInfo.serverStatus.sortKey)
                                                                                            sharedUiInfo.serverStatus.sortDir = sharedUiInfo.serverStatus.sortDir == 0;
                                                                                        UI_ServersSort((unsigned int)column | 0x100000000LL);
                                                                                    }
                                                                                }
                                                                                else
                                                                                {
                                                                                    Cbuf_AddText(localClientNum, "disconnect\n");
                                                                                    Key_SetCatcher(localClientNum, 16);
                                                                                    Menus_CloseAll(localClientNum, dc);
                                                                                    Menus_OpenByName(localClientNum, dc, "main");
                                                                                }
                                                                            }
                                                                            else
                                                                            {
                                                                                CL_PauseGame(1);
                                                                                Key_SetCatcher(localClientNum, 16);
                                                                                Menus_CloseAll(localClientNum, dc);
                                                                                Menus_OpenByName(localClientNum, dc, "setup_menu2");
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                            UI_BuildServerDisplayList(localClientNum, (uiInfo_s *)dc, 1);
                                                                            UI_FeederSelection(localClientNum, contextIndex, 2.0, 0);
                                                                        }
                                                                    }
                                                                    else
                                                                    {
                                                                        UI_UpdateDisplayServers(localClientNum, uiInfoArray);
                                                                        if (sharedUiInfo.serverStatus.currentServer >= 0
                                                                            && sharedUiInfo.serverStatus.currentServer < sharedUiInfo.serverStatus.numDisplayServers)
                                                                        {
                                                                            LAN_GetServerSecurityId(
                                                                                ui_netSource->current.integer,
                                                                                *(_DWORD *)&sharedUiInfo.serverStatusAddress[4
                                                                                * sharedUiInfo.serverStatus.currentServer
                                                                                - 81328],
                                                                                (bdSecurityID *)sharedUiInfo.serverStatusSecurityID,
                                                                                8);
                                                                            UI_BuildServerStatusScoreBoard(localClientNum, uiInfoArray, 1);
                                                                        }
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    Dvar_SetBool((dvar_s*)ui_browserShowInfo, 0);
                                                                    UI_UpdateDisplayServers(localClientNum, uiInfoArray);
                                                                    if (sharedUiInfo.serverStatus.currentServer >= 0
                                                                        && sharedUiInfo.serverStatus.currentServer < sharedUiInfo.serverStatus.numDisplayServers)
                                                                    {
                                                                        LAN_GetServerSecurityId(
                                                                            ui_netSource->current.integer,
                                                                            *(_DWORD *)&sharedUiInfo.serverStatusAddress[4
                                                                            * sharedUiInfo.serverStatus.currentServer
                                                                            - 81328],
                                                                            (bdSecurityID *)sharedUiInfo.serverStatusSecurityID,
                                                                            8);
                                                                        UI_BuildServerStatus(localClientNum, uiInfoArray, 1);
                                                                    }
                                                                }
                                                            }
                                                            else
                                                            {
                                                                UI_StopServerRefresh();
                                                                sharedUiInfo.serverStatus.nextDisplayRefresh = 0;
                                                                sharedUiInfo.nextServerStatusRefresh = 0;
                                                                dc[1].localVars.table[65].name = 0;
                                                            }
                                                        }
                                                        else if (sharedUiInfo.serverStatus.refreshActive)
                                                        {
                                                            UI_StopServerRefresh();
                                                            sharedUiInfo.serverStatus.nextDisplayRefresh = 0;
                                                            sharedUiInfo.nextServerStatusRefresh = 0;
                                                            sharedUiInfo.nextServerStatusScoreBoardRefresh = 0;
                                                            dc[1].localVars.table[65].name = 0;
                                                            UI_BuildServerDisplayList(localClientNum, (uiInfo_s *)dc, 1);
                                                        }
                                                        else
                                                        {
                                                            Menus_CloseByName(localClientNum, dc, "joinserver");
                                                            Menus_OpenByName(localClientNum, dc, "main");
                                                        }
                                                    }
                                                    else if (Int_Parse((const char **)args, &i))
                                                    {
                                                        UI_NetSource_UpdateDisplayList(i);
                                                    }
                                                }
                                                else
                                                {
                                                    if (sharedUiInfo.modIndex >= 0x40u
                                                        && !Assert_MyHandler(
                                                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                                                            1719,
                                                            0,
                                                            "sharedUiInfo.modIndex doesn't index MAX_MODS\n\t%i not in [0, %i)",
                                                            sharedUiInfo.modIndex,
                                                            64))
                                                    {
                                                        __debugbreak();
                                                    }
                                                    if (sharedUiInfo.modList[sharedUiInfo.modIndex].modName)
                                                    {
                                                        Com_sprintf(
                                                            dest,
                                                            0x104u,
                                                            "%s/%s",
                                                            "mods",
                                                            sharedUiInfo.modList[sharedUiInfo.modIndex].modName);
                                                        if (useFastFile->current.enabled)
                                                            DB_SyncXAssets();
                                                        Dvar_SetStringByName("fs_game", dest);
                                                        Cbuf_AddText(localClientNum, "vid_restart\n");
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                UI_StartServerRefresh(localClientNum, contextIndex, 0);
                                                UI_BuildServerDisplayList(localClientNum, (uiInfo_s *)dc, 1);
                                            }
                                        }
                                        else
                                        {
                                            UI_StartServerRefresh(localClientNum, contextIndex, 1);
                                            UI_BuildServerDisplayList(localClientNum, (uiInfo_s *)dc, 1);
                                            feederID_4a = LiveGroups_GetCount((char*)"online/mp");
                                            v10 = UI_SafeTranslateString("PLATFORM_PLAYERS_ONLINE");
                                            v11 = UI_ReplaceConversionInt(v10, feederID_4a);
                                            Dvar_SetString((dvar_s*)ui_browserPlayerCount, v11);
                                            CountByType = LiveGroups_GetCountByType(SERVER_GROUP_RANKED);
                                            v13 = LiveGroups_GetCountByType(SERVER_GROUP_UNRANKED) + CountByType;
                                            feederID_4b = LiveGroups_GetCountByType(SERVER_GROUP_WAGER) + v13;
                                            v14 = UI_SafeTranslateString("PLATFORM_ALL_SERVERS");
                                            v15 = UI_ReplaceConversionInt(v14, feederID_4b);
                                            Dvar_SetString((dvar_s*)ui_browserDedicatedServerCount, v15);
                                        }
                                    }
                                    else
                                    {
                                        v9 = va(
                                            "callvote g_gametype %s\n",
                                            sharedUiInfo.gameTypes[ui_netGameType->current.integer].gameType);
                                        Cbuf_AddText(localClientNum, v9);
                                    }
                                }
                                else if (ui_currentNetMap->current.integer >= 0
                                    && ui_currentNetMap->current.integer < sharedUiInfo.mapCount)
                                {
                                    v8 = va("callvote map %s\n", sharedUiInfo.mapList[ui_currentNetMap->current.integer].mapLoadName);
                                    Cbuf_AddText(localClientNum, v8);
                                }
                            }
                            else if (String_Parse((const char **)args, v40, 1024)
                                && String_Parse((const char **)args, dvarName, 1024))
                            {
                                feederID_4 = Dvar_GetString(dvarName);
                                String = Dvar_GetString(v40);
                                v7 = va("callvote typemap %s %s\n", String, feederID_4);
                                Cbuf_AddText(localClientNum, v7);
                            }
                        }
                        else
                        {
                            v5 = va(
                                "callvote typemap %s %s\n",
                                sharedUiInfo.gameTypes[ui_netGameType->current.integer].gameType,
                                sharedUiInfo.mapList[ui_currentNetMap->current.integer].mapLoadName);
                            Cbuf_AddText(localClientNum, v5);
                        }
                    }
                    else
                    {
                        UI_LoadMods();
                        sharedUiInfo.modIndex = 0;
                    }
                }
                else
                {
                    UI_LoadArenas();
                    UI_SelectCurrentGameType();
                    UI_MapCountByGameType();
                    Menu_SetFeederSelection(localClientNum, dc, 0, 4, 0, "createserver");
                    UI_SelectCurrentMap(localClientNum);
                }
            }
            else
            {
            LABEL_25:
                ;
                //BG_EvalVehicleName();
            }
        }
        else if (String_Parse((const char **)args, v41, 1024))
        {
            Com_PrintError(13, "Fixme krassi: openurl\n ");
        }
        else
        {
            Com_PrintError(13, "openurl invalid argument %s\n ", out);
        }
    }
}

char __cdecl UI_GetOpenOrCloseMenuOnDvarArgs(
                const char **args,
                const char *cmd,
                char *dvarName,
                int dvarNameLen,
                char *testValue,
                int testValueLen,
                char *menuName,
                int menuNameLen)
{
    if ( String_Parse(args, dvarName, dvarNameLen) )
    {
        if ( String_Parse(args, testValue, testValueLen) )
        {
            if ( String_Parse(args, menuName, menuNameLen) )
            {
                return 1;
            }
            else
            {
                Com_Printf(13, "%s: invalid menu name.\n", cmd);
                return 0;
            }
        }
        else
        {
            Com_Printf(13, "%s: invalid test value.\n", cmd);
            return 0;
        }
    }
    else
    {
        Com_Printf(13, "%s: invalid dvar name.\n", cmd);
        return 0;
    }
}

void __cdecl UI_OpenMenuOnDvar(
                int localClientNum,
                uiInfo_s *uiInfo,
                const char *cmd,
                const char *menuName,
                const char *dvarName,
                const char *testValue)
{
    bool wantMatch; // [esp+3h] [ebp-1h]

    if ( !cmd && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp", 1369, 0, "%s", "cmd") )
        __debugbreak();
    if ( I_stricmp(cmd, "openMenuOnDvar")
        && I_stricmp(cmd, "openMenuOnDvarNot")
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                    1370,
                    0,
                    "%s\n\t(cmd) = %s",
                    "(!I_stricmp( cmd, \"openMenuOnDvar\" ) || !I_stricmp( cmd, \"openMenuOnDvarNot\" ))",
                    cmd) )
    {
        __debugbreak();
    }
    if ( !menuName && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp", 1371, 0, "%s", "menuName") )
        __debugbreak();
    wantMatch = I_stricmp(cmd, "openMenuOnDvar") == 0;
    if ( UI_DvarValueTest(cmd, dvarName, testValue, wantMatch) )
        Menus_OpenByName(localClientNum, &uiInfo->uiDC, menuName);
}

bool __cdecl UI_DvarValueTest(const char *cmd, const char *dvarName, const char *testValue, bool wantMatch)
{
    const char *dvarValue; // [esp+8h] [ebp-4h]

    if ( !cmd && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp", 1348, 0, "%s", "cmd") )
        __debugbreak();
    if ( !dvarName && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp", 1349, 0, "%s", "dvarName") )
        __debugbreak();
    if ( !testValue && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp", 1350, 0, "%s", "testValue") )
        __debugbreak();
    if ( Dvar_FindVar(dvarName) )
    {
        dvarValue = Dvar_GetVariantString(dvarName);
        return (I_stricmp(testValue, dvarValue) == 0) == wantMatch;
    }
    else
    {
        Com_Printf(13, "%s: cannot find dvar %s\n", cmd, dvarName);
        return 0;
    }
}

void __cdecl UI_CloseMenuOnDvar(
                int localClientNum,
                uiInfo_s *uiInfo,
                const char *cmd,
                const char *menuName,
                const char *dvarName,
                const char *testValue)
{
    bool wantMatch; // [esp+3h] [ebp-1h]

    if ( !cmd && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp", 1382, 0, "%s", "cmd") )
        __debugbreak();
    if ( I_stricmp(cmd, "closeMenuOnDvar")
        && I_stricmp(cmd, "closeMenuOnDvarNot")
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                    1383,
                    0,
                    "%s\n\t(cmd) = %s",
                    "(!I_stricmp( cmd, \"closeMenuOnDvar\" ) || !I_stricmp( cmd, \"closeMenuOnDvarNot\" ))",
                    cmd) )
    {
        __debugbreak();
    }
    if ( !menuName && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp", 1384, 0, "%s", "menuName") )
        __debugbreak();
    wantMatch = I_stricmp(cmd, "closeMenuOnDvar") == 0;
    if ( UI_DvarValueTest(cmd, dvarName, testValue, wantMatch) )
        Menus_CloseByName(localClientNum, &uiInfo->uiDC, menuName);
}

void __cdecl UI_Pause(int localClientNum, int b)
{
    if ( b )
    {
        CL_PauseGame(1);
        Key_SetCatcher(localClientNum, 16);
    }
    else
    {
        Key_RemoveCatcher(localClientNum, -17);
        Key_ClearStates(localClientNum);
        CL_PauseGame(0);
    }
}

void __cdecl UI_MouseEvent(int localClientNum, int x, int y)
{
    float v3; // eax
    bool v4; // [esp+0h] [ebp-8h]
    uiInfo_s *uiInfo; // [esp+4h] [ebp-4h]

    uiInfo = UI_GetInfo(localClientNum);
    v3 = uiInfo->uiDC.cursor.y;
    uiInfo->uiDC.prevCursor.x = uiInfo->uiDC.cursor.x;
    uiInfo->uiDC.prevCursor.y = v3;
    uiInfo->uiDC.cursor.x = (double)x / (ScrPlace_HiResGetScale() * scrPlaceFull.scaleVirtualToFull[0]);
    uiInfo->uiDC.cursor.y = (double)y / (ScrPlace_HiResGetScale() * scrPlaceFull.scaleVirtualToFull[1]);
    v4 = uiInfo->uiDC.cursor.x >= 0.0
        && uiInfo->uiDC.cursor.x <= 640.0
        && uiInfo->uiDC.cursor.y >= 0.0
        && uiInfo->uiDC.cursor.y <= 480.0;
    uiInfo->uiDC.isCursorVisible = v4;
    CL_ShowSystemCursor(uiInfo->uiDC.isCursorVisible == 0);
    if ( uiInfo->uiDC.isCursorVisible )
    {
        if ( Menu_Count(&uiInfo->uiDC) > 0 )
            Display_MouseMove(localClientNum, &uiInfo->uiDC);
    }
}

uiMenuCommand_t __cdecl UI_GetActiveMenu(int localClientNum)
{
    return UI_GetInfo(localClientNum)->currentMenuType;
}

const char *__cdecl UI_GetTopActiveMenuName(int localClientNum)
{
    uiInfo_s *uiInfo; // [esp+0h] [ebp-8h]
    int topMenuStackIndex; // [esp+4h] [ebp-4h]

    uiInfo = UI_GetInfo(localClientNum);
    topMenuStackIndex = uiInfo->uiDC.openMenuCount - 1;
    if ( topMenuStackIndex < 0 || topMenuStackIndex >= uiInfo->uiDC.menuCount )
        return 0;
    if ( !uiInfo->uiDC.menuStack[topMenuStackIndex].menu
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                    2057,
                    0,
                    "%s",
                    "uiInfo->uiDC.menuStack[topMenuStackIndex].menu") )
    {
        __debugbreak();
    }
    return uiInfo->uiDC.menuStack[topMenuStackIndex].menu->window.name;
}

int __cdecl UI_IsFullscreen(int localClientNum)
{
    uiInfo_s *uiInfo; // [esp+0h] [ebp-4h]

    uiInfo = UI_GetInfo(localClientNum);
    return Menus_AnyFullScreenVisible(&uiInfo->uiDC);
}

double __cdecl UI_GetBlurRadius(int localClientNum)
{
    uiInfo_s *uiInfo; // [esp+0h] [ebp-4h]

    uiInfo = UI_GetInfo(localClientNum);
    if ( !uiInfo && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp", 2178, 0, "%s", "uiInfo") )
        __debugbreak();
    return uiInfo->uiDC.blurRadiusOut;
}

char errorString[1024];
char *__cdecl UI_SafeTranslateString(const char *reference)
{
    char v2; // [esp+3h] [ebp-11h]
    char *v3; // [esp+8h] [ebp-Ch]
    const char *v4; // [esp+Ch] [ebp-8h]
    const char *translation; // [esp+10h] [ebp-4h]

    if ( *reference == 21 )
    {
        ++reference;
        translation = 0;
    }
    else
    {
        translation = SEH_StringEd_GetString((char*)reference);
    }
    if ( !translation )
    {
        if ( loc_warnings->current.enabled )
        {
            if ( loc_warningsAsErrors->current.enabled )
                Com_Error(ERR_LOCALIZATION, "Could not translate string \"%s\"", reference);
            else
                Com_PrintWarning(13, "WARNING: Could not translate string \"%s\"\n", reference);
            strcpy(errorString, "^1UNLOCALIZED(^7");
            I_strncat(errorString, 1024, reference);
            I_strncat(errorString, 1024, "^1)^7");
        }
        else
        {
            v4 = reference;
            v3 = errorString;
            do
            {
                v2 = *v4;
                *v3++ = *v4++;
            }
            while ( v2 );
        }
        return errorString;
    }
    return (char *)translation;
}

bool __cdecl UI_AnyMenuVisible(int localClientNum)
{
    return UI_GetInfo(localClientNum)->uiDC.openMenuCount != 0;
}

char *__cdecl UI_ReplaceConversionString(char *sourceString, const char *replaceString)
{
    return UI_ReplaceConversionStrings(sourceString, 1, &replaceString);
}

char *__cdecl UI_ReplaceConversionStrings(char *sourceString, int numStrings, const char **replaceStrings)
{
    char outputString[1024]; // [esp+0h] [ebp-430h] BYREF
    int argNum; // [esp+404h] [ebp-2Ch]
    ConversionArguments convArgs; // [esp+408h] [ebp-28h] BYREF

    memset(convArgs.args, 0, sizeof(convArgs.args));
    convArgs.argCount = numStrings;
    for ( argNum = 0; argNum < numStrings; ++argNum )
        convArgs.args[argNum] = replaceStrings[argNum];
    UI_ReplaceConversions(sourceString, &convArgs, outputString, 1024);
    return va("%s", outputString);
}

char *__cdecl UI_ReplaceConversionInt(char *sourceString, int replaceInt)
{
    return UI_ReplaceConversionInts(sourceString, 1, &replaceInt);
}

char *__cdecl UI_ReplaceConversionInts(char *sourceString, int numInts, int *replaceInts)
{
    char tempStrings[16][32]; // [esp+0h] [ebp-630h] BYREF
    char outputString[1024]; // [esp+200h] [ebp-430h] BYREF
    int argNum; // [esp+604h] [ebp-2Ch]
    ConversionArguments convArgs; // [esp+608h] [ebp-28h] BYREF

    memset(convArgs.args, 0, sizeof(convArgs.args));
    convArgs.argCount = numInts;
    for ( argNum = 0; argNum < numInts; ++argNum )
    {
        sprintf(tempStrings[argNum], "%d", replaceInts[argNum]);
        convArgs.args[argNum] = tempStrings[argNum];
    }
    UI_ReplaceConversions(sourceString, &convArgs, outputString, 1024);
    return va("%s", outputString);
}

void __cdecl UI_FilterStringForButtonAnimation(char *str, unsigned int strMaxSize)
{
    unsigned int idx; // [esp+0h] [ebp-8h]

    if ( SEH_GetCurrentLanguage() != 10 && (int)Sys_Milliseconds() % 1000 > 800 )
    {
        for ( idx = 0; str[idx] && idx < strMaxSize; ++idx )
        {
            if ( str[idx] == 16 )
            {
                str[idx] = -68;
            }
            else if ( str[idx] == 17 )
            {
                str[idx] = -67;
            }
        }
    }
}

void __cdecl UI_ReplaceConversions(
                char *sourceString,
                ConversionArguments *arguments,
                char *outputString,
                int outputStringSize)
{
    char *v4; // eax
    signed int v5; // [esp+0h] [ebp-38h]
    int argIndex; // [esp+24h] [ebp-14h]
    int argStringIndex; // [esp+28h] [ebp-10h]
    int index; // [esp+2Ch] [ebp-Ch]
    int outputStringCounter; // [esp+30h] [ebp-8h]
    int sourceStringLength; // [esp+34h] [ebp-4h]

    if ( !outputString
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp", 2348, 0, "%s", "outputString") )
    {
        __debugbreak();
    }
    if ( outputString )
    {
        if ( outputStringSize < 0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp", 2352, 0, "%s", "outputStringSize >= 0") )
        {
            __debugbreak();
        }
        if ( outputStringSize >= 0 )
        {
            memset(outputString, 0, outputStringSize);
            outputStringCounter = 0;
            if ( !sourceString
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp", 2359, 0, "%s", "sourceString") )
            {
                __debugbreak();
            }
            if ( sourceString )
            {
                //strstr((unsigned __int8 *)sourceString, AMPERSAND_2X);
                v4 = strstr(sourceString, "&&");
                if ( v4 )
                {
                    if ( !arguments
                        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp", 2369, 0, "%s", "arguments") )
                    {
                        __debugbreak();
                    }
                    if ( arguments )
                    {
                        if ( arguments->argCount > 9
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                                        2373,
                                        0,
                                        "%s\n\t(arguments->argCount) = %i",
                                        "(arguments->argCount <= 9)",
                                        arguments->argCount) )
                        {
                            __debugbreak();
                        }
                        if ( arguments->argCount <= 9 )
                        {
                            sourceStringLength = strlen(sourceString);
                            if ( sourceStringLength <= 0
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                                            2379,
                                            0,
                                            "%s\n\t(sourceStringLength) = %i",
                                            "(sourceStringLength > 0)",
                                            sourceStringLength) )
                            {
                                __debugbreak();
                            }
                            if ( sourceStringLength > 0 )
                            {
                                index = 0;
                                while ( index < sourceStringLength )
                                {
                                    if ( !strncmp(&sourceString[index], "&&", 2u) && isdigit(sourceString[index + 2]))
                                    {
                                        argIndex = sourceString[index + 2] - 49;
                                        if ( (argIndex < 0 || argIndex >= arguments->argCount)
                                            && !Assert_MyHandler(
                                                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                                                        2392,
                                                        0,
                                                        "%s\n\t(argIndex) = %i",
                                                        "(argIndex >= 0 && argIndex < arguments->argCount)",
                                                        argIndex) )
                                        {
                                            __debugbreak();
                                        }
                                        if ( argIndex < 0 || argIndex >= arguments->argCount )
                                            return;
                                        if ( argIndex >= 9
                                            && !Assert_MyHandler(
                                                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                                                        2395,
                                                        0,
                                                        "%s\n\t(argIndex) = %i",
                                                        "(argIndex < 9)",
                                                        argIndex) )
                                        {
                                            __debugbreak();
                                        }
                                        if ( argIndex >= 9 )
                                            return;
                                        if ( !arguments->args[argIndex]
                                            && !Assert_MyHandler(
                                                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                                                        2398,
                                                        0,
                                                        "%s",
                                                        "arguments->args[argIndex]") )
                                        {
                                            __debugbreak();
                                        }
                                        if ( !arguments->args[argIndex] )
                                            return;
                                        v5 = strlen(arguments->args[argIndex]);
                                        for ( argStringIndex = 0; argStringIndex < v5; ++argStringIndex )
                                            outputString[outputStringCounter++] = arguments->args[argIndex][argStringIndex];
                                        index += 3;
                                    }
                                    else
                                    {
                                        outputString[outputStringCounter++] = sourceString[index++];
                                    }
                                }
                                UI_FilterStringForButtonAnimation(outputString, outputStringSize);
                            }
                        }
                    }
                }
                else
                {
                    I_strncpyz(outputString, sourceString, outputStringSize);
                }
            }
        }
    }
}

void __cdecl UI_CloseAll(int localClientNum)
{
    uiInfo_s *uiInfo; // [esp+0h] [ebp-4h]

    uiInfo = UI_GetInfo(localClientNum);
    Menus_CloseAll(localClientNum, &uiInfo->uiDC);
    UI_SetActiveMenu(localClientNum, UIMENU_NONE);
}

void __cdecl UI_CloseFocusedMenu(int localClientNum)
{
    uiInfo_s *uiInfo; // [esp+0h] [ebp-8h]

    uiInfo = UI_GetInfo(localClientNum);
    if ( Menu_Count(&uiInfo->uiDC) > 0 )
    {
        if ( Menu_GetFocused(&uiInfo->uiDC) )
        {
            if ( !Menus_AnyFullScreenVisible(&uiInfo->uiDC) )
                Menus_CloseAll(localClientNum, &uiInfo->uiDC);
        }
        else if ( Key_IsCatcherActive(localClientNum, 16) )
        {
            Key_RemoveCatcher(localClientNum, -17);
        }
    }
}

bool __cdecl Menu_IsMenuOpenAndVisible(int localClientNum, const char *menuName)
{
    uiInfo_s *uiInfo; // [esp+0h] [ebp-8h]
    menuDef_t *menu; // [esp+4h] [ebp-4h]
    int savedregs; // [esp+8h] [ebp+0h] BYREF

    uiInfo = UI_GetInfo(localClientNum);
    menu = Menus_FindByName(&uiInfo->uiDC, menuName);
    if ( !menu )
        return 0;
    if ( Menus_MenuIsInStack(&uiInfo->uiDC, menu) )
        return Menu_IsVisible(localClientNum, &uiInfo->uiDC, menu) != 0;
    return 0;
}

int __cdecl UI_PopupScriptMenu(int localClientNum, const char *menuName, bool useMouse)
{
    int UIContextIndex; // eax
    uiInfo_s *uiInfo; // [esp+8h] [ebp-8h]
    menuDef_t *pFocus; // [esp+Ch] [ebp-4h]

    uiInfo = UI_GetInfo(localClientNum);
    pFocus = Menu_GetFocused(&uiInfo->uiDC);
    if ( pFocus && uiInfo->currentMenuType != UIMENU_SCRIPT_POPUP && uiInfo->currentMenuType != UIMENU_SCOREBOARD )
        return 0;
    if ( !pFocus || I_stricmp(pFocus->window.name, menuName) )
    {
        uiInfo->currentMenuType = UIMENU_SCRIPT_POPUP;
        if ( !useMouse )
        {
            uiInfo->uiDC.cursor.x = 639.0f;
            uiInfo->uiDC.cursor.y = 479.0f;
            UI_SetSystemCursorPos(&uiInfo->uiDC, 639.0, 479.0);
        }
        Menus_CloseAll(localClientNum, &uiInfo->uiDC);
        Menus_OpenByName(localClientNum, &uiInfo->uiDC, menuName);
        UIContextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
        Key_ContextIndex_SetCatcher(UIContextIndex, 16);
    }
    return 1;
}

void __cdecl UI_ClosePopupScriptMenu(int localClientNum, bool allowResponse)
{
    uiInfo_s *uiInfo; // [esp+0h] [ebp-4h]

    uiInfo = UI_GetInfo(localClientNum);
    if ( uiInfo->currentMenuType == UIMENU_SCRIPT_POPUP )
    {
        uiInfo->allowScriptMenuResponse = allowResponse;
        UI_CloseFocusedMenu(localClientNum);
        uiInfo->allowScriptMenuResponse = 1;
    }
}

bool __cdecl UI_AllowScriptMenuResponse(int localClientNum)
{
    return UI_GetInfo(localClientNum)->allowScriptMenuResponse;
}

void __cdecl UI_CloseInGameMenu(int localClientNum)
{
    if ( !UI_IsFullscreen(localClientNum) && UI_GetActiveMenu(localClientNum) == UIMENU_INGAME )
        UI_CloseFocusedMenu(localClientNum);
}

void __cdecl UI_UpdatePendingPings(uiInfo_s *uiInfo)
{
    LAN_ResetPings(ui_netSource->current.integer);
    sharedUiInfo.serverStatus.refreshActive = 1;
    sharedUiInfo.serverStatus.refreshtime = uiInfo->uiDC.realTime + 1000;
}

int __cdecl UI_CheckExecKey(int localClientNum, int key)
{
    uiInfo_s *uiInfo; // [esp+0h] [ebp-Ch]
    menuDef_t *menu; // [esp+4h] [ebp-8h]
    ItemKeyHandler *handler; // [esp+8h] [ebp-4h]

    uiInfo = UI_GetInfo(localClientNum);
    menu = Menu_GetFocused(&uiInfo->uiDC);
    if ( g_editingField )
        return 1;
    if ( key > 256 )
        return 0;
    if ( !menu )
        return 0;
    for ( handler = menu->onKey; handler; handler = handler->next )
    {
        if ( handler->key == key )
            return 1;
    }
    return 0;
}

int __cdecl UI_OwnerDrawWidth(int ownerDraw, Font_s *font, float scale)
{
    const char *NetSources; // eax
    char *v5; // eax
    const char **ServerFilter; // eax
    char *v7; // eax
    char *v8; // eax
    int filter; // [esp+8h] [ebp-Ch] BYREF
    const char *s; // [esp+10h] [ebp-4h]

    s = 0;
    switch (ownerDraw)
    {
    case 206:
        s = sharedUiInfo.gameTypes[ui_netGameTypeName->current.integer].gameTypeName;
        break;
    case 220:
        if (ui_netSource->current.integer > sharedUiInfo.numJoinGameTypes)
            Dvar_SetInt((dvar_s*)ui_netSource, 0);
        NetSources = UI_GetNetSources(ui_netSource->current.unsignedInt);
        v5 = va("EXE_NETSOURCE %s", NetSources);
        s = SEH_LocalizeTextMessage(v5, "net source", LOCMSG_SAFE);
        break;
    case 222:
        if ((unsigned int)ui_serverFilterType >= 2)
            ui_serverFilterType = 0;
        ServerFilter = UI_GetServerFilter((const char **)&filter, ui_serverFilterType);
        v7 = va("EXE_SERVERFILTER %s", *ServerFilter);
        s = SEH_LocalizeTextMessage(v7, "server filter", LOCMSG_SAFE);
        break;
    case 247:
        v8 = va("ui_lastServerRefresh_%i", ui_netSource->current.integer);
        s = Dvar_GetVariantString(v8);
        break;
    case 250:
        if (Display_KeyBindPending())
            s = UI_SafeTranslateString("EXE_KEYWAIT");
        else
            s = UI_SafeTranslateString("EXE_KEYCHANGE");
        break;
    default:
        break;
    }
    if (s)
        return UI_TextWidth(s, 0, font, scale);
    else
        return 0;
}

char __cdecl UI_DrawRecordLevel(int contextIndex, rectDef_s *rect)
{
    ScreenPlacement *scrPlace; // [esp+1Ch] [ebp-Ch]
    float size; // [esp+20h] [ebp-8h]
    float curLevel; // [esp+24h] [ebp-4h]

    scrPlace = &scrPlaceView[contextIndex];
    curLevel = Voice_GetVoiceLevel();
    if ( curLevel > 0.0 )
    {
        size = (float)(rect->w * curLevel) - 4.0;
        if ( size > 0.0 )
            UI_FillRect(
                scrPlace,
                rect->x + 2.0,
                rect->y + 2.0,
                size,
                rect->h - 4.0,
                rect->horzAlign,
                rect->vertAlign,
                colorRed);
    }
    UI_FillRect(scrPlace, rect->x, rect->y, rect->w, 1.0, rect->horzAlign, rect->vertAlign, colorDkGrey);
    UI_FillRect(
        scrPlace,
        rect->x,
        (float)(rect->y + rect->h) - 1.0,
        rect->w,
        1.0,
        rect->horzAlign,
        rect->vertAlign,
        colorDkGrey);
    UI_FillRect(scrPlace, rect->x, rect->y + 1.0, 1.0, rect->h - 2.0, rect->horzAlign, rect->vertAlign, colorDkGrey);
    UI_FillRect(
        scrPlace,
        (float)(rect->x + rect->w) - 1.0,
        rect->y + 1.0,
        1.0,
        rect->h - 2.0,
        rect->horzAlign,
        rect->vertAlign,
        colorDkGrey);
    return 1;
}

int __cdecl UI_GetTalkerClientNum(int localClientNum, int contextIndex, int num)
{
    signed int client; // [esp+0h] [ebp-8h]
    int talker; // [esp+4h] [ebp-4h]

    talker = 0;
    for ( client = 0; client < com_maxclients->current.integer; ++client )
    {
        if ( !CL_IsClientLocal(client) && CL_IsPlayerTalking(localClientNum, client) )
        {
            if ( talker == num )
                return client;
            ++talker;
        }
    }
    return -1;
}

void __cdecl UI_DrawPrettyLine(
                int contextIndex,
                float *p1,
                float *p2,
                float lineWidth,
                int horzAlign,
                int vertAlign,
                float *color)
{
    float v7; // xmm0_4
    unsigned __int16 CodeConst_GenericParam0; // ax
    unsigned __int16 CodeConst_GenericParam1; // ax
    int i; // [esp+2Ch] [ebp-54h]
    float p2y; // [esp+30h] [ebp-50h] BYREF
    const ScreenPlacement *scrPlace; // [esp+34h] [ebp-4Ch]
    float rectPos[2]; // [esp+38h] [ebp-48h] BYREF
    float scriptVector1[4]; // [esp+40h] [ebp-40h] BYREF
    float rectDim[2]; // [esp+50h] [ebp-30h] BYREF
    float tempH; // [esp+58h] [ebp-28h] BYREF
    float capWidth; // [esp+5Ch] [ebp-24h]
    float p1x; // [esp+60h] [ebp-20h] BYREF
    float p2x; // [esp+64h] [ebp-1Ch] BYREF
    float tempW; // [esp+68h] [ebp-18h] BYREF
    float p1y; // [esp+6Ch] [ebp-14h] BYREF
    float scriptVector0[4]; // [esp+70h] [ebp-10h] BYREF

    scrPlace = &scrPlaceView[contextIndex];
    capWidth = 1.0f;
    tempW = 1.0f;
    tempH = 1.0f;
    for ( i = 0; i < 2; ++i )
    {
        if ( p2[i] <= p1[i] )
        {
            rectPos[i] = p2[i];
            v7 = p1[i] - p2[i];
        }
        else
        {
            rectPos[i] = p1[i];
            v7 = p2[i] - p1[i];
        }
        rectDim[i] = v7;
        rectPos[i] = rectPos[i] - lineWidth;
        rectDim[i] = (float)(lineWidth * 2.0) + rectDim[i];
    }
    tempW = rectDim[0];
    tempH = rectDim[1];
    p1x = *p1;
    p1y = p1[1];
    ScrPlace_ApplyRect(scrPlace, &p1x, &p1y, &tempW, &tempH, horzAlign, vertAlign);
    p2x = *p2;
    p2y = p2[1];
    ScrPlace_ApplyRect(scrPlace, &p2x, &p2y, &tempW, &tempH, horzAlign, vertAlign);
    ScrPlace_ApplyRect(scrPlace, rectPos, &rectPos[1], rectDim, &rectDim[1], horzAlign, vertAlign);
    scriptVector0[0] = rectDim[0];
    scriptVector0[1] = rectDim[1];
    scriptVector0[2] = lineWidth;
    scriptVector0[3] = capWidth;
    CodeConst_GenericParam0 = GetCodeConst_GenericParam0();
    CL_SetCustomConstant(CodeConst_GenericParam0, scriptVector0);
    scriptVector1[0] = p1x - rectPos[0];
    scriptVector1[1] = p1y - rectPos[1];
    scriptVector1[2] = p2x - rectPos[0];
    scriptVector1[3] = p2y - rectPos[1];
    CodeConst_GenericParam1 = GetCodeConst_GenericParam1();
    CL_SetCustomConstant(CodeConst_GenericParam1, scriptVector1);
    CL_DrawStretchPicPhysical(
        rectPos[0],
        rectPos[1],
        rectDim[0],
        rectDim[1],
        0.0,
        0.0,
        1.0,
        1.0,
        color,
        sharedUiInfo.assets.line);
}

void __cdecl UI_OwnerDraw(
                int localClientNum,
                itemDef_s *item,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                int ownerDraw,
                int ownerDrawFlags,
                float *color,
                Material *material,
                rectDef_s parentRect,
                const char *dvarName)
{
    int contextIndex; // [esp+48h] [ebp-1Ch]
    rectDef_s rect; // [esp+4Ch] [ebp-18h] BYREF

    if ( CL_LocalClient_IsCUIFlagSet(localClientNum, 32) )
        CG_OwnerDraw(
            localClientNum,
            parentRect,
            x,
            y,
            w,
            h,
            horzAlign,
            vertAlign,
            ownerDraw,
            ownerDrawFlags,
            color,
            material);
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    rect.horzAlign = horzAlign;
    rect.vertAlign = vertAlign;
    contextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
    if ( contextIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                    3644,
                    0,
                    "%s",
                    "contextIndex != INVALID_UI_CONTEXT") )
    {
        __debugbreak();
    }
    switch ( ownerDraw )
    {
        case 254:
            UI_DrawMapPreview(contextIndex, &rect, color);
            break;
        case 264:
            ProfLoad_DrawOverlay(&rect);
            break;
        case 265:
            UI_DrawRecordLevel(contextIndex, &rect);
            break;
        case 266:
            UI_DrawLocalTalking(contextIndex, &rect, color);
            break;
        case 281:
            UI_DrawPlaylistIcon(contextIndex, &rect, color);
            break;
        case 282:
            UI_DrawCategoryIcon(contextIndex, &rect, color);
            break;
        case 301:
            UI_DrawControllerIcon(localClientNum, contextIndex, &rect, color);
            break;
        case 304:
            UI_DrawCustomEmblem(contextIndex, &rect, color);
            break;
        case 349:
            UI_DrawEmblemIconThumbnail(contextIndex, &rect, color);
            break;
        case 350:
            UI_DrawEmblemColors(contextIndex, &rect);
            break;
        case 352:
            UI_DrawPlayerEmblem(localClientNum, contextIndex, item, &rect, color);
            break;
        case 355:
            UI_PrettyLineOwnerdraw(contextIndex, &rect, item->window.borderSize, color);
            break;
        case 384:
            UI_DrawEmblemSelector(localClientNum, contextIndex, item, &rect, color);
            break;
        case 389:
            UI_DrawPlayerEmblemServer(localClientNum, contextIndex, item, &rect, color);
            break;
        default:
            break;
    }
    UI_Project_OwnerDraw(
        localClientNum,
        item,
        x,
        y,
        w,
        h,
        horzAlign,
        vertAlign,
        ownerDraw,
        ownerDrawFlags,
        color,
        material,
        parentRect,
        dvarName);
}

void __cdecl UI_DrawLocalTalking(int contextIndex, const rectDef_s *rect, const float *color)
{
    Material *material; // [esp+20h] [ebp-4h]

    if ( SV_VoiceEnabled() && cl_voice->current.enabled && IN_IsTalkKeyHeld() )
    {
        material = Material_RegisterHandle("voice_on", 7);
        UI_DrawHandlePic(
            &scrPlaceView[contextIndex],
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            color,
            material);
    }
}

void __cdecl UI_DrawPlaylistIcon(int localClientNum, const rectDef_s *rect, const float *color)
{
    char *playlistIconName; // [esp+20h] [ebp-10h]
    unsigned int playlistId; // [esp+24h] [ebp-Ch]
    Material *mtl; // [esp+28h] [ebp-8h]
    int localControllerIndex; // [esp+2Ch] [ebp-4h]

    localControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    playlistId = Playlist_GetPlaylistIdForNum(localControllerIndex, playlist->current.integer);
    playlistIconName = (char *)Playlist_GetPlaylistIconName(localControllerIndex, playlistId);
    if ( playlistIconName )
    {
        if ( *playlistIconName )
        {
            mtl = Material_RegisterHandle(playlistIconName, 3);
            UI_DrawHandlePic(
                &scrPlaceView[localClientNum],
                rect->x,
                rect->y,
                rect->w,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                color,
                mtl);
        }
    }
}

void __cdecl UI_DrawCategoryIcon(int localClientNum, rectDef_s *rect, float *color)
{
    char *categoryIconName; // [esp+20h] [ebp-10h]
    Material *mtl; // [esp+24h] [ebp-Ch]
    unsigned int categoryId; // [esp+28h] [ebp-8h]
    int localControllerIndex; // [esp+2Ch] [ebp-4h]

    if ( xblive_wagermatch->current.enabled )
        categoryId = Playlist_GetCategoryIdForNum(wagerCategory->current.integer);
    else
        categoryId = Playlist_GetCategoryIdForNum(category->current.integer);
    localControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    categoryIconName = (char *)Playlist_GetCategoryIconName(localControllerIndex, categoryId);
    if ( *categoryIconName )
    {
        mtl = Material_RegisterHandle(categoryIconName, 3);
        UI_DrawHandlePic(
            &scrPlaceView[localClientNum],
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            color,
            mtl);
    }
}

void __cdecl UI_DrawControllerIcon(int localClientNum, int contextIndex, const rectDef_s *rect, const float *color)
{
    char controllerIconName[32]; // [esp+20h] [ebp-2Ch] BYREF
    Material *mtl; // [esp+44h] [ebp-8h]
    int localControllerIndex; // [esp+48h] [ebp-4h]

    localControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( localControllerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                    3563,
                    0,
                    "%s",
                    "localControllerIndex != INVALID_CONTROLLER_PORT") )
    {
        __debugbreak();
    }
    sprintf(controllerIconName, "controller_icon%d", localControllerIndex + 1);
    if ( controllerIconName[0] )
    {
        mtl = Material_RegisterHandle(controllerIconName, 3);
        UI_DrawHandlePic(
            &scrPlaceView[contextIndex],
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            color,
            mtl);
    }
}

void __cdecl UI_PrettyLineOwnerdraw(int contextIndex, rectDef_s *rect, float lineWidth, float *color)
{
    float pt1[2]; // [esp+10h] [ebp-10h] BYREF
    float pt2[2]; // [esp+18h] [ebp-8h] BYREF

    pt1[0] = rect->x;
    pt1[1] = rect->y;
    pt2[0] = rect->x + rect->w;
    pt2[1] = rect->y + rect->h;
    UI_DrawPrettyLine(contextIndex, pt1, pt2, lineWidth, rect->horzAlign, rect->vertAlign, color);
}

void __cdecl UI_OwnerDrawText(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                float text_x,
                float text_y,
                int ownerDraw,
                int ownerDrawFlags,
                int align,
                Font_s *font,
                float scale,
                float *color,
                Material *material,
                int textStyle,
                rectDef_s parentRect,
                int textAlignMode,
                char *text)
{
    rectDef_s rect; // [esp+64h] [ebp-18h] BYREF

    if ( localClientNum == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                    3722,
                    0,
                    "%s",
                    "localClientNum != INVALID_CLIENT_NUMBER") )
    {
        __debugbreak();
    }
    if ( CL_LocalClient_IsCUIFlagSet(localClientNum, 32) )
        CG_OwnerDrawText(
            localClientNum,
            parentRect,
            x,
            y,
            w,
            h,
            horzAlign,
            vertAlign,
            text_x,
            text_y,
            ownerDraw,
            ownerDrawFlags,
            align,
            font,
            scale,
            color,
            material,
            textStyle,
            textAlignMode,
            item,
            text);
    rect.x = x + text_x;
    rect.y = y + text_y;
    rect.w = w;
    rect.h = h;
    rect.horzAlign = horzAlign;
    rect.vertAlign = vertAlign;
    switch ( ownerDraw )
    {
        case 206:
            UI_DrawGameType(contextIndex, &rect, font, scale, color, textStyle);
            break;
        case 220:
            UI_DrawNetSource(contextIndex, &rect, font, scale, color, textStyle);
            break;
        case 222:
            UI_DrawNetFilter(contextIndex, &rect, font, scale, color, textStyle);
            break;
        case 245:
            UI_DrawNetGameType(contextIndex, &rect, font, scale, color, textStyle);
            break;
        case 247:
            UI_DrawServerRefreshDate(contextIndex, &rect, font, scale, color, textStyle);
            break;
        case 250:
            UI_DrawKeyBindStatus(contextIndex, &rect, font, scale, color, textStyle);
            break;
        case 253:
            UI_DrawJoinGameType(contextIndex, &rect, font, scale, color, textStyle);
            break;
        case 267:
        case 268:
        case 269:
        case 270:
            UI_DrawTalkerNum(localClientNum, contextIndex, ownerDraw - 267, &rect, font, color, scale, textStyle);
            break;
        case 272:
            UI_DrawLoggedInUser(localClientNum, contextIndex, &rect, font, scale, color, textStyle);
            break;
        case 273:
            UI_DrawReservedSlots(contextIndex, &rect, font, color, scale, textStyle);
            break;
        case 275:
            UI_DrawPlaylistDescription(localClientNum, &rect, font, color, scale, textStyle, textAlignMode);
            break;
        case 276:
            UI_DrawLoggedInUserName(localClientNum, contextIndex, &rect, font, scale, color, textStyle);
            break;
        case 302:
            UI_DrawScrollingText(contextIndex, &rect, font, scale, color, textStyle);
            break;
        case 303:
            UI_DrawScrollingTextCategory(contextIndex, &rect, font, scale, color, textStyle);
            break;
        case 310:
            UI_DrawJoinMod(contextIndex, &rect, font, scale, color, textStyle);
            break;
        default:
            break;
    }
    UI_Project_OwnerDrawText(
        localClientNum,
        contextIndex,
        item,
        x,
        y,
        w,
        h,
        horzAlign,
        vertAlign,
        text_x,
        text_y,
        ownerDraw,
        ownerDrawFlags,
        align,
        font,
        scale,
        color,
        material,
        textStyle,
        parentRect,
        textAlignMode);
}

void __cdecl UI_DrawKeyBindStatus(
                int contextIndex,
                rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                int textStyle)
{
    char *v6; // eax
    float x; // [esp+0h] [ebp-24h]
    float y; // [esp+4h] [ebp-20h]
    int horzAlign; // [esp+8h] [ebp-1Ch]
    int vertAlign; // [esp+Ch] [ebp-18h]
    const char *v11; // [esp+1Ch] [ebp-8h]

    if ( Display_KeyBindPending() )
        v11 = "EXE_KEYWAIT";
    else
        v11 = "EXE_KEYCHANGE";
    vertAlign = rect->vertAlign;
    horzAlign = rect->horzAlign;
    y = rect->y;
    x = rect->x;
    v6 = UI_SafeTranslateString(v11);
    UI_DrawText(&scrPlaceView[contextIndex], v6, 0x7FFFFFFF, font, x, y, horzAlign, vertAlign, scale, color, textStyle);
}

void __cdecl UI_DrawTalkerNum(
                int localClientNum,
                int contextIndex,
                int num,
                rectDef_s *rect,
                Font_s *font,
                float *color,
                float textScale,
                int style)
{
    int client; // [esp+20h] [ebp-38h]
    Material *material; // [esp+24h] [ebp-34h]
    char name[40]; // [esp+28h] [ebp-30h] BYREF
    int textHeight; // [esp+54h] [ebp-4h]

    client = UI_GetTalkerClientNum(localClientNum, contextIndex, num);
    if ( client >= 0 )
    {
        if ( CL_GetClientName(localClientNum, client, name, 38, 1) )
        {
            material = Material_RegisterHandle("voice_on", 7);
            textHeight = UI_TextHeight(font, textScale);
            UI_DrawHandlePic(
                &scrPlaceView[contextIndex],
                rect->x,
                rect->y,
                rect->w,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                color,
                material);
            UI_DrawText(
                &scrPlaceView[contextIndex],
                name,
                32,
                font,
                (float)(rect->x + rect->w) + 2.0,
                (float)((float)textHeight + rect->y) + (float)((float)(rect->h - (float)textHeight) / 2.0),
                rect->horzAlign,
                rect->vertAlign,
                textScale,
                color,
                style);
        }
        else
        {
            Com_PrintWarning(13, "Unable to find client %i but they are talking\n", client);
        }
    }
}

void __cdecl UI_DrawReservedSlots(
                int contextIndex,
                rectDef_s *rect,
                Font_s *font,
                float *color,
                float textScale,
                int style)
{
    int PrivateClients; // eax
    char *s; // [esp+20h] [ebp-4h]

    PrivateClients = Com_GetPrivateClients();
    s = va("%i", PrivateClients - 1);
    UI_TextHeight(font, textScale);
    UI_DrawText(
        &scrPlaceView[contextIndex],
        s,
        0x7FFFFFFF,
        font,
        (float)(rect->x + rect->w) + 2.0,
        rect->y,
        rect->horzAlign,
        rect->vertAlign,
        textScale,
        color,
        style);
}

void __cdecl UI_DrawLoggedInUser(
                int localClientNum,
                int contextIndex,
                rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                int textStyle)
{
    char *v7; // eax
    int v8; // eax
    char *v9; // eax
    char *v10; // eax
    char *name; // [esp+1Ch] [ebp-8h]
    float xadj; // [esp+20h] [ebp-4h]

    name = (char *)Dvar_GetString("com_playerProfile");
    if ( I_strcmp(name, "default") )
    {
        if ( Dvar_GetBool("dw_loggedin") )
        {
            v9 = UI_SafeTranslateString("PLATFORM_ONLINE");
            name = va("%s: %s", v9, name);
            v8 = UI_TextWidth(name, 0x7FFFFFFF, font, scale);
        }
        else
        {
            if ( !Dvar_GetBool("dw_loggedin") )
            {
                v10 = UI_SafeTranslateString("PLATFORM_OFFLINE");
                name = va("%s: %s", v10, name);
            }
            v8 = UI_TextWidth(name, 0x7FFFFFFF, font, scale);
        }
    }
    else
    {
        v7 = UI_SafeTranslateString("PLATFORM_OFFLINE");
        name = va("%s", v7);
        v8 = UI_TextWidth(name, 0x7FFFFFFF, font, scale);
    }
    xadj = UI_GetTextAlignAdj(2, rect->w, (float)v8);
    UI_DrawText(
        &scrPlaceFull,
        name,
        0x7FFFFFFF,
        font,
        rect->x + xadj,
        rect->y,
        rect->horzAlign,
        rect->vertAlign,
        scale,
        color,
        textStyle);
}

double __cdecl UI_GetTextAlignAdj(int alignment, float width, float textWidth)
{
    if ( alignment == 1 )
    {
        if ( (float)(0.0 - (float)((float)(width - textWidth) * 0.5)) < 0.0 )
            return (float)((float)(width - textWidth) * 0.5);
        else
            return 0.0f;
    }
    else if ( alignment == 2 )
    {
        if ( (float)(0.0 - (float)(width - textWidth)) < 0.0 )
            return (float)(width - textWidth);
        else
            return 0.0f;
    }
    else
    {
        if ( alignment )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                            3114,
                            0,
                            "%s\n\t(alignment) = %i",
                            "(alignment == 0)",
                            alignment) )
                __debugbreak();
        }
        return 0.0;
    }
}

void __cdecl UI_DrawLoggedInUserName(
                int localClientNum,
                int contextIndex,
                rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                int textStyle)
{
    char *v7; // eax
    char *v8; // eax
    float x; // [esp+0h] [ebp-24h]
    float y; // [esp+4h] [ebp-20h]
    int horzAlign; // [esp+8h] [ebp-1Ch]
    int vertAlign; // [esp+Ch] [ebp-18h]
    const char *name; // [esp+1Ch] [ebp-8h]
    char *namea; // [esp+1Ch] [ebp-8h]
    int localControllerIndex; // [esp+20h] [ebp-4h]

    localControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    name = CL_ControllerIndex_GetUsername();
    if ( *name )
    {
        v8 = Clan_GetName(localControllerIndex);
        namea = (char *)Com_DisplayName(name, v8, 3);
        UI_DrawText(
            &scrPlaceFull,
            namea,
            0x7FFFFFFF,
            font,
            rect->x,
            rect->y,
            rect->horzAlign,
            rect->vertAlign,
            scale,
            color,
            textStyle);
    }
    else
    {
        vertAlign = rect->vertAlign;
        horzAlign = rect->horzAlign;
        y = rect->y;
        x = rect->x;
        v7 = UI_SafeTranslateString("XBOXLIVE_NOTSIGNEDIN");
        UI_DrawText(&scrPlaceFull, v7, 0x7FFFFFFF, font, x, y, horzAlign, vertAlign, scale, color, textStyle);
    }
}

void __cdecl UI_DrawPlaylistDescription(
                int localClientNum,
                rectDef_s *rect,
                Font_s *font,
                float *color,
                float textScale,
                int style,
                char textAlignment)
{
    int ControllerIndex; // eax
    unsigned int Xp; // eax
    int v9; // eax
    int v10; // eax
    char *v11; // eax
    int v12; // eax
    char *v13; // eax
    char *v14; // eax
    const char *v15; // eax
    int Prestige; // [esp+10h] [ebp-44h]
    int scale; // [esp+18h] [ebp-3Ch]
    int scalea; // [esp+18h] [ebp-3Ch]
    int scaleb; // [esp+18h] [ebp-3Ch]
    int scalec; // [esp+18h] [ebp-3Ch]
    int playlistId; // [esp+20h] [ebp-34h]
    PlaylistLockState lockState; // [esp+24h] [ebp-30h]
    const char *desc; // [esp+30h] [ebp-24h]
    rectDef_s textRect; // [esp+34h] [ebp-20h] BYREF
    int localControllerIndex; // [esp+4Ch] [ebp-8h]
    int partyMemberCount; // [esp+50h] [ebp-4h]

    scale = playlist->current.integer;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    playlistId = Playlist_GetPlaylistIdForNum(ControllerIndex, scale);
    localControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    partyMemberCount = 1;
    Prestige = LiveStats_GetPrestige(localControllerIndex);
    Xp = LiveStats_GetXp(localControllerIndex);
    lockState = (PlaylistLockState)Playlist_IsLocked(localControllerIndex, playlistId, Xp, Prestige, 0, 1);
    switch ( lockState )
    {
        case PLS_UNLOCKED:
            v9 = Com_LocalClient_GetControllerIndex(localClientNum);
            desc = Playlist_GetPlaylistDescription(v9, playlistId);
            if ( desc && *desc )
                goto LABEL_21;
            return;
        case PLS_FETCHING:
            return;
        case PLS_XP:
            desc = GetXpLockDescription(localClientNum, playlistId);
            goto LABEL_21;
        case PLS_PLEVEL:
            desc = GetPLevelLockDescription(localClientNum, playlistId);
            goto LABEL_21;
        case PLS_MAXPARTY:
            v10 = Com_LocalClient_GetControllerIndex(localClientNum);
            scalea = Playlist_GetMaxPartySize(v10, playlistId);
            v11 = UI_SafeTranslateString("MPUI_MAXPARTYSIZE");
            desc = UI_ReplaceConversionInt(v11, scalea);
            goto LABEL_21;
        case PLS_MINPARTY:
            v12 = Com_LocalClient_GetControllerIndex(localClientNum);
            scaleb = Playlist_GetMinPartySize(v12, playlistId);
            v13 = UI_SafeTranslateString("MPUI_MINPARTYSIZE");
            desc = UI_ReplaceConversionInt(v13, scaleb);
            goto LABEL_21;
        case PLS_GAMETYPEBAN:
            desc = UI_SafeTranslateString("MPUI_GAMETYPE_BAN");
            goto LABEL_21;
        case PLS_COLLECTORS:
            desc = UI_SafeTranslateString("MPUI_COLLECTORS_ONLY");
            goto LABEL_21;
        case PLS_REQUIRED_DLC_DISABLED:
            desc = UI_SafeTranslateString("MP_PLAYLIST_MAPS_NOT_ENABLED");
            goto LABEL_21;
        case PLS_REQUIRED_DLC_NOT_AVAILABLE:
            desc = UI_SafeTranslateString("MPUI_PLAYLIST_REQUIRES_DLC");
            goto LABEL_21;
        case PLS_CODPOINTS:
            scalec = Playlist_GetWagerBet(localControllerIndex, playlistId);
            v14 = UI_SafeTranslateString("MPUI_WAGERMATCH_NOT_ENOUGH_CP");
            desc = UI_ReplaceConversionInt(v14, scalec);
            goto LABEL_21;
        case PLS_VISDVAR:
            desc = UI_SafeTranslateString("UIPLS_PLAYLIST_NOTVISIBLE_DESC");
            goto LABEL_21;
        case PLS_LOCALPLAYERS:
            desc = UI_SafeTranslateString("MPUI_PLAYLIST_LOCALPLAYERS");
            goto LABEL_21;
        case PLS_NOGUESTS:
            desc = UI_SafeTranslateString("MPUI_PLAYLIST_NOGUESTS_DESC");
LABEL_21:
            UI_TextHeight(font, textScale);
            UI_DrawWrappedText(
                &scrPlaceView[localClientNum],
                desc,
                rect,
                font,
                rect->x,
                rect->y,
                textScale,
                color,
                style,
                textAlignment,
                &textRect);
            break;
        default:
            v15 = va("Unknown playlist lock state: %i\n", lockState);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp", 3381, 0, v15) )
                __debugbreak();
            break;
    }
}

char *__cdecl GetXpLockDescription(int localClientNum, int playlistId)
{
    int UnlockXp; // eax
    const char *RankData; // eax
    const char *v4; // eax
    char *v5; // eax
    int controllerIndex; // [esp+0h] [ebp-440h]
    int rank; // [esp+4h] [ebp-43Ch]
    char outputString[1024]; // [esp+8h] [ebp-438h] BYREF
    char tempString[8]; // [esp+408h] [ebp-38h] BYREF
    ConversionArguments convArgs; // [esp+414h] [ebp-2Ch] BYREF
    const char *rankString; // [esp+43Ch] [ebp-4h]

    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( controllerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                    3204,
                    0,
                    "%s",
                    "controllerIndex != INVALID_CONTROLLER_PORT") )
    {
        __debugbreak();
    }
    UnlockXp = Playlist_GetUnlockXp(controllerIndex, playlistId);
    rank = CL_GetRankForXp(abs(UnlockXp));
    RankData = CL_GetRankData(rank, MP_RANKTABLE_FULLRANK);
    rankString = UI_SafeTranslateString(RankData);
    v4 = CL_GetRankData(rank, MP_RANKTABLE_DISPLAYLEVEL);
    Com_sprintf(tempString, 5u, "%s", v4);
    if ( !*rankString )
        Com_PrintWarning(13, "Empty rank string for rank %i\n", rank);
    memset(&convArgs.args[2], 0, 28);
    convArgs.argCount = 2;
    convArgs.args[0] = rankString;
    convArgs.args[1] = tempString;
    if ( Playlist_GetUnlockXp(controllerIndex, playlistId) >= 0 )
        v5 = UI_SafeTranslateString("MPUI_UNLOCKXP");
    else
        v5 = UI_SafeTranslateString("MPUI_LOCKXP");
    UI_ReplaceConversions(v5, &convArgs, outputString, 1024);
    return va("%s", outputString);
}

char *__cdecl GetPLevelLockDescription(int localClientNum, int playlistId)
{
    char *v2; // eax
    int UnlockPLevel; // [esp-4h] [ebp-10h]
    int controllerIndex; // [esp+4h] [ebp-8h]
    char *outputString; // [esp+8h] [ebp-4h]

    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( controllerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                    3232,
                    0,
                    "%s",
                    "controllerIndex != INVALID_CONTROLLER_PORT") )
    {
        __debugbreak();
    }
    UnlockPLevel = Playlist_GetUnlockPLevel(controllerIndex, playlistId);
    v2 = UI_SafeTranslateString("CLASS_PRESTIGE_UNLOCK_DESC");
    outputString = UI_ReplaceConversionInt(v2, UnlockPLevel);
    return va("%s", outputString);
}

void __cdecl UI_DrawScrollingText(
                int contextIndex,
                rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                int style)
{
    float scrX; // [esp+4h] [ebp-4Ch]
    float baseX; // [esp+8h] [ebp-48h]
    float baseY; // [esp+Ch] [ebp-44h]
    float baseAlpha; // [esp+10h] [ebp-40h]
    rectDef_s scissorRect; // [esp+28h] [ebp-28h] BYREF
    float outXCoord; // [esp+40h] [ebp-10h] BYREF
    float outAlpha; // [esp+44h] [ebp-Ch] BYREF
    const char *text; // [esp+48h] [ebp-8h]
    float outYCoord; // [esp+4Ch] [ebp-4h] BYREF

    baseAlpha = color[3];
    baseY = rect->y;
    baseX = rect->x;
    scrX = ScrPlace_ApplyX(&scrPlaceView[contextIndex], baseX, rect->horzAlign);
    text = LiveTicker_GetCurrentMessage(scrX, baseX, baseY, baseAlpha, font, scale, &outXCoord, &outYCoord, &outAlpha);
    color[3] = 1.0f;
    scissorRect = *rect;
    ScrPlace_ApplyRect(
        &scrPlaceView[contextIndex],
        &scissorRect.x,
        &scissorRect.y,
        &scissorRect.w,
        &scissorRect.h,
        scissorRect.horzAlign,
        scissorRect.vertAlign);
    scissorRect.y = scissorRect.y - scissorRect.h;
    R_AddCmdSetScissorValues(1, (int)scissorRect.x, (int)scissorRect.y, (int)scissorRect.w, (int)scissorRect.h);
    UI_DrawText(
        &scrPlaceView[contextIndex],
        (char *)text,
        0x7FFFFFFF,
        font,
        outXCoord,
        outYCoord,
        rect->horzAlign,
        rect->vertAlign,
        scale,
        color,
        style);
    R_AddCmdSetScissorValues(0, 0, 0, 0, 0);
}

void __cdecl UI_DrawScrollingTextCategory(
                int contextIndex,
                rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                int style)
{
    int v6; // eax
    float outScale; // [esp+20h] [ebp-10h] BYREF
    float outXCoord; // [esp+24h] [ebp-Ch] BYREF
    float outAlpha; // [esp+28h] [ebp-8h] BYREF
    const char *text; // [esp+2Ch] [ebp-4h]

    text = LiveTicker_GetCurrentMessageCategory(color[3], rect->x, rect->w, font, scale, &outXCoord, &outAlpha, &outScale);
    color[3] = outAlpha;
    v6 = UI_TextWidth(text, 32, font, scale);
    Dvar_SetIntIfChanged((dvar_s *)tickerHeaderWidth, v6);
    UI_DrawText(
        &scrPlaceView[contextIndex],
        (char *)text,
        0x7FFFFFFF,
        font,
        outXCoord,
        rect->y,
        rect->horzAlign,
        rect->vertAlign,
        outScale,
        color,
        style);
}

bool __cdecl UI_OwnerDrawVisible(__int16 flags)
{
    bool vis; // [esp+0h] [ebp-4h]

    vis = 1;
    if ( (flags & 4) != 0 )
        vis = ui_netSource->current.integer == 3;
    if ( (flags & 0x1000) != 0 && ui_netSource->current.integer == 3 )
        return 0;
    return vis;
}

int __cdecl UI_OwnerDrawHandleKey(int ownerDraw, int flags, int key)
{
    switch ( ownerDraw )
    {
        case 206:
            return UI_GameType_HandleKey(flags, key, 1);
        case 220:
            UI_NetSource_HandleKey(flags, key);
            return 0;
        case 222:
            UI_NetFilter_HandleKey(flags, key);
            return 0;
        case 245:
            return UI_NetGameType_HandleKey(flags, key);
        case 253:
            return UI_JoinGameType_HandleKey(flags, key);
        case 310:
            return UI_JoinMod_HandleKey(flags, key);
        default:
            return 0;
    }
}

void __cdecl UI_InitOnceForAllClients()
{
    const char *v0; // eax
    double Float; // st6
    Font_s *FontHandleDefault; // eax

    UI_InitUIInfos();
    if ( useFastFile->current.enabled )
        DB_ResetZoneSize(0);
    //if ( !G_ExitAfterConnectPaths() && !useFastFile->current.enabled )
        //BLOPS_NULLSUB();
    String_Init();
    UI_RegisterDvars();
    UI_AssetCache();
    UI_GetGameTypesList();
    
    iassert(sharedUiInfo.numGameTypes <= ARRAY_COUNT(sharedUiInfo.gameTypes));
    ui_netGameType = _Dvar_RegisterInt("ui_netGametype", 0, 0, sharedUiInfo.numGameTypes - 1, 1u, "Game type");

    UI_LoadMods();
    UI_LoadArenas();
    UI_LoadCustomMatchGameTypes();
    UI_LoadMaps();
    if ( g_mapname[0] && !useFastFile->current.enabled )
    {
        v0 = va("%s%s.csv", "maps/mp/", g_mapname);
        UI_MapLoadInfo(v0);
    }
    UI_ServersSort(8);
    Float = Dvar_GetFloat("m_pitch");
    Dvar_SetBoolByName("ui_mousePitch", Float < 0.0);
    if ( ui_netGameType->current.integer >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                    4009,
                    0,
                    "%s\n\t(ui_netGameType->current.integer) = %i",
                    "(ui_netGameType->current.integer >= 0 && ui_netGameType->current.integer < (sizeof( sharedUiInfo.gameTypes ) /"
                    " (sizeof( sharedUiInfo.gameTypes[0] ) * (sizeof( sharedUiInfo.gameTypes ) != 4 || sizeof( sharedUiInfo.gameTyp"
                    "es[0] ) <= 4))))",
                    ui_netGameType->current.integer) )
    {
        __debugbreak();
    }
    Dvar_SetString((dvar_s*)ui_netGameTypeName, sharedUiInfo.gameTypes[ui_netGameType->current.integer].gameType);
    uiscript_debug = _Dvar_RegisterInt("uiscript_debug", 0, 0, 2, 0, "spam debug info for the ui script");
    UI_Project_InitOnceForAllClients();
    FontHandleDefault = UI_GetFontHandleDefault(0.315);
    SD_TruncateAudioDeviceNames(FontHandleDefault, 0.315, 280);
}

void UI_RegisterDvars()
{
    ui_animSpeedScale = _Dvar_RegisterFloat(
                                                "ui_animSpeedScale",
                                                1.0,
                                                0.001,
                                                3.4028235e38,
                                                0x80u,
                                                "Scales the UI animation speed.");
    ui_customModeIndex = _Dvar_RegisterInt(
                                                 "ui_customModeIndex",
                                                 0,
                                                 -1,
                                                 32,
                                                 0,
                                                 "Custom game mode selected index into sharedUiInfo.customGameTypes");
    ui_customModeName = _Dvar_RegisterString("ui_customModeName", (char *)"", 0, "Custom game mode name");
    ui_customModeEditName = _Dvar_RegisterString(
                                                        "ui_customModeEditName",
                                                        (char *)"",
                                                        0,
                                                        "Name to give the currently edited custom game mode when editing is complete");
    ui_customModeDesc = _Dvar_RegisterString(
                                                "ui_customModeDesc",
                                                (char *)"",
                                                0,
                                                "Description for the custom game mode.");
    ui_customModeEditDesc = _Dvar_RegisterString(
                                                        "ui_customModeEditDesc",
                                                        (char *)"",
                                                        0,
                                                        "Description for the custom game mode while editing.");
    ui_customClassName = _Dvar_RegisterString("ui_customClassName", (char *)"", 0, "Custom Class name");
    g_allowvote = _Dvar_RegisterBool("g_allowvote", 1, 1u, 0);
    cg_brass = _Dvar_RegisterBool("cg_brass", 1, 1u, 0);
    fx_marks = _Dvar_RegisterBool("fx_marks", 1, 1u, 0);
    server1 = _Dvar_RegisterString("server1", (char *)"", 1u, "Server display");
    server2 = _Dvar_RegisterString("server2", (char *)"", 1u, "Server display");
    server3 = _Dvar_RegisterString("server3", (char *)"", 1u, "Server display");
    server4 = _Dvar_RegisterString("server4", (char *)"", 1u, "Server display");
    server5 = _Dvar_RegisterString("server5", (char *)"", 1u, "Server display");
    server6 = _Dvar_RegisterString("server6", (char *)"", 1u, "Server display");
    server7 = _Dvar_RegisterString("server7", (char *)"", 1u, "Server display");
    server8 = _Dvar_RegisterString("server8", (char *)"", 1u, "Server display");
    server9 = _Dvar_RegisterString("server9", (char *)"", 1u, "Server display");
    server10 = _Dvar_RegisterString("server10", (char *)"", 1u, "Server display");
    server11 = _Dvar_RegisterString("server11", (char *)"", 1u, "Server display");
    server12 = _Dvar_RegisterString("server12", (char *)"", 1u, "Server display");
    server13 = _Dvar_RegisterString("server13", (char *)"", 1u, "Server display");
    server14 = _Dvar_RegisterString("server14", (char *)"", 1u, "Server display");
    server15 = _Dvar_RegisterString("server15", (char *)"", 1u, "Server display");
    server16 = _Dvar_RegisterString("server16", (char *)"", 1u, "Server display");
    ui_netSource = _Dvar_RegisterInt(
                                     "ui_netSource",
                                     1,
                                     0,
                                     5,
                                     0,
                                     "The network source where:\n    0:Unranked\n    1:Ranked\n    2:Wager\n\t\t3:Favourites\n    4:Friends");
    ui_smallFont = _Dvar_RegisterFloat("ui_smallFont", 0.25, 0.0, 1.0, 1u, "Small font scale");
    ui_bigFont = _Dvar_RegisterFloat("ui_bigFont", 0.40000001, 0.0, 1.0, 1u, "Big font scale");
    ui_extraBigFont = 0;
    ui_currentMap = _Dvar_RegisterInt("ui_currentMap", 0, 0, 0x7FFFFFFF, 1u, "Current map index");
    ui_gametype = _Dvar_RegisterString("ui_gametype", (char *)"", 0, "Current game type");
    ui_mapname = _Dvar_RegisterString("ui_mapname", (char *)"", 0, "Current map name");
    ui_demoname = _Dvar_RegisterString("ui_demoname", (char *)"", 0, "Current selected demo name");
    ui_currentRecentPlayer = _Dvar_RegisterInt("ui_currentRecentPlayer", 0, 0, 100, 0, "Selected recent player");
    ui_serverStatusTimeOut = _Dvar_RegisterInt(
                                                         "ui_serverStatusTimeOut",
                                                         7000,
                                                         0,
                                                         0x7FFFFFFF,
                                                         1u,
                                                         "Time in milliseconds before a server status request times out");
    ui_buildLocation = _Dvar_RegisterVec2(
                                             "ui_buildLocation",
                                             (16.0),
                                             (0.0),
                                             -10000.0,
                                             10000.0,
                                             0,
                                             "Where to draw the build number");
    ui_buildSize = _Dvar_RegisterFloat("ui_buildSize", 0.2, 0.0, 1.0, 0, "Font size to use for the build number");
    ui_showList = _Dvar_RegisterBool("ui_showList", 0, 0, "Show onscreen list of currently visible menus");
    ui_showMenuOnly = _Dvar_RegisterString(
                                            "ui_showMenuOnly",
                                            (char *)"",
                                            0,
                                            "If set, only menus using this name will draw.");
    ui_menuLvlNotify = _Dvar_RegisterBool(
                                             "ui_menuLvlNotify",
                                             0,
                                             0x80u,
                                             "If set, send code notification to script; default off");
    selectedFriendName = _Dvar_RegisterString(
                                                 "selectedFriendName",
                                                 (char *)"",
                                                 0,
                                                 "The name of the currently selected friend.");
    selectedFriendClanTag = _Dvar_RegisterString(
                                                        "selectedFriendClanTag",
                                                        (char *)"",
                                                        0,
                                                        "The clan tag of the currently selected friend.");
    selectedPlayerClanName = _Dvar_RegisterString(
                                                         "selectedPlayerClanName",
                                                         (char *)"",
                                                         0,
                                                         "The name of the selected player clan name");
    selectedFriendIndex = _Dvar_RegisterInt(
                                                    "selectedFriendIndex",
                                                    0,
                                                    0x80000000,
                                                    0x7FFFFFFF,
                                                    0x40u,
                                                    "Currently selected friend index.");
    selectedStoreItemIndex = _Dvar_RegisterInt(
                                                         "selectedStoreItemIndex",
                                                         0,
                                                         0x80000000,
                                                         0x7FFFFFFF,
                                                         0x40u,
                                                         "Currently selected in-game store item index");
    selectedGroupIndex = _Dvar_RegisterInt(
                                                 "selectedGroupIndex",
                                                 0,
                                                 0x80000000,
                                                 0x7FFFFFFF,
                                                 0x40u,
                                                 "Currently selected group index");
    selectedPlayerXuid = _Dvar_RegisterString(
                                                 "selectedPlayerXuid",
                                                 (char *)"",
                                                 0,
                                                 "Currently selected player xuid.");
    selectedMenuItemIndex = _Dvar_RegisterInt(
                                                        "selectedMenuItemIndex",
                                                        0,
                                                        0x80000000,
                                                        0x7FFFFFFF,
                                                        0,
                                                        "Current selected event index.");
    highlightedPlayerXuid = _Dvar_RegisterString(
                                                        "highlightedPlayerXuid",
                                                        (char *)"",
                                                        0,
                                                        "Currently highlighted player xuid.");
    highlightedPlayerName = _Dvar_RegisterString(
                                                        "highlightedPlayerName",
                                                        (char *)"",
                                                        0,
                                                        "Currently highlighted player name.");
    highlightedPlayerClanTag = _Dvar_RegisterString(
                                                             "highlightedPlayerClanTag",
                                                             (char *)"",
                                                             0,
                                                             "Currently highlighted player clan tag.");
    splitscreen_lobbyPlayerCount = _Dvar_RegisterString(
                                                                     "splitscreen_lobbyPlayerCount",
                                                                     (char *)"",
                                                                     0,
                                                                     "Splitscreen lobby player count string.");
    splitscreen_partyPlayerCount = _Dvar_RegisterString(
                                                                     "splitscreen_partyPlayerCount",
                                                                     (char *)"",
                                                                     0,
                                                                     "Splitscreen lobby party player count string.");
    tickerHeaderWidth = _Dvar_RegisterInt(
                                                "tickerHeaderWidth",
                                                0,
                                                0x80000000,
                                                0x7FFFFFFF,
                                                0,
                                                "The width of the ticker header text used to align the the ticker message.");
    ui_borderLowLightScale = _Dvar_RegisterFloat(
                                                         "ui_borderLowLightScale",
                                                         0.60000002,
                                                         0.0,
                                                         1.0,
                                                         0,
                                                         "Scales the border color for the lowlight color on certain UI borders");
    ui_cinematicsTimestamp = _Dvar_RegisterBool(
                                                         "ui_cinematicsTimestamp",
                                                         0,
                                                         0,
                                                         "Shows cinematics timestamp on subtitle UI elements.");
    ui_connectScreenTextGlowColor = _Dvar_RegisterVec4(
                                                                        "ui_connectScreenTextGlowColor",
                                                                        (0.0),
                                                                        (0.0),
                                                                        (0.0),
                                                                        (1.0),
                                                                        0.0,
                                                                        1.0,
                                                                        0,
                                                                        "Glow color applied to the mode and map name strings on the connect screen.");
    ui_drawCrosshair = _Dvar_RegisterBool("ui_drawCrosshair", 1, 1u, "Whether to draw crosshairs.");
    ui_onCloseArgs = _Dvar_RegisterString(
                                         "ui_onCloseArgs",
                                         (char *)"",
                                         0x40u,
                                         "Passes on close arguments to a menu.");
    notice_onEscArg = _Dvar_RegisterString("notice_onEscArg", (char *)"", 0x40u, "UI Notice EscArg");
    statusinfo_onEscArg =_Dvar_RegisterString("statusinfo_onEscArg", (char *)"", 0x40u, "UI Statusinfo EscArg");
    ui_ethernetLinkActive = _Dvar_RegisterBool(
                                                        "ui_ethernetLinkActive",
                                                        1,
                                                        0,
                                                        "true if a network connection is available(and active)");
    ui_listboxIndex = _Dvar_RegisterInt(
                                            "ui_listboxIndex",
                                            0,
                                            0x80000000,
                                            0x7FFFFFFF,
                                            0,
                                            "Current position in a listbox. Used to position associated drop down menu.");
    ui_readingSaveDevice = _Dvar_RegisterBool(
                                                     "ui_readingSaveDevice",
                                                     0,
                                                     0x40u,
                                                     "Whether or not the reading save device menu is open.");
    ui_signedInToProfile = _Dvar_RegisterBool(
                                                     "ui_signedInToProfile",
                                                     0,
                                                     0x40u,
                                                     "Whether or not the profile has been read.");
    emblem_scroll_delay_first = _Dvar_RegisterInt("emblem_scroll_delay_first", 150, 0, 1000, 0, "First repeat delay for emblem editor");
    emblem_scroll_delay_rest = _Dvar_RegisterInt("emblem_scroll_delay_rest", 50, 0, 1000, 0, "Repeat delay for emblem editor");
    UI_Project_RegisterDvars();
    UI_FriendsRegisterDvars();
    UI_RegisterDvars_PC();
}

void UI_AssetCache()
{
    sharedUiInfo.assets.whiteMaterial = Material_RegisterHandle("white", 3);
    sharedUiInfo.assets.scrollBar = Material_RegisterHandle("menu_mp_lobby_scrollbar_main", 3);
    sharedUiInfo.assets.scrollBarArrowDown = Material_RegisterHandle("ui_scrollbar_arrow_dwn_a", 3);
    sharedUiInfo.assets.scrollBarArrowUp = Material_RegisterHandle("ui_scrollbar_arrow_up_a", 3);
    sharedUiInfo.assets.scrollBarArrowLeft = Material_RegisterHandle("ui_scrollbar_arrow_left", 3);
    sharedUiInfo.assets.scrollBarArrowRight = Material_RegisterHandle("ui_scrollbar_arrow_right", 3);
    sharedUiInfo.assets.scrollBarThumb = Material_RegisterHandle("menu_mp_lobby_scrollbar_block", 3);
    sharedUiInfo.assets.sliderBar = Material_RegisterHandle("ui_slider2", 3);
    sharedUiInfo.assets.sliderThumb = Material_RegisterHandle("ui_arrow_right", 3);
    sharedUiInfo.assets.dvarlistArrowLeft = Material_RegisterHandle("ui_arrow_left", 3);
    sharedUiInfo.assets.dvarlistArrowRight = Material_RegisterHandle("ui_arrow_right", 3);
    sharedUiInfo.assets.line = Material_RegisterHandle("ui_line", 3);
    sharedUiInfo.assets.cursor = Material_RegisterHandle("ui_cursor", 0);
    sharedUiInfo.assets.bigFont = CL_RegisterFont("fonts/bigfont", 0);
    sharedUiInfo.assets.smallFont = CL_RegisterFont("fonts/smallfont", 0);
    sharedUiInfo.assets.consoleFont = CL_RegisterFont("fonts/consolefont", 0);
    sharedUiInfo.assets.boldFont = CL_RegisterFont("fonts/boldfont", 0);
    sharedUiInfo.assets.textFont = CL_RegisterFont("fonts/normalfont", 0);
    sharedUiInfo.assets.extraBigFont = CL_RegisterFont("fonts/extrabigfont", 0);
    UI_Project_AssetCache();
}

void UI_InitUIInfos()
{
    int infoIndex; // [esp+4h] [ebp-94h]
    char menuname[132]; // [esp+8h] [ebp-90h] BYREF
    uiInfo_s *uiInfo; // [esp+90h] [ebp-8h]
    MenuList *menuList; // [esp+94h] [ebp-4h]

    for ( infoIndex = 0; infoIndex < 1; ++infoIndex )
    {
        uiInfo = UI_UIContext_GetInfo(infoIndex);
        uiInfo->uiDC.contextIndex = 0;
        uiInfo->uiDC.contextIndex = infoIndex;
        uiInfo->allowScriptMenuResponse = 1;
        Menu_Setup(&uiInfo->uiDC);
        CL_GetScreenDimensions(&uiInfo->uiDC.screenWidth, &uiInfo->uiDC.screenHeight, &uiInfo->uiDC.screenAspect);
        if ( 480 * uiInfo->uiDC.screenWidth <= 640 * uiInfo->uiDC.screenHeight )
            uiInfo->uiDC.bias = 0.0f;
        else
            uiInfo->uiDC.bias = (float)((float)uiInfo->uiDC.screenWidth - (float)((float)uiInfo->uiDC.screenHeight * 1.3333334))
                                                * 0.5;
        if ( useFastFile->current.enabled )
        {
            Com_sprintf(menuname, 0x80u, "%spatch.txt", "ui_mp/");
            menuList = UI_LoadMenus(menuname, 3);
            UI_AddMenuList(0, &uiInfo->uiDC, menuList, 1);
            Com_sprintf(menuname, 0x80u, "%scode.txt", "ui_mp/");
            menuList = UI_LoadMenus(menuname, 3);
            UI_AddMenuList(0, &uiInfo->uiDC, menuList, 1);
        }
        if ( !G_ExitAfterToolComplete() && (!g_mapname[0] || !useFastFile->current.enabled) )
        {
            Com_sprintf(menuname, 0x80u, "%smenus.txt", "ui_mp/");
            menuList = UI_LoadMenus(menuname, 3);
            UI_AddMenuList(0, &uiInfo->uiDC, menuList, 1);
        }
        Menus_CloseAll(0, &uiInfo->uiDC);
    }
}

void __cdecl UI_Init(int localClientNum)
{
    g_ingameMenusLoaded[localClientNum] = 0;
}

char __cdecl UI_KeyEvent_AutoJoinButtonPressed(int localClientNum, UiContext *dc, menuDef_t *menu, int key, int down)
{
    int i; // [esp+0h] [ebp-10h]
    int twokeys[2]; // [esp+4h] [ebp-Ch] BYREF
    int numBinds; // [esp+Ch] [ebp-4h]

    numBinds = Key_GetCommandAssignment(localClientNum, "acceptInvitation", twokeys, 0);
    for ( i = 0; i < numBinds; ++i )
    {
        if ( key == twokeys[i] && down )
            return 1;
    }
    return 0;
}

int bypassKeyClear;
void __cdecl UI_KeyEvent(int localClientNum, int key, int down)
{
    connstate_t connstate; // [esp+0h] [ebp-Ch]
    uiInfo_s *uiInfo; // [esp+4h] [ebp-8h]
    menuDef_t *menu; // [esp+8h] [ebp-4h]
    int savedregs; // [esp+Ch] [ebp+0h] BYREF

    uiInfo = UI_GetInfo(localClientNum);
    if ( Menu_Count(&uiInfo->uiDC) )
    {
        menu = Menu_GetFocused(&uiInfo->uiDC);
        if ( !menu )
            goto LABEL_24;
        if ( UI_KeysBypassMenu(localClientNum) )
            bypassKeyClear = 1;
        if ( UI_KeyEvent_CancelButtonPressed(&uiInfo->uiDC, menu, key, down) )
            Menus_CloseAll(localClientNum, &uiInfo->uiDC);
        if ( UI_KeyEvent_AutoJoinButtonPressed(localClientNum, &uiInfo->uiDC, menu, key, down) )
            Live_AcceptLastInvite_f();
        if ( Key_IsCatcherActive(localClientNum, 336) )
            Menu_HandleKey(localClientNum, &uiInfo->uiDC, menu, key, down);
        if ( !Menu_GetFocused(&uiInfo->uiDC) )
        {
LABEL_24:
            if ( Key_IsCatcherActive(localClientNum, 16) && Com_LocalClient_IsBeingUsed(localClientNum) )
            {
                Key_RemoveCatcher(localClientNum, -17);
                if ( !bypassKeyClear )
                    Key_ClearStates(localClientNum);
                bypassKeyClear = 0;
                connstate = CL_GetLocalClientConnectionState(localClientNum);
                if ( connstate == CA_ACTIVE )
                {
                    Com_DPrintf(13, "PC ONLY: No menu has focus and == CA_ACTIVE, so forcing UIMENU_NONE");
                    UI_SetActiveMenu(localClientNum, UIMENU_NONE);
                }
                else if ( connstate >= CA_CONNECTING )
                {
                    Com_DPrintf(13, "PC ONLY: No menu has focus and %s, what should I do?", (const char *)connstate);
                }
                else
                {
                    Com_DPrintf(13, "PC ONLY: No menu has focus and < CA_CONNECTING, so forcing UIMENU_MAIN");
                    UI_SetActiveMenu(localClientNum, UIMENU_MAIN);
                }
                CL_PauseGame(0);
            }
        }
    }
}

bool __cdecl UI_KeyEvent_CancelButtonPressed(UiContext *dc, menuDef_t *menu, int key, int down)
{
    return (key == 2 || key == 27) && down && !Menus_AnyFullScreenVisible(dc) && !Menus_DoesEventExist(menu, "onESC");
}

void __cdecl UI_CloseAllMenus(int localClientNum)
{
    UI_CloseAll(localClientNum);
}

void __cdecl UI_UIContext_OpenMenu(int localClientNum, int contextIndex, const char *menuName)
{
    uiInfo_s *uiInfo; // [esp+0h] [ebp-4h]

    uiInfo = UI_UIContext_GetInfo(contextIndex);
    Menus_OpenByName(localClientNum, &uiInfo->uiDC, menuName);
}

void __cdecl UI_UIContext_CloseMenu(int localClientNum, int contextIndex, const char *menuName)
{
    uiInfo_s *uiInfo; // [esp+0h] [ebp-4h]

    uiInfo = UI_UIContext_GetInfo(contextIndex);
    Menus_CloseByName(localClientNum, &uiInfo->uiDC, menuName);
}

void __cdecl UI_UIContext_CloseMenuImmediate(int localClientNum, int contextIndex, const char *menuName)
{
    uiInfo_s *uiInfo; // [esp+0h] [ebp-4h]

    uiInfo = UI_UIContext_GetInfo(contextIndex);
    Menus_CloseImmediateByName(localClientNum, &uiInfo->uiDC, menuName);
}

void __cdecl UI_UIContext_OpenToastPopup(
                int contextIndex,
                const char *toastPopupIconName,
                const char *GLOBAL_EMPTY_STRING,
                const char *toastPopupDesc,
                int toastPopupDuration)
{
    int openSlideSpeed; // [esp+0h] [ebp-20h]
    int openFadingTime; // [esp+4h] [ebp-1Ch]
    uiInfo_s *uiInfo; // [esp+18h] [ebp-8h]
    menuDef_t *toastPopupMenu; // [esp+1Ch] [ebp-4h]

    uiInfo = UI_UIContext_GetInfo(contextIndex);
    I_strncpyz(uiInfo->toastPopupIconName, toastPopupIconName, 256);
    I_strncpyz(uiInfo->toastPopupTitle, GLOBAL_EMPTY_STRING, 256);
    I_strncpyz(uiInfo->toastPopupDesc, toastPopupDesc, 256);
    uiInfo->toastPopupDuration = toastPopupDuration;
    uiInfo->toastPopupOpened = 1;
    uiInfo->toastPopupTimeCounter = Sys_Milliseconds();
    toastPopupMenu = Menus_FindByName(&uiInfo->uiDC, "menu_toast_popup");
    if ( toastPopupMenu->openSlideSpeed <= 0 || toastPopupMenu->openSlideDirection >= 4u )
    {
        if ( toastPopupMenu->openFadingTime > 0 )
        {
            Window_AddDynamicFlags(contextIndex, &toastPopupMenu->window, 0x80000);
            toastPopupMenu->fadeTimeCounter = Sys_Milliseconds();
        }
    }
    else
    {
        Window_AddDynamicFlags(contextIndex, &toastPopupMenu->window, 0x20000);
        toastPopupMenu->slideTimeCounter = Sys_Milliseconds();
        if ( toastPopupMenu->openFadingTime > 0 )
        {
            Window_AddDynamicFlags(contextIndex, &toastPopupMenu->window, 0x80000);
            if ( toastPopupMenu->openSlideSpeed < toastPopupMenu->openFadingTime )
                openFadingTime = toastPopupMenu->openFadingTime;
            else
                openFadingTime = toastPopupMenu->openSlideSpeed;
            toastPopupMenu->openSlideSpeed = openFadingTime;
            if ( toastPopupMenu->openSlideSpeed < toastPopupMenu->openFadingTime )
                openSlideSpeed = toastPopupMenu->openFadingTime;
            else
                openSlideSpeed = toastPopupMenu->openSlideSpeed;
            toastPopupMenu->openFadingTime = openSlideSpeed;
            toastPopupMenu->fadeTimeCounter = Sys_Milliseconds();
        }
    }
    Window_RemoveDynamicFlags(contextIndex, &toastPopupMenu->window, 270336);
}

void __cdecl UI_OpenMenu(int localClientNum, const char *menuName)
{
    int UIContextIndex; // eax

    UIContextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
    UI_UIContext_OpenMenu(localClientNum, UIContextIndex, menuName);
}

void __cdecl UI_CloseMenu(int localClientNum, const char *menuName)
{
    int UIContextIndex; // eax

    UIContextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
    UI_UIContext_CloseMenu(localClientNum, UIContextIndex, menuName);
}

void __cdecl UI_CloseMenuImmediate(int localClientNum, const char *menuName)
{
    int UIContextIndex; // eax

    UIContextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
    UI_UIContext_CloseMenuImmediate(localClientNum, UIContextIndex, menuName);
}

void __cdecl UI_OpenToastPopup(
                int localClientNum,
                const char *toastPopupIconName,
                const char *GLOBAL_EMPTY_STRING,
                const char *toastPopupDesc,
                int toastPopupDuration)
{
    int UIContextIndex; // eax

    UIContextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
    UI_UIContext_OpenToastPopup(UIContextIndex, toastPopupIconName, GLOBAL_EMPTY_STRING, toastPopupDesc, toastPopupDuration);
}

void __cdecl UI_DrawMapLevelshot(int localClientNum)
{
    uiInfo_s *uiInfo; // [esp+24h] [ebp-10h]
    menuDef_t *menu; // [esp+28h] [ebp-Ch]
    menuDef_t *menua; // [esp+28h] [ebp-Ch]
    int contextIndex; // [esp+2Ch] [ebp-8h]
    int savedregs; // [esp+34h] [ebp+0h] BYREF

    contextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
    if ( contextIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp",
                    4415,
                    0,
                    "%s",
                    "contextIndex != INVALID_UI_CONTEXT") )
    {
        __debugbreak();
    }
    uiInfo = UI_GetInfo(localClientNum);
    menu = Menus_FindByName(&uiInfo->uiDC, "pregame_loaderror_mp");
    if ( !menu || !Menus_MenuIsInStack(&uiInfo->uiDC, menu) )
    {
        if ( g_showLoadingScreenMenu )
        {
            if ( useFastFile->current.enabled )
                menua = DB_FindXAssetHeader(ASSET_TYPE_MENU, (char*)"connect", 1, -1).menu;
            else
                menua = Menus_FindByName(&uiInfo->uiDC, "connect");
        }
        else
        {
            menua = 0;
        }
        if ( menua )
        {
            if ( !Menus_MenuIsInStack(&uiInfo->uiDC, menua) )
                Menus_Open(localClientNum, &uiInfo->uiDC, menua);
            uiInfo->uiDC.blurRadiusOut = 0.0f;
            Window_SetDynamicFlags(contextIndex, &menua->window, 16388);
            Menu_Paint(
                localClientNum,
                &uiInfo->uiDC,
                0,
                menua,
                (const ScreenPlacement *)0xFFFFFFFF);
        }
        else
        {
            UI_FillRect(&scrPlaceView[contextIndex], 0.0, 0.0, 640.0, 480.0, 0, 0, colorBlack);
        }
    }
}

void __cdecl UI_LoadIngameMenus(int contextIndex)
{
    const char *v1; // eax
    uiInfo_s *uiInfo; // [esp+0h] [ebp-8h]
    MenuList *menuList; // [esp+4h] [ebp-4h]

    if ( !g_ingameMenusLoaded[contextIndex] )
    {
        g_ingameMenusLoaded[contextIndex] = 1;
        v1 = va("%singame.txt", "ui_mp/");
        menuList = UI_LoadMenus(v1, 3);
        uiInfo = UI_UIContext_GetInfo(contextIndex);
        UI_AddMenuList(contextIndex, &uiInfo->uiDC, menuList, 1);
    }
}

unsigned __int8 __cdecl UI_ToUpper_CodePage_1250(unsigned __int8 character)
{
    if ( character >= 0x61u && character <= 0x7Au )
        return character - 32;
    if ( character >= 0xE0u && character != 247 && character != 255 )
        return character - 32;
    switch ( character )
    {
        case 0x9Au:
        case 0x9Cu:
        case 0x9Eu:
        case 0x9Fu:
        case 0xB3u:
        case 0xBAu:
        case 0xBFu:
            return character - 16;
        case 0xB9u:
            return character - 20;
        case 0xBEu:
            return character - 2;
    }
    return character;
}

unsigned __int8 __cdecl UI_ToUpper_CodePage_1251(unsigned __int8 character)
{
    if ( character >= 0x61u && character <= 0x7Au )
        return character - 32;
    if ( character >= 0xE0u )
        return character - 32;
    switch ( character )
    {
        case 0xBFu:
        case 0xBAu:
        case 0xB8u:
        case 0x90u:
        case 0x9Au:
        case 0x9Cu:
        case 0x9Du:
        case 0x9Eu:
        case 0x9Fu:
            return character - 16;
        case 0xBEu:
        case 0xB3u:
        case 0xA2u:
            return character - 1;
        case 0xBCu:
            return character - 25;
        case 0xB4u:
            return character - 15;
        case 0x83u:
            return character - 2;
    }
    return character;
}

unsigned __int8 __cdecl UI_ToUpper_CodePage_1252(unsigned __int8 character)
{
    if ( character >= 0x61u && character <= 0x7Au )
        return character - 32;
    if ( character >= 0xE0u && character != 255 && character != 240 && character != 247 )
        return character - 32;
    if ( character == 154 || character == 156 || character == 158 )
        return character - 32;
    if ( character == 255 )
        return -97;
    return character;
}

unsigned __int8 __cdecl UI_CharToUpper(unsigned __int8 character)
{
    unsigned __int8 result; // al

    if ( !loc_language
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_main.cpp", 4574, 0, "%s", "loc_language") )
    {
        __debugbreak();
    }
    switch ( loc_language->current.integer )
    {
        case 8:
            result = UI_ToUpper_CodePage_1251(character);
            break;
        case 9:
            result = UI_ToUpper_CodePage_1250(character);
            break;
        case 0xA:
        case 0xB:
            result = character;
            break;
        default:
            result = UI_ToUpper_CodePage_1252(character);
            break;
    }
    return result;
}

char *__cdecl UI_ToUpper(const char *sourceString)
{
    unsigned int i; // [esp+0h] [ebp-40Ch]
    char upperString[1028]; // [esp+4h] [ebp-408h] BYREF

    memset(upperString, 0, 1024);
    for ( i = 0; sourceString[i] && i < 0x400; ++i )
        upperString[i] = UI_CharToUpper(sourceString[i]);
    return va("%s", upperString);
}

