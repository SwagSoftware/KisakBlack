#include "r_sky.h"
#include <universal/assertive.h>
#include <universal/com_files.h>
#include <universal/q_shared.h>
#include <universal/com_math_anglevectors.h>
#include <qcommon/common.h>
#include <qcommon/cmd.h>
#include "r_dvars.h"
#include "r_init.h"
#include "rb_sky.h"

const dvar_t *r_sunsprite_shader;
const dvar_t *r_sunsprite_size;
const dvar_t *r_sunflare_shader;
const dvar_t *r_sunflare_min_size;
const dvar_t *r_sunflare_min_angle;
const dvar_t *r_sunflare_max_size;
const dvar_t *r_sunflare_max_angle;
const dvar_t *r_sunflare_max_alpha;
const dvar_t *r_sunflare_fadein;
const dvar_t *r_sunflare_fadeout;
const dvar_t *r_sunblind_min_angle;
const dvar_t *r_sunblind_max_angle;
const dvar_t *r_sunblind_max_darken;
const dvar_t *r_sunblind_fadein;
const dvar_t *r_sunblind_fadeout;
const dvar_t *r_sunglare_min_angle;
const dvar_t *r_sunglare_max_angle;
const dvar_t *r_sunglare_max_lighten;
const dvar_t *r_sunglare_fadein;
const dvar_t *r_sunglare_fadeout;
const dvar_t *r_sun_fx_position;

const char *sunDvarNames[21] =
{
  "r_sunsprite_shader",
  "r_sunsprite_size",
  "r_sunflare_shader",
  "r_sunflare_min_size",
  "r_sunflare_min_angle",
  "r_sunflare_max_size",
  "r_sunflare_max_angle",
  "r_sunflare_max_alpha",
  "r_sunflare_fadein",
  "r_sunflare_fadeout",
  "r_sunblind_min_angle",
  "r_sunblind_max_angle",
  "r_sunblind_max_darken",
  "r_sunblind_fadein",
  "r_sunblind_fadeout",
  "r_sunglare_min_angle",
  "r_sunglare_max_angle",
  "r_sunglare_max_lighten",
  "r_sunglare_fadein",
  "r_sunglare_fadeout",
  "r_sun_fx_position"
};


void __cdecl R_RegisterSunDvars()
{
    r_sunsprite_shader = _Dvar_RegisterString(
                                                 "r_sunsprite_shader",
                                                 "sun",
                                                 0,
                                                 "name for static sprite; can be any material");
    r_sunsprite_size = _Dvar_RegisterFloat(
                                             "r_sunsprite_size",
                                             16.0,
                                             1.0,
                                             1000.0,
                                             0,
                                             "diameter in pixels at 640x480 and 80 fov");
    r_sunflare_shader = _Dvar_RegisterString(
                                                "r_sunflare_shader",
                                                "sun_flare",
                                                0,
                                                "name for flare effect; can be any material");
    r_sunflare_min_size = _Dvar_RegisterFloat(
                                                    "r_sunflare_min_size",
                                                    0.0,
                                                    0.0,
                                                    10000.0,
                                                    0,
                                                    "smallest size of flare effect in pixels at 640x480");
    r_sunflare_min_angle = _Dvar_RegisterFloat(
                                                     "r_sunflare_min_angle",
                                                     45.0,
                                                     0.0,
                                                     90.0,
                                                     0,
                                                     "angle from sun in degrees outside which effect is 0");
    r_sunflare_max_size = _Dvar_RegisterFloat(
                                                    "r_sunflare_max_size",
                                                    2500.0,
                                                    0.0,
                                                    10000.0,
                                                    0,
                                                    "largest size of flare effect in pixels at 640x480");
    r_sunflare_max_angle = _Dvar_RegisterFloat(
                                                     "r_sunflare_max_angle",
                                                     2.0,
                                                     0.0,
                                                     90.0,
                                                     0,
                                                     "angle from sun in degrees inside which effect is max");
    r_sunflare_max_alpha = _Dvar_RegisterFloat(
                                                     "r_sunflare_max_alpha",
                                                     1.0,
                                                     0.0,
                                                     1.0,
                                                     0,
                                                     "0-1 vertex color and alpha of sun at max effect");
    r_sunflare_fadein = _Dvar_RegisterFloat(
                                                "r_sunflare_fadein",
                                                1.0,
                                                0.0,
                                                60.0,
                                                0,
                                                "time in seconds to fade alpha from 0% to 100%");
    r_sunflare_fadeout = _Dvar_RegisterFloat(
                                                 "r_sunflare_fadeout",
                                                 1.0,
                                                 0.0,
                                                 60.0,
                                                 0,
                                                 "time in seconds to fade alpha from 100% to 0%");
    r_sunblind_min_angle = _Dvar_RegisterFloat(
                                                     "r_sunblind_min_angle",
                                                     30.0,
                                                     0.0,
                                                     90.0,
                                                     0,
                                                     "angle from sun in degrees outside which blinding is 0");
    r_sunblind_max_angle = _Dvar_RegisterFloat(
                                                     "r_sunblind_max_angle",
                                                     5.0,
                                                     0.0,
                                                     90.0,
                                                     0,
                                                     "angle from sun in degrees inside which blinding is max");
    r_sunblind_max_darken = _Dvar_RegisterFloat(
                                                        "r_sunblind_max_darken",
                                                        0.75,
                                                        0.0,
                                                        1.0,
                                                        0,
                                                        "0-1 fraction for how black the world is at max blind");
    r_sunblind_fadein = _Dvar_RegisterFloat(
                                                "r_sunblind_fadein",
                                                0.5,
                                                0.0,
                                                60.0,
                                                0,
                                                "time in seconds to fade blind from 0% to 100%");
    r_sunblind_fadeout = _Dvar_RegisterFloat(
                                                 "r_sunblind_fadeout",
                                                 3.0,
                                                 0.0,
                                                 60.0,
                                                 0,
                                                 "time in seconds to fade blind from 100% to 0%");
    r_sunglare_min_angle = _Dvar_RegisterFloat(
                                                     "r_sunglare_min_angle",
                                                     30.0,
                                                     0.0,
                                                     90.0,
                                                     0,
                                                     "angle from sun in degrees inside which glare is maximum");
    r_sunglare_max_angle = _Dvar_RegisterFloat(
                                                     "r_sunglare_max_angle",
                                                     5.0,
                                                     0.0,
                                                     90.0,
                                                     0,
                                                     "angle from sun in degrees inside which glare is minimum");
    r_sunglare_max_lighten = _Dvar_RegisterFloat(
                                                         "r_sunglare_max_lighten",
                                                         0.75,
                                                         0.0,
                                                         1.0,
                                                         0,
                                                         "0-1 fraction for how white the world is at max glare");
    r_sunglare_fadein = _Dvar_RegisterFloat(
                                                "r_sunglare_fadein",
                                                0.5,
                                                0.0,
                                                60.0,
                                                0,
                                                "time in seconds to fade glare from 0% to 100%");
    r_sunglare_fadeout = _Dvar_RegisterFloat(
                                                 "r_sunglare_fadeout",
                                                 3.0,
                                                 0.0,
                                                 60.0,
                                                 0,
                                                 "time in seconds to fade glare from 100% to 0%");
    r_sun_fx_position = _Dvar_RegisterVec3(
                                                "r_sun_fx_position",
                                                (0.0),
                                                (0.0),
                                                (0.0),
                                                -360.0,
                                                360.0,
                                                0,
                                                "Position in degrees of the sun effect");
}

#if 0
void __cdecl R_SetSunFromDvars(sunflare_t *sun)
{
    double v1; // xmm0_8
    double v2; // xmm0_8
    double v3; // xmm0_8
    double v4; // xmm0_8
    double v5; // xmm0_8
    double v6; // xmm0_8
    long double v7; // [esp+0h] [ebp-90h]
    long double v8; // [esp+0h] [ebp-90h]
    long double v9; // [esp+0h] [ebp-90h]
    long double v10; // [esp+0h] [ebp-90h]
    long double v11; // [esp+0h] [ebp-90h]
    long double v12; // [esp+0h] [ebp-90h]

    if ( !sun && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_sky.cpp", 102, 0, "%s", "sun") )
        __debugbreak();
    sun->spriteMaterial = Material_RegisterHandle((char *)r_sunsprite_shader->current.integer, 6);
    sun->spriteSize = r_sunsprite_size->current.value;
    sun->flareMaterial = Material_RegisterHandle((char *)r_sunflare_shader->current.integer, 6);
    sun->flareMinSize = r_sunflare_min_size->current.value * 0.5;
    v1 = (float)DEG2RAD(r_sunflare_min_angle->current.value);
    __libm_sse2_cos(v7);
    *(float *)&v1 = v1;
    sun->flareMinDot = *(float *)&v1;
    sun->flareMaxSize = r_sunflare_max_size->current.value * 0.5;
    v2 = (float)DEG2RAD(r_sunflare_max_angle->current.value);
    __libm_sse2_cos(v8);
    *(float *)&v2 = v2;
    sun->flareMaxDot = *(float *)&v2;
    sun->flareMaxAlpha = r_sunflare_max_alpha->current.value;
    sun->flareFadeInTime = SnapFloatToInt((float)(r_sunflare_fadein->current.value * 1000.0));
    sun->flareFadeOutTime = SnapFloatToInt((float)(r_sunflare_fadeout->current.value * 1000.0));
    v3 = (float)DEG2RAD(r_sunblind_min_angle->current.value);
    __libm_sse2_cos(v9);
    *(float *)&v3 = v3;
    sun->blindMinDot = *(float *)&v3;
    v4 = (float)DEG2RAD(r_sunblind_max_angle->current.value);
    __libm_sse2_cos(v10);
    *(float *)&v4 = v4;
    sun->blindMaxDot = *(float *)&v4;
    sun->blindMaxDarken = r_sunblind_max_darken->current.value;
    sun->blindFadeInTime = SnapFloatToInt((float)(r_sunblind_fadein->current.value * 1000.0));
    sun->blindFadeOutTime = SnapFloatToInt((float)(r_sunblind_fadeout->current.value * 1000.0));
    v5 = (float)DEG2RAD(r_sunglare_min_angle->current.value);
    __libm_sse2_cos(v11);
    *(float *)&v5 = v5;
    sun->glareMinDot = *(float *)&v5;
    v6 = (float)DEG2RAD(r_sunglare_max_angle->current.value);
    __libm_sse2_cos(v12);
    *(float *)&v6 = v6;
    sun->glareMaxDot = *(float *)&v6;
    sun->glareMaxLighten = r_sunglare_max_lighten->current.value;
    sun->glareFadeInTime = SnapFloatToInt((float)(r_sunglare_fadein->current.value * 1000.0));
    sun->glareFadeOutTime = SnapFloatToInt((float)(r_sunglare_fadeout->current.value * 1000.0));
    AngleVectors(&r_sun_fx_position->current.value, sun->sunFxPosition, 0, 0);
    sun->hasValidData = 1;
}
#endif

// aislop
void __cdecl R_SetSunFromDvars(sunflare_t *sun)
{
    iassert(sun);

    sun->spriteMaterial = Material_RegisterHandle((char *)r_sunsprite_shader->current.integer, 6);
    sun->spriteSize = r_sunsprite_size->current.value;
    sun->flareMaterial = Material_RegisterHandle((char *)r_sunflare_shader->current.integer, 6);
    sun->flareMinSize = r_sunflare_min_size->current.value * 0.5f;
    sun->flareMinDot = cosf(DEG2RAD(r_sunflare_min_angle->current.value));
    sun->flareMaxSize = r_sunflare_max_size->current.value * 0.5f;
    sun->flareMaxDot = cosf(DEG2RAD(r_sunflare_max_angle->current.value));
    sun->flareMaxAlpha = r_sunflare_max_alpha->current.value;
    sun->flareFadeInTime = (int)(r_sunflare_fadein->current.value * 1000.0f + 9.3132257e-10f);
    sun->flareFadeOutTime = (int)(r_sunflare_fadeout->current.value * 1000.0f + 9.3132257e-10f);
    sun->blindMinDot = cosf(DEG2RAD(r_sunblind_min_angle->current.value));
    sun->blindMaxDot = cosf(DEG2RAD(r_sunblind_max_angle->current.value));
    sun->blindMaxDarken = r_sunblind_max_darken->current.value;
    sun->blindFadeInTime = (int)(r_sunblind_fadein->current.value * 1000.0f + 9.3132257e-10f);
    sun->blindFadeOutTime = (int)(r_sunblind_fadeout->current.value * 1000.0f + 9.3132257e-10f);
    sun->glareMinDot = cosf(DEG2RAD(r_sunglare_min_angle->current.value));
    sun->glareMaxDot = cosf(DEG2RAD(r_sunglare_max_angle->current.value));
    sun->glareMaxLighten = r_sunglare_max_lighten->current.value;
    sun->glareFadeInTime = (int)(r_sunglare_fadein->current.value * 1000.0f + 9.3132257e-10f);
    sun->glareFadeOutTime = (int)(r_sunglare_fadeout->current.value * 1000.0f + 9.3132257e-10f);
    AngleVectors(&r_sun_fx_position->current.value, sun->sunFxPosition, NULL, NULL);
    sun->hasValidData = 1;
}


void __cdecl R_LoadSunThroughDvars(const char *sunName, sunflare_t *sun)
{
    unsigned int SunDvarCount; // eax
    char *v3; // [esp-8h] [ebp-14h]
    char *fullpath; // [esp+0h] [ebp-Ch]
    char *sunFile; // [esp+8h] [ebp-4h] BYREF

    if ( !sunName && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_sky.cpp", 148, 0, "%s", "sunName") )
        __debugbreak();
    if ( !sun && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_sky.cpp", 149, 0, "%s", "sun") )
        __debugbreak();
    fullpath = va("sun/%s.sun", sunName);
    if ( FS_ReadFile(fullpath, (void **)&sunFile) >= 0 )
    {
        v3 = sunFile;
        SunDvarCount = R_GetSunDvarCount();
        if ( Com_LoadDvarsFromBuffer(sunDvarNames, SunDvarCount, v3, fullpath) )
            R_SetSunFromDvars(sun);
        FS_FreeFile(sunFile);
    }
    else
    {
        Com_Printf(8, "WARNING: couldn't load sun file '%s'\n", fullpath);
    }
}

unsigned int __cdecl R_GetSunDvarCount()
{
    return 21;
}

void __cdecl R_Cmd_LoadSun()
{
    const char *v0; // eax
    sunflare_t *p_sun; // [esp-4h] [ebp-14h]

    if ( Cmd_Argc() == 2 )
    {
        if ( sv_cheats->current.enabled )
        {
            if ( rgp.world )
            {
                p_sun = &rgp.world->sun;
                v0 = Cmd_Argv(1);
                R_LoadSunThroughDvars(v0, p_sun);
            }
            else
            {
                Com_Printf(8, "You can't r_loadsun while a map isn't loaded\n");
            }
        }
        else
        {
            Com_Printf(8, "You must have cheats enabled to use r_loadsun\n");
        }
    }
    else
    {
        Com_Printf(8, "USAGE: r_loadsun <sunname>\n    sunname must not have an extension\n");
    }
}

void __cdecl R_Cmd_SaveSun()
{
    const char *v0; // eax

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        R_SaveSunFromDvars(v0);
    }
    else
    {
        Com_Printf(8, "USAGE: r_savesun <sunname>\n    sunname must not have an extension\n");
    }
}

void __cdecl R_SaveSunFromDvars(const char *sunName)
{
    char *v1; // eax
    char buffer; // [esp+10h] [ebp-2008h] BYREF
    _BYTE v3[3]; // [esp+11h] [ebp-2007h] BYREF

    if ( Com_SaveDvarsToBuffer(sunDvarNames, 0x15u, &buffer, 0x2000u) )
    {
        v1 = va("sun/%s.sun", sunName);
        FS_WriteFile(v1, &buffer, &v3[strlen(&buffer)] - v3);
    }
}

void __cdecl R_FlushSun()
{
    SunFlareDynamic *sunFlare; // [esp+0h] [ebp-8h]
    unsigned int viewIndex; // [esp+4h] [ebp-4h]

    for ( viewIndex = 0; viewIndex < 4; ++viewIndex )
    {
        sunFlare = &sunFlareArray[viewIndex];
        sunFlare->currentBlind = 0.0f;
        sunFlare->currentGlare = 0.0f;
        sunFlare->flareIntensity = 0.0f;
        sunFlare->hitNum = 0;
        sunFlare->lastDot = 0.0f;
        sunFlare->lastTime = 0;
        sunFlare->lastVisibility = 0.0f;
    }
}

