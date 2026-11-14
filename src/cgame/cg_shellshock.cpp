#include "cg_shellshock.h"

#include <cgame_mp/cg_local_mp.h>
#include <bgame/bg_pmove.h>
#include <cgame_mp/cg_view_mp.h>
#include <client/splitscreen.h>
#include <cmath>
#include <sound/snd_public_async.h>
#include "cg_sound.h"
#include <client_mp/cl_cgame_mp.h>
#include <sound/snd.h>
#include <demo/demo_playback.h>
#include <qcommon/common.h>

const float cg_perturbations[131][2] =
{
  { -0.56355101, -0.0044300002 },
  { -0.28206199, -0.75793302 },
  { 0.41304699, 0.24424601 },
  { 0.52789497, -0.72389698 },
  { -0.329777, 0.66979998 },
  { -0.39424801, -0.76309001 },
  { 0.12620699, 0.497769 },
  { 0.0049859998, -0.01413 },
  { 0.55991298, 0.112825 },
  { -0.33308899, -0.57328302 },
  { 0.33540401, -0.107176 },
  { -0.56906003, -0.21314099 },
  { -0.166676, 0.78508401 },
  { 0.29959199, 0.037593 },
  { -0.51686698, 0.510759 },
  { 0.138009, 0.034823 },
  { -0.156167, 0.82904798 },
  { -0.99945801, 0.020316999 },
  { 0.30002901, 0.25294399 },
  { 0.030215001, -0.29573199 },
  { -0.91736197, -0.050710998 },
  { 0.044176999, -0.26928899 },
  { 0.58842403, 0.36257699 },
  { -0.379913, 0.619214 },
  { 0.204432, -0.019423001 },
  { 0.018499, 0.468079 },
  { 0.91618699, -0.247878 },
  { 0.0037990001, 0.10821 },
  { 0.057363, 0.60623997 },
  { 0.324595, 0.158733 },
  { -0.130529, -0.18338799 },
  { 0.71567202, -0.36385801 },
  { 0.984258, 0.106096 },
  { -0.0033130001, 0.34553501 },
  { -0.320351, -0.57393599 },
  { 0.063455001, -0.003239 },
  { -0.57017303, -0.75931299 },
  { 0.106456, 0.28372601 },
  { -0.668163, 0.142388 },
  { -0.50111902, -0.72000599 },
  { -0.253281, 0.524032 },
  { -0.064084001, -0.165943 },
  { -0.194672, 0.43355 },
  { -0.2818, -0.41774401 },
  { 0.045786001, 0.40298599 },
  { 0.105064, -0.55893701 },
  { 0.312244, 0.68831801 },
  { -0.26329401, -0.25681099 },
  { 0.65918601, 0.070671998 },
  { 0.093625002, -0.046812002 },
  { -0.87502003, 0.28850901 },
  { 0.32935899, 0.105941 },
  { -0.181309, 0.25986499 },
  { 0.26159701, -0.074069999 },
  { -0.29608199, 0.031858001 },
  { 0.038584001, 0.565947 },
  { -0.253445, -0.71786499 },
  { -0.211836, 0.336521 },
  { 0.89012301, 0.00495 },
  { -0.97982502, -0.17079 },
  { 0.045345999, 0.02224 },
  { -0.34579599, 0.52271199 },
  { 0.108525, 0.165424 },
  { -0.57279599, -0.47339901 },
  { 0.36860499, -0.86584401 },
  { 0.075571001, -0.327703 },
  { -0.466353, -0.56559402 },
  { -0.35883701, 0.61030197 },
  { 0.60388398, 0.44002301 },
  { 0.0024649999, -0.144449 },
  { -0.29491499, 0.79996997 },
  { -0.028347, -0.112071 },
  { -0.0094720004, 0.68606102 },
  { 0.071149997, 0.01991 },
  { 0.96269, 0.024925999 },
  { 0.30920801, 0.87154901 },
  { -0.123782, -0.31230101 },
  { -0.43305501, -0.89598101 },
  { 0.96249503, -0.26377699 },
  { -0.51146001, -0.359478 },
  { -0.044013001, 0.02021 },
  { -0.10934, -0.76122999 },
  { 0.171003, -0.107461 },
  { 0.41891199, 0.435294 },
  { 0.44494, -0.139643 },
  { 0.518574, 0.36596501 },
  { -0.50699699, 0.65559697 },
  { 0.51052499, 0.50896102 },
  { -0.29617301, -0.67583698 },
  { 0.85133201, 0.307192 },
  { -0.0084739998, -0.18874399 },
  { 0.55270302, 0.427086 },
  { 0.080334, -0.0028049999 },
  { 0.035656001, 0.610991 },
  { 0.77059299, 0.39887401 },
  { -0.52213699, 0.32436201 },
  { 0.0060450002, 0.042787999 },
  { 0.482456, 0.84899402 },
  { 0.22605801, -0.522367 },
  { -0.67460603, -0.54781401 },
  { -0.441998, 0.59884 },
  { -0.183957, -0.27023399 },
  { 0.51885003, 0.63494599 },
  { 0.43038601, 0.125257 },
  { -0.185496, -0.26445901 },
  { 0.02369, 0.312978 },
  { -0.444287, 0.84992802 },
  { 0.291978, -0.89767897 },
  { -0.045825999, -0.047127999 },
  { -0.114246, 0.51197499 },
  { 0.73813301, 0.60766703 },
  { -0.78688902, -0.38405699 },
  { 0.18299299, 0.265086 },
  { -0.39945, -0.30903101 },
  { -0.48289499, 0.26566201 },
  { 0.059671, 0.097759999 },
  { 0.79317403, -0.015972 },
  { 0.201658, 0.49244499 },
  { -0.707371, -0.02619 },
  { -0.32088199, 0.37228 },
  { 0.57281297, -0.53725499 },
  { 0.33761901, 0.116293 },
  { -0.60653698, 0.173373 },
  { -0.166593, -0.33511201 },
  { -0.58399302, 0.182916 },
  { -0.57351899, -0.623348 },
  { -0.39270699, 0.44947401 },
  { 0.151474, 0.84040099 },
  { -0.56355101, -0.0044300002 },
  { -0.28206199, -0.75793302 },
  { 0.41304699, 0.24424601 }
};



void __cdecl CG_PerturbCamera(cg_s *cgameGlob)
{
    float rot[3][3]; // [esp+18h] [ebp-48h] BYREF
    float axis[3][3]; // [esp+3Ch] [ebp-24h] BYREF

    if ( cgameGlob->shellshock.viewDelta[0] != 0.0 || cgameGlob->shellshock.viewDelta[1] != 0.0 )
    {
        rot[0][0] = 1.0;
        rot[0][1] = cgameGlob->shellshock.viewDelta[0];
        rot[0][2] = cgameGlob->shellshock.viewDelta[1];
        rot[2][0] = 0.0;
        rot[2][1] = 0.0;
        rot[2][2] = 1.0;
        Vec3Normalize(rot[0]);
        Vec3Cross(rot[2], rot[0], rot[1]);
        Vec3Normalize(rot[1]);
        Vec3Cross(rot[0], rot[1], rot[2]);
        AxisCopy(cgameGlob->refdef.viewaxis, axis);
        MatrixMultiply(rot, axis, cgameGlob->refdef.viewaxis);
    }
}

int __cdecl CG_DrawShellShockSavedScreenBlendBlurred(
                int localClientNum,
                const shellshock_parms_t *parms,
                int start,
                int duration)
{
    int dt; // [esp+24h] [ebp-10h]
    cg_s *cgameGlob; // [esp+28h] [ebp-Ch]
    const ClientViewParams *view; // [esp+2Ch] [ebp-8h]
    int screenBlendTime; // [esp+30h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( start && duration > 0 )
    {
        dt = duration + start - cgameGlob->time;
        if ( dt > 0 )
        {
            screenBlendTime = parms->screenBlend.blurredEffectTime;
            if ( dt < parms->screenBlend.blurredFadeTime )
                screenBlendTime = SnapFloatToInt((float)((float)screenBlendTime
                                                                            * (float)((float)dt / (float)parms->screenBlend.blurredFadeTime)));
            if ( cgameGlob->shellshock.hasSavedScreen )
            {
                view = CG_GetLocalClientViewParams(localClientNum);
                CG_BlendSavedScreenShockBlurred(screenBlendTime, view->x, view->y, view->width, view->height, localClientNum);
            }
            SaveScreenToBuffer(localClientNum);
            cgameGlob->shellshock.hasSavedScreen = 1;
            return 1;
        }
        else
        {
            cgameGlob->shellshock.hasSavedScreen = 0;
            return 0;
        }
    }
    else
    {
        cgameGlob->shellshock.hasSavedScreen = 0;
        return 0;
    }
}

void __cdecl SaveScreenToBuffer(int localClientNum)
{
    const ClientViewParams *view; // [esp+14h] [ebp-8h]

    if ( CL_LocalClient_GetActiveCount() > 1 )
    {
        view = CG_GetLocalClientViewParams(localClientNum);
        CG_SaveScreenSection(view->x, view->y, view->width, view->height, localClientNum);
    }
    else
    {
        CG_SaveScreen(localClientNum);
    }
}

int __cdecl CG_DrawShellShockSavedScreenBlendFlashed(
                int localClientNum,
                const shellshock_parms_t *parms,
                int start,
                int duration)
{
    int dt; // [esp+18h] [ebp-14h]
    cg_s *cgameGlob; // [esp+1Ch] [ebp-10h]
    float whiteFactor; // [esp+20h] [ebp-Ch]
    float whiteFactora; // [esp+20h] [ebp-Ch]
    float whiteFactorb; // [esp+20h] [ebp-Ch]
    const ClientViewParams *view; // [esp+24h] [ebp-8h]
    float grabFactor; // [esp+28h] [ebp-4h]
    float grabFactora; // [esp+28h] [ebp-4h]
    float grabFactorb; // [esp+28h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( start && duration > 0 )
    {
        dt = duration + start - cgameGlob->time;
        if ( dt > 0 )
        {
            whiteFactor = (float)parms->screenBlend.flashWhiteFadeTime;
            grabFactor = (float)parms->screenBlend.flashShotFadeTime;
            if ( whiteFactor <= (float)dt )
                whiteFactora = 1.0f;
            else
                whiteFactora = (float)dt / whiteFactor;
            if ( grabFactor <= (float)dt )
                grabFactora = 1.0f;
            else
                grabFactora = (float)dt / grabFactor;
            whiteFactorb = BlendSmooth(whiteFactora);
            grabFactorb = BlendSmooth(grabFactora);
            if ( cgameGlob->shellshock.hasSavedScreen )
            {
                view = CG_GetLocalClientViewParams(localClientNum);
                CG_BlendSavedScreenShockFlashed(
                    localClientNum,
                    whiteFactorb,
                    grabFactorb,
                    view->x,
                    view->y,
                    view->width,
                    view->height);
            }
            else
            {
                SaveScreenToBuffer(localClientNum);
            }
            cgameGlob->shellshock.hasSavedScreen = 1;
            return 1;
        }
        else
        {
            cgameGlob->shellshock.hasSavedScreen = 0;
            return 0;
        }
    }
    else
    {
        cgameGlob->shellshock.hasSavedScreen = 0;
        return 0;
    }
}

double __cdecl BlendSmooth(float percent)
{
    float sinval; // [esp+10h] [ebp-4h]

    sinval = sin((float)((float)(percent - 0.5) * 3.1415927));
    return (sinval + 1.0f) * 0.5f;
}

void __cdecl EndShellShock(int localClientNum)
{
    EndShellShockSound(localClientNum);
    EndShellShockLookControl(localClientNum);
    EndShellShockCamera(localClientNum);
    EndShellShockScreen(localClientNum);
}

void __cdecl EndShellShockSound(int localClientNum)
{
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    SND_SetSnapshot(SND_SNAPSHOT_SHELLSHOCK, "default", 0.0, 1.0);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->shellshock.loopEndTime )
    {
        cgameGlob->shellshock.loopEndTime = 0;
        CG_PlaySound(localClientNum, 1023, (float*)vec3_origin, 0, false, 1.0, (char*)cgameGlob->shellshock.parms->sound.endAbort);
    }
}

void __cdecl EndShellShockLookControl(int localClientNum)
{
    CG_GetLocalClientGlobals(localClientNum)->shellshock.sensitivity = 1.0f;
    CL_CapTurnRate(localClientNum, 0.0, 0.0);
}

void __cdecl EndShellShockCamera(int localClientNum)
{
    cg_s *cgameGlob; // eax

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgameGlob->shellshock.viewDelta[0] = 0.0f;
    cgameGlob->shellshock.viewDelta[1] = 0.0f;
}

void __cdecl EndShellShockScreen(int localClientNum)
{
    CG_GetLocalClientGlobals(localClientNum)->shellshock.hasSavedScreen = 0;
}

void __cdecl CG_UpdateShellShock(int localClientNum, const shellshock_parms_t *parms, int start, int duration)
{
    bool killcamEntityEnabled; // [esp+7h] [ebp-5h]
    int time; // [esp+8h] [ebp-4h]

    time = CG_GetLocalClientGlobals(localClientNum)->time - start;
    if ( start && time >= 0 )
    {
        killcamEntityEnabled = CG_KillCamEntityEnabled(localClientNum);
        UpdateShellShockSound(localClientNum, parms, time, duration);
        if ( Demo_IsThirdPersonCamera() || Demo_IsMovieCamera() || killcamEntityEnabled )
        {
            EndShellShockLookControl(localClientNum);
            EndShellShockCamera(localClientNum);
            EndShellShockScreen(localClientNum);
        }
        else
        {
            UpdateShellShockLookControl(localClientNum, parms, time, duration);
            UpdateShellShockCamera(localClientNum, parms, time, duration);
            UpdateShellShockVisionSet(localClientNum, parms, time, duration);
        }
    }
    else
    {
        SND_DeactivateEnvironmentEffects(2, 0);
        EndShellShock(localClientNum);
    }
}

void __cdecl UpdateShellShockSound(int localClientNum, const shellshock_parms_t *parms, int time, int duration)
{
    bool shellshockActive; // [esp+1Bh] [ebp-19h]
    int dt; // [esp+1Ch] [ebp-18h]
    cg_s *cgameGlob; // [esp+20h] [ebp-14h]
    SndEntHandle sndEnt; // [esp+24h] [ebp-10h]

    shellshockActive = 0;
    if ( !parms && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_shellshock.cpp", 452, 0, "%s", "parms") )
        __debugbreak();
    if ( time < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_shellshock.cpp", 453, 0, "%s", "time >= 0") )
    {
        __debugbreak();
    }
    if ( duration < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_shellshock.cpp", 454, 0, "%s", "duration >= 0") )
    {
        __debugbreak();
    }
    if ( parms->sound.affect )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        dt = parms->sound.fadeOutTime + parms->sound.modEndDelay + duration - time;
        if ( time >= parms->sound.fadeInTime )
        {
            if ( dt <= parms->sound.fadeOutTime )
            {
                if ( dt >= 0 && dt < parms->sound.fadeOutTime )
                {
                    SND_DeactivateEnvironmentEffects(2, dt);
                    SND_SetSnapshot(SND_SNAPSHOT_SHELLSHOCK, "default", 0.0, 1.0);
                }
            }
            else
            {
                SND_SetEnvironmentEffects(2, parms->sound.roomtype, parms->sound.drylevel, parms->sound.wetlevel, 0);
                SND_SetSnapshot(SND_SNAPSHOT_SHELLSHOCK, parms->sound.snapshot, 0.0, 1.0);
                shellshockActive = 1;
            }
        }
        else
        {
            SND_SetEnvironmentEffects(
                2,
                parms->sound.roomtype,
                parms->sound.drylevel,
                parms->sound.wetlevel,
                parms->sound.fadeInTime - time);
            SND_SetSnapshot(SND_SNAPSHOT_SHELLSHOCK, parms->sound.snapshot, 0.0, 1.0);
            shellshockActive = 1;
        }
        sndEnt.field = CG_SndEntHandle(localClientNum, cgameGlob->clientNum).field;
        if ( cgameGlob->shellshockSoundActive && shellshockActive )
        {
            SND_Play(parms->sound.loop, 0, 1.0, sndEnt, 0, 0, 0);
        }
        else if ( !cgameGlob->shellshockSoundActive && shellshockActive && parms->sound.loop[0] )
        {
            SND_Play(parms->sound.loop, parms->sound.fadeInTime, 1.0, sndEnt, 0, 0, 0);
            cgameGlob->shellshockSoundActive = 1;
        }
        else if ( cgameGlob->shellshockSoundActive )
        {
            if ( !shellshockActive )
            {
                SND_Play(parms->sound.loop, -parms->sound.fadeOutTime, 1.0, sndEnt, 0, 0, 0);
                SND_Play(parms->sound.end, 0, 1.0, sndEnt, 0, 0, 0);
                cgameGlob->shellshockSoundActive = 0;
            }
        }
    }
    else
    {
        SND_DeactivateEnvironmentEffects(2, 0);
    }
}

void __cdecl UpdateShellShockLookControl(int localClientNum, const shellshock_parms_t *parms, int time, int duration)
{
    float fade; // [esp+8h] [ebp-8h]
    cg_s *cgameGlob; // [esp+Ch] [ebp-4h]

    if ( !parms && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_shellshock.cpp", 516, 0, "%s", "parms") )
        __debugbreak();
    if ( time < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_shellshock.cpp", 517, 0, "%s", "time >= 0") )
    {
        __debugbreak();
    }
    if ( duration < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_shellshock.cpp", 518, 0, "%s", "duration >= 0") )
    {
        __debugbreak();
    }
    if ( !parms->lookControl.affect )
        goto LABEL_11;
    if ( duration - time < parms->lookControl.fadeTime )
    {
        if ( duration - time <= 0 )
        {
LABEL_11:
            EndShellShockLookControl(localClientNum);
            return;
        }
        fade = (float)((float)(duration - time) * 1.0) / (float)parms->lookControl.fadeTime;
    }
    else
    {
        fade = 1.0f;
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( fade == 1.0 )
    {
        cgameGlob->shellshock.sensitivity = parms->lookControl.mouseSensitivity;
        CL_CapTurnRate(localClientNum, parms->lookControl.maxPitchSpeed, parms->lookControl.maxYawSpeed);
    }
    else
    {
        cgameGlob->shellshock.sensitivity = (float)((float)(parms->lookControl.mouseSensitivity - 1.0) * fade) + 1.0;
        CL_CapTurnRate(localClientNum, parms->lookControl.maxPitchSpeed / fade, parms->lookControl.maxYawSpeed / fade);
    }
}

void __cdecl UpdateShellShockCamera(int localClientNum, const shellshock_parms_t *parms, int time, int duration)
{
    int dt; // [esp+20h] [ebp-20h]
    float t; // [esp+24h] [ebp-1Ch]
    float ta; // [esp+24h] [ebp-1Ch]
    cg_s *cgameGlob; // [esp+28h] [ebp-18h]
    float radius; // [esp+2Ch] [ebp-14h]
    int base; // [esp+38h] [ebp-8h]
    float scale; // [esp+3Ch] [ebp-4h]

    dt = duration - time;
    if ( duration - time > 0 )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        if ( !parms
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_shellshock.cpp", 581, 0, "%s", "parms") )
        {
            __debugbreak();
        }
        scale = 1.0f;
        if ( dt < parms->view.fadeTime )
            scale = (float)dt / (float)parms->view.fadeTime;
        radius = parms->view.kickRadius * (float)((float)((float)(3.0 - (float)(2.0 * scale)) * scale) * scale);
        t = (float)time * parms->view.kickRate;
        base = (int)(t - 0.4999999990686774);
        ta = t - (float)base;
        {
            unsigned int idx = ((unsigned __int8)base + 61 * (unsigned __int8)duration) & 0x7F;
            cgameGlob->shellshock.viewDelta[0] = CubicInterpolate(
                ta,
                cg_perturbations[idx + 0][0],
                cg_perturbations[idx + 1][0],
                cg_perturbations[idx + 2][0],
                cg_perturbations[idx + 3][0]) * radius;
            cgameGlob->shellshock.viewDelta[1] = CubicInterpolate(
                ta,
                cg_perturbations[idx + 0][1],
                cg_perturbations[idx + 1][1],
                cg_perturbations[idx + 2][1],
                cg_perturbations[idx + 3][1]) * radius;
        }
    }
    else
    {
        EndShellShockCamera(localClientNum);
    }
}

double __cdecl CubicInterpolate(float t, float x0, float x1, float x2, float x3)
{
    return ((t * (float)((float)((float)(x3 - x2) + x1) - x0)
                 + (float)((float)(x0 - x1) - (float)((float)((float)(x3 - x2) + x1) - x0)))
                * t
                + (float)(x2 - x0))
             * t
             + x1;
}

void __cdecl UpdateShellShockVisionSet(int localClientNum, const shellshock_parms_t *parms, int time, int duration)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    if ( !parms && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_shellshock.cpp", 603, 0, "%s", "parms") )
        __debugbreak();
    if ( time < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_shellshock.cpp", 604, 0, "%s", "time >= 0") )
    {
        __debugbreak();
    }
    if ( duration < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_shellshock.cpp", 605, 0, "%s", "duration >= 0") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( parms->visionSet.name[0] )
    {
        if ( time >= duration || cgameGlob->shellshock.visionSetStarted )
        {
            if ( time >= duration && cgameGlob->shellshock.visionSetStarted )
            {
                cgameGlob->shellshock.visionSetStarted = 0;
                CG_VisionSetStartLerp_To(
                    cgameGlob->localClientNum,
                    VISIONSETMODE_NAKED,
                    VISIONSETLERP_TO_SMOOTH,
                    cgameGlob->visionNameNaked,
                    parms->visionSet.outTime);
            }
        }
        else
        {
            cgameGlob->shellshock.visionSetStarted = 1;
            CG_VisionSetStartLerp_To(
                cgameGlob->localClientNum,
                VISIONSETMODE_NAKED,
                VISIONSETLERP_TO_SMOOTH,
                parms->visionSet.name,
                parms->visionSet.inTime);
        }
    }
    else if ( cgameGlob->shellshock.visionSetStarted )
    {
        cgameGlob->shellshock.visionSetStarted = 0;
        CG_VisionSetStartLerp_To(
            cgameGlob->localClientNum,
            VISIONSETMODE_NAKED,
            VISIONSETLERP_TO_SMOOTH,
            cgameGlob->visionNameNaked,
            3000);
    }
}

void __cdecl CG_StartShellShock(cg_s *cgameGlob, const shellshock_parms_t *parms, int start, int duration)
{
    cgameGlob->shellshock.parms = parms;
    cgameGlob->shellshock.startTime = start;
    cgameGlob->shellshock.duration = duration;
}

bool __cdecl CG_Flashbanged(int localClientNum)
{
    const cg_s *cgameGlob; // [esp+4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    return cgameGlob->shellshock.duration + cgameGlob->shellshock.startTime - cgameGlob->time > 0
            && cgameGlob->shellshock.parms->screenBlend.type != SHELLSHOCK_VIEWTYPE_BLURRED;
}

bool __cdecl CG_Flared(int localClientNum)
{
    const cg_s *cgameGlob; // [esp+14h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    return (cgameGlob->snap->ps.perks[1] & 0x400) == 0 && cgameGlob->snap->ps.visionSetLerpRatio >= 0.25;
}

