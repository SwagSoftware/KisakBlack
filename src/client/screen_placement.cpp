#include "screen_placement.h"

#include <cstring>
#include <gfx_d3d/r_init.h>
#include <client_mp/cl_main_mp.h>

ScreenPlacement scrPlaceView[1];
ScreenPlacement scrPlaceFull;
ScreenPlacement scrPlaceFullUnsafe;

double __cdecl ScrPlace_HiResGetScale()
{
    if ( rg.renderHiResShot )
        return (double)rg.hiResShotTiles;
    else
        return 1.0;
}

void __cdecl ScrPlace_HiResApplyRect(float *x, float *y, float *w, float *h)
{
    if ( rg.renderHiResShot )
    {
        *x = (float)rg.hiResShotTiles * *x;
        *y = (float)rg.hiResShotTiles * *y;
        *w = (float)rg.hiResShotTiles * *w;
        *h = (float)rg.hiResShotTiles * *h;
        *x = *x - (double)cls.vidConfig.displayWidth * rg.hiResShotRow;
        *y = *y - (double)cls.vidConfig.displayHeight * rg.hiResShotCol;
    }
}

void __cdecl ScrPlace_SetupUI3DForFullscreen(ScreenPlacement *ui3DScrPlace, ScreenPlacement *fullScrPlace)
{
    if ( ui3DScrPlace )
    {
        if ( fullScrPlace )
            memcpy(ui3DScrPlace, fullScrPlace, sizeof(ScreenPlacement));
    }
}

void __cdecl ScrPlace_SetupFloatViewport(
                ScreenPlacement *scrPlace,
                float viewportX,
                float viewportY,
                float viewportWidth,
                float viewportHeight)
{
    float adjustedRealWidth; // [esp+84h] [ebp-10h]

    memset(scrPlace, 0xB0u, sizeof(ScreenPlacement));
    scrPlace->realViewportBase[0] = 0.0f;
    scrPlace->realViewportBase[1] = 0.0f;
    scrPlace->realViewportSize[0] = viewportWidth;
    scrPlace->realViewportSize[1] = viewportHeight;
    adjustedRealWidth = (float)(1.3333334 * viewportHeight) / cls.vidConfig.aspectRatioScenePixel;
    if ( adjustedRealWidth > viewportWidth )
        adjustedRealWidth = viewportWidth;
    ScrPlace_CalcSafeAreaOffsets(
        viewportX,
        viewportY,
        viewportWidth,
        viewportHeight,
        viewportWidth / adjustedRealWidth,
        viewportHeight / viewportHeight,
        1.0,
        1.0,
        scrPlace->realViewableMin,
        scrPlace->realViewableMax,
        scrPlace->virtualViewableMin,
        scrPlace->virtualViewableMax);
    ScrPlace_CalcSafeAreaOffsets(
        viewportX,
        viewportY,
        viewportWidth,
        viewportHeight,
        viewportWidth / adjustedRealWidth,
        viewportHeight / viewportHeight,
        1.0,
        1.0,
        scrPlace->realTweakableMin,
        scrPlace->realTweakableMax,
        scrPlace->virtualTweakableMin,
        scrPlace->virtualTweakableMax);
    scrPlace->scaleVirtualToReal[0] = adjustedRealWidth / 640.0;
    scrPlace->scaleVirtualToReal[1] = viewportHeight / 480.0;
    scrPlace->scaleVirtualToFull[0] = viewportWidth / 640.0;
    scrPlace->scaleVirtualToFull[1] = viewportHeight / 480.0;
    scrPlace->scaleRealToVirtual[0] = 640.0 / adjustedRealWidth;
    scrPlace->scaleRealToVirtual[1] = 480.0 / viewportHeight;
    scrPlace->subScreen[0] = (float)(viewportWidth - adjustedRealWidth) * 0.5;
    scrPlace->subScreen[1] = (float)(viewportHeight - viewportHeight) * 0.5;
}

void __cdecl ScrPlace_CalcSafeAreaOffsets(
                float viewportX,
                float viewportY,
                float viewportWidth,
                float viewportHeight,
                float horzAspectScale,
                float vertAspectScale,
                float safeAreaRatioHorz,
                float safeAreaRatioVert,
                float *realViewableMin,
                float *realViewableMax,
                float *virtualViewableMin,
                float *virtualViewableMax)
{
    const char *v12; // eax
    const char *v13; // eax
    float v14; // [esp+18h] [ebp-80h]
    float v15; // [esp+1Ch] [ebp-7Ch]
    float v16; // [esp+20h] [ebp-78h]
    float v17; // [esp+24h] [ebp-74h]
    float v18; // [esp+38h] [ebp-60h]
    float v19; // [esp+3Ch] [ebp-5Ch]
    float v20; // [esp+68h] [ebp-30h]
    float v21; // [esp+6Ch] [ebp-2Ch]
    float safeAreaSize; // [esp+88h] [ebp-10h]
    float safeAreaSize_4; // [esp+8Ch] [ebp-Ch]

    if ( (safeAreaRatioHorz < 0.0 || safeAreaRatioHorz > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp",
                    84,
                    0,
                    "safeAreaRatioHorz not in [0, 1]\n\t%g not in [%g, %g]",
                    safeAreaRatioHorz,
                    0.0,
                    1.0) )
    {
        __debugbreak();
    }
    if ( (safeAreaRatioVert < 0.0 || safeAreaRatioVert > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp",
                    85,
                    0,
                    "safeAreaRatioVert not in [0, 1]\n\t%g not in [%g, %g]",
                    safeAreaRatioVert,
                    0.0,
                    1.0) )
    {
        __debugbreak();
    }
    if ( !cls.vidConfig.displayWidth
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp",
                    86,
                    0,
                    "%s\n\t(cls.vidConfig.displayWidth) = %i",
                    "(cls.vidConfig.displayWidth > 0)",
                    0) )
    {
        __debugbreak();
    }
    if ( !cls.vidConfig.displayHeight
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp",
                    87,
                    0,
                    "%s\n\t(cls.vidConfig.displayHeight) = %i",
                    "(cls.vidConfig.displayHeight > 0)",
                    0) )
    {
        __debugbreak();
    }
    if ( viewportX < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp",
                    88,
                    0,
                    "%s\n\t(viewportX) = %g",
                    "(viewportX >= 0)",
                    viewportX) )
    {
        __debugbreak();
    }
    if ( viewportY < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp",
                    89,
                    0,
                    "%s\n\t(viewportY) = %g",
                    "(viewportY >= 0)",
                    viewportY) )
    {
        __debugbreak();
    }
    if ( viewportWidth <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp",
                    90,
                    0,
                    "%s\n\t(viewportWidth) = %g",
                    "(viewportWidth > 0)",
                    viewportWidth) )
    {
        __debugbreak();
    }
    if ( viewportHeight <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp",
                    91,
                    0,
                    "%s\n\t(viewportHeight) = %g",
                    "(viewportHeight > 0)",
                    viewportHeight) )
    {
        __debugbreak();
    }
    if ( (double)cls.vidConfig.displayWidth < viewportX + viewportWidth )
    {
        v12 = va("%g + %g > %i", viewportX, viewportWidth, cls.vidConfig.displayWidth);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp",
                        92,
                        0,
                        "%s\n\t%s",
                        "viewportX + viewportWidth <= cls.vidConfig.displayWidth",
                        v12) )
            __debugbreak();
    }
    if ( (double)cls.vidConfig.displayHeight < viewportY + viewportHeight )
    {
        v13 = va("%g + %g > %i", viewportY, viewportHeight, cls.vidConfig.displayHeight);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp",
                        93,
                        0,
                        "%s\n\t%s",
                        "viewportY + viewportHeight <= cls.vidConfig.displayHeight",
                        v13) )
            __debugbreak();
    }
    safeAreaSize = (1.0 - safeAreaRatioHorz) * 0.5 * (double)cls.vidConfig.displayWidth;
    safeAreaSize_4 = (1.0 - safeAreaRatioVert) * 0.5 * (double)cls.vidConfig.displayHeight;
    v19 = floor(safeAreaSize + 0.5);
    v18 = floor(safeAreaSize_4 + 0.5);
    if ( (float)(viewportX - v19) < 0.0 )
        v17 = v19;
    else
        v17 = viewportX;
    if ( (float)(viewportY - v18) < 0.0 )
        v16 = v18;
    else
        v16 = viewportY;
    v20 = (double)cls.vidConfig.displayWidth - v19;
    if ( (float)(v20 - (float)(viewportX + viewportWidth)) < 0.0 )
        v15 = (double)cls.vidConfig.displayWidth - v19;
    else
        v15 = viewportX + viewportWidth;
    v21 = (double)cls.vidConfig.displayHeight - v18;
    if ( (float)(v21 - (float)(viewportY + viewportHeight)) < 0.0 )
        v14 = (double)cls.vidConfig.displayHeight - v18;
    else
        v14 = viewportY + viewportHeight;
    *realViewableMin = v17 - viewportX;
    realViewableMin[1] = v16 - viewportY;
    *realViewableMax = v15 - viewportX;
    realViewableMax[1] = v14 - viewportY;
    *virtualViewableMin = (float)(*realViewableMin * horzAspectScale) * (float)(640.0 / viewportWidth);
    virtualViewableMin[1] = (float)(realViewableMin[1] * vertAspectScale) * (float)(480.0 / viewportHeight);
    *virtualViewableMax = (float)(*realViewableMax * horzAspectScale) * (float)(640.0 / viewportWidth);
    virtualViewableMax[1] = (float)(realViewableMax[1] * vertAspectScale) * (float)(480.0 / viewportHeight);
}

void __cdecl ScrPlace_SetupViewport(
                ScreenPlacement *scrPlace,
                int viewportX,
                int viewportY,
                int viewportWidth,
                int viewportHeight)
{
    ScrPlace_SetupFloatViewport(scrPlace, (float)viewportX, (float)viewportY, (float)viewportWidth, (float)viewportHeight);
}

void __cdecl ScrPlace_SetupUnsafeViewport(
                ScreenPlacement *scrPlace,
                int viewportX,
                int viewportY,
                int viewportWidth,
                int viewportHeight)
{
    ScrPlace_SetupFloatViewport(scrPlace, (float)viewportX, (float)viewportY, (float)viewportWidth, (float)viewportHeight);
}

double __cdecl ScrPlace_ApplyXinverse(const ScreenPlacement *scrPlace, float x, int horzAlign)
{
    float xa; // [esp+18h] [ebp+Ch]

    if ( rg.renderHiResShot )
    {
        xa = (double)cls.vidConfig.displayWidth * rg.hiResShotRow + x;
        x = xa / (float)rg.hiResShotTiles;
    }
    switch ( horzAlign )
    {
        case 1:
            return (x - scrPlace->realViewableMin[0]) / scrPlace->scaleVirtualToReal[0];
        case 4:
            return (x - scrPlace->realViewportBase[0]) / scrPlace->scaleVirtualToFull[0];
        case 5:
            return x;
    }
    if ( horzAlign )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp",
                        257,
                        0,
                        "%s\n\t(horzAlign) = %i",
                        "(horzAlign == 0)",
                        horzAlign) )
            __debugbreak();
    }
    return (x - scrPlace->subScreen[0]) / scrPlace->scaleVirtualToReal[0];
}

double __cdecl ScrPlace_ApplyYinverse(const ScreenPlacement *scrPlace, float y, int vertAlign)
{
    float ya; // [esp+18h] [ebp+Ch]

    if ( rg.renderHiResShot )
    {
        ya = (double)cls.vidConfig.displayWidth * rg.hiResShotCol + y;
        y = ya / (float)rg.hiResShotTiles;
    }
    switch ( vertAlign )
    {
        case 4:
            return (y - scrPlace->realViewportBase[1]) / scrPlace->scaleVirtualToFull[1];
        case 5:
            return y;
        case 6:
            return y / scrPlace->scaleRealToVirtual[1];
    }
    if ( vertAlign )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp",
                        291,
                        0,
                        "%s\n\t(vertAlign) = %i",
                        "(vertAlign == 0)",
                        vertAlign) )
            __debugbreak();
    }
    return (y - scrPlace->subScreen[1]) / scrPlace->scaleVirtualToReal[1];
}

double __cdecl ScrPlace_ApplyX(const ScreenPlacement *scrPlace, float x, int horzAlign)
{
    float ret; // [esp+Ch] [ebp-4h]

    switch ( horzAlign )
    {
        case 1:
            ret = (float)(x * scrPlace->scaleVirtualToReal[0]) + scrPlace->realViewableMin[0];
            break;
        case 2:
            ret = (float)((float)(x * scrPlace->scaleVirtualToReal[0]) + (float)(scrPlace->realViewportSize[0] * 0.5))
                    + scrPlace->realViewportBase[0];
            break;
        case 3:
            ret = (float)(x * scrPlace->scaleVirtualToReal[0]) + scrPlace->realViewableMax[0];
            break;
        case 4:
            ret = (float)(x * scrPlace->scaleVirtualToFull[0]) + scrPlace->realViewportBase[0];
            break;
        case 5:
            ret = x;
            break;
        case 6:
            ret = x * scrPlace->scaleRealToVirtual[0];
            break;
        case 7:
            ret = (float)(x * scrPlace->scaleVirtualToReal[0])
                    + (float)((float)(scrPlace->realViewableMin[0] + scrPlace->realViewableMax[0]) * 0.5);
            break;
        case 8:
            ret = (float)(x * scrPlace->scaleVirtualToReal[0]) + scrPlace->realTweakableMin[0];
            break;
        case 9:
            ret = (float)(x * scrPlace->scaleVirtualToReal[0])
                    + (float)((float)(scrPlace->realTweakableMin[0] + scrPlace->realTweakableMax[0]) * 0.5);
            break;
        case 10:
            ret = (float)(x * scrPlace->scaleVirtualToReal[0]) + scrPlace->realTweakableMax[0];
            break;
        default:
            if ( horzAlign
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp",
                            321,
                            0,
                            "%s\n\t(horzAlign) = %i",
                            "(horzAlign == 0)",
                            horzAlign) )
            {
                __debugbreak();
            }
            ret = (float)(x * scrPlace->scaleVirtualToReal[0]) + scrPlace->subScreen[0];
            break;
    }
    if ( rg.renderHiResShot )
        return (float)((float)((float)rg.hiResShotTiles * ret) - (double)cls.vidConfig.displayWidth * rg.hiResShotRow);
    return ret;
}

double __cdecl ScrPlace_ApplyY(const ScreenPlacement *scrPlace, float y, int vertAlign)
{
    float ret; // [esp+Ch] [ebp-4h]

    switch ( vertAlign )
    {
        case 1:
            ret = (float)(y * scrPlace->scaleVirtualToReal[1]) + scrPlace->realViewableMin[1];
            break;
        case 2:
            ret = (float)((float)(y * scrPlace->scaleVirtualToReal[1]) + (float)(scrPlace->realViewportSize[1] * 0.5))
                    + scrPlace->realViewportBase[1];
            break;
        case 3:
            ret = (float)(y * scrPlace->scaleVirtualToReal[1]) + scrPlace->realViewableMax[1];
            break;
        case 4:
            ret = (float)(y * scrPlace->scaleVirtualToFull[1]) + scrPlace->realViewportBase[1];
            break;
        case 5:
            ret = y;
            break;
        case 6:
            ret = y * scrPlace->scaleRealToVirtual[1];
            break;
        case 7:
            ret = (float)(y * scrPlace->scaleVirtualToReal[1])
                    + (float)((float)(scrPlace->realViewableMin[1] + scrPlace->realViewableMax[1]) * 0.5);
            break;
        case 8:
            ret = (float)(y * scrPlace->scaleVirtualToReal[1]) + scrPlace->realTweakableMin[1];
            break;
        case 9:
            ret = (float)(y * scrPlace->scaleVirtualToReal[1])
                    + (float)((float)(scrPlace->realTweakableMin[1] + scrPlace->realTweakableMax[1]) * 0.5);
            break;
        case 10:
            ret = (float)(y * scrPlace->scaleVirtualToReal[1]) + scrPlace->realTweakableMax[1];
            break;
        default:
            if ( vertAlign
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp",
                            388,
                            0,
                            "%s\n\t(vertAlign) = %i",
                            "(vertAlign == 0)",
                            vertAlign) )
            {
                __debugbreak();
            }
            ret = (float)(y * scrPlace->scaleVirtualToReal[1]) + scrPlace->subScreen[1];
            break;
    }
    if ( rg.renderHiResShot )
        return (float)((float)((float)rg.hiResShotTiles * ret) - (double)cls.vidConfig.displayHeight * rg.hiResShotCol);
    return ret;
}

double __cdecl ScrPlace_ApplyW(const ScreenPlacement *scrPlace, float w, int horzAlign)
{
    switch ( horzAlign )
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            goto $LN10_32;
        case 4:
            w = w * scrPlace->scaleVirtualToFull[0];
            break;
        case 5:
            break;
        default:
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp",
                            456,
                            0,
                            "invalid horizontal alignment case") )
                __debugbreak();
$LN10_32:
            w = w * scrPlace->scaleVirtualToReal[0];
            break;
    }
    if ( rg.renderHiResShot )
        return (float)((float)rg.hiResShotTiles * w);
    return w;
}

double __cdecl ScrPlace_ApplyH(const ScreenPlacement *scrPlace, float h, int vertAlign)
{
    switch ( vertAlign )
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            goto $LN9_35;
        case 4:
            h = h * scrPlace->scaleVirtualToFull[1];
            break;
        case 5:
            return h;
        default:
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp",
                            498,
                            0,
                            "invalid vertical alignment case") )
                __debugbreak();
$LN9_35:
            h = h * scrPlace->scaleVirtualToReal[1];
            break;
    }
    return h;
}

void __cdecl ScrPlace_ApplyRect(
                const ScreenPlacement *scrPlace,
                float *x,
                float *y,
                float *w,
                float *h,
                int horzAlign,
                int vertAlign)
{
    if ( !x && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp", 545, 0, "%s", "x") )
        __debugbreak();
    if ( !y && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp", 546, 0, "%s", "y") )
        __debugbreak();
    if ( !w && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp", 547, 0, "%s", "w") )
        __debugbreak();
    if ( !h && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp", 548, 0, "%s", "h") )
        __debugbreak();
    switch ( horzAlign )
    {
        case 0:
            goto $LN36_9;
        case 1:
            *x = (float)(*x * scrPlace->scaleVirtualToReal[0]) + scrPlace->realViewableMin[0];
            *w = *w * scrPlace->scaleVirtualToReal[0];
            break;
        case 2:
            *x = (float)((float)(*x * scrPlace->scaleVirtualToReal[0]) + (float)(scrPlace->realViewportSize[0] * 0.5))
                 + scrPlace->realViewportBase[0];
            *w = *w * scrPlace->scaleVirtualToReal[0];
            break;
        case 3:
            *x = (float)(*x * scrPlace->scaleVirtualToReal[0]) + scrPlace->realViewableMax[0];
            *w = *w * scrPlace->scaleVirtualToReal[0];
            break;
        case 4:
            *x = (float)(scrPlace->scaleVirtualToFull[0] + scrPlace->realViewportBase[0]) * *x;
            *w = *w * scrPlace->scaleVirtualToFull[0];
            break;
        case 5:
            break;
        case 6:
            *x = *x * scrPlace->scaleRealToVirtual[0];
            *w = *w * scrPlace->scaleRealToVirtual[0];
            break;
        case 7:
            *x = (float)(*x * scrPlace->scaleVirtualToReal[0])
                 + (float)((float)(scrPlace->realViewableMin[0] + scrPlace->realViewableMax[0]) * 0.5);
            *w = *w * scrPlace->scaleVirtualToReal[0];
            break;
        case 8:
            *x = (float)(*x * scrPlace->scaleVirtualToReal[0]) + scrPlace->realTweakableMin[0];
            *w = *w * scrPlace->scaleVirtualToReal[0];
            break;
        case 9:
            *x = (float)(*x * scrPlace->scaleVirtualToReal[0])
                 + (float)((float)(scrPlace->realTweakableMin[0] + scrPlace->realTweakableMax[0]) * 0.5);
            *w = *w * scrPlace->scaleVirtualToReal[0];
            break;
        case 10:
            *x = (float)(*x * scrPlace->scaleVirtualToReal[0]) + scrPlace->realTweakableMax[0];
            *w = *w * scrPlace->scaleVirtualToReal[0];
            break;
        default:
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp",
                            566,
                            0,
                            "invalid horizontal alignment case") )
                __debugbreak();
$LN36_9:
            *x = (float)(*x * scrPlace->scaleVirtualToReal[0]) + scrPlace->subScreen[0];
            *w = *w * scrPlace->scaleVirtualToReal[0];
            break;
    }
    switch ( vertAlign )
    {
        case 0:
            goto $LN37_6;
        case 1:
            *y = (float)(*y * scrPlace->scaleVirtualToReal[1]) + scrPlace->realViewableMin[1];
            *h = *h * scrPlace->scaleVirtualToReal[1];
            break;
        case 2:
            *y = (float)((float)(*y * scrPlace->scaleVirtualToReal[1]) + (float)(scrPlace->realViewportSize[1] * 0.5))
                 + scrPlace->realViewportBase[1];
            *h = *h * scrPlace->scaleVirtualToReal[1];
            break;
        case 3:
            *y = (float)(*y * scrPlace->scaleVirtualToReal[1]) + scrPlace->realViewableMax[1];
            *h = *h * scrPlace->scaleVirtualToReal[1];
            break;
        case 4:
            *y = (float)(scrPlace->scaleVirtualToFull[1] + scrPlace->realViewportBase[1]) * *y;
            *h = *h * scrPlace->scaleVirtualToFull[1];
            break;
        case 5:
            break;
        case 6:
            *y = *y * scrPlace->scaleRealToVirtual[1];
            *h = *h * scrPlace->scaleRealToVirtual[1];
            break;
        case 7:
            *y = (float)(*y * scrPlace->scaleVirtualToReal[1])
                 + (float)((float)(scrPlace->realViewableMin[1] + scrPlace->realViewableMax[1]) * 0.5);
            *h = *h * scrPlace->scaleVirtualToReal[1];
            break;
        case 8:
            *y = (float)(*y * scrPlace->scaleVirtualToReal[1]) + scrPlace->realTweakableMin[1];
            *h = *h * scrPlace->scaleVirtualToReal[1];
            break;
        case 9:
            *y = (float)(*y * scrPlace->scaleVirtualToReal[1])
                 + (float)((float)(scrPlace->realTweakableMin[1] + scrPlace->realTweakableMax[1]) * 0.5);
            *h = *h * scrPlace->scaleVirtualToReal[1];
            break;
        case 10:
            *y = (float)(*y * scrPlace->scaleVirtualToReal[1]) + scrPlace->realTweakableMax[1];
            *h = *h * scrPlace->scaleVirtualToReal[1];
            break;
        default:
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client\\screen_placement.cpp",
                            614,
                            0,
                            "invalid vertical alignment case") )
                __debugbreak();
$LN37_6:
            *y = (float)(*y * scrPlace->scaleVirtualToReal[1]) + scrPlace->subScreen[1];
            *h = *h * scrPlace->scaleVirtualToReal[1];
            break;
    }
    if ( rg.renderHiResShot )
    {
        *x = (float)rg.hiResShotTiles * *x;
        *y = (float)rg.hiResShotTiles * *y;
        *w = (float)rg.hiResShotTiles * *w;
        *h = (float)rg.hiResShotTiles * *h;
        *x = *x - (double)cls.vidConfig.displayWidth * rg.hiResShotRow;
        *y = *y - (double)cls.vidConfig.displayHeight * rg.hiResShotCol;
    }
}

