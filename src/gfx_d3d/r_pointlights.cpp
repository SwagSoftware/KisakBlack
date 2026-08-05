#include "r_pointlights.h"
#include <universal/com_math.h>
#include "r_dvars.h"

#include <cstring>
#include "r_init.h"

float gridBasisDirs[56][3];

unsigned int g_heroLightCount;
unsigned int g_heroLightTreeCount;
GfxHeroLight *g_heroLights;
GfxHeroLightTree *g_heroLightTree;

void __cdecl GenerateLightGridBasisDirs()
{
    int x; // [esp+Ch] [ebp-1Ch]
    int y; // [esp+10h] [ebp-18h]
    int z; // [esp+14h] [ebp-14h]
    float dir[3]; // [esp+18h] [ebp-10h] BYREF
    int basisIndex; // [esp+24h] [ebp-4h]

    basisIndex = 0;
    for ( z = 0; z < 4; ++z )
    {
        dir[2] = (float)((float)z * 0.66666669) - 1.0;
        for ( y = 0; y < 4; ++y )
        {
            dir[1] = (float)((float)y * 0.66666669) - 1.0;
            for ( x = 0; x < 4; ++x )
            {
                if ( x <= 0 || x >= 3 || y <= 0 || y >= 3 || z <= 0 || z >= 3 )
                {
                    dir[0] = (float)((float)x * 0.66666669) - 1.0;
                    Vec3NormalizeTo(dir, gridBasisDirs[basisIndex++]);
                }
            }
        }
    }
    if ( basisIndex != 56
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_pointlights.cpp",
                    41,
                    1,
                    "%s",
                    "basisIndex == GFX_LIGHTGRID_SAMPLE_COUNT") )
    {
        __debugbreak();
    }
}

void __cdecl GatherIncidentEnergyInSpaceForLightFromDir(
                const float *energy,
                const float *lightFromDir,
                float (*incidentEnergy)[3])
{
    float *v3; // [esp+0h] [ebp-14h]
    float attenuation; // [esp+Ch] [ebp-8h]
    int sampleIndex; // [esp+10h] [ebp-4h]

    for ( sampleIndex = 0; sampleIndex < 56; ++sampleIndex )
    {
        attenuation = (float)((float)(gridBasisDirs[sampleIndex][0] * *lightFromDir)
                                                + (float)(gridBasisDirs[sampleIndex][1] * lightFromDir[1]))
                                + (float)(gridBasisDirs[sampleIndex][2] * lightFromDir[2]);
        if ( attenuation > 0.0 )
        {
            v3 = &(*incidentEnergy)[3 * sampleIndex];
            *v3 = (float)(attenuation * *energy) + *v3;
            v3[1] = (float)(attenuation * energy[1]) + v3[1];
            v3[2] = (float)(attenuation * energy[2]) + v3[2];
        }
    }
}

void __cdecl AddLightGridLightingForDir(float (*lightingForDir)[3], GfxDecodedLightGridColors *colors)
{
    float *src; // [esp+Ch] [ebp-Ch]
    float *dest; // [esp+10h] [ebp-8h]
    int sampleIndex; // [esp+14h] [ebp-4h]

    for ( sampleIndex = 0; sampleIndex < 56; ++sampleIndex )
    {
        src = &(*lightingForDir)[3 * sampleIndex];
        *src = sqrtf(*src);
        src[1] = sqrtf(src[1]);
        src[2] = sqrtf(src[2]);
        dest = colors->rgb[sampleIndex];
        *dest = *src + *dest;
        dest[1] = src[1] + dest[1];
        dest[2] = src[2] + dest[2];
    }
}

char __cdecl EvaluateHeroLightForGrid(
                const GfxHeroLight *light,
                const float *xyz,
                float *dirToLight,
                float *attenuatedColor)
{
    float v5; // [esp+0h] [ebp-4Ch]
    float v6; // [esp+4h] [ebp-48h]
    float v7; // [esp+Ch] [ebp-40h]
    float v8; // [esp+10h] [ebp-3Ch]
    float delta; // [esp+20h] [ebp-2Ch]
    float delta_4; // [esp+24h] [ebp-28h]
    float delta_8; // [esp+28h] [ebp-24h]
    float dist; // [esp+2Ch] [ebp-20h]
    float spotScale; // [esp+30h] [ebp-1Ch]
    float spotDot; // [esp+34h] [ebp-18h]
    int exponent; // [esp+38h] [ebp-14h]
    float spotFactor; // [esp+3Ch] [ebp-10h]
    float distSq; // [esp+48h] [ebp-4h]

    delta = light->origin[0] - *xyz;
    delta_4 = light->origin[1] - xyz[1];
    delta_8 = light->origin[2] - xyz[2];
    distSq = (float)((float)(delta * delta) + (float)(delta_4 * delta_4)) + (float)(delta_8 * delta_8);
    if ( distSq > (float)(light->radius * light->radius) )
        return 0;
    dist = sqrtf(distSq);
    if ( dist < 0.001 )
        return 0;
    *dirToLight = (float)(1.0 / dist) * delta;
    dirToLight[1] = (float)(1.0 / dist) * delta_4;
    dirToLight[2] = (float)(1.0 / dist) * delta_8;
    spotFactor = 1.0f;
    if ( light->type == 2 )
    {
        spotDot = (float)((float)(*dirToLight * light->dir[0]) + (float)(dirToLight[1] * light->dir[1]))
                        + (float)(dirToLight[2] * light->dir[2]);
        if ( light->cosHalfFovOuter >= spotDot )
            return 0;
        if ( light->cosHalfFovInner > spotDot )
        {
            spotScale = (float)(spotDot - light->cosHalfFovOuter) / (float)(light->cosHalfFovInner - light->cosHalfFovOuter);
            if ( spotScale < 1.0 )
            {
                for ( exponent = 0; exponent < light->exponent; ++exponent )
                    spotFactor = spotFactor * spotScale;
            }
        }
    }
    v7 = dist / light->radius;
    if ( (float)(v7 - 1.0) < 0.0 )
        v8 = dist / light->radius;
    else
        v8 = 1.0f;
    if ( (float)(0.0 - v7) < 0.0 )
        v5 = v8;
    else
        v5 = 0.0f;
    v6 = (float)(1.0 - v5) * spotFactor;
    *attenuatedColor = v6 * light->color[0];
    attenuatedColor[1] = v6 * light->color[1];
    attenuatedColor[2] = v6 * light->color[2];
    return 1;
}

// (aislop)
void R_AdjustLightColorSamples(GfxDecodedLightGridColors *colors)
{
    const float intensity = r_lightGridIntensity->current.value;

    float meanSample = 0.0f;
    for (unsigned int i = 0; i < 0x38; ++i)
    {
        float *rgb = colors->rgb[i];

        rgb[0] *= intensity;
        rgb[1] *= intensity;
        rgb[2] *= intensity;

        meanSample += rgb[0] + rgb[1] + rgb[2];
    }

    meanSample /= (0x38 * 3);

    const float exponent = r_lightGridIntensity->current.value;

    for (unsigned int i = 0; i < 0x38; ++i)
    {
        float *rgb = colors->rgb[i];

        rgb[0] = powf(rgb[0], exponent);
        rgb[1] = powf(rgb[1], exponent);
        rgb[2] = powf(rgb[2], exponent);
    }
}

bool g_initialized;
void __cdecl R_AddHeroOnlyLightsToGridColors(GfxDecodedLightGridColors *packed, const float *heroPos)
{
    GfxHeroLightTree *tree; // [esp+0h] [ebp-354h]
    unsigned int node; // [esp+Ch] [ebp-348h]
    float dirToLight[3]; // [esp+10h] [ebp-344h] BYREF
    unsigned int stack[32]; // [esp+1Ch] [ebp-338h]
    float attenuatedColor[3]; // [esp+9Ch] [ebp-2B8h] BYREF
    unsigned int stackPtr; // [esp+A8h] [ebp-2ACh]
    float incidentEnergy[56][3]; // [esp+ACh] [ebp-2A8h] BYREF
    bool foundAny; // [esp+353h] [ebp-1h]

    if ( r_heroLighting->current.enabled && heroPos )
    {
        if ( r_lightGridEnableTweaks->current.enabled && r_lightGridUseTweakedValues->current.enabled )
            R_AdjustLightColorSamples(packed);
        if ( !g_initialized )
        {
            g_initialized = 1;
            R_InitHeroLights();
        }
        if ( g_heroLightCount )
        {
            stack[0] = 0;
            stackPtr = 1;
            foundAny = 0;
            while ( stackPtr )
            {
                node = stack[--stackPtr];
                if ( node < g_heroLightTreeCount )
                {
                    tree = &g_heroLightTree[node];
                    if ( tree->mins[0] <= *heroPos
                        && tree->mins[1] <= heroPos[1]
                        && tree->mins[2] <= heroPos[2]
                        && *heroPos <= tree->maxs[0]
                        && heroPos[1] <= tree->maxs[1]
                        && heroPos[2] <= tree->maxs[2] )
                    {
                        stack[stackPtr++] = 2 * node + 1;
                        stack[stackPtr++] = 2 * node + 2;
                        if ( stackPtr >= 0x20
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_pointlights.cpp",
                                        282,
                                        0,
                                        "%s",
                                        "stackPtr < 32") )
                        {
                            __debugbreak();
                        }
                    }
                }
                else if ( node - g_heroLightTreeCount < g_heroLightCount
                             && EvaluateHeroLightForGrid(
                                        &g_heroLights[node - g_heroLightTreeCount],
                                        heroPos,
                                        dirToLight,
                                        attenuatedColor) )
                {
                    if ( !foundAny )
                    {
                        foundAny = 1;
                        memset(incidentEnergy, 0, sizeof(incidentEnergy));
                    }
                    GatherIncidentEnergyInSpaceForLightFromDir(attenuatedColor, dirToLight, incidentEnergy);
                }
            }
            if ( foundAny )
                AddLightGridLightingForDir(incidentEnergy, packed);
        }
    }
}

void R_InitHeroLights()
{
    g_heroLightCount = rgp.world->heroLightCount;
    g_heroLightTreeCount = rgp.world->heroLightTreeCount;
    g_heroLights = rgp.world->heroLights;
    g_heroLightTree = rgp.world->heroLightTree;
}
