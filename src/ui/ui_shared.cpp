#include "ui_shared.h"
#include "ui_shared_obj.h"
#include "ui_localvars.h"
#include "ui_utils.h"
#include <universal/q_parse.h>
#include <universal/assertive.h>
#include <stringed/stringed_hooks.h>
#include <qcommon/common.h>

#include <string.h>
#include <gfx_d3d/r_material.h>
#include <universal/com_expressions_eval.h>
#include <win32/win_shared.h>
#include <gfx_d3d/rb_draw3d.h>
#include <gfx_d3d/r_rendercmds.h>
#include <client_mp/cl_scrn_mp.h>
#include "ui_feeders.h"
#include <cgame_mp/cg_main_mp.h>
#include <client/splitscreen.h>
#include <universal/com_stringtable.h>
#include <qcommon/com_clients.h>
#include <client_mp/cl_cgame_mp.h>
#include <client/cl_keys.h>
#include <ui_mp/ui_feeders_mp.h>
#include <win32/win_gamerprofile.h>
#include <win32/win_gamepad.h>
#include <cgame_mp/cg_consolecmds_mp.h>
#include <cgame_mp/cg_newDraw_mp.h>
#include <cgame/cg_hudelem.h>
#include <gfx_d3d/r_ui3d.h>
#include "ui_atoms.h"
#include <client/cl_console.h>
#include <iterator>
#include <client_mp/cl_input_mp.h>
#include <sound/snd_public_async.h>
#include <cgame/cg_sound.h>

cmd_function_s UI_SetLocalVarBool_f_VAR;
cmd_function_s UI_SetLocalVarInt_f_VAR;
cmd_function_s UI_SetLocalVarFloat_f_VAR;
cmd_function_s UI_SetLocalVarString_f_VAR;
cmd_function_s UI_FadeItem_f_VAR;
cmd_function_s UI_ShowItem_f_VAR;
cmd_function_s UI_HideItem_f_VAR;
cmd_function_s UI_ShowMenu_f_VAR;
cmd_function_s UI_HideMenu_f_VAR;
cmd_function_s UI_OpenMenu_f_VAR;
cmd_function_s UI_CloseMenu_f_VAR;
cmd_function_s UI_OpenMenuImmediate_f_VAR;
cmd_function_s UI_CloseMenuImmediate_f_VAR;
cmd_function_s UI_ChangeMenuOpenSlideDirection_f_VAR;
cmd_function_s UI_ChangeMenuCloseSlideDirection_f_VAR;
cmd_function_s UI_OpenToastPopup_f_VAR;
cmd_function_s UI_FocusItem_f_VAR;
cmd_function_s UI_PlaySound_f_VAR;
cmd_function_s UI_MoveFeeder_f_VAR;
cmd_function_s UI_AddToFeeder_f_VAR;
cmd_function_s UI_AddToFeederExtended_f_VAR;
cmd_function_s UI_ClearFeederWithoutResetCursor_f_VAR;
cmd_function_s UI_ClearFeeder_f_VAR;
cmd_function_s UI_RaiseFeederEvent_f_VAR;
cmd_function_s UI_RefreshFeeder_f_VAR;
cmd_function_s UI_RefreshFeederSelection_f_VAR;
cmd_function_s UI_ChangeRowStatus_f_VAR;
cmd_function_s UI_ValidatePrivateMatchGametype_f_VAR;
cmd_function_s UI_SetActiveMenu_f_VAR;

sharedUiInfo_t sharedUiInfo;

scrollInfo_s scrollInfo;

int g_editingField;
itemDef_s *g_editItem;
rectDef_s rect;

void(__cdecl *captureFunc)(int, UiContext *, void *);
void *captureData;
itemDef_s *itemCapture;
itemDef_s *g_bindItem;

int mouseLocationX = 2147483647;
int mouseLocationY = 2147483647;
int mouseTimeUntilReadyToMove;
int lastListBoxClickTime;
int g_waitingForKey;
int inHandleKey;
int g_debugMode;

struct commandDef_t // sizeof=0x8
{                                       // XREF: .rdata:commandList/r
    const char *name;                   // XREF: Item_RunScript+AC/r
    void (__cdecl *handler)(int, UiContext *, itemDef_s *, const char **);
};

const commandDef_t commandList[50] =
{
  { "fadein", Script_FadeIn },
  { "fadeout", Script_FadeOut },
  { "show", Script_Show },
  { "hide", Script_Hide },
  { "showMenu", Script_ShowMenu },
  { "hideMenu", Script_HideMenu },
  { "setcolor", Script_SetColor },
  { "open", Script_Open },
  { "openImmediate", Script_OpenImmediate },
  { "close", Script_Close },
  { "closeImmediate", Script_CloseImmediate },
  { "ingameopen", Script_InGameOpen },
  { "ingameclose", Script_InGameClose },
  { "setbackground", Script_SetBackground },
  { "setitemcolor", Script_SetItemColor },
  { "focusfirst", Script_FocusFirstInMenu },
  { "setfocus", Script_SetFocus },
  { "setfocusbydvar", Script_SetFocusByDvar },
  { "changeState", Script_ChangeState },
  { "setdvar", Script_SetDvar },
  { "resetdvar", Script_ResetDvar },
  { "exec", Script_Exec },
  { "execdvar", Script_ExecDvar },
  { "execnow", Script_ExecNow },
  { "execkeyhandler", Script_ExecKeyHandler },
  { "execkeypress", Script_ExecKeyPress },
  { "execOnDvarStringValue", Script_ExecOnDvarStringValue },
  { "execOnDvarIntValue", Script_ExecOnDvarIntValue },
  { "execOnDvarFloatValue", Script_ExecOnDvarFloatValue },
  { "execNowOnDvarStringValue", Script_ExecNowOnDvarStringValue },
  { "execNowOnDvarIntValue", Script_ExecNowOnDvarIntValue },
  { "execNowOnDvarFloatValue", Script_ExecNowOnDvarFloatValue },
  { "play", Script_Play },
  { "scriptmenuresponse", Script_ScriptMenuResponse },
  { "scriptMenuRespondOnDvarStringValue", Script_RespondOnDvarStringValue },
  { "scriptMenuRespondOnDvarIntValue", Script_RespondOnDvarIntValue },
  { "scriptMenuRespondOnDvarFloatValue", Script_RespondOnDvarFloatValue },
  { "setLocalVarBool", Script_SetLocalVarBool },
  { "setLocalVarInt", Script_SetLocalVarInt },
  { "setLocalVarFloat", Script_SetLocalVarFloat },
  { "setLocalVarString", Script_SetLocalVarString },
  { "feederTop", Script_FeederTop },
  { "feederBottom", Script_FeederBottom },
  { "setDvarFromLocString", Script_SetDvarFromLocString },
  { "setUIVisibilityBit", Script_SetUIVisibilityBit },
  { "setDvarStringUsingTable", Script_SetDvarStringUsingTable },
  { "openforgametype", Script_OpenForGameType },
  { "closeforgametype", Script_CloseForGameType },
  { "activateblur", Script_ActivateBlur },
  { "deactivateblur", Script_DeactivateBlur }
};



void __cdecl Menu_Setup(UiContext *dc)
{
    dc->menuCount = 0;
    dc->openMenuCount = 0;
    dc->blurMenuStackIndex = -1;
    Item_SetupKeywordHash();
    Menu_SetupKeywordHash();
    UILocalVar_Init(&dc->localVars);
}

void __cdecl LerpColor(float *a, float *b, float *c, float t)
{
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 4; ++i )
    {
        c[i] = (float)((float)(b[i] - a[i]) * t) + a[i];
        if ( c[i] >= 0.0 )
        {
            if ( c[i] > 1.0 )
                c[i] = 1.0f;
        }
        else
        {
            c[i] = 0.0f;
        }
    }
}

int __cdecl Color_Parse(const char **p, float (*c)[4])
{
    float f; // [esp+0h] [ebp-8h] BYREF
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 4; ++i )
    {
        if ( !Float_Parse(p, &f) )
            return 0;
        (*c)[i] = f;
    }
    return 1;
}

int __cdecl String_Parse(const char **p, char *out, int len)
{
    const char *pszTranslated; // [esp+0h] [ebp-8h]
    parseInfo_t *token; // [esp+4h] [ebp-4h]

    token = Com_ParseOnLine(p);
    if ( !token && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 376, 0, "%s", "token") )
        __debugbreak();
    if ( !*p )
        return 0;
    if ( token->token[0] == 64 )
    {
        pszTranslated = SEH_StringEd_GetString(&token->token[1]);
        if ( pszTranslated )
        {
            I_strncpyz(out, pszTranslated, len);
            return 1;
        }
        if ( Dvar_GetBool("loc_warnings") )
        {
            if ( Dvar_GetBool("loc_warningsAsErrors") )
                Com_Error(ERR_LOCALIZATION, "Could not translate menu string reference %s", token->token);
            else
                Com_PrintWarning(13, "WARNING: Could not translate menu string reference %s\n", token->token);
        }
    }
    I_strncpyz(out, token->token, len);
    return 1;
}

bool __cdecl Command_Parse(char **p, char *out, int len)
{
    char *firstNonWhitespace; // [esp+0h] [ebp-1Ch]
    char *firstQuote; // [esp+4h] [ebp-18h]
    char *lastQuote; // [esp+8h] [ebp-14h]
    const char *c; // [esp+Ch] [ebp-10h]
    char *lastNonWhitespace; // [esp+10h] [ebp-Ch]
    int count; // [esp+14h] [ebp-8h]
    int quoteCount; // [esp+18h] [ebp-4h]

    count = 0;
    quoteCount = 0;
    firstNonWhitespace = 0;
    firstQuote = 0;
    lastNonWhitespace = 0;
    lastQuote = 0;
    *out = 0;
    while ( **p )
    {
        if ( count == len )
            return 0;
        c = *p;
        out[count] = **p;
        if ( *c == 34 && *(c - 1) != 92 )
        {
            ++quoteCount;
            if ( !firstQuote )
                firstQuote = &out[count];
            lastQuote = &out[count];
        }
        if ( *c != 32 && *c != 9 )
        {
            if ( !firstNonWhitespace )
                firstNonWhitespace = &out[count];
            if ( *c != 59 )
                lastNonWhitespace = &out[count];
        }
        ++count;
        ++*p;
        if ( *c == 59 && (quoteCount & 1) == 0 )
            break;
    }
    out[count] = 0;
    if ( quoteCount == 2 && firstQuote && lastQuote && firstQuote == firstNonWhitespace && lastQuote == lastNonWhitespace )
    {
        *firstQuote = 32;
        *lastQuote = 32;
    }
    return count > 0;
}

bool __cdecl Rect_ContainsPoint(int contextIndex, const rectDef_s *rect, float x, float y)
{
    float compareY; // [esp+8h] [ebp-24h]
    rectDef_s compareRect; // [esp+Ch] [ebp-20h] BYREF
    const ScreenPlacement *scrPlace; // [esp+24h] [ebp-8h]
    float compareX; // [esp+28h] [ebp-4h]

    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 936, 0, "%s", "rect") )
        __debugbreak();
    compareRect.x = rect->x;
    compareRect.y = rect->y;
    compareRect.w = rect->w;
    compareRect.h = rect->h;
    scrPlace = &scrPlaceView[contextIndex];
    compareX = ScrPlace_ApplyX(scrPlace, x, 4);
    compareY = ScrPlace_ApplyY(scrPlace, y, 4);
    ScrPlace_ApplyRect(
        scrPlace,
        &compareRect.x,
        &compareRect.y,
        &compareRect.w,
        &compareRect.h,
        rect->horzAlign,
        rect->vertAlign);
    return compareX >= compareRect.x
            && (float)(compareRect.x + compareRect.w) >= compareX
            && compareY >= compareRect.y
            && (float)(compareRect.y + compareRect.h) >= compareY;
}

itemDef_s *__cdecl Menu_GetMatchingItemByNumber(menuDef_t *menu, int index, char *name)
{
    char *v3; // eax
    int wildcard; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    int count; // [esp+Ch] [ebp-4h]

    count = 0;
    wildcard = -1;
    v3 = strstr(name, "*");
    if ( v3 )
        wildcard = (unsigned int)v3 - (unsigned int)name;
    for ( i = 0; i < menu->itemCount; ++i )
    {
        if ( wildcard == -1 )
        {
            if ( menu->items[i]->window.name && !I_stricmp(menu->items[i]->window.name, name)
                || menu->items[i]->window.group && !I_stricmp(menu->items[i]->window.group, name) )
            {
                if ( count == index )
                    return menu->items[i];
                ++count;
            }
        }
        else if ( menu->items[i]->window.name && !I_strncmp(menu->items[i]->window.name, name, wildcard)
                     || menu->items[i]->window.group && !I_strncmp(menu->items[i]->window.group, name, wildcard) )
        {
            if ( count == index )
                return menu->items[i];
            ++count;
        }
    }
    return 0;
}

void __cdecl Script_SetColor(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    float *out; // [esp+0h] [ebp-414h]
    char name[1028]; // [esp+4h] [ebp-410h] BYREF
    float f; // [esp+40Ch] [ebp-8h] BYREF
    int i; // [esp+410h] [ebp-4h]

    if ( String_Parse(args, name, 1024) )
    {
        out = 0;
        if ( I_stricmp(name, "backcolor") )
        {
            if ( I_stricmp(name, "forecolor") )
            {
                if ( !I_stricmp(name, "bordercolor") )
                    out = item->window.borderColor;
            }
            else
            {
                out = item->window.foreColor;
                Window_AddDynamicFlags(dc->contextIndex, &item->window, 0x10000);
            }
        }
        else
        {
            out = item->window.backColor;
            Window_AddDynamicFlags(dc->contextIndex, &item->window, 0x8000);
        }
        if ( out )
        {
            for ( i = 0; i < 4 && Float_Parse(args, &f); ++i )
                out[i] = f;
        }
    }
}

void __cdecl Script_SetBackground(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if ( String_Parse(args, name, 1024) )
        item->window.background = Material_RegisterHandle(name, item->imageTrack);
}

itemDef_s *__cdecl Menu_FindItemByName(menuDef_t *menu, const char *p)
{
    int i; // [esp+0h] [ebp-4h]

    if ( !menu || !p )
        return 0;
    for ( i = 0; i < menu->itemCount; ++i )
    {
        if ( menu->items[i]->window.name && !I_stricmp(p, menu->items[i]->window.name) )
            return menu->items[i];
    }
    return 0;
}

void __cdecl UI_UpdateContextItem(
                UiContext *dc,
                const char *menuName,
                const char *itemName,
                float offsetX,
                float offsetY)
{
    UI_UpdateContextItemInternal(dc, menuName, itemName, offsetX, offsetY);
}

void __cdecl UI_UpdateContextItemInternal(
                UiContext *dc,
                const char *menuName,
                const char *itemName,
                float offsetX,
                float offsetY)
{
    int i; // [esp+8h] [ebp-8h]
    menuDef_t *menu; // [esp+Ch] [ebp-4h]

    menu = Menus_FindByName(dc, menuName);
    if ( menu )
    {
        for ( i = 0; i < menu->itemCount; ++i )
        {
            if ( menu->items[i]->window.name && !I_stricmp(itemName, menu->items[i]->window.name) )
            {
                Item_SnapRectClientToMouse(dc, menu->items[i], dc->cursor.x + offsetX, dc->cursor.y + offsetY);
                return;
            }
        }
    }
}

void __cdecl Item_SnapRectClientToMouse(UiContext *dc, itemDef_s *item, float x, float y)
{
    float v4; // [esp+8h] [ebp-50h]
    float v5; // [esp+Ch] [ebp-4Ch]
    float v6; // [esp+10h] [ebp-48h]
    float v7; // [esp+14h] [ebp-44h]
    float v8; // [esp+18h] [ebp-40h]
    float v9; // [esp+1Ch] [ebp-3Ch]
    float compareY; // [esp+20h] [ebp-38h]
    const ScreenPlacement *scrPlace; // [esp+24h] [ebp-34h]
    float compareX; // [esp+28h] [ebp-30h]
    float compareH; // [esp+2Ch] [ebp-2Ch]
    itemDef_s *sbList; // [esp+30h] [ebp-28h]
    float compareW; // [esp+34h] [ebp-24h]
    menuDef_t *sb; // [esp+38h] [ebp-20h]
    int i; // [esp+3Ch] [ebp-1Ch]
    rectDef_s rect; // [esp+40h] [ebp-18h] BYREF

    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 882, 0, "%s", "item") )
        __debugbreak();
    scrPlace = &scrPlaceView[dc->contextIndex];
    compareX = ScrPlace_ApplyX(scrPlace, x, 4);
    compareY = ScrPlace_ApplyY(scrPlace, y, 4);
    compareW = ScrPlace_ApplyX(scrPlace, item->window.rect.w, 4);
    compareH = ScrPlace_ApplyY(scrPlace, item->window.rect.h, 4);
    sbList = 0;
    for ( i = dc->openMenuCount - 1; i >= 0 && !sbList; --i )
    {
        sb = dc->menuStack[i].menu;
        if ( sb )
            sbList = Menu_FindItemByName(sb, "browser");
    }
    if ( sbList )
    {
        rect = sbList->window.rect;
        ScrPlace_ApplyRect(scrPlace, &rect.x, &rect.y, &rect.w, &rect.h, rect.horzAlign, rect.vertAlign);
        v8 = rect.x;
        v9 = (float)(rect.x + rect.w) - compareW;
        if ( v9 <= rect.x
            && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 730, 0, "%s", "min < max") )
        {
            __debugbreak();
        }
        if ( v8 <= compareX )
        {
            if ( compareX > v9 )
                compareX = v9;
        }
        else
        {
            compareX = v8;
        }
        v6 = rect.y;
        v7 = (float)(rect.y + rect.h) - compareH;
        if ( v7 <= rect.y
            && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 730, 0, "%s", "min < max") )
        {
            __debugbreak();
        }
        if ( v6 <= compareY )
        {
            if ( compareY > v7 )
                compareY = v7;
        }
        else
        {
            compareY = v6;
        }
    }
    else
    {
        v5 = scrPlace->realViewportSize[0] - compareW;
        if ( v5 <= 0.0
            && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 730, 0, "%s", "min < max") )
        {
            __debugbreak();
        }
        if ( compareX >= 0.0 )
        {
            if ( compareX > v5 )
                compareX = v5;
        }
        else
        {
            compareX = 0.0f;
        }
        v4 = scrPlace->realViewportSize[1] - compareH;
        if ( v4 <= 0.0
            && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 730, 0, "%s", "min < max") )
        {
            __debugbreak();
        }
        if ( compareY >= 0.0 )
        {
            if ( compareY > v4 )
                compareY = v4;
        }
        else
        {
            compareY = 0.0f;
        }
    }
    item->window.rectClient.x = ScrPlace_ApplyXinverse(scrPlace, compareX, item->window.rectClient.horzAlign);
    item->window.rectClient.y = ScrPlace_ApplyYinverse(scrPlace, compareY, item->window.rectClient.vertAlign);
    item->window.rect.x = item->window.rectClient.x;
    item->window.rect.y = item->window.rectClient.y;
}

void __cdecl UI_AnimateItem(
                int localClientNum,
                UiContext *dc,
                const char *menuName,
                const char *itemName,
                const char *newStateName,
                int milliseconds)
{
    UI_AnimateItemInternal(localClientNum, dc, menuName, itemName, newStateName, milliseconds, 0);
}

void __cdecl UI_AnimateItemInternal(
                int localClientNum,
                UiContext *dc,
                const char *menuName,
                const char *itemName,
                const char *newStateName,
                int milliseconds,
                bool sysTime)
{
    itemDef_s *item; // [esp+0h] [ebp-14h]
    animParamsDef_t *newState; // [esp+4h] [ebp-10h]
    int i; // [esp+8h] [ebp-Ch]
    bool itemExists; // [esp+Eh] [ebp-6h]
    bool stateExists; // [esp+Fh] [ebp-5h]
    menuDef_t *menu; // [esp+10h] [ebp-4h]

    menu = Menus_FindByName(dc, menuName);
    if ( menu )
    {
        itemExists = 0;
        stateExists = 0;
        for ( i = 0; i < menu->itemCount; ++i )
        {
            if ( *itemName == 42 || menu->items[i]->window.name && !I_stricmp(itemName, menu->items[i]->window.name) )
            {
                itemExists = 1;
                item = menu->items[i];
                newState = Item_FindStateByName(item, newStateName);
                if ( newState )
                {
                    stateExists = 1;
                    Item_AnimateToState(localClientNum, dc, item, newState, milliseconds, sysTime);
                }
            }
        }
        if ( itemExists )
        {
            if ( !stateExists )
                Com_PrintWarning(
                    16,
                    "Warning : ui_animate - item \"%s\" in menu \"%s\" does not contain a state named \"%s\".\n",
                    itemName,
                    menuName,
                    newStateName);
        }
        else
        {
            Com_PrintWarning(
                16,
                "Warning : ui_animate - menu \"%s\" does not contain an item named \"%s\".\n",
                menuName,
                itemName);
        }
    }
}

animParamsDef_t *__cdecl Item_FindStateByName(itemDef_s *item, const char *p)
{
    int i; // [esp+0h] [ebp-4h]

    if ( !item || !p || !item->animInfo )
        return 0;
    for ( i = 0; i < item->animInfo->animStateCount; ++i )
    {
        if ( item->animInfo->animStates[i]->name && !I_stricmp(p, item->animInfo->animStates[i]->name) )
            return item->animInfo->animStates[i];
    }
    return 0;
}

void __cdecl Item_AnimateToState(
                int localClientNum,
                UiContext *dc,
                itemDef_s *item,
                animParamsDef_t *state,
                int milliseconds,
                bool sysTime)
{
    menuDef_t *parent; // esi
    menuDef_t *v7; // esi
    bool wasAnimating; // [esp+13h] [ebp-1h]

    wasAnimating = 0;
    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 1272, 0, "%s", "item") )
        __debugbreak();
    if ( !state && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 1273, 0, "%s", "state") )
        __debugbreak();
    if ( item->animInfo )
    {
        if ( item->animInfo->animating )
            wasAnimating = 1;
        Item_CompleteAnimation(item);
        Item_PropertiesToAnimState(item, &item->animInfo->currentAnimState);
        if ( item->rectExpData )
        {
            if ( wasAnimating )
            {
                if ( item->rectExpData->rectXExp.filename )
                    item->animInfo->currentAnimState.rectClient.x = item->window.rect.x;
                if ( item->rectExpData->rectYExp.filename )
                    item->animInfo->currentAnimState.rectClient.y = item->window.rect.y;
                if ( item->rectExpData->rectWExp.filename )
                    item->animInfo->currentAnimState.rectClient.w = item->window.rect.w;
                if ( item->rectExpData->rectHExp.filename )
                    item->animInfo->currentAnimState.rectClient.h = item->window.rect.h;
            }
            else
            {
                if ( item->rectExpData->rectXExp.filename )
                {
                    parent = item->parent;
                    item->animInfo->currentAnimState.rectClient.x = GetExpressionFloat(
                                                                                                                        localClientNum,
                                                                                                                        item,
                                                                                                                        &item->rectExpData->rectXExp)
                                                                                                                + parent->window.rect.x;
                }
                if ( item->rectExpData->rectYExp.filename )
                {
                    v7 = item->parent;
                    item->animInfo->currentAnimState.rectClient.y = GetExpressionFloat(
                                                                                                                        localClientNum,
                                                                                                                        item,
                                                                                                                        &item->rectExpData->rectYExp)
                                                                                                                + v7->window.rect.y;
                }
                if ( item->rectExpData->rectWExp.filename )
                    item->animInfo->currentAnimState.rectClient.w = GetExpressionFloat(
                                                                                                                        localClientNum,
                                                                                                                        item,
                                                                                                                        &item->rectExpData->rectWExp);
                if ( item->rectExpData->rectHExp.filename )
                    item->animInfo->currentAnimState.rectClient.h = GetExpressionFloat(
                                                                                                                        localClientNum,
                                                                                                                        item,
                                                                                                                        &item->rectExpData->rectHExp);
            }
        }
        memcpy(&item->animInfo->nextAnimState, state, sizeof(item->animInfo->nextAnimState));
        item->animInfo->animDuration = (int)(float)((float)milliseconds * (float)(1.0 / ui_animSpeedScale->current.value));
        if ( sysTime )
        {
            item->animInfo->animStartTime = Sys_Milliseconds();
            item->animInfo->animating = 2;
        }
        else
        {
            item->animInfo->animStartTime = dc->realTime;
            item->animInfo->animating = 1;
        }
        Item_UpdateAnimation(localClientNum, dc, item);
    }
}

void __cdecl Item_CompleteAnimation(itemDef_s *item)
{
    if ( item->animInfo )
    {
        if ( item->animInfo->animating )
            item->animInfo->animating = 0;
    }
}

void __cdecl Item_UpdateAnimation(int localClientNum, UiContext *dc, itemDef_s *item)
{
    float v3; // [esp+4h] [ebp-14h]
    float v4; // [esp+8h] [ebp-10h]
    float v5; // [esp+Ch] [ebp-Ch]
    float lerpValue; // [esp+10h] [ebp-8h]
    int msElapsed; // [esp+14h] [ebp-4h]

    if ( item->animInfo )
    {
        if ( !item->animInfo->animating
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                        1238,
                        0,
                        "%s",
                        "item->animInfo->animating") )
        {
            __debugbreak();
        }
        if ( item->animInfo->animating == 1 )
        {
            msElapsed = dc->realTime - item->animInfo->animStartTime;
        }
        else
        {
            if ( item->animInfo->animating != 2
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                            1246,
                            0,
                            "%s",
                            "item->animInfo->animating == UI_ANIM_ANIMATING_SYS") )
            {
                __debugbreak();
            }
            msElapsed = Sys_Milliseconds() - item->animInfo->animStartTime;
        }
        if ( msElapsed < 0 )
            msElapsed = item->animInfo->animDuration;
        if ( item->animInfo->animDuration )
        {
            v4 = (float)msElapsed / (float)item->animInfo->animDuration;
            if ( (float)(v4 - 1.0) < 0.0 )
                v5 = (float)msElapsed / (float)item->animInfo->animDuration;
            else
                v5 = 1.0f;
            if ( (float)(0.0 - v4) < 0.0 )
                v3 = v5;
            else
                v3 = 0.0f;
            lerpValue = v3;
        }
        else
        {
            lerpValue = 1.0f;
        }
        Item_LerpAnimationParameters(localClientNum, dc->contextIndex, item, lerpValue);
        if ( msElapsed >= item->animInfo->animDuration )
        {
            item->animInfo->animating = 0;
            UIAnimation_RunEventScript(localClientNum, dc, item, &item->animInfo->nextAnimState, "onEnter");
        }
    }
}

void __cdecl Item_LerpAnimationParameters(int localClientNum, int contextIndex, itemDef_s *item, float lerpValue)
{
    float v4; // [esp+4h] [ebp-1Ch]
    float v5; // [esp+8h] [ebp-18h]
    float v6; // [esp+Ch] [ebp-14h]
    float v7; // [esp+10h] [ebp-10h]
    listBoxDef_s *listPtr; // [esp+18h] [ebp-8h]
    const char *maxScroll; // [esp+1Ch] [ebp-4h]

    if ( item->animInfo )
    {
        if ( item->rectExpData )
        {
            if ( localClientNum == -1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                            1177,
                            0,
                            "%s",
                            "localClientNum != INVALID_CLIENT_NUMBER") )
            {
                __debugbreak();
            }
            if ( item->rectExpData->rectXExp.filename )
            {
                v7 = (1.0 - lerpValue) * item->animInfo->currentAnimState.rectClient.x;
                item->window.rectClient.x = GetExpressionFloat(localClientNum, item, &item->rectExpData->rectXExp) * lerpValue
                                                                    + v7;
            }
            else
            {
                item->window.rectClient.x = (float)((float)(1.0 - lerpValue) * item->animInfo->currentAnimState.rectClient.x)
                                                                    + (float)(item->animInfo->nextAnimState.rectClient.x * lerpValue);
            }
            if ( item->rectExpData->rectYExp.filename )
            {
                v6 = (1.0 - lerpValue) * item->animInfo->currentAnimState.rectClient.y;
                item->window.rectClient.y = GetExpressionFloat(localClientNum, item, &item->rectExpData->rectYExp) * lerpValue
                                                                    + v6;
            }
            else
            {
                item->window.rectClient.y = (float)((float)(1.0 - lerpValue) * item->animInfo->currentAnimState.rectClient.y)
                                                                    + (float)(item->animInfo->nextAnimState.rectClient.y * lerpValue);
            }
            if ( item->rectExpData->rectWExp.filename )
            {
                v5 = (1.0 - lerpValue) * item->animInfo->currentAnimState.rectClient.w;
                item->window.rectClient.w = GetExpressionFloat(localClientNum, item, &item->rectExpData->rectWExp) * lerpValue
                                                                    + v5;
            }
            else
            {
                item->window.rectClient.w = (float)((float)(1.0 - lerpValue) * item->animInfo->currentAnimState.rectClient.w)
                                                                    + (float)(item->animInfo->nextAnimState.rectClient.w * lerpValue);
            }
            if ( item->rectExpData->rectHExp.filename )
            {
                v4 = (1.0 - lerpValue) * item->animInfo->currentAnimState.rectClient.h;
                item->window.rectClient.h = GetExpressionFloat(localClientNum, item, &item->rectExpData->rectHExp) * lerpValue
                                                                    + v4;
            }
            else
            {
                item->window.rectClient.h = (float)((float)(1.0 - lerpValue) * item->animInfo->currentAnimState.rectClient.h)
                                                                    + (float)(item->animInfo->nextAnimState.rectClient.h * lerpValue);
            }
        }
        else
        {
            item->window.rectClient.x = (float)((float)(1.0 - lerpValue) * item->animInfo->currentAnimState.rectClient.x)
                                                                + (float)(item->animInfo->nextAnimState.rectClient.x * lerpValue);
            item->window.rectClient.y = (float)((float)(1.0 - lerpValue) * item->animInfo->currentAnimState.rectClient.y)
                                                                + (float)(item->animInfo->nextAnimState.rectClient.y * lerpValue);
            item->window.rectClient.w = (float)((float)(1.0 - lerpValue) * item->animInfo->currentAnimState.rectClient.w)
                                                                + (float)(item->animInfo->nextAnimState.rectClient.w * lerpValue);
            item->window.rectClient.h = (float)((float)(1.0 - lerpValue) * item->animInfo->currentAnimState.rectClient.h)
                                                                + (float)(item->animInfo->nextAnimState.rectClient.h * lerpValue);
        }
        item->window.borderSize = (float)((float)(1.0 - lerpValue) * item->animInfo->currentAnimState.borderSize)
                                                        + (float)(item->animInfo->nextAnimState.borderSize * lerpValue);
        item->window.rotation = (float)((float)(1.0 - lerpValue) * item->animInfo->currentAnimState.rotation)
                                                    + (float)(item->animInfo->nextAnimState.rotation * lerpValue);
        LerpColor(
            item->animInfo->currentAnimState.foreColor,
            item->animInfo->nextAnimState.foreColor,
            item->window.foreColor,
            lerpValue);
        LerpColor(
            item->animInfo->currentAnimState.backColor,
            item->animInfo->nextAnimState.backColor,
            item->window.backColor,
            lerpValue);
        LerpColor(
            item->animInfo->currentAnimState.borderColor,
            item->animInfo->nextAnimState.borderColor,
            item->window.borderColor,
            lerpValue);
        LerpColor(
            item->animInfo->currentAnimState.outlineColor,
            item->animInfo->nextAnimState.outlineColor,
            item->window.outlineColor,
            lerpValue);
        if ( Item_IsTextDefType(item) )
            item->typeData.textDef->textscale = (float)((float)(1.0 - lerpValue) * item->animInfo->currentAnimState.textScale)
                                                                                + (float)(item->animInfo->nextAnimState.textScale * lerpValue);
        if ( item->dataType == 4 && item->window.name && !I_strcmp(item->window.name, "browser") )
        {
            listPtr = Item_GetListBoxDef(item);
            maxScroll = Item_ListBox_MaxScroll(localClientNum, contextIndex, item);
            if ( listPtr->startPos[contextIndex] > (int)maxScroll )
                listPtr->startPos[contextIndex] = (int)maxScroll;
        }
    }
}

void __cdecl UI_AnimateItemSysTime(
                int localClientNum,
                UiContext *dc,
                const char *menuName,
                const char *itemName,
                const char *newStateName,
                int milliseconds)
{
    UI_AnimateItemInternal(localClientNum, dc, menuName, itemName, newStateName, milliseconds, 1);
}

void __cdecl UI_SetShaderTime(int localClientNum)
{
    float gameTime; // xmm0_4
    unsigned __int16 CodeConst_GameTimeParam; // ax
    float timeVec[4]; // [esp+Ch] [ebp-10h] BYREF

    gameTime = (float)UI_GetInfo(localClientNum)->uiDC.realTime / 1000.0;
    R_CalcGameTimeVec(gameTime, timeVec);
    CodeConst_GameTimeParam = GetCodeConst_GameTimeParam();
    R_AddCmdSetCustomConstant(CodeConst_GameTimeParam, timeVec);
}

void __cdecl Script_SetItemColor(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    int j; // [esp+0h] [ebp-824h]
    itemDef_s *item2; // [esp+4h] [ebp-820h]
    int count; // [esp+8h] [ebp-81Ch]
    char itemname[1024]; // [esp+Ch] [ebp-818h] BYREF
    char name[1028]; // [esp+40Ch] [ebp-418h] BYREF
    float color[4]; // [esp+814h] [ebp-10h] BYREF

    if ( String_Parse(args, itemname, 1024) )
    {
        if ( String_Parse(args, name, 1024) )
        {
            count = Menu_ItemsMatchingGroup(item->parent, itemname);
            if ( Color_Parse(args, (float (*)[4])color) )
            {
                for ( j = 0; j < count; ++j )
                {
                    item2 = Menu_GetMatchingItemByNumber(item->parent, j, itemname);
                    if ( item2 )
                        Item_SetColor(dc->contextIndex, item2, name, color);
                }
            }
        }
    }
}

void __cdecl Item_SetColor(int contextIndex, itemDef_s *item, const char *name, float *color)
{
    float *out; // [esp+0h] [ebp-4h]

    out = 0;
    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 297, 0, "%s", "item != NULL") )
        __debugbreak();
    if ( I_stricmp(name, "backcolor") )
    {
        if ( I_stricmp(name, "forecolor") )
        {
            if ( !I_stricmp(name, "bordercolor") )
                out = item->window.borderColor;
        }
        else
        {
            out = item->window.foreColor;
            Window_AddDynamicFlags(contextIndex, &item->window, 0x10000);
        }
    }
    else
    {
        out = item->window.backColor;
    }
    if ( out )
    {
        *out = *color;
        out[1] = color[1];
        out[2] = color[2];
        out[3] = color[3];
    }
}

int __cdecl Menu_ItemsMatchingGroup(menuDef_t *menu, char *name)
{
    char *v2; // eax
    int wildcard; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    int count; // [esp+Ch] [ebp-4h]

    count = 0;
    wildcard = -1;
    v2 = strstr(name, "*");
    if ( v2 )
        wildcard = (unsigned int)v2 - (unsigned int)name;
    for ( i = 0; i < menu->itemCount; ++i )
    {
        if ( wildcard == -1 )
        {
            if ( menu->items[i]->window.name && !I_stricmp(menu->items[i]->window.name, name)
                || menu->items[i]->window.group && !I_stricmp(menu->items[i]->window.group, name) )
            {
                ++count;
            }
        }
        else if ( menu->items[i]->window.name && !I_strncmp(menu->items[i]->window.name, name, wildcard)
                     || menu->items[i]->window.group && !I_strncmp(menu->items[i]->window.group, name, wildcard) )
        {
            ++count;
        }
    }
    return count;
}

int __cdecl Menus_MenuIsInStack(UiContext *dc, menuDef_t *menu)
{
    int menuIndex; // [esp+0h] [ebp-4h]

    if ( !dc && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 1532, 0, "%s", "dc") )
        __debugbreak();
    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 1533, 0, "%s", "menu") )
        __debugbreak();
    for ( menuIndex = dc->openMenuCount - 1; menuIndex >= 0; --menuIndex )
    {
        if ( dc->menuStack[menuIndex].menu == menu )
            return 1;
    }
    return 0;
}

int __cdecl Menus_MenuInStackLocalClientNum(UiContext *dc, menuDef_t *menu)
{
    int menuIndex; // [esp+0h] [ebp-4h]

    if ( !dc && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 1548, 0, "%s", "dc") )
        __debugbreak();
    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 1549, 0, "%s", "menu") )
        __debugbreak();
    for ( menuIndex = dc->openMenuCount - 1; menuIndex >= 0; --menuIndex )
    {
        if ( dc->menuStack[menuIndex].menu == menu )
            return dc->menuStack[menuIndex].localClientNum;
    }
    return -1;
}

menuDef_t *__cdecl Menus_FindByName(const UiContext *dc, const char *p)
{
    menuDef_t *v3; // [esp+14h] [ebp-Ch]
    int i; // [esp+1Ch] [ebp-4h]

    PROF_SCOPED("Menus_FindByName");

    for ( i = 0; i < dc->menuCount; ++i )
    {
        if ( !I_stricmp(dc->Menus[i]->window.name, p) )
        {
            v3 = dc->Menus[i];
            return v3;
        }
    }

    return 0;
}

void __cdecl Menus_HideByName(const UiContext *dc, const char *menuName)
{
    menuDef_t *menu; // [esp+0h] [ebp-4h]

    if ( !dc && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 1580, 0, "%s", "dc") )
        __debugbreak();
    if ( !menuName && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 1581, 0, "%s", "menuName") )
        __debugbreak();
    menu = Menus_FindByName(dc, menuName);
    if ( menu )
        Window_RemoveDynamicFlags(dc->contextIndex, &menu->window, 4);
}

void __cdecl Menus_ShowByName(const UiContext *dc, const char *windowName)
{
    menuDef_t *menu; // [esp+0h] [ebp-4h]

    if ( !dc && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 1593, 0, "%s", "dc") )
        __debugbreak();
    if ( !windowName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 1594, 0, "%s", "windowName") )
    {
        __debugbreak();
    }
    menu = Menus_FindByName(dc, windowName);
    if ( menu )
        Window_AddDynamicFlags(dc->contextIndex, &menu->window, 4);
}

void __cdecl Menus_CloseByName(int localClientNum, UiContext *dc, const char *p)
{
    menuDef_t *menu; // [esp+0h] [ebp-4h]

    menu = Menus_FindByName(dc, p);
    if ( menu )
    {
        dc->enableSlideAndFadeEffectForMenu = 1;
        Menus_Close(localClientNum, dc, menu);
    }
}

void __cdecl Menus_Close(int localClientNum, UiContext *dc, menuDef_t *menu)
{
    int closeSlideSpeed; // [esp+0h] [ebp-2Ch]
    int closeFadingTime; // [esp+4h] [ebp-28h]
    bool noFocus; // [esp+1Ah] [ebp-12h]
    bool hadFocus; // [esp+1Bh] [ebp-11h]
    int menuNum; // [esp+1Ch] [ebp-10h]
    int menuNuma; // [esp+1Ch] [ebp-10h]
    int menuNumb; // [esp+1Ch] [ebp-10h]
    bool closeSlideAnimStarted; // [esp+23h] [ebp-9h]
    menuDef_t *menuDef; // [esp+24h] [ebp-8h]
    menuDef_t *menuDefa; // [esp+24h] [ebp-8h]
    bool closeFadingEffectStarted; // [esp+2Bh] [ebp-1h]
    int savedregs; // [esp+2Ch] [ebp+0h] BYREF

    if ( !dc && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 1750, 0, "%s", "dc") )
        __debugbreak();
    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 1751, 0, "%s", "menu") )
        __debugbreak();
    if ( !Menus_MenuIsInStack(dc, menu) )
    {
LABEL_56:
        Window_RemoveDynamicFlags(dc->contextIndex, &menu->window, 6);
        return;
    }
    Menus_CompleteItemAnimations(localClientNum, dc, menu);
    if ( !dc->enableSlideAndFadeEffectForMenu )
        goto LABEL_36;
    closeSlideAnimStarted = (Window_GetDynamicFlags(dc->contextIndex, &menu->window) & 0x40000) != 0;
    closeFadingEffectStarted = (Window_GetDynamicFlags(dc->contextIndex, &menu->window) & 0x2000) != 0;
    if ( menu->closeSlideSpeed > 0
        && menu->closeSlideDirection < 4u
        && !closeSlideAnimStarted
        && menu->slideTimeCounter != -3 )
    {
        Window_AddDynamicFlags(dc->contextIndex, &menu->window, 0x40000);
        menu->slideTimeCounter = Sys_Milliseconds();
        closeSlideAnimStarted = 1;
    }
    if ( menu->closeFadingTime > 0 && !closeFadingEffectStarted && menu->fadeTimeCounter != -3 )
    {
        Window_AddDynamicFlags(dc->contextIndex, &menu->window, 0x2000);
        menu->fadeTimeCounter = Sys_Milliseconds();
        closeFadingEffectStarted = 1;
    }
    if ( !closeSlideAnimStarted && !closeFadingEffectStarted
        || menu->slideTimeCounter == -3
        || menu->fadeTimeCounter == -3 )
    {
        goto LABEL_36;
    }
    if ( closeSlideAnimStarted && closeFadingEffectStarted )
    {
        if ( menu->closeSlideSpeed < menu->closeFadingTime )
            closeFadingTime = menu->closeFadingTime;
        else
            closeFadingTime = menu->closeSlideSpeed;
        menu->closeSlideSpeed = closeFadingTime;
        if ( menu->closeSlideSpeed < menu->closeFadingTime )
            closeSlideSpeed = menu->closeFadingTime;
        else
            closeSlideSpeed = menu->closeSlideSpeed;
        menu->closeFadingTime = closeSlideSpeed;
    }
    if ( (!closeSlideAnimStarted || (signed int)(Sys_Milliseconds() - menu->slideTimeCounter) >= menu->closeSlideSpeed)
        && (!closeFadingEffectStarted || (signed int)(Sys_Milliseconds() - menu->fadeTimeCounter) >= menu->closeFadingTime) )
    {
LABEL_36:
        Menu_ClearFocus(localClientNum, dc, menu);
        Menus_ClearFlagsOnClose(localClientNum, dc, menu);
        Menu_RunCloseScript(localClientNum, dc, menu);
        hadFocus = Window_HasFocus(dc->contextIndex, &menu->window);
        Menus_RemoveFromStack(localClientNum, dc, menu);
        if ( hadFocus )
        {
            Menu_RunLeaveFocusScript(localClientNum, dc, menu);
            for ( menuNum = dc->openMenuCount - 1; menuNum >= 0; --menuNum )
            {
                menuDef = dc->menuStack[menuNum].menu;
                if ( Window_IsVisible(dc->contextIndex, &menuDef->window) )
                {
                    Menu_GainFocusDueToClose(localClientNum, dc, menuDef);
                    break;
                }
            }
        }
        if ( dc->openMenuCount > 0 )
        {
            noFocus = 1;
            for ( menuNuma = 0; menuNuma < dc->openMenuCount; ++menuNuma )
            {
                menuDefa = dc->menuStack[menuNuma].menu;
                if ( Window_IsVisible(dc->contextIndex, &menuDefa->window)
                    && Window_HasFocus(dc->contextIndex, &menuDefa->window) )
                {
                    noFocus = 0;
                    break;
                }
            }
            if ( noFocus )
            {
                if ( !menu->window.name
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                                1845,
                                0,
                                "%s",
                                "menu->window.name") )
                {
                    __debugbreak();
                }
                Com_PrintWarning(13, "WARNING: No menu has focus after closing %s.\n Active menus: \n", menu->window.name);
                for ( menuNumb = 0; menuNumb < dc->openMenuCount; ++menuNumb )
                    Com_PrintWarning(13, "    %d:    %s\n", menuNumb, dc->menuStack[menuNumb].menu->window.name);
            }
        }
        goto LABEL_56;
    }
}

void    Menu_RunLeaveFocusScript(
                int localClientNum,
                UiContext *dc,
                menuDef_t *menu)
{
    itemDef_s item;

    iassert(menu);
    if (menu->onEvent)
    {
        item.parent = menu;
        item.onEvent = menu->onEvent;
        Item_RunEventScript(localClientNum, dc, &item, "leaveFocus");
    }
}

itemDef_s *__cdecl Menu_ClearFocus(int localClientNum, UiContext *dc, menuDef_t *menu)
{
    itemDef_s *ret; // [esp+0h] [ebp-Ch]
    int i; // [esp+8h] [ebp-4h]

    if ( !menu )
        return 0;
    ret = 0;
    for ( i = 0; i < menu->itemCount; ++i )
    {
        if ( Window_HasFocus(dc->contextIndex, &menu->items[i]->window) )
        {
            if ( ret && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 817, 0, "%s", "ret == NULL") )
                __debugbreak();
            ret = menu->items[i];
            Window_RemoveDynamicFlags(dc->contextIndex, &ret->window, 2);
            if ( Item_IsFocusDefType(menu->items[i]) )
            {
                Item_GetFocusItemDef(menu->items[i]);
                Item_RunEventScript(localClientNum, dc, menu->items[i], "leaveFocus");
            }
        }
    }
    return ret;
}

int __cdecl Menus_RemoveFromStack(int localClientNum, UiContext *dc, menuDef_t *pMenu)
{
    int v3; // eax
    int i; // [esp+4h] [ebp-4h]

    for ( i = dc->openMenuCount - 1; ; --i )
    {
        if ( i < 0 )
            return 0;
        if ( dc->menuStack[i].menu == pMenu )
            break;
    }
    --dc->openMenuCount;
    while ( i < dc->openMenuCount )
    {
        v3 = dc->menuStack[i + 1].localClientNum;
        dc->menuStack[i].menu = dc->menuStack[i + 1].menu;
        dc->menuStack[i++].localClientNum = v3;
    }
    return 1;
}

void __cdecl Menu_GainFocusDueToClose(int localClientNum, UiContext *dc, menuDef_t *menu)
{
    if ( !dc && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 1674, 0, "%s", "dc") )
        __debugbreak();
    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 1675, 0, "%s", "menu") )
        __debugbreak();
    if ( Window_HasFocus(dc->contextIndex, &menu->window)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    1677,
                    0,
                    "%s",
                    "!Window_HasFocus( dc->contextIndex, &menu->window )") )
    {
        __debugbreak();
    }
    Window_AddDynamicFlags(dc->contextIndex, &menu->window, 2);
    Menu_CallOnFocusDueToOpen(localClientNum, dc, menu);
}

void __cdecl Menu_CallOnFocusDueToOpen(int localClientNum, UiContext *dc, menuDef_t *menu)
{
    bool anyFound; // [esp+3h] [ebp-5h]
    int i; // [esp+4h] [ebp-4h]
    int savedregs; // [esp+8h] [ebp+0h] BYREF

    anyFound = 0;
    if ( (Window_GetDynamicFlags(dc->contextIndex, &menu->window) & 0xA0000) == 0 )
    {
        Menu_RunFocusScript(localClientNum, dc, menu);
        for ( i = 0; i < menu->itemCount; ++i )
        {
            if ( Window_HasFocus(dc->contextIndex, &menu->items[i]->window) && Item_IsFocusDefType(menu->items[i]) )
            {
                Item_RunEventScript(localClientNum, dc, menu->items[i], "onFocus");
                if ( anyFound
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 1662, 0, "%s", "!anyFound") )
                {
                    __debugbreak();
                }
                anyFound = 1;
            }
        }
    }
}

void Menu_RunFocusScript(int localClientNum, UiContext *dc, menuDef_t *menu)
{
    iassert(menu);

    if ( menu->onEvent )
    {
        itemDef_s item; // [esp+0h] [ebp-110h]

        item.parent = menu;
        item.onEvent = menu->onEvent;
        Item_RunEventScript(localClientNum, dc, &item, "onFocus");
    }
}

void Menu_RunCloseScript(int localClientNum, UiContext *dc, menuDef_t *menu)
{
    iassert(menu);

    if (Window_IsVisible(dc->contextIndex, &menu->window) && menu->onEvent)
    {
        itemDef_s item;

        item.parent = menu;
        item.onEvent = menu->onEvent;
        Item_RunEventScript(localClientNum, dc, &item, "onClose");
    }
}

void __cdecl Menus_CompleteItemAnimations(int localClientNum, UiContext *dc, menuDef_t *menu)
{
    itemDef_s *item; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < menu->itemCount; ++i )
    {
        item = menu->items[i];
        if ( item->animInfo )
        {
            if ( item->animInfo->animating )
            {
                item->animInfo->animDuration = 0;
                Item_UpdateAnimation(localClientNum, dc, item);
            }
        }
    }
}

void __cdecl Menus_ClearFlagsOnClose(int localClientNum, UiContext *dc, menuDef_t *menu)
{
    itemDef_s *nextItem; // [esp+0h] [ebp-Ch]
    focusItemDef_s *focusPtr; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    for ( i = 0; i < menu->itemCount; ++i )
    {
        nextItem = menu->items[i];
        if ( nextItem )
        {
            focusPtr = Item_GetFocusItemDef(nextItem);
            if ( (Window_GetDynamicFlags(dc->contextIndex, &nextItem->window) & 0x40) != 0 )
            {
                if ( focusPtr )
                    Item_RunScript(localClientNum, dc, nextItem, focusPtr->mouseExitText);
                Window_RemoveDynamicFlags(dc->contextIndex, &nextItem->window, 64);
            }
            if ( (Window_GetDynamicFlags(dc->contextIndex, &nextItem->window) & 1) != 0 )
            {
                Window_RemoveDynamicFlags(dc->contextIndex, &nextItem->window, 1);
                if ( focusPtr )
                    Item_RunScript(localClientNum, dc, nextItem, focusPtr->mouseExit);
            }
        }
    }
}

void __cdecl Menus_CloseImmediateByName(int localClientNum, UiContext *dc, const char *p)
{
    menuDef_t *menu; // [esp+0h] [ebp-4h]

    menu = Menus_FindByName(dc, p);
    if ( menu )
    {
        dc->enableSlideAndFadeEffectForMenu = 0;
        Menus_Close(localClientNum, dc, menu);
    }
}

void __cdecl Menus_RemoveMenu(XAssetHeader header)
{
    int i; // [esp+0h] [ebp-Ch]
    int localClientNum; // [esp+4h] [ebp-8h]
    uiInfo_s *uiInfo; // [esp+8h] [ebp-4h]

    for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
    {
        uiInfo = UI_GetInfo(localClientNum);
        for ( i = 0; i < uiInfo->uiDC.menuCount; ++i )
        {
            if ( header.menu == uiInfo->uiDC.Menus[i] )
            {
                memmove(
                    (unsigned __int8 *)&uiInfo->uiDC.Menus[i],
                    (unsigned __int8 *)&uiInfo->uiDC.Menus[i + 1],
                    4 * (uiInfo->uiDC.menuCount - i) - 4);
                --uiInfo->uiDC.menuCount;
                --i;
            }
        }
    }
}

char __cdecl Item_DoesEventExist(itemDef_s *item, const char *eventName)
{
    GenericEventHandler *eventRunner; // [esp+0h] [ebp-4h]

    if ( item && item->onEvent )
    {
        for ( eventRunner = item->onEvent; eventRunner; eventRunner = eventRunner->next )
        {
            if ( !I_stricmp(eventRunner->name, eventName) )
                return 1;
        }
    }
    return 0;
}

char __cdecl Menus_DoesEventExist(menuDef_t *menu, const char *eventName)
{
    GenericEventHandler *eventRunner; // [esp+0h] [ebp-4h]

    if ( menu && menu->onEvent )
    {
        for ( eventRunner = menu->onEvent; eventRunner; eventRunner = eventRunner->next )
        {
            if ( !I_stricmp(eventRunner->name, eventName) )
                return 1;
        }
    }
    return 0;
}

void __cdecl Menus_SetFocusToItem(int localClientNum, UiContext *dc, const char *menuName, const char *itemName)
{
    itemDef_s *focusItem; // [esp+8h] [ebp-Ch]
    menuDef_t *menu; // [esp+Ch] [ebp-8h]
    const rectDef_s *rect; // [esp+10h] [ebp-4h]

    menu = Menus_FindByName(dc, menuName);
    if ( menu )
    {
        focusItem = Menu_FindFirstFocusableItemByName(localClientNum, dc, menu, itemName);
        if ( focusItem )
        {
            rect = Window_GetRect(&focusItem->window);
            if ( !Item_SetFocus(localClientNum, dc, focusItem, rect->x, rect->y) )
                Com_DPrintf(
                    13,
                    "setFocus: error focusing widget '%s' (widget was found but could not accept focus)\n",
                    itemName);
        }
        else
        {
            Com_Printf(13, "setFocusToItem: could not find widget named '%s' in menu '%s'\n", itemName, menuName);
        }
    }
    else
    {
        Com_DPrintf(13, "setFocusToItem: could not find menu named '%s'\n", menuName);
    }
}

itemDef_s *__cdecl Menu_FindFirstFocusableItemByName(int localClientNum, UiContext *dc, menuDef_t *menu, const char *p)
{
    int lastItemWithName; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    lastItemWithName = -1;
    if ( !menu || !p )
        return 0;
    for ( i = 0; i < menu->itemCount; ++i )
    {
        if ( menu->items[i]->window.name && !I_stricmp(p, menu->items[i]->window.name) && menu->items[i] )
        {
            if ( Item_IsVisible(localClientNum, dc->contextIndex, menu->items[i])
                && (menu->items[i]->window.staticFlags & 0x100000) == 0 )
            {
                return menu->items[i];
            }
            lastItemWithName = i;
        }
    }
    if ( lastItemWithName <= -1 )
        return 0;
    else
        return menu->items[lastItemWithName];
}

void __cdecl Menus_CloseAll(int localClientNum, UiContext *dc)
{
    int i; // [esp+0h] [ebp-4h]

    dc->enableSlideAndFadeEffectForMenu = 0;
    for ( i = 0; i < dc->menuCount; ++i )
        Menus_Close(localClientNum, dc, dc->Menus[i]);
    dc->blurMenuStackIndex = -1;
}

void __cdecl Menus_CloseAllBehindMain(int localClientNum, UiContext *dc)
{
    menuDef_t *mainMenu; // [esp+0h] [ebp-4h]

    mainMenu = Menus_FindByName(dc, "main");
    if ( mainMenu && Menus_MenuIsInStack(dc, mainMenu) )
    {
        dc->enableSlideAndFadeEffectForMenu = 0;
        while ( dc->openMenuCount && dc->menuStack[0].menu != mainMenu )
            Menus_Close(localClientNum, dc, dc->menuStack[0].menu);
    }
}

void __cdecl UI_ClearErrors()
{
    Dvar_SetStringByName("error_menu_info", (char *)"");
    Dvar_SetStringByName("com_errorMessage", (char *)"");
    Dvar_SetBoolByName("com_isNotice", 0);
    Dvar_SetStringByName("com_errorTitle", (char *)"");
    Dvar_SetStringByName("ui_onCloseArgs", (char *)"");
}

void __cdecl Script_Show(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if ( String_Parse(args, name, 1024) )
        Menu_ShowItemByName(dc->contextIndex, item->parent, name, 1);
}

void __cdecl Menu_ShowItemByName(int contextIndex, menuDef_t *menu, char *p, int bShow)
{
    itemDef_s *item; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    int count; // [esp+8h] [ebp-4h]

    count = Menu_ItemsMatchingGroup(menu, p);
    for ( i = 0; i < count; ++i )
    {
        item = Menu_GetMatchingItemByNumber(menu, i, p);
        if ( item )
            Item_Show(contextIndex, item, bShow);
    }
}

void __cdecl Item_Show(int contextIndex, itemDef_s *item, int bShow)
{
    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 272, 0, "%s", "item") )
        __debugbreak();
    if ( bShow )
        Window_AddDynamicFlags(contextIndex, &item->window, 4);
    else
        Window_RemoveDynamicFlags(contextIndex, &item->window, 4);
}

void __cdecl Script_Hide(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if ( String_Parse(args, name, 1024) )
        Menu_ShowItemByName(dc->contextIndex, item->parent, name, 0);
}

void __cdecl Script_FadeIn(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if ( String_Parse(args, name, 1024) )
        Menu_FadeItemByName(dc->contextIndex, item->parent, name, 0);
}

void __cdecl Menu_FadeItemByName(int contextIndex, menuDef_t *menu, char *p, int fadeOut)
{
    itemDef_s *item; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    int count; // [esp+8h] [ebp-4h]

    count = Menu_ItemsMatchingGroup(menu, p);
    for ( i = 0; i < count; ++i )
    {
        item = Menu_GetMatchingItemByNumber(menu, i, p);
        if ( item )
            Item_Fade(contextIndex, item, fadeOut);
    }
}

void __cdecl Item_Fade(int contextIndex, itemDef_s *item, int fadeOut)
{
    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 281, 0, "%s", "item") )
        __debugbreak();
    if ( fadeOut )
    {
        Window_AddDynamicFlags(contextIndex, &item->window, 20);
        Window_RemoveDynamicFlags(contextIndex, &item->window, 32);
    }
    else
    {
        Window_AddDynamicFlags(contextIndex, &item->window, 36);
        Window_RemoveDynamicFlags(contextIndex, &item->window, 16);
    }
}

void __cdecl Script_FadeOut(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if ( String_Parse(args, name, 1024) )
        Menu_FadeItemByName(dc->contextIndex, item->parent, name, 1);
}

void __cdecl Script_ShowMenu(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if ( String_Parse(args, name, 1024) )
        Menus_ShowByName(dc, name);
}

void __cdecl Script_HideMenu(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if ( String_Parse(args, name, 1024) )
        Menus_HideByName(dc, name);
}

void __cdecl Script_Open(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    const char *String; // eax
    char searchValue[1024]; // [esp+0h] [ebp-808h] BYREF
    char name[1028]; // [esp+400h] [ebp-408h] BYREF

    if ( String_Parse(args, name, 1024) )
    {
        if ( I_stricmp(name, "dvarstring") )
        {
            Menus_OpenByName(localClientNum, dc, name);
        }
        else
        {
            do
            {
                do
                    String_Parse(args, name, 1024);
                while ( !I_stricmp(name, "(") );
            }
            while ( !I_stricmp(name, "\"") );
            String = Dvar_GetString(name);
            I_strncpyz(searchValue, String, 1024);
            while ( !I_stricmp(name, "(") || !I_stricmp(name, "\"") )
                String_Parse(args, name, 1024);
            Menus_OpenByName(localClientNum, dc, searchValue);
        }
    }
}

void __cdecl Script_OpenImmediate(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    const char *String; // eax
    char searchValue[1024]; // [esp+0h] [ebp-808h] BYREF
    char name[1028]; // [esp+400h] [ebp-408h] BYREF

    if ( String_Parse(args, name, 1024) )
    {
        if ( I_stricmp(name, "dvarstring") )
        {
            Menus_OpenImmediateByName(localClientNum, dc, name);
        }
        else
        {
            do
            {
                do
                    String_Parse(args, name, 1024);
                while ( !I_stricmp(name, "(") );
            }
            while ( !I_stricmp(name, "\"") );
            String = Dvar_GetString(name);
            I_strncpyz(searchValue, String, 1024);
            while ( !I_stricmp(name, "(") || !I_stricmp(name, "\"") )
                String_Parse(args, name, 1024);
            Menus_OpenImmediateByName(localClientNum, dc, searchValue);
        }
    }
}

void __cdecl Script_OpenForGameType(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    const char *String; // eax
    const char *v5; // eax
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if ( String_Parse(args, name, 1024) )
    {
        String = Dvar_GetString(item->dvar);
        v5 = va(name, String);
        Menus_OpenByName(localClientNum, dc, v5);
    }
}

void __cdecl Script_CloseForGameType(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    const char *String; // eax
    const char *v5; // eax
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if ( String_Parse(args, name, 1024) )
    {
        String = Dvar_GetString(item->dvar);
        v5 = va(name, String);
        Menus_CloseByName(localClientNum, dc, v5);
    }
}

void __cdecl Script_ActivateBlur(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    const char *menuName; // [esp+20h] [ebp-4h]

    menuName = item->parent->window.name;
    if ( !CL_AnyLocalClientStateActive() )
    {
        if ( menuName && strlen(menuName) )
        {
            if ( strlen(menuName) < 0x40 )
            {
                RemoveMenuFromBlurStack(localClientNum, dc, menuName);
                if ( dc->blurMenuStackIndex < 7 )
                {
                    I_strncpyz(dc->blurMenuStack[++dc->blurMenuStackIndex].menuName, menuName, 64);
                    UI_AnimateItem(localClientNum, dc, "UIBlur", "BlurItem", "FadeIn", 180);
                }
                else
                {
                    Com_PrintWarning(13, "WARNING: Tried to activate blur on too many menus simultaneously!\n");
                }
            }
            else
            {
                Com_PrintWarning(
                    13,
                    "WARNING: Menu name %s is longer than %d characters. Blur will not function for this menu.\n",
                    menuName,
                    63);
            }
        }
        else
        {
            Com_PrintWarning(
                13,
                "WARNING: Tried to activate blur on an unnamed menu. Blur will not function correctly for this menu.\n");
        }
    }
}

char __cdecl RemoveMenuFromBlurStack(int localClientNum, UiContext *dc, const char *menuName)
{
    int j; // [esp+20h] [ebp-8h]
    int i; // [esp+24h] [ebp-4h]

    if ( dc->blurMenuStackIndex < 0 )
        return 0;
    if ( menuName && strlen(menuName) )
    {
        if ( strlen(menuName) < 0x40 )
        {
            for ( i = 0; ; ++i )
            {
                if ( i > dc->blurMenuStackIndex )
                    return 0;
                if ( !I_strnicmp(dc->blurMenuStack[i].menuName, menuName, 64) )
                    break;
            }
            for ( j = i; j < dc->blurMenuStackIndex; ++j )
                I_strncpyz(dc->blurMenuStack[j].menuName, dc->blurMenuStack[j + 1].menuName, 64);
            if ( --dc->blurMenuStackIndex < 0 )
                UI_AnimateItem(localClientNum, dc, "UIBlur", "BlurItem", "FadeOut", 180);
            return 1;
        }
        else
        {
            Com_PrintWarning(
                13,
                "WARNING: Menu name %s is longer than %d characters. Blur will not function for this menu.\n",
                menuName,
                63);
            return 0;
        }
    }
    else
    {
        Com_PrintWarning(
            13,
            "WARNING: Tried to remove unnamed menu from blur stack. Blur will not function correctly for this menu.\n");
        return 0;
    }
}

void __cdecl Script_DeactivateBlur(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    if ( !CL_AnyLocalClientStateActive() )
        RemoveMenuFromBlurStack(localClientNum, dc, item->parent->window.name);
}

void __cdecl Script_SetDvarStringUsingTable(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    char dvarName[1024]; // [esp+0h] [ebp-808h] BYREF
    char arg[1024]; // [esp+400h] [ebp-408h] BYREF
    const char *dvarValue; // [esp+804h] [ebp-4h]

    do
        String_Parse(args, arg, 1024);
    while ( !I_stricmp(arg, "(") );
    I_strncpyz(dvarName, arg, 1024);
    String_Parse(args, arg, 1024);
    while ( !I_stricmp(arg, ",") || !I_stricmp(arg, "(") )
        String_Parse(args, arg, 1024);
    if ( I_stricmp(arg, "tablelookup") )
        dvarValue = arg;
    else
        dvarValue = Script_TableLookupParse(dc, args);
    Dvar_SetFromStringByName(dvarName, (char *)dvarValue);
    String_Parse(args, arg, 1024);
    while ( !I_stricmp(arg, ")") && String_Parse(args, arg, 1024) )
        ;
    Com_UngetToken();
}

const char *__cdecl Script_TableLookupParse(UiContext *dc, const char **args)
{
    const char *String; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    char searchValue[1024]; // [esp+0h] [ebp-818h] BYREF
    int seachValueTemp; // [esp+400h] [ebp-418h] BYREF
    const StringTable *tablePtr; // [esp+404h] [ebp-414h] BYREF
    int returnValueColumn; // [esp+408h] [ebp-410h] BYREF
    int comparisonColumn; // [esp+40Ch] [ebp-40Ch]
    char arg[1028]; // [esp+410h] [ebp-408h] BYREF

    do
    {
        do
            String_Parse(args, arg, 1024);
        while ( !I_stricmp(arg, "tablelookup") );
    }
    while ( !I_stricmp(arg, ",") || !I_stricmp(arg, "(") );
    if ( I_stricmp(arg, "dvarString") && I_stricmp(arg, "dvarstring") )
    {
        StringTable_GetAsset(arg, (XAssetHeader *)&tablePtr);
    }
    else
    {
        do
            String_Parse(args, arg, 1024);
        while ( !I_stricmp(arg, "(") );
        String = Dvar_GetString(arg);
        StringTable_GetAsset(String, (XAssetHeader *)&tablePtr);
        String_Parse(args, arg, 1024);
    }
    do
        String_Parse(args, arg, 1024);
    while ( !I_stricmp(arg, ",") );
    comparisonColumn = atoi(arg);
    String_Parse(args, arg, 1024);
    while ( !I_stricmp(arg, ",") )
        String_Parse(args, arg, 1024);
    if ( I_stricmp(arg, "dvarInt") && I_stricmp(arg, "dvarint") )
    {
        if ( I_stricmp(arg, "dvarString") && I_stricmp(arg, "dvarstring") )
        {
            if ( I_stricmp(arg, "tableLookup") && I_stricmp(arg, "tablelookup") )
            {
                if ( I_stricmp(arg, "int") )
                {
                    I_strncpyz(searchValue, arg, 1024);
                }
                else
                {
                    do
                        String_Parse(args, arg, 1024);
                    while ( !I_stricmp(arg, "(") );
                    if ( !I_stricmp(arg, "tablelookup") )
                    {
                        v5 = Script_TableLookupParse(dc, args);
                        I_strncpyz(searchValue, v5, 1024);
                    }
                    String_Parse(args, arg, 1024);
                    Script_OpParse(&seachValueTemp, args);
                    _itoa(seachValueTemp, searchValue, 0xAu);
                }
            }
            else
            {
                v4 = Script_TableLookupParse(dc, args);
                I_strncpyz(searchValue, v4, 1024);
                Script_OpParse(&seachValueTemp, args);
                _itoa(seachValueTemp, searchValue, 0xAu);
            }
        }
        else
        {
            do
                String_Parse(args, arg, 1024);
            while ( !I_stricmp(arg, "(") );
            v3 = Dvar_GetString(arg);
            I_strncpyz(searchValue, v3, 1024);
            String_Parse(args, arg, 1024);
        }
    }
    else
    {
        do
            String_Parse(args, arg, 1024);
        while ( !I_stricmp(arg, "(") );
        seachValueTemp = Dvar_GetInt(arg);
        String_Parse(args, arg, 1024);
        Script_OpParse(&seachValueTemp, args);
        _itoa(seachValueTemp, searchValue, 0xAu);
    }
    do
        String_Parse(args, arg, 1024);
    while ( !I_stricmp(arg, ",") );
    if ( I_stricmp(arg, "int") )
        returnValueColumn = atoi(arg);
    else
        returnValueColumn = Script_IntParse(dc, args);
    Script_OpParse(&returnValueColumn, args);
    return StringTable_Lookup(tablePtr, comparisonColumn, searchValue, returnValueColumn);
}

void __cdecl Script_OpParse(int *operand, const char **args)
{
    char op[1028]; // [esp+0h] [ebp-810h] BYREF
    int delta; // [esp+404h] [ebp-40Ch]
    char arg[1028]; // [esp+408h] [ebp-408h] BYREF

    String_Parse(args, arg, 1024);
    I_strncpyz(op, arg, 1024);
    if ( I_stricmp(op, "+") )
    {
        if ( I_stricmp(op, "-") )
        {
            if ( I_stricmp(op, ")") )
                Com_PrintError(13, "Script_OpParse cannot handle a '%s'.\n", op);
        }
        else
        {
            String_Parse(args, arg, 1024);
            delta = atoi(arg);
            *operand -= delta;
            String_Parse(args, arg, 1024);
        }
    }
    else
    {
        String_Parse(args, arg, 1024);
        delta = atoi(arg);
        *operand += delta;
        String_Parse(args, arg, 1024);
    }
}

int __cdecl Script_IntParse(UiContext *dc, const char **args)
{
    int statValue; // [esp+0h] [ebp-40Ch]
    char arg[1028]; // [esp+4h] [ebp-408h] BYREF

    do
        String_Parse(args, arg, 1024);
    while ( !I_stricmp(arg, "(") );
    statValue = atoi(arg);
    String_Parse(args, arg, 1024);
    return statValue;
}

void __cdecl Script_Close(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+14h] [ebp-408h] BYREF

    if ( String_Parse(args, name, 1024) )
    {
        if ( !strcmp(name, "self") )
        {
            dc->enableSlideAndFadeEffectForMenu = 1;
            Menus_Close(localClientNum, dc, item->parent);
        }
        else
        {
            Menus_CloseByName(localClientNum, dc, name);
        }
    }
}

void __cdecl Script_CloseImmediate(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+14h] [ebp-408h] BYREF

    if ( String_Parse(args, name, 1024) )
    {
        if ( !strcmp(name, "self") )
        {
            dc->enableSlideAndFadeEffectForMenu = 0;
            Menus_Close(localClientNum, dc, item->parent);
        }
        else
        {
            Menus_CloseImmediateByName(localClientNum, dc, name);
        }
    }
}

void __cdecl Script_InGameOpen(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if ( String_Parse(args, name, 1024) )
    {
        if ( CL_IsLocalClientInGame(localClientNum) )
            Menus_OpenByName(localClientNum, dc, name);
    }
}

void __cdecl Script_InGameClose(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if ( String_Parse(args, name, 1024) )
    {
        if ( CL_IsLocalClientInGame(localClientNum) )
            Menus_CloseByName(localClientNum, dc, name);
    }
}

void __cdecl Script_FocusFirstInMenu(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    const char *name; // [esp+0h] [ebp-Ch]
    itemDef_s *focusItem; // [esp+8h] [ebp-4h]

    focusItem = Menu_FocusFirstSelectableItem(localClientNum, dc, item->parent);
    if ( focusItem )
    {
        if ( Item_IsTextField(focusItem) )
            Item_TextField_BeginEdit(localClientNum, dc->contextIndex, focusItem);
    }
    else if ( item->parent->window.name )
    {
        Com_Printf(13, "focusFirst: no itemDefs in %s were selectable\n", item->parent->window.name);
    }
    else
    {
        if ( item->window.name )
            name = item->window.name;
        else
            name = "itemDef's unnamed menu";
        Com_Printf(13, "focusFirst: no itemDefs in %s were selectable\n", name);
    }
}

itemDef_s *__cdecl Menu_FocusFirstSelectableItem(int localClientNum, UiContext *dc, menuDef_t *menu)
{
    unsigned int cursor; // [esp+8h] [ebp-4h]
    int cursora; // [esp+8h] [ebp-4h]

    if ( Menu_HandleMouseMove(localClientNum, dc, menu) )
    {
        cursor = Menu_GetCursorItem(dc->contextIndex, menu);
        if ( cursor >= menu->itemCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                        2612,
                        0,
                        "cursor doesn't index menu->itemCount\n\t%i not in [0, %i)",
                        cursor,
                        menu->itemCount) )
        {
            __debugbreak();
        }
        return menu->items[cursor];
    }
    else
    {
        cursora = 0;
        Menu_SetCursorItem(dc->contextIndex, menu, 0);
        while ( cursora < menu->itemCount )
        {
            if ( Item_SetFocus(localClientNum, dc, menu->items[cursora], dc->cursor.x, dc->cursor.y) )
                return menu->items[cursora];
            Menu_SetCursorItem(dc->contextIndex, menu, ++cursora);
        }
        Menu_SetCursorItem(dc->contextIndex, menu, 0);
        return 0;
    }
}

int __cdecl Menu_GetCursorItem(int contextIndex, menuDef_t *menu)
{
    if ( contextIndex
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\ui\\../ui/ui_utils.h",
                    49,
                    0,
                    "contextIndex doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    contextIndex,
                    1) )
    {
        __debugbreak();
    }
    return menu->cursorItem[contextIndex];
}

void __cdecl Script_SetFocus(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+8h] [ebp-410h] BYREF
    itemDef_s *focusItem; // [esp+410h] [ebp-8h]
    const rectDef_s *rect; // [esp+414h] [ebp-4h]

    if ( String_Parse(args, name, 1024) )
    {
        focusItem = Menu_FindFirstFocusableItemByName(localClientNum, dc, item->parent, name);
        if ( focusItem )
        {
            rect = Window_GetRect(&focusItem->window);
            if ( Item_SetFocus(localClientNum, dc, focusItem, rect->x, rect->y) )
            {
                if ( Item_IsTextField(focusItem) )
                    Item_TextField_BeginEdit(localClientNum, dc->contextIndex, focusItem);
            }
            else
            {
                Com_DPrintf(13, "setFocus: error focusing widget '%s' (widget was found but could not accept focus)\n", name);
            }
        }
        else
        {
            Com_Printf(13, "setFocus: could not find widget named '%s'\n", name);
        }
    }
}

void __cdecl Script_SetFocusByDvar(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    itemDef_s *focusItem; // [esp+8h] [ebp-418h]
    menuDef_t *parent; // [esp+Ch] [ebp-414h]
    char dvarName[1028]; // [esp+10h] [ebp-410h] BYREF
    int i; // [esp+418h] [ebp-8h]
    const rectDef_s *rect; // [esp+41Ch] [ebp-4h]

    if ( String_Parse(args, dvarName, 1024) )
    {
        parent = item->parent;
        if ( !parent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 2688, 0, "%s", "parent") )
            __debugbreak();
        for ( i = 0; i < parent->itemCount; ++i )
        {
            focusItem = parent->items[i];
            if ( focusItem->typeData.textDef && (focusItem->dvarFlags & 0x10) != 0 )
            {
                if ( !focusItem->dvarTest )
                    Com_Error(ERR_DROP, "Script_SetFocusByDvar: Item's dvarTest field is empty.");
                if ( !I_stricmp(focusItem->dvarTest, dvarName) )
                {
                    if ( Item_EnableShowViaDvar(focusItem, 16) )
                    {
                        rect = Window_GetRect(&focusItem->window);
                        if ( Item_SetFocus(localClientNum, dc, focusItem, rect->x, rect->y) )
                            break;
                    }
                }
            }
        }
    }
}

void __cdecl Script_ChangeState(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    animParamsDef_t *newState; // [esp+0h] [ebp-810h]
    int milliSeconds; // [esp+4h] [ebp-80Ch]
    char newStateName[1024]; // [esp+8h] [ebp-808h] BYREF
    char milliSecondsString[1028]; // [esp+408h] [ebp-408h] BYREF

    if ( String_Parse(args, newStateName, 1024) && String_Parse(args, milliSecondsString, 1024) )
    {
        milliSeconds = atoi(milliSecondsString);
        newState = Item_FindStateByName(item, newStateName);
        if ( newState )
            Item_AnimateToState(localClientNum, dc, item, newState, milliSeconds, 0);
    }
}

void __cdecl Script_SetDvar(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    const char *String; // eax
    const char *v5; // eax
    int Int; // eax
    char secondDvarVal[1024]; // [esp+8h] [ebp-C08h] BYREF
    char val[1024]; // [esp+408h] [ebp-808h] BYREF
    char dvarName[1028]; // [esp+808h] [ebp-408h] BYREF

    if ( String_Parse(args, dvarName, 1024) && String_Parse(args, val, 1024) )
    {
        if ( I_stricmp(val, "dvarString") )
        {
            if ( I_stricmp(val, "dvarInt") )
            {
                Dvar_SetFromStringByName(dvarName, val);
            }
            else
            {
                do
                    String_Parse(args, val, 1024);
                while ( !I_stricmp(val, "(") );
                Int = Dvar_GetInt(val);
                Dvar_SetIntByName(dvarName, Int);
                String_Parse(args, val, 1024);
            }
        }
        else
        {
            String_Parse(args, val, 1024);
            String_Parse(args, val, 1024);
            String = Dvar_GetString(val);
            I_strncpyz(secondDvarVal, String, 1024);
            String_Parse(args, val, 1024);
            if ( !I_stricmp(val, "+") )
            {
                String_Parse(args, val, 1024);
                v5 = Dvar_GetString(val);
                I_strncat(secondDvarVal, 1024, v5);
                String_Parse(args, val, 1024);
            }
            Dvar_SetFromStringByName(dvarName, secondDvarVal);
        }
    }
}

void __cdecl Script_ResetDvar(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    dvar_s *dvar; // [esp+0h] [ebp-40Ch]
    char dvarName[1028]; // [esp+4h] [ebp-408h] BYREF

    if ( String_Parse(args, dvarName, 1024) )
    {
        dvar = (dvar_s *)Dvar_FindVar(dvarName);
        if ( dvar )
            Dvar_Reset(dvar, DVAR_SOURCE_INTERNAL);
    }
}

void __cdecl Script_ExecKeyHandler(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    int v4; // eax
    char val[1028]; // [esp+0h] [ebp-408h] BYREF
    int savedregs; // [esp+408h] [ebp+0h] BYREF

    if ( String_Parse(args, val, 1024) )
    {
        v4 = atoi(val);
        Menu_CheckOnKey(localClientNum, dc, item->parent, v4);
    }
}

void __cdecl Script_ExecKeyPress(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    int v4; // eax
    int v5; // eax
    char val[1028]; // [esp+0h] [ebp-408h] BYREF
    int savedregs; // [esp+408h] [ebp+0h] BYREF

    if ( String_Parse(args, val, 1024) )
    {
        v4 = atoi(val);
        Menu_HandleKey(localClientNum, dc, item->parent, v4, 1);
        v5 = atoi(val);
        Menu_HandleKey(localClientNum, dc, item->parent, v5, 0);
    }
}

void __cdecl Script_Exec(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    int controllerIndex; // [esp+0h] [ebp-4h]

    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( controllerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    2824,
                    0,
                    "%s",
                    "controllerIndex != INVALID_CONTROLLER_PORT") )
    {
        __debugbreak();
    }
    Script_ExecHandler(localClientNum, controllerIndex, item, (char**)args, Script_AddTextWrapper);
}

void __cdecl Script_ExecHandler(
                int localClientNum,
                int controllerIndex,
                itemDef_s *item,
                char **args,
                void (__cdecl *textCallback)(int, int, itemDef_s *, const char *))
{
    char val[1028]; // [esp+0h] [ebp-408h] BYREF

    if ( Command_Parse(args, val, 1023) )
    {
        I_strncat(val, 1024, "\n");
        textCallback(localClientNum, controllerIndex, item, val);
    }
}

void __cdecl Script_AddTextWrapper(int localClientNum, int controllerIndex, itemDef_s *__formal, const char *text)
{
    Cbuf_AddText(localClientNum, text);
}

void __cdecl Script_ExecNow(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    int controllerIndex; // [esp+0h] [ebp-4h]

    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( controllerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    2833,
                    0,
                    "%s",
                    "controllerIndex != INVALID_CONTROLLER_PORT") )
    {
        __debugbreak();
    }
    Script_ExecHandler(
        localClientNum,
        controllerIndex,
        item,
        (char**)args,
        (void (__cdecl *)(int, int, itemDef_s *, const char *))Cbuf_ExecuteBufferUI);
}

void __cdecl Script_ExecDvar(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    const char *v4; // eax
    int controllerIndex; // [esp+0h] [ebp-80Ch]
    char dvarName[1024]; // [esp+4h] [ebp-808h] BYREF
    char command[1024]; // [esp+404h] [ebp-408h] BYREF
    const char *dvarValue; // [esp+808h] [ebp-4h]

    if ( String_Parse(args, dvarName, 1024) )
    {
        dvarValue = Dvar_GetVariantString(dvarName);
        controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        if ( controllerIndex == -1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                        2848,
                        0,
                        "%s",
                        "controllerIndex != INVALID_CONTROLLER_PORT") )
        {
            __debugbreak();
        }
        v4 = va("%s\n", dvarValue);
        I_strncpyz(command, v4, 1024);
        Script_AddTextWrapper(localClientNum, controllerIndex, item, command);
    }
}

void __cdecl Script_ExecOnDvarStringValue(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    Script_ConditionalExecHandler(localClientNum, dc, item, args, Script_ExecIfStringsEqual, Script_AddTextWrapper);
}

void __cdecl Script_ConditionalExecHandler(
                int localClientNum,
                UiContext *dc,
                itemDef_s *item,
                char **args,
                bool (__cdecl *shouldExec)(const char *, const char *),
                void (__cdecl *textCallback)(int, int, itemDef_s *, const char *))
{
    int controllerIndex; // [esp+0h] [ebp-C0Ch]
    char testValue[1024]; // [esp+4h] [ebp-C08h] BYREF
    char dvarName[1024]; // [esp+404h] [ebp-808h] BYREF
    char command[1024]; // [esp+804h] [ebp-408h] BYREF
    const char *dvarValue; // [esp+C08h] [ebp-4h]

    if ( String_Parse((const char **)args, dvarName, 1024) )
    {
        if ( String_Parse((const char **)args, testValue, 1024) )
        {
            if ( Command_Parse(args, command, 1023) )
            {
                dvarValue = Dvar_GetVariantString(dvarName);
                if ( shouldExec(dvarValue, testValue) )
                {
                    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
                    if ( controllerIndex == -1
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                                    2869,
                                    0,
                                    "%s",
                                    "controllerIndex != INVALID_CONTROLLER_PORT") )
                    {
                        __debugbreak();
                    }
                    I_strncat(command, 1024, "\n");
                    textCallback(localClientNum, controllerIndex, item, command);
                }
            }
        }
    }
}

bool __cdecl Script_ExecIfStringsEqual(const char *dvarValue, const char *testValue)
{
    return I_stricmp(dvarValue, testValue) == 0;
}

void __cdecl Script_ExecOnDvarIntValue(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    Script_ConditionalExecHandler(localClientNum, dc, item, args, Script_ExecIfIntsEqual, Script_AddTextWrapper);
}

bool __cdecl Script_ExecIfIntsEqual(const char *dvarValue, const char *testValue)
{
    int v2; // esi

    v2 = atoi(dvarValue);
    return v2 == atoi(testValue);
}

void __cdecl Script_ExecOnDvarFloatValue(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    Script_ConditionalExecHandler(localClientNum, dc, item, args, Script_ExecIfFloatsEqual, Script_AddTextWrapper);
}

bool __cdecl Script_ExecIfFloatsEqual(const char *dvarValue, const char *testValue)
{
    long double v3; // [esp+4h] [ebp-Ch]
    float v4; // [esp+Ch] [ebp-4h]

    v3 = atof(testValue);
    v4 = atof(dvarValue) - v3;
    return fabs(v4) < 0.0000099999997;
}

void __cdecl Script_ExecNowOnDvarStringValue(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    Script_ConditionalExecHandler(
        localClientNum,
        dc,
        item,
        args,
        Script_ExecIfStringsEqual,
        (void (__cdecl *)(int, int, itemDef_s *, const char *))Cbuf_ExecuteBufferUI);
}

void __cdecl Script_ExecNowOnDvarIntValue(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    Script_ConditionalExecHandler(
        localClientNum,
        dc,
        item,
        args,
        Script_ExecIfIntsEqual,
        (void (__cdecl *)(int, int, itemDef_s *, const char *))Cbuf_ExecuteBufferUI);
}

void __cdecl Script_ExecNowOnDvarFloatValue(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    Script_ConditionalExecHandler(
        localClientNum,
        dc,
        item,
        args,
        Script_ExecIfFloatsEqual,
        (void (__cdecl *)(int, int, itemDef_s *, const char *))Cbuf_ExecuteBufferUI);
}

void __cdecl Script_RespondOnDvarStringValue(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    Script_ConditionalResponseHandler(localClientNum, dc, item, args, Script_ExecIfStringsEqual);
}

void __cdecl Script_ConditionalResponseHandler(
                int localClientNum,
                UiContext *dc,
                itemDef_s *item,
                const char **args,
                bool (__cdecl *shouldRespond)(const char *, const char *))
{
    const char *v5; // eax
    int iIndex; // [esp+0h] [ebp-C10h]
    char *pszName; // [esp+4h] [ebp-C0Ch]
    char testValue[1024]; // [esp+8h] [ebp-C08h] BYREF
    char dvarName[1024]; // [esp+408h] [ebp-808h] BYREF
    char command[1024]; // [esp+808h] [ebp-408h] BYREF
    const char *dvarValue; // [esp+C0Ch] [ebp-4h]

    if ( String_Parse(args, dvarName, 1024) )
    {
        if ( String_Parse(args, testValue, 1024) )
        {
            if ( String_Parse(args, command, 1024) )
            {
                dvarValue = Dvar_GetVariantString(dvarName);
                if ( shouldRespond(dvarValue, testValue) )
                {
                    for ( iIndex = 0; iIndex < 32; ++iIndex )
                    {
                        pszName = CL_GetConfigString(iIndex + 2548);
                        if ( *pszName )
                        {
                            if ( !I_stricmp(item->parent->window.name, pszName) )
                                break;
                        }
                    }
                    if ( iIndex == 32 )
                        iIndex = -1;
                    v5 = va("cmd mr %i %i %s\n", cls.serverId, iIndex, command);
                    Cbuf_AddText(localClientNum, v5);
                }
            }
        }
    }
}

void __cdecl Script_RespondOnDvarIntValue(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    Script_ConditionalResponseHandler(localClientNum, dc, item, args, Script_ExecIfIntsEqual);
}

void __cdecl Script_RespondOnDvarFloatValue(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    Script_ConditionalResponseHandler(localClientNum, dc, item, args, Script_ExecIfFloatsEqual);
}

void __cdecl Script_SetLocalVarBool(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    int v4; // eax
    UILocalVarContext *var; // [esp+0h] [ebp-10Ch]
    char string[260]; // [esp+4h] [ebp-108h] BYREF

    var = Script_ParseLocalVar(dc, args);
    if ( String_Parse(args, string, 256) )
    {
        if ( var )
        {
            v4 = atoi(string);
            UILocalVar_SetBool(var->table, v4 != 0);
        }
    }
}

UILocalVarContext *__cdecl Script_ParseLocalVar(UiContext *dc, const char **args)
{
    char varName[260]; // [esp+0h] [ebp-108h] BYREF

    if ( String_Parse(args, varName, 256) )
        return UILocalVar_FindOrCreate(&dc->localVars, varName);
    else
        return 0;
}

void __cdecl Script_SetLocalVarInt(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    int v4; // eax
    UILocalVarContext *var; // [esp+0h] [ebp-10Ch]
    char string[260]; // [esp+4h] [ebp-108h] BYREF

    var = Script_ParseLocalVar(dc, args);
    if ( String_Parse(args, string, 256) )
    {
        if ( var )
        {
            v4 = atoi(string);
            UILocalVar_SetInt(var->table, v4);
        }
    }
}

void __cdecl Script_SetLocalVarFloat(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    float f; // [esp+4h] [ebp-110h]
    UILocalVarContext *var; // [esp+8h] [ebp-10Ch]
    char string[260]; // [esp+Ch] [ebp-108h] BYREF

    var = Script_ParseLocalVar(dc, args);
    if ( String_Parse(args, string, 256) )
    {
        if ( var )
        {
            f = atof(string);
            UILocalVar_SetFloat(var->table, f);
        }
    }
}

void __cdecl Script_SetLocalVarString(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    UILocalVarContext *var; // [esp+0h] [ebp-10Ch]
    char string[260]; // [esp+4h] [ebp-108h] BYREF

    var = Script_ParseLocalVar(dc, args);
    if ( String_Parse(args, string, 256) )
    {
        if ( var )
            UILocalVar_SetString(var->table, string);
    }
}

void __cdecl Script_FeederTop(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    listBoxDef_s *listPtr; // [esp+8h] [ebp-4h]

    listPtr = Item_GetListBoxDef(item);
    if ( listPtr )
    {
        if ( listPtr->notselectable )
        {
            listPtr->startPos[dc->contextIndex] = 0;
        }
        else
        {
            listPtr->cursorPos[dc->contextIndex] = 0;
            listPtr->startPos[dc->contextIndex] = 0;
            UI_FeederSelection(localClientNum, dc->contextIndex, listPtr->special, 0);
        }
    }
}

void __cdecl Script_FeederBottom(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    int v3; // [esp+8h] [ebp-18h]
    const char *v4; // [esp+Ch] [ebp-14h]
    const char *max; // [esp+10h] [ebp-10h]
    listBoxDef_s *listPtr; // [esp+14h] [ebp-Ch]
    int viewmax; // [esp+18h] [ebp-8h]

    listPtr = Item_GetListBoxDef(item);
    if ( listPtr )
    {
        if ( listPtr->notselectable )
        {
            listPtr->startPos[dc->contextIndex] = (int)Item_ListBox_MaxScroll(localClientNum, dc->contextIndex, item);
        }
        else
        {
            max = Item_ListBox_MaxScroll(localClientNum, dc->contextIndex, item);
            viewmax = Item_ListBox_Viewmax(localClientNum, dc->contextIndex, item);
            v4 = (const char *)UI_FeederCount(localClientNum, dc->contextIndex, listPtr->special, listPtr) - 1;
            if ( (int)v4 > 0 )
                v3 = (int)v4;
            else
                v3 = 0;
            Item_ListBox_SetCursorPos(localClientNum, dc->contextIndex, item, (int)max, viewmax, v3, 1);
        }
    }
}

void __cdecl Script_SetDvarFromLocString(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    const char *VariantString; // eax
    char string; // [esp+0h] [ebp-1C08h] BYREF
    char pszInputBuffer[4095]; // [esp+1h] [ebp-1C07h] BYREF
    char dest[1024]; // [esp+1000h] [ebp-C08h] BYREF
    char out[1024]; // [esp+1400h] [ebp-808h] BYREF
    char dvarName[1024]; // [esp+1800h] [ebp-408h] BYREF
    char *src; // [esp+1C04h] [ebp-4h]

    do
        String_Parse((const char **)args, out, 1024);
    while ( !I_stricmp(out, "(") );
    if ( I_stricmp(out, "dvarString") && I_stricmp(out, "dvarstring") )
    {
        I_strncpyz(dvarName, out, 1024);
    }
    else
    {
        do
            String_Parse((const char **)args, out, 1024);
        while ( !I_stricmp(out, "(") );
        I_strncpyz(dest, out, 1024);
        VariantString = Dvar_GetVariantString(dest);
        I_strncpyz(dvarName, VariantString, 1024);
    }
    do
    {
        do
            String_Parse((const char **)args, out, 1024);
        while ( !I_stricmp(out, ",") );
    }
    while ( !I_stricmp(out, ")") );
    I_strncpyz(&string, out, 4096);
    if ( string == 64 )
    {
        src = SEH_LocalizeTextMessage(pszInputBuffer, "dvar string", LOCMSG_NOERR);
        if ( src )
        {
            if ( *src )
                I_strncpyz(&string, src, 4096);
        }
    }
    Dvar_SetCommand(dvarName, &string);
    String_Parse((const char **)args, out, 1024);
    while ( !I_stricmp(out, ")") )
        String_Parse((const char **)args, out, 1024);
    Com_UngetToken();
}

void __cdecl Script_Play(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    char val[1024];

    if (!IsDedicatedServer())
    {
        if (String_Parse(args, val, sizeof(val)))
        {
            UI_PlaySound(dc->contextIndex, val);
        }
    }
}

void __cdecl Script_ScriptMenuResponse(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    const char *v4; // eax
    unsigned int stringSize; // [esp+0h] [ebp-45Ch]
    char windowName[68]; // [esp+4h] [ebp-458h] BYREF
    int iIndex; // [esp+48h] [ebp-414h]
    const char *pszName; // [esp+4Ch] [ebp-410h]
    const char *splitscreenSuffix; // [esp+50h] [ebp-40Ch]
    char val[1028]; // [esp+54h] [ebp-408h] BYREF

    if ( UI_AllowScriptMenuResponse(localClientNum) && String_Parse(args, val, 1024) )
    {
        memset(windowName, 0, 64);
        splitscreenSuffix = I_stristr(item->parent->window.name, "_splitscreen");
        if ( splitscreenSuffix )
        {
            stringSize = splitscreenSuffix - item->parent->window.name + 1;
            if ( stringSize > 0x40 )
                stringSize = 64;
            I_strncpyz(windowName, item->parent->window.name, stringSize);
        }
        else
        {
            I_strncpyz(windowName, item->parent->window.name, 64);
        }
        for ( iIndex = 0; iIndex < 32; ++iIndex )
        {
            pszName = CL_GetConfigString(iIndex + 2548);
            if ( *pszName )
            {
                if ( !I_stricmp(windowName, pszName) )
                    break;
            }
        }
        if ( iIndex == 32 )
            iIndex = -1;
        v4 = va("cmd mr %i %i %s\n", cls.serverId, iIndex, val);
        Cbuf_AddText(localClientNum, v4);
    }
}

void __cdecl Script_SetUIVisibilityBit(int localClientNum, UiContext *dc, itemDef_s *item, const char **args)
{
    char valueString[132]; // [esp+0h] [ebp-110h] BYREF
    int bitShift; // [esp+84h] [ebp-8Ch]
    char bitShiftString[128]; // [esp+88h] [ebp-88h] BYREF
    int value; // [esp+10Ch] [ebp-4h]

    if ( String_Parse(args, bitShiftString, 4) )
    {
        if ( String_Parse(args, valueString, 128) )
        {
            bitShift = atoi(bitShiftString);
            value = atoi(valueString);
            UI_SetLocalUIVisbilityBit(localClientNum, bitShift, value);
        }
    }
}

bool __cdecl Item_IsOutcomeAccepted(bool outcome, bool fireOnTrue)
{
    if ( fireOnTrue )
        return outcome;
    else
        return !outcome;
}

char __cdecl Item_ArePrereqsSatisfied(GenericEventScript *eventScript, nestingStack_t *stack)
{
    int i; // [esp+0h] [ebp-Ch]
    bool isPrereqSatisfied; // [esp+7h] [ebp-5h]
    ScriptCondition *currentPrereq; // [esp+8h] [ebp-4h]

    for ( currentPrereq = eventScript->prerequisites; currentPrereq; currentPrereq = currentPrereq->next )
    {
        isPrereqSatisfied = 0;
        for ( i = 0; i < stack->top; ++i )
        {
            if ( currentPrereq->constructID == stack->members[i].constructID
                && currentPrereq->blockID == stack->members[i].blockID
                && Item_IsOutcomeAccepted(stack->members[i].outcome, currentPrereq->fireOnTrue) )
            {
                isPrereqSatisfied = 1;
            }
        }
        if ( !isPrereqSatisfied )
            return 0;
    }
    return 1;
}

void __cdecl Item_MarkProcessed(GenericEventScript *eventScript, nestingStack_t *stack, bool isConditionSatisfied)
{
    if ( stack->top < 64 )
    {
        stack->members[stack->top].blockID = eventScript->blockID;
        stack->members[stack->top].constructID = eventScript->constructID;
        stack->members[stack->top++].outcome = isConditionSatisfied;
    }
    else
    {
        Com_PrintError(16, "If..Else construct stack overflow.\n");
    }
}

char __cdecl Item_IsBlockProcessed(GenericEventScript *eventScript, nestingStack_t *stack, bool *result)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < stack->top; ++i )
    {
        if ( stack->members[i].blockID == eventScript->blockID && stack->members[i].constructID == eventScript->constructID )
        {
            *result = stack->members[i].outcome;
            return 1;
        }
    }
    return 0;
}

void __cdecl Item_RunSingleEventScript(
                int localClientNum,
                UiContext *dc,
                itemDef_s *item,
                GenericEventScript *eventScript)
{
    bool result; // [esp+3h] [ebp-311h] BYREF
    nestingStack_t stack; // [esp+4h] [ebp-310h] BYREF
    bool isConditionSatisfied; // [esp+313h] [ebp-1h]

    stack.top = 0;
    while ( eventScript )
    {
        isConditionSatisfied = 0;
        if ( !eventScript->blockID && !eventScript->constructID )
        {
            Item_RunScript(localClientNum, dc, item, eventScript->action);
            goto LABEL_15;
        }
        if ( Item_ArePrereqsSatisfied(eventScript, &stack) )
        {
            if ( eventScript->type == 3 )
                goto LABEL_13;
            if ( Item_IsBlockProcessed(eventScript, &stack, &result) )
            {
                if ( result )
                {
                    Item_RunScript(localClientNum, dc, item, eventScript->action);
                    isConditionSatisfied = 1;
                }
                goto LABEL_14;
            }
            if ( Item_IsConditionSatisfied(localClientNum, dc, item, eventScript) )
            {
LABEL_13:
                Item_RunScript(localClientNum, dc, item, eventScript->action);
                isConditionSatisfied = 1;
            }
        }
LABEL_14:
        Item_MarkProcessed(eventScript, &stack, isConditionSatisfied);
LABEL_15:
        eventScript = eventScript->next;
    }
}

bool __cdecl Item_IsConditionSatisfied(
                int localClientNum,
                UiContext *dc,
                itemDef_s *item,
                GenericEventScript *eventScript)
{
    bool v4; // al

    if ( localClientNum == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    3974,
                    0,
                    "%s",
                    "localClientNum != INVALID_CLIENT_NUMBER") )
    {
        __debugbreak();
    }
    if ( eventScript->type == 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    3975,
                    0,
                    "%s",
                    "eventScript->type != CONDITION_TYPE_ELSE") )
    {
        __debugbreak();
    }
    if ( eventScript->type == 3 || !eventScript->condition.filename )
        return 0;
    v4 = IsExpressionTrue(localClientNum, item, &eventScript->condition);
    return Item_IsOutcomeAccepted(v4, eventScript->fireOnTrue);
}

void __cdecl Item_RunEventScript(int localClientNum, UiContext *dc, itemDef_s *item, const char *eventName)
{
    GenericEventHandler *eventHandler; // [esp+4h] [ebp-4h]

    if ( item->onEvent )
    {
        for ( eventHandler = item->onEvent; eventHandler; eventHandler = eventHandler->next )
        {
            if ( !I_stricmp(eventHandler->name, eventName) )
            {
                Item_RunSingleEventScript(localClientNum, dc, item, eventHandler->eventScript);
                return;
            }
        }
    }
}

void __cdecl Item_RunScript(int localClientNum, UiContext *dc, itemDef_s *item, const char *s)
{
    __int64 v4; // [esp-8h] [ebp-420h]
    int bRan; // [esp+0h] [ebp-418h]
    int controllerIndex; // [esp+4h] [ebp-414h]
    char command[1028]; // [esp+8h] [ebp-410h] BYREF
    unsigned int i; // [esp+410h] [ebp-8h]
    const char *p; // [esp+414h] [ebp-4h] BYREF

    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( item && s && *s )
    {
        p = s;
        while ( String_Parse(&p, command, 1024) )
        {
            if ( command[0] != 59 || command[1] )
            {
                bRan = 0;
                for ( i = 0; i < 0x32; ++i )
                {
                    if ( !I_stricmp(command, commandList[i].name) )
                    {
                        commandList[i].handler(localClientNum, dc, item, &p);
                        localClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
                        bRan = 1;
                        break;
                    }
                }
                if ( !bRan )
                {
                    HIDWORD(v4) = (DWORD)s;
                    LODWORD(v4) = (DWORD)&p;
                    UI_RunMenuScript(localClientNum, dc->contextIndex, v4);
                }
            }
        }
    }
}

void __cdecl UIAnimation_RunEventScript(
                int localClientNum,
                UiContext *dc,
                itemDef_s *item,
                animParamsDef_t *animState,
                const char *eventName)
{
    GenericEventHandler *eventHandler; // [esp+4h] [ebp-4h]

    if ( animState->onEvent )
    {
        for ( eventHandler = animState->onEvent; eventHandler; eventHandler = eventHandler->next )
        {
            if ( !I_stricmp(eventHandler->name, eventName) )
            {
                Item_RunSingleEventScript(localClientNum, dc, item, eventHandler->eventScript);
                return;
            }
        }
    }
}


void __cdecl UI_RegisterCmds()
{
    Cmd_AddCommandInternal("setlocalvarbool", UI_SetLocalVarBool_f, &UI_SetLocalVarBool_f_VAR);
    Cmd_AddCommandInternal("setlocalvarint", UI_SetLocalVarInt_f, &UI_SetLocalVarInt_f_VAR);
    Cmd_AddCommandInternal("setlocalvarfloat", UI_SetLocalVarFloat_f, &UI_SetLocalVarFloat_f_VAR);
    Cmd_AddCommandInternal("setlocalvarstring", UI_SetLocalVarString_f, &UI_SetLocalVarString_f_VAR);
    Cmd_AddCommandInternal("fadeitem", UI_FadeItem_f, &UI_FadeItem_f_VAR);
    Cmd_AddCommandInternal("showitem", UI_ShowItem_f, &UI_ShowItem_f_VAR);
    Cmd_AddCommandInternal("hideitem", UI_HideItem_f, &UI_HideItem_f_VAR);
    Cmd_AddCommandInternal("showmenu", UI_ShowMenu_f, &UI_ShowMenu_f_VAR);
    Cmd_AddCommandInternal("hidemenu", UI_HideMenu_f, &UI_HideMenu_f_VAR);
    Cmd_AddCommandInternal("openmenu", UI_OpenMenu_f, &UI_OpenMenu_f_VAR);
    Cmd_AddCommandInternal("closemenu", UI_CloseMenu_f, &UI_CloseMenu_f_VAR);
    Cmd_AddCommandInternal("openmenuimmediate", UI_OpenMenuImmediate_f, &UI_OpenMenuImmediate_f_VAR);
    Cmd_AddCommandInternal("closemenuimmediate", UI_CloseMenuImmediate_f, &UI_CloseMenuImmediate_f_VAR);
    Cmd_AddCommandInternal(
        "changeMenuOpenSlideDirection",
        UI_ChangeMenuOpenSlideDirection_f,
        &UI_ChangeMenuOpenSlideDirection_f_VAR);
    Cmd_AddCommandInternal(
        "changeMenuCloseSlideDirection",
        UI_ChangeMenuCloseSlideDirection_f,
        &UI_ChangeMenuCloseSlideDirection_f_VAR);
    Cmd_AddCommandInternal("opentoastpopup", UI_OpenToastPopup_f, &UI_OpenToastPopup_f_VAR);
    Cmd_AddCommandInternal("focusitem", UI_FocusItem_f, &UI_FocusItem_f_VAR);
    Cmd_AddCommandInternal("playsound", UI_PlaySound_f, &UI_PlaySound_f_VAR);
    Cmd_AddCommandInternal("movefeeder", UI_MoveFeeder_f, &UI_MoveFeeder_f_VAR);
    Cmd_AddCommandInternal("addToFeeder", UI_AddToFeeder_f, &UI_AddToFeeder_f_VAR);
    Cmd_AddCommandInternal("addToFeederExtended", UI_AddToFeederExtended_f, &UI_AddToFeederExtended_f_VAR);
    Cmd_AddCommandInternal(
        "clearFeederWithoutResetCursor",
        UI_ClearFeederWithoutResetCursor_f,
        &UI_ClearFeederWithoutResetCursor_f_VAR);
    Cmd_AddCommandInternal("clearFeeder", UI_ClearFeeder_f, &UI_ClearFeeder_f_VAR);
    Cmd_AddCommandInternal("raiseFeederEvent", UI_RaiseFeederEvent_f, &UI_RaiseFeederEvent_f_VAR);
    Cmd_AddCommandInternal("refreshFeeder", UI_RefreshFeeder_f, &UI_RefreshFeeder_f_VAR);
    Cmd_AddCommandInternal("refreshFeederSelection", UI_RefreshFeederSelection_f, &UI_RefreshFeederSelection_f_VAR);
    Cmd_AddCommandInternal("changeRowStatus", UI_ChangeRowStatus_f, &UI_ChangeRowStatus_f_VAR);
    Cmd_AddCommandInternal(
        "validatePrivateMatchGametype",
        UI_ValidatePrivateMatchGametype_f,
        &UI_ValidatePrivateMatchGametype_f_VAR);
    Cmd_AddCommandInternal("setactivemenu", UI_SetActiveMenu_f, &UI_SetActiveMenu_f_VAR);
}

void __cdecl UI_SetLocalVarBool_f()
{
    char *v0; // eax
    const char *v1; // eax
    int v2; // eax
    UILocalVarContext *var; // [esp+10h] [ebp-8h]
    uiInfo_s *uiInfo; // [esp+14h] [ebp-4h]

    uiInfo = UI_GetInfo(0);
    v0 = (char *)Cmd_Argv(1);
    var = UILocalVar_FindOrCreate(&uiInfo->uiDC.localVars, v0);
    if ( var )
    {
        v1 = Cmd_Argv(2);
        v2 = atoi(v1);
        UILocalVar_SetBool(var->table, v2 != 0);
    }
}

void __cdecl UI_SetLocalVarInt_f()
{
    char *v0; // eax
    const char *v1; // eax
    int v2; // eax
    UILocalVarContext *var; // [esp+10h] [ebp-8h]
    uiInfo_s *uiInfo; // [esp+14h] [ebp-4h]

    uiInfo = UI_GetInfo(0);
    v0 = (char *)Cmd_Argv(1);
    var = UILocalVar_FindOrCreate(&uiInfo->uiDC.localVars, v0);
    if ( var )
    {
        v1 = Cmd_Argv(2);
        v2 = atoi(v1);
        UILocalVar_SetInt(var->table, v2);
    }
}

void __cdecl UI_SetLocalVarFloat_f()
{
    char *v0; // eax
    const char *v1; // eax
    float f; // [esp+4h] [ebp-1Ch]
    UILocalVarContext *var; // [esp+18h] [ebp-8h]
    uiInfo_s *uiInfo; // [esp+1Ch] [ebp-4h]

    uiInfo = UI_GetInfo(0);
    v0 = (char *)Cmd_Argv(1);
    var = UILocalVar_FindOrCreate(&uiInfo->uiDC.localVars, v0);
    if ( var )
    {
        v1 = Cmd_Argv(2);
        f = atof(v1);
        UILocalVar_SetFloat(var->table, f);
    }
}

void __cdecl UI_SetLocalVarString_f()
{
    char *v0; // eax
    char *v1; // eax
    UILocalVarContext *var; // [esp+10h] [ebp-8h]
    uiInfo_s *uiInfo; // [esp+14h] [ebp-4h]

    uiInfo = UI_GetInfo(0);
    v0 = (char *)Cmd_Argv(1);
    var = UILocalVar_FindOrCreate(&uiInfo->uiDC.localVars, v0);
    if ( var )
    {
        v1 = (char *)Cmd_Argv(2);
        UILocalVar_SetString(var->table, v1);
    }
}

void __cdecl UI_FadeItem_f()
{
    const char *v0; // eax
    const char *v1; // eax
    itemDef_s *fadeItem; // [esp+28h] [ebp-10h]
    bool fadeOut; // [esp+2Ch] [ebp-Ch]
    itemDef_s *baseItem; // [esp+30h] [ebp-8h]
    int contextIndex; // [esp+34h] [ebp-4h]

    baseItem = Cmd_ItemDef();
    if ( baseItem && Cmd_Argc() != 1 )
    {
        v0 = Cmd_Argv(1);
        fadeOut = I_stricmp(v0, "out") == 0;
        contextIndex = Com_LocalClient_GetUIContextIndex(0);
        if ( contextIndex == -1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                        4230,
                        0,
                        "%s",
                        "contextIndex != INVALID_UI_CONTEXT") )
        {
            __debugbreak();
        }
        if ( Cmd_Argc() <= 2 )
        {
            Item_Fade(contextIndex, baseItem, fadeOut);
        }
        else
        {
            v1 = Cmd_Argv(2);
            fadeItem = Menu_FindItemByName(baseItem->parent, v1);
            if ( fadeItem )
                Item_Fade(contextIndex, fadeItem, fadeOut);
        }
    }
}

void __cdecl UI_ShowItem_f()
{
    const char *v0; // eax
    itemDef_s *showItem; // [esp+18h] [ebp-Ch]
    itemDef_s *baseItem; // [esp+1Ch] [ebp-8h]
    int contextIndex; // [esp+20h] [ebp-4h]

    baseItem = Cmd_ItemDef();
    if ( baseItem )
    {
        contextIndex = Com_LocalClient_GetUIContextIndex(0);
        if ( contextIndex == -1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                        4254,
                        0,
                        "%s",
                        "contextIndex != INVALID_UI_CONTEXT") )
        {
            __debugbreak();
        }
        if ( Cmd_Argc() == 2 )
        {
            v0 = Cmd_Argv(1);
            showItem = Menu_FindItemByName(baseItem->parent, v0);
            if ( showItem )
                Item_Show(contextIndex, showItem, 1);
        }
        else
        {
            Item_Show(contextIndex, baseItem, 1);
        }
    }
}

void __cdecl UI_HideItem_f()
{
    const char *v0; // eax
    itemDef_s *showItem; // [esp+18h] [ebp-Ch]
    itemDef_s *baseItem; // [esp+1Ch] [ebp-8h]
    int contextIndex; // [esp+20h] [ebp-4h]

    baseItem = Cmd_ItemDef();
    if ( baseItem )
    {
        contextIndex = Com_LocalClient_GetUIContextIndex(0);
        if ( contextIndex == -1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                        4276,
                        0,
                        "%s",
                        "contextIndex != INVALID_UI_CONTEXT") )
        {
            __debugbreak();
        }
        if ( Cmd_Argc() == 2 )
        {
            v0 = Cmd_Argv(1);
            showItem = Menu_FindItemByName(baseItem->parent, v0);
            if ( showItem )
                Item_Show(contextIndex, showItem, 0);
        }
        else
        {
            Item_Show(contextIndex, baseItem, 0);
        }
    }
}

void __cdecl UI_ShowMenu_f()
{
    const char *v0; // eax
    uiInfo_s *uiInfo; // [esp+10h] [ebp-4h]

    uiInfo = UI_GetInfo(0);
    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        Menus_ShowByName(&uiInfo->uiDC, v0);
    }
}

void __cdecl UI_HideMenu_f()
{
    const char *v0; // eax
    uiInfo_s *uiInfo; // [esp+10h] [ebp-4h]

    uiInfo = UI_GetInfo(0);
    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        Menus_HideByName(&uiInfo->uiDC, v0);
    }
}

void __cdecl UI_OpenMenu_f()
{
    const char *v0; // eax
    uiInfo_s *uiInfo; // [esp+8h] [ebp-4h]

    uiInfo = UI_GetInfo(0);
    v0 = Cmd_Argv(1);
    Menus_OpenByName(0, &uiInfo->uiDC, v0);
}

void __cdecl UI_CloseMenu_f()
{
    const char *v0; // eax
    uiInfo_s *uiInfo; // [esp+8h] [ebp-4h]

    uiInfo = UI_GetInfo(0);
    v0 = Cmd_Argv(1);
    Menus_CloseByName(0, &uiInfo->uiDC, v0);
}

void __cdecl UI_OpenMenuImmediate_f()
{
    const char *v0; // eax
    uiInfo_s *uiInfo; // [esp+8h] [ebp-4h]

    uiInfo = UI_GetInfo(0);
    v0 = Cmd_Argv(1);
    Menus_OpenImmediateByName(0, &uiInfo->uiDC, v0);
}

void __cdecl UI_CloseMenuImmediate_f()
{
    const char *v0; // eax
    uiInfo_s *uiInfo; // [esp+8h] [ebp-4h]

    uiInfo = UI_GetInfo(0);
    v0 = Cmd_Argv(1);
    Menus_CloseImmediateByName(0, &uiInfo->uiDC, v0);
}

void __cdecl UI_ChangeMenuOpenSlideDirection_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    uiInfo_s *uiInfo; // [esp+28h] [ebp-Ch]
    menuDef_t *menu; // [esp+2Ch] [ebp-8h]
    unsigned int newSlideDirection; // [esp+30h] [ebp-4h]

    if ( Cmd_Argc() == 3 )
    {
        uiInfo = UI_GetInfo(0);
        v0 = Cmd_Argv(1);
        menu = Menus_FindByName(&uiInfo->uiDC, v0);
        if ( menu )
        {
            v2 = Cmd_Argv(2);
            newSlideDirection = atoi(v2);
            if ( newSlideDirection < 4 )
            {
                menu->openSlideDirection = newSlideDirection;
            }
            else
            {
                v3 = Cmd_Argv(1);
                Com_Printf(13, "Invalid open slide direction specified for menu: %s \n", v3);
            }
        }
        else
        {
            v1 = Cmd_Argv(1);
            Com_Printf(13, "Couldnt find menu: %s \n", v1);
        }
    }
    else
    {
        Com_Printf(13, "Usage: changeMenuOpenSlideDirection <menu name> <new direction>\n");
    }
}

void __cdecl UI_ChangeMenuCloseSlideDirection_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    uiInfo_s *uiInfo; // [esp+28h] [ebp-Ch]
    menuDef_t *menu; // [esp+2Ch] [ebp-8h]
    unsigned int newSlideDirection; // [esp+30h] [ebp-4h]

    if ( Cmd_Argc() == 3 )
    {
        uiInfo = UI_GetInfo(0);
        v0 = Cmd_Argv(1);
        menu = Menus_FindByName(&uiInfo->uiDC, v0);
        if ( menu )
        {
            v2 = Cmd_Argv(2);
            newSlideDirection = atoi(v2);
            if ( newSlideDirection < 4 )
            {
                menu->closeSlideDirection = newSlideDirection;
            }
            else
            {
                v3 = Cmd_Argv(1);
                Com_Printf(13, "Invalid open slide direction specified for menu: %s \n", v3);
            }
        }
        else
        {
            v1 = Cmd_Argv(1);
            Com_Printf(13, "Couldnt find menu: %s \n", v1);
        }
    }
    else
    {
        Com_Printf(13, "Usage: changeMenuCloseSlideDirection <menu name> <new direction>\n");
    }
}

void __cdecl UI_OpenToastPopup_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    int openSlideSpeed; // [esp+0h] [ebp-44h]
    int openFadingTime; // [esp+4h] [ebp-40h]
    uiInfo_s *uiInfo; // [esp+38h] [ebp-Ch]
    menuDef_t *toastPopupMenu; // [esp+3Ch] [ebp-8h]
    int contextIndex; // [esp+40h] [ebp-4h]

    contextIndex = Com_LocalClient_GetUIContextIndex(0);
    if ( contextIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    4411,
                    0,
                    "%s",
                    "contextIndex != INVALID_UI_CONTEXT") )
    {
        __debugbreak();
    }
    uiInfo = UI_UIContext_GetInfo(contextIndex);
    v0 = Cmd_Argv(1);
    I_strncpyz(uiInfo->toastPopupIconName, v0, 256);
    v1 = Cmd_Argv(2);
    I_strncpyz(uiInfo->toastPopupTitle, v1, 256);
    v2 = Cmd_Argv(3);
    I_strncpyz(uiInfo->toastPopupDesc, v2, 256);
    v3 = Cmd_Argv(4);
    uiInfo->toastPopupDuration = atoi(v3);
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

void __cdecl UI_FocusItem_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    uiInfo_s *uiInfo; // [esp+30h] [ebp-14h]
    itemDef_s *focusItem; // [esp+34h] [ebp-10h]
    itemDef_s *baseItem; // [esp+38h] [ebp-Ch]
    int contextIndex; // [esp+3Ch] [ebp-8h]
    const rectDef_s *rect; // [esp+40h] [ebp-4h]

    baseItem = Cmd_ItemDef();
    if ( baseItem )
    {
        contextIndex = Com_LocalClient_GetUIContextIndex(0);
        if ( contextIndex == -1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                        4455,
                        0,
                        "%s",
                        "contextIndex != INVALID_UI_CONTEXT") )
        {
            __debugbreak();
        }
        uiInfo = UI_UIContext_GetInfo(contextIndex);
        if ( Cmd_Argc() <= 1 )
        {
            focusItem = baseItem;
        }
        else
        {
            v0 = Cmd_Argv(1);
            focusItem = Menu_FindFirstFocusableItemByName(0, &uiInfo->uiDC, baseItem->parent, v0);
            if ( !focusItem )
            {
                v1 = Cmd_Argv(1);
                Com_Printf(13, "setFocus: could not find widget named '%s'\n", v1);
                return;
            }
        }
        rect = Window_GetRect(&focusItem->window);
        if ( Item_SetFocus(0, &uiInfo->uiDC, focusItem, rect->x, rect->y) )
        {
            if ( Item_IsTextField(focusItem) )
                Item_TextField_BeginEdit(0, contextIndex, focusItem);
        }
        else
        {
            v2 = Cmd_Argv(1);
            Com_DPrintf(13, "setFocus: error focusing widget '%s' (widget was found but could not accept focus)\n", v2);
        }
    }
}

void __cdecl UI_MoveFeeder_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    int v5; // eax
    const char *v6; // eax
    int v7; // eax
    int v8; // [esp+8h] [ebp-80h]
    int special; // [esp+Ch] [ebp-7Ch]
    int v10; // [esp+10h] [ebp-78h]
    int v11; // [esp+24h] [ebp-64h]
    int v12; // [esp+38h] [ebp-50h]
    const char *max; // [esp+68h] [ebp-20h]
    itemDef_s *feederItem; // [esp+6Ch] [ebp-1Ch]
    listBoxDef_s *listPtr; // [esp+74h] [ebp-14h]
    int viewmax; // [esp+78h] [ebp-10h]
    itemDef_s *baseItem; // [esp+7Ch] [ebp-Ch]
    int contextIndex; // [esp+80h] [ebp-8h]

    baseItem = Cmd_ItemDef();
    if ( baseItem && Cmd_Argc() != 1 )
    {
        if ( Cmd_Argc() <= 2 )
        {
            feederItem = baseItem;
        }
        else
        {
            v0 = Cmd_Argv(2);
            feederItem = Menu_FindItemByName(baseItem->parent, v0);
            if ( !feederItem )
            {
                v1 = Cmd_Argv(2);
                Com_Printf(13, "selectFeeder: could not find widget named '%s'\n", v1);
                return;
            }
        }
        listPtr = Item_GetListBoxDef(feederItem);
        if ( listPtr )
        {
            contextIndex = Com_LocalClient_GetUIContextIndex(0);
            if ( contextIndex == -1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                            4518,
                            0,
                            "%s",
                            "contextIndex != INVALID_UI_CONTEXT") )
            {
                __debugbreak();
            }
            max = Item_ListBox_MaxScroll(0, contextIndex, feederItem);
            viewmax = Item_ListBox_Viewmax(0, contextIndex, feederItem);
            v12 = UI_FeederCount(0, contextIndex, listPtr->special, listPtr) - 1;
            if ( (int)v12 > 0 )
                v10 = (int)v12;
            else
                v10 = 0;
            v2 = Cmd_Argv(1);
            if ( I_stricmp("top", v2) )
            {
                v3 = Cmd_Argv(1);
                if ( I_stricmp("bottom", v3) )
                {
                    if ( !listPtr->notselectable )
                    {
                        special = (int)listPtr->special;
                        if ( special == 32 || special == 47 || special == 50 )
                        {
                            v11 = UI_FeederCount(0, contextIndex, listPtr->special, listPtr) - 1;
                            if ( (int)v11 > 0 )
                                v8 = (int)v11;
                            else
                                v8 = 0;
                            v4 = Cmd_Argv(1);
                            v5 = atoi(v4);
                            Item_ListBox_Scroll(0, contextIndex, feederItem, v8, (int)max, viewmax, v5);
                        }
                        else
                        {
                            v6 = Cmd_Argv(1);
                            v7 = atoi(v6);
                            Item_ListBox_SetCursorPos(0, contextIndex, feederItem, (int)max, viewmax, v7, 1);
                        }
                    }
                }
                else if ( listPtr->notselectable )
                {
                    listPtr->startPos[contextIndex] = (int)max;
                }
                else
                {
                    Item_ListBox_SetCursorPos(0, contextIndex, feederItem, (int)max, viewmax, v10, 1);
                }
            }
            else
            {
                listPtr->startPos[contextIndex] = 0;
                if ( !listPtr->notselectable )
                {
                    listPtr->cursorPos[contextIndex] = 0;
                    UI_FeederSelection(0, contextIndex, listPtr->special, 0);
                }
            }
        }
    }
}

void __cdecl UI_AddToFeeder_f()
{
    int v0; // esi
    const char *v1; // eax
    itemDef_s *v2; // eax
    const char *v3; // [esp-Ch] [ebp-64h]
    int v4; // [esp-8h] [ebp-60h]
    const char *argArray[10]; // [esp+2Ch] [ebp-2Ch] BYREF
    int numArgs; // [esp+54h] [ebp-4h]

    numArgs = 0;
    if ( Cmd_Argc() >= 3 )
    {
        while ( 1 )
        {
            v0 = numArgs + 2;
            if ( v0 >= Cmd_Argc() )
                break;
            v1 = Cmd_Argv(numArgs + 2);
            argArray[numArgs++] = v1;
        }
        v4 = numArgs;
        v3 = Cmd_Argv(1);
        v2 = Cmd_ItemDef();
        UI_AddToFeeder(v2, v3, v4, argArray);
    }
    else
    {
        Com_Printf(13, "Usage: addToFeeder <event index> <args>..\n");
    }
}

void __cdecl UI_AddToFeederExtended_f()
{
    int v0; // esi
    const char *v1; // eax
    const char *v2; // eax
    itemDef_s *v3; // eax
    const char *v4; // [esp-14h] [ebp-80h]
    const char *v5; // [esp-10h] [ebp-7Ch]
    int v6; // [esp-8h] [ebp-74h]
    bool disableCondition; // [esp+3Fh] [ebp-2Dh]
    const char *argArray[10]; // [esp+40h] [ebp-2Ch] BYREF
    int numArgs; // [esp+68h] [ebp-4h]

    numArgs = 0;
    if ( Cmd_Argc() >= 5 )
    {
        while ( 1 )
        {
            v0 = numArgs + 4;
            if ( v0 >= Cmd_Argc() )
                break;
            v1 = Cmd_Argv(numArgs + 4);
            argArray[numArgs++] = v1;
        }
        v2 = Cmd_Argv(3);
        disableCondition = atoi(v2) != 0;
        v6 = numArgs;
        v5 = Cmd_Argv(2);
        v4 = Cmd_Argv(1);
        v3 = Cmd_ItemDef();
        UI_AddToFeederExtended(v3, v4, v5, disableCondition, v6, argArray);
    }
    else
    {
        Com_Printf(13, "Usage: addToFeederExtended <onAction event> <onFocus event>    <disable condition> <args>..\n");
    }
}

void __cdecl UI_ClearFeeder_f()
{
    itemDef_s *v0; // eax
    int contextIndex; // [esp+10h] [ebp-4h]

    if ( Cmd_Argc() == 1 )
    {
        contextIndex = Com_LocalClient_GetUIContextIndex(0);
        if ( contextIndex == -1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                        4617,
                        0,
                        "%s",
                        "contextIndex != INVALID_UI_CONTEXT") )
        {
            __debugbreak();
        }
        v0 = Cmd_ItemDef();
        UI_ClearFeeder(contextIndex, v0, 1);
    }
    else
    {
        Com_Printf(13, "Usage: clearFeeder\n");
    }
}

void __cdecl UI_ClearFeederWithoutResetCursor_f()
{
    itemDef_s *v0; // eax
    int contextIndex; // [esp+10h] [ebp-4h]

    if ( Cmd_Argc() == 1 )
    {
        contextIndex = Com_LocalClient_GetUIContextIndex(0);
        if ( contextIndex == -1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                        4630,
                        0,
                        "%s",
                        "contextIndex != INVALID_UI_CONTEXT") )
        {
            __debugbreak();
        }
        v0 = Cmd_ItemDef();
        UI_ClearFeeder(contextIndex, v0, 0);
    }
    else
    {
        Com_Printf(13, "Usage: clearFeederWithoutResetCursor\n");
    }
}

void __cdecl UI_RaiseFeederEvent_f()
{
    itemDef_s *v0; // eax
    itemDef_s *v1; // eax
    const char *v2; // [esp-4h] [ebp-34h]

    if ( Cmd_Argc() == 1 || Cmd_Argc() == 2 )
    {
        if ( Cmd_Argc() == 2 )
        {
            v2 = Cmd_Argv(1);
            v0 = Cmd_ItemDef();
            UI_RaiseFeederEvent(0, v0, v2);
        }
        else
        {
            v1 = Cmd_ItemDef();
            UI_RaiseFeederEvent(0, v1, "");
        }
    }
    else
    {
        Com_Printf(13, "Usage: raiseFeederEvent <optional event name>\n");
    }
}

void __cdecl UI_RefreshFeeder_f()
{
    if ( Cmd_Argc() != 2 )
        Com_Printf(13, "Usage: refreshFeeder <populate event name>\n");
}

void __cdecl UI_RefreshFeederSelection_f()
{
    uiInfo_s *Info; // eax
    const char *v1; // eax
    menuDef_t *menu; // [esp+20h] [ebp-18h]
    itemDef_s *item; // [esp+24h] [ebp-14h]
    int selection; // [esp+28h] [ebp-10h]
    listBoxDef_s *listPtr; // [esp+2Ch] [ebp-Ch]
    int contextIndex; // [esp+30h] [ebp-8h]
    int count; // [esp+34h] [ebp-4h]

    item = Cmd_ItemDef();
    if ( Cmd_Argc() > 1 )
    {
        Info = UI_GetInfo(0);
        menu = Menu_GetFocused(&Info->uiDC);
        v1 = Cmd_Argv(1);
        item = Menu_FindItemByName(menu, v1);
    }
    if ( item )
    {
        listPtr = Item_GetListBoxDef(item);
        if ( listPtr )
        {
            contextIndex = Com_LocalClient_GetUIContextIndex(0);
            count = UI_FeederCount(0, contextIndex, listPtr->special, listPtr);
            selection = listPtr->cursorPos[contextIndex];
            if ( selection < 0 || selection >= count )
                selection = 0;
            UI_FeederSelection(0, contextIndex, listPtr->special, selection);
        }
        else
        {
            Com_Printf(13, "refreshFeederSelection: No list box found for itemdef.\n");
        }
    }
    else
    {
        Com_Printf(13, "refreshFeederSelection: Could not find an itemdef.\n");
    }
}

void __cdecl UI_ChangeRowStatus_f()
{
    const char *v0; // eax
    const char *v1; // eax
    itemDef_s *v2; // eax
    int v3; // [esp-8h] [ebp-28h]
    int v4; // [esp-4h] [ebp-24h]

    if ( Cmd_Argc() == 4 )
    {
        v0 = Cmd_Argv(3);
        v4 = atoi(v0);
        v1 = Cmd_Argv(2);
        v3 = atoi(v1);
        v2 = Cmd_ItemDef();
        UI_ChangeRowStatus(v2, v3, v4);
    }
    else
    {
        Com_Printf(13, "Usage: changeRowStatus <menu name> <row index> <new status>\n");
    }
}

void __cdecl UI_PlaySound_f()
{
    char UIContextIndex; // al
    char *v1; // [esp-4h] [ebp-Ch]

    v1 = (char *)Cmd_Argv(1);
    UIContextIndex = Com_LocalClient_GetUIContextIndex(0);
    UI_PlaySound(UIContextIndex, v1);
}

void __cdecl UI_ValidatePrivateMatchGametype_f()
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < sharedUiInfo.numCustomMatchGameTypes; ++i )
    {
        if ( !I_strcmp(ui_gametype->current.string, sharedUiInfo.customMatchGameTypes[i].gameType) )
            return;
    }
    Dvar_SetString((dvar_s *)ui_gametype, (const char *)&sharedUiInfo.numCustomMatchGameTypes);
}

void __cdecl UI_SetActiveMenu_f()
{
    const char *s; // [esp+14h] [ebp-4h]

    if ( Cmd_Argc() == 2 )
    {
        s = Cmd_Argv(1);
        Com_Printf(13, "Unhandled type %s in setActiveMenu command\n", s);
    }
    else
    {
        Com_Printf(13, "Usage: setActiveMenu <menuType>\n");
    }
}

int __cdecl Item_SetFocus(int localClientNum, UiContext *dc, itemDef_s *item, float x, float y)
{
    const char *v6; // eax
    bool v7; // [esp+8h] [ebp-48h]
    listBoxDef_s *listPtr; // [esp+10h] [ebp-40h]
    menuDef_t *serverBrowser; // [esp+14h] [ebp-3Ch]
    rectDef_s r; // [esp+1Ch] [ebp-34h] BYREF
    const rectDef_s *textRect; // [esp+34h] [ebp-1Ch]
    itemDef_s *oldFocus; // [esp+38h] [ebp-18h]
    menuDef_t *focusedMenu; // [esp+3Ch] [ebp-14h]
    focusItemDef_s *focusPtr; // [esp+40h] [ebp-10h]
    menuDef_t *parent; // [esp+44h] [ebp-Ch]
    bool isFilterWindow; // [esp+4Bh] [ebp-5h]
    int i; // [esp+4Ch] [ebp-4h]

    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 4847, 0, "%s", "item != NULL") )
        __debugbreak();
    if ( !item || (item->window.staticFlags & 0x100000) != 0 || !Window_IsVisible(dc->contextIndex, &item->window) )
        return 0;
    focusPtr = Item_GetFocusItemDef(item);
    if ( Window_HasFocus(dc->contextIndex, &item->window) && Item_IsVisible(localClientNum, dc->contextIndex, item) )
        return 1;
    parent = item->parent;
    v7 = parent && parent->window.name && !I_strcmp(parent->window.name, "filter_window");
    isFilterWindow = v7;
    if ( parent )
    {
        if ( !isFilterWindow && !Window_HasFocus(dc->contextIndex, &parent->window) )
        {
            focusedMenu = Menu_GetFocused(dc);
            if ( focusedMenu )
            {
                if ( Rect_ContainsPoint(dc->contextIndex, &focusedMenu->window.rect, x, y)
                    && Rect_ContainsPoint(dc->contextIndex, &parent->window.rect, x, y) )
                {
                    return 0;
                }
            }
        }
    }
    if ( (item->dvarFlags & 3) != 0 && !Item_EnableShowViaDvar(item, 1) )
        return 0;
    if ( !Item_IsVisible(localClientNum, dc->contextIndex, item) )
        return 0;
    oldFocus = Menu_ClearFocus(localClientNum, dc, item->parent);
    if ( item->type == 1 )
    {
        textRect = (const rectDef_s *)Item_GetTextRect(dc->contextIndex, item);
        r = *textRect;
        r.y = r.y - r.h;
        r.horzAlign = textRect->horzAlign;
        r.vertAlign = textRect->vertAlign;
        if ( Rect_ContainsPoint(dc->contextIndex, &r, x, y) )
        {
            Window_AddDynamicFlags(dc->contextIndex, &item->window, 2);
        }
        else if ( oldFocus )
        {
            if ( Item_IsFocusDefType(oldFocus) )
                Item_GetFocusItemDef(oldFocus);
            Window_AddDynamicFlags(dc->contextIndex, &oldFocus->window, 2);
            Item_RunEventScript(localClientNum, dc, item, "onFocus");
        }
    }
    else
    {
        Window_AddDynamicFlags(dc->contextIndex, &item->window, 2);
        if ( isFilterWindow )
        {
            serverBrowser = Menu_GetFocused(dc);
            if ( serverBrowser )
            {
                v6 = strstr(serverBrowser->window.name, "server_browser");
                if ( v6 )
                    Window_RemoveDynamicFlags(dc->contextIndex, &serverBrowser->window, 2);
            }
            Window_AddDynamicFlags(dc->contextIndex, &item->parent->window, 2);
        }
        Item_RunEventScript(localClientNum, dc, item, "onFocus");
    }
    for ( i = 0; i < parent->itemCount; ++i )
    {
        if ( parent->items[i] == item )
        {
            Menu_SetCursorItem(dc->contextIndex, parent, i);
            break;
        }
    }
    if ( item->type == 4 )
    {
        listPtr = Item_GetListBoxDef(item);
        if ( !listPtr && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 4961, 0, "%s", "listPtr") )
            __debugbreak();
        UI_OverrideCursorPos(localClientNum, dc->contextIndex, item);
        if ( listPtr && listPtr->cursorPos[dc->contextIndex] >= 0 )
        {
            if ( item->parent->window.name )
            {
                if ( Menu_IsMenuOpenAndVisible(dc->contextIndex, item->parent->window.name) )
                    Item_RunEventScript(localClientNum, dc, item, "onListboxSelectionChange");
            }
            UI_FeederSelection(localClientNum, dc->contextIndex, listPtr->special, listPtr->cursorPos[dc->contextIndex]);
        }
    }
    return 1;
}

textDef_s *__cdecl Item_GetTextRect(int contextIndex, const itemDef_s *item)
{
    if ( contextIndex
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\ui\\../ui/ui_utils.h",
                    56,
                    0,
                    "contextIndex doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    contextIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( !item && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\ui\\../ui/ui_utils.h", 57, 0, "%s", "item") )
        __debugbreak();
    if ( item->typeData.textDef )
        return (textDef_s *)((char *)item->typeData.textDef + 24 * contextIndex);
    else
        return 0;
}

const char *__cdecl Item_ListBox_MaxScroll(int localClientNum, int contextIndex, itemDef_s *item)
{
    int v3; // esi
    int v6; // [esp+10h] [ebp-8h]
    listBoxDef_s *listPtr; // [esp+14h] [ebp-4h]

    listPtr = Item_GetListBoxDef(item);
    v3 = Item_ListBox_Viewmax(localClientNum, contextIndex, item);
    v6 = UI_FeederCount(localClientNum, contextIndex, listPtr->special, listPtr) - v3;
    if (v6 > 0)
        return (const char *)v6;
    else
        return 0;
}

int __cdecl Item_ListBox_Viewmax(int localClientNum, int contextIndex, itemDef_s *item)
{
    float v4; // [esp+8h] [ebp-2Ch]
    float v5; // [esp+10h] [ebp-24h]
    listBoxDef_s *listPtr; // [esp+24h] [ebp-10h]
    float unitSize; // [esp+28h] [ebp-Ch]
    const rectDef_s *rect; // [esp+2Ch] [ebp-8h]
    float totalSize; // [esp+30h] [ebp-4h]

    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 5032, 0, "%s", "item") )
        __debugbreak();
    rect = Window_GetRect(&item->window);
    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 5036, 0, "%s", "listPtr") )
        __debugbreak();
    if ( Window_IsHorizontal(&item->window) )
    {
        if ( (float)(0.0 - (float)(rect->w - 2.0)) < 0.0 )
            v5 = rect->w - 2.0;
        else
            v5 = 0.0f;
        totalSize = v5;
        unitSize = listPtr->elementWidth;
    }
    else
    {
        if ( (float)(0.0 - (float)(rect->h - 2.0)) < 0.0 )
            v4 = rect->h - 2.0;
        else
            v4 = 0.0f;
        totalSize = v4;
        unitSize = listPtr->elementHeight;
    }
    if ( totalSize < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    5050,
                    0,
                    "%s\n\t(totalSize) = %g",
                    "(totalSize >= 0)",
                    totalSize) )
    {
        __debugbreak();
    }
    if ( unitSize < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    5051,
                    0,
                    "%s\n\t(unitSize) = %g",
                    "(unitSize >= 0)",
                    unitSize) )
    {
        __debugbreak();
    }
    return (int)(float)(totalSize / unitSize);
}

bool __cdecl Window_IsHorizontal(const windowDef_t *w)
{
    if ( !w && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\ui\\../ui/ui_utils.h", 78, 0, "%s", "w") )
        __debugbreak();
    return (w->staticFlags & 0x200000) != 0;
}

double __cdecl Item_ListBox_GetMaxHeight(int localClientNum, int contextIndex, itemDef_s *item)
{
    int i; // [esp+0h] [ebp-Ch]
    float exprH; // [esp+4h] [ebp-8h]
    float largestHeight; // [esp+8h] [ebp-4h]

    if ( !item->animInfo )
        return item->window.rect.h;
    largestHeight = item->window.rect.h;
    if ( item->rectExpData )
    {
        if ( item->rectExpData->rectHExp.filename )
        {
            exprH = GetExpressionFloat(localClientNum, item, &item->rectExpData->rectHExp);
            if ( exprH > largestHeight )
                largestHeight = exprH;
        }
    }
    for ( i = 0; i < item->animInfo->animStateCount; ++i )
    {
        if ( item->animInfo->animStates[i]->rectClient.h > largestHeight )
            largestHeight = item->animInfo->animStates[i]->rectClient.h;
    }
    return largestHeight;
}

bool __cdecl Item_ListBox_RequiresScroll(int localClientNum, int contextIndex, itemDef_s *item)
{
    listBoxDef_s *listPtr; // [esp+10h] [ebp-Ch]
    int maxItemCount; // [esp+14h] [ebp-8h]

    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 5060, 0, "%s", "listPtr") )
        __debugbreak();
    if ( listPtr->noScrollBars )
        return 0;
    if ( Window_GetRect(&item->window)->h <= 0.0 )
        return 0;
    if ( Window_IsHorizontal(&item->window) )
        return 1;
    maxItemCount = Item_ListBox_Viewmax(localClientNum, contextIndex, item);
    return (int)UI_FeederCount(localClientNum, contextIndex, listPtr->special, listPtr) > maxItemCount;
}

void __cdecl Item_ListBox_SetCursorPos(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                int max,
                int viewmax,
                int newCursorPos,
                bool autoScroll)
{
    int delta; // [esp+8h] [ebp-Ch]
    uiInfo_s *uiInfo; // [esp+Ch] [ebp-8h]
    listBoxDef_s *listPtr; // [esp+10h] [ebp-4h]

    listPtr = Item_GetListBoxDef(item);
    delta = newCursorPos - listPtr->cursorPos[contextIndex];
    listPtr->cursorPos[contextIndex] = newCursorPos;
    if ( listPtr->special != 54.0 && autoScroll )
    {
        if ( delta >= 0 )
        {
            if ( delta > 0 )
            {
                if ( newCursorPos > max + viewmax - 1 )
                    return;
                if ( newCursorPos > listPtr->startPos[contextIndex] + viewmax - 1 )
                    listPtr->startPos[contextIndex] = newCursorPos - (viewmax - 1);
            }
        }
        else
        {
            if ( newCursorPos < 0 )
                return;
            if ( newCursorPos < listPtr->startPos[contextIndex] )
                listPtr->startPos[contextIndex] = newCursorPos;
        }
    }
    Dvar_SetInt((dvar_s *)ui_listboxIndex, newCursorPos - listPtr->startPos[contextIndex]);
    UI_FeederSelection(localClientNum, contextIndex, listPtr->special, newCursorPos);
    uiInfo = UI_UIContext_GetInfo(contextIndex);
    if ( listPtr->special == 54.0 && *listPtr->rows[listPtr->cursorPos[contextIndex]].onFocusEventName )
        Item_RunEventScript(
            localClientNum,
            &uiInfo->uiDC,
            item,
            listPtr->rows[listPtr->cursorPos[contextIndex]].onFocusEventName);
    if ( item->parent->window.name )
    {
        if ( Menu_IsMenuOpenAndVisible(contextIndex, item->parent->window.name) )
            Item_RunEventScript(localClientNum, &uiInfo->uiDC, item, "onListboxSelectionChange");
    }
}

void __cdecl Item_ListBox_Scroll(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                int max,
                int scrollmax,
                int viewmax,
                int delta)
{
    int newPos; // [esp+0h] [ebp-8h]
    listBoxDef_s *listPtr; // [esp+4h] [ebp-4h]

    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 5732, 0, "%s", "listPtr") )
        __debugbreak();
    newPos = delta + listPtr->startPos[contextIndex];
    if ( newPos <= scrollmax && newPos >= 0 )
        listPtr->startPos[contextIndex] = newPos;
}

bool __cdecl Item_IsTextField(const itemDef_s *item)
{
    bool result; // al

    switch ( item->type )
    {
        case 5:
        case 7:
        case 0xD:
        case 0xE:
        case 0x16:
        case 0x1E:
            result = 1;
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

void __cdecl Item_TextField_BeginEdit(int localClientNum, int contextIndex, itemDef_s *item)
{
    unsigned int v3; // [esp+0h] [ebp-20h]
    int i; // [esp+18h] [ebp-8h]
    editFieldDef_s *editPtr; // [esp+1Ch] [ebp-4h]

    editPtr = Item_GetEditFieldDef(item);
    if ( editPtr )
        editPtr->paintOffset = 0;
    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 6476, 0, "%s", "item") )
        __debugbreak();
    if ( item->dvar )
        v3 = strlen(Dvar_GetVariantString(item->dvar));
    else
        v3 = 0;
    editPtr->cursorPos[contextIndex] = v3;
    g_editingField = 1;
    g_editItem = item;
    Key_SetOverstrikeMode(localClientNum, 1);
    for ( i = 0; i < item->parent->itemCount; ++i )
    {
        if ( item->parent->items[i] == item )
        {
            Menu_SetCursorItem(contextIndex, item->parent, i);
            return;
        }
    }
}

void __cdecl Menus_Open(int localClientNum, UiContext *dc, menuDef_t *menu)
{
    int openSlideSpeed; // [esp+0h] [ebp-1Ch]
    int openFadingTime; // [esp+4h] [ebp-18h]
    int i; // [esp+18h] [ebp-4h]
    int ia; // [esp+18h] [ebp-4h]

    for ( i = dc->openMenuCount - 1; i >= 0; --i )
        Menu_LoseFocusDueToOpen(localClientNum, dc, dc->menuStack[i].menu);
    for ( ia = 0; ia < dc->menuCount; ++ia )
    {
        if ( (Window_GetDynamicFlags(dc->contextIndex, &dc->Menus[ia]->window) & 2) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                        7381,
                        0,
                        "%s",
                        "!(Window_GetDynamicFlags( dc->contextIndex, &dc->Menus[i]->window ) & WINDOWDYNAMIC_HASFOCUS)") )
        {
            __debugbreak();
        }
    }
    Menus_AddToStack(localClientNum, dc, menu);
    if ( dc->enableSlideAndFadeEffectForMenu )
    {
        if ( menu->openSlideSpeed > 0 && menu->openSlideDirection < 4u )
        {
            Window_AddDynamicFlags(dc->contextIndex, &menu->window, 0x20000);
            menu->slideTimeCounter = Sys_Milliseconds();
        }
        if ( menu->openFadingTime > 0 )
        {
            Window_AddDynamicFlags(dc->contextIndex, &menu->window, 0x80000);
            menu->fadeTimeCounter = Sys_Milliseconds();
        }
        if ( (Window_GetDynamicFlags(dc->contextIndex, &menu->window) & 0x20000) != 0
            && (Window_GetDynamicFlags(dc->contextIndex, &menu->window) & 0x80000) != 0 )
        {
            if ( menu->openSlideSpeed < menu->openFadingTime )
                openFadingTime = menu->openFadingTime;
            else
                openFadingTime = menu->openSlideSpeed;
            menu->openSlideSpeed = openFadingTime;
            if ( menu->openSlideSpeed < menu->openFadingTime )
                openSlideSpeed = menu->openFadingTime;
            else
                openSlideSpeed = menu->openSlideSpeed;
            menu->openFadingTime = openSlideSpeed;
        }
    }
    Menus_SetupOpenMenu(localClientNum, dc, menu);
    if ( dc->openMenuCount > 0 && dc->Menus[2 * dc->openMenuCount + 599] != menu )
    {
        Menu_LoseFocusDueToOpen(localClientNum, dc, menu);
        Menus_SetupOpenMenu(localClientNum, dc, dc->Menus[2 * dc->openMenuCount + 599]);
    }
}

void __cdecl Menus_AddToStack(int localClientNum, UiContext *dc, menuDef_t *pMenu)
{
    int v3; // edx
    int i; // [esp+4h] [ebp-4h]

    Menus_RemoveFromStack(localClientNum, dc, pMenu);
    if ( dc->openMenuCount == 16 )
        Com_Error(ERR_DROP, "Too many menus opened");
    for ( i = ++dc->openMenuCount - 2; i >= -1; --i )
    {
        if ( i == -1 || dc->menuStack[i].menu && dc->menuStack[i].menu->priority >= pMenu->priority )
        {
            dc->menuStack[i + 1].menu = pMenu;
            dc->menuStack[i + 1].localClientNum = localClientNum;
            return;
        }
        v3 = dc->menuStack[i].localClientNum;
        dc->menuStack[i + 1].menu = dc->menuStack[i].menu;
        dc->menuStack[i + 1].localClientNum = v3;
    }
}

void __cdecl Menu_LoseFocusDueToOpen(int localClientNum, UiContext *dc, menuDef_t *menu)
{
    bool anyFound; // [esp+3h] [ebp-5h]
    int i; // [esp+4h] [ebp-4h]

    if ( !dc && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 1609, 0, "%s", "dc") )
        __debugbreak();
    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 1610, 0, "%s", "menu") )
        __debugbreak();
    if ( Window_HasFocus(dc->contextIndex, &menu->window) )
    {
        Menu_RunLeaveFocusScript(localClientNum, dc, menu);
        Window_RemoveDynamicFlags(dc->contextIndex, &menu->window, 2);
        anyFound = 0;
        for ( i = 0; i < menu->itemCount; ++i )
        {
            if ( Window_HasFocus(dc->contextIndex, &menu->items[i]->window) && Item_IsFocusDefType(menu->items[i]) )
            {
                Item_RunEventScript(localClientNum, dc, menu->items[i], "leaveFocus");
                if ( anyFound
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 1630, 0, "%s", "!anyFound") )
                {
                    __debugbreak();
                }
                anyFound = 1;
            }
        }
    }
}

int __cdecl Menus_OpenByName(int localClientNum, UiContext *dc, const char *p)
{
    menuDef_t *pMenu; // [esp+0h] [ebp-4h]

    pMenu = Menus_FindByName(dc, p);
    if ( pMenu )
    {
        dc->enableSlideAndFadeEffectForMenu = 1;
        Menus_Open(localClientNum, dc, pMenu);
        return 1;
    }
    else
    {
        Com_DPrintf(13, "Could not find menu '%s'\n", p);
        return 0;
    }
}

int __cdecl Menus_OpenImmediateByName(int localClientNum, UiContext *dc, const char *p)
{
    menuDef_t *pMenu; // [esp+0h] [ebp-4h]

    pMenu = Menus_FindByName(dc, p);
    if ( pMenu )
    {
        dc->enableSlideAndFadeEffectForMenu = 0;
        Menus_Open(localClientNum, dc, pMenu);
        return 1;
    }
    else
    {
        Com_DPrintf(13, "Could not find menu '%s'\n", p);
        return 0;
    }
}

void    Menus_SetupOpenMenu(int localClientNum, UiContext *dc, menuDef_t *menu)
{
    itemDef_s item;

    Window_AddDynamicFlags(dc->contextIndex, &menu->window, 6);
    Menu_CallOnFocusDueToOpen(localClientNum, dc, menu);

    if (Menu_GetCursorItem(dc->contextIndex, menu) == -1)
    {
        Menu_SetNextCursorItem(localClientNum, dc, menu, 0, 0);
    }

    if (menu->onEvent)
    {
        item.parent = menu;
        item.onEvent = menu->onEvent;
        Item_RunEventScript(localClientNum, dc, &item, "onOpen");
    }

    if (dc->isCursorVisible)
    {
        mouseLocationX = INT_MIN;
        mouseLocationY = INT_MIN;
        mouseTimeUntilReadyToMove = Sys_Milliseconds() + 500;
    }

    if (menu->soundName)
    {
        if (!CL_IsLocalClientInGame(localClientNum))
        {
            UI_PlaySound(dc->contextIndex, (char*)menu->soundName);
        }
    }
}

itemDef_s *__cdecl Menu_SetNextCursorItem(
                int localClientNum,
                UiContext *dc,
                menuDef_t *menu,
                int usedTab,
                int usedKeyboard)
{
    int cursorItema; // [esp+8h] [ebp-14h]
    int cursorItem; // [esp+8h] [ebp-14h]
    editFieldDef_s *editPtr; // [esp+Ch] [ebp-10h]
    int newIndex; // [esp+10h] [ebp-Ch]
    int oldCursor; // [esp+14h] [ebp-8h]
    int wrapped; // [esp+18h] [ebp-4h]

    wrapped = 0;
    if ( Dvar_GetBool("ui_ignoreMousePos") )
    {
        if ( Dvar_GetInt("ui_prevTextEntryBox") == -1 )
        {
            oldCursor = Menu_GetCursorItem(dc->contextIndex, menu);
            Dvar_SetIntByName("ui_prevTextEntryBox", oldCursor);
        }
        else
        {
            oldCursor = Dvar_GetInt("ui_prevTextEntryBox");
        }
        if ( oldCursor == -1 )
        {
            oldCursor = 0;
            Menu_SetCursorItem(dc->contextIndex, menu, 0);
        }
        while ( Dvar_GetInt("ui_prevTextEntryBox") < menu->itemCount )
        {
            newIndex = Dvar_GetInt("ui_prevTextEntryBox") + 1;
            Dvar_SetIntByName("ui_prevTextEntryBox", newIndex);
            Menu_SetCursorItem(dc->contextIndex, menu, newIndex);
            if ( newIndex < menu->itemCount )
            {
                if ( Item_IsEditFieldDef(menu->items[newIndex]) )
                {
                    editPtr = Item_GetEditFieldDef(menu->items[newIndex]);
                    if ( editPtr
                        && editPtr->maxCharsGotoNext
                        && Item_SetFocus(
                                 localClientNum,
                                 dc,
                                 menu->items[newIndex],
                                 menu->items[newIndex]->window.rect.x,
                                 menu->items[newIndex]->window.rect.y) )
                    {
                        return menu->items[newIndex];
                    }
                }
            }
            else
            {
                if ( !usedTab )
                {
                    Item_GetEditFieldDef(menu->items[oldCursor]);
                    Dvar_SetIntByName("ui_prevTextEntryBox", oldCursor);
                    Menu_SetCursorItem(dc->contextIndex, menu, oldCursor);
                    return menu->items[oldCursor];
                }
                Dvar_SetIntByName("ui_prevTextEntryBox", 0);
            }
        }
    }
    else
    {
        oldCursor = Menu_GetCursorItem(dc->contextIndex, menu);
        if ( oldCursor == -1 && menu->itemCount <= 0 )
        {
            Menu_SetCursorItem(dc->contextIndex, menu, 0);
            wrapped = 1;
        }
        while ( Menu_GetCursorItem(dc->contextIndex, menu) < menu->itemCount )
        {
            cursorItema = Menu_GetCursorItem(dc->contextIndex, menu);
            Menu_SetCursorItem(dc->contextIndex, menu, cursorItema + 1);
            if ( Menu_GetCursorItem(dc->contextIndex, menu) >= menu->itemCount )
            {
                if ( wrapped )
                    return menu->items[oldCursor];
                wrapped = 1;
                Menu_SetCursorItem(dc->contextIndex, menu, 0);
            }
            cursorItem = Menu_GetCursorItem(dc->contextIndex, menu);
            if ( (!usedKeyboard || Item_HasFocusEvent(localClientNum, dc, menu->items[cursorItem]))
                && Item_SetFocus(localClientNum, dc, menu->items[cursorItem], dc->cursor.x, dc->cursor.y) )
            {
                return menu->items[Menu_GetCursorItem(dc->contextIndex, menu)];
            }
        }
    }
    Menu_SetCursorItem(dc->contextIndex, menu, oldCursor);
    return 0;
}

int __cdecl Item_HasFocusEvent(int localClientNum, UiContext *dc, itemDef_s *item)
{
    GenericEventHandler *eventHandler; // [esp+0h] [ebp-4h]

    if ( item->onEvent )
    {
        for ( eventHandler = item->onEvent; eventHandler; eventHandler = eventHandler->next )
        {
            if ( !I_stricmp(eventHandler->name, "onfocus") || !I_stricmp(eventHandler->name, "leavefocus") )
                return 1;
        }
    }
    return 0;
}

void __cdecl Menus_PrintAllLoadedMenus(UiContext *dc)
{
    int i; // [esp+0h] [ebp-4h]

    Com_Printf(16, "Currently loaded UI menus (CG menus not included):\n");
    for ( i = 0; i < dc->menuCount; ++i )
        Com_Printf(16, "%i. %s\n", i, dc->Menus[i]->window.name);
    Com_Printf(16, "\n%i menus total\n", dc->menuCount);
}

int    Menu_CheckOnKey(int localClientNum, UiContext *dc, menuDef_t *menu, int key)
{
    focusItemDef_s *FocusItemDef; // [esp-Ch] [ebp-134h]
    itemDef_s *v7; // [esp-4h] [ebp-12Ch]
    int i; // [esp+0h] [ebp-128h]
    itemDef_s it; // [esp+4h] [ebp-124h] BYREF
    ItemKeyHandler *handler; // [esp+118h] [ebp-10h]
    UiContext *dca; // [esp+124h] [ebp-4h] BYREF
    menuDef_t *menua; // [esp+128h] [ebp+0h]

    for (handler = menu->onKey; handler; handler = handler->next)
    {
        if (handler->key == -1 || handler->key == key)
        {
        LABEL_22:
            it.parent = menu;
            Item_RunSingleEventScript(localClientNum, dc, &it, handler->keyScript);
            return 1;
        }
    }
    for (i = 0; i < menu->itemCount; ++i)
    {
        v7 = menu->items[i];
        if (Item_IsFocusDefType(v7)
            && Window_IsVisible(dc->contextIndex, &v7->window)
            && (Window_HasFocus(dc->contextIndex, &v7->window) || (v7->window.staticFlags & 0x100000) != 0)
            && Item_IsVisible(localClientNum, dc->contextIndex, v7)
            && ((v7->dvarFlags & 3) == 0 || Item_EnableShowViaDvar(v7, 1)))
        {
            FocusItemDef = Item_GetFocusItemDef(menu->items[i]);
            if (FocusItemDef)
            {
                for (handler = FocusItemDef->onKey; handler; handler = handler->next)
                {
                    if (handler->key == key)
                        goto LABEL_22;
                }
            }
        }
    }
    return 0;
}

void    Menu_HandleKey(int localClientNum, UiContext *dc, menuDef_t *menu, int key, int down)
{
    int ControllerIndex; // eax
    const rectDef_s *v7; // eax
    const rectDef_s *v8; // eax
    const rectDef_s *Rect; // eax
    float x; // [esp+0h] [ebp-16Ch]
    float xa; // [esp+0h] [ebp-16Ch]
    float xb; // [esp+0h] [ebp-16Ch]
    float y; // [esp+4h] [ebp-168h]
    float ya; // [esp+4h] [ebp-168h]
    float yb; // [esp+4h] [ebp-168h]
    rectDef_s tmpRect; // [esp+14h] [ebp-158h] BYREF
    listBoxDef_s *listPtr; // [esp+2Ch] [ebp-140h]
    itemDef_s v18; // [esp+30h] [ebp-13Ch] BYREF
    int v19; // [esp+144h] [ebp-28h]
    int i; // [esp+148h] [ebp-24h]
    int staticFlags; // [esp+14Ch] [ebp-20h]
    const char *binding; // [esp+150h] [ebp-1Ch]
    int inHandler; // [esp+154h] [ebp-18h]
    itemDef_s *item; // [esp+158h] [ebp-14h]
    itemDef_s *overItem; // [esp+15Ch] [ebp-10h]

    overItem = 0;
    item = 0;
    inHandler = 0;
    if ((Window_GetDynamicFlags(dc->contextIndex, &menu->window) & 0xE2000) != 0
        || Menu_ItemsAreAnimating(localClientNum, dc, menu))
    {
        return;
    }
    inHandler = 1;
    if (g_waitingForKey && down)
    {
        Item_Bind_HandleKey(localClientNum, dc, g_bindItem, key, down);
        inHandler = 0;
        return;
    }
    if (!g_editingField || !down)
        goto LABEL_15;
    if (!Item_TextField_HandleKey(localClientNum, dc, g_editItem, key))
    {
        g_editingField = 0;
        g_editItem = 0;
        inHandler = 0;
        return;
    }
    if (key == 200 || key == 201 || key == 202)
    {
        g_editingField = 0;
        g_editItem = 0;
        Display_MouseMove(0, dc);
    LABEL_15:
        if (menu)
        {
            if (!down
                || (key & 0x400) != 0
                || (binding = Key_GetBinding(dc->contextIndex, key, 0)) == 0
                || (!menu->allowedBinding || I_stricmp(binding, menu->allowedBinding)) && I_stricmp(binding, "screenshotJpeg"))
            {
                if (!down
                    || (staticFlags = menu->window.staticFlags, (staticFlags & 0x1000000) != 0)
                    || menu->fullScreen
                    || (y = dc->cursor.y,
                        x = dc->cursor.x,
                        v7 = Window_GetRect(&menu->window),
                        Rect_ContainsPoint(dc->contextIndex, v7, x, y))
                    || inHandleKey
                    || key != 200 && key != 201 && key != 202)
                {
                    for (i = 0; i < menu->itemCount; ++i)
                    {
                        if (Item_IsVisible(localClientNum, dc->contextIndex, menu->items[i]))
                        {
                            if (Window_HasFocus(dc->contextIndex, &menu->items[i]->window))
                                item = menu->items[i];
                        }
                    }
                    if (item && Item_HandleKey(localClientNum, dc, item, key, down) && !Dvar_GetInt("ui_choice_noaction"))
                    {
                        Item_Action(localClientNum, dc, item);
                        inHandler = 0;
                    }
                    else if (down)
                    {
                        if ((key <= 0 || key > 255 || !Menu_CheckOnKey(localClientNum, dc, menu, key))
                            && (key != 205 && key != 206 || item && item->type == 4))
                        {
                            v19 = key - 1;
                            switch (key)
                            {
                            case 1:
                            case 13:
                            case 14:
                            case 191:
                            case 202:
                                if (item)
                                {
                                    if (!Item_IsTextField(item))
                                        goto LABEL_109;
                                    item->typeData.textDef->textTypeData.focusItemDef->focusTypeData.listBox->cursorPos[dc->contextIndex] = 0;
                                    g_editingField = 1;
                                    g_editItem = item;
                                    Key_SetOverstrikeMode(localClientNum, 1);
                                }
                                break;
                            case 2:
                            case 15:
                            case 27:
                                if (!g_waitingForKey && menu->onEvent)
                                {
                                    v18.parent = menu;
                                    v18.onEvent = menu->onEvent;
                                    Item_RunEventScript(localClientNum, dc, &v18, "onESC");
                                }
                                break;
                            case 154:
                            case 183:
                            case 206:
                                Menu_SetPrevCursorItem(localClientNum, dc, menu, 1);
                                break;
                            case 155:
                            case 189:
                            case 205:
                                Menu_SetNextCursorItem(localClientNum, dc, menu, 0, 1);
                                break;
                            case 177:
                                if (Dvar_GetInt("developer"))
                                    g_debugMode ^= 1u;
                                break;
                            case 178:
                                if (Dvar_GetInt("developer"))
                                    Cbuf_AddText(localClientNum, "screenshot\n");
                                break;
                            case 200:
                            case 201:
                                if (item)
                                {
                                    if (item->type == 1)
                                    {
                                        ya = dc->cursor.y;
                                        xa = dc->cursor.x;
                                        v8 = Item_CorrectedTextRect(dc->contextIndex, item);
                                        if (Rect_ContainsPoint(dc->contextIndex, v8, xa, ya))
                                            Item_Action(localClientNum, dc, item);
                                    }
                                    else
                                    {
                                        yb = dc->cursor.y;
                                        xb = dc->cursor.x;
                                        Rect = Window_GetRect(&item->window);
                                        if (Rect_ContainsPoint(dc->contextIndex, Rect, xb, yb))
                                        {
                                            if (Item_IsTextField(item))
                                            {
                                                Item_TextField_BeginEdit(localClientNum, dc->contextIndex, item);
                                            }
                                            else
                                            {
                                                if (item->type != 4)
                                                    goto LABEL_109;
                                                listPtr = Item_GetListBoxDef(item);
                                                if (!listPtr
                                                    && !Assert_MyHandler(
                                                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                                                        7954,
                                                        0,
                                                        "%s",
                                                        "listPtr"))
                                                {
                                                    __debugbreak();
                                                }
                                                if (!listPtr->noScrollBars
                                                    && Item_ListBox_RequiresScroll(localClientNum, dc->contextIndex, item))
                                                {
                                                    tmpRect = *Window_GetRect(&item->window);
                                                    tmpRect.w = tmpRect.w - 14.0;
                                                    if (Rect_ContainsPoint(dc->contextIndex, &tmpRect, dc->cursor.x, dc->cursor.y))
                                                        Item_Action(localClientNum, dc, item);
                                                }
                                                else
                                                {
                                                LABEL_109:
                                                    Item_Action(localClientNum, dc, item);
                                                }
                                            }
                                        }
                                        else if (Item_IsModal(item))
                                        {
                                            for (i = 0; i < menu->itemCount; ++i)
                                            {
                                                if (Item_IsVisible(localClientNum, dc->contextIndex, menu->items[i])
                                                    && (Window_GetDynamicFlags(dc->contextIndex, &menu->items[i]->window) & 1) != 0
                                                    && (menu->items[i]->window.staticFlags & 0x100000) == 0)
                                                {
                                                    overItem = menu->items[i];
                                                }
                                            }
                                            if (overItem
                                                && (overItem->window.staticFlags & 0x100000) == 0
                                                && Window_IsVisible(dc->contextIndex, &overItem->window))
                                            {
                                                if (overItem->type == 4)
                                                {
                                                    Item_SetFocus(
                                                        localClientNum,
                                                        dc,
                                                        overItem,
                                                        overItem->window.rect.x,
                                                        overItem->window.rect.y);
                                                    Item_ListBox_HandleKey(localClientNum, dc, overItem, key, down, 0);
                                                }
                                                else if (overItem->window.group
                                                    && *overItem->window.group
                                                    && item->window.group
                                                    && *item->window.group
                                                    && !I_strcmp(item->window.group, overItem->window.group))
                                                {
                                                    Item_Action(localClientNum, dc, overItem);
                                                }
                                                else
                                                {
                                                    Item_SetFocus(
                                                        localClientNum,
                                                        dc,
                                                        overItem,
                                                        overItem->window.rect.x,
                                                        overItem->window.rect.y);
                                                    Item_Action(localClientNum, dc, overItem);
                                                }
                                            }
                                            else
                                            {
                                                Item_LoseFocus(dc->contextIndex, dc, item);
                                            }
                                        }
                                    }
                                }
                                break;
                            default:
                                return;
                            }
                        }
                    }
                    else
                    {
                        inHandler = 0;
                    }
                }
                else
                {
                    inHandleKey = 1;
                    Menus_HandleOOBClick(localClientNum, dc, menu, key, down);
                    inHandleKey = 0;
                    inHandler = 0;
                }
            }
            else
            {
                ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
                Cbuf_ExecuteBuffer(dc->contextIndex, ControllerIndex, (char *)binding);
            }
        }
        else
        {
            inHandler = 0;
        }
    }
}

itemDef_s *__cdecl Item_LoseFocus(int localClientNum, UiContext *dc, itemDef_s *item)
{
    Window_RemoveDynamicFlags(dc->contextIndex, &item->window, 2);
    if ( Item_IsFocusDefType(item) )
        Item_RunEventScript(localClientNum, dc, item, "leaveFocus");
    return item;
}

bool __cdecl Item_IsModal(itemDef_s *item)
{
    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 849, 0, "%s", "item") )
        __debugbreak();
    return item->window.modal != 0;
}

int __cdecl Item_ListBox_HandleKey(int localClientNum, UiContext *dc, itemDef_s *item, int key, int down, int force)
{
    int result; // eax
    menuDef_t *v7; // eax
    menuDef_t *Focused; // eax
    int v9; // eax
    int special; // [esp+8h] [ebp-B8h]
    int v11; // [esp+Ch] [ebp-B4h]
    int v12; // [esp+10h] [ebp-B0h]
    int v13; // [esp+14h] [ebp-ACh]
    int v14; // [esp+18h] [ebp-A8h]
    int v15; // [esp+20h] [ebp-A0h]
    int v16; // [esp+28h] [ebp-98h]
    int v17; // [esp+2Ch] [ebp-94h]
    int v18; // [esp+34h] [ebp-8Ch]
    int v19; // [esp+38h] [ebp-88h]
    bool v20; // [esp+3Ch] [ebp-84h]
    int v22; // [esp+58h] [ebp-68h]
    int v23; // [esp+60h] [ebp-60h]
    int v24; // [esp+68h] [ebp-58h]
    int v25; // [esp+70h] [ebp-50h]
    rectDef_s tmpRect; // [esp+7Ch] [ebp-44h] BYREF
    int max; // [esp+94h] [ebp-2Ch]
    int delta; // [esp+98h] [ebp-28h]
    int validMousePosition; // [esp+9Ch] [ebp-24h]
    focusItemDef_s *focusPtr; // [esp+A0h] [ebp-20h]
    listBoxDef_s *listPtr; // [esp+A4h] [ebp-1Ch]
    int viewmax; // [esp+A8h] [ebp-18h]
    int flags; // [esp+ACh] [ebp-14h]
    const rectDef_s *rect; // [esp+B0h] [ebp-10h]
    int count; // [esp+B4h] [ebp-Ch]
    ItemKeyHandler *handler; // [esp+B8h] [ebp-8h]
    int lastIndex; // [esp+BCh] [ebp-4h]

    focusPtr = Item_GetFocusItemDef(item);
    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr )
        return 0;
    rect = Window_GetRect(&item->window);
    v20 = Item_IsModal(item) || Rect_ContainsPoint(dc->contextIndex, rect, dc->cursor.x, dc->cursor.y);
    validMousePosition = v20;
    count = (int)UI_FeederCount(localClientNum, dc->contextIndex, listPtr->special, listPtr);
    if ( count - 1 > 0 )
        v19 = count - 1;
    else
        v19 = 0;
    lastIndex = v19;
    flags = Window_GetDynamicFlags(dc->contextIndex, &item->window);
    if ( !force && (!validMousePosition || (flags & 2) == 0) )
        return 0;
    UI_OverrideCursorPos(localClientNum, dc->contextIndex, item);
    max = (int)Item_ListBox_MaxScroll(localClientNum, dc->contextIndex, item);
    viewmax = Item_ListBox_Viewmax(localClientNum, dc->contextIndex, item);
    if ( Window_IsHorizontal(&item->window) )
    {
        if ( key == 30 || key == 22 || key == 156 || key == 185 )
        {
            if ( listPtr->usePaging && key == 22 )
                v18 = viewmax;
            else
                v18 = 1;
            delta = v18;
            if ( listPtr->notselectable )
            {
                if ( listPtr->startPos[dc->contextIndex] - delta > 0 )
                    v17 = listPtr->startPos[dc->contextIndex] - delta;
                else
                    v17 = 0;
                listPtr->startPos[dc->contextIndex] = v17;
            }
            else
            {
                v25 = listPtr->cursorPos[dc->contextIndex] - delta;
                if ( v25 > 0 )
                    Item_ListBox_SetCursorPos(localClientNum, dc->contextIndex, item, max, viewmax, v25, 1);
                else
                    Item_ListBox_SetCursorPos(localClientNum, dc->contextIndex, item, max, viewmax, 0, 1);
            }
            return 1;
        }
        if ( key == 31 || key == 23 || key == 157 || key == 187 )
        {
            if ( listPtr->usePaging && key == 23 )
                v16 = viewmax;
            else
                v16 = 1;
            delta = v16;
            if ( listPtr->notselectable )
            {
                if ( delta + listPtr->startPos[dc->contextIndex] < max )
                    v15 = delta + listPtr->startPos[dc->contextIndex];
                else
                    v15 = max;
                listPtr->startPos[dc->contextIndex] = v15;
            }
            else
            {
                v24 = delta + listPtr->cursorPos[dc->contextIndex];
                if ( v24 < lastIndex )
                    Item_ListBox_SetCursorPos(localClientNum, dc->contextIndex, item, max, viewmax, v24, 1);
                else
                    Item_ListBox_SetCursorPos(localClientNum, dc->contextIndex, item, max, viewmax, lastIndex, 1);
            }
            return 1;
        }
LABEL_88:
        if ( key == 1 || key == 200 || key == 201 || key == 13 || key == 32 )
        {
            if ( (flags & 0x100) != 0 )
            {
                if ( listPtr->startPos[dc->contextIndex] - 1 > 0 )
                    v14 = listPtr->startPos[dc->contextIndex] - 1;
                else
                    v14 = 0;
                listPtr->startPos[dc->contextIndex] = v14;
                return 1;
            }
            if ( (flags & 0x200) != 0 )
            {
                if ( listPtr->startPos[dc->contextIndex] + 1 < max )
                    v13 = listPtr->startPos[dc->contextIndex] + 1;
                else
                    v13 = max;
                listPtr->startPos[dc->contextIndex] = v13;
                return 1;
            }
            if ( (flags & 0x800) != 0 )
            {
                if ( listPtr->startPos[dc->contextIndex] - viewmax > 0 )
                    v12 = listPtr->startPos[dc->contextIndex] - viewmax;
                else
                    v12 = 0;
                listPtr->startPos[dc->contextIndex] = v12;
                return 1;
            }
            if ( (flags & 0x1000) != 0 )
            {
                if ( viewmax + listPtr->startPos[dc->contextIndex] < max )
                    v11 = viewmax + listPtr->startPos[dc->contextIndex];
                else
                    v11 = max;
                listPtr->startPos[dc->contextIndex] = v11;
                return 1;
            }
            if ( (flags & 0x400) == 0 )
            {
                if ( listPtr->special == 24.0 )
                {
                    UI_OverrideCursorPos(localClientNum, dc->contextIndex, item);
                    UI_FeederSelection(localClientNum, dc->contextIndex, listPtr->special, listPtr->cursorPos[dc->contextIndex]);
                }
                if ( Item_IsModal(item) )
                {
                    if ( (key == 200 || key == 201) && !Rect_ContainsPoint(dc->contextIndex, rect, dc->cursor.x, dc->cursor.y) )
                        return 0;
                    if ( (key == 200 || key == 201)
                        && listPtr->mousePos >= 0
                        && (listPtr->cursorPos[dc->contextIndex] < 0
                         || listPtr->mousePos < (int)UI_FeederCount(localClientNum, dc->contextIndex, listPtr->special, listPtr)) )
                    {
                        listPtr->cursorPos[dc->contextIndex] = listPtr->mousePos;
                    }
                    special = (int)listPtr->special;
                    if ( special == 77 )
                    {
                        if ( !UI_FeederItemEnabled(
                                        localClientNum,
                                        dc->contextIndex,
                                        listPtr->special,
                                        listPtr->cursorPos[dc->contextIndex]) )
                            return 0;
                    }
                    else if ( special == 78 )
                    {
                        UI_FeederDoubleClick(
                            localClientNum,
                            dc->contextIndex,
                            listPtr->special,
                            (itemGroup_t)listPtr->cursorPos[dc->contextIndex]);
                    }
                    if ( key != 200 && key != 32 && key != 13 )
                    {
                        if ( key == 201 )
                            Item_RunEventScript(localClientNum, dc, item, "RightClick");
                        return 1;
                    }
                    goto LABEL_147;
                }
                if ( !UI_FeederItemEnabled(
                                localClientNum,
                                dc->contextIndex,
                                listPtr->special,
                                listPtr->cursorPos[dc->contextIndex]) )
                {
                    if ( listPtr->special == 1.0 )
                        UI_FeederDoubleClickDlcMaps(localClientNum, dc->contextIndex, listPtr->cursorPos[dc->contextIndex]);
                    return 1;
                }
                if ( !UI_FeederDoubleClick(
                                localClientNum,
                                dc->contextIndex,
                                listPtr->special,
                                (itemGroup_t)listPtr->cursorPos[dc->contextIndex]) )
                {
                    if ( key != 13 )
                    {
                        if ( dc->realTime < lastListBoxClickTime
                            && Item_DoesEventExist(item, "doubleClick")
                            && key == 200
                            && listPtr->cursorPos[dc->contextIndex] == listPtr->mousePos
                            && ListBox_HasValidCursorPos(dc->contextIndex, item) )
                        {
                            if ( listPtr->noScrollBars || !Item_ListBox_RequiresScroll(localClientNum, dc->contextIndex, item) )
                            {
                                Item_RunEventScript(localClientNum, dc, item, "doubleClick");
                            }
                            else
                            {
                                tmpRect = *Window_GetRect(&item->window);
                                tmpRect.w = tmpRect.w - 14.0;
                                if ( Rect_ContainsPoint(dc->contextIndex, &tmpRect, dc->cursor.x, dc->cursor.y) )
                                    Item_RunEventScript(localClientNum, dc, item, "doubleClick");
                            }
                        }
                        lastListBoxClickTime = dc->realTime + 300;
                        if ( listPtr->cursorPos[dc->contextIndex] != listPtr->mousePos )
                        {
                            if ( listPtr->mousePos >= 0 )
                            {
                                if ( listPtr->cursorPos[dc->contextIndex] < 0
                                    || (v9 = UI_FeederCount(localClientNum, dc->contextIndex, listPtr->special, listPtr),
                                            listPtr->mousePos < (int)v9) )
                                {
                                    listPtr->cursorPos[dc->contextIndex] = listPtr->mousePos;
                                }
                            }
                            UI_FeederSelection(
                                localClientNum,
                                dc->contextIndex,
                                listPtr->special,
                                listPtr->cursorPos[dc->contextIndex]);
                        }
                        if ( key == 201 )
                        {
                            Item_RunEventScript(localClientNum, dc, item, "RightClick");
                        }
                        else if ( key == 200 )
                        {
                            for ( handler = focusPtr->onKey; handler; handler = handler->next )
                            {
                                if ( handler->key == 200 )
                                {
                                    Item_RunSingleEventScript(localClientNum, dc, item, handler->keyScript);
                                    return 1;
                                }
                            }
                        }
                        return 1;
                    }
LABEL_147:
                    Item_RunEventScript(localClientNum, dc, item, "doubleClick");
                }
            }
            return 1;
        }
        switch ( key )
        {
            case 165:
            case 182:
                Script_FeederTop(localClientNum, dc, item, NULL);
                return 1;
            case 166:
            case 188:
                Script_FeederBottom(localClientNum, dc, item, NULL);
                return 1;
            case 164:
            case 184:
LABEL_53:
                Item_ListBox_Page(localClientNum, dc->contextIndex, item, lastIndex, max, viewmax, -viewmax);
                return 1;
            case 163:
            case 190:
LABEL_71:
                Item_ListBox_Page(localClientNum, dc->contextIndex, item, lastIndex, max, viewmax, viewmax);
                return 1;
            case 32:
            case 13:
                Item_RunEventScript(localClientNum, dc, item, "doubleClick");
                return 1;
        }
        for ( handler = focusPtr->onKey; handler; handler = handler->next )
        {
            if ( handler->key == key )
            {
                Item_RunSingleEventScript(localClientNum, dc, item, handler->keyScript);
                return 1;
            }
        }
        return 0;
    }
    if ( UI_OverrideKeyPress(localClientNum, dc->contextIndex, item, key) )
        return 1;
    switch ( key )
    {
        case 20:
            if ( listPtr->usePaging )
                goto LABEL_53;
            goto $LN80_1;
        case 21:
            if ( listPtr->usePaging )
                goto LABEL_71;
            goto $LN72_1;
        case 28:
        case 154:
        case 183:
$LN80_1:
            if ( lastIndex )
            {
                v23 = listPtr->cursorPos[dc->contextIndex] - 1;
                if ( lastIndex <= 0
                    && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 719, 0, "%s", "min < max") )
                {
                    __debugbreak();
                }
                if ( v23 >= 0 )
                {
                    if ( v23 <= lastIndex )
                        Item_ListBox_SetCursorPos(localClientNum, dc->contextIndex, item, max, viewmax, v23, 1);
                    else
                        Item_ListBox_SetCursorPos(localClientNum, dc->contextIndex, item, max, viewmax, lastIndex, 1);
                }
                else
                {
                    Item_ListBox_SetCursorPos(localClientNum, dc->contextIndex, item, max, viewmax, 0, 1);
                }
            }
            result = 1;
            break;
        case 29:
        case 155:
        case 189:
$LN72_1:
            if ( lastIndex )
            {
                v22 = listPtr->cursorPos[dc->contextIndex] + 1;
                if ( lastIndex <= 0
                    && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 719, 0, "%s", "min < max") )
                {
                    __debugbreak();
                }
                if ( v22 >= 0 )
                {
                    if ( v22 <= lastIndex )
                        Item_ListBox_SetCursorPos(localClientNum, dc->contextIndex, item, max, viewmax, v22, 1);
                    else
                        Item_ListBox_SetCursorPos(localClientNum, dc->contextIndex, item, max, viewmax, lastIndex, 1);
                }
                else
                {
                    Item_ListBox_SetCursorPos(localClientNum, dc->contextIndex, item, max, viewmax, 0, 1);
                }
            }
            result = 1;
            break;
        case 205:
            if ( Item_IsModal(item) )
            {
                if ( listPtr->usePaging )
                {
                    Item_ListBox_Page(localClientNum, dc->contextIndex, item, lastIndex, max, viewmax, 1);
                    Focused = Menu_GetFocused(dc);
                    Menu_HandleMouseMove(localClientNum, dc, Focused);
                }
            }
            else
            {
                Item_ListBox_Scroll(localClientNum, dc->contextIndex, item, lastIndex, max, viewmax, 1);
            }
            result = 1;
            break;
        case 206:
            if ( Item_IsModal(item) )
            {
                if ( listPtr->usePaging )
                {
                    Item_ListBox_Page(localClientNum, dc->contextIndex, item, lastIndex, max, viewmax, -1);
                    v7 = Menu_GetFocused(dc);
                    Menu_HandleMouseMove(localClientNum, dc, v7);
                }
            }
            else
            {
                Item_ListBox_Scroll(localClientNum, dc->contextIndex, item, lastIndex, max, viewmax, -1);
            }
            result = 1;
            break;
        default:
            goto LABEL_88;
    }
    return result;
}

void __cdecl Item_ListBox_Page(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                int max,
                int scrollmax,
                int viewmax,
                int delta)
{
    int v7; // [esp+4h] [ebp-18h]
    int v8; // [esp+Ch] [ebp-10h]
    int v9; // [esp+14h] [ebp-8h]
    listBoxDef_s *listPtr; // [esp+18h] [ebp-4h]

    if ( scrollmax )
    {
        listPtr = Item_GetListBoxDef(item);
        if ( !listPtr && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 5718, 0, "%s", "listPtr") )
            __debugbreak();
        if ( delta + listPtr->startPos[contextIndex] < scrollmax )
            v9 = delta + listPtr->startPos[contextIndex];
        else
            v9 = scrollmax;
        if ( v9 > 0 )
            v7 = v9;
        else
            v7 = 0;
        listPtr->startPos[contextIndex] = v7;
        if ( !listPtr->notselectable )
        {
            if ( delta + listPtr->cursorPos[contextIndex] < max )
                v8 = delta + listPtr->cursorPos[contextIndex];
            else
                v8 = max;
            if ( v8 > 0 )
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, item, max, viewmax, v8, 1);
            else
                Item_ListBox_SetCursorPos(localClientNum, contextIndex, item, max, viewmax, 0, 1);
        }
    }
}

bool __cdecl Item_TextField_HandleKey(int localClientNum, UiContext *dc, itemDef_s *item, int key)
{
    const char *VariantString; // eax
    const char *v6; // eax
    const char *v7; // eax
    int OverstrikeMode; // eax
    bool v9; // [esp+10h] [ebp-434h]
    int itemIndex; // [esp+24h] [ebp-420h]
    editFieldDef_s *editPtr; // [esp+28h] [ebp-41Ch]
    int len; // [esp+2Ch] [ebp-418h]
    itemDef_s *newItem; // [esp+30h] [ebp-414h]
    itemDef_s *newItema; // [esp+30h] [ebp-414h]
    itemDef_s *newItemb; // [esp+30h] [ebp-414h]
    itemDef_s *newItemc; // [esp+30h] [ebp-414h]
    int cursorPos; // [esp+34h] [ebp-410h]
    int cursorPosa; // [esp+34h] [ebp-410h]
    int cursorPosb; // [esp+34h] [ebp-410h]
    bool validInput; // [esp+3Bh] [ebp-409h] BYREF
    char buff[1024]; // [esp+3Ch] [ebp-408h] BYREF
    int memMoveCount; // [esp+440h] [ebp-4h]

    editPtr = Item_GetEditFieldDef(item);
    if ( !editPtr )
        return 0;
    if ( !item->dvar )
        return 0;
    memset(buff, 0, sizeof(buff));
    VariantString = Dvar_GetVariantString(item->dvar);
    I_strncpyz(buff, VariantString, 1024);
    len = &buff[strlen(buff) + 1] - &buff[1];
    if ( editPtr->maxChars )
    {
        if ( len > editPtr->maxChars )
        {
            len = editPtr->maxChars;
            buff[len] = 0;
            if ( editPtr->cursorPos[dc->contextIndex] > editPtr->maxChars )
                editPtr->cursorPos[dc->contextIndex] = editPtr->maxChars;
        }
    }
    if ( (key & 0x400) != 0 )
    {
        key &= ~0x400u;
        if ( key == 8 )
        {
            if ( editPtr->cursorPos[dc->contextIndex] > 0 )
            {
                cursorPos = editPtr->cursorPos[dc->contextIndex];
                memMoveCount = len + 1 - cursorPos;
                if ( (memMoveCount <= 0 || memMoveCount > len)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                                6562,
                                0,
                                "%s\n\t(memMoveCount) = %i",
                                "(memMoveCount > 0 && memMoveCount <= len)",
                                memMoveCount) )
                {
                    __debugbreak();
                }
                memmove((unsigned __int8 *)&buff[cursorPos - 1], (unsigned __int8 *)&buff[cursorPos], memMoveCount);
            }
            Dvar_SetFromStringByName(item->dvar, buff);
            v6 = Dvar_GetVariantString(item->dvar);
            I_strncpyz(buff, v6, 1024);
            editPtr->cursorPos[dc->contextIndex] = Item_GetCursorPosOffset(dc->contextIndex, item, buff, -1);
            Item_TextField_EnsureCursorVisible(dc->contextIndex, item, buff);
            return 1;
        }
        if ( item->type == 13 && !I_isforfilename(key) )
            return 1;
        if ( key < 32 || !item->dvar )
            return 1;
        if ( key == 64 )
            return 1;
        if ( key == 94 )
            return 1;
        if ( item->type == 7 && !I_isdigit(key) )
            return 0;
        if ( item->type == 22 && !I_isalnum(key) )
            return 1;
        if ( item->type == 30 )
        {
            v9 = I_isdigit(key) || key == Com_GetDecimalDelimiter();
            validInput = v9;
            if ( !v9 )
                return 0;
            if ( key == Com_GetDecimalDelimiter() )
                key = 46;
        }
        if ( item->type == 14 )
            key = toupper(key);
        if ( Dvar_GetBool("ui_ignoreMousePos") )
        {
            itemIndex = Menu_GetItemIndex(item->parent, item);
            if ( itemIndex >= 0 && itemIndex != Dvar_GetInt("ui_prevTextEntryBox") )
                Dvar_SetIntByName("ui_prevTextEntryBox", itemIndex);
        }
        if ( !Key_GetOverstrikeMode(localClientNum) )
        {
            if ( len == 255 || editPtr->maxChars && len >= editPtr->maxChars )
                return 1;
            cursorPosa = editPtr->cursorPos[dc->contextIndex];
            memMoveCount = len + 1 - cursorPosa;
            if ( (memMoveCount <= 0 || memMoveCount > len + 1)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                            6650,
                            0,
                            "%s\n\t(memMoveCount) = %i",
                            "(memMoveCount > 0 && memMoveCount <= (len + 1))",
                            memMoveCount) )
            {
                __debugbreak();
            }
            memmove((unsigned __int8 *)&buff[cursorPosa + 1], (unsigned __int8 *)&buff[cursorPosa], memMoveCount);
            goto LABEL_65;
        }
        if ( !editPtr->maxChars || editPtr->cursorPos[dc->contextIndex] < editPtr->maxChars )
        {
LABEL_65:
            buff[editPtr->cursorPos[dc->contextIndex]] = key;
            Dvar_SetFromStringByName(item->dvar, buff);
            v7 = Dvar_GetVariantString(item->dvar);
            I_strncpyz(buff, v7, 1024);
            strlen(buff);
            editPtr->cursorPos[dc->contextIndex] = Item_GetCursorPosOffset(dc->contextIndex, item, buff, 1);
            Item_TextField_EnsureCursorVisible(dc->contextIndex, item, buff);
            if ( editPtr->maxChars && editPtr->cursorPos[dc->contextIndex] >= editPtr->maxChars && editPtr->maxCharsGotoNext )
            {
                newItema = Menu_SetNextCursorItem(localClientNum, dc, item->parent, 0, 0);
                if ( newItema != item )
                    newItema->typeData.textDef->textTypeData.focusItemDef->focusTypeData.listBox->cursorPos[dc->contextIndex] = 0;
                if ( newItema && Item_IsTextField(newItema) )
                    g_editItem = newItema;
            }
LABEL_98:
            if ( key == 9 || key == 155 || key == 189 )
            {
                newItemb = Menu_SetNextCursorItem(localClientNum, dc, item->parent, 1, 0);
                if ( newItemb )
                {
                    if ( Item_IsTextField(newItemb) )
                        g_editItem = newItemb;
                }
            }
            if ( key == 154 || key == 183 )
            {
                newItemc = Menu_SetPrevCursorItem(localClientNum, dc, item->parent, 0);
                if ( newItemc )
                {
                    if ( Item_IsTextField(newItemc) )
                        g_editItem = newItemc;
                }
            }
            if ( key == 13 || key == 191 )
                Item_RunEventScript(localClientNum, dc, item, "onAccept");
            return key != 13 && key != 191 && key != 27
                    || item && item->window.name && !I_strcmp(item->window.name, "chat_text_input");
        }
        if ( editPtr->maxCharsGotoNext )
        {
            newItem = Menu_SetNextCursorItem(localClientNum, dc, item->parent, 0, 0);
            if ( newItem )
            {
                if ( Item_IsTextField(newItem) )
                    g_editItem = newItem;
            }
        }
        return 1;
    }
    else
    {
        if ( key != 162 && key != 193 )
        {
            switch ( key )
            {
                case 157:
                case 187:
                    editPtr->cursorPos[dc->contextIndex] = Item_GetCursorPosOffset(dc->contextIndex, item, buff, 1);
                    Item_TextField_EnsureCursorVisible(dc->contextIndex, item, buff);
                    return 1;
                case 156:
                case 185:
                    editPtr->cursorPos[dc->contextIndex] = Item_GetCursorPosOffset(dc->contextIndex, item, buff, -1);
                    Item_TextField_EnsureCursorVisible(dc->contextIndex, item, buff);
                    return 1;
                case 165:
                case 182:
                    editPtr->cursorPos[dc->contextIndex] = 0;
                    editPtr->paintOffset = 0;
                    return 1;
                case 166:
                case 188:
                    editPtr->cursorPos[dc->contextIndex] = len;
                    Item_TextField_EnsureCursorVisible(dc->contextIndex, item, buff);
                    return 1;
                case 161:
                case 192:
                    OverstrikeMode = Key_GetOverstrikeMode(dc->contextIndex);
                    Key_SetOverstrikeMode(dc->contextIndex, OverstrikeMode == 0);
                    return 1;
            }
            goto LABEL_98;
        }
        if ( editPtr->cursorPos[dc->contextIndex] < len )
        {
            cursorPosb = editPtr->cursorPos[dc->contextIndex];
            memMoveCount = len - cursorPosb;
            if ( (len - cursorPosb <= 0 || memMoveCount > len)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                            6702,
                            0,
                            "%s\n\t(memMoveCount) = %i",
                            "(memMoveCount > 0 && memMoveCount <= len)",
                            memMoveCount) )
            {
                __debugbreak();
            }
            memmove((unsigned __int8 *)&buff[cursorPosb], (unsigned __int8 *)&buff[cursorPosb + 1], memMoveCount);
            Dvar_SetFromStringByName(item->dvar, buff);
        }
        return 1;
    }
}

int __cdecl Menu_GetItemIndex(menuDef_t *menu, itemDef_s *item)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < menu->itemCount; ++i )
    {
        if ( menu->items[i] == item )
            return i;
    }
    return -1;
}

void __cdecl Item_TextField_EnsureCursorVisible(int contextIndex, itemDef_s *item, const char *text)
{
    editFieldDef_s *editPtr; // [esp+0h] [ebp-Ch]
    int paintOffsetMin; // [esp+4h] [ebp-8h]
    int cursorPos; // [esp+8h] [ebp-4h]

    editPtr = Item_GetEditFieldDef(item);
    cursorPos = editPtr->cursorPos[contextIndex];
    if ( editPtr->paintOffset <= cursorPos )
    {
        if ( editPtr->maxPaintChars )
        {
            paintOffsetMin = Item_GetCursorPosOffset(contextIndex, item, text, -editPtr->maxPaintChars);
            if ( editPtr->paintOffset < paintOffsetMin )
                editPtr->paintOffset = paintOffsetMin;
        }
    }
    else
    {
        editPtr->paintOffset = cursorPos;
    }
}

int __cdecl Item_HandleKey(int localClientNum, UiContext *dc, itemDef_s *item, int key, int down)
{
    int result; // eax

    if ( itemCapture )
    {
        itemCapture = 0;
        captureFunc = 0;
        captureData = 0;
    }
    else if ( down && (key == 200 || key == 201 || key == 202) )
    {
        Item_StartCapture(localClientNum, dc, item, key);
    }
    if ( !down )
        return 0;
    switch ( item->type )
    {
        case 3:
            result = 0;
            break;
        case 4:
            result = Item_ListBox_HandleKey(localClientNum, dc, item, key, down, 0);
            break;
        case 5:
        case 7:
        case 0xD:
        case 0xE:
        case 0x16:
        case 0x1E:
            result = 0;
            break;
        case 6:
        case 0x14:
            result = Item_OwnerDraw_HandleKey(item, key);
            break;
        case 8:
            result = Item_Slider_HandleKey(dc, item, key);
            break;
        case 9:
            result = Item_YesNo_HandleKey(dc, item, key);
            break;
        case 0xA:
            result = Item_Multi_HandleKey(dc, item, key);
            break;
        case 0xB:
            result = Item_DvarEnum_HandleKey(dc, item, key);
            break;
        case 0xC:
        case 0x10:
            result = Item_Bind_HandleKey(localClientNum, dc, item, key, down);
            break;
        case 0x19:
            result = 0;
            break;
        case 0x1B:
            result = 0;
            break;
        case 0x1C:
            result = 0;
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

int __cdecl Item_OwnerDraw_HandleKey(itemDef_s *item, int key)
{
    return UI_OwnerDrawHandleKey(item->window.ownerDraw, item->window.ownerDrawFlags, key);
}

int __cdecl Item_YesNo_HandleKey(UiContext *dc, itemDef_s *item, int key)
{
    const char *VariantString; // eax
    int v5; // eax
    char *v6; // eax
    bool v7; // [esp+0h] [ebp-28h]
    char v8; // [esp+7h] [ebp-21h]
    char dvarString[28]; // [esp+8h] [ebp-20h] BYREF

    if ( !item->dvar )
        return 0;
    if ( !Item_ShouldHandleKey(dc, item, key) )
        return 0;
    if ( key == 200 || key == 201 || key == 202 )
    {
        v8 = 1;
    }
    else
    {
        v7 = key == 13 || key == 156 || key == 157 || key == 164 || key == 163;
        v8 = v7;
    }
    if ( !v8 )
        return 0;
    VariantString = Dvar_GetVariantString(item->dvar);
    I_strncpyz(dvarString, VariantString, 25);
    v5 = atoi(dvarString);
    v6 = va("%i", v5 == 0);
    Dvar_SetFromStringByName(item->dvar, v6);
    return 1;
}

bool __cdecl Item_ShouldHandleKey(UiContext *dc, itemDef_s *item, int key)
{
    if ( !Window_HasFocus(dc->contextIndex, &item->window) )
        return 0;
    return key != 200 && key != 201 && key != 202 || Item_ContainsMouse(dc, item);
}

bool __cdecl Item_ContainsMouse(UiContext *dc, itemDef_s *item)
{
    const rectDef_s *Rect; // eax
    float x; // [esp+0h] [ebp-Ch]
    float y; // [esp+4h] [ebp-8h]
    int v6; // [esp+8h] [ebp-4h]

    v6 = 0;
    if ( dc->isCursorVisible )
    {
        y = dc->cursor.y;
        x = dc->cursor.x;
        Rect = Window_GetRect(&item->window);
        if ( Rect_ContainsPoint(dc->contextIndex, Rect, x, y) )
            return 1;
    }
    return v6;
}

int __cdecl Item_Multi_HandleKey(UiContext *dc, itemDef_s *item, int key)
{
    char *v4; // eax
    multiDef_s *multiPtr; // [esp+8h] [ebp-10h]
    int next; // [esp+Ch] [ebp-Ch]
    int count; // [esp+10h] [ebp-8h]
    int current; // [esp+14h] [ebp-4h]

    if ( !dc && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 6392, 0, "%s", "dc") )
        __debugbreak();
    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 6393, 0, "%s", "item") )
        __debugbreak();
    if ( !item->dvar
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 6394, 0, "%s", "item->dvar") )
    {
        __debugbreak();
    }
    multiPtr = Item_GetMultiDef(item);
    if ( !multiPtr )
        return 0;
    if ( !Item_ShouldHandleKey(dc, item, key) )
        return 0;
    current = Item_Multi_FindDvarByValue(item);
    count = Item_Multi_CountSettings(item);
    next = Item_List_NextEntryForKey(key, current, count);
    if ( next == current )
        return 0;
    if ( multiPtr->actionOnEnterPressOnly )
        return 0;
    while ( !multiPtr->dvarList[next] || !*multiPtr->dvarList[next] )
        next = Item_List_NextEntryForKey(key, next, count);
    if ( multiPtr->strDef )
    {
        Dvar_SetFromStringByName(item->dvar, (char *)multiPtr->dvarStr[next]);
    }
    else
    {
        v4 = va("%g", multiPtr->dvarValue[next]);
        Dvar_SetFromStringByName(item->dvar, v4);
    }
    return 1;
}

int __cdecl Item_Multi_CountSettings(itemDef_s *item)
{
    multiDef_s *multiPtr; // [esp+0h] [ebp-4h]

    multiPtr = Item_GetMultiDef(item);
    if ( multiPtr )
        return multiPtr->count;
    else
        return 0;
}

int __cdecl Item_Multi_FindDvarByValue(itemDef_s *item)
{
    const char *VariantString; // eax
    multiDef_s *multiPtr; // [esp+0h] [ebp-10h]
    const char *string; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    int ia; // [esp+8h] [ebp-8h]
    float value; // [esp+Ch] [ebp-4h]

    multiPtr = Item_GetMultiDef(item);
    if ( !multiPtr )
        return 0;
    if ( multiPtr->strDef )
    {
        string = Dvar_GetVariantString(item->dvar);
        for ( i = 0; i < multiPtr->count; ++i )
        {
            if ( !I_stricmp(string, multiPtr->dvarStr[i]) )
                return i;
        }
    }
    else
    {
        VariantString = Dvar_GetVariantString(item->dvar);
        value = atof(VariantString);
        for ( ia = 0; ia < multiPtr->count; ++ia )
        {
            if ( multiPtr->dvarValue[ia] == value )
                return ia;
        }
    }
    return 0;
}

int __cdecl Item_List_NextEntryForKey(int key, int current, int count)
{
    bool v4; // [esp+0h] [ebp-Ch]
    bool v5; // [esp+4h] [ebp-8h]
    char v6; // [esp+Ah] [ebp-2h]
    char v7; // [esp+Bh] [ebp-1h]

    if ( count < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    6373,
                    0,
                    "%s\n\t(count) = %i",
                    "(count >= 0)",
                    count) )
    {
        __debugbreak();
    }
    if ( (current < 0 || current >= count && count)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    6374,
                    0,
                    "%s\n\t(current) = %i",
                    "(current >= 0 && (current < count || count == 0))",
                    current) )
    {
        __debugbreak();
    }
    if ( !count )
        return 0;
    if ( key == 200 || key == 202 )
    {
        v7 = 1;
    }
    else
    {
        v5 = key == 13 || key == 163 || key == 157 || key == 31 || key == 23;
        v7 = v5;
    }
    if ( v7 )
        return (current + 1) % count;
    if ( key == 201 )
    {
        v6 = 1;
    }
    else
    {
        v4 = key == 164 || key == 156 || key == 30 || key == 22;
        v6 = v4;
    }
    if ( v6 )
        return (current + count - 1) % count;
    else
        return current;
}

int __cdecl Item_DvarEnum_HandleKey(UiContext *dc, itemDef_s *item, int key)
{
    char *v4; // eax
    int next; // [esp+0h] [ebp-Ch]
    int count; // [esp+4h] [ebp-8h]
    int current; // [esp+8h] [ebp-4h]

    if ( !dc && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 6431, 0, "%s", "dc") )
        __debugbreak();
    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 6432, 0, "%s", "item") )
        __debugbreak();
    if ( !item->dvar
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 6433, 0, "%s", "item->dvar") )
    {
        __debugbreak();
    }
    if ( !Item_ShouldHandleKey(dc, item, key) )
        return 0;
    current = Item_DvarEnum_EnumIndex(item);
    count = Item_DvarEnum_CountSettings(item);
    next = Item_List_NextEntryForKey(key, current, count);
    if ( next == current )
        return 0;
    v4 = va("%i", next);
    Dvar_SetFromStringByName(item->dvar, v4);
    return 1;
}

int __cdecl Item_DvarEnum_CountSettings(itemDef_s *item)
{
    enumDvarDef_s *enumDvarPtr; // [esp+0h] [ebp-8h]
    const dvar_s *enumDvar; // [esp+4h] [ebp-4h]

    enumDvarPtr = Item_GetEnumDvarName(item);
    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 6221, 0, "%s", "item") )
        __debugbreak();
    if ( item->type != 11
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    6222,
                    0,
                    "%s\n\t(item->type) = %i",
                    "(item->type == 11)",
                    item->type) )
    {
        __debugbreak();
    }
    if ( !enumDvarPtr
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 6223, 0, "%s", "enumDvarPtr") )
    {
        __debugbreak();
    }
    enumDvar = Dvar_FindVar(enumDvarPtr->enumDvarName);
    if ( enumDvar->type == DVAR_TYPE_ENUM )
        return enumDvar->domain.enumeration.stringCount;
    else
        return 0;
}

int __cdecl Item_DvarEnum_EnumIndex(itemDef_s *item)
{
    enumDvarDef_s *enumDvarPtr; // [esp+0h] [ebp-10h]
    int enumIndex; // [esp+4h] [ebp-Ch]
    int enumIndexa; // [esp+4h] [ebp-Ch]
    const dvar_s *enumDvar; // [esp+8h] [ebp-8h]
    const char *enumString; // [esp+Ch] [ebp-4h]

    enumDvarPtr = Item_GetEnumDvarName(item);
    enumDvar = Dvar_FindVar(enumDvarPtr->enumDvarName);
    if ( enumDvar->type != DVAR_TYPE_ENUM )
        return 0;
    enumString = Dvar_GetVariantString(item->dvar);
    if ( isdigit(*enumString) )
    {
        enumIndex = atoi(enumString);
        if ( enumIndex >= 0 && enumIndex < enumDvar->domain.enumeration.stringCount )
            return enumIndex;
    }
    for ( enumIndexa = 0; enumIndexa < enumDvar->domain.enumeration.stringCount; ++enumIndexa )
    {
        if ( !I_stricmp(enumString, *(const char **)(enumDvar->domain.integer.max + 4 * enumIndexa)) )
            return enumIndexa;
    }
    return 0;
}

void __cdecl Item_StartCapture(int localClientNum, UiContext *dc, itemDef_s *item, int key)
{
    int type; // [esp+8h] [ebp-8h]
    __int16 flags; // [esp+Ch] [ebp-4h]

    type = item->type;
    if ( type == 4 )
    {
        flags = Item_ListBox_OverLB(localClientNum, dc->contextIndex, item, dc->cursor.x, dc->cursor.y);
        if ( (flags & 0x300) != 0 )
        {
            scrollInfo.nextScrollTime = dc->realTime + 500;
            scrollInfo.nextAdjustTime = dc->realTime + 150;
            scrollInfo.adjustValue = 500;
            scrollInfo.scrollKey = key;
            scrollInfo.scrollDir = (flags & 0x100) != 0;
            scrollInfo.item = item;
            captureData = &scrollInfo;
            captureFunc = Scroll_ListBox_AutoFunc;
            itemCapture = item;
        }
        else if ( (flags & 0x400) != 0 )
        {
            scrollInfo.scrollKey = key;
            scrollInfo.item = item;
            *(cursor_t *)&scrollInfo.xStart = dc->cursor;
            captureData = &scrollInfo;
            captureFunc = (void (__cdecl *)(int, UiContext *, void *))Scroll_ListBox_ThumbFunc;
            itemCapture = item;
        }
    }
    else if ( type == 8 && (Item_Slider_OverSlider(dc->contextIndex, item, dc->cursor.x, dc->cursor.y) & 0x400) != 0 )
    {
        scrollInfo.scrollKey = key;
        scrollInfo.item = item;
        *(cursor_t *)&scrollInfo.xStart = dc->cursor;
        captureData = &scrollInfo;
        captureFunc = (void (__cdecl *)(int, UiContext *, void *))Scroll_Slider_ThumbFunc;
        itemCapture = item;
    }
}

int __cdecl Item_Slider_OverSlider(int contextIndex, itemDef_s *item, float x, float y)
{
    rectDef_s r; // [esp+8h] [ebp-1Ch] BYREF
    const rectDef_s *rect; // [esp+20h] [ebp-4h]

    rect = Window_GetRect(&item->window);
    r.x = Item_Slider_ThumbPosition(item) - 5.0;
    r.y = rect->y - 2.0;
    r.w = 10.0f;
    r.h = 20.0f;
    r.horzAlign = rect->horzAlign;
    r.vertAlign = rect->vertAlign;
    if ( Rect_ContainsPoint(contextIndex, &r, x, y) )
        return 1024;
    else
        return 0;
}

double __cdecl Item_Slider_ThumbPosition(itemDef_s *item)
{
    const char *VariantString; // eax
    editFieldDef_s *editDef; // [esp+10h] [ebp-14h]
    textDef_s *textDefPtr; // [esp+14h] [ebp-10h]
    float x; // [esp+18h] [ebp-Ch]
    const rectDef_s *rect; // [esp+1Ch] [ebp-8h]
    float value; // [esp+20h] [ebp-4h]

    editDef = Item_GetEditFieldDef(item);
    textDefPtr = Item_GetTextDef(item);
    if ( !editDef )
        return 0.0;
    rect = Window_GetRect(&item->window);
    x = Item_GetRectPlacementX(textDefPtr->textAlignMode & 3, rect->x + textDefPtr->textalignx, rect->w, 100.0);
    VariantString = Dvar_GetVariantString(item->dvar);
    value = atof(VariantString);
    if ( editDef->minVal <= value )
    {
        if ( value > editDef->maxVal )
            value = editDef->maxVal;
    }
    else
    {
        value = editDef->minVal;
    }
    return (float)(x
                             + (float)((float)((float)(value - editDef->minVal) / (float)(editDef->maxVal - editDef->minVal)) * 100.0));
}

double __cdecl Item_GetRectPlacementX(int alignX, float x0, float containerWidth, float selfWidth)
{
    if ( !alignX )
        return x0;
    if ( alignX == 1 )
        return (containerWidth - selfWidth) * 0.5 + x0;
    if ( alignX != 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    231,
                    0,
                    "alignX == ITEM_ALIGN_RIGHT\n\t%i, %i",
                    alignX,
                    2) )
    {
        __debugbreak();
    }
    return containerWidth - selfWidth + x0;
}

int __cdecl Item_ListBox_OverLB(int localClientNum, int contextIndex, itemDef_s *item, float x, float y)
{
    float thumbstart; // [esp+Ch] [ebp-2Ch]
    float thumbstarta; // [esp+Ch] [ebp-2Ch]
    rectDef_s r; // [esp+10h] [ebp-28h] BYREF
    listBoxDef_s *listPtr; // [esp+28h] [ebp-10h]
    float listHeight; // [esp+2Ch] [ebp-Ch]
    const rectDef_s *rect; // [esp+30h] [ebp-8h]
    int count; // [esp+34h] [ebp-4h]

    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 5219, 0, "%s", "item") )
        __debugbreak();
    listPtr = Item_GetListBoxDef(item);
    count = (int)UI_FeederCount(localClientNum, contextIndex, listPtr->special, listPtr);
    if ( !listPtr || !Item_ListBox_RequiresScroll(localClientNum, contextIndex, item) )
        return 0;
    rect = Window_GetRect(&item->window);
    r.horzAlign = rect->horzAlign;
    r.vertAlign = rect->vertAlign;
    if ( Window_IsHorizontal(&item->window) )
    {
        r.x = rect->x;
        r.y = (float)(rect->y + rect->h) - 16.0;
        r.w = 16.0f;
        r.h = 16.0f;
        if ( Rect_ContainsPoint(contextIndex, &r, x, y) )
            return 256;
        r.x = (float)(rect->x + rect->w) - 16.0;
        if ( Rect_ContainsPoint(contextIndex, &r, x, y) )
            return 512;
        thumbstart = Item_ListBox_ThumbPosition(localClientNum, contextIndex, item);
        r.x = thumbstart;
        if ( Rect_ContainsPoint(contextIndex, &r, x, y) )
            return 1024;
        r.x = rect->x + 16.0;
        r.w = thumbstart - r.x;
        if ( Rect_ContainsPoint(contextIndex, &r, x, y) )
            return 2048;
        r.x = thumbstart + 16.0;
        r.w = (float)(rect->x + rect->w) - 16.0;
        if ( Rect_ContainsPoint(contextIndex, &r, x, y) )
            return 4096;
    }
    else
    {
        listHeight = Item_ListBox_ScrollHeight(localClientNum, contextIndex, item);
        r.x = (float)(rect->x + rect->w) - 16.0;
        r.y = rect->y + 3.0;
        r.w = 16.0f;
        r.h = 16.0f;
        if ( Rect_ContainsPoint(contextIndex, &r, x, y) )
            return 256;
        r.y = (float)((float)(rect->y + 3.0) + listHeight) - 16.0;
        if ( Rect_ContainsPoint(contextIndex, &r, x, y) )
            return 512;
        thumbstarta = Item_ListBox_ThumbPosition(localClientNum, contextIndex, item);
        r.y = thumbstarta;
        if ( Rect_ContainsPoint(contextIndex, &r, x, y) )
            return 1024;
        r.y = rect->y + 16.0;
        r.h = thumbstarta - r.y;
        if ( Rect_ContainsPoint(contextIndex, &r, x, y) )
            return 2048;
        r.y = thumbstarta + 16.0;
        r.h = (float)(rect->y + listHeight) - 16.0;
        if ( Rect_ContainsPoint(contextIndex, &r, x, y) )
            return 4096;
    }
    return 0;
}

double __cdecl Item_ListBox_ScrollHeight(int localClientNum, int contextIndex, itemDef_s *item)
{
    listBoxDef_s *listPtr; // [esp+4h] [ebp-4h]

    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 5097, 0, "%s", "listPtr") )
        __debugbreak();
    return (double)Item_ListBox_Viewmax(localClientNum, contextIndex, item) * listPtr->elementHeight - 2.0;
}

double __cdecl Item_ListBox_ThumbPosition(int localClientNum, int contextIndex, itemDef_s *item)
{
    const char *max; // [esp+4h] [ebp-18h]
    float pos; // [esp+8h] [ebp-14h]
    float posa; // [esp+8h] [ebp-14h]
    listBoxDef_s *listPtr; // [esp+Ch] [ebp-10h]
    float scrollHeight; // [esp+14h] [ebp-8h]
    const rectDef_s *rect; // [esp+18h] [ebp-4h]

    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr )
        return 0.0;
    max = Item_ListBox_MaxScroll(localClientNum, contextIndex, item);
    rect = Window_GetRect(&item->window);
    if ( Window_IsHorizontal(&item->window) )
    {
        if ( (int)max <= 0 )
            pos = 0.0f;
        else
            pos = (float)((float)((float)(rect->w - 32.0) - 2.0) - 16.0) / (float)(int)max;
        return rect->x + 1.0 + 16.0 + (float)((float)listPtr->startPos[contextIndex] * pos);
    }
    else
    {
        scrollHeight = Item_ListBox_ScrollHeight(localClientNum, contextIndex, item);
        if ( (int)max <= 0 )
            posa = 0.0f;
        else
            posa = (float)((float)(scrollHeight - 32.0) - 16.0) / (float)(int)max;
        return rect->y + 3.0 + 16.0 + (float)((float)listPtr->startPos[contextIndex] * posa);
    }
}

void __cdecl Scroll_ListBox_AutoFunc(int localClientNum, UiContext *dc, void *p)
{
    if ( dc->realTime > *(unsigned int *)p )
    {
        Item_ListBox_HandleKey(0, dc, *((itemDef_s **)p + 6), *((unsigned int *)p + 3), 1, 0);
        *(unsigned int *)p = *((unsigned int *)p + 2) + dc->realTime;
    }
    if ( dc->realTime > *((unsigned int *)p + 1) )
    {
        *((unsigned int *)p + 1) = dc->realTime + 150;
        if ( *((int *)p + 2) > 20 )
            *((unsigned int *)p + 2) -= 40;
    }
}

void __cdecl Scroll_ListBox_ThumbFunc(int localClientNum, UiContext *dc, scrollInfo_s *p)
{
    int pos; // [esp+Ch] [ebp-34h]
    int posa; // [esp+Ch] [ebp-34h]
    const char *max; // [esp+10h] [ebp-30h]
    const char *maxa; // [esp+10h] [ebp-30h]
    rectDef_s r; // [esp+14h] [ebp-2Ch] BYREF
    scrollInfo_s *v8; // [esp+2Ch] [ebp-14h]
    listBoxDef_s *listPtr; // [esp+30h] [ebp-10h]
    const rectDef_s *rect; // [esp+34h] [ebp-Ch]
    float x; // [esp+38h] [ebp-8h]
    float y; // [esp+3Ch] [ebp-4h]

    if ( dc->isCursorVisible )
    {
        v8 = p;
        listPtr = Item_GetListBoxDef(p->item);
        if ( listPtr )
        {
            if ( Window_IsHorizontal(&v8->item->window) )
            {
                if ( dc->cursor.x == v8->xStart )
                    return;
                rect = Window_GetRect(&v8->item->window);
                r.x = rect->x;
                r.y = rect->y;
                r.w = rect->w;
                r.h = rect->h;
                x = ScrPlace_ApplyX(&scrPlaceView[localClientNum], dc->cursor.x, 4);
                y = ScrPlace_ApplyY(&scrPlaceView[localClientNum], dc->cursor.y, 4);
                ScrPlace_ApplyRect(&scrPlaceView[localClientNum], &r.x, &r.y, &r.w, &r.h, rect->horzAlign, rect->vertAlign);
                r.x = (float)(r.x + 16.0) + 1.0;
                r.y = (float)((float)(r.y + r.h) - 16.0) - 1.0;
                r.h = 16.0f;
                r.w = (float)(r.w - 32.0) - 2.0;
                max = Item_ListBox_MaxScroll(localClientNum, dc->contextIndex, v8->item);
                pos = (int)(float)((float)((float)(x - r.x) / r.w) * (float)(int)max);
                if ( pos >= 0 )
                {
                    if ( pos > (int)max )
                        pos = (int)max;
                }
                else
                {
                    pos = 0;
                }
                listPtr->startPos[dc->contextIndex] = pos;
                v8->xStart = dc->cursor.x;
            }
            else if ( dc->cursor.y != v8->yStart )
            {
                rect = Window_GetRect(&v8->item->window);
                r.x = rect->x;
                r.y = rect->y;
                r.w = rect->w;
                r.h = Item_ListBox_ScrollHeight(localClientNum, dc->contextIndex, v8->item);
                x = ScrPlace_ApplyX(&scrPlaceView[localClientNum], dc->cursor.x, 4);
                y = ScrPlace_ApplyY(&scrPlaceView[localClientNum], dc->cursor.y, 4);
                ScrPlace_ApplyRect(&scrPlaceView[localClientNum], &r.x, &r.y, &r.w, &r.h, rect->horzAlign, rect->vertAlign);
                r.x = (float)((float)(r.x + r.w) - 16.0) - 1.0;
                r.y = (float)(r.y + 16.0) + 1.0;
                r.h = (float)(r.h - 32.0) - 2.0;
                r.w = 16.0f;
                maxa = Item_ListBox_MaxScroll(localClientNum, dc->contextIndex, v8->item);
                posa = (int)(float)((float)((float)(y - r.y) / r.h) * (float)(int)maxa);
                if ( posa >= 0 )
                {
                    if ( posa > (int)maxa )
                        posa = (int)maxa;
                }
                else
                {
                    posa = 0;
                }
                listPtr->startPos[dc->contextIndex] = posa;
                v8->yStart = dc->cursor.y;
            }
            if ( dc->realTime > v8->nextScrollTime )
            {
                Item_ListBox_HandleKey(0, dc, v8->item, v8->scrollKey, 1, 0);
                v8->nextScrollTime = v8->adjustValue + dc->realTime;
            }
            if ( dc->realTime > v8->nextAdjustTime )
            {
                v8->nextAdjustTime = dc->realTime + 150;
                if ( v8->adjustValue > 20 )
                    v8->adjustValue -= 40;
            }
        }
    }
}

void __cdecl Scroll_Slider_ThumbFunc(int localClientNum, UiContext *dc, itemDef_s **p)
{
    Scroll_Slider_SetThumbPos(dc, p[6]);
}

void __cdecl Scroll_Slider_SetThumbPos(UiContext *dc, itemDef_s *item)
{
    char *v2; // eax
    float v3; // [esp+Ch] [ebp-38h]
    float v4; // [esp+14h] [ebp-30h]
    float hIgnored; // [esp+18h] [ebp-2Ch] BYREF
    float yIgnored; // [esp+1Ch] [ebp-28h] BYREF
    const ScreenPlacement *scrPlace; // [esp+20h] [ebp-24h]
    float usableStart; // [esp+24h] [ebp-20h] BYREF
    editFieldDef_s *editDef; // [esp+28h] [ebp-1Ch]
    textDef_s *textDefPtr; // [esp+2Ch] [ebp-18h]
    float cursorx; // [esp+30h] [ebp-14h]
    float usableWidth; // [esp+34h] [ebp-10h] BYREF
    float x; // [esp+38h] [ebp-Ch]
    const rectDef_s *rect; // [esp+3Ch] [ebp-8h]
    float value; // [esp+40h] [ebp-4h]

    editDef = Item_GetEditFieldDef(item);
    textDefPtr = Item_GetTextDef(item);
    if ( editDef )
    {
        rect = Window_GetRect(&item->window);
        x = Item_GetRectPlacementX(textDefPtr->textAlignMode & 3, rect->x + textDefPtr->textalignx, rect->w, 100.0);
        scrPlace = &scrPlaceView[dc->contextIndex];
        cursorx = ScrPlace_ApplyX(scrPlace, dc->cursor.x, 4);
        usableStart = (float)(x + 5.0) + 1.0;
        usableWidth = 88.0f;
        yIgnored = 0.0f;
        hIgnored = 0.0f;
        ScrPlace_ApplyRect(scrPlace, &usableStart, &yIgnored, &usableWidth, &hIgnored, rect->horzAlign, rect->vertAlign);
        if ( (float)((float)(cursorx - usableStart) + 5.0) >= 0.0 )
        {
            if ( (float)((float)(cursorx - usableStart) - usableWidth) < 0.0 )
                v4 = cursorx - usableStart;
            else
                v4 = usableWidth;
            if ( (float)(0.0 - (float)(cursorx - usableStart)) < 0.0 )
                v3 = v4;
            else
                v3 = 0.0f;
            value = v3 / usableWidth;
            value = (float)(editDef->maxVal - editDef->minVal) * (float)(v3 / usableWidth);
            value = value + editDef->minVal;
            v2 = va("%g", value);
            Dvar_SetFromStringByName(item->dvar, v2);
        }
    }
}

int __cdecl Item_Slider_HandleKey(UiContext *dc, itemDef_s *item, int key)
{
    const char *VariantString; // eax
    char *v5; // eax
    char *v6; // eax
    float maxVal; // [esp+8h] [ebp-38h]
    bool v8; // [esp+Ch] [ebp-34h]
    float minVal; // [esp+10h] [ebp-30h]
    bool v10; // [esp+14h] [ebp-2Ch]
    char v11; // [esp+23h] [ebp-1Dh]
    char v12; // [esp+2Fh] [ebp-11h]
    editFieldDef_s *editDef; // [esp+30h] [ebp-10h]
    float step; // [esp+34h] [ebp-Ch]
    const dvar_s *dvar; // [esp+38h] [ebp-8h]
    float value; // [esp+3Ch] [ebp-4h]

    if ( !item->dvar )
        return 0;
    dvar = Dvar_FindVar(item->dvar);
    if ( !dvar )
        return 0;
    if ( !Item_ShouldHandleKey(dc, item, key) )
        return 0;
    if ( key == 200 || key == 201 || key == 202 )
    {
        Scroll_Slider_SetThumbPos(dc, item);
        return 1;
    }
    else
    {
        editDef = Item_GetEditFieldDef(item);
        if ( editDef )
        {
            step = (float)(editDef->maxVal - editDef->minVal) * 0.050000001;
            VariantString = Dvar_GetVariantString(item->dvar);
            value = atof(VariantString);
            if ( (dvar->type == DVAR_TYPE_INT || dvar->type == DVAR_TYPE_INT64) && step < 1.0 )
                step = 1.0f;
            if ( key == 22 || key == 30 )
            {
                v12 = 1;
            }
            else
            {
                v10 = key == 156 || key == 164;
                v12 = v10;
            }
            if ( v12 )
            {
                if ( (float)((float)(value - step) - editDef->minVal) < 0.0 )
                    minVal = editDef->minVal;
                else
                    minVal = value - step;
                v5 = va("%g", minVal);
                Dvar_SetFromStringByName(item->dvar, v5);
                return 1;
            }
            else
            {
                if ( key == 23 || key == 31 )
                {
                    v11 = 1;
                }
                else
                {
                    v8 = key == 157 || key == 163;
                    v11 = v8;
                }
                if ( v11 )
                {
                    if ( (float)(editDef->maxVal - (float)(value + step)) < 0.0 )
                        maxVal = editDef->maxVal;
                    else
                        maxVal = value + step;
                    v6 = va("%g", maxVal);
                    Dvar_SetFromStringByName(item->dvar, v6);
                    return 1;
                }
                else
                {
                    return 0;
                }
            }
        }
        else
        {
            return 0;
        }
    }
}

void __cdecl Item_Action(int localClientNum, UiContext *dc, itemDef_s *item)
{
    if ( dc->openMenuCount )
    {
        if ( item )
            Item_RunEventScript(localClientNum, dc, item, "action");
    }
}

itemDef_s *__cdecl Menu_SetPrevCursorItem(int localClientNum, UiContext *dc, menuDef_t *menu, int usedKeyboard)
{
    int CursorItem; // eax
    int v5; // eax
    int v6; // eax
    float x; // [esp+0h] [ebp-10h]
    float y; // [esp+4h] [ebp-Ch]
    int oldCursor; // [esp+8h] [ebp-8h]
    int wrapped; // [esp+Ch] [ebp-4h]

    wrapped = 0;
    oldCursor = Menu_GetCursorItem(dc->contextIndex, menu);
    if ( Menu_GetCursorItem(dc->contextIndex, menu) < 0 )
    {
        Menu_SetCursorItem(dc->contextIndex, menu, menu->itemCount - 1);
        wrapped = 1;
    }
    while ( Menu_GetCursorItem(dc->contextIndex, menu) > -1 )
    {
        CursorItem = Menu_GetCursorItem(dc->contextIndex, menu);
        Menu_SetCursorItem(dc->contextIndex, menu, CursorItem - 1);
        if ( Menu_GetCursorItem(dc->contextIndex, menu) < 0 && !wrapped )
        {
            wrapped = 1;
            Menu_SetCursorItem(dc->contextIndex, menu, menu->itemCount - 1);
        }
        if ( Menu_GetCursorItem(dc->contextIndex, menu) < 0 )
            break;
        if ( usedKeyboard )
        {
            v5 = Menu_GetCursorItem(dc->contextIndex, menu);
            if ( !Item_HasFocusEvent(localClientNum, dc, menu->items[v5]) )
                continue;
        }
        y = dc->cursor.y;
        x = dc->cursor.x;
        v6 = Menu_GetCursorItem(dc->contextIndex, menu);
        if ( Item_SetFocus(localClientNum, dc, menu->items[v6], x, y) )
            return menu->items[Menu_GetCursorItem(dc->contextIndex, menu)];
    }
    Menu_SetCursorItem(dc->contextIndex, menu, oldCursor);
    return 0;
}

void __cdecl Menus_HandleOOBClick(int localClientNum, UiContext *dc, menuDef_t *menu, int key, int down)
{
    int j; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 7531, 0, "%s", "menu") )
        __debugbreak();
    if ( menu )
    {
        if (down && (menu->window.staticFlags & 0x2000000) != 0)
            Menus_Close(localClientNum, dc, menu);
        Menu_ClearFocus(localClientNum, dc, menu);
        for ( i = dc->openMenuCount - 1; i >= 0; --i )
        {
            if ( Menu_OverActiveItem(dc->contextIndex, dc->menuStack[i].menu, dc->cursor.x, dc->cursor.y) )
            {
                for ( j = dc->openMenuCount - 1; j >= 0; --j )
                    Window_RemoveDynamicFlags(dc->contextIndex, &dc->menuStack[j].menu->window, 2);
                Window_AddDynamicFlags(dc->contextIndex, &dc->menuStack[i].menu->window, 6);
                Menu_HandleKey(0, dc, dc->menuStack[i].menu, key, down);
                break;
            }
        }
        if ( !Display_VisibleMenuCount(dc) )
            UI_Pause(localClientNum, 0);
    }
}

int __cdecl Display_VisibleMenuCount(UiContext *dc)
{
    int i; // [esp+0h] [ebp-8h]
    int count; // [esp+4h] [ebp-4h]

    count = 0;
    for ( i = 0; i < dc->menuCount; ++i )
    {
        if ( (Window_GetDynamicFlags(dc->contextIndex, &dc->Menus[i]->window) & 0x4004) != 0 )
            ++count;
    }
    return count;
}

rectDef_s *__cdecl Item_CorrectedTextRect(int contextIndex, itemDef_s *item)
{
    rect.x = 0.0;
    rect.y = 0.0;
    rect.w = 0.0;
    rect.h = 0.0;
    rect.horzAlign = 0;
    rect.vertAlign = 0;
    if ( Item_IsTextDefType(item) )
    {
        rect = Item_GetTextRect(contextIndex, item)->textRect[0];
        if ( rect.w != 0.0 )
            rect.y = rect.y - rect.h;
    }
    else
    {
        rect = *Item_GetNonTextRect(contextIndex, item);
    }
    return &rect;
}

const rectDef_s *__cdecl Item_GetNonTextRect(int contextIndex, const itemDef_s *item)
{
    if ( contextIndex
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\ui\\../ui/ui_utils.h",
                    68,
                    0,
                    "contextIndex doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    contextIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( !item && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\ui\\../ui/ui_utils.h", 69, 0, "%s", "item") )
        __debugbreak();
    return &item->window.rect;
}

char __cdecl Menu_ItemsAreAnimating(int localClientNum, UiContext *dc, menuDef_t *menu)
{
    itemDef_s *nextItem; // [esp+0h] [ebp-8h]
    int menuIndex; // [esp+4h] [ebp-4h]

    for ( menuIndex = 0; menuIndex < menu->itemCount; ++menuIndex )
    {
        nextItem = menu->items[menuIndex];
        if ( !nextItem->type && nextItem->animInfo && nextItem->animInfo->animating )
            return 1;
    }
    return 0;
}

void __cdecl Item_TextColor(UiContext *dc, itemDef_s *item, float (*newColor)[4])
{
    UILocalVarContext *LocalVarsContext; // eax
    float v4; // xmm0_4
    float *disableColor; // edx
    UILocalVarContext *var; // [esp+20h] [ebp-20h]
    float lowLight[4]; // [esp+24h] [ebp-1Ch] BYREF
    menuDef_t *parent; // [esp+34h] [ebp-Ch]
    textDef_s *textDefPtr; // [esp+38h] [ebp-8h]
    int flags; // [esp+3Ch] [ebp-4h] BYREF

    parent = item->parent;
    textDefPtr = Item_GetTextDef(item);
    LocalVarsContext = UI_UIContext_GetLocalVarsContext(dc->contextIndex);
    var = UILocalVar_Find(LocalVarsContext, "ui_choicegroup");
    flags = Window_GetDynamicFlags(dc->contextIndex, &item->window);
    Fade(
        &flags,
        &item->window.foreColor[3],
        parent->fadeClamp,
        &item->window.nextTime,
        parent->fadeCycle,
        1,
        parent->fadeAmount,
        parent->fadeInAmount,
        dc);
    Window_SetDynamicFlags(dc->contextIndex, &item->window, flags);
    if (Window_HasFocus(dc->contextIndex, &item->window))
    {
        if (!var || I_strcmp(var->table[0].u.string, "popmenu") || (item->parent->window.staticFlags & 0x1000000) != 0)
        {
            (*newColor)[0] = item->window.foreColor[0];
            (*newColor)[1] = item->window.foreColor[1];
            (*newColor)[2] = item->window.foreColor[2];
            (*newColor)[3] = item->window.foreColor[3];
        }
        else
        {
            (*newColor)[0] = item->window.foreColor[0];
            (*newColor)[1] = item->window.foreColor[1];
            (*newColor)[2] = item->window.foreColor[2];
            (*newColor)[3] = item->window.foreColor[3];
        }
    }
    else if (textDefPtr->textStyle != 1 || ((dc->realTime / 256) & 1) != 0)
    {
        (*newColor)[0] = item->window.foreColor[0];
        (*newColor)[1] = item->window.foreColor[1];
        (*newColor)[2] = item->window.foreColor[2];
        (*newColor)[3] = item->window.foreColor[3];
    }
    else
    {
        lowLight[0] = 0.80000001 * item->window.foreColor[0];
        lowLight[1] = 0.80000001 * item->window.foreColor[1];
        lowLight[2] = 0.80000001 * item->window.foreColor[2];
        lowLight[3] = 0.80000001 * item->window.foreColor[3];
        
        //v4 = __libm_sse2_sin((float)(dc->realTime / 75));
        v4 = sin((float)(dc->realTime / 75));
        LerpColor(item->window.foreColor, lowLight, (float *)newColor, (float)(v4 * 0.5) + 0.5);
    }
    if (item->enableDvar
        && *item->enableDvar
        && item->dvarTest
        && *item->dvarTest
        && (item->dvarFlags & 3) != 0
        && !Item_EnableShowViaDvar(item, 1))
    {
        disableColor = parent->disableColor;
        (*newColor)[0] = parent->disableColor[0];
        (*newColor)[1] = disableColor[1];
        (*newColor)[2] = disableColor[2];
        (*newColor)[3] = disableColor[3];
    }
}

void __cdecl Fade(
                int *flags,
                float *f,
                float clamp,
                int *nextTime,
                int offsetTime,
                int bFlags,
                float fadeAmount,
                float fadeInAmount,
                UiContext *dc)
{
    if ( (*flags & 0x30) != 0 && dc->realTime > *nextTime )
    {
        *nextTime = offsetTime + dc->realTime;
        if ( (*flags & 0x10) != 0 )
        {
            *f = *f - fadeAmount;
            if ( bFlags )
            {
                if ( *f <= 0.0 )
                    *flags &= 0xFFFFFFEB;
            }
        }
        else
        {
            *f = *f + fadeInAmount;
            if ( *f >= clamp )
            {
                *f = clamp;
                if ( bFlags )
                    *flags &= ~0x20u;
            }
        }
    }
}

double __cdecl Item_GetTextPlacementX(int alignX, float x0, float containerWidth, float selfWidth)
{
    if ( !alignX )
        return x0;
    if ( alignX == 1 )
        return (containerWidth - selfWidth) * 0.5 + x0;
    if ( alignX != 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    242,
                    0,
                    "alignX == ITEM_ALIGN_RIGHT\n\t%i, %i",
                    alignX,
                    2) )
    {
        __debugbreak();
    }
    return containerWidth - selfWidth + x0;
}

double __cdecl Item_GetTextPlacementY(int alignY, float y0, float containerHeight, float selfHeight)
{
    switch ( alignY )
    {
        case 0:
            return y0;
        case 4:
            return y0 + selfHeight;
        case 8:
            return (containerHeight + selfHeight) * 0.5 + y0;
    }
    if ( alignY != 12
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    266,
                    0,
                    "%s\n\t(alignY) = %i",
                    "(alignY == 12)",
                    alignY) )
    {
        __debugbreak();
    }
    return y0 + containerHeight;
}

int __cdecl UI_PickWordWrapLineWidth(
                const char *text,
                int bufferSize,
                Font_s *font,
                float normalizedScale,
                int targetLineCount,
                int widthGuess,
                int widthLimit)
{
    int lineCount; // [esp+4h] [ebp-8h]
    const char *p; // [esp+8h] [ebp-4h]

    while ( widthGuess < widthLimit )
    {
        p = text;
        lineCount = 0;
        while ( *p )
        {
            for ( p = R_TextLineWrapPosition(p, bufferSize, widthGuess, font, normalizedScale);
                        isspace(*(unsigned __int8 *)p);
                        ++p )
            {
                ;
            }
            ++lineCount;
        }
        if ( lineCount == targetLineCount )
            return widthGuess;
        widthGuess += 8;
    }
    return widthLimit;
}

void __cdecl UI_DrawWrappedText(
                const ScreenPlacement *scrPlace,
                const char *text,
                const rectDef_s *rect,
                Font_s *font,
                float x,
                float y,
                float scale,
                const float *color,
                int style,
                char textAlignMode,
                rectDef_s *textRect)
{
    DrawWrappedText(scrPlace, text, rect, font, x, y, scale, color, style, textAlignMode, textRect, 0, 0, 0);
}

void __cdecl DrawWrappedText(
                const ScreenPlacement *scrPlace,
                const char *text,
                const rectDef_s *rect,
                Font_s *font,
                float x,
                float y,
                float scale,
                const float *color,
                int style,
                char textAlignMode,
                rectDef_s *textRect,
                bool subtitle,
                const float *subtitleGlowColor,
                bool cinematic)
{
    float v14; // [esp+28h] [ebp-440h]
    float v15; // [esp+2Ch] [ebp-43Ch]
    float lineX; // [esp+34h] [ebp-434h]
    float height; // [esp+3Ch] [ebp-42Ch]
    float normalizedScale; // [esp+40h] [ebp-428h]
    float textWidth; // [esp+44h] [ebp-424h]
    unsigned int len; // [esp+48h] [ebp-420h]
    int targetLineWidth; // [esp+4Ch] [ebp-41Ch]
    const char *wrapPosition; // [esp+50h] [ebp-418h]
    float lineWidth; // [esp+54h] [ebp-414h]
    char buff[1028]; // [esp+58h] [ebp-410h] BYREF
    int xAlignMode; // [esp+460h] [ebp-8h]
    const char *p; // [esp+464h] [ebp-4h]
    float xa; // [esp+480h] [ebp+18h]
    float ya; // [esp+484h] [ebp+1Ch]

    if ( rect == textRect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 8246, 0, "%s", "rect != textRect") )
    {
        __debugbreak();
    }
    textRect->x = x + rect->w;
    textRect->y = y;
    textRect->w = 0.0f;
    textRect->h = 0.0f;
    normalizedScale = R_NormalizedTextScale(font, scale);
    height = (float)UI_TextHeight(font, scale);
    textWidth = (float)UI_TextWidth(text, 0, font, scale);
    xAlignMode = textAlignMode & 3;
    xa = Item_GetTextPlacementX(xAlignMode, x, rect->w, 0.0);
    ya = Item_GetTextPlacementY(textAlignMode & 0xC, y, rect->h, height);
    if ( rect->w == 0.0 || textWidth <= rect->w || xAlignMode != 1 )
    {
        targetLineWidth = (int)rect->w;
    }
    else
    {
        v15 = ceil((float)(textWidth / rect->w));
        v14 = ceil((float)(textWidth / (float)(int)v15));
        targetLineWidth = UI_PickWordWrapLineWidth(text, 1024, font, normalizedScale, (int)v15, (int)v14, (int)rect->w);
    }
    p = text;
    while ( *p )
    {
        wrapPosition = R_TextLineWrapPosition(p, 1024, targetLineWidth, font, normalizedScale);
        len = wrapPosition - p;
        if ( wrapPosition - p <= 0 )
        {
            Com_Printf(13, "WARNING: Error wrapping text: %s\n", text);
            break;
        }
        memcpy((unsigned __int8 *)buff, (unsigned __int8 *)p, len);
        buff[len] = 0;
        lineWidth = (float)UI_TextWidth(buff, 0, font, scale);
        if ( xAlignMode == 1 )
        {
            lineX = xa - (float)(lineWidth * 0.5);
        }
        else if ( xAlignMode == 2 )
        {
            lineX = xa - lineWidth;
        }
        else
        {
            lineX = xa;
        }
        if ( subtitle )
        {
            if ( !subtitleGlowColor
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 8303, 0, "%s", "subtitleGlowColor") )
            {
                __debugbreak();
            }
            UI_DrawTextWithGlow(
                scrPlace,
                buff,
                0x7FFFFFFF,
                font,
                lineX,
                ya,
                rect->horzAlign,
                rect->vertAlign,
                scale,
                color,
                style,
                subtitleGlowColor,
                1,
                cinematic);
        }
        else
        {
            UI_DrawText(scrPlace, buff, 0x7FFFFFFF, font, lineX, ya, rect->horzAlign, rect->vertAlign, scale, color, style);
        }
        ya = ya + height;
        if ( lineWidth > textRect->w )
        {
            if ( textRect->x < lineX
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                            8314,
                            0,
                            "textRect->x >= lineX\n\t%g, %g",
                            textRect->x,
                            lineX) )
            {
                __debugbreak();
            }
            textRect->x = lineX;
            textRect->w = lineWidth;
        }
        for ( p += len; *p != 10 && isspace(*(unsigned __int8 *)p); ++p )
            ;
    }
    textRect->h = ya - textRect->y;
}

void __cdecl UI_DrawWrappedTextSubtitled(
                const ScreenPlacement *scrPlace,
                const char *text,
                const rectDef_s *rect,
                Font_s *font,
                float x,
                float y,
                float scale,
                const float *color,
                int style,
                char textAlignMode,
                rectDef_s *textRect,
                const float *subtitleGlowColor,
                bool cinematic)
{
    DrawWrappedText(
        scrPlace,
        text,
        rect,
        font,
        x,
        y,
        scale,
        color,
        style,
        textAlignMode,
        textRect,
        1,
        subtitleGlowColor,
        cinematic);
}

char __cdecl UI_GetSticksTranslatedString(int localClientNum, const char *command, char *keys)
{
    GamerSettingState *ProfileSettings; // eax
    GamerSettingState *v4; // eax
    char *v5; // eax
    char *v6; // eax
    const char *v7; // eax
    const char *v9; // eax
    const char *v10; // eax
    char *v11; // eax
    char *v12; // eax
    char *v13; // eax
    const char *String; // eax
    const char *v15; // eax
    char *v16; // eax
    char *v17; // eax
    char binding0[64]; // [esp+0h] [ebp-108h] BYREF
    char binding1[64]; // [esp+40h] [ebp-C8h] BYREF
    char binding2[64]; // [esp+80h] [ebp-88h] BYREF
    char binding3[68]; // [esp+C0h] [ebp-48h] BYREF

    if ( I_stricmp(command, "+movestick") )
    {
        if ( I_stricmp(command, "+lookstick") )
        {
            if ( I_stricmp(command, "+forward")
                && I_stricmp(command, "+back")
                && I_stricmp(command, "+moveleft")
                && I_stricmp(command, "+moveright")
                || !Dvar_GetBool("gpad_enabled") )
            {
                return 0;
            }
            else
            {
                String = Dvar_GetString("gpad_sticksConfig");
                if ( I_strcmp(String, "thumbstick_default")
                    && (v15 = Dvar_GetString("gpad_sticksConfig"), I_strcmp(v15, "thumbstick_legacy")) )
                {
                    v17 = UI_SafeTranslateString("PLATFORM_RIGHT_STICK");
                    I_strncpyz(keys, v17, 256);
                }
                else
                {
                    v16 = UI_SafeTranslateString("PLATFORM_LEFT_STICK");
                    I_strncpyz(keys, v16, 256);
                }
                return 1;
            }
        }
        else
        {
            if ( Dvar_GetBool("gpad_enabled") )
            {
                v9 = Dvar_GetString("gpad_sticksConfig");
                if ( I_strcmp(v9, "thumbstick_default")
                    && (v10 = Dvar_GetString("gpad_sticksConfig"), I_strcmp(v10, "thumbstick_legacy")) )
                {
                    v12 = UI_SafeTranslateString("PLATFORM_LEFT_STICK");
                    I_strncpyz(keys, v12, 256);
                }
                else
                {
                    v11 = UI_SafeTranslateString("PLATFORM_RIGHT_STICK");
                    I_strncpyz(keys, v11, 256);
                }
            }
            else
            {
                v13 = UI_SafeTranslateString("MENU_MOUSE_LOOK");
                I_strncpyz(keys, v13, 256);
            }
            return 1;
        }
    }
    else
    {
        if ( Dvar_GetBool("gpad_enabled") )
        {
            ProfileSettings = GamerProfile_GetProfileSettings(localClientNum);
            if ( I_strcmp(ProfileSettings->gpadSticksConfig, "thumbstick_default")
                && (v4 = GamerProfile_GetProfileSettings(localClientNum), I_strcmp(v4->gpadSticksConfig, "thumbstick_legacy")) )
            {
                v6 = UI_SafeTranslateString("PLATFORM_RIGHT_STICK");
                I_strncpyz(keys, v6, 256);
            }
            else
            {
                v5 = UI_SafeTranslateString("PLATFORM_LEFT_STICK");
                I_strncpyz(keys, v5, 256);
            }
        }
        else
        {
            UI_GetKeyBindingLocalizedString(localClientNum, "+forward", binding0, 0);
            UI_GetKeyBindingLocalizedString(localClientNum, "+back", binding1, 0);
            UI_GetKeyBindingLocalizedString(localClientNum, "+moveleft", binding2, 0);
            UI_GetKeyBindingLocalizedString(localClientNum, "+moveright", binding3, 0);
            v7 = va("%s,%s,%s,%s", binding0, binding1, binding2, binding3);
            I_strncpyz(keys, v7, 256);
        }
        return 1;
    }
}

int __cdecl UI_GetKeyBindingLocalizedString(int localClientNum, const char *command, char *keys, int bindNum)
{
    char *v4; // eax
    const char *v6; // [esp+4h] [ebp-124h]
    const char *translation; // [esp+10h] [ebp-118h]
    const char *translationa; // [esp+10h] [ebp-118h]
    const char *translation_4; // [esp+14h] [ebp-114h]
    const char *consoleCmd; // [esp+18h] [ebp-110h] BYREF
    int bindCount; // [esp+1Ch] [ebp-10Ch]
    char bindings[2][128]; // [esp+20h] [ebp-108h] BYREF
    const char *conjunction; // [esp+124h] [ebp-4h]

    if ( zombietron->current.enabled )
        consoleCmd = command;
    else
        GetPlatformCommand(localClientNum, command, &consoleCmd);
    if ( gpad_enabled && gpad_enabled->current.enabled )
    {
        bindCount = CL_GetGamePadBinding(localClientNum, consoleCmd, bindings, bindNum != 0);
        if ( !bindCount && !bindNum )
            bindCount = CL_GetKeyBinding(localClientNum, consoleCmd, bindings, 1);
    }
    else
    {
        bindCount = CL_GetKeyBinding(localClientNum, command, bindings, bindNum != 0);
        if ( !bindCount )
            bindCount = CL_GetKeyBinding(localClientNum, consoleCmd, bindings, bindNum != 0);
        if ( !bindCount && !bindNum )
            bindCount = CL_GetKeyBinding(localClientNum, consoleCmd, bindings, 1);
    }
    if ( bindCount )
    {
        if ( bindCount == 1 )
        {
            translation = SEH_StringEd_GetString(bindings[0]);
            if ( translation )
                I_strncpyz(keys, translation, 256);
            else
                I_strncpyz(keys, bindings[0], 256);
        }
        else
        {
            if ( bindCount != 2
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                            8973,
                            0,
                            "%s\n\t(bindCount) = %i",
                            "(bindCount == 2)",
                            bindCount) )
            {
                __debugbreak();
            }
            translationa = SEH_StringEd_GetString(bindings[0]);
            translation_4 = SEH_StringEd_GetString(bindings[1]);
            conjunction = UI_SafeTranslateString("KEY_OR");
            if ( translation_4 )
                v6 = translation_4;
            else
                v6 = bindings[1];
            if ( translationa )
                Com_sprintf(keys, 0x100u, "%s %s %s", translationa, conjunction, v6);
            else
                Com_sprintf(keys, 0x100u, "%s %s %s", bindings[0], conjunction, v6);
        }
    }
    else
    {
        v4 = UI_SafeTranslateString("KEY_UNBOUND");
        I_strncpyz(keys, v4, 256);
    }
    return bindCount;
}

void __cdecl GetPlatformCommand(int localClientNum, const char *command, const char **consoleCommand)
{
    GamerSettingState *ProfileSettings; // eax
    const char *String; // eax
    const char *v5; // eax

    if ( gpad_enabled && gpad_enabled->current.enabled )
    {
        if ( !strcmp(command, "+activate") )
        {
            *consoleCommand = "+usereload";
        }
        else if ( !strcmp(command, "+reload") )
        {
            *consoleCommand = "+usereload";
        }
        else if ( !strcmp(command, "+melee_breath") )
        {
            *consoleCommand = "+holdbreath";
        }
        else
        {
            ProfileSettings = GamerProfile_GetProfileSettings(localClientNum);
            if ( I_strcmp(ProfileSettings->gpadButtonsConfig, "buttons_charlie")
                && (String = Dvar_GetString("gpad_buttonsConfig"), I_strcmp(String, "buttons_nomad"))
                && (v5 = Dvar_GetString("gpad_buttonsConfig"), I_strcmp(v5, "buttons_nomad_alt"))
                || strcmp(command, "+speed_throw") )
            {
                *consoleCommand = command;
            }
            else
            {
                *consoleCommand = "+toggleads_throw";
            }
        }
    }
    else if ( !strcmp(command, "+speed_throw") )
    {
        *consoleCommand = "+toggleads_throw";
    }
    else
    {
        *consoleCommand = command;
    }
}

int __cdecl Display_KeyBindPending()
{
    return g_waitingForKey;
}

int __cdecl Item_Bind_HandleKey(int localClientNum, UiContext *dc, itemDef_s *item, int key, int down)
{
    int type; // [esp+0h] [ebp-18h]
    bool v7; // [esp+7h] [ebp-11h]
    int bindNum; // [esp+8h] [ebp-10h]
    int bindCount; // [esp+Ch] [ebp-Ch]
    int boundKeys[2]; // [esp+10h] [ebp-8h] BYREF

    type = item->type;
    if ( type == 12 )
    {
        bindNum = 0;
    }
    else
    {
        if ( type != 16 )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                            9105,
                            0,
                            "Got an unexpected bind type") )
                __debugbreak();
            return 0;
        }
        bindNum = 1;
    }
    if ( g_waitingForKey )
    {
        if ( g_bindItem )
        {
            if ( (key & 0x400) != 0 )
            {
                return 1;
            }
            else if ( key == 27 )
            {
                g_waitingForKey = 0;
                return 1;
            }
            else if ( key == 96 )
            {
                return 1;
            }
            else
            {
                Sys_EnterCriticalSection(CRITSECT_KEY_BINDINGS);
                bindCount = Key_GetCommandAssignment(localClientNum, item->dvar, boundKeys, bindNum);
                if ( bindNum )
                {
                    if ( key == 127 || bindCount == 2 )
                    {
                        Key_SetBinding(localClientNum, boundKeys[0], (char *)"", 1);
                        Key_SetBinding(localClientNum, boundKeys[1], (char *)"", 1);
                    }
                    if ( key != 127 )
                        Key_SetBinding(localClientNum, key, (char *)item->dvar, 1);
                }
                else
                {
                    if ( key == 127 || bindCount == 2 )
                    {
                        Key_SetBinding(localClientNum, boundKeys[0], (char *)"", 0);
                        Key_SetBinding(localClientNum, boundKeys[1], (char *)"", 0);
                    }
                    if ( key != 127 )
                        Key_SetBinding(localClientNum, key, (char *)item->dvar, 0);
                }
                Sys_LeaveCriticalSection(CRITSECT_KEY_BINDINGS);
                g_waitingForKey = 0;
                return 1;
            }
        }
        else
        {
            return 0;
        }
    }
    else if ( down && (key != 200 || !Item_ContainsMouse(dc, item) ? (v7 = key == 13) : (v7 = 1), v7) )
    {
        g_waitingForKey = 1;
        g_bindItem = item;
        return 1;
    }
    else
    {
        return 0;
    }
}

menuDef_t *__cdecl Menu_GetFocused(UiContext *dc)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = dc->openMenuCount - 1; i >= 0; --i )
    {
        if ( Window_HasFocus(dc->contextIndex, &dc->menuStack[i].menu->window)
            && Window_IsVisible(dc->contextIndex, &dc->menuStack[i].menu->window) )
        {
            return dc->menuStack[i].menu;
        }
    }
    return 0;
}

void __cdecl Menu_SetFeederSelection(
                int localClientNum,
                UiContext *dc,
                menuDef_t *menu,
                int feeder,
                int index,
                const char *name)
{
    int v6; // eax
    int viewMax; // [esp+14h] [ebp-10h]
    int i; // [esp+18h] [ebp-Ch]
    itemDef_s *item; // [esp+1Ch] [ebp-8h]
    listBoxDef_s *listPtr; // [esp+20h] [ebp-4h]

    if ( !menu )
    {
        if ( name )
            menu = Menus_FindByName(dc, name);
        else
            menu = Menu_GetFocused(dc);
    }
    if ( menu )
    {
        for ( i = 0; i < menu->itemCount; ++i )
        {
            item = menu->items[i];
            if ( item->type == 4 )
            {
                listPtr = Item_GetListBoxDef(item);
                if ( listPtr->special == (float)feeder )
                {
                    viewMax = Item_ListBox_Viewmax(localClientNum, dc->contextIndex, item);
                    if ( index >= listPtr->startPos[dc->contextIndex] )
                    {
                        if ( index - listPtr->startPos[dc->contextIndex] > viewMax )
                            listPtr->startPos[dc->contextIndex] = index - viewMax + 1;
                    }
                    else
                    {
                        listPtr->startPos[dc->contextIndex] = index;
                    }
                    v6 = UI_FeederCount(localClientNum, dc->contextIndex, listPtr->special, listPtr);
                    Item_ListBox_SetCursorPos(localClientNum, dc->contextIndex, item, v6 - 1, viewMax, index, 1);
                }
            }
        }
    }
}

int __cdecl Menu_GetFeederSelection(UiContext *dc, menuDef_t *menu, int feeder, const char *name)
{
    int i; // [esp+0h] [ebp-Ch]
    itemDef_s *item; // [esp+4h] [ebp-8h]
    listBoxDef_s *listPtr; // [esp+8h] [ebp-4h]

    if ( !menu )
    {
        if ( name )
            menu = Menus_FindByName(dc, name);
        else
            menu = Menu_GetFocused(dc);
    }
    if ( menu )
    {
        for ( i = 0; i < menu->itemCount; ++i )
        {
            item = menu->items[i];
            if ( item->type == 4 )
            {
                listPtr = Item_GetListBoxDef(item);
                if ( listPtr->special == (float)feeder )
                    return listPtr->cursorPos[dc->contextIndex];
            }
        }
    }
    return -1;
}

int __cdecl Menus_AnyFullScreenVisible(UiContext *dc)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = dc->openMenuCount - 1; i >= 0; --i )
    {
        if ( Window_IsVisible(dc->contextIndex, &dc->menuStack[i].menu->window) && dc->menuStack[i].menu->fullScreen )
            return 1;
    }
    return 0;
}

int __cdecl Menu_HandleMouseMove(int localClientNum, UiContext *dc, menuDef_t *menu)
{
    const rectDef_s *Rect; // eax
    const rectDef_s *v5; // eax
    char DynamicFlags; // al
    const rectDef_s *v7; // eax
    int pass; // [esp+8h] [ebp-20h]
    itemDef_s *focusItem; // [esp+Ch] [ebp-1Ch]
    textDef_s *textDefPtr; // [esp+10h] [ebp-18h]
    int i; // [esp+14h] [ebp-14h]
    cursor_t x; // [esp+18h] [ebp-10h]
    int focusSet; // [esp+20h] [ebp-8h]
    itemDef_s *overItem; // [esp+24h] [ebp-4h]

    focusSet = 0;
    focusItem = 0;
    if ( !dc->isCursorVisible )
        return 0;
    if ( !menu )
        return 0;
    if ( menu->ui3dWindowId >= 0 )
        return 0;
    if ( (Window_GetDynamicFlags(dc->contextIndex, &menu->window) & 0x4004) == 0 )
        return 0;
    if ( itemCapture )
        return 0;
    if ( g_waitingForKey )
        return 0;
    if ( Menu_ItemsAreAnimating(localClientNum, dc, menu) )
    {
        mouseLocationX = 0x80000000;
        mouseLocationY = 0x80000000;
        mouseTimeUntilReadyToMove = Sys_Milliseconds() + 500;
        return 0;
    }
    else
    {
        x = dc->cursor;
        for ( pass = 0; pass < 2; ++pass )
        {
            for ( i = menu->itemCount - 1; i >= 0; --i )
            {
                if ( (Window_GetDynamicFlags(dc->contextIndex, &menu->items[i]->window) & 0x4004) != 0
                    && ((menu->items[i]->dvarFlags & 3) == 0 || Item_EnableShowViaDvar(menu->items[i], 1))
                    && ((menu->items[i]->dvarFlags & 0xC) == 0 || Item_EnableShowViaDvar(menu->items[i], 4))
                    && Item_IsVisible(localClientNum, dc->contextIndex, menu->items[i]) )
                {
                    if ( Window_HasFocus(dc->contextIndex, &menu->items[i]->window) && !focusItem )
                    {
                        focusItem = menu->items[i];
                        if ( Item_IsModal(focusItem) )
                            focusSet = 1;
                    }
                    Rect = Window_GetRect(&menu->items[i]->window);
                    if ( Rect_ContainsPoint(dc->contextIndex, Rect, x.x, x.y) )
                    {
                        if ( pass == 1 )
                        {
                            overItem = menu->items[i];
                            textDefPtr = Item_GetTextDef(overItem);
                            if ( overItem->type != 1
                                || !textDefPtr->text
                                || (v5 = Item_CorrectedTextRect(dc->contextIndex, overItem),
                                        Rect_ContainsPoint(dc->contextIndex, v5, x.x, x.y)) )
                            {
                                DynamicFlags = Window_GetDynamicFlags(dc->contextIndex, &overItem->window);
                                if ( IsVisible(DynamicFlags) )
                                {
                                    Item_MouseEnter(localClientNum, dc, overItem, x.x, x.y);
                                    Item_SetMouseOver(dc, overItem, 1);
                                    if ( !focusSet )
                                    {
                                        focusSet = Item_SetFocus(localClientNum, dc, overItem, x.x, x.y);
                                        if ( focusSet )
                                            focusItem = overItem;
                                    }
                                }
                            }
                        }
                    }
                    else if ( (Window_GetDynamicFlags(dc->contextIndex, &menu->items[i]->window) & 1) != 0 )
                    {
                        Item_MouseLeave(localClientNum, dc, menu->items[i]);
                        Item_SetMouseOver(dc, menu->items[i], 0);
                    }
                }
            }
        }
        if ( !focusSet )
        {
            if ( focusItem )
            {
                v7 = Window_GetRect(&focusItem->window);
                if ( !Rect_ContainsPoint(dc->contextIndex, v7, x.x, x.y) )
                    Menu_ClearFocus(localClientNum, dc, menu);
            }
        }
        if ( focusSet )
        {
            mouseLocationX = (int)x.x;
            mouseLocationY = (int)x.y;
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

void __cdecl Item_MouseEnter(int localClientNum, UiContext *dc, itemDef_s *item, float x, float y)
{
    rectDef_s r; // [esp+8h] [ebp-24h] BYREF
    focusItemDef_s *focusPtr; // [esp+20h] [ebp-Ch]
    int flags; // [esp+24h] [ebp-8h]
    const rectDef_s *textRect; // [esp+28h] [ebp-4h]

    textRect = 0;
    focusPtr = Item_GetFocusItemDef(item);
    if ( Item_IsTextDefType(item) )
        textRect = (const rectDef_s *)Item_GetTextRect(dc->contextIndex, item);
    if ( item )
    {
        if ( textRect )
        {
            if ( focusPtr )
            {
                r = *textRect;
                r.y = r.y - r.h;
                r.horzAlign = textRect->horzAlign;
                r.vertAlign = textRect->vertAlign;
                if ( (item->dvarFlags & 3) == 0 || Item_EnableShowViaDvar(item, 1) )
                {
                    if ( Item_IsVisible(localClientNum, dc->contextIndex, item) )
                    {
                        flags = Window_GetDynamicFlags(dc->contextIndex, &item->window);
                        if ( Rect_ContainsPoint(dc->contextIndex, &r, x, y) )
                        {
                            if ( (flags & 0x40) == 0 && focusPtr->mouseEnterText )
                            {
                                Item_RunScript(localClientNum, dc, item, focusPtr->mouseEnterText);
                                Window_AddDynamicFlags(dc->contextIndex, &item->window, 64);
                            }
                            if ( (flags & 1) == 0 )
                            {
                                if ( focusPtr->mouseEnter )
                                {
                                    Item_RunScript(localClientNum, dc, item, focusPtr->mouseEnter);
                                    Window_AddDynamicFlags(dc->contextIndex, &item->window, 1);
                                }
                            }
                        }
                        else
                        {
                            if ( (flags & 0x40) != 0 && focusPtr->mouseExitText )
                            {
                                Item_RunScript(localClientNum, dc, item, focusPtr->mouseExitText);
                                Window_RemoveDynamicFlags(dc->contextIndex, &item->window, 64);
                            }
                            if ( (flags & 1) == 0 && focusPtr->mouseEnter )
                            {
                                Item_RunScript(localClientNum, dc, item, focusPtr->mouseEnter);
                                Window_AddDynamicFlags(dc->contextIndex, &item->window, 1);
                            }
                            if ( item->type == 4 )
                                Item_ListBox_MouseEnter(localClientNum, dc->contextIndex, item, x, y);
                        }
                    }
                }
            }
        }
    }
}

void __cdecl Item_ListBox_MouseEnter(int localClientNum, int contextIndex, itemDef_s *item, float x, float y)
{
    int v5; // eax
    int v6; // [esp+8h] [ebp-3Ch]
    int v7; // [esp+Ch] [ebp-38h]
    rectDef_s r; // [esp+1Ch] [ebp-28h] BYREF
    listBoxDef_s *listPtr; // [esp+34h] [ebp-10h]
    int mousePos; // [esp+38h] [ebp-Ch]
    int i; // [esp+3Ch] [ebp-8h]
    const rectDef_s *rect; // [esp+40h] [ebp-4h]

    listPtr = Item_GetListBoxDef(item);
    if ( listPtr )
    {
        Window_RemoveDynamicFlags(contextIndex, &item->window, 7936);
        v5 = Item_ListBox_OverLB(localClientNum, contextIndex, item, x, y);
        Window_AddDynamicFlags(contextIndex, &item->window, v5);
        rect = Window_GetRect(&item->window);
        r.horzAlign = rect->horzAlign;
        r.vertAlign = rect->vertAlign;
        if ( Window_IsHorizontal(&item->window) )
        {
            if ( (Window_GetDynamicFlags(contextIndex, &item->window) & 0x1F00) == 0 )
            {
                for ( i = 0; i < listPtr->numColumns; ++i )
                {
                    if ( listPtr->columnInfo[i].elementStyle == 1 )
                    {
                        r.x = rect->x + listPtr->columnInfo[i].rect.x;
                        r.y = rect->y + listPtr->columnInfo[i].rect.y;
                        r.h = listPtr->columnInfo[i].rect.h;
                        r.w = listPtr->columnInfo[i].rect.w - (float)listPtr->drawPadding;
                        if ( Rect_ContainsPoint(contextIndex, &r, x, y) )
                        {
                            mousePos = Item_ListBox_GetIndexForMouse(contextIndex, item, x, y);
                            if ( listPtr->endPos[contextIndex] - 1 < mousePos )
                                v7 = listPtr->endPos[contextIndex] - 1;
                            else
                                v7 = mousePos;
                            listPtr->mousePos = v7;
                            if ( Item_IsModal(item) )
                                Item_ListBox_ModalSelection(localClientNum, contextIndex, item, (int)x, (int)y);
                        }
                    }
                }
            }
        }
        else if ( (Window_GetDynamicFlags(contextIndex, &item->window) & 0x1F00) == 0 )
        {
            r.x = rect->x;
            r.y = rect->y;
            r.w = rect->w - (float)(Item_ListBox_RequiresScroll(localClientNum, contextIndex, item) ? 0x10 : 0);
            r.h = rect->h;
            if ( Rect_ContainsPoint(contextIndex, &r, x, y) )
            {
                mousePos = Item_ListBox_GetIndexForMouse(contextIndex, item, x, y);
                v6 = listPtr->endPos[contextIndex] - 1 < mousePos ? listPtr->endPos[contextIndex] - 1 : mousePos;
                listPtr->mousePos = v6;
                if ( Item_IsModal(item) )
                    Item_ListBox_ModalSelection(localClientNum, contextIndex, item, (int)x, (int)y);
            }
        }
    }
}

int __cdecl Item_ListBox_GetIndexForMouse(int contextIndex, itemDef_s *item, float x, float y)
{
    int v5; // [esp+8h] [ebp-5Ch]
    float compareY; // [esp+38h] [ebp-2Ch]
    rectDef_s compareRect; // [esp+3Ch] [ebp-28h] BYREF
    const ScreenPlacement *scrPlace; // [esp+54h] [ebp-10h]
    float compareX; // [esp+58h] [ebp-Ch]
    listBoxDef_s *listPtr; // [esp+5Ch] [ebp-8h]
    const rectDef_s *rect; // [esp+60h] [ebp-4h]

    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 5310, 0, "%s", "item") )
        __debugbreak();
    rect = Window_GetRect(&item->window);
    listPtr = Item_GetListBoxDef(item);
    compareRect.x = rect->x;
    compareRect.y = rect->y;
    compareRect.w = listPtr->elementWidth;
    compareRect.h = listPtr->elementHeight;
    scrPlace = &scrPlaceView[contextIndex];
    compareX = ScrPlace_ApplyX(scrPlace, x, 4);
    compareY = ScrPlace_ApplyY(scrPlace, y, 4);
    ScrPlace_ApplyRect(
        scrPlace,
        &compareRect.x,
        &compareRect.y,
        &compareRect.w,
        &compareRect.h,
        rect->horzAlign,
        rect->vertAlign);
    if ( Window_IsHorizontal(&item->window) )
        return (int)((float)((float)(compareX - compareRect.x) / compareRect.w) - 0.4999999990686774)
                 + listPtr->startPos[contextIndex];
    if ( (int)((float)((float)((float)(compareY - 2.0) - compareRect.y) / compareRect.h) - 0.4999999990686774) > 0 )
        v5 = (int)((float)((float)((float)(compareY - 2.0) - compareRect.y) / compareRect.h) - 0.4999999990686774);
    else
        v5 = 0;
    return v5 + listPtr->startPos[contextIndex];
}

void __cdecl Item_ListBox_ModalSelection(int localClientNum, int contextIndex, itemDef_s *item, int x, int y)
{
    uiInfo_s *uiInfo; // [esp+8h] [ebp-14h]
    const char *max; // [esp+Ch] [ebp-10h]
    int newPos; // [esp+10h] [ebp-Ch]
    listBoxDef_s *listPtr; // [esp+14h] [ebp-8h]
    int viewmax; // [esp+18h] [ebp-4h]

    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 5390, 0, "%s", "item") )
        __debugbreak();
    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 5394, 0, "%s", "listPtr") )
        __debugbreak();
    Item_Listbox_Mouse_Column_Set(localClientNum, contextIndex, item, x, y);
    if ( listPtr )
    {
        if ( listPtr->mousePos != listPtr->cursorPos[contextIndex] )
        {
            newPos = listPtr->mousePos;
            max = Item_ListBox_MaxScroll(localClientNum, contextIndex, item);
            viewmax = Item_ListBox_Viewmax(localClientNum, contextIndex, item);
            if ( !listPtr->notselectable
                && newPos >= 0
                && newPos < (int)UI_FeederCount(localClientNum, contextIndex, listPtr->special, listPtr) )
            {
                if ( listPtr->special == 80.0 )
                {
                    listPtr->cursorPos[contextIndex] = newPos;
                    Dvar_SetInt((dvar_s *)ui_listboxIndex, newPos - listPtr->startPos[contextIndex]);
                    UI_FeederSelection(localClientNum, contextIndex, listPtr->special, newPos);
                    uiInfo = UI_UIContext_GetInfo(contextIndex);
                    if ( item->parent->window.name )
                    {
                        if ( Menu_IsMenuOpenAndVisible(contextIndex, item->parent->window.name) )
                            Item_RunEventScript(localClientNum, &uiInfo->uiDC, item, "onListboxSelectionChange");
                    }
                }
                else
                {
                    Item_ListBox_SetCursorPos(localClientNum, contextIndex, item, (int)max, viewmax, newPos, 0);
                }
            }
        }
    }
}

void __cdecl Item_Listbox_Mouse_Column_Set(int localClientNum, int contextIndex, itemDef_s *item, int x, int y)
{
    char *v5; // eax
    char *v6; // eax
    int special; // [esp+8h] [ebp-30h]
    int columnIndex; // [esp+Ch] [ebp-2Ch]
    rectDef_s boxRect; // [esp+10h] [ebp-28h] BYREF
    const ScreenPlacement *scrPlace; // [esp+28h] [ebp-10h]
    const rectDef_s *windowRect; // [esp+2Ch] [ebp-Ch]
    listBoxDef_s *listPtr; // [esp+30h] [ebp-8h]
    const rectDef_s *columnRect; // [esp+34h] [ebp-4h]

    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 5341, 0, "%s", "item") )
        __debugbreak();
    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 5345, 0, "%s", "listPtr") )
        __debugbreak();
    special = (int)listPtr->special;
    if ( special >= 79 && special <= 80 )
    {
        scrPlace = &scrPlaceView[contextIndex];
        windowRect = Window_GetRect(&item->window);
        boxRect.horzAlign = windowRect->horzAlign;
        boxRect.vertAlign = windowRect->vertAlign;
        boxRect.h = windowRect->h;
        boxRect.w = listPtr->columnInfo[0].rect.w;
        for ( columnIndex = 0; columnIndex < 3; ++columnIndex )
        {
            columnRect = &listPtr->columnInfo[columnIndex].rect;
            boxRect.x = columnRect->x + windowRect->x;
            boxRect.y = listPtr->columnInfo[columnIndex].rect.y + windowRect->y;
            if ( Rect_ContainsPoint(contextIndex, &boxRect, (float)x, (float)y) )
            {
                if ( (int)listPtr->special == 80 )
                {
                    v5 = va("%i", columnIndex);
                    Dvar_SetCommand("fsPrivateSlotCol", v5);
                }
                else
                {
                    if ( (int)listPtr->special != 79
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                                    5375,
                                    0,
                                    "%s",
                                    "FEEDER_FILESHARE_SLOTS == static_cast<int>( listPtr->special )") )
                    {
                        __debugbreak();
                    }
                    v6 = va("%i", columnIndex);
                    Dvar_SetCommand("fsOtherUserPrivateSlotCol", v6);
                }
                return;
            }
        }
    }
}

void __cdecl Item_MouseLeave(int localClientNum, UiContext *dc, itemDef_s *item)
{
    focusItemDef_s *focusPtr; // [esp+0h] [ebp-4h]

    focusPtr = Item_GetFocusItemDef(item);
    if ( focusPtr )
    {
        if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 5565, 0, "%s", "item") )
            __debugbreak();
        if ( item )
        {
            if ( (Window_GetDynamicFlags(dc->contextIndex, &item->window) & 0x40) != 0 )
            {
                Item_RunScript(localClientNum, dc, item, focusPtr->mouseExitText);
                Window_RemoveDynamicFlags(dc->contextIndex, &item->window, 64);
            }
            Item_RunScript(localClientNum, dc, item, focusPtr->mouseExit);
            Window_RemoveDynamicFlags(dc->contextIndex, &item->window, 768);
            if ( item->type == 4 )
                Window_RemoveDynamicFlags(dc->contextIndex, &item->window, 7936);
        }
    }
}

void __cdecl Item_SetMouseOver(const UiContext *dc, itemDef_s *item, int focus)
{
    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 5588, 0, "%s", "item") )
        __debugbreak();
    if ( item )
    {
        if ( focus )
            Window_AddDynamicFlags(dc->contextIndex, &item->window, 1);
        else
            Window_RemoveDynamicFlags(dc->contextIndex, &item->window, 1);
    }
}

bool __cdecl IsVisible(char flags)
{
    return (flags & 4) != 0 && (flags & 0x10) == 0;
}

char    Menu_IsVisible(int localClientNum, UiContext *dc, menuDef_t *menu)
{
        if ( !Window_IsVisible(dc->contextIndex, &menu->window) )
        return 0;
    if ( menu->window.ownerDrawFlags && !UI_OwnerDrawVisible(menu->window.ownerDrawFlags) )
        return 0;
    if ( localClientNum == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    10251,
                    0,
                    "%s",
                    "localClientNum != INVALID_CLIENT_NUMBER") )
    {
        __debugbreak();
    }
    if ( !CL_LocalClient_IsCUIFlagSet(localClientNum, 32) )
        goto LABEL_18;
    if ( (menu->window.staticFlags & 0x20000000) != 0 && CG_ScopeIsOverlayed(localClientNum) )
        return 0;
    if ( (menu->window.staticFlags & 0x10000000) != 0 && (CG_Flashbanged(localClientNum) || CG_Flared(localClientNum)) )
        return 0;
LABEL_18:
    if ( (menu->window.staticFlags & 0x40000000) != 0
        && (CL_GetLocalClientUIGlobals(localClientNum)->keyCatchers & 0x10) != 0 )
    {
        return 0;
    }

    itemDef_s dummyDef; // [esp+0h] [ebp-110h]
    dummyDef.parent = menu;

    if (__PAIR64__(
        HIDWORD(sharedUiInfo.visibilityBits[localClientNum]) & HIDWORD(menu->showBits),
        sharedUiInfo.visibilityBits[localClientNum] & LODWORD(menu->showBits)) != menu->showBits
        || __PAIR64__(
            HIDWORD(sharedUiInfo.visibilityBits[localClientNum]) | HIDWORD(menu->hideBits),
            LODWORD(sharedUiInfo.visibilityBits[localClientNum]) | LODWORD(menu->hideBits)) != menu->hideBits)
    {
        return 0;
    }

    if ( !menu->visibleExp.filename || IsExpressionTrue(localClientNum, &dummyDef, &menu->visibleExp) )
        return 1;
    if ( uiscript_debug && uiscript_debug->current.integer )
    {
        if ( menu->window.name )
            Com_Printf(13, "hiding the %s menu becuase the 'visible when' expression was false\n", menu->window.name);
        else
            Com_Printf(13, "hiding the %s menu becuase the 'visible when' expression was false\n", "unnamed");
    }
    return 0;
}

void __cdecl Menu_PerformTransitionEffects(int localClientNum, UiContext *dc, menuDef_t *menu)
{
    double v3; // xmm0_8
    float v4; // xmm0_4
    double v5; // xmm0_8
    float v6; // xmm0_4
    int v7; // [esp-20h] [ebp-3Ch]
    int v8; // [esp-14h] [ebp-30h]
    signed int v9; // [esp-Ch] [ebp-28h]
    signed int v10; // [esp-Ch] [ebp-28h]
    float openFadingTime; // [esp-8h] [ebp-24h]
    float v12; // [esp-8h] [ebp-24h]
    float openSlideSpeed; // [esp-4h] [ebp-20h]
    float v14; // [esp-4h] [ebp-20h]
    int timeElapsed; // [esp+0h] [ebp-1Ch]
    float fadeTime; // [esp+4h] [ebp-18h]
    unsigned __int8 direction[4]; // [esp+Ch] [ebp-10h]

    direction[3] = 0;
    direction[2] = 0;
    direction[1] = 0;
    direction[0] = 0;
    timeElapsed = -1;
    openSlideSpeed = 0.0f;
    openFadingTime = 0.0f;
    if ((Window_GetDynamicFlags(dc->contextIndex, &menu->window) & 0x20000) != 0)
    {
        direction[3] = 1;
        timeElapsed = menu->openSlideDirection;
        openSlideSpeed = (float)menu->openSlideSpeed;
        Window_RemoveDynamicFlags(dc->contextIndex, &menu->window, 0x40000);
    }
    if ((Window_GetDynamicFlags(dc->contextIndex, &menu->window) & 0x40000) != 0)
    {
        direction[2] = 1;
        timeElapsed = menu->closeSlideDirection;
        openSlideSpeed = (float)menu->closeSlideSpeed;
        Window_RemoveDynamicFlags(dc->contextIndex, &menu->window, 0x20000);
    }
    if ((Window_GetDynamicFlags(dc->contextIndex, &menu->window) & 0x80000) != 0)
    {
        direction[1] = 1;
        openFadingTime = (float)menu->openFadingTime;
        Window_RemoveDynamicFlags(dc->contextIndex, &menu->window, 0x2000);
    }
    if ((Window_GetDynamicFlags(dc->contextIndex, &menu->window) & 0x2000) != 0)
    {
        direction[0] = 1;
        openFadingTime = (float)menu->closeFadingTime;
        Window_RemoveDynamicFlags(dc->contextIndex, &menu->window, 0x80000);
    }
    if (direction[3] || direction[2])
    {
        v9 = Sys_Milliseconds() - menu->slideTimeCounter;
        v14 = (float)(1.0 / ui_animSpeedScale->current.value) * openSlideSpeed;
        if ((int)v14 < v9)
            v8 = (int)v14;
        else
            v8 = v9;
        //v3 = __libm_sse2_sin((double)v8 * (1.5707 / v14));
        v3 = sin((double)v8 * (1.5707 / v14));
        //v4 = __libm_sse2_sin(v3 * 1.5707);
        v4 = sin(v3 * 1.5707);
        if (cls.vidConfig.aspectRatioWindow == 0.0)
            fadeTime = 480.0f;
        else
            fadeTime = 480.0 * cls.vidConfig.aspectRatioWindow;
        switch (timeElapsed)
        {
        case 0:
            if (direction[3])
            {
                menu->window.rect.x = (-(menu->initialRectInfo.w))
                    + (float)((float)(menu->initialRectInfo.w + menu->initialRectInfo.x) * v4);
                menu->window.rect.y = menu->initialRectInfo.y;
            }
            else
            {
                menu->window.rect.x = (float)((float)(fadeTime - menu->initialRectInfo.x) * v4) + menu->initialRectInfo.x;
            }
            break;
        case 1:
            if (direction[3])
            {
                menu->window.rect.x = fadeTime - (float)((float)(fadeTime - menu->initialRectInfo.x) * v4);
                menu->window.rect.y = menu->initialRectInfo.y;
            }
            else
            {
                menu->window.rect.x = menu->initialRectInfo.x - (float)((float)(menu->initialRectInfo.x + fadeTime) * v4);
            }
            break;
        case 2:
            if (direction[3])
            {
                menu->window.rect.y = (-(480.0 - menu->initialRectInfo.y))
                    + (float)(480.0 * v4);
                menu->window.rect.x = menu->initialRectInfo.x;
            }
            else
            {
                menu->window.rect.y = (float)((float)(480.0 - menu->initialRectInfo.y) * v4) + menu->initialRectInfo.y;
            }
            break;
        case 3:
            if (direction[3])
            {
                menu->window.rect.y = 480.0 - (float)((float)(480.0 - menu->initialRectInfo.y) * v4);
                menu->window.rect.x = menu->initialRectInfo.x;
            }
            else
            {
                menu->window.rect.y = menu->initialRectInfo.y
                    - (float)((float)(menu->initialRectInfo.y + menu->window.rect.h) * v4);
            }
            break;
        default:
            Com_Printf(13, "Invalid value for openSlideDirection.");
            break;
        }
        if ((float)v9 >= v14)
        {
            if (direction[3])
            {
                Window_RemoveDynamicFlags(dc->contextIndex, &menu->window, 0x20000);
                if (dc->Menus[2 * dc->openMenuCount + 599] == menu)
                    Menu_CallOnFocusDueToOpen(localClientNum, dc, menu);
                menu->slideTimeCounter = -1;
            }
            else
            {
                Window_RemoveDynamicFlags(dc->contextIndex, &menu->window, 0x40000);
                menu->slideTimeCounter = -3;
            }
        }
    }
    if (direction[1] || direction[0])
    {
        v10 = Sys_Milliseconds() - menu->fadeTimeCounter;
        v12 = (float)(1.0 / ui_animSpeedScale->current.value) * openFadingTime;
        v7 = (int)v12 < v10 ? (int)v12 : v10;
        //v5 = __libm_sse2_sin((double)v7 * (1.5707 / v12));
        v5 = sin((double)v7 * (1.5707 / v12));
        //v6 = __libm_sse2_sin(v5 * 1.5707);
        v6 = sin(v5 * 1.5707);
        if (direction[1])
        {
            menu->window.foreColor[3] = v6;
            menu->window.backColor[3] = v6;
        }
        else
        {
            menu->window.foreColor[3] = 1.0 - v6;
            menu->window.backColor[3] = 1.0 - v6;
            v6 = 1.0 - v6;
        }
        menu->window.borderColor[3] = v6;
        if ((float)v10 >= v12)
        {
            if (direction[1])
            {
                Window_RemoveDynamicFlags(dc->contextIndex, &menu->window, 0x80000);
                if (dc->Menus[2 * dc->openMenuCount + 599] == menu)
                    Menu_CallOnFocusDueToOpen(localClientNum, dc, menu);
                menu->fadeTimeCounter = -1;
            }
            else
            {
                Window_RemoveDynamicFlags(dc->contextIndex, &menu->window, 0x2000);
                menu->fadeTimeCounter = -3;
            }
        }
    }
}

char    Menu_Paint(
                int localClientNum,
                UiContext *dc,
                ScreenPlacementStack *scrPlaceViewStack,
                menuDef_t *menu,
                const ScreenPlacement *UI3DOverrideId)
{
    const char *v7; // eax
    float alpha; // [esp+3Ch] [ebp-1B0h]
    const rectDef_s *rect; // [esp+54h] [ebp-198h]
    const rectDef_s *recta; // [esp+54h] [ebp-198h]
    int i; // [esp+90h] [ebp-15Ch]
    int ia; // [esp+90h] [ebp-15Ch]
    int ib; // [esp+90h] [ebp-15Ch]
    bool anyItemVisible; // [esp+97h] [ebp-155h]
    ScopedUI3DStack windowIdStack; // [esp+B8h] [ebp-134h]
    GfxUI3DStack *UI3DStack; // [esp+BCh] [ebp-130h]
    const ScreenPlacement *scrPlace; // [esp+C0h] [ebp-12Ch]
    int ui3dWindow; // [esp+C8h] [ebp-124h]
    int ui3dWindowId; // [esp+CCh] [ebp-120h]
    itemDef_s dummyDef; // [esp+D0h] [ebp-11Ch] BYREF
    //_UNKNOWN *v23[2]; // [esp+1E0h] [ebp-Ch] BYREF
    //ScreenPlacementStack *scrPlaceViewStacka; // [esp+1ECh] [ebp+0h]
    //
    //v23[0] = a1;
    //v23[1] = scrPlaceViewStacka;
    dummyDef.parent = menu;

    iassert(menu);

    if (menu->ui3dWindowId < 0)
        ui3dWindowId = -1;
    else
        ui3dWindowId = menu->ui3dWindowId;
    ui3dWindow = ui3dWindowId;
    if (ui3dWindowId == -1 && (int)UI3DOverrideId >= 0)
        ui3dWindow = (int)UI3DOverrideId;
    if (ui3dWindow < 0)
        scrPlace = &scrPlaceView[dc->contextIndex];
    else
        scrPlace = R_UI3D_ScrPlaceFromTextureWindow(ui3dWindow);
    UI3DStack = R_GetUI3DStack();
    windowIdStack.mStack = UI3DStack;
    R_UI3DStack_Push(UI3DStack, ui3dWindow);

    iassert((scrPlaceViewStack == 0) || scrPlaceViewStack->contextIndex == dc->contextIndex);

    ScopedScrPlaceViewStack scopedScrPlaceStack(scrPlaceViewStack, scrPlace); // [esp+B4h] [ebp-138h] BYREF
    //ScopedScrPlaceViewStack::ScopedScrPlaceViewStack(&scopedScrPlaceStack, scrPlaceViewStack, scrPlace);
    if (*(_BYTE *)ui_showMenuOnly->current.integer
        && menu->window.name
        && I_stricmp(menu->window.name, ui_showMenuOnly->current.string))
    {
        R_UI3DStack_Pop(windowIdStack.mStack);
        return 0;
    }

    {
        PROF_SCOPED("Menu_IsVisible");
        if (!Menu_IsVisible(localClientNum, dc, menu))
        {
            R_UI3DStack_Pop(windowIdStack.mStack);
            return 0;
        }
    }

    {
        PROF_SCOPED("Menu_AnyItemsVisible");
        anyItemVisible = 0;
        for (i = 0; i < menu->itemCount && !anyItemVisible; ++i)
        {
            anyItemVisible = (menu->items[i]->window.staticFlags & 0x100000) == 0;
            if (menu->items[i]->animInfo && menu->items[i]->animInfo->animating)
                anyItemVisible = 1;
            if (menu->items[i]->window.foreColor[3] > 0.000001)
                anyItemVisible = 1;
        }
        if (menu->itemCount > 0 && !anyItemVisible)
        {
            R_UI3DStack_Pop(windowIdStack.mStack);
            return 0;
        }
    }
    
    if (menu->soundName && !CL_IsLocalClientInGame(localClientNum))
        UI_PlaySound(dc->contextIndex, (char *)menu->soundName);

    if (menu->blurRadius != 0.0 && !zombietron->current.enabled)
        dc->blurRadiusOut = sqrtf((float)(menu->blurRadius * menu->blurRadius) + (float)(dc->blurRadiusOut * dc->blurRadiusOut));

    {
        PROF_SCOPED("Item_UpdateAnimation");
        for (ia = 0; ia < menu->itemCount; ++ia)
        {
            if (menu->items[ia]->animInfo && menu->items[ia]->animInfo->animating)
                Item_UpdateAnimation(localClientNum, dc, menu->items[ia]);
        }
    }
    
    Menu_PerformTransitionEffects(localClientNum, dc, menu);

    {
        PROF_SCOPED("Menu_UpdatePosition");
        if (menu->rectXExp.filename)
            menu->window.rect.x = GetExpressionFloat(localClientNum, &dummyDef, &menu->rectXExp);
        if (menu->rectYExp.filename)
            menu->window.rect.y = GetExpressionFloat(localClientNum, &dummyDef, &menu->rectYExp);
        Menu_UpdatePosition(dc->contextIndex, menu);
    }
    
    if (menu->fullScreen)
    {
        if (menu->window.background)
        {
            PROF_SCOPED("UI_DrawHandlePic");
            rect = Window_GetRect(&menu->window);
            UI_DrawHandlePic(
                &scrPlaceView[dc->contextIndex],
                0.0,
                0.0,
                640.0,
                480.0,
                rect->horzAlign,
                rect->vertAlign,
                0,
                menu->window.background);
        }
    }

    {
        PROF_SCOPED("Window_Paint");
        Window_Paint(
            localClientNum,
            dc,
            &menu->window,
            menu->fadeAmount,
            menu->fadeInAmount,
            menu->fadeClamp,
            (float)menu->fadeCycle,
            -1,
            0);
    }
    
    {
        v7 = va("Item_Paint (%d)", menu->itemCount);
        PROF_SCOPED_RUNTIME_NAME(v7);
        for (ib = 0; ib < menu->itemCount; ++ib)
        {
            if ((Window_GetDynamicFlags(dc->contextIndex, &menu->window) & 0x82000) != 0 || menu->fadeTimeCounter == -3)
            {
                if (menu->fadeTimeCounter == -3)
                    break;
                alpha = menu->items[ib]->window.foreColor[3];
                menu->items[ib]->window.foreColor[3] = alpha * menu->window.foreColor[3];
                menu->items[ib]->window.borderColor[3] = menu->items[ib]->window.borderColor[3] * menu->window.foreColor[3];
                menu->items[ib]->window.backColor[3] = menu->items[ib]->window.backColor[3] * menu->window.foreColor[3];
                Item_Paint(localClientNum, dc, menu->items[ib]);
                menu->items[ib]->window.foreColor[3] = alpha;
            }
            else
            {
                Item_Paint(localClientNum, dc, menu->items[ib]);
            }
        }
    }
    
    menu->window.rect.x = menu->initialRectInfo.x;
    menu->window.rect.y = menu->initialRectInfo.y;

    {
        PROF_SCOPED("g_debugMode");

        if (g_debugMode)
        {
            recta = Window_GetRect(&menu->window);
            UI_DrawRect(
                &scrPlaceView[dc->contextIndex],
                recta->x,
                recta->y,
                recta->w,
                recta->h,
                recta->horzAlign,
                recta->vertAlign,
                1.0,
                colorMagenta);
        }
    }
    
    //ScopedScrPlaceViewStack::~ScopedScrPlaceViewStack(&scopedScrPlaceStack);
    R_UI3DStack_Pop(windowIdStack.mStack);
    return 1;
}

void __cdecl Window_Paint(
                int localClientNum,
                UiContext *dc,
                windowDef_t *w,
                float fadeAmount,
                float fadeInAmount,
                float fadeClamp,
                float fadeCycle,
                int itemType,
                itemDef_s *item)
{
    float v9; // xmm0_4
    float v10; // xmm0_4
    float *v11; // [esp+2Ch] [ebp-D8h]
    float *v12; // [esp+30h] [ebp-D4h]
    float *v13; // [esp+34h] [ebp-D0h]
    float *v14; // [esp+38h] [ebp-CCh]
    float *v15; // [esp+3Ch] [ebp-C8h]
    float *v16; // [esp+40h] [ebp-C4h]
    float *v17; // [esp+44h] [ebp-C0h]
    float v18; // [esp+4Ch] [ebp-B8h]
    float value; // [esp+54h] [ebp-B0h]
    listBoxDef_s *listPtr; // [esp+7Ch] [ebp-88h]
    rectDef_s fillRect; // [esp+80h] [ebp-84h]
    float lowColor[4]; // [esp+98h] [ebp-6Ch] BYREF
    float lowLight[4]; // [esp+A8h] [ebp-5Ch] BYREF
    const ScreenPlacement *scrPlace; // [esp+B8h] [ebp-4Ch]
    const float *foreColor; // [esp+BCh] [ebp-48h]
    rectDef_s borderRect; // [esp+C0h] [ebp-44h]
    unsigned __int8 style; // [esp+DBh] [ebp-29h]
    float oldColor[4]; // [esp+DCh] [ebp-28h] BYREF
    int flags; // [esp+ECh] [ebp-18h] BYREF
    const rectDef_s *origRect; // [esp+F0h] [ebp-14h]
    float newColor[4]; // [esp+F4h] [ebp-10h] BYREF

    origRect = Window_GetRect(w);
    fillRect = *origRect;
    borderRect.x = origRect->x;
    borderRect.y = fillRect.y;
    borderRect.w = fillRect.w;
    borderRect.h = fillRect.h;
    *(_QWORD *)&borderRect.horzAlign = *(_QWORD *)&fillRect.horzAlign;
    scrPlace = &scrPlaceView[dc->contextIndex];
    if (itemType == 4)
    {
        listPtr = Item_GetListBoxDef(item);
        if (!listPtr && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 533, 0, "%s", "listPtr"))
            __debugbreak();
        if (!listPtr->noScrollBars && !Item_ListBox_RequiresScroll(localClientNum, dc->contextIndex, item))
        {
            borderRect.w = borderRect.w - 14.0;
            fillRect.w = fillRect.w - 14.0;
        }
    }
    if (g_debugMode)
        UI_DrawRect(
            scrPlace,
            borderRect.x,
            borderRect.y,
            borderRect.w,
            borderRect.h,
            origRect->horzAlign,
            origRect->vertAlign,
            1.0,
            colorWhite);
    if (w && (w->style || w->border))
    {
        if (w->border && w->borderSize > 0.0)
        {
            fillRect.x = fillRect.x + w->borderSize;
            fillRect.y = fillRect.y + w->borderSize;
            fillRect.w = fillRect.w - (float)(w->borderSize + 1.0);
            fillRect.h = fillRect.h - (float)(w->borderSize + 1.0);
        }
        style = w->style;
        if (style == 12)
            style = 3;
        switch (style)
        {
        case 1u:
            if (w->background)
            {
                flags = Window_GetDynamicFlags(dc->contextIndex, w);
                Fade(&flags, &w->backColor[3], fadeClamp, &w->nextTime, (int)fadeCycle, 1, fadeAmount, fadeInAmount, dc);
                Window_SetDynamicFlags(dc->contextIndex, w, flags);
                if (w->rotation == 0.0)
                    UI_DrawHandlePic(
                        scrPlace,
                        fillRect.x,
                        fillRect.y,
                        fillRect.w,
                        fillRect.h,
                        origRect->horzAlign,
                        origRect->vertAlign,
                        w->backColor,
                        w->background);
                else
                    UI_DrawHandlePicRotated(
                        scrPlace,
                        fillRect.x,
                        fillRect.y,
                        fillRect.w,
                        fillRect.h,
                        origRect->horzAlign,
                        origRect->vertAlign,
                        w->backColor,
                        w->rotation,
                        w->background);
            }
            else if (w->rotation == 0.0)
            {
                UI_FillRect(
                    scrPlace,
                    fillRect.x,
                    fillRect.y,
                    fillRect.w,
                    fillRect.h,
                    origRect->horzAlign,
                    origRect->vertAlign,
                    w->backColor);
            }
            else
            {
                UI_DrawHandlePicRotated(
                    scrPlace,
                    fillRect.x,
                    fillRect.y,
                    fillRect.w,
                    fillRect.h,
                    origRect->horzAlign,
                    origRect->vertAlign,
                    w->backColor,
                    w->rotation,
                    sharedUiInfo.assets.whiteMaterial);
            }
            break;
        case 2u:
            //BG_EvalVehicleName();
            break;
        case 3u:
            if ((Window_GetDynamicFlags(dc->contextIndex, &item->parent->window) & 0x2000) != 0)
            {
                foreColor = w->foreColor;
            }
            else
            {
                if ((Window_GetDynamicFlags(dc->contextIndex, w) & 0x10000) != 0)
                    v17 = w->foreColor;
                else
                    v17 = 0;
                foreColor = v17;
            }
            if (itemType == 17)
            {
                oldColor[0] = *foreColor;
                oldColor[1] = foreColor[1];
                oldColor[2] = foreColor[2];
                oldColor[3] = foreColor[3];
                lowLight[0] = 0.80000001 * oldColor[0];
                lowLight[1] = 0.80000001 * oldColor[1];
                lowLight[2] = 0.80000001 * oldColor[2];
                lowLight[3] = 0.80000001 * oldColor[3];
                //v9 = __libm_sse2_sin((float)(dc->realTime / 150));
                v9 = sin((float)(dc->realTime / 150));
                LerpColor(oldColor, lowLight, newColor, (float)(v9 * 0.5) + 0.5);
                if (w->rotation == 0.0)
                    UI_DrawHandlePic(
                        scrPlace,
                        fillRect.x,
                        fillRect.y,
                        fillRect.w,
                        fillRect.h,
                        origRect->horzAlign,
                        origRect->vertAlign,
                        newColor,
                        w->background);
                else
                    UI_DrawHandlePicRotated(
                        scrPlace,
                        fillRect.x,
                        fillRect.y,
                        fillRect.w,
                        fillRect.h,
                        origRect->horzAlign,
                        origRect->vertAlign,
                        newColor,
                        w->rotation,
                        w->background);
            }
            else
            {
                if (w->rotation != 0.0)
                    goto LABEL_42;
                UI_DrawHandlePic(
                    scrPlace,
                    fillRect.x,
                    fillRect.y,
                    fillRect.w,
                    fillRect.h,
                    origRect->horzAlign,
                    origRect->vertAlign,
                    foreColor,
                    w->background);
            }
            break;
        case 5u:
            if (w->background)
            {
                if ((Window_GetDynamicFlags(dc->contextIndex, w) & 0x10000) != 0)
                    v14 = w->foreColor;
                else
                    v14 = 0;
                foreColor = v14;
                if (w->rotation == 0.0)
                    UI_DrawHandlePic(
                        scrPlace,
                        fillRect.x,
                        fillRect.y,
                        fillRect.w,
                        fillRect.h,
                        origRect->horzAlign,
                        origRect->vertAlign,
                        foreColor,
                        w->background);
                else
                    LABEL_42:
                UI_DrawHandlePicRotated(
                    scrPlace,
                    fillRect.x,
                    fillRect.y,
                    fillRect.w,
                    fillRect.h,
                    origRect->horzAlign,
                    origRect->vertAlign,
                    foreColor,
                    w->rotation,
                    w->background);
            }
            break;
        case 6u:
            if ((Window_GetDynamicFlags(dc->contextIndex, w) & 0x10000) != 0)
                v16 = w->foreColor;
            else
                v16 = 0;
            foreColor = v16;
            UI_DrawLoadBar(
                scrPlace,
                fillRect.x,
                fillRect.y,
                fillRect.w,
                fillRect.h,
                origRect->horzAlign,
                origRect->vertAlign,
                v16,
                w->background);
            break;
        case 7u:
            if (sharedUiInfo.loadingScreen)
            {
                if ((Window_GetDynamicFlags(dc->contextIndex, w) & 0x10000) != 0)
                    v15 = w->foreColor;
                else
                    v15 = 0;
                foreColor = v15;
                UI_DrawHandlePic(
                    scrPlace,
                    fillRect.x,
                    fillRect.y,
                    fillRect.w,
                    fillRect.h,
                    origRect->horzAlign,
                    origRect->vertAlign,
                    v15,
                    sharedUiInfo.loadingScreen);
            }
            break;
        case 8u:
            if ((Window_GetDynamicFlags(dc->contextIndex, &item->parent->window) & 0x2000) != 0)
            {
                foreColor = w->foreColor;
            }
            else
            {
                if ((Window_GetDynamicFlags(dc->contextIndex, w) & 0x10000) != 0)
                    v13 = w->foreColor;
                else
                    v13 = 0;
                foreColor = v13;
            }
            UI_DrawSpinner(
                scrPlace,
                fillRect.x,
                fillRect.y,
                fillRect.w,
                fillRect.h,
                origRect->horzAlign,
                origRect->vertAlign,
                foreColor);
            break;
        case 9u:
            if ((Window_GetDynamicFlags(dc->contextIndex, &item->parent->window) & 0x2000) != 0)
            {
                foreColor = w->foreColor;
            }
            else
            {
                if ((Window_GetDynamicFlags(dc->contextIndex, w) & 0x10000) != 0)
                    v12 = w->foreColor;
                else
                    v12 = 0;
                foreColor = v12;
            }
            UI_DrawSpinnerLoadbar(
                scrPlace,
                fillRect.x,
                fillRect.y,
                fillRect.w,
                fillRect.h,
                origRect->horzAlign,
                origRect->vertAlign,
                foreColor);
            break;
        case 0xAu:
            if ((Window_GetDynamicFlags(dc->contextIndex, &item->parent->window) & 0x2000) != 0)
            {
                foreColor = w->foreColor;
            }
            else
            {
                if ((Window_GetDynamicFlags(dc->contextIndex, w) & 0x10000) != 0)
                    v11 = w->foreColor;
                else
                    v11 = 0;
                foreColor = v11;
            }
            if (itemType == 17)
            {
                oldColor[0] = *foreColor;
                oldColor[1] = foreColor[1];
                oldColor[2] = foreColor[2];
                oldColor[3] = foreColor[3];
                lowLight[0] = 0.80000001 * oldColor[0];
                lowLight[1] = 0.80000001 * oldColor[1];
                lowLight[2] = 0.80000001 * oldColor[2];
                lowLight[3] = 0.80000001 * oldColor[3];
                //v10 = __libm_sse2_sin((float)(dc->realTime / 150));
                v10 = sin((float)(dc->realTime / 150));
                LerpColor(oldColor, lowLight, newColor, (float)(v10 * 0.5) + 0.5);
                UI_DrawHandlePicFramed(
                    scrPlace,
                    fillRect.x,
                    fillRect.y,
                    fillRect.w,
                    fillRect.h,
                    origRect->horzAlign,
                    origRect->vertAlign,
                    w->frameSize,
                    w->frameTexSize,
                    w->frameSides,
                    newColor,
                    w->background);
            }
            else
            {
                UI_DrawHandlePicFramed(
                    scrPlace,
                    fillRect.x,
                    fillRect.y,
                    fillRect.w,
                    fillRect.h,
                    origRect->horzAlign,
                    origRect->vertAlign,
                    w->frameSize,
                    w->frameTexSize,
                    w->frameSides,
                    foreColor,
                    w->background);
            }
            break;
        case 0xBu:
            if (w->background)
            {
                flags = Window_GetDynamicFlags(dc->contextIndex, w);
                Fade(&flags, &w->backColor[3], fadeClamp, &w->nextTime, (int)fadeCycle, 1, fadeAmount, fadeInAmount, dc);
                Window_SetDynamicFlags(dc->contextIndex, w, flags);
                UI_DrawHandlePicFramed(
                    scrPlace,
                    fillRect.x,
                    fillRect.y,
                    fillRect.w,
                    fillRect.h,
                    origRect->horzAlign,
                    origRect->vertAlign,
                    w->frameSize,
                    w->frameTexSize,
                    w->frameSides,
                    w->backColor,
                    w->background);
            }
            break;
        default:
            break;
        }
        switch (w->border)
        {
        case 1u:
            UI_DrawRect(
                scrPlace,
                borderRect.x,
                borderRect.y,
                borderRect.w,
                borderRect.h,
                origRect->horzAlign,
                origRect->vertAlign,
                w->borderSize,
                w->borderColor);
            break;
        case 2u:
            UI_DrawTopBottom(
                scrPlace,
                borderRect.x,
                borderRect.y,
                borderRect.w,
                borderRect.h,
                origRect->horzAlign,
                origRect->vertAlign,
                w->borderSize,
                w->borderColor);
            break;
        case 3u:
            UI_DrawSides(
                scrPlace,
                borderRect.x,
                borderRect.y,
                borderRect.w,
                borderRect.h,
                origRect->horzAlign,
                origRect->vertAlign,
                w->borderSize,
                w->borderColor);
            break;
        case 4u:
            //BG_EvalVehicleName();
            //BG_EvalVehicleName();
            break;
        case 5u:
            value = ui_borderLowLightScale->current.value;
            lowColor[0] = value * w->borderColor[0];
            lowColor[1] = value * w->borderColor[1];
            lowColor[2] = value * w->borderColor[2];
            lowColor[3] = w->borderColor[3];
            UI_DrawHighlightRect(
                scrPlace,
                borderRect.x,
                borderRect.y,
                borderRect.w,
                borderRect.h,
                origRect->horzAlign,
                origRect->vertAlign,
                w->borderSize,
                w->borderColor,
                lowColor);
            break;
        case 6u:
            v18 = ui_borderLowLightScale->current.value;
            lowColor[0] = v18 * w->borderColor[0];
            lowColor[1] = v18 * w->borderColor[1];
            lowColor[2] = v18 * w->borderColor[2];
            lowColor[3] = w->borderColor[3];
            UI_DrawHighlightRect(
                scrPlace,
                borderRect.x,
                borderRect.y,
                borderRect.w,
                borderRect.h,
                origRect->horzAlign,
                origRect->vertAlign,
                w->borderSize,
                lowColor,
                w->borderColor);
            break;
        }
    }
}

void __cdecl Item_Paint(int localClientNum, UiContext *dc, itemDef_s *item)
{
    menuDef_t *v3; // esi
    menuDef_t *v4; // esi
    const char *String; // eax
    char *ExpressionResultString; // eax
    rectDef_s *r; // [esp+40h] [ebp-60h]
    menuDef_t *parent; // [esp+54h] [ebp-4Ch]
    char lowerCaseName[68]; // [esp+58h] [ebp-48h] BYREF
    int savedregs; // [esp+A0h] [ebp+0h] BYREF

    parent = item->parent;
    if ( item )
    {
        if ( item->window.ownerDrawFlags )
        {
            if ( UI_OwnerDrawVisible(item->window.ownerDrawFlags) )
                Window_AddDynamicFlags(dc->contextIndex, &item->window, 4);
            else
                Window_RemoveDynamicFlags(dc->contextIndex, &item->window, 4);
        }
        if ( (item->dvarFlags & 0xC) == 0 || Item_EnableShowViaDvar(item, 4) )
        {
            if ( item->forecolorAExp.filename )
                item->window.foreColor[3] = GetExpressionFloat(localClientNum, item, &item->forecolorAExp);
            //if ( item->window.name )
            //    //PIXBeginNamedEvent(-1, "Item_IsVisible %s", item->window.name);
            //else
            //    //PIXBeginNamedEvent(-1, "Item_IsVisible");
            if ( Item_IsVisible(localClientNum, dc->contextIndex, item) )
            {
                //if ( GetCurrentThreadId() == g_DXDeviceThread )
                    //D3DPERF_EndEvent();
                if ( item->rectExpData && (!item->animInfo || !item->animInfo->animating) )
                {
                    if ( item->rectExpData->rectXExp.filename )
                    {
                        v3 = item->parent;
                        item->window.rect.x = GetExpressionFloat(localClientNum, item, &item->rectExpData->rectXExp)
                                                                + v3->window.rect.x;
                    }
                    if ( item->rectExpData->rectYExp.filename )
                    {
                        v4 = item->parent;
                        item->window.rect.y = GetExpressionFloat(localClientNum, item, &item->rectExpData->rectYExp)
                                                                + v4->window.rect.y;
                    }
                    if ( item->rectExpData->rectWExp.filename )
                        item->window.rect.w = GetExpressionFloat(localClientNum, item, &item->rectExpData->rectWExp);
                    if ( item->rectExpData->rectHExp.filename )
                        item->window.rect.h = GetExpressionFloat(localClientNum, item, &item->rectExpData->rectHExp);
                }
                if ( item->window.style == 5 )
                {
                    String = Dvar_GetString(item->dvar);
                    I_strncpyz(lowerCaseName, String, 64);
                    I_strlwr(lowerCaseName);
                    item->window.background = Material_RegisterHandle(lowerCaseName, item->imageTrack);
                }
                else if ( item->type == 2 && LODWORD(item->typeData.textDef->textRect[0].x) )
                {
                    ExpressionResultString = GetExpressionResultString(
                                                                         localClientNum,
                                                                         item,
                                                                         (ExpressionStatement *)item->typeData.textDef);
                    I_strncpyz(lowerCaseName, ExpressionResultString, 64);
                    I_strlwr(lowerCaseName);
                    item->window.background = Material_RegisterHandle(lowerCaseName, item->imageTrack);
                }
                Window_Paint(
                    localClientNum,
                    dc,
                    &item->window,
                    parent->fadeAmount,
                    parent->fadeInAmount,
                    parent->fadeClamp,
                    (float)parent->fadeCycle,
                    item->type,
                    item);
                if ( g_debugMode )
                {
                    r = Item_CorrectedTextRect(dc->contextIndex, item);
                    UI_DrawRect(
                        &scrPlaceView[dc->contextIndex],
                        r->x,
                        r->y,
                        r->w,
                        r->h,
                        r->horzAlign,
                        r->vertAlign,
                        1.0,
                        colorGreen);
                }
                if ( item->window.style != 5 )
                {
                    switch ( item->type )
                    {
                        case 1:
                        case 3:
                            Item_Text_Paint(localClientNum, dc, item);
                            break;
                        case 4:
                            Item_ListBox_Paint(localClientNum, dc, item);
                            break;
                        case 5:
                        case 7:
                        case 0xD:
                        case 0xE:
                        case 0x16:
                        case 0x1E:
                            Item_TextField_Paint(localClientNum, dc, item);
                            break;
                        case 6:
                        case 0x12:
                        case 0x14:
                            Item_OwnerDraw_Paint(localClientNum, dc, item);
                            break;
                        case 8:
                            Item_Slider_Paint(dc, item);
                            break;
                        case 9:
                            Item_YesNo_Paint(localClientNum, dc, item);
                            break;
                        case 0xA:
                            Item_Multi_Paint(localClientNum, dc, item);
                            break;
                        case 0xB:
                            Item_DvarEnum_Paint(localClientNum, dc, item);
                            break;
                        case 0xC:
                        case 0x10:
                            Item_Bind_Paint(localClientNum, dc, item);
                            break;
                        case 0xF:
                            Item_GameMsgWindow_Paint(localClientNum, dc, item);
                            break;
                        default:
                            return;
                    }
                }
            }
            else //if ( GetCurrentThreadId() == g_DXDeviceThread )
            {
                //D3DPERF_EndEvent();
            }
        }
        else if ( Window_HasFocus(dc->contextIndex, &item->window) )
        {
            Menu_HandleKey(localClientNum, dc, parent, 155, 1);
            Menu_HandleKey(localClientNum, dc, parent, 155, 0);
        }
    }
}

float MY_SUBTITLE_GLOWCOLOR[4] = { 0.0, 0.30000001, 0.0, 1.0 };

void __cdecl Item_Text_Paint(int localClientNum, UiContext *dc, itemDef_s *item)
{
    const char *VariantString; // eax
    int v4; // eax
    float arrowSize; // [esp+2Ch] [ebp-850h]
    float rightArrowX; // [esp+30h] [ebp-84Ch]
    float leftArrowX; // [esp+34h] [ebp-848h]
    multiDef_s *multiPtr; // [esp+38h] [ebp-844h]
    Font_s *font; // [esp+3Ch] [ebp-840h]
    bool subtitle; // [esp+43h] [ebp-839h]
    char *textPtr; // [esp+44h] [ebp-838h]
    textDef_s *textDefPtr; // [esp+4Ch] [ebp-830h]
    float glowAlpha; // [esp+50h] [ebp-82Ch]
    bool drawArrows; // [esp+57h] [ebp-825h]
    float color[4]; // [esp+58h] [ebp-824h] BYREF
    const rectDef_s *rect; // [esp+68h] [ebp-814h]
    char text[1024]; // [esp+6Ch] [ebp-810h] BYREF
    const rectDef_s *textRect; // [esp+46Ch] [ebp-410h]
    bool cinematic; // [esp+473h] [ebp-409h]
    char textWithBinding[1028]; // [esp+474h] [ebp-408h] BYREF

    textDefPtr = Item_GetTextDef(item);
    cinematic = (textDefPtr->itemFlags & 2) != 0;
    subtitle = cinematic;
    if ( cinematic )
        textDefPtr->text = "NOT USING CINEMATIC_SUBTITLES";
    if ( textDefPtr && textDefPtr->text )
    {
        textPtr = (char *)textDefPtr->text;
    }
    else if ( textDefPtr && textDefPtr->textExpData && textDefPtr->textExpData->textExp.filename )
    {
        textPtr = GetExpressionResultString(localClientNum, item, &textDefPtr->textExpData->textExp);
    }
    else
    {
        if ( !item->dvar )
            return;
        VariantString = Dvar_GetVariantString(item->dvar);
        I_strncpyz(text, VariantString, 1024);
        textPtr = text;
    }
    if ( *textPtr == 64 )
        textPtr = UI_SafeTranslateString(textPtr + 1);
    if ( *textPtr )
    {
        if ( SEH_ReplaceDirectiveInStringWithBinding(localClientNum, textPtr, textWithBinding) )
            textPtr = textWithBinding;
        Item_TextColor(dc, item, (float (*)[4])color);
        Item_SetTextExtents(dc->contextIndex, item, textPtr);
        if ( (0x800000 & item->window.staticFlags) != 0 )
        {
            Item_Text_AutoWrapped_Paint(dc->contextIndex, item, textPtr, color, subtitle, MY_SUBTITLE_GLOWCOLOR, cinematic);
        }
        else
        {
            textRect = (const rectDef_s *)Item_GetTextRect(dc->contextIndex, item);
            rect = Window_GetRect(&item->window);
            font = Item_GetFont(dc->contextIndex, item);
            glowAlpha = item->window.outlineColor[3];
            drawArrows = 0;
            if ( item->type == 10 && (item->window.dynamicFlags[dc->contextIndex] & 2) != 0 )
            {
                multiPtr = Item_GetMultiDef(item);
                if ( multiPtr )
                    drawArrows = multiPtr->actionOnEnterPressOnly == 0;
            }
            else if ( item->type == 11 && (item->window.dynamicFlags[dc->contextIndex] & 2) != 0 )
            {
                drawArrows = 1;
            }
            if ( drawArrows )
            {
                arrowSize = textRect->h;
                leftArrowX = textRect->x - arrowSize;
                v4 = UI_TextWidth(textPtr, 64, font, textDefPtr->textscale);
                rightArrowX = (float)v4 + textRect->x;
                UI_DrawHandlePic(
                    &scrPlaceView[dc->contextIndex],
                    leftArrowX,
                    textRect->y - arrowSize,
                    arrowSize,
                    arrowSize,
                    rect->horzAlign,
                    rect->vertAlign,
                    color,
                    sharedUiInfo.assets.dvarlistArrowLeft);
                UI_DrawHandlePic(
                    &scrPlaceView[dc->contextIndex],
                    rightArrowX,
                    textRect->y - arrowSize,
                    arrowSize,
                    arrowSize,
                    rect->horzAlign,
                    rect->vertAlign,
                    color,
                    sharedUiInfo.assets.dvarlistArrowRight);
            }
            if ( subtitle )
            {
                UI_DrawTextWithGlow(
                    &scrPlaceView[dc->contextIndex],
                    textPtr,
                    0x7FFFFFFF,
                    font,
                    textRect->x,
                    textRect->y,
                    rect->horzAlign,
                    rect->vertAlign,
                    textDefPtr->textscale,
                    color,
                    textDefPtr->textStyle,
                    MY_SUBTITLE_GLOWCOLOR,
                    subtitle,
                    cinematic);
            }
            else if ( glowAlpha == 0.0 )
            {
                if ( item->window.rotation == 0.0 )
                    UI_DrawText(
                        &scrPlaceView[dc->contextIndex],
                        textPtr,
                        0x7FFFFFFF,
                        font,
                        textRect->x,
                        textRect->y,
                        rect->horzAlign,
                        rect->vertAlign,
                        textDefPtr->textscale,
                        color,
                        textDefPtr->textStyle);
                else
                    UI_DrawTextRotated(
                        &scrPlaceView[dc->contextIndex],
                        textPtr,
                        0x7FFFFFFF,
                        font,
                        textRect->x,
                        textRect->y,
                        rect->horzAlign,
                        rect->vertAlign,
                        textDefPtr->textscale,
                        color,
                        textDefPtr->textStyle,
                        item->window.rotation);
            }
            else
            {
                UI_DrawTextWithGlow(
                    &scrPlaceView[dc->contextIndex],
                    textPtr,
                    0x7FFFFFFF,
                    font,
                    textRect->x,
                    textRect->y,
                    rect->horzAlign,
                    rect->vertAlign,
                    textDefPtr->textscale,
                    color,
                    textDefPtr->textStyle,
                    item->window.outlineColor,
                    0,
                    cinematic);
            }
        }
    }
}

Font_s *__cdecl Item_GetFont(int contextIndex, const itemDef_s *item)
{
    if ( item->typeData.textDef )
        return UI_GetFontHandle(
                         &scrPlaceView[contextIndex],
                         item->typeData.textDef->fontEnum,
                         item->typeData.textDef->textscale);
    else
        return UI_GetFontHandle(&scrPlaceView[contextIndex], 0, 0.55000001);
}

void __cdecl Item_SetTextExtents(int contextIndex, itemDef_s *item, const char *text)
{
    bool v3; // [esp+Ch] [ebp-44h]
    bool v4; // [esp+10h] [ebp-40h]
    Font_s *font; // [esp+14h] [ebp-3Ch]
    bool isOwnerDraw; // [esp+1Bh] [ebp-35h]
    float xAdj; // [esp+1Ch] [ebp-34h]
    rectDef_s newRect; // [esp+28h] [ebp-28h] BYREF
    textDef_s *textDefPtr; // [esp+40h] [ebp-10h]
    const char *dvarString; // [esp+44h] [ebp-Ch]
    bool isDvarField; // [esp+4Bh] [ebp-5h]
    int xAlignMode; // [esp+4Ch] [ebp-4h]

    textDefPtr = Item_GetTextDef(item);
    if ( !text && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 8103, 0, "%s", "text") )
        __debugbreak();
    xAlignMode = textDefPtr->textAlignMode & 3;
    isOwnerDraw = item->type == 6;
    v4 = item->type != 6 && item->dvar;
    isDvarField = v4;
    v3 = xAlignMode && (isOwnerDraw || isDvarField);
    if ( v3 || Item_GetTextRect(contextIndex, item)->textRect[0].w == 0.0 )
    {
        font = Item_GetFont(contextIndex, item);
        newRect.x = textDefPtr->textalignx;
        newRect.y = textDefPtr->textaligny;
        newRect.w = (float)UI_TextWidth(text, 0, font, textDefPtr->textscale);
        newRect.h = (float)UI_TextHeight(font, textDefPtr->textscale);
        newRect.horzAlign = item->window.rect.horzAlign;
        newRect.vertAlign = item->window.rect.vertAlign;
        if ( xAlignMode )
        {
            if ( xAlignMode != 2
                && xAlignMode != 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                            8128,
                            0,
                            "%s\n\t(xAlignMode) = %i",
                            "(xAlignMode == 2 || xAlignMode == 1)",
                            xAlignMode) )
            {
                __debugbreak();
            }
            xAdj = item->window.rect.w - (float)UI_TextWidth(text, 0, font, textDefPtr->textscale);
            if ( isOwnerDraw )
            {
                xAdj = xAdj - (float)UI_OwnerDrawWidth(item->window.ownerDraw, font, textDefPtr->textscale);
            }
            else if ( isDvarField && Item_IsTextField(item) )
            {
                dvarString = Dvar_GetVariantString(item->dvar);
                xAdj = xAdj - (float)UI_TextWidth(dvarString, 0, font, textDefPtr->textscale);
            }
            if ( xAlignMode == 1 )
                xAdj = xAdj * 0.5;
            newRect.x = newRect.x + xAdj;
        }
        newRect.y = Item_GetTextPlacementY(textDefPtr->textAlignMode & 0xC, newRect.y, item->window.rect.h, newRect.h);
        ToWindowCoords(&newRect.x, &newRect.y, &item->window);
        Item_SetTextRect(contextIndex, item, &newRect);
    }
}

void __cdecl ToWindowCoords(float *x, float *y, const windowDef_t *window)
{
    const rectDef_s *Rect; // eax

    Rect = Window_GetRect(window);
    *x = *x + Rect->x;
    *y = *y + Rect->y;
}

void __cdecl Item_Text_AutoWrapped_Paint(
                int contextIndex,
                itemDef_s *item,
                const char *text,
                const float *color,
                bool subtitle,
                const float *subtitleGlowColor,
                bool cinematic)
{
    Font_s *font; // [esp+24h] [ebp-14h]
    textDef_s *textDefPtr; // [esp+28h] [ebp-10h]
    float x; // [esp+2Ch] [ebp-Ch] BYREF
    float y; // [esp+30h] [ebp-8h] BYREF
    const rectDef_s *rect; // [esp+34h] [ebp-4h]

    textDefPtr = Item_GetTextDef(item);
    rect = Window_GetRect(&item->window);
    font = Item_GetFont(contextIndex, item);
    x = textDefPtr->textalignx;
    y = textDefPtr->textaligny;
    ToWindowCoords(&x, &y, &item->window);
    if ( subtitle )
    {
        if ( !subtitleGlowColor
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 8475, 0, "%s", "subtitleGlowColor") )
        {
            __debugbreak();
        }
        UI_DrawWrappedTextSubtitled(
            &scrPlaceView[contextIndex],
            text,
            rect,
            font,
            x,
            y,
            textDefPtr->textscale,
            color,
            textDefPtr->textStyle,
            textDefPtr->textAlignMode,
            &textDefPtr->textRect[contextIndex],
            subtitleGlowColor,
            cinematic);
    }
    else
    {
        UI_DrawWrappedText(
            &scrPlaceView[contextIndex],
            text,
            rect,
            font,
            x,
            y,
            textDefPtr->textscale,
            color,
            textDefPtr->textStyle,
            textDefPtr->textAlignMode,
            &textDefPtr->textRect[contextIndex]);
    }
}

void __cdecl Item_TextField_Paint(int localClientNum, UiContext *dc, itemDef_s *item)
{
    const char *VariantString; // eax
    float v4; // xmm0_4
    int maxPaintChars; // [esp+24h] [ebp-464h]
    int v6; // [esp+28h] [ebp-460h]
    char cursor; // [esp+33h] [ebp-455h]
    editFieldDef_s *editPtr; // [esp+34h] [ebp-454h]
    float lowLight[4]; // [esp+38h] [ebp-450h] BYREF
    Font_s *font; // [esp+48h] [ebp-440h]
    int offset; // [esp+4Ch] [ebp-43Ch]
    menuDef_t *parent; // [esp+50h] [ebp-438h]
    textDef_s *textDefPtr; // [esp+54h] [ebp-434h]
    char buff[1028]; // [esp+58h] [ebp-430h] BYREF
    int maxChars; // [esp+460h] [ebp-28h]
    float x; // [esp+464h] [ebp-24h]
    const rectDef_s *rect; // [esp+468h] [ebp-20h]
    const dvar_s *dvar; // [esp+46Ch] [ebp-1Ch]
    const char *text; // [esp+470h] [ebp-18h]
    const rectDef_s *textRect; // [esp+474h] [ebp-14h]
    float newColor[4]; // [esp+478h] [ebp-10h] BYREF

    parent = item->parent;
    editPtr = Item_GetEditFieldDef(item);
    textDefPtr = Item_GetTextDef(item);
    if (editPtr)
    {
        Item_Text_Paint(localClientNum, dc, item);
        buff[0] = 0;
        if (item->dvar)
        {
            if (item->type == 30)
            {
                dvar = Dvar_FindVar(item->dvar);
                if (dvar && dvar->type == DVAR_TYPE_FLOAT)
                    Com_LocalizedFloatToString(dvar->current.value, buff, 0x400u, 2u);
            }
            else
            {
                VariantString = Dvar_GetVariantString(item->dvar);
                I_strncpyz(buff, VariantString, 1024);
            }
        }
        Item_SetTextExtents(dc->contextIndex, item, buff);
        parent = item->parent;
        if (Window_HasFocus(dc->contextIndex, &item->window))
        {
            lowLight[0] = 0.80000001 * parent->focusColor[0];
            lowLight[1] = 0.80000001 * parent->focusColor[1];
            lowLight[2] = 0.80000001 * parent->focusColor[2];
            lowLight[3] = 0.80000001 * parent->focusColor[3];
            //v4 = __libm_sse2_sin((float)(dc->realTime / 75));
            v4 = sin((float)(dc->realTime / 75));
            LerpColor(parent->focusColor, lowLight, newColor, (float)(v4 * 0.5) + 0.5);
        }
        else
        {
            newColor[0] = item->window.foreColor[0];
            newColor[1] = item->window.foreColor[1];
            newColor[2] = item->window.foreColor[2];
            newColor[3] = item->window.foreColor[3];
        }
        if (textDefPtr->text && *textDefPtr->text)
            v6 = 8;
        else
            v6 = 0;
        offset = v6;
        textRect = (const rectDef_s *)Item_GetTextRect(dc->contextIndex, item);
        x = (float)(textRect->x + textRect->w) + (float)offset;
        text = &buff[editPtr->paintOffset];
        if (editPtr->maxPaintChars)
            maxPaintChars = editPtr->maxPaintChars;
        else
            maxPaintChars = 0x7FFFFFFF;
        maxChars = maxPaintChars;
        font = Item_GetFont(dc->contextIndex, item);
        if (item == g_editItem && g_editingField)
        {
            cursor = Key_GetOverstrikeMode(dc->contextIndex) != 0 ? 95 : 124;
            rect = Window_GetRect(&item->window);
            UI_DrawTextWithCursor(
                &scrPlaceView[dc->contextIndex],
                (char *)text,
                maxChars,
                font,
                x,
                textRect->y,
                rect->horzAlign,
                rect->vertAlign,
                textDefPtr->textscale,
                newColor,
                textDefPtr->textStyle,
                editPtr->cursorPos[dc->contextIndex] - editPtr->paintOffset,
                cursor);
        }
        else
        {
            UI_DrawText(
                &scrPlaceView[dc->contextIndex],
                (char *)text,
                maxChars,
                font,
                x,
                textRect->y,
                textRect->horzAlign,
                textRect->vertAlign,
                textDefPtr->textscale,
                newColor,
                textDefPtr->textStyle);
        }
    }
}

void __cdecl Item_YesNo_Paint(int localClientNum, UiContext *dc, itemDef_s *item)
{
    const char *VariantString; // eax
    float v4; // [esp+0h] [ebp-Ch]
    textDef_s *textDefPtr; // [esp+4h] [ebp-8h]

    textDefPtr = Item_GetTextDef(item);
    if ( item->dvar )
    {
        VariantString = Dvar_GetVariantString(item->dvar);
        v4 = atof(VariantString);
    }
    else
    {
        v4 = 0.0f;
    }
    if ( v4 == 0.0 )
        textDefPtr->text = UI_SafeTranslateString("EXE_NO");
    else
        textDefPtr->text = UI_SafeTranslateString("EXE_YES");
    Item_Text_Paint(localClientNum, dc, item);
}

void __cdecl Item_Multi_Paint(int localClientNum, UiContext *dc, itemDef_s *item)
{
    textDef_s *textDefPtr; // [esp+0h] [ebp-4h]

    textDefPtr = Item_GetTextDef(item);
    textDefPtr->text = Item_Multi_Setting(item);
    Item_Text_Paint(localClientNum, dc, item);
}

const char *__cdecl Item_Multi_Setting(itemDef_s *item)
{
    const char *VariantString; // eax
    multiDef_s *multiPtr; // [esp+0h] [ebp-10h]
    const char *string; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    int ia; // [esp+8h] [ebp-8h]
    float value; // [esp+Ch] [ebp-4h]

    multiPtr = Item_GetMultiDef(item);
    if ( !multiPtr )
        return "<dvarStrList or dvarFloatList not set>";
    if ( multiPtr->strDef )
    {
        string = Dvar_GetVariantString(item->dvar);
        for ( i = 0; i < multiPtr->count; ++i )
        {
            if ( !I_stricmp(string, multiPtr->dvarStr[i]) )
                return multiPtr->dvarList[i];
        }
    }
    else
    {
        VariantString = Dvar_GetVariantString(item->dvar);
        value = atof(VariantString);
        for ( ia = 0; ia < multiPtr->count; ++ia )
        {
            if ( multiPtr->dvarValue[ia] == value )
                return multiPtr->dvarList[ia];
        }
    }
    return "";
}

void __cdecl Item_DvarEnum_Paint(int localClientNum, UiContext *dc, itemDef_s *item)
{
    textDef_s *textDefPtr; // [esp+0h] [ebp-4h]

    textDefPtr = Item_GetTextDef(item);
    textDefPtr->text = Item_DvarEnum_Setting(item);
    Item_Text_Paint(localClientNum, dc, item);
}

const char *__cdecl Item_DvarEnum_Setting(itemDef_s *item)
{
    const char *v2; // eax
    enumDvarDef_s *enumDvarPtr; // [esp+0h] [ebp-Ch]
    int enumIndex; // [esp+4h] [ebp-8h]
    const dvar_s *enumDvar; // [esp+8h] [ebp-4h]

    enumDvarPtr = Item_GetEnumDvarName(item);
    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 6333, 0, "%s", "item") )
        __debugbreak();
    if ( item->type != 11
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    6334,
                    0,
                    "%s\n\t(item->type) = %i",
                    "(item->type == 11)",
                    item->type) )
    {
        __debugbreak();
    }
    if ( !enumDvarPtr->enumDvarName )
        return "<dvarEnumList not set>";
    enumDvar = Dvar_FindVar(enumDvarPtr->enumDvarName);
    if ( enumDvar->type != DVAR_TYPE_ENUM )
        return "<not an enum dvar>";
    if ( !enumDvar->domain.enumeration.stringCount )
        return "";
    enumIndex = Item_DvarEnum_EnumIndex(item);
    if ( enumIndex < 0 || enumIndex >= enumDvar->domain.enumeration.stringCount )
    {
        v2 = va("%s %i:%i", enumDvarPtr->enumDvarName, enumDvar->domain.enumeration.stringCount, enumIndex);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                        6346,
                        0,
                        "%s\n\t%s",
                        "enumIndex >= 0 && enumIndex < enumDvar->domain.enumeration.stringCount",
                        v2) )
            __debugbreak();
    }
    return *(const char **)(enumDvar->domain.integer.max + 4 * enumIndex);
}

void __cdecl Item_Slider_Paint(UiContext *dc, itemDef_s *item)
{
    const char *VariantString; // eax
    float v3; // xmm0_4
    float v4; // [esp+24h] [ebp-74h]
    int blockNumber; // [esp+2Ch] [ebp-6Ch]
    float lowLight[4]; // [esp+30h] [ebp-68h] BYREF
    float thumbPositionX; // [esp+40h] [ebp-58h]
    const ScreenPlacement *scrPlace; // [esp+44h] [ebp-54h]
    float sliderBlockPosition; // [esp+48h] [ebp-50h]
    menuDef_t *parent; // [esp+4Ch] [ebp-4Ch]
    float BLOCK_EPSILON; // [esp+50h] [ebp-48h]
    textDef_s *textDefPtr; // [esp+54h] [ebp-44h]
    float sliderColorEmpty[4]; // [esp+58h] [ebp-40h] BYREF
    float sliderColorFilled[4]; // [esp+68h] [ebp-30h] BYREF
    float x; // [esp+78h] [ebp-20h]
    float y; // [esp+7Ch] [ebp-1Ch]
    const rectDef_s *rect; // [esp+80h] [ebp-18h]
    float value; // [esp+84h] [ebp-14h]
    float newColor[4]; // [esp+88h] [ebp-10h] BYREF

    sliderColorFilled[0] = 0.95999998;
    sliderColorFilled[1] = 0.57999998;
    sliderColorFilled[2] = 0.11;
    sliderColorFilled[3] = 1.0;
    sliderColorEmpty[0] = 0.95999998;
    sliderColorEmpty[1] = 0.57999998;
    sliderColorEmpty[2] = 0.11;
    sliderColorEmpty[3] = 0.2;
    parent = item->parent;
    textDefPtr = Item_GetTextDef(item);
    BLOCK_EPSILON = 2.0f;
    if (item->dvar)
    {
        VariantString = Dvar_GetVariantString(item->dvar);
        v4 = atof(VariantString);
    }
    else
    {
        v4 = 0.0f;
    }
    value = v4;
    if (Window_HasFocus(dc->contextIndex, &item->window))
    {
        lowLight[0] = 0.80000001 * parent->focusColor[0];
        lowLight[1] = 0.80000001 * parent->focusColor[1];
        lowLight[2] = 0.80000001 * parent->focusColor[2];
        lowLight[3] = 0.80000001 * parent->focusColor[3];
        //v3 = __libm_sse2_sin((float)(dc->realTime / 75));
        v3 = sin((float)(dc->realTime / 75));
        LerpColor(parent->focusColor, lowLight, newColor, (float)(v3 * 0.5) + 0.5);
    }
    else
    {
        newColor[0] = item->window.foreColor[0];
        newColor[1] = item->window.foreColor[1];
        newColor[2] = item->window.foreColor[2];
        newColor[3] = item->window.foreColor[3];
    }
    rect = Window_GetRect(&item->window);
    x = Item_GetRectPlacementX(textDefPtr->textAlignMode & 3, rect->x + textDefPtr->textalignx, rect->w, 100.0);
    y = Item_GetRectPlacementY(textDefPtr->textAlignMode & 0xC, rect->y + textDefPtr->textaligny, rect->h, 9.0);
    scrPlace = &scrPlaceView[dc->contextIndex];
    thumbPositionX = Item_Slider_ThumbPosition(item);
    for (blockNumber = 0; blockNumber < 10; ++blockNumber)
    {
        sliderBlockPosition = (float)(10 * blockNumber) + x;
        if ((float)(thumbPositionX + BLOCK_EPSILON) <= (float)(sliderBlockPosition + 10.0))
        {
            if (sliderBlockPosition <= (float)(thumbPositionX - BLOCK_EPSILON))
            {
                UI_DrawHandlePic(
                    scrPlace,
                    sliderBlockPosition,
                    y,
                    4.0,
                    9.0,
                    rect->horzAlign,
                    rect->vertAlign,
                    sliderColorFilled,
                    sharedUiInfo.assets.whiteMaterial);
                UI_DrawHandlePic(
                    scrPlace,
                    sliderBlockPosition + 4.0,
                    y,
                    4.0,
                    9.0,
                    rect->horzAlign,
                    rect->vertAlign,
                    sliderColorEmpty,
                    sharedUiInfo.assets.whiteMaterial);
            }
            else
            {
                UI_DrawHandlePic(
                    scrPlace,
                    sliderBlockPosition,
                    y,
                    9.0,
                    9.0,
                    rect->horzAlign,
                    rect->vertAlign,
                    sliderColorEmpty,
                    sharedUiInfo.assets.whiteMaterial);
            }
        }
        else
        {
            UI_DrawHandlePic(
                scrPlace,
                sliderBlockPosition,
                y,
                9.0,
                9.0,
                rect->horzAlign,
                rect->vertAlign,
                sliderColorFilled,
                sharedUiInfo.assets.whiteMaterial);
        }
    }
    UI_DrawHandlePicRotated(
        scrPlace,
        (float)(thumbPositionX - 3.0) - 1.0,
        (float)(y + 9.0) - 1.0,
        7.0,
        7.0,
        rect->horzAlign,
        rect->vertAlign,
        sliderColorFilled,
        -90.0,
        sharedUiInfo.assets.sliderThumb);
}

double __cdecl Item_GetRectPlacementY(int alignY, float y0, float containerHeight, float selfHeight)
{
    if ( alignY == 12 )
        return containerHeight - selfHeight + y0;
    if ( alignY == 8 )
        return (containerHeight - selfHeight) * 0.5 + y0;
    if ( alignY != 4
        && alignY
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    253,
                    0,
                    "%s\n\t(alignY) = %i",
                    "(alignY == 4 || alignY == 0)",
                    alignY) )
    {
        __debugbreak();
    }
    return y0;
}

void __cdecl Item_Bind_Paint(int localClientNum, UiContext *dc, itemDef_s *item)
{
    int type; // [esp+0h] [ebp-114h]
    int bindNum; // [esp+4h] [ebp-110h]
    textDef_s *textDefPtr; // [esp+8h] [ebp-10Ch]
    char nameBind[260]; // [esp+Ch] [ebp-108h] BYREF

    textDefPtr = Item_GetTextDef(item);
    type = item->type;
    if ( type == 12 )
    {
        bindNum = 0;
    }
    else
    {
        if ( type != 16 )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                            9056,
                            0,
                            "Got an unexpected bind type") )
                __debugbreak();
            return;
        }
        bindNum = 1;
    }
    if ( g_waitingForKey && g_bindItem == item )
    {
        textDefPtr->text = "@MENU_BIND_KEY_PENDING";
    }
    else
    {
        UI_GetKeyBindingLocalizedString(localClientNum, item->dvar, nameBind, bindNum);
        textDefPtr->text = nameBind;
    }
    Item_Text_Paint(localClientNum, dc, item);
    textDefPtr->text = "";
}

void __cdecl Item_ListBox_Paint(int localClientNum, UiContext *dc, itemDef_s *item)
{
    int v3; // [esp+24h] [ebp-60h]
    float h; // [esp+28h] [ebp-5Ch]
    bool selected; // [esp+3Fh] [ebp-45h]
    int scrollbarSize; // [esp+40h] [ebp-44h]
    float screenH; // [esp+44h] [ebp-40h] BYREF
    float screenW; // [esp+48h] [ebp-3Ch] BYREF
    float screenX; // [esp+4Ch] [ebp-38h] BYREF
    float screenY; // [esp+50h] [ebp-34h] BYREF
    float animSize; // [esp+54h] [ebp-30h]
    Material *image; // [esp+58h] [ebp-2Ch]
    int j; // [esp+5Ch] [ebp-28h]
    const ScreenPlacement *scrPlace; // [esp+60h] [ebp-24h]
    float thumb; // [esp+64h] [ebp-20h]
    listBoxDef_s *listPtr; // [esp+68h] [ebp-1Ch]
    float size; // [esp+6Ch] [ebp-18h]
    int i; // [esp+70h] [ebp-14h]
    const rectDef_s *rect; // [esp+74h] [ebp-10h]
    float x; // [esp+78h] [ebp-Ch]
    float y; // [esp+7Ch] [ebp-8h]
    int count; // [esp+80h] [ebp-4h]

    listPtr = Item_GetListBoxDef(item);
    if ( listPtr )
    {
        rect = Window_GetRect(&item->window);
        scrPlace = &scrPlaceView[dc->contextIndex];
        count = (int)UI_FeederCount(localClientNum, dc->contextIndex, listPtr->special, listPtr);
        if ( !listPtr->noScrollBars && !Item_ListBox_RequiresScroll(localClientNum, dc->contextIndex, item) )
            item->window.rect.w = item->window.rect.w - 16.0;
        if ( Window_IsHorizontal(&item->window) )
        {
            x = rect->x + 1.0;
            y = (float)((float)(rect->y + rect->h) - 16.0) - 1.0;
            UI_DrawHandlePic(
                scrPlace,
                x,
                y,
                16.0,
                16.0,
                rect->horzAlign,
                rect->vertAlign,
                0,
                sharedUiInfo.assets.scrollBarArrowLeft);
            x = x + 15.0;
            size = rect->w - 32.0;
            UI_DrawHandlePic(
                scrPlace,
                x,
                y,
                size + 1.0,
                16.0,
                rect->horzAlign,
                rect->vertAlign,
                0,
                sharedUiInfo.assets.scrollBar);
            x = (float)(size - 1.0) + x;
            UI_DrawHandlePic(
                scrPlace,
                x,
                y,
                16.0,
                16.0,
                rect->horzAlign,
                rect->vertAlign,
                0,
                sharedUiInfo.assets.scrollBarArrowRight);
            thumb = Item_ListBox_ThumbDrawPosition(localClientNum, dc, item);
            if ( thumb > (float)((float)(x - 16.0) - 1.0) )
                thumb = (float)(x - 16.0) - 1.0;
            UI_DrawHandlePic(
                scrPlace,
                thumb,
                y,
                16.0,
                16.0,
                rect->horzAlign,
                rect->vertAlign,
                0,
                sharedUiInfo.assets.scrollBarThumb);
            listPtr->endPos[dc->contextIndex] = listPtr->startPos[dc->contextIndex];
            size = rect->w - 2.0;
            for ( j = 0; j < listPtr->numColumns; ++j )
            {
                if ( listPtr->columnInfo[j].elementStyle == 1 )
                {
                    x = rect->x + 1.0;
                    y = rect->y + 1.0;
                    for ( i = listPtr->startPos[dc->contextIndex]; i < count; ++i )
                    {
                        image = UI_FeederItemImage(listPtr->special, i);
                        if ( image )
                            UI_DrawHandlePic(
                                scrPlace,
                                x + 1.0,
                                y + 1.0,
                                listPtr->elementWidth - 2.0,
                                listPtr->elementHeight - 2.0,
                                rect->horzAlign,
                                rect->vertAlign,
                                0,
                                image);
                        if ( i == listPtr->cursorPos[dc->contextIndex] )
                            UI_DrawRect(
                                scrPlace,
                                x,
                                y,
                                listPtr->elementWidth - 1.0,
                                listPtr->elementHeight - 1.0,
                                rect->horzAlign,
                                rect->vertAlign,
                                item->window.borderSize,
                                item->window.borderColor);
                        size = size - listPtr->elementWidth;
                        if ( listPtr->elementWidth > size )
                        {
                            listPtr->drawPadding = (int)size;
                            break;
                        }
                        x = x + listPtr->elementWidth;
                        ++listPtr->endPos[dc->contextIndex];
                    }
                }
            }
        }
        else
        {
            if ( listPtr->special == 54.0 )
            {
                UI_ClearFeeder(dc->contextIndex, item, 0);
                UI_RaiseFeederEvent(dc->contextIndex, item, "Populate");
            }
            UI_OverrideCursorPos(localClientNum, dc->contextIndex, item);
            if ( Item_ListBox_RequiresScroll(localClientNum, dc->contextIndex, item) )
            {
                x = (float)(rect->x + rect->w) - 16.0;
                y = rect->y + 3.0;
                listPtr->endPos[dc->contextIndex] = listPtr->startPos[dc->contextIndex];
                size = Item_ListBox_ScrollHeight(localClientNum, dc->contextIndex, item);
                UI_DrawHandlePic(scrPlace, x, y, 16.0, size, rect->horzAlign, rect->vertAlign, 0, sharedUiInfo.assets.scrollBar);
                if ( g_debugMode )
                {
                    UI_DrawRect(scrPlace, x, y, 16.0, size, rect->horzAlign, rect->vertAlign, 1.0, colorWhite);
                    UI_DrawRect(scrPlace, x, y, 16.0, 16.0, rect->horzAlign, rect->vertAlign, 1.0, colorWhite);
                    UI_DrawRect(
                        scrPlace,
                        x,
                        (float)(y + size) - 16.0,
                        16.0,
                        16.0,
                        rect->horzAlign,
                        rect->vertAlign,
                        1.0,
                        colorWhite);
                }
                y = (float)(size - 1.0) + y;
                thumb = Item_ListBox_ThumbDrawPosition(localClientNum, dc, item);
                if ( thumb > (float)(y - 1.0) )
                    thumb = y - 1.0;
                UI_DrawHandlePic(
                    scrPlace,
                    x,
                    thumb,
                    16.0,
                    16.0,
                    rect->horzAlign,
                    rect->vertAlign,
                    0,
                    sharedUiInfo.assets.scrollBarThumb);
                if ( g_debugMode )
                    UI_DrawRect(scrPlace, x, thumb, 16.0, 16.0, rect->horzAlign, rect->vertAlign, 1.0, colorWhite);
            }
            y = rect->y + 1.0;
            size = rect->h - 2.0;
            if ( size >= 0.0 )
            {
                if ( !item->animInfo || !item->animInfo->animating )
                    goto LABEL_89;
                if ( (float)(item->animInfo->currentAnimState.rectClient.h - item->animInfo->nextAnimState.rectClient.h) < 0.0 )
                    h = item->animInfo->nextAnimState.rectClient.h;
                else
                    h = item->animInfo->currentAnimState.rectClient.h;
                animSize = h - 2.0;
                if ( (float)(h - 2.0) > size )
                    size = animSize;
                if ( size >= 0.0 )
                {
LABEL_89:
                    if ( item->animInfo && item->animInfo->animating )
                    {
                        screenX = rect->x;
                        screenY = rect->y;
                        screenW = rect->w;
                        screenH = rect->h;
                        ScrPlace_ApplyRect(scrPlace, &screenX, &screenY, &screenW, &screenH, rect->horzAlign, rect->vertAlign);
                        R_AddCmdSetScissorValues(
                            1,
                            (int)(float)(screenX + 0.5),
                            (int)(float)(screenY + 0.5),
                            (int)(float)(screenW + 0.5),
                            (int)(float)(screenH + 0.5));
                    }
                    listPtr->endPos[dc->contextIndex] = listPtr->startPos[dc->contextIndex];
                    for ( i = listPtr->startPos[dc->contextIndex]; i < count; ++i )
                    {
                        x = rect->x + 1.0;
                        Item_ListBox_PaintBackground(localClientNum, dc->contextIndex, item, x, y, i);
                        scrollbarSize = 16;
                        if ( !Item_ListBox_RequiresScroll(localClientNum, dc->contextIndex, item) )
                            scrollbarSize = 0;
                        if ( listPtr->elementBackgroundColor[3] != 0.0 )
                            UI_FillRect(
                                scrPlace,
                                x + 2.0,
                                y + 2.0,
                                (float)(rect->w - (float)scrollbarSize) - 4.0,
                                listPtr->elementHeight - 2.0,
                                rect->horzAlign,
                                rect->vertAlign,
                                listPtr->elementBackgroundColor);
                        if ( i == listPtr->cursorPos[dc->contextIndex]
                            && (!Item_IsModal(item) || (item->window.dynamicFlags[dc->contextIndex] & 2) != 0) )
                        {
                            Item_ListBox_PaintHighlight(localClientNum, dc->contextIndex, item, x, y);
                        }
                        for ( j = 0; ; ++j )
                        {
                            v3 = listPtr->numColumns > 1 ? listPtr->numColumns : 1;
                            if ( j >= v3 )
                                break;
                            if ( listPtr->columnInfo[j].elementStyle == 1 )
                            {
                                x = (float)(rect->x + listPtr->columnInfo[j].rect.x) + 1.0;
                                image = UI_FeederItemImage(listPtr->special, i);
                                if ( image )
                                {
                                    if ( listPtr->numColumns <= 0 )
                                        UI_DrawHandlePic(
                                            scrPlace,
                                            x + 1.0,
                                            y + 1.0,
                                            listPtr->elementWidth - 2.0,
                                            listPtr->elementHeight - 2.0,
                                            rect->horzAlign,
                                            rect->vertAlign,
                                            0,
                                            image);
                                    else
                                        UI_DrawHandlePic(
                                            scrPlace,
                                            x + 1.0,
                                            y + 1.0,
                                            listPtr->columnInfo[j].rect.w - 2.0,
                                            listPtr->elementHeight - 2.0,
                                            rect->horzAlign,
                                            rect->vertAlign,
                                            0,
                                            image);
                                }
                                if ( i == listPtr->cursorPos[dc->contextIndex] )
                                    UI_DrawRect(
                                        scrPlace,
                                        x,
                                        y,
                                        listPtr->elementWidth - 1.0,
                                        listPtr->elementHeight - 1.0,
                                        rect->horzAlign,
                                        rect->vertAlign,
                                        item->window.borderSize,
                                        item->window.borderColor);
                                ++listPtr->endPos[dc->contextIndex];
                            }
                            else
                            {
                                x = rect->x + 1.0;
                                selected = 0;
                                if ( i == listPtr->cursorPos[dc->contextIndex]
                                    && (!Item_IsModal(item) || (item->window.dynamicFlags[dc->contextIndex] & 2) != 0) )
                                {
                                    selected = 1;
                                }
                                if ( listPtr->numColumns <= 0 )
                                    Item_ListBox_PaintTextElem(
                                        localClientNum,
                                        dc->contextIndex,
                                        item,
                                        (itemGroup_t)i,
                                        0,
                                        0x7FFFFFFF,
                                        0,
                                        12,
                                        x,
                                        y,
                                        listPtr->elementWidth,
                                        listPtr->elementHeight,
                                        selected);
                                else
                                    Item_ListBox_PaintTextElem(
                                        localClientNum,
                                        dc->contextIndex,
                                        item,
                                        (itemGroup_t)i,
                                        j,
                                        listPtr->columnInfo[j].maxChars,
                                        listPtr->columnInfo[j].rect.horzAlign,
                                        listPtr->columnInfo[j].rect.vertAlign,
                                        x + listPtr->columnInfo[j].rect.x,
                                        y + listPtr->columnInfo[j].rect.y,
                                        listPtr->columnInfo[j].rect.w,
                                        listPtr->columnInfo[j].rect.h,
                                        selected);
                            }
                        }
                        ++listPtr->endPos[dc->contextIndex];
                        size = size - listPtr->elementHeight;
                        if ( size >= 0.0 && listPtr->elementHeight > size && (i + 1 != count || listPtr->startPos[dc->contextIndex]) )
                        {
                            if ( !item->animInfo || !item->animInfo->animating )
                            {
                                //BLOPS_NULLSUB();
                                listPtr->drawPadding = (int)(float)(listPtr->elementHeight - size);
                            }
                            break;
                        }
                        y = y + listPtr->elementHeight;
                    }
                    if ( item->animInfo && item->animInfo->animating )
                        R_AddCmdSetScissorValues(0, 0, 0, 0, 0);
                }
            }
        }
    }
}

double __cdecl Item_ListBox_ThumbDrawPosition(int localClientNum, UiContext *dc, itemDef_s *item)
{
    return Item_ListBox_ThumbPosition(localClientNum, dc->contextIndex, item);
}

void __cdecl Item_ListBox_PaintTextElem(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                itemGroup_t row,
                int col,
                int maxChars,
                int halignment,
                int valignment,
                float x,
                float y,
                float w,
                float h,
                bool onFocus)
{
    int ControllerIndex; // eax
    int v14; // eax
    float special; // [esp+10h] [ebp-70h]
    rectDef_s ownerDrawRect; // [esp+28h] [ebp-58h] BYREF
    float imageAngle; // [esp+40h] [ebp-40h] BYREF
    float languageMultiplier; // [esp+44h] [ebp-3Ch]
    Font_s *font; // [esp+48h] [ebp-38h]
    language_t language; // [esp+4Ch] [ebp-34h]
    const ScreenPlacement *scrPlace; // [esp+50h] [ebp-30h]
    bool optionalOwnerDraw; // [esp+57h] [ebp-29h] BYREF
    listBoxDef_s *listPtr; // [esp+58h] [ebp-28h]
    Material *optionalImage; // [esp+5Ch] [ebp-24h] BYREF
    textDef_s *textDefPtr; // [esp+60h] [ebp-20h]
    float xadj; // [esp+64h] [ebp-1Ch]
    const rectDef_s *rect; // [esp+68h] [ebp-18h]
    DvarValue color; // [esp+6Ch] [ebp-14h] BYREF
    const char *text; // [esp+7Ch] [ebp-4h]

    textDefPtr = Item_GetTextDef(item);
    listPtr = Item_GetListBoxDef(item);
    language = (language_t)SEH_GetCurrentLanguage();
    languageMultiplier = 1.0f;
    rect = Window_GetRect(&item->window);
    scrPlace = &scrPlaceView[contextIndex];
    imageAngle = 0.0f;
    optionalOwnerDraw = 0;
    color.integer64 = LODWORD(colorWhite[0]) | 0x3F80000000000000LL;
    color.vector[2] = 1.0;
    text = (const char *)UI_FeederItemText(
                                                 localClientNum,
                                                 contextIndex,
                                                 item,
                                                 listPtr->special,
                                                 row,
                                                 col,
                                                 &optionalImage,
                                                 &imageAngle,
                                                 &optionalOwnerDraw);
    special = listPtr->special;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    UI_FeederItemColor(ControllerIndex, contextIndex, item, special, row, col, &color, onFocus);
    if ( (Window_GetDynamicFlags(contextIndex, &item->parent->window) & 0x82000) != 0 )
        color.vector[3] = color.vector[3] * item->parent->window.foreColor[3];
    if ( optionalImage && w > 0.0 && h > 0.0 )
    {
        UI_DrawHandlePicRotated(
            scrPlace,
            x,
            y + 2.0,
            w - 1.0,
            h - 2.0,
            rect->horzAlign,
            rect->vertAlign,
            &color.value,
            imageAngle,
            optionalImage);
    }
    else if ( text && *text )
    {
        font = Item_GetFont(contextIndex, item);
        v14 = UI_TextWidth(text, maxChars, font, textDefPtr->textscale);
        xadj = Item_GetTextAlignAdj(halignment, w, (float)v14);
        if ( (Window_HasFocus(contextIndex, &item->window) || !Item_IsModal(item))
            && onFocus
            && listPtr->elementHighlightColor[3] != 0.0 )
        {
            color.integer64 = *(_QWORD *)listPtr->elementHighlightColor;
            color.vector[2] = listPtr->elementHighlightColor[2];
        }
        languageMultiplier = Item_ListBox_LanguageScale(language, (int)listPtr->special, col);
        UI_DrawText(
            scrPlace,
            (char *)text,
            maxChars,
            font,
            (float)(x + xadj) + textDefPtr->textalignx,
            (float)(y + h) + textDefPtr->textaligny,
            rect->horzAlign,
            rect->vertAlign,
            textDefPtr->textscale * languageMultiplier,
            &color.value,
            textDefPtr->textStyle);
    }
    else if ( optionalOwnerDraw )
    {
        color = *(DvarValue *)item->window.foreColor;
        ownerDrawRect.x = x;
        ownerDrawRect.y = y;
        ownerDrawRect.w = w;
        ownerDrawRect.h = h;
        ownerDrawRect.horzAlign = rect->horzAlign;
        ownerDrawRect.vertAlign = rect->vertAlign;
        UI_FeederItemOwnerDraw(localClientNum, contextIndex, item, listPtr->special, &ownerDrawRect, row, col, &color.value);
    }
}

double __cdecl Item_GetTextAlignAdj(int alignment, float width, float textWidth)
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
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                            9183,
                            0,
                            "%s\n\t(alignment) = %i",
                            "(alignment == 0)",
                            alignment) )
                __debugbreak();
        }
        return 0.0;
    }
}

double __cdecl Item_ListBox_LanguageScale(language_t language, int feederID, int col)
{
    float languageMultiplier; // [esp+4h] [ebp-4h]

    languageMultiplier = 1.0f;
    if ( language == LANGUAGE_RUSSIAN || language == LANGUAGE_POLISH || language == LANGUAGE_JAPANESE )
    {
        switch ( feederID )
        {
            case 18:
            case 24:
            case 30:
            case 31:
                if ( col == 7 )
                    languageMultiplier = 0.85f;
                goto $LN4_131;
            case 32:
            case 50:
            case 101:
                if ( col == 5 )
                    languageMultiplier = 0.85f;
                break;
            case 67:
                if ( !col )
                    languageMultiplier = 0.82f;
                break;
            case 74:
                if ( !col )
                    languageMultiplier = 0.85f;
                break;
            case 83:
$LN4_131:
                if ( col == 4 )
                    languageMultiplier = 0.85f;
                break;
            default:
                return languageMultiplier;
        }
    }
    return languageMultiplier;
}

void __cdecl Item_ListBox_PaintBackground(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                float x,
                float y,
                int row)
{
    ScreenPlacement *scrPlace; // [esp+20h] [ebp-20h]
    listBoxDef_s *listPtr; // [esp+24h] [ebp-1Ch]
    Material *optionalBgImage; // [esp+28h] [ebp-18h]
    const rectDef_s *rect; // [esp+2Ch] [ebp-14h]
    float color[4]; // [esp+30h] [ebp-10h] BYREF

    if ( !item->window.style )
    {
        listPtr = Item_GetListBoxDef(item);
        if ( !listPtr && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 9323, 0, "%s", "listPtr") )
            __debugbreak();
        rect = Window_GetRect(&item->window);
        scrPlace = &scrPlaceView[contextIndex];
        UI_FeederBackColor(localClientNum, contextIndex, item, listPtr->special, row, color);
        optionalBgImage = listPtr->backgroundItemListbox;
        if ( optionalBgImage )
            UI_DrawHandlePic(
                scrPlace,
                x + 2.0,
                (float)(listPtr->elementHeight / 7.0) + y,
                (float)(rect->w - 16.0) - 4.0,
                listPtr->elementHeight - 2.0,
                rect->horzAlign,
                rect->vertAlign,
                color,
                optionalBgImage);
        else
            UI_FillRect(
                scrPlace,
                x + 2.0,
                y + 2.0,
                (float)(rect->w - 16.0) - 4.0,
                listPtr->elementHeight,
                rect->horzAlign,
                rect->vertAlign,
                color);
    }
}

// local variable allocation has failed, the output may be wrong!
void __cdecl Item_ListBox_PaintHighlight(int localClientNum, int contextIndex, itemDef_s *item, float x, float y)
{
    float v5; // xmm0_4
    float color[4]; // [esp+24h] [ebp-4Ch] BYREF
    float lowLight[4]; // [esp+34h] [ebp-3Ch] BYREF
    const ScreenPlacement *scrPlace; // [esp+44h] [ebp-2Ch]
    uiInfo_s *uiInfo; // [esp+48h] [ebp-28h]
    listBoxDef_s *listPtr; // [esp+4Ch] [ebp-24h]
    Material *optionalImage; // [esp+50h] [ebp-20h]
    Material *optionalHighlightImage; // [esp+54h] [ebp-1Ch]
    int scrollbarSize; // [esp+58h] [ebp-18h]
    const rectDef_s *rect; // [esp+5Ch] [ebp-14h]
    float newColor[4]; // [esp+60h] [ebp-10h] BYREF

    scrollbarSize = 16;
    listPtr = Item_GetListBoxDef(item);
    if (!listPtr && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 9348, 0, "%s", "listPtr"))
        __debugbreak();
    rect = Window_GetRect(&item->window);
    scrPlace = &scrPlaceView[contextIndex];
    uiInfo = UI_UIContext_GetInfo(contextIndex);
    if (listPtr->noBlinkingHighlight || (Window_GetDynamicFlags(contextIndex, &item->parent->window) & 0x82000) != 0)
    {
        newColor[0] = item->window.outlineColor[0];
        newColor[1] = item->window.outlineColor[1];
        newColor[2] = item->window.outlineColor[2];
        newColor[3] = item->window.outlineColor[3];
    }
    else
    {
        lowLight[0] = 0.80000001 * item->window.outlineColor[0];
        lowLight[1] = 0.80000001 * item->window.outlineColor[1];
        lowLight[2] = 0.80000001 * item->window.outlineColor[2];
        lowLight[3] = 0.80000001 * item->window.outlineColor[3];
        //v5 = __libm_sse2_sin((float)(uiInfo->uiDC.realTime / 150));
        v5 = sin((float)(uiInfo->uiDC.realTime / 150));
        LerpColor(item->window.outlineColor, lowLight, newColor, (float)(v5 * 0.5) + 0.5);
    }
    if (!Item_ListBox_RequiresScroll(localClientNum, contextIndex, item))
        scrollbarSize = 0;
    optionalHighlightImage = listPtr->highlightTexture;
    if (optionalHighlightImage)
    {
        UI_DrawHandlePic(
            scrPlace,
            x + 2.0,
            (float)(listPtr->elementHeight / 7.0) + y,
            (float)(rect->w - (float)scrollbarSize) - 4.0,
            listPtr->elementHeight - 2.0,
            rect->horzAlign,
            rect->vertAlign,
            newColor,
            optionalHighlightImage);
    }
    else
    {
        UI_FillRect(
            scrPlace,
            x + 2.0,
            y + 2.0,
            (float)(rect->w - (float)scrollbarSize) - 4.0,
            listPtr->elementHeight - 2.0,
            rect->horzAlign,
            rect->vertAlign,
            newColor);
        UI_DrawRect(
            scrPlace,
            x + 1.0,
            y + 1.0,
            (float)(rect->w - (float)scrollbarSize) - 3.0,
            listPtr->elementHeight + 1.0,
            rect->horzAlign,
            rect->vertAlign,
            1.0,
            listPtr->selectBorder);
    }
    optionalImage = listPtr->selectIcon;
    if (optionalImage)
    {
        if (!listPtr->notselectable)
        {
            color[0] = 1.0f;
            color[1] = 1.0f;
            color[2] = 1.0f;
            color[3] = item->window.foreColor[3];
            UI_DrawHandlePic(
                scrPlace,
                x + 3.0,
                (float)(listPtr->elementHeight / 5.0) + y,
                listPtr->elementHeight - 4.0,
                listPtr->elementHeight - 4.0,
                rect->horzAlign,
                rect->vertAlign,
                color,
                optionalImage);
        }
    }
}

void __cdecl Item_OwnerDraw_Paint(int localClientNum, UiContext *dc, itemDef_s *item)
{
    float v4; // xmm0_4
    float v5; // xmm0_4
    const char *VariantString; // eax
    rectDef_s v7; // [esp+40h] [ebp-470h]
    float lowLight[4]; // [esp+6Ch] [ebp-444h] BYREF
    Font_s *font; // [esp+7Ch] [ebp-434h]
    const char *textPtr; // [esp+80h] [ebp-430h]
    menuDef_t *parent; // [esp+84h] [ebp-42Ch]
    textDef_s *textDefPtr; // [esp+88h] [ebp-428h]
    float color[4]; // [esp+8Ch] [ebp-424h] BYREF
    int flags; // [esp+9Ch] [ebp-414h] BYREF
    const rectDef_s *rect; // [esp+A0h] [ebp-410h]
    const rectDef_s *textRect; // [esp+A4h] [ebp-40Ch]
    char text[1028]; // [esp+A8h] [ebp-408h] BYREF

    textDefPtr = Item_GetTextDef(item);
    if (!item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 9726, 0, "%s", "item"))
        __debugbreak();
    parent = item->parent;
    flags = Window_GetDynamicFlags(dc->contextIndex, &item->window);
    Fade(
        &flags,
        &item->window.foreColor[3],
        parent->fadeClamp,
        &item->window.nextTime,
        parent->fadeCycle,
        1,
        parent->fadeAmount,
        parent->fadeInAmount,
        dc);
    Window_SetDynamicFlags(dc->contextIndex, &item->window, flags);
    color[0] = item->window.foreColor[0];
    color[1] = item->window.foreColor[1];
    color[2] = item->window.foreColor[2];
    color[3] = item->window.foreColor[3];
    if (Window_HasFocus(dc->contextIndex, &item->window))
    {
        lowLight[0] = 0.80000001 * parent->focusColor[0];
        lowLight[1] = 0.80000001 * parent->focusColor[1];
        lowLight[2] = 0.80000001 * parent->focusColor[2];
        lowLight[3] = 0.80000001 * parent->focusColor[3];
        //v4 = __libm_sse2_sin((float)(dc->realTime / 75));
        v4 = sin((float)(dc->realTime / 75));
        LerpColor(parent->focusColor, lowLight, color, (float)(v4 * 0.5) + 0.5);
    }
    else if (textDefPtr && textDefPtr->textStyle == 1 && ((dc->realTime / 256) & 1) == 0)
    {
        lowLight[0] = 0.80000001 * item->window.foreColor[0];
        lowLight[1] = 0.80000001 * item->window.foreColor[1];
        lowLight[2] = 0.80000001 * item->window.foreColor[2];
        lowLight[3] = 0.80000001 * item->window.foreColor[3];
        //v5 = __libm_sse2_sin((float)(dc->realTime / 75));
        v5 = sin((float)(dc->realTime / 75));
        LerpColor(item->window.foreColor, lowLight, color, (float)(v5 * 0.5) + 0.5);
    }
    if ((item->dvarFlags & 3) != 0 && !Item_EnableShowViaDvar(item, 1))
    {
        color[0] = parent->disableColor[0];
        color[1] = parent->disableColor[1];
        color[2] = parent->disableColor[2];
        color[3] = parent->disableColor[3];
    }
    rect = Window_GetRect(&item->window);
    font = Item_GetFont(dc->contextIndex, item);
    textPtr = 0;
    if (textDefPtr && textDefPtr->text)
    {
        textPtr = textDefPtr->text;
    }
    else if (textDefPtr && textDefPtr->textExpData && textDefPtr->textExpData->textExp.filename)
    {
        textPtr = GetExpressionResultString(dc->contextIndex, item, &textDefPtr->textExpData->textExp);
    }
    else if (item->dvar)
    {
        VariantString = Dvar_GetVariantString(item->dvar);
        I_strncpyz(text, VariantString, 1024);
        textPtr = text;
    }
    if (textDefPtr && textDefPtr->text)
    {
        Item_Text_Paint(localClientNum, dc, item);
        textRect = (const rectDef_s *)Item_GetTextRect(dc->contextIndex, item);
        v7 = item->parent->window.rect;
        if (*textDefPtr->text)
            UI_OwnerDrawText(
                localClientNum,
                dc->contextIndex,
                item,
                (float)(textRect->x + textRect->w) + 8.0,
                rect->y,
                rect->w,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                textDefPtr->textaligny,
                item->window.ownerDraw,
                item->window.ownerDrawFlags,
                textDefPtr->alignment,
                font,
                textDefPtr->textscale,
                color,
                item->window.background,
                textDefPtr->textStyle,
                v7,
                textDefPtr->textAlignMode,
                (char *)textDefPtr->text);
        else
            UI_OwnerDrawText(
                localClientNum,
                dc->contextIndex,
                item,
                textRect->x + textRect->w,
                rect->y,
                rect->w,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                textDefPtr->textaligny,
                item->window.ownerDraw,
                item->window.ownerDrawFlags,
                textDefPtr->alignment,
                font,
                textDefPtr->textscale,
                color,
                item->window.background,
                textDefPtr->textStyle,
                v7,
                textDefPtr->textAlignMode,
                0);
    }
    else if (textPtr && *textPtr && textDefPtr && textDefPtr->textExpData && (item->type == 18 || item->type == 20))
    {
        textRect = (const rectDef_s *)Item_GetTextRect(dc->contextIndex, item);
        if (textPtr)
            Item_SetTextExtents(dc->contextIndex, item, textPtr);
        UI_OwnerDrawText(
            localClientNum,
            dc->contextIndex,
            item,
            textRect->x,
            textRect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            textDefPtr->textalignx,
            textDefPtr->textaligny,
            item->window.ownerDraw,
            item->window.ownerDrawFlags,
            textDefPtr->alignment,
            font,
            textDefPtr->textscale,
            color,
            item->window.background,
            textDefPtr->textStyle,
            item->parent->window.rect,
            textDefPtr->textAlignMode,
            (char *)textPtr);
    }
    else if (item->type == 18 || item->type == 20)
    {
        UI_OwnerDrawText(
            localClientNum,
            dc->contextIndex,
            item,
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            textDefPtr->textalignx,
            textDefPtr->textaligny,
            item->window.ownerDraw,
            item->window.ownerDrawFlags,
            textDefPtr->alignment,
            font,
            textDefPtr->textscale,
            color,
            item->window.background,
            textDefPtr->textStyle,
            item->parent->window.rect,
            textDefPtr->textAlignMode,
            (char *)textPtr);
    }
    else if (item->type == 6)
    {
        UI_OwnerDraw(
            localClientNum,
            item,
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            item->window.ownerDraw,
            item->window.ownerDrawFlags,
            color,
            item->window.background,
            item->parent->window.rect,
            item->dvar);
    }
}

void __cdecl Item_GameMsgWindow_Paint(int localClientNum, UiContext *dc, itemDef_s *item)
{
    const char *v3; // eax
    const char *v4; // eax
    Font_s *font; // [esp+18h] [ebp-20h]
    gameMsgDef_s *gameMsgPtr; // [esp+1Ch] [ebp-1Ch]
    textDef_s *textDefPtr; // [esp+20h] [ebp-18h]
    float color[4]; // [esp+24h] [ebp-14h] BYREF
    const rectDef_s *rect; // [esp+34h] [ebp-4h]

    textDefPtr = Item_GetTextDef(item);
    gameMsgPtr = Item_GetGameMsgDef(item);
    if ( !dc && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 9814, 0, "%s", "dc") )
        __debugbreak();
    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 9815, 0, "%s", "item") )
        __debugbreak();
    if ( CL_LocalClient_IsCUIFlagSet(localClientNum, 32) )
    {
        if ( gameMsgPtr->gameMsgWindowIndex > 2u )
        {
            v3 = va("Game message window index %d is not valid. Must be in [0, %d).", gameMsgPtr->gameMsgWindowIndex, 3);
            Com_Error(ERR_DROP, v3);
        }
        if ( gameMsgPtr->gameMsgWindowMode >= 2u && gameMsgPtr->gameMsgWindowMode != 3 && gameMsgPtr->gameMsgWindowMode != 2 )
        {
            v4 = va("Game message window mode %d is not valid.", gameMsgPtr->gameMsgWindowMode);
            Com_Error(ERR_DROP, v4);
        }
        color[0] = item->window.foreColor[0];
        color[1] = item->window.foreColor[1];
        color[2] = item->window.foreColor[2];
        color[3] = item->window.foreColor[3];
        rect = Window_GetRect(&item->window);
        font = Item_GetFont(dc->contextIndex, item);
        Con_DrawGameMessageWindow(
            localClientNum,
            gameMsgPtr->gameMsgWindowIndex,
            (int)rect->x,
            (int)rect->y,
            rect->horzAlign,
            rect->vertAlign,
            font,
            textDefPtr->textscale,
            color,
            textDefPtr->textStyle,
            textDefPtr->textAlignMode,
            (msgwnd_mode_t)gameMsgPtr->gameMsgWindowMode);
    }
}

int __cdecl Menu_Count(UiContext *dc)
{
    return dc->menuCount;
}

void __cdecl Menu_PaintAll_BeginVisibleList(char *stringBegin, unsigned int stringSize)
{
    char VISIBLE_LIST_PREFIX[14]; // [esp+0h] [ebp-14h] BYREF

    strcpy(VISIBLE_LIST_PREFIX, "ui_showlist: ");
    if ( stringSize < 0xE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    10651,
                    0,
                    "%s",
                    "stringSize >= sizeof( VISIBLE_LIST_PREFIX )") )
    {
        __debugbreak();
    }
    *(unsigned int *)stringBegin = *(unsigned int *)VISIBLE_LIST_PREFIX;
    *((unsigned int *)stringBegin + 1) = *(unsigned int *)&VISIBLE_LIST_PREFIX[4];
    *((unsigned int *)stringBegin + 2) = *(unsigned int *)&VISIBLE_LIST_PREFIX[8];
    *((_WORD *)stringBegin + 6) = *(_WORD *)&VISIBLE_LIST_PREFIX[12];
}

void __cdecl Menu_PaintAll_AppendToVisibleList(char *stringBegin, unsigned int stringSize, const char *stringToAppend)
{
#if 0
    unsigned int v3; // [esp+0h] [ebp-64h]
    std::reverse_iterator<char *> result; // [esp+44h] [ebp-20h] BYREF
    std::reverse_iterator<char *> _First; // [esp+48h] [ebp-1Ch]
    std::reverse_iterator<char *> _Last; // [esp+4Ch] [ebp-18h]
    char _Val; // [esp+53h] [ebp-11h] BYREF
    const char *lastNewline; // [esp+54h] [ebp-10h]
    int VISIBLE_LIST_LINE_LENGTH; // [esp+58h] [ebp-Ch]
    char *stringEnd; // [esp+5Ch] [ebp-8h]
    const char *terminus; // [esp+60h] [ebp-4h]

    VISIBLE_LIST_LINE_LENGTH = 80;
    v3 = strlen(stringBegin);
    stringEnd = &stringBegin[v3];
    I_strncat(stringBegin, stringSize, stringToAppend);
    _Val = 10;
    _Last.current = stringBegin;
    _First.current = &stringBegin[v3];
    lastNewline = std::find<std::reverse_iterator<char *>,char>(
                                    &result,
                                    (std::reverse_iterator<char *>)&stringBegin[v3],
                                    (std::reverse_iterator<char *>)stringBegin,
                                    &_Val)->current
                            - 1;
    if ( stringEnd - lastNewline <= 80 )
        terminus = ", ";
    else
        terminus = ",\n    ";
    I_strncat(stringBegin, stringSize, terminus);
#endif
    const int VISIBLE_LIST_LINE_LENGTH = 80;

    // Find current end before append
    size_t oldLen = strlen(stringBegin);
    char *stringEnd = stringBegin + oldLen;

    // Append the new item
    I_strncat(stringBegin, stringSize, stringToAppend);

    // Find last newline before the append
    const char *lastNewline = strrchr(stringBegin, '\n');

    if (!lastNewline)
    {
        // No newline in string — treat start as line beginning
        lastNewline = stringBegin - 1;
    }

    size_t currentLineLength = stringEnd - lastNewline;

    const char *terminus =
        (currentLineLength <= VISIBLE_LIST_LINE_LENGTH)
        ? ", "
        : ",\n    ";

    I_strncat(stringBegin, stringSize, terminus);
}

void __cdecl Menu_PaintAll_DrawVisibleList(char *stringBegin, UiContext *dc)
{
    float color[4]; // [esp+28h] [ebp-18h] BYREF
    float y; // [esp+38h] [ebp-8h]
    float MESSAGE_SCALE; // [esp+3Ch] [ebp-4h]

    MESSAGE_SCALE = 0.2f;
    color[0] = 0.75f;
    color[1] = 1.0f;
    color[2] = 0.5f;
    color[3] = 1.0f;
    if ( dc->FPS == 0.0 )
        y = 320.0f;
    else
        y = 400.0f;
    UI_DrawText(
        &scrPlaceFull,
        stringBegin,
        0x7FFFFFFF,
        sharedUiInfo.assets.smallFont,
        0.0,
        y,
        0,
        0,
        MESSAGE_SCALE,
        color,
        0);
}

void __cdecl Menu_PaintAll(int localClientNum, UiContext *dc)
{
    const char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    char *v11; // eax
    char *v12; // eax
    Font_s *FontHandle; // [esp+14h] [ebp-5ACh]
    int v14; // [esp+54h] [ebp-56Ch]
    int v15; // [esp+5Ch] [ebp-564h]
    int contextIndex; // [esp+80h] [ebp-540h]
    Font_s *v17; // [esp+84h] [ebp-53Ch]
    Font_s *font; // [esp+88h] [ebp-538h]
    int UI3DOverrideId; // [esp+90h] [ebp-530h]
    GfxUI3DStack *windowIdStack; // [esp+94h] [ebp-52Ch]
    menuDef_t *toastPopupMenu; // [esp+9Ch] [ebp-524h]
    bool showVisibleList; // [esp+A3h] [ebp-51Dh]
    int blurFocus; // [esp+A4h] [ebp-51Ch]
    int menuIndex; // [esp+A8h] [ebp-518h]
    int menuIndexa; // [esp+A8h] [ebp-518h]
    int menuIndexb; // [esp+A8h] [ebp-518h]
    int menuIndexc; // [esp+A8h] [ebp-518h]
    menuDef_t *menu; // [esp+ACh] [ebp-514h]
    menuDef_t *menua; // [esp+ACh] [ebp-514h]
    menuDef_t *menub; // [esp+ACh] [ebp-514h]
    menuDef_t *menuc; // [esp+ACh] [ebp-514h]
    menuDef_t *blurMenu; // [esp+B0h] [ebp-510h]
    char drew; // [esp+B7h] [ebp-509h]
    char drewa; // [esp+B7h] [ebp-509h]
    ScreenPlacementStack scrPlaceStack; // [esp+B8h] [ebp-508h] BYREF
    ScreenPlacementStack *scrPlaceStackPtr; // [esp+1B4h] [ebp-40Ch]
    char visibleList[1024]; // [esp+1B8h] [ebp-408h] BYREF
    int drawStart; // [esp+5BCh] [ebp-4h]
    int savedregs; // [esp+5C0h] [ebp+0h] BYREF

    contextIndex = dc->contextIndex;
    scrPlaceStack.size = 0;
    scrPlaceStack.contextIndex = contextIndex;
    scrPlaceStackPtr = &scrPlaceStack;
    showVisibleList = ui_showList->current.enabled;

    PROF_SCOPED("menu paint all");
    
    {
        PROF_SCOPED("Menu_PaintAll_BeginVisibleList");
        if (showVisibleList)
            Menu_PaintAll_BeginVisibleList(visibleList, 0x400u);
    }
    
    dc->blurRadiusOut = 0.0f;
    if ( captureFunc )
        captureFunc(localClientNum, dc, captureData);
    toastPopupMenu = Menus_FindByName(dc, "menu_toast_popup");

    {
        PROF_SCOPED("menu paint");

        for (menuIndex = 0; menuIndex < Menu_Count(dc); ++menuIndex)
        {
            menu = dc->Menus[menuIndex];
            if (!menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 10755, 0, "%s", "menu"))
                __debugbreak();
            if (!Menus_MenuIsInStack(dc, menu) && menu != toastPopupMenu)
            {
                {
                    v2 = va("Menu_Paint %s", menu->window.name);
                    PROF_SCOPED_RUNTIME_NAME(v2);
                    drew = Menu_Paint(
                        localClientNum,
                        dc,
                        scrPlaceStackPtr,
                        menu,
                        (const ScreenPlacement *)0xFFFFFFFF);
                }

                if (drew)
                {
                    if (showVisibleList)
                        Menu_PaintAll_AppendToVisibleList(visibleList, 0x400u, menu->window.name);
                }
            }
        }
    }

    drawStart = 0;
    for ( menuIndexa = dc->openMenuCount - 1; menuIndexa >= 0; --menuIndexa )
    {
        menua = dc->menuStack[menuIndexa].menu;
        if ( !menua && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 10773, 0, "%s", "menu") )
            __debugbreak();
        if ( menua->fullScreen )
        {
            drawStart = menuIndexa;
            break;
        }
    }

    {
        PROF_SCOPED("blurMenu");
        blurMenu = Menus_FindByName(dc, "UIBlur");
        if (blurMenu)
        {
            if ((Window_GetDynamicFlags(dc->contextIndex, &blurMenu->window) & 4) == 0)
                Window_SetDynamicFlags(dc->contextIndex, &blurMenu->window, 4);
            Menu_Paint(
                localClientNum,
                dc,
                scrPlaceStackPtr,
                blurMenu,
                (const ScreenPlacement *)0xFFFFFFFF);
        }
    }

    for ( menuIndexb = drawStart; menuIndexb < dc->openMenuCount; ++menuIndexb )
    {
        menub = dc->menuStack[menuIndexb].menu;
        if ( !menub && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 10796, 0, "%s", "menu") )
            __debugbreak();
        if ( toastPopupMenu != menub && (menub->slideTimeCounter == -3 || menub->fadeTimeCounter == -3) )
        {
            v15 = localClientNum;
            if ( Menu_DoesMenuOrParentsHaveControlFlag(dc, dc->menuStack[menuIndexb].menu, 3) )
                v15 = dc->menuStack[menuIndexb].localClientNum;
            Menus_Close(v15, dc, menub);
            menub->slideTimeCounter = -1;
            menub->fadeTimeCounter = -1;
        }
    }

    {
        PROF_SCOPED("UI3DOverride");

        blurFocus = GetBlurFocus(localClientNum, dc);
        if (blurFocus != -1)
        {
            windowIdStack = R_GetUI3DStack();
            R_UI3DStack_Push(windowIdStack, 0);
            UI_SetShaderTime(localClientNum);
            R_UI3DStack_Pop(windowIdStack);
        }
        for (menuIndexc = drawStart; menuIndexc < dc->openMenuCount; ++menuIndexc)
        {
            menuc = dc->menuStack[menuIndexc].menu;
            if (!menuc && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 10821, 0, "%s", "menu"))
                __debugbreak();
            UI3DOverrideId = -1;
            if (menuIndexc < blurFocus)
                UI3DOverrideId = 0;
            v14 = localClientNum;
            if (Menu_DoesMenuOrParentsHaveControlFlag(dc, dc->menuStack[menuIndexc].menu, 3))
                v14 = dc->menuStack[menuIndexc].localClientNum;
            drewa = Menu_Paint(
                v14,
                dc,
                scrPlaceStackPtr,
                menuc,
                (const ScreenPlacement *)UI3DOverrideId);
            if (drewa && showVisibleList)
                Menu_PaintAll_AppendToVisibleList(visibleList, 0x400u, menuc->window.name);
            if (drewa && g_debugMode && menuIndexc == dc->openMenuCount - 1)
            {
                font = UI_GetFontHandle(&scrPlaceView[dc->contextIndex], 0, 0.5);
                v3 = va("menu: %s", menuc->window.name);
                UI_DrawText(&scrPlaceView[dc->contextIndex], v3, 0x7FFFFFFF, font, 200.0, 45.0, 0, 0, 0.5, colorWhite, 0);
            }
        }
    }
    
    {
        PROF_SCOPED("g_debugMode");
        if (g_debugMode)
        {
            FontHandle = UI_GetFontHandle(&scrPlaceView[dc->contextIndex], 0, 0.5);
            v4 = va("fps: %f", dc->FPS);
            UI_DrawText(&scrPlaceView[dc->contextIndex], v4, 0x7FFFFFFF, FontHandle, 200.0, 25.0, 0, 0, 0.5, colorWhite, 0);
            v17 = UI_GetFontHandle(&scrPlaceView[dc->contextIndex], 3, 0.25);
            v5 = va("x: %.0f, y: %.0f", dc->cursor.x, dc->cursor.y);
            UI_DrawText(&scrPlaceView[dc->contextIndex], v5, 0x7FFFFFFF, v17, 5.0, 25.0, 0, 0, 0.5, colorRed, 0);
            v6 = va(
                "x: %.0f, y: %.0f",
                (float)((float)((float)dc->screenWidth * dc->cursor.x) / 640.0),
                (float)((float)((float)dc->screenHeight * dc->cursor.y) / 480.0));
            UI_DrawText(&scrPlaceView[dc->contextIndex], v6, 0x7FFFFFFF, v17, 5.0, 45.0, 0, 0, 0.5, colorRed, 0);
            v7 = va("x: %.0f, y: %.0f", (float)(dc->cursor.x - 640.0), dc->cursor.y);
            UI_DrawText(&scrPlaceView[dc->contextIndex], v7, 0x7FFFFFFF, v17, 500.0, 25.0, 0, 0, 0.5, colorRed, 0);
            v8 = va(
                "x: %.0f, y: %.0f",
                (float)((float)((float)(dc->cursor.x - 640.0) * (float)dc->screenWidth) / 640.0),
                (float)((float)((float)dc->screenHeight * dc->cursor.y) / 480.0));
            UI_DrawText(&scrPlaceView[dc->contextIndex], v8, 0x7FFFFFFF, v17, 500.0, 45.0, 0, 0, 0.5, colorRed, 0);
            v9 = va("x: %.0f, y: %.0f", (float)(dc->cursor.x - 640.0), (float)(dc->cursor.y - 480.0));
            UI_DrawText(&scrPlaceView[dc->contextIndex], v9, 0x7FFFFFFF, v17, 500.0, 445.0, 0, 0, 0.5, colorRed, 0);
            v10 = va(
                "x: %.0f, y: %.0f",
                (float)((float)((float)(dc->cursor.x - 640.0) * (float)dc->screenWidth) / 640.0),
                (float)((float)((float)(dc->cursor.y - 480.0) * (float)dc->screenHeight) / 480.0));
            UI_DrawText(&scrPlaceView[dc->contextIndex], v10, 0x7FFFFFFF, v17, 500.0, 465.0, 0, 0, 0.5, colorRed, 0);
            v11 = va("x: %.0f, y: %.0f", dc->cursor.x, (float)(dc->cursor.y - 480.0));
            UI_DrawText(&scrPlaceView[dc->contextIndex], v11, 0x7FFFFFFF, v17, 5.0, 445.0, 0, 0, 0.5, colorRed, 0);
            v12 = va(
                "x: %.0f, y: %.0f",
                (float)((float)((float)dc->screenWidth * dc->cursor.x) / 640.0),
                (float)((float)((float)(dc->cursor.y - 480.0) * (float)dc->screenHeight) / 480.0));
            UI_DrawText(&scrPlaceView[dc->contextIndex], v12, 0x7FFFFFFF, v17, 5.0, 465.0, 0, 0, 0.5, colorRed, 0);
        }
    }
    
    {
        PROF_SCOPED("Menu_PaintAll_DrawVisibleList");
        if (showVisibleList)
            Menu_PaintAll_DrawVisibleList(visibleList, dc);
    }
}

int __cdecl GetBlurFocus(int localClientNum, UiContext *dc)
{
    int i; // [esp+0h] [ebp-Ch]
    int blurMenuStackIndex; // [esp+4h] [ebp-8h]
    menuDef_t *blurMenu; // [esp+8h] [ebp-4h]

    blurMenu = Menus_FindByName(dc, "UIBlur");
    blurMenuStackIndex = dc->blurMenuStackIndex;
    if ( !blurMenu )
        return -1;
    if ( (blurMenu->itemCount != 1 || !*blurMenu->items)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    2280,
                    0,
                    "%s",
                    "blurMenu->itemCount == 1 && blurMenu->items[0]") )
    {
        __debugbreak();
    }
    if ( blurMenu->itemCount != 1 || !*blurMenu->items )
        return -1;
    if ( (*blurMenu->items)->window.foreColor[3] <= 0.0 )
        return -1;
    if ( blurMenuStackIndex < 0 )
        blurMenuStackIndex = 0;
    if ( blurMenuStackIndex >= 8
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    2290,
                    0,
                    "%s",
                    "( blurMenuStackIndex >= 0 ) && ( blurMenuStackIndex < MAX_BLUR_MENUS )") )
    {
        __debugbreak();
    }
    if ( blurMenuStackIndex >= 8 )
        return -1;
    for ( i = 0; i < dc->openMenuCount; ++i )
    {
        if ( dc->menuStack[i].menu->window.name
            && !I_strnicmp(dc->menuStack[i].menu->window.name, dc->blurMenuStack[blurMenuStackIndex].menuName, 64) )
        {
            return i;
        }
    }
    RemoveMenuFromBlurStack(localClientNum, dc, dc->blurMenuStack[blurMenuStackIndex].menuName);
    return 16;
}

void __cdecl UI_SetSystemCursorPos(UiContext *dc, float x, float y)
{
    float v3; // [esp+8h] [ebp-20h]
    float v4; // [esp+Ch] [ebp-1Ch]
    float v5; // [esp+10h] [ebp-18h]
    float v6; // [esp+14h] [ebp-14h]
    float v7; // [esp+18h] [ebp-10h]
    float v8; // [esp+1Ch] [ebp-Ch]

    v6 = x * scrPlaceFull.scaleVirtualToFull[0];
    v8 = ScrPlace_HiResGetScale() * v6;
    v5 = floor(v8 + 0.5);
    v4 = y * scrPlaceFull.scaleVirtualToFull[1];
    v7 = ScrPlace_HiResGetScale() * v4;
    v3 = floor(v7 + 0.5);
    CL_SetCursorPos((int)v5, (int)v3);
}

int __cdecl Display_MouseMove(int localClientNum, UiContext *dc)
{
    menuDef_t *menu; // [esp+8h] [ebp-14h]
    int i; // [esp+10h] [ebp-Ch]
    int x; // [esp+14h] [ebp-8h]
    int y; // [esp+18h] [ebp-4h]

    x = (int)dc->cursor.x;
    y = (int)dc->cursor.y;
    if ( mouseLocationX == 0x7FFFFFFF && mouseLocationY == 0x7FFFFFFF )
    {
        mouseLocationX = (int)dc->cursor.x;
        mouseLocationY = y;
    }
    if ( x == mouseLocationX && y == mouseLocationY )
        return 0;
    menu = Menu_GetFocused(dc);
    if (menu && (menu->window.staticFlags & 0x1000000) != 0)
    {
        Menu_HandleMouseMove(localClientNum, dc, menu);
        UI_ResetCachedMousePosition(localClientNum, dc);
        return 1;
    }
    else
    {
        for ( i = dc->openMenuCount - 1;
                    i >= 0
             && !Menu_HandleMouseMove(localClientNum, dc, dc->menuStack[i].menu)
             && !dc->menuStack[i].menu->fullScreen;
                    --i )
        {
            ;
        }
        UI_ResetCachedMousePosition(localClientNum, dc);
        return 1;
    }
}

int __cdecl UI_ResetCachedMousePosition(int localClientNum, UiContext *dc)
{
    if ( mouseTimeUntilReadyToMove && (int)Sys_Milliseconds() < mouseTimeUntilReadyToMove )
        return 0;
    mouseLocationX = (int)dc->cursor.x;
    mouseLocationY = (int)dc->cursor.y;
    mouseTimeUntilReadyToMove = 0;
    return 1;
}

int __cdecl Menu_OverActiveItem(int contextIndex, menuDef_t *menu, float x, float y)
{
    const rectDef_s *Rect; // eax
    const rectDef_s *v5; // eax
    const rectDef_s *v6; // eax
    itemDef_s *overItem; // [esp+Ch] [ebp-10h]
    int i; // [esp+10h] [ebp-Ch]
    textDef_s *textDefPtr; // [esp+14h] [ebp-8h]

    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 10966, 0, "%s", "menu") )
        __debugbreak();
    if ( menu )
    {
        if ( (Window_GetDynamicFlags(contextIndex, &menu->window) & 0x4004) != 0 )
        {
            Rect = Window_GetRect(&menu->window);
            if ( Rect_ContainsPoint(contextIndex, Rect, x, y) )
            {
                for ( i = 0; i < menu->itemCount; ++i )
                {
                    if ( (Window_GetDynamicFlags(contextIndex, &menu->items[i]->window) & 0x4004) != 0
                        && (menu->items[i]->window.staticFlags & 0x100000) == 0 )
                    {
                        v5 = Window_GetRect(&menu->items[i]->window);
                        if ( Rect_ContainsPoint(contextIndex, v5, x, y) )
                        {
                            overItem = menu->items[i];
                            textDefPtr = Item_GetTextDef(overItem);
                            if ( overItem->type != 1 || !textDefPtr->text )
                                return 1;
                            v6 = Item_CorrectedTextRect(contextIndex, overItem);
                            if ( Rect_ContainsPoint(contextIndex, v6, x, y) )
                                return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

void __cdecl UI_AddMenuList(int localClientNum, UiContext *dc, MenuList *menuList, int close)
{
    int i; // [esp+0h] [ebp-4h]

    if ( menuList )
    {
        for ( i = 0; i < menuList->menuCount; ++i )
            UI_AddMenu(localClientNum, dc, menuList->menus[i], close);
    }
}

void __cdecl UI_AddMenu(int localClientNum, UiContext *dc, menuDef_t *menu, int close)
{
    int i; // [esp+8h] [ebp-8h]

    for ( i = 0; i < dc->menuCount; ++i )
    {
        if ( dc->Menus[i] == menu )
            return;
    }
    if ( dc->menuCount >= 600 )
        Com_Error(ERR_DROP, "UI_AddMenu: EXE_ERR_OUT_OF_MEMORY");
    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 11051, 0, "%s", "menu") )
        __debugbreak();
    if ( dc->menuCount >= 0x258u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                    11052,
                    0,
                    "dc->menuCount doesn't index ARRAY_COUNT( dc->Menus )\n\t%i not in [0, %i)",
                    dc->menuCount,
                    600) )
    {
        __debugbreak();
    }
    if ( useFastFile->current.enabled
        && DB_FindXAssetHeader(ASSET_TYPE_MENU, (char*)menu->window.name, 1, -1).menu != menu
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp", 11058, 0, "%s", "touchMenu == menu") )
    {
        __debugbreak();
    }
    dc->Menus[dc->menuCount++] = menu;
    if ( close )
        Menus_Close(localClientNum, dc, menu);
}

void __cdecl UI_PlaySound(char context, char *aliasname)
{
    SndEntHandle entHandle; // [esp+1Ch] [ebp-4h]

    SND_FadeIn();
    entHandle.field = SND_EntHandle(context, 4095, 0, 0, 1, TEAM_FREE).field;
    SND_Play(aliasname, 0, 1.0, entHandle, 0, 0, 0);
}

void __cdecl UI_ClearLocalUIVisibilityBits(int localClientNum)
{
    LODWORD(sharedUiInfo.localVisibilityBits[localClientNum]) = 0;
    HIDWORD(sharedUiInfo.localVisibilityBits[localClientNum]) = 0;
}

void __cdecl UI_SetLocalUIVisbilityBit(int localClientNum, int bitShift, int value)
{
    int v3; // edx

    if (bitShift >= 0)
    {
        if (bitShift <= 64)
        {
            if (value)
            {
                v3 = HIDWORD(sharedUiInfo.localVisibilityBits[localClientNum]) | ((unsigned __int64)(1LL << bitShift) >> 32);
                LODWORD(sharedUiInfo.localVisibilityBits[localClientNum]) |= 1LL << bitShift;
            }
            else
            {
                v3 = HIDWORD(sharedUiInfo.localVisibilityBits[localClientNum]) & ~((unsigned __int64)(1LL << bitShift) >> 32);
                LODWORD(sharedUiInfo.localVisibilityBits[localClientNum]) &= ~(unsigned int)(1LL << bitShift);
            }
            HIDWORD(sharedUiInfo.localVisibilityBits[localClientNum]) = v3;
        }
        else
        {
            Com_PrintError(13, "Trying to set a UI visibility bit at position %d , but max allowed bits are %d", bitShift, 64);
        }
    }
    else
    {
        Com_PrintError(13, "Negative bitShift position given for setting UI visibility bit.");
    }
}

int __cdecl UI_GetMenuScreen()
{
    int menu; // [esp+0h] [ebp-4h]

    menu = UI_GetForcedMenuScreen();
    if ( menu >= 0 )
        return menu;
    else
        return 1;
}

int __cdecl UI_GetForcedMenuScreen()
{
    return -1;
}

int __cdecl UI_GetMenuScreenForError()
{
    int menu; // [esp+0h] [ebp-4h]

    menu = UI_GetForcedMenuScreen();
    if ( menu >= 0 )
        return menu;
    else
        return 0;
}

char __cdecl Menu_DoesMenuOrParentsHaveControlFlag(UiContext *dc, menuDef_t *menu, int controlFlag)
{
    int menuIndex; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]
    int ia; // [esp+4h] [ebp-4h]

    if ( menu )
    {
        menuIndex = -1;
        for ( i = dc->openMenuCount - 1; i >= 0; --i )
        {
            if ( menu == dc->menuStack[i].menu )
            {
                menuIndex = i;
                break;
            }
        }
        if ( menuIndex >= dc->openMenuCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared.cpp",
                        11250,
                        0,
                        "%s",
                        "menuIndex < dc->openMenuCount") )
        {
            __debugbreak();
        }
        if ( menuIndex >= 0 )
        {
            for ( ia = menuIndex; ia >= 0; --ia )
            {
                if ( dc->menuStack[ia].menu->control == controlFlag )
                    return 1;
            }
        }
    }
    return 0;
}

void __cdecl UI_SetLoadingScreenMaterial(const char * name)
{
    if (!IsDedicatedServer())
    {
        Material *mat; // [esp+0h] [ebp-4h]

        iassert(name);
        iassert(name[0]);

        mat = Material_RegisterHandle(va("loadscreen_%s", name), 36);
        if (Material_IsDefault(mat))
            sharedUiInfo.loadingScreen = 0;
        else
            sharedUiInfo.loadingScreen = mat;
    }
}

