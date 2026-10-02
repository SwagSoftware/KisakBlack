#include "devgui_util.h"

#include <client_mp/cl_main_mp.h>
#include <gfx_d3d/r_rendercmds.h>
#include <gfx_d3d/r_font.h>

unsigned int __cdecl DevGui_GetScreenWidth()
{
    return cls.vidConfig.displayWidth;
}

unsigned int __cdecl DevGui_GetScreenHeight()
{
    return cls.vidConfig.displayHeight;
}

void __cdecl DevGui_DrawBox(int x, int y, int w, int h, const unsigned __int8 *color)
{
    float unpackedColor[4]; // [esp+48h] [ebp-10h] BYREF

    if ( !w && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui_util.cpp", 114, 0, "%s", "w") )
        __debugbreak();
    if ( !h && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui_util.cpp", 115, 0, "%s", "h") )
        __debugbreak();
    Byte4UnpackRgba(color, unpackedColor);
    R_AddCmdDrawStretchPic((float)x, (float)y, (float)w, (float)h, 0.0, 0.0, 0.0, 0.0, unpackedColor, cls.whiteMaterial);
}

void __cdecl DevGui_DrawBoxCentered(int centerX, int centerY, int w, int h, const unsigned __int8 *color)
{
    DevGui_DrawBox(centerX - w / 2, centerY - h / 2, w, h, color);
}

void __cdecl DevGui_DrawBevelBox(int x, int y, int w, int h, float shade, const unsigned __int8 *color)
{
    float v6; // [esp+0h] [ebp-90h]
    float v7; // [esp+4h] [ebp-8Ch]
    float v8; // [esp+8h] [ebp-88h]
    float v9; // [esp+Ch] [ebp-84h]
    float v10; // [esp+10h] [ebp-80h]
    float v11; // [esp+14h] [ebp-7Ch]
    float v12; // [esp+18h] [ebp-78h]
    float v13; // [esp+1Ch] [ebp-74h]
    float v14; // [esp+20h] [ebp-70h]
    float v15; // [esp+24h] [ebp-6Ch]
    float v16; // [esp+28h] [ebp-68h]
    float v17; // [esp+2Ch] [ebp-64h]
    float v18; // [esp+30h] [ebp-60h]
    float v19; // [esp+34h] [ebp-5Ch]
    float v20; // [esp+38h] [ebp-58h]
    float v21; // [esp+3Ch] [ebp-54h]
    float v22; // [esp+40h] [ebp-50h]
    float v23; // [esp+44h] [ebp-4Ch]
    float v24; // [esp+48h] [ebp-48h]
    float v25; // [esp+4Ch] [ebp-44h]
    float v26; // [esp+50h] [ebp-40h]
    float v27; // [esp+54h] [ebp-3Ch]
    float v28; // [esp+58h] [ebp-38h]
    float v29; // [esp+5Ch] [ebp-34h]
    int vtxs[4][2]; // [esp+60h] [ebp-30h] BYREF
    float unpackedColor[4]; // [esp+80h] [ebp-10h] BYREF

    if ( (float)w < 8.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui_util.cpp",
                    150,
                    0,
                    "%s",
                    "w >= (DEVGUI_BEVEL_SIZE * 2.0f)") )
    {
        __debugbreak();
    }
    if ( (float)h < 8.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui_util.cpp",
                    151,
                    0,
                    "%s",
                    "h >= (DEVGUI_BEVEL_SIZE * 2.0f)") )
    {
        __debugbreak();
    }
    Byte4UnpackRgba(color, unpackedColor);
    DevGui_DrawBox(x, y, w, h, color);
    unpackedColor[0] = shade * unpackedColor[0];
    unpackedColor[1] = shade * unpackedColor[1];
    unpackedColor[2] = shade * unpackedColor[2];
    if ( (float)(unpackedColor[0] - 1.0) < 0.0 )
        v29 = unpackedColor[0];
    else
        v29 = 1.0f;
    if ( (float)(0.0 - unpackedColor[0]) < 0.0 )
        v17 = v29;
    else
        v17 = 0.0f;
    unpackedColor[0] = v17;
    if ( (float)(unpackedColor[1] - 1.0) < 0.0 )
        v28 = unpackedColor[1];
    else
        v28 = 1.0f;
    if ( (float)(0.0 - unpackedColor[1]) < 0.0 )
        v16 = v28;
    else
        v16 = 0.0f;
    unpackedColor[1] = v16;
    if ( (float)(unpackedColor[2] - 1.0) < 0.0 )
        v27 = unpackedColor[2];
    else
        v27 = 1.0f;
    if ( (float)(0.0 - unpackedColor[2]) < 0.0 )
        v15 = v27;
    else
        v15 = 0.0f;
    unpackedColor[2] = v15;
    *(_QWORD *)&vtxs[0][0] = __PAIR64__(y, x);
    vtxs[1][0] = x + 4;
    vtxs[1][1] = y + 4;
    vtxs[2][0] = x + 4;
    vtxs[2][1] = y + h - 4;
    vtxs[3][0] = x;
    vtxs[3][1] = h + y;
    DevGui_DrawQuad(vtxs, unpackedColor);
    unpackedColor[0] = shade * unpackedColor[0];
    unpackedColor[1] = shade * unpackedColor[1];
    unpackedColor[2] = shade * unpackedColor[2];
    if ( (float)(unpackedColor[0] - 1.0) < 0.0 )
        v26 = unpackedColor[0];
    else
        v26 = 1.0f;
    if ( (float)(0.0 - unpackedColor[0]) < 0.0 )
        v14 = v26;
    else
        v14 = 0.0f;
    unpackedColor[0] = v14;
    if ( (float)(unpackedColor[1] - 1.0) < 0.0 )
        v25 = unpackedColor[1];
    else
        v25 = 1.0f;
    if ( (float)(0.0 - unpackedColor[1]) < 0.0 )
        v13 = v25;
    else
        v13 = 0.0f;
    unpackedColor[1] = v13;
    if ( (float)(unpackedColor[2] - 1.0) < 0.0 )
        v24 = unpackedColor[2];
    else
        v24 = 1.0f;
    if ( (float)(0.0 - unpackedColor[2]) < 0.0 )
        v12 = v24;
    else
        v12 = 0.0f;
    unpackedColor[2] = v12;
    *(_QWORD *)&vtxs[0][0] = __PAIR64__(y, x);
    vtxs[1][0] = w + x;
    vtxs[1][1] = y;
    vtxs[2][0] = x + w - 4;
    vtxs[2][1] = y + 4;
    vtxs[3][0] = x + 4;
    vtxs[3][1] = y + 4;
    DevGui_DrawQuad(vtxs, unpackedColor);
    unpackedColor[0] = shade * unpackedColor[0];
    unpackedColor[1] = shade * unpackedColor[1];
    unpackedColor[2] = shade * unpackedColor[2];
    if ( (float)(unpackedColor[0] - 1.0) < 0.0 )
        v23 = unpackedColor[0];
    else
        v23 = 1.0f;
    if ( (float)(0.0 - unpackedColor[0]) < 0.0 )
        v11 = v23;
    else
        v11 = 0.0f;
    unpackedColor[0] = v11;
    if ( (float)(unpackedColor[1] - 1.0) < 0.0 )
        v22 = unpackedColor[1];
    else
        v22 = 1.0f;
    if ( (float)(0.0 - unpackedColor[1]) < 0.0 )
        v10 = v22;
    else
        v10 = 0.0f;
    unpackedColor[1] = v10;
    if ( (float)(unpackedColor[2] - 1.0) < 0.0 )
        v21 = unpackedColor[2];
    else
        v21 = 1.0f;
    if ( (float)(0.0 - unpackedColor[2]) < 0.0 )
        v9 = v21;
    else
        v9 = 0.0f;
    unpackedColor[2] = v9;
    vtxs[0][0] = x;
    vtxs[0][1] = h + y;
    vtxs[1][0] = x + 4;
    vtxs[1][1] = y + h - 4;
    vtxs[2][0] = x + w - 4;
    vtxs[2][1] = vtxs[1][1];
    vtxs[3][0] = w + x;
    vtxs[3][1] = h + y;
    DevGui_DrawQuad(vtxs, unpackedColor);
    unpackedColor[0] = shade * unpackedColor[0];
    unpackedColor[1] = shade * unpackedColor[1];
    unpackedColor[2] = shade * unpackedColor[2];
    if ( (float)(unpackedColor[0] - 1.0) < 0.0 )
        v20 = unpackedColor[0];
    else
        v20 = 1.0f;
    if ( (float)(0.0 - unpackedColor[0]) < 0.0 )
        v8 = v20;
    else
        v8 = 0.0f;
    unpackedColor[0] = v8;
    if ( (float)(unpackedColor[1] - 1.0) < 0.0 )
        v19 = unpackedColor[1];
    else
        v19 = 1.0f;
    if ( (float)(0.0 - unpackedColor[1]) < 0.0 )
        v7 = v19;
    else
        v7 = 0.0f;
    unpackedColor[1] = v7;
    if ( (float)(unpackedColor[2] - 1.0) < 0.0 )
        v18 = unpackedColor[2];
    else
        v18 = 1.0f;
    if ( (float)(0.0 - unpackedColor[2]) < 0.0 )
        v6 = v18;
    else
        v6 = 0.0f;
    unpackedColor[2] = v6;
    vtxs[0][0] = w + x;
    vtxs[0][1] = y;
    vtxs[1][0] = w + x;
    vtxs[1][1] = h + y;
    vtxs[2][0] = x + w - 4;
    vtxs[2][1] = y + h - 4;
    vtxs[3][0] = vtxs[2][0];
    vtxs[3][1] = y + 4;
    DevGui_DrawQuad(vtxs, unpackedColor);
}

void __cdecl DevGui_DrawQuad(const int (*vtxs)[2], const float *color)
{
    float xy[4][2]; // [esp+0h] [ebp-20h] BYREF

    xy[0][0] = (float)(*vtxs)[0];
    xy[0][1] = (float)(*vtxs)[1];
    xy[1][0] = (float)(*vtxs)[2];
    xy[1][1] = (float)(*vtxs)[3];
    xy[2][0] = (float)(*vtxs)[4];
    xy[2][1] = (float)(*vtxs)[5];
    xy[3][0] = (float)(*vtxs)[6];
    xy[3][1] = (float)(*vtxs)[7];
    R_AddCmdDrawQuadPic(xy, color, cls.whiteMaterial);
}

void  DevGui_DrawLine(float *start, float *end, int width, const unsigned __int8 *color)
{
    float h; // [esp+Ch] [ebp-C0h]
    float x; // [esp+2Ch] [ebp-A0h]
    float y; // [esp+30h] [ebp-9Ch]
    float v7; // [esp+38h] [ebp-94h]
    float v8; // [esp+3Ch] [ebp-90h]
    float v9; // [esp+40h] [ebp-8Ch]
    float v10; // [esp+44h] [ebp-88h]
    float v11; // [esp+48h] [ebp-84h]
    float v12; // [esp+4Ch] [ebp-80h]
    float v13; // [esp+50h] [ebp-7Ch]
    float v14; // [esp+54h] [ebp-78h]
    float v15; // [esp+58h] [ebp-74h]
    float v16; // [esp+70h] [ebp-5Ch]
    float v17; // [esp+74h] [ebp-58h]
    float v18; // [esp+78h] [ebp-54h]
    float v19; // [esp+7Ch] [ebp-50h]
    float v20; // [esp+80h] [ebp-4Ch]
    float v21; // [esp+84h] [ebp-48h]
    float v22; // [esp+88h] [ebp-44h]
    float v23; // [esp+8Ch] [ebp-40h]
    float pos; // [esp+90h] [ebp-3Ch]
    float pos_4; // [esp+94h] [ebp-38h]
    float diff[3]; // [esp+98h] [ebp-34h] BYREF
    float tl[2]; // [esp+A4h] [ebp-28h]
    float angle; // [esp+ACh] [ebp-20h]
    float len; // [esp+B0h] [ebp-1Ch]
    float br[2]; // [esp+B4h] [ebp-18h]
    float unpackedColor[4]; // [esp+BCh] [ebp-10h] BYREF

    v22 = *start;
    v23 = *end;
    v15 = v23 - v22;
    if (v15 < 0.0)
        v14 = v23;
    else
        v14 = v22;
    tl[0] = v14;
    v20 = start[1];
    v21 = end[1];
    v13 = v21 - v20;
    if (v13 < 0.0)
        v12 = v21;
    else
        v12 = v20;
    tl[1] = v12;
    v18 = *start;
    v19 = *end;
    v11 = v18 - v19;
    if (v11 < 0.0)
        v10 = v19;
    else
        v10 = v18;
    br[0] = v10;
    v16 = start[1];
    v17 = end[1];
    v9 = v16 - v17;
    if (v9 < 0.0)
        v8 = v17;
    else
        v8 = v16;
    br[1] = v8;
    diff[0] = *end - *start;
    diff[1] = end[1] - start[1];
    diff[2] = 0.0;
    pos = (br[0] - tl[0]) / 2.0 + tl[0];
    pos_4 = (v8 - tl[1]) / 2.0 + tl[1];
    len = Vec2Length(diff);
    Vec3Normalize(diff);
    v7 = acos(diff[0]);
    angle = RAD2DEG(v7);
    if (start[1] > (double)end[1])
        angle = -angle;
    while (angle < 0.0)
        angle = angle + 360.0;
    Byte4UnpackRgba(color, unpackedColor);
    h = (float)width;
    y = pos_4 - (double)(width / 2);
    x = pos - len / 2.0;
    R_AddCmdDrawStretchPicRotateXY(x, y, len, h, 0.0, 0.0, 1.0, 1.0, angle, unpackedColor, cls.whiteMaterial);
}

void __cdecl DevGui_DrawFont(int x, int y, const unsigned __int8 *color, char *text, float xScale, float yScale)
{
    int FontHeight; // eax
    float unpackedColor[4]; // [esp+1Ch] [ebp-10h] BYREF

    if ( !text && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\devgui\\devgui_util.cpp", 288, 0, "%s", "text") )
        __debugbreak();
    if ( *text )
    {
        Byte4UnpackRgba(color, unpackedColor);
        FontHeight = DevGui_GetFontHeight();
        R_AddCmdDrawText(
            text,
            0x7FFFFFFF,
            cls.consoleFont,
            (float)x,
            (float)(y + FontHeight),
            xScale,
            yScale,
            0.0,
            unpackedColor,
            0);
    }
}

int __cdecl DevGui_GetFontWidth(const char *text)
{
    return R_TextWidth(text, 0, cls.consoleFont);
}

int __cdecl DevGui_GetFontHeight()
{
    return R_TextHeight(cls.consoleFont);
}

