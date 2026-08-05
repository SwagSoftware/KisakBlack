#include "g_hudelem.h"
#include <cgame/cg_hudelem.h>
#include <game_mp/g_spawn_mp.h>
#include <game_mp/g_utils_mp.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/cscr_stringlist.h>
#include <clientscript/scr_const.h>
#include <server_mp/sv_init_mp.h>
#include <bgame/bg_misc.h>
#include <server/sv_game.h>
#include <cgame/cg_scr_main.h>
#include <bgame/bg_weapons_def.h>
#include <stringed/stringed_hooks.h>

const game_hudelem_field_t fields_0[28] =
{
  { "x", 0, 4, F_FLOAT, 0, 0, NULL, NULL },
  { "y", 4, 4, F_FLOAT, 0, 0, NULL, NULL },
  { "z", 8, 4, F_FLOAT, 0, 0, NULL, NULL },
  { "fontscale", 12, 4, F_FLOAT, -1, 0, &HudElem_SetFontScale, NULL },
  { "font", 101, 1, F_INT, -1, 0, &HudElem_SetFont, &HudElem_GetFont },
  { "alignx", 102, 1, F_INT, 3, 2, &HudElem_SetAlignX, &HudElem_GetAlignX },
  { "aligny", 102, 1, F_INT, 3, 0, &HudElem_SetAlignY, &HudElem_GetAlignY },
  {
    "horzalign",
    103,
    1,
    F_INT,
    15,
    4,
    &HudElem_SetHorzAlign,
    &HudElem_GetHorzAlign
  },
  {
    "vertalign",
    103,
    1,
    F_INT,
    15,
    0,
    &HudElem_SetVertAlign,
    &HudElem_GetVertAlign
  },
  { "color", 16, 4, F_INT, -1, 0, &HudElem_SetColor, &HudElem_GetColor },
  { "alpha", 16, 4, F_INT, -1, 0, &HudElem_SetAlpha, &HudElem_GetAlpha },
  { "label", 72, 2, F_INT, -1, 0, &HudElem_SetLocalizedString, NULL },
  { "sort", 56, 4, F_FLOAT, 0, 0, NULL, NULL },
  {
    "foreground",
    98,
    2,
    F_INT,
    -1,
    0,
    &HudElem_SetFlagForeground,
    &HudElem_GetFlagForeground
  },
  {
    "hidewhendead",
    98,
    2,
    F_INT,
    -1,
    0,
    &HudElem_SetFlagHideWhenDead,
    &HudElem_GetFlagHideWhenDead
  },
  {
    "hidewheninkillcam",
    98,
    2,
    F_INT,
    -1,
    0,
    &HudElem_SetFlagHideWhenInKillCam,
    &HudElem_GetFlagHideWhenInKillCam
  },
  {
    "hidewhenindemo",
    98,
    2,
    F_INT,
    -1,
    0,
    &HudElem_SetFlagHideWhenInDemo,
    &HudElem_GetFlagHideWhenInDemo
  },
  {
    "overrridewhenindemo",
    98,
    2,
    F_INT,
    -1,
    0,
    &HudElem_SetFlagOverrideWhenInDemo,
    &HudElem_GetFlagOverrideWhenInDemo
  },
  {
    "hidewhileremotecontrolling",
    98,
    2,
    F_INT,
    -1,
    0,
    &HudElem_SetFlagHideWhileRemoteControlling,
    &HudElem_GetFlagHideWhileRemoteControlling
  },
  {
    "hidewheninmenu",
    98,
    2,
    F_INT,
    -1,
    0,
    &HudElem_SetFlagHideWhenInMenu,
    &HudElem_GetFlagHideWhenInMenu
  },
  {
    "fadewhentargeted",
    98,
    2,
    F_INT,
    -1,
    0,
    &HudElem_SetFlagFadeWhenTargeted,
    &HudElem_GetFlagFadeWhenTargeted
  },
  {
    "fontstyle3d",
    98,
    2,
    F_INT,
    -1,
    0,
    &HudElem_SetFontStyle3d,
    &HudElem_GetFontStyle3d
  },
  {
    "font3duseglowcolor",
    98,
    2,
    F_INT,
    -1,
    0,
    &HudElem_SetFont3dUseGlowColor,
    &HudElem_GetFont3dUseGlowColor
  },
  {
    "glowcolor",
    60,
    4,
    F_INT,
    -1,
    0,
    &HudElem_SetGlowColor,
    &HudElem_GetGlowColor
  },
  {
    "glowalpha",
    60,
    4,
    F_INT,
    -1,
    0,
    &HudElem_SetGlowAlpha,
    &HudElem_GetGlowAlpha
  },
  { "archived", 120, 4, F_INT, -1, 0, &HudElem_SetBoolean, NULL },
  {
    "ui3dwindow",
    109,
    1,
    F_BYTE,
    -1,
    0,
    &HudElem_SetUI3DWindow,
    &HudElem_GetUI3DWindow
  },
  { NULL, 0, 0, F_INT, 0, 0, NULL, NULL }
};

// Looks congruent to retail blops mp latest
const BuiltinMethodDef methods_0[26] =
{
  { "settext", &HECmd_SetText, 0 },
  { "clearalltextafterhudelem", &HECmd_ClearAllTextAfterHudElem, 0 },
  { "setshader", &HECmd_SetMaterial, 0 },
  { "settargetent", &HECmd_SetTargetEnt, 0 },
  { "cleartargetent", &HECmd_ClearTargetEnt, 0 },
  { "settimer", &HECmd_SetTimer, 0 },
  { "settimerup", &HECmd_SetTimerUp, 0 },
  { "settenthstimer", &HECmd_SetTenthsTimer, 0 },
  { "settenthstimerup", &HECmd_SetTenthsTimerUp, 0 },
  { "setclock", &HECmd_SetClock, 0 },
  { "setclockup", &HECmd_SetClockUp, 0 },
  { "setvalue", &HECmd_SetValue, 0 },
  { "setwaypoint", &HECmd_SetWaypoint, 0 },
  { "fadeovertime", &HECmd_FadeOverTime, 0 },
  { "scaleovertime", &HECmd_ScaleOverTime, 0 },
  { "moveovertime", &HECmd_MoveOverTime, 0 },
  { "reset", &HECmd_Reset, 0 },
  { "destroy", &HECmd_Destroy, 0 },
  { "setpulsefx", &HECmd_SetPulseFX, 0 },
  { "setcod7decodefx", &HECmd_SetCOD7DecodeFX, 0 },
  { "setredactfx", &HECmd_SetRedactFX, 0 },
  { "gettextwidth", &HECmd_GetTextWidth, 0 },
  { "setplayernamestring", &HECmd_SetPlayerNameString, 0 },
  { "setmapnamestring", &HECmd_SetMapNameString, 0 },
  { "setgametypestring", &HECmd_SetGameTypeString, 0 },
  { "setwargamedata", &HECmd_SetWarGameData, 0 }
};



const char *g_he_textstyle[3] = { "normal", "shadowed", "shadowedmore" };
const char *g_he_font[7] = { "default", "bigfixed", "smallfixed", "objective", "big", "small", "extrabig" };
const char *g_he_alignx[3] =
{ "left", "center", "right" };

const char *g_he_aligny[3] =
{ "top", "middle", "bottom" };

const char *g_he_horzalign[11] =
{
  "subleft",
  "left",
  "center",
  "right",
  "fullscreen",
  "noscale",
  "alignto640",
  "center_safearea",
  "user_left",
  "user_center",
  "user_right"
};

const char *g_he_vertalign[11] =
{
  "subtop",
  "top",
  "middle",
  "bottom",
  "fullscreen",
  "noscale",
  "alignto480",
  "center_safearea",
  "user_top",
  "user_center",
  "user_bottom"
};

hudelem_s g_dummyHudCurrent_0;

game_hudelem_s *__cdecl HudElem_Alloc(int clientNum, int teamNum)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 0x400; ++i )
    {
        if ( !g_hudelems[i].elem.type )
        {
            HudElem_SetDefaults(&g_hudelems[i]);
            g_hudelems[i].clientNum = clientNum;
            g_hudelems[i].team = teamNum;
            return &g_hudelems[i];
        }
    }
    return 0;
}

void __cdecl HudElem_SetDefaults(game_hudelem_s *hud)
{
    if ( !hud && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp", 241, 0, "%s", "hud") )
        __debugbreak();
    if ( (unsigned int)(hud - g_hudelems) >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    242,
                    0,
                    "hud - g_hudelems doesn't index ARRAY_COUNT( g_hudelems )\n\t%i not in [0, %i)",
                    hud - g_hudelems,
                    1024) )
    {
        __debugbreak();
    }
    hud->elem.type = 1;
    hud->elem.x = 0.0f;
    hud->elem.y = 0.0f;
    hud->elem.z = 0.0f;
    hud->elem.targetEntNum = 1023;
    hud->elem.font = 0;
    hud->elem.alignOrg = 0;
    hud->elem.alignScreen = 0;
    hud->elem.color.rgba = -1;
    hud->elem.glowColor.rgba = 0;
    hud->elem.fromColor.rgba = 0;
    hud->elem.fadeStartTime = 0;
    hud->elem.fadeTime = 0;
    hud->elem.label = 0;
    hud->elem.sort = 0.0f;
    hud->elem.flags = 0;
    hud->elem.fxBirthTime = 0;
    hud->elem.fxLetterTime = 0;
    hud->elem.fxDecayStartTime = 0;
    hud->elem.fxDecayDuration = 0;
    hud->elem.soundID = 0;
    hud->elem.moveStartTime = 0;
    hud->elem.moveTime = 0;
    hud->elem.fontScale = 1.0f;
    hud->archived = 1;
    hud->elem.ui3dWindow = -1;
    HudElem_ClearTypeSettings(hud);
}

void __cdecl HudElem_ClearTypeSettings(game_hudelem_s *hud)
{
    hud->elem.width = 0;
    hud->elem.height = 0;
    hud->elem.materialIndex = 0;
    hud->elem.fromX = 0.0f;
    hud->elem.fromY = 0.0f;
    hud->elem.fromAlignOrg = 0;
    hud->elem.fromAlignScreen = 0;
    hud->elem.fromWidth = 0;
    hud->elem.fromHeight = 0;
    hud->elem.scaleStartTime = 0;
    hud->elem.scaleTime = 0;
    hud->elem.time = 0;
    hud->elem.duration = 0;
    hud->elem.value = 0.0f;
    hud->elem.text = 0;
}

game_hudelem_s *__cdecl HudElem_DebugAlloc(int clientNum)
{
    game_hudelem_s *elem; // [esp+0h] [ebp-4h]

    elem = HudElem_Alloc(clientNum, 0);
    if ( elem )
        elem->elem.flags |= 0x2000u;
    return elem;
}

void __cdecl HudElem_Free(game_hudelem_s *hud)
{
    if ( !hud && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp", 348, 0, "%s", "hud") )
        __debugbreak();
    if ( (unsigned int)(hud - g_hudelems) >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    349,
                    0,
                    "hud - g_hudelems doesn't index ARRAY_COUNT( g_hudelems )\n\t%i not in [0, %i)",
                    hud - g_hudelems,
                    1024) )
    {
        __debugbreak();
    }
    if ( (!hud->elem.type || hud->elem.type >= 0x10u)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    350,
                    0,
                    "%s\n\t(hud->elem.type) = %i",
                    "(hud->elem.type > HE_TYPE_FREE && hud->elem.type < HE_TYPE_COUNT)",
                    hud->elem.type) )
    {
        __debugbreak();
    }
    Scr_FreeHudElem(hud);
    hud->elem.type = 0;
}

void __cdecl HudElem_ClientDisconnect(gentity_s *ent)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 0x400; ++i )
    {
        if ( g_hudelems[i].elem.type )
        {
            if ( g_hudelems[i].clientNum == ent->s.number )
                HudElem_Free(&g_hudelems[i]);
        }
    }
}

void __cdecl HudElem_DestroyAll()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 0x400; ++i )
    {
        if ( g_hudelems[i].elem.type )
            HudElem_Free(&g_hudelems[i]);
    }
    memset(g_hudelems, 0, sizeof(g_hudelems));
}

void __cdecl HudElem_SetLocalizedString(game_hudelem_s *hud, int offset)
{
    int size; // [esp+0h] [ebp-18h]
    int v3; // [esp+4h] [ebp-14h]
    char *v4; // [esp+Ch] [ebp-Ch]
    char *string; // [esp+10h] [ebp-8h]
    const game_hudelem_field_t *f; // [esp+14h] [ebp-4h]

    f = &fields_0[offset];
    string = Scr_GetIString(0, SCRIPTINSTANCE_SERVER);
    v3 = G_LocalizedStringIndex(string);
    v4 = (char *)hud + f->ofs;
    size = f->size;
    switch ( size )
    {
        case 1:
            *v4 = v3;
            break;
        case 2:
            *(_WORD *)v4 = v3;
            break;
        case 4:
            *(unsigned int *)v4 = v3;
            break;
    }
}

void __cdecl HudElem_SetFlagForeground(game_hudelem_s *hud, int offset)
{
    int v2; // [esp+0h] [ebp-24h]
    int size; // [esp+4h] [ebp-20h]
    unsigned int v4; // [esp+8h] [ebp-1Ch]
    int v5; // [esp+10h] [ebp-14h]
    const game_hudelem_field_t *f; // [esp+18h] [ebp-Ch]
    int *flags; // [esp+1Ch] [ebp-8h]

    f = &fields_0[offset];
    flags = (int *)((char *)hud + f->ofs);
    if ( Scr_GetInt(0, SCRIPTINSTANCE_SERVER) )
    {
        v5 = GetField((char *)flags, f->size) | 1;
        size = f->size;
        switch ( size )
        {
            case 1:
                *(_BYTE *)flags = v5;
                break;
            case 2:
                *(_WORD *)flags = v5;
                break;
            case 4:
                *flags = v5;
                break;
        }
    }
    else
    {
        v4 = GetField((char *)flags, f->size) & 0xFFFFFFFE;
        v2 = f->size;
        switch ( v2 )
        {
            case 1:
                *(_BYTE *)flags = v4;
                break;
            case 2:
                *(_WORD *)flags = v4;
                break;
            case 4:
                *flags = v4;
                break;
        }
    }
}

int __cdecl GetField(char *i, int size)
{
    switch ( size )
    {
        case 1:
            return *i;
        case 2:
            return *(__int16 *)i;
        case 4:
            return *(unsigned int *)i;
    }
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp", 187, 0, "unknown field size") )
        __debugbreak();
    return 0;
}

void __cdecl HudElem_GetFlagForeground(game_hudelem_s *hud, int offset)
{
    if ( fields_0[offset].ofs != 98
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    495,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.flags )") )
    {
        __debugbreak();
    }
    if ( (hud->elem.flags & 1) != 0 )
        Scr_AddBool(1u, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddBool(0, SCRIPTINSTANCE_SERVER);
}

void __cdecl HudElem_SetFontStyle3d(game_hudelem_s *hud, int offset)
{
    const char *v2; // eax
    int v3; // [esp+0h] [ebp-54h]
    int size; // [esp+4h] [ebp-50h]
    int v5; // [esp+8h] [ebp-4Ch]
    int v6; // [esp+Ch] [ebp-48h]
    int v7; // [esp+10h] [ebp-44h]
    int v8; // [esp+14h] [ebp-40h]
    unsigned int v9; // [esp+18h] [ebp-3Ch]
    int v10; // [esp+20h] [ebp-34h]
    unsigned int v11; // [esp+28h] [ebp-2Ch]
    int v12; // [esp+30h] [ebp-24h]
    unsigned int v13; // [esp+38h] [ebp-1Ch]
    unsigned int v14; // [esp+40h] [ebp-14h]
    char *pszStyle; // [esp+48h] [ebp-Ch]
    const game_hudelem_field_t *f; // [esp+4Ch] [ebp-8h]
    int *flags; // [esp+50h] [ebp-4h]

    f = &fields_0[offset];
    flags = (int *)((char *)hud + f->ofs);
    pszStyle = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    if ( I_stricmp(pszStyle, "normal") )
    {
        if ( I_stricmp(pszStyle, "shadowed") )
        {
            if ( I_stricmp(pszStyle, "shadowedmore") )
            {
                v2 = va("unknown text style '%s', should be normal, shadowed, or shadowedmore", pszStyle);
                Scr_ParamError(0, v2, SCRIPTINSTANCE_SERVER);
            }
            else
            {
                v10 = GetField((char *)flags, f->size) | 0x10;
                size = f->size;
                switch ( size )
                {
                    case 1:
                        *(_BYTE *)flags = v10;
                        break;
                    case 2:
                        *(_WORD *)flags = v10;
                        break;
                    case 4:
                        *flags = v10;
                        break;
                }
                v9 = GetField((char *)flags, f->size) & 0xFFFFFFF7;
                v3 = f->size;
                switch ( v3 )
                {
                    case 1:
                        *(_BYTE *)flags = v9;
                        break;
                    case 2:
                        *(_WORD *)flags = v9;
                        break;
                    case 4:
                        *flags = v9;
                        break;
                }
            }
        }
        else
        {
            v12 = GetField((char *)flags, f->size) | 8;
            v6 = f->size;
            switch ( v6 )
            {
                case 1:
                    *(_BYTE *)flags = v12;
                    break;
                case 2:
                    *(_WORD *)flags = v12;
                    break;
                case 4:
                    *flags = v12;
                    break;
            }
            v11 = GetField((char *)flags, f->size) & 0xFFFFFFEF;
            v5 = f->size;
            switch ( v5 )
            {
                case 1:
                    *(_BYTE *)flags = v11;
                    break;
                case 2:
                    *(_WORD *)flags = v11;
                    break;
                case 4:
                    *flags = v11;
                    break;
            }
        }
    }
    else
    {
        v14 = GetField((char *)flags, f->size) & 0xFFFFFFF7;
        v8 = f->size;
        switch ( v8 )
        {
            case 1:
                *(_BYTE *)flags = v14;
                break;
            case 2:
                *(_WORD *)flags = v14;
                break;
            case 4:
                *flags = v14;
                break;
        }
        v13 = GetField((char *)flags, f->size) & 0xFFFFFFEF;
        v7 = f->size;
        switch ( v7 )
        {
            case 1:
                *(_BYTE *)flags = v13;
                break;
            case 2:
                *(_WORD *)flags = v13;
                break;
            case 4:
                *flags = v13;
                break;
        }
    }
}

void __cdecl HudElem_GetFontStyle3d(game_hudelem_s *hud, int offset)
{
    if ( fields_0[offset].ofs != 98
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    546,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.flags )") )
    {
        __debugbreak();
    }
    if ( (hud->elem.flags & 8) != 0 )
    {
        Scr_AddString((char *)g_he_textstyle[1], SCRIPTINSTANCE_SERVER);
    }
    else if ( (hud->elem.flags & 0x10) != 0 )
    {
        Scr_AddString((char *)g_he_textstyle[2], SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Scr_AddString((char *)g_he_textstyle[0], SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl HudElem_SetFont3dUseGlowColor(game_hudelem_s *hud, int offset)
{
    int v2; // [esp+0h] [ebp-24h]
    int size; // [esp+4h] [ebp-20h]
    unsigned int v4; // [esp+8h] [ebp-1Ch]
    int v5; // [esp+10h] [ebp-14h]
    const game_hudelem_field_t *f; // [esp+18h] [ebp-Ch]
    int *flags; // [esp+1Ch] [ebp-8h]

    f = &fields_0[offset];
    flags = (int *)((char *)hud + f->ofs);
    if ( Scr_GetInt(0, SCRIPTINSTANCE_SERVER) )
    {
        v5 = GetField((char *)flags, f->size) | 0x20;
        size = f->size;
        switch ( size )
        {
            case 1:
                *(_BYTE *)flags = v5;
                break;
            case 2:
                *(_WORD *)flags = v5;
                break;
            case 4:
                *flags = v5;
                break;
        }
    }
    else
    {
        v4 = GetField((char *)flags, f->size) & 0xFFFFFFDF;
        v2 = f->size;
        switch ( v2 )
        {
            case 1:
                *(_BYTE *)flags = v4;
                break;
            case 2:
                *(_WORD *)flags = v4;
                break;
            case 4:
                *flags = v4;
                break;
        }
    }
}

void __cdecl HudElem_GetFont3dUseGlowColor(game_hudelem_s *hud, int offset)
{
    if ( fields_0[offset].ofs != 98
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    582,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.flags )") )
    {
        __debugbreak();
    }
    if ( (hud->elem.flags & 0x20) != 0 )
        Scr_AddBool(1u, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddBool(0, SCRIPTINSTANCE_SERVER);
}

void __cdecl HudElem_SetFlagHideWhenDead(game_hudelem_s *hud, int offset)
{
    int v2; // [esp+0h] [ebp-24h]
    int size; // [esp+4h] [ebp-20h]
    unsigned int v4; // [esp+8h] [ebp-1Ch]
    int v5; // [esp+10h] [ebp-14h]
    const game_hudelem_field_t *f; // [esp+18h] [ebp-Ch]
    int *flags; // [esp+1Ch] [ebp-8h]

    f = &fields_0[offset];
    flags = (int *)((char *)hud + f->ofs);
    if ( Scr_GetInt(0, SCRIPTINSTANCE_SERVER) )
    {
        v5 = GetField((char *)flags, f->size) | 2;
        size = f->size;
        switch ( size )
        {
            case 1:
                *(_BYTE *)flags = v5;
                break;
            case 2:
                *(_WORD *)flags = v5;
                break;
            case 4:
                *flags = v5;
                break;
        }
    }
    else
    {
        v4 = GetField((char *)flags, f->size) & 0xFFFFFFFD;
        v2 = f->size;
        switch ( v2 )
        {
            case 1:
                *(_BYTE *)flags = v4;
                break;
            case 2:
                *(_WORD *)flags = v4;
                break;
            case 4:
                *flags = v4;
                break;
        }
    }
}

void __cdecl HudElem_GetFlagHideWhenDead(game_hudelem_s *hud, int offset)
{
    if ( fields_0[offset].ofs != 98
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    610,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.flags )") )
    {
        __debugbreak();
    }
    if ( (hud->elem.flags & 2) != 0 )
        Scr_AddBool(1u, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddBool(0, SCRIPTINSTANCE_SERVER);
}

void __cdecl HudElem_SetFlagHideWhenInKillCam(game_hudelem_s *hud, int offset)
{
    int v2; // [esp+0h] [ebp-24h]
    int size; // [esp+4h] [ebp-20h]
    unsigned int v4; // [esp+8h] [ebp-1Ch]
    int v5; // [esp+10h] [ebp-14h]
    const game_hudelem_field_t *f; // [esp+18h] [ebp-Ch]
    int *flags; // [esp+1Ch] [ebp-8h]

    f = &fields_0[offset];
    flags = (int *)((char *)hud + f->ofs);
    if ( Scr_GetInt(0, SCRIPTINSTANCE_SERVER) )
    {
        v5 = GetField((char *)flags, f->size) | 0x40;
        size = f->size;
        switch ( size )
        {
            case 1:
                *(_BYTE *)flags = v5;
                break;
            case 2:
                *(_WORD *)flags = v5;
                break;
            case 4:
                *flags = v5;
                break;
        }
    }
    else
    {
        v4 = GetField((char *)flags, f->size) & 0xFFFFFFBF;
        v2 = f->size;
        switch ( v2 )
        {
            case 1:
                *(_BYTE *)flags = v4;
                break;
            case 2:
                *(_WORD *)flags = v4;
                break;
            case 4:
                *flags = v4;
                break;
        }
    }
}

void __cdecl HudElem_GetFlagHideWhenInKillCam(game_hudelem_s *hud, int offset)
{
    if ( fields_0[offset].ofs != 98
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    637,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.flags )") )
    {
        __debugbreak();
    }
    if ( (hud->elem.flags & 0x40) != 0 )
        Scr_AddBool(1u, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddBool(0, SCRIPTINSTANCE_SERVER);
}

void __cdecl HudElem_SetFlagHideWhenInDemo(game_hudelem_s *hud, int offset)
{
    int v2; // [esp+0h] [ebp-24h]
    int size; // [esp+4h] [ebp-20h]
    unsigned int v4; // [esp+8h] [ebp-1Ch]
    int v5; // [esp+10h] [ebp-14h]
    const game_hudelem_field_t *f; // [esp+18h] [ebp-Ch]
    int *flags; // [esp+1Ch] [ebp-8h]

    f = &fields_0[offset];
    flags = (int *)((char *)hud + f->ofs);
    if ( Scr_GetInt(0, SCRIPTINSTANCE_SERVER) )
    {
        v5 = GetField((char *)flags, f->size) | 0x200;
        size = f->size;
        switch ( size )
        {
            case 1:
                *(_BYTE *)flags = v5;
                break;
            case 2:
                *(_WORD *)flags = v5;
                break;
            case 4:
                *flags = v5;
                break;
        }
    }
    else
    {
        v4 = GetField((char *)flags, f->size) & 0xFFFFFDFF;
        v2 = f->size;
        switch ( v2 )
        {
            case 1:
                *(_BYTE *)flags = v4;
                break;
            case 2:
                *(_WORD *)flags = v4;
                break;
            case 4:
                *flags = v4;
                break;
        }
    }
}

void __cdecl HudElem_GetFlagHideWhenInDemo(game_hudelem_s *hud, int offset)
{
    if ( fields_0[offset].ofs != 98
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    664,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.flags )") )
    {
        __debugbreak();
    }
    if ( (hud->elem.flags & 0x200) != 0 )
        Scr_AddBool(1u, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddBool(0, SCRIPTINSTANCE_SERVER);
}

void __cdecl HudElem_SetFlagOverrideWhenInDemo(game_hudelem_s *hud, int offset)
{
    int v2; // [esp+0h] [ebp-24h]
    int size; // [esp+4h] [ebp-20h]
    unsigned int v4; // [esp+8h] [ebp-1Ch]
    int v5; // [esp+10h] [ebp-14h]
    const game_hudelem_field_t *f; // [esp+18h] [ebp-Ch]
    int *flags; // [esp+1Ch] [ebp-8h]

    f = &fields_0[offset];
    flags = (int *)((char *)hud + f->ofs);
    if ( Scr_GetInt(0, SCRIPTINSTANCE_SERVER) )
    {
        v5 = GetField((char *)flags, f->size) | 0x400;
        size = f->size;
        switch ( size )
        {
            case 1:
                *(_BYTE *)flags = v5;
                break;
            case 2:
                *(_WORD *)flags = v5;
                break;
            case 4:
                *flags = v5;
                break;
        }
    }
    else
    {
        v4 = GetField((char *)flags, f->size) & 0xFFFFFBFF;
        v2 = f->size;
        switch ( v2 )
        {
            case 1:
                *(_BYTE *)flags = v4;
                break;
            case 2:
                *(_WORD *)flags = v4;
                break;
            case 4:
                *flags = v4;
                break;
        }
    }
}

void __cdecl HudElem_GetFlagOverrideWhenInDemo(game_hudelem_s *hud, int offset)
{
    if ( fields_0[offset].ofs != 98
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    691,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.flags )") )
    {
        __debugbreak();
    }
    if ( (hud->elem.flags & 0x400) != 0 )
        Scr_AddBool(1u, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddBool(0, SCRIPTINSTANCE_SERVER);
}

void __cdecl HudElem_SetFlagHideWhileRemoteControlling(game_hudelem_s *hud, int offset)
{
    int v2; // [esp+0h] [ebp-24h]
    int size; // [esp+4h] [ebp-20h]
    unsigned int v4; // [esp+8h] [ebp-1Ch]
    int v5; // [esp+10h] [ebp-14h]
    const game_hudelem_field_t *f; // [esp+18h] [ebp-Ch]
    int *flags; // [esp+1Ch] [ebp-8h]

    f = &fields_0[offset];
    flags = (int *)((char *)hud + f->ofs);
    if ( Scr_GetInt(0, SCRIPTINSTANCE_SERVER) )
    {
        v5 = GetField((char *)flags, f->size) | 0x800;
        size = f->size;
        switch ( size )
        {
            case 1:
                *(_BYTE *)flags = v5;
                break;
            case 2:
                *(_WORD *)flags = v5;
                break;
            case 4:
                *flags = v5;
                break;
        }
    }
    else
    {
        v4 = GetField((char *)flags, f->size) & 0xFFFFF7FF;
        v2 = f->size;
        switch ( v2 )
        {
            case 1:
                *(_BYTE *)flags = v4;
                break;
            case 2:
                *(_WORD *)flags = v4;
                break;
            case 4:
                *flags = v4;
                break;
        }
    }
}

void __cdecl HudElem_GetFlagHideWhileRemoteControlling(game_hudelem_s *hud, int offset)
{
    if ( fields_0[offset].ofs != 98
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    718,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.flags )") )
    {
        __debugbreak();
    }
    if ( (hud->elem.flags & 0x800) != 0 )
        Scr_AddBool(1u, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddBool(0, SCRIPTINSTANCE_SERVER);
}

void __cdecl HudElem_SetFlagHideWhenInMenu(game_hudelem_s *hud, int offset)
{
    int v2; // [esp+0h] [ebp-24h]
    int size; // [esp+4h] [ebp-20h]
    unsigned int v4; // [esp+8h] [ebp-1Ch]
    int v5; // [esp+10h] [ebp-14h]
    const game_hudelem_field_t *f; // [esp+18h] [ebp-Ch]
    int *flags; // [esp+1Ch] [ebp-8h]

    f = &fields_0[offset];
    flags = (int *)((char *)hud + f->ofs);
    if ( Scr_GetInt(0, SCRIPTINSTANCE_SERVER) )
    {
        v5 = GetField((char *)flags, f->size) | 4;
        size = f->size;
        switch ( size )
        {
            case 1:
                *(_BYTE *)flags = v5;
                break;
            case 2:
                *(_WORD *)flags = v5;
                break;
            case 4:
                *flags = v5;
                break;
        }
    }
    else
    {
        v4 = GetField((char *)flags, f->size) & 0xFFFFFFFB;
        v2 = f->size;
        switch ( v2 )
        {
            case 1:
                *(_BYTE *)flags = v4;
                break;
            case 2:
                *(_WORD *)flags = v4;
                break;
            case 4:
                *flags = v4;
                break;
        }
    }
}

void __cdecl HudElem_GetFlagHideWhenInMenu(game_hudelem_s *hud, int offset)
{
    if ( fields_0[offset].ofs != 98
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    745,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.flags )") )
    {
        __debugbreak();
    }
    if ( (hud->elem.flags & 4) != 0 )
        Scr_AddBool(1u, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddBool(0, SCRIPTINSTANCE_SERVER);
}

void __cdecl HudElem_SetFlagFadeWhenTargeted(game_hudelem_s *hud, int offset)
{
    int v2; // [esp+0h] [ebp-24h]
    int size; // [esp+4h] [ebp-20h]
    unsigned int v4; // [esp+8h] [ebp-1Ch]
    int v5; // [esp+10h] [ebp-14h]
    const game_hudelem_field_t *f; // [esp+18h] [ebp-Ch]
    int *flags; // [esp+1Ch] [ebp-8h]

    f = &fields_0[offset];
    flags = (int *)((char *)hud + f->ofs);
    if ( Scr_GetInt(0, SCRIPTINSTANCE_SERVER) )
    {
        v5 = GetField((char *)flags, f->size) | 0x100;
        size = f->size;
        switch ( size )
        {
            case 1:
                *(_BYTE *)flags = v5;
                break;
            case 2:
                *(_WORD *)flags = v5;
                break;
            case 4:
                *flags = v5;
                break;
        }
    }
    else
    {
        v4 = GetField((char *)flags, f->size) & 0xFFFFFEFF;
        v2 = f->size;
        switch ( v2 )
        {
            case 1:
                *(_BYTE *)flags = v4;
                break;
            case 2:
                *(_WORD *)flags = v4;
                break;
            case 4:
                *flags = v4;
                break;
        }
    }
}

void __cdecl HudElem_GetFlagFadeWhenTargeted(game_hudelem_s *hud, int offset)
{
    if ( fields_0[offset].ofs != 98
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    771,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.flags )") )
    {
        __debugbreak();
    }
    if ( (hud->elem.flags & 0x100) != 0 )
        Scr_AddBool(1u, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddBool(0, SCRIPTINSTANCE_SERVER);
}

void __cdecl HudElem_SetBoolean(game_hudelem_s *hud, int offset)
{
    const game_hudelem_field_t *v2; // eax
    int size; // [esp+0h] [ebp-14h]
    char *v4; // [esp+8h] [ebp-Ch]
    int value; // [esp+10h] [ebp-4h]

    value = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    v2 = &fields_0[offset];
    v4 = (char *)hud + v2->ofs;
    size = v2->size;
    switch ( size )
    {
        case 1:
            *v4 = value;
            break;
        case 2:
            *(_WORD *)v4 = value;
            break;
        case 4:
            *(unsigned int *)v4 = value;
            break;
    }
}

void __cdecl HudElem_SetColor(game_hudelem_s *hud, int offset)
{
    float v2; // [esp+0h] [ebp-60h]
    float v3; // [esp+4h] [ebp-5Ch]
    float v4; // [esp+8h] [ebp-58h]
    float v5; // [esp+20h] [ebp-40h]
    float v6; // [esp+38h] [ebp-28h]
    float v7; // [esp+50h] [ebp-10h]
    float color[3]; // [esp+54h] [ebp-Ch] BYREF

    if ( fields_0[offset].ofs != 16
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    798,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.color )") )
    {
        __debugbreak();
    }
    Scr_GetVector(0, color, SCRIPTINSTANCE_SERVER);
    if ( (float)(color[0] - 1.0) < 0.0 )
        v7 = color[0];
    else
        v7 = 1.0f;
    if ( (float)(0.0 - v7) < 0.0 )
        v4 = v7;
    else
        v4 = 0.0f;
    hud->elem.color.r = (int)((float)(255.0 * v4) + 9.313225746154785e-10);
    if ( (float)(color[1] - 1.0) < 0.0 )
        v6 = color[1];
    else
        v6 = 1.0f;
    if ( (float)(0.0 - v6) < 0.0 )
        v3 = v6;
    else
        v3 = 0.0f;
    hud->elem.color.g = (int)((float)(255.0 * v3) + 9.313225746154785e-10);
    if ( (float)(color[2] - 1.0) < 0.0 )
        v5 = color[2];
    else
        v5 = 1.0f;
    if ( (float)(0.0 - v5) < 0.0 )
        v2 = v5;
    else
        v2 = 0.0f;
    hud->elem.color.b = (int)((float)(255.0 * v2) + 9.313225746154785e-10);
}

void __cdecl HudElem_GetColor(game_hudelem_s *hud, int offset)
{
    float color[3]; // [esp+0h] [ebp-Ch] BYREF

    if ( fields_0[offset].ofs != 16
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    811,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.color )") )
    {
        __debugbreak();
    }
    color[0] = (float)hud->elem.color.r * 0.0039215689;
    color[1] = (float)hud->elem.color.g * 0.0039215689;
    color[2] = (float)hud->elem.color.b * 0.0039215689;
    Scr_AddVector(color, SCRIPTINSTANCE_SERVER);
}

void __cdecl HudElem_SetAlpha(game_hudelem_s *hud, int offset)
{
    float v2; // [esp+0h] [ebp-20h]
    float v3; // [esp+18h] [ebp-8h]
    float alpha; // [esp+1Ch] [ebp-4h]

    if ( fields_0[offset].ofs != 16
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    823,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.color )") )
    {
        __debugbreak();
    }
    alpha = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    if ( (float)(alpha - 1.0) < 0.0 )
        v3 = alpha;
    else
        v3 = 1.0f;
    if ( (float)(0.0 - v3) < 0.0 )
        v2 = v3;
    else
        v2 = 0.0f;
    hud->elem.color.a = (int)((float)(255.0 * v2) + 9.313225746154785e-10);
}

void __cdecl HudElem_GetAlpha(game_hudelem_s *hud, int offset)
{
    if ( fields_0[offset].ofs != 16
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    832,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.color )") )
    {
        __debugbreak();
    }
    Scr_AddFloat((float)hud->elem.color.a * 0.0039215689, SCRIPTINSTANCE_SERVER);
}

void __cdecl HudElem_SetGlowColor(game_hudelem_s *hud, int offset)
{
    float v2; // [esp+0h] [ebp-60h]
    float v3; // [esp+4h] [ebp-5Ch]
    float v4; // [esp+8h] [ebp-58h]
    float v5; // [esp+20h] [ebp-40h]
    float v6; // [esp+38h] [ebp-28h]
    float v7; // [esp+50h] [ebp-10h]
    float glowColor[3]; // [esp+54h] [ebp-Ch] BYREF

    if ( fields_0[offset].ofs != 60
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    842,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.glowColor )") )
    {
        __debugbreak();
    }
    Scr_GetVector(0, glowColor, SCRIPTINSTANCE_SERVER);
    if ( (float)(glowColor[0] - 1.0) < 0.0 )
        v7 = glowColor[0];
    else
        v7 = 1.0f;
    if ( (float)(0.0 - v7) < 0.0 )
        v4 = v7;
    else
        v4 = 0.0f;
    hud->elem.glowColor.r = (int)((float)(255.0 * v4) + 9.313225746154785e-10);
    if ( (float)(glowColor[1] - 1.0) < 0.0 )
        v6 = glowColor[1];
    else
        v6 = 1.0f;
    if ( (float)(0.0 - v6) < 0.0 )
        v3 = v6;
    else
        v3 = 0.0f;
    hud->elem.glowColor.g = (int)((float)(255.0 * v3) + 9.313225746154785e-10);
    if ( (float)(glowColor[2] - 1.0) < 0.0 )
        v5 = glowColor[2];
    else
        v5 = 1.0f;
    if ( (float)(0.0 - v5) < 0.0 )
        v2 = v5;
    else
        v2 = 0.0f;
    hud->elem.glowColor.b = (int)((float)(255.0 * v2) + 9.313225746154785e-10);
}

void __cdecl HudElem_GetGlowColor(game_hudelem_s *hud, int offset)
{
    float glowColor[3]; // [esp+0h] [ebp-Ch] BYREF

    if ( fields_0[offset].ofs != 60
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    855,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.glowColor )") )
    {
        __debugbreak();
    }
    glowColor[0] = (float)hud->elem.glowColor.r * 0.0039215689;
    glowColor[1] = (float)hud->elem.glowColor.g * 0.0039215689;
    glowColor[2] = (float)hud->elem.glowColor.b * 0.0039215689;
    Scr_AddVector(glowColor, SCRIPTINSTANCE_SERVER);
}

void __cdecl HudElem_SetGlowAlpha(game_hudelem_s *hud, int offset)
{
    float v2; // [esp+0h] [ebp-20h]
    float v3; // [esp+18h] [ebp-8h]
    float glowAlpha; // [esp+1Ch] [ebp-4h]

    if ( fields_0[offset].ofs != 60
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    868,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.glowColor )") )
    {
        __debugbreak();
    }
    glowAlpha = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    if ( (float)(glowAlpha - 1.0) < 0.0 )
        v3 = glowAlpha;
    else
        v3 = 1.0f;
    if ( (float)(0.0 - v3) < 0.0 )
        v2 = v3;
    else
        v2 = 0.0f;
    hud->elem.glowColor.a = (int)((float)(255.0 * v2) + 9.313225746154785e-10);
}

void __cdecl HudElem_GetGlowAlpha(game_hudelem_s *hud, int offset)
{
    if ( fields_0[offset].ofs != 60
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    877,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.glowColor )") )
    {
        __debugbreak();
    }
    Scr_AddFloat((float)hud->elem.glowColor.a * 0.0039215689, SCRIPTINSTANCE_SERVER);
}

void __cdecl HudElem_SetFontScale(game_hudelem_s *hud, int offset)
{
    const char *v2; // eax
    float scale; // [esp+Ch] [ebp-4h]

    if ( fields_0[offset].ofs != 12
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    887,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.fontScale )") )
    {
        __debugbreak();
    }
    scale = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    if ( scale <= 0.0 )
    {
        v2 = va("font scale was %g; should be > 0", scale);
        Scr_Error(v2, 0);
    }
    hud->elem.fontScale = scale;
}

void __cdecl HudElem_SetFont(game_hudelem_s *hud, int offset)
{
    HudElem_SetEnumString(hud, &fields_0[offset], g_he_font, 7);
}

void __cdecl HudElem_SetEnumString(
                game_hudelem_s *hud,
                const game_hudelem_field_t *f,
                const char **names,
                int nameCount)
{
    const char *v4; // eax
    int size; // [esp+0h] [ebp-820h]
    char *v6; // [esp+8h] [ebp-818h]
    char *selectedName; // [esp+Ch] [ebp-814h]
    char errormsg[2052]; // [esp+10h] [ebp-810h] BYREF
    int nameIndex; // [esp+818h] [ebp-8h]
    int value; // [esp+81Ch] [ebp-4h]

    if ( !hud && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp", 401, 0, "%s", "hud") )
        __debugbreak();
    if ( !f && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp", 402, 0, "%s", "f") )
        __debugbreak();
    if ( !names && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp", 403, 0, "%s", "names") )
        __debugbreak();
    if ( nameCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    404,
                    0,
                    "%s\n\t(nameCount) = %i",
                    "(nameCount > 0)",
                    nameCount) )
    {
        __debugbreak();
    }
    value = GetField((char *)hud + f->ofs, f->size);
    selectedName = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    for ( nameIndex = 0; nameIndex < nameCount; ++nameIndex )
    {
        if ( !I_stricmp(selectedName, names[nameIndex]) )
        {
            value = value & ~(f->mask << f->shift) | (nameIndex << f->shift);
            v6 = (char *)hud + f->ofs;
            size = f->size;
            switch ( size )
            {
                case 1:
                    *v6 = value;
                    break;
                case 2:
                    *(_WORD *)v6 = value;
                    break;
                case 4:
                    *(unsigned int *)v6 = value;
                    break;
            }
            return;
        }
    }
    sprintf(errormsg, "\"%s\" is not a valid value for hudelem field \"%s\"\nShould be one of:", selectedName, f->name);
    for ( nameIndex = 0; nameIndex < nameCount; ++nameIndex )
    {
        v4 = va(" %s", names[nameIndex]);
        strncat(errormsg, v4, 0x800u);
        errormsg[2047] = 0;
    }
    Scr_Error(errormsg, 0);
}

void __cdecl HudElem_GetFont(game_hudelem_s *hud, int offset)
{
    HudElem_GetEnumString(hud, &fields_0[offset], g_he_font, 7);
}

void __cdecl HudElem_GetEnumString(
                game_hudelem_s *hud,
                const game_hudelem_field_t *f,
                const char **names,
                int nameCount)
{
    int index; // [esp+0h] [ebp-8h]

    if ( !hud && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp", 434, 0, "%s", "hud") )
        __debugbreak();
    if ( !f && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp", 435, 0, "%s", "f") )
        __debugbreak();
    if ( !names && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp", 436, 0, "%s", "names") )
        __debugbreak();
    if ( nameCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    437,
                    0,
                    "%s\n\t(nameCount) = %i",
                    "(nameCount > 0)",
                    nameCount) )
    {
        __debugbreak();
    }
    index = f->mask & (GetField((char *)hud + f->ofs, f->size) >> f->shift);
    if ( (index < 0 || index >= nameCount)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    441,
                    0,
                    "%s\n\t(index) = %i",
                    "(index >= 0 && index < nameCount)",
                    index) )
    {
        __debugbreak();
    }
    Scr_AddString((char *)names[index], SCRIPTINSTANCE_SERVER);
}

void __cdecl HudElem_SetAlignX(game_hudelem_s *hud, int offset)
{
    HudElem_SetEnumString(hud, &fields_0[offset], g_he_alignx, 3);
}

void __cdecl HudElem_GetAlignX(game_hudelem_s *hud, int offset)
{
    HudElem_GetEnumString(hud, &fields_0[offset], g_he_alignx, 3);
}

void __cdecl HudElem_SetAlignY(game_hudelem_s *hud, int offset)
{
    HudElem_SetEnumString(hud, &fields_0[offset], g_he_aligny, 3);
}

void __cdecl HudElem_GetAlignY(game_hudelem_s *hud, int offset)
{
    HudElem_GetEnumString(hud, &fields_0[offset], g_he_aligny, 3);
}

void __cdecl HudElem_SetHorzAlign(game_hudelem_s *hud, int offset)
{
    HudElem_SetEnumString(hud, &fields_0[offset], g_he_horzalign, 11);
}

void __cdecl HudElem_GetHorzAlign(game_hudelem_s *hud, int offset)
{
    HudElem_GetEnumString(hud, &fields_0[offset], g_he_horzalign, 11);
}

void __cdecl HudElem_SetVertAlign(game_hudelem_s *hud, int offset)
{
    HudElem_SetEnumString(hud, &fields_0[offset], g_he_vertalign, 11);
}

void __cdecl HudElem_GetVertAlign(game_hudelem_s *hud, int offset)
{
    HudElem_GetEnumString(hud, &fields_0[offset], g_he_vertalign, 11);
}

void __cdecl HudElem_SetUI3DWindow(game_hudelem_s *hud, int offset)
{
    if ( fields_0[offset].ofs != 109
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    957,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.ui3dWindow )") )
    {
        __debugbreak();
    }
    hud->elem.ui3dWindow = (unsigned __int8)Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
}

void __cdecl HudElem_GetUI3DWindow(game_hudelem_s *hud, int offset)
{
    if ( fields_0[offset].ofs != 109
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    965,
                    0,
                    "%s",
                    "fields[offset].ofs == HEOFS( elem.ui3dWindow )") )
    {
        __debugbreak();
    }
    Scr_AddInt(hud->elem.ui3dWindow, SCRIPTINSTANCE_SERVER);
}

void __cdecl Scr_GetHudElemField(unsigned int entnum, unsigned int offset)
{
    int size; // [esp+0h] [ebp-Ch]
    game_hudelem_s *hud; // [esp+4h] [ebp-8h]
    const game_hudelem_field_t *f; // [esp+8h] [ebp-4h]

    if ( offset >= 0x1B
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    975,
                    0,
                    "%s",
                    "(unsigned)offset < ARRAY_COUNT( fields ) - 1") )
    {
        __debugbreak();
    }
    if ( entnum >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    976,
                    0,
                    "%s",
                    "(unsigned)entnum < ARRAY_COUNT( g_hudelems )") )
    {
        __debugbreak();
    }
    f = &fields_0[offset];
    hud = &g_hudelems[entnum];
    if ( f->getter )
    {
        f->getter(hud, offset);
    }
    else
    {
        size = f->size;
        if ( size == 1 )
        {
            GScr_GetGenericField((unsigned __int8 *)hud, F_BYTE, f->ofs, 0);
        }
        else if ( size == 2 )
        {
            GScr_GetGenericField((unsigned __int8 *)hud, F_SHORT, f->ofs, 0);
        }
        else
        {
            if ( f->size != 4
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                            996,
                            0,
                            "%s\n\t(f->size) = %i",
                            "(f->size == 4)",
                            f->size) )
            {
                __debugbreak();
            }
            GScr_GetGenericField((unsigned __int8 *)hud, f->type, f->ofs, 0);
        }
    }
}

void __cdecl Scr_SetHudElemField(unsigned int entnum, unsigned int offset)
{
    game_hudelem_s *hud; // [esp+0h] [ebp-8h]
    const game_hudelem_field_t *f; // [esp+4h] [ebp-4h]

    if ( offset >= 0x1B
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    1007,
                    0,
                    "%s",
                    "(unsigned)offset < ARRAY_COUNT( fields ) - 1") )
    {
        __debugbreak();
    }
    if ( entnum >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    1008,
                    0,
                    "%s",
                    "(unsigned)entnum < ARRAY_COUNT( g_hudelems )") )
    {
        __debugbreak();
    }
    f = &fields_0[offset];
    hud = &g_hudelems[entnum];
    if ( f->setter )
    {
        f->setter(hud, offset);
    }
    else
    {
        if ( f->size != 4
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp", 1019, 0, "%s", "f->size == 4") )
        {
            __debugbreak();
        }
        GScr_SetGenericField((unsigned __int8 *)hud, f->type, f->ofs, 0);
    }
}

void __cdecl Scr_FreeHudElemConstStrings(game_hudelem_s *hud)
{
    const game_hudelem_field_t *f; // [esp+0h] [ebp-4h]

    for ( f = fields_0; f->name; ++f )
    {
        if ( f->type == F_STRING )
            Scr_SetString((unsigned __int16 *)((char *)hud + f->ofs), 0, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl GScr_NewHudElem()
{
    game_hudelem_s *hud; // [esp+0h] [ebp-4h]

    hud = HudElem_Alloc(1023, 0);
    if ( !hud )
        Scr_Error("out of hudelems", 0);
    Scr_AddHudElem(hud);
}

void __cdecl GScr_NewDebugHudElem()
{
    game_hudelem_s *hud; // [esp+0h] [ebp-4h]

    hud = HudElem_DebugAlloc(1023);
    if ( !hud )
        Scr_Error("out of hudelems", 0);
    Scr_AddHudElem(hud);
}

void __cdecl GScr_NewClientHudElem()
{
    game_hudelem_s *hud; // [esp+0h] [ebp-8h]
    gentity_s *ent; // [esp+4h] [ebp-4h]

    ent = Scr_GetEntity(0);
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp", 1097, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->r.inuse
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp", 1098, 0, "%s", "ent->r.inuse") )
    {
        __debugbreak();
    }
    if ( !ent->client )
        Scr_ParamError(0, "not a client", SCRIPTINSTANCE_SERVER);
    hud = HudElem_Alloc(ent->s.number, 0);
    if ( !hud )
        Scr_Error("out of hudelems", 0);
    Scr_AddHudElem(hud);
}

void __cdecl GScr_NewScoreHudElem()
{
    game_hudelem_s *hud; // [esp+0h] [ebp-8h]
    gentity_s *ent; // [esp+4h] [ebp-4h]

    ent = Scr_GetEntity(0);
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp", 1129, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->r.inuse
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp", 1130, 0, "%s", "ent->r.inuse") )
    {
        __debugbreak();
    }
    if ( !ent->client )
        Scr_ParamError(0, "not a client", SCRIPTINSTANCE_SERVER);
    hud = HudElem_Alloc(ent->s.number, 0);
    hud->elem.type = 7;
    if ( !hud )
        Scr_Error("out of hudelems", 0);
    Scr_AddHudElem(hud);
}

void __cdecl GScr_NewTeamHudElem()
{
    game_hudelem_s *v0; // eax
    char *String; // eax
    const char *v2; // eax
    unsigned __int16 teamName; // [esp+0h] [ebp-Ch]
    game_hudelem_s *hud; // [esp+8h] [ebp-4h]

    teamName = (unsigned __int16)Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    if ( teamName == scr_const.allies )
    {
        v0 = HudElem_Alloc(1023, 2);
    }
    else if ( teamName == scr_const.axis )
    {
        v0 = HudElem_Alloc(1023, 1);
    }
    else if ( teamName == scr_const.spectator )
    {
        v0 = HudElem_Alloc(1023, 3);
    }
    else
    {
        String = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
        v2 = va("team \"%s\" should be \"allies\", \"axis\", or \"spectator\"", String);
        Scr_ParamError(0, v2, SCRIPTINSTANCE_SERVER);
        v0 = HudElem_Alloc(1023, 0);
    }
    hud = v0;
    if ( !v0 )
        Scr_Error("out of hudelems", 0);
    Scr_AddHudElem(hud);
}

void __cdecl GScr_AddFieldsForHudElems()
{
    const game_hudelem_field_t *f; // [esp+0h] [ebp-4h]

    for ( f = fields_0; f->name; ++f )
    {
        if ( ((f - fields_0) & 0xC000) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                        1194,
                        0,
                        "%s",
                        "((f - fields) & ENTFIELD_MASK) == ENTFIELD_ENTITY") )
        {
            __debugbreak();
        }
        if ( f - fields_0 != (unsigned __int16)(f - fields_0)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                        1195,
                        0,
                        "%s",
                        "(f - fields) == (unsigned short)( f - fields )") )
        {
            __debugbreak();
        }
        Scr_AddClassField(1u, (char *)f->name, (unsigned __int16)(f - fields_0), SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl HECmd_SetText(scr_entref_t entref)
{
    int NumParam; // eax
    int v2; // eax
    char string[1024]; // [esp+0h] [ebp-408h] BYREF
    game_hudelem_s *hud; // [esp+404h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    HudElem_ClearTypeSettings(hud);
    NumParam = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    Scr_ConstructMessageString(0, NumParam - 1, "Hud Elem String", string, 0x400u);
    hud->elem.type = 1;
    v2 = G_LocalizedStringIndex(string);
    AssignToSmallerType<short>(&hud->elem.text, v2);
}

game_hudelem_s *__cdecl HECmd_GetHudElem(scr_entref_t entref)
{
    if ( entref.classnum == 1 )
    {
        if ( entref.entnum >= 0x400u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                        1205,
                        0,
                        "%s",
                        "entref.entnum < ARRAY_COUNT( g_hudelems )") )
        {
            __debugbreak();
        }
        return &g_hudelems[entref.entnum];
    }
    else
    {
        Scr_ObjectError("not a hud element", SCRIPTINSTANCE_SERVER);
        return 0;
    }
}

void __cdecl HECmd_ClearAllTextAfterHudElem(scr_entref_t entref)
{
    int configStringIndex; // [esp+0h] [ebp-8h]
    game_hudelem_s *hud; // [esp+4h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    if ( !hud->elem.text )
        Scr_Error(
            "Hud elem doesn't reference any text.    Make sure to call setText before using clearAllTextAfterHudElem.",
            0);
    for ( configStringIndex = hud->elem.text + 1; configStringIndex < 1023; ++configStringIndex )
        SV_SetConfigstring(configStringIndex + 515, (char *)"");
}

void __cdecl HECmd_SetMaterial(scr_entref_t entref)
{
    char *String; // eax
    const char *v2; // eax
    const char *v3; // eax
    VariableUnion width; // [esp+0h] [ebp-14h]
    VariableUnion height; // [esp+4h] [ebp-10h]
    int materialIndex; // [esp+8h] [ebp-Ch]
    int numParam; // [esp+Ch] [ebp-8h]
    game_hudelem_s *hud; // [esp+10h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    numParam = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    if ( numParam != 1 && numParam != 3 )
        Scr_Error("USAGE: <hudelem> setShader(\"materialname\"[, optional_width, optional_height]);", 0);
    String = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    materialIndex = G_MaterialIndex(String);
    if ( numParam == 1 )
    {
        width.intValue = 0;
        height.intValue = 0;
    }
    else
    {
        width.intValue = Scr_GetInt(1u, SCRIPTINSTANCE_SERVER);
        if ( width.intValue < 0 )
        {
            v2 = va("width %i < 0", width.intValue);
            Scr_ParamError(1u, v2, SCRIPTINSTANCE_SERVER);
        }
        height.intValue = Scr_GetInt(2u, SCRIPTINSTANCE_SERVER);
        if ( height.intValue < 0 )
        {
            v3 = va("height %i < 0", height.intValue);
            Scr_ParamError(2u, v3, SCRIPTINSTANCE_SERVER);
        }
    }
    HudElem_ClearTypeSettings(hud);
    hud->elem.type = 8;
    AssignToSmallerType<unsigned char>(&hud->elem.materialIndex, materialIndex);
    AssignToSmallerType<short>(&hud->elem.width, width.intValue);
    AssignToSmallerType<short>(&hud->elem.height, height.intValue);
}

void __cdecl HECmd_SetWarGameData(scr_entref_t entref)
{
    VariableUnion v1; // eax
    game_hudelem_s *hud; // [esp+4h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) == 3 )
    {
        HudElem_ClearTypeSettings(hud);
        hud->elem.type = 6;
        hud->elem.value = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
        v1.intValue = Scr_GetInt(1u, SCRIPTINSTANCE_SERVER);
        AssignToSmallerType<unsigned char>(&hud->elem.materialIndex, v1.intValue);
        hud->elem.time = Scr_GetInt(2u, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Scr_Error("USAGE: <hudelem> setWarGameData( momentumProgress, momentumMultiplier, momentumBlitzkriegTime );", 0);
    }
}

void __cdecl HECmd_SetTargetEnt(scr_entref_t entref)
{
    game_hudelem_s *hud; // [esp+0h] [ebp-Ch]
    gentity_s *ent; // [esp+4h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    ent = Scr_GetEntity(0);
    AssignToSmallerType<short>(&hud->elem.targetEntNum, ent->s.number);
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if ( (ent->r.svFlags & 0x10) == 0 && !ent->client && ent->s.eType != 11 && !weapDef->plantable )
        Com_PrintWarning(15, "SetTargetEnt() called on a non-broadcasting entity, may not show in client snapshots.\n");
}

void __cdecl HECmd_ClearTargetEnt(scr_entref_t entref)
{
    HECmd_GetHudElem(entref)->elem.targetEntNum = 1023;
}

void __cdecl HECmd_SetTimer(scr_entref_t entref)
{
    HECmd_SetTimer_Internal(entref, HE_TYPE_TIMER_DOWN, "setTimer");
}

void __cdecl HECmd_SetTimer_Internal(scr_entref_t entref, he_type_t type, const char *cmdName)
{
    const char *v3; // eax
    const char *v4; // eax
    float v5; // [esp+Ch] [ebp-18h]
    game_hudelem_s *hud; // [esp+1Ch] [ebp-8h]
    int time; // [esp+20h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    if ( type != HE_TYPE_TIMER_DOWN
        && type != HE_TYPE_TIMER_UP
        && type != HE_TYPE_TENTHS_TIMER_DOWN
        && type != HE_TYPE_TENTHS_TIMER_UP
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    1410,
                    0,
                    "%s\n\t(type) = %i",
                    "(type == HE_TYPE_TIMER_DOWN || type == HE_TYPE_TIMER_UP || type == HE_TYPE_TENTHS_TIMER_DOWN || type == HE_TYP"
                    "E_TENTHS_TIMER_UP)",
                    type) )
    {
        __debugbreak();
    }
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1 )
    {
        v3 = va("USAGE: <hudelem> %s(time_in_seconds);\n", cmdName);
        Scr_Error(v3, 0);
    }
    v5 = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER) * 1000.0;
    time = (int)(v5 + 0.4999999990686774);
    if ( time <= 0 && type != HE_TYPE_TIMER_UP )
    {
        v4 = va("time %g should be > 0", (float)((float)time * 0.001));
        Scr_ParamError(0, v4, SCRIPTINSTANCE_SERVER);
    }
    HudElem_ClearTypeSettings(hud);
    AssignToSmallerType<unsigned char>(&hud->elem.type, type);
    hud->elem.time = time + level.time;
}

void __cdecl HECmd_SetTimerUp(scr_entref_t entref)
{
    HECmd_SetTimer_Internal(entref, HE_TYPE_TIMER_UP, "setTimerUp");
}

void __cdecl HECmd_SetTenthsTimer(scr_entref_t entref)
{
    HECmd_SetTimer_Internal(entref, HE_TYPE_TENTHS_TIMER_DOWN, "setTenthsTimer");
}

void __cdecl HECmd_SetTenthsTimerUp(scr_entref_t entref)
{
    HECmd_SetTimer_Internal(entref, HE_TYPE_TENTHS_TIMER_UP, "setTenthsTimerUp");
}

void __cdecl HECmd_SetClock(scr_entref_t entref)
{
    HECmd_SetClock_Internal(entref, HE_TYPE_CLOCK_DOWN, "setClock");
}

void __cdecl HECmd_SetClock_Internal(scr_entref_t entref, he_type_t type, const char *cmdName)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    char *String; // eax
    const char *v7; // eax
    const char *v8; // eax
    float v9; // [esp+Ch] [ebp-3Ch]
    float v10; // [esp+1Ch] [ebp-2Ch]
    int duration; // [esp+2Ch] [ebp-1Ch]
    int materialIndex; // [esp+30h] [ebp-18h]
    VariableUnion width; // [esp+34h] [ebp-14h]
    VariableUnion height; // [esp+38h] [ebp-10h]
    int numParam; // [esp+3Ch] [ebp-Ch]
    game_hudelem_s *hud; // [esp+40h] [ebp-8h]
    int time; // [esp+44h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    if ( type != HE_TYPE_CLOCK_DOWN
        && type != HE_TYPE_CLOCK_UP
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    1436,
                    0,
                    "%s\n\t(type) = %i",
                    "(type == HE_TYPE_CLOCK_DOWN || type == HE_TYPE_CLOCK_UP)",
                    type) )
    {
        __debugbreak();
    }
    numParam = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    if ( numParam != 3 && numParam != 5 )
    {
        v3 = va(
                     "USAGE: <hudelem> %s(time_in_seconds, total_clock_time_in_seconds, shadername[, width, height]);\n",
                     cmdName);
        Scr_Error(v3, 0);
    }
    v10 = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER) * 1000.0;
    time = (int)(v10 + 0.4999999990686774);
    if ( time <= 0 && type != HE_TYPE_CLOCK_UP )
    {
        v4 = va("time %g should be > 0", (float)((float)time * 0.001));
        Scr_ParamError(0, v4, SCRIPTINSTANCE_SERVER);
    }
    v9 = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER) * 1000.0;
    duration = (int)(v9 + 0.4999999990686774);
    if ( duration <= 0 )
    {
        v5 = va("duration %g should be > 0", (float)((float)duration * 0.001));
        Scr_ParamError(1u, v5, SCRIPTINSTANCE_SERVER);
    }
    String = Scr_GetString(2u, SCRIPTINSTANCE_SERVER);
    materialIndex = G_MaterialIndex(String);
    if ( numParam == 3 )
    {
        width.intValue = 0;
        height.intValue = 0;
    }
    else
    {
        width.intValue = Scr_GetInt(3u, SCRIPTINSTANCE_SERVER);
        if ( width.intValue < 0 )
        {
            v7 = va("width %i < 0", width.intValue);
            Scr_ParamError(3u, v7, SCRIPTINSTANCE_SERVER);
        }
        height.intValue = Scr_GetInt(4u, SCRIPTINSTANCE_SERVER);
        if ( height.intValue < 0 )
        {
            v8 = va("height %i < 0", height.intValue);
            Scr_ParamError(4u, v8, SCRIPTINSTANCE_SERVER);
        }
    }
    HudElem_ClearTypeSettings(hud);
    AssignToSmallerType<unsigned char>(&hud->elem.type, type);
    hud->elem.time = time + level.time;
    hud->elem.duration = duration;
    AssignToSmallerType<unsigned char>(&hud->elem.materialIndex, materialIndex);
    AssignToSmallerType<short>(&hud->elem.width, width.intValue);
    AssignToSmallerType<short>(&hud->elem.height, height.intValue);
}

void __cdecl HECmd_SetClockUp(scr_entref_t entref)
{
    HECmd_SetClock_Internal(entref, HE_TYPE_CLOCK_UP, "setClockUp");
}

void __cdecl HECmd_SetValue(scr_entref_t entref)
{
    game_hudelem_s *hud; // [esp+0h] [ebp-8h]
    float value; // [esp+4h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    value = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    HudElem_ClearTypeSettings(hud);
    if ( hud->elem.type != 7 )
        hud->elem.type = 2;
    hud->elem.value = value;
}

void __cdecl HECmd_SetWaypoint(scr_entref_t entref)
{
    int v1; // eax
    int constantSize; // [esp+0h] [ebp-10h]
    int numParam; // [esp+4h] [ebp-Ch]
    game_hudelem_s *hud; // [esp+8h] [ebp-8h]
    char *materialName; // [esp+Ch] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    numParam = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    constantSize = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    hud->elem.type = 15;
    hud->elem.value = (float)constantSize;
    if ( numParam == 1 )
    {
        hud->elem.offscreenMaterialIdx = 0;
    }
    else
    {
        materialName = Scr_GetString(1u, SCRIPTINSTANCE_SERVER);
        if ( materialName && *materialName )
        {
            v1 = G_MaterialIndex(materialName);
            AssignToSmallerType<unsigned char>(&hud->elem.offscreenMaterialIdx, v1);
        }
        if ( numParam == 3 )
            hud->elem.time = Scr_GetInt(2u, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl HECmd_FadeOverTime(scr_entref_t entref)
{
    const char *v1; // eax
    const char *v2; // eax
    float fadeTime; // [esp+20h] [ebp-8h]
    game_hudelem_s *hud; // [esp+24h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    fadeTime = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    if ( fadeTime > 0.0 )
    {
        if ( fadeTime > 60.0 )
        {
            v2 = va("fade time %g > 60", fadeTime);
            Scr_ParamError(0, v2, SCRIPTINSTANCE_SERVER);
        }
    }
    else
    {
        v1 = va("fade time %g <= 0", fadeTime);
        Scr_ParamError(0, v1, SCRIPTINSTANCE_SERVER);
    }
    BG_LerpHudColors(&hud->elem, level.time, &hud->elem.fromColor);
    hud->elem.fadeStartTime = level.time;
    AssignToSmallerType<short>(&hud->elem.fadeTime, (int)((float)(1000.0 * fadeTime) + 9.313225746154785e-10));
}

void __cdecl HECmd_ScaleOverTime(scr_entref_t entref)
{
    const char *v1; // eax
    const char *v2; // eax
    VariableUnion width; // [esp+20h] [ebp-10h]
    VariableUnion height; // [esp+24h] [ebp-Ch]
    game_hudelem_s *hud; // [esp+28h] [ebp-8h]
    float scaleTime; // [esp+2Ch] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 3 )
        Scr_Error("hudelem scaleOverTime(time_in_seconds, new_width, new_height)", 0);
    scaleTime = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    if ( scaleTime > 0.0 )
    {
        if ( scaleTime > 60.0 )
        {
            v2 = va("scale time %g > 60", scaleTime);
            Scr_ParamError(0, v2, SCRIPTINSTANCE_SERVER);
        }
    }
    else
    {
        v1 = va("scale time %g <= 0", scaleTime);
        Scr_ParamError(0, v1, SCRIPTINSTANCE_SERVER);
    }
    width.intValue = Scr_GetInt(1u, SCRIPTINSTANCE_SERVER);
    height.intValue = Scr_GetInt(2u, SCRIPTINSTANCE_SERVER);
    hud->elem.scaleStartTime = level.time;
    AssignToSmallerType<short>(&hud->elem.scaleTime, (int)((float)(1000.0 * scaleTime) + 9.313225746154785e-10));
    hud->elem.fromWidth = hud->elem.width;
    hud->elem.fromHeight = hud->elem.height;
    AssignToSmallerType<short>(&hud->elem.width, width.intValue);
    AssignToSmallerType<short>(&hud->elem.height, height.intValue);
}

void __cdecl HECmd_MoveOverTime(scr_entref_t entref)
{
    const char *v1; // eax
    const char *v2; // eax
    game_hudelem_s *hud; // [esp+20h] [ebp-8h]
    float moveTime; // [esp+24h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    moveTime = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    if ( moveTime > 0.0 )
    {
        if ( moveTime > 60.0 )
        {
            v2 = va("move time %g > 60", moveTime);
            Scr_ParamError(0, v2, SCRIPTINSTANCE_SERVER);
        }
    }
    else
    {
        v1 = va("move time %g <= 0", moveTime);
        Scr_ParamError(0, v1, SCRIPTINSTANCE_SERVER);
    }
    hud->elem.moveStartTime = level.time;
    AssignToSmallerType<short>(&hud->elem.moveTime, (int)((float)(1000.0 * moveTime) + 9.313225746154785e-10));
    hud->elem.fromX = hud->elem.x;
    hud->elem.fromY = hud->elem.y;
    hud->elem.fromAlignOrg = hud->elem.alignOrg;
    hud->elem.fromAlignScreen = hud->elem.alignScreen;
}

void __cdecl HECmd_Reset(scr_entref_t entref)
{
    game_hudelem_s *hud; // [esp+8h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    HudElem_SetDefaults(hud);
}

void __cdecl HECmd_Destroy(scr_entref_t entref)
{
    game_hudelem_s *hud; // [esp+8h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    HudElem_Free(hud);
}

void __cdecl HECmd_SetPlayerNameString(scr_entref_t entref)
{
    game_hudelem_s *hud; // [esp+0h] [ebp-8h]
    gentity_s *player; // [esp+4h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    player = Scr_GetEntity(0);
    if ( player )
    {
        if ( player->client )
        {
            HudElem_ClearTypeSettings(hud);
            hud->elem.type = 3;
            hud->elem.value = (float)player->s.number;
        }
        else
        {
            Com_Printf(24, "Invalid entity passed to hudelem setplayernamestring(), entity is not a client\n");
        }
    }
    else
    {
        Com_Printf(24, "Invalid entity passed to hudelem setplayernamestring()\n");
    }
}

void __cdecl HECmd_SetGameTypeString(scr_entref_t entref)
{
    char *gametype; // [esp+0h] [ebp-8h]
    game_hudelem_s *hud; // [esp+4h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    gametype = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    if ( gametype )
    {
        if ( Scr_GetGameTypeNameForScript(gametype) )
        {
            SV_SetConfigstring(20, gametype);
            HudElem_ClearTypeSettings(hud);
            hud->elem.type = 5;
            hud->elem.value = 20.0f;
        }
        else
        {
            Com_Printf(24, "Invalid gametype '%s'\n", gametype);
        }
    }
    else
    {
        Com_Printf(24, "Invalid entity passed to hudelem setgametypestring()\n");
    }
}

void __cdecl HECmd_SetMapNameString(scr_entref_t entref)
{
    game_hudelem_s *hud; // [esp+0h] [ebp-8h]
    char *mapname; // [esp+4h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    mapname = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    if ( mapname )
    {
        if ( SV_MapExists(mapname) )
        {
            SV_SetConfigstring(19, mapname);
            HudElem_ClearTypeSettings(hud);
            hud->elem.type = 4;
        }
        else
        {
            Com_Printf(24, "Invalid map name passed to hudelem setmapnamestring(), map not found\n");
        }
    }
    else
    {
        Com_Printf(24, "Invalid mapname passed to hudelem setmapnamestring()\n");
    }
}

void __cdecl HECmd_SetPulseFX(scr_entref_t entref)
{
    VariableUnion v1; // eax
    VariableUnion v2; // eax
    VariableUnion v3; // eax
    game_hudelem_s *hud; // [esp+4h] [ebp-8h]
    gclient_s *ps; // [esp+8h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 3 )
        Scr_Error("USAGE: <hudelem> SetPulseFX( <speed>, <decayStart>, <decayDuration> );\n", 0);
    hud = HECmd_GetHudElem(entref);
    hud->elem.fxBirthTime = level.time;
    v1.intValue = GetIntGTZero(0);
    AssignToSmallerType<unsigned short>(&hud->elem.fxLetterTime, v1.intValue);
    v2.intValue = GetIntGTZero(1u);
    AssignToSmallerType<unsigned short>(&hud->elem.fxDecayStartTime, v2.intValue);
    v3.intValue = GetIntGTZero(2u);
    AssignToSmallerType<unsigned short>(&hud->elem.fxDecayDuration, v3.intValue);
    if ( hud->clientNum == 1023 )
    {
        Scr_Error("SetPulseFX called on a non-client entity hud element\n", 0);
    }
    else
    {
        if ( hud->clientNum >= (unsigned int)level.maxclients
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                        2001,
                        0,
                        "hud->clientNum doesn't index level.maxclients\n\t%i not in [0, %i)",
                        hud->clientNum,
                        level.maxclients) )
        {
            __debugbreak();
        }
        ps = &level.clients[hud->clientNum];
        ++ps->ps.hudElemLastAssignedSoundID;
        ps->ps.hudElemLastAssignedSoundID %= 32;
        if ( !ps->ps.hudElemLastAssignedSoundID )
            ++ps->ps.hudElemLastAssignedSoundID;
        AssignToSmallerType<unsigned char>(&hud->elem.soundID, ps->ps.hudElemLastAssignedSoundID);
    }
}

int __cdecl GetIntGTZero(unsigned int index)
{
    const char *v1; // eax
    int number; // [esp+0h] [ebp-4h]

    number = Scr_GetInt(index, SCRIPTINSTANCE_SERVER);
    if ( number < 0 )
    {
        Scr_ParamError(index, va("Time (%i) must be greater than zero.", number), SCRIPTINSTANCE_SERVER);
    }
    return number;
}

void __cdecl HECmd_SetCOD7DecodeFX(scr_entref_t entref)
{
    VariableUnion v1; // eax
    VariableUnion v2; // eax
    VariableUnion v3; // eax
    game_hudelem_s *hud; // [esp+4h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 3 )
        Scr_Error("USAGE: <hudelem> SetCOD7DecodeFX( <decodeTime>, <decayStart>, <decayDuration> );\n", 0);
    hud = HECmd_GetHudElem(entref);
    hud->elem.flags |= 0x1000u;
    hud->elem.fxBirthTime = level.time;
    v1.intValue = GetIntGTZero(0);
    AssignToSmallerType<unsigned short>(&hud->elem.fxLetterTime, v1.intValue);
    v2.intValue = GetIntGTZero(1u);
    AssignToSmallerType<unsigned short>(&hud->elem.fxDecayStartTime, v2.intValue);
    v3.intValue = GetIntGTZero(2u);
    AssignToSmallerType<unsigned short>(&hud->elem.fxDecayDuration, v3.intValue);
}

void __cdecl HECmd_SetRedactFX(scr_entref_t entref)
{
    VariableUnion v1; // eax
    VariableUnion v2; // eax
    VariableUnion v3; // eax
    VariableUnion v4; // eax
    game_hudelem_s *hud; // [esp+4h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 4 )
        Scr_Error(
            "USAGE: <hudelem> SetRedactFX( <decayStart>, <decayDuration>, <redactDecayStart>, <redactDecayDuration>);\n",
            0);
    hud = HECmd_GetHudElem(entref);
    hud->elem.flags |= 0x80u;
    hud->elem.fxBirthTime = level.time;
    v1.intValue = GetIntGTZero(0);
    AssignToSmallerType<unsigned short>(&hud->elem.fxDecayStartTime, v1.intValue);
    v2.intValue = GetIntGTZero(1u);
    AssignToSmallerType<unsigned short>(&hud->elem.fxDecayDuration, v2.intValue);
    v3.intValue = GetIntGTZero(2u);
    AssignToSmallerType<unsigned short>(&hud->elem.fxRedactDecayStartTime, v3.intValue);
    v4.intValue = GetIntGTZero(3u);
    AssignToSmallerType<unsigned short>(&hud->elem.fxRedactDecayDuration, v4.intValue);
}

void __cdecl HECmd_GetTextWidth(scr_entref_t entref)
{
    char *translation; // [esp+4h] [ebp-98h]
    Font_s *font; // [esp+8h] [ebp-94h]
    int width; // [esp+Ch] [ebp-90h]
    game_hudelem_s *hud; // [esp+10h] [ebp-8Ch]
    char text[132]; // [esp+14h] [ebp-88h] BYREF

    hud = HECmd_GetHudElem(entref);
    if ( hud->elem.type != 1 )
        Scr_Error("GetTextWidth called on a non text type hud elem", 0);
    SV_GetConfigstring(hud->elem.text + 515, text, 128);
    translation = SEH_LocalizeTextMessage(text, text, LOCMSG_SAFE);
    if ( !translation )
        translation = text;
    font = UI_GetFontHandleDefault(hud->elem.fontScale);
    width = UI_TextWidth(translation, 0, font, 1.0);
    Scr_AddInt(width / 2, SCRIPTINSTANCE_SERVER);
}

void (__cdecl *__cdecl HudElem_GetMethod(const char **pName))(scr_entref_t)
{
    unsigned int i; // [esp+18h] [ebp-4h]

    for ( i = 0; i < ARRAY_COUNT(methods_0); ++i )
    {
        if ( !strcmp(*pName, methods_0[i].actionString) )
        {
            *pName = methods_0[i].actionString;
            return methods_0[i].actionFunc;
        }
    }
    return 0;
}

void __cdecl HudElem_UpdateClient(gclient_s *client, int clientNum, hudelem_update_t which)
{
    int archivalCount; // [esp+8h] [ebp-14h]
    int currentCount; // [esp+Ch] [ebp-10h]
    game_hudelem_s *hud; // [esp+10h] [ebp-Ch]
    unsigned int i; // [esp+14h] [ebp-8h]
    hudelem_s *elem; // [esp+18h] [ebp-4h]

    if ( (clientNum < 0 || clientNum >= level.maxclients)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    2192,
                    0,
                    "%s\n\t(clientNum) = %i",
                    "(clientNum >= 0 && clientNum < level.maxclients)",
                    clientNum) )
    {
        __debugbreak();
    }
    if ( !level.gentities[clientNum].r.inuse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                    2193,
                    0,
                    "%s",
                    "level.gentities[clientNum].r.inuse") )
    {
        __debugbreak();
    }
    if ( !client && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp", 2194, 0, "%s", "client") )
        __debugbreak();
    archivalCount = 0;
    currentCount = 0;
    hud = g_hudelems;
    for ( i = 0; i < 0x400; ++i )
    {
        if ( hud->elem.type
            && (hud->elem.flags & 0x2000) == 0
            && (!hud->team || hud->team == client->sess.cs.team)
            && (hud->clientNum == 1023 || hud->clientNum == clientNum) )
        {
            if ( hud->archived )
            {
                if ( (which & 1) != 0 )
                {
                    elem = &client->ps.hud.archival[archivalCount];
                    if ( archivalCount < 31 )
                    {
                        ++archivalCount;
LABEL_27:
                        memcpy(elem, hud, sizeof(hudelem_s));
                    }
                }
            }
            else if ( (which & 2) != 0 )
            {
                elem = &client->ps.hud.current[currentCount];
                if ( currentCount < 31 )
                {
                    ++currentCount;
                    goto LABEL_27;
                }
            }
        }
        ++hud;
    }
    if ( (which & 1) != 0 )
    {
        while ( archivalCount < 31 && client->ps.hud.archival[archivalCount].type )
        {
            memset(
                (unsigned __int8 *)&client->ps.hud.archival[archivalCount],
                0,
                sizeof(client->ps.hud.archival[archivalCount]));
            if ( client->ps.hud.archival[archivalCount].type
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                            2248,
                            0,
                            "%s",
                            "client->ps.hud.archival[archivalCount].type == HE_TYPE_FREE") )
            {
                __debugbreak();
            }
            ++archivalCount;
        }
        while ( archivalCount < 31 )
        {
            if ( memcmp(&client->ps.hud.archival[archivalCount], &g_dummyHudCurrent_0, 0x70u)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                            2255,
                            0,
                            "%s",
                            "!memcmp( &client->ps.hud.archival[archivalCount], &g_dummyHudCurrent, sizeof( g_dummyHudCurrent ) )") )
            {
                __debugbreak();
            }
            ++archivalCount;
        }
    }
    if ( (which & 2) != 0 )
    {
        while ( currentCount < 31 && client->ps.hud.current[currentCount].type )
        {
            memset(&client->ps.hud.current[currentCount], 0, sizeof(client->ps.hud.current[currentCount]));
            if ( client->ps.hud.current[currentCount].type
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                            2267,
                            0,
                            "%s",
                            "client->ps.hud.current[currentCount].type == HE_TYPE_FREE") )
            {
                __debugbreak();
            }
            ++currentCount;
        }
        while ( currentCount < 31 )
        {
            if ( memcmp(&client->ps.hud.current[currentCount], &g_dummyHudCurrent_0, 0x70u)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                            2274,
                            0,
                            "%s",
                            "!memcmp( &client->ps.hud.current[currentCount], &g_dummyHudCurrent, sizeof( g_dummyHudCurrent ) )") )
            {
                __debugbreak();
            }
            ++currentCount;
        }
    }
}

void __cdecl HudElem_ClearClient(gclient_s *client, hudelem_update_t which)
{
    if ( (which & 2) != 0 )
        HudElem_ClearClientSingle(client->ps.hud.current, 31);
    if ( (which & 1) != 0 )
        HudElem_ClearClientSingle(client->ps.hud.archival, 31);
}

void __cdecl HudElem_ClearClientSingle(hudelem_s *elems, int max)
{
    int elemCount; // [esp+8h] [ebp-4h]

    for ( elemCount = 0; elemCount < max && elems[elemCount].type; ++elemCount )
    {
        memset(&elems[elemCount], 0, sizeof(hudelem_s));
        if ( elems[elemCount].type )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                            2288,
                            0,
                            "%s",
                            "elems[elemCount].type == HE_TYPE_FREE") )
                __debugbreak();
        }
    }
    while ( elemCount < max )
    {
        if ( memcmp(&elems[elemCount], &g_dummyHudCurrent_0, 0x70u)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_hudelem.cpp",
                        2295,
                        0,
                        "%s",
                        "!memcmp( &elems[elemCount], &g_dummyHudCurrent, sizeof( g_dummyHudCurrent ) )") )
        {
            __debugbreak();
        }
        ++elemCount;
    }
}

