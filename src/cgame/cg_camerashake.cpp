#include "cg_camerashake.h"
#include <demo/demo_playback.h>

CameraShakeSet s_cameraShakeSet[1];

void __cdecl CG_StartShakeCamera(int localClientNum, float p, int duration, float *src, float radius)
{
    int j; // [esp+8h] [ebp-38h]
    const cg_s *cgameGlob; // [esp+Ch] [ebp-34h]
    float minsize; // [esp+10h] [ebp-30h]
    int i; // [esp+14h] [ebp-2Ch]
    CameraShakeSet *cameraShakeArray; // [esp+18h] [ebp-28h]
    CameraShake buildShake; // [esp+1Ch] [ebp-24h] BYREF

    if ( p <= 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_camerashake.cpp", 88, 0, "%s", "p > 0.0f") )
    {
        __debugbreak();
    }
    if ( duration <= 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_camerashake.cpp", 89, 0, "%s", "duration > 0") )
    {
        __debugbreak();
    }
    if ( radius <= 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_camerashake.cpp", 90, 0, "%s", "radius > 0.0f") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    buildShake.size = 0.0f;
    buildShake.rumbleScale = 0.0f;
    buildShake.scale = p;
    buildShake.length = (float)duration;
    buildShake.time = cgameGlob->time;
    buildShake.src[0] = *src;
    buildShake.src[1] = src[1];
    buildShake.src[2] = src[2];
    buildShake.radius = radius;
    CG_UpdateCameraShake(cgameGlob, &buildShake);
    cameraShakeArray = &s_cameraShakeSet[localClientNum];
    for ( i = 0; i < 4; ++i )
    {
        if ( cameraShakeArray->shakes[i].time > cgameGlob->time
            || (float)cgameGlob->time >= (float)((float)cameraShakeArray->shakes[i].time + cameraShakeArray->shakes[i].length) )
        {
            goto LABEL_25;
        }
    }
    minsize = buildShake.size;
    if ( i != 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_camerashake.cpp",
                    118,
                    0,
                    "%s",
                    "i == MAX_CAMERA_SHAKE") )
    {
        __debugbreak();
    }
    for ( j = 0; j < 4; ++j )
    {
        if ( minsize > cameraShakeArray->shakes[j].size )
        {
            minsize = cameraShakeArray->shakes[j].size;
            i = j;
        }
    }
    if ( i != 4 )
LABEL_25:
        memcpy(&cameraShakeArray->shakes[i], &buildShake, sizeof(cameraShakeArray->shakes[i]));
}

int __cdecl CG_UpdateCameraShake(const cg_s *cgameGlob, CameraShake *shake)
{
    int dtime; // [esp+24h] [ebp-14h]
    float dist; // [esp+28h] [ebp-10h]
    float val; // [esp+2Ch] [ebp-Ch]
    float vala; // [esp+2Ch] [ebp-Ch]
    float timePercent; // [esp+30h] [ebp-8h]
    float x; // [esp+34h] [ebp-4h]

    if ( (LODWORD(shake->length) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_camerashake.cpp",
                    42,
                    0,
                    "%s",
                    "!IS_NAN(shake->length)") )
    {
        __debugbreak();
    }
    if ( (LODWORD(shake->radius) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_camerashake.cpp",
                    43,
                    0,
                    "%s",
                    "!IS_NAN(shake->radius)") )
    {
        __debugbreak();
    }
    if ( (LODWORD(shake->scale) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_camerashake.cpp",
                    44,
                    0,
                    "%s",
                    "!IS_NAN(shake->scale)") )
    {
        __debugbreak();
    }
    if ( Demo_IsPaused() || Demo_IsCompleted() || Demo_GetClipPausedState() )
        return 0;
    dtime = cgameGlob->time - shake->time;
    if ( dtime < 0 || (float)dtime >= shake->length )
        return 0;
    if ( shake->length <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_camerashake.cpp",
                    54,
                    0,
                    "%s\n\t(shake->length) = %g",
                    "(shake->length > 0.0f)",
                    shake->length) )
    {
        __debugbreak();
    }
    if ( shake->radius <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_camerashake.cpp",
                    55,
                    0,
                    "%s\n\t(shake->radius) = %g",
                    "(shake->radius > 0.0f)",
                    shake->radius) )
    {
        __debugbreak();
    }
    if ( shake->scale <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_camerashake.cpp",
                    56,
                    0,
                    "%s\n\t(shake->scale) = %g",
                    "(shake->scale > 0.0f)",
                    shake->scale) )
    {
        __debugbreak();
    }
    dist = Vec3Distance(shake->src, cgameGlob->refdef.vieworg);
    val = 1.0 - (float)(dist / shake->radius);
    timePercent = (float)dtime / shake->length;
    if ( (timePercent < 0.0 || timePercent >= 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_camerashake.cpp",
                    62,
                    0,
                    "%s\n\t(timePercent) = %g",
                    "(timePercent >= 0.0f && timePercent < 1.0f)",
                    timePercent) )
    {
        __debugbreak();
    }
    x = (float)(1.0 - timePercent) * shake->scale;
    if ( x <= 0.0 )
        return 0;
    if ( val < 0.0 )
        vala = val / x;
    else
        vala = val * x;
    shake->size = vala;
    shake->rumbleScale = x;
    return 1;
}

void __cdecl CG_ShakeCamera(int localClientNum)
{
    double v1; // xmm0_8
    double v2; // xmm0_8
    double v3; // xmm0_8
    long double v4; // [esp+0h] [ebp-2Ch]
    long double v5; // [esp+0h] [ebp-2Ch]
    long double v6; // [esp+0h] [ebp-2Ch]
    CameraShakeSet *camShakeSet; // [esp+Ch] [ebp-20h]
    CameraShake *cameraShake; // [esp+10h] [ebp-1Ch]
    cg_s *cgameGlob; // [esp+14h] [ebp-18h]
    float rumbleScale; // [esp+1Ch] [ebp-10h]
    float scale; // [esp+20h] [ebp-Ch]
    int i; // [esp+24h] [ebp-8h]
    float sx; // [esp+28h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    camShakeSet = &s_cameraShakeSet[localClientNum];
    scale = 0.0f;
    rumbleScale = 0.0f;
    sx = (float)cgameGlob->time / 600.0;
    for ( i = 0; i < 4; ++i )
    {
        cameraShake = &camShakeSet->shakes[i];
        if ( CG_UpdateCameraShake(cgameGlob, cameraShake) )
        {
            if ( cameraShake->size > scale )
            {
                scale = cameraShake->size;
                rumbleScale = cameraShake->rumbleScale;
            }
        }
    }
    if ( cgameGlob->rumbleScale > scale )
    {
        scale = cgameGlob->rumbleScale;
        rumbleScale = scale;
    }
   //if ( scale > 0.0 )
   //{
   //    if ( scale > 1.0 )
   //        scale = 1.0f;
   //    v1 = (float)((float)(25.132742 * sx) + camShakeSet->phase);
   //    __libm_sse2_sin(v4);
   //    *(float *)&v1 = v1;
   //    cgameGlob->refdefViewAngles[0] = cgameGlob->refdefViewAngles[0]
   //                                                                 + (float)((float)((float)(*(float *)&v1 * rumbleScale) * 18.0) * scale);
   //    *((float *)&v5 + 1) = (float)(47.12389 * sx) + camShakeSet->phase;
   //    v2 = *((float *)&v5 + 1);
   //    __libm_sse2_sin(v5);
   //    *(float *)&v2 = v2;
   //    cgameGlob->refdefViewAngles[1] = cgameGlob->refdefViewAngles[1]
   //                                                                 + (float)((float)((float)(*(float *)&v2 * rumbleScale) * 16.0) * scale);
   //    *(float *)&v6 = (float)(37.699112 * sx) + camShakeSet->phase;
   //    v3 = *(float *)&v6;
   //    __libm_sse2_sin(v6);
   //    *(float *)&v3 = v3;
   //    cgameGlob->refdefViewAngles[2] = cgameGlob->refdefViewAngles[2]
   //                                                                 + (float)((float)((float)(*(float *)&v3 * rumbleScale) * 10.0) * scale);
   //}
    if (scale > 0.0f)
    {
        if (scale > 1.0f)
            scale = 1.0f;

        float angleX = 25.132742f * sx + camShakeSet->phase;
        float angleY = 47.12389f * sx + camShakeSet->phase;
        float angleZ = 37.699112f * sx + camShakeSet->phase;

        cgameGlob->refdefViewAngles[0] += sinf(angleX) * rumbleScale * 18.0f * scale;
        cgameGlob->refdefViewAngles[1] += sinf(angleY) * rumbleScale * 16.0f * scale;
        cgameGlob->refdefViewAngles[2] += sinf(angleZ) * rumbleScale * 10.0f * scale;
    }
    else
    {
        camShakeSet->phase = Com_CRandom() * 3.1415927;
    }
}

void __cdecl CG_ClearCameraShakes(int localClientNum)
{
    memset((unsigned __int8 *)&s_cameraShakeSet[localClientNum], 0, 0x90u);
}

