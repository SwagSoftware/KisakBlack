#include "r_water.h"
#include "r_material.h"
#include <universal/q_shared.h>
#include "r_image_load_common.h"
#include "r_image.h"
#include "r_dvars.h"
#include <universal/com_workercmds.h>
#include "r_workercmds.h"

WaterGlob waterGlob;
volatile unsigned int g_waterLock;
WaterGlobStatic waterGlobStatic;

volatile unsigned int r_waterLimit = 1;
jqModule r_waterModule =
{
    .Name = "r_water",
    .Type = JQ_WORKER_GENERIC,
    .Code = (int(__cdecl *)(jqBatch *))r_waterCallback,
};
jqWorkerCmd r_waterWorkerCmd = { &r_waterModule, 4u, 0, 0, &r_waterLimit, NULL, 0u };

void __cdecl R_UploadWaterTextureInternal(water_t **data)
{
    water_t *water; // [esp+4h] [ebp-8h]

    water = *data;
    WaterFrequenciesAtTime(waterGlob.H, water, water->writable.floatTime);
    WaterAmplitudesFromFrequencies(waterGlob.H, water);
    WaterPixelsFromAmplitudes((GfxColor *)waterGlob.pixels, waterGlob.H, water);
    GenerateMipMaps(D3DFMT_L8, waterGlob.pixels, water);
    _InterlockedExchangeAdd(&g_waterLock, 0xFFFFFFFF);
}

void __cdecl WaterFrequenciesAtTime(complex_s *H, const water_t *water, float t)
{
    __int64 v3; // [esp+0h] [ebp-4Ch]
    float sinReal; // [esp+30h] [ebp-1Ch]
    int vecKCount; // [esp+34h] [ebp-18h]
    int vecKIndex; // [esp+38h] [ebp-14h]
    float *wTerm; // [esp+3Ch] [ebp-10h]
    complex_s *H0; // [esp+40h] [ebp-Ch]
    float sinImag; // [esp+44h] [ebp-8h]
    float ta; // [esp+5Ch] [ebp+10h]

    if ( !H && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp", 157, 0, "%s", "H") )
        __debugbreak();
    if ( !water && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp", 158, 0, "%s", "water") )
        __debugbreak();
    ta = t * 162.97466;
    vecKCount = water->N * water->M;
    if ( vecKCount > 4096
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp",
                    163,
                    0,
                    "%s\n\t(vecKCount) = %i",
                    "(vecKCount <= (64 * 64))",
                    vecKCount) )
    {
        __debugbreak();
    }
    vecKIndex = 0;
    wTerm = water->wTerm;
    H0 = water->H0;
    while ( vecKIndex < vecKCount )
    {
        _mm_prefetch((const char *)wTerm + 512, 1);
        _mm_prefetch((const char *)&H0[64], 1);
        if ( *(unsigned int *)wTerm )
        {
            v3 = (__int64)(*wTerm * ta);
            sinReal = waterGlobStatic.sinTable[((v3 & 0x3FF) + 255) & 0x3FF];
            sinImag = waterGlobStatic.sinTable[v3 & 0x3FF];
            if ( (LODWORD(H0->real) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp",
                            180,
                            0,
                            "%s",
                            "!IS_NAN(H0->real)") )
            {
                __debugbreak();
            }
            if ( (LODWORD(H0->imag) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp",
                            181,
                            0,
                            "%s",
                            "!IS_NAN(H0->imag)") )
            {
                __debugbreak();
            }
            if ( (LODWORD(sinReal) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp",
                            182,
                            0,
                            "%s",
                            "!IS_NAN(sinReal)") )
            {
                __debugbreak();
            }
            if ( (LODWORD(sinImag) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp",
                            183,
                            0,
                            "%s",
                            "!IS_NAN(sinImag)") )
            {
                __debugbreak();
            }
            H[vecKIndex].real = H0->real * sinReal;
            H[vecKIndex].imag = H0->imag * sinImag;
            if ( (LODWORD(H[vecKIndex].real) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp",
                            187,
                            0,
                            "%s",
                            "!IS_NAN(H[vecKIndex].real)") )
            {
                __debugbreak();
            }
            if ( (LODWORD(H[vecKIndex].imag) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp",
                            188,
                            0,
                            "%s",
                            "!IS_NAN(H[vecKIndex].imag)") )
            {
                __debugbreak();
            }
        }
        else
        {
            H[vecKIndex].real = 0.0f;
            H[vecKIndex].imag = 0.0f;
        }
        ++vecKIndex;
        ++wTerm;
        ++H0;
    }
}

void __cdecl WaterAmplitudesFromFrequencies(complex_s *H, const water_t *water)
{
    int fftIndex; // [esp+0h] [ebp-Ch]
    int fftIndexa; // [esp+0h] [ebp-Ch]
    unsigned int log2_m; // [esp+4h] [ebp-8h]
    int waterIndex; // [esp+8h] [ebp-4h]
    int waterIndexa; // [esp+8h] [ebp-4h]

    if ( !H && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp", 216, 0, "%s", "H") )
        __debugbreak();
    if ( !water && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp", 217, 0, "%s", "water") )
        __debugbreak();
    if ( water->M != water->N
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp",
                    218,
                    0,
                    "%s",
                    "water->M == water->N") )
    {
        __debugbreak();
    }
    for ( log2_m = 0; water->M != 1 << log2_m; ++log2_m )
        ;
    for ( waterIndex = 0; waterIndex < water->N; ++waterIndex )
    {
        fftIndex = water->M * waterIndex;
        if ( fftIndex < 0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp", 228, 0, "%s", "fftIndex >= 0") )
        {
            __debugbreak();
        }
        if ( fftIndex >= 4096
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp", 229, 0, "%s", "fftIndex < HCOUNT") )
        {
            __debugbreak();
        }
        FFT(&H[fftIndex], log2_m, waterGlobStatic.fftBitswap, waterGlobStatic.fftTrigTable);
    }
    TransposeArray(H, water->M);
    for ( waterIndexa = 0; waterIndexa < water->M; ++waterIndexa )
    {
        fftIndexa = water->N * waterIndexa;
        if ( fftIndexa < 0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp", 240, 0, "%s", "fftIndex >= 0") )
        {
            __debugbreak();
        }
        if ( fftIndexa >= 4096
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp", 241, 0, "%s", "fftIndex < HCOUNT") )
        {
            __debugbreak();
        }
        FFT(&H[fftIndexa], log2_m, waterGlobStatic.fftBitswap, waterGlobStatic.fftTrigTable);
    }
    TransposeArray(H, water->M);
}

void __cdecl TransposeArray(complex_s *H, unsigned int M)
{
    unsigned int v2; // edx
    float real; // ecx
    float imag; // edx
    unsigned int v5; // eax
    unsigned int j; // [esp+4h] [ebp-10h]
    complex_s temp; // [esp+8h] [ebp-Ch]
    unsigned int i; // [esp+10h] [ebp-4h]

    for ( i = 0; i < M; ++i )
    {
        for ( j = 0; j < i; ++j )
        {
            temp = H[j + M * i];
            v2 = i + M * j;
            real = H[v2].real;
            imag = H[v2].imag;
            v5 = j + M * i;
            H[v5].real = real;
            H[v5].imag = imag;
            H[i + M * j] = temp;
        }
    }
}

void __cdecl WaterPixelsFromAmplitudes(GfxColor *pixels, complex_s *H, const water_t *water)
{
    float v3; // [esp+4h] [ebp-58h]
    float v4; // [esp+8h] [ebp-54h]
    float v5; // [esp+Ch] [ebp-50h]
    float v6; // [esp+10h] [ebp-4Ch]
    float height; // [esp+3Ch] [ebp-20h]
    float height_4; // [esp+40h] [ebp-1Ch]
    float height_8; // [esp+44h] [ebp-18h]
    float height_12; // [esp+48h] [ebp-14h]
    unsigned int ixy; // [esp+4Ch] [ebp-10h]
    unsigned int ixya; // [esp+4Ch] [ebp-10h]
    float dz; // [esp+50h] [ebp-Ch]
    GfxColor color; // [esp+54h] [ebp-8h]
    unsigned int count; // [esp+58h] [ebp-4h]

    if ( !pixels && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp", 257, 0, "%s", "pixels") )
        __debugbreak();
    if ( !H && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp", 258, 0, "%s", "H") )
        __debugbreak();
    if ( water->M != water->N
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp",
                    259,
                    0,
                    "%s",
                    "water->M == water->N") )
    {
        __debugbreak();
    }
    count = water->N * water->M;
    if ( (count & 3) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp",
                    262,
                    0,
                    "%s\n\t(count) = %i",
                    "(!(count & 3))",
                    count) )
    {
        __debugbreak();
    }
    dz = 1.0 / (double)count;
    ixy = 0;
    while ( ixy < count )
    {
        height = sqrtf((float)(H[ixy].real * H[ixy].real) + (float)(H[ixy].imag * H[ixy].imag));
        ixya = ixy + 1;
        height_4 = sqrtf((float)(H[ixya].real * H[ixya].real) + (float)(H[ixya].imag * H[ixya].imag));
        ++ixya;
        height_8 = sqrtf((float)(H[ixya].real * H[ixya].real) + (float)(H[ixya].imag * H[ixya].imag));
        height_12 = sqrtf((float)(H[ixya + 1].real * H[ixya + 1].real) + (float)(H[ixya + 1].imag * H[ixya + 1].imag));
        ixy = ixya + 2;
        if ( (float)((float)(height * dz) - 1.0) < 0.0 )
            v6 = height * dz;
        else
            v6 = 1.0f;
        if ( (float)((float)(height_4 * dz) - 1.0) < 0.0 )
            v5 = height_4 * dz;
        else
            v5 = 1.0f;
        if ( (float)((float)(height_8 * dz) - 1.0) < 0.0 )
            v4 = height_8 * dz;
        else
            v4 = 1.0f;
        if ( (float)((float)(height_12 * dz) - 1.0) < 0.0 )
            v3 = height_12 * dz;
        else
            v3 = 1.0f;
        color.array[3] = (int)(float)(255.99899 * v3);
        color.array[0] = (int)(float)(255.99899 * v6);
        color.array[1] = (int)(float)(255.99899 * v5);
        color.array[2] = (int)(float)(255.99899 * v4);
        pixels->packed = color.packed;
        ++pixels;
    }
}

void __cdecl GenerateMipMaps(_D3DFORMAT format, unsigned __int8 *pixels, water_t *water)
{
    int srcWidth; // [esp+0h] [ebp-8h]
    unsigned int mipIndex; // [esp+4h] [ebp-4h]

    if ( !pixels && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp", 589, 0, "%s", "pixels") )
        __debugbreak();
    if ( !water && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp", 590, 0, "%s", "water") )
        __debugbreak();
    if ( water->M != water->N
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp",
                    591,
                    0,
                    "%s",
                    "water->M == water->N") )
    {
        __debugbreak();
    }
    Image_UploadData(water->image, D3DFMT_L8, D3DCUBEMAP_FACE_POSITIVE_X, 0, waterGlob.pixels);
    srcWidth = water->M;
    mipIndex = 1;
    while ( srcWidth > 1 )
    {
        R_DownsampleMipMapBilinear(pixels, srcWidth, srcWidth, 1, pixels);
        Image_UploadData(water->image, format, D3DCUBEMAP_FACE_POSITIVE_X, mipIndex, pixels);
        srcWidth >>= 1;
        ++mipIndex;
    }
}

void __cdecl R_UploadWaterTexture(water_t *water, float floatTime)
{
    if ( !water && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp", 704, 0, "%s", "water") )
        __debugbreak();
    if ( water->writable.floatTime != floatTime )
    {
        water->writable.floatTime = floatTime;
        if ( r_drawWater->current.enabled )
            Sys_AddWorkerCmdInternal(&r_waterWorkerCmd, (unsigned __int8 *)&water, 0);
    }
}

//void __cdecl R_InitWater()
//{
//    double v0; // xmm0_8
//    long double tableIndex; // [esp+0h] [ebp-4h]
//
//    LODWORD(tableIndex) = 0;
//    while ( SLODWORD(tableIndex) < 1024 )
//    {
//        v0 = (float)((float)((float)SLODWORD(tableIndex) * 0.3515625) * 0.017453292);
//        __libm_sse2_sin(tableIndex);
//        *(float *)&v0 = v0;
//        waterGlobStatic.sinTable[LODWORD(tableIndex)++] = *(float *)&v0;
//    }
//    FFT_Init(waterGlobStatic.fftBitswap, waterGlobStatic.fftTrigTable);
//}

// aislop (sin)
void R_InitWater(void)
{
    for (int i = 0; i < 1024; ++i)
    {
        /* degrees -> radians */
        const float angle =
            DEG2RAD((float)i * 0.3515625f);

        waterGlobStatic.sinTable[i] = sinf(angle);
    }

    FFT_Init(
        waterGlobStatic.fftBitswap,
        waterGlobStatic.fftTrigTable
    );
}


void __cdecl Load_PicmipWater(water_t **waterRef)
{
    complex_s *H0; // eax
    float real; // edx
    float imag; // eax
    complex_s *v4; // ecx
    int v5; // [esp+4h] [ebp-2Ch]
    int v6; // [esp+8h] [ebp-28h]
    int m; // [esp+14h] [ebp-1Ch]
    int srcIndex; // [esp+18h] [ebp-18h]
    int downsample; // [esp+1Ch] [ebp-14h]
    int n; // [esp+24h] [ebp-Ch]
    int dstIndex; // [esp+2Ch] [ebp-4h]

    if ( (*waterRef)->M >> r_picmip_water->current.integer < 4 )
        v6 = 4;
    else
        v6 = (*waterRef)->M >> r_picmip_water->current.integer;
    if ( (*waterRef)->N >> r_picmip_water->current.integer < 4 )
        v5 = 4;
    else
        v5 = (*waterRef)->N >> r_picmip_water->current.integer;
    if ( v6 != (*waterRef)->M || v5 != (*waterRef)->N )
    {
        if ( (*waterRef)->M != (*waterRef)->N
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp",
                        754,
                        0,
                        "%s",
                        "(*waterRef)->M == (*waterRef)->N") )
        {
            __debugbreak();
        }
        if ( v6 != v5
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water.cpp", 755, 0, "%s", "M == N") )
        {
            __debugbreak();
        }
        downsample = (*waterRef)->M / v6;
        (*waterRef)->M = v6;
        (*waterRef)->N = v5;
        srcIndex = 0;
        dstIndex = 0;
        for ( m = 0; m < v6; ++m )
        {
            for ( n = 0; n < v5; ++n )
            {
                H0 = (*waterRef)->H0;
                real = H0[srcIndex].real;
                imag = H0[srcIndex].imag;
                v4 = (*waterRef)->H0;
                v4[dstIndex].real = real;
                v4[dstIndex++].imag = imag;
                srcIndex += downsample;
            }
            srcIndex += v6 * downsample * (downsample - 1);
        }
    }
}

