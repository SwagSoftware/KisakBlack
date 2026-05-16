#include "sv_bot_mp.h"
#include <server/server.h>
#include <game/actor_navigation.h>
#include "sv_main_mp.h"
#include <game_mp/g_main_mp.h>
#include <qcommon/msg_mp.h>
#include <server/sv_world.h>
#include <universal/com_math_anglevectors.h>
#include <bgame/bg_weapons_ammo.h>
#include <bgame/bg_misc.h>
#include <game_mp/g_active_mp.h>
#include <game_mp/g_spawn_mp.h>
#include <clientscript/scr_const.h>
#include <clientscript/cscr_stringlist.h>
#include <bgame/bg_pmove.h>
#include <cgame/cg_drawtools.h>
#include <live/live_friends_pc.h>
#include <client_mp/sv_client_mp.h>
#include <client_mp/g_client_mp.h>
#include <game/g_debug.h>
#include <game/g_bsp.h>

const char *botNames[47] =
{
  "DVonderhaar",
  "DBunting",
  "AConserva",
  "MScronce",
  "RFeltrin",
  "SJCrowe",
  "BNandakumar",
  "RMoffat",
  "AHoggatt",
  "CCowell",
  "SNouriani",
  "SRoud",
  "LAJohansen",
  "PBabar",
  "JMattis",
  "PTasker",
  "AEady",
  "IJKowalski",
  "CCSchneider",
  "GIDoron",
  "TJSchneider",
  "VKSharma",
  "MCG Gordon",
  "MLLeslie",
  "MLAM1A",
  "DLH Harper",
  "DMRoche",
  "DAA Anthony",
  "JBojorquez",
  "MDonlon",
  "AKrauss",
  "GJNg",
  "EFRich",
  "TEWells",
  "TJKeegan",
  "ABhura",
  "BLMercado",
  "JMorelli",
  "KTouevsky",
  "EOughton",
  "LZide",
  "JShubert",
  "KTakacs",
  "CStastny",
  "ARomo",
  "RFord",
  "Larry"
};

const dvar_t *sv_botsAllowMovement;
const dvar_t *sv_botsForceStand;
const dvar_t *sv_botsForceCrouch;
const dvar_t *sv_botsForceProne;
const dvar_t *sv_botsPressAttackBtn;
const dvar_t *sv_botsPressMeleeBtn;
const dvar_t *sv_botsForceFragOnly;
const dvar_t *sv_botsForceSpecialOnly;
const dvar_t *sv_botsIgnoreHumans;
const dvar_t *sv_botSnapshotDebug;
const dvar_t *sv_botForceYaw;
const dvar_t *sv_botDebug;
const dvar_t *sv_botDebugX;
const dvar_t *sv_botDebugY;
const dvar_t *sv_botDebugThreat;
const dvar_t *sv_botDebugPaths;
const dvar_t *sv_botFov;
const dvar_t *sv_botGoalRadius;
const dvar_t *sv_botCloseDistance;
const dvar_t *sv_botSprintDistance;
const dvar_t *sv_botCrouchDistance;
const dvar_t *sv_botTargetLeadBias;
const dvar_t *sv_botUseFriendNames;
const dvar_t *sv_botAllowGrenades;
const dvar_t *sv_botMinGrenadeTime;
const dvar_t *sv_botMaxGrenadeTime;
const dvar_t *sv_botMinAdsTime;
const dvar_t *sv_botMaxAdsTime;
const dvar_t *sv_botMinPitchTime;
const dvar_t *sv_botMaxPitchTime;
const dvar_t *sv_botMinCrouchTime;
const dvar_t *sv_botMaxCrouchTime;
const dvar_t *sv_botMinDeathTime;
const dvar_t *sv_botMaxDeathTime;
const dvar_t *sv_botMinFireTime;
const dvar_t *sv_botMaxFireTime;
const dvar_t *sv_botMinReactionTime;
const dvar_t *sv_botMaxReactionTime;
const dvar_t *sv_botYawSpeed;
const dvar_t *sv_botYawSpeedAds;
const dvar_t *sv_botPitchUp;
const dvar_t *sv_botPitchDown;
const dvar_t *sv_botPitchSpeed;
const dvar_t *sv_botPitchSpeedAds;
const dvar_t *sv_botStrafeChance;
const dvar_t *sv_botMinStrafeTime;
const dvar_t *sv_botMaxStrafeTime;

bot_info_t botInfos[32];

float g_botFovCos;

float botFov = -1.0f;
const char right_0[4] =
{ '\x81', '\xC0', '@', '\x7F' };

char rightAds[4] =
{ '\x81', '\x7F', '\0', '\0' };

char forward_0[8] =
{ '\xC0', '@', '\0', '\0', '\0', '\0', '\0', '\0' };

void __cdecl SV_BotSwitchWeapon(const client_t *bot, int weapIndex)
{
    if ( !bot->bIsTestClient
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_bot_mp.cpp",
                    2776,
                    0,
                    "%s",
                    "bot->bIsTestClient") )
    {
        __debugbreak();
    }
    botInfos[bot->gentity->s.number].weapon = weapIndex;
}

void __cdecl SV_BotSetScriptGoal(const client_t *bot, const float *goal, float goalRadius)
{
    bot_info_t *botInfo; // [esp+4h] [ebp-4h]

    if ( !bot->bIsTestClient
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_bot_mp.cpp",
                    2784,
                    0,
                    "%s",
                    "bot->bIsTestClient") )
    {
        __debugbreak();
    }
    botInfo = &botInfos[bot->gentity->s.number];
    botInfo->scriptGoal[0] = *goal;
    botInfo->scriptGoal[1] = goal[1];
    botInfo->scriptGoal[2] = goal[2];
    botInfo->scriptGoalRadius = goalRadius;
    botInfo->flags |= 4u;
}

void __cdecl SV_BotClearScriptGoal(const client_t *bot)
{
    bot_info_t *v1; // eax

    if ( !bot->bIsTestClient
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_bot_mp.cpp",
                    2795,
                    0,
                    "%s",
                    "bot->bIsTestClient") )
    {
        __debugbreak();
    }
    v1 = &botInfos[bot->gentity->s.number];
    v1->flags &= ~4u;
    Path_Clear(&v1->path);
}

void __cdecl SV_BotSetScriptEnemy(const client_t *bot, const gentity_s *enemy)
{
    bot_info_t *botInfo; // [esp+0h] [ebp-4h]

    if ( !bot->bIsTestClient
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_bot_mp.cpp",
                    2804,
                    0,
                    "%s",
                    "bot->bIsTestClient") )
    {
        __debugbreak();
    }
    botInfo = &botInfos[bot->gentity->s.number];
    if ( enemy && enemy->r.inuse && enemy->health > 0 )
    {
        botInfo->threat.enemy = enemy;
        botInfo->threat.acquireTime = svs.time;
        botInfo->flags |= 8u;
    }
}

void __cdecl SV_BotClearScriptEnemy(const client_t *bot)
{
    bot_info_t *botInfo; // [esp+0h] [ebp-4h]

    if ( !bot->bIsTestClient
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_bot_mp.cpp",
                    2830,
                    0,
                    "%s",
                    "bot->bIsTestClient") )
    {
        __debugbreak();
    }
    botInfo = &botInfos[bot->gentity->s.number];
    botInfo->flags &= ~8u;
    Bot_ClearThreat(botInfo);
    Path_Clear(&botInfo->path);
    botInfo->sightHitNum = -1;
}

void __cdecl Bot_ClearThreat(bot_info_t *botInfo)
{
    memset((unsigned __int8 *)&botInfo->threat, 0, sizeof(botInfo->threat));
    botInfo->attackerEnt = 0;
    botInfo->flags &= ~1u;
    botInfo->flags &= ~0x40u;
    botInfo->flags &= ~0x20u;
}

void __cdecl SV_BotPressUseButton(const client_t *bot, int msec)
{
    iassert(bot->bIsTestClient);
    botInfos[bot->gentity->s.number].useButtonEndTime = msec + svs.time;
}

void __cdecl SV_BotPressAttackButton(const client_t* bot)
{
    iassert(bot->bIsTestClient);
    botInfos[bot->gentity->s.number].flags |= 0x1000u;
}

void __cdecl SV_BotSetAttacker(const client_t *bot, const gentity_s *attacker)
{
    bot_info_t *botInfo; // [esp+4h] [ebp-4h]

    botInfo = &botInfos[bot->gentity->s.number];
    if ( attacker && attacker->s.number != 1023 && attacker->s.number != 1022 && attacker->r.inuse )
        botInfo->attackerEnt = attacker;
    else
        botInfo->attackerEnt = 0;
}

void __cdecl SV_BotThink(client_t *bot, usercmd_s *cmd)
{
    int j; // [esp+10h] [ebp-54h]
    int i; // [esp+4Ch] [ebp-18h]
    char *Name; // [esp+50h] [ebp-14h]
    bot_info_t *botInfo; // [esp+58h] [ebp-Ch]
    unsigned int clientNum; // [esp+5Ch] [ebp-8h]

    Name = va("SV_BotThink '%s'", bot->name);
    PROF_SCOPED_RUNTIME_NAME(Name);

    if ( !bot->bIsTestClient
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_bot_mp.cpp",
                    2865,
                    0,
                    "%s",
                    "bot->bIsTestClient") )
    {
        __debugbreak();
    }
    clientNum = bot - svs.clients;
    if ( clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_bot_mp.cpp",
                    2868,
                    0,
                    "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    clientNum,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    botInfo = &botInfos[clientNum];
    memcpy(cmd, &bot->lastUsercmd, sizeof(usercmd_s));
    for ( i = 0; i < 2; ++i )
        cmd->button_bits.array[i] = 0;
    if ( !bot->gentity )
    {
        return;
    }
    if ( G_GetClientArchiveTime(clientNum) )
    {
        return;
    }
    if ( bot->lastUsercmd.serverTime <= 0 )
    {
        Bot_Clear(botInfo);
        if ( Com_HashString(bot->name, 0) == 812562687 )
            bot->gentity->client->sess.cs.score.kills = 1;
    }
    if ( bot->gentity->health <= 0 )
    {
        Bot_Clear(botInfo);
        return;
    }
    if ( fabs(botFov - sv_botFov->current.value) > 0.000001 )
    {
        botFov = sv_botFov->current.value;
        g_botFovCos = cos((float)((float)(botFov * 0.5) * 0.017453292));
    }
    cmd->serverTime = svs.time;
    Bot_UpdateTimedAction(&botInfo->adsEndTime);
    Bot_UpdateTimedAction(&botInfo->pitchEndTime);
    Bot_UpdateTimedAction(&botInfo->crouchEndTime);
    Bot_UpdateTimedAction(&botInfo->useButtonEndTime);
    if ( (bot->gentity->client->flags & 4) != 0 )
    {
        cmd->forwardmove = 0;
        cmd->rightmove = 0;
        if ( botInfo->useButtonEndTime )
        {
            cmd->button_bits.setBit(9);
            cmd->button_bits.setBit(5);
        }
        bot->header.deltaMessage = bot->header.netchan.outgoingSequence - 1;
        return;
    }
    else
    {
        Bot_UpdateWeapon(botInfo, bot, cmd);
        Bot_UpdateThreat(botInfo, bot);
        Bot_UpdateSight(botInfo, bot);
        Bot_UpdateMovement(botInfo, bot, cmd);
        Bot_UpdateCombat(botInfo, bot, cmd);
        Bot_UpdateReload(botInfo, bot, cmd);
        Bot_UpdateGlassSmash(bot, cmd);
        if ( botInfo->useButtonEndTime )
        {
            for ( j = 0; j < 2; ++j )
                cmd->button_bits.array[j] = 0;
            cmd->button_bits.setBit(9);
            cmd->button_bits.setBit(5);
        }
        if ( sv_botsForceStand->current.enabled )
        {
            cmd->button_bits.resetBit(9);
            cmd->button_bits.resetBit(8);
            cmd->button_bits.resetBit(0xC);
        }
        if ( sv_botsForceCrouch->current.enabled )
        {
            cmd->button_bits.setBit(9);
            cmd->button_bits.resetBit(8);
            cmd->button_bits.resetBit(1);
            cmd->button_bits.resetBit(0x2C);
            cmd->button_bits.resetBit(0xC);
        }
        if ( sv_botsForceProne->current.enabled )
        {
            cmd->button_bits.setBit(8);
            cmd->button_bits.resetBit(9);
            cmd->button_bits.resetBit(1);
            cmd->button_bits.resetBit(0x2C);
            cmd->button_bits.resetBit(0xC);
        }
        bot->header.deltaMessage = bot->header.netchan.outgoingSequence - 1;
    }
}

int __cdecl Com_HashString(const char *fname, int len)
{
    int hash; // [esp+0h] [ebp-Ch]
    int i; // [esp+8h] [ebp-4h]

    if ( !fname )
        return 0;
    hash = 5381;
    for ( i = 0; fname[i] && (!len || i < len); ++i )
        hash = tolower(fname[i]) + 33 * hash;
    return hash;
}

void __cdecl Bot_UpdateTimedAction(int *timedAction)
{
    if ( svs.time >= *timedAction )
        *timedAction = 0;
}

void __cdecl Bot_UpdateSight(bot_info_t *botInfo, const client_t *bot)
{
    const float *v4; // [esp+24h] [ebp-7Ch]
    const float *v5; // [esp+30h] [ebp-70h]
    const gentity_s *v6; // [esp+34h] [ebp-6Ch]
    const gentity_s *enemy; // [esp+38h] [ebp-68h]
    const float *v8; // [esp+3Ch] [ebp-64h]
    float *currentOrigin; // [esp+40h] [ebp-60h]
    col_context_t context; // [esp+5Ch] [ebp-44h] BYREF
    bool hadSight; // [esp+87h] [ebp-19h]
    float enemyOrigin[3]; // [esp+88h] [ebp-18h] BYREF
    float botOrigin[3]; // [esp+94h] [ebp-Ch] BYREF

    PROF_SCOPED("Bot_UpdateSight");

    //col_context_t::col_context_t(&context);
    if ( botInfo->threat.enemy )
    {
        if ( botInfo->threat.distSq >= 3025.0 )
        {
            if ( g_botFovCos > botInfo->threat.dot )
            {
                botInfo->sightHitNum = -1;
                return;
            }
        }
        else if ( botInfo->threat.dot < 0.69999999 )
        {
            botInfo->sightHitNum = -1;
            return;
        }
        currentOrigin = bot->gentity->r.currentOrigin;
        botOrigin[0] = *currentOrigin;
        botOrigin[1] = currentOrigin[1];
        botOrigin[2] = currentOrigin[2];
        botOrigin[2] = botOrigin[2] + bot->gentity->client->ps.viewHeightCurrent;
        v8 = botInfo->threat.enemy->r.currentOrigin;
        enemyOrigin[0] = *v8;
        enemyOrigin[1] = v8[1];
        enemyOrigin[2] = v8[2];
        enemy = botInfo->threat.enemy;
        if ( enemy && enemy->client )
        {
            enemyOrigin[2] = enemyOrigin[2] + botInfo->threat.enemy->client->ps.viewHeightCurrent;
            if ( (botInfo->threat.enemy->client->ps.eFlags & 0x4000) == 0 )
                AddLeanToPosition(
                    enemyOrigin,
                    botInfo->threat.enemy->client->ps.viewangles[1],
                    botInfo->threat.enemy->client->ps.leanf,
                    16.0,
                    20.0);
        }
        else
        {
            enemyOrigin[2] = (float)(botInfo->threat.enemy->r.maxs[2] / 2.0) + enemyOrigin[2];
        }
        context.passEntityNum0 = bot->gentity->s.number;
        context.passEntityNum1 = botInfo->threat.enemy->s.number;
        context.mask = 0x2818001;
        v6 = botInfo->threat.enemy;
        if ( v6 && v6->scr_vehicle )
            context.mask &= ~0x10000u;
        hadSight = botInfo->sightHitNum == 0;
        SV_SightTracePoint(&botInfo->sightHitNum, botOrigin, enemyOrigin, &context);
        if ( hadSight || botInfo->sightHitNum )
            botInfo->flags &= ~0x20u;
        else
            botInfo->flags |= 0x20u;
        if ( botInfo->sightHitNum )
        {
            if ( botInfo->threat.lastSightTime )
            {
                if ( svs.time - botInfo->threat.lastSightTime < 1000 )
                {
                    v5 = botInfo->threat.enemy->r.currentOrigin;
                    botInfo->threat.lastOrigin[0] = *v5;
                    botInfo->threat.lastOrigin[1] = v5[1];
                    botInfo->threat.lastOrigin[2] = v5[2];
                }
                if ( svs.time - botInfo->threat.lastSightTime <= 100
                    || (float)((float)((float)(botInfo->threat.grenadeOrigin[0] * botInfo->threat.grenadeOrigin[0])
                                                     + (float)(botInfo->threat.grenadeOrigin[1] * botInfo->threat.grenadeOrigin[1]))
                                     + (float)(botInfo->threat.grenadeOrigin[2] * botInfo->threat.grenadeOrigin[2])) < 1.0e-12 )
                {
                    v4 = botInfo->threat.enemy->r.currentOrigin;
                    botInfo->threat.grenadeOrigin[0] = *v4;
                    botInfo->threat.grenadeOrigin[1] = v4[1];
                    botInfo->threat.grenadeOrigin[2] = v4[2];
                }
            }
        }
        else
        {
            botInfo->threat.lastSightTime = svs.time;
        }
        return;
    }
    botInfo->sightHitNum = -1;
}

void __cdecl Bot_UpdateThreat(bot_info_t *botInfo, const client_t *bot)
{
    bool v2; // [esp+4h] [ebp-768h]
    bool v3; // [esp+8h] [ebp-764h]
    const gentity_s *attackerEnt; // [esp+1Ch] [ebp-750h]
    const gentity_s *v5; // [esp+40h] [ebp-72Ch]
    team_t enemyTeam; // [esp+4Ch] [ebp-720h]
    bot_threat_t threats[32]; // [esp+54h] [ebp-718h] BYREF
    const client_t *enemy; // [esp+758h] [ebp-14h]
    int i; // [esp+75Ch] [ebp-10h]
    const playerState_s *ps; // [esp+760h] [ebp-Ch]
    bot_threat_t *threat; // [esp+764h] [ebp-8h]
    int botTeam; // [esp+768h] [ebp-4h]

    //PIXBeginNamedEvent(-1, "Bot_UpdateThreat");
    botTeam = bot->gentity->client->sess.cs.team;
    ps = (const playerState_s *)G_GetPlayerState(bot->gentity->s.number);
    if ( botTeam == 3 )
    {
        Bot_ClearThreat(botInfo);
        //if ( GetCurrentThreadId() == g_DXDeviceThread )
            //D3DPERF_EndEvent();
        return;
    }
    memset((unsigned __int8 *)threats, 0, sizeof(threats));
    if ( sv_botsIgnoreHumans->current.enabled && botInfo->threat.enemy && Bot_IsEnemyHuman(botInfo->threat.enemy) )
        Bot_ClearThreat(botInfo);
    if ( botInfo->threat.enemy )
    {
        v5 = botInfo->threat.enemy;
        v3 = v5 && v5->client;
        if ( v3 && (botInfo->threat.enemy->client->flags & 1) != 0 )
            Bot_ClearThreat(botInfo);
    }
    if ( botInfo->threat.enemy
        && (botInfo->sightHitNum && !botInfo->threat.lastSightTime && svs.time - botInfo->threat.acquireTime > 10000
         || botInfo->sightHitNum && botInfo->threat.lastSightTime && svs.time - botInfo->threat.lastSightTime > 10000) )
    {
        Bot_ClearThreat(botInfo);
    }
    if ( ps->damageCount && botInfo->threat.deathEndTime )
        botInfo->threat.deathEndTime = svs.time;
    if ( botInfo->threat.enemy && botInfo->threat.deathEndTime && svs.time >= botInfo->threat.deathEndTime )
    {
        Bot_ClearThreat(botInfo);
        Path_Clear(&botInfo->path);
    }
    if ( botInfo->threat.enemy && !botInfo->threat.deathEndTime && botInfo->threat.enemy->health <= 0 )
    {
        botInfo->threat.deathEndTime = svs.time
                                                                 + irand(sv_botMinDeathTime->current.integer, sv_botMaxDeathTime->current.integer);
        //if ( GetCurrentThreadId() != g_DXDeviceThread )
        //    return;
        goto LABEL_82;
    }
    if ( botInfo->threat.enemy && botInfo->threat.enemy->health > 0 )
    {
        Bot_IsFacingEnemy(bot, botInfo->threat.enemy, &botInfo->threat.dot);
        Bot_IsInRangeOfEnemy(bot, botInfo->threat.enemy, &botInfo->threat.distSq);
    }
    if ( (botInfo->flags & 8) != 0 )
    {
        //if ( g_DXDeviceThread != GetCurrentThreadId() )
        //    return;
        goto LABEL_82;
    }
    if ( botInfo->threat.enemy
        && botInfo->threat.enemy->health > 0
        && (float)(sv_botCloseDistance->current.value * sv_botCloseDistance->current.value) > botInfo->threat.distSq )
    {
        //if ( GetCurrentThreadId() != g_DXDeviceThread )
        //    return;
        goto LABEL_82;
    }
    if ( botInfo->threat.enemy && Bot_IsInLastStand(bot) )
    {
        //if ( g_DXDeviceThread != GetCurrentThreadId() )
        //    return;
        goto LABEL_82;
    }
    if ( botInfo->sightHitNum && ps->damageCount )
    {
        attackerEnt = botInfo->attackerEnt;
        v2 = attackerEnt && attackerEnt->s.number != 1023 && attackerEnt->s.number != 1022 && attackerEnt->r.inuse;
        if ( v2 && botInfo->threat.enemy != botInfo->attackerEnt )
        {
            botInfo->threat.enemy = botInfo->attackerEnt;
            Bot_IsFacingEnemy(bot, botInfo->threat.enemy, &botInfo->threat.dot);
            Bot_IsInRangeOfEnemy(bot, botInfo->threat.enemy, &botInfo->threat.distSq);
            botInfo->threat.acquireTime = svs.time;
            botInfo->flags |= 1u;
            //if ( g_DXDeviceThread != GetCurrentThreadId() )
            //    return;
            goto LABEL_82;
        }
        botInfo->attackerEnt = 0;
    }
    i = 0;
    enemy = svs.clients;
    while ( i < com_maxclients->current.integer )
    {
        if ( enemy != bot
            && enemy->header.state == CS_ACTIVE
            && enemy->gentity->health > 0
            && (enemy->gentity->client->flags & 1) == 0
            && (!sv_botsIgnoreHumans->current.enabled || enemy->bIsTestClient) )
        {
            enemyTeam = enemy->gentity->client->sess.cs.team;
            if ( enemyTeam != TEAM_SPECTATOR && (!botTeam || enemyTeam != botTeam) )
            {
                threat = &threats[enemy->gentity->s.number];
                Bot_IsFacingEnemy(bot, enemy->gentity, &threat->dot);
                if ( Bot_ThreatSightCheck(bot, enemy->gentity, threat->dot) )
                {
                    Bot_IsInRangeOfEnemy(bot, enemy->gentity, &threat->distSq);
                    threat->enemy = enemy->gentity;
                }
            }
        }
        ++i;
        ++enemy;
    }
    Bot_UpdateBestThreat(botInfo, threats, 32);
    //if ( GetCurrentThreadId() == g_DXDeviceThread )
LABEL_82:
    ;
        //D3DPERF_EndEvent();
}

bool __cdecl Bot_IsEnemyHuman(const gentity_s *enemy)
{
    return enemy && enemy->client && svs.clients[enemy->s.number].bIsTestClient == 0;
}

bool __cdecl Bot_IsInLastStand(const client_t *bot)
{
    gclient_s *ps; // [esp+4h] [ebp-4h]

    ps = G_GetPlayerState(bot->gentity->s.number);
    return ps->ps.pm_type == 6 || ps->ps.pm_type == 7 || ps->ps.pm_type == 8;
}

float __cdecl SV_BotGetLookaheadDist(const client_t *bot)
{
    iassert(bot->bIsTestClient);

    path_t* path = &botInfos[bot->gentity->s.number].path;
    if (Path_Exists(path))
        return path->fLookaheadDist;
    return 0.0f;
}

bool __cdecl SV_BotGetLookaheadDir(const client_t* bot, float* outDir)
{
    iassert(bot->bIsTestClient);

    outDir[0] = outDir[1] = outDir[2] = 0.0f;
    path_t* path = &botInfos[bot->gentity->s.number].path;
    if (!Path_Exists(path))
        return false;
    
    Vec3Copy(path->lookaheadDir, outDir);
    return true;
}

gentity_s* __cdecl SV_BotGetThreat(const client_t* bot)
{
    iassert(bot->bIsTestClient);

    gentity_s* pThreat = (gentity_s*)botInfos[bot->gentity->s.number].threat.enemy;
    if(!pThreat || pThreat->s.number == 1023 || pThreat->s.number == 1022 || !pThreat->r.inuse)
        return nullptr;
    return pThreat;
}

bool __cdecl SV_BotHasScriptGoal(const client_t* bot)
{
    iassert(bot->bIsTestClient);

    return (botInfos[bot->gentity->s.number].flags & 4) != 0;
}

bool __cdecl Bot_IsFacingEnemy(const client_t *bot, const gentity_s *enemy, float *dot)
{
    float *currentOrigin; // [esp+3Ch] [ebp-30h]
    bool v5; // [esp+4Ch] [ebp-20h]
    bool v6; // [esp+4Dh] [ebp-1Fh]
    float vForward[3]; // [esp+50h] [ebp-1Ch] BYREF
    float distSq; // [esp+5Ch] [ebp-10h]
    float vDelta[3]; // [esp+60h] [ebp-Ch] BYREF

    //PIXBeginNamedEvent(-1, "Bot_IsFacingEnemy");
    if ( enemy )
    {
        currentOrigin = bot->gentity->r.currentOrigin;
        vDelta[0] = enemy->r.currentOrigin[0] - *currentOrigin;
        vDelta[1] = enemy->r.currentOrigin[1] - currentOrigin[1];
        vDelta[2] = enemy->r.currentOrigin[2] - currentOrigin[2];
        Vec3Normalize(vDelta);
        AngleVectors(bot->gentity->client->ps.viewangles, vForward, 0, 0);
        Vec3Normalize(vForward);
        *dot = (float)((float)(vForward[0] * vDelta[0]) + (float)(vForward[1] * vDelta[1]))
                 + (float)(vForward[2] * vDelta[2]);
        distSq = Vec3DistanceSq(bot->gentity->r.currentOrigin, enemy->r.currentOrigin);
        if ( distSq >= 3025.0 )
        {
            v5 = *dot > g_botFovCos;
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                //D3DPERF_EndEvent();
            return v5;
        }
        else
        {
            v6 = *dot > 0.69999999;
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                //D3DPERF_EndEvent();
            return v6;
        }
    }
    else
    {
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
        return 0;
    }
}

char __cdecl Bot_IsInRangeOfEnemy(const client_t *bot, const gentity_s *enemy, float *distSq)
{
    bool v4; // [esp+28h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+2Ch] [ebp-8h]

    //PIXBeginNamedEvent(-1, "Bot_IsInRangeOfEnemy");
    if ( enemy )
    {
        *distSq = Vec3DistanceSq(bot->gentity->r.currentOrigin, enemy->r.currentOrigin);
        if ( bot->gentity->client->ps.weapon )
        {
            weapDef = BG_GetWeaponDef(bot->gentity->client->ps.weapon);
            v4 = (float)(weapDef->fMinDamageRange * weapDef->fMinDamageRange) > *distSq;
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                //D3DPERF_EndEvent();
            return v4;
        }
        else
        {
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                //D3DPERF_EndEvent();
            return 1;
        }
    }
    else
    {
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
        return 0;
    }
}

void __cdecl Bot_UpdateBestThreat(bot_info_t *botInfo, bot_threat_t *threats, int numThreats)
{
    bot_threat_t *bestThreat; // [esp+14h] [ebp-14h]
    bool veryClose; // [esp+19h] [ebp-Fh]
    bool betterAngle; // [esp+1Ah] [ebp-Eh]
    bool closer; // [esp+1Bh] [ebp-Dh]
    int i; // [esp+1Ch] [ebp-Ch]
    bot_threat_t *threat; // [esp+20h] [ebp-8h]
    bool human; // [esp+27h] [ebp-1h]

    bestThreat = &botInfo->threat;
    for ( i = 0; i < numThreats; ++i )
    {
        threat = &threats[i];
        if ( threat->enemy )
        {
            if ( !bestThreat->enemy )
            {
                bestThreat = &threats[i];
                break;
            }
            if ( bestThreat->enemy != threat->enemy )
            {
                betterAngle = (float)(1.0 - botInfo->threat.dot) > (float)(1.0 - threat->dot);
                closer = botInfo->threat.distSq > threat->distSq;
                veryClose = (float)(sv_botCloseDistance->current.value * sv_botCloseDistance->current.value) > threat->distSq;
                human = Bot_IsEnemyHuman(botInfo->threat.enemy);
                if ( veryClose )
                {
                    bestThreat = &threats[i];
                    break;
                }
                if ( bestThreat->acquireTime && svs.time - bestThreat->acquireTime > 500 )
                {
                    if ( closer && betterAngle )
                    {
                        bestThreat = &threats[i];
                    }
                    else if ( human && random() < 0.75 )
                    {
                        bestThreat = &threats[i];
                    }
                    else if ( closer && random() < 0.5 )
                    {
                        bestThreat = &threats[i];
                    }
                    else if ( betterAngle && random() < 0.5 )
                    {
                        bestThreat = &threats[i];
                    }
                    else
                    {
                        bestThreat->acquireTime = svs.time;
                    }
                }
            }
        }
    }
    if ( bestThreat )
    {
        if ( bestThreat == &botInfo->threat )
        {
            botInfo->flags &= ~1u;
        }
        else
        {
            memcpy(&botInfo->threat, bestThreat, sizeof(botInfo->threat));
            botInfo->threat.acquireTime = svs.time;
            botInfo->flags |= 1u;
        }
    }
    else
    {
        botInfo->flags &= ~1u;
    }
}

bool __cdecl Bot_ThreatSightCheck(const client_t *bot, const gentity_s *enemy, float fovDot)
{
    float *currentOrigin; // [esp+20h] [ebp-58h]
    bool v6; // [esp+2Eh] [ebp-4Ah]
    col_context_t context; // [esp+30h] [ebp-48h] BYREF
    float enemyOrigin[3]; // [esp+5Ch] [ebp-1Ch] BYREF
    float botOrigin[3]; // [esp+68h] [ebp-10h] BYREF
    int hitNum; // [esp+74h] [ebp-4h] BYREF

    //PIXBeginNamedEvent(-1, "Bot_ThreatSightCheck");
    //col_context_t::col_context_t(&context);
    if ( g_botFovCos <= fovDot )
    {
        currentOrigin = bot->gentity->r.currentOrigin;
        botOrigin[0] = *currentOrigin;
        botOrigin[1] = currentOrigin[1];
        botOrigin[2] = currentOrigin[2];
        botOrigin[2] = botOrigin[2] + bot->gentity->client->ps.viewHeightCurrent;
        enemyOrigin[0] = enemy->r.currentOrigin[0];
        enemyOrigin[1] = enemy->r.currentOrigin[1];
        enemyOrigin[2] = enemy->r.currentOrigin[2];
        if ( enemy && enemy->client )
        {
            enemyOrigin[2] = enemyOrigin[2] + enemy->client->ps.viewHeightCurrent;
            if ( (enemy->client->ps.eFlags & 0x4000) == 0 )
                AddLeanToPosition(enemyOrigin, enemy->client->ps.viewangles[1], enemy->client->ps.leanf, 16.0, 20.0);
        }
        else
        {
            enemyOrigin[2] = (float)(enemy->r.maxs[2] / 2.0) + enemyOrigin[2];
        }
        context.passEntityNum0 = bot->gentity->s.number;
        context.passEntityNum1 = enemy->s.number;
        context.mask = 0x2818011;
        SV_SightTracePoint(&hitNum, botOrigin, enemyOrigin, &context);
        v6 = hitNum == 0;
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
        return v6;
    }
    else
    {
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
        return 0;
    }
}

void __cdecl Bot_UpdateWeapon(const bot_info_t *botInfo, const client_t *bot, usercmd_s *cmd)
{
    char v3; // [esp+0h] [ebp-44h]
    bool v4; // [esp+4h] [ebp-40h]
    const gentity_s *v5; // [esp+1Ch] [ebp-28h]
    const gentity_s *enemy; // [esp+20h] [ebp-24h]
    int k; // [esp+2Ch] [ebp-18h]
    int j; // [esp+30h] [ebp-14h]
    int i; // [esp+34h] [ebp-10h]
    gclient_s *ps; // [esp+3Ch] [ebp-8h]
    const WeaponDef *weapDef; // [esp+40h] [ebp-4h]
    const WeaponDef *weapDefa; // [esp+40h] [ebp-4h]
    const WeaponDef *weapDefb; // [esp+40h] [ebp-4h]

    //PIXBeginNamedEvent(-1, "Bot_UpdateWeapon");
    ps = G_GetPlayerState(bot->gentity->s.number);
    if ( ps->ps.weaponstate )
    {
        //if ( g_DXDeviceThread != GetCurrentThreadId() )
        //    return;
        goto LABEL_59;
    }
    if ( botInfo->weapon )
        cmd->weapon = botInfo->weapon;
    enemy = botInfo->threat.enemy;
    v4 = enemy && enemy->pTurretInfo;
    if ( v4 || ((v5 = botInfo->threat.enemy) == 0 || !v5->scr_vehicle ? (v3 = 0) : (v3 = 1), v3) )
    {
        if ( BG_GetWeaponDef(botInfo->weapon)->weapType != WEAPTYPE_PROJECTILE )
        {
            for ( i = 0; i < 15; ++i )
            {
                if ( ps->ps.heldWeapons[i].weapon )
                {
                    if ( ps->ps.heldWeapons[i].weapon != cmd->weapon )
                    {
                        if ( BG_WeaponAmmo(&ps->ps, ps->ps.heldWeapons[i].weapon) )
                        {
                            weapDef = BG_GetWeaponDef(ps->ps.heldWeapons[i].weapon);
                            if ( weapDef->weapType != WEAPTYPE_BOMB
                                && weapDef->weapClass != WEAPCLASS_PISTOL
                                && weapDef->weapClass != WEAPCLASS_TURRET
                                && weapDef->weapType == WEAPTYPE_PROJECTILE )
                            {
                                cmd->weapon = ps->ps.heldWeapons[i].weapon;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    if ( !cmd->weapon || !BG_PlayerHasWeapon(&ps->ps, cmd->weapon) || !BG_WeaponAmmo(&ps->ps, cmd->weapon) )
    {
        for ( j = 0; j < 15; ++j )
        {
            if ( ps->ps.heldWeapons[j].weapon )
            {
                if ( ps->ps.heldWeapons[j].weapon != cmd->weapon )
                {
                    if ( BG_WeaponAmmo(&ps->ps, ps->ps.heldWeapons[j].weapon) )
                    {
                        weapDefa = BG_GetWeaponDef(ps->ps.heldWeapons[j].weapon);
                        if ( !weapDefa->requireLockonToFire
                            && weapDefa->weapType != WEAPTYPE_BOMB
                            && weapDefa->weapClass != WEAPCLASS_TURRET
                            && (weapDefa->weapType == WEAPTYPE_BULLET || weapDefa->weapType == WEAPTYPE_PROJECTILE) )
                        {
                            cmd->weapon = ps->ps.heldWeapons[j].weapon;
                            break;
                        }
                    }
                }
            }
        }
    }
    if ( !cmd->weapon )
    {
        for ( k = 0; k < 15; ++k )
        {
            if ( ps->ps.heldWeapons[k].weapon )
            {
                if ( ps->ps.heldWeapons[k].weapon != cmd->weapon )
                {
                    weapDefb = BG_GetWeaponDef(ps->ps.heldWeapons[k].weapon);
                    if ( weapDefb->weapType != WEAPTYPE_BOMB
                        && weapDefb->weapClass != WEAPCLASS_TURRET
                        && (weapDefb->weapType == WEAPTYPE_BULLET || weapDefb->weapType == WEAPTYPE_PROJECTILE) )
                    {
                        cmd->weapon = ps->ps.heldWeapons[k].weapon;
                        break;
                    }
                }
            }
        }
    }
    //if ( g_DXDeviceThread == GetCurrentThreadId() )
LABEL_59:
    ;
        //D3DPERF_EndEvent();
}

void __cdecl Bot_UpdateMovement(bot_info_t *botInfo, const client_t *bot, usercmd_s *cmd)
{
    const gentity_s *enemy; // [esp+1Ch] [ebp-20h]

    //PIXBeginNamedEvent(-1, "Bot_UpdateMovement");
    if ( !sv_botsAllowMovement->current.enabled )
    {
        cmd->forwardmove = 0;
        cmd->rightmove = 0;
        if ( sv_botForceYaw->current.integer >= 0 )
            cmd->angles[1] = (unsigned __int16)(int)(float)((float)((float)sv_botForceYaw->current.integer
                                                                                                                        - bot->gentity->client->ps.delta_angles[1])
                                                                                                        * 182.04445);
        //if ( GetCurrentThreadId() != g_DXDeviceThread )
        //    return;
        goto LABEL_31;
    }
    if ( (bot->gentity->client->ps.pm_flags & 4) != 0 || (bot->gentity->client->ps.pm_flags & 0x4000) != 0 )
    {
        Bot_UpdateMantle(botInfo, bot, cmd);
        //if ( g_DXDeviceThread != GetCurrentThreadId() )
        //    return;
        goto LABEL_31;
    }
    if ( Bot_IsFlashbanged(bot) )
    {
        Bot_RandomInput(botInfo, cmd);
        //if ( GetCurrentThreadId() != g_DXDeviceThread )
        //    return;
        goto LABEL_31;
    }
    Bot_GetStrafeInput(bot, botInfo, cmd);
    Bot_UpdatePath(botInfo, bot);
    if ( Path_Exists(&botInfo->path) || !botInfo->threat.enemy || !botInfo->sightHitNum )
    {
        if ( Path_Exists(&botInfo->path) || !botInfo->sightHitNum )
        {
            Bot_UpdateDirection(botInfo, bot, cmd);
            Bot_UpdateSpeed(botInfo, bot, cmd);
            Bot_UpdateMantle(botInfo, bot, cmd);
            //if ( g_DXDeviceThread != GetCurrentThreadId() )
            //    return;
        }
        else
        {
            Bot_RandomInput(botInfo, cmd);
            //if ( g_DXDeviceThread != GetCurrentThreadId() )
            //    return;
        }
        goto LABEL_31;
    }
    Bot_UpdateDirection(botInfo, bot, cmd);
    enemy = botInfo->threat.enemy;
    if ( enemy && enemy->client )
    {
        Bot_RandomInput(botInfo, cmd);
    }
    else
    {
        cmd->forwardmove = 0;
        cmd->rightmove = 0;
    }
    //if ( g_DXDeviceThread == GetCurrentThreadId() )
LABEL_31:
    ;
        //D3DPERF_EndEvent();
}

char right[12] =
{ '\x81', '\xC0', '@', '\x7F', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };

char forward[8] =
{ '\xC0', '@', '\x7F', '\x7F', '\x7F', '\x81', '\0', '\0' };

void __cdecl Bot_RandomInput(bot_info_t *botInfo, usercmd_s *cmd)
{
    if ( svs.time >= botInfo->threat.strafeEndTime )
    {
        cmd->forwardmove = forward[irand(0, 6)];
        cmd->rightmove = right[irand(0, 9)];
        botInfo->threat.strafeEndTime = svs.time + irand(sv_botMinStrafeTime->current.integer, sv_botMaxStrafeTime->current.integer);
    }
}

void __cdecl Bot_GetStrafeInput(const client_t *bot, bot_info_t *botInfo, usercmd_s *cmd)
{
    gentity_s *gentity; // eax
    bool v4; // [esp+0h] [ebp-A0h]
    col_context_t context; // [esp+18h] [ebp-88h] BYREF
    float vEnd[3]; // [esp+40h] [ebp-60h] BYREF
    trace_t trace; // [esp+4Ch] [ebp-54h] BYREF
    float vStart[3]; // [esp+88h] [ebp-18h] BYREF
    bool checkStrafe; // [esp+97h] [ebp-9h]
    const playerState_s *ps; // [esp+98h] [ebp-8h]
    float distSq; // [esp+9Ch] [ebp-4h] BYREF

    //PIXBeginNamedEvent(-1, "Bot_GetStrafeInput");
    ps = (const playerState_s *)G_GetPlayerState(bot->gentity->s.number);
    if ( (botInfo->flags & 0x40) != 0 )
    {
        memset(&trace, 0, 16);
        gentity = bot->gentity;
        vStart[0] = gentity->r.currentOrigin[0];
        vStart[1] = gentity->r.currentOrigin[1];
        vStart[2] = gentity->r.currentOrigin[2];
        Bot_PredictPosition(bot->gentity, ps, vEnd);
        vStart[2] = (float)(ps->viewHeightCurrent * 0.5) + vStart[2];
        vEnd[2] = (float)(ps->viewHeightCurrent * 0.5) + vEnd[2];
        //col_context_t::col_context_t(&context);
        G_TraceCapsule(
            &trace,
            vStart,
            bot->gentity->r.mins,
            bot->gentity->r.maxs,
            vEnd,
            bot->gentity->s.number,
            0x2820011,
            &context);
        if ( trace.startsolid || trace.allsolid || trace.fraction != 1.0 )
            Bot_RandomStrafe(ps, cmd);
    }
    if ( !botInfo->sightHitNum && !Bot_IsInRangeOfEnemy(bot, botInfo->threat.enemy, &distSq) )
    {
        //if ( g_DXDeviceThread != GetCurrentThreadId() )
        //    return;
        goto LABEL_21;
    }
    v4 = !botInfo->sightHitNum && ((botInfo->flags & 0x20) != 0 || svs.time >= botInfo->threat.strafeEndTime);
    checkStrafe = v4;
    if ( v4 )
    {
        if ( sv_botStrafeChance->current.value <= random() )
        {
            cmd->forwardmove = 0;
            cmd->rightmove = 0;
            botInfo->flags &= ~0x40u;
        }
        else
        {
            Bot_RandomStrafe(ps, cmd);
            botInfo->flags |= 0x40u;
        }
        botInfo->threat.strafeEndTime = svs.time
                                                                    + irand(sv_botMinStrafeTime->current.integer, sv_botMaxStrafeTime->current.integer);
    }
    //if ( GetCurrentThreadId() == g_DXDeviceThread )
LABEL_21:
    ;
        //D3DPERF_EndEvent();
}

void __cdecl Bot_RandomStrafe(const playerState_s *ps, usercmd_s *cmd)
{
    cmd->forwardmove = forward_0[irand(0, 5)];
    if ( ps->fWeaponPosFrac < 1.0 )
        cmd->rightmove = right_0[irand(0, 4)];
    else
        cmd->rightmove = rightAds[irand(0, 2)];
}

void __cdecl Bot_UpdatePath(bot_info_t *botInfo, const client_t *bot)
{
    const pathnode_t *node; // [esp+44h] [ebp-10h]
    gclient_s *ps; // [esp+50h] [ebp-4h]

    //PIXBeginNamedEvent(-1, "Bot_UpdatePath");
    if ( botInfo->sightHitNum && Bot_UpdateScriptGoal(botInfo, bot) )
    {
        //if ( g_DXDeviceThread != GetCurrentThreadId() )
        //    return;
        goto LABEL_46;
    }
    if ( Bot_UpdateScriptEnemy(botInfo, bot) )
    {
        //if ( g_DXDeviceThread != GetCurrentThreadId() )
        //    return;
        goto LABEL_46;
    }
    if ( Bot_IsInLastStand(bot) )
    {
        //if ( g_DXDeviceThread != GetCurrentThreadId() )
        //    return;
        goto LABEL_46;
    }
    if ( (botInfo->flags & 0x80) != 0 )
    {
        //if ( g_DXDeviceThread != GetCurrentThreadId() )
        //    return;
        goto LABEL_46;
    }
    ps = G_GetPlayerState(bot - svs.clients);
    if ( (ps->ps.pm_flags & 4) != 0 || (ps->ps.pm_flags & 0x4000) != 0 )
    {
        //if ( g_DXDeviceThread != GetCurrentThreadId() )
        //    return;
        goto LABEL_46;
    }
    if ( (botInfo->flags & 2) == 0
        || Bot_IsAtNegotiationNode(botInfo, bot, &botInfo->path)
        || Bot_IsMovingFoward(botInfo, bot, &ps->ps) )
    {
        botInfo->lastMoveTime = svs.time;
    }
    else if ( !botInfo->sightHitNum || !Path_Exists(&botInfo->path) || svs.time - botInfo->lastMoveTime >= 2000 )
    {
        botInfo->flags &= ~2u;
        Path_Clear(&botInfo->path);
    }
    if ( Bot_PointInGoal(bot->gentity->r.currentOrigin, &botInfo->path, 0.0) )
    {
        botInfo->flags &= ~2u;
        Path_Clear(&botInfo->path);
        if ( botInfo->threat.enemy )
            botInfo->flags |= 1u;
    }
    if ( !botInfo->sightHitNum && (botInfo->flags & 2) != 0 )
    {
        botInfo->flags &= ~2u;
        botInfo->flags |= 1u;
        Path_Clear(&botInfo->path);
    }
    if ( (botInfo->flags & 1) != 0 )
    {
        botInfo->flags &= ~2u;
        botInfo->flags &= ~1u;
        Bot_GetPathToGoal(bot, botInfo->threat.enemy->r.currentOrigin, &botInfo->path, 1);
        //if ( g_DXDeviceThread != GetCurrentThreadId() )
        //    return;
        goto LABEL_46;
    }
    if ( botInfo->threat.enemy && !Bot_PointInGoal(botInfo->threat.enemy->r.currentOrigin, &botInfo->path, 0.0) )
    {
        Bot_GetPathToGoal(bot, botInfo->threat.enemy->r.currentOrigin, &botInfo->path, 1);
        botInfo->flags &= ~2u;
        //if ( g_DXDeviceThread != GetCurrentThreadId() )
        //    return;
        goto LABEL_46;
    }
    if ( !Path_Exists(&botInfo->path) )
    {
        if ( gameWorldMp.path.nodeCount )
        {
            node = &gameWorldMp.path.nodes[irand(0, gameWorldMp.path.nodeCount)];
            if ( node->constant.type == NODE_PATHNODE && !node->constant.script_noteworthy )
            {
                botInfo->flags |= 2u;
                botInfo->lastMoveTime = svs.time;
                Bot_GetPathToGoal(bot, node->constant.vOrigin, &botInfo->path, 1);
            }
        }
    }
    //if ( g_DXDeviceThread == GetCurrentThreadId() )
LABEL_46:
    ;
        //D3DPERF_EndEvent();
}

char __cdecl Bot_IsMovingFoward(const bot_info_t *botInfo, const client_t *bot, const playerState_s *ps)
{
    bool v4; // [esp+32h] [ebp-16h]
    float vForward[3]; // [esp+38h] [ebp-10h] BYREF
    float vDot; // [esp+44h] [ebp-4h]

    //PIXBeginNamedEvent(-1, "Bot_IsMovingFoward");
    if ( (botInfo->flags & 0x10) != 0 )
    {
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
        return 1;
    }
    else if ( (botInfo->flags & 0x100) != 0 )
    {
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
        return 1;
    }
    else if ( (ps->pm_flags & 4) != 0 || (ps->pm_flags & 0x4000) != 0 )
    {
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
        return 1;
    }
    else if ( Bot_IsThrowingGrenade(botInfo, bot) )
    {
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
        return 1;
    }
    else
    {
        AngleVectors(ps->viewangles, vForward, 0, 0);
        vDot = (float)((float)(ps->velocity[0] * vForward[0]) + (float)(ps->velocity[1] * vForward[1]))
                 + (float)(ps->velocity[2] * vForward[2]);
        v4 = fabs(vDot) > 1.0;
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
        return v4;
    }
}

bool __cdecl Bot_IsThrowingGrenade(const bot_info_t *botInfo, const client_t *bot)
{
    if ( (botInfo->flags & 0x400) != 0 || (botInfo->flags & 0x800) != 0 )
        return 1;
    return bot->gentity->client->ps.weaponstate >= 20 && bot->gentity->client->ps.weaponstate <= 25;
}

char __cdecl Bot_PointInGoal(const float *point, const path_t *path, float goalRadius)
{
    //PIXBeginNamedEvent(-1, "Bot_PointInGoal");
    if ( goalRadius == 0.0 )
        goalRadius = sv_botGoalRadius->current.value;
    if ( Path_Exists(path) && goalRadius * goalRadius > Vec3DistanceSq(point, (const float *)path) )
    {
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
        return 1;
    }
    else
    {
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
        return 0;
    }
}

bool __cdecl Bot_GetPathToGoal(const client_t *bot, const float *goal, path_t *path, bool allowMantle)
{
    bool v5; // [esp+Ah] [ebp-2h]

    //PIXBeginNamedEvent(-1, "Bot_GetPathToGoal");
    Path_Begin(path);
    if ( allowMantle )
        Path_FindPath(path, TEAM_FREE, bot->gentity->r.currentOrigin, goal, 1);
    if ( !Bot_PathValid(bot->gentity, path) )
    {
        Path_Begin(path);
        Path_FindPath(path, TEAM_FREE, bot->gentity->r.currentOrigin, goal, 0);
    }
    v5 = Path_Exists(path);
    //if ( GetCurrentThreadId() == g_DXDeviceThread )
        //D3DPERF_EndEvent();
    return v5;
}

char __cdecl Bot_PathValid(const gentity_s *bot, const path_t *path)
{
    col_context_t context; // [esp+3Ch] [ebp-98h] BYREF
    const pathnode_t *mantleEnd; // [esp+68h] [ebp-6Ch]
    float vEnd[3]; // [esp+6Ch] [ebp-68h] BYREF
    pmove_t *pm; // [esp+78h] [ebp-5Ch]
    int traverseCount; // [esp+7Ch] [ebp-58h]
    trace_t trace; // [esp+80h] [ebp-54h] BYREF
    const pathnode_t *mantleStart; // [esp+BCh] [ebp-18h]
    int i; // [esp+C0h] [ebp-14h]
    const playerState_s *ps; // [esp+C4h] [ebp-10h]
    float vStart[3]; // [esp+C8h] [ebp-Ch] BYREF

    //PIXBeginNamedEvent(-1, "Bot_PathValid");
    memset(&trace, 0, 16);
    if ( Path_Exists(path) )
    {
        if ( Path_HasNegotiationNode(path) )
        {
            i = path->wPathLen - 1;
            traverseCount = 0;
            while ( i >= 0 )
            {
                if ( path->pts[i].iNodeNum >= 0 )
                {
                    if ( Path_ConvertIndexToNode(path->pts[i].iNodeNum)->constant.type == NODE_NEGOTIATION_BEGIN )
                        ++traverseCount;
                    if ( traverseCount > 1 )
                    {
                        //if ( g_DXDeviceThread == GetCurrentThreadId() )
                            //D3DPERF_EndEvent();
                        return 0;
                    }
                }
                --i;
            }
            if ( path->wNegotiationStartNode - 1 < 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_bot_mp.cpp",
                            622,
                            0,
                            "%s",
                            "path->wNegotiationStartNode - 1 >= 0") )
            {
                __debugbreak();
            }
            pm = &g_pmove[bot->s.number];
            ps = (const playerState_s *)G_GetPlayerState(bot->s.number);
            mantleStart = Path_GetNegotiationNode(path);
            mantleEnd = Path_ConvertIndexToNode(path->pts[path->wNegotiationStartNode - 1].iNodeNum);
            vStart[0] = mantleStart->constant.vOrigin[0];
            vStart[1] = mantleStart->constant.vOrigin[1];
            vStart[2] = mantleStart->constant.vOrigin[2];
            vStart[2] = vStart[2] + 8.0;
            vEnd[0] = mantleEnd->constant.vOrigin[0];
            vEnd[1] = mantleEnd->constant.vOrigin[1];
            vEnd[2] = mantleEnd->constant.vOrigin[2];
            vEnd[2] = vEnd[2] + 8.0;
            //col_context_t::col_context_t(&context);
            G_TraceCapsule(
                &trace,
                vStart,
                bot->r.mins,
                bot->r.maxs,
                vEnd,
                bot->s.number,
                0x1000000,
                &context);
            if ((trace.cflags & 0x1000000) != 0)
            {
                //if ( g_DXDeviceThread == GetCurrentThreadId() )
                    //D3DPERF_EndEvent();
                return 1;
            }
            else
            {
                //if ( g_DXDeviceThread == GetCurrentThreadId() )
                    //D3DPERF_EndEvent();
                return 0;
            }
        }
        else
        {
            //if ( GetCurrentThreadId() == g_DXDeviceThread )
                //D3DPERF_EndEvent();
            return 1;
        }
    }
    else
    {
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
        return 0;
    }
}

char __cdecl Bot_IsAtNegotiationNode(bot_info_t *botInfo, const client_t *bot, path_t *path)
{
    float goal[3]; // [esp+70h] [ebp-20h] BYREF
    float start_z; // [esp+7Ch] [ebp-14h]
    float diff; // [esp+80h] [ebp-10h]
    float distSq; // [esp+84h] [ebp-Ch]
    float end_z; // [esp+88h] [ebp-8h]

    //PIXBeginNamedEvent(-1, "Bot_IsAtNegotiationNode");
    if ( (botInfo->flags & 0x80) != 0 )
    {
        if ( path->wNegotiationStartNode - 1 >= 0 )
        {
            start_z = path->pts[path->wNegotiationStartNode].vOrigPoint[2];
            end_z = path->pts[path->wNegotiationStartNode - 1].vOrigPoint[2];
            diff = end_z - start_z;
            distSq = Vec3DistanceSq(bot->gentity->r.currentOrigin, path->pts[path->wNegotiationStartNode - 1].vOrigPoint);
            if ( (float)(end_z - start_z) >= -24.0
                || distSq >= 4096.0
                || fabs(end_z - bot->gentity->r.currentOrigin[2]) >= 5.0 )
            {
                //if ( g_DXDeviceThread == GetCurrentThreadId() )
                    //D3DPERF_EndEvent();
                return 1;
            }
            else
            {
                botInfo->flags &= ~0x80u;
                botInfo->flags &= ~0x100u;
                botInfo->flags &= ~0x200u;
                goal[0] = path->pts[0].vOrigPoint[0];
                goal[1] = path->pts[0].vOrigPoint[1];
                goal[2] = path->pts[0].vOrigPoint[2];
                Bot_GetPathToGoal(bot, goal, path, 0);
                //if ( g_DXDeviceThread == GetCurrentThreadId() )
                    //D3DPERF_EndEvent();
                return 0;
            }
        }
        else
        {
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                //D3DPERF_EndEvent();
            return 1;
        }
    }
    else if ( Path_Exists(path) )
    {
        if ( Path_HasNegotiationNode(path) )
        {
            if ( path->lookaheadNextNode == path->wNegotiationStartNode )
            {
                if ( path->fLookaheadDistToNextNode <= 0.000001 )
                {
                    if ( Vec3DistanceSq(bot->gentity->r.currentOrigin, path->pts[path->wNegotiationStartNode].vOrigPoint) >= 256.0 )
                    {
                        //if ( GetCurrentThreadId() == g_DXDeviceThread )
                            //D3DPERF_EndEvent();
                        return 0;
                    }
                    else
                    {
                        botInfo->flags |= 0x80u;
                        botInfo->flags |= 0x100u;
                        botInfo->flags |= 0x200u;
                        //if ( GetCurrentThreadId() == g_DXDeviceThread )
                            //D3DPERF_EndEvent();
                        return 1;
                    }
                }
                else
                {
                    //if ( g_DXDeviceThread == GetCurrentThreadId() )
                        //D3DPERF_EndEvent();
                    return 0;
                }
            }
            else
            {
                //if ( GetCurrentThreadId() == g_DXDeviceThread )
                    //D3DPERF_EndEvent();
                return 0;
            }
        }
        else
        {
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                //D3DPERF_EndEvent();
            return 0;
        }
    }
    else
    {
        //if ( GetCurrentThreadId() == g_DXDeviceThread )
            //D3DPERF_EndEvent();
        return 0;
    }
}

char __cdecl Bot_UpdateScriptGoal(bot_info_t *botInfo, const client_t *bot)
{
    //PIXBeginNamedEvent(-1, "Bot_UpdateScriptGoal");
    if ( (botInfo->flags & 4) != 0 )
    {
        if ( Bot_PointInGoal(bot->gentity->r.currentOrigin, &botInfo->path, botInfo->scriptGoalRadius) )
        {
            Scr_Notify(bot->gentity, scr_const.goal, 0);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                //D3DPERF_EndEvent();
            return 1;
        }
        else if ( Path_Exists(&botInfo->path)
                     && Bot_PointInGoal(botInfo->scriptGoal, &botInfo->path, botInfo->scriptGoalRadius) )
        {
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                //D3DPERF_EndEvent();
            return 1;
        }
        else if ( Bot_GetPathToGoal(bot, botInfo->scriptGoal, &botInfo->path, 1) )
        {
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                //D3DPERF_EndEvent();
            return 1;
        }
        else
        {
            Scr_Notify(bot->gentity, scr_const.bad_path, 0);
            SV_BotClearScriptGoal(bot);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                //D3DPERF_EndEvent();
            return 0;
        }
    }
    else
    {
        //if ( GetCurrentThreadId() == g_DXDeviceThread )
            //D3DPERF_EndEvent();
        return 0;
    }
}

char __cdecl Bot_UpdateScriptEnemy(bot_info_t *botInfo, const client_t *bot)
{
    const gentity_s *enemy; // [esp+1Ch] [ebp-24h]

    //PIXBeginNamedEvent(-1, "Bot_UpdateScriptEnemy");
    if ( (botInfo->flags & 8) != 0 )
    {
        if ( botInfo->threat.enemy && botInfo->threat.enemy->r.inuse )
        {
            if ( botInfo->threat.enemy->health > 0 )
            {
                enemy = botInfo->threat.enemy;
                if ( enemy && enemy->scr_vehicle )
                {
                    Path_Clear(&botInfo->path);
                    //if ( GetCurrentThreadId() == g_DXDeviceThread )
                        //D3DPERF_EndEvent();
                    return 1;
                }
                else if ( Bot_GetPathToGoal(bot, botInfo->threat.enemy->r.currentOrigin, &botInfo->path, 1) )
                {
                    //if ( GetCurrentThreadId() == g_DXDeviceThread )
                        //D3DPERF_EndEvent();
                    return 1;
                }
                else
                {
                    Scr_Notify(bot->gentity, scr_const.bad_path, 0);
                    SV_BotClearScriptEnemy(bot);
                    //if ( GetCurrentThreadId() == g_DXDeviceThread )
                        //D3DPERF_EndEvent();
                    return 0;
                }
            }
            else
            {
                SV_BotClearScriptEnemy(bot);
                //if ( g_DXDeviceThread == GetCurrentThreadId() )
                    //D3DPERF_EndEvent();
                return 0;
            }
        }
        else
        {
            SV_BotClearScriptEnemy(bot);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                //D3DPERF_EndEvent();
            return 0;
        }
    }
    else
    {
        //if ( GetCurrentThreadId() == g_DXDeviceThread )
            //D3DPERF_EndEvent();
        return 0;
    }
}

void __cdecl Bot_PredictPosition(const gentity_s *ent, const playerState_s *ps, float *vPos)
{
    trajectory_t tr; // [esp+20h] [ebp-24h] BYREF

    tr.trBase[0] = ent->r.currentOrigin[0];
    tr.trBase[1] = ent->r.currentOrigin[1];
    tr.trBase[2] = ent->r.currentOrigin[2];
    if ( ((LODWORD(tr.trBase[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(tr.trBase[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(tr.trBase[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_bot_mp.cpp",
                    1618,
                    0,
                    "%s",
                    "!IS_NAN((tr.trBase)[0]) && !IS_NAN((tr.trBase)[1]) && !IS_NAN((tr.trBase)[2])") )
    {
        __debugbreak();
    }
    tr.trDelta[0] = ps->velocity[0];
    tr.trDelta[1] = ps->velocity[1];
    tr.trDelta[2] = ps->velocity[2];
    if ( ((LODWORD(tr.trDelta[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(tr.trDelta[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(tr.trDelta[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_bot_mp.cpp",
                    1621,
                    0,
                    "%s",
                    "!IS_NAN((tr.trDelta)[0]) && !IS_NAN((tr.trDelta)[1]) && !IS_NAN((tr.trDelta)[2])") )
    {
        __debugbreak();
    }
    tr.trTime = svs.time;
    tr.trType = 3;
    BG_EvaluateTrajectory(&tr, svs.time + 50 * sv_botTargetLeadBias->current.integer, vPos);
}

void __cdecl Bot_UpdateDirection(bot_info_t *botInfo, const client_t *bot, usercmd_s *cmd)
{
    double v3; // st7
    const gentity_s *v4; // eax
    const gentity_s *enemy; // eax
    char v6; // [esp+14h] [ebp-100h]
    bool v8; // [esp+1Ch] [ebp-F8h]
    float *v10; // [esp+60h] [ebp-B4h]
    float *v11; // [esp+74h] [ebp-A0h]
    float *currentOrigin; // [esp+80h] [ebp-94h]
    const gentity_s *v13; // [esp+8Ch] [ebp-88h]
    const gentity_s *v14; // [esp+A0h] [ebp-74h]
    float diff; // [esp+B4h] [ebp-60h]
    float angle; // [esp+C0h] [ebp-54h]
    float head; // [esp+C4h] [ebp-50h]
    float dir[3]; // [esp+CCh] [ebp-48h] BYREF
    bool ads; // [esp+DBh] [ebp-39h]
    float yaw; // [esp+DCh] [ebp-38h]
    float pitch; // [esp+E0h] [ebp-34h]
    float enemyOrigin[3]; // [esp+E4h] [ebp-30h] BYREF
    float botOrigin[3]; // [esp+F0h] [ebp-24h] BYREF
    float viewAngles[3]; // [esp+FCh] [ebp-18h] BYREF
    const playerState_s *ps; // [esp+108h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+10Ch] [ebp-8h]
    const playerState_s *enemyPs; // [esp+110h] [ebp-4h]

    //PIXBeginNamedEvent(-1, "Bot_UpdateDirection");
    ps = (const playerState_s *)G_GetPlayerState(bot->gentity->s.number);
    weapDef = BG_GetWeaponDef(ps->weapon);
    if ( botInfo->threat.enemy && botInfo->threat.deathEndTime )
    {
        cmd->angles[0] = bot->lastUsercmd.angles[0];
        cmd->angles[1] = bot->lastUsercmd.angles[1];
        cmd->angles[2] = 0;
        //if ( GetCurrentThreadId() == g_DXDeviceThread )
    LABEL_4:
        ;
            //D3DPERF_EndEvent();
    }
    else
    {
        if ( botInfo->sightHitNum || !cmd->button_bits.testBit(0xBu) || ps->fWeaponPosFrac >= 1.0 )
        {
            if ( botInfo->sightHitNum )
            {
                if ( Bot_IsThrowingGrenade(botInfo, bot) )
                {
                    viewAngles[0] = ps->viewangles[0];
                    viewAngles[1] = ps->viewangles[1];
                    viewAngles[2] = ps->viewangles[2];
                    if ( botInfo->threat.enemy )
                    {
                        if ( botInfo->sightHitNum
                            && botInfo->threat.lastSightTime
                            && svs.time - botInfo->threat.lastSightTime < 5000 )
                        {
                            currentOrigin = bot->gentity->r.currentOrigin;
                            dir[0] = botInfo->threat.grenadeOrigin[0] - *currentOrigin;
                            dir[1] = botInfo->threat.grenadeOrigin[1] - currentOrigin[1];
                            dir[2] = botInfo->threat.grenadeOrigin[2] - currentOrigin[2];
                            Vec3Normalize(dir);
                            vectoangles(dir, viewAngles);
                        }
                    }
                    viewAngles[0] = botInfo->targetPitch;
                }
                else
                {
                    if ( Bot_IsInLastStand(bot) )
                    {
                        cmd->angles[0] = bot->lastUsercmd.angles[0];
                        cmd->angles[1] = bot->lastUsercmd.angles[1];
                        cmd->angles[2] = 0;
                        //if ( g_DXDeviceThread == GetCurrentThreadId() )
                            goto LABEL_4;
                        return;
                    }
                    if ( Path_Exists(&botInfo->path) || !botInfo->threat.enemy )
                    {
                        if ( botInfo->threat.enemy
                            && (!botInfo->sightHitNum
                             || !botInfo->threat.lastSightTime
                             || svs.time - botInfo->threat.lastSightTime >= 5000
                                ? (v6 = 0)
                                : (v6 = 1),
                                    v6) )
                        {
                            Path_UpdateLookahead(&botInfo->path, bot->gentity->r.currentOrigin, 0, 0, 0, 1);
                            v10 = bot->gentity->r.currentOrigin;
                            botOrigin[0] = *v10;
                            botOrigin[1] = v10[1];
                            botOrigin[2] = v10[2] + ps->viewHeightCurrent;
                            if ( Vec3DistanceSq(bot->gentity->r.currentOrigin, botInfo->threat.lastOrigin) < 4096.0 )
                                botInfo->threat.lastSightTime = 0;
                            enemyOrigin[0] = botInfo->threat.lastOrigin[0];
                            enemyOrigin[1] = botInfo->threat.lastOrigin[1];
                            enemyOrigin[2] = botInfo->threat.lastOrigin[2];
                            enemyOrigin[2] = enemyOrigin[2] + 40.0;
                            dir[0] = enemyOrigin[0] - botOrigin[0];
                            dir[1] = enemyOrigin[1] - botOrigin[1];
                            dir[2] = enemyOrigin[2] - botOrigin[2];
                            Vec3Normalize(dir);
                            vectoangles(dir, viewAngles);
                        }
                        else
                        {
                            Path_UpdateLookahead(&botInfo->path, bot->gentity->r.currentOrigin, 0, 0, 0, 1);
                            vectoangles(botInfo->path.lookaheadDir, viewAngles);
                            if ( Bot_IsAtNegotiationNode(botInfo, bot, &botInfo->path) )
                                vectoangles(
                                    gameWorldMp.path.nodes[botInfo->path.pts[botInfo->path.lookaheadNextNode].iNodeNum].constant.forward,
                                    viewAngles);
                            angle = AngleNormalize180(viewAngles[0]);
                            if ( fabs(angle) >= 75.0 )
                                Path_Clear(&botInfo->path);
                            if ( fabs(angle) <= 5.0 && ps->fWeaponPosFrac <= 0.000001 )
                            {
                                if ( fabs(ps->viewangles[0] - botInfo->targetPitch) <= 0.5
                                    || fabs(botInfo->targetPitch) > sv_botPitchDown->current.value )
                                {
                                    botInfo->targetPitch = flrand(sv_botPitchUp->current.value, sv_botPitchDown->current.value);
                                }
                                viewAngles[0] = botInfo->targetPitch;
                            }
                        }
                    }
                    else
                    {
                        v11 = bot->gentity->r.currentOrigin;
                        botOrigin[0] = *v11;
                        botOrigin[1] = v11[1];
                        botOrigin[2] = v11[2];
                        botOrigin[2] = botOrigin[2] + ps->viewHeightCurrent;
                        enemy = botInfo->threat.enemy;
                        enemyOrigin[0] = enemy->r.currentOrigin[0];
                        enemyOrigin[1] = enemy->r.currentOrigin[1];
                        enemyOrigin[2] = enemy->r.currentOrigin[2];
                        dir[0] = enemyOrigin[0] - botOrigin[0];
                        dir[1] = enemyOrigin[1] - botOrigin[1];
                        dir[2] = enemyOrigin[2] - botOrigin[2];
                        Vec3Normalize(dir);
                        vectoangles(dir, viewAngles);
                    }
                }
            }
            else
            {
                Bot_PredictPosition(bot->gentity, ps, botOrigin);
                botOrigin[2] = botOrigin[2] + ps->viewHeightCurrent;
                v14 = botInfo->threat.enemy;
                if ( v14 && v14->client )
                {
                    enemyPs = (const playerState_s *)G_GetPlayerState(botInfo->threat.enemy->s.number);
                    Bot_PredictPosition(botInfo->threat.enemy, enemyPs, enemyOrigin);
                    if ( (enemyPs->eFlags & 0x4000) == 0 )
                        AddLeanToPosition(enemyOrigin, enemyPs->viewangles[1], enemyPs->leanf, 16.0, 20.0);
                    head = enemyOrigin[2] + enemyPs->viewHeightCurrent;
                    v3 = Vec3DistanceSq(botOrigin, enemyOrigin);
                    if ( weapDef->fMaxDamageRange * weapDef->fMaxDamageRange > v3 )
                        enemyOrigin[2] = (float)(enemyPs->viewHeightCurrent / 3.0) + enemyOrigin[2];
                    enemyOrigin[2] = flrand(enemyOrigin[2], head);
                }
                else
                {
                    v4 = botInfo->threat.enemy;
                    enemyOrigin[0] = v4->r.currentOrigin[0];
                    enemyOrigin[1] = v4->r.currentOrigin[1];
                    enemyOrigin[2] = v4->r.currentOrigin[2] + 15.0;
                }
                v13 = botInfo->threat.enemy;
                v8 = v13 && v13->pTurretInfo;
                if ( v8 && weapDef->weapType == WEAPTYPE_PROJECTILE )
                    enemyOrigin[2] = botInfo->threat.enemy->r.currentOrigin[2];
                dir[0] = enemyOrigin[0] - botOrigin[0];
                dir[1] = enemyOrigin[1] - botOrigin[1];
                dir[2] = enemyOrigin[2] - botOrigin[2];
                Vec3Normalize(dir);
                vectoangles(dir, viewAngles);
                if ( !Bot_IsThrowingGrenade(botInfo, bot) && ((botInfo->flags & 0x20) != 0 || !botInfo->pitchEndTime) )
                {
                    botInfo->targetPitch = viewAngles[0];
                    botInfo->pitchEndTime = svs.time
                                                                + irand(sv_botMinPitchTime->current.integer, sv_botMaxPitchTime->current.integer);
                }
                viewAngles[0] = botInfo->targetPitch;
            }
            ads = ps->fWeaponPosFrac > 0.0;
            yaw = Bot_UpdateYaw(ps->viewangles[1], viewAngles[1], ads, botInfo);
            pitch = Bot_UpdatePitch(ps->viewangles[0], viewAngles[0], ads);
            if ( (ps->pm_flags & 0x8000) != 0 )
                pitch = ps->viewangles[0];
            if ( Bot_IsAtNegotiationNode(botInfo, bot, &botInfo->path) )
            {
                diff = AngleNormalize180(yaw - viewAngles[1]);
                if ( diff >= 5.0 || diff <= -5.0 )
                {
                    botInfo->flags |= 0x100u;
                }
                else
                {
                    botInfo->flags &= ~0x100u;
                    yaw = viewAngles[1];
                }
            }
            cmd->angles[0] = (unsigned __int16)(int)(float)((float)(pitch - bot->gentity->client->ps.delta_angles[0])
                                                                                                        * 182.04445);
            cmd->angles[1] = (unsigned __int16)(int)(float)((float)(yaw - bot->gentity->client->ps.delta_angles[1]) * 182.04445);
            cmd->angles[2] = (unsigned __int16)(int)(float)(bot->gentity->client->ps.delta_angles[2] * 182.04445);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                //D3DPERF_EndEvent();
            return;
        }
        cmd->angles[0] = bot->lastUsercmd.angles[0];
        cmd->angles[1] = bot->lastUsercmd.angles[1];
        cmd->angles[2] = 0;
        //if ( GetCurrentThreadId() == g_DXDeviceThread )
            goto LABEL_4;
    }
}

double __cdecl Bot_UpdatePitch(float currentPitch, float targetPitch, bool forceSlow)
{
    float v4; // [esp+10h] [ebp-28h]
    float v5; // [esp+1Ch] [ebp-1Ch]
    float v6; // [esp+24h] [ebp-14h]
    float rate; // [esp+28h] [ebp-10h]

    //PIXBeginNamedEvent(-1, "Bot_UpdatePitch");
    rate = sv_botPitchSpeed->current.value;
    if ( forceSlow )
        rate = sv_botPitchSpeedAds->current.value;
    v5 = AngleNormalize180(targetPitch - currentPitch);
    //if ( (float)((float)(rate - (float)(COERCE_FLOAT(COERCE_UNSIGNED_INT(fabs(v5) / 180.0) ^ _mask__NegFloat_) * rate)) - rate) < 0.0 )
    if ((float)((float)(rate - (float)((-(fabs(v5) / 180.0)) * rate)) - rate) < 0.0)
    {
        //v4 = rate - (float)(COERCE_FLOAT(COERCE_UNSIGNED_INT(fabs(v5) / 180.0) ^ _mask__NegFloat_) * rate);
        v4 = rate - (float)((-(fabs(v5) / 180.0)) * rate);
    }
    else
        v4 = rate;
    v6 = DiffTrackAngle(targetPitch, currentPitch, v4, 0.050000001);
    //if ( g_DXDeviceThread == GetCurrentThreadId() )
        //D3DPERF_EndEvent();
    return v6;
}

double __cdecl Bot_UpdateYaw(float currentYaw, float targetYaw, bool forceSlow, bot_info_t *botInfo)
{
    __int16 v4; // cx
    float v6; // [esp+10h] [ebp-2Ch]
    float v7; // [esp+14h] [ebp-28h]
    float v8; // [esp+20h] [ebp-1Ch]
    float v9; // [esp+28h] [ebp-14h]
    float rate; // [esp+30h] [ebp-Ch]
    float ratea; // [esp+30h] [ebp-Ch]
    float rateb; // [esp+30h] [ebp-Ch]
    float diff; // [esp+34h] [ebp-8h]

    //PIXBeginNamedEvent(-1, "Bot_UpdateYaw");
    rate = sv_botYawSpeed->current.value;
    if ( forceSlow )
        rate = sv_botYawSpeedAds->current.value;
    v8 = AngleNormalize180(targetYaw - currentYaw);
    diff = fabs(v8);
    if ( fabs(v8) < 100.0 )
    {
        //if ( (float)((float)(rate - (float)(COERCE_FLOAT(COERCE_UNSIGNED_INT(diff / 180.0) ^ _mask__NegFloat_) * rate)) - rate) < 0.0 )
        if ((float)((float)(rate - (float)((-(diff / 180.0)) * rate)) - rate) < 0.0)
        {
            //v6 = rate - (float)(COERCE_FLOAT(COERCE_UNSIGNED_INT(diff / 180.0) ^ _mask__NegFloat_) * rate);
            v6 = rate - (float)((-(diff / 180.0)) * rate);
        }
        else
            v6 = rate;
        rateb = v6;
    }
    else
    {
        ratea = rate - (float)((float)(diff / 180.0) * rate);
        if ( (float)(1.0 - ratea) < 0.0 )
            v7 = ratea;
        else
            v7 = 1.0f;
        rateb = v7;
    }
    if ( diff < 50.0 )
        v4 = botInfo->flags & 0xFFEF;
    else
        v4 = botInfo->flags | 0x10;
    botInfo->flags = v4;
    v9 = DiffTrackAngle(targetYaw, currentYaw, rateb, 0.050000001);
    //if ( g_DXDeviceThread == GetCurrentThreadId() )
        //D3DPERF_EndEvent();
    return v9;
}

void __cdecl Bot_UpdateMantle(bot_info_t *botInfo, const client_t *bot, usercmd_s *cmd)
{
    gclient_s *ps; // [esp+24h] [ebp-4h]

    //PIXBeginNamedEvent(-1, "Bot_UpdateMantle");
    ps = G_GetPlayerState(bot->gentity->s.number);
    if ( (ps->ps.pm_flags & 4) != 0 || (ps->ps.pm_flags & 0x4000) != 0 )
    {
        cmd->forwardmove = 127;
        cmd->rightmove = 0;
        botInfo->path.wNegotiationStartNode = 0;
        botInfo->flags &= ~0x80u;
        botInfo->flags &= ~0x100u;
        botInfo->flags &= ~0x200u;
        //if ( GetCurrentThreadId() != g_DXDeviceThread )
        //    return;
LABEL_22:
        //D3DPERF_EndEvent();
        return;
    }
    if ( !Bot_IsAtNegotiationNode(botInfo, bot, &botInfo->path) )
    {
        //if ( g_DXDeviceThread != GetCurrentThreadId() )
        //    return;
        goto LABEL_22;
    }
    if ( (botInfo->flags & 0x100) == 0 )
    {
        cmd->forwardmove = 127;
        cmd->rightmove = 0;
        cmd->button_bits.resetBit(9);
        cmd->button_bits.resetBit(8);
        if ( (ps->ps.mantleState.flags & 0x10) != 0 )
            goto LABEL_20;
        if ( bot->lastUsercmd.forwardmove > 0 && !Bot_IsMovingFoward(botInfo, bot, &ps->ps) && (botInfo->flags & 0x200) != 0 )
        {
            botInfo->flags &= ~0x200u;
            cmd->button_bits.setBit(2);
            goto LABEL_21;
        }
        if ( bot->lastUsercmd.forwardmove > 0 && !Bot_IsMovingFoward(botInfo, bot, &ps->ps) && !ps->ps.weaponstate )
LABEL_20:
            cmd->button_bits.setBit(0xAu);
LABEL_21:
        //if ( g_DXDeviceThread != GetCurrentThreadId() )
        //    return;
        goto LABEL_22;
    }
    cmd->forwardmove = 0;
    cmd->rightmove = 0;
    //if ( GetCurrentThreadId() == g_DXDeviceThread )
        goto LABEL_22;
}

void __cdecl Bot_UpdateSpeed(bot_info_t *botInfo, const client_t *bot, usercmd_s *cmd)
{
    bool v4; // [esp+8h] [ebp-20h]
    bool v5; // [esp+Ch] [ebp-1Ch]
    const WeaponVariantDef *weapVariantDef; // [esp+1Ch] [ebp-Ch]
    gclient_s *ps; // [esp+20h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+24h] [ebp-4h]

    //PIXBeginNamedEvent(-1, "Bot_UpdateSpeed");
    ps = G_GetPlayerState(bot->gentity->s.number);
    weapDef = BG_GetWeaponDef(ps->ps.weapon);
    weapVariantDef = BG_GetWeaponVariantDef(ps->ps.weapon);
    if ( botInfo->sightHitNum || !BG_WeaponAmmo(&ps->ps, ps->ps.weapon) )
    {
        if ( (botInfo->flags & 4) != 0
            && Bot_PointInGoal(bot->gentity->r.currentOrigin, &botInfo->path, botInfo->scriptGoalRadius) )
        {
            cmd->forwardmove = 0;
            cmd->rightmove = 0;
        }
        else
        {
            cmd->forwardmove = 127;
            cmd->rightmove = 0;
        }
    }
    else
    {
        v5 = weapDef->bDualWield || weapDef->impactType == IMPACT_TYPE_SHOTGUN;
        if ( !v5
            && botInfo->threat.distSq > (float)(sv_botCloseDistance->current.value * sv_botCloseDistance->current.value) )
        {
            Bot_SetTimedAction(0xBu, &botInfo->adsEndTime, sv_botMinAdsTime, sv_botMaxAdsTime, cmd, &bot->lastUsercmd);
        }
        if ( botInfo->threat.distSq > (float)(weapDef->fMinDamageRange * weapDef->fMinDamageRange) )
            cmd->forwardmove = 127;
    }
    if ( botInfo->adsEndTime )
        cmd->button_bits.setBit(0xBu);
    if ( weapVariantDef->overlayMaterial && cmd->button_bits.testBit(0xBu) )
    {
        cmd->forwardmove = 0;
        cmd->rightmove = 0;
    }
    Bot_UpdateSprint(botInfo, bot, cmd);
    Bot_UpdateStance(botInfo, bot, cmd);
    if ( Bot_IsInLastStand(bot) )
    {
        cmd->forwardmove = 0;
        cmd->rightmove = 0;
    }
    if ( (botInfo->flags & 0x10) != 0 )
    {
        cmd->forwardmove = 0;
        cmd->rightmove = 0;
        cmd->button_bits.resetBit(1);
    }
    if ( !botInfo->sightHitNum )
    {
        v4 = weapDef->bDualWield || weapDef->impactType == IMPACT_TYPE_SHOTGUN;
        if ( v4 && botInfo->threat.distSq > (float)(weapDef->fMaxDamageRange * weapDef->fMaxDamageRange) )
        {
            if ( Path_Exists(&botInfo->path) )
            {
                Bot_StrafeOnPath(&ps->ps, &botInfo->path, cmd);
            }
            else
            {
                cmd->forwardmove = 127;
                cmd->rightmove = 0;
            }
        }
    }
    if ( botInfo->threat.enemy )
    {
        if ( botInfo->sightHitNum && botInfo->threat.lastSightTime && svs.time - botInfo->threat.lastSightTime < 5000 )
            Bot_StrafeOnPath(&ps->ps, &botInfo->path, cmd);
    }
    //if ( g_DXDeviceThread == GetCurrentThreadId() )
        //D3DPERF_EndEvent();
}

void __cdecl Bot_StrafeOnPath(const playerState_s *ps, const path_t *path, usercmd_s *cmd)
{
    float vDir[3]; // [esp+0h] [ebp-30h] BYREF
    float axis[3][3]; // [esp+Ch] [ebp-24h] BYREF

    AnglesToAxis(ps->viewangles, axis);
    Vec3Rotate(path->lookaheadDir, axis, vDir);
    cmd->forwardmove = (int)(float)(127.0 * vDir[0]);
    cmd->rightmove = (int)(float)(-127.0 * vDir[1]);
}

void __cdecl Bot_SetTimedAction(
                unsigned int button,
                int *timedAction,
                const dvar_s *minTime,
                const dvar_s *maxTime,
                usercmd_s *cmd,
                const usercmd_s *lastCmd)
{
    cmd->button_bits.setBit(button);
    if ( !lastCmd->button_bits.testBit(button) )
        *timedAction = svs.time + irand(minTime->current.integer, maxTime->current.integer);
}

void __cdecl Bot_UpdateStance(bot_info_t *botInfo, const client_t *bot, usercmd_s *cmd)
{
    gclient_s *ps; // [esp+10h] [ebp-4h]

    //PIXBeginNamedEvent(-1, "Bot_UpdateStance");
    if ( Path_Exists(&botInfo->path)
        && !Bot_IsAtNegotiationNode(botInfo, bot, &botInfo->path)
        && sv_botCrouchDistance->current.value > botInfo->path.fLookaheadDist
        && random() < 0.050000001 )
    {
        if ( svs.time >= botInfo->lastMoveTime )
            Bot_SetTimedAction(9u, &botInfo->crouchEndTime, sv_botMinCrouchTime, sv_botMaxCrouchTime, cmd, &bot->lastUsercmd);
        if ( random() < 0.2 )
            Bot_SetTimedAction(0xBu, &botInfo->adsEndTime, sv_botMinAdsTime, sv_botMaxAdsTime, cmd, &bot->lastUsercmd);
    }
    if ( botInfo->crouchEndTime )
        cmd->button_bits.setBit(9);
    else
        cmd->button_bits.resetBit(9);
    cmd->button_bits.resetBit(8);
    ps = G_GetPlayerState(bot - svs.clients);
    if ( botInfo->sightHitNum
        && !Bot_IsMovingFoward(botInfo, bot, &ps->ps)
        && !Bot_IsAtNegotiationNode(botInfo, bot, &botInfo->path)
        && Path_Exists(&botInfo->path)
        && !Bot_PointInGoal(bot->gentity->r.currentOrigin, &botInfo->path, 0.0)
        && svs.time - botInfo->lastMoveTime >= 250 )
    {
        Bot_SetTimedAction(9u, &botInfo->crouchEndTime, sv_botMinCrouchTime, sv_botMaxCrouchTime, cmd, &bot->lastUsercmd);
    }
    //if ( g_DXDeviceThread == GetCurrentThreadId() )
        //D3DPERF_EndEvent();
}

void __cdecl Bot_UpdateSprint(bot_info_t *botInfo, const client_t *bot, usercmd_s *cmd)
{
    char v3; // [esp+4h] [ebp-5Ch]
    float angles[3]; // [esp+48h] [ebp-18h] BYREF
    const WeaponDef *weapDef; // [esp+54h] [ebp-Ch]
    const playerState_s *ps; // [esp+58h] [ebp-8h]
    float distSq; // [esp+5Ch] [ebp-4h] BYREF

    //PIXBeginNamedEvent(-1, "Bot_UpdateSprint");
    ps = (const playerState_s *)G_GetPlayerState(bot->gentity->s.number);
    if ( cmd->button_bits.testBit(0xB) )
    {
        //if ( g_DXDeviceThread != GetCurrentThreadId() )
        //    return;
        goto LABEL_12;
    }
    if ( ps->weaponstate != 10
        && ps->weaponstate != 12
        && ps->weaponstate != 14
        && ps->weaponstate != 13
        && ps->weaponstate != 11
        && ps->weaponstate != 15
        && ps->weaponstate != 16 )
    {
        weapDef = BG_GetWeaponDef(ps->weapon);
        if ( !botInfo->sightHitNum
            && (weapDef->bDualWield || weapDef->impactType == IMPACT_TYPE_SHOTGUN ? (v3 = 1) : (v3 = 0),
                    v3 && !Bot_IsInRangeOfEnemy(bot, botInfo->threat.enemy, &distSq))
            || botInfo->sightHitNum )
        {
            if ( (botInfo->flags & 0x10) == 0 )
            {
                vectoangles(botInfo->path.lookaheadDir, angles);
                AngleNormalize180(ps->viewangles[1] - angles[1]);
                if ( Path_Exists(&botInfo->path)
                    && botInfo->path.fLookaheadDist >= sv_botSprintDistance->current.value
                    && svs.time >= botInfo->lastMoveTime )
                {
                    cmd->button_bits.setBit(1);
                }
                if ( (ps->pm_flags & 0x8000) != 0 && ps->damageCount > 1 && random() < 0.30000001 )
                    cmd->button_bits.setBit(0x2C);
                //if ( g_DXDeviceThread == GetCurrentThreadId() )
                    //D3DPERF_EndEvent();
                return;
            }
            //if ( g_DXDeviceThread != GetCurrentThreadId() )
            //    return;
        }
        //else if ( g_DXDeviceThread != GetCurrentThreadId() )
        //{
        //    return;
        //}
        //D3DPERF_EndEvent();
        return;
    }
    //if ( GetCurrentThreadId() == g_DXDeviceThread )
LABEL_12:
    ;
        //D3DPERF_EndEvent();
}

bool __cdecl Bot_IsFlashbanged(const client_t *bot)
{
    gclient_s *ps; // [esp+4h] [ebp-4h]

    ps = G_GetPlayerState(bot->gentity->s.number);
    return (ps->ps.pm_flags & 0x10000) != 0
            && svs.time < ps->ps.shellshockDuration + ps->ps.shellshockTime
            && BG_GetShellshockParms(ps->ps.shellshockIndex)->screenBlend.type == SHELLSHOCK_VIEWTYPE_FLASHED;
}

void __cdecl Bot_UpdateReload(bot_info_t *botInfo, const client_t *bot, usercmd_s *cmd)
{
    int v3; // [esp+0h] [ebp-3Ch]
    int v4; // [esp+4h] [ebp-38h]
    int maxClip; // [esp+28h] [ebp-14h]
    int curClip; // [esp+30h] [ebp-Ch]
    gclient_s *ps; // [esp+34h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+38h] [ebp-4h]

    //PIXBeginNamedEvent(-1, "Bot_UpdateReload");
    ps = G_GetPlayerState(bot->gentity->s.number);
    if ( ps->ps.weapon && !ps->ps.weaponstate && !BG_WeaponIsClipOnly(ps->ps.weapon) )
    {
        weapDef = BG_GetWeaponDef(ps->ps.weapon);
        curClip = BG_GetAmmoInClip(&ps->ps, ps->ps.weapon);
        v4 = curClip > 999 ? 999 : curClip;
        maxClip = BG_GetClipSize(ps->ps.weapon);
        v3 = maxClip > 999 ? 999 : maxClip;
        if ( (float)((float)v3 * weapDef->lowAmmoWarningThreshold) >= (float)v4 && (botInfo->sightHitNum || random() < 0.1) )
            cmd->button_bits.setBit(5);
    }
    //if ( GetCurrentThreadId() == g_DXDeviceThread )
        //D3DPERF_EndEvent();
}

void __cdecl Bot_UpdateGlassSmash(const client_t *bot, usercmd_s *cmd)
{
    if ( !(svs.time % 1000) )
    {
        if ( g_pmove[bot->gentity->s.number].numGlassTouch )
            cmd->button_bits.setBit(2);
    }
}

void __cdecl Bot_UpdateCombat(bot_info_t *botInfo, const client_t *bot, usercmd_s *cmd)
{
    bool v3; // [esp+0h] [ebp-84h]
    char v4; // [esp+4h] [ebp-80h]
    bool v5; // [esp+8h] [ebp-7Ch]
    const gentity_s *v6; // [esp+14h] [ebp-70h]
    const gentity_s *v7; // [esp+18h] [ebp-6Ch]
    const gentity_s *enemy; // [esp+24h] [ebp-60h]
    gclient_s *ps; // [esp+7Ch] [ebp-8h]
    const WeaponDef *weapDef; // [esp+80h] [ebp-4h]

    //PIXBeginNamedEvent(-1, "Bot_UpdateCombat");
    ps = G_GetPlayerState(bot->gentity->s.number);
    if ( Bot_IsFlashbanged(bot) )
    {
        if ( random() < 0.0099999998 )
            cmd->button_bits.setBit(2);
        //if ( GetCurrentThreadId() != g_DXDeviceThread )
        //    return;
LABEL_98:
        //D3DPERF_EndEvent();
        return;
    }
    if ( ps->ps.throwBackGrenadeTimeLeft > 0 && random() < 0.89999998 )
    {
        cmd->button_bits.resetBit(0);
        cmd->button_bits.resetBit(0xB);
        cmd->button_bits.resetBit(1);
        cmd->button_bits.setBit(0xE);
        //if ( g_DXDeviceThread != GetCurrentThreadId() )
        //    return;
        goto LABEL_98;
    }
    if ( Bot_UpdateGrenadeThrow(botInfo, bot, cmd) )
    {
        //if ( GetCurrentThreadId() != g_DXDeviceThread )
        //    return;
        goto LABEL_98;
    }
    if ( botInfo->sightHitNum )
    {
        //if ( GetCurrentThreadId() != g_DXDeviceThread )
        //    return;
        goto LABEL_98;
    }
    if ( botInfo->threat.enemy->health <= 0 )
    {
        //if ( g_DXDeviceThread != GetCurrentThreadId() )
        //    return;
        goto LABEL_98;
    }
    if ( Bot_UpdateMelee(botInfo, bot, cmd) )
    {
        //if ( GetCurrentThreadId() != g_DXDeviceThread )
        //    return;
        goto LABEL_98;
    }
    if ( !sv_botsPressAttackBtn->current.enabled )
    {
        //if ( g_DXDeviceThread != GetCurrentThreadId() )
        //    return;
        goto LABEL_98;
    }
    weapDef = BG_GetWeaponDef(ps->ps.weapon);
    if ( (botInfo->flags & 0x20) != 0 || !weapDef->bDualWield && ps->ps.fWeaponPosFrac < 1.0 )
    {
        if ( botInfo->threat.distSq <= (float)(sv_botCloseDistance->current.value * sv_botCloseDistance->current.value) )
            botInfo->threat.reactionEndTime = svs.time;
        else
            botInfo->threat.reactionEndTime = svs.time
                                                                            + irand(
                                                                                    sv_botMinReactionTime->current.integer,
                                                                                    sv_botMaxReactionTime->current.integer);
    }
    if ( svs.time < botInfo->threat.reactionEndTime )
    {
        //if ( g_DXDeviceThread != GetCurrentThreadId() )
        //    return;
        goto LABEL_98;
    }
    if ( !BG_WeaponAmmo(&ps->ps, ps->ps.weapon) )
    {
        //if ( g_DXDeviceThread != GetCurrentThreadId() )
        //    return;
        goto LABEL_98;
    }
    if ( weapDef->fireType && weapDef->fireType != WEAPON_FIRETYPE_MINIGUN && svs.time < botInfo->weaponDelayEndTime )
    {
        //if ( GetCurrentThreadId() != g_DXDeviceThread )
        //    return;
        goto LABEL_98;
    }
    enemy = botInfo->threat.enemy;
    v5 = enemy && enemy->scr_vehicle;
    if ( v5 && weapDef->lockOnSpeed )
    {
        if ( !botInfo->threat.acquireTime && ps->ps.fWeaponPosFrac >= 1.0 )
            botInfo->threat.acquireTime = svs.time;
        if ( svs.time - botInfo->threat.acquireTime < weapDef->lockOnSpeed + 500 )
        {
            //if ( g_DXDeviceThread != GetCurrentThreadId() )
            //    return;
            goto LABEL_98;
        }
        botInfo->threat.acquireTime = svs.time;
        cmd->button_bits.setBit(0);
    }
    if ( !ps->ps.weaponstate )
    {
        if ( sv_botsForceFragOnly->current.enabled )
        {
            if ( random() < 0.0099999998 )
                cmd->button_bits.setBit(0xE);
        }
        else if ( sv_botsForceSpecialOnly->current.enabled )
        {
            if ( random() < 0.0099999998 )
                cmd->button_bits.setBit(0xF);
        }
        else
        {
            if ( random() < 0.0049999999 || sv_botsForceFragOnly->current.enabled )
                goto LABEL_87;
            if ( random() < 0.0049999999
                || sv_botsForceSpecialOnly->current.enabled
                || ((v7 = botInfo->threat.enemy) == 0 || !v7->pTurretInfo ? (v4 = 0) : (v4 = 1),
                        v4 && botInfo->threat.distSq < 4194304.0 && random() < 0.050000001) )
            {
                cmd->button_bits.setBit(0xF);
                goto LABEL_95;
            }
            v6 = botInfo->threat.enemy;
            v3 = v6 && v6->pTurretInfo;
            if ( v3 && botInfo->threat.distSq < 4194304.0 && random() < 0.050000001 )
            {
LABEL_87:
                cmd->button_bits.setBit(0xE);
            }
            else if ( cmd->button_bits.testBit(0xB) && ps->ps.fWeaponPosFrac >= 1.0 )
            {
                cmd->button_bits.setBit(0);
            }
            else if ( !cmd->button_bits.testBit(0xB) )
            {
                cmd->button_bits.setBit(0);
                if ( weapDef->bDualWield && random() < 0.89999998 )
                    cmd->button_bits.setBit(0x18);
            }
        }
    }
LABEL_95:
    if ( cmd->button_bits.testBit(0) )
        botInfo->weaponDelayEndTime = svs.time
                                                                + irand(sv_botMinFireTime->current.integer, sv_botMaxFireTime->current.integer);
    //if ( g_DXDeviceThread == GetCurrentThreadId() )
        goto LABEL_98;
}

char __cdecl Bot_UpdateMelee(bot_info_t *botInfo, const client_t *bot, usercmd_s *cmd)
{
    //PIXBeginNamedEvent(-1, "Bot_UpdateMelee");
    if ( sv_botsPressMeleeBtn->current.enabled )
    {
        if ( botInfo->sightHitNum )
        {
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                //D3DPERF_EndEvent();
            return 0;
        }
        else if ( (float)(40.0 * 40.0) <= botInfo->threat.distSq || random() >= 0.75 )
        {
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                //D3DPERF_EndEvent();
            return 0;
        }
        else
        {
            cmd->button_bits.setBit(2);
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                //D3DPERF_EndEvent();
            return 1;
        }
    }
    else
    {
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
        return 0;
    }
}

char __cdecl Bot_UpdateGrenadeThrow(bot_info_t *botInfo, const client_t *bot, usercmd_s *cmd)
{
    double v5; // st7
    double v6; // st7
    double v7; // st7
    float pitchDiff; // [esp+78h] [ebp-24h]
    float range; // [esp+80h] [ebp-1Ch]
    OffhandSlot offhandSlot; // [esp+84h] [ebp-18h] BYREF
    float desiredPitch; // [esp+88h] [ebp-14h] BYREF
    float heightDiff; // [esp+8Ch] [ebp-10h]
    const playerState_s *ps; // [esp+90h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+94h] [ebp-8h]
    int offHandIndex; // [esp+98h] [ebp-4h]

    //PIXBeginNamedEvent(-1, "Bot_UpdateGrenadeThrow");
    if (sv_botsPressAttackBtn->current.enabled)
    {
        ps = (const playerState_s *)G_GetPlayerState(bot->gentity->s.number);
        weapDef = BG_GetWeaponDef(ps->offHandIndex);
        if (Bot_IsThrowingGrenade(botInfo, bot))
        {
            cmd->forwardmove = 0;
            cmd->rightmove = 0;
            botInfo->grenadeTime = svs.time + 2000;
        }
        if ((botInfo->flags & 0x400) != 0 || (botInfo->flags & 0x800) != 0)
        {
            pitchDiff = AngleNormalize180(ps->viewangles[0] - botInfo->targetPitch);
            cmd->button_bits.setBit(15 - ((botInfo->flags & 0x400) != 0));
            if (pitchDiff < 5.0
                && pitchDiff > -5.0
                && ps->weaponstate >= 22
                && (ps->damageCount >= 1 || !weapDef->bCookOffHold || ps->grenadeTimeLeft <= irand(1700, 2500)))
            {
                cmd->button_bits.resetBit(15 - ((botInfo->flags & 0x400) != 0));
                botInfo->flags &= ~0x400u;
                botInfo->flags &= ~0x800u;
            }
            cmd->button_bits.resetBit(0xB);
            //if (GetCurrentThreadId() == g_DXDeviceThread)
            //    D3DPERF_EndEvent();
            return 1;
        }
        else if (Bot_ShouldThrowGrenade(botInfo, bot))
        {
            if (Bot_GrenadePickType(botInfo, ps, &offhandSlot))
            {
                range = Vec3Distance(bot->gentity->r.currentOrigin, botInfo->threat.enemy->r.currentOrigin);
                offHandIndex = BG_GetFirstAvailableOffhand(ps, offhandSlot);
                weapDef = BG_GetWeaponDef(offHandIndex);
                if (offhandSlot == OFFHAND_SLOT_LETHAL_GRENADE && (float)weapDef->iExplosionRadius > range)
                {
                    //if (g_DXDeviceThread == GetCurrentThreadId())
                    //    D3DPERF_EndEvent();
                    return 0;
                }
                else
                {
                    heightDiff = botInfo->threat.enemy->r.currentOrigin[2] - bot->gentity->r.currentOrigin[2];
                    if (Bot_GrenadeInRange(range, heightDiff, ps->viewangles, weapDef, &desiredPitch))
                    {
                        if (weapDef->rotateType == WEAPROTATE_BLADE_ROTATE)
                        {
                            v5 = flrand(1.0, 3.0);
                            desiredPitch = v5 + desiredPitch;
                        }
                        else if (heightDiff <= -5.0 || heightDiff >= 5.0)
                        {
                            v7 = flrand(7.0, 10.0);
                            desiredPitch = v7 + desiredPitch;
                        }
                        else
                        {
                            v6 = flrand(10.0, 13.0);
                            desiredPitch = v6 + desiredPitch;
                        }
                        if (Bot_GrenadeSightTrace(bot, (float)weapDef->iProjectileSpeed, desiredPitch))
                        {
                            botInfo->targetPitch = desiredPitch;
                            botInfo->pitchEndTime = 0;
                            cmd->button_bits.resetBit(0xB);
                            if (offhandSlot == OFFHAND_SLOT_LETHAL_GRENADE)
                            {
                                botInfo->flags |= 0x400u;
                                cmd->button_bits.setBit(0xE);
                            }
                            else
                            {
                                botInfo->flags |= 0x800u;
                                cmd->button_bits.setBit(0xF);
                            }
                            //if (g_DXDeviceThread == GetCurrentThreadId())
                                //D3DPERF_EndEvent();
                            return 1;
                        }
                        else
                        {
                            botInfo->grenadeTime = svs.time + 2000;
                            //if (g_DXDeviceThread == GetCurrentThreadId())
                                //D3DPERF_EndEvent();
                            return 0;
                        }
                    }
                    else
                    {
                        //if (g_DXDeviceThread == GetCurrentThreadId())
                            //D3DPERF_EndEvent();
                        return 0;
                    }
                }
            }
            else
            {
                botInfo->grenadeTime = svs.time + 2000;
                //if (GetCurrentThreadId() == g_DXDeviceThread)
                    //D3DPERF_EndEvent();
                return 0;
            }
        }
        else
        {
            //if (GetCurrentThreadId() == g_DXDeviceThread)
                //D3DPERF_EndEvent();
            return 0;
        }
    }
    else
    {
        //if (GetCurrentThreadId() == g_DXDeviceThread)
        //    D3DPERF_EndEvent();
        return 0;
    }
}

char __cdecl Bot_GrenadeSightTrace(const client_t *bot, float projectileSpeed, float throwPitch)
{
    gentity_s *gentity; // edx
    col_context_t context; // [esp+1Ch] [ebp-64h] BYREF
    float vEnd[3]; // [esp+48h] [ebp-38h] BYREF
    int hit; // [esp+54h] [ebp-2Ch] BYREF
    float angles[3]; // [esp+58h] [ebp-28h] BYREF
    float vForward[3]; // [esp+64h] [ebp-1Ch] BYREF
    const playerState_s *ps; // [esp+70h] [ebp-10h]
    float vStart[3]; // [esp+74h] [ebp-Ch] BYREF

    //PIXBeginNamedEvent(-1, "Bot_GrenadeSightTrace");
    //col_context_t::col_context_t(&context);
    ps = (const playerState_s *)G_GetPlayerState(bot->gentity->s.number);
    angles[0] = ps->viewangles[0];
    angles[1] = ps->viewangles[1];
    angles[2] = ps->viewangles[2];
    gentity = bot->gentity;
    vStart[0] = gentity->r.currentOrigin[0];
    vStart[1] = gentity->r.currentOrigin[1];
    vStart[2] = gentity->r.currentOrigin[2];
    angles[0] = throwPitch;
    vStart[2] = vStart[2] + ps->viewHeightCurrent;
    AngleVectors(angles, vForward, 0, 0);
    vEnd[0] = (float)(384.0 * vForward[0]) + vStart[0];
    vEnd[1] = (float)(384.0 * vForward[1]) + vStart[1];
    vEnd[2] = (float)(384.0 * vForward[2]) + vStart[2];
    context.mask = 1;
    if ( SV_SightTracePoint(&hit, vStart, vEnd, &context) )
    {
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
        return 1;
    }
    else
    {
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
        return 0;
    }
}

char __cdecl Bot_GrenadeInRange(
                float range,
                float height,
                const float *angles,
                const WeaponDef *weapDef,
                float *outPitch)
{
    double v6; // xmm0_8
    long double v7; // [esp+0h] [ebp-78h]
    float iProjectileSpeedForward; // [esp+20h] [ebp-58h]
    float iProjectileSpeedRelativeUp; // [esp+2Ch] [ebp-4Ch]
    float iProjectileSpeed; // [esp+30h] [ebp-48h]
    float vTossVel; // [esp+38h] [ebp-40h]
    float vTossVel_4; // [esp+3Ch] [ebp-3Ch]
    float vTossVel_8; // [esp+40h] [ebp-38h]
    float forwardHoriz[3]; // [esp+44h] [ebp-34h] BYREF
    float g; // [esp+50h] [ebp-28h]
    float denominator; // [esp+54h] [ebp-24h]
    float launchSpeedSq; // [esp+58h] [ebp-20h]
    float vForward[3]; // [esp+5Ch] [ebp-1Ch] BYREF
    float numerator; // [esp+68h] [ebp-10h]
    float vUp[3]; // [esp+6Ch] [ebp-Ch] BYREF

    //PIXBeginNamedEvent(-1, "Bot_GrenadeInRange");
    AngleVectors(angles, vForward, 0, vUp);
    iProjectileSpeed = (float)weapDef->iProjectileSpeed;
    iProjectileSpeedRelativeUp = (float)weapDef->iProjectileSpeedRelativeUp;
    vTossVel = (float)(iProjectileSpeedRelativeUp * vUp[0]) + (float)(iProjectileSpeed * vForward[0]);
    vTossVel_4 = (float)(iProjectileSpeedRelativeUp * vUp[1]) + (float)(iProjectileSpeed * vForward[1]);
    vTossVel_8 = (float)weapDef->iProjectileSpeedUp
                         + (float)((float)(iProjectileSpeedRelativeUp * vUp[2]) + (float)(iProjectileSpeed * vForward[2]));
    if ( weapDef->iProjectileSpeedForward )
    {
        forwardHoriz[0] = vForward[0];
        forwardHoriz[1] = vForward[1];
        Vec2NormalizeFast(forwardHoriz);
        iProjectileSpeedForward = (float)weapDef->iProjectileSpeedForward;
        vTossVel = (float)(iProjectileSpeedForward * forwardHoriz[0]) + vTossVel;
        vTossVel_4 = (float)(iProjectileSpeedForward * forwardHoriz[1]) + vTossVel_4;
    }
    launchSpeedSq = (float)((float)(vTossVel * vTossVel) + (float)(vTossVel_4 * vTossVel_4))
                                + (float)(vTossVel_8 * vTossVel_8);
    launchSpeedSq = (float)weapDef->iProjectileSpeed * (float)weapDef->iProjectileSpeed;
    g = bg_gravity->current.value;
    numerator = launchSpeedSq
                        - sqrtf(
                                (float)(launchSpeedSq * launchSpeedSq)
                            - (float)((float)((float)((float)(range * range) * g) + (float)((float)(2.0 * height) * launchSpeedSq)) * g));
    denominator = g * range;
    if ( (float)(g * range) == 0.0 )
    {
        *outPitch = 0.0f;
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
        return 0;
    }
    else
    {
        v6 = (float)(numerator / denominator);
        //__libm_sse2_atan(v7);
        //*(float *)&v6 = v6;
        *outPitch = atan(v6);//*(float *)&v6;
        if ( (*(unsigned int *)outPitch & 0x7F800000) == 0x7F800000 )
        {
            *outPitch = 0.0f;
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                //D3DPERF_EndEvent();
            return 0;
        }
        else
        {
            *outPitch = (float)(*outPitch * 57.295776) * -1.0;
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                //D3DPERF_EndEvent();
            return 1;
        }
    }
}

char __cdecl Bot_GrenadePickType(const bot_info_t *botInfo, const playerState_s *ps, OffhandSlot *slot)
{
    bool v4; // [esp+0h] [ebp-24h]
    const gentity_s *enemy; // [esp+14h] [ebp-10h]
    unsigned int offHandIndex; // [esp+20h] [ebp-4h]

    //PIXBeginNamedEvent(-1, "Bot_GrenadePickType");
    *slot = OFFHAND_SLOT_LETHAL_GRENADE;
    offHandIndex = BG_GetFirstAvailableOffhand(ps, *slot);
    if ( BG_GetWeaponDef(offHandIndex)->rotateType == WEAPROTATE_BLADE_ROTATE && botInfo->sightHitNum )
        offHandIndex = 0;
    if ( !offHandIndex && botInfo->sightHitNum )
    {
        *slot = OFFHAND_SLOT_TACTICAL_GRENADE;
        offHandIndex = BG_GetFirstAvailableOffhand(ps, *slot);
    }
    if ( !offHandIndex && !botInfo->sightHitNum )
    {
        enemy = botInfo->threat.enemy;
        v4 = enemy && enemy->pTurretInfo;
        if ( v4 || random() < 0.0099999998 )
        {
            *slot = OFFHAND_SLOT_TACTICAL_GRENADE;
            offHandIndex = BG_GetFirstAvailableOffhand(ps, *slot);
        }
    }
    if ( offHandIndex )
    {
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
        return 1;
    }
    else
    {
        *slot = OFFHAND_SLOT_NONE;
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
        return 0;
    }
}

char __cdecl Bot_ShouldThrowGrenade(bot_info_t *botInfo, const client_t *bot)
{
    int AmmoInClip; // eax
    bool v4; // [esp+0h] [ebp-CCh]
    bool v5; // [esp+4h] [ebp-C8h]
    char v6; // [esp+8h] [ebp-C4h]
    const gentity_s *v8; // [esp+60h] [ebp-6Ch]
    const gentity_s *enemy; // [esp+84h] [ebp-48h]
    bool noAmmo; // [esp+BBh] [ebp-11h]
    gclient_s *ps; // [esp+BCh] [ebp-10h]
    const WeaponDef *weapDef; // [esp+C0h] [ebp-Ch]
    float dot; // [esp+C4h] [ebp-8h] BYREF
    float distSq; // [esp+C8h] [ebp-4h] BYREF

    //PIXBeginNamedEvent(-1, "Bot_ShouldThrowGrenade");
    if ( sv_botAllowGrenades->current.enabled )
    {
        ps = G_GetPlayerState(bot->gentity->s.number);
        if ( ps->ps.weaponstate )
        {
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                //D3DPERF_EndEvent();
            return 0;
        }
        else if ( botInfo->threat.enemy )
        {
            if ( botInfo->threat.enemy->health > 0 )
            {
                enemy = botInfo->threat.enemy;
                if ( enemy && enemy->scr_vehicle )
                {
                    //if ( g_DXDeviceThread == GetCurrentThreadId() )
                        //D3DPERF_EndEvent();
                    return 0;
                }
                else if ( !botInfo->sightHitNum
                             || (!botInfo->threat.lastSightTime || svs.time - botInfo->threat.lastSightTime >= 5000
                                 ? (v6 = 0)
                                 : (v6 = 1),
                                     v6 || ps->ps.damageCount >= 1) )
                {
                    if ( Bot_IsFacingEnemy(bot, botInfo->threat.enemy, &dot) )
                    {
                        if ( svs.time >= botInfo->grenadeTime )
                        {
                            weapDef = BG_GetWeaponDef(ps->ps.weapon);
                            v8 = botInfo->threat.enemy;
                            v5 = v8 && v8->pTurretInfo;
                            if ( !v5 || weapDef->weapType == WEAPTYPE_PROJECTILE )
                            {
                                AmmoInClip = BG_GetAmmoInClip(&ps->ps, ps->ps.weapon);
                                noAmmo = AmmoInClip <= 0;
                                if ( AmmoInClip <= 0
                                    || (float)(sv_botCloseDistance->current.value * sv_botCloseDistance->current.value) <= botInfo->threat.distSq )
                                {
                                    v4 = botInfo->sightHitNum
                                        && botInfo->threat.lastSightTime
                                        && svs.time - botInfo->threat.lastSightTime < 5000;
                                    if ( v4
                                        && svs.time + irand(sv_botMinGrenadeTime->current.integer, sv_botMaxGrenadeTime->current.integer) > botInfo->threat.lastSightTime )
                                    {
                                        if ( random() >= 0.75 )
                                        {
                                            botInfo->grenadeTime = svs.time + 2000;
                                            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                                                //D3DPERF_EndEvent();
                                            return 0;
                                        }
                                        else
                                        {
                                            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                                                //D3DPERF_EndEvent();
                                            return 1;
                                        }
                                    }
                                    else if ( botInfo->sightHitNum
                                                 || Bot_IsInRangeOfEnemy(bot, botInfo->threat.enemy, &distSq)
                                                 || random() >= 0.25 )
                                    {
                                        if ( botInfo->sightHitNum
                                            || ps->ps.fWeaponPosFrac <= 0.0
                                            || noAmmo
                                            || ps->ps.damageCount
                                            || random() >= 0.0049999999 )
                                        {
                                            if ( botInfo->sightHitNum
                                                || ps->ps.fWeaponPosFrac <= 0.0
                                                || !ps->ps.damageCount
                                                || random() >= 0.0099999998 )
                                            {
                                                if ( botInfo->sightHitNum || !noAmmo )
                                                {
                                                    if ( botInfo->sightHitNum && ps->ps.damageCount )
                                                    {
                                                        //if ( g_DXDeviceThread == GetCurrentThreadId() )
                                                            //D3DPERF_EndEvent();
                                                        return 1;
                                                    }
                                                    else
                                                    {
                                                        //if ( g_DXDeviceThread == GetCurrentThreadId() )
                                                            //D3DPERF_EndEvent();
                                                        return 0;
                                                    }
                                                }
                                                else
                                                {
                                                    //if ( g_DXDeviceThread == GetCurrentThreadId() )
                                                        //D3DPERF_EndEvent();
                                                    return 1;
                                                }
                                            }
                                            else
                                            {
                                                //if ( g_DXDeviceThread == GetCurrentThreadId() )
                                                    //D3DPERF_EndEvent();
                                                return 1;
                                            }
                                        }
                                        else
                                        {
                                            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                                                //D3DPERF_EndEvent();
                                            return 1;
                                        }
                                    }
                                    else
                                    {
                                        //if ( g_DXDeviceThread == GetCurrentThreadId() )
                                            //D3DPERF_EndEvent();
                                        return 1;
                                    }
                                }
                                else
                                {
                                    //if ( g_DXDeviceThread == GetCurrentThreadId() )
                                        //D3DPERF_EndEvent();
                                    return 0;
                                }
                            }
                            else
                            {
                                //if ( g_DXDeviceThread == GetCurrentThreadId() )
                                    //D3DPERF_EndEvent();
                                return 1;
                            }
                        }
                        else
                        {
                            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                                //D3DPERF_EndEvent();
                            return 0;
                        }
                    }
                    else
                    {
                        //if ( g_DXDeviceThread == GetCurrentThreadId() )
                            //D3DPERF_EndEvent();
                        return 0;
                    }
                }
                else
                {
                    //if ( g_DXDeviceThread == GetCurrentThreadId() )
                        //D3DPERF_EndEvent();
                    return 0;
                }
            }
            else
            {
                //if ( g_DXDeviceThread == GetCurrentThreadId() )
                    //D3DPERF_EndEvent();
                return 0;
            }
        }
        else
        {
            //if ( g_DXDeviceThread == GetCurrentThreadId() )
                //D3DPERF_EndEvent();
            return 0;
        }
    }
    else
    {
        //if ( g_DXDeviceThread == GetCurrentThreadId() )
            //D3DPERF_EndEvent();
        return 0;
    }
}

void __cdecl Bot_Clear(bot_info_t *botInfo)
{
    //PIXBeginNamedEvent(-1, "Bot_Clear");
    memset((unsigned __int8 *)botInfo, 0, sizeof(bot_info_t));
    Path_Clear(&botInfo->path);
    //if ( g_DXDeviceThread == GetCurrentThreadId() )
        //D3DPERF_EndEvent();
}

void __cdecl SV_BotRegisterDvars()
{
    sv_botsAllowMovement = _Dvar_RegisterBool("sv_botsAllowMovement", 0, 0, "Allow testclients to pathfind and move");
    sv_botsForceStand = _Dvar_RegisterBool("sv_botsForceStand", 0, 0, "Make testclients always stand");
    sv_botsForceCrouch = _Dvar_RegisterBool("sv_botsForceCrouch", 0, 0, "Make testclients always crouch");
    sv_botsForceProne = _Dvar_RegisterBool("sv_botsForceProne", 0, 0, "Make testclients always prone");
    sv_botsPressAttackBtn = _Dvar_RegisterBool("sv_botsPressAttackBtn", 0, 0, "Allow testclients to press attack button");
    sv_botsPressMeleeBtn = _Dvar_RegisterBool("sv_botsPressMeleeBtn", 0, 0, "Allow testclients to press melee button");
    sv_botsForceFragOnly = _Dvar_RegisterBool("sv_botsForceFragOnly", 0, 0, "Force testclients to Frag grenades only");
    sv_botsForceSpecialOnly = _Dvar_RegisterBool(
                                                            "sv_botsForceSpecialOnly",
                                                            0,
                                                            0,
                                                            "Force testclients special frag grenades only");
    sv_botsIgnoreHumans = _Dvar_RegisterBool("sv_botsIgnoreHumans", 0, 0, "Testclients will ignore human players");
    sv_botSnapshotDebug = _Dvar_RegisterBool(
                                                    "sv_botSnapshotDebug",
                                                    0,
                                                    0,
                                                    "Simulate writing/sending snapshots to bots. Used for snapshot debugging.");
    sv_botForceYaw = _Dvar_RegisterInt("sv_botForceYaw", -1, 0, 360, 0, "Debug yaw used by bot when movement is disabled");
    sv_botDebug = _Dvar_RegisterInt("sv_botDebug", -1, -1, 32, 0, "Clientnum of bot to debug");
    sv_botDebugX = _Dvar_RegisterFloat("sv_botDebugX", 600.0, 0.0, 800.0, 0, "X-position of debug overlay");
    sv_botDebugY = _Dvar_RegisterFloat("sv_botDebugY", 0.0, 0.0, 450.0, 0, "Y-position of debug overlay");
    sv_botDebugThreat = _Dvar_RegisterInt("sv_botDebugThreat", -1, -1, 32, 0, "Clientnum of bot to debug threat");
    sv_botDebugPaths = _Dvar_RegisterInt("sv_botDebugPaths", -1, -1, 32, 0, "Clientnum of bot to debug path");
    sv_botFov = _Dvar_RegisterFloat("sv_botFov", 65.0, 1.0, 160.0, 0x80u, "Bot field of view angle in degrees");
    sv_botGoalRadius = _Dvar_RegisterFloat("sv_botGoalRadius", 128.0, 0.0, 4096.0, 0x80u, "Bot goal radius");
    sv_botCloseDistance = _Dvar_RegisterFloat(
                                                    "sv_botCloseDistance",
                                                    256.0,
                                                    0.0,
                                                    4096.0,
                                                    0x80u,
                                                    "Bot enemy close distance. Used for threat selection");
    sv_botSprintDistance = _Dvar_RegisterFloat(
                                                     "sv_botSprintDistance",
                                                     512.0,
                                                     1.0,
                                                     4096.0,
                                                     0x80u,
                                                     "If the path lookahead distance is this number or greater, the bot will sprint");
    sv_botCrouchDistance = _Dvar_RegisterFloat(
                                                     "sv_botCrouchDistance",
                                                     32.0,
                                                     1.0,
                                                     4096.0,
                                                     0x80u,
                                                     "If the path lookahead distance is less than this number, the bot will crouch");
    sv_botTargetLeadBias = _Dvar_RegisterInt(
                                                     "sv_botTargetLeadBias",
                                                     4,
                                                     0,
                                                     25,
                                                     0x80u,
                                                     "The number of server frames used to predict an entity's position. Used for bot aiming when firing.");
    sv_botUseFriendNames = _Dvar_RegisterBool(
                                                     "sv_botUseFriendNames",
                                                     1,
                                                     0,
                                                     "Testclients will use names from the players' friends list");
    sv_botAllowGrenades = _Dvar_RegisterBool("sv_botAllowGrenades", 1, 0, "Allow the bots to throw grenades");
    sv_botMinGrenadeTime = _Dvar_RegisterInt(
                                                     "sv_botMinGrenadeTime",
                                                     500,
                                                     0,
                                                     10000,
                                                     0x80u,
                                                     "The minimum amount of time in ms the bot must lose sight of his enemy before throwing a grenade.");
    sv_botMaxGrenadeTime = _Dvar_RegisterInt(
                                                     "sv_botMaxGrenadeTime",
                                                     1000,
                                                     0,
                                                     10000,
                                                     0x80u,
                                                     "The maximum amount of time in ms the bot must lose sight of his enemy before throwing a grenade.");
    sv_botMinAdsTime = _Dvar_RegisterInt(
                                             "sv_botMinAdsTime",
                                             1000,
                                             0,
                                             10000,
                                             0x80u,
                                             "The minimum amount of time in ms the bot will remain in ADS. Used to prevent repeatedly popping i"
                                             "n and out of ads.");
    sv_botMaxAdsTime = _Dvar_RegisterInt(
                                             "sv_botMaxAdsTime",
                                             1500,
                                             0,
                                             10000,
                                             0x80u,
                                             "The maximum amount of time in ms the bot will remain in ADS. Used to prevent repeatedly popping i"
                                             "n and out of ads.");
    sv_botMinPitchTime = _Dvar_RegisterInt(
                                                 "sv_botMinPitchTime",
                                                 500,
                                                 0,
                                                 10000,
                                                 0x80u,
                                                 "The minimum amount of time in ms the bot will hold his current pitch on the target when engaged");
    sv_botMaxPitchTime = _Dvar_RegisterInt(
                                                 "sv_botMaxPitchTime",
                                                 1000,
                                                 0,
                                                 10000,
                                                 0x80u,
                                                 "The maximum amount of time in ms the bot will hold his current pitch on the target when engaged");
    sv_botMinCrouchTime = _Dvar_RegisterInt(
                                                    "sv_botMinCrouchTime",
                                                    2000,
                                                    0,
                                                    10000,
                                                    0x80u,
                                                    "The minimum amount of time in ms the bot will hold his crouch stance");
    sv_botMaxCrouchTime = _Dvar_RegisterInt(
                                                    "sv_botMaxCrouchTime",
                                                    4000,
                                                    0,
                                                    10000,
                                                    0x80u,
                                                    "The maximum amount of time in ms the bot will hold his crouch stance");
    sv_botMinDeathTime = _Dvar_RegisterInt(
                                                 "sv_botMinDeathTime",
                                                 500,
                                                 0,
                                                 10000,
                                                 0x80u,
                                                 "The minimum amount of time in ms the bot will hold his position while his target is dead");
    sv_botMaxDeathTime = _Dvar_RegisterInt(
                                                 "sv_botMaxDeathTime",
                                                 1000,
                                                 0,
                                                 10000,
                                                 0x80u,
                                                 "The maximum amount of time in ms the bot will hold his position while his target is dead");
    sv_botMinFireTime = _Dvar_RegisterInt(
                                                "sv_botMinFireTime",
                                                400,
                                                0,
                                                2000,
                                                0x80u,
                                                "When firing a non-automatic weapon, the minimum amount of time the bot will delay before firing "
                                                "again. Used to simulate the time to depress and press the controller attack trigger.");
    sv_botMaxFireTime = _Dvar_RegisterInt(
                                                "sv_botMaxFireTime",
                                                600,
                                                0,
                                                2000,
                                                0x80u,
                                                "When firing a non-automatic weapon, the maximum amount of time the bot will delay before firing "
                                                "again. Used to simulate the time to depress and press the controller attack trigger.");
    sv_botMinReactionTime = _Dvar_RegisterInt(
                                                        "sv_botMinReactionTime",
                                                        500,
                                                        0,
                                                        5000,
                                                        0x80u,
                                                        "Bot minimum time to react when first seeing a target");
    sv_botMaxReactionTime = _Dvar_RegisterInt(
                                                        "sv_botMaxReactionTime",
                                                        1000,
                                                        0,
                                                        5000,
                                                        0x80u,
                                                        "Bot maximum time to react when first seeing a target");
    sv_botYawSpeed = _Dvar_RegisterFloat("sv_botYawSpeed", 4.0, 0.0, 100.0, 0x80u, "Bot yaw rate for normal turning");
    sv_botYawSpeedAds = _Dvar_RegisterFloat(
                                                "sv_botYawSpeedAds",
                                                5.0,
                                                0.0,
                                                100.0,
                                                0x80u,
                                                "Bot yaw rate for slower (fine tuning) turning");
    sv_botPitchUp = _Dvar_RegisterFloat("sv_botPitchUp", -10.0, -90.0, 0.0, 0x80u, "Bot pitch maximum (looking upward)");
    sv_botPitchDown = _Dvar_RegisterFloat(
                                            "sv_botPitchDown",
                                            20.0,
                                            0.0,
                                            90.0,
                                            0x80u,
                                            "Bot pitch minimum (looking downward)");
    sv_botPitchSpeed = _Dvar_RegisterFloat("sv_botPitchSpeed", 2.0, 0.0, 100.0, 0x80u, "Bot pitch lerp speed");
    sv_botPitchSpeedAds = _Dvar_RegisterFloat(
                                                    "sv_botPitchSpeedAds",
                                                    5.0,
                                                    0.0,
                                                    100.0,
                                                    0x80u,
                                                    "Bot pitch lerp speed used when engaging");
    sv_botStrafeChance = _Dvar_RegisterFloat(
                                                 "sv_botStrafeChance",
                                                 0.1,
                                                 0.0,
                                                 1.0,
                                                 0x80u,
                                                 "The percentage chance that the bot will strafe when engaged");
    sv_botMinStrafeTime = _Dvar_RegisterInt(
                                                    "sv_botMinStrafeTime",
                                                    3000,
                                                    0,
                                                    10000,
                                                    0x80u,
                                                    "Bot minimum time to strafe when engaged");
    sv_botMaxStrafeTime = _Dvar_RegisterInt(
                                                    "sv_botMaxStrafeTime",
                                                    6000,
                                                    0,
                                                    10000,
                                                    0x80u,
                                                    "Bot maximum time to strafe when engaged");
}

void __cdecl Bot_DrawDebug(const ScreenPlacement *scrPlace)
{
    char *v1; // eax
    int SprintLeft; // eax
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    char *v11; // eax
    char *v12; // eax
    char *v13; // eax
    char *v14; // eax
    char *v15; // eax
    char *v16; // eax
    char *v17; // eax
    int AmmoInClip; // eax
    char *v19; // eax
    int AmmoNotInClip; // [esp+4h] [ebp-DCh]
    float *v21; // [esp+14h] [ebp-CCh]
    float *v22; // [esp+18h] [ebp-C8h]
    float *v23; // [esp+1Ch] [ebp-C4h]
    int health; // [esp+24h] [ebp-BCh]
    float *v25; // [esp+28h] [ebp-B8h]
    float dot; // [esp+2Ch] [ebp-B4h]
    float *v27; // [esp+30h] [ebp-B0h]
    float v28; // [esp+34h] [ebp-ACh]
    float *v29; // [esp+38h] [ebp-A8h]
    int v30; // [esp+3Ch] [ebp-A4h]
    float *v31; // [esp+40h] [ebp-A0h]
    const char *v32; // [esp+48h] [ebp-98h]
    float *v33; // [esp+4Ch] [ebp-94h]
    float *v34; // [esp+50h] [ebp-90h]
    float *v35; // [esp+54h] [ebp-8Ch]
    float fLookaheadDist; // [esp+58h] [ebp-88h]
    float *v37; // [esp+5Ch] [ebp-84h]
    float *v38; // [esp+60h] [ebp-80h]
    float scriptGoalRadius; // [esp+64h] [ebp-7Ch]
    float *v40; // [esp+68h] [ebp-78h]
    int v41; // [esp+7Ch] [ebp-64h]
    float *v42; // [esp+80h] [ebp-60h]
    const char *v43; // [esp+84h] [ebp-5Ch]
    float *v44; // [esp+88h] [ebp-58h]
    float *v45; // [esp+8Ch] [ebp-54h]
    const char *v46; // [esp+90h] [ebp-50h]
    float *color; // [esp+94h] [ebp-4Ch]
    char v48; // [esp+9Ch] [ebp-44h]
    const gentity_s *enemy; // [esp+B0h] [ebp-30h]
    const client_t *v50; // [esp+B4h] [ebp-2Ch]
    bool dead; // [esp+BFh] [ebp-21h]
    char *name; // [esp+C0h] [ebp-20h]
    const bot_info_t *botInfo; // [esp+C4h] [ebp-1Ch]
    const WeaponVariantDef *weapVariantDef; // [esp+C8h] [ebp-18h]
    gclient_s *ps; // [esp+CCh] [ebp-14h]
    const WeaponDef *weapDef; // [esp+D0h] [ebp-10h]
    int clientNum; // [esp+D4h] [ebp-Ch]
    int x; // [esp+D8h] [ebp-8h]
    float y; // [esp+DCh] [ebp-4h] BYREF

    clientNum = sv_botDebug->current.integer;
    name = (char *)"<None>";
    x = sv_botDebugX->current.integer;
    y = sv_botDebugY->current.value;
    if ( clientNum >= 0 )
    {
        v50 = &svs.clients[clientNum];
        if ( v50->header.state == CS_ACTIVE && v50->bIsTestClient && v50->gentity )
        {
            botInfo = &botInfos[clientNum];
            ps = G_GetPlayerState(clientNum);
            weapDef = BG_GetWeaponDef(ps->ps.weapon);
            weapVariantDef = BG_GetWeaponVariantDef(ps->ps.weapon);
            dead = v50->gentity->health <= 0;
            if ( botInfo->threat.enemy
                && ((enemy = botInfo->threat.enemy) == 0 || !enemy->client ? (v48 = 0) : (v48 = 1), v48) )
            {
                name = svs.clients[botInfo->threat.enemy->s.number].name;
            }
            else if ( botInfo->threat.enemy )
            {
                name = SL_ConvertToString(botInfo->threat.enemy->classname, SCRIPTINSTANCE_SERVER);
            }
            Bot_DrawString(scrPlace, (char*)v50->name, *(float *)&x, &y, colorMdCyan);
            Bot_DrawString(scrPlace, (char *)"", *(float *)&x, &y, colorWhite);
            Bot_DrawString(scrPlace, (char *)"MOVEMENT", *(float *)&x, &y, colorMdCyan);
            if ( dead )
            {
                Bot_DrawString(scrPlace, (char *)"STANCE: Dead", *(float *)&x, &y, colorRedFaded);
            }
            else
            {
                switch ( ps->ps.viewHeightTarget )
                {
                    case 0xB:
                        Bot_DrawString(scrPlace, (char *)"STANCE: Prone", *(float *)&x, &y, colorWhite);
                        break;
                    case 0x16:
                        Bot_DrawString(scrPlace, (char *)"STANCE: Last Stand", *(float *)&x, &y, colorWhite);
                        break;
                    case 0x28:
                        Bot_DrawString(scrPlace, (char *)"STANCE: Crouched", *(float *)&x, &y, colorWhite);
                        break;
                    case 0x3C:
                        Bot_DrawString(scrPlace, (char *)"STANCE: Standing", *(float *)&x, &y, colorWhite);
                        break;
                    default:
                        Bot_DrawString(scrPlace, (char *)"STANCE: <Unknown>", *(float *)&x, &y, colorWhite);
                        break;
                }
            }
            if ( dead )
                color = (float *)colorWhiteFaded;
            else
                color = (float *)colorWhite;
            if ( (ps->ps.pm_flags & 0x8000) != 0 )
                v46 = "<YES>";
            else
                v46 = "<NO>";
            v1 = va("SPRINTING: %s", v46);
            Bot_DrawString(scrPlace, v1, *(float *)&x, &y, color);
            if ( dead )
                v45 = (float *)colorWhiteFaded;
            else
                v45 = (float *)colorWhite;
            SprintLeft = PM_GetSprintLeft(&ps->ps, svs.time);
            v3 = va("SPRINT TIME LEFT: %d", SprintLeft);
            Bot_DrawString(scrPlace, v3, *(float *)&x, &y, v45);
            if ( dead )
                v44 = (float *)colorWhiteFaded;
            else
                v44 = (float *)colorWhite;
            if ( ps->ps.fWeaponPosFrac <= 0.0 )
                v43 = "<NO>";
            else
                v43 = "<YES>";
            v4 = va("ADS: %s", v43);
            Bot_DrawString(scrPlace, v4, *(float *)&x, &y, v44);
            if ( dead )
                v42 = (float *)colorWhiteFaded;
            else
                v42 = (float *)colorWhite;
            if ( botInfo->adsEndTime - svs.time <= 0 )
                v41 = 0;
            else
                v41 = botInfo->adsEndTime - svs.time;
            v5 = va("ADS TIME LEFT: %d", v41);
            Bot_DrawString(scrPlace, v5, *(float *)&x, &y, v42);
            if ( dead )
                Bot_DrawString(scrPlace, (char *)"", *(float *)&x, &y, colorWhiteFaded);
            else
                Bot_DrawString(scrPlace, (char *)"", *(float *)&x, &y, colorWhite);
            Bot_DrawString(scrPlace, (char *)"GOAL", *(float *)&x, &y, colorMdCyan);
            if ( (botInfo->flags & 4) != 0 )
            {
                if ( dead )
                    Bot_DrawString(scrPlace, (char *)"TYPE: <Script Goal>", *(float *)&x, &y, colorWhiteFaded);
                else
                    Bot_DrawString(scrPlace, (char *)"TYPE: <Script Goal>", *(float *)&x, &y, colorWhite);
            }
            else if ( (botInfo->flags & 2) != 0 )
            {
                if ( dead )
                    Bot_DrawString(scrPlace, (char *)"TYPE: <Random Node>", *(float *)&x, &y, colorWhiteFaded);
                else
                    Bot_DrawString(scrPlace, (char *)"TYPE: <Random Node>", *(float *)&x, &y, colorWhite);
            }
            else if ( dead )
            {
                Bot_DrawString(scrPlace, (char *)"TYPE: <Threat>", *(float *)&x, &y, colorWhiteFaded);
            }
            else
            {
                Bot_DrawString(scrPlace, (char *)"TYPE: <Threat>", *(float *)&x, &y, colorWhite);
            }
            if ( dead )
                v40 = (float *)colorWhiteFaded;
            else
                v40 = (float *)colorWhite;
            if ( (botInfo->flags & 4) != 0 )
                scriptGoalRadius = botInfo->scriptGoalRadius;
            else
                scriptGoalRadius = sv_botGoalRadius->current.value;
            v6 = va("GOAL RADIUS: %.1f", scriptGoalRadius);
            Bot_DrawString(scrPlace, v6, *(float *)&x, &y, v40);
            if ( dead )
                v38 = (float *)colorWhiteFaded;
            else
                v38 = (float *)colorWhite;
            v7 = va("SIGHT DIST: %.1f", weapDef->fMinDamageRange);
            Bot_DrawString(scrPlace, v7, *(float *)&x, &y, v38);
            if ( dead )
                v37 = (float *)colorWhiteFaded;
            else
                v37 = (float *)colorWhite;
            if ( Path_Exists(&botInfo->path) )
                fLookaheadDist = botInfo->path.fLookaheadDist;
            else
                fLookaheadDist = 0.0f;
            v8 = va("LOOK AHEAD DIST: %.1f", fLookaheadDist);
            Bot_DrawString(scrPlace, v8, *(float *)&x, &y, v37);
            Bot_DrawString(scrPlace, (char *)"", *(float *)&x, &y, colorWhite);
            Bot_DrawString(scrPlace, (char *)"THREAT", *(float *)&x, &y, colorMdCyan);
            if ( dead )
                v35 = (float *)colorWhiteFaded;
            else
                v35 = (float *)colorWhite;
            v9 = va("ENEMY: %s", name);
            Bot_DrawString(scrPlace, v9, *(float *)&x, &y, v35);
            if ( dead )
                v34 = (float *)colorWhiteFaded;
            else
                v34 = (float *)colorWhite;
            v10 = va("ACQUIRE TIME: %d", botInfo->threat.acquireTime);
            Bot_DrawString(scrPlace, v10, *(float *)&x, &y, v34);
            if ( dead )
                v33 = (float *)colorWhiteFaded;
            else
                v33 = (float *)colorWhite;
            if ( botInfo->sightHitNum )
                v32 = "<NO>";
            else
                v32 = "<YES>";
            v11 = va("LOS: %s", v32);
            Bot_DrawString(scrPlace, v11, *(float *)&x, &y, v33);
            if ( botInfo->threat.enemy && botInfo->threat.lastSightTime == svs.time )
            {
                if ( dead )
                    Bot_DrawString(scrPlace, (char *)"LAST SEEN: <NOW>", *(float *)&x, &y, colorWhiteFaded);
                else
                    Bot_DrawString(scrPlace, (char *)"LAST SEEN: <NOW>", *(float *)&x, &y, colorWhite);
            }
            else
            {
                if ( dead )
                    v31 = (float *)colorWhiteFaded;
                else
                    v31 = (float *)colorWhite;
                if ( botInfo->threat.lastSightTime <= 0 )
                    v30 = 0;
                else
                    v30 = svs.time - botInfo->threat.lastSightTime;
                v12 = va("LAST SEEN: %d", v30);
                Bot_DrawString(scrPlace, v12, *(float *)&x, &y, v31);
            }
            if ( dead )
                v29 = (float *)colorWhiteFaded;
            else
                v29 = (float *)colorWhite;
            if ( botInfo->threat.enemy )
                v28 = sqrtf(botInfo->threat.distSq);
            else
                v28 = 0.0f;
            v13 = va("DISTANCE: %.1f", v28);
            Bot_DrawString(scrPlace, v13, *(float *)&x, &y, v29);
            if ( dead )
                v27 = (float *)colorWhiteFaded;
            else
                v27 = (float *)colorWhite;
            if ( botInfo->threat.enemy )
                dot = botInfo->threat.dot;
            else
                dot = 0.0f;
            v14 = va("DOT: %.2f", dot);
            Bot_DrawString(scrPlace, v14, *(float *)&x, &y, v27);
            if ( dead )
                v25 = (float *)colorWhiteFaded;
            else
                v25 = (float *)colorWhite;
            if ( botInfo->threat.enemy )
                health = botInfo->threat.enemy->health;
            else
                health = 0;
            v15 = va("HEALTH: %d", health);
            Bot_DrawString(scrPlace, v15, *(float *)&x, &y, v25);
            if ( dead )
                Bot_DrawString(scrPlace, (char *)"", *(float *)&x, &y, colorWhiteFaded);
            else
                Bot_DrawString(scrPlace, (char *)"", *(float *)&x, &y, colorWhite);
            Bot_DrawString(scrPlace, (char *)"COMBAT", *(float *)&x, &y, colorMdCyan);
            if ( dead )
                v23 = (float *)colorWhiteFaded;
            else
                v23 = (float *)colorWhite;
            v16 = va("HEALTH: %d", v50->gentity->health);
            Bot_DrawString(scrPlace, v16, *(float *)&x, &y, v23);
            if ( dead )
                v22 = (float *)colorWhiteFaded;
            else
                v22 = (float *)colorWhite;
            v17 = va("WEAPON: %s", weapVariantDef->szInternalName);
            Bot_DrawString(scrPlace, v17, *(float *)&x, &y, v22);
            if ( dead )
                v21 = (float *)colorWhiteFaded;
            else
                v21 = (float *)colorWhite;
            AmmoNotInClip = BG_GetAmmoNotInClip(&ps->ps, ps->ps.weapon);
            AmmoInClip = BG_GetAmmoInClip(&ps->ps, ps->ps.weapon);
            v19 = va("AMMO: %d/%d", AmmoInClip, AmmoNotInClip);
            Bot_DrawString(scrPlace, v19, *(float *)&x, &y, v21);
        }
    }
}

void __cdecl Bot_DrawString(const ScreenPlacement *scrPlace, char *text, float x, float *y, const float *color)
{
    CG_DrawDevString(scrPlace, x, *y, 0.80000001, 0.80000001, text, color, 5, cgMedia.smallDevFont);
    *y = *y + 10.0;
}

void __cdecl SV_DrawBotPath()
{
    int clientNum; // [esp+4h] [ebp-4h]
    unsigned int clientNuma; // [esp+4h] [ebp-4h]

    if ( sv_botDebugPaths->current.integer > -1 )
    {
        if ( sv_botDebugPaths->current.integer < com_maxclients->current.integer )
        {
            clientNuma = sv_botDebugPaths->current.unsignedInt;
            if ( clientNuma >= com_maxclients->current.integer
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_bot_mp.cpp",
                            3243,
                            0,
                            "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                            clientNuma,
                            com_maxclients->current.integer) )
            {
                __debugbreak();
            }
            Bot_DrawPath(&svs.clients[clientNuma]);
        }
        else
        {
            for ( clientNum = 0; clientNum < com_maxclients->current.integer; ++clientNum )
                Bot_DrawPath(&svs.clients[clientNum]);
        }
    }
}

void __cdecl Bot_DrawPath(const client_t *bot)
{
    bot_info_t *botPath; // [esp+0h] [ebp-4h]

    if ( bot->header.state == CS_ACTIVE && bot->bIsTestClient && bot->gentity->health > 0 )
    {
        botPath = &botInfos[bot->gentity->s.number];
        if ( Path_Exists(&botPath->path) )
            Path_DebugDraw(&botPath->path, bot->gentity->r.currentOrigin, bot->gentity->s.number, -1);
    }
}

void __cdecl SV_DrawBotThreat()
{
    int clientNum; // [esp+4h] [ebp-4h]
    unsigned int clientNuma; // [esp+4h] [ebp-4h]
    int savedregs; // [esp+8h] [ebp+0h] BYREF

    if ( sv_botDebugThreat->current.integer > -1 )
    {
        if ( sv_botDebugThreat->current.integer < com_maxclients->current.integer )
        {
            clientNuma = sv_botDebugThreat->current.unsignedInt;
            if ( clientNuma >= com_maxclients->current.integer
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_bot_mp.cpp",
                            3367,
                            0,
                            "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                            clientNuma,
                            com_maxclients->current.integer) )
            {
                __debugbreak();
            }
            Bot_DrawThreat(&svs.clients[clientNuma]);
        }
        else
        {
            for ( clientNum = 0; clientNum < com_maxclients->current.integer; ++clientNum )
                Bot_DrawThreat(&svs.clients[clientNum]);
        }
    }
}

void    Bot_DrawThreat(const client_t *bot)
{
    float v2; // xmm0_4
    float *v3; // [esp-10h] [ebp-B4h]
    float v4[3]; // [esp-Ch] [ebp-B0h] BYREF
    float ends[4][3]; // [esp+0h] [ebp-A4h] BYREF
    int len; // [esp+30h] [ebp-74h]
    float angles[4]; // [esp+34h] [ebp-70h] BYREF
    int i; // [esp+44h] [ebp-60h]
    float fMinDamageRange; // [esp+48h] [ebp-5Ch]
    float vForward[3]; // [esp+4Ch] [ebp-58h] BYREF
    const float (*color)[4]; // [esp+58h] [ebp-4Ch]
    const float *v12; // [esp+5Ch] [ebp-48h]
    float vEnd[4]; // [esp+64h] [ebp-40h] BYREF
    float vStart[3]; // [esp+74h] [ebp-30h] BYREF
    float *currentOrigin; // [esp+80h] [ebp-24h]
    const WeaponDef *weapDef; // [esp+84h] [ebp-20h]
    const playerState_s *ps; // [esp+88h] [ebp-1Ch]
    const gentity_s *enemy; // [esp+8Ch] [ebp-18h]
    const bot_info_t *botInfo; // [esp+90h] [ebp-14h]
    int clientNum; // [esp+94h] [ebp-10h]
    //_UNKNOWN *v22; // [esp+98h] [ebp-Ch]
    //const client_t *bota; // [esp+9Ch] [ebp-8h]
    //int vars0; // [esp+A4h] [ebp+0h]
    //
    //v22 = a1;
    //bota = (const client_t *)vars0;
    if (bot->header.state == CS_ACTIVE && bot->bIsTestClient && bot->gentity->health > 0)
    {
        clientNum = bot->gentity->s.number;
        botInfo = &botInfos[clientNum];
        enemy = botInfo->threat.enemy;
        ps = (const playerState_s *)G_GetPlayerState(bot->gentity->s.number);
        weapDef = BG_GetWeaponDef(ps->weapon);
        currentOrigin = bot->gentity->r.currentOrigin;
        vStart[0] = *currentOrigin;
        vStart[1] = currentOrigin[1];
        vStart[2] = currentOrigin[2] + ps->viewHeightCurrent;
        if (enemy)
        {
            //LODWORD(vEnd[3]) = enemy->r.currentOrigin;
            vEnd[0] = enemy->r.currentOrigin[0];
            vEnd[1] = enemy->r.currentOrigin[1];
            vEnd[2] = enemy->r.currentOrigin[2];
            if (enemy->client != 0)
                vEnd[2] = vEnd[2] + enemy->client->ps.viewHeightCurrent;
            if (botInfo->sightHitNum)
                v12 = colorYellow;
            else
                v12 = colorRed;
            color = (const float (*)[4])v12;
            G_DebugLine(vStart, vEnd, colorRed, 0);
        }
        else
        {
            color = (const float (*)[4])colorGreen;
            AngleVectors(ps->viewangles, vForward, 0, 0);
            fMinDamageRange = weapDef->fMinDamageRange;
            vEnd[0] = (float)(fMinDamageRange * vForward[0]) + vStart[0];
            vEnd[1] = (float)(fMinDamageRange * vForward[1]) + vStart[1];
            vEnd[2] = (float)(fMinDamageRange * vForward[2]) + vStart[2];
            G_DebugLine(vStart, vEnd, (const float *)color, 0);
        }
        for (i = 0; i < 4; ++i)
        {
            //LODWORD(angles[3]) = ps->viewangles;
            angles[0] = ps->viewangles[0];
            angles[1] = ps->viewangles[1];
            angles[2] = ps->viewangles[2];
            len = i;
            switch (i)
            {
            case 0:
                angles[1] = (float)(sv_botFov->current.value * 0.5) + angles[1];
                break;
            case 1:
                angles[1] = (float)((-sv_botFov->current.integer) * 0.5) + angles[1];
                break;
            case 2:
                angles[0] = (float)(sv_botFov->current.value * 0.5) + angles[0];
                break;
            case 3:
                angles[0] = (float)((-sv_botFov->current.integer) * 0.5) + angles[0];
                break;
            default:
                break;
            }
            ends[3][2] = (float)(sv_botFov->current.value * 0.5) * 0.017453292;
            //v2 = __libm_sse2_cos(ends[3][2]);
            v2 = cos(ends[3][2]);
            ends[3][1] = v2;
            ends[3][0] = (float)(1.0 / v2) * weapDef->fMinDamageRange;
            AngleVectors(angles, vForward, 0, 0);
            vEnd[0] = (float)(ends[3][0] * vForward[0]) + vStart[0];
            vEnd[1] = (float)(ends[3][0] * vForward[1]) + vStart[1];
            vEnd[2] = (float)(ends[3][0] * vForward[2]) + vStart[2];
            G_DebugLine(vStart, vEnd, (const float *)color, 0);
            v3 = &v4[3 * i];
            *v3 = vEnd[0];
            v3[1] = vEnd[1];
            v3[2] = vEnd[2];
        }
        G_DebugLine(v4, ends[1], (const float *)color, 0);
        G_DebugLine(v4, ends[2], (const float *)color, 0);
        G_DebugLine(ends[0], ends[1], (const float *)color, 0);
        G_DebugLine(ends[0], ends[2], (const float *)color, 0);
    }
}

int nameIndex = -1;
const char *__cdecl SV_BotNameRandom()
{
    if ( nameIndex == -1 )
    {
        nameIndex = irand(0, 47);
    }
    else if ( (unsigned int)++nameIndex >= 0x2F )
    {
        nameIndex = 0;
    }
    return botNames[nameIndex];
}

char __cdecl Friends_IsInCurrentSession(const char *name)
{
    client_t *clients; // [esp+0h] [ebp-2Ch]
    int i; // [esp+4h] [ebp-28h]
    char cleanName[32]; // [esp+8h] [ebp-24h] BYREF

    SV_SanitizeName(name, cleanName, 32);
    i = 0;
    clients = svs.clients;
    while ( i < com_maxclients->current.integer )
    {
        if ( !I_stricmp(clients->name, cleanName) )
            return 1;
        ++i;
        ++clients;
    }
    return 0;
}

char __cdecl SV_BotNameRandomFriend(int controller, char *buf, int bufLen)
{
    int numIteration; // [esp+0h] [ebp-CCh]
    FriendInfo friendInfo; // [esp+4h] [ebp-C8h] BYREF
    int maxIterations; // [esp+C0h] [ebp-Ch]
    int friendIndex; // [esp+C4h] [ebp-8h]
    int friendCount; // [esp+C8h] [ebp-4h]

    maxIterations = 18;
    if ( !sv_botUseFriendNames->current.enabled )
        return 0;
    friendCount = Friends_GetCount(controller, 0);
    if ( friendCount <= 0 )
        return 0;
    for ( numIteration = 0; numIteration < 18; ++numIteration )
    {
        friendIndex = rand() % friendCount;
        Friends_GetByIndex(controller, 0, friendIndex, &friendInfo);
        if ( (friendInfo.status & 0x30) == 0
            && !Friends_IsInCurrentSession(friendInfo.name)
            && !Bot_NameUsed(friendInfo.name) )
        {
            I_strncpyz(buf, friendInfo.name, bufLen);
            return 1;
        }
    }
    return 0;
}

client_t *__cdecl Bot_NameUsed(const char *name)
{
    int i; // [esp+0h] [ebp-8h]
    client_t *v3; // [esp+4h] [ebp-4h]

    for ( i = 0; i < com_maxclients->current.integer; ++i )
    {
        v3 = &svs.clients[i];
        if ( v3->bIsTestClient && v3 != (client_t *)-21872 && !I_stricmp(v3->name, name) )
            return v3;
    }
    return 0;
}

void __cdecl SV_BotNameUpdate(const char *name)
{
    const char *v1; // eax
    client_t *bot; // [esp+0h] [ebp-4h]

    bot = Bot_NameUsed(name);
    if ( bot )
    {
        if ( (unsigned int)(bot - svs.clients) >= com_maxclients->current.integer
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_bot_mp.cpp",
                        3517,
                        0,
                        "bot - svs.clients doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                        bot - svs.clients,
                        com_maxclients->current.integer) )
        {
            __debugbreak();
        }
        v1 = SV_BotNameRandom();
        Info_SetValueForKey(bot->userinfo, "name", v1);
        SV_UserinfoChanged(bot);
        ClientUserinfoChanged(bot - svs.clients);
    }
}

