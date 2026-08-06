#include "ui_screenshot.h"
#include <universal/assertive.h>
#include <universal/com_memory.h>

void __cdecl UI_Screenshot_Success(dwFileShareReadFileTask *task)
{
#ifdef KISAK_SCREENSHOT
    if ( task->buffer == s_buffer )
    {
        if ( s_state != SCREENSHOT_DOWNLOADING
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_screenshot.cpp",
                        67,
                        0,
                        "%s",
                        "s_state == SCREENSHOT_DOWNLOADING") )
        {
            __debugbreak();
        }
        s_state = SCREENSHOT_DOWNLOADED;
    }
#endif
}

void __cdecl UI_Screenshot_Failure(dwFileShareReadFileTask *task)
{
#ifdef KISAK_SCREENSHOT
    if ( task->buffer == s_buffer && s_state )
    {
        if ( s_state != SCREENSHOT_DOWNLOADING
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_screenshot.cpp",
                        78,
                        0,
                        "%s",
                        "s_state == SCREENSHOT_DOWNLOADING") )
        {
            __debugbreak();
        }
        s_state = SCREENSHOT_ERROR;
    }
#endif
}

void __cdecl UI_ScreenshotAlloc()
{
#ifdef KISAK_SCREENSHOT
    if ( s_buffer
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_screenshot.cpp", 87, 0, "%s", "s_buffer == NULL") )
    {
        __debugbreak();
    }
    s_buffer = Hunk_Alloc(0x100000u, "ScreenshotRead", 36);
#endif
}

void __cdecl UI_ScreenshotShutdown()
{
#ifdef KISAK_SCREENSHOT
    s_buffer = 0;
    if ( s_state == SCREENSHOT_DOWNLOADING )
        LiveStorage_FileShare_AbortOperation(0, FILESHARE_LOCATION_USERSTORAGE);
    if ( s_state != SCREENSHOT_CREATED )
        s_state = SCREENSHOT_IDLE;
#endif
}

void __cdecl UI_ScreenshotDecompressCallback(void *data)
{
#ifdef KISAK_SCREENSHOT
    const char *v1; // eax
    const char *v2; // eax
    int v3; // [esp+0h] [ebp-1Ch]
    int v4; // [esp+4h] [ebp-18h]
    int semaphore; // [esp+8h] [ebp-14h]
    int hr; // [esp+Ch] [ebp-10h]
    _D3DLOCKED_RECT rect; // [esp+14h] [ebp-8h] BYREF

    s_image = (GfxImage *)Z_VirtualAlloc(52, "TempImage", 23);
    Image_Create2DTexture_PC(s_image, *((_WORD *)data + 2), *((_WORD *)data + 4), 1u, 0, *((_D3DFORMAT *)data + 3));
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("s_image->texture.map->LockRect(0, &rect, 0, 0x00002000L)\n");
    hr = ((int (__stdcall *)(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int))s_image->texture.basemap->__vftable[1].Release)(
                 (GfxTexture)s_image->texture.basemap,
                 0,
                 &rect,
                 0,
                 0x2000);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v1 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_screenshot.cpp (%i) s_image->texture.map->LockRect(0, &rect, 0, 0x000020"
            "00L) failed: %s\n",
            122,
            v1);
    }
    memcpy((unsigned __int8 *)rect.pBits, *(unsigned __int8 **)data, 4 * *((unsigned int *)data + 2) * *((unsigned int *)data + 1));
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("s_image->texture.map->UnlockRect(0)\n");
    v4 = s_image->texture.basemap->__vftable[1].GetDevice(s_image->texture.basemap, 0);
    if ( v4 < 0 )
    {
        ++g_disableRendering;
        v2 = R_ErrorDescription(v4);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_screenshot.cpp (%i) s_image->texture.map->UnlockRect(0) failed: %s\n",
            124,
            v2);
    }
    Z_VirtualFree(*(void **)data, 23);
#endif
}

void __cdecl UI_ScreenshotUpdate(int localClientNum, int contextIndex)
{
#ifdef KISAK_SCREENSHOT
    if ( Sys_IsMainThread() )
    {
        switch ( s_state )
        {
            case SCREENSHOT_DOWNLOADED:
                R_BeginRemoteScreenUpdate();
                if ( UI_ScreenshotDecompress(localClientNum, contextIndex) )
                {
                    if ( !s_image
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_screenshot.cpp",
                                    256,
                                    0,
                                    "%s",
                                    "s_image != NULL") )
                    {
                        __debugbreak();
                    }
                    s_state = SCREENSHOT_CREATED;
                }
                else
                {
                    s_state = SCREENSHOT_ERROR;
                }
                R_EndRemoteScreenUpdate(0);
                break;
            case SCREENSHOT_CREATED:
                UI_ScreenshotHandleInput(localClientNum);
                if ( !Menu_IsMenuOpenAndVisible(localClientNum, "menu_screenshot") )
                {
                    Image_Release(s_image);
                    Z_VirtualFree(s_image, 23);
                    s_image = 0;
                    s_state = SCREENSHOT_IDLE;
                }
                break;
            case SCREENSHOT_ERROR:
                s_state = SCREENSHOT_IDLE;
                Com_Error(ERR_DROP, "EXE_ERR_OPENING_SCREENSHOT");
                break;
        }
    }
#endif
}

char __cdecl UI_ScreenshotDecompress(int localClientNum, int contextIndex)
{
#ifdef KISAK_SCREENSHOT
    uiInfo_s *Info; // eax
    ScreenshotDecompressParams Params; // [esp+0h] [ebp-14h] BYREF
    void *jpeg; // [esp+10h] [ebp-4h]

    if ( s_image
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_screenshot.cpp", 136, 0, "%s", "s_image == NULL") )
    {
        __debugbreak();
    }
    jpeg = R_ReadJpgSetup(s_buffer, s_jpegSize, &Params.width, &Params.height, &Params.format);
    if ( !jpeg )
        return 0;
    Params.jpeg = Z_VirtualAlloc(4 * Params.height * Params.width, "TempImage", 23);
    R_ReadJpg((jpeg_decompress_struct *)jpeg, (unsigned __int8 *)Params.jpeg, 4 * Params.width, Params.height);
    RB_Resource_CallbackParam(UI_ScreenshotDecompressCallback, &Params);
    RB_Resource_Flush();
    Info = UI_UIContext_GetInfo(contextIndex);
    UI_AnimateItem(localClientNum, &Info->uiDC, "menu_screenshot", "*", "expand", 500);
    return 1;
#else
    return 0;
#endif
}

void __cdecl UI_ScreenshotHandleInput(int localClientNum)
{
#ifdef KISAK_SCREENSHOT
    float v1; // xmm0_4
    float v2; // xmm0_4
    float v3; // [esp+8h] [ebp-18h]
    float v4; // [esp+Ch] [ebp-14h]
    uiInfo_s *uiInfo; // [esp+14h] [ebp-Ch]
    itemDef_s *previewItem; // [esp+18h] [ebp-8h]
    menuDef_t *emblemMenu; // [esp+1Ch] [ebp-4h]

    uiInfo = UI_GetInfo(localClientNum);
    emblemMenu = Menus_FindByName(&uiInfo->uiDC, "menu_screenshot");
    previewItem = Menu_FindItemByName(emblemMenu, "screenshot");
    if ( Rect_ContainsPoint(
                 uiInfo->uiDC.contextIndex,
                 &previewItem->window.rect,
                 uiInfo->uiDC.cursor.x,
                 uiInfo->uiDC.cursor.y) )
    {
        if ( !Key_IsDown(localClientNum, 200) || Key_IsDown(localClientNum, 201) )
        {
            if ( Key_IsDown(localClientNum, 200) && Key_IsDown(localClientNum, 201) )
                s_screenshotZoom = s_screenshotZoom
                                                 - (float)((float)((float)(uiInfo->uiDC.cursor.y - uiInfo->uiDC.prevCursor.y)
                                                                                 / (float)uiInfo->uiDC.screenHeight)
                                                                 * 8.0);
        }
        else
        {
            v1 = s_center_x
                 - (float)((float)(uiInfo->uiDC.cursor.x - uiInfo->uiDC.prevCursor.x) / previewItem->window.rect.w)
                 / (s_screenshotZoom + 1.0)
                 * (float)(uiInfo->uiDC.screenAspect / 1.3333334);
            s_center_x = v1;
            v2 = s_center_y
                 - (float)((float)(uiInfo->uiDC.cursor.y - uiInfo->uiDC.prevCursor.y) / previewItem->window.rect.h)
                 / (s_screenshotZoom + 1.0);
            s_center_y = v2;
        }
        if ( (float)(s_screenshotZoom - 1.0) < 0.0 )
            v4 = s_screenshotZoom;
        else
            v4 = 1.0f;
        if ( (float)(0.0 - s_screenshotZoom) < 0.0 )
            v3 = v4;
        else
            v3 = 0.0f;
        s_screenshotZoom = v3;
    }
#endif
}

void __cdecl UI_ScreenshotDownload(int localClientNum, int context, unsigned __int64 fileID, unsigned int fileSize)
{
#ifdef KISAK_SCREENSHOT
    int ControllerIndex; // eax
    fileShareReadFileInfo fileInfo; // [esp+0h] [ebp-30h] BYREF

    if ( s_state
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_screenshot.cpp",
                    294,
                    0,
                    "%s",
                    "s_state == SCREENSHOT_IDLE") )
    {
        __debugbreak();
    }
    if ( !s_buffer )
        UI_ScreenshotAlloc();
    s_jpegSize = fileSize;
    s_state = SCREENSHOT_DOWNLOADING;
    s_cancel = 0;
    fileInfo.fileID = fileID;
    fileInfo.fileSize = fileSize;
    fileInfo.location = FILESHARE_LOCATION_USERSTORAGE;
    fileInfo.buffer = (char *)s_buffer;
    fileInfo.cacheBuffer = s_buffer;
    fileInfo.bufferSize = fileSize;
    fileInfo.isStreamed = 0;
    fileInfo.successCallback = UI_Screenshot_Success;
    fileInfo.failureCallback = UI_Screenshot_Failure;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    s_downloadTask = LiveStorage_FileShare_ReadFile(ControllerIndex, &fileInfo);
#endif
}

void __cdecl UI_ScreenshotDraw(int localClientNum, int contextIndex, const rectDef_s *rect, const float *color)
{
#ifdef KISAK_SCREENSHOT
    float v4; // [esp+28h] [ebp-40h]
    float v5; // [esp+2Ch] [ebp-3Ch]
    float v6; // [esp+34h] [ebp-34h]
    float v7; // [esp+3Ch] [ebp-2Ch]
    float Stick; // [esp+40h] [ebp-28h]
    float zoom; // [esp+44h] [ebp-24h]
    uiInfo_s *uiInfo; // [esp+48h] [ebp-20h]
    float size; // [esp+4Ch] [ebp-1Ch]
    int idx; // [esp+50h] [ebp-18h]
    float x; // [esp+54h] [ebp-14h] BYREF
    float y; // [esp+58h] [ebp-10h] BYREF
    float h; // [esp+5Ch] [ebp-Ch] BYREF
    float dy; // [esp+60h] [ebp-8h]
    float w; // [esp+64h] [ebp-4h] BYREF

    idx = Com_LocalClient_GetControllerIndex(localClientNum);
    zoom = GPad_GetButton(idx, GPAD_R_TRIG);
    if ( zoom < 0.05f )
        zoom = s_screenshotZoom;
    uiInfo = UI_UIContext_GetInfo(contextIndex);
    size = 0.5 / (float)(zoom + 1.0);
    Stick = GPad_GetStick(idx, GPAD_LX);
    dy = GPad_GetStick(idx, GPAD_LY);
    if ( fabs(Stick) > 0.1 )
        s_center_x = (float)((float)((float)uiInfo->uiDC.frameTime / 1000.0) * Stick) + s_center_x;
    if ( fabs(dy) > 0.1 )
        s_center_y = s_center_y - (float)((float)((float)uiInfo->uiDC.frameTime / 1000.0) * dy);
    if ( (float)(s_center_x - (float)(1.0 - size)) < 0.0 )
        v7 = s_center_x;
    else
        v7 = 1.0 - size;
    if ( (float)(size - s_center_x) < 0.0 )
        v5 = v7;
    else
        v5 = 0.5 / (float)(zoom + 1.0);
    s_center_x = v5;
    if ( (float)(s_center_y - (float)(1.0 - size)) < 0.0 )
        v6 = s_center_y;
    else
        v6 = 1.0 - size;
    if ( (float)(size - s_center_y) < 0.0 )
        v4 = v6;
    else
        v4 = 0.5 / (float)(zoom + 1.0);
    s_center_y = v4;
    if ( s_state == SCREENSHOT_CREATED )
    {
        if ( !s_image
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_screenshot.cpp", 357, 0, "%s", "s_image") )
        {
            __debugbreak();
        }
        x = rect->x;
        y = rect->y;
        w = rect->w;
        h = rect->h;
        ScrPlace_ApplyRect(&scrPlaceView[contextIndex], &x, &y, &w, &h, rect->horzAlign, rect->vertAlign);
        R_AddCmdDrawStretchComposite(
            x,
            y,
            w,
            h,
            s_center_x - size,
            s_center_y - size,
            s_center_x + size,
            s_center_y + size,
            color,
            s_image);
    }
#endif
}

void __cdecl UI_ScreenshotInit()
{
#ifdef KISAK_SCREENSHOT
    if ( !s_inited )
    {
        Cmd_AddCommandInternal("screenshotDownload", UI_ScreenshotDownload_f, &UI_ScreenshotDownload_f_VAR);
        Cmd_AddCommandInternal("screenshotAbortDownload", UI_ScreenshotAbortDownload_f, &UI_ScreenshotAbortDownload_f_VAR);
        Cmd_AddCommandInternal("screenshotZoom", UI_ScreenshotZoom_f, &UI_ScreenshotZoom_f_VAR);
    }
    s_inited = 1;
#endif
}

void __cdecl UI_ScreenshotDownload_f()
{
#ifdef KISAK_SCREENSHOT
    const char *v0; // eax
    const char *v1; // eax
    int UIContextIndex; // eax
    unsigned __int64 v3; // [esp-Ch] [ebp-1Ch]
    unsigned int v4; // [esp-4h] [ebp-14h]

    v0 = Cmd_Argv(2);
    v4 = atoi(v0);
    v1 = Cmd_Argv(1);
    v3 = I_atoi64(v1);
    UIContextIndex = Com_ControllerIndex_GetUIContextIndex(0);
    UI_ScreenshotDownload(0, UIContextIndex, v3, v4);
#endif
}

void __cdecl UI_ScreenshotAbortDownload_f()
{
#ifdef KISAK_SCREENSHOT
    LiveStorage_FileShare_AbortOperation(0, FILESHARE_LOCATION_USERSTORAGE);
    if ( s_state != SCREENSHOT_CREATED )
        s_state = SCREENSHOT_IDLE;
#endif
}

void __cdecl UI_ScreenshotZoom_f()
{
#ifdef KISAK_SCREENSHOT
    const char *v0; // eax
    float v1; // [esp+0h] [ebp-1Ch]
    float v2; // [esp+4h] [ebp-18h]
    float v3; // [esp+8h] [ebp-14h]

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        v2 = atof(v0);
        s_screenshotZoom = v2 + s_screenshotZoom;
        if ( (float)(s_screenshotZoom - 1.0) < 0.0 )
            v3 = s_screenshotZoom;
        else
            v3 = 1.0f;
        if ( (float)(0.0 - s_screenshotZoom) < 0.0 )
            v1 = v3;
        else
            v1 = 0.0f;
        s_screenshotZoom = v1;
    }
#endif
}

