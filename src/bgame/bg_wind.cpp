#include "bg_wind.h"
#include <cgame_mp/cg_local_mp.h>
#include <cgame/cg_drawtools.h>
#include <qcommon/cm_trace.h>
#include <game/g_debug.h>
#include <gfx_d3d/r_wind.h>
#include <client/splitscreen.h>
#include <cgame_mp/cg_main_mp.h>
#include <gfx_d3d/r_foliage.h>
#include <EffectsCore/fx_wind.h>

const dvar_t *g_EnableGlobalWind;
const dvar_t *g_EnableGrassWind;
const dvar_t *g_GlobalWindVector;
const dvar_t *g_GlobalLowWindAltitude;
const dvar_t *g_GlobalHiWindAltitude;
const dvar_t *g_GlobalLowWindStrengthPercentage;
const dvar_t *g_GlobalWindLeafScale;
const dvar_t *g_GlobalWindGrassGustInterval;
const dvar_t *g_GlobalWindGrassScale;
const dvar_t *g_GlobalWindGrassTension;
const dvar_t *g_GlobalWindGustSpeedMultiplier;
const dvar_t *g_GlobalWindGustRadius;
const dvar_t *g_GlobalWindGustStrength;
const dvar_t *g_GlobalWindGustDistance;
const dvar_t *g_GlobalWindDebugDisplay;
const dvar_t *g_GlobalTreeBend;
const dvar_t *g_GlobalTreeRandom;
const dvar_t *g_GlobalTreeFrequency;
const dvar_t *g_localWindVector;
const dvar_t *g_localWindAmplitude;
const dvar_t *g_localWindFrequency;
const dvar_t *g_localWindGustRadius;
const dvar_t *g_localWindGustStrength;

float windDebugPos[3];

bool debugValLastFrame;

GrassWind g_GrassWind[1][16];
float g_WindVariants[17];
VariantWind g_VariantWind[16];
LOCALIZED_WIND g_localWind;

void __cdecl BG_Wind_RegisterDvars()
{
    g_EnableGlobalWind = _Dvar_RegisterBool("enable_global_wind", 0, 0x1000u, "enable wind effect");
    g_EnableGrassWind = _Dvar_RegisterBool("enable_grass_wind", 0, 0x1000u, "enable grass wind effect");
    g_GlobalWindVector = _Dvar_RegisterVec3(
                                                 "wind_global_vector",
                                                 (1.0),
                                                 (0.0),
                                                 (0.0),
                                                 -1000.0,
                                                 1000.0,
                                                 0x1000u,
                                                 "global wind force in units/sec. This is also the hi-level wind");
    g_GlobalLowWindAltitude = _Dvar_RegisterFloat(
                                                            "wind_global_low_altitude",
                                                            0.0,
                                                            -100000.0,
                                                            100000.0,
                                                            0x1000u,
                                                            "altitude of global low-level wind");
    g_GlobalHiWindAltitude = _Dvar_RegisterFloat(
                                                         "wind_global_hi_altitude",
                                                         0.0,
                                                         -100000.0,
                                                         100000.0,
                                                         0x1000u,
                                                         "altitude of global hi-level wind");
    g_GlobalLowWindStrengthPercentage = _Dvar_RegisterFloat(
                                                                                "wind_global_low_strength_percent",
                                                                                1.0,
                                                                                -20.0,
                                                                                20.0,
                                                                                0x1000u,
                                                                                "strength of global low-level wind, expressed as a percentage of wind_global_vector");
    g_GlobalWindLeafScale = _Dvar_RegisterFloat(
                                                        "wind_leaf_scale",
                                                        176.0,
                                                        0.0,
                                                        50000.0,
                                                        0x1000u,
                                                        "global wind grass tension (stiffness)");
    g_GlobalWindGrassGustInterval = _Dvar_RegisterFloat(
                                                                        "wind_grass_gustinterval",
                                                                        250.0,
                                                                        1.0,
                                                                        50000.0,
                                                                        0x1000u,
                                                                        "global wind force gust interval");
    g_GlobalWindGrassScale = _Dvar_RegisterFloat(
                                                         "wind_grass_scale",
                                                         10000.0,
                                                         1.0,
                                                         50000.0,
                                                         0x1000u,
                                                         "global wind force scaler");
    g_GlobalWindGrassTension = _Dvar_RegisterFloat(
                                                             "wind_grass_tension",
                                                             0.001,
                                                             0.0,
                                                             1.0,
                                                             0x1000u,
                                                             "global wind grass tension (stiffness)");
    g_GlobalWindGustSpeedMultiplier = _Dvar_RegisterFloat(
                                                                            "wind_grass_gust_speed",
                                                                            1.0,
                                                                            0.0,
                                                                            10000.0,
                                                                            0x1000u,
                                                                            "speed multiplier for gusts");
    g_GlobalWindGustRadius = _Dvar_RegisterFloat(
                                                         "wind_grass_gust_radius",
                                                         400.0,
                                                         0.0,
                                                         10000.0,
                                                         0x1000u,
                                                         "size of global wind gust effector");
    g_GlobalWindGustStrength = _Dvar_RegisterFloat(
                                                             "wind_grass_gust_strength",
                                                             0.5,
                                                             0.0,
                                                             1.0,
                                                             0x1000u,
                                                             "percentage to bump the wind in a gust");
    g_GlobalWindGustDistance = _Dvar_RegisterFloat(
                                                             "wind_grass_gust_distance",
                                                             2000.0,
                                                             0.0,
                                                             100000.0,
                                                             0x1000u,
                                                             "max distance at which to place gusts");
    g_GlobalWindDebugDisplay = _Dvar_RegisterBool("wind_debug_display", 0, 0x1080u, "shows wind debug display");
    g_GlobalTreeBend = _Dvar_RegisterFloat("tree_bend", 1.0, 0.0, 5.0, 0x1000u, "bend amount of tree trunk");
    g_GlobalTreeRandom = _Dvar_RegisterFloat(
                                                 "tree_random",
                                                 0.1,
                                                 0.0,
                                                 5.0,
                                                 0x1000u,
                                                 "random addition to bend amount of tree trunk");
    g_GlobalTreeFrequency = _Dvar_RegisterFloat(
                                                        "tree_frequency",
                                                        2.0,
                                                        0.0,
                                                        20.0,
                                                        0x1000u,
                                                        "frequency of tree bend oscillation");
    g_localWindVector = _Dvar_RegisterVec4(
                                                "wind_local_vector",
                                                (0.0),
                                                (1.0),
                                                (0.0),
                                                (0.0),
                                                -1000000.0,
                                                1000000.0,
                                                0x1000u,
                                                "localized wind force in units/sec");
    g_localWindAmplitude = _Dvar_RegisterVec4(
                                                     "wind_local_amplitude",
                                                     (1.0),
                                                     (1.0),
                                                     (1.0),
                                                     (0.0),
                                                     -1000000.0,
                                                     1000000.0,
                                                     0x1000u,
                                                     "localized wind amplitude");
    g_localWindFrequency = _Dvar_RegisterVec3(
                                                     "wind_local_frequency",
                                                     (6.0),
                                                     (6.0),
                                                     (6.0),
                                                     0.0,
                                                     40.0,
                                                     0x1000u,
                                                     "frequency of tree bend oscillation");
    g_localWindGustRadius = _Dvar_RegisterFloat(
                                                        "wind_local_gust_radius",
                                                        1000.0,
                                                        0.0,
                                                        10000.0,
                                                        0x1000u,
                                                        "size of local wind gust effector");
    g_localWindGustStrength = _Dvar_RegisterFloat(
                                                            "wind_local_gust_strength",
                                                            0.80000001,
                                                            0.0,
                                                            1.0,
                                                            0x1000u,
                                                            "percentage to bump the wind in a local gust");
}

void __cdecl DisplayWindDebug()
{
    float value; // [esp+1Ch] [ebp-ACh]
    float start[3]; // [esp+28h] [ebp-A0h] BYREF
    float end[3]; // [esp+34h] [ebp-94h] BYREF
    float debugCircleDir[3]; // [esp+40h] [ebp-88h] BYREF
    int i; // [esp+4Ch] [ebp-7Ch]
    float loWindStart[3]; // [esp+50h] [ebp-78h] BYREF
    float pos[3]; // [esp+5Ch] [ebp-6Ch] BYREF
    float vertLineStart[3]; // [esp+68h] [ebp-60h] BYREF
    float vertColor[4]; // [esp+74h] [ebp-54h] BYREF
    float hiWindEnd[3]; // [esp+84h] [ebp-44h] BYREF
    float lineColor[4]; // [esp+90h] [ebp-38h] BYREF
    float hiWindStart[3]; // [esp+A0h] [ebp-28h] BYREF
    float vertLineEnd[3]; // [esp+ACh] [ebp-1Ch] BYREF
    float loWindEnd[3]; // [esp+B8h] [ebp-10h] BYREF
    cg_s *cgameGlob; // [esp+C4h] [ebp-4h]

    if (g_GlobalWindDebugDisplay)
    {
        cgameGlob = CG_GetLocalClientGlobals(0);
        if (g_GlobalWindDebugDisplay->current.enabled)
        {
            if (!debugValLastFrame && cgameGlob)
            {
                windDebugPos[0] = (float)(100.0 * cgameGlob->refdef.viewaxis[0][0]) + cgameGlob->refdef.vieworg[0];
                windDebugPos[1] = (float)(100.0 * cgameGlob->refdef.viewaxis[0][1]) + cgameGlob->refdef.vieworg[1];
                windDebugPos[2] = (float)(100.0 * cgameGlob->refdef.viewaxis[0][2]) + cgameGlob->refdef.vieworg[2];
            }
            vertLineStart[0] = windDebugPos[0];
            vertLineStart[1] = windDebugPos[1];
            vertLineStart[2] = 10000.0f;
            vertLineEnd[0] = windDebugPos[0];
            vertLineEnd[1] = windDebugPos[1];
            vertLineEnd[2] = -10000.0f;
            loWindStart[0] = windDebugPos[0];
            loWindStart[1] = windDebugPos[1];
            loWindStart[2] = g_GlobalLowWindAltitude->current.value;
            hiWindStart[0] = windDebugPos[0];
            hiWindStart[1] = windDebugPos[1];
            hiWindStart[2] = g_GlobalHiWindAltitude->current.value;
            value = g_GlobalLowWindStrengthPercentage->current.value;
            loWindEnd[0] = (float)(value * g_GlobalWindVector->current.value) + windDebugPos[0];
            loWindEnd[1] = (float)(value * g_GlobalWindVector->current.vector[1]) + windDebugPos[1];
            loWindEnd[2] = (float)(value * g_GlobalWindVector->current.vector[2]) + loWindStart[2];
            hiWindEnd[0] = windDebugPos[0] + g_GlobalWindVector->current.value;
            hiWindEnd[1] = windDebugPos[1] + g_GlobalWindVector->current.vector[1];
            hiWindEnd[2] = hiWindStart[2] + g_GlobalWindVector->current.vector[2];
            vertColor[0] = 0.75f;
            vertColor[1] = 0.75f;
            vertColor[2] = 0.75f;
            vertColor[3] = 1.0f;
            CG_DebugLine(vertLineStart, vertLineEnd, vertColor, 0, 2);
            CG_DebugLine(loWindEnd, hiWindEnd, vertColor, 0, 2);
            lineColor[0] = 1.0f;
            lineColor[1] = 0.0f;
            lineColor[2] = 1.0f;
            lineColor[3] = 1.0f;
            CG_DebugLine(loWindStart, loWindEnd, lineColor, 0, 2);
            CG_DebugLine(hiWindStart, hiWindEnd, lineColor, 0, 2);
            for (i = 0; i < 16; ++i)
            {
                pos[0] = g_GrassWind[0][i].pos[0];
                pos[1] = g_GrassWind[0][i].pos[1];
                pos[2] = 0.0f;
                start[0] = pos[0];
                start[1] = pos[1];
                end[0] = pos[0];
                end[1] = pos[1];
                end[2] = 0.0f;
                start[2] = 1000.0f;
                CM_TracePointDown(start, end, 2097, 0x3F00000, pos, 0, 0);
                G_DebugStar(pos, lineColor, 3);
                debugCircleDir[0] = 0.0f;
                debugCircleDir[1] = 0.0f;
                debugCircleDir[2] = 1.0f;
                G_DebugCircleEx(pos, g_GlobalWindGustRadius->current.value, debugCircleDir, colorWhite, 1, 1);
            }
        }
        debugValLastFrame = g_GlobalWindDebugDisplay->current.enabled;
    }
}

void __cdecl SeedVariantWind(
                VariantWind *wind,
                int i,
                int start_time,
                const float *seedWindVector,
                float windLeafScale)
{
    int v5; // esi
    float v6; // [esp+Ch] [ebp-3Ch]
    float v7; // [esp+10h] [ebp-38h]
    float v8; // [esp+14h] [ebp-34h]
    float v9; // [esp+1Ch] [ebp-2Ch]
    float v10; // [esp+20h] [ebp-28h]
    float v11; // [esp+24h] [ebp-24h]
    float v_scale; // [esp+28h] [ebp-20h]
    float varint_4; // [esp+3Ch] [ebp-Ch]
    float varint_8; // [esp+40h] [ebp-8h]

    wind->reset_timer = start_time - 1875 * (16 - i);
    wind->gust_start = start_time - (int)(float)(g_GlobalWindGrassGustInterval->current.value / 16.0) * (16 - i);
    v5 = (int)(float)(g_GlobalWindGrassGustInterval->current.value / 2.0);
    wind->gust_dur = rand() % v5 + v5;
    varint_4 = g_WindVariants[(i + 1) % 17];
    varint_8 = g_WindVariants[(i + 2) % 17];
    wind->wind_vector[0] = *seedWindVector * g_WindVariants[i % 17];
    wind->wind_vector[1] = seedWindVector[1] * varint_4;
    wind->wind_vector[2] = seedWindVector[2] * varint_8;
    wind->wind_vector[0] = *seedWindVector + wind->wind_vector[0];
    wind->wind_vector[1] = seedWindVector[1] + wind->wind_vector[1];
    wind->wind_vector[2] = seedWindVector[2] + wind->wind_vector[2];
    v_scale = (float)((float)((float)(wind->wind_vector[0] * wind->wind_vector[0])
                                                    + (float)(wind->wind_vector[1] * wind->wind_vector[1]))
                                    + (float)(wind->wind_vector[2] * wind->wind_vector[2]))
                    / (float)(windLeafScale * windLeafScale);
    v9 = (float)(rand() - 0x3FFF) / 32767.0;
    v10 = (float)(rand() - 0x3FFF) / 32767.0;
    v11 = (float)(rand() - 0x3FFF) / 32767.0;
    wind->rustle_amplitude[0] = v_scale * v9;
    wind->rustle_amplitude[1] = v_scale * v10;
    wind->rustle_amplitude[2] = v_scale * v11;
    v6 = (float)(rand() - 0x3FFF) / 32767.0;
    v7 = (float)(rand() - 0x3FFF) / 32767.0;
    v8 = (float)(rand() - 0x3FFF) / 32767.0;
    wind->rustle_angular_frequency[0] = (float)(v_scale * 3.1415927) * v6;
    wind->rustle_angular_frequency[1] = (float)(v_scale * 3.1415927) * v7;
    wind->rustle_angular_frequency[2] = (float)(v_scale * 3.1415927) * v8;
}

void __cdecl BG_SeedVarintWinds(unsigned int variant_seed, int start_time)
{
    int j; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]
    int ia; // [esp+8h] [ebp-4h]
    int ib; // [esp+8h] [ebp-4h]

    srand(variant_seed);
    for (i = 0; i < 17; ++i)
        g_WindVariants[i] = (float)((float)(rand() - 0x3FFF) * 0.1) / 32767.0;
    for (ia = 0; ia < 16; ++ia)
        SeedVariantWind(
            &g_VariantWind[ia],
            ia,
            start_time,
            &g_GlobalWindVector->current.value,
            g_GlobalWindLeafScale->current.value);
    SeedVariantWind(
        &g_localWind.wind,
        0,
        start_time,
        &g_GlobalWindVector->current.value,
        g_GlobalWindLeafScale->current.value);
    for (j = 0; j < 1; ++j)
    {
        for (ib = 0; ib < 16; ++ib)
        {
            g_GrassWind[j][ib].start_time = 0;
            g_GrassWind[j][ib].life_span = 0;
        }
    }
}

void    BG_UpdateVariantWind(
                VariantWind *wind,
                int variantIndex,
                const float *seedWindVector,
                int cur_time)
{
    if (wind->reset_timer <= cur_time)
    {
        wind->reset_timer = cur_time + 30000;

        const float v0 = g_WindVariants[variantIndex % 17];
        const float v1 = g_WindVariants[(variantIndex + 1) % 17];
        const float v2 = g_WindVariants[(variantIndex + 2) % 17];

        wind->wind_vector[0] = seedWindVector[0] * v0 + seedWindVector[0];
        wind->wind_vector[1] = seedWindVector[1] * v1 + seedWindVector[1];
        wind->wind_vector[2] = seedWindVector[2] * v2 + seedWindVector[2];
    }

    if (wind->gust_start + wind->gust_dur <= cur_time)
    {
        wind->gust_start = cur_time;

        const int halfInterval =
            (int)(g_GlobalWindGrassGustInterval->current.value * 0.5f);

        wind->gust_dur = (rand() % halfInterval) + halfInterval;

        const float mag2 =
            wind->wind_vector[0] * wind->wind_vector[0]
            + wind->wind_vector[1] * wind->wind_vector[1]
            + wind->wind_vector[2] * wind->wind_vector[2];

        const float gustStrength =
            Com_Random() * (-mag2) + mag2 + 1.0f;

        wind->blade_acceleration =
            gustStrength / g_GlobalWindGrassScale->current.value;
    }

    wind->blade_velocity += wind->blade_acceleration;

    wind->blade_acceleration += -g_GlobalWindGrassTension->current.value * wind->blade_velocity;
}

unsigned int _S1_0;
int last_time;
float g_LastGlobalWindVector[4];
void CG_UpdateWind(int cur_time)
{
    float v3; // xmm0_4
    float v4; // xmm0_4
    double v5; // st7
    int v6; // eax
    float v7; // xmm0_4
    float v8; // xmm0_4
    FX_WindInfo fxWind; // [esp+18h] [ebp-FCh] BYREF
    const DvarValue *v10; // [esp+34h] [ebp-E0h]
    float treeBend; // [esp+38h] [ebp-DCh]
    float v12; // [esp+3Ch] [ebp-D8h]
    float v13; // [esp+40h] [ebp-D4h]
    float v14; // [esp+44h] [ebp-D0h]
    float v15; // [esp+48h] [ebp-CCh]
    float v16; // [esp+4Ch] [ebp-C8h]
    float v17; // [esp+50h] [ebp-C4h]
    GrassWind *v18; // [esp+54h] [ebp-C0h]
    GrassWind *v19; // [esp+58h] [ebp-BCh]
    float *dir; // [esp+5Ch] [ebp-B8h]
    float v21; // [esp+60h] [ebp-B4h]
    float v22; // [esp+64h] [ebp-B0h]
    float scalar; // [esp+68h] [ebp-ACh]
    GrassWind *v24; // [esp+6Ch] [ebp-A8h]
    GrassWind *v25; // [esp+70h] [ebp-A4h]
    float v26; // [esp+74h] [ebp-A0h]
    float rot[3]; // [esp+78h] [ebp-9Ch]
    float v28; // [esp+84h] [ebp-90h]
    float dist[3]; // [esp+88h] [ebp-8Ch] BYREF
    float *v30; // [esp+94h] [ebp-80h]
    float *origin; // [esp+98h] [ebp-7Ch]
    GrassWind *v32; // [esp+9Ch] [ebp-78h]
    centity_s *ent; // [esp+A0h] [ebp-74h]
    float v34[5]; // [esp+A4h] [ebp-70h] BYREF
    float v35; // [esp+B8h] [ebp-5Ch]
    float v36; // [esp+BCh] [ebp-58h]
    cg_s *cgameGlob; // [esp+C0h] [ebp-54h]
    int localClientNum; // [esp+C4h] [ebp-50h]
    float windRustleAngle[5]; // [esp+C8h] [ebp-4Ch] BYREF
    float v40; // [esp+DCh] [ebp-38h]
    float v41; // [esp+E0h] [ebp-34h]
    int j; // [esp+E4h] [ebp-30h]
    int i; // [esp+E8h] [ebp-2Ch]
    const DvarValue *v44; // [esp+ECh] [ebp-28h]
    const DvarValue *p_current; // [esp+F4h] [ebp-20h]
    int v47; // [esp+104h] [ebp-10h]
    //_UNKNOWN *v48; // [esp+108h] [ebp-Ch]
    //int cur_timea; // [esp+10Ch] [ebp-8h]
    //int vars0; // [esp+114h] [ebp+0h]
    //
    //v48 = a1;
    //cur_timea = vars0;
    // 

    PROF_SCOPED("CG_UpdateWind");

    if ((_S1_0 & 1) == 0)
    {
        _S1_0 |= 1u;
        last_time = cur_time;
    }
    if (g_EnableGlobalWind->current.enabled)
    {
        if (cg_paused->current.integer)
        {
            last_time = cur_time;
        }
        else
        {
            p_current = &g_GlobalWindVector->current;
            if (g_GlobalWindVector->current.value != g_LastGlobalWindVector[0]
                || p_current->vector[1] != g_LastGlobalWindVector[1]
                || p_current->vector[2] != g_LastGlobalWindVector[2]
                || p_current->vector[3] != g_LastGlobalWindVector[3])
            {
                v44 = &g_GlobalWindVector->current;
                *(DvarValue *)g_LastGlobalWindVector = g_GlobalWindVector->current;
                BG_SeedVarintWinds(0x7FFFu, cur_time);
            }
            for (i = 0; i < 16; ++i)
            {
                BG_UpdateVariantWind(&g_VariantWind[i], i, &g_GlobalWindVector->current.value, cur_time);
                for (j = 0; j < 3; ++j)
                {
                    v41 = (float)((float)cur_time / 1000.0) * g_VariantWind[i].rustle_angular_frequency[j];
                    v40 = fmod(v41, 6.2831855);
                    windRustleAngle[4] = v40;
                    v3 = sin(v40);
                    windRustleAngle[3] = v3;
                    windRustleAngle[j] = g_VariantWind[i].rustle_amplitude[j] * v3;
                }
                R_SetVariantWindRustleAngle(i, windRustleAngle);
                R_SetVariantWindSpringAngle(i, g_VariantWind[i].blade_velocity);
            }
            if (g_EnableGrassWind->current.enabled)
            {
                for (localClientNum = 0; localClientNum < 1; ++localClientNum)
                {
                    if (CL_LocalClient_IsActive(localClientNum))
                    {
                        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
                        if (cgameGlob)
                        {
                            for (i = 0; i < 16; ++i)
                            {
                                if (i || g_localWind.entnum == 1023)
                                {
                                    //a2 = 28 * i;
                                    if (g_GrassWind[localClientNum][i].life_span + g_GrassWind[localClientNum][i].start_time >= cur_time)
                                    {
                                        scalar = (float)(cur_time - last_time) / 1000.0;
                                        v22 = scalar;
                                        dir = g_GrassWind[localClientNum][i].dir;
                                        v21 = scalar * *dir;
                                        v22 = scalar * dir[1];
                                        v19 = &g_GrassWind[localClientNum][i];
                                        v18 = v19;
                                        v19->pos[0] = v19->pos[0] + v21;
                                        v19->pos[1] = v18->pos[1] + v22;
                                    }
                                    else
                                    {
                                        g_GrassWind[localClientNum][i].dir[0] = (float)(g_GlobalWindVector->current.value
                                            * g_WindVariants[i % 17])
                                            + g_GlobalWindVector->current.value;
                                        g_GrassWind[localClientNum][i].dir[1] = (float)(g_GlobalWindVector->current.vector[1]
                                            * g_WindVariants[(i + 1) % 17])
                                            + g_GlobalWindVector->current.vector[1];
                                        g_GrassWind[localClientNum][i].dir[0] = g_GrassWind[localClientNum][i].dir[0]
                                            * g_GlobalWindGustSpeedMultiplier->current.value;
                                        g_GrassWind[localClientNum][i].dir[1] = g_GrassWind[localClientNum][i].dir[1]
                                            * g_GlobalWindGustSpeedMultiplier->current.value;
                                        v5 = flrand(0.1, 1.0);
                                        g_GrassWind[localClientNum][i].strength_skew = v5;
                                        g_GrassWind[localClientNum][i].start_time = cur_time;
                                        v6 = irand(5000, 10000);
                                        g_GrassWind[localClientNum][i].life_span = v6;
                                        v30 = g_GrassWind[localClientNum][i].dir;
                                        dist[0] = *v30;
                                        dist[1] = v30[1];
                                        dist[2] = v30[2];
                                        //LODWORD(dist[0]) ^= _mask__NegFloat_;
                                        //LODWORD(dist[1]) ^= _mask__NegFloat_;
                                        //LODWORD(dist[2]) ^= _mask__NegFloat_;
                                        dist[0] = -dist[0];
                                        dist[1] = -dist[1];
                                        dist[2] = -dist[2];
                                        v28 = (float)((float)g_GrassWind[localClientNum][i].life_span / 2.0) / 1000.0;
                                        dist[0] = v28 * dist[0];
                                        dist[1] = v28 * dist[1];
                                        dist[2] = v28 * dist[2];
                                        g_GrassWind[localClientNum][i].pos[0] = cgameGlob->refdef.vieworg[0] + dist[0];
                                        g_GrassWind[localClientNum][i].pos[1] = cgameGlob->refdef.vieworg[1] + dist[1];
                                        Vec3Normalize(dist);
                                        rot[0] = -dist[1];
                                        rot[1] = dist[0];
                                        rot[2] = 0.0f;
                                        v26 = flrand(-g_GlobalWindGustDistance->current.value, g_GlobalWindGustDistance->current.value);
                                        rot[0] = v26 * rot[0];
                                        rot[1] = v26 * rot[1];
                                        rot[2] = v26 * 0.0;
                                        v25 = &g_GrassWind[localClientNum][i];
                                        v24 = v25;
                                        v25->pos[0] = v25->pos[0] + rot[0];
                                        v25->pos[1] = v24->pos[1] + rot[1];
                                        v25->dir[0] = v24->dir[0] + rot[2];
                                    }
                                    v17 = (float)((float)(cur_time - g_GrassWind[localClientNum][i].start_time)
                                        / (float)g_GrassWind[localClientNum][i].life_span)
                                        * 3.1415927;
                                    v16 = fmod(v17, 6.2831855);
                                    v15 = v16;
                                    v7 = sin(v16);
                                    v14 = v7;
                                    R_FoliageSetGrassWindForces(
                                        localClientNum,
                                        g_GrassWind[localClientNum][i].pos,
                                        g_GlobalWindGustRadius->current.value,
                                        (float)(g_GlobalWindGustStrength->current.value * g_GrassWind[localClientNum][i].strength_skew) * v7);
                                }
                                else
                                {
                                    BG_UpdateVariantWind(&g_localWind.wind, 0, &g_localWindVector->current.value, cur_time);
                                    for (j = 0; j < 3; ++j)
                                    {
                                        v36 = (float)((float)cur_time / 1000.0) * g_localWindFrequency->current.vector[j];
                                        v35 = fmod(v36, 6.2831855);
                                        v34[4] = v35;
                                        v4 = sin(v35);
                                        v34[3] = v4;
                                        v34[j] = g_localWindAmplitude->current.vector[j] * v4;
                                    }
                                    ent = CG_GetEntity(localClientNum, g_localWind.entnum);
                                    R_SetLocalWind(ent->pose.origin, g_GlobalWindGustRadius->current.value, v34);
                                    v32 = g_GrassWind[localClientNum];
                                    origin = ent->pose.origin;
                                    v32->pos[0] = ent->pose.origin[0];
                                    v32->pos[1] = origin[1];
                                    R_FoliageSetGrassWindForces(
                                        localClientNum,
                                        ent->pose.origin,
                                        g_localWindGustRadius->current.value,
                                        g_localWindGustStrength->current.value);
                                }
                            }
                        }
                    }
                }
            }
            DisplayWindDebug();
            v13 = (float)((float)cur_time * g_GlobalTreeFrequency->current.value) / 1000.0;
            v8 = sin(v13);
            v12 = v8;
            treeBend = (float)(g_GlobalTreeRandom->current.value * v8) + g_GlobalTreeBend->current.value;
            R_SetWindDirection((float*)&g_GlobalWindVector->current.value, treeBend);
            v10 = &g_GlobalWindVector->current;
            LODWORD(fxWind.windVector[0]) = g_GlobalWindVector->current.integer;
            fxWind.windVector[1] = g_GlobalWindVector->current.vector[1];
            fxWind.windVector[2] = g_GlobalWindVector->current.vector[2];
            LODWORD(fxWind.hiWindAltitude) = g_GlobalHiWindAltitude->current.integer;
            LODWORD(fxWind.lowWindAltitude) = g_GlobalLowWindAltitude->current.integer;
            LODWORD(fxWind.lowWindStrengthPercent) = g_GlobalLowWindStrengthPercentage->current.integer;
            FX_SetGlobalWind(&fxWind);
            last_time = cur_time;
        }
    }
    else
    {
        v47 = 0;
    }
}

void __cdecl BG_GetVariantWindVector(int which, float *out)
{
    VariantWind *v2; // eax

    if ( which >= 16
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_wind.cpp",
                    511,
                    0,
                    "%s",
                    "which<MAX_VARIANT_WIND") )
    {
        __debugbreak();
    }
    *out = g_VariantWind[which].wind_vector[0];
    v2 = &g_VariantWind[which];
    out[1] = v2->wind_vector[1];
    out[2] = v2->wind_vector[2];
}

void __cdecl BG_GetGlobalWind(float *out)
{
    const DvarValue *p_current; // [esp+0h] [ebp-4h]

    if ( g_GlobalWindVector )
    {
        p_current = &g_GlobalWindVector->current;
        *out = g_GlobalWindVector->current.value;
        out[1] = p_current->vector[1];
        out[2] = p_current->vector[2];
    }
    else
    {
        *out = 0.0f;
        out[1] = 0.0f;
        out[2] = 0.0f;
    }
}

void __cdecl CG_EnableLocalWindSource(int entnum)
{
    g_localWind.entnum = entnum;
}

double __cdecl CG_GetLocalWindSourceScale(int localClientNum, float *pos)
{
    float gust_dist; // [esp+14h] [ebp-Ch]
    centity_s *ent; // [esp+18h] [ebp-8h]
    float ent_dist; // [esp+1Ch] [ebp-4h]

    if ( g_localWind.entnum == 1023 )
        return 0.0;
    ent = CG_GetEntity(localClientNum, g_localWind.entnum);
    gust_dist = g_GlobalWindGustRadius->current.value * g_GlobalWindGustRadius->current.value;
    ent_dist = Vec3DistanceSq(pos, ent->pose.origin);
    if ( ent_dist <= gust_dist )
        return (gust_dist - ent_dist) / gust_dist;
    else
        return 0.0;
}

