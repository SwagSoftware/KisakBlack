#include "rb_light.h"
#include <qcommon/threads.h>
#include "r_dvars.h"
#include "r_debug.h"
#include <universal/com_memory.h>
#include <cgame_mp/cg_ents_mp.h>
#include <qcommon/com_bsp.h>
#include <universal/com_files.h>
#include <libs/libtomcrypt-1.17/src/headers/tomcrypt.h>

unsigned int lightPointCount;
tlAtomicMutex lightGridMutex;
htab **LightGridHashtable;
htab *LightGridMemory;
unsigned int nextAvailable;
int s_lightGridRowDelta;
int s_lightGridSliceDelta;

void __cdecl R_ShowLightVisCachePoints(const float *viewOrigin, const DpvsPlane *clipPlanes, int clipPlaneCount)
{
    float *color; // [esp+Ch] [ebp-A4h]
    int i; // [esp+7Ch] [ebp-34h]
    int spread; // [esp+80h] [ebp-30h]
    float origin[3]; // [esp+84h] [ebp-2Ch] BYREF
    unsigned int z; // [esp+90h] [ebp-20h]
    unsigned int iz; // [esp+94h] [ebp-1Ch]
    unsigned int iy; // [esp+98h] [ebp-18h]
    int dz; // [esp+9Ch] [ebp-14h]
    unsigned int ix; // [esp+A0h] [ebp-10h]
    unsigned int x; // [esp+A4h] [ebp-Ch]
    unsigned int y; // [esp+A8h] [ebp-8h]
    int dy; // [esp+ACh] [ebp-4h]

    if ( Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_light.cpp",
                    330,
                    0,
                    "%s",
                    "!Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    if ( lightPointCount )
    {
        spread = r_vc_showlog->current.integer;
        if ( spread > 0 )
        {
            x = (int)((float)(*viewOrigin - -131072.0) + 9.313225746154785e-10) >> 5;
            y = (int)((float)(viewOrigin[1] - -131072.0) + 9.313225746154785e-10) >> 5;
            z = (int)((float)(viewOrigin[2] - -131072.0) - 0.4999999990686774) >> 6;
            origin[0] = (double)x * 32.0 + -131072.0;
            origin[1] = (double)y * 32.0 + -131072.0;
            origin[2] = (double)z * 64.0 + -131072.0;
            for ( dz = -1; dz <= 1; ++dz )
            {
                iz = dz + z;
                if ( dz + z <= 0x1000 )
                {
                    for ( dy = -spread; dy <= spread; ++dy )
                    {
                        iy = dy + y;
                        if ( dy + y <= 0x2000 )
                        {
                            for ( i = -spread; i <= spread; ++i )
                            {
                                ix = i + x;
                                if ( i + x <= 0x2000 )
                                {
                                    origin[0] = (double)ix * 32.0 + -131072.0;
                                    origin[1] = (double)iy * 32.0 + -131072.0;
                                    origin[2] = (double)iz * 64.0 + -131072.0;
                                    if ( !R_CullPointAndRadius(origin, 0.0, clipPlanes, clipPlaneCount)
                                        && !R_SortedHistoryEntry(ix, iy, iz, SH_ADD_NEVER) )
                                    {
                                        if ( (unsigned int)dz < 2 )
                                            color = (float *)colorGreen;
                                        else
                                            color = (float *)colorYellow;
                                        R_AddDebugString(&frontEndDataOut->debugGlobals, origin, color, 1.0, (char*)".");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

bool __cdecl R_SortedHistoryEntry(
                unsigned __int16 x,
                unsigned __int16 y,
                unsigned __int16 z,
                GfxSortedHistoryAdd addMode)
{
    volatile unsigned int Target[5]; // [esp+0h] [ebp-2Ch] BYREF
    htab *added; // [esp+14h] [ebp-18h]
    bool result; // [esp+1Bh] [ebp-11h]
    unsigned __int16 gsp[3]; // [esp+1Ch] [ebp-10h] BYREF

    result = 0;
    gsp[0] = x;
    gsp[1] = y;
    gsp[2] = z;
    //tlAtomicMutex::Lock(&lightGridMutex);
    lightGridMutex.Lock();
    if ( findhash((const unsigned __int16 (*)[3])gsp) )
    {
        result = 1;
    }
    else if ( addMode == SH_ADD_IF_NEW && lightPointCount < 0x100000 )
    {
        added = addHash((const unsigned __int16 (*)[3])gsp);
        result = added != 0;
    }
    if ( !--lightGridMutex.LockCount )
    {
        Target[0] = 0;
        InterlockedExchange(Target, 0);
        lightGridMutex.ThreadId = 0;
    }
    return result;
}

htab *__cdecl addHash(const unsigned __int16 (*data)[3])
{
    unsigned __int64 v2; // rax
    htab *curhash; // [esp+0h] [ebp-14h]
    htab *newhash; // [esp+4h] [ebp-10h]
    unsigned int hashkey; // [esp+8h] [ebp-Ch]

    newhash = htabAlloc();
    if ( !newhash )
        return 0;
    hashkey = HashXyz(data);
    if ( hashkey >= 0x100007
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_light.cpp",
                    244,
                    0,
                    "%s",
                    "hashkey < HASHTABLE_SIZE") )
    {
        __debugbreak();
    }
    LODWORD(v2) = HashPackPoint(data);
    newhash->value = v2;
    newhash->next = 0;
    curhash = LightGridHashtable[hashkey];
    if ( curhash )
    {
        while ( curhash->next )
            curhash = curhash->next;
        curhash->next = newhash;
    }
    else
    {
        LightGridHashtable[hashkey] = newhash;
    }
    ++lightPointCount;
    return newhash;
}

htab *__cdecl htabAlloc()
{
    if ( !LightGridMemory )
        return 0;
    if ( nextAvailable >= 0x100000 )
        return 0;
    return &LightGridMemory[nextAvailable++];
}

unsigned __int64 __cdecl HashXyz(const unsigned __int16 (*p)[3])
{
    return (15473 * (unsigned __int16)(*p)[2] + 25031 * (unsigned __int16)(*p)[1] + 40499 * (*p)[0]) % 0x100007uLL;
}

int __cdecl HashPackPoint(const unsigned __int16 (*p)[3])
{
    return *(unsigned int *)p;
}

htab *__cdecl findhash(const unsigned __int16 (*data)[3])
{
    __int64 v2; // rax
    htab *curhash; // [esp+4h] [ebp-10h]
    unsigned int hashkey; // [esp+8h] [ebp-Ch]

    if ( !LightGridMemory || !LightGridHashtable )
        return 0;
    hashkey = HashXyz(data);
    if ( hashkey >= 0x100007
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_light.cpp",
                    272,
                    0,
                    "%s",
                    "hashkey < HASHTABLE_SIZE") )
    {
        __debugbreak();
    }
    LODWORD(v2) = HashPackPoint(data);
    for ( curhash = LightGridHashtable[hashkey]; curhash; curhash = curhash->next )
    {
        if ( curhash->value == v2 )
            return curhash;
    }
    return 0;
}

void __cdecl R_ApplyLightGridColorsPatch(const GfxModelLightingPatch *patch, unsigned __int8 *pixels)
{
    float *v2; // [esp+8h] [ebp-798h]
    int j; // [esp+90h] [ebp-710h]
    unsigned int i; // [esp+94h] [ebp-70Ch]
    GfxDecodedLightGridColors decodedColors; // [esp+98h] [ebp-708h] BYREF
    const float *heroPos; // [esp+41Ch] [ebp-384h]
    GfxDecodedLightGridColors accumulatedColors; // [esp+420h] [ebp-380h] BYREF

    if ( patch->useHeroLighting )
        v2 = (float*)patch->heroPos;
    else
        v2 = 0;
    heroPos = v2;
    if ( patch->colorsCount == 1 )
    {
        R_DecodeLightGridColors(&rgp.world->lightGrid.colors[patch->colorsIndex[0]], &decodedColors);
        R_AddHeroOnlyLightsToGridColors(&decodedColors, heroPos);
        R_SetLightGridColorsVec4(&decodedColors, patch->primaryLightWeight, pixels);
    }
    else
    {
        R_DecodeLightGridColorsWeighted(
            &rgp.world->lightGrid.colors[patch->colorsIndex[0]],
            &accumulatedColors,
            patch->colorsWeight[0]);
        for ( i = 1; i < patch->colorsCount; ++i )
        {
            R_DecodeLightGridColorsWeighted(
                &rgp.world->lightGrid.colors[patch->colorsIndex[i]],
                &decodedColors,
                patch->colorsWeight[i]);
            for ( j = 0; j < 56; ++j )
            {
                accumulatedColors.rgb[j][0] = accumulatedColors.rgb[j][0] + decodedColors.rgb[j][0];
                accumulatedColors.rgb[j][1] = accumulatedColors.rgb[j][1] + decodedColors.rgb[j][1];
                accumulatedColors.rgb[j][2] = accumulatedColors.rgb[j][2] + decodedColors.rgb[j][2];
            }
        }
        R_AddHeroOnlyLightsToGridColors(&accumulatedColors, heroPos);
        R_SetLightGridColorsVec4(&accumulatedColors, patch->primaryLightWeight, pixels);
    }
}

void __cdecl R_SetLightGridColorsVec4(
                const GfxDecodedLightGridColors *colors,
                float primaryWeight,
                unsigned __int8 *pixels)
{
    int i; // [esp+16Ch] [ebp-14h]
    float ambient[4]; // [esp+170h] [ebp-10h] BYREF
    unsigned __int8 *pixelsa; // [esp+190h] [ebp+10h]
    unsigned __int8 *pixelsb; // [esp+190h] [ebp+10h]
    unsigned __int8 *pixelsc; // [esp+190h] [ebp+10h]
    unsigned __int8 *pixelsd; // [esp+190h] [ebp+10h]
    unsigned __int8 *pixelse; // [esp+190h] [ebp+10h]
    unsigned __int8 *pixelsf; // [esp+190h] [ebp+10h]
    unsigned __int8 *pixelsg; // [esp+190h] [ebp+10h]
    unsigned __int8 *pixelsh; // [esp+190h] [ebp+10h]
    unsigned __int8 *pixelsi; // [esp+190h] [ebp+10h]
    unsigned __int8 *pixelsj; // [esp+190h] [ebp+10h]
    unsigned __int8 *pixelsk; // [esp+190h] [ebp+10h]
    unsigned __int8 *pixelsl; // [esp+190h] [ebp+10h]
    unsigned __int8 *pixelsm; // [esp+190h] [ebp+10h]
    unsigned __int8 *pixelsn; // [esp+190h] [ebp+10h]
    unsigned __int8 *pixelso; // [esp+190h] [ebp+10h]

    memset(ambient, 0, sizeof(ambient));
    for ( i = 0; i < 56; ++i )
    {
        ambient[0] = ambient[0] + colors->rgb[i][0];
        ambient[1] = ambient[1] + colors->rgb[i][1];
        ambient[2] = ambient[2] + colors->rgb[i][2];
    }
    ambient[0] = ambient[0] / 56.0;
    ambient[1] = ambient[1] / 56.0;
    ambient[2] = ambient[2] / 56.0;
    SetPixelColorLightGridVec4_PC(pixels, (const float *)colors, primaryWeight);
    SetPixelColorLightGridVec4_PC(pixels + 4, colors->rgb[1], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixels + 8, colors->rgb[2], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixels + 12, colors->rgb[3], primaryWeight);
    pixelsa = &pixels[s_lightGridRowDelta];
    SetPixelColorLightGridVec4_PC(pixelsa, colors->rgb[4], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsa + 4, colors->rgb[5], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsa + 8, colors->rgb[6], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsa + 12, colors->rgb[7], primaryWeight);
    pixelsb = &pixelsa[s_lightGridRowDelta];
    SetPixelColorLightGridVec4_PC(pixelsb, colors->rgb[8], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsb + 4, colors->rgb[9], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsb + 8, colors->rgb[10], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsb + 12, colors->rgb[11], primaryWeight);
    pixelsc = &pixelsb[s_lightGridRowDelta];
    SetPixelColorLightGridVec4_PC(pixelsc, colors->rgb[12], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsc + 4, colors->rgb[13], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsc + 8, colors->rgb[14], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsc + 12, colors->rgb[15], primaryWeight);
    pixelsd = &pixelsc[s_lightGridSliceDelta];
    SetPixelColorLightGridVec4_PC(pixelsd, colors->rgb[16], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsd + 4, colors->rgb[17], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsd + 8, colors->rgb[18], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsd + 12, colors->rgb[19], primaryWeight);
    pixelse = &pixelsd[s_lightGridRowDelta];
    SetPixelColorLightGridVec4_PC(pixelse, colors->rgb[20], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelse + 4, ambient, primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelse + 8, ambient, primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelse + 12, colors->rgb[21], primaryWeight);
    pixelsf = &pixelse[s_lightGridRowDelta];
    SetPixelColorLightGridVec4_PC(pixelsf, colors->rgb[22], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsf + 4, ambient, primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsf + 8, ambient, primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsf + 12, colors->rgb[23], primaryWeight);
    pixelsg = &pixelsf[s_lightGridRowDelta];
    SetPixelColorLightGridVec4_PC(pixelsg, colors->rgb[24], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsg + 4, colors->rgb[25], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsg + 8, colors->rgb[26], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsg + 12, colors->rgb[27], primaryWeight);
    pixelsh = &pixelsg[s_lightGridSliceDelta];
    SetPixelColorLightGridVec4_PC(pixelsh, colors->rgb[28], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsh + 4, colors->rgb[29], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsh + 8, colors->rgb[30], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsh + 12, colors->rgb[31], primaryWeight);
    pixelsi = &pixelsh[s_lightGridRowDelta];
    SetPixelColorLightGridVec4_PC(pixelsi, colors->rgb[32], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsi + 4, ambient, primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsi + 8, ambient, primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsi + 12, colors->rgb[33], primaryWeight);
    pixelsj = &pixelsi[s_lightGridRowDelta];
    SetPixelColorLightGridVec4_PC(pixelsj, colors->rgb[34], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsj + 4, ambient, primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsj + 8, ambient, primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsj + 12, colors->rgb[35], primaryWeight);
    pixelsk = &pixelsj[s_lightGridRowDelta];
    SetPixelColorLightGridVec4_PC(pixelsk, colors->rgb[36], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsk + 4, colors->rgb[37], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsk + 8, colors->rgb[38], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsk + 12, colors->rgb[39], primaryWeight);
    pixelsl = &pixelsk[s_lightGridSliceDelta];
    SetPixelColorLightGridVec4_PC(pixelsl, colors->rgb[40], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsl + 4, colors->rgb[41], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsl + 8, colors->rgb[42], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsl + 12, colors->rgb[43], primaryWeight);
    pixelsm = &pixelsl[s_lightGridRowDelta];
    SetPixelColorLightGridVec4_PC(pixelsm, colors->rgb[44], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsm + 4, colors->rgb[45], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsm + 8, colors->rgb[46], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsm + 12, colors->rgb[47], primaryWeight);
    pixelsn = &pixelsm[s_lightGridRowDelta];
    SetPixelColorLightGridVec4_PC(pixelsn, colors->rgb[48], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsn + 4, colors->rgb[49], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsn + 8, colors->rgb[50], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelsn + 12, colors->rgb[51], primaryWeight);
    pixelso = &pixelsn[s_lightGridRowDelta];
    SetPixelColorLightGridVec4_PC(pixelso, colors->rgb[52], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelso + 4, colors->rgb[53], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelso + 8, colors->rgb[54], primaryWeight);
    SetPixelColorLightGridVec4_PC(pixelso + 12, colors->rgb[55], primaryWeight);
}

void __cdecl SetPixelColorLightGridVec4_PC(unsigned __int8 *v, const float *color, float primaryWeight)
{
    float v3; // [esp+0h] [ebp-24h]
    float v4; // [esp+4h] [ebp-20h]
    float v5; // [esp+8h] [ebp-1Ch]

    if ( (float)(*color / 31.875) <= 1.0 )
        v5 = *color / 31.875;
    else
        v5 = 1.0f;
    if ( (float)(color[1] / 31.875) <= 1.0 )
        v4 = color[1] / 31.875;
    else
        v4 = 1.0f;
    if ( (float)(color[2] / 31.875) <= 1.0 )
        v3 = color[2] / 31.875;
    else
        v3 = 1.0f;
    *(unsigned int *)v = ((unsigned __int8)(int)(float)(primaryWeight * 255.0) << 24)
                             | (unsigned __int8)(int)(float)(sqrtf(v3) * 255.0)
                             | ((unsigned __int8)(int)(float)(sqrtf(v4) * 255.0) << 8)
                             | ((unsigned __int8)(int)(float)(sqrtf(v5) * 255.0) << 16);
}

void __cdecl R_DecodeLightGridColors(
                const GfxCompressedLightGridColors *colors,
                GfxDecodedLightGridColors *decodedColors)
{
    float Y; // [esp+1Ch] [ebp-28h]
    float Ya; // [esp+1Ch] [ebp-28h]
    unsigned int packed; // [esp+20h] [ebp-24h]
    float u; // [esp+30h] [ebp-14h]
    float ua; // [esp+30h] [ebp-14h]
    unsigned int x; // [esp+34h] [ebp-10h]
    float w; // [esp+3Ch] [ebp-8h]
    float wa; // [esp+3Ch] [ebp-8h]
    int i; // [esp+40h] [ebp-4h]

    for ( i = 0; i < 56; ++i )
    {
        x = colors->rgb[i][0];
        packed = (colors->rgb[i][2] << 16) | x | (colors->rgb[i][1] << 8);
        Y = (float)((packed >> 12) & 0xFFFLL);
        Ya = (float)((float)(Y / 4095.0) * (float)(Y / 4095.0)) * 31.875;
        u = (float)((packed >> 6) & 0x3FLL);
        w = (float)(x & 0x3F);
        ua = u / 63.0;
        wa = w / 63.0;
        decodedColors->rgb[i][0] = (float)(ua * Ya) / 0.25;
        decodedColors->rgb[i][1] = (float)((float)(Ya - (float)((float)((float)(ua * Ya) / 0.25) * 0.25))
                                                                         - (float)((float)((float)(wa * Ya) / 0.25) * 0.25))
                                                         / 0.5;
        decodedColors->rgb[i][2] = (float)(wa * Ya) / 0.25;
    }
}

void __cdecl R_DecodeLightGridColorsWeighted(
                const GfxCompressedLightGridColors *colors,
                GfxDecodedLightGridColors *decodedColors,
                float weight)
{
    float Y; // [esp+1Ch] [ebp-28h]
    float Ya; // [esp+1Ch] [ebp-28h]
    unsigned int packed; // [esp+20h] [ebp-24h]
    float u; // [esp+30h] [ebp-14h]
    float ua; // [esp+30h] [ebp-14h]
    unsigned int x; // [esp+34h] [ebp-10h]
    float w; // [esp+3Ch] [ebp-8h]
    float wa; // [esp+3Ch] [ebp-8h]
    int i; // [esp+40h] [ebp-4h]

    for ( i = 0; i < 56; ++i )
    {
        x = colors->rgb[i][0];
        packed = (colors->rgb[i][2] << 16) | x | (colors->rgb[i][1] << 8);
        Y = (float)((packed >> 12) & 0xFFFLL);
        Ya = (float)((float)(Y / 4095.0) * (float)(Y / 4095.0)) * 31.875;
        u = (float)((packed >> 6) & 0x3FLL);
        w = (float)(x & 0x3F);
        ua = u / 63.0;
        wa = w / 63.0;
        decodedColors->rgb[i][0] = (float)((float)(ua * Ya) / 0.25) * weight;
        decodedColors->rgb[i][1] = (float)((float)((float)(Ya - (float)((float)((float)(ua * Ya) / 0.25) * 0.25))
                                                                                         - (float)((float)((float)(wa * Ya) / 0.25) * 0.25))
                                                                         / 0.5)
                                                         * weight;
        decodedColors->rgb[i][2] = (float)((float)(wa * Ya) / 0.25) * weight;
    }
}

unsigned __int8 __cdecl R_GetPrimaryLightFromGrid(
                const GfxLightGrid *lightGrid,
                const float *samplePos,
                unsigned __int8 sunPrimaryLightIndex)
{
    float cornerWeight[8]; // [esp+0h] [ebp-50h] BYREF
    const GfxLightGridEntry *entry; // [esp+20h] [ebp-30h]
    unsigned int cornerIndex; // [esp+24h] [ebp-2Ch]
    unsigned __int8 primaryLightIndex; // [esp+2Bh] [ebp-25h]
    const GfxLightGridEntry *cornerEntry[8]; // [esp+2Ch] [ebp-24h] BYREF
    unsigned int defaultGridEntry; // [esp+4Ch] [ebp-4h] BYREF

    if ( !lightGrid
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_light.cpp", 2037, 0, "%s", "lightGrid") )
    {
        __debugbreak();
    }
    primaryLightIndex = R_LightGridLookup(lightGrid, samplePos, cornerWeight, cornerEntry, &defaultGridEntry);
    if ( primaryLightIndex == 255 )
        primaryLightIndex = sunPrimaryLightIndex;
    for ( cornerIndex = 0; cornerIndex < 8; ++cornerIndex )
    {
        entry = cornerEntry[cornerIndex];
        if ( entry
            && entry->primaryLightIndex != 255
            && (!entry->primaryLightIndex || entry->primaryLightIndex == primaryLightIndex) )
        {
            return primaryLightIndex;
        }
    }
    return sunPrimaryLightIndex;
}

unsigned __int8 __cdecl R_LightGridLookup(
    const GfxLightGrid *lightGrid,
    const float *samplePos,
    float *cornerWeight,
    const GfxLightGridEntry **cornerEntry,
    unsigned int *defaultGridEntry)
{
    float v6; // [esp+20h] [ebp-68h]
    float v7; // [esp+24h] [ebp-64h]
    float v8; // [esp+28h] [ebp-60h]
    bool v9; // [esp+2Eh] [ebp-5Ah]
    unsigned __int8 v10; // [esp+2Fh] [ebp-59h]
    GfxLightGridEntry localEntry; // [esp+3Ch] [ebp-4Ch] BYREF
    unsigned int pos[3]; // [esp+40h] [ebp-48h] BYREF
    bool honorSuppression; // [esp+4Fh] [ebp-39h]
    const GfxLightGridEntry *entry; // [esp+50h] [ebp-38h]
    unsigned int cornerTraceBit; // [esp+54h] [ebp-34h]
    unsigned int cornerIndex; // [esp+58h] [ebp-30h]
    float axisLerp[3]; // [esp+5Ch] [ebp-2Ch]
    unsigned __int8 primaryLightIndex; // [esp+6Bh] [ebp-1Dh]
    const GfxLightGridEntry *remoteEntry; // [esp+6Ch] [ebp-1Ch]
    float bestPrimaryLightWeight; // [esp+70h] [ebp-18h]
    bool suppressEntry; // [esp+77h] [ebp-11h]
    float quadWeight; // [esp+78h] [ebp-10h]
    bool suppressEntryLog[8]; // [esp+7Ch] [ebp-Ch] BYREF

    v8 = floor(*samplePos);
    pos[0] = ((int)v8 + 0x20000) >> 5;
    v7 = floor(samplePos[1]);
    pos[1] = ((int)v7 + 0x20000) >> 5;
    v6 = floor(samplePos[2]);
    pos[2] = ((int)v6 + 0x20000) >> 6;
    if ((lightGrid->rowAxis || lightGrid->colAxis != 1)
        && (lightGrid->rowAxis != 1 || lightGrid->colAxis)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_light.cpp",
            1934,
            1,
            "%s",
            "(lightGrid->rowAxis == 0 && lightGrid->colAxis == 1) || (lightGrid->rowAxis == 1 && lightGrid->colAxis == 0)"))
    {
        __debugbreak();
    }
    axisLerp[0] = (samplePos[lightGrid->rowAxis] - -131072.0) * 0.03125 - (double)pos[lightGrid->rowAxis];
    axisLerp[1] = (samplePos[lightGrid->colAxis] - -131072.0) * 0.03125 - (double)pos[lightGrid->colAxis];
    axisLerp[2] = (samplePos[2] - -131072.0) * 0.015625 - (double)pos[2];
    quadWeight = (float)(1.0 - axisLerp[1]) * (float)(1.0 - axisLerp[2]);
    *cornerWeight = (float)(1.0 - axisLerp[0]) * quadWeight;
    cornerWeight[4] = quadWeight * axisLerp[0];
    quadWeight = (float)(1.0 - axisLerp[1]) * axisLerp[2];
    cornerWeight[1] = (float)(1.0 - axisLerp[0]) * quadWeight;
    cornerWeight[5] = quadWeight * axisLerp[0];
    quadWeight = (float)(1.0 - axisLerp[2]) * axisLerp[1];
    cornerWeight[2] = (float)(1.0 - axisLerp[0]) * quadWeight;
    cornerWeight[6] = quadWeight * axisLerp[0];
    quadWeight = axisLerp[1] * axisLerp[2];
    cornerWeight[3] = (float)(1.0 - axisLerp[0]) * (float)(axisLerp[1] * axisLerp[2]);
    cornerWeight[7] = quadWeight * axisLerp[0];
    *defaultGridEntry = 1;
    R_GetLightGridSampleEntryQuad(lightGrid, pos, cornerEntry, defaultGridEntry);
    ++pos[lightGrid->rowAxis];
    R_GetLightGridSampleEntryQuad(lightGrid, pos, cornerEntry + 4, defaultGridEntry);
    --pos[lightGrid->rowAxis];
    if (r_vc_makelog->current.integer)
        R_UpdateVisHistory(lightGrid, pos);
    primaryLightIndex = 0;
    bestPrimaryLightWeight = 0.0f;
    honorSuppression = 0;
    cornerIndex = 0;
    cornerTraceBit = 1;
    while (cornerIndex < 8)
    {
        remoteEntry = cornerEntry[cornerIndex];
        if (!remoteEntry)
            goto LABEL_10;
        if (cornerWeight[cornerIndex] < 0.001)
        {
            cornerEntry[cornerIndex] = 0;
            goto LABEL_10;
        }
        localEntry = *remoteEntry;
        entry = &localEntry;
        suppressEntry = !R_IsValidLightGridSample(lightGrid, &localEntry, cornerIndex, pos, samplePos);
        suppressEntryLog[cornerIndex] = suppressEntry;
        if (suppressEntry)
        {
            if (honorSuppression)
            {
                cornerEntry[cornerIndex] = 0;
                goto LABEL_10;
            }
        }
        else if (!honorSuppression)
        {
            honorSuppression = 1;
            bestPrimaryLightWeight = cornerWeight[cornerIndex];
            primaryLightIndex = entry->primaryLightIndex;
            memset(cornerEntry, 0, 4 * cornerIndex);
            goto LABEL_10;
        }
        v10 = entry->primaryLightIndex;
        if (primaryLightIndex)
        {
            if (v10)
                v9 = primaryLightIndex == 255 || v10 != 255 && cornerWeight[cornerIndex] > bestPrimaryLightWeight;
            else
                v9 = 0;
        }
        else
        {
            v9 = 1;
        }
        if (v9)
        {
            bestPrimaryLightWeight = cornerWeight[cornerIndex];
            primaryLightIndex = entry->primaryLightIndex;
        }
    LABEL_10:
        ++cornerIndex;
        cornerTraceBit *= 2;
    }
    if (r_showLightGrid->current.enabled)
        R_ShowLightGrid(lightGrid, pos, samplePos, cornerEntry, suppressEntryLog, honorSuppression);
    return primaryLightIndex;
}

void __cdecl R_ShowLightGrid(
                const GfxLightGrid *lightGrid,
                const unsigned int *pos,
                const float *samplePos,
                const GfxLightGridEntry **cornerEntry,
                bool *suppressEntry,
                bool honorSuppression)
{
    unsigned int yBit; // [esp+Ch] [ebp-28h]
    float samplePos2Eye_4; // [esp+14h] [ebp-20h]
    float samplePos2Eye_8; // [esp+18h] [ebp-1Ch]
    unsigned int z; // [esp+1Ch] [ebp-18h]
    unsigned int cornerIndex; // [esp+20h] [ebp-14h]
    unsigned int xBit; // [esp+28h] [ebp-Ch]
    unsigned int x; // [esp+2Ch] [ebp-8h]
    unsigned int y; // [esp+30h] [ebp-4h]

    samplePos2Eye_4 = frontEndDataOut->viewParms[0].origin[1] - samplePos[1];
    samplePos2Eye_8 = frontEndDataOut->viewParms[0].origin[2] - samplePos[2];
    if ( (float)((float)((float)((float)(frontEndDataOut->viewParms[0].origin[0] - *samplePos)
                                                         * (float)(frontEndDataOut->viewParms[0].origin[0] - *samplePos))
                                         + (float)(samplePos2Eye_4 * samplePos2Eye_4))
                         + (float)(samplePos2Eye_8 * samplePos2Eye_8)) <= 810000.0 )
    {
        R_ShowGridOrigin(samplePos);
        R_ShowGridBox(pos);
        if ( !rgp.world
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_light.cpp", 139, 0, "%s", "rgp.world") )
        {
            __debugbreak();
        }
        if ( lightGrid->rowAxis )
        {
            xBit = 2;
            yBit = 4;
        }
        else
        {
            xBit = 4;
            yBit = 2;
        }
        for ( cornerIndex = 0; cornerIndex < 8; ++cornerIndex )
        {
            if ( cornerEntry[cornerIndex] )
            {
                x = *pos + ((xBit & cornerIndex) != 0);
                y = pos[1] + ((yBit & cornerIndex) != 0);
                z = pos[2] + ((cornerIndex & 1) != 0);
                if ( suppressEntry[cornerIndex] && honorSuppression )
                {
                    R_ShowGridCorner(x, y, z, 1.0, colorYellow);
                }
                else if ( !suppressEntry[cornerIndex] || honorSuppression )
                {
                    R_ShowGridCorner(x, y, z, 0.89999998, colorGreen);
                }
                else
                {
                    R_ShowGridCorner(x, y, z, 1.1, colorRed);
                }
            }
        }
    }
}

void __cdecl R_ShowGridOrigin(const float *origin)
{
    float boxMaxs[3]; // [esp+Ch] [ebp-18h] BYREF
    float boxMins[3]; // [esp+18h] [ebp-Ch] BYREF

    boxMins[0] = *origin - 0.5;
    boxMins[1] = origin[1] - 0.5;
    boxMins[2] = origin[2] - 0.5;
    boxMaxs[0] = *origin + 0.5;
    boxMaxs[1] = origin[1] + 0.5;
    boxMaxs[2] = origin[2] + 0.5;
    R_AddDebugBox(&frontEndDataOut->debugGlobals, boxMins, boxMaxs, colorBlue);
}

void __cdecl R_ShowGridBox(const unsigned int *pos)
{
    float origin; // [esp+18h] [ebp-24h]
    float origin_4; // [esp+1Ch] [ebp-20h]
    float origin_8; // [esp+20h] [ebp-1Ch]
    float boxMaxs[3]; // [esp+24h] [ebp-18h] BYREF
    float boxMins[3]; // [esp+30h] [ebp-Ch] BYREF

    origin = (double)*pos * 32.0 + -131072.0;
    origin_4 = (double)pos[1] * 32.0 + -131072.0;
    origin_8 = (double)pos[2] * 64.0 + -131072.0;
    boxMins[0] = origin;
    boxMins[1] = origin_4;
    boxMins[2] = origin_8;
    boxMaxs[0] = origin + 32.0;
    boxMaxs[1] = origin_4 + 32.0;
    boxMaxs[2] = origin_8 + 64.0;
    R_AddDebugBox(&frontEndDataOut->debugGlobals, boxMins, boxMaxs, colorWhite);
}

void __cdecl R_ShowGridCorner(unsigned int x, unsigned int y, unsigned int z, float halfSize, const float *color)
{
    float origin; // [esp+18h] [ebp-24h]
    float origin_4; // [esp+1Ch] [ebp-20h]
    float origin_8; // [esp+20h] [ebp-1Ch]
    float boxMaxs[3]; // [esp+24h] [ebp-18h] BYREF
    float boxMins[3]; // [esp+30h] [ebp-Ch] BYREF

    origin = (double)x * 32.0 + -131072.0;
    origin_4 = (double)y * 32.0 + -131072.0;
    origin_8 = (double)z * 64.0 + -131072.0;
    boxMins[0] = origin - halfSize;
    boxMins[1] = origin_4 - halfSize;
    boxMins[2] = origin_8 - halfSize;
    boxMaxs[0] = origin + halfSize;
    boxMaxs[1] = origin_4 + halfSize;
    boxMaxs[2] = origin_8 + halfSize;
    R_AddDebugBox(&frontEndDataOut->debugGlobals, boxMins, boxMaxs, color);
}

void __cdecl R_UpdateVisHistory(const GfxLightGrid *lightGrid, const unsigned int *pos)
{
    unsigned int yBit; // [esp+0h] [ebp-1Ch]
    unsigned int cornerIndex; // [esp+8h] [ebp-14h]
    unsigned int xBit; // [esp+10h] [ebp-Ch]

    if ( !LightGridHashtable || !LightGridMemory )
        AllocAllMemoryNeeded();
    if ( lightGrid->rowAxis )
    {
        xBit = 2;
        yBit = 4;
    }
    else
    {
        xBit = 4;
        yBit = 2;
    }
    for ( cornerIndex = 0; cornerIndex < 8; ++cornerIndex )
        R_SortedHistoryEntry(
            *(_WORD *)pos + ((xBit & cornerIndex) != 0),
            *((_WORD *)pos + 2) + ((yBit & cornerIndex) != 0),
            *((_WORD *)pos + 4) + ((cornerIndex & 1) != 0),
            SH_ADD_IF_NEW);
}

void AllocAllMemoryNeeded()
{
    if (!LightGridHashtable)
        LightGridHashtable = (htab **)Z_VirtualAlloc(0x40001C, "initLightGridPoints", 0);
    if (!LightGridMemory)
        LightGridMemory = (htab *)Z_VirtualAlloc(0x1000000, "initLightGridMemory", 0);
}

void __cdecl R_GetLightGridSampleEntryQuad(
                const GfxLightGrid *lightGrid,
                const unsigned int *pos,
                const GfxLightGridEntry **entries,
                unsigned int *defaultGridEntry)
{
    const GfxLightGridEntry *v4; // [esp+0h] [ebp-68h]
    const GfxLightGridEntry *v5; // [esp+4h] [ebp-64h]
    int v6; // [esp+8h] [ebp-60h]
    const GfxLightGridEntry *v7; // [esp+Ch] [ebp-5Ch]
    const GfxLightGridEntry *v8; // [esp+10h] [ebp-58h]
    const GfxLightGridEntry *v9; // [esp+14h] [ebp-54h]
    const GfxLightGridEntry *v10; // [esp+18h] [ebp-50h]
    int v11; // [esp+1Ch] [ebp-4Ch]
    const GfxLightGridEntry *v12; // [esp+20h] [ebp-48h]
    const GfxLightGridEntry *v13; // [esp+24h] [ebp-44h]
    unsigned __int16 v14; // [esp+28h] [ebp-40h]
    unsigned int lookup; // [esp+2Ch] [ebp-3Ch]
    unsigned int lookupa; // [esp+2Ch] [ebp-3Ch]
    unsigned int lookupb; // [esp+2Ch] [ebp-3Ch]
    unsigned int lookupc; // [esp+2Ch] [ebp-3Ch]
    unsigned int firstBlockEntry; // [esp+30h] [ebp-38h]
    unsigned int firstBlockEntrya; // [esp+30h] [ebp-38h]
    unsigned int z; // [esp+38h] [ebp-30h]
    unsigned int localZ; // [esp+40h] [ebp-28h]
    const unsigned __int8 *rleData; // [esp+48h] [ebp-20h]
    const unsigned __int8 *rleDataa; // [esp+48h] [ebp-20h]
    const GfxLightGridRow *remoteRow; // [esp+4Ch] [ebp-1Ch]
    unsigned int rleSizeFull; // [esp+50h] [ebp-18h]
    unsigned int colIndex; // [esp+54h] [ebp-14h]
    const unsigned __int8 *remote_rleData; // [esp+58h] [ebp-10h]
    unsigned int rowIndex; // [esp+60h] [ebp-8h]
    unsigned int baseZ; // [esp+64h] [ebp-4h]
    unsigned int baseZa; // [esp+64h] [ebp-4h]
    unsigned int baseZb; // [esp+64h] [ebp-4h]
    unsigned int baseZc; // [esp+64h] [ebp-4h]

    rowIndex = pos[lightGrid->rowAxis] - lightGrid->mins[lightGrid->rowAxis];
    if ( rowIndex >= lightGrid->maxs[lightGrid->rowAxis] + 1 - (unsigned int)lightGrid->mins[lightGrid->rowAxis] )
        v14 = -1;
    else
        v14 = lightGrid->rowDataStart[rowIndex];
    if ( v14 == 0xFFFF )
    {
        *entries = 0;
        entries[1] = 0;
        entries[2] = 0;
        entries[3] = 0;
        return;
    }
    if ( 4 * (unsigned int)v14 >= lightGrid->rawRowDataSize
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_light.cpp",
                    1694,
                    0,
                    "rowDataStart * 4 doesn't index lightGrid->rawRowDataSize\n\t%i not in [0, %i)",
                    4 * v14,
                    lightGrid->rawRowDataSize) )
    {
        __debugbreak();
    }
    remoteRow = (const GfxLightGridRow *)&lightGrid->rawRowData[4 * v14];
    if ( remoteRow->firstEntry >= lightGrid->entryCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_light.cpp",
                    1699,
                    0,
                    "row->firstEntry doesn't index lightGrid->entryCount\n\t%i not in [0, %i)",
                    remoteRow->firstEntry,
                    lightGrid->entryCount) )
    {
        __debugbreak();
    }
    colIndex = pos[lightGrid->colAxis] - remoteRow->colStart;
    z = pos[2] - remoteRow->zStart;
    if ( colIndex + 1 > remoteRow->colCount )
    {
        *entries = 0;
        entries[1] = 0;
        entries[2] = 0;
        entries[3] = 0;
        return;
    }
    if ( z + 1 > remoteRow->zCount )
    {
        *entries = 0;
        entries[1] = 0;
        entries[2] = 0;
        entries[3] = 0;
        if ( pos[2] < remoteRow->zStart )
            *defaultGridEntry = 0;
        return;
    }
    firstBlockEntry = remoteRow->firstEntry;
    remote_rleData = (const unsigned __int8 *)&remoteRow[1];
    rleSizeFull = (remoteRow->zCount > 0xFFu) + 3;
    rleData = (const unsigned __int8 *)&remoteRow[1];
    if ( colIndex == -1 )
    {
        *entries = 0;
        entries[1] = 0;
        baseZ = LOBYTE(remoteRow[1].colCount);
        if ( remoteRow->zCount > 0xFFu )
            baseZ += HIBYTE(remoteRow[1].colCount) << 8;
        lookup = z - baseZ + firstBlockEntry;
        if ( z - baseZ < HIBYTE(remoteRow[1].colStart) )
            v13 = &lightGrid->entries[lookup];
        else
            v13 = 0;
        entries[2] = v13;
        if ( z - baseZ + 1 < HIBYTE(remoteRow[1].colStart) )
            v12 = &lightGrid->entries[lookup + 1];
        else
            v12 = 0;
        entries[3] = v12;
        if ( z < baseZ )
            *defaultGridEntry = 0;
    }
    else
    {
        while ( colIndex >= *rleData )
        {
            colIndex -= *rleData;
            firstBlockEntry += rleData[1] * *rleData;
            if ( rleData[1] )
                v11 = (remoteRow->zCount > 0xFFu) + 3;
            else
                v11 = 2;
            remote_rleData += v11;
            rleData = remote_rleData;
        }
        if ( rleData[1] )
        {
            baseZb = rleData[2];
            if ( remoteRow->zCount > 0xFFu )
                baseZb += rleData[3] << 8;
            if ( z < baseZb )
                *defaultGridEntry = 0;
            localZ = z - baseZb;
            lookupa = z - baseZb + firstBlockEntry + colIndex * rleData[1];
            if ( z - baseZb < rleData[1] )
                v10 = &lightGrid->entries[lookupa];
            else
                v10 = 0;
            *entries = v10;
            if ( localZ + 1 < rleData[1] )
                v9 = &lightGrid->entries[lookupa + 1];
            else
                v9 = 0;
            entries[1] = v9;
            if ( colIndex + 1 < *rleData )
            {
                lookupb = lookupa + rleData[1];
                if ( localZ < rleData[1] )
                    v8 = &lightGrid->entries[lookupb];
                else
                    v8 = 0;
                entries[2] = v8;
                if ( localZ + 1 < rleData[1] )
                    v7 = &lightGrid->entries[lookupb + 1];
                else
                    v7 = 0;
                entries[3] = v7;
                return;
            }
        }
        else
        {
            *entries = 0;
            entries[1] = 0;
            if ( rleData[3] )
            {
                baseZa = rleData[4];
                if ( remoteRow->zCount > 0xFFu )
                    baseZa += rleData[5] << 8;
                if ( z < baseZa + rleData[3] )
                    *defaultGridEntry = 0;
            }
            if ( colIndex + 1 < *rleData )
            {
                entries[2] = 0;
                entries[3] = 0;
                return;
            }
        }
        if ( pos[lightGrid->colAxis] + 1 == remoteRow->colCount + remoteRow->colStart )
        {
            entries[2] = 0;
            entries[3] = 0;
        }
        else
        {
            firstBlockEntrya = firstBlockEntry + rleData[1] * *rleData;
            if ( rleData[1] )
                v6 = rleSizeFull;
            else
                v6 = 2;
            rleDataa = &rleData[v6];
            baseZc = rleDataa[2];
            if ( remoteRow->zCount > 0xFFu )
                baseZc += rleDataa[3] << 8;
            lookupc = z - baseZc + firstBlockEntrya;
            if ( z - baseZc < rleDataa[1] )
                v5 = &lightGrid->entries[lookupc];
            else
                v5 = 0;
            entries[2] = v5;
            if ( z - baseZc + 1 < rleDataa[1] )
                v4 = &lightGrid->entries[lookupc + 1];
            else
                v4 = 0;
            entries[3] = v4;
        }
    }
}

bool __cdecl R_IsValidLightGridSample(
                const GfxLightGrid *lightGrid,
                const GfxLightGridEntry *entry,
                char cornerIndex,
                const unsigned int *pos,
                const float *samplePos)
{
    float traceDir[3]; // [esp+24h] [ebp-24h] BYREF
    float gridPos[3]; // [esp+30h] [ebp-18h]
    float nudgedGridPos[3]; // [esp+3Ch] [ebp-Ch] BYREF

    gridPos[0] = (double)*pos * 32.0 + -131072.0;
    gridPos[1] = (double)pos[1] * 32.0 + -131072.0;
    gridPos[2] = (double)pos[2] * 64.0 + -131072.0;
    gridPos[lightGrid->rowAxis] = (float)((cornerIndex & 4) != 0 ? 0x20 : 0) + gridPos[lightGrid->rowAxis];
    gridPos[lightGrid->colAxis] = (float)((cornerIndex & 2) != 0 ? 0x20 : 0) + gridPos[lightGrid->colAxis];
    gridPos[2] = (float)((cornerIndex & 1) != 0 ? 0x40 : 0) + gridPos[2];
    traceDir[0] = *samplePos - gridPos[0];
    traceDir[1] = samplePos[1] - gridPos[1];
    traceDir[2] = samplePos[2] - gridPos[2];
    Vec3Normalize(traceDir);
    nudgedGridPos[0] = (float)(0.0099999998 * traceDir[0]) + gridPos[0];
    nudgedGridPos[1] = (float)(0.0099999998 * traceDir[1]) + gridPos[1];
    nudgedGridPos[2] = (float)(0.0099999998 * traceDir[2]) + gridPos[2];
    return CM_BoxSightTrace(0, samplePos, nudgedGridPos, vec3_origin, vec3_origin, 0, 8193) == 0;
}

unsigned int __cdecl R_GetLightingAtPoint(
    const GfxLightGrid *remoteLightGrid,
    const float *samplePos,
    unsigned int nonSunPrimaryLightIndex,
    unsigned __int16 dest,
    GfxModelLightExtrapolation extrapolateBehavior,
    bool useHeroLighting)
{
    float v7; // [esp+Ch] [ebp-F4h]
    float v8; // [esp+10h] [ebp-F0h]
    float v9; // [esp+14h] [ebp-ECh]
    float v10; // [esp+18h] [ebp-E8h]
    unsigned __int16 colorsIndex; // [esp+1Eh] [ebp-E2h]
    unsigned int i; // [esp+20h] [ebp-E0h]
    GfxLightGridEntry localEntry; // [esp+48h] [ebp-B8h] BYREF
    unsigned int pos[3]; // [esp+4Ch] [ebp-B4h]
    float cornerWeight[8]; // [esp+58h] [ebp-A8h] BYREF
    const GfxLightGridEntry *entry; // [esp+78h] [ebp-88h]
    const float *heroPos; // [esp+7Ch] [ebp-84h]
    unsigned int cornerIndex; // [esp+80h] [ebp-80h]
    unsigned __int16 sampleColors[8]; // [esp+84h] [ebp-7Ch] BYREF
    unsigned int primaryLightIndex; // [esp+98h] [ebp-68h]
    const GfxLightGridEntry *remoteEntry; // [esp+9Ch] [ebp-64h]
    float primaryVisibleWeight; // [esp+A0h] [ebp-60h]
    const ComPrimaryLight *light; // [esp+A4h] [ebp-5Ch]
    float maxWeight; // [esp+A8h] [ebp-58h]
    const GfxLightGrid *lightGrid; // [esp+ACh] [ebp-54h]
    unsigned int sampleCount; // [esp+B0h] [ebp-50h]
    const GfxLightGridEntry *cornerEntry[8]; // [esp+B4h] [ebp-4Ch] BYREF
    unsigned int defaultGridEntry; // [esp+D4h] [ebp-2Ch] BYREF
    float primaryOccludedWeight; // [esp+D8h] [ebp-28h]
    float sampleWeight[8]; // [esp+DCh] [ebp-24h] BYREF
    const ComPrimaryLight *remoteLight; // [esp+FCh] [ebp-4h]

    if (!remoteLightGrid
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_light.cpp", 2287, 0, "%s", "remoteLightGrid"))
    {
        __debugbreak();
    }
    lightGrid = remoteLightGrid;
    primaryLightIndex = R_LightGridLookup(remoteLightGrid, samplePos, cornerWeight, cornerEntry, &defaultGridEntry);
    if (primaryLightIndex == 255)
    {
        primaryLightIndex = LOBYTE(lightGrid->sunPrimaryLightIndex);
    }
    else if (lightGrid->hasLightRegions && primaryLightIndex != lightGrid->sunPrimaryLightIndex)
    {
        primaryLightIndex = nonSunPrimaryLightIndex;
    }
    v9 = floor(*samplePos);
    pos[0] = ((int)v9 + 0x20000) >> 5;
    v8 = floor(samplePos[1]);
    pos[1] = ((int)v8 + 0x20000) >> 5;
    v7 = floor(samplePos[2]);
    pos[2] = ((int)v7 + 0x20000) >> 6;
    heroPos = useHeroLighting ? samplePos : 0;
    sampleCount = 0;
    maxWeight = 0.0f;
    primaryVisibleWeight = 0.0f;
    primaryOccludedWeight = 0.0f;
    for (cornerIndex = 0; cornerIndex < 8; ++cornerIndex)
    {
        remoteEntry = cornerEntry[cornerIndex];
        if (remoteEntry)
        {
            localEntry = *remoteEntry;
            entry = &localEntry;
            if (localEntry.primaryLightIndex == primaryLightIndex)
            {
                primaryVisibleWeight = primaryVisibleWeight + cornerWeight[cornerIndex];
            }
            else if (!entry->primaryLightIndex || entry->primaryLightIndex == 255 && primaryLightIndex)
            {
                remoteLight = Com_GetPrimaryLight(primaryLightIndex);
                light = remoteLight;
                if (R_CanLightInfluenceLightGridCorner(lightGrid, remoteLight, samplePos, cornerIndex))
                    primaryOccludedWeight = primaryOccludedWeight + cornerWeight[cornerIndex];
            }
            maxWeight = maxWeight + cornerWeight[cornerIndex];
            v10 = cornerWeight[cornerIndex];
            colorsIndex = entry->colorsIndex;
            for (i = 0; i < sampleCount; ++i)
            {
                if (sampleColors[i] == colorsIndex)
                {
                    sampleWeight[i] = sampleWeight[i] + v10;
                    goto LABEL_10;
                }
            }
            sampleColors[sampleCount] = colorsIndex;
            sampleWeight[sampleCount++] = v10;
        }
    LABEL_10:
        ;
    }
    if (maxWeight < 0.0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_light.cpp",
            2339,
            0,
            "%s\n\t(maxWeight) = %g",
            "(maxWeight >= 0.0f)",
            maxWeight))
    {
        __debugbreak();
    }
    if (primaryVisibleWeight < 0.0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_light.cpp",
            2340,
            0,
            "%s\n\t(primaryVisibleWeight) = %g",
            "(primaryVisibleWeight >= 0.0f)",
            primaryVisibleWeight))
    {
        __debugbreak();
    }
    if (!sampleCount)
        return R_ExtrapolateLightingAtPoint(lightGrid, heroPos, dest, extrapolateBehavior, defaultGridEntry);
    if (maxWeight <= 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_light.cpp", 2349, 1, "%s", "maxWeight > 0.0f"))
    {
        __debugbreak();
    }
    if (primaryLightIndex)
    {
        if (primaryOccludedWeight == 0.0)
        {
            if (primaryVisibleWeight != 0.0)
                primaryVisibleWeight = 1.0f;
        }
        else
        {
            primaryVisibleWeight = primaryVisibleWeight / (float)(primaryVisibleWeight + primaryOccludedWeight);
        }
    }
    else
    {
        primaryVisibleWeight = 0.0f;
    }
    if (sampleCount == 1)
        R_SetLightGridColorsFromIndex(lightGrid, sampleColors[0], heroPos, primaryVisibleWeight, dest);
    else
        R_BlendAndSetLightGridColors(
            lightGrid,
            (unsigned char*)sampleColors,
            sampleWeight,
            sampleCount,
            heroPos,
            primaryVisibleWeight,
            1.0 / maxWeight,
            dest);
    return primaryLightIndex;
}

void __cdecl R_SetLightGridColorsFromIndex(
                const GfxLightGrid *lightGrid,
                unsigned int colorsIndex,
                const float *heroPos,
                float primaryLightWeight,
                unsigned __int16 dest)
{
    GfxModelLightingPatch *patch; // [esp+Ch] [ebp-4h]

    patch = R_BackEndDataAllocAndClearModelLightingPatch(frontEndDataOut);
    patch->modelLightingIndex = dest;
    patch->primaryLightWeight = primaryLightWeight;
    patch->colorsCount = 1;
    patch->colorsIndex[0] = truncate_cast<unsigned short>(colorsIndex);
    patch->useHeroLighting = heroPos != 0;
    if ( heroPos )
    {
        patch->heroPos[0] = *heroPos;
        patch->heroPos[1] = heroPos[1];
        patch->heroPos[2] = heroPos[2];
    }
}

GfxModelLightingPatch *__cdecl R_BackEndDataAllocAndClearModelLightingPatch(GfxBackEndData *frontEndDataOut)
{
    unsigned int patchIndex; // [esp+4h] [ebp-4h]

    patchIndex = _InterlockedExchangeAdd(&frontEndDataOut->modelLightingPatchCount, 1u);
    if ( patchIndex >= 0x1000 )
        Com_Error(ERR_FATAL, "modelLightingPatchList ran out of elements.");
    memset(
        (unsigned __int8 *)&frontEndDataOut->modelLightingPatchList[patchIndex],
        0,
        sizeof(frontEndDataOut->modelLightingPatchList[patchIndex]));
    return &frontEndDataOut->modelLightingPatchList[patchIndex];
}

void __cdecl R_BlendAndSetLightGridColors(
                const GfxLightGrid *lightGrid,
                unsigned __int8 *colorsIndex,
                const float *colorsWeight,
                unsigned int colorsCount,
                const float *heroPos,
                float primaryLightWeight,
                float weightNormalizeScale,
                unsigned __int16 dest)
{
    unsigned int i; // [esp+Ch] [ebp-8h]
    GfxModelLightingPatch *patch; // [esp+10h] [ebp-4h]

    patch = R_BackEndDataAllocAndClearModelLightingPatch(frontEndDataOut);
    patch->modelLightingIndex = dest;
    patch->primaryLightWeight = primaryLightWeight;
    patch->colorsCount = truncate_cast<unsigned char>(colorsCount);
    for ( i = 0; i < colorsCount; ++i )
        patch->colorsWeight[i] = colorsWeight[i] * weightNormalizeScale;
    memcpy((unsigned __int8 *)patch->colorsIndex, colorsIndex, 2 * colorsCount);
    patch->useHeroLighting = heroPos != 0;
    if ( heroPos )
    {
        patch->heroPos[0] = *heroPos;
        patch->heroPos[1] = heroPos[1];
        patch->heroPos[2] = heroPos[2];
    }
}

unsigned __int8 __cdecl R_ExtrapolateLightingAtPoint(
                const GfxLightGrid *lightGrid,
                const float *heroPos,
                unsigned __int16 dest,
                GfxModelLightExtrapolation extrapolateBehavior,
                unsigned int defaultGridEntry)
{
    if ( extrapolateBehavior == GFX_MODELLIGHT_SHOW_MISSING
        && !defaultGridEntry
        && r_showMissingLightGrid->current.enabled
        || lightGrid->colorCount <= defaultGridEntry )
    {
        R_SetLightGridColorsFromIndex(lightGrid, lightGrid->colorCount - 1, 0, 1.0, dest);
        return 0;
    }
    else
    {
        R_SetLightGridColorsFromIndex(lightGrid, defaultGridEntry, heroPos, 1.0, dest);
        return lightGrid->sunPrimaryLightIndex;
    }
}

char __cdecl R_CanLightInfluenceLightGridCorner(
                const GfxLightGrid *lightGrid,
                const ComPrimaryLight *light,
                const float *samplePos,
                char cornerIndex)
{
    float v5; // [esp+8h] [ebp-24h]
    float v6; // [esp+Ch] [ebp-20h]
    float v7; // [esp+10h] [ebp-1Ch]
    float gridPos[3]; // [esp+20h] [ebp-Ch] BYREF

    if ( !light->type )
        return 0;
    if ( light->type == 1 )
        return 1;
    v7 = floor((float)(*samplePos * 0.03125));
    gridPos[0] = v7 * 32.0;
    v6 = floor((float)(samplePos[1] * 0.03125));
    gridPos[1] = v6 * 32.0;
    v5 = floor((float)(*samplePos * 0.015625));
    gridPos[2] = v5 * 64.0;
    gridPos[lightGrid->rowAxis] = (float)((cornerIndex & 4) != 0 ? 0x20 : 0) + gridPos[lightGrid->rowAxis];
    gridPos[lightGrid->colAxis] = (float)((cornerIndex & 2) != 0 ? 0x20 : 0) + gridPos[lightGrid->colAxis];
    gridPos[2] = (float)((cornerIndex & 1) != 0 ? 0x40 : 0) + gridPos[2];
    return Com_CanPrimaryLightAffectPoint(light, gridPos);
}

void __cdecl R_GetAverageLightingAtPoint(const float *samplePos, float *outColor)
{
    float v2; // [esp+60h] [ebp-7D8h]
    unsigned __int16 colorsIndex; // [esp+66h] [ebp-7D2h]
    unsigned int k; // [esp+68h] [ebp-7D0h]
    int m; // [esp+6Ch] [ebp-7CCh]
    int j; // [esp+70h] [ebp-7C8h]
    unsigned int i; // [esp+74h] [ebp-7C4h]
    GfxDecodedLightGridColors decodedColors; // [esp+78h] [ebp-7C0h] BYREF
    GfxDecodedLightGridColors accumulatedColors; // [esp+3F8h] [ebp-440h] BYREF
    GfxCompressedLightGridColors *remoteColors; // [esp+77Ch] [ebp-BCh]
    GfxCompressedLightGridColors *colors; // [esp+780h] [ebp-B8h]
    float average[4]; // [esp+784h] [ebp-B4h] BYREF
    float cornerWeight[8]; // [esp+794h] [ebp-A4h] BYREF
    const GfxLightGridEntry *entry; // [esp+7B4h] [ebp-84h]
    unsigned int cornerIndex; // [esp+7B8h] [ebp-80h]
    unsigned __int16 sampleColors[8]; // [esp+7BCh] [ebp-7Ch]
    float weightNormalizeScale; // [esp+7D0h] [ebp-68h]
    unsigned __int8 primaryLightIndex; // [esp+7D7h] [ebp-61h]
    float colorWithSunAlpha[4]; // [esp+7D8h] [ebp-60h] BYREF
    float maxWeight; // [esp+7E8h] [ebp-50h]
    unsigned int sampleCount; // [esp+7ECh] [ebp-4Ch]
    const GfxLightGridEntry *cornerEntry[8]; // [esp+7F0h] [ebp-48h] BYREF
    unsigned int defaultGridEntry; // [esp+810h] [ebp-28h] BYREF
    float sampleWeight[8]; // [esp+814h] [ebp-24h]
    float primaryWeight; // [esp+834h] [ebp-4h]

    sampleCount = 0;
    maxWeight = 0.0f;
    primaryWeight = 0.0f;
    primaryLightIndex = R_LightGridLookup(&rgp.world->lightGrid, samplePos, cornerWeight, cornerEntry, &defaultGridEntry);
    if ( primaryLightIndex != 1 )
    {
        memset(colorWithSunAlpha, 0, 12);
        colorWithSunAlpha[3] = 0.25f;
        goto LABEL_35;
    }
    for ( cornerIndex = 0; cornerIndex < 8; ++cornerIndex )
    {
        entry = cornerEntry[cornerIndex];
        if ( !entry )
            continue;
        if ( entry->primaryLightIndex && entry->primaryLightIndex != 255 )
        {
            if ( entry->primaryLightIndex != primaryLightIndex )
                continue;
            primaryWeight = primaryWeight + cornerWeight[cornerIndex];
        }
        maxWeight = maxWeight + cornerWeight[cornerIndex];
        v2 = cornerWeight[cornerIndex];
        colorsIndex = entry->colorsIndex;
        for ( k = 0; k < sampleCount; ++k )
        {
            if ( sampleColors[k] == colorsIndex )
            {
                sampleWeight[k] = sampleWeight[k] + v2;
                goto LABEL_4;
            }
        }
        sampleColors[sampleCount] = colorsIndex;
        sampleWeight[sampleCount++] = v2;
LABEL_4:
        ;
    }
    if ( maxWeight < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_light.cpp",
                    2416,
                    0,
                    "%s\n\t(maxWeight) = %g",
                    "(maxWeight >= 0.0f)",
                    maxWeight) )
    {
        __debugbreak();
    }
    if ( primaryWeight < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_light.cpp",
                    2417,
                    0,
                    "%s\n\t(primaryWeight) = %g",
                    "(primaryWeight >= 0.0f)",
                    primaryWeight) )
    {
        __debugbreak();
    }
    if ( sampleCount )
    {
        weightNormalizeScale = 1.0 / maxWeight;
        remoteColors = &rgp.world->lightGrid.colors[sampleColors[0]];
        colors = remoteColors;
        R_DecodeLightGridColorsWeighted(remoteColors, &accumulatedColors, sampleWeight[0] * (float)(1.0 / maxWeight));
        for ( i = 1; i < sampleCount; ++i )
        {
            remoteColors = &rgp.world->lightGrid.colors[sampleColors[i]];
            colors = remoteColors;
            R_DecodeLightGridColorsWeighted(remoteColors, &decodedColors, sampleWeight[i] * weightNormalizeScale);
            for ( j = 0; j < 56; ++j )
            {
                accumulatedColors.rgb[j][0] = accumulatedColors.rgb[j][0] + decodedColors.rgb[j][0];
                accumulatedColors.rgb[j][1] = accumulatedColors.rgb[j][1] + decodedColors.rgb[j][1];
                accumulatedColors.rgb[j][2] = accumulatedColors.rgb[j][2] + decodedColors.rgb[j][2];
            }
        }
        memset(average, 0, sizeof(average));
        for ( m = 0; m < 56; ++m )
        {
            average[0] = average[0] + accumulatedColors.rgb[m][0];
            average[1] = average[1] + accumulatedColors.rgb[m][1];
            average[2] = average[2] + accumulatedColors.rgb[m][2];
        }
        colorWithSunAlpha[0] = average[0] / 56.0;
        colorWithSunAlpha[1] = average[1] / 56.0;
        colorWithSunAlpha[2] = average[2] / 56.0;
        colorWithSunAlpha[3] = primaryWeight;
    }
    else
    {
        memset(colorWithSunAlpha, 0, 12);
        colorWithSunAlpha[3] = 1.0f;
    }
LABEL_35:
    *outColor = (float)((float)(rgp.world->sunParse.sunSettings[0].sunDiffuseColor[0]
                                                        * rgp.world->sunParse.sunSettings[0].sunDiffuseColor[3])
                                        * colorWithSunAlpha[3])
                        + colorWithSunAlpha[0];
    outColor[1] = (float)((float)(rgp.world->sunParse.sunSettings[0].sunDiffuseColor[1]
                                                            * rgp.world->sunParse.sunSettings[0].sunDiffuseColor[3])
                                            * colorWithSunAlpha[3])
                            + colorWithSunAlpha[1];
    outColor[2] = (float)((float)(rgp.world->sunParse.sunSettings[0].sunDiffuseColor[2]
                                                            * rgp.world->sunParse.sunSettings[0].sunDiffuseColor[3])
                                            * colorWithSunAlpha[3])
                            + colorWithSunAlpha[2];
}

void __cdecl R_GetAverageSecondaryLightingAtPoint(const float *samplePos, float *outColor)
{
    float v2; // [esp+8h] [ebp-7ECh]
    float *v3; // [esp+Ch] [ebp-7E8h]
    float *v4; // [esp+10h] [ebp-7E4h]
    float *v5; // [esp+18h] [ebp-7DCh]
    float v6; // [esp+48h] [ebp-7ACh]
    unsigned __int16 colorsIndex; // [esp+4Eh] [ebp-7A6h]
    unsigned int k; // [esp+50h] [ebp-7A4h]
    int m; // [esp+54h] [ebp-7A0h]
    int j; // [esp+58h] [ebp-79Ch]
    GfxDecodedLightGridColors decodedColors; // [esp+5Ch] [ebp-798h] BYREF
    GfxCompressedLightGridColors *colors; // [esp+3DCh] [ebp-418h]
    unsigned int i; // [esp+3E0h] [ebp-414h]
    const GfxLightGridEntry *entry; // [esp+3E4h] [ebp-410h]
    unsigned int cornerIndex; // [esp+3E8h] [ebp-40Ch]
    float cornerWeight[8]; // [esp+3ECh] [ebp-408h] BYREF
    GfxDecodedLightGridColors accumulatedColors; // [esp+40Ch] [ebp-3E8h] BYREF
    unsigned __int16 sampleColors[8]; // [esp+790h] [ebp-64h]
    float weightNormalizeScale; // [esp+7A4h] [ebp-50h]
    float maxWeight; // [esp+7A8h] [ebp-4Ch]
    unsigned int sampleCount; // [esp+7ACh] [ebp-48h]
    const GfxLightGridEntry *cornerEntry[8]; // [esp+7B0h] [ebp-44h] BYREF
    unsigned int defaultGridEntry; // [esp+7D0h] [ebp-24h] BYREF
    float sampleWeight[8]; // [esp+7D4h] [ebp-20h]

    sampleCount = 0;
    maxWeight = 0.0f;
    R_LightGridLookup(&rgp.world->lightGrid, samplePos, cornerWeight, cornerEntry, &defaultGridEntry);
    for ( cornerIndex = 0; cornerIndex < 8; ++cornerIndex )
    {
        entry = cornerEntry[cornerIndex];
        if ( entry )
        {
            maxWeight = maxWeight + cornerWeight[cornerIndex];
            v6 = cornerWeight[cornerIndex];
            colorsIndex = entry->colorsIndex;
            for ( k = 0; k < sampleCount; ++k )
            {
                if ( sampleColors[k] == colorsIndex )
                {
                    sampleWeight[k] = sampleWeight[k] + v6;
                    goto LABEL_2;
                }
            }
            sampleColors[sampleCount] = colorsIndex;
            sampleWeight[sampleCount++] = v6;
        }
LABEL_2:
        ;
    }
    if ( maxWeight < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_light.cpp",
                    2503,
                    0,
                    "%s\n\t(maxWeight) = %g",
                    "(maxWeight >= 0.0f)",
                    maxWeight) )
    {
        __debugbreak();
    }
    memset(&accumulatedColors, 0, sizeof(accumulatedColors));
    if ( maxWeight <= 0.0 )
        v2 = 0.0f;
    else
        v2 = 1.0 / maxWeight;
    weightNormalizeScale = v2;
    for ( i = 0; i < sampleCount; ++i )
    {
        colors = &rgp.world->lightGrid.colors[sampleColors[i]];
        R_DecodeLightGridColorsWeighted(colors, &decodedColors, sampleWeight[i] * weightNormalizeScale);
        for ( j = 0; j < 56; ++j )
        {
            v4 = accumulatedColors.rgb[j];
            v5 = decodedColors.rgb[j];
            *v4 = *v5 + *v4;
            v4[1] = v5[1] + v4[1];
            v4[2] = v5[2] + v4[2];
        }
    }
    *outColor = 0.0f;
    outColor[1] = 0.0f;
    outColor[2] = 0.0f;
    for ( m = 0; m < 56; ++m )
    {
        v3 = accumulatedColors.rgb[m];
        *outColor = *v3 + *outColor;
        outColor[1] = v3[1] + outColor[1];
        outColor[2] = v3[2] + outColor[2];
    }
    *outColor = 0.017857144 * *outColor;
    outColor[1] = 0.017857144 * outColor[1];
    outColor[2] = 0.017857144 * outColor[2];
}

void __cdecl R_SetLightGridSampleDeltas(int rowStride, int sliceStride)
{
    s_lightGridRowDelta = rowStride;
    s_lightGridSliceDelta = sliceStride - 3 * rowStride;
}

void __cdecl R_InitLightVisHistory(const char *bspName)
{
    LONG Target[5]; // [esp+0h] [ebp-70h] BYREF
    unsigned __int16 gsp[3]; // [esp+14h] [ebp-5Ch] BYREF
    unsigned int i; // [esp+1Ch] [ebp-54h]
    char filename[68]; // [esp+20h] [ebp-50h] BYREF
    unsigned __int16 (*buffer)[3]; // [esp+68h] [ebp-8h] BYREF
    int count; // [esp+6Ch] [ebp-4h]

    if (r_vc_makelog->current.integer == 2)
    {
        AllocAllMemoryNeeded();
        R_LightVisHistoryFilename((char*)bspName, filename);
        count = FS_ReadFile(filename, (void **)&buffer);
        if (count >= 0)
        {
            if (!(count % 6u))
            {
                if (count > 0x600000)
                    count = 0x600000;
                //tlAtomicMutex::Lock(&lightGridMutex);
                lightGridMutex.Lock();
                for (i = 0; i < count / 6u; ++i)
                {
                    gsp[0] = buffer[i][0];
                    gsp[1] = buffer[i][1];
                    gsp[2] = buffer[i][2];
                    addHash((const unsigned __int16 (*)[3])gsp);
                }
                if (!--lightGridMutex.LockCount)
                {
                    Target[0] = 0;
                    InterlockedExchange(Target, 0);
                    lightGridMutex.ThreadId = 0;
                }
            }
            FS_FreeFile(buffer);
        }
    }
}

void __cdecl R_LightVisHistoryFilename(char *bspName, char *filename)
{
    if ( !bspName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_light.cpp", 2543, 0, "%s", "bspName") )
    {
        __debugbreak();
    }
    Com_StripExtension(bspName, filename);
    if ( strlen(filename) + 5 >= 0x40 )
        Com_Error(ERR_DROP, "light grid log filename '%s.grid' is too long", filename);
    strcat(filename, ".grid");
}

void __cdecl R_SaveLightVisHistory()
{
    htab *curhash; // [esp+4h] [ebp-58h]
    unsigned int i; // [esp+8h] [ebp-54h]
    char filename[68]; // [esp+Ch] [ebp-50h] BYREF
    unsigned __int16 (*buffer)[3]; // [esp+54h] [ebp-8h]
    unsigned int count; // [esp+58h] [ebp-4h]

    if ( lightPointCount )
    {
        if ( rgp.world )
        {
            buffer = (unsigned __int16 (*)[3])Z_VirtualAlloc(6 * lightPointCount, "R_SaveLightVisHistory", 0);
            if ( buffer )
            {
                R_LightVisHistoryFilename((char *)rgp.world->name, filename);
                count = 0;
                for ( i = 0; i < 0x100007; ++i )
                {
                    for ( curhash = LightGridHashtable[i]; curhash; curhash = curhash->next )
                    {
                        HashUnpackPoint(curhash->value, (unsigned __int16 (*)[3])buffer[count++]);
                        if ( count >= 0x100000
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_light.cpp",
                                        2610,
                                        0,
                                        "%s",
                                        "count < MAX_MAP_LIGHTGRID_POINTS") )
                        {
                            __debugbreak();
                        }
                    }
                }
                if ( count != lightPointCount
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_light.cpp",
                                2616,
                                0,
                                "%s",
                                "count==lightPointCount") )
                {
                    __debugbreak();
                }
                FS_WriteFile(filename, (char *)buffer, 6 * count);
                Z_VirtualFree(buffer, 0);
                Z_VirtualFree(LightGridMemory, 0);
                Z_VirtualFree(LightGridHashtable, 0);
                LightGridHashtable = 0;
                LightGridMemory = 0;
            }
        }
    }
}

void __cdecl HashUnpackPoint(unsigned __int64 value, unsigned __int16 (*p)[3])
{
    *(unsigned int *)p = value;
    (*p)[2] = WORD2(value);
}

// in libtomcrypt
//int __cdecl find_hash(const char *a1)
//{
//    int result; // eax
//    const char **v2; // esi
//
//    if ( !a1 )
//        crypt_argchk(
//            (char*)"name != NULL",
//            (char*)"C:\\projects_pc\\cod\\codsrc\\libs\\libtomcrypt-1.17\\src\\misc\\crypt\\crypt_find_hash.c",
//            26);
//    result = 0;
//    v2 = (const char **)hash_descriptor;
//    while ( !*v2 || strcmp(*v2, a1) )
//    {
//        v2 += 26;
//        ++result;
//        if ( (int)v2 >= (int)cipher_descriptor )
//            return -1;
//    }
//    return result;
//}
//
//