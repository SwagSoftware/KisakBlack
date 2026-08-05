#include "actor_physics.h"

#include <cgame_mp/cg_predict_mp.h>
#include <bgame/bg_slidemove.h>
#include <bgame/bg_misc.h>
#include <game_mp/g_main_mp.h>
#include <game_mp/g_utils_mp.h>
#include "g_mover.h"
#include "bullet.h"
#include "actor_events.h"

actor_physics_local_t g_apl;
actor_physics_t *g_pPhys;

void __cdecl AIPhys_AddTouchEnt(actor_physics_t *pPhys, int entityNum)
{
    int i; // [esp+0h] [ebp-4h]

    if ( entityNum != 1022 && pPhys->iNumTouch != 32 )
    {
        for ( i = 0; i < pPhys->iNumTouch; ++i )
        {
            if ( pPhys->iTouchEnts[i] == entityNum )
                return;
        }
        pPhys->iTouchEnts[pPhys->iNumTouch++] = entityNum;
    }
}

void __cdecl setup_gjkcc_input(actor_physics_t *pPhys, gjkcc_input_t *gjkcc_in)
{
    gjkcc_in->gjkcc_id = (unsigned int)pPhys;
    gjkcc_in->is_server_thread = 1;
    gjkcc_in->proximity_data = &pPhys->proximity_data;
    gjkcc_in->proximity_mask = 0x2838013;
    gjkcc_in->m_ent_num = pPhys->iEntNum;
    gjkcc_in->m_gjk_query_flags = 3;
    gjkcc_in->m_gjk_cg = 0;
    gjkcc_in->m_mat = 0;
}

void __cdecl ai_physics_trace(
                trace_t *trace,
                float *start,
                float *mins,
                float *maxs,
                float *end,
                int passEntityNum,
                int contentmask,
                actor_physics_t *pPhys)
{
    col_context_t context; // [esp+70h] [ebp-4Ch] BYREF
    gjkcc_input_t v9; // [esp+98h] [ebp-24h] BYREF
    int savedregs; // [esp+BCh] [ebp+0h] BYREF

    if ( phys_ai_collision_mode->current.integer == 1 )
    {
        if ( pPhys )
        {
            gjk_player_trace(pPhys->m_gjkcc_input, trace, start, mins, maxs, end, passEntityNum, contentmask);
        }
        else
        {
            v9.gjkcc_id = 0;
            v9.is_server_thread = 1;
            v9.proximity_data = 0;
            v9.proximity_mask = 0;
            v9.m_ent_num = -1;
            v9.m_gjk_query_flags = 3;
            v9.m_gjk_cg = 0;
            v9.m_mat = 0;
            gjkcc_prolog(&v9, start);
            gjk_player_trace(&v9, trace, start, mins, maxs, end, passEntityNum, contentmask);
            gjkcc_epilog(&v9, start);
        }
    }
    else
    {
        //col_context_t::col_context_t(&context);
        //colgeom_visitor_inlined_t<200>::update(
            pPhys->proximity_data.update(
            start,
            end,
            mins,
            maxs,
            0x2838013);
        context.prims = pPhys->proximity_data.prims;
        context.nprims = pPhys->proximity_data.nprims;
        G_TraceCapsule(trace, start, mins, maxs, end, passEntityNum, contentmask, &context);
    }
}

void __cdecl setup_ai_gjk_slide_move_input(actor_physics_t *pPhys, ai_gjk_slide_move_input_t *input, int gravity)
{
    input->clientnum = pPhys->iEntNum;
    input->do_step_down = 0;
    input->frametime = g_apl.fFrameTime;
    input->gravity = (int)pPhys->fGravity;
    input->has_gravity = gravity;
    input->maxs = (const float (*)[3])pPhys->vMaxs;
    input->mins = (const float (*)[3])pPhys->vMins;
    input->position = (const float (*)[3])pPhys;
    input->tracemask = g_apl.iTraceMask;
    input->velocity = (const float (*)[3])pPhys->vVelocity;
    input->m_pPhys = pPhys;
}

void __cdecl AIPhys_FoliageSounds(actor_physics_t *pPhys)
{
    col_context_t context; // [esp+80h] [ebp-94h] BYREF
    int iInterval; // [esp+A8h] [ebp-6Ch]
    float fSpeedFrac; // [esp+ACh] [ebp-68h]
    float vVel[3]; // [esp+B0h] [ebp-64h] BYREF
    float xyspeed; // [esp+BCh] [ebp-58h]
    trace_t trace; // [esp+C0h] [ebp-54h] BYREF
    float vMins[3]; // [esp+FCh] [ebp-18h] BYREF
    float vMaxs[3]; // [esp+108h] [ebp-Ch] BYREF

    vVel[0] = pPhys->vVelocity[0];
    vVel[1] = pPhys->vVelocity[1];
    vVel[2] = 0.0f;
    xyspeed = Vec3Length(vVel);
    if ( bg_foliagesnd_minspeed->current.value <= xyspeed )
    {
        if ( (float)(bg_foliagesnd_maxspeed->current.value - bg_foliagesnd_minspeed->current.value) <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_physics.cpp",
                        977,
                        1,
                        "%s",
                        "bg_foliagesnd_maxspeed->current.value - bg_foliagesnd_minspeed->current.value > 0") )
        {
            __debugbreak();
        }
        fSpeedFrac = (float)(xyspeed - bg_foliagesnd_minspeed->current.value)
                             / (float)(bg_foliagesnd_maxspeed->current.value - bg_foliagesnd_minspeed->current.value);
        if ( fSpeedFrac > 1.0 )
            fSpeedFrac = 1.0f;
        iInterval = (int)(float)((float)((float)(bg_foliagesnd_fastinterval->current.integer
                                                                                     - bg_foliagesnd_slowinterval->current.integer)
                                                                     * fSpeedFrac)
                                                     + (float)bg_foliagesnd_slowinterval->current.integer);
        if ( iInterval + pPhys->foliageSoundTime < level.time )
        {
            vMins[0] = 0.75 * pPhys->vMins[0];
            vMins[1] = 0.75 * pPhys->vMins[1];
            vMins[2] = 0.75 * pPhys->vMins[2];
            vMaxs[0] = 0.75 * pPhys->vMaxs[0];
            vMaxs[1] = 0.75 * pPhys->vMaxs[1];
            vMaxs[2] = 0.75 * pPhys->vMaxs[2];
            vMaxs[2] = pPhys->vMaxs[2] * 0.89999998;
            //colgeom_visitor_inlined_t<200>::update(
                pPhys->proximity_data.update(
                pPhys->vOrigin,
                pPhys->vOrigin,
                vMins,
                vMaxs,
                0x2838013);
            //col_context_t::col_context_t(&context);
            context.prims = pPhys->proximity_data.prims;
            context.nprims = pPhys->proximity_data.nprims;
            G_TraceCapsule(&trace, pPhys->vOrigin, vMins, vMaxs, pPhys->vOrigin, pPhys->iEntNum, 2, &context);
            if ( trace.startsolid )
            {
                G_AddEvent(&g_entities[pPhys->iEntNum], 1u, 0);
                pPhys->foliageSoundTime = level.time;
            }
        }
    }
    else if ( bg_foliagesnd_resetinterval->current.integer + pPhys->foliageSoundTime < level.time )
    {
        pPhys->foliageSoundTime = 0;
    }
}

bool __cdecl Actor_Physics_1(actor_physics_t *pPhys)
{
    bool bSuccess; // [esp+2Ch] [ebp-4h]

    PROF_SCOPED("Actor_Physics");

    g_pPhys = pPhys;
    pPhys->iHitEntnum = 1023;
    pPhys->iNumTouch = 0;
    pPhys->bDeflected = 0;
    memset(&g_apl, 0, sizeof(g_apl));
    g_apl.iTraceMask = pPhys->iTraceMask;
    g_apl.vPrevOrigin[0] = pPhys->vOrigin[0];
    g_apl.vPrevOrigin[1] = pPhys->vOrigin[1];
    g_apl.vPrevOrigin[2] = pPhys->vOrigin[2];
    g_apl.vPrevVelocity[0] = pPhys->vVelocity[0];
    g_apl.vPrevVelocity[1] = pPhys->vVelocity[1];
    g_apl.vPrevVelocity[2] = pPhys->vVelocity[2];
    g_apl.fFrameTime = (float)pPhys->iMsec * 0.001;
    if ( pPhys->iMsec <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_physics.cpp",
                    1039,
                    0,
                    "%s",
                    "pPhys->iMsec > 0") )
    {
        __debugbreak();
    }
    if ( pPhys->prone )
        g_apl.stepheight = 10.0f;
    else
        g_apl.stepheight = 18.0f;
    pPhys->bHasGroundPlane = 0;
    pPhys->groundplaneSlope = 0.0f;
    if ( pPhys->ePhysicsType == AIPHYS_NOCLIP )
    {
        AIPhys_NoClipMove(pPhys);
        return 1;
    }
    else if ( pPhys->ePhysicsType == AIPHYS_NOGRAVITY )
    {
        AIPhys_NoGravityMove(pPhys);
        return 1;
    }
    else
    {
        AIPhys_GroundTrace(pPhys);
        if ( pPhys->ePhysicsType == AIPHYS_ZONLY_PHYSICS_RELATIVE )
        {
            bSuccess = AIPhys_ZOnlyPhysicsMove(pPhys);
        }
        else if ( g_apl.bIsWalking )
        {
            bSuccess = AIPhys_WalkMove(pPhys);
        }
        else
        {
            bSuccess = AIPhys_AirMove(pPhys);
        }
        if ( g_apl.bGroundPlane && g_apl.groundTrace.normal.vec.v[2] >= 0.30000001 )
        {
            pPhys->bHasGroundPlane = 1;
            pPhys->groundplaneSlope = g_apl.groundTrace.normal.vec.v[2];
            pPhys->iSurfaceType = (unsigned __int8)((int)(0x3F00000
                                                                                                    & g_apl.groundTrace.sflags) >> 20);
        }
        else
        {
            pPhys->iSurfaceType = 0;
        }
        return bSuccess;
    }
}

bool __cdecl AIPhys_AirMove(actor_physics_t *pPhys)
{
    if ( g_apl.bGroundPlane )
        AIPhys_ClipVelocity(
            pPhys->vVelocity,
            g_apl.groundTrace.normal.vec.v,
            g_apl.groundTrace.walkable,
            pPhys->vVelocity,
            1.001);
    return AIPhys_StepSlideMove(pPhys, 1, 0);
}

void __cdecl AIPhys_ClipVelocity(const float *in, const float *normal, bool isWalkable, float *out, float overbounce)
{
    int i; // [esp+4h] [ebp-8h]
    float backoff; // [esp+8h] [ebp-4h]
    float backoffa; // [esp+8h] [ebp-4h]

    if ( isWalkable && (float)((float)(*in * *in) + (float)(in[1] * in[1])) >= (float)(in[2] * in[2]) )
    {
        out[2] = (float)((-*in) * *normal) - (float)(in[1] * normal[1]);
        *out = *in * normal[2];
        out[1] = in[1] * normal[2];
    }
    else
    {
        backoff = (float)((float)(*in * *normal) + (float)(in[1] * normal[1])) + (float)(in[2] * normal[2]);
        if ( backoff >= 0.0 )
            backoffa = backoff / overbounce;
        else
            backoffa = backoff * overbounce;
        for ( i = 0; i < 3; ++i )
            out[i] = in[i] - (float)(normal[i] * backoffa);
    }
}

bool __cdecl AIPhys_StepSlideMove(actor_physics_t *pPhys, int gravity, int zonly)
{
    float v4; // [esp+10h] [ebp-95Ch]
    float diff2_4; // [esp+98h] [ebp-8D4h]
    actor_physics_local_t localPhys; // [esp+B4h] [ebp-8B8h] BYREF
    actor_physics_t phys; // [esp+11Ch] [ebp-850h] BYREF
    SlideMoveResult moveResult; // [esp+8ECh] [ebp-80h]
    float stepheight; // [esp+8F0h] [ebp-7Ch]
    float start_o[3]; // [esp+8F4h] [ebp-78h] BYREF
    trace_t trace; // [esp+900h] [ebp-6Ch] BYREF
    float endpos[3]; // [esp+93Ch] [ebp-30h] BYREF
    float start_v[3]; // [esp+948h] [ebp-24h]
    float up[3]; // [esp+954h] [ebp-18h] BYREF
    float down[3]; // [esp+960h] [ebp-Ch] BYREF

    //colgeom_visitor_inlined_t<200>::colgeom_visitor_inlined_t<200>(&phys.proximity_data);
    start_o[0] = pPhys->vOrigin[0];
    start_o[1] = pPhys->vOrigin[1];
    start_o[2] = pPhys->vOrigin[2];
    start_v[0] = pPhys->vVelocity[0];
    start_v[1] = pPhys->vVelocity[1];
    start_v[2] = pPhys->vVelocity[2];
    moveResult = (SlideMoveResult)AIPhys_SlideMove(pPhys, gravity, zonly);
    if ( moveResult == SLIDEMOVE_COMPLETE )
    {
        //phys.proximity_data.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&visitor_base_t::`vftable';
        return 1;
    }
    if ( moveResult == SLIDEMOVE_FAIL )
    {
        //phys.proximity_data.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&visitor_base_t::`vftable';
        return 0;
    }
    if ( moveResult != SLIDEMOVE_CLIPPED
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_physics.cpp",
                    566,
                    0,
                    "%s",
                    "moveResult == SLIDEMOVE_CLIPPED") )
    {
        __debugbreak();
    }
    stepheight = g_apl.stepheight;
    if ( pPhys->vVelocity[2] > 0.0 && !g_apl.bIsWalking )
    {
        down[0] = start_o[0];
        down[1] = start_o[1];
        down[2] = start_o[2] - stepheight;
        ai_physics_trace(&trace, start_o, pPhys->vMins, pPhys->vMaxs, down, pPhys->iEntNum, g_apl.iTraceMask, pPhys);
        if ( trace.fraction == 1.0 || !trace.walkable )
        {
            //phys.proximity_data.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&visitor_base_t::`vftable';
            return 1;
        }
    }
    up[0] = start_o[0];
    up[1] = start_o[1];
    up[2] = start_o[2] + stepheight;
    ai_physics_trace(&trace, start_o, pPhys->vMins, pPhys->vMaxs, up, pPhys->iEntNum, g_apl.iTraceMask, pPhys);
    if ( trace.allsolid )
    {
        //phys.proximity_data.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&visitor_base_t::`vftable';
        return trace.fraction != 0.0;
    }
    //actor_physics_t::operator=(&phys, pPhys);
    phys = *pPhys;
    memcpy(&localPhys, &g_apl, sizeof(localPhys));
    pPhys->iHitEntnum = 1023;
    g_apl.bGroundPlane = 0;
    Vec3Lerp(start_o, up, trace.fraction, endpos);
    pPhys->vOrigin[0] = endpos[0];
    pPhys->vOrigin[1] = endpos[1];
    pPhys->vOrigin[2] = endpos[2];
    pPhys->vVelocity[0] = start_v[0];
    pPhys->vVelocity[1] = start_v[1];
    pPhys->vVelocity[2] = start_v[2];
    moveResult = (SlideMoveResult)AIPhys_SlideMove(pPhys, gravity, zonly);
    if ( moveResult == SLIDEMOVE_FAIL )
        goto LABEL_22;
    diff2_4 = (float)((float)(g_apl.fFrameTime * start_v[1]) + start_o[1]) - pPhys->vOrigin[1];
    if ( (float)((float)((float)((float)((float)(g_apl.fFrameTime * start_v[0]) + start_o[0]) - pPhys->vOrigin[0])
                                         * (float)((float)((float)(g_apl.fFrameTime * start_v[0]) + start_o[0]) - pPhys->vOrigin[0]))
                         + (float)(diff2_4 * diff2_4)) > (float)((float)((float)((float)((float)(g_apl.fFrameTime * start_v[0])
                                                                                                                                                     + start_o[0])
                                                                                                                                     - phys.vOrigin[0])
                                                                                                                     * (float)((float)((float)(g_apl.fFrameTime * start_v[0])
                                                                                                                                                     + start_o[0])
                                                                                                                                     - phys.vOrigin[0]))
                                                                                                     + (float)((float)((float)((float)(g_apl.fFrameTime * start_v[1])
                                                                                                                                                     + start_o[1])
                                                                                                                                     - phys.vOrigin[1])
                                                                                                                     * (float)((float)((float)(g_apl.fFrameTime * start_v[1])
                                                                                                                                                     + start_o[1])
                                                                                                                                     - phys.vOrigin[1]))) )
        goto LABEL_22;
    down[0] = pPhys->vOrigin[0];
    down[1] = pPhys->vOrigin[1];
    down[2] = pPhys->vOrigin[2];
    down[2] = (float)(start_o[2] - endpos[2]) + down[2];
    ai_physics_trace(&trace, pPhys->vOrigin, pPhys->vMins, pPhys->vMaxs, down, pPhys->iEntNum, g_apl.iTraceMask, pPhys);
    if ( !trace.startsolid )
        Vec3Lerp(pPhys->vOrigin, down, trace.fraction, pPhys->vOrigin);
    if ( trace.fraction < 1.0 )
    {
        if ( trace.normal.vec.v[2] < 0.30000001 )
        {
LABEL_22:
            //actor_physics_t::operator=(pPhys, &phys);
            *pPhys = phys;
            memcpy(&g_apl, &localPhys, sizeof(g_apl));
            //phys.proximity_data.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&visitor_base_t::`vftable';
            return 1;
        }
        AIPhys_ClipVelocity(pPhys->vVelocity, trace.normal.vec.v, trace.walkable, pPhys->vVelocity, 1.001);
        if ( (float)(start_o[2] - phys.vOrigin[2]) < 0.0 )
            v4 = phys.vOrigin[2];
        else
            v4 = start_o[2];
        if ( pPhys->vOrigin[2] > (float)(v4 + 0.1) )
            pPhys->iHitEntnum = 1023;
    }
    return 1;
}

int __cdecl AIPhys_SlideMove(actor_physics_t *pPhys, int gravity, int zonly)
{
    unsigned __int16 EntityHitId; // ax
    float *v5; // [esp+54h] [ebp-184h]
    ai_gjk_slide_move_input_t input; // [esp+98h] [ebp-140h] BYREF
    int retv; // [esp+C8h] [ebp-110h]
    gjk_slide_move_output_t output; // [esp+CCh] [ebp-10Ch] BYREF
    const gjkcc_input_t *gjkcc_in; // [esp+E8h] [ebp-F0h]
    int j; // [esp+ECh] [ebp-ECh]
    float d; // [esp+F0h] [ebp-E8h]
    float fInto; // [esp+F4h] [ebp-E4h]
    float vDir[3]; // [esp+F8h] [ebp-E0h] BYREF
    int k; // [esp+104h] [ebp-D4h]
    float fTimeLeft; // [esp+108h] [ebp-D0h]
    float planes[5][3]; // [esp+10Ch] [ebp-CCh] BYREF
    float vEnd[3]; // [esp+148h] [ebp-90h] BYREF
    float vEndVelocity[3]; // [esp+154h] [ebp-84h] BYREF
    float vEndClipVelocity[3]; // [esp+160h] [ebp-78h] BYREF
    int iBumpCount; // [esp+16Ch] [ebp-6Ch]
    trace_t trace; // [esp+170h] [ebp-68h] BYREF
    float vClipVelocity[3]; // [esp+1A8h] [ebp-30h] BYREF
    int iMaxBumps; // [esp+1B4h] [ebp-24h]
    int i; // [esp+1B8h] [ebp-20h]
    int iNumPlanes; // [esp+1BCh] [ebp-1Ch]
    float vPrimalVelocity[3]; // [esp+1C0h] [ebp-18h]
    bool isWalkable[8]; // [esp+1CCh] [ebp-Ch]

    if ( phys_ai_collision_mode->current.integer == 1 )
    {
        //input.__vftable = (ai_gjk_slide_move_input_t_vtbl *)&ai_gjk_slide_move_input_t::`vftable';
        setup_ai_gjk_slide_move_input(pPhys, &input, gravity);
        gjkcc_in = pPhys->m_gjkcc_input;
        retv = gjk_slide_move(gjkcc_in, &input, &output);
        pPhys->bStuck = 0;
        pPhys->vOrigin[0] = output.new_position[0];
        pPhys->vOrigin[1] = output.new_position[1];
        pPhys->vOrigin[2] = output.new_position[2];
        pPhys->vVelocity[0] = output.new_velocity[0];
        pPhys->vVelocity[1] = output.new_velocity[1];
        pPhys->vVelocity[2] = output.new_velocity[2];
        return retv != 0;
    }
    memset(&trace, 0, 16);
    iMaxBumps = 4;
    vPrimalVelocity[0] = pPhys->vVelocity[0];
    vPrimalVelocity[1] = pPhys->vVelocity[1];
    vPrimalVelocity[2] = pPhys->vVelocity[2];
    vEndVelocity[0] = pPhys->vVelocity[0];
    vEndVelocity[1] = pPhys->vVelocity[1];
    vEndVelocity[2] = pPhys->vVelocity[2];
    if ( gravity )
    {
        vEndVelocity[2] = vEndVelocity[2] - (float)(pPhys->fGravity * g_apl.fFrameTime);
        pPhys->vVelocity[2] = (float)(pPhys->vVelocity[2] + vEndVelocity[2]) * 0.5;
        vPrimalVelocity[2] = vEndVelocity[2];
        if ( g_apl.bGroundPlane )
        {
            if ( !zonly )
                AIPhys_ClipVelocity(
                    pPhys->vVelocity,
                    g_apl.groundTrace.normal.vec.v,
                    g_apl.groundTrace.walkable,
                    pPhys->vVelocity,
                    1.001);
        }
    }
    fTimeLeft = g_apl.fFrameTime;
    if ( g_apl.bGroundPlane )
    {
        iNumPlanes = 1;
        *(_QWORD *)&planes[0][0] = *(_QWORD *)g_apl.groundTrace.normal.vec.v;
        LODWORD(planes[0][2]) = g_apl.groundTrace.normal.vec.u[2];
        isWalkable[0] = g_apl.groundTrace.walkable;
    }
    else
    {
        iNumPlanes = 0;
    }
    Vec3NormalizeTo(pPhys->vVelocity, planes[iNumPlanes]);
    isWalkable[iNumPlanes++] = pPhys->vVelocity[2] > 0.69999999;
    for ( iBumpCount = 0; ; ++iBumpCount )
    {
        if ( iBumpCount >= iMaxBumps )
        {
LABEL_66:
            if ( gravity )
            {
                Vec3Copy(vEndVelocity, g_pPhys->vVelocity);
            }
            return iBumpCount != 0;
        }
        Vec3Mad(pPhys->vOrigin, fTimeLeft, pPhys->vVelocity, vEnd);

        ai_physics_trace(&trace, pPhys->vOrigin, pPhys->vMins, pPhys->vMaxs, vEnd, pPhys->iEntNum, g_apl.iTraceMask, pPhys);
        if ( trace.fraction <= 0.0 )
            break;
        Vec3Lerp(pPhys->vOrigin, vEnd, trace.fraction, pPhys->vOrigin);
        if ( trace.fraction == 1.0 )
            goto LABEL_66;
LABEL_23:
        pPhys->bDeflected = 1;
        if ( !trace.walkable && pPhys->iHitEntnum == 1023 && (trace.normal.vec.v[0] != 0.0 || trace.normal.vec.v[1] != 0.0) )
        {
            pPhys->iHitEntnum = Trace_GetEntityHitId(&trace);
            pPhys->vHitOrigin[0] = pPhys->vOrigin[0];
            pPhys->vHitOrigin[1] = pPhys->vOrigin[1];
            *(_QWORD *)pPhys->vHitNormal = *(_QWORD *)trace.normal.vec.v;
            pPhys->bStuck = 0;
        }
        EntityHitId = Trace_GetEntityHitId(&trace);
        AIPhys_AddTouchEnt(pPhys, EntityHitId);
        fTimeLeft = fTimeLeft - (float)(fTimeLeft * trace.fraction);
        if ( iNumPlanes >= 5 )
        {
            Vec3Clear(pPhys->vVelocity);
            return SLIDEMOVE_CLIPPED;
        }
        for ( i = 0; i < iNumPlanes; ++i )
        {
            if ( (float)((float)((float)(trace.normal.vec.v[0] * planes[i][0]) + (float)(trace.normal.vec.v[1] * planes[i][1]))
                                 + (float)(trace.normal.vec.v[2] * planes[i][2])) > 0.99 )
            {
                if ( trace.fraction == 0.0
                    && pPhys->iHitEntnum == 1023
                    && (trace.normal.vec.v[0] != 0.0 || trace.normal.vec.v[1] != 0.0) )
                {
                    pPhys->iHitEntnum = Trace_GetEntityHitId(&trace);
                    pPhys->vHitOrigin[0] = pPhys->vOrigin[0];
                    pPhys->vHitOrigin[1] = pPhys->vOrigin[1];
                    *(_QWORD *)pPhys->vHitNormal = *(_QWORD *)trace.normal.vec.v;
                    pPhys->bStuck = 0;
                }
                pPhys->vVelocity[0] = trace.normal.vec.v[0] + pPhys->vVelocity[0];
                pPhys->vVelocity[1] = trace.normal.vec.v[1] + pPhys->vVelocity[1];
                pPhys->vVelocity[2] = trace.normal.vec.v[2] + pPhys->vVelocity[2];
                break;
            }
        }
        if ( i >= iNumPlanes )
        {
            v5 = planes[iNumPlanes];
            *(_QWORD *)v5 = *(_QWORD *)trace.normal.vec.v;
            v5[2] = trace.normal.vec.v[2];
            isWalkable[iNumPlanes++] = trace.walkable;
            for ( i = 0; i < iNumPlanes; ++i )
            {
                fInto = (float)((float)(pPhys->vVelocity[0] * planes[i][0]) + (float)(pPhys->vVelocity[1] * planes[i][1]))
                            + (float)(pPhys->vVelocity[2] * planes[i][2]);
                if ( fInto < 0.1 )
                {
                    if ( (-(fInto)) > g_apl.fImpactSpeed )
                        g_apl.fImpactSpeed = -fInto;
                    AIPhys_ClipVelocity(pPhys->vVelocity, planes[i], isWalkable[i], vClipVelocity, 1.001);
                    AIPhys_ClipVelocity(vEndVelocity, planes[i], isWalkable[i], vEndClipVelocity, 1.001);
                    for ( j = 0; j < iNumPlanes; ++j )
                    {
                        if ( j != i
                            && (float)((float)((float)(vClipVelocity[0] * planes[j][0]) + (float)(vClipVelocity[1] * planes[j][1]))
                                             + (float)(vClipVelocity[2] * planes[j][2])) < 0.1 )
                        {
                            AIPhys_ClipVelocity(vClipVelocity, planes[j], isWalkable[j], vClipVelocity, 1.001);
                            AIPhys_ClipVelocity(vEndClipVelocity, planes[j], isWalkable[j], vEndClipVelocity, 1.001);
                            if ( (float)((float)((float)(vClipVelocity[0] * planes[i][0]) + (float)(vClipVelocity[1] * planes[i][1]))
                                                 + (float)(vClipVelocity[2] * planes[i][2])) < 0.0 )
                            {
                                Vec3Cross(planes[i], planes[j], vDir);
                                Vec3Normalize(vDir);
                                d = (float)((float)(vDir[0] * pPhys->vVelocity[0]) + (float)(vDir[1] * pPhys->vVelocity[1]))
                                    + (float)(vDir[2] * pPhys->vVelocity[2]);
                                vClipVelocity[0] = d * vDir[0];
                                vClipVelocity[1] = d * vDir[1];
                                vClipVelocity[2] = d * vDir[2];
                                Vec3Cross(planes[i], planes[j], vDir);
                                Vec3Normalize(vDir);
                                d = (float)((float)(vDir[0] * vEndVelocity[0]) + (float)(vDir[1] * vEndVelocity[1]))
                                    + (float)(vDir[2] * vEndVelocity[2]);
                                vEndClipVelocity[0] = d * vDir[0];
                                vEndClipVelocity[1] = d * vDir[1];
                                vEndClipVelocity[2] = d * vDir[2];
                                for ( k = 0; k < iNumPlanes; ++k )
                                {
                                    if ( k != i
                                        && k != j
                                        && (float)((float)((float)(vClipVelocity[0] * planes[k][0])
                                                                         + (float)(vClipVelocity[1] * planes[k][1]))
                                                         + (float)(vClipVelocity[2] * planes[k][2])) < 0.1 )
                                    {
                                        pPhys->vVelocity[0] = 0.0f;
                                        pPhys->vVelocity[1] = 0.0f;
                                        pPhys->vVelocity[2] = 0.0f;
                                        return 1;
                                    }
                                }
                            }
                        }
                    }
                    if ( pPhys->iHitEntnum == 1023 )
                    {
                        vDir[0] = vClipVelocity[0] - pPhys->vVelocity[0];
                        vDir[1] = vClipVelocity[1] - pPhys->vVelocity[1];
                        pPhys->iHitEntnum = Trace_GetEntityHitId(&trace);
                        pPhys->vHitOrigin[0] = pPhys->vOrigin[0];
                        pPhys->vHitOrigin[1] = pPhys->vOrigin[1];
                        pPhys->vHitNormal[0] = vDir[0];
                        pPhys->vHitNormal[1] = vDir[1];
                        pPhys->bStuck = 0;
                    }
                    pPhys->vVelocity[0] = vClipVelocity[0];
                    pPhys->vVelocity[1] = vClipVelocity[1];
                    pPhys->vVelocity[2] = vClipVelocity[2];
                    vEndVelocity[0] = vEndClipVelocity[0];
                    vEndVelocity[1] = vEndClipVelocity[1];
                    vEndVelocity[2] = vEndClipVelocity[2];
                    break;
                }
            }
        }
    }
    if ( !trace.startsolid )
        goto LABEL_23;
    pPhys->vVelocity[2] = 0.0f;
    if ( !iBumpCount && g_apl.groundTrace.startsolid )
        return 2;
    if ( pPhys->iHitEntnum == 1023 )
    {
        pPhys->iHitEntnum = Trace_GetEntityHitId(&trace);
        pPhys->vHitOrigin[0] = pPhys->vOrigin[0];
        pPhys->vHitOrigin[1] = pPhys->vOrigin[1];
        pPhys->bStuck = 1;
    }
    return 1;
}

void __thiscall ai_gjk_slide_move_input_t::custom_process(gjk_trace_output_t *gto)
{
    unsigned __int16 EntityHitId; // ax
    unsigned __int16 EntID; // [esp+Ch] [ebp-40h]
    trace_t trace; // [esp+10h] [ebp-3Ch] BYREF

    this->m_pPhys->bDeflected = 1;
    fill_results_type_and_id(gto, &trace);
    EntID = Trace_GetEntityHitId(&trace);
    EntityHitId = Trace_GetEntityHitId(&trace);
    AIPhys_AddTouchEnt(this->m_pPhys, EntityHitId);
    if ( this->m_pPhys->iHitEntnum == 1023 && (gto->m_hit_normal.x != 0.0 || gto->m_hit_normal.y != 0.0) )
    {
        this->m_pPhys->iHitEntnum = EntID;
        Phys_NitrousVecToVec3(&gto->m_hit_point, this->m_pPhys->vHitOrigin);
        Phys_NitrousVecToVec3(&gto->m_hit_normal, this->m_pPhys->vHitNormal);
    }
}

// default should be ok I guess
//actor_physics_t *__thiscall actor_physics_t::operator=(actor_physics_t *this, const actor_physics_t *__that)
//{
//    unsigned int _S9; // [esp+Ch] [ebp-20h]
//    unsigned int _S8; // [esp+10h] [ebp-1Ch]
//    unsigned int _S7; // [esp+14h] [ebp-18h]
//    unsigned int _S6; // [esp+18h] [ebp-14h]
//    unsigned int _S5; // [esp+1Ch] [ebp-10h]
//    unsigned int _S4; // [esp+20h] [ebp-Ch]
//    unsigned int _S3; // [esp+24h] [ebp-8h]
//    unsigned int _S2; // [esp+28h] [ebp-4h]
//
//    for ( _S2 = 0; _S2 < 3; ++_S2 )
//        this->vOrigin[_S2] = __that->vOrigin[_S2];
//    for ( _S3 = 0; _S3 < 3; ++_S3 )
//        this->vVelocity[_S3] = __that->vVelocity[_S3];
//    this->groundEntNum = __that->groundEntNum;
//    this->iFootstepTimer = __that->iFootstepTimer;
//    this->bHasGroundPlane = __that->bHasGroundPlane;
//    this->groundplaneSlope = __that->groundplaneSlope;
//    this->iSurfaceType = __that->iSurfaceType;
//    for ( _S4 = 0; _S4 < 3; ++_S4 )
//        this->vWishDelta[_S4] = __that->vWishDelta[_S4];
//    this->bIsAlive = __that->bIsAlive;
//    this->iEntNum = __that->iEntNum;
//    this->ePhysicsType = __that->ePhysicsType;
//    this->fGravity = __that->fGravity;
//    this->iMsec = __that->iMsec;
//    for ( _S5 = 0; _S5 < 3; ++_S5 )
//        this->vMins[_S5] = __that->vMins[_S5];
//    for ( _S6 = 0; _S6 < 3; ++_S6 )
//        this->vMaxs[_S6] = __that->vMaxs[_S6];
//    this->prone = __that->prone;
//    this->iTraceMask = __that->iTraceMask;
//    this->foliageSoundTime = __that->foliageSoundTime;
//    this->iNumTouch = __that->iNumTouch;
//    for ( _S7 = 0; _S7 < 0x20; ++_S7 )
//        this->iTouchEnts[_S7] = __that->iTouchEnts[_S7];
//    this->iHitEntnum = __that->iHitEntnum;
//    for ( _S8 = 0; _S8 < 3; ++_S8 )
//        this->vHitOrigin[_S8] = __that->vHitOrigin[_S8];
//    for ( _S9 = 0; _S9 < 3; ++_S9 )
//        this->vHitNormal[_S9] = __that->vHitNormal[_S9];
//    this->bStuck = __that->bStuck;
//    this->bDeflected = __that->bDeflected;
//    this->m_gjkcc_input = __that->m_gjkcc_input;
//    colgeom_visitor_inlined_t<200>::operator=(&this->proximity_data, &__that->proximity_data);
//    return this;
//}

//colgeom_visitor_inlined_t<200> *__thiscall colgeom_visitor_inlined_t<200>::operator=(const colgeom_visitor_inlined_t<200> *__that)
//{
//    const CollisionAabbTree *tree; // edx
//    unsigned int _S10; // [esp+8h] [ebp-4h]
//
//    colgeom_visitor_t::operator=(this, __that);
//    this->nprims = __that->nprims;
//    this->overflow = __that->overflow;
//    for ( _S10 = 0; _S10 < 0xC8; ++_S10 )
//    {
//        tree = __that->prims[_S10].tree;
//        this->prims[_S10].type = __that->prims[_S10].type;
//        this->prims[_S10].tree = tree;
//    }
//    return this;
//}
//
//colgeom_visitor_t *__thiscall colgeom_visitor_t::operator=(colgeom_visitor_t *this, const colgeom_visitor_t *__that)
//{
//    this->m_mn = __that->m_mn;
//    this->m_mx = __that->m_mx;
//    this->m_p0 = __that->m_p0;
//    this->m_p1 = __that->m_p1;
//    this->m_delta = __that->m_delta;
//    this->m_rvec = __that->m_rvec;
//    this->m_radius = __that->m_radius;
//    this->m_mask = __that->m_mask;
//    this->m_threadInfo = __that->m_threadInfo;
//    return this;
//}
//

bool __cdecl AIPhys_WalkMove(actor_physics_t *pPhys)
{
    float v2; // [esp+28h] [ebp-14h]
    float vel; // [esp+30h] [ebp-Ch]
    float oldvel; // [esp+34h] [ebp-8h]
    float oldvel_4; // [esp+38h] [ebp-4h]

    v2 = 1.0 / g_apl.fFrameTime;
    pPhys->vVelocity[0] = (float)(1.0 / g_apl.fFrameTime) * pPhys->vWishDelta[0];
    pPhys->vVelocity[1] = v2 * pPhys->vWishDelta[1];
    pPhys->vVelocity[2] = 0.0f;
    vel = Vec2Length(pPhys->vVelocity);
    oldvel = pPhys->vVelocity[0];
    oldvel_4 = pPhys->vVelocity[1];
    AIPhys_ClipVelocity(
        pPhys->vVelocity,
        g_apl.groundTrace.normal.vec.v,
        g_apl.groundTrace.walkable,
        pPhys->vVelocity,
        1.001);
    if ( (float)((float)(pPhys->vVelocity[0] * oldvel) + (float)(pPhys->vVelocity[1] * oldvel_4)) > 0.0 )
    {
        Vec3Normalize(pPhys->vVelocity);
        pPhys->vVelocity[0] = vel * pPhys->vVelocity[0];
        pPhys->vVelocity[1] = vel * pPhys->vVelocity[1];
        pPhys->vVelocity[2] = vel * pPhys->vVelocity[2];
    }
    return pPhys->vVelocity[0] == 0.0 && pPhys->vVelocity[1] == 0.0 && g_apl.bGroundPlane
            || AIPhys_StepSlideMove(pPhys, 0, 0);
}

bool __cdecl AIPhys_ZOnlyPhysicsMove(actor_physics_t *pPhys)
{
    float v2; // [esp+4h] [ebp-8h]

    v2 = 1.0 / g_apl.fFrameTime;
    pPhys->vVelocity[0] = (float)(1.0 / g_apl.fFrameTime) * pPhys->vWishDelta[0];
    pPhys->vVelocity[1] = v2 * pPhys->vWishDelta[1];
    pPhys->vVelocity[2] = 0.0f;
    return AIPhys_StepSlideMove(pPhys, 1, 1);
}

void __cdecl AIPhys_NoClipMove(actor_physics_t *pPhys)
{
    pPhys->vOrigin[0] = pPhys->vOrigin[0] + pPhys->vWishDelta[0];
    pPhys->vOrigin[1] = pPhys->vOrigin[1] + pPhys->vWishDelta[1];
    pPhys->vOrigin[2] = pPhys->vOrigin[2] + pPhys->vWishDelta[2];
}

void __cdecl AIPhys_NoGravityMove(actor_physics_t *pPhys)
{
    float v1; // [esp+4h] [ebp-8h]

    v1 = 1.0 / g_apl.fFrameTime;
    pPhys->vVelocity[0] = (float)(1.0 / g_apl.fFrameTime) * pPhys->vWishDelta[0];
    pPhys->vVelocity[1] = v1 * pPhys->vWishDelta[1];
    pPhys->vVelocity[2] = v1 * pPhys->vWishDelta[2];
    AIPhys_SlideMove(pPhys, 0, 1);
}

void __cdecl AIPhys_GroundTrace(actor_physics_t *pPhys)
{
    unsigned __int16 EntityHitId; // ax
    float stepheight; // [esp+10h] [ebp-84h]
    bool v3; // [esp+17h] [ebp-7Dh]
    float start[3]; // [esp+24h] [ebp-70h] BYREF
    float end[3]; // [esp+30h] [ebp-64h] BYREF
    bool use_big_trace; // [esp+3Fh] [ebp-55h]
    float endpos[3]; // [esp+40h] [ebp-54h] BYREF
    trace_t trace; // [esp+4Ch] [ebp-48h] BYREF
    float first_hit_time; // [esp+88h] [ebp-Ch] BYREF
    bool last_ground_entity_was_a_mover; // [esp+8Fh] [ebp-5h]
    const gjkcc_input_t *gjkcc_in; // [esp+90h] [ebp-4h]

    last_ground_entity_was_a_mover = entity_is_a_mover(pPhys->groundEntNum);
    start[0] = pPhys->vOrigin[0];
    start[1] = pPhys->vOrigin[1];
    end[0] = pPhys->vOrigin[0];
    end[1] = pPhys->vOrigin[1];
    start[2] = pPhys->vOrigin[2] + 0.25;
    v3 = last_ground_entity_was_a_mover || pPhys->vVelocity[2] <= 0.0 && pPhys->groundEntNum != 1023;
    use_big_trace = v3;
    if ( v3 )
        stepheight = g_apl.stepheight;
    else
        stepheight = 0.25f;
    end[2] = pPhys->vOrigin[2] - stepheight;
    gjkcc_in = pPhys->m_gjkcc_input;
    first_hit_time = 1.0f;
    if ( phys_ai_collision_mode->current.integer == 1 )
        PM_gjk_ground_trace(
            gjkcc_in,
            &trace,
            start,
            pPhys->vMins,
            pPhys->vMaxs,
            end,
            pPhys->iEntNum,
            g_apl.iTraceMask,
            &first_hit_time);
    else
        ai_physics_trace(&trace, start, pPhys->vMins, pPhys->vMaxs, end, pPhys->iEntNum, g_apl.iTraceMask, pPhys);
    memcpy(&g_apl.groundTrace, &trace, sizeof(g_apl.groundTrace));
    if ( !trace.startsolid )
        goto LABEL_24;
    if ((trace.cflags & 0x2000000) != 0)
    {
        EntityHitId = Trace_GetEntityHitId(&trace);
        AIPhys_AddTouchEnt(pPhys, EntityHitId);
        g_apl.iTraceMask &= ~0x2000000u;
        if ( phys_ai_collision_mode->current.integer == 1 )
            PM_gjk_ground_trace(
                gjkcc_in,
                &trace,
                start,
                pPhys->vMins,
                pPhys->vMaxs,
                end,
                pPhys->iEntNum,
                g_apl.iTraceMask,
                &first_hit_time);
        else
            ai_physics_trace(&trace, start, pPhys->vMins, pPhys->vMaxs, end, pPhys->iEntNum, g_apl.iTraceMask, pPhys);
        memcpy(&g_apl.groundTrace, &trace, sizeof(g_apl.groundTrace));
    }
    if ( !trace.startsolid )
        goto LABEL_24;
    start[0] = pPhys->vOrigin[0];
    start[1] = pPhys->vOrigin[1];
    start[2] = pPhys->vOrigin[2];
    if ( phys_ai_collision_mode->current.integer == 1 )
        PM_gjk_ground_trace(
            gjkcc_in,
            &trace,
            start,
            pPhys->vMins,
            pPhys->vMaxs,
            end,
            pPhys->iEntNum,
            g_apl.iTraceMask,
            &first_hit_time);
    else
        ai_physics_trace(&trace, start, pPhys->vMins, pPhys->vMaxs, end, pPhys->iEntNum, g_apl.iTraceMask, pPhys);
    memcpy(&g_apl.groundTrace, &trace, sizeof(g_apl.groundTrace));
    if ( trace.startsolid )
    {
        pPhys->groundEntNum = Trace_GetEntityHitId(&trace);
        g_apl.bGroundPlane = 0;
        g_apl.bIsWalking = 1;
    }
    else
    {
LABEL_24:
        if ( trace.fraction == 1.0 )
        {
            pPhys->groundEntNum = 1023;
            g_apl.bGroundPlane = 0;
            g_apl.bIsWalking = 0;
        }
        else if ( pPhys->vVelocity[2] <= 0.0
                     || (float)((float)((float)(pPhys->vVelocity[0] * trace.normal.vec.v[0])
                                                        + (float)(pPhys->vVelocity[1] * trace.normal.vec.v[1]))
                                        + (float)(pPhys->vVelocity[2] * trace.normal.vec.v[2])) <= 10.0
                     || last_ground_entity_was_a_mover )
        {
            if ( trace.walkable )
            {
                if ( phys_ai_collision_mode->current.integer == 1 )
                    Vec3Lerp(start, end, first_hit_time, endpos);
                else
                    Vec3Lerp(start, end, trace.fraction, endpos);
                if ( pPhys->vOrigin[2] > endpos[2] || pPhys->ePhysicsType == AIPHYS_ZONLY_PHYSICS_RELATIVE )
                    pPhys->vOrigin[2] = endpos[2];
                g_apl.bGroundPlane = 1;
                g_apl.bIsWalking = 1;
                pPhys->groundEntNum = Trace_GetEntityHitId(&trace);
                AIPhys_AddTouchEnt(pPhys, pPhys->groundEntNum);
            }
            else
            {
                pPhys->groundEntNum = 1023;
                g_apl.bGroundPlane = 1;
                g_apl.bIsWalking = 0;
                if ( pPhys->iHitEntnum == 1023 && (trace.normal.vec.v[0] != 0.0 || trace.normal.vec.v[1] != 0.0) )
                {
                    pPhys->iHitEntnum = Trace_GetEntityHitId(&trace);
                    pPhys->vHitOrigin[0] = pPhys->vOrigin[0];
                    pPhys->vHitOrigin[1] = pPhys->vOrigin[1];
                    *(_QWORD *)pPhys->vHitNormal = *(_QWORD *)trace.normal.vec.v;
                    pPhys->bStuck = 0;
                }
            }
        }
        else
        {
            pPhys->groundEntNum = 1023;
            g_apl.bGroundPlane = 0;
            g_apl.bIsWalking = 0;
        }
    }
}

int __cdecl Actor_Physics(actor_physics_t *pPhys)
{
    int retv; // [esp+0h] [ebp-4h]
    int savedregs; // [esp+4h] [ebp+0h] BYREF

    retv = Actor_Physics_1(pPhys);
    if ( render_actor_collision->current.integer == 1 )
        render_gjkcc_collision(
            (float (*)[3])pPhys->vMins,
            (float (*)[3])pPhys->vMaxs,
            (float (*)[3])pPhys);
    return retv;
}

int __cdecl Actor_Physics_z(actor_physics_t *pPhys)
{
    int retv; // [esp+0h] [ebp-24h]
    gjkcc_input_t gjkcc_in; // [esp+4h] [ebp-20h] BYREF
    int savedregs; // [esp+24h] [ebp+0h] BYREF

    setup_gjkcc_input(pPhys, &gjkcc_in);
    pPhys->m_gjkcc_input = &gjkcc_in;
    gjkcc_prolog(&gjkcc_in, pPhys->vOrigin);
    retv = Actor_Physics(pPhys);
    gjkcc_epilog(&gjkcc_in, pPhys->vOrigin);
    pPhys->m_gjkcc_input = 0;
    return retv;
}

void __cdecl Actor_PostPhysics(actor_physics_t *pPhys)
{
    g_pPhys = pPhys;
    if ( pPhys->bIsAlive )
        AIPhys_Footsteps(pPhys);
    AIPhys_FoliageSounds(pPhys);
}

void __cdecl AIPhys_Footsteps(actor_physics_t *pPhys)
{
    char v1; // al
    gentity_s *ent; // [esp+14h] [ebp-Ch]
    int iOldFootstepTimer; // [esp+1Ch] [ebp-4h]

    iOldFootstepTimer = pPhys->iFootstepTimer;
    pPhys->iFootstepTimer = 0;
    if ( pPhys->groundEntNum != 1023
        && (pPhys->ePhysicsType == AIPHYS_NORMAL_ABSOLUTE
         || pPhys->ePhysicsType == AIPHYS_NORMAL_RELATIVE
         || pPhys->ePhysicsType == AIPHYS_ZONLY_PHYSICS_RELATIVE) )
    {
        pPhys->iFootstepTimer = pPhys->iMsec + iOldFootstepTimer;
        if ( pPhys->iFootstepTimer >= 500 )
        {
            ent = &level.gentities[pPhys->iEntNum];
            if ( !ent->r.inuse
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_physics.cpp", 935, 0, "%s", "ent->r.inuse") )
            {
                __debugbreak();
            }
            if ( !ent->actor
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_physics.cpp", 936, 0, "%s", "ent->actor") )
            {
                __debugbreak();
            }
            if ( !ent->sentient
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_physics.cpp",
                            937,
                            0,
                            "%s",
                            "ent->sentient") )
            {
                __debugbreak();
            }
            v1 = Sentient_EnemyTeam(ent->sentient->eTeam);
            Actor_BroadcastPointEvent(ent, AI_EV_FOOTSTEP, 1 << v1, pPhys->vOrigin, 0.0);
            pPhys->iFootstepTimer %= 500;
        }
    }
}

//colgeom_visitor_inlined_t<200> *__thiscall colgeom_visitor_inlined_t<200>::colgeom_visitor_inlined_t<200>(
//                colgeom_visitor_inlined_t<200> *this)
//{
//    colgeom_visitor_t::colgeom_visitor_t(this);
//    this->__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&colgeom_visitor_inlined_t<200>::`vftable';
//    colgeom_visitor_inlined_t<500>::reset(this);
//    return this;
//}
//
//