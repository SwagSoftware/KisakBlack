#include "ui_shared_obj.h"
#include <universal/com_files.h>
#include <universal/q_parse.h>
#include "ui_utils.h"
#include "l_precomp.h"
#include <universal/eval.h>
#include <client_mp/cl_scrn_mp.h>
#include <qcommon/dobj_management.h>
#include <game_mp/g_main_mp.h>

#include <cctype>

template <typename T, int HASH_SIZE, int HASH_SEED>
struct KeywordHashEntry
{
    const char *keyword;
    int(__cdecl *func)(T *def, int handle);

    // --------------------------------------------------------
    // Hash core (seeded runtime version)
    // --------------------------------------------------------
    static int KeySeed(const char *keyword, int hashCount, int seed)
    {
        int hash = 0;

        for (int i = 0; keyword[i]; ++i)
            hash += (i + seed) * std::tolower((unsigned char)keyword[i]);

        hash += (hash >> 8);

        return hash % hashCount;
    }

    // --------------------------------------------------------
    // Compile-time specialized hash (fast path)
    // Uses power-of-two mask instead of %
    // --------------------------------------------------------
    static int Key(const char *keyword)
    {
        int hash = 0;

        for (int i = 0; keyword[i]; ++i)
            hash += (i + HASH_SEED) * std::tolower((unsigned char)keyword[i]);

        hash += (hash >> 8);

        return hash & (HASH_SIZE - 1);
    }

    // --------------------------------------------------------
    // Lookup
    // --------------------------------------------------------
    static const KeywordHashEntry *
        Find(const KeywordHashEntry **table, const char *keyword)
    {
        const KeywordHashEntry *entry = table[Key(keyword)];

        if (!entry)
            return nullptr;

        if (I_stricmp(entry->keyword, keyword) != 0)
            return nullptr;

        return entry;
    }

    // --------------------------------------------------------
    // Add (asserts on collision)
    // --------------------------------------------------------
    static void Add(
        const KeywordHashEntry **table,
        const KeywordHashEntry *entry)
    {
        int hash = Key(entry->keyword);

        if (table[hash])
        {
            if (!Assert_MyHandler(
                "ui_shared_obj.cpp",
                1211,
                0,
                "%s",
                "table[hash] == NULL"))
            {
                __debugbreak();
            }
        }

        table[hash] = entry;
    }

    // --------------------------------------------------------
    // Validate seed (no collisions allowed)
    // --------------------------------------------------------
    static bool IsValidSeed(
        const KeywordHashEntry *array,
        int count,
        int seed)
    {
        unsigned char used[HASH_SIZE];
        std::memset(used, 0, sizeof(used));

        for (int i = 0; i < count; ++i)
        {
            int hash = KeySeed(array[i].keyword, HASH_SIZE, seed);

            if (used[hash])
                return false;

            used[hash] = 1;
        }

        return true;
    }

    // --------------------------------------------------------
    // Brute force seed picker
    // --------------------------------------------------------
    static int PickSeed(
        const KeywordHashEntry *array,
        int count)
    {
        for (int seed = 0;; ++seed)
        {
            if (IsValidSeed(array, count, seed))
                return seed;

            if (seed == 0x10000)
            {
                if (!Assert_MyHandler(
                    "ui_shared_obj.cpp",
                    1189,
                    0,
                    "seed != 65536\n\t%i, %i",
                    0x10000,
                    0x10000))
                {
                    __debugbreak();
                }
            }
        }
    }

    // --------------------------------------------------------
    // Validate compile-time seed
    // --------------------------------------------------------
    static void Validate(
        const KeywordHashEntry *array,
        int count)
    {
        if (!IsValidSeed(array, count, HASH_SEED))
        {
            int picked = PickSeed(array, count);

            if (!Assert_MyHandler(
                "ui_shared_obj.cpp",
                1201,
                0,
                "%s\n\t(KeywordHash_PickSeed( array, count )) = %i",
                "(KeywordHash_IsValidSeed( array, count, HASH_SEED ))",
                picked))
            {
                __debugbreak();
            }
        }
    }
};



const KeywordHashEntry<itemDef_s, 1024, 6> itemParseKeywords[91] =
{
  { "name", &ItemParse_name },
  { "text", &ItemParse_text },
  { "textfile", &ItemParse_textfile },
  { "textsavegame", &ItemParse_textsavegame },
  { "textcinematicsubtitle", &ItemParse_textcinematicsubtitle },
  { "group", &ItemParse_group },
  { "rect", &ItemParse_rect },
  { "origin", &ItemParse_origin },
  { "style", &ItemParse_style },
  { "decoration", &ItemParse_decoration },
  { "notselectable", &ItemParse_notselectable },
  { "noscrollbars", &ItemParse_noScrollBars },
  { "modal", &ItemParse_modal },
  { "frame", &ItemParse_frame },
  { "noBlinkingHighlight", &ItemParse_noBlinkingHighlight },
  { "usepaging", &ItemParse_usePaging },
  { "autowrapped", &ItemParse_autowrapped },
  { "horizontalscroll", &ItemParse_horizontalscroll },
  { "type", &ItemParse_type },
  { "elementwidth", &ItemParse_elementwidth },
  { "elementheight", &ItemParse_elementheight },
  { "feeder", &ItemParse_special },
  { "elementtype", &ItemParse_elementtype },
  { "columns", &ItemParse_columns },
  { "userarea", &ItemParse_userarea },
  { "menuarea", &ItemParse_menuarea },
  { "menuItemsDef", &ItemParse_menuItemsDef },
  { "border", &ItemParse_border },
  { "bordersize", &ItemParse_bordersize },
  { "visible", &ItemParse_visible },
  { "visibilityBits", &ItemParse_visiblityBits },
  { "ownerdraw", &ItemParse_ownerdraw },
  { "align", &ItemParse_align },
  { "textalign", &ItemParse_textalign },
  { "textalignx", &ItemParse_textalignx },
  { "textaligny", &ItemParse_textaligny },
  { "textscale", &ItemParse_textscale },
  { "textstyle", &ItemParse_textstyle },
  { "textfont", &ItemParse_textfont },
  { "backcolor", &ItemParse_backcolor },
  { "forecolor", &ItemParse_forecolor },
  { "bordercolor", &ItemParse_bordercolor },
  { "outlinecolor", &ItemParse_outlinecolor },
  { "rotation", &ItemParse_rotation },
  { "background", &ItemParse_background },
  { "onFocus", &ItemParse_onFocus },
  { "leaveFocus", &ItemParse_leaveFocus },
  { "mouseEnter", &ItemParse_mouseEnter },
  { "mouseExit", &ItemParse_mouseExit },
  { "mouseEnterText", &ItemParse_mouseEnterText },
  { "mouseExitText", &ItemParse_mouseExitText },
  { "action", &ItemParse_action },
  { "accept", &ItemParse_accept },
  { "special", &ItemParse_special },
  { "dvar", &ItemParse_dvar },
  { "maxChars", &ItemParse_maxChars },
  { "maxCharsGotoNext", &Item_Parse_maxCharsGotoNext },
  { "maxPaintChars", &ItemParse_maxPaintChars },
  { "focusSound", &ItemParse_focusSound },
  { "dvarFloat", &ItemParse_dvarFloat },
  { "dvarStrList", &ItemParse_dvarStrList },
  { "dvarFloatList", &ItemParse_dvarFloatList },
  { "dvarEnumList", &ItemParse_dvarEnumList },
  { "actionOnEnterPressOnly", &ItemParse_actionOnEnterPressOnly },
  { "ownerdrawFlag", &ItemParse_ownerdrawFlag },
  { "enableDvar", &ItemParse_enableDvar },
  { "dvarTest", &ItemParse_dvarTest },
  { "disableDvar", &ItemParse_disableDvar },
  { "showDvar", &ItemParse_showDvar },
  { "hideDvar", &ItemParse_hideDvar },
  { "focusDvar", &ItemParse_focusDvar },
  { "doubleclick", &ItemParse_doubleClick },
  { "rightclick", &ItemParse_rightClick },
  { "execKey", &ItemParse_execKey },
  { "execKeyInt", &ItemParse_execKeyInt },
  { "onEvent", &ItemParse_onEvent },
  { "exp", &ItemParse_execExp },
  { "gamemsgwindowindex", &ItemParse_gameMsgWindowIndex },
  { "gamemsgwindowmode", &ItemParse_gameMsgWindowMode },
  { "selectBorder", &ItemParse_selectBorder },
  { "elementHighlightColor", &ItemParse_elementHighlightColor },
  { "elementBackgroundColor", &ItemParse_elementBackgroundColor },
  { "disablecolor", &ItemParse_disableColor },
  { "focusColor", &ItemParse_focusColor },
  { "selectIcon", &ItemParse_selectIcon },
  { "backgroundItemListbox", &ItemParse_backgroundItemListbox },
  { "highlightTexture", &ItemParse_highlightTexture },
  { "onListboxSelectionChange", &ItemParse_onListboxSelectionChange },
  { "ui3dWindowId", &ItemParse_ui3dWindowId },
  { "state", &ItemParse_state },
  { "onEnter", &ItemParse_onEnter }
};

const KeywordHashEntry<menuDef_t, 1024, 128> menuParseKeywords[50] =
{
  { "name", &MenuParse_name },
  { "fullscreen", &MenuParse_fullscreen },
  { "rect", &MenuParse_rect },
  { "style", (int(*)(menuDef_t*, int))&ItemParse_style},
  { "visible", &MenuParse_visible },
  { "visibilityBits", &MenuParse_visiblityBits },
  { "onOpen", &MenuParse_onOpen },
  { "onFocus", &MenuParse_onFocus },
  { "leaveFocus", &MenuParse_leaveFocus },
  { "onClose", &MenuParse_onClose },
  { "onESC", &MenuParse_onESC },
  { "border", &MenuParse_border },
  { "borderSize", &MenuParse_borderSize },
  { "backcolor", &MenuParse_backcolor },
  { "forecolor", &MenuParse_forecolor },
  { "bordercolor", &MenuParse_bordercolor },
  { "focuscolor", &MenuParse_focuscolor },
  { "disablecolor", &MenuParse_disablecolor },
  { "outlinecolor", &MenuParse_outlinecolor },
  { "background", &MenuParse_background },
  { "ownerdraw", &MenuParse_ownerdraw },
  { "ownerdrawFlag", (int(*)(menuDef_t *, int)) &ItemParse_ownerdrawFlag },
  { "outOfBoundsClick", &MenuParse_outOfBounds },
  { "soundLoop", &MenuParse_soundLoop },
  { "itemDef", &MenuParse_itemDef },
  { "exp", &MenuParse_execExp },
  { "popup", &MenuParse_popup },
  { "fadeClamp", &MenuParse_fadeClamp },
  { "fadeCycle", &MenuParse_fadeCycle },
  { "fadeAmount", &MenuParse_fadeAmount },
  { "fadeInAmount", &MenuParse_fadeInAmount },
  { "execKey", &MenuParse_execKey },
  { "execKeyInt", &MenuParse_execKeyInt },
  { "blurWorld", &MenuParse_blurWorld },
  { "legacySplitScreenScale", &MenuParse_legacySplitScreenScale },
  { "hiddenDuringScope", &MenuParse_hiddenDuringScope },
  { "hiddenDuringFlashbang", &MenuParse_hiddenDuringFlashbang },
  { "hiddenDuringUI", &MenuParse_hiddenDuringUI },
  { "allowedBinding", &MenuParse_allowedBinding },
  { "allowSignIn", &MenuParse_allowSignIn },
  { "ui3dWindowId", &MenuParse_ui3dWindowId },
  { "priority", &MenuParse_priority },
  { "openSlideSpeed", &MenuParse_openSlideSpeed },
  { "closeSlideSpeed", &MenuParse_closeSlideSpeed },
  { "openSlideDirection", &MenuParse_openSlideDirection },
  { "closeSlideDirection", &MenuParse_closeSlideDirection },
  { "openFadingTime", &MenuParse_openFadingTime },
  { "closeFadingTime", &MenuParse_closeFadingTime },
  { "control", &MenuParse_control },
  { "frame", (int(*)(menuDef_t *, int)) &ItemParse_frame }
};



conditionStack_t g_conditionStack;
int g_constructUniqueID;
nestingCounts_t g_nestingCounts;
int g_blockUniqueID;

const KeywordHashEntry<menuDef_t, 1024, 128> *menuParseKeywordHash[1024];
const KeywordHashEntry<itemDef_s, 1024, 6> *itemParseKeywordHash[1024];

using ItemKeyword = KeywordHashEntry<itemDef_s, 1024, 6>;
using MenuKeyword = KeywordHashEntry<menuDef_t, 1024, 128>;

struct// $CD64A558AFC89A5F4974E935559855BB // sizeof=0x141C
{                                       // XREF: .data:g_load_0/r
    loadAssets_t loadAssets;            // XREF: Asset_Parse+100/o
                                        // Asset_Parse+13E/o ...
    MenuList menuList;                  // XREF: UI_LoadMenu_LoadObj+17/w
                                        // UI_LoadMenu_LoadObj:loc_77E012/o ...
    itemDef_s *items[512];              // XREF: Menu_Init+7C/o
                                        // Menu_PostParse+5A/o
    animParamsDef_t *animStates[256];   // XREF: Item_Init+72/o
                                        // Item_PostParse+6B/o ...
    menuDef_t *menus[512];              // XREF: UI_LoadMenu_LoadObj+17/o
                                        // UI_LoadMenus_LoadObj+1A/o
} g_load_0;

void __cdecl Menus_FreeAllMemory(UiContext *dc)
{
    int menu; // [esp+0h] [ebp-4h]

    for ( menu = 0; menu < dc->menuCount; ++menu )
        Menu_FreeMemory(dc->Menus[menu]);
}

void __cdecl Menu_FreeMemory(menuDef_t *menu)
{
    int item; // [esp+0h] [ebp-4h]

    for ( item = 0; item < menu->itemCount; ++item )
        Menu_FreeItemMemory(menu->items[item]);
    Expression_Free(&menu->visibleExp);
    Expression_Free(&menu->rectXExp);
    Expression_Free(&menu->rectYExp);
}

void __cdecl Menu_FreeItemMemory(itemDef_s *item)
{
    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared_obj.cpp", 136, 0, "%s", "item") )
        __debugbreak();
    Expression_Free(&item->visibleExp);
    if ( item->typeData.textDef )
        Expression_Free((ExpressionStatement *)item->typeData.textDef);
    if ( item->typeData.textDef && item->typeData.textDef->textExpData )
        Expression_Free(&item->typeData.textDef->textExpData->textExp);
    if ( item->rectExpData )
    {
        Expression_Free(&item->rectExpData->rectXExp);
        Expression_Free(&item->rectExpData->rectYExp);
        Expression_Free(&item->rectExpData->rectWExp);
        Expression_Free(&item->rectExpData->rectHExp);
    }
}

void __cdecl UI_MapLoadInfo(const char *filename)
{
    const char *parse; // [esp+14h] [ebp-118h] BYREF
    int tokenLen; // [esp+18h] [ebp-114h]
    char key[256]; // [esp+1Ch] [ebp-110h] BYREF
    const char *token; // [esp+120h] [ebp-Ch]
    char *loadfile; // [esp+124h] [ebp-8h] BYREF
    const char *value; // [esp+128h] [ebp-4h]

    if ( *filename )
    {
        if ( FS_ReadFile(filename, (void **)&loadfile) >= 0 )
        {
            parse = loadfile;
            Com_BeginParseSession(filename);
            Com_SetCSV(1);
            while ( 1 )
            {
                token = (const char *)Com_Parse(&parse);
                if ( !*token )
                    break;
                tokenLen = strlen(token) + 1;
                if ( (unsigned int)tokenLen >= 0x100 )
                {
                    Com_EndParseSession();
                    Com_Error(ERR_DROP, "key '%s' is %i > %i characters long", key, tokenLen - 1, 255);
                }
                memcpy((unsigned __int8 *)key, (unsigned __int8 *)token, tokenLen);
                value = (const char *)Com_ParseOnLine(&parse);
                if ( !*value )
                {
                    Com_EndParseSession();
                    Com_Error(ERR_DROP, "key '%s' missing value in '%s'\n", key, filename);
                    break;
                }
                UI_ParseMenuMaterial(key, (char *)value);
            }
            Com_EndParseSession();
            FS_FreeFile(loadfile);
        }
        else
        {
            Com_PrintWarning(13, "WARNING: Could not find '%s'.\n", filename);
        }
    }
}

void __cdecl UI_ParseMenuMaterial(const char *key, char *value)
{
    Material *material; // [esp+0h] [ebp-4Ch]
    char name[68]; // [esp+4h] [ebp-48h] BYREF

    material = Material_RegisterHandle(value, 3);
    Com_sprintf(name, 0x40u, "$%s", key);
    I_strlwr(name);
    Material_Duplicate(material, name);
}

void __cdecl PC_ResetConditionStack()
{
    g_conditionStack.size = 0;
}

int __cdecl PC_CndStackPush(
                int handle,
                ExpressionStatement expressionStatement,
                bool fireOnTrue,
                bool incrementLevel,
                int blockID,
                int constructID,
                int type)
{
    if ( g_conditionStack.size == 64 )
    {
        Com_PrintError(16, "Condition parsing encountered a stack overflow. Too many nested ifs?\n");
        return 0;
    }
    else
    {
        ++g_conditionStack.size;
        if ( type != 3 )
            g_conditionStack.members[g_conditionStack.size - 1].condition = expressionStatement;
        g_conditionStack.members[g_conditionStack.size - 1].fireOnTrue = fireOnTrue;
        g_conditionStack.members[g_conditionStack.size - 1].blockID = blockID;
        g_conditionStack.members[g_conditionStack.size - 1].constructID = constructID;
        g_conditionStack.members[g_conditionStack.size - 1].type = type;
        if ( incrementLevel )
        {
            if ( g_conditionStack.size == 1 )
                g_conditionStack.members[0].level = 1;
            else
                g_conditionStack.members[g_conditionStack.size - 1].level = g_conditionStack.members[g_conditionStack.size - 2].level
                                                                                                                                    + 1;
        }
        else
        {
            if ( g_conditionStack.size <= 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared_obj.cpp",
                            727,
                            0,
                            "%s",
                            "g_conditionStack.size > 0") )
            {
                __debugbreak();
            }
            g_conditionStack.members[g_conditionStack.size - 1].level = g_conditionStack.members[g_conditionStack.size - 2].level;
        }
        return 1;
    }
}

int __cdecl PC_CndStackModifyTop()
{
    if ( g_conditionStack.size )
    {
        g_conditionStack.members[g_conditionStack.size - 1].fireOnTrue = 0;
        return 1;
    }
    else
    {
        Com_PrintError(16, "Condition parsing encountered a stack error.\n");
        return 0;
    }
}

int __cdecl PC_CndStackPopLevel()
{
    int i; // [esp+0h] [ebp-8h]
    int levelToPop; // [esp+4h] [ebp-4h]

    if ( g_conditionStack.size )
    {
        levelToPop = g_conditionStack.members[g_conditionStack.size - 1].level;
        for ( i = g_conditionStack.size - 1; i >= 0; --i )
        {
            if ( g_conditionStack.members[i].level == levelToPop )
                --g_conditionStack.size;
        }
        return 1;
    }
    else
    {
        Com_PrintError(16, "Condition parsing encountered a stack error.\n");
        return 0;
    }
}

int __cdecl PC_CndStackAddConditionsToScript(GenericEventScript **baseScript, char *accumulatedScriptCommands)
{
    conditionStackMember_t *v2; // eax
    int i; // [esp+20h] [ebp-14h]
    ScriptCondition *newScriptCondition; // [esp+24h] [ebp-10h]
    ScriptCondition *scriptConditionRunner; // [esp+28h] [ebp-Ch]
    GenericEventScript *eventScriptRunner; // [esp+2Ch] [ebp-8h]
    GenericEventScript *newEventScript; // [esp+30h] [ebp-4h]

    newEventScript = (GenericEventScript *)UI_Alloc(0x2Cu, 4);
    if ( accumulatedScriptCommands && *accumulatedScriptCommands )
    {
        newEventScript->action = (const char *)UI_Alloc(strlen(accumulatedScriptCommands) + 1, 4);
        I_strncpyz((char *)newEventScript->action, accumulatedScriptCommands, strlen(accumulatedScriptCommands));
    }
    else
    {
        newEventScript->action = 0;
    }
    newEventScript->next = 0;
    for ( i = 0; i < g_conditionStack.size - 1; ++i )
    {
        if ( g_conditionStack.members[i].type != 3 )
        {
            newScriptCondition = (ScriptCondition *)UI_Alloc(0x10u, 4);
            newScriptCondition->fireOnTrue = g_conditionStack.members[i].fireOnTrue;
            newScriptCondition->constructID = g_conditionStack.members[i].constructID;
            newScriptCondition->blockID = g_conditionStack.members[i].blockID;
            if ( newEventScript->prerequisites )
            {
                for ( scriptConditionRunner = newEventScript->prerequisites;
                            scriptConditionRunner->next;
                            scriptConditionRunner = scriptConditionRunner->next )
                {
                    ;
                }
                scriptConditionRunner->next = newScriptCondition;
            }
            else
            {
                newEventScript->prerequisites = newScriptCondition;
            }
        }
    }
    v2 = &g_conditionStack.members[g_conditionStack.size - 1];
    newEventScript->condition.filename = v2->condition.filename;
    newEventScript->condition.line = v2->condition.line;
    newEventScript->condition.numRpn = v2->condition.numRpn;
    newEventScript->condition.rpn = v2->condition.rpn;
    newEventScript->fireOnTrue = g_conditionStack.members[g_conditionStack.size - 1].fireOnTrue;
    newEventScript->blockID = g_conditionStack.members[g_conditionStack.size - 1].blockID;
    newEventScript->constructID = g_conditionStack.members[g_conditionStack.size - 1].constructID;
    newEventScript->type = g_conditionStack.members[g_conditionStack.size - 1].type;
    if ( *baseScript )
    {
        for ( eventScriptRunner = *baseScript; eventScriptRunner->next; eventScriptRunner = eventScriptRunner->next )
            ;
        eventScriptRunner->next = newEventScript;
    }
    else
    {
        *baseScript = newEventScript;
    }
    memset(accumulatedScriptCommands, 0, 0x1400u);
    return 1;
}

int __cdecl MenuParse_name(menuDef_t *menu, int handle)
{
    if ( !PC_String_Parse(handle, &menu->window.name) )
        return 0;
    I_strlwr((char *)menu->window.name);
    return 1;
}

int __cdecl PC_String_Parse(int handle, const char **out)
{
    pc_token_s token; // [esp+0h] [ebp-418h] BYREF

    if ( !PC_ReadTokenHandle(handle, &token) )
        return 0;
    *out = String_Alloc(token.string);
    return 1;
}

int    MenuParse_fullscreen(menuDef_t *menu, int handle)
{
    return PC_Int_Parse(handle, &menu->fullScreen);
}

int    PC_Int_Parse(int handle, int *i)
{
    pc_token_s token; // [esp+0h] [ebp-418h] BYREF
    int negative; // [esp+414h] [ebp-4h]

    if ( !PC_ReadTokenHandle(handle, &token) )
        return 0;
    if ( token.string[0] == 40 )
        return PC_Int_Expression_Parse(handle, i);
    negative = 0;
    if ( token.string[0] == 45 )
    {
        if ( !PC_ReadTokenHandle(handle, &token) )
            return 0;
        negative = 1;
    }
    if ( token.type == 3 )
    {
        *i = token.intvalue;
        if ( negative )
            *i = -*i;
        return 1;
    }
    else
    {
        PC_SourceError(handle, (char*)"expected integer but found %s\n", token.string);
        return 0;
    }
}

char string_1[4096];
void PC_SourceError(int handle, const char *format, ...)
{
    char filename[132]; // [esp+0h] [ebp-90h] BYREF
    char *argptr; // [esp+88h] [ebp-8h]
    int line; // [esp+8Ch] [ebp-4h] BYREF
    va_list va; // [esp+A0h] [ebp+10h] BYREF

    va_start(va, format);
    _vsnprintf(string_1, 0x1000u, format, va);
    argptr = 0;
    filename[0] = 0;
    line = 0;
    PC_SourceFileAndLine(handle, filename, &line);
    Com_PrintError(13, "Menu load error: %s, line %d: %s\n", filename, line, string_1);
}

int    PC_Int_Expression_Parse(int handle, int *i)
{
    EvalValue result; // [esp+0h] [ebp-5468h] BYREF
    EvalValue v5; // [esp+10h] [ebp-5458h]
    EvalOperatorType op; // [esp+24h] [ebp-5444h] BYREF
    EvalValue v7; // [esp+28h] [ebp-5440h]
    Eval eval; // [esp+38h] [ebp-5430h] BYREF
    int v9; // [esp+504Ch] [ebp-41Ch]
    pc_token_s pc_token; // [esp+5050h] [ebp-418h] BYREF

    memset(&eval.opStackPos, 0, 13);
    v9 = 0;
    while ( 1 )
    {
        while ( 1 )
        {
            if ( !PC_ReadTokenHandle(handle, &pc_token) )
                return 0;
            if ( pc_token.type != 3 )
                break;
            if ( !Eval_PushInteger(&eval, pc_token.intvalue) )
                goto LABEL_6;
        }
        if ( !Eval_OperatorForToken(pc_token.string, &op) )
        {
            PC_SourceError(handle, (char*)"expected operator but found %s\n", pc_token.string);
            return 0;
        }
        if ( op == EVAL_OP_RPAREN )
            break;
        if ( op == EVAL_OP_LPAREN && ++v9 > 16 )
        {
            PC_SourceError(handle, (char *)"too much recursive macro expansion\n");
            return 0;
        }
LABEL_16:
        Eval_PushOperator(&eval, op);
    }
    if ( v9 )
    {
        --v9;
        goto LABEL_16;
    }
    if ( Eval_AnyMissingOperands(&eval) )
    {
LABEL_6:
        PC_SourceError(handle, (char *)"error evaluating expression\n");
        return 0;
    }
    v5 = *Eval_Solve(&result, &eval);
    v7 = v5;
    if ( v5.type != EVAL_VALUE_INT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared_obj.cpp",
                    539,
                    0,
                    "%s",
                    "result.type == EVAL_VALUE_INT") )
    {
        __debugbreak();
    }
    *i = v7.u.i;
    return 1;
}

int    MenuParse_rect(menuDef_t *menu, int handle)
{
    int result; // eax

    result = PC_Rect_Parse(handle, &menu->window.rect);
    menu->initialRectInfo = menu->window.rect;
    return result;
}

int    PC_Rect_Parse(int handle, rectDef_s *r)
{
    int refPoint; // [esp+8h] [ebp-4h] BYREF

    if ( !PC_Float_Parse(handle, &r->x)
        || !PC_Float_Parse(handle, &r->y)
        || !PC_Float_Parse(handle, &r->w)
        || !PC_Float_Parse(handle, &r->h) )
    {
        return 0;
    }
    if ( !PC_Flag_ParseOptional(handle, &r->horzAlign) )
        r->horzAlign = 0;
    if ( !PC_Flag_ParseOptional(handle, &r->vertAlign) )
        r->vertAlign = 0;
    if ( PC_Flag_ParseOptional(handle, &refPoint) && refPoint )
    {
        if ( refPoint == 1 )
        {
            r->x = r->x - (float)(r->w * 0.5);
        }
        else if ( refPoint == 2 )
        {
            r->x = r->x - r->w;
        }
        else
        {
            PC_SourceError(handle, (char *)"invalid horizontal reference point\n");
        }
    }
    if ( PC_Flag_ParseOptional(handle, &refPoint) && refPoint )
    {
        if ( refPoint == 1 )
        {
            r->y = r->y - (float)(r->h * 0.5);
        }
        else if ( refPoint == 2 )
        {
            r->y = r->y - r->h;
        }
        else
        {
            PC_SourceError(handle, (char *)"invalid vertical reference point\n");
        }
    }
    return 1;
}

int    PC_Float_Parse(int handle, float *f)
{
    pc_token_s token; // [esp+0h] [ebp-418h] BYREF
    int negative; // [esp+414h] [ebp-4h]

    if ( !PC_ReadTokenHandle(handle, &token) )
        return 0;
    if ( token.string[0] == 40 )
        return PC_Float_Expression_Parse(handle, f);
    negative = 0;
    if ( token.string[0] == 45 )
    {
        if ( !PC_ReadTokenHandle(handle, &token) )
            return 0;
        negative = 1;
    }
    if ( token.type == 3 )
    {
        if (negative)
        {
            //*(unsigned int *)f = LODWORD(token.floatvalue) ^ _mask__NegFloat_;
            *f = -(token.floatvalue);
        }
        else
            *f = token.floatvalue;
        return 1;
    }
    else
    {
        PC_SourceError(handle, (char *)"expected float but found %s\n", token.string);
        return 0;
    }
}

int    PC_Float_Expression_Parse(int handle, float *f)
{
    float d; // xmm0_4
    EvalValue result; // [esp+8h] [ebp-5468h] BYREF
    EvalValue v6; // [esp+18h] [ebp-5458h]
    EvalOperatorType op; // [esp+2Ch] [ebp-5444h] BYREF
    EvalValue v8; // [esp+30h] [ebp-5440h]
    Eval eval; // [esp+40h] [ebp-5430h] BYREF
    int v10; // [esp+5054h] [ebp-41Ch]
    pc_token_s pc_token; // [esp+5058h] [ebp-418h] BYREF

    memset(&eval.opStackPos, 0, 13);
    v10 = 0;
    while ( 1 )
    {
        while ( 1 )
        {
            if ( !PC_ReadTokenHandle(handle, &pc_token) )
                return 0;
            if ( pc_token.type != 3 )
                break;
            if ( !Eval_PushNumber(&eval, pc_token.floatvalue) )
                goto LABEL_6;
        }
        if ( !Eval_OperatorForToken(pc_token.string, &op) )
        {
            PC_SourceError(handle, "expected operator but found %s\n", pc_token.string);
            return 0;
        }
        if ( op == EVAL_OP_RPAREN )
            break;
        if ( op == EVAL_OP_LPAREN && ++v10 > 16 )
        {
            PC_SourceError(handle, "too much recursive macro expansion\n");
            return 0;
        }
LABEL_16:
        Eval_PushOperator(&eval, op);
    }
    if ( v10 )
    {
        --v10;
        goto LABEL_16;
    }
    if ( Eval_AnyMissingOperands(&eval) )
    {
LABEL_6:
        PC_SourceError(handle, "error evaluating expression\n");
        return 0;
    }
    v6 = *Eval_Solve(&result, &eval);
    v8 = v6;
    if ( v6.type )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared_obj.cpp",
                        351,
                        0,
                        "%s",
                        "result.type == EVAL_VALUE_DOUBLE") )
            __debugbreak();
    }
    d = v8.u.d;
    *f = d;
    return 1;
}

int __cdecl PC_Flag_ParseOptional(int handle, int *i)
{
    pc_token_s token; // [esp+0h] [ebp-418h] BYREF

    if ( !PC_ReadTokenHandle(handle, &token) )
        return 0;
    if ( token.type == 3 )
    {
        *i = token.intvalue;
        return 1;
    }
    else
    {
        PC_UnreadLastTokenHandle(handle);
        return 0;
    }
}

int    PC_Byte_Parse(int handle, unsigned __int8 *b)
{
    int i; // [esp+0h] [ebp-4h] BYREF

    if ( !PC_Int_Parse(handle, &i) )
        return 0;
    if ( (unsigned int)i < 0x100 )
    {
        *b = i;
        return 1;
    }
    else
    {
        PC_SourceError(handle, "expect int value between 0 and 255, but got '%d'\n", i);
        return 0;
    }
}

int __cdecl MenuParse_visible(menuDef_t *menu, int handle)
{
    const char *string; // [esp+0h] [ebp-Ch] BYREF
    int flags; // [esp+8h] [ebp-4h]

    if ( !PC_String_Parse(handle, &string) )
        return 0;
    if ( !I_stricmp(string, "when") || !I_stricmp(string, "if") )
    {
        flags = Window_GetDynamicFlags(0, &menu->window);
        Window_SetDynamicFlags(0, &menu->window, flags | 4);
        if ( Expression_Read(handle, &menu->visibleExp) )
            return 1;
    }
    if ( atoi(string) )
    {
        flags = Window_GetDynamicFlags(0, &menu->window);
        Window_SetDynamicFlags(0, &menu->window, flags | 4);
    }
    return 1;
}

int __cdecl MenuParse_visiblityBits(menuDef_t *menu, int handle)
{
    int v3; // edx
    int v4; // edx
    bool setNextBitAsHideBit; // [esp+23h] [ebp-41Dh]
    const char *string; // [esp+24h] [ebp-41Ch] BYREF
    pc_token_s token; // [esp+28h] [ebp-418h] BYREF
    int flags; // [esp+43Ch] [ebp-4h]

    if ( !PC_String_Parse(handle, &string) )
        return 0;
    if ( I_stricmp(string, "bits") )
    {
        PC_SourceError(handle, "Expected 'bits( ' after visibilityBits.\n");
        return 0;
    }
    else if ( PC_ReadTokenHandle(handle, &token) )
    {
        if ( I_strcmp(token.string, "(") )
        {
            PC_SourceError(handle, "Expected '(' after 'bits' .\n");
            return 0;
        }
        else
        {
            menu->showBits = 0;
            menu->hideBits = -1;
            setNextBitAsHideBit = 0;
            while ( 1 )
            {
                if ( !PC_ReadTokenHandle(handle, &token) )
                    goto LABEL_39;
                if ( token.type != 3 && I_strcmp(token.string, "!") && I_strcmp(token.string, ")") )
                    break;
                if ( token.type == 3 )
                {
                    if ( setNextBitAsHideBit )
                    {
                        if ( (menu->showBits & (1LL << SLOBYTE(token.intvalue))) != 0 )
                        {
LABEL_25:
                            PC_SourceError(handle, "Cannot have bit %d set and reset at the same time.\n", token.intvalue);
                            return 0;
                        }
                        v4 = HIDWORD(menu->hideBits) & ~((unsigned __int64)(1LL << SLOBYTE(token.intvalue)) >> 32);
                        LODWORD(menu->hideBits) &= ~(unsigned int)(1LL << SLOBYTE(token.intvalue));
                        HIDWORD(menu->hideBits) = v4;
                        setNextBitAsHideBit = 0;
                    }
                    else
                    {
                        if ( (menu->hideBits & (1LL << SLOBYTE(token.intvalue))) == 0 )
                            goto LABEL_25;
                        v3 = HIDWORD(menu->showBits) | ((unsigned __int64)(1LL << SLOBYTE(token.intvalue)) >> 32);
                        LODWORD(menu->showBits) |= 1LL << SLOBYTE(token.intvalue);
                        HIDWORD(menu->showBits) = v3;
                    }
                }
                else if ( I_strcmp(token.string, "!") )
                {
                    if ( !I_strcmp(token.string, ")") )
                    {
                        if ( setNextBitAsHideBit )
                        {
                            PC_SourceError(handle, "Expected an integer after '!' but found ')' .\n");
                            return 0;
                        }
                        if ( !menu->showBits && (HIDWORD(menu->hideBits) & menu->hideBits) == -1 )
                        {
                            PC_SourceError(handle, "No bits specified.\n");
                            return 0;
                        }
LABEL_39:
                        flags = Window_GetDynamicFlags(0, &menu->window);
                        Window_SetDynamicFlags(0, &menu->window, flags | 4);
                        return 1;
                    }
                }
                else
                {
                    if ( setNextBitAsHideBit )
                    {
                        PC_SourceError(handle, "Expected an integer after '!' but found another '!' .\n");
                        return 0;
                    }
                    setNextBitAsHideBit = 1;
                }
            }
            if ( setNextBitAsHideBit )
            {
                PC_SourceError(handle, "Expected an integer after '!' but found %s\n", token.string);
                return 0;
            }
            else if ( menu->showBits || (HIDWORD(menu->hideBits) & menu->hideBits) != -1 )
            {
                PC_SourceError(handle, "Expected an integer or a closing ')' but found %s\n", token.string);
                return 0;
            }
            else
            {
                PC_SourceError(handle, "Expected an integer but found %s\n", token.string);
                return 0;
            }
        }
    }
    else
    {
        return 0;
    }
}

int __cdecl MenuParse_onFocus(menuDef_t *menu, int handle)
{
    GenericEventHandler *eventHandler; // [esp+0h] [ebp-4h]

    eventHandler = (GenericEventHandler *)UI_Alloc(0xCu, 4);
    eventHandler->name = String_Alloc("onFocus");
    eventHandler->next = menu->onEvent;
    menu->onEvent = eventHandler;
    return PC_EventScript_Parse(handle, &eventHandler->eventScript);
}


int __cdecl PC_EventScript_Parse(int handle, GenericEventScript **baseScript)
{
    const char *v3; // eax
    unsigned __int8 v4[5120]; // [esp+14h] [ebp-2C30h] BYREF
    ExpressionStatement statement; // [esp+1414h] [ebp-1830h] BYREF
    char dst[5127]; // [esp+1424h] [ebp-1820h] BYREF
    bool v7; // [esp+282Bh] [ebp-419h]
    pc_token_s pc_token; // [esp+282Ch] [ebp-418h] BYREF

    memset(dst, 0, 0x1400u);
    memset(v4, 0, sizeof(v4));
    if ( !PC_ReadTokenHandle(handle, &pc_token) )
        return 0;
    if ( I_stricmp(pc_token.string, "{") )
        return 0;
    PC_ResetConditionStack();
    v7 = 1;
    g_nestingCounts.bracketNestingLevel = 0;
    g_nestingCounts.ifNestingLevel = 0;
    g_constructUniqueID = 0;
    g_blockUniqueID = 0;
    while ( 1 )
    {
        while ( 1 )
        {
            if ( !PC_ReadTokenHandle(handle, &pc_token) )
                return 0;
            if ( &pc_token != (pc_token_s *)-16 && pc_token.string[0] == 125 )
                break;
            if ( !v7 || I_stricmp(pc_token.string, "if") )
            {
                if ( !v7 || I_stricmp(pc_token.string, "elseif") )
                {
                    if ( !v7 || I_stricmp(pc_token.string, "else") )
                    {
                        if ( pc_token.type <= 0 )
                            goto LABEL_72;
                        if ( pc_token.type <= 2 )
                        {
                            v3 = va("\"%s\" ", pc_token.string);
                            I_strncat(dst, 5120, v3);
                            goto LABEL_75;
                        }
                        if ( pc_token.type != 6 )
                        {
LABEL_72:
                            I_strncat(dst, 5120, pc_token.string);
                            if ( pc_token.type != 5 || pc_token.subtype != 30 )
                                I_strncat(dst, 5120, " ");
                        }
LABEL_75:
                        if ( (unsigned int)(&dst[strlen(dst) + 1] - &dst[1] + 1) >= 0x1400 )
                        {
                            Com_PrintError(16, "action block too long that starts with: %s\n", dst);
                            return 0;
                        }
                        v7 = !I_stricmp(pc_token.string, "}") || !I_stricmp(pc_token.string, ";");
                    }
                    else
                    {
                        if ( g_nestingCounts.bracketNestingLevel == g_nestingCounts.ifNestingLevel )
                        {
                            Com_PrintError(16, "Misplaced 'else'.\n");
                            return 0;
                        }
                        if ( !PC_ReadTokenHandle(handle, &pc_token) )
                            return 0;
                        if ( &pc_token == (pc_token_s *)-16 || pc_token.string[0] != 123 )
                        {
                            Com_PrintError(16, "Expected '{' after 'else'.\n");
                            return 0;
                        }
                        ++g_blockUniqueID;
                        PC_CndStackModifyTop();
                        PC_CndStackPush(
                            handle,
                            g_conditionStack.members[g_conditionStack.size - 1].condition,
                            0,
                            0,
                            g_blockUniqueID,
                            g_conditionStack.members[g_conditionStack.size - 1].constructID,
                            3);
                        ++g_nestingCounts.bracketNestingLevel;
                    }
                }
                else
                {
                    if ( g_nestingCounts.bracketNestingLevel == g_nestingCounts.ifNestingLevel )
                    {
                        Com_PrintError(16, "Misplaced 'elseif'.\n");
                        return 0;
                    }
                    if ( !Expression_Read(handle, &statement) )
                        goto LABEL_36;
                    if ( !PC_ReadTokenHandle(handle, &pc_token) )
                        return 0;
                    if ( &pc_token == (pc_token_s *)-16 || pc_token.string[0] != 123 )
                    {
                        Com_PrintError(16, "Expected '{' after 'elseif'.\n");
                        return 0;
                    }
                    PC_CndStackModifyTop();
                    if ( !PC_CndStackPush(
                                    handle,
                                    statement,
                                    1,
                                    0,
                                    ++g_blockUniqueID,
                                    g_conditionStack.members[g_conditionStack.size - 1].constructID,
                                    2) )
                    {
LABEL_43:
                        Com_PrintError(16, "Could not push onto condition stack.\n");
                        return 0;
                    }
                    ++g_nestingCounts.bracketNestingLevel;
                }
            }
            else
            {
                if ( !PC_CndStackAddConditionsToScript(baseScript, dst) )
                    goto LABEL_34;
                if ( !Expression_Read(handle, &statement) )
                {
LABEL_36:
                    Com_PrintError(16, "Could not read condition expression.\n");
                    return 0;
                }
                if ( !PC_ReadTokenHandle(handle, &pc_token) )
                    return 0;
                if ( &pc_token == (pc_token_s *)-16 || pc_token.string[0] != 123 )
                {
                    Com_PrintError(16, "Expected '{' after 'if'.\n");
                    return 0;
                }
                if ( !PC_CndStackPush(handle, statement, 1, 1, ++g_blockUniqueID, ++g_constructUniqueID, 1) )
                    goto LABEL_43;
                ++g_nestingCounts.ifNestingLevel;
                ++g_nestingCounts.bracketNestingLevel;
            }
        }
        if ( !g_conditionStack.size )
            break;
        if ( !PC_CndStackAddConditionsToScript(baseScript, dst) )
            goto LABEL_34;
        if ( !PC_ReadTokenHandle(handle, &pc_token) )
            return 0;
        if ( I_stricmp(pc_token.string, "else") && I_stricmp(pc_token.string, "elseif") )
        {
            if ( !PC_CndStackPopLevel() )
            {
                Com_PrintError(16, "Condition stack encountered an error while popping nested if levels.\n");
                return 0;
            }
            --g_nestingCounts.ifNestingLevel;
        }
        if ( (!I_stricmp(pc_token.string, "else") || !I_stricmp(pc_token.string, "elseif"))
            && !g_conditionStack.members[g_conditionStack.size - 1].fireOnTrue )
        {
            Com_PrintError(16, "Misplaced '%s'.\n", pc_token.string);
            return 0;
        }
        PC_UnreadLastTokenHandle(handle);
        --g_nestingCounts.bracketNestingLevel;
    }
    if ( !PC_CndStackAddConditionsToScript(baseScript, dst) )
    {
LABEL_34:
        Com_PrintError(16, "Could not add conditions to script.\n");
        return 0;
    }
    if ( g_nestingCounts.bracketNestingLevel == g_nestingCounts.ifNestingLevel
        && !g_nestingCounts.bracketNestingLevel
        && !g_nestingCounts.ifNestingLevel )
    {
        return 1;
    }
    Com_PrintError(16, "Malformed condition structure. Are all brackets well placed?\n");
    return 0;
}

int __cdecl MenuParse_leaveFocus(menuDef_t *menu, int handle)
{
    GenericEventHandler *eventHandler; // [esp+0h] [ebp-4h]

    eventHandler = (GenericEventHandler *)UI_Alloc(0xCu, 4);
    eventHandler->name = String_Alloc("leaveFocus");
    eventHandler->next = menu->onEvent;
    menu->onEvent = eventHandler;
    return PC_EventScript_Parse(handle, &eventHandler->eventScript);
}

int __cdecl MenuParse_onOpen(menuDef_t *menu, int handle)
{
    GenericEventHandler *eventHandler; // [esp+0h] [ebp-4h]

    eventHandler = (GenericEventHandler *)UI_Alloc(0xCu, 4);
    eventHandler->name = String_Alloc("onOpen");
    eventHandler->next = menu->onEvent;
    menu->onEvent = eventHandler;
    return PC_EventScript_Parse(handle, &eventHandler->eventScript);
}

int __cdecl MenuParse_onClose(menuDef_t *menu, int handle)
{
    GenericEventHandler *eventHandler; // [esp+0h] [ebp-4h]

    eventHandler = (GenericEventHandler *)UI_Alloc(0xCu, 4);
    eventHandler->name = String_Alloc("onClose");
    eventHandler->next = menu->onEvent;
    menu->onEvent = eventHandler;
    return PC_EventScript_Parse(handle, &eventHandler->eventScript);
}

int __cdecl MenuParse_onESC(menuDef_t *menu, int handle)
{
    GenericEventHandler *eventHandler; // [esp+0h] [ebp-4h]

    eventHandler = (GenericEventHandler *)UI_Alloc(0xCu, 4);
    eventHandler->name = String_Alloc("onESC");
    eventHandler->next = menu->onEvent;
    menu->onEvent = eventHandler;
    return PC_EventScript_Parse(handle, &eventHandler->eventScript);
}

int __cdecl MenuParse_execExp(menuDef_t *menu, int handle)
{
    const char *expressionType; // [esp+0h] [ebp-41Ch] BYREF
    pc_token_s token; // [esp+4h] [ebp-418h] BYREF
    int flags; // [esp+418h] [ebp-4h]

    if ( !PC_String_Parse(handle, &expressionType) )
        return 0;
    if ( I_stricmp(expressionType, "visible") )
    {
        if ( !I_stricmp(expressionType, "rect") )
        {
            if ( !PC_ReadTokenHandle(handle, &token) )
            {
                Com_PrintError(16, "ERROR: line ended early after \"exp rect\"\n");
                return 0;
            }
            if ( I_stricmp(token.string, "X") )
            {
                if ( I_stricmp(token.string, "Y") )
                {
                    Com_PrintError(16, "ERROR: Expected 'X' or 'Y' after \"exp rect\" but found \"%s\"\n", token.string);
                    return 0;
                }
                if ( Expression_Read(handle, &menu->rectYExp) )
                    return 1;
            }
            else if ( Expression_Read(handle, &menu->rectXExp) )
            {
                return 1;
            }
        }
    }
    else
    {
        if ( !PC_ReadTokenHandle(handle, &token) )
        {
            Com_PrintError(16, "ERROR: line ended early after \"visible\"\n");
            return 0;
        }
        if ( I_stricmp(token.string, "when") )
        {
            Com_PrintError(16, "ERROR: Expected 'when' after \"visible\" but found \"%s\"\n", token.string);
            return 0;
        }
        flags = Window_GetDynamicFlags(0, &menu->window);
        Window_SetDynamicFlags(0, &menu->window, flags | 4);
        if ( Expression_Read(handle, &menu->visibleExp) )
            return 1;
    }
    return 0;
}

int    MenuParse_border(menuDef_t *menu, int handle)
{
    return PC_Byte_Parse(handle, &menu->window.border);
}

int    MenuParse_borderSize(menuDef_t *menu, int handle)
{
    return PC_Float_Parse(handle, &menu->window.borderSize);
}

int    MenuParse_backcolor(menuDef_t *menu, int handle)
{
    return PC_Color_Parse(handle, (float (*)[4])menu->window.backColor);
}

int    PC_Color_Parse(int handle, float (*c)[4])
{
    float f; // [esp+0h] [ebp-8h] BYREF
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 4; ++i )
    {
        if ( !PC_Float_Parse(handle, &f) )
            return 0;
        (*c)[i] = f;
    }
    return 1;
}

int    MenuParse_forecolor(menuDef_t *menu, int handle)
{
    int flags; // [esp+0h] [ebp-4h]

    if ( !PC_Color_Parse(handle, (float (*)[4])menu->window.foreColor) )
        return 0;
    flags = Window_GetDynamicFlags(0, &menu->window);
    Window_SetDynamicFlags(0, &menu->window, flags | 0x10000);
    return 1;
}

int    MenuParse_bordercolor(menuDef_t *menu, int handle)
{
    return PC_Color_Parse(handle, (float (*)[4])menu->window.borderColor);
}

int    MenuParse_focuscolor(menuDef_t *menu, int handle)
{
    return PC_Color_Parse(handle, (float (*)[4])menu->focusColor);
}

int    MenuParse_disablecolor(menuDef_t *menu, int handle)
{
    return PC_Color_Parse(handle, (float (*)[4])menu->disableColor);
}

int    MenuParse_outlinecolor(menuDef_t *menu, int handle)
{
    return PC_Color_Parse(handle, (float (*)[4])menu->window.outlineColor);
}

int __cdecl MenuParse_background(menuDef_t *menu, int handle)
{
    char name[64]; // [esp+0h] [ebp-48h] BYREF
    const char *buff; // [esp+44h] [ebp-4h] BYREF

    if ( !PC_String_Parse(handle, &buff) )
        return 0;
    I_strncpyz(name, buff, 64);
    I_strlwr(name);
    menu->window.background = Material_RegisterHandle(name, menu->imageTrack);
    return 1;
}

int    ItemParse_ownerdrawFlag(itemDef_s *item, int handle)
{
    int i; // [esp+0h] [ebp-4h] BYREF

    if ( !PC_Int_Parse(handle, &i) )
        return 0;
    item->window.ownerDrawFlags |= i;
    return 1;
}

int    MenuParse_ownerdraw(menuDef_t *menu, int handle)
{
    return PC_Int_Parse(handle, &menu->window.ownerDraw);
}

int __cdecl MenuParse_popup(menuDef_t *menu, int handle)
{
    Window_SetStaticFlags(&menu->window, menu->window.staticFlags | 0x1000000);
    return 1;
}

int __cdecl MenuParse_outOfBounds(menuDef_t *menu, int handle)
{
    Window_SetStaticFlags(&menu->window, menu->window.staticFlags | 0x2000000);
    return 1;
}

int __cdecl MenuParse_soundLoop(menuDef_t *menu, int handle)
{
    pc_token_s token; // [esp+0h] [ebp-418h] BYREF

    if ( !PC_ReadTokenHandle(handle, &token) )
        return 0;
    if ( token.string[0] )
        menu->soundName = String_Alloc(token.string);
    return 1;
}

int    MenuParse_fadeClamp(menuDef_t *menu, int handle)
{
    return PC_Float_Parse(handle, &menu->fadeClamp);
}

int    MenuParse_fadeAmount(menuDef_t *menu, int handle)
{
    return PC_Float_Parse(handle, &menu->fadeAmount);
}

int    MenuParse_fadeInAmount(menuDef_t *menu, int handle)
{
    return PC_Float_Parse(handle, &menu->fadeInAmount);
}

int    MenuParse_fadeCycle(menuDef_t *menu, int handle)
{
    return PC_Int_Parse(handle, &menu->fadeCycle);
}

int __cdecl MenuParse_itemDef(menuDef_t *menu, int handle)
{
    itemDef_s *item; // [esp+0h] [ebp-4h]

    if ( menu->itemCount >= 512 )
    {
        Com_Error(ERR_DROP, "too many itemDefs for menu %s. MAX is %i", menu->window.name, 512);
    }
    else
    {
        item = (itemDef_s *)UI_Alloc(0x110u, 4);
        Item_Init(item, menu->imageTrack);
        Item_ValidateTypeData(item, handle);
        if ( !Item_Parse(handle, item) )
        {
            Menu_FreeItemMemory(item);
            return 0;
        }
        Item_PostParse(item);
        Item_InitControls(item);
        item->parent = menu;
        menu->items[menu->itemCount++] = item;
    }
    return 1;
}

void __cdecl Item_Init(itemDef_s *item, int imageTrack)
{
    memset(item, 0, sizeof(itemDef_s));
    item->imageTrack = imageTrack;
    item->ui3dWindowId = -1;
    item->showBits = 0;
    item->hideBits = -1;
    if ( item->animInfo )
        item->animInfo->animStates = g_load_0.animStates;
    Window_Init(&item->window);
}

void __cdecl Window_Init(windowDef_t *w)
{
    memset(w, 0, sizeof(windowDef_t));
    w->borderSize = 1.0f;
    w->foreColor[3] = 1.0f;
    w->foreColor[2] = 1.0f;
    w->foreColor[1] = 1.0f;
    w->foreColor[0] = 1.0f;
}

int __cdecl Item_Parse(int handle, itemDef_s *item)
{
    const KeywordHashEntry<itemDef_s,1024,6> *key; // [esp+0h] [ebp-41Ch]
    pc_token_s token; // [esp+4h] [ebp-418h] BYREF

    if ( !PC_ReadTokenHandle(handle, &token) )
        return 0;
    if ( token.string[0] != 123 )
        return 0;
    do
    {
        while ( 1 )
        {
            do
            {
                if ( !PC_ReadTokenHandle(handle, &token) )
                {
                    PC_SourceError(handle, "end of file inside menu item\n");
                    return 0;
                }
                if ( token.string[0] == 125 )
                    return 1;
            }
            while ( token.string[0] == 59 );
            //key = KeywordHash_Find_itemDef_s_1024_6_(itemParseKeywordHash, token.string);

            key = ItemKeyword::Find(itemParseKeywordHash, token.string);
            if ( key )
                break;
            PC_SourceError(handle, "unknown menu item keyword %s", token.string);
        }
    }
    while ( key->func(item, handle) );
    PC_SourceError(handle, "couldn't parse menu item keyword %s", token.string);
    return 0;
}

void __cdecl Item_PostParse(itemDef_s *item)
{
    unsigned int size; // [esp+0h] [ebp-4h]

    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared_obj.cpp", 1274, 0, "%s", "item") )
        __debugbreak();
    if ( item->animInfo )
    {
        size = 4 * item->animInfo->animStateCount;
        item->animInfo->animStates = (animParamsDef_t **)UI_Alloc(size, 4);
        memcpy((unsigned __int8 *)item->animInfo->animStates, (unsigned __int8 *)g_load_0.animStates, size);
    }
}

void __cdecl Item_InitControls(itemDef_s *item)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    if ( item && item->type == 4 )
    {
        listPtr = Item_GetListBoxDef(item);
        listPtr->cursorPos[0] = 0;
        if ( listPtr )
        {
            listPtr->startPos[0] = 0;
            listPtr->endPos[0] = 0;
        }
    }
}

void __cdecl Item_ValidateTypeData(itemDef_s *item, int handle)
{
    unsigned int *textDef; // ecx
    editFieldDef_s *editDef; // [esp+0h] [ebp-4h]

    if ( item->typeData.textDef )
    {
        if ( item->dataType != item->type )
            PC_SourceError(
                handle,
                "Attempting to change type from %d to %d.\nMove the type definition higher up in the itemDef.\n",
                item->dataType,
                item->type);
    }
    else
    {
        item->dataType = item->type;
        switch ( item->type )
        {
            case 5:
            case 7:
            case 0xD:
            case 0xE:
            case 9:
            case 0xC:
            case 0x10:
            case 0x1E:
            case 8:
            case 0x16:
                item->typeData.textDef = (textDef_s *)UI_Alloc(0x44u, 4);
                item->typeData.textDef->textTypeData.focusItemDef = (focusItemDef_s *)UI_Alloc(0x18u, 4);
                item->typeData.textDef->textTypeData.focusItemDef->focusTypeData.listBox = (listBoxDef_s *)UI_Alloc(0x24u, 4);
                item->typeData.textDef->textscale = 0.55f;
                if ( item->type == 5
                    || item->type == 13
                    || item->type == 7
                    || item->type == 14
                    || item->type == 30
                    || item->type == 22 )
                {
                    editDef = Item_GetEditFieldDef(item);
                    if ( !editDef
                        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared_obj.cpp", 1807, 0, "%s", "editDef") )
                    {
                        __debugbreak();
                    }
                    if ( !editDef->maxPaintChars )
                        editDef->maxPaintChars = 256;
                }
                break;
            case 0xB:
                item->typeData.textDef = (textDef_s *)UI_Alloc(0x44u, 4);
                item->typeData.textDef->textTypeData.focusItemDef = (focusItemDef_s *)UI_Alloc(0x18u, 4);
                item->typeData.textDef->textTypeData.focusItemDef->focusTypeData.listBox = (listBoxDef_s *)UI_Alloc(4u, 4);
                item->typeData.textDef->textscale = 0.55f;
                break;
            case 0xA:
                item->typeData.textDef = (textDef_s *)UI_Alloc(0x44u, 4);
                item->typeData.textDef->textTypeData.focusItemDef = (focusItemDef_s *)UI_Alloc(0x18u, 4);
                item->typeData.textDef->textTypeData.focusItemDef->focusTypeData.listBox = (listBoxDef_s *)UI_Alloc(0x18Cu, 4);
                item->typeData.textDef->textscale = 0.55f;
                break;
            case 2:
            case 6:
                item->typeData.textDef = (textDef_s *)UI_Alloc(0x10u, 4);
                textDef = (unsigned int*)item->typeData.textDef;
                *textDef = 0;
                textDef[1] = 0;
                textDef[2] = 0;
                textDef[3] = 0;
                break;
            case 1:
            case 0x12:
                item->typeData.textDef = (textDef_s *)UI_Alloc(0x44u, 4);
                item->typeData.textDef->textscale = 0.55f;
                break;
            case 3:
            case 0x14:
                item->typeData.textDef = (textDef_s *)UI_Alloc(0x44u, 4);
                item->typeData.textDef->textTypeData.focusItemDef = (focusItemDef_s *)UI_Alloc(0x18u, 4);
                item->typeData.textDef->textscale = 0.55f;
                break;
            case 0x15:
            case 0x13:
                item->typeData.textDef = (textDef_s *)UI_Alloc(0x18u, 4);
                break;
            case 4:
                item->typeData.textDef = (textDef_s *)UI_Alloc(0x44u, 4);
                item->typeData.textDef->textTypeData.focusItemDef = (focusItemDef_s *)UI_Alloc(0x18u, 4);
                item->typeData.textDef->textTypeData.focusItemDef->focusTypeData.listBox = (listBoxDef_s *)UI_Alloc(0x29Cu, 4);
                item->typeData.textDef->textscale = 0.55f;
                break;
            case 0xF:
                item->typeData.textDef = (textDef_s *)UI_Alloc(0x44u, 4);
                item->typeData.textDef->textTypeData.focusItemDef = (focusItemDef_s *)UI_Alloc(8u, 4);
                item->typeData.textDef->textscale = 0.55f;
                break;
        }
    }
}

int __cdecl MenuParse_execKey(menuDef_t *menu, int handle)
{
    __int16 keyindex; // [esp+0h] [ebp-Ch]
    char keyname; // [esp+7h] [ebp-5h] BYREF
    ItemKeyHandler *handler; // [esp+8h] [ebp-4h]

    if ( !PC_Char_Parse(handle, &keyname) )
        return 0;
    keyindex = (unsigned __int8)keyname;
    handler = (ItemKeyHandler *)UI_Alloc(0xCu, 4);
    handler->key = keyindex;
    if ( !PC_EventScript_Parse(handle, &handler->keyScript) )
        return 0;
    handler->next = menu->onKey;
    menu->onKey = handler;
    return 1;
}

int __cdecl PC_Char_Parse(int handle, char *out)
{
    pc_token_s token; // [esp+0h] [ebp-418h] BYREF

    if ( !PC_ReadTokenHandle(handle, &token) )
        return 0;
    *out = token.string[0];
    return 1;
}

int    MenuParse_execKeyInt(menuDef_t *menu, int handle)
{
    int keyname; // [esp+0h] [ebp-8h] BYREF
    ItemKeyHandler *handler; // [esp+4h] [ebp-4h]

    if ( !PC_Int_Parse(handle, &keyname) )
        return 0;
    handler = (ItemKeyHandler *)UI_Alloc(0xCu, 4);
    handler->key = keyname;
    if ( !PC_EventScript_Parse(handle, &handler->keyScript) )
        return 0;
    handler->next = menu->onKey;
    menu->onKey = handler;
    return 1;
}

int    MenuParse_blurWorld(menuDef_t *menu, int handle)
{
    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared_obj.cpp", 1946, 0, "%s", "menu") )
        __debugbreak();
    if ( !PC_Float_Parse(handle, &menu->blurRadius) )
        return 0;
    if ( menu->blurRadius >= 0.0 )
        return 1;
    PC_SourceError(handle, "blur must be >= 0; %g is invalid", menu->blurRadius);
    return 0;
}

int __cdecl MenuParse_legacySplitScreenScale(menuDef_t *menu, int handle)
{
    return SetItemStaticFlag(menu, handle, (int)&objBuf[1758][2]);
}

int __cdecl SetItemStaticFlag(menuDef_t *menu, int handle, int flag)
{
    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared_obj.cpp", 1962, 0, "%s", "menu") )
        __debugbreak();
    Window_SetStaticFlags(&menu->window, flag | menu->window.staticFlags);
    return 1;
}

int __cdecl MenuParse_hiddenDuringScope(menuDef_t *menu, int handle)
{
    return SetItemStaticFlag(menu, handle, 0x20000000);
}

int __cdecl MenuParse_hiddenDuringFlashbang(menuDef_t *menu, int handle)
{
    return SetItemStaticFlag(menu, handle, 0x10000000);
}

int __cdecl MenuParse_hiddenDuringUI(menuDef_t *menu, int handle)
{
    return SetItemStaticFlag(menu, handle, 0x40000000);
}

int __cdecl MenuParse_allowedBinding(menuDef_t *menu, int handle)
{
    if ( menu->allowedBinding )
        PC_SourceError(handle, "Only one 'allowedBinding' is supported");
    return PC_String_Parse(handle, &menu->allowedBinding);
}

int __cdecl MenuParse_allowSignIn(menuDef_t *menu, int handle)
{
    __debugbreak();
#if 0 // KISAKTODO:
    return SetItemStaticFlag(menu, handle, 0x8000000);
#else
    return 0;
#endif
}

int    MenuParse_ui3dWindowId(menuDef_t *menu, int handle)
{
    return PC_Int_Parse(handle, &menu->ui3dWindowId);
}

int    MenuParse_priority(menuDef_t *menu, int handle)
{
    int v; // [esp+0h] [ebp-4h] BYREF

    if ( !PC_Int_Parse(handle, &v) )
        return 0;
    menu->priority = v;
    return 1;
}

int    MenuParse_openSlideSpeed(menuDef_t *menu, int handle)
{
    int v; // [esp+0h] [ebp-4h] BYREF

    if ( !PC_Int_Parse(handle, &v) )
        return 0;
    if ( v > 0 )
    {
        menu->openSlideSpeed = v;
        return 1;
    }
    else
    {
        PC_SourceError(handle, "openSlideSpeed cannot be negative or zero; %i is invalid", v);
        return 0;
    }
}

int    MenuParse_closeSlideSpeed(menuDef_t *menu, int handle)
{
    int v; // [esp+0h] [ebp-4h] BYREF

    if ( !PC_Int_Parse(handle, &v) )
        return 0;
    if ( v > 0 )
    {
        menu->closeSlideSpeed = v;
        return 1;
    }
    else
    {
        PC_SourceError(handle, "closeSlideSpeed cannot be negative or zero; %i is invalid", v);
        return 0;
    }
}

int    MenuParse_openSlideDirection(menuDef_t *menu, int handle)
{
    int v; // [esp+0h] [ebp-4h] BYREF

    if ( !PC_Int_Parse(handle, &v) )
        return 0;
    if ( (unsigned int)v < 4 )
    {
        menu->openSlideDirection = v;
        return 1;
    }
    else
    {
        PC_SourceError(handle, "openSlideDirection must be between 0 and 3. %i is invalid", v);
        return 0;
    }
}

int    MenuParse_closeSlideDirection(menuDef_t *menu, int handle)
{
    int v; // [esp+0h] [ebp-4h] BYREF

    if ( !PC_Int_Parse(handle, &v) )
        return 0;
    if ( (unsigned int)v < 4 )
    {
        menu->closeSlideDirection = v;
        return 1;
    }
    else
    {
        PC_SourceError(handle, "closeSlideDirection must be between 0 and 3. %i is invalid", v);
        return 0;
    }
}

int    MenuParse_openFadingTime(menuDef_t *menu, int handle)
{
    int v; // [esp+0h] [ebp-4h] BYREF

    if ( !PC_Int_Parse(handle, &v) )
        return 0;
    if ( v > 0 )
    {
        menu->openFadingTime = v;
        return 1;
    }
    else
    {
        PC_SourceError(handle, "openFadingTime cannot be negative or zero; %i is invalid", v);
        return 0;
    }
}

int    MenuParse_closeFadingTime(menuDef_t *menu, int handle)
{
    int v; // [esp+0h] [ebp-4h] BYREF

    if ( !PC_Int_Parse(handle, &v) )
        return 0;
    if ( v > 0 )
    {
        menu->closeFadingTime = v;
        return 1;
    }
    else
    {
        PC_SourceError(handle, "closeFadingTime cannot be negative or zero; %i is invalid", v);
        return 0;
    }
}

int    MenuParse_control(menuDef_t *menu, int handle)
{
    return PC_Int_Parse(handle, &menu->control);
}

int    ItemParse_frame(itemDef_s *item, int handle)
{
    return WindowParse_frame(&item->window, handle);
}

int    WindowParse_frame(windowDef_t *window, int handle)
{
    int bits; // [esp+0h] [ebp-4h] BYREF

    window->frameTexSize = 0.5f;
    window->frameSize = 0.0f;
    if ( !PC_Float_Parse(handle, &window->frameSize) )
        return 0;
    if ( !PC_Float_Parse(handle, &window->frameTexSize) )
        return 0;
    while ( PC_Flag_ParseOptional(handle, &bits) )
    {
        if ( (unsigned int)bits >= 0x100 )
        {
            PC_SourceError(handle, "frame side value our of range '%d'\n", bits);
            return 0;
        }
        window->frameSides |= bits;
    }
    if ( !window->frameSides )
        window->frameSides = -1;
    return 1;
}

void __cdecl Menu_SetupKeywordHash()
{
    //unsigned int i; // [esp+0h] [ebp-4h]
    //
    //KeywordHash_Validate_menuDef_t_1024_128_(menuParseKeywords, 50);
    //memset(menuParseKeywordHash, 0, sizeof(menuParseKeywordHash));
    //for ( i = 0; i < 0x32; ++i )
    //    KeywordHash_Add_menuDef_t_1024_128_(
    //        menuParseKeywordHash,
    //        (const KeywordHashEntry<menuDef_t,1024,128> *)(8 * i + 13621960));


    constexpr unsigned int COUNT = 50;

    MenuKeyword::Validate(menuParseKeywords, COUNT);

    memset(menuParseKeywordHash, 0, sizeof(menuParseKeywordHash));

    for (unsigned int i = 0; i < COUNT; ++i)
    {
        MenuKeyword::Add(
            menuParseKeywordHash,
            &menuParseKeywords[i]);
    }
}

int __cdecl ItemParse_name(itemDef_s *item, int handle)
{
    return PC_String_Parse(handle, &item->window.name);
}

int __cdecl ItemParse_focusSound(itemDef_s *item, int handle)
{
    GenericEventHandler *eventHandler; // [esp+0h] [ebp-4h]

    eventHandler = (GenericEventHandler *)UI_Alloc(0xCu, 4);
    eventHandler->name = String_Alloc("focusSound");
    eventHandler->next = item->onEvent;
    item->onEvent = eventHandler;
    return PC_EventScript_Parse(handle, &eventHandler->eventScript);
}

int __cdecl ItemParse_text(itemDef_s *item, int handle)
{
    textDef_s *textDefPtr; // [esp+0h] [ebp-4h]

    textDefPtr = Item_GetTextDef(item);
    return PC_String_Parse(handle, &textDefPtr->text);
}

int __cdecl ItemParse_textfile(itemDef_s *item, int handle)
{
    textDef_s *textDefPtr; // [esp+0h] [ebp-420h]
    char *newtext; // [esp+4h] [ebp-41Ch]
    pc_token_s token; // [esp+8h] [ebp-418h] BYREF

    textDefPtr = Item_GetTextDef(item);
    if ( !PC_ReadTokenHandle(handle, &token) )
        return 0;
    newtext = UI_FileText(token.string);
    textDefPtr->text = String_Alloc(newtext);
    return 1;
}

char menuBuf1[4096];
char *__cdecl UI_FileText(char *fileName)
{
    unsigned int len; // [esp+4h] [ebp-8h]
    int f; // [esp+8h] [ebp-4h] BYREF

    len = FS_FOpenFileByMode(fileName, &f, FS_READ);
    if ( !f )
        return 0;
    if ( len < 0x1000 )
    {
        FS_Read((unsigned __int8 *)menuBuf1, len, f);
        menuBuf1[len] = 0;
        FS_FCloseFile(f);
        return menuBuf1;
    }
    else
    {
        FS_FCloseFile(f);
        Com_PrintError(16, "Menu file %s is larger than the %i byte buffer used to parse menu files\n", fileName, 4096);
        return 0;
    }
}

int __cdecl ItemParse_textsavegame(itemDef_s *item, int handle)
{
    textDef_s *TextDef; // eax

    TextDef = Item_GetTextDef(item);
    TextDef->text = "savegameinfo";
    TextDef->itemFlags |= 1u;
    return 1;
}

int __cdecl ItemParse_textcinematicsubtitle(itemDef_s *item, int handle)
{
    textDef_s *TextDef; // eax

    TextDef = Item_GetTextDef(item);
    TextDef->text = "cinematicsubtitle";
    TextDef->itemFlags |= 2u;
    return 1;
}

int __cdecl ItemParse_group(itemDef_s *item, int handle)
{
    return PC_String_Parse(handle, &item->window.group);
}

int    ItemParse_rect(itemDef_s *item, int handle)
{
    return PC_Rect_Parse(handle, &item->window.rectClient);
}

int    ItemParse_origin(itemDef_s *item, int handle)
{
    int x; // [esp+0h] [ebp-8h] BYREF
    int y; // [esp+4h] [ebp-4h] BYREF

    if ( !PC_Int_Parse(handle, &x) )
        return 0;
    if ( !PC_Int_Parse(handle, &y) )
        return 0;
    item->window.rectClient.x = (float)x + item->window.rectClient.x;
    item->window.rectClient.y = (float)y + item->window.rectClient.y;
    return 1;
}

int    ItemParse_style(itemDef_s *item, int handle)
{
    return PC_Byte_Parse(handle, &item->window.style);
}

int __cdecl ItemParse_decoration(itemDef_s *item, int handle)
{
    Window_SetStaticFlags(&item->window, item->window.staticFlags | 0x100000);
    return 1;
}

int __cdecl ItemParse_notselectable(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr )
        return 0;
    if ( item->type == 4 )
        listPtr->notselectable = 1;
    return 1;
}

int __cdecl ItemParse_noScrollBars(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr )
        return 0;
    if ( item->type == 4 )
        listPtr->noScrollBars = 1;
    return 1;
}

int __cdecl ItemParse_noBlinkingHighlight(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr )
        return 0;
    if ( item->type == 4 )
        listPtr->noBlinkingHighlight = 1;
    return 1;
}

int __cdecl ItemParse_usePaging(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr )
        return 0;
    if ( item->type == 4 )
        listPtr->usePaging = 1;
    return 1;
}

int __cdecl ItemParse_autowrapped(itemDef_s *item, int handle)
{
    Window_SetStaticFlags(&item->window, 0x800000 | item->window.staticFlags);
    return 1;
}

int __cdecl ItemParse_horizontalscroll(itemDef_s *item, int handle)
{
    Window_SetStaticFlags(&item->window, item->window.staticFlags | 0x200000);
    return 1;
}

int    ItemParse_type(itemDef_s *item, int handle)
{
    if ( !PC_Int_Parse(handle, &item->type) )
        return 0;
    Item_ValidateTypeData(item, handle);
    return 1;
}

int ItemParse_elementwidth(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    return listPtr && PC_Float_Parse(handle, &listPtr->elementWidth) != 0;
}

int ItemParse_elementheight(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    return listPtr && PC_Float_Parse(handle, &listPtr->elementHeight) != 0;
}

int ItemParse_special(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    listPtr = Item_GetListBoxDef(item);
    return PC_Float_Parse(handle, &listPtr->special) != 0;
}

int    ItemParse_elementtype(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]
    int ia; // [esp+4h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    if ( !item->typeData.textDef )
        return 0;
    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr )
        return 0;
    PC_Int_Parse(handle, &listPtr->columnInfo[0].elementStyle);
    for ( i = 1; i < 16; ++i )
        listPtr->columnInfo[i].elementStyle = listPtr->columnInfo[0].elementStyle;
    for ( ia = 1; ia < listPtr->numColumns; ++ia )
    {
        listPtr->columnInfo[ia].elementStyle = 0;
        PC_Int_Parse(handle, &listPtr->columnInfo[ia].elementStyle);
    }
    return 1;
}

int    ItemParse_columns(itemDef_s *item, int handle)
{
    int pos; // [esp+0h] [ebp-1Ch] BYREF
    int width; // [esp+4h] [ebp-18h] BYREF
    int align; // [esp+8h] [ebp-14h] BYREF
    int maxChars; // [esp+Ch] [ebp-10h] BYREF
    listBoxDef_s *listPtr; // [esp+10h] [ebp-Ch]
    int num; // [esp+14h] [ebp-8h] BYREF
    int i; // [esp+18h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    if ( !item->typeData.textDef )
        return 0;
    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr )
        return 0;
    if ( !PC_Int_Parse(handle, &num) )
        return 0;
    if ( num > 16 )
        num = 16;
    listPtr->numColumns = num;
    for ( i = 0; i < num; ++i )
    {
        if ( !PC_Int_Parse(handle, &pos) || !PC_Int_Parse(handle, &width) || !PC_Int_Parse(handle, &maxChars) )
            return 0;
        listPtr->columnInfo[i].rect.x = (float)pos;
        listPtr->columnInfo[i].rect.w = (float)width;
        listPtr->columnInfo[i].rect.h = listPtr->elementHeight;
        listPtr->columnInfo[i].maxChars = maxChars;
        if ( PC_Int_ParseLine(handle, &align) )
            listPtr->columnInfo[i].rect.horzAlign = align;
        else
            listPtr->columnInfo[i].rect.horzAlign = 0;
    }
    return 1;
}

int __cdecl PC_Int_ParseLine(int handle, int *i)
{
    pc_token_s token; // [esp+0h] [ebp-418h] BYREF
    int negative; // [esp+414h] [ebp-4h]

    if ( !PC_ReadLineHandle(handle, &token) )
        return 0;
    negative = 0;
    if ( token.string[0] == 45 )
    {
        if ( !PC_ReadLineHandle(handle, &token) )
            return 0;
        negative = 1;
    }
    if ( token.type == 3 )
    {
        *i = token.intvalue;
        if ( negative )
            *i = -*i;
        return 1;
    }
    else
    {
        PC_SourceError(handle, "expected integer but found %s\n", token.string);
        return 0;
    }
}

int    ItemParse_menuItemsDef(itemDef_s *item, int handle)
{
    unsigned __int8 *v4; // eax
    int j; // [esp+10h] [ebp-D8h]
    int i; // [esp+14h] [ebp-D4h]
    const char *nextKey; // [esp+18h] [ebp-D0h] BYREF
    listBoxDef_s *listPtr; // [esp+1Ch] [ebp-CCh]
    MenuCell tempCells[16]; // [esp+20h] [ebp-C8h] BYREF
    int cellCount; // [esp+E4h] [ebp-4h]

    cellCount = 0;
    Item_ValidateTypeData(item, handle);
    if ( item->typeData.textDef )
    {
        listPtr = Item_GetListBoxDef(item);
        if ( listPtr )
        {
            if ( PC_String_Parse(handle, &nextKey)
                && !I_stricmp(nextKey, "{")
                && PC_String_Parse(handle, &nextKey)
                && !I_stricmp(nextKey, "maxRows")
                && PC_Int_Parse(handle, &listPtr->maxRows) )
            {
                listPtr->rowCount = 0;
                listPtr->numColumns = 0;
                listPtr->rows = (MenuRow *)UI_Alloc(24 * listPtr->maxRows, 4);
                if ( !PC_String_Parse(handle, &nextKey) )
                    return 1;
                while ( !I_stricmp(nextKey, "menuColDef") && cellCount < 16 )
                {
                    if ( !ItemParse_menuColDef(listPtr, handle, tempCells, cellCount) )
                    {
                        Com_PrintError(13, "menuItemDef Error: Failed parsing menuColDef\n");
                        return 0;
                    }
                    if ( !PC_String_Parse(handle, &nextKey) )
                    {
                        Com_PrintError(13, "menuItemDef Error: Could not parse key.\n");
                        return 0;
                    }
                    ++cellCount;
                }
                for ( i = 0; i < listPtr->maxRows; ++i )
                {
                    listPtr->rows[i].cells = (MenuCell *)UI_Alloc(12 * cellCount, 4);
                    listPtr->rows[i].eventName = (char *)UI_Alloc(0x20u, 4);
                    listPtr->rows[i].onFocusEventName = (char *)UI_Alloc(0x20u, 4);
                    for ( j = 0; j < cellCount; ++j )
                    {
                        listPtr->rows[i].cells[j].maxChars = tempCells[j].maxChars;
                        listPtr->rows[i].cells[j].type = tempCells[j].type;
                        v4 = UI_Alloc(tempCells[j].maxChars, 4);
                        listPtr->rows[i].cells[j].stringValue = (char *)v4;
                    }
                }
                if ( I_stricmp(nextKey, "}") )
                {
                    Com_PrintError(13, "menuItemDef Error: Expected '}'\n");
                    return 0;
                }
                else
                {
                    return 1;
                }
            }
            else
            {
                Com_PrintError(13, "menuItemDef Error: Expected 'maxRows' <maxRows>\n");
                return 0;
            }
        }
        else
        {
            Com_PrintError(13, "menuItemDef Error: No listBoxDef found in itemDef.\n");
            return 0;
        }
    }
    else
    {
        Com_PrintError(13, "menuItemDef Error: Validate Type Data Failed.\n");
        return 0;
    }
}

int    ItemParse_menuColDef(
                listBoxDef_s *listPtr,
                int handle,
                MenuCell *cells,
                int cellCount)
{
    rectDef_s *p_rect; // edx
    int i; // [esp+0h] [ebp-28h]
    const char *nextKey; // [esp+4h] [ebp-24h] BYREF
    int colType; // [esp+8h] [ebp-20h] BYREF
    int maxChars; // [esp+Ch] [ebp-1Ch] BYREF
    rectDef_s rect; // [esp+10h] [ebp-18h] BYREF

    colType = 0;
    maxChars = 50;
    memset(&rect, 0, 20);
    rect.vertAlign = 4;
    if ( !PC_String_Parse(handle, &nextKey) || I_stricmp(nextKey, "{") )
    {
        Com_PrintError(13, "menuColDef Error: Expected '{'", nextKey);
        return 0;
    }
    else if ( !PC_String_Parse(handle, &nextKey) || I_stricmp(nextKey, "type") || PC_Int_Parse(handle, &colType) )
    {
        if ( (!colType || colType == 2 || colType == 3 || colType == 5 || colType == 1 || colType == 4)
            && PC_String_Parse(handle, &nextKey)
            && !I_stricmp(nextKey, "rect")
            && !PC_Rect_Parse(handle, &rect) )
        {
            Com_PrintError(13, "menuColDef Error: Expected 'rect'\n");
            return 0;
        }
        else if ( !PC_String_Parse(handle, &nextKey)
                     || I_stricmp(nextKey, "maxChars")
                     || PC_Int_Parse(handle, &maxChars) )
        {
            if ( PC_String_Parse(handle, &nextKey) && !I_stricmp(nextKey, "}") )
            {
                cells[cellCount].maxChars = maxChars;
                cells[cellCount].type = colType;
                for ( i = 0; i < listPtr->maxRows; ++i )
                {
                    listPtr->columnInfo[listPtr->numColumns].maxChars = maxChars;
                    p_rect = &listPtr->columnInfo[listPtr->numColumns].rect;
                    p_rect->x = rect.x;
                    p_rect->y = rect.y;
                    p_rect->w = rect.w;
                    p_rect->h = rect.h;
                    p_rect->horzAlign = rect.horzAlign;
                    p_rect->vertAlign = rect.vertAlign;
                }
                ++listPtr->numColumns;
                return 1;
            }
            else
            {
                Com_PrintError(13, "menuColDef Error: Expected '}'", nextKey);
                return 0;
            }
        }
        else
        {
            Com_PrintError(13, "menuColDef Error: Expected 'maxChars'\n");
            return 0;
        }
    }
    else
    {
        Com_PrintError(13, "menuColDef Error: Expected 'type'.\n");
        return 0;
    }
}

int    ItemParse_menuarea(itemDef_s *item, int handle)
{
    int j; // [esp+1Ch] [ebp-14h]
    int k; // [esp+20h] [ebp-10h]
    int i; // [esp+24h] [ebp-Ch]
    listBoxDef_s *listPtr; // [esp+28h] [ebp-8h]
    int colType; // [esp+2Ch] [ebp-4h] BYREF

    Item_ValidateTypeData(item, handle);
    if ( !item->typeData.textDef )
        return 0;
    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr )
        return 0;
    if ( PC_Int_Parse(handle, &listPtr->maxRows) && PC_Int_Parse(handle, &listPtr->numColumns) )
    {
        listPtr->rowCount = 0;
        listPtr->rows = (MenuRow *)UI_Alloc(24 * listPtr->maxRows, 4);
        for ( i = 0; i < listPtr->maxRows; ++i )
            listPtr->rows[i].cells = (MenuCell *)UI_Alloc(12 * listPtr->numColumns, 4);
        for ( k = 0; k < listPtr->numColumns; ++k )
        {
            if ( !PC_Float_Parse(handle, &listPtr->columnInfo[k].rect.x)
                || !PC_Float_Parse(handle, &listPtr->columnInfo[k].rect.y)
                || !PC_Float_Parse(handle, &listPtr->columnInfo[k].rect.w)
                || !PC_Float_Parse(handle, &listPtr->columnInfo[k].rect.h)
                || !PC_Int_Parse(handle, &listPtr->columnInfo[k].maxChars)
                || !PC_Int_ParseLine(handle, &listPtr->columnInfo[k].rect.horzAlign)
                || !PC_Int_ParseLine(handle, &listPtr->columnInfo[k].rect.vertAlign)
                || !PC_Int_ParseLine(handle, &colType) )
            {
                return 0;
            }
            for ( j = 0; j < listPtr->maxRows; ++j )
            {
                listPtr->rows[j].cells[k].maxChars = listPtr->columnInfo[k].maxChars;
                //HIDWORD(a1) = 12 * k;
                listPtr->rows[j].cells[k].stringValue = (char *)UI_Alloc(listPtr->columnInfo[k].maxChars, 4);
                listPtr->rows[j].cells[k].type = colType;
            }
        }
    }
    return 1;
}

int    ItemParse_userarea(itemDef_s *item, int handle)
{
    int vertAlign; // [esp+0h] [ebp-28h] BYREF
    int horzAlign; // [esp+4h] [ebp-24h] BYREF
    int maxChars; // [esp+8h] [ebp-20h] BYREF
    float x; // [esp+Ch] [ebp-1Ch] BYREF
    float y; // [esp+10h] [ebp-18h] BYREF
    float h; // [esp+14h] [ebp-14h] BYREF
    float w; // [esp+18h] [ebp-10h] BYREF
    listBoxDef_s *listPtr; // [esp+1Ch] [ebp-Ch]
    int num; // [esp+20h] [ebp-8h] BYREF
    int i; // [esp+24h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    if ( !item->typeData.textDef )
        return 0;
    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr )
        return 0;
    if ( !PC_Int_Parse(handle, &num) )
        return 0;
    if ( num > 16 )
        num = 16;
    listPtr->numColumns = num;
    for ( i = 0; i < num; ++i )
    {
        if ( !PC_Float_Parse(handle, &x)
            || !PC_Float_Parse(handle, &y)
            || !PC_Float_Parse(handle, &w)
            || !PC_Float_Parse(handle, &h)
            || !PC_Int_Parse(handle, &maxChars) )
        {
            return 0;
        }
        listPtr->columnInfo[i].rect.x = x;
        listPtr->columnInfo[i].rect.y = y;
        listPtr->columnInfo[i].rect.w = w;
        listPtr->columnInfo[i].rect.h = h;
        listPtr->columnInfo[i].maxChars = maxChars;
        if ( PC_Int_ParseLine(handle, &horzAlign) )
            listPtr->columnInfo[i].rect.horzAlign = horzAlign;
        else
            listPtr->columnInfo[i].rect.horzAlign = 0;
        if ( PC_Int_ParseLine(handle, &vertAlign) )
            listPtr->columnInfo[i].rect.vertAlign = vertAlign;
        else
            listPtr->columnInfo[i].rect.vertAlign = 4;
    }
    return 1;
}

int ItemParse_border(itemDef_s *item, int handle)
{
    return PC_Byte_Parse(handle, &item->window.border) != 0;
}

int ItemParse_bordersize(itemDef_s *item, int handle)
{
    return PC_Float_Parse(handle, &item->window.borderSize) != 0;
}

int __cdecl ItemParse_visible(itemDef_s *item, int handle)
{
    const char *string; // [esp+0h] [ebp-Ch] BYREF
    int flags; // [esp+8h] [ebp-4h]

    if ( !PC_String_Parse(handle, &string) )
        return 0;
    if ( !I_stricmp(string, "when") || !I_stricmp(string, "if") )
    {
        flags = Window_GetDynamicFlags(0, &item->window);
        Window_SetDynamicFlags(0, &item->window, flags | 4);
        if ( Expression_Read(handle, &item->visibleExp) )
            return 1;
    }
    if ( atoi(string) )
    {
        flags = Window_GetDynamicFlags(0, &item->window);
        Window_SetDynamicFlags(0, &item->window, flags | 4);
    }
    return 1;
}

int __cdecl ItemParse_visiblityBits(itemDef_s *item, int handle)
{
    int v3; // edx
    int v4; // edx
    bool setNextBitAsHideBit; // [esp+23h] [ebp-41Dh]
    const char *string; // [esp+24h] [ebp-41Ch] BYREF
    pc_token_s token; // [esp+28h] [ebp-418h] BYREF
    int flags; // [esp+43Ch] [ebp-4h]

    if ( !PC_String_Parse(handle, &string) )
        return 0;
    if ( I_stricmp(string, "bits") )
    {
        PC_SourceError(handle, "Expected 'bits( ' after visibilityBits.\n");
        return 0;
    }
    else if ( PC_ReadTokenHandle(handle, &token) )
    {
        if ( I_strcmp(token.string, "(") )
        {
            PC_SourceError(handle, "Expected '(' after 'bits' .\n");
            return 0;
        }
        else
        {
            item->showBits = 0;
            item->hideBits = -1;
            setNextBitAsHideBit = 0;
            while ( 1 )
            {
                if ( !PC_ReadTokenHandle(handle, &token) )
                    goto LABEL_39;
                if ( token.type != 3 && I_strcmp(token.string, "!") && I_strcmp(token.string, ")") )
                    break;
                if ( token.type == 3 )
                {
                    if ( setNextBitAsHideBit )
                    {
                        if ( (item->showBits & (1LL << SLOBYTE(token.intvalue))) != 0 )
                        {
LABEL_25:
                            PC_SourceError(handle, "Cannot have bit %d set and reset at the same time.\n", token.intvalue);
                            return 0;
                        }
                        v4 = HIDWORD(item->hideBits) & ~((unsigned __int64)(1LL << SLOBYTE(token.intvalue)) >> 32);
                        LODWORD(item->hideBits) &= ~(unsigned int)(1LL << SLOBYTE(token.intvalue));
                        HIDWORD(item->hideBits) = v4;
                        setNextBitAsHideBit = 0;
                    }
                    else
                    {
                        if ( (item->hideBits & (1LL << SLOBYTE(token.intvalue))) == 0 )
                            goto LABEL_25;
                        v3 = HIDWORD(item->showBits) | ((unsigned __int64)(1LL << SLOBYTE(token.intvalue)) >> 32);
                        LODWORD(item->showBits) |= 1LL << SLOBYTE(token.intvalue);
                        HIDWORD(item->showBits) = v3;
                    }
                }
                else if ( I_strcmp(token.string, "!") )
                {
                    if ( !I_strcmp(token.string, ")") )
                    {
                        if ( setNextBitAsHideBit )
                        {
                            PC_SourceError(handle, "Expected an integer after '!' but found ')' .\n");
                            return 0;
                        }
                        if ( !item->showBits && (HIDWORD(item->hideBits) & item->hideBits) == -1 )
                        {
                            PC_SourceError(handle, "No bits specified.\n");
                            return 0;
                        }
LABEL_39:
                        flags = Window_GetDynamicFlags(0, &item->window);
                        Window_SetDynamicFlags(0, &item->window, flags | 4);
                        return 1;
                    }
                }
                else
                {
                    if ( setNextBitAsHideBit )
                    {
                        PC_SourceError(handle, "Expected an integer after '!' but found another '!' .\n");
                        return 0;
                    }
                    setNextBitAsHideBit = 1;
                }
            }
            if ( setNextBitAsHideBit )
            {
                PC_SourceError(handle, "Expected an integer after '!' but found %s\n", token.string);
                return 0;
            }
            else if ( item->showBits || (HIDWORD(item->hideBits) & item->hideBits) != -1 )
            {
                PC_SourceError(handle, "Expected an integer or a closing ')' but found %s\n", token.string);
                return 0;
            }
            else
            {
                PC_SourceError(handle, "Expected an integer but found %s\n", token.string);
                return 0;
            }
        }
    }
    else
    {
        return 0;
    }
}

int ItemParse_ownerdraw(itemDef_s *item, int handle)
{
    if ( !Item_IsOwnerDrawDefType(item) )
        PC_SourceError(handle, "ownerdraw does not have a type set\n");
    return PC_Int_Parse(handle, &item->window.ownerDraw) != 0;
}

int ItemParse_align(itemDef_s *item, int handle)
{
    textDef_s *textDefPtr; // [esp+0h] [ebp-4h]

    textDefPtr = Item_GetTextDef(item);
    return PC_Int_Parse(handle, &textDefPtr->alignment) != 0;
}

int    ItemParse_textalign(itemDef_s *item, int handle)
{
    textDef_s *textDefPtr; // [esp+0h] [ebp-4h]

    textDefPtr = Item_GetTextDef(item);
    if ( !PC_Int_Parse(handle, &textDefPtr->textAlignMode) )
        return 0;
    if ( ItemParse_IsValidTextAlignment(textDefPtr->textAlignMode) )
        return 1;
    PC_SourceError(handle, "expected ITEM_ALIGN_* value\n");
    return 0;
}

bool __cdecl ItemParse_IsValidTextAlignment(unsigned int textAlignMode)
{
    return textAlignMode < 0x10 && (textAlignMode & 3) != 3;
}

int ItemParse_textalignx(itemDef_s *item, int handle)
{
    textDef_s *textDefPtr; // [esp+0h] [ebp-4h]

    textDefPtr = Item_GetTextDef(item);
    return PC_Float_Parse(handle, &textDefPtr->textalignx) != 0;
}

int ItemParse_textaligny(itemDef_s *item, int handle)
{
    textDef_s *textDefPtr; // [esp+0h] [ebp-4h]

    textDefPtr = Item_GetTextDef(item);
    return PC_Float_Parse(handle, &textDefPtr->textaligny) != 0;
}

int ItemParse_textscale(itemDef_s *item, int handle)
{
    textDef_s *textDefPtr; // [esp+0h] [ebp-4h]

    textDefPtr = Item_GetTextDef(item);
    return PC_Float_Parse(handle, &textDefPtr->textscale) != 0;
}

int ItemParse_textstyle(itemDef_s *item, int handle)
{
    textDef_s *textDefPtr; // [esp+0h] [ebp-4h]

    textDefPtr = Item_GetTextDef(item);
    return PC_Int_Parse(handle, &textDefPtr->textStyle) != 0;
}

int ItemParse_rotation(itemDef_s *item, int handle)
{
    return PC_Float_Parse(handle, &item->window.rotation) != 0;
}

int ItemParse_textfont(itemDef_s *item, int handle)
{
    textDef_s *textDefPtr; // [esp+0h] [ebp-4h]

    textDefPtr = Item_GetTextDef(item);
    return PC_Int_Parse(handle, &textDefPtr->fontEnum) != 0;
}

int    ItemParse_backcolor(itemDef_s *item, int handle)
{
    float f; // [esp+0h] [ebp-8h] BYREF
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 4; ++i )
    {
        if ( !PC_Float_Parse(handle, &f) )
            return 0;
        item->window.backColor[i] = f;
    }
    return 1;
}

int    ItemParse_forecolor(itemDef_s *item, int handle)
{
    float f; // [esp+0h] [ebp-Ch] BYREF
    int i; // [esp+4h] [ebp-8h]
    int flags; // [esp+8h] [ebp-4h]

    for ( i = 0; i < 4; ++i )
    {
        if ( !PC_Float_Parse(handle, &f) )
            return 0;
        item->window.foreColor[i] = f;
        flags = Window_GetDynamicFlags(0, &item->window);
        Window_SetDynamicFlags(0, &item->window, flags | 0x10000);
    }
    return 1;
}

int    ItemParse_bordercolor(itemDef_s *item, int handle)
{
    float f; // [esp+0h] [ebp-8h] BYREF
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 4; ++i )
    {
        if ( !PC_Float_Parse(handle, &f) )
            return 0;
        item->window.borderColor[i] = f;
    }
    return 1;
}

int __cdecl ItemParse_modal(itemDef_s *item, int handle)
{
    item->window.modal = 1;
    return 1;
}

int ItemParse_outlinecolor(itemDef_s *item, int handle)
{
    return PC_Color_Parse(handle, (float (*)[4])item->window.outlineColor) != 0;
}

int __cdecl ItemParse_background(itemDef_s *item, int handle)
{
    char name[64]; // [esp+0h] [ebp-48h] BYREF
    const char *temp; // [esp+44h] [ebp-4h] BYREF

    if ( !PC_String_Parse(handle, &temp) )
        return 0;
    I_strncpyz(name, temp, 64);
    I_strlwr(name);
    item->window.background = Material_RegisterHandle(name, item->imageTrack);
    return 1;
}

int __cdecl ItemParse_doubleClick(itemDef_s *item, int handle)
{
    GenericEventHandler *eventHandler; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    if ( !item->typeData.textDef )
        return 0;
    if ( !Item_GetListBoxDef(item) )
        return 0;
    eventHandler = (GenericEventHandler *)UI_Alloc(0xCu, 4);
    eventHandler->name = String_Alloc("doubleClick");
    eventHandler->next = item->onEvent;
    item->onEvent = eventHandler;
    return PC_EventScript_Parse(handle, &eventHandler->eventScript);
}

int __cdecl ItemParse_rightClick(itemDef_s *item, int handle)
{
    GenericEventHandler *eventHandler; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    if ( !item->typeData.textDef )
        return 0;
    if ( !Item_GetListBoxDef(item) )
        return 0;
    eventHandler = (GenericEventHandler *)UI_Alloc(0xCu, 4);
    eventHandler->name = String_Alloc("RightClick");
    eventHandler->next = item->onEvent;
    item->onEvent = eventHandler;
    return PC_EventScript_Parse(handle, &eventHandler->eventScript);
}

int __cdecl ItemParse_onEnter(itemDef_s *item, int handle)
{
    GenericEventHandler *eventHandler; // [esp+0h] [ebp-4h]

    eventHandler = (GenericEventHandler *)UI_Alloc(0xCu, 4);
    eventHandler->name = String_Alloc("onEnter");
    eventHandler->next = item->animInfo->animStates[item->animInfo->animStateCount]->onEvent;
    item->animInfo->animStates[item->animInfo->animStateCount]->onEvent = eventHandler;
    return PC_EventScript_Parse(handle, &eventHandler->eventScript);
}

int __cdecl ItemParse_onFocus(itemDef_s *item, int handle)
{
    GenericEventHandler *eventHandler; // [esp+0h] [ebp-4h]

    eventHandler = (GenericEventHandler *)UI_Alloc(0xCu, 4);
    eventHandler->name = String_Alloc("onFocus");
    eventHandler->next = item->onEvent;
    item->onEvent = eventHandler;
    return PC_EventScript_Parse(handle, &eventHandler->eventScript);
}

int __cdecl ItemParse_leaveFocus(itemDef_s *item, int handle)
{
    GenericEventHandler *eventHandler; // [esp+0h] [ebp-4h]

    eventHandler = (GenericEventHandler *)UI_Alloc(0xCu, 4);
    eventHandler->name = String_Alloc("leaveFocus");
    eventHandler->next = item->onEvent;
    item->onEvent = eventHandler;
    return PC_EventScript_Parse(handle, &eventHandler->eventScript);
}

int __cdecl ItemParse_mouseEnter(itemDef_s *item, int handle)
{
    focusItemDef_s *focusPtr; // [esp+0h] [ebp-4h]

    focusPtr = Item_GetFocusItemDef(item);
    return PC_Script_Parse(handle, &focusPtr->mouseEnter) != 0;
}

int __cdecl PC_Script_Parse(int handle, const char **out)
{
    const char *v3; // eax
    char dst[5120]; // [esp+34h] [ebp-1818h] BYREF
    pc_token_s pc_token; // [esp+1434h] [ebp-418h] BYREF

    memset(dst, 0, sizeof(dst));
    if ( !PC_ReadTokenHandle(handle, &pc_token) )
        return 0;
    if ( I_stricmp(pc_token.string, "{") )
        return 0;
    do
    {
        if ( !PC_ReadTokenHandle(handle, &pc_token) )
            return 0;
        if ( !I_stricmp(pc_token.string, "}") )
        {
            *out = String_Alloc(dst);
            return 1;
        }
        if ( strlen(pc_token.string) + &dst[strlen(dst) + 1] - &dst[1] > 0x1400 )
            break;
        if ( pc_token.type <= 0 )
            goto LABEL_16;
        if ( pc_token.type <= 2 )
        {
            v3 = va("\"%s\" ", pc_token.string);
            I_strncat(dst, 5120, v3);
        }
        else if ( pc_token.type != 6 )
        {
LABEL_16:
            I_strncat(dst, 5120, pc_token.string);
            if ( pc_token.type != 5 || pc_token.subtype != 30 )
                I_strncat(dst, 5120, " ");
        }
    }
    while ( (unsigned int)(&dst[strlen(dst) + 1] - &dst[1] + 1) <= 0x1400 );
    Com_PrintError(16, "action block too long that starts with: %s\n", dst);
    return 0;
}

int __cdecl ItemParse_mouseExit(itemDef_s *item, int handle)
{
    focusItemDef_s *focusPtr; // [esp+0h] [ebp-4h]

    focusPtr = Item_GetFocusItemDef(item);
    return PC_Script_Parse(handle, &focusPtr->mouseExit) != 0;
}

int __cdecl ItemParse_mouseEnterText(itemDef_s *item, int handle)
{
    focusItemDef_s *focusPtr; // [esp+0h] [ebp-4h]

    focusPtr = Item_GetFocusItemDef(item);
    return PC_Script_Parse(handle, &focusPtr->mouseEnterText) != 0;
}

int __cdecl ItemParse_mouseExitText(itemDef_s *item, int handle)
{
    focusItemDef_s *focusPtr; // [esp+0h] [ebp-4h]

    focusPtr = Item_GetFocusItemDef(item);
    return PC_Script_Parse(handle, &focusPtr->mouseExitText) != 0;
}

int __cdecl ItemParse_action(itemDef_s *item, int handle)
{
    GenericEventHandler *eventHandler; // [esp+0h] [ebp-4h]

    eventHandler = (GenericEventHandler *)UI_Alloc(0xCu, 4);
    eventHandler->name = String_Alloc("action");
    eventHandler->next = item->onEvent;
    item->onEvent = eventHandler;
    return PC_EventScript_Parse(handle, &eventHandler->eventScript);
}

int __cdecl ItemParse_accept(itemDef_s *item, int handle)
{
    GenericEventHandler *eventHandler; // [esp+0h] [ebp-4h]

    eventHandler = (GenericEventHandler *)UI_Alloc(0xCu, 4);
    eventHandler->name = String_Alloc("onAccept");
    eventHandler->next = item->onEvent;
    item->onEvent = eventHandler;
    return PC_EventScript_Parse(handle, &eventHandler->eventScript);
}

int __cdecl ItemParse_dvarTest(itemDef_s *item, int handle)
{
    return PC_String_Parse(handle, &item->dvarTest) != 0;
}

int __cdecl ItemParse_dvar(itemDef_s *item, int handle)
{
    editFieldDef_s *editPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    if ( !PC_String_Parse(handle, &item->dvar) )
        return 0;
    if ( item->typeData.textDef && Item_IsEditFieldDef(item) )
    {
        editPtr = Item_GetEditFieldDef(item);
        if ( !editPtr
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared_obj.cpp", 3444, 0, "%s", "editPtr") )
        {
            __debugbreak();
        }
        editPtr->minVal = -1.0f;
        editPtr->maxVal = -1.0f;
        editPtr->defVal = -1.0f;
    }
    return 1;
}

int    ItemParse_maxChars(itemDef_s *item, int handle)
{
    editFieldDef_s *editPtr; // [esp+0h] [ebp-8h]
    int maxChars; // [esp+4h] [ebp-4h] BYREF

    Item_ValidateTypeData(item, handle);
    if ( !item->typeData.textDef )
        return 0;
    if ( !PC_Int_Parse(handle, &maxChars) )
        return 0;
    editPtr = Item_GetEditFieldDef(item);
    if ( !editPtr )
        return 0;
    editPtr->maxChars = maxChars;
    return 1;
}

int __cdecl Item_Parse_maxCharsGotoNext(itemDef_s *item, int handle)
{
    editFieldDef_s *editPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    if ( !item->typeData.textDef )
        return 0;
    editPtr = Item_GetEditFieldDef(item);
    if ( !editPtr )
        return 0;
    editPtr->maxCharsGotoNext = 1;
    return 1;
}

int    ItemParse_maxPaintChars(itemDef_s *item, int handle)
{
    editFieldDef_s *editPtr; // [esp+0h] [ebp-8h]
    int maxChars; // [esp+4h] [ebp-4h] BYREF

    Item_ValidateTypeData(item, handle);
    if ( !item->typeData.textDef )
        return 0;
    if ( !PC_Int_Parse(handle, &maxChars) )
        return 0;
    editPtr = Item_GetEditFieldDef(item);
    if ( !editPtr )
        return 0;
    editPtr->maxPaintChars = maxChars;
    return 1;
}

int ItemParse_dvarFloat(itemDef_s *item, int handle)
{
    editFieldDef_s *editPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    if ( !item->typeData.textDef )
        return 0;
    editPtr = Item_GetEditFieldDef(item);
    if ( !editPtr )
        return 0;
    return PC_String_Parse(handle, &item->dvar)
            && PC_Float_Parse(handle, &editPtr->defVal)
            && PC_Float_Parse(handle, &editPtr->minVal)
            && PC_Float_Parse(handle, &editPtr->maxVal);
}

int __cdecl ItemParse_dvarStrList(itemDef_s *item, int handle)
{
    int pass; // [esp+0h] [ebp-420h]
    multiDef_s *multiPtr; // [esp+4h] [ebp-41Ch]
    pc_token_s token; // [esp+8h] [ebp-418h] BYREF

    Item_ValidateTypeData(item, handle);
    if ( !item->typeData.textDef )
        return 0;
    if ( item->type != 10 )
        return 0;
    multiPtr = Item_GetMultiDef(item);
    if ( !multiPtr
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared_obj.cpp", 3542, 1, "%s", "multiPtr") )
    {
        __debugbreak();
    }
    multiPtr->count = 0;
    multiPtr->strDef = 1;
    multiPtr->actionOnEnterPressOnly = 0;
    if ( !PC_ReadTokenHandle(handle, &token) )
        return 0;
    if ( token.string[0] != 123 )
        return 0;
    pass = 0;
    do
    {
        while ( 1 )
        {
            do
            {
                if ( !PC_ReadTokenHandle(handle, &token) )
                {
                    PC_SourceError(handle, "end of file inside menu item\n");
                    return 0;
                }
                if ( token.string[0] == 125 )
                    return 1;
            }
            while ( token.string[0] == 44 || token.string[0] == 59 );
            if ( pass )
                break;
            multiPtr->dvarList[multiPtr->count] = String_Alloc(token.string);
            pass = 1;
        }
        multiPtr->dvarStr[multiPtr->count] = String_Alloc(token.string);
        pass = 0;
        ++multiPtr->count;
    }
    while ( multiPtr->count < 32 );
    return 0;
}

int    ItemParse_dvarFloatList(itemDef_s *item, int handle)
{
    multiDef_s *multiPtr; // [esp+0h] [ebp-41Ch]
    pc_token_s token; // [esp+4h] [ebp-418h] BYREF

    Item_ValidateTypeData(item, handle);
    if ( !item->typeData.textDef )
        return 0;
    if ( item->type != 10 )
        return 0;
    multiPtr = Item_GetMultiDef(item);
    if ( !multiPtr
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared_obj.cpp", 3604, 1, "%s", "multiPtr") )
    {
        __debugbreak();
    }
    multiPtr->count = 0;
    multiPtr->strDef = 0;
    multiPtr->actionOnEnterPressOnly = 0;
    if ( !PC_ReadTokenHandle(handle, &token) )
        return 0;
    if ( token.string[0] != 123 )
        return 0;
    do
    {
        do
        {
            if ( !PC_ReadTokenHandle(handle, &token) )
            {
                PC_SourceError(handle, "end of file inside menu item\n");
                return 0;
            }
            if ( token.string[0] == 125 )
                return 1;
        }
        while ( token.string[0] == 44 || token.string[0] == 59 );
        multiPtr->dvarList[multiPtr->count] = String_Alloc(token.string);
        if ( !PC_Float_Parse(handle, &multiPtr->dvarValue[multiPtr->count]) )
            return 0;
        ++multiPtr->count;
    }
    while ( multiPtr->count < 32 );
    return 0;
}

int __cdecl ItemParse_dvarEnumList(itemDef_s *item, int handle)
{
    enumDvarDef_s *enumDvarPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    enumDvarPtr = Item_GetEnumDvarName(item);
    if ( item->type != 11 )
        return 0;
    if ( !enumDvarPtr->enumDvarName )
        return PC_String_Parse(handle, &enumDvarPtr->enumDvarName);
    PC_SourceError(handle, "enumDvarList already given");
    return 0;
}

int __cdecl ItemParse_actionOnEnterPressOnly(itemDef_s *item, int handle)
{
    multiDef_s *multiPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    if ( !item->typeData.textDef )
        return 0;
    if ( item->type != 10 )
        return 0;
    multiPtr = Item_GetMultiDef(item);
    if ( !multiPtr
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared_obj.cpp", 3675, 1, "%s", "multiPtr") )
    {
        __debugbreak();
    }
    multiPtr->actionOnEnterPressOnly = 1;
    return 1;
}

int __cdecl ItemParse_enableDvar(itemDef_s *item, int handle)
{
    if ( !PC_Script_Parse(handle, &item->enableDvar) )
        return 0;
    item->dvarFlags |= 1u;
    return 1;
}

int __cdecl ItemParse_disableDvar(itemDef_s *item, int handle)
{
    if ( !PC_Script_Parse(handle, &item->enableDvar) )
        return 0;
    item->dvarFlags |= 2u;
    return 1;
}

int __cdecl ItemParse_showDvar(itemDef_s *item, int handle)
{
    if ( !PC_Script_Parse(handle, &item->enableDvar) )
        return 0;
    item->dvarFlags |= 4u;
    return 1;
}

int __cdecl ItemParse_hideDvar(itemDef_s *item, int handle)
{
    if ( !PC_Script_Parse(handle, &item->enableDvar) )
        return 0;
    item->dvarFlags |= 8u;
    return 1;
}

int __cdecl ItemParse_focusDvar(itemDef_s *item, int handle)
{
    if ( !PC_Script_Parse(handle, &item->enableDvar) )
        return 0;
    item->dvarFlags |= 0x10u;
    return 1;
}

int __cdecl ItemParse_onEvent(itemDef_s *item, int handle)
{
    GenericEventHandler *eventHandler; // [esp+0h] [ebp-4h]

    eventHandler = (GenericEventHandler *)UI_Alloc(0xCu, 4);
    if ( !PC_String_Parse(handle, &eventHandler->name) )
        return 0;
    eventHandler->next = item->onEvent;
    item->onEvent = eventHandler;
    return PC_EventScript_Parse(handle, &eventHandler->eventScript) != 0;
}

int __cdecl ItemParse_execKey(itemDef_s *item, int handle)
{
    focusItemDef_s *focusPtr; // [esp+0h] [ebp-10h]
    __int16 keyindex; // [esp+4h] [ebp-Ch]
    char keyname; // [esp+Bh] [ebp-5h] BYREF
    ItemKeyHandler *handler; // [esp+Ch] [ebp-4h]

    if ( !PC_Char_Parse(handle, &keyname) )
        return 0;
    keyindex = (unsigned __int8)keyname;
    focusPtr = Item_GetFocusItemDef(item);
    handler = (ItemKeyHandler *)UI_Alloc(0xCu, 4);
    handler->key = keyindex;
    if ( !PC_EventScript_Parse(handle, &handler->keyScript) )
        return 0;
    handler->next = focusPtr->onKey;
    focusPtr->onKey = handler;
    return 1;
}

int    ItemParse_execKeyInt(itemDef_s *item, int handle)
{
    focusItemDef_s *focusPtr; // [esp+0h] [ebp-Ch]
    int keyname; // [esp+4h] [ebp-8h] BYREF
    ItemKeyHandler *handler; // [esp+8h] [ebp-4h]

    if ( !PC_Int_Parse(handle, &keyname) )
        return 0;
    focusPtr = Item_GetFocusItemDef(item);
    handler = (ItemKeyHandler *)UI_Alloc(0xCu, 4);
    handler->key = keyname;
    if ( !PC_EventScript_Parse(handle, &handler->keyScript) )
        return 0;
    handler->next = focusPtr->onKey;
    focusPtr->onKey = handler;
    return 1;
}

int __cdecl ItemParse_execExp(itemDef_s *item, int handle)
{
    const char *expressionType; // [esp+0h] [ebp-10h] BYREF
    ownerDrawDef_s *ownerDrawDefPtr; // [esp+4h] [ebp-Ch]
    textDef_s *textDefPtr; // [esp+8h] [ebp-8h]
    int flags; // [esp+Ch] [ebp-4h]

    if ( !PC_String_Parse(handle, &expressionType) )
        return 0;
    if ( I_stricmp(expressionType, "visible") )
    {
        if ( I_stricmp(expressionType, "text") )
        {
            if ( I_stricmp(expressionType, "material") )
            {
                if ( I_stricmp(expressionType, "rect") )
                {
                    if ( I_stricmp(expressionType, "forecolor") )
                    {
                        if ( !I_stricmp(expressionType, "ownerdata") )
                        {
                            ownerDrawDefPtr = Item_GetOwnerDrawDef(item);
                            if ( Expression_Read(handle, &ownerDrawDefPtr->dataExp) )
                                return 1;
                        }
                    }
                    else
                    {
                        if ( !PC_String_Parse(handle, &expressionType) )
                        {
                            Com_PrintError(16, "ERROR: line ended early after \"exp forecolor\"\n");
                            return 0;
                        }
                        if ( I_stricmp(expressionType, "A") )
                        {
                            Com_PrintError(16, "ERROR: Expected 'A' after \"exp forecolor\" but found \"%s\"\n", expressionType);
                            return 0;
                        }
                        if ( Expression_Read(handle, &item->forecolorAExp) )
                            return 1;
                    }
                }
                else
                {
                    if ( !PC_String_Parse(handle, &expressionType) )
                    {
                        Com_PrintError(16, "ERROR: line ended early after \"exp rect\"\n");
                        return 0;
                    }
                    if ( !item->rectExpData )
                        item->rectExpData = (rectData_s *)UI_Alloc(0x40u, 4);
                    if ( I_stricmp(expressionType, "X") )
                    {
                        if ( !I_stricmp(expressionType, "Y") && Expression_Read(handle, &item->rectExpData->rectYExp) )
                            return 1;
                    }
                    else if ( Expression_Read(handle, &item->rectExpData->rectXExp) )
                    {
                        return 1;
                    }
                    if ( I_stricmp(expressionType, "W") )
                    {
                        if ( I_stricmp(expressionType, "H") )
                        {
                            Com_PrintError(
                                16,
                                "ERROR: Expected 'X', 'Y', 'W', or 'H' after \"exp rect\" but found \"%s\"\n",
                                expressionType);
                            return 0;
                        }
                        if ( Expression_Read(handle, &item->rectExpData->rectHExp) )
                            return 1;
                    }
                    else if ( Expression_Read(handle, &item->rectExpData->rectWExp) )
                    {
                        return 1;
                    }
                }
            }
            else
            {
                if ( item->type != 2 )
                    Com_Error(ERR_DROP, "set type to ITEM_TYPE_IMAGE if using keyword \"material\" \n");
                if ( Expression_Read(handle, (ExpressionStatement *)item->typeData.textDef) )
                    return 1;
            }
        }
        else
        {
            textDefPtr = Item_GetTextDef(item);
            if ( !textDefPtr->textExpData )
                textDefPtr->textExpData = (textExp_s *)UI_Alloc(0x10u, 4);
            if ( Expression_Read(handle, &textDefPtr->textExpData->textExp) )
                return 1;
        }
    }
    else
    {
        flags = Window_GetDynamicFlags(0, &item->window);
        Window_SetDynamicFlags(0, &item->window, flags | 4);
        if ( Expression_Read(handle, &item->visibleExp) )
            return 1;
    }
    return 0;
}

int    ItemParse_gameMsgWindowIndex(itemDef_s *item, int handle)
{
    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared_obj.cpp", 3922, 0, "%s", "item") )
        __debugbreak();
    return PC_Int_Parse(handle, (int *)item->typeData.textDef->textTypeData.focusItemDef);
}

int    ItemParse_gameMsgWindowMode(itemDef_s *item, int handle)
{
    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared_obj.cpp", 3929, 0, "%s", "item") )
        __debugbreak();
    return PC_Int_Parse(handle, (int *)&item->typeData.textDef->textTypeData.focusItemDef->mouseExitText);
}

int    ItemParse_selectBorder(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    if ( listPtr )
        return PC_Color_Parse(handle, (float (*)[4])listPtr->selectBorder);
    else
        return 0;
}

int    ItemParse_elementHighlightColor(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    if ( listPtr )
        return PC_Color_Parse(handle, (float (*)[4])listPtr->elementHighlightColor);
    else
        return 0;
}

int    ItemParse_elementBackgroundColor(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    if ( listPtr )
        return PC_Color_Parse(handle, (float (*)[4])listPtr->elementBackgroundColor);
    else
        return 0;
}

int    ItemParse_disableColor(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    if ( listPtr )
        return PC_Color_Parse(handle, (float (*)[4])listPtr->disableColor);
    else
        return 0;
}

int    ItemParse_focusColor(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    if ( listPtr )
        return PC_Color_Parse(handle, (float (*)[4])listPtr->focusColor);
    else
        return 0;
}

int __cdecl ItemParse_selectIcon(itemDef_s *item, int handle)
{
    char name[64]; // [esp+4h] [ebp-48h] BYREF
    const char *temp; // [esp+48h] [ebp-4h] BYREF

    Item_ValidateTypeData(item, handle);
    if ( !Item_GetListBoxDef(item) )
        return 0;
    if ( !PC_String_Parse(handle, &temp) )
        return 0;
    I_strncpyz(name, temp, 64);
    I_strlwr(name);
    return 1;
}

int __cdecl ItemParse_backgroundItemListbox(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4Ch]
    char name[64]; // [esp+4h] [ebp-48h] BYREF
    const char *temp; // [esp+48h] [ebp-4h] BYREF

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr )
        return 0;
    if ( !PC_String_Parse(handle, &temp) )
        return 0;
    I_strncpyz(name, temp, 64);
    I_strlwr(name);
    listPtr->backgroundItemListbox = Material_RegisterHandle(name, item->imageTrack);
    return 1;
}

int __cdecl ItemParse_highlightTexture(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4Ch]
    char name[64]; // [esp+4h] [ebp-48h] BYREF
    const char *temp; // [esp+48h] [ebp-4h] BYREF

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    if ( !listPtr )
        return 0;
    if ( !PC_String_Parse(handle, &temp) )
        return 0;
    I_strncpyz(name, temp, 64);
    I_strlwr(name);
    listPtr->highlightTexture = Material_RegisterHandle(name, item->imageTrack);
    return 1;
}

int __cdecl ItemParse_onListboxSelectionChange(itemDef_s *item, int handle)
{
    GenericEventHandler *eventHandler; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    if ( !Item_GetListBoxDef(item) )
        return 0;
    eventHandler = (GenericEventHandler *)UI_Alloc(0xCu, 4);
    eventHandler->name = String_Alloc("onListboxSelectionChange");
    eventHandler->next = item->onEvent;
    item->onEvent = eventHandler;
    return PC_EventScript_Parse(handle, &eventHandler->eventScript);
}

int    ItemParse_ui3dWindowId(itemDef_s *item, int handle)
{
    return PC_Int_Parse(handle, &item->ui3dWindowId);
}

int __cdecl ItemParse_state(itemDef_s *item, int handle)
{
    animParamsDef_t tempState; // [esp+8h] [ebp-78h] BYREF
    animParamsDef_t *animParams; // [esp+78h] [ebp-8h]
    int success; // [esp+7Ch] [ebp-4h]

    if ( !item->animInfo )
    {
        item->animInfo = (UIAnimInfo *)UI_Alloc(0xECu, 4);
        item->animInfo->animStates = g_load_0.animStates;
        *item->animInfo->animStates = (animParamsDef_t *)UI_Alloc(0x6Cu, 4);
        Item_PropertiesToAnimState(item, *item->animInfo->animStates);
        **(unsigned int **)item->animInfo->animStates = (unsigned int)String_Alloc("Default");
        ++item->animInfo->animStateCount;
    }
    animParams = (animParamsDef_t *)UI_Alloc(0x6Cu, 4);
    item->animInfo->animStates[item->animInfo->animStateCount] = animParams;
    Item_PropertiesToAnimState(item, item->animInfo->animStates[item->animInfo->animStateCount]);
    success = Item_Parse(handle, item);
    if ( !success )
        PC_SourceError(handle, "Error parsing state!");
    memcpy(&tempState, item->animInfo->animStates[item->animInfo->animStateCount], sizeof(tempState));
    Item_PropertiesToAnimState(item, item->animInfo->animStates[item->animInfo->animStateCount]);
    Item_AnimStateToProperties(&tempState, item);
    ++item->animInfo->animStateCount;
    return success;
}

void __cdecl Item_SetupKeywordHash()
{
    //unsigned int i; // [esp+0h] [ebp-4h]
    //
    //KeywordHash_Validate_itemDef_s_1024_6_(itemParseKeywords, 91);
    //memset(itemParseKeywordHash, 0, sizeof(itemParseKeywordHash));
    //for ( i = 0; i < 0x5B; ++i )
    //    KeywordHash_Add_itemDef_s_1024_6_(
    //        itemParseKeywordHash,
    //        (const KeywordHashEntry<itemDef_s,1024,6> *)(8 * i + 13622360));
    //
    ItemKeyword::Validate(itemParseKeywords, 91);

    memset(itemParseKeywordHash, 0, sizeof(itemParseKeywordHash));

    for (unsigned int i = 0; i < 91; ++i)
    {
        ItemKeyword::Add(
            itemParseKeywordHash,
            &itemParseKeywords[i]);
    }
}

MenuList *__cdecl UI_LoadMenu(const char *menuFile, int imageTrack)
{
    if ( useFastFile->current.enabled )
        return (MenuList *)((int (__cdecl *)(const char *, int))UI_LoadMenus_FastFile)(menuFile, imageTrack);
    else
        return (MenuList *)((int (__cdecl *)(const char *, int))UI_LoadMenu_LoadObj)(menuFile, imageTrack);
}

MenuList * UI_LoadMenu_LoadObj(char *menuFile, int imageTrack)
{
    memset(&g_load_0, 0, sizeof(g_load_0));
    g_load_0.menuList.menus = g_load_0.menus;
    if ( !UI_ParseMenuInternal(menuFile, imageTrack) )
    {
        Com_PrintWarning(13, "WARNING: menu file not found: %s\n", menuFile);
        if ( !UI_ParseMenuInternal((char*)"ui/default.menu", imageTrack) )
            Com_Error(ERR_DROP, "default.menu file not found. This is a default menu that you should have.");
    }
    return &g_load_0.menuList;
}

char    UI_ParseMenuInternal(char *menuFile, int imageTrack)
{
    int handle; // [esp+0h] [ebp-424h]
    const char *builtinDefines[2]; // [esp+4h] [ebp-420h] BYREF
    pc_token_s token; // [esp+Ch] [ebp-418h] BYREF

    builtinDefines[0] = "PC";
    builtinDefines[1] = 0;
    Com_Printf(13, "\tLoading '%s'...\n", menuFile);
    handle = PC_LoadSourceHandle(menuFile, builtinDefines);
    if ( handle )
    {
        while ( PC_ReadTokenHandle(handle, &token) )
        {
            if ( I_stricmp(token.string, "}") && I_stricmp(token.string, "{") )
            {
                if ( I_stricmp(token.string, "assetGlobalDef") )
                {
                    if ( I_stricmp(token.string, "menudef") )
                    {
                        PC_SourceError(
                            handle,
                            "Unknown token %s in menu file.    Expected \"menudef\" or \"assetglobaldef\".\n",
                            token.string);
                    }
                    else if ( !Menu_New(handle, imageTrack) )
                    {
                        break;
                    }
                }
                else if ( !Asset_Parse(handle) )
                {
                    break;
                }
            }
        }
        PC_FreeSourceHandle(handle);
        return 1;
    }
    else
    {
        Com_PrintError(13, "Couldn't find menu file '%s'\n", menuFile);
        return 0;
    }
}

int    Asset_Parse(int handle)
{
    pc_token_s token; // [esp+0h] [ebp-418h] BYREF

    if ( !PC_ReadTokenHandle(handle, &token) )
        return 0;
    if ( I_stricmp(token.string, "{") )
        return 0;
    do
    {
        while ( 1 )
        {
            while ( 1 )
            {
                while ( 1 )
                {
                    while ( 1 )
                    {
                        do
                        {
                            if ( !PC_ReadTokenHandle(handle, &token) )
                                return 0;
                        }
                        while ( !I_stricmp(token.string, ";") );
                        if ( !I_stricmp(token.string, "}") )
                            return 1;
                        if ( I_stricmp(token.string, "fadeClamp") )
                            break;
                        if ( !PC_Float_Parse(handle, &g_load_0.loadAssets.fadeClamp) )
                            return 0;
                    }
                    if ( I_stricmp(token.string, "fadeCycle") )
                        break;
                    if ( !PC_Int_Parse(handle, &g_load_0.loadAssets.fadeCycle) )
                        return 0;
                }
                if ( I_stricmp(token.string, "fadeAmount") )
                    break;
                if ( !PC_Float_Parse(handle, &g_load_0.loadAssets.fadeAmount) )
                    return 0;
            }
            if ( !I_stricmp(token.string, "fadeInAmount") )
                break;
            PC_SourceError(
                handle,
                "Unknown token %s in assetGlobalDef.    Valid commands are 'fadeClamp', 'fadeCycle', 'fadeAmount', and 'fadeInAmount'\n",
                token.string);
        }
    }
    while ( PC_Float_Parse(handle, &g_load_0.loadAssets.fadeInAmount) );
    return 0;
}

char __cdecl Menu_New(int handle, int imageTrack)
{
    menuDef_t *menu; // [esp+0h] [ebp-4h]

    menu = (menuDef_t *)UI_Alloc(0x190u, 4);
    Menu_Init(menu, imageTrack);
    if ( Menu_Parse(handle, menu) )
    {
        if ( menu->window.name )
        {
            Menu_PostParse(menu);
            if ( g_load_0.menuList.menuCount >= 512 )
                Com_Error(ERR_DROP, "Menu_New: EXE_ERR_OUT_OF_MEMORY");
            g_load_0.menuList.menus[g_load_0.menuList.menuCount++] = menu;
            return 1;
        }
        else
        {
            PC_SourceError(handle, "menu has no name");
            Menu_FreeMemory(menu);
            return 0;
        }
    }
    else
    {
        Menu_FreeMemory(menu);
        return 0;
    }
}

void __cdecl Menu_Init(menuDef_t *menu, int imageTrack)
{
    memset(menu, 0, sizeof(menuDef_t));
    Menu_SetCursorItem(0, menu, -1);
    menu->fadeAmount = g_load_0.loadAssets.fadeAmount;
    menu->fadeInAmount = g_load_0.loadAssets.fadeInAmount;
    menu->fadeClamp = g_load_0.loadAssets.fadeClamp;
    menu->fadeCycle = g_load_0.loadAssets.fadeCycle;
    menu->imageTrack = imageTrack;
    menu->items = g_load_0.items;
    menu->ui3dWindowId = -1;
    menu->priority = 2;
    menu->openSlideSpeed = -1;
    menu->closeSlideSpeed = -1;
    menu->openSlideDirection = -1;
    menu->closeSlideDirection = -1;
    menu->openFadingTime = -1;
    menu->closeFadingTime = -1;
    menu->slideTimeCounter = -1;
    menu->fadeTimeCounter = -1;
    menu->showBits = 0;
    menu->hideBits = -1;
    Window_Init(&menu->window);
}

int __cdecl Menu_Parse(int handle, menuDef_t *menu)
{
    const KeywordHashEntry<menuDef_t,1024,128> *key; // [esp+0h] [ebp-41Ch]
    pc_token_s token; // [esp+4h] [ebp-418h] BYREF

    if ( !PC_ReadTokenHandle(handle, &token) )
        return 0;
    if ( token.string[0] != 123 )
        return 0;
    do
    {
        while ( 1 )
        {
            do
            {
                memset(&token, 0, sizeof(token));
                if ( !PC_ReadTokenHandle(handle, &token) )
                {
                    PC_SourceError(handle, "end of file inside menu\n");
                    return 0;
                }
                if ( token.string[0] == 125 )
                    return 1;
            }
            while ( token.string[0] == 59 );
            //key = KeywordHash_Find_menuDef_t_1024_128_(menuParseKeywordHash, token.string);
            key = MenuKeyword::Find(menuParseKeywordHash, token.string);
            if ( key )
                break;
            PC_SourceError(handle, "unknown menu keyword %s", token.string);
        }
    }
    while ( key->func(menu, handle) );
    PC_SourceError(handle, "couldn't parse menu keyword %s", token.string);
    return 0;
}

void __cdecl Menu_PostParse(menuDef_t *menu)
{
    unsigned int size; // [esp+0h] [ebp-4h]

    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_shared_obj.cpp", 4324, 0, "%s", "menu") )
        __debugbreak();
    size = 4 * menu->itemCount;
    menu->items = (itemDef_s **)UI_Alloc(size, 4);
    memcpy((unsigned __int8 *)menu->items, (unsigned __int8 *)g_load_0.items, size);
    if ( menu->fullScreen )
    {
        menu->window.rect.x = 0.0f;
        menu->window.rect.y = 0.0f;
        menu->window.rect.w = 640.0f;
        menu->window.rect.h = 480.0f;
    }
    Menu_UpdatePosition(0, menu);
}

MenuList *__cdecl UI_LoadMenus(const char *menuFile, int imageTrack)
{
    if ( G_ExitAfterToolComplete() )
        return 0;
    if ( useFastFile->current.enabled )
        return (MenuList *)((int (__cdecl *)(const char *, int))UI_LoadMenus_FastFile)(menuFile, imageTrack);
    return (MenuList *)((int (__cdecl *)(const char *, int))UI_LoadMenus_LoadObj)(menuFile, imageTrack);
}

char menuBuf[32768];
MenuList * UI_LoadMenus_LoadObj(char *menuFile, int imageTrack)
{
    int len; // [esp+0h] [ebp-10h]
    int f; // [esp+4h] [ebp-Ch] BYREF
    const char *token; // [esp+8h] [ebp-8h]
    const char *p; // [esp+Ch] [ebp-4h] BYREF

    memset(&g_load_0, 0, sizeof(g_load_0));
    g_load_0.menuList.menus = g_load_0.menus;
    len = FS_FOpenFileByMode(menuFile, &f, FS_READ);
    if ( !f )
    {
        Com_Printf(13, "^3WARNING: menu file not found: %s\n", menuFile);
        len = FS_FOpenFileByMode((char*)"ui/default.menu", &f, FS_READ);
        if ( !f )
            Com_Error(ERR_DROP, "default.menu file not found. This is a default menu that you should have.");
    }
    if ( len >= 0x8000 )
    {
        FS_FCloseFile(f);
        Com_Error(ERR_DROP, "^1menu file too large: %s is %i, max allowed is %i", menuFile, len, 0x8000);
    }
    FS_Read((unsigned __int8 *)menuBuf, len, f);
    menuBuf[len] = 0;
    FS_FCloseFile(f);
    Com_Compress(menuBuf);
    p = menuBuf;
    Com_BeginParseSession(menuFile);
    do
        token = (const char *)Com_Parse(&p);
    while ( token
             && *token
             && *token != 125
             && I_stricmp(token, "}")
             && (I_stricmp(token, "loadmenu") || Load_Menu(&p, imageTrack)) );
    Com_EndParseSession();
    return &g_load_0.menuList;
}

int    Load_Menu(const char **p, int imageTrack)
{
    parseInfo_t *token; // [esp+0h] [ebp-4h]

    if ( Com_Parse(p)->token[0] != 123 )
        return 0;
    while ( 1 )
    {
        token = Com_Parse(p);
        if ( !I_stricmp(token->token, "}") )
            return 1;
        if ( !token || !token->token[0] )
            break;
        UI_ParseMenuInternal(token->token, imageTrack);
    }
    return 0;
}

MenuList *__cdecl UI_LoadMenus_FastFile(const char *menuFile)
{
    return DB_FindXAssetHeader(ASSET_TYPE_MENULIST, (char*)menuFile, 1, -1).menuList;
}

int __cdecl KeywordHash_KeySeed(const char *keyword, int hashCount, int seed)
{
    int hash; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    hash = 0;
    for ( i = 0; keyword[i]; ++i )
        hash += (i + seed) * tolower(keyword[i]);
    return (hash + (hash >> 8)) % hashCount;
}

