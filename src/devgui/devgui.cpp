#include "devgui.h"
#include <universal/assertive.h>
#include <qcommon/common.h>
#include <string.h>
#include <universal/com_memory.h>
#include <clientscript/cscr_main.h>
#include <qcommon/cmd.h>
#include <gfx_d3d/r_font.h>
#include <client_mp/cl_main_mp.h>
#include <client/cl_keys.h>
#include "devgui_util.h"
#include "devgui_input.h"
#include <universal/com_pack.h>

devguiGlob_t devguiGlob;

const dvar_t *devgui_colorBgnd;
const dvar_t *devgui_colorText;
const dvar_t *devgui_colorBgndSel;
const dvar_t *devgui_colorTextSel;
const dvar_t *devgui_colorBgndGray;
const dvar_t *devgui_colorTextGray;
const dvar_t *devgui_colorBgndGraySel;
const dvar_t *devgui_colorTextGraySel;
const dvar_t *devgui_colorSliderBgnd;
const dvar_t *devgui_colorSliderKnob;
const dvar_t *devgui_colorSliderKnobSel;
const dvar_t *devgui_bevelShade;
const dvar_t *devgui_colorGraphKnotNormal;
const dvar_t *devgui_colorGraphKnotSelected;
const dvar_t *devgui_colorGraphKnotEditing;
const dvar_t *devgui_zoomEnabled;

void __cdecl DevGui_AddDvar(const char *path, const dvar_s *dvar)
{
    unsigned __int16 handle; // [esp+0h] [ebp-8h]
    devguiGlob_t *menu; // [esp+4h] [ebp-4h]

    if ( !path && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 543, 0, "%s", "path") )
        __debugbreak();
    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 544, 0, "%s", "dvar") )
        __debugbreak();
    if ( DevGui_IsValidPath(path) )
    {
        handle = DevGui_ConstructPath_r(0, path);
        menu = DevGui_GetMenu(handle);
        if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 551, 0, "%s", "menu") )
            __debugbreak();
        if ( !menu->menus[0].childType && !menu->menus[0].child.menu
            || menu->menus[0].childType == 1 && menu->menus[0].child.dvar == dvar )
        {
            menu->menus[0].childType = 1;
            menu->menus[0].child.command = (const char *)dvar;
        }
        else
        {
            Com_Printf(
                11,
                "Path '%s' can't be used for dvar '%s' because it is already used for something else.\n",
                path,
                dvar->name);
        }
    }
}

devguiGlob_t *__cdecl DevGui_GetMenu(unsigned __int16 handle)
{
    if ( (!handle || handle > 0x800u)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    214,
                    0,
                    "handle not in [1, ARRAY_COUNT( devguiGlob.menus )]\n\t%i not in [%i, %i]",
                    handle,
                    1,
                    2048) )
    {
        __debugbreak();
    }
    return (devguiGlob_t *)((char *)&devguiGlob + 40 * handle - 40);
}

unsigned __int16 __cdecl DevGui_ConstructPath_r(unsigned __int16 parent, const char *path)
{
    char label[28]; // [esp+0h] [ebp-28h] BYREF
    DevGuiTokenResult tokResult; // [esp+20h] [ebp-8h]
    __int16 sortKey; // [esp+24h] [ebp-4h] BYREF

    do
    {
        tokResult = DevGui_PathToken(&path, label, &sortKey);
        if ( tokResult == DEVGUI_TOKEN_ERROR
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                        493,
                        0,
                        "%s",
                        "tokResult != DEVGUI_TOKEN_ERROR") )
        {
            __debugbreak();
        }
        parent = DevGui_RegisterMenu(parent, label, sortKey);
    }
    while ( tokResult != DEVGUI_TOKEN_LAST );
    return parent;
}

unsigned __int16 __cdecl DevGui_RegisterMenu(unsigned __int16 parentHandle, const char *label, __int16 sortKey)
{
    uint16_t childHandle; // [esp+0h] [ebp-4h]

    childHandle = DevGui_FindMenu(parentHandle, label);
    if (!childHandle)
        return DevGui_CreateMenu(parentHandle, label, sortKey);
    return childHandle;
}

int __cdecl DevGui_CreateMenu(uint16_t parentHandle, const char *label, __int16 sortKey)
{
    if (IsDedicatedServer())
    {
        return 0;
    }

    char v5; // [esp+3h] [ebp-25h]
    DevMenuItem *v6; // [esp+8h] [ebp-20h]
    unsigned __int16 handle; // [esp+10h] [ebp-18h]
    unsigned __int16 *prevNext; // [esp+14h] [ebp-14h]
    DevMenuItem *menu; // [esp+18h] [ebp-10h]
    unsigned __int16 prev; // [esp+1Ch] [ebp-Ch]
    devguiGlob_t *nextMenu; // [esp+20h] [ebp-8h]
    devguiGlob_t *parentMenu; // [esp+24h] [ebp-4h]

    menu = devguiGlob.nextFreeMenu;
    if (!devguiGlob.nextFreeMenu)
        Com_Error(ERR_DROP, "Too many devgui entries (more than %i)", 2048);
    devguiGlob.nextFreeMenu = *(DevMenuItem **)menu->label;
    handle = DevGui_GetMenuHandle(menu);
    v6 = menu;
    do
    {
        v5 = *label;
        v6->label[0] = *label++;
        v6 = (DevMenuItem *)((char *)v6 + 1);
    } while (v5);
    menu->childType = 0;
    menu->childMenuMemory = 0;
    menu->sortKey = sortKey;
    menu->parent = parentHandle;
    menu->child.menu = 0;
    if (parentHandle)
        parentMenu = DevGui_GetMenu(parentHandle);
    else
        parentMenu = (devguiGlob_t *)&devguiGlob.topmostMenu;
    prev = 0;
    for (prevNext = (unsigned __int16 *)&parentMenu->menus[0].child; *prevNext; prevNext = &nextMenu->menus[0].nextSibling)
    {
        nextMenu = DevGui_GetMenu(*prevNext);
        if (!DevGui_CompareMenus(nextMenu->menus, menu)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                350,
                0,
                "%s",
                "DevGui_CompareMenus( nextMenu, menu ) != 0"))
        {
            __debugbreak();
        }
        if (DevGui_CompareMenus(nextMenu->menus, menu) > 0)
            break;
        prev = *prevNext;
    }
    menu->nextSibling = *prevNext;
    menu->prevSibling = prev;
    *prevNext = handle;
    if (menu->nextSibling)
        DevGui_GetMenu(menu->nextSibling)->menus[0].prevSibling = handle;
    return handle;
}

unsigned __int16 __cdecl DevGui_GetMenuHandle(DevMenuItem *menu)
{
    unsigned __int16 handle; // [esp+0h] [ebp-4h]

    handle = ((char *)menu - (char *)&devguiGlob) / 40 + 1;
    if ( ((unsigned __int16)(((char *)menu - (char *)&devguiGlob) / 40) == 0xFFFF || handle > 0x800u)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    233,
                    0,
                    "handle not in [1, ARRAY_COUNT( devguiGlob.menus )]\n\t%i not in [%i, %i]",
                    handle,
                    1,
                    2048) )
    {
        __debugbreak();
    }
    return ((char *)menu - (char *)&devguiGlob) / 40 + 1;
}

int __cdecl DevGui_CompareMenus(const DevMenuItem *menu0, const DevMenuItem *menu1)
{
    if ( !menu0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 271, 0, "%s", "menu0") )
        __debugbreak();
    if ( !menu1 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 272, 0, "%s", "menu1") )
        __debugbreak();
    if ( menu0->sortKey == menu1->sortKey )
        return I_stricmp(menu0->label, menu1->label);
    else
        return menu0->sortKey - menu1->sortKey;
}

unsigned __int16 __cdecl DevGui_FindMenu(unsigned __int16 parentHandle, const char *label)
{
    devguiGlob_t *childMenu; // [esp+0h] [ebp-Ch]
    unsigned __int16 childHandle; // [esp+4h] [ebp-8h]
    devguiGlob_t *parentMenu; // [esp+8h] [ebp-4h]

    if ( parentHandle )
        parentMenu = DevGui_GetMenu(parentHandle);
    else
        parentMenu = (devguiGlob_t *)&devguiGlob.topmostMenu;
    if ( parentMenu->menus[0].childType
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    377,
                    0,
                    "%s\n\t(parentMenu->childType) = %i",
                    "(parentMenu->childType == DEV_CHILD_MENU)",
                    parentMenu->menus[0].childType) )
    {
        __debugbreak();
    }
    for ( childHandle = parentMenu->menus[0].child.menu; childHandle; childHandle = childMenu->menus[0].nextSibling )
    {
        childMenu = DevGui_GetMenu(childHandle);
        if ( !childMenu
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 382, 0, "%s", "childMenu") )
        {
            __debugbreak();
        }
        if ( !I_stricmp(label, (const char *)childMenu) )
            return childHandle;
    }
    return 0;
}

DevGuiTokenResult __cdecl DevGui_PathToken(const char **pathInOut, char *label, __int16 *sortKeyOut)
{
    const char *path; // [esp+0h] [ebp-10h]
    __int16 sign; // [esp+4h] [ebp-Ch]
    int labelLen; // [esp+8h] [ebp-8h]
    __int16 sortKey; // [esp+Ch] [ebp-4h]

    if ( !pathInOut
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 409, 0, "%s", "pathInOut") )
    {
        __debugbreak();
    }
    if ( !*pathInOut
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 410, 0, "%s", "*pathInOut") )
    {
        __debugbreak();
    }
    if ( !label && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 411, 0, "%s", "label") )
        __debugbreak();
    if ( !sortKeyOut
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 412, 0, "%s", "sortKeyOut") )
    {
        __debugbreak();
    }
    path = *pathInOut;
    labelLen = 0;
    *sortKeyOut = 0x7FFF;
    while ( *path )
    {
        if ( *path == 58 )
        {
            label[labelLen] = 0;
            ++path;
            sortKey = 0;
            sign = 1;
            if ( *path == 45 )
            {
                sign = -1;
                ++path;
            }
            else if ( *path == 43 )
            {
                ++path;
            }
            if ( *path < 48 || *path > 57 )
                return DEVGUI_TOKEN_ERROR;
            do
                sortKey = 10 * sortKey + *path++ - 48;
            while ( *path >= 48 && *path <= 57 );
            *sortKeyOut = sign * sortKey;
            if ( *path && (*path != 47 || path[1] == 47) )
                return DEVGUI_TOKEN_ERROR;
        }
        else
        {
            if ( *path == 47 && *++path != 47 )
            {
                label[labelLen] = 0;
                *pathInOut = path;
                return DEVGUI_TOKEN_MORE;
            }
            if ( labelLen < 25 )
                label[labelLen++] = *path;
            ++path;
        }
    }
    if ( !labelLen )
        return DEVGUI_TOKEN_ERROR;
    *pathInOut = path;
    label[labelLen] = 0;
    return DEVGUI_TOKEN_LAST;
}

char __cdecl DevGui_IsValidPath(const char *path)
{
    char label[28]; // [esp+20h] [ebp-2Ch] BYREF
    const char *originalPath; // [esp+40h] [ebp-Ch]
    DevGuiTokenResult tokResult; // [esp+44h] [ebp-8h]
    __int16 sortKey; // [esp+48h] [ebp-4h] BYREF

    if ( !path && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 508, 0, "%s", "path") )
        __debugbreak();
    if ( strlen(path) <= 0x78 )
    {
        originalPath = path;
        tokResult = DevGui_PathToken(&path, label, &sortKey);
        if ( tokResult == DEVGUI_TOKEN_LAST )
        {
            Com_Printf(11, "Path '%s' must have at least one menu separator ('/' character).\n", originalPath);
            return 0;
        }
        else
        {
            do
            {
                if ( tokResult == DEVGUI_TOKEN_ERROR )
                {
                    Com_Printf(
                        11,
                        "path '%s' is invalid.    Format is 'menu name:sortkey/submenu/...', where 'sortkey' is any (possibly signed) integer.\n",
                        originalPath);
                    return 0;
                }
                tokResult = DevGui_PathToken(&path, label, &sortKey);
            }
            while ( tokResult != DEVGUI_TOKEN_LAST );
            return 1;
        }
    }
    else
    {
        Com_Printf(11, "Path '%s' must be no longer than %i characters (currently %i).\n", path, 120, strlen(path));
        return 0;
    }
}

void __cdecl DevGui_AddCommand(const char *path, char *command)
{
    unsigned __int16 handle; // [esp+0h] [ebp-8h]
    devguiGlob_t *menu; // [esp+4h] [ebp-4h]

    if ( !path && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 568, 0, "%s", "path") )
        __debugbreak();
    if ( !command && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 569, 0, "%s", "command") )
        __debugbreak();
    if ( DevGui_IsValidPath(path) )
    {
        handle = DevGui_ConstructPath_r(0, path);
        menu = DevGui_GetMenu(handle);
        if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 576, 0, "%s", "menu") )
            __debugbreak();
        if ( !menu->menus[0].childType && !menu->menus[0].child.menu
            || menu->menus[0].childType == 2 && menu->menus[0].child.command == command )
        {
            menu->menus[0].childType = 2;
            menu->menus[0].child.command = CopyString(command, "DevGui_AddCommand", 0, SCRIPTINSTANCE_SERVER);
        }
        else
        {
            Com_Printf(
                11,
                "Path '%s' can't be used for command '%s' because it is already used for something else.\n",
                path,
                command);
        }
    }
}

void __cdecl DevGui_AddGraph(const char *path, DevGraph *graph)
{
    if ( !graph && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 593, 0, "%s", "graph") )
        __debugbreak();
    if ( !graph->knots
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 593, 0, "%s", "graph->knots") )
    {
        __debugbreak();
    }
    if ( !graph->knotCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 593, 0, "%s", "graph->knotCount") )
    {
        __debugbreak();
    }
    if ( graph->knotCountMax <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    593,
                    0,
                    "%s",
                    "graph->knotCountMax > 0") )
    {
        __debugbreak();
    }
    if ( !path && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 594, 0, "%s", "path") )
        __debugbreak();


    if (DevGui_IsInitialized())
        return;

    if (IsDedicatedServer())
        return;

    if (DevGui_IsValidPath(path))
    {
        uint16_t handle; // [esp+0h] [ebp-8h]
        devguiGlob_t *menu; // [esp+4h] [ebp-4h]

        handle = DevGui_ConstructPath_r(0, path);
        menu = DevGui_GetMenu(handle);

        iassert(menu);

        if (!menu->menus[0].childType && !menu->menus[0].child.menu
            || menu->menus[0].childType == 3 && menu->menus[0].child.graph == graph)
        {
            menu->menus[0].childType = 3;
            menu->menus[0].child.command = (const char *)graph;
        }
        else
        {
            Com_Printf(11, "Path '%s' can't be added for this graph because it is already used for something else.\n", path);
        }
    }
}

void __cdecl DevGui_RemoveMenu(const char *path)
{
    const char *v1; // eax
    unsigned __int16 handle; // [esp+0h] [ebp-38h]
    char label[28]; // [esp+4h] [ebp-34h] BYREF
    DevMenuItem *parent; // [esp+24h] [ebp-14h]
    DevMenuItem *menu; // [esp+28h] [ebp-10h]
    DevMenuItem *sibling; // [esp+2Ch] [ebp-Ch]
    DevGuiTokenResult tokResult; // [esp+30h] [ebp-8h]
    __int16 sortKey; // [esp+34h] [ebp-4h] BYREF

    if ( DevGui_IsValidPath(path) )
    {
        handle = 0;
        while ( 1 )
        {
            tokResult = DevGui_PathToken(&path, label, &sortKey);
            if ( tokResult == DEVGUI_TOKEN_ERROR
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                            693,
                            0,
                            "%s",
                            "tokResult != DEVGUI_TOKEN_ERROR") )
            {
                __debugbreak();
            }
            handle = DevGui_FindMenu(handle, label);
            if ( !handle )
                break;
            if ( tokResult == DEVGUI_TOKEN_LAST )
            {
                menu = (DevMenuItem *)DevGui_GetMenu(handle);
                if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 703, 0, "%s", "menu") )
                    __debugbreak();
                if ( menu->nextSibling )
                {
                    sibling = (DevMenuItem *)DevGui_GetMenu(menu->nextSibling);
                    sibling->prevSibling = menu->prevSibling;
                }
                if ( menu->prevSibling )
                {
                    sibling = (DevMenuItem *)DevGui_GetMenu(menu->prevSibling);
                    sibling->nextSibling = menu->nextSibling;
                }
                else
                {
                    if ( !menu->parent
                        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 716, 0, "%s", "menu->parent") )
                    {
                        __debugbreak();
                    }
                    parent = (DevMenuItem *)DevGui_GetMenu(menu->parent);
                    if ( !parent
                        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 718, 0, "%s", "parent") )
                    {
                        __debugbreak();
                    }
                    if ( parent->childType )
                    {
                        v1 = va("menu %s type %i", parent->label, parent->childType);
                        if ( !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                                        719,
                                        0,
                                        "%s\n\t%s",
                                        "parent->childType == DEV_CHILD_MENU",
                                        v1) )
                            __debugbreak();
                    }
                    parent->child.menu = menu->nextSibling;
                }
                menu->nextSibling = 0;
                menu->prevSibling = 0;
                DevGui_FreeMenu_r(handle);
                return;
            }
        }
    }
}

void __cdecl DevGui_FreeMenu_r(unsigned __int16 handle)
{
    devguiGlob_t *menu; // [esp+0h] [ebp-4h]

    if ( handle )
    {
        if ( devguiGlob.selectedMenu == handle )
        {
            devguiGlob.selectedMenu = 0;
            devguiGlob.isActive = 0;
            devguiGlob.editingMenuItem = 0;
        }
        menu = DevGui_GetMenu(handle);
        if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 295, 0, "%s", "menu") )
            __debugbreak();
        if ( menu->menus[0].childType == 2 )
        {
            FreeString(menu->menus[0].child.command, 0, SCRIPTINSTANCE_SERVER);
            menu->menus[0].child.command = 0;
        }
        else if ( !menu->menus[0].childType )
        {
            DevGui_FreeMenu_r(menu->menus[0].child.menu);
        }
        DevGui_FreeMenu_r(menu->menus[0].nextSibling);
        *(unsigned int *)menu->menus[0].label = (unsigned int)devguiGlob.nextFreeMenu;
        devguiGlob.nextFreeMenu = (DevMenuItem *)menu;
    }
}

void __cdecl DevGui_OpenMenu(const char *path)
{
    unsigned __int16 handle; // [esp+0h] [ebp-30h]
    char label[28]; // [esp+4h] [ebp-2Ch] BYREF
    DevMenuItem *menu; // [esp+24h] [ebp-Ch]
    DevGuiTokenResult tokResult; // [esp+28h] [ebp-8h]
    __int16 sortKey; // [esp+2Ch] [ebp-4h] BYREF

    if ( DevGui_IsValidPath(path) )
    {
        handle = 0;
        while ( 1 )
        {
            tokResult = DevGui_PathToken(&path, label, &sortKey);
            if ( tokResult == DEVGUI_TOKEN_ERROR
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                            744,
                            0,
                            "%s",
                            "tokResult != DEVGUI_TOKEN_ERROR") )
            {
                __debugbreak();
            }
            handle = DevGui_FindMenu(handle, label);
            if ( !handle )
                break;
            if ( tokResult == DEVGUI_TOKEN_LAST )
            {
                menu = (DevMenuItem *)DevGui_GetMenu(handle);
                if ( menu->childType == 2 )
                {
                    Cbuf_InsertText(0, (char *)menu->child.command);
                }
                else
                {
                    devguiGlob.isActive = 1;
                    devguiGlob.selectedMenu = handle;
                    devguiGlob.editingMenuItem = DevGui_EditableMenuItem(menu);
                    if ( !devguiGlob.editingMenuItem && !menu->childType )
                        devguiGlob.selectedMenu = menu->child.menu;
                    devguiGlob.selRow = 0;
                }
                return;
            }
        }
    }
}

bool __cdecl DevGui_EditableMenuItem(const DevMenuItem *menu)
{
    if ( menu->childType == 3 )
        return 1;
    if ( menu->childType != 1 )
        return 0;
    if ( *((unsigned int *)menu->child.command + 4) == 7 )
        return 0;
    return *((unsigned int *)menu->child.command + 4) != 6 || *((unsigned int *)menu->child.command + 22);
}

void __cdecl DevGui_Draw(int localClientNum)
{
    int origin[2]; // [esp+0h] [ebp-10h] BYREF
    DevMenuItem *menuItem; // [esp+8h] [ebp-8h]
    unsigned __int16 parent; // [esp+Ch] [ebp-4h]

    if ( devguiGlob.isActive )
    {
        menuItem = (DevMenuItem *)DevGui_GetMenu(devguiGlob.selectedMenu);
        if ( devguiGlob.editingMenuItem )
        {
            if ( menuItem->childType != 3
                && menuItem->childType != 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                            1549,
                            0,
                            "%s\n\t(menuItem->childType) = %i",
                            "(menuItem->childType == DEV_CHILD_GRAPH || menuItem->childType == DEV_CHILD_DVAR)",
                            menuItem->childType) )
            {
                __debugbreak();
            }
            if ( menuItem->childType == 1 )
                DevGui_DrawSliders(menuItem);
            else
                DevGui_DrawGraph(menuItem, localClientNum);
        }
        else
        {
            origin[0] = devguiGlob.left;
            origin[1] = devguiGlob.top;
            parent = DevGui_GetMenuParent(devguiGlob.selectedMenu);
            DevGui_ChooseOrigin(origin);
            DevGui_DrawMenu(parent, devguiGlob.selectedMenu, origin);
        }
        if ( devguiGlob.bindNextKey )
            DevGui_DrawBindNextKey();
    }
}

unsigned __int16 __cdecl DevGui_GetMenuParent(unsigned __int16 handle)
{
    return DevGui_GetMenu(handle)->menus[0].parent;
}

void __cdecl DevGui_DrawMenu(unsigned __int16 menuHandle, unsigned __int16 activeChild, int *origin)
{
    devguiGlob_t *menu; // [esp+0h] [ebp-4h]

    if ( !origin && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 1069, 0, "%s", "origin") )
        __debugbreak();
    menu = DevGui_GetMenu(menuHandle);
    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 1072, 0, "%s", "menu") )
        __debugbreak();
    if ( menu->menus[0].parent )
        DevGui_DrawMenu(menu->menus[0].parent, menuHandle, origin);
    else
        DevGui_DrawMenuHorizontally(&devguiGlob.topmostMenu, menuHandle, origin);
    DevGui_DrawMenuVertically(menu->menus, activeChild, origin);
}

void __cdecl DevGui_DrawMenuVertically(const DevMenuItem *menu, unsigned __int16 activeChild, int *origin)
{
    int v3; // esi
    devguiGlob_t *childMenu; // [esp+Ch] [ebp-50h]
    devguiGlob_t *childMenua; // [esp+Ch] [ebp-50h]
    int origW; // [esp+10h] [ebp-4Ch]
    float fontYScale; // [esp+14h] [ebp-48h]
    int activeChildIndex; // [esp+18h] [ebp-44h]
    int origH; // [esp+1Ch] [ebp-40h]
    unsigned __int8 bgndColor[4]; // [esp+24h] [ebp-38h] BYREF
    float fontXScale; // [esp+28h] [ebp-34h]
    int fontYOffset; // [esp+2Ch] [ebp-30h]
    unsigned __int8 textColor[4]; // [esp+30h] [ebp-2Ch] BYREF
    int subMenuStringPos; // [esp+34h] [ebp-28h]
    int visibleMenuCount; // [esp+38h] [ebp-24h]
    int x; // [esp+3Ch] [ebp-20h]
    int y; // [esp+40h] [ebp-1Ch]
    int fontXOffset; // [esp+44h] [ebp-18h]
    int h; // [esp+48h] [ebp-14h]
    float shade; // [esp+4Ch] [ebp-10h]
    int childCount; // [esp+50h] [ebp-Ch]
    unsigned __int16 childHandle; // [esp+54h] [ebp-8h]
    int w; // [esp+58h] [ebp-4h]

    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 889, 0, "%s", "menu") )
        __debugbreak();
    x = *origin;
    y = origin[1];
    origW = DevGui_MaxChildMenuWidth(menu);
    origH = R_TextHeight(cls.consoleFont) + 8;
    w = origW;
    h = origH;
    fontXScale = 1.0f;
    fontYScale = 1.0f;
    fontXOffset = 4;
    fontYOffset = 4;
    childCount = 0;
    activeChildIndex = 0;
    for ( childHandle = menu->child.menu; childHandle; childHandle = childMenu->menus[0].nextSibling )
    {
        childMenu = DevGui_GetMenu(childHandle);
        if ( childHandle == activeChild )
            activeChildIndex = childCount;
        ++childCount;
    }
    visibleMenuCount = (devguiGlob.bottom - 2 * h - y) / h;
    if ( visibleMenuCount < childCount && activeChildIndex > visibleMenuCount )
    {
        *(unsigned int *)bgndColor = devgui_colorBgnd->current.integer;
        *(unsigned int *)textColor = devgui_colorText->current.integer;
        shade = devgui_bevelShade->current.value;
        DevGui_DrawBevelBox(x, y, w, h, shade, bgndColor);
        DevGui_DrawFont(fontXOffset + x, fontYOffset + y, textColor, (char*)"...", fontXScale, 1.0);
        y += h;
    }
    childCount = 0;
    for ( childHandle = menu->child.menu; childHandle; childHandle = childMenua->menus[0].nextSibling )
    {
        childMenua = DevGui_GetMenu(childHandle);
        if ( activeChildIndex - visibleMenuCount <= childCount )
        {
            if ( childHandle == activeChild )
            {
                *origin = w + x;
                origin[1] = y;
                if ( DevGui_MenuItemDisabled(childMenua->menus) )
                {
                    *(unsigned int *)bgndColor = devgui_colorBgndGraySel->current.integer;
                    *(unsigned int *)textColor = devgui_colorTextGraySel->current.integer;
                }
                else
                {
                    *(unsigned int *)bgndColor = devgui_colorBgndSel->current.integer;
                    *(unsigned int *)textColor = devgui_colorTextSel->current.integer;
                }
                if ( childHandle == devguiGlob.selectedMenu && DevGui_IsButtonDown(INPUT_ACCEPT) )
                    shade = 2.0 - devgui_bevelShade->current.value;
                else
                    shade = devgui_bevelShade->current.value;
                if ( devgui_zoomEnabled->current.enabled )
                {
                    w = (int)(float)((float)w * 1.5);
                    h *= 2;
                    fontXScale = fontXScale * 1.5;
                    fontYScale = fontYScale * 1.5;
                    fontXOffset += 4;
                    fontYOffset += 14;
                }
            }
            else
            {
                if ( devgui_zoomEnabled->current.enabled )
                {
                    w = origW;
                    h = origH;
                    fontXScale = 1.0f;
                    fontYScale = 1.0f;
                    fontXOffset = 4;
                    fontYOffset = 4;
                }
                if ( DevGui_MenuItemDisabled(childMenua->menus) )
                {
                    *(unsigned int *)bgndColor = devgui_colorBgndGray->current.integer;
                    *(unsigned int *)textColor = devgui_colorTextGray->current.integer;
                }
                else
                {
                    *(unsigned int *)bgndColor = devgui_colorBgnd->current.integer;
                    *(unsigned int *)textColor = devgui_colorText->current.integer;
                }
                shade = devgui_bevelShade->current.value;
            }
            v3 = x + w - 4 - fontXOffset;
            subMenuStringPos = v3 - DevGui_SubMenuTextWidth();
            DevGui_DrawBevelBox(x, y, w, h, shade, bgndColor);
            DevGui_DrawFont(fontXOffset + x, fontYOffset + y, textColor, (char *)childMenua, fontXScale, fontYScale);
            if ( !childMenua->menus[0].childType && childMenua->menus[0].child.menu )
                DevGui_DrawFont(subMenuStringPos, fontYOffset + y, textColor, (char*)" >", fontXScale, fontYScale);
            y += h;
            ++childCount;
        }
        else
        {
            ++childCount;
        }
    }
}

bool __cdecl DevGui_MenuItemDisabled(const DevMenuItem *menu)
{
    return menu->childType == 1 && !DevGui_EditableMenuItem(menu);
}

int __cdecl DevGui_SubMenuTextWidth()
{
    return R_TextWidth(" >", 0, cls.consoleFont);
}

int __cdecl DevGui_MaxChildMenuWidth(const DevMenuItem *menu)
{
    devguiGlob_t *childMenu; // [esp+0h] [ebp-10h]
    int widthCur; // [esp+4h] [ebp-Ch]
    int widthMax; // [esp+8h] [ebp-8h]
    unsigned __int16 childHandle; // [esp+Ch] [ebp-4h]

    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 795, 0, "%s", "menu") )
        __debugbreak();
    if ( menu->childType
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    796,
                    0,
                    "%s",
                    "menu->childType == DEV_CHILD_MENU") )
    {
        __debugbreak();
    }
    widthMax = DevGui_MenuItemWidth(menu);
    for ( childHandle = menu->child.menu; childHandle; childHandle = childMenu->menus[0].nextSibling )
    {
        childMenu = DevGui_GetMenu(childHandle);
        widthCur = DevGui_MenuItemWidth(childMenu->menus);
        if ( widthMax < widthCur )
            widthMax = widthCur;
    }
    return widthMax;
}

int __cdecl DevGui_MenuItemWidth(const DevMenuItem *menu)
{
    int width; // [esp+0h] [ebp-4h]

    width = R_TextWidth(menu->label, 0, cls.consoleFont) + 8;
    if ( !menu->childType && menu->child.menu )
        width += DevGui_SubMenuTextWidth();
    return width;
}

void __cdecl DevGui_DrawMenuHorizontally(const DevMenuItem *menu, unsigned __int16 activeChild, int *origin)
{
    devguiGlob_t *childMenu; // [esp+8h] [ebp-20h]
    unsigned __int8 bgndColor[4]; // [esp+Ch] [ebp-1Ch] BYREF
    unsigned __int8 textColor[4]; // [esp+10h] [ebp-18h] BYREF
    int x; // [esp+14h] [ebp-14h]
    int y; // [esp+18h] [ebp-10h]
    int h; // [esp+1Ch] [ebp-Ch]
    unsigned __int16 childHandle; // [esp+20h] [ebp-8h]
    int w; // [esp+24h] [ebp-4h]

    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 1025, 0, "%s", "menu") )
        __debugbreak();
    x = *origin;
    y = origin[1];
    h = R_TextHeight(cls.consoleFont) + 8;
    for ( childHandle = menu->child.menu; childHandle; childHandle = childMenu->menus[0].nextSibling )
    {
        childMenu = DevGui_GetMenu(childHandle);
        w = DevGui_MenuItemWidth(childMenu->menus);
        if ( childHandle == activeChild )
        {
            *origin = x;
            origin[1] = h + y;
            *(unsigned int *)bgndColor = devgui_colorBgndSel->current.integer;
            *(unsigned int *)textColor = devgui_colorTextSel->current.integer;
        }
        else
        {
            *(unsigned int *)bgndColor = devgui_colorBgnd->current.integer;
            *(unsigned int *)textColor = devgui_colorText->current.integer;
        }
        DevGui_DrawBevelBox(x, y, w, h, devgui_bevelShade->current.value, bgndColor);
        DevGui_DrawFont(x + 4, y + 4, textColor, (char *)childMenu, 1.0, 1.0);
        x += w;
    }
}

void __cdecl DevGui_ChooseOrigin(int *origin)
{
    unsigned __int16 handle; // [esp+0h] [ebp-18h]
    devguiGlob_t *childMenu; // [esp+4h] [ebp-14h]
    devguiGlob_t *menu; // [esp+8h] [ebp-10h]
    unsigned __int16 activeChild; // [esp+Ch] [ebp-Ch]
    unsigned __int16 childHandle; // [esp+10h] [ebp-8h]
    int w; // [esp+14h] [ebp-4h]

    w = 0;
    activeChild = devguiGlob.selectedMenu;
    for ( handle = DevGui_GetMenuParent(devguiGlob.selectedMenu); handle; handle = menu->menus[0].parent )
    {
        menu = DevGui_GetMenu(handle);
        if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 1096, 0, "%s", "menu") )
            __debugbreak();
        w += DevGui_MaxChildMenuWidth(menu->menus);
        activeChild = handle;
    }
    for ( childHandle = devguiGlob.topmostMenu.child.menu;
                childHandle != activeChild;
                childHandle = childMenu->menus[0].nextSibling )
    {
        childMenu = DevGui_GetMenu(childHandle);
        w += DevGui_MenuItemWidth(childMenu->menus);
    }
    *origin = devguiGlob.left;
    origin[1] = devguiGlob.top;
    if ( w > devguiGlob.right - devguiGlob.left )
        *origin -= w - (devguiGlob.right - devguiGlob.left);
}

void __cdecl DevGui_DrawSliders(const DevMenuItem *menu)
{
    const char *v1; // eax
    const char *v2; // eax
    float v3; // [esp+8h] [ebp-BCh]
    const DvarValue *v4; // [esp+Ch] [ebp-B8h]
    const DvarValue *v5; // [esp+10h] [ebp-B4h]
    const DvarValue *v6; // [esp+14h] [ebp-B0h]
    double v7; // [esp+18h] [ebp-ACh]
    double v8; // [esp+20h] [ebp-A4h]
    double v9; // [esp+28h] [ebp-9Ch]
    double v10; // [esp+30h] [ebp-94h]
    double v11; // [esp+38h] [ebp-8Ch]
    double v12; // [esp+40h] [ebp-84h]
    const DvarValue *p_current; // [esp+48h] [ebp-7Ch]
    unsigned __int8 color[4]; // [esp+58h] [ebp-6Ch] BYREF
    double G; // [esp+5Ch] [ebp-68h]
    double X; // [esp+64h] [ebp-60h]
    double Y; // [esp+6Ch] [ebp-58h]
    unsigned __int8 gammaColor[4]; // [esp+78h] [ebp-4Ch] BYREF
    double R; // [esp+7Ch] [ebp-48h]
    double B; // [esp+84h] [ebp-40h]
    double Z; // [esp+8Ch] [ebp-38h]
    int rowHeight; // [esp+98h] [ebp-2Ch]
    const unsigned __int8 *knobColor; // [esp+9Ch] [ebp-28h]
    int width; // [esp+A0h] [ebp-24h]
    int height; // [esp+A4h] [ebp-20h]
    float fraction; // [esp+A8h] [ebp-1Ch]
    int rowCount; // [esp+ACh] [ebp-18h]
    int row; // [esp+B0h] [ebp-14h]
    const dvar_s *dvar; // [esp+B4h] [ebp-10h]
    int x; // [esp+B8h] [ebp-Ch]
    int y; // [esp+BCh] [ebp-8h]
    int rowWidth; // [esp+C0h] [ebp-4h]

    if ( menu->childType != 1 )
    {
        v1 = va("menu %s type %i", menu->label, menu->childType);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                        1254,
                        0,
                        "%s\n\t%s",
                        "menu->childType == DEV_CHILD_DVAR",
                        v1) )
            __debugbreak();
    }
    dvar = menu->child.dvar;
    if ( !dvar
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    1256,
                    0,
                    "%s\n\t(menu->label) = %s",
                    "(dvar)",
                    menu->label) )
    {
        __debugbreak();
    }
    rowWidth = devguiGlob.sliderWidth;
    rowHeight = DevGui_GetFontHeight();
    rowCount = DevGui_DvarRowCount(dvar);
    width = rowWidth + 8;
    height = rowHeight * (rowCount + 3) + 2 * rowCount + 16;
    if ( dvar->type == DVAR_TYPE_COLOR || dvar->type == DVAR_TYPE_LINEAR_COLOR_RGB || dvar->type == DVAR_TYPE_COLOR_XYZ )
        height += rowHeight + 2;
    x = devguiGlob.left + (devguiGlob.right - devguiGlob.left - width) / 2;
    y = devguiGlob.bottom - height;
    DevGui_DrawBevelBox(
        x,
        devguiGlob.bottom - height,
        width,
        height,
        devgui_bevelShade->current.value,
        (const unsigned __int8 *)&devgui_colorBgnd->current);
    x += 4;
    y += 6;
    DevGui_DrawSliderPath(x, y);
    y += rowHeight + 2;
    DevGui_DrawDvarDescription(x, y, dvar);
    switch ( dvar->type )
    {
        case DVAR_TYPE_COLOR:
            y += rowHeight + 2;
            DevGui_DrawBox(x, y, rowWidth, rowHeight, (const unsigned __int8 *)&dvar->latched);
            for ( row = 0; row < rowCount; ++row )
            {
                y += rowHeight + 2;
                fraction = (float)((float)dvar->latched.color[row] * 1.0) / 255.0;
                if ( row == devguiGlob.selRow )
                    p_current = &devgui_colorSliderKnobSel->current;
                else
                    p_current = &devgui_colorSliderKnob->current;
                knobColor = (const unsigned __int8 *)p_current;
                DevGui_DrawSingleSlider(x, y, rowWidth, rowHeight, fraction, (const unsigned __int8 *)p_current);
            }
            break;
        case DVAR_TYPE_COLOR_XYZ:
            X = dvar->latched.value;
            Y = dvar->latched.vector[1];
            Z = dvar->latched.vector[2];
            X = X / 100.0;
            Y = Y / 100.0;
            Z = Z / 100.0;
            R = X * 3.241 + Y * -1.5374 + Z * -0.4986;
            G = X * -0.9692 + Y * 1.876 + Z * 0.0416;
            B = X * 0.0556 + Y * -0.204 + Z * 1.057;
            if ( R >= 0.0 )
                v12 = R;
            else
                v12 = 0.0;
            R = v12;
            if ( v12 <= 1.0 )
                v11 = R;
            else
                v11 = 1.0;
            R = v11;
            if ( G >= 0.0 )
                v10 = G;
            else
                v10 = 0.0;
            G = v10;
            if ( v10 <= 1.0 )
                v9 = G;
            else
                v9 = 1.0;
            G = v9;
            if ( B >= 0.0 )
                v8 = B;
            else
                v8 = 0.0;
            B = v8;
            if ( v8 <= 1.0 )
                v7 = B;
            else
                v7 = 1.0;
            B = v7;
            gammaColor[0] = (int)(sqrt(R) * 255.0);
            gammaColor[1] = (int)(sqrt(G) * 255.0);
            gammaColor[2] = (int)(sqrt(v7) * 255.0);
            gammaColor[3] = -1;
            y += rowHeight + 2;
            DevGui_DrawBox(x, y, rowWidth, rowHeight, gammaColor);
            for ( row = 0; row < rowCount; ++row )
            {
                y += rowHeight + 2;
                fraction = (float)(dvar->latched.vector[row] - dvar->domain.value.min)
                                 / (float)(dvar->domain.value.max - dvar->domain.value.min);
                if ( row == devguiGlob.selRow )
                    v6 = &devgui_colorSliderKnobSel->current;
                else
                    v6 = &devgui_colorSliderKnob->current;
                knobColor = (const unsigned __int8 *)v6;
                DevGui_DrawSingleSlider(x, y, rowWidth, rowHeight, fraction, (const unsigned __int8 *)v6);
            }
            break;
        case DVAR_TYPE_LINEAR_COLOR_RGB:
            color[0] = (int)(float)(sqrtf(dvar->latched.value) * 255.0);
            color[1] = (int)(float)(sqrtf(dvar->latched.vector[1]) * 255.0);
            color[2] = (int)(float)(sqrtf(dvar->latched.vector[2]) * 255.0);
            color[3] = -1;
            y += rowHeight + 2;
            DevGui_DrawBox(x, y, rowWidth, rowHeight, color);
            for ( row = 0; row < rowCount; ++row )
            {
                y += rowHeight + 2;
                fraction = (float)(dvar->latched.vector[row] - dvar->domain.value.min)
                                 / (float)(dvar->domain.value.max - dvar->domain.value.min);
                if ( row == devguiGlob.selRow )
                    v5 = &devgui_colorSliderKnobSel->current;
                else
                    v5 = &devgui_colorSliderKnob->current;
                knobColor = (const unsigned __int8 *)v5;
                DevGui_DrawSingleSlider(x, y, rowWidth, rowHeight, fraction, (const unsigned __int8 *)v5);
            }
            break;
        case DVAR_TYPE_FLOAT_2:
        case DVAR_TYPE_FLOAT_3:
        case DVAR_TYPE_FLOAT_4:
            for ( row = 0; row < rowCount; ++row )
            {
                y += rowHeight + 2;
                fraction = (float)(dvar->latched.vector[row] - dvar->domain.value.min)
                                 / (float)(dvar->domain.value.max - dvar->domain.value.min);
                if ( row == devguiGlob.selRow )
                    v4 = &devgui_colorSliderKnobSel->current;
                else
                    v4 = &devgui_colorSliderKnob->current;
                knobColor = (const unsigned __int8 *)v4;
                DevGui_DrawSingleSlider(x, y, rowWidth, rowHeight, fraction, (const unsigned __int8 *)v4);
            }
            break;
        default:
            y += rowHeight + 2;
            if ( dvar->type )
            {
                switch ( dvar->type )
                {
                    case DVAR_TYPE_INT:
                        if ( dvar->domain.integer.max == dvar->domain.enumeration.stringCount )
                            fraction = 0.5f;
                        else
                            fraction = (float)(dvar->latched.integer - dvar->domain.enumeration.stringCount)
                                             / (float)(dvar->domain.integer.max - dvar->domain.enumeration.stringCount);
                        break;
                    case DVAR_TYPE_ENUM:
                        if ( dvar->domain.enumeration.stringCount > 1 )
                            fraction = (float)dvar->latched.integer / (float)(dvar->domain.enumeration.stringCount - 1);
                        else
                            fraction = 0.5f;
                        break;
                    case DVAR_TYPE_FLOAT:
                        fraction = (float)(dvar->latched.value - dvar->domain.value.min)
                                         / (float)(dvar->domain.value.max - dvar->domain.value.min);
                        break;
                    default:
                        v2 = va("unhandled dvar type %i", dvar->type);
                        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 1397, 1, v2) )
                            __debugbreak();
                        fraction = 0.5f;
                        break;
                }
            }
            else
            {
                if ( dvar->latched.enabled )
                    v3 = 1.0f;
                else
                    v3 = 0.0f;
                fraction = v3;
            }
            DevGui_DrawSingleSlider(
                x,
                y,
                rowWidth,
                rowHeight,
                fraction,
                (const unsigned __int8 *)&devgui_colorSliderKnobSel->current);
            break;
    }
    DevGui_DrawDvarValue(x, y + rowHeight + 2, dvar);
}

void __cdecl DevGui_DrawSliderPath(int x, int y)
{
    char path[132]; // [esp+8h] [ebp-88h] BYREF

    DevGui_GetSliderPath(devguiGlob.selectedMenu, path, 0);
    DevGui_DrawFont(x, y, (const unsigned __int8 *)&devgui_colorText->current, path, 1.0, 1.0);
}

int __cdecl DevGui_GetSliderPath(unsigned __int16 menuHandle, char *path, int pathLen)
{
    int SliderPath; // eax
    devguiGlob_t *menu; // [esp+10h] [ebp-8h]
    int labelLen; // [esp+14h] [ebp-4h]

    menu = DevGui_GetMenu(menuHandle);
    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 1121, 0, "%s", "menu") )
        __debugbreak();
    if ( menu->menus[0].parent )
    {
        SliderPath = DevGui_GetSliderPath(menu->menus[0].parent, path, pathLen);
        path[SliderPath] = 47;
        pathLen = SliderPath + 1;
    }
    labelLen = strlen((const char *)menu);
    if ( labelLen + pathLen > 120
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    1130,
                    0,
                    "%s\n\t(path + pathLen) = %s",
                    "(pathLen + labelLen <= 120)",
                    &path[pathLen]) )
    {
        __debugbreak();
    }
    memcpy((unsigned __int8 *)&path[pathLen], (unsigned __int8 *)menu, labelLen + 1);
    return labelLen + pathLen;
}

void __cdecl DevGui_DrawDvarDescription(int x, int y, const dvar_s *dvar)
{
    if ( dvar->description )
        DevGui_DrawFont(x, y, (const unsigned __int8 *)&devgui_colorText->current, (char *)dvar->description, 1.0, 1.0);
}

void __cdecl DevGui_DrawSingleSlider(
                int x,
                int y,
                int rowWidth,
                int rowHeight,
                float fraction,
                const unsigned __int8 *knobColor)
{
    DevGui_DrawBevelBox(
        x,
        y,
        rowWidth,
        rowHeight,
        devgui_bevelShade->current.value,
        (const unsigned __int8 *)&devgui_colorSliderBgnd->current);
    DevGui_DrawBevelBox(
        (int)((float)((float)x + (float)((float)(rowWidth - 8) * fraction)) + 9.313225746154785e-10),
        y,
        8,
        rowHeight,
        devgui_bevelShade->current.value,
        knobColor);
}

void __cdecl DevGui_DrawDvarValue(int x, int y, const dvar_s *dvar)
{
    const char *v3; // eax
    float v4; // xmm0_4
    float v5; // xmm0_4
    float v6; // xmm0_4
    double v7; // [esp+30h] [ebp-7Ch]
    double v8; // [esp+38h] [ebp-74h]
    double v9; // [esp+40h] [ebp-6Ch]
    float v10; // [esp+48h] [ebp-64h]
    float v11; // [esp+4Ch] [ebp-60h]
    float v12; // [esp+50h] [ebp-5Ch]
    const char *v13; // [esp+54h] [ebp-58h]
    double G; // [esp+64h] [ebp-48h]
    double X; // [esp+6Ch] [ebp-40h]
    int r; // [esp+78h] [ebp-34h]
    double Y; // [esp+7Ch] [ebp-30h]
    double R; // [esp+84h] [ebp-28h]
    int g; // [esp+8Ch] [ebp-20h]
    int b; // [esp+90h] [ebp-1Ch]
    double B; // [esp+94h] [ebp-18h]
    double Z; // [esp+9Ch] [ebp-10h]
    char *text; // [esp+A8h] [ebp-4h]

    if (dvar->type)
    {
        switch (dvar->type)
        {
        case DVAR_TYPE_ENUM:
            v3 = Dvar_DisplayableLatchedValue(dvar);
            text = va("%i: %s", dvar->latched.integer, v3);
            break;
        case DVAR_TYPE_LINEAR_COLOR_RGB:
            if (dvar->latched.vector[2] <= 0.0031308001)
            {
                v12 = dvar->latched.vector[2] * 12.92;
            }
            else
            {
                v4 = pow(dvar->latched.vector[2], 0.4166666567325592);
                v12 = (float)(v4 * 1.0549999) - 0.055;
            }
            if (dvar->latched.vector[1] <= 0.0031308001)
            {
                v11 = dvar->latched.vector[1] * 12.92;
            }
            else
            {
                v5 = pow(dvar->latched.vector[1], 0.4166666567325592);
                v11 = (float)(v5 * 1.0549999) - 0.055;
            }
            if (dvar->latched.value <= 0.0031308001)
            {
                v10 = dvar->latched.value * 12.92;
            }
            else
            {
                v6 = pow(dvar->latched.value, 0.4166666567325592);
                v10 = (float)(v6 * 1.0549999) - 0.055;
            }
            text = va(
                "sRGB: %g %g %g Linear: %g %g %g",
                v10,
                v11,
                v12,
                dvar->latched.value,
                dvar->latched.vector[1],
                dvar->latched.vector[2]);
            break;
        case DVAR_TYPE_COLOR_XYZ:
            X = dvar->latched.value / 100.0;
            Y = dvar->latched.vector[1] / 100.0;
            Z = dvar->latched.vector[2] / 100.0;
            R = X * 3.241 + Y * -1.5374 + Z * -0.4986;
            G = X * -0.9692 + Y * 1.876 + Z * 0.0416;
            B = X * 0.0556 + Y * -0.204 + Z * 1.057;
            if (R <= 0.0031308)
                v9 = R * 12.92;
            else
                v9 = (pow(R, 0.4166666666666667) - 0.055) * 1.055;
            if (G <= 0.0031308)
                v8 = G * 12.92;
            else
                v8 = (pow(G, 0.4166666666666667) - 0.055) * 1.055;
            if (B <= 0.0031308)
                v7 = B * 12.92;
            else
                v7 = (pow(B, 0.4166666666666667) - 0.055) * 1.055;
            r = (int)floor(v9 * 255.0 + 0.5);
            g = (int)floor(v8 * 255.0 + 0.5);
            b = (int)floor(v7 * 255.0 + 0.5);
            text = va(
                "XYZ:%f %f %f sRGB: %d %d %d",
                dvar->latched.value,
                dvar->latched.vector[1],
                dvar->latched.vector[2],
                r,
                g,
                b);
            break;
        default:
            text = (char *)Dvar_DisplayableLatchedValue(dvar);
            break;
        }
    }
    else
    {
        if (dvar->latched.enabled)
            v13 = "On";
        else
            v13 = "Off";
        text = (char *)v13;
    }
    DevGui_DrawFont(x, y, (const unsigned __int8 *)&devgui_colorText->current, text, 1.0, 1.0);
}

int __cdecl DevGui_DvarRowCount(const dvar_s *dvar)
{
    int result; // eax

    switch ( dvar->type )
    {
        case DVAR_TYPE_FLOAT_2:
            result = 2;
            break;
        case DVAR_TYPE_FLOAT_3:
        case DVAR_TYPE_LINEAR_COLOR_RGB:
        case DVAR_TYPE_COLOR_XYZ:
            result = 3;
            break;
        case DVAR_TYPE_FLOAT_4:
        case DVAR_TYPE_COLOR:
            result = 4;
            break;
        default:
            result = 1;
            break;
    }
    return result;
}

void DevGui_DrawBindNextKey()
{
    unsigned __int8 fadeColor[4]; // [esp+10h] [ebp-1Ch] BYREF
    unsigned __int8 textColor[4]; // [esp+14h] [ebp-18h] BYREF
    int x; // [esp+18h] [ebp-14h]
    int y; // [esp+1Ch] [ebp-10h]
    int h; // [esp+20h] [ebp-Ch]
    const char *text; // [esp+24h] [ebp-8h]
    int w; // [esp+28h] [ebp-4h]

    *(unsigned int *)fadeColor = -1442840576;
    DevGui_DrawBox(
        devguiGlob.left,
        devguiGlob.top,
        devguiGlob.right - devguiGlob.left,
        devguiGlob.bottom - devguiGlob.top,
        fadeColor);
    text = "Press key to bind (ESC to cancel)...";
    w = DevGui_GetFontWidth("Press key to bind (ESC to cancel)...");
    h = DevGui_GetFontHeight();
    x = (devguiGlob.right + devguiGlob.left - w) / 2;
    y = (devguiGlob.bottom + devguiGlob.top - h) / 2;
    *(unsigned int *)textColor = -1;
    DevGui_DrawFont(x, y, textColor, (char *)text, 1.0, 1.0);
}

static const char *MYINSTRUCTIONS = "<Y> Edits node,  <LB> Adds new node,  <RB> Removes node,  <START> Saves to disk";
void __cdecl DevGui_DrawGraph(const DevMenuItem *menu, int localClientNum)
{
    const char *v2; // eax
    float *v3; // ecx
    float *v4; // [esp+18h] [ebp-15Ch]
    float *v5; // [esp+1Ch] [ebp-158h]
    int rowHeight; // [esp+20h] [ebp-154h]
    int graphBottomY; // [esp+24h] [ebp-150h]
    float nextKnot; // [esp+28h] [ebp-14Ch]
    float nextKnot_4; // [esp+2Ch] [ebp-148h]
    int width; // [esp+30h] [ebp-144h]
    DevGraph *graph; // [esp+38h] [ebp-13Ch]
    int knotX; // [esp+3Ch] [ebp-138h]
    float endKnotPos[2]; // [esp+40h] [ebp-134h] BYREF
    int knotY; // [esp+48h] [ebp-12Ch]
    int x; // [esp+4Ch] [ebp-128h]
    int y; // [esp+50h] [ebp-124h]
    int rowWidth; // [esp+54h] [ebp-120h]
    float startKnotPos[2]; // [esp+58h] [ebp-11Ch] BYREF
    float knot[2]; // [esp+60h] [ebp-114h]
    int knotIndex; // [esp+68h] [ebp-10Ch]
    char text[260]; // [esp+6Ch] [ebp-108h] BYREF
    int savedregs; // [esp+174h] [ebp+0h] BYREF

    if ( menu->childType != 3 )
    {
        v2 = va("menu %s type %i", menu->label, menu->childType);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                        1453,
                        0,
                        "%s\n\t%s",
                        "menu->childType == DEV_CHILD_GRAPH",
                        v2) )
            __debugbreak();
    }
    graph = menu->child.graph;
    if ( !graph && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 1456, 0, "%s", "graph") )
        __debugbreak();
    if ( !graph->knots
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 1456, 0, "%s", "graph->knots") )
    {
        __debugbreak();
    }
    if ( !graph->knotCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 1456, 0, "%s", "graph->knotCount") )
    {
        __debugbreak();
    }
    if ( graph->knotCountMax <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    1456,
                    0,
                    "%s",
                    "graph->knotCountMax > 0") )
    {
        __debugbreak();
    }
    if ( !graph
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    1458,
                    0,
                    "%s\n\t(menu->label) = %s",
                    "(graph)",
                    menu->label) )
    {
        __debugbreak();
    }
    rowWidth = devguiGlob.sliderWidth;
    rowHeight = DevGui_GetFontHeight();
    width = rowWidth + 8;
    x = devguiGlob.left + (devguiGlob.right - devguiGlob.left - (rowWidth + 8)) / 2;
    y = devguiGlob.bottom - (3 * rowHeight + 16);
    DevGui_DrawBevelBox(
        x,
        y,
        rowWidth + 8,
        3 * rowHeight + 16,
        devgui_bevelShade->current.value,
        (const unsigned __int8 *)&devgui_colorBgnd->current);
    graphBottomY = (int)(float)((float)y * 0.94999999);
    DevGui_DrawBox(x, graphBottomY, rowWidth + 8, 2, (const unsigned __int8 *)&devgui_colorBgndSel->current);
    x += 4;
    y += 6;
    DevGui_DrawSliderPath(x, y);
    for ( knotIndex = 0; knotIndex < *graph->knotCount; ++knotIndex )
    {
        v5 = graph->knots[knotIndex];
        knot[0] = *v5;
        knot[1] = v5[1];
        knotX = x + (int)(float)((float)width * knot[0]);
        knotY = graphBottomY - (int)(float)((float)(graphBottomY - 48) * knot[1]);
        if ( knotIndex == graph->selectedKnot )
        {
            if ( devguiGlob.editingKnot )
                DevGui_DrawBoxCentered(knotX, knotY, 19, 19, (const unsigned __int8 *)&devgui_colorGraphKnotEditing->current);
            else
                DevGui_DrawBoxCentered(knotX, knotY, 16, 16, (const unsigned __int8 *)&devgui_colorGraphKnotSelected->current);
        }
        else
        {
            DevGui_DrawBoxCentered(knotX, knotY, 12, 12, (const unsigned __int8 *)&devgui_colorGraphKnotNormal->current);
        }
    }
    for ( knotIndex = 0; knotIndex < *graph->knotCount - 1; ++knotIndex )
    {
        v4 = graph->knots[knotIndex];
        knot[0] = *v4;
        knot[1] = v4[1];
        nextKnot = graph->knots[knotIndex + 1][0];
        nextKnot_4 = graph->knots[knotIndex + 1][1];
        startKnotPos[0] = (float)((float)width * knot[0]) + (float)x;
        startKnotPos[1] = (float)graphBottomY - (float)((float)(graphBottomY - 48) * knot[1]);
        endKnotPos[0] = (float)((float)width * nextKnot) + (float)x;
        endKnotPos[1] = (float)graphBottomY - (float)((float)(graphBottomY - 48) * nextKnot_4);
        DevGui_DrawLine(
            startKnotPos,
            endKnotPos,
            2,
            (const unsigned __int8 *)&devgui_colorGraphKnotNormal->current);
    }
    y += rowHeight + 2;
    v3 = graph->knots[graph->selectedKnot];
    knot[0] = *v3;
    knot[1] = v3[1];
    if ( graph->textCallback )
        ((void (__cdecl *)(DevGraph *, unsigned int, unsigned int, char *, int))graph->textCallback)(
            graph,
            LODWORD(knot[0]),
            LODWORD(knot[1]),
            text,
            256);
    else
        sprintf(text, "X: %.4f, Y: %.4f", knot[0], knot[1]);
    DevGui_DrawFont(x, y, (const unsigned __int8 *)&devgui_colorText->current, text, 1.0, 1.0);
    y += rowHeight + 2;
    DevGui_DrawFont(x, y, (const unsigned __int8 *)&devgui_colorText->current, (char *)MYINSTRUCTIONS, 1.0, 1.0);
    if ( graph->eventCallback )
        graph->eventCallback(graph, EVENT_DRAW, localClientNum);
}

void __cdecl DevGui_Init()
{
    unsigned int menuIndex; // [esp+0h] [ebp-Ch]
    int screen_yPad; // [esp+4h] [ebp-8h]
    int screen_xPad; // [esp+8h] [ebp-4h]

    DevGui_RegisterDvars();
    screen_xPad = RETURN_ZERO32();
    screen_yPad = RETURN_ZERO32();
    for ( menuIndex = 0; menuIndex < 0x7FF; ++menuIndex )
        *(unsigned int *)devguiGlob.menus[menuIndex].label = (unsigned int)&devguiGlob.menus[menuIndex + 1];
    *(unsigned int *)devguiGlob.menus[menuIndex].label = 0;
    devguiGlob.nextFreeMenu = (DevMenuItem *)&devguiGlob;
    devguiGlob.topmostMenu.childType = 0;
    devguiGlob.topmostMenu.childMenuMemory = 0;
    devguiGlob.topmostMenu.child.menu = 0;
    devguiGlob.topmostMenu.nextSibling = 0;
    devguiGlob.topmostMenu.prevSibling = 0;
    devguiGlob.topmostMenu.parent = 0;
    devguiGlob.isActive = 0;
    devguiGlob.editingMenuItem = 0;
    devguiGlob.selRow = -1;
    devguiGlob.editingKnot = 0;
    devguiGlob.left = screen_xPad;
    devguiGlob.right = DevGui_GetScreenWidth() - screen_xPad;
    devguiGlob.top = screen_yPad;
    devguiGlob.bottom = DevGui_GetScreenHeight() - screen_yPad;
    devguiGlob.sliderWidth = 3 * (devguiGlob.right - devguiGlob.left) / 4;
    DevGui_InputInit();
    devguiGlob.isInitialized = 1;
}



void DevGui_RegisterDvars()
{
    const dvar_s *result; // eax

    devgui_colorBgnd = _Dvar_RegisterColor(
                                             "devgui_colorBgnd",
                                             0.0,
                                             0.41999999,
                                             0.0,
                                             0.75,
                                             0,
                                             "Color background for the devgui");
    devgui_colorText = _Dvar_RegisterColor("devgui_colorText", 1.0, 1.0, 1.0, 1.0, 0, "Text color for the devgui");
    devgui_colorBgndSel = _Dvar_RegisterColor(
                                                    "devgui_colorBgndSel",
                                                    0.0,
                                                    0.69999999,
                                                    0.0,
                                                    0.75,
                                                    0,
                                                    "Selection background color for the devgui");
    devgui_colorTextSel = _Dvar_RegisterColor(
                                                    "devgui_colorTextSel",
                                                    1.0,
                                                    1.0,
                                                    0.0,
                                                    1.0,
                                                    0,
                                                    "Selection text color for the devgui");
    devgui_colorBgndGray = _Dvar_RegisterColor(
                                                     "devgui_colorBgndGray",
                                                     0.2,
                                                     0.2,
                                                     0.2,
                                                     0.89999998,
                                                     0,
                                                     "Grayed out background color for the devgui");
    devgui_colorTextGray = _Dvar_RegisterColor(
                                                     "devgui_colorTextGray",
                                                     0.69999999,
                                                     0.69999999,
                                                     0.69999999,
                                                     1.0,
                                                     0,
                                                     "Greyed out text color for the devgui");
    devgui_colorBgndGraySel = _Dvar_RegisterColor(
                                                            "devgui_colorBgndGraySel",
                                                            0.40000001,
                                                            0.40000001,
                                                            0.40000001,
                                                            0.89999998,
                                                            0,
                                                            "Greyed out, selected background color for the devgui");
    devgui_colorTextGraySel = _Dvar_RegisterColor(
                                                            "devgui_colorTextGraySel",
                                                            1.0,
                                                            1.0,
                                                            0.0,
                                                            1.0,
                                                            0,
                                                            "Greyed out, selected text color for the devgui");
    devgui_colorSliderBgnd = _Dvar_RegisterColor(
                                                         "devgui_colorSliderBgnd",
                                                         1.0,
                                                         1.0,
                                                         1.0,
                                                         0.75,
                                                         0,
                                                         "Color slider background for the devgui");
    devgui_colorSliderKnob = _Dvar_RegisterColor(
                                                         "devgui_colorSliderKnob",
                                                         1.0,
                                                         1.0,
                                                         1.0,
                                                         1.0,
                                                         0,
                                                         "Knob color for the devgui");
    devgui_colorSliderKnobSel = _Dvar_RegisterColor(
                                                                "devgui_colorSliderKnobSel",
                                                                1.0,
                                                                1.0,
                                                                0.0,
                                                                1.0,
                                                                0,
                                                                "Selected knob color for the devgui");
    devgui_bevelShade = _Dvar_RegisterFloat("devgui_bevelShade", 0.69999999, 0.0, 1.0, 0, "Bevel shade for the devgui");
    devgui_colorGraphKnotNormal = _Dvar_RegisterColor(
                                                                    "devgui_colorGraphKnotNormal",
                                                                    0.0,
                                                                    1.0,
                                                                    1.0,
                                                                    0.69999999,
                                                                    0,
                                                                    "Devgiu Color graph knot normal color");
    devgui_colorGraphKnotSelected = _Dvar_RegisterColor(
                                                                        "devgui_colorGraphKnotSelected",
                                                                        1.0,
                                                                        0.0,
                                                                        0.0,
                                                                        0.69999999,
                                                                        0,
                                                                        "Devgui color graph knot selected color");
    devgui_colorGraphKnotEditing = _Dvar_RegisterColor(
                                                                     "devgui_colorGraphKnotEditing",
                                                                     1.0,
                                                                     0.0,
                                                                     1.0,
                                                                     1.0,
                                                                     0,
                                                                     "Devgui color graph knot editing color");
    devgui_zoomEnabled = _Dvar_RegisterBool("devgui_zoomEnabled", 0, 0, "Enlarges the currently selected Devgui cell.");
}

void __cdecl DevGui_Shutdown()
{
    DevGui_InputShutdown();
    DevGui_MenuShutdown();
}

void DevGui_MenuShutdown()
{
    DevGui_FreeMenu_r(devguiGlob.topmostMenu.child.menu);
}

void __cdecl DevGui_KeyPressed(int key)
{
    char *v1; // eax
    unsigned __int16 handle; // [esp+0h] [ebp-8Ch]
    char path[128]; // [esp+4h] [ebp-88h] BYREF
    DevMenuItem *menu; // [esp+88h] [ebp-4h]

    if ( devguiGlob.bindNextKey )
    {
        devguiGlob.bindNextKey = 0;
        if ( key != 27 )
        {
            if ( key == 9 || key == 167 )
            {
                Com_Printf(11, "Can't rebind 'tab' or 'F1'\n");
            }
            else
            {
                handle = devguiGlob.selectedMenu;
                if ( !devguiGlob.selectedMenu
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 2447, 0, "%s", "handle") )
                {
                    __debugbreak();
                }
                menu = (DevMenuItem *)DevGui_GetMenu(handle);
                if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 2449, 0, "%s", "menu") )
                    __debugbreak();
                if ( menu->parent && (!menu->childType || menu->childType == 1 && !devguiGlob.editingMenuItem) )
                    handle = menu->parent;
                DevGui_GetSliderPath(handle, path, 0);
                v1 = va("devgui_open \"%s\"", path);
                Key_SetBinding(0, key, v1, 0);
            }
        }
    }
}

void __cdecl DevGui_Update(int localClientNum, float deltaTime)
{
    bool v2; // al
    bool IsButtonDown; // [esp+8h] [ebp-8h]
    devguiGlob_t *selMenuItem; // [esp+Ch] [ebp-4h]

    PROF_SCOPED("DevGui_Update"); // LWSS ADD

    if ( devguiGlob.isActive && !devguiGlob.bindNextKey && DevGui_InputUpdate(localClientNum, deltaTime) )
    {
        selMenuItem = DevGui_GetMenu(devguiGlob.selectedMenu);
        if ( !selMenuItem
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 2486, 0, "%s", "selMenuItem") )
        {
            __debugbreak();
        }
        if ( devguiGlob.editingMenuItem && selMenuItem->menus[0].childType == 3 )
        {
            DevGui_UpdateGraph(localClientNum, deltaTime);
        }
        else
        {
            if ( devguiGlob.editingMenuItem )
            {
                if ( selMenuItem->menus[0].childType != 1
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                                2497,
                                0,
                                "%s",
                                "selMenuItem->childType == DEV_CHILD_DVAR") )
                {
                    __debugbreak();
                }
                IsButtonDown = DevGui_IsButtonDown(INPUT_RESET);
                v2 = DevGui_IsButtonDown(INPUT_LINK);
                DevGui_UpdateDvar(deltaTime, v2, IsButtonDown);
                DevGui_UpdateSelection();
            }
            else
            {
                DevGui_MoveSelectionHorizontally();
                DevGui_MoveSelectionVertically();
            }
            if ( DevGui_IsButtonReleased(INPUT_ACCEPT) )
                DevGui_Accept(localClientNum);
            if ( DevGui_IsButtonReleased(INPUT_REJECT) )
                DevGui_Reject();
        }
    }
}

void DevGui_MoveSelectionHorizontally()
{
    int scroll; // [esp+0h] [ebp-4h]

    for ( scroll = DevGui_GetMenuScroll(SCROLL_XAXIS); scroll < 0; ++scroll )
        DevGui_MoveLeft();
    while ( scroll > 0 )
    {
        DevGui_MoveRight();
        --scroll;
    }
}

void DevGui_MoveLeft()
{
    devguiGlob_t *menu; // [esp+0h] [ebp-4h]

    if ( !devguiGlob.selectedMenu
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    1701,
                    0,
                    "%s",
                    "devguiGlob.selectedMenu") )
    {
        __debugbreak();
    }
    for ( menu = DevGui_GetMenu(devguiGlob.selectedMenu);
                menu->menus[0].parent;
                menu = DevGui_GetMenu(devguiGlob.selectedMenu) )
    {
        devguiGlob.selectedMenu = menu->menus[0].parent;
    }
    DevGui_SelectPrevMenuItem();
}

void DevGui_SelectPrevMenuItem()
{
    devguiGlob_t *v0; // [esp+0h] [ebp-Ch]
    devguiGlob_t *menu; // [esp+4h] [ebp-8h]

    if ( !devguiGlob.selectedMenu
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    1611,
                    0,
                    "%s",
                    "devguiGlob.selectedMenu") )
    {
        __debugbreak();
    }
    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if ( menu->menus[0].parent )
        v0 = DevGui_GetMenu(menu->menus[0].parent);
    else
        v0 = 0;
    if ( menu->menus[0].prevSibling )
    {
        if ( v0 )
            --v0->menus[0].childMenuMemory;
        devguiGlob.selectedMenu = menu->menus[0].prevSibling;
    }
    else
    {
        if ( v0 )
            v0->menus[0].childMenuMemory = 0;
        while ( menu->menus[0].nextSibling )
        {
            if ( v0 )
                ++v0->menus[0].childMenuMemory;
            devguiGlob.selectedMenu = menu->menus[0].nextSibling;
            menu = DevGui_GetMenu(devguiGlob.selectedMenu);
        }
    }
    DevGui_SelectTopLevelChild();
}

void DevGui_SelectTopLevelChild()
{
    devguiGlob_t *menu; // [esp+0h] [ebp-4h]

    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 1594, 0, "%s", "menu") )
        __debugbreak();
    if ( !menu->menus[0].parent )
    {
        if ( menu->menus[0].childType
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                        1598,
                        0,
                        "%s\n\t(menu->childType) = %i",
                        "(menu->childType == DEV_CHILD_MENU)",
                        menu->menus[0].childType) )
        {
            __debugbreak();
        }
        if ( !menu->menus[0].child.menu
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 1599, 0, "%s", "menu->child.menu") )
        {
            __debugbreak();
        }
        devguiGlob.selectedMenu = menu->menus[0].child.menu;
        DevGui_AdvanceChildNum(menu->menus[0].childMenuMemory);
    }
}

void __cdecl DevGui_AdvanceChildNum(int numberToAdvance)
{
    devguiGlob_t *menu; // [esp+0h] [ebp-8h]
    int numberIter; // [esp+4h] [ebp-4h]

    for ( numberIter = 0; numberIter != numberToAdvance; ++numberIter )
    {
        menu = DevGui_GetMenu(devguiGlob.selectedMenu);
        if ( !menu->menus[0].nextSibling )
            break;
        devguiGlob.selectedMenu = menu->menus[0].nextSibling;
        DevGui_GetMenu(devguiGlob.selectedMenu);
    }
}

void DevGui_MoveRight()
{
    devguiGlob_t *menu; // [esp+0h] [ebp-4h]

    if ( !devguiGlob.selectedMenu
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    1716,
                    0,
                    "%s",
                    "devguiGlob.selectedMenu") )
    {
        __debugbreak();
    }
    for ( menu = DevGui_GetMenu(devguiGlob.selectedMenu);
                menu->menus[0].parent;
                menu = DevGui_GetMenu(devguiGlob.selectedMenu) )
    {
        devguiGlob.selectedMenu = menu->menus[0].parent;
    }
    DevGui_SelectNextMenuItem();
}

void DevGui_SelectNextMenuItem()
{
    devguiGlob_t *menuParent; // [esp+0h] [ebp-8h]
    devguiGlob_t *menu; // [esp+4h] [ebp-4h]

    if ( !devguiGlob.selectedMenu
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    1657,
                    0,
                    "%s",
                    "devguiGlob.selectedMenu") )
    {
        __debugbreak();
    }
    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if ( menu->menus[0].parent )
    {
        menuParent = DevGui_GetMenu(menu->menus[0].parent);
        if ( menu->menus[0].nextSibling )
            ++menuParent->menus[0].childMenuMemory;
        else
            menuParent->menus[0].childMenuMemory = 0;
    }
    if ( menu->menus[0].nextSibling )
    {
        devguiGlob.selectedMenu = menu->menus[0].nextSibling;
    }
    else
    {
        while ( menu->menus[0].prevSibling )
        {
            devguiGlob.selectedMenu = menu->menus[0].prevSibling;
            menu = DevGui_GetMenu(devguiGlob.selectedMenu);
        }
    }
    DevGui_SelectTopLevelChild();
}

void DevGui_MoveSelectionVertically()
{
    int scroll; // [esp+0h] [ebp-4h]

    for ( scroll = DevGui_GetMenuScroll(SCROLL_YAXIS); scroll < 0; ++scroll )
        DevGui_MoveDown();
    while ( scroll > 0 )
    {
        DevGui_MoveUp();
        --scroll;
    }
}

void DevGui_MoveUp()
{
    devguiGlob_t *menu; // [esp+0h] [ebp-4h]

    if ( !devguiGlob.selectedMenu
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    1735,
                    0,
                    "%s",
                    "devguiGlob.selectedMenu") )
    {
        __debugbreak();
    }
    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if ( !menu->menus[0].parent )
    {
        if ( menu->menus[0].childType
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                        1739,
                        0,
                        "%s\n\t(menu->childType) = %i",
                        "(menu->childType == DEV_CHILD_MENU)",
                        menu->menus[0].childType) )
        {
            __debugbreak();
        }
        devguiGlob.selectedMenu = menu->menus[0].child.menu;
        DevGui_GetMenu(devguiGlob.selectedMenu);
    }
    DevGui_SelectPrevMenuItem();
}

void DevGui_MoveDown()
{
    devguiGlob_t *menu; // [esp+0h] [ebp-4h]

    if ( !devguiGlob.selectedMenu
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    1752,
                    0,
                    "%s",
                    "devguiGlob.selectedMenu") )
    {
        __debugbreak();
    }
    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if ( !menu->menus[0].parent )
    {
        if ( menu->menus[0].childType
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                        1756,
                        0,
                        "%s\n\t(menu->childType) = %i",
                        "(menu->childType == DEV_CHILD_MENU)",
                        menu->menus[0].childType) )
        {
            __debugbreak();
        }
        devguiGlob.selectedMenu = menu->menus[0].child.menu;
        DevGui_GetMenu(devguiGlob.selectedMenu);
    }
    DevGui_SelectNextMenuItem();
}

void __cdecl DevGui_Accept(int localClientNum)
{
    devguiGlob_t *menu; // [esp+4h] [ebp-4h]

    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 1811, 0, "%s", "menu") )
        __debugbreak();
    switch ( menu->menus[0].childType )
    {
        case 0u:
            devguiGlob.selectedMenu = menu->menus[0].child.menu;
            DevGui_AdvanceChildNum(menu->menus[0].childMenuMemory);
            break;
        case 1u:
            if ( DevGui_EditableMenuItem(menu->menus) )
            {
                if ( devguiGlob.editingMenuItem )
                {
                    Dvar_MakeLatchedValueCurrent((dvar_s *)menu->menus[0].child.command);
                    Dvar_SetModified(menu->menus[0].child.dvar);
                }
                devguiGlob.editingMenuItem = !devguiGlob.editingMenuItem;
                devguiGlob.selRow = 0;
            }
            break;
        case 2u:
            Cbuf_InsertText(localClientNum, (char *)menu->menus[0].child.command);
            break;
        case 3u:
            devguiGlob.editingMenuItem = !devguiGlob.editingMenuItem;
            devguiGlob.selRow = 0;
            if ( menu->menus[0].child.command && *((unsigned int *)menu->menus[0].child.command + 4) )
                (*((void (__cdecl **)(DevMenuChild, unsigned int, int))menu->menus[0].child.command + 4))(
                    menu->menus[0].child,
                    0,
                    localClientNum);
            break;
        default:
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 1850, 1, "unhandled case") )
                __debugbreak();
            break;
    }
}

void DevGui_Reject()
{
    devguiGlob_t *menu; // [esp+0h] [ebp-4h]

    if ( !devguiGlob.selectedMenu
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    1860,
                    0,
                    "%s",
                    "devguiGlob.selectedMenu") )
    {
        __debugbreak();
    }
    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if ( devguiGlob.editingMenuItem )
    {
        Dvar_ClearLatchedValue((dvar_s *)menu->menus[0].child.command);
        devguiGlob.editingMenuItem = 0;
    }
    else if ( menu->menus[0].parent && DevGui_GetMenu(menu->menus[0].parent)->menus[0].parent )
    {
        devguiGlob.selectedMenu = menu->menus[0].parent;
    }
    else if ( menu->menus[0].parent )
    {
        if ( !DevGui_GetMenu(menu->menus[0].parent)->menus[0].parent )
            DevGui_Toggle();
    }
}

int DevGui_UpdateSelection()
{
    int result; // eax
    int scroll; // [esp+0h] [ebp-4h]

    result = DevGui_GetMenuScroll(SCROLL_YAXIS);
    for ( scroll = (__int16)result; scroll < 0; ++scroll )
        result = DevGui_ScrollDown();
    while ( scroll > 0 )
    {
        result = DevGui_ScrollUp();
        --scroll;
    }
    return result;
}

int DevGui_ScrollUp()
{
    int result; // eax
    devguiGlob_t *menu; // [esp+4h] [ebp-8h]
    const dvar_s *dvar; // [esp+8h] [ebp-4h]

    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if ( menu->menus[0].childType == 3 )
        return DevGui_ScrollUpInternal();
    if ( menu->menus[0].childType != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    1969,
                    0,
                    "%s",
                    "menu->childType == DEV_CHILD_DVAR") )
    {
        __debugbreak();
    }
    dvar = DevGui_SelectedDvar();
    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 1972, 0, "%s", "dvar") )
        __debugbreak();
    result = DevGui_DvarRowCount(dvar);
    if ( result == 1 )
        return DevGui_ScrollUpInternal();
    if ( devguiGlob.selRow )
        --devguiGlob.selRow;
    else
        devguiGlob.selRow = result - 1;
    return result;
}

const dvar_s *__cdecl DevGui_SelectedDvar()
{
    const char *v0; // eax
    devguiGlob_t *menu; // [esp+0h] [ebp-8h]
    const dvar_s *dvar; // [esp+4h] [ebp-4h]

    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 1935, 0, "%s", "menu") )
        __debugbreak();
    if ( menu->menus[0].childType != 1 )
    {
        v0 = va("menu %s type %i", (const char *)menu, menu->menus[0].childType);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                        1936,
                        0,
                        "%s\n\t%s",
                        "menu->childType == DEV_CHILD_DVAR",
                        v0) )
            __debugbreak();
    }
    dvar = menu->menus[0].child.dvar;
    if ( !dvar
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    1938,
                    0,
                    "%s\n\t(menu->label) = %s",
                    "(dvar)",
                    (const char *)menu) )
    {
        __debugbreak();
    }
    return dvar;
}

int DevGui_ScrollUpInternal()
{
    int result; // eax
    devguiGlob_t *menu; // [esp+0h] [ebp-4h]

    do
    {
        DevGui_MoveUp();
        menu = DevGui_GetMenu(devguiGlob.selectedMenu);
        result = DevGui_EditableMenuItem(menu->menus);
    }
    while ( !(_BYTE)result );
    return result;
}

int DevGui_ScrollDown()
{
    int result; // eax
    devguiGlob_t *menu; // [esp+4h] [ebp-8h]
    const dvar_s *dvar; // [esp+8h] [ebp-4h]

    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if ( menu->menus[0].childType == 3 )
        return DevGui_ScrollDownInternal();
    if ( menu->menus[0].childType != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    2012,
                    0,
                    "%s",
                    "menu->childType == DEV_CHILD_DVAR") )
    {
        __debugbreak();
    }
    dvar = DevGui_SelectedDvar();
    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 2015, 0, "%s", "dvar") )
        __debugbreak();
    result = DevGui_DvarRowCount(dvar);
    if ( result == 1 )
        return DevGui_ScrollDownInternal();
    if ( ++devguiGlob.selRow == result )
        devguiGlob.selRow = 0;
    return result;
}

int DevGui_ScrollDownInternal()
{
    int result; // eax
    devguiGlob_t *menu; // [esp+0h] [ebp-4h]

    do
    {
        DevGui_MoveDown();
        menu = DevGui_GetMenu(devguiGlob.selectedMenu);
        result = DevGui_EditableMenuItem(menu->menus);
    }
    while ( !(_BYTE)result );
    return result;
}

void __cdecl DevGui_UpdateDvar(float deltaTime, bool channelLink, bool dvarReset)
{
    const char *v3; // eax
    float floatValue; // [esp+4Ch] [ebp-40h]
    float floatValuea; // [esp+4Ch] [ebp-40h]
    bool boolValue; // [esp+53h] [ebp-39h]
    __int64 int64Value; // [esp+54h] [ebp-38h]
    float stepc; // [esp+60h] [ebp-2Ch]
    float step; // [esp+60h] [ebp-2Ch]
    float stepa; // [esp+60h] [ebp-2Ch]
    float stepb; // [esp+60h] [ebp-2Ch]
    DvarValue vector; // [esp+64h] [ebp-28h]
    const dvar_s *dvar; // [esp+74h] [ebp-18h]
    float color[4]; // [esp+78h] [ebp-14h] BYREF
    int intValue; // [esp+88h] [ebp-4h]

    dvar = DevGui_SelectedDvar();
    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 2086, 0, "%s", "dvar") )
        __debugbreak();
    if ( dvarReset )
        Dvar_Reset((dvar_s *)dvar, DVAR_SOURCE_DEVGUI);
    switch ( dvar->type )
    {
        case DVAR_TYPE_BOOL:
            boolValue = (unsigned int)DevGui_UpdateIntScroll(deltaTime, dvar->latched.color[0] != 0, 0, 1, SCROLL_XAXIS) != 0;
            if ( boolValue != dvar->latched.enabled )
                Dvar_SetBoolFromSource((dvar_s *)dvar, boolValue, DVAR_SOURCE_DEVGUI);
            break;
        case DVAR_TYPE_FLOAT:
            stepc = DevGui_PickFloatScrollStep(dvar->domain.value.min, dvar->domain.value.max);
            floatValue = DevGui_UpdateFloatScroll(
                                         deltaTime,
                                         dvar->latched.value,
                                         dvar->domain.value.min,
                                         dvar->domain.value.max,
                                         stepc,
                                         SCROLL_XAXIS);
            if ( floatValue != dvar->latched.value )
                Dvar_SetFloatFromSource((dvar_s *)dvar, floatValue, DVAR_SOURCE_DEVGUI);
            break;
        case DVAR_TYPE_FLOAT_2:
            step = DevGui_PickFloatScrollStep(dvar->domain.value.min, dvar->domain.value.max);
            vector.integer64 = dvar->latched.integer64;
            if ( channelLink )
            {
                vector.value = DevGui_UpdateFloatScroll(
                                                 deltaTime,
                                                 vector.value,
                                                 dvar->domain.value.min,
                                                 dvar->domain.value.max,
                                                 step,
                                                 SCROLL_XAXIS);
                vector.vector[1] = DevGui_UpdateFloatScroll(
                                                         deltaTime,
                                                         vector.vector[1],
                                                         dvar->domain.value.min,
                                                         dvar->domain.value.max,
                                                         step,
                                                         SCROLL_XAXIS);
            }
            else
            {
                vector.vector[devguiGlob.selRow] = DevGui_UpdateFloatScroll(
                                                                                         deltaTime,
                                                                                         vector.vector[devguiGlob.selRow],
                                                                                         dvar->domain.value.min,
                                                                                         dvar->domain.value.max,
                                                                                         step,
                                                                                         SCROLL_XAXIS);
            }
            if ( dvar->latched.value != vector.value || dvar->latched.vector[1] != vector.vector[1] )
                Dvar_SetVec2FromSource((dvar_s *)dvar, vector.value, vector.vector[1], DVAR_SOURCE_DEVGUI);
            break;
        case DVAR_TYPE_FLOAT_3:
        case DVAR_TYPE_LINEAR_COLOR_RGB:
        case DVAR_TYPE_COLOR_XYZ:
            stepa = DevGui_PickFloatScrollStep(dvar->domain.value.min, dvar->domain.value.max);
            vector.integer64 = dvar->latched.integer64;
            vector.vector[2] = dvar->latched.vector[2];
            if ( channelLink )
            {
                vector.value = DevGui_UpdateFloatScroll(
                                                 deltaTime,
                                                 vector.value,
                                                 dvar->domain.value.min,
                                                 dvar->domain.value.max,
                                                 stepa,
                                                 SCROLL_XAXIS);
                vector.vector[1] = DevGui_UpdateFloatScroll(
                                                         deltaTime,
                                                         vector.vector[1],
                                                         dvar->domain.value.min,
                                                         dvar->domain.value.max,
                                                         stepa,
                                                         SCROLL_XAXIS);
                vector.vector[2] = DevGui_UpdateFloatScroll(
                                                         deltaTime,
                                                         vector.vector[2],
                                                         dvar->domain.value.min,
                                                         dvar->domain.value.max,
                                                         stepa,
                                                         SCROLL_XAXIS);
            }
            else
            {
                vector.vector[devguiGlob.selRow] = DevGui_UpdateFloatScroll(
                                                                                         deltaTime,
                                                                                         vector.vector[devguiGlob.selRow],
                                                                                         dvar->domain.value.min,
                                                                                         dvar->domain.value.max,
                                                                                         stepa,
                                                                                         SCROLL_XAXIS);
            }
            if ( dvar->latched.value != vector.value
                || dvar->latched.vector[1] != vector.vector[1]
                || dvar->latched.vector[2] != vector.vector[2] )
            {
                Dvar_SetVec3FromSource(
                    (dvar_s *)dvar,
                    vector.value,
                    vector.vector[1],
                    vector.vector[2],
                    DVAR_SOURCE_DEVGUI);
            }
            break;
        case DVAR_TYPE_FLOAT_4:
            stepb = DevGui_PickFloatScrollStep(dvar->domain.value.min, dvar->domain.value.max);
            vector = dvar->latched;
            if ( channelLink )
            {
                vector.value = DevGui_UpdateFloatScroll(
                                                 deltaTime,
                                                 vector.value,
                                                 dvar->domain.value.min,
                                                 dvar->domain.value.max,
                                                 stepb,
                                                 SCROLL_XAXIS);
                vector.vector[1] = DevGui_UpdateFloatScroll(
                                                         deltaTime,
                                                         vector.vector[1],
                                                         dvar->domain.value.min,
                                                         dvar->domain.value.max,
                                                         stepb,
                                                         SCROLL_XAXIS);
                vector.vector[2] = DevGui_UpdateFloatScroll(
                                                         deltaTime,
                                                         vector.vector[2],
                                                         dvar->domain.value.min,
                                                         dvar->domain.value.max,
                                                         stepb,
                                                         SCROLL_XAXIS);
                vector.vector[3] = DevGui_UpdateFloatScroll(
                                                         deltaTime,
                                                         vector.vector[3],
                                                         dvar->domain.value.min,
                                                         dvar->domain.value.max,
                                                         stepb,
                                                         SCROLL_XAXIS);
            }
            else
            {
                vector.vector[devguiGlob.selRow] = DevGui_UpdateFloatScroll(
                                                                                         deltaTime,
                                                                                         vector.vector[devguiGlob.selRow],
                                                                                         dvar->domain.value.min,
                                                                                         dvar->domain.value.max,
                                                                                         stepb,
                                                                                         SCROLL_XAXIS);
            }
            if ( dvar->latched.value != vector.value
                || dvar->latched.vector[1] != vector.vector[1]
                || dvar->latched.vector[2] != vector.vector[2]
                || dvar->latched.vector[3] != vector.vector[3] )
            {
                Dvar_SetVec4FromSource(
                    (dvar_s *)dvar,
                    vector.value,
                    vector.vector[1],
                    vector.vector[2],
                    vector.vector[3],
                    DVAR_SOURCE_DEVGUI);
            }
            break;
        case DVAR_TYPE_INT:
            intValue = DevGui_UpdateIntScroll(
                                     deltaTime,
                                     dvar->latched.integer,
                                     dvar->domain.enumeration.stringCount,
                                     dvar->domain.integer.max,
                                     SCROLL_XAXIS);
            if ( intValue != dvar->latched.integer )
                Dvar_SetIntFromSource((dvar_s *)dvar, intValue, DVAR_SOURCE_DEVGUI);
            break;
        case DVAR_TYPE_ENUM:
            intValue = DevGui_UpdateIntScroll(
                                     deltaTime,
                                     dvar->latched.integer,
                                     0,
                                     dvar->domain.enumeration.stringCount - 1,
                                     SCROLL_XAXIS);
            if ( intValue != dvar->latched.integer )
                Dvar_SetIntFromSource((dvar_s *)dvar, intValue, DVAR_SOURCE_DEVGUI);
            break;
        case DVAR_TYPE_COLOR:
            Byte4UnpackRgba((const unsigned __int8 *)&dvar->latched, color);
            floatValuea = color[devguiGlob.selRow];
            if ( channelLink )
            {
                color[0] = DevGui_UpdateFloatScroll(deltaTime, color[0], 0.0, 1.0, 0.019607844, SCROLL_XAXIS);
                color[1] = DevGui_UpdateFloatScroll(deltaTime, color[1], 0.0, 1.0, 0.019607844, SCROLL_XAXIS);
                color[2] = DevGui_UpdateFloatScroll(deltaTime, color[2], 0.0, 1.0, 0.019607844, SCROLL_XAXIS);
                color[3] = DevGui_UpdateFloatScroll(deltaTime, color[3], 0.0, 1.0, 0.019607844, SCROLL_XAXIS);
            }
            else
            {
                color[devguiGlob.selRow] = DevGui_UpdateFloatScroll(
                                                                         deltaTime,
                                                                         color[devguiGlob.selRow],
                                                                         0.0,
                                                                         1.0,
                                                                         0.019607844,
                                                                         SCROLL_XAXIS);
            }
            if ( floatValuea != color[devguiGlob.selRow] )
                Dvar_SetColorFromSource((dvar_s *)dvar, color[0], color[1], color[2], color[3], DVAR_SOURCE_DEVGUI);
            break;
        case DVAR_TYPE_INT64:
            int64Value = DevGui_UpdateIntScroll(
                                         deltaTime,
                                         dvar->latched.integer64,
                                         dvar->domain.integer64.min,
                                         dvar->domain.integer64.max,
                                         SCROLL_XAXIS);
            if ( int64Value != dvar->latched.integer64 )
                Dvar_SetInt64FromSource((dvar_s *)dvar, int64Value, DVAR_SOURCE_DEVGUI);
            break;
        default:
            v3 = va("invalid dvar type %i", dvar->type);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 2192, 1, v3) )
                __debugbreak();
            break;
    }
}

double __cdecl DevGui_PickFloatScrollStep(float min, float max)
{
    float range; // [esp+30h] [ebp-Ch]
    float step; // [esp+34h] [ebp-8h]
    float roundedStep; // [esp+38h] [ebp-4h]

    range = max - min;
    if ( max == (float)(int)(max + 9.313225746154785e-10) && min == (float)(int)(min + 9.313225746154785e-10) )
    {
        for ( step = 1.0f; range > (float)(step * 100.0); step = step * 2.0 )
            ;
        while ( (float)(step * 100.0) > range )
            step = step * 0.5;
    }
    else
    {
        step = range * 0.0099999998;
        roundedStep = (float)(int)((float)(range * 0.0099999998) + 9.313225746154785e-10);
        if ( roundedStep != 0.0 && fabs(roundedStep - step) < 0.1 )
            return (float)(int)((float)(range * 0.0099999998) + 9.313225746154785e-10);
    }
    return step;
}

void __cdecl DevGui_UpdateGraph(int localClientNum, float deltaTime)
{
    float v2; // [esp+18h] [ebp-7Ch]
    float v3; // [esp+20h] [ebp-74h]
    float v4; // [esp+24h] [ebp-70h]
    bool v5; // [esp+28h] [ebp-6Ch]
    float v6; // [esp+2Ch] [ebp-68h]
    float v7; // [esp+30h] [ebp-64h]
    float v8; // [esp+34h] [ebp-60h]
    float v9; // [esp+38h] [ebp-5Ch]
    float v10; // [esp+3Ch] [ebp-58h]
    float v11; // [esp+48h] [ebp-4Ch]
    float *v12; // [esp+50h] [ebp-44h]
    float *v13; // [esp+54h] [ebp-40h]
    __int16 xAxisDelta; // [esp+58h] [ebp-3Ch]
    bool graphUpdated; // [esp+5Fh] [ebp-35h]
    int currentKnotCount; // [esp+60h] [ebp-34h]
    DevGraph *graph; // [esp+6Ch] [ebp-28h]
    devguiGlob_t *menu; // [esp+74h] [ebp-20h]
    float updatedY; // [esp+78h] [ebp-1Ch]
    float updatedX; // [esp+84h] [ebp-10h]
    float knot; // [esp+88h] [ebp-Ch]
    float knot_4; // [esp+8Ch] [ebp-8h]
    float deltaY; // [esp+90h] [ebp-4h]

    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if ( !menu && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 2294, 0, "%s", "menu") )
        __debugbreak();
    if ( menu->menus[0].childType != 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    2295,
                    0,
                    "%s",
                    "menu->childType == DEV_CHILD_GRAPH") )
    {
        __debugbreak();
    }
    if ( !menu->menus[0].child.command
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 2296, 0, "%s", "menu->child.graph") )
    {
        __debugbreak();
    }
    graph = menu->menus[0].child.graph;
    if ( !graph && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 2300, 0, "%s", "graph") )
        __debugbreak();
    if ( !graph->knots
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 2300, 0, "%s", "graph->knots") )
    {
        __debugbreak();
    }
    if ( !graph->knotCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 2300, 0, "%s", "graph->knotCount") )
    {
        __debugbreak();
    }
    if ( graph->knotCountMax <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    2300,
                    0,
                    "%s",
                    "graph->knotCountMax > 0") )
    {
        __debugbreak();
    }
    currentKnotCount = *graph->knotCount;
    if ( (currentKnotCount < 2 || currentKnotCount > graph->knotCountMax)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    2303,
                    0,
                    "%s",
                    "currentKnotCount >= 2 && currentKnotCount <= graph->knotCountMax") )
    {
        __debugbreak();
    }
    if ( graph->selectedKnot < 0 || graph->selectedKnot >= currentKnotCount )
        graph->selectedKnot = 0;
    if ( DevGui_IsButtonPressed(INPUT_GRAPH_EDIT) )
        devguiGlob.editingKnot = !devguiGlob.editingKnot;
    if ( DevGui_IsButtonPressed(INPUT_GRAPH_ADD) )
    {
        DevGui_AddGraphKnot(graph, localClientNum);
        devguiGlob.editingKnot = 0;
    }
    else if ( DevGui_IsButtonPressed(INPUT_GRAPH_REMOVE) )
    {
        DevGui_RemoveGraphKnot(graph, localClientNum);
        devguiGlob.editingKnot = 0;
    }
    else if ( DevGui_IsButtonPressed(INPUT_GRAPH_SAVE) )
    {
        if ( graph->eventCallback )
            graph->eventCallback(graph, EVENT_SAVE, localClientNum);
    }
    else if ( DevGui_IsButtonPressed(INPUT_ACCEPT) )
    {
        if ( graph->eventCallback )
            graph->eventCallback(graph, EVENT_ACCEPT, localClientNum);
    }
    else if ( DevGui_IsButtonPressed(INPUT_REJECT) )
    {
        if ( graph->eventCallback )
            graph->eventCallback(graph, EVENT_DEACTIVATE, localClientNum);
        devguiGlob.editingMenuItem = 0;
    }
    else if ( devguiGlob.editingKnot )
    {
        v13 = graph->knots[graph->selectedKnot];
        knot = *v13;
        knot_4 = v13[1];
        updatedX = DevGui_UpdateFloatScroll(deltaTime, *v13, 0.0, 1.0, 0.029999999, SCROLL_XAXIS);
        updatedY = DevGui_UpdateFloatScroll(deltaTime, knot_4, 0.0, 1.0, 0.029999999, SCROLL_YAXIS);
        if ( graph->selectedKnot <= 0 || graph->selectedKnot + 1 >= currentKnotCount )
        {
            if ( graph->disableEditingEndPoints )
            {
                graphUpdated = 0;
            }
            else
            {
                graphUpdated = fabs(updatedY - knot_4) > 0.000001;
                graph->knots[graph->selectedKnot][1] = (float)((float)(updatedY - knot_4) * 4.0)
                                                                                         + graph->knots[graph->selectedKnot][1];
                v6 = graph->knots[graph->selectedKnot][1];
                if ( (float)(v6 - 1.0) < 0.0 )
                    v7 = graph->knots[graph->selectedKnot][1];
                else
                    v7 = 1.0f;
                if ( (float)(0.0 - v6) < 0.0 )
                    v2 = v7;
                else
                    v2 = 0.0f;
                graph->knots[graph->selectedKnot][1] = v2;
            }
        }
        else
        {
            deltaY = updatedY - knot_4;
            v5 = fabs(updatedX - knot) > 0.000001
                || fabs(deltaY) > 0.000001;
            graphUpdated = v5;
            graph->knots[graph->selectedKnot][0] = (float)((float)(updatedX - knot) * 3.0)
                                                                                     + graph->knots[graph->selectedKnot][0];
            graph->knots[graph->selectedKnot][1] = (float)(deltaY * 4.0) + graph->knots[graph->selectedKnot][1];
            v12 = graph->knots[graph->selectedKnot - 1];
            v10 = graph->knots[graph->selectedKnot][0];
            if ( (float)(v10 - (float)(graph->knots[graph->selectedKnot + 1][0] - 0.0049999999)) < 0.0 )
                v11 = graph->knots[graph->selectedKnot][0];
            else
                v11 = graph->knots[graph->selectedKnot + 1][0] - 0.0049999999;
            if ( (float)((float)(*v12 + 0.0049999999) - v10) < 0.0 )
                v4 = v11;
            else
                v4 = *v12 + 0.0049999999;
            graph->knots[graph->selectedKnot][0] = v4;
            v8 = graph->knots[graph->selectedKnot][1];
            if ( (float)(v8 - 1.0) < 0.0 )
                v9 = graph->knots[graph->selectedKnot][1];
            else
                v9 = 1.0f;
            if ( (float)(0.0 - v8) < 0.0 )
                v3 = v9;
            else
                v3 = 0.0f;
            graph->knots[graph->selectedKnot][1] = v3;
        }
        if ( graphUpdated && graph->eventCallback )
            graph->eventCallback(graph, EVENT_UPDATE, localClientNum);
    }
    else
    {
        xAxisDelta = DevGui_GetMenuScroll(SCROLL_XAXIS);
        if ( xAxisDelta >= 0 || graph->selectedKnot <= 0 )
        {
            if ( xAxisDelta > 0 && currentKnotCount > graph->selectedKnot + 1 )
                ++graph->selectedKnot;
        }
        else
        {
            --graph->selectedKnot;
        }
        DevGui_UpdateSelection();
    }
}

void __cdecl DevGui_AddGraphKnot(DevGraph *graph, int localClientNum)
{
    float *v2; // [esp+4h] [ebp-20h]
    float *v3; // [esp+Ch] [ebp-18h]
    float *v4; // [esp+10h] [ebp-14h]
    float averageX; // [esp+14h] [ebp-10h]
    float averageXa; // [esp+14h] [ebp-10h]
    int currentKnotCount; // [esp+18h] [ebp-Ch]
    int knotIndex; // [esp+1Ch] [ebp-8h]
    float averageY; // [esp+20h] [ebp-4h]
    float averageYa; // [esp+20h] [ebp-4h]

    if ( !graph && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 2205, 0, "%s", "graph") )
        __debugbreak();
    if ( !graph->knots
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 2205, 0, "%s", "graph->knots") )
    {
        __debugbreak();
    }
    if ( !graph->knotCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 2205, 0, "%s", "graph->knotCount") )
    {
        __debugbreak();
    }
    if ( graph->knotCountMax <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    2205,
                    0,
                    "%s",
                    "graph->knotCountMax > 0") )
    {
        __debugbreak();
    }
    if ( graph->selectedKnot < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    2206,
                    0,
                    "%s",
                    "graph->selectedKnot >= 0") )
    {
        __debugbreak();
    }
    if ( graph->selectedKnot >= *graph->knotCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    2207,
                    0,
                    "%s",
                    "graph->selectedKnot < *graph->knotCount") )
    {
        __debugbreak();
    }
    if ( *graph->knotCount == graph->knotCountMax )
    {
        Com_Printf(11, "^3Maximum number of knots have reached for this graph\n");
    }
    else
    {
        currentKnotCount = *graph->knotCount;
        if ( graph->selectedKnot + 1 >= currentKnotCount )
        {
            averageXa = (float)(graph->knots[graph->selectedKnot][0] + graph->knots[graph->selectedKnot - 1][0]) * 0.5;
            averageYa = (float)(graph->knots[graph->selectedKnot][1] + graph->knots[graph->selectedKnot - 1][1]) * 0.5;
            v2 = graph->knots[currentKnotCount];
            *v2 = *(v2 - 2);
            v2[1] = *(v2 - 1);
            graph->knots[currentKnotCount - 1][0] = averageXa;
            graph->knots[currentKnotCount - 1][1] = averageYa;
            ++*graph->knotCount;
        }
        else
        {
            averageX = (float)(graph->knots[graph->selectedKnot][0] + graph->knots[graph->selectedKnot + 1][0]) * 0.5;
            averageY = (float)(graph->knots[graph->selectedKnot][1] + graph->knots[graph->selectedKnot + 1][1]) * 0.5;
            for ( knotIndex = *graph->knotCount - 1; knotIndex >= graph->selectedKnot; --knotIndex )
            {
                v3 = graph->knots[knotIndex + 1];
                v4 = graph->knots[knotIndex];
                *v3 = *v4;
                v3[1] = v4[1];
            }
            graph->knots[graph->selectedKnot + 1][0] = averageX;
            graph->knots[++graph->selectedKnot][1] = averageY;
            ++*graph->knotCount;
        }
        if ( graph->eventCallback )
            graph->eventCallback(graph, EVENT_UPDATE, localClientNum);
    }
}

void __cdecl DevGui_RemoveGraphKnot(DevGraph *graph, int localClientNum)
{
    float *v2; // [esp+0h] [ebp-10h]
    int currentKnotCount; // [esp+8h] [ebp-8h]
    int knotIndex; // [esp+Ch] [ebp-4h]

    if ( !graph && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 2255, 0, "%s", "graph") )
        __debugbreak();
    if ( !graph->knots
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 2255, 0, "%s", "graph->knots") )
    {
        __debugbreak();
    }
    if ( !graph->knotCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp", 2255, 0, "%s", "graph->knotCount") )
    {
        __debugbreak();
    }
    if ( graph->knotCountMax <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    2255,
                    0,
                    "%s",
                    "graph->knotCountMax > 0") )
    {
        __debugbreak();
    }
    if ( graph->selectedKnot < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    2257,
                    0,
                    "%s",
                    "graph->selectedKnot >= 0") )
    {
        __debugbreak();
    }
    if ( graph->selectedKnot >= *graph->knotCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                    2258,
                    0,
                    "%s",
                    "graph->selectedKnot < *graph->knotCount") )
    {
        __debugbreak();
    }
    currentKnotCount = *graph->knotCount;
    if ( graph->selectedKnot && graph->selectedKnot + 1 != currentKnotCount )
    {
        for ( knotIndex = graph->selectedKnot; knotIndex < currentKnotCount - 1; ++knotIndex )
        {
            v2 = graph->knots[knotIndex];
            *v2 = v2[2];
            v2[1] = v2[3];
        }
        graph->knots[currentKnotCount - 1][0] = -1.0f;
        graph->knots[currentKnotCount - 1][1] = -1.0f;
        --*graph->knotCount;
        if ( graph->eventCallback )
            graph->eventCallback(graph, EVENT_UPDATE, localClientNum);
    }
}

void __cdecl DevGui_Toggle()
{
    if ( devguiGlob.topmostMenu.child.menu )
    {
        if ( devguiGlob.selectedMenu )
            goto LABEL_12;
        if ( devguiGlob.isActive
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                        2528,
                        0,
                        "%s",
                        "!devguiGlob.isActive") )
        {
            __debugbreak();
        }
        devguiGlob.selectedMenu = devguiGlob.topmostMenu.child.menu;
        if ( devguiGlob.topmostMenu.child.menu )
        {
            DevGui_SelectTopLevelChild();
            if ( devguiGlob.selectedMenu )
            {
LABEL_12:
                devguiGlob.isActive = !devguiGlob.isActive;
                if ( devguiGlob.isActive )
                    DevGui_SelectGamepad(0);
            }
        }
    }
    else if ( devguiGlob.isActive
                 && !Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui.cpp",
                             2522,
                             0,
                             "%s",
                             "!devguiGlob.isActive") )
    {
        __debugbreak();
    }
}

bool __cdecl DevGui_IsActive()
{
    return devguiGlob.isActive;
}

bool __cdecl DevGui_IsInitialized()
{
    return devguiGlob.isInitialized;
}

