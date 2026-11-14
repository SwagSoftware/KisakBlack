#include "cg_hudelem.h"
#include <stringed/stringed_hooks.h>
#include <ui/ui_shared.h>
#include <cgame_mp/cg_local_mp.h>
#include <client_mp/cl_cgame_mp.h>
#include <cgame_mp/cg_main_mp.h>
#include <cgame_mp/cg_view_mp.h>
#include <demo/demo_playback.h>
#include <gfx_d3d/r_ui3d.h>
#include <bgame/bg_misc.h>
#include <client/cl_console.h>
#include <cgame_mp/cg_newDraw_mp.h>
#include "cg_drawtools.h"
#include <gfx_d3d/rb_corona.h>
#include <EffectsCore/fx_sprite.h>
#include <qcommon/common.h>

const dvar_t *waypointDebugDraw;
const dvar_t *waypointIconWidth;
const dvar_t *waypointIconHeight;
const dvar_t *waypointOffscreenPointerWidth;
const dvar_t *waypointOffscreenPointerHeight;
const dvar_t *waypointOffscreenPointerDistance;
const dvar_t *waypointOffscreenDistanceThresholdAlpha;
const dvar_t *waypointOffscreenPadLeft;
const dvar_t *waypointOffscreenPadRight;
const dvar_t *waypointOffscreenPadTop;
const dvar_t *waypointOffscreenPadBottom;
const dvar_t *waypointOffscreenRoundedCorners;
const dvar_t *waypointOffscreenCornerRadius;
const dvar_t *waypointOffscreenScaleLength;
const dvar_t *waypointOffscreenScaleSmallest;
const dvar_t *waypointDistScaleRangeMin;
const dvar_t *waypointDistScaleRangeMax;
const dvar_t *waypointDistScaleSmallest;
const dvar_t *waypointDistFade;
const dvar_t *waypointTimeFade;
const dvar_t *waypointSplitscreenScale;
const dvar_t *waypointTweakY;
const dvar_t *hudElemPausedBrightness;
const dvar_t *waypointPlayerOffsetProne;
const dvar_t *waypointPlayerOffsetCrouch;
const dvar_t *waypointPlayerOffsetStand;
const dvar_t *waypointPlayerOffsetRevive;

game_hudelem_s g_hudelems[1024];

const float s_alignScale[4] = { 0.0, 0.5, 1.0, 0.0 };
bool r_highRezWaypoints = true;

void __cdecl CG_HudElemRegisterDvars()
{
    waypointDebugDraw = _Dvar_RegisterBool("waypointDebugDraw", 0, 0, "");
    waypointIconWidth = _Dvar_RegisterFloat(
                                                "waypointIconWidth",
                                                36.0,
                                                1.1754944e-38,
                                                3.4028235e38,
                                                0,
                                                "Width of the offscreen pointer.");
    waypointIconHeight = _Dvar_RegisterFloat(
                                                 "waypointIconHeight",
                                                 36.0,
                                                 1.1754944e-38,
                                                 3.4028235e38,
                                                 0,
                                                 "Height of the offscreen pointer.");
    waypointOffscreenPointerWidth = _Dvar_RegisterFloat(
                                                                        "waypointOffscreenPointerWidth",
                                                                        25.0,
                                                                        1.1754944e-38,
                                                                        3.4028235e38,
                                                                        0,
                                                                        "Width of the offscreen pointer.");
    waypointOffscreenPointerHeight = _Dvar_RegisterFloat(
                                                                         "waypointOffscreenPointerHeight",
                                                                         12.0,
                                                                         1.1754944e-38,
                                                                         3.4028235e38,
                                                                         0,
                                                                         "Height of the offscreen pointer.");
    waypointOffscreenPointerDistance = _Dvar_RegisterFloat(
                                                                             "waypointOffscreenPointerDistance",
                                                                             30.0,
                                                                             1.1754944e-38,
                                                                             3.4028235e38,
                                                                             0,
                                                                             "Distance from the center of the offscreen objective icon to the center its arrow.");
    waypointOffscreenDistanceThresholdAlpha = _Dvar_RegisterFloat(
                                                                                            "waypointOffscreenDistanceThresholdAlpha",
                                                                                            30.0,
                                                                                            0.0,
                                                                                            3.4028235e38,
                                                                                            0,
                                                                                            "Distance from the threshold over which offscreen objective icons lerp their alpha.");
    waypointOffscreenPadLeft = _Dvar_RegisterFloat(
                                                             "waypointOffscreenPadLeft",
                                                             103.0,
                                                             0.0,
                                                             3.4028235e38,
                                                             0,
                                                             "Offset from the edge.");
    waypointOffscreenPadRight = _Dvar_RegisterFloat(
                                                                "waypointOffscreenPadRight",
                                                                0.0,
                                                                0.0,
                                                                3.4028235e38,
                                                                0,
                                                                "Offset from the edge.");
    waypointOffscreenPadTop = _Dvar_RegisterFloat(
                                                            "waypointOffscreenPadTop",
                                                            0.0,
                                                            0.0,
                                                            3.4028235e38,
                                                            0,
                                                            "Offset from the edge.");
    waypointOffscreenPadBottom = _Dvar_RegisterFloat(
                                                                 "waypointOffscreenPadBottom",
                                                                 30.0,
                                                                 0.0,
                                                                 3.4028235e38,
                                                                 0,
                                                                 "Offset from the edge.");
    waypointOffscreenRoundedCorners = _Dvar_RegisterBool(
                                                                            "waypointOffscreenRoundedCorners",
                                                                            1,
                                                                            0,
                                                                            "Off-screen icons take rounded corners when true.    90-degree corners when false.");
    waypointOffscreenCornerRadius = _Dvar_RegisterFloat(
                                                                        "waypointOffscreenCornerRadius",
                                                                        105.0,
                                                                        0.0,
                                                                        3.4028235e38,
                                                                        0,
                                                                        "Size of the rounded corners.");
    waypointOffscreenScaleLength = _Dvar_RegisterFloat(
                                                                     "waypointOffscreenScaleLength",
                                                                     500.0,
                                                                     1.1754944e-38,
                                                                     3.4028235e38,
                                                                     0,
                                                                     "How far the offscreen icon scale travels from full to smallest scale.");
    waypointOffscreenScaleSmallest = _Dvar_RegisterFloat(
                                                                         "waypointOffscreenScaleSmallest",
                                                                         1.0,
                                                                         0.0,
                                                                         3.4028235e38,
                                                                         0,
                                                                         "Smallest scale that the offscreen effect uses.");
    waypointDistScaleRangeMin = _Dvar_RegisterFloat(
                                                                "waypointDistScaleRangeMin",
                                                                1000.0,
                                                                0.0,
                                                                3.4028235e38,
                                                                0,
                                                                "Distance from player that icon distance scaling starts.");
    waypointDistScaleRangeMax = _Dvar_RegisterFloat(
                                                                "waypointDistScaleRangeMax",
                                                                3000.0,
                                                                0.0,
                                                                3.4028235e38,
                                                                0,
                                                                "Distance from player that icon distance scaling ends.");
    waypointDistScaleSmallest = _Dvar_RegisterFloat(
                                                                "waypointDistScaleSmallest",
                                                                0.80000001,
                                                                0.0,
                                                                3.4028235e38,
                                                                0,
                                                                "Smallest scale that the distance effect uses.");
    waypointDistFade = _Dvar_RegisterFloat(
                                             "waypointDistFade",
                                             300.0,
                                             0.0,
                                             3.4028235e38,
                                             0x80u,
                                             "Distance to start fading waypoints when the player is ADS");
    waypointTimeFade = _Dvar_RegisterInt(
                                             "waypointTimeFade",
                                             500,
                                             0,
                                             10000,
                                             0x80u,
                                             "Time in ms to fade waypoints the player is ADS");
    waypointSplitscreenScale = _Dvar_RegisterFloat(
                                                             "waypointSplitscreenScale",
                                                             1.8,
                                                             0.1,
                                                             3.4028235e38,
                                                             0,
                                                             "Scale applied to waypoint icons in splitscreen views.");
    waypointTweakY = _Dvar_RegisterFloat("waypointTweakY", -17.0, -3.4028235e38, 3.4028235e38, 0, "");
    hudElemPausedBrightness = _Dvar_RegisterFloat(
                                                            "hudElemPausedBrightness",
                                                            0.40000001,
                                                            0.0,
                                                            1.0,
                                                            0x80u,
                                                            "Brightness of the hudelems when the game is paused.");
    waypointPlayerOffsetProne = _Dvar_RegisterFloat(
                                                                "waypointPlayerOffsetProne",
                                                                30.0,
                                                                0.0,
                                                                3.4028235e38,
                                                                0x80u,
                                                                "For waypoints pointing to players, how high to offset off of their origin when they are prone.");
    waypointPlayerOffsetCrouch = _Dvar_RegisterFloat(
                                                                 "waypointPlayerOffsetCrouch",
                                                                 56.0,
                                                                 0.0,
                                                                 3.4028235e38,
                                                                 0x80u,
                                                                 "For waypoints pointing to players, how high to offset off of their origin when they are crouching.");
    waypointPlayerOffsetStand = _Dvar_RegisterFloat(
                                                                "waypointPlayerOffsetStand",
                                                                74.0,
                                                                0.0,
                                                                3.4028235e38,
                                                                0x80u,
                                                                "For waypoints pointing to players, how high to offset off of their origin when they are standing.");
    waypointPlayerOffsetRevive = _Dvar_RegisterFloat(
                                                                 "waypointPlayerOffsetRevive",
                                                                 64.0,
                                                                 0.0,
                                                                 3.4028235e38,
                                                                 0x80u,
                                                                 "For waypoints pointing to players, how high to offset off of their origin when they are in last stand.");
}

void __cdecl CG_TranslateHudElemMessage(
                int localClientNum,
                const char *message,
                const char *messageType,
                char *hudElemString)
{
    const char *v4; // eax
    char *translatedString; // [esp+10h] [ebp-Ch]
    unsigned int stringLen; // [esp+14h] [ebp-8h] BYREF
    unsigned int searchPos; // [esp+18h] [ebp-4h] BYREF

    if ( !message
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 347, 0, "%s", "message") )
    {
        __debugbreak();
    }
    if ( !hudElemString
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 348, 0, "%s", "hudElemString") )
    {
        __debugbreak();
    }
    translatedString = (char *)SEH_LocalizeTextMessage(message, messageType, LOCMSG_SAFE);
    stringLen = strlen(translatedString);
    if ( stringLen + 1 <= 0x100 )
    {
        memcpy((unsigned __int8 *)hudElemString, (unsigned __int8 *)translatedString, stringLen);
        hudElemString[stringLen] = 0;
        searchPos = 0;
        while ( ReplaceDirective(localClientNum, &searchPos, &stringLen, hudElemString) )
            ;
    }
    else
    {
        v4 = va("Translated message too long to process: %s\n", message);
        Com_Error(ERR_DROP, v4);
    }
}

char __cdecl ReplaceDirective(int localClientNum, unsigned int *searchPos, unsigned int *dstLen, char *dstString)
{
    char *v4; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    signed int directiveLen; // [esp+34h] [ebp-324h]
    const char *startTokenPos; // [esp+38h] [ebp-320h]
    int newStringLen; // [esp+3Ch] [ebp-31Ch]
    unsigned int bindingLen; // [esp+40h] [ebp-318h]
    unsigned int endLen; // [esp+44h] [ebp-314h]
    char keyBinding[256]; // [esp+48h] [ebp-310h] BYREF
    unsigned __int8 srcString[260]; // [esp+148h] [ebp-210h] BYREF
    int beginLen; // [esp+24Ch] [ebp-10Ch]
    char directive[256]; // [esp+250h] [ebp-108h] BYREF
    const char *endTokenPos; // [esp+354h] [ebp-4h]
    char *dstStringa; // [esp+36Ch] [ebp+14h]
    unsigned __int8 *dstStringb; // [esp+36Ch] [ebp+14h]

    if ( !searchPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 272, 0, "%s", "searchPos") )
    {
        __debugbreak();
    }
    if ( !dstLen && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 274, 0, "%s", "dstLen") )
        __debugbreak();
    if ( *dstLen >= 0x100
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp",
                    275,
                    0,
                    "*dstLen doesn't index MAX_HUDELEM_TEXT_LEN\n\t%i not in [0, %i)",
                    *dstLen,
                    256) )
    {
        __debugbreak();
    }
    if ( *searchPos > *dstLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp",
                    276,
                    0,
                    "*searchPos <= *dstLen\n\t%i, %i",
                    *searchPos,
                    *dstLen) )
    {
        __debugbreak();
    }
    if ( !dstString
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 277, 0, "%s", "dstString") )
    {
        __debugbreak();
    }
    if ( *dstLen != strlen(dstString)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp",
                    278,
                    0,
                    "*dstLen == strlen( dstString )\n\t%i, %i",
                    *dstLen,
                    strlen(dstString)) )
    {
        __debugbreak();
    }
    memcpy(srcString, (unsigned __int8 *)dstString, *dstLen);
    srcString[*dstLen] = 0;
    v4 = strstr((char*)&srcString[*searchPos], "[{");
    startTokenPos = (const char *)v4;
    if ( !v4 )
        return 0;
    v6 = strstr(v4, "}]");
    endTokenPos = v6;
    if ( v6 )
    {
        directiveLen = endTokenPos - startTokenPos - 2;
        if ( directiveLen < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp",
                        297,
                        1,
                        "%s\n\t(directiveLen) = %i",
                        "(directiveLen >= 0)",
                        directiveLen) )
        {
            __debugbreak();
        }
        if ( directiveLen )
        {
            memcpy((unsigned __int8 *)directive, (unsigned __int8 *)startTokenPos + 2, directiveLen);
            directive[directiveLen] = 0;
            GetHudelemDirective(localClientNum, directive, keyBinding);
            bindingLen = &keyBinding[strlen(keyBinding) + 1] - &keyBinding[1];
            newStringLen = *dstLen - directiveLen + bindingLen - 4;
            if ( newStringLen + 1 <= 256 )
            {
                beginLen = startTokenPos - (const char *)srcString;
                dstStringa = &dstString[startTokenPos - (const char *)srcString];
                memcpy((unsigned __int8 *)dstStringa, (unsigned __int8 *)keyBinding, bindingLen);
                dstStringb = (unsigned __int8 *)&dstStringa[bindingLen];
                endLen = newStringLen - beginLen - bindingLen;
                memcpy(dstStringb, (unsigned __int8 *)endTokenPos + 2, endLen);
                dstStringb[endLen] = 0;
                *searchPos = bindingLen + beginLen;
                *dstLen = newStringLen;
                if ( *searchPos > *dstLen
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp",
                                335,
                                1,
                                "*searchPos <= *dstLen\n\t%i, %i",
                                *searchPos,
                                *dstLen) )
                {
                    __debugbreak();
                }
                return 1;
            }
            else
            {
                v9 = va("String too long to add key binding: %s\n", dstString);
                Com_Error(ERR_DROP, v9);
                return 0;
            }
        }
        else
        {
            v8 = va("Directive empty in string '%s'", dstString);
            Com_Error(ERR_DROP, v8);
            return 0;
        }
    }
    else
    {
        v7 = va("No end token to match begin token in string '%s'", dstString);
        Com_Error(ERR_DROP, v7);
        return 0;
    }
}

void __cdecl GetHudelemDirective(int localClientNum, char *directive, char *result)
{
    char *v3; // eax
    char arg0[256]; // [esp+0h] [ebp-208h] BYREF
    char name[260]; // [esp+100h] [ebp-108h] BYREF

    if ( !UI_GetSticksTranslatedString(localClientNum, directive, result) )
    {
        if ( UI_GetKeyBindingLocalizedString(localClientNum, directive, result, 0) )
        {
            UI_FilterStringForButtonAnimation(result, 0x100u);
        }
        else
        {
            ParseDirective(directive, name, arg0);
            if ( I_stricmp(name, "FAKE_INTRO_SECONDS") )
            {
                if ( !I_stricmp(name, "gocrouch")
                    && (UI_GetKeyBindingLocalizedString(localClientNum, "togglecrouch", result, 0)
                     || UI_GetKeyBindingLocalizedString(localClientNum, "+movedown", result, 0)) )
                {
                    UI_FilterStringForButtonAnimation(result, 0x100u);
                }
                else
                {
                    v3 = UI_SafeTranslateString("KEY_UNBOUND");
                    Com_sprintf(result, 0x100u, "\"%s\"", v3);
                }
            }
            else
            {
                DirectiveFakeIntroSeconds(localClientNum, arg0, result);
            }
        }
    }
}

void __cdecl DirectiveFakeIntroSeconds(int localClientNum, const char *arg0, char *result)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]
    int fakeSeconds; // [esp+4h] [ebp-4h] BYREF

    fakeSeconds = 0;
    sscanf(arg0, "%d", &fakeSeconds);
    if ( (unsigned int)fakeSeconds > 0x28 )
    {
        fakeSeconds = 0;
        Com_PrintWarning(
            1,
            "Argument \"%s\" given for FAKE_INTRO_SECONDS is outside the acceptible range of (%d,%d).\n",
            arg0,
            0,
            40);
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    Com_sprintf(result, 4, "%02d", fakeSeconds + cgameGlob->time / 1000);
}

void __cdecl ParseDirective(char *directive, char *resultName, char *resultArg0)
{
    const char *v3; // eax
    const char *argpos; // [esp+4h] [ebp-4h]

    if ( !directive
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 195, 0, "%s", "directive") )
    {
        __debugbreak();
    }
    v3 = strstr(directive, ":");
    argpos = v3;
    if ( v3 )
    {
        memcpy((unsigned __int8 *)resultName, (unsigned __int8 *)directive, v3 - directive);
        resultName[argpos - directive] = 0;
        I_strncpyz(resultArg0, argpos + 1, 256);
    }
    else
    {
        I_strncpyz(resultName, directive, 256);
        *resultArg0 = 0;
    }
}

char __cdecl WorldPosToScreenPos(int localClientNum, const float *worldPos, float *outScreenPos)
{
    float v4; // [esp+0h] [ebp-4Ch]
    float v5; // [esp+Ch] [ebp-40h]
    ScreenPlacement *scrPlace; // [esp+28h] [ebp-24h]
    const cg_s *cgameGlob; // [esp+2Ch] [ebp-20h]
    float projections[3]; // [esp+30h] [ebp-1Ch] BYREF
    const refdef_s *refdef; // [esp+3Ch] [ebp-10h]
    float x; // [esp+40h] [ebp-Ch]
    float y; // [esp+44h] [ebp-8h]
    float w; // [esp+48h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    refdef = &cgameGlob->refdef;
    scrPlace = &scrPlaceView[localClientNum];
    GetViewAxisProjections(cgameGlob->refdef.vieworg, cgameGlob->refdef.viewaxis, worldPos, projections);
    w = projections[0];
    if ( projections[0] >= 0.0 )
    {
        x = projections[1] / refdef->tanHalfFovX;
        *outScreenPos = (float)((float)(0.5 * scrPlace->realViewportSize[0]) * (float)(1.0 - (float)(x / w)))
                                    + scrPlace->realViewportBase[0];
        y = projections[2] / refdef->tanHalfFovY;
        outScreenPos[1] = (float)((float)(0.5 * scrPlace->realViewportSize[1]) * (float)(1.0 - (float)(y / w)))
                                        + scrPlace->realViewportBase[1];
        return 1;
    }
    else
    {
        //*(unsigned int *)outScreenPos = LODWORD(projections[1]) ^ _mask__NegFloat_;
        outScreenPos[0] = -projections[1];
        //*((unsigned int *)outScreenPos + 1) = LODWORD(projections[2]) ^ _mask__NegFloat_;
        outScreenPos[1] = -projections[1];
        if ( fabsf(*(unsigned int *)outScreenPos) >= 0.001
            || fabsf((unsigned int)outScreenPos[1]) >= 0.001 )
        {
            if (fabsf(*(unsigned int *)outScreenPos) < 0.001 )
                *outScreenPos = 0.001f;
            if (fabsf((unsigned int)outScreenPos[1]) < 0.001 )
                outScreenPos[1] = 0.001f;
            while ( scrPlace->realViewportSize[0] > fabsf(*(unsigned int *)outScreenPos) )
            {
                v5 = scrPlace->realViewportSize[0];
                *outScreenPos = v5 * *outScreenPos;
                outScreenPos[1] = v5 * outScreenPos[1];
            }
            while ( scrPlace->realViewportSize[1] > fabsf((unsigned int)outScreenPos[1]) )
            {
                v4 = scrPlace->realViewportSize[1];
                *outScreenPos = v4 * *outScreenPos;
                outScreenPos[1] = v4 * outScreenPos[1];
            }
        }
        else
        {
            outScreenPos[1] = scrPlace->realViewportSize[1] * 2.0;
        }
        return 0;
    }
}

void __cdecl GetViewAxisProjections(
                const float *vieworg,
                const float (*viewaxis)[3],
                const float *worldPoint,
                float *projections)
{
    float eyeDelta; // [esp+4h] [ebp-10h]
    float eyeDelta_4; // [esp+8h] [ebp-Ch]
    float eyeDelta_8; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    eyeDelta = *worldPoint - *vieworg;
    eyeDelta_4 = worldPoint[1] - vieworg[1];
    eyeDelta_8 = worldPoint[2] - vieworg[2];
    for ( i = 0; i < 3; ++i )
        projections[i] = (float)((float)(eyeDelta * (float)(*viewaxis)[3 * i])
                                                     + (float)(eyeDelta_4 * (float)(*viewaxis)[3 * i + 1]))
                                     + (float)(eyeDelta_8 * (float)(*viewaxis)[3 * i + 2]);
}

char __cdecl WorldPosToExtraCamScreenPos(int localClientNum, const float *worldPos, float *outScreenPos)
{
    float v4; // [esp+0h] [ebp-4Ch]
    float v5; // [esp+Ch] [ebp-40h]
    ScreenPlacement *scrPlace; // [esp+28h] [ebp-24h]
    const cg_s *cgameGlob; // [esp+2Ch] [ebp-20h]
    float projections[3]; // [esp+30h] [ebp-1Ch] BYREF
    const refdef_s *refdef; // [esp+3Ch] [ebp-10h]
    float x; // [esp+40h] [ebp-Ch]
    float y; // [esp+44h] [ebp-8h]
    float w; // [esp+48h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    refdef = &cgameGlob->refdef;
    scrPlace = &scrPlaceView[localClientNum];
    GetViewAxisProjections(
        cgameGlob->refdef.preExtraCamVieworg,
        cgameGlob->refdef.preExtraCamViewaxis,
        worldPos,
        projections);
    w = projections[0];
    if ( projections[0] >= 0.0 )
    {
        x = projections[1] / refdef->preExtraCamTanHalfFovX;
        *outScreenPos = (float)((float)(0.5 * scrPlace->realViewportSize[0]) * (float)(1.0 - (float)(x / w)))
                                    + scrPlace->realViewportBase[0];
        y = projections[2] / refdef->preExtraCamTanHalfFovY;
        outScreenPos[1] = (float)((float)(0.5 * scrPlace->realViewportSize[1]) * (float)(1.0 - (float)(y / w)))
                                        + scrPlace->realViewportBase[1];
        return 1;
    }
    else
    {
        //*(unsigned int *)outScreenPos = LODWORD(projections[1]) ^ _mask__NegFloat_;
        outScreenPos[0] = -projections[1];
        //*((unsigned int *)outScreenPos + 1) = LODWORD(projections[2]) ^ _mask__NegFloat_;
        outScreenPos[1] = -projections[2];
        if ( fabsf(*(unsigned int *)outScreenPos) >= 0.001
            || fabsf((unsigned int)outScreenPos[1]) >= 0.001 )
        {
            if (fabsf(*(unsigned int *)outScreenPos) < 0.001 )
                *outScreenPos = 0.001f;
            if (fabsf((unsigned int)outScreenPos[1]) < 0.001 )
                outScreenPos[1] = 0.001f;
            while ( scrPlace->realViewportSize[0] > fabsf(*(unsigned int *)outScreenPos) )
            {
                v5 = scrPlace->realViewportSize[0];
                *outScreenPos = v5 * *outScreenPos;
                outScreenPos[1] = v5 * outScreenPos[1];
            }
            while ( scrPlace->realViewportSize[1] > fabsf((unsigned int)outScreenPos[1]) )
            {
                v4 = scrPlace->realViewportSize[1];
                *outScreenPos = v4 * *outScreenPos;
                outScreenPos[1] = v4 * outScreenPos[1];
            }
        }
        else
        {
            outScreenPos[1] = scrPlace->realViewportSize[1] * 2.0;
        }
        return 0;
    }
}

bool __cdecl ClampScreenPosToEdges(
                int localClientNum,
                float *point,
                float padLeft,
                float padRight,
                float padTop,
                float padBottom,
                float *resultNormal,
                float *resultDist)
{
    bool v9; // [esp+28h] [ebp-A0h]
    bool v10; // [esp+2Ch] [ebp-9Ch]
    float v11; // [esp+30h] [ebp-98h]
    float v12; // [esp+34h] [ebp-94h]
    float v13[3]; // [esp+50h] [ebp-78h] BYREF
    float v[5]; // [esp+5Ch] [ebp-6Ch] BYREF
    float v15; // [esp+70h] [ebp-58h]
    float v16; // [esp+74h] [ebp-54h]
    float v17; // [esp+78h] [ebp-50h]
    float v18; // [esp+7Ch] [ebp-4Ch]
    float dist; // [esp+80h] [ebp-48h]
    float dir[2]; // [esp+84h] [ebp-44h] BYREF
    bool top; // [esp+8Fh] [ebp-39h]
    float radius; // [esp+90h] [ebp-38h]
    bool left; // [esp+95h] [ebp-33h]
    bool horzQualify; // [esp+96h] [ebp-32h]
    bool vertQualify; // [esp+97h] [ebp-31h]
    float focus[2]; // [esp+98h] [ebp-30h]
    float borderTop; // [esp+A0h] [ebp-28h]
    float borderBottom; // [esp+A4h] [ebp-24h]
    ScreenPlacement *scrPlace; // [esp+A8h] [ebp-20h]
    bool clamped; // [esp+AFh] [ebp-19h]
    float halfWidth; // [esp+B0h] [ebp-18h]
    float borderRight; // [esp+B4h] [ebp-14h]
    float pointOriginal[2]; // [esp+B8h] [ebp-10h]
    float halfHeight; // [esp+C0h] [ebp-8h]
    float borderLeft; // [esp+C4h] [ebp-4h]

    if ( !resultDist
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 1327, 0, "%s", "resultDist") )
    {
        __debugbreak();
    }
    scrPlace = &scrPlaceView[localClientNum];
    pointOriginal[0] = *point;
    pointOriginal[1] = point[1];
    halfWidth = scrPlace->realViewportSize[0] * 0.5;
    halfHeight = scrPlace->realViewportSize[1] * 0.5;
    //borderLeft = COERCE_FLOAT(LODWORD(halfWidth) ^ _mask__NegFloat_) + (float)(scrPlace->realViewableMin[0] + padLeft);
    borderLeft = -halfWidth + (float)(scrPlace->realViewableMin[0] + padLeft);
    borderRight = halfWidth - (float)((float)(scrPlace->realViewportSize[0] - scrPlace->realViewableMax[0]) + padRight);
    //borderTop = COERCE_FLOAT(LODWORD(halfHeight) ^ _mask__NegFloat_) + (float)(scrPlace->realViewableMin[1] + padTop);
    borderTop = -halfHeight + (float)(scrPlace->realViewableMin[1] + padTop);
    borderBottom = halfHeight - (float)((float)(scrPlace->realViewportSize[1] - scrPlace->realViewableMax[1]) + padBottom);
    *point = *point - halfWidth;
    point[1] = point[1] - halfHeight;
    clamped = 0;
    if ( borderLeft <= *point )
    {
        if ( *point > borderRight )
        {
            v17 = borderRight / *point;
            *point = v17 * *point;
            point[1] = v17 * point[1];
            clamped = 1;
        }
    }
    else
    {
        v18 = borderLeft / *point;
        *point = v18 * *point;
        point[1] = v18 * point[1];
        clamped = 1;
    }
    if ( borderTop <= point[1] )
    {
        if ( point[1] > borderBottom )
        {
            v15 = borderBottom / point[1];
            *point = v15 * *point;
            point[1] = v15 * point[1];
            clamped = 1;
        }
    }
    else
    {
        v16 = borderTop / point[1];
        *point = v16 * *point;
        point[1] = v16 * point[1];
        clamped = 1;
    }
    if ( waypointOffscreenRoundedCorners->current.enabled )
    {
        radius = (float)((float)(scrPlace->scaleVirtualToReal[0] + scrPlace->scaleVirtualToReal[1])
                                     * waypointOffscreenCornerRadius->current.value)
                     * 0.5;
        left = *point < 0.0;
        top = point[1] < 0.0;
        if ( left )
            v12 = borderLeft + radius;
        else
            v12 = borderRight - radius;
        focus[0] = v12;
        if ( top )
            v11 = borderTop + radius;
        else
            v11 = borderBottom - radius;
        focus[1] = v11;
        if ( waypointDebugDraw->current.enabled )
        {
            CL_DrawStretchPicPhysical(
                (float)(focus[0] + halfWidth) - 1.0,
                (float)(focus[1] + halfHeight) - 1.0,
                3.0,
                3.0,
                0.0,
                0.0,
                1.0,
                1.0,
                colorYellow,
                cgMedia.whiteMaterial);
            CL_DrawStretchPicPhysical(
                (float)(*point + halfWidth) - 1.0,
                (float)(point[1] + halfHeight) - 1.0,
                3.0,
                3.0,
                0.0,
                0.0,
                1.0,
                1.0,
                colorGreen,
                cgMedia.whiteMaterial);
        }
        dir[0] = *point - focus[0];
        dir[1] = point[1] - focus[1];
        Vec2Normalize(dir);
        v10 = left && dir[0] < 0.0 || !left && dir[0] > 0.0;
        horzQualify = v10;
        v9 = top && dir[1] < 0.0 || !top && dir[1] > 0.0;
        vertQualify = v9;
        if ( horzQualify && vertQualify )
        {
            v[0] = *point - focus[0];
            v[1] = point[1] - focus[1];
            dist = Vec2Length(v);
            if ( dist > radius )
            {
                *point = (float)(radius * dir[0]) + focus[0];
                point[1] = (float)(radius * dir[1]) + focus[1];
                clamped = 1;
            }
        }
    }
    *point = *point + halfWidth;
    point[1] = point[1] + halfHeight;
    if ( clamped )
    {
        v13[0] = pointOriginal[0] - *point;
        v13[1] = pointOriginal[1] - point[1];
        *resultDist = Vec2Length(v13);
        *resultNormal = pointOriginal[0] - *point;
        resultNormal[1] = pointOriginal[1] - point[1];
        Vec2Normalize(resultNormal);
    }
    return clamped;
}

void __cdecl CG_Draw2dHudElems(int localClientNum, int foreground)
{
    bool v2; // [esp+0h] [ebp-1038h]
    cg_s *cgameGlob; // [esp+28h] [ebp-1010h]
    bool v4; // [esp+2Fh] [ebp-1009h]
    hudelem_s *elems[1025]; // [esp+30h] [ebp-1008h] BYREF
    int elemCount; // [esp+1034h] [ebp-4h]

    PROF_SCOPED("CG_Draw2dHudElems");

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    elemCount = GetSortedHudElems(localClientNum, elems);
    v4 = cgameGlob->nextSnap->ps.pm_type < 9;
    v2 = cgameGlob->inKillCam || cgameGlob->nextSnap->ps.pm_type == 4;
    if ( (cgameGlob->nextSnap->ps.eFlags2 & 0x10000000) != 0 )
        v4 = 1;
    if ( cgameGlob->cameraMode == 1 )
    {
        return;
    }
    else
    {
        CG_Draw2dHudElemList(elems, elemCount, localClientNum, foreground, v4, v2);
        if ( !localClientNum )
        {
            elemCount = GetSortedDebugHudElems(elems);
            if ( elemCount )
            {
                CG_Draw2dHudElemList(elems, elemCount, 0, foreground, v4, v2);
            }
        }
    }
}

int __cdecl GetSortedHudElems(int localClientNum, hudelem_s **elems)
{
    const cg_s *clientGlob; // [esp+0h] [ebp-Ch]
    playerState_s *ps; // [esp+4h] [ebp-8h]
    int elemCount; // [esp+8h] [ebp-4h] BYREF

    clientGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !clientGlob->nextSnap )
        return 0;
    ps = &clientGlob->nextSnap->ps;
    elemCount = 0;
    CopyInUseHudElems(elems, &elemCount, ps->hud.current, 31);
    CopyInUseHudElems(elems, &elemCount, ps->hud.archival, 31);
    qsort(elems, elemCount, 4u, compare_hudelems);
    return elemCount;
}

void __cdecl CopyInUseHudElems(hudelem_s **elems, int *elemCount, hudelem_s *elemSrcArray, int elemSrcArrayCount)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < elemSrcArrayCount && elemSrcArray[i].type; ++i )
        elems[(*elemCount)++] = &elemSrcArray[i];
}

int __cdecl compare_hudelems(const void *pe0, const void *pe1)
{
    float delta; // [esp+0h] [ebp-Ch]

    delta = *(float *)(*(unsigned int *)pe0 + 56) - *(float *)(*(unsigned int *)pe1 + 56);
    if ( delta >= 0.0 )
        return delta > 0.0;
    else
        return -1;
}

int __cdecl GetSortedDebugHudElems(hudelem_s **elems)
{
    int elemCount; // [esp+0h] [ebp-4h] BYREF

    elemCount = 0;
    CopyInUseDebugHudElems(elems, &elemCount, g_hudelems, 1024);
    qsort(elems, elemCount, 4u, compare_hudelems);
    return elemCount;
}

void __cdecl CopyInUseDebugHudElems(
                hudelem_s **elems,
                int *elemCount,
                game_hudelem_s *elemSrcArray,
                int elemSrcArrayCount)
{
    int i; // [esp+0h] [ebp-4h]

    *elemCount = 0;
    for ( i = 0; i < elemSrcArrayCount; ++i )
    {
        if ( elemSrcArray[i].elem.type )
        {
            if ( (elemSrcArray[i].elem.flags & 0x2000) != 0 )
                elems[(*elemCount)++] = &elemSrcArray[i].elem;
        }
    }
}

void __cdecl CG_Draw2dHudElemList(
                hudelem_s **elems,
                int elemCount,
                int localClientNum,
                int foreground,
                bool playerAlive,
                bool inKillCam)
{
    int i; // [esp+0h] [ebp-8h]
    cg_s *cgameGlob; // [esp+4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    for ( i = 0; i < elemCount; ++i )
    {
        if ( (playerAlive || (elems[i]->flags & 2) == 0)
            && (!foreground || (elems[i]->flags & 1) != 0)
            && (foreground || (elems[i]->flags & 1) == 0)
            && ((elems[i]->flags & 4) == 0 || !UI_AnyMenuVisible(localClientNum))
            && (!inKillCam || (elems[i]->flags & 0x40) == 0)
            && (!CG_IsTvguided(localClientNum, 0) && (cgameGlob->nextSnap->ps.eFlags2 & 0x10000000) == 0
             || (elems[i]->flags & 0x800) == 0)
            && (!Demo_IsPlaying() || (elems[i]->flags & 0x200) == 0 && !Demo_IsGameHudHidden()) )
        {
            DrawSingleHudElem2d(localClientNum, elems[i]);
        }
    }
}

void __cdecl DrawSingleHudElem2d(int localClientNum, const hudelem_s *elem)
{
    const ScreenPlacement *v2; // [esp+4h] [ebp-46Ch]
    char hudElemString[260]; // [esp+18h] [ebp-458h] BYREF
    int ui3dWindow; // [esp+11Ch] [ebp-354h]
    const ScreenPlacement *scrPlace; // [esp+120h] [ebp-350h]
    cg_hudelem_t cghe; // [esp+128h] [ebp-348h] BYREF
    hudelem_color_t toColor; // [esp+36Ch] [ebp-104h] BYREF
    ScreenPlacementStack scrPlaceStack; // [esp+370h] [ebp-100h] BYREF
    ScopedUI3DStack windowIdStack; // [esp+46Ch] [ebp-4h]

    if ( elem->type )
    {
        ui3dWindow = elem->ui3dWindow;
        if ( ui3dWindow < 0 )
            v2 = &scrPlaceView[localClientNum];
        else
            v2 = R_UI3D_ScrPlaceFromTextureWindow(ui3dWindow);
        scrPlace = v2;
        windowIdStack.mStack = R_GetUI3DStack();
        R_UI3DStack_Push(windowIdStack.mStack, ui3dWindow);
        scrPlaceStack.size = 0;
        scrPlaceStack.contextIndex = localClientNum;
        ScopedScrPlaceViewStack scopedScrPlaceStack(&scrPlaceStack, v2);
        //ScopedScrPlaceViewStack::ScopedScrPlaceViewStack(&scopedScrPlaceStack, &scrPlaceStack, v2);
        if ( elem->type == 15 )
        {
            DrawOffscreenViewableWaypoint(localClientNum, elem);
            //ScopedScrPlaceViewStack::~ScopedScrPlaceViewStack(&scopedScrPlaceStack);
            R_UI3DStack_Pop(windowIdStack.mStack);
        }
        else
        {
            cghe.timeNow = CG_GetLocalClientGlobals(localClientNum)->time;
            BG_LerpHudColors(elem, cghe.timeNow, &toColor);
            if ( toColor.a )
            {
                HudElemColorToVec4(&toColor, cghe.color);
                GetHudElemInfo(localClientNum, elem, &cghe, hudElemString);
                if ( cghe.hudElemLabel[0] )
                {
                    DrawHudElemString(localClientNum, &scrPlaceView[localClientNum], cghe.hudElemLabel, elem, &cghe);
                    cghe.x = cghe.x + cghe.labelWidth;
                }
                switch ( elem->type )
                {
                    case 1u:
                    case 2u:
                    case 3u:
                    case 4u:
                    case 5u:
                    case 9u:
                    case 0xAu:
                    case 0xBu:
                    case 0xCu:
                        if ( cghe.hudElemText[0] )
                            DrawHudElemString(localClientNum, &scrPlaceView[localClientNum], cghe.hudElemText, elem, &cghe);
                        break;
                    case 6u:
                        UpdateHudElemWarGameData(localClientNum, elem);
                        break;
                    case 7u:
                        if ( cghe.hudElemText[0] )
                            DrawHudElemString(localClientNum, &scrPlaceView[localClientNum], cghe.hudElemText, elem, &cghe);
                        break;
                    case 8u:
                        DrawHudElemMaterial(localClientNum, elem, &cghe);
                        break;
                    case 0xDu:
                    case 0xEu:
                        DrawHudElemClock(localClientNum, elem, &cghe);
                        break;
                    default:
                        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 1862, 0, "invalid case") )
                            __debugbreak();
                        break;
                }
                //ScopedScrPlaceViewStack::~ScopedScrPlaceViewStack(&scopedScrPlaceStack);
                R_UI3DStack_Pop(windowIdStack.mStack);
            }
            else
            {
                //ScopedScrPlaceViewStack::~ScopedScrPlaceViewStack(&scopedScrPlaceStack);
                R_UI3DStack_Pop(windowIdStack.mStack);
            }
        }
    }
}

ScopedScrPlaceViewStack::ScopedScrPlaceViewStack(ScreenPlacementStack *stack, const ScreenPlacement *newPlacement)
{
    int size; // [esp+Ch] [ebp-4h]

    this->mStack = stack;
    if ( this->mStack )
    {
        size = stack->size;
        if ( size >= 2
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../ui/ui_ui3d_util.h",
                        44,
                        0,
                        "%s",
                        "size < ScreenPlacementStack::MAX_STACK_SIZE") )
        {
            __debugbreak();
        }
        memcpy(&stack->stack[size], &scrPlaceView[stack->contextIndex], sizeof(stack->stack[size]));
        stack->size = size + 1;
        memcpy(&scrPlaceView[stack->contextIndex], newPlacement, sizeof(ScreenPlacement));
    }
    //return this;
}

ScopedScrPlaceViewStack::~ScopedScrPlaceViewStack()
{
    if ( this->mStack )
    {
        if ( this->mStack->size <= 0
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../ui/ui_ui3d_util.h",
                        57,
                        0,
                        "%s",
                        "mStack->size > 0") )
        {
            __debugbreak();
        }
        memcpy(
            &scrPlaceView[this->mStack->contextIndex],
            &this->mStack->stack[--this->mStack->size],
            sizeof(ScreenPlacement));
    }
}

void __cdecl GetHudElemInfo(int localClientNum, const hudelem_s *elem, cg_hudelem_t *cghe, char *hudElemString)
{
    const char *v4; // eax
    char *ConfigString; // eax
    char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    float v9; // [esp+20h] [ebp-1Ch]
    float v10; // [esp+20h] [ebp-1Ch]
    int fontEnum; // [esp+24h] [ebp-18h]
    const ScreenPlacement *scrPlace; // [esp+28h] [ebp-14h]
    float baseFontScale; // [esp+2Ch] [ebp-10h]
    int namedClientIndex; // [esp+34h] [ebp-8h]
    const cg_s *clientGlob; // [esp+38h] [ebp-4h]

    scrPlace = &scrPlaceView[localClientNum];
    switch ( elem->font )
    {
        case 0u:
            goto $LN32_2;
        case 1u:
            baseFontScale = 0.5f;
            fontEnum = 4;
            break;
        case 2u:
            baseFontScale = (1.0f / 3.0f);
            fontEnum = 5;
            break;
        case 3u:
            baseFontScale = 0.25f;
            fontEnum = 1;
            break;
        case 4u:
            baseFontScale = 0.25f;
            fontEnum = 2;
            break;
        case 5u:
            baseFontScale = 0.25f;
            fontEnum = 3;
            break;
        case 6u:
            baseFontScale = 0.25f;
            fontEnum = 6;
            break;
        default:
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 842, 0, "invalid case") )
                __debugbreak();
$LN32_2:
            baseFontScale = 0.25f;
            fontEnum = 0;
            break;
    }
    v9 = elem->fontScale * baseFontScale;
    v10 = ScrPlace_HiResGetScale() * v9;
    cghe->fontScale = v10 * scrPlace->scaleVirtualToReal[1];
    cghe->font = UI_GetFontHandle(scrPlace, fontEnum, cghe->fontScale);
    cghe->fontHeight = (float)UI_TextHeight(cghe->font, cghe->fontScale);
    cghe->hudElemLabel[0] = 0;
    if ( elem->label )
        SafeTranslateHudElemString(localClientNum, elem->label, cghe->hudElemLabel);
    cghe->hudElemText[0] = 0;
    switch ( elem->type )
    {
        case 1u:
            if ( elem->text )
                SafeTranslateHudElemString(localClientNum, elem->text, cghe->hudElemText);
            break;
        case 2u:
        case 7u:
            Com_sprintf(cghe->hudElemText, 0x100u, "%g", elem->value);
            break;
        case 3u:
            namedClientIndex = SnapFloatToInt(elem->value);
            if ( namedClientIndex >= 0 && namedClientIndex < com_maxclients->current.integer )
            {
                clientGlob = CG_GetLocalClientGlobals(localClientNum);
                if ( (unsigned int)namedClientIndex >= 0x20
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp",
                                896,
                                0,
                                "namedClientIndex doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                                namedClientIndex,
                                32) )
                {
                    __debugbreak();
                }
                v4 = BG_DisplayName(&clientGlob->bgs.clientinfo[namedClientIndex], 3);
                I_strncpyz(cghe->hudElemText, v4, 256);
            }
            break;
        case 4u:
            ConfigString = CL_GetConfigString(0x13u);
            I_strncpyz(cghe->hudElemText, ConfigString, 256);
            break;
        case 5u:
            v6 = CL_GetConfigString(0x14u);
            I_strncpyz(cghe->hudElemText, v6, 256);
            break;
        case 9u:
        case 0xAu:
            v7 = HudElemTimerString(elem, cghe->timeNow);
            CopyStringToHudElemString(v7, cghe->hudElemText);
            break;
        case 0xBu:
        case 0xCu:
            v8 = HudElemTenthsTimerString(elem, cghe->timeNow);
            CopyStringToHudElemString(v8, cghe->hudElemText);
            break;
        default:
            break;
    }
    if ( cghe->hudElemLabel[0] && cghe->hudElemText[0] )
        ConsolidateHudElemText(elem, cghe, hudElemString);
    if ( cghe->hudElemLabel[0] )
        cghe->labelWidth = HudElemStringWidth(cghe->hudElemLabel, elem, cghe);
    else
        cghe->labelWidth = 0.0f;
    if ( cghe->hudElemText[0] )
        cghe->textWidth = HudElemStringWidth(cghe->hudElemText, elem, cghe);
    else
        cghe->textWidth = 0.0f;
    cghe->width = HudElemWidth(scrPlace, elem, cghe);
    cghe->height = HudElemHeight(scrPlace, elem, cghe);
    SetHudElemPos(&scrPlaceView[localClientNum], elem, cghe);
}

void __cdecl SafeTranslateHudElemString(int localClientNum, int index, char *hudElemString)
{
    char *ConfigString; // eax

    if ( !hudElemString
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 374, 0, "%s", "hudElemString") )
    {
        __debugbreak();
    }
    if ( index )
    {
        ConfigString = CL_GetConfigString(index + 515);
        CG_TranslateHudElemMessage(localClientNum, ConfigString, "hudelem string", hudElemString);
    }
}

double __cdecl HudElemStringWidth(const char *string, const hudelem_s *elem, const cg_hudelem_t *cghe)
{
    float v4; // [esp+4h] [ebp-10h]
    float v5; // [esp+Ch] [ebp-8h]

    if ( elem->type == 15 )
    {
        v5 = (double)R_TextWidth(cghe->hudElemLabel, 0, cghe->font) * cghe->fontScale;
        return v5 / CL_GetScreenAspectRatioDisplayPixel();
    }
    else
    {
        v4 = (float)UI_TextWidth(string, 0, cghe->font, cghe->fontScale);
        return v4 / CL_GetScreenAspectRatioDisplayPixel();
    }
}

char *__cdecl HudElemTimerString(const hudelem_s *elem, int timeNow)
{
    int HudElemTime; // eax
    int hours; // [esp+0h] [ebp-10h]
    int seconds; // [esp+4h] [ebp-Ch]
    int minutes; // [esp+8h] [ebp-8h]

    HudElemTime = GetHudElemTime(elem, timeNow);
    hours = HudElemTime / 1000 / 3600;
    minutes = HudElemTime / 1000 % 3600 / 60;
    seconds = HudElemTime / 1000 % 3600 % 60;
    if ( hours )
        return va("%i:%02i:%02i", hours, minutes, seconds);
    else
        return va("%i:%02i", minutes, seconds);
}

int __cdecl GetHudElemTime(const hudelem_s *elem, int timeNow)
{
    int result; // eax
    int time; // [esp+4h] [ebp-4h]

    switch ( elem->type )
    {
        case 9u:
            time = elem->time - timeNow + 999;
            goto LABEL_9;
        case 0xAu:
        case 0xCu:
        case 0xEu:
            time = timeNow - elem->time;
            goto LABEL_9;
        case 0xBu:
            time = elem->time - timeNow + 99;
            goto LABEL_9;
        case 0xDu:
            time = elem->time - timeNow;
LABEL_9:
            if ( time < 0 )
                time = 0;
            result = time;
            break;
        default:
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 417, 0, "invalid case") )
                __debugbreak();
            result = 0;
            break;
    }
    return result;
}

char *__cdecl HudElemTenthsTimerString(const hudelem_s *elem, int timeNow)
{
    int HudElemTime; // eax
    int hours; // [esp+0h] [ebp-14h]
    int seconds; // [esp+4h] [ebp-10h]
    int minutes; // [esp+8h] [ebp-Ch]
    int tenths; // [esp+10h] [ebp-4h]

    HudElemTime = GetHudElemTime(elem, timeNow);
    hours = HudElemTime / 100 / 36000;
    minutes = HudElemTime / 100 % 36000 / 600;
    seconds = HudElemTime / 100 % 36000 % 600 / 10;
    tenths = HudElemTime / 100 % 36000 % 600 % 10;
    if ( hours )
        return va("%i:%02i:%02i.%i", hours, minutes, seconds, tenths);
    else
        return va("%i:%02i.%i", minutes, seconds, tenths);
}

double __cdecl HudElemWidth(const ScreenPlacement *scrPlace, const hudelem_s *elem, const cg_hudelem_t *cghe)
{
    double result; // st7

    switch ( elem->type )
    {
        case 1u:
        case 2u:
        case 3u:
        case 4u:
        case 5u:
        case 9u:
        case 0xAu:
        case 0xBu:
        case 0xCu:
            result = cghe->labelWidth + cghe->textWidth;
            break;
        case 6u:
            result = 0.0;
            break;
        case 7u:
            result = cghe->labelWidth + cghe->textWidth;
            break;
        case 8u:
        case 0xDu:
        case 0xEu:
            result = HudElemMaterialWidth(scrPlace, elem, cghe) + cghe->labelWidth;
            break;
        case 0xFu:
            result = (double)elem->width;
            break;
        default:
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 576, 0, "invalid case") )
                __debugbreak();
            result = 0.0;
            break;
    }
    return result;
}

double __cdecl HudElemMaterialWidth(const ScreenPlacement *scrPlace, const hudelem_s *elem, const cg_hudelem_t *cghe)
{
    float width; // [esp+18h] [ebp-10h]
    float lerp; // [esp+1Ch] [ebp-Ch]
    int deltaTime; // [esp+20h] [ebp-8h]
    float fromWidth; // [esp+24h] [ebp-4h]

    width = HudElemMaterialSpecifiedWidth(scrPlace, elem->alignScreen, elem->width, cghe);
    if ( elem->scaleTime <= 0 )
        return width;
    deltaTime = cghe->timeNow - elem->scaleStartTime;
    if ( deltaTime >= elem->scaleTime )
        return width;
    fromWidth = HudElemMaterialSpecifiedWidth(scrPlace, elem->fromAlignScreen, elem->fromWidth, cghe);
    if ( deltaTime <= 0 )
        return fromWidth;
    lerp = (float)deltaTime / (float)elem->scaleTime;
    if ( (lerp < 0.0 || lerp > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp",
                    515,
                    1,
                    "lerp not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
                    lerp,
                    0.0,
                    1.0) )
    {
        __debugbreak();
    }
    return (width - fromWidth) * lerp + fromWidth;
}

double __cdecl HudElemMaterialSpecifiedWidth(
                const ScreenPlacement *scrPlace,
                char alignScreen,
                int sizeVirtual,
                const cg_hudelem_t *cghe)
{
    float v5; // [esp+0h] [ebp-4h]

    if ( !sizeVirtual )
        return cghe->fontHeight;
    if ( (alignScreen & 0xF0) == 0x40 )
        return ScrPlace_HiResGetScale() * ((double)sizeVirtual * scrPlace->scaleVirtualToFull[0]);
    v5 = ScrPlace_HiResGetScale() * (float)sizeVirtual;
    return v5 * scrPlace->scaleVirtualToReal[0];
}

double __cdecl HudElemHeight(const ScreenPlacement *scrPlace, const hudelem_s *elem, const cg_hudelem_t *cghe)
{
    double result; // st7
    float height; // [esp+4h] [ebp-4h]

    switch ( elem->type )
    {
        case 1u:
        case 2u:
        case 3u:
        case 4u:
        case 5u:
        case 9u:
        case 0xAu:
        case 0xBu:
        case 0xCu:
            height = cghe->fontHeight;
            goto LABEL_10;
        case 6u:
            height = 0.0f;
            goto LABEL_10;
        case 7u:
            height = cghe->fontHeight;
            goto LABEL_10;
        case 8u:
        case 0xDu:
        case 0xEu:
            height = HudElemMaterialHeight(scrPlace, elem, cghe);
            goto LABEL_10;
        case 0xFu:
            height = (float)elem->height;
LABEL_10:
            if ( cghe != (const cg_hudelem_t *)-20 && cghe->fontHeight > height )
                height = cghe->fontHeight;
            result = height;
            break;
        default:
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 620, 0, "invalid case") )
                __debugbreak();
            result = 0.0;
            break;
    }
    return result;
}

double __cdecl HudElemMaterialHeight(const ScreenPlacement *scrPlace, const hudelem_s *elem, const cg_hudelem_t *cghe)
{
    float height; // [esp+18h] [ebp-10h]
    float lerp; // [esp+1Ch] [ebp-Ch]
    int deltaTime; // [esp+20h] [ebp-8h]
    float fromHeight; // [esp+24h] [ebp-4h]

    height = HudElemMaterialSpecifiedHeight(scrPlace, elem->alignScreen, elem->height, cghe);
    if ( elem->scaleTime <= 0 )
        return height;
    deltaTime = cghe->timeNow - elem->scaleStartTime;
    if ( deltaTime >= elem->scaleTime )
        return height;
    fromHeight = HudElemMaterialSpecifiedHeight(scrPlace, elem->fromAlignScreen, elem->fromHeight, cghe);
    if ( deltaTime <= 0 )
        return fromHeight;
    lerp = (float)deltaTime / (float)elem->scaleTime;
    if ( (lerp < 0.0 || lerp > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp",
                    540,
                    1,
                    "lerp not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
                    lerp,
                    0.0,
                    1.0) )
    {
        __debugbreak();
    }
    return (height - fromHeight) * lerp + fromHeight;
}

double __cdecl HudElemMaterialSpecifiedHeight(
                const ScreenPlacement *scrPlace,
                char alignScreen,
                int sizeVirtual,
                const cg_hudelem_t *cghe)
{
    float v5; // [esp+0h] [ebp-4h]

    if ( !sizeVirtual )
        return cghe->fontHeight;
    if ( (alignScreen & 0xF) == 4 )
        return ScrPlace_HiResGetScale() * ((double)sizeVirtual * scrPlace->scaleVirtualToFull[1]);
    v5 = ScrPlace_HiResGetScale() * (float)sizeVirtual;
    return v5 * scrPlace->scaleVirtualToReal[1];
}

void __cdecl SetHudElemPos(const ScreenPlacement *scrPlace, const hudelem_s *elem, cg_hudelem_t *cghe)
{
    float from[2]; // [esp+40h] [ebp-14h] BYREF
    float lerp; // [esp+48h] [ebp-Ch]
    float to[2]; // [esp+4Ch] [ebp-8h] BYREF

    if ( !elem && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 722, 0, "%s", "elem") )
        __debugbreak();
    if ( !cghe && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 723, 0, "%s", "cghe") )
        __debugbreak();
    lerp = HudElemMovementFrac(elem, cghe->timeNow);
    if ( lerp == 1.0 )
    {
        GetHudElemOrg(
            scrPlace,
            elem->alignOrg,
            elem->alignScreen,
            elem->x,
            elem->y,
            cghe->width,
            cghe->height,
            &cghe->x,
            &cghe->y);
        cghe->x = (float)SnapFloatToInt(cghe->x);
        cghe->y = (float)SnapFloatToInt(cghe->y);
    }
    else
    {
        GetHudElemOrg(
            scrPlace,
            elem->fromAlignOrg,
            elem->fromAlignScreen,
            elem->fromX,
            elem->fromY,
            cghe->width,
            cghe->height,
            from,
            &from[1]);
        GetHudElemOrg(scrPlace, elem->alignOrg, elem->alignScreen, elem->x, elem->y, cghe->width, cghe->height, to, &to[1]);
        cghe->x = (float)((float)(to[0] - from[0]) * lerp) + from[0];
        cghe->y = (float)((float)(to[1] - from[1]) * lerp) + from[1];
    }
}

void __cdecl GetHudElemOrg(
                const ScreenPlacement *scrPlace,
                int alignOrg,
                int alignScreen,
                float xVirtual,
                float yVirtual,
                float width,
                float height,
                float *orgX,
                float *orgY)
{
    float x; // [esp+18h] [ebp-8h]
    float y; // [esp+1Ch] [ebp-4h]

    if ( !orgX && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 672, 0, "%s", "orgX") )
        __debugbreak();
    if ( !orgY && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 673, 0, "%s", "orgY") )
        __debugbreak();
    x = ScrPlace_ApplyX(scrPlace, xVirtual, (alignScreen >> 4) & 0xF);
    *orgX = AlignHudElemX(alignOrg, x, width);
    y = ScrPlace_ApplyY(scrPlace, yVirtual, alignScreen & 0xF);
    *orgY = AlignHudElemY(alignOrg, y, height);
}

double __cdecl AlignHudElemX(int alignOrg, float x, float width)
{
    unsigned int alignX; // [esp+0h] [ebp-4h]

    alignX = (alignOrg >> 2) & 3;
    if ( alignX > 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp",
                    648,
                    0,
                    "%s\n\t(alignOrg) = %i",
                    "(alignX == 0 || alignX == 1 || alignX == 2)",
                    alignOrg) )
    {
        __debugbreak();
    }
    return x - width * s_alignScale[alignX];
}

double __cdecl AlignHudElemY(int alignOrg, float y, float height)
{
    int alignY; // [esp+0h] [ebp-4h]

    alignY = alignOrg & 3;
    if ( (alignOrg & 3) != 0
        && alignY != 1
        && alignY != 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp",
                    659,
                    0,
                    "%s\n\t(alignOrg) = %i",
                    "(alignY == 0 || alignY == 1 || alignY == 2)",
                    alignOrg) )
    {
        __debugbreak();
    }
    return y - height * s_alignScale[alignY];
}

double __cdecl HudElemMovementFrac(const hudelem_s *elem, int timeNow)
{
    int time; // [esp+4h] [ebp-4h]

    if ( elem->moveTime <= 0 )
        return 1.0;
    time = timeNow - elem->moveStartTime;
    if ( time <= 0 )
        return 0.0;
    if ( time < elem->moveTime )
        return (double)time / (double)elem->moveTime;
    return 1.0;
}

void __cdecl ConsolidateHudElemText(const hudelem_s *elem, cg_hudelem_t *cghe, char *hudElemString)
{
    int len; // [esp+8h] [ebp-Ch]
    int textIndex; // [esp+Ch] [ebp-8h]
    int labelIndex; // [esp+10h] [ebp-4h]

    len = 0;
    for ( labelIndex = 0; len < 255 && cghe->hudElemLabel[labelIndex]; ++labelIndex )
    {
        if ( cghe->hudElemLabel[labelIndex] == '&'
            && cghe->hudElemLabel[labelIndex + 1] == '&'
            && cghe->hudElemLabel[labelIndex + 2] == '1')
        {
            labelIndex += 3;
            break;
        }
        hudElemString[len++] = cghe->hudElemLabel[labelIndex];
    }
    for ( textIndex = 0; len < 255 && cghe->hudElemText[textIndex]; ++textIndex )
        hudElemString[len++] = cghe->hudElemText[textIndex];
    while ( len < 255 && cghe->hudElemLabel[labelIndex] )
        hudElemString[len++] = cghe->hudElemLabel[labelIndex++];
    hudElemString[len] = 0;
    memcpy(cghe->hudElemText, hudElemString, sizeof(cghe->hudElemText));
    cghe->textWidth = HudElemStringWidth(cghe->hudElemText, elem, cghe);
    cghe->hudElemLabel[0] = 0;
    cghe->labelWidth = 0.0f;
}

void __cdecl CopyStringToHudElemString(char *string, char *hudElemString)
{
    signed int v2; // edx
    const char *v3; // eax
    int stringLen; // [esp+10h] [ebp-4h]

    if ( !string && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 808, 0, "%s", "string") )
        __debugbreak();
    if ( !hudElemString
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 809, 0, "%s", "hudElemString") )
    {
        __debugbreak();
    }
    v2 = strlen(string);
    stringLen = v2;
    if ( v2 < 256 )
    {
        memcpy((unsigned __int8 *)hudElemString, (unsigned __int8 *)string, v2);
        hudElemString[stringLen] = 0;
    }
    else
    {
        v3 = va("Hud elem string too long, %s", string);
        Com_Error(ERR_DROP, v3);
    }
}

void __cdecl HudElemColorToVec4(const hudelem_color_t *hudElemColor, float *resultColor)
{
    float value; // [esp+18h] [ebp-4h]

    if ( !hudElemPausedBrightness
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp",
                    951,
                    0,
                    "%s",
                    "hudElemPausedBrightness") )
    {
        __debugbreak();
    }
    if ( !cl_paused
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 952, 0, "%s", "cl_paused") )
    {
        __debugbreak();
    }
    *resultColor = (float)hudElemColor->r * 0.0039215689;
    resultColor[1] = (float)hudElemColor->g * 0.0039215689;
    resultColor[2] = (float)hudElemColor->b * 0.0039215689;
    resultColor[3] = (float)hudElemColor->a * 0.0039215689;
    if ( cl_paused->current.integer )
    {
        value = hudElemPausedBrightness->current.value;
        *resultColor = value * *resultColor;
        resultColor[1] = value * resultColor[1];
        resultColor[2] = value * resultColor[2];
    }
    if ( (*resultColor < 0.0 || *resultColor > 1.000001)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp",
                    962,
                    0,
                    "resultColor[0] not in [0.0f, 1.000001f]\n\t%g not in [%g, %g]",
                    *resultColor,
                    0.0,
                    1.000000953674316) )
    {
        __debugbreak();
    }
    if ( (resultColor[1] < 0.0 || resultColor[1] > 1.000001)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp",
                    963,
                    0,
                    "resultColor[1] not in [0.0f, 1.000001f]\n\t%g not in [%g, %g]",
                    resultColor[1],
                    0.0,
                    1.000000953674316) )
    {
        __debugbreak();
    }
    if ( (resultColor[2] < 0.0 || resultColor[2] > 1.000001)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp",
                    964,
                    0,
                    "resultColor[2] not in [0.0f, 1.000001f]\n\t%g not in [%g, %g]",
                    resultColor[2],
                    0.0,
                    1.000000953674316) )
    {
        __debugbreak();
    }
    if ( (resultColor[3] < 0.0 || resultColor[3] > 1.000001)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp",
                    965,
                    0,
                    "resultColor[3] not in [0.0f, 1.000001f]\n\t%g not in [%g, %g]",
                    resultColor[3],
                    0.0,
                    1.000000953674316) )
    {
        __debugbreak();
    }
}

float glowColor[4];
void __cdecl DrawHudElemString(
                int localClientNum,
                const ScreenPlacement *scrPlace,
                char *text,
                const hudelem_s *elem,
                cg_hudelem_t *cghe)
{
    float v5; // [esp+40h] [ebp-30h]
    float v6; // [esp+40h] [ebp-30h]
    float v7; // [esp+48h] [ebp-28h]
    cg_s *cgameGlob; // [esp+4Ch] [ebp-24h]
    int strLength; // [esp+50h] [ebp-20h]
    float textScale; // [esp+54h] [ebp-1Ch]
    int textStyle; // [esp+58h] [ebp-18h]
    int fxBirthTime; // [esp+5Ch] [ebp-14h]
    float y; // [esp+64h] [ebp-Ch]
    float scaleX; // [esp+68h] [ebp-8h]
    float dy; // [esp+6Ch] [ebp-4h]

    textStyle = 3;
    if ( !text && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 980, 0, "%s", "text") )
        __debugbreak();
    if ( !*text && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 981, 0, "%s", "text[0]") )
        __debugbreak();
    if ( !elem && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 982, 0, "%s", "elem") )
        __debugbreak();
    if ( !cghe && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 983, 0, "%s", "cghe") )
        __debugbreak();
    if ( cghe->color[3] == 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 985, 0, "%s", "cghe->color[3]") )
    {
        __debugbreak();
    }
    //y = OffsetHudElemY(elem, cghe, COERCE_FLOAT(COERCE_UNSIGNED_INT(cghe->height - cghe->fontHeight) ^ _mask__NegFloat_));
    y = OffsetHudElemY(elem, cghe, -(cghe->height - cghe->fontHeight));
    textScale = R_NormalizedTextScale(cghe->font, cghe->fontScale);
    v7 = textScale / ScrPlace_HiResGetScale();
    v5 = v7 * scrPlace->scaleRealToVirtual[1];
    v6 = ScrPlace_HiResGetScale() * v5;
    scaleX = v6 * scrPlace->scaleVirtualToReal[0];
    dy = cghe->fontHeight;
    fxBirthTime = elem->fxBirthTime;
    if ( fxBirthTime && fxBirthTime > cghe->timeNow )
        fxBirthTime = cghe->timeNow;
    HudElemColorToVec4(&elem->glowColor, glowColor);
    if ( elem->type == 11 )
        textStyle = 0;
    if ( fxBirthTime )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        strLength = SEH_PrintStrlen(text);
        CL_PlayTextFXPulseSounds(
            localClientNum,
            cghe->timeNow,
            strLength,
            fxBirthTime,
            elem->fxLetterTime,
            elem->fxDecayStartTime,
            &cgameGlob->hudElemSound[elem->soundID].lastPlayedTime);
    }
    if ( (elem->flags & 0x80) != 0 )
    {
        CL_DrawTextPhysicalWithRedactEffects(
            text,
            0x7FFFFFFF,
            cghe->font,
            cghe->x,
            y + dy,
            scaleX,
            textScale,
            cghe->color,
            textStyle,
            glowColor,
            cgMedia.textDecodeCharacters,
            cgMedia.textDecodeCharactersGlow,
            fxBirthTime,
            elem->fxLetterTime,
            elem->fxDecayStartTime,
            elem->fxDecayDuration,
            elem->fxRedactDecayStartTime,
            elem->fxRedactDecayDuration);
    }
    else if ( (elem->flags & 0x1000) != 0 )
    {
        CL_DrawTextPhysicalWithCOD7DecodeEffects(
            text,
            0x7FFFFFFF,
            cghe->font,
            cghe->x,
            y + dy,
            scaleX,
            textScale,
            cghe->color,
            textStyle,
            glowColor,
            cgMedia.textDecodeCharacters,
            cgMedia.textDecodeCharactersGlow,
            fxBirthTime,
            elem->fxLetterTime,
            elem->fxDecayStartTime,
            elem->fxDecayDuration);
    }
    else
    {
        CL_DrawTextPhysicalWithEffects(
            text,
            0x7FFFFFFF,
            cghe->font,
            cghe->x,
            y + dy,
            1.0,
            scaleX,
            textScale,
            cghe->color,
            textStyle,
            glowColor,
            cgMedia.textDecodeCharacters,
            cgMedia.textDecodeCharactersGlow,
            fxBirthTime,
            elem->fxLetterTime,
            elem->fxDecayStartTime,
            elem->fxDecayDuration);
    }
}

double __cdecl OffsetHudElemY(const hudelem_s *elem, const cg_hudelem_t *cghe, float offsetY)
{
    float from; // [esp+14h] [ebp-Ch]
    float lerp; // [esp+18h] [ebp-8h]
    float to; // [esp+1Ch] [ebp-4h]

    lerp = HudElemMovementFrac(elem, cghe->timeNow);
    if ( lerp == 1.0 )
        return AlignHudElemY(elem->alignOrg, cghe->y, offsetY);
    from = AlignHudElemY(elem->fromAlignOrg, cghe->y, offsetY);
    to = AlignHudElemY(elem->alignOrg, cghe->y, offsetY);
    return (from - to) * lerp + from;
}

void __cdecl DrawHudElemClock(int localClientNum, const hudelem_s *elem, const cg_hudelem_t *cghe)
{
    float width; // [esp+28h] [ebp-64h]
    float height; // [esp+2Ch] [ebp-60h]
    float angle; // [esp+30h] [ebp-5Ch]
    char materialName[68]; // [esp+34h] [ebp-58h] BYREF
    Material *handMaterial; // [esp+7Ch] [ebp-10h]
    int time; // [esp+80h] [ebp-Ch]
    Material *faceMaterial; // [esp+84h] [ebp-8h]
    float y; // [esp+88h] [ebp-4h]

    if ( cghe->color[3] == 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 1043, 0, "%s", "cghe->color[3]") )
    {
        __debugbreak();
    }
    if ( CG_ServerMaterialName(localClientNum, elem->materialIndex, materialName, 0x3Au) )
    {
        faceMaterial = Material_RegisterHandle(materialName, 7);
        I_strncat(materialName, 64, "needle");
        handMaterial = Material_RegisterHandle(materialName, 7);
        time = GetHudElemTime(elem, cghe->timeNow);
        if ( elem->duration )
            angle = AngleNormalize360((float)((float)time * 360.0) / (float)elem->duration);
        else
            angle = AngleNormalize360((float)time * 0.0060000001);
        width = HudElemMaterialWidth(&scrPlaceView[localClientNum], elem, cghe);
        height = HudElemMaterialHeight(&scrPlaceView[localClientNum], elem, cghe);
        //y = OffsetHudElemY(elem, cghe, COERCE_FLOAT(COERCE_UNSIGNED_INT(cghe->height - height) ^ _mask__NegFloat_));
        y = OffsetHudElemY(elem, cghe, -(cghe->height - height));
        CL_DrawStretchPicPhysical(cghe->x, y, width, height, 0.0, 0.0, 1.0, 1.0, cghe->color, faceMaterial);
        CG_DrawRotatedPicPhysical(
            &scrPlaceView[localClientNum],
            cghe->x,
            y,
            width,
            height,
            angle,
            cghe->color,
            handMaterial);
    }
}

void __cdecl DrawHudElemMaterial(int localClientNum, const hudelem_s *elem, cg_hudelem_t *cghe)
{
    Material *material; // [esp+28h] [ebp-5Ch]
    float width; // [esp+2Ch] [ebp-58h]
    float height; // [esp+30h] [ebp-54h]
    char materialName[68]; // [esp+34h] [ebp-50h] BYREF
    float screenPos[2]; // [esp+7Ch] [ebp-8h]

    if ( cghe->color[3] == 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp", 1074, 0, "%s", "cghe->color[3]") )
    {
        __debugbreak();
    }
    if ( CG_ServerMaterialName(localClientNum, elem->materialIndex, materialName, 0x40u) )
    {
        material = Material_RegisterHandle(materialName, 7);
        width = HudElemMaterialWidth(&scrPlaceView[localClientNum], elem, cghe);
        height = HudElemMaterialHeight(&scrPlaceView[localClientNum], elem, cghe);
        screenPos[0] = cghe->x;
        screenPos[1] = OffsetHudElemY(
                                         elem,
                                         cghe,
                                         //COERCE_FLOAT(COERCE_UNSIGNED_INT(cghe->height - height) ^ _mask__NegFloat_));
                                         -(cghe->height - height));
        CL_DrawStretchPicPhysical(screenPos[0], screenPos[1], width, height, 0.0, 0.0, 1.0, 1.0, cghe->color, material);
    }
}

void __cdecl UpdateHudElemWarGameData(int localClientNum, const hudelem_s *elem)
{
    float warDesiredMomentumProgress; // [esp+0h] [ebp-14h]
    cg_s *cgameGlob; // [esp+Ch] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->inKillCam )
    {
        cgameGlob->warCurrentMomentumProgress = cgameGlob->warDesiredMomentumProgress;
    }
    else
    {
        cgameGlob->warDesiredMomentumProgress = elem->value;
        cgameGlob->warMomentumMultiplier = elem->materialIndex;
        cgameGlob->warMomentumBlitzkriegTime = elem->time;
        if ( cgameGlob->warDesiredMomentumProgress <= cgameGlob->warCurrentMomentumProgress )
        {
            cgameGlob->warCurrentMomentumProgress = cgameGlob->warDesiredMomentumProgress;
        }
        else
        {
            if ( (float)(cgameGlob->warDesiredMomentumProgress - (float)(cgameGlob->warCurrentMomentumProgress + 0.0049999999)) < 0.0 )
                warDesiredMomentumProgress = cgameGlob->warDesiredMomentumProgress;
            else
                warDesiredMomentumProgress = cgameGlob->warCurrentMomentumProgress + 0.0049999999;
            cgameGlob->warCurrentMomentumProgress = warDesiredMomentumProgress;
        }
    }
}

void    DrawOffscreenViewableWaypoint(int localClientNum, const hudelem_s *elem)
{
    float v3; // xmm0_4
    float v4; // xmm0_4
    double v5; // st7
    float v6; // [esp+38h] [ebp-494h]
    float v7; // [esp+38h] [ebp-494h]
    float v8; // [esp+40h] [ebp-48Ch]
    float v9; // [esp+40h] [ebp-48Ch]
    int textStyle; // [esp+44h] [ebp-488h]
    float objDist; // [esp+48h] [ebp-484h]
    float delta[4]; // [esp+4Ch] [ebp-480h] BYREF
    cg_s *v13; // [esp+5Ch] [ebp-470h]
    float UNITS_PER_METER; // [esp+60h] [ebp-46Ch]
    float EXTRA_SCALE_FACTOR; // [esp+64h] [ebp-468h]
    cg_hudelem_t cghe; // [esp+68h] [ebp-464h] BYREF
    char text[261]; // [esp+2A8h] [ebp-224h] BYREF
    bool dontDraw; // [esp+3ADh] [ebp-11Fh]
    bool v19; // [esp+3AEh] [ebp-11Eh]
    bool shouldDrawDistanceText; // [esp+3AFh] [ebp-11Dh]
    float screenPosArrow[2]; // [esp+3B0h] [ebp-11Ch] BYREF
    float angle; // [esp+3B8h] [ebp-114h]
    int v23; // [esp+3BCh] [ebp-110h]
    float v24; // [esp+3C0h] [ebp-10Ch]
    float scale; // [esp+3C4h] [ebp-108h]
    float shrinkDist; // [esp+3C8h] [ebp-104h]
    float colorArrow[4]; // [esp+3CCh] [ebp-100h] BYREF
    bool isWaypointOffscreen; // [esp+3DDh] [ebp-EFh]
    bool v29; // [esp+3DEh] [ebp-EEh]
    bool didClamp; // [esp+3DFh] [ebp-EDh]
    float clampedDir[2]; // [esp+3E0h] [ebp-ECh] BYREF
    float clampedDist; // [esp+3E8h] [ebp-E4h] BYREF
    float worldDistance; // [esp+3ECh] [ebp-E0h]
    cg_s *cgameGlob; // [esp+3F0h] [ebp-DCh]
    float screenPos[3]; // [esp+3F4h] [ebp-D8h] BYREF
    centity_s *cent; // [esp+400h] [ebp-CCh]
    cg_s *LocalClientGlobals; // [esp+404h] [ebp-C8h]
    float worldPos[3]; // [esp+408h] [ebp-C4h] BYREF
    float z; // [esp+414h] [ebp-B8h]
    float y; // [esp+418h] [ebp-B4h]
    float x; // [esp+41Ch] [ebp-B0h]
    float tweak2dY; // [esp+420h] [ebp-ACh]
    float padBottom; // [esp+424h] [ebp-A8h]
    float padTop; // [esp+428h] [ebp-A4h]
    float padRight; // [esp+42Ch] [ebp-A0h]
    float padLeft; // [esp+430h] [ebp-9Ch]
    float padding; // [esp+434h] [ebp-98h]
    float distanceThresholdAlpha; // [esp+438h] [ebp-94h]
    float pointerDistance; // [esp+43Ch] [ebp-90h]
    float pointerHeight; // [esp+440h] [ebp-8Ch] BYREF
    float pointerWidth; // [esp+444h] [ebp-88h] BYREF
    float iconHeight; // [esp+448h] [ebp-84h] BYREF
    float iconWidth; // [esp+44Ch] [ebp-80h] BYREF
    float scaleVirtualToRealAvg; // [esp+450h] [ebp-7Ch]
    ScreenPlacement *scrPlace; // [esp+454h] [ebp-78h]
    float color[4]; // [esp+458h] [ebp-74h] BYREF
    Material *material; // [esp+468h] [ebp-64h]
    hudelem_color_t toColor; // [esp+46Ch] [ebp-60h] BYREF
    int time; // [esp+470h] [ebp-5Ch]
    int hasMaterial; // [esp+474h] [ebp-58h]
    char materialName[64]; // [esp+478h] [ebp-54h] BYREF
    //_UNKNOWN *v62; // [esp+4C0h] [ebp-Ch]
    //int localClientNuma; // [esp+4C4h] [ebp-8h]
    //int vars0; // [esp+4CCh] [ebp+0h]
    //
    //v62 = a1;
    //localClientNuma = vars0;

    iassert(elem);
    iassert(elem->type == HE_TYPE_WAYPOINT);

    hasMaterial = CG_ServerMaterialName(localClientNum, elem->offscreenMaterialIdx, materialName, 0x40u);
    if (hasMaterial)
    {
        if (!CG_Flashbanged(localClientNum) && !CG_Flared(localClientNum))
        {
            time = CG_GetLocalClientGlobals(localClientNum)->time;
            BG_LerpHudColors(elem, time, &toColor);
            if (toColor.a)
            {
                if (hasMaterial)
                    material = Material_RegisterHandle(materialName, 7);
                else
                    material = 0;
                HudElemColorToVec4(&toColor, color);
                scrPlace = &scrPlaceView[localClientNum];
                scaleVirtualToRealAvg = (float)(scrPlace->scaleVirtualToReal[0] + scrPlace->scaleVirtualToReal[1]) * 0.5;
                iconWidth = waypointIconWidth->current.value * scrPlace->scaleVirtualToReal[0];
                iconHeight = waypointIconHeight->current.value * scrPlace->scaleVirtualToReal[1];
                pointerWidth = waypointOffscreenPointerWidth->current.value * scrPlace->scaleVirtualToReal[0];
                pointerHeight = waypointOffscreenPointerHeight->current.value * scrPlace->scaleVirtualToReal[1];
                pointerDistance = waypointOffscreenPointerDistance->current.value * scaleVirtualToRealAvg;
                distanceThresholdAlpha = waypointOffscreenDistanceThresholdAlpha->current.value * scaleVirtualToRealAvg;
                if (distanceThresholdAlpha < 0.1)
                    distanceThresholdAlpha = 0.1f;
                padding = (float)(0.5 * pointerHeight) + pointerDistance;
                padLeft = (float)(waypointOffscreenPadLeft->current.value * scrPlace->scaleVirtualToReal[0]) + padding;
                padRight = (float)(waypointOffscreenPadRight->current.value * scrPlace->scaleVirtualToReal[0]) + padding;
                padTop = (float)(waypointOffscreenPadTop->current.value * scrPlace->scaleVirtualToReal[1]) + padding;
                padBottom = (float)(waypointOffscreenPadBottom->current.value * scrPlace->scaleVirtualToReal[1]) + padding;
                tweak2dY = waypointTweakY->current.value * scrPlace->scaleVirtualToReal[1];
                if (elem->targetEntNum == 1023)
                {
                    x = elem->x;
                    y = elem->y;
                    z = elem->z;
                    worldPos[0] = x;
                    worldPos[1] = y;
                    worldPos[2] = z;
                }
                else
                {
                    LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
                    if (elem->targetEntNum == LocalClientGlobals->predictedPlayerEntity.nextState.number)
                        return;
                    cent = CG_GetEntity(localClientNum, elem->targetEntNum);
                    if (((*((_DWORD *)cent + 201) >> 1) & 1) == 0)
                    {
                        Com_PrintWarning(
                            1,
                            "DrawOffscreenViewableWaypoint(): targetEnt %i not in snapshot, may not be a network-broadcasting entity.",
                            elem->targetEntNum);
                        return;
                    }
                    //LODWORD(screenPos[2]) = cent->pose.origin;
                    worldPos[0] = cent->pose.origin[0];
                    worldPos[1] = cent->pose.origin[1];
                    worldPos[2] = cent->pose.origin[2];
                    if (cent->nextState.eType == 1)
                    {
                        if (LocalClientGlobals->bgs.clientinfo[elem->targetEntNum].lastStandStartTime)
                        {
                            worldPos[2] = worldPos[2] + waypointPlayerOffsetRevive->current.value;
                        }
                        else if ((cent->currentState.eFlags & 8) != 0)
                        {
                            worldPos[2] = worldPos[2] + waypointPlayerOffsetProne->current.value;
                        }
                        else
                        {
                            if ((cent->currentState.eFlags & 4) != 0)
                                v3 = worldPos[2] + waypointPlayerOffsetCrouch->current.value;
                            else
                                v3 = worldPos[2] + waypointPlayerOffsetStand->current.value;
                            worldPos[2] = v3;
                        }
                    }
                }
                if (elem->ui3dWindow < 0)
                    WorldPosToScreenPos(localClientNum, worldPos, screenPos);
                else
                    WorldPosToExtraCamScreenPos(localClientNum, worldPos, screenPos);
                screenPos[1] = screenPos[1] + tweak2dY;
                cgameGlob = CG_GetLocalClientGlobals(localClientNum);
                worldDistance = Vec3Distance(cgameGlob->refdef.vieworg, worldPos);
                didClamp = ClampScreenPosToEdges(
                    localClientNum,
                    screenPos,
                    padLeft,
                    padRight,
                    padTop,
                    padBottom,
                    clampedDir,
                    &clampedDist);
                v29 = didClamp && clampedDist > 0.0;
                isWaypointOffscreen = v29;
                if (v29)
                {
                    colorArrow[0] = color[0];
                    colorArrow[1] = color[1];
                    colorArrow[2] = color[2];
                    colorArrow[3] = color[3];
                    if (distanceThresholdAlpha > clampedDist)
                        colorArrow[3] = (float)(clampedDist / distanceThresholdAlpha) * colorArrow[3];
                    shrinkDist = waypointOffscreenScaleLength->current.value * scaleVirtualToRealAvg;
                    if (shrinkDist <= clampedDist)
                    {
                        scale = waypointOffscreenScaleSmallest->current.value;
                    }
                    else
                    {
                        scale = clampedDist / shrinkDist;
                        scale = (float)((float)(1.0 - (float)(clampedDist / shrinkDist)) * 1.0)
                            + (float)(waypointOffscreenScaleSmallest->current.value * (float)(clampedDist / shrinkDist));
                    }
                    iconWidth = iconWidth * scale;
                    iconHeight = iconHeight * scale;
                    v24 = clampedDir[0];
                    v23 = -(clampedDir[1]);
                    //v4 = __libm_sse2_atan2(clampedDir[0]);
                    v4 = atan2(clampedDir[0], -clampedDir[1]);
                    angle = (float)RAD2DEG(v4);
                    screenPosArrow[0] = (float)(pointerDistance * clampedDir[0]) + screenPos[0];
                    screenPosArrow[1] = (float)(pointerDistance * clampedDir[1]) + screenPos[1];
                    ScrPlace_HiResApplyRect(screenPosArrow, &screenPosArrow[1], &pointerWidth, &pointerHeight);
                    CL_DrawStretchPicPhysicalRotateXYW(
                        screenPosArrow[0] - (float)(pointerWidth * 0.5),
                        screenPosArrow[1] - (float)(pointerHeight * 0.5),
                        worldDistance,
                        pointerWidth,
                        pointerHeight,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        angle,
                        colorArrow,
                        cgMedia.offscreenObjectivePointer);
                }
                else if ((elem->flags & 0x100) != 0)
                {
                    WaypointTargetFade(localClientNum, elem, screenPos, color);
                }
                shouldDrawDistanceText = elem->time == 1;
                v19 = shouldDrawDistanceText && isWaypointOffscreen;
                dontDraw = v19;
                scale = GetScaleForDistance(localClientNum, worldPos);
                if (hasMaterial)
                {
                    iconWidth = iconWidth * scale;
                    iconHeight = iconHeight * scale;
                    if (!dontDraw)
                    {
                        ScrPlace_HiResApplyRect(screenPos, &screenPos[1], &iconWidth, &iconHeight);
                        CL_DrawStretchPicPhysicalW(
                            screenPos[0] - (float)(iconWidth * 0.5),
                            screenPos[1] - (float)(iconHeight * 0.5),
                            worldDistance,
                            iconWidth,
                            iconHeight,
                            0.0,
                            0.0,
                            1.0,
                            1.0,
                            color,
                            material);
                    }
                }
                GetHudElemInfo(localClientNum, elem, &cghe, text);
                if (shouldDrawDistanceText && !dontDraw)
                {
                    EXTRA_SCALE_FACTOR = 1.5f;
                    UNITS_PER_METER = 39.369999f;
                    v13 = CG_GetLocalClientGlobals(localClientNum);
                    //LODWORD(delta[3]) = v13->refdef.vieworg;
                    delta[0] = worldPos[0] - v13->refdef.vieworg[0];
                    delta[1] = worldPos[1] - v13->refdef.vieworg[1];
                    delta[2] = worldPos[2] - v13->refdef.vieworg[2];
                    objDist = Vec2Length(delta);
                    if (cghe.hudElemLabel[0])
                        Com_sprintf(cghe.hudElemLabel, 0x100u, "%s %im", cghe.hudElemLabel, (int)(float)(objDist / UNITS_PER_METER));
                    else
                        Com_sprintf(cghe.hudElemLabel, 0x100u, "%im", (int)(float)(objDist / UNITS_PER_METER));
                    v5 = HudElemStringWidth(cghe.hudElemLabel, elem, &cghe);
                    cghe.labelWidth = v5 * EXTRA_SCALE_FACTOR;
                    scale = scale * EXTRA_SCALE_FACTOR;
                }
                if (cghe.hudElemLabel[0] && !dontDraw)
                {
                    if (hasMaterial)
                        screenPos[1] = screenPos[1] - 15.0;
                    textStyle = 0;
                    if ((elem->flags & 8) != 0)
                    {
                        textStyle = 3;
                    }
                    else if ((elem->flags & 0x10) != 0)
                    {
                        textStyle = 6;
                    }
                    if ((elem->flags & 0x20) != 0)
                        HudElemColorToVec4(&elem->glowColor, color);
                    v8 = scale;
                    v9 = v8 / ScrPlace_HiResGetScale();
                    v6 = v9 * scrPlace->scaleRealToVirtual[1];
                    v7 = ScrPlace_HiResGetScale() * v6;
                    CL_DrawTextPhysical(
                        cghe.hudElemLabel,
                        32,
                        cghe.font,
                        screenPos[0] - (float)(cghe.labelWidth * 0.5),
                        screenPos[1],
                        worldDistance,
                        cghe.fontScale * (float)(v7 * scrPlace->scaleVirtualToReal[0]),
                        cghe.fontScale * scale,
                        color,
                        textStyle);
                }
            }
        }
    }
}

double __cdecl GetScaleForDistance(int localClientNum, const float *worldPos)
{
    cg_s *cgameGlob; // [esp+10h] [ebp-Ch]
    float range; // [esp+14h] [ebp-8h]
    float dist3D; // [esp+18h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    dist3D = Vec3Distance(cgameGlob->refdef.vieworg, worldPos);
    if ( waypointDistScaleRangeMin->current.value >= dist3D )
        return 1.0;
    if ( dist3D >= waypointDistScaleRangeMax->current.value )
        return waypointDistScaleSmallest->current.value;
    range = waypointDistScaleRangeMax->current.value - waypointDistScaleRangeMin->current.value;
    if ( range <= 0.0 )
        range = 1.0f;
    return waypointDistScaleSmallest->current.value
             * (float)((float)(dist3D - waypointDistScaleRangeMin->current.value) / range)
             + (1.0 - (float)((float)(dist3D - waypointDistScaleRangeMin->current.value) / range)) * 1.0;
}

void __cdecl WaypointTargetFade(int localClientNum, const hudelem_s *elem, const float *screenPos, float *color)
{
    float v4; // [esp+0h] [ebp-34h]
    float v5; // [esp+4h] [ebp-30h]
    float timeFrac; // [esp+Ch] [ebp-28h]
    cg_s *cgameGlob; // [esp+10h] [ebp-24h]
    float percent; // [esp+14h] [ebp-20h]
    int currentTime; // [esp+18h] [ebp-1Ch]
    float distSq; // [esp+24h] [ebp-10h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( CG_GetPredictedPlayerState(localClientNum)->fWeaponPosFrac >= 1.0 )
    {
        if ( waypointDistFade->current.value > 0.0 )
        {
            v4 = (float)(scrPlaceView[localClientNum].realViewportSize[0] * 0.5) - *screenPos;
            v5 = (float)(scrPlaceView[localClientNum].realViewportSize[1] * 0.5) - screenPos[1];
            distSq = (float)(v4 * v4) + (float)(v5 * v5);
            if ( distSq <= (float)(waypointDistFade->current.value * waypointDistFade->current.value) )
            {
                currentTime = CG_GetLocalClientGlobals(localClientNum)->time;
                if ( !cgameGlob->waypointFadeTime )
                    cgameGlob->waypointFadeTime = currentTime;
                percent = distSq / (float)(waypointDistFade->current.value * waypointDistFade->current.value);
                timeFrac = 0.0f;
                if ( currentTime <= waypointTimeFade->current.integer + cgameGlob->waypointFadeTime
                    && cgameGlob->waypointFadeTime < waypointTimeFade->current.integer + currentTime )
                {
                    timeFrac = WaypointFadeGetTimeFrac(cgameGlob, currentTime);
                }
                if ( timeFrac > percent )
                    percent = timeFrac;
                color[3] = (float)((float)elem->color.a / 255.0) * percent;
            }
        }
    }
    else
    {
        cgameGlob->waypointFadeTime = 0;
    }
}

double __cdecl WaypointFadeGetTimeFrac(const cg_s *cgameGlob, int currentTime)
{
    float v4; // [esp+4h] [ebp-Ch]
    float frac; // [esp+Ch] [ebp-4h]

    if ( waypointTimeFade->current.integer <= 0 )
        return 0.0;
    frac = 1.0 - (float)((float)(currentTime - cgameGlob->waypointFadeTime) / (float)waypointTimeFade->current.integer);
    if ( (float)(frac - 1.0) < 0.0 )
        v4 = 1.0 - (float)((float)(currentTime - cgameGlob->waypointFadeTime) / (float)waypointTimeFade->current.integer);
    else
        v4 = 1.0f;
    if ( (float)(0.0 - frac) < 0.0 )
        return v4;
    else
        return 0.0f;
}

void __cdecl CG_AddDrawSurfsFor3dHudElems(int localClientNum)
{
    int i; // [esp+0h] [ebp-104h]
    hudelem_s *elems[62]; // [esp+4h] [ebp-100h] BYREF
    int elemCount; // [esp+100h] [ebp-4h]
    int savedregs; // [esp+104h] [ebp+0h] BYREF

    if ( !r_highRezWaypoints && CG_ShouldDrawHud(localClientNum) )
    {
        elemCount = GetSortedHudElems(localClientNum, elems);
        if ( (unsigned int)elemCount > 0x3E
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp",
                        2114,
                        0,
                        "%s",
                        "elemCount <= ARRAY_COUNT( elems )") )
        {
            __debugbreak();
        }
        for ( i = 0; i < elemCount; ++i )
        {
            if ( elems[i]->type == 15 )
                AddDrawSurfForHudElemWaypoint(localClientNum, elems[i]);
        }
    }
}

void AddDrawSurfForHudElemWaypoint(int localClientNum, const hudelem_s *elem)
{
    float v2; // xmm0_4
    FxSprite sprite; // [esp+0h] [ebp-90h] BYREF
    float z; // [esp+24h] [ebp-6Ch]
    float y; // [esp+28h] [ebp-68h]
    float x; // [esp+2Ch] [ebp-64h]
    float radius; // [esp+30h] [ebp-60h]
    int renderFxFlags; // [esp+34h] [ebp-5Ch]
    float height; // [esp+38h] [ebp-58h]
    Material *material; // [esp+3Ch] [ebp-54h]
    char materialname[68]; // [esp+40h] [ebp-50h] BYREF
    hudelem_color_t color; // [esp+84h] [ebp-Ch] BYREF
    int time; // [esp+88h] [ebp-8h]

    time = CG_GetLocalClientGlobals(localClientNum)->time;
    BG_LerpHudColors(elem, time, &color);
    if (color.a)
    {
        if (!CG_ServerMaterialName(localClientNum, elem->offscreenMaterialIdx, materialname, 0x40u))
        {
            if (CG_ServerMaterialName(localClientNum, elem->materialIndex, materialname, 0x40u))
            {
                material = Material_RegisterHandle(materialname, 7);
                height = HudElemWaypointHeight(localClientNum, elem);
                if (height != 0.0)
                {
                    if (elem->value <= 0.0)
                    {
                        renderFxFlags = 0;
                        v2 = height;
                    }
                    else
                    {
                        renderFxFlags = 3;
                        v2 = height * 0.0043000001;
                    }
                    radius = v2;
                    x = elem->x;
                    y = elem->y;
                    z = elem->z;
                    sprite.pos[0] = x;
                    sprite.pos[1] = y;
                    sprite.pos[2] = z;
                    *(hudelem_color_t *)sprite.rgbaColor = color;
                    sprite.flags = renderFxFlags;
                    sprite.radius = v2;
                    sprite.minScreenRadius = 0.0f;
                    sprite.material = material;
                    FX_SpriteAdd(&sprite);
                }
            }
        }
    }
}

double __cdecl HudElemWaypointHeight(int localClientNum, const hudelem_s *elem)
{
    float height; // [esp+1Ch] [ebp-10h]
    float lerp; // [esp+20h] [ebp-Ch]
    int deltaTime; // [esp+24h] [ebp-8h]
    float fromHeight; // [esp+28h] [ebp-4h]

    height = (float)elem->height;
    if ( elem->scaleTime <= 0 )
        return height;
    deltaTime = CG_GetLocalClientGlobals(localClientNum)->time - elem->scaleStartTime;
    if ( deltaTime >= elem->scaleTime )
        return height;
    fromHeight = (float)elem->fromHeight;
    if ( deltaTime <= 0 )
        return fromHeight;
    lerp = (float)deltaTime / (float)elem->scaleTime;
    if ( (lerp < 0.0 || lerp > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp",
                    1190,
                    1,
                    "lerp not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
                    lerp,
                    0.0,
                    1.0) )
    {
        __debugbreak();
    }
    return (height - fromHeight) * lerp + fromHeight;
}

void __cdecl RB_DrawWaypoints(int localClientNum)
{
    int i; // [esp+24h] [ebp-104h]
    hudelem_s *elems[62]; // [esp+28h] [ebp-100h] BYREF
    int elemCount; // [esp+124h] [ebp-4h]

    PROF_SCOPED("RB_DrawWaypoints");

    if ( !r_highRezWaypoints )
    {
        return;
    }
    if ( !CG_ShouldDrawHud(localClientNum) )
    {
        return;
    }
    if ( Demo_IsPlaying() && (Demo_IsMovieCamera() || Demo_IsGameHudHidden()) )
    {
        return;
    }
    elemCount = GetSortedHudElems(localClientNum, elems);
    if ( (unsigned int)elemCount > 0x3E
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_hudelem.cpp",
                    2227,
                    0,
                    "%s",
                    "elemCount <= ARRAY_COUNT( elems )") )
    {
        __debugbreak();
    }
    for ( i = 0; i < elemCount; ++i )
    {
        if ( elems[i]->type == 15 )
            RB_AddWaypoint(localClientNum, elems[i]);
    }
}

void __cdecl RB_AddWaypoint(int localClientNum, const hudelem_s *elem)
{
    float y; // [esp+18h] [ebp-94h]
    float z; // [esp+1Ch] [ebp-90h]
    int time; // [esp+2Ch] [ebp-80h]
    centity_s *cent; // [esp+30h] [ebp-7Ch]
    float pos[3]; // [esp+34h] [ebp-78h] BYREF
    Material *material; // [esp+40h] [ebp-6Ch]
    hudelem_color_t hudColor; // [esp+44h] [ebp-68h] BYREF
    const cg_s *cgameGlob; // [esp+48h] [ebp-64h]
    float height; // [esp+4Ch] [ebp-60h]
    float radius; // [esp+50h] [ebp-5Ch]
    char materialName[68]; // [esp+54h] [ebp-58h] BYREF
    bool fixedSize; // [esp+9Eh] [ebp-Eh]
    bool forceNear; // [esp+9Fh] [ebp-Dh]
    int renderFxFlags; // [esp+A0h] [ebp-Ch]
    GfxColor color; // [esp+A4h] [ebp-8h]
    bool alignBottom; // [esp+ABh] [ebp-1h]

    time = CG_GetLocalClientGlobals(localClientNum)->time;
    BG_LerpHudColors(elem, time, &hudColor);
    if ( hudColor.a )
    {
        if ( !CG_ServerMaterialName(localClientNum, elem->offscreenMaterialIdx, materialName, 0x40u) )
        {
            if ( CG_ServerMaterialName(localClientNum, elem->materialIndex, materialName, 0x40u) )
            {
                material = Material_RegisterHandle(materialName, 7);
                height = HudElemWaypointHeight(localClientNum, elem);
                if ( height != 0.0 )
                {
                    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
                    if ( elem->targetEntNum == 1023 )
                    {
                        y = elem->y;
                        z = elem->z;
                        pos[0] = elem->x;
                        pos[1] = y;
                        pos[2] = z;
                    }
                    else
                    {
                        if ( elem->targetEntNum == cgameGlob->predictedPlayerEntity.nextState.number )
                            return;
                        cent = CG_GetEntity(localClientNum, elem->targetEntNum);
                        if ( ((*((unsigned int *)cent + 201) >> 1) & 1) == 0 )
                        {
                            Com_PrintWarning(
                                1,
                                "RB_AddWayPoint(): targetEnt %i not in snapshot, may not be a network-broadcasting entity.\n",
                                elem->targetEntNum);
                            return;
                        }
                        pos[0] = cent->pose.origin[0];
                        pos[1] = cent->pose.origin[1];
                        pos[2] = cent->pose.origin[2];
                        pos[0] = pos[0] + elem->x;
                        pos[1] = pos[1] + elem->y;
                        pos[2] = pos[2] + elem->z;
                    }
                    if ( elem->value <= 0.0 )
                    {
                        renderFxFlags = 0;
                        radius = height;
                    }
                    else
                    {
                        renderFxFlags = 3;
                        radius = height * 0.0043000001;
                        pos[2] = pos[2] + height;
                    }
                    color.packed = hudColor.b | (hudColor.g << 8) | (hudColor.r << 16) | 0xFF000000;
                    fixedSize = (renderFxFlags & 2) != 0;
                    alignBottom = (renderFxFlags & 1) != 0;
                    forceNear = 0;
                    RB_DrawWaypoint(material, color, pos, radius, fixedSize, alignBottom, 0);
                }
            }
        }
    }
}

