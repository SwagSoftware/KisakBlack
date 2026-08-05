#include "actor_threat.h"
#include "actor.h"
#include <game_mp/actor_mp.h>
#include <game_mp/g_main_mp.h>
#include "actor_state.h"
#include <game_mp/g_utils_mp.h>
#include "bullet.h"
#include <bgame/bg_perks.h>
#include <cgame/cg_drawtools.h>
#include <client_mp/cl_cgame_mp.h>
#include "actor_pain.h"
#include "g_debug.h"
#include <flame/flame_system.h>

threat_bias_t g_threatBias;
int g_skipDebugString;
char g_threatDebugStrings[10][64];

static const float AI_THREAT_DISTANCE_RATE = (float)(1.0 / (float)(2500.0 * 2500.0)) * 5000.0;

const char *g_threatDebugLabels[10] =
{
  "Total:",
  "Flashed",
  "Suppressed",
  "Bias",
  "BiasGroup",
  "Attacker",
  "CurBonus",
  "Awareness",
  "Dist",
  "Scariness"
};


bool __fastcall Actor_CheckIgnore(sentient_s *self, sentient_s *enemy)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 135, 0, "%s", "self") )
        __debugbreak();
    if ( !enemy && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 136, 0, "%s", "enemy") )
        __debugbreak();
    return enemy->bIgnoreMe
            || Actor_GetThreatBias(enemy->iThreatBiasGroupIndex, self->iThreatBiasGroupIndex) == 0x80000000;
}

void __cdecl Actor_InitThreatBiasGroups()
{
    *(unsigned int *)g_threatBias.groupName = 0;
    *(unsigned int *)&g_threatBias.groupName[2] = 0;
    *(unsigned int *)&g_threatBias.groupName[4] = 0;
    *(unsigned int *)&g_threatBias.groupName[6] = 0;
    *(unsigned int *)&g_threatBias.groupName[8] = 0;
    *(unsigned int *)&g_threatBias.groupName[10] = 0;
    *(unsigned int *)&g_threatBias.groupName[12] = 0;
    *(unsigned int *)&g_threatBias.groupName[14] = 0;
    memset(g_threatBias.threatTable, 0, sizeof(g_threatBias.threatTable));
    g_threatBias.threatGroupCount = 1;
}

int __fastcall Actor_GetThreatBias(int groupSelf, int groupEnemy)
{
    if ( (groupSelf < 0 || groupSelf >= g_threatBias.threatGroupCount)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp",
                    276,
                    0,
                    "%s",
                    "groupSelf >= 0 && groupSelf < g_threatBias.threatGroupCount") )
    {
        __debugbreak();
    }
    if ( (groupEnemy < 0 || groupEnemy >= g_threatBias.threatGroupCount)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp",
                    277,
                    0,
                    "%s",
                    "groupEnemy >= 0 && groupEnemy < g_threatBias.threatGroupCount") )
    {
        __debugbreak();
    }
    return g_threatBias.threatTable[groupSelf][groupEnemy];
}

void __fastcall Actor_FlagEnemyUnattackable(actor_s *self)
{
    sentient_s *enemy; // [esp+4h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 292, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 293, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    enemy = Actor_GetTargetSentient(self);
    if ( enemy )
    {
        if ( self->eState[self->stateLevel] == AIS_EXPOSED )
        {
            self->sentientInfo[enemy - level.sentients].attackTime = 0x7FFFFFFF;
            Actor_SetSubState(self, STATE_EXPOSED_NONCOMBAT);
        }
    }
}

bool __fastcall Actor_CaresAboutInfo(actor_s *self, sentient_s *pOther)
{
    int lastKnownPosTime; // [esp+8h] [ebp-8h]

    if ( !pOther && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 319, 0, "%s", "pOther") )
        __debugbreak();
    lastKnownPosTime = self->sentientInfo[pOther - level.sentients].lastKnownPosTime;
    return lastKnownPosTime <= 0 || level.time - lastKnownPosTime >= 2000;
}

double __cdecl Sentient_GetScarinessForDistance(sentient_s *self, sentient_s *enemy, float fDist)
{
    float fAccuracy; // [esp+4h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 816, 0, "%s", "self") )
        __debugbreak();
    if ( !self->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 817, 0, "%s", "self->ent") )
    {
        __debugbreak();
    }
    if ( !enemy && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 818, 0, "%s", "enemy") )
        __debugbreak();
    if ( fDist < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 819, 0, "%s", "fDist >= 0") )
    {
        __debugbreak();
    }
    if ( ai_playerNearRange->current.value < fDist )
    {
        if ( fDist >= ai_playerFarRange->current.value
            || ai_playerNearAccuracy->current.value == ai_playerFarAccuracy->current.value )
        {
            fAccuracy = ai_playerFarAccuracy->current.value;
        }
        else
        {
            fAccuracy = (float)((float)(ai_playerFarAccuracy->current.value - ai_playerNearAccuracy->current.value)
                                                * (float)((float)(fDist - ai_playerNearRange->current.value)
                                                                / (float)(ai_playerFarRange->current.value - ai_playerNearRange->current.value)))
                                + ai_playerNearAccuracy->current.value;
        }
    }
    else
    {
        fAccuracy = ai_playerNearAccuracy->current.value;
    }
    return fAccuracy * 5.0;
}

void __fastcall Actor_InitThreatUpdateInterval(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 955, 0, "%s", "self") )
        __debugbreak();
    if ( ai_threatUpdateInterval->current.integer )
    {
        self->threatUpdateTime = ai_threatUpdateInterval->current.integer
                                                     * (level.time
                                                        / ai_threatUpdateInterval->current.integer);
        self->threatUpdateTime += G_irand(0, ai_threatUpdateInterval->current.integer);
    }
    else
    {
        self->threatUpdateTime = 0;
    }
}

void __fastcall Actor_UpdateThreat(actor_s *self)
{
    char *v1; // eax
    char *v2; // eax
    gentity_s *v3; // esi
    char *v4; // eax
    char *v5; // eax
    bool v6; // [esp+10h] [ebp-88h]
    bool v7; // [esp+14h] [ebp-84h]
    bool v8; // [esp+18h] [ebp-80h]
    gentity_s *veh; // [esp+68h] [ebp-30h]
    bool goodEnemyOnly; // [esp+6Fh] [ebp-29h]
    int maxThreat; // [esp+70h] [ebp-28h]
    gentity_s *scriptTargetEnt; // [esp+74h] [ebp-24h]
    sentient_s *enemy; // [esp+78h] [ebp-20h]
    gentity_s *pScariestEnemy; // [esp+7Ch] [ebp-1Ch]
    int iTeamFlags; // [esp+88h] [ebp-10h]
    sentient_info_t *pInfo; // [esp+8Ch] [ebp-Ch]
    int threat; // [esp+94h] [ebp-4h]

    PROF_SCOPED("Actor_UpdateThreat");

    iassert(self);
    iassert(self->sentient);

    if ( ai_threatUpdateInterval->current.integer )
    {
        if ( level.time < self->threatUpdateTime )
        {
            return;
        }
        Actor_IncrementThreatTime(self);
    }
    if ( ai_showPotentialThreatDir->current.enabled )
        Actor_PotentialThreat_Debug(self);
    scriptTargetEnt = Actor_GetScriptTargetEntity(self);
    if ( self->sentient->bIgnoreAll )
    {
        if ( Actor_GetTargetEntity(self) )
        {
            if ( Actor_GetTargetEntity(self) == scriptTargetEnt )
            {
                v1 = va("scripted enemy (%0.3f)", 1.0);
                DebugThreatStringSimple(self, scriptTargetEnt, v1, colorGreen);
            }
            else
            {
                Sentient_SetEnemy(self->sentient, 0, 1);
            }
        }
        return;
    }
    if ( scriptTargetEnt && self->sentient->entityTargetThreat == 1.0 )
    {
        v2 = va("scripted enemy (%0.3f)", 1.0);
        DebugThreatStringSimple(self, scriptTargetEnt, v2, colorGreen);
        Sentient_SetEnemy(self->sentient, scriptTargetEnt, 1);
        return;
    }
    iTeamFlags = 1 << Sentient_EnemyTeam(self->sentient->eTeam);
    maxThreat = -2147483647;
    pScariestEnemy = 0;
    goodEnemyOnly = 0;
    self->hasThreateningEnemy = 0;
    for ( enemy = Sentient_FirstSentient(iTeamFlags); enemy; enemy = Sentient_NextSentient(enemy, iTeamFlags) )
    {
        pInfo = &self->sentientInfo[enemy - level.sentients];
        if ( self->sentientInfo[enemy - level.sentients].lastKnownPosTime <= 0 )
        {
            DebugThreatStringSimple(self, enemy->ent, (char*)"unaware", colorRed);
            continue;
        }
        if ( (enemy->ent->flags & 4) != 0 || Actor_CheckIgnore(self->sentient, enemy) )
            goto LABEL_77;
        if ( enemy->ent->client && enemy->ent->client->sess.sessionState )
        {
            DebugThreatStringSimple(self, enemy->ent, (char *)"client not-playing", colorRed);
            continue;
        }
        if ( self->sentient != enemy )
        {
            //if ( EntHandle::isDefined(&self->sentient->scriptOwner)
            if ( self->sentient->scriptOwner.isDefined()
                //&& enemy->ent == EntHandle::ent(&self->sentient->scriptOwner) )
                && enemy->ent == self->sentient->scriptOwner.ent() )
            {
                DebugThreatStringSimple(self, enemy->ent, (char *)"client is owner", colorRed);
                continue;
            }
            if ( enemy->ent->actor
                //&& EntHandle::isDefined(&self->sentient->scriptOwner)
                && self->sentient->scriptOwner.isDefined()
                //&& EntHandle::isDefined(&enemy->scriptOwner)
                && enemy->scriptOwner.isDefined()
                //&& (v3 = EntHandle::ent(&self->sentient->scriptOwner), v3 == EntHandle::ent(&enemy->scriptOwner))
                && (v3 = self->sentient->scriptOwner.ent(), v3 == enemy->scriptOwner.ent())
                || enemy->ent->actor )
            {
                DebugThreatStringSimple(self, enemy->ent, (char *)"same owner so friendly", colorRed);
                continue;
            }
            if ( enemy->ent->client && (enemy->ent->client->ps.eFlags & 0x4000) != 0 )
            {
                veh = &g_entities[enemy->ent->client->ps.viewlocked_entNum];
                if ( !veh
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 1137, 0, "%s", "veh") )
                {
                    __debugbreak();
                }
                if ( !BG_GetVehicleInfo(veh->scr_vehicle->infoIdx)->remoteControl )
                {
                    DebugThreatStringSimple(self, enemy->ent, (char*)"client in vehicle", colorRed);
                    continue;
                }
            }
            if ( !perk_dogsAttackGhost->current.enabled && enemy->ent->client && (enemy->ent->client->ps.perks[1] & 8) != 0 )
            {
                DebugThreatStringSimple(self, enemy->ent, (char *)"client has ghost perk", colorRed);
                continue;
            }
            if ( enemy->ent->client && (enemy->ent->client->flags & 1) != 0 )
            {
                DebugThreatStringSimple(self, enemy->ent, (char *)"client has no clip", colorRed);
                continue;
            }
            v8 = pInfo->VisCache.iLastVisTime && level.time - pInfo->VisCache.iLastVisTime < 10000;
            v7 = pInfo->iLastAttackMeTime && level.time - pInfo->iLastAttackMeTime < 10000;
            v6 = (v8 || v7) && pInfo->attackTime <= level.time;
            self->hasThreateningEnemy = v6;
            if ( !v6 && goodEnemyOnly )
            {
                DebugThreatStringSimple(self, enemy->ent, (char *)"goodOnly", colorRed);
                continue;
            }
            threat = Actor_UpdateSingleThreat(self, enemy);
            if ( threat == 0x80000000 )
            {
LABEL_77:
                DebugThreatStringSimple(self, enemy->ent, (char *)"ignoreme", colorRed);
                continue;
            }
            if ( maxThreat < threat
                || !goodEnemyOnly && v6
                //|| SentientHandle::isDefined(&self->pFavoriteEnemy) && enemy == SentientHandle::sentient(&self->pFavoriteEnemy) )
                || self->pFavoriteEnemy.isDefined() && enemy == self->pFavoriteEnemy.sentient())
            {
                //if ( SentientHandle::isDefined(&self->pFavoriteEnemy) && enemy == SentientHandle::sentient(&self->pFavoriteEnemy) )
                if ( self->pFavoriteEnemy.isDefined() && enemy == self->pFavoriteEnemy.sentient())
                {
                    maxThreat = 2147483646;
                }
                else
                {
                    maxThreat = threat;
                }
                pScariestEnemy = enemy->ent;
                goodEnemyOnly = v6;
            }
        }
    }
    if ( scriptTargetEnt )
    {
        if ( self->sentient->entityTargetThreat <= (float)((float)maxThreat / 7000.0) )
        {
            v5 = va("enemy (%0.3f)", self->sentient->entityTargetThreat);
            DebugThreatStringSimple(self, scriptTargetEnt, v5, colorYellow);
            if ( pScariestEnemy )
                DebugThreatStringSimple(self, pScariestEnemy, (char *)"enemy", colorGreen);
        }
        else
        {
            pScariestEnemy = scriptTargetEnt;
            v4 = va("scripted enemy (%0.3f)", self->sentient->entityTargetThreat);
            DebugThreatStringSimple(self, scriptTargetEnt, v4, colorGreen);
        }
    }
    else if ( pScariestEnemy )
    {
        DebugThreatStringSimple(self, pScariestEnemy, (char *)"enemy", colorGreen);
    }
    Sentient_SetEnemy(self->sentient, pScariestEnemy, 1);
}

void __cdecl DebugThreatStringSimple(const actor_s *self, gentity_s *enemy, char *string, const float *color)
{
    float displayPos[3]; // [esp+10h] [ebp-18h] BYREF
    float start[3]; // [esp+1Ch] [ebp-Ch] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 422, 0, "%s", "self") )
        __debugbreak();
    if ( ai_debugThreatSelection->current.enabled && ai_debugEntIndex->current.integer == self->ent->s.number )
    {
        if ( !enemy && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 426, 0, "%s", "enemy") )
            __debugbreak();
        if ( !string
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 427, 0, "%s", "string") )
        {
            __debugbreak();
        }
        Sentient_GetDebugEyePosition(self->ent->sentient, start);
        if ( enemy->sentient )
        {
            Sentient_GetDebugEyePosition(enemy->sentient, displayPos);
        }
        else
        {
            displayPos[0] = enemy->r.currentOrigin[0];
            displayPos[1] = enemy->r.currentOrigin[1];
            displayPos[2] = enemy->r.currentOrigin[2];
        }
        CG_DebugLine(start, displayPos, color, 0, ai_threatUpdateInterval->current.integer / 50);
        displayPos[2] = displayPos[2] + 16.0;
        G_AddDebugString(displayPos, color, 1.0, string, ai_threatUpdateInterval->current.integer / 50);
    }
}

int __fastcall Actor_UpdateSingleThreat(actor_s *self, sentient_s *enemy)
{
    const char *v3; // eax
    bool v4; // [esp+Ch] [ebp-48h]
    bool isPlayer; // [esp+28h] [ebp-2Ch]
    float scarinessa; // [esp+2Ch] [ebp-28h]
    float scariness; // [esp+2Ch] [ebp-28h]
    int isVisible; // [esp+34h] [ebp-20h]
    bool friendlyTimingOut; // [esp+38h] [ebp-1Ch]
    sentient_info_t *pInfo; // [esp+3Ch] [ebp-18h]
    float enemyDist; // [esp+40h] [ebp-14h]
    int threatBiasScore; // [esp+44h] [ebp-10h]
    bool isCurrentEnemy; // [esp+48h] [ebp-Ch]
    int threata; // [esp+4Ch] [ebp-8h]
    int threatb; // [esp+4Ch] [ebp-8h]
    int threatc; // [esp+4Ch] [ebp-8h]
    int threatd; // [esp+4Ch] [ebp-8h]
    int threate; // [esp+4Ch] [ebp-8h]
    int threat; // [esp+4Ch] [ebp-8h]
    int threatf; // [esp+4Ch] [ebp-8h]
    bool isDamaged; // [esp+50h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 853, 0, "%s", "self") )
        __debugbreak();
    if ( !enemy && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 854, 0, "%s", "enemy") )
        __debugbreak();
    friendlyTimingOut = 0;
    pInfo = &self->sentientInfo[enemy - level.sentients];
    if ( !self->bPacifist
        || self->sentientInfo[enemy - level.sentients].iLastAttackMeTime
        && level.time - self->sentientInfo[enemy - level.sentients].iLastAttackMeTime < self->iPacifistWait )
    {
        threatBiasScore = Actor_GetThreatBias(enemy->iThreatBiasGroupIndex, self->sentient->iThreatBiasGroupIndex);
        if ( threatBiasScore == 0x80000000 )
        {
            return 0x80000000;
        }
        else
        {
            enemyDist = Vec3Distance(self->ent->r.currentOrigin, pInfo->vLastKnownPos);
            isCurrentEnemy = enemy == Actor_GetTargetSentient(self);
            if ( Actor_ShouldIgnoreFromAttackerCount(self, enemy, isCurrentEnemy) )
            {
                return 0x80000000;
            }
            else if ( Actor_CanBePathedTo(self, enemy, isCurrentEnemy) )
            {
                isVisible = pInfo->VisCache.bVisible;
                v4 = pInfo->VisCache.bVisible || Actor_IsFullyAware(self, enemy, isCurrentEnemy);
                isPlayer = enemy->ent->client != 0;
                isDamaged = (double)enemy->ent->maxHealth * 0.8 > (double)enemy->ent->health;
                if ( !v4 && isCurrentEnemy )
                    friendlyTimingOut = level.time - pInfo->lastKnownPosTime < 10000;
                if ( v4 || friendlyTimingOut )
                {
                    scarinessa = Sentient_GetScarinessForDistance(self->sentient, enemy, enemyDist);
                    scariness = scarinessa - Sentient_GetScarinessForDistance(enemy, self->sentient, enemyDist);
                }
                else
                {
                    scariness = 0.0f;
                }
                DebugResetThreatStrings(self);
                DebugSetThreatString(TDS_THREATBIAS_GROUP, threatBiasScore);
                DebugSetThreatString(TDS_THREATBIAS, enemy->iThreatBias);
                threata = enemy->iThreatBias + threatBiasScore;
                threatb = threata + Actor_ThreatFromVisibilityAndAwareness(isVisible, v4, friendlyTimingOut);
                threatc = threatb + Actor_ThreatFromScariness(scariness);
                threatd = threatc + Actor_ThreatFromDistance(enemyDist);
                threate = threatd + Actor_ThreatFromAttackerCount(self, enemy, isCurrentEnemy);
                threat = threate + Actor_ThreatBonusForCurrentEnemy(isCurrentEnemy, v4, friendlyTimingOut, isPlayer, isDamaged);
                if ( enemyDist > 256.0 )
                    threat += Actor_ThreatCoveringFire(self, enemy);
                threatf = threat + Actor_ThreatFlashed(enemy);
                v3 = va("%d (%0.3f)", threatf, (float)((float)threatf / 7000.0));
                DebugSetThreatStringFromString(TDS_TOTAL_THREAT, v3);
                DebugThreatStringAll(self, enemy, threatf);
                return threatf;
            }
            else
            {
                return 0x80000000;
            }
        }
    }
    else
    {
        DebugThreatStringSimple(self, enemy->ent, (char *)"pacifist", colorRed);
        return 0x80000000;
    }
}



void __cdecl DebugResetThreatStrings(const actor_s *self)
{
    bool v1; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 341, 0, "%s", "self") )
        __debugbreak();
    v1 = !ai_debugThreatSelection->current.enabled || ai_debugEntIndex->current.integer != self->ent->s.number;
    g_skipDebugString = v1;
    if ( !v1 )
    {
        for ( i = 0; i < 10; ++i )
            g_threatDebugStrings[i][0] = 0;
    }
}

void __cdecl DebugSetThreatString(ThreatDebugStringCategory category, int threat)
{
    if ( !g_skipDebugString )
    {
        if ( threat )
            sprintf(g_threatDebugStrings[category], "%s %d", g_threatDebugLabels[category], threat);
    }
}

void __cdecl DebugSetThreatStringFromString(ThreatDebugStringCategory category, const char *string)
{
    if ( !g_skipDebugString )
    {
        if ( string )
            sprintf(g_threatDebugStrings[category], "%s %s", g_threatDebugLabels[category], string);
    }
}

void __cdecl DebugThreatStringAll(const actor_s *self, sentient_s *enemy, int threat)
{
    float v3; // [esp+Ch] [ebp-38h]
    float v4; // [esp+10h] [ebp-34h]
    int i; // [esp+14h] [ebp-30h]
    float displayPos[3]; // [esp+18h] [ebp-2Ch] BYREF
    float fraction; // [esp+24h] [ebp-20h]
    float start[3]; // [esp+28h] [ebp-1Ch] BYREF
    float color[4]; // [esp+34h] [ebp-10h] BYREF

    iassert(self);

    if ( ai_debugThreatSelection->current.enabled && ai_debugEntIndex->current.integer == self->ent->s.number )
    {
        iassert(enemy);
        fraction = (float)threat / 7000.0;
        if ( (float)(fraction - 1.0) < 0.0 )
            v4 = fraction;
        else
            v4 = 1.0f;

        if ( (float)(0.0 - fraction) < 0.0 )
            v3 = v4;
        else
            v3 = 0.0f;
        fraction = v3;

        color[0] = (float)(v3 * 0.5) + 0.5;
        color[1] = color[0];
        color[2] = 0.0f;
        color[3] = 1.0f;

        Sentient_GetDebugEyePosition(self->ent->sentient, start);
        Sentient_GetDebugEyePosition(enemy, displayPos);

        CG_DebugLine(start, displayPos, color, 0, ai_threatUpdateInterval->current.integer / 50);

        displayPos[2] = displayPos[2] + 32.0;

        for ( i = 0; i < ARRAYSIZE(g_threatDebugStrings); ++i )
        {
            char *str = g_threatDebugStrings[i];

            if ( str[0] )
                G_AddDebugString(displayPos, color, 0.5, str, ai_threatUpdateInterval->current.integer / 50);

            displayPos[2] += 8.0f;
        }
    }
}

int __fastcall Actor_ThreatFromScariness(float fScariness)
{
    int v4; // [esp+0h] [ebp-Ch]
    int v5; // [esp+4h] [ebp-8h]

    if ( (int)(float)(-100.0 * fScariness) < 1000 )
        v5 = (int)(float)(-100.0 * fScariness);
    else
        v5 = 1000;
    if ( v5 > -500 )
        v4 = v5;
    else
        v4 = -500;
    DebugSetThreatString(TDS_SCARINESS_THREAT, v4);
    return v4;
}

int __fastcall Actor_ThreatFromDistance(float fDistance)
{
    int threat; // [esp+1Ch] [ebp-4h]

    iassert(fDistance >= 0);

    if ( fDistance >= 2500.0 )
        return 0;

    threat = (int)((float)((float)(AI_THREAT_DISTANCE_RATE * (float)(2500.0 - fDistance)) * (float)(2500.0 - fDistance)) + 9.313225746154785e-10);
    DebugSetThreatStringFromString(TDS_DIST_THREAT, va("%d (%0.1f)", threat, fDistance));
    return threat;
}

bool __fastcall Actor_CanBePathedTo(actor_s *self, sentient_s *enemy, int isCurrentEnemy)
{
    float current_dist; // [esp+14h] [ebp-10h]
    pathnode_t *enemyNode; // [esp+18h] [ebp-Ch]

    enemyNode = Sentient_NearestNode(enemy);
    if ( !enemyNode )
        return 0;
    current_dist = Vec3DistanceSq(enemyNode->constant.vOrigin, enemy->ent->r.currentOrigin);
    if ( (float)(200.0 * 200.0) > current_dist )
    {
        self->lastPathToEnemyTime = level.time;
        return 1;
    }
    return isCurrentEnemy && ai_noPathToEnemyGiveupTime->current.integer > level.time - self->lastPathToEnemyTime;
}

bool __fastcall Actor_IsFullyAware(actor_s *self, sentient_s *enemy, int isCurrentEnemy)
{
    pathnode_t *enemyNode; // [esp+14h] [ebp-10h]
    pathnode_t *myNode; // [esp+18h] [ebp-Ch]
    int isFullyAware; // [esp+1Ch] [ebp-8h]

    isFullyAware = 0;
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 582, 0, "%s", "self") )
        __debugbreak();
    if ( !enemy && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 583, 0, "%s", "enemy") )
        __debugbreak();
    if ( !isCurrentEnemy )
        return level.time - self->sentientInfo[enemy - level.sentients].lastKnownPosTime < 10000;
    if ( Vec3DistanceSq(self->sentientInfo[enemy - level.sentients].vLastKnownPos, enemy->ent->r.currentOrigin) < 4096.0 )
    {
        if ( self->sentient->pClaimedNode )
            myNode = self->sentient->pClaimedNode;
        else
            myNode = Sentient_NearestNode(self->sentient);
        enemyNode = Sentient_NearestNode(enemy);
        if ( myNode && enemyNode && Path_NodesVisible(myNode, enemyNode) )
        {
            DebugThreatNodes(self, enemy, myNode, enemyNode, colorBlue);
            return 1;
        }
    }
    return isFullyAware;
}

void __cdecl DebugThreatNodes(
                const actor_s *self,
                sentient_s *enemy,
                pathnode_t *selfNode,
                pathnode_t *enemyNode,
                const float *color)
{
    float viewPos[3]; // [esp+8h] [ebp-24h] BYREF
    float start[3]; // [esp+14h] [ebp-18h] BYREF
    float end[3]; // [esp+20h] [ebp-Ch] BYREF

    if ( ai_debugThreatSelection->current.enabled && ai_debugEntIndex->current.integer == self->ent->s.number )
    {
        CL_GetDebugViewPos(viewPos);
        if ( selfNode )
        {
            Path_DrawDebugNode(viewPos, selfNode);
            start[0] = selfNode->constant.vOrigin[0];
            start[1] = selfNode->constant.vOrigin[1];
            start[2] = selfNode->constant.vOrigin[2];
        }
        else
        {
            Sentient_GetDebugEyePosition(self->ent->sentient, start);
        }
        if ( enemyNode )
        {
            Path_DrawDebugNode(viewPos, enemyNode);
            end[0] = enemyNode->constant.vOrigin[0];
            end[1] = enemyNode->constant.vOrigin[1];
            end[2] = enemyNode->constant.vOrigin[2];
        }
        else
        {
            Sentient_GetDebugEyePosition(enemy->ent->sentient, end);
        }
        start[2] = start[2] + 16.0;
        end[2] = end[2] + 16.0;
        CG_DebugLine(start, end, color, 1, ai_threatUpdateInterval->current.integer / 50);
    }
}

int __fastcall Actor_ThreatFromVisibilityAndAwareness(int isVisible, int isFullyAware, int friendlyTimingOut)
{
    const char *v3; // eax
    int threat; // [esp+8h] [ebp-4h]

    threat = 0;
    if ( isVisible )
    {
        threat = 1000;
    }
    else if ( isFullyAware )
    {
        threat = 500;
    }
    else if ( friendlyTimingOut )
    {
        threat = 250;
    }
    if ( isVisible )
    {
        v3 = va("%d (visible)", threat);
        DebugSetThreatStringFromString(TDS_AWARENESS, v3);
    }
    else
    {
        DebugSetThreatString(TDS_AWARENESS, threat);
    }
    return threat;
}

int __fastcall Actor_ThreatFromAttackerCount(actor_s *self, sentient_s *enemy, int isCurrentEnemy)
{
    int v4; // [esp+0h] [ebp-1Ch]
    int attackerCount; // [esp+4h] [ebp-18h]
    int threat; // [esp+18h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 654, 0, "%s", "self") )
        __debugbreak();
    if ( !enemy && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 655, 0, "%s", "enemy") )
        __debugbreak();
    if ( isCurrentEnemy )
        attackerCount = enemy->attackerCount - 1;
    else
        attackerCount = enemy->attackerCount;
    if ( -150 * attackerCount < -1000 )
        v4 = -1000;
    else
        v4 = -150 * attackerCount;
    threat = v4;
    //if ( EntHandle::isDefined(&enemy->syncedMeleeEnt) && self->ent != EntHandle::ent(&enemy->syncedMeleeEnt) )
    if ( enemy->syncedMeleeEnt.isDefined() && self->ent != enemy->syncedMeleeEnt.ent())
        threat = v4 - 10000;
    if ( Flame_GetLocalClientSourceRange() && !isCurrentEnemy && attackerCount >= ai_maxAttackerCount->current.integer )
        threat -= 10000;
    DebugSetThreatString(TDS_ATTACKER_COUNT, threat);
    return threat;
}

bool __fastcall Actor_ShouldIgnoreFromAttackerCount(actor_s *self, sentient_s *enemy, int isCurrentEnemy)
{
    int attackerCount; // [esp+0h] [ebp-10h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 679, 0, "%s", "self") )
        __debugbreak();
    if ( !enemy && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 680, 0, "%s", "enemy") )
        __debugbreak();
    if ( isCurrentEnemy )
        attackerCount = enemy->attackerCount - 1;
    else
        attackerCount = enemy->attackerCount;
    return !isCurrentEnemy && attackerCount >= ai_maxAttackerCount->current.integer;
}

int __fastcall Actor_ThreatBonusForCurrentEnemy(
                int isCurrentEnemy,
                int isFullyAware,
                int friendlyTimingOut,
                int isPlayer,
                int isDamaged)
{
    int threat; // [esp+8h] [ebp-4h]

    threat = 0;
    if ( !isCurrentEnemy )
        goto LABEL_10;
    if ( !isFullyAware )
    {
        if ( friendlyTimingOut )
        {
            threat = 200;
            DebugSetThreatString(TDS_CUR_ENEMY_BONUS, 200);
            return threat;
        }
        threat = 250;
LABEL_10:
        DebugSetThreatString(TDS_CUR_ENEMY_BONUS, threat);
        return threat;
    }
    if ( isPlayer && isDamaged )
    {
        threat = 1000;
        DebugSetThreatString(TDS_CUR_ENEMY_BONUS, 1000);
    }
    else
    {
        threat = 500;
        DebugSetThreatString(TDS_CUR_ENEMY_BONUS, 500);
    }
    return threat;
}

int __fastcall Actor_ThreatCoveringFire(actor_s *self, sentient_s *enemy)
{
    pathnode_t *node; // [esp+1Ch] [ebp-8h]
    actor_s *actor; // [esp+20h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 726, 0, "%s", "self") )
        __debugbreak();
    if ( !enemy && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 727, 0, "%s", "enemy") )
        __debugbreak();
    if ( !self->provideCoveringFire || Actor_IsMoving(self) )
        return 0;
    actor = enemy->ent->actor;
    if ( !actor || actor->ignoreSuppression || Actor_InPain(actor) || Actor_IsDying(actor) )
        goto LABEL_14;
    if ( actor->suppressionMeter <= 0.25 )
        return 0;
    node = enemy->pClaimedNode;
    if ( node
        && Actor_IsNearClaimedNode(actor)
        && (float)((float)((float)(self->ent->r.currentOrigin[0] - node->constant.vOrigin[0]) * node->constant.forward[0])
                         + (float)((float)(self->ent->r.currentOrigin[1] - node->constant.vOrigin[1]) * node->constant.forward[1])) < 0.0 )
    {
        return 0;
    }
    if ( !Actor_IsMoving(actor) )
    {
LABEL_14:
        DebugSetThreatString(TDS_SUPPRESSED, -3000);
        return -3000;
    }
    return 0;
}

int __fastcall Actor_ThreatFlashed(sentient_s *enemy)
{
    int flashed; // [esp+4h] [ebp-8h]
    actor_s *actor; // [esp+8h] [ebp-4h]

    if ( !enemy && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 778, 0, "%s", "enemy") )
        __debugbreak();
    flashed = 0;
    actor = enemy->ent->actor;
    if ( actor )
        flashed = actor->flashBanged;
    if ( !flashed )
        return 0;
    DebugSetThreatString(TDS_FLASHED, 200);
    return 200;
}

void __fastcall Actor_IncrementThreatTime(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp", 975, 0, "%s", "self") )
        __debugbreak();
    if ( !ai_threatUpdateInterval->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp",
                    976,
                    0,
                    "%s",
                    "ai_threatUpdateInterval->current.integer") )
    {
        __debugbreak();
    }
    self->threatUpdateTime += ai_threatUpdateInterval->current.integer;
    if ( self->threatUpdateTime <= level.time )
    {
        self->threatUpdateTime %= ai_threatUpdateInterval->current.integer;
        self->threatUpdateTime += ai_threatUpdateInterval->current.integer
                                                        * (level.time / ai_threatUpdateInterval->current.integer + 1);
    }
    if ( level.time >= self->threatUpdateTime
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_threat.cpp",
                    986,
                    0,
                    "%s",
                    "level.time < self->threatUpdateTime") )
    {
        __debugbreak();
    }
}

void __fastcall Actor_SetPotentialThreat(potential_threat_t *self, float yaw)
{
    self->isEnabled = 1;

    self->direction[0] = cos(DEG2RAD(yaw));
    self->direction[1] = sin(DEG2RAD(yaw));
}

void __fastcall Actor_ClearPotentialThreat(potential_threat_t *self)
{
    self->isEnabled = 0;
}

void __cdecl Actor_PotentialThreat_Debug(actor_s *self)
{
    float xyz[3]; // [esp+Ch] [ebp-18h] BYREF
    float largePotentialThreatVector[3]; // [esp+18h] [ebp-Ch] BYREF

    Sentient_GetDebugEyePosition(self->ent->sentient, xyz);
    if ( self->potentialThreat.isEnabled )
    {
        largePotentialThreatVector[0] = (float)(32.0 * self->potentialThreat.direction[0]) + xyz[0];
        largePotentialThreatVector[1] = (float)(32.0 * self->potentialThreat.direction[1]) + xyz[1];
        largePotentialThreatVector[2] = xyz[2];
        G_DebugLine(xyz, largePotentialThreatVector, colorRed, 0);
    }
    else
    {
        G_AddDebugString(xyz, colorWhite, 1.0, (char*)"No Threat", 1);
    }
}

