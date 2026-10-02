#include "g_scr_mover.h"
#include <game_mp/g_main_mp.h>
#include <clientscript/cscr_vm.h>
#include <bgame/bg_misc.h>
#include <server/sv_world.h>
#include <game_mp/g_spawn_mp.h>
#include <clientscript/scr_const.h>
#include <server/sv_game.h>
#include <game_mp/g_utils_mp.h>
#include <qcommon/dobj_management.h>
#include <universal/com_math_anglevectors.h>
#include <qcommon/cm_load.h>
#include <clientscript/cscr_stringlist.h>
#include <cgame/cg_scr_main.h>

void __cdecl Reached_ScriptMover(gentity_s *pEnt)
{
    int bMoveFinished; // [esp+20h] [ebp-4h]
    int bMoveFinisheda; // [esp+20h] [ebp-4h]
    int savedregs; // [esp+24h] [ebp+0h] BYREF

    if ( pEnt->s.lerp.pos.trType )
    {
        if ( pEnt->s.lerp.pos.trDuration + pEnt->s.lerp.pos.trTime <= level.time )
        {
            bMoveFinished = ScriptMover_UpdateMove(
                                                &pEnt->s.lerp.pos,
                                                pEnt->r.currentOrigin,
                                                pEnt->mover.speed,
                                                pEnt->mover.midTime,
                                                pEnt->mover.decelTime,
                                                &pEnt->trigger.exposureLerpToLighter,
                                                &pEnt->trigger.exposureFeather[1],
                                                pEnt->mover.pos3);
            BG_EvaluateTrajectory(&pEnt->s.lerp.pos, level.time, pEnt->r.currentOrigin);
            SV_LinkEntity(pEnt);
            if ( bMoveFinished )
                Scr_Notify(pEnt, scr_const.movedone, 0);
        }
    }
    if ( pEnt->s.lerp.apos.trType && pEnt->s.lerp.apos.trDuration + pEnt->s.lerp.apos.trTime <= level.time )
    {
        bMoveFinisheda = ScriptMover_UpdateMove(
                                             &pEnt->s.lerp.apos,
                                             pEnt->r.currentAngles,
                                             pEnt->mover.aSpeed,
                                             pEnt->mover.aMidTime,
                                             pEnt->mover.aDecelTime,
                                             pEnt->mover.apos1,
                                             pEnt->mover.apos2,
                                             pEnt->mover.apos3);
        BG_EvaluateTrajectory(&pEnt->s.lerp.apos, level.time, pEnt->r.currentAngles);
        SV_LinkEntity(pEnt);
        if ( bMoveFinisheda )
        {
            pEnt->r.currentAngles[0] = AngleNormalize180(pEnt->r.currentAngles[0]);
            pEnt->r.currentAngles[1] = AngleNormalize360(pEnt->r.currentAngles[1]);
            pEnt->r.currentAngles[2] = AngleNormalize180(pEnt->r.currentAngles[2]);
            Scr_Notify(pEnt, scr_const.rotatedone, 0);
        }
    }
}

int __cdecl ScriptMover_UpdateMove(
                trajectory_t *pTr,
                float *vCurrPos,
                float fSpeed,
                float fMidTime,
                float fDecelTime,
                const float *vPos1,
                const float *vPos2,
                const float *vPos3)
{
    float fDelta; // [esp+38h] [ebp-14h]
    float vMove[3]; // [esp+3Ch] [ebp-10h] BYREF
    int trDuration; // [esp+48h] [ebp-4h]

    trDuration = (int)(float)(fMidTime * 1000.0);
    if ( pTr->trType == 8 && trDuration > 0 )
    {
        pTr->trTime = level.time;
        pTr->trDuration = trDuration;
        pTr->trBase[0] = *vPos1;
        pTr->trBase[1] = vPos1[1];
        pTr->trBase[2] = vPos1[2];
        vMove[0] = *vPos2 - *vPos1;
        vMove[1] = vPos2[1] - vPos1[1];
        vMove[2] = vPos2[2] - vPos1[2];
        if ( !trDuration
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp", 38, 0, "%s", "trDuration") )
        {
            __debugbreak();
        }
        fDelta = 1000.0 / (float)trDuration;
        pTr->trDelta[0] = fDelta * vMove[0];
        pTr->trDelta[1] = fDelta * vMove[1];
        pTr->trDelta[2] = fDelta * vMove[2];
        if ( ((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                        41,
                        0,
                        "%s",
                        "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])") )
        {
            __debugbreak();
        }
        pTr->trType = 4;
        return 0;
    }
    if ( (pTr->trType == 8 && trDuration <= 0 || pTr->trType == 4) && fDecelTime > 0.0 )
    {
        pTr->trTime = level.time;
        pTr->trDuration = (int)(float)(fDecelTime * 1000.0);
        pTr->trBase[0] = *vPos2;
        pTr->trBase[1] = vPos2[1];
        pTr->trBase[2] = vPos2[2];
        vMove[0] = *vPos3 - *vPos2;
        vMove[1] = vPos3[1] - vPos2[1];
        vMove[2] = vPos3[2] - vPos2[2];
        Vec3Normalize(vMove);
        vMove[0] = fSpeed * vMove[0];
        vMove[1] = fSpeed * vMove[1];
        vMove[2] = fSpeed * vMove[2];
        pTr->trDelta[0] = vMove[0];
        pTr->trDelta[1] = vMove[1];
        pTr->trDelta[2] = vMove[2];
        if ( ((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                        55,
                        0,
                        "%s",
                        "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])") )
        {
            __debugbreak();
        }
        pTr->trType = 9;
        return 0;
    }
    if ( pTr->trType == 6 || pTr->trType == 13 )
    {
        BG_EvaluateTrajectory(pTr, level.time, pTr->trBase);
    }
    else
    {
        pTr->trBase[0] = *vPos3;
        pTr->trBase[1] = vPos3[1];
        pTr->trBase[2] = vPos3[2];
    }
    pTr->trTime = level.time;
    pTr->trType = 0;
    return 1;
}

void __cdecl ScriptMover_SetupPhysicsLaunch(
                trajectory_t *pTr,
                trajectory_t *paTr,
                const float *contact_point,
                const float *initial_force)
{
    float currPos[3]; // [esp+10h] [ebp-18h] BYREF
    float currApos[3]; // [esp+1Ch] [ebp-Ch] BYREF

    if ( pTr->trType == 10 )
        ++pTr->trDuration;
    else
        pTr->trDuration = 0;
    pTr->trTime = level.time;
    BG_EvaluateTrajectory(pTr, level.time, currPos);
    BG_EvaluateTrajectory(paTr, level.time, currApos);
    pTr->trBase[0] = currPos[0];
    pTr->trBase[1] = currPos[1];
    pTr->trBase[2] = currPos[2];
    paTr->trBase[0] = currApos[0];
    paTr->trBase[1] = currApos[1];
    paTr->trBase[2] = currApos[2];
    pTr->trDelta[0] = *contact_point;
    pTr->trDelta[1] = contact_point[1];
    pTr->trDelta[2] = contact_point[2];
    paTr->trDelta[0] = *initial_force;
    paTr->trDelta[1] = initial_force[1];
    paTr->trDelta[2] = initial_force[2];
    pTr->trType = 10;
    paTr->trType = 10;
}

void __cdecl InitScriptMover(gentity_s *pSelf)
{
    pSelf->handler = 8;
    pSelf->s.lerp.eFlags |= 0x10u;
    pSelf->r.svFlags = 0;
    pSelf->s.eType = ET_SCRIPTMOVER;
    pSelf->s.lerp.pos.trBase[0] = pSelf->r.currentOrigin[0];
    pSelf->s.lerp.pos.trBase[1] = pSelf->r.currentOrigin[1];
    pSelf->s.lerp.pos.trBase[2] = pSelf->r.currentOrigin[2];
    pSelf->s.lerp.pos.trType = 0;
    pSelf->s.lerp.apos.trBase[0] = pSelf->r.currentAngles[0];
    pSelf->s.lerp.apos.trBase[1] = pSelf->r.currentAngles[1];
    pSelf->s.lerp.apos.trBase[2] = pSelf->r.currentAngles[2];
    pSelf->s.lerp.apos.trType = 0;
    pSelf->flags |= 0x1000u;
}

void __cdecl SP_script_brushmodel(gentity_s *self, SpawnVar *v)
{
    if ( SV_SetBrushModel(self) )
    {
        InitScriptMover(self);
        SV_LinkEntity(self);
        if ( (self->spawnflags & 1) != 0 )
            self->flags |= (FL_DYNAMICPATH | FL_AUTO_BLOCKPATHS);

        static_assert(0x40000800u == (FL_DYNAMICPATH | FL_AUTO_BLOCKPATHS));
    }
    else
    {
        Com_PrintError(
            1,
            "Killing script_brushmodel at (%f %f %f) because the brush model is invalid.\n",
            self->s.lerp.pos.trBase[0],
            self->s.lerp.pos.trBase[1],
            self->s.lerp.pos.trBase[2]);
        G_FreeEntity(self);
    }
}

void __cdecl SP_script_model(gentity_s *pSelf, SpawnVar *v)
{
    DObj *obj; // [esp+0h] [ebp-4h]
    int savedregs; // [esp+4h] [ebp+0h] BYREF

    G_DObjUpdate(pSelf);
    InitScriptMover(pSelf);
    pSelf->r.svFlags |= 4u;
    pSelf->r.contents = 8320;
    obj = Com_GetServerDObj(pSelf->s.number);
    if ( obj )
    {
        pSelf->r.contents |= DObjGetContents(obj);
        DObjCalcBounds(obj, pSelf->r.mins, pSelf->r.maxs);
    }
    SV_LinkEntity(pSelf);

    iassert(pSelf->handler == ENT_HANDLER_SCRIPT_MOVER);
    pSelf->handler = ENT_HANDLER_SCRIPT_MODEL;
    if ( (pSelf->spawnflags & 1) != 0 )
        pSelf->flags |= (FL_DYNAMICPATH | FL_AUTO_BLOCKPATHS);

    static_assert(0x40000800u == (FL_DYNAMICPATH | FL_AUTO_BLOCKPATHS));
}

void __cdecl SP_script_origin(gentity_s *pSelf, SpawnVar *v)
{
    int savedregs; // [esp+0h] [ebp+0h] BYREF

    InitScriptMover(pSelf);
    pSelf->r.contents = 0;
    SV_LinkEntity(pSelf);
    pSelf->r.svFlags |= 1u;
}

void __cdecl ScriptEntCmdGetCommandTimes(float *pfTotalTime, float *pfAccelTime, float *pfDecelTime)
{
    float fTotalTimeRoundedUp; // [esp+0h] [ebp-8h]
    int iNumParms; // [esp+4h] [ebp-4h]

    *pfTotalTime = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
    if ( *pfTotalTime <= 0.0 )
        Scr_ParamError(1u, "total time must be positive", SCRIPTINSTANCE_SERVER);
    if ( *pfTotalTime < 0.001 )
        *pfTotalTime = 0.001f;
    iNumParms = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    if ( iNumParms <= 2 )
    {
        *pfAccelTime = 0.0f;
        *pfDecelTime = 0.0f;
    }
    else
    {
        *pfAccelTime = Scr_GetFloat(2u, SCRIPTINSTANCE_SERVER);
        if ( *pfAccelTime < 0.0 )
            Scr_ParamError(2u, "accel time must be nonnegative", SCRIPTINSTANCE_SERVER);
        if ( iNumParms <= 3 )
        {
            *pfDecelTime = 0.0f;
        }
        else
        {
            *pfDecelTime = Scr_GetFloat(3u, SCRIPTINSTANCE_SERVER);
            if ( *pfDecelTime < 0.0 )
                Scr_ParamError(3u, "decel time must be nonnegative", SCRIPTINSTANCE_SERVER);
        }
    }
    if ( (float)(*pfAccelTime + *pfDecelTime) > *pfTotalTime )
    {
        fTotalTimeRoundedUp = *pfTotalTime * 1.0000005;
        if ( (float)(*pfAccelTime + *pfDecelTime) <= fTotalTimeRoundedUp )
            *pfTotalTime = fTotalTimeRoundedUp;
        else
            Scr_Error("accel time plus decel time is greater than total time", 0);
    }
}

void __cdecl ScriptEntCmd_MoveTo(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+Ch] [ebp-1Ch]
    float fTotalTime; // [esp+10h] [ebp-18h] BYREF
    float fAccelTime; // [esp+14h] [ebp-14h] BYREF
    float fDecelTime; // [esp+18h] [ebp-10h] BYREF
    float vPos[3]; // [esp+1Ch] [ebp-Ch] BYREF

    if ( entref.classnum )
    {
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x400u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                        612,
                        0,
                        "%s",
                        "entref.entnum < MAX_GENTITIES") )
        {
            __debugbreak();
        }
        pSelf = &g_entities[entref.entnum];
        if ( pSelf->classname != scr_const.script_brushmodel
            && pSelf->classname != scr_const.script_model
            && pSelf->classname != scr_const.script_origin
            && pSelf->classname != scr_const.light )
        {
            v1 = va("entity %i is not a script_brushmodel, script_model, script_origin, or light", entref.entnum);
            Scr_ObjectError(v1, SCRIPTINSTANCE_SERVER);
        }
    }
    Scr_GetVector(0, vPos, SCRIPTINSTANCE_SERVER);
    ScriptEntCmdGetCommandTimes(&fTotalTime, &fAccelTime, &fDecelTime);
    ScriptMover_Move(pSelf, vPos, fTotalTime, fAccelTime, fDecelTime);
}

void __cdecl ScriptMover_Move(gentity_s *pEnt, const float *vPos, float fTotalTime, float fAccelTime, float fDecelTime)
{
    float origin[3]; // [esp+2Ch] [ebp-Ch] BYREF
    int savedregs; // [esp+38h] [ebp+0h] BYREF

    origin[0] = pEnt->r.currentOrigin[0];
    origin[1] = pEnt->r.currentOrigin[1];
    origin[2] = pEnt->r.currentOrigin[2];

    ScriptMover_SetupMove(
        &pEnt->s.lerp.pos,
        vPos,
        fTotalTime,
        fAccelTime,
        fDecelTime,
        origin,
        &pEnt->mover.speed,
        &pEnt->mover.midTime,
        &pEnt->mover.decelTime,
        pEnt->mover.pos1,
        pEnt->mover.pos2,
        pEnt->mover.pos3);

    SV_LinkEntity(pEnt);
}

void __cdecl ScriptMover_SetupMove(
                trajectory_t *pTr,
                const float *vPos,
                float fTotalTime,
                float fAccelTime,
                float fDecelTime,
                float *vCurrPos,
                float *pfSpeed,
                float *pfMidTime,
                float *pfDecelTime,
                float *vPos1,
                float *vPos2,
                float *vPos3)
{
    float v12; // [esp+0h] [ebp-8Ch]
    float v13; // [esp+28h] [ebp-64h]
    float v14; // [esp+44h] [ebp-48h]
    float fDist; // [esp+6Ch] [ebp-20h]
    float fDelta; // [esp+70h] [ebp-1Ch]
    float fDeltaa; // [esp+70h] [ebp-1Ch]
    float vMaxSpeed[3]; // [esp+74h] [ebp-18h] BYREF
    float vMove[3]; // [esp+80h] [ebp-Ch] BYREF

    vMove[0] = *vPos - *vCurrPos;
    vMove[1] = vPos[1] - vCurrPos[1];
    vMove[2] = vPos[2] - vCurrPos[2];
    if ( pTr->trType )
        BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
    if ( fAccelTime == 0.0 && fDecelTime == 0.0 )
    {
        pTr->trTime = level.time;
        pTr->trDuration = (int)(float)(fTotalTime * 1000.0);
        *pfMidTime = fTotalTime;
        *pfDecelTime = 0.0f;
        *vPos3 = *vPos;
        vPos3[1] = vPos[1];
        vPos3[2] = vPos[2];
        pTr->trBase[0] = *vCurrPos;
        pTr->trBase[1] = vCurrPos[1];
        pTr->trBase[2] = vCurrPos[2];
        if ( !pTr->trDuration
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp", 140, 0, "%s", "pTr->trDuration") )
        {
            __debugbreak();
        }
        fDelta = 1000.0 / (float)pTr->trDuration;
        pTr->trDelta[0] = fDelta * vMove[0];
        pTr->trDelta[1] = fDelta * vMove[1];
        pTr->trDelta[2] = fDelta * vMove[2];
        if ( ((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                        143,
                        0,
                        "%s",
                        "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])") )
        {
            __debugbreak();
        }
        pTr->trType = 4;
        BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
    }
    else
    {
        *pfMidTime = (float)(fTotalTime - fAccelTime) - fDecelTime;
        *pfDecelTime = fDecelTime;
        fDist = Vec3Length(vMove);
        if ( (float)((float)((float)(2.0 * fTotalTime) - fAccelTime) - fDecelTime) == 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                        155,
                        0,
                        "%s",
                        "(2.0f * fTotalTime) - fAccelTime - fDecelTime") )
        {
            __debugbreak();
        }
        *pfSpeed = (float)(2.0 * fDist) / (float)((float)((float)(2.0 * fTotalTime) - fAccelTime) - fDecelTime);
        Vec3NormalizeTo(vMove, vMaxSpeed);
        v14 = *pfSpeed;
        vMaxSpeed[0] = *pfSpeed * vMaxSpeed[0];
        vMaxSpeed[1] = v14 * vMaxSpeed[1];
        vMaxSpeed[2] = v14 * vMaxSpeed[2];
        if ( fAccelTime == 0.0 )
        {
            *vPos1 = *vCurrPos;
            vPos1[1] = vCurrPos[1];
            vPos1[2] = vCurrPos[2];
            if ( *pfMidTime == 0.0 )
            {
                pTr->trTime = level.time;
                pTr->trDuration = (int)(float)(*pfDecelTime * 1000.0);
                pTr->trBase[0] = *vCurrPos;
                pTr->trBase[1] = vCurrPos[1];
                pTr->trBase[2] = vCurrPos[2];
                pTr->trDelta[0] = vMaxSpeed[0];
                pTr->trDelta[1] = vMaxSpeed[1];
                pTr->trDelta[2] = vMaxSpeed[2];
                if ( ((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                                198,
                                0,
                                "%s",
                                "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])") )
                {
                    __debugbreak();
                }
                pTr->trType = 9;
            }
            else
            {
                pTr->trTime = level.time;
                pTr->trDuration = (int)(float)(*pfMidTime * 1000.0);
                pTr->trBase[0] = *vCurrPos;
                pTr->trBase[1] = vCurrPos[1];
                pTr->trBase[2] = vCurrPos[2];
                v13 = *pfMidTime;
                vMove[0] = *pfMidTime * vMaxSpeed[0];
                vMove[1] = v13 * vMaxSpeed[1];
                vMove[2] = v13 * vMaxSpeed[2];
                if ( !pTr->trDuration
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                                184,
                                0,
                                "%s",
                                "pTr->trDuration") )
                {
                    __debugbreak();
                }
                fDeltaa = 1000.0 / (float)pTr->trDuration;
                pTr->trDelta[0] = fDeltaa * vMove[0];
                pTr->trDelta[1] = fDeltaa * vMove[1];
                pTr->trDelta[2] = fDeltaa * vMove[2];
                if ( ((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                                187,
                                0,
                                "%s",
                                "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])") )
                {
                    __debugbreak();
                }
                pTr->trType = 4;
            }
        }
        else
        {
            pTr->trTime = level.time;
            pTr->trDuration = (int)(float)(fAccelTime * 1000.0);
            pTr->trBase[0] = *vCurrPos;
            pTr->trBase[1] = vCurrPos[1];
            pTr->trBase[2] = vCurrPos[2];
            pTr->trDelta[0] = vMaxSpeed[0];
            pTr->trDelta[1] = vMaxSpeed[1];
            pTr->trDelta[2] = vMaxSpeed[2];
            if ( ((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                            167,
                            0,
                            "%s",
                            "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])") )
            {
                __debugbreak();
            }
            pTr->trType = 8;
            BG_EvaluateTrajectory(pTr, pTr->trDuration + level.time, vPos1);
        }
        v12 = *pfMidTime;
        *vPos2 = (float)(*pfMidTime * vMaxSpeed[0]) + *vPos1;
        vPos2[1] = (float)(v12 * vMaxSpeed[1]) + vPos1[1];
        vPos2[2] = (float)(v12 * vMaxSpeed[2]) + vPos1[2];
        *vPos3 = *vPos;
        vPos3[1] = vPos[1];
        vPos3[2] = vPos[2];
        BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
    }
}

void __cdecl ScriptEntCmd_GravityMove(scr_entref_t entref)
{
    const char *v1; // eax
    const char *v2; // eax
    float velocity[3]; // [esp+28h] [ebp-14h] BYREF
    gentity_s *pSelf; // [esp+34h] [ebp-8h]
    float fTotalTime; // [esp+38h] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x400u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                        637,
                        0,
                        "%s",
                        "entref.entnum < MAX_GENTITIES") )
        {
            __debugbreak();
        }
        pSelf = &g_entities[entref.entnum];
        if ( pSelf->classname != scr_const.script_brushmodel
            && pSelf->classname != scr_const.script_model
            && pSelf->classname != scr_const.script_origin
            && pSelf->classname != scr_const.light )
        {
            v1 = va("entity %i is not a script_brushmodel, script_model, script_origin, or light", entref.entnum);
            Scr_ObjectError(v1, SCRIPTINSTANCE_SERVER);
        }
    }
    Scr_GetVector(0, velocity, SCRIPTINSTANCE_SERVER);
    if ( (LODWORD(velocity[0]) & 0x7F800000) == 0x7F800000
        || (LODWORD(velocity[1]) & 0x7F800000) == 0x7F800000
        || (LODWORD(velocity[2]) & 0x7F800000) == 0x7F800000 )
    {
        v2 = va("invalid velocity parameter in movegravity command: %f %f %f", velocity[0], velocity[1], velocity[2]);
        Scr_Error(v2, 0);
    }
    fTotalTime = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
    ScriptMover_GravityMove(pSelf, velocity, fTotalTime);
}

void __cdecl ScriptMover_GravityMove(gentity_s *mover, const float *velocity, float totalTime)
{
    trajectory_t *trajectory; // [esp+24h] [ebp-4h]
    int savedregs; // [esp+28h] [ebp+0h] BYREF

    if ( !mover && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp", 356, 0, "%s", "mover") )
        __debugbreak();

    nanassertvec3(velocity);

    trajectory = &mover->s.lerp.pos;
    mover->s.lerp.pos.trTime = level.time;
    mover->s.lerp.pos.trDuration = (int)(float)(totalTime * 1000.0);
    mover->s.lerp.pos.trBase[0] = mover->r.currentOrigin[0];
    mover->s.lerp.pos.trBase[1] = mover->r.currentOrigin[1];
    mover->s.lerp.pos.trBase[2] = mover->r.currentOrigin[2];
    mover->s.lerp.pos.trDelta[0] = *velocity;
    mover->s.lerp.pos.trDelta[1] = velocity[1];
    mover->s.lerp.pos.trDelta[2] = velocity[2];
    if ( ((LODWORD(mover->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(mover->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(mover->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                    367,
                    0,
                    "%s",
                    "!IS_NAN((trajectory->trDelta)[0]) && !IS_NAN((trajectory->trDelta)[1]) && !IS_NAN((trajectory->trDelta)[2])") )
    {
        __debugbreak();
    }
    trajectory->trType = 6;
    BG_EvaluateTrajectory(trajectory, level.time, mover->r.currentOrigin);
    SV_LinkEntity(mover);
}

void __cdecl ScriptEnt_MoveAxis(scr_entref_t entref, int iAxis)
{
    const char *v2; // eax
    gentity_s *pSelf; // [esp+10h] [ebp-20h]
    float fTotalTime; // [esp+14h] [ebp-1Ch] BYREF
    float fAccelTime; // [esp+18h] [ebp-18h] BYREF
    float fDecelTime; // [esp+1Ch] [ebp-14h] BYREF
    float vPos[3]; // [esp+20h] [ebp-10h] BYREF
    float fMove; // [esp+2Ch] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x400u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                        656,
                        0,
                        "%s",
                        "entref.entnum < MAX_GENTITIES") )
        {
            __debugbreak();
        }
        pSelf = &g_entities[entref.entnum];
        if ( pSelf->classname != scr_const.script_brushmodel
            && pSelf->classname != scr_const.script_model
            && pSelf->classname != scr_const.script_origin
            && pSelf->classname != scr_const.light )
        {
            v2 = va("entity %i is not a script_brushmodel, script_model, script_origin, or light", entref.entnum);
            Scr_ObjectError(v2, SCRIPTINSTANCE_SERVER);
        }
    }
    fMove = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    ScriptEntCmdGetCommandTimes(&fTotalTime, &fAccelTime, &fDecelTime);
    vPos[0] = pSelf->r.currentOrigin[0];
    vPos[1] = pSelf->r.currentOrigin[1];
    vPos[2] = pSelf->r.currentOrigin[2];
    vPos[iAxis] = vPos[iAxis] + fMove;
    ScriptMover_Move(pSelf, vPos, fTotalTime, fAccelTime, fDecelTime);
}

void __cdecl ScriptEntCmd_MoveX(scr_entref_t entref)
{
    ScriptEnt_MoveAxis(entref, 0);
}

void __cdecl ScriptEntCmd_MoveY(scr_entref_t entref)
{
    ScriptEnt_MoveAxis(entref, 1);
}

void __cdecl ScriptEntCmd_MoveZ(scr_entref_t entref)
{
    ScriptEnt_MoveAxis(entref, 2);
}

void __cdecl ScriptEntCmd_RotateTo(scr_entref_t entref)
{
    const char *v1; // eax
    float v2; // [esp+Ch] [ebp-3Ch]
    gentity_s *pSelf; // [esp+1Ch] [ebp-2Ch]
    float fTotalTime; // [esp+20h] [ebp-28h] BYREF
    float fAccelTime; // [esp+24h] [ebp-24h] BYREF
    float vDest[3]; // [esp+28h] [ebp-20h] BYREF
    float fDecelTime; // [esp+34h] [ebp-14h] BYREF
    float vRot[3]; // [esp+38h] [ebp-10h] BYREF
    int i; // [esp+44h] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x400u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                        751,
                        0,
                        "%s",
                        "entref.entnum < MAX_GENTITIES") )
        {
            __debugbreak();
        }
        pSelf = &g_entities[entref.entnum];
        if ( pSelf->classname != scr_const.script_brushmodel
            && pSelf->classname != scr_const.script_model
            && pSelf->classname != scr_const.script_origin
            && pSelf->classname != scr_const.light )
        {
            v1 = va("entity %i is not a script_brushmodel, script_model, script_origin, or light", entref.entnum);
            Scr_ObjectError(v1, SCRIPTINSTANCE_SERVER);
        }
    }
    Scr_GetVector(0, vDest, SCRIPTINSTANCE_SERVER);
    ScriptEntCmdGetCommandTimes(&fTotalTime, &fAccelTime, &fDecelTime);
    for ( i = 0; i < 3; ++i )
    {
        v2 = AngleNormalize180(vDest[i] - pSelf->r.currentAngles[i]);
        vRot[i] = pSelf->r.currentAngles[i] + v2;
    }
    ScriptMover_Rotate(pSelf, vRot, fTotalTime, fAccelTime, fDecelTime);
}

void __cdecl ScriptMover_Rotate(
                gentity_s *pEnt,
                const float *vRot,
                float fTotalTime,
                float fAccelTime,
                float fDecelTime)
{
    float angles[3]; // [esp+2Ch] [ebp-Ch] BYREF
    int savedregs; // [esp+38h] [ebp+0h] BYREF

    angles[0] = pEnt->r.currentAngles[0];
    angles[1] = pEnt->r.currentAngles[1];
    angles[2] = pEnt->r.currentAngles[2];
    ScriptMover_SetupMove(
        &pEnt->s.lerp.apos,
        vRot,
        fTotalTime,
        fAccelTime,
        fDecelTime,
        angles,
        &pEnt->mover.aSpeed,
        &pEnt->mover.aMidTime,
        &pEnt->mover.aDecelTime,
        pEnt->mover.apos1,
        pEnt->mover.apos2,
        pEnt->mover.apos3);
    SV_LinkEntity(pEnt);
}

void __cdecl ScriptEntCmd_DevAddPitch(scr_entref_t entref)
{
    ScriptEnt_DevAddRotate(entref, 1u);
}

void __cdecl ScriptEnt_DevAddRotate(scr_entref_t entref, unsigned int iAxis)
{
    const char *v2; // eax
    long double v3; // st7
    int i; // [esp+18h] [ebp-5Ch]
    gentity_s *pSelf; // [esp+1Ch] [ebp-58h]
    float axisOut[3][3]; // [esp+20h] [ebp-54h] BYREF
    float fDelta; // [esp+44h] [ebp-30h]
    float axisIn[3][3]; // [esp+48h] [ebp-2Ch] BYREF
    float fCos; // [esp+6Ch] [ebp-8h]
    float fSin; // [esp+70h] [ebp-4h]
    int savedregs; // [esp+74h] [ebp+0h] BYREF

    if ( iAxis > 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                    773,
                    0,
                    "%s",
                    "iAxis >= 0 && iAxis < 3") )
    {
        __debugbreak();
    }
    if ( entref.classnum )
    {
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x400u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                        775,
                        0,
                        "%s",
                        "entref.entnum < MAX_GENTITIES") )
        {
            __debugbreak();
        }
        pSelf = &g_entities[entref.entnum];
        if ( pSelf->classname != scr_const.script_brushmodel
            && pSelf->classname != scr_const.script_model
            && pSelf->classname != scr_const.script_origin
            && pSelf->classname != scr_const.light )
        {
            v2 = va("entity %i is not a script_brushmodel, script_model, script_origin, or light", entref.entnum);
            Scr_ObjectError(v2, SCRIPTINSTANCE_SERVER);
        }
    }
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) == 1 )
    {
        fDelta = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
        v3 = (float)(DEG2RAD(fDelta));
        fCos = cos(v3);
        fSin = sin(v3);
        AnglesToAxis(pSelf->r.currentAngles, axisIn);
        for ( i = 0; i < 3; ++i )
        {
            axisOut[iAxis][i] = axisIn[iAxis][i];
            axisOut[(int)(iAxis + 1) % 3][i] = (float)(axisIn[(int)(iAxis + 1) % 3][i] * fCos)
                                                                             + (float)(axisIn[(int)(iAxis + 2) % 3][i] * fSin);
            axisOut[(int)(iAxis + 2) % 3][i] = (float)(axisIn[(int)(iAxis + 2) % 3][i] * fCos)
                                                                             - (float)(axisIn[(int)(iAxis + 1) % 3][i] * fSin);
        }
        AxisToAngles(axisOut, pSelf->r.currentAngles);
        pSelf->s.lerp.apos.trType = 0;
        pSelf->s.lerp.apos.trBase[0] = pSelf->r.currentAngles[0];
        pSelf->s.lerp.apos.trBase[1] = pSelf->r.currentAngles[1];
        pSelf->s.lerp.apos.trBase[2] = pSelf->r.currentAngles[2];
        SV_LinkEntity(pSelf);
    }
    else
    {
        Scr_Error("ScriptEnt_ProtoAddRotate: expect exactly one parameter.", 0);
    }
}

void __cdecl ScriptEntCmd_DevAddYaw(scr_entref_t entref)
{
    ScriptEnt_DevAddRotate(entref, 2u);
}

void __cdecl ScriptEntCmd_DevAddRoll(scr_entref_t entref)
{
    ScriptEnt_DevAddRotate(entref, 0);
}

void __cdecl ScriptEnt_RotateAxis(scr_entref_t entref, int iAxis)
{
    const char *v2; // eax
    gentity_s *pSelf; // [esp+10h] [ebp-20h]
    float fTotalTime; // [esp+14h] [ebp-1Ch] BYREF
    float fAccelTime; // [esp+18h] [ebp-18h] BYREF
    float fDecelTime; // [esp+1Ch] [ebp-14h] BYREF
    float vRot[3]; // [esp+20h] [ebp-10h] BYREF
    float fMove; // [esp+2Ch] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x400u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                        872,
                        0,
                        "%s",
                        "entref.entnum < MAX_GENTITIES") )
        {
            __debugbreak();
        }
        pSelf = &g_entities[entref.entnum];
        if ( pSelf->classname != scr_const.script_brushmodel
            && pSelf->classname != scr_const.script_model
            && pSelf->classname != scr_const.script_origin
            && pSelf->classname != scr_const.light )
        {
            v2 = va("entity %i is not a script_brushmodel, script_model, script_origin, or light", entref.entnum);
            Scr_ObjectError(v2, SCRIPTINSTANCE_SERVER);
        }
    }
    fMove = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    ScriptEntCmdGetCommandTimes(&fTotalTime, &fAccelTime, &fDecelTime);
    vRot[0] = pSelf->r.currentAngles[0];
    vRot[1] = pSelf->r.currentAngles[1];
    vRot[2] = pSelf->r.currentAngles[2];
    vRot[iAxis] = vRot[iAxis] + fMove;
    ScriptMover_Rotate(pSelf, vRot, fTotalTime, fAccelTime, fDecelTime);
}

void __cdecl ScriptEntCmd_RotatePitch(scr_entref_t entref)
{
    ScriptEnt_RotateAxis(entref, 0);
}

void __cdecl ScriptEntCmd_RotateYaw(scr_entref_t entref)
{
    ScriptEnt_RotateAxis(entref, 1);
}

void __cdecl ScriptEntCmd_RotateRoll(scr_entref_t entref)
{
    ScriptEnt_RotateAxis(entref, 2);
}

void __cdecl ScriptEntCmd_Vibrate(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+24h] [ebp-5Ch]
    float impulseVector[3]; // [esp+28h] [ebp-58h] BYREF
    float amplitude; // [esp+34h] [ebp-4Ch]
    float scaledImpulseVector[3]; // [esp+38h] [ebp-48h]
    const char *error; // [esp+44h] [ebp-3Ch]
    float time; // [esp+48h] [ebp-38h]
    float vibrationAngles[3]; // [esp+4Ch] [ebp-34h]
    float period; // [esp+58h] [ebp-28h]
    float axis[3][3]; // [esp+5Ch] [ebp-24h] BYREF

    error = "illegal call to vibrate()\n";
    if ( entref.classnum )
    {
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x400u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                        972,
                        0,
                        "%s",
                        "entref.entnum < MAX_GENTITIES") )
        {
            __debugbreak();
        }
        pSelf = &g_entities[entref.entnum];
        if ( pSelf->classname != scr_const.script_brushmodel
            && pSelf->classname != scr_const.script_model
            && pSelf->classname != scr_const.script_origin
            && pSelf->classname != scr_const.light )
        {
            v1 = va("entity %i is not a script_brushmodel, script_model, script_origin, or light", entref.entnum);
            Scr_ObjectError(v1, SCRIPTINSTANCE_SERVER);
        }
    }
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) == 4 )
    {
        Scr_GetVector(0, impulseVector, SCRIPTINSTANCE_SERVER);
        amplitude = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
        period = Scr_GetFloat(2u, SCRIPTINSTANCE_SERVER);
        time = Scr_GetFloat(3u, SCRIPTINSTANCE_SERVER);
        Vec3Normalize(impulseVector);
        scaledImpulseVector[0] = amplitude * impulseVector[0];
        scaledImpulseVector[1] = amplitude * impulseVector[1];
        scaledImpulseVector[2] = amplitude * impulseVector[2];
        AnglesToAxis(pSelf->r.currentAngles, axis);
        vibrationAngles[0] = (float)((float)(axis[0][0] * scaledImpulseVector[0])
                                                             + (float)(axis[0][1] * scaledImpulseVector[1]))
                                             + (float)(axis[0][2] * scaledImpulseVector[2]);
        (vibrationAngles[2]) = -((float)((float)(axis[1][0] * scaledImpulseVector[0])
            + (float)(axis[1][1] * scaledImpulseVector[1]))
            + (float)(axis[1][2] * scaledImpulseVector[2]));// ^_mask__NegFloat_;
        vibrationAngles[1] = 0.0f;
        pSelf->mover.apos3[0] = pSelf->r.currentAngles[0];
        pSelf->mover.apos3[1] = pSelf->r.currentAngles[1];
        pSelf->mover.apos3[2] = pSelf->r.currentAngles[2];
        pSelf->s.lerp.apos.trDuration = (int)(period * 1000.0);
        pSelf->s.lerp.apos.trTime = level.time + (int)(float)(time * 1000.0);
        pSelf->s.lerp.apos.trBase[0] = pSelf->r.currentAngles[0];
        pSelf->s.lerp.apos.trBase[1] = pSelf->r.currentAngles[1];
        pSelf->s.lerp.apos.trBase[2] = pSelf->r.currentAngles[2];
        pSelf->s.lerp.apos.trDelta[0] = vibrationAngles[0];
        pSelf->s.lerp.apos.trDelta[1] = vibrationAngles[1];
        pSelf->s.lerp.apos.trDelta[2] = vibrationAngles[2];
        pSelf->s.lerp.apos.trType = 5;
    }
    else
    {
        Scr_Error(error, 0);
    }
}

void __cdecl ScriptEntCmd_RotateVelocity(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+Ch] [ebp-1Ch]
    float fTotalTime; // [esp+10h] [ebp-18h] BYREF
    float fAccelTime; // [esp+14h] [ebp-14h] BYREF
    float fDecelTime; // [esp+18h] [ebp-10h] BYREF
    float vSpeed[3]; // [esp+1Ch] [ebp-Ch] BYREF

    if ( entref.classnum )
    {
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x400u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                        1027,
                        0,
                        "%s",
                        "entref.entnum < MAX_GENTITIES") )
        {
            __debugbreak();
        }
        pSelf = &g_entities[entref.entnum];
        if ( pSelf->classname != scr_const.script_brushmodel
            && pSelf->classname != scr_const.script_model
            && pSelf->classname != scr_const.script_origin
            && pSelf->classname != scr_const.light )
        {
            v1 = va("entity %i is not a script_brushmodel, script_model, script_origin, or light", entref.entnum);
            Scr_ObjectError(v1, SCRIPTINSTANCE_SERVER);
        }
    }
    Scr_GetVector(0, vSpeed, SCRIPTINSTANCE_SERVER);
    ScriptEntCmdGetCommandTimes(&fTotalTime, &fAccelTime, &fDecelTime);
    ScriptMover_RotateSpeed(pSelf, vSpeed, fTotalTime, fAccelTime, fDecelTime);
}

void __cdecl ScriptMover_RotateSpeed(
                gentity_s *pEnt,
                const float *vRotSpeed,
                float fTotalTime,
                float fAccelTime,
                float fDecelTime)
{
    int savedregs; // [esp+28h] [ebp+0h] BYREF

    ScriptMover_SetupMoveSpeed(
        &pEnt->s.lerp.apos,
        vRotSpeed,
        fTotalTime,
        fAccelTime,
        fDecelTime,
        pEnt->r.currentAngles,
        &pEnt->mover.aSpeed,
        &pEnt->mover.aMidTime,
        &pEnt->mover.aDecelTime,
        pEnt->mover.apos1,
        pEnt->mover.apos2,
        pEnt->mover.apos3);
    SV_LinkEntity(pEnt);
}

void __cdecl ScriptMover_SetupMoveSpeed(
                trajectory_t *pTr,
                const float *vSpeed,
                float fTotalTime,
                float fAccelTime,
                float fDecelTime,
                float *vCurrPos,
                float *pfSpeed,
                float *pfMidTime,
                float *pfDecelTime,
                float *vPos1,
                float *vPos2,
                float *vPos3)
{
    float v12; // [esp+Ch] [ebp-7Ch]
    trajectory_t tr; // [esp+64h] [ebp-24h] BYREF

    if ( pTr->trType )
        BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
    if ( fAccelTime == 0.0 && fDecelTime == 0.0 )
    {
        pTr->trTime = level.time;
        pTr->trDuration = (int)(float)(fTotalTime * 1000.0);
        *pfMidTime = fTotalTime;
        *pfDecelTime = 0.0f;
        pTr->trBase[0] = *vCurrPos;
        pTr->trBase[1] = vCurrPos[1];
        pTr->trBase[2] = vCurrPos[2];
        pTr->trDelta[0] = *vSpeed;
        pTr->trDelta[1] = vSpeed[1];
        pTr->trDelta[2] = vSpeed[2];
        if ( ((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                        260,
                        0,
                        "%s",
                        "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])") )
        {
            __debugbreak();
        }
        pTr->trType = 4;
        BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
        BG_EvaluateTrajectory(pTr, pTr->trDuration + level.time, vPos3);
    }
    else
    {
        *pfMidTime = (float)(fTotalTime - fAccelTime) - fDecelTime;
        *pfDecelTime = fDecelTime;
        *pfSpeed = Vec3Length(vSpeed);
        if ( fAccelTime == 0.0 )
        {
            *vPos1 = *vCurrPos;
            vPos1[1] = vCurrPos[1];
            vPos1[2] = vCurrPos[2];
            if ( *pfMidTime == 0.0 )
            {
                pTr->trTime = level.time;
                pTr->trDuration = (int)(float)(*pfDecelTime * 1000.0);
                pTr->trBase[0] = *vCurrPos;
                pTr->trBase[1] = vCurrPos[1];
                pTr->trBase[2] = vCurrPos[2];
                pTr->trDelta[0] = *vSpeed;
                pTr->trDelta[1] = vSpeed[1];
                pTr->trDelta[2] = vSpeed[2];
                if ( ((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                                309,
                                0,
                                "%s",
                                "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])") )
                {
                    __debugbreak();
                }
                pTr->trType = 9;
            }
            else
            {
                pTr->trTime = level.time;
                pTr->trDuration = (int)(float)(*pfMidTime * 1000.0);
                pTr->trBase[0] = *vCurrPos;
                pTr->trBase[1] = vCurrPos[1];
                pTr->trBase[2] = vCurrPos[2];
                pTr->trDelta[0] = *vSpeed;
                pTr->trDelta[1] = vSpeed[1];
                pTr->trDelta[2] = vSpeed[2];
                if ( ((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                                299,
                                0,
                                "%s",
                                "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])") )
                {
                    __debugbreak();
                }
                pTr->trType = 4;
            }
        }
        else
        {
            pTr->trTime = level.time;
            pTr->trDuration = (int)(float)(fAccelTime * 1000.0);
            pTr->trBase[0] = *vCurrPos;
            pTr->trBase[1] = vCurrPos[1];
            pTr->trBase[2] = vCurrPos[2];
            pTr->trDelta[0] = *vSpeed;
            pTr->trDelta[1] = vSpeed[1];
            pTr->trDelta[2] = vSpeed[2];
            if ( ((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                            282,
                            0,
                            "%s",
                            "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])") )
            {
                __debugbreak();
            }
            pTr->trType = 8;
            BG_EvaluateTrajectory(pTr, pTr->trDuration + level.time, vPos1);
        }
        v12 = *pfMidTime;
        *vPos2 = (float)(*pfMidTime * *vSpeed) + *vPos1;
        vPos2[1] = (float)(v12 * vSpeed[1]) + vPos1[1];
        vPos2[2] = (float)(v12 * vSpeed[2]) + vPos1[2];
        if ( *pfDecelTime == 0.0 )
        {
            *vPos3 = *vPos2;
            vPos3[1] = vPos2[1];
            vPos3[2] = vPos2[2];
        }
        else
        {
            tr.trType = 9;
            tr.trTime = level.time;
            tr.trDuration = (int)(float)(*pfDecelTime * 1000.0);
            tr.trBase[0] = *vPos2;
            tr.trBase[1] = vPos2[1];
            tr.trBase[2] = vPos2[2];
            tr.trDelta[0] = *vSpeed;
            tr.trDelta[1] = vSpeed[1];
            tr.trDelta[2] = vSpeed[2];
            if ( ((LODWORD(tr.trDelta[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(tr.trDelta[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(tr.trDelta[2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                            327,
                            0,
                            "%s",
                            "!IS_NAN((tr.trDelta)[0]) && !IS_NAN((tr.trDelta)[1]) && !IS_NAN((tr.trDelta)[2])") )
            {
                __debugbreak();
            }
            BG_EvaluateTrajectory(&tr, tr.trDuration + level.time, vPos3);
        }
        BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
    }
}

void __cdecl ScriptEntCmd_SetCanDamage(scr_entref_t entref)
{
    gentity_s *pSelf; // [esp+0h] [ebp-Ch]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) == 1 )
    {
        if ( entref.classnum )
        {
            Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
            pSelf = 0;
        }
        else
        {
            if ( entref.entnum >= 0x400u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                            1061,
                            0,
                            "%s",
                            "entref.entnum < MAX_GENTITIES") )
            {
                __debugbreak();
            }
            pSelf = &g_entities[entref.entnum];
        }
        pSelf->takedamage = (unsigned __int8)Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Scr_Error("illegal call to setcandamage()\n", 0);
    }
}

void __cdecl ScriptEntCmd_PhysicsLaunch(scr_entref_t entref)
{
    const char *v1; // eax
    char *v2; // eax
    double v3; // [esp+0h] [ebp-3Ch]
    double v4; // [esp+8h] [ebp-34h]
    double v5; // [esp+10h] [ebp-2Ch]
    gentity_s *pSelf; // [esp+20h] [ebp-1Ch]
    float contact_point[3]; // [esp+24h] [ebp-18h] BYREF
    float initial_force[3]; // [esp+30h] [ebp-Ch] BYREF

    if ( entref.classnum )
    {
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x400u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                        1094,
                        0,
                        "%s",
                        "entref.entnum < MAX_GENTITIES") )
        {
            __debugbreak();
        }
        pSelf = &g_entities[entref.entnum];
        if ( pSelf->classname != scr_const.script_brushmodel
            && pSelf->classname != scr_const.script_model
            && pSelf->classname != scr_const.script_origin
            && pSelf->classname != scr_const.light )
        {
            v1 = va("entity %i is not a script_brushmodel, script_model, script_origin, or light", entref.entnum);
            Scr_ObjectError(v1, SCRIPTINSTANCE_SERVER);
        }
    }
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) == 2 )
    {
        Scr_GetVector(0, contact_point, SCRIPTINSTANCE_SERVER);
        Scr_GetVector(1u, initial_force, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        memset(contact_point, 0, sizeof(contact_point));
        memset(initial_force, 0, sizeof(initial_force));
    }
    if ( pSelf->classname != scr_const.script_brushmodel
        || cm.leafbrushNodes[CM_ClipHandleToModel(pSelf->s.index.brushmodel)->leaf.leafBrushNode].leafBrushCount )
    {
        ScriptMover_SetupPhysicsLaunch(&pSelf->s.lerp.pos, &pSelf->s.lerp.apos, contact_point, initial_force);
    }
    else
    {
        v5 = pSelf->r.currentOrigin[2];
        v4 = pSelf->r.currentOrigin[1];
        v3 = pSelf->r.currentOrigin[0];
        v2 = SL_ConvertToString(pSelf->targetname, SCRIPTINSTANCE_SERVER);
        Com_PrintWarning(20, "WARNING: Brushmodel without brushes '%s' (%.1f %.1f %.1f)\n", v2, v3, v4, v5);
    }
}

gentity_s *__cdecl GetEntity_0(scr_entref_t entref)
{
    if (entref.classnum)
    {
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
        return 0;
    }
    else
    {
        if (entref.entnum >= 0x400u
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_mover.cpp",
                532,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES"))
        {
            __debugbreak();
        }
        return &g_entities[entref.entnum];
    }
}

void __cdecl ScriptEntCmd_Solid(scr_entref_t entref)
{
    gentity_s *pSelf; // [esp+8h] [ebp-4h]
    int savedregs; // [esp+Ch] [ebp+0h] BYREF

    pSelf = GetEntity_0(entref);
    if ( pSelf->classname == scr_const.script_origin )
    {
        Com_DPrintf(24, "cannot use the solid/notsolid commands on a script_origin entity( number %i )\n", pSelf->s.number);
    }
    else
    {
        if ( pSelf->classname == scr_const.script_model || pSelf->classname == scr_const.rocket )
            pSelf->r.contents = 8320;
        else
            pSelf->r.contents = 1;
        pSelf->s.lerp.eFlags &= ~1u;
        SV_LinkEntity(pSelf);
    }
}

void __cdecl ScriptEntCmd_NotSolid(scr_entref_t entref)
{
    gentity_s *pSelf; // [esp+8h] [ebp-4h]
    int savedregs; // [esp+Ch] [ebp+0h] BYREF

    pSelf = GetEntity_0(entref);
    if ( pSelf->classname == scr_const.script_origin )
    {
        Com_DPrintf(24, "cannot use the solid/notsolid commands on a script_origin entity( number %i )\n", pSelf->s.number);
    }
    else
    {
        pSelf->r.contents = 0;
        pSelf->s.lerp.eFlags |= 1u;
        SV_LinkEntity(pSelf);
    }
}

// Looks congruent to retail blops MP latest
const BuiltinMethodDef methods_1[18] =
{
  { "moveto", &ScriptEntCmd_MoveTo, 0 },
  { "movex", &ScriptEntCmd_MoveX, 0 },
  { "movey", &ScriptEntCmd_MoveY, 0 },
  { "movez", &ScriptEntCmd_MoveZ, 0 },
  { "movegravity", &ScriptEntCmd_GravityMove, 0 },
  { "rotateto", &ScriptEntCmd_RotateTo, 0 },
  { "rotatepitch", &ScriptEntCmd_RotatePitch, 0 },
  { "rotateyaw", &ScriptEntCmd_RotateYaw, 0 },
  { "rotateroll", &ScriptEntCmd_RotateRoll, 0 },
  { "devaddpitch", &ScriptEntCmd_DevAddPitch, 1 },
  { "devaddyaw", &ScriptEntCmd_DevAddYaw, 1 },
  { "devaddroll", &ScriptEntCmd_DevAddRoll, 1 },
  { "vibrate", &ScriptEntCmd_Vibrate, 0 },
  { "rotatevelocity", &ScriptEntCmd_RotateVelocity, 0 },
  { "solid", &ScriptEntCmd_Solid, 0 },
  { "notsolid", &ScriptEntCmd_NotSolid, 0 },
  { "setcandamage", &ScriptEntCmd_SetCanDamage, 0 },
  { "physicslaunch", &ScriptEntCmd_PhysicsLaunch, 0 }
};

void (__cdecl *__cdecl ScriptEnt_GetMethod(const char **pName))(scr_entref_t)
{
    unsigned int i; // [esp+18h] [ebp-4h]

    for ( i = 0; i < ARRAY_COUNT(methods_1); ++i )
    {
        if ( !strcmp(*pName, methods_1[i].actionString) )
        {
            *pName = methods_1[i].actionString;
            return methods_1[i].actionFunc;
        }
    }
    return 0;
}

