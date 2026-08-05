#include "cg_visionsets.h"
#include <universal/com_memory.h>
#include <universal/dvar.h>
#include <universal/q_shared.h>
#include <xanim/xmodel.h>
#include <devgui/devgui.h>
#include <cgame_mp/cg_local_mp.h>
#include <cgame_mp/cg_main_mp.h>

#include <cmath>
#include <universal/com_loadutils.h>
#include <qcommon/common.h>
#include <universal/q_parse.h>
#include <client_mp/cl_cgame_mp.h>
#include <cgame_mp/cg_consolecmds_mp.h>
#include <cgame_mp/cg_view_mp.h>
#include "cg_camera.h"
#include <bgame/bg_weapons.h>
#include <bgame/bg_weapons_def.h>
#include <gfx_d3d/r_font.h>
#include <gfx_d3d/r_rendercmds.h>
#include <gfx_d3d/r_exposure.h>
#include <client/client.h>

const dvar_s *nightVisionFadeInOutTime;
const dvar_s *nightVisionPowerOnTime;
const dvar_s *nightVisionDisableEffects;
const dvar_s *flareDisableEffects;
const dvar_s *showVisionSetDebugInfo;

const char *MYDEFAULTVISIONNAME = "default";

visField_t visionDefFields[68] =
{
  { "r_filmEnable", 0, 0 },
  { "r_filmContrast", 92, 2 },
  { "r_filmColorTemp", 44, 2 },
  { "r_filmHue", 20, 2 },
  { "r_filmSaturation", 32, 2 },
  { "r_filmDarkTint", 80, 2 },
  { "r_filmMidTint", 68, 2 },
  { "r_filmLightTint", 56, 2 },
  { "r_filmMidStart", 4, 1 },
  { "r_filmMidEnd", 8, 1 },
  { "r_filmDarkFeather", 12, 1 },
  { "r_filmLightFeather", 16, 1 },
  { "r_filmBleach", 104, 2 },
  { "r_primaryLightUseTweaks", 176, 0 },
  { "r_primaryLightTweakDiffuseStrength", 180, 1 },
  { "r_primaryLightTweakSpecularStrength", 184, 1 },
  { "r_filmLut", 116, 4 },
  { "r_reviveFX_enable", 120, 0 },
  { "r_reviveFX_edgeColorTemp", 124, 1 },
  { "r_reviveFX_edgeSaturation", 128, 1 },
  { "r_reviveFX_edgeScale", 132, 2 },
  { "r_reviveFX_edgeContrast", 144, 2 },
  { "r_reviveFX_edgeOffset", 156, 2 },
  { "r_reviveFX_edgeMaskAdjust", 168, 1 },
  { "r_reviveFX_edgeAmount", 172, 1 },
  { "snd_masterRingmod", 188, 1 },
  { "snd_reverbRingmod", 192, 1 },
  { "snd_hiFilter", 196, 1 },
  { "snd_lowFilter", 200, 1 },
  { "r_sCurveEnable", 204, 5 },
  { "r_sCurveShoulderStrength", 208, 1 },
  { "r_sCurveLinearStrength", 212, 1 },
  { "r_sCurveLinearAngle", 216, 1 },
  { "r_sCurveToeStrength", 220, 1 },
  { "r_sCurveToeNumerator", 224, 1 },
  { "r_sCurveToeDenominator", 228, 1 },
  { "r_postEmissiveBrightening", 232, 1 },
  { "r_bloomBlurRadius", 236, 1 },
  { "r_bloomTintWeights", 240, 3 },
  { "r_bloomColorScale", 256, 3 },
  { "r_bloomTintScale", 272, 3 },
  { "r_bloomCurveBreakpoint", 288, 3 },
  { "r_bloomCurveLoBlack", 304, 3 },
  { "r_bloomCurveLoGamma", 320, 3 },
  { "r_bloomCurveLoWhite", 336, 3 },
  { "r_bloomCurveLoRemapBlack", 352, 3 },
  { "r_bloomCurveLoRemapWhite", 368, 3 },
  { "r_bloomCurveHiBlack", 384, 3 },
  { "r_bloomCurveHiGamma", 400, 3 },
  { "r_bloomCurveHiWhite", 416, 3 },
  { "r_bloomCurveHiRemapBlack", 432, 3 },
  { "r_bloomCurveHiRemapWhite", 448, 3 },
  { "r_bloomExpansionControl", 464, 3 },
  { "r_bloomExpansionWeights", 480, 3 },
  { "r_bloomExpansionSource", 496, 5 },
  { "r_bloomPersistence", 500, 1 },
  { "r_bloomStreakXLevels0", 504, 3 },
  { "r_bloomStreakXLevels1", 520, 3 },
  { "r_bloomStreakXInnerTint", 536, 2 },
  { "r_bloomStreakXOuterTint", 548, 2 },
  { "r_bloomStreakXTintControl", 560, 3 },
  { "r_bloomStreakXTint", 576, 2 },
  { "r_bloomStreakYLevels0", 588, 3 },
  { "r_bloomStreakYLevels1", 604, 3 },
  { "r_bloomStreakYInnerTint", 620, 2 },
  { "r_bloomStreakYOuterTint", 632, 2 },
  { "r_bloomStreakYTintControl", 644, 3 },
  { "r_bloomStreakYTint", 660, 2 }
};



void __cdecl CG_RegisterVisionSetsDvars()
{
    nightVisionFadeInOutTime = _Dvar_RegisterFloat(
                                                             "nightVisionFadeInOutTime",
                                                             0.1,
                                                             0.0,
                                                             10000.0,
                                                             0x1000u,
                                                             "How long the fade to/from black lasts when putting on or removing night vision goggles.");
    nightVisionPowerOnTime = _Dvar_RegisterFloat(
                                                         "nightVisionPowerOnTime",
                                                         0.30000001,
                                                         0.0,
                                                         10000.0,
                                                         0x1000u,
                                                         "How long the black-to-nightvision fade lasts when turning on the goggles.");
    nightVisionDisableEffects = _Dvar_RegisterBool("nightVisionDisableEffects", 0, 0x1000u, "");
    flareDisableEffects = _Dvar_RegisterBool("flareDisableEffects", 0, 0x80u, "");
    showVisionSetDebugInfo = _Dvar_RegisterInt("showVisionSetDebugInfo", -1, -1, 3, 0x80u, "Enables visionset debug info");
}

void __cdecl CG_InitVisionSetsMenu()
{
    DB_EnumXAssets(ASSET_TYPE_RAWFILE, (void (__cdecl *)(XAssetHeader, void *))CG_AddVisionSetMenuItem, 0, 0);
}

void __cdecl CG_AddVisionSetMenuItem(XAssetHeader header)
{
    char devguiPath[260]; // [esp+0h] [ebp-310h] BYREF
    const char *visionSetNameEnd; // [esp+104h] [ebp-20Ch]
    char visionSetName[256]; // [esp+108h] [ebp-208h] BYREF
    char command[256]; // [esp+208h] [ebp-108h] BYREF
    const char *visionSetNameBegin; // [esp+30Ch] [ebp-4h]

    if ( !header.xmodelPieces
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp", 51, 0, "%s", "header.rawfile") )
    {
        __debugbreak();
    }
    if ( !header.xmodelPieces->name
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp",
                    52,
                    0,
                    "%s",
                    "header.rawfile->name") )
    {
        __debugbreak();
    }
    visionSetNameEnd = I_stristr(header.xmodelPieces->name, ".vision");
    if ( visionSetNameEnd )
    {
        visionSetNameBegin = I_stristr(header.xmodelPieces->name, "/");
        if ( !visionSetNameBegin
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp",
                        60,
                        1,
                        "%s",
                        "visionSetNameBegin") )
        {
            __debugbreak();
        }
        ++visionSetNameBegin;
        if ( visionSetNameEnd - visionSetNameBegin >= 256
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp",
                        64,
                        1,
                        "%s",
                        "visionSetNameEnd - visionSetNameBegin < static_cast<int>( sizeof( visionSetName ) )") )
        {
            __debugbreak();
        }
        strncpy(
            visionSetName,
            visionSetNameBegin,
            visionSetNameEnd - visionSetNameBegin);
        visionSetName[visionSetNameEnd - visionSetNameBegin] = 0;
        _snprintf(devguiPath, 0x100u, "Art/Post FX/Vision Sets/%s", visionSetName);
        _snprintf(command, 0x100u, "VisionSetNaked %s", visionSetName);
        DevGui_AddCommand(devguiPath, command);
    }
}

void __cdecl CG_InitVisionSets(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    strcpy(cgameGlob->visionNameInfrared, "infrared");
    CG_VisionSetStartLerp_To(
        localClientNum,
        VISIONSETMODE_INFRARED,
        VISIONSETLERP_CUSTOM_VALUE,
        cgameGlob->visionNameInfrared,
        0);
    strcpy(cgameGlob->visionNameTvguided, "tvguided_mp");
    strcpy(cgameGlob->visionNameExtraCam, "camera_spike_mp");
    CG_VisionSetStartLerp_To(
        localClientNum,
        VISIONSETMODE_TVGUIDED,
        VISIONSETLERP_CUSTOM_VALUE,
        cgameGlob->visionNameTvguided,
        0);
    CG_VisionSetStartLerp_To(
        localClientNum,
        VISIONSETMODE_EXTRACAM,
        VISIONSETLERP_CUSTOM_VALUE,
        cgameGlob->visionNameExtraCam,
        0);
    strcpy(cgameGlob->visionNameFlare, "flare");
    CG_VisionSetStartLerp_To(
        localClientNum,
        VISIONSETMODE_FLARE,
        VISIONSETLERP_CUSTOM_VALUE,
        cgameGlob->visionNameFlare,
        0);
}

double __cdecl CalculateFlareRatio(float flareRatio, float flareRatioLastFrame)
{
    float v4; // [esp+4h] [ebp-4h]

    if ( (float)(flareRatio - flareRatioLastFrame) > cg_visionSetLerpMaxIncreasePerFrame->current.value )
        flareRatio = flareRatioLastFrame + cg_visionSetLerpMaxIncreasePerFrame->current.value;
    if ( (float)(flareRatioLastFrame - flareRatio) > cg_visionSetLerpMaxDecreasePerFrame->current.value )
        flareRatio = flareRatioLastFrame - cg_visionSetLerpMaxDecreasePerFrame->current.value;
    if ( (float)(flareRatio - 1.0) < 0.0 )
        v4 = flareRatio;
    else
        v4 = 1.0f;
    if ( (float)(0.0 - flareRatio) < 0.0 )
        return v4;
    else
        return 0.0f;
}

void __cdecl CG_VisionSetsUpdate(int localClientNum)
{
    float flareRatio; // [esp+28h] [ebp-Ch]
    float flareRatioa; // [esp+28h] [ebp-Ch]
    cg_s *cgameGlob; // [esp+2Ch] [ebp-8h]
    int idx; // [esp+30h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->visionNameNaked[0] )
        SetDefaultVision(localClientNum);
    for ( idx = 0; idx < 6; ++idx )
    {
        if ( idx == 2 )
        {
            flareRatio = 0.0f;
            if ( (cgameGlob->snap->ps.perks[1] & 0x400) == 0 )
                flareRatio = cgameGlob->snap->ps.visionSetLerpRatio;
            CG_VisionSetStartLerp_To(
                localClientNum,
                VISIONSETMODE_FLARE,
                VISIONSETLERP_CUSTOM_VALUE,
                cgameGlob->visionNameFlare,
                2000);
            flareRatioa = CalculateFlareRatio(flareRatio, cgameGlob->flareRatioLastFrame);
            UpdateVarsLerpCustom(
                flareRatioa,
                cgameGlob->visionSetCurrent,
                &cgameGlob->visionSetTo[2],
                &cgameGlob->visionSetLerpData[2],
                &cgameGlob->visionSetCurrent[2]);
            cgameGlob->flareRatioLastFrame = flareRatioa;
        }
        else
        {
            UpdateVarsLerp(
                cgameGlob->time,
                &cgameGlob->visionSetFrom[idx],
                &cgameGlob->visionSetTo[idx],
                &cgameGlob->visionSetLerpData[idx],
                &cgameGlob->visionSetCurrent[idx]);
        }
    }
}

void __cdecl UpdateVarsLerpCustom(
                float flareRatio,
                const visionSetVars_t *from,
                const visionSetVars_t *to,
                visionSetLerpData_t *lerpData,
                visionSetVars_t *result)
{
    const bool *voidFrom; // [esp+18h] [ebp-10h]
    const bool *voidTo; // [esp+1Ch] [ebp-Ch]
    float *voidResult; // [esp+20h] [ebp-8h]
    int fieldNum; // [esp+24h] [ebp-4h]

    if ( lerpData->style != VISIONSETLERP_CUSTOM_VALUE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp",
                    522,
                    0,
                    "%s",
                    "lerpData->style == VISIONSETLERP_CUSTOM_VALUE") )
    {
        __debugbreak();
    }
    if ( !from && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp", 523, 0, "%s", "from") )
        __debugbreak();
    if ( !to && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp", 524, 0, "%s", "to") )
        __debugbreak();
    if ( !lerpData
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp", 525, 0, "%s", "lerpData") )
    {
        __debugbreak();
    }
    if ( !result
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp", 526, 0, "%s", "result") )
    {
        __debugbreak();
    }
    for ( fieldNum = 0; fieldNum < 68; ++fieldNum )
    {
        voidFrom = &from->filmEnable + visionDefFields[fieldNum].offset;
        voidTo = &to->filmEnable + visionDefFields[fieldNum].offset;
        voidResult = (float *)(&result->filmEnable + visionDefFields[fieldNum].offset);
        switch ( visionDefFields[fieldNum].fieldType )
        {
            case 0:
                *(_BYTE *)voidResult = LerpBool(*voidFrom, *voidTo, flareRatio, lerpData->style);
                break;
            case 1:
                *voidResult = LerpFloat(*(float *)voidFrom, *(float *)voidTo, flareRatio, lerpData->style);
                break;
            case 3:
                LerpVec4((float *)voidFrom, (float *)voidTo, flareRatio, lerpData->style, voidResult);
                break;
            case 4:
                *voidResult = LerpStepFloat(*(float *)voidFrom, *(float *)voidTo, flareRatio, lerpData->style);
                break;
            case 5:
                *(unsigned int *)voidResult = LerpInteger(*(unsigned int *)voidFrom, *(unsigned int *)voidTo, flareRatio, lerpData->style);
                break;
            case 6:
                *voidResult = 0.0;
                break;
            default:
                if ( visionDefFields[fieldNum].fieldType != 2
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp",
                                562,
                                0,
                                "%s",
                                "fieldDef->fieldType == DTYPE_VEC3") )
                {
                    __debugbreak();
                }
                LerpVec3((float *)voidFrom, (float *)voidTo, flareRatio, lerpData->style, voidResult);
                break;
        }
    }
}

bool __cdecl LerpBool(bool from, bool to, float fraction, visionSetLerpStyle_t style)
{
    if ( style == VISIONSETLERP_NONE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp",
                    420,
                    0,
                    "%s",
                    "style != VISIONSETLERP_NONE") )
    {
        __debugbreak();
    }
    if ( style < VISIONSETLERP_BACKFORTH_LINEAR || style > VISIONSETLERP_BACKFORTH_SMOOTH || fraction < 0.5 )
        return to;
    else
        return from;
}

int __cdecl LerpInteger(int from, int to, float fraction, visionSetLerpStyle_t style)
{
    if ( style == VISIONSETLERP_NONE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp",
                    438,
                    0,
                    "%s",
                    "style != VISIONSETLERP_NONE") )
    {
        __debugbreak();
    }
    return to;
}

double __cdecl LerpStepFloat(float from, float to, float fraction, visionSetLerpStyle_t style)
{
    if ( style == VISIONSETLERP_NONE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp",
                    454,
                    0,
                    "%s",
                    "style != VISIONSETLERP_NONE") )
    {
        __debugbreak();
    }
    if ( style < VISIONSETLERP_BACKFORTH_LINEAR || style > VISIONSETLERP_BACKFORTH_SMOOTH || fraction < 0.5 )
        return to;
    else
        return from;
}

double __cdecl LerpFloat(float from, float to, float fraction, visionSetLerpStyle_t style)
{
    double v5; // st7
    long double var4; // [esp+0h] [ebp-4h]

    iassert(style != VISIONSETLERP_NONE);

    //LODWORD(var4) = style - 2;
    switch (style)
    {
    case VISIONSETLERP_TO_LINEAR:
        return (to - from) * fraction + from;
    case VISIONSETLERP_TO_SMOOTH:
        fraction = sin(fraction * 3.141592741012573 * 0.5);
        return (to - from) * fraction + from;
    case VISIONSETLERP_BACKFORTH_SMOOTH:
        fraction = sin(fraction * 3.141592741012573 * 0.5);
        break;
    case VISIONSETLERP_CUSTOM_VALUE:
        return (to - from) * fraction + from;
    }

    iassert((style == VISIONSETLERP_BACKFORTH_SMOOTH) || (style == VISIONSETLERP_BACKFORTH_LINEAR));

    if ( fraction >= 0.5 )
        return (from - to) * (fraction - 0.5) + (from - to) * (fraction - 0.5) + to;

    v5 = (to - from) * fraction;
    return v5 + v5 + from;
}

void __cdecl LerpVec3(float *from, float *to, float fraction, visionSetLerpStyle_t style, float *result)
{
    *result = LerpFloat(*from, *to, fraction, style);
    result[1] = LerpFloat(from[1], to[1], fraction, style);
    result[2] = LerpFloat(from[2], to[2], fraction, style);
}

void __cdecl LerpVec4(float *from, float *to, float fraction, visionSetLerpStyle_t style, float *result)
{
    *result = LerpFloat(*from, *to, fraction, style);
    result[1] = LerpFloat(from[1], to[1], fraction, style);
    result[2] = LerpFloat(from[2], to[2], fraction, style);
    result[3] = LerpFloat(from[3], to[3], fraction, style);
}

void __cdecl UpdateVarsLerp(
                int time,
                const visionSetVars_t *from,
                const visionSetVars_t *to,
                visionSetLerpData_t *lerpData,
                visionSetVars_t *result)
{
    float v5; // [esp+1Ch] [ebp-24h]
    visionSetLerpStyle_t style; // [esp+20h] [ebp-20h]
    float v7; // [esp+24h] [ebp-1Ch]
    const bool *voidFrom; // [esp+2Ch] [ebp-14h]
    const bool *voidTo; // [esp+30h] [ebp-10h]
    float *voidResult; // [esp+34h] [ebp-Ch]
    float fraction; // [esp+38h] [ebp-8h]
    int fieldNum; // [esp+3Ch] [ebp-4h]

    if ( !from && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp", 575, 0, "%s", "from") )
        __debugbreak();
    if ( !to && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp", 576, 0, "%s", "to") )
        __debugbreak();
    if ( !lerpData
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp", 577, 0, "%s", "lerpData") )
    {
        __debugbreak();
    }
    if ( !result
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp", 578, 0, "%s", "result") )
    {
        __debugbreak();
    }
    if ( lerpData->style > (unsigned int)VISIONSETLERP_NONE )
    {
        if ( lerpData->timeDuration + lerpData->timeStart >= time )
        {
            fraction = (float)(time - lerpData->timeStart) / (float)lerpData->timeDuration;
            if ( (float)(fraction - 1.0) < 0.0 )
                v7 = (float)(time - lerpData->timeStart) / (float)lerpData->timeDuration;
            else
                v7 = 1.0f;
            if ( (float)(0.0 - fraction) < 0.0 )
                v5 = v7;
            else
                v5 = 0.0f;
            for ( fieldNum = 0; fieldNum < 68; ++fieldNum )
            {
                voidFrom = &from->filmEnable + visionDefFields[fieldNum].offset;
                voidTo = &to->filmEnable + visionDefFields[fieldNum].offset;
                voidResult = (float *)(&result->filmEnable + visionDefFields[fieldNum].offset);
                switch ( visionDefFields[fieldNum].fieldType )
                {
                    case 0:
                        *(_BYTE *)voidResult = LerpBool(*voidFrom, *voidTo, v5, lerpData->style);
                        break;
                    case 1:
                        *voidResult = LerpFloat(*(float *)voidFrom, *(float *)voidTo, v5, lerpData->style);
                        break;
                    case 3:
                        LerpVec4((float *)voidFrom, (float *)voidTo, v5, lerpData->style, voidResult);
                        break;
                    case 4:
                        *voidResult = LerpStepFloat(*(float *)voidFrom, *(float *)voidTo, v5, lerpData->style);
                        break;
                    case 5:
                        *(unsigned int *)voidResult = LerpInteger(*(unsigned int *)voidFrom, *(unsigned int *)voidTo, v5, lerpData->style);
                        break;
                    case 6:
                        *voidResult = 0.0;
                        break;
                    default:
                        if ( visionDefFields[fieldNum].fieldType != 2
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp",
                                        638,
                                        0,
                                        "%s",
                                        "fieldDef->fieldType == DTYPE_VEC3") )
                        {
                            __debugbreak();
                        }
                        LerpVec3((float *)voidFrom, (float *)voidTo, v5, lerpData->style, voidResult);
                        break;
                }
            }
        }
        else
        {
            style = lerpData->style;
            if ( style < VISIONSETLERP_TO_LINEAR || style > VISIONSETLERP_TO_SMOOTH )
            {
                if ( lerpData->style != VISIONSETLERP_BACKFORTH_LINEAR
                    && lerpData->style != VISIONSETLERP_BACKFORTH_SMOOTH
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp",
                                593,
                                0,
                                "%s",
                                "(lerpData->style == VISIONSETLERP_BACKFORTH_LINEAR) || (lerpData->style == VISIONSETLERP_BACKFORTH_SMOOTH)") )
                {
                    __debugbreak();
                }
                memcpy(result, from, sizeof(visionSetVars_t));
            }
            else
            {
                memcpy(result, to, sizeof(visionSetVars_t));
            }
            lerpData->style = VISIONSETLERP_NONE;
        }
    }
}

char __cdecl VisionSetCurrent(int localClientNum, visionSetMode_t mode, const char *name)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !GetVisionSet(localClientNum, name, &cgameGlob->visionSetCurrent[mode]) )
        return 0;
    cgameGlob->visionSetLerpData[mode].style = VISIONSETLERP_NONE;
    return 1;
}

char __cdecl GetVisionSet(int localClientNum, const char *name, visionSetVars_t *resultSettings)
{
    cg_s *cgameGlob; // [esp+8h] [ebp-Ch]
    int idx; // [esp+10h] [ebp-4h]
    int idxa; // [esp+10h] [ebp-4h]

    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp", 379, 0, "%s", "name") )
        __debugbreak();
    if ( !resultSettings
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp", 380, 0, "%s", "resultSettings") )
    {
        __debugbreak();
    }
    if ( !*name )
        return 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    for ( idx = 0; idx < 8 && I_stricmp(name, cgameGlob->visionSetPreLoadedName[idx]); ++idx )
        ;
    if ( idx == 8 )
    {
        for ( idxa = 0; idxa < 8 && cgameGlob->visionSetPreLoadedName[idxa][0]; ++idxa )
            ;
        if ( idxa == 8 )
            idxa = 0;
        if ( LoadVisionFile(name, &cgameGlob->visionSetPreLoaded[idxa]) )
        {
            memcpy(resultSettings, &cgameGlob->visionSetPreLoaded[idxa], sizeof(visionSetVars_t));
            I_strncpyz(cgameGlob->visionSetPreLoadedName[idxa], name, 64);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        memcpy(resultSettings, &cgameGlob->visionSetPreLoaded[idx], sizeof(visionSetVars_t));
        return 1;
    }
}

char __cdecl LoadVisionFile(const char *name, visionSetVars_t *resultSettings)
{
    char *fileBuf; // [esp+0h] [ebp-50h]
    char success; // [esp+7h] [ebp-49h]
    char fullPath[68]; // [esp+8h] [ebp-48h] BYREF

    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp", 352, 0, "%s", "name") )
        __debugbreak();
    if ( !resultSettings
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp", 353, 0, "%s", "resultSettings") )
    {
        __debugbreak();
    }
    fileBuf = RawBufferOpen(name, "vision/%s.vision");
    if ( !fileBuf )
        return 0;
    Com_sprintf(fullPath, 0x40u, "vision/%s.vision", name);
    success = LoadVisionSettingsFromBuffer(fileBuf, fullPath, resultSettings);
    resultSettings->name[0] = 0;
    if ( success )
        I_strncpyz(resultSettings->name, name, 64);
    Com_UnloadRawTextFile(fileBuf);
    return success;
}

char *__cdecl RawBufferOpen(const char *name, const char *formatFullPath)
{
    char *filebuf; // [esp+0h] [ebp-4Ch]
    char fullpath[68]; // [esp+4h] [ebp-48h] BYREF

    Com_sprintf(fullpath, 0x40u, formatFullPath, name);
    filebuf = Com_LoadRawTextFile(fullpath);
    if ( filebuf )
        return filebuf;
    Com_PrintError(17, "couldn't open '%s'.\n", fullpath);
    Com_sprintf(fullpath, 0x40u, formatFullPath, "default");
    filebuf = Com_LoadRawTextFile(fullpath);
    if ( filebuf )
        return filebuf;
    Com_PrintError(17, "couldn't open '%s'. This is a default file that you should have.\n", fullpath);
    return 0;
}

char __cdecl LoadVisionSettingsFromBuffer(const char *buffer, const char *filename, visionSetVars_t *settings)
{
    signed int fieldNum; // [esp+0h] [ebp-5Ch]
    unsigned __int8 wasRead[76]; // [esp+4h] [ebp-58h] BYREF
    const char *token; // [esp+58h] [ebp-4h]

    if ( !settings
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp", 307, 0, "%s", "settings") )
    {
        __debugbreak();
    }
    memset(wasRead, 0, 0x44u);
    Com_BeginParseSession(filename);
    while ( 1 )
    {
        token = (const char *)Com_Parse(&buffer);
        if ( !*token )
            break;
        for ( fieldNum = 0; ; ++fieldNum )
        {
            if ( fieldNum >= 68 )
            {
                Com_PrintWarning(16, "WARNING: unknown dvar '%s' in file '%s'\n", token, filename);
                goto next_var;
            }
            if ( !wasRead[fieldNum] && !I_stricmp(token, visionDefFields[fieldNum].name) )
                break;
        }
        token = (const char *)Com_ParseOnLine(&buffer);
        if ( ApplyTokenToField(fieldNum, (char *)token, settings) )
            wasRead[fieldNum] = 1;
        else
            Com_PrintWarning(16, "WARNING: malformed dvar '%s' in file '%s'\n", token, filename);
next_var:
        Com_SkipRestOfLine(&buffer);
    }
    Com_EndParseSession();
    return 1;
}

bool __cdecl ApplyTokenToField(unsigned int fieldNum, char *token, visionSetVars_t *settings)
{
    bool result; // al
    float *vec4Field; // [esp+Ch] [ebp-40h]
    int tempInt; // [esp+10h] [ebp-3Ch] BYREF
    float tempVec4[4]; // [esp+14h] [ebp-38h] BYREF
    float *vec3Field; // [esp+24h] [ebp-28h]
    void *voidField; // [esp+28h] [ebp-24h]
    Material **matField; // [esp+2Ch] [ebp-20h]
    int scanResult; // [esp+30h] [ebp-1Ch]
    int *intField; // [esp+34h] [ebp-18h]
    float tempFloat; // [esp+38h] [ebp-14h] BYREF
    float tempVec3[3]; // [esp+3Ch] [ebp-10h] BYREF
    bool *boolField; // [esp+48h] [ebp-4h]

    if ( fieldNum > 0x44
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp",
                    237,
                    0,
                    "fieldNum not in [0, numVisionDefFields]\n\t%i not in [%i, %i]",
                    fieldNum,
                    0,
                    68) )
    {
        __debugbreak();
    }
    if ( !token && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp", 238, 0, "%s", "token") )
        __debugbreak();
    if ( !settings
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp", 239, 0, "%s", "settings") )
    {
        __debugbreak();
    }
    voidField = &settings->filmEnable + visionDefFields[fieldNum].offset;
    switch ( visionDefFields[fieldNum].fieldType )
    {
        case 0:
            boolField = (bool *)voidField;
            scanResult = sscanf(token, "%i", &tempInt);
            if ( scanResult == 1 )
            {
                *boolField = tempInt != 0;
                goto LABEL_32;
            }
            result = 0;
            break;
        case 1:
        case 4:
            if ( sscanf(token, "%f", &tempFloat) == 1 )
            {
                *(float *)voidField = tempFloat;
                goto LABEL_32;
            }
            result = 0;
            break;
        case 3:
            vec4Field = (float *)voidField;
            scanResult = sscanf(token, "%f %f %f %f", tempVec4, &tempVec4[1], &tempVec4[2], &tempVec4[3]);
            if ( scanResult == 4 )
            {
                *vec4Field = tempVec4[0];
                vec4Field[1] = tempVec4[1];
                vec4Field[2] = tempVec4[2];
                vec4Field[3] = tempVec4[3];
                goto LABEL_32;
            }
            result = 0;
            break;
        case 5:
            intField = (int *)voidField;
            scanResult = sscanf(token, "%i", &tempInt);
            if ( scanResult == 1 )
            {
                *intField = tempInt;
                goto LABEL_32;
            }
            result = 0;
            break;
        case 6:
            matField = (Material **)voidField;
            *(unsigned int *)voidField = (unsigned int)Material_Register(token, 6);
            if ( *matField )
                goto LABEL_32;
            result = 0;
            break;
        default:
            if ( visionDefFields[fieldNum].fieldType != 2
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp",
                            288,
                            0,
                            "%s",
                            "fieldDef->fieldType == DTYPE_VEC3") )
            {
                __debugbreak();
            }
            vec3Field = (float *)voidField;
            if ( sscanf(token, "%f %f %f", tempVec3, &tempVec3[1], &tempVec3[2]) == 3 )
            {
                *vec3Field = tempVec3[0];
                vec3Field[1] = tempVec3[1];
                vec3Field[2] = tempVec3[2];
LABEL_32:
                result = 1;
            }
            else
            {
                result = 0;
            }
            break;
    }
    return result;
}

bool __cdecl CG_VisionSetStartLerp_To(
                int localClientNum,
                visionSetMode_t mode,
                visionSetLerpStyle_t style,
                const char *nameTo,
                int duration)
{
    cg_s *cgameGlob; // [esp+8h] [ebp-8h]

    if ( (unsigned int)mode > VISIONSETMODECOUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_visionsets.cpp",
                    750,
                    0,
                    "mode not in [0, VISIONSETMODECOUNT]\n\t%i not in [%i, %i]",
                    mode,
                    0,
                    6) )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( duration <= 0 || cgameGlob->visionSetLerpData[mode].style == VISIONSETLERP_UNDEFINED )
        return VisionSetCurrent(localClientNum, mode, nameTo);
    if ( !GetVisionSet(localClientNum, nameTo, &cgameGlob->visionSetTo[mode]) )
        return 0;
    memcpy(&cgameGlob->visionSetFrom[mode], &cgameGlob->visionSetCurrent[mode], sizeof(cgameGlob->visionSetFrom[mode]));
    cgameGlob->visionSetLerpData[mode].style = style;
    cgameGlob->visionSetLerpData[mode].timeDuration = duration;
    cgameGlob->visionSetLerpData[mode].timeStart = cgameGlob->time;
    return 1;
}

void __cdecl SetDefaultVision(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    I_strncpyz(cgameGlob->visionNameNaked, MYDEFAULTVISIONNAME, 64);
    CG_VisionSetStartLerp_To(localClientNum, VISIONSETMODE_NAKED, VISIONSETLERP_TO_SMOOTH, cgameGlob->visionNameNaked, 0);
}

void __cdecl CG_VisionSetConfigString_Naked(int localClientNum)
{
    const char *v1; // eax
    int duration; // [esp+0h] [ebp-10h]
    cg_s *cgameGlob; // [esp+4h] [ebp-Ch]
    const char *configString; // [esp+8h] [ebp-8h] BYREF
    const char *token; // [esp+Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    configString = CL_GetConfigString(1550);
    token = Com_Parse(&configString)->token;
    I_strncpyz(cgameGlob->visionNameNaked, token, 64);
    v1 = Com_Parse(&configString)->token;
    duration = atoi(v1);
    CG_VisionSetStartLerp_To(
        localClientNum,
        VISIONSETMODE_NAKED,
        VISIONSETLERP_TO_SMOOTH,
        cgameGlob->visionNameNaked,
        duration);
}

void __cdecl CG_VisionSetConfigString_Night(int localClientNum)
{
    const char *v1; // eax
    int duration; // [esp+0h] [ebp-10h]
    cg_s *cgameGlob; // [esp+4h] [ebp-Ch]
    const char *configString; // [esp+8h] [ebp-8h] BYREF
    const char *token; // [esp+Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    configString = CL_GetConfigString(1551);
    token = Com_Parse(&configString)->token;
    I_strncpyz(cgameGlob->visionNameNight, token, 64);
    v1 = Com_Parse(&configString)->token;
    duration = atoi(v1);
    CG_VisionSetStartLerp_To(
        localClientNum,
        VISIONSETMODE_NIGHT,
        VISIONSETLERP_TO_SMOOTH,
        cgameGlob->visionNameNight,
        duration);
}

void __cdecl CG_VisionSetMyChanges()
{
    cg_s *cgameGlob; // [esp+0h] [ebp-Ch]
    unsigned int visSetIdx; // [esp+4h] [ebp-8h]
    int localClientNum; // [esp+8h] [ebp-4h]

    for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
    {
        if ( CL_LocalClient_IsCUIFlagSet(localClientNum, 32) )
        {
            cgameGlob = CG_GetLocalClientGlobals(localClientNum);
            for ( visSetIdx = 0; visSetIdx < 8; ++visSetIdx )
                cgameGlob->visionSetPreLoadedName[visSetIdx][0] = 0;
            memset(cgameGlob->visionSetPreLoaded, 0, sizeof(cgameGlob->visionSetPreLoaded));
            if ( cgameGlob->visionNameNaked[0] )
                CG_VisionSetStartLerp_To(
                    localClientNum,
                    VISIONSETMODE_NAKED,
                    VISIONSETLERP_TO_LINEAR,
                    cgameGlob->visionNameNaked,
                    0);
            if ( cgameGlob->visionNameNight[0] )
                CG_VisionSetStartLerp_To(
                    localClientNum,
                    VISIONSETMODE_NIGHT,
                    VISIONSETLERP_TO_LINEAR,
                    cgameGlob->visionNameNight,
                    0);
            if ( cgameGlob->visionNameInfrared[0] )
                CG_VisionSetStartLerp_To(
                    localClientNum,
                    VISIONSETMODE_INFRARED,
                    VISIONSETLERP_CUSTOM_VALUE,
                    cgameGlob->visionNameInfrared,
                    0);
            if ( cgameGlob->visionNameTvguided[0] )
                CG_VisionSetStartLerp_To(
                    localClientNum,
                    VISIONSETMODE_TVGUIDED,
                    VISIONSETLERP_CUSTOM_VALUE,
                    cgameGlob->visionNameTvguided,
                    0);
            if ( cgameGlob->visionNameExtraCam[0] )
                CG_VisionSetStartLerp_To(
                    localClientNum,
                    VISIONSETMODE_EXTRACAM,
                    VISIONSETLERP_CUSTOM_VALUE,
                    cgameGlob->visionNameExtraCam,
                    0);
        }
    }
}

void __cdecl CG_VisionSetUpdateTweaksFromFile_Film()
{
    visionSetVars_t setVars; // [esp+10h] [ebp-2E8h] BYREF

    if ( LoadVisionFileForTweaks(&setVars) )
    {
        Dvar_SetFloatByName("r_filmLut", setVars.filmLut);
        Dvar_SetBoolByName("r_filmTweakEnable", setVars.filmEnable);
        Dvar_SetFloatByName("r_filmTweakMidStart", setVars.filmMidStart);
        Dvar_SetFloatByName("r_filmTweakMidEnd", setVars.filmMidEnd);
        Dvar_SetFloatByName("r_filmTweakDarkFeather", setVars.filmDarkFeather);
        Dvar_SetFloatByName("r_filmTweakLightFeather", setVars.filmLightFeather);
        Dvar_SetVec3ByName(
            "r_filmTweakBleach",
            LODWORD(setVars.filmBleach[0]),
            LODWORD(setVars.filmBleach[1]),
            setVars.filmBleach[2]);
        Dvar_SetVec3ByName("r_filmTweakHue", LODWORD(setVars.filmHue[0]), LODWORD(setVars.filmHue[1]), setVars.filmHue[2]);
        Dvar_SetVec3ByName(
            "r_filmTweakSaturation",
            LODWORD(setVars.filmSaturation[0]),
            LODWORD(setVars.filmSaturation[1]),
            setVars.filmSaturation[2]);
        Dvar_SetVec3ByName(
            "r_filmTweakColorTemp",
            LODWORD(setVars.filmColorTemp[0]),
            LODWORD(setVars.filmColorTemp[1]),
            setVars.filmColorTemp[2]);
        Dvar_SetVec3ByName(
            "r_filmTweakDarkTint",
            LODWORD(setVars.filmDarkTint[0]),
            LODWORD(setVars.filmDarkTint[1]),
            setVars.filmDarkTint[2]);
        Dvar_SetVec3ByName(
            "r_filmTweakMidTint",
            LODWORD(setVars.filmMidTint[0]),
            LODWORD(setVars.filmMidTint[1]),
            setVars.filmMidTint[2]);
        Dvar_SetVec3ByName(
            "r_filmTweakLightTint",
            LODWORD(setVars.filmLightTint[0]),
            LODWORD(setVars.filmLightTint[1]),
            setVars.filmLightTint[2]);
        Dvar_SetVec3ByName(
            "r_filmTweakContrast",
            LODWORD(setVars.filmContrast[0]),
            LODWORD(setVars.filmContrast[1]),
            setVars.filmContrast[2]);
        Dvar_SetIntByName("r_sCurveEnable", setVars.sCurveEnable);
        Dvar_SetFloatByName("r_sCurveShoulderStrength", setVars.sCurveShoulderStrength);
        Dvar_SetFloatByName("r_sCurveLinearStrength", setVars.sCurveLinearStrength);
        Dvar_SetFloatByName("r_sCurveLinearAngle", setVars.sCurveLinearAngle);
        Dvar_SetFloatByName("r_sCurveToeStrength", setVars.sCurveToeStrength);
        Dvar_SetFloatByName("r_sCurveToeNumerator", setVars.sCurveToeNumerator);
        Dvar_SetFloatByName("r_sCurveToeDenominator", setVars.sCurveToeDenominator);
    }
    else
    {
        Com_PrintWarning(16, "WARNING: Couldn't update film tweak vars from file.    Vision file is likely not in use.\n");
    }
}

bool __cdecl LoadVisionFileForTweaks(visionSetVars_t *setVars)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-10h]
    const char *setName; // [esp+Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(0);
    if ( CG_InFlareVision(0) )
    {
        setName = cgameGlob->visionNameFlare;
    }
    else if ( CG_IsInfrared(0) )
    {
        setName = cgameGlob->visionNameInfrared;
    }
    else if ( CG_IsTvguided(0, 0) )
    {
        setName = cgameGlob->visionNameTvguided;
    }
    else if ( CG_LookingThroughNightVision(0) )
    {
        setName = cgameGlob->visionNameNight;
    }
    else
    {
        setName = cgameGlob->visionNameNaked;
    }
    return *setName && GetVisionSet(0, setName, setVars) != 0;
}

void __cdecl CG_VisionSetUpdateTweaksFromFile_CharPrimary()
{
    visionSetVars_t setVars; // [esp+8h] [ebp-2E8h] BYREF

    if ( LoadVisionFileForTweaks(&setVars) )
    {
        Dvar_SetBoolByName("r_primaryLightUseTweaks", setVars.charPrimaryUseTweaks);
        Dvar_SetFloatByName("r_primaryLightTweakDiffuseStrength", setVars.charPrimaryDiffuseScale);
        Dvar_SetFloatByName("r_primaryLightTweakSpecularStrength", setVars.charPrimarySpecularScale);
    }
    else
    {
        Com_PrintWarning(
            16,
            "WARNING: Couldn't update character primary light tweak vars from file.    Vision file is likely not in use.\n");
    }
}

void __cdecl CG_VisionSetUpdateTweaksFromFile_Bloom()
{
    visionSetVars_t setVars; // [esp+14h] [ebp-2E8h] BYREF

    if ( LoadVisionFileForTweaks(&setVars) )
    {
        Dvar_SetFloatByName("r_bloomBlurRadius", setVars.bloomBlurRadius);
        Dvar_SetVec4ByName(
            "r_bloomTintWeights",
            LODWORD(setVars.bloomTintWeights[0]),
            LODWORD(setVars.bloomTintWeights[1]),
            LODWORD(setVars.bloomTintWeights[2]),
            LODWORD(setVars.bloomTintWeights[3]));
        Dvar_SetVec4ByName(
            "r_bloomColorScale",
            LODWORD(setVars.bloomColorScale[0]),
            LODWORD(setVars.bloomColorScale[1]),
            LODWORD(setVars.bloomColorScale[2]),
            LODWORD(setVars.bloomColorScale[3]));
        Dvar_SetVec4ByName(
            "r_bloomTintScale",
            LODWORD(setVars.bloomTintScale[0]),
            LODWORD(setVars.bloomTintScale[1]),
            LODWORD(setVars.bloomTintScale[2]),
            LODWORD(setVars.bloomTintScale[3]));
        Dvar_SetVec4ByName(
            "r_bloomCurveBreakpoint",
            LODWORD(setVars.bloomCurveBreakpoint[0]),
            LODWORD(setVars.bloomCurveBreakpoint[1]),
            LODWORD(setVars.bloomCurveBreakpoint[2]),
            LODWORD(setVars.bloomCurveBreakpoint[3]));
        Dvar_SetVec4ByName(
            "r_bloomCurveLoBlack",
            LODWORD(setVars.bloomCurveLoBlack[0]),
            LODWORD(setVars.bloomCurveLoBlack[1]),
            LODWORD(setVars.bloomCurveLoBlack[2]),
            LODWORD(setVars.bloomCurveLoBlack[3]));
        Dvar_SetVec4ByName(
            "r_bloomCurveLoGamma",
            LODWORD(setVars.bloomCurveLoGamma[0]),
            LODWORD(setVars.bloomCurveLoGamma[1]),
            LODWORD(setVars.bloomCurveLoGamma[2]),
            LODWORD(setVars.bloomCurveLoGamma[3]));
        Dvar_SetVec4ByName(
            "r_bloomCurveLoWhite",
            LODWORD(setVars.bloomCurveLoWhite[0]),
            LODWORD(setVars.bloomCurveLoWhite[1]),
            LODWORD(setVars.bloomCurveLoWhite[2]),
            LODWORD(setVars.bloomCurveLoWhite[3]));
        Dvar_SetVec4ByName(
            "r_bloomCurveLoRemapBlack",
            LODWORD(setVars.bloomCurveLoRemapBlack[0]),
            LODWORD(setVars.bloomCurveLoRemapBlack[1]),
            LODWORD(setVars.bloomCurveLoRemapBlack[2]),
            LODWORD(setVars.bloomCurveLoRemapBlack[3]));
        Dvar_SetVec4ByName(
            "r_bloomCurveLoRemapWhite",
            LODWORD(setVars.bloomCurveLoRemapWhite[0]),
            LODWORD(setVars.bloomCurveLoRemapWhite[1]),
            LODWORD(setVars.bloomCurveLoRemapWhite[2]),
            LODWORD(setVars.bloomCurveLoRemapWhite[3]));
        Dvar_SetVec4ByName(
            "r_bloomCurveHiBlack",
            LODWORD(setVars.bloomCurveHiBlack[0]),
            LODWORD(setVars.bloomCurveHiBlack[1]),
            LODWORD(setVars.bloomCurveHiBlack[2]),
            LODWORD(setVars.bloomCurveHiBlack[3]));
        Dvar_SetVec4ByName(
            "r_bloomCurveHiGamma",
            LODWORD(setVars.bloomCurveHiGamma[0]),
            LODWORD(setVars.bloomCurveHiGamma[1]),
            LODWORD(setVars.bloomCurveHiGamma[2]),
            LODWORD(setVars.bloomCurveHiGamma[3]));
        Dvar_SetVec4ByName(
            "r_bloomCurveHiWhite",
            LODWORD(setVars.bloomCurveHiWhite[0]),
            LODWORD(setVars.bloomCurveHiWhite[1]),
            LODWORD(setVars.bloomCurveHiWhite[2]),
            LODWORD(setVars.bloomCurveHiWhite[3]));
        Dvar_SetVec4ByName(
            "r_bloomCurveHiRemapBlack",
            LODWORD(setVars.bloomCurveHiRemapBlack[0]),
            LODWORD(setVars.bloomCurveHiRemapBlack[1]),
            LODWORD(setVars.bloomCurveHiRemapBlack[2]),
            LODWORD(setVars.bloomCurveHiRemapBlack[3]));
        Dvar_SetVec4ByName(
            "r_bloomCurveHiRemapWhite",
            LODWORD(setVars.bloomCurveHiRemapWhite[0]),
            LODWORD(setVars.bloomCurveHiRemapWhite[1]),
            LODWORD(setVars.bloomCurveHiRemapWhite[2]),
            LODWORD(setVars.bloomCurveHiRemapWhite[3]));
        Dvar_SetVec4ByName(
            "r_bloomExpansionControl",
            LODWORD(setVars.bloomExpansionControl[0]),
            LODWORD(setVars.bloomExpansionControl[1]),
            LODWORD(setVars.bloomExpansionControl[2]),
            LODWORD(setVars.bloomExpansionControl[3]));
        Dvar_SetVec4ByName(
            "r_bloomExpansionWeights",
            LODWORD(setVars.bloomExpansionWeights[0]),
            LODWORD(setVars.bloomExpansionWeights[1]),
            LODWORD(setVars.bloomExpansionWeights[2]),
            LODWORD(setVars.bloomExpansionWeights[3]));
        Dvar_SetIntByName("r_bloomExpansionSource", setVars.bloomExpansionSource);
        Dvar_SetFloatByName("r_bloomPersistence", setVars.bloomPersistence);
        Dvar_SetVec4ByName(
            "r_bloomStreakXLevels0",
            LODWORD(setVars.bloomStreakXLevels0[0]),
            LODWORD(setVars.bloomStreakXLevels0[1]),
            LODWORD(setVars.bloomStreakXLevels0[2]),
            LODWORD(setVars.bloomStreakXLevels0[3]));
        Dvar_SetVec4ByName(
            "r_bloomStreakXLevels1",
            LODWORD(setVars.bloomStreakXLevels1[0]),
            LODWORD(setVars.bloomStreakXLevels1[1]),
            LODWORD(setVars.bloomStreakXLevels1[2]),
            LODWORD(setVars.bloomStreakXLevels1[3]));
        Dvar_SetVec3ByName(
            "r_bloomStreakXInnerTint",
            LODWORD(setVars.bloomStreakXInnerTint[0]),
            LODWORD(setVars.bloomStreakXInnerTint[1]),
            setVars.bloomStreakXInnerTint[2]);
        Dvar_SetVec3ByName(
            "r_bloomStreakXOuterTint",
            LODWORD(setVars.bloomStreakXOuterTint[0]),
            LODWORD(setVars.bloomStreakXOuterTint[1]),
            setVars.bloomStreakXOuterTint[2]);
        Dvar_SetVec4ByName(
            "r_bloomStreakXTintControl",
            LODWORD(setVars.bloomStreakXTintControl[0]),
            LODWORD(setVars.bloomStreakXTintControl[1]),
            LODWORD(setVars.bloomStreakXTintControl[2]),
            LODWORD(setVars.bloomStreakXTintControl[3]));
        Dvar_SetVec3ByName(
            "r_bloomStreakXTint",
            LODWORD(setVars.bloomStreakXTint[0]),
            LODWORD(setVars.bloomStreakXTint[1]),
            setVars.bloomStreakXTint[2]);
        Dvar_SetVec4ByName(
            "r_bloomStreakYLevels0",
            LODWORD(setVars.bloomStreakYLevels0[0]),
            LODWORD(setVars.bloomStreakYLevels0[1]),
            LODWORD(setVars.bloomStreakYLevels0[2]),
            LODWORD(setVars.bloomStreakYLevels0[3]));
        Dvar_SetVec4ByName(
            "r_bloomStreakYLevels1",
            LODWORD(setVars.bloomStreakYLevels1[0]),
            LODWORD(setVars.bloomStreakYLevels1[1]),
            LODWORD(setVars.bloomStreakYLevels1[2]),
            LODWORD(setVars.bloomStreakYLevels1[3]));
        Dvar_SetVec3ByName(
            "r_bloomStreakYInnerTint",
            LODWORD(setVars.bloomStreakYInnerTint[0]),
            LODWORD(setVars.bloomStreakYInnerTint[1]),
            setVars.bloomStreakYInnerTint[2]);
        Dvar_SetVec3ByName(
            "r_bloomStreakYOuterTint",
            LODWORD(setVars.bloomStreakYOuterTint[0]),
            LODWORD(setVars.bloomStreakYOuterTint[1]),
            setVars.bloomStreakYOuterTint[2]);
        Dvar_SetVec4ByName(
            "r_bloomStreakYTintControl",
            LODWORD(setVars.bloomStreakYTintControl[0]),
            LODWORD(setVars.bloomStreakYTintControl[1]),
            LODWORD(setVars.bloomStreakYTintControl[2]),
            LODWORD(setVars.bloomStreakYTintControl[3]));
        Dvar_SetVec3ByName(
            "r_bloomStreakYTint",
            LODWORD(setVars.bloomStreakYTint[0]),
            LODWORD(setVars.bloomStreakYTint[1]),
            setVars.bloomStreakYTint[2]);
    }
    else
    {
        Com_PrintWarning(16, "WARNING: Couldn't update bloom tweak vars from file.    Vision file is likely not in use.\n");
    }
}

bool __cdecl CG_InFlareVision(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    if ( CL_GetLocalClientConnectionState(localClientNum) < 10 )
        return 0;
    if ( flareDisableEffects->current.enabled )
        return 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    return cgameGlob->predictedPlayerState.visionSetLerpRatio != 0.0 || cgameGlob->flareRatioLastFrame >= 0.001;
}

char __cdecl CG_LookingThroughNightVision(int localClientNum)
{
    int weapIndex; // [esp+4h] [ebp-10h]
    playerState_s *ps; // [esp+Ch] [ebp-8h]
    const WeaponDef *weapDef; // [esp+10h] [ebp-4h]

    if ( CL_GetLocalClientConnectionState(localClientNum) < 10 )
        return 0;
    if ( nightVisionDisableEffects->current.enabled )
        return 0;
    if ( zombiemode->current.enabled )
        return 0;
    ps = &CG_GetLocalClientGlobals(localClientNum)->predictedPlayerState;
    weapIndex = BG_GetViewmodelWeaponIndex(ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    if ( ps->weaponstate == 33 )
    {
        if ( weapDef->nightVisionWearTime - ps->weaponTime >= weapDef->nightVisionWearTimePowerUp )
            return 1;
    }
    else if ( ps->weaponstate == 34 )
    {
        if ( weapDef->nightVisionRemoveTime - ps->weaponTime <= weapDef->nightVisionRemoveTimePowerDown )
            return 1;
    }
    else if ( (ps->weapFlags & 0x40) != 0 )
    {
        return 1;
    }
    return 0;
}

static float textStartY = 200.0f;
static float curChannelScaleMult = 1.2f;
static float textX = 75.0f;
static float textScaleX = 1.25f;
static float textScaleY = 1.5f;
static float textVertSpacing = 20.0f;

void __cdecl DrawVisionSetDebug(int localClientNum, visionSetMode_t curChannel)
{
    float v2; // [esp+1Ch] [ebp-140h]
    const float *v3; // [esp+20h] [ebp-13Ch]
    int modeIndex; // [esp+2Ch] [ebp-130h]
    float textY; // [esp+30h] [ebp-12Ch]
    const char *visionsetModeNames[6]; // [esp+34h] [ebp-128h]
    Font_s *font; // [esp+4Ch] [ebp-110h]
    cg_s *cgameGlob; // [esp+50h] [ebp-10Ch]
    char text[260]; // [esp+54h] [ebp-108h] BYREF

    if ( showVisionSetDebugInfo && showVisionSetDebugInfo->current.integer == localClientNum )
    {
        visionsetModeNames[0] = "NAKED";
        visionsetModeNames[1] = "NIGHT";
        visionsetModeNames[2] = "FLARE";
        visionsetModeNames[3] = "INFRARED";
        visionsetModeNames[4] = "TVGUIDED";
        visionsetModeNames[5] = "EXTRACAM";
        font = R_RegisterFont("fonts/consoleFont", 1);
        textY = textStartY;
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        R_AddCmdProjectionSet2D();
        for ( modeIndex = 0; modeIndex < 6; ++modeIndex )
        {
            _snprintf(
                text,
                0x100u,
                "%s c:%s f:%s t:%s",
                visionsetModeNames[modeIndex],
                cgameGlob->visionSetCurrent[modeIndex].name,
                cgameGlob->visionSetFrom[modeIndex].name,
                cgameGlob->visionSetTo[modeIndex].name);
            if ( modeIndex == curChannel )
                v3 = colorRed;
            else
                v3 = colorWhite;
            if ( modeIndex == curChannel )
                v2 = curChannelScaleMult;
            else
                v2 = 1.0f;
            R_AddCmdDrawText(text, 256, font, textX, textY, textScaleX * v2, textScaleY * v2, 0.0, v3, 0);
            textY = textY + textVertSpacing;
        }
    }
}

void __cdecl CG_VisionSetApplyToRefdef(int localClientNum, bool isExtracam)
{
    GfxReviveFx *reviveFx; // [esp+134h] [ebp-20h]
    cg_s *cgameGlob; // [esp+138h] [ebp-1Ch]
    GfxLightScale *charLightScale; // [esp+144h] [ebp-10h]
    visionSetMode_t visionChannel; // [esp+150h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    reviveFx = &cgameGlob->refdef.visionset.reviveFx;
    charLightScale = &cgameGlob->refdef.visionset.charPrimaryLightScale;
    if ( CG_InFlareVision(localClientNum) )
    {
        visionChannel = VISIONSETMODE_FLARE;
        DrawVisionSetDebug(localClientNum, VISIONSETMODE_FLARE);
    }
    else if ( CG_IsInfrared(localClientNum) )
    {
        visionChannel = VISIONSETMODE_INFRARED;
        DrawVisionSetDebug(localClientNum, VISIONSETMODE_INFRARED);
    }
    else if ( isExtracam )
    {
        visionChannel = VISIONSETMODE_EXTRACAM;
        DrawVisionSetDebug(localClientNum, VISIONSETMODE_EXTRACAM);
    }
    else if ( CG_IsTvguided(localClientNum, 0) )
    {
        visionChannel = VISIONSETMODE_TVGUIDED;
        DrawVisionSetDebug(localClientNum, VISIONSETMODE_TVGUIDED);
    }
    else
    {
        visionChannel = (visionSetMode_t)(CG_LookingThroughNightVision(localClientNum) != 0);
        DrawVisionSetDebug(localClientNum, visionChannel);
    }
    if ( cgameGlob->visionSetLerpData[visionChannel].style )
    {
        cgameGlob->refdef.visionset.film.enabled = cgameGlob->visionSetCurrent[visionChannel].filmEnable;
        cgameGlob->refdef.visionset.film.filmHue[0] = cgameGlob->visionSetCurrent[visionChannel].filmHue[0];
        cgameGlob->refdef.visionset.film.filmHue[1] = cgameGlob->visionSetCurrent[visionChannel].filmHue[1];
        cgameGlob->refdef.visionset.film.filmHue[2] = cgameGlob->visionSetCurrent[visionChannel].filmHue[2];
        cgameGlob->refdef.visionset.film.filmSaturation[0] = cgameGlob->visionSetCurrent[visionChannel].filmSaturation[0];
        cgameGlob->refdef.visionset.film.filmSaturation[1] = cgameGlob->visionSetCurrent[visionChannel].filmSaturation[1];
        cgameGlob->refdef.visionset.film.filmSaturation[2] = cgameGlob->visionSetCurrent[visionChannel].filmSaturation[2];
        cgameGlob->refdef.visionset.film.filmColorTemp[0] = cgameGlob->visionSetCurrent[visionChannel].filmColorTemp[0];
        cgameGlob->refdef.visionset.film.filmColorTemp[1] = cgameGlob->visionSetCurrent[visionChannel].filmColorTemp[1];
        cgameGlob->refdef.visionset.film.filmColorTemp[2] = cgameGlob->visionSetCurrent[visionChannel].filmColorTemp[2];
        cgameGlob->refdef.visionset.film.filmDarkTint[0] = cgameGlob->visionSetCurrent[visionChannel].filmDarkTint[0];
        cgameGlob->refdef.visionset.film.filmDarkTint[1] = cgameGlob->visionSetCurrent[visionChannel].filmDarkTint[1];
        cgameGlob->refdef.visionset.film.filmDarkTint[2] = cgameGlob->visionSetCurrent[visionChannel].filmDarkTint[2];
        cgameGlob->refdef.visionset.film.filmMidTint[0] = cgameGlob->visionSetCurrent[visionChannel].filmMidTint[0];
        cgameGlob->refdef.visionset.film.filmMidTint[1] = cgameGlob->visionSetCurrent[visionChannel].filmMidTint[1];
        cgameGlob->refdef.visionset.film.filmMidTint[2] = cgameGlob->visionSetCurrent[visionChannel].filmMidTint[2];
        cgameGlob->refdef.visionset.film.filmLightTint[0] = cgameGlob->visionSetCurrent[visionChannel].filmLightTint[0];
        cgameGlob->refdef.visionset.film.filmLightTint[1] = cgameGlob->visionSetCurrent[visionChannel].filmLightTint[1];
        cgameGlob->refdef.visionset.film.filmLightTint[2] = cgameGlob->visionSetCurrent[visionChannel].filmLightTint[2];
        cgameGlob->refdef.visionset.film.filmContrast[0] = cgameGlob->visionSetCurrent[visionChannel].filmContrast[0];
        cgameGlob->refdef.visionset.film.filmContrast[1] = cgameGlob->visionSetCurrent[visionChannel].filmContrast[1];
        cgameGlob->refdef.visionset.film.filmContrast[2] = cgameGlob->visionSetCurrent[visionChannel].filmContrast[2];
        cgameGlob->refdef.visionset.film.filmBleach[0] = cgameGlob->visionSetCurrent[visionChannel].filmBleach[0];
        cgameGlob->refdef.visionset.film.filmBleach[1] = cgameGlob->visionSetCurrent[visionChannel].filmBleach[1];
        cgameGlob->refdef.visionset.film.filmBleach[2] = cgameGlob->visionSetCurrent[visionChannel].filmBleach[2];
        cgameGlob->refdef.visionset.film.sCurveEnable = (float)cgameGlob->visionSetCurrent[visionChannel].sCurveEnable;
        cgameGlob->refdef.visionset.film.sCurveShoulderStrength = cgameGlob->visionSetCurrent[visionChannel].sCurveShoulderStrength;
        cgameGlob->refdef.visionset.film.sCurveLinearStrength = cgameGlob->visionSetCurrent[visionChannel].sCurveLinearStrength;
        cgameGlob->refdef.visionset.film.sCurveLinearAngle = cgameGlob->visionSetCurrent[visionChannel].sCurveLinearAngle;
        cgameGlob->refdef.visionset.film.sCurveToeStrength = cgameGlob->visionSetCurrent[visionChannel].sCurveToeStrength;
        cgameGlob->refdef.visionset.film.sCurveToeNumerator = cgameGlob->visionSetCurrent[visionChannel].sCurveToeNumerator;
        cgameGlob->refdef.visionset.film.sCurveToeDenominator = cgameGlob->visionSetCurrent[visionChannel].sCurveToeDenominator;
        cgameGlob->refdef.visionset.film.filmMidStart = cgameGlob->visionSetCurrent[visionChannel].filmMidStart;
        cgameGlob->refdef.visionset.film.filmMidEnd = cgameGlob->visionSetCurrent[visionChannel].filmMidEnd;
        cgameGlob->refdef.visionset.film.filmDarkFeather = cgameGlob->visionSetCurrent[visionChannel].filmDarkFeather;
        cgameGlob->refdef.visionset.film.filmLightFeather = cgameGlob->visionSetCurrent[visionChannel].filmLightFeather;
        cgameGlob->refdef.visionset.film.filmLut = cgameGlob->visionSetCurrent[visionChannel].filmLut;
        cgameGlob->refdef.visionset.bloom.bloomTintWeights[0] = cgameGlob->visionSetCurrent[visionChannel].bloomTintWeights[0];
        cgameGlob->refdef.visionset.bloom.bloomTintWeights[1] = cgameGlob->visionSetCurrent[visionChannel].bloomTintWeights[1];
        cgameGlob->refdef.visionset.bloom.bloomTintWeights[2] = cgameGlob->visionSetCurrent[visionChannel].bloomTintWeights[2];
        cgameGlob->refdef.visionset.bloom.bloomTintWeights[3] = cgameGlob->visionSetCurrent[visionChannel].bloomTintWeights[3];
        cgameGlob->refdef.visionset.bloom.bloomColorScale[0] = cgameGlob->visionSetCurrent[visionChannel].bloomColorScale[0];
        cgameGlob->refdef.visionset.bloom.bloomColorScale[1] = cgameGlob->visionSetCurrent[visionChannel].bloomColorScale[1];
        cgameGlob->refdef.visionset.bloom.bloomColorScale[2] = cgameGlob->visionSetCurrent[visionChannel].bloomColorScale[2];
        cgameGlob->refdef.visionset.bloom.bloomColorScale[3] = cgameGlob->visionSetCurrent[visionChannel].bloomColorScale[3];
        cgameGlob->refdef.visionset.bloom.bloomTintScale[0] = cgameGlob->visionSetCurrent[visionChannel].bloomTintScale[0];
        cgameGlob->refdef.visionset.bloom.bloomTintScale[1] = cgameGlob->visionSetCurrent[visionChannel].bloomTintScale[1];
        cgameGlob->refdef.visionset.bloom.bloomTintScale[2] = cgameGlob->visionSetCurrent[visionChannel].bloomTintScale[2];
        cgameGlob->refdef.visionset.bloom.bloomTintScale[3] = cgameGlob->visionSetCurrent[visionChannel].bloomTintScale[3];
        cgameGlob->refdef.visionset.bloom.bloomCurveBreakpoint[0] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveBreakpoint[0];
        cgameGlob->refdef.visionset.bloom.bloomCurveBreakpoint[1] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveBreakpoint[1];
        cgameGlob->refdef.visionset.bloom.bloomCurveBreakpoint[2] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveBreakpoint[2];
        cgameGlob->refdef.visionset.bloom.bloomCurveBreakpoint[3] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveBreakpoint[3];
        cgameGlob->refdef.visionset.bloom.bloomCurveLoBlack[0] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveLoBlack[0];
        cgameGlob->refdef.visionset.bloom.bloomCurveLoBlack[1] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveLoBlack[1];
        cgameGlob->refdef.visionset.bloom.bloomCurveLoBlack[2] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveLoBlack[2];
        cgameGlob->refdef.visionset.bloom.bloomCurveLoBlack[3] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveLoBlack[3];
        cgameGlob->refdef.visionset.bloom.bloomCurveLoGamma[0] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveLoGamma[0];
        cgameGlob->refdef.visionset.bloom.bloomCurveLoGamma[1] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveLoGamma[1];
        cgameGlob->refdef.visionset.bloom.bloomCurveLoGamma[2] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveLoGamma[2];
        cgameGlob->refdef.visionset.bloom.bloomCurveLoGamma[3] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveLoGamma[3];
        cgameGlob->refdef.visionset.bloom.bloomCurveLoWhite[0] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveLoWhite[0];
        cgameGlob->refdef.visionset.bloom.bloomCurveLoWhite[1] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveLoWhite[1];
        cgameGlob->refdef.visionset.bloom.bloomCurveLoWhite[2] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveLoWhite[2];
        cgameGlob->refdef.visionset.bloom.bloomCurveLoWhite[3] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveLoWhite[3];
        cgameGlob->refdef.visionset.bloom.bloomCurveLoRemapBlack[0] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveLoRemapBlack[0];
        cgameGlob->refdef.visionset.bloom.bloomCurveLoRemapBlack[1] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveLoRemapBlack[1];
        cgameGlob->refdef.visionset.bloom.bloomCurveLoRemapBlack[2] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveLoRemapBlack[2];
        cgameGlob->refdef.visionset.bloom.bloomCurveLoRemapBlack[3] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveLoRemapBlack[3];
        cgameGlob->refdef.visionset.bloom.bloomCurveLoRemapWhite[0] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveLoRemapWhite[0];
        cgameGlob->refdef.visionset.bloom.bloomCurveLoRemapWhite[1] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveLoRemapWhite[1];
        cgameGlob->refdef.visionset.bloom.bloomCurveLoRemapWhite[2] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveLoRemapWhite[2];
        cgameGlob->refdef.visionset.bloom.bloomCurveLoRemapWhite[3] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveLoRemapWhite[3];
        cgameGlob->refdef.visionset.bloom.bloomCurveHiBlack[0] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveHiBlack[0];
        cgameGlob->refdef.visionset.bloom.bloomCurveHiBlack[1] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveHiBlack[1];
        cgameGlob->refdef.visionset.bloom.bloomCurveHiBlack[2] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveHiBlack[2];
        cgameGlob->refdef.visionset.bloom.bloomCurveHiBlack[3] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveHiBlack[3];
        cgameGlob->refdef.visionset.bloom.bloomCurveHiGamma[0] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveHiGamma[0];
        cgameGlob->refdef.visionset.bloom.bloomCurveHiGamma[1] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveHiGamma[1];
        cgameGlob->refdef.visionset.bloom.bloomCurveHiGamma[2] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveHiGamma[2];
        cgameGlob->refdef.visionset.bloom.bloomCurveHiGamma[3] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveHiGamma[3];
        cgameGlob->refdef.visionset.bloom.bloomCurveHiWhite[0] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveHiWhite[0];
        cgameGlob->refdef.visionset.bloom.bloomCurveHiWhite[1] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveHiWhite[1];
        cgameGlob->refdef.visionset.bloom.bloomCurveHiWhite[2] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveHiWhite[2];
        cgameGlob->refdef.visionset.bloom.bloomCurveHiWhite[3] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveHiWhite[3];
        cgameGlob->refdef.visionset.bloom.bloomCurveHiRemapBlack[0] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveHiRemapBlack[0];
        cgameGlob->refdef.visionset.bloom.bloomCurveHiRemapBlack[1] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveHiRemapBlack[1];
        cgameGlob->refdef.visionset.bloom.bloomCurveHiRemapBlack[2] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveHiRemapBlack[2];
        cgameGlob->refdef.visionset.bloom.bloomCurveHiRemapBlack[3] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveHiRemapBlack[3];
        cgameGlob->refdef.visionset.bloom.bloomCurveHiRemapWhite[0] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveHiRemapWhite[0];
        cgameGlob->refdef.visionset.bloom.bloomCurveHiRemapWhite[1] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveHiRemapWhite[1];
        cgameGlob->refdef.visionset.bloom.bloomCurveHiRemapWhite[2] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveHiRemapWhite[2];
        cgameGlob->refdef.visionset.bloom.bloomCurveHiRemapWhite[3] = cgameGlob->visionSetCurrent[visionChannel].bloomCurveHiRemapWhite[3];
        cgameGlob->refdef.visionset.bloom.bloomExpansionControl[0] = cgameGlob->visionSetCurrent[visionChannel].bloomExpansionControl[0];
        cgameGlob->refdef.visionset.bloom.bloomExpansionControl[1] = cgameGlob->visionSetCurrent[visionChannel].bloomExpansionControl[1];
        cgameGlob->refdef.visionset.bloom.bloomExpansionControl[2] = cgameGlob->visionSetCurrent[visionChannel].bloomExpansionControl[2];
        cgameGlob->refdef.visionset.bloom.bloomExpansionControl[3] = cgameGlob->visionSetCurrent[visionChannel].bloomExpansionControl[3];
        cgameGlob->refdef.visionset.bloom.bloomExpansionWeights[0] = cgameGlob->visionSetCurrent[visionChannel].bloomExpansionWeights[0];
        cgameGlob->refdef.visionset.bloom.bloomExpansionWeights[1] = cgameGlob->visionSetCurrent[visionChannel].bloomExpansionWeights[1];
        cgameGlob->refdef.visionset.bloom.bloomExpansionWeights[2] = cgameGlob->visionSetCurrent[visionChannel].bloomExpansionWeights[2];
        cgameGlob->refdef.visionset.bloom.bloomExpansionWeights[3] = cgameGlob->visionSetCurrent[visionChannel].bloomExpansionWeights[3];
        cgameGlob->refdef.visionset.bloom.bloomBlurRadius = cgameGlob->visionSetCurrent[visionChannel].bloomBlurRadius;
        cgameGlob->refdef.visionset.bloom.bloomExpansionSource = cgameGlob->visionSetCurrent[visionChannel].bloomExpansionSource;
        cgameGlob->refdef.visionset.bloom.bloomPersistence = cgameGlob->visionSetCurrent[visionChannel].bloomPersistence;
        cgameGlob->refdef.visionset.bloom.bloomStreakXLevels0[0] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXLevels0[0];
        cgameGlob->refdef.visionset.bloom.bloomStreakXLevels0[1] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXLevels0[1];
        cgameGlob->refdef.visionset.bloom.bloomStreakXLevels0[2] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXLevels0[2];
        cgameGlob->refdef.visionset.bloom.bloomStreakXLevels0[3] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXLevels0[3];
        cgameGlob->refdef.visionset.bloom.bloomStreakXLevels1[0] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXLevels1[0];
        cgameGlob->refdef.visionset.bloom.bloomStreakXLevels1[1] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXLevels1[1];
        cgameGlob->refdef.visionset.bloom.bloomStreakXLevels1[2] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXLevels1[2];
        cgameGlob->refdef.visionset.bloom.bloomStreakXLevels1[3] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXLevels1[3];
        cgameGlob->refdef.visionset.bloom.bloomStreakXInnerTint[0] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXInnerTint[0];
        cgameGlob->refdef.visionset.bloom.bloomStreakXInnerTint[1] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXInnerTint[1];
        cgameGlob->refdef.visionset.bloom.bloomStreakXInnerTint[2] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXInnerTint[2];
        cgameGlob->refdef.visionset.bloom.bloomStreakXOuterTint[0] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXOuterTint[0];
        cgameGlob->refdef.visionset.bloom.bloomStreakXOuterTint[1] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXOuterTint[1];
        cgameGlob->refdef.visionset.bloom.bloomStreakXOuterTint[2] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXOuterTint[2];
        cgameGlob->refdef.visionset.bloom.bloomStreakXTintControl[0] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXTintControl[0];
        cgameGlob->refdef.visionset.bloom.bloomStreakXTintControl[1] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXTintControl[1];
        cgameGlob->refdef.visionset.bloom.bloomStreakXTintControl[2] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXTintControl[2];
        cgameGlob->refdef.visionset.bloom.bloomStreakXTintControl[3] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXTintControl[3];
        cgameGlob->refdef.visionset.bloom.bloomStreakXTint[0] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXTint[0];
        cgameGlob->refdef.visionset.bloom.bloomStreakXTint[1] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXTint[1];
        cgameGlob->refdef.visionset.bloom.bloomStreakXTint[2] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakXTint[2];
        cgameGlob->refdef.visionset.bloom.bloomStreakYLevels0[0] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYLevels0[0];
        cgameGlob->refdef.visionset.bloom.bloomStreakYLevels0[1] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYLevels0[1];
        cgameGlob->refdef.visionset.bloom.bloomStreakYLevels0[2] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYLevels0[2];
        cgameGlob->refdef.visionset.bloom.bloomStreakYLevels0[3] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYLevels0[3];
        cgameGlob->refdef.visionset.bloom.bloomStreakYLevels1[0] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYLevels1[0];
        cgameGlob->refdef.visionset.bloom.bloomStreakYLevels1[1] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYLevels1[1];
        cgameGlob->refdef.visionset.bloom.bloomStreakYLevels1[2] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYLevels1[2];
        cgameGlob->refdef.visionset.bloom.bloomStreakYLevels1[3] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYLevels1[3];
        cgameGlob->refdef.visionset.bloom.bloomStreakYInnerTint[0] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYInnerTint[0];
        cgameGlob->refdef.visionset.bloom.bloomStreakYInnerTint[1] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYInnerTint[1];
        cgameGlob->refdef.visionset.bloom.bloomStreakYInnerTint[2] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYInnerTint[2];
        cgameGlob->refdef.visionset.bloom.bloomStreakYOuterTint[0] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYOuterTint[0];
        cgameGlob->refdef.visionset.bloom.bloomStreakYOuterTint[1] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYOuterTint[1];
        cgameGlob->refdef.visionset.bloom.bloomStreakYOuterTint[2] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYOuterTint[2];
        cgameGlob->refdef.visionset.bloom.bloomStreakYTintControl[0] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYTintControl[0];
        cgameGlob->refdef.visionset.bloom.bloomStreakYTintControl[1] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYTintControl[1];
        cgameGlob->refdef.visionset.bloom.bloomStreakYTintControl[2] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYTintControl[2];
        cgameGlob->refdef.visionset.bloom.bloomStreakYTintControl[3] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYTintControl[3];
        cgameGlob->refdef.visionset.bloom.bloomStreakYTint[0] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYTint[0];
        cgameGlob->refdef.visionset.bloom.bloomStreakYTint[1] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYTint[1];
        cgameGlob->refdef.visionset.bloom.bloomStreakYTint[2] = cgameGlob->visionSetCurrent[visionChannel].bloomStreakYTint[2];
        reviveFx->enabled = cgameGlob->visionSetCurrent[visionChannel].reviveEnable;
        cgameGlob->refdef.visionset.reviveFx.reviveEdgeColorTemp = cgameGlob->visionSetCurrent[visionChannel].reviveEdgeColorTemp;
        cgameGlob->refdef.visionset.reviveFx.reviveEdgeSaturation = cgameGlob->visionSetCurrent[visionChannel].reviveEdgeSaturation;
        cgameGlob->refdef.visionset.reviveFx.reviveEdgeScale[0] = cgameGlob->visionSetCurrent[visionChannel].reviveEdgeScale[0];
        cgameGlob->refdef.visionset.reviveFx.reviveEdgeScale[1] = cgameGlob->visionSetCurrent[visionChannel].reviveEdgeScale[1];
        cgameGlob->refdef.visionset.reviveFx.reviveEdgeScale[2] = cgameGlob->visionSetCurrent[visionChannel].reviveEdgeScale[2];
        cgameGlob->refdef.visionset.reviveFx.reviveEdgeContrast[0] = cgameGlob->visionSetCurrent[visionChannel].reviveEdgeContrast[0];
        cgameGlob->refdef.visionset.reviveFx.reviveEdgeContrast[1] = cgameGlob->visionSetCurrent[visionChannel].reviveEdgeContrast[1];
        cgameGlob->refdef.visionset.reviveFx.reviveEdgeContrast[2] = cgameGlob->visionSetCurrent[visionChannel].reviveEdgeContrast[2];
        cgameGlob->refdef.visionset.reviveFx.reviveEdgeOffset[0] = cgameGlob->visionSetCurrent[visionChannel].reviveEdgeOffset[0];
        cgameGlob->refdef.visionset.reviveFx.reviveEdgeOffset[1] = cgameGlob->visionSetCurrent[visionChannel].reviveEdgeOffset[1];
        cgameGlob->refdef.visionset.reviveFx.reviveEdgeOffset[2] = cgameGlob->visionSetCurrent[visionChannel].reviveEdgeOffset[2];
        cgameGlob->refdef.visionset.reviveFx.reviveEdgeMaskAdjust = cgameGlob->visionSetCurrent[visionChannel].reviveEdgeMaskAdjust;
        cgameGlob->refdef.visionset.reviveFx.reviveEdgeAmount = cgameGlob->visionSetCurrent[visionChannel].reviveEdgeAmount;
        if ( cgameGlob->visionSetCurrent[visionChannel].charPrimaryUseTweaks )
        {
            charLightScale->diffuseScale = cgameGlob->visionSetCurrent[visionChannel].charPrimaryDiffuseScale;
            cgameGlob->refdef.visionset.charPrimaryLightScale.specularScale = cgameGlob->visionSetCurrent[visionChannel].charPrimarySpecularScale;
        }
        else
        {
            charLightScale->diffuseScale = 1.0f;
            cgameGlob->refdef.visionset.charPrimaryLightScale.specularScale = 1.0f;
        }
        cgameGlob->refdef.postEmissiveBrightening = cgameGlob->visionSetCurrent[visionChannel].postEmissiveBrightening;
    }
    else
    {
        cgameGlob->refdef.visionset.film.enabled = 0;
        reviveFx->enabled = 0;
        cgameGlob->refdef.postEmissiveBrightening = 0.0f;
    }
}

void __cdecl CG_InitExposure(int localClientNum, char *mapname)
{
    char *v2; // eax
    char *v3; // eax
    char v4; // [esp+23h] [ebp-121h]
    char *v5; // [esp+28h] [ebp-11Ch]
    char *v6; // [esp+2Ch] [ebp-118h]
    const char *basename; // [esp+30h] [ebp-114h]
    char *r; // [esp+34h] [ebp-110h]
    char *fileBuf; // [esp+38h] [ebp-10Ch]
    char temp[260]; // [esp+3Ch] [ebp-108h] BYREF

    v2 = strrchr(mapname, '/');
    if (v2)
        r = v2 + 1;
    else
        r = mapname;
    r[255] = 0;
    v6 = r;
    v5 = temp;
    do
    {
        v4 = *v6;
        *v5++ = *v6++;
    }
    while ( v4 );
    v3 = strstr(temp, "_geo");
    if ( v3 )
        *v3 = 0;
    R_SetExposureToDefault();
    fileBuf = RawBufferOpen(temp, "exposure/%s.xpo");
    if ( !fileBuf && !I_strncmp("so_", mapname, strlen("so_")) )
    {
        for ( basename = &mapname[strlen("so_")]; *basename && *basename != 95; ++basename )
            ;
        if ( !*basename )
            Com_PrintError(1, "Bad specop level name\n");
        Com_sprintf(temp, 0x100u, "%s", basename + 1);
        fileBuf = RawBufferOpen(temp, "exposure/%s.xpo");
    }
    if ( fileBuf )
    {
        R_LoadExposureData((unsigned int*)fileBuf);
        Com_UnloadRawTextFile(fileBuf);
    }
}

