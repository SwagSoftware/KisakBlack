#include "cl_debugdata.h"
#include <qcommon/threads.h>
#include <gfx_d3d/r_debug_alloc.h>
#include <gfx_d3d/r_debug.h>

#include <client_mp/cl_main_mp.h>


thread_local int gTls_isSverThread; // ??

clientDebugStringInfo_t * clStr = &cls.debug.clStrings;
clientDebugSphereInfo_t * clSphere = &cls.debug.clSpheres;
clientDebugLineInfo_t * clLine = &cls.debug.clLines;
clientDebugStringInfo_t * svStrBuff = &cls.debug.svStringsBuffer;
clientDebugStringInfo_t * svStr = &cls.debug.svStrings;
clientDebugSphereInfo_t * svSphereBuff = &cls.debug.svSpheresBuffer;
clientDebugSphereInfo_t * svSphere = &cls.debug.svSpheres;
clientDebugLineInfo_t * svLineBuff = &cls.debug.svLinesBuffer;
clientDebugLineInfo_t * svLine = &cls.debug.svLines;

void __cdecl CL_DebugInitSVThreadVariables()
{
    if ( !Sys_IsServerThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_debugdata.cpp",
                    19,
                    0,
                    "%s\n\t%s",
                    "Sys_IsServerThread()",
                    "CL_DebugInitSVThreadVariables() must be called from server thread") )
    {
        __debugbreak();
    }
    gTls_isSverThread = 1;
}

void __cdecl CL_AddDebugString(const float *xyz, const float *color, float scale, char *text, int duration)
{
    clientDebugStringInfo_t *info; // [esp+10h] [ebp-8h]
    int fromServer; // [esp+14h] [ebp-4h]

    if ( cls.rendererStarted && CreateDebugStringsIfNeeded() )
    {
        //fromServer = *(unsigned int *)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 12);
        fromServer = gTls_isSverThread;
        if ( fromServer )
            info = &cls.debug.svStrings;
        else
            info = &cls.debug.clStrings;
        AddDebugStringInternal(xyz, color, scale, text, duration, info);
        if ( fromServer )
            cls.debug.fromServer = 1;
    }
}

bool __cdecl CreateDebugStringsIfNeeded()
{
    if ( cls.debug.clStrings.strings )
    {
        if ( !cls.debug.svStrings.strings
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_debugdata.cpp",
                        41,
                        0,
                        "%s",
                        "cls.debug.svStrings.strings") )
        {
            __debugbreak();
        }
        return cls.debug.clStrings.durations
                && cls.debug.svStrings.strings
                && cls.debug.svStrings.durations
                && cls.debug.svStringsBuffer.strings;
    }
    else
    {
        if ( cls.debug.svStrings.strings
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_debugdata.cpp",
                        45,
                        0,
                        "%s",
                        "NULL == cls.debug.svStrings.strings") )
        {
            __debugbreak();
        }
        if ( cls.debug.svStringsBuffer.strings
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_debugdata.cpp",
                        46,
                        0,
                        "%s",
                        "NULL == cls.debug.svStringsBuffer.strings") )
        {
            __debugbreak();
        }
        cls.debug.clStrings.max = 1024;
        cls.debug.svStrings.max = 1024;
        cls.debug.svStringsBuffer.max = 1024;
        R_DebugAlloc((void **)&cls.debug.clStrings.strings, 0x20000, "Client Debug Strings");
        R_DebugAlloc((void **)&cls.debug.clStrings.durations, 4 * cls.debug.clStrings.max, "Client Debug Strings");
        cls.debug.clStrings.num = 0;
        R_DebugAlloc((void **)&cls.debug.svStrings.strings, cls.debug.svStrings.max << 7, "Client Debug Strings");
        R_DebugAlloc((void **)&cls.debug.svStrings.durations, 4 * cls.debug.svStrings.max, "Client Debug Strings");
        cls.debug.svStrings.num = 0;
        R_DebugAlloc((void **)&cls.debug.svStringsBuffer.strings, cls.debug.svStrings.max << 7, "Client Debug Strings");
        cls.debug.svStringsBuffer.num = 0;
        return cls.debug.clStrings.strings
                && cls.debug.clStrings.durations
                && cls.debug.svStrings.strings
                && cls.debug.svStrings.durations
                && cls.debug.svStringsBuffer.strings;
    }
}

void __cdecl AddDebugStringInternal(
                const float *xyz,
                const float *color,
                float scale,
                char *text,
                int duration,
                clientDebugStringInfo_t *info)
{
    trDebugString_t *string; // [esp+4h] [ebp-4h]

    if ( info->num + 1 <= info->max )
    {
        string = &info->strings[info->num];
        string->xyz[0] = *xyz;
        string->xyz[1] = xyz[1];
        string->xyz[2] = xyz[2];
        string->color[0] = *color;
        string->color[1] = color[1];
        string->color[2] = color[2];
        string->color[3] = color[3];
        string->scale = scale;
        strncpy(string->text, text, 0x5Fu);
        string->text[95] = 0;
        info->durations[info->num++] = duration;
    }
}

void __cdecl CL_AddDebugSphere(
                const float *center,
                float radius,
                const float *color,
                int sideCount,
                int depthTest,
                int duration)
{
    clientDebugSphereInfo_t *info; // [esp+18h] [ebp-8h]
    int fromServer; // [esp+1Ch] [ebp-4h]

    if ( cls.rendererStarted && CreateDebugSpheresIfNeeded() )
    {
        //fromServer = *(unsigned int *)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 12);
        fromServer = gTls_isSverThread;
        if ( fromServer )
            info = &cls.debug.svSpheres;
        else
            info = &cls.debug.clSpheres;
        AddDebugSphereInternal(center, radius, color, sideCount, depthTest, duration, info);
        if ( fromServer )
            cls.debug.fromServer = 1;
    }
}

bool __cdecl CreateDebugSpheresIfNeeded()
{
    if ( cls.debug.clSpheres.spheres )
    {
        return cls.debug.clSpheres.durations
                && cls.debug.svSpheres.spheres
                && cls.debug.svSpheres.durations
                && cls.debug.svSpheresBuffer.spheres;
    }
    else
    {
        if ( cls.debug.svSpheres.spheres
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_debugdata.cpp",
                        117,
                        0,
                        "%s",
                        "NULL == cls.debug.svSpheres.spheres") )
        {
            __debugbreak();
        }
        if ( cls.debug.svSpheresBuffer.spheres
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_debugdata.cpp",
                        118,
                        0,
                        "%s",
                        "NULL == cls.debug.svSpheresBuffer.spheres") )
        {
            __debugbreak();
        }
        cls.debug.clSpheres.max = 128;
        cls.debug.svSpheres.max = 128;
        cls.debug.svSpheresBuffer.max = 128;
        R_DebugAlloc((void **)&cls.debug.clSpheres.spheres, 5120, "Client Debug Spheres");
        R_DebugAlloc((void **)&cls.debug.clSpheres.durations, 512, "Client Debug Spheres");
        cls.debug.clSpheres.num = 0;
        R_DebugAlloc((void **)&cls.debug.svSpheres.spheres, 5120, "Server Debug Spheres");
        R_DebugAlloc((void **)&cls.debug.svSpheres.durations, 512, "Server Debug Spheres");
        cls.debug.svSpheres.num = 0;
        R_DebugAlloc((void **)&cls.debug.svSpheresBuffer.spheres, 5120, "Server buffer Debug Spheres");
        cls.debug.svSpheresBuffer.num = 0;
        return cls.debug.clSpheres.spheres
                && cls.debug.clSpheres.durations
                && cls.debug.svSpheres.spheres
                && cls.debug.svSpheres.durations
                && cls.debug.svSpheresBuffer.spheres;
    }
}

void __cdecl AddDebugSphereInternal(
                const float *center,
                float radius,
                const float *color,
                int sideCount,
                int depthTest,
                int duration,
                clientDebugSphereInfo_t *info)
{
    trDebugSphere_t *sphere; // [esp+4h] [ebp-4h]

    if ( info->num + 1 <= info->max )
    {
        sphere = &info->spheres[info->num];
        sphere->center[0] = *center;
        sphere->center[1] = center[1];
        sphere->center[2] = center[2];
        sphere->radius = radius;
        sphere->color[0] = *color;
        sphere->color[1] = color[1];
        sphere->color[2] = color[2];
        sphere->color[3] = color[3];
        sphere->depthTest = depthTest;
        sphere->sideCount = sideCount;
        info->durations[info->num++] = duration;
    }
}

void __cdecl CL_AddDebugLine(const float *start, const float *end, const float *color, int depthTest, int duration)
{
    int fromServer; // [esp+4h] [ebp-4h]

    if ( cls.rendererStarted && CreateDebugLinesIfNeeded() )
    {
        //fromServer = *(unsigned int *)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 12);
        fromServer = gTls_isSverThread;
        if ( fromServer )
            AddDebugLineInternal(start, end, color, depthTest, duration, &cls.debug.svLines);
        else
            AddDebugLineInternal(start, end, color, depthTest, duration, &cls.debug.clLines);
        if ( fromServer )
            cls.debug.fromServer = 1;
    }
}

bool __cdecl CreateDebugLinesIfNeeded()
{
    if ( cls.debug.clLines.lines )
    {
        return cls.debug.clLines.durations
                && cls.debug.svLines.lines
                && cls.debug.svLines.durations
                && cls.debug.svLinesBuffer.lines;
    }
    else
    {
        if ( cls.debug.svLines.lines
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_debugdata.cpp",
                        185,
                        0,
                        "%s",
                        "NULL == cls.debug.svLines.lines") )
        {
            __debugbreak();
        }
        if ( cls.debug.svLinesBuffer.lines
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_debugdata.cpp",
                        186,
                        0,
                        "%s",
                        "NULL == cls.debug.svLinesBuffer.lines") )
        {
            __debugbreak();
        }
        cls.debug.clLines.max = 2048;
        cls.debug.svLines.max = 4096;
        cls.debug.svLinesBuffer.max = 4096;
        R_DebugAlloc((void **)&cls.debug.clLines.lines, 90112, "Client Debug Lines");
        R_DebugAlloc((void **)&cls.debug.clLines.durations, 0x2000, "Client Debug Lines");
        cls.debug.clLines.num = 0;
        R_DebugAlloc((void **)&cls.debug.svLines.lines, 180224, "Client Debug Lines");
        R_DebugAlloc((void **)&cls.debug.svLines.durations, 0x4000, "Client Debug Lines");
        cls.debug.svLines.num = 0;
        R_DebugAlloc((void **)&cls.debug.svLinesBuffer.lines, 180224, "Client Debug Lines");
        cls.debug.svLinesBuffer.num = 0;
        return cls.debug.clLines.lines
                && cls.debug.clLines.durations
                && cls.debug.svLines.lines
                && cls.debug.svLines.durations
                && cls.debug.svLinesBuffer.lines;
    }
}

void __cdecl AddDebugLineInternal(
                const float *start,
                const float *end,
                const float *color,
                int depthTest,
                int duration,
                clientDebugLineInfo_t *info)
{
    trDebugLine_t *line; // [esp+8h] [ebp-4h]

    if ( info->num + 1 <= info->max )
    {
        line = &info->lines[info->num];
        line->start[0] = *start;
        line->start[1] = start[1];
        line->start[2] = start[2];
        line->end[0] = *end;
        line->end[1] = end[1];
        line->end[2] = end[2];
        line->color[0] = *color;
        line->color[1] = color[1];
        line->color[2] = color[2];
        line->color[3] = color[3];
        line->depthTest = depthTest;
        info->durations[info->num++] = duration;
    }
}

void __cdecl CL_AddDebugAxis(const float *origin, const float (*axis)[3], float length, int doDepthTest, int duration)
{
    clientDebugLineInfo_t *info; // [esp+8h] [ebp-2Ch]
    float z[3]; // [esp+Ch] [ebp-28h] BYREF
    float x[3]; // [esp+18h] [ebp-1Ch] BYREF
    float y[3]; // [esp+24h] [ebp-10h] BYREF
    int fromServer; // [esp+30h] [ebp-4h]

    if ( cls.rendererStarted && CreateDebugLinesIfNeeded() )
    {
        //fromServer = *(unsigned int *)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 12);
        fromServer = gTls_isSverThread;
        if ( fromServer )
            info = &cls.debug.svLines;
        else
            info = &cls.debug.clLines;
        x[0] = (float)(length * (*axis)[0]) + *origin;
        x[1] = (float)(length * (float)(*axis)[1]) + origin[1];
        x[2] = (float)(length * (float)(*axis)[2]) + origin[2];
        y[0] = (float)(length * (float)(*axis)[3]) + *origin;
        y[1] = (float)(length * (float)(*axis)[4]) + origin[1];
        y[2] = (float)(length * (float)(*axis)[5]) + origin[2];
        z[0] = (float)(length * (float)(*axis)[6]) + *origin;
        z[1] = (float)(length * (float)(*axis)[7]) + origin[1];
        z[2] = (float)(length * (float)(*axis)[8]) + origin[2];
        AddDebugLineInternal(origin, x, colorRed, doDepthTest, duration, info);
        AddDebugLineInternal(origin, y, colorGreen, doDepthTest, duration, info);
        AddDebugLineInternal(origin, z, colorBlue, doDepthTest, duration, info);
        if ( fromServer )
            cls.debug.fromServer = 1;
    }
}

void __cdecl CL_AddDebugStarWithText(
                const float *point,
                const float *starColor,
                const float *textColor,
                char *string,
                float fontsize,
                int duration)
{
    float lineEnd[3]; // [esp+Ch] [ebp-1Ch] BYREF
    float lineStart[3]; // [esp+18h] [ebp-10h] BYREF
    float starsize; // [esp+24h] [ebp-4h]

    starsize = 5.0 * fontsize;
    lineStart[0] = *point;
    lineStart[1] = point[1];
    lineStart[2] = point[2];
    lineEnd[0] = *point;
    lineEnd[1] = point[1];
    lineEnd[2] = point[2];
    lineStart[0] = lineStart[0] + (float)(5.0 * fontsize);
    lineEnd[0] = lineEnd[0] - (float)(5.0 * fontsize);
    CL_AddDebugLine(lineStart, lineEnd, starColor, 0, duration);
    lineStart[0] = lineStart[0] - starsize;
    lineEnd[0] = lineEnd[0] + starsize;
    lineStart[1] = lineStart[1] + starsize;
    lineEnd[1] = lineEnd[1] - starsize;
    CL_AddDebugLine(lineStart, lineEnd, starColor, 0, duration);
    lineStart[1] = lineStart[1] - starsize;
    lineEnd[1] = lineEnd[1] + starsize;
    lineStart[2] = lineStart[2] + starsize;
    lineEnd[2] = lineEnd[2] - starsize;
    CL_AddDebugLine(lineStart, lineEnd, starColor, 0, duration);
    if ( string )
    {
        if ( *string )
            CL_AddDebugString(point, textColor, fontsize, string, duration);
    }
}

const float MYNULLTEXTCOLOR[4] = { 0.0, 0.0, 0.0, 0.0 };
void __cdecl CL_AddDebugStar(const float *point, const float *color, int duration)
{
    CL_AddDebugStarWithText(point, color, MYNULLTEXTCOLOR, 0, 1.0, duration);
}

void __cdecl CL_AddDebugBox(
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

    v7 = (float)(yaw * 0.017453292);
    fCos = cos(v7);
    fSin = sin(v7);
    for ( i = 0; i < 8; ++i )
    {
        for ( j = 0; j < 3; ++j )
        {
            if ( (i & (1 << j)) != 0 )
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
    for ( ia = 0; ia < 0xC; ++ia )
        CL_AddDebugLine(&v[3 * iEdgePairs[ia][0]], &v[3 * iEdgePairs[ia][1]], color, depthTest, duration);
}

void __cdecl CL_AddDebugBox(
                const float (*axis)[3],
                const float *origin,
                const float *mins,
                const float *maxs,
                const float *color,
                int depthTest,
                int duration)
{
    float v7; // [esp+0h] [ebp-90h]
    float *v8; // [esp+4h] [ebp-8Ch]
    float v9; // [esp+Ch] [ebp-84h]
    float v10; // [esp+14h] [ebp-7Ch]
    float v11; // [esp+18h] [ebp-78h]
    unsigned int j; // [esp+1Ch] [ebp-74h]
    float xformed_4; // [esp+24h] [ebp-6Ch]
    float xformed_8; // [esp+28h] [ebp-68h]
    unsigned int i; // [esp+2Ch] [ebp-64h]
    unsigned int ia; // [esp+2Ch] [ebp-64h]
    float v[8][3]; // [esp+30h] [ebp-60h] BYREF

    for ( i = 0; i < 8; ++i )
    {
        for ( j = 0; j < 3; ++j )
        {
            if ( (i & (1 << j)) != 0 )
                v7 = maxs[j];
            else
                v7 = mins[j];
            v[i][j] = v7;
        }
        v11 = v[i][0];
        v10 = v[i][1];
        v9 = v[i][2];
        xformed_4 = (float)(v9 * (float)(*axis)[7])
                            + (float)((float)(v10 * (float)(*axis)[4]) + (float)((float)(v11 * (float)(*axis)[1]) + origin[1]));
        xformed_8 = (float)(v9 * (float)(*axis)[8])
                            + (float)((float)(v10 * (float)(*axis)[5]) + (float)((float)(v11 * (float)(*axis)[2]) + origin[2]));
        v8 = v[i];
        *v8 = (float)(v9 * (float)(*axis)[6])
                + (float)((float)(v10 * (float)(*axis)[3]) + (float)((float)(v11 * (*axis)[0]) + *origin));
        v8[1] = xformed_4;
        v8[2] = xformed_8;
    }
    for ( ia = 0; ia < 0xC; ++ia )
        CL_AddDebugLine(v[iEdgePairs[ia][0]], v[iEdgePairs[ia][1]], color, depthTest, duration);
}

void __cdecl CL_FlushDebugClientData()
{
    if ( cls.rendererStarted )
    {
        FlushDebugStrings(&cls.debug.clStrings, 0);
        FlushDebugLines(&cls.debug.clLines, 0);
        FlushDebugSpheres(&cls.debug.clSpheres, 0);
    }
}

void __cdecl FlushDebugStrings(clientDebugStringInfo_t *info, int fromServer)
{
    int idx; // [esp+8h] [ebp-4h]

    if ( info->strings )
    {
        idx = 0;
        while ( idx < info->num )
        {
            if ( --info->durations[idx] > 0 )
            {
                if ( fromServer )
                    cls.debug.fromServer = 1;
                ++idx;
            }
            else
            {
                info->durations[idx] = info->durations[--info->num];
                memcpy(&info->strings[idx], &info->strings[info->num], sizeof(info->strings[idx]));
            }
        }
    }
}

void __cdecl FlushDebugLines(clientDebugLineInfo_t *info, int fromServer)
{
    int idx; // [esp+8h] [ebp-4h]

    if ( info->lines )
    {
        idx = 0;
        while ( idx < info->num )
        {
            if ( --info->durations[idx] > 0 )
            {
                if ( fromServer )
                    cls.debug.fromServer = 1;
                ++idx;
            }
            else
            {
                info->durations[idx] = info->durations[--info->num];
                memcpy(&info->lines[idx], &info->lines[info->num], sizeof(info->lines[idx]));
            }
        }
    }
}

void __cdecl FlushDebugSpheres(clientDebugSphereInfo_t *info, int fromServer)
{
    int idx; // [esp+8h] [ebp-4h]

    if ( info->spheres )
    {
        idx = 0;
        while ( idx < info->num )
        {
            if ( --info->durations[idx] > 0 )
            {
                if ( fromServer )
                    cls.debug.fromServer = 1;
                ++idx;
            }
            else
            {
                info->durations[idx] = info->durations[--info->num];
                memcpy(&info->spheres[idx], &info->spheres[info->num], sizeof(info->spheres[idx]));
            }
        }
    }
}

void __cdecl CL_UpdateDebugClientData()
{
    if ( cls.rendererStarted )
    {
        R_CopyDebugStrings(clStr->strings, clStr->num, svStrBuff->strings, svStrBuff->num, svStrBuff->max + clStr->max);
        R_CopyDebugLines(clLine->lines, clLine->num, svLineBuff->lines, svLineBuff->num, svLineBuff->max + clLine->max);
        R_CopyDebugSpheres(
            clSphere->spheres,
            clSphere->num,
            svSphereBuff->spheres,
            svSphereBuff->num,
            svSphereBuff->max + clSphere->max);
    }
}

void __cdecl CL_FlushDebugServerData()
{
    if ( cls.rendererStarted )
    {
        FlushDebugStrings(&cls.debug.svStrings, 1);
        FlushDebugLines(&cls.debug.svLines, 1);
        FlushDebugSpheres(&cls.debug.svSpheres, 1);
    }
}

void __cdecl CL_UpdateDebugServerData()
{
    int copySize; // [esp+0h] [ebp-4h]
    int copySizea; // [esp+0h] [ebp-4h]
    int copySizeb; // [esp+0h] [ebp-4h]

    if ( cls.rendererStarted )
    {
        copySize = svStr->num << 7;
        svStrBuff->num = svStr->num;
        memcpy((unsigned __int8 *)svStrBuff->strings, (unsigned __int8 *)svStr->strings, copySize);
        copySizea = 44 * svLine->num;
        svLineBuff->num = svLine->num;
        memcpy((unsigned __int8 *)svLineBuff->lines, (unsigned __int8 *)svLine->lines, copySizea);
        copySizeb = 40 * svSphere->num;
        svSphereBuff->num = svSphere->num;
        memcpy((unsigned __int8 *)svSphereBuff->spheres, (unsigned __int8 *)svSphere->spheres, copySizeb);
    }
}

void __cdecl CL_ShutdownDebugData()
{
    R_DebugFree((void **)&cls.debug.clSpheres.spheres);
    R_DebugFree((void **)&cls.debug.clSpheres.durations);
    R_DebugFree((void **)&cls.debug.svSpheres.spheres);
    R_DebugFree((void **)&cls.debug.svSpheres.durations);
    R_DebugFree((void **)&cls.debug.svSpheresBuffer.spheres);
    R_DebugFree((void **)&cls.debug.svSpheresBuffer.durations);
    R_DebugFree((void **)&cls.debug.clLines.lines);
    R_DebugFree((void **)&cls.debug.clLines.durations);
    R_DebugFree((void **)&cls.debug.svLines.lines);
    R_DebugFree((void **)&cls.debug.svLines.durations);
    R_DebugFree((void **)&cls.debug.svLinesBuffer.lines);
    R_DebugFree((void **)&cls.debug.svLinesBuffer.durations);
    R_DebugFree((void **)&cls.debug.clStrings.strings);
    R_DebugFree((void **)&cls.debug.clStrings.durations);
    R_DebugFree((void **)&cls.debug.svStrings.strings);
    R_DebugFree((void **)&cls.debug.svStrings.durations);
    R_DebugFree((void **)&cls.debug.svStringsBuffer.strings);
    R_DebugFree((void **)&cls.debug.svStringsBuffer.durations);
    memset(&cls.debug, 0, sizeof(cls.debug));
    R_ShutdownDebug();
    //BLOPS_NULLSUB();
}

