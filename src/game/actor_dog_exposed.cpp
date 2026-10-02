#include "actor_dog_exposed.h"
#include <game_mp/g_main_mp.h>
#include "actor_state.h"
#include <game_mp/actor_mp.h>
#include "actor_corpse.h"
#include "actor_orientation.h"
#include "actor_team_move.h"
#include "actor_exposed.h"
#include "actor_navigation.h"
#include <qcommon/cm_world.h>
#include <bgame/bg_dog.h>
#include "g_debug.h"

bool __fastcall Actor_Dog_Exposed_Start(actor_s *self, ai_state_t ePrevState)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_dog_exposed.cpp", 34, 0, "%s", "self") )
        __debugbreak();
    self->ProneInfo.prone = 1;
    self->ProneInfo.orientPitch = 1;
    self->ProneInfo.iProneTime = level.time;
    self->ProneInfo.iProneTrans = 500;
    Actor_SetSubState(self, STATE_EXPOSED_COMBAT);
    return 1;
}

void __fastcall Actor_Dog_Exposed_Finish(actor_s *self, ai_state_t eNextState)
{
    self->ProneInfo.prone = 0;
    self->ProneInfo.orientPitch = 0;
    self->ProneInfo.fTorsoPitch = 0.0f;
}

void __fastcall Actor_Dog_Exposed_Suspend(actor_s *self, ai_state_t eNextState)
{
    Actor_Dog_Exposed_Finish(self, eNextState);
}

float dog_max_attack_height = 80.0f;
bool __fastcall Actor_Dog_PointNearAttackPoint(const float *vPoint, const float *vGoalPos, float buffer)
{
    float v4; // [esp+10h] [ebp-8h]
    float deltaHeight; // [esp+14h] [ebp-4h]

    deltaHeight = vPoint[2] - vGoalPos[2];
    if ( (float)(deltaHeight * deltaHeight) > (float)(dog_max_attack_height * dog_max_attack_height) )
        return 0;
    v4 = vGoalPos[1] - vPoint[1];
    return (float)(buffer * buffer) >= (float)((float)((float)(*vGoalPos - *vPoint) * (float)(*vGoalPos - *vPoint))
                                                                                     + (float)(v4 * v4));
}

double __fastcall Actor_Dog_GetDeltaTurnYaw(actor_s *self)
{
    float lookAheadYaw; // [esp+14h] [ebp-14h]
    float lookAheadAngles[3]; // [esp+18h] [ebp-10h] BYREF
    float currentYaw; // [esp+24h] [ebp-4h]

    vectoangles(self->Path.lookaheadDir, lookAheadAngles);
    lookAheadYaw = AngleNormalize180(lookAheadAngles[1]);
    currentYaw = AngleNormalize180(self->ent->r.currentAngles[1]);
    return AngleNormalize180(lookAheadYaw - currentYaw);
}

actor_think_result_t __fastcall Actor_Dog_Exposed_Think(actor_s *self)
{
    bool v4; // [esp+8h] [ebp-48h]
    bool v5; // [esp+Ch] [ebp-44h]
    float v7; // [esp+24h] [ebp-2Ch]
    float v8; // [esp+28h] [ebp-28h]
    int goalPosSet; // [esp+34h] [ebp-1Ch] BYREF
    bool noDodge; // [esp+3Bh] [ebp-15h]
    sentient_s *enemy; // [esp+3Ch] [ebp-14h]
    int enemyInAttackRange; // [esp+40h] [ebp-10h]
    float MAX_MOVE_DIST_SQ; // [esp+44h] [ebp-Ch]
    int enemyInGoal; // [esp+48h] [ebp-8h]
    int attackScriptRunning; // [esp+4Ch] [ebp-4h]

    PROF_SCOPED("dogexposedthink");

    if ( !self
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_dog_exposed.cpp", 624, 0, "%s", "self") )
    {
        __debugbreak();
    }
    if ( !self->sentient
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_dog_exposed.cpp",
                    625,
                    0,
                    "%s",
                    "self->sentient") )
    {
        __debugbreak();
    }
    enemyInAttackRange = 0;
    goalPosSet = 0;
    attackScriptRunning = 0;
    noDodge = 1;
    self->pszDebugInfo = "exposed";
    Actor_PreThink(self);
    enemy = Actor_GetTargetSentient(self);
    v5 = (AnimScriptList *)self->pAnimScriptFunc == &g_scr_data.dogAnim
        && Actor_IsAnimScriptAlive(self)
        && !self->safeToChangeScript;
    attackScriptRunning = v5;
    if ( !v5 )
        Actor_OrientPitchToGround(self->ent, 1);
    if ( !attackScriptRunning && enemy )
        enemyInAttackRange = Actor_Dog_IsEnemyInAttackRange(self, enemy, &goalPosSet);
    v4 = enemy && Actor_PointAtGoal(enemy->ent->r.currentOrigin, &self->codeGoal);
    enemyInGoal = v4;
    if ( !v4 )
    {
        self->useEnemyGoal = 0;
        self->useMeleeAttackSpot = 0;
    }
    if ( !goalPosSet )
        Actor_UpdateGoalPos(self);
    MAX_MOVE_DIST_SQ = 225.0f;
    if ( Actor_HasPath(self) )
    {
        v7 = self->Path.vFinalGoal[0] - self->codeGoal.pos[0];
        v8 = self->Path.vFinalGoal[1] - self->codeGoal.pos[1];
        if ( (float)((float)(v7 * v7) + (float)(v8 * v8)) > MAX_MOVE_DIST_SQ )
            Actor_ClearPath(self);
    }
    if ( !enemy || self->useEnemyGoal )
    {
        Actor_FindPathToGoalDirect(self);
    }
    else if ( !Actor_HasPath(self) )
    {
        Actor_FindPathToGoalNearestNode(self);
        noDodge = 0;
    }
    if ( !Actor_HasPath(self) )
    {
        if ( self->useMeleeAttackSpot )
        {
            Actor_UpdateGoalPos(self);
            Actor_FindPathToGoalDirect(self);
        }
        if ( enemy && !Actor_HasPath(self) )
        {
            Actor_FindPathToGoalNearestNode(self);
            noDodge = 0;
        }
    }
    self->noDodgeMove = noDodge;
    if ( !self->flashBanged || Actor_Dog_IsInSyncedMelee(self, enemy) )
    {
        if ( attackScriptRunning || enemyInAttackRange )
        {
            Actor_Dog_Attack(self);
        }
        else if ( Actor_HasPath(self) )
        {
            if ( Actor_IsMoving(self) && Actor_Dog_ObstacleInPath(self) )
            {
                Actor_SetOrientMode(self, AI_ORIENT_TO_MOTION);
                Actor_SetAnimScript(
                    self,
                    &g_animScriptTable[self->species]->jump,
                    AI_MOVE_RUN,
                    AI_ANIM_USE_BOTH_DELTAS_NOCLIP,
                    AI_ANIM_FUNCTION_MOVE);
            }
            else if ( Actor_IsMoving(self) && Actor_Dog_ShouldTurn(self) )
            {
                Actor_SetOrientMode(self, AI_ORIENT_TO_MOTION);
                Actor_SetAnimScript(self, &g_animScriptTable[self->species]->turn, AI_MOVE_RUN, AI_ANIM_MOVE_CODE, AI_ANIM_FUNCTION_MOVE);
            }
            else if ( (self->pAnimScriptFunc != &g_scr_data.dogAnim.jump
                            || !Actor_IsAnimScriptAlive(self)
                            || self->safeToChangeScript)
                         && (self->pAnimScriptFunc != &g_scr_data.dogAnim.turn
                            || !Actor_IsAnimScriptAlive(self)
                            || self->safeToChangeScript) )
            {
                Actor_SetOrientMode(self, AI_ORIENT_TO_MOTION);
                Actor_MoveAlongPathWithTeam(self, 1, 0, 0);
            }
        }
        else
        {
            self->useMeleeAttackSpot = 0;
            Actor_AnimStop(self, &g_animScriptTable[self->species]->stop);
        }
    }
    else
    {
        Actor_Exposed_FlashBanged(self);
    }
    Actor_PostThink(self);
    return ACTOR_THINK_DONE;
}

bool __cdecl Actor_Dog_IsInSyncedMelee(actor_s *self, sentient_s *enemy)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_dog_exposed.cpp", 79, 0, "%s", "self") )
        __debugbreak();
    //return enemy && EntHandle::isDefined(&enemy->syncedMeleeEnt) && EntHandle::ent(&enemy->syncedMeleeEnt) == self->ent;
    return enemy && enemy->syncedMeleeEnt.isDefined() && enemy->syncedMeleeEnt.ent() == self->ent;
}

void __cdecl Actor_Dog_Attack(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_dog_exposed.cpp", 93, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_dog_exposed.cpp",
                    94,
                    0,
                    "%s",
                    "self->sentient") )
    {
        __debugbreak();
    }
    //if ( EntHandle::isDefined(&self->sentient->targetEnt) )
    if ( self->sentient->targetEnt.isDefined() )
    {
        if ( (AnimScriptList *)self->pAnimScriptFunc == &g_scr_data.dogAnim && !Actor_IsAnimScriptAlive(self) )
            Actor_KillAnimScript(self);
        Actor_SetAnimScript(self, &g_animScriptTable[self->species]->combat, AI_MOVE_STOP, AI_ANIM_MOVE_CODE, AI_ANIM_FUNCTION_STOP);
    }
}

void __fastcall Actor_FindPathToGoalNearestNode(actor_s *self)
{
    pathnode_t *nodeTo; // [esp+18h] [ebp-31Ch]
    pathsort_t nodes[64]; // [esp+1Ch] [ebp-318h] BYREF
    sentient_s *enemy; // [esp+31Ch] [ebp-18h]
    int nodeCount; // [esp+320h] [ebp-14h] BYREF
    pathnode_t *nodeFrom; // [esp+324h] [ebp-10h]
    float targetOrigin[3]; // [esp+328h] [ebp-Ch] BYREF

    targetOrigin[0] = self->codeGoal.pos[0];
    targetOrigin[1] = self->codeGoal.pos[1];
    targetOrigin[2] = self->codeGoal.pos[2];
    if ( self->codeGoalSrc == AI_GOAL_SRC_ENEMY )
        targetOrigin[2] = targetOrigin[2] + 64.0;
    enemy = Actor_GetTargetSentient(self);
    if ( enemy )
        nodeTo = Sentient_NearestNode(enemy);
    else
        nodeTo = Path_NearestNode(
                             targetOrigin,
                             nodes,
                             -2,
                             self->codeGoal.radius,
                             &nodeCount,
                             64,
                             NEAREST_NODE_DO_HEIGHT_CHECK);
    if ( nodeTo )
    {
        nodeFrom = Sentient_NearestNode(self->sentient);
        if ( nodeFrom )
            Path_FindPathGetCloseAsPossible(
                &self->Path,
                self->sentient->eTeam,
                nodeFrom,
                self->ent->r.currentOrigin,
                nodeTo,
                nodeTo->constant.vOrigin,
                1);
    }
    else
    {
        Actor_FindPathToGoalDirect(self);
    }
}

char __fastcall Actor_Dog_ObstacleInPath(const actor_s *self)
{
    float v2; // [esp+14h] [ebp-1024h]
    float v3; // [esp+18h] [ebp-1020h]
    float v4; // [esp+1Ch] [ebp-101Ch]
    float mins[3]; // [esp+20h] [ebp-1018h] BYREF
    int v8; // [esp+2Ch] [ebp-100Ch]
    int entityList[1024]; // [esp+30h] [ebp-1008h] BYREF
    gentity_s *v10; // [esp+1030h] [ebp-8h]
    int i; // [esp+1034h] [ebp-4h]

    v2 = self->ent->r.currentOrigin[0];
    v3 = self->ent->r.currentOrigin[1];
    v4 = self->ent->r.currentOrigin[2] + 20.0;
    mins[0] = self->vLookForward[0];
    mins[1] = self->vLookForward[1];
    mins[2] = self->vLookForward[2];
    mins[0] = (float)(32.0 * mins[0]) + v2;
    mins[1] = (float)(32.0 * mins[1]) + v3;
    mins[2] = (float)(32.0 * mins[2]) + v4;
    v8 = CM_AreaEntities(mins, mins, entityList, 1024, 0x200000);
    for ( i = 0; i < v8; ++i )
    {
        v10 = &g_entities[entityList[i]];
        if ( v10->r.inuse
            && (v10->r.contents & 1) != 0
            && (v10->s.lerp.eFlags & 0x20) == 0
            && (v10->r.contents & 0x100) == 0 )
        {
            return 1;
        }
    }
    return 0;
}

bool __fastcall Actor_Dog_ShouldTurn(actor_s *self)
{
    float angle; // [esp+8h] [ebp-4h]

    if ( self->pAnimScriptFunc == &g_scr_data.dogAnim.turn && Actor_IsAnimScriptAlive(self) && !self->safeToChangeScript )
        return 0;
    if ( dog_turn_min_goal_dist->current.value > self->Path.fLookaheadDist )
        return 0;
    angle = Actor_Dog_GetDeltaTurnYaw(self);
    return fabs(angle) > dog_turn90_angle->current.value;
}

int __cdecl Actor_Dog_IsEnemyInAttackRange(actor_s *self, sentient_s *enemy, int *goalPosSet)
{
    bool v3; // al
    float *currentOrigin; // eax
    col_context_t context; // [esp+24h] [ebp-A4h] BYREF
    float mins[3]; // [esp+4Ch] [ebp-7Ch] BYREF
    float enemyToMe[2]; // [esp+94h] [ebp-34h] BYREF
    float cos45; // [esp+9Ch] [ebp-2Ch]
    float enemyToAttackSpot[2]; // [esp+A0h] [ebp-28h] BYREF
    float enemyPos[3]; // [esp+A8h] [ebp-20h] BYREF
    int enemyInAttackRange; // [esp+B4h] [ebp-14h]
    float bufferedAttackDist; // [esp+B8h] [ebp-10h]
    float attackPos[3]; // [esp+BCh] [ebp-Ch] BYREF

    if ( !self
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_dog_exposed.cpp", 539, 0, "%s", "self") )
    {
        __debugbreak();
    }
    if ( !enemy
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_dog_exposed.cpp", 540, 0, "%s", "enemy") )
    {
        __debugbreak();
    }
    if ( !goalPosSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_dog_exposed.cpp", 541, 0, "%s", "goalPosSet") )
    {
        __debugbreak();
    }
    enemyInAttackRange = 0;
    bufferedAttackDist = Actor_Dog_GetEnemyPos(self, enemy, enemyPos);
    *goalPosSet = 0;
    self->useEnemyGoal = 0;
    self->useMeleeAttackSpot = 0;
    v3 = Actor_Dog_PointNearAttackPoint(self->ent->r.currentOrigin, enemyPos, bufferedAttackDist);
    enemyInAttackRange = v3;
    if ( v3 )
    {
        self->useEnemyGoal = 1;
        self->useMeleeAttackSpot = 1;
    }
    if ( Actor_SetMeleeAttackSpot(self, enemyPos, attackPos) )
    {
        if ( enemyInAttackRange )
        {
            cos45 = 0.70700002f;
            enemyToAttackSpot[0] = attackPos[0] - enemyPos[0];
            enemyToAttackSpot[1] = attackPos[1] - enemyPos[1];
            Vec2Normalize(enemyToAttackSpot);
            currentOrigin = self->ent->r.currentOrigin;
            enemyToMe[0] = *currentOrigin - enemyPos[0];
            enemyToMe[1] = currentOrigin[1] - enemyPos[1];
            Vec2Normalize(enemyToMe);
            enemyInAttackRange = (float)((float)(enemyToAttackSpot[0] * enemyToMe[0])
                                                                 + (float)(enemyToAttackSpot[1] * enemyToMe[1])) > cos45;
        }
        Actor_UpdateMeleeGoalPos(self, attackPos);
        *goalPosSet = 1;
    }
    else if ( enemyInAttackRange )
    {
        trace_t trace; // [esp+58h] [ebp-70h] BYREF
        mins[0] = actorMins[0];
        mins[1] = -15.0;
        mins[2] = 18.0f;
        //col_context_t::col_context_t(&context);
        G_TraceCapsule(
            &trace,
            self->ent->r.currentOrigin,
            mins,
            actorMaxs,
            enemyPos,
            enemy->ent->s.number,
            0x2820011,
            &context);
        if ( trace.hitType == TRACE_HITTYPE_ENTITY && trace.hitId == enemy->ent->s.number )
            return 1;
        if ( trace.fraction < 1.0 || trace.startsolid || trace.allsolid )
            return 0;
    }
    return enemyInAttackRange;
}

float DROP_AMOUNT = 90.0f;
char __fastcall Actor_SetMeleeAttackSpot(actor_s *self, const float *enemyPosition, float *attackPosition)
{
    char *v4; // eax
    float v7; // [esp+20h] [ebp-118h]
    float v8; // [esp+24h] [ebp-114h]
    float v9; // [esp+30h] [ebp-108h]
    float v10; // [esp+34h] [ebp-104h]
    float *v11; // [esp+38h] [ebp-100h]
    float meleeAttackDist; // [esp+3Ch] [ebp-FCh]
    float *currentOrigin; // [esp+50h] [ebp-E8h]
    float dropPosition[3]; // [esp+54h] [ebp-E4h] BYREF
    float endPos[3]; // [esp+60h] [ebp-D8h] BYREF
    col_context_t context; // [esp+6Ch] [ebp-CCh] BYREF
    int currentOccupier; // [esp+94h] [ebp-A4h]
    float currentValue; // [esp+98h] [ebp-A0h]
    int currentIndex; // [esp+9Ch] [ebp-9Ch]
    int j; // [esp+A0h] [ebp-98h]
    float dirFromEnemy[2]; // [esp+A4h] [ebp-94h] BYREF
    float mins[3]; // [esp+ACh] [ebp-8Ch] BYREF
    sentient_s *enemy; // [esp+B8h] [ebp-80h]
    float bestFraction; // [esp+BCh] [ebp-7Ch]
    float maxs[3]; // [esp+C0h] [ebp-78h] BYREF
    trace_t trace; // [esp+CCh] [ebp-6Ch] BYREF
    float dotProducts[4]; // [esp+108h] [ebp-30h]
    int indices[4]; // [esp+118h] [ebp-20h]
    float bestPosition[3]; // [esp+128h] [ebp-10h]
    int i; // [esp+134h] [ebp-4h]

    if ( !self
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_dog_exposed.cpp", 163, 0, "%s", "self") )
    {
        __debugbreak();
    }
    if ( !enemyPosition
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_dog_exposed.cpp",
                    164,
                    0,
                    "%s",
                    "enemyPosition") )
    {
        __debugbreak();
    }
    bestFraction = 0.0f;
    enemy = Actor_GetTargetSentient(self);
    currentOrigin = self->ent->r.currentOrigin;
    dirFromEnemy[0] = *currentOrigin - *enemyPosition;
    dirFromEnemy[1] = currentOrigin[1] - enemyPosition[1];
    Vec2Normalize(dirFromEnemy);
    for ( i = 0; i < 4; ++i )
    {
        dotProducts[i] = (float)(dirFromEnemy[0] * (float)meleeAttackOffsets[i][0])
                                     + (float)(dirFromEnemy[1] * (float)meleeAttackOffsets[i][1]);
        indices[i] = i;
    }
    for ( i = 1; i < 4; ++i )
    {
        currentValue = dotProducts[indices[i]];
        currentIndex = indices[i];
        for ( j = i - 1; j >= 0 && dotProducts[indices[j]] <= currentValue; --j )
            indices[j + 1] = indices[j];
        indices[j + 1] = currentIndex;
    }
    for ( i = 0; i < 4; ++i )
    {
        if ( enemy->meleeAttackerSpot[i] == self->ent->s.number )
        {
            enemy->meleeAttackerSpot[i] = 0;
            break;
        }
    }
    attackPosition[2] = enemyPosition[2];
    mins[0] = actorMins[0];
    mins[1] = -15.0;
    mins[2] = 18.0f;
    maxs[0] = actorMaxs[0];
    maxs[1] = 15.0;
    maxs[2] = 48.0;
    for ( i = 0; i < 4; ++i )
    {
        currentOccupier = enemy->meleeAttackerSpot[indices[i]];
        if ( currentOccupier >= 1024
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_dog_exposed.cpp",
                        233,
                        0,
                        "%s",
                        "currentOccupier < MAX_GENTITIES") )
        {
            __debugbreak();
        }
        v11 = (float *)meleeAttackOffsets[indices[i]];
        meleeAttackDist = self->meleeAttackDist;
        *attackPosition = (float)(meleeAttackDist * *v11) + *enemyPosition;
        attackPosition[1] = (float)(meleeAttackDist * v11[1]) + enemyPosition[1];
        if ( currentOccupier <= 0 )
            goto LABEL_33;
        if ( !g_entities[currentOccupier].r.inuse
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_dog_exposed.cpp",
                        239,
                        0,
                        "%s",
                        "g_entities[currentOccupier].r.inuse") )
        {
            __debugbreak();
        }
        v9 = g_entities[currentOccupier].r.currentOrigin[0] - *attackPosition;
        v10 = g_entities[currentOccupier].r.currentOrigin[1] - attackPosition[1];
        v7 = self->ent->r.currentOrigin[0] - *attackPosition;
        v8 = self->ent->r.currentOrigin[1] - attackPosition[1];
        if ( (float)((float)(v7 * v7) + (float)(v8 * v8)) < (float)((float)(v9 * v9) + (float)(v10 * v10)) )
        {
LABEL_33:
            //col_context_t::col_context_t(&context);
            G_TraceCapsule(&trace, attackPosition, mins, maxs, attackPosition, enemy->ent->s.number, 0x20000, &context);
            if ( !trace.startsolid && !trace.allsolid )
                G_TraceCapsule(&trace, attackPosition, mins, maxs, enemyPosition, enemy->ent->s.number, 41943057, &context);
            if ( trace.fraction < 1.0 && ai_debugMeleeAttackSpots->current.enabled )
            {
                Vec3Lerp(attackPosition, enemyPosition, trace.fraction, endPos);
                G_DebugStar(endPos, colorYellow, 1);
            }
            if ( trace.fraction != 1.0 || trace.startsolid || trace.allsolid )
            {
                if ( trace.fraction > bestFraction )
                {
                    bestFraction = trace.fraction;
                    bestPosition[0] = *attackPosition;
                    bestPosition[1] = attackPosition[1];
                    bestPosition[2] = attackPosition[2];
                }
                if ( ai_debugMeleeAttackSpots->current.enabled )
                    G_DebugLine(attackPosition, enemyPosition, colorRed, 0);
            }
            else
            {
                dropPosition[0] = *attackPosition;
                dropPosition[1] = attackPosition[1];
                dropPosition[2] = attackPosition[2];
                dropPosition[2] = dropPosition[2] - DROP_AMOUNT;
                G_TraceCapsule(&trace, attackPosition, mins, actorMaxs, dropPosition, enemy->ent->s.number, 41943057, &context);
                if ( trace.fraction < 1.0 && !trace.allsolid && !trace.startsolid )
                {
                    attackPosition[2] = (float)(attackPosition[2] - (float)(DROP_AMOUNT * trace.fraction)) + 1.0;
                    enemy->meleeAttackerSpot[indices[i]] = self->ent->s.number;
                    break;
                }
                if ( ai_debugMeleeAttackSpots->current.enabled )
                    G_DebugLine(attackPosition, dropPosition, colorRed, 0);
            }
        }
    }
    if ( i == 4 )
    {
        if ( bestFraction <= 0.94999999 )
            return 0;
        *attackPosition = bestPosition[0];
        attackPosition[1] = bestPosition[1];
        attackPosition[2] = bestPosition[2];
    }
    if ( ai_debugMeleeAttackSpots->current.enabled && enemy )
    {
        G_DebugCircle(attackPosition, 15.0, colorYellow, 0, 1, 0);
        v4 = va("%i", self->ent->s.number);
        G_AddDebugString(attackPosition, colorYellow, 0.69999999, v4, 0);
        G_DebugLine(attackPosition, enemyPosition, colorGreen, 0);
    }
    return 1;
}

void __fastcall Actor_UpdateMeleeGoalPos(actor_s *self, float *goalPos)
{
    if ( !self
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_dog_exposed.cpp", 340, 0, "%s", "self") )
    {
        __debugbreak();
    }
    if ( !goalPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_dog_exposed.cpp", 341, 0, "%s", "goalPos") )
    {
        __debugbreak();
    }
    self->codeGoal.pos[0] = *goalPos;
    self->codeGoal.pos[1] = goalPos[1];
    self->codeGoal.pos[2] = goalPos[2];
    self->codeGoalSrc = AI_GOAL_SRC_ENEMY;
    self->codeGoal.node = 0;
    self->codeGoal.volume = 0;
    Actor_SetGoalRadius(&self->codeGoal, self->pathEnemyFightDist);
}

double __cdecl Actor_Dog_GetEnemyPos(actor_s *self, sentient_s *enemy, float *enemyPos)
{
    gclient_s *client; // eax
    float *currentOrigin; // [esp+8h] [ebp-10h]
    float bufferedAttackDist; // [esp+14h] [ebp-4h]

    if ( !enemy
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_dog_exposed.cpp", 396, 0, "%s", "enemy") )
    {
        __debugbreak();
    }
    if ( !enemyPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_dog_exposed.cpp", 397, 0, "%s", "enemyPos") )
    {
        __debugbreak();
    }
    currentOrigin = enemy->ent->r.currentOrigin;
    *enemyPos = *currentOrigin;
    enemyPos[1] = currentOrigin[1];
    enemyPos[2] = currentOrigin[2];
    bufferedAttackDist = self->meleeAttackDist + 15.0;
    if ( enemy->ent->client )
    {
        if ( (float)((float)(enemy->ent->client->ps.velocity[0] * enemy->ent->client->ps.velocity[0])
                             + (float)(enemy->ent->client->ps.velocity[1] * enemy->ent->client->ps.velocity[1])) > 1.0 )
        {
            client = enemy->ent->client;
            *enemyPos = (float)(0.25 * client->ps.velocity[0]) + *enemyPos;
            enemyPos[1] = (float)(0.25 * client->ps.velocity[1]) + enemyPos[1];
            enemyPos[2] = (float)(0.25 * client->ps.velocity[2]) + enemyPos[2];
            bufferedAttackDist = bufferedAttackDist + 15.0;
        }
        //if ( EntHandle::isDefined(&enemy->ent->sentient->syncedMeleeEnt) )
        if ( enemy->ent->sentient->syncedMeleeEnt.isDefined() )
            enemyPos[2] = enemyPos[2] + 64.0;
    }
    return bufferedAttackDist;
}

