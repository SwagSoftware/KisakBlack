#include "r_utils.h"
#include <universal/com_memory.h>
#include "r_dpvs.h"
#include <EffectsCore/fx_convert.h>
#include <cgame/cg_world.h>
#include "r_debug.h"
#include "r_dvars.h"
#include "rb_backend.h"
#include <win32/win_shared.h>

unsigned int __cdecl R_HashAssetName(const char *name)
{
    const char *pos; // [esp+0h] [ebp-8h]
    unsigned int hash; // [esp+4h] [ebp-4h]

    hash = 0;
    for ( pos = name; *pos; ++pos )
    {
        if ( *pos >= 65
            && *pos <= 90
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_utils.cpp",
                        59,
                        0,
                        "%s\n\t(name) = %s",
                        "(*pos < 'A' || *pos > 'Z')",
                        name) )
        {
            __debugbreak();
        }
        if ( *pos == 92
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_utils.cpp",
                        60,
                        0,
                        "%s\n\t(name) = %s",
                        "(*pos != '\\\\' || *pos == '/')",
                        name) )
        {
            __debugbreak();
        }
        hash = *pos ^ (33 * hash);
    }
    return hash;
}

unsigned int __cdecl R_HashString(const char *string)
{
    unsigned int hash; // [esp+4h] [ebp-4h]

    hash = 0;
    while ( *string )
        hash = (*string++ | 0x20) ^ (33 * hash);
    return hash;
}

char *__cdecl R_AllocGlobalVariable(unsigned int bytes, const char *name)
{
    return Z_VirtualAlloc(bytes, name, 19);
}

void __cdecl R_FreeGlobalVariable(void *var)
{
    Z_VirtualFree(var, 19);
}

char __cdecl R_CullPointAndRadius(const float *pt, float radius, const DpvsPlane *clipPlanes, int clipPlaneCount)
{
    int planeIndex; // [esp+8h] [ebp-4h]

    for ( planeIndex = 0; planeIndex < clipPlaneCount; ++planeIndex )
    {
        //if ( COERCE_FLOAT(LODWORD(radius) ^ _mask__NegFloat_) > (float)((float)((float)((float)(*pt * clipPlanes[planeIndex].coeffs[0]) + (float)(pt[1] * clipPlanes[planeIndex].coeffs[1])) + (float)(pt[2] * clipPlanes[planeIndex].coeffs[2])) + clipPlanes[planeIndex].coeffs[3]) )
        if ( (-(radius)) > (float)((float)((float)((float)(*pt * clipPlanes[planeIndex].coeffs[0]) + (float)(pt[1] * clipPlanes[planeIndex].coeffs[1])) + (float)(pt[2] * clipPlanes[planeIndex].coeffs[2])) + clipPlanes[planeIndex].coeffs[3]) )
            return 1;
    }
    return 0;
}

void __cdecl R_ConvertColorToBytes(const float *colorFloat, unsigned __int8 *colorBytes)
{
    if ( colorFloat )
        Byte4PackVertexColor(colorFloat, colorBytes);
    else
        *(unsigned int *)colorBytes = -1;
}

int __cdecl R_PickEntityBone(int traceMask, const float *org, const float *dir, int boneIndex, char **boneName)
{
    float xend[3]; // [esp+8h] [ebp-A0h] BYREF
    float bonePos[3]; // [esp+14h] [ebp-94h] BYREF
    float zend[3]; // [esp+20h] [ebp-88h] BYREF
    float end[3]; // [esp+2Ch] [ebp-7Ch] BYREF
    float boneAxis[3][3]; // [esp+38h] [ebp-70h] BYREF
    trace_t trace; // [esp+5Ch] [ebp-4Ch] BYREF
    float yend[3]; // [esp+98h] [ebp-10h] BYREF
    unsigned __int16 hitEntId; // [esp+A4h] [ebp-4h]

    end[0] = (float)(262144.0 * *dir) + *org;
    end[1] = (float)(262144.0 * dir[1]) + org[1];
    end[2] = (float)(262144.0 * dir[2]) + org[2];
    CG_LocationalTrace(&trace, org, end, 0, traceMask, 0, 0);
    hitEntId = Trace_GetEntityHitId(&trace);
    if ( !CG_GetEntityBoneInfo(hitEntId, boneIndex, bonePos, boneAxis, boneName) )
        return 0;
    xend[0] = (float)(30.0 * boneAxis[0][0]) + bonePos[0];
    xend[1] = (float)(30.0 * boneAxis[0][1]) + bonePos[1];
    xend[2] = (float)(30.0 * boneAxis[0][2]) + bonePos[2];
    yend[0] = (float)(30.0 * boneAxis[1][0]) + bonePos[0];
    yend[1] = (float)(30.0 * boneAxis[1][1]) + bonePos[1];
    yend[2] = (float)(30.0 * boneAxis[1][2]) + bonePos[2];
    zend[0] = (float)(30.0 * boneAxis[2][0]) + bonePos[0];
    zend[1] = (float)(30.0 * boneAxis[2][1]) + bonePos[1];
    zend[2] = (float)(30.0 * boneAxis[2][2]) + bonePos[2];
    R_AddDebugLine(&frontEndDataOut->debugGlobals, bonePos, xend, colorRed, 1);
    R_AddDebugLine(&frontEndDataOut->debugGlobals, bonePos, yend, colorGreen, 1);
    R_AddDebugLine(&frontEndDataOut->debugGlobals, bonePos, zend, colorBlue, 1);
    return 1;
}

//double __cdecl FresnelTerm(float n0, float n1, float cosIncidentAngle)
//{
//    float v3; // xmm0_4
//    long double v5; // [esp+8h] [ebp-6Ch]
//    long double v6; // [esp+8h] [ebp-6Ch]
//    long double v7; // [esp+8h] [ebp-6Ch]
//    long double v8; // [esp+8h] [ebp-6Ch]
//    long double v9; // [esp+8h] [ebp-6Ch]
//    long double v10; // [esp+8h] [ebp-6Ch]
//    long double v11; // [esp+8h] [ebp-6Ch]
//    double tanRatio_4; // [esp+20h] [ebp-54h]
//    double sinRatio_4; // [esp+38h] [ebp-3Ch]
//    double sinSum_4; // [esp+48h] [ebp-2Ch]
//    double sinTransmissionAngle_4; // [esp+58h] [ebp-1Ch]
//
//    if ( cosIncidentAngle < -1.0
//        && !Assert_MyHandler(
//                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_utils.cpp",
//                    347,
//                    1,
//                    "%s\n\t(cosIncidentAngle) = %g",
//                    "(cosIncidentAngle >= -1)",
//                    cosIncidentAngle) )
//    {
//        __debugbreak();
//    }
//    if ( cosIncidentAngle > 1.0
//        && !Assert_MyHandler(
//                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_utils.cpp",
//                    348,
//                    1,
//                    "%s\n\t(cosIncidentAngle) = %g",
//                    "(cosIncidentAngle <= 1)",
//                    cosIncidentAngle) )
//    {
//        __debugbreak();
//    }
//    __libm_sse2_acos(v5);
//    sinTransmissionAngle_4 = fabs(cosIncidentAngle);
//    __libm_sse2_sin(v6);
//    sinSum_4 = (float)(n0 / n1) * sinTransmissionAngle_4;
//    if ( sinSum_4 <= 1.0 )
//    {
//        if ( sinSum_4 < -1.0 )
//            sinSum_4 = -1.0;
//    }
//    else
//    {
//        sinSum_4 = 1.0;
//    }
//    __libm_sse2_asin(v7);
//    __libm_sse2_sin(v8);
//    sinRatio_4 = sinTransmissionAngle_4 + sinSum_4;
//    if ( sinTransmissionAngle_4 + sinSum_4 == 0.0
//        && !Assert_MyHandler(
//                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_utils.cpp",
//                    361,
//                    1,
//                    "%s\n\t(sinSum) = %g",
//                    "(sinSum != 0)",
//                    sinRatio_4) )
//    {
//        __debugbreak();
//    }
//    __libm_sse2_sin(v9);
//    __libm_sse2_tan(v10);
//    tanRatio_4 = sinTransmissionAngle_4 + sinSum_4;
//    if ( sinTransmissionAngle_4 + sinSum_4 == 0.0
//        && !Assert_MyHandler(
//                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_utils.cpp",
//                    367,
//                    1,
//                    "%s\n\t(tanSum) = %g",
//                    "(tanSum != 0)",
//                    tanRatio_4) )
//    {
//        __debugbreak();
//    }
//    __libm_sse2_tan(v11);
//    v3 = ((sinTransmissionAngle_4 - sinSum_4) / sinRatio_4 * ((sinTransmissionAngle_4 - sinSum_4) / sinRatio_4)
//            + (sinTransmissionAngle_4 - sinSum_4) / tanRatio_4 * ((sinTransmissionAngle_4 - sinSum_4) / tanRatio_4))
//         * 0.5;
//    if ( v3 < 0.0 )
//        return 0.0;
//    if ( v3 <= 1.0 )
//        return v3;
//    return 1.0;
//}

// aislop (sse maths)
double FresnelTerm(float n0, float n1, float cosIncidentAngle)
{
    /* domain validation */
    if (cosIncidentAngle < -1.0f || cosIncidentAngle > 1.0f)
    {
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_utils.cpp",
            347,
            1,
            "%s\n\t(cosIncidentAngle) = %g",
            "(cosIncidentAngle >= -1 && cosIncidentAngle <= 1)",
            cosIncidentAngle))
        {
            __debugbreak();
        }
    }

    /* incident angle */
    const float theta_i = acosf(fabsf(cosIncidentAngle));

    /* Snell’s law */
    float sin_theta_t = (n0 / n1) * sinf(theta_i);

    /* clamp for asin */
    if (sin_theta_t > 1.0f) sin_theta_t = 1.0f;
    if (sin_theta_t < -1.0f) sin_theta_t = -1.0f;

    const float theta_t = asinf(sin_theta_t);

    const float sin_sum = sinf(theta_i + theta_t);
    const float sin_diff = sinf(theta_i - theta_t);

    if (sin_sum == 0.0f)
    {
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_utils.cpp",
            361,
            1,
            "%s\n\t(sinSum) = %g",
            "(sinSum != 0)",
            sin_sum))
        {
            __debugbreak();
        }
    }

    const float tan_sum = tanf(theta_i + theta_t);
    const float tan_diff = tanf(theta_i - theta_t);

    if (tan_sum == 0.0f)
    {
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_utils.cpp",
            367,
            1,
            "%s\n\t(tanSum) = %g",
            "(tanSum != 0)",
            tan_sum))
        {
            __debugbreak();
        }
    }

    const float Rs = sin_diff / sin_sum;
    const float Rp = tan_diff / tan_sum;

    float F = 0.5f * (Rs * Rs + Rp * Rp);

    /* clamp result */
    if (F < 0.0f)
        return 0.0;
    if (F > 1.0f)
        return 1.0;

    return F;
}

char __cdecl R_GetClearColor(float *unpackedRgba)
{
    float *color; // [esp+0h] [ebp-4h]

    if ( r_clear->current.integer && (r_clear->current.integer != 1 || developer->current.integer) )
    {
        if ( r_clear->current.integer == 4 && backEndData && backEndData->fogSettings.density != 0.0 )
        {
            color = backEndData->fogSettings.color;
            *unpackedRgba = backEndData->fogSettings.color[0];
            unpackedRgba[1] = color[1];
            unpackedRgba[2] = color[2];
            unpackedRgba[3] = color[3];
            unpackedRgba[3] = 1.0f;
            return 1;
        }
        else if ( r_clear->current.integer == 3 || (Sys_Milliseconds() & 0x200) == 0 )
        {
            Byte4UnpackRgba((const unsigned __int8 *)&r_clearColor->current, unpackedRgba);
            return 1;
        }
        else
        {
            Byte4UnpackRgba((const unsigned __int8 *)&r_clearColor2->current, unpackedRgba);
            return 1;
        }
    }
    else
    {
        *unpackedRgba = 0.0f;
        unpackedRgba[1] = 0.0f;
        unpackedRgba[2] = 0.0f;
        unpackedRgba[3] = 0.0f;
        return 0;
    }
}

