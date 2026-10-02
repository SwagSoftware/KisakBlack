#include "actor_orientation.h"
#include <universal/com_math_anglevectors.h>
#include <game_mp/g_main_mp.h>
#include <clientscript/scr_const.h>
#include <game_mp/g_spawn_mp.h>
#include "bullet.h"
#include "actor_navigation.h"
#include "actor_exposed.h"
#include <game_mp/actor_mp.h>
#include "actor_senses.h"

const float ACTOR_MAX_LOOK_PITCH_RATE = 0.8f;
const float ACTOR_MAX_LOOK_YAW_RATE = 0.8f;
const float ACTOR_MAX_BODY_YAW_RATE = 300.0f;
const float ACTOR_MAX_PRONE_YAW_RATE = 360.0f;
const float PATH_MAX_ENEMY_FACE_DISTANCE_SQRD = 122500.0f;
const float ACTOR_FORCE_FACE_ENEMY_TIME = 300.0f;


void __fastcall Actor_SetDesiredLookAngles(ai_orient_t *pOrient, float fPitch, float fYaw)
{
    if ( !pOrient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_orientation.cpp", 32, 0, "%s", "pOrient") )
    {
        __debugbreak();
    }
    pOrient->fDesiredLookPitch = AngleNormalize360(fPitch);
    pOrient->fDesiredLookYaw = AngleNormalize360(fYaw);
}

void __fastcall Actor_SetDesiredBodyAngle(ai_orient_t *pOrient, float fAngle)
{
    if ( !pOrient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_orientation.cpp", 49, 0, "%s", "pOrient") )
    {
        __debugbreak();
    }
    pOrient->fDesiredBodyYaw = AngleNormalize360(fAngle);
}

void __fastcall Actor_SetDesiredAngles(ai_orient_t *pOrient, float fPitch, float fYaw)
{
    Actor_SetDesiredLookAngles(pOrient, fPitch, fYaw);
    Actor_SetDesiredBodyAngle(pOrient, fYaw);
}

void __fastcall Actor_SetLookAngles(actor_s *self, float fPitch, float fYaw)
{
    float vLookAngles[3]; // [esp+8h] [ebp-Ch] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_orientation.cpp", 78, 0, "%s", "self") )
        __debugbreak();
    self->fLookPitch = AngleNormalize360(fPitch);
    vLookAngles[0] = self->fLookPitch;
    self->fLookYaw = AngleNormalize360(fYaw);
    vLookAngles[1] = self->fLookYaw;
    vLookAngles[2] = 0.0f;
    AngleVectors(vLookAngles, self->vLookForward, self->vLookRight, self->vLookUp);
}

void __fastcall Actor_SetBodyAngle(actor_s *self, float fAngle)
{
    gentity_s *ent; // [esp+Ch] [ebp-4h]

    if ( !self
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_orientation.cpp", 105, 0, "%s", "self") )
    {
        __debugbreak();
    }
    ent = self->ent;
    if ( !self->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_orientation.cpp", 108, 0, "%s", "ent") )
    {
        __debugbreak();
    }
    ent->r.currentAngles[0] = 0.0f;
    ent->r.currentAngles[1] = AngleNormalize360(fAngle);
    ent->r.currentAngles[2] = 0.0f;
    if ( ai_debugAnimDeltas->current.integer == ent->s.number )
        Com_Printf(18, "%i currentYaw = %g\n", level.time, ent->r.currentAngles[1]);
}

void __fastcall Actor_ChangeAngles(actor_s *self, float fPitch, float fYaw)
{
    Actor_SetDesiredAngles(
        &self->CodeOrient,
        self->CodeOrient.fDesiredLookPitch + fPitch,
        self->CodeOrient.fDesiredBodyYaw + fYaw);
    Actor_SetDesiredAngles(
        &self->ScriptOrient,
        self->ScriptOrient.fDesiredLookPitch + fPitch,
        self->ScriptOrient.fDesiredBodyYaw + fYaw);
    Actor_SetBodyAngle(self, self->ent->r.currentAngles[1] + fYaw);
    Actor_SetLookAngles(self, self->fLookPitch + fPitch, self->fLookYaw + fYaw);
}

void __fastcall Actor_UpdateLookAngles(actor_s *self)
{
    float delta; // [esp+24h] [ebp-10h]
    float deltaa; // [esp+24h] [ebp-10h]
    float fPitch; // [esp+28h] [ebp-Ch]
    ai_orient_t *pOrient; // [esp+30h] [ebp-4h]

    if ( self->ScriptOrient.eMode )
        pOrient = &self->ScriptOrient;
    else
        pOrient = &self->CodeOrient;
    delta = AngleNormalize180(pOrient->fDesiredLookPitch - self->fLookPitch);
    if ( delta <= (float)(0.80000001 * 50.0) )
    {
        if ( (float)((-(ACTOR_MAX_LOOK_PITCH_RATE)) * 50.0) > delta )
            delta =  (-(ACTOR_MAX_LOOK_PITCH_RATE)) * 50.0;
    }
    else
    {
        delta = 0.80000001 * 50.0;
    }
    fPitch = self->fLookPitch + delta;
    deltaa = AngleNormalize180(pOrient->fDesiredLookYaw - self->fLookYaw);
    if ( deltaa <= (float)(0.80000001 * 50.0) )
    {
        if ( (float)((-(ACTOR_MAX_LOOK_YAW_RATE)) * 50.0) > deltaa )
            deltaa = (-(ACTOR_MAX_LOOK_YAW_RATE)) * 50.0;
    }
    else
    {
        deltaa = 0.80000001 * 50.0;
    }
    Actor_SetLookAngles(self, fPitch, self->fLookYaw + deltaa);
}

void __cdecl CalcYaw(float dt, float *yaw, float *yawVeloc, float yawAccel)
{
    float yawVelocNew; // [esp+0h] [ebp-4h]

    yawVelocNew = (float)(yawAccel * dt) + *yawVeloc;
    *yaw = (float)((float)((float)(*yawVeloc + yawVelocNew) * dt) / 2.0) + *yaw;
    *yawVeloc = yawVelocNew;
}

void __cdecl StepYaw(float dt, float *yaw, float *yawVeloc, float targetYaw, float targetVeloc)
{
    float v5; // [esp+10h] [ebp-30h]
    float v6; // [esp+14h] [ebp-2Ch]
    float value; // [esp+18h] [ebp-28h]
    float angleDelta; // [esp+28h] [ebp-18h]
    float maxVeloc; // [esp+30h] [ebp-10h]
    float decel; // [esp+34h] [ebp-Ch]

    angleDelta = AngleDelta(targetYaw, *yaw);
    if ( angleDelta >= 0.0 )
        value = ai_angularYawAccelRate->current.value;
    else
        (value) = -ai_angularYawAccelRate->current.value;
    decel = (-(value)) * ai_angularYawDecelFactor->current.value;
    maxVeloc = sqrtf(
                             (float)((float)((float)((float)((float)(-2.0 * angleDelta) * value) * decel)
                                                         + (float)((float)(targetVeloc * targetVeloc) * value))
                                         - (float)((float)(*yawVeloc * *yawVeloc) * decel))
                         / (float)(value - decel));
    if (angleDelta < 0.0)
        (maxVeloc) = -maxVeloc; // ^= _mask__NegFloat_;
    if ( (float)((float)((float)(maxVeloc - *yawVeloc) / value) - dt) < 0.0 )
        v6 = (float)(maxVeloc - *yawVeloc) / value;
    else
        v6 = dt;
    if ( v6 > 0.0 )
    {
        CalcYaw(v6, yaw, yawVeloc, value);
        dt = dt - v6;
    }
    if ( (float)((float)((float)(targetVeloc - maxVeloc) / decel) - dt) < 0.0 )
        v5 = (float)(targetVeloc - maxVeloc) / decel;
    else
        v5 = dt;
    if ( v5 > 0.0 )
        CalcYaw(v5, yaw, yawVeloc, decel);
}

void __fastcall Actor_UpdateBodyAngle(actor_s *self)
{
    float v1; // [esp+14h] [ebp-44h]
    float *currentOrigin; // [esp+1Ch] [ebp-3Ch]
    float fDesiredBodyYaw; // [esp+20h] [ebp-38h]
    float maxTurnRate; // [esp+30h] [ebp-28h]
    float desiredDir[3]; // [esp+34h] [ebp-24h] BYREF
    float angles[3]; // [esp+40h] [ebp-18h] BYREF
    float delta; // [esp+4Ch] [ebp-Ch]
    float fYaw; // [esp+50h] [ebp-8h] BYREF
    float fYawVeloc; // [esp+54h] [ebp-4h] BYREF

    if ( self->ScriptOrient.eMode )
        self->fDesiredBodyYaw = self->ScriptOrient.fDesiredBodyYaw;
    else
        self->fDesiredBodyYaw = self->CodeOrient.fDesiredBodyYaw;
    delta = AngleNormalize180(self->fDesiredBodyYaw - self->ent->r.currentAngles[1]);
    if ( delta == 0.0 && self->bNotifyTurnDone )
    {
        Scr_Notify(self->ent, scr_const.orientdone, 0);
        self->bNotifyTurnDone = 0;
    }
    if ( ai_showPaths->current.integer > 1 )
    {
        fDesiredBodyYaw = self->fDesiredBodyYaw;
        angles[0] = 0.0f;
        angles[1] = fDesiredBodyYaw;
        angles[2] = 0.0f;
        AngleVectors(angles, desiredDir, 0, 0);
        currentOrigin = self->ent->r.currentOrigin;
        desiredDir[0] = (float)(10.0 * desiredDir[0]) + *currentOrigin;
        desiredDir[1] = (float)(10.0 * desiredDir[1]) + currentOrigin[1];
        desiredDir[2] = (float)(10.0 * desiredDir[2]) + currentOrigin[2];
    }
    fYawVeloc = 0.0f;
    fYaw = self->ent->r.currentAngles[1];
    if ( ai_angularYawEnabled->current.enabled )
    {
        fYaw = self->ent->r.currentAngles[1];
        fYawVeloc = self->yawVeloc;
        StepYaw(0.05f, &fYaw, &fYawVeloc, self->fDesiredBodyYaw, 0.0);
    }
    else
    {
        if ( BG_ActorIsProne(&self->ProneInfo, level.time) )
            v1 = ACTOR_MAX_PRONE_YAW_RATE;
        else
            v1 = ACTOR_MAX_BODY_YAW_RATE;
        maxTurnRate = v1 / 20.0;
        if ( delta <= (float)(v1 / 20.0) )
        {
            if ( (-(maxTurnRate)) > delta )
                delta = -maxTurnRate;
        }
        else
        {
            delta = v1 / 20.0;
        }
        fYaw = self->ent->r.currentAngles[1] + delta;
    }
    Actor_SetBodyAngle(self, fYaw);
    self->yawVeloc = fYawVeloc;
}

void __fastcall Actor_FaceVector(ai_orient_t *pOrient, const float *v)
{
    float vAngle[3]; // [esp+10h] [ebp-Ch] BYREF

    if ( *v == 0.0
        && v[1] == 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_orientation.cpp", 305, 0, "%s", "v[0] || v[1]") )
    {
        __debugbreak();
    }
    vectoangles(v, vAngle);
    Actor_SetDesiredAngles(pOrient, vAngle[0], vAngle[1]);
}

void __fastcall Actor_SetAnglesToLikelyEnemyPath(actor_s *self)
{
    float *currentAngles; // [esp+8h] [ebp-18h]
    float *currentOrigin; // [esp+Ch] [ebp-14h]
    const float *vOrigin; // [esp+10h] [ebp-10h]
    float vDelta[3]; // [esp+14h] [ebp-Ch] BYREF

    if ( !self
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_orientation.cpp", 374, 0, "%s", "self") )
    {
        __debugbreak();
    }
    if ( !self->faceLikelyEnemyPathNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_orientation.cpp",
                    375,
                    0,
                    "%s",
                    "self->faceLikelyEnemyPathNode") )
    {
        __debugbreak();
    }
    currentOrigin = self->ent->r.currentOrigin;
    vOrigin = self->faceLikelyEnemyPathNode->constant.vOrigin;
    vDelta[0] = *vOrigin - *currentOrigin;
    vDelta[1] = vOrigin[1] - currentOrigin[1];
    vDelta[2] = vOrigin[2] - currentOrigin[2];
    if ( (float)((float)(vDelta[0] * vDelta[0]) + (float)(vDelta[1] * vDelta[1])) < 1.0 )
    {
        currentAngles = self->ent->r.currentAngles;
        self->anglesToLikelyEnemyPath[0] = *currentAngles;
        self->anglesToLikelyEnemyPath[1] = currentAngles[1];
        self->anglesToLikelyEnemyPath[2] = currentAngles[2];
    }
    else
    {
        vectoangles(vDelta, self->anglesToLikelyEnemyPath);
    }
}

char __fastcall Actor_GetAnglesToLikelyEnemyPath(actor_s *self)
{
    const pathnode_t *v2; // eax
    int i; // [esp+20h] [ebp-3Ch]
    const pathnode_t *node; // [esp+24h] [ebp-38h]
    sentient_s *enemy; // [esp+2Ch] [ebp-30h]
    float fDistSqrd; // [esp+30h] [ebp-2Ch]
    float fMinDistSqrd; // [esp+34h] [ebp-28h]
    sentient_s *pFacingEnemy; // [esp+38h] [ebp-24h]
    sentient_info_t *pInfo; // [esp+40h] [ebp-1Ch]
    char eTeam; // [esp+44h] [ebp-18h]
    float vEnemyPos[3]; // [esp+4Ch] [ebp-10h] BYREF
    int CHECK_INTERVAL; // [esp+58h] [ebp-4h]

    CHECK_INTERVAL = 500;
    if ( self->sentient->bIgnoreAll )
        return 0;
    eTeam = Sentient_EnemyTeam(self->sentient->eTeam);
    if ( self->faceLikelyEnemyPathNode )
    {
        if ( self->faceLikelyEnemyPathNeedCheckTime > level.time )
            return 1;
        if ( self->faceLikelyEnemyPathNeedRecalculateTime > level.time )
        {
            v2 = Sentient_NearestNode(self->sentient);
            if ( Path_NodesVisible(v2, self->faceLikelyEnemyPathNode) )
            {
                Actor_SetAnglesToLikelyEnemyPath(self);
                self->faceLikelyEnemyPathNeedCheckTime = level.time + 500;
                return 1;
            }
        }
    }
    else if ( self->faceLikelyEnemyPathNeedRecalculateTime > level.time )
    {
        return 0;
    }
    self->faceLikelyEnemyPathNode = 0;
    pFacingEnemy = 0;
    fMinDistSqrd = FLT_MAX;
    enemy = level.sentients;
    for ( i = 0; i < 48; ++i )
    {
        if ( enemy->inuse
            && ((1 << eTeam) & (1 << enemy->eTeam)) != 0
            && !Actor_CheckIgnore(self->sentient, enemy)
            //&& (!EntHandle::isDefined(&self->sentient->scriptOwner)
            && (!self->sentient->scriptOwner.isDefined()
             //|| enemy->ent != EntHandle::ent(&self->sentient->scriptOwner)) )
             || enemy->ent != self->sentient->scriptOwner.ent()) )
        {
            Sentient_GetOrigin(enemy, vEnemyPos);
            fDistSqrd = Vec3DistanceSq(vEnemyPos, self->ent->r.currentOrigin);
            if ( enemy->ent->client )
                fDistSqrd = fDistSqrd * 0.25;
            if ( fMinDistSqrd > fDistSqrd )
            {
                fMinDistSqrd = fDistSqrd;
                pFacingEnemy = enemy;
            }
        }
        ++enemy;
    }
    if ( pFacingEnemy )
    {
        if ( fMinDistSqrd <= (float)(pFacingEnemy->maxVisibleDist * pFacingEnemy->maxVisibleDist) )
            pInfo = 0;
        else
            pInfo = &self->sentientInfo[pFacingEnemy - level.sentients];
        node = Path_FindFacingNode(self->sentient, pFacingEnemy, pInfo);
        if ( node )
        {
            self->faceLikelyEnemyPathNode = node;
            Actor_SetAnglesToLikelyEnemyPath(self);
            self->faceLikelyEnemyPathNeedRecalculateTime = level.time + 3000;
            self->faceLikelyEnemyPathNeedCheckTime = level.time + 500;
            return 1;
        }
        else
        {
            self->faceLikelyEnemyPathNeedRecalculateTime = level.time + 3000;
            self->faceLikelyEnemyPathNeedCheckTime = self->faceLikelyEnemyPathNeedRecalculateTime;
            return 0;
        }
    }
    else
    {
        self->faceLikelyEnemyPathNeedRecalculateTime = level.time + 500;
        self->faceLikelyEnemyPathNeedCheckTime = self->faceLikelyEnemyPathNeedRecalculateTime;
        return 0;
    }
}

void __fastcall Actor_FaceLikelyEnemyPath(actor_s *self, ai_orient_t *pOrient)
{
    pathnode_t *node; // [esp+10h] [ebp-4h]

    if ( self->faceLikelyEnemyPathNeedCheckTime <= level.time )
    {
        if ( Actor_GetAnglesToLikelyEnemyPath(self) )
        {
            Actor_SetDesiredAngles(pOrient, self->anglesToLikelyEnemyPath[0], self->anglesToLikelyEnemyPath[1]);
        }
        else
        {
            node = self->sentient->pClaimedNode;
            if ( node && Path_IsValidClaimNode(node) && Actor_PointNearNode(self->ent->r.currentOrigin, node) )
                Actor_SetDesiredAngles(pOrient, 0.0, node->constant.fAngle);
        }
    }
}

void __fastcall Actor_FaceEnemy(actor_s *self, ai_orient_t *pOrient)
{
    float *currentOrigin; // eax
    float *v5; // [esp+14h] [ebp-20h]
    sentient_s *enemy; // [esp+20h] [ebp-14h]
    float v[3]; // [esp+28h] [ebp-Ch] BYREF

    iassert(self);
    iassert(self->sentient);

    if ( Actor_GetTargetEntity(self) )
    {
        enemy = Actor_GetTargetSentient(self);
        if ( !enemy || self->sentientInfo[enemy - level.sentients].VisCache.bVisible )
        {
            Actor_GetTargetPosition(self, v);
            currentOrigin = self->ent->r.currentOrigin;
            v[0] = v[0] - *currentOrigin;
            v[1] = v[1] - currentOrigin[1];
            v[2] = v[2] - currentOrigin[2];
            if ( (float)((float)(v[0] * v[0]) + (float)(v[1] * v[1])) >= 1.0 )
                Actor_FaceVector(pOrient, v);
        }
        else
        {
            v5 = self->ent->r.currentOrigin;
            v[0] = self->sentientInfo[enemy - level.sentients].vLastKnownPos[0] - *v5;
            v[1] = self->sentientInfo[enemy - level.sentients].vLastKnownPos[1] - v5[1];
            v[2] = self->sentientInfo[enemy - level.sentients].vLastKnownPos[2] - v5[2];
            if ( (float)((float)(v[0] * v[0]) + (float)(v[1] * v[1])) >= 1.0 )
                Actor_FaceVector(pOrient, v);
        }
    }
    else
    {
        Actor_FaceLikelyEnemyPath(self, pOrient);
    }
}

void __fastcall Actor_DecideOrientation(actor_s *self)
{
    const char *v1; // eax
    pathnode_t *node; // [esp+10h] [ebp-8h]
    ai_orient_t *pOrient; // [esp+14h] [ebp-4h]

    if ( !self
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_orientation.cpp", 771, 0, "%s", "self") )
    {
        __debugbreak();
    }
    if ( !self->sentient
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_orientation.cpp",
                    772,
                    0,
                    "%s",
                    "self->sentient") )
    {
        __debugbreak();
    }
    self->isFacingMotion = 0;
    if ( self->ScriptOrient.eMode )
        pOrient = &self->ScriptOrient;
    else
        pOrient = &self->CodeOrient;
    switch ( pOrient->eMode )
    {
        case AI_ORIENT_DONT_CHANGE:
            return;
        case AI_ORIENT_TO_MOTION:
            Actor_FaceMotion(self, pOrient);
            break;
        case AI_ORIENT_TO_ENEMY:
            Actor_FaceEnemy(self, pOrient);
            break;
        case AI_ORIENT_TO_ENEMY_OR_MOTION:
            Actor_FaceEnemyOrMotion(self, pOrient);
            break;
        case AI_ORIENT_TO_ENEMY_OR_MOTION_SIDESTEP:
            Actor_FaceEnemyOrMotionSidestep(self, pOrient);
            break;
        case AI_ORIENT_TO_GOAL:
            node = self->sentient->pClaimedNode;
            if ( node )
            {
                if ( Path_IsValidClaimNode(node) )
                    Actor_SetDesiredAngles(pOrient, 0.0, node->constant.fAngle);
            }
            break;
        default:
            v1 = va(
                         "invalid orient mode %i (code = %i, script = %i)",
                         pOrient->eMode,
                         self->CodeOrient.eMode,
                         self->ScriptOrient.eMode);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_orientation.cpp", 830, 0, v1) )
                __debugbreak();
            break;
    }
}

void __fastcall Actor_FaceMotion(actor_s *self, ai_orient_t *pOrient)
{
    float vDir[3]; // [esp+2Ch] [ebp-18h] BYREF
    float forward[3]; // [esp+38h] [ebp-Ch] BYREF

    if ( !self
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_orientation.cpp", 325, 0, "%s", "self") )
    {
        __debugbreak();
    }
    self->isFacingMotion = 1;
    if ( Actor_HasPath(self) && self->eAnimMode == AI_ANIM_MOVE_CODE )
    {
        if ( self->moveMode
            && Path_CompleteLookahead(&self->Path)
            && self->Path.fLookaheadDist < 60.0
            && (float)((float)(self->vLookForward[0] * self->Path.lookaheadDir[0])
                             + (float)(self->vLookForward[1] * self->Path.lookaheadDir[1])) < 0.0 )
        {
            goto dont_change;
        }
        if ( ai_angularYawEnabled->current.enabled )
        {
            vDir[0] = self->Physics.vWishDelta[0];
            vDir[1] = self->Physics.vWishDelta[1];
        }
        else
        {
            Actor_GetMoveHistoryAverage(self, vDir);
        }
        if ( vDir[0] == 0.0 && vDir[1] == 0.0
            || !self->moveMode
            && (Vec2Normalize(vDir),
                    YawVectors(pOrient->fDesiredBodyYaw, forward, 0),
                    (float)((float)(forward[0] * vDir[0]) + (float)(forward[1] * vDir[1])) >= 0.89999998) )
        {
dont_change:
            Actor_SetDesiredAngles(pOrient, self->ent->r.currentAngles[0], self->ent->r.currentAngles[1]);
        }
        else
        {
            Actor_FaceVector(pOrient, vDir);
        }
    }
}

void __fastcall Actor_FaceEnemyOrMotion(actor_s *self, ai_orient_t *pOrient)
{
    float *currentOrigin; // [esp+14h] [ebp-2Ch]
    float enemyDistSqrd; // [esp+24h] [ebp-1Ch]
    float vEnemyDir[3]; // [esp+28h] [ebp-18h] BYREF
    float vEnemyPos[3]; // [esp+34h] [ebp-Ch] BYREF

    iassert(self);
    iassert(pOrient);

    if ( !Actor_HasPath(self) || self->Physics.vVelocity[0] == 0.0 && self->Physics.vVelocity[1] == 0.0 )
    {
        Actor_FaceEnemy(self, pOrient);
    }
    else if ( Actor_GetTargetEntity(self) && Actor_CanSeeEnemy(self) )
    {
        Actor_GetTargetPosition(self, vEnemyPos);
        enemyDistSqrd = Vec3DistanceSq(self->ent->r.currentOrigin, vEnemyPos);
        if ( enemyDistSqrd <= 122500.0 )
        {
            currentOrigin = self->ent->r.currentOrigin;
            vEnemyDir[0] = vEnemyPos[0] - *currentOrigin;
            vEnemyDir[1] = vEnemyPos[1] - currentOrigin[1];
            vEnemyDir[2] = vEnemyPos[2] - currentOrigin[2];
            Vec3Normalize(vEnemyDir);
            if ( Path_MayFaceEnemy(&self->Path, vEnemyDir, self->ent->r.currentOrigin) )
                Actor_FaceEnemy(self, pOrient);
            else
                Actor_FaceMotion(self, pOrient);
        }
        else
        {
            Actor_FaceMotion(self, pOrient);
        }
    }
    else
    {
        Actor_FaceMotion(self, pOrient);
    }
}

void __fastcall Actor_FaceEnemyOrMotionSidestep(actor_s *self, ai_orient_t *pOrient)
{
    float *currentOrigin; // [esp+14h] [ebp-1Ch]
    float vEnemyDir[3]; // [esp+18h] [ebp-18h] BYREF
    float vEnemyPos[3]; // [esp+24h] [ebp-Ch] BYREF

    if ( !self
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_orientation.cpp", 733, 0, "%s", "self") )
    {
        __debugbreak();
    }
    if ( !pOrient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_orientation.cpp", 734, 0, "%s", "pOrient") )
    {
        __debugbreak();
    }
    if ( !Actor_HasPath(self) || self->Physics.vVelocity[0] == 0.0 && self->Physics.vVelocity[1] == 0.0 )
    {
        Actor_FaceEnemy(self, pOrient);
    }
    else
    {
        Actor_GetTargetPosition(self, vEnemyPos);
        currentOrigin = self->ent->r.currentOrigin;
        vEnemyDir[0] = vEnemyPos[0] - *currentOrigin;
        vEnemyDir[1] = vEnemyPos[1] - currentOrigin[1];
        vEnemyDir[2] = vEnemyPos[2] - currentOrigin[2];
        Vec3Normalize(vEnemyDir);
        if ( Path_MayFaceEnemy(&self->Path, vEnemyDir, self->ent->r.currentOrigin) )
            Actor_FaceEnemy(self, pOrient);
        else
            Actor_FaceMotion(self, pOrient);
    }
}

void __fastcall Actor_SetOrientMode(actor_s *self, ai_orient_mode_t eMode)
{
    if ( !self
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_orientation.cpp", 845, 0, "%s", "self") )
    {
        __debugbreak();
    }
    if ( (eMode <= AI_ORIENT_INVALID || eMode >= AI_ORIENT_COUNT)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_orientation.cpp",
                    846,
                    0,
                    "%s",
                    "eMode > AI_ORIENT_INVALID && eMode < AI_ORIENT_COUNT") )
    {
        __debugbreak();
    }
    //if ( self->sentient && EntHandle::isDefined(&self->sentient->syncedMeleeEnt) )
    if ( self->sentient && self->sentient->syncedMeleeEnt.isDefined() )
        self->CodeOrient.eMode = AI_ORIENT_DONT_CHANGE;
    else
        self->CodeOrient.eMode = eMode;
}

void __fastcall Actor_ClearScriptOrient(actor_s *self)
{
    if ( self->ScriptOrient.eMode )
        self->CodeOrient = self->ScriptOrient;
    else
        self->ScriptOrient = self->CodeOrient;
    self->ScriptOrient.eMode = AI_ORIENT_INVALID;
}

