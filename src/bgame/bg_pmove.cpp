#include "bg_pmove.h"
#include "bg_public.h"

#include <cgame_mp/cg_predict_mp.h>
#include <game/actor_physics.h>
#include "bg_misc.h"
#include "bg_slidemove.h"
#include <glass/glass_server.h>
#include "bg_weapons_ammo.h"
#include <universal/com_math_anglevectors.h>
#include "bg_dtp.h"
#include <win32/win_shared.h>
#include "bg_jump.h"
#include <client/splitscreen.h>
#include <game_mp/g_main_mp.h>

const scriptAnimMoveTypes_t notMovingAnims[3] =
{ ANIM_MT_IDLE, ANIM_MT_TURNRIGHT, ANIM_MT_TURNLEFT };

float GUNNER_CROUCH_TIME = 400.0f;
float PRONE_FEET_DIST_TURNED = 55.0f;

const float pm_ladderScale = 0.5;
const float pm_waterWadeScale = 0.60000002;
const float pm_prone_accelerate = 19.0;
const float pm_ducked_accelerate = 12.0;
const float pm_accelerate = 9.0;
const float pm_airaccelerate = 1.0;
const float pm_slideaccelerate = 2.0;
const float pm_wateraccelerate = 2.0;
const float pm_flyaccelerate = 8.0;
const float pm_waterfriction = 1.0;
const float pm_ladderfriction = 16.0;
const float pm_spectatorfriction = 5.0;

const scriptAnimMoveTypes_t moveAnimTable[6][3][2] =
{
  {
    { ANIM_MT_RUN, ANIM_MT_STUMBLE },
    { ANIM_MT_WALK, ANIM_MT_STUMBLE_WALK },
    { ANIM_MT_SHUFFLE, ANIM_MT_SHUFFLE }
  },
  {
    { ANIM_MT_WALK, ANIM_MT_WALK },
    { ANIM_MT_WALK, ANIM_MT_WALK },
    { ANIM_MT_SHUFFLE, ANIM_MT_SHUFFLE }
  },
  {
    { ANIM_MT_RUN, ANIM_MT_STUMBLE },
    { ANIM_MT_WALK, ANIM_MT_STUMBLE },
    { ANIM_MT_SHUFFLE, ANIM_MT_SHUFFLE }
  },
  {
    { ANIM_MT_RUN, ANIM_MT_STUMBLE },
    { ANIM_MT_WALK, ANIM_MT_STUMBLE_WALK },
    { ANIM_MT_SHUFFLE, ANIM_MT_SHUFFLE }
  },
  {
    { ANIM_MT_WALK, ANIM_MT_WALK },
    { ANIM_MT_WALK, ANIM_MT_WALK },
    { ANIM_MT_SHUFFLE, ANIM_MT_SHUFFLE }
  },
  {
    { ANIM_MT_RUN, ANIM_MT_STUMBLE },
    { ANIM_MT_WALK, ANIM_MT_STUMBLE },
    { ANIM_MT_SHUFFLE, ANIM_MT_SHUFFLE }
  }
};


const float bobFactorTable[6][2] =
{
  { 0.33500001, 0.28 },
  { 0.25, 0.23999999 },
  { 0.34, 0.28999999 },
  { 0.36000001, 0.30000001 },
  { 0.25, 0.23999999 },
  { 0.34, 0.28999999 }
};

viewLerpWaypoint_s viewLerp_StandCrouch[9] =
{
  { 0, 60.0, 0 },
  { 1, 59.5, 0 },
  { 4, 58.5, 0 },
  { 30, 56.0, 0 },
  { 80, 44.0, 0 },
  { 90, 41.5, 0 },
  { 95, 40.5, 0 },
  { 100, 40.0, 0 },
  { -1, 0.0, 0 }
};

viewLerpWaypoint_s viewLerp_CrouchStand[9] =
{
  { 0, 40.0, 0 },
  { 5, 40.5, 0 },
  { 10, 41.5, 0 },
  { 20, 44.0, 0 },
  { 70, 56.0, 0 },
  { 96, 58.5, 0 },
  { 99, 59.5, 0 },
  { 100, 60.0, 0 },
  { -1, 0.0, 0 }
};

viewLerpWaypoint_s viewLerp_CrouchProne[11] =
{
  { 0, 40.0, 0 },
  { 11, 38.0, 0 },
  { 22, 33.0, 0 },
  { 34, 25.0, 0 },
  { 45, 16.0, 0 },
  { 50, 15.0, 0 },
  { 55, 16.0, 0 },
  { 70, 18.0, 0 },
  { 90, 17.0, 0 },
  { 100, 11.0, 0 },
  { -1, 0.0, 0 }
};

viewLerpWaypoint_s viewLerp_ProneCrouch[8] =
{
  { 0, 11.0, 0 },
  { 5, 10.0, 0 },
  { 30, 21.0, 0 },
  { 50, 25.0, 0 },
  { 67, 31.0, 0 },
  { 83, 34.0, 0 },
  { 100, 40.0, 0 },
  { -1, 0.0, 0 }
};


// *WARNING* One or more selections were skipped as they could not be interpreted as c data


const float CorrectSolidDeltas[26][3] =
{
  { 0.0, 0.0, 1.0 },
  { -1.0, 0.0, 1.0 },
  { 0.0, -1.0, 1.0 },
  { 1.0, 0.0, 1.0 },
  { 0.0, 1.0, 1.0 },
  { -1.0, 0.0, 0.0 },
  { 0.0, -1.0, 0.0 },
  { 1.0, 0.0, 0.0 },
  { 0.0, 1.0, 0.0 },
  { 0.0, 0.0, -1.0 },
  { -1.0, 0.0, -1.0 },
  { 0.0, -1.0, -1.0 },
  { 1.0, 0.0, -1.0 },
  { 0.0, 1.0, -1.0 },
  { -1.0, -1.0, 1.0 },
  { 1.0, -1.0, 1.0 },
  { 1.0, 1.0, 1.0 },
  { -1.0, 1.0, 1.0 },
  { -1.0, -1.0, 0.0 },
  { 1.0, -1.0, 0.0 },
  { 1.0, 1.0, 0.0 },
  { -1.0, 1.0, 0.0 },
  { -1.0, -1.0, -1.0 },
  { 1.0, -1.0, -1.0 },
  { 1.0, 1.0, -1.0 },
  { -1.0, 1.0, -1.0 }
};




void __cdecl setup_gjkcc_input(pmove_t *pm, gjkcc_input_t *gjkcc_in)
{
    gjkcc_in->gjkcc_id = (unsigned int)pm;
    gjkcc_in->is_server_thread = pm->handler == 1;
    gjkcc_in->proximity_data = &pm->proximity_data;
    gjkcc_in->proximity_mask = 0x3818813;
    if ( pm->handler == 1 )
        gjkcc_in->m_ent_num = pm->ps->clientNum;
    else
        gjkcc_in->m_ent_num = -1;
    gjkcc_in->m_gjk_query_flags = 3;
    gjkcc_in->m_gjk_cg = 0;
    gjkcc_in->m_mat = 0;
}

int g_num_client_trace_calls;
int g_num_server_trace_calls;

void __cdecl PM_trace(
                pmove_t *pm,
                trace_t *results,
                const float *start,
                const float *mins,
                const float *maxs,
                const float *end,
                int passEntityNum,
                int contentMask)
{
    int handler; // ecx
    col_context_t context; // [esp+70h] [ebp-2Ch] BYREF
    bool is_server_thread; // [esp+9Bh] [ebp-1h]

    handler = pm->handler;
    is_server_thread = handler == 1;
    if ( handler == 1 )
        ++g_num_server_trace_calls;
    else
        ++g_num_client_trace_calls;

    //col_context_t::col_context_t(&context);

//    colgeom_visitor_inlined_t<200>::update(
        pm->proximity_data.update(
        start,
        end,
        mins,
        maxs,
        MASK_PLAYER_FULL);

    iassert((MASK_PLAYER_FULL & contentMask) == contentMask);

    context.prims = pm->proximity_data.prims;
    context.nprims = pm->proximity_data.nprims;
    pmoveHandlers[pm->handler].trace(results, start, mins, maxs, end, passEntityNum, contentMask, &context);
}

void __cdecl PM_playerTrace(
                pmove_t *pm,
                trace_t *results,
                float *start,
                float *mins,
                float *maxs,
                float *end,
                int passEntityNum,
                int contentMask)
{
    unsigned __int16 EntityHitId; // ax
    const gjkcc_input_t *gjkcc_in; // [esp+104h] [ebp-4h]
    int savedregs; // [esp+108h] [ebp+0h] BYREF

    if ( phys_player_collision_mode->current.integer == 1 )
    {
        gjkcc_in = pm->m_gjkcc_input;
        gjk_player_trace(gjkcc_in, results, start, mins, maxs, end, passEntityNum, contentMask);

        if (results->fraction > 0.0)
        {
            moveclip_t clip; // [esp+78h] [ebp-90h] BYREF

            //TraceExtents::TraceExtents(&clip.extents);
            clip.extents.start.vec.v[0] = start[0];
            clip.extents.start.vec.v[1] = start[1];
            clip.extents.start.vec.v[2] = start[2];
            clip.extents.end.vec.v[0] = end[0];
            clip.extents.end.vec.v[1] = end[1];
            clip.extents.end.vec.v[2] = end[2];
            clip.mins[0] = mins[0];
            clip.mins[1] = mins[1];
            clip.mins[2] = mins[2];
            clip.maxs[0] = maxs[0];
            clip.maxs[1] = maxs[1];
            clip.maxs[2] = maxs[2];
            clip.contentmask = contentMask;
            GlassSv_ClipMoveTrace(&clip, results);
        }

        return;
    }

    col_context_t context; // [esp+DCh] [ebp-2Ch] BYREF

    //col_context_t::col_context_t(&context);
    pm->proximity_data.update(start, end, mins, maxs, MASK_PLAYER_FULL);

    iassert((MASK_PLAYER_FULL & contentMask) == contentMask);

    context.prims = pm->proximity_data.prims;
    context.nprims = pm->proximity_data.nprims;
    pmoveHandlers[pm->handler].trace(results, start, mins, maxs, end, passEntityNum, contentMask, &context);
    if (results->startsolid && (results->cflags & 0x2008000) != 0)
    {
        EntityHitId = Trace_GetEntityHitId(results);
        PM_AddTouchEnt(pm, EntityHitId);
        pm->tracemask &= 0xFDFF7FFF;
        pmoveHandlers[pm->handler].trace(results, start, mins, maxs, end, passEntityNum, contentMask & 0xFDFF7FFF, &context);

        if ( results->fraction > 0.0 )
        {
            moveclip_t clip; // [esp+78h] [ebp-90h] BYREF

            //TraceExtents::TraceExtents(&clip.extents);
            clip.extents.start.vec.v[0] = start[0];
            clip.extents.start.vec.v[1] = start[1];
            clip.extents.start.vec.v[2] = start[2];
            clip.extents.end.vec.v[0] = end[0];
            clip.extents.end.vec.v[1] = end[1];
            clip.extents.end.vec.v[2] = end[2];
            clip.mins[0] = mins[0];
            clip.mins[1] = mins[1];
            clip.mins[2] = mins[2];
            clip.maxs[0] = maxs[0];
            clip.maxs[1] = maxs[1];
            clip.maxs[2] = maxs[2];
            clip.contentmask = contentMask;
            GlassSv_ClipMoveTrace(&clip, results);
        }
    }
}

void __cdecl PM_AddEvent(playerState_s *ps, unsigned int newEvent)
{
    BG_AddPredictableEventToPlayerstate(newEvent, 0, ps);
}

void __cdecl PM_AddTouchEnt(pmove_t *pm, int entityNum)
{
    int i; // [esp+0h] [ebp-4h]

    if ( entityNum != 1022 )
    {
        if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 427, 0, "%s", "pm") )
            __debugbreak();
        if ( pm->numtouch != 32 )
        {
            for ( i = 0; i < pm->numtouch; ++i )
            {
                if ( pm->touchents[i] == entityNum )
                    return;
            }
            pm->touchents[pm->numtouch++] = entityNum;
        }
    }
}

void __cdecl PM_AddTouchGlass(pmove_t *pm, unsigned int glassId)
{
    int i; // [esp+0h] [ebp-4h]

    if ( glassId != -1 )
    {
        if ( glassId >= 0x3E8
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                        460,
                        0,
                        "%s",
                        "glassId >= 0 && glassId < 1000") )
        {
            __debugbreak();
        }
        if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 462, 0, "%s", "pm") )
            __debugbreak();
        if ( pm->numGlassTouch != 32 )
        {
            for ( i = 0; i < pm->numGlassTouch; ++i )
            {
                if ( pm->touchGlasses[i] == glassId )
                    return;
            }
            pm->touchGlasses[pm->numGlassTouch++] = glassId;
        }
    }
}

void __cdecl PM_ClipVelocity(const float *in, const float *normal, float *out)
{
    float scale; // [esp+0h] [ebp-14h]
    float parallel; // [esp+10h] [ebp-4h]

    parallel = Vec3Dot(in, normal);
    parallel = parallel - I_fabs(parallel) * EQUAL_EPSILON;
    scale = -parallel;
    Vec3Mad(in, scale, normal, out);
}

void __cdecl PM_ProjectVelocity(const float *velIn, const float *normal, float *velOut)
{
    float lengthSq2D; // [esp+24h] [ebp-20h]
    float adjusted[3]; // [esp+28h] [ebp-1Ch] BYREF
    float adjustedLengthSq; // [esp+34h] [ebp-10h]
    float newZ; // [esp+38h] [ebp-Ch]
    float lengthScale; // [esp+3Ch] [ebp-8h]
    float originalLengthSq; // [esp+40h] [ebp-4h]

    lengthSq2D = Vec2LengthSq(velIn);

    if (I_fabs(normal[2]) < EQUAL_EPSILON || lengthSq2D == 0.0)
    {
        velOut[0] = velIn[0];
        velOut[1] = velIn[1];
        velOut[2] = velIn[2];
    }
    else
    {
        newZ = -(normal[1] * velIn[1] + normal[0] * velIn[0]) / normal[2];

        adjusted[0] = velIn[0];
        adjusted[1] = velIn[1];
        adjusted[2] = newZ;

        originalLengthSq = velIn[2] * velIn[2] + lengthSq2D;
        adjustedLengthSq = newZ * newZ + lengthSq2D;
        lengthScale = sqrt(originalLengthSq / adjustedLengthSq);
        if (lengthScale < 1.0 || newZ < 0.0 || velIn[2] > 0.0)
        {
            Vec3Scale(adjusted, lengthScale, velOut);
        }
    }
}

int __cdecl PM_GetEffectiveStance(const playerState_s *ps)
{
    if ( ps->viewHeightTarget == 22 )
    {
        if ( ps->waterlevel < 2 )
            return (ps->waterlevel != 0) + 1;
        else
            return 0;
    }
    else if ( ps->viewHeightTarget == 40 )
    {
        return 2;
    }
    else
    {
        return ps->viewHeightTarget == 11;
    }
}

int __cdecl PM_GetSprintLeft(const playerState_s *ps, int gametime)
{
    int v4; // [esp+10h] [ebp-14h]
    int sprintLeft; // [esp+1Ch] [ebp-8h]
    int maxSprintTime; // [esp+20h] [ebp-4h]

    maxSprintTime = BG_GetMaxSprintTime(ps);
    if ( player_sprintUnlimited->current.enabled )
    {
        sprintLeft = maxSprintTime;
    }
    else if ( (ps->perks[1] & 0x80000) != 0 )
    {
        sprintLeft = maxSprintTime;
    }
    else if ( ps->sprintState.lastSprintStart )
    {
        if ( ps->sprintState.lastSprintStart <= ps->sprintState.lastSprintEnd )
        {
            if ( ps->sprintState.sprintDelay )
                sprintLeft = gametime
                                     + ps->sprintState.sprintStartMaxLength
                                     - (ps->sprintState.lastSprintEnd
                                        - ps->sprintState.lastSprintStart)
                                     - ps->sprintState.lastSprintEnd
                                     - ps->sprintState.sprintCooldown;
            else
                sprintLeft = gametime
                                     + ps->sprintState.sprintStartMaxLength
                                     - (ps->sprintState.lastSprintEnd
                                        - ps->sprintState.lastSprintStart)
                                     - ps->sprintState.lastSprintEnd;
        }
        else
        {
            sprintLeft = ps->sprintState.sprintStartMaxLength - (gametime - ps->sprintState.lastSprintStart);
        }
    }
    else
    {
        sprintLeft = maxSprintTime;
    }
    if ( sprintLeft < 0 )
        v4 = 0;
    else
        v4 = sprintLeft;
    if ( maxSprintTime < v4 )
        return maxSprintTime;
    else
        return v4;
}

int __cdecl PM_GetSprintLeftLastTime(const playerState_s *ps)
{
    int v3; // [esp+10h] [ebp-14h]
    int maxSprintTime; // [esp+20h] [ebp-4h]

    if ( PM_IsSprinting(ps)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                    635,
                    0,
                    "%s",
                    "!PM_IsSprinting( ps )") )
    {
        __debugbreak();
    }
    maxSprintTime = BG_GetMaxSprintTime(ps);
    if ( player_sprintUnlimited->current.enabled )
        return maxSprintTime;
    if ( (ps->perks[1] & 0x80000) != 0 )
        return maxSprintTime;
    if ( ps->sprintState.sprintStartMaxLength - (ps->sprintState.lastSprintEnd - ps->sprintState.lastSprintStart) < 0 )
        v3 = 0;
    else
        v3 = ps->sprintState.sprintStartMaxLength - (ps->sprintState.lastSprintEnd - ps->sprintState.lastSprintStart);
    if ( maxSprintTime < v3 )
        return maxSprintTime;
    else
        return v3;
}

bool __cdecl PM_IsSprinting(const playerState_s *ps)
{
    return ps->sprintState.lastSprintStart && ps->sprintState.lastSprintStart > ps->sprintState.lastSprintEnd;
}

bool __cdecl PM_CanStand(playerState_s *ps, pmove_t *pm)
{
    return (ps->pm_flags & 3) == 0
            || is_not_penetrating(
                     pm,
                     ps->origin,
                     (float *)playerMins,
                     (float *)playerMaxs,
                     ps->origin,
                     pm->tracemask & 0xFDFF7FFF);
}

bool __cdecl is_not_penetrating(pmove_t *pm, float *start, float *mins, float *maxs, float *end, int contentMask)
{
    trace_t trace; // [esp+4h] [ebp-40h] BYREF
    playerState_s *ps; // [esp+40h] [ebp-4h]

    ps = pm->ps;
    PM_playerTrace(pm, &trace, start, mins, maxs, end, ps->clientNum, contentMask);
    return phys_player_collision_mode->current.integer == 1 && !trace.startsolid
            || phys_player_collision_mode->current.integer != 1 && !trace.allsolid;
}

double __cdecl PM_DamageScale_Walk(int damage_timer)
{
    float timer_max; // [esp+4h] [ebp-Ch]

    if ( !damage_timer )
        return 1.0;
    timer_max = player_dmgtimer_maxTime->current.value;
    if ( timer_max == 0.0 )
        return 1.0;
    else
        return (float)((float)((float)damage_timer * (float)((-player_dmgtimer_minScale->current.value) / timer_max)) + 1.0);
}

unsigned int __cdecl PM_GroundSurfaceType(playerState_s *ps, pml_t *pml)
{
    unsigned int iSurfType; // [esp+0h] [ebp-4h]

    if ( !pml && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 1482, 0, "%s", "pml") )
        __debugbreak();
    if ( (pml->groundTrace.sflags & 0x2000) != 0 )
        return 0;
    iSurfType = (unsigned __int8)((int)(0x3F00000 & pml->groundTrace.sflags) >> 20);
    if ( iSurfType >= 0x1F
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                    1488,
                    0,
                    "%s",
                    "iSurfType >= 0 && iSurfType < SURF_TYPECOUNT") )
    {
        __debugbreak();
    }
    if ( ps )
    {
        if ( ps->waterlevel )
            return 20;
    }
    return iSurfType;
}

int __cdecl PM_GetWaterLevel(const playerState_s *ps, float waterHeight)
{
    if ( waterHeight >= (float)(ps->origin[2] + 60.0) )
        return 5;
    if ( waterHeight >= (float)(ps->origin[2] + 54.0) )
        return 4;
    if ( waterHeight >= (float)(ps->origin[2] + 45.0) )
        return 3;
    if ( waterHeight < (float)(ps->origin[2] + 24.0) )
        return waterHeight >= (float)(ps->origin[2] + 3.0);
    return 2;
}

int __cdecl PM_GetViewHeightLerpTime(const playerState_s *ps, int iTarget, int bDown)
{
    if ( iTarget == 11 )
        return 400;
    if ( iTarget != 40 )
        return 200;
    if ( bDown )
        return 200;
    return 400;
}

bool __cdecl PlayerProneAllowed(pmove_t *pm)
{
    playerState_s *ps; // [esp+30h] [ebp-4h]
    int savedregs; // [esp+34h] [ebp+0h] BYREF

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 3112, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 3114, 0, "%s", "ps") )
        __debugbreak();
    if ( BG_WeaponBlocksProne(pm->ps->weapon) )
        return 0;
    if ( (ps->pm_flags & 1) != 0 )
        return 1;
    if ( ps->groundEntityNum == 1023 )
        return 0;
    if ( pm->ps->waterlevel >= 1 )
        return 0;
    return BG_CheckProne(
                     ps,
                     ps->clientNum,
                     ps->origin,
                     pm->maxs[0],
                     30.0,
                     ps->viewangles[1],
                     &ps->fTorsoPitch,
                     &ps->fWaistPitch,
                     0,
                     ps->groundEntityNum != 1023,
                     1,
                     pm->handler,
                     PCT_CLIENT,
                     50.0) != 0;
}

void __cdecl PM_FootstepEvent(pmove_t *pm, pml_t *pml, int iOldBobCycle, int iNewBobCycle, int bFootStep)
{
    unsigned int v5; // eax
    unsigned int v6; // [esp-8h] [ebp-88h]
    float mins[3]; // [esp+10h] [ebp-70h] BYREF
    float vEnd[3]; // [esp+1Ch] [ebp-64h] BYREF
    int iClipMask; // [esp+28h] [ebp-58h]
    float fTraceDist; // [esp+2Ch] [ebp-54h]
    float maxs[3]; // [esp+30h] [ebp-50h] BYREF
    trace_t trace; // [esp+3Ch] [ebp-44h] BYREF
    int iSurfaceType; // [esp+78h] [ebp-8h]
    playerState_s *ps; // [esp+7Ch] [ebp-4h]

    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 3756, 0, "%s", "ps") )
        __debugbreak();
    if ( iOldBobCycle >> 6 != iNewBobCycle >> 6 )
    {
        if ( ps->groundEntityNum == 1023 )
        {
            if ( bFootStep && (ps->pm_flags & 8) != 0 )
            {
                mins[0] = pm->mins[0];
                mins[1] = pm->mins[1];
                mins[0] = mins[0] + 6.0;
                mins[1] = mins[1] + 6.0;
                mins[2] = 8.0f;
                maxs[0] = pm->maxs[0];
                maxs[1] = pm->maxs[1];
                maxs[2] = pm->maxs[2];
                maxs[0] = maxs[0] - 6.0;
                maxs[1] = maxs[1] - 6.0;
                if ( maxs[2] < 8.0 )
                    maxs[2] = mins[2];
                if ( maxs[0] < mins[0]
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                                3784,
                                0,
                                "%s",
                                "maxs[0] >= mins[0]") )
                {
                    __debugbreak();
                }
                if ( maxs[1] < mins[1]
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                                3785,
                                0,
                                "%s",
                                "maxs[1] >= mins[1]") )
                {
                    __debugbreak();
                }
                if ( maxs[2] < mins[2]
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                                3786,
                                0,
                                "%s",
                                "maxs[2] >= mins[2]") )
                {
                    __debugbreak();
                }
                iClipMask = pm->tracemask & 0xFDFE7FFF;
                fTraceDist = -31.0f;
                vEnd[0] = (float)(-31.0 * ps->vLadderVec[0]) + ps->origin[0];
                vEnd[1] = (float)(-31.0 * ps->vLadderVec[1]) + ps->origin[1];
                vEnd[2] = (float)(-31.0 * ps->vLadderVec[2]) + ps->origin[2];
                if ( phys_player_collision_mode->current.integer == 1 )
                    PM_trace(pm, &trace, ps->origin, mins, maxs, vEnd, ps->clientNum, iClipMask);
                else
                    PM_playerTrace(pm, &trace, ps->origin, mins, maxs, vEnd, ps->clientNum, iClipMask);
                iSurfaceType = (unsigned __int8)((int)(0x3F00000 & trace.sflags) >> 20);
                if ( trace.fraction == 1.0 || !iSurfaceType )
                    iSurfaceType = 21;
                BG_AddPredictableEventToPlayerstate(0x68u, iSurfaceType, ps);
            }
        }
        else if ( bFootStep )
        {
            v6 = PM_GroundSurfaceType(ps, pml);
            v5 = PM_FootstepType(pm);
            BG_AddPredictableEventToPlayerstate(v5, v6, ps);
        }
    }
}

int __cdecl PM_FootstepType(pmove_t *pm)
{
    int iPMFlags; // [esp+0h] [ebp-8h]
    playerState_s *ps; // [esp+4h] [ebp-4h]

    ps = pm->ps;
    iPMFlags = pm->ps->pm_flags;
    if ( (iPMFlags & 1) != 0 )
        return 108;
    if ( (iPMFlags & 2) != 0 )
    {
        if ( (iPMFlags & 0x40) == 0 && ps->leanf == 0.0 )
        {
            if ( player_runThreshhold->current.value <= pm->xyspeed )
                return 106;
            else
                return 107;
        }
        else
        {
            return 107;
        }
    }
    else if ( (iPMFlags & 0x40) == 0 && ps->leanf == 0.0 )
    {
        if ( (iPMFlags & 0x8000) != 0 )
        {
            return 103;
        }
        else if ( player_runThreshhold->current.value <= pm->xyspeed )
        {
            return 104;
        }
        else
        {
            return 105;
        }
    }
    else
    {
        return 105;
    }
}

bool __cdecl PM_ShouldMakeFootsteps(pmove_t *pm)
{
    return pm->xyspeed >= player_footstepsThreshhold->current.value;
}

void __cdecl PM_UpdateLean(
                playerState_s *ps,
                float msec,
                usercmd_s *cmd,
                void (__cdecl *capsuleTrace)(trace_t *, const float *, const float *, const float *, const float *, int, int, col_context_t *))
{
    float v4; // [esp+10h] [ebp-C4h]
    float fLeanFrac; // [esp+14h] [ebp-C0h]
    float v6; // [esp+18h] [ebp-BCh]
    float leanf; // [esp+24h] [ebp-B0h]
    float v8; // [esp+28h] [ebp-ACh]
    float fLean; // [esp+2Ch] [ebp-A8h]
    col_context_t context; // [esp+30h] [ebp-A4h] BYREF
    int iStance; // [esp+58h] [ebp-7Ch]
    float fLeanMax; // [esp+5Ch] [ebp-78h]
    float start[3]; // [esp+60h] [ebp-74h] BYREF
    float end[3]; // [esp+6Ch] [ebp-68h] BYREF
    float leanofs; // [esp+78h] [ebp-5Ch]
    trace_t trace; // [esp+7Ch] [ebp-58h] BYREF
    float tmins[3]; // [esp+B8h] [ebp-1Ch] BYREF
    float tmaxs[3]; // [esp+C4h] [ebp-10h] BYREF
    int leaning; // [esp+D0h] [ebp-4h]

    leaning = 0;
    leanofs = 0.0f;
    //col_context_t::col_context_t(&context);
    if ( ps->weaponstate != 35
        && (cmd->button_bits.testBit(6u) || cmd->button_bits.testBit(7u))
        && (ps->pm_flags & 0x800) == 0
        && ps->pm_type < 9
        && (ps->groundEntityNum != 1023 || ps->pm_type == 1) )
    {
        if ( cmd->button_bits.testBit(6u) )
            --leaning;
        if ( cmd->button_bits.testBit(7u) )
            ++leaning;
    }
    if ( (ps->eFlags & 0x4300) != 0
        && (leaning = 0, (ps->eFlags & 0x4000) != 0)
        && ps->vehiclePos >= 1
        && ps->vehiclePos <= 4
        && ps->vehicleType != 6 )
    {
        if ( cmd->button_bits.testBit(9u) )
            ps->leanf = (float)(msec / GUNNER_CROUCH_TIME) + ps->leanf;
        else
            ps->leanf = ps->leanf - (float)(msec / GUNNER_CROUCH_TIME);
        leanf = ps->leanf;
        if ( (float)(leanf - 1.0) < 0.0 )
            v8 = ps->leanf;
        else
            v8 = 1.0f;
        if ( (float)(0.0 - leanf) < 0.0 )
            v6 = v8;
        else
            v6 = 0.0f;
        ps->leanf = v6;
    }
    else
    {
        iStance = PM_GetEffectiveStance(ps);
        if ( iStance == 1 )
            fLeanMax = 0.25f;
        else
            fLeanMax = 0.5f;
        leanofs = ps->leanf;
        if ( leaning )
        {
            if ( leaning <= 0 )
            {
                if ( leanofs > (-(fLeanMax)) )
                    leanofs = leanofs - (float)((float)(msec / 350.0) * fLeanMax);
                if ( (-(fLeanMax)) > leanofs )
                    leanofs = -fLeanMax;
            }
            else
            {
                if ( fLeanMax > leanofs )
                    leanofs = (float)((float)(msec / 350.0) * fLeanMax) + leanofs;
                if ( leanofs > fLeanMax )
                    leanofs = fLeanMax;
            }
        }
        else if ( leanofs <= 0.0 )
        {
            if ( leanofs < 0.0 )
            {
                leanofs = (float)((float)(msec / 280.0) * fLeanMax) + leanofs;
                if ( leanofs > 0.0 )
                    leanofs = 0.0f;
            }
        }
        else
        {
            leanofs = leanofs - (float)((float)(msec / 280.0) * fLeanMax);
            if ( leanofs < 0.0 )
                leanofs = 0.0f;
        }
        ps->leanf = leanofs;
        if ( ps->leanf != 0.0 )
        {
            fLeanFrac = ps->leanf < 0.0 ? -1.0f : 1.0f;
            start[0] = ps->origin[0];
            start[1] = ps->origin[1];
            start[2] = ps->origin[2];
            start[2] = start[2] + ps->viewHeightCurrent;
            end[0] = start[0];
            end[1] = start[1];
            end[2] = start[2];
            AddLeanToPosition(end, ps->viewangles[1], fLeanFrac, 16.0, 20.0);
            tmins[0] = -8.0f;
            tmins[1] = -8.0f;
            tmins[2] = -8.0f;
            tmaxs[0] = 8.0f;
            tmaxs[1] = 8.0f;
            tmaxs[2] = 8.0f;
            capsuleTrace(
                &trace,
                start,
                tmins,
                tmaxs,
                end,
                ps->clientNum,
                0x3818813, 
                &context);
            fLean = UnGetLeanFraction(trace.fraction);
            if ( fabs(ps->leanf) > fLean )
            {
                if ( ps->leanf < 0.0 )
                    v4 = -1.0f;
                else
                    v4 = 1.0f;
                ps->leanf = fLean * v4;
            }
        }
    }
}

void __cdecl PM_UpdateViewAngles_RangeLimited(playerState_s *ps)
{
    float delta; // [esp+Ch] [ebp-8h]
    float deltaa; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    for ( i = 0; i < 2; ++i )
    {
        if ( ps->viewAngleClampRange[i] < 180.0 )
        {
            delta = AngleDelta(ps->viewAngleClampBase[i], ps->viewangles[i]);
            if ( delta > ps->viewAngleClampRange[i] || (-(ps->viewAngleClampRange[i])) > delta )
            {
                if ( delta <= ps->viewAngleClampRange[i] )
                    deltaa = delta + ps->viewAngleClampRange[i];
                else
                    deltaa = delta - ps->viewAngleClampRange[i];
                ps->delta_angles[i] = ps->delta_angles[i] + deltaa;
                if ( deltaa <= 0.0 )
                    ps->viewangles[i] = AngleNormalize360(ps->viewAngleClampBase[i] + ps->viewAngleClampRange[i]);
                else
                    ps->viewangles[i] = AngleNormalize360(ps->viewAngleClampBase[i] - ps->viewAngleClampRange[i]);
            }
        }
    }
}

void __cdecl PM_UpdateViewAngles(
                int localClientNum,
                playerState_s *ps,
                float msec,
                usercmd_s *cmd,
                unsigned __int8 handler)
{
    float temp; // [esp+10h] [ebp-Ch]
    float oldViewYaw; // [esp+14h] [ebp-8h]

    if ( ps->pm_type != 5 )
    {
        if ( ps->pm_type >= 9 && (ps->eFlags2 & 0x10000000) == 0 )
        {
            if ( ps->stats[1] == 999 )
            {
                temp = AngleNormalize360((float)((float)cmd->angles[1] * 0.0054931641) + ps->delta_angles[1]);
                ps->stats[1] = (int)(float)(temp * 0.0054931641);
            }
LABEL_31:
            PM_UpdateLean(ps, msec, cmd, pmoveHandlers[handler].trace);
            return;
        }
        oldViewYaw = ps->viewangles[1];
        PM_UpdateViewAngles_Clamp(ps, cmd, handler);
        PM_UpdateViewLockedEnt(ps, cmd, handler);
        if ( (ps->eFlags & 0x4300) != 0 )
        {
            if ( (ps->eFlags & 0x4000) != 0 && ps->vehiclePos >= 1 && ps->vehiclePos <= 4 )
                PM_UpdateLean(ps, msec, cmd, pmoveHandlers[handler].trace);
            if ( handler != 1 || ps->vehiclePos < 1 || ps->vehiclePos > 4 )
                PM_UpdateViewAngles_RangeLimited(ps);
        }
        else
        {
            if ( (ps->pm_flags & 4) != 0 )
            {
                Mantle_CapView(ps);
                return;
            }
            if ( (ps->pm_flags & 8) != 0 && ps->groundEntityNum == 1023 && bg_ladder_yawcap->current.value != 0.0 )
                PM_UpdateViewAngles_LadderClamp(ps);
            if ( (ps->pm_flags & 1) != 0 && !bgs->clientinfo[ps->clientNum].needsRevive)
            {
                if ( (ps->eFlags & 0x300) != 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                                5356,
                                0,
                                "%s",
                                "!(ps->eFlags & EF_TURRET_ACTIVE)") )
                {
                    __debugbreak();
                }
                PM_UpdateViewAngles_Prone(ps, msec, cmd, handler, oldViewYaw);
            }
            if ( ps->pm_type != 3 && ps->pm_type != 2 && ps->pm_type != 4 )
                goto LABEL_31;
        }
    }
}

void __cdecl PM_UpdateViewLockedEnt(playerState_s *ps, usercmd_s *cmd, unsigned __int8 handler)
{
    double v3; // st7
    float dir[3]; // [esp+Ch] [ebp-40h] BYREF
    float origin[3]; // [esp+18h] [ebp-34h] BYREF
    float viewPos[3]; // [esp+24h] [ebp-28h]
    float angles[3]; // [esp+30h] [ebp-1Ch] BYREF
    float lookAngles[3]; // [esp+3Ch] [ebp-10h] BYREF
    int i; // [esp+48h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 4929, 0, "%s", "ps") )
        __debugbreak();
    if ( player_viewLockEnt->current.integer )
    {
        pmoveHandlers[handler].getEntityOriginAngles(ps->clientNum, player_viewLockEnt->current.integer, origin, angles);
        viewPos[0] = ps->origin[0];
        viewPos[1] = ps->origin[1];
        viewPos[2] = ps->origin[2];
        viewPos[2] = viewPos[2] + ps->viewHeightCurrent;
        dir[0] = origin[0] - viewPos[0];
        dir[1] = origin[1] - viewPos[1];
        dir[2] = origin[2] - viewPos[2];
        vectoangles(dir, lookAngles);
        for ( i = 0; i < 2; ++i )
        {
            v3 = AngleDelta(lookAngles[i], ps->viewangles[i]);
            ps->delta_angles[i] = v3 + ps->delta_angles[i];
            ps->viewangles[i] = lookAngles[i];
        }
    }
}

void __cdecl PM_UpdateViewAngles_Clamp(playerState_s *ps, usercmd_s *cmd, unsigned __int8 handler)
{
    double v3; // st7
    float temp; // [esp+Ch] [ebp-10h]
    int i; // [esp+10h] [ebp-Ch]
    float maxPitch; // [esp+14h] [ebp-8h]
    float minPitch; // [esp+18h] [ebp-4h]

    minPitch = player_view_pitch_up->current.value;
    maxPitch = player_view_pitch_down->current.value;
    for ( i = 0; i < 3; ++i )
    {
        temp = AngleNormalize180((float)((float)cmd->angles[i] * 0.0054931641) + ps->delta_angles[i]);
        if ( !handler && (ps->eFlags & 0x4000) != 0 && ps->vehiclePos >= 1 && ps->vehiclePos <= 4 && ps->leanf != 0.0 )
        {
            ps->delta_angles[i] = ps->delta_angles[i] - (float)(temp - ps->viewangles[i]);
            continue;
        }
        if ( i )
            goto LABEL_15;
        if ( temp <= maxPitch )
        {
            if ( -minPitch > temp )
            {
                ps->delta_angles[0] = -minPitch - (float)((float)cmd->angles[0] * 0.0054931641);
                temp = -minPitch;
            }
LABEL_15:
            v3 = AngleNormalize180(temp);
            goto LABEL_16;
        }
        ps->delta_angles[0] = maxPitch - (float)((float)cmd->angles[0] * 0.0054931641);
        v3 = AngleNormalize180(maxPitch);
LABEL_16:
        ps->viewangles[i] = v3;
    }
}

void __cdecl PM_UpdateViewAngles_LadderClamp(playerState_s *ps)
{
    double v1; // st7
    float delta; // [esp+8h] [ebp-8h]
    float deltaa; // [esp+8h] [ebp-8h]
    float ladderFacing; // [esp+Ch] [ebp-4h]

    ladderFacing = vectoyaw(ps->vLadderVec) + 180.0;
    delta = AngleDelta(ladderFacing, ps->viewangles[1]);
    if ( delta > bg_ladder_yawcap->current.value || -bg_ladder_yawcap->current.value > delta )
    {
        if ( delta <= bg_ladder_yawcap->current.value )
            deltaa = delta + bg_ladder_yawcap->current.value;
        else
            deltaa = delta - bg_ladder_yawcap->current.value;
        ps->delta_angles[1] = ps->delta_angles[1] + deltaa;
        if ( deltaa <= 0.0 )
            v1 = AngleNormalize360(ladderFacing + bg_ladder_yawcap->current.value);
        else
            v1 = AngleNormalize360(ladderFacing - bg_ladder_yawcap->current.value);
        ps->viewangles[1] = v1;
    }
}

void __cdecl PM_UpdateViewAngles_Prone(
                playerState_s *ps,
                float msec,
                usercmd_s *cmd,
                unsigned __int8 handler,
                float oldViewYaw)
{
    float v5; // xmm0_4
    int i; // eax
    unsigned __int8 v7; // al
    unsigned __int8 v8; // al
    unsigned __int8 v9; // al
    bool v10; // [esp+34h] [ebp-2Ch]
    bool v11; // [esp+38h] [ebp-28h]
    float maxDeltaYaw; // [esp+3Ch] [ebp-24h]
    float delta; // [esp+40h] [ebp-20h]
    float deltaa; // [esp+40h] [ebp-20h]
    float deltab; // [esp+40h] [ebp-20h]
    float newViewYaw; // [esp+44h] [ebp-1Ch]
    float newProneYaw; // [esp+48h] [ebp-18h]
    float newProneYawa; // [esp+48h] [ebp-18h]
    int bProneOK; // [esp+54h] [ebp-Ch]
    int bProneOKa; // [esp+54h] [ebp-Ch]
    bool bRetry; // [esp+58h] [ebp-8h]
    bool bRetrya; // [esp+58h] [ebp-8h]
    int proneBlocked; // [esp+5Ch] [ebp-4h]
    int savedregs; // [esp+60h] [ebp+0h] BYREF

    newViewYaw = ps->viewangles[1];
    proneBlocked = 0;
    delta = AngleDelta(ps->proneDirection, newViewYaw);
    v11 = delta > (float)(bg_prone_yawcap->current.value - 5.0) || (-(bg_prone_yawcap->current.value - 5.0)) > delta;
    v10 = (cmd->forwardmove || cmd->rightmove) && delta != 0.0;
    if ( v11 || v10 )
    {
        maxDeltaYaw = (float)(55.0 * msec) * 0.001;
        if ( maxDeltaYaw <= fabs(delta) )
        {
            if ( delta <= 0.0 )
                v5 = ps->proneDirection + maxDeltaYaw;
            else
                v5 = ps->proneDirection - maxDeltaYaw;
            newProneYaw = v5;
        }
        else
        {
            newProneYaw = ps->viewangles[1];
        }
        bRetry = 1;
        for ( i = BG_CheckProneTurned(ps, newProneYaw, handler); !i; i = BG_CheckProneTurned(ps, newProneYaw, handler) )
        {
            if ( !bRetry )
                goto LABEL_33;
            deltaa = AngleDelta(ps->proneDirection, newProneYaw);
            bRetry = fabs(deltaa) > 1.0;
            if ( fabs(deltaa) <= 1.0 )
            {
                proneBlocked = 1;
            }
            else if ( deltaa <= 0.0 )
            {
                deltaa = -1.0f;
            }
            else
            {
                deltaa = 1.0f;
            }
            newProneYaw = AngleNormalize360(newProneYaw + deltaa);
        }
        v7 = BG_CheckProne(
                     ps,
                     ps->clientNum,
                     ps->origin,
                     15.0,
                     30.0,
                     ps->viewangles[1],
                     0,
                     0,
                     1,
                     ps->groundEntityNum != 1023,
                     1,
                     handler,
                     PCT_CLIENT,
                     PRONE_FEET_DIST_TURNED);
        bProneOK = v7;
        if ( v7 )
        {
            v8 = BG_CheckProne(
                         ps,
                         ps->clientNum,
                         ps->origin,
                         15.0,
                         30.0,
                         newProneYaw,
                         0,
                         0,
                         1,
                         ps->groundEntityNum != 1023,
                         1,
                         handler,
                         PCT_CLIENT,
                         PRONE_FEET_DIST_TURNED);
            bProneOK = v8;
            if ( v8 )
                ps->proneDirection = newProneYaw;
        }
        if ( !bProneOK )
            proneBlocked = 1;
    }
LABEL_33:
    deltab = AngleDelta(ps->proneDirection, ps->viewangles[1]);
    if ( deltab != 0.0 )
    {
        newProneYawa = ps->proneDirection;
        bRetrya = 1;
        while ( 1 )
        {
            v9 = BG_CheckProne(
                         ps,
                         ps->clientNum,
                         ps->origin,
                         15.0,
                         30.0,
                         newProneYawa,
                         0,
                         0,
                         1,
                         ps->groundEntityNum != 1023,
                         1,
                         handler,
                         PCT_CLIENT,
                         PRONE_FEET_DIST_TURNED);
            bProneOKa = v9;
            if ( v9 )
            {
                if ( BG_CheckProneTurned(ps, newProneYawa, handler) )
                    break;
            }
            if ( !bRetrya )
                goto LABEL_46;
            bRetrya = fabs(deltab) > 1.0;
            if ( fabs(deltab) > 1.0 )
            {
                if ( deltab <= 0.0 )
                    deltab = -1.0f;
                else
                    deltab = 1.0f;
            }
            proneBlocked = 1;
            ps->delta_angles[1] = ps->delta_angles[1] + deltab;
            ps->viewangles[1] = AngleNormalize360(ps->viewangles[1] + deltab);
            deltab = AngleDelta(ps->proneDirection, ps->viewangles[1]);
            if ( !bProneOKa )
                newProneYawa = AngleNormalize360(newProneYawa + deltab);
        }
        ps->proneDirection = newProneYawa;
    }
LABEL_46:
    PM_UpdateViewAngles_ProneYawClamp(ps, deltab, proneBlocked, oldViewYaw, newViewYaw);
    PM_UpdateViewAngles_PronePitchClamp(ps);
}

int __cdecl BG_CheckProneTurned(playerState_s *ps, float newProneYaw, unsigned __int8 handler)
{
    float delta; // [esp+2Ch] [ebp-10h]
    float testYaw; // [esp+30h] [ebp-Ch]
    float fraction; // [esp+34h] [ebp-8h]
    int savedregs; // [esp+3Ch] [ebp+0h] BYREF

    delta = AngleDelta(newProneYaw, ps->viewangles[1]);
    fraction = fabs(delta) / 240.0;
    testYaw = AngleNormalize360(newProneYaw - (float)((float)(1.0 - fraction) * delta));
    return (unsigned __int8)BG_CheckProne(
                                                        ps,
                                                        ps->clientNum,
                                                        ps->origin,
                                                        15.0,
                                                        30.0,
                                                        testYaw,
                                                        &ps->fTorsoPitch,
                                                        &ps->fWaistPitch,
                                                        1,
                                                        ps->groundEntityNum != 1023,
                                                        1,
                                                        handler,
                                                        PCT_CLIENT,
                                                        (float)(fraction * PRONE_FEET_DIST_TURNED) + (float)((float)(1.0 - fraction) * 50.0));
}

void __cdecl PM_UpdateViewAngles_ProneYawClamp(
                playerState_s *ps,
                float delta,
                int proneBlocked,
                float oldViewYaw,
                float newViewYaw)
{
    double v5; // st7
    float deltaYaw2; // [esp+8h] [ebp-8h]
    float deltaYaw1; // [esp+Ch] [ebp-4h]
    float deltaYaw1a; // [esp+Ch] [ebp-4h]
    float deltaa; // [esp+1Ch] [ebp+Ch]

    if ( delta > bg_prone_yawcap->current.value || -bg_prone_yawcap->current.value > delta )
    {
        if ( delta <= bg_prone_yawcap->current.value )
            deltaa = delta + bg_prone_yawcap->current.value;
        else
            deltaa = delta - bg_prone_yawcap->current.value;
        ps->delta_angles[1] = ps->delta_angles[1] + deltaa;
        if ( deltaa <= 0.0 )
            v5 = AngleNormalize360(ps->proneDirection + bg_prone_yawcap->current.value);
        else
            v5 = AngleNormalize360(ps->proneDirection - bg_prone_yawcap->current.value);
        ps->viewangles[1] = v5;
    }
    if ( proneBlocked )
    {
        ps->pm_flags |= 0x1000u;
        BG_AddPredictableEventToPlayerstate(0xCDu, 3u, ps);
        deltaYaw1 = AngleDelta(oldViewYaw, ps->viewangles[1]);
        if ( fabs(deltaYaw1) <= 1.0 )
        {
            deltaYaw2 = AngleDelta(newViewYaw, ps->viewangles[1]);
            if ( (float)(deltaYaw1 * deltaYaw2) > 0.0 )
            {
                deltaYaw1a = deltaYaw1 * 0.98;
                ps->viewangles[1] = AngleNormalize360(ps->viewangles[1] + deltaYaw1a);
                ps->delta_angles[1] = ps->delta_angles[1] + deltaYaw1a;
            }
        }
    }
}

void __cdecl PM_UpdateViewAngles_PronePitchClamp(playerState_s *ps)
{
    double v1; // st7
    float delta; // [esp+10h] [ebp-4h]
    float deltaa; // [esp+10h] [ebp-4h]

    delta = AngleDelta(ps->proneTorsoPitch, ps->viewangles[0]);
    if ( delta > 45.0 || delta < -45.0 )
    {
        if ( delta <= 45.0 )
            deltaa = delta + 45.0;
        else
            deltaa = delta - 45.0;
        ps->delta_angles[0] = ps->delta_angles[0] + deltaa;
        if ( deltaa <= 0.0 )
            v1 = AngleNormalize180(ps->proneTorsoPitch + 45.0);
        else
            v1 = AngleNormalize180(ps->proneTorsoPitch - 45.0);
        ps->viewangles[0] = v1;
    }
}

void __cdecl PM_UpdatePronePitch(pmove_t *pm, pml_t *pml)
{
    char v2; // al
    double v3; // st7
    float v4; // [esp+2Ch] [ebp-34h]
    float v5; // [esp+30h] [ebp-30h]
    float fTargPitch; // [esp+54h] [ebp-Ch]
    float fTargPitcha; // [esp+54h] [ebp-Ch]
    float delta; // [esp+58h] [ebp-8h]
    float deltaa; // [esp+58h] [ebp-8h]
    playerState_s *ps; // [esp+5Ch] [ebp-4h]
    int savedregs; // [esp+60h] [ebp+0h] BYREF

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 5393, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 5396, 0, "%s", "ps") )
        __debugbreak();
    if ( (ps->pm_flags & 1) != 0 )
    {
        if ( ps->groundEntityNum == 1023 )
        {
            if ( ps->viewHeightTarget != 22 )
            {
                if ( pml->groundPlane )
                    v2 = BG_CheckProne(
                                 ps,
                                 ps->clientNum,
                                 ps->origin,
                                 15.0,
                                 30.0,
                                 ps->proneDirection,
                                 &ps->fTorsoPitch,
                                 &ps->fWaistPitch,
                                 1,
                                 ps->groundEntityNum != 1023,
                                 pml->groundTrace.walkable,
                                 pm->handler,
                                 PCT_CLIENT,
                                 50.0);
                else
                    v2 = BG_CheckProne(
                                 ps,
                                 ps->clientNum,
                                 ps->origin,
                                 15.0,
                                 30.0,
                                 ps->proneDirection,
                                 &ps->fTorsoPitch,
                                 &ps->fWaistPitch,
                                 1,
                                 ps->groundEntityNum != 1023,
                                 1,
                                 pm->handler,
                                 PCT_CLIENT,
                                 50.0);
                if ( !v2 || ps->waterlevel >= 2 )
                {
                    BG_AddPredictableEventToPlayerstate(9u, 0, ps);
                    ps->pm_flags |= 0x1000u;
                }
                goto LABEL_24;
            }
        }
        else if ( (!pml->groundPlane || pml->groundTrace.walkable)
                     && (ps->viewHeightTarget != 22
                        || fabs(ps->fTorsoPitch) <= 50.0
                        && fabs(ps->fWaistPitch) <= 50.0) )
        {
            goto LABEL_24;
        }
        BG_AddPredictableEventToPlayerstate(9u, 0, ps);
LABEL_24:
        if ( pml->groundPlane )
        {
            if ( ps->pm_type == 6 )
                fTargPitch = PitchForYawOnNormal(ps->viewangles[1], pml->groundTrace.normal.vec.v);
            else
                fTargPitch = PitchForYawOnNormal(ps->proneDirection, pml->groundTrace.normal.vec.v);
        }
        else
        {
            fTargPitch = 0.0f;
        }
        delta = AngleDelta(fTargPitch, ps->proneDirectionPitch);
        if ( delta != 0.0 )
        {
            if ( fabs(delta) <= (float)(70.0 * pml->frametime) )
            {
                ps->proneDirectionPitch = ps->proneDirectionPitch + delta;
            }
            else
            {
                if ( delta < 0.0 )
                    v5 = -1.0f;
                else
                    v5 = 1.0f;
                ps->proneDirectionPitch = (float)((float)(70.0 * pml->frametime) * v5) + ps->proneDirectionPitch;
            }
            ps->proneDirectionPitch = AngleNormalize180(ps->proneDirectionPitch);
        }
        if ( pml->groundPlane )
        {
            fTargPitcha = PitchForYawOnNormal(ps->viewangles[1], pml->groundTrace.normal.vec.v);
            v3 = AngleDelta(fTargPitcha, ps->proneTorsoPitch);
        }
        else
        {
            v3 = AngleDelta(0.0, ps->proneTorsoPitch);
        }
        deltaa = v3;
        if ( deltaa != 0.0 )
        {
            if ( fabs(deltaa) <= (float)(70.0 * pml->frametime) )
            {
                ps->proneTorsoPitch = ps->proneTorsoPitch + deltaa;
            }
            else
            {
                if ( deltaa < 0.0 )
                    v4 = -1.0f;
                else
                    v4 = 1.0f;
                ps->proneTorsoPitch = (float)((float)(70.0 * pml->frametime) * v4) + ps->proneTorsoPitch;
            }
            ps->proneTorsoPitch = AngleNormalize180(ps->proneTorsoPitch);
        }
    }
}

void __cdecl PM_SetProneMovementOverride(playerState_s *ps)
{
    if ( (ps->pm_flags & 1) != 0 )
        ps->pm_flags |= 0x200u;
}

void __cdecl PM_MeleeChargeStart(pmove_t *pm, pml_t *pml)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 5917, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 5920, 0, "%s", "ps") )
        __debugbreak();
    if ( player_bayonetLaunchDebugging->current.enabled )
    {
        pm->cmd.meleeChargeDist = -6;
        pm->cmd.meleeChargeYaw = ps->viewangles[1];
    }
    if ( PM_MeleeChargeIsValid(pm, pml) )
    {
        ps->pm_flags |= 0x20000u;
        ps->meleeChargeYaw = pm->cmd.meleeChargeYaw;
        ps->meleeChargeDist = pm->cmd.meleeChargeDist;
        ps->meleeChargeTime = 0;
    }
    else
    {
        PM_MeleeChargeClear(ps);
    }
}

void __cdecl PM_MeleeChargeClear(playerState_s *ps)
{
    float value; // [esp+0h] [ebp-Ch]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 5857, 0, "%s", "ps") )
        __debugbreak();
    ps->pm_flags &= ~0x20000u;
    ps->meleeChargeYaw = 0.0f;
    ps->meleeChargeDist = 0;
    ps->meleeChargeTime = 0;
    if ( player_bayonetLaunchProof->current.enabled )
    {
        if ( (float)(player_bayonetLaunchZCap->current.value - ps->velocity[2]) < 0.0 )
            value = player_bayonetLaunchZCap->current.value;
        else
            value = ps->velocity[2];
        ps->velocity[2] = value;
    }
}

bool __cdecl PM_MeleeChargeIsValid(pmove_t *pm, pml_t *pml)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 5885, 0, "%s", "pm") )
        __debugbreak();
    if ( !pml && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 5886, 0, "%s", "pml") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 5889, 0, "%s", "ps") )
        __debugbreak();
    if ( (ps->pm_flags & 0x20000) != 0 )
        return 0;
    if ( !pm->cmd.meleeChargeDist )
        return 0;
    if ( ps->pm_type )
        return 0;
    if ( (ps->eFlags & 0x300) != 0 )
        return 0;
    if ( (ps->pm_flags & 0xC) != 0 )
        return 0;
    return !PM_IsInAir(pm);
}

bool __cdecl PM_IsInAir(pmove_t *pm)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 4461, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 4463, 0, "%s", "ps") )
        __debugbreak();
    if ( ps->groundEntityNum != 1023 )
        return 0;
    if ( ps->pm_type == 1 )
        return 0;
    return ps->pm_type != 6;
}

void __cdecl PM_CalcMeleeChargeTime(playerState_s *ps, int *outTime, float *outChargeVel)
{
    float value; // [esp+10h] [ebp-24h]
    float chargeDir[2]; // [esp+14h] [ebp-20h] BYREF
    float range; // [esp+1Ch] [ebp-18h]
    float chargeVel; // [esp+20h] [ebp-14h]
    float chargeTime; // [esp+24h] [ebp-10h]
    int weaponIndex; // [esp+28h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+2Ch] [ebp-8h]

    weaponIndex = 0;
    if ( BG_IsUseAsMeleeWeapon(ps->weapon) )
        weaponIndex = ps->weapon;
    else
        weaponIndex = ps->meleeWeapon;
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( BG_IsBayonetWeapon(weaponIndex) )
        value = player_bayonetRange->current.value;
    else
        value = player_meleeRange->current.value;
    range = value;
    if ( (float)ps->meleeChargeDist > value )
        chargeTime = (float)weapDef->meleeChargeDelay * 0.001;
    else
        chargeTime = (float)weapDef->iMeleeDelay * 0.001;
    if ( chargeTime > 0.0 )
    {
        chargeVel = (float)((float)ps->meleeChargeDist / chargeTime) * 2.0;
        YawVectors2D(ps->meleeChargeYaw, chargeDir, 0);
        *outChargeVel = chargeVel * chargeDir[0];
        outChargeVel[1] = chargeVel * chargeDir[1];
        *outTime = (int)(float)(chargeTime * 1000.0);
    }
    else
    {
        PM_MeleeChargeClear(ps);
    }
}

void __cdecl Pmove_1(pmove_t *pm)
{
    char *Name; // [esp+18h] [ebp-14h]
    int msec; // [esp+1Ch] [ebp-10h]
    int finalTime; // [esp+20h] [ebp-Ch]
    playerState_s *ps; // [esp+28h] [ebp-4h]

    //Name = va("Pmove %d", pm->ps->clientNum);
    PROF_SCOPED("Pmove"); 
    ZoneTextF("clientnum: %d", pm->ps->clientNum);

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 6753, 0, "%s", "ps") )
        __debugbreak();
    finalTime = pm->cmd.serverTime;
    if ( finalTime < ps->commandTime )
    {
        return;
    }
    if ( finalTime > ps->commandTime + 1000 )
        ps->commandTime = finalTime - 1000;
    pm->numtouch = 0;
    pm->numGlassTouch = 0;
    while ( ps->commandTime != finalTime )
    {
        msec = finalTime - ps->commandTime;
        if ( msec > 66 )
            msec = 66;
        pm->cmd.serverTime = msec + ps->commandTime;
        PmoveSingle(pm);
        memcpy(&pm->oldcmd, &pm->cmd, sizeof(pm->oldcmd));
    }
}

void __cdecl PmoveSingle(pmove_t *pm)
{
    int EffectiveStance; // eax
    float *v2; // [esp+10h] [ebp-188h]
    float v3; // [esp+14h] [ebp-184h]
    float *v4; // [esp+38h] [ebp-160h]
    float *v5; // [esp+44h] [ebp-154h]
    int jj; // [esp+74h] [ebp-124h]
    float *v7; // [esp+78h] [ebp-120h]
    int ii; // [esp+80h] [ebp-118h]
    int n; // [esp+84h] [ebp-114h]
    float *v10; // [esp+88h] [ebp-110h]
    int m; // [esp+90h] [ebp-108h]
    int k; // [esp+94h] [ebp-104h]
    float *v13; // [esp+98h] [ebp-100h]
    int j; // [esp+A0h] [ebp-F8h]
    float *v15; // [esp+A4h] [ebp-F4h]
    int i; // [esp+ACh] [ebp-ECh]
    float *velocity; // [esp+B0h] [ebp-E8h]
    float move; // [esp+BCh] [ebp-DCh]
    float move_4; // [esp+C0h] [ebp-D8h]
    float move_8; // [esp+C4h] [ebp-D4h]
    const WeaponDef *weapDefDW; // [esp+CCh] [ebp-CCh]
    pml_t pml; // [esp+F8h] [ebp-A0h] BYREF
    int stance; // [esp+18Ch] [ebp-Ch]
    playerState_s *ps; // [esp+190h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+194h] [ebp-4h]

    memset(&pml.groundTrace, 0, 16);
    set_stance(pm);
    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 6122, 0, "%s", "ps") )
        __debugbreak();
    weapDef = BG_GetWeaponDef(ps->weapon);
    ps->waterlevel = 0;
    if ( (ps->eFlags2 & 0x10000000) != 0 )
    {
        velocity = ps->velocity;
        ps->velocity[0] = 0.0f;
        velocity[1] = 0.0f;
        velocity[2] = 0.0f;
    }
    if ( (ps->pm_flags & 0x20000) != 0 )
    {
        pm->cmd.forwardmove = 127;
        pm->cmd.rightmove = 0;
    }
    if ( (ps->pm_flags & 0x800) != 0 || ps->weaponstate == 35 )
    {
        bitarray<51> mask_bits(12, 8, 9, -1); // [esp+F0h] [ebp-A8h] BYREF
        //bitarray<51>::bitarray<51>(&mask_bits, 12, 8, 9, -1);
        for ( i = 0; i < 2; ++i )
            pm->cmd.button_bits.array[i] &= mask_bits.array[i];
        pm->cmd.forwardmove = 0;
        pm->cmd.rightmove = 0;
        v15 = ps->velocity;
        ps->velocity[0] = 0.0f;
        v15[1] = 0.0f;
        v15[2] = 0.0f;
    }
    else if ( (ps->pm_flags & 0x400) != 0 )
    {
        bitarray<51> v25(12, 8, 9, 0, -1); // [esp+E8h] [ebp-B0h] BYREF
        //bitarray<51>::bitarray<51>(&v25, 12, 8, 9, 0, -1);
        for ( j = 0; j < 2; ++j )
            pm->cmd.button_bits.array[j] &= v25.array[j];
        pm->cmd.forwardmove = 0;
        pm->cmd.rightmove = 0;
        v13 = ps->velocity;
        ps->velocity[0] = 0.0f;
        v13[1] = 0.0f;
        v13[2] = 0.0f;
    }
    else if ( PM_IsPlayerFrozenByWeapon(ps) )
    {
        pm->cmd.forwardmove = 0;
        pm->cmd.rightmove = 0;

        bitarray<51> v24(10, 6, 7, -1); // [esp+E0h] [ebp-B8h] BYREF
        //bitarray<51>::bitarray<51>(&v24, 10, 6, 7, -1);
        for ( k = 0; k < 2; ++k )
            v24.array[k] = ~v24.array[k];
        for ( m = 0; m < 2; ++m )
            pm->cmd.button_bits.array[m] &= v24.array[m];
        v10 = ps->velocity;
        ps->velocity[0] = 0.0f;
        v10[1] = 0.0f;
        v10[2] = 0.0f;
    }
    else if ( (ps->eFlags2 & 0x10000000) != 0 && (ps->eFlags & 0x4000) == 0 )
    {
        pm->cmd.forwardmove = 0;
        pm->cmd.rightmove = 0;

        bitarray<51> v23(10, 6, 7, -1); // [esp+D8h] [ebp-C0h] BYREF
        //bitarray<51>::bitarray<51>(&v23, 10, 6, 7, -1);
        for ( n = 0; n < 2; ++n )
            v23.array[n] = ~v23.array[n];
        for ( ii = 0; ii < 2; ++ii )
            pm->cmd.button_bits.array[ii] &= v23.array[ii];
        v7 = ps->velocity;
        ps->velocity[0] = 0.0f;
        v7[1] = 0.0f;
        v7[2] = 0.0f;
    }
    if ( pm->cmd.button_bits.testBit(0x1Du) )
    {
        //bitarray<51>::bitarray<51>(&v22, 29, 1, 11, 12, 8, 9, -1);
        bitarray<51> v22(29, 1, 11, 12, 8, 9, -1); // [esp+D0h] [ebp-C8h] BYREF

        for ( jj = 0; jj < 2; ++jj )
            pm->cmd.button_bits.array[jj] &= v22.array[jj];
        pm->cmd.forwardmove = 0;
        pm->cmd.rightmove = 0;
    }
    ps->pm_flags &= ~0x1000u;
    if ( ps->pm_type >= 9 && (ps->eFlags2 & 0x10000000) == 0 )
        pm->tracemask &= 0xFDFF7FFF;
    if ( (ps->pm_flags & 1) == 0 || BG_UsingSniperScope(ps) || (ps->eFlags & 0x300) != 0 )
    {
        ps->pm_flags &= ~0x200u;
    }
    else if ( (pm->cmd.forwardmove == pm->oldcmd.forwardmove
                    || fabs((float)pm->cmd.forwardmove) <= fabs((float)pm->oldcmd.forwardmove))
                 && (pm->cmd.rightmove == pm->oldcmd.rightmove
                    || fabs((float)pm->cmd.rightmove) <= fabs((float)pm->oldcmd.rightmove)) )
    {
        if ( (ps->pm_flags & 0x10) == 0 && (ps->weaponstate < 6u || ps->weaponstate == 10) )
        {
            if ( BG_GetWeaponDef(ps->weapon)->bDualWield
                && (!ps->weaponstateLeft
                 || ps->weaponstateLeft == 10
                 || ps->weaponstate == 1
                 || ps->weaponstate == 2
                 || ps->weaponstate == 3
                 || ps->weaponstate == 4
                 || ps->weaponstate == 5) )
            {
                ps->pm_flags &= ~0x200u;
            }
            else if ( !BG_GetWeaponDef(ps->weapon)->bDualWield )
            {
                ps->pm_flags &= ~0x200u;
            }
        }
    }
    else if ( (ps->pm_flags & 0x400000) == 0 && PM_InteruptWeaponWithProneMove(ps) )
    {
        ps->pm_flags &= ~0x200u;
        PM_ExitAimDownSight(ps);
    }
    stance = PM_GetEffectiveStance(ps);
    if ( (ps->pm_flags & 0x10) != 0 && stance == 1 && !BG_UsingSniperScope(ps) )
    {
        pm->cmd.forwardmove = 0;
        pm->cmd.rightmove = 0;
    }
    if ( !pm->cmd.button_bits.testBit(0x1Du) || ps->locationSelectionType )
        ps->eFlags &= ~0x200000u;
    else
        ps->eFlags |= 0x200000u;
    ps->eFlags &= ~0x40u;
    if ( ps->pm_type != 5 && (ps->pm_flags & 0x400) == 0 )
    {
        if ( (ps->eFlags & 0x4000) != 0 )
        {
            if ( pm->cmd.button_bits.testBit(0x22u)
                || pm->cmd.button_bits.testBit(0x23u)
                || pm->cmd.button_bits.testBit(0) )
            {
                ps->eFlags |= 0x40u;
            }
        }
        else if ( (ps->eFlags & 0x300) == 0 )
        {
            if ( (!ps->weaponstate || ps->weaponstate == 6 || ps->weaponstate == 31)
                && (PM_WeaponAmmoAvailable(ps) || weapDef->unlimitedAmmo)
                && !BG_PlayerWeaponOverheating(ps, ps->weapon)
                && (ps->weapFlags & 8) == 0 )
            {
                if ( pm->cmd.button_bits.testBit(0) && ps->waterlevel < 3 )
                {
                    ps->eFlags |= 0x40u;
                }
                else if ( PM_IsWeaponMinFireTimeElapsed(ps, 0) )
                {
                    ps->eFlags |= 0x40u;
                }
            }
            if ( weapDef->bDualWield && (!ps->weaponstateLeft || ps->weaponstateLeft == 6 || ps->weaponstateLeft == 31) )
            {
                weapDefDW = BG_GetWeaponDef(weapDef->dualWieldWeaponIndex);
                if ( (BG_GetAmmoInClip(ps, weapDef->dualWieldWeaponIndex) || weapDefDW->unlimitedAmmo)
                    && !BG_PlayerWeaponOverheating(ps, weapDef->dualWieldWeaponIndex)
                    && (ps->weapFlags & 8) == 0 )
                {
                    if ( pm->cmd.button_bits.testBit(0x18u) && ps->waterlevel < 3 )
                    {
                        ps->eFlags |= 0x40u;
                    }
                    else if ( PM_IsWeaponMinFireTimeElapsed(ps, 1) )
                    {
                        ps->eFlags |= 0x40u;
                    }
                }
            }
        }
    }
    if ( ps->pm_type < 9
        && !pm->cmd.button_bits.testBit(0)
        && !pm->cmd.button_bits.testBit(8u) )
    {
        ps->pm_flags &= ~0x400u;
    }
    memset(&pml, 0, sizeof(pml));
    pml.msec = pm->cmd.serverTime - ps->commandTime;
    if ( pml.msec >= 1 )
    {
        if ( pml.msec > 200 )
            pml.msec = 200;
    }
    else
    {
        pml.msec = 1;
    }
    ps->commandTime = pm->cmd.serverTime;
    pml.previous_origin[0] = ps->origin[0];
    pml.previous_origin[1] = ps->origin[1];
    pml.previous_origin[2] = ps->origin[2];
    pml.previous_velocity[0] = ps->velocity[0];
    pml.previous_velocity[1] = ps->velocity[1];
    pml.previous_velocity[2] = ps->velocity[2];
    pml.frametime = (float)pml.msec * 0.001;
    PM_AdjustAimSpreadScale(pm, &pml);
    PM_UpdateViewAngles(pm->localClientNum, ps, (float)pml.msec, &pm->cmd, pm->handler);
    PM_SetWaterLevel(pm);
    if ( player_topDownCamMode->current.integer < 2 || ps->pm_type == 2 )
    {
        AngleVectors(ps->viewangles, pml.forward, pml.right, pml.up);
    }
    else
    {
        YawVectors(player_topDownCamAngles->current.vector[1], pml.forward, pml.right);
        memset(pml.up, 0, sizeof(pml.up));
    }
    if ( pm->cmd.forwardmove >= 0 )
    {
        if ( pm->cmd.forwardmove > 0 || !pm->cmd.forwardmove && pm->cmd.rightmove )
            ps->pm_flags &= ~0x20u;
    }
    else
    {
        ps->pm_flags |= 0x20u;
    }
    if ( ps->pm_type >= 7 && (ps->eFlags2 & 0x10000000) == 0 )
    {
        pm->cmd.forwardmove = 0;
        pm->cmd.rightmove = 0;
    }
    if ( stance == 1 && (ps->pm_flags & 0x200) != 0 )
    {
        pm->cmd.forwardmove = 0;
        pm->cmd.rightmove = 0;
    }
    if ( !Dtp_CanMove(pm) )
    {
        pm->cmd.forwardmove = 0;
        pm->cmd.rightmove = 0;
    }
    Mantle_ClearHint(ps);
    BG_AnimUpdatePlayerStateConditions(pm);
    switch ( ps->pm_type )
    {
        case 1:
        case 0xA:
            PM_ClearLadderFlag(ps);
            ps->groundEntityNum = 1023;
            memset(&pml.walking, 0, 12);
            v5 = ps->velocity;
            ps->velocity[0] = 0.0f;
            v5[1] = 0.0f;
            v5[2] = 0.0f;
            PM_UpdateScriptedAnim(pm, &pml);
            PM_UpdateAimDownSightFlag(pm, &pml);
            PM_UpdateSprint(pm, &pml);
            PM_UpdatePlayerWalkingFlag(pm);
            PM_DropTimers(ps, &pml);
            PM_CheckDuck(pm, &pml);
            PM_Footsteps(pm, &pml);
            PM_Weapon(pm, &pml);
            PM_VehicleDrive(pm, &pml);
            if ( (pm->ps->pm_flags & 0x400000) != 0 && pm->ps->pm_type == 1 )
                Dtp_Interrupt(pm);
            break;
        case 2:
            ps->groundEntityNum = 1023;
            PM_ClearLadderFlag(ps);
            PM_UpdateAimDownSightFlag(pm, &pml);
            PM_UpdateSprint(pm, &pml);
            PM_UpdatePlayerWalkingFlag(pm);
            PM_DropTimers(ps, &pml);
            PM_NoclipMove(pm, &pml);
            PM_UpdateAimDownSightLerp(pm, &pml);
            break;
        case 3:
            ps->groundEntityNum = 1023;
            PM_ClearLadderFlag(ps);
            PM_UpdateAimDownSightFlag(pm, &pml);
            PM_UpdateSprint(pm, &pml);
            PM_UpdatePlayerWalkingFlag(pm);
            PM_DropTimers(ps, &pml);
            PM_UFOMove(pm, &pml);
            PM_UpdateAimDownSightLerp(pm, &pml);
            break;
        case 4:
            PM_ClearLadderFlag(ps);
            PM_UpdateAimDownSightFlag(pm, &pml);
            PM_UpdateSprint(pm, &pml);
            PM_UpdatePlayerWalkingFlag(pm);
            PM_CheckDuck(pm, &pml);
            PM_DropTimers(ps, &pml);
            PM_FlyMove(pm, &pml);
            PM_UpdateAimDownSightLerp(pm, &pml);
            BG_ClipCameraToHeliPatch(ps->origin, pml.previous_origin, ps->velocity, 0);
            break;
        case 5:
            PM_ClearLadderFlag(ps);
            PM_UpdateAimDownSightFlag(pm, &pml);
            PM_UpdateSprint(pm, &pml);
            PM_UpdateAimDownSightLerp(pm, &pml);
            break;
        case 6:
        case 7:
            if ( ps->weaponstate == 35 )
                PM_ResetWeaponState(ps);
            PM_ClearLadderFlag(ps);
            ps->eFlags &= 0xFFFFFCFF;
            goto LABEL_151;
        default:
LABEL_151:
            if ( pm->ps->weaponstate == 35 && (pm->ps->eFlags & 0x300) != 0 )
                ps->weaponstate = WEAPON_DEPLOYED;
            if ( pm->ps->weaponstate == WEAPON_DEPLOYED
                && (pm->ps->weapFlags & 0x1000) == 0
                && (BG_GetAmmoInClip(pm->ps, pm->ps->weapon) <= 0 || (pm->ps->eFlags & 0x300) == 0) )
            {
                pm->ps->weapFlags |= 0x1000u;
                PM_AddEvent(pm->ps, 39);
            }
            if ( (ps->eFlags & 0x300) != 0 )
            {
                PM_ClearLadderFlag(ps);
                ps->groundEntityNum = 1023;
                memset(&pml.walking, 0, 12);
                v4 = ps->velocity;
                ps->velocity[0] = 0.0f;
                v4[1] = 0.0f;
                v4[2] = 0.0f;
                PM_UpdateAimDownSightFlag(pm, &pml);
                PM_UpdateSprint(pm, &pml);
                PM_UpdatePlayerWalkingFlag(pm);
                TurretNVGTrigger(pm);
                PM_DropTimers(ps, &pml);
                PM_CheckDuck(pm, &pml);
                PM_UpdateAimDownSightLerp(pm, &pml);
                PM_Footsteps(pm, &pml);
                if ( pm->ps->weaponstate != 35 && pm->ps->weaponstate != 36 && pm->ps->weaponstate != 37 )
                    PM_ResetWeaponState(ps);
                PM_Weapon_Turret(pm, &pml);
            }
            else if ( (ps->eFlags2 & 0x10000000) != 0 )
            {
                PM_GroundTrace(pm, &pml);
                pm->mins[0] = playerMins[0];
                pm->mins[1] = -15.0;
                pm->mins[2] = 0.0;
                pm->maxs[0] = playerMaxs[0];
                pm->maxs[1] = 15.0;
                pm->maxs[2] = 70.0;
                PM_UpdateAimDownSightFlag(pm, &pml);
                PM_DropTimers(ps, &pml);
                PM_UpdateSpinLerp(pm, &pml);
                PM_VehicleDrive(pm, &pml);
                PM_UpdateAimDownSightLerp(pm, &pml);
                PM_StepSlideMove(pm, &pml, pml.groundPlane == 0);
                ps->damageTimer -= (int)(float)(pml.frametime * 1000.0);
                if ( ps->damageTimer <= 0 )
                    ps->damageTimer = 0;
                EffectiveStance = PM_GetEffectiveStance(ps);
                PM_Footsteps_NotMoving(pm, EffectiveStance, 1);
                PM_Weapon(pm, &pml);
                pm->ps->bRunLeftGun = 1;
                PM_Weapon(pm, &pml);
                pm->ps->bRunLeftGun = 0;
            }
            else
            {
                if ( (ps->pm_flags & 4) == 0 )
                {
                    pm->mins[0] = playerMins[0];
                    pm->mins[1] = -15.0;
                    pm->mins[2] = 0.0;
                    pm->maxs[0] = playerMaxs[0];
                    pm->maxs[1] = 15.0;
                    pm->maxs[2] = 70.0;
                    PM_UpdateAimDownSightFlag(pm, &pml);
                    PM_UpdateSprint(pm, &pml);
                    PM_UpdatePlayerWalkingFlag(pm);
                    PM_CheckDuck(pm, &pml);
                    PM_GroundTrace(pm, &pml);
                }
                Mantle_Check(pm, &pml);
                if ( (ps->pm_flags & 4) != 0 )
                {
                    PM_ClearLadderFlag(ps);
                    ps->groundEntityNum = 1023;
                    pml.groundPlane = 0;
                    pml.walking = 0;
                    PM_UpdateAimDownSightFlag(pm, &pml);
                    PM_UpdateSprint(pm, &pml);
                    PM_UpdatePlayerWalkingFlag(pm);
                    PM_CheckDuck(pm, &pml);
                    Mantle_Move(pm, ps, &pml);
                    PM_Weapon(pm, &pml);
                    pm->ps->bRunLeftGun = 1;
                    PM_Weapon(pm, &pml);
                    pm->ps->bRunLeftGun = 0;
                }
                else
                {
                    PM_UpdateScriptedAnim(pm, &pml);
                    PM_UpdateVisionAnims(pm, &pml);
                    PM_UpdatePronePitch(pm, &pml);
                    PM_DropTimers(ps, &pml);
                    if ( ps->pm_type >= 6 )
                        PM_DeadMove(ps, &pml);
                    PM_CheckLadderMove(pm, &pml);
                    PM_CheckMeleeCharge(pm, &pml);
                    Dtp_CheckForEnd(pm);

                    {
                        PROF_SCOPED("PM_Move");
                        if ((ps->pm_flags & 8) != 0)
                        {
                            PM_LadderMove(pm, &pml);
                        }
                        else if ((ps->pm_flags & 0x20000) != 0)
                        {
                            PM_MeleeChargeMove(pm, &pml);
                        }
                        else if (ps->waterlevel < 3)
                        {
                            if (pml.walking)
                                PM_WalkMove(pm, &pml);
                            else
                                PM_AirMove(pm, &pml);
                        }
                        else
                        {
                            PM_SwimMove(pm, &pml);
                        }
                        PM_UpdatePush(pm, &pml);
                    }

                    PM_GroundTrace(pm, &pml);
                    PM_Footsteps(pm, &pml);
                    PM_Weapon(pm, &pml);
                    pm->ps->bRunLeftGun = 1;
                    PM_Weapon(pm, &pml);
                    pm->ps->bRunLeftGun = 0;
                    PM_FoliageSounds(pm);
                    move = ps->origin[0] - pml.previous_origin[0];
                    move_4 = ps->origin[1] - pml.previous_origin[1];
                    move_8 = ps->origin[2] - pml.previous_origin[2];
                    if ( (float)(0.25
                                         * (float)((float)((float)(ps->velocity[0] * ps->velocity[0])
                                                                         + (float)(ps->velocity[1] * ps->velocity[1]))
                                                         + (float)(ps->velocity[2] * ps->velocity[2]))) > (float)((float)((float)((float)(move * move) + (float)(move_4 * move_4))
                                                                                                                                                                                        + (float)(move_8 * move_8))
                                                                                                                                                                        / (float)(pml.frametime
                                                                                                                                                                                        * pml.frametime)) )
                    {
                        v2 = ps->velocity;
                        v3 = 1.0 / pml.frametime;
                        ps->velocity[0] = (float)(1.0 / pml.frametime) * move;
                        v2[1] = v3 * move_4;
                        v2[2] = v3 * move_8;
                    }
                    Sys_SnapVector(ps->velocity);
                }
            }
            break;
    }
}

void __cdecl PM_UpdateVisionAnims(pmove_t *pm, pml_t *pml)
{
    __int16 animNum; // [esp+0h] [ebp-8h]
    const animScriptCommand_t *animCmd; // [esp+4h] [ebp-4h]

    if ( pm->ps->pm_type < 9 )
    {
        if ( BG_IsShellshockAnim(pm->ps) )
        {
            BG_AnimScriptEvent(pm, ANIM_ET_SHELLSHOCK, 1, 0);
        }
        else if ( pm->ps->visionSetLerpRatio <= 0.0 )
        {
            animNum = pm->ps->torsoAnim & 0xFBFF;
            animCmd = BG_AnimScriptEventGetCommand(pm->ps, ANIM_ET_SHELLSHOCK);
            if ( animCmd )
            {
                if ( animCmd->animIndex[0] == animNum )
                    BG_AnimScriptAnimation(pm, AISTATE_COMBAT, ANIM_MT_IDLE, 1);
            }
        }
        else
        {
            BG_AnimScriptEvent(pm, ANIM_ET_FLARED, 1, 0);
        }
    }
}

void __cdecl PM_UpdateScriptedAnim(pmove_t *pm, pml_t *pml)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 897, 0, "%s", "ps") )
        __debugbreak();
    if ( (ps->pm_flags & 0x100000) != 0 )
    {
        if ( !ps->scriptedAnimTime )
            ps->scriptedAnimTime = pm->cmd.serverTime + BG_AnimScriptEvent(pm, ANIM_ET_SCRIPTEVENT, 0, 1);
        if ( pm->cmd.serverTime < ps->scriptedAnimTime )
        {
            BG_AnimScriptEvent(pm, ANIM_ET_SCRIPTEVENT, 1, 0);
        }
        else
        {
            ps->scriptedAnimTime = 0;
            if ( !pm->handler )
                ps->scriptedAnim = 0;
            ps->pm_flags &= ~0x100000u;
            ps->torsoTimer = 0;
            ps->legsTimer = 0;
        }
    }
}

void __cdecl PM_UpdateSprint(pmove_t *pm, const pml_t *pml)
{
    bool unlimited_sprint; // [esp+13h] [ebp-11h]
    SprintState *p_sprintState; // [esp+14h] [ebp-10h]
    int sprintLeft; // [esp+18h] [ebp-Ch]
    playerState_s *ps; // [esp+1Ch] [ebp-8h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 934, 0, "%s", "ps") )
        __debugbreak();
    p_sprintState = &ps->sprintState;
    if ( ps->sprintState.sprintButtonUpRequired && !pm->cmd.button_bits.testBit(1u) )
        p_sprintState->sprintButtonUpRequired = 0;
    if ( ps->pm_type < 2u && (ps->weaponstate < 47 || ps->weaponstate > 49) && BG_GetMaxSprintTime(ps) > 0 )
    {
        if ( (ps->pm_flags & 0x8000) != 0 )
        {
            unlimited_sprint = player_sprintUnlimited->current.enabled;
            if ( (ps->perks[1] & 0x80000) != 0 )
                unlimited_sprint = 1;
            if ( unlimited_sprint
                || pm->cmd.serverTime - ps->sprintState.lastSprintStart < ps->sprintState.sprintStartMaxLength )
            {
                if ( PM_SprintEndingButtons(pm) )
                {
                    PM_EndSprint(ps, pm);
                    if ( pm->cmd.button_bits.testBit(0xAu) )
                        ps->sprintState.lastSprintEnd += 800;
                }
            }
            else
            {
                PM_EndSprint(ps, pm);
                ps->sprintState.sprintDelay = 1;
            }
        }
        else if ( !ps->sprintState.sprintDelay
                     || pm->cmd.serverTime - ps->sprintState.lastSprintEnd >= ps->sprintState.sprintCooldown )
        {
            if ( pm->cmd.button_bits.testBit(1u) )
            {
                if ( (ps->pm_flags & 0x40000) == 0
                    && !p_sprintState->sprintButtonUpRequired
                    && !PM_SprintStartInterferingButtons(ps, pm->cmd.forwardmove, &pm->cmd.button_bits)
                    && PM_CanCrouch(ps, pm)
                    && PM_CanStand(ps, pm) )
                {
                    sprintLeft = PM_GetSprintLeft(ps, pm->cmd.serverTime);
                    if ( (float)sprintLeft > (float)(player_sprintMinTime->current.value * 1000.0) )
                        PM_StartSprint(ps, pm, pml, sprintLeft);
                }
            }
        }
    }
    else
    {
        PM_EndSprint(ps, pm);
    }
}

void __cdecl PM_StartSprint(playerState_s *ps, pmove_t *pm, const pml_t *pml, int sprintLeft)
{
    if ( ps->sprintState.lastSprintEnd
        && ps->sprintState.lastSprintEnd < ps->sprintState.lastSprintStart
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                    674,
                    0,
                    "%s",
                    "ss->lastSprintEnd == 0 || ss->lastSprintEnd >= ss->lastSprintStart") )
    {
        __debugbreak();
    }
    ps->sprintState.sprintStartMaxLength = sprintLeft;
    ps->sprintState.lastSprintStart = pm->cmd.serverTime;
    ps->pm_flags |= 0x8000u;
}

void __cdecl PM_EndSprint(playerState_s *ps, pmove_t *pm)
{
    if ( (ps->pm_flags & 0x8000) != 0 )
    {
        ps->sprintState.sprintDelay = 0;
        ps->sprintState.lastSprintEnd = pm->cmd.serverTime;
        ps->pm_flags &= ~0x8000u;
        if ( pm->cmd.button_bits.testBit(1u) )
            ps->sprintState.sprintButtonUpRequired = 1;
    }
}

bool __cdecl PM_SprintStartInterferingButtons(const playerState_s *ps, int forwardSpeed, bitarray<51> *button_bits)
{
    if ( (ps->pm_flags & 8) != 0 )
        return 1;
    if ( ps->waterlevel >= 2 )
        return 1;
    if ( forwardSpeed <= player_sprintForwardMinimum->current.integer )
        return 1;
    if ( button_bits->testBit(0xAu) )
        return 1;
    if ( button_bits->testBit(4u) )
        return 1;
    if ( button_bits->testBit(2u) )
        return 1;
    if ( button_bits->testBit(0) )
        return 1;
    if ( button_bits->testBit(5u) )
        return 1;
    if ( button_bits->testBit(0xEu) )
        return 1;
    if ( button_bits->testBit(0xFu) )
        return 1;
    if ( button_bits->testBit(0x18u) && BG_GetWeaponDef(ps->weapon)->offhandSlot != OFFHAND_SLOT_EQUIPMENT )
        return 1;
    if ( ps->leanf != 0.0 )
        return 1;
    if ( (ps->pm_flags & 0x1001C) != 0 )
        return 1;
    if ( (ps->pm_flags & 0x4000) != 0 && !ps->pm_time )
        return 0;
    if ( ps->weaponstate == 17
        || ps->weaponstate == 18
        || ps->weaponstate == 19
        || ps->weaponstate >= 20 && ps->weaponstate <= 25 )
    {
        return 1;
    }
    if ( BG_GetWeaponDef(ps->weapon)->offhandSlot == OFFHAND_SLOT_EQUIPMENT
        && (ps->weaponstate == 6
         || ps->weaponstate == 7
         || ps->weaponstate == 31
         || ps->weaponstate == 17
         || ps->weaponstate == 18
         || ps->weaponstate == 19) )
    {
        return 1;
    }
    if ( (0x800000 & ps->pm_flags) != 0 )
        return 1;
    return (ps->pm_flags & 0x400000) != 0;
}

bool __cdecl PM_SprintEndingButtons(const pmove_t *pm)
{
    if ( pm->ps->waterlevel >= 2 )
        return 1;
    if ( (pm->ps->pm_flags & 0x10018) != 0 )
        return 1;
    if ( pm->cmd.forwardmove <= player_sprintForwardMinimum->current.integer )
        return 1;
    if ( pm->cmd.button_bits.testBit(0xAu) )
        return 1;
    if ( pm->cmd.button_bits.testBit(4u) )
        return 1;
    if ( pm->cmd.button_bits.testBit(2u) )
        return 1;
    if ( pm->cmd.button_bits.testBit(0) )
        return 1;
    if ( pm->cmd.button_bits.testBit(5u) )
        return 1;
    if ( pm->cmd.button_bits.testBit(0xEu) )
        return 1;
    if ( pm->cmd.button_bits.testBit(0xFu) )
        return 1;
    if ( pm->cmd.button_bits.testBit(8u) )
        return 1;
    if ( pm->cmd.button_bits.testBit(9u) )
        return 1;
    if ( pm->ps->leanf != 0.0 )
        return 1;
    if ( pm->cmd.button_bits.testBit(0x18u)
        && BG_GetWeaponDef(pm->ps->weapon)->offhandSlot != OFFHAND_SLOT_EQUIPMENT )
    {
        return 1;
    }
    if ( pm->ps->weaponstate == 17
        || pm->ps->weaponstate == 18
        || pm->ps->weaponstate == 19
        || pm->ps->weaponstate >= 20 && pm->ps->weaponstate <= 25
        || pm->ps->weaponstate == 33
        || pm->ps->weaponstate == 34 )
    {
        return 1;
    }
    if ( (0x800000 & pm->ps->pm_flags) != 0 )
        return 1;
    return (pm->ps->pm_flags & 0x40000) != 0;
}

bool __cdecl PM_CanCrouch(playerState_s *ps, pmove_t *pm)
{
    bool result; // al
    float save_maxs2; // [esp+0h] [ebp-8h]

    if ( (ps->pm_flags & 1) == 0 )
        return 1;
    save_maxs2 = pm->maxs[2];
    pm->maxs[2] = 50.0f;
    result = is_not_penetrating(pm, ps->origin, pm->mins, pm->maxs, ps->origin, pm->tracemask & 0xFDFF7FFF);
    pm->maxs[2] = save_maxs2;
    return result;
}

void __cdecl PM_FlyMove(pmove_t *pm, pml_t *pml)
{
    float wishdir[3]; // [esp+18h] [ebp-40h] BYREF
    float wishvel[3]; // [esp+24h] [ebp-34h] BYREF
    float wishspeed; // [esp+30h] [ebp-28h]
    float forward[3]; // [esp+34h] [ebp-24h] BYREF
    float up[3]; // [esp+40h] [ebp-18h] BYREF
    int i; // [esp+4Ch] [ebp-Ch]
    float scale; // [esp+50h] [ebp-8h]
    playerState_s *ps; // [esp+54h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 1519, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 1522, 0, "%s", "ps") )
        __debugbreak();
    PM_Friction(ps, pml, pm);
    scale = PM_CmdScale(ps, &pm->cmd);
    if ( scale == 0.0 )
    {
        memset(wishvel, 0, sizeof(wishvel));
    }
    else
    {
        up[1] = 0.0f;
        up[0] = 0.0f;
        up[2] = 1.0f;
        Vec3Cross(up, pml->right, forward);
        for ( i = 0; i < 3; ++i )
            wishvel[i] = (float)((float)((float)pm->cmd.forwardmove * forward[i])
                                                 + (float)((float)pm->cmd.rightmove * pml->right[i]))
                                 * scale;
    }
    if ( ps->speed )
    {
        scale = PM_MoveScale(ps, 0.0, 0.0, 127.0);
        if ( pm->cmd.button_bits.testBit(6u) )
            wishvel[2] = wishvel[2] - (float)(127.0 * scale);
        if ( pm->cmd.button_bits.testBit(7u) )
            wishvel[2] = (float)(127.0 * scale) + wishvel[2];
    }
    wishdir[0] = wishvel[0];
    wishdir[1] = wishvel[1];
    wishdir[2] = wishvel[2];
    wishspeed = Vec3Normalize(wishdir);
    PM_Accelerate(ps, pml, wishdir, wishspeed, 8.0);
    PM_StepSlideMove(pm, pml, 0);
}

void __cdecl PM_Friction(playerState_s *ps, pml_t *pml, pmove_t *pm)
{
    float value; // [esp+0h] [ebp-44h]
    float v4; // [esp+4h] [ebp-40h]
    float scales[6]; // [esp+Ch] [ebp-38h]
    float control; // [esp+24h] [ebp-20h]
    float *vel; // [esp+28h] [ebp-1Ch]
    float newspeed; // [esp+2Ch] [ebp-18h]
    float drop; // [esp+30h] [ebp-14h]
    float speed; // [esp+34h] [ebp-10h]
    float vec[3]; // [esp+38h] [ebp-Ch] BYREF

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 1029, 0, "%s", "ps") )
        __debugbreak();
    vel = ps->velocity;
    vec[0] = ps->velocity[0];
    vec[1] = ps->velocity[1];
    vec[2] = ps->velocity[2];
    if ( pml->walking )
        vec[2] = 0.0f;
    speed = Vec3Length(vec);
    if ( speed >= 1.0 )
    {
        drop = 0.0f;
        if ( (ps->pm_flags & 0x20000) != 0 )
        {
            if ( ps->meleeChargeTime <= 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                            1048,
                            0,
                            "%s",
                            "ps->meleeChargeTime > 0") )
            {
                __debugbreak();
            }
            drop = (float)(speed / (float)((float)ps->meleeChargeTime * 0.001)) * pml->frametime;
        }
        else if ( ps->waterlevel <= 1 )
        {
            if ( pml->walking && (pml->groundTrace.sflags & 2) == 0 && (ps->pm_flags & 0x100) == 0 )
            {
                if ( stopspeed->current.value <= speed )
                    value = speed;
                else
                    value = stopspeed->current.value;
                control = value;
                if ( (ps->pm_flags & 0x80) != 0 )
                {
                    control = control * 0.30000001;
                }
                else if ( (ps->pm_flags & 0x4000) != 0 )
                {
                    control = Jump_ReduceFriction(ps) * control;
                }
                if ( (ps->pm_flags & 0x400000) != 0 )
                    control = Dtp_ReduceFriction(pm, ps) * control;
                drop = (float)((float)(control * friction->current.value) * pml->frametime) + drop;
            }
            if ( (pml->groundTrace.sflags & 2) != 0 )
                drop = (float)((float)(speed * player_sliding_friction->current.value) * pml->frametime) + drop;
        }
        if ( ps->waterlevel )
        {
            scales[0] = 0.0f;
            scales[1] = 1.0f;
            scales[2] = 2.0f;
            scales[3] = 3.0f;
            scales[4] = 3.0f;
            scales[5] = 3.0f;
            drop = (float)((float)((float)(speed * 1.0) * scales[ps->waterlevel]) * pml->frametime) + drop;
        }
        if ( ps->pm_type == 4 )
            drop = (float)((float)(speed * 5.0) * pml->frametime) + drop;
        newspeed = speed - drop;
        if ( (float)(speed - drop) < 0.0 )
            newspeed = 0.0f;
        v4 = newspeed / speed;
        *vel = (float)(newspeed / speed) * *vel;
        vel[1] = v4 * vel[1];
        vel[2] = v4 * vel[2];
    }
    else
    {
        *vel = 0.0f;
        vel[1] = 0.0f;
        vel[2] = 0.0f;
    }
}

void __cdecl PM_Accelerate(playerState_s *ps, const pml_t *pml, float *wishdir, float wishspeed, float accel)
{
    float value; // [esp+0h] [ebp-58h]
    float wishVelocity_4; // [esp+2Ch] [ebp-2Ch]
    float wishVelocity_8; // [esp+30h] [ebp-28h]
    float pushDir[3]; // [esp+34h] [ebp-24h] BYREF
    float pushLen; // [esp+40h] [ebp-18h]
    float canPush; // [esp+44h] [ebp-14h]
    float control; // [esp+48h] [ebp-10h]
    float addspeed; // [esp+4Ch] [ebp-Ch]
    float currentspeed; // [esp+50h] [ebp-8h]
    float accelspeed; // [esp+54h] [ebp-4h]

    if ( (ps->pm_flags & 8) != 0 )
    {
        wishVelocity_4 = wishspeed * wishdir[1];
        wishVelocity_8 = wishspeed * wishdir[2];
        pushDir[0] = (float)(wishspeed * *wishdir) - ps->velocity[0];
        pushDir[1] = wishVelocity_4 - ps->velocity[1];
        pushDir[2] = wishVelocity_8 - ps->velocity[2];
        pushLen = Vec3Normalize(pushDir);
        canPush = (float)(accel * pml->frametime) * wishspeed;
        if ( canPush > pushLen )
            canPush = pushLen;
        ps->velocity[0] = (float)(canPush * pushDir[0]) + ps->velocity[0];
        ps->velocity[1] = (float)(canPush * pushDir[1]) + ps->velocity[1];
        ps->velocity[2] = (float)(canPush * pushDir[2]) + ps->velocity[2];
    }
    else
    {
        if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 1124, 0, "%s", "ps") )
            __debugbreak();
        currentspeed = (float)((float)(ps->velocity[0] * *wishdir) + (float)(ps->velocity[1] * wishdir[1]))
                                 + (float)(ps->velocity[2] * wishdir[2]);
        addspeed = wishspeed - currentspeed;
        if ( (float)(wishspeed - currentspeed) > 0.0 )
        {
            if ( stopspeed->current.value <= wishspeed )
                value = wishspeed;
            else
                value = stopspeed->current.value;
            control = value;
            accelspeed = (float)(accel * pml->frametime) * value;
            if ( accelspeed > addspeed )
                accelspeed = addspeed;
            ps->velocity[0] = (float)(accelspeed * *wishdir) + ps->velocity[0];
            ps->velocity[1] = (float)(accelspeed * wishdir[1]) + ps->velocity[1];
            ps->velocity[2] = (float)(accelspeed * wishdir[2]) + ps->velocity[2];
        }
    }
}

double __cdecl PM_MoveScale(playerState_s *ps, float fmove, float rmove, float umove)
{
    float max; // [esp+0h] [ebp-Ch]
    float scale; // [esp+8h] [ebp-4h]
    float scalea; // [esp+8h] [ebp-4h]

    max = fabs(fmove);
    if ( fabs(rmove) > fabs(fmove) )
        max = fabs(rmove);
    if ( fabs(umove) > max )
        max = fabs(umove);
    if ( max == 0.0 )
        return 0.0;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 1204, 0, "%s", "ps") )
        __debugbreak();
    scale = (float)((float)ps->speed * max)
                / (float)(127.0 * sqrtf((float)((float)(fmove * fmove) + (float)(rmove * rmove)) + (float)(umove * umove)));
    if ( (ps->pm_flags & 0x40) == 0 && ps->leanf == 0.0 )
        scalea = scale * 1.0;
    else
        scalea = scale * 0.40000001;
    if ( ps->pm_type == 2 )
        scalea = scalea * 3.0;
    if ( ps->pm_type == 3 )
        scalea = scalea * cg_ufo_scaler->current.value;
    if ( ps->pm_type == 4 )
        return (float)(scalea * player_spectateSpeedScale->current.value);
    return scalea;
}

double __cdecl PM_CmdScale(playerState_s *ps, usercmd_s *cmd)
{
    int max; // [esp+4h] [ebp-Ch]
    float total; // [esp+8h] [ebp-8h]
    float scale; // [esp+Ch] [ebp-4h]
    float scalea; // [esp+Ch] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 1238, 0, "%s", "ps") )
        __debugbreak();
    total = sqrtf((float)(cmd->rightmove * cmd->rightmove + cmd->forwardmove * cmd->forwardmove));
    max = abs8(cmd->forwardmove);
    if ( abs8(cmd->rightmove) > max )
        max = abs8(cmd->rightmove);
    if ( !max )
        return 0.0;
    scale = (float)((float)ps->speed * (float)max) / (float)(127.0 * total);
    if ( (ps->pm_flags & 0x40) != 0 || ps->leanf != 0.0 || (ps->pm_flags & 0x400000) != 0 )
        scalea = scale * 0.40000001;
    else
        scalea = scale * 1.0;
    if ( ps->pm_type == 2 )
        scalea = scalea * 3.0;
    if ( ps->pm_type == 3 )
        scalea = scalea * 6.0;
    if ( ps->pm_type == 4 )
        return (float)(scalea * player_spectateSpeedScale->current.value);
    return scalea;
}

void __cdecl PM_AirMove(pmove_t *pm, pml_t *pml)
{
    int j; // [esp+38h] [ebp-6Ch]
    float fmove; // [esp+3Ch] [ebp-68h]
    float wishdir[3]; // [esp+40h] [ebp-64h] BYREF
    float wishvel[3]; // [esp+4Ch] [ebp-58h]
    float wishspeed; // [esp+58h] [ebp-4Ch]
    float smove; // [esp+5Ch] [ebp-48h]
    int i; // [esp+60h] [ebp-44h]
    float scale; // [esp+64h] [ebp-40h]
    playerState_s *ps; // [esp+68h] [ebp-3Ch]
    usercmd_s cmd; // [esp+6Ch] [ebp-38h] BYREF

    for ( j = 0; j < 2; ++j )
        cmd.button_bits.array[j] = 0;
    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 1593, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 1596, 0, "%s", "ps") )
        __debugbreak();
    if ( (ps->pm_flags & 0x400000) != 0 )
        Dtp_AirMove(ps, pm);
    PM_Friction(ps, pml, pm);
    fmove = (float)pm->cmd.forwardmove;
    smove = (float)pm->cmd.rightmove;
    memcpy(&cmd, &pm->cmd, sizeof(cmd));
    scale = PM_CmdScale(ps, &cmd);
    pml->forward[2] = 0.0f;
    pml->right[2] = 0.0f;
    Vec3Normalize(pml->forward);
    Vec3Normalize(pml->right);
    for ( i = 0; i < 2; ++i )
        wishvel[i] = (float)(pml->forward[i] * fmove) + (float)(pml->right[i] * smove);
    wishvel[2] = 0.0f;
    wishdir[0] = wishvel[0];
    wishdir[1] = wishvel[1];
    wishdir[2] = 0.0f;
    wishspeed = Vec3Normalize(wishdir);
    wishspeed = wishspeed * scale;
    PM_Accelerate(ps, pml, wishdir, wishspeed, 1.0);
    PM_DoSlideAdjustments(ps, pml);
    if ( pml->groundPlane )
        PM_ClipVelocity(ps->velocity, pml->groundTrace.normal.vec.v, ps->velocity);
    PM_StepSlideMove(pm, pml, 1);
    PM_SetMovementDir(pm, pml);
}

float accel = 2.0f;
void __cdecl PM_DoSlideAdjustments(playerState_s *ps, const pml_t *pml)
{
    float nvel[3]; // [esp+2Ch] [ebp-24h] BYREF
    float dir[3]; // [esp+38h] [ebp-18h] BYREF
    float wishspeed; // [esp+44h] [ebp-Ch]
    float len2; // [esp+48h] [ebp-8h]
    float vel_cap; // [esp+4Ch] [ebp-4h]

    if ( (pml->groundTrace.sflags & 2) != 0 )
    {
        *(_QWORD *)dir = *(_QWORD *)pml->groundTrace.normal.vec.v;
        dir[2] = 0.0f;
        Vec3Normalize(dir);
        wishspeed = player_sliding_wishspeed->current.value;
        PM_Accelerate(ps, pml, dir, wishspeed, accel);
        vel_cap = player_sliding_velocity_cap->current.value;
        len2 = Vec3Length(ps->velocity);
        if ( len2 > (float)(vel_cap * vel_cap) )
        {
            Vec3NormalizeTo(ps->velocity, nvel);
            ps->velocity[0] = vel_cap * nvel[0];
            ps->velocity[1] = vel_cap * nvel[1];
            ps->velocity[2] = vel_cap * nvel[2];
        }
    }
}

void __cdecl PM_SetMovementDir(pmove_t *pm, pml_t *pml)
{
    float dir[3]; // [esp+24h] [ebp-24h] BYREF
    int moveyaw; // [esp+30h] [ebp-18h]
    float moved[3]; // [esp+34h] [ebp-14h] BYREF
    float speed; // [esp+40h] [ebp-8h]
    playerState_s *ps; // [esp+44h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 1403, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 1406, 0, "%s", "ps") )
        __debugbreak();
    if ( (ps->pm_flags & 1) == 0 || (ps->eFlags & 0x300) != 0 )
    {
        if ( (ps->pm_flags & 8) != 0 )
        {
            speed = vectoyaw(ps->vLadderVec) + 180.0;
            moveyaw = (int)AngleDelta(speed, ps->viewangles[1]);
            if ( (int)abs(moveyaw) > 90 )
            {
                if ( moveyaw <= 0 )
                    moveyaw = -90;
                else
                    moveyaw = 90;
            }
            ps->movementDir = (char)moveyaw;
        }
        else
        {
            moved[0] = ps->origin[0] - pml->previous_origin[0];
            moved[1] = ps->origin[1] - pml->previous_origin[1];
            moved[2] = ps->origin[2] - pml->previous_origin[2];
            if ( !pm->cmd.forwardmove && !pm->cmd.rightmove
                || ps->groundEntityNum == 1023
                || (speed = Vec3Length(moved), speed == 0.0)
                || speed <= (float)(pml->frametime * 5.0)
                || zombietron->current.enabled )
            {
                ps->movementDir = 0;
            }
            else
            {
                Vec3NormalizeTo(moved, dir);
                vectoangles(dir, dir);
                moveyaw = (int)AngleDelta(dir[1], ps->viewangles[1]);
                if ( pm->cmd.forwardmove < 0 )
                    moveyaw = (int)AngleNormalize180((float)moveyaw + 180.0);
                if ( (int)abs(moveyaw) > 90 )
                {
                    if ( moveyaw <= 0 )
                        moveyaw = -90;
                    else
                        moveyaw = 90;
                }
                ps->movementDir = (char)moveyaw;
            }
        }
    }
    else
    {
        moveyaw = (int)AngleDelta(ps->proneDirection, ps->viewangles[1]);
        if ( (int)abs(moveyaw) > 90 )
        {
            if ( moveyaw <= 0 )
                moveyaw = -90;
            else
                moveyaw = 90;
        }
        ps->movementDir = (char)moveyaw;
    }
}

void __cdecl PM_SwimMove(pmove_t *pm, pml_t *pml)
{
    float v2; // [esp+10h] [ebp-ACh]
    float moveSpeedScaleMultiplier; // [esp+14h] [ebp-A8h]
    float v4; // [esp+18h] [ebp-A4h]
    int j; // [esp+2Ch] [ebp-90h]
    float fmove; // [esp+30h] [ebp-8Ch]
    float wishdir[3]; // [esp+34h] [ebp-88h] BYREF
    float mod; // [esp+40h] [ebp-7Ch]
    float undertow_scale; // [esp+44h] [ebp-78h] BYREF
    float wishvel[3]; // [esp+48h] [ebp-74h]
    float dir[3]; // [esp+54h] [ebp-68h] BYREF
    float floatspeed; // [esp+60h] [ebp-5Ch]
    float wishspeed; // [esp+64h] [ebp-58h]
    float floatdir[3]; // [esp+68h] [ebp-54h] BYREF
    float smove; // [esp+74h] [ebp-48h]
    int i; // [esp+78h] [ebp-44h]
    float scale; // [esp+7Ch] [ebp-40h]
    playerState_s *ps; // [esp+80h] [ebp-3Ch]
    usercmd_s cmd; // [esp+84h] [ebp-38h] BYREF

    for ( j = 0; j < 2; ++j )
        cmd.button_bits.array[j] = 0;
    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 1674, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 1677, 0, "%s", "ps") )
        __debugbreak();
    PM_Friction(ps, pml, pm);
    fmove = (float)pm->cmd.forwardmove;
    smove = (float)pm->cmd.rightmove;
    memcpy(&cmd, &pm->cmd, sizeof(cmd));
    scale = PM_CmdScale(ps, &cmd);
    scale = scale * player_waterSpeedScale->current.value;
    for ( i = 0; i < 3; ++i )
        wishvel[i] = (float)(pml->forward[i] * fmove) + (float)(pml->right[i] * smove);
    if ( ps->waterlevel == 3 && wishvel[2] >= 0.0 )
        wishvel[2] = 0.0f;
    wishdir[0] = wishvel[0];
    wishdir[1] = wishvel[1];
    wishdir[2] = wishvel[2];
    wishspeed = Vec3Normalize(wishdir);
    wishspeed = wishspeed * scale;
    floatdir[0] = 0.0f;
    floatdir[1] = 0.0f;
    floatdir[2] = 1.0f;
    floatspeed = 0.0f;
    moveSpeedScaleMultiplier = ps->moveSpeedScaleMultiplier;
    if ( (float)(moveSpeedScaleMultiplier - 1.5) < 0.0 )
        v4 = ps->moveSpeedScaleMultiplier;
    else
        v4 = 1.5f;
    if ( (float)(0.5 - moveSpeedScaleMultiplier) < 0.0 )
        v2 = v4;
    else
        v2 = 0.5f;
    mod = v2;
    if ( v2 <= 1.0 )
        floatspeed = (float)(player_floatSpeed->current.value / 2.0)
                             + (float)((float)((float)(1.0 - mod) * -2.0) * player_floatSpeed->current.value);
    else
        floatspeed = mod * player_floatSpeed->current.value;
    if ( ps->waterlevel > 3 && fmove == 0.0 && smove == 0.0 )
    {
        if ( floatspeed < 0.0 )
        {
            floatspeed = floatspeed * -1.0;
            floatdir[2] = -1.0f;
        }
        PM_Accelerate(ps, pml, floatdir, floatspeed, 2.0);
    }
    PM_Accelerate(ps, pml, wishdir, wishspeed, 2.0);
    if ( CM_GetWaterForce(ps->origin, dir, &undertow_scale) )
        PM_Accelerate(ps, pml, dir, undertow_scale, 2.0);
    pml->groundPlane = 0;
    pml->walking = 0;
    PM_StepSlideMove(pm, pml, 0);
    PM_SetMovementDir(pm, pml);
}

void __cdecl PM_WalkMove(pmove_t *pm, pml_t *pml)
{
    int i; // [esp+3Ch] [ebp-74h]
    float fmove; // [esp+40h] [ebp-70h]
    float wishdir[3]; // [esp+44h] [ebp-6Ch] BYREF
    float wishvel[3]; // [esp+50h] [ebp-60h] BYREF
    int iStance; // [esp+5Ch] [ebp-54h]
    float wishspeed; // [esp+60h] [ebp-50h]
    float acceleration; // [esp+64h] [ebp-4Ch]
    float smove; // [esp+68h] [ebp-48h]
    float scale; // [esp+6Ch] [ebp-44h]
    playerState_s *ps; // [esp+70h] [ebp-40h]
    usercmd_s cmd; // [esp+74h] [ebp-3Ch] BYREF
    int canJump; // [esp+ACh] [ebp-4h]

    for ( i = 0; i < 2; ++i )
        cmd.button_bits.array[i] = 0;
    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 1777, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 1780, 0, "%s", "ps") )
        __debugbreak();
    if ( (ps->pm_flags & 0x4000) != 0 )
        Jump_ApplySlowdown(ps);
    if ( (ps->pm_flags & 0x8000) != 0 )
        pm->cmd.rightmove = (int)(float)((float)pm->cmd.rightmove * player_sprintStrafeSpeedScale->current.value);
    canJump = 1;
    if ( ps->groundEntityNum != 1023 )
        canJump = pmoveHandlers[pm->handler].isEntWalkable(pm->localClientNum, ps->groundEntityNum) != 0;
    if ( canJump && Jump_Check(pm, pml) || Dtp_Update(pm, pml) )
    {
        PM_AirMove(pm, pml);
    }
    else
    {
        PM_Friction(ps, pml, pm);
        fmove = (float)pm->cmd.forwardmove;
        smove = (float)pm->cmd.rightmove;
        memcpy(&cmd, &pm->cmd, sizeof(cmd));
        scale = PM_CmdScale_Walk(pm, &cmd);
        scale = PM_DamageScale_Walk(ps->damageTimer) * scale;
        ps->damageTimer -= (int)(float)(pml->frametime * 1000.0);
        if ( ps->damageTimer <= 0 )
            ps->damageTimer = 0;
        pml->forward[2] = 0.0f;
        pml->right[2] = 0.0f;
        Vec2Normalize(pml->forward);
        Vec2Normalize(pml->right);
        wishvel[0] = (float)(fmove * pml->forward[0]) + (float)(smove * pml->right[0]);
        wishvel[1] = (float)(fmove * pml->forward[1]) + (float)(smove * pml->right[1]);
        wishvel[2] = 0.0f;
        wishspeed = Vec3NormalizeTo(wishvel, wishdir);
        wishspeed = wishspeed * scale;
        PM_ProjectVelocity(wishdir, pml->groundTrace.normal.vec.v, wishdir);
        iStance = PM_GetEffectiveStance(ps);
        if ( (pml->groundTrace.sflags & 2) != 0 )
        {
            acceleration = pm_slideaccelerate;
        }
        else if ( (ps->pm_flags & 0x100) != 0 )
        {
            acceleration = pm_airaccelerate;
        }
        else if ( iStance == 1 )
        {
            acceleration = pm_prone_accelerate;
        }
        else if ( iStance == 2 )
        {
            acceleration = pm_ducked_accelerate;
        }
        else
        {
            acceleration = pm_accelerate;
        }
        if ( (ps->pm_flags & 0x80) != 0 )
            acceleration = acceleration * 0.25;
        PM_Accelerate(ps, pml, wishdir, wishspeed, acceleration);
        PM_DoSlideAdjustments(ps, pml);
        if ( (pml->groundTrace.sflags & 2) != 0 || (ps->pm_flags & 0x100) != 0 )
            ps->velocity[2] = ps->velocity[2] - (float)((float)ps->gravity * pml->frametime);
        PM_ProjectVelocity(ps->velocity, pml->groundTrace.normal.vec.v, ps->velocity);
        PM_StepSlideMove(pm, pml, 0);
        PM_SetMovementDir(pm, pml);
    }
}

double __cdecl PM_CmdScale_Walk(pmove_t *pm, usercmd_s *cmd)
{
    float v2; // xmm0_4
    float v4; // [esp+0h] [ebp-3Ch]
    bool v5; // [esp+4h] [ebp-38h]
    const WeaponDef *weapon; // [esp+24h] [ebp-18h]
    float total; // [esp+28h] [ebp-14h]
    float scale; // [esp+34h] [ebp-8h]
    float scalea; // [esp+34h] [ebp-8h]
    float scaleb; // [esp+34h] [ebp-8h]
    playerState_s *ps; // [esp+38h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 1290, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 1292, 0, "%s", "ps") )
        __debugbreak();
    v5 = (ps->pm_flags & 1) != 0 && ps->fWeaponPosFrac > 0.0;
    total = sqrtf((float)(cmd->rightmove * cmd->rightmove + cmd->forwardmove * cmd->forwardmove));
    if ( cmd->forwardmove >= 0 )
        v2 = fabs((float)cmd->forwardmove);
    else
        v2 = fabs((float)cmd->forwardmove * player_backSpeedScale->current.value);
    if ( (float)(v2
                         - fabs((float)cmd->rightmove * player_strafeSpeedScale->current.value)) < 0.0 )
        v4 = fabs((float)cmd->rightmove * player_strafeSpeedScale->current.value);
    else
        v4 = v2;
    if ( v4 == 0.0 )
        return 0.0;
    scale = (float)((float)ps->speed * v4) / (float)(127.0 * total);
    if ( (ps->pm_flags & 0x40) != 0 || ps->leanf != 0.0 || v5 )
        scalea = scale * 0.40000001;
    else
        scalea = scale * 1.0;
    if ( (ps->pm_flags & 0x8000) != 0 && ps->viewHeightTarget == 60 )
        scalea = scalea * player_sprintSpeedScale->current.value;
    if ( ps->pm_type == 2 )
    {
        scaleb = scalea * 3.0;
    }
    else if ( ps->pm_type == 3 )
    {
        scaleb = scalea * 6.0;
    }
    else
    {
        scaleb = PM_CmdScaleForStance(pm) * scalea;
        if ( ps->waterlevel >= 1 )
            scaleb = scaleb * (float)(1.0 - (float)((float)(1.0 - 0.60000002) * (float)((float)ps->waterlevel / 3.0)));
    }
    weapon = BG_GetWeaponDef(ps->weapon);
    if ( !ps->weapon || weapon->moveSpeedScale <= 0.0 || (ps->pm_flags & 0x40) != 0 || v5 )
    {
        if ( ps->weapon && weapon->adsMoveSpeedScale > 0.0 )
            scaleb = scaleb * weapon->adsMoveSpeedScale;
    }
    else
    {
        scaleb = scaleb * weapon->moveSpeedScale;
    }
    if ( (ps->pm_flags & 0x10000) != 0 )
        scaleb = scaleb * BG_GetShellshockParms(ps->shellshockIndex)->movement.scale;
    return (float)(scaleb * ps->moveSpeedScaleMultiplier);
}

double __cdecl PM_CmdScaleForStance(const pmove_t *pm)
{
    int stance; // [esp+0h] [ebp-8h]
    float lerpFrac; // [esp+4h] [ebp-4h]
    float lerpFraca; // [esp+4h] [ebp-4h]

    if ( (pm->ps->pm_flags & 0x400000) != 0 )
        return 1.0;
    lerpFrac = PM_GetViewHeightLerp(pm, 40, 11);
    if ( lerpFrac != 0.0 )
        return 0.15000001 * lerpFrac + (1.0 - lerpFrac) * 0.64999998;
    lerpFraca = PM_GetViewHeightLerp(pm, 11, 40);
    if ( lerpFraca != 0.0 )
        return 0.64999998 * lerpFraca + (1.0 - lerpFraca) * 0.15000001;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 762, 0, "%s", "pm->ps") )
        __debugbreak();
    stance = PM_GetEffectiveStance(pm->ps);
    if ( stance == 1 )
        return 0.15000001;
    if ( stance == 2 )
        return 0.64999998;
    return 1.0;
}

void __cdecl PM_DeadMove(playerState_s *ps, pml_t *pml)
{
    float forwarda; // [esp+1Ch] [ebp-4h]
    float forward; // [esp+1Ch] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 1888, 0, "%s", "ps") )
        __debugbreak();
    if ( pml->walking )
    {
        forwarda = Vec3Length(ps->velocity);
        forward = forwarda - 20.0;
        if ( forward > 0.0 )
        {
            Vec3Normalize(ps->velocity);
            ps->velocity[0] = forward * ps->velocity[0];
            ps->velocity[1] = forward * ps->velocity[1];
            ps->velocity[2] = forward * ps->velocity[2];
        }
        else
        {
            ps->velocity[0] = 0.0f;
            ps->velocity[1] = 0.0f;
            ps->velocity[2] = 0.0f;
        }
    }
}

void __cdecl PM_NoclipMove(pmove_t *pm, pml_t *pml)
{
    float value; // [esp+Ch] [ebp-78h]
    float *origin; // [esp+10h] [ebp-74h]
    float *v4; // [esp+14h] [ebp-70h]
    float frametime; // [esp+18h] [ebp-6Ch]
    float *v6; // [esp+1Ch] [ebp-68h]
    float *velocity; // [esp+2Ch] [ebp-58h]
    float *v8; // [esp+30h] [ebp-54h]
    float *v9; // [esp+34h] [ebp-50h]
    float fmove; // [esp+3Ch] [ebp-48h]
    float wishdir[3]; // [esp+40h] [ebp-44h] BYREF
    float control; // [esp+4Ch] [ebp-38h]
    float wishvel[3]; // [esp+50h] [ebp-34h]
    float wishspeed; // [esp+5Ch] [ebp-28h]
    float curFriction; // [esp+60h] [ebp-24h]
    float newspeed; // [esp+64h] [ebp-20h]
    float drop; // [esp+68h] [ebp-1Ch]
    float speed; // [esp+6Ch] [ebp-18h]
    float smove; // [esp+70h] [ebp-14h]
    int i; // [esp+74h] [ebp-10h]
    float scale; // [esp+78h] [ebp-Ch]
    playerState_s *ps; // [esp+7Ch] [ebp-8h]
    float umove; // [esp+80h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 1931, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 1934, 0, "%s", "ps") )
        __debugbreak();
    ps->viewHeightTarget = 60;
    speed = Vec3Length(ps->velocity);
    if ( speed >= 1.0 )
    {
        drop = 0.0f;
        curFriction = friction->current.value * 1.5;
        if ( stopspeed->current.value <= speed )
            value = speed;
        else
            value = stopspeed->current.value;
        control = value;
        drop = (float)((float)(value * curFriction) * pml->frametime) + drop;
        newspeed = speed - drop;
        if ( (float)(speed - drop) < 0.0 )
            newspeed = 0.0f;
        newspeed = newspeed / speed;
        velocity = ps->velocity;
        v8 = ps->velocity;
        ps->velocity[0] = newspeed * ps->velocity[0];
        velocity[1] = newspeed * v8[1];
        velocity[2] = newspeed * v8[2];
    }
    else
    {
        v9 = ps->velocity;
        ps->velocity[0] = vec3_origin[0];
        v9[1] = 0.0;
        v9[2] = 0.0;
    }
    fmove = (float)pm->cmd.forwardmove;
    smove = (float)pm->cmd.rightmove;
    umove = 0.0f;
    if ( pm->cmd.button_bits.testBit(7u) )
        umove = umove + 127.0;
    if ( pm->cmd.button_bits.testBit(6u) )
        umove = umove - 127.0;
    scale = PM_MoveScale(ps, fmove, smove, umove);
    for ( i = 0; i < 3; ++i )
        wishvel[i] = (float)((float)(pml->forward[i] * fmove) + (float)(pml->right[i] * smove))
                             + (float)(pml->up[i] * umove);
    wishdir[0] = wishvel[0];
    wishdir[1] = wishvel[1];
    wishdir[2] = wishvel[2];
    wishspeed = Vec3Normalize(wishdir);
    wishspeed = wishspeed * scale;
    PM_Accelerate(ps, pml, wishdir, wishspeed, 9.0);
    origin = ps->origin;
    v4 = ps->velocity;
    frametime = pml->frametime;
    v6 = ps->origin;
    ps->origin[0] = (float)(frametime * ps->velocity[0]) + ps->origin[0];
    origin[1] = (float)(frametime * v4[1]) + v6[1];
    origin[2] = (float)(frametime * v4[2]) + v6[2];
}

void __cdecl PM_UFOMove(pmove_t *pm, pml_t *pml)
{
    float value; // [esp+Ch] [ebp-90h]
    float *origin; // [esp+10h] [ebp-8Ch]
    float *v4; // [esp+14h] [ebp-88h]
    float frametime; // [esp+18h] [ebp-84h]
    float *v6; // [esp+1Ch] [ebp-80h]
    float *velocity; // [esp+2Ch] [ebp-70h]
    float *v8; // [esp+30h] [ebp-6Ch]
    float *v9; // [esp+34h] [ebp-68h]
    float forward[3]; // [esp+3Ch] [ebp-60h] BYREF
    float up[3]; // [esp+48h] [ebp-54h] BYREF
    int i; // [esp+54h] [ebp-48h]
    float fmove; // [esp+58h] [ebp-44h]
    float wishdir[3]; // [esp+5Ch] [ebp-40h] BYREF
    float control; // [esp+68h] [ebp-34h]
    float wishvel[3]; // [esp+6Ch] [ebp-30h]
    float wishspeed; // [esp+78h] [ebp-24h]
    float curFriction; // [esp+7Ch] [ebp-20h]
    float newspeed; // [esp+80h] [ebp-1Ch]
    float drop; // [esp+84h] [ebp-18h]
    float speed; // [esp+88h] [ebp-14h]
    float smove; // [esp+8Ch] [ebp-10h]
    float scale; // [esp+90h] [ebp-Ch]
    playerState_s *ps; // [esp+94h] [ebp-8h]
    float umove; // [esp+98h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 2009, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 2012, 0, "%s", "ps") )
        __debugbreak();
    ps->viewHeightTarget = 60;
    fmove = (float)pm->cmd.forwardmove;
    smove = (float)pm->cmd.rightmove;
    umove = 0.0f;
    if ( pm->cmd.button_bits.testBit(7u) )
        umove = umove + 127.0;
    if ( pm->cmd.button_bits.testBit(6u) )
        umove = umove - 127.0;
    if ( fmove == 0.0 && smove == 0.0 && umove == 0.0 )
        speed = 0.0f;
    else
        speed = Vec3Length(ps->velocity);
    if ( speed >= 1.0 )
    {
        drop = 0.0f;
        curFriction = friction->current.value * 1.5;
        if ( stopspeed->current.value <= speed )
            value = speed;
        else
            value = stopspeed->current.value;
        control = value;
        drop = (float)((float)(value * curFriction) * pml->frametime) + drop;
        newspeed = speed - drop;
        if ( (float)(speed - drop) < 0.0 )
            newspeed = 0.0f;
        newspeed = newspeed / speed;
        velocity = ps->velocity;
        v8 = ps->velocity;
        ps->velocity[0] = newspeed * ps->velocity[0];
        velocity[1] = newspeed * v8[1];
        velocity[2] = newspeed * v8[2];
    }
    else
    {
        v9 = ps->velocity;
        ps->velocity[0] = vec3_origin[0];
        v9[1] = 0.0;
        v9[2] = 0.0;
    }
    scale = PM_MoveScale(ps, fmove, smove, umove);
    up[1] = 0.0f;
    up[0] = 0.0f;
    up[2] = 1.0f;
    Vec3Cross(up, pml->right, forward);
    for ( i = 0; i < 3; ++i )
        wishvel[i] = (float)((float)(forward[i] * fmove) + (float)(pml->right[i] * smove)) + (float)(up[i] * umove);
    wishdir[0] = wishvel[0];
    wishdir[1] = wishvel[1];
    wishdir[2] = wishvel[2];
    wishspeed = Vec3Normalize(wishdir);
    wishspeed = wishspeed * scale;
    PM_Accelerate(ps, pml, wishdir, wishspeed, 9.0);
    origin = ps->origin;
    v4 = ps->velocity;
    frametime = pml->frametime;
    v6 = ps->origin;
    ps->origin[0] = (float)(frametime * ps->velocity[0]) + ps->origin[0];
    origin[1] = (float)(frametime * v4[1]) + v6[1];
    origin[2] = (float)(frametime * v4[2]) + v6[2];
}

void __cdecl PM_GroundTrace(pmove_t *pm, pml_t *pml)
{
    float v2; // xmm0_4
    unsigned __int16 EntityHitId; // ax
    unsigned __int16 v4; // ax
    unsigned int eventParm; // [esp+44h] [ebp-64h]
    int stype; // [esp+48h] [ebp-60h]
    float start[3]; // [esp+4Ch] [ebp-5Ch] BYREF
    trace_t trace; // [esp+58h] [ebp-50h] BYREF
    playerState_s *ps; // [esp+94h] [ebp-14h]
    float point[3]; // [esp+98h] [ebp-10h] BYREF
    const gjkcc_input_t *gjkcc_in; // [esp+A4h] [ebp-4h]
    int savedregs; // [esp+A8h] [ebp+0h] BYREF

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 2548, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 2551, 0, "%s", "ps") )
        __debugbreak();
    ps->groundType = 0;
    start[0] = ps->origin[0];
    start[1] = ps->origin[1];
    point[0] = ps->origin[0];
    point[1] = ps->origin[1];
    if ( (ps->eFlags & 0x300) != 0 )
    {
        start[2] = ps->origin[2];
        v2 = ps->origin[2] - 1.0;
    }
    else
    {
        start[2] = ps->origin[2] + 0.25;
        v2 = ps->origin[2] - 0.25;
    }
    point[2] = v2;
    gjkcc_in = pm->m_gjkcc_input;
    if ( phys_player_collision_mode->current.integer == 1 )
        PM_gjk_ground_trace(
            gjkcc_in,
            &trace,
            start,
            pm->mins,
            pm->maxs,
            point,
            ps->clientNum,
            pm->tracemask,
            0);
    else
        PM_playerTrace(pm, &trace, start, pm->mins, pm->maxs, point, ps->clientNum, pm->tracemask);
    memcpy(&pml->groundTrace, &trace, sizeof(pml->groundTrace));
    ps->groundType = (unsigned __int8)((int)(0x3F00000 & trace.sflags) >> 20);
    if ( (pml->groundTrace.sflags & 2) != 0 )
    {
        ps->pm_flags |= 0x800000;
        if ( !ps->slideTime )
        {
            ps->slideTime = pm->cmd.serverTime;
            BG_AddPredictableEventToPlayerstate(9u, 0, ps);
            if ( pm->handler == 1 )
            {
                stype = Dtp_CalcSurfaceType(pm, pml);
                BG_AddPredictableEventToPlayerstate(0xBCu, stype, ps);
            }
        }
    }
    else
    {
        ps->pm_flags &= ~0x800000u;
        if ( ps->slideTime )
        {
            ps->slideTime = 0;
            BG_AddPredictableEventToPlayerstate(8u, 0, ps);
            if ( pm->handler == 1 )
            {
                eventParm = Dtp_CalcSurfaceType(pm, pml);
                BG_AddPredictableEventToPlayerstate(0xBDu, eventParm, ps);
            }
        }
    }
    if ( !trace.allsolid || PM_CorrectAllSolid(pm, pml, &trace) )
    {
        if ( trace.startsolid )
        {
            start[2] = ps->origin[2] - 0.001;
            if ( phys_player_collision_mode->current.integer == 1 )
                PM_gjk_ground_trace(
                    gjkcc_in,
                    &trace,
                    start,
                    pm->mins,
                    pm->maxs,
                    point,
                    ps->clientNum,
                    pm->tracemask,
                    0);
            else
                PM_playerTrace(pm, &trace, start, pm->mins, pm->maxs, point, ps->clientNum, pm->tracemask);
            if ( trace.startsolid )
            {
                EntityHitId = Trace_GetEntityHitId(&trace);
                ps->groundEntityNum = EntityHitId;
                pml->groundPlane = 1;
                pml->almostGroundPlane = 1;
                pml->walking = trace.walkable;
                return;
            }
            memcpy(&pml->groundTrace, &trace, sizeof(pml->groundTrace));
        }
        if ( trace.fraction == 1.0 )
        {
            PM_GroundTraceMissed(pm, pml);
        }
        else
        {
            if ( trace.normal.vec.v[0] == 0.0
                && trace.normal.vec.v[1] == 0.0
                && trace.normal.vec.v[2] == 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                            2648,
                            0,
                            "%s",
                            "trace.normal[0] || trace.normal[1] || trace.normal[2]") )
            {
                __debugbreak();
            }
            if ( phys_player_collision_mode->current.integer == 1
                || (ps->pm_flags & 8) != 0
                || ps->velocity[2] <= 0.0
                || (float)((float)((float)(ps->velocity[0] * trace.normal.vec.v[0])
                                                 + (float)(ps->velocity[1] * trace.normal.vec.v[1]))
                                 + (float)(ps->velocity[2] * trace.normal.vec.v[2])) <= 10.0
                || ps->groundEntityNum != 1023 && ps->groundEntityNum != 1022 )
            {
                if ( trace.walkable )
                {
                    pml->groundPlane = 1;
                    pml->almostGroundPlane = 1;
                    pml->walking = 1;
                    if ( ps->groundEntityNum == 1023 )
                        PM_CrashLand(pm, pml);
                    v4 = Trace_GetEntityHitId(&trace);
                    ps->groundEntityNum = v4;
                    PM_AddTouchEnt(pm, ps->groundEntityNum);
                }
                else
                {
                    ps->groundEntityNum = 1023;
                    pml->groundPlane = 1;
                    pml->almostGroundPlane = 1;
                    pml->walking = 0;
                    Jump_ClearState(ps);
                }
            }
            else
            {
                point[2] = point[2] - 32.0;
                trace_t airTrace; // [esp+Ch] [ebp-9Ch] BYREF
                if ( phys_player_collision_mode->current.integer == 1 )
                    PM_gjk_ground_trace(
                        gjkcc_in,
                        &airTrace,
                        start,
                        pm->mins,
                        pm->maxs,
                        point,
                        ps->clientNum,
                        pm->tracemask,
                        0);
                else
                    PM_playerTrace(pm, &airTrace, start, pm->mins, pm->maxs, point, ps->clientNum, pm->tracemask);
                if ( airTrace.fraction == 1.0 && (ps->pm_flags & 0x400000) == 0 )
                    BG_AnimScriptEvent(pm, ANIM_ET_JUMP, 0, 0);
                pml->almostGroundPlane = 0;
                ps->groundEntityNum = 1023;
                pml->groundPlane = 0;
                pml->walking = 0;
            }
        }
    }
}

void __cdecl PM_CrashLand(pmove_t *pm, pml_t *pml)
{
    unsigned int v2; // eax
    unsigned int v3; // eax
    unsigned int v4; // eax
    unsigned int v5; // [esp+0h] [ebp-78h]
    unsigned int v6; // [esp+0h] [ebp-78h]
    unsigned int v7; // [esp+0h] [ebp-78h]
    playerState_s *v8; // [esp+4h] [ebp-74h]
    playerState_s *v9; // [esp+4h] [ebp-74h]
    playerState_s *v10; // [esp+4h] [ebp-74h]
    int v11; // [esp+14h] [ebp-64h]
    float *v12; // [esp+18h] [ebp-60h]
    float *v13; // [esp+1Ch] [ebp-5Ch]
    float *velocity; // [esp+20h] [ebp-58h]
    float *v15; // [esp+24h] [ebp-54h]
    float *v16; // [esp+28h] [ebp-50h]
    float *v17; // [esp+2Ch] [ebp-4Ch]
    int v18; // [esp+34h] [ebp-44h]
    int damage; // [esp+38h] [ebp-40h] BYREF
    float fallHeight; // [esp+3Ch] [ebp-3Ch]
    float t; // [esp+40h] [ebp-38h]
    float vel; // [esp+44h] [ebp-34h]
    float c; // [esp+48h] [ebp-30h]
    float dist; // [esp+4Ch] [ebp-2Ch]
    float landVel; // [esp+50h] [ebp-28h]
    int viewDip; // [esp+54h] [ebp-24h]
    int stunTime; // [esp+58h] [ebp-20h]
    float b; // [esp+5Ch] [ebp-1Ch]
    float acc; // [esp+60h] [ebp-18h]
    int surfaceType; // [esp+64h] [ebp-14h]
    float a; // [esp+68h] [ebp-10h]
    playerState_s *ps; // [esp+6Ch] [ebp-Ch]
    float den; // [esp+70h] [ebp-8h]
    float fSpeedMult; // [esp+74h] [ebp-4h]

    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 2268, 0, "%s", "ps") )
        __debugbreak();
    if ( ps->waterlevel != 3 )
    {
        dist = pml->previous_origin[2] - ps->origin[2];
        vel = pml->previous_velocity[2];
        //LODWORD(acc) = COERCE_UNSIGNED_INT((float)ps->gravity) ^ _mask__NegFloat_;
        acc = -ps->gravity;
        a = acc * 0.5;
        b = vel;
        c = dist;
        den = (float)(vel * vel) - (float)((float)(4.0 * (float)(acc * 0.5)) * dist);
        if ( den >= 0.0 )
        {
            //t = (float)(COERCE_FLOAT(LODWORD(b) ^ _mask__NegFloat_) - sqrtf(den)) / (float)(2.0 * a);
            t = (float)((-(b)) - sqrtf(den)) / (float)(2.0 * a);
            landVel = (float)((float)(t * acc) + vel) * -1.0;
            fallHeight = (float)(landVel * landVel) / (float)((float)ps->gravity * 2.0);
            if ( bg_fallDamageMinHeight->current.value <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                            2292,
                            0,
                            "%s",
                            "bg_fallDamageMinHeight->current.value > 0") )
            {
                __debugbreak();
            }
            if ( bg_fallDamageMaxHeight->current.value <= bg_fallDamageMinHeight->current.value
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                            2293,
                            0,
                            "%s",
                            "bg_fallDamageMinHeight->current.value < bg_fallDamageMaxHeight->current.value") )
            {
                __debugbreak();
            }
            if ( bg_fallDamageMinHeight->current.value < fallHeight )
            {
                if ( fallHeight < bg_fallDamageMaxHeight->current.value )
                {
                    damage = (int)(float)((float)((float)(fallHeight - bg_fallDamageMinHeight->current.value)
                                                                            / (float)(bg_fallDamageMaxHeight->current.value
                                                                                            - bg_fallDamageMinHeight->current.value))
                                                            * 100.0);
                    if ( damage < 100 )
                        v18 = damage;
                    else
                        v18 = 100;
                    if ( v18 > 0 )
                        v11 = v18;
                    else
                        v11 = 0;
                    damage = v11;
                }
                else
                {
                    damage = 100;
                }
            }
            else
            {
                damage = 0;
            }
            Dtp_AdjustDamage(ps, fallHeight, &damage);
            if ( (ps->perks[0] & 0x2000) != 0 )
                damage = 0;
            if ( (pml->groundTrace.sflags & 1) != 0 || ps->pm_type >= 9 )
                damage = 0;
            if ( ps->waterlevel == 2 )
                damage = (int)(float)((float)damage * 0.5);
            if ( fallHeight > 12.0 )
            {
                viewDip = (int)(float)((float)((float)((float)(fallHeight - 12.0) / 26.0) * 4.0) + 4.0);
                if ( viewDip > 24 )
                    viewDip = 24;
                if ( (ps->pm_flags & 0x400000) == 0 )
                    BG_AnimScriptEvent(pm, ANIM_ET_LAND, 0, 1);
            }
            else
            {
                viewDip = 0;
            }
            surfaceType = PM_GroundSurfaceType(ps, pml);
            if ( damage )
            {
                if ( damage >= 100 || (pml->groundTrace.sflags & 2) != 0 )
                {
                    velocity = ps->velocity;
                    v15 = ps->velocity;
                    ps->velocity[0] = 0.67000002 * ps->velocity[0];
                    velocity[1] = 0.67000002 * v15[1];
                    velocity[2] = 0.67000002 * v15[2];
                }
                else
                {
                    stunTime = 35 * damage + 500;
                    if ( stunTime > 2000 )
                        stunTime = 2000;
                    if ( stunTime > 500 )
                    {
                        if ( stunTime < 1500 )
                            fSpeedMult = 0.5 - (float)((float)((float)((float)stunTime - 500.0) / 1000.0) * 0.30000001);
                        else
                            fSpeedMult = 0.2f;
                    }
                    else
                    {
                        fSpeedMult = 0.5f;
                    }
                    ps->pm_time = stunTime;
                    ps->pm_flags |= 0x80u;
                    v16 = ps->velocity;
                    v17 = ps->velocity;
                    ps->velocity[0] = fSpeedMult * ps->velocity[0];
                    v16[1] = fSpeedMult * v17[1];
                    v16[2] = fSpeedMult * v17[2];
                }
                v8 = ps;
                v5 = damage;
                v2 = PM_DamageLandingForSurface(ps, pml);
                BG_AddPredictableEventToPlayerstate(v2, v5, v8);
            }
            else if ( fallHeight > 4.0 )
            {
                if ( fallHeight >= 8.0 )
                {
                    if ( fallHeight >= 12.0 )
                    {
                        v12 = ps->velocity;
                        v13 = ps->velocity;
                        ps->velocity[0] = 0.67000002 * ps->velocity[0];
                        v12[1] = 0.67000002 * v13[1];
                        v12[2] = 0.67000002 * v13[2];
                        v10 = ps;
                        v7 = viewDip;
                        v4 = PM_HardLandingForSurface(ps, pml);
                    }
                    else
                    {
                        v10 = ps;
                        v7 = surfaceType;
                        v4 = PM_MediumLandingForSurface();
                    }
                    BG_AddPredictableEventToPlayerstate(v4, v7, v10);
                }
                else
                {
                    v9 = ps;
                    v6 = surfaceType;
                    v3 = PM_LightLandingForSurface();
                    BG_AddPredictableEventToPlayerstate(v3, v6, v9);
                }
            }
        }
    }
}

int __cdecl PM_LightLandingForSurface()
{
    return 105;
}

int __cdecl PM_MediumLandingForSurface()
{
    return 104;
}

unsigned int __cdecl PM_HardLandingForSurface(playerState_s *ps, pml_t *pml)
{
    return PM_GroundSurfaceType(ps, pml) + 111;
}

unsigned int __cdecl PM_DamageLandingForSurface(playerState_s *ps, pml_t *pml)
{
    unsigned int iSurfType; // [esp+18h] [ebp-4h]

    iSurfType = PM_GroundSurfaceType(ps, pml);
    if ( !iSurfType )
        Com_PrintError(
            1,
            "PM_DamageLandingForSurface has been called with a ground surface of type 'NONE' near (%.2f %.2f %.2f). \n"
            "This means a player has landed on a surface that wasn't properly setup to be used as a ground surface. \n"
            "Use a different material which has a surface type set.\n",
            pml->previous_origin[0],
            pml->previous_origin[1],
            pml->previous_origin[2]);
    return iSurfType + 142;
}

int __cdecl PM_CorrectAllSolid(pmove_t *pm, pml_t *pml, trace_t *trace)
{
    unsigned int i; // [esp+1Ch] [ebp-14h]
    playerState_s *ps; // [esp+20h] [ebp-10h]
    float point[3]; // [esp+24h] [ebp-Ch] BYREF

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 2436, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 2439, 0, "%s", "ps") )
        __debugbreak();
    for ( i = 0; i < 0x1A; ++i )
    {
        point[0] = ps->origin[0] + (float)CorrectSolidDeltas[i][0];
        point[1] = ps->origin[1] + (float)CorrectSolidDeltas[i][1];
        point[2] = ps->origin[2] + (float)CorrectSolidDeltas[i][2];
        PM_playerTrace(pm, trace, point, pm->mins, pm->maxs, point, ps->clientNum, pm->tracemask);
        if ( !trace->startsolid )
        {
            ps->origin[0] = point[0];
            ps->origin[1] = point[1];
            ps->origin[2] = point[2];
            point[2] = (float)(ps->origin[2] - 1.0) - 0.25;
            PM_playerTrace(pm, trace, ps->origin, pm->mins, pm->maxs, point, ps->clientNum, pm->tracemask);
            memcpy(&pml->groundTrace, trace, sizeof(pml->groundTrace));
            Vec3Lerp(ps->origin, point, trace->fraction, ps->origin);
            return 1;
        }
    }
    ps->groundEntityNum = Trace_GetEntityHitId(trace);
    pml->groundPlane = 0;
    pml->almostGroundPlane = 0;
    pml->walking = 0;
    Jump_ClearState(ps);
    return 0;
}

void __cdecl PM_GroundTraceMissed(pmove_t *pm, pml_t *pml)
{
    trace_t trace; // [esp+10h] [ebp-4Ch] BYREF
    playerState_s *ps; // [esp+4Ch] [ebp-10h]
    float point[3]; // [esp+50h] [ebp-Ch] BYREF

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 2485, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 2488, 0, "%s", "ps") )
        __debugbreak();
    if ( ps->groundEntityNum == 1023 )
    {
        point[0] = ps->origin[0];
        point[1] = ps->origin[1];
        point[2] = ps->origin[2];
        point[2] = point[2] - 1.0;
        PM_playerTrace(pm, &trace, ps->origin, pm->mins, pm->maxs, point, ps->clientNum, pm->tracemask);
        pml->almostGroundPlane = trace.fraction != 1.0;
    }
    else
    {
        point[0] = ps->origin[0];
        point[1] = ps->origin[1];
        point[2] = ps->origin[2];
        point[2] = point[2] - 64.0;
        PM_playerTrace(pm, &trace, ps->origin, pm->mins, pm->maxs, point, ps->clientNum, pm->tracemask);
        if ( trace.fraction == 1.0 )
        {
            if ( (ps->pm_flags & 0x400000) == 0 )
                BG_AnimScriptEvent(pm, ANIM_ET_JUMP, 0, 1);
            pml->almostGroundPlane = 0;
        }
        else
        {
            pml->almostGroundPlane = trace.fraction < 0.015625;
            if ( !pml->almostGroundPlane && (ps->pm_flags & 0x400000) == 0 )
                BG_AnimScriptEvent(pm, ANIM_ET_JUMP, 0, 1);
        }
    }
    ps->groundEntityNum = 1023;
    pml->groundPlane = 0;
    pml->walking = 0;
}

void __cdecl PM_SetWaterLevel(pmove_t *pm)
{
    float waterHeight; // [esp+8h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 2749, 0, "%s", "pm") )
        __debugbreak();
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 2750, 0, "%s", "pm->ps") )
        __debugbreak();
    waterHeight = CM_GetWaterHeight(pm->ps->origin, 200.0, -200.0);
    pm->ps->waterlevel = PM_GetWaterLevel(pm->ps, waterHeight);
}

double __cdecl PM_GetViewHeightLerp(const pmove_t *pm, int iFromHeight, int iToHeight)
{
    float fLerpFrac; // [esp+4h] [ebp-8h]
    playerState_s *ps; // [esp+8h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps->viewHeightLerpTime )
        return 0.0;
    if ( iFromHeight != -1
        && iToHeight != -1
        && (iToHeight != ps->viewHeightLerpTarget
         || iToHeight == 40
         && (iFromHeight != 11 || ps->viewHeightLerpDown)
         && (iFromHeight != 60 || !ps->viewHeightLerpDown)) )
    {
        return 0.0;
    }
    fLerpFrac = (float)(pm->cmd.serverTime - ps->viewHeightLerpTime)
                        / (float)PM_GetViewHeightLerpTime(ps, ps->viewHeightLerpTarget, ps->viewHeightLerpDown);
    if ( fLerpFrac >= 0.0 )
    {
        if ( fLerpFrac > 1.0 )
            return 1.0f;
    }
    else
    {
        return 0.0f;
    }
    return fLerpFrac;
}

bool __cdecl PM_IsPlayerFrozenByWeapon(const playerState_s *ps)
{
    return (ps->weaponstate == 6
             || ps->weaponstate == 7
             || ps->weaponstate == 31
             || ps->weaponstate == 17
             || ps->weaponstate == 18
             || ps->weaponstate == 19)
            && ps->weapon
            && BG_GetWeaponDef(ps->weapon)->freezeMovementWhenFiring;
}

void __cdecl PM_CheckDuck(pmove_t *pm, pml_t *pml)
{
    double v2; // st7
    col_context_t v3; // [esp+168h] [ebp-C8h] BYREF
    float delta; // [esp+190h] [ebp-A0h]
    float vPoint[3]; // [esp+194h] [ebp-9Ch] BYREF
    float vEnd[3]; // [esp+1A0h] [ebp-90h] BYREF
    float v7; // [esp+1ACh] [ebp-84h]
    bool v8; // [esp+1B3h] [ebp-7Dh]
    float save_maxs2; // [esp+1B4h] [ebp-7Ch]
    bool not_pen; // [esp+1BBh] [ebp-75h]
    col_context_t context; // [esp+1BCh] [ebp-74h] BYREF
    int iStance; // [esp+1E4h] [ebp-4Ch]
    int bWasProne; // [esp+1E8h] [ebp-48h]
    int bWasStanding; // [esp+1ECh] [ebp-44h]
    trace_t trace; // [esp+1F0h] [ebp-40h] BYREF
    playerState_s *ps; // [esp+22Ch] [ebp-4h]

    //col_context_t::col_context_t(&context);
    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 3168, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 3171, 0, "%s", "ps") )
        __debugbreak();
    pm->proneChange = 0;
    if ( ps->pm_type == 4 )
    {
        pm->mins[0] = -8.0f;
        pm->mins[1] = -8.0f;
        pm->mins[2] = -8.0f;
        pm->maxs[0] = 8.0f;
        pm->maxs[1] = 8.0f;
        pm->maxs[2] = 16.0f;
        ps->pm_flags &= 0xFFFFFFFC;
        if ( pm->cmd.button_bits.testBit(8u) )
        {
            //pm->cmd.button_bits.resetBit(8u);
            pm->cmd.button_bits.resetBit(8u);
            BG_AddPredictableEventToPlayerstate(8u, 0, ps);
        }
        ps->viewHeightTarget = 0;
        ps->viewHeightCurrent = 0.0f;
        return;
    }
    bWasProne = (ps->pm_flags & 1) != 0;
    bWasStanding = (ps->pm_flags & 3) == 0;
    pm->mins[0] = playerMins[0];
    pm->mins[1] = -15.0;
    pm->mins[2] = 0.0;
    pm->maxs[0] = playerMaxs[0];
    pm->maxs[1] = 15.0;
    pm->maxs[2] = 70.0;
    if ( ps->pm_type == 9 )
    {
        ps->viewHeightTarget = 8;
        if ( (ps->pm_flags & 2) != 0 )
        {
            pm->maxs[2] = 50.0f;
        }
        else if ( (ps->pm_flags & 1) != 0 )
        {
            pm->maxs[2] = 30.0f;
        }
LABEL_25:
        PM_ViewHeightAdjust(pm, pml);
        return;
    }
    if ( bWasProne && ps->waterlevel >= 1 )
    {
        ps->viewHeightTarget = 40;
        BG_AddPredictableEventToPlayerstate(9u, 0, ps);
        if ( ps->pm_type != 8 && ps->pm_type != 6 && ps->pm_type != 7 )
            BG_AnimScriptEvent(pm, ANIM_ET_PRONE_TO_CROUCH, 0, 1);
        ps->pm_flags &= ~1u;
        ps->pm_flags |= 0x1000u;
        if ( ps->waterlevel >= 3 )
        {
            ps->viewHeightTarget = 60;
            ps->pm_flags &= 0xFFFFFFFC;
            BG_AddPredictableEventToPlayerstate(8u, 0, ps);
        }
        goto LABEL_25;
    }
    if ( (ps->pm_flags & 3) != 0 && ((ps->eFlags & 0x4000) != 0 || ps->waterlevel >= 2) )
    {
        ps->viewHeightTarget = 60;
        ps->pm_flags &= 0xFFFFFFFC;
        BG_AddPredictableEventToPlayerstate(8u, 0, ps);
        PM_ViewHeightAdjust(pm, pml);
    }
    else if ( (ps->pm_flags & 0x8000) != 0 )
    {
        if ( (ps->pm_flags & 1) != 0 )
            BG_AnimScriptEvent(pm, ANIM_ET_PRONE_TO_SPRINT, 0, 1);
        ps->viewHeightTarget = 60;
        ps->eFlags &= 0xFFFFFFF3;
        ps->pm_flags &= 0xFFFFFFFC;
        BG_AddPredictableEventToPlayerstate(8u, 0, ps);
        PM_ViewHeightAdjust(pm, pml);
    }
    else
    {
        if ( (ps->eFlags & 0x300) != 0 )
        {
            if ( (ps->eFlags & 0x100) == 0 || (ps->eFlags & 0x200) != 0 )
            {
                if ( (ps->eFlags & 0x200) == 0 || (ps->eFlags & 0x100) != 0 )
                {
                    ps->pm_flags &= 0xFFFFFFFC;
                }
                else
                {
                    ps->pm_flags |= 2u;
                    ps->pm_flags &= ~1u;
                }
            }
            else
            {
                ps->pm_flags |= 1u;
                ps->pm_flags &= ~2u;
            }
        }
        else if ( (ps->eFlags & 0x4000) != 0 )
        {
            ps->pm_flags &= 0xFFFFFFFC;
        }
        else if ( (ps->pm_flags & 0xC00) == 0 && !PM_IsPlayerFrozenByWeapon(ps) && ps->weaponstate != 35 )
        {
            if ( ps->pm_type == 6 || ps->pm_type == 7 )
            {
                ps->pm_flags &= ~1u;
                ps->pm_flags |= 2u;
            }
            else if ( (ps->pm_flags & 0x400000) == 0 && Dtp_CanMove(pm) )
            {
                if ( (ps->pm_flags & 8) != 0
                    && (pm->cmd.button_bits.testBit(8u) || pm->cmd.button_bits.testBit(9u)) )
                {
                    pm->cmd.button_bits.resetBit(8u);
                    pm->cmd.button_bits.resetBit(9u);
                    BG_AddPredictableEventToPlayerstate(8u, 0, ps);
                }
                if ( !pm->cmd.button_bits.testBit(8u) || (ps->pm_flags & 0x400) != 0 )
                {
                    if ( pm->cmd.button_bits.testBit(9u) )
                    {
                        if ( pm->ps->waterlevel >= 2 )
                        {
                            BG_AddPredictableEventToPlayerstate(8u, 0, ps);
                        }
                        else if ( (ps->pm_flags & 1) != 0 )
                        {
                            save_maxs2 = pm->maxs[2];
                            pm->maxs[2] = 50.0f;
                            not_pen = is_not_penetrating(pm, ps->origin, pm->mins, pm->maxs, ps->origin, pm->tracemask & 0xFDFF7FFF);
                            pm->maxs[2] = save_maxs2;
                            if ( not_pen )
                            {
                                if ( ps->pm_type != 8 && ps->pm_type != 6 && ps->pm_type != 7 && ps->groundEntityNum != 1023 )
                                    BG_AnimScriptEvent(pm, ANIM_ET_PRONE_TO_CROUCH, 0, 1);
                                ps->pm_flags &= ~1u;
                                ps->pm_flags |= 2u;
                            }
                            else if ( !pm->cmd.button_bits.testBit(0xCu) )
                            {
                                BG_AddPredictableEventToPlayerstate(0xAu, 2u, ps);
                            }
                        }
                        else if ( (ps->eFlags & 0x4000) == 0 )
                        {
                            if ( (ps->pm_flags & 2) == 0 && bg_playStandToCrouchAnims->current.enabled )
                                BG_AnimScriptEvent(pm, ANIM_ET_STAND_TO_CROUCH, 0, 0);
                            ps->pm_flags |= 2u;
                        }
                    }
                    else if ( (ps->pm_flags & 1) != 0 )
                    {
                        if ( is_not_penetrating(pm, ps->origin, pm->mins, pm->maxs, ps->origin, pm->tracemask & 0xFDFF7FFF) )
                        {
                            BG_AnimScriptEvent(pm, ANIM_ET_PRONE_TO_STAND, 0, 0);
                            ps->pm_flags &= 0xFFFFFFFC;
                        }
                        else
                        {
                            v7 = pm->maxs[2];
                            pm->maxs[2] = 50.0f;
                            v8 = is_not_penetrating(pm, ps->origin, pm->mins, pm->maxs, ps->origin, pm->tracemask & 0xFDFF7FFF);
                            pm->maxs[2] = v7;
                            if ( v8 )
                            {
                                if ( ps->pm_type != 8 && ps->pm_type != 6 && ps->pm_type != 7 )
                                    BG_AnimScriptEvent(pm, ANIM_ET_PRONE_TO_CROUCH, 0, 1);
                                ps->pm_flags &= ~1u;
                                ps->pm_flags |= 2u;
                            }
                            else if ( !pm->cmd.button_bits.testBit(0xCu) )
                            {
                                BG_AddPredictableEventToPlayerstate(0xAu, 1u, ps);
                            }
                        }
                    }
                    else if ( (ps->pm_flags & 2) != 0 )
                    {
                        if ( is_not_penetrating(pm, ps->origin, pm->mins, pm->maxs, ps->origin, pm->tracemask & 0xFDFF7FFF) )
                        {
                            if ( bg_playStandToCrouchAnims->current.enabled )
                                BG_AnimScriptEvent(pm, ANIM_ET_CROUCH_TO_STAND, 0, 0);
                            ps->pm_flags &= ~2u;
                        }
                        else if ( !pm->cmd.button_bits.testBit(0xCu) )
                        {
                            BG_AddPredictableEventToPlayerstate(9u, 1u, ps);
                        }
                    }
                }
                else if ( PlayerProneAllowed(pm) )
                {
                    ps->pm_flags |= 1u;
                    ps->pm_flags &= ~2u;
                }
                else if ( ps->groundEntityNum != 1023 )
                {
                    ps->pm_flags |= 0x1000u;
                    BG_AddPredictableEventToPlayerstate(0xCDu, 3u, ps);
                    if ( !pm->cmd.button_bits.testBit(0xCu) )
                    {
                        if ( (ps->pm_flags & 1) != 0 || (ps->pm_flags & 2) != 0 )
                            BG_AddPredictableEventToPlayerstate(9u, 0, ps);
                        else
                            BG_AddPredictableEventToPlayerstate(8u, 0, ps);
                    }
                }
            }
            else
            {
                pm->cmd.button_bits.setBit(8u);
                pm->cmd.button_bits.resetBit(9u);
                pm->cmd.button_bits.resetBit(1u);
                ps->eFlags &= ~4u;
                ps->pm_flags &= ~2u;
                ps->viewHeightTarget = 11;
                BG_AddPredictableEventToPlayerstate(0xAu, 0, ps);
            }
        }
        if ( !ps->viewHeightLerpTime )
        {
            if ( ps->pm_type == 6 || ps->pm_type == 7 )
            {
                switch ( ps->viewHeightTarget )
                {
                    case 0x3C:
                        BG_AnimScriptEvent(pm, ANIM_ET_STAND_TO_LASTSTAND, 0, 1);
                        break;
                    case 0x28:
                        BG_AnimScriptEvent(pm, ANIM_ET_CROUCH_TO_LASTSTAND, 0, 1);
                        break;
                    case 0xB:
                        BG_AnimScriptEvent(pm, ANIM_ET_PRONE_TO_LASTSTAND, 0, 1);
                        break;
                }
                ps->viewHeightTarget = 22;
            }
            else if ( (ps->pm_flags & 1) != 0 )
            {
                if ( ps->viewHeightTarget == 60 )
                {
                    ps->viewHeightTarget = 40;
                }
                else if ( ps->viewHeightTarget != 11 )
                {
                    ps->viewHeightTarget = 11;
                    pm->proneChange = 1;
                    BG_PlayAnim(ps, 0, ANIM_BP_TORSO, 0, 0, 1, 1);
                    Jump_ActivateSlowdown(ps);
                }
            }
            else if ( ps->viewHeightTarget == 11 )
            {
                ps->viewHeightTarget = 40;
                pm->proneChange = 1;
                BG_PlayAnim(ps, 0, ANIM_BP_TORSO, 0, 0, 1, 1);
            }
            else if ( (ps->pm_flags & 2) != 0 )
            {
                ps->viewHeightTarget = 40;
            }
            else
            {
                ps->viewHeightTarget = 60;
            }
        }
        PM_ViewHeightAdjust(pm, pml);
        iStance = PM_GetEffectiveStance(ps);
        if ( iStance == 1 )
        {
            pm->maxs[2] = 30.0f;
            ps->eFlags |= 8u;
            ps->eFlags &= ~4u;
            ps->pm_flags |= 1u;
            ps->pm_flags &= ~2u;
        }
        else if ( iStance == 2 )
        {
            pm->maxs[2] = 50.0f;
            ps->eFlags |= 4u;
            ps->eFlags &= ~8u;
            ps->pm_flags |= 2u;
            ps->pm_flags &= ~1u;
        }
        else
        {
            pm->maxs[2] = 70.0;
            ps->eFlags &= 0xFFFFFFF3;
            ps->pm_flags &= 0xFFFFFFFC;
        }
        if ( (ps->pm_flags & 0x400000) != 0 )
        {
            if ( ps->viewHeightCurrent >= 30.0 )
            {
                if ( ps->viewHeightCurrent >= 50.0 )
                    pm->maxs[2] = 70.0;
                else
                    pm->maxs[2] = 50.0f;
            }
            else
            {
                pm->maxs[2] = 30.0f;
            }
        }
        if ( (ps->pm_flags & 1) != 0 )
        {
            if ( !bWasProne )
            {
                //col_context_t::col_context_t(&v3);
                if ( pm->cmd.forwardmove || pm->cmd.rightmove )
                {
                    ps->pm_flags &= ~0x200u;
                    PM_ExitAimDownSight(ps);
                }
                vEnd[0] = ps->origin[0];
                vEnd[1] = ps->origin[1];
                vEnd[2] = ps->origin[2];
                vEnd[2] = vEnd[2] + 10.0;
                //colgeom_visitor_inlined_t<200>::update(
                    pm->proximity_data.update(
                    ps->origin,
                    vEnd,
                    pm->mins,
                    pm->maxs,
                    0x3818813);
                v3.prims = pm->proximity_data.prims;
                v3.nprims = pm->proximity_data.nprims;
                pmoveHandlers[pm->handler].trace(
                    &trace,
                    ps->origin,
                    pm->mins,
                    pm->maxs,
                    vEnd,
                    ps->clientNum,
                    pm->tracemask & 0xFDFF7FFF,
                    &v3);
                Vec3Lerp(ps->origin, vEnd, trace.fraction, vEnd);
                //colgeom_visitor_inlined_t<200>::update(
                    pm->proximity_data.update(
                    vEnd,
                    ps->origin,
                    pm->mins,
                    pm->maxs,
                    0x3818813);
                v3.prims = pm->proximity_data.prims;
                v3.nprims = pm->proximity_data.nprims;
                pmoveHandlers[pm->handler].trace(
                    &trace,
                    vEnd,
                    pm->mins,
                    pm->maxs,
                    ps->origin,
                    ps->clientNum,
                    pm->tracemask & 0xFDFF7FFF,
                    &v3);
                Vec3Lerp(vEnd, ps->origin, trace.fraction, ps->origin);
                ps->proneDirection = ps->viewangles[1];
                vPoint[0] = ps->origin[0];
                vPoint[1] = ps->origin[1];
                vPoint[2] = ps->origin[2];
                vPoint[2] = vPoint[2] - 0.25;
                //colgeom_visitor_inlined_t<200>::update(
                    pm->proximity_data.update(
                    ps->origin,
                    vPoint,
                    pm->mins,
                    pm->maxs,
                    0x3818813);
                v3.prims = pm->proximity_data.prims;
                v3.nprims = pm->proximity_data.nprims;
                pmoveHandlers[pm->handler].trace(
                    &trace,
                    ps->origin,
                    pm->mins,
                    pm->maxs,
                    vPoint,
                    ps->clientNum,
                    pm->tracemask & 0xFDFF7FFF,
                    &v3);
                if ( trace.startsolid || trace.fraction >= 1.0 || ps->pm_type == 6 )
                {
                    ps->proneDirectionPitch = 0.0f;
                }
                else
                {
                    if ( trace.normal.vec.v[0] == 0.0
                        && trace.normal.vec.v[1] == 0.0
                        && trace.normal.vec.v[2] == 0.0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                                    3718,
                                    0,
                                    "%s",
                                    "trace.normal[0] || trace.normal[1] || trace.normal[2]") )
                    {
                        __debugbreak();
                    }
                    v2 = PitchForYawOnNormal(ps->proneDirection, trace.normal.vec.v);
                    ps->proneDirectionPitch = v2;
                }
                delta = AngleDelta(ps->proneDirectionPitch, ps->viewangles[0]);
                if ( delta >= -45.0 )
                {
                    if ( delta <= 45.0 )
                        ps->proneTorsoPitch = ps->proneDirectionPitch;
                    else
                        ps->proneTorsoPitch = ps->viewangles[0] + 45.0;
                }
                else
                {
                    ps->proneTorsoPitch = ps->viewangles[0] - 45.0;
                }
            }
            if ( ps->pm_type == 6 )
                ps->proneDirection = ps->viewangles[1];
        }
    }
}

void __cdecl PM_ViewHeightAdjust(pmove_t *pm, pml_t *pml)
{
    double v2; // st7
    double v3; // st7
    double v4; // st7
    const char *v5; // eax
    float fNewPosOfs; // [esp+0h] [ebp-10h] BYREF
    int iLerpFrac; // [esp+4h] [ebp-Ch]
    int iLerpTime; // [esp+8h] [ebp-8h]
    playerState_s *ps; // [esp+Ch] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 2908, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 2911, 0, "%s", "ps") )
        __debugbreak();
    if ( ps->viewHeightTarget && ps->viewHeightCurrent != 0.0 )
    {
        if ( ps->viewHeightCurrent != (float)ps->viewHeightTarget || ps->viewHeightLerpTime )
        {
            iLerpFrac = 0;
            fNewPosOfs = 0.0f;
            if ( ps->viewHeightTarget == 11 || ps->viewHeightTarget == 40 || ps->viewHeightTarget == 60 )
            {
                if ( ps->viewHeightLerpTime )
                {
                    iLerpTime = PM_GetViewHeightLerpTime(ps, ps->viewHeightLerpTarget, ps->viewHeightLerpDown);
                    iLerpFrac = 100 * (pm->cmd.serverTime - ps->viewHeightLerpTime) / iLerpTime;
                    if ( iLerpFrac >= 0 )
                    {
                        if ( iLerpFrac > 100 )
                            iLerpFrac = 100;
                    }
                    else
                    {
                        iLerpFrac = 0;
                    }
                    if ( iLerpFrac == 100 )
                    {
                        if ( ps->pm_type == 8 )
                            ps->pm_type = 0;
                        ps->viewHeightCurrent = (float)ps->viewHeightLerpTarget;
                        ps->viewHeightLerpTime = 0;
                    }
                    else
                    {
                        if ( ps->viewHeightLerpTarget == 11 )
                        {
                            v2 = PM_ViewHeightTableLerp(iLerpFrac, viewLerp_CrouchProne, &fNewPosOfs);
                            ps->viewHeightCurrent = v2;
                        }
                        else if ( ps->viewHeightLerpTarget == 40 )
                        {
                            if ( ps->viewHeightLerpDown )
                                v3 = PM_ViewHeightTableLerp(iLerpFrac, viewLerp_StandCrouch, &fNewPosOfs);
                            else
                                v3 = PM_ViewHeightTableLerp(iLerpFrac, viewLerp_ProneCrouch, &fNewPosOfs);
                            ps->viewHeightCurrent = v3;
                        }
                        else
                        {
                            v4 = PM_ViewHeightTableLerp(iLerpFrac, viewLerp_CrouchStand, &fNewPosOfs);
                            ps->viewHeightCurrent = v4;
                        }
                        if ( fNewPosOfs != 0.0
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                                        2988,
                                        0,
                                        "Wow, someone added view height offsets back in!    You need to restore this code and the viewHeightLerpPosAdj field") )
                        {
                            __debugbreak();
                        }
                    }
                }
                if ( ps->viewHeightLerpTime )
                {
                    if ( ps->viewHeightTarget != ps->viewHeightLerpTarget
                        && (ps->viewHeightTarget < ps->viewHeightLerpTarget && !ps->viewHeightLerpDown
                         || ps->viewHeightTarget > ps->viewHeightLerpTarget && ps->viewHeightLerpDown) )
                    {
                        iLerpFrac = 100 - iLerpFrac;
                        ps->viewHeightLerpDown ^= 1u;
                        if ( ps->viewHeightLerpDown )
                        {
                            if ( ps->viewHeightLerpTarget == 60 )
                            {
                                ps->viewHeightLerpTarget = 40;
                            }
                            else if ( ps->viewHeightLerpTarget == 40 )
                            {
                                ps->viewHeightLerpTarget = 11;
                            }
                        }
                        else if ( ps->viewHeightLerpTarget == 11 )
                        {
                            ps->viewHeightLerpTarget = 40;
                        }
                        else if ( ps->viewHeightLerpTarget == 40 )
                        {
                            ps->viewHeightLerpTarget = 60;
                        }
                        if ( iLerpFrac == 100 )
                        {
                            ps->viewHeightCurrent = (float)ps->viewHeightLerpTarget;
                            ps->viewHeightLerpTime = 0;
                        }
                        else
                        {
                            iLerpTime = PM_GetViewHeightLerpTime(ps, ps->viewHeightLerpTarget, ps->viewHeightLerpDown);
                            ps->viewHeightLerpTime = pm->cmd.serverTime
                                                                         - (int)(float)((float)((float)iLerpFrac * 0.0099999998) * (float)iLerpTime);
                            if ( ps->viewHeightLerpTarget == 11 )
                            {
                                PM_ViewHeightTableLerp(iLerpFrac, viewLerp_CrouchProne, &fNewPosOfs);
                            }
                            else if ( ps->viewHeightLerpTarget == 40 )
                            {
                                if ( ps->viewHeightLerpDown )
                                    PM_ViewHeightTableLerp(iLerpFrac, viewLerp_StandCrouch, &fNewPosOfs);
                                else
                                    PM_ViewHeightTableLerp(iLerpFrac, viewLerp_ProneCrouch, &fNewPosOfs);
                            }
                            else
                            {
                                PM_ViewHeightTableLerp(iLerpFrac, viewLerp_CrouchStand, &fNewPosOfs);
                            }
                            if ( fNewPosOfs != 0.0
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                                            3059,
                                            0,
                                            "Wow, someone added view height offsets back in!    You need to restore the viewHeightLerpPosAdj field back in!") )
                            {
                                __debugbreak();
                            }
                        }
                    }
                }
                else if ( ps->viewHeightCurrent != (float)ps->viewHeightTarget )
                {
                    ps->viewHeightLerpTime = pm->cmd.serverTime;
                    switch ( ps->viewHeightTarget )
                    {
                        case 0xB:
                            ps->viewHeightLerpDown = 1;
                            if ( ps->viewHeightCurrent <= 40.0 )
                                ps->viewHeightLerpTarget = 11;
                            else
                                ps->viewHeightLerpTarget = 40;
                            break;
                        case 0x28:
                            ps->viewHeightLerpDown = ps->viewHeightCurrent > (float)ps->viewHeightTarget;
                            ps->viewHeightLerpTarget = 40;
                            break;
                        case 0x3C:
                            ps->viewHeightLerpDown = 0;
                            if ( ps->viewHeightCurrent >= 40.0 )
                                ps->viewHeightLerpTarget = 60;
                            else
                                ps->viewHeightLerpTarget = 40;
                            break;
                        default:
                            v5 = va("View height lerp to %i reached bad place\n", ps->viewHeightTarget);
                            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 3100, 0, v5) )
                                __debugbreak();
                            break;
                    }
                }
            }
            else
            {
                ps->viewHeightLerpTime = 0;
                if ( (float)ps->viewHeightTarget <= ps->viewHeightCurrent )
                {
                    ps->viewHeightCurrent = ps->viewHeightCurrent - (float)(180.0 * pml->frametime);
                    if ( (float)ps->viewHeightTarget >= ps->viewHeightCurrent )
                        ps->viewHeightCurrent = (float)ps->viewHeightTarget;
                }
                else
                {
                    ps->viewHeightCurrent = (float)(180.0 * pml->frametime) + ps->viewHeightCurrent;
                    if ( ps->viewHeightCurrent >= (float)ps->viewHeightTarget )
                        ps->viewHeightCurrent = (float)ps->viewHeightTarget;
                }
            }
        }
    }
    else if ( ps->pm_type == 4 )
    {
        ps->viewHeightCurrent = 0.0f;
    }
    else
    {
        ps->viewHeightCurrent = (float)ps->viewHeightTarget;
    }
}

double __cdecl PM_ViewHeightTableLerp(int iFrac, viewLerpWaypoint_s *pTable, float *pfPosOfs)
{
    float fEntryFrac; // [esp+0h] [ebp-10h]
    viewLerpWaypoint_s *pCurr; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    viewLerpWaypoint_s *pPrev; // [esp+Ch] [ebp-4h]

    if ( iFrac )
    {
        if ( iFrac >= 100
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 2811, 0, "%s", "iFrac < 100") )
        {
            __debugbreak();
        }
        pCurr = pTable + 1;
        i = 1;
        do
        {
            if ( iFrac == pCurr->iFrac )
            {
                *pfPosOfs = (float)pCurr->iOffset;
                return pCurr->fViewHeight;
            }
            if ( pCurr->iFrac > iFrac )
            {
                pPrev = &pTable[i - 1];
                if ( pCurr->iFrac - pPrev->iFrac <= 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                                2833,
                                0,
                                "%s",
                                "(pCurr->iFrac - pPrev->iFrac) > 0") )
                {
                    __debugbreak();
                }
                fEntryFrac = (float)(iFrac - pPrev->iFrac) / (float)(pCurr->iFrac - pPrev->iFrac);
                *pfPosOfs = (float)pPrev->iOffset + (float)((float)(pCurr->iOffset - pPrev->iOffset) * fEntryFrac);
                return (pCurr->fViewHeight - pPrev->fViewHeight) * fEntryFrac + pPrev->fViewHeight;
            }
            pCurr = &pTable[++i];
        }
        while ( pCurr->iFrac != -1 );
        if ( !va("No encapsulating table entries found for fraction %i", iFrac)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                        2846,
                        0,
                        "%s",
                        "va( \"No encapsulating table entries found for fraction %i\", iFrac )") )
        {
            __debugbreak();
        }
        *pfPosOfs = (float)pTable->iOffset;
        return pTable->fViewHeight;
    }
    else
    {
        *pfPosOfs = (float)pTable->iOffset;
        return pTable->fViewHeight;
    }
}

void __cdecl PM_Footsteps_NotMoving(pmove_t *pm, int stance, bool allow_flinch)
{
    int turnAdjust; // [esp+0h] [ebp-14h]
    int animResult; // [esp+4h] [ebp-10h]
    clientInfo_t *ci; // [esp+8h] [ebp-Ch]
    scriptAnimMoveTypes_t anim; // [esp+Ch] [ebp-8h]
    playerState_s *ps; // [esp+10h] [ebp-4h]

    ps = pm->ps;
    if ( pm->xyspeed < 1.0 && ps->waterlevel < 3 )
        ps->bobCycle = 0;
    turnAdjust = 0;
    if ( ps->clientNum >= com_maxclients->current.integer )
        ci = 0;
    else
        ci = &bgs->clientinfo[ps->clientNum];
    if ( ci && player_turnAnims->current.enabled && (ps->pm_flags & 4) == 0 )
        turnAdjust = PM_Footsteps_TurnAnim(ci);
    anim = PM_GetNotMovingAnim(turnAdjust);
    if ( ps->waterlevel >= 3 )
    {
        BG_AnimScriptAnimation(pm, AISTATE_COMBAT, ANIM_MT_SWIM, 0);
        return;
    }
    if ( turnAdjust )
    {
        animResult = BG_AnimScriptAnimation(pm, AISTATE_COMBAT, anim, 0);
        if ( animResult > 0 && !ci->turnAnimEndTime )
        {
            if ( ps->legsAnimDuration <= 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                            4065,
                            0,
                            "%s",
                            "ps->legsAnimDuration > 0") )
            {
                __debugbreak();
            }
            ci->turnAnimEndTime = ps->legsAnimDuration + bgs->time;
            if (xanim_debug->current.enabled)
                Com_Printf(
                    17,
                    "[%i] turn anim should end at %i\n",
                    bgs->time,
                    ps->legsAnimDuration + bgs->time);
        }
    }
    else
    {
        if ( allow_flinch && PM_ShouldFlinch(ps) )
            goto LABEL_24;
        animResult = BG_AnimScriptAnimation(pm, AISTATE_COMBAT, anim, 0);
    }
    if ( animResult >= 0 )
        return;
    if ( allow_flinch && PM_ShouldFlinch(ps) )
    {
LABEL_24:
        BG_AnimScriptEvent(pm, ANIM_ET_FLINCH, 1, 1);
        return;
    }
    BG_AnimScriptAnimation(pm, AISTATE_COMBAT, ANIM_MT_IDLE, 0);
}

int __cdecl PM_Footsteps_TurnAnim(clientInfo_t *ci)
{
    float diff; // [esp+10h] [ebp-8h]
    int turnAdjust; // [esp+14h] [ebp-4h]

    turnAdjust = 0;
    if (ci->turnAnimType && ci->turnAnimEndTime && xanim_debug->current.enabled)
        Com_DPrintf(
            17,
            "turn anim end time is %i, time is %i\n",
            ci->turnAnimEndTime,
            bgs->time);
    if (ci->legs.yawing)
    {
        diff = AngleNormalize180(ci->legs.yawAngle - ci->torso.yawAngle);
        if (diff <= 0.0)
        {
            if (diff >= 0.0)
                turnAdjust = ci->turnAnimType;
            else
                turnAdjust = 2;
        }
        else
        {
            turnAdjust = 1;
        }
        ci->turnAnimType = turnAdjust;
        ci->turnAnimEndTime = bgs->time + 100;
    }
    else if (ci->turnAnimEndTime)
    {
        ci->turnAnimEndTime = 0;
        if (xanim_debug->current.enabled)
            Com_Printf(
                17,
                "[%i] playing idle anim after turn anim\n",
                bgs->time);
    }
    return turnAdjust;
}

scriptAnimMoveTypes_t __cdecl PM_GetNotMovingAnim(int turnAdjust)
{
    return notMovingAnims[turnAdjust];
}

bool __cdecl PM_ShouldFlinch(playerState_s *ps)
{
    int flinch_end_time; // [esp+0h] [ebp-4h]

    if ( (float)ps->viewHeightTarget != ps->viewHeightCurrent )
        return 0;
    flinch_end_time = ps->damageDuration - player_dmgtimer_flinchTime->current.integer;
    if ( flinch_end_time < 0 )
        flinch_end_time = 0;
    return ps->damageTimer > flinch_end_time;
}

void __cdecl PM_VehicleDrive(pmove_t *pm, pml_t *pml)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    if (!pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 4331, 0, "%s", "pm"))
        __debugbreak();
    ps = pm->ps;
    if ((pm->ps->eFlags & 0x4000) != 0 && ps->viewlocked_entNum != 1023 && !ps->vehiclePos)
    {
        if (!pmoveHandlers[pm->handler].setVehDriverInputs
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                4336,
                0,
                "%s",
                "pmoveHandlers[pm->handler].setVehDriverInputs"))
        {
            __debugbreak();
        }
        pmoveHandlers[pm->handler].setVehDriverInputs(pm->localClientNum, ps->viewlocked_entNum, &pm->cmd);
    }
}

float dtpBobMove = 0.24;
void __cdecl PM_Footsteps(pmove_t *pm, pml_t *pml)
{
    scriptAnimMoveTypes_t ActiveCount; // eax
    int v3; // edx
    BOOL Footsteps; // eax
    BOOL v5; // [esp+18h] [ebp-30h]
    float fMaxSpeed; // [esp+24h] [ebp-24h]
    int iStance; // [esp+28h] [ebp-20h]
    int walking; // [esp+2Ch] [ebp-1Ch]
    int sprinting; // [esp+30h] [ebp-18h]
    BOOL animWalking; // [esp+34h] [ebp-14h]
    playerState_s *ps; // [esp+38h] [ebp-10h]
    float bobmove; // [esp+3Ch] [ebp-Ch]
    int old; // [esp+40h] [ebp-8h]
    PmStanceFrontBack stanceFrontBack; // [esp+44h] [ebp-4h]

    if (!pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 4498, 0, "%s", "pm"))
        __debugbreak();
    ps = pm->ps;
    if (!pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 4501, 0, "%s", "ps"))
        __debugbreak();
    BG_CheckThread();
    if (!bgs
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 4506, 0, "%s", "bgs"))
    {
        __debugbreak();
    }
    if (ps->pm_type < 9)
    {
        if (ps->clientNum < com_maxclients->current.integer)
            bgs->clientinfo[ps->clientNum].turnAnimEndTime = 0;
        pm->xyspeed = Vec2Length(ps->velocity);
        pm->currentPitch = PM_CalcPlayerPitch(ps, pml);
        pm->averagePitch = (float)((float)bg_slopeFrames->current.integer * pm->averagePitch)
            - pm->pitchHistory[pm->nextPitch];
        pm->pitchHistory[pm->nextPitch] = pm->currentPitch;
        pm->averagePitch = (float)(pm->averagePitch + pm->currentPitch) / (float)bg_slopeFrames->current.integer;
        if (++pm->nextPitch >= bg_slopeFrames->current.integer)
            pm->nextPitch = 0;
        if ((pm->ps->pm_flags & 0x400000) != 0)
            ps->bobCycle = BG_CalcBob(pm, pml, ps->bobCycle, dtpBobMove);
        if ((ps->eFlags & 0x300) != 0)
        {
            ActiveCount = (scriptAnimMoveTypes_t)CL_LocalClient_GetActiveCount();
            BG_AnimScriptAnimation(pm, AISTATE_COMBAT, ActiveCount, 0);
            return;
        }
        iStance = PM_GetEffectiveStance(ps);
        stanceFrontBack = (PmStanceFrontBack)PM_GetStanceEx(iStance, ps->pm_flags & 0x20);
        if (PM_IsInAir(pm)
            && !ps->waterlevel
            && (ps->pm_flags & 0x400000) == 0
            && (0x800000 & ps->pm_flags) == 0
            && (ps->pm_flags & 4) == 0)
        {
            if (!PM_Footstep_LadderMove(pm, pml))
                PM_ApplyLegAnimations(pm, pml, iStance, stanceFrontBack);
            if (iStance != (ps->pm_flags & 3)
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                    4559,
                    0,
                    "%s",
                    "iStance == (ps->pm_flags & ( PMF_DUCKED | PMF_PRONE) )"))
            {
                __debugbreak();
            }
            return;
        }
        v5 = (ps->pm_flags & 0x40) != 0 || ps->leanf != 0.0;
        walking = v5;
        v3 = ps->pm_flags & 0x8000;
        sprinting = v3 != 0;
        animWalking = walking;
        if (v3
            && v5
            && ps->leanf == 0.0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                4567,
                0,
                "%s",
                "!sprinting || !walking || ps->leanf"))
        {
            __debugbreak();
        }
        if ((player_moveThreshhold->current.value > pm->xyspeed || ps->pm_type == 1)
            && (0x800000 & ps->pm_flags) == 0
            && (ps->pm_flags & 4) == 0)
        {
            PM_Footsteps_NotMoving(pm, iStance, 1);
            if (ps->waterlevel >= 3)
                ps->bobCycle = BG_CalcBob(pm, pml, ps->bobCycle, bg_weaponBobFrequencySwimming->current.value);
            return;
        }
        if (!pm->cmd.forwardmove && !pm->cmd.rightmove && (0x800000 & ps->pm_flags) == 0)
        {
            PM_Footstep_NotTryingToMove(pm);
            if (ps->waterlevel >= 3)
                ps->bobCycle = BG_CalcBob(pm, pml, ps->bobCycle, bg_weaponBobFrequencySwimming->current.value);
            return;
        }
        if (!iStance && (pm->ps->pm_flags & 0x400000) == 0)
        {
            walking |= player_runbkThreshhold->current.value >= pm->xyspeed;
            sprinting = (pm->xyspeed >= player_sprintThreshhold->current.value) & (unsigned __int8)sprinting;
            if (player_animWalkThreshhold->current.value > pm->xyspeed)
            {
                if (player_enableShuffleAnims->current.enabled)
                    PM_ApplyMovementAnimations(pm, pml, stanceFrontBack, 2, sprinting);
                else
                    PM_ApplyMovementAnimations(pm, pml, stanceFrontBack, 1, sprinting);
            LABEL_60:
                fMaxSpeed = PM_GetMaxSpeed(pm, walking, sprinting);
                bobmove = PM_GetBobMove(pm, stanceFrontBack, pm->xyspeed, fMaxSpeed, walking, sprinting);
                if ((pm->ps->pm_flags & 0x400000) == 0 && (0x800000 & pm->ps->pm_flags) == 0)
                {
                    old = ps->bobCycle;
                    if (ps->waterlevel >= 3)
                        bobmove = bg_weaponBobFrequencySwimming->current.value;
                    ps->bobCycle = BG_CalcBob(pm, pml, old, bobmove);
                    Footsteps = PM_ShouldMakeFootsteps(pm);
                    PM_FootstepEvent(pm, pml, old, ps->bobCycle, Footsteps);
                }
                return;
            }
            animWalking = player_animRunThreshhold->current.value > pm->xyspeed;
        }
        PM_ApplyMovementAnimations(pm, pml, stanceFrontBack, animWalking, sprinting);
        goto LABEL_60;
    }
}

int __cdecl PM_GetStanceEx(int stance, int backward)
{
    if ( stance >= 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                    574,
                    0,
                    "%s",
                    "stance < PM_STANCE_BACKWARD_FIRST") )
    {
        __debugbreak();
    }
    if ( backward )
        return stance + 3;
    else
        return stance;
}

char __cdecl PM_Footstep_LadderMove(pmove_t *pm, pml_t *pml)
{
    float fMaxSpeed; // [esp+0h] [ebp-14h]
    float fLadderSpeed; // [esp+4h] [ebp-10h]
    playerState_s *ps; // [esp+8h] [ebp-Ch]
    float bobmove; // [esp+Ch] [ebp-8h]
    int old; // [esp+10h] [ebp-4h]

    ps = pm->ps;
    if ( (pm->ps->pm_flags & 8) == 0 )
        return 0;
    if ( pm->cmd.serverTime - ps->jumpTime < 300 )
        return 1;
    fLadderSpeed = ps->velocity[2];
    fMaxSpeed = (float)(0.5 * 1.5) * 127.0;
    if ( (ps->pm_flags & 0x40) == 0 && ps->leanf == 0.0 )
        bobmove = (float)(fLadderSpeed / (float)(fMaxSpeed * 1.0)) * 0.44999999;
    else
        bobmove = (float)(fLadderSpeed / (float)(fMaxSpeed * 0.40000001)) * 0.34999999;
    if ( fLadderSpeed < 0.0 )
        BG_AnimScriptAnimation(pm, AISTATE_COMBAT, ANIM_MT_CLIMBDOWN, 0);
    else
        BG_AnimScriptAnimation(pm, AISTATE_COMBAT, ANIM_MT_CLIMBUP, 1);
    old = ps->bobCycle;
    ps->bobCycle = (unsigned __int8)(int)(float)((float)old + (float)((float)pml->msec * bobmove));
    PM_FootstepEvent(pm, pml, old, ps->bobCycle, 1);
    return 1;
}

double __cdecl PM_GetMaxSpeed(pmove_t *pm, int walking, int sprinting)
{
    float fMaxSpeed; // [esp+0h] [ebp-8h]
    float fMaxSpeeda; // [esp+0h] [ebp-8h]
    float fMaxSpeedb; // [esp+0h] [ebp-8h]
    const WeaponDef *weapon; // [esp+4h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 4104, 0, "%s", "pm") )
        __debugbreak();
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 4105, 0, "%s", "pm->ps") )
        __debugbreak();
    fMaxSpeed = (float)pm->ps->speed;
    if ( pm->cmd.forwardmove )
    {
        if ( pm->cmd.rightmove )
        {
            fMaxSpeed = (float)((float)((float)((float)((float)(player_strafeSpeedScale->current.value - 1.0) * 0.75) + 1.0)
                                                                + 1.0)
                                                * 0.5)
                                * fMaxSpeed;
            if ( pm->cmd.forwardmove < 0 )
                fMaxSpeed = (float)((float)(player_backSpeedScale->current.value + 1.0) * 0.5) * fMaxSpeed;
        }
        else if ( pm->cmd.forwardmove < 0 )
        {
            fMaxSpeed = fMaxSpeed * player_backSpeedScale->current.value;
        }
    }
    else if ( pm->cmd.rightmove )
    {
        fMaxSpeed = (float)((float)((float)(player_strafeSpeedScale->current.value - 1.0) * 0.75) + 1.0) * fMaxSpeed;
    }
    if ( walking )
    {
        fMaxSpeeda = fMaxSpeed * 0.40000001;
    }
    else if ( sprinting )
    {
        fMaxSpeeda = fMaxSpeed * player_sprintSpeedScale->current.value;
    }
    else
    {
        fMaxSpeeda = fMaxSpeed * 1.0;
    }
    if ( pm->ps->weapon )
    {
        weapon = BG_GetWeaponDef(pm->ps->weapon);
        if ( weapon->moveSpeedScale <= 0.0 || (pm->ps->pm_flags & 0x40) != 0 )
        {
            if ( weapon->adsMoveSpeedScale > 0.0 )
                fMaxSpeeda = fMaxSpeeda * weapon->adsMoveSpeedScale;
        }
        else
        {
            fMaxSpeeda = fMaxSpeeda * weapon->moveSpeedScale;
        }
    }
    fMaxSpeedb = PM_CmdScaleForStance(pm) * fMaxSpeeda;
    return (float)(fMaxSpeedb * pm->ps->moveSpeedScaleMultiplier);
}

double __cdecl PM_GetBobMove(
                pmove_t *pm,
                PmStanceFrontBack stance,
                float xyspeed,
                float fMaxSpeed,
                int walking,
                int sprinting)
{
    float bobFactor; // [esp+0h] [ebp-4h]

    if ( stance || !sprinting )
        bobFactor = bobFactorTable[stance][walking];
    else
        bobFactor = player_sprintCameraBob->current.value;
    return xyspeed / fMaxSpeed * bobFactor;
}

void __cdecl PM_Footstep_NotTryingToMove(pmove_t *pm)
{
    playerState_s *ps; // [esp+4h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 4298, 0, "%s", "ps") )
        __debugbreak();
    if ( pm->xyspeed <= 120.0 )
    {
        if ( ps->waterlevel < 3 )
        {
            if ( PM_ShouldFlinch(ps) )
                BG_AnimScriptEvent(pm, ANIM_ET_FLINCH, 1, 1);
            else
                BG_AnimScriptAnimation(pm, AISTATE_COMBAT, ANIM_MT_IDLE, 0);
        }
        else
        {
            BG_AnimScriptAnimation(pm, AISTATE_COMBAT, ANIM_MT_SWIM, 0);
        }
    }
}

int __cdecl BG_CalcBob(pmove_t *pm, pml_t *pml, int old, float bobmove)
{
    float v5; // [esp+0h] [ebp-18h]
    float v6; // [esp+8h] [ebp-10h]
    float v7; // [esp+10h] [ebp-8h]

    if ( (pm->ps->pm_flags & 0x400000) != 0 )
    {
        v7 = (float)pml->msec * bobmove;
        return (unsigned __int8)(int)(float)((float)old + (float)(v7 * BG_GetWeaponDef(pm->ps->weapon)->fDtpCycleScale));
    }
    else if ( (pm->ps->pm_flags & 0x8000) != 0 && pm->ps->viewHeightTarget == 40 )
    {
        v6 = (float)pml->msec * bobmove;
        return (unsigned __int8)(int)(float)((float)old
                                                                             + (float)(v6 * BG_GetWeaponDef(pm->ps->weapon)->fDuckedSprintCycleScale));
    }
    else if ( (pm->ps->pm_flags & 0x8000) != 0 || pm->ps->waterlevel >= 3 )
    {
        v5 = (float)pml->msec * bobmove;
        return (unsigned __int8)(int)(float)((float)old + (float)(v5 * BG_GetWeaponDef(pm->ps->weapon)->fSprintCycleScale));
    }
    else
    {
        return (unsigned __int8)(int)(float)((float)old + (float)((float)pml->msec * bobmove));
    }
}

void __cdecl PM_ApplyMovementAnimations(
                pmove_t *pm,
                pml_t *pml,
                PmStanceFrontBack stanceFrontBack,
                int walking,
                int sprinting)
{
    scriptAnimMoveTypes_t moveAnim; // [esp+0h] [ebp-Ch]
    int animResult; // [esp+4h] [ebp-8h]
    playerState_s *ps; // [esp+8h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 4359, 0, "%s", "ps") )
        __debugbreak();
    moveAnim = (scriptAnimMoveTypes_t)PM_GetMoveAnim(ps, pml, stanceFrontBack, walking, sprinting);
    if ( sprinting )
        animResult = BG_AnimScriptAnimation(pm, AISTATE_COMBAT, moveAnim, 1);
    else
        animResult = BG_AnimScriptAnimation(pm, AISTATE_COMBAT, moveAnim, 0);
    if ( animResult < 0 )
        BG_AnimScriptAnimation(pm, AISTATE_COMBAT, ANIM_MT_IDLE, 0);
}

int __cdecl PM_GetMoveAnim(playerState_s *ps, pml_t *pml, PmStanceFrontBack stance, int walking, int sprinting)
{
    scriptAnimMoveTypes_t moveAnim; // [esp+0h] [ebp-Ch]
    int stumble_end_time; // [esp+4h] [ebp-8h]

    stumble_end_time = ps->damageDuration - player_dmgtimer_stumbleTime->current.integer;
    if ( stumble_end_time < 0 )
        stumble_end_time = 0;
    moveAnim = moveAnimTable[stance][walking][ps->damageTimer > stumble_end_time];
    if ( stance == PM_STANCE_STAND && sprinting )
        moveAnim = ps->damageTimer > stumble_end_time ? ANIM_MT_STUMBLE_SPRINT : ANIM_MT_SPRINT;
    if ( (ps->pm_flags & 0x400000) != 0 )
        moveAnim = ANIM_MT_DTP;
    if ( (pml->groundTrace.sflags & 2) != 0 )
        moveAnim = ANIM_MT_SLIDE;
    if ( ps->waterlevel >= 3 )
        return 24;
    return moveAnim;
}

void __cdecl PM_ApplyLegAnimations(pmove_t *pm, pml_t *pml, int iStance, PmStanceFrontBack stanceFrontBack)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 4386, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 4389, 0, "%s", "ps") )
        __debugbreak();
    if ( pml->almostGroundPlane && !pml->walking && (ps->pm_flags & 4) == 0 && !ps->pm_type )
    {
        if ( player_moveThreshhold->current.value <= pm->xyspeed )
            PM_ApplyMovementAnimations(pm, pml, stanceFrontBack, 1, 0);
        else
            PM_Footsteps_NotMoving(pm, iStance, 0);
    }
}

double __cdecl PM_CalcPlayerPitch(playerState_s *ps, pml_t *pml)
{
    float v3; // [esp+Ch] [ebp-38h]
    __int64 v4; // [esp+14h] [ebp-30h]
    float playerYaw; // [esp+1Ch] [ebp-28h]
    float yawTrans; // [esp+20h] [ebp-24h]
    float groundNormAngles[3]; // [esp+28h] [ebp-1Ch] BYREF
    float groundTrace[3]; // [esp+34h] [ebp-10h] BYREF
    float pitchTrans; // [esp+40h] [ebp-4h]

    playerYaw = AngleNormalize360(ps->viewangles[1]);
    if ( !pml->groundPlane )
        return 0.0;
    v4 = *(_QWORD *)&pml->groundTrace.normal.vec.unitVec[1].packed;
    groundTrace[0] = pml->groundTrace.normal.vec.v[0];
    *(_QWORD *)&groundTrace[1] = v4;
    vectoangles(groundTrace, groundNormAngles);
    yawTrans = AngleNormalize360(groundNormAngles[1]);
    v3 = AngleDelta(playerYaw, yawTrans);
    if ( fabs(v3) >= 35.0 )
    {
        if ( fabs(v3) <= 145.0 )
            pitchTrans = 0.0f;
        else
            pitchTrans = groundNormAngles[0] + 90.0;
    }
    else
    {
        pitchTrans = -(groundNormAngles[0] + 90.0);
    }
    return AngleNormalize180(pitchTrans);
}

void __cdecl PM_FoliageSounds(pmove_t *pm)
{
    float mins[3]; // [esp+8h] [ebp-60h] BYREF
    float speedFrac; // [esp+14h] [ebp-54h]
    trace_t trace; // [esp+18h] [ebp-50h] BYREF
    float maxs[3]; // [esp+54h] [ebp-14h] BYREF
    int interval; // [esp+60h] [ebp-8h]
    playerState_s *ps; // [esp+64h] [ebp-4h]

    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 4652, 0, "%s", "ps") )
        __debugbreak();
    if ( bg_foliagesnd_minspeed->current.value <= pm->xyspeed )
    {
        if ( (float)(bg_foliagesnd_maxspeed->current.value - bg_foliagesnd_minspeed->current.value) <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                        4662,
                        1,
                        "%s",
                        "bg_foliagesnd_maxspeed->current.value - bg_foliagesnd_minspeed->current.value > 0") )
        {
            __debugbreak();
        }
        speedFrac = (float)(pm->xyspeed - bg_foliagesnd_minspeed->current.value)
                            / (float)(bg_foliagesnd_maxspeed->current.value - bg_foliagesnd_minspeed->current.value);
        if ( speedFrac > 1.0 )
            speedFrac = 1.0f;
        interval = (int)(float)((float)((float)(bg_foliagesnd_fastinterval->current.integer
                                                                                    - bg_foliagesnd_slowinterval->current.integer)
                                                                    * speedFrac)
                                                    + (float)bg_foliagesnd_slowinterval->current.integer);
        if ( interval + ps->foliageSoundTime < pm->cmd.serverTime )
        {
            mins[0] = 0.75 * pm->mins[0];
            mins[1] = 0.75 * pm->mins[1];
            mins[2] = 0.75 * pm->mins[2];
            maxs[0] = 0.75 * pm->maxs[0];
            maxs[1] = 0.75 * pm->maxs[1];
            maxs[2] = 0.75 * pm->maxs[2];
            maxs[2] = pm->maxs[2] * 0.89999998;
            if ( phys_player_collision_mode->current.integer == 1 )
                PM_trace(pm, &trace, ps->origin, mins, maxs, ps->origin, ps->clientNum, 2);
            else
                PM_playerTrace(pm, &trace, ps->origin, mins, maxs, ps->origin, ps->clientNum, 2);
            if ( trace.startsolid )
            {
                PM_AddEvent(ps, 1u);
                ps->foliageSoundTime = pm->cmd.serverTime;
            }
        }
    }
    else if ( bg_foliagesnd_resetinterval->current.integer + ps->foliageSoundTime < pm->cmd.serverTime )
    {
        ps->foliageSoundTime = 0;
    }
}

void __cdecl PM_DropTimers(playerState_s *ps, pml_t *pml)
{
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 4696, 0, "%s", "ps") )
        __debugbreak();
    if ( ps->pm_time )
    {
        if ( (ps->pm_flags & 0x200000) != 0 && ps->groundEntityNum == 1022 )
        {
            ps->pm_flags &= ~0x200000u;
            ps->pm_time = 0;
        }
        if ( pml->msec < ps->pm_time )
        {
            ps->pm_time -= pml->msec;
        }
        else
        {
            if ( (ps->pm_flags & 0x4000) != 0 )
                Jump_ClearState(ps);
            ps->pm_flags &= 0xFFDFBE7F;
            ps->pm_time = 0;
        }
    }
    if ( ps->legsTimer > 0 )
    {
        ps->legsTimer -= pml->msec;
        if ( ps->legsTimer < 0 )
        {
            ps->legsTimer = 0;
            if ( G_IsServerGameSystem(ps->clientNum) )
                Com_Printf(19, "end legs\n");
        }
    }
    if ( ps->torsoTimer > 0 )
    {
        ps->torsoTimer -= pml->msec;
        if ( ps->torsoTimer < 0 )
        {
            ps->torsoTimer = 0;
            if ( G_IsServerGameSystem(ps->clientNum) )
                Com_Printf(19, "end torso\n");
        }
    }
}

void __cdecl PM_UpdatePlayerWalkingFlag(pmove_t *pm)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 5512, 0, "%s", "ps") )
        __debugbreak();
    ps->pm_flags &= ~0x40u;
    if ( ps->pm_type < 9 )
    {
        if ( pm->cmd.button_bits.testBit(0xBu) )
        {
            if ( (ps->pm_flags & 1) == 0
                && (ps->pm_flags & 0x10) != 0
                && ps->weaponstate != 10
                && ps->weaponstate != 12
                && ps->weaponstate != 14
                && ps->weaponstate != 13
                && ps->weaponstate != 11
                && ps->weaponstate != 15
                && ps->weaponstate != 16 )
            {
                ps->pm_flags |= 0x40u;
                if ( (ps->otherFlags & 4) == 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                                5536,
                                0,
                                "%s",
                                "ps->otherFlags & POF_PLAYER") )
                {
                    __debugbreak();
                }
            }
        }
    }
}

void __cdecl PM_ClearLadderFlag(playerState_s *ps)
{
    if ( (ps->pm_flags & 8) != 0 )
    {
        ps->pm_flags |= 0x2000u;
        ps->pm_flags &= ~8u;
    }
}

void __cdecl PM_CheckLadderMove(pmove_t *pm, pml_t *pml)
{
    bool v2; // [esp+0h] [ebp-A4h]
    float *vLadderVec; // [esp+Ch] [ebp-98h]
    float vLadderCheckDir[3]; // [esp+2Ch] [ebp-78h] BYREF
    float mins[3]; // [esp+38h] [ebp-6Ch] BYREF
    float tracedist; // [esp+44h] [ebp-60h]
    float spot[3]; // [esp+48h] [ebp-5Ch] BYREF
    int fellOffLadderInAir; // [esp+54h] [ebp-50h]
    float maxs[3]; // [esp+58h] [ebp-4Ch] BYREF
    trace_t trace; // [esp+64h] [ebp-40h] BYREF
    playerState_s *ps; // [esp+A0h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 5574, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 5577, 0, "%s", "ps") )
        __debugbreak();
    if ( pml->walking || ps->waterlevel >= 3 )
        ps->pm_flags &= ~0x2000u;
    if ( !ps->pm_time || (ps->pm_flags & 8) != 0 || (ps->pm_flags & 0x180) == 0 )
    {
        if ( pml->walking )
            tracedist = 8.0f;
        else
            tracedist = 30.0f;
        v2 = (ps->pm_flags & 8) != 0 && ps->groundEntityNum == 1023;
        fellOffLadderInAir = v2;
        if ( v2 )
        {
            vLadderCheckDir[0] = -ps->vLadderVec[0];
            vLadderCheckDir[1] = -ps->vLadderVec[1];
            vLadderCheckDir[2] = -ps->vLadderVec[2];
        }
        else
        {
            vLadderCheckDir[0] = pml->forward[0];
            vLadderCheckDir[1] = pml->forward[1];
            vLadderCheckDir[2] = 0.0f;
            Vec3Normalize(vLadderCheckDir);
        }
        if ( ps->pm_type < 9 )
        {
            if ( (ps->pm_flags & 0x2000) != 0 || PM_GetEffectiveStance(ps) == 1 || pm->cmd.serverTime - ps->jumpTime < 300 )
            {
                PM_ClearLadderFlag(ps);
            }
            else
            {
                mins[0] = pm->mins[0];
                mins[1] = pm->mins[1];
                mins[0] = mins[0] + 6.0;
                mins[1] = mins[1] + 6.0;
                mins[2] = 8.0f;
                maxs[0] = pm->maxs[0];
                maxs[1] = pm->maxs[1];
                maxs[2] = pm->maxs[2];
                maxs[0] = maxs[0] - 6.0;
                maxs[1] = maxs[1] - 6.0;
                if ( maxs[2] < 8.0 )
                    maxs[2] = mins[2];
                if ( maxs[0] < mins[0]
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                                5653,
                                0,
                                "%s",
                                "maxs[0] >= mins[0]") )
                {
                    __debugbreak();
                }
                if ( maxs[1] < mins[1]
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                                5654,
                                0,
                                "%s",
                                "maxs[1] >= mins[1]") )
                {
                    __debugbreak();
                }
                if ( maxs[2] < mins[2]
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp",
                                5655,
                                0,
                                "%s",
                                "maxs[2] >= mins[2]") )
                {
                    __debugbreak();
                }
                spot[0] = (float)(tracedist * vLadderCheckDir[0]) + ps->origin[0];
                spot[1] = (float)(tracedist * vLadderCheckDir[1]) + ps->origin[1];
                spot[2] = (float)(tracedist * vLadderCheckDir[2]) + ps->origin[2];
                if ( phys_player_collision_mode->current.integer == 1 )
                    PM_trace(pm, &trace, ps->origin, mins, maxs, spot, ps->clientNum, pm->tracemask);
                else
                    PM_playerTrace(pm, &trace, ps->origin, mins, maxs, spot, ps->clientNum, pm->tracemask);
                if ( trace.fraction >= 1.0 || (trace.sflags & 8) == 0 || pml->walking && pm->cmd.forwardmove <= 0 )
                    goto LABEL_57;
                if ( (ps->pm_flags & 8) != 0 )
                    goto LABEL_54;
                vLadderVec = ps->vLadderVec;
                LODWORD(ps->vLadderVec[0]) = trace.normal.vec.u[0];
                *(_QWORD *)(vLadderVec + 1) = *(_QWORD *)&trace.normal.vec.unitVec[1].packed;
                vLadderCheckDir[0] = -ps->vLadderVec[0];
                vLadderCheckDir[1] = -ps->vLadderVec[1];
                vLadderCheckDir[2] = -ps->vLadderVec[2];
                spot[0] = (float)(tracedist * vLadderCheckDir[0]) + ps->origin[0];
                spot[1] = (float)(tracedist * vLadderCheckDir[1]) + ps->origin[1];
                spot[2] = (float)(tracedist * vLadderCheckDir[2]) + ps->origin[2];
                if ( phys_player_collision_mode->current.integer == 1 )
                    PM_trace(pm, &trace, ps->origin, mins, maxs, spot, ps->clientNum, pm->tracemask);
                else
                    PM_playerTrace(pm, &trace, ps->origin, mins, maxs, spot, ps->clientNum, pm->tracemask);
                if ( trace.fraction < 1.0 && (trace.sflags & 8) != 0 )
                {
LABEL_54:
                    PM_SetLadderFlag(ps);
                }
                else
                {
LABEL_57:
                    PM_ClearLadderFlag(ps);
                    if ( fellOffLadderInAir )
                        BG_AnimScriptEvent(pm, ANIM_ET_JUMP, 0, 1);
                }
            }
        }
        else
        {
            ps->groundEntityNum = 1023;
            pml->groundPlane = 0;
            pml->almostGroundPlane = 0;
            pml->walking = 0;
            PM_ClearLadderFlag(ps);
        }
    }
}

void __cdecl PM_SetLadderFlag(playerState_s *ps)
{
    ps->pm_flags |= 8u;
}

void __cdecl PM_LadderMove(pmove_t *pm, pml_t *pml)
{
    float v2; // [esp+8h] [ebp-D4h]
    float *v3; // [esp+Ch] [ebp-D0h]
    float *v4; // [esp+10h] [ebp-CCh]
    float *v5; // [esp+14h] [ebp-C8h]
    float *v6; // [esp+24h] [ebp-B8h]
    float *vLadderVec; // [esp+28h] [ebp-B4h]
    float v8; // [esp+2Ch] [ebp-B0h]
    float *v9; // [esp+30h] [ebp-ACh]
    float *v10; // [esp+3Ch] [ebp-A0h]
    float *v11; // [esp+40h] [ebp-9Ch]
    float *velocity; // [esp+48h] [ebp-94h]
    float v13; // [esp+4Ch] [ebp-90h]
    float *v14; // [esp+50h] [ebp-8Ch]
    float v15; // [esp+78h] [ebp-64h]
    float wishdir[3]; // [esp+94h] [ebp-48h] BYREF
    float fSideSpeed; // [esp+A0h] [ebp-3Ch]
    float wishvel[3]; // [esp+A4h] [ebp-38h] BYREF
    float vTempRight[3]; // [esp+B0h] [ebp-2Ch] BYREF
    float vSideDir[2]; // [esp+BCh] [ebp-20h] BYREF
    float fSpeedDrop; // [esp+C4h] [ebp-18h]
    float wishspeed; // [esp+C8h] [ebp-14h]
    float upscale; // [esp+CCh] [ebp-10h]
    int moveyaw; // [esp+D0h] [ebp-Ch]
    float scale; // [esp+D4h] [ebp-8h]
    playerState_s *ps; // [esp+D8h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 5715, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 5718, 0, "%s", "ps") )
        __debugbreak();
    if ( Jump_Check(pm, pml) )
    {
        PM_AirMove(pm, pml);
    }
    else
    {
        upscale = (float)(pml->forward[2] + 0.25) * 2.5;
        if ( upscale <= 1.0 )
        {
            if ( upscale < -1.0 )
                upscale = -1.0f;
        }
        else
        {
            upscale = 1.0f;
        }
        pml->forward[2] = 0.0f;
        Vec3Normalize(pml->forward);
        pml->right[2] = 0.0f;
        Vec3NormalizeTo(pml->right, vTempRight);
        ProjectPointOnPlane(vTempRight, ps->vLadderVec, pml->right);
        scale = PM_CmdScale(ps, &pm->cmd);
        memset(wishvel, 0, sizeof(wishvel));
        if ( pm->cmd.forwardmove )
            wishvel[2] = (float)((float)(0.5 * upscale) * scale) * (float)pm->cmd.forwardmove;
        if ( pm->cmd.rightmove )
        {
            v15 = (float)(0.2 * scale) * (float)pm->cmd.rightmove;
            wishvel[0] = (float)(v15 * pml->right[0]) + wishvel[0];
            wishvel[1] = (float)(v15 * pml->right[1]) + wishvel[1];
            wishvel[2] = (float)(v15 * pml->right[2]) + wishvel[2];
        }
        wishspeed = Vec3NormalizeTo(wishvel, wishdir);
        PM_Accelerate(ps, pml, wishdir, wishspeed, 9.0);
        if ( !pm->cmd.forwardmove )
        {
            if ( ps->velocity[2] <= 0.0 )
            {
                ps->velocity[2] = (float)((float)ps->gravity * pml->frametime) + ps->velocity[2];
                if ( ps->velocity[2] > 0.0 )
                    ps->velocity[2] = 0.0f;
            }
            else
            {
                ps->velocity[2] = ps->velocity[2] - (float)((float)ps->gravity * pml->frametime);
                if ( ps->velocity[2] < 0.0 )
                    ps->velocity[2] = 0.0f;
            }
        }
        if ( !pm->cmd.rightmove )
        {
            vSideDir[0] = pml->right[0];
            vSideDir[1] = pml->right[1];
            Vec2Normalize(vSideDir);
            fSideSpeed = (float)(vSideDir[0] * ps->velocity[0]) + (float)(vSideDir[1] * ps->velocity[1]);
            if ( fSideSpeed != 0.0 )
            {
                velocity = ps->velocity;
                v13 = -fSideSpeed;
                v14 = ps->velocity;
                ps->velocity[0] = (float)((-(fSideSpeed)) * vSideDir[0]) + ps->velocity[0];
                velocity[1] = (float)(v13 * vSideDir[1]) + v14[1];
                fSpeedDrop = (float)(fSideSpeed * pml->frametime) * 16.0;
                if ( fabs(fSideSpeed) > fabs(fSpeedDrop) )
                {
                    if ( fabs(fSpeedDrop) < 1.0 )
                    {
                        if ( fSpeedDrop < 0.0 )
                            v2 = -1.0f;
                        else
                            v2 = 1.0f;
                        fSpeedDrop = v2;
                    }
                    fSideSpeed = fSideSpeed - fSpeedDrop;
                    v10 = ps->velocity;
                    v11 = ps->velocity;
                    ps->velocity[0] = (float)(fSideSpeed * vSideDir[0]) + ps->velocity[0];
                    v10[1] = (float)(fSideSpeed * vSideDir[1]) + v11[1];
                }
            }
        }
        if ( !pml->walking )
        {
            fSideSpeed = (float)(ps->vLadderVec[0] * ps->velocity[0]) + (float)(ps->vLadderVec[1] * ps->velocity[1]);
            v6 = ps->velocity;
            vLadderVec = ps->vLadderVec;
            v8 = -fSideSpeed;
            v9 = ps->velocity;
            ps->velocity[0] = (float)((-(fSideSpeed)) * ps->vLadderVec[0]) + ps->velocity[0];
            v6[1] = (float)(v8 * vLadderVec[1]) + v9[1];
            if ( (ps->velocity[0] != 0.0 || ps->velocity[1] != 0.0 || ps->velocity[2] != 0.0)
                && (float)(ps->velocity[2] * ps->velocity[2]) >= (float)((float)(ps->velocity[0] * ps->velocity[0])
                                                                                                                             + (float)(ps->velocity[1] * ps->velocity[1])) )
            {
                fSideSpeed = -50.0f;
                v3 = ps->velocity;
                v4 = ps->vLadderVec;
                v5 = ps->velocity;
                ps->velocity[0] = (float)(-50.0 * ps->vLadderVec[0]) + ps->velocity[0];
                v3[1] = (float)(fSideSpeed * v4[1]) + v5[1];
            }
        }
        PM_StepSlideMove(pm, pml, 0);
        scale = vectoyaw(ps->vLadderVec) + 180.0;
        moveyaw = (int)AngleDelta(scale, ps->viewangles[1]);
        if ( (int)abs(moveyaw) > 75 )
        {
            if ( moveyaw <= 0 )
                moveyaw = -75;
            else
                moveyaw = 75;
        }
        ps->movementDir = (char)moveyaw;
    }
}

void __cdecl PM_CheckMeleeCharge(pmove_t *pm, pml_t *pml)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 5985, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 5988, 0, "%s", "ps") )
        __debugbreak();
    if ( (ps->pm_flags & 0x20000) != 0 )
    {
        if ( !ps->meleeChargeTime )
            PM_CalcMeleeChargeTime(ps, &ps->meleeChargeTime, ps->velocity);
    }
    else
    {
        PM_MeleeChargeClear(ps);
    }
}

void __cdecl PM_MeleeChargeMove(pmove_t *pm, pml_t *pml)
{
    float *v2; // [esp+0h] [ebp-34h]
    float *v3; // [esp+4h] [ebp-30h]
    float *velocity; // [esp+8h] [ebp-2Ch]
    float v5; // [esp+Ch] [ebp-28h]
    float dir[3]; // [esp+1Ch] [ebp-18h] BYREF
    float newSpeed; // [esp+28h] [ebp-Ch]
    float speed; // [esp+2Ch] [ebp-8h]
    playerState_s *ps; // [esp+30h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 6013, 0, "%s", "pm") )
        __debugbreak();
    if ( !pml && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 6014, 0, "%s", "pml") )
        __debugbreak();
    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 6017, 0, "%s", "ps") )
        __debugbreak();
    speed = Vec3NormalizeTo(ps->velocity, dir);
    newSpeed = speed;
    if ( speed > 0.0 )
    {
        newSpeed = speed - (float)((float)(speed / (float)((float)ps->meleeChargeTime * 0.001)) * pml->frametime);
        if ( newSpeed < 0.0 )
            newSpeed = 0.0f;
        velocity = ps->velocity;
        v5 = (float)(speed + newSpeed) * 0.5;
        ps->velocity[0] = v5 * dir[0];
        velocity[1] = v5 * dir[1];
        velocity[2] = v5 * dir[2];
    }
    PM_ProjectVelocity(ps->velocity, pml->groundTrace.normal.vec.v, ps->velocity);
    if ( ps->velocity[0] != 0.0 || ps->velocity[1] != 0.0 )
        PM_StepSlideMove(pm, pml, 0);
    v3 = ps->velocity;
    ps->velocity[0] = newSpeed * dir[0];
    v3[1] = newSpeed * dir[1];
    v3[2] = newSpeed * dir[2];
    ps->meleeChargeTime -= pml->msec;
    if ( ps->meleeChargeTime <= 0 )
    {
        v2 = ps->velocity;
        ps->velocity[0] = 0.0f;
        v2[1] = 0.0f;
        PM_MeleeChargeClear(ps);
    }
    PM_SetMovementDir(pm, pml);
}

void __cdecl TurretNVGTrigger(pmove_t *pm)
{
    playerState_s *ps; // [esp+8h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 6058, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 6060, 0, "%s", "ps") )
        __debugbreak();
    if ( !pm->oldcmd.button_bits.testBit(0x12u) && pm->cmd.button_bits.testBit(0x12u) )
    {
        if ( (ps->weapFlags & 0x40) != 0 )
        {
            ps->weapFlags &= ~0x40u;
            PM_AddEvent(ps, 0x5Du);
        }
        else
        {
            ps->weapFlags |= 0x40u;
            PM_AddEvent(ps, 0x5Cu);
        }
    }
}

void __cdecl PM_UpdatePush(pmove_t *pm, pml_t *pml)
{
    gjkcc_input_t gjkcc_in; // [esp+10h] [ebp-24h] BYREF
    playerState_s *ps; // [esp+30h] [ebp-4h]
    int savedregs; // [esp+34h] [ebp+0h] BYREF

    ps = pm->ps;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_pmove.cpp", 6088, 0, "%s", "ps") )
        __debugbreak();
    if ( playerPushAmount->current.value != 0.0 )
    {
        setup_gjkcc_input(pm, &gjkcc_in);
        gjk_sentient_push(
            pm,
            pml,
            ps->origin,
            pm->mins,
            pm->maxs,
            pm->tracemask,
            ps->clientNum,
            &gjkcc_in,
            playerPushAmount->current.value,
            pml->frametime,
            ps->velocity,
            ps->origin);
    }
}

void __cdecl set_stance(pmove_t *pm)
{
    playerState_s *ps; // [esp+8h] [ebp-4h]

    ps = pm->ps;
    pm->mins[0] = playerMins[0];
    pm->mins[1] = -15.0;
    pm->mins[2] = 0.0;
    pm->maxs[0] = playerMaxs[0];
    pm->maxs[1] = 15.0;
    pm->maxs[2] = 70.0;
    if ( (ps->pm_flags & 1) != 0 )
    {
        pm->maxs[2] = 30.0f;
    }
    else if ( (ps->pm_flags & 2) != 0 )
    {
        pm->maxs[2] = 50.0f;
    }
}

void __cdecl Pmove(pmove_t *pm)
{
    gjkcc_input_t gjkcc_in; // [esp+0h] [ebp-20h] BYREF
    int savedregs; // [esp+20h] [ebp+0h] BYREF

    setup_gjkcc_input(pm, &gjkcc_in);
    pm->m_gjkcc_input = &gjkcc_in;
    gjkcc_prolog(&gjkcc_in, pm->ps->origin);
    Pmove_1(pm);
    gjkcc_epilog(&gjkcc_in, pm->ps->origin);
    pm->m_gjkcc_input = 0;
}