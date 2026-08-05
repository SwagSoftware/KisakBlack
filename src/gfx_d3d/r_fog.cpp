#include "r_fog.h"

#include <cstring>
#include "r_init.h"

void __cdecl R_ClearFogs()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 4; ++i )
    {
        memset(&rg.clientFogs[i], 0, 0x190u);
        rg.clientFogs[i].index = 0;
    }
}

void __cdecl R_SetFogFromServer(
                unsigned int localClientNum,
                float start,
                float r,
                float g,
                float b,
                float density,
                float heightDensity,
                float baseHeight,
                float fogColorScale,
                float sunFogColorR,
                float sunFogColorG,
                float sunFogColorB,
                float sunFogDirX,
                float sunFogDirY,
                float sunFogDirZ,
                float sunFogStartAng,
                float sunFogEndAng,
                float fogMaxOpacity)
{
    GfxClientFog *fog; // [esp+0h] [ebp-4h]

    if ( localClientNum >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_fog.cpp",
                    32,
                    0,
                    "localClientNum doesn't index ARRAY_COUNT(rg.clientFogs)\n\t%i not in [0, %i)",
                    localClientNum,
                    4) )
    {
        __debugbreak();
    }
    fog = &rg.clientFogs[localClientNum];
    fog->settings[1].fogStart = start;
    fog->settings[1].heightDensity = heightDensity;
    fog->settings[1].density = density;
    fog->settings[1].baseHeight = baseHeight;
    fog->settings[1].color[0] = r;
    fog->settings[1].color[1] = g;
    fog->settings[1].color[2] = b;
    fog->settings[1].color[3] = fogColorScale;
    fog->settings[1].sunFogColor[0] = sunFogColorR;
    fog->settings[1].sunFogColor[1] = sunFogColorG;
    fog->settings[1].sunFogColor[2] = sunFogColorB;
    fog->settings[1].sunFogColor[3] = fogMaxOpacity;
    fog->settings[1].sunFogDir[0] = sunFogDirX;
    fog->settings[1].sunFogDir[1] = sunFogDirY;
    fog->settings[1].sunFogDir[2] = sunFogDirZ;
    fog->settings[1].sunFogEndAng = sunFogEndAng;
    fog->settings[1].sunFogStartAng = sunFogStartAng;
    fog->settings[1].maxDensity = 100.0 * density;
}

void __cdecl R_GetFogSettings(float *fogSettings)
{
    float *fogSettingsa; // [esp+Ch] [ebp+8h]

    *fogSettings = rg.clientFogs[0].settings[1].fogStart;
    fogSettingsa = fogSettings + 1;
    *fogSettingsa++ = 1.0 / rg.clientFogs[0].settings[1].density;
    *fogSettingsa++ = rg.clientFogs[0].settings[1].baseHeight;
    *fogSettingsa++ = 1.0 / rg.clientFogs[0].settings[1].heightDensity;
    *fogSettingsa++ = rg.clientFogs[0].settings[1].color[0];
    *fogSettingsa++ = rg.clientFogs[0].settings[1].color[1];
    *fogSettingsa++ = rg.clientFogs[0].settings[1].color[2];
    *fogSettingsa++ = rg.clientFogs[0].settings[1].color[3];
    *fogSettingsa++ = rg.clientFogs[0].settings[1].sunFogColor[0];
    *fogSettingsa++ = rg.clientFogs[0].settings[1].sunFogColor[1];
    *fogSettingsa++ = rg.clientFogs[0].settings[1].sunFogColor[2];
    *fogSettingsa++ = rg.clientFogs[0].settings[1].sunFogDir[0];
    *fogSettingsa++ = rg.clientFogs[0].settings[1].sunFogDir[1];
    *fogSettingsa++ = rg.clientFogs[0].settings[1].sunFogDir[2];
    *fogSettingsa++ = rg.clientFogs[0].settings[1].sunFogStartAng;
    *fogSettingsa = rg.clientFogs[0].settings[1].sunFogEndAng;
    fogSettingsa[1] = rg.clientFogs[0].settings[1].sunFogColor[3];
}

void __cdecl R_SwitchFog(unsigned int localClientNum, unsigned int fogvar, int startTime, int transitionTime)
{
    GfxClientFog *fog; // [esp+8h] [ebp-4h]

    if ( localClientNum >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_fog.cpp",
                    91,
                    0,
                    "localClientNum doesn't index ARRAY_COUNT(rg.clientFogs)\n\t%i not in [0, %i)",
                    localClientNum,
                    4) )
    {
        __debugbreak();
    }
    if ( fogvar > 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_fog.cpp",
                    92,
                    0,
                    "%s\n\t(fogvar) = %i",
                    "(fogvar >= FOG_NONE && fogvar < FOG_COUNT)",
                    fogvar) )
    {
        __debugbreak();
    }
    fog = &rg.clientFogs[localClientNum];
    fog->index = fogvar;
    if ( fog->settings[2].density == 0.0 )
    {
        memcpy(&fog->settings[3], &fog->settings[fog->index], sizeof(fog->settings[3]));
        transitionTime = 0;
    }
    else
    {
        memcpy(&fog->settings[3], &fog->settings[2], sizeof(fog->settings[3]));
    }
    memcpy(&fog->settings[4], &fog->settings[fog->index], sizeof(fog->settings[4]));
    if ( transitionTime )
    {
        fog->settings[4].startTime = startTime;
        fog->settings[4].finishTime = transitionTime + startTime;
    }
    else
    {
        fog->settings[4].startTime = 0;
        fog->settings[4].finishTime = 0;
    }
}

