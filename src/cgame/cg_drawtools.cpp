#include "cg_drawtools.h"
#include <gfx_d3d/r_gfx.h>
#include <gfx_d3d/r_material.h>
#include <gfx_d3d/r_rendercmds.h>
#include <gfx_d3d/r_font.h>
#include <cgame_mp/cg_main_mp.h>
#include <client_mp/cl_cgame_mp.h>
#include <universal/q_parse.h>
#include <universal/com_stringtable.h>
#include <qcommon/com_clients.h>
#include <ui/ui_localvars.h>
#include <clientscript/cscr_vm.h>
#include "cg_draw_names.h"
#include <xanim/xmodel.h>
#include <qcommon/dobj_management.h>
#include <xanim/dobj_utils.h>
#include <universal/com_math_anglevectors.h>
#include <xanim/xmodel_utils.h>
#include <client/cl_debugdata.h>

float color_2[4];

int boxVerts_0[24][3] =
{
  { 0, 0, 0 },
  { 1, 0, 0 },
  { 0, 0, 0 },
  { 0, 1, 0 },
  { 1, 1, 0 },
  { 1, 0, 0 },
  { 1, 1, 0 },
  { 0, 1, 0 },
  { 0, 0, 1 },
  { 1, 0, 1 },
  { 0, 0, 1 },
  { 0, 1, 1 },
  { 1, 1, 1 },
  { 1, 0, 1 },
  { 1, 1, 1 },
  { 0, 1, 1 },
  { 0, 0, 0 },
  { 0, 0, 1 },
  { 1, 0, 0 },
  { 1, 0, 1 },
  { 0, 1, 0 },
  { 0, 1, 1 },
  { 1, 1, 0 },
  { 1, 1, 1 }
};

double __cdecl CG_GetPicWidth(Material *mat)
{
    return (double)Material_FromHandle(mat)->textureTable->u.image->width;
}

const Material *__cdecl Material_FromHandle(Material *handle)
{
    if ( !handle
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../gfx_d3d/r_material.h", 538, 0, "%s", "handle") )
    {
        __debugbreak();
    }
    if ( !handle->info.name
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../gfx_d3d/r_material.h",
                    539,
                    0,
                    "%s",
                    "handle->info.name") )
    {
        __debugbreak();
    }
    if ( !*handle->info.name
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../gfx_d3d/r_material.h",
                    541,
                    0,
                    "%s",
                    "handle->info.name[0]") )
    {
        __debugbreak();
    }
    return handle;
}

const float sign[4][2] =
{ { -1.0, -1.0 }, { 1.0, -1.0 }, { 1.0, 1.0 }, { -1.0, 1.0 } };

void __cdecl CG_DrawRotatedPicPhysicalW(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float w,
    float width,
    float height,
    float angle,
    const float *color,
    Material *material)
{
    long double v9; // st7
    float v10; // [esp+Ch] [ebp-70h]
    float v11; // [esp+Ch] [ebp-70h]
    float v12; // [esp+10h] [ebp-6Ch]
    float v13; // [esp+10h] [ebp-6Ch]
    float v14; // [esp+14h] [ebp-68h]
    float v15; // [esp+18h] [ebp-64h]
    float v16; // [esp+1Ch] [ebp-60h]
    float v17; // [esp+20h] [ebp-5Ch]
    float cosval; // [esp+30h] [ebp-4Ch]
    float halfWidth; // [esp+34h] [ebp-48h]
    float verts[4][2]; // [esp+38h] [ebp-44h] BYREF
    int i; // [esp+58h] [ebp-24h]
    float scale[2][2]; // [esp+5Ch] [ebp-20h]
    float halfHeight; // [esp+6Ch] [ebp-10h]
    float center[2]; // [esp+70h] [ebp-Ch]
    float sinval; // [esp+78h] [ebp-4h]

    v9 = (float)(DEG2RAD(angle));
    cosval = cos(v9);
    sinval = sin(v9);
    v17 = (float)(width * 0.5) / ScrPlace_HiResGetScale();
    halfWidth = v17 * scrPlace->scaleRealToVirtual[0];
    v16 = (float)(height * 0.5) / ScrPlace_HiResGetScale();
    halfHeight = v16 * scrPlace->scaleRealToVirtual[1];
    v15 = x / ScrPlace_HiResGetScale();
    center[0] = (float)(v15 * scrPlace->scaleRealToVirtual[0]) + halfWidth;
    v14 = y / ScrPlace_HiResGetScale();
    center[1] = (float)(v14 * scrPlace->scaleRealToVirtual[1]) + halfHeight;
    scale[0][0] = cosval * halfWidth;
    scale[0][1] = sinval * halfWidth;
    scale[1][0] = sinval * halfHeight;
    scale[1][1] = cosval * halfHeight;
    for (i = 0; i < 4; ++i)
    {
        v12 = (float)((float)((float)sign[i][0] * scale[0][0]) + center[0]) - (float)((float)sign[i][1] * scale[1][0]);
        v13 = ScrPlace_HiResGetScale() * v12;
        verts[i][0] = v13 * scrPlace->scaleVirtualToReal[0];
        v10 = (float)((float)((float)sign[i][0] * scale[0][1]) + center[1]) + (float)((float)sign[i][1] * scale[1][1]);
        v11 = ScrPlace_HiResGetScale() * v10;
        verts[i][1] = v11 * scrPlace->scaleVirtualToReal[1];
    }
    R_AddCmdDrawQuadPicW(verts, w, color, material);
}

void __cdecl CG_DrawRotatedPicPhysical(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float width,
                float height,
                float angle,
                const float *color,
                Material *material)
{
    CG_DrawRotatedPicPhysicalW(scrPlace, x, y, 1.0, width, height, angle, color, material);
}

void __cdecl CG_DrawRotatedPicW(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float width,
                float height,
                int horzAlign,
                int vertAlign,
                float angle,
                const float *color,
                Material *material)
{
    ScrPlace_ApplyRect(scrPlace, &x, &y, &width, &height, horzAlign, vertAlign);
    CG_DrawRotatedPicPhysicalW(scrPlace, x, y, w, width, height, angle, color, material);
}

void __cdecl CG_DrawRotatedPic(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float width,
                float height,
                int horzAlign,
                int vertAlign,
                float angle,
                const float *color,
                Material *material)
{
    ScrPlace_ApplyRect(scrPlace, &x, &y, &width, &height, horzAlign, vertAlign);
    CG_DrawRotatedPicPhysical(scrPlace, x, y, width, height, angle, color, material);
}

void __cdecl CG_DrawRotatedQuadPic(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                const float (*verts)[2],
                float angle,
                const float *color,
                Material *material)
{
    long double v7; // st7
    float v8; // [esp+0h] [ebp-70h]
    float v9; // [esp+0h] [ebp-70h]
    float v10; // [esp+8h] [ebp-68h]
    float v11; // [esp+8h] [ebp-68h]
    float v12; // [esp+Ch] [ebp-64h]
    float v13; // [esp+10h] [ebp-60h]
    float v14; // [esp+10h] [ebp-60h]
    float v15; // [esp+14h] [ebp-5Ch]
    float v16; // [esp+18h] [ebp-58h]
    float v17; // [esp+1Ch] [ebp-54h]
    float v18; // [esp+1Ch] [ebp-54h]
    float v19; // [esp+24h] [ebp-4Ch]
    float v20; // [esp+24h] [ebp-4Ch]
    float v21; // [esp+28h] [ebp-48h]
    float v22; // [esp+2Ch] [ebp-44h]
    float v23; // [esp+2Ch] [ebp-44h]
    float v24; // [esp+30h] [ebp-40h]
    float v25; // [esp+34h] [ebp-3Ch]
    float c; // [esp+44h] [ebp-2Ch]
    float xy[4][2]; // [esp+48h] [ebp-28h] BYREF
    float s; // [esp+68h] [ebp-8h]
    int i; // [esp+6Ch] [ebp-4h]

    v7 = (float)(DEG2RAD(angle));
    c = cos(v7);
    s = sin(v7);
    for ( i = 0; i < 4; ++i )
    {
        v25 = x / ScrPlace_HiResGetScale();
        v24 = v25 * scrPlace->scaleRealToVirtual[0];
        v22 = (*verts)[2 * i];
        v23 = v22 / ScrPlace_HiResGetScale();
        v21 = v23 * scrPlace->scaleRealToVirtual[0];
        v19 = (*verts)[2 * i + 1];
        v20 = v19 / ScrPlace_HiResGetScale();
        v17 = (float)((float)(v21 * c) + v24) - (float)((float)(v20 * scrPlace->scaleRealToVirtual[1]) * s);
        v18 = ScrPlace_HiResGetScale() * v17;
        xy[i][0] = v18 * scrPlace->scaleVirtualToReal[0];
        v16 = y / ScrPlace_HiResGetScale();
        v15 = v16 * scrPlace->scaleRealToVirtual[1];
        v13 = (*verts)[2 * i];
        v14 = v13 / ScrPlace_HiResGetScale();
        v12 = v14 * scrPlace->scaleRealToVirtual[0];
        v10 = (*verts)[2 * i + 1];
        v11 = v10 / ScrPlace_HiResGetScale();
        v8 = (float)((float)(v12 * s) + v15) + (float)((float)(v11 * scrPlace->scaleRealToVirtual[1]) * c);
        v9 = ScrPlace_HiResGetScale() * v8;
        xy[i][1] = v9 * scrPlace->scaleVirtualToReal[1];
    }
    R_AddCmdDrawQuadPic(xy, color, material);
}

const float sign_0[4][2] =
{ { 1.0, -1.0 }, { 1.0, 1.0 }, { -1.0, 1.0 }, { -1.0, -1.0 } };

void __cdecl CG_DrawVLine(
    const ScreenPlacement *scrPlace,
    float x,
    float top,
    float lineWidth,
    float height,
    int horzAlign,
    int vertAlign,
    const float *color,
    Material *material)
{
    float halfWidth; // [esp+0h] [ebp-34h]
    float verts[4][2]; // [esp+4h] [ebp-30h] BYREF
    int i; // [esp+24h] [ebp-10h]
    float halfHeight; // [esp+28h] [ebp-Ch]
    float center[2]; // [esp+2Ch] [ebp-8h]

    ScrPlace_ApplyRect(scrPlace, &x, &top, &lineWidth, &height, horzAlign, vertAlign);
    halfWidth = lineWidth * 0.5;
    halfHeight = height * 0.5;
    center[0] = x;
    center[1] = top + (float)(height * 0.5);
    for (i = 0; i < 4; ++i)
    {
        verts[i][0] = (float)((float)sign_0[i][0] * halfWidth) + center[0];
        verts[i][1] = (float)((float)sign_0[i][1] * halfHeight) + center[1];
    }
    R_AddCmdDrawQuadPic(verts, color, material);
}

double __cdecl Vec2NormalizeTo(const float *v, float *out)
{
    float v3; // [esp+0h] [ebp-10h]
    float length; // [esp+Ch] [ebp-4h]

    length = sqrtf((float)(*v * *v) + (float)(v[1] * v[1]));
    //if ( COERCE_FLOAT(LODWORD(length) ^ _mask__NegFloat_) < 0.0 )
    if ( -(length) < 0.0 )
        v3 = length;
    else
        v3 = 1.0f;
    *out = *v * (float)(1.0 / v3);
    out[1] = v[1] * (float)(1.0 / v3);
    return length;
}

void __cdecl CG_DrawStringExt(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                char *string,
                const float *setColor,
                int forceColor,
                int shadow,
                float charHeight)
{
    Font_s *font; // [esp+1Ch] [ebp-8h]
    float fontScale; // [esp+20h] [ebp-4h]
    float ya; // [esp+34h] [ebp+10h]

    ya = (float)(charHeight * 0.80000001) + y;
    fontScale = charHeight / 48.0;
    if ( !setColor )
        setColor = colorWhite;
    font = UI_GetFontHandle(scrPlace, 5, fontScale);
    UI_DrawText(scrPlace, string, 0x7FFFFFFF, font, x, ya, 1, 1, fontScale, setColor, shadow != 0 ? 3 : 0);
}

int __cdecl CG_DrawDevString(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float xScale,
                float yScale,
                char *s,
                const float *color,
                char align,
                Font_s *font)
{
    int step; // [esp+28h] [ebp-4h]

    if ( (align & 3) != 1
        && (align & 3) != 2
        && (align & 3) != 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_drawtools.cpp",
                    279,
                    0,
                    "%s",
                    "(align & CG_ALIGN_X) == CG_ALIGN_LEFT || (align & CG_ALIGN_X) == CG_ALIGN_RIGHT || (align & CG_ALIGN_X) == CG_ALIGN_CENTER") )
    {
        __debugbreak();
    }
    if ( (align & 3) == 2 )
    {
        x = x - (float)((float)R_TextWidth(s, 0, font) * xScale);
    }
    else if ( (align & 3) == 3 )
    {
        x = x - (float)((float)((float)R_TextWidth(s, 0, font) * xScale) * 0.5);
    }
    if ( (align & 0xC) != 4
        && (align & 0xC) != 8
        && (align & 0xC) != 0xC
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_drawtools.cpp",
                    291,
                    0,
                    "%s",
                    "(align & CG_ALIGN_Y) == CG_ALIGN_TOP || (align & CG_ALIGN_Y) == CG_ALIGN_BOTTOM || (align & CG_ALIGN_Y) == CG_ALIGN_MIDDLE") )
    {
        __debugbreak();
    }
    step = R_TextHeight(font);
    if ( (align & 0xC) == 4 )
    {
        y = y + (float)((float)step * yScale);
    }
    else if ( (align & 0xC) == 0xC )
    {
        y = (float)((float)((float)step * yScale) * 0.5) + y;
    }
    CL_DrawText(scrPlace, s, 0x7FFFFFFF, font, x, y, 1, 1, xScale, yScale, color, 0);
    return step;
}

int __cdecl CG_DrawBigDevString(const ScreenPlacement *scrPlace, float x, float y, char *s, float alpha, char align)
{
    float color[4]; // [esp+14h] [ebp-10h] BYREF

    color[0] = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    color[3] = alpha;
    return CG_DrawBigDevStringColor(scrPlace, x, y, s, color, align);
}

int __cdecl CG_DrawBigDevStringColor(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                char *s,
                const float *color,
                char align)
{
    return CG_DrawDevString(scrPlace, x, y, 1.0, 1.0, s, color, align, cgMedia.bigDevFont);
}

int __cdecl CG_DrawSmallDevStringColor(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                char *s,
                const float *color,
                char align)
{
    return CG_DrawDevString(scrPlace, x, y, 0.56300002, 0.56300002, s, color, align, cgMedia.smallDevFont);
}

double __cdecl CG_FadeAlpha(int timeNow, int startMsec, int totalMsec, int fadeMsec)
{
    int t; // [esp+4h] [ebp-4h]

    t = timeNow - startMsec;
    if ( fadeMsec <= 0 || totalMsec - t >= fadeMsec )
        return 1.0;
    else
        return (double)(totalMsec - t) * 1.0 / (double)fadeMsec;
}

float *__cdecl CG_FadeColor(int timeNow, int startMsec, int totalMsec, int fadeMsec)
{
    if (!startMsec)
        return 0;
    if (timeNow - startMsec >= totalMsec)
        return 0;
    color_2[3] = CG_FadeAlpha(timeNow, startMsec, totalMsec, fadeMsec);
    color_2[2] = 1.0f;
    color_2[1] = 1.0f;
    color_2[0] = 1.0f;
    return color_2;
}

void __cdecl CG_MiniMapChanged(int localClientNum)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    cg_s *cgameGlob; // [esp+4h] [ebp-2Ch]
    const char *string; // [esp+8h] [ebp-28h] BYREF
    const char *material; // [esp+Ch] [ebp-24h]
    float toLR[2]; // [esp+10h] [ebp-20h]
    float south[2]; // [esp+18h] [ebp-18h]
    float east[2]; // [esp+20h] [ebp-10h]
    float lowerRight[2]; // [esp+28h] [ebp-8h]

    string = CL_GetConfigString(1549);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    material = Com_Parse(&string)->token;
    cgameGlob->compassMapMaterial = Material_RegisterHandle((char*)material, 7);
    v1 = Com_Parse(&string)->token;
    cgameGlob->compassMapUpperLeft[0] = atof(v1);
    v2 = Com_Parse(&string)->token;
    cgameGlob->compassMapUpperLeft[1] = atof(v2);
    v3 = Com_Parse(&string)->token;
    lowerRight[0] = atof(v3);
    v4 = Com_Parse(&string)->token;
    lowerRight[1] = atof(v4);
    east[0] = cgameGlob->compassNorth[1];
    east[1] = -cgameGlob->compassNorth[0];
    south[0] = -cgameGlob->compassNorth[0];
    south[1] = -cgameGlob->compassNorth[1];
    toLR[0] = lowerRight[0] - cgameGlob->compassMapUpperLeft[0];
    toLR[1] = lowerRight[1] - cgameGlob->compassMapUpperLeft[1];
    cgameGlob->compassMapWorldSize[0] = (float)(toLR[0] * east[0]) + (float)(toLR[1] * east[1]);
    cgameGlob->compassMapWorldSize[1] = (float)(toLR[0] * south[0]) + (float)(toLR[1] * south[1]);
    if ( cgameGlob->compassMapWorldSize[0] == 0.0 )
        cgameGlob->compassMapWorldSize[0] = 1000.0f;
    if ( cgameGlob->compassMapWorldSize[1] == 0.0 )
        cgameGlob->compassMapWorldSize[1] = 1000.0f;
}

void __cdecl CG_NorthDirectionChanged(int localClientNum)
{
    float v1; // [esp+8h] [ebp-Ch]
    cg_s *cgameGlob; // [esp+Ch] [ebp-8h]
    const char *pszString; // [esp+10h] [ebp-4h]

    pszString = CL_GetConfigString(1548);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgameGlob->compassNorthYaw = atof(pszString);
    v1 = DEG2RAD(cgameGlob->compassNorthYaw);
    cgameGlob->compassNorth[0] = cos(v1);
    cgameGlob->compassNorth[1] = sin(v1);
    CG_MiniMapChanged(localClientNum);
}

void __cdecl CG_DebugBox(
    const float *origin,
    const float *mins,
    const float *maxs,
    float yaw,
    const float *color,
    int depthTest,
    int duration)
{
    long double v7; // st7
    float v8; // [esp+0h] [ebp-9Ch]
    float *v9; // [esp+4h] [ebp-98h]
    unsigned int j; // [esp+1Ch] [ebp-80h]
    float rotated_4; // [esp+24h] [ebp-78h]
    unsigned int i; // [esp+2Ch] [ebp-70h]
    unsigned int ia; // [esp+2Ch] [ebp-70h]
    float fCos; // [esp+30h] [ebp-6Ch]
    float v[25]; // [esp+34h] [ebp-68h] BYREF
    float fSin; // [esp+98h] [ebp-4h]

    v7 = (float)(DEG2RAD(yaw));
    fCos = cos(v7);
    fSin = sin(v7);
    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 3; ++j)
        {
            if ((i & (1 << j)) != 0)
                v8 = maxs[j];
            else
                v8 = mins[j];
            v[3 * i + j] = v8;
        }
        rotated_4 = (float)(v[3 * i] * fSin) + (float)(v[3 * i + 1] * fCos);
        v[3 * i] = (float)(v[3 * i] * fCos) - (float)(v[3 * i + 1] * fSin);
        v[3 * i + 1] = rotated_4;
        v9 = &v[3 * i];
        *v9 = *v9 + *origin;
        v9[1] = v9[1] + origin[1];
        v9[2] = v9[2] + origin[2];
    }
    for (ia = 0; ia < 0xC; ++ia)
        CG_DebugLine(&v[3 * iEdgePairs[ia][0]], &v[3 * iEdgePairs[ia][1]], color, depthTest, duration);
}

void __cdecl CG_DebugBoxOriented(
    const float *origin,
    const float *mins,
    const float *maxs,
    const float (*rotation)[3],
    const float *color,
    int depthTest,
    int duration)
{
    float v7; // [esp+0h] [ebp-84h]
    float *v8; // [esp+4h] [ebp-80h]
    float *v9; // [esp+Ch] [ebp-78h]
    unsigned int j; // [esp+10h] [ebp-74h]
    float rotated[3]; // [esp+14h] [ebp-70h] BYREF
    unsigned int i; // [esp+20h] [ebp-64h]
    float v[8][3]; // [esp+24h] [ebp-60h] BYREF

    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 3; ++j)
        {
            if ((i & (1 << j)) != 0)
                v7 = maxs[j];
            else
                v7 = mins[j];
            v[i][j] = v7;
        }
        MatrixTransformVector(v[i], rotation, rotated);
        v9 = v[i];
        *v9 = rotated[0];
        v9[1] = rotated[1];
        v9[2] = rotated[2];
        v8 = v[i];
        *v8 = *v8 + *origin;
        v8[1] = v8[1] + origin[1];
        v8[2] = v8[2] + origin[2];
    }
    for (i = 0; i < 0xC; ++i)
        CG_DebugLine(v[iEdgePairs[i][0]], v[iEdgePairs[i][1]], color, depthTest, duration);
}

void __cdecl CG_ScoreboardTeamColor(int localClientNum, int team, float *color)
{
    int UIContextIndex; // eax
    UILocalVarContext *LocalVarsContext; // eax
    const char *String; // eax
    const char *v6; // eax
    UILocalVarContext *var; // [esp+8h] [ebp-2Ch]
    const StringTable *tablePtr; // [esp+10h] [ebp-24h] BYREF
    float COLOR_ALLIES[4]; // [esp+14h] [ebp-20h]
    float COLOR_AXIS[4]; // [esp+24h] [ebp-10h]

    COLOR_ALLIES[0] = 0.384f;
    COLOR_ALLIES[1] = 0.725f;
    COLOR_ALLIES[2] = 0.725f;
    COLOR_ALLIES[3] = 1.0f;
    COLOR_AXIS[0] = 0.619f;
    COLOR_AXIS[1] = 0.043f;
    COLOR_AXIS[2] = 0.058f;
    COLOR_AXIS[3] = 1.0f;
    if ( useFastFile->current.enabled )
    {
        UIContextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
        LocalVarsContext = UI_UIContext_GetLocalVarsContext(UIContextIndex);
        var = UILocalVar_Find(LocalVarsContext, "ui_team");
        StringTable_GetAsset("mp/mapsTable.csv", (XAssetHeader *)&tablePtr);
        String = Dvar_GetString("mapname");
        StringTable_Lookup(tablePtr, 0, String, 1);
        switch ( team )
        {
            case 0:
                if ( !var || I_strcmp(var->table[0].u.string, "opfor") )
                {
                    if ( !var || I_strcmp(var->table[0].u.string, "marines") )
                    {
                        Dvar_GetUnpackedColor(cg_ScoresColor_Free, color);
                    }
                    else
                    {
                        *color = COLOR_ALLIES[0];
                        color[1] = COLOR_ALLIES[1];
                        color[2] = COLOR_ALLIES[2];
                        color[3] = COLOR_ALLIES[3];
                    }
                }
                else
                {
                    *color = COLOR_AXIS[0];
                    color[1] = COLOR_AXIS[1];
                    color[2] = COLOR_AXIS[2];
                    color[3] = COLOR_AXIS[3];
                }
                break;
            case 1:
                *color = COLOR_AXIS[0];
                color[1] = COLOR_AXIS[1];
                color[2] = COLOR_AXIS[2];
                color[3] = COLOR_AXIS[3];
                break;
            case 2:
                *color = COLOR_ALLIES[0];
                color[1] = COLOR_ALLIES[1];
                color[2] = COLOR_ALLIES[2];
                color[3] = COLOR_ALLIES[3];
                break;
            case 3:
                Dvar_GetUnpackedColor(cg_ScoresColor_Spectator, color);
                break;
            default:
                v6 = va("Unknown team: %i", team);
                if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_drawtools.cpp", 821, 0, v6) )
                    __debugbreak();
                break;
        }
    }
    else
    {
        Com_Printf(16, "You cannot do table lookups without fastfiles.\n");
        Scr_AddString((char *)"", SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl CG_RelativeTeamColor(int clientNum, float *color, int localClientNum)
{
    cg_s *cgameGlob; // [esp+4h] [ebp-10h]
    int savedAlpha; // [esp+Ch] [ebp-8h]
    int teamIndicator; // [esp+10h] [ebp-4h]

    teamIndicator = CG_GetTeamIndicator();
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_drawtools.cpp",
                    836,
                    0,
                    "cgameGlob->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cgameGlob->clientNum,
                    32) )
    {
        __debugbreak();
    }
    savedAlpha = *((unsigned int *)color + 3);
    if ( cgameGlob->bgs.clientinfo[clientNum].team == TEAM_SPECTATOR )
    {
        Dvar_GetUnpackedColor(cg_TeamColor_Spectator, color);
    }
    else if ( clientNum == cgameGlob->clientNum
                 || cgameGlob->bgs.clientinfo[clientNum].team
                 && cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team == cgameGlob->bgs.clientinfo[clientNum].team )
    {
        if ( teamIndicator == 3 )
            Dvar_GetUnpackedColor(cg_TeamColor_MyTeamAlt, color);
        else
            Dvar_GetUnpackedColor(cg_TeamColor_MyTeam, color);
    }
    else if ( teamIndicator == 3 )
    {
        Dvar_GetUnpackedColor(cg_TeamColor_EnemyTeamAlt, color);
    }
    else
    {
        Dvar_GetUnpackedColor(cg_TeamColor_EnemyTeam, color);
    }
    *((unsigned int *)color + 3) = savedAlpha;
}

void __cdecl CG_XModelDebugBoxes(
                int localClientNum,
                const centity_s *cent,
                const float *color,
                int *partBits,
                int duration)
{
    DObjAnimMat *boneMatrix; // [esp+50h] [ebp-330h]
    unsigned int j; // [esp+54h] [ebp-32Ch]
    XBoneInfo *boneInfoArray[160]; // [esp+58h] [ebp-328h] BYREF
    int numBones; // [esp+2D8h] [ebp-A8h]
    int boneIndex; // [esp+2DCh] [ebp-A4h]
    DObj *obj; // [esp+2E0h] [ebp-A0h]
    float start[3]; // [esp+2E4h] [ebp-9Ch] BYREF
    float end[3]; // [esp+2F0h] [ebp-90h] BYREF
    int size; // [esp+2FCh] [ebp-84h]
    float boneAxis[4][3]; // [esp+300h] [ebp-80h] BYREF
    int localBoneIndex; // [esp+330h] [ebp-50h]
    int (*tempBoxVerts)[3]; // [esp+334h] [ebp-4Ch]
    float org[3]; // [esp+338h] [ebp-48h] BYREF
    XBoneInfo *boneInfo; // [esp+344h] [ebp-3Ch]
    float axis[3][3]; // [esp+348h] [ebp-38h] BYREF
    float vec[3]; // [esp+36Ch] [ebp-14h] BYREF
    int modelCount; // [esp+378h] [ebp-8h]
    int modelIndex; // [esp+37Ch] [ebp-4h]

    obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_drawtools.cpp", 900, 0, "%s", "obj") )
        __debugbreak();
    numBones = DObjNumBones(obj);

    iassert(numBones <= DOBJ_MAX_PARTS);

    DObjGetBoneInfo(obj, boneInfoArray);
    boneMatrix = DObjGetRotTransArray(obj);
    AnglesToAxis(cent->pose.angles, axis);
    modelCount = DObjGetNumModels(obj);
    boneIndex = 0;
    for ( modelIndex = 0; modelIndex < modelCount; ++modelIndex )
    {
        size = XModelNumBones(DObjGetModel(obj, modelIndex));
        if ( DObjIgnoreCollision(obj, modelIndex) )
        {
            boneIndex += size;
            boneMatrix += size;
        }
        else
        {
            localBoneIndex = 0;
            while ( localBoneIndex < size )
            {
                if ( !partBits || (partBits[boneIndex >> 5] & (0x80000000 >> (boneIndex & 0x1F))) != 0 )
                {
                    boneInfo = boneInfoArray[boneIndex];
                    tempBoxVerts = boxVerts_0;
                    ConvertQuatToMat(boneMatrix, boneAxis);
                    boneAxis[3][0] = boneMatrix->trans[0];
                    boneAxis[3][1] = boneMatrix->trans[1];
                    boneAxis[3][2] = boneMatrix->trans[2];
                    for ( j = 0; j < 0xC; ++j )
                    {
                        org[0] = boneInfo->bounds[(*tempBoxVerts)[0]][0];
                        org[1] = boneInfo->bounds[(*tempBoxVerts)[1]][1];
                        org[2] = boneInfo->bounds[(*tempBoxVerts)[2]][2];
                        MatrixTransformVector43(org, boneAxis, vec);
                        start[0] = vec[0];
                        start[1] = vec[1];
                        start[2] = vec[2];
                        org[0] = boneInfo->bounds[(*++tempBoxVerts)[0]][0];
                        org[1] = boneInfo->bounds[(*tempBoxVerts)[1]][1];
                        org[2] = boneInfo->bounds[(*tempBoxVerts)[2]][2];
                        MatrixTransformVector43(org, boneAxis, vec);
                        end[0] = vec[0];
                        end[1] = vec[1];
                        end[2] = vec[2];
                        ++tempBoxVerts;
                        CG_DebugLine(start, end, color, 0, duration);
                    }
                }
                ++localBoneIndex;
                ++boneMatrix;
                ++boneIndex;
            }
        }
    }
}

void __cdecl ConvertQuatToMat(const DObjAnimMat *mat, float (*axis)[3])
{
    float transWeight; // [esp+0h] [ebp-48h]
    float yy; // [esp+18h] [ebp-30h]
    float xy; // [esp+1Ch] [ebp-2Ch]
    float zz; // [esp+20h] [ebp-28h]
    float zw; // [esp+24h] [ebp-24h]
    float scaledQuat; // [esp+28h] [ebp-20h]
    float scaledQuat_4; // [esp+2Ch] [ebp-1Ch]
    float scaledQuat_8; // [esp+30h] [ebp-18h]
    float yw; // [esp+34h] [ebp-14h]
    float xz; // [esp+38h] [ebp-10h]
    float yz; // [esp+3Ch] [ebp-Ch]
    float xx; // [esp+40h] [ebp-8h]
    float xw; // [esp+44h] [ebp-4h]

    if ( ((LODWORD(mat->quat[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(mat->quat[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(mat->quat[2]) & 0x7F800000) == 0x7F800000
         || (LODWORD(mat->quat[3]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h",
                    1501,
                    0,
                    "%s",
                    "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])") )
    {
        __debugbreak();
    }
    if ( (LODWORD(mat->transWeight) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h",
                    1502,
                    0,
                    "%s",
                    "!IS_NAN(mat->transWeight)") )
    {
        __debugbreak();
    }
    transWeight = mat->transWeight;
    scaledQuat = transWeight * mat->quat[0];
    scaledQuat_4 = transWeight * mat->quat[1];
    scaledQuat_8 = transWeight * mat->quat[2];
    xx = scaledQuat * mat->quat[0];
    xy = scaledQuat * mat->quat[1];
    xz = scaledQuat * mat->quat[2];
    xw = scaledQuat * mat->quat[3];
    yy = scaledQuat_4 * mat->quat[1];
    yz = scaledQuat_4 * mat->quat[2];
    yw = scaledQuat_4 * mat->quat[3];
    zz = scaledQuat_8 * mat->quat[2];
    zw = scaledQuat_8 * mat->quat[3];
    (*axis)[0] = 1.0 - (float)(yy + zz);
    (*axis)[1] = xy + zw;
    (*axis)[2] = xz - yw;
    (*axis)[3] = xy - zw;
    (*axis)[4] = 1.0 - (float)(xx + zz);
    (*axis)[5] = yz + xw;
    (*axis)[6] = xz + yw;
    (*axis)[7] = yz - xw;
    (*axis)[8] = 1.0 - (float)(xx + yy);
}

void __cdecl CG_DebugLine(const float *start, const float *end, const float *color, int depthTest, int duration)
{
    CL_AddDebugLine(start, end, color, depthTest, duration);
}

void __cdecl CG_DebugSphere(
                const float *center,
                float radius,
                const float *floatColor,
                int sideCount,
                int depthTest,
                int duration)
{
    CL_AddDebugSphere(center, radius, floatColor, sideCount, depthTest, duration);
}

