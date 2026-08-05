#include "ui_utils.h"
#include <universal/assertive.h>
#include <client_mp/cl_scrn_mp.h>
#include "ui_feeders.h"
#include <universal/com_expressions_eval.h>
#include <universal/com_memory.h>
#include <qcommon/threads.h>
#include <universal/q_parse.h>
#include <universal/com_files.h>
#include <win32/win_shared.h>
#include <game_mp/g_main_mp.h>
#include <cgame_mp/cg_main_mp.h>

stringDef_s *g_strHandle[2048];

void __cdecl Window_SetDynamicFlags(int contextIndex, windowDef_t *w, int flags)
{
    if ( contextIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp",
                    53,
                    0,
                    "contextIndex doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    contextIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( flags
        && (flags & 0xFFFFF) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp",
                    54,
                    0,
                    "%s\n\t(flags) = %i",
                    "(flags == 0 || flags & 0x000FFFFF)",
                    flags) )
    {
        __debugbreak();
    }
    if ( (flags & 0xFFF00000) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp",
                    55,
                    0,
                    "%s",
                    "!(flags & WINDOWSTATIC_CHECKMASK)") )
    {
        __debugbreak();
    }
    w->dynamicFlags[contextIndex] = flags;
}

void __cdecl Window_AddDynamicFlags(int contextIndex, windowDef_t *w, int newFlags)
{
    int flags; // [esp+0h] [ebp-4h]

    flags = Window_GetDynamicFlags(contextIndex, w);
    Window_SetDynamicFlags(contextIndex, w, newFlags | flags);
}

void __cdecl Window_RemoveDynamicFlags(int contextIndex, windowDef_t *w, int newFlags)
{
    int modifiedFlags; // [esp+0h] [ebp-8h]
    int flags; // [esp+4h] [ebp-4h]

    flags = Window_GetDynamicFlags(contextIndex, w);
    modifiedFlags = newFlags;
    if ( (newFlags & 4) != 0 )
        modifiedFlags = newFlags | 2;
    Window_SetDynamicFlags(contextIndex, w, flags & ~modifiedFlags);
}

void __cdecl Window_SetStaticFlags(windowDef_t *w, int flags)
{
    if ( flags
        && (flags & 0xFFF00000) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp",
                    92,
                    0,
                    "%s\n\t(flags) = %i",
                    "(flags == 0 || flags & 0xFFF00000)",
                    flags) )
    {
        __debugbreak();
    }
    if ( (flags & 0xFFFFF) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp",
                    93,
                    0,
                    "%s",
                    "!(flags & WINDOWDYNAMIC_CHECKMASK)") )
    {
        __debugbreak();
    }
    w->staticFlags = flags;
}

void __cdecl Menu_SetCursorItem(int contextIndex, menuDef_t *menu, int cursorItem)
{
    if ( contextIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp",
                    100,
                    0,
                    "contextIndex doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    contextIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp", 101, 0, "%s", "menu") )
        __debugbreak();
    menu->cursorItem[contextIndex] = cursorItem;
}

int __cdecl Item_IsVisible(int localClientNum, int contextIndex, itemDef_s *item)
{
    if ( !Window_IsVisible(contextIndex, &item->window) )
        return 0;
    if ( (item->dvarFlags & 0xC) != 0 && !Item_EnableShowViaDvar(item, 4) )
        return 0;
    if ((sharedUiInfo.visibilityBits[localClientNum] & LODWORD(item->showBits)) != LODWORD(item->showBits)
        || __PAIR64__(
            LODWORD(sharedUiInfo.visibilityBits[localClientNum]) | LODWORD(item->hideBits),
            HIDWORD(sharedUiInfo.visibilityBits[localClientNum]) & HIDWORD(item->showBits)) != *(unsigned __int64 *)((char *)&item->showBits + 4)
        || (HIDWORD(sharedUiInfo.visibilityBits[localClientNum]) | HIDWORD(item->hideBits)) != HIDWORD(item->hideBits))
    {
        return 0;
    }
    if ( !item->visibleExp.filename || IsExpressionTrue(localClientNum, item, &item->visibleExp) )
        return 1;
    if ( uiscript_debug && uiscript_debug->current.integer )
    {
        if ( item->window.name )
            Com_Printf(13, "Item %s is hidden because its 'visible when' expression is false\n", item->window.name);
        else
            Com_Printf(13, "Item %s is hidden because its 'visible when' expression is false\n", "unnamed");
    }
    return 0;
}

bool __cdecl Item_EnableShowViaDvar(const itemDef_s *item, int flag)
{
    const char *testValue; // [esp+0h] [ebp-40Ch]
    char val[1024]; // [esp+4h] [ebp-408h] BYREF
    const char *p; // [esp+408h] [ebp-4h] BYREF

    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp", 226, 0, "%s", "item") )
        __debugbreak();
    if ( !item->enableDvar || !*item->enableDvar || !item->dvarTest || !*item->dvarTest )
        return 1;
    testValue = Dvar_GetVariantString(item->dvarTest);
    p = item->enableDvar;
    do
    {
        if ( !String_Parse(&p, val, 1024) )
            return (flag & item->dvarFlags) == 0;
    }
    while ( val[0] == 59 && !val[1] || I_stricmp(testValue, val) );
    return (flag & item->dvarFlags) != 0;
}

void __cdecl Item_SetTextRect(int contextIndex, itemDef_s *item, const rectDef_s *textRect)
{
    if ( contextIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp",
                    253,
                    0,
                    "contextIndex doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    contextIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp", 254, 0, "%s", "item") )
        __debugbreak();
    if ( !textRect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp", 255, 0, "%s", "textRect") )
        __debugbreak();
    if ( textRect->horzAlign > 0xAu
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp",
                    256,
                    0,
                    "%s\n\t(textRect->horzAlign) = %i",
                    "(textRect->horzAlign >= 0 && textRect->horzAlign <= 10)",
                    textRect->horzAlign) )
    {
        __debugbreak();
    }
    if ( textRect->vertAlign > 0xAu
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp",
                    257,
                    0,
                    "%s\n\t(textRect->vertAlign) = %i",
                    "(textRect->vertAlign >= 0 && textRect->vertAlign <= 10)",
                    textRect->vertAlign) )
    {
        __debugbreak();
    }
    item->typeData.textDef->textRect[contextIndex] = *textRect;
}

int __cdecl Item_GetCursorPosOffset(int contextIndex, itemDef_s *item, const char *text, int delta)
{
    int pos; // [esp+0h] [ebp-8h]

    if ( contextIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp",
                    275,
                    0,
                    "contextIndex doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    contextIndex,
                    1) )
    {
        __debugbreak();
    }
    pos = Item_GetEditFieldDef(item)->cursorPos[contextIndex];
    if ( delta > 0 )
    {
        while ( 1 )
        {
            while ( &text[pos]
                     && text[pos] == 94
                     && text[pos + 1]
                     && text[pos + 1] != 94
                     && text[pos + 1] >= 48
                     && text[pos + 1] <= 64 )
                pos += 2;
            if ( !text[pos] || !delta )
                break;
            ++pos;
            --delta;
        }
    }
    else
    {
        while ( pos && delta )
        {
            if ( pos < 2
                || &text[pos] == (const char *)2
                || text[pos - 2] != 94
                || !text[pos - 1]
                || text[pos - 1] == 94
                || text[pos - 1] < 48
                || text[pos - 1] > 64 )
            {
                --pos;
                ++delta;
            }
            else
            {
                pos -= 2;
            }
        }
    }
    return pos;
}

bool __cdecl ListBox_HasValidCursorPos(int contextIndex, itemDef_s *item)
{
    const listBoxDef_s *listPtr; // [esp+4h] [ebp-8h]
    int cursorPos; // [esp+8h] [ebp-4h]

    if ( contextIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp",
                    318,
                    0,
                    "contextIndex doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    contextIndex,
                    1) )
    {
        __debugbreak();
    }
    listPtr = Item_GetListBoxDef(item);
    cursorPos = listPtr->cursorPos[contextIndex];
    return cursorPos >= listPtr->startPos[contextIndex] && cursorPos < listPtr->endPos[contextIndex];
}

void __cdecl Menu_UpdatePosition(int contextIndex, menuDef_t *menu)
{
    int i; // [esp+10h] [ebp-10h]
    float x; // [esp+14h] [ebp-Ch]
    float y; // [esp+18h] [ebp-8h]
    const rectDef_s *rect; // [esp+1Ch] [ebp-4h]

    if ( menu )
    {
        rect = Window_GetRect(&menu->window);
        x = rect->x;
        y = rect->y;
        if ( menu->window.border )
        {
            x = x + menu->window.borderSize;
            y = y + menu->window.borderSize;
        }
        for ( i = 0; i < menu->itemCount; ++i )
            Item_SetScreenCoords(contextIndex, menu->items[i], x, y, rect->horzAlign, rect->vertAlign);
    }
}

void __cdecl Item_SetScreenCoords(int contextIndex, itemDef_s *item, float x, float y, int horzAlign, int vertAlign)
{
    rectDef_s newRect; // [esp+0h] [ebp-1Ch] BYREF
    const rectDef_s *textRect; // [esp+18h] [ebp-4h]

    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp", 333, 0, "%s", "item") )
        __debugbreak();
    if ( item )
    {
        if ( item->window.border )
        {
            x = x + item->window.borderSize;
            y = y + item->window.borderSize;
        }
        item->window.rect = item->window.rectClient;
        item->window.rect.x = item->window.rect.x + x;
        item->window.rect.y = item->window.rect.y + y;
        if ( !item->window.rect.horzAlign && !item->window.rect.vertAlign )
        {
            item->window.rect.horzAlign = horzAlign;
            item->window.rect.vertAlign = vertAlign;
        }
        textRect = (const rectDef_s *)Item_GetTextRect(contextIndex, item);
        if ( Item_IsTextDefType(item) )
        {
            if ( textRect )
            {
                newRect.x = textRect->x;
                newRect.y = textRect->y;
                memset(&newRect.w, 0, 16);
                Item_SetTextRect(contextIndex, item, &newRect);
            }
        }
    }
}

int __cdecl Item_IsEditFieldDef(itemDef_s *item)
{
    int result; // eax

    switch ( item->dataType )
    {
        case 5:
        case 7:
        case 8:
        case 9:
        case 0xC:
        case 0xD:
        case 0xE:
        case 0x10:
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

int __cdecl Item_IsTextDefType(itemDef_s *item)
{
    int result; // eax

    switch ( item->dataType )
    {
        case 1:
        case 3:
        case 4:
        case 5:
        case 7:
        case 8:
        case 9:
        case 0xA:
        case 0xB:
        case 0xC:
        case 0xD:
        case 0xE:
        case 0xF:
        case 0x10:
        case 0x12:
        case 0x14:
        case 0x16:
            result = 1;
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

int __cdecl Item_IsFocusDefType(itemDef_s *item)
{
    int result; // eax

    switch ( item->dataType )
    {
        case 3:
        case 4:
        case 5:
        case 7:
        case 8:
        case 9:
        case 0xA:
        case 0xB:
        case 0xC:
        case 0xD:
        case 0xE:
        case 0x10:
        case 0x13:
        case 0x14:
        case 0x15:
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

bool __cdecl Item_IsOwnerDrawDefType(itemDef_s *item)
{
    int dataType; // [esp+0h] [ebp-4h]

    dataType = item->dataType;
    return dataType == 6 || dataType > 17 && dataType <= 20;
}

editFieldDef_s *__cdecl Item_GetEditFieldDef(itemDef_s *item)
{
    if ( Item_IsEditFieldDef(item) )
        return item->typeData.textDef->textTypeData.focusItemDef->focusTypeData.editField;
    Com_PrintError(
        13,
        "Menu Error: Expecting type: ITEM_TYPE_EDITFIELD, ITEM_TYPE_NUMERICFIELD, ITEM_TYPE_DECIMALFIELD, ITEM_TYPE_VALIDFILE"
        "FIELD, ITEM_TYPE_UPREDITFIELD, ITEM_TYPE_YESNO, ITEM_TYPE_BIND, ITEM_TYPE_SLIDER, ITEM_TYPE_ALPHANUMERICFIELD\n");
    return 0;
}

multiDef_s *__cdecl Item_GetMultiDef(itemDef_s *item)
{
    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp", 499, 0, "%s", "item") )
        __debugbreak();
    if ( item->dataType != 10
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp",
                    500,
                    0,
                    "%s\n\t(item->dataType) = %i",
                    "(item->dataType == 10)",
                    item->dataType) )
    {
        __debugbreak();
    }
    return item->typeData.textDef->textTypeData.focusItemDef->focusTypeData.multi;
}

ownerDrawDef_s *__cdecl Item_GetOwnerDrawDef(itemDef_s *item)
{
    if ( item->dataType == 6 )
        return item->typeData.ownerDrawDef;
    Com_PrintError(13, "Menu Error: Expecting type: ITEM_TYPE_OWNERDRAW\n");
    return 0;
}

textDef_s *__cdecl Item_GetTextDef(itemDef_s *item)
{
    if ( Item_IsTextDefType(item) )
        return item->typeData.textDef;
    else
        return 0;
}

focusItemDef_s *__cdecl Item_GetFocusItemDef(itemDef_s *item)
{
    if ( item->dataType == 21 )
        return item->typeData.blankButtonDef;
    if ( Item_IsFocusDefType(item) )
        return item->typeData.textDef->textTypeData.focusItemDef;
    return 0;
}

listBoxDef_s *__cdecl Item_GetListBoxDef(itemDef_s *item)
{
    if ( item->dataType == 4 )
        return item->typeData.textDef->textTypeData.focusItemDef->focusTypeData.listBox;
    Com_PrintError(13, "Menu Error: Expecting type: ITEM_TYPE_LISTBOX\n");
    return 0;
}

double __cdecl Item_GetListBoxFeederID(const itemDef_s *item)
{
    if ( item->dataType == 4 )
        return item->typeData.textDef->textTypeData.focusItemDef->focusTypeData.listBox->special;
    Com_PrintError(13, "Menu Error: Expecting type: ITEM_TYPE_LISTBOX\n");
    return -1.0;
}

enumDvarDef_s *__cdecl Item_GetEnumDvarName(itemDef_s *item)
{
    if ( item->dataType == 11 )
        return item->typeData.textDef->textTypeData.focusItemDef->focusTypeData.enumDvar;
    Com_PrintError(13, "Menu Error: Expecting type: ITEM_TYPE_DVARENUM\n");
    return 0;
}

gameMsgDef_s *__cdecl Item_GetGameMsgDef(itemDef_s *item)
{
    if ( item->dataType == 15 )
        return item->typeData.textDef->textTypeData.gameMsgDef;
    Com_PrintError(13, "Menu Error: Expecting type: ITEM_TYPE_GAME_MESSAGE_WINDOW\n");
    return 0;
}

void __cdecl Item_PropertiesToAnimState(itemDef_s *item, animParamsDef_t *animParams)
{
    animParams->name = item->window.name;
    animParams->rectClient = item->window.rectClient;
    animParams->borderSize = item->window.borderSize;
    animParams->rotation = item->window.rotation;
    animParams->foreColor[0] = item->window.foreColor[0];
    animParams->foreColor[1] = item->window.foreColor[1];
    animParams->foreColor[2] = item->window.foreColor[2];
    animParams->foreColor[3] = item->window.foreColor[3];
    animParams->backColor[0] = item->window.backColor[0];
    animParams->backColor[1] = item->window.backColor[1];
    animParams->backColor[2] = item->window.backColor[2];
    animParams->backColor[3] = item->window.backColor[3];
    animParams->borderColor[0] = item->window.borderColor[0];
    animParams->borderColor[1] = item->window.borderColor[1];
    animParams->borderColor[2] = item->window.borderColor[2];
    animParams->borderColor[3] = item->window.borderColor[3];
    animParams->outlineColor[0] = item->window.outlineColor[0];
    animParams->outlineColor[1] = item->window.outlineColor[1];
    animParams->outlineColor[2] = item->window.outlineColor[2];
    animParams->outlineColor[3] = item->window.outlineColor[3];
    if ( Item_IsTextDefType(item) )
        animParams->textScale = item->typeData.textDef->textscale;
}

void __cdecl Item_AnimStateToProperties(animParamsDef_t *animParams, itemDef_s *item)
{
    item->window.name = animParams->name;
    item->window.rectClient = animParams->rectClient;
    item->window.borderSize = animParams->borderSize;
    item->window.rotation = animParams->rotation;
    item->window.foreColor[0] = animParams->foreColor[0];
    item->window.foreColor[1] = animParams->foreColor[1];
    item->window.foreColor[2] = animParams->foreColor[2];
    item->window.foreColor[3] = animParams->foreColor[3];
    item->window.backColor[0] = animParams->backColor[0];
    item->window.backColor[1] = animParams->backColor[1];
    item->window.backColor[2] = animParams->backColor[2];
    item->window.backColor[3] = animParams->backColor[3];
    item->window.borderColor[0] = animParams->borderColor[0];
    item->window.borderColor[1] = animParams->borderColor[1];
    item->window.borderColor[2] = animParams->borderColor[2];
    item->window.borderColor[3] = animParams->borderColor[3];
    item->window.outlineColor[0] = animParams->outlineColor[0];
    item->window.outlineColor[1] = animParams->outlineColor[1];
    item->window.outlineColor[2] = animParams->outlineColor[2];
    item->window.outlineColor[3] = animParams->outlineColor[3];
    if ( Item_IsTextDefType(item) )
        item->typeData.textDef->textscale = animParams->textScale;
}

unsigned __int8 *__cdecl UI_Alloc(unsigned int size, int alignment)
{
    return Hunk_AllocAlign(size, alignment, "UI_Alloc", 36);
}

void __cdecl String_Init()
{
    memset(g_strHandle, 0, sizeof(g_strHandle));
}

static const char *staticNULL = "";
const char *__cdecl String_Alloc(const char *p)
{
    char v2; // [esp+3h] [ebp-45h]
    unsigned __int8 *v3; // [esp+8h] [ebp-40h]
    const char *v4; // [esp+Ch] [ebp-3Ch]
    stringDef_s *str; // [esp+34h] [ebp-14h]
    stringDef_s *stra; // [esp+34h] [ebp-14h]
    stringDef_s *strb; // [esp+34h] [ebp-14h]
    int hash; // [esp+38h] [ebp-10h]
    unsigned __int8 *s; // [esp+40h] [ebp-8h]
    stringDef_s *last; // [esp+44h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp", 656, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !p )
        return 0;
    if ( !*p )
        return staticNULL;
    hash = hashForString(p);
    for ( str = g_strHandle[hash]; str; str = str->next )
    {
        if ( !str->str && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp", 673, 0, "%s", "str->str") )
            __debugbreak();
        if ( !strcmp(p, str->str) )
            return str->str;
    }
    s = UI_Alloc(strlen(p) + 1, 1);
    v4 = p;
    v3 = s;
    do
    {
        v2 = *v4;
        *v3++ = *v4++;
    }
    while ( v2 );
    stra = g_strHandle[hash];
    last = stra;
    while ( stra && stra->next )
    {
        last = stra;
        stra = stra->next;
    }
    strb = (stringDef_s *)UI_Alloc(8u, 4);
    strb->next = 0;
    strb->str = (const char *)s;
    if ( last )
        last->next = strb;
    else
        g_strHandle[hash] = strb;
    return (const char *)s;
}

int __cdecl hashForString(const char *str)
{
    int hash; // [esp+0h] [ebp-Ch]
    int i; // [esp+8h] [ebp-4h]

    hash = 0;
    for ( i = 0; str[i]; ++i )
        hash += tolower(str[i]) * (i + 119);
    return hash & 0x7FF;
}

int __cdecl Int_Parse(const char **p, int *i)
{
    parseInfo_t *token; // [esp+0h] [ebp-4h]

    token = Com_ParseOnLine(p);
    if ( !token || !token->token[0] )
        return 0;
    *i = atoi(token->token);
    return 1;
}

int __cdecl Float_Parse(const char **p, float *f)
{
    parseInfo_t *token; // [esp+0h] [ebp-4h]

    token = Com_ParseOnLine(p);
    if ( !token || !token->token[0] )
        return 0;
    *f = atof(token->token);
    return 1;
}

void __cdecl UI_GetGameTypesList()
{
    sharedUiInfo.numGameTypes = 0;
    sharedUiInfo.numJoinGameTypes = 0;
    I_strncpyz(sharedUiInfo.joinGameTypes[0].gameType, "All", 12);
    sharedUiInfo.joinGameTypes[sharedUiInfo.numJoinGameTypes++].gameTypeName[0] = 0;
    if (useFastFile->current.enabled)
        UI_GetGameTypesList_FastFile();
    else
        UI_GetGameTypesList_LoadObj();
    if (!sharedUiInfo.numGameTypes)
        Com_Error(ERR_FATAL, "No game type scripts found in %sgametypes folder", "maps/mp/");
}

void UI_GetGameTypesList_LoadObj()
{
    char *v1; // eax
    unsigned int v2; // [esp+0h] [ebp-1030h]
    char *src; // [esp+10h] [ebp-1020h]
    char *data_p; // [esp+18h] [ebp-1018h] BYREF
    char *v5; // [esp+1Ch] [ebp-1014h]
    char listbuf[4096]; // [esp+20h] [ebp-1010h] BYREF
    int i; // [esp+1024h] [ebp-Ch]
    char *MenuBuffer; // [esp+1028h] [ebp-8h]
    int FileList; // [esp+102Ch] [ebp-4h]

    FileList = FS_GetFileList("maps/mp/gametypes", (char*)"gsc", FS_LIST_PURE_ONLY, listbuf, 4096);
    src = listbuf;
    for (i = 0; i < FileList; ++i)
    {
        v2 = strlen(src);
        if (*src == 95)
        {
            src += v2 + 1;
        }
        else
        {
            if (!I_stricmp(&src[v2 - 4], ".gsc"))
                src[v2 - 4] = 0;
            if (sharedUiInfo.numGameTypes == 32 || sharedUiInfo.numJoinGameTypes == 32)
            {
                Com_Printf(13, "Too many game type scripts found! Only loading the first %i\n", 31);
                return;
            }
            I_strncpyz(sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameType, src, 12);
            I_strncpyz(
                sharedUiInfo.joinGameTypes[sharedUiInfo.numJoinGameTypes].gameType,
                sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameType,
                12);
            v1 = va("%s%s%s.txt", "maps/mp/", "gametypes/", src);
            MenuBuffer = UI_GetMenuBuffer(v1);
            data_p = MenuBuffer;
            if (MenuBuffer)
            {
                v5 = (char *)Com_Parse((const char **)&data_p);
                I_strncpyz(sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameTypeName, v5, 32);
            }
            else
            {
                I_strncpyz(
                    sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameTypeName,
                    sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameType,
                    32);
            }
            I_strncpyz(
                sharedUiInfo.joinGameTypes[sharedUiInfo.numJoinGameTypes].gameTypeName,
                sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameTypeName,
                32);
            ++sharedUiInfo.numGameTypes;
            ++sharedUiInfo.numJoinGameTypes;
            src += v2 + 1;
        }
    }
}

void UI_GetGameTypesList_FastFile()
{
    char *v1; // eax
    char *v2; // eax
    parseInfo_t *pszFileName; // [esp+4h] [ebp-18h]
    const char *pBuffParse; // [esp+8h] [ebp-14h] BYREF
    const char *pToken; // [esp+Ch] [ebp-10h]
    RawFile *gametypesFile; // [esp+10h] [ebp-Ch]
    char *pBuff; // [esp+14h] [ebp-8h]
    const char *gametypesBuf; // [esp+18h] [ebp-4h] BYREF

    v1 = va("%sgametypes/_gametypes.txt", "maps/mp/");
    gametypesFile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, v1, 1, -1).rawfile;
    if (gametypesFile)
    {
        gametypesBuf = gametypesFile->buffer;
        while (1)
        {
            pszFileName = Com_Parse(&gametypesBuf);
            if (!gametypesBuf)
                break;
            if (sharedUiInfo.numGameTypes == 32 || sharedUiInfo.numJoinGameTypes == 32)
            {
                Com_Printf(13, "Too many game type scripts found! Only loading the first %i\n", 31);
                return;
            }
            I_strncpyz(sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameType, pszFileName->token, 12);
            I_strncpyz(
                sharedUiInfo.joinGameTypes[sharedUiInfo.numJoinGameTypes].gameType,
                sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameType,
                12);
            v2 = va("%sgametypes/%s.txt", "maps/mp/", pszFileName->token);
            pBuff = UI_GetMenuBuffer(v2);
            pBuffParse = pBuff;
            if (pBuff)
            {
                pToken = (const char *)Com_Parse(&pBuffParse);
                I_strncpyz(sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameTypeName, pToken, 32);
                I_strncpyz(
                    sharedUiInfo.joinGameTypes[sharedUiInfo.numJoinGameTypes].gameTypeName,
                    sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameTypeName,
                    32);
                pToken = (const char *)Com_Parse(&pBuffParse);
                I_strncpyz(sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameTypeNameCaps, pToken, 32);
                I_strncpyz(
                    sharedUiInfo.joinGameTypes[sharedUiInfo.numJoinGameTypes].gameTypeNameCaps,
                    sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameTypeNameCaps,
                    32);
                pToken = (const char *)Com_Parse(&pBuffParse);
                I_strncpyz(sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameTypeShortName, pToken, 32);
            }
            else
            {
                I_strncpyz(
                    sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameTypeName,
                    sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameType,
                    32);
                I_strncpyz(
                    sharedUiInfo.joinGameTypes[sharedUiInfo.numJoinGameTypes].gameTypeName,
                    sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameTypeName,
                    32);
                I_strncpyz(
                    sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameTypeShortName,
                    sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameTypeName,
                    32);
            }
            ++sharedUiInfo.numGameTypes;
            ++sharedUiInfo.numJoinGameTypes;
        }
    }
}

const char *__cdecl UI_GetBusyDotsIndicator()
{
    int v1; // [esp+0h] [ebp-8h]

    v1 = ((int)Sys_Milliseconds() / 500) & 3;
    switch ( v1 )
    {
        case 1:
            return ".    ";
        case 2:
            return ".. ";
        case 3:
            return "...";
    }
    return "     ";
}

void __cdecl UI_ListMenus_f()
{
    uiInfo_s *uiInfo; // [esp+0h] [ebp-4h]

    uiInfo = UI_GetInfo(0);
    Menus_PrintAllLoadedMenus(&uiInfo->uiDC);
}

void __cdecl CL_SelectStringTableEntryInDvar_f()
{
    const char *v0; // eax
    unsigned int v1; // eax
    const char *v2; // eax
    int v3; // eax
    const char *v4; // eax
    char *ColumnValueForRow; // [esp-4h] [ebp-30h]
    float rowCount; // [esp+0h] [ebp-2Ch]
    const StringTable *table; // [esp+24h] [ebp-8h] BYREF
    int row; // [esp+28h] [ebp-4h]

    if ( !G_ExitAfterToolComplete() )
    {
        if ( Cmd_Argc() >= 4 )
        {
            v0 = Cmd_Argv(1);
            StringTable_GetAsset(v0, (XAssetHeader *)&table);
            v1 = Sys_Milliseconds();
            srand(v1);
            rowCount = (float)table->rowCount;
            row = (int)(float)((float)(rowCount * (float)rand()) / 32767.0);
            v2 = Cmd_Argv(2);
            v3 = atoi(v2);
            ColumnValueForRow = (char *)StringTable_GetColumnValueForRow(table, row, v3);
            v4 = Cmd_Argv(3);
            Dvar_SetStringByName(v4, ColumnValueForRow);
        }
        else
        {
            Com_Printf(16, "usage: selectStringTableEntryInDvar <tableFileName> <columnNum> <dvarName>");
        }
    }
}

bool __cdecl UI_VerifyString(const char *str)
{
    unsigned int v1; // kr00_4
    bool v3; // [esp+0h] [ebp-20h]
    bool hasNonSpace; // [esp+1Bh] [ebp-5h]
    unsigned int idx; // [esp+1Ch] [ebp-4h]

    if ( !str && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils.cpp", 1058, 0, "%s", "str") )
        __debugbreak();
    v1 = strlen(str);
    hasNonSpace = 0;
    for ( idx = 0; idx < v1; ++idx )
    {
        if ( str[idx] == 94
            || str[idx] == -92
            || str[idx] == 123
            || str[idx] == 125
            || str[idx] == 64
            || str[idx] == 37
            || str[idx] == 92
            || str[idx] == 59
            || str[idx] == 34
            || str[idx] < 32 )
        {
            return 0;
        }
        v3 = hasNonSpace || str[idx] != 32;
        hasNonSpace = v3;
    }
    return hasNonSpace;
}

