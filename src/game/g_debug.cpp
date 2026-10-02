#include "g_debug.h"
#include <client/cl_debugdata.h>
#include <universal/com_math.h>
#include <cgame/cg_drawtools.h>
#include <game_mp/g_main_mp.h>
#include <universal/com_math_anglevectors.h>
#include <qcommon/cm_load.h>
#include <qcommon/cm_showcollision.h>

DebugDrawBrushInfo g_debugDrawBrushInfo;

void __cdecl G_DebugStar(const float *point, const float *color, int duration)
{
    CL_AddDebugStar(point, color, duration);
}

void __cdecl G_DebugCircleEx(
                const float *center,
                float radius,
                const float *dir,
                const float *color,
                int depthTest,
                int duration)
{
    float *v6; // [esp+8h] [ebp-11Ch]
    float *v7; // [esp+14h] [ebp-110h]
    float fAngle; // [esp+30h] [ebp-F4h]
    float fCos; // [esp+34h] [ebp-F0h]
    float fSin; // [esp+38h] [ebp-ECh]
    float normal[3]; // [esp+3Ch] [ebp-E8h] BYREF
    float right[3]; // [esp+48h] [ebp-DCh] BYREF
    float up[3]; // [esp+54h] [ebp-D0h] BYREF
    unsigned int i; // [esp+60h] [ebp-C4h]
    float v[16][3]; // [esp+64h] [ebp-C0h] BYREF

    Vec3NormalizeTo(dir, normal);
    PerpendicularVector(normal, right);
    Vec3Cross(normal, right, up);
    for ( i = 0; i < 0x10; ++i )
    {
        fAngle = (double)i * 0.39269909;
        fCos = cos(fAngle);
        fSin = sin(fAngle);
        v7 = v[i];
        *v7 = (float)((float)(fSin * radius) * up[0]) + *center;
        v7[1] = (float)((float)(fSin * radius) * up[1]) + center[1];
        v7[2] = (float)((float)(fSin * radius) * up[2]) + center[2];
        v6 = v[i];
        *v6 = (float)((float)(fCos * radius) * right[0]) + *v6;
        v6[1] = (float)((float)(fCos * radius) * right[1]) + v6[1];
        v6[2] = (float)((float)(fCos * radius) * right[2]) + v6[2];
    }
    for ( i = 0; i < 0x10; ++i )
        CG_DebugLine(v[i], v[(i + 1) % 0x10], color, depthTest, duration);
}

void __cdecl G_DebugLine(const float *start, const float *end, const float *color, int depthTest)
{
    CL_AddDebugLine(start, end, color, depthTest, 0);
}

void __cdecl G_DebugCircle(
                const float *center,
                float radius,
                const float *color,
                int depthTest,
                int onGround,
                int duration)
{
    float eyepos_4; // [esp+1Ch] [ebp-14h]
    float eyepos_8; // [esp+20h] [ebp-10h]
    float dir[3]; // [esp+24h] [ebp-Ch] BYREF

    if ( onGround )
    {
        dir[0] = 0.0f;
        dir[1] = 0.0f;
        dir[2] = 1.0f;
    }
    else
    {
        eyepos_4 = level.clients->ps.origin[1];
        eyepos_8 = level.clients->ps.origin[2] + level.clients->ps.viewHeightCurrent;
        dir[0] = *center - level.clients->ps.origin[0];
        dir[1] = center[1] - eyepos_4;
        dir[2] = center[2] - eyepos_8;
    }
    G_DebugCircleEx(center, radius, dir, color, depthTest, duration);
}

void __cdecl G_DebugArc(
                const float *center,
                float radius,
                float angle0,
                float angle1,
                const float *color,
                int depthTest,
                int duration)
{
    float fAngle; // [esp+10h] [ebp-D4h]
    float fCos; // [esp+14h] [ebp-D0h]
    float fSin; // [esp+18h] [ebp-CCh]
    unsigned int i; // [esp+1Ch] [ebp-C8h]
    unsigned int ia; // [esp+1Ch] [ebp-C8h]
    float scale; // [esp+20h] [ebp-C4h]
    float v[16][3]; // [esp+24h] [ebp-C0h] BYREF

    scale = (float)(angle1 - angle0) / 15.0;
    if ( scale < 0.0 )
    {
        angle0 = angle0 - 360.0;
        scale = (float)(angle1 - angle0) / 15.0;
    }
    for ( i = 0; i < 0x10; ++i )
    {
        fAngle = DEG2RAD((double)i * scale + angle0);
        fCos = cos(fAngle);
        fSin = sin(fAngle);
        v[i][0] = (float)(fCos * radius) + *center;
        v[i][1] = (float)(fSin * radius) + center[1];
        v[i][2] = center[2];
    }
    for ( ia = 0; ia < 0xF; ++ia )
        CG_DebugLine(v[ia], v[ia + 1], color, depthTest, duration);
}

void __cdecl G_DebugDrawBrushModel(gentity_s *entity, const float *color, int depthTest, int duration)
{
    cmodel_t *cmod; // [esp+8h] [ebp-8h]
    unsigned int brushModel; // [esp+Ch] [ebp-4h]

    if ( !entity && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_debug.cpp", 309, 0, "%s", "entity") )
        __debugbreak();
    brushModel = entity->s.index.brushmodel;
    if ( entity->s.index.brushmodel )
    {
        g_debugDrawBrushInfo.depthTest = depthTest;
        g_debugDrawBrushInfo.duration = duration;
        AnglesToAxis(entity->r.currentAngles, g_debugDrawBrushInfo.transform);
        g_debugDrawBrushInfo.transform[3][0] = entity->r.currentOrigin[0];
        g_debugDrawBrushInfo.transform[3][1] = entity->r.currentOrigin[1];
        g_debugDrawBrushInfo.transform[3][2] = entity->r.currentOrigin[2];
        cmod = CM_ClipHandleToModel(brushModel);
        if ( !cmod && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_debug.cpp", 323, 0, "%s", "cmod") )
            __debugbreak();
        G_DebugDrawBrush_r(&cm.leafbrushNodes[cmod->leaf.leafBrushNode], color);
    }
}

void __cdecl G_DebugDrawBrush_r(cLeafBrushNode_s *node, const float *color)
{
    int k; // [esp+4h] [ebp-Ch]
    unsigned __int16 *brushes; // [esp+8h] [ebp-8h]

    if ( !node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_debug.cpp", 283, 0, "%s", "node") )
        __debugbreak();
    if ( node->leafBrushCount <= 0 )
    {
        G_DebugDrawBrush_r(node + 1, color);
    }
    else
    {
        brushes = node->data.leaf.brushes;
        for ( k = 0; k < node->leafBrushCount; ++k )
            CM_ShowSingleBrushCollision(&cm.brushes[brushes[k]], color, DrawBrushPoly);
    }
}

void __cdecl DrawBrushPoly(int numPoints, float (*points)[3], const float *color)
{
    int i; // [esp+0h] [ebp-1Ch]
    float start[3]; // [esp+4h] [ebp-18h] BYREF
    float end[3]; // [esp+10h] [ebp-Ch] BYREF

    MatrixTransformVector43((const float *)points, g_debugDrawBrushInfo.transform, start);
    for ( i = 0; i < numPoints - 1; ++i )
    {
        MatrixTransformVector43(&(*points)[3 * i + 3], g_debugDrawBrushInfo.transform, end);
        CG_DebugLine(start, end, color, g_debugDrawBrushInfo.depthTest, g_debugDrawBrushInfo.duration);
        start[0] = end[0];
        start[1] = end[1];
        start[2] = end[2];
    }
    MatrixTransformVector43((const float *)points, g_debugDrawBrushInfo.transform, end);
    CG_DebugLine(start, end, color, g_debugDrawBrushInfo.depthTest, g_debugDrawBrushInfo.duration);
}

