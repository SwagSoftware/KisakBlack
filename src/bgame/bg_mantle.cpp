#include "bg_mantle.h"
#include "bg_misc.h"
#include "bg_local.h"
#include "bg_perks.h"
#include <qcommon/common.h>
#include "bg_weapons_def.h"
#include <universal/com_math_anglevectors.h>
#include "bg_pmove.h"
#include <cgame/cg_drawtools.h>
#include <cgame_mp/cg_predict_mp.h>


const dvar_t *mantle_enable;
const dvar_t *mantle_debug;
const dvar_t *mantle_check_range;
const dvar_t *mantle_check_radius;
const dvar_t *mantle_check_angle;
const dvar_t *mantle_check_glass_extra_range;
const dvar_t *mantle_view_yawcap;
const dvar_t *mantle_weapon_height;
const dvar_t *mantle_weapon_anim_height;
const dvar_t *vehicle_push_during_mantle;
const dvar_t *prone_bipod_enable;

struct MantleAnimTransition // sizeof=0xC
{                                       // XREF: .data:s_mantleTrans/r
    int upAnimIndex;
    int overAnimIndex;
    float height;                       // XREF: Mantle_RegisterDvars(void)+190/r
};

MantleAnimTransition s_mantleTrans[7] =
{
  { 1, 8, 57.0 },
  { 2, 8, 51.0 },
  { 3, 9, 45.0 },
  { 4, 9, 39.0 },
  { 5, 9, 33.0 },
  { 6, 10, 27.0 },
  { 7, 10, 21.0 }
};

const char *s_mantleAnimNames[11] =
{
  "mp_mantle_root",
  "mp_mantle_up_57",
  "mp_mantle_up_51",
  "mp_mantle_up_45",
  "mp_mantle_up_39",
  "mp_mantle_up_33",
  "mp_mantle_up_27",
  "mp_mantle_up_21",
  "mp_mantle_over_high",
  "mp_mantle_over_mid",
  "player_mantle_over_low"
};

const float turretPronePitchMax = 8.0f;
const float turretTestDist = 16.0f;
const float heightTestDist = 3.0f;
const float dropTraceWidth = 2.0f;
const float turretTestFeetRaise = 8.0f;
const float turretTestFeetDist = -40.0f;

XAnim_s *s_mantleAnims;

void __cdecl Mantle_RegisterDvars()
{
    mantle_enable = _Dvar_RegisterBool("mantle_enable", 1, 0x180u, "Enable player mantling");
    mantle_debug = _Dvar_RegisterBool("mantle_debug", 0, 0x180u, "Show debug information for mantling");
    mantle_check_range = _Dvar_RegisterFloat(
                                                 "mantle_check_range",
                                                 20.0,
                                                 0.0,
                                                 128.0,
                                                 0x180u,
                                                 "The minimum distance from a player to a mantle surface to allow a mantle");
    mantle_check_radius = _Dvar_RegisterFloat(
                                                    "mantle_check_radius",
                                                    0.1,
                                                    0.0,
                                                    15.0,
                                                    0x180u,
                                                    "The player radius to test against while mantling");
    mantle_check_angle = _Dvar_RegisterFloat(
                                                 "mantle_check_angle",
                                                 60.0,
                                                 0.0,
                                                 180.0,
                                                 0x180u,
                                                 "The minimum angle from the player to a mantle surface to allow a mantle");
    mantle_check_glass_extra_range = _Dvar_RegisterFloat(
                                                                         "mantle_check_glass_extra_range",
                                                                         10.0,
                                                                         0.0,
                                                                         32.0,
                                                                         0x180u,
                                                                         "The extra distance to test if glass is ");
    mantle_view_yawcap = _Dvar_RegisterFloat(
                                                 "mantle_view_yawcap",
                                                 60.0,
                                                 0.0,
                                                 180.0,
                                                 0x180u,
                                                 "The angle at which to restrict a sideways turn while mantling");
    mantle_weapon_height = _Dvar_RegisterFloat(
                                                     "mantle_weapon_height",
                                                     39.0,
                                                     0.0,
                                                     s_mantleTrans[0].height,
                                                     0x180u,
                                                     "If the player is mantling at this height or lower, his weapon will remain equipped.");
    mantle_weapon_anim_height = _Dvar_RegisterFloat(
                                                                "mantle_weapon_anim_height",
                                                                32.0,
                                                                0.0,
                                                                s_mantleTrans[0].height,
                                                                0x180u,
                                                                "If the player is mantling at this height or higher, the mantle animation will play.");
    vehicle_push_during_mantle = _Dvar_RegisterBool("vehicle_push_during_mantle", 1, 0x180u, "Enable player mantling");
    prone_bipod_enable = _Dvar_RegisterBool("prone_bipod_enable", 1, 0x180u, "Enable bipods whilst prone");
}

void __cdecl Mantle_CreateAnims(void *(__cdecl *xanimAlloc)(unsigned int))
{
    double Height; // st7
    double v2; // st7
    float v3; // [esp+1Ch] [ebp-2Ch]
    float delta[3]; // [esp+28h] [ebp-20h] BYREF
    int animIndex; // [esp+34h] [ebp-14h]
    int transIndex; // [esp+38h] [ebp-10h]
    float rot[3]; // [esp+3Ch] [ebp-Ch] BYREF

    if ( !s_mantleAnims )
    {
        s_mantleAnims = XAnimCreateAnims("PLAYER_MANTLE", 11, xanimAlloc);
        if ( !s_mantleAnims
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp", 840, 0, "%s", "s_mantleAnims") )
        {
            __debugbreak();
        }
        XAnimBlend(s_mantleAnims, 0, s_mantleAnimNames[0], 1u, 0xAu, 0);
        for ( animIndex = 1; animIndex < 11; ++animIndex )
            BG_CreateXAnim(s_mantleAnims, animIndex, (char *)s_mantleAnimNames[animIndex]);
        for ( transIndex = 0; transIndex < 7; ++transIndex )
        {
            animIndex = Trans_GetUpIndex(transIndex);
            XAnimGetAbsDelta(s_mantleAnims, animIndex, rot, delta, 1.0);
            if ( fabs(delta[0] - 16.0) > 1.0 )
                Com_Error(
                    ERR_DROP,
                    "Mantle anim [%s] has X translation %f, should be %f\n",
                    s_mantleAnimNames[animIndex],
                    delta[0],
                    16.0);
            if ( fabs(delta[1]) > 1.0 )
                Com_Error(
                    ERR_DROP,
                    "Mantle anim [%s] has Y translation %f, should be %f\n",
                    s_mantleAnimNames[animIndex],
                    delta[1],
                    0.0);
            Height = Trans_GetHeight(transIndex);
            v3 = delta[2] - Height;
            if ( fabs(v3) > 1.0 )
            {
                v2 = Trans_GetHeight(transIndex);
                Com_Error(
                    ERR_DROP,
                    "Mantle anim [%s] has Z translation %f, should be %f\n",
                    s_mantleAnimNames[animIndex],
                    delta[2],
                    v2);
            }
        }
    }
}

int __cdecl Trans_GetUpIndex(unsigned int transIndex)
{
    if ( transIndex >= 8
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp",
                    206,
                    0,
                    "transIndex not in [0, MANTLE_UP_COUNT]\n\t%i not in [%i, %i]",
                    transIndex,
                    0,
                    7) )
    {
        __debugbreak();
    }
    return s_mantleTrans[transIndex].upAnimIndex;
}

int __cdecl Trans_GetOverIndex(unsigned int transIndex)
{
    if ( transIndex >= 8
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp",
                    218,
                    0,
                    "transIndex not in [0, MANTLE_UP_COUNT]\n\t%i not in [%i, %i]",
                    transIndex,
                    0,
                    7) )
    {
        __debugbreak();
    }
    return s_mantleTrans[transIndex].overAnimIndex;
}

double __cdecl Trans_GetHeight(unsigned int transIndex)
{
    if ( transIndex >= 8
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp",
                    230,
                    0,
                    "transIndex not in [0, MANTLE_UP_COUNT]\n\t%i not in [%i, %i]",
                    transIndex,
                    0,
                    7) )
    {
        __debugbreak();
    }
    return s_mantleTrans[transIndex].height;
}

void __cdecl Mantle_ShutdownAnims()
{
    s_mantleAnims = 0;
}

void __cdecl Mantle_Check(pmove_t *pm, pml_t *pml)
{
    float mantleDir[3]; // [esp+8h] [ebp-84h] BYREF
    trace_t trace; // [esp+14h] [ebp-78h] BYREF
    MantleResults mresults; // [esp+50h] [ebp-3Ch] BYREF
    playerState_s *ps; // [esp+88h] [ebp-4h]
    int savedregs; // [esp+8Ch] [ebp+0h] BYREF

    Mantle_DebugPrint("====== Mantle Debug ======");
    pm->ps->mantleState.flags &= ~0x100u;
    pm->ps->mountAvailable = 0;
    if ( mantle_enable->current.enabled )
    {
        ps = pm->ps;
        if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp", 996, 0, "%s", "ps") )
            __debugbreak();
        Mantle_ClearHint(ps);
        if ( ps->pm_type < 9 )
        {
            if ( (ps->pm_flags & 4) != 0 )
            {
                Mantle_DebugPrint("Mantle Failed: Player already mantling");
            }
            else if ( (ps->eFlags & 0xC) != 0 )
            {
                if ( (ps->eFlags & 8) != 0 && prone_bipod_enable->current.enabled )
                    Mount_CheckProne(pm);
                Mantle_DebugPrint("Mantle Failed: Player not standing");
            }
            else if ( Mantle_FindMantleSurface(pm, pml, &trace, mantleDir) )
            {
                memset(&mresults, 0, sizeof(mresults));
                mresults.dir[0] = mantleDir[0];
                mresults.dir[1] = mantleDir[1];
                mresults.dir[2] = mantleDir[2];
                mresults.startPos[0] = ps->origin[0];
                mresults.startPos[1] = ps->origin[1];
                mresults.startPos[2] = ps->origin[2];
                if ( (trace.sflags & 0x8000000) != 0 )
                {
                    mresults.flags |= 1u;
                }
                else if ( (trace.sflags & 0x4000000) != 0 )
                {
                    mresults.flags |= 2u;
                }
                if ( (trace.sflags & 0x10000000) != 0 )
                    mresults.flags |= 0x20u;
                if ( !Mantle_CheckLedge(pm, pml, &mresults, 60.0) && !Mantle_CheckLedge(pm, pml, &mresults, 40.0) )
                    Mantle_CheckLedge(pm, pml, &mresults, 20.0);
            }
        }
        else
        {
            Mantle_DebugPrint("Mantle Failed: Player is dead");
        }
    }
    else
    {
        Mantle_DebugPrint("Mantle Failed: Not enabled");
    }
}

void __cdecl Mantle_DebugPrint(const char *msg)
{
    if ( !msg && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp", 192, 0, "%s", "msg") )
        __debugbreak();
    if ( mantle_debug->current.enabled )
        Com_Printf(17, "%s\n", msg);
}

void __cdecl Mount_CheckProne(pmove_t *pm)
{
    float *mountPos; // [esp+0h] [ebp-C4h]
    float *origin; // [esp+4h] [ebp-C0h]
    float proneDirection; // [esp+2Ch] [ebp-98h]
    float lookVec[3]; // [esp+3Ch] [ebp-88h] BYREF
    float dropMins[3]; // [esp+48h] [ebp-7Ch] BYREF
    trace_t trace; // [esp+54h] [ebp-70h] BYREF
    float lookAngles[3]; // [esp+90h] [ebp-34h] BYREF
    float dropMaxs[3]; // [esp+9Ch] [ebp-28h] BYREF
    playerState_s *ps; // [esp+A8h] [ebp-1Ch]
    float traceStart[3]; // [esp+ACh] [ebp-18h] BYREF
    float traceEnd[3]; // [esp+B8h] [ebp-Ch] BYREF

    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp", 543, 0, "%s", "ps") )
        __debugbreak();
    if ( BG_GetWeaponDef(ps->weapon)->mountableWeapon
        && Mount_CanPlayerDeployTurret(ps)
        && turretPronePitchMax > fabs(ps->proneDirectionPitch) )
    {
        traceStart[0] = ps->origin[0];
        traceStart[1] = ps->origin[1];
        traceStart[2] = ps->origin[2];
        proneDirection = ps->proneDirection;
        lookAngles[0] = ps->proneDirectionPitch;
        lookAngles[1] = proneDirection;
        lookAngles[2] = 0.0f;
        AngleVectors(lookAngles, lookVec, 0, 0);
        traceStart[0] = (float)(turretTestDist * lookVec[0]) + traceStart[0];
        traceStart[1] = (float)(turretTestDist * lookVec[1]) + traceStart[1];
        traceStart[2] = (float)(turretTestDist * lookVec[2]) + traceStart[2];
        traceEnd[0] = traceStart[0];
        traceEnd[1] = traceStart[1];
        traceEnd[2] = traceStart[2] - heightTestDist;
        //dropMins[0] = -dropTraceWidth;
        dropMins[0] = -dropTraceWidth;
        //dropMins[1] = -dropTraceWidth;
        dropMins[1] = -dropTraceWidth;
        dropMins[2] = 0.0f;
        dropMaxs[0] = dropTraceWidth;
        dropMaxs[1] = dropTraceWidth;
        dropMaxs[2] = dropTraceWidth * 2.0;
        PM_trace(pm, &trace, traceStart, dropMins, dropMaxs, traceEnd, ps->clientNum, pm->tracemask);
        if ( trace.fraction < 1.0 )
        {
            traceStart[0] = ps->origin[0];
            traceStart[1] = ps->origin[1];
            traceStart[2] = ps->origin[2];
            traceStart[2] = traceStart[2] + turretTestFeetRaise;
            traceEnd[0] = (float)(turretTestFeetDist * lookVec[0]) + traceStart[0];
            traceEnd[1] = (float)(turretTestFeetDist * lookVec[1]) + traceStart[1];
            traceEnd[2] = (float)(turretTestFeetDist * lookVec[2]) + traceStart[2];
            PM_trace(pm, &trace, traceStart, dropMins, dropMaxs, traceEnd, ps->clientNum, pm->tracemask);
            if ( trace.fraction < 1.0
                || (traceStart[0] = traceEnd[0],
                        traceStart[1] = traceEnd[1],
                        traceStart[2] = traceEnd[2],
                        traceEnd[2] = traceEnd[2] - (float)(turretTestDist + turretTestFeetRaise),
                        PM_trace(pm, &trace, traceStart, dropMins, dropMaxs, traceEnd, ps->clientNum, pm->tracemask),
                        trace.fraction < 1.0) )
            {
                ps->mountAvailable = 1;
                ps->cursorHintEntIndex = 1023;
                mountPos = ps->mountPos;
                origin = ps->origin;
                ps->mountPos[0] = ps->origin[0];
                mountPos[1] = origin[1];
                mountPos[2] = origin[2];
                ps->mountDir = ps->viewangles[1];
                ps->mantleState.flags |= 0x480u;
            }
        }
    }
}

char __cdecl Mantle_CheckLedge(pmove_t *pm, pml_t *pml, MantleResults *mresults, float height)
{
    const char *v4; // eax
    float v7; // [esp+8h] [ebp-88h]
    float mins[3]; // [esp+1Ch] [ebp-74h] BYREF
    float start[3]; // [esp+28h] [ebp-68h] BYREF
    float end[3]; // [esp+34h] [ebp-5Ch] BYREF
    trace_t trace; // [esp+40h] [ebp-50h] BYREF
    float maxs[3]; // [esp+7Ch] [ebp-14h] BYREF
    int mask; // [esp+88h] [ebp-8h]
    playerState_s *ps; // [esp+8Ch] [ebp-4h]
    int savedregs; // [esp+90h] [ebp+0h] BYREF

    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp", 712, 0, "%s", "ps") )
        __debugbreak();
    v4 = va("Checking for ledge at %f units", height);
    Mantle_DebugPrint(v4);
    //mins[0] = -playerMaxs[0];
    mins[0] = -playerMaxs[0];
    //mins[1] = -playerMaxs[0];
    mins[1] = -playerMaxs[0];
    mins[2] = 0.0f;
    maxs[0] = playerMaxs[0];
    maxs[1] = playerMaxs[0];
    maxs[2] = 15.0 * 2.0;
    start[0] = mresults->startPos[0];
    start[1] = mresults->startPos[1];
    start[2] = mresults->startPos[2];
    start[2] = start[2] + height;
    end[0] = (float)(16.0 * mresults->dir[0]) + start[0];
    end[1] = (float)(16.0 * mresults->dir[1]) + start[1];
    end[2] = (float)(16.0 * mresults->dir[2]) + start[2];
    mask = pm->tracemask & 0xFDFF7FFF;
    PM_trace(pm, &trace, start, mins, maxs, end, ps->clientNum, mask);
    if ( trace.startsolid || trace.fraction < 1.0 )
    {
        Mantle_DebugPrint("Mantle Failed: Forward movement is blocked");
        return 0;
    }
    else
    {
        start[0] = end[0];
        start[1] = end[1];
        start[2] = end[2];
        end[2] = mresults->startPos[2] + 18.0;
        PM_trace(pm, &trace, start, mins, maxs, end, ps->clientNum, mask);
        if ( trace.startsolid || trace.fraction == 1.0 )
        {
            Mantle_DebugPrint("Mantle Failed: Can't find ledge");
            return 0;
        }
        else if ( trace.walkable )
        {
            mresults->ledgePos[0] = end[0];
            mresults->ledgePos[1] = end[1];
            mresults->ledgePos[2] = end[2];
            mresults->ledgePos[2] = (float)((float)(end[2] - start[2]) * trace.fraction) + start[2];
            if ( (float)(mresults->ledgePos[2] - mresults->startPos[2]) <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp",
                            759,
                            0,
                            "%s\n\t((mresults->ledgePos[2] - mresults->startPos[2])) = %g",
                            "(((mresults->ledgePos[2] - mresults->startPos[2]) > 0.0f))",
                            (float)(mresults->ledgePos[2] - mresults->startPos[2])) )
            {
                __debugbreak();
            }
            maxs[2] = 50.0f;
            PM_trace(pm, &trace, mresults->ledgePos, mins, maxs, mresults->ledgePos, ps->clientNum, pm->tracemask);
            if ( trace.startsolid )
            {
                Mantle_DebugPrint("Mantle Failed: Player can't fit crouched on ledge");
                return 0;
            }
            else
            {
                start[0] = mresults->startPos[0];
                start[1] = mresults->startPos[1];
                start[2] = mresults->startPos[2];
                start[2] = start[2] + height;
                v7 = mantle_check_glass_extra_range->current.value + 16.0;
                end[0] = (float)(v7 * mresults->dir[0]) + start[0];
                end[1] = (float)(v7 * mresults->dir[1]) + start[1];
                end[2] = (float)(v7 * mresults->dir[2]) + start[2];
                maxs[2] = 15.0 * 2.0;
                PM_trace(pm, &trace, start, mins, maxs, end, ps->clientNum, mask);
                if ( trace.startsolid || trace.fraction < 1.0 && trace.hitType == TRACE_HITTYPE_GLASS )
                {
                    Mantle_DebugPrint("Mantle Failed: Glass is no the way");
                    return 0;
                }
                else
                {
                    if ( (ps->mantleState.flags & 0x200) == 0 )
                        Mount_CheckLedge(pm, pml, mresults);
                    if ( (mresults->flags & 1) != 0 || (mresults->flags & 2) != 0 )
                    {
                        ps->mantleState.flags |= 0x10u;
                        mresults->flags |= 0x10u;
                        Mantle_DebugPrint("Mantle available!");
                        Mantle_CalcEndPos(pm, mresults);
                        //if ( bitarray<51>::testBit(&pm->cmd.button_bits, 0xAu) )
                        if ( pm->cmd.button_bits.testBit(10) )
                        {
                            if ( (ps->eFlags & 4) == 0 )
                            {
                                PM_trace(
                                    pm,
                                    &trace,
                                    mresults->ledgePos,
                                    playerMins,
                                    playerMaxs,
                                    mresults->ledgePos,
                                    ps->clientNum,
                                    pm->tracemask);
                                if ( trace.startsolid )
                                    mresults->flags |= 4u;
                                PM_trace(
                                    pm,
                                    &trace,
                                    mresults->endPos,
                                    playerMins,
                                    playerMaxs,
                                    mresults->endPos,
                                    ps->clientNum,
                                    pm->tracemask);
                                if ( !trace.startsolid )
                                    mresults->flags |= 8u;
                            }
                            Mantle_Start(pm, ps, mresults);
                            return 1;
                        }
                        else
                        {
                            return 1;
                        }
                    }
                    else
                    {
                        return 0;
                    }
                }
            }
        }
        else
        {
            Mantle_DebugPrint("Mantle Failed: Ledge is too steep");
            return 0;
        }
    }
}

void __cdecl Mantle_CalcEndPos(pmove_t *pm, MantleResults *mresults)
{
    float mins[3]; // [esp+20h] [ebp-70h] BYREF
    float start[3]; // [esp+2Ch] [ebp-64h] BYREF
    float end[3]; // [esp+38h] [ebp-58h] BYREF
    trace_t trace; // [esp+44h] [ebp-4Ch] BYREF
    float maxs[3]; // [esp+80h] [ebp-10h] BYREF
    playerState_s *ps; // [esp+8Ch] [ebp-4h]

    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp", 385, 0, "%s", "ps") )
        __debugbreak();
    if ( (mresults->flags & 1) != 0 )
    {
        mins[0] = playerMins[0];
        mins[1] = -15.0;
        mins[2] = 0.0;
        maxs[0] = playerMaxs[0];
        maxs[1] = 15.0;
        maxs[2] = 50.0f;
        start[0] = mresults->ledgePos[0];
        start[1] = mresults->ledgePos[1];
        start[2] = mresults->ledgePos[2];
        end[0] = (float)(31.0 * mresults->dir[0]) + start[0];
        end[1] = (float)(31.0 * mresults->dir[1]) + start[1];
        end[2] = (float)(31.0 * mresults->dir[2]) + start[2];
        PM_trace(pm, &trace, start, mins, maxs, end, ps->clientNum, pm->tracemask);
        if ( trace.startsolid || trace.fraction < 1.0 )
        {
            mresults->flags &= 0xFFFFFFFC;
            mresults->endPos[0] = mresults->ledgePos[0];
            mresults->endPos[1] = mresults->ledgePos[1];
            mresults->endPos[2] = mresults->ledgePos[2];
        }
        else
        {
            start[0] = end[0];
            start[1] = end[1];
            start[2] = end[2];
            end[2] = end[2] - 18.0;
            PM_trace(pm, &trace, start, mins, maxs, end, ps->clientNum, pm->tracemask);
            if ( trace.startsolid || trace.fraction < 1.0 )
            {
                mresults->flags &= 0xFFFFFFFC;
                mresults->endPos[0] = mresults->ledgePos[0];
                mresults->endPos[1] = mresults->ledgePos[1];
                mresults->endPos[2] = mresults->ledgePos[2];
            }
            else
            {
                mresults->endPos[0] = end[0];
                mresults->endPos[1] = end[1];
                mresults->endPos[2] = end[2];
                mresults->endPos[2] = (float)((float)(end[2] - start[2]) * trace.fraction) + start[2];
            }
        }
    }
    else
    {
        mresults->endPos[0] = mresults->ledgePos[0];
        mresults->endPos[1] = mresults->ledgePos[1];
        mresults->endPos[2] = mresults->ledgePos[2];
    }
}

void __cdecl Mantle_Start(pmove_t *pm, playerState_s *ps, MantleResults *mresults)
{
    int UpLength; // esi
    float trans[3]; // [esp+2Ch] [ebp-14h] BYREF
    MantleState *mstate; // [esp+38h] [ebp-8h]
    int mantleTime; // [esp+3Ch] [ebp-4h]

    mstate = &ps->mantleState;
    ps->mantleState.yaw = vectoyaw(mresults->dir);
    mstate->timer = 0;
    mstate->transIndex = Mantle_FindTransition(mresults->startPos[2], mresults->ledgePos[2]);
    mstate->flags = mresults->flags;
    UpLength = Mantle_GetUpLength(mstate);
    mantleTime = Mantle_GetOverLength(mstate) + UpLength;
    Mantle_GetAnimDelta(mstate, mantleTime, trans);
    ps->origin[0] = mresults->endPos[0] - trans[0];
    ps->origin[1] = mresults->endPos[1] - trans[1];
    ps->origin[2] = mresults->endPos[2] - trans[2];
    if ( (ps->pm_flags & 4) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp",
                    497,
                    0,
                    "%s",
                    "!( ps->pm_flags & PMF_MANTLE )") )
    {
        __debugbreak();
    }
    ps->pm_flags |= 4u;
    Mantle_SetHaveWeapon(mstate, ps);
    if ( (ps->perks[0] & 0x10000) != 0 )
        mstate->flags |= 0x800u;
    BG_AddPredictableEventToPlayerstate(0x6Du, 0, ps);
    ps->eFlags |= 0x8000u;
    pm->mantleEndPos[0] = mresults->endPos[0];
    pm->mantleEndPos[1] = mresults->endPos[1];
    pm->mantleEndPos[2] = mresults->endPos[2];
    pm->mantleDuration = mantleTime;
    pm->mantleStarted = 1;
}

int __cdecl Mantle_GetUpLength(const MantleState *mstate)
{
    unsigned int animIndex; // [esp+0h] [ebp-8h]
    int time; // [esp+4h] [ebp-4h]

    animIndex = Trans_GetUpIndex(mstate->transIndex);
    time = XAnimGetLengthMsec(s_mantleAnims, animIndex);
    if ( (mstate->flags & 0x800) != 0 )
        return (int)(float)((float)time * perk_mantleReduction->current.value);
    return time;
}

int __cdecl Mantle_GetOverLength(const MantleState *mstate)
{
    unsigned int animIndex; // [esp+0h] [ebp-8h]

    if ( (mstate->flags & 1) == 0 )
        return 0;
    animIndex = Trans_GetOverIndex(mstate->transIndex);
    return XAnimGetLengthMsec(s_mantleAnims, animIndex);
}

void __cdecl Mantle_GetAnimDelta(MantleState *mstate, int time, float *delta)
{
    const char *v3; // eax
    float frac; // [esp+4h] [ebp-24h]
    int upTime; // [esp+8h] [ebp-20h]
    float trans[3]; // [esp+Ch] [ebp-1Ch] BYREF
    int animIndex; // [esp+18h] [ebp-10h]
    float rot[2]; // [esp+1Ch] [ebp-Ch] BYREF
    int overTime; // [esp+24h] [ebp-4h]

    upTime = Mantle_GetUpLength(mstate);
    overTime = Mantle_GetOverLength(mstate);
    if ( time < 0 || time > overTime + upTime )
    {
        v3 = va("time: %i, upTime: %i, overTime: %i", time, upTime, overTime);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp",
                        315,
                        0,
                        "%s\n\t%s",
                        "(time >= 0) && (time <= (upTime + overTime))",
                        v3) )
            __debugbreak();
    }
    if ( time > upTime )
    {
        frac = (float)(time - upTime) / (float)overTime;
        animIndex = Trans_GetUpIndex(mstate->transIndex);
        XAnimGetAbsDelta(s_mantleAnims, animIndex, rot, trans, 1.0);
        animIndex = Trans_GetOverIndex(mstate->transIndex);
        XAnimGetAbsDelta(s_mantleAnims, animIndex, rot, delta, frac);
        *delta = *delta + trans[0];
        delta[1] = delta[1] + trans[1];
        delta[2] = delta[2] + trans[2];
    }
    else
    {
        animIndex = Trans_GetUpIndex(mstate->transIndex);
        XAnimGetAbsDelta(s_mantleAnims, animIndex, rot, delta, (float)time / (float)upTime);
    }
    VectorAngleMultiply(delta, mstate->yaw);
}

int __cdecl Mantle_FindTransition(float curHeight, float goalHeight)
{
    float v3; // [esp+8h] [ebp-1Ch]
    float v4; // [esp+Ch] [ebp-18h]
    float height; // [esp+14h] [ebp-10h]
    int bestIndex; // [esp+18h] [ebp-Ch]
    signed int transIndex; // [esp+1Ch] [ebp-8h]
    float bestDiff; // [esp+20h] [ebp-4h]

    height = goalHeight - curHeight;
    if ( (float)(goalHeight - curHeight) <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp",
                    353,
                    0,
                    "%s\n\t(height) = %g",
                    "((height > 0))",
                    height) )
    {
        __debugbreak();
    }
    bestIndex = 0;
    v4 = Trans_GetHeight(0) - height;
    bestDiff = fabs(v4);
    for ( transIndex = 1; transIndex < 7; ++transIndex )
    {
        v3 = Trans_GetHeight(transIndex) - height;
        if ( bestDiff > fabs(v3) )
        {
            bestIndex = transIndex;
            bestDiff = fabs(v3);
        }
    }
    return bestIndex;
}

void __cdecl Mantle_SetHaveWeapon(MantleState *mstate, const playerState_s *ps)
{
    BG_GetWeaponDef(ps->weapon);
    Trans_GetHeight(mstate->transIndex);
}

// local variable allocation has failed, the output may be wrong!
void Mount_CheckLedge(pmove_t *pm, pml_t *pml, MantleResults *mresults)
{
    float angle; // [esp+4h] [ebp-108h]
    long double v5; // [esp+10h] [ebp-FCh]
    float color[6]; // [esp+18h] [ebp-F4h] BYREF
    float dropMaxs[3]; // [esp+30h] [ebp-DCh] BYREF
    float v8[3]; // [esp+3Ch] [ebp-D0h] BYREF
    float dropMins[3]; // [esp+48h] [ebp-C4h] BYREF
    float *traceEnd; // [esp+54h] [ebp-B8h]
    float v11; // [esp+58h] [ebp-B4h]
    float traceStart[3]; // [esp+5Ch] [ebp-B0h] BYREF
    float v13[3]; // [esp+68h] [ebp-A4h] BYREF
    float ledgeAngles; // [esp+74h] [ebp-98h]
    float ledgeVec[3]; // [esp+78h] [ebp-94h] BYREF
    float *ledgePos; // [esp+84h] [ebp-88h]
    float *origin; // [esp+88h] [ebp-84h]
    float lookVec[3]; // [esp+8Ch] [ebp-80h] BYREF
    float v19; // [esp+98h] [ebp-74h]
    float v20; // [esp+9Ch] [ebp-70h]
    float up[3]; // [esp+A0h] [ebp-6Ch]
    float *velocity; // [esp+ACh] [ebp-60h]
    bool drawLines; // [esp+B3h] [ebp-59h]
    int drawTime; // [esp+B4h] [ebp-58h]
    playerState_s *ps; // [esp+B8h] [ebp-54h]
    float heightTestDist; // [esp+BCh] [ebp-50h]
    float dropTraceWidth; // [esp+C0h] [ebp-4Ch]
    trace_t trace; // [esp+C4h] [ebp-48h] BYREF
    //cStaticModel_s *v29; // [esp+100h] [ebp-Ch]
    //pmove_t *pm_a; // [esp+104h] [ebp-8h]
    //void *ret; // [esp+10Ch] [ebp+0h]

    //v29 = a1;
    //pm_a = (pmove_t *)ret;
    dropTraceWidth = 2.0f;
    heightTestDist = 6.0f;
    ps = pm->ps;
    if (!ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp", 616, 0, "%s", "ps"))
        __debugbreak();
    drawTime = 100;
    drawLines = cg_debugMounting->current.integer > 2;
    velocity = ps->velocity;
    if ((float)((float)((float)(ps->velocity[0] * ps->velocity[0]) + (float)(ps->velocity[1] * ps->velocity[1]))
        + (float)(ps->velocity[2] * ps->velocity[2])) > 0.0)
        drawLines = 0;
    up[0] = 0.0f;
    up[1] = 0.0f;
    up[2] = 1.0f;
    v20 = mresults->ledgePos[2] - ps->origin[2];
    //LODWORD(v19) = LODWORD(v20) & _mask__AbsFloat_;
    v19 = fabsf(v20);
    if (BG_GetWeaponDef(ps->weapon)->mountableWeapon
        && (mresults->flags & 0x20) != 0
        && v19 < 50.0
        && Mount_CanPlayerDeployTurret(ps))
    {
        lookVec[0] = 0.0f;
        lookVec[1] = 1.0f;
        lookVec[2] = 0.0f;
        angle = AngleNormalize360(ps->viewangles[1]);
        YawVectors(angle, lookVec, 0);
        origin = ps->origin;
        ledgePos = mresults->ledgePos;
        ledgeVec[0] = mresults->ledgePos[0] - ps->origin[0];
        ledgeVec[1] = mresults->ledgePos[1] - ps->origin[1];
        ledgeVec[2] = 0.0f;
        Vec3Normalize(ledgeVec);
        ledgeAngles = (float)((float)(lookVec[0] * ledgeVec[0]) + (float)(lookVec[1] * ledgeVec[1]))
            + (float)(lookVec[2] * ledgeVec[2]);
        //__libm_sse2_acos(v5);
        ledgeAngles = acosf(ledgeAngles);
        if ((float)(ledgeAngles * 57.295776) < 30.0)
        {
            vectoangles(mresults->dir, v13);
            YawVectors(v13[1] + 180.0, traceStart, 0);
            v11 = 36.0f;
            traceEnd = mresults->ledgePos;
            traceStart[0] = (float)(36.0 * traceStart[0]) + mresults->ledgePos[0];
            traceStart[1] = (float)(36.0 * traceStart[1]) + mresults->ledgePos[1];
            traceStart[2] = (float)(36.0 * traceStart[2]) + mresults->ledgePos[2];
            traceStart[2] = ps->origin[2];
            dropMins[0] = traceStart[0];
            dropMins[1] = traceStart[1];
            dropMins[2] = traceStart[2];
            traceStart[2] = traceStart[2] + heightTestDist;
            dropMins[2] = dropMins[2] - heightTestDist;
            //LODWORD(v8[0]) = LODWORD(dropTraceWidth) ^ _mask__NegFloat_;
            v8[0] = -dropTraceWidth;
            //LODWORD(v8[1]) = LODWORD(dropTraceWidth) ^ _mask__NegFloat_;
            v8[1] = -dropTraceWidth;
            v8[2] = 0.0f;
            dropMaxs[0] = dropTraceWidth;
            dropMaxs[1] = dropTraceWidth;
            dropMaxs[2] = dropTraceWidth * 2.0;
            PM_trace(pm, &trace, traceStart, v8, dropMaxs, dropMins, ps->clientNum, pm->tracemask);
            if (trace.fraction >= 1.0)
            {
                if (drawLines)
                {
                    color[0] = 1.0f;
                    color[1] = 0.0f;
                    color[2] = 0.0f;
                    color[3] = 1.0f;
                    CG_DebugBox(dropMins, v8, dropMaxs, 0.0, color, 0, 100);
                }
            }
            else
            {
                ps->mountAvailable = 1;
                ps->cursorHintEntIndex = 1023;
                //LODWORD(color[5]) = ps->mountPos;
                //LODWORD(color[4]) = mresults->ledgePos;
                ps->mountPos[0] = mresults->ledgePos[0];
                //*(float *)(LODWORD(color[5]) + 4) = *(float *)(LODWORD(color[4]) + 4);
                ps->mountPos[1] = mresults->ledgePos[1];
                //*(float *)(LODWORD(color[5]) + 8) = *(float *)(LODWORD(color[4]) + 8);
                ps->mountPos[2] = mresults->ledgePos[2];
                ps->mountDir = v13[1];
                ps->mantleState.flags |= 0x80u;
                ps->mantleState.flags &= ~0x400u;
                mresults->flags |= 0x80u;
                if (drawLines)
                {
                    color[0] = 0.0f;
                    color[1] = 1.0f;
                    color[2] = 0.0f;
                    color[3] = 1.0f;
                    CG_DebugBox(dropMins, v8, dropMaxs, 0.0, color, 0, 100);
                }
            }
        }
    }
}

char    Mantle_FindMantleSurface(pmove_t *pm, pml_t *pml, trace_t *trace, float *mantleDir)
{
    float len; // [esp-8h] [ebp-80h]
    float end[4]; // [esp-4h] [ebp-7Ch] BYREF
    float start[5]; // [esp+Ch] [ebp-6Ch] BYREF
    float traceDir[3]; // [esp+20h] [ebp-58h] BYREF
    float traceDist; // [esp+2Ch] [ebp-4Ch]
    float innerDist; // [esp+30h] [ebp-48h]
    float maxs[3]; // [esp+34h] [ebp-44h] BYREF
    int integer; // [esp+44h] [ebp-34h]
    float mins[3]; // [esp+4Ch] [ebp-2Ch] BYREF
    float playerRadius; // [esp+64h] [ebp-14h]
    playerState_s *ps; // [esp+68h] [ebp-10h]

    ps = pm->ps;

    iassert(ps);

    playerRadius = playerMaxs[0];
    mins[0] = -mantle_check_radius->current.value;
    mins[1] = -mantle_check_radius->current.value;
    mins[2] = 0.0;

    maxs[0] = mantle_check_radius->current.value;
    maxs[1] = mantle_check_radius->current.value;
    maxs[2] = 70.0;

    iassert((maxs[0] - mins[0]) <= (playerMaxs[2] - playerMins[2]));
    iassert((maxs[1] - mins[1]) <= (playerMaxs[2] - playerMins[2]));

    innerDist = playerRadius - mantle_check_radius->current.value;
    traceDist = mantle_check_range->current.value + innerDist;

    traceDir[0] = pml->forward[0];
    traceDir[1] = pml->forward[1];
    traceDir[2] = 0.0f;

    Vec3Normalize(traceDir);

    Vec3Mad(ps->origin, -innerDist, traceDir, start);
    Vec3Mad(ps->origin, traceDist, traceDir, end);

    PM_trace(pm, trace, start, mins, maxs, end, ps->clientNum, 0x1000000);
    if (trace->startsolid || trace->allsolid)
    {
        Mantle_DebugPrint("Mantle Failed: Mantle brush is too thick");
        return 0;
    }
    else if (trace->fraction == 1.0)
    {
        Mantle_DebugPrint("Mantle Failed: No mantle surface found");
        return 0;
    }
    else if ((trace->sflags & 0x1C000000) != 0)
    {
        mantleDir[0] = -trace->normal.vec.v[0];
        mantleDir[1] = -trace->normal.vec.v[1];
        mantleDir[2] = -trace->normal.vec.v[2];
        mantleDir[2] = 0.0f;
        len = Vec3Normalize(mantleDir);
        if (len >= 0.000099999997)
        {
            float dot = Vec3Dot(traceDir, mantleDir);
            if ((float)(acos(dot) * 57.295776) <= mantle_check_angle->current.value)
            {
                return 1;
            }
            else
            {
                Mantle_DebugPrint("Mantle Failed: Player is not facing mantle surface");
                return 0;
            }
        }
        else
        {
            Mantle_DebugPrint("Mantle Failed: Mantle surface has vertical normal");
            return 0;
        }
    }
    else
    {
        Mantle_DebugPrint("Mantle Failed: No mantle surface with MANTLE_ON, MANTLE_OVER or MOUNT found");
        return 0;
    }
}

void __cdecl Mantle_Move(pmove_t *pm, playerState_s *ps, pml_t *pml)
{
    int UpLength; // esi
    float v4; // [esp+8h] [ebp-38h]
    float trans[3]; // [esp+14h] [ebp-2Ch] BYREF
    float prevTrans[3]; // [esp+20h] [ebp-20h] BYREF
    int prevTime; // [esp+2Ch] [ebp-14h]
    MantleState *mstate; // [esp+30h] [ebp-10h]
    int animIndex; // [esp+34h] [ebp-Ch]
    int deltaTime; // [esp+38h] [ebp-8h]
    int mantleLength; // [esp+3Ch] [ebp-4h]

    if ( (ps->pm_flags & 4) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp",
                    1070,
                    0,
                    "%s",
                    "ps->pm_flags & PMF_MANTLE") )
    {
        __debugbreak();
    }
    if ( pml->msec < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp",
                    1071,
                    0,
                    "%s\n\t(pml->msec) = %i",
                    "(pml->msec >= 0)",
                    pml->msec) )
    {
        __debugbreak();
    }
    if ( mantle_enable->current.enabled )
    {
        mstate = &ps->mantleState;
        ps->mantleState.flags &= ~0x10u;
        if ( (mstate->flags & 4) != 0 )
            BG_AddPredictableEventToPlayerstate(9u, 0, ps);
        UpLength = Mantle_GetUpLength(mstate);
        mantleLength = Mantle_GetOverLength(mstate) + UpLength;
        prevTime = mstate->timer;
        mstate->timer += pml->msec;
        if ( mstate->timer > mantleLength )
            mstate->timer = mantleLength;
        deltaTime = mstate->timer - prevTime;
        Mantle_GetAnimDelta(mstate, prevTime, prevTrans);
        Mantle_GetAnimDelta(mstate, mstate->timer, trans);
        animIndex = Mantle_GetAnim(mstate);
        BG_AnimScriptAnimation(pm, AISTATE_COMBAT, (scriptAnimMoveTypes_t)(animIndex + 8), 1);
        trans[0] = trans[0] - prevTrans[0];
        trans[1] = trans[1] - prevTrans[1];
        trans[2] = trans[2] - prevTrans[2];
        ps->origin[0] = trans[0] + ps->origin[0];
        ps->origin[1] = trans[1] + ps->origin[1];
        ps->origin[2] = trans[2] + ps->origin[2];
        v4 = 1.0 / (float)((float)deltaTime * 0.001);
        ps->velocity[0] = v4 * trans[0];
        ps->velocity[1] = v4 * trans[1];
        ps->velocity[2] = v4 * trans[2];
        if ( mstate->timer == mantleLength )
        {
            if ( (ps->pm_flags & 4) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp",
                            1109,
                            0,
                            "%s",
                            "( ps->pm_flags & PMF_MANTLE )") )
            {
                __debugbreak();
            }
            ps->pm_flags &= ~4u;
            pm->mantleStarted = 0;
            if ( (mstate->flags & 1) != 0 )
                BG_AnimScriptEvent(pm, ANIM_ET_JUMP, 0, 1);
            if ( (mstate->flags & 8) != 0 )
            {
                BG_AddPredictableEventToPlayerstate(8u, 0, ps);
                ps->eFlags &= ~0x8000u;
            }
            mstate->flags |= 0x100u;
        }
    }
}

int __cdecl Mantle_GetAnim(MantleState *mstate)
{
    int upTime; // [esp+0h] [ebp-Ch]
    int overTime; // [esp+8h] [ebp-4h]

    upTime = Mantle_GetUpLength(mstate);
    overTime = Mantle_GetOverLength(mstate);
    if ( (mstate->timer < 0 || mstate->timer > overTime + upTime)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp",
                    289,
                    0,
                    "%s",
                    "(mstate->timer >= 0) && (mstate->timer <= (upTime + overTime))") )
    {
        __debugbreak();
    }
    if ( mstate->timer > upTime )
        return Trans_GetOverIndex(mstate->transIndex);
    else
        return Trans_GetUpIndex(mstate->transIndex);
}

void __cdecl Mantle_CapView(playerState_s *ps)
{
    float value; // [esp+8h] [ebp-Ch]
    float delta; // [esp+Ch] [ebp-8h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp", 1144, 0, "%s", "ps") )
        __debugbreak();
    if ( (ps->pm_flags & 4) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp",
                    1145,
                    0,
                    "%s",
                    "ps->pm_flags & PMF_MANTLE") )
    {
        __debugbreak();
    }
    if ( mantle_enable->current.enabled )
    {
        delta = AngleDelta(ps->mantleState.yaw, ps->viewangles[1]);
        //if ( delta < COERCE_FLOAT(mantle_view_yawcap->current.integer ^ _mask__NegFloat_)
        if ( delta < -mantle_view_yawcap->current.value
            || mantle_view_yawcap->current.value < delta )
        {
            //while ( COERCE_FLOAT(mantle_view_yawcap->current.integer ^ _mask__NegFloat_) > delta )
            while ( -mantle_view_yawcap->current.value > delta )
                delta = delta + mantle_view_yawcap->current.value;
            while ( delta > mantle_view_yawcap->current.value )
                delta = delta - mantle_view_yawcap->current.value;
            if ( delta <= 0.0 )
                value = mantle_view_yawcap->current.value;
            else
            {
                //LODWORD(value) = mantle_view_yawcap->current.integer ^ _mask__NegFloat_;
                value = -mantle_view_yawcap->current.value;
            }
            ps->delta_angles[1] = ps->delta_angles[1] + delta;
            ps->viewangles[1] = AngleNormalize360(ps->mantleState.yaw + value);
        }
    }
}

void __cdecl Mantle_ClearHint(playerState_s *ps)
{
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp", 1173, 0, "%s", "ps") )
        __debugbreak();
    ps->mantleState.flags &= 0xFFFFFF6F;
}

bool __cdecl Mantle_IsWeaponInactive(const playerState_s *ps)
{
    if ( !mantle_enable->current.enabled )
        return 0;
    if ( (ps->pm_flags & 4) != 0 )
        return (ps->mantleState.flags & 0x40) == 0;
    return 0;
}

bool __cdecl Mantle_IsWeaponEquipped(const playerState_s *ps)
{
    return (ps->pm_flags & 4) != 0 && !Mantle_IsWeaponInactive(ps);
}

bool __cdecl Mantle_DoAnim(const playerState_s *ps)
{
    return Trans_GetHeight(ps->mantleState.transIndex) > mantle_weapon_anim_height->current.value;
}

int __cdecl Mantle_GetDuration(const playerState_s *ps)
{
    int UpLength; // esi

    UpLength = Mantle_GetUpLength(&ps->mantleState);
    return UpLength + Mantle_GetOverLength(&ps->mantleState);
}

double __cdecl Mantle_GetMantleFrac(const playerState_s *ps)
{
    int mantleLength; // [esp+4h] [ebp-4h]

    mantleLength = Mantle_GetDuration(ps);
    if ( mantleLength <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_mantle.cpp",
                    1238,
                    0,
                    "%s\n\t(mantleLength) = %i",
                    "(mantleLength > 0)",
                    mantleLength) )
    {
        __debugbreak();
    }
    return (double)ps->mantleState.timer / (double)mantleLength;
}

bool __cdecl Mount_CanPlayerDeployTurret(playerState_s *ps)
{
    int pmflags; // [esp+8h] [ebp-8h]

    if ( ps->weaponstate == 10
        || ps->weaponstate == 12
        || ps->weaponstate == 14
        || ps->weaponstate == 13
        || ps->weaponstate == 11
        || ps->weaponstate == 15
        || ps->weaponstate == 16 )
    {
        return 0;
    }
    if ( ps->weaponstate == 17 || ps->weaponstate == 18 || ps->weaponstate == 19 )
        return 0;
    if ( ps->weaponstate == 1
        || ps->weaponstate == 2
        || ps->weaponstate == 3
        || ps->weaponstate == 4
        || ps->weaponstate == 5 )
    {
        return 0;
    }
    if ( ps->weaponstate == 3 || ps->weaponstate == 4 || ps->weaponstate == 5 )
        return 0;
    if ( ps->weaponstate == 1 || ps->weaponstate == 2 )
        return 0;
    if ( ps->weaponstate >= 27 && ps->weaponstate <= 29 )
        return 0;
    if ( ps->weaponstate == 35 || ps->weaponstate == 36 || ps->weaponstate == 37 )
        return 0;
    if ( ps->weaponstate >= 20 && ps->weaponstate <= 25 )
        return 0;
    if ( (ps->eFlags & 0x300) != 0 )
        return 0;
    if ( BG_GetAmmoInClip(ps, ps->weapon) <= 0 )
        return 0;
    pmflags = ps->pm_flags;
    if ( (pmflags & 1) != 0 )
        pmflags &= ~0x4000u;
    return (pmflags & 0x2E00C) == 0;
}

int __cdecl BG_GetAmmoInClip(const playerState_s *ps, unsigned int weaponIndex)
{
    const WeaponVariantDef *WeaponVariantDef; // eax

    if ( !ps && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.h", 130, 0, "%s", "ps") )
        __debugbreak();
    WeaponVariantDef = BG_GetWeaponVariantDef(weaponIndex);
    return BG_GetAmmoInClipForWeaponDef(ps, WeaponVariantDef);
}

int __cdecl BG_GetAmmoInClipForWeaponDef(const playerState_s *ps, const WeaponVariantDef *weapVarDef)
{
    int i; // [esp+4h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.h", 119, 0, "%s", "ps") )
        __debugbreak();
    if ( !weapVarDef
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.h", 120, 0, "%s", "weapVarDef") )
    {
        __debugbreak();
    }
    for ( i = 0; i < 15; ++i )
    {
        if ( ps->ammoInClip[i].clipIndex == weapVarDef->iClipIndex )
            return ps->ammoInClip[i].count;
    }
    return 0;
}

