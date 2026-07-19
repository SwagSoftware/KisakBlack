#include "com_math.h"

#include <stdlib.h>
#include "assertive.h"
#include <cmath>
#include <cstring>
#include "q_shared.h"
#include "com_math_anglevectors.h"
#include <qcommon/common.h>

float bytedirs[162][3] =
{
  { -0.52573103, 0.0, 0.85065103 },
  { -0.44286299, 0.238856, 0.86418802 },
  { -0.29524201, 0.0, 0.955423 },
  { -0.309017, 0.5, 0.809017 },
  { -0.16246, 0.26286599, 0.951056 },
  { 0.0, 0.0, 1.0 },
  { 0.0, 0.85065103, 0.52573103 },
  { -0.14762101, 0.71656698, 0.68171799 },
  { 0.14762101, 0.71656698, 0.68171799 },
  { 0.0, 0.52573103, 0.85065103 },
  { 0.309017, 0.5, 0.809017 },
  { 0.52573103, 0.0, 0.85065103 },
  { 0.29524201, 0.0, 0.955423 },
  { 0.44286299, 0.238856, 0.86418802 },
  { 0.16246, 0.26286599, 0.951056 },
  { -0.68171799, 0.14762101, 0.71656698 },
  { -0.809017, 0.309017, 0.5 },
  { -0.58778501, 0.42532501, 0.688191 },
  { -0.85065103, 0.52573103, 0.0 },
  { -0.86418802, 0.44286299, 0.238856 },
  { -0.71656698, 0.68171799, 0.14762101 },
  { -0.688191, 0.58778501, 0.42532501 },
  { -0.5, 0.809017, 0.309017 },
  { -0.238856, 0.86418802, 0.44286299 },
  { -0.42532501, 0.688191, 0.58778501 },
  { -0.71656698, 0.68171799, -0.14762101 },
  { -0.5, 0.809017, -0.309017 },
  { -0.52573103, 0.85065103, 0.0 },
  { 0.0, 0.85065103, -0.52573103 },
  { -0.238856, 0.86418802, -0.44286299 },
  { 0.0, 0.955423, -0.29524201 },
  { -0.26286599, 0.951056, -0.16246 },
  { 0.0, 1.0, 0.0 },
  { 0.0, 0.955423, 0.29524201 },
  { -0.26286599, 0.951056, 0.16246 },
  { 0.238856, 0.86418802, 0.44286299 },
  { 0.26286599, 0.951056, 0.16246 },
  { 0.5, 0.809017, 0.309017 },
  { 0.238856, 0.86418802, -0.44286299 },
  { 0.26286599, 0.951056, -0.16246 },
  { 0.5, 0.809017, -0.309017 },
  { 0.85065103, 0.52573103, 0.0 },
  { 0.71656698, 0.68171799, 0.14762101 },
  { 0.71656698, 0.68171799, -0.14762101 },
  { 0.52573103, 0.85065103, 0.0 },
  { 0.42532501, 0.688191, 0.58778501 },
  { 0.86418802, 0.44286299, 0.238856 },
  { 0.688191, 0.58778501, 0.42532501 },
  { 0.809017, 0.309017, 0.5 },
  { 0.68171799, 0.14762101, 0.71656698 },
  { 0.58778501, 0.42532501, 0.688191 },
  { 0.955423, 0.29524201, 0.0 },
  { 1.0, 0.0, 0.0 },
  { 0.951056, 0.16246, 0.26286599 },
  { 0.85065103, -0.52573103, 0.0 },
  { 0.955423, -0.29524201, 0.0 },
  { 0.86418802, -0.44286299, 0.238856 },
  { 0.951056, -0.16246, 0.26286599 },
  { 0.809017, -0.309017, 0.5 },
  { 0.68171799, -0.14762101, 0.71656698 },
  { 0.85065103, 0.0, 0.52573103 },
  { 0.86418802, 0.44286299, -0.238856 },
  { 0.809017, 0.309017, -0.5 },
  { 0.951056, 0.16246, -0.26286599 },
  { 0.52573103, 0.0, -0.85065103 },
  { 0.68171799, 0.14762101, -0.71656698 },
  { 0.68171799, -0.14762101, -0.71656698 },
  { 0.85065103, 0.0, -0.52573103 },
  { 0.809017, -0.309017, -0.5 },
  { 0.86418802, -0.44286299, -0.238856 },
  { 0.951056, -0.16246, -0.26286599 },
  { 0.14762101, 0.71656698, -0.68171799 },
  { 0.309017, 0.5, -0.809017 },
  { 0.42532501, 0.688191, -0.58778501 },
  { 0.44286299, 0.238856, -0.86418802 },
  { 0.58778501, 0.42532501, -0.688191 },
  { 0.688191, 0.58778501, -0.42532501 },
  { -0.14762101, 0.71656698, -0.68171799 },
  { -0.309017, 0.5, -0.809017 },
  { 0.0, 0.52573103, -0.85065103 },
  { -0.52573103, 0.0, -0.85065103 },
  { -0.44286299, 0.238856, -0.86418802 },
  { -0.29524201, 0.0, -0.955423 },
  { -0.16246, 0.26286599, -0.951056 },
  { 0.0, 0.0, -1.0 },
  { 0.29524201, 0.0, -0.955423 },
  { 0.16246, 0.26286599, -0.951056 },
  { -0.44286299, -0.238856, -0.86418802 },
  { -0.309017, -0.5, -0.809017 },
  { -0.16246, -0.26286599, -0.951056 },
  { 0.0, -0.85065103, -0.52573103 },
  { -0.14762101, -0.71656698, -0.68171799 },
  { 0.14762101, -0.71656698, -0.68171799 },
  { 0.0, -0.52573103, -0.85065103 },
  { 0.309017, -0.5, -0.809017 },
  { 0.44286299, -0.238856, -0.86418802 },
  { 0.16246, -0.26286599, -0.951056 },
  { 0.238856, -0.86418802, -0.44286299 },
  { 0.5, -0.809017, -0.309017 },
  { 0.42532501, -0.688191, -0.58778501 },
  { 0.71656698, -0.68171799, -0.14762101 },
  { 0.688191, -0.58778501, -0.42532501 },
  { 0.58778501, -0.42532501, -0.688191 },
  { 0.0, -0.955423, -0.29524201 },
  { 0.0, -1.0, 0.0 },
  { 0.26286599, -0.951056, -0.16246 },
  { 0.0, -0.85065103, 0.52573103 },
  { 0.0, -0.955423, 0.29524201 },
  { 0.238856, -0.86418802, 0.44286299 },
  { 0.26286599, -0.951056, 0.16246 },
  { 0.5, -0.809017, 0.309017 },
  { 0.71656698, -0.68171799, 0.14762101 },
  { 0.52573103, -0.85065103, 0.0 },
  { -0.238856, -0.86418802, -0.44286299 },
  { -0.5, -0.809017, -0.309017 },
  { -0.26286599, -0.951056, -0.16246 },
  { -0.85065103, -0.52573103, 0.0 },
  { -0.71656698, -0.68171799, -0.14762101 },
  { -0.71656698, -0.68171799, 0.14762101 },
  { -0.52573103, -0.85065103, 0.0 },
  { -0.5, -0.809017, 0.309017 },
  { -0.238856, -0.86418802, 0.44286299 },
  { -0.26286599, -0.951056, 0.16246 },
  { -0.86418802, -0.44286299, 0.238856 },
  { -0.809017, -0.309017, 0.5 },
  { -0.688191, -0.58778501, 0.42532501 },
  { -0.68171799, -0.14762101, 0.71656698 },
  { -0.44286299, -0.238856, 0.86418802 },
  { -0.58778501, -0.42532501, 0.688191 },
  { -0.309017, -0.5, 0.809017 },
  { -0.14762101, -0.71656698, 0.68171799 },
  { -0.42532501, -0.688191, 0.58778501 },
  { -0.16246, -0.26286599, 0.951056 },
  { 0.44286299, -0.238856, 0.86418802 },
  { 0.16246, -0.26286599, 0.951056 },
  { 0.309017, -0.5, 0.809017 },
  { 0.14762101, -0.71656698, 0.68171799 },
  { 0.0, -0.52573103, 0.85065103 },
  { 0.42532501, -0.688191, 0.58778501 },
  { 0.58778501, -0.42532501, 0.688191 },
  { 0.688191, -0.58778501, 0.42532501 },
  { -0.955423, 0.29524201, 0.0 },
  { -0.951056, 0.16246, 0.26286599 },
  { -1.0, 0.0, 0.0 },
  { -0.85065103, 0.0, 0.52573103 },
  { -0.955423, -0.29524201, 0.0 },
  { -0.951056, -0.16246, 0.26286599 },
  { -0.86418802, 0.44286299, -0.238856 },
  { -0.951056, 0.16246, -0.26286599 },
  { -0.809017, 0.309017, -0.5 },
  { -0.86418802, -0.44286299, -0.238856 },
  { -0.951056, -0.16246, -0.26286599 },
  { -0.809017, -0.309017, -0.5 },
  { -0.68171799, 0.14762101, -0.71656698 },
  { -0.68171799, -0.14762101, -0.71656698 },
  { -0.85065103, 0.0, -0.52573103 },
  { -0.688191, 0.58778501, -0.42532501 },
  { -0.58778501, 0.42532501, -0.688191 },
  { -0.42532501, 0.688191, -0.58778501 },
  { -0.42532501, -0.688191, -0.58778501 },
  { -0.58778501, -0.42532501, -0.688191 },
  { -0.688191, -0.58778501, -0.42532501 }
};

float bradfordMA[4][4] =
{
  { 0.8951, -0.75019997, 0.038899999, 0.0 },
  { 0.26640001, 1.7135, -0.068499997, 0.0 },
  { -0.16140001, 0.036699999, 1.0296, 0.0 },
  { 0.0, 0.0, 0.0, 1.0 }
};

float sRGBtoXYZ[4][4] =
{
  { 0.41240001, 0.21259999, 0.019300001, 0.0 },
  { 0.3576, 0.71520001, 0.1192, 0.0 },
  { 0.1805, 0.0722, 0.95050001, 0.0 },
  { 0.0, 0.0, 0.0, 1.0 }
};

float XYZtosRGB[4][4] =
{
  { 3.24071, -0.96925801, 0.055635199, 0.0 },
  { -1.5372601, 1.87599, -0.203996, 0.0 },
  { -0.49857101, 0.041555699, 1.05707, 0.0 },
  { 0.0, 0.0, 0.0, 1.0 }
};

float bradfordMI[4][4] =
{
  { 0.98699301, 0.43230501, -0.008529, 0.0 },
  { -0.147054, 0.51836002, 0.040043, 0.0 },
  { 0.159963, 0.049291, 0.96848702, 0.0 },
  { 0.0, 0.0, 0.0, 1.0 }
};



double __cdecl Com_Random()
{
    return (double)rand() / 32768.0;
}

double __cdecl Com_CRandom()
{
    return Com_Random() * 2.0 - 1.0;
}

void __cdecl GaussianRandom(float *f0, float *f1)
{
        float v2; // [esp+0h] [ebp-1Ch]
        float v3; // [esp+4h] [ebp-18h]
        float v4; // [esp+8h] [ebp-14h]
        float x; // [esp+10h] [ebp-Ch]
        float y; // [esp+14h] [ebp-8h]
        float w; // [esp+18h] [ebp-4h]

        iassert(f0);
        iassert(f1);

        do
        {
                x = Com_CRandom();
                y = Com_CRandom();
                w = x * x + y * y;
        } while (w > 1.0);
        v4 = log(w);
        v3 = v4 * -2.0 / w;
        v2 = sqrt(v3);
        *f0 = x * v2;
        *f1 = y * v2;
}

unsigned int __cdecl RandWithSeed(int *seed)
{
    *seed = 1103515245 * *seed + 12345;
    return *seed / 0x10000 % 0x8000u;
}

double __cdecl randomhash(int value)
{
    return (double)(int)RandWithSeed(&value) / 32768.0;
}

void __cdecl PointInCircleFromUniformDeviates(float radiusDeviate, float yawDeviate, float *point)
{
    long double v3; // st7
    float r; // [esp+8h] [ebp-14h]
    float sinYaw; // [esp+14h] [ebp-8h]
    float cosYaw; // [esp+18h] [ebp-4h]

    r = sqrtf(radiusDeviate);
    v3 = (float)(6.2831855 * yawDeviate);
    cosYaw = cos(v3);
    sinYaw = sin(v3);
    *point = r * cosYaw;
    point[1] = r * sinYaw;
}

double __cdecl LinearTrack(float tgt, float cur, float rate, float deltaTime)
{
    float err; // [esp+4h] [ebp-8h]
    float step; // [esp+8h] [ebp-4h]

    err = tgt - cur;
    if ((float)(tgt - cur) <= 0.0)
    {
        //step = COERCE_FLOAT(LODWORD(rate) ^ _mask__NegFloat_) * deltaTime;
        step = -rate * deltaTime;
    }
    else
    {
        step = rate * deltaTime;
    }

    if ( fabs(err) <= 0.001 )
        return tgt;

    if ( fabs(step) <= fabs(err) )
        return cur + step;

    return tgt;
}

double __cdecl LinearTrackAngle(float tgt, float cur, float rate, float deltaTime)
{
    float angle; // [esp+14h] [ebp-4h]

    while ( (float)(tgt - cur) > 180.0 )
        tgt = tgt - 360.0;
    while ( (float)(tgt - cur) < -180.0 )
        tgt = tgt + 360.0;
    angle = LinearTrack(tgt, cur, rate, deltaTime);
    return AngleNormalize180(angle);
}

double __cdecl DiffTrack(float tgt, float cur, float rate, float deltaTime)
{
    float step; // [esp+8h] [ebp-4h]

    step = (float)(rate * (float)(tgt - cur)) * deltaTime;
    if ( fabs(tgt - cur) <= 0.001 )
        return tgt;
    if ( fabs(step) <= fabs(tgt - cur) )
        return cur + step;
    return tgt;
}

double __cdecl DiffTrackAngle(float tgt, float cur, float rate, float deltaTime)
{
    float angle; // [esp+14h] [ebp-4h]

    while ( (float)(tgt - cur) > 180.0 )
        tgt = tgt - 360.0;
    while ( (float)(tgt - cur) < -180.0 )
        tgt = tgt + 360.0;
    angle = DiffTrack(tgt, cur, rate, deltaTime);
    return AngleNormalize180(angle);
}

double __cdecl GraphGetValueFromFraction(int knotCount, const float (*knots)[2], float fraction)
{
    float result; // [esp+Ch] [ebp-Ch]
    float adjustedFrac; // [esp+10h] [ebp-8h]
    int knotIndex; // [esp+14h] [ebp-4h]

    result = -1.0f;
    if ( !knots && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 467, 0, "%s", "knots") )
        __debugbreak();
    if ( knotCount < 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp",
                    468,
                    0,
                    "%s\n\t(knotCount) = %i",
                    "(knotCount >= 2)",
                    knotCount) )
    {
        __debugbreak();
    }
    if ( (fraction < 0.0 || fraction > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp",
                    469,
                    0,
                    "%s\n\t(fraction) = %g",
                    "(fraction >= 0.0f && fraction <= 1.0f)",
                    fraction) )
    {
        __debugbreak();
    }
    if ( (*knots)[2 * knotCount - 2] != 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp",
                    470,
                    0,
                    "%s\n\t(knots[knotCount - 1][0]) = %g",
                    "(knots[knotCount - 1][0] == 1.0f)",
                    (*knots)[2 * knotCount - 2]) )
    {
        __debugbreak();
    }
    for ( knotIndex = 1; knotIndex < knotCount; ++knotIndex )
    {
        if ( (*knots)[2 * knotIndex] >= fraction )
        {
            adjustedFrac = (float)(fraction - (float)(*knots)[2 * knotIndex - 2])
                                     / (float)((float)(*knots)[2 * knotIndex] - (float)(*knots)[2 * knotIndex - 2]);
            if ( (adjustedFrac < 0.0 || adjustedFrac > 1.0)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp",
                            478,
                            0,
                            "%s\n\t(adjustedFrac) = %g",
                            "(adjustedFrac >= 0.0f && adjustedFrac <= 1.0f)",
                            adjustedFrac) )
            {
                __debugbreak();
            }
            if ( ((*knots)[2 * knotIndex - 1] < 0.0 || (*knots)[2 * knotIndex - 1] > 1.0)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp",
                            479,
                            0,
                            "%s\n\t(knots[knotIndex - 1][1]) = %g",
                            "(knots[knotIndex - 1][1] >= 0.0f && knots[knotIndex - 1][1] <= 1.0f)",
                            (*knots)[2 * knotIndex - 1]) )
            {
                __debugbreak();
            }
            if ( ((*knots)[2 * knotIndex + 1] < 0.0 || (*knots)[2 * knotIndex + 1] > 1.0)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp",
                            480,
                            0,
                            "%s\n\t(knots[knotIndex][1]) = %g",
                            "(knots[knotIndex][1] >= 0.0f && knots[knotIndex][1] <= 1.0f)",
                            (*knots)[2 * knotIndex + 1]) )
            {
                __debugbreak();
            }
            result = (float)((float)((float)(*knots)[2 * knotIndex + 1] - (float)(*knots)[2 * knotIndex - 1]) * adjustedFrac)
                         + (float)(*knots)[2 * knotIndex - 1];
            break;
        }
    }
    if ( (result < 0.0 || result > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp",
                    487,
                    0,
                    "%s\n\t(result) = %g",
                    "(result >= 0.0f && result <= 1.0f)",
                    result) )
    {
        __debugbreak();
    }
    return result;
}

double __cdecl Q_acos(float c)
{
    float v2; // [esp+0h] [ebp-8h]

    v2 = acos(c);
    if (v2 > 3.141592741012573)
        return 3.1415927f;
    if (v2 >= -3.141592741012573)
        return v2;
    return 3.1415927f;
}

char __cdecl ClampChar(int i)
{
    if ( i < -128 )
        return 0x80;
    if ( i <= 127 )
        return i;
    return 127;
}

__int16 __cdecl ClampShort(int i)
{
    if ( i < -32768 )
        return 0x8000;
    if ( i <= 0x7FFF )
        return i;
    return 0x7FFF;
}

unsigned __int8 __cdecl DirToByte(const float *dir)
{
    unsigned __int8 best; // [esp+Ah] [ebp-6h]
    unsigned __int8 i; // [esp+Bh] [ebp-5h]
    float bestd; // [esp+Ch] [ebp-4h]

    if ( !dir )
        return 0;
    bestd = 0.0f;
    best = 0;
    for ( i = 0; i < 0xA2u; ++i )
    {
        if ( (float)((float)((float)(*dir * bytedirs[i][0]) + (float)(dir[1] * bytedirs[i][1]))
                             + (float)(dir[2] * bytedirs[i][2])) > bestd )
        {
            bestd = (float)((float)(*dir * bytedirs[i][0]) + (float)(dir[1] * bytedirs[i][1]))
                        + (float)(dir[2] * bytedirs[i][2]);
            best = i;
        }
    }
    return best;
}

void __cdecl ByteToDir(unsigned int b, float *dir)
{
    float *v2; // ecx

    if ( b < 0xA2 )
    {
        *dir = bytedirs[b][0];
        v2 = bytedirs[b];
        dir[1] = v2[1];
        dir[2] = v2[2];
    }
    else
    {
        *dir = vec3_origin[0];
        dir[1] = 0.0;
        dir[2] = 0.0;
    }
}

int __cdecl VecNCompareCustomEpsilon(const float *v0, const float *v1, float epsilon, int coordCount)
{
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < coordCount; ++i )
    {
        if ( (float)((float)(v0[i] - v1[i]) * (float)(v0[i] - v1[i])) > (float)(epsilon * epsilon) )
            return 0;
    }
    return 1;
}

void __cdecl Vec3ProjectionCoords(const float *dir, int *xCoord, int *yCoord)
{
    float dirSq; // [esp+0h] [ebp-Ch]
    float dirSq_4; // [esp+4h] [ebp-8h]
    float dirSq_8; // [esp+8h] [ebp-4h]

    dirSq = *dir * *dir;
    dirSq_4 = dir[1] * dir[1];
    dirSq_8 = dir[2] * dir[2];
    if ( dirSq_8 < dirSq || dirSq_8 < dirSq_4 )
    {
        if ( dirSq_4 < dirSq || dirSq_4 < dirSq_8 )
        {
            if ( *dir <= 0.0 )
            {
                *xCoord = 2;
                *yCoord = 1;
            }
            else
            {
                *xCoord = 1;
                *yCoord = 2;
            }
        }
        else if ( dir[1] <= 0.0 )
        {
            *xCoord = 0;
            *yCoord = 2;
        }
        else
        {
            *xCoord = 2;
            *yCoord = 0;
        }
    }
    else if ( dir[2] <= 0.0 )
    {
        *xCoord = 1;
        *yCoord = 0;
    }
    else
    {
        *xCoord = 0;
        *yCoord = 1;
    }
}

void __cdecl RotatePointAroundVector(float *dst, const float *dir, const float *point, float degrees)
{
    mat3x3 m; // [esp+1Ch] [ebp-E0h] BYREF
    mat3x3 rot; // [esp+74h] [ebp-88h] BYREF

    mat3x3 tmpmat; // [esp+B4h] [ebp-48h] BYREF
    mat3x3 im; // [esp+D8h] [ebp-24h] BYREF

    float rad; // [esp+40h] [ebp-BCh]
    float vr[3]; // [esp+44h] [ebp-B8h] BYREF


    float vf[3]; // [esp+98h] [ebp-64h] BYREF
    float vup[3]; // [esp+A4h] [ebp-58h] BYREF
    int i; // [esp+B0h] [ebp-4Ch]

    iassert(dir[0] || dir[1] || dir[2]);

    vf[0] = *dir;
    vf[1] = dir[1];
    vf[2] = dir[2];

    PerpendicularVector(dir, vr);
    Vec3Cross(vr, vf, vup);

    m[0][0] = vr[0];
    m[1][0] = vr[1];
    m[2][0] = vr[2];
    m[0][1] = vup[0];
    m[1][1] = vup[1];
    m[2][1] = vup[2];
    m[0][2] = vf[0];
    m[1][2] = vf[1];
    m[2][2] = vf[2];

    memcpy(im, m, sizeof(im));
    im[0][1] = vr[1];
    im[0][2] = vr[2];
    im[1][0] = vup[0];
    im[1][2] = vup[2];
    im[2][0] = vf[0];
    im[2][1] = vf[1];

    mat3x3 zrot{};

    zrot[2][2] = 1.0;
    zrot[1][1] = 1.0;
    zrot[0][0] = 1.0;
    rad = degrees * 0.01745329238474369;
    iassert(!isnan(rad));

    zrot[0][0] = cos(rad);
    zrot[0][1] = sin(rad);

    iassert(!isnan(zrot[0][1]));
    iassert(!isnan(zrot[0][0]));

    zrot[1][0] = -zrot[0][1];
    zrot[1][1] = zrot[0][0];

    MatrixMultiply(m, zrot, tmpmat);
    MatrixMultiply(tmpmat, im, rot);

    for (i = 0; i < 3; ++i)
        dst[i] = rot[i][0] * *point + rot[i][1] * point[1] + rot[i][2] * point[2];
}

void __cdecl Vec3Basis_RightHanded(const float *forward, float *left, float *up)
{
    PerpendicularVector(forward, up);
    Vec3Cross(up, forward, left);
}

float __cdecl vectoyaw(const float *vec)
{
    float v2; // [esp+0h] [ebp-14h]
    float v3; // [esp+4h] [ebp-10h]
    float yawa; // [esp+10h] [ebp-4h]

    if (vec[1] == 0.0 && *vec == 0.0)
    {
        return 0.0;
    }
    else
    {
        v3 = atan2(vec[1], *vec);
        yawa = v3 * 180.0 / 3.141592741012573;
        if (yawa < 0.0)
            v2 = 360.0;
        else
            v2 = 0.0;
        return (yawa + v2);
    }
}

float __cdecl vectosignedyaw(const float *vec)
{
    float v2; // [esp+0h] [ebp-10h]
    float yaw; // [esp+Ch] [ebp-4h]

    if (vec[1] == 0.0 && *vec == 0.0)
    {
        return 0.0;
    }
    else
    {
        v2 = atan2(vec[1], *vec);
        yaw = v2 * 180.0 / 3.141592741012573;
        iassert(yaw >= -180);
        iassert(yaw <= 180);
    }
    return yaw;
}

float __cdecl vectopitch(const float *vec)
{
    float v2; // [esp+0h] [ebp-20h]
    float v3; // [esp+4h] [ebp-1Ch]
    float v4; // [esp+8h] [ebp-18h]
    float v6; // [esp+14h] [ebp-Ch]
    float pitcha; // [esp+1Ch] [ebp-4h]

    if (vec[1] == 0.0 && *vec == 0.0)
    {
        if (-vec[2] < 0.0)
            return 270.0;
        else
            return 90.0;
    }
    else
    {
        v6 = vec[1] * vec[1] + *vec * *vec;
        v4 = sqrt(v6);
        v3 = atan2(vec[2], v4);
        pitcha = v3 * -180.0 / 3.141592741012573;
        if (pitcha < 0.0)
            v2 = 360.0;
        else
            v2 = 0.0;
        return (pitcha + v2);
    }
}

void __cdecl vectoangles(const float *vec, float *angles)
{
    float v2; // [esp+0h] [ebp-34h]
    float v3; // [esp+4h] [ebp-30h]
    float v4; // [esp+8h] [ebp-2Ch]
    float v5; // [esp+Ch] [ebp-28h]
    float v6; // [esp+10h] [ebp-24h]
    float v7; // [esp+14h] [ebp-20h]
    float v8; // [esp+1Ch] [ebp-18h]
    float yaw; // [esp+28h] [ebp-Ch]
    float yawa; // [esp+28h] [ebp-Ch]
    float pitch; // [esp+30h] [ebp-4h]
    float pitcha; // [esp+30h] [ebp-4h]

    if (vec[1] == 0.0 && *vec == 0.0)
    {
        yaw = 0.0;
        if (-vec[2] < 0.0)
            v7 = 270.0;
        else
            v7 = 90.0;
        pitch = v7;
    }
    else
    {
        v6 = atan2(vec[1], *vec);
        yawa = v6 * 180.0 / 3.141592741012573;
        if (yawa < 0.0)
            v5 = 360.0;
        else
            v5 = 0.0;
        yaw = yawa + v5;
        v8 = vec[1] * vec[1] + *vec * *vec;
        v4 = sqrt(v8);
        v3 = atan2(vec[2], v4);
        pitcha = v3 * -180.0 / 3.141592741012573;
        if (pitcha < 0.0)
            v2 = 360.0;
        else
            v2 = 0.0;
        pitch = pitcha + v2;
    }
    *angles = pitch;
    angles[1] = yaw;
    angles[2] = 0.0;
}

void __cdecl UnitQuatToAngles(const float *quat, float *angles)
{
    float axis[3][3]; // [esp+0h] [ebp-24h] BYREF

    UnitQuatToAxis(quat, axis);
    AxisToAngles(axis, angles);
}

void __cdecl YawVectors(float yaw, float *forward, float *right)
{
    float cy; // [esp+8h] [ebp-Ch]
    float angle; // [esp+Ch] [ebp-8h]
    float sy; // [esp+10h] [ebp-4h]

    angle = yaw * 0.01745329238474369;
    cy = cos(angle);
    sy = sin(angle);
    if (forward)
    {
        *forward = cy;
        forward[1] = sy;
        forward[2] = 0.0;
    }
    if (right)
    {
        *right = sy;
        right[1] = -cy;
        right[2] = 0.0;
    }
}

void __cdecl YawVectors2D(float yaw, float *forward, float *right)
{
    float cy; // [esp+8h] [ebp-Ch]
    float angle; // [esp+Ch] [ebp-8h]
    float sy; // [esp+10h] [ebp-4h]

    angle = yaw * 0.01745329238474369;
    cy = cos(angle);
    sy = sin(angle);
    if (forward)
    {
        *forward = cy;
        forward[1] = sy;
    }
    if (right)
    {
        *right = sy;
        right[1] = -cy;
    }
}

void __cdecl PerpendicularVector(const float *src, float *dst)
{
    const char *v2; // eax
    float scale; // [esp+18h] [ebp-34h]
    int pos; // [esp+38h] [ebp-14h]
    float d; // [esp+3Ch] [ebp-10h]
    float srcSq[3]; // [esp+40h] [ebp-Ch]

    iassert(Vec3IsNormalized(src));

    srcSq[0] = *src * *src;
    srcSq[1] = src[1] * src[1];
    srcSq[2] = src[2] * src[2];
    pos = srcSq[0] > srcSq[1];
    if (srcSq[pos] > srcSq[2])
        pos = 2;
    d = -src[pos];
    Vec3Scale(src, d, dst);
    dst[pos] = dst[pos] + 1.0;
    Vec3Normalize(dst);
}

double __cdecl PointToLineDistSq(const float *point, const float *start, const float *end)
{
    float segDot; // [esp+10h] [ebp-20h]
    float seg; // [esp+14h] [ebp-1Ch]
    float seg_4; // [esp+18h] [ebp-18h]
    float seg_8; // [esp+1Ch] [ebp-14h]
    float ptToLine; // [esp+20h] [ebp-10h]
    float ptToLine_4; // [esp+24h] [ebp-Ch]
    float ptToLine_8; // [esp+28h] [ebp-8h]
    float ptToLineDot; // [esp+2Ch] [ebp-4h]

    seg = *end - *start;
    seg_4 = end[1] - start[1];
    seg_8 = end[2] - start[2];
    ptToLine = *point - *start;
    ptToLine_4 = point[1] - start[1];
    ptToLine_8 = point[2] - start[2];
    segDot = (float)((float)(seg * seg) + (float)(seg_4 * seg_4)) + (float)(seg_8 * seg_8);

    iassert(segDot);

    ptToLineDot = (float)((float)(ptToLine * seg) + (float)(ptToLine_4 * seg_4)) + (float)(ptToLine_8 * seg_8);
    return     (float)((float)(-(ptToLineDot / segDot) * seg_8) + ptToLine_8)
             * (float)((float)(-(ptToLineDot / segDot) * seg_8) + ptToLine_8)
             + (float)((float)(-(ptToLineDot / segDot) * seg_4) + ptToLine_4)
             * (float)((float)(-(ptToLineDot / segDot) * seg_4) + ptToLine_4)
             + (float)((float)(-(ptToLineDot / segDot) * seg) + ptToLine)
             * (float)((float)(-(ptToLineDot / segDot) * seg) + ptToLine);
}

float PointToLineDistSq2D(const float *point, const float *start, const float *end)
{
    float dx = end[0] - start[0];
    float dy = end[1] - start[1];

    float px = point[0] - start[0];
    float py = point[1] - start[1];

    float segDot = dx * dx + dy * dy;

    if (segDot == 0.0f)
    {
        iassert(0);
        return 0.0f;
    }

    float proj = -(px * dx + py * dy) / segDot;

    float projX = dx * proj + px;
    float projY = dy * proj + py;

    float distSq = projX * projX + projY * projY;
    return distSq;
}

int __cdecl BoxInPlanes(const float (*planes)[4], unsigned int numPlanes, const float *mins, const float *maxs)
{
    float v5; // [esp+0h] [ebp-2Ch]
    float v6; // [esp+4h] [ebp-28h]
    float v7; // [esp+8h] [ebp-24h]
    float v8; // [esp+Ch] [ebp-20h]
    float v9; // [esp+10h] [ebp-1Ch]
    float v10; // [esp+14h] [ebp-18h]
    float v11; // [esp+18h] [ebp-14h]
    float v12; // [esp+1Ch] [ebp-10h]
    const float *plane; // [esp+20h] [ebp-Ch]
    unsigned int planeIndex; // [esp+28h] [ebp-4h]

    for ( planeIndex = 0; planeIndex < numPlanes; ++planeIndex )
    {
        plane = &(*planes)[4 * planeIndex];
        if ( (float)((float)((float)((float)(*plane * *mins) + (float)(plane[1] * mins[1])) + (float)(plane[2] * mins[2]))
                             + plane[3]) < 0.0 )
            v12 = 1.0f;
        else
            v12 = 0.0f;
        if ( (float)((float)((float)((float)(*plane * *mins) + (float)(plane[1] * mins[1])) + (float)(plane[2] * maxs[2]))
                             + plane[3]) < 0.0 )
            v11 = 1.0f;
        else
            v11 = 0.0f;
        if ( (float)((float)((float)((float)(*plane * *mins) + (float)(plane[1] * maxs[1])) + (float)(plane[2] * mins[2]))
                             + plane[3]) < 0.0 )
            v10 = 1.0f;
        else
            v10 = 0.0f;
        if ( (float)((float)((float)((float)(*plane * *mins) + (float)(plane[1] * maxs[1])) + (float)(plane[2] * maxs[2]))
                             + plane[3]) < 0.0 )
            v9 = 1.0f;
        else
            v9 = 0.0f;
        if ( (float)((float)((float)((float)(*plane * *maxs) + (float)(plane[1] * mins[1])) + (float)(plane[2] * mins[2]))
                             + plane[3]) < 0.0 )
            v8 = 1.0f;
        else
            v8 = 0.0f;
        if ( (float)((float)((float)((float)(*plane * *maxs) + (float)(plane[1] * mins[1])) + (float)(plane[2] * maxs[2]))
                             + plane[3]) < 0.0 )
            v7 = 1.0f;
        else
            v7 = 0.0f;
        if ( (float)((float)((float)((float)(*plane * *maxs) + (float)(plane[1] * maxs[1])) + (float)(plane[2] * mins[2]))
                             + plane[3]) < 0.0 )
            v6 = 1.0f;
        else
            v6 = 0.0f;
        if ( (float)((float)((float)((float)(*plane * *maxs) + (float)(plane[1] * maxs[1])) + (float)(plane[2] * maxs[2]))
                             + plane[3]) < 0.0 )
            v5 = 1.0f;
        else
            v5 = 0.0f;
        if ( (float)((float)((float)((float)((float)((float)((float)((float)(v12 + 0.0) + v11) + v10) + v9) + v8) + v7) + v6)
                             + v5) == 0.0 )
            return 0;
    }
    return 1;
}

int __cdecl SphereInPlanes(const float (*planes)[4], unsigned int numPlanes, const float *center, float radius)
{
    unsigned int planeIndex; // [esp+4h] [ebp-4h]

    for ( planeIndex = 0; planeIndex < numPlanes; ++planeIndex )
    {
        if ( (float)((float)((float)((float)((float)((float)(*planes)[4 * planeIndex] * *center)
                                                                             + (float)((float)(*planes)[4 * planeIndex + 1] * center[1]))
                                                             + (float)((float)(*planes)[4 * planeIndex + 2] * center[2]))
                                             + (float)(*planes)[4 * planeIndex + 3])
                             - radius) > 0.0 )
            return 0;
    }
    return 1;
}

void __cdecl MatrixIdentity33(float (*out)[3])
{
    if ( !out && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 1227, 0, "%s", "out") )
        __debugbreak();
    (*out)[0] = 0.0;
    (*out)[1] = 0.0;
    (*out)[2] = 0.0;
    (*out)[3] = 0.0;
    (*out)[4] = 0.0;
    (*out)[5] = 0.0;
    (*out)[6] = 0.0;
    (*out)[7] = 0.0;
    (*out)[8] = 0.0;
    (*out)[0] = 1.0f;
    (*out)[4] = 1.0f;
    (*out)[8] = 1.0f;
}

void __cdecl MatrixIdentity44(float (*out)[4])
{
    if ( !out && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 1236, 0, "%s", "out") )
        __debugbreak();
    memcpy(out, identityMatrix44, 0x40u);
}

void __cdecl MatrixMultiply(const float (*in1)[3], const float (*in2)[3], float (*out)[3])
{
    (*out)[0] = (float)((float)((*in1)[0] * (*in2)[0]) + (float)((float)(*in1)[1] * (float)(*in2)[3]))
                        + (float)((float)(*in1)[2] * (float)(*in2)[6]);
    (*out)[1] = (float)((float)((*in1)[0] * (float)(*in2)[1]) + (float)((float)(*in1)[1] * (float)(*in2)[4]))
                        + (float)((float)(*in1)[2] * (float)(*in2)[7]);
    (*out)[2] = (float)((float)((*in1)[0] * (float)(*in2)[2]) + (float)((float)(*in1)[1] * (float)(*in2)[5]))
                        + (float)((float)(*in1)[2] * (float)(*in2)[8]);
    (*out)[3] = (float)((float)((float)(*in1)[3] * (*in2)[0]) + (float)((float)(*in1)[4] * (float)(*in2)[3]))
                        + (float)((float)(*in1)[5] * (float)(*in2)[6]);
    (*out)[4] = (float)((float)((float)(*in1)[3] * (float)(*in2)[1]) + (float)((float)(*in1)[4] * (float)(*in2)[4]))
                        + (float)((float)(*in1)[5] * (float)(*in2)[7]);
    (*out)[5] = (float)((float)((float)(*in1)[3] * (float)(*in2)[2]) + (float)((float)(*in1)[4] * (float)(*in2)[5]))
                        + (float)((float)(*in1)[5] * (float)(*in2)[8]);
    (*out)[6] = (float)((float)((float)(*in1)[6] * (*in2)[0]) + (float)((float)(*in1)[7] * (float)(*in2)[3]))
                        + (float)((float)(*in1)[8] * (float)(*in2)[6]);
    (*out)[7] = (float)((float)((float)(*in1)[6] * (float)(*in2)[1]) + (float)((float)(*in1)[7] * (float)(*in2)[4]))
                        + (float)((float)(*in1)[8] * (float)(*in2)[7]);
    (*out)[8] = (float)((float)((float)(*in1)[6] * (float)(*in2)[2]) + (float)((float)(*in1)[7] * (float)(*in2)[5]))
                        + (float)((float)(*in1)[8] * (float)(*in2)[8]);
}

void __cdecl MatrixVecMultiply(const float (*mulMat)[3], const float *mulVec, float *solution)
{
    *solution = (float)((float)((*mulMat)[0] * *mulVec) + (float)((float)(*mulMat)[1] * mulVec[1]))
                        + (float)((float)(*mulMat)[2] * mulVec[2]);
    solution[1] = (float)((float)((float)(*mulMat)[3] * *mulVec) + (float)((float)(*mulMat)[4] * mulVec[1]))
                            + (float)((float)(*mulMat)[5] * mulVec[2]);
    solution[2] = (float)((float)((float)(*mulMat)[6] * *mulVec) + (float)((float)(*mulMat)[7] * mulVec[1]))
                            + (float)((float)(*mulMat)[8] * mulVec[2]);
}

void __cdecl MatrixVecMultiply(const float (*mulMat)[4], const float *mulVec, float *solution)
{
    *solution = (float)((float)((float)((*mulMat)[0] * *mulVec) + (float)((float)(*mulMat)[1] * mulVec[1]))
                                        + (float)((float)(*mulMat)[2] * mulVec[2]))
                        + (float)((float)(*mulMat)[3] * mulVec[3]);
    solution[1] = (float)((float)((float)((float)(*mulMat)[4] * *mulVec) + (float)((float)(*mulMat)[5] * mulVec[1]))
                                            + (float)((float)(*mulMat)[6] * mulVec[2]))
                            + (float)((float)(*mulMat)[7] * mulVec[3]);
    solution[2] = (float)((float)((float)((float)(*mulMat)[8] * *mulVec) + (float)((float)(*mulMat)[9] * mulVec[1]))
                                            + (float)((float)(*mulMat)[10] * mulVec[2]))
                            + (float)((float)(*mulMat)[11] * mulVec[3]);
    solution[3] = (float)((float)((float)((float)(*mulMat)[12] * *mulVec) + (float)((float)(*mulMat)[13] * mulVec[1]))
                                            + (float)((float)(*mulMat)[14] * mulVec[2]))
                            + (float)((float)(*mulMat)[15] * mulVec[3]);
}

void __cdecl MatrixVecMultiplyProject(const float (*mulMat)[4], const float *mulVec, float *solution)
{
    float ret[4]; // [esp+0h] [ebp-24h] BYREF
    float mul[4]; // [esp+10h] [ebp-14h] BYREF
    float invW; // [esp+20h] [ebp-4h]

    mul[0] = *mulVec;
    mul[1] = mulVec[1];
    mul[2] = mulVec[2];
    mul[3] = 1.0f;
    MatrixVecMultiply(mulMat, mul, ret);
    invW = 1.0 / ret[3];
    *solution = ret[0] * (float)(1.0 / ret[3]);
    solution[1] = ret[1] * invW;
    solution[2] = ret[2] * invW;
}

void __cdecl MatrixMultiplyEquals(const float (*in)[3], float (*out)[3])
{
    //__int64 temp; // [esp+8h] [ebp-18h]
    float temp_0;
    float temp_4;
    float temp_8; // [esp+10h] [ebp-10h]
    float temp_12; // [esp+14h] [ebp-Ch]
    float temp_16; // [esp+18h] [ebp-8h]
    float temp_20; // [esp+1Ch] [ebp-4h]

    temp_0 = (float)((float)((*in)[0] * (*out)[0]) + (float)((float)(*in)[1] * (*out)[3]))
                                    + (float)((float)(*in)[2] * (*out)[6]);
    temp_4 = (float)((float)((*in)[0] * (*out)[1]) + (float)((float)(*in)[1] * (*out)[4]))
                                                + (float)((float)(*in)[2] * (*out)[7]);
    temp_8 = (float)((float)((*in)[0] * (*out)[2]) + (float)((float)(*in)[1] * (*out)[5]))
                 + (float)((float)(*in)[2] * (*out)[8]);
    temp_12 = (float)((float)((float)(*in)[3] * (*out)[0]) + (float)((float)(*in)[4] * (*out)[3]))
                    + (float)((float)(*in)[5] * (*out)[6]);
    temp_16 = (float)((float)((float)(*in)[3] * (*out)[1]) + (float)((float)(*in)[4] * (*out)[4]))
                    + (float)((float)(*in)[5] * (*out)[7]);
    temp_20 = (float)((float)((float)(*in)[3] * (*out)[2]) + (float)((float)(*in)[4] * (*out)[5]))
                    + (float)((float)(*in)[5] * (*out)[8]);
    (*out)[6] = (float)((float)((float)(*in)[6] * (*out)[0]) + (float)((float)(*in)[7] * (*out)[3]))
                        + (float)((float)(*in)[8] * (*out)[6]);
    (*out)[7] = (float)((float)((float)(*in)[6] * (*out)[1]) + (float)((float)(*in)[7] * (*out)[4]))
                        + (float)((float)(*in)[8] * (*out)[7]);
    (*out)[8] = (float)((float)((float)(*in)[6] * (*out)[2]) + (float)((float)(*in)[7] * (*out)[5]))
                        + (float)((float)(*in)[8] * (*out)[8]);
    //*(_QWORD *)&(*out)[0] = temp;
    (*out)[0] = temp_0;
    (*out)[1] = temp_4;
    (*out)[2] = temp_8;
    (*out)[3] = temp_12;
    (*out)[4] = temp_16;
    (*out)[5] = temp_20;
}

void __cdecl MatrixMultiply43(const float (*in1)[3], const float (*in2)[3], float (*out)[3])
{
    if ( in1 == out
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 1449, 0, "%s", "in1 != out") )
    {
        __debugbreak();
    }
    if ( in2 == out
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 1450, 0, "%s", "in2 != out") )
    {
        __debugbreak();
    }
    (*out)[0] = (float)((float)((*in1)[0] * (*in2)[0]) + (float)((float)(*in1)[1] * (float)(*in2)[3]))
                        + (float)((float)(*in1)[2] * (float)(*in2)[6]);
    (*out)[3] = (float)((float)((float)(*in1)[3] * (*in2)[0]) + (float)((float)(*in1)[4] * (float)(*in2)[3]))
                        + (float)((float)(*in1)[5] * (float)(*in2)[6]);
    (*out)[6] = (float)((float)((float)(*in1)[6] * (*in2)[0]) + (float)((float)(*in1)[7] * (float)(*in2)[3]))
                        + (float)((float)(*in1)[8] * (float)(*in2)[6]);
    (*out)[1] = (float)((float)((*in1)[0] * (float)(*in2)[1]) + (float)((float)(*in1)[1] * (float)(*in2)[4]))
                        + (float)((float)(*in1)[2] * (float)(*in2)[7]);
    (*out)[4] = (float)((float)((float)(*in1)[3] * (float)(*in2)[1]) + (float)((float)(*in1)[4] * (float)(*in2)[4]))
                        + (float)((float)(*in1)[5] * (float)(*in2)[7]);
    (*out)[7] = (float)((float)((float)(*in1)[6] * (float)(*in2)[1]) + (float)((float)(*in1)[7] * (float)(*in2)[4]))
                        + (float)((float)(*in1)[8] * (float)(*in2)[7]);
    (*out)[2] = (float)((float)((*in1)[0] * (float)(*in2)[2]) + (float)((float)(*in1)[1] * (float)(*in2)[5]))
                        + (float)((float)(*in1)[2] * (float)(*in2)[8]);
    (*out)[5] = (float)((float)((float)(*in1)[3] * (float)(*in2)[2]) + (float)((float)(*in1)[4] * (float)(*in2)[5]))
                        + (float)((float)(*in1)[5] * (float)(*in2)[8]);
    (*out)[8] = (float)((float)((float)(*in1)[6] * (float)(*in2)[2]) + (float)((float)(*in1)[7] * (float)(*in2)[5]))
                        + (float)((float)(*in1)[8] * (float)(*in2)[8]);
    (*out)[9] = (float)((float)((float)((float)(*in1)[9] * (*in2)[0]) + (float)((float)(*in1)[10] * (float)(*in2)[3]))
                                        + (float)((float)(*in1)[11] * (float)(*in2)[6]))
                        + (float)(*in2)[9];
    (*out)[10] = (float)((float)((float)((float)(*in1)[9] * (float)(*in2)[1])
                                                         + (float)((float)(*in1)[10] * (float)(*in2)[4]))
                                         + (float)((float)(*in1)[11] * (float)(*in2)[7]))
                         + (float)(*in2)[10];
    (*out)[11] = (float)((float)((float)((float)(*in1)[9] * (float)(*in2)[2])
                                                         + (float)((float)(*in1)[10] * (float)(*in2)[5]))
                                         + (float)((float)(*in1)[11] * (float)(*in2)[8]))
                         + (float)(*in2)[11];
}

void __cdecl MatrixMultiply44(const float (*in1)[4], const float (*in2)[4], float (*out)[4])
{
    if ( in1 == out
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 1488, 0, "%s", "in1 != out") )
    {
        __debugbreak();
    }
    if ( in2 == out
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 1489, 0, "%s", "in2 != out") )
    {
        __debugbreak();
    }
    (*out)[0] = (float)((float)((float)((*in1)[0] * (*in2)[0]) + (float)((float)(*in1)[1] * (float)(*in2)[4]))
                                        + (float)((float)(*in1)[2] * (float)(*in2)[8]))
                        + (float)((float)(*in1)[3] * (float)(*in2)[12]);
    (*out)[1] = (float)((float)((float)((*in1)[0] * (float)(*in2)[1]) + (float)((float)(*in1)[1] * (float)(*in2)[5]))
                                        + (float)((float)(*in1)[2] * (float)(*in2)[9]))
                        + (float)((float)(*in1)[3] * (float)(*in2)[13]);
    (*out)[2] = (float)((float)((float)((*in1)[0] * (float)(*in2)[2]) + (float)((float)(*in1)[1] * (float)(*in2)[6]))
                                        + (float)((float)(*in1)[2] * (float)(*in2)[10]))
                        + (float)((float)(*in1)[3] * (float)(*in2)[14]);
    (*out)[3] = (float)((float)((float)((*in1)[0] * (float)(*in2)[3]) + (float)((float)(*in1)[1] * (float)(*in2)[7]))
                                        + (float)((float)(*in1)[2] * (float)(*in2)[11]))
                        + (float)((float)(*in1)[3] * (float)(*in2)[15]);
    (*out)[4] = (float)((float)((float)((float)(*in1)[4] * (*in2)[0]) + (float)((float)(*in1)[5] * (float)(*in2)[4]))
                                        + (float)((float)(*in1)[6] * (float)(*in2)[8]))
                        + (float)((float)(*in1)[7] * (float)(*in2)[12]);
    (*out)[5] = (float)((float)((float)((float)(*in1)[4] * (float)(*in2)[1]) + (float)((float)(*in1)[5] * (float)(*in2)[5]))
                                        + (float)((float)(*in1)[6] * (float)(*in2)[9]))
                        + (float)((float)(*in1)[7] * (float)(*in2)[13]);
    (*out)[6] = (float)((float)((float)((float)(*in1)[4] * (float)(*in2)[2]) + (float)((float)(*in1)[5] * (float)(*in2)[6]))
                                        + (float)((float)(*in1)[6] * (float)(*in2)[10]))
                        + (float)((float)(*in1)[7] * (float)(*in2)[14]);
    (*out)[7] = (float)((float)((float)((float)(*in1)[4] * (float)(*in2)[3]) + (float)((float)(*in1)[5] * (float)(*in2)[7]))
                                        + (float)((float)(*in1)[6] * (float)(*in2)[11]))
                        + (float)((float)(*in1)[7] * (float)(*in2)[15]);
    (*out)[8] = (float)((float)((float)((float)(*in1)[8] * (*in2)[0]) + (float)((float)(*in1)[9] * (float)(*in2)[4]))
                                        + (float)((float)(*in1)[10] * (float)(*in2)[8]))
                        + (float)((float)(*in1)[11] * (float)(*in2)[12]);
    (*out)[9] = (float)((float)((float)((float)(*in1)[8] * (float)(*in2)[1]) + (float)((float)(*in1)[9] * (float)(*in2)[5]))
                                        + (float)((float)(*in1)[10] * (float)(*in2)[9]))
                        + (float)((float)(*in1)[11] * (float)(*in2)[13]);
    (*out)[10] = (float)((float)((float)((float)(*in1)[8] * (float)(*in2)[2])
                                                         + (float)((float)(*in1)[9] * (float)(*in2)[6]))
                                         + (float)((float)(*in1)[10] * (float)(*in2)[10]))
                         + (float)((float)(*in1)[11] * (float)(*in2)[14]);
    (*out)[11] = (float)((float)((float)((float)(*in1)[8] * (float)(*in2)[3])
                                                         + (float)((float)(*in1)[9] * (float)(*in2)[7]))
                                         + (float)((float)(*in1)[10] * (float)(*in2)[11]))
                         + (float)((float)(*in1)[11] * (float)(*in2)[15]);
    (*out)[12] = (float)((float)((float)((float)(*in1)[12] * (*in2)[0]) + (float)((float)(*in1)[13] * (float)(*in2)[4]))
                                         + (float)((float)(*in1)[14] * (float)(*in2)[8]))
                         + (float)((float)(*in1)[15] * (float)(*in2)[12]);
    (*out)[13] = (float)((float)((float)((float)(*in1)[12] * (float)(*in2)[1])
                                                         + (float)((float)(*in1)[13] * (float)(*in2)[5]))
                                         + (float)((float)(*in1)[14] * (float)(*in2)[9]))
                         + (float)((float)(*in1)[15] * (float)(*in2)[13]);
    (*out)[14] = (float)((float)((float)((float)(*in1)[12] * (float)(*in2)[2])
                                                         + (float)((float)(*in1)[13] * (float)(*in2)[6]))
                                         + (float)((float)(*in1)[14] * (float)(*in2)[10]))
                         + (float)((float)(*in1)[15] * (float)(*in2)[14]);
    (*out)[15] = (float)((float)((float)((float)(*in1)[12] * (float)(*in2)[3])
                                                         + (float)((float)(*in1)[13] * (float)(*in2)[7]))
                                         + (float)((float)(*in1)[14] * (float)(*in2)[11]))
                         + (float)((float)(*in1)[15] * (float)(*in2)[15]);
}

void __cdecl MatrixTranspose(const float (*in)[3], float (*out)[3])
{
    if ( in == out
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 1520, 0, "%s", "in != out") )
    {
        __debugbreak();
    }
    (*out)[0] = (*in)[0];
    (*out)[1] = (*in)[3];
    (*out)[2] = (*in)[6];
    (*out)[3] = (*in)[1];
    (*out)[4] = (*in)[4];
    (*out)[5] = (*in)[7];
    (*out)[6] = (*in)[2];
    (*out)[7] = (*in)[5];
    (*out)[8] = (*in)[8];
}

void __cdecl MatrixTranspose44(const mat4x4 &in, mat4x4 &out)
{
        iassert(in != out);

        (out)[0][0] = (in)[0][0];
        (out)[0][1] = (in)[1][0];
        (out)[0][2] = (in)[2][0];
        (out)[0][3] = (in)[3][0];
        (out)[1][0] = (in)[0][1];
        (out)[1][1] = (in)[1][1];
        (out)[1][2] = (in)[2][1];
        (out)[1][3] = (in)[3][1];
        (out)[2][0] = (in)[0][2];
        (out)[2][1] = (in)[1][2];
        (out)[2][2] = (in)[2][2];
        (out)[2][3] = (in)[3][2];
        (out)[3][0] = (in)[0][3];
        (out)[3][1] = (in)[1][3];
        (out)[3][2] = (in)[2][3];
        (out)[3][3] = (in)[3][3];
}

void __cdecl MatrixInverse(const float (*in)[3], float (*out)[3])
{
    float det; // [esp+0h] [ebp-4h]
    float deta; // [esp+0h] [ebp-4h]

    iassert(in != out);
    det = (float)((float)((float)((float)((float)(*in)[8] * (float)(*in)[4]) - (float)((float)(*in)[7] * (float)(*in)[5]))
                                            * (*in)[0])
                            - (float)((float)((float)((float)(*in)[8] * (float)(*in)[1]) - (float)((float)(*in)[7] * (float)(*in)[2]))
                                            * (float)(*in)[3]))
            + (float)((float)((float)((float)(*in)[5] * (float)(*in)[1]) - (float)((float)(*in)[4] * (float)(*in)[2]))
                            * (float)(*in)[6]);
    iassert(det);
    deta = 1.0 / det;
    (*out)[0] = (float)((float)((float)(*in)[8] * (float)(*in)[4]) - (float)((float)(*in)[7] * (float)(*in)[5])) * deta;
    (*out)[1] = -((float)((float)(*in)[8] * (float)(*in)[1]) - (float)((float)(*in)[7] * (float)(*in)[2])) * deta;
    (*out)[2] = (float)((float)((float)(*in)[5] * (float)(*in)[1]) - (float)((float)(*in)[4] * (float)(*in)[2])) * deta;
    (*out)[3] = -((float)((float)(*in)[8] * (float)(*in)[3]) - (float)((float)(*in)[6] * (float)(*in)[5])) * deta;
    (*out)[4] = (float)((float)((float)(*in)[8] * (*in)[0]) - (float)((float)(*in)[6] * (float)(*in)[2])) * deta;
    (*out)[5] = -((float)((float)(*in)[5] * (*in)[0]) - (float)((float)(*in)[3] * (float)(*in)[2])) * deta;
    (*out)[6] = (float)((float)((float)(*in)[7] * (float)(*in)[3]) - (float)((float)(*in)[6] * (float)(*in)[4])) * deta;
    (*out)[7] = -((float)((float)(*in)[7] * (*in)[0]) - (float)((float)(*in)[6] * (float)(*in)[1])) * deta;
    (*out)[8] = (float)((float)((float)(*in)[4] * (*in)[0]) - (float)((float)(*in)[3] * (float)(*in)[1])) * deta;
}

void __cdecl MatrixInverseOrthogonal43(const float (*in)[3], float (*out)[3])
{
    float origin[3]; // [esp+4h] [ebp-Ch] BYREF

    MatrixTranspose(in, out);
    origin[0] = 0.0 - (float)(*in)[9];
    origin[1] = 0.0 - (float)(*in)[10];
    origin[2] = 0.0 - (float)(*in)[11];
    MatrixTransformVector(origin, out, &(*out)[9]);
}

void __cdecl MatrixInverse44(const mat4x4 &mat, mat4x4 &dst)
{
        float src[16]; // [esp+0h] [ebp-78h]
        float tmp[12]; // [esp+44h] [ebp-34h]

        float det; // [esp+40h] [ebp-38h]
        int i; // [esp+74h] [ebp-4h]

        iassert(mat != dst);

        for (i = 0; i < 4; ++i)
        {
            src[i] = (mat)[i][0];
            src[i + 4] = (mat)[i][1];
            src[i + 8] = (mat)[i][2];
            src[i + 12] = (mat)[i][3];
        }
        tmp[0] = src[10] * src[15];
        tmp[1] = src[11] * src[14];
        tmp[2] = src[9] * src[15];
        tmp[3] = src[11] * src[13];
        tmp[4] = src[9] * src[14];
        tmp[5] = src[10] * src[13];
        tmp[6] = src[8] * src[15];
        tmp[7] = src[11] * src[12];
        tmp[8] = src[8] * src[14];
        tmp[9] = src[10] * src[12];
        tmp[10] = src[8] * src[13];
        tmp[11] = src[9] * src[12];
        (dst)[0][0] = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7];
        (dst)[0][0] = (dst)[0][0] - (tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7]);
        (dst)[0][1] = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7];
        (dst)[0][1] = (dst)[0][1] - (tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7]);
        (dst)[0][2] = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
        (dst)[0][2] = (dst)[0][2] - (tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7]);
        (dst)[0][3] = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
        (dst)[0][3] = (dst)[0][3] - (tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6]);
        (dst)[1][0] = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3];
        (dst)[1][0] = (dst)[1][0] - (tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3]);
        (dst)[1][1] = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3];
        (dst)[1][1] = (dst)[1][1] - (tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3]);
        (dst)[1][2] = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
        (dst)[1][2] = (dst)[1][2] - (tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3]);
        (dst)[1][3] = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
        (dst)[1][3] = (dst)[1][3] - (tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2]);
        tmp[0] = src[2] * src[7];
        tmp[1] = src[3] * src[6];
        tmp[2] = src[1] * src[7];
        tmp[3] = src[3] * src[5];
        tmp[4] = src[1] * src[6];
        tmp[5] = src[2] * src[5];
        tmp[6] = src[0] * src[7];
        tmp[7] = src[3] * src[4];
        tmp[8] = src[0] * src[6];
        tmp[9] = src[2] * src[4];
        tmp[10] = src[0] * src[5];
        tmp[11] = src[1] * src[4];
        (dst)[2][0] = tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15];
        (dst)[2][0] = (dst)[2][0] - (tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15]);
        (dst)[2][1] = tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15];
        (dst)[2][1] = (dst)[2][1] - (tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15]);
        (dst)[2][2] = tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15];
        (dst)[2][2] = (dst)[2][2] - (tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15]);
        (dst)[2][3] = tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14];
        (dst)[2][3] = (dst)[2][3] - (tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14]);
        (dst)[3][0] = tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9];
        (dst)[3][0] = (dst)[3][0] - (tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10]);
        (dst)[3][1] = tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10];
        (dst)[3][1] = (dst)[3][1] - (tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8]);
        (dst)[3][2] = tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8];
        (dst)[3][2] = (dst)[3][2] - (tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9]);
        (dst)[3][3] = tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9];
        (dst)[3][3] = (dst)[3][3] - (tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8]);
        det = src[0] * (dst)[0][0] + src[1] * (dst)[0][1] + src[2] * (dst)[0][2] + src[3] * (dst)[0][3];

        iassert(det != 0.0);

        det = 1.0 / det;
        for (i = 0; i < 16; ++i)
                (dst)[0][i] = (dst)[0][i] * det;
}

void __cdecl MatrixTransformVector44(const float *vec, const float (*mat)[4], float *out)
{
    if ( vec == out
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 1742, 0, "%s", "vec != out") )
    {
        __debugbreak();
    }
    *out = (float)((float)((float)(*vec * (*mat)[0]) + (float)(vec[1] * (float)(*mat)[4]))
                             + (float)(vec[2] * (float)(*mat)[8]))
             + (float)(vec[3] * (float)(*mat)[12]);
    out[1] = (float)((float)((float)(*vec * (float)(*mat)[1]) + (float)(vec[1] * (float)(*mat)[5]))
                                 + (float)(vec[2] * (float)(*mat)[9]))
                 + (float)(vec[3] * (float)(*mat)[13]);
    out[2] = (float)((float)((float)(*vec * (float)(*mat)[2]) + (float)(vec[1] * (float)(*mat)[6]))
                                 + (float)(vec[2] * (float)(*mat)[10]))
                 + (float)(vec[3] * (float)(*mat)[14]);
    out[3] = (float)((float)((float)(*vec * (float)(*mat)[3]) + (float)(vec[1] * (float)(*mat)[7]))
                                 + (float)(vec[2] * (float)(*mat)[11]))
                 + (float)(vec[3] * (float)(*mat)[15]);
}

void __cdecl MatrixTransformVector43(const float *in1, const float (*in2)[3], float *out)
{
    if ( in1 == out
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 1752, 0, "%s", "in1 != out") )
    {
        __debugbreak();
    }
    *out = (float)((float)((float)(*in1 * (*in2)[0]) + (float)(in1[1] * (float)(*in2)[3]))
                             + (float)(in1[2] * (float)(*in2)[6]))
             + (float)(*in2)[9];
    out[1] = (float)((float)((float)(*in1 * (float)(*in2)[1]) + (float)(in1[1] * (float)(*in2)[4]))
                                 + (float)(in1[2] * (float)(*in2)[7]))
                 + (float)(*in2)[10];
    out[2] = (float)((float)((float)(*in1 * (float)(*in2)[2]) + (float)(in1[1] * (float)(*in2)[5]))
                                 + (float)(in1[2] * (float)(*in2)[8]))
                 + (float)(*in2)[11];
}

void __cdecl MatrixTransposeTransformVector43(const float *in1, const float (*in2)[3], float *out)
{
    float temp; // [esp+4h] [ebp-Ch]
    float temp_4; // [esp+8h] [ebp-8h]
    float temp_8; // [esp+Ch] [ebp-4h]

    if ( in1 == out
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 1762, 0, "%s", "in1 != out") )
    {
        __debugbreak();
    }
    temp = *in1 - (float)(*in2)[9];
    temp_4 = in1[1] - (float)(*in2)[10];
    temp_8 = in1[2] - (float)(*in2)[11];
    *out = (float)((float)((*in2)[0] * temp) + (float)((float)(*in2)[1] * temp_4)) + (float)((float)(*in2)[2] * temp_8);
    out[1] = (float)((float)((float)(*in2)[3] * temp) + (float)((float)(*in2)[4] * temp_4))
                 + (float)((float)(*in2)[5] * temp_8);
    out[2] = (float)((float)((float)(*in2)[6] * temp) + (float)((float)(*in2)[7] * temp_4))
                 + (float)((float)(*in2)[8] * temp_8);
}

void __cdecl MatrixTransformVector43Equals(float *out, const float *in)
{
    float temp; // [esp+0h] [ebp-8h]
    float temp_4; // [esp+4h] [ebp-4h]

    temp = (float)((float)((float)(*out * *in) + (float)(out[1] * in[3])) + (float)(out[2] * in[6])) + in[9];
    temp_4 = (float)((float)((float)(*out * in[1]) + (float)(out[1] * in[4])) + (float)(out[2] * in[7])) + in[10];
    out[2] = (float)((float)((float)(*out * in[2]) + (float)(out[1] * in[5])) + (float)(out[2] * in[8])) + in[11];
    *out = temp;
    out[1] = temp_4;
}

void __cdecl VectorAngleMultiply(float *vec, float angle)
{
    long double v2; // st7
    float temp; // [esp+Ch] [ebp-Ch]
    float x; // [esp+10h] [ebp-8h]
    float y; // [esp+14h] [ebp-4h]

    v2 = (float)(angle * 0.017453292);
    x = cos(v2);
    y = sin(v2);
    temp = (float)(*vec * x) - (float)(vec[1] * y);
    vec[1] = (float)(vec[1] * x) + (float)(*vec * y);
    *vec = temp;
}

void __cdecl UnitQuatToAxis(const float *quat, float (*axis)[3])
{
    const char *v2; // eax
    float yy; // [esp+24h] [ebp-30h]
    float xy; // [esp+28h] [ebp-2Ch]
    float scaledZ; // [esp+2Ch] [ebp-28h]
    float zz; // [esp+30h] [ebp-24h]
    float zw; // [esp+34h] [ebp-20h]
    float scaledX; // [esp+38h] [ebp-1Ch]
    float scaledY; // [esp+3Ch] [ebp-18h]
    float yw; // [esp+40h] [ebp-14h]
    float xz; // [esp+44h] [ebp-10h]
    float yz; // [esp+48h] [ebp-Ch]
    float xx; // [esp+4Ch] [ebp-8h]
    float xw; // [esp+50h] [ebp-4h]

    if ( !Vec4IsNormalized(quat) )
    {
        v2 = va("%g %g %g %g", *quat, quat[1], quat[2], quat[3]);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp",
                        1799,
                        0,
                        "%s\n\t%s",
                        "Vec4IsNormalized( quat )",
                        v2) )
            __debugbreak();
    }
    scaledX = *quat + *quat;
    xx = scaledX * *quat;
    xy = scaledX * quat[1];
    xz = scaledX * quat[2];
    xw = scaledX * quat[3];
    scaledY = quat[1] + quat[1];
    yy = scaledY * quat[1];
    yz = scaledY * quat[2];
    yw = scaledY * quat[3];
    scaledZ = quat[2] + quat[2];
    zw = scaledZ * quat[3];
    zz = scaledZ * quat[2];
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

void __cdecl UnitQuatToForward(const float *quat, float *forward)
{
    const char *v2; // eax

    if ( !Vec4IsNormalized(quat) )
    {
        v2 = va("%g %g %g %g", *quat, quat[1], quat[2], quat[3]);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp",
                        1831,
                        0,
                        "%s\n\t%s",
                        "Vec4IsNormalized( quat )",
                        v2) )
            __debugbreak();
    }
    *forward = 1.0 - (float)((float)((float)(quat[1] * quat[1]) + (float)(quat[2] * quat[2])) * 2.0);
    forward[1] = (float)((float)(*quat * quat[1]) + (float)(quat[2] * quat[3])) * 2.0;
    forward[2] = (float)((float)(*quat * quat[2]) - (float)(quat[1] * quat[3])) * 2.0;
}

void __cdecl QuatMultiplyInverse(const float *in1, const float *in2, float *out)
{
    out[0] = -in1[0] * in2[3]
        + in1[3] * in2[0]
        - in1[2] * in2[1]
        + in1[1] * in2[2];

    out[1] = -in1[1] * in2[3]
        + in1[2] * in2[0]
        + in1[3] * in2[1]
        - in1[0] * in2[2];

    out[2] = -in1[2] * in2[3]
        - in1[1] * in2[0]
        + in1[0] * in2[1]
        + in1[3] * in2[2];

    out[3] = in1[3] * in2[3]
        + in1[0] * in2[0]
        + in1[1] * in2[1]
        + in1[2] * in2[2];
}

void __cdecl QuatSlerp(const float *from, const float *to, float frac, float *result)
{
    float v4; // st6
    float v5; // [esp+0h] [ebp-30h]
    float v6; // [esp+4h] [ebp-2Ch]
    float v7; // [esp+8h] [ebp-28h]
    float v8; // [esp+Ch] [ebp-24h]
    float v9; // [esp+10h] [ebp-20h]
    float v10; // [esp+14h] [ebp-1Ch]
    float scaleTo; // [esp+20h] [ebp-10h]
    float scaleFrom; // [esp+24h] [ebp-Ch]
    float dot; // [esp+28h] [ebp-8h]
    bool dotWasNeg; // [esp+2Fh] [ebp-1h]

    dot = Vec4Dot(from, to);
    if (dot >= 0.0)
    {
        dotWasNeg = 0;
    }
    else
    {
        dotWasNeg = 1;
        dot = dot * -1.0;
    }
    if (dot <= 0.94999999)
    {
        v10 = acos(dot);
        v9 = sin(v10);
        v8 = 1.0 - v10 * frac;
        v7 = sin(v8);
        scaleFrom = v7 / v9;
        v6 = v10 * frac;
        v5 = sin(v6);
        scaleTo = v5 / v9;
    }
    else
    {
        scaleFrom = 1.0 - frac;
        scaleTo = frac;
    }
    if (dotWasNeg)
    {
        *result = scaleFrom * *from + scaleTo * *to * -1.0;
        result[1] = scaleFrom * from[1] + scaleTo * to[1] * -1.0;
        result[2] = scaleFrom * from[2] + scaleTo * to[2] * -1.0;
        v4 = scaleTo * to[3] * -1.0;
    }
    else
    {
        *result = scaleFrom * *from + scaleTo * *to;
        result[1] = scaleFrom * from[1] + scaleTo * to[1];
        result[2] = scaleFrom * from[2] + scaleTo * to[2];
        v4 = scaleTo * to[3];
    }
    result[3] = scaleFrom * from[3] + v4;
}

double __cdecl RotationToYaw(const float *rot)
{
    float v2; // [esp+0h] [ebp-18h]
    float v3; // [esp+4h] [ebp-14h]
    float v4; // [esp+8h] [ebp-10h]
    float r; // [esp+Ch] [ebp-Ch]
    float ra; // [esp+Ch] [ebp-Ch]
    float zz; // [esp+14h] [ebp-4h]

    zz = *rot * *rot;
    r = rot[1] * rot[1] + zz;
    iassert(r);
    ra = 2.0 / r;
    v4 = ra * (rot[1] * *rot);
    v3 = 1.0 - ra * zz;
    v2 = atan2(v4, v3);
    return (v2 * 57.2957763671875);
}

void __cdecl MatrixRotationZ(float (*mat)[3], float degree)
{
    float radians = degree * (3.14159265f / 180.0f);
    float s = sinf(radians);
    float c = cosf(radians);

    mat[0][0] = c; mat[0][1] = -s; mat[0][2] = 0.0f;
    mat[1][0] = s; mat[1][1] = c; mat[1][2] = 0.0f;
    mat[2][0] = 0.0f; mat[2][1] = 0.0f; mat[2][2] = 1.0f;
}

void __cdecl FinitePerspectiveMatrix(float tanHalfFovX, float tanHalfFovY, float zNear, float zFar, float (*mtx)[4])
{
    iassert(mtx);
    iassert(zNear > 0.0f);
    iassert(zFar > zNear);

    memset((unsigned __int8 *)mtx, 0, 0x40u);

    (*mtx)[0] = 1.0 / tanHalfFovX;
    (*mtx)[5] = 1.0 / tanHalfFovY;
    (*mtx)[10] = -zFar / (zNear - zFar);
    (*mtx)[11] = 1.0;
    (*mtx)[14] = zNear * zFar / (zNear - zFar);
}

// Some aislop used here to cleanup
// Helper: rotate a pair of orthonormal basis vectors (right, up) around forward by 'angle' (radians).
static void RotateBasisAroundForward(const float forward[3], const float right_in[3], const float up_in[3], float angle, float right_out[3], float up_out[3])
{
    float s = sinf(angle);
    float c = cosf(angle);

    // rotatedRight = right*c - up*s
    right_out[0] = right_in[0] * c - up_in[0] * s;
    right_out[1] = right_in[1] * c - up_in[1] * s;
    right_out[2] = right_in[2] * c - up_in[2] * s;

    // rotatedUp = right*s + up*c
    up_out[0] = right_in[0] * s + up_in[0] * c;
    up_out[1] = right_in[1] * s + up_in[1] * c;
    up_out[2] = right_in[2] * s + up_in[2] * c;
}

// Build a 4x4 column-major matrix given basis vectors (right, up, forward).
static void BuildBasisMatrixColumnMajor(const float right[3], const float up[3], const float forward[3], float outM[4][4])
{
    // column 0 = right
    outM[0][0] = right[0];
    outM[1][0] = right[1];
    outM[2][0] = right[2];
    outM[3][0] = 0.0f;

    // column 1 = up
    outM[0][1] = up[0];
    outM[1][1] = up[1];
    outM[2][1] = up[2];
    outM[3][1] = 0.0f;

    // column 2 = forward
    outM[0][2] = forward[0];
    outM[1][2] = forward[1];
    outM[2][2] = forward[2];
    outM[3][2] = 0.0f;

    // column 3 = translation (none)
    outM[0][3] = 0.0f;
    outM[1][3] = 0.0f;
    outM[2][3] = 0.0f;
    outM[3][3] = 1.0f;
}

/*
 SpotLightViewMatrix
  - direction: 3-float vector describing the spotlight pointing direction.
  - rotation: roll around the forward axis in radians.
  - mtx: output 4x4 matrix (column-major) describing the spotlight's view/orientation.
*/
void SpotLightViewMatrix(const float *direction, float rotation, float (*mtx)[4])
{
    float forward[3];
    float up[3];
    float right[3];
    float rrot[3], urot[3];

    // forward = -direction (we want the camera/light to look toward 'direction')
    forward[0] = -direction[0];
    forward[1] = -direction[1];
    forward[2] = -direction[2];

    // Get any vector perpendicular to forward
    // PerpendicularVector expects a source and writes the result into dst.
    // We pass 'forward' (not normalized) — PerpendicularVector should handle that, but we will normalize afterwards.
    PerpendicularVector(forward, up);

    // right = cross(up, forward)
    Vec3Cross(up, forward, right);

    // Normalize axes (defensive)
    Vec3Normalize(right);
    Vec3Normalize(up);
    Vec3Normalize(forward);

    // Apply roll rotation around forward axis by rotating the right/up pair
    RotateBasisAroundForward(forward, right, up, rotation, rrot, urot);

    // Build column-major matrix: [ right | up | forward | 0 ]
    BuildBasisMatrixColumnMajor(rrot, urot, forward, mtx);
}

/*
 SpotLightViewMatrixDir3
  - dirx, diry, dirz: three basis vectors (right, up, forward) or axis vectors. They are assumed to be
    a valid orthogonal-ish basis (not necessarily normalized).
  - mtx: output 4x4 matrix (column-major).
  This function simply normalizes the supplied axes and writes them into the output matrix.
*/
void SpotLightViewMatrixDir3(const float *dirx, const float *diry, const float *dirz, float (*mtx)[4])
{
    float right[3], up[3], forward[3];

    right[0] = dirx[0];
    right[1] = dirx[1];
    right[2] = dirx[2];

    up[0] = diry[0];
    up[1] = diry[1];
    up[2] = diry[2];

    forward[0] = dirz[0];
    forward[1] = dirz[1];
    forward[2] = dirz[2];

    // Normalize inputs (defensive)
    Vec3Normalize(right);
    Vec3Normalize(up);
    Vec3Normalize(forward);

    BuildBasisMatrixColumnMajor(right, up, forward, mtx);
}

void __cdecl SpotLightProjectionMatrix(float cosFov, float zNear, float zFar, float (*mtx)[4])
{
    float v4; // [esp+0h] [ebp-10h]
    float Q; // [esp+8h] [ebp-8h]
    float cotanFov; // [esp+Ch] [ebp-4h]

    memset((unsigned __int8 *)mtx, 0, 0x40u);
    if ( zNear >= 0.001 )
        v4 = zNear;
    else
        v4 = 0.001f;
    Q = zFar / (float)(zFar - v4);
    cotanFov = 1.0 / (float)(sqrtf(1.0 - (float)(cosFov * cosFov)) / cosFov);
    (*mtx)[0] = cotanFov;
    (*mtx)[5] = cotanFov;
    (*mtx)[10] = Q;
    (*mtx)[11] = 1.0f;
    //(*mtx)[14] = COERCE_FLOAT(LODWORD(Q) ^ _mask__NegFloat_) * v4;
    (*mtx)[14] = -Q * v4;
}

void __cdecl InfinitePerspectiveMatrix(float tanHalfFovX, float tanHalfFovY, float zNear, float (*mtx)[4])
{
        iassert(mtx);
        iassert(zNear > 0);

        memset((unsigned __int8 *)mtx, 0, sizeof(mat4x4));

        (*mtx)[0] = MAX_11BIT_FLT / tanHalfFovX;
        (*mtx)[5] = MAX_11BIT_FLT / tanHalfFovY;
        (*mtx)[10] = MAX_11BIT_FLT;
        (*mtx)[11] = 1.0f;
        (*mtx)[14] = -zNear * MAX_11BIT_FLT;
}

void __cdecl MatrixForViewer(const float *origin, const float (*axis)[3], float (*mtx)[4])
{
    iassert(mtx);
    iassert(origin);
    iassert(axis);

    ((*mtx)[0]) = -((*axis)[3]);
    ((*mtx)[4]) = -((*axis)[4]);
    ((*mtx)[8]) = -((*axis)[5]);
    ((*mtx)[12]) = -((float)((float)(*origin * (*mtx)[0]) + (float)(origin[1] * (*mtx)[4])) + (float)(origin[2] * (*mtx)[8]));
    (*mtx)[1] = (*axis)[6];
    (*mtx)[5] = (*axis)[7];
    (*mtx)[9] = (*axis)[8];
    ((*mtx)[13]) = -((float)((float)(*origin * (*mtx)[1]) + (float)(origin[1] * (*mtx)[5])) + (float)(origin[2] * (*mtx)[9]));
    (*mtx)[2] = (*axis)[0];
    (*mtx)[6] = (*axis)[1];
    (*mtx)[10] = (*axis)[2];
    ((*mtx)[14]) = -((float)((float)(*origin * (*mtx)[2]) + (float)(origin[1] * (*mtx)[6])) + (float)(origin[2] * (*mtx)[10]));
    (*mtx)[3] = 0.0f;
    (*mtx)[7] = 0.0f;
    (*mtx)[11] = 0.0f;
    (*mtx)[15] = 1.0f;
}

void __cdecl AnglesSubtract(const float *v1, const float *v2, float *v3)
{
    *v3 = AngleNormalize180(*v1 - *v2);
    v3[1] = AngleNormalize180(v1[1] - v2[1]);
    v3[2] = AngleNormalize180(v1[2] - v2[2]);
}

double __cdecl AngleNormalize360(float angle)
{
    float v3; // [esp+Ch] [ebp-10h]
    float result; // [esp+14h] [ebp-8h]

    v3 = floor((float)(angle * 0.0027777778));
    result = ((float)(angle * 0.0027777778) - v3) * 360.0;
    if ( (float)(result - 360.0) < 0.0 )
        return (float)(((float)(angle * 0.0027777778) - v3) * 360.0);
    else
        return (float)(result - 360.0);
}

double __cdecl AngleDelta(float angle1, float angle2)
{
    return AngleNormalize180(angle1 - angle2);
}

double __cdecl RadiusFromBounds(const float *mins, const float *maxs)
{
    float v4; // [esp+4h] [ebp-4h]

    v4 = RadiusFromBoundsSq(mins, maxs);
    return (float)sqrt(v4);
}

double __cdecl RadiusFromBounds2D(const float *mins, const float *maxs)
{
    float v4; // [esp+4h] [ebp-4h]

    v4 = RadiusFromBounds2DSq(mins, maxs);
    return (float)sqrt(v4);
}

double __cdecl RadiusFromBoundsSq(const float *mins, const float *maxs)
{
    float v3; // [esp+0h] [ebp-24h]
    int i; // [esp+10h] [ebp-14h]
    float corner[3]; // [esp+14h] [ebp-10h]
    float a; // [esp+20h] [ebp-4h]

    for ( i = 0; i < 3; ++i )
    {
        a = fabs(mins[i]);
        if ( a <= fabs(maxs[i]) )
            v3 = fabs(maxs[i]);
        else
            v3 = a;
        corner[i] = v3;
    }
    return corner[2] * corner[2] + corner[1] * corner[1] + corner[0] * corner[0];
}

double __cdecl RadiusFromBounds2DSq(const float *mins, const float *maxs)
{
    float v3; // [esp+0h] [ebp-20h]
    int i; // [esp+10h] [ebp-10h]
    float corner[2]; // [esp+14h] [ebp-Ch]
    float a; // [esp+1Ch] [ebp-4h]

    for ( i = 0; i < 2; ++i )
    {
        a = fabs(mins[i]);
        if ( a <= fabs(maxs[i]) )
            v3 = fabs(maxs[i]);
        else
            v3 = a;
        corner[i] = v3;
    }
    return corner[1] * corner[1] + corner[0] * corner[0];
}

void __cdecl ExtendBounds(float *mins, float *maxs, const float *offset)
{
    if ( *offset <= 0.0 )
        *mins = *mins + *offset;
    else
        *maxs = *maxs + *offset;
    if ( offset[1] <= 0.0 )
        mins[1] = mins[1] + offset[1];
    else
        maxs[1] = maxs[1] + offset[1];
    if ( offset[2] <= 0.0 )
        mins[2] = mins[2] + offset[2];
    else
        maxs[2] = maxs[2] + offset[2];
}

void __cdecl ExpandBoundsToWidth(float *mins, float *maxs)
{
    float v2; // [esp+0h] [ebp-18h]
    float diff; // [esp+4h] [ebp-14h]
    float size_8; // [esp+14h] [ebp-4h]

    if ( *maxs < *mins
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp",
                    2489,
                    0,
                    "%s",
                    "maxs[0] >= mins[0]") )
    {
        __debugbreak();
    }
    if ( maxs[1] < mins[1]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp",
                    2490,
                    0,
                    "%s",
                    "maxs[1] >= mins[1]") )
    {
        __debugbreak();
    }
    if ( maxs[2] < mins[2]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp",
                    2491,
                    0,
                    "%s",
                    "maxs[2] >= mins[2]") )
    {
        __debugbreak();
    }
    size_8 = maxs[2] - mins[2];
    if ( (float)((float)(*maxs - *mins) - (float)(maxs[1] - mins[1])) < 0.0 )
        v2 = maxs[1] - mins[1];
    else
        v2 = *maxs - *mins;
    if ( v2 > size_8 )
    {
        diff = (float)(v2 - size_8) * 0.5;
        mins[2] = mins[2] - diff;
        maxs[2] = maxs[2] + diff;
    }
}

void __cdecl ClearBounds(float *mins, float *maxs)
{
    mins[0] = 131072.0f;
    mins[1] = 131072.0f;
    mins[2] = 131072.0f;

    maxs[0] = -131072.0f;
    maxs[1] = -131072.0f;
    maxs[2] = -131072.0f;
}

bool __cdecl IsClearedBounds(const float *mins, const float *maxs)
{
    if ( *mins != 131072.0 || mins[1] != 131072.0 || mins[2] != 131072.0 )
        return 0;
    return *maxs == -131072.0 && maxs[1] == -131072.0 && maxs[2] == -131072.0;
}

void __cdecl ClearBounds2D(float *mins, float *maxs)
{
    mins[0] = 131072.0;
    mins[1] = 131072.0;
    maxs[0] = -131072.0;
    maxs[1] = -131072.0;
}

void __cdecl AddPointToBounds(const float *v, float *mins, float *maxs)
{
    if ( *mins > *v )
        *mins = *v;
    if ( *v > *maxs )
        *maxs = *v;
    if ( mins[1] > v[1] )
        mins[1] = v[1];
    if ( v[1] > maxs[1] )
        maxs[1] = v[1];
    if ( mins[2] > v[2] )
        mins[2] = v[2];
    if ( v[2] > maxs[2] )
        maxs[2] = v[2];
}

void __cdecl AddPointToBounds2D(const float *v, float *mins, float *maxs)
{
    if ( *mins > *v )
        *mins = *v;
    if ( *v > *maxs )
        *maxs = *v;
    if ( mins[1] > v[1] )
        mins[1] = v[1];
    if ( v[1] > maxs[1] )
        maxs[1] = v[1];
}

bool __cdecl PointInBounds(const float *v, const float *mins, const float *maxs)
{
    if ( !v && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 2593, 0, "%s", "v") )
        __debugbreak();
    if ( !mins && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 2594, 0, "%s", "mins") )
        __debugbreak();
    if ( !maxs && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 2595, 0, "%s", "maxs") )
        __debugbreak();
    if ( *mins > *v || *v > *maxs )
        return 0;
    if ( mins[1] > v[1] || v[1] > maxs[1] )
        return 0;
    return mins[2] <= v[2] && v[2] <= maxs[2];
}

bool __cdecl PointInBounds2D(const float *v, const float *mins, const float *maxs)
{
    if ( !v && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 2609, 0, "%s", "v") )
        __debugbreak();
    if ( !mins && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 2610, 0, "%s", "mins") )
        __debugbreak();
    if ( !maxs && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 2611, 0, "%s", "maxs") )
        __debugbreak();
    if ( *mins > *v || *v > *maxs )
        return 0;
    return mins[1] <= v[1] && v[1] <= maxs[1];
}

bool __cdecl BoundsOverlap(const float *mins0, const float *maxs0, const float *mins1, const float *maxs1)
{
    if ( *mins0 > *maxs1 || *mins1 > *maxs0 )
        return 0;
    if ( mins0[1] > maxs1[1] || mins1[1] > maxs0[1] )
        return 0;
    return mins0[2] <= maxs1[2] && mins1[2] <= maxs0[2];
}

void __cdecl ExpandBounds(const float *addedmins, const float *addedmaxs, float *mins, float *maxs)
{
    if ( *mins > *addedmins )
        *mins = *addedmins;
    if ( *addedmaxs > *maxs )
        *maxs = *addedmaxs;
    if ( mins[1] > addedmins[1] )
        mins[1] = addedmins[1];
    if ( addedmaxs[1] > maxs[1] )
        maxs[1] = addedmaxs[1];
    if ( mins[2] > addedmins[2] )
        mins[2] = addedmins[2];
    if ( addedmaxs[2] > maxs[2] )
        maxs[2] = addedmaxs[2];
}

void __cdecl ExpandBounds2D(const float *addedmins, const float *addedmaxs, float *mins, float *maxs)
{
    if ( *mins > *addedmins )
        *mins = *addedmins;
    if ( *addedmaxs > *maxs )
        *maxs = *addedmaxs;
    if ( mins[1] > addedmins[1] )
        mins[1] = addedmins[1];
    if ( addedmaxs[1] > maxs[1] )
        maxs[1] = addedmaxs[1];
}

void __cdecl GetRotatedBounds(
                const float (*baseBounds)[3],
                const float *origin,
                const float (*axis)[3],
                float (*rotatedBounds)[3])
{
    int axisIndex; // [esp+10h] [ebp-8h]
    int offset; // [esp+14h] [ebp-4h]
    int offseta; // [esp+14h] [ebp-4h]
    int offsetb; // [esp+14h] [ebp-4h]

    for ( axisIndex = 0; axisIndex < 3; ++axisIndex )
    {
        (*rotatedBounds)[axisIndex] = origin[axisIndex];
        (*rotatedBounds)[axisIndex + 3] = origin[axisIndex];
        offset = (*axis)[axisIndex] >= 0.0 ? 0 : 0xC;
        (*rotatedBounds)[axisIndex] = (float)(*(float *)((char *)*baseBounds + offset) * (float)(*axis)[axisIndex])
                                                                + (*rotatedBounds)[axisIndex];
        (*rotatedBounds)[axisIndex + 3] = (float)(*(float *)((char *)&(*baseBounds)[3] - offset) * (float)(*axis)[axisIndex])
                                                                        + (*rotatedBounds)[axisIndex + 3];
        offseta = (*axis)[axisIndex + 3] >= 0.0 ? 0 : 0xC;
        (*rotatedBounds)[axisIndex] = (float)(*(float *)((char *)&(*baseBounds)[1] + offseta) * (float)(*axis)[axisIndex + 3])
                                                                + (*rotatedBounds)[axisIndex];
        (*rotatedBounds)[axisIndex + 3] = (float)(*(float *)((char *)&(*baseBounds)[4] - offseta)
                                                                                        * (float)(*axis)[axisIndex + 3])
                                                                        + (*rotatedBounds)[axisIndex + 3];
        offsetb = (*axis)[axisIndex + 6] >= 0.0 ? 0 : 0xC;
        (*rotatedBounds)[axisIndex] = (float)(*(float *)((char *)&(*baseBounds)[2] + offsetb) * (float)(*axis)[axisIndex + 6])
                                                                + (*rotatedBounds)[axisIndex];
        (*rotatedBounds)[axisIndex + 3] = (float)(*(float *)((char *)&(*baseBounds)[5] - offsetb)
                                                                                        * (float)(*axis)[axisIndex + 6])
                                                                        + (*rotatedBounds)[axisIndex + 3];
    }
}

void __cdecl AxisClear(float (*axis)[3])
{
    (*axis)[0] = 1.0f;
    (*axis)[1] = 0.0f;
    (*axis)[2] = 0.0f;
    (*axis)[3] = 0.0f;
    (*axis)[4] = 1.0f;
    (*axis)[5] = 0.0f;
    (*axis)[6] = 0.0f;
    (*axis)[7] = 0.0f;
    (*axis)[8] = 1.0f;
}

void __cdecl AxisTranspose(const float (*in)[3], float (*out)[3])
{
    if ( in == out
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 2750, 0, "%s", "in != out") )
    {
        __debugbreak();
    }
    (*out)[0] = (*in)[0];
    (*out)[1] = (*in)[3];
    (*out)[2] = (*in)[6];
    (*out)[3] = (*in)[1];
    (*out)[4] = (*in)[4];
    (*out)[5] = (*in)[7];
    (*out)[6] = (*in)[2];
    (*out)[7] = (*in)[5];
    (*out)[8] = (*in)[8];
}

void __cdecl AxisTransformVec3(const float (*axes)[3], const float *vec, float *out)
{
    *out = (float)((float)(*vec * (*axes)[0]) + (float)(vec[1] * (float)(*axes)[3])) + (float)(vec[2] * (float)(*axes)[6]);
    out[1] = (float)((float)(*vec * (float)(*axes)[1]) + (float)(vec[1] * (float)(*axes)[4]))
                 + (float)(vec[2] * (float)(*axes)[7]);
    out[2] = (float)((float)(*vec * (float)(*axes)[2]) + (float)(vec[1] * (float)(*axes)[5]))
                 + (float)(vec[2] * (float)(*axes)[8]);
}

void __cdecl YawToAxis(float yaw, float (*axis)[3])
{
    float right[3]; // [esp+10h] [ebp-Ch] BYREF

    YawVectors(yaw, (float *)axis, right);
    (*axis)[6] = 0.0f;
    (*axis)[7] = 0.0f;
    (*axis)[8] = 1.0f;
    (*axis)[3] = 0.0 - right[0];
    (*axis)[4] = 0.0 - right[1];
    (*axis)[5] = 0.0 - right[2];
}

void __cdecl AxisToAngles(const float (*axis)[3], float *angles)
{
    float v2; // [esp+0h] [ebp-38h]
    float rad; // [esp+18h] [ebp-20h]
    float rada; // [esp+18h] [ebp-20h]
    float right[3]; // [esp+1Ch] [ebp-1Ch] BYREF
    float temp; // [esp+28h] [ebp-10h]
    float pitch; // [esp+2Ch] [ebp-Ch]
    float fCos; // [esp+30h] [ebp-8h]
    float fSin; // [esp+34h] [ebp-4h]

    vectoangles(axis[0], angles);
    right[0] = axis[1][0];
    right[1] = axis[1][1];
    right[2] = axis[1][2];
    rad = -angles[1] * 0.01745329238474369;
    fCos = cos(rad);
    fSin = sin(rad);
    temp = fCos * right[0] - fSin * right[1];
    right[1] = fSin * right[0] + fCos * right[1];
    rada = -*angles * 0.01745329238474369;
    fCos = cos(rada);
    fSin = sin(rada);
    right[0] = fSin * right[2] + fCos * temp;
    right[2] = fCos * right[2] - fSin * temp;
    pitch = vectosignedpitch(right);
    if (right[1] >= 0.0)
    {
        angles[2] = -pitch;
    }
    else
    {
        if (pitch >= 0.0)
            v2 = -180.0;
        else
            v2 = 180.0;
        angles[2] = pitch + v2;
    }
}

void __cdecl Axis4ToAngles(const float (*axis)[4], float *angles)
{
    float v2; // [esp+0h] [ebp-3Ch]
    float rad; // [esp+18h] [ebp-24h]
    float rada; // [esp+18h] [ebp-24h]
    float right[4]; // [esp+1Ch] [ebp-20h] BYREF
    float temp; // [esp+2Ch] [ebp-10h]
    float pitch; // [esp+30h] [ebp-Ch]
    float fCos; // [esp+34h] [ebp-8h]
    float fSin; // [esp+38h] [ebp-4h]

    vectoangles((const float *)axis, angles);
    right[0] = (*axis)[4];
    right[1] = (*axis)[5];
    right[2] = (*axis)[6];
    right[3] = (*axis)[7];
    //rad = COERCE_FLOAT(*((unsigned int *)angles + 1) ^ _mask__NegFloat_) * 0.017453292;
    rad = -angles[1] * 0.017453292;
    fCos = cos(rad);
    fSin = sin(rad);
    temp = (float)(fCos * right[0]) - (float)(fSin * right[1]);
    right[1] = (float)(fSin * right[0]) + (float)(fCos * right[1]);
    //rada = COERCE_FLOAT(*(unsigned int *)angles ^ _mask__NegFloat_) * 0.017453292;
    rada = -angles[0] * 0.017453292f;
    fCos = cos(rada);
    fSin = sin(rada);
    right[0] = (float)(fSin * right[2]) + (float)(fCos * temp);
    right[2] = (float)(fCos * right[2]) - (float)(fSin * temp);
    pitch = vectosignedpitch(right);
    if ( right[1] >= 0.0 )
    {
        //*((unsigned int *)angles + 2) = LODWORD(pitch) ^ _mask__NegFloat_;
        angles[2] = -pitch;
    }
    else
    {
        if ( pitch >= 0.0 )
            v2 = -180.0f;
        else
            v2 = 180.0f;
        angles[2] = pitch + v2;
    }
}

int __cdecl IntersectPlanes(const float **plane, float *xyz)
{
    float invDeterminant; // [esp+0h] [ebp-28h]
    float determinant; // [esp+8h] [ebp-20h]

    determinant = (plane[1][1] * plane[2][2] - plane[2][1] * plane[1][2]) * **plane
        + (plane[2][1] * (*plane)[2] - (*plane)[1] * plane[2][2]) * *plane[1]
        + ((*plane)[1] * plane[1][2] - plane[1][1] * (*plane)[2]) * *plane[2];
    if (I_fabs(determinant) < EQUAL_EPSILON)
        return 0;
    invDeterminant = 1.0 / determinant;
    *xyz = ((plane[1][1] * plane[2][2] - plane[2][1] * plane[1][2]) * (*plane)[3]
        + (plane[2][1] * (*plane)[2] - (*plane)[1] * plane[2][2]) * plane[1][3]
        + ((*plane)[1] * plane[1][2] - plane[1][1] * (*plane)[2]) * plane[2][3])
        * invDeterminant;
    xyz[1] = ((plane[1][2] * *plane[2] - plane[2][2] * *plane[1]) * (*plane)[3]
        + (plane[2][2] * **plane - (*plane)[2] * *plane[2]) * plane[1][3]
        + ((*plane)[2] * *plane[1] - plane[1][2] * **plane) * plane[2][3])
        * invDeterminant;
    xyz[2] = ((*plane[1] * plane[2][1] - *plane[2] * plane[1][1]) * (*plane)[3]
        + (*plane[2] * (*plane)[1] - **plane * plane[2][1]) * plane[1][3]
        + (**plane * plane[1][1] - *plane[1] * (*plane)[1]) * plane[2][3])
        * invDeterminant;
    return 1;
}

void __cdecl SnapPointToIntersectingPlanes(const float **planes, float *xyz, float snapGrid, float snapEpsilon)
{
    float v4; // [esp+0h] [ebp-68h]
    float v5; // [esp+4h] [ebp-64h]
    float v6; // [esp+Ch] [ebp-5Ch]
    float v7; // [esp+10h] [ebp-58h]
    float v9; // [esp+1Ch] [ebp-4Ch]
    float v10; // [esp+20h] [ebp-48h]
    float v11; // [esp+24h] [ebp-44h]
    float v12; // [esp+28h] [ebp-40h]
    float v13; // [esp+30h] [ebp-38h]
    float snapped[3]; // [esp+40h] [ebp-28h] BYREF
    float baseError; // [esp+4Ch] [ebp-1Ch]
    float maxBaseError; // [esp+50h] [ebp-18h]
    float snapError; // [esp+54h] [ebp-14h]
    float maxSnapError; // [esp+58h] [ebp-10h]
    float rounded; // [esp+5Ch] [ebp-Ch]
    int axis; // [esp+60h] [ebp-8h]
    int planeIndex; // [esp+64h] [ebp-4h]

    for (axis = 0; axis < 3; ++axis)
    {
        v13 = xyz[axis] / snapGrid;
        rounded = SnapFloat(v13) * snapGrid;
        v12 = rounded - xyz[axis];
        v9 = I_fabs(v12);
        if (snapEpsilon <= v9)
            snapped[axis] = xyz[axis];
        else
            snapped[axis] = rounded;
    }
    if (*xyz != snapped[0] || xyz[1] != snapped[1] || xyz[2] != snapped[2])
    {
        maxSnapError = 0.0;
        maxBaseError = snapEpsilon;
        for (planeIndex = 0; planeIndex < 3; ++planeIndex)
        {
            v7 = planes[planeIndex][3];
            v11 = Vec3Dot(planes[planeIndex], snapped) - v7;
            v6 = I_fabs(v11);
            snapError = v6;
            if (v6 > maxSnapError)
                maxSnapError = snapError;
            v5 = planes[planeIndex][3];
            v10 = Vec3Dot(planes[planeIndex], xyz) - v5;
            v4 = I_fabs(v10);
            baseError = v4;
            if (v4 > maxBaseError)
                maxBaseError = baseError;
        }
        if (maxBaseError > maxSnapError)
        {
            *xyz = snapped[0];
            xyz[1] = snapped[1];
            xyz[2] = snapped[2];
        }
    }
}

int __cdecl ProjectedWindingContainsCoplanarPoint(
                const float (*verts)[3],
                int vertCount,
                int x,
                int y,
                const float *point)
{
    int vertIndex; // [esp+10h] [ebp-10h]
    int vertIndexPrev; // [esp+14h] [ebp-Ch]

    vertIndexPrev = vertCount - 1;
    for ( vertIndex = 0; vertIndex < vertCount; ++vertIndex )
    {
        if ( (float)((float)((float)(point[x] - (float)(*verts)[3 * vertIndexPrev + x])
                                             * (float)((float)(*verts)[3 * vertIndex + y] - (float)(*verts)[3 * vertIndexPrev + y]))
                             + (float)((float)(point[y] - (float)(*verts)[3 * vertIndexPrev + y])
                                             * (float)((float)(*verts)[3 * vertIndexPrev + x] - (float)(*verts)[3 * vertIndex + x]))) < 0.0 )
            return 0;
        vertIndexPrev = vertIndex;
    }
    return 1;
}

int __cdecl PlaneFromPoints(float *plane, const float *v0, const float *v1, const float *v2)
{
    float v2_v0[3]; // [esp+8h] [ebp-20h] BYREF
    float v1_v0[3]; // [esp+14h] [ebp-14h] BYREF
    float length; // [esp+20h] [ebp-8h]
    float lengthSq; // [esp+24h] [ebp-4h]

    v1_v0[0] = *v1 - *v0;
    v1_v0[1] = v1[1] - v0[1];
    v1_v0[2] = v1[2] - v0[2];
    v2_v0[0] = *v2 - *v0;
    v2_v0[1] = v2[1] - v0[1];
    v2_v0[2] = v2[2] - v0[2];
    Vec3Cross(v2_v0, v1_v0, plane);
    lengthSq = (float)((float)(*plane * *plane) + (float)(plane[1] * plane[1])) + (float)(plane[2] * plane[2]);
    if ( lengthSq >= 2.0 )
        goto LABEL_7;
    if ( lengthSq == 0.0 )
        return 0;
    if ( (float)((float)((float)((float)((float)(v2_v0[0] * v2_v0[0]) + (float)(v2_v0[1] * v2_v0[1]))
                                                         + (float)(v2_v0[2] * v2_v0[2]))
                                         * (float)((float)((float)(v1_v0[0] * v1_v0[0]) + (float)(v1_v0[1] * v1_v0[1]))
                                                         + (float)(v1_v0[2] * v1_v0[2])))
                         * 0.0000010000001) >= lengthSq )
    {
        v1_v0[0] = *v2 - *v1;
        v1_v0[1] = v2[1] - v1[1];
        v1_v0[2] = v2[2] - v1[2];
        v2_v0[0] = *v0 - *v1;
        v2_v0[1] = v0[1] - v1[1];
        v2_v0[2] = v0[2] - v1[2];
        Vec3Cross(v2_v0, v1_v0, plane);
        if ( (float)((float)((float)((float)((float)(v2_v0[0] * v2_v0[0]) + (float)(v2_v0[1] * v2_v0[1]))
                                                             + (float)(v2_v0[2] * v2_v0[2]))
                                             * (float)((float)((float)(v1_v0[0] * v1_v0[0]) + (float)(v1_v0[1] * v1_v0[1]))
                                                             + (float)(v1_v0[2] * v1_v0[2])))
                             * 0.0000010000001) >= lengthSq )
            return 0;
    }
LABEL_7:
    length = sqrtf(lengthSq);
    *plane = *plane / length;
    plane[1] = plane[1] / length;
    plane[2] = plane[2] / length;
    plane[3] = (float)((float)(*v0 * *plane) + (float)(v0[1] * plane[1])) + (float)(v0[2] * plane[2]);
    return 1;
}

void __cdecl ProjectPointOnPlane(const float *const f1, const float *const normal, float *const result)
{
    const char *v3; // eax
    double v4; // [esp+18h] [ebp-14h]
    float d; // [esp+28h] [ebp-4h]

    iassert(Vec3IsNormalized(normal));
    d = -Vec3Dot(normal, f1);
    Vec3Mad(f1, d, normal, result);
}

void __cdecl SetPlaneSignbits(cplane_s *out)
{
    int j; // [esp+0h] [ebp-8h]
    unsigned __int8 bits; // [esp+7h] [ebp-1h]

    bits = 0;
    for ( j = 0; j < 3; ++j )
    {
        if ( out->normal[j] < 0.0 )
            bits |= 1 << j;
    }
    out->signbits = bits;
}

int bops_initialized;
int Ljmptab[8];
int __cdecl BoxOnPlaneSide(const float *emins, const float *emaxs, const cplane_s *p)
{
    // KISAKTODO: Needs ASM jump table and assembly bits (Probably critical function lmao)
    // 
    //int signbits; // eax
    //
    //if (bops_initialized != 1)
    //{
    //    bops_initialized = 1;
    //    Ljmptab[0] = (int)&Lcase0;
    //    Ljmptab[1] = (int)&Lcase1;
    //    Ljmptab[2] = (int)&Lcase2;
    //    Ljmptab[3] = (int)&Lcase3;
    //    Ljmptab[4] = (int)&Lcase4;
    //    Ljmptab[5] = (int)&Lcase5;
    //    Ljmptab[6] = (int)&Lcase6;
    //    Ljmptab[7] = (int)&Lcase7;
    //}
    //signbits = p->signbits;
    //if ((unsigned __int8)signbits < 8u)
    //    __asm { jmp     Ljmptab[eax * 4] }
    //__debugbreak();

    float v3;
    float v4;

    // LWSS: Note that this is not generic-able. The maths are slightly changed for each case.
    // These are opposite per-line
    // v3 = MAX, MAX, MAX
    // v4 = MIN, MIN, MIN
    // ... 
    // v3 = MIN, MAX, MIN
    // v4 = MAX, MIN, MAX
    switch (p->signbits)
    {
    case 0:
        v3 = (p->normal[0] * emaxs[0]) + (emaxs[1] * p->normal[1]) + (emaxs[2] * p->normal[2]);
        v4 = (p->normal[0] * emins[0]) + (emins[1] * p->normal[1]) + (emins[2] * p->normal[2]);
        break;
    case 1:
        v3 = (p->normal[0] * emins[0]) + (emaxs[1] * p->normal[1]) + (emaxs[2] * p->normal[2]);
        v4 = (p->normal[0] * emaxs[0]) + (emins[1] * p->normal[1]) + (emins[2] * p->normal[2]);
        break;
    case 2:
        v3 = (p->normal[0] * emaxs[0]) + (emaxs[2] * p->normal[2]) + (emins[1] * p->normal[1]);
        v4 = (p->normal[0] * emins[0]) + (emins[2] * p->normal[2]) + (emaxs[1] * p->normal[1]);
        break;
    case 3:
        v3 = (p->normal[0] * emins[0]) + (emaxs[2] * p->normal[2]) + (emins[1] * p->normal[1]);
        v4 = (p->normal[0] * emaxs[0]) + (emins[2] * p->normal[2]) + (emaxs[1] * p->normal[1]);
        break;
    case 4:
        v3 = (p->normal[0] * emaxs[0]) + (emaxs[1] * p->normal[1]) + (emins[2] * p->normal[2]);
        v4 = (p->normal[0] * emins[0]) + (emins[1] * p->normal[1]) + (emaxs[2] * p->normal[2]);
        break;
    case 5:
        v3 = (p->normal[0] * emins[0]) + (emaxs[1] * p->normal[1]) + (emins[2] * p->normal[2]);
        v4 = (p->normal[0] * emaxs[0]) + (emins[1] * p->normal[1]) + (emaxs[2] * p->normal[2]);
        break;
    case 6:
        v3 = (p->normal[0] * emaxs[0]) + (emins[1] * p->normal[1]) + (emins[2] * p->normal[2]);
        v4 = (p->normal[0] * emins[0]) + (emaxs[1] * p->normal[1]) + (emaxs[2] * p->normal[2]);
        break;
    case 7:
        v3 = (p->normal[0] * emins[0]) + (emins[1] * p->normal[1]) + (emins[2] * p->normal[2]);
        v4 = (p->normal[0] * emaxs[0]) + (emaxs[1] * p->normal[1]) + (emaxs[2] * p->normal[2]);
        break;
    default:
        iassert(alwaysfails);
        //if (!alwaysfails)
        //    MyAssertHandler(".\\universal\\com_math.cpp", 3473, 1, "BoxOnPlaneSide: invalid signbits for plane");

        __debugbreak();
        __debugbreak();
        __debugbreak();
        __debugbreak();
        __debugbreak();
        break;
    }

    return (2 * (v4 < p->dist)) | (v3 > p->dist); // KISAKTODO: probably BoxDistSqrdExceeds()
    //return BoxDistSqrdExceeds(emins, emaxs, p->normal, *(float *)&pa);
}

int __cdecl IsPosInsideArc(
                const float *pos,
                float posRadius,
                const float *arcOrigin,
                float arcRadius,
                float arcAngle0,
                float arcAngle1,
                float arcHalfHeight)
{
    float originDelta[3]; // [esp+10h] [ebp-18h] BYREF
    float distDelta; // [esp+1Ch] [ebp-Ch]
    float posYaw; // [esp+20h] [ebp-8h]
    float adjustedDistDelta; // [esp+24h] [ebp-4h]

    if ( !pos && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 3472, 0, "%s", "pos") )
        __debugbreak();
    if ( !arcOrigin
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 3473, 0, "%s", "arcOrigin") )
    {
        __debugbreak();
    }
    originDelta[0] = *pos - *arcOrigin;
    originDelta[1] = pos[1] - arcOrigin[1];
    originDelta[2] = pos[2] - arcOrigin[2];
    distDelta = Vec2Normalize(originDelta);
    adjustedDistDelta = distDelta - posRadius;
    if ( (float)((float)(distDelta - posRadius) * (float)(distDelta - posRadius)) > (float)(arcRadius * arcRadius) )
        return 0;
    if ( (float)(arcOrigin[2] - arcHalfHeight) > pos[2] || pos[2] > (float)(arcOrigin[2] + arcHalfHeight) )
        return 0;
    posYaw = vectoyaw(originDelta);
    posYaw = AngleNormalize360(posYaw);
    if ( arcAngle1 <= arcAngle0 )
    {
        if ( arcAngle1 > posYaw || posYaw > arcAngle0 )
            return 1;
    }
    else if ( arcAngle1 > posYaw && posYaw > arcAngle0 )
    {
        return 1;
    }
    return 0;
}

bool __cdecl BoxDistSqrdExceeds(const float *absmin, const float *absmax, const float *org, float fogOpaqueDistSqrd)
{
    float v5; // [esp+4h] [ebp-2Ch]
    float minsSqrd; // [esp+8h] [ebp-28h]
    float mins[3]; // [esp+Ch] [ebp-24h]
    float total; // [esp+18h] [ebp-18h]
    float maxs[3]; // [esp+1Ch] [ebp-14h]
    int i; // [esp+28h] [ebp-8h]
    float maxsSqrd; // [esp+2Ch] [ebp-4h]

    mins[0] = *absmin - *org;
    mins[1] = absmin[1] - org[1];
    mins[2] = absmin[2] - org[2];
    maxs[0] = *absmax - *org;
    maxs[1] = absmax[1] - org[1];
    maxs[2] = absmax[2] - org[2];
    total = 0.0f;
    for ( i = 0; i < 3; ++i )
    {
        if ( (float)(mins[i] * maxs[i]) > 0.0 )
        {
            minsSqrd = mins[i] * mins[i];
            maxsSqrd = maxs[i] * maxs[i];
            if ( minsSqrd <= maxsSqrd )
                v5 = minsSqrd;
            else
                v5 = maxsSqrd;
            total = total + v5;
        }
    }
    return total > fogOpaqueDistSqrd;
}

double __cdecl Q_rint(float in)
{
    return (float)floor(in + 0.5);
}

double __cdecl ColorNormalize(const float *in, float *out)
{
    float max; // [esp+0h] [ebp-8h]

    max = *in;
    if ( in[1] > *in )
        max = in[1];
    if ( in[2] > max )
        max = in[2];
    if ( max == 0.0 )
    {
        out[2] = 1.0f;
        out[1] = 1.0f;
        *out = 1.0f;
        return 0.0;
    }
    else
    {
        *out = (float)(1.0 / max) * *in;
        out[1] = (float)(1.0 / max) * in[1];
        out[2] = (float)(1.0 / max) * in[2];
        return max;
    }
}

void ColorSRGBtoLinear(const float* in, float* out)
{
    for (int i = 0; i < 3; ++i)
    {
        float s = in[i];
        float linear;

        if (s <= 0.04045f)
        {
            linear = s / 12.92f;
        }
        else
        {
            linear = powf((s + 0.055f) / 1.055f, 2.4f);
        }

        out[i] = linear;
    }
}

double __cdecl PitchForYawOnNormal(float fYaw, const float *normal)
{
    float v3; // [esp+Ch] [ebp-14h]
    float forward[3]; // [esp+14h] [ebp-Ch] BYREF

    if ( *normal == 0.0
        && normal[1] == 0.0
        && normal[2] == 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp",
                    3675,
                    0,
                    "%s",
                    "normal[0] || normal[1] || normal[2]") )
    {
        __debugbreak();
    }
    YawVectors(fYaw, forward, 0);
    if ( normal[2] == 0.0 )
        return 270.0;
    v3 = atan((float)((float)((float)(*normal * forward[0]) + (float)(normal[1] * forward[1])) / normal[2]));
    return v3 * 180.0 / 3.1415927;
}

void __cdecl NearestPitchAndYawOnPlane(const float *angles, const float *normal, float *result)
{
    float projected[3]; // [esp+0h] [ebp-18h] BYREF
    float forward[3]; // [esp+Ch] [ebp-Ch] BYREF

    if ( *normal == 0.0
        && normal[1] == 0.0
        && normal[2] == 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp",
                    3705,
                    0,
                    "%s",
                    "normal[0] || normal[1] || normal[2]") )
    {
        __debugbreak();
    }
    AngleVectors(angles, forward, 0, 0);
    ProjectPointOnPlane(forward, normal, projected);
    vectoangles(projected, result);
}

unsigned int holdrand_0;
void __cdecl Rand_Init(int seed)
{
    holdrand_0 = seed;
}

double __cdecl flrand(float min, float max)
{
    float result; // [esp+8h] [ebp-4h]

    holdrand_0 = 214013 * holdrand_0 + 2531011;
    result = (float)(holdrand_0 >> 17);
    return (float)((float)((float)((float)(max - min) * result) / 32768.0) + min);
}

int __cdecl irand(int min, int max)
{
    holdrand_0 = 214013 * holdrand_0 + 2531011;
    return (((holdrand_0 >> 17) * (__int64)(max - min)) >> 15) + min;
}

void __cdecl AxisToQuat(const float (*mat)[3], float *out)
{
    float *v2; // [esp+8h] [ebp-50h]
    float invLength; // [esp+Ch] [ebp-4Ch]
    float test[4][4]; // [esp+10h] [ebp-48h] BYREF
    int best; // [esp+50h] [ebp-8h]
    float testSizeSq; // [esp+54h] [ebp-4h]

    test[0][0] = (float)(*mat)[5] - (float)(*mat)[7];
    test[0][1] = (float)(*mat)[6] - (float)(*mat)[2];
    test[0][2] = (float)(*mat)[1] - (float)(*mat)[3];
    test[0][3] = (float)((float)((*mat)[0] + (float)(*mat)[4]) + (float)(*mat)[8]) + 1.0;
    testSizeSq = Vec4LengthSq(test[0]);
    if ( testSizeSq < 1.0 )
    {
        test[1][0] = (float)(*mat)[6] + (float)(*mat)[2];
        test[1][1] = (float)(*mat)[7] + (float)(*mat)[5];
        test[1][2] = (float)((float)((float)(*mat)[8] - (float)(*mat)[4]) - (*mat)[0]) + 1.0;
        test[1][3] = test[0][2];
        testSizeSq = Vec4LengthSq(test[1]);
        if ( testSizeSq < 1.0 )
        {
            test[2][0] = (float)((float)((*mat)[0] - (float)(*mat)[4]) - (float)(*mat)[8]) + 1.0;
            test[2][1] = (float)(*mat)[3] + (float)(*mat)[1];
            *(_QWORD *)&test[2][2] = __PAIR64__(LODWORD(test[0][0]), LODWORD(test[1][0]));
            testSizeSq = Vec4LengthSq(test[2]);
            if ( testSizeSq < 1.0 )
            {
                test[3][0] = test[2][1];
                test[3][1] = (float)((float)((float)(*mat)[4] - (*mat)[0]) - (float)(*mat)[8]) + 1.0;
                *(_QWORD *)&test[3][2] = __PAIR64__(LODWORD(test[0][1]), LODWORD(test[1][1]));
                testSizeSq = Vec4LengthSq(test[3]);
                if ( testSizeSq < 1.0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp",
                                3801,
                                0,
                                "%s\n\t(testSizeSq) = %g",
                                "(testSizeSq >= 1.0f)",
                                testSizeSq) )
                {
                    __debugbreak();
                }
                best = 3;
            }
            else
            {
                best = 2;
            }
        }
        else
        {
            best = 1;
        }
    }
    else
    {
        best = 0;
    }
    if ( testSizeSq == 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp", 3806, 0, "%s", "testSizeSq") )
    {
        __debugbreak();
    }
    invLength = 1.0 / sqrtf(testSizeSq);
    v2 = test[best];
    *out = invLength * *v2;
    out[1] = invLength * v2[1];
    out[2] = invLength * v2[2];
    out[3] = invLength * v2[3];
}

void __cdecl QuatLerp(const float *qa, const float *qb, float frac, float *out)
{
    float dot; // [esp+8h] [ebp-4h]

    dot = Vec4Dot(qa, qb);
    if (dot < 0.0)
    {
        *out = -*qb;
        out[1] = -qb[1];
        out[2] = -qb[2];
        out[3] = -qb[3];
        Vec4Lerp(qa, out, frac, out);
    }
    else
    {
        Vec4Lerp(qa, qb, frac, out);
    }
}

bool __cdecl CullBoxFromCone(
                const float *coneOrg,
                const float *coneDir,
                float cosHalfFov,
                const float *boxCenter,
                const float *boxHalfSize)
{
    float v6; // [esp+0h] [ebp-64h]
    float v7; // [esp+4h] [ebp-60h]
    float v8; // [esp+8h] [ebp-5Ch]
    float dist; // [esp+2Ch] [ebp-38h]
    float deltaMid; // [esp+30h] [ebp-34h]
    float deltaMid_4; // [esp+34h] [ebp-30h]
    float deltaMid_8; // [esp+38h] [ebp-2Ch]
    float sinHalfFovSq; // [esp+3Ch] [ebp-28h]
    float perpendicular; // [esp+44h] [ebp-20h]
    float perpendicular_4; // [esp+48h] [ebp-1Ch]
    float perpendicular_8; // [esp+4Ch] [ebp-18h]
    float scale; // [esp+50h] [ebp-14h]
    float perpLenSq; // [esp+54h] [ebp-10h]
    float farCorner; // [esp+58h] [ebp-Ch]
    float farCorner_4; // [esp+5Ch] [ebp-8h]
    float farCorner_8; // [esp+60h] [ebp-4h]

    if ( cosHalfFov < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp",
                    3926,
                    0,
                    "%s",
                    "cosHalfFov >= 0.0f") )
    {
        __debugbreak();
    }
    deltaMid = *boxCenter - *coneOrg;
    deltaMid_4 = boxCenter[1] - coneOrg[1];
    deltaMid_8 = boxCenter[2] - coneOrg[2];
    if ( *coneDir < 0.0 )
        v8 = -1.0f;
    else
        v8 = 1.0f;
    farCorner = deltaMid - (float)(*boxHalfSize * v8);
    if ( coneDir[1] < 0.0 )
        v7 = -1.0f;
    else
        v7 = 1.0f;
    farCorner_4 = deltaMid_4 - (float)(boxHalfSize[1] * v7);
    if ( coneDir[2] < 0.0 )
        v6 = -1.0f;
    else
        v6 = 1.0f;
    farCorner_8 = deltaMid_8 - (float)(boxHalfSize[2] * v6);
    dist = (float)((float)(farCorner * *coneDir) + (float)(farCorner_4 * coneDir[1])) + (float)(farCorner_8 * coneDir[2]);
    if ( dist >= 0.0 )
        return 1;
    perpendicular =   (float)( -dist * coneDir[0]) + farCorner;
    perpendicular_4 = (float)( -dist * coneDir[1]) + farCorner_4;
    perpendicular_8 = (float)( -dist * coneDir[2]) + farCorner_8;
    perpLenSq = (float)((float)(perpendicular * perpendicular) + (float)(perpendicular_4 * perpendicular_4))
                        + (float)(perpendicular_8 * perpendicular_8);
    sinHalfFovSq = 1.0 - (float)(cosHalfFov * cosHalfFov);
    if ( (float)((float)(dist * dist) * sinHalfFovSq) >= (float)(perpLenSq * (float)(cosHalfFov * cosHalfFov)) )
        return 0;
    scale = cosHalfFov / sqrtf(sinHalfFovSq * perpLenSq);
    return (float)((float)((float)((float)((float)((float)((float)((float)(scale * perpendicular) + *coneDir) * deltaMid)
                                                                                             + (float)((float)((float)(scale * perpendicular_4) + coneDir[1])
                                                                                                             * deltaMid_4))
                                                                             + (float)((float)((float)(scale * perpendicular_8) + coneDir[2]) * deltaMid_8))
                                                             - fabs(*boxHalfSize * (float)((float)(scale * perpendicular) + *coneDir)))
                                             - fabs(boxHalfSize[1] * (float)((float)(scale * perpendicular_4) + coneDir[1])))
                             - fabs(boxHalfSize[2] * (float)((float)(scale * perpendicular_8) + coneDir[2]))) >= 0.0;
}

bool __cdecl CullBoxFromSphere(const float *sphereOrg, float radius, const float *boxCenter, const float *boxHalfSize)
{
    float v5; // [esp+0h] [ebp-3Ch]
    float v6; // [esp+4h] [ebp-38h]
    float v7; // [esp+8h] [ebp-34h]

    if ( (float)((float)(fabs(*sphereOrg - *boxCenter) - *boxHalfSize)
                         - 0.0) < 0.0 )
        v7 = 0.0f;
    else
        v7 = fabs(*sphereOrg - *boxCenter) - *boxHalfSize;
    if ( (float)((float)(fabs(sphereOrg[1] - boxCenter[1]) - boxHalfSize[1])
                         - 0.0) < 0.0 )
        v6 = 0.0f;
    else
        v6 = fabs(sphereOrg[1] - boxCenter[1]) - boxHalfSize[1];
    if ( (float)((float)(fabs(sphereOrg[2] - boxCenter[2]) - boxHalfSize[2])
                         - 0.0) < 0.0 )
        v5 = 0.0f;
    else
        v5 = fabs(sphereOrg[2] - boxCenter[2]) - boxHalfSize[2];
    return (float)((float)((float)(v7 * v7) + (float)(v6 * v6)) + (float)(v5 * v5)) > (float)(radius * radius);
}

bool __cdecl CullBoxFromConicSectionOfSphere(
                const float *coneOrg,
                const float *coneDir,
                float cosHalfFov,
                float radius,
                const float *boxCenter,
                const float *boxHalfSize)
{
    float v7; // [esp+0h] [ebp-A0h]
    float v8; // [esp+4h] [ebp-9Ch]
    float v9; // [esp+8h] [ebp-98h]
    float v10; // [esp+Ch] [ebp-94h]
    float v11; // [esp+10h] [ebp-90h]
    float v12; // [esp+14h] [ebp-8Ch]
    float dist; // [esp+5Ch] [ebp-44h]
    float deltaMid; // [esp+60h] [ebp-40h]
    float deltaMid_4; // [esp+64h] [ebp-3Ch]
    float deltaMid_8; // [esp+68h] [ebp-38h]
    float sinHalfFovSq; // [esp+6Ch] [ebp-34h]
    float perpendicular; // [esp+80h] [ebp-20h]
    float perpendicular_4; // [esp+84h] [ebp-1Ch]
    float perpendicular_8; // [esp+88h] [ebp-18h]
    float scale; // [esp+8Ch] [ebp-14h]
    float perpLenSq; // [esp+90h] [ebp-10h]
    float farCorner; // [esp+94h] [ebp-Ch]
    float farCorner_4; // [esp+98h] [ebp-8h]
    float farCorner_8; // [esp+9Ch] [ebp-4h]

    if ( cosHalfFov < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp",
                    3999,
                    0,
                    "%s",
                    "cosHalfFov >= 0.0f") )
    {
        __debugbreak();
    }
    deltaMid = *boxCenter - *coneOrg;
    deltaMid_4 = boxCenter[1] - coneOrg[1];
    deltaMid_8 = boxCenter[2] - coneOrg[2];
    if ( (float)((float)(fabs(deltaMid) - *boxHalfSize) - 0.0) < 0.0 )
        v12 = 0.0f;
    else
        v12 = fabs(deltaMid) - *boxHalfSize;
    if ( (float)((float)(fabs(deltaMid_4) - boxHalfSize[1]) - 0.0) < 0.0 )
        v11 = 0.0f;
    else
        v11 = fabs(deltaMid_4) - boxHalfSize[1];
    if ( (float)((float)(fabs(deltaMid_8) - boxHalfSize[2]) - 0.0) < 0.0 )
        v10 = 0.0f;
    else
        v10 = fabs(deltaMid_8) - boxHalfSize[2];
    if ( (float)((float)((float)(v12 * v12) + (float)(v11 * v11)) + (float)(v10 * v10)) > (float)(radius * radius) )
        return 1;
    if ( *coneDir < 0.0 )
        v9 = -1.0f;
    else
        v9 = 1.0f;
    farCorner = deltaMid - (float)(*boxHalfSize * v9);
    if ( coneDir[1] < 0.0 )
        v8 = -1.0f;
    else
        v8 = 1.0f;
    farCorner_4 = deltaMid_4 - (float)(boxHalfSize[1] * v8);
    if ( coneDir[2] < 0.0 )
        v7 = -1.0f;
    else
        v7 = 1.0f;
    farCorner_8 = deltaMid_8 - (float)(boxHalfSize[2] * v7);
    dist = (float)((float)(farCorner * *coneDir) + (float)(farCorner_4 * coneDir[1])) + (float)(farCorner_8 * coneDir[2]);
    if ( dist >= 0.0 )
        return 1;
    perpendicular =   (float)(-dist * coneDir[0]) + farCorner;
    perpendicular_4 = (float)(-dist * coneDir[1]) + farCorner_4;
    perpendicular_8 = (float)(-dist * coneDir[2]) + farCorner_8;
    perpLenSq = (float)((float)(perpendicular * perpendicular) + (float)(perpendicular_4 * perpendicular_4))
                        + (float)(perpendicular_8 * perpendicular_8);
    sinHalfFovSq = 1.0 - (float)(cosHalfFov * cosHalfFov);
    if ( (float)((float)(dist * dist) * sinHalfFovSq) >= (float)(perpLenSq * (float)(cosHalfFov * cosHalfFov)) )
        return 0;
    scale = cosHalfFov / sqrtf(sinHalfFovSq * perpLenSq);
    return (float)((float)((float)((float)((float)((float)((float)((float)(scale * perpendicular) + *coneDir) * deltaMid)
                                                                                             + (float)((float)((float)(scale * perpendicular_4) + coneDir[1])
                                                                                                             * deltaMid_4))
                                                                             + (float)((float)((float)(scale * perpendicular_8) + coneDir[2]) * deltaMid_8))
                                                             - fabs(*boxHalfSize * (float)((float)(scale * perpendicular) + *coneDir)))
                                             - fabs(boxHalfSize[1] * (float)((float)(scale * perpendicular_4) + coneDir[1])))
                             - fabs(boxHalfSize[2] * (float)((float)(scale * perpendicular_8) + coneDir[2]))) >= 0.0;
}

bool __cdecl CullSphereFromCone(
                const float *coneOrg,
                const float *coneDir,
                float cosHalfFov,
                const float *sphereCenter,
                float radius)
{
    float delta; // [esp+8h] [ebp-28h]
    float delta_4; // [esp+Ch] [ebp-24h]
    float delta_8; // [esp+10h] [ebp-20h]
    float dist; // [esp+14h] [ebp-1Ch]
    float discriminant; // [esp+18h] [ebp-18h]
    float perpendicular_4; // [esp+24h] [ebp-Ch]
    float perpendicular_8; // [esp+28h] [ebp-8h]

    if ( cosHalfFov < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.cpp",
                    4061,
                    0,
                    "%s",
                    "cosHalfFov >= 0.0f") )
    {
        __debugbreak();
    }
    delta = *sphereCenter - *coneOrg;
    delta_4 = sphereCenter[1] - coneOrg[1];
    delta_8 = sphereCenter[2] - coneOrg[2];
    dist = (float)((float)(delta * *coneDir) + (float)(delta_4 * coneDir[1])) + (float)(delta_8 * coneDir[2]);
    if ( dist >= radius )
        return 1;
    //perpendicular_4 = (float)(COERCE_FLOAT(LODWORD(dist) ^ _mask__NegFloat_) * coneDir[1]) + delta_4;
    perpendicular_4 = (float)(-dist * coneDir[1]) + delta_4;
    //perpendicular_8 = (float)(COERCE_FLOAT(LODWORD(dist) ^ _mask__NegFloat_) * coneDir[2]) + delta_8;
    perpendicular_8 = (float)(-dist * coneDir[2]) + delta_8;
    discriminant = (float)(sqrtf(1.0 - (float)(cosHalfFov * cosHalfFov)) * dist) - radius;
    //return (float)((float)((float)((float)((float)((float)(COERCE_FLOAT(LODWORD(dist) ^ _mask__NegFloat_) * *coneDir)
    return (float)((float)((float)((float)((float)((float)(-dist * *coneDir)
                                                                                             + delta)
                                                                             //* (float)((float)(COERCE_FLOAT(LODWORD(dist) ^ _mask__NegFloat_) * *coneDir)
                                                                             * (float)((float)( -dist * *coneDir)
                                                                                             + delta))
                                                             + (float)(perpendicular_4 * perpendicular_4))
                                             + (float)(perpendicular_8 * perpendicular_8))
                             * (float)(cosHalfFov * cosHalfFov)) >= (float)(discriminant * discriminant);
}

void __cdecl colorTempMatrix(float (*finalMatrix)[4], float colorTemp)
{
    float v2; // [esp+2E8h] [ebp-148h]
    float v3; // [esp+2F0h] [ebp-140h]
    float v4; // [esp+2F8h] [ebp-138h]
    float v5; // [esp+300h] [ebp-130h]
    float v6; // [esp+308h] [ebp-128h]
    float v7; // [esp+310h] [ebp-120h]
    float v8; // [esp+318h] [ebp-118h]
    float v9; // [esp+320h] [ebp-110h]
    float v10; // [esp+328h] [ebp-108h]
    float v11[16]; // [esp+330h] [ebp-100h] BYREF
    float in1[32]; // [esp+370h] [ebp-C0h] BYREF
    float v13[4]; // [esp+3F0h] [ebp-40h] BYREF
    float out[4]; // [esp+400h] [ebp-30h] BYREF
    float vec[4]; // [esp+410h] [ebp-20h] BYREF
    float XYZ[4]; // [esp+420h] [ebp-10h] BYREF

    colorTempToXYZ(6500.0, XYZ);
    colorTempToXYZ(colorTemp, vec);
    MatrixTransformVector44(XYZ, bradfordMA, out);
    MatrixTransformVector44(vec, bradfordMA, v13);
    in1[16] = v13[0] / out[0];
    memset(&in1[17], 0, 16);
    in1[21] = v13[1] / out[1];
    memset(&in1[22], 0, 16);
    in1[26] = v13[2] / out[2];
    memset(&in1[27], 0, 16);
    in1[31] = 1.0f;
    MatrixMultiply44(bradfordMA, (const float (*)[4])&in1[16], (float (*)[4])in1);
    MatrixMultiply44((const float (*)[4])in1, bradfordMI, (float (*)[4])v11);
    MatrixMultiply44(sRGBtoXYZ, (const float (*)[4])v11, (float (*)[4])in1);
    MatrixMultiply44((const float (*)[4])in1, XYZtosRGB, finalMatrix);
    (*finalMatrix)[0] = (*finalMatrix)[0] + 0.00048828125;
    v10 = floor((float)((*finalMatrix)[0] * 1024.0));
    (*finalMatrix)[0] = v10 / 1024.0;
    (*finalMatrix)[1] = (*finalMatrix)[1] + 0.00048828125;
    v9 = floor((float)((*finalMatrix)[1] * 1024.0));
    (*finalMatrix)[1] = v9 / 1024.0;
    (*finalMatrix)[2] = (*finalMatrix)[2] + 0.00048828125;
    v8 = floor((float)((*finalMatrix)[2] * 1024.0));
    (*finalMatrix)[2] = v8 / 1024.0;
    (*finalMatrix)[4] = (*finalMatrix)[4] + 0.00048828125;
    v7 = floor((float)((*finalMatrix)[4] * 1024.0));
    (*finalMatrix)[4] = v7 / 1024.0;
    (*finalMatrix)[5] = (*finalMatrix)[5] + 0.00048828125;
    v6 = floor((float)((*finalMatrix)[5] * 1024.0));
    (*finalMatrix)[5] = v6 / 1024.0;
    (*finalMatrix)[6] = (*finalMatrix)[6] + 0.00048828125;
    v5 = floor((float)((*finalMatrix)[6] * 1024.0));
    (*finalMatrix)[6] = v5 / 1024.0;
    (*finalMatrix)[8] = (*finalMatrix)[8] + 0.00048828125;
    v4 = floor((float)((*finalMatrix)[8] * 1024.0));
    (*finalMatrix)[8] = v4 / 1024.0;
    (*finalMatrix)[9] = (*finalMatrix)[9] + 0.00048828125;
    v3 = floor((float)((*finalMatrix)[9] * 1024.0));
    (*finalMatrix)[9] = v3 / 1024.0;
    (*finalMatrix)[10] = (*finalMatrix)[10] + 0.00048828125;
    v2 = floor((float)((*finalMatrix)[10] * 1024.0));
    (*finalMatrix)[10] = v2 / 1024.0;
}

void __cdecl colorTempToXYZ(float colorTemp, float *XYZ)
{
    float v2; // xmm0_4
    float v3; // xmm0_4
    double v4; // [esp+0h] [ebp-50h]
    double v5; // [esp+8h] [ebp-48h]
    double yd; // [esp+18h] [ebp-38h]
    double xd; // [esp+20h] [ebp-30h]
    double t3; // [esp+28h] [ebp-28h]
    double t2; // [esp+30h] [ebp-20h]
    double xd2; // [esp+38h] [ebp-18h]
    double xd3; // [esp+40h] [ebp-10h]
    float x; // [esp+48h] [ebp-8h]

    if ( colorTemp >= 1650.0 )
        v5 = colorTemp;
    else
        v5 = 1650.0;
    if ( v5 <= 25000.0 )
        v4 = v5;
    else
        v4 = 25000.0;
    t2 = v4 * v4;
    t3 = v4 * v4 * v4;
    if ( v4 >= 4000.0 )
    {
        if ( v4 >= 7000.0 )
            xd = -2006400000.0 / t3 + 1901800.0 / t2 + 247.48 / v4 + 0.23704;
        else
            xd = -4607000000.0 / t3 + 2967800.0 / t2 + 99.11 / v4 + 0.244063;
    }
    else
    {
        xd = -266123900.0 / t3 - 234358.0 / t2 + 877.6956 / v4 + 0.17991;
    }
    xd2 = xd * xd;
    xd3 = xd * xd * xd;
    if ( v4 >= 4000.0 )
    {
        yd = -3.0 * xd2 + 2.87 * xd - 0.275;
    }
    else if ( xd > 0.50338 || v4 <= 2222.0 )
    {
        yd = -1.1063814 * xd3 - 1.3481102 * xd2 + 2.18555832 * xd - 0.20219683;
    }
    else
    {
        yd = -0.9549476 * xd3 - 1.37418593 * xd2 + 2.09137015 * xd - 0.16748867;
    }
    v2 = xd;
    x = v2;
    v3 = yd;
    *XYZ = x;
    XYZ[1] = v3;
    XYZ[2] = (float)(1.0 - x) - v3;
    XYZ[3] = 1.0f;
}

// (aislop)
// rotates hue by "hue" degrees
void colorHueMatrix(float finalMatrix[4][4], float hue)
{
    float rot[4][4];
    float basis1[4][4];
    float basis2[4][4];
    float tmp[4][4];

    const float H = hue * (3.14159265358979323846f / 180.0f);
    const float s = sinf(H);
    const float c = cosf(H);

    memset(basis1, 0, sizeof(basis1));

    basis1[0][0] = 0.81649655f;   // R
    basis1[0][1] = 0.0f;          // G
    basis1[0][2] = 0.53451085f;   // B

    basis1[1][0] = -0.40824828f;
    basis1[1][1] = 0.70710665f;
    basis1[1][2] = 1.0555116f;

    basis1[2][0] = -0.40824828f;
    basis1[2][1] = -0.70710677f;
    basis1[2][2] = 0.14204822f;

    basis1[3][3] = 1.0f;

    memset(rot, 0, sizeof(rot));

    rot[0][0] = 1.0f;  // luma unaffected
    rot[1][1] = c;
    rot[1][2] = -s;
    rot[2][1] = s;
    rot[2][2] = c;
    rot[3][3] = 1.0f;

    memset(basis2, 0, sizeof(basis2));

    basis2[0][0] = 0.84678853f;
    basis2[0][1] = -0.37795621f;
    basis2[0][2] = -0.37795624f;

    basis2[1][0] = -0.37292805f;
    basis2[1][1] = 0.33417869f;
    basis2[1][2] = -1.0800347f;

    basis2[2][0] = 0.5773502f;
    basis2[2][1] = 0.5773502f;
    basis2[2][2] = 0.5773502f;

    basis2[3][3] = 1.0f;

    MatrixMultiply44(basis1, rot, tmp);

    MatrixMultiply44(tmp, basis2, finalMatrix);

    for (int r = 0; r < 3; r++)
    {
        for (int c2 = 0; c2 < 3; c2++)
        {
            float v = finalMatrix[r][c2] + 0.00048828125f; // +1/2048
            v = floorf(v * 1024.0f) * (1.0f / 1024.0f);
            finalMatrix[r][c2] = v;
        }
    }
}


void __cdecl colorSaturationMatrix(float (*finalMatrix)[4], float saturation)
{
    float r; // [esp+0h] [ebp-Ch]
    float g; // [esp+4h] [ebp-8h]

    r = (float)(1.0 - saturation) * 0.25;
    g = (float)(1.0 - saturation) * 0.5;
    (*finalMatrix)[0] = r + saturation;
    (*finalMatrix)[1] = r;
    (*finalMatrix)[2] = r;
    (*finalMatrix)[3] = 0.0f;
    (*finalMatrix)[4] = g;
    (*finalMatrix)[5] = g + saturation;
    (*finalMatrix)[6] = g;
    (*finalMatrix)[7] = 0.0f;
    (*finalMatrix)[8] = r;
    (*finalMatrix)[9] = r;
    (*finalMatrix)[10] = r + saturation;
    (*finalMatrix)[15] = 0.0f;
    (*finalMatrix)[12] = 0.0f;
    (*finalMatrix)[13] = 0.0f;
    (*finalMatrix)[14] = 0.0f;
    (*finalMatrix)[15] = 1.0f;
}

void __cdecl AxisCopy(const float (*in)[3], float (*out)[3])
{
    (*out)[0] = (*in)[0];
    (*out)[1] = (*in)[1];
    (*out)[2] = (*in)[2];
    (*out)[3] = (*in)[3];
    (*out)[4] = (*in)[4];
    (*out)[5] = (*in)[5];
    (*out)[6] = (*in)[6];
    (*out)[7] = (*in)[7];
    (*out)[8] = (*in)[8];
}

void __cdecl Vec3Rotate(const float *in, const float (*matrix)[3], float *out)
{
    if (in == out
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_vector.h", 525, 0, "%s", "in != out"))
    {
        __debugbreak();
    }
    *out = (float)((float)(*in * (*matrix)[0]) + (float)(in[1] * (float)(*matrix)[1]))
        + (float)(in[2] * (float)(*matrix)[2]);
    out[1] = (float)((float)(*in * (float)(*matrix)[3]) + (float)(in[1] * (float)(*matrix)[4]))
        + (float)(in[2] * (float)(*matrix)[5]);
    out[2] = (float)((float)(*in * (float)(*matrix)[6]) + (float)(in[1] * (float)(*matrix)[7]))
        + (float)(in[2] * (float)(*matrix)[8]);
}

float __cdecl Vec3Length(const vec3r v)
{
    return sqrtf((v[2] * v[2]) + (v[1] * v[1]) + (v[0] * v[0]));
}

float __cdecl Vec3LengthSq(const float* v)
{
    float fDistSqrd = (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    iassert(!IS_NAN(fDistSqrd));
    return fDistSqrd;
}

void __cdecl Vec3NormalizeFast(float *v)
{
    float number; // [esp+0h] [ebp-1Ch]
    float invLength; // [esp+18h] [ebp-4h]

    number = Vec3LengthSq(v);
    invLength = I_rsqrt(number);
    v[0] = v[0] * invLength;
    v[1] = v[1] * invLength;
    v[2] = v[2] * invLength;
}

void __cdecl Vec3Lerp(const float *start, const float *end, float fraction, float *endpos)
{
        *endpos = (float)((float)(*end - *start) * fraction) + *start;
        endpos[1] = (float)((float)(end[1] - start[1]) * fraction) + start[1];
        endpos[2] = (float)((float)(end[2] - start[2]) * fraction) + start[2];
}

float __cdecl Vec3Distance(const float *v1, const float *v2)
{
    float dir[3]; // [esp+4h] [ebp-Ch] BYREF

    dir[0] = *v2 - *v1;
    dir[1] = v2[1] - v1[1];
    dir[2] = v2[2] - v1[2];
    return Vec3Length(dir);
}

float __cdecl Vec3DistanceSq(const float *p1, const float *p2)
{
    float v[3]; // [esp+4h] [ebp-8h]

    v[0] = p2[0] - p1[0];
    v[1] = p2[1] - p1[1];
    v[2] = p2[2] - p1[2];

    return Vec3LengthSq(v);
}

float Vec3Normalize(float *v)
{
        float ilength; // [esp+Ch] [ebp-8h]
        float length; // [esp+10h] [ebp-4h]

        length = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

        if (length > 0.0f)
                ilength = (1.0f / length);
        else
                ilength = 1.0f;

        v[0] = v[0] * ilength;
        v[1] = v[1] * ilength;
        v[2] = v[2] * ilength;

        return length;
}

float __cdecl Vec2Normalize(float *v)
{
        float ilength; // [esp+Ch] [ebp-8h]
        float length; // [esp+10h] [ebp-4h]

        length = sqrt(v[0] * v[0] + v[1] * v[1]);

        if (length > 0.0f)
                ilength = 1.0f / length;
        else
                ilength = 1.0f;

        v[0] = v[0] * ilength;
        v[1] = v[1] * ilength;

        return length;
}

void __cdecl Vec3Cross(const float *v0, const float *v1, float *cross)
{
    iassert(v0 != cross);
    iassert(v1 != cross);

    *cross = (float)(v0[1] * v1[2]) - (float)(v0[2] * v1[1]);
    cross[1] = (float)(v0[2] * *v1) - (float)(*v0 * v1[2]);
    cross[2] = (float)(*v0 * v1[1]) - (float)(v0[1] * *v1);
}

bool __cdecl Vec3IsNormalized(const float *v)
{
    return fabs((float)((float)((float)(*v * *v) + (float)(v[1] * v[1])) + (float)(v[2] * v[2])) - 1.0) < 0.002f;
}

float __cdecl Vec2Length(const float *v)
{
        return sqrtf(v[1] * v[1] + v[0] * v[0]);
}

float Vec2LengthSq(const float *v)
{
    return (v[0] * v[0]) + (v[1] * v[1]);
}

float AngleNormalize180(float angle)
{
        angle = fmodf(angle + 180.0f, 360.0f);
        if (angle < 0.0f)
                angle += 360.0f;
        return angle - 180.0f;
}

void __cdecl Vec3Scale(const float *v, float scale, float *result)
{
    result[0] = scale * v[0];
    result[1] = scale * v[1];
    result[2] = scale * v[2];
}

void __cdecl Vec3Avg(const float *a, const float *b, float *sum)
{
    sum[0] = (a[0] + b[0]) * 0.5f;
    sum[1] = (a[1] + b[1]) * 0.5f;
    sum[2] = (a[2] + b[2]) * 0.5f;
}

void __cdecl Vec3Mul(const float *a, const float *b, float *product)
{
    *product = *a * *b;
    product[1] = a[1] * b[1];
    product[2] = a[2] * b[2];
}

float __cdecl Vec3NormalizeTo(const vec3r v, vec3r out)
{
    float v3; // [esp+0h] [ebp-14h]
    float v4; // [esp+4h] [ebp-10h]
    float ilength; // [esp+Ch] [ebp-8h]
    float length; // [esp+10h] [ebp-4h]

    length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
    v4 = sqrt(length);
    if (-v4 < 0.0)
        v3 = v4;
    else
        v3 = 1.0;
    ilength = 1.0 / v3;

    out[0] = v[0] * ilength;
    out[1] = v[1] * ilength;
    out[2] = v[2] * ilength;
    return v4;
}

float __cdecl Vec3Dot(const vec3r a, const vec3r b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

float __cdecl Vec4Normalize(float *v)
{
    float v2; // [esp+0h] [ebp-Ch]
    float ilength; // [esp+4h] [ebp-8h]
    float length; // [esp+8h] [ebp-4h]

    length = *v * *v + v[1] * v[1] + v[2] * v[2] + v[3] * v[3];
    v2 = sqrt(length);
    if (v2 != 0.0)
    {
        ilength = 1.0 / v2;
        *v = *v * ilength;
        v[1] = v[1] * ilength;
        v[2] = v[2] * ilength;
        v[3] = v[3] * ilength;
    }
    return v2;
}

void __cdecl Vec4Lerp(const float *from, const float *to, float frac, float *result)
{
    *result = (float)((float)(*to - *from) * frac) + *from;
    result[1] = (float)((float)(to[1] - from[1]) * frac) + from[1];
    result[2] = (float)((float)(to[2] - from[2]) * frac) + from[2];
    result[3] = (float)((float)(to[3] - from[3]) * frac) + from[3];
}

void __cdecl Vec3Min(const float *a, const float *b, float *result)
{
    int v3; // [esp+0h] [ebp-Ch]
    int v4; // [esp+4h] [ebp-8h]
    int v5; // [esp+8h] [ebp-4h]

    if (*b <= *a)
        v5 = *(unsigned int *)b;
    else
        v5 = *(unsigned int *)a;
    *(unsigned int *)result = v5;
    if (b[1] <= a[1])
        v4 = *((unsigned int *)b + 1);
    else
        v4 = *((unsigned int *)a + 1);
    *((unsigned int *)result + 1) = v4;
    if (b[2] <= a[2])
        v3 = *((unsigned int *)b + 2);
    else
        v3 = *((unsigned int *)a + 2);
    *((unsigned int *)result + 2) = v3;
}

void __cdecl Vec3Max(const float *a, const float *b, float *result)
{
    int v3; // [esp+0h] [ebp-Ch]
    int v4; // [esp+4h] [ebp-8h]
    int v5; // [esp+8h] [ebp-4h]

    if (*a <= *b)
        v5 = *(unsigned int *)b;
    else
        v5 = *(unsigned int *)a;
    *(unsigned int *)result = v5;
    if (a[1] <= b[1])
        v4 = *((unsigned int *)b + 1);
    else
        v4 = *((unsigned int *)a + 1);
    *((unsigned int *)result + 1) = v4;
    if (a[2] <= b[2])
        v3 = *((unsigned int *)b + 2);
    else
        v3 = *((unsigned int *)a + 2);
    *((unsigned int *)result + 2) = v3;
}

void __cdecl Vec3Abs(const float *a, float *res)
{
    //*(_DWORD *)res = *(_DWORD *)a & _mask__AbsFloat_;
    //*((_DWORD *)res + 1) = (_DWORD)a[1] & _mask__AbsFloat_;
    //*((_DWORD *)res + 2) = (_DWORD)a[2] & _mask__AbsFloat_;
    res[0] = fabs(a[0]);
    res[1] = fabs(a[1]);
    res[2] = fabs(a[2]);
}

float __cdecl Vec4LengthSq(const float *v)
{
    return *v * *v + v[1] * v[1] + v[2] * v[2] + v[3] * v[3];
}

bool __cdecl Vec4IsNormalized(const float *v)
{
    float v2; // [esp+4h] [ebp-4h]

    v2 = Vec4LengthSq(v) - 1.0f;
    return fabs(v2) < 0.002f;
}

float __cdecl Vec4Dot(const float *a, const float *b)
{
    return (float)(*a * *b + a[1] * b[1] + a[2] * b[2] + a[3] * b[3]);
}

void __cdecl Vec4Set(float *v, float x, float y, float z, float w)
{
    *v = x;
    v[1] = y;
    v[2] = z;
    v[3] = w;
}

void __cdecl Vec4Copy(const float *from, float *to)
{
    *to = *from;
    to[1] = from[1];
    to[2] = from[2];
    to[3] = from[3];
}

void __cdecl Vec3Mad(const float *start, float scale, const float *dir, float *result)
{
    result[0] = start[0] + (scale * dir[0]);
    result[1] = start[1] + (scale * dir[1]);
    result[2] = start[2] + (scale * dir[2]);
}

void __cdecl Vec3ScaleMad(float scale0, const float *dir0, float scale1, const float *dir1, float *result)
{
    *result = (float)(scale0 * *dir0) + (float)(scale1 * *dir1);
    result[1] = (float)(scale0 * dir0[1]) + (float)(scale1 * dir1[1]);
    result[2] = (float)(scale0 * dir0[2]) + (float)(scale1 * dir1[2]);
}

void __cdecl Vec3AddScalar(const float *a, float s, float *sum)
{
    *sum = *a + s;
    sum[1] = a[1] + s;
    sum[2] = a[2] + s;
}

bool __cdecl Vec3Compare(const float *a, const float *b)
{
    return *a == *b && a[1] == b[1] && a[2] == b[2];
}

void __cdecl Vec3Add(const float *a, const float *b, float *sum)
{
    sum[0] = a[0] + b[0];
    sum[1] = a[1] + b[1];
    sum[2] = a[2] + b[2];
}

void __cdecl Vec4Add(const float *a, const float *b, float *sum)
{
    *sum = *a + *b;
    sum[1] = a[1] + b[1];
    sum[2] = a[2] + b[2];
    sum[3] = a[3] + b[3];
}

void __cdecl Vec3Sub(const float *a, const float *b, float *diff)
{
    *diff = *a - *b;
    diff[1] = a[1] - b[1];
    diff[2] = a[2] - b[2];
}

void Vec2Copy(const float *from, float *to)
{
    to[0] = from[0];
    to[1] = from[1];
}

void __cdecl Vec3Copy(const float *from, float *to)
{
    to[0] = from[0];
    to[1] = from[1];
    to[2] = from[2];
}

void __cdecl Vec2NormalizeFast(float *v)
{
    float number; // [esp+18h] [ebp-8h]
    float invLength; // [esp+1Ch] [ebp-4h]

    number = *v * *v + v[1] * v[1];
    invLength = I_rsqrt(number);
    *v = *v * invLength;
    v[1] = v[1] * invLength;
}

float Q_fabs(float f)
{
    int tmp = *(int *)&f;
    tmp &= 0x7FFFFFFF;
    return *(float *)&tmp;
}