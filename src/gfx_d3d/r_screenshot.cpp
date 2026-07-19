#include <CubeMapGenLib/CCubeMapProcessor.h>

#include "r_screenshot.h"
#include "r_init.h"
#include <universal/com_memory.h>
#include <qcommon/common.h>
#include "r_dvars.h"
#include "rb_logfile.h"
#include "r_utils.h"
#include <d3dx9.h>
#include <universal/com_files.h>
#include <qcommon/cmd.h>
#include "rb_resource.h"
#include "r_jpeg.h"
#include "r_state.h"
#include "rb_backend.h"

const float cubemapShotAxis[7][3][3] =
{
  { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } },
  { { 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 } },
  { { 0.0, 0.0, -1.0 }, { -1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 } },
  { { 0.0, 1.0, 0.0 }, { -1.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 } },
  { { 0.0, -1.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 } },
  { { 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }, { 0.0, 0.0, 1.0 } },
  { { -1.0, 0.0, 0.0 }, { 0.0, -1.0, 0.0 }, { 0.0, 0.0, 1.0 } }
};

unsigned int exponenttable[64];
unsigned int mantissatable[2048];
unsigned int offsettable[64];

struct //$069481853E63AB7B456FCC17924CBF75 // sizeof=0x48
{                                       // XREF: .data:cubeShotGlob/r
    unsigned __int8 *pixels[6];         // XREF: R_CubemapShotWriteTargaFile+BD/r
    float *pixelsHDR[6];                // XREF: R_CopyCubemapShot+75/w
    float *pixelsHDRcorrected[6];       // XREF: R_CopyCubemapShot+C5/w
} cubeShotGlob;

unsigned __int8 *__cdecl R_TakeResampledScreenshot(int width, int height, int bytesPerPixel, int headerSize)
{
    unsigned int displayHeight; // [esp+0h] [ebp-14h]
    unsigned int displayWidth; // [esp+4h] [ebp-10h]
    unsigned __int8 *buffer; // [esp+8h] [ebp-Ch]

    if ( width < (int)vidConfig.displayWidth )
        displayWidth = vidConfig.displayWidth;
    else
        displayWidth = width;
    if ( height < (int)vidConfig.displayHeight )
        displayHeight = vidConfig.displayHeight;
    else
        displayHeight = height;
    buffer = (unsigned __int8 *)Z_Malloc(headerSize + bytesPerPixel * displayHeight * displayWidth, "R_ScreenShot", 23);
    if ( R_GetFrontBufferData(0, 0, vidConfig.displayWidth, vidConfig.displayHeight, bytesPerPixel, buffer) )
    {
        R_ResampleImage(vidConfig.displayWidth, vidConfig.displayHeight, width, height, bytesPerPixel, &buffer[headerSize]);
        return buffer;
    }
    else
    {
        Z_Free((char *)buffer, 23);
        return 0;
    }
}

void __cdecl R_ResampleImage(
                int oldWidth,
                int oldHeight,
                int newWidth,
                int newHeight,
                int bytesPerPixel,
                unsigned __int8 *data)
{
    unsigned __int8 *src; // [esp+0h] [ebp-10h]
    unsigned __int8 *srca; // [esp+0h] [ebp-10h]
    unsigned __int8 *srcb; // [esp+0h] [ebp-10h]
    unsigned __int8 *srcc; // [esp+0h] [ebp-10h]
    int row; // [esp+4h] [ebp-Ch]
    int rowa; // [esp+4h] [ebp-Ch]
    unsigned __int8 *dst; // [esp+8h] [ebp-8h]
    unsigned __int8 *dsta; // [esp+8h] [ebp-8h]
    unsigned __int8 *dstb; // [esp+8h] [ebp-8h]
    unsigned __int8 *dstc; // [esp+8h] [ebp-8h]
    int col; // [esp+Ch] [ebp-4h]
    int cola; // [esp+Ch] [ebp-4h]

    if ( oldWidth <= newWidth )
    {
        if ( oldWidth < newWidth )
        {
            srca = &data[bytesPerPixel * oldWidth * (oldHeight - 1)];
            dsta = &data[bytesPerPixel * newWidth * (oldHeight - 1)];
            for ( rowa = oldHeight - 1; rowa >= 0; --rowa )
            {
                R_UpsamplePixelData(oldWidth, newWidth, 1, bytesPerPixel, srca, dsta);
                srca -= bytesPerPixel * oldWidth;
                dsta -= bytesPerPixel * newWidth;
            }
        }
    }
    else
    {
        src = data;
        dst = data;
        for ( row = 0; row < oldHeight; ++row )
        {
            R_DownsamplePixelData(oldWidth, newWidth, 1, bytesPerPixel, src, dst);
            src += bytesPerPixel * oldWidth;
            dst += bytesPerPixel * newWidth;
        }
    }
    if ( oldHeight <= newHeight )
    {
        if ( oldHeight < newHeight )
        {
            srcc = &data[bytesPerPixel * (newWidth - 1)];
            dstc = srcc;
            for ( cola = newWidth - 1; cola >= 0; --cola )
            {
                R_UpsamplePixelData(oldHeight, newHeight, newWidth, bytesPerPixel, srcc, dstc);
                srcc -= bytesPerPixel;
                dstc -= bytesPerPixel;
            }
        }
    }
    else
    {
        srcb = data;
        dstb = data;
        for ( col = 0; col < newWidth; ++col )
        {
            R_DownsamplePixelData(oldHeight, newHeight, newWidth, bytesPerPixel, srcb, dstb);
            srcb += bytesPerPixel;
            dstb += bytesPerPixel;
        }
    }
}

void __cdecl R_DownsamplePixelData(
                int oldSize,
                int newSize,
                int stride,
                int bytesPerPixel,
                unsigned __int8 *src,
                unsigned __int8 *dst)
{
    int nextSample; // [esp+3Ch] [ebp-1Ch]
    float colorScale; // [esp+40h] [ebp-18h]
    int residual; // [esp+44h] [ebp-14h]
    int column; // [esp+48h] [ebp-10h]
    int color; // [esp+4Ch] [ebp-Ch]
    int color_4; // [esp+50h] [ebp-8h]
    int color_4a; // [esp+50h] [ebp-8h]
    int color_8; // [esp+54h] [ebp-4h]
    int color_8a; // [esp+54h] [ebp-4h]

    colorScale = 1.0 / (float)oldSize;
    nextSample = bytesPerPixel * stride;
    residual = newSize;
    for ( column = 0; column < newSize; ++column )
    {
        color = residual * *src;
        color_4 = residual * src[1];
        color_8 = residual * src[2];
        src += nextSample;
        while ( newSize + residual - oldSize <= 0 )
        {
            color += newSize * *src;
            color_4 += newSize * src[1];
            color_8 += newSize * src[2];
            residual += newSize;
            src += nextSample;
        }
        residual = newSize + residual - oldSize;
        color_4a = color_4 + src[1] * (newSize - residual);
        color_8a = color_8 + src[2] * (newSize - residual);
        *dst = (int)((float)((float)(color + *src * (newSize - residual)) * colorScale) + 9.313225746154785e-10);
        dst[1] = (int)((float)((float)color_4a * colorScale) + 9.313225746154785e-10);
        dst[2] = (int)((float)((float)color_8a * colorScale) + 9.313225746154785e-10);
        dst += nextSample;
    }
}

void __cdecl R_UpsamplePixelData(
                int oldSize,
                int newSize,
                int stride,
                int bytesPerPixel,
                unsigned __int8 *src,
                unsigned __int8 *dst)
{
    double v6; // st7
    int backwardWeight; // [esp+78h] [ebp-18h]
    int nextSample; // [esp+7Ch] [ebp-14h]
    float colorScale; // [esp+80h] [ebp-10h]
    unsigned __int8 *currSrc; // [esp+84h] [ebp-Ch]
    int column; // [esp+88h] [ebp-8h]
    int forwardWeight; // [esp+8Ch] [ebp-4h]
    unsigned __int8 *dsta; // [esp+ACh] [ebp+1Ch]

    nextSample = bytesPerPixel * stride;
    currSrc = &src[bytesPerPixel * stride * (oldSize - 1)];
    dsta = &dst[bytesPerPixel * stride * (newSize - 1)];
    currSrc[bytesPerPixel * stride] = *currSrc;
    currSrc[nextSample + 1] = currSrc[1];
    currSrc[nextSample + 2] = currSrc[2];
    forwardWeight = newSize - oldSize;
    backwardWeight = oldSize + newSize;
    colorScale = 0.5 / (float)newSize;
    for ( column = newSize - 1; column >= 0; --column )
    {
        if ( currSrc < src )
        {
            *dsta = (int)((float)((float)((forwardWeight + backwardWeight) * *src) * colorScale) + 9.313225746154785e-10);
            dsta[1] = (int)((float)((float)((forwardWeight + backwardWeight) * src[1]) * colorScale) + 9.313225746154785e-10);
            v6 = (float)((float)((forwardWeight + backwardWeight) * src[2]) * colorScale) + 9.313225746154785e-10;
        }
        else
        {
            *dsta = (int)((float)((float)(forwardWeight * currSrc[nextSample] + backwardWeight * *currSrc) * colorScale)
                                    + 9.313225746154785e-10);
            dsta[1] = (int)((float)((float)(forwardWeight * currSrc[nextSample + 1] + backwardWeight * currSrc[1]) * colorScale)
                                        + 9.313225746154785e-10);
            v6 = (float)((float)(forwardWeight * currSrc[nextSample + 2] + backwardWeight * currSrc[2]) * colorScale)
                 + 9.313225746154785e-10;
        }
        dsta[2] = (int)v6;
        dsta -= nextSample;
        backwardWeight += 2 * oldSize;
        forwardWeight -= 2 * oldSize;
        if ( forwardWeight < 0 )
        {
            backwardWeight -= 2 * newSize;
            forwardWeight += 2 * newSize;
            currSrc -= nextSample;
        }
    }
}

char __cdecl R_GetFrontBufferData(int x, int y, int width, int height, int bytesPerPixel, unsigned __int8 *buffer)
{
    const char *v7; // eax
    const char *v8; // eax
    HRESULT v9; // [esp-4h] [ebp-7Ch]
    HRESULT v10; // [esp-4h] [ebp-7Ch]
    IDirect3DSurface9 *v11; // [esp+0h] [ebp-78h]
    IDirect3DSurface9 *var; // [esp+4h] [ebp-74h]
    IDirect3DSurface9 *varCopy; // [esp+8h] [ebp-70h]
    tagMONITORINFO monitorInfo; // [esp+Ch] [ebp-6Ch] BYREF
    HRESULT hr; // [esp+34h] [ebp-44h]
    _D3DLOCKED_RECT lockedRect; // [esp+38h] [ebp-40h] BYREF
    unsigned __int8 *dstPixel; // [esp+40h] [ebp-38h]
    const unsigned __int8 *srcPixel; // [esp+44h] [ebp-34h]
    HMONITOR__ *monitor; // [esp+48h] [ebp-30h]
    int surfHeight; // [esp+4Ch] [ebp-2Ch]
    tagRECT sourceRect; // [esp+50h] [ebp-28h] BYREF
    int surfWidth; // [esp+60h] [ebp-18h]
    int row; // [esp+64h] [ebp-14h]
    tagPOINT pt; // [esp+68h] [ebp-10h] BYREF
    IDirect3DSurface9 *surface; // [esp+70h] [ebp-8h] BYREF
    int col; // [esp+74h] [ebp-4h]

    surface = 0;
    pt.x = x;
    pt.y = y;
    if ( vidConfig.isFullscreen )
    {
        surfWidth = vidConfig.displayWidth;
        surfHeight = vidConfig.displayHeight;
    }
    else
    {
        monitor = MonitorFromWindow((HWND)dx.windows[0].swapChain, 2u);
        monitorInfo.cbSize = 40;
        if ( !GetMonitorInfoA(monitor, &monitorInfo) )
        {
            Com_PrintError(8, "ERROR: cannot take screenshot: couldn't get screen dimensions\n");
            return 0;
        }
        ClientToScreen((HWND)dx.windows[0].swapChain, &pt);
        if ( pt.x < monitorInfo.rcMonitor.left
            || pt.y < monitorInfo.rcMonitor.top
            || width + pt.x > monitorInfo.rcMonitor.right
            || height + pt.y > monitorInfo.rcMonitor.bottom )
        {
            Com_PrintError(8, "ERROR: cannot take screenshot: game window is partially off-screen\n");
            return 0;
        }
        surfWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
        surfHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
        pt.x -= monitorInfo.rcMonitor.left;
        pt.y -= monitorInfo.rcMonitor.top;
    }
    //hr = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, int, int, int, IDirect3DSurface9 **, unsigned int))dx.device->CreateOffscreenPlainSurface)(
    //             dx.device,
    //             dx.device,
    //             surfWidth,
    //             surfHeight,
    //             21,
    //             3,
    //             &surface,
    //             0);
    hr = dx.device->CreateOffscreenPlainSurface(surfWidth, surfHeight, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &surface, NULL);
    if ( hr < 0 )
        goto LABEL_12;
    // The decompiler labeled this object 'width', but at this vtable offset (index 4)
    // it is the window's swap chain; call GetFrontBufferData(surface) on it. Reading
    // the int 'width' field as a pointer (as decompiled) dereferences garbage.
    hr = dx.windows[0].swapChain->GetFrontBufferData(surface);
    if ( hr < 0 )
    {
        do
        {
            if ( r_logFile )
            {
                if ( r_logFile->current.integer )
                    RB_LogPrint("surface->Release()\n");
            }
            varCopy = surface;
            surface = 0;
            R_ReleaseAndSetNULL<IDirect3DIndexBuffer9>(
                varCopy,
                "surface",
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                383);
        }
        while ( alwaysfails );
LABEL_12:
        v9 = hr;
        v7 = R_ErrorDescription(hr);
        Com_PrintError(8, "ERROR: cannot take screenshot: couldn't create the off-screen surface: %s (0x%08x)\n", v7, v9);
        return 0;
    }
    sourceRect.left = pt.x;
    sourceRect.right = width + pt.x;
    sourceRect.top = pt.y;
    sourceRect.bottom = height + pt.y;
    hr = surface->LockRect(&lockedRect, &sourceRect, 16u);
    if ( hr >= 0 )
    {
        srcPixel = (const unsigned __int8 *)lockedRect.pBits;
        dstPixel = buffer;
        if ( bytesPerPixel == 3 )
        {
            for ( row = 0; row < height; ++row )
            {
                for ( col = 0; col < width; ++col )
                {
                    *dstPixel = *srcPixel;
                    dstPixel[1] = srcPixel[1];
                    dstPixel[2] = srcPixel[2];
                    dstPixel += 3;
                    srcPixel += 4;
                }
                srcPixel += lockedRect.Pitch - 4 * width;
            }
        }
        else if ( lockedRect.Pitch == 4 * width )
        {
            Com_Memcpy(dstPixel, srcPixel, 4 * height * width);
        }
        else
        {
            for ( row = 0; row < height; ++row )
            {
                Com_Memcpy(dstPixel, srcPixel, 4 * width);
                dstPixel += 4 * width;
                srcPixel += lockedRect.Pitch;
            }
        }
        surface->UnlockRect();
        do
        {
            if ( r_logFile && r_logFile->current.integer )
                RB_LogPrint("surface->Release()\n");
            v11 = surface;
            surface = 0;
            R_ReleaseAndSetNULL<IDirect3DIndexBuffer9>(
                v11,
                "surface",
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                444);
        }
        while ( alwaysfails );
        return 1;
    }
    else
    {
        do
        {
            if ( r_logFile && r_logFile->current.integer )
                RB_LogPrint("surface->Release()\n");
            var = surface;
            surface = 0;
            R_ReleaseAndSetNULL<IDirect3DIndexBuffer9>(
                var,
                "surface",
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                395);
        }
        while ( alwaysfails );
        v10 = hr;
        v8 = R_ErrorDescription(hr);
        Com_PrintError(8, "ERROR: cannot take screenshot: LockRect failed: %s (0x%08x)\n", v8, v10);
        return 0;
    }
}

void __cdecl R_TakeScreenshotJpgCallback(char *param)
{
    R_TakeScreenshot(param, D3DXIFF_JPG);
}

char __cdecl R_TakeScreenshot(char *filename, int format)
{
    const char *v3; // eax
    HRESULT v4; // [esp-4h] [ebp-170h]
    IDirect3DSurface9 *var; // [esp+0h] [ebp-16Ch]
    IDirect3DSurface9 *varCopy; // [esp+4h] [ebp-168h]
    tagMONITORINFO monitorInfo; // [esp+8h] [ebp-164h] BYREF
    HRESULT hr; // [esp+30h] [ebp-13Ch]
    int width; // [esp+34h] [ebp-138h]
    int height; // [esp+38h] [ebp-134h]
    char ospath[260]; // [esp+3Ch] [ebp-130h] BYREF
    HMONITOR__ *monitor; // [esp+144h] [ebp-28h]
    int surfHeight; // [esp+148h] [ebp-24h]
    tagRECT sourceRect; // [esp+14Ch] [ebp-20h] BYREF
    int surfWidth; // [esp+15Ch] [ebp-10h]
    tagPOINT pt; // [esp+160h] [ebp-Ch] BYREF
    IDirect3DSurface9 *surface; // [esp+168h] [ebp-4h] BYREF

    surface = 0;
    width = vidConfig.displayWidth;
    height = vidConfig.displayHeight;
    pt.x = 0;
    pt.y = 0;
    if ( vidConfig.isFullscreen )
    {
        surfWidth = vidConfig.displayWidth;
        surfHeight = vidConfig.displayHeight;
    }
    else
    {
        monitor = MonitorFromWindow((HWND)dx.windows[0].swapChain, 2u);
        monitorInfo.cbSize = 40;
        if ( !GetMonitorInfoA(monitor, &monitorInfo) )
        {
            Com_PrintError(8, "ERROR: cannot take screenshot: couldn't get screen dimensions\n");
            return 0;
        }
        ClientToScreen((HWND)dx.windows[0].swapChain, &pt);
        if ( pt.x < monitorInfo.rcMonitor.left
            || pt.y < monitorInfo.rcMonitor.top
            || width + pt.x > monitorInfo.rcMonitor.right
            || height + pt.y > monitorInfo.rcMonitor.bottom )
        {
            Com_PrintError(8, "ERROR: cannot take screenshot: game window is partially off-screen\n");
            return 0;
        }
        surfWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
        surfHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
        pt.x -= monitorInfo.rcMonitor.left;
        pt.y -= monitorInfo.rcMonitor.top;
    }
    //hr = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, int, int, int, IDirect3DSurface9 **, unsigned int))dx.device->CreateOffscreenPlainSurface)(
    //             dx.device,
    //             dx.device,
    //             surfWidth,
    //             surfHeight,
    //             21,
    //             3,
    //             &surface,
    //             0);
    hr = dx.device->CreateOffscreenPlainSurface(surfWidth, surfHeight, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &surface, NULL);
    if ( hr < 0 )
        goto LABEL_12;
    // The decompiler labeled this object 'width', but at this vtable offset (index 4)
    // it is the window's swap chain; call GetFrontBufferData(surface) on it. Reading
    // the int 'width' field as a pointer (as decompiled) dereferences garbage.
    hr = dx.windows[0].swapChain->GetFrontBufferData(surface);
    if ( hr < 0 )
    {
        do
        {
            if ( r_logFile )
            {
                if ( r_logFile->current.integer )
                    RB_LogPrint("surface->Release()\n");
            }
            varCopy = surface;
            surface = 0;
            R_ReleaseAndSetNULL<IDirect3DIndexBuffer9>(
                varCopy,
                "surface",
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                953);
        }
        while ( alwaysfails );
LABEL_12:
        v4 = hr;
        v3 = R_ErrorDescription(hr);
        Com_PrintError(8, "ERROR: cannot take screenshot: couldn't create the off-screen surface: %s (0x%08x)\n", v3, v4);
        return 0;
    }
    sourceRect.left = pt.x;
    sourceRect.right = width + pt.x;
    sourceRect.top = pt.y;
    sourceRect.bottom = height + pt.y;
    FS_BuildOSPath(fs_gamedir, 0, filename, ospath);
    if ( !FS_CreatePath(ospath) )
        D3DXSaveSurfaceToFileA(ospath, (D3DXIMAGE_FILEFORMAT)format, surface, 0, &sourceRect);
    do
    {
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint("surface->Release()\n");
        var = surface;
        surface = 0;
        R_ReleaseAndSetNULL<IDirect3DIndexBuffer9>(
            var,
            "surface",
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
            968);
    }
    while ( alwaysfails );
    return 1;
}

void __cdecl R_TakeScreenshotTgaCallback(char *param)
{
    R_TakeScreenshot(param, D3DXIFF_TGA);
}

static char filename[256];
static int lastNumber = 0;
void __cdecl R_ScreenshotCommand(GfxScreenshotType type)
{
    const char *v2; // eax
    const char *v3; // eax
    int v4; // [esp+8h] [ebp-84h]
    const char *extension; // [esp+88h] [ebp-4h]

    if ( type )
    {
        if ( type != R_SCREENSHOT_TGA )
        {
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp", 1004, 1, "unhandled case") )
                __debugbreak();
            return;
        }
        extension = "tga";
    }
    else
    {
        extension = "jpg";
    }
    if ( !strcmp(Cmd_Argv(1), "levelshot") )
    {
        R_LevelShot();
        return;
    }
    if ( !strcmp(Cmd_Argv(1), "savegame") && Cmd_Argc() == 3 && *Cmd_Argv(2) )
    {
        v2 = Cmd_Argv(2);
        R_SaveGameShot(v2);
        return;
    }
    v4 = strcmp(Cmd_Argv(1), "silent");
    if ( Cmd_Argc() == 2 && v4 )
    {
        v3 = Cmd_Argv(1);
        Com_sprintf(filename, 0x100u, "screenshots/%s.%s", v3, extension);
    }
    else
    {
        while ( lastNumber <= 9999 )
        {
            R_ScreenshotFilename(lastNumber, extension, filename);
            if ( !FS_FileExists(filename) )
                break;
            ++lastNumber;
        }
        if ( lastNumber >= 9999 )
        {
            Com_Printf(8, "ScreenShot: Couldn't create a file\n");
            return;
        }
        ++lastNumber;
    }
    if ( type )
        RB_Resource_CallbackParam((void (__cdecl *)(void *))R_TakeScreenshotTgaCallback, filename);
    else
        RB_Resource_CallbackParam((void (__cdecl *)(void *))R_TakeScreenshotJpgCallback, filename);
    if ( v4 )
        Com_Printf(8, "Wrote %s\n", filename);
}

void __cdecl R_ScreenshotFilename(unsigned int lastNumber, const char *extension, char *fileName)
{
    if ( lastNumber < 0x2710 )
        Com_sprintf(fileName, 0x100u, "screenshots/shot%04i.%s", lastNumber, extension);
    else
        Com_sprintf(fileName, 0x100u, "screenshots/shot9999.%s", extension);
}

void R_LevelShot()
{
    unsigned __int8 *buffer; // [esp+0h] [ebp-10Ch]
    char checkname[260]; // [esp+4h] [ebp-108h] BYREF

    sprintf(checkname, "levelshots/%s.tga", rgp.world->baseName);
    buffer = R_TakeResampledScreenshot(128, 128, 3, 18);
    if ( buffer )
    {
        *(unsigned int *)buffer = 0;
        *((unsigned int *)buffer + 1) = 0;
        *((unsigned int *)buffer + 2) = 0;
        *((unsigned int *)buffer + 3) = 0;
        *((_WORD *)buffer + 8) = 0;
        buffer[2] = 2;
        buffer[12] = 0x80;
        buffer[14] = 0x80;
        buffer[16] = 24;
        FS_WriteFile(checkname, (char *)buffer, 0xC012u);
        Z_Free((char *)buffer, 23);
        Com_Printf(8, "Wrote %s\n", checkname);
    }
}

void __cdecl R_SaveGameShot(const char *saveName)
{
    char filename[256]; // [esp+0h] [ebp-110h] BYREF
    unsigned __int8 *pixels; // [esp+104h] [ebp-Ch]
    int width; // [esp+108h] [ebp-8h]
    int height; // [esp+10Ch] [ebp-4h]

    width = 512;
    height = 512;
    sprintf(filename, "%s.jpg", saveName);
    pixels = R_TakeResampledScreenshot(width, height, 3, 0);
    if ( pixels )
    {
        R_SaveJpg(filename, 90, width, height, pixels, 0);
        Z_Free((char *)pixels, 23);
    }
}

void __cdecl R_BeginHiResShot(int tiles)
{
    rg.renderHiResShot = 1;
    rg.hiResShotTiles = tiles;
}

void __cdecl R_UpdateHiResShot(float row, float col)
{
    rg.hiResShotRow = row;
    rg.hiResShotCol = col;
}

void __cdecl R_EndHiResShot()
{
    rg.renderHiResShot = 0;
    rg.hiResShotTiles = 0;
}

void __cdecl R_BeginCubemapShot(int pixelWidthHeight, int pixelBorder)
{
    if ( pixelWidthHeight <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                    1459,
                    0,
                    "%s\n\t(pixelWidthHeight) = %i",
                    "(pixelWidthHeight > 0)",
                    pixelWidthHeight) )
    {
        __debugbreak();
    }
    if ( pixelWidthHeight >= 0x10000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                    1460,
                    0,
                    "%s\n\t(pixelWidthHeight) = %i",
                    "(pixelWidthHeight < 65536)",
                    pixelWidthHeight) )
    {
        __debugbreak();
    }
    if ( pixelBorder < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                    1461,
                    0,
                    "%s\n\t(pixelBorder) = %i",
                    "(pixelBorder >= 0)",
                    pixelBorder) )
    {
        __debugbreak();
    }
    if ( pixelBorder >= pixelWidthHeight
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                    1462,
                    0,
                    "%s\n\t(pixelBorder) = %i",
                    "(pixelBorder < pixelWidthHeight)",
                    pixelBorder) )
    {
        __debugbreak();
    }
    gfxMetrics.cubemapShotRes = pixelWidthHeight;
    gfxMetrics.cubemapShotPixelBorder = pixelBorder;
    R_CubemapShotSetInitialState();
}

int R_CubemapShotSetInitialState()
{
  const char *v0; // eax
  int result; // eax
  int hr; // [esp+Ch] [ebp-8h]

  R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
  R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
  if ( r_logFile && r_logFile->current.integer )
    RB_LogPrint(
      "dx.device->Clear( 0, 0, 0x00000001l | 0x00000002l | 0x00000004l, ((D3DCOLOR)((((255)&0xff)<<24)|(((255)&0xff)<<16)"
      "|(((0)&0xff)<<8)|((255)&0xff))), (( 1/(((((( 1/(((((0 || 0 || ( 1/(((-0x7fffffff))/(0x7fffffff)) == 1)) ? (0x7ffff"
      "fff) : (-0x7fffffff)) * (((0 || 0 || ( 1/(((-0x7fffffff))/(0x7fffffff)) == 1))&~1) == 0)))/(0x7fffffff)) == 1) || "
      "( 1/(((((( 1/(((((( 1/(((-0x7fffffff))/(0x7fffffff)) == 1) || 0 || ( 1/(((-0x7fffffff))/(0x7fffffff)) == 1)) ? (0x"
      "7fffffff) : (-0x7fffffff)) * (((( 1/(((-0x7fffffff))/(0x7fffffff)) == 1) || 0 || ( 1/(((-0x7fffffff))/(0x7fffffff)"
      ") == 1))&~1) == 0)))/(0x7fffffff)) == 1) && 1) ? (0x7fffffff) : (-0x7fffffff)) * (((( 1/(((((( 1/(((-0x7fffffff))/"
      "(0x7fffffff)) == 1) || 0 || ( 1/(((-0x7fffffff))/(0x7fffffff)) == 1)) ? (0x7fffffff) : (-0x7fffffff)) * (((( 1/((("
      "-0x7fffffff))/(0x7fffffff)) == 1) || 0 || ( 1/(((-0x7fffffff))/(0x7fffffff)) == 1))&~1) == 0)))/(0x7fffffff)) == 1"
      ") && 1)&~1) == 0)))/(0x7fffffff)) == 1)) ? (0x7fffffff) : (-0x7fffffff)) * (((( 1/(((((0 || 0 || ( 1/(((-0x7ffffff"
      "f))/(0x7fffffff)) == 1)) ? (0x7fffffff) : (-0x7fffffff)) * (((0 || 0 || ( 1/(((-0x7fffffff))/(0x7fffffff)) == 1))&"
      "~1) == 0)))/(0x7fffffff)) == 1) || ( 1/(((((( 1/(((((( 1/(((-0x7fffffff))/(0x7fffffff)) == 1) || 0 || ( 1/(((-0x7f"
      "ffffff))/(0x7fffffff)) == 1)) ? (0x7fffffff) : (-0x7fffffff)) * (((( 1/(((-0x7fffffff))/(0x7fffffff)) == 1) || 0 |"
      "| ( 1/(((-0x7fffffff))/(0x7fffffff)) == 1))&~1) == 0)))/(0x7fffffff)) == 1) && 1) ? (0x7fffffff) : (-0x7fffffff)) "
      "* (((( 1/(((((( 1/(((-0x7fffffff))/(0x7fffffff)) == 1) || 0 || ( 1/(((-0x7fffffff))/(0x7fffffff)) == 1)) ? (0x7fff"
      "ffff) : (-0x7fffffff)) * (((( 1/(((-0x7fffffff))/(0x7fffffff)) == 1) || 0 || ( 1/(((-0x7fffffff))/(0x7fffffff)) =="
      " 1))&~1) == 0)))/(0x7fffffff)) == 1) && 1)&~1) == 0)))/(0x7fffffff)) == 1))&~1) == 0)))/(0x7fffffff)) == 1) ? 0.0f : 1.0f), 0 )\n");
  //hr = ((int (__stdcall *)(IDirect3DDevice9 *, unsigned int, unsigned int, int, int, unsigned int, unsigned int))dx.device->Clear)(
  //       dx.device,
  //       0,
  //       0,
  //       7,
  //       -65281,
  //       1.0,
  //       0);
  hr = dx.device->Clear(0, 0, 7, -65281, 1.0, 0);
  if ( hr < 0 )
  {
    ++g_disableRendering;
    v0 = R_ErrorDescription(hr);
    Com_Error(
      ERR_FATAL,
      "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp (%i) dx.device->Clear( 0, 0, 0x00000001l | 0x00000002"
      "l | 0x00000004l, ((D3DCOLOR)((((255)&0xff)<<24)|(((255)&0xff)<<16)|(((0)&0xff)<<8)|((255)&0xff))), (( 1/(((((( 1/("
      "((((0 || 0 || ( 1/(((-0x7fffffff))/(0x7fffffff)) == 1)) ? (0x7fffffff) : (-0x7fffffff)) * (((0 || 0 || ( 1/(((-0x7"
      "fffffff))/(0x7fffffff)) == 1))&~1) == 0)))/(0x7fffffff)) == 1) || ( 1/(((((( 1/(((((( 1/(((-0x7fffffff))/(0x7fffff"
      "ff)) == 1) || 0 || ( 1/(((-0x7fffffff))/(0x7fffffff)) == 1)) ? (0x7fffffff) : (-0x7fffffff)) * (((( 1/(((-0x7fffff"
      "ff))/(0x7fffffff)) == 1) || 0 || ( 1/(((-0x7fffffff))/(0x7fffffff)) == 1))&~1) == 0)))/(0x7fffffff)) == 1) && 1) ?"
      " (0x7fffffff) : (-0x7fffffff)) * (((( 1/(((((( 1/(((-0x7fffffff))/(0x7fffffff)) == 1) || 0 || ( 1/(((-0x7fffffff))"
      "/(0x7fffffff)) == 1)) ? (0x7fffffff) : (-0x7fffffff)) * (((( 1/(((-0x7fffffff))/(0x7fffffff)) == 1) || 0 || ( 1/(("
      "(-0x7fffffff))/(0x7fffffff)) == 1))&~1) == 0)))/(0x7fffffff)) == 1) && 1)&~1) == 0)))/(0x7fffffff)) == 1)) ? (0x7f"
      "ffffff) : (-0x7fffffff)) * (((( 1/(((((0 || 0 || ( 1/(((-0x7fffffff))/(0x7fffffff)) == 1)) ? (0x7fffffff) : (-0x7f"
      "ffffff)) * (((0 || 0 || ( 1/(((-0x7fffffff))/(0x7fffffff)) == 1))&~1) == 0)))/(0x7fffffff)) == 1) || ( 1/(((((( 1/"
      "(((((( 1/(((-0x7fffffff))/(0x7fffffff)) == 1) || 0 || ( 1/(((-0x7fffffff))/(0x7fffffff)) == 1)) ? (0x7fffffff) : ("
      "-0x7fffffff)) * (((( 1/(((-0x7fffffff))/(0x7fffffff)) == 1) || 0 || ( 1/(((-0x7fffffff))/(0x7fffffff)) == 1))&~1) "
      "== 0)))/(0x7fffffff)) == 1) && 1) ? (0x7fffffff) : (-0x7fffffff)) * (((( 1/(((((( 1/(((-0x7fffffff))/(0x7fffffff))"
      " == 1) || 0 || ( 1/(((-0x7fffffff))/(0x7fffffff)) == 1)) ? (0x7fffffff) : (-0x7fffffff)) * (((( 1/(((-0x7fffffff))"
      "/(0x7fffffff)) == 1) || 0 || ( 1/(((-0x7fffffff))/(0x7fffffff)) == 1))&~1) == 0)))/(0x7fffffff)) == 1) && 1)&~1) ="
      "= 0)))/(0x7fffffff)) == 1))&~1) == 0)))/(0x7fffffff)) == 1) ? 0.0f : 1.0f), 0 ) failed: %s\n",
      1446,
      v0);
  }
  result = 0;
  return result;
}

void __cdecl R_EndCubemapShot(CubemapShot shotIndex)
{
    R_CopyCubemapShot(shotIndex);
    R_CubemapShotRestoreState();
}

void __cdecl R_CopyCubemapShot(CubemapShot shotIndex)
{
    int imgIndex; // [esp+0h] [ebp-8h]
    int sizeInBytes; // [esp+4h] [ebp-4h]

    if ( (shotIndex <= CUBEMAPSHOT_NONE || shotIndex >= CUBEMAPSHOT_COUNT)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                    1399,
                    0,
                    "%s\n\t(shotIndex) = %i",
                    "(shotIndex > CUBEMAPSHOT_NONE && shotIndex < CUBEMAPSHOT_COUNT)",
                    shotIndex) )
    {
        __debugbreak();
    }
    imgIndex = shotIndex - 1;
    sizeInBytes = 16 * gfxMetrics.cubemapShotRes * gfxMetrics.cubemapShotRes;
    cubeShotGlob.pixelsHDR[shotIndex - 1] = (float *)Z_VirtualAlloc(sizeInBytes, "R_CopyCubemapShotHDR", 23);
    if ( !cubeShotGlob.pixelsHDR[shotIndex - 1]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                    1404,
                    1,
                    "%s",
                    "cubeShotGlob.pixelsHDR[imgIndex]") )
    {
        __debugbreak();
    }
    cubeShotGlob.pixelsHDRcorrected[imgIndex] = (float *)Z_VirtualAlloc(sizeInBytes, "R_CopyCubemapShotHDRcorrected", 23);
    if ( !cubeShotGlob.pixelsHDRcorrected[imgIndex]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                    1406,
                    1,
                    "%s",
                    "cubeShotGlob.pixelsHDRcorrected[imgIndex]") )
    {
        __debugbreak();
    }
    R_CubemapShotCopySurfaceToBufferHDR(cubeShotGlob.pixelsHDR[imgIndex], sizeInBytes);
}

void __cdecl R_CubemapShotCopySurfaceToBufferHDR(float *buffer, int bufferSizeInBytes)
{
    if ( bufferSizeInBytes <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                    1302,
                    0,
                    "%s\n\t(bufferSizeInBytes) = %i",
                    "(bufferSizeInBytes > 0)",
                    bufferSizeInBytes) )
    {
        __debugbreak();
    }
    R_GetBackBufferDataHDR(
        gfxMetrics.cubemapShotPixelBorder,
        gfxMetrics.cubemapShotPixelBorder,
        gfxMetrics.cubemapShotRes,
        gfxMetrics.cubemapShotRes,
        4,
        buffer);
}

char __cdecl R_GetBackBufferDataHDR(int x, int y, int width, int height, int bytesPerPixel, float *buffer)
{
    const char *v6; // eax
    unsigned __int16 r; // [esp+4h] [ebp-60h]
    float rr; // [esp+8h] [ebp-5Ch]
    float gg; // [esp+Ch] [ebp-58h]
    unsigned __int16 g; // [esp+10h] [ebp-54h]
    unsigned __int16 b; // [esp+14h] [ebp-50h]
    float bb; // [esp+18h] [ebp-4Ch]
    int i; // [esp+28h] [ebp-3Ch]
    IDirect3DSurface9 *surface1; // [esp+2Ch] [ebp-38h]
    int hr; // [esp+30h] [ebp-34h]
    _D3DLOCKED_RECT lockedRect; // [esp+34h] [ebp-30h] BYREF
    float *dstPixel; // [esp+3Ch] [ebp-28h]
    const unsigned __int8 *srcPixel; // [esp+40h] [ebp-24h]
    tagRECT sourceRect; // [esp+44h] [ebp-20h] BYREF
    int row; // [esp+54h] [ebp-10h]
    IDirect3DSurface9 *surf; // [esp+58h] [ebp-Ch]
    unsigned __int16 *bitss; // [esp+5Ch] [ebp-8h]
    int col; // [esp+60h] [ebp-4h]

    initHalfToFloat();
    Sleep(0x3E8u);
    surf = gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].surface.color;
    surface1 = gfxRenderTargets[R_RENDERTARGET_16BIT_SYSTEM].surface.color;
    dx.device->GetRenderTargetData(gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].surface.color, gfxRenderTargets[R_RENDERTARGET_16BIT_SYSTEM].surface.color);
    sourceRect.left = x;
    sourceRect.right = width + x;
    sourceRect.top = y + 2;
    sourceRect.bottom = height + y + 2;
    //hr = ((int(__thiscall *)(IDirect3DSurface9 *, IDirect3DSurface9 *, _D3DLOCKED_RECT *, tagRECT *, int))surface1->LockRect)(
    //    surface1,
    //    surface1,
    //    &lockedRect,
    //    &sourceRect,
    //    16);
    hr = surface1->LockRect(&lockedRect, &sourceRect, 16);
    if (hr >= 0)
    {
        srcPixel = (const unsigned __int8 *)lockedRect.pBits;
        dstPixel = buffer;
        bitss = (unsigned __int16 *)lockedRect.pBits;
        for (row = 0; row < height; ++row)
        {
            bitss = (unsigned __int16 *)srcPixel;
            i = 0;
            for (col = 0; col < width; ++col)
            {
                r = bitss[i];
                g = bitss[i + 1];
                b = bitss[i + 2];
                i += 4;
                rr = COERCE_FLOAT(half2float(r));
                gg = COERCE_FLOAT(half2float(g));
                bb = COERCE_FLOAT(half2float(b));
                *dstPixel = (float)(rr * rr) * 16.0;
                dstPixel[1] = (float)(gg * gg) * 16.0;
                dstPixel[2] = (float)(bb * bb) * 16.0;
                dstPixel[3] = 1.0f;
                dstPixel += 4;
            }
            srcPixel += lockedRect.Pitch;
        }
        surface1->UnlockRect();
        return 1;
    }
    else
    {
        v6 = R_ErrorDescription(hr);
        Com_PrintError(8, "ERROR: cannot take screenshot: LockRect failed: %s (0x%08x)\n", v6, hr);
        return 0;
    }
}

unsigned int __cdecl half2float(unsigned __int16 h)
{
    return exponenttable[(int)h >> 10] + mantissatable[(h & 0x3FF) + offsettable[(int)h >> 10]];
}

void initHalfToFloat()
{
    signed int i; // [esp+0h] [ebp-4h]
    int ia; // [esp+0h] [ebp-4h]
    int ib; // [esp+0h] [ebp-4h]
    int ic; // [esp+0h] [ebp-4h]
    int id; // [esp+0h] [ebp-4h]

    mantissatable[0] = 0;
    for ( i = 1; i <= 1023; ++i )
        mantissatable[i] = convertmantissa(i);
    for ( ia = 1024; ia <= 2047; ++ia )
        mantissatable[ia] = ((ia - 1024) << 13) + 939524096;
    exponenttable[0] = 0;
    exponenttable[32] = 0x80000000;
    for ( ib = 1; ib <= 30; ++ib )
        exponenttable[ib] = ib << 23;
    for ( ic = 33; ic <= 62; ++ic )
        exponenttable[ic] = ((ic - 32) << 23) + 0x80000000;
    exponenttable[31] = 1199570944;
    exponenttable[63] = -947912704;
    for ( id = 0; id <= 63; ++id )
        offsettable[id] = 1024;
    offsettable[0] = 0;
    offsettable[32] = 0;
}

unsigned int __cdecl convertmantissa(unsigned int i)
{
    unsigned int m; // [esp+0h] [ebp-8h]
    unsigned int e; // [esp+4h] [ebp-4h]

    m = i << 13;
    e = 0;
    while ((m & 0x800000) == 0)
    {
        e -= 0x800000;
        m *= 2;
    }
    return (e + 0x38800000) | m & 0xFF7FFFFF;
}

void R_CubemapShotRestoreState()
{
  R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
  R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
}

void __cdecl R_SaveCubemapShot(char *filename, CubemapShot shotIndex, float n0, float n1)
{
    R_CubemapShotWriteTargaFile(filename, shotIndex, n0, n1);
}

void __cdecl R_CubemapShotWriteTargaFile(char *filename, CubemapShot shotIndex, float n0, float n1)
{
    int imgIndex; // [esp+Ch] [ebp-Ch]
    int fileSize; // [esp+10h] [ebp-8h]
    unsigned __int8 *targa; // [esp+14h] [ebp-4h]

    if ( (shotIndex <= CUBEMAPSHOT_NONE || shotIndex >= CUBEMAPSHOT_COUNT)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                    1423,
                    0,
                    "%s\n\t(shotIndex) = %i",
                    "(shotIndex > CUBEMAPSHOT_NONE && shotIndex < CUBEMAPSHOT_COUNT)",
                    shotIndex) )
    {
        __debugbreak();
    }
    imgIndex = shotIndex - 1;
    fileSize = 4 * gfxMetrics.cubemapShotRes * gfxMetrics.cubemapShotRes + 18;
    targa = (unsigned __int8 *)Z_VirtualAlloc(fileSize, "R_CubemapShotWriteTargaFile", 23);
    if ( !targa
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp", 1428, 1, "%s", "targa") )
    {
        __debugbreak();
    }
    R_CubemapShotWriteTargaHeader(gfxMetrics.cubemapShotRes, targa);
    R_CubemapShotCopyBufferToTarga(cubeShotGlob.pixels[imgIndex], targa);
    R_CubemapShotApplyFresnelToTarga(shotIndex, n0, n1, targa);
    FS_WriteFile(filename, (char *)targa, fileSize);
    Z_VirtualFree(targa, 23);
    Z_VirtualFree(cubeShotGlob.pixels[imgIndex], 23);
}

void __cdecl R_CubemapShotWriteTargaHeader(int res, unsigned __int8 *fileBuffer)
{
    if ( res <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                    1334,
                    0,
                    "%s\n\t(res) = %i",
                    "(res > 0)",
                    res) )
    {
        __debugbreak();
    }
    *(unsigned int *)fileBuffer = 0;
    *((unsigned int *)fileBuffer + 1) = 0;
    *((unsigned int *)fileBuffer + 2) = 0;
    *((unsigned int *)fileBuffer + 3) = 0;
    *((_WORD *)fileBuffer + 8) = 0;
    fileBuffer[2] = 2;
    *((_WORD *)fileBuffer + 6) = res;
    fileBuffer[14] = fileBuffer[12];
    fileBuffer[15] = fileBuffer[13];
    fileBuffer[16] = 32;
}

void __cdecl R_CubemapShotCopyBufferToTarga(const unsigned __int8 *srcBuffer, unsigned __int8 *targa)
{
    int srcIndex; // [esp+0h] [ebp-10h]
    int colIndex; // [esp+4h] [ebp-Ch]
    int rowIndex; // [esp+8h] [ebp-8h]
    int dstIndex; // [esp+Ch] [ebp-4h]

    for ( rowIndex = 0; rowIndex < gfxMetrics.cubemapShotRes; ++rowIndex )
    {
        for ( colIndex = 0; colIndex < gfxMetrics.cubemapShotRes; ++colIndex )
        {
            srcIndex = 4 * (colIndex + rowIndex * gfxMetrics.cubemapShotRes);
            dstIndex = srcIndex + 18;
            targa[dstIndex + 2] = srcBuffer[srcIndex + 2];
            targa[dstIndex + 1] = srcBuffer[srcIndex + 1];
            targa[dstIndex] = srcBuffer[srcIndex];
            targa[dstIndex + 3] = srcBuffer[srcIndex + 3];
        }
    }
}

void __cdecl R_CubemapShotApplyFresnelToTarga(CubemapShot shotIndex, float n0, float n1, unsigned __int8 *targa)
{
    int colIndex; // [esp+8h] [ebp-10h]
    int rowIndex; // [esp+Ch] [ebp-Ch]
    int dstIndex; // [esp+10h] [ebp-8h]

    if ( (shotIndex <= CUBEMAPSHOT_NONE || shotIndex >= CUBEMAPSHOT_COUNT)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                    1381,
                    0,
                    "%s\n\t(shotIndex) = %i",
                    "(shotIndex > CUBEMAPSHOT_NONE && shotIndex < CUBEMAPSHOT_COUNT)",
                    shotIndex) )
    {
        __debugbreak();
    }
    for ( rowIndex = 0; rowIndex < gfxMetrics.cubemapShotRes; ++rowIndex )
    {
        for ( colIndex = 0; colIndex < gfxMetrics.cubemapShotRes; ++colIndex )
        {
            dstIndex = 4 * (colIndex + rowIndex * gfxMetrics.cubemapShotRes) + 18;
            targa[dstIndex + 3] = R_CubemapShotCalcReflectionFactor(shotIndex, colIndex, rowIndex, n0, n1);
        }
    }
}

unsigned __int8 __cdecl R_CubemapShotCalcReflectionFactor(
                int shotIndex,
                int colIndex,
                int rowIndex,
                float n0,
                float n1)
{
    float dir[3]; // [esp+44h] [ebp-10h] BYREF
    float refraction; // [esp+50h] [ebp-4h]

    if ( shotIndex <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                    1107,
                    0,
                    "%s\n\t(shotIndex) = %i",
                    "(shotIndex > CUBEMAPSHOT_NONE)",
                    shotIndex) )
    {
        __debugbreak();
    }
    if ( shotIndex >= 7
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                    1108,
                    0,
                    "%s\n\t(shotIndex) = %i",
                    "(shotIndex < CUBEMAPSHOT_COUNT)",
                    shotIndex) )
    {
        __debugbreak();
    }
    if ( colIndex < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                    1109,
                    0,
                    "%s\n\t(colIndex) = %i",
                    "(colIndex >= 0)",
                    colIndex) )
    {
        __debugbreak();
    }
    if ( colIndex >= gfxMetrics.cubemapShotRes
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                    1110,
                    0,
                    "%s\n\t(colIndex) = %i",
                    "(colIndex < gfxMetrics.cubemapShotRes)",
                    colIndex) )
    {
        __debugbreak();
    }
    if ( rowIndex < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                    1111,
                    0,
                    "%s\n\t(rowIndex) = %i",
                    "(rowIndex >= 0)",
                    rowIndex) )
    {
        __debugbreak();
    }
    if ( rowIndex >= gfxMetrics.cubemapShotRes
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                    1112,
                    0,
                    "%s\n\t(rowIndex) = %i",
                    "(rowIndex < gfxMetrics.cubemapShotRes)",
                    rowIndex) )
    {
        __debugbreak();
    }
    if ( n0 == 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                    1113,
                    0,
                    "%s\n\t(n0) = %g",
                    "(n0 != 0)",
                    n0) )
    {
        __debugbreak();
    }
    if ( n1 == 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_screenshot.cpp",
                    1114,
                    0,
                    "%s\n\t(n1) = %g",
                    "(n1 != 0)",
                    n1) )
    {
        __debugbreak();
    }
    dir[0] = (float)((float)gfxMetrics.cubemapShotRes * 0.5) * (float)cubemapShotAxis[shotIndex][0][0];
    dir[1] = (float)((float)gfxMetrics.cubemapShotRes * 0.5) * (float)cubemapShotAxis[shotIndex][0][1];
    dir[2] = (float)((float)gfxMetrics.cubemapShotRes * 0.5) * (float)cubemapShotAxis[shotIndex][0][2];
    dir[0] = (float)((float)((float)((float)colIndex - (float)((float)gfxMetrics.cubemapShotRes * 0.5)) + 0.5)
                                 * (float)cubemapShotAxis[shotIndex][1][0])
                 + dir[0];
    dir[1] = (float)((float)((float)((float)colIndex - (float)((float)gfxMetrics.cubemapShotRes * 0.5)) + 0.5)
                                 * (float)cubemapShotAxis[shotIndex][1][1])
                 + dir[1];
    dir[2] = (float)((float)((float)((float)colIndex - (float)((float)gfxMetrics.cubemapShotRes * 0.5)) + 0.5)
                                 * (float)cubemapShotAxis[shotIndex][1][2])
                 + dir[2];
    dir[0] = (float)((float)((float)((float)rowIndex - (float)((float)gfxMetrics.cubemapShotRes * 0.5)) + 0.5)
                                 * (float)cubemapShotAxis[shotIndex][2][0])
                 + dir[0];
    dir[1] = (float)((float)((float)((float)rowIndex - (float)((float)gfxMetrics.cubemapShotRes * 0.5)) + 0.5)
                                 * (float)cubemapShotAxis[shotIndex][2][1])
                 + dir[1];
    dir[2] = (float)((float)((float)((float)rowIndex - (float)((float)gfxMetrics.cubemapShotRes * 0.5)) + 0.5)
                                 * (float)cubemapShotAxis[shotIndex][2][2])
                 + dir[2];
    Vec3Normalize(dir);
    refraction = FresnelTerm(n0, n1, dir[2]);
    return (int)((float)(refraction * 255.0) + 9.313225746154785e-10);
}

void __cdecl R_CreateReflectionRawDataFromCubemapShot(DiskGfxReflectionProbe *probeRawData, int downSampleRes)
{
    int v2; // [esp+1Ch] [ebp-27A8h]
    int m_Width; // [esp+20h] [ebp-27A4h]
    float v4; // [esp+24h] [ebp-27A0h]
    float v5; // [esp+28h] [ebp-279Ch]
    float v6; // [esp+2Ch] [ebp-2798h]
    int jj; // [esp+30h] [ebp-2794h]
    int ii; // [esp+38h] [ebp-278Ch]
    int n; // [esp+3Ch] [ebp-2788h]
    int m; // [esp+44h] [ebp-2780h]
    int k; // [esp+48h] [ebp-277Ch]
    int j; // [esp+4Ch] [ebp-2778h]
    int i; // [esp+50h] [ebp-2774h]
    unsigned __int8 *pixels; // [esp+64h] [ebp-2760h]
    int size; // [esp+70h] [ebp-2754h]
    float outColor[3]; // [esp+80h] [ebp-2744h] BYREF
    //float v17; // [esp+84h] [ebp-2740h]
    //float v18; // [esp+88h] [ebp-273Ch]
    CCubeMapProcessor v19; // [esp+8Ch] [ebp-2738h] BYREF
    int v20; // [esp+27C0h] [ebp-4h]

    size = gfxMetrics.cubemapShotRes;
    pixels = probeRawData->pixels;
    R_CubemapShotRotateAntiClockwise(
        cubeShotGlob.pixelsHDR[0],
        cubeShotGlob.pixelsHDRcorrected[0],
        gfxMetrics.cubemapShotRes);
    R_CubemapShotFlipVertical(cubeShotGlob.pixelsHDR[0], size);
    R_CubemapShotRotateClockwise(cubeShotGlob.pixelsHDR[1], cubeShotGlob.pixelsHDRcorrected[1], size);
    R_CubemapShotFlipVertical(cubeShotGlob.pixelsHDR[1], size);
    R_CubemapShotFlipVertical(cubeShotGlob.pixelsHDR[2], size);
    R_CubemapShotFlipHorizontal(cubeShotGlob.pixelsHDR[3], size);
    R_CubemapShotRotateAntiClockwise(cubeShotGlob.pixelsHDR[4], cubeShotGlob.pixelsHDRcorrected[4], size);
    R_CubemapShotFlipVertical(cubeShotGlob.pixelsHDR[4], size);
    R_CubemapShotRotateAntiClockwise(cubeShotGlob.pixelsHDR[5], cubeShotGlob.pixelsHDRcorrected[5], size);
    R_CubemapShotFlipVertical(cubeShotGlob.pixelsHDR[5], size);
    R_GetAverageSecondaryLightingAtPoint(probeRawData->origin, outColor);
    if ( outColor[0] <= 0.0 )
        v6 = 0.0f;
    else
        v6 = 1.0 / outColor[0];
    outColor[0] = v6;
    if (outColor[1] <= 0.0 )
        v5 = 0.0f;
    else
        v5 = 1.0 / outColor[1];
    outColor[1] = v5;
    if (outColor[2] <= 0.0 )
        v4 = 0.0f;
    else
        v4 = 1.0 / outColor[2];
    outColor[2] = v4;
    for ( i = 0; i < 6; ++i )
        R_CubemapFaceNormalize(cubeShotGlob.pixelsHDR[i], size, outColor);
    //CCubeMapProcessor::CCubeMapProcessor(&v19);
    //CCubeMapProcessor::Init(&v19, size, downSampleRes, 16, 4);
    v19.Init(size, downSampleRes, 16, 4);
    for (j = 0; j < 6; ++j)
    {
        //CCubeMapProcessor::SetInputFaceData(
        //    &v19,
        //    j,
        //    30,
        //    4,
        //    16 * size,
        //    (unsigned __int16 *)cubeShotGlob.pixelsHDR[j],
        //    63.900002,
        //    1.0,
        //    1.0);
        v19.SetInputFaceData(j, 30, 4, 16 * size, (unsigned short *)cubeShotGlob.pixelsHDR[j], 63.9f, 1.0f, 1.0f);
    }
        
    v20 = 3;
    //CCubeMapProcessor::InitiateFiltering(
    //    &v19,
    //    90.112 / (float)downSampleRes,
    //    (float)(90.112 / (float)downSampleRes) * 2.0,
    //    2.0,
    //    3u,
    //    2,
    //    3,
    //    1);
        v19.InitiateFiltering(
        90.112 / (float)downSampleRes,
        (float)(90.112 / (float)downSampleRes) * 2.0,
        2.0,
        3u,
        2,
        3,
        1);

    while ( v19.GetStatus() == 1)
        Sleep(200);

    for ( k = 0; k < 6; ++k )
    {
        for ( m = 0; m < 1; ++m )
        {
            if ( v19.m_OutputSurface[m][k].m_Width <= 4 )
                m_Width = 4;
            else
                m_Width = v19.m_OutputSurface[m][k].m_Width;
            v19.GetOutputFaceData(k, m, 100, 4, m_Width, (unsigned int *)pixels, 0.125, 2.0);
            pixels += m_Width * m_Width / 2;
        }
    }
    for ( n = 0; n < 6; ++n )
    {
        for ( ii = 1; ii < v19.m_NumMipLevels; ++ii )
        {
            if ( v19.m_OutputSurface[ii][n].m_Width <= 4 )
                v2 = 4;
            else
                v2 = v19.m_OutputSurface[ii][n].m_Width;
            v19.GetOutputFaceData(n, ii, 100, 4, v2, (unsigned int *)pixels, 0.125, 2.0);
            pixels += v2 * v2 / 2;
        }
    }
    for ( jj = 0; jj < 6; ++jj )
    {
        Z_VirtualFree(cubeShotGlob.pixelsHDR[jj], 23);
        Z_VirtualFree(cubeShotGlob.pixelsHDRcorrected[jj], 23);
    }
    //CCubeMapProcessor::~CCubeMapProcessor(&v19);
}

void __cdecl R_CubemapFaceNormalize(float *inbuffer, int size, float *average)
{
    int i; // [esp+0h] [ebp-14h]
    float g; // [esp+Ch] [ebp-8h]
    float b; // [esp+10h] [ebp-4h]

    for ( i = 0; i < size * size; ++i )
    {
        g = inbuffer[1];
        b = inbuffer[2];
        *inbuffer = *inbuffer * *average;
        inbuffer[1] = g * average[1];
        inbuffer[2] = b * average[2];
        inbuffer += 4;
    }
}

void __cdecl R_CubemapShotRotateClockwise(float *inbuffer, float *tmpbuffer, int size)
{
    int i; // [esp+0h] [ebp-14h]
    int x; // [esp+4h] [ebp-10h]
    int y; // [esp+8h] [ebp-Ch]
    float *dstpixela; // [esp+Ch] [ebp-8h]
    float *dstpixel; // [esp+Ch] [ebp-8h]
    float *srcpixela; // [esp+10h] [ebp-4h]
    float *srcpixel; // [esp+10h] [ebp-4h]

    for ( y = 0; y < size; ++y )
    {
        for ( x = 0; x < size; ++x )
        {
            srcpixela = &inbuffer[4 * x + 4 * size * y];
            dstpixela = &tmpbuffer[4 * (size - 1 - y) + 4 * size * x];
            *dstpixela = *srcpixela;
            dstpixela[1] = srcpixela[1];
            dstpixela[2] = srcpixela[2];
            dstpixela[3] = srcpixela[3];
        }
    }
    srcpixel = tmpbuffer;
    dstpixel = inbuffer;
    for ( i = 0; i < size * size; ++i )
    {
        *dstpixel = *srcpixel;
        dstpixel[1] = srcpixel[1];
        dstpixel[2] = srcpixel[2];
        dstpixel[3] = srcpixel[3];
        srcpixel += 4;
        dstpixel += 4;
    }
}

void __cdecl R_CubemapShotRotateAntiClockwise(float *inbuffer, float *tmpbuffer, int size)
{
    int i; // [esp+0h] [ebp-14h]
    int x; // [esp+4h] [ebp-10h]
    int y; // [esp+8h] [ebp-Ch]
    float *dstpixela; // [esp+Ch] [ebp-8h]
    float *dstpixel; // [esp+Ch] [ebp-8h]
    float *srcpixela; // [esp+10h] [ebp-4h]
    float *srcpixel; // [esp+10h] [ebp-4h]

    for ( y = 0; y < size; ++y )
    {
        for ( x = 0; x < size; ++x )
        {
            srcpixela = &inbuffer[4 * x + 4 * size * y];
            dstpixela = &tmpbuffer[4 * y + 4 * size * (size - 1 - x)];
            *dstpixela = *srcpixela;
            dstpixela[1] = srcpixela[1];
            dstpixela[2] = srcpixela[2];
            dstpixela[3] = srcpixela[3];
        }
    }
    srcpixel = tmpbuffer;
    dstpixel = inbuffer;
    for ( i = 0; i < size * size; ++i )
    {
        *dstpixel = *srcpixel;
        dstpixel[1] = srcpixel[1];
        dstpixel[2] = srcpixel[2];
        dstpixel[3] = srcpixel[3];
        srcpixel += 4;
        dstpixel += 4;
    }
}

void __cdecl R_CubemapShotFlipVertical(float *buffer, int size)
{
    int x; // [esp+0h] [ebp-20h]
    int y; // [esp+4h] [ebp-1Ch]
    float tmp0; // [esp+8h] [ebp-18h]
    int tmp1; // [esp+Ch] [ebp-14h]
    int tmp2; // [esp+10h] [ebp-10h]
    float *dstpixel; // [esp+14h] [ebp-Ch]
    int tmp3; // [esp+18h] [ebp-8h]
    float *srcpixel; // [esp+1Ch] [ebp-4h]

    for ( y = 0; y < size / 2; ++y )
    {
        for ( x = 0; x < size; ++x )
        {
            srcpixel = &buffer[4 * x + 4 * size * y];
            dstpixel = &buffer[4 * x + 4 * size * (size - 1 - y)];
            tmp0 = *srcpixel;
            tmp1 = *((unsigned int *)srcpixel + 1);
            tmp2 = *((unsigned int *)srcpixel + 2);
            tmp3 = *((unsigned int *)srcpixel + 3);
            *srcpixel = *dstpixel;
            srcpixel[1] = dstpixel[1];
            srcpixel[2] = dstpixel[2];
            srcpixel[3] = dstpixel[3];
            *dstpixel = tmp0;
            *((unsigned int *)dstpixel + 1) = tmp1;
            *((unsigned int *)dstpixel + 2) = tmp2;
            *((unsigned int *)dstpixel + 3) = tmp3;
        }
    }
}

void __cdecl R_CubemapShotFlipHorizontal(float *buffer, int size)
{
    int x; // [esp+0h] [ebp-20h]
    int y; // [esp+4h] [ebp-1Ch]
    float tmp0; // [esp+8h] [ebp-18h]
    int tmp1; // [esp+Ch] [ebp-14h]
    int tmp2; // [esp+10h] [ebp-10h]
    float *dstpixel; // [esp+14h] [ebp-Ch]
    int tmp3; // [esp+18h] [ebp-8h]
    float *srcpixel; // [esp+1Ch] [ebp-4h]

    for ( y = 0; y < size; ++y )
    {
        for ( x = 0; x < size / 2; ++x )
        {
            srcpixel = &buffer[4 * x + 4 * size * y];
            dstpixel = &buffer[4 * size * y + 4 * (size - 1 - x)];
            tmp0 = *srcpixel;
            tmp1 = *((unsigned int *)srcpixel + 1);
            tmp2 = *((unsigned int *)srcpixel + 2);
            tmp3 = *((unsigned int *)srcpixel + 3);
            *srcpixel = *dstpixel;
            srcpixel[1] = dstpixel[1];
            srcpixel[2] = dstpixel[2];
            srcpixel[3] = dstpixel[3];
            *dstpixel = tmp0;
            *((unsigned int *)dstpixel + 1) = tmp1;
            *((unsigned int *)dstpixel + 2) = tmp2;
            *((unsigned int *)dstpixel + 3) = tmp3;
        }
    }
}

void __cdecl R_LightingFromCubemapShots(const float *baseColor)
{
    int pixelsPerFace; // [esp+0h] [ebp-1Ch]
    float (*linearColors[6])[3]; // [esp+4h] [ebp-18h] BYREF

    pixelsPerFace = gfxMetrics.cubemapShotRes * gfxMetrics.cubemapShotRes;
    linearColors[0] = (float (*)[3])Z_VirtualAlloc(72 * pixelsPerFace, "R_LightingFromCubemapShot", 23);
    linearColors[1] = (float (*)[3])linearColors[0][pixelsPerFace];
    linearColors[2] = (float (*)[3])linearColors[1][pixelsPerFace];
    linearColors[3] = (float (*)[3])linearColors[2][pixelsPerFace];
    linearColors[4] = (float (*)[3])linearColors[3][pixelsPerFace];
    linearColors[5] = (float (*)[3])linearColors[4][pixelsPerFace];
    R_CubemapShotExtractLinearLight(
        cubeShotGlob.pixels,
        gfxMetrics.cubemapShotRes,
        gfxMetrics.cubemapShotRes,
        linearColors);
    R_CubemapLighting(linearColors, gfxMetrics.cubemapShotRes, gfxMetrics.cubemapShotRes, baseColor, cubeShotGlob.pixels);
    Z_VirtualFree(linearColors[0], 23);
}

void __cdecl R_CubemapLighting(
                float (**linearColors)[3],
                int width,
                int height,
                const float *baseColor,
                unsigned __int8 **pixels)
{
    float dir[3]; // [esp+Ch] [ebp-18h] BYREF
    int faceIndex; // [esp+18h] [ebp-Ch]
    int x; // [esp+1Ch] [ebp-8h]
    int y; // [esp+20h] [ebp-4h]

    for ( faceIndex = 0; faceIndex < 6; ++faceIndex )
    {
        for ( y = 0; y < height; ++y )
        {
            for ( x = 0; x < width; ++x )
            {
                R_GetDirForCubemapPixel(
                    faceIndex,
                    (float)((float)((float)((float)x + 0.5) * 2.0) / (float)width) - 1.0,
                    (float)((float)((float)((float)y + 0.5) * 2.0) / (float)height) - 1.0,
                    dir);
                R_CubemapLightingForDir(linearColors, width, height, dir, baseColor, &pixels[faceIndex][4 * x + 4 * y * width]);
            }
        }
    }
}

void __cdecl R_GetDirForCubemapPixel(int faceIndex, float x, float y, float *dir)
{
    const float *v4; // [esp+14h] [ebp-8h]

    v4 = cubemapShotAxis[faceIndex + 1][0];
    *dir = (float)(x * (float)cubemapShotAxis[faceIndex + 1][1][0]) + *v4;
    dir[1] = (float)(x * (float)cubemapShotAxis[faceIndex + 1][1][1]) + v4[1];
    dir[2] = (float)(x * (float)cubemapShotAxis[faceIndex + 1][1][2]) + v4[2];
    *dir = (float)(y * (float)cubemapShotAxis[faceIndex + 1][2][0]) + *dir;
    dir[1] = (float)(y * (float)cubemapShotAxis[faceIndex + 1][2][1]) + dir[1];
    dir[2] = (float)(y * (float)cubemapShotAxis[faceIndex + 1][2][2]) + dir[2];
    Vec3Normalize(dir);
}

//void __cdecl R_CubemapLightingForDir(
//                float (**linearColors)[3],
//                int width,
//                int height,
//                const float *dir,
//                const float *baseColor,
//                unsigned __int8 *pixel)
//{
//    long double v6; // [esp+Ch] [ebp-88h]
//    long double v7; // [esp+Ch] [ebp-88h]
//    long double v8; // [esp+Ch] [ebp-88h]
//    float v9; // [esp+Ch] [ebp-88h]
//    float v10; // [esp+10h] [ebp-84h]
//    long double v11; // [esp+14h] [ebp-80h]
//    long double v12; // [esp+14h] [ebp-80h]
//    long double v13; // [esp+14h] [ebp-80h]
//    float v14; // [esp+14h] [ebp-80h]
//    float *v15; // [esp+64h] [ebp-30h]
//    int faceIndex; // [esp+68h] [ebp-2Ch]
//    int sampleCount; // [esp+6Ch] [ebp-28h]
//    int x; // [esp+70h] [ebp-24h]
//    int y; // [esp+74h] [ebp-20h]
//    float color; // [esp+78h] [ebp-1Ch]
//    float colora; // [esp+78h] [ebp-1Ch]
//    float color_4; // [esp+7Ch] [ebp-18h]
//    float color_4a; // [esp+7Ch] [ebp-18h]
//    float color_8; // [esp+80h] [ebp-14h]
//    float color_8a; // [esp+80h] [ebp-14h]
//    float sourceDir[3]; // [esp+84h] [ebp-10h] BYREF
//    float facing; // [esp+90h] [ebp-4h]
//
//    color = 0.0f;
//    color_4 = 0.0f;
//    color_8 = 0.0f;
//    sampleCount = 0;
//    for ( faceIndex = 0; faceIndex < 6; ++faceIndex )
//    {
//        for ( y = 0; y < height; ++y )
//        {
//            for ( x = 0; x < width; ++x )
//            {
//                R_GetDirForCubemapPixel(
//                    faceIndex,
//                    (float)((float)((float)((float)x + 0.5) * 2.0) / (float)width) - 1.0,
//                    (float)((float)((float)((float)y + 0.5) * 2.0) / (float)height) - 1.0,
//                    sourceDir);
//                facing = (float)((float)(*dir * sourceDir[0]) + (float)(dir[1] * sourceDir[1])) + (float)(dir[2] * sourceDir[2]);
//                if ( facing > 0.0 )
//                {
//                    v15 = &linearColors[faceIndex][x][3 * width * y];
//                    color = (float)(facing * *v15) + color;
//                    color_4 = (float)(facing * v15[1]) + color_4;
//                    color_8 = (float)(facing * v15[2]) + color_8;
//                    ++sampleCount;
//                }
//            }
//        }
//    }
//    __libm_sse2_pow(v6, v11);
//    __libm_sse2_pow(v7, v12);
//    __libm_sse2_pow(v8, v13);
//    colora = (float)((float)(1.0 / (float)sampleCount) * color) * *baseColor;
//    color_4a = (float)((float)(1.0 / (float)sampleCount) * color_4) * baseColor[1];
//    color_8a = (float)((float)(1.0 / (float)sampleCount) * color_8) * baseColor[2];
//    if ( (float)(colora - 1.0) < 0.0 )
//        v14 = colora;
//    else
//        v14 = 1.0f;
//    pixel[2] = (int)((float)(255.0 * v14) + 9.313225746154785e-10);
//    if ( (float)(color_4a - 1.0) < 0.0 )
//        v10 = color_4a;
//    else
//        v10 = 1.0f;
//    pixel[1] = (int)((float)(255.0 * v10) + 9.313225746154785e-10);
//    if ( (float)(color_8a - 1.0) < 0.0 )
//        v9 = color_8a;
//    else
//        v9 = 1.0f;
//    *pixel = (int)((float)(255.0 * v9) + 9.313225746154785e-10);
//    pixel[3] = -1;
//}

// aislop
void __cdecl R_CubemapLightingForDir(
    float (**linearColors)[3],
    int width,
    int height,
    const float *dir,
    const float *baseColor,
    unsigned __int8 *pixel)
{
    int faceIndex, x, y;
    int sampleCount = 0;

    float color = 0.0f;
    float color_4 = 0.0f;
    float color_8 = 0.0f;

    float sourceDir[3];
    float facing;

    for (faceIndex = 0; faceIndex < 6; ++faceIndex)
    {
        for (y = 0; y < height; ++y)
        {
            for (x = 0; x < width; ++x)
            {
                R_GetDirForCubemapPixel(
                    faceIndex,
                    ((x + 0.5f) * 2.0f / width) - 1.0f,
                    ((y + 0.5f) * 2.0f / height) - 1.0f,
                    sourceDir);

                facing =
                    dir[0] * sourceDir[0] +
                    dir[1] * sourceDir[1] +
                    dir[2] * sourceDir[2];

                if (facing > 0.0f)
                {
                    float *c = &linearColors[faceIndex][x][3 * width * y];

                    color += facing * c[0];
                    color_4 += facing * c[1];
                    color_8 += facing * c[2];

                    ++sampleCount;
                }
            }
        }
    }

    if (sampleCount > 0)
    {
        float invSamples = 1.0f / (float)sampleCount;

        float r = color * invSamples * baseColor[0];
        float g = color_4 * invSamples * baseColor[1];
        float b = color_8 * invSamples * baseColor[2];

        r = powf(r, 1.0f / 2.2f);
        g = powf(g, 1.0f / 2.2f);
        b = powf(b, 1.0f / 2.2f);

        if (r > 1.0f) r = 1.0f;
        if (g > 1.0f) g = 1.0f;
        if (b > 1.0f) b = 1.0f;

        pixel[2] = (unsigned __int8)(255.0f * r + 0.5f);
        pixel[1] = (unsigned __int8)(255.0f * g + 0.5f);
        pixel[0] = (unsigned __int8)(255.0f * b + 0.5f);
        pixel[3] = 255;
    }
    else
    {
        pixel[0] = pixel[1] = pixel[2] = 0;
        pixel[3] = 255;
    }
}


//void __cdecl R_CubemapShotExtractLinearLight(
//                unsigned __int8 **pixels,
//                int width,
//                int height,
//                float (**linearColors)[3])
//{
//    double v4; // xmm0_8
//    double v5; // xmm0_8
//    double v6; // xmm0_8
//    long double v7; // [esp+0h] [ebp-28h]
//    long double v8; // [esp+0h] [ebp-28h]
//    long double v9; // [esp+0h] [ebp-28h]
//    long double v10; // [esp+8h] [ebp-20h]
//    long double v11; // [esp+8h] [ebp-20h]
//    long double v12; // [esp+8h] [ebp-20h]
//    int faceIndex; // [esp+18h] [ebp-10h]
//    int x; // [esp+1Ch] [ebp-Ch]
//    int y; // [esp+20h] [ebp-8h]
//    int pixelIndex; // [esp+24h] [ebp-4h]
//
//    for ( faceIndex = 0; faceIndex < 6; ++faceIndex )
//    {
//        pixelIndex = 0;
//        for ( y = 0; y < height; ++y )
//        {
//            for ( x = 0; x < width; ++x )
//            {
//                v4 = (float)((float)pixels[faceIndex][4 * pixelIndex + 2] * 0.0039215689);
//                __libm_sse2_pow(v7, v10);
//                *(float *)&v4 = v4;
//                linearColors[faceIndex][pixelIndex][0] = *(float *)&v4;
//                *((float *)&v11 + 1) = (float)pixels[faceIndex][4 * pixelIndex + 1] * 0.0039215689;
//                v5 = *((float *)&v11 + 1);
//                __libm_sse2_pow(v8, v11);
//                *(float *)&v5 = v5;
//                LODWORD(v12) = LODWORD(v5);
//                linearColors[faceIndex][pixelIndex][1] = *(float *)&v5;
//                *((float *)&v9 + 1) = (float)pixels[faceIndex][4 * pixelIndex] * 0.0039215689;
//                v6 = *((float *)&v9 + 1);
//                __libm_sse2_pow(v9, v12);
//                *(float *)&v6 = v6;
//                LODWORD(v7) = LODWORD(v6);
//                linearColors[faceIndex][pixelIndex++][2] = *(float *)&v6;
//            }
//        }
//    }
//}


void __cdecl R_CubemapShotExtractLinearLight(
    unsigned __int8 **pixels,
    int width,
    int height,
    float (**linearColors)[3])
{
    int faceIndex;
    int x, y;
    int pixelIndex;

    for (faceIndex = 0; faceIndex < 6; ++faceIndex)
    {
        pixelIndex = 0;

        for (y = 0; y < height; ++y)
        {
            for (x = 0; x < width; ++x)
            {
                float r = pixels[faceIndex][4 * pixelIndex + 2] * (1.0f / 255.0f);
                float g = pixels[faceIndex][4 * pixelIndex + 1] * (1.0f / 255.0f);
                float b = pixels[faceIndex][4 * pixelIndex + 0] * (1.0f / 255.0f);

                r = powf(r, 2.2f);
                g = powf(g, 2.2f);
                b = powf(b, 2.2f);

                linearColors[faceIndex][pixelIndex][0] = r;
                linearColors[faceIndex][pixelIndex][1] = g;
                linearColors[faceIndex][pixelIndex][2] = b;

                ++pixelIndex;
            }
        }
    }
}
