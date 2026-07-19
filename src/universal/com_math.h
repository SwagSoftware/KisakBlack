#pragma once
#include "assertive.h"

#include <math.h>
#include "com_pack.h"

#define EQUAL_EPSILON 0.001f
#define ZERO_EPSILON 0.000001f
#define WEIGHT_EPSILON EQUAL_EPSILON

#define MAX_11BIT_FLT 0.99951172f // not a real name

// TODO change if we ever actually use classes
#define vec2r float*
#define vec3r float*
#define vec4r float*

// note - row major order
using mat3x3 = float[3][3];
using mat4x3 = float[4][3];
using mat4x4 = float[4][4];

struct cplane_s // sizeof=0x14
{                                                                             // XREF: cplane_t/r
                                                                                // CM_TraceThroughBrush/r ...
        float normal[3];                                        // XREF: CM_TraceThroughBrush+629/w
                                                                                // CM_TraceThroughBrush+636/w ...
        float dist;
        unsigned __int8 type;
        unsigned __int8 signbits;
        unsigned __int8 pad[2];
};


struct float44 // sizeof=0x40
{                                                                             // XREF: GfxLight/r GfxLight/r
        //$ED5082F4EF9C51C3CAAE283CF5E38ECF ___u0;
        union// $ED5082F4EF9C51C3CAAE283CF5E38ECF // sizeof=0x40
        {                                                                             // XREF: float44/r
                float m[4][4];
                float member[16];
        };
};

union alignas(16) float4 
{
        float v[4];
        unsigned int u[4];
        PackedUnitVec unitVec[4];
};

union float4x3 {
    union {
        float4 x;
        float4 y;
        float4 z;
    };

    mat4x3 mat;
};

union float4x4 {
    struct {
        float4 x;
        float4 y;
        float4 z;
        float4 w;
    };

    mat4x4 mat;
};

struct vector3 // sizeof=0x30
{                                       // XREF: vector4/r
    float4 x;                           // XREF: CreateClipMatrix+63/r
    float4 y;                           // XREF: CreateClipMatrix+78/r
    float4 z;                           // XREF: CreateClipMatrix+86/r
};

struct vector4 : vector3 // sizeof=0x40
{                                       // XREF: float4x4/r
    float4 w;                           // XREF: CreateClipMatrix+94/r
};

struct alignas(16) hybrid_vector // sizeof=0x10
{                                                                             // XREF: colgeom_visitor_t/r
        float4 vec;                                                 // XREF: AimTarget_IsTargetVisible+1B/w
};
static_assert(sizeof(hybrid_vector) == 16);

struct orientation_t // sizeof=0x30
{                                                                             // XREF: .rdata:orientation_t const orIdentity/r
    float origin[3];                                        // XREF: Weapon_DrawAxisOrigin(orientation_t,float)+EB/r
    float axis[3][3];                                     // XREF: CG_Laser_Add(centity_s *,DObj *,cpose_t *,float const * const,LaserOwnerEnum)+50/o
};

double __cdecl Com_Random();
double __cdecl Com_CRandom();
void __cdecl GaussianRandom(float *f0, float *f1);
unsigned int __cdecl RandWithSeed(int *seed);
double __cdecl randomhash(int value);
void __cdecl PointInCircleFromUniformDeviates(float radiusDeviate, float yawDeviate, float *point);
double __cdecl LinearTrack(float tgt, float cur, float rate, float deltaTime);
double __cdecl LinearTrackAngle(float tgt, float cur, float rate, float deltaTime);
double __cdecl DiffTrack(float tgt, float cur, float rate, float deltaTime);
double __cdecl DiffTrackAngle(float tgt, float cur, float rate, float deltaTime);
double __cdecl GraphGetValueFromFraction(int knotCount, const float (*knots)[2], float fraction);
double __cdecl Q_acos(float c);
char __cdecl ClampChar(int i);
__int16 __cdecl ClampShort(int i);
unsigned __int8 __cdecl DirToByte(const float *dir);
void __cdecl ByteToDir(unsigned int b, float *dir);
int __cdecl VecNCompareCustomEpsilon(const float *v0, const float *v1, float epsilon, int coordCount);
void __cdecl Vec3ProjectionCoords(const float *dir, int *xCoord, int *yCoord);
void __cdecl RotatePointAroundVector(float *dst, const float *dir, const float *point, float degrees);
void __cdecl Vec3Basis_RightHanded(const float *forward, float *left, float *up);
float __cdecl vectoyaw(const float *vec);
float __cdecl vectosignedyaw(const float *vec);
float __cdecl vectopitch(const float *vec);
void __cdecl vectoangles(const float *vec, float *angles);
void __cdecl UnitQuatToAngles(const float *quat, float *angles);
void __cdecl YawVectors(float yaw, float *forward, float *right);
void __cdecl YawVectors2D(float yaw, float *forward, float *right);
void __cdecl PerpendicularVector(const float *src, float *dst);
double __cdecl PointToLineDistSq(const float *point, const float *start, const float *end);
float __cdecl PointToLineDistSq2D(const float *point, const float *start, const float *end);
int __cdecl BoxInPlanes(const float (*planes)[4], unsigned int numPlanes, const float *mins, const float *maxs);
int __cdecl SphereInPlanes(const float (*planes)[4], unsigned int numPlanes, const float *center, float radius);
void __cdecl MatrixIdentity33(float (*out)[3]);
void __cdecl MatrixIdentity44(float (*out)[4]);
void __cdecl MatrixMultiply(const float (*in1)[3], const float (*in2)[3], float (*out)[3]);
void __cdecl MatrixVecMultiply(const float (*mulMat)[3], const float *mulVec, float *solution);
void __cdecl MatrixVecMultiply(const float (*mulMat)[4], const float *mulVec, float *solution);
void __cdecl MatrixVecMultiplyProject(const float (*mulMat)[4], const float *mulVec, float *solution);
void __cdecl MatrixMultiplyEquals(const float (*in)[3], float (*out)[3]);
void __cdecl MatrixMultiply43(const float (*in1)[3], const float (*in2)[3], float (*out)[3]);
void __cdecl MatrixMultiply44(const float (*in1)[4], const float (*in2)[4], float (*out)[4]);
void __cdecl MatrixTranspose(const float (*in)[3], float (*out)[3]);
void __cdecl MatrixTranspose44(const mat4x4 &in, mat4x4 &out);
void __cdecl MatrixInverse(const float (*in)[3], float (*out)[3]);
void __cdecl MatrixInverseOrthogonal43(const float (*in)[3], float (*out)[3]);
void __cdecl MatrixInverse44(const mat4x4 &mat, mat4x4 &dst);
void __cdecl MatrixTransformVector44(const float *vec, const float (*mat)[4], float *out);
void __cdecl MatrixTransformVector43(const float *in1, const float (*in2)[3], float *out);
void __cdecl MatrixTransposeTransformVector43(const float *in1, const float (*in2)[3], float *out);
void __cdecl MatrixTransformVector43Equals(float *out, const float *in);
void __cdecl VectorAngleMultiply(float *vec, float angle);
void __cdecl UnitQuatToAxis(const float *quat, float (*axis)[3]);
void __cdecl UnitQuatToForward(const float *quat, float *forward);
void __cdecl QuatMultiplyInverse(const float *in1, const float *in2, float *out);
void __cdecl QuatSlerp(const float *from, const float *to, float frac, float *result);
double __cdecl RotationToYaw(const float *rot);
void __cdecl MatrixRotationZ(float (*mat)[3], float degree);
void __cdecl FinitePerspectiveMatrix(float tanHalfFovX, float tanHalfFovY, float zNear, float zFar, float (*mtx)[4]);

void    SpotLightViewMatrix(const float *direction, float rotation, float (*mtx)[4]);
void    SpotLightViewMatrixDir3(
                const float *dirx,
                const float *diry,
                const float *dirz,
                float (*mtx)[4]);
void __cdecl SpotLightProjectionMatrix(float cosFov, float zNear, float zFar, float (*mtx)[4]);
void __cdecl InfinitePerspectiveMatrix(float tanHalfFovX, float tanHalfFovY, float zNear, float (*mtx)[4]);

void __cdecl MatrixForViewer(const float *origin, const float (*axis)[3], float (*mtx)[4]);
void __cdecl AnglesSubtract(const float *v1, const float *v2, float *v3);
double __cdecl AngleNormalize360(float angle);
double __cdecl AngleDelta(float angle1, float angle2);
double __cdecl RadiusFromBounds(const float *mins, const float *maxs);
double __cdecl RadiusFromBounds2D(const float *mins, const float *maxs);
double __cdecl RadiusFromBoundsSq(const float *mins, const float *maxs);
double __cdecl RadiusFromBounds2DSq(const float *mins, const float *maxs);
void __cdecl ExtendBounds(float *mins, float *maxs, const float *offset);
void __cdecl ExpandBoundsToWidth(float *mins, float *maxs);
void __cdecl ClearBounds(float *mins, float *maxs);
bool __cdecl IsClearedBounds(const float *mins, const float *maxs);
void __cdecl ClearBounds2D(float *mins, float *maxs);
void __cdecl AddPointToBounds(const float *v, float *mins, float *maxs);
void __cdecl AddPointToBounds2D(const float *v, float *mins, float *maxs);
bool __cdecl PointInBounds(const float *v, const float *mins, const float *maxs);
bool __cdecl PointInBounds2D(const float *v, const float *mins, const float *maxs);
bool __cdecl BoundsOverlap(const float *mins0, const float *maxs0, const float *mins1, const float *maxs1);
void __cdecl ExpandBounds(const float *addedmins, const float *addedmaxs, float *mins, float *maxs);
void __cdecl ExpandBounds2D(const float *addedmins, const float *addedmaxs, float *mins, float *maxs);
void __cdecl GetRotatedBounds(
                const float (*baseBounds)[3],
                const float *origin,
                const float (*axis)[3],
                float (*rotatedBounds)[3]);
void __cdecl AxisClear(float (*axis)[3]);
void __cdecl AxisTranspose(const float (*in)[3], float (*out)[3]);
void __cdecl AxisTransformVec3(const float (*axes)[3], const float *vec, float *out);
void __cdecl YawToAxis(float yaw, float (*axis)[3]);
void __cdecl AxisToAngles(const float (*axis)[3], float *angles);
void __cdecl Axis4ToAngles(const float (*axis)[4], float *angles);
int __cdecl IntersectPlanes(const float **plane, float *xyz);
void __cdecl SnapPointToIntersectingPlanes(const float **planes, float *xyz, float snapGrid, float snapEpsilon);
int __cdecl ProjectedWindingContainsCoplanarPoint(
                const float (*verts)[3],
                int vertCount,
                int x,
                int y,
                const float *point);
int __cdecl PlaneFromPoints(float *plane, const float *v0, const float *v1, const float *v2);
void __cdecl ProjectPointOnPlane(const float *p, const float *normal, float *dst);
void __cdecl SetPlaneSignbits(cplane_s *out);
int __cdecl BoxOnPlaneSide(const float *emins, const float *emaxs, const cplane_s *p);
int __cdecl IsPosInsideArc(
                const float *pos,
                float posRadius,
                const float *arcOrigin,
                float arcRadius,
                float arcAngle0,
                float arcAngle1,
                float arcHalfHeight);
bool __cdecl BoxDistSqrdExceeds(const float *absmin, const float *absmax, const float *org, float fogOpaqueDistSqrd);
double __cdecl Q_rint(float in);
double __cdecl ColorNormalize(const float *in, float *out);
void __cdecl ColorSRGBtoLinear(const float *in, float *out);
double __cdecl PitchForYawOnNormal(float fYaw, const float *normal);
void __cdecl NearestPitchAndYawOnPlane(const float *angles, const float *normal, float *result);
void __cdecl Rand_Init(int seed);
double __cdecl flrand(float min, float max);
int __cdecl irand(int min, int max);
void __cdecl AxisToQuat(const float (*mat)[3], float *out);
void __cdecl QuatLerp(const float *qa, const float *qb, float frac, float *out);
bool __cdecl CullBoxFromCone(
                const float *coneOrg,
                const float *coneDir,
                float cosHalfFov,
                const float *boxCenter,
                const float *boxHalfSize);
bool __cdecl CullBoxFromSphere(const float *sphereOrg, float radius, const float *boxCenter, const float *boxHalfSize);
bool __cdecl CullBoxFromConicSectionOfSphere(
                const float *coneOrg,
                const float *coneDir,
                float cosHalfFov,
                float radius,
                const float *boxCenter,
                const float *boxHalfSize);
bool __cdecl CullSphereFromCone(
                const float *coneOrg,
                const float *coneDir,
                float cosHalfFov,
                const float *sphereCenter,
                float radius);
void __cdecl colorTempMatrix(float (*finalMatrix)[4], float colorTemp);
void __cdecl colorTempToXYZ(float colorTemp, float *XYZ);
void __cdecl colorHueMatrix(float (*finalMatrix)[4], float hue);
void __cdecl colorSaturationMatrix(float (*finalMatrix)[4], float saturation);
void __cdecl AxisCopy(const float (*in)[3], float (*out)[3]);

float __cdecl Vec2Length(const float *v);
float Vec2LengthSq(const float *v);
void __cdecl Vec2NormalizeFast(float *v);

float __cdecl Vec3Length(const vec3r v);
float __cdecl Vec3LengthSq(const float *v);
void __cdecl Vec3NormalizeFast(float *v);
void __cdecl Vec3Rotate(const float *in, const float (*matrix)[3], float *out);
void __cdecl Vec3Lerp(const float *start, const float *end, float fraction, float *endpos);
float __cdecl Vec3DistanceSq(const float *p1, const float *p2);
float __cdecl Vec3Distance(const float *v1, const float *v2);

float __cdecl Vec3Normalize(float *v);
float __cdecl Vec3NormalizeTo(const vec3r v, vec3r out);
float __cdecl Vec2Normalize(float *v);
void __cdecl Vec3Cross(const float *v0, const float *v1, float *cross);
float __cdecl Vec3Dot(const vec3r a, const vec3r b);
void __cdecl Vec3Min(const float *a, const float *b, float *result);
void __cdecl Vec3Max(const float *a, const float *b, float *result);
void __cdecl Vec3Abs(const float *a, float *res);
bool __cdecl Vec3IsNormalized(const float *v);
void __cdecl Vec3Scale(const float *v, float scale, float *result);
void __cdecl Vec3Avg(const float *a, const float *b, float *sum);
void __cdecl Vec3Mul(const float *a, const float *b, float *product);
void __cdecl Vec3Mad(const float *start, float scale, const float *dir, float *result);
#define VectorMA(...) \
    typedef char VectorMA_From_Quake_Is_Vec3Mad_same_args_though[-1]

bool __cdecl Vec3Compare(const float *a, const float *b);
void __cdecl Vec3ScaleMad(float scale0, const float *dir0, float scale1, const float *dir1, float *result);
void __cdecl Vec3Add(const float *a, const float *b, float *sum);
void __cdecl Vec4Add(const float *a, const float *b, float *sum);
void __cdecl Vec3AddScalar(const float *a, float s, float *sum);
void __cdecl Vec3Sub(const float *a, const float *b, float *diff);
void Vec2Copy(const float *from, float *to);
void __cdecl Vec3Copy(const float *from, float *to);
#define VectorCopy(...) \
    typedef char VectorCopy_From_Quake_Is_Vec3Copy_same_args_though[-1]

void __cdecl Vec4Lerp(const float *from, const float *to, float frac, float *result);
float __cdecl Vec4Normalize(float *v);
float __cdecl Vec4LengthSq(const float *v);
bool __cdecl Vec4IsNormalized(const float *v);
float __cdecl Vec4Dot(const float *a, const float *b);
void __cdecl Vec4Set(float *v, float x, float y, float z, float w);
void __cdecl Vec4Copy(const float *from, float *to);

float AngleNormalize180(float angle);

#define Vec3Clear(a)			((a)[0]=(a)[1]=(a)[2]=0)

inline void MatrixTransformVector(const float *in1, const float (*in2)[3], float *out)
{
    iassert(in1 != out);

    out[0] = (float)((float)(*in1 * (*in2)[0]) + (float)(in1[1] * (float)(*in2)[3])) + (float)(in1[2] * (float)(*in2)[6]);
    out[1] = (float)((float)(*in1 * (float)(*in2)[1]) + (float)(in1[1] * (float)(*in2)[4]))
        + (float)(in1[2] * (float)(*in2)[7]);
    out[2] = (float)((float)(*in1 * (float)(*in2)[2]) + (float)(in1[1] * (float)(*in2)[5]))
        + (float)(in1[2] * (float)(*in2)[8]);
}

inline void __cdecl MatrixTransposeTransformVector(const float *in1, const float (*in2)[3], float *out)
{
    if (in1 == out
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 1423, 0, "%s", "in1 != out"))
    {
        __debugbreak();
    }
    *out = (float)((float)(*in1 * (*in2)[0]) + (float)(in1[1] * (float)(*in2)[1])) + (float)(in1[2] * (float)(*in2)[2]);
    out[1] = (float)((float)(*in1 * (float)(*in2)[3]) + (float)(in1[1] * (float)(*in2)[4]))
        + (float)(in1[2] * (float)(*in2)[5]);
    out[2] = (float)((float)(*in1 * (float)(*in2)[6]) + (float)(in1[1] * (float)(*in2)[7]))
        + (float)(in1[2] * (float)(*in2)[8]);
}
/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
inline float __cdecl Q_rsqrt(float number)
{
    iassert(number);
    iassert(!isnan(number));

    union standards_compliant_fp_bit_hack {
        int i;
        float f;
    };

    standards_compliant_fp_bit_hack v;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y = number;
    v.f = y;						// evil floating point bit level hacking
    v.i = 0x5f3759df - (v.i >> 1);               // what the fuck?
    y = v.f;
    y = y * (threehalfs - (x2 * y * y));   // 1st iteration
    //	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

    // TODO: use rsqrtss instead since it's not 1990 anymore
    return y;
}

inline float I_fmax(float x, float y)
{
    if ((x - y) < 0.0)
        return y;
    else
        return x;
}

inline float I_fmin(float x, float y)
{
    if ((y - x) < 0.0)
        return y;
    else
        return x;
}

float Q_fabs(float f);

#define I_rsqrt Q_rsqrt
#define I_fabs Q_fabs

#define IS_NAN(x) (isnan(x))

constexpr float vec2_origin[2] = { 0.0, 0.0 };
constexpr float vec3_origin[3] = { 0.0, 0.0, 0.0 };
constexpr float vec4_origin[4] = { 0.0, 0.0, 0.0, 0.0 };

constexpr float ikIdentityMatrix44[4][4] =
{
  { 1.0, 0.0, 0.0, 0.0 },
  { 0.0, 1.0, 0.0, 0.0 },
  { 0.0, 0.0, 1.0, 0.0 },
  { 0.0, 0.0, 0.0, 1.0 }
};

constexpr float identityMatrix44[4][4] =
{
  { 1.0, 0.0, 0.0, 0.0 },
  { 0.0, 1.0, 0.0, 0.0 },
  { 0.0, 0.0, 1.0, 0.0 },
  { 0.0, 0.0, 0.0, 1.0 }
};

// another one
constexpr float g_identityMatrix44[4][4] =
{
  { 1.0, 0.0, 0.0, 0.0 },
  { 0.0, 1.0, 0.0, 0.0 },
  { 0.0, 0.0, 1.0, 0.0 },
  { 0.0, 0.0, 0.0, 1.0 }
};

constexpr float4 g_swizzleXYZA = {
    .u = {
        0x00010203,
        0x04050607,
        0x08090A0B,
        0x10111213 // this row goes over 16
    }
};
// each individual member never goes over 16
constexpr float4 g_swizzleYZXW = {
    .u = {
        0x04050607,
        0x08090A0B,
        0x00010203,
        0x0C0D0E0F
    }
};

constexpr float4 g_keepXYW = {
    .u = {
        0xFFFFFFFF,
        0xFFFFFFFF,
        0,
        0xFFFFFFFF
    }
};

constexpr float4 g_keepXYZ = {
    .u = {
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0
    }
};


constexpr float4 g_zero = { { 0.0, 0.0, 0.0, 0.0 } };

constexpr float quatZRot[4] = { 0.0, 0.0, 1.0, 0.0 };

constexpr float c_fadeRate = 1.0;

const float SQRT2_0 = sqrtf(2.0);

constexpr orientation_t orIdentity
{
    .origin = {0.0f, 0.0f, 0.0f},
    .axis = {{1.0f,0.0f,0.0f},{0.0f,1.0f,0.0f},{0.0f,0.0f,1.0f}}
};