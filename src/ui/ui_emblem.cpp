#include "ui_emblem.h"
#include <client/cl_compositing.h>
#include <client/cl_main.h>
#include "ui_utils.h"
#include <universal/com_expressions_eval.h>
#include <universal/offline.h>
#include <qcommon/com_clients.h>
#include "ui_shared.h"
#include <client/cl_keys.h>
#include <gfx_d3d/r_rendercmds.h>

CompositeEmblemLayer s_emblem[12];
CompositeEmblemLayer *s_selected;

bool s_emblemMoveDownRepeatEnabled;
bool s_emblemMoveUpRepeatEnabled;
bool s_emblemMoveLeftRepeatEnabled;
bool s_emblemMoveRightRepeatEnabled;

bool s_emblemScaleUpRepeatEnabled;
bool s_emblemScaleDownRepeatEnabled;
bool s_emblemRotateLeftRepeatEnabled;
bool s_emblemRotateRightRepeatEnabled;

int s_mouseOverColorIdx;

bool s_emblemIsTranslating;
bool s_emblemIsRotating;
bool s_emblemIsScaling;

float s_emblemOffsetX;
float s_emblemOffsetY;

int s_selectedLayer;

__int16 s_backgroundID = -1;

EmblemFilterState s_filterStates[4] =
{
  { "MENU_EMBLEM_ALL", 0u, 0u },
  { "MENU_EMBLEM_UNLOCKED", 0u, 1u },
  { "MENU_EMBLEM_PURCHASED", 2u, 0u },
  { "MENU_EMBLEM_AFFORDABLE", 4u, 1u }
};

void __cdecl UI_DrawCustomEmblem(int contextIndex, const rectDef_s *rect, const float *color)
{
    UI_DrawCustomEmblemInternal(contextIndex, rect, color, s_emblem, 12, 1);
}

void __cdecl UI_DrawCustomEmblemInternal(
                int contextIndex,
                const rectDef_s *rect,
                const float *color,
                CompositeEmblemLayer *layers,
                int layerCount,
                bool withSpinner)
{
    float x; // [esp+1Ch] [ebp-10h] BYREF
    float y; // [esp+20h] [ebp-Ch] BYREF
    float h; // [esp+24h] [ebp-8h] BYREF
    float w; // [esp+28h] [ebp-4h] BYREF
    int savedregs; // [esp+2Ch] [ebp+0h] BYREF

    x = rect->x;
    y = rect->y;
    w = rect->w;
    h = rect->h;
    ScrPlace_ApplyRect(&scrPlaceView[contextIndex], &x, &y, &w, &h, rect->horzAlign, rect->vertAlign);
    if ( !CL_CompositeDrawEmblemPhysical(x, y, w, h, color, layers, layerCount) && withSpinner )
        CL_DrawSpinnerPhysical(x, y, w, h, color);
}

void __cdecl UI_DrawEmblemSelector(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                const rectDef_s *rect,
                const float *color)
{
    char *ExpressionResultString; // eax
    CompositeEmblemLayer layer; // [esp+0h] [ebp-28h] BYREF
    ownerDrawDef_s *ownerDraw; // [esp+20h] [ebp-8h]
    __int16 id; // [esp+24h] [ebp-4h]

    ownerDraw = Item_GetOwnerDrawDef(item);
    if ( ownerDraw )
    {
        ExpressionResultString = GetExpressionResultString(localClientNum, item, &ownerDraw->dataExp);
        id = atoi(ExpressionResultString);
        memset(layer.pos, 0, 22);
        layer.colorIdx = BG_EmblemsGetIconDefaultColor(id);
        layer.icon = id;
        UI_DrawCustomEmblemInternal(contextIndex, rect, color, &layer, 1, 0);
    }
}

void __cdecl UI_DrawPlayerEmblemServer(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                const rectDef_s *rect,
                const float *color)
{
    char *ExpressionResultString; // eax
    GfxImage *PlayerEmblem; // eax
    ownerDrawDef_s *ownerDraw; // [esp+2Ch] [ebp-20h]
    float x; // [esp+30h] [ebp-1Ch] BYREF
    float y; // [esp+34h] [ebp-18h] BYREF
    float h; // [esp+38h] [ebp-14h] BYREF
    unsigned __int64 xuid; // [esp+3Ch] [ebp-10h]
    float w; // [esp+48h] [ebp-4h] BYREF

    ownerDraw = Item_GetOwnerDrawDef(item);
    if ( ownerDraw )
    {
        ExpressionResultString = GetExpressionResultString(localClientNum, item, &ownerDraw->dataExp);
        xuid = I_atoi64(ExpressionResultString);
        x = rect->x;
        y = rect->y;
        w = rect->w;
        h = rect->h;
        ScrPlace_ApplyRect(&scrPlaceView[contextIndex], &x, &y, &w, &h, rect->horzAlign, rect->vertAlign);
        PlayerEmblem = PCache_GetPlayerEmblem(-1, xuid);
        if ( PlayerEmblem )
            R_AddCmdDrawStretchComposite(x, y, w, h, 0.0, 0.0, 1.0, 1.0, color, PlayerEmblem);
    }
}

void __cdecl UI_DrawPlayerEmblem(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                const rectDef_s *rect,
                const float *color)
{
    char *ExpressionResultString; // eax
    ownerDrawDef_s *ownerDraw; // [esp+0h] [ebp-Ch]
    __int64 xuid; // [esp+4h] [ebp-8h]

    ownerDraw = Item_GetOwnerDrawDef(item);
    if ( ownerDraw )
    {
        ExpressionResultString = GetExpressionResultString(localClientNum, item, &ownerDraw->dataExp);
        xuid = I_atoi64(ExpressionResultString);
        UI_DrawPlayerEmblemByXuid(localClientNum, contextIndex, item, rect, color, xuid);
    }
}

void __cdecl UI_DrawPlayerEmblemByXuid(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                const rectDef_s *rect,
                const float *color,
                unsigned __int64 xuid)
{
    int localClientNumForMenu; // [esp+28h] [ebp-28h]
    GfxImage *image; // [esp+2Ch] [ebp-24h]
    int controllerIndex; // [esp+30h] [ebp-20h]
    uiInfo_s *uiInfo; // [esp+38h] [ebp-18h]
    PCachePlayerEmblem *emblem; // [esp+3Ch] [ebp-14h]
    float x; // [esp+40h] [ebp-10h] BYREF
    float y; // [esp+44h] [ebp-Ch] BYREF
    float h; // [esp+48h] [ebp-8h] BYREF
    float w; // [esp+4Ch] [ebp-4h] BYREF
    int savedregs; // [esp+50h] [ebp+0h] BYREF

    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( xuid )
    {
        uiInfo = UI_UIContext_GetInfo(contextIndex);
        if ( Menu_DoesMenuOrParentsHaveControlFlag(&uiInfo->uiDC, item->parent, 3) )
        {
            localClientNumForMenu = Menus_MenuInStackLocalClientNum(&uiInfo->uiDC, item->parent);
            if ( localClientNumForMenu != -1 )
                controllerIndex = Com_LocalClient_GetControllerIndex(localClientNumForMenu);
        }
        x = rect->x;
        y = rect->y;
        w = rect->w;
        h = rect->h;
        ScrPlace_ApplyRect(&scrPlaceView[contextIndex], &x, &y, &w, &h, rect->horzAlign, rect->vertAlign);
        if ( CL_CompositeUseGenerated(w, h) && (image = PCache_GetPlayerEmblem(controllerIndex, xuid)) != 0 )
        {
            R_AddCmdDrawStretchComposite(x, y, w, h, 0.0, 0.0, 1.0, 1.0, color, image);
        }
        else
        {
            PCache_Lock();
            emblem = (PCachePlayerEmblem *)PCache_GetComponent(controllerIndex, xuid, 1u);
            if ( (!PCache_TouchComponent(&emblem->c)
                 || !CL_CompositeDrawEmblemPhysical(x, y, w, h, color, emblem->layers, 12))
                && (emblem->c.state & 0x20) == 0 )
            {
                CL_DrawSpinnerPhysical(x, y, w, h, color);
            }
            PCache_Unlock();
        }
    }
}

void __cdecl UI_DrawEmblemIconThumbnail(int contextIndex, const rectDef_s *rect, const float *color)
{
    CompositeEmblemLayer layer; // [esp+0h] [ebp-20h] BYREF

    if ( s_selected )
    {
        memset(layer.pos, 0, 20);
        layer.colorIdx = s_selected->colorIdx;
        layer.icon = s_selected->icon;
        UI_DrawCustomEmblemInternal(contextIndex, rect, color, &layer, 1, 1);
    }
}

void __cdecl UI_DrawEmblemColors(int contextIndex, const rectDef_s *rect)
{
    float by; // [esp+28h] [ebp-54h]
    float bw; // [esp+30h] [ebp-4Ch]
    float v4; // [esp+34h] [ebp-48h]
    float h; // [esp+38h] [ebp-44h]
    float blockSize; // [esp+3Ch] [ebp-40h]
    float spaceSize; // [esp+44h] [ebp-38h]
    int numInRow; // [esp+48h] [ebp-34h]
    float mouseY; // [esp+50h] [ebp-2Ch]
    float dispX; // [esp+54h] [ebp-28h] BYREF
    Material *mat; // [esp+58h] [ebp-24h]
    int idx; // [esp+5Ch] [ebp-20h]
    float dispH; // [esp+60h] [ebp-1Ch] BYREF
    float dispY; // [esp+64h] [ebp-18h] BYREF
    float dispW; // [esp+68h] [ebp-14h] BYREF
    UiContext *dc; // [esp+6Ch] [ebp-10h]
    float halfSpaceSize; // [esp+70h] [ebp-Ch]
    int count; // [esp+74h] [ebp-8h]
    float mouseX; // [esp+78h] [ebp-4h]

    count = BG_EmblemsGetColorCount();
    numInRow = count / 4;
    dispX = rect->x;
    dispY = rect->y;
    dispW = rect->w;
    dispH = rect->h * 4.0;
    mat = Material_RegisterHandle((char*)"emblem_lut_2d", 3);
    ScrPlace_ApplyRect(&scrPlaceView[contextIndex], &dispX, &dispY, &dispW, &dispH, rect->horzAlign, rect->vertAlign);
    dc = (UiContext *)UI_GetInfo(0);
    mouseX = ScrPlace_ApplyX(&scrPlaceView[contextIndex], dc->cursor.x, 4);
    mouseY = ScrPlace_ApplyY(&scrPlaceView[contextIndex], dc->cursor.y, 4);
    blockSize = (float)((float)(1.0 - 0.2) * dispW) / (float)numInRow;
    spaceSize = (float)(dispW / (float)numInRow) - blockSize;
    halfSpaceSize = spaceSize * 0.5;
    for ( idx = 0; idx < count; ++idx )
    {
        v4 = (float)((float)(idx % numInRow) * (float)(blockSize + spaceSize)) + dispX;
        by = (float)((float)(idx / numInRow) * (float)(blockSize + spaceSize)) + dispY;
        h = blockSize;
        bw = blockSize;
        if ( s_selected && idx == s_selected->colorIdx )
        {
            bw = blockSize + halfSpaceSize;
            h = blockSize + halfSpaceSize;
            by = by - halfSpaceSize;
        }
        R_AddCmdDrawStretchPic(
            v4,
            by,
            bw,
            h,
            0.625,
            (float)((float)idx * 0.0078125) + 0.00390625,
            0.625,
            (float)((float)idx * 0.0078125) + 0.00390625,
            colorWhite,
            mat);
        if ( mouseX >= v4 && (float)(v4 + bw) > mouseX && mouseY >= by && (float)(by + h) > mouseY )
            s_mouseOverColorIdx = idx;
    }
}

void __cdecl UI_EmblemUpdate(int localClientNum)
{
    char *v1; // eax
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    float v11; // [esp+10h] [ebp-88h]
    float v12; // [esp+14h] [ebp-84h]
    float v13; // [esp+18h] [ebp-80h] BYREF
    float v14; // [esp+1Ch] [ebp-7Ch] BYREF
    float v15; // [esp+20h] [ebp-78h] BYREF
    float v16[3]; // [esp+24h] [ebp-74h] BYREF
    float x; // [esp+30h] [ebp-68h] BYREF
    float y; // [esp+34h] [ebp-64h] BYREF
    float w; // [esp+38h] [ebp-60h] BYREF
    float h[3]; // [esp+3Ch] [ebp-5Ch] BYREF
    float mouseOffsetX; // [esp+48h] [ebp-50h]
    float mouseY; // [esp+4Ch] [ebp-4Ch]
    float mouseOffsetY; // [esp+50h] [ebp-48h]
    float mouseX; // [esp+54h] [ebp-44h]
    rectDef_s r; // [esp+58h] [ebp-40h] BYREF
    float ANGULAR_VELOCITY; // [esp+70h] [ebp-28h]
    int controllerIndex; // [esp+74h] [ebp-24h]
    uiInfo_s *uiInfo; // [esp+78h] [ebp-20h]
    float timeScale; // [esp+7Ch] [ebp-1Ch]
    float deltaX; // [esp+80h] [ebp-18h]
    itemDef_s *previewItem; // [esp+84h] [ebp-14h]
    float MOVEMENT_VELOCITY; // [esp+88h] [ebp-10h]
    float SCALE_VELOCITY; // [esp+8Ch] [ebp-Ch]
    float deltaY; // [esp+90h] [ebp-8h]
    menuDef_t *emblemMenu; // [esp+94h] [ebp-4h]

    ANGULAR_VELOCITY = 72.0f;
    MOVEMENT_VELOCITY = 0.4f;
    SCALE_VELOCITY = 0.8f;
    uiInfo = UI_GetInfo(localClientNum);
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    timeScale = 1.0 / uiInfo->uiDC.FPS;
    previewItem = 0;
    emblemMenu = 0;
    if ( Key_IsDown(localClientNum, 200) && s_emblemMoveDownRepeatEnabled )
    {
        v1 = va("emblemMove 0 %f", (float)(timeScale * MOVEMENT_VELOCITY));
        Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, v1);
        s_emblemMoveUpRepeatEnabled = 0;
    }
    else if ( Key_IsDown(localClientNum, 200) && s_emblemMoveUpRepeatEnabled )
    {
        //v2 = va("emblemMove 0 %f", (float)(COERCE_FLOAT(LODWORD(MOVEMENT_VELOCITY) ^ _mask__NegFloat_) * timeScale));
        v2 = va("emblemMove 0 %f", -MOVEMENT_VELOCITY * timeScale);
        Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, v2);
        s_emblemMoveDownRepeatEnabled = 0;
    }
    else
    {
        s_emblemMoveUpRepeatEnabled = 0;
        s_emblemMoveDownRepeatEnabled = 0;
    }
    if ( Key_IsDown(localClientNum, 200) && s_emblemMoveRightRepeatEnabled )
    {
        v3 = va("emblemMove %f 0", (float)(timeScale * MOVEMENT_VELOCITY));
        Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, v3);
        s_emblemMoveLeftRepeatEnabled = 0;
    }
    else if ( Key_IsDown(localClientNum, 200) && s_emblemMoveLeftRepeatEnabled )
    {
        //v4 = va("emblemMove %f 0", (float)(COERCE_FLOAT(LODWORD(MOVEMENT_VELOCITY) ^ _mask__NegFloat_) * timeScale));
        v4 = va("emblemMove %f 0", -MOVEMENT_VELOCITY * timeScale);
        Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, v4);
        s_emblemMoveRightRepeatEnabled = 0;
    }
    else
    {
        s_emblemMoveLeftRepeatEnabled = 0;
        s_emblemMoveRightRepeatEnabled = 0;
    }
    if ( Key_IsDown(localClientNum, 200) && s_emblemScaleUpRepeatEnabled )
    {
        v5 = va("emblemScale %f %f", (float)(timeScale * SCALE_VELOCITY), (float)(timeScale * SCALE_VELOCITY));
        Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, v5);
        s_emblemScaleDownRepeatEnabled = 0;
    }
    else if ( Key_IsDown(localClientNum, 200) && s_emblemScaleDownRepeatEnabled )
    {
        v6 = va(
                     "emblemScale %f %f",
                     //(float)(COERCE_FLOAT(LODWORD(SCALE_VELOCITY) ^ _mask__NegFloat_) * timeScale),
                     -SCALE_VELOCITY * timeScale,
                     //(float)(COERCE_FLOAT(LODWORD(SCALE_VELOCITY) ^ _mask__NegFloat_) * timeScale));
                     -SCALE_VELOCITY * timeScale);
        Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, v6);
        s_emblemScaleUpRepeatEnabled = 0;
    }
    else
    {
        s_emblemScaleUpRepeatEnabled = 0;
        s_emblemScaleDownRepeatEnabled = 0;
    }
    if ( Key_IsDown(localClientNum, 200) && s_emblemRotateLeftRepeatEnabled )
    {
        v7 = va("emblemRotate -%f", (float)(timeScale * ANGULAR_VELOCITY));
        Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, v7);
        s_emblemRotateRightRepeatEnabled = 0;
    }
    else if ( Key_IsDown(localClientNum, 200) && s_emblemRotateRightRepeatEnabled )
    {
        v8 = va("emblemRotate %f", (float)(timeScale * ANGULAR_VELOCITY));
        Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, v8);
        s_emblemRotateLeftRepeatEnabled = 0;
    }
    else
    {
        s_emblemRotateLeftRepeatEnabled = 0;
        s_emblemRotateRightRepeatEnabled = 0;
    }
    if ( Key_IsDown(localClientNum, 200) && Key_IsDown(localClientNum, 201) )
    {
        emblemMenu = Menus_FindByName(&uiInfo->uiDC, "emblem_edit");
        if ( emblemMenu )
        {
            s_emblemIsRotating = 0;
            s_emblemIsTranslating = 0;
            previewItem = Menu_FindItemByName(emblemMenu, "emblem_preview");
            if ( previewItem )
            {
                if ( s_emblemIsTranslating
                    || Rect_ContainsPoint(
                             uiInfo->uiDC.contextIndex,
                             &previewItem->window.rect,
                             uiInfo->uiDC.cursor.x,
                             uiInfo->uiDC.cursor.y) )
                {
                    v12 = ScrPlace_ApplyY(&scrPlaceView[uiInfo->uiDC.contextIndex], uiInfo->uiDC.cursor.y, 4);
                    deltaY = v12 - ScrPlace_ApplyY(&scrPlaceView[uiInfo->uiDC.contextIndex], uiInfo->uiDC.prevCursor.y, 4);
                    r.x = previewItem->window.rect.x;
                    r.y = previewItem->window.rect.y;
                    r.w = previewItem->window.rect.w;
                    r.h = previewItem->window.rect.h;
                    ScrPlace_ApplyRect(
                        &scrPlaceView[localClientNum],
                        &r.x,
                        &r.y,
                        &r.w,
                        &r.h,
                        previewItem->window.rect.horzAlign,
                        previewItem->window.rect.vertAlign);
                    deltaY = deltaY / r.h;
                    deltaY = deltaY * -10.0;
                    v9 = va("emblemScale %f %f", deltaY, deltaY);
                    Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, v9);
                    s_emblemIsScaling = 1;
                }
            }
        }
    }
    else if ( s_selected && Key_IsDown(localClientNum, 200) )
    {
        emblemMenu = Menus_FindByName(&uiInfo->uiDC, "emblem_edit");
        if ( emblemMenu )
        {
            s_emblemIsRotating = 0;
            s_emblemIsScaling = 0;
            previewItem = Menu_FindItemByName(emblemMenu, "emblem_preview");
            if ( previewItem )
            {
                if ( s_emblemIsTranslating
                    || Rect_ContainsPoint(
                             uiInfo->uiDC.contextIndex,
                             &previewItem->window.rect,
                             uiInfo->uiDC.cursor.x,
                             uiInfo->uiDC.cursor.y) )
                {
                    mouseX = ScrPlace_ApplyX(&scrPlaceView[uiInfo->uiDC.contextIndex], uiInfo->uiDC.cursor.x, 4);
                    mouseY = ScrPlace_ApplyY(&scrPlaceView[uiInfo->uiDC.contextIndex], uiInfo->uiDC.cursor.y, 4);
                    x = previewItem->window.rect.x;
                    y = previewItem->window.rect.y;
                    w = previewItem->window.rect.w;
                    h[0] = previewItem->window.rect.h;
                    ScrPlace_ApplyRect(
                        &scrPlaceView[localClientNum],
                        &x,
                        &y,
                        &w,
                        h,
                        previewItem->window.rect.horzAlign,
                        previewItem->window.rect.vertAlign);
                    mouseOffsetY = mouseY - (float)((float)(h[0] * 0.5) + y);
                    mouseOffsetX = (float)(mouseX - (float)((float)(w * 0.5) + x)) / w;
                    mouseOffsetY = mouseOffsetY / h[0];
                    if ( !s_emblemIsTranslating )
                    {
                        s_emblemOffsetX = s_selected->pos[0] - mouseOffsetX;
                        s_emblemOffsetY = s_selected->pos[1] - mouseOffsetY;
                    }
                    s_selected->pos[0] = mouseOffsetX + s_emblemOffsetX;
                    s_selected->pos[1] = mouseOffsetY + s_emblemOffsetY;
                    UI_EmblemClampLayer(s_selected);
                    s_emblemIsTranslating = 1;
                }
            }
        }
    }
    else if ( Key_IsDown(localClientNum, 201) )
    {
        s_emblemIsTranslating = 0;
        s_emblemIsScaling = 0;
        emblemMenu = Menus_FindByName(&uiInfo->uiDC, "emblem_edit");
        if ( emblemMenu )
        {
            previewItem = Menu_FindItemByName(emblemMenu, "emblem_preview");
            if ( previewItem )
            {
                if ( s_emblemIsRotating
                    || Rect_ContainsPoint(
                             uiInfo->uiDC.contextIndex,
                             &previewItem->window.rect,
                             uiInfo->uiDC.cursor.x,
                             uiInfo->uiDC.cursor.y) )
                {
                    s_emblemIsRotating = 1;
                    v11 = ScrPlace_ApplyX(&scrPlaceView[uiInfo->uiDC.contextIndex], uiInfo->uiDC.cursor.x, 4);
                    deltaX = v11 - ScrPlace_ApplyX(&scrPlaceView[uiInfo->uiDC.contextIndex], uiInfo->uiDC.prevCursor.x, 4);
                    v13 = previewItem->window.rect.x;
                    v14 = previewItem->window.rect.y;
                    v15 = previewItem->window.rect.w;
                    v16[0] = previewItem->window.rect.h;
                    ScrPlace_ApplyRect(
                        &scrPlaceView[localClientNum],
                        &v13,
                        &v14,
                        &v15,
                        v16,
                        previewItem->window.rect.horzAlign,
                        previewItem->window.rect.vertAlign);
                    deltaX = (float)(180.0 / v15) * deltaX;
                    v10 = va("emblemRotate %f", deltaX);
                    Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, v10);
                }
            }
        }
    }
    else
    {
        s_emblemIsRotating = 0;
        s_emblemIsTranslating = 0;
        s_emblemIsScaling = 0;
    }
}

void __cdecl UI_EmblemClampLayer(CompositeEmblemLayer *layer)
{
    float v1; // [esp+0h] [ebp-30h]
    float v2; // [esp+4h] [ebp-2Ch]
    float v3; // [esp+8h] [ebp-28h]
    float v4; // [esp+Ch] [ebp-24h]
    float v5; // [esp+10h] [ebp-20h]
    float v6; // [esp+14h] [ebp-1Ch]
    float v7; // [esp+18h] [ebp-18h]
    float v8; // [esp+1Ch] [ebp-14h]
    float v9; // [esp+20h] [ebp-10h]
    float v10; // [esp+24h] [ebp-Ch]
    float v11; // [esp+28h] [ebp-8h]
    float v12; // [esp+2Ch] [ebp-4h]

    v11 = layer->pos[0];
    if ( (float)(v11 - 1.0) < 0.0 )
        v12 = layer->pos[0];
    else
        v12 = 1.0f;
    if ( (float)(-1.0 - v11) < 0.0 )
        v4 = v12;
    else
        v4 = -1.0f;
    layer->pos[0] = v4;
    v9 = layer->pos[1];
    if ( (float)(v9 - 1.0) < 0.0 )
        v10 = layer->pos[1];
    else
        v10 = 1.0f;
    if ( (float)(-1.0 - v9) < 0.0 )
        v3 = v10;
    else
        v3 = -1.0f;
    layer->pos[1] = v3;
    v7 = layer->scale[0];
    if ( (float)(v7 - 4.0) < 0.0 )
        v8 = layer->scale[0];
    else
        v8 = 4.0f;
    if ( (float)(-4.0 - v7) < 0.0 )
        v2 = v8;
    else
        v2 = -4.0f;
    layer->scale[0] = v2;
    v5 = layer->scale[1];
    if ( (float)(v5 - 4.0) < 0.0 )
        v6 = layer->scale[1];
    else
        v6 = 4.0f;
    if ( (float)(-4.0 - v5) < 0.0 )
        v1 = v6;
    else
        v1 = -4.0f;
    layer->scale[1] = v1;
    if ( layer->angle >= 360.0 )
        layer->angle = layer->angle - 360.0;
    if ( layer->angle < 0.0 )
        layer->angle = layer->angle + 360.0;
}

int __cdecl UI_EmblemGetSelectedLayer()
{
    return s_selectedLayer;
}

bool __cdecl UI_EmblemCanOutlineLayer(unsigned int layer)
{
    return layer < 0xC && s_emblem[layer].icon != -1 && BG_EmblemsGetIconOutlineSize(s_emblem[layer].icon) > 0.0;
}

bool __cdecl UI_EmblemCanDuplicateLayer(int controllerIndex)
{
    int i; // [esp+4h] [ebp-Ch]
    int purchasedCount; // [esp+8h] [ebp-8h]
    int emptyCount; // [esp+Ch] [ebp-4h]

    purchasedCount = BG_EmblemsGetPurchasedLayerCount(controllerIndex);
    emptyCount = 0;
    for ( i = 0; i < purchasedCount; ++i )
    {
        if ( s_emblem[i].icon == -1 )
            ++emptyCount;
    }
    return emptyCount > 0 && emptyCount < purchasedCount;
}

static char sTemp[64];
void __cdecl UI_EmblemDuplicate_f()
{
    int purchasedCount; // [esp+8h] [ebp-8h]
    int hole; // [esp+Ch] [ebp-4h]

    purchasedCount = BG_EmblemsGetPurchasedLayerCount(0);
    if ( UI_EmblemCanDuplicateLayer(0) )
    {
        for ( hole = 0; hole < purchasedCount && s_emblem[hole].icon != -1; ++hole )
            ;

        iassert(hole != purchasedCount);
        iassert(hole != s_selectedLayer);

        if ( hole >= s_selectedLayer )
        {
            while ( hole != s_selectedLayer )
            {
                s_emblem[hole] = s_emblem[hole + 1];
                --hole;
            }
        }
        else
        {
            while ( hole != s_selectedLayer )
            {
                s_emblem[hole] = s_emblem[hole - 1];
                ++hole;
            }
            s_selected = &s_emblem[--s_selectedLayer];
        }
    }
}

bool __cdecl UI_EmblemIsModified(int controllerIndex)
{
    unsigned __int64 v1; // rax
    bool v3; // [esp+8h] [ebp-194h]
    PCachePublicProfile *profile; // [esp+10h] [ebp-18Ch]
    CompositeEmblemLayer layers[12]; // [esp+14h] [ebp-188h] BYREF
    __int16 backgroundID; // [esp+198h] [ebp-4h] BYREF

    PCache_Lock();
    LODWORD(v1) = Live_GetXuid(controllerIndex);
    profile = (PCachePublicProfile *)PCache_GetComponent(controllerIndex, v1, 0);
    PCache_GetProfileEmblem(profile, layers, 12, &backgroundID);
    v3 = memcmp(layers, s_emblem, 0x180u) || backgroundID != s_backgroundID;
    PCache_Unlock();
    return v3;
}

const char *__cdecl UI_EmblemGetLayerName(unsigned int layer)
{
    const char *IconDesc; // eax

    if ( layer >= 0xC || s_emblem[layer].icon == -1 )
        return "";
    IconDesc = BG_EmblemsGetIconDesc(-1, s_emblem[layer].icon);
    return va("@%s", IconDesc);
}

__int16 __cdecl UI_EmblemGetSelectedBackgroundID()
{
    return s_backgroundID;
}

char *__cdecl UI_EmblemGetStateDisplay(int controllerIndex, int idx)
{
    return UI_SafeTranslateString(s_filterStates[(idx % 4 + 4) % 4].loc);
}

char *__cdecl UI_EmblemGetCategoryDisplay(int controllerIndex, int idx)
{
    int v2; // et2
    char *v3; // eax
    const char *CategoryDesc; // eax
    int IconCount; // [esp-4h] [ebp-8h]
    int count; // [esp+0h] [ebp-4h]
    int idxa; // [esp+10h] [ebp+Ch]

    count = BG_EmblemsGetCategoryCount() - 2;
    v2 = (count + idx % count) % count;
    idxa = v2 + 3;
    if ( v2 )
    {
        IconCount = BG_EmblemsGetIconCount(controllerIndex, 1 << (idxa - 1), 0);
        CategoryDesc = BG_EmblemsGetCategoryDesc(idxa - 1);
        v3 = UI_SafeTranslateString(CategoryDesc);
    }
    else
    {
        IconCount = BG_EmblemsGetIconCount(controllerIndex, 0, 0);
        v3 = UI_SafeTranslateString("EM_CATEGORY_ALL");
    }
    return va("%s (%d)", v3, IconCount);
}

int __cdecl UI_EmblemGetFilterCount(int controllerIndex, int state, int category)
{
    int v3; // eax
    int count; // [esp+4h] [ebp-8h]
    int categorya; // [esp+1Ch] [ebp+10h]

    count = BG_EmblemsGetCategoryCount() + 1;
    v3 = (state % 4 + 4) % 4;
    categorya = (count - 3 + category % (count - 3)) % (count - 3) + 3;
    if ( categorya == 3 )
        return BG_EmblemsGetIconCount(controllerIndex, s_filterStates[v3].include, s_filterStates[v3].exclude);
    else
        return BG_EmblemsGetIconCount(
                         controllerIndex,
                         s_filterStates[v3].include | (1 << (categorya - 1)),
                         s_filterStates[v3].exclude);
}

__int16 __cdecl UI_EmblemGetFilterIconID(int controllerIndex, int state, int category, int index)
{
    int v4; // eax
    int count; // [esp+4h] [ebp-8h]
    int categorya; // [esp+1Ch] [ebp+10h]

    count = BG_EmblemsGetCategoryCount() + 1;
    v4 = (state % 4 + 4) % 4;
    categorya = (count - 3 + category % (count - 3)) % (count - 3) + 3;
    if ( categorya == 3 )
        return BG_EmblemsGetIconID(controllerIndex, index, s_filterStates[v4].include, s_filterStates[v4].exclude);
    else
        return BG_EmblemsGetIconID(
                         controllerIndex,
                         index,
                         s_filterStates[v4].include | (1 << (categorya - 1)),
                         s_filterStates[v4].exclude);
}

void __cdecl UI_EmblemSelect(unsigned int layer)
{
    if ( layer >= 0xC )
    {
        s_selectedLayer = -1;
    }
    else
    {
        s_selectedLayer = layer;
        s_selected = &s_emblem[layer];
    }
}

void __cdecl UI_EmblemSelect_f()
{
    const char *v0; // eax
    unsigned int layer; // [esp+8h] [ebp-4h]

    v0 = Cmd_Argv(1);
    layer = atoi(v0);
    UI_EmblemSelect(layer);
}

void __cdecl UI_EmblemReset_f()
{
    if ( s_selected )
        CL_CompositeResetLayer(s_selected);
}

void __cdecl UI_EmblemClear_f()
{
    if ( s_selected )
        CL_CompositeClearLayer(s_selected);
}

void __cdecl UI_EmblemClearAll_f()
{
    int layer; // [esp+0h] [ebp-4h]

    for ( layer = 0; layer < 12; ++layer )
        CL_CompositeClearLayer(&s_emblem[layer]);
}

void __cdecl UI_EmblemRotate_f()
{
    const char *v0; // eax
    float v1; // [esp+0h] [ebp-Ch]

    if ( s_selected )
    {
        v0 = Cmd_Argv(1);
        v1 = atof(v0);
        s_selected->angle = v1 + s_selected->angle;
        UI_EmblemClampLayer(s_selected);
    }
}

void __cdecl UI_EmblemMove_f()
{
    const char *v0; // eax
    const char *v1; // eax
    float v2; // [esp+0h] [ebp-18h]
    float v3; // [esp+4h] [ebp-14h]

    if ( s_selected )
    {
        v0 = Cmd_Argv(1);
        v3 = atof(v0);
        s_selected->pos[0] = v3 + s_selected->pos[0];
        v1 = Cmd_Argv(2);
        v2 = atof(v1);
        s_selected->pos[1] = v2 + s_selected->pos[1];
        UI_EmblemClampLayer(s_selected);
    }
}

void __cdecl UI_EmblemScale_f()
{
    const char *v0; // eax
    const char *v1; // eax
    float v2; // [esp+0h] [ebp-18h]
    float v3; // [esp+4h] [ebp-14h]

    if ( s_selected )
    {
        v0 = Cmd_Argv(1);
        v3 = atof(v0);
        s_selected->scale[0] = v3 + s_selected->scale[0];
        v1 = Cmd_Argv(2);
        v2 = atof(v1);
        s_selected->scale[1] = v2 + s_selected->scale[1];
        UI_EmblemClampLayer(s_selected);
    }
}

void __cdecl UI_EmblemIcon_f()
{
    const char *v0; // eax

    if ( s_selected )
    {
        v0 = Cmd_Argv(1);
        s_selected->icon = atoi(v0);
        if ( s_selected->scale[0] == 0.0 || s_selected->scale[1] == 0.0 )
            UI_EmblemReset_f();
        s_selected->colorIdx = BG_EmblemsGetIconDefaultColor(s_selected->icon);
    }
}

void __cdecl UI_EmblemPalette_f()
{
    const char *v0; // eax

    if ( s_selected )
    {
        v0 = Cmd_Argv(1);
        s_selected->colorIdx = atoi(v0);
    }
}

void __cdecl UI_EmblemPaletteCycle_f()
{
    const char *v0; // eax
    int v1; // eax

    if ( s_selected )
    {
        v0 = Cmd_Argv(1);
        v1 = atoi(v0);
        s_selected->colorIdx = UI_EmblemCyclePalette(s_selected->colorIdx, v1);
    }
}

int __cdecl UI_EmblemCyclePalette(int index, int direction)
{
    int count; // [esp+0h] [ebp-4h]
    int indexa; // [esp+Ch] [ebp+8h]

    count = BG_EmblemsGetColorCount();
    indexa = direction + index;
    if ( indexa < 0 )
        indexa += count;
    return indexa % count;
}

void __cdecl UI_EmblemPalettePick_f()
{
    if ( s_selected )
    {
        if ( s_mouseOverColorIdx >= BG_EmblemsGetColorCount()
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_emblem.cpp",
                        975,
                        0,
                        "%s",
                        "s_mouseOverColorIdx < BG_EmblemsGetColorCount()") )
        {
            __debugbreak();
        }
        s_selected->colorIdx = s_mouseOverColorIdx;
    }
}

void __cdecl UI_EmblemOutline_f()
{
    const char *v0; // eax

    if ( s_selected )
    {
        v0 = Cmd_Argv(1);
        s_selected->outline = atoi(v0) > 0;
    }
}

void __cdecl UI_EmblemToggleOutline_f()
{
    if ( s_selected )
        s_selected->outline = !s_selected->outline;
}

void __cdecl UI_EmblemFlip_f()
{
    const char *v0; // eax

    if ( s_selected )
    {
        v0 = Cmd_Argv(1);
        s_selected->flip = atoi(v0) > 0;
    }
}

void __cdecl UI_EmblemToggleFlip_f()
{
    if ( s_selected )
        s_selected->flip = !s_selected->flip;
}

void __cdecl UI_EmblemMoveLayer_f()
{
    const char *v0; // eax
    int v1; // eax

    v0 = Cmd_Argv(1);
    v1 = atoi(v0);
    UI_EmblemMoveLayer(0, v1);
}

void __cdecl UI_EmblemMoveLayer(int controllerIndex, int targetz)
{
    signed int v2; // [esp+8h] [ebp-34h]
    int v3; // [esp+Ch] [ebp-30h]
    int v4; // [esp+10h] [ebp-2Ch]
    int currentz; // [esp+14h] [ebp-28h]
    CompositeEmblemLayer workLayer; // [esp+18h] [ebp-24h] BYREF
    int direction; // [esp+38h] [ebp-4h]

    if ( s_selected )
    {
        v3 = BG_EmblemsGetPurchasedLayerCount(controllerIndex) - 1;
        v4 = targetz < v3 ? targetz : v3;
        v2 = v4 > 0 ? v4 : 0;
        if ( v2 != s_selectedLayer )
        {
            currentz = s_selectedLayer;
            if ( v2 >= s_selectedLayer )
                direction = 1;
            else
                direction = -1;
            memcpy(&workLayer, &s_emblem[s_selectedLayer], sizeof(workLayer));
            while ( v2 != currentz )
            {
                memcpy(&s_emblem[currentz], &s_emblem[direction + currentz], sizeof(CompositeEmblemLayer));
                currentz += direction;
            }
            memcpy(&s_emblem[v2], &workLayer, sizeof(CompositeEmblemLayer));
            UI_EmblemSelect(v2);
            if ( s_selected != &s_emblem[v2]
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_emblem.cpp",
                            1048,
                            0,
                            "%s",
                            "s_selected == &s_emblem[targetz]") )
            {
                __debugbreak();
            }
        }
    }
}

void __cdecl UI_EmblemMoveLayerRelative_f()
{
    const char *v0; // eax
    int v1; // eax

    v0 = Cmd_Argv(1);
    v1 = atoi(v0);
    UI_EmblemMoveLayer(0, s_selectedLayer + v1);
}

void __cdecl UI_EmblemCopy_f()
{
    const char *v0; // eax
    int v1; // [esp+8h] [ebp-14h]
    int v2; // [esp+Ch] [ebp-10h]
    int src; // [esp+18h] [ebp-4h]

    if ( s_selected )
    {
        v0 = Cmd_Argv(1);
        src = atoi(v0);
        if ( src < 11 )
            v2 = src;
        else
            v2 = 11;
        if ( v2 > 0 )
            v1 = v2;
        else
            v1 = 0;
        if ( v1 != s_selectedLayer )
            memcpy(s_selected, &s_emblem[v1], sizeof(CompositeEmblemLayer));
    }
}

void __cdecl UI_EmblemBeginEdit_f()
{
    Key_AddCatcher(0, 128);
}

void __cdecl UI_EmblemEndEdit_f()
{
    Key_RemoveCatcher(0, -129);
}

void __cdecl UI_EmblemSetProfile_f()
{
    LiveCounter_IncrementCounterValueByName("global_emblem_created", 1u);
    PCache_SetProfileEmblem(0, s_emblem, 12, s_backgroundID);
}

void __cdecl UI_EmblemGetProfile_f()
{
    unsigned __int64 v0; // rax
    PCachePublicProfile *profile; // [esp+4h] [ebp-4h]

    PCache_Lock();
    LODWORD(v0) = Live_GetXuid(0);
    profile = (PCachePublicProfile *)PCache_GetComponent(0, v0, 0);
    PCache_GetProfileEmblem(profile, s_emblem, 12, &s_backgroundID);
    PCache_Unlock();
}

void __cdecl UI_EmblemSelectBackground_f()
{
    const char *v0; // eax
    int index; // [esp+8h] [ebp-4h]

    v0 = Cmd_Argv(1);
    index = atoi(v0);
    s_backgroundID = BG_EmblemsGetBackgroundID(index);
}

void __cdecl UI_EmblemDump_f()
{
    int idx; // [esp+30h] [ebp-8h]
    CompositeEmblemLayer *layer; // [esp+34h] [ebp-4h]

    Com_Printf(0, "\n// EMBLEM BEGIN\n");
    Com_Printf(0, "emblemClearAll;\n");
    for ( idx = 0; idx < 12; ++idx )
    {
        if ( s_emblem[idx].icon != -1 )
        {
            layer = &s_emblem[idx];
            Com_Printf(
                0,
                "emblemSelect %d; emblemIcon %d; emblemPalette %d; emblemMove %f %f; emblemScale %f %f; emblemRotate %f; emblemFl"
                "ip %d; emblemOutline %d;\n",
                idx,
                layer->icon,
                layer->colorIdx,
                layer->pos[0],
                layer->pos[1],
                layer->scale[0],
                layer->scale[1],
                layer->angle,
                layer->flip,
                layer->outline);
        }
    }
    Com_Printf(0, "emblemSetProfile;\n");
    Com_Printf(0, "// EMBLEM END\n\n");
}

void __cdecl UI_EmblemClearDefaults_f()
{
    BG_EmblemsClearDefaults();
}

void __cdecl UI_EmblemSetAsDefault_f()
{
    const char *v0; // eax
    const char *v1; // eax
    int maxlevel; // [esp+10h] [ebp-8h]
    int minlevel; // [esp+14h] [ebp-4h]

    v0 = Cmd_Argv(1);
    minlevel = atoi(v0);
    v1 = Cmd_Argv(2);
    maxlevel = atoi(v1);
    BG_EmblemsSetDefaultForLevel(minlevel, maxlevel, s_emblem, 12);
}

void __cdecl UI_EmblemMoveUpRepeatEnabled_f()
{
    s_emblemMoveUpRepeatEnabled = 1;
}

void __cdecl UI_EmblemMoveDownRepeatEnabled_f()
{
    s_emblemMoveDownRepeatEnabled = 1;
}

void __cdecl UI_EmblemMoveLeftRepeatEnabled_f()
{
    s_emblemMoveLeftRepeatEnabled = 1;
}

void __cdecl UI_EmblemMoveRightRepeatEnabled_f()
{
    s_emblemMoveRightRepeatEnabled = 1;
}

void __cdecl UI_EmblemScaleUpRepeatEnabled_f()
{
    s_emblemScaleUpRepeatEnabled = 1;
}

void __cdecl UI_EmblemScaleDownRepeatEnabled_f()
{
    s_emblemScaleDownRepeatEnabled = 1;
}

void __cdecl UI_EmblemRotateLeftRepeatEnabled_f()
{
    s_emblemRotateLeftRepeatEnabled = 1;
}

void __cdecl UI_EmblemRotateRightRepeatEnabled_f()
{
    s_emblemRotateRightRepeatEnabled = 1;
}

void __cdecl UI_EmblemRepeatBttnsLooseFocus_f()
{
    s_emblemMoveUpRepeatEnabled = 0;
    s_emblemMoveDownRepeatEnabled = 0;
    s_emblemMoveLeftRepeatEnabled = 0;
    s_emblemMoveRightRepeatEnabled = 0;
    s_emblemScaleUpRepeatEnabled = 0;
    s_emblemScaleDownRepeatEnabled = 0;
    s_emblemRotateLeftRepeatEnabled = 0;
    s_emblemRotateRightRepeatEnabled = 0;
}


cmd_function_s UI_EmblemSelect_f_VAR;
cmd_function_s UI_EmblemReset_f_VAR;
cmd_function_s UI_EmblemClearAll_f_VAR;
cmd_function_s UI_EmblemClear_f_VAR;
cmd_function_s UI_EmblemRotate_f_VAR;
cmd_function_s UI_EmblemToggleFlip_f_VAR;
cmd_function_s UI_EmblemFlip_f_VAR;
cmd_function_s UI_EmblemMove_f_VAR;
cmd_function_s UI_EmblemScale_f_VAR;
cmd_function_s UI_EmblemIcon_f_VAR;
cmd_function_s UI_EmblemPalette_f_VAR;
cmd_function_s UI_EmblemOutline_f_VAR;
cmd_function_s UI_EmblemToggleOutline_f_VAR;
cmd_function_s UI_EmblemMoveLayer_f_VAR;
cmd_function_s UI_EmblemMoveLayerRelative_f_VAR;
cmd_function_s UI_EmblemCopy_f_VAR;
cmd_function_s UI_EmblemBeginEdit_f_VAR;
cmd_function_s UI_EmblemEndEdit_f_VAR;
cmd_function_s UI_EmblemDuplicate_f_VAR;
cmd_function_s UI_EmblemPaletteCycle_f_VAR;
cmd_function_s UI_EmblemGetProfile_f_VAR;
cmd_function_s UI_EmblemSetProfile_f_VAR;
cmd_function_s UI_EmblemSelectBackground_f_VAR;
cmd_function_s UI_EmblemDump_f_VAR;
cmd_function_s UI_EmblemClearDefaults_f_VAR;
cmd_function_s UI_EmblemSetAsDefault_f_VAR;
cmd_function_s UI_EmblemMoveUpRepeatEnabled_f_VAR;
cmd_function_s UI_EmblemMoveDownRepeatEnabled_f_VAR;
cmd_function_s UI_EmblemMoveLeftRepeatEnabled_f_VAR;
cmd_function_s UI_EmblemMoveRightRepeatEnabled_f_VAR;
cmd_function_s UI_EmblemScaleUpRepeatEnabled_f_VAR;
cmd_function_s UI_EmblemScaleDownRepeatEnabled_f_VAR;
cmd_function_s UI_EmblemRotateLeftRepeatEnabled_f_VAR;
cmd_function_s UI_EmblemRotateRightRepeatEnabled_f_VAR;
cmd_function_s UI_EmblemRepeatBttnsLooseFocus_f_VAR;
cmd_function_s UI_EmblemPalettePick_f_VAR;

void __cdecl UI_EmblemRegisterCmds()
{
    Cmd_AddCommandInternal("emblemSelect", UI_EmblemSelect_f, &UI_EmblemSelect_f_VAR);
    Cmd_AddCommandInternal("emblemReset", UI_EmblemReset_f, &UI_EmblemReset_f_VAR);
    Cmd_AddCommandInternal("emblemClearAll", UI_EmblemClearAll_f, &UI_EmblemClearAll_f_VAR);
    Cmd_AddCommandInternal("emblemClear", UI_EmblemClear_f, &UI_EmblemClear_f_VAR);
    Cmd_AddCommandInternal("emblemRotate", UI_EmblemRotate_f, &UI_EmblemRotate_f_VAR);
    Cmd_AddCommandInternal("emblemToggleFlip", UI_EmblemToggleFlip_f, &UI_EmblemToggleFlip_f_VAR);
    Cmd_AddCommandInternal("emblemFlip", UI_EmblemFlip_f, &UI_EmblemFlip_f_VAR);
    Cmd_AddCommandInternal("emblemMove", UI_EmblemMove_f, &UI_EmblemMove_f_VAR);
    Cmd_AddCommandInternal("emblemScale", UI_EmblemScale_f, &UI_EmblemScale_f_VAR);
    Cmd_AddCommandInternal("emblemIcon", UI_EmblemIcon_f, &UI_EmblemIcon_f_VAR);
    Cmd_AddCommandInternal("emblemPalette", UI_EmblemPalette_f, &UI_EmblemPalette_f_VAR);
    Cmd_AddCommandInternal("emblemOutline", UI_EmblemOutline_f, &UI_EmblemOutline_f_VAR);
    Cmd_AddCommandInternal("emblemToggleOutline", UI_EmblemToggleOutline_f, &UI_EmblemToggleOutline_f_VAR);
    Cmd_AddCommandInternal("emblemMoveLayer", UI_EmblemMoveLayer_f, &UI_EmblemMoveLayer_f_VAR);
    Cmd_AddCommandInternal("emblemMoveLayerRelative", UI_EmblemMoveLayerRelative_f, &UI_EmblemMoveLayerRelative_f_VAR);
    Cmd_AddCommandInternal("emblemCopy", UI_EmblemCopy_f, &UI_EmblemCopy_f_VAR);
    Cmd_AddCommandInternal("emblemBeginEdit", UI_EmblemBeginEdit_f, &UI_EmblemBeginEdit_f_VAR);
    Cmd_AddCommandInternal("emblemEndEdit", UI_EmblemEndEdit_f, &UI_EmblemEndEdit_f_VAR);
    Cmd_AddCommandInternal("emblemDuplicate", UI_EmblemDuplicate_f, &UI_EmblemDuplicate_f_VAR);
    Cmd_AddCommandInternal("emblemPaletteCycle", UI_EmblemPaletteCycle_f, &UI_EmblemPaletteCycle_f_VAR);
    Cmd_AddCommandInternal("emblemGetProfile", UI_EmblemGetProfile_f, &UI_EmblemGetProfile_f_VAR);
    Cmd_AddCommandInternal("emblemSetProfile", UI_EmblemSetProfile_f, &UI_EmblemSetProfile_f_VAR);
    Cmd_AddCommandInternal("emblemSelectBackground", UI_EmblemSelectBackground_f, &UI_EmblemSelectBackground_f_VAR);
    Cmd_AddCommandInternal("emblemDump", UI_EmblemDump_f, &UI_EmblemDump_f_VAR);
    Cmd_AddCommandInternal("emblemClearDefaults", UI_EmblemClearDefaults_f, &UI_EmblemClearDefaults_f_VAR);
    Cmd_AddCommandInternal("emblemSetAsDefault", UI_EmblemSetAsDefault_f, &UI_EmblemSetAsDefault_f_VAR);
    Cmd_AddCommandInternal(
        "emblemMoveUpRepeatEnabled",
        UI_EmblemMoveUpRepeatEnabled_f,
        &UI_EmblemMoveUpRepeatEnabled_f_VAR);
    Cmd_AddCommandInternal(
        "emblemMoveDownRepeatEnabled",
        UI_EmblemMoveDownRepeatEnabled_f,
        &UI_EmblemMoveDownRepeatEnabled_f_VAR);
    Cmd_AddCommandInternal(
        "emblemMoveLeftRepeatEnabled",
        UI_EmblemMoveLeftRepeatEnabled_f,
        &UI_EmblemMoveLeftRepeatEnabled_f_VAR);
    Cmd_AddCommandInternal(
        "emblemMoveRightRepeatEnabled",
        UI_EmblemMoveRightRepeatEnabled_f,
        &UI_EmblemMoveRightRepeatEnabled_f_VAR);
    Cmd_AddCommandInternal(
        "emblemScaleUpRepeatEnabled",
        UI_EmblemScaleUpRepeatEnabled_f,
        &UI_EmblemScaleUpRepeatEnabled_f_VAR);
    Cmd_AddCommandInternal(
        "emblemScaleDownRepeatEnabled",
        UI_EmblemScaleDownRepeatEnabled_f,
        &UI_EmblemScaleDownRepeatEnabled_f_VAR);
    Cmd_AddCommandInternal(
        "emblemRotateLeftRepeatEnabled",
        UI_EmblemRotateLeftRepeatEnabled_f,
        &UI_EmblemRotateLeftRepeatEnabled_f_VAR);
    Cmd_AddCommandInternal(
        "emblemRotateRightRepeatEnabled",
        UI_EmblemRotateRightRepeatEnabled_f,
        &UI_EmblemRotateRightRepeatEnabled_f_VAR);
    Cmd_AddCommandInternal(
        "emblemRepeatBttnsLooseFocus",
        UI_EmblemRepeatBttnsLooseFocus_f,
        &UI_EmblemRepeatBttnsLooseFocus_f_VAR);
    Cmd_AddCommandInternal("emblemPalettePick", UI_EmblemPalettePick_f, &UI_EmblemPalettePick_f_VAR);
}

